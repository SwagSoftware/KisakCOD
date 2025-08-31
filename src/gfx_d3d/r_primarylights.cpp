#include "r_primarylights.h"
#include <bgame/bg_local.h>
#include <qcommon/com_bsp.h>
#include "r_init.h"
#include "r_scene.h"
#include "r_dvars.h"
#include "r_spotshadow.h"

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_local.h>
#include <cgame/cg_ents.h>
#endif

GfxShadowedLightHistory s_shadowHistory[4];
void __cdecl R_ClearShadowedPrimaryLightHistory(int localClientNum)
{
    GfxShadowedLightHistory *history; // [esp+0h] [ebp-4h]

    history = &s_shadowHistory[localClientNum];
    history->entryCount = 0;
    history->lastUpdateTime = 0;
    Com_Memset(history->shadowableLightWasUsed, 0, 32);
}

void __cdecl R_AddDynamicShadowableLight(GfxViewInfo *viewInfo, const GfxLight *visibleLight)
{
    if (viewInfo->shadowableLightCount == 255)
    {
        Com_PrintError(1, "Too many total shadowable lights (%d)\n", viewInfo->shadowableLightCount);
    }
    else
    {
        Com_BitSetAssert(scene.shadowableLightIsUsed, viewInfo->shadowableLightCount, 128);
        memcpy(
            &viewInfo->shadowableLights[viewInfo->shadowableLightCount++],
            visibleLight,
            sizeof(viewInfo->shadowableLights[viewInfo->shadowableLightCount++]));
    }
}

bool __cdecl R_IsDynamicShadowedLight(unsigned int shadowableLightIndex)
{
    iassert( comWorld.isInUse );
    return shadowableLightIndex >= comWorld.primaryLightCount;
}

bool __cdecl R_IsPrimaryLight(unsigned int shadowableLightIndex)
{
    iassert( comWorld.isInUse );
    return shadowableLightIndex < comWorld.primaryLightCount;
}

void __cdecl R_ChooseShadowedLights(GfxViewInfo *viewInfo)
{
    DWORD v2; // eax
    GfxCandidateShadowedLight candidateLights[5]; // [esp+14h] [ebp-74h] BYREF
    unsigned int timeDelta; // [esp+3Ch] [ebp-4Ch]
    unsigned int entryIndex; // [esp+40h] [ebp-48h]
    unsigned int scanIndex; // [esp+44h] [ebp-44h]
    GfxShadowedLightHistory *shadowHistory; // [esp+48h] [ebp-40h]
    unsigned int leadingZeros; // [esp+4Ch] [ebp-3Ch]
    unsigned int bitIndex; // [esp+50h] [ebp-38h]
    unsigned int usedBits; // [esp+54h] [ebp-34h]
    float fadeDelta; // [esp+58h] [ebp-30h]
    unsigned int candidateLightIndex; // [esp+5Ch] [ebp-2Ch]
    unsigned int candidateLightCount; // [esp+60h] [ebp-28h]
    unsigned int scanLimit; // [esp+64h] [ebp-24h]
    unsigned int shadowableLightIsUsed[8]; // [esp+68h] [ebp-20h] BYREF

    shadowHistory = &s_shadowHistory[viewInfo->localClientNum];
    timeDelta = viewInfo->sceneDef.time - shadowHistory->lastUpdateTime;
    if (timeDelta)
    {
        shadowHistory->lastUpdateTime = viewInfo->sceneDef.time;
        fadeDelta = (double)timeDelta * (EQUAL_EPSILON / sm_spotShadowFadeTime->current.value);
        R_FadeOutShadowHistoryEntries(shadowHistory, fadeDelta);
        memcpy(shadowableLightIsUsed, scene.shadowableLightIsUsed, sizeof(shadowableLightIsUsed));
        candidateLightCount = 0;
        iassert( comWorld.isInUse );
        if (viewInfo->shadowableLightCount < comWorld.primaryLightCount)
            MyAssertHandler(
                ".\\r_primarylights.cpp",
                226,
                0,
                "%s",
                "viewInfo->shadowableLightCount >= Com_GetPrimaryLightCount()");
        scanLimit = (viewInfo->shadowableLightCount + 31) >> 5;
        for (scanIndex = 0; scanIndex != scanLimit; ++scanIndex)
        {
            usedBits = scene.shadowableLightIsUsed[scanIndex];
            while (1)
            {
                if (!_BitScanReverse(&v2, usedBits))
                    v2 = 63;// `CountLeadingZeros'::`2': : notFound;
                leadingZeros = v2 ^ 0x1F;
                if ((v2 ^ 0x1F) == 0x20)
                    break;
                bitIndex = 31 - leadingZeros;
                usedBits &= ~(1 << (31 - leadingZeros));
                candidateLightCount = R_AddPotentiallyShadowedLight(
                    viewInfo,
                    31 - leadingZeros + 32 * scanIndex,
                    candidateLights,
                    candidateLightCount);
            }
        }
        for (candidateLightIndex = 0; candidateLightIndex < candidateLightCount; ++candidateLightIndex)
            R_AddShadowedLightToShadowHistory(
                shadowHistory,
                candidateLights[candidateLightIndex].shadowableLightIndex,
                fadeDelta);
        memcpy(shadowHistory, shadowableLightIsUsed, 0x20u);
    }
    else if (rgp.world->sunPrimaryLightIndex
        && Com_BitCheckAssert(scene.shadowableLightIsUsed, rgp.world->sunPrimaryLightIndex, 128))
    {
        Com_BitSetAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32);
    }
    if (viewInfo->spotShadowCount)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            248,
            1,
            "viewInfo->spotShadowCount == 0\n\t%i, %i",
            viewInfo->spotShadowCount,
            0);
    if (r_rendererInUse->current.integer || gfxMetrics.shadowmapBuildTechType != TECHNIQUE_BUILD_SHADOWMAP_COLOR)
    {
        for (entryIndex = 0; entryIndex < shadowHistory->entryCount; ++entryIndex)
            R_AddShadowsForLight(
                viewInfo,
                shadowHistory->entries[entryIndex].shadowableLightIndex,
                shadowHistory->entries[entryIndex].fade);
    }
}

