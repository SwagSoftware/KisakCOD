#pragma once
#include "r_gfx.h"
#include "r_bsp.h"

struct GfxSModelDrawSurfData // sizeof=0x18
{                                       // ...
    GfxDelayedCmdBuf delayedCmdBuf;
    GfxDrawSurfList drawSurfList;       // ...
};

void __cdecl R_AddDelayedStaticModelDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    struct XSurface *xsurf,
    unsigned __int8 *list,
    unsigned int count);
void __cdecl R_EndDumpStaticModelLodInfo();
void __cdecl R_AddAllStaticModelSurfacesCamera();
void __cdecl R_SkinStaticModelsCameraForLod(
    const XModel *model,
    unsigned __int8 primaryLightIndex,
    unsigned __int8 *list,
    unsigned int count,
    unsigned int surfType,
    unsigned int lod,
    GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_SkinStaticModelsCamera(
    const XModel *model,
    unsigned __int8 primaryLightIndex,
    unsigned __int16 (*staticModelLodList)[4][128],
    unsigned __int16 (*staticModelLodCount)[4],
    GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_SkinStaticModelsCameraForSurface(
    const XModel *model,
    unsigned __int8 primaryLightIndex,
    unsigned __int16 (*staticModelLodList)[128],
    unsigned __int16 *staticModelLodCount,
    unsigned int surfType,
    GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_ShowCountsStaticModel(int smodelIndex, int lod);
void __cdecl R_DumpStaticModelLodInfo(const GfxStaticModelDrawInst *smodelDrawInst, float dist);
void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, const float dist);
void __cdecl R_SortAllStaticModelSurfacesCamera();
void __cdecl R_SortAllStaticModelSurfacesSunShadow();
void __cdecl R_AddAllStaticModelSurfacesSunShadow();
void __cdecl R_AddAllStaticModelSurfacesRangeSunShadow(unsigned int partitionIndex, unsigned int maxDrawSurfCount);
void __cdecl R_SkinStaticModelsShadowForLod(
    const XModel *model,
    unsigned __int8 *list,
    unsigned int count,
    unsigned int surfType,
    unsigned int lod,
    GfxSModelDrawSurfData *surfData);
void __cdecl R_SkinStaticModelsShadow(
    const XModel *model,
    unsigned __int16 (*staticModelLodList)[4][128],
    unsigned __int16 (*staticModelLodCount)[4],
    GfxSModelDrawSurfData *surfData);
void __cdecl R_SkinStaticModelsShadowForSurface(
    const XModel *model,
    unsigned __int16 (*staticModelLodList)[128],
    unsigned __int16 *staticModelLodCount,
    unsigned int surfType,
    GfxSModelDrawSurfData *surfData);
void __cdecl R_AddAllStaticModelSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex);
