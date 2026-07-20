#include "kisak_platform_android.h"

#include "kisak_android_fs_runtime.h"
#include "kisak_engine_filesystem_bridge_android.h"
#include "kisak_cod4_filesystem_android.h"
#include "kisak_fastfile_android.h"
#include "kisak_zone_parser_android.h"
#include "kisak_zone_rawfile_android.h"
#include "kisak_zone_loader_android.h"
#include "kisak_menu_scene_android.h"

#include <android/log.h>

#include <filesystem>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

namespace {
bool HasExtension(const fs::path& path, const char* extension) {
    return path.has_extension() && path.extension() == extension;
}

bool StartsWith(const std::string& text, const char* prefix) {
    return text.rfind(prefix, 0) == 0;
}

// Decompresses one representative zone to prove the FastFile pipeline works;
// code_post_gfx.ff is the smallest zone the real engine loads first.
std::string ProbeRepresentativeFastFile(const KisakCod4FilesystemMount& mount) {
    if (mount.fastFiles.empty()) {
        return "FastFile probe: aucun .ff monte";
    }

    const KisakMountedFastFile* chosen = &mount.fastFiles.front();
    for (const KisakMountedFastFile& fastFile : mount.fastFiles) {
        if (fastFile.basename == "code_post_gfx.ff") {
            chosen = &fastFile;
            break;
        }
    }

    constexpr uint64_t kMaxRetainedZoneBytes = 64ull * 1024 * 1024;
    std::vector<uint8_t> zone;
    const KisakFastFileProbe probe =
        ProbeFastFile(mount.rootPath + "/" + chosen->relativePath, &zone, kMaxRetainedZoneBytes);
    const std::string description = DescribeFastFileProbe(probe);
    __android_log_print(
        probe.fullyDecompressed ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
        "KisakCODAndroid",
        "FastFile probe: %s",
        description.c_str()
    );

    std::string zoneDescription = "zone non parsee (decompression incomplete)";
    std::string rawFileDescription = "rawfiles non extraits";
    if (probe.fullyDecompressed && probe.decompressedBytes <= kMaxRetainedZoneBytes) {
        const KisakZoneParse parse = ParseZoneAssetDirectory(zone);
        zoneDescription = DescribeZoneParse(parse);
        __android_log_print(
            parse.valid ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
            "KisakCODAndroid",
            "Zone parse: %s",
            zoneDescription.c_str()
        );

        const std::vector<KisakZoneRawFile> rawFiles = ScanZoneRawFiles(zone);
        std::string zoneStem = chosen->basename;
        if (const size_t dot = zoneStem.rfind('.'); dot != std::string::npos) {
            zoneStem.erase(dot);
        }
        const std::string dumpRoot = mount.rootPath + "/dump/rawfiles/" + zoneStem;
        std::string dumpErrors;
        const uint32_t written = DumpZoneRawFiles(zone, rawFiles, dumpRoot, dumpErrors);
        std::ostringstream rawOut;
        rawOut << DescribeZoneRawFiles(rawFiles)
               << ", " << written << " dumpes vers " << dumpRoot;
        if (!dumpErrors.empty()) {
            rawOut << " erreurs=" << dumpErrors;
        }
        rawFileDescription = rawOut.str();
        __android_log_print(
            written == rawFiles.size() ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
            "KisakCODAndroid",
            "Zone rawfiles: %s",
            rawFileDescription.c_str()
        );
    }
    return "FastFile probe: " + description
        + "\nZone parse: " + zoneDescription
        + "\nZone rawfiles: " + rawFileDescription;
}

// Runs the ported 64-bit zone loader against the boot zones needed for the
// front-end menu: code_post_gfx.ff (fonts/base assets), common.ff (shared
// front-end assets) and ui.ff (the single-player menu definitions). Each
// loaded zone is published to the render thread's menu scene.
std::string RunZoneLoaderProbe(const KisakCod4FilesystemMount& mount) {
    constexpr uint64_t kMaxZoneBytes = 512ull * 1024 * 1024;
    const char* kBootZones[] = {"code_post_gfx.ff", "common.ff", "ui.ff"};

    std::string summary;
    for (const char* zoneName : kBootZones) {
        const KisakMountedFastFile* chosen = nullptr;
        for (const KisakMountedFastFile& fastFile : mount.fastFiles) {
            if (fastFile.basename == zoneName) {
                chosen = &fastFile;
                break;
            }
        }
        if (chosen == nullptr) {
            summary += std::string("Zone loader: ") + zoneName + " introuvable\n";
            continue;
        }

        std::vector<uint8_t> zone;
        const KisakFastFileProbe probe =
            ProbeFastFile(mount.rootPath + "/" + chosen->relativePath, &zone, kMaxZoneBytes);
        if (!probe.fullyDecompressed || probe.decompressedBytes > kMaxZoneBytes) {
            summary += std::string("Zone loader ") + zoneName
                + ": decompression incomplete (" + probe.error + ")\n";
            continue;
        }

        auto result = std::make_shared<KisakZoneLoadResult>(LoadZoneAssets(zone));
        const std::string description =
            "Zone loader " + chosen->basename + ": " + DescribeZoneLoad(*result);
        __android_log_print(
            result->error.empty() ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
            "KisakCODAndroid",
            "%s",
            description.c_str()
        );
        if (result->error.empty() && result->stopReason.empty()) {
            AddKisakMenuZone(std::move(result));
        }
        summary += description + "\n";
    }
    return summary;
}
} // namespace

KisakDataProbe ProbeCod4DataDirectory(const std::string& rootPath) {
    KisakDataProbe probe;
    probe.rootPath = rootPath;

    std::error_code error;
    const fs::path root(rootPath);
    if (!fs::exists(root, error)) {
        probe.rootCreated = fs::create_directories(root, error);
    }
    probe.rootExists = fs::exists(root, error) && fs::is_directory(root, error);
    probe.mainExists = fs::is_directory(root / "main", error);
    probe.zoneExists = fs::is_directory(root / "zone", error);

    if (probe.rootExists) {
        for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, error), end;
             it != end && !error;
             it.increment(error)) {
            if (!it->is_regular_file(error)) {
                continue;
            }
            const fs::path path = it->path();
            const std::string filename = path.filename().string();
            if (HasExtension(path, ".iwd")) {
                ++probe.iwdCount;
                if (StartsWith(filename, "localized_")) {
                    ++probe.localizedIwdCount;
                }
            } else if (HasExtension(path, ".ff")) {
                ++probe.fastFileCount;
            }
        }
    }

    const KisakCod4FilesystemMount mount = MountCod4Filesystem(rootPath);
    const KisakEngineFilesystemPlan enginePlan = BuildEngineFilesystemStartupPlan(mount);
    const KisakAndroidFsRuntime fsRuntime = StartAndroidFsRuntime(mount, enginePlan);
    std::ostringstream out;
    out << "Dossier cree: " << (probe.rootCreated ? "oui" : "non") << '\n'
        << ".iwd trouves: " << probe.iwdCount << '\n'
        << "localized_*.iwd: " << probe.localizedIwdCount << '\n'
        << ".ff trouves: " << probe.fastFileCount << "\n\n"
        << mount.message << "\n\n"
        << enginePlan.message << "\n\n"
        << fsRuntime.message << "\n\n"
        << ProbeRepresentativeFastFile(mount) << "\n\n"
        << RunZoneLoaderProbe(mount);

    probe.message = out.str();
    return probe;
}
