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

    iassert( light );
    if (light->type != 2 && light->type != 3)
        MyAssertHandler(
            ".\\qcommon\\com_bsp.cpp",
            25,
            0,
            "%s\n\t(light->type) = %i",
            "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
            light->type);
    iassert( point );
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
    iassert( IsFastFileLoad() );
    if (comWorld.isInUse)
        Sys_Error("Cannot unload world while it is in use");
}

unsigned int Com_FindClosestPrimaryLight(const float *origin)
{
    unsigned int result; // r3
    double v3; // fp0
    unsigned int v4; // r11
    float *v5; // r10
    double v6; // fp10
    double v7; // fp8
    double v8; // fp10
    double v9; // fp9
    double v10; // fp10
    double v11; // fp8
    double v12; // fp10
    double v13; // fp9
    double v14; // fp10
    double v15; // fp8
    double v16; // fp10
    double v17; // fp9
    double v18; // fp10
    double v19; // fp8
    double v20; // fp10
    float *v21; // r10
    double v22; // fp13
    double v23; // fp11
    double v24; // fp13

    iassert( comWorld.isInUse );
    result = 0;
    v3 = FLT_MAX;
    v4 = 2;
    if ((signed int)(comWorld.primaryLightCount - 2) >= 4)
    {
        v5 = &comWorld.primaryLights[2].origin[2];
        do
        {
            v6 = (float)(*(v5 - 2) - *origin);
            v7 = (float)(*(v5 - 1) - origin[1]);
            v8 = (float)((float)((float)v7 * (float)v7)
                + (float)((float)((float)v6 * (float)v6) + (float)((float)(*v5 - origin[2]) * (float)(*v5 - origin[2]))));
            if (v8 < v3)
            {
                v3 = v8;
                result = v4;
            }
            v9 = (float)(v5[17] - origin[2]);
            v10 = (float)(v5[15] - *origin);
            v11 = (float)(v5[16] - origin[1]);
            v12 = (float)((float)((float)v11 * (float)v11)
                + (float)((float)((float)v10 * (float)v10) + (float)((float)v9 * (float)v9)));
            if (v12 < v3)
            {
                v3 = v12;
                result = v4 + 1;
            }
            v13 = (float)(v5[34] - origin[2]);
            v14 = (float)(v5[32] - *origin);
            v15 = (float)(v5[33] - origin[1]);
            v16 = (float)((float)((float)v15 * (float)v15)
                + (float)((float)((float)v14 * (float)v14) + (float)((float)v13 * (float)v13)));
            if (v16 < v3)
            {
                v3 = v16;
                result = v4 + 2;
            }
            v17 = (float)(v5[51] - origin[2]);
            v18 = (float)(v5[49] - *origin);
            v19 = (float)(v5[50] - origin[1]);
            v20 = (float)((float)((float)v19 * (float)v19)
                + (float)((float)((float)v18 * (float)v18) + (float)((float)v17 * (float)v17)));
            if (v20 < v3)
            {
                v3 = v20;
                result = v4 + 3;
            }
            v4 += 4;
            v5 += 68;
        } while (v4 < comWorld.primaryLightCount - 3);
    }
    if (v4 < comWorld.primaryLightCount)
    {
        v21 = &comWorld.primaryLights[v4].origin[2];
        do
        {
            v22 = (float)(*(v21 - 2) - *origin);
            v23 = (float)(*(v21 - 1) - origin[1]);
            v24 = (float)((float)((float)v23 * (float)v23)
                + (float)((float)((float)v22 * (float)v22)
                    + (float)((float)(*v21 - origin[2]) * (float)(*v21 - origin[2]))));
            if (v24 < v3)
            {
                v3 = v24;
                result = v4;
            }
            ++v4;
            v21 += 17;
        } while (v4 < comWorld.primaryLightCount);
    }
    return result;
}