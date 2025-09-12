#include "r_bsp.h"
#include "r_init.h"
#include "r_dvars.h"
#include "r_model_lighting.h"
#include "r_staticmodelcache.h"
#include "rb_logfile.h"
#include "r_buffers.h"
#include "rb_light.h"
#include "r_dpvs.h"
#include "r_shadowcookie.h"
#include "r_sky.h"
#include <database/database.h>


//struct GfxWorld s_world    85b28080     gfx_d3d : r_bsp.obj // LWSS: moved to db_registry for DEDICATED


void __cdecl R_ReloadWorld()
{
    iassert( s_world.vd.worldVb == NULL_VERTEX_BUFFER );
    if (s_world.vertexCount)
        R_CreateWorldVertexBuffer(&s_world.vd.worldVb, (int *)s_world.vd.vertices, 44 * s_world.vertexCount);
    iassert( s_world.vld.layerVb == NULL_VERTEX_BUFFER );
    if (s_world.vertexLayerDataSize)
        R_CreateWorldVertexBuffer(&s_world.vld.layerVb, (int *)s_world.vld.data, s_world.vertexLayerDataSize);
}

void __cdecl R_ShutdownWorld()
{
    R_ReleaseWorld();
    rgp.world = 0;
    iassert( s_world.vd.worldVb == NULL_VERTEX_BUFFER );
    iassert( s_world.vld.layerVb == NULL_VERTEX_BUFFER );
    s_world.vertexCount = 0;
    s_world.vertexLayerDataSize = 0;
}

void __cdecl R_ReleaseWorld()
{
    if (rgp.world)
    {
        R_ResetModelLighting();
        R_FlushStaticModelCache();
    }
    if (s_world.vertexCount)
    {
        iassert( s_world.vd.worldVb != NULL_VERTEX_BUFFER );
        R_FreeStaticVertexBuffer(s_world.vd.worldVb);
        s_world.vd.worldVb = 0;
    }
    iassert( s_world.vd.worldVb == NULL_VERTEX_BUFFER );
    if (s_world.vertexLayerDataSize)
    {
        iassert( s_world.vld.layerVb != NULL_VERTEX_BUFFER );
        R_FreeStaticVertexBuffer(s_world.vld.layerVb);
        s_world.vld.layerVb = 0;
    }
    iassert( s_world.vld.layerVb == NULL_VERTEX_BUFFER );
}

void __cdecl R_InterpretSunLightParseParams(SunLightParseParams *sunParse)
{
    float *sunColorFromBsp; // [esp+0h] [ebp-8h]
    float *color; // [esp+4h] [ebp-4h]

    iassert( rgp.world );
    iassert( rgp.world->sunLight );

    R_InterpretSunLightParseParamsIntoLights(sunParse, rgp.world->sunLight);

    sunColorFromBsp = rgp.world->sunColorFromBsp;
    color = rgp.world->sunLight->color;
    rgp.world->sunColorFromBsp[0] = *color;
    sunColorFromBsp[1] = color[1];
    sunColorFromBsp[2] = color[2];
}

void __cdecl R_UpdateLightsFromDvars()
{
    SunLightParseParams sunParse; // [esp+8h] [ebp-8Ch] BYREF

    if (!sm_enable->current.enabled)
    {
        if (r_lightTweakSunDirection->reset.vector[0] != r_lightTweakSunDirection->current.vector[0]
            || r_lightTweakSunDirection->reset.vector[1] != r_lightTweakSunDirection->current.vector[1]
            || r_lightTweakSunDirection->reset.vector[2] != r_lightTweakSunDirection->current.vector[2])
        {
            Dvar_SetFloat((dvar_s *)sm_sunSampleSizeNear, 1.0);
            Dvar_SetBool((dvar_s *)sm_enable, 1);
        }
    }
    R_CopyParseParamsFromDvars(&sunParse);
    R_InterpretSunLightParseParams(&sunParse);
}

