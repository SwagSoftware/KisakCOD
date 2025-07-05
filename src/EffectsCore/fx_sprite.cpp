#include "fx_system.h"

#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/r_scene.h>

#include <client_mp/client_mp.h>
#include <cgame_mp/cg_local_mp.h>

FxSpriteInfo g_spriteInfo;

static FxSprite g_sprites[96]; // ADDED

void __cdecl FX_SpriteGenerateVerts(FxGenerateVertsCmd *cmd)
{
    int32_t i; // [esp+0h] [ebp-8h]
    FxSpriteInfo *spriteInfo; // [esp+4h] [ebp-4h]

    if (!cmd)
        MyAssertHandler(".\\EffectsCore\\fx_sprite.cpp", 248, 0, "%s", "cmd");
    spriteInfo = cmd->spriteInfo;
    if (!spriteInfo)
        MyAssertHandler(".\\EffectsCore\\fx_sprite.cpp", 253, 0, "%s", "spriteInfo");
    //for (i = 0; i < (int)spriteInfo->indices; ++i)
    for (i = 0; i < spriteInfo->indexCount; ++i)
        FX_GenerateSpriteCodeMeshVerts(&g_sprites[i], cmd); // LWSS: changed to `g_sprites`
}

void __cdecl FX_GenerateSpriteCodeMeshVerts(FxSprite* sprite, FxGenerateVertsCmd* cmd)
{
    if ((sprite->flags & 2) != 0)
        FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
            sprite->material,
            sprite->pos,
            sprite->radius,
            sprite->rgbaColor,
            sprite->flags,
            cmd);
    else
        FX_GenerateSpriteCodeMeshVertsFixedWorldSize(
            sprite->material,
            sprite->pos,
            sprite->radius,
            sprite->minScreenRadius,
            sprite->rgbaColor,
            sprite->flags,
            cmd);
}

void __cdecl FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
    Material *material,
    const float *pos,
    float radius,
    const uint8_t *rgbaColor,
    char spriteFlags,
    FxGenerateVertsCmd *cmd)
{
    float worldRadius; // [esp+Ch] [ebp-8h] BYREF

    if (FX_HeightScreenToWorld(pos, radius, &worldRadius, cmd))
        FX_BuildSpriteCodeMeshVerts(material, pos, worldRadius, rgbaColor, spriteFlags);
}

void __cdecl FX_BuildSpriteCodeMeshVerts(
    Material* material,
    const float* pos,
    float worldRadius,
    const uint8_t* rgbaColor,
    char spriteFlags)
{
    uint32_t v5; // [esp+10h] [ebp-30h]
    uint32_t v6; // [esp+14h] [ebp-2Ch]
    uint32_t LocalClientNum; // [esp+18h] [ebp-28h]
    float left[3]; // [esp+1Ch] [ebp-24h] BYREF
    float worldOrigin[3]; // [esp+28h] [ebp-18h] BYREF
    float up[3]; // [esp+34h] [ebp-Ch] BYREF
    cg_s *cgameGlob;

    worldOrigin[0] = *pos;
    worldOrigin[1] = pos[1];
    worldOrigin[2] = pos[2];
    if ((spriteFlags & 1) != 0)
        worldOrigin[2] = worldOrigin[2] + worldRadius;

    cgameGlob = CG_GetLocalClientGlobals(R_GetLocalClientNum());

    Vec3Scale(cgameGlob->refdef.viewaxis[1], worldRadius, left);
    Vec3Scale(cgameGlob->refdef.viewaxis[2], worldRadius, up);

    FX_BuildQuadStampCodeMeshVerts(
        material,
        cgameGlob->refdef.viewaxis[0],
        worldOrigin,
        left,
        up,
        rgbaColor,
        COERCE_INT(0.0),
        COERCE_INT(0.0),
        COERCE_INT(1.0),
        COERCE_INT(1.0));
}

