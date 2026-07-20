#pragma once

#include "kisak_cod4_filesystem_android.h"
#include "kisak_cod4_virtual_file_android.h"

#include <string>
#include <vector>

struct KisakEngineFilesystemPlan {
    std::string basePath;
    std::string homePath;
    std::string cdPath;
    std::string gameName;
    std::string fsGame;
    std::string fastFileRoot;
    std::vector<KisakVirtualFileRead> readProbes;
    std::vector<std::string> startupCalls;
    bool basepathReady = false;
    bool fastFileRootReady = false;
    bool startupReady = false;
    std::string message;
};

KisakEngineFilesystemPlan BuildEngineFilesystemStartupPlan(
    const KisakCod4FilesystemMount& mount
);
