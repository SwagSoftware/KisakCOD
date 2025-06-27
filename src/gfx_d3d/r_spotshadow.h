#pragma once
#include "r_rendercmds.h"
#include "r_scene.h"

struct GfxSpotShadowEntCmd // sizeof=0x8
{                                       // ...
    GfxSceneEntity *sceneEnt;           // ...
    const GfxLight *light;              // ...
};

void __cdecl R_AddSpotShadowEntCmd(const GfxSpotShadowEntCmd *data);
char __cdecl R_AddSpotShadowsForLight(
    GfxViewInfo *viewInfo,
    GfxLight *light,
    unsigned int shadowableLightIndex,
    float spotShadowFade);
void __cdecl R_SetViewParmsForLight(const GfxLight *light, GfxViewParms *viewParms, float nearPlaneBias);
void __cdecl R_GetSpotShadowLookupMatrix(
    const GfxViewParms *shadowViewParms,
    unsigned int spotShadowIndex,
    unsigned int tileCount,
    GfxMatrix *lookupMatrix);
void __cdecl R_AddSpotShadowModelEntities(
    unsigned int localClientNum,
    unsigned int primaryLightIndex,
    const GfxLight *light);
void __cdecl R_GenerateAllSortedSpotShadowDrawSurfs(GfxViewInfo *viewInfo);
void __cdecl R_GenerateSortedPrimarySpotShadowDrawSurfs(
    const GfxViewInfo *viewInfo,
    unsigned int spotShadowIndex,
    unsigned int shadowableLightIndex);
void __cdecl R_EmitSpotShadowMapSurfs(GfxViewInfo *viewInfo);


unsigned int R_InitSpotShadowMeshes();
void __cdecl R_ShutdownSpotShadowMeshes();
void RB_SpotShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo);