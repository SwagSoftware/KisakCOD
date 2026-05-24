#pragma once
#include "rb_backend.h"

struct GfxTrianglesDrawStream // sizeof=0x30
{                                       // ...
    uint32_t reflectionProbeCount;  // ...
    uint32_t lightmapCount;         // ...
    GfxTexture *reflectionProbeTextures; // ...
    GfxTexture *lightmapPrimaryTextures; // ...
    GfxTexture *lightmapSecondaryTextures; // ...
    GfxTexture *whiteTexture;           // ...
    const uint32_t *primDrawSurfPos; // ...
    const GfxTexture *reflectionProbeTexture; // ...
    const GfxTexture *lightmapPrimaryTexture; // ...
    const GfxTexture *lightmapSecondaryTexture; // ...
    uint32_t customSamplerFlags;    // ...
    int hasSunDirChanged;               // ...
};

void __cdecl R_SetStreamSource(
    GfxCmdBufPrimState *primState,
    IDirect3DVertexBuffer9 *vb,
    uint32_t vertexOffset,
    uint32_t vertexStride);
void __cdecl R_HW_SetSamplerTexture(IDirect3DDevice9 *device, uint32_t samplerIndex, const GfxTexture *texture);
void __cdecl R_SetStreamsForBspSurface(GfxCmdBufPrimState *state, const srfTriangles_t *tris);
void __cdecl R_DrawBspDrawSurfsLit(
    const uint32_t *primDrawSurfPos,
    GfxCmdBufContext context,
    GfxCmdBufContext prepassContext);
void __cdecl R_DrawTrianglesLit(
    GfxTrianglesDrawStream *drawStream,
    GfxCmdBufPrimState *primState,
    GfxCmdBufPrimState *prepassPrimState);
void __cdecl R_DrawBspTris(GfxCmdBufPrimState *state, const srfTriangles_t *tris, uint32_t triCount);
int __cdecl R_ReadBspDrawSurfs(
    const uint32_t **primDrawSurfPos,
    const uint16_t **list,
    uint32_t *count);
void __cdecl R_DrawBspDrawSurfs(const uint32_t *primDrawSurfPos, GfxCmdBufState *state);
void __cdecl R_DrawTriangles(GfxTrianglesDrawStream *drawStream, GfxCmdBufPrimState *state);

void __cdecl R_DrawPreTessTris(
    GfxCmdBufPrimState *state,
    const srfTriangles_t *tris,
    uint32_t baseIndex,
    uint32_t triCount);

void __cdecl R_DrawBspDrawSurfsPreTess(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawBspDrawSurfsLitPreTess(const uint32_t *primDrawSurfPos, GfxCmdBufContext context);