unsigned int __cdecl R_AddPotentiallyShadowedLight(
    const GfxViewInfo *viewInfo,
    unsigned int shadowableLightIndex,
    GfxCandidateShadowedLight *candidateLights,
    unsigned int candidateLightCount)
{
    float v5; // ecx
    unsigned int insertIndex; // [esp+14h] [ebp-8h]
    float score; // [esp+18h] [ebp-4h]

    if (!shadowableLightIndex)
        return candidateLightCount;
    if (shadowableLightIndex == rgp.world->sunPrimaryLightIndex)
    {
        Com_BitSetAssert(frontEndDataOut->shadowableLightHasShadowMap, shadowableLightIndex, 32);
        return candidateLightCount;
    }
    else if (viewInfo->shadowableLights[shadowableLightIndex].canUseShadowMap && sm_spotEnable->current.enabled)
    {
        score = R_ShadowedSpotLightScore(&viewInfo->viewParms, &viewInfo->shadowableLights[shadowableLightIndex]);
        for (insertIndex = candidateLightCount;
            insertIndex && candidateLights[insertIndex - 1].score < (double)score;
            --insertIndex)
        {
            v5 = candidateLights[insertIndex - 1].score;
            candidateLights[insertIndex].shadowableLightIndex = candidateLights[insertIndex - 1].shadowableLightIndex;
            candidateLights[insertIndex].score = v5;
        }
        candidateLights[insertIndex].shadowableLightIndex = shadowableLightIndex;
        candidateLights[insertIndex].score = score;
        if ((signed int)(candidateLightCount + 1) < sm_maxLights->current.integer)
            return candidateLightCount + 1;
        else
            return sm_maxLights->current.unsignedInt;
    }
    else
    {
        return candidateLightCount;
    }
}

const float vec3_colorintensity[3] = { 0.2989f, 0.587f, 0.114f };
double __cdecl R_ShadowedSpotLightScore(const GfxViewParms *viewParms, const GfxLight *light)
{
    float scale; // [esp+10h] [ebp-34h]
    float intensity; // [esp+18h] [ebp-2Ch]
    float eyeRefPoint[3]; // [esp+1Ch] [ebp-28h] BYREF
    float deltaToLight[3]; // [esp+28h] [ebp-1Ch] BYREF
    float distToLightFocus; // [esp+34h] [ebp-10h]
    float deltaToFocus[3]; // [esp+38h] [ebp-Ch] BYREF

    Vec3Mad(viewParms->origin, sm_lightScore_eyeProjectDist->current.value, viewParms->axis[0], eyeRefPoint);
    Vec3Sub(light->origin, eyeRefPoint, deltaToLight);
    scale = -light->radius * sm_lightScore_spotProjectFrac->current.value;
    Vec3Mad(deltaToLight, scale, light->dir, deltaToFocus);
    distToLightFocus = Vec3Length(deltaToFocus);
    intensity = Vec3Dot(light->color, vec3_colorintensity);
    return (float)(light->radius * intensity / (distToLightFocus + 1.0));
}

void __cdecl R_AddShadowsForLight(GfxViewInfo *viewInfo, unsigned int shadowableLightIndex, float spotShadowFade)
{
    if (R_AddSpotShadowsForLight(
        viewInfo,
        &viewInfo->shadowableLights[shadowableLightIndex],
        shadowableLightIndex,
        spotShadowFade))
    {
        Com_BitSetAssert(frontEndDataOut->shadowableLightHasShadowMap, shadowableLightIndex, 32);
    }
}

void __cdecl R_AddShadowedLightToShadowHistory(
    GfxShadowedLightHistory *shadowHistory,
    unsigned int shadowableLightIndex,
    float fadeDelta)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float v5; // [esp+8h] [ebp-Ch]
    float v6; // [esp+Ch] [ebp-8h]
    unsigned int historyIndex; // [esp+10h] [ebp-4h]

    for (historyIndex = 0; historyIndex != shadowHistory->entryCount; ++historyIndex)
    {
        if (shadowHistory->entries[historyIndex].shadowableLightIndex == shadowableLightIndex)
        {
            shadowHistory->entries[historyIndex].isFadingOut = 0;
            v6 = fadeDelta + shadowHistory->entries[historyIndex].fade;
            v5 = v6 - 1.0;
            if (v5 < 0.0)
                v4 = fadeDelta + shadowHistory->entries[historyIndex].fade;
            else
                v4 = 1.0;
            shadowHistory->entries[historyIndex].fade = v4;
            return;
        }
    }
    if (shadowHistory->entryCount < sm_maxLights->current.integer)
    {
        shadowHistory->entries[shadowHistory->entryCount].shadowableLightIndex = shadowableLightIndex;
        if (shadowHistory->entries[shadowHistory->entryCount].shadowableLightIndex != shadowableLightIndex)
            MyAssertHandler(
                ".\\r_primarylights.cpp",
                117,
                0,
                "%s",
                "shadowHistory->entries[shadowHistory->entryCount].shadowableLightIndex == shadowableLightIndex");
        shadowHistory->entries[shadowHistory->entryCount].isFadingOut = 0;
        if (Com_BitCheckAssert(shadowHistory->shadowableLightWasUsed, shadowableLightIndex, 32))
            v3 = fadeDelta;
        else
            v3 = 1.0;
        shadowHistory->entries[shadowHistory->entryCount++].fade = v3;
    }
}

