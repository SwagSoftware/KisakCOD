#pragma once

#include <universal/com_math.h>

struct PackedTexCoords // sizeof=0x4
{                                       // ...
    PackedTexCoords()
    {
        packed = 0;
    }
    PackedTexCoords(unsigned int i)
    {
        packed = i;
    }
    unsigned int packed;
};


PackedUnitVec __cdecl Vec3PackUnitVec(const float *unitVec);
PackedTexCoords __cdecl Vec2PackTexCoords(const float *in);
void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to);
void __cdecl Byte4PackRgba(const float *from, unsigned __int8 *to);
void __cdecl Byte4UnpackRgba(const unsigned __int8 *from, float *to);
void __cdecl Byte4CopyRgbaToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo);
void __cdecl Byte4CopyBgraToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo);