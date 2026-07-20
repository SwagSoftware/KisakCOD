#include "kisak_cod4_filesystem_android.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <system_error>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {
constexpr uint32_t kZipEndOfCentralDirectorySignature = 0x06054b50;
constexpr uint32_t kZipCentralDirectoryFileHeaderSignature = 0x02014b50;

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string NormalizeArchiveName(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');
    return ToLower(std::move(value));
}

std::string RelativeToRoot(const fs::path& root, const fs::path& path) {
    std::error_code error;
    fs::path relative = fs::relative(path, root, error);
    if (error) {
        relative = path.filename();
    }
    return relative.generic_string();
}

bool HasExtension(const fs::path& path, const char* extension) {
    return path.has_extension() && ToLower(path.extension().string()) == extension;
}

bool StartsWith(const std::string& text, const char* prefix) {
    return text.rfind(prefix, 0) == 0;
}

bool Contains(const std::string& text, const char* needle) {
    return text.find(needle) != std::string::npos;
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

bool ParseIwdZipDirectory(
    const fs::path& iwdPath,
    KisakMountedIwd& archive,
    std::unordered_map<std::string, std::string>& assetIndex
) {
    std::ifstream file(iwdPath, std::ios::binary);
    if (!file) {
        archive.error = "ouverture impossible";
        return false;
    }

    file.seekg(0, std::ios::end);
    const std::streamoff fileSize = file.tellg();
    if (fileSize < 22) {
        archive.error = "archive trop petite";
        return false;
    }

    const std::streamoff tailSize = std::min<std::streamoff>(fileSize, 65557);
    std::vector<unsigned char> tail(static_cast<size_t>(tailSize));
    file.seekg(fileSize - tailSize, std::ios::beg);
    file.read(reinterpret_cast<char*>(tail.data()), tailSize);
    if (!file) {
        archive.error = "lecture EOCD impossible";
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
        archive.error = "EOCD ZIP introuvable";
        return false;
    }

    const unsigned char* eocd = &tail[eocdOffset];
    const uint16_t entryCount = ReadLe16(eocd + 10);
    const uint32_t centralDirectoryOffset = ReadLe32(eocd + 16);
    if (centralDirectoryOffset >= static_cast<uint64_t>(fileSize)) {
        archive.error = "central directory hors fichier";
        return false;
    }

    file.seekg(centralDirectoryOffset, std::ios::beg);
    for (uint16_t entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
        std::array<unsigned char, 46> header{};
        file.read(reinterpret_cast<char*>(header.data()), header.size());
        if (!file || ReadLe32(header.data()) != kZipCentralDirectoryFileHeaderSignature) {
            archive.error = "header central directory invalide";
            return false;
        }

        const uint16_t filenameLength = ReadLe16(header.data() + 28);
        const uint16_t extraLength = ReadLe16(header.data() + 30);
        const uint16_t commentLength = ReadLe16(header.data() + 32);
        std::string filename(filenameLength, '\0');
        file.read(filename.data(), filenameLength);
        if (!file) {
            archive.error = "nom d'entree ZIP tronque";
            return false;
        }
        file.seekg(static_cast<std::streamoff>(extraLength) + commentLength, std::ios::cur);
        if (!file) {
            archive.error = "entree ZIP tronquee";
            return false;
        }

        const std::string normalized = NormalizeArchiveName(std::move(filename));
        if (!normalized.empty() && normalized.back() != '/') {
            assetIndex.emplace(normalized, archive.relativePath);
        }
    }

    archive.entryCount = entryCount;
    archive.validZip = true;
    return true;
}

std::vector<fs::path> ListFilesWithExtension(const fs::path& root, const char* extension) {
    std::vector<fs::path> files;
    std::error_code error;
    if (!fs::is_directory(root, error)) {
        return files;
    }

    for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, error), end;
         it != end && !error;
         it.increment(error)) {
        if (it->is_regular_file(error) && HasExtension(it->path(), extension)) {
            files.push_back(it->path());
        }
    }

    std::sort(files.begin(), files.end(), [](const fs::path& lhs, const fs::path& rhs) {
        return ToLower(lhs.generic_string()) < ToLower(rhs.generic_string());
    });
    return files;
}

void AddResolvedAsset(
    KisakCod4FilesystemMount& mount,
    const std::unordered_map<std::string, std::string>& assetIndex,
    const std::string& name
) {
    const std::string normalized = NormalizeArchiveName(name);
    KisakResolvedAsset resolved;
    resolved.name = normalized;

    const fs::path loosePath = fs::path(mount.rootPath) / normalized;
    std::error_code error;
    if (fs::is_regular_file(loosePath, error)) {
        resolved.found = true;
        resolved.source = RelativeToRoot(fs::path(mount.rootPath), loosePath);
    } else if (auto it = assetIndex.find(normalized); it != assetIndex.end()) {
        resolved.found = true;
        resolved.source = it->second;
    }

    mount.resolvedAssets.push_back(std::move(resolved));
}

