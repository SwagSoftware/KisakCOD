#include "fx_system.h"
#include <aim_assist/aim_assist.h>


void __cdecl FX_EvaluateVisAlpha(FxElemPreVisualState *preVisState, FxElemVisualState *visState)
{
    float valueLerpInv; // [esp+30h] [ebp-8h]
    float valueLerp; // [esp+34h] [ebp-4h]

    valueLerp = flt_8801E4[preVisState->randomSeed];
    valueLerpInv = 1.0 - valueLerp;
    visState->color[3] = FX_InterpolateColor(
        preVisState->refState,
        valueLerp,
        valueLerpInv,
        preVisState->sampleLerp,
        preVisState->sampleLerpInv,
        3);
}

unsigned __int8 __cdecl FX_InterpolateColor(
    const FxElemVisStateSample *refState,
    float valueLerp,
    float valueLerpInv,
    float sampleLerp,
    float sampleLerpInv,
    int channel)
{
    float v7; // [esp+14h] [ebp-18h]
    float valueFrom; // [esp+24h] [ebp-8h]
    float valueTo; // [esp+28h] [ebp-4h]

    valueFrom = (double)refState->base.color[channel] * valueLerpInv
        + (double)refState->amplitude.color[channel] * valueLerp;
    valueTo = (double)refState[1].base.color[channel] * valueLerpInv
        + (double)refState[1].amplitude.color[channel] * valueLerp;
    v7 = sampleLerp * valueTo + sampleLerpInv * valueFrom;
    return (int)(v7 + 9.313225746154785e-10);
}

void __cdecl FX_SetupVisualState(
    const FxElemDef *elemDef,
    const FxEffect *effect,
    int randomSeed,
    float normTimeUpdateEnd,
    FxElemPreVisualState *preVisState)
{
    float v5; // [esp+8h] [ebp-10h]
    float samplePoint; // [esp+10h] [ebp-8h]

    samplePoint = (double)elemDef->visStateIntervalCount * normTimeUpdateEnd;
    v5 = floor(samplePoint);
    preVisState->sampleLerp = samplePoint - (double)(int)v5;
    preVisState->sampleLerpInv = 1.0 - preVisState->sampleLerp;
    preVisState->elemDef = elemDef;
    preVisState->effect = effect;
    preVisState->refState = &elemDef->visSamples[v5];
    preVisState->randomSeed = randomSeed;
    preVisState->distanceFade = 255;
}

void __cdecl FX_EvaluateSize(FxElemPreVisualState *preVisState, FxElemVisualState *visState)
{
    visState->size[0] = FX_InterpolateSize(
        preVisState->refState,
        preVisState->randomSeed,
        FXRAND_SIZE_0,
        preVisState->sampleLerp,
        preVisState->sampleLerpInv,
        0);
    if ((preVisState->elemDef->flags & 0x10000000) != 0)
        visState->size[1] = FX_InterpolateSize(
            preVisState->refState,
            preVisState->randomSeed,
            FXRAND_SIZE_1,
            preVisState->sampleLerp,
            preVisState->sampleLerpInv,
            1);
    else
        visState->size[1] = visState->size[0];
}

double __cdecl FX_InterpolateSize(
    const FxElemVisStateSample *refState,
    int randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float sampleLerpInv,
    int channel)
{
    float valueFrom; // [esp+4h] [ebp-Ch]
    float valueTo; // [esp+8h] [ebp-8h]
    float valueLerp; // [esp+Ch] [ebp-4h]

    valueLerp = fx_randomTable[randomKey + randomSeed];
    valueFrom = valueLerp * refState->amplitude.size[channel] + refState->base.size[channel];
    valueTo = valueLerp * refState[1].amplitude.size[channel] + refState[1].base.size[channel];
    return (float)(valueFrom * sampleLerpInv + valueTo * sampleLerp);
}

void __cdecl FX_EvaluateVisualState(FxElemPreVisualState *preVisState, float msecLifeSpan, FxElemVisualState *visState)
{
    float valueLerpInv; // [esp+B4h] [ebp-20h]
    const FxElemDef *elemDef; // [esp+B8h] [ebp-1Ch]
    int randomSeed; // [esp+BCh] [ebp-18h]
    float valueLerp; // [esp+C0h] [ebp-14h]
    float sampleLerpInv; // [esp+C4h] [ebp-10h]
    const FxElemVisStateSample *refState; // [esp+C8h] [ebp-Ch]
    float sampleLerp; // [esp+CCh] [ebp-8h]

    elemDef = preVisState->elemDef;
    if (!elemDef)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 182, 0, "%s", "elemDef");
    refState = preVisState->refState;
    randomSeed = preVisState->randomSeed;
    valueLerp = flt_8801E4[randomSeed];
    valueLerpInv = 1.0 - valueLerp;
    sampleLerp = preVisState->sampleLerp;
    sampleLerpInv = preVisState->sampleLerpInv;
    visState->color[0] = FX_InterpolateColor(refState, valueLerp, valueLerpInv, sampleLerp, sampleLerpInv, 0);
    visState->color[1] = FX_InterpolateColor(refState, valueLerp, valueLerpInv, sampleLerp, sampleLerpInv, 1);
    visState->color[2] = FX_InterpolateColor(refState, valueLerp, valueLerpInv, sampleLerp, sampleLerpInv, 2);
    visState->color[3] = FX_InterpolateColor(refState, valueLerp, valueLerpInv, sampleLerp, sampleLerpInv, 3);
    if (elemDef->lightingFrac)
        FX_EvaluateVisualState_DoLighting(preVisState, visState, elemDef);
    visState->rotationTotal = elemDef->initialRotation.amplitude * flt_8801E8[randomSeed] + elemDef->initialRotation.base;
    visState->rotationTotal = FX_IntegrateRotationFromZero(
        refState,
        randomSeed,
        FXRAND_ROTATION_DELTA,
        sampleLerp,
        msecLifeSpan)
        + visState->rotationTotal;
    visState->color[3] = (unsigned __int16)(LOWORD(preVisState->distanceFade) * visState->color[3]) >> 8;
}

double __cdecl FX_IntegrateRotationFromZero(
    const FxElemVisStateSample *refState,
    int randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float msecLifeSpan)
{
    float valueLerp; // [esp+4h] [ebp-10h]
    float rotationTotal; // [esp+8h] [ebp-Ch]
    float rotationTotala; // [esp+8h] [ebp-Ch]
    float rotationTotalb; // [esp+8h] [ebp-Ch]
    float weight; // [esp+Ch] [ebp-8h]
    float weight_4; // [esp+10h] [ebp-4h]

    valueLerp = fx_randomTable[randomKey + randomSeed];
    weight_4 = sampleLerp * sampleLerp * 0.5;
    weight = sampleLerp - weight_4;
    rotationTotal = valueLerp * refState->amplitude.rotationTotal + refState->base.rotationTotal;
    rotationTotala = (valueLerp * refState->amplitude.rotationDelta + refState->base.rotationDelta) * weight
        + rotationTotal;
    rotationTotalb = (valueLerp * refState[1].amplitude.rotationDelta + refState[1].base.rotationDelta) * weight_4
        + rotationTotala;
    return (float)(rotationTotalb * msecLifeSpan);
}

void __cdecl FX_EvaluateVisualState_DoLighting(
    FxElemPreVisualState *preVisState,
    FxElemVisualState *visState,
    const FxElemDef *elemDef)
{
    unsigned __int8 v3; // [esp+0h] [ebp-1Ch]
    int v4; // [esp+4h] [ebp-18h]
    unsigned int lightFactor; // [esp+Ch] [ebp-10h]
    unsigned int colorIndex; // [esp+10h] [ebp-Ch]
    unsigned int outColorIndex; // [esp+14h] [ebp-8h]
    unsigned __int8 lightColor[4]; // [esp+18h] [ebp-4h] BYREF

    if (!preVisState->effect)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 156, 0, "%s", "preVisState->effect");
    FX_UnpackColor565(preVisState->effect->packedLighting, lightColor, &lightColor[1], &lightColor[2]);
    for (colorIndex = 0; colorIndex != 3; ++colorIndex)
    {
        outColorIndex = mapping[colorIndex];
        lightFactor = (2 * lightColor[colorIndex] - 255) * elemDef->lightingFrac / 255 + 255;
        if (lightFactor > 0x1FE)
            MyAssertHandler(
                ".\\EffectsCore\\fx_draw.cpp",
                163,
                0,
                "%s\n\t(lightFactor) = %i",
                "(lightFactor >= 0 && lightFactor <= 510)",
                lightFactor);
        if ((int)(lightFactor * visState->color[outColorIndex]) / 255 < 255)
            v4 = (int)(lightFactor * visState->color[outColorIndex]) / 255;
        else
            v4 = 255;
        if (v4 > 0)
            v3 = v4;
        else
            v3 = 0;
        visState->color[outColorIndex] = v3;
    }
}

