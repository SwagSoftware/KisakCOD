#include "r_rendertarget.h"
#include "r_init.h"
#include "r_image.h"
#include "r_dvars.h"
#include "rb_backend.h"
#include "rb_logfile.h"


//GfxRenderTarget *gfxRenderTargets 85b5db38     gfx_d3d : r_rendertarget.obj

void __cdecl AssertUninitializedRenderTarget(const GfxRenderTarget *renderTarget)
{
    if (!renderTarget)
        MyAssertHandler(".\\r_rendertarget.cpp", 140, 0, "%s", "renderTarget");
    if (renderTarget->image)
        MyAssertHandler(".\\r_rendertarget.cpp", 141, 0, "%s", "renderTarget->image == NULL");
    if (renderTarget->surface.color)
        MyAssertHandler(".\\r_rendertarget.cpp", 143, 0, "%s", "renderTarget->surface.color == NULL");
    if (renderTarget->surface.depthStencil)
        MyAssertHandler(".\\r_rendertarget.cpp", 144, 0, "%s", "renderTarget->surface.depthStencil == NULL");
    if (renderTarget->width)
        MyAssertHandler(".\\r_rendertarget.cpp", 146, 0, "%s", "renderTarget->width == 0");
    if (renderTarget->height)
        MyAssertHandler(".\\r_rendertarget.cpp", 147, 0, "%s", "renderTarget->height == 0");
}

bool __cdecl R_IsDepthStencilFormatOk(_D3DFORMAT renderTargetFormat, _D3DFORMAT depthStencilFormat)
{
    return dx.d3d9->CheckDeviceFormat(
        dx.adapterIndex,
        D3DDEVTYPE_HAL,
        D3DFMT_X8R8G8B8,
        2,
        D3DRTYPE_SURFACE,
        depthStencilFormat) >= 0
        && 
        dx.d3d9->CheckDepthStencilMatch(
            dx.adapterIndex,
            D3DDEVTYPE_HAL,
            D3DFMT_X8R8G8B8,
            renderTargetFormat,
            depthStencilFormat) >= 0;
}

int __cdecl R_GetDepthStencilFormat(_D3DFORMAT renderTargetFormat)
{
    if (R_IsDepthStencilFormatOk(renderTargetFormat, D3DFMT_D24FS8))
        return 83;
    else
        return 75;
}

void __cdecl R_InitRenderTargets()
{
    R_InitRenderTargets_PC();
}

void R_InitRenderTargets_PC()
{
    _D3DFORMAT backBufferFormat; // [esp+0h] [ebp-4h]

    backBufferFormat = R_InitFrameBufferRenderTarget();
    if (!g_allocateMinimalResources)
    {
        if (r_floatz->current.enabled)
            R_InitFullscreenRenderTargetImage(
                4,
                FULLSCREEN_SCENE,
                0,
                D3DFMT_R32F,
                RENDERTARGET_USAGE_RENDER,
                &gfxRenderTargets[5]);
        R_ShareRenderTarget(R_RENDERTARGET_RESOLVED_SCENE, R_RENDERTARGET_DYNAMICSHADOWS);
        R_ShareRenderTarget(R_RENDERTARGET_RESOLVED_SCENE, R_RENDERTARGET_RESOLVED_POST_SUN);
        R_InitFullscreenRenderTargetImage(
            10,
            FULLSCREEN_DISPLAY,
            0,
            backBufferFormat,
            RENDERTARGET_USAGE_TEXTURE,
            gfxRenderTargets);
        R_InitShadowmapRenderTarget(2, 0x400u, 2u, &gfxRenderTargets[13]);
        R_InitShadowmapRenderTarget(3, 0x200u, 4u, &gfxRenderTargets[14]);
        R_InitShadowCookieRenderTarget(&gfxRenderTargets[9]);
        R_InitShadowCookieBlurRenderTarget(&gfxRenderTargets[10]);
        R_InitFullscreenRenderTargetImage(
            5,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[11]);
        R_InitFullscreenRenderTargetImage(
            6,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[12]);
        R_InitFullscreenRenderTargetImage(
            7,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[7]);
        R_InitFullscreenRenderTargetImage(
            8,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[8]);
    }
}

