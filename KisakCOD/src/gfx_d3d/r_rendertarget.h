#pragma once
#include "r_rendercmds.h"

enum RenderTargetUsage : __int32
{                                       // ...
    RENDERTARGET_USAGE_DEPTH = 0x0,
    RENDERTARGET_USAGE_RENDER = 0x1,
    RENDERTARGET_USAGE_RENDER_SHARE_SCENE = 0x2,
    RENDERTARGET_USAGE_TEXTURE = 0x3,
};

void __cdecl AssertUninitializedRenderTarget(const GfxRenderTarget *renderTarget);
int __cdecl R_GetDepthStencilFormat(_D3DFORMAT renderTargetFormat);
void __cdecl R_InitRenderTargets();
void R_InitRenderTargets_PC();
void __cdecl R_ShareRenderTarget(GfxRenderTargetId idFrom, GfxRenderTargetId idTo);
void __cdecl R_InitFullscreenRenderTargetImage(
    int imageProgType,
    FullscreenType screenType,
    int picmip,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget);
void __cdecl R_GetFullScreenRes(FullscreenType screenType, int *fullscreenWidth, int *fullscreenHeight);
IDirect3DSurface9 *__cdecl R_AssignSingleSampleDepthStencilSurface();
void __cdecl R_InitRenderTargetImage(
    int imageProgType,
    unsigned __int16 width,
    unsigned __int16 height,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget);
void __cdecl R_AssignImageToRenderTargetColor(GfxRenderTargetSurface *surface, GfxImage *image);
void __cdecl R_InitShadowmapRenderTarget(
    int imageProgType,
    unsigned __int16 tileRes,
    unsigned __int16 tileRowCount,
    GfxRenderTarget *renderTarget);
void __cdecl R_InitAndTrackRenderTargetImage(
    int imageProgType,
    unsigned __int16 width,
    unsigned __int16 height,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget);
void __cdecl R_InitShadowCookieRenderTarget(GfxRenderTarget *renderTarget);
void __cdecl R_AssignShadowCookieDepthStencilSurface(GfxRenderTargetSurface *surface);
_D3DFORMAT __cdecl R_InitFrameBufferRenderTarget();
void __cdecl R_ShutdownRenderTargets();
const char *__cdecl R_RenderTargetName(GfxRenderTargetId renderTargetId);
