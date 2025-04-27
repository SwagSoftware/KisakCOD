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

// KISAKTODO: Move more stuff into here. The Bgra/4byte stuff probably belongs in here.

inline void __cdecl Byte4CopyBgraToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo)
{
    *nativeTo = (rgbaFrom[3] << 24) | *rgbaFrom | (rgbaFrom[1] << 8) | (rgbaFrom[2] << 16);
}



PackedUnitVec __cdecl Vec3PackUnitVec(const float *unitVec);
PackedTexCoords __cdecl Vec2PackTexCoords(const float *in);