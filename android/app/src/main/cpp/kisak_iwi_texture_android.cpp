#include "kisak_iwi_texture_android.h"

#include "kisak_iwi_wavelet_android.h"

#include <algorithm>
#include <array>
#include <sstream>

namespace {
constexpr size_t kIwiHeaderSize = 28;
constexpr uint8_t kIwiVersionCod4 = 6;

constexpr uint8_t kFormatBitmapRgba = 1;
constexpr uint8_t kFormatBitmapRgb = 2;
constexpr uint8_t kFormatBitmapLumAlpha = 3;
constexpr uint8_t kFormatBitmapLum = 4;
constexpr uint8_t kFormatBitmapAlpha = 5;
constexpr uint8_t kFormatWaveletRgba = 6;
constexpr uint8_t kFormatWaveletRgb = 7;
constexpr uint8_t kFormatWaveletLumAlpha = 8;
constexpr uint8_t kFormatWaveletLum = 9;
constexpr uint8_t kFormatWaveletAlpha = 10;
constexpr uint8_t kFormatDxt1 = 11;
constexpr uint8_t kFormatDxt3 = 12;
constexpr uint8_t kFormatDxt5 = 13;

constexpr uint8_t kFlagCubemap = 0x4;
constexpr uint8_t kFlagVolmap = 0x8;

const char* FormatName(uint8_t format) {
    switch (format) {
        case kFormatBitmapRgba: return "RGBA8";
        case kFormatBitmapRgb: return "RGB8";
        case kFormatBitmapLumAlpha: return "LA8";
        case kFormatBitmapLum: return "L8";
        case kFormatBitmapAlpha: return "A8";
        case kFormatWaveletRgba: return "Wavelet RGBA8";
        case kFormatWaveletRgb: return "Wavelet RGB8";
        case kFormatWaveletLumAlpha: return "Wavelet LA8";
        case kFormatWaveletLum: return "Wavelet L8";
        case kFormatWaveletAlpha: return "Wavelet A8";
        case kFormatDxt1: return "DXT1";
        case kFormatDxt3: return "DXT3";
        case kFormatDxt5: return "DXT5";
        default: return "inconnu";
    }
}

size_t TopMipByteCount(uint8_t format, uint16_t width, uint16_t height) {
    const size_t blocksX = (static_cast<size_t>(width) + 3) / 4;
    const size_t blocksY = (static_cast<size_t>(height) + 3) / 4;
    const size_t pixels = static_cast<size_t>(width) * height;
    switch (format) {
        case kFormatBitmapRgba: return pixels * 4;
        case kFormatBitmapRgb: return pixels * 3;
        case kFormatBitmapLumAlpha: return pixels * 2;
        case kFormatBitmapLum:
        case kFormatBitmapAlpha: return pixels;
        case kFormatDxt1: return blocksX * blocksY * 8;
        case kFormatDxt3:
        case kFormatDxt5: return blocksX * blocksY * 16;
        default: return 0;
    }
}

bool IsWaveletFormat(uint8_t format) {
    return format >= kFormatWaveletRgba && format <= kFormatWaveletAlpha;
}

void DecodeRgb565(uint16_t value, uint8_t* rgb) {
    rgb[0] = static_cast<uint8_t>(((value >> 11) & 0x1f) * 255 / 31);
    rgb[1] = static_cast<uint8_t>(((value >> 5) & 0x3f) * 255 / 63);
    rgb[2] = static_cast<uint8_t>((value & 0x1f) * 255 / 31);
}

// Decodes one 8-byte DXT color block into 16 RGBA texels. alphaFromColor
// enables the DXT1 punch-through mode when c0 <= c1.
void DecodeColorBlock(const uint8_t* block, bool alphaFromColor, uint8_t* out) {
    const uint16_t c0 = static_cast<uint16_t>(block[0] | (block[1] << 8));
    const uint16_t c1 = static_cast<uint16_t>(block[2] | (block[3] << 8));

    std::array<std::array<uint8_t, 4>, 4> palette{};
    DecodeRgb565(c0, palette[0].data());
    palette[0][3] = 255;
    DecodeRgb565(c1, palette[1].data());
    palette[1][3] = 255;

    if (!alphaFromColor || c0 > c1) {
        for (int channel = 0; channel < 3; ++channel) {
            palette[2][channel] = static_cast<uint8_t>((2 * palette[0][channel] + palette[1][channel]) / 3);
            palette[3][channel] = static_cast<uint8_t>((palette[0][channel] + 2 * palette[1][channel]) / 3);
        }
        palette[2][3] = 255;
        palette[3][3] = 255;
    } else {
        for (int channel = 0; channel < 3; ++channel) {
            palette[2][channel] = static_cast<uint8_t>((palette[0][channel] + palette[1][channel]) / 2);
            palette[3][channel] = 0;
        }
        palette[2][3] = 255;
        palette[3][3] = 0;
    }

    const uint32_t indices = static_cast<uint32_t>(block[4])
        | (static_cast<uint32_t>(block[5]) << 8)
        | (static_cast<uint32_t>(block[6]) << 16)
        | (static_cast<uint32_t>(block[7]) << 24);
    for (int texel = 0; texel < 16; ++texel) {
        const uint32_t index = (indices >> (texel * 2)) & 0x3;
        std::copy_n(palette[index].data(), 4, out + texel * 4);
    }
}

// Fills the 16 alpha values of a DXT5 interpolated-alpha block.
void DecodeDxt5AlphaBlock(const uint8_t* block, uint8_t* alphas) {
    const uint8_t a0 = block[0];
    const uint8_t a1 = block[1];
    std::array<uint8_t, 8> palette{};
    palette[0] = a0;
    palette[1] = a1;
    if (a0 > a1) {
        for (int i = 1; i < 7; ++i) {
            palette[i + 1] = static_cast<uint8_t>(((7 - i) * a0 + i * a1) / 7);
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            palette[i + 1] = static_cast<uint8_t>(((5 - i) * a0 + i * a1) / 5);
        }
        palette[6] = 0;
        palette[7] = 255;
    }

    uint64_t indices = 0;
    for (int i = 0; i < 6; ++i) {
        indices |= static_cast<uint64_t>(block[2 + i]) << (i * 8);
    }
    for (int texel = 0; texel < 16; ++texel) {
        alphas[texel] = palette[(indices >> (texel * 3)) & 0x7];
    }
}

void WriteBlockToImage(
    const uint8_t* texels,
    uint16_t width,
    uint16_t height,
    size_t blockX,
    size_t blockY,
    std::vector<uint8_t>& rgba
) {
    for (int row = 0; row < 4; ++row) {
        const size_t y = blockY * 4 + row;
        if (y >= height) {
            break;
        }
        for (int col = 0; col < 4; ++col) {
            const size_t x = blockX * 4 + col;
            if (x >= width) {
                break;
            }
            const uint8_t* texel = texels + (row * 4 + col) * 4;
            std::copy_n(texel, 4, rgba.data() + (y * width + x) * 4);
        }
    }
}

bool DecodeDxt(
    uint8_t format,
    const uint8_t* data,
    uint16_t width,
    uint16_t height,
    std::vector<uint8_t>& rgba
) {
    const size_t blocksX = (static_cast<size_t>(width) + 3) / 4;
    const size_t blocksY = (static_cast<size_t>(height) + 3) / 4;
    const size_t blockBytes = format == kFormatDxt1 ? 8 : 16;

    for (size_t blockY = 0; blockY < blocksY; ++blockY) {
        for (size_t blockX = 0; blockX < blocksX; ++blockX) {
            const uint8_t* block = data + (blockY * blocksX + blockX) * blockBytes;
            std::array<uint8_t, 64> texels{};

            if (format == kFormatDxt1) {
                DecodeColorBlock(block, true, texels.data());
            } else if (format == kFormatDxt3) {
                DecodeColorBlock(block + 8, false, texels.data());
                for (int texel = 0; texel < 16; ++texel) {
                    const uint8_t pair = block[texel / 2];
                    const uint8_t nibble = (texel & 1) ? (pair >> 4) : (pair & 0xf);
                    texels[texel * 4 + 3] = static_cast<uint8_t>(nibble * 17);
                }
            } else {
                DecodeColorBlock(block + 8, false, texels.data());
                std::array<uint8_t, 16> alphas{};
                DecodeDxt5AlphaBlock(block, alphas.data());
                for (int texel = 0; texel < 16; ++texel) {
                    texels[texel * 4 + 3] = alphas[texel];
                }
            }

            WriteBlockToImage(texels.data(), width, height, blockX, blockY, rgba);
        }
    }
    return true;
}

void DecodeBitmap(
    uint8_t format,
    const uint8_t* data,
    uint16_t width,
    uint16_t height,
    std::vector<uint8_t>& rgba
) {
    const size_t pixels = static_cast<size_t>(width) * height;
    for (size_t i = 0; i < pixels; ++i) {
        uint8_t* out = rgba.data() + i * 4;
        switch (format) {
            case kFormatBitmapRgba:
                std::copy_n(data + i * 4, 4, out);
                break;
            case kFormatBitmapRgb:
                std::copy_n(data + i * 3, 3, out);
                out[3] = 255;
                break;
            case kFormatBitmapLumAlpha:
                out[0] = out[1] = out[2] = data[i * 2];
                out[3] = data[i * 2 + 1];
                break;
            case kFormatBitmapLum:
                out[0] = out[1] = out[2] = data[i];
                out[3] = 255;
                break;
            case kFormatBitmapAlpha:
                out[0] = out[1] = out[2] = 255;
                out[3] = data[i];
                break;
            default:
                break;
        }
    }
}
} // namespace