void __cdecl R_ShareRenderTarget(GfxRenderTargetId idFrom, GfxRenderTargetId idTo)
{
    GfxRenderTarget *v2; // ecx
    GfxRenderTarget *v3; // edx

    AssertUninitializedRenderTarget(&gfxRenderTargets[idTo]);
    v2 = &gfxRenderTargets[idFrom];
    v3 = &gfxRenderTargets[idTo];
    v3->image = v2->image;
    v3->surface.color = v2->surface.color;
    v3->surface.depthStencil = v2->surface.depthStencil;
    v3->width = v2->width;
    v3->height = v2->height;
    if (gfxRenderTargets[idTo].surface.color)
        gfxRenderTargets[idTo].surface.color->AddRef();
    if (gfxRenderTargets[idTo].surface.depthStencil)
        gfxRenderTargets[idTo].surface.depthStencil->AddRef();
}

void __cdecl R_InitFullscreenRenderTargetImage(
    int imageProgType,
    FullscreenType screenType,
    int picmip,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget)
{
    unsigned __int16 v6; // [esp+0h] [ebp-20h]
    unsigned __int16 v7; // [esp+4h] [ebp-1Ch]
    int fullscreenWidth; // [esp+18h] [ebp-8h] BYREF
    int fullscreenHeight; // [esp+1Ch] [ebp-4h] BYREF

    R_GetFullScreenRes(screenType, &fullscreenWidth, &fullscreenHeight);
    if (fullscreenWidth >> picmip > 1)
        v7 = fullscreenWidth >> picmip;
    else
        v7 = 1;
    if (fullscreenHeight >> picmip > 1)
        v6 = fullscreenHeight >> picmip;
    else
        v6 = 1;
    R_InitRenderTargetImage(imageProgType, v7, v6, format, usage, renderTarget);
    if (usage == RENDERTARGET_USAGE_RENDER_SHARE_SCENE)
    {
        if (!alwaysfails)
            MyAssertHandler(
                ".\\r_rendertarget.cpp",
                751,
                0,
                "RENDERTARGET_USAGE_RENDER_SHARE_SCENE only implemented for XBOX");
    }
    else if (usage == RENDERTARGET_USAGE_RENDER)
    {
        renderTarget->surface.depthStencil = R_AssignSingleSampleDepthStencilSurface();
    }
    Image_TrackFullscreenTexture(renderTarget->image, fullscreenWidth, fullscreenHeight, picmip, format);
}

void __cdecl R_GetFullScreenRes(FullscreenType screenType, int *fullscreenWidth, int *fullscreenHeight)
{
    unsigned int sceneHeight; // [esp+0h] [ebp-8h]
    unsigned int sceneWidth; // [esp+4h] [ebp-4h]

    if ((unsigned int)screenType > FULLSCREEN_SCENE)
        MyAssertHandler(
            ".\\r_rendertarget.cpp",
            467,
            0,
            "%s\n\t(screenType) = %i",
            "(screenType == FULLSCREEN_DISPLAY || screenType == FULLSCREEN_MIXED || screenType == FULLSCREEN_SCENE)",
            screenType);
    if (screenType)
        sceneWidth = vidConfig.sceneWidth;
    else
        sceneWidth = vidConfig.displayWidth;
    *fullscreenWidth = sceneWidth;
    if (screenType == FULLSCREEN_SCENE)
        sceneHeight = vidConfig.sceneHeight;
    else
        sceneHeight = vidConfig.displayHeight;
    *fullscreenHeight = sceneHeight;
}

void __cdecl R_GetFrameBufferDepthStencilRes(int *depthStencilWidth, int *depthStencilHeight)
{
    *depthStencilWidth = vidConfig.displayWidth;
    *depthStencilHeight = vidConfig.displayHeight;
}

IDirect3DSurface9 *__cdecl R_AssignSingleSampleDepthStencilSurface()
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-Ch]
    int depthStencilWidth; // [esp+4h] [ebp-8h] BYREF
    int depthStencilHeight; // [esp+8h] [ebp-4h] BYREF

    if (!dx.singleSampleDepthStencilSurface && dx.multiSampleType == D3DMULTISAMPLE_NONE)
        dx.singleSampleDepthStencilSurface = gfxRenderTargets[1].surface.depthStencil;
    if (dx.singleSampleDepthStencilSurface)
    {
        dx.singleSampleDepthStencilSurface->AddRef();
        return dx.singleSampleDepthStencilSurface;
    }
    else
    {
        R_GetFrameBufferDepthStencilRes(&depthStencilWidth, &depthStencilHeight);
        hr = dx.device->CreateDepthStencilSurface(
            depthStencilWidth,
            depthStencilHeight,
            dx.depthStencilFormat,
            D3DMULTISAMPLE_NONE,
            0,
            0,
            &dx.singleSampleDepthStencilSurface,
            0);
        if (hr < 0)
        {
            v1 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "Couldn't create a %i x %i depth-stencil surface: %s\n",
                depthStencilWidth,
                depthStencilHeight,
                v1);
        }
        if (!dx.singleSampleDepthStencilSurface)
            MyAssertHandler(".\\r_rendertarget.cpp", 528, 0, "%s", "dx.singleSampleDepthStencilSurface");
        return dx.singleSampleDepthStencilSurface;
    }
}