void __cdecl FX_UnpackColor565(
    unsigned __int16 packed,
    unsigned __int8 *outR,
    unsigned __int8 *outG,
    unsigned __int8 *outB)
{
    *outR = 8 * packed;
    *outR |= (int)*outR >> 5;
    *outG = ((int)packed >> 3) & 0xF8;
    *outG |= (int)*outG >> 5;
    *outB = HIBYTE(packed) & 0xF8;
    *outB |= (int)*outB >> 5;
}

void __cdecl FX_DrawElem_BillboardSprite(FxDrawState *draw)
{
    float *origin; // ecx
    float *v2; // [esp+4h] [ebp-2Ch]
    float *v3; // [esp+8h] [ebp-28h]
    float normal[3]; // [esp+Ch] [ebp-24h] BYREF
    float tangent[3]; // [esp+18h] [ebp-18h] BYREF
    float binormal[3]; // [esp+24h] [ebp-Ch] BYREF

    if (!FX_CullElementForDraw_Sprite(draw))
    {
        v3 = draw->camera->axis[0];
        normal[0] = -*v3;
        normal[1] = -v3[1];
        normal[2] = -v3[2];
        v2 = draw->camera->axis[1];
        tangent[0] = -*v2;
        tangent[1] = -v2[1];
        tangent[2] = -v2[2];
        origin = draw->camera->origin;
        binormal[0] = origin[34];
        binormal[1] = origin[35];
        binormal[2] = origin[36];
        FX_GenSpriteVerts(draw, tangent, binormal, normal);
    }
}

