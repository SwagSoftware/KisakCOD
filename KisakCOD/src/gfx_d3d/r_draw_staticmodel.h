#pragma once
#include "rb_backend.h"

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
