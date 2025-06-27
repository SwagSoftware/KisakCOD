#pragma once
#include "rb_backend.h"


int __cdecl R_SetupMaterial(
    GfxCmdBufContext context,
    GfxCmdBufContext *prepassContext,
    const GfxDrawSurfListInfo *info,
    GfxDrawSurf drawSurf);
int __cdecl R_SetPrepassMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, MaterialTechniqueType techType);
int __cdecl R_SetMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, MaterialTechniqueType techType);
void __cdecl R_SetGameTime(GfxCmdBufSourceState *source, float gameTime);
int __cdecl R_UpdateMaterialTime(GfxCmdBufSourceState *source, float materialTime);
