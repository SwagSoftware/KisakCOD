#pragma once

#include "kisak_zone_parser_android.h"

#include <cstdint>
#include <string>
#include <vector>

// Result of decompressing a COD4 FastFile (.ff) zone and reading its XFile
// header. The zone payload itself is streamed and only measured, not kept.
struct KisakFastFileProbe {
    std::string path;
    bool found = false;
    bool headerValid = false;
    bool fullyDecompressed = false;
    uint32_t version = 0;
    uint64_t fileBytes = 0;
    uint64_t decompressedBytes = 0;
    uint32_t zoneSize = 0;
    uint32_t zoneExternalSize = 0;
    std::vector<uint32_t> zoneBlockSizes;
    std::string error;
};

struct KisakFastFileDirectoryProbe {
    KisakFastFileProbe fastFile;
    KisakZoneParse zone;
};

// zoneOut, when non-null, receives the decompressed zone bytes up to
// maxZoneBytes (decompression still runs to the end to measure the total).
KisakFastFileProbe ProbeFastFile(
    const std::string& absolutePath,
    std::vector<uint8_t>* zoneOut = nullptr,
    uint64_t maxZoneBytes = 0
);

KisakFastFileProbe ProbeFastFileHeader(const std::string& absolutePath);
KisakFastFileDirectoryProbe ProbeFastFileAssetDirectory(
    const std::string& absolutePath,
    uint64_t maxPrefixBytes = 8ull * 1024 * 1024
);

KisakFastFileProbe ProbeFastFileBytes(
    const std::string& label,
    const std::vector<uint8_t>& bytes,
    std::vector<uint8_t>* zoneOut = nullptr,
    uint64_t maxZoneBytes = 0
);

KisakFastFileProbe ProbeFastFileHeaderBytes(
    const std::string& label,
    const std::vector<uint8_t>& bytes
);

std::string DescribeFastFileProbe(const KisakFastFileProbe& probe);
std::string DescribeFastFileDirectoryProbe(const KisakFastFileDirectoryProbe& probe);
