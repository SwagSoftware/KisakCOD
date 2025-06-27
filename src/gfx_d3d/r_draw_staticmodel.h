#pragma once
#include "rb_backend.h"
#include "rb_tess.h"

struct GfxStaticModelDrawStream // sizeof=0x1C
{                                       // ...
    const unsigned int *primDrawSurfPos; // ...
    const GfxTexture *reflectionProbeTexture; // ...
    unsigned int customSamplerFlags;    // ...
    XSurface *localSurf;
    unsigned int smodelCount;
    const unsigned __int16 *smodelList;
    unsigned int reflectionProbeIndex;
};

void __cdecl R_DrawStaticModelSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
int __cdecl R_GetNextStaticModelSurf(GfxStaticModelDrawStream *drawStream, XSurface **outSurf);
void __cdecl R_DrawStaticModelSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelDrawSurfNonOptimized(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
void __cdecl R_SetStaticModelVertexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf);
void __cdecl R_DrawStaticModelDrawSurfPlacement(
    const GfxStaticModelDrawInst *smodelDrawInst,
    GfxCmdBufSourceState *source);
void __cdecl R_DrawStaticModelDrawSurfLightingNonOptimized(
    GfxStaticModelDrawStream *drawStream,
    GfxCmdBufContext context);

void __cdecl R_DrawStaticModelCachedSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelCachedSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_SetupCachedStaticModelLighting(GfxCmdBufSourceState *source);
int __cdecl R_ReadStaticModelPreTessDrawSurf(
    GfxReadCmdBuf *readCmdBuf,
    GfxStaticModelPreTessSurf *pretessSurf,
    unsigned int *firstIndex,
    unsigned int *count);
void __cdecl R_DrawStaticModelsPreTessDrawSurf(
    GfxStaticModelPreTessSurf pretessSurf,
    unsigned int firstIndex,
    unsigned int count,
    GfxCmdBufContext context);
void __cdecl R_DrawStaticModelsPreTessDrawSurfLighting(
    GfxStaticModelPreTessSurf pretessSurf,
    unsigned int firstIndex,
    unsigned int count,
    GfxCmdBufContext context);

void __cdecl R_DrawStaticModelSkinnedSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelSkinnedSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelsSkinnedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);

unsigned int __cdecl R_ReadPrimDrawSurfInt(GfxReadCmdBuf *cmdBuf);
const unsigned int *__cdecl R_ReadPrimDrawSurfData(GfxReadCmdBuf *cmdBuf, unsigned int count);