void __cdecl R_FadeOutShadowHistoryEntries(GfxShadowedLightHistory *shadowHistory, float fadeDelta)
{
    unsigned int v2; // eax
    int v3; // edx
    float fade; // eax
    unsigned int entryIndex; // [esp+4h] [ebp-4h]

    entryIndex = 0;
    while (entryIndex != shadowHistory->entryCount)
    {
        iassert( shadowHistory->entries[entryIndex].fade > 0.0f );
        if (Com_BitCheckAssert(scene.shadowableLightIsUsed, shadowHistory->entries[entryIndex].shadowableLightIndex, 128))
        {
            if (shadowHistory->entries[entryIndex].isFadingOut)
            {
                shadowHistory->entries[entryIndex].fade = shadowHistory->entries[entryIndex].fade - fadeDelta;
                if (shadowHistory->entries[entryIndex].fade < 0.009999999776482582)
                    goto LABEL_10;
                ++entryIndex;
            }
            else
            {
                shadowHistory->entries[entryIndex++].isFadingOut = 1;
            }
        }
        else
        {
        LABEL_10:
            v2 = --shadowHistory->entryCount;
            v3 = *(unsigned int *)&shadowHistory->entries[v2].shadowableLightIndex;
            fade = shadowHistory->entries[v2].fade;
            *(unsigned int *)&shadowHistory->entries[entryIndex].shadowableLightIndex = v3;
            shadowHistory->entries[entryIndex].fade = fade;
        }
    }
}

void __cdecl R_LinkSphereEntityToPrimaryLights(
    unsigned int localClientNum,
    unsigned int entityNum,
    const float *origin,
    float radius)
{
    float v4; // [esp+Ch] [ebp-2Ch]
    float v5; // [esp+1Ch] [ebp-1Ch]
    float diff[3]; // [esp+20h] [ebp-18h] BYREF
    unsigned int primaryLightIndex; // [esp+2Ch] [ebp-Ch]
    const ComPrimaryLight *light; // [esp+30h] [ebp-8h]
    unsigned int bitIndex; // [esp+34h] [ebp-4h]

    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if (light->type != 2 && light->type != 3)
            MyAssertHandler(
                ".\\r_primarylights.cpp",
                303,
                0,
                "%s\n\t(light->type) = %i",
                "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                light->type);
        Vec3Sub(origin, light->origin, diff);
        v5 = Vec3LengthSq(diff);
        v4 = (light->radius + radius) * (light->radius + radius);
        if (v5 < (double)v4
            && (light->type != 2
                || light->cosHalfFovExpanded < 0.0
                || !CullSphereFromCone(light->origin, light->dir, light->cosHalfFovExpanded, origin, radius)))
        {
            bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
            Com_BitSetAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
        }
    }
}

unsigned int __cdecl R_GetPrimaryLightEntityShadowBit(
    unsigned int localClientNum,
    unsigned int entnum,
    unsigned int primaryLightIndex)
{
    if (rgp.world->sunPrimaryLightIndex > 1)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            266,
            0,
            "%s",
            "rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_SUN || rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_NONE");
    if (rgp.world->sunPrimaryLightIndex + 1 > primaryLightIndex || primaryLightIndex > rgp.world->primaryLightCount - 1)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            267,
            0,
            "primaryLightIndex not in [rgp.world->sunPrimaryLightIndex + 1, rgp.world->primaryLightCount - 1]\n"
            "\t%i not in [%i, %i]",
            primaryLightIndex,
            rgp.world->sunPrimaryLightIndex + 1,
            rgp.world->primaryLightCount - 1);
    return primaryLightIndex
        - (rgp.world->sunPrimaryLightIndex
            + 1)
        + (rgp.world->primaryLightCount - (rgp.world->sunPrimaryLightIndex + 1))
        * (entnum + gfxCfg.entCount * localClientNum);
}