void __cdecl FX_BuildQuadStampCodeMeshVerts(
    Material *material,
    const float *viewAxis,
    const float *origin,
    const float *left,
    const float *up,
    const uint8_t *rgbaColor,
    int32_t s0,
    int32_t t0,
    int32_t s1,
    int32_t t1)
{
    double v10; // st7
    __int16 v11; // [esp+10h] [ebp-1DCh]
    __int16 v12; // [esp+14h] [ebp-1D8h]
    __int16 v13; // [esp+18h] [ebp-1D4h]
    __int16 v14; // [esp+1Ch] [ebp-1D0h]
    __int16 v15; // [esp+20h] [ebp-1CCh]
    __int16 v16; // [esp+24h] [ebp-1C8h]
    __int16 v17; // [esp+28h] [ebp-1C4h]
    __int16 v18; // [esp+2Ch] [ebp-1C0h]
    int32_t v19; // [esp+7Ch] [ebp-170h]
    int32_t v20; // [esp+90h] [ebp-15Ch]
    int32_t v21; // [esp+B0h] [ebp-13Ch]
    int32_t v22; // [esp+C4h] [ebp-128h]
    int32_t v23; // [esp+E4h] [ebp-108h]
    int32_t v24; // [esp+F8h] [ebp-F4h]
    int32_t v25; // [esp+118h] [ebp-D4h]
    int32_t v26; // [esp+12Ch] [ebp-C0h]
    PackedUnitVec v27; // [esp+148h] [ebp-A4h]
    float v28; // [esp+154h] [ebp-98h]
    float v29; // [esp+158h] [ebp-94h]
    float v30; // [esp+15Ch] [ebp-90h]
    PackedUnitVec v31; // [esp+168h] [ebp-84h]
    float v32; // [esp+174h] [ebp-78h]
    float v33; // [esp+178h] [ebp-74h]
    float v34; // [esp+17Ch] [ebp-70h]
    float testBinormal[3]; // [esp+1B0h] [ebp-3Ch] BYREF
    GfxPackedVertex *verts; // [esp+1BCh] [ebp-30h]
    r_double_index_t *indices; // [esp+1C0h] [ebp-2Ch] BYREF
    GfxColor nativeColor; // [esp+1C4h] [ebp-28h] BYREF
    r_double_index_t index; // [esp+1C8h] [ebp-24h]
    float leftDown[3]; // [esp+1CCh] [ebp-20h] BYREF
    uint16_t baseVertex; // [esp+1D8h] [ebp-14h] BYREF
    float leftUp[3]; // [esp+1DCh] [ebp-10h] BYREF
    GfxPackedVertex *baseVerts; // [esp+1E8h] [ebp-4h]

    if (R_ReserveCodeMeshVerts(4, &baseVertex) && R_ReserveCodeMeshIndices(6, &indices))
    {
        Vec3Cross(viewAxis, left, testBinormal);
        if (Vec3Dot(up, testBinormal) <= 0.0)
        {
            v10 = Vec3Dot(up, testBinormal);
            MyAssertHandler(".\\EffectsCore\\fx_sprite.cpp", 61, 0, "Vec3Dot( up, testBinormal ) > 0.0f\n\t%g, %g", v10, 0.0);
        }
        index.value[0] = baseVertex;
        index.value[1] = baseVertex + 1;
        *indices = index;
        index.value[0] = baseVertex + 3;
        index.value[1] = baseVertex + 3;
        indices[1] = index;
        index.value[0] = baseVertex + 1;
        index.value[1] = baseVertex + 2;
        indices[2] = index;
        R_AddCodeMeshDrawSurf(material, indices, 6u, 0, 0, "");
        Vec3Add(left, up, leftUp);
        Vec3Sub(left, up, leftDown);
        Byte4CopyRgbaToVertexColor(rgbaColor, (uint8_t *)&nativeColor);
        v32 = -*viewAxis;
        v33 = -viewAxis[1];
        v34 = -viewAxis[2];
        v31.array[0] = (int)(v32 * 127.0 + 127.5);
        v31.array[1] = (int)(v33 * 127.0 + 127.5);
        v31.array[2] = (int)(v34 * 127.0 + 127.5);
        v31.array[3] = 63;
        v28 = -*left;
        v29 = -left[1];
        v30 = -left[2];
        v27.array[0] = (int)(v28 * 127.0 + 127.5);
        v27.array[1] = (int)(v29 * 127.0 + 127.5);
        v27.array[2] = (int)(v30 * 127.0 + 127.5);
        v27.array[3] = 63;
        baseVerts = R_GetCodeMeshVerts(baseVertex);
        verts = baseVerts;
        Vec3Add(origin, leftUp, baseVerts->xyz);
        verts->binormalSign = 1.0;
        verts->normal = v31;
        verts->color = nativeColor;
        if ((int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF)
            v26 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v26 = 0x3FFF;
        if (v26 > -16384)
            v18 = v26;
        else
            v18 = -16384;
        if ((int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF)
            v25 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v25 = 0x3FFF;
        if (v25 > -16384)
            v17 = v25;
        else
            v17 = -16384;
        verts->texCoord.packed = (v17 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v18 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts->tangent = v27;
        Vec3Sub(origin, leftDown, verts[1].xyz);
        verts[1].binormalSign = 1.0;
        verts[1].normal = v31;
        verts[1].color = nativeColor;
        if ((int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF)
            v24 = (int)((2 * s1) ^ 0x80000000) >> 14;
        else
            v24 = 0x3FFF;
        if (v24 > -16384)
            v16 = v24;
        else
            v16 = -16384;
        if ((int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF)
            v23 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v23 = 0x3FFF;
        if (v23 > -16384)
            v15 = v23;
        else
            v15 = -16384;
        verts[1].texCoord.packed = (v15 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v16 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
        verts[1].tangent = v27;
        Vec3Sub(origin, leftUp, verts[2].xyz);
        verts[2].binormalSign = 1.0;
        verts[2].normal = v31;
        verts[2].color = nativeColor;
        if ((int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF)
            v22 = (int)((2 * s1) ^ 0x80000000) >> 14;
        else
            v22 = 0x3FFF;
        if (v22 > -16384)
            v14 = v22;
        else
            v14 = -16384;
        if ((int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF)
            v21 = (int)((2 * t1) ^ 0x80000000) >> 14;
        else
            v21 = 0x3FFF;
        if (v21 > -16384)
            v13 = v21;
        else
            v13 = -16384;
        verts[2].texCoord.packed = (v13 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v14 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
        verts[2].tangent = v27;
        Vec3Add(origin, leftDown, verts[3].xyz);
        verts[3].binormalSign = 1.0;
        verts[3].normal = v31;
        verts[3].color = nativeColor;
        if ((int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF)
            v20 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v20 = 0x3FFF;
        if (v20 > -16384)
            v12 = v20;
        else
            v12 = -16384;
        if ((int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF)
            v19 = (int)((2 * t1) ^ 0x80000000) >> 14;
        else
            v19 = 0x3FFF;
        if (v19 > -16384)
            v11 = v19;
        else
            v11 = -16384;
        verts[3].texCoord.packed = (v11 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v12 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts[3].tangent = v27;
    }
}

char __cdecl FX_HeightScreenToWorld(
    const float *worldOrigin,
    float screenHeight,
    float *worldHeight,
    FxGenerateVertsCmd *cmd)
{
    uint32_t LocalClientNum; // [esp+8h] [ebp-Ch]
    float clipSpaceW; // [esp+Ch] [ebp-8h]
    float clipSpaceHeight; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(screenHeight > 0);
    iassert(cmd);

    clipSpaceW = FX_GetClipSpaceW(worldOrigin, cmd->vieworg, cmd->viewaxis);
    if (clipSpaceW < 0.000001f)
        return 0;

    cgameGlob = CG_GetLocalClientGlobals(R_GetLocalClientNum());
    clipSpaceHeight = screenHeight * clipSpaceW + screenHeight * clipSpaceW;
    *worldHeight = clipSpaceHeight * cgameGlob->refdef.tanHalfFovY;
    iassert(*worldHeight > 0.0f);

    return 1;
}

double __cdecl FX_GetClipSpaceW(const float *worldPoint, float *vieworg, float (*viewaxis)[3])
{
    float eyeDelta[3]; // [esp+0h] [ebp-Ch] BYREF

    Vec3Sub(worldPoint, vieworg, eyeDelta);
    return Vec3Dot(eyeDelta, (const float *)viewaxis);
}

void __cdecl FX_GenerateSpriteCodeMeshVertsFixedWorldSize(
    Material* material,
    const float* pos,
    float radius,
    float minScreenRadius,
    const uint8_t* rgbaColor,
    char spriteFlags,
    FxGenerateVertsCmd* cmd)
{
    float worldRadius; // [esp+Ch] [ebp-10h]
    float screenRadius; // [esp+14h] [ebp-8h] BYREF
    float screenScale; // [esp+18h] [ebp-4h]

    worldRadius = radius;
    if (minScreenRadius > 0.0)
    {
        if (!FX_HeightWorldToScreen(pos, radius, &screenRadius, cmd))
            return;
        if (screenRadius <= 0.0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_sprite.cpp",
                222,
                0,
                "%s\n\t(screenRadius) = %g",
                "(screenRadius > 0)",
                screenRadius);
        if (minScreenRadius > (double)screenRadius)
        {
            screenScale = minScreenRadius / screenRadius;
            worldRadius = radius * screenScale;
        }
    }
    FX_BuildSpriteCodeMeshVerts(material, pos, worldRadius, rgbaColor, spriteFlags);
}

char __cdecl FX_HeightWorldToScreen(
    const float *worldOrigin,
    float worldHeight,
    float *screenHeight,
    FxGenerateVertsCmd *cmd)
{
    uint32_t LocalClientNum; // [esp+8h] [ebp-Ch]
    float clipSpaceW; // [esp+Ch] [ebp-8h]
    float clipSpaceHeight; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(worldHeight > 0);
    iassert(cmd);
    
    clipSpaceW = FX_GetClipSpaceW(worldOrigin, cmd->vieworg, cmd->viewaxis);
    if (clipSpaceW < 0.000001f)
        return 0;

    cgameGlob = CG_GetLocalClientGlobals(R_GetLocalClientNum());
    clipSpaceHeight = worldHeight / cgameGlob->refdef.tanHalfFovY;
    *screenHeight = clipSpaceHeight / (clipSpaceW + clipSpaceW);

    iassert(*screenHeight > 0.0f);

    return 1;
}

void __cdecl FX_SpriteBegin()
{
    g_spriteInfo.indices = 0;
    g_spriteInfo.indexCount = 0;
}

void __cdecl FX_SpriteAdd(FxSprite *sprite)
{
    if ((int)g_spriteInfo.indexCount < arr_cnt(g_sprites))
        qmemcpy(&g_sprites[g_spriteInfo.indexCount++], sprite, sizeof(FxSprite));
        //qmemcpy(&g_sprites[(int)g_spriteInfo.indices++], sprite, sizeof(FxSprite));
}

FxSpriteInfo *__cdecl FX_SpriteGetInfo()
{
    return &g_spriteInfo;
}

