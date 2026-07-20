#include "kisak_fastfile_android.h"

#include <zlib.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <sstream>

namespace {
constexpr char kFastFileMagic[8] = {'I', 'W', 'f', 'f', 'u', '1', '0', '0'};
constexpr uint32_t kFastFileVersionCod4 = 5;
constexpr size_t kZoneBlockSizeCount = 9;
constexpr size_t kXFileHeaderBytes = (2 + kZoneBlockSizeCount) * 4;
constexpr uint64_t kMaxDecompressedBytes = 512ull * 1024 * 1024;
constexpr size_t kChunkBytes = 256 * 1024;

uint32_t ReadLe32(const uint8_t* data) {
    return static_cast<uint32_t>(data[0])
        | (static_cast<uint32_t>(data[1]) << 8)
        | (static_cast<uint32_t>(data[2]) << 16)
        | (static_cast<uint32_t>(data[3]) << 24);
}
} // namespace

KisakFastFileProbe ProbeFastFile(
    const std::string& absolutePath,
    std::vector<uint8_t>* zoneOut,
    uint64_t maxZoneBytes
) {
    KisakFastFileProbe probe;
    probe.path = absolutePath;
    if (zoneOut != nullptr) {
        zoneOut->clear();
    }

    std::ifstream file(absolutePath, std::ios::binary);
    if (!file) {
        probe.error = "fichier introuvable";
        return probe;
    }
    probe.found = true;

    file.seekg(0, std::ios::end);
    probe.fileBytes = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::array<uint8_t, 12> header{};
    file.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!file || std::memcmp(header.data(), kFastFileMagic, sizeof(kFastFileMagic)) != 0) {
        probe.error = "magic IWffu100 absent";
        return probe;
    }
    probe.version = ReadLe32(header.data() + 8);
    if (probe.version != kFastFileVersionCod4) {
        probe.error = "version FastFile non supportee: " + std::to_string(probe.version);
        return probe;
    }
    probe.headerValid = true;

    z_stream stream{};
    if (inflateInit(&stream) != Z_OK) {
        probe.error = "inflateInit a echoue";
        return probe;
    }

    std::vector<uint8_t> input(kChunkBytes);
    std::vector<uint8_t> output(kChunkBytes);
    std::vector<uint8_t> xfileHeader;
    xfileHeader.reserve(kXFileHeaderBytes);

    int zlibResult = Z_OK;
    while (zlibResult != Z_STREAM_END) {
        if (stream.avail_in == 0) {
            file.read(reinterpret_cast<char*>(input.data()), input.size());
            const std::streamsize readBytes = file.gcount();
            if (readBytes <= 0) {
                probe.error = "flux zlib tronque";
                break;
            }
            stream.next_in = input.data();
            stream.avail_in = static_cast<uInt>(readBytes);
        }

        stream.next_out = output.data();
        stream.avail_out = static_cast<uInt>(output.size());
        zlibResult = inflate(&stream, Z_NO_FLUSH);
        if (zlibResult != Z_OK && zlibResult != Z_STREAM_END) {
            probe.error = "inflate a echoue: " + std::to_string(zlibResult);
            break;
        }

        const size_t produced = output.size() - stream.avail_out;
        if (xfileHeader.size() < kXFileHeaderBytes && produced > 0) {
            const size_t wanted = kXFileHeaderBytes - xfileHeader.size();
            const size_t take = std::min(wanted, produced);
            xfileHeader.insert(xfileHeader.end(), output.data(), output.data() + take);
        }
        if (zoneOut != nullptr && zoneOut->size() < maxZoneBytes && produced > 0) {
            const size_t take = std::min<uint64_t>(maxZoneBytes - zoneOut->size(), produced);
            zoneOut->insert(zoneOut->end(), output.data(), output.data() + take);
        }
        probe.decompressedBytes += produced;
        if (probe.decompressedBytes > kMaxDecompressedBytes) {
            probe.error = "zone decompresse au-dela de la limite de securite";
            break;
        }
    }
    inflateEnd(&stream);
    probe.fullyDecompressed = zlibResult == Z_STREAM_END;

    if (xfileHeader.size() >= kXFileHeaderBytes) {
        probe.zoneSize = ReadLe32(xfileHeader.data());
        probe.zoneExternalSize = ReadLe32(xfileHeader.data() + 4);
        for (size_t i = 0; i < kZoneBlockSizeCount; ++i) {
            probe.zoneBlockSizes.push_back(ReadLe32(xfileHeader.data() + 8 + i * 4));
        }
    }
    return probe;
}

