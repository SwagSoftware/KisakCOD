#pragma once

#include <cstdint>
#include <string>
#include <vector>

bool DecodeIwiWaveletRgba(
    uint8_t format,
    const uint8_t* data,
    size_t dataSize,
    uint16_t width,
    uint16_t height,
    std::vector<uint8_t>& rgba,
    std::string& error
);