void __cdecl FX_GenSpriteVerts(FxDrawState *draw, const float *tangent, const float *binormal, const float *normal)
{
    float scale1; // [esp+8h] [ebp-274h]
    __int16 v5; // [esp+14h] [ebp-268h]
    __int16 v6; // [esp+18h] [ebp-264h]
    __int16 v7; // [esp+1Ch] [ebp-260h]
    __int16 v8; // [esp+20h] [ebp-25Ch]
    __int16 v9; // [esp+24h] [ebp-258h]
    __int16 v10; // [esp+28h] [ebp-254h]
    __int16 v11; // [esp+2Ch] [ebp-250h]
    __int16 v12; // [esp+30h] [ebp-24Ch]
    float v13; // [esp+7Ch] [ebp-200h]
    int v14; // [esp+80h] [ebp-1FCh]
    float v15; // [esp+90h] [ebp-1ECh]
    int v16; // [esp+94h] [ebp-1E8h]
    int v17; // [esp+B4h] [ebp-1C8h]
    float v18; // [esp+C4h] [ebp-1B8h]
    int v19; // [esp+C8h] [ebp-1B4h]
    int v20; // [esp+E8h] [ebp-194h]
    int v21; // [esp+FCh] [ebp-180h]
    float v22; // [esp+118h] [ebp-164h]
    int v23; // [esp+11Ch] [ebp-160h]
    int v24; // [esp+130h] [ebp-14Ch]
    PackedUnitVec v25; // [esp+14Ch] [ebp-130h]
    PackedUnitVec v26; // [esp+16Ch] [ebp-110h]
    float rotationTotal; // [esp+1A8h] [ebp-D4h]
    int t0; // [esp+1E4h] [ebp-98h] BYREF
    r_double_index_t *baseIndices; // [esp+1E8h] [ebp-94h] BYREF
    float dt; // [esp+1ECh] [ebp-90h] BYREF
    FxElemVisuals visuals; // [esp+1F0h] [ebp-8Ch]
    float leftSide[3]; // [esp+1F4h] [ebp-88h] BYREF
    PackedUnitVec packedNormal; // [esp+200h] [ebp-7Ch]
    PackedUnitVec packedTangent; // [esp+204h] [ebp-78h]
    float rightSide[3]; // [esp+208h] [ebp-74h] BYREF
    FxSpriteInfo *sprite; // [esp+214h] [ebp-68h]
    float testBinormal[3]; // [esp+218h] [ebp-64h] BYREF
    float cosRot; // [esp+224h] [ebp-58h]
    float left[3]; // [esp+228h] [ebp-54h] BYREF
    float v40; // [esp+234h] [ebp-48h] BYREF
    GfxPackedVertex *verts; // [esp+238h] [ebp-44h]
    float sinRot; // [esp+23Ch] [ebp-40h]
    r_double_index_t *indices; // [esp+240h] [ebp-3Ch]
    r_double_index_t index; // [esp+244h] [ebp-38h]
    float rotatedTangent[3]; // [esp+248h] [ebp-34h] BYREF
    float up[3]; // [esp+254h] [ebp-28h] BYREF
    const FxSystem *system; // [esp+260h] [ebp-1Ch]
    int s0; // [esp+264h] [ebp-18h] BYREF
    unsigned __int16 baseVertex; // [esp+268h] [ebp-14h] BYREF
    float rotatedBinormal[3]; // [esp+26Ch] [ebp-10h] BYREF
    GfxPackedVertex *baseVerts; // [esp+278h] [ebp-4h]

    system = draw->system;
    sprite = &system->sprite;
    visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
    if (!visuals.anonymous)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 298, 0, "%s", "visuals.material");
    if (sprite->material != visuals.anonymous && sprite->indexCount)
    {
        if (!sprite->name)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 304, 0, "%s", "sprite->name");
        if (!sprite->material)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 305, 0, "%s", "sprite->material");
        if (!sprite->indices)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 306, 0, "%s", "sprite->indices");
        R_AddCodeMeshDrawSurf(sprite->material, sprite->indices, sprite->indexCount, 0, 0, sprite->name);
        sprite->indexCount = 0;
    }
    if (R_ReserveCodeMeshVerts(4, &baseVertex) && R_ReserveCodeMeshIndices(6, &baseIndices))
    {
        if (sprite->material != visuals.anonymous)
        {
            sprite->name = draw->effect->def->name;
            sprite->material = visuals.material;
            sprite->indices = baseIndices;
        }
        sprite->indexCount += 6;
        //Profile_Begin(198);
        FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
        //Profile_EndInternal(0);
        rotationTotal = draw->visState.rotationTotal;
        cosRot = cos(rotationTotal);
        sinRot = sin(rotationTotal);
        Vec3ScaleMad(cosRot, tangent, sinRot, binormal, rotatedTangent);
        scale1 = -cosRot;
        Vec3ScaleMad(sinRot, tangent, scale1, binormal, rotatedBinormal);
        Vec3Scale(rotatedTangent, draw->visState.size[0], left);
        Vec3Scale(rotatedBinormal, draw->visState.size[1], up);
        Vec3Sub(draw->posWorld, left, leftSide);
        Vec3Add(draw->posWorld, left, rightSide);
        FX_GetSpriteTexCoords(draw, (float *)&s0, &v40, (float *)&t0, &dt);
        v26.array[0] = (int)(*normal * 127.0 + 127.5);
        v26.array[1] = (int)(normal[1] * 127.0 + 127.5);
        v26.array[2] = (int)(normal[2] * 127.0 + 127.5);
        v26.array[3] = 63;
        packedNormal.packed = v26.packed;
        v25.array[0] = (int)(rotatedTangent[0] * 127.0 + 127.5);
        v25.array[1] = (int)(rotatedTangent[1] * 127.0 + 127.5);
        v25.array[2] = (int)(rotatedTangent[2] * 127.0 + 127.5);
        v25.array[3] = 63;
        packedTangent.packed = v25.packed;
        indices = baseIndices;
        index.value[0] = baseVertex;
        index.value[1] = baseVertex + 1;
        *baseIndices = index;
        ++indices;
        index.value[0] = baseVertex + 2;
        index.value[1] = baseVertex + 2;
        *indices++ = index;
        index.value[0] = baseVertex + 3;
        index.value[1] = baseVertex;
        *indices++ = index;
        Vec3Cross(normal, rotatedTangent, testBinormal);
        if (Vec3Dot(testBinormal, rotatedBinormal) > 0.0)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 361, 0, "%s", "Vec3Dot( testBinormal, rotatedBinormal ) <= 0.0f");
        baseVerts = R_GetCodeMeshVerts(baseVertex);
        verts = baseVerts;
        Vec3Add(leftSide, up, baseVerts->xyz);
        verts->binormalSign = -1.0;
        verts->normal = packedNormal;
        verts->color.packed = *(unsigned int *)draw->visState.color;
        if ((int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF)
            v24 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v24 = 0x3FFF;
        if (v24 > -16384)
            v12 = v24;
        else
            v12 = -16384;
        v22 = dt + *(float *)&t0;
        if ((int)((2 * LODWORD(v22)) ^ 0x80000000) >> 14 < 0x3FFF)
            v23 = (int)((2 * LODWORD(v22)) ^ 0x80000000) >> 14;
        else
            v23 = 0x3FFF;
        if (v23 > -16384)
            v11 = v23;
        else
            v11 = -16384;
        verts->texCoord.packed = (v11 & 0x3FFF | (SLODWORD(v22) >> 16) & 0xC000)
            + ((v12 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts->tangent = packedTangent;
        ++verts;
        Vec3Sub(leftSide, up, verts->xyz);
        verts->binormalSign = -1.0;
        verts->normal = packedNormal;
        verts->color.packed = *(unsigned int *)draw->visState.color;
        if ((int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF)
            v21 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v21 = 0x3FFF;
        if (v21 > -16384)
            v10 = v21;
        else
            v10 = -16384;
        if ((int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF)
            v20 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v20 = 0x3FFF;
        if (v20 > -16384)
            v9 = v20;
        else
            v9 = -16384;
        verts->texCoord.packed = (v9 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v10 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts->tangent = packedTangent;
        ++verts;
        Vec3Sub(rightSide, up, verts->xyz);
        verts->binormalSign = -1.0;
        verts->normal = packedNormal;
        verts->color.packed = *(unsigned int *)draw->visState.color;
        v18 = v40 + *(float *)&s0;
        if ((int)((2 * LODWORD(v18)) ^ 0x80000000) >> 14 < 0x3FFF)
            v19 = (int)((2 * LODWORD(v18)) ^ 0x80000000) >> 14;
        else
            v19 = 0x3FFF;
        if (v19 > -16384)
            v8 = v19;
        else
            v8 = -16384;
        if ((int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF)
            v17 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v17 = 0x3FFF;
        if (v17 > -16384)
            v7 = v17;
        else
            v7 = -16384;
        verts->texCoord.packed = (v7 & 0x3FFF | (t0 >> 16) & 0xC000)
            + ((v8 & 0x3FFF | (SLODWORD(v18) >> 16) & 0xC000) << 16);
        verts->tangent = packedTangent;
        ++verts;
        Vec3Add(rightSide, up, verts->xyz);
        verts->binormalSign = -1.0;
        verts->normal = packedNormal;
        verts->color.packed = *(unsigned int *)draw->visState.color;
        v15 = v40 + *(float *)&s0;
        if ((int)((2 * LODWORD(v15)) ^ 0x80000000) >> 14 < 0x3FFF)
            v16 = (int)((2 * LODWORD(v15)) ^ 0x80000000) >> 14;
        else
            v16 = 0x3FFF;
        if (v16 > -16384)
            v6 = v16;
        else
            v6 = -16384;
        v13 = dt + *(float *)&t0;
        if ((int)((2 * LODWORD(v13)) ^ 0x80000000) >> 14 < 0x3FFF)
            v14 = (int)((2 * LODWORD(v13)) ^ 0x80000000) >> 14;
        else
            v14 = 0x3FFF;
        if (v14 > -16384)
            v5 = v14;
        else
            v5 = -16384;
        verts->texCoord.packed = (v5 & 0x3FFF | (SLODWORD(v13) >> 16) & 0xC000)
            + ((v6 & 0x3FFF | (SLODWORD(v15) >> 16) & 0xC000) << 16);
        verts->tangent = packedTangent;
    }
}

void __cdecl FX_GetSpriteTexCoords(const FxDrawState *draw, float *s0, float *ds, float *t0, float *dt)
{
    unsigned __int8 colBits; // [esp+14h] [ebp-14h]
    int atlasIndex; // [esp+18h] [ebp-10h]
    int atlasIndexa; // [esp+18h] [ebp-10h]
    const FxElemDef *elemDef; // [esp+1Ch] [ebp-Ch]
    int atlasCount; // [esp+20h] [ebp-8h]
    unsigned __int8 rowBits; // [esp+24h] [ebp-4h]

    if (!draw)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 215, 0, "%s", "draw");
    elemDef = draw->elemDef;
    if (!elemDef)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 217, 0, "%s", "elemDef");
    atlasCount = elemDef->atlas.entryCount;
    if (atlasCount == 1)
    {
        *s0 = 0.0;
        *ds = 1.0;
        *t0 = 0.0;
        *dt = 1.0;
    }
    else
    {
        if (atlasCount <= 1 || atlasCount > 256 || (atlasCount & (atlasCount - 1)) != 0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_draw.cpp",
                228,
                0,
                "%s\n\t(atlasCount) = %i",
                "(atlasCount > 1 && atlasCount <= 256 && (((atlasCount) & ((atlasCount) - 1)) == 0))",
                atlasCount);
        if ((elemDef->atlas.behavior & 3) != 0)
        {
            if ((elemDef->atlas.behavior & 3) == 1)
            {
                atlasIndex = (atlasCount * LOWORD(fx_randomTable[draw->randomSeed + 22])) >> 16;
            }
            else
            {
                if ((elemDef->atlas.behavior & 3) != 2)
                    MyAssertHandler(
                        ".\\EffectsCore\\fx_draw.cpp",
                        241,
                        1,
                        "elemDef->atlas.behavior & FX_ATLAS_START_MASK == FX_ATLAS_START_INDEXED\n\t%i, %i",
                        elemDef->atlas.behavior & 3,
                        2);
                atlasIndex = (atlasCount - 1) & draw->elem->sequence;
            }
        }
        else
        {
            atlasIndex = elemDef->atlas.index;
        }
        if ((elemDef->atlas.behavior & 4) != 0)
        {
            atlasIndex += (int)((double)atlasCount * draw->normTimeUpdateEnd);
        }
        else if (elemDef->atlas.fps)
        {
            atlasIndex += elemDef->atlas.fps * (int)draw->msecElapsed / 1000;
        }
        if ((elemDef->atlas.behavior & 8) != 0 && atlasIndex >= atlasCount * elemDef->atlas.loopCount)
            atlasIndex = atlasCount - 1;
        atlasIndexa = atlasIndex & (atlasCount - 1);
        rowBits = elemDef->atlas.rowIndexBits;
        colBits = elemDef->atlas.colIndexBits;
        *ds = 1.0 / (double)(1 << colBits);
        *s0 = (double)(atlasIndexa & ((1 << colBits) - 1)) * *ds;
        *dt = 1.0 / (double)(1 << rowBits);
        *t0 = (double)(atlasIndexa >> colBits) * *dt;
    }
}

bool __cdecl FX_CullElementForDraw_Sprite(const FxDrawState *draw)
{
    bool result; // al
    float v2; // [esp+4h] [ebp-18h]
    float v3; // [esp+8h] [ebp-14h]
    float v4; // [esp+Ch] [ebp-10h]
    float v5; // [esp+10h] [ebp-Ch]
    unsigned int frustumPlaneCount; // [esp+18h] [ebp-4h]

    result = 0;
    if (fx_cull_elem_draw->current.enabled)
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        v4 = draw->visState.size[0];
        v5 = draw->visState.size[1];
        v3 = v4 - v5;
        v2 = v3 < 0.0 ? v5 : v4;
        if (FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, v2))
            return 1;
    }
    return result;
}

unsigned int __cdecl FX_CullElementForDraw_FrustumPlaneCount(const FxDrawState *draw)
{
    if (!draw || !draw->camera || draw->camera->frustumPlaneCount < 5)
        MyAssertHandler(
            ".\\EffectsCore\\fx_draw.cpp",
            537,
            0,
            "%s",
            "draw && draw->camera && draw->camera->frustumPlaneCount >= 5");
    if ((draw->elemDef->flags & 0x400) != 0)
        return 5;
    else
        return draw->camera->frustumPlaneCount;
}

void __cdecl FX_DrawElem_OrientedSprite(FxDrawState *draw)
{
    if (!FX_CullElementForDraw_Sprite(draw))
        FX_GenSpriteVerts(draw, draw->orient.axis[1], draw->orient.axis[2], draw->orient.axis[0]);
}

void __cdecl FX_DrawElem_Tail(FxDrawState *draw)
{
    float scale; // [esp+8h] [ebp-48h]
    float normal[3]; // [esp+2Ch] [ebp-24h] BYREF
    float deltaCamera[3]; // [esp+38h] [ebp-18h] BYREF
    float tangent[3]; // [esp+44h] [ebp-Ch] BYREF

    FX_GetVelocityAtTime(
        draw->elemDef,
        draw->randomSeed,
        draw->msecLifeSpan,
        draw->msecElapsed,
        &draw->orient,
        draw->elem->baseVel,
        draw->velDirWorld);
    Vec3Normalize(draw->velDirWorld);
    if (!FX_CullElementForDraw_Tail(draw))
    {
        scale = -draw->visState.size[1];
        Vec3Mad(draw->posWorld, scale, draw->velDirWorld, draw->posWorld);
        Vec3Sub(draw->camera->origin, draw->posWorld, deltaCamera);
        Vec3Cross(draw->velDirWorld, deltaCamera, tangent);
        Vec3Normalize(tangent);
        Vec3Cross(tangent, draw->velDirWorld, normal);
        FX_GenSpriteVerts(draw, tangent, draw->velDirWorld, normal);
    }
}

bool __cdecl FX_CullElementForDraw_Tail(const FxDrawState *draw)
{
    bool result; // al
    float scale; // [esp+Ch] [ebp-14h]
    float endpoint[3]; // [esp+10h] [ebp-10h] BYREF
    unsigned int frustumPlaneCount; // [esp+1Ch] [ebp-4h]

    result = 0;
    if (fx_cull_elem_draw->current.enabled)
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        scale = draw->visState.size[1] * -2.0;
        Vec3Mad(draw->posWorld, scale, draw->velDirWorld, endpoint);
        if (FX_CullCylinder(draw->camera, frustumPlaneCount, draw->posWorld, endpoint, draw->visState.size[0]))
            return 1;
    }
    return result;
}