KisakFastFileProbe ProbeFastFileHeader(const std::string& absolutePath) {
    KisakFastFileProbe probe;
    probe.path = absolutePath;

    std::ifstream file(absolutePath, std::ios::binary);
    if (!file) {
        probe.error = "fichier introuvable";
        return probe;
    }
    probe.found = true;

    file.seekg(0, std::ios::end);
    probe.fileBytes = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::array<uint8_t, 12> header{};
    file.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!file || std::memcmp(header.data(), kFastFileMagic, sizeof(kFastFileMagic)) != 0) {
        probe.error = "magic IWffu100 absent";
        return probe;
    }
    probe.version = ReadLe32(header.data() + 8);
    if (probe.version != kFastFileVersionCod4) {
        probe.error = "version FastFile non supportee: " + std::to_string(probe.version);
        return probe;
    }
    probe.headerValid = true;

    z_stream stream{};
    if (inflateInit(&stream) != Z_OK) {
        probe.error = "inflateInit a echoue";
        return probe;
    }

    std::vector<uint8_t> input(kChunkBytes);
    std::vector<uint8_t> output(kChunkBytes);
    std::vector<uint8_t> xfileHeader;
    xfileHeader.reserve(kXFileHeaderBytes);

    while (xfileHeader.size() < kXFileHeaderBytes) {
        if (stream.avail_in == 0) {
            file.read(reinterpret_cast<char*>(input.data()), input.size());
            const std::streamsize readBytes = file.gcount();
            if (readBytes <= 0) {
                probe.error = "flux zlib tronque";
                break;
            }
            stream.next_in = input.data();
            stream.avail_in = static_cast<uInt>(readBytes);
        }

        stream.next_out = output.data();
        stream.avail_out = static_cast<uInt>(output.size());
        const int zlibResult = inflate(&stream, Z_NO_FLUSH);
        if (zlibResult != Z_OK && zlibResult != Z_STREAM_END) {
            probe.error = "inflate a echoue: " + std::to_string(zlibResult);
            break;
        }
        const size_t produced = output.size() - stream.avail_out;
        if (produced > 0) {
            const size_t wanted = kXFileHeaderBytes - xfileHeader.size();
            const size_t take = std::min(wanted, produced);
            xfileHeader.insert(xfileHeader.end(), output.data(), output.data() + take);
            probe.decompressedBytes += produced;
        }
        if (zlibResult == Z_STREAM_END) {
            probe.fullyDecompressed = true;
            break;
        }
    }
    inflateEnd(&stream);

    if (xfileHeader.size() >= kXFileHeaderBytes) {
        probe.zoneSize = ReadLe32(xfileHeader.data());
        probe.zoneExternalSize = ReadLe32(xfileHeader.data() + 4);
        for (size_t i = 0; i < kZoneBlockSizeCount; ++i) {
            probe.zoneBlockSizes.push_back(ReadLe32(xfileHeader.data() + 8 + i * 4));
        }
    } else if (probe.error.empty()) {
        probe.error = "header XFile tronque";
    }
    return probe;
}

KisakFastFileDirectoryProbe ProbeFastFileAssetDirectory(
    const std::string& absolutePath,
    uint64_t maxPrefixBytes
) {
    KisakFastFileDirectoryProbe directory;
    KisakFastFileProbe& probe = directory.fastFile;
    probe.path = absolutePath;

    std::ifstream file(absolutePath, std::ios::binary);
    if (!file) {
        probe.error = "fichier introuvable";
        return directory;
    }
    probe.found = true;

    file.seekg(0, std::ios::end);
    probe.fileBytes = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::array<uint8_t, 12> header{};
    file.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!file || std::memcmp(header.data(), kFastFileMagic, sizeof(kFastFileMagic)) != 0) {
        probe.error = "magic IWffu100 absent";
        return directory;
    }
    probe.version = ReadLe32(header.data() + 8);
    if (probe.version != kFastFileVersionCod4) {
        probe.error = "version FastFile non supportee: " + std::to_string(probe.version);
        return directory;
    }
    probe.headerValid = true;

    z_stream stream{};
    if (inflateInit(&stream) != Z_OK) {
        probe.error = "inflateInit a echoue";
        return directory;
    }

    std::vector<uint8_t> input(kChunkBytes);
    std::vector<uint8_t> output(kChunkBytes);
    std::vector<uint8_t> zonePrefix;
    zonePrefix.reserve(std::min<uint64_t>(maxPrefixBytes, kChunkBytes));

    int zlibResult = Z_OK;
    while (zlibResult != Z_STREAM_END && !directory.zone.valid && probe.error.empty()) {
        if (stream.avail_in == 0) {
            file.read(reinterpret_cast<char*>(input.data()), input.size());
            const std::streamsize readBytes = file.gcount();
            if (readBytes <= 0) {
                probe.error = "flux zlib tronque";
                break;
            }
            stream.next_in = input.data();
            stream.avail_in = static_cast<uInt>(readBytes);
        }

        stream.next_out = output.data();
        stream.avail_out = static_cast<uInt>(output.size());
        zlibResult = inflate(&stream, Z_NO_FLUSH);
        if (zlibResult != Z_OK && zlibResult != Z_STREAM_END) {
            probe.error = "inflate a echoue: " + std::to_string(zlibResult);
            break;
        }

        const size_t produced = output.size() - stream.avail_out;
        if (produced > 0) {
            if (zonePrefix.size() + produced > maxPrefixBytes) {
                probe.error = "repertoire d'assets au-dela du prefixe autorise";
                break;
            }
            zonePrefix.insert(zonePrefix.end(), output.data(), output.data() + produced);
            probe.decompressedBytes += produced;
        }

        directory.zone = ParseZoneAssetDirectoryPrefix(zonePrefix);
        if (!directory.zone.valid && !directory.zone.needsMore) {
            probe.error = "repertoire d'assets: " + directory.zone.error;
            break;
        }
    }
    inflateEnd(&stream);
    probe.fullyDecompressed = zlibResult == Z_STREAM_END;

    if (directory.zone.valid) {
        probe.zoneSize = directory.zone.xfileSize;
        probe.zoneExternalSize = directory.zone.xfileExternalSize;
        probe.zoneBlockSizes = directory.zone.blockSizes;
    } else if (probe.error.empty()) {
        probe.error = "repertoire d'assets tronque";
    }
    return directory;
}

