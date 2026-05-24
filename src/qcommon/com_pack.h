#pragma once

#include <universal/com_math.h>
#include <cstdint>

struct PackedTexCoords // sizeof=0x4
{                                       // ...
    PackedTexCoords()
    {
        packed = 0;
    }
    PackedTexCoords(uint32_t i)
    {
        packed = i;
    }
    uint32_t packed;
};

void __cdecl Vec2UnpackTexCoords(PackedTexCoords in, float *out);
void __cdecl Vec3UnpackUnitVec(PackedUnitVec in, float *out);
PackedUnitVec __cdecl Vec3PackUnitVec(const float *unitVec);
PackedTexCoords __cdecl Vec2PackTexCoords(const float *in);
void __cdecl Byte4PackVertexColor(const float *from, uint8_t *to);
void __cdecl Byte4PackRgba(const float *from, uint8_t *to);
void __cdecl Byte4UnpackRgba(const uint8_t *from, float *to);
void __cdecl Byte4CopyRgbaToVertexColor(const uint8_t *rgbaFrom, uint8_t *nativeTo);
void __cdecl Byte4CopyBgraToVertexColor(const uint8_t *rgbaFrom, uint8_t *nativeTo);