char __cdecl FX_CullCylinder(
    const FxCamera *camera,
    unsigned int frustumPlaneCount,
    const float *posWorld0,
    const float *posWorld1,
    float radius)
{
    const char *v5; // eax
    const char *v6; // eax
    double v8; // [esp+18h] [ebp-18h]
    float pointToPlaneDist; // [esp+28h] [ebp-8h]
    float pointToPlaneDista; // [esp+28h] [ebp-8h]
    unsigned int planeIndex; // [esp+2Ch] [ebp-4h]

    if (!camera->isValid)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 620, 0, "%s", "camera->isValid");
    if (frustumPlaneCount != camera->frustumPlaneCount && frustumPlaneCount != 5)
    {
        v5 = va("%i, %i", frustumPlaneCount, camera->frustumPlaneCount);
        MyAssertHandler(
            ".\\EffectsCore\\fx_draw.cpp",
            621,
            0,
            "%s\n\t%s",
            "frustumPlaneCount == camera->frustumPlaneCount || frustumPlaneCount == 5",
            v5);
    }
    for (planeIndex = 0; planeIndex < frustumPlaneCount; ++planeIndex)
    {
        if (!Vec3IsNormalized(camera->frustum[planeIndex]))
        {
            v8 = Vec3Length(camera->frustum[planeIndex]);
            v6 = va(
                "(%g %g %g) len %g",
                camera->frustum[planeIndex][0],
                camera->frustum[planeIndex][1],
                camera->frustum[planeIndex][2],
                v8);
            MyAssertHandler(
                ".\\EffectsCore\\fx_draw.cpp",
                625,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( camera->frustum[planeIndex] )",
                v6);
        }
        pointToPlaneDist = Vec3Dot(camera->frustum[planeIndex], posWorld0) - camera->frustum[planeIndex][3];
        if (pointToPlaneDist <= -radius)
        {
            pointToPlaneDista = Vec3Dot(camera->frustum[planeIndex], posWorld1) - camera->frustum[planeIndex][3];
            if (pointToPlaneDista <= -radius)
                return 1;
        }
    }
    return 0;
}

void __cdecl FX_DrawElem_Cloud(FxDrawState *draw)
{
    GfxParticleCloud *cloud; // [esp+50h] [ebp-Ch]
    FxElemVisuals visuals; // [esp+54h] [ebp-8h]

    if (fx_drawClouds->current.enabled)
    {
        FX_GetVelocityAtTime(
            draw->elemDef,
            draw->randomSeed,
            draw->msecLifeSpan,
            draw->msecElapsed,
            &draw->orient,
            draw->elem->baseVel,
            draw->velDirWorld);
        Vec3Normalize(draw->velDirWorld);
        draw->visState.scale = FX_InterpolateScale(
            draw->preVisState.refState,
            draw->preVisState.randomSeed,
            FXRAND_SCALE,
            draw->preVisState.sampleLerp,
            draw->preVisState.sampleLerpInv);
        if (draw->visState.scale != 0.0 && !FX_CullElementForDraw_Cloud(draw))
        {
            InterlockedIncrement(&draw->system->gfxCloudCount);
            visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
            if (!visuals.anonymous)
                MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 792, 0, "%s", "visuals.material");
            cloud = R_AddParticleCloudToScene(visuals.material);
            if (cloud)
            {
                //Profile_Begin(198);
                FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
                //Profile_EndInternal(0);
                FX_SetPlacement(draw, &cloud->placement);
                cloud->color.packed = *(unsigned int *)draw->visState.color;
                *(double *)cloud->radius = *(double *)draw->visState.size;
                Vec3Sub(draw->posWorld, draw->velDirWorld, cloud->endpos);
            }
        }
    }
}

void __cdecl FX_SetPlacement(const FxDrawState *draw, GfxScaledPlacement *placement)
{
    float axis[3][3]; // [esp+14h] [ebp-28h] BYREF
    float msecForAxis; // [esp+38h] [ebp-4h]

    msecForAxis = FX_GetMsecForSamplingAxis(draw->msecElapsed, draw->msecLifeSpan, draw->elem->atRestFraction);
    FX_GetElemAxis(draw->elemDef, draw->randomSeed, &draw->orient, msecForAxis, axis);
    AxisToQuat(axis, placement->base.quat);
    placement->base.origin[0] = draw->posWorld[0];
    placement->base.origin[1] = draw->posWorld[1];
    placement->base.origin[2] = draw->posWorld[2];
    placement->scale = draw->visState.scale;
}

double __cdecl FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int atRestFraction)
{
    float msecAtRest; // [esp+8h] [ebp-8h]
    float msecSinceAtRest; // [esp+Ch] [ebp-4h]

    msecAtRest = (double)atRestFraction * msecLifeSpan * 0.003921568859368563;
    msecSinceAtRest = msecElapsed - msecAtRest;
    if (msecSinceAtRest <= 0.0)
        return msecElapsed;
    if (msecSinceAtRest < 300.0)
        return (float)(msecElapsed - msecSinceAtRest * msecSinceAtRest * 0.001666666707023978);
    else
        return (float)(msecAtRest + 150.0);
}

double __cdecl FX_InterpolateScale(
    const FxElemVisStateSample *refState,
    int randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float sampleLerpInv)
{
    float valueFrom; // [esp+4h] [ebp-Ch]
    float valueTo; // [esp+8h] [ebp-8h]
    float valueLerp; // [esp+Ch] [ebp-4h]

    valueLerp = fx_randomTable[randomKey + randomSeed];
    valueFrom = valueLerp * refState->amplitude.scale + refState->base.scale;
    valueTo = valueLerp * refState[1].amplitude.scale + refState[1].base.scale;
    return (float)(valueFrom * sampleLerpInv + valueTo * sampleLerp);
}

bool __cdecl FX_CullElementForDraw_Cloud(const FxDrawState *draw)
{
    bool result; // al
    float v2; // [esp+4h] [ebp-18h]
    float v3; // [esp+8h] [ebp-14h]
    float v4; // [esp+Ch] [ebp-10h]
    float v5; // [esp+10h] [ebp-Ch]
    float radius; // [esp+14h] [ebp-8h]
    unsigned int frustumPlaneCount; // [esp+18h] [ebp-4h]

    result = 0;
    if (fx_cull_elem_draw->current.enabled)
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        v4 = draw->visState.size[0];
        v5 = draw->visState.size[1];
        v3 = v4 - v5;
        v2 = v3 < 0.0 ? v5 : v4;
        radius = draw->visState.scale + v2;
        if (FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, radius))
            return 1;
    }
    return result;
}

void __cdecl FX_DrawElem_Model(FxDrawState *draw)
{
    FxElemVisuals visuals; // [esp+20h] [ebp-24h]
    GfxScaledPlacement placement; // [esp+24h] [ebp-20h] BYREF

    draw->visState.scale = FX_InterpolateScale(
        draw->preVisState.refState,
        draw->preVisState.randomSeed,
        FXRAND_SCALE,
        draw->preVisState.sampleLerp,
        draw->preVisState.sampleLerpInv);
    if (draw->visState.scale != 0.0)
    {
        FX_SetPlacement(draw, &placement);
        if (draw->elemDef->elemType != 5)
            MyAssertHandler(
                ".\\EffectsCore\\fx_draw.cpp",
                844,
                0,
                "%s\n\t(draw->elemDef->elemType) = %i",
                "(draw->elemDef->elemType == FX_ELEM_TYPE_MODEL)",
                draw->elemDef->elemType);
        if (((unsigned int)svs.snapshotClients[20582].attachModelIndex & draw->elemDef->flags) != 0)
            FX_SetPlacementFromPhysics(draw, &placement.base);
        visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
        if (!visuals.anonymous)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 851, 0, "%s", "visuals.model");
        R_FilterXModelIntoScene(visuals.model, &placement, 0, (unsigned __int16 *)&draw->elem->u);
    }
}

