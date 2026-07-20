#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct KisakMountedIwd {
    std::string relativePath;
    std::string basename;
    bool localized = false;
    bool validZip = false;
    uint32_t entryCount = 0;
    std::string error;
};

struct KisakMountedFastFile {
    std::string relativePath;
    std::string basename;
    bool localized = false;
    bool multiplayer = false;
    bool loadFastFile = false;
};

struct KisakResolvedAsset {
    std::string name;
    std::string source;
    bool found = false;
};

struct KisakCod4FilesystemMount {
    std::string rootPath;
    bool rootExists = false;
    bool mainExists = false;
    bool zoneExists = false;
    bool englishZoneExists = false;
    bool localizationFileExists = false;
    bool singlePlayerExeExists = false;
    bool multiPlayerExeExists = false;
    bool mounted = false;
    uint32_t totalIwdEntries = 0;
    std::vector<std::string> searchRoots;
    std::vector<KisakMountedIwd> iwds;
    std::vector<KisakMountedFastFile> fastFiles;
    std::vector<KisakResolvedAsset> resolvedAssets;
    std::string message;
};

KisakCod4FilesystemMount MountCod4Filesystem(const std::string& rootPath);