void __cdecl R_LinkBoxEntityToPrimaryLights(
    unsigned int localClientNum,
    unsigned int entityNum,
    const float *mins,
    const float *maxs)
{
    double v4; // st7
    float v5; // [esp+Ch] [ebp-4Ch]
    char v6; // [esp+1Fh] [ebp-39h]
    GfxLightRegion *v7; // [esp+20h] [ebp-38h]
    unsigned int i; // [esp+24h] [ebp-34h]
    float diff[3]; // [esp+28h] [ebp-30h] BYREF
    unsigned int primaryLightIndex; // [esp+34h] [ebp-24h]
    const ComPrimaryLight *light; // [esp+38h] [ebp-20h]
    unsigned int bitIndex; // [esp+3Ch] [ebp-1Ch]
    float boxHalfSize[3]; // [esp+40h] [ebp-18h] BYREF
    float boxMidPoint[3]; // [esp+4Ch] [ebp-Ch] BYREF

    Vec3Avg(mins, maxs, boxMidPoint);
    Vec3Sub(boxMidPoint, mins, boxHalfSize);
    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if (light->type != 2 && light->type != 3)
            MyAssertHandler(
                ".\\r_primarylights.cpp",
                332,
                0,
                "%s\n\t(light->type) = %i",
                "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                light->type);
        v4 = PointToBoxDistSq(light->origin, mins, maxs);
        v5 = light->radius * light->radius;
        if (v5 > v4
            && (light->type != 2
                || light->cosHalfFovExpanded < 0.0
                || !CullBoxFromCone(light->origin, light->dir, light->cosHalfFovExpanded, boxMidPoint, boxHalfSize)))
        {
            v7 = &rgp.world->lightRegion[primaryLightIndex];
            if (v7->hullCount)
            {
                Vec3Sub(boxMidPoint, light->origin, diff);
                for (i = 0; i < v7->hullCount; ++i)
                {
                    if (!R_CullBoxFromLightRegionHull(&v7->hulls[i], diff, boxHalfSize))
                    {
                        v6 = 0;
                        goto LABEL_20;
                    }
                }
                v6 = 1;
            }
            else
            {
                v6 = 0;
            }
        LABEL_20:
            if (!v6)
            {
                bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
                Com_BitSetAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
            }
        }
    }
}

char __cdecl R_CullBoxFromLightRegionHull(
    const GfxLightRegionHull *hull,
    const float *boxMidPoint,
    const float *boxHalfSize)
{
    float v4; // [esp+0h] [ebp-C8h]
    float v5; // [esp+4h] [ebp-C4h]
    float v6; // [esp+8h] [ebp-C0h]
    float v7; // [esp+Ch] [ebp-BCh]
    float v8; // [esp+10h] [ebp-B8h]
    float v9; // [esp+14h] [ebp-B4h]
    float v10; // [esp+18h] [ebp-B0h]
    float v11; // [esp+1Ch] [ebp-ACh]
    float v12; // [esp+20h] [ebp-A8h]
    float v13; // [esp+24h] [ebp-A4h]
    float v14; // [esp+28h] [ebp-A0h]
    float v15; // [esp+2Ch] [ebp-9Ch]
    float v16; // [esp+30h] [ebp-98h]
    float v17; // [esp+34h] [ebp-94h]
    float v18; // [esp+38h] [ebp-90h]
    float v19; // [esp+3Ch] [ebp-8Ch]
    float v20; // [esp+40h] [ebp-88h]
    float v21; // [esp+44h] [ebp-84h]
    float v22; // [esp+48h] [ebp-80h]
    float v23; // [esp+4Ch] [ebp-7Ch]
    float v24; // [esp+54h] [ebp-74h]
    float v25; // [esp+74h] [ebp-54h]
    float v26; // [esp+7Ch] [ebp-4Ch]
    float v27; // [esp+84h] [ebp-44h]
    float v28; // [esp+8Ch] [ebp-3Ch]
    float v29; // [esp+94h] [ebp-34h]
    float v30; // [esp+9Ch] [ebp-2Ch]
    float v31; // [esp+A4h] [ebp-24h]
    float v32; // [esp+ACh] [ebp-1Ch]
    float v33; // [esp+B4h] [ebp-14h]
    float halfSizeOnAxis; // [esp+B8h] [ebp-10h]
    float halfSizeOnAxisa; // [esp+B8h] [ebp-10h]
    float halfSizeOnAxisb; // [esp+B8h] [ebp-10h]
    float halfSizeOnAxisc; // [esp+B8h] [ebp-10h]
    GfxLightRegionAxis *dir; // [esp+BCh] [ebp-Ch]
    float midPointOnAxis; // [esp+C0h] [ebp-8h]
    float midPointOnAxisa; // [esp+C0h] [ebp-8h]
    float midPointOnAxisb; // [esp+C0h] [ebp-8h]
    float midPointOnAxisc; // [esp+C0h] [ebp-8h]
    float midPointOnAxisd; // [esp+C0h] [ebp-8h]
    float midPointOnAxise; // [esp+C0h] [ebp-8h]
    float midPointOnAxisf; // [esp+C0h] [ebp-8h]
    unsigned int axisIter; // [esp+C4h] [ebp-4h]

    v33 = *boxMidPoint - hull->kdopMidPoint[0];
    v23 = I_fabs(v33);
    if (v23 >= *boxHalfSize + hull->kdopHalfSize[0])
        return 1;
    v32 = boxMidPoint[1] - hull->kdopMidPoint[1];
    v22 = I_fabs(v32);
    if (v22 >= boxHalfSize[1] + hull->kdopHalfSize[1])
        return 1;
    v31 = boxMidPoint[2] - hull->kdopMidPoint[2];
    v21 = I_fabs(v31);
    if (v21 >= boxHalfSize[2] + hull->kdopHalfSize[2])
        return 1;
    halfSizeOnAxis = *boxHalfSize + boxHalfSize[1];
    midPointOnAxis = *boxMidPoint + boxMidPoint[1];
    v30 = midPointOnAxis - hull->kdopMidPoint[3];
    v20 = I_fabs(v30);
    v19 = halfSizeOnAxis + hull->kdopHalfSize[3];
    if (v20 >= (double)v19)
        return 1;
    midPointOnAxisa = *boxMidPoint - boxMidPoint[1];
    v29 = midPointOnAxisa - hull->kdopMidPoint[4];
    v18 = I_fabs(v29);
    v17 = halfSizeOnAxis + hull->kdopHalfSize[4];
    if (v18 >= (double)v17)
        return 1;
    halfSizeOnAxisa = *boxHalfSize + boxHalfSize[2];
    midPointOnAxisb = *boxMidPoint + boxMidPoint[2];
    v28 = midPointOnAxisb - hull->kdopMidPoint[5];
    v16 = I_fabs(v28);
    v15 = halfSizeOnAxisa + hull->kdopHalfSize[5];
    if (v16 >= (double)v15)
        return 1;
    midPointOnAxisc = *boxMidPoint - boxMidPoint[2];
    v27 = midPointOnAxisc - hull->kdopMidPoint[6];
    v14 = I_fabs(v27);
    v13 = halfSizeOnAxisa + hull->kdopHalfSize[6];
    if (v14 >= (double)v13)
        return 1;
    halfSizeOnAxisb = boxHalfSize[1] + boxHalfSize[2];
    midPointOnAxisd = boxMidPoint[1] + boxMidPoint[2];
    v26 = midPointOnAxisd - hull->kdopMidPoint[7];
    v12 = I_fabs(v26);
    v11 = halfSizeOnAxisb + hull->kdopHalfSize[7];
    if (v12 >= (double)v11)
        return 1;
    midPointOnAxise = boxMidPoint[1] - boxMidPoint[2];
    v25 = midPointOnAxise - hull->kdopMidPoint[8];
    v10 = I_fabs(v25);
    v9 = halfSizeOnAxisb + hull->kdopHalfSize[8];
    if (v10 >= (double)v9)
        return 1;
    for (axisIter = 0; axisIter < hull->axisCount; ++axisIter)
    {
        dir = &hull->axis[axisIter];
        v8 = I_fabs(dir->dir[0]);
        v7 = I_fabs(dir->dir[1]);
        v6 = I_fabs(dir->dir[2]);
        halfSizeOnAxisc = *boxHalfSize * v8 + boxHalfSize[1] * v7 + boxHalfSize[2] * v6;
        midPointOnAxisf = *boxMidPoint * dir->dir[0] + boxMidPoint[1] * dir->dir[1] + boxMidPoint[2] * dir->dir[2];
        v24 = midPointOnAxisf - dir->midPoint;
        v5 = I_fabs(v24);
        v4 = halfSizeOnAxisc + dir->halfSize;
        if (v5 >= (double)v4)
            return 1;
    }
    return 0;
}

