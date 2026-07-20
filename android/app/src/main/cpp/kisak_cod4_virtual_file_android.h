#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct KisakVirtualFileRead {
    std::string qpath;
    std::string source;
    bool found = false;
    bool compressed = false;
    uint16_t compressionMethod = 0;
    uint32_t compressedSize = 0;
    uint32_t uncompressedSize = 0;
    std::vector<uint8_t> data;
    std::string error;
};

KisakVirtualFileRead ReadCod4VirtualFile(
    const std::string& rootPath,
    const std::string& qpath,
    uint32_t maxUncompressedBytes
);

std::string DescribeVirtualFileRead(const KisakVirtualFileRead& read);
