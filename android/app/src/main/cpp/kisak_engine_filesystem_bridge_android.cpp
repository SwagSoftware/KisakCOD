#include "kisak_engine_filesystem_bridge_android.h"

#include <algorithm>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

namespace {
bool Resolved(const KisakCod4FilesystemMount& mount, const char* name) {
    for (const KisakResolvedAsset& asset : mount.resolvedAssets) {
        if (asset.name == name && asset.found) {
            return true;
        }
    }
    return false;
}

bool HasFastFile(const KisakCod4FilesystemMount& mount, const char* relativePath) {
    for (const KisakMountedFastFile& fastFile : mount.fastFiles) {
        if (fastFile.relativePath == relativePath) {
            return true;
        }
    }
    return false;
}

std::string BuildPlanMessage(const KisakEngineFilesystemPlan& plan) {
    std::ostringstream out;
    out << "Bridge moteur:\n"
        << "fs_basepath: " << plan.basePath << '\n'
        << "fs_homepath: " << plan.homePath << '\n'
        << "fs_cdpath: " << (plan.cdPath.empty() ? "<vide>" : plan.cdPath) << '\n'
        << "fs_game: " << (plan.fsGame.empty() ? "<base>" : plan.fsGame) << '\n'
        << "gameName: " << plan.gameName << '\n'
        << "FastFile root: " << plan.fastFileRoot << '\n'
        << "Basepath pret: " << (plan.basepathReady ? "oui" : "non") << '\n'
        << "FastFiles prets: " << (plan.fastFileRootReady ? "oui" : "non") << '\n'
        << "Plan FS_Startup pret: " << (plan.startupReady ? "oui" : "non") << '\n';

    out << "Appels cibles:\n";
    for (const std::string& call : plan.startupCalls) {
        out << "- " << call << '\n';
    }

    out << "Lectures VFS:\n";
    for (const KisakVirtualFileRead& read : plan.readProbes) {
        out << "- " << DescribeVirtualFileRead(read) << '\n';
    }

    if (plan.startupReady) {
        out << "\nPret pour integration com_files: definir les dvars ci-dessus puis appeler FS_Startup(\"main\").";
    } else {
        out << "\nBridge incomplet: le moteur refuserait encore FS_InitFilesystem().";
    }
    return out.str();
}
} // namespace

KisakEngineFilesystemPlan BuildEngineFilesystemStartupPlan(
    const KisakCod4FilesystemMount& mount
) {
    KisakEngineFilesystemPlan plan;
    plan.basePath = mount.rootPath;
    plan.homePath = mount.rootPath;
    plan.cdPath = "";
    plan.gameName = "main";
    plan.fsGame = "";
    plan.fastFileRoot = (fs::path(mount.rootPath) / "zone" / "english").generic_string();

    plan.startupCalls.push_back("Dvar_SetString(fs_basepath, \"" + plan.basePath + "\")");
    plan.startupCalls.push_back("Dvar_SetString(fs_homepath, \"" + plan.homePath + "\")");
    plan.startupCalls.push_back("Dvar_SetString(fs_cdpath, \"\")");
    plan.startupCalls.push_back("Dvar_SetString(fs_game, \"\")");
    plan.startupCalls.push_back("SEH_InitLanguage()");
    plan.startupCalls.push_back("FS_Startup(\"main\")");
    plan.startupCalls.push_back("FS_IsBasePathValid() -> fileSysCheck.cfg");
    plan.readProbes.push_back(ReadCod4VirtualFile(mount.rootPath, "fileSysCheck.cfg", 4096));
    plan.readProbes.push_back(ReadCod4VirtualFile(mount.rootPath, "default.cfg", 4096));

    plan.basepathReady = mount.mounted
        && mount.localizationFileExists
        && mount.singlePlayerExeExists
        && mount.multiPlayerExeExists
        && Resolved(mount, "filesyscheck.cfg");

    plan.fastFileRootReady = mount.englishZoneExists
        && HasFastFile(mount, "zone/english/code_post_gfx.ff")
        && HasFastFile(mount, "zone/english/common.ff")
        && HasFastFile(mount, "zone/english/ui.ff");

    const bool vfsReadReady = std::all_of(
        plan.readProbes.begin(),
        plan.readProbes.end(),
        [](const KisakVirtualFileRead& read) {
            return read.found && read.error.empty() && !read.data.empty();
        }
    );

    plan.startupReady = plan.basepathReady && plan.fastFileRootReady && vfsReadReady;
    plan.message = BuildPlanMessage(plan);
    return plan;
}