std::string BuildMountMessage(const KisakCod4FilesystemMount& mount) {
    std::ostringstream out;
    out << "Data root: " << mount.rootPath << '\n'
        << "Root OK: " << (mount.rootExists ? "oui" : "non") << '\n'
        << "main/: " << (mount.mainExists ? "present" : "manquant") << '\n'
        << "zone/: " << (mount.zoneExists ? "present" : "manquant") << '\n'
        << "zone/english/: " << (mount.englishZoneExists ? "present" : "manquant") << '\n'
        << "localization.txt: " << (mount.localizationFileExists ? "present" : "manquant") << '\n'
        << "iw3sp.exe / iw3mp.exe: " << (mount.singlePlayerExeExists ? "oui" : "non")
        << " / " << (mount.multiPlayerExeExists ? "oui" : "non") << '\n'
        << "Search roots: " << mount.searchRoots.size() << '\n'
        << "IWD montes: " << mount.iwds.size() << '\n'
        << "Entrees IWD indexees: " << mount.totalIwdEntries << '\n'
        << "FastFiles .ff: " << mount.fastFiles.size() << '\n';

    const size_t shownIwdCount = std::min<size_t>(mount.iwds.size(), 4);
    if (shownIwdCount > 0) {
        out << "IWD exemples:";
        for (size_t i = 0; i < shownIwdCount; ++i) {
            out << ' ' << mount.iwds[i].basename;
        }
        if (mount.iwds.size() > shownIwdCount) {
            out << " ...";
        }
        out << '\n';
    }

    out << "Resolution assets:\n";
    for (const KisakResolvedAsset& asset : mount.resolvedAssets) {
        out << "- " << asset.name << ": ";
        if (asset.found) {
            out << asset.source;
        } else {
            out << "introuvable";
        }
        out << '\n';
    }

    if (mount.mounted) {
        out << "\nFilesystem COD4 monte. Prochaine etape: brancher FS_Startup/FS_AddGameDirectory sur cet index Android.";
    } else {
        out << "\nMount incomplet: verifiez que les fichiers COD4 originaux sont bien copies.";
    }
    return out.str();
}
} // namespace

KisakCod4FilesystemMount MountCod4Filesystem(const std::string& rootPath) {
    KisakCod4FilesystemMount mount;
    mount.rootPath = rootPath;

    std::error_code error;
    const fs::path root(rootPath);
    mount.rootExists = fs::is_directory(root, error);
    mount.mainExists = fs::is_directory(root / "main", error);
    mount.zoneExists = fs::is_directory(root / "zone", error);
    mount.englishZoneExists = fs::is_directory(root / "zone" / "english", error);
    mount.localizationFileExists = fs::is_regular_file(root / "localization.txt", error);
    mount.singlePlayerExeExists = fs::is_regular_file(root / "iw3sp.exe", error);
    mount.multiPlayerExeExists = fs::is_regular_file(root / "iw3mp.exe", error);

    if (mount.mainExists) {
        mount.searchRoots.push_back((root / "main").generic_string());
    }
    if (mount.englishZoneExists) {
        mount.searchRoots.push_back((root / "zone" / "english").generic_string());
    }
    if (fs::is_directory(root / "Mods", error)) {
        mount.searchRoots.push_back((root / "Mods").generic_string());
    }

    std::unordered_map<std::string, std::string> assetIndex;
    for (const fs::path& iwdPath : ListFilesWithExtension(root, ".iwd")) {
        KisakMountedIwd archive;
        archive.relativePath = RelativeToRoot(root, iwdPath);
        archive.basename = iwdPath.filename().string();
        const std::string basenameLower = ToLower(archive.basename);
        archive.localized = StartsWith(basenameLower, "localized_");

        ParseIwdZipDirectory(iwdPath, archive, assetIndex);
        mount.totalIwdEntries += archive.entryCount;
        mount.iwds.push_back(std::move(archive));
    }

    for (const fs::path& ffPath : ListFilesWithExtension(root / "zone", ".ff")) {
        KisakMountedFastFile fastFile;
        fastFile.relativePath = RelativeToRoot(root, ffPath);
        fastFile.basename = ffPath.filename().string();
        const std::string basenameLower = ToLower(fastFile.basename);
        fastFile.localized = StartsWith(basenameLower, "localized_");
        fastFile.multiplayer = StartsWith(basenameLower, "mp_") || Contains(basenameLower, "_mp");
        fastFile.loadFastFile = Contains(basenameLower, "_load.");
        mount.fastFiles.push_back(std::move(fastFile));
    }

    AddResolvedAsset(mount, assetIndex, "main/iw_00.iwd");
    AddResolvedAsset(mount, assetIndex, "zone/english/common.ff");
    AddResolvedAsset(mount, assetIndex, "images/$white.iwi");
    AddResolvedAsset(mount, assetIndex, "images/gamefonts_pc.iwi");
    AddResolvedAsset(mount, assetIndex, "default.cfg");
    AddResolvedAsset(mount, assetIndex, "fileSysCheck.cfg");

    const bool archivesValid = !mount.iwds.empty()
        && std::all_of(mount.iwds.begin(), mount.iwds.end(), [](const KisakMountedIwd& archive) {
            return archive.validZip;
        });
    const bool samplesResolved = std::all_of(
        mount.resolvedAssets.begin(),
        mount.resolvedAssets.end(),
        [](const KisakResolvedAsset& asset) { return asset.found; }
    );

    mount.mounted = mount.rootExists
        && mount.mainExists
        && mount.englishZoneExists
        && mount.localizationFileExists
        && archivesValid
        && !mount.fastFiles.empty()
        && samplesResolved;
    mount.message = BuildMountMessage(mount);
    return mount;
}