void __cdecl FX_SetPlacementFromPhysics(const FxDrawState *draw, GfxPlacement *placement)
{
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Phys_ObjGetInterpolatedState(PHYS_WORLD_FX, (dxBody *)draw->elem->physObjId, placement->origin, placement->quat);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl FX_DrawElem_Light(FxDrawState *draw)
{
    float green; // [esp+4Ch] [ebp-Ch]
    float blue; // [esp+50h] [ebp-8h]
    float red; // [esp+54h] [ebp-4h]

    if (!FX_CullElementForDraw_Light(draw))
    {
        //Profile_Begin(198);
        FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
        //Profile_EndInternal(0);
        red = (double)draw->visState.color[2] * 0.003921568859368563;
        green = (double)draw->visState.color[1] * 0.003921568859368563;
        blue = (double)draw->visState.color[0] * 0.003921568859368563;
        R_AddOmniLightToScene(draw->posWorld, draw->visState.size[0], red, green, blue);
    }
}

bool __cdecl FX_CullElementForDraw_Light(const FxDrawState *draw)
{
    bool result; // al
    unsigned int frustumPlaneCount; // [esp+4h] [ebp-4h]

    result = 0;
    if (fx_cull_elem_draw->current.enabled)
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        if (FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, draw->visState.size[0]))
            return 1;
    }
    return result;
}

void __cdecl FX_DrawElem_SpotLight(FxDrawState *draw)
{
    float green; // [esp+4Ch] [ebp-Ch]
    float blue; // [esp+50h] [ebp-8h]
    float red; // [esp+54h] [ebp-4h]

    if (!FX_CullElementForDraw_Light(draw))
    {
        //Profile_Begin(198);
        FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
        //Profile_EndInternal(0);
        red = (double)draw->visState.color[2] * 0.003921568859368563;
        green = (double)draw->visState.color[1] * 0.003921568859368563;
        blue = (double)draw->visState.color[0] * 0.003921568859368563;
        R_AddSpotLightToScene(draw->posWorld, draw->orient.axis[0], draw->visState.size[0], red, green, blue);
    }
}

void __cdecl FX_DrawNonSpriteElems(FxSystem *system)
{
    FxEffect *effect; // [esp+3Ch] [ebp-8h]
    volatile int activeIndex; // [esp+40h] [ebp-4h]

    //Profile_Begin(201);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1370, 0, "%s", "system");
    if (!system->camera.isValid)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1371, 0, "%s", "system->camera.isValid");
    FX_BeginIteratingOverEffects_Cooperative(system);
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
    {
        effect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
        FX_DrawNonSpriteEffect(system, effect, 1u, system->msecDraw);
    }
    if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
        FX_RunGarbageCollection(system);
    //Profile_EndInternal(0);
}

void __cdecl FX_BeginIteratingOverEffects_Cooperative(FxSystem *system)
{
    volatile int iteratorCount; // [esp+0h] [ebp-Ch]

    if (system->isArchiving)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 479, 0, "%s", "!system->isArchiving");
    do
    {
        if (system->iteratorCount < 0)
            iteratorCount = 0;
        else
            iteratorCount = system->iteratorCount;
    } while (InterlockedCompareExchange(&system->iteratorCount, iteratorCount + 1, iteratorCount) != iteratorCount);
}

void __cdecl FX_DrawNonSpriteEffect(FxSystem *system, FxEffect *effect, unsigned int elemClass, int drawTime)
{
    unsigned __int16 elemHandle; // [esp+0h] [ebp-BCh]
    FxDrawState drawState; // [esp+4h] [ebp-B8h] BYREF
    const FxElemDef *elemDef; // [esp+B0h] [ebp-Ch]
    const FxElemDef *elemDefs; // [esp+B4h] [ebp-8h]
    FxElem *elem; // [esp+B8h] [ebp-4h]

    drawState.effect = effect;
    drawState.msecDraw = drawTime;
    elemHandle = effect->firstElemHandle[elemClass];
    if (elemHandle != 0xFFFF)
    {
        drawState.system = system;
        elemDefs = drawState.effect->def->elemDefs;
        while (elemHandle != 0xFFFF)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elem = &FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle)->item;
            elemDef = &elemDefs[elem->defIndex];
            if (elemDef->elemType <= 3u)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_draw.cpp",
                    1355,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType > FX_ELEM_TYPE_LAST_SPRITE)",
                    elemDef->elemType);
            FX_DrawElement(system, elemDef, elem, &drawState);
            elemHandle = elem->nextElemHandleInEffect;
        }
    }
}

void __cdecl FX_DrawElement(FxSystem *system, const FxElemDef *elemDef, const FxElem *elem, FxDrawState *draw)
{
    if (elemDef->elemType >= 8u)
        MyAssertHandler(
            ".\\EffectsCore\\fx_draw.cpp",
            1010,
            0,
            "elemDef->elemType doesn't index FX_ELEM_TYPE_LAST_DRAWN + 1\n\t%i not in [0, %i)",
            elemDef->elemType,
            8);
    if (elemDef->visualCount && elem->msecBegin <= draw->msecDraw)
    {
        draw->elem = elem;
        draw->elemDef = elemDef;
        FX_DrawElement_Setup_1_(system, draw, elem->msecBegin, elem->sequence, elem->origin, 0);
        if (!s_drawElemHandler[elemDef->elemType])
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1025, 0, "%s", "s_drawElemHandler[elemDef->elemType]");
        s_drawElemHandler[elemDef->elemType](draw);
    }
}

void __cdecl FX_DrawSpotLight(FxSystem *system)
{
    FxEffect *v1; // eax
    volatile int msecDraw; // [esp-4h] [ebp-Ch]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1389, 0, "%s", "system");
    if (!system->camera.isValid)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1390, 0, "%s", "system->camera.isValid");
    FX_BeginIteratingOverEffects_Cooperative(system);
    if (system->activeSpotLightElemCount > 0)
    {
        if (system->activeSpotLightEffectCount != 1)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1395, 0, "%s", "system->activeSpotLightEffectCount == 1");
        if (system->activeSpotLightElemCount != 1)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1396, 0, "%s", "system->activeSpotLightElemCount == 1");
        msecDraw = system->msecDraw;
        v1 = FX_EffectFromHandle(system, system->activeSpotLightEffectHandle);
        FX_DrawSpotLightEffect(system, v1, msecDraw);
    }
    if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
        FX_RunGarbageCollection(system);
}

void __cdecl FX_DrawSpotLightEffect(FxSystem *system, FxEffect *effect, int drawTime)
{
    unsigned __int16 activeSpotLightElemHandle; // [esp+2h] [ebp-BAh]
    FxDrawState drawState; // [esp+4h] [ebp-B8h] BYREF
    const FxElemDef *elemDef; // [esp+B0h] [ebp-Ch]
    const FxElemDef *elemDefs; // [esp+B4h] [ebp-8h]
    FxElem *elem; // [esp+B8h] [ebp-4h]

    if (system->activeSpotLightEffectCount <= 0)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1320, 0, "%s", "system->activeSpotLightEffectCount > 0");
    if (system->activeSpotLightElemCount <= 0)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1321, 0, "%s", "system->activeSpotLightElemCount > 0");
    drawState.effect = effect;
    drawState.system = system;
    drawState.msecDraw = drawTime;
    elemDefs = effect->def->elemDefs;
    activeSpotLightElemHandle = system->activeSpotLightElemHandle;
    if (!system)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
    elem = (FxElem *)FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, activeSpotLightElemHandle);
    elemDef = &elemDefs[elem->defIndex];
    if (elemDef->elemType != 7)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1329, 0, "%s", "elemDef->elemType == FX_ELEM_TYPE_SPOT_LIGHT");
    FX_DrawElement(system, elemDef, elem, &drawState);
}