void __cdecl R_LinkDynEntToPrimaryLights(
    unsigned int dynEntId,
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs)
{
    float v[6]; // [esp+0h] [ebp-60h] BYREF
    char v5; // [esp+1Bh] [ebp-45h]
    GfxLightRegion *v6; // [esp+1Ch] [ebp-44h]
    unsigned int i; // [esp+20h] [ebp-40h]
    float diff[3]; // [esp+24h] [ebp-3Ch] BYREF
    unsigned int primaryLightIndex; // [esp+30h] [ebp-30h]
    const ComPrimaryLight *light; // [esp+34h] [ebp-2Ch]
    unsigned int bitIndex; // [esp+38h] [ebp-28h]
    float boxHalfSize[3]; // [esp+3Ch] [ebp-24h] BYREF
    unsigned int bestPrimaryLightIndex; // [esp+48h] [ebp-18h]
    float boxMidPoint[3]; // [esp+4Ch] [ebp-14h] BYREF
    float distSq; // [esp+58h] [ebp-8h]
    float minDistSq; // [esp+5Ch] [ebp-4h]

    Vec3Avg(mins, maxs, boxMidPoint);
    Vec3Sub(boxMidPoint, mins, boxHalfSize);
    bestPrimaryLightIndex = 0;
    minDistSq = FLT_MAX;
    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if (light->type != 2 && light->type != 3)
            MyAssertHandler(
                ".\\r_primarylights.cpp",
                369,
                0,
                "%s\n\t(light->type) = %i",
                "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                light->type);
        if (!Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize))
        {
            v6 = &rgp.world->lightRegion[primaryLightIndex];
            if (v6->hullCount)
            {
                Vec3Sub(boxMidPoint, light->origin, diff);
                for (i = 0; i < v6->hullCount; ++i)
                {
                    if (!R_CullBoxFromLightRegionHull(&v6->hulls[i], diff, boxHalfSize))
                    {
                        v5 = 0;
                        goto LABEL_17;
                    }
                }
                v5 = 1;
            }
            else
            {
                v5 = 0;
            }
        LABEL_17:
            if (!v5)
            {
                bitIndex = R_GetPrimaryLightDynEntShadowBit(dynEntId, primaryLightIndex);
                Com_BitSetAssert(rgp.world->primaryLightDynEntShadowVis[drawType], bitIndex, 0xFFFFFFF);
                Vec3Sub(boxMidPoint, light->origin, v);
                distSq = Vec3LengthSq(v);
                if (minDistSq > (double)distSq)
                {
                    bestPrimaryLightIndex = primaryLightIndex;
                    minDistSq = distSq;
                }
            }
        }
    }
    if (drawType == DYNENT_DRAW_MODEL)
    {
        if (bestPrimaryLightIndex != (unsigned __int8)bestPrimaryLightIndex)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                bestPrimaryLightIndex,
                (unsigned __int8)bestPrimaryLightIndex);
        rgp.world->nonSunPrimaryLightForModelDynEnt[dynEntId] = bestPrimaryLightIndex;
    }
}

