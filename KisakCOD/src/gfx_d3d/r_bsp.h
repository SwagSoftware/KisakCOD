#pragma once
#include "r_gfx.h"
#include <universal/q_shared.h>



// r_bsp
void __cdecl R_InterpretSunLightParseParams(SunLightParseParams *sunParse);
void __cdecl R_UpdateLightsFromDvars();
void __cdecl R_CopyParseParamsFromDvars(SunLightParseParams *sunParse);
void __cdecl R_GetNormalizedColorFromDvar(const dvar_s *dvar, float *outVec);
void __cdecl R_LoadWorld(char *name, int *checksum, int savegame);
void __cdecl R_CopyParseParamsToDvars(const SunLightParseParams *sunParse, int savegame);
void R_InitDynamicData();
void __cdecl R_SetWorldPtr_FastFile(const char *name);



// r_bsp_load_obj
void __cdecl R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *sunParse, GfxLight *sunLight);
void __cdecl R_SetUpSunLight(const float *sunColor, const float *sunDirection, GfxLight *light);
void __cdecl R_InitPrimaryLights(GfxLight *primaryLights);
void __cdecl R_AddShadowSurfaceToPrimaryLight(
    GfxWorld *world,
    unsigned int primaryLightIndex,
    unsigned int sortedSurfIndex);
void __cdecl R_ForEachPrimaryLightAffectingSurface(
    GfxWorld *world,
    const GfxSurface *surface,
    unsigned int sortedSurfIndex,
    void(__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int));



// r_add_bsp
struct GfxSModelDrawSurfLightingData // sizeof=0x28
{                                       // ...
    GfxDelayedCmdBuf delayedCmdBuf;
    GfxDrawSurfList drawSurf[3];        // ...
};
void __cdecl R_InitBspDrawSurf(GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_AddBspDrawSurfs(
    GfxDrawSurf drawSurf,
    unsigned __int8 *list,
    unsigned int count,
    GfxBspDrawSurfData *surfData);
void __cdecl R_AddAllBspDrawSurfacesCamera();
void __cdecl R_AddAllBspDrawSurfacesRangeCamera(
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int stage,
    unsigned int maxDrawSurfCount);
void __cdecl R_AddAllBspDrawSurfacesCameraNonlit(
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int stage);
void __cdecl R_AddAllBspDrawSurfacesSunShadow();
void __cdecl R_AddAllBspDrawSurfacesRangeSunShadow(
    unsigned int partitionIndex,
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int maxDrawSurfCount);
void __cdecl R_AddAllBspDrawSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex);


// r_add_cmdbuf
void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
void __cdecl R_EndCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
int __cdecl R_AllocDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    GfxDrawSurf drawSurf,
    GfxDrawSurfList *drawSurfList,
    unsigned int size);
void __cdecl R_WritePrimDrawSurfInt(GfxDelayedCmdBuf *delayedCmdBuf, unsigned int value);
void __cdecl R_WritePrimDrawSurfData(GfxDelayedCmdBuf *delayedCmdBuf, unsigned __int8 *data, unsigned int count);
