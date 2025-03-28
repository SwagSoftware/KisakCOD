#include "fx_system.h"
#include <qcommon/mem_track.h>


void __cdecl TRACK_fx_random()
{
	track_static_alloc_internal((void *)fx_randomTable, 2028, "fx_randomTable", 8);
}

void __cdecl FX_RandomDir(int seed, float *dir)
{
    const char *v2; // eax
    double v3; // [esp+18h] [ebp-3Ch]
    float v4; // [esp+20h] [ebp-34h]
    float v5; // [esp+24h] [ebp-30h]
    float height; // [esp+40h] [ebp-14h]
    float yaw; // [esp+44h] [ebp-10h]
    float sinYaw; // [esp+48h] [ebp-Ch]
    float cosYaw; // [esp+4Ch] [ebp-8h]

    height = flt_8801B0[seed] + flt_8801B0[seed] - 1.0;
    if (height < -1.0 || height > 1.0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_random.cpp",
            189,
            0,
            "%s\n\t(height) = %g",
            "(height >= -1.0f && height <= 1.0f)",
            height);
    v5 = 1.0 - height * height;
    v4 = sqrt(v5);
    yaw = flt_8801AC[seed] * 6.283185482025146;
    cosYaw = cos(yaw);
    sinYaw = sin(yaw);
    *dir = v4 * cosYaw;
    dir[1] = v4 * sinYaw;
    dir[2] = height;
    if (!Vec3IsNormalized(dir))
    {
        v3 = Vec3Length(dir);
        v2 = va("(%g %g %g) len %g", *dir, dir[1], dir[2], v3);
        MyAssertHandler(".\\EffectsCore\\fx_random.cpp", 199, 0, "%s\n\t%s", "Vec3IsNormalized( dir )", v2);
    }
}

void __cdecl FX_RandomlyRotateAxis(const float (*axisIn)[3], int randomSeed, float (*axisOut)[3])
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    double v6; // [esp+18h] [ebp-28h]
    double v7; // [esp+18h] [ebp-28h]
    double v8; // [esp+18h] [ebp-28h]
    float rotation; // [esp+3Ch] [ebp-4h]

    rotation = flt_8801E8[randomSeed] * 360.0;
    (*axisOut)[0] = (*axisIn)[0];
    (*axisOut)[1] = (*axisIn)[1];
    (*axisOut)[2] = (*axisIn)[2];
    RotatePointAroundVector(&(*axisOut)[3], (const float *)axisOut, &(*axisIn)[3], rotation);
    Vec3Cross((const float *)axisOut, &(*axisOut)[3], &(*axisOut)[6]);
    if (!Vec3IsNormalized((const float *)axisOut))
    {
        v6 = Vec3Length((const float *)axisOut);
        v3 = va("(%g %g %g) len %g", (*axisOut)[0], (*axisOut)[1], (*axisOut)[2], v6);
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1772, 0, "%s\n\t%s", "Vec3IsNormalized( axisOut[0] )", v3);
    }
    if (!Vec3IsNormalized(&(*axisOut)[3]))
    {
        v7 = Vec3Length(&(*axisOut)[3]);
        v4 = va("(%g %g %g) len %g", (*axisOut)[3], (*axisOut)[4], (*axisOut)[5], v7);
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1773, 0, "%s\n\t%s", "Vec3IsNormalized( axisOut[1] )", v4);
    }
    if (!Vec3IsNormalized(&(*axisOut)[6]))
    {
        v8 = Vec3Length(&(*axisOut)[6]);
        v5 = va("(%g %g %g) len %g", (*axisOut)[6], (*axisOut)[7], (*axisOut)[8], v8);
        MyAssertHandler(".\\EffectsCore\\fx_system.cpp", 1774, 0, "%s\n\t%s", "Vec3IsNormalized( axisOut[2] )", v5);
    }
}