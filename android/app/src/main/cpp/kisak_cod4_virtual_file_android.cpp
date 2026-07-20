#include "kisak_cod4_virtual_file_android.h"

#include <zlib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

namespace {
constexpr uint32_t kZipEndOfCentralDirectorySignature = 0x06054b50;
constexpr uint32_t kZipCentralDirectoryFileHeaderSignature = 0x02014b50;
constexpr uint32_t kZipLocalFileHeaderSignature = 0x04034b50;
constexpr uint16_t kZipStored = 0;
constexpr uint16_t kZipDeflated = 8;

struct ZipEntry {
    std::string name;
    uint16_t compressionMethod = 0;
    uint32_t compressedSize = 0;
    uint32_t uncompressedSize = 0;
    uint32_t localHeaderOffset = 0;
};

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string NormalizeQpath(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');
    while (!value.empty() && value.front() == '/') {
        value.erase(value.begin());
    }
    return ToLower(std::move(value));
}

uint16_t ReadLe16(const unsigned char* data) {
    return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
}

uint32_t ReadLe32(const unsigned char* data) {
    return static_cast<uint32_t>(data[0])
        | (static_cast<uint32_t>(data[1]) << 8)
        | (static_cast<uint32_t>(data[2]) << 16)
        | (static_cast<uint32_t>(data[3]) << 24);
}

bool HasExtension(const fs::path& path, const char* extension) {
    return path.has_extension() && ToLower(path.extension().string()) == extension;
}

std::vector<fs::path> ListIwdFiles(const fs::path& root) {
    std::vector<fs::path> files;
    std::error_code error;
    if (!fs::is_directory(root, error)) {
        return files;
    }

    for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, error), end;
         it != end && !error;
         it.increment(error)) {
        if (it->is_regular_file(error) && HasExtension(it->path(), ".iwd")) {
            files.push_back(it->path());
        }
    }

    std::sort(files.begin(), files.end(), [](const fs::path& lhs, const fs::path& rhs) {
        return ToLower(lhs.generic_string()) < ToLower(rhs.generic_string());
    });
    return files;
}

std::string RelativeToRoot(const fs::path& root, const fs::path& path) {
    std::error_code error;
    fs::path relative = fs::relative(path, root, error);
    if (error) {
        relative = path.filename();
    }
    return relative.generic_string();
}

bool FindZipEntry(std::ifstream& file, const std::string& normalizedQpath, ZipEntry& entry, std::string& error) {
    file.seekg(0, std::ios::end);
    const std::streamoff fileSize = file.tellg();
    if (fileSize < 22) {
        error = "archive trop petite";
        return false;
    }

    const std::streamoff tailSize = std::min<std::streamoff>(fileSize, 65557);
    std::vector<unsigned char> tail(static_cast<size_t>(tailSize));
    file.seekg(fileSize - tailSize, std::ios::beg);
    file.read(reinterpret_cast<char*>(tail.data()), tailSize);
    if (!file) {
        error = "lecture EOCD impossible";
        return false;
    }

    size_t eocdOffset = std::string::npos;
    for (size_t i = tail.size() - 22; i != std::string::npos; --i) {
        if (ReadLe32(&tail[i]) == kZipEndOfCentralDirectorySignature) {
            eocdOffset = i;
            break;
        }
        if (i == 0) {
            break;
        }
    }
    if (eocdOffset == std::string::npos) {
        error = "EOCD ZIP introuvable";
        return false;
    }

    const unsigned char* eocd = &tail[eocdOffset];
    const uint16_t entryCount = ReadLe16(eocd + 10);
    const uint32_t centralDirectoryOffset = ReadLe32(eocd + 16);
    file.seekg(centralDirectoryOffset, std::ios::beg);

    for (uint16_t entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
        std::array<unsigned char, 46> header{};
        file.read(reinterpret_cast<char*>(header.data()), header.size());
        if (!file || ReadLe32(header.data()) != kZipCentralDirectoryFileHeaderSignature) {
            error = "header central directory invalide";
            return false;
        }

        const uint16_t method = ReadLe16(header.data() + 10);
        const uint32_t compressedSize = ReadLe32(header.data() + 20);
        const uint32_t uncompressedSize = ReadLe32(header.data() + 24);
        const uint16_t filenameLength = ReadLe16(header.data() + 28);
        const uint16_t extraLength = ReadLe16(header.data() + 30);
        const uint16_t commentLength = ReadLe16(header.data() + 32);
        const uint32_t localHeaderOffset = ReadLe32(header.data() + 42);

        std::string filename(filenameLength, '\0');
        file.read(filename.data(), filenameLength);
        if (!file) {
            error = "nom d'entree ZIP tronque";
            return false;
        }
        file.seekg(static_cast<std::streamoff>(extraLength) + commentLength, std::ios::cur);
        if (!file) {
            error = "entree ZIP tronquee";
            return false;
        }

        if (NormalizeQpath(std::move(filename)) == normalizedQpath) {
            entry.name = normalizedQpath;
            entry.compressionMethod = method;
            entry.compressedSize = compressedSize;
            entry.uncompressedSize = uncompressedSize;
            entry.localHeaderOffset = localHeaderOffset;
            return true;
        }
    }

    return false;
}