void __cdecl R_CopyParseParamsFromDvars(SunLightParseParams *sunParse)
{
    int v1; // [esp+10h] [ebp-8h]
    float sunLightMin; // [esp+14h] [ebp-4h]

    memcpy(sunParse, &rgp.world->sunParse, sizeof(SunLightParseParams));
    sunLightMin = (sunParse->sunLight - sunParse->ambientScale) * sunParse->diffuseFraction + sunParse->ambientScale;
    sunParse->sunLight = r_lightTweakSunLight->current.value;
    if (sunLightMin < (double)sunParse->sunLight)
    {
        sunParse->diffuseFraction = (sunLightMin - sunParse->ambientScale) / (sunParse->sunLight - sunParse->ambientScale);
    }
    else
    {
        sunParse->sunLight = sunLightMin;
        sunParse->diffuseFraction = 1.0;
        Dvar_SetFloat((dvar_s *)r_lightTweakSunLight, sunLightMin);
        Dvar_ClearModified((dvar_s*)r_lightTweakSunLight);
    }

    Dvar_SetFloat((dvar_s *)r_lightTweakDiffuseFraction, sunParse->diffuseFraction);
    R_GetNormalizedColorFromDvar(r_lightTweakSunColor, sunParse->sunColor);
    sunParse->diffuseColorHasBeenSet = 1;
    iassert(r_lightTweakSunDirection->current.vector);
    sunParse->angles[0] = r_lightTweakSunDirection->current.vector[0];
    sunParse->angles[1] = r_lightTweakSunDirection->current.vector[1];
    sunParse->angles[2] = r_lightTweakSunDirection->current.vector[2];
}

void __cdecl R_GetNormalizedColorFromDvar(const dvar_s *dvar, float *outVec)
{
    int channelIter; // [esp+4h] [ebp-4h]

    for (channelIter = 0; channelIter != 3; ++channelIter)
        outVec[channelIter] = (float)dvar->current.color[channelIter];
    ColorNormalize(outVec, outVec);
}

void __cdecl R_LoadWorld(char *name, int *checksum, int savegame)
{
    SunLightParseParams sunParse; // [esp+4h] [ebp-90h] BYREF
    unsigned int reflectionProbeIndex; // [esp+8Ch] [ebp-8h]
    int lightmapIndex; // [esp+90h] [ebp-4h]

    iassert( !rgp.world );
    R_InitLightVisHistory(name);
    if (IsFastFileLoad())
        R_SetWorldPtr_FastFile(name);
    else
        R_SetWorldPtr_LoadObj(name);
    if (checksum)
        *checksum = rgp.world->checksum;
    R_CopyParseParamsToDvars(&rgp.world->sunParse, savegame);
    R_CopyParseParamsFromDvars(&sunParse);
    R_InterpretSunLightParseParams(&sunParse);
    R_FlushSun();
    R_ResetShadowCookies();
    R_InitDynamicData();
    R_ResetModelLighting();
    RB_SetBspImages();
    DynEntCl_InitFilter();
    R_GenerateShadowMapCasterCells();
    for (reflectionProbeIndex = 0; reflectionProbeIndex < rgp.world->reflectionProbeCount; ++reflectionProbeIndex)
        rgp.world->reflectionProbeTextures[reflectionProbeIndex].basemap = rgp.world->reflectionProbes[reflectionProbeIndex].reflectionImage->texture.basemap;
    for (lightmapIndex = 0; lightmapIndex < rgp.world->lightmapCount; ++lightmapIndex)
    {
        rgp.world->lightmapPrimaryTextures[lightmapIndex].basemap = rgp.world->lightmaps[lightmapIndex].primary->texture.basemap;
        rgp.world->lightmapSecondaryTextures[lightmapIndex].basemap = rgp.world->lightmaps[lightmapIndex].secondary->texture.basemap;
    }
}

