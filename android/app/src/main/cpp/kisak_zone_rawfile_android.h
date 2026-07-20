#pragma once

#include <cstdint>
#include <string>
#include <vector>

// One RawFile asset recovered from a decompressed zone. The offset points at
// the serialized RawFile struct inside the zone buffer; content is the
// len-byte buffer that follows the inline name.
struct KisakZoneRawFile {
    std::string name;
    uint32_t length = 0;
    size_t zoneOffset = 0;
    size_t contentOffset = 0;
};

// Scans a decompressed zone for serialized RawFile assets using the inline
// pattern {name=-1, len, buffer=-1, name chars, NUL, len bytes, NUL}. This
// recovers rawfiles whose name and buffer are stored inline (the common
// case); entries referencing shared data need the full zone loader.
std::vector<KisakZoneRawFile> ScanZoneRawFiles(const std::vector<uint8_t>& zone);

// Writes every scanned rawfile under dumpRoot, preserving subdirectories.
// Returns the number of files written; errors are reported via errorOut.
uint32_t DumpZoneRawFiles(
    const std::vector<uint8_t>& zone,
    const std::vector<KisakZoneRawFile>& rawFiles,
    const std::string& dumpRoot,
    std::string& errorOut
);

std::string DescribeZoneRawFiles(const std::vector<KisakZoneRawFile>& rawFiles);
