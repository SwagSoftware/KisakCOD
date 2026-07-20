#include "kisak_android_fs_runtime.h"

#include <mutex>
#include <sstream>

namespace {
std::mutex g_fsMutex;
bool g_initialized = false;
std::string g_basePath;
std::string g_gameDir;

bool ReadSucceeded(const KisakVirtualFileRead& read) {
    return read.found && read.error.empty() && !read.data.empty();
}

bool PlanReadSucceeded(const KisakEngineFilesystemPlan& plan, const char* qpath) {
    for (const KisakVirtualFileRead& read : plan.readProbes) {
        if (read.qpath == qpath) {
            return ReadSucceeded(read);
        }
    }
    return false;
}
} // namespace

KisakAndroidFsRuntime StartAndroidFsRuntime(
    const KisakCod4FilesystemMount& mount,
    const KisakEngineFilesystemPlan& plan
) {
    KisakAndroidFsRuntime runtime;
    runtime.initialized = plan.startupReady;
    runtime.basePathValid = PlanReadSucceeded(plan, "filesyscheck.cfg");
    runtime.preparedSearchPathCount = static_cast<uint32_t>(mount.searchRoots.size() + mount.iwds.size());
    runtime.mountedIwdCount = static_cast<uint32_t>(mount.iwds.size());
    runtime.filesInIwd = mount.totalIwdEntries;
    runtime.fastFileCount = static_cast<uint32_t>(mount.fastFiles.size());
    runtime.fsGamedir = plan.gameName;

    {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        g_initialized = runtime.initialized;
        g_basePath = runtime.initialized ? plan.basePath : "";
        g_gameDir = runtime.initialized ? plan.gameName : "";
    }

    const KisakVirtualFileRead smokeRead = runtime.initialized
        ? AndroidFsReadFile("fileSysCheck.cfg", 4096)
        : KisakVirtualFileRead{};

    std::ostringstream out;
    out << "FS runtime Android:\n"
        << "FS_Initialized: " << (runtime.initialized ? "oui" : "non") << '\n'
        << "FS_IsBasePathValid: " << (runtime.basePathValid ? "oui" : "non") << '\n'
        << "fs_gamedir: " << (runtime.fsGamedir.empty() ? "<vide>" : runtime.fsGamedir) << '\n'
        << "searchpaths prepares: " << runtime.preparedSearchPathCount << '\n'
        << "iwd montes: " << runtime.mountedIwdCount << '\n'
        << "fichiers dans iwd: " << runtime.filesInIwd << '\n'
        << "fastfiles: " << runtime.fastFileCount << '\n';
    if (runtime.initialized) {
        out << "FS_ReadFile smoke: " << DescribeVirtualFileRead(smokeRead) << '\n'
            << "\nAPI native prete: AndroidFsReadFile(qpath) peut servir de backend temporaire a FS_FOpenFileRead.";
    } else {
        out << "\nRuntime non initialise: le bridge FS_Startup n'est pas encore pret.";
    }
    runtime.message = out.str();
    return runtime;
}

bool AndroidFsInitialized() {
    std::lock_guard<std::mutex> lock(g_fsMutex);
    return g_initialized;
}

std::string AndroidFsBasePath() {
    std::lock_guard<std::mutex> lock(g_fsMutex);
    return g_basePath;
}

KisakVirtualFileRead AndroidFsReadFile(
    const std::string& qpath,
    uint32_t maxUncompressedBytes
) {
    std::string basePath;
    {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        basePath = g_basePath;
    }

    if (basePath.empty()) {
        KisakVirtualFileRead result;
        result.qpath = qpath;
        result.error = "runtime FS non initialise";
        return result;
    }
    return ReadCod4VirtualFile(basePath, qpath, maxUncompressedBytes);
}

void ShutdownAndroidFsRuntime() {
    std::lock_guard<std::mutex> lock(g_fsMutex);
    g_initialized = false;
    g_basePath.clear();
    g_gameDir.clear();
}