KisakIwiTexture DecodeIwiTexture(const std::vector<uint8_t>& fileBytes) {
    KisakIwiTexture texture;
    if (fileBytes.size() < kIwiHeaderSize) {
        texture.error = "fichier IWI trop petit";
        return texture;
    }
    if (fileBytes[0] != 'I' || fileBytes[1] != 'W' || fileBytes[2] != 'i') {
        texture.error = "magic IWi absent";
        return texture;
    }
    if (fileBytes[3] != kIwiVersionCod4) {
        texture.error = "version IWI non supportee: " + std::to_string(fileBytes[3]);
        return texture;
    }

    texture.format = fileBytes[4];
    texture.flags = fileBytes[5];
    texture.width = static_cast<uint16_t>(fileBytes[6] | (fileBytes[7] << 8));
    texture.height = static_cast<uint16_t>(fileBytes[8] | (fileBytes[9] << 8));
    texture.formatName = FormatName(texture.format);

    if (texture.flags & (kFlagCubemap | kFlagVolmap)) {
        texture.error = "cubemap/volmap non supporte";
        return texture;
    }
    if (texture.width == 0 || texture.height == 0) {
        texture.error = "dimensions IWI invalides";
        return texture;
    }

    if (IsWaveletFormat(texture.format)) {
        std::string waveletError;
        if (!DecodeIwiWaveletRgba(
                texture.format,
                fileBytes.data() + kIwiHeaderSize,
                fileBytes.size() - kIwiHeaderSize,
                texture.width,
                texture.height,
                texture.rgba,
                waveletError)) {
            texture.error = waveletError;
            return texture;
        }
        texture.valid = true;
        return texture;
    }

    const size_t mipBytes = TopMipByteCount(texture.format, texture.width, texture.height);
    if (mipBytes == 0) {
        texture.error = "format IWI non supporte: " + std::to_string(texture.format);
        return texture;
    }
    if (fileBytes.size() < kIwiHeaderSize + mipBytes) {
        texture.error = "donnees IWI tronquees";
        return texture;
    }

    // Mip levels are stored smallest first, so the full-resolution level is
    // the last mipBytes of the file (also true for textures without mips).
    const uint8_t* topMip = fileBytes.data() + fileBytes.size() - mipBytes;
    texture.rgba.assign(static_cast<size_t>(texture.width) * texture.height * 4, 0);

    if (texture.format == kFormatDxt1 || texture.format == kFormatDxt3 || texture.format == kFormatDxt5) {
        DecodeDxt(texture.format, topMip, texture.width, texture.height, texture.rgba);
    } else {
        DecodeBitmap(texture.format, topMip, texture.width, texture.height, texture.rgba);
    }

    texture.valid = true;
    return texture;
}