void __cdecl FX_DrawSpriteElems(FxSystem *system, int drawTime)
{
    int numTrailEffects; // [esp+38h] [ebp-820h]
    unsigned __int16 effectHandle; // [esp+3Ch] [ebp-81Ch]
    FxEffect *effect; // [esp+40h] [ebp-818h]
    FxEffect *effecta; // [esp+40h] [ebp-818h]
    FxSpriteInfo *sprite; // [esp+44h] [ebp-814h]
    unsigned __int16 trailEffects[1026]; // [esp+48h] [ebp-810h]
    int i; // [esp+850h] [ebp-8h]
    int activeIndex; // [esp+854h] [ebp-4h]

    //Profile_Begin(201);
    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1510, 0, "%s", "system");
    if (!system->camera.isValid)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1511, 0, "%s", "system->camera.isValid");
    system->gfxCloudCount = 0;
    sprite = &system->sprite;
    system->sprite.indices = 0;
    system->sprite.indexCount = 0;
    system->sprite.name = 0;
    system->sprite.material = 0;
    numTrailEffects = 0;
    FX_BeginIteratingOverEffects_Cooperative(system);
    for (activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
    {
        effectHandle = system->allEffectHandles[activeIndex & 0x3FF];
        effect = FX_EffectFromHandle(system, effectHandle);
        FX_DrawSpriteEffect(system, effect, drawTime);
        FX_DrawNonSpriteEffect(system, effect, 2u, drawTime);
        if (effect->firstTrailHandle != 0xFFFF)
            trailEffects[numTrailEffects++] = effectHandle;
    }
    if (numTrailEffects > 0)
    {
        for (i = 0; i < numTrailEffects; ++i)
        {
            effecta = FX_EffectFromHandle(system, trailEffects[i]);
            FX_DrawTrailsForEffect(system, effecta, drawTime);
        }
    }
    if (!InterlockedDecrement(&system->iteratorCount) && system->needsGarbageCollection)
        FX_RunGarbageCollection(system);
    if (system->sprite.indexCount)
    {
        if (!system->sprite.name)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1555, 0, "%s", "sprite->name");
        if (!system->sprite.material)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1556, 0, "%s", "sprite->material");
        if (!sprite->indices)
            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1557, 0, "%s", "sprite->indices");
        R_AddCodeMeshDrawSurf(
            system->sprite.material,
            system->sprite.indices,
            system->sprite.indexCount,
            0,
            0,
            system->sprite.name);
        system->sprite.indexCount = 0;
        sprite->indices = 0;
    }
    //Profile_EndInternal(0);
}

void __cdecl FX_DrawTrailsForEffect(FxSystem *system, FxEffect *effect, int drawTime)
{
    FxDrawState drawState; // [esp+0h] [ebp-B8h] BYREF
    unsigned __int16 trailHandle; // [esp+B0h] [ebp-8h]
    FxTrail *trail; // [esp+B4h] [ebp-4h]

    drawState.system = system;
    drawState.effect = effect;
    drawState.msecDraw = drawTime;
    for (trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->nextTrailHandle)
    {
        if (!system)
            MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 362, 0, "%s", "system");
        trail = (FxTrail *)FX_PoolFromHandle_Generic<FxTrail, 128>(system->trails, trailHandle);
        FX_DrawTrail(system, &drawState, trail);
    }
}

void __cdecl FX_DrawTrail(FxSystem *system, FxDrawState *draw, FxTrail *trail)
{
    double v3; // st7
    const char *v4; // eax
    float v5; // [esp+24h] [ebp-194h]
    float v6; // [esp+98h] [ebp-120h]
    FxTrailSegmentDrawState tailSegmentDrawState; // [esp+A0h] [ebp-118h] BYREF
    float alpha; // [esp+DCh] [ebp-DCh]
    r_double_index_t *reservedIndices; // [esp+E0h] [ebp-D8h] BYREF
    int trailDefIndCount; // [esp+E4h] [ebp-D4h]
    FxTrailSegmentDrawState lastSegmentDrawState; // [esp+E8h] [ebp-D0h] BYREF
    FxElemVisuals visuals; // [esp+124h] [ebp-94h]
    unsigned __int16 vertsPerSegment; // [esp+128h] [ebp-90h]
    int upperBoundSegmentCount; // [esp+12Ch] [ebp-8Ch]
    FxTrailSegmentDrawState segmentDrawState; // [esp+130h] [ebp-88h] BYREF
    int exactSegmentCount; // [esp+170h] [ebp-48h]
    FxSpriteInfo *sprite; // [esp+174h] [ebp-44h]
    unsigned __int16 reservedBaseVertex; // [esp+178h] [ebp-40h] BYREF
    GfxPackedVertex *reservedVerts; // [esp+17Ch] [ebp-3Ch]
    int indicesToReserve; // [esp+180h] [ebp-38h]
    const FxTrailElem *trailElem; // [esp+184h] [ebp-34h]
    float uCoordOffset; // [esp+188h] [ebp-30h]
    float basis[2][3]; // [esp+18Ch] [ebp-2Ch] BYREF
    float lastSegmentNormTime; // [esp+1A4h] [ebp-14h]
    int trailDefVertCount; // [esp+1A8h] [ebp-10h]
    int curSegment; // [esp+1ACh] [ebp-Ch]
    unsigned __int16 trailElemHandle; // [esp+1B0h] [ebp-8h]
    float segmentNormTime; // [esp+1B4h] [ebp-4h] BYREF

    sprite = &system->sprite;
    draw->elemDef = &draw->effect->def->elemDefs[trail->defIndex];
    if (draw->elemDef->visualCount)
    {
        trailElemHandle = trail->firstElemHandle;
        if (trailElemHandle != 0xFFFF)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 348, 0, "%s", "system");
            trailElem = (const FxTrailElem *)FX_PoolFromHandle_Generic<FxTrailElem, 2048>(system->trailElems, trailElemHandle);
            v6 = trailElem->spawnDist / (double)draw->elemDef->trailDef->repeatDist;
            v5 = floor(v6);
            uCoordOffset = -v5;
            if (draw->elemDef->trailDef->scrollTimeMsec)
            {
                if (draw->elemDef->trailDef->scrollTimeMsec <= 0)
                    v3 = 1.0
                    - (double)(draw->msecDraw % draw->elemDef->trailDef->scrollTimeMsec)
                    / (double)draw->elemDef->trailDef->scrollTimeMsec
                    + uCoordOffset;
                else
                    v3 = (double)(draw->msecDraw % -draw->elemDef->trailDef->scrollTimeMsec)
                    / (double)-draw->elemDef->trailDef->scrollTimeMsec
                    + uCoordOffset;
                uCoordOffset = v3;
            }
            upperBoundSegmentCount = 0;
            for (trailElemHandle = trail->firstElemHandle;
                trailElemHandle != 0xFFFF;
                trailElemHandle = trailElem->nextTrailElemHandle)
            {
                if (!system)
                    MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 348, 0, "%s", "system");
                trailElem = (const FxTrailElem *)FX_PoolFromHandle_Generic<FxTrailElem, 2048>(
                    system->trailElems,
                    trailElemHandle);
                if (trailElem->msecBegin <= draw->msecDraw)
                    ++upperBoundSegmentCount;
            }
            trailDefVertCount = draw->elemDef->trailDef->vertCount;
            trailDefIndCount = draw->elemDef->trailDef->indCount;
            if (R_ReserveCodeMeshVerts(trailDefVertCount * upperBoundSegmentCount, &reservedBaseVertex))
            {
                reservedVerts = R_GetCodeMeshVerts(reservedBaseVertex);
                exactSegmentCount = 0;
                lastSegmentNormTime = 1.0;
                memset((unsigned __int8 *)&lastSegmentDrawState, 0, sizeof(lastSegmentDrawState));
                for (trailElemHandle = trail->firstElemHandle;
                    trailElemHandle != 0xFFFF;
                    trailElemHandle = trailElem->nextTrailElemHandle)
                {
                    if (!system)
                        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 348, 0, "%s", "system");
                    trailElem = (const FxTrailElem *)FX_PoolFromHandle_Generic<FxTrailElem, 2048>(
                        system->trailElems,
                        trailElemHandle);
                    if (trailElem->msecBegin <= draw->msecDraw)
                    {
                        FX_DrawElement_Setup_1_(
                            system,
                            draw,
                            trailElem->msecBegin,
                            trailElem->sequence,
                            trailElem->origin,
                            &segmentNormTime);
                        FX_TrailElem_UncompressBasis(trailElem->basis, basis);
                        //Profile_Begin(198);
                        FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
                        //Profile_EndInternal(0);
                        Fx_GenTrail_PopulateSegmentDrawState(draw, trailElem->spawnDist, uCoordOffset, basis, &segmentDrawState);
                        if (segmentNormTime < 1.0)
                        {
                            if (trailElemHandle == trail->firstElemHandle)
                            {
                                if (!trailElem->sequence)
                                    segmentDrawState.color[3] = 0;
                            }
                            else if (lastSegmentNormTime >= 1.0)
                            {
                                memcpy(&tailSegmentDrawState, &lastSegmentDrawState, sizeof(tailSegmentDrawState));
                                alpha = (1.0 - lastSegmentNormTime) / (segmentNormTime - lastSegmentNormTime);
                                tailSegmentDrawState.uCoord = (segmentDrawState.uCoord - lastSegmentDrawState.uCoord) * alpha
                                    + lastSegmentDrawState.uCoord;
                                Vec3Lerp(lastSegmentDrawState.posWorld, segmentDrawState.posWorld, alpha, tailSegmentDrawState.posWorld);
                                Vec3Lerp(lastSegmentDrawState.basis[0], segmentDrawState.basis[0], alpha, tailSegmentDrawState.basis[0]);
                                Vec3Lerp(lastSegmentDrawState.basis[1], segmentDrawState.basis[1], alpha, tailSegmentDrawState.basis[1]);
                                FX_GenTrail_VertsForSegment(
                                    &tailSegmentDrawState,
                                    &reservedVerts[trailDefVertCount * exactSegmentCount++]);
                            }
                            FX_GenTrail_VertsForSegment(&segmentDrawState, &reservedVerts[trailDefVertCount * exactSegmentCount++]);
                        }
                        lastSegmentNormTime = segmentNormTime;
                        memcpy(&lastSegmentDrawState, &segmentDrawState, sizeof(lastSegmentDrawState));
                    }
                }
                if (exactSegmentCount > upperBoundSegmentCount)
                {
                    v4 = va(
                        "Too optimistic: exactSegmentCount = %i, upperBoundSegmentCount = %i",
                        exactSegmentCount,
                        upperBoundSegmentCount);
                    MyAssertHandler(
                        ".\\EffectsCore\\fx_draw.cpp",
                        1157,
                        0,
                        "%s\n\t%s",
                        "exactSegmentCount <= upperBoundSegmentCount",
                        v4);
                }
                if (exactSegmentCount > 1)
                {
                    indicesToReserve = 3 * trailDefIndCount;
                    vertsPerSegment = draw->elemDef->trailDef->vertCount;
                    visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
                    if (sprite->material != visuals.anonymous && sprite->indexCount)
                    {
                        if (!sprite->name)
                            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1169, 0, "%s", "sprite->name");
                        if (!sprite->material)
                            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1170, 0, "%s", "sprite->material");
                        if (!sprite->indices)
                            MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1171, 0, "%s", "sprite->indices");
                        R_AddCodeMeshDrawSurf(sprite->material, sprite->indices, sprite->indexCount, 0, 0, sprite->name);
                        sprite->indexCount = 0;
                    }
                    curSegment = 0;
                    while (curSegment < exactSegmentCount - 1 && R_ReserveCodeMeshIndices(indicesToReserve, &reservedIndices))
                    {
                        if (sprite->material != visuals.anonymous)
                        {
                            sprite->name = draw->effect->def->name;
                            sprite->material = visuals.material;
                            sprite->indices = reservedIndices;
                        }
                        sprite->indexCount += indicesToReserve;
                        FX_GenTrail_IndsForSegment(draw, reservedBaseVertex, reservedIndices);
                        ++curSegment;
                        reservedBaseVertex += vertsPerSegment;
                    }
                }
            }
        }
    }
}

