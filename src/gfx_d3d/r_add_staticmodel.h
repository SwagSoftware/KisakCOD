#pragma once
#include "r_gfx.h"
#include "r_bsp.h"

#define R_MAX_PRETESS_INDICES 0x100000

struct GfxSModelDrawSurfData // sizeof=0x18
{                                       // ...
    GfxDelayedCmdBuf delayedCmdBuf;
    GfxDrawSurfList drawSurfList;       // ...
};

void __cdecl R_AddDelayedStaticModelDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    struct XSurface *xsurf,
    uint8_t *list,
    uint32_t count);
void __cdecl R_EndDumpStaticModelLodInfo();
void __cdecl R_AddAllStaticModelSurfacesCamera();
void __cdecl R_SkinStaticModelsCameraForLod(
    const XModel *model,
    uint8_t primaryLightIndex,
    uint8_t *list,
    uint32_t count,
    uint32_t surfType,
    uint32_t lod,
    GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_SkinStaticModelsCamera(
    const XModel *model,
    uint8_t primaryLightIndex,
    uint16_t (*staticModelLodList)[4][128],
    uint16_t (*staticModelLodCount)[4],
    GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_SkinStaticModelsCameraForSurface(
    const XModel *model,
    uint8_t primaryLightIndex,
    uint16_t (*staticModelLodList)[128],
    uint16_t *staticModelLodCount,
    uint32_t surfType,
    GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_ShowCountsStaticModel(int smodelIndex, int lod);
void __cdecl R_DumpStaticModelLodInfo(const GfxStaticModelDrawInst *smodelDrawInst, float dist);
void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, const float dist);
void __cdecl R_SortAllStaticModelSurfacesCamera();
void __cdecl R_SortAllStaticModelSurfacesSunShadow();
void __cdecl R_AddAllStaticModelSurfacesSunShadow();
void __cdecl R_AddAllStaticModelSurfacesRangeSunShadow(uint32_t partitionIndex, uint32_t maxDrawSurfCount);
void __cdecl R_SkinStaticModelsShadowForLod(
    const XModel *model,
    uint8_t *list,
    uint32_t count,
    uint32_t surfType,
    uint32_t lod,
    GfxSModelDrawSurfData *surfData);
void __cdecl R_SkinStaticModelsShadow(
    const XModel *model,
    uint16_t (*staticModelLodList)[4][128],
    uint16_t (*staticModelLodCount)[4],
    GfxSModelDrawSurfData *surfData);
void __cdecl R_SkinStaticModelsShadowForSurface(
    const XModel *model,
    uint16_t (*staticModelLodList)[128],
    uint16_t *staticModelLodCount,
    uint32_t surfType,
    GfxSModelDrawSurfData *surfData);
void __cdecl R_AddAllStaticModelSurfacesSpotShadow(uint32_t spotShadowIndex, uint32_t primaryLightIndex);