void __cdecl R_AssignImageToRenderTargetDepthStencil(GfxRenderTargetSurface *surface, GfxImage *image)
{
    surface->depthStencil = Image_GetSurface(image);
}

void __cdecl R_InitRenderTargetImage(
    int imageProgType,
    unsigned __int16 width,
    unsigned __int16 height,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget)
{
    unsigned int renderTargetId; // [esp+0h] [ebp-4h]

    AssertUninitializedRenderTarget(renderTarget);
    if (!width)
        MyAssertHandler(".\\r_rendertarget.cpp", 679, 0, "%s\n\t(width) = %i", "(width > 0)", 0);
    if (!height)
        MyAssertHandler(".\\r_rendertarget.cpp", 680, 0, "%s\n\t(height) = %i", "(height > 0)", 0);
    renderTargetId = renderTarget - gfxRenderTargets;
    if (renderTargetId >= 0xF)
        MyAssertHandler(
            ".\\r_rendertarget.cpp",
            683,
            0,
            "renderTargetId doesn't index R_RENDERTARGET_COUNT\n\t%i not in [0, %i)",
            renderTargetId,
            15);
    renderTarget->image = Image_AllocProg(imageProgType, 6u, 0);
    if (!renderTarget->image)
        MyAssertHandler(".\\r_rendertarget.cpp", 686, 1, "%s", "renderTarget->image");
    Image_SetupRenderTarget(renderTarget->image, width, height, format);
    if (usage)
        R_AssignImageToRenderTargetColor(&renderTarget->surface, renderTarget->image);
    else
        R_AssignImageToRenderTargetDepthStencil(&renderTarget->surface, renderTarget->image);
    renderTarget->width = width;
    renderTarget->height = height;
}

void __cdecl R_AssignImageToRenderTargetColor(GfxRenderTargetSurface *surface, GfxImage *image)
{
    surface->color = Image_GetSurface(image);
}

void __cdecl R_InitShadowmapRenderTarget(
    int imageProgType,
    unsigned __int16 tileRes,
    unsigned __int16 tileRowCount,
    GfxRenderTarget *renderTarget)
{
    const char *v4; // eax
    const char *v5; // eax
    unsigned __int16 totalHeight; // [esp+0h] [ebp-10h]
    int hr; // [esp+4h] [ebp-Ch]
    int hra; // [esp+4h] [ebp-Ch]
    RenderTargetUsage usage; // [esp+8h] [ebp-8h]

    AssertUninitializedRenderTarget(renderTarget);
    totalHeight = tileRowCount * tileRes;
    if (((tileRes - 1) & tileRes) != 0)
        MyAssertHandler(
            ".\\r_rendertarget.cpp",
            802,
            0,
            "%s\n\t(totalWidth) = %i",
            "((((totalWidth) & ((totalWidth) - 1)) == 0))",
            tileRes);
    if (((totalHeight - 1) & totalHeight) != 0)
        MyAssertHandler(
            ".\\r_rendertarget.cpp",
            803,
            0,
            "%s\n\t(totalHeight) = %i",
            "((((totalHeight) & ((totalHeight) - 1)) == 0))",
            totalHeight);
    usage = (RenderTargetUsage)(gfxMetrics.shadowmapBuildTechType != TECHNIQUE_BUILD_SHADOWMAP_DEPTH);
    R_InitAndTrackRenderTargetImage(
        imageProgType,
        tileRes,
        totalHeight,
        gfxMetrics.shadowmapFormatPrimary,
        usage,
        renderTarget);
    if (usage)
    {
        hra = dx.device->CreateDepthStencilSurface(
            tileRes,
            totalHeight,
            gfxMetrics.shadowmapFormatSecondary,
            D3DMULTISAMPLE_NONE,
            0,
            0,
            &renderTarget->surface.depthStencil,
            0);
        if (hra < 0)
        {
            v5 = R_ErrorDescription(hra);
            Com_Error(ERR_FATAL, "Couldn't create a %i x %i depth-stencil surface: %s\n", tileRes, totalHeight, v5);
        }
    }
    else
    {
        hr = dx.device->CreateRenderTarget(
            tileRes,
            totalHeight,
            gfxMetrics.shadowmapFormatSecondary,
            D3DMULTISAMPLE_NONE,
            0,
            0,
            (IDirect3DSurface9 **)&renderTarget->surface,
            0);
        if (hr < 0)
        {
            v4 = R_ErrorDescription(hr);
            Com_Error(ERR_FATAL, "Couldn't create a %i x %i render target surface: %s\n", tileRes, totalHeight, v4);
        }
    }
}