void __cdecl FX_TrailElem_UncompressBasis(const char (*inBasis)[3], float (*basis)[3])
{
    int basisVecIter; // [esp+4h] [ebp-8h]
    int dimIter; // [esp+8h] [ebp-4h]

    for (basisVecIter = 0; basisVecIter != 2; ++basisVecIter)
    {
        for (dimIter = 0; dimIter != 3; ++dimIter)
            (*basis)[3 * basisVecIter + dimIter] = (double)(*inBasis)[3 * basisVecIter + dimIter] * 0.007874015718698502;
    }
}

void __cdecl FX_GenTrail_IndsForSegment(
    FxDrawState *draw,
    unsigned __int16 reservedBaseVertex,
    r_double_index_t *outIndices)
{
    unsigned __int16 quadInds_2; // [esp+2h] [ebp-2Eh]
    r_double_index_t quadInds_4; // [esp+4h] [ebp-2Ch]
    r_double_index_t index; // [esp+Ch] [ebp-24h]
    int indCount; // [esp+10h] [ebp-20h]
    FxTrailDef *trailDef; // [esp+18h] [ebp-18h]
    unsigned __int16 farBase; // [esp+20h] [ebp-10h]
    unsigned __int16 *inds; // [esp+24h] [ebp-Ch]
    int indPairIter; // [esp+28h] [ebp-8h]
    r_double_index_t *outIndicesa; // [esp+40h] [ebp+10h]

    trailDef = draw->elemDef->trailDef;
    if (!trailDef)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 414, 0, "%s", "trailDef");
    inds = trailDef->inds;
    indCount = trailDef->indCount;
    if (2 * (indCount / 2) != indCount)
        MyAssertHandler(
            ".\\EffectsCore\\fx_draw.cpp",
            419,
            0,
            "%s\n\t(trailDef->indCount) = %i",
            "(indPairCount * 2 == indCount)",
            trailDef->indCount);
    farBase = LOWORD(trailDef->vertCount) + reservedBaseVertex;
    for (indPairIter = 0; indPairIter != indCount / 2; ++indPairIter)
    {
        quadInds_2 = inds[2 * indPairIter + 1] + reservedBaseVertex;
        quadInds_4.value[0] = inds[2 * indPairIter] + farBase;
        quadInds_4.value[1] = inds[2 * indPairIter + 1] + farBase;
        index.value[0] = inds[2 * indPairIter] + reservedBaseVertex;
        index.value[1] = quadInds_2;
        *outIndices = index;
        outIndicesa = outIndices + 1;
        *outIndicesa = quadInds_4;
        index.value[0] = quadInds_4.value[0];
        index.value[1] = quadInds_2;
        outIndicesa[1] = index;
        outIndices = outIndicesa + 2;
    }
}

void __cdecl Fx_GenTrail_PopulateSegmentDrawState(
    FxDrawState *draw,
    float spawnDist,
    float uCoordOffset,
    const float (*basis)[3],
    FxTrailSegmentDrawState *outState)
{
    outState->trailDef = draw->elemDef->trailDef;
    outState->posWorld[0] = draw->posWorld[0];
    outState->posWorld[1] = draw->posWorld[1];
    outState->posWorld[2] = draw->posWorld[2];
    outState->basis[0][0] = (*basis)[0];
    outState->basis[0][1] = (*basis)[1];
    outState->basis[0][2] = (*basis)[2];
    outState->basis[1][0] = (*basis)[3];
    outState->basis[1][1] = (*basis)[4];
    outState->basis[1][2] = (*basis)[5];
    outState->rotation = draw->visState.rotationTotal;
    *(double *)outState->size = *(double *)draw->visState.size;
    *(unsigned int *)outState->color = *(unsigned int *)draw->visState.color;
    outState->uCoord = spawnDist / (double)draw->elemDef->trailDef->repeatDist + uCoordOffset;
}

