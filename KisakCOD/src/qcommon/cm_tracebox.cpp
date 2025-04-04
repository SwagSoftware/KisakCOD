#include "qcommon.h"



void __cdecl CM_CalcTraceExtents(TraceExtents *extents)
{
    double v1; // [esp+0h] [ebp-10h]
    float diff; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        diff = extents->start[i] - extents->end[i];
        if (diff == 0.0)
            v1 = 0.0;
        else
            v1 = 1.0 / diff;
        extents->invDelta[i] = v1;
    }
}

int __cdecl CM_TraceBox(const TraceExtents *extents, float *mins, float *maxs, float fraction)
{
    float v5; // [esp+0h] [ebp-3Ch]
    float v6; // [esp+4h] [ebp-38h]
    float v7; // [esp+8h] [ebp-34h]
    float v8; // [esp+Ch] [ebp-30h]
    int t; // [esp+1Ch] [ebp-20h]
    float enterFrac; // [esp+20h] [ebp-1Ch]
    float dist1; // [esp+24h] [ebp-18h]
    float frac; // [esp+28h] [ebp-14h]
    float fraca; // [esp+28h] [ebp-14h]
    float sign; // [esp+2Ch] [ebp-10h]
    float dist2; // [esp+34h] [ebp-8h]

    enterFrac = 0.0;
    sign = -1.0;
    while (1)
    {
        if ((COERCE_UNSIGNED_INT(*mins) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(mins[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(mins[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\qcommon\\cm_tracebox.cpp",
                46,
                0,
                "%s",
                "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])");
        }
        for (t = 0; t < 3; ++t)
        {
            dist1 = (extents->start[t] - mins[t]) * sign;
            dist2 = (extents->end[t] - mins[t]) * sign;
            if (dist1 <= 0.0)
            {
                if (dist2 > 0.0)
                {
                    fraca = dist1 * extents->invDelta[t] * sign;
                    if (fraca <= (double)enterFrac)
                        return 1;
                    v6 = fraca - fraction;
                    if (v6 < 0.0)
                        v5 = dist1 * extents->invDelta[t] * sign;
                    else
                        v5 = fraction;
                    fraction = v5;
                }
            }
            else
            {
                if (dist2 > 0.0)
                    return 1;
                frac = dist1 * extents->invDelta[t] * sign;
                if (fraction <= (double)frac)
                    return 1;
                v8 = enterFrac - frac;
                if (v8 < 0.0)
                    v7 = dist1 * extents->invDelta[t] * sign;
                else
                    v7 = enterFrac;
                enterFrac = v7;
            }
        }
        if (sign == 1.0)
            break;
        sign = 1.0;
        mins = maxs;
    }
    return 0;
}

bool __cdecl CM_TraceSphere(const TraceExtents *extents, const float *origin, float radius, float fraction)
{
    double v4; // st7
    float scale; // [esp+0h] [ebp-5Ch]
    float v7; // [esp+18h] [ebp-44h]
    float v8; // [esp+1Ch] [ebp-40h]
    float v9; // [esp+20h] [ebp-3Ch]
    float v10; // [esp+24h] [ebp-38h]
    float v11; // [esp+28h] [ebp-34h]
    float delta[3]; // [esp+2Ch] [ebp-30h] BYREF
    float sphereFraction; // [esp+38h] [ebp-24h]
    float startOffset[3]; // [esp+3Ch] [ebp-20h] BYREF
    float offset[4]; // [esp+48h] [ebp-14h] BYREF
    float lengthSq; // [esp+58h] [ebp-4h]

    Vec3Sub(origin, extents->start, startOffset);
    Vec3Sub(extents->end, extents->start, delta);
    lengthSq = Vec3LengthSq(delta);
    if (-lengthSq < 0.0)
        v10 = lengthSq;
    else
        v10 = 1.0;
    lengthSq = v10;
    if (v10 == 0.0)
        MyAssertHandler(".\\qcommon\\cm_tracebox.cpp", 100, 0, "%s", "lengthSq");
    v4 = Vec3Dot(startOffset, delta);
    sphereFraction = v4 / lengthSq;
    v9 = sphereFraction - fraction;
    if (v9 < 0.0)
        v11 = sphereFraction;
    else
        v11 = fraction;
    v8 = 0.0 - sphereFraction;
    if (v8 < 0.0)
        v7 = v11;
    else
        v7 = 0.0;
    offset[3] = v7;
    scale = -v7;
    Vec3Mad(startOffset, scale, delta, offset);
    return Vec3LengthSq(offset) >= radius * radius;
}