void __cdecl R_InitAndTrackRenderTargetImage(
    int imageProgType,
    unsigned __int16 width,
    unsigned __int16 height,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget)
{
    R_InitRenderTargetImage(imageProgType, width, height, format, usage, renderTarget);
    Image_TrackTexture(renderTarget->image, 3, format, width, height, 1);
}

void __cdecl R_InitShadowCookieBlurRenderTarget(GfxRenderTarget *renderTarget)
{
    AssertUninitializedRenderTarget(renderTarget);
    R_InitAndTrackRenderTargetImage(1, 0x80u, 0x80u, D3DFMT_A8R8G8B8, RENDERTARGET_USAGE_RENDER, renderTarget);
    R_AssignShadowCookieDepthStencilSurface(&renderTarget->surface);
}

void __cdecl R_InitShadowCookieRenderTarget(GfxRenderTarget *renderTarget)
{
    AssertUninitializedRenderTarget(renderTarget);
    R_InitAndTrackRenderTargetImage(0, 0x80u, 0x80u, D3DFMT_A8R8G8B8, RENDERTARGET_USAGE_RENDER, renderTarget);
    if (!renderTarget->surface.color)
        MyAssertHandler(".\\r_rendertarget.cpp", 863, 1, "%s", "renderTarget->surface.color");
    R_AssignShadowCookieDepthStencilSurface(&renderTarget->surface);
}

void __cdecl R_AssignShadowCookieDepthStencilSurface(GfxRenderTargetSurface *surface)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-8h]
    _D3DFORMAT depthStencilFormat; // [esp+4h] [ebp-4h]

    if (gfxRenderTargets[9].surface.depthStencil)
    {
        surface->depthStencil = gfxRenderTargets[9].surface.depthStencil;
        surface->depthStencil->AddRef();
    }
    else
    {
        depthStencilFormat = (_D3DFORMAT)R_GetDepthStencilFormat(D3DFMT_A8R8G8B8);
        hr = dx.device->CreateDepthStencilSurface(
            128u,
            128u,
            depthStencilFormat,
            D3DMULTISAMPLE_NONE,
            0,
            0,
            &surface->depthStencil,
            0);
        if (hr < 0)
        {
            v1 = R_ErrorDescription(hr);
            Com_Error(ERR_FATAL, "Couldn't create a %i x %i depth-stencil surface: %s\n", 128, 128, v1);
        }
    }
}

const char *__cdecl R_DescribeFormat(_D3DFORMAT format)
{
    const char *result; // eax

    switch (format)
    {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_A8B8G8R8:
        result = "24-bit color with 8-bit alpha";
        break;
    case D3DFMT_R5G6B5:
        result = "16-bit color";
        break;
    case D3DFMT_A1R5G5B5:
        result = "15-bit color with 1-bit alpha";
        break;
    case D3DFMT_D16_LOCKABLE:
    case D3DFMT_D16:
        result = "16-bit depth without stencil";
        break;
    case D3DFMT_D15S1:
        result = "15-bit depth with 1-bit stencil";
        break;
    case D3DFMT_D24S8:
        result = "24-bit depth with 8-bit stencil";
        break;
    case D3DFMT_D24X8:
        result = "24-bit depth without stencil";
        break;
    default:
        result = va("unknown format 0x%08x", format);
        break;
    }
    return result;
}