KisakFastFileProbe ProbeFastFileBytes(
    const std::string& label,
    const std::vector<uint8_t>& bytes,
    std::vector<uint8_t>* zoneOut,
    uint64_t maxZoneBytes
) {
    KisakFastFileProbe probe;
    probe.path = label;
    probe.fileBytes = bytes.size();
    if (zoneOut != nullptr) {
        zoneOut->clear();
    }
    if (bytes.size() < 12) {
        probe.error = "buffer trop petit";
        return probe;
    }
    probe.found = true;
    if (std::memcmp(bytes.data(), kFastFileMagic, sizeof(kFastFileMagic)) != 0) {
        probe.error = "magic IWffu100 absent";
        return probe;
    }
    probe.version = ReadLe32(bytes.data() + 8);
    if (probe.version != kFastFileVersionCod4) {
        probe.error = "version FastFile non supportee: " + std::to_string(probe.version);
        return probe;
    }
    probe.headerValid = true;

    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(bytes.data() + 12));
    stream.avail_in = static_cast<uInt>(bytes.size() - 12);
    if (inflateInit(&stream) != Z_OK) {
        probe.error = "inflateInit a echoue";
        return probe;
    }

    std::vector<uint8_t> output(kChunkBytes);
    std::vector<uint8_t> xfileHeader;
    xfileHeader.reserve(kXFileHeaderBytes);

    int zlibResult = Z_OK;
    while (zlibResult != Z_STREAM_END) {
        stream.next_out = output.data();
        stream.avail_out = static_cast<uInt>(output.size());
        zlibResult = inflate(&stream, Z_NO_FLUSH);
        if (zlibResult != Z_OK && zlibResult != Z_STREAM_END) {
            probe.error = "inflate a echoue: " + std::to_string(zlibResult);
            break;
        }

        const size_t produced = output.size() - stream.avail_out;
        if (xfileHeader.size() < kXFileHeaderBytes && produced > 0) {
            const size_t wanted = kXFileHeaderBytes - xfileHeader.size();
            const size_t take = std::min(wanted, produced);
            xfileHeader.insert(xfileHeader.end(), output.data(), output.data() + take);
        }
        if (zoneOut != nullptr && zoneOut->size() < maxZoneBytes && produced > 0) {
            const size_t take = std::min<uint64_t>(maxZoneBytes - zoneOut->size(), produced);
            zoneOut->insert(zoneOut->end(), output.data(), output.data() + take);
        }
        probe.decompressedBytes += produced;
        if (probe.decompressedBytes > kMaxDecompressedBytes) {
            probe.error = "zone decompresse au-dela de la limite de securite";
            break;
        }
        if (stream.avail_in == 0 && zlibResult != Z_STREAM_END) {
            probe.error = "flux zlib tronque";
            break;
        }
    }
    inflateEnd(&stream);
    probe.fullyDecompressed = zlibResult == Z_STREAM_END;

    if (xfileHeader.size() >= kXFileHeaderBytes) {
        probe.zoneSize = ReadLe32(xfileHeader.data());
        probe.zoneExternalSize = ReadLe32(xfileHeader.data() + 4);
        for (size_t i = 0; i < kZoneBlockSizeCount; ++i) {
            probe.zoneBlockSizes.push_back(ReadLe32(xfileHeader.data() + 8 + i * 4));
        }
    }
    return probe;
}