bool __cdecl Com_CullBoxFromPrimaryLight(
    const ComPrimaryLight *light,
    const float *boxMidPoint,
    const float *boxHalfSize)
{
    if (light->type == 2 && light->cosHalfFovExpanded >= 0.0)
        return CullBoxFromConicSectionOfSphere(
            light->origin,
            light->dir,
            light->cosHalfFovExpanded,
            light->radius,
            boxMidPoint,
            boxHalfSize);
    else
        return CullBoxFromSphere(light->origin, light->radius, boxMidPoint, boxHalfSize);
}

unsigned int __cdecl R_GetPrimaryLightDynEntShadowBit(unsigned int entnum, unsigned int primaryLightIndex)
{
    if (rgp.world->sunPrimaryLightIndex > 1)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            284,
            0,
            "%s",
            "rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_SUN || rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_NONE");
    if (primaryLightIndex <= rgp.world->sunPrimaryLightIndex)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            285,
            0,
            "primaryLightIndex > rgp.world->sunPrimaryLightIndex\n\t%i, %i",
            primaryLightIndex,
            rgp.world->sunPrimaryLightIndex);
    return primaryLightIndex
        - (rgp.world->sunPrimaryLightIndex
            + 1)
        + (rgp.world->primaryLightCount - (rgp.world->sunPrimaryLightIndex + 1)) * entnum;
}

void __cdecl R_UnlinkEntityFromPrimaryLights(unsigned int localClientNum, unsigned int entityNum)
{
    unsigned int primaryLightIndex; // [esp+Ch] [ebp-8h]
    unsigned int bitIndex; // [esp+10h] [ebp-4h]

    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
        Com_BitClearAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
    }
}

void __cdecl R_UnlinkDynEntFromPrimaryLights(unsigned int dynEntId, DynEntityDrawType drawType)
{
    unsigned int primaryLightIndex; // [esp+Ch] [ebp-8h]
    unsigned int bitIndex; // [esp+10h] [ebp-4h]

    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        bitIndex = R_GetPrimaryLightDynEntShadowBit(dynEntId, primaryLightIndex);
        Com_BitClearAssert(rgp.world->primaryLightDynEntShadowVis[drawType], bitIndex, 0xFFFFFFF);
    }
}

bool __cdecl R_IsEntityVisibleToPrimaryLight(
    unsigned int localClientNum,
    unsigned int entityNum,
    unsigned int primaryLightIndex)
{
    unsigned int bitIndex; // [esp+Ch] [ebp-4h]

    bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
    return Com_BitCheckAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
}

bool __cdecl R_IsDynEntVisibleToPrimaryLight(
    unsigned int dynEntId,
    DynEntityDrawType drawType,
    unsigned int primaryLightIndex)
{
    unsigned int bitIndex; // [esp+Ch] [ebp-4h]

    bitIndex = R_GetPrimaryLightDynEntShadowBit(dynEntId, primaryLightIndex);
    return Com_BitCheckAssert(rgp.world->primaryLightDynEntShadowVis[drawType], bitIndex, 0xFFFFFFF);
}

int __cdecl R_IsEntityVisibleToAnyShadowedPrimaryLight(const GfxViewInfo *viewInfo, unsigned int entityNum)
{
    unsigned int baseBitIndex; // [esp+0h] [ebp-10h]
    unsigned int relevantPrimaryLightCount; // [esp+4h] [ebp-Ch]
    unsigned int ignoredPrimaryLightCount; // [esp+8h] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+Ch] [ebp-4h]

    ignoredPrimaryLightCount = rgp.world->sunPrimaryLightIndex + 1;
    relevantPrimaryLightCount = rgp.world->primaryLightCount - ignoredPrimaryLightCount;
    baseBitIndex = relevantPrimaryLightCount * (entityNum + gfxCfg.entCount * viewInfo->localClientNum)
        - ignoredPrimaryLightCount;
    if (relevantPrimaryLightCount + 1 < viewInfo->spotShadowCount)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            460,
            0,
            "relevantPrimaryLightCount + GFX_MAX_EMISSIVE_SPOT_LIGHTS >= viewInfo->spotShadowCount\n\t%i, %i",
            relevantPrimaryLightCount + 1,
            viewInfo->spotShadowCount);
    for (spotShadowIndex = 0; spotShadowIndex < viewInfo->spotShadowCount; ++spotShadowIndex)
    {
        if (R_IsEntityVisibleToShadowedPrimaryLight(
            baseBitIndex,
            viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex))
        {
            return 1;
        }
    }
    return 0;
}

bool __cdecl R_IsEntityVisibleToShadowedPrimaryLight(unsigned int baseBitIndex, unsigned int shadowableLightIndex)
{
    return R_IsPrimaryLight(shadowableLightIndex)
        && Com_BitCheckAssert(rgp.world->primaryLightEntityShadowVis, shadowableLightIndex + baseBitIndex, 0xFFFFFFF);
}