void __cdecl R_InitFrameBufferRenderTarget_Win32(GfxRenderTarget *renderTarget)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int depthStencilWidth; // [esp+0h] [ebp-10h] BYREF
    int depthStencilHeight; // [esp+4h] [ebp-Ch] BYREF
    HRESULT v6; // [esp+8h] [ebp-8h]
    HRESULT hr; // [esp+Ch] [ebp-4h]

    if (!renderTarget)
        MyAssertHandler(".\\r_rendertarget.cpp", 886, 0, "%s", "renderTarget");
    renderTarget->width = vidConfig.displayWidth;
    renderTarget->height = vidConfig.displayHeight;
    hr = dx.device->GetSwapChain(0, &dx.windows[0].swapChain);
    if (hr < 0)
    {
        v1 = R_ErrorDescription(hr);
        Com_Error(ERR_FATAL, "Couldn't get an interface to the swap chain: %s\n", v1);
    }
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("dx.device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &renderTarget->surface.color )\n");
        v6 = dx.device->GetBackBuffer(
            0,
            0,
            D3DBACKBUFFER_TYPE_MONO,
            &renderTarget->surface.color);
        if (v6 < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(v6);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_rendertarget.cpp (%i) dx.device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &renderTarget->surface.col"
                    "or ) failed: %s\n",
                    895,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    if (!renderTarget->surface.color)
        MyAssertHandler(".\\r_rendertarget.cpp", 896, 0, "%s", "renderTarget->surface.color");
    if (g_allocateMinimalResources)
    {
        renderTarget->surface.depthStencil = 0;
    }
    else
    {
        R_GetFrameBufferDepthStencilRes(&depthStencilWidth, &depthStencilHeight);
        hr = dx.device->CreateDepthStencilSurface(
            depthStencilWidth,
            depthStencilHeight,
            dx.depthStencilFormat,
            dx.multiSampleType,
            dx.multiSampleQuality,
            0,
            &renderTarget->surface.depthStencil,
            0);
        if (hr < 0)
        {
            v3 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "Couldn't create a %i x %i depth-stencil surface: %s\n",
                depthStencilWidth,
                depthStencilHeight,
                v3);
        }
    }
}

_D3DFORMAT __cdecl R_InitFrameBufferRenderTarget()
{
    const char *v0; // eax
    const char *v1; // eax
    _D3DSURFACE_DESC surfaceDesc; // [esp+0h] [ebp-20h] BYREF

    R_InitFrameBufferRenderTarget_Win32(&gfxRenderTargets[1]);
    R_ShareRenderTarget(R_RENDERTARGET_FRAME_BUFFER, R_RENDERTARGET_SCENE);
    v0 = R_DescribeFormat(D3DFMT_A8R8G8B8);
    Com_Printf(8, "Requested frame buffer to be %s\n", v0);
    gfxRenderTargets[1].surface.color->GetDesc(&surfaceDesc);
    if (surfaceDesc.Format == D3DFMT_UNKNOWN)
        MyAssertHandler(".\\r_rendertarget.cpp", 1035, 0, "%s", "surfaceDesc.Format != D3DFMT_UNKNOWN");
    v1 = R_DescribeFormat(surfaceDesc.Format);
    Com_Printf(8, "DirectX returned a frame buffer that is %s\n", v1);
    if (!g_allocateMinimalResources)
        R_InitFullscreenRenderTargetImage(
            9,
            FULLSCREEN_SCENE,
            0,
            surfaceDesc.Format,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[4]);
    return surfaceDesc.Format;
}

void __cdecl R_ShutdownRenderTargets()
{
    int renderTargetId; // [esp+0h] [ebp-4h]

    for (renderTargetId = 0; renderTargetId < 15; ++renderTargetId)
    {
        if (gfxRenderTargets[renderTargetId].surface.color)
            gfxRenderTargets[renderTargetId].surface.color->Release();
        if (gfxRenderTargets[renderTargetId].surface.depthStencil)
            gfxRenderTargets[renderTargetId].surface.depthStencil->Release();
        if (gfxRenderTargets[renderTargetId].image)
            Image_Release(gfxRenderTargets[renderTargetId].image);
    }
    memset((unsigned __int8 *)gfxRenderTargets, 0, sizeof(gfxRenderTargets));
    dx.singleSampleDepthStencilSurface = 0;
}


const char *s_renderTargetNames[15] =
{
  "R_RENDERTARGET_SAVED_SCREEN",
  "R_RENDERTARGET_FRAME_BUFFER",
  "R_RENDERTARGET_SCENE",
  "R_RENDERTARGET_RESOLVED_POST_SUN",
  "R_RENDERTARGET_RESOLVED_SCENE",
  "R_RENDERTARGET_FLOAT_Z",
  "R_RENDERTARGET_DYNAMICSHADOWS",
  "R_RENDERTARGET_PINGPONG_0",
  "R_RENDERTARGET_PINGPONG_1",
  "R_RENDERTARGET_SHADOWCOOKIE",
  "R_RENDERTARGET_SHADOWCOOKIE_BLUR",
  "R_RENDERTARGET_POST_EFFECT_0",
  "R_RENDERTARGET_POST_EFFECT_1",
  "R_RENDERTARGET_SHADOWMAP_SUN",
  "R_RENDERTARGET_SHADOWMAP_SPOT"
}; // idb
const char *__cdecl R_RenderTargetName(GfxRenderTargetId renderTargetId)
{
    return s_renderTargetNames[renderTargetId];
}

