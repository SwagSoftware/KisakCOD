#pragma once

#include "kisak_cod4_filesystem_android.h"
#include "kisak_cod4_virtual_file_android.h"
#include "kisak_engine_filesystem_bridge_android.h"

#include <cstdint>
#include <string>

struct KisakAndroidFsRuntime {
    bool initialized = false;
    bool basePathValid = false;
    uint32_t preparedSearchPathCount = 0;
    uint32_t mountedIwdCount = 0;
    uint32_t filesInIwd = 0;
    uint32_t fastFileCount = 0;
    std::string fsGamedir;
    std::string message;
};

KisakAndroidFsRuntime StartAndroidFsRuntime(
    const KisakCod4FilesystemMount& mount,
    const KisakEngineFilesystemPlan& plan
);

bool AndroidFsInitialized();
std::string AndroidFsBasePath();

KisakVirtualFileRead AndroidFsReadFile(
    const std::string& qpath,
    uint32_t maxUncompressedBytes
);

void ShutdownAndroidFsRuntime();