int __cdecl R_IsDynEntVisibleToAnyShadowedPrimaryLight(
    const GfxViewInfo *viewInfo,
    unsigned int dynEntId,
    DynEntityDrawType drawType)
{
    unsigned int relevantPrimaryLightCount; // [esp+4h] [ebp-Ch]
    unsigned int ignoredPrimaryLightCount; // [esp+8h] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+Ch] [ebp-4h]

    ignoredPrimaryLightCount = rgp.world->sunPrimaryLightIndex + 1;
    relevantPrimaryLightCount = rgp.world->primaryLightCount - ignoredPrimaryLightCount;
    if (relevantPrimaryLightCount + 1 < viewInfo->spotShadowCount)
        MyAssertHandler(
            ".\\r_primarylights.cpp",
            495,
            0,
            "relevantPrimaryLightCount + GFX_MAX_EMISSIVE_SPOT_LIGHTS >= viewInfo->spotShadowCount\n\t%i, %i",
            relevantPrimaryLightCount + 1,
            viewInfo->spotShadowCount);
    for (spotShadowIndex = 0; spotShadowIndex < viewInfo->spotShadowCount; ++spotShadowIndex)
    {
        if (R_IsDynEntVisibleToShadowedPrimaryLight(
            relevantPrimaryLightCount * dynEntId - ignoredPrimaryLightCount,
            drawType,
            viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex))
        {
            return 1;
        }
    }
    return 0;
}

bool __cdecl R_IsDynEntVisibleToShadowedPrimaryLight(
    unsigned int baseBitIndex,
    DynEntityDrawType drawType,
    unsigned int shadowableLightIndex)
{
    return R_IsPrimaryLight(shadowableLightIndex)
        && Com_BitCheckAssert(
            rgp.world->primaryLightDynEntShadowVis[drawType],
            shadowableLightIndex + baseBitIndex,
            0xFFFFFFF);
}

unsigned int __cdecl R_GetNonSunPrimaryLightForBox(
    const GfxViewInfo *viewInfo,
    const float *boxMidPoint,
    const float *boxHalfSize)
{
    char v4; // [esp+3h] [ebp-95h]
    GfxLightRegion *v5; // [esp+4h] [ebp-94h]
    unsigned int i; // [esp+80h] [ebp-18h]
    float diff[3]; // [esp+84h] [ebp-14h] BYREF
    unsigned int primaryLightIndex; // [esp+90h] [ebp-8h]
    const ComPrimaryLight *light; // [esp+94h] [ebp-4h]

    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        iassert((light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI));
        if (!Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize))
        {
            v5 = &rgp.world->lightRegion[primaryLightIndex];
            if (v5->hullCount)
            {
                Vec3Sub(boxMidPoint, light->origin, diff);
                for (i = 0; i < v5->hullCount; ++i)
                {
                    if (!R_CullBoxFromLightRegionHull(&v5->hulls[i], diff, boxHalfSize))
                    {
                        v4 = 0;
                        goto LABEL_17;
                    }
                }
                v4 = 1;
            }
            else
            {
                v4 = 0;
            }
        LABEL_17:
            if (!v4)
                return primaryLightIndex;
        }
    }
    return 0;
}

unsigned int __cdecl R_GetNonSunPrimaryLightForSphere(const GfxViewInfo *viewInfo, const float *origin, float radius)
{
    char v4; // [esp+7h] [ebp-89h]
    GfxLightRegion *v5; // [esp+8h] [ebp-88h]
    unsigned int i; // [esp+68h] [ebp-28h]
    float diff[7]; // [esp+6Ch] [ebp-24h] BYREF
    unsigned int primaryLightIndex; // [esp+88h] [ebp-8h]
    const ComPrimaryLight *light; // [esp+8Ch] [ebp-4h]

    for (primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
        primaryLightIndex < rgp.world->primaryLightCount;
        ++primaryLightIndex)
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if (light->type != 2 && light->type != 3)
            MyAssertHandler(
                ".\\r_primarylights.cpp",
                539,
                0,
                "%s\n\t(light->type) = %i",
                "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                light->type);
        if (!Com_CullSphereFromPrimaryLight(light, origin, radius))
        {
            v5 = &rgp.world->lightRegion[primaryLightIndex];
            if (v5->hullCount)
            {
                Vec3Sub(origin, light->origin, diff);
                for (i = 0; i < v5->hullCount; ++i)
                {
                    if (!R_CullSphereFromLightRegionHull(&v5->hulls[i], diff, radius))
                    {
                        v4 = 0;
                        goto LABEL_17;
                    }
                }
                v4 = 1;
            }
            else
            {
                v4 = 0;
            }
        LABEL_17:
            if (!v4)
                return primaryLightIndex;
        }
    }
    return 0;
}

