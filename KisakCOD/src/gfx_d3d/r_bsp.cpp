#include "r_bsp.h"
#include "r_init.h"
#include "r_dvars.h"


//struct GfxWorld s_world    85b28080     gfx_d3d : r_bsp.obj

GfxWorld s_world;


void __cdecl R_InterpretSunLightParseParams(SunLightParseParams *sunParse)
{
    float *sunColorFromBsp; // [esp+0h] [ebp-8h]
    float *color; // [esp+4h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_bsp.cpp", 155, 0, "%s", "rgp.world");
    if (!rgp.world->sunLight)
        MyAssertHandler(".\\r_bsp.cpp", 156, 0, "%s", "rgp.world->sunLight");

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
    int v2; // [esp+8Ch] [ebp-8h]
    int v3; // [esp+90h] [ebp-4h]

    if (!sm_enable->current.enabled)
    {
        v2 = r_lightTweakSunDirection.integer + 44;
        v3 = r_lightTweakSunDirection.integer + 12;
        if (*(float *)(r_lightTweakSunDirection.integer + 44) != *(float *)(r_lightTweakSunDirection.integer + 12)
            || *(float *)(v2 + 4) != *(float *)(v3 + 4)
            || *(float *)(v2 + 8) != *(float *)(v3 + 8))
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
        Dvar_ClearModified(r_lightTweakSunLight);
    }
    Dvar_SetFloat((dvar_s *)r_lightTweakDiffuseFraction, sunParse->diffuseFraction);
    R_GetNormalizedColorFromDvar(r_lightTweakSunColor, sunParse->sunColor);
    sunParse->diffuseColorHasBeenSet = 1;
    if (r_lightTweakSunDirection.integer == -12)
        MyAssertHandler(".\\r_bsp.cpp", 148, 0, "%s", "r_lightTweakSunDirection->current.vector");
    v1 = r_lightTweakSunDirection.integer + 12;
    sunParse->angles[0] = *(float *)(r_lightTweakSunDirection.integer + 12);
    sunParse->angles[1] = *(float *)(v1 + 4);
    sunParse->angles[2] = *(float *)(v1 + 8);
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

    if (rgp.world)
        MyAssertHandler(".\\r_bsp.cpp", 303, 0, "%s", "!rgp.world");
    R_InitLightVisHistory(name);
    if (useFastFile->current.enabled)
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
    float saveDirection; // [esp+14h] [ebp-Ch]
    float saveDirection_4; // [esp+18h] [ebp-8h]
    float saveDirection_8; // [esp+1Ch] [ebp-4h]

    if ((*(_WORD *)(r_lightTweakSunDirection.integer + 8) & 0x1000) == 0)
        MyAssertHandler(
            ".\\r_bsp.cpp",
            239,
            0,
            "%s\n\t(r_lightTweakSunDirection->name) = %s",
            "(r_lightTweakSunDirection->flags & (1 << 12))",
            *(const char **)r_lightTweakSunDirection.integer);
    saveDirection = *(float *)(r_lightTweakSunDirection.integer + 12);
    saveDirection_4 = *(float *)(r_lightTweakSunDirection.integer + 16);
    saveDirection_8 = *(float *)(r_lightTweakSunDirection.integer + 20);
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
        (dvar_s *)r_lightTweakSunDirection.integer,
        sunParse->angles[0],
        sunParse->angles[1],
        sunParse->angles[2]);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakAmbient, r_lightTweakAmbient->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakDiffuseFraction, r_lightTweakDiffuseFraction->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunLight, r_lightTweakSunLight->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakAmbientColor, r_lightTweakAmbientColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunColor, r_lightTweakSunColor->current);
    Dvar_ChangeResetValue((dvar_s *)r_lightTweakSunDiffuseColor, r_lightTweakSunDiffuseColor->current);
    Dvar_ChangeResetValue(
        (dvar_s *)r_lightTweakSunDirection.integer,
        *(DvarValue *)(r_lightTweakSunDirection.integer + 12));
    if (savegame)
        Dvar_SetVec3((dvar_s *)r_lightTweakSunDirection.integer, saveDirection, saveDirection_4, saveDirection_8);
}

void R_InitDynamicData()
{
    R_InitStaticModelLighting();
}

void __cdecl R_SetWorldPtr_FastFile(const char *name)
{
    rgp.world = DB_FindXAssetHeader(ASSET_TYPE_GFXWORLD, name).gfxWorld;
    rgp.needSortMaterials = 1;
}