// Cubemap IWI (sky boxes): mips are stored smallest first like 2D files,
// with the 6 faces consecutive inside each mip level. Decodes the 6 top-mip
// faces to RGBA8.
KisakIwiCubemap DecodeIwiCubemap(const std::vector<uint8_t>& fileBytes) {
    KisakIwiCubemap cubemap;
    if (fileBytes.size() < kIwiHeaderSize
        || fileBytes[0] != 'I' || fileBytes[1] != 'W' || fileBytes[2] != 'i'
        || fileBytes[3] != kIwiVersionCod4) {
        cubemap.error = "en-tete IWI invalide";
        return cubemap;
    }
    const uint8_t format = fileBytes[4];
    const uint8_t flags = fileBytes[5];
    cubemap.width = static_cast<uint16_t>(fileBytes[6] | (fileBytes[7] << 8));
    cubemap.height = static_cast<uint16_t>(fileBytes[8] | (fileBytes[9] << 8));
    if ((flags & kFlagCubemap) == 0) {
        cubemap.error = "IWI non-cubemap";
        return cubemap;
    }
    const size_t faceBytes = TopMipByteCount(format, cubemap.width, cubemap.height);
    if (faceBytes == 0) {
        cubemap.error = "format cubemap non supporte: " + std::to_string(format);
        return cubemap;
    }
    if (fileBytes.size() < kIwiHeaderSize + faceBytes * 6) {
        cubemap.error = "cubemap tronque";
        return cubemap;
    }
    // Top mip = the last 6 faces of the file.
    const uint8_t* topMip = fileBytes.data() + fileBytes.size() - faceBytes * 6;
    for (int face = 0; face < 6; ++face) {
        cubemap.faces[face].assign(static_cast<size_t>(cubemap.width) * cubemap.height * 4, 0);
        const uint8_t* data = topMip + faceBytes * face;
        if (format == kFormatDxt1 || format == kFormatDxt3 || format == kFormatDxt5) {
            DecodeDxt(format, data, cubemap.width, cubemap.height, cubemap.faces[face]);
        } else {
            DecodeBitmap(format, data, cubemap.width, cubemap.height, cubemap.faces[face]);
        }
    }
    cubemap.valid = true;
    return cubemap;
}

