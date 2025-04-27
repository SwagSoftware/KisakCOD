#include "com_pack.h"
#include <universal/q_shared.h>

// KISAKTODO: Move more stuff into here. The Bgra/4byte stuff probably belongs in here.

PackedUnitVec __cdecl Vec3PackUnitVec(const float *unitVec)
{
    float v2; // [esp+0h] [ebp-8Ch]
    float v3; // [esp+4h] [ebp-88h]
    float v4; // [esp+40h] [ebp-4Ch]
    float v5; // [esp+44h] [ebp-48h]
    PackedUnitVec out; // [esp+58h] [ebp-34h]
    unsigned __int8 testEncoding[4]; // [esp+5Ch] [ebp-30h]
    float decodeScale; // [esp+60h] [ebp-2Ch]
    float encodeScale; // [esp+64h] [ebp-28h]
    float normalized[3]; // [esp+68h] [ebp-24h] BYREF
    float bestLenError; // [esp+74h] [ebp-18h]
    float bestDirError; // [esp+78h] [ebp-14h]
    float lenError; // [esp+7Ch] [ebp-10h]
    float decoded[3]; // [esp+80h] [ebp-Ch] BYREF

    Vec3NormalizeTo(unitVec, normalized);
    out.packed = 0;
    bestDirError = 3.4028235e38;
    bestLenError = 3.4028235e38;
    testEncoding[3] = 0;
    do
    {
        encodeScale = 32385.0 / (testEncoding[3] - -192.0);
        testEncoding[0] = (normalized[0] * encodeScale + 127.5);
        testEncoding[1] = (normalized[1] * encodeScale + 127.5);
        testEncoding[2] = (normalized[2] * encodeScale + 127.5);
        decodeScale = (testEncoding[3] - -192.0) / 32385.0;
        decoded[0] = (testEncoding[0] - 127.0) * decodeScale;
        decoded[1] = (testEncoding[1] - 127.0) * decodeScale;
        decoded[2] = (testEncoding[2] - 127.0) * decodeScale;
        v5 = Vec3Normalize(decoded) - 1.0;
        v3 = fabs(v5);
        lenError = v3;
        if (v3 < 0.001000000047497451)
        {
            v4 = Vec3Dot(decoded, normalized) - 1.0;
            v2 = fabs(v4);
            if (v2 < bestDirError || v2 == bestDirError && lenError < bestLenError)
            {
                bestDirError = v2;
                bestLenError = lenError;
                out.packed = *testEncoding;
                if (lenError + v2 == 0.0)
                    return out;
            }
        }
        ++testEncoding[3];
    } while (testEncoding[3]);
    if (!out.packed)
        MyAssertHandler(".\\universal\\com_pack.cpp", 163, 0, "%s", "out.packed != 0");
    return out;
}

PackedTexCoords __cdecl Vec2PackTexCoords(const float *in)
{
    __int16 v2; // [esp+0h] [ebp-3Ch]
    __int16 v3; // [esp+4h] [ebp-38h]
    int v4; // [esp+Ch] [ebp-30h]
    float v5; // [esp+18h] [ebp-24h]
    int v6; // [esp+20h] [ebp-1Ch]

    if (((uintptr_t)(2 * *in) ^ 0x80000000) >> 14 < 0x3FFF)
        v6 = ((uintptr_t)(2 * *in) ^ 0x80000000) >> 14;
    else
        v6 = 0x3FFF;
    if (v6 > -16384)
        v3 = v6;
    else
        v3 = -16384;
    v5 = in[1];
    if (((2 * LODWORD(v5)) ^ 0x80000000) >> 14 < 0x3FFF)
        v4 = ((2 * LODWORD(v5)) ^ 0x80000000) >> 14;
    else
        v4 = 0x3FFF;
    if (v4 > -16384)
        v2 = v4;
    else
        v2 = -16384;
    return ((v2 & 0x3FFF | (SLODWORD(v5) >> 16) & 0xC000) + ((v3 & 0x3FFF | (COERCE_INT(*in) >> 16) & 0xC000) << 16));
}