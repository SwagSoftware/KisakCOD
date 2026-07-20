#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Parsed view of the start of a decompressed COD4 zone: the XFile header,
// the ScriptStringList, and the XAsset directory (type of every asset).
// Asset payloads are not parsed; they require the per-type IW3 structs.
struct KisakZoneParse {
    bool valid = false;
    bool needsMore = false;
    uint32_t xfileSize = 0;
    uint32_t xfileExternalSize = 0;
    std::vector<uint32_t> blockSizes;
    uint32_t scriptStringCount = 0;
    std::vector<std::string> scriptStrings;
    uint32_t assetCount = 0;
    std::vector<uint32_t> assetTypes;
    std::map<std::string, uint32_t> assetTypeHistogram;
    size_t parsedBytes = 0;
    std::string error;
};

KisakZoneParse ParseZoneAssetDirectory(const std::vector<uint8_t>& zone);
KisakZoneParse ParseZoneAssetDirectoryPrefix(const std::vector<uint8_t>& zonePrefix);

std::string DescribeZoneParse(const KisakZoneParse& parse);