bool InflateRawDeflate(const std::vector<uint8_t>& compressed, std::vector<uint8_t>& output, std::string& error) {
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(compressed.data()));
    stream.avail_in = static_cast<uInt>(compressed.size());
    stream.next_out = reinterpret_cast<Bytef*>(output.data());
    stream.avail_out = static_cast<uInt>(output.size());

    int result = inflateInit2(&stream, -MAX_WBITS);
    if (result != Z_OK) {
        error = "inflateInit2 a echoue";
        return false;
    }

    result = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    if (result != Z_STREAM_END) {
        error = "inflate a echoue";
        return false;
    }
    return true;
}

bool ReadEntryData(std::ifstream& file, const ZipEntry& entry, std::vector<uint8_t>& data, std::string& error) {
    std::array<unsigned char, 30> localHeader{};
    file.seekg(entry.localHeaderOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(localHeader.data()), localHeader.size());
    if (!file || ReadLe32(localHeader.data()) != kZipLocalFileHeaderSignature) {
        error = "local header ZIP invalide";
        return false;
    }

    const uint16_t filenameLength = ReadLe16(localHeader.data() + 26);
    const uint16_t extraLength = ReadLe16(localHeader.data() + 28);
    file.seekg(static_cast<std::streamoff>(filenameLength) + extraLength, std::ios::cur);
    if (!file) {
        error = "data offset ZIP invalide";
        return false;
    }

    std::vector<uint8_t> compressed(entry.compressedSize);
    file.read(reinterpret_cast<char*>(compressed.data()), compressed.size());
    if (!file) {
        error = "donnees ZIP tronquees";
        return false;
    }

    if (entry.compressionMethod == kZipStored) {
        data = std::move(compressed);
        return true;
    }

    if (entry.compressionMethod != kZipDeflated) {
        error = "compression ZIP non supportee";
        return false;
    }

    data.assign(entry.uncompressedSize, 0);
    return InflateRawDeflate(compressed, data, error);
}

std::string BuildTextPreview(const std::vector<uint8_t>& data) {
    std::ostringstream out;
    const size_t count = std::min<size_t>(data.size(), 80);
    for (size_t i = 0; i < count; ++i) {
        const unsigned char ch = data[i];
        if (ch == '\n' || ch == '\r' || ch == '\t') {
            out << ' ';
        } else if (std::isprint(ch)) {
            out << static_cast<char>(ch);
        } else {
            out << '.';
        }
    }
    return out.str();
}
} // namespace

KisakVirtualFileRead ReadCod4VirtualFile(
    const std::string& rootPath,
    const std::string& qpath,
    uint32_t maxUncompressedBytes
) {
    KisakVirtualFileRead result;
    result.qpath = NormalizeQpath(qpath);

    const fs::path root(rootPath);
    const fs::path loosePath = root / result.qpath;
    std::error_code errorCode;
    if (fs::is_regular_file(loosePath, errorCode)) {
        const uintmax_t fileSize = fs::file_size(loosePath, errorCode);
        result.uncompressedSize = errorCode ? 0 : static_cast<uint32_t>(std::min<uintmax_t>(fileSize, UINT32_MAX));
        if (result.uncompressedSize > maxUncompressedBytes) {
            result.error = "fichier trop grand pour le probe";
            return result;
        }
        std::ifstream file(loosePath, std::ios::binary);
        result.data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        result.found = file.good() || file.eof();
        result.source = RelativeToRoot(root, loosePath);
        return result;
    }

    for (const fs::path& iwdPath : ListIwdFiles(root)) {
        std::ifstream file(iwdPath, std::ios::binary);
        if (!file) {
            continue;
        }

        ZipEntry entry;
        std::string zipError;
        if (!FindZipEntry(file, result.qpath, entry, zipError)) {
            continue;
        }

        result.found = true;
        result.source = RelativeToRoot(root, iwdPath);
        result.compressionMethod = entry.compressionMethod;
        result.compressedSize = entry.compressedSize;
        result.uncompressedSize = entry.uncompressedSize;
        result.compressed = entry.compressionMethod != kZipStored;

        if (entry.uncompressedSize > maxUncompressedBytes) {
            result.error = "entree trop grande pour le probe";
            return result;
        }

        if (!ReadEntryData(file, entry, result.data, result.error)) {
            result.data.clear();
        }
        return result;
    }

    result.error = "introuvable";
    return result;
}

std::string DescribeVirtualFileRead(const KisakVirtualFileRead& read) {
    std::ostringstream out;
    out << read.qpath << ": ";
    if (!read.found) {
        out << "introuvable";
        if (!read.error.empty()) {
            out << " (" << read.error << ')';
        }
        return out.str();
    }

    out << read.data.size() << "/" << read.uncompressedSize << " octets depuis " << read.source;
    if (read.compressed) {
        out << " deflate " << read.compressedSize << "->" << read.uncompressedSize;
    }
    if (!read.error.empty()) {
        out << " erreur=" << read.error;
    } else if (!read.data.empty()) {
        out << " preview=\"" << BuildTextPreview(read.data) << '"';
    }
    return out.str();
}
