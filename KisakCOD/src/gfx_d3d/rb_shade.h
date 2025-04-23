#pragma once
#include "rb_backend.h"


void __cdecl R_SetVertexDecl(GfxCmdBufPrimState *primState, const MaterialVertexDeclaration *vertexDecl);
void __cdecl RB_ClearPixelShader();
void __cdecl R_HW_SetPixelShader(IDirect3DDevice9 *device, const MaterialPixelShader *mtlShader);
void __cdecl RB_ClearVertexShader();
void __cdecl R_HW_SetVertexShader(IDirect3DDevice9 *device, const MaterialVertexShader *mtlShader);
void __cdecl RB_ClearVertexDecl();
void __cdecl RB_SetTessTechnique(const Material *material, MaterialTechniqueType techType);
void __cdecl RB_BeginSurface(const Material *material, MaterialTechniqueType techType);
void __cdecl RB_EndTessSurface();
GfxPrimStats *RB_EndSurfacePrologue();
void RB_EndSurfaceEpilogue();
void RB_DrawTessSurface();
void __cdecl R_DrawTessTechnique(GfxCmdBufContext context, const GfxDrawPrimArgs *args);
void __cdecl RB_TessOverflow();

