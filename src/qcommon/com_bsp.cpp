#include "com_bsp.h"
#include <win32/win_local.h>

ComWorld comWorld;

char __cdecl Com_CanPrimaryLightAffectPoint(const ComPrimaryLight *light, const float *point)
{
    float v3; // [esp+Ch] [ebp-24h]
    float deltaToLight[3]; // [esp+18h] [ebp-18h] BYREF
    float cosHalfFov; // [esp+24h] [ebp-Ch]
    float spotDotTimesDist; // [esp+28h] [ebp-8h]
    float distSq; // [esp+2Ch] [ebp-4h]

    if (!light)
        MyAssertHandler(".\\qcommon\\com_bsp.cpp", 24, 0, "%s", "light");
    if (light->type != 2 && light->type != 3)
        MyAssertHandler(
            ".\\qcommon\\com_bsp.cpp",
            25,
            0,
            "%s\n\t(light->type) = %i",
            "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
            light->type);
    if (!point)
        MyAssertHandler(".\\qcommon\\com_bsp.cpp", 26, 0, "%s", "point");
    Vec3Sub(light->origin, point, deltaToLight);
    distSq = Vec3LengthSq(deltaToLight);
    v3 = light->radius * light->radius;
    if (distSq >= (double)v3)
        return 0;
    if (light->type == 3 || light->rotationLimit <= -light->cosHalfFovOuter)
        return 1;
    spotDotTimesDist = Vec3Dot(deltaToLight, light->dir);
    if (light->rotationLimit == 1.0)
    {
        cosHalfFov = light->cosHalfFovOuter;
    }
    else
    {
        cosHalfFov = CosOfSumOfArcCos(light->cosHalfFovOuter, light->rotationLimit);
        if (cosHalfFov <= 0.0)
            return spotDotTimesDist <= cosHalfFov * light->radius;
    }
    return spotDotTimesDist > 0.0 && cosHalfFov * cosHalfFov * distSq <= spotDotTimesDist * spotDotTimesDist;
}

double __cdecl CosOfSumOfArcCos(float cos0, float cos1)
{
    float v4; // [esp+4h] [ebp-18h]
    float v5; // [esp+10h] [ebp-Ch]
    float sinSq1; // [esp+14h] [ebp-8h]
    float sinSq0; // [esp+18h] [ebp-4h]

    sinSq0 = 1.0 - cos0 * cos0;
    sinSq1 = 1.0 - cos1 * cos1;
    v5 = sinSq1 * sinSq0;
    v4 = sqrt(v5);
    return (float)(cos0 * cos1 - v4);
}

void __cdecl Com_UnloadWorld()
{
    if (!useFastFile->current.enabled)
        MyAssertHandler(".\\qcommon\\com_bsp.cpp", 86, 0, "%s", "IsFastFileLoad()");
    if (comWorld.isInUse)
        Sys_Error("Cannot unload world while it is in use");
}

