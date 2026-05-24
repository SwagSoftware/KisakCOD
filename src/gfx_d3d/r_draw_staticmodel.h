#pragma once
#include "rb_backend.h"
#include "rb_tess.h"

struct GfxStaticModelDrawStream // sizeof=0x1C
{                                       // ...
    const uint32_t *primDrawSurfPos; // ...
    const GfxTexture *reflectionProbeTexture; // ...
    uint32_t customSamplerFlags;    // ...
    XSurface *localSurf;
    uint32_t smodelCount;
    const uint16_t *smodelList;
    uint32_t reflectionProbeIndex;
};

void __cdecl R_DrawStaticModelSurfLit(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
int __cdecl R_GetNextStaticModelSurf(GfxStaticModelDrawStream *drawStream, XSurface **outSurf);
void __cdecl R_DrawStaticModelSurf(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelDrawSurfNonOptimized(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
void __cdecl R_SetStaticModelVertexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf);
void __cdecl R_DrawStaticModelDrawSurfPlacement(
    const GfxStaticModelDrawInst *smodelDrawInst,
    GfxCmdBufSourceState *source);
void __cdecl R_DrawStaticModelDrawSurfLightingNonOptimized(
    GfxStaticModelDrawStream *drawStream,
    GfxCmdBufContext context);

void __cdecl R_DrawStaticModelCachedSurfLit(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelCachedSurf(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_SetupCachedStaticModelLighting(GfxCmdBufSourceState *source);
int __cdecl R_ReadStaticModelPreTessDrawSurf(
    GfxReadCmdBuf *readCmdBuf,
    GfxStaticModelPreTessSurf *pretessSurf,
    uint32_t *firstIndex,
    uint32_t *count);
void __cdecl R_DrawStaticModelsPreTessDrawSurf(
    GfxStaticModelPreTessSurf pretessSurf,
    uint32_t firstIndex,
    uint32_t count,
    GfxCmdBufContext context);
void __cdecl R_DrawStaticModelsPreTessDrawSurfLighting(
    GfxStaticModelPreTessSurf pretessSurf,
    uint32_t firstIndex,
    uint32_t count,
    GfxCmdBufContext context);

void __cdecl R_DrawStaticModelSkinnedSurf(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelSkinnedSurfLit(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelsSkinnedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);

uint32_t __cdecl R_ReadPrimDrawSurfInt(GfxReadCmdBuf *cmdBuf);
const uint32_t *__cdecl R_ReadPrimDrawSurfData(GfxReadCmdBuf *cmdBuf, uint32_t count);