#pragma once
#include "rb_backend.h"

struct GfxTrianglesDrawStream // sizeof=0x30
{                                       // ...
    unsigned int reflectionProbeCount;  // ...
    unsigned int lightmapCount;         // ...
    GfxTexture *reflectionProbeTextures; // ...
    GfxTexture *lightmapPrimaryTextures; // ...
    GfxTexture *lightmapSecondaryTextures; // ...
    GfxTexture *whiteTexture;           // ...
    const unsigned int *primDrawSurfPos; // ...
    const GfxTexture *reflectionProbeTexture; // ...
    const GfxTexture *lightmapPrimaryTexture; // ...
    const GfxTexture *lightmapSecondaryTexture; // ...
    unsigned int customSamplerFlags;    // ...
};

void __cdecl R_SetStreamSource(
    GfxCmdBufPrimState *primState,
    IDirect3DVertexBuffer9 *vb,
    unsigned int vertexOffset,
    unsigned int vertexStride);
void __cdecl R_HW_SetSamplerTexture(IDirect3DDevice9 *device, unsigned int samplerIndex, const GfxTexture *texture);
void __cdecl R_SetStreamsForBspSurface(GfxCmdBufPrimState *state, const srfTriangles_t *tris);
void __cdecl R_DrawBspDrawSurfsLit(
    const unsigned int *primDrawSurfPos,
    GfxCmdBufContext context,
    GfxCmdBufContext prepassContext);
void __cdecl R_DrawTrianglesLit(
    GfxTrianglesDrawStream *drawStream,
    GfxCmdBufPrimState *primState,
    GfxCmdBufPrimState *prepassPrimState);
void __cdecl R_DrawBspTris(GfxCmdBufPrimState *state, const srfTriangles_t *tris, unsigned int triCount);
int __cdecl R_ReadBspDrawSurfs(
    const unsigned int **primDrawSurfPos,
    const unsigned __int16 **list,
    unsigned int *count);
void __cdecl R_DrawBspDrawSurfs(const unsigned int *primDrawSurfPos, GfxCmdBufState *state);
void __cdecl R_DrawTriangles(GfxTrianglesDrawStream *drawStream, GfxCmdBufPrimState *state);