#pragma once
#include "rb_backend.h"


void __cdecl R_DrawXModelRigidModelSurf(GfxCmdBufContext context, XSurface *xsurf);

unsigned int __cdecl R_DrawXModelRigidSurf(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context);

unsigned int __cdecl R_DrawXModelRigidSurfCamera(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context);

unsigned int __cdecl R_DrawXModelRigidSurfLit(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context);