char __cdecl R_CullSphereFromLightRegionHull(const GfxLightRegionHull *hull, const float *origin, float radius)
{
    float v4; // [esp+0h] [ebp-ACh]
    float v5; // [esp+4h] [ebp-A8h]
    float v6; // [esp+8h] [ebp-A4h]
    float v7; // [esp+Ch] [ebp-A0h]
    float v8; // [esp+10h] [ebp-9Ch]
    float v9; // [esp+14h] [ebp-98h]
    float v10; // [esp+18h] [ebp-94h]
    float v11; // [esp+1Ch] [ebp-90h]
    float v12; // [esp+20h] [ebp-8Ch]
    float v13; // [esp+24h] [ebp-88h]
    float v14; // [esp+28h] [ebp-84h]
    float v15; // [esp+2Ch] [ebp-80h]
    float v16; // [esp+30h] [ebp-7Ch]
    float v17; // [esp+34h] [ebp-78h]
    float v18; // [esp+38h] [ebp-74h]
    float v19; // [esp+3Ch] [ebp-70h]
    float v20; // [esp+40h] [ebp-6Ch]
    float v21; // [esp+44h] [ebp-68h]
    float v22; // [esp+48h] [ebp-64h]
    float v23; // [esp+4Ch] [ebp-60h]
    float v24; // [esp+54h] [ebp-58h]
    float v25; // [esp+5Ch] [ebp-50h]
    float v26; // [esp+64h] [ebp-48h]
    float v27; // [esp+6Ch] [ebp-40h]
    float v28; // [esp+74h] [ebp-38h]
    float v29; // [esp+7Ch] [ebp-30h]
    float v30; // [esp+84h] [ebp-28h]
    float v31; // [esp+8Ch] [ebp-20h]
    float v32; // [esp+94h] [ebp-18h]
    float v33; // [esp+9Ch] [ebp-10h]
    float originOnAxis; // [esp+A0h] [ebp-Ch]
    float originOnAxisa; // [esp+A0h] [ebp-Ch]
    float originOnAxisb; // [esp+A0h] [ebp-Ch]
    float originOnAxisc; // [esp+A0h] [ebp-Ch]
    float originOnAxisd; // [esp+A0h] [ebp-Ch]
    float originOnAxise; // [esp+A0h] [ebp-Ch]
    float originOnAxisf; // [esp+A0h] [ebp-Ch]
    unsigned int axisIter; // [esp+A8h] [ebp-4h]

    v33 = *origin - hull->kdopMidPoint[0];
    v23 = I_fabs(v33);
    v22 = radius + hull->kdopHalfSize[0];
    if (v23 >= (double)v22)
        return 1;
    v32 = origin[1] - hull->kdopMidPoint[1];
    v21 = I_fabs(v32);
    v20 = radius + hull->kdopHalfSize[1];
    if (v21 >= (double)v20)
        return 1;
    v31 = origin[2] - hull->kdopMidPoint[2];
    v19 = I_fabs(v31);
    v18 = radius + hull->kdopHalfSize[2];
    if (v19 >= (double)v18)
        return 1;
    originOnAxis = *origin + origin[1];
    v30 = originOnAxis - hull->kdopMidPoint[3];
    v17 = I_fabs(v30);
    v16 = radius + hull->kdopHalfSize[3];
    if (v17 >= (double)v16)
        return 1;
    originOnAxisa = *origin - origin[1];
    v29 = originOnAxisa - hull->kdopMidPoint[4];
    v15 = I_fabs(v29);
    v14 = radius + hull->kdopHalfSize[4];
    if (v15 >= (double)v14)
        return 1;
    originOnAxisb = *origin + origin[2];
    v28 = originOnAxisb - hull->kdopMidPoint[5];
    v13 = I_fabs(v28);
    v12 = radius + hull->kdopHalfSize[5];
    if (v13 >= (double)v12)
        return 1;
    originOnAxisc = *origin - origin[2];
    v27 = originOnAxisc - hull->kdopMidPoint[6];
    v11 = I_fabs(v27);
    v10 = radius + hull->kdopHalfSize[6];
    if (v11 >= (double)v10)
        return 1;
    originOnAxisd = origin[1] + origin[2];
    v26 = originOnAxisd - hull->kdopMidPoint[7];
    v9 = I_fabs(v26);
    v8 = radius + hull->kdopHalfSize[7];
    if (v9 >= (double)v8)
        return 1;
    originOnAxise = origin[1] - origin[2];
    v25 = originOnAxise - hull->kdopMidPoint[8];
    v7 = I_fabs(v25);
    v6 = radius + hull->kdopHalfSize[8];
    if (v7 >= (double)v6)
        return 1;
    for (axisIter = 0; axisIter < hull->axisCount; ++axisIter)
    {
        originOnAxisf = *origin * hull->axis[axisIter].dir[0]
            + origin[1] * hull->axis[axisIter].dir[1]
            + origin[2] * hull->axis[axisIter].dir[2];
        v24 = originOnAxisf - hull->axis[axisIter].midPoint;
        v5 = I_fabs(v24);
        v4 = radius + hull->axis[axisIter].halfSize;
        if (v5 >= (double)v4)
            return 1;
    }
    return 0;
}

bool __cdecl Com_CullSphereFromPrimaryLight(const ComPrimaryLight *light, const float *origin, float radius)
{
    float v4; // [esp+Ch] [ebp-10h]
    float diff[3]; // [esp+10h] [ebp-Ch] BYREF

    Vec3Sub(origin, light->origin, diff);
    v4 = Vec3LengthSq(diff);
    if (v4 >= (radius + light->radius) * (radius + light->radius))
        return 1;
    if (light->type == 2 && light->cosHalfFovExpanded >= 0.0)
        return CullSphereFromCone(light->origin, light->dir, light->cosHalfFovExpanded, origin, radius);
    return 0;
}

