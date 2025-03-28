#include "fx_system.h"


void __cdecl FX_PostLight_GenerateVerts(FxPostLightInfo *postLightInfoAddr, FxSystem *system)
{
    int v2; // ecx
    float scale0; // [esp+0h] [ebp-F8h]
    float v4; // [esp+14h] [ebp-E4h]
    float scale1; // [esp+18h] [ebp-E0h]
    float v6; // [esp+24h] [ebp-D4h]
    float *v7; // [esp+28h] [ebp-D0h]
    float v8; // [esp+30h] [ebp-C8h]
    float v9; // [esp+34h] [ebp-C4h]
    float v10; // [esp+3Ch] [ebp-BCh]
    float v11; // [esp+40h] [ebp-B8h]
    float v12; // [esp+44h] [ebp-B4h]
    float c; // [esp+4Ch] [ebp-ACh]
    float s; // [esp+50h] [ebp-A8h]
    float unitOffset[3]; // [esp+54h] [ebp-A4h] BYREF
    int around; // [esp+60h] [ebp-98h]
    float ortho0[3]; // [esp+64h] [ebp-94h] BYREF
    float radius; // [esp+70h] [ebp-88h]
    float normalizedDelta[3]; // [esp+74h] [ebp-84h] BYREF
    float ortho1[3]; // [esp+80h] [ebp-78h] BYREF
    r_double_index_t workingIndex; // [esp+8Ch] [ebp-6Ch]
    r_double_index_t *baseIndices; // [esp+90h] [ebp-68h] BYREF
    float (*baseArgs)[4]; // [esp+94h] [ebp-64h]
    float posDeltaLenSq; // [esp+98h] [ebp-60h]
    float posDelta[3]; // [esp+9Ch] [ebp-5Ch] BYREF
    GfxPackedVertex *verts; // [esp+A8h] [ebp-50h]
    r_double_index_t *indices; // [esp+ACh] [ebp-4Ch]
    unsigned int argOffset; // [esp+B0h] [ebp-48h] BYREF
    FxPostLight *postLight; // [esp+B4h] [ebp-44h]
    unsigned __int16 baseVertex; // [esp+B8h] [ebp-40h] BYREF
    float (*args)[4]; // [esp+BCh] [ebp-3Ch]
    GfxPackedVertex *baseVerts; // [esp+C0h] [ebp-38h]
    int VERT_COUNT; // [esp+C4h] [ebp-34h]
    float lightOrigin[3]; // [esp+C8h] [ebp-30h] BYREF
    FxPostLightInfo *postLightInfo; // [esp+D4h] [ebp-24h]
    int postLightIter; // [esp+D8h] [ebp-20h]
    float eyeOffset[5]; // [esp+DCh] [ebp-1Ch] BYREF
    float POLYGON_RADIUS_GROW; // [esp+F0h] [ebp-8h]
    int POINTS_AROUND; // [esp+F4h] [ebp-4h]

    POINTS_AROUND = 8;
    POLYGON_RADIUS_GROW = 1.4142135;
    VERT_COUNT = 16;
    LODWORD(eyeOffset[4]) = 84;
    postLightInfo = postLightInfoAddr;
    eyeOffset[0] = system->camera.origin[0];
    eyeOffset[1] = system->camera.origin[1];
    eyeOffset[2] = system->camera.origin[2];
    for (postLightIter = 0; postLightIter != postLightInfo->postLightCount; ++postLightIter)
    {
        postLight = &postLightInfo->postLights[postLightIter];
        if (postLight->radius < 0.001000000047497451)
            MyAssertHandler(
                ".\\EffectsCore\\fx_postlight.cpp",
                100,
                0,
                "%s\n\t(postLight->radius) = %g",
                "(postLight->radius >= 0.001f)",
                postLight->radius);
        Vec3Sub(postLight->end, postLight->begin, posDelta);
        posDeltaLenSq = Vec3LengthSq(posDelta);
        if (posDeltaLenSq >= 0.00009999999747378752)
        {
            if (!R_ReserveCodeMeshIndices(84, &baseIndices)
                || !R_ReserveCodeMeshVerts(16, &baseVertex)
                || !R_ReserveCodeMeshArgs(2, &argOffset))
            {
                return;
            }
            baseVerts = R_GetCodeMeshVerts(baseVertex);
            baseArgs = R_GetCodeMeshArgs(argOffset);
            Vec3Sub(postLight->begin, eyeOffset, lightOrigin);
            args = baseArgs;
            v10 = lightOrigin[1];
            v11 = lightOrigin[2];
            v12 = 1.0 / postLight->radius;
            (*baseArgs)[0] = lightOrigin[0];
            (*args)[1] = v10;
            (*args)[2] = v11;
            (*args)[3] = v12;
            v7 = &(*args)[4];
            v8 = posDelta[1];
            v9 = posDelta[2];
            (*args)[4] = posDelta[0];
            v7[1] = v8;
            v7[2] = v9;
            v7[3] = 1.0 / posDeltaLenSq;
            radius = postLight->radius;
            Vec3NormalizeTo(posDelta, normalizedDelta);
            PerpendicularVector(normalizedDelta, ortho0);
            Vec3Cross(normalizedDelta, ortho0, ortho1);
            verts = baseVerts;
            for (around = 0; around != 8; ++around)
            {
                v6 = ((double)around + (double)around) * 3.141592741012573 / 8.0;
                c = cos(v6);
                s = sin(v6);
                Vec3ScaleMad(s, ortho0, c, ortho1, unitOffset);
                scale1 = radius * POLYGON_RADIUS_GROW;
                scale0 = -radius;
                Vec3MadMad(postLight->begin, scale0, normalizedDelta, scale1, unitOffset, verts->xyz);
                v4 = radius * POLYGON_RADIUS_GROW;
                Vec3MadMad(postLight->end, radius, normalizedDelta, v4, unitOffset, verts[8].xyz);
                verts->texCoord.packed = 0;
                verts->color.packed = postLight->color.packed;
                verts[8].texCoord.packed = 0;
                verts[8].color.packed = postLight->color.packed;
                ++verts;
            }
            indices = baseIndices;
            for (around = 0; around != 8; ++around)
            {
                v2 = (around + 1) % 8;
                workingIndex.value[0] = around + baseVertex;
                workingIndex.value[1] = v2 + baseVertex;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + around + 8;
                workingIndex.value[1] = workingIndex.value[0];
                *indices++ = workingIndex;
                workingIndex.value[0] = v2 + baseVertex;
                workingIndex.value[1] = v2 + baseVertex + 8;
                *indices++ = workingIndex;
            }
            for (around = 0; around != 6; around += 2)
            {
                workingIndex.value[0] = baseVertex + around + 2;
                workingIndex.value[1] = baseVertex + around + 1;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex;
                workingIndex.value[1] = baseVertex + around + 3;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + around + 2;
                workingIndex.value[1] = baseVertex;
                *indices++ = workingIndex;
            }
            for (around = 0; around != 6; around += 2)
            {
                workingIndex.value[0] = baseVertex + 8;
                workingIndex.value[1] = baseVertex + around + 9;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + around + 10;
                workingIndex.value[1] = baseVertex + 8;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + around + 10;
                workingIndex.value[1] = baseVertex + around + 11;
                *indices++ = workingIndex;
            }
            R_AddCodeMeshDrawSurf(postLight->material, baseIndices, 0x54u, argOffset, 2u, "PostLight");
        }
    }
}

void __cdecl FX_PostLight_Begin()
{
    g_postLightInfo.postLightCount = 0;
}

void __cdecl FX_PostLight_Add(FxPostLight *postLight)
{
    if (g_postLightInfo.postLightCount != 96)
        memcpy(
            &g_postLightInfo.postLights[g_postLightInfo.postLightCount++],
            postLight,
            sizeof(g_postLightInfo.postLights[g_postLightInfo.postLightCount++]));
}

FxPostLightInfo *__cdecl FX_PostLight_GetInfo()
{
    return &g_postLightInfo;
}