KisakFastFileProbe ProbeFastFileHeaderBytes(
    const std::string& label,
    const std::vector<uint8_t>& bytes
) {
    KisakFastFileProbe probe;
    probe.path = label;
    probe.fileBytes = bytes.size();
    if (bytes.size() < 12) {
        probe.error = "buffer trop petit";
        return probe;
    }
    probe.found = true;
    if (std::memcmp(bytes.data(), kFastFileMagic, sizeof(kFastFileMagic)) != 0) {
        probe.error = "magic IWffu100 absent";
        return probe;
    }
    probe.version = ReadLe32(bytes.data() + 8);
    if (probe.version != kFastFileVersionCod4) {
        probe.error = "version FastFile non supportee: " + std::to_string(probe.version);
        return probe;
    }
    probe.headerValid = true;

    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(bytes.data() + 12));
    stream.avail_in = static_cast<uInt>(bytes.size() - 12);
    if (inflateInit(&stream) != Z_OK) {
        probe.error = "inflateInit a echoue";
        return probe;
    }

    std::vector<uint8_t> output(kChunkBytes);
    std::vector<uint8_t> xfileHeader;
    xfileHeader.reserve(kXFileHeaderBytes);
    while (xfileHeader.size() < kXFileHeaderBytes) {
        stream.next_out = output.data();
        stream.avail_out = static_cast<uInt>(output.size());
        const int zlibResult = inflate(&stream, Z_NO_FLUSH);
        if (zlibResult != Z_OK && zlibResult != Z_STREAM_END) {
            probe.error = "inflate a echoue: " + std::to_string(zlibResult);
            break;
        }
        const size_t produced = output.size() - stream.avail_out;
        if (produced > 0) {
            const size_t wanted = kXFileHeaderBytes - xfileHeader.size();
            const size_t take = std::min(wanted, produced);
            xfileHeader.insert(xfileHeader.end(), output.data(), output.data() + take);
            probe.decompressedBytes += produced;
        }
        if (zlibResult == Z_STREAM_END) {
            probe.fullyDecompressed = true;
            break;
        }
        if (stream.avail_in == 0 && produced == 0) {
            probe.error = "flux zlib tronque";
            break;
        }
    }
    inflateEnd(&stream);

    if (xfileHeader.size() >= kXFileHeaderBytes) {
        probe.zoneSize = ReadLe32(xfileHeader.data());
        probe.zoneExternalSize = ReadLe32(xfileHeader.data() + 4);
        for (size_t i = 0; i < kZoneBlockSizeCount; ++i) {
            probe.zoneBlockSizes.push_back(ReadLe32(xfileHeader.data() + 8 + i * 4));
        }
    } else if (probe.error.empty()) {
        probe.error = "header XFile tronque";
    }
    return probe;
}

std::string DescribeFastFileProbe(const KisakFastFileProbe& probe) {
    std::ostringstream out;
    out << probe.path << ": ";
    if (!probe.found) {
        out << "introuvable";
        return out.str();
    }
    if (!probe.headerValid) {
        out << "header invalide (" << probe.error << ')';
        return out.str();
    }

    out << "IWffu100 v" << probe.version
        << " " << probe.fileBytes << " octets compresses -> "
        << probe.decompressedBytes << " octets zone"
        << (probe.fullyDecompressed ? "" : " [partiel]");
    if (!probe.zoneBlockSizes.empty()) {
        out << ", XFile size=" << probe.zoneSize
            << " externalSize=" << probe.zoneExternalSize
            << " blocks=[";
        for (size_t i = 0; i < probe.zoneBlockSizes.size(); ++i) {
            out << (i ? "," : "") << probe.zoneBlockSizes[i];
        }
        out << ']';
    }
    if (!probe.error.empty()) {
        out << " erreur=" << probe.error;
    }
    return out.str();
}

std::string DescribeFastFileDirectoryProbe(const KisakFastFileDirectoryProbe& probe) {
    std::ostringstream out;
    out << DescribeFastFileProbe(probe.fastFile);
    if (probe.zone.valid) {
        out << "; directory: " << DescribeZoneParse(probe.zone);
    } else if (probe.fastFile.headerValid && probe.fastFile.error.empty()) {
        out << "; directory: " << DescribeZoneParse(probe.zone);
    }
    return out.str();
}