// Zone-embedded GfxImageLoadDef payloads store D3D-format data with mip 0
// FIRST (the opposite of .iwi files). Decodes the top mip to RGBA8.
KisakIwiTexture DecodeD3DTopMip(
    uint32_t d3dFormat,
    uint16_t width,
    uint16_t height,
    const uint8_t* data,
    size_t dataSize
) {
    KisakIwiTexture texture;
    texture.width = width;
    texture.height = height;

    uint8_t format = 0;
    switch (d3dFormat) {
        case 0x31545844u: format = kFormatDxt1; texture.formatName = "DXT1"; break;
        case 0x33545844u: format = kFormatDxt3; texture.formatName = "DXT3"; break;
        case 0x35545844u: format = kFormatDxt5; texture.formatName = "DXT5"; break;
        case 21u: format = kFormatBitmapRgba; texture.formatName = "A8R8G8B8"; break; // BGRA memory order
        case 22u: format = kFormatBitmapRgba; texture.formatName = "X8R8G8B8"; break;
        case 50u: format = kFormatBitmapLum; texture.formatName = "L8"; break;
        case 51u: format = kFormatBitmapAlpha; texture.formatName = "A8"; break;
        default:
            texture.error = "format D3D non supporte: " + std::to_string(d3dFormat);
            return texture;
    }
    texture.format = format;

    const size_t mipBytes = TopMipByteCount(format, width, height);
    if (mipBytes == 0 || dataSize < mipBytes) {
        texture.error = "payload D3D tronque (" + std::to_string(dataSize)
            + " octets pour " + std::to_string(mipBytes) + ")";
        return texture;
    }
    texture.rgba.assign(static_cast<size_t>(width) * height * 4, 0);
    if (format == kFormatDxt1 || format == kFormatDxt3 || format == kFormatDxt5) {
        DecodeDxt(format, data, width, height, texture.rgba);
    } else {
        DecodeBitmap(format, data, width, height, texture.rgba);
        if (d3dFormat == 21u || d3dFormat == 22u) {
            // D3DFMT_[AX]8R8G8B8 memory order is BGRA; swap to RGBA.
            for (size_t i = 0; i < texture.rgba.size(); i += 4) {
                std::swap(texture.rgba[i], texture.rgba[i + 2]);
            }
            if (d3dFormat == 22u) {
                for (size_t i = 3; i < texture.rgba.size(); i += 4) {
                    texture.rgba[i] = 255;
                }
            }
        }
    }
    texture.valid = true;
    return texture;
}

std::string DescribeIwiTexture(const KisakIwiTexture& texture) {
    std::ostringstream out;
    if (!texture.valid) {
        out << "IWI invalide: " << texture.error;
        return out.str();
    }
    out << texture.width << 'x' << texture.height
        << " format=" << texture.formatName
        << " flags=0x" << std::hex << static_cast<int>(texture.flags) << std::dec
        << " rgba=" << texture.rgba.size() << " octets";
    return out.str();
}