void __cdecl FX_GenTrail_VertsForSegment(const FxTrailSegmentDrawState *segmentDrawState, GfxPackedVertex *remoteVerts)
{
    float scale1; // [esp+8h] [ebp-10Ch]
    __int16 v3; // [esp+38h] [ebp-DCh]
    __int16 v4; // [esp+3Ch] [ebp-D8h]
    float scale0; // [esp+40h] [ebp-D4h]
    float v6; // [esp+44h] [ebp-D0h]
    PackedUnitVec v7; // [esp+48h] [ebp-CCh]
    float v8; // [esp+50h] [ebp-C4h]
    float v9; // [esp+54h] [ebp-C0h]
    int v10; // [esp+5Ch] [ebp-B8h]
    float texCoord; // [esp+68h] [ebp-ACh]
    int v12; // [esp+70h] [ebp-A4h]
    float rotation; // [esp+94h] [ebp-80h]
    float normal; // [esp+9Ch] [ebp-78h]
    float normal_4; // [esp+A0h] [ebp-74h]
    float normal_8; // [esp+A4h] [ebp-70h]
    float temp; // [esp+ACh] [ebp-68h]
    float temp_4; // [esp+B0h] [ebp-64h]
    float temp_8; // [esp+B4h] [ebp-60h]
    float leftFloat4; // [esp+BCh] [ebp-58h]
    float leftFloat4_4; // [esp+C0h] [ebp-54h]
    float leftFloat4_8; // [esp+C4h] [ebp-50h]
    FxTrailDef *trailDef; // [esp+CCh] [ebp-48h]
    float cosRot; // [esp+D4h] [ebp-40h]
    float left[3]; // [esp+D8h] [ebp-3Ch] BYREF
    float sinRot; // [esp+E4h] [ebp-30h]
    float4 upFloat4; // [esp+E8h] [ebp-2Ch]
    int uCoord; // [esp+FCh] [ebp-18h]
    float up[3]; // [esp+100h] [ebp-14h] BYREF
    int vertCount; // [esp+10Ch] [ebp-8h]
    int vertIter; // [esp+110h] [ebp-4h]

    trailDef = segmentDrawState->trailDef;
    if (!segmentDrawState->trailDef)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 499, 0, "%s", "trailDef");
    if (trailDef->vertCount <= 0)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 500, 0, "%s", "trailDef->vertCount > 0");
    if (trailDef->indCount <= 0)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 501, 0, "%s", "trailDef->indCount > 0");
    rotation = segmentDrawState->rotation;
    cosRot = cos(rotation);
    sinRot = sin(rotation);
    Vec3ScaleMad(cosRot, segmentDrawState->basis[0], sinRot, segmentDrawState->basis[1], left);
    scale1 = -cosRot;
    Vec3ScaleMad(sinRot, segmentDrawState->basis[0], scale1, segmentDrawState->basis[1], up);
    leftFloat4 = left[0];
    leftFloat4_4 = left[1];
    leftFloat4_8 = left[2];
    upFloat4.v[0] = up[0];
    upFloat4.v[1] = up[1];
    upFloat4.v[2] = up[2];
    upFloat4.v[3] = 0.0;
    uCoord = SLODWORD(segmentDrawState->uCoord);
    vertCount = trailDef->vertCount;
    for (vertIter = 0; vertIter != vertCount; ++vertIter)
    {
        v6 = trailDef->verts[vertIter].pos[1] * segmentDrawState->size[1];
        scale0 = trailDef->verts[vertIter].pos[0] * segmentDrawState->size[0];
        Vec3MadMad(segmentDrawState->posWorld, scale0, left, v6, up, remoteVerts->xyz);
        remoteVerts->color.packed = *(unsigned int *)segmentDrawState->color;
        if ((int)((2 * uCoord) ^ 0x80000000) >> 14 < 0x3FFF)
            v12 = (int)((2 * uCoord) ^ 0x80000000) >> 14;
        else
            v12 = 0x3FFF;
        if (v12 > -16384)
            v4 = v12;
        else
            v4 = -16384;
        texCoord = trailDef->verts[vertIter].texCoord;
        if ((int)((2 * LODWORD(texCoord)) ^ 0x80000000) >> 14 < 0x3FFF)
            v10 = (int)((2 * LODWORD(texCoord)) ^ 0x80000000) >> 14;
        else
            v10 = 0x3FFF;
        if (v10 > -16384)
            v3 = v10;
        else
            v3 = -16384;
        remoteVerts->texCoord.packed = (v3 & 0x3FFF | (SLODWORD(texCoord) >> 16) & 0xC000)
            + ((v4 & 0x3FFF | (uCoord >> 16) & 0xC000) << 16);
        v9 = trailDef->verts[vertIter].normal[0];
        temp = v9 * leftFloat4;
        temp_4 = v9 * leftFloat4_4;
        temp_8 = v9 * leftFloat4_8;
        v8 = trailDef->verts[vertIter].normal[1];
        normal = v8 * upFloat4.v[0] + temp;
        normal_4 = v8 * upFloat4.v[1] + temp_4;
        normal_8 = v8 * upFloat4.v[2] + temp_8;
        v7.array[0] = (int)(normal * 127.0 + 127.5);
        v7.array[1] = (int)(normal_4 * 127.0 + 127.5);
        v7.array[2] = (int)(normal_8 * 127.0 + 127.5);
        v7.array[3] = 63;
        remoteVerts->normal = v7;
        remoteVerts->tangent.packed = 1065320446;
        ++remoteVerts;
    }
}

void __cdecl Vec3MadMad(
    const float *start,
    float scale0,
    const float *dir0,
    float scale1,
    const float *dir1,
    float *result)
{
    *result = scale0 * *dir0 + *start + scale1 * *dir1;
    result[1] = scale0 * dir0[1] + start[1] + scale1 * dir1[1];
    result[2] = scale0 * dir0[2] + start[2] + scale1 * dir1[2];
}

void __cdecl FX_DrawSpriteEffect(FxSystem *system, FxEffect *effect, int drawTime)
{
    unsigned __int16 elemHandle; // [esp+0h] [ebp-C0h]
    const FxElemDef *elemDef; // [esp+4h] [ebp-BCh]
    FxDrawState drawState; // [esp+8h] [ebp-B8h] BYREF
    const FxElemDef *elemDefs; // [esp+B8h] [ebp-8h]
    FxElem *elem; // [esp+BCh] [ebp-4h]

    drawState.effect = effect;
    drawState.msecDraw = drawTime;
    elemHandle = effect->firstElemHandle[0];
    if (elemHandle != 0xFFFF)
    {
        drawState.system = system;
        elemDefs = drawState.effect->def->elemDefs;
        while (elemHandle != 0xFFFF)
        {
            if (!system)
                MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_system.h", 334, 0, "%s", "system");
            elem = (FxElem *)FX_PoolFromHandle_Generic<FxElem, 2048>(system->elems, elemHandle);
            elemDef = &elemDefs[elem->defIndex];
            if (elemDef->elemType > 3u)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_draw.cpp",
                    1304,
                    0,
                    "elemDef->elemType <= FX_ELEM_TYPE_LAST_SPRITE\n\t%i, %i",
                    elemDef->elemType,
                    3);
            if (elemDef->visualCount)
                FX_DrawElement(system, elemDef, elem, &drawState);
            elemHandle = elem->nextElemHandleInEffect;
        }
    }
}

void __cdecl FX_GenerateVerts(FxGenerateVertsCmd *cmd)
{
    int drawTime; // [esp+50h] [ebp-8h]
    FxSystem *localSystem; // [esp+54h] [ebp-4h]

    localSystem = cmd->system;
    //Profile_Begin(196);
    R_BeginCodeMeshVerts();
    drawTime = localSystem->msecDraw;
    if (drawTime >= 0)
    {
        FX_SpriteGenerateVerts(cmd);
        FX_Beam_GenerateVerts(cmd);
        FX_PostLight_GenerateVerts(cmd->postLightInfo, localSystem);
        if (fx_enable->current.enabled && fx_draw->current.enabled)
            FX_DrawSpriteElems(localSystem, drawTime);
        R_EndCodeMeshVerts();
        FX_ToggleVisBlockerFrame(localSystem);
        //Profile_EndInternal(0);
    }
    else
    {
        R_EndCodeMeshVerts();
        //Profile_EndInternal(0);
    }
}

void __cdecl FX_FillGenerateVertsCmd(int localClientNum, FxGenerateVertsCmd *cmd)
{
    unsigned int v2; // [esp+0h] [ebp-10h]
    unsigned int v3; // [esp+Ch] [ebp-4h]

    if (!cmd)
        MyAssertHandler(".\\EffectsCore\\fx_draw.cpp", 1642, 0, "%s", "cmd");
    cmd->system = FX_GetSystem(localClientNum);
    cmd->beamInfo = FX_Beam_GetInfo();
    cmd->postLightInfo = FX_PostLight_GetInfo();
    cmd->spriteInfo = FX_SpriteGetInfo();
    cmd->localClientNum = localClientNum;
    v3 = R_GetLocalClientNum();
    if (v3)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            v3);
    cmd->vieworg[0] = MEMORY[0x9D8718][0];
    cmd->vieworg[1] = MEMORY[0x9D8718][1];
    cmd->vieworg[2] = MEMORY[0x9D8718][2];
    v2 = R_GetLocalClientNum();
    if (v2)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            v2);
    AxisCopy((const float (*)[3])MEMORY[0x9D8724], cmd->viewaxis);
}

void __cdecl FX_EvaluateDistanceFade(FxDrawState *draw)
{
    float v1; // [esp+18h] [ebp-3Ch]
    float v2; // [esp+1Ch] [ebp-38h]
    float diff[3]; // [esp+38h] [ebp-1Ch] BYREF
    const FxElemDef *def; // [esp+44h] [ebp-10h]
    float dist; // [esp+48h] [ebp-Ch]
    float fadeInFrac; // [esp+4Ch] [ebp-8h]
    float fadeOutFrac; // [esp+50h] [ebp-4h]

    def = draw->elemDef;
    if (def->fadeInRange.amplitude != 0.0 || def->fadeOutRange.amplitude != 0.0)
    {
        fadeInFrac = 1.0;
        fadeOutFrac = 1.0;
        Vec3Sub(draw->posWorld, draw->camera->origin, diff);
        dist = Vec3Length(diff);
        if (def->fadeInRange.amplitude != 0.0)
            fadeInFrac = FX_ClampRangeLerp(dist, &def->fadeInRange);
        if (def->fadeOutRange.amplitude != 0.0)
            fadeOutFrac = 1.0 - FX_ClampRangeLerp(dist, &def->fadeOutRange);
        v2 = fadeOutFrac - fadeInFrac;
        if (v2 < 0.0)
            v1 = fadeOutFrac;
        else
            v1 = fadeInFrac;
        draw->preVisState.distanceFade = (__int64)(v1 * 255.0 + 0.5);
    }
}

double __cdecl FX_ClampRangeLerp(float dist, const FxFloatRange *range)
{
    float baseDist; // [esp+0h] [ebp-8h]
    float value; // [esp+4h] [ebp-4h]

    baseDist = dist - range->base;
    value = 0.0;
    if (baseDist >= 0.0)
    {
        if (range->amplitude > (double)baseDist)
            return (float)(1.0 - baseDist / range->amplitude);
    }
    else
    {
        return (float)1.0;
    }
    return value;
}

