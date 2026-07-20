#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Decoded RGBA8 image built from a COD4 IWI (IWi version 5) file.
struct KisakIwiTexture {
    bool valid = false;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t format = 0;
    uint8_t flags = 0;
    std::string formatName;
    std::vector<uint8_t> rgba;
    std::string error;
};

KisakIwiTexture DecodeIwiTexture(const std::vector<uint8_t>& fileBytes);

// Decoded RGBA8 cubemap (sky boxes), D3D face order +X -X +Y -Y +Z -Z.
struct KisakIwiCubemap {
    bool valid = false;
    uint16_t width = 0;
    uint16_t height = 0;
    std::vector<uint8_t> faces[6];
    std::string error;
};

KisakIwiCubemap DecodeIwiCubemap(const std::vector<uint8_t>& fileBytes);

// Decodes the top mip of a zone-embedded D3D image payload (mip 0 stored
// first, unlike .iwi files). d3dFormat is a FOURCC (DXT1/3/5) or D3DFMT enum.
KisakIwiTexture DecodeD3DTopMip(
    uint32_t d3dFormat,
    uint16_t width,
    uint16_t height,
    const uint8_t* data,
    size_t dataSize);

std::string DescribeIwiTexture(const KisakIwiTexture& texture);
