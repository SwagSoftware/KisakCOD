#include "kisak_iwi_wavelet_android.h"

#include <algorithm>

#define KISAK_ANDROID_WAVELET_STANDALONE
#include "../../../../../src/gfx_d3d/r_image_wavelet.cpp"

namespace {
int WaveletChannels(uint8_t format) {
    switch (format) {
        case 6: return 4;  // A8R8G8B8
        case 7: return 3;  // X8R8G8B8
        case 8: return 2;  // A8L8
        case 9: return 1;  // L8
        case 10: return 1; // A8
        default: return 0;
    }
}

int CountMipmaps(uint16_t width, uint16_t height) {
    int mipCount = 1;
    int w = width;
    int h = height;
    while (w > 1 || h > 1) {
        w = std::max(w >> 1, 1);
        h = std::max(h >> 1, 1);
        ++mipCount;
    }
    return mipCount;
}

void ConvertWaveletToRgba(uint8_t format, const uint8_t* decoded, size_t pixels, int bpp, std::vector<uint8_t>& rgba) {
    rgba.assign(pixels * 4, 0);
    for (size_t i = 0; i < pixels; ++i) {
        const uint8_t* src = decoded + i * bpp;
        uint8_t* dst = rgba.data() + i * 4;
        switch (format) {
            case 6:
                dst[0] = src[2];
                dst[1] = src[1];
                dst[2] = src[0];
                dst[3] = src[3];
                break;
            case 7:
                dst[0] = src[2];
                dst[1] = src[1];
                dst[2] = src[0];
                dst[3] = 255;
                break;
            case 8:
                dst[0] = dst[1] = dst[2] = src[0];
                dst[3] = src[1];
                break;
            case 9:
                dst[0] = dst[1] = dst[2] = src[0];
                dst[3] = 255;
                break;
            case 10:
                dst[0] = dst[1] = dst[2] = 255;
                dst[3] = src[0];
                break;
            default:
                break;
        }
    }
}
} // namespace

bool DecodeIwiWaveletRgba(
    uint8_t format,
    const uint8_t* data,
    size_t dataSize,
    uint16_t width,
    uint16_t height,
    std::vector<uint8_t>& rgba,
    std::string& error
) {
    const int channels = WaveletChannels(format);
    if (channels == 0) {
        error = "format wavelet non supporte";
        return false;
    }
    if (data == nullptr || dataSize == 0) {
        error = "donnees wavelet absentes";
        return false;
    }

    const int bpp = channels == 3 ? 4 : channels;
    const size_t pixelCount = static_cast<size_t>(width) * height;
    const size_t totalSize = pixelCount * static_cast<size_t>(bpp);
    if (pixelCount == 0 || totalSize == 0) {
        error = "dimensions wavelet invalides";
        return false;
    }

    std::vector<uint8_t> stream(data, data + dataSize);
    stream.resize(stream.size() + 4, 0);
    std::vector<uint8_t> decoded(totalSize, 0);

    WaveletDecode decode{};
    decode.width = width;
    decode.height = height;
    decode.mipLevel = CountMipmaps(width, height) - 1;
    decode.channels = channels;
    decode.bpp = bpp;
    decode.data = stream.data();

    uint8_t* previous = nullptr;
    while (decode.mipLevel >= 0) {
        const int levelWidth = std::max(width >> decode.mipLevel, 1);
        const int levelHeight = std::max(height >> decode.mipLevel, 1);
        const size_t levelSize = static_cast<size_t>(bpp) * levelWidth * levelHeight;
        if (levelSize > decoded.size()) {
            error = "niveau wavelet invalide";
            return false;
        }
        uint8_t* current = decoded.data() + decoded.size() - levelSize;
        Wavelet_DecompressLevel(previous, current, &decode);
        previous = current;
        --decode.mipLevel;
    }

    ConvertWaveletToRgba(format, decoded.data(), pixelCount, bpp, rgba);
    return true;
}