void __cdecl R_CopyParseParamsToDvars(const SunLightParseParams *sunParse, int savegame)
{
    float saveDirection[3];

    iassert(r_lightTweakSunDirection->flags & (1 << 12));
    saveDirection[0] = r_lightTweakSunDirection->current.vector[0];
    saveDirection[1] = r_lightTweakSunDirection->current.vector[1];
    saveDirection[2] = r_lightTweakSunDirection->current.vector[2];
    Dvar_SetFloat((dvar_s *)r_lightTweakAmbient, sunParse->ambientScale);
    Dvar_SetFloat((dvar_s *)r_lightTweakDiffuseFraction, sunParse->diffuseFraction);
    Dvar_SetFloat((dvar_s *)r_lightTweakSunLight, sunParse->sunLight);
    Dvar_SetColor(
        (dvar_s *)r_lightTweakAmbientColor,
        sunParse->ambientColor[0],
        sunParse->ambientColor[1],
        sunParse->ambientColor[2],
        1.0);
    Dvar_SetColor(
        (dvar_s *)r_lightTweakSunColor,
        sunParse->sunColor[0],
        sunParse->sunColor[1],
        sunParse->sunColor[2],
        1.0);
    Dvar_SetColor(
        (dvar_s *)r_lightTweakSunDiffuseColor,
        sunParse->diffuseColor[0],
        sunParse->diffuseColor[1],
        sunParse->diffuseColor[2],
        1.0);
    Dvar_SetVec3(
        (dvar_s *)r_lightTweakSunDirection,
        sunParse->angles[0],
        sunParse->angles[1],
        sunParse->angles[2]);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakAmbient, r_lightTweakAmbient->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakDiffuseFraction, r_lightTweakDiffuseFraction->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunLight, r_lightTweakSunLight->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakAmbientColor, r_lightTweakAmbientColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunColor, r_lightTweakSunColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunDiffuseColor, r_lightTweakSunDiffuseColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunDirection, r_lightTweakSunDirection->current);
    if (savegame)
        Dvar_SetVec3((dvar_s *)r_lightTweakSunDirection, saveDirection[0], saveDirection[1], saveDirection[2]);
}

void R_InitDynamicData()
{
    R_InitStaticModelLighting();
}

void __cdecl R_SetWorldPtr_LoadObj(const char *name)
{
    rgp.world = R_LoadWorldInternal(name);
    iassert( rgp.world );
}

void __cdecl R_SetWorldPtr_FastFile(const char *name)
{
    rgp.world = DB_FindXAssetHeader(ASSET_TYPE_GFXWORLD, name).gfxWorld;
    rgp.needSortMaterials = 1;
}

void R_SetSunLightOverride(float *sunColor)
{
    iassert(sunColor);

    rg.useSunLightOverride = 1;
    rg.sunLightOverride[0] = sunColor[0];
    rg.sunLightOverride[1] = sunColor[1];
    rg.sunLightOverride[2] = sunColor[2];
}

void R_ResetSunLightOverride()
{
    rg.useSunLightOverride = 0;
}

void R_SetSunDirectionOverride(float *sunDir)
{
    iassert(sunDir);
    rg.useSunDirOverride = 1;
    rg.useSunDirLerp = 0;
    rg.sunDirOverride[0] = sunDir[0];
    rg.sunDirOverride[1] = sunDir[1];
    rg.sunDirOverride[2] = sunDir[2];
}

void R_LerpSunDirectionOverride(float *sunDirBegin, float *sunDirEnd, int lerpBeginTime, int lerpEndTime)
{
    iassert(sunDirBegin);
    iassert(sunDirEnd);

    rg.useSunDirOverride = 1;
    rg.useSunDirLerp = 1;
    rg.sunDirOverride[0] = sunDirBegin[0];
    rg.sunDirOverride[1] = sunDirBegin[1];
    rg.sunDirOverride[2] = sunDirBegin[2];
    rg.sunDirOverrideTarget[0] = *sunDirEnd;
    rg.sunDirOverrideTarget[1] = sunDirEnd[1];
    rg.sunDirOverrideTarget[2] = sunDirEnd[2];
    rg.sunDirLerpBeginTime = lerpBeginTime;
    rg.sunDirLerpEndTime = lerpEndTime;
}

void R_ResetSunDirectionOverride()
{
    rg.useSunDirOverride = 0;
}

void R_ResetSunLightParseParams()
{
    R_UpdateLightsFromDvars();
}

unsigned int R_GetDebugReflectionProbeLocs(float (*locArray)[3], unsigned int maxCount)
{
    unsigned int result; // r3
    int v4; // r9
    unsigned int v5; // r10
    GfxReflectionProbe *v6; // r8

    result = maxCount;
    if (s_world.reflectionProbeCount - 1 < maxCount)
        result = s_world.reflectionProbeCount - 1;
    if (result)
    {
        v4 = 0;
        v5 = result;
        do
        {
            --v5;
            v6 = &s_world.reflectionProbes[++v4];
            (*locArray)[0] = v6->origin[0];
            (*locArray)[1] = v6->origin[1];
            (*locArray++)[2] = v6->origin[2];
        } while (v5);
    }
    return result;
}