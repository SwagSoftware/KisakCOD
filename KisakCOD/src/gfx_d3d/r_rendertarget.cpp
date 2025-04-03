#include "r_rendertarget.h"
#include "r_init.h"
#include "r_image.h"


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
    return ((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, int, int, int, _D3DFORMAT))dx.d3d9->CheckDeviceFormat)(
        dx.d3d9,
        dx.d3d9,
        dx.adapterIndex,
        1,
        22,
        2,
        1,
        depthStencilFormat) >= 0
        && dx.d3d9->CheckDepthStencilMatch(
            dx.d3d9,
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
            R_InitFullscreenRenderTargetImage(4, FULLSCREEN_SCENE, 0, D3DFMT_R32F, RENDERTARGET_USAGE_RENDER, &renderTarget);
        R_ShareRenderTarget(R_RENDERTARGET_RESOLVED_SCENE, R_RENDERTARGET_DYNAMICSHADOWS);
        R_ShareRenderTarget(R_RENDERTARGET_RESOLVED_SCENE, R_RENDERTARGET_RESOLVED_POST_SUN);
        R_InitFullscreenRenderTargetImage(
            10,
            FULLSCREEN_DISPLAY,
            0,
            backBufferFormat,
            RENDERTARGET_USAGE_TEXTURE,
            gfxRenderTargets);
        R_InitShadowmapRenderTarget(2, 0x400u, 2u, &stru_EA74FF4);
        R_InitShadowmapRenderTarget(3, 0x200u, 4u, &stru_EA75008);
        R_InitShadowCookieRenderTarget(&stru_EA74FA4);
        R_InitShadowCookieBlurRenderTarget(&stru_EA74FB8);
        R_InitFullscreenRenderTargetImage(
            5,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &stru_EA74FCC);
        R_InitFullscreenRenderTargetImage(
            6,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &stru_EA74FE0);
        R_InitFullscreenRenderTargetImage(
            7,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &stru_EA74F7C);
        R_InitFullscreenRenderTargetImage(
            8,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &stru_EA74F90);
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
    if (dword_EA74EF4[5 * idTo])
        (*(void(__stdcall **)(int))(*(unsigned int *)dword_EA74EF4[5 * idTo] + 4))(dword_EA74EF4[5 * idTo]);
    if (dword_EA74EF8[5 * idTo])
        (*(void(__stdcall **)(int))(*(unsigned int *)dword_EA74EF8[5 * idTo] + 4))(dword_EA74EF8[5 * idTo]);
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
        renderTarget->surface.depthStencil = R_AssignSingleSampleDepthStencilSurface(screenType);
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

IDirect3DSurface9 *__cdecl R_AssignSingleSampleDepthStencilSurface()
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-Ch]
    int depthStencilWidth; // [esp+4h] [ebp-8h] BYREF
    int depthStencilHeight; // [esp+8h] [ebp-4h] BYREF

    if (!dx.singleSampleDepthStencilSurface && dx.multiSampleType == D3DMULTISAMPLE_NONE)
        dx.singleSampleDepthStencilSurface = stru_EA74F04.surface.depthStencil;
    if (dx.singleSampleDepthStencilSurface)
    {
        dx.singleSampleDepthStencilSurface->AddRef(dx.singleSampleDepthStencilSurface);
        return dx.singleSampleDepthStencilSurface;
    }
    else
    {
        R_GetFrameBufferDepthStencilRes(&depthStencilWidth, &depthStencilHeight);
        hr = dx.device->CreateDepthStencilSurface(
            dx.device,
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
    usage = gfxMetrics.shadowmapBuildTechType != TECHNIQUE_BUILD_SHADOWMAP_DEPTH;
    R_InitAndTrackRenderTargetImage(
        imageProgType,
        tileRes,
        totalHeight,
        gfxMetrics.shadowmapFormatPrimary,
        usage,
        renderTarget);
    if (usage)
    {
        hra = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, unsigned int, _D3DFORMAT, unsigned int, unsigned int, unsigned int, IDirect3DSurface9 **, unsigned int))dx.device->CreateDepthStencilSurface)(
            dx.device,
            dx.device,
            tileRes,
            totalHeight,
            gfxMetrics.shadowmapFormatSecondary,
            0,
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
            dx.device,
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

    if (stru_EA74FA4.surface.depthStencil)
    {
        surface->depthStencil = stru_EA74FA4.surface.depthStencil;
        surface->depthStencil->AddRef(surface->depthStencil);
    }
    else
    {
        depthStencilFormat = R_GetDepthStencilFormat(D3DFMT_A8R8G8B8);
        hr = dx.device->CreateDepthStencilSurface(
            dx.device,
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

_D3DFORMAT __cdecl R_InitFrameBufferRenderTarget()
{
    const char *v0; // eax
    const char *v1; // eax
    _D3DSURFACE_DESC surfaceDesc; // [esp+0h] [ebp-20h] BYREF

    R_InitFrameBufferRenderTarget_Win32(&stru_EA74F04);
    R_ShareRenderTarget(R_RENDERTARGET_FRAME_BUFFER, R_RENDERTARGET_SCENE);
    v0 = R_DescribeFormat(D3DFMT_A8R8G8B8);
    Com_Printf(8, "Requested frame buffer to be %s\n", v0);
    stru_EA74F04.surface.color->GetDesc(stru_EA74F04.surface.color, &surfaceDesc);
    if (surfaceDesc.Format == D3DFMT_UNKNOWN)
        MyAssertHandler(".\\r_rendertarget.cpp", 1035, 0, "%s", "surfaceDesc.Format != D3DFMT_UNKNOWN");
    v1 = R_DescribeFormat(surfaceDesc.Format);
    Com_Printf(8, "DirectX returned a frame buffer that is %s\n", v1);
    if (!g_allocateMinimalResources)
        R_InitFullscreenRenderTargetImage(9, FULLSCREEN_SCENE, 0, surfaceDesc.Format, RENDERTARGET_USAGE_RENDER, &image);
    return surfaceDesc.Format;
}

void __cdecl R_ShutdownRenderTargets()
{
    int renderTargetId; // [esp+0h] [ebp-4h]

    for (renderTargetId = 0; renderTargetId < 15; ++renderTargetId)
    {
        if (dword_EA74EF4[5 * renderTargetId])
            (*(void(__stdcall **)(int))(*(unsigned int *)dword_EA74EF4[5 * renderTargetId] + 8))(dword_EA74EF4[5 * renderTargetId]);
        if (dword_EA74EF8[5 * renderTargetId])
            (*(void(__stdcall **)(int))(*(unsigned int *)dword_EA74EF8[5 * renderTargetId] + 8))(dword_EA74EF8[5 * renderTargetId]);
        if (gfxRenderTargets[renderTargetId].image)
            Image_Release(gfxRenderTargets[renderTargetId].image);
    }
    memset((unsigned __int8 *)gfxRenderTargets, 0, sizeof(gfxRenderTargets));
    dx.singleSampleDepthStencilSurface = 0;
}

const char *__cdecl R_RenderTargetName(GfxRenderTargetId renderTargetId)
{
    return s_renderTargetNames[renderTargetId];
}

