#include "r_init.h"
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>

//int g_disableRendering   85825680     gfx_d3d:r_init.obj
//struct GfxAssets gfxAssets 85825684     gfx_d3d :r_init.obj
//struct DxGlobals dx        85825688     gfx_d3d :r_init.obj
DxGlobals dx;
//struct r_global_permanent_t rgp 85825700     gfx_d3d :r_init.obj
r_global_permanent_t rgp;
//struct vidConfig_t vidConfig 85827880     gfx_d3d :r_init.obj
vidConfig_t vidConfig;
//struct GfxConfiguration gfxCfg 858278b8     gfx_d3d : r_init.obj
//struct GfxGlobals r_glob   858278d0     gfx_d3d : r_init.obj
//struct r_globals_t rg      858278e0     gfx_d3d :r_init.obj
r_globals_t rg;
//struct GfxMetrics gfxMetrics 85827c00     gfx_d3d : r_init.obj
//int marker_r_init        85827c18     gfx_d3d : r_init.obj
//BOOL g_allocateMinimalResources 85827c1c     gfx_d3d : r_init.obj

GfxConfiguration gfxCfg;
GfxGlobals r_glob;

int g_disableRendering;

void __cdecl TRACK_r_init()
{
    track_static_alloc_internal(&rgp, 8576, "rgp", 18);
    track_static_alloc_internal(&rg, 33552, "rg", 18);
    track_static_alloc_internal(&vidConfig, 48, "vidConfig", 18);
    track_static_alloc_internal(&dx, 11488, "dx", 18);
}

void __cdecl  R_FatalInitError(const char *msg)
{
    Com_Printf(8, "********** DirectX returned an unrecoverable error code during initialization  **********\n");
    Com_Printf(8, "********** Initialization also happens while playing if DirectX loses a device **********\n");
    Com_Printf(8, "********** Consult the readme for how to continue from this problem            **********\n");
    Com_Printf(8, "\n%s\n", msg);
    Sys_DirectXFatalError();
}

void __cdecl  R_FatalLockError(HRESULT hr)
{
    const char *v1; // eax

    Com_Printf(8, "********** DirectX failed a call to lock a vertex buffer or an index buffer **********\n");
    v1 = R_ErrorDescription(hr);
    Com_Printf(8, "********** error information:  %s\n", v1);
    Sys_DirectXFatalError();
}

const char *__cdecl R_ErrorDescription(HRESULT hr)
{
    return DXGetErrorDescription9A(hr);
}

void __cdecl R_SetColorMappings()
{
    GfxGammaRamp gammaRamp; // [esp+0h] [ebp-208h] BYREF

    if (vidConfig.deviceSupportsGamma)
    {
        R_CalcGammaRamp(&gammaRamp);
        RB_SetGammaRamp(&gammaRamp);
    }
}

void __cdecl R_CalcGammaRamp(GfxGammaRamp *gammaRamp)
{
    float v1; // [esp+8h] [ebp-30h]
    float v2; // [esp+Ch] [ebp-2Ch]
    float v3; // [esp+18h] [ebp-20h]
    unsigned __int16 adjustedColorValue; // [esp+28h] [ebp-10h]
    unsigned __int16 colorTableIndex; // [esp+2Ch] [ebp-Ch]
    float exponent; // [esp+30h] [ebp-8h]

    if (!gammaRamp)
        MyAssertHandler(".\\r_init.cpp", 740, 0, "%s", "gammaRamp");
    if (r_gamma->current.value <= 0.0)
        MyAssertHandler(
            ".\\r_init.cpp",
            741,
            0,
            "%s\n\t(r_gamma->current.value) = %g",
            "(r_gamma->current.value > 0)",
            r_gamma->current.value);
    exponent = 1.0 / r_gamma->current.value;
    for (colorTableIndex = 0; colorTableIndex < 0x100u; ++colorTableIndex)
    {
        if (exponent == 1.0)
        {
            adjustedColorValue = 257 * colorTableIndex;
        }
        else
        {
            v2 = (double)colorTableIndex / 255.0;
            v1 = pow(v2, exponent);
            if (v1 < 0.0 || v1 >= 1.000007629394531)
                MyAssertHandler(
                    ".\\r_init.cpp",
                    754,
                    0,
                    "%s\n\t(unitScaleValue) = %g",
                    "(unitScaleValue >= 0 && unitScaleValue < 1 + 0.5f / 65535)",
                    v1);
            v3 = v1 * 65535.0;
            adjustedColorValue = (int)(v3 + 9.313225746154785e-10);
        }
        gammaRamp->entries[colorTableIndex] = adjustedColorValue;
    }
}

void __cdecl R_GammaCorrect(unsigned __int8 *buffer, int bufSize)
{
    int tableIndex; // [esp+0h] [ebp-210h]
    GfxGammaRamp gammaRamp; // [esp+8h] [ebp-208h] BYREF
    int inValue; // [esp+20Ch] [ebp-4h]

    if (!buffer)
        MyAssertHandler(".\\r_init.cpp", 784, 0, "%s", "buffer");
    if (bufSize <= 0)
        MyAssertHandler(".\\r_init.cpp", 785, 0, "%s\n\t(bufSize) = %i", "(bufSize > 0)", bufSize);
    R_CalcGammaRamp(&gammaRamp);
    for (tableIndex = 0; tableIndex < bufSize; ++tableIndex)
    {
        inValue = buffer[tableIndex];
        buffer[tableIndex] = 255 * gammaRamp.entries[inValue] / 0xFFFF;
    }
}

void __cdecl SetGfxConfig(const GfxConfiguration *config)
{
    const char *v1; // eax

    if (!config)
        MyAssertHandler(".\\r_init.cpp", 807, 0, "%s", "config");
    if (!config->maxClientViews || config->maxClientViews > 4)
        MyAssertHandler(
            ".\\r_init.cpp",
            808,
            0,
            "config->maxClientViews not in [1, GFX_MAX_CLIENT_VIEWS]\n\t%i not in [%i, %i]",
            config->maxClientViews,
            1,
            4);
    if (config->critSectCount != 22)
    {
        v1 = va("%d != %d", config->critSectCount, 22);
        MyAssertHandler(".\\r_init.cpp", 810, 0, "%s\n\t%s", "config->critSectCount == CRITSECT_COUNT", v1);
    }
    memcpy(&gfxCfg, config, sizeof(gfxCfg));
}

void __cdecl R_InitThreads()
{
    void *v0; // ecx

    if (r_glob.isRenderingRemoteUpdate)
        MyAssertHandler(".\\r_init.cpp", 2315, 0, "%s", "!r_glob.isRenderingRemoteUpdate");
    R_InitRenderThread();
    R_InitWorkerThreads(v0);
}

void __cdecl R_ShutdownStreams()
{
    if (dx.device)
    {
        if (!dx.deviceLost)
            R_ClearAllStreamSources(&gfxCmdBufState.prim);
    }
}

void __cdecl R_Shutdown(int destroyWindow)
{
    if (rg.registered)
    {
        R_SyncRenderThread();
        rg.registered = 0;
        if (!r_glob.haveThreadOwnership)
            MyAssertHandler(".\\r_init.cpp", 2413, 0, "%s", "r_glob.haveThreadOwnership");
        r_glob.startedRenderThread = 0;
        R_ShutdownStreams();
        R_ShutdownMaterialUsage();
        R_ShutdownDebug();
        R_SaveLightVisHistory();
        R_ShutdownLightDefs();
        R_ShutdownWorld();
        if (!useFastFile->current.enabled)
        {
            R_ShutdownLoadWater();
            R_ShutdownFonts();
            Material_Shutdown();
            R_ShutdownImages();
        }
        R_ResetModelLighting();
        rgp.world = 0;
        R_UnlockSkinnedCache();
        R_FlushStaticModelCache();
        if (destroyWindow)
        {
            R_ShutdownDirect3D();
            R_ShutdownRenderCommands();
        }
        R_UnregisterCmds();
    }
}

void R_ShutdownDirect3D()
{
    IDirect3DSurface9 *var; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *varCopy; // [esp+8h] [ebp-4h]

    if (useFastFile->current.enabled)
        R_UnloadGraphicsAssets();
    R_Cinematic_Shutdown();
    R_ReleaseForShutdownOrReset();
    while (dx.windowCount)
    {
        if (!dx.windows[--dx.windowCount].hwnd)
            MyAssertHandler(".\\r_init.cpp", 2205, 0, "%s", "dx.windows[dx.windowCount].hwnd");
        if (IsWindow(dx.windows[dx.windowCount].hwnd))
            DestroyWindow(dx.windows[dx.windowCount].hwnd);
        dx.windows[dx.windowCount].hwnd = 0;
    }
    if (dx.device)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.device->Release()\n");
            varCopy = dx.device;
            dx.device = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>((IDirect3DSurface9 *)varCopy, "dx.device", ".\\r_init.cpp", 2214);
        } while (alwaysfails);
    }
    if (dx.d3d9)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.d3d9->Release()\n");
            var = (IDirect3DSurface9 *)dx.d3d9;
            dx.d3d9 = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(var, "dx.d3d9", ".\\r_init.cpp", 2217);
        } while (alwaysfails);
    }
}

void __cdecl R_UnloadWorld()
{
    if (!useFastFile->current.enabled)
        MyAssertHandler(".\\r_init.cpp", 2468, 0, "%s", "IsFastFileLoad()");
    if (rgp.world)
        Sys_Error("Cannot unload bsp while it is in use");
}

void __cdecl R_BeginRegistration(vidConfig_t *vidConfigOut)
{
    if (rg.registered)
        MyAssertHandler(".\\r_init.cpp", 2478, 0, "%s", "!rg.registered");
    R_Init();
    if (!dx.d3d9 || !dx.device)
        MyAssertHandler(".\\r_init.cpp", 2484, 0, "%s", "dx.d3d9 && dx.device");
    if (!rg.registered)
        MyAssertHandler(".\\r_init.cpp", 2486, 1, "%s", "rg.registered == true");
    if (!vidConfigOut)
        MyAssertHandler(".\\r_init.cpp", 2496, 0, "%s", "vidConfigOut");
    memcpy(vidConfigOut, &vidConfig, sizeof(vidConfig_t));
    if (r_glob.startedRenderThread)
        MyAssertHandler(".\\r_init.cpp", 2500, 0, "%s", "!r_glob.startedRenderThread");
    r_glob.startedRenderThread = 1;
    R_ReleaseThreadOwnership();
}

void R_Init()
{
    HRESULT hr; // [esp+0h] [ebp-4h]

    Com_Printf(8, "----- R_Init -----\n");
    Swap_Init();
    R_Register();
    R_InitGlobalStructs();
    R_InitDrawMethod();
    R_InitGraphicsApi();
    RB_RegisterBackendAssets();
    R_InitWater();
    if (!dx.deviceLost)
    {
        hr = dx.device->TestCooperativeLevel(dx.device);
        if (hr != -2005530520 && hr != -2005530519)
        {
            dx.sunSpriteSamples = RB_CalcSunSpriteSamples();
            if (!dx.sunSpriteSamples)
            {
                Com_Printf(8, "Sun sprite occlusion query calibration failed; reverting to low-quality sun visibility test");
                RB_FreeSunSpriteQueries();
            }
        }
    }
    RB_ProfileInit();
}

void R_InitGraphicsApi()
{
    GfxWindowParms wndParms; // [esp+4h] [ebp-28h] BYREF

    if ((dx.device != 0) != (dx.d3d9 != 0))
        MyAssertHandler(".\\r_init.cpp", 1981, 0, "%s", "(dx.device != NULL) == (dx.d3d9 != NULL)");
    if (dx.device)
    {
        R_InitSystems();
    }
    else
    {
        R_PreCreateWindow();
        while (1)
        {
            R_SetWndParms(&wndParms);
            if (R_CreateGameWindow(&wndParms))
                break;
            if (!R_ReduceWindowSettings())
                R_FatalInitError("Couldn't initialize renderer");
        }
    }
}

void R_InitSystems()
{
    R_InitImages();
    Material_Init();
    R_InitFonts();
    R_InitLoadWater();
    R_InitLightDefs();
    R_ClearFogs();
    R_InitDebug();
    rg.registered = 1;
}

char __cdecl R_PreCreateWindow()
{
    if (dx.d3d9)
    {
        if (!alwaysfails)
            MyAssertHandler(".\\r_init.cpp", 1345, 1, "D3D re-initialized before being shutdown");
    }
    else
    {
        Com_Printf(8, "Getting Direct3D 9 interface...\n");
        dx.d3d9 = Direct3DCreate9(0x20u);
        if (!dx.d3d9)
        {
            Com_Printf(8, "Direct3D 9 failed to initialize\n");
            return 0;
        }
    }
    dx.adapterIndex = R_ChooseAdapter();
    R_StoreDirect3DCaps(dx.adapterIndex);
    R_EnumDisplayModes(dx.adapterIndex);
    return 1;
}

void __cdecl R_StoreDirect3DCaps(unsigned int adapterIndex)
{
    unsigned int MaxUserClipPlanes; // [esp+0h] [ebp-138h]
    unsigned int MaxTextureHeight; // [esp+4h] [ebp-134h]
    _D3DCAPS9 caps; // [esp+8h] [ebp-130h] BYREF

    R_GetDirect3DCaps(adapterIndex, &caps);
    R_PickRenderer(&caps);
    if ((int)caps.MaxTextureHeight < (int)caps.MaxTextureWidth)
        MaxTextureHeight = caps.MaxTextureHeight;
    else
        MaxTextureHeight = caps.MaxTextureWidth;
    vidConfig.maxTextureSize = MaxTextureHeight;
    vidConfig.maxTextureMaps = caps.MaxSimultaneousTextures;
    vidConfig.deviceSupportsGamma = (caps.Caps2 & 0x20000) != 0;
    if (caps.MaxSimultaneousTextures > 0x10)
        MyAssertHandler(
            ".\\r_init.cpp",
            700,
            1,
            "%s\n\t(vidConfig.maxTextureMaps) = %i",
            "(vidConfig.maxTextureMaps <= 16)",
            vidConfig.maxTextureMaps);
    if ((int)caps.MaxUserClipPlanes < 6)
        MaxUserClipPlanes = caps.MaxUserClipPlanes;
    else
        MaxUserClipPlanes = 6;
    gfxMetrics.maxClipPlanes = MaxUserClipPlanes;
    gfxMetrics.hasAnisotropicMinFilter = (caps.TextureFilterCaps & 0x400) != 0;
    gfxMetrics.hasAnisotropicMagFilter = ((unsigned int)&svs.clients[29].netchanOutgoingBuffer[109728]
        & caps.TextureFilterCaps) != 0;
    gfxMetrics.maxAnisotropy = caps.MaxAnisotropy;
    gfxMetrics.slopeScaleDepthBias = (caps.RasterCaps & 0x2000000) != 0;
    gfxMetrics.canMipCubemaps = (caps.TextureCaps & 0x10000) != 0;
    gfxMetrics.hasTransparencyMsaa = R_CheckTransparencyMsaa(adapterIndex);
    R_SetShadowmapFormats_DX(adapterIndex);
}

void __cdecl R_GetDirect3DCaps(unsigned int adapterIndex, _D3DCAPS9 *caps)
{
    const char *v2; // eax
    const char *v3; // eax
    int hr; // [esp+0h] [ebp-8h]
    int attempt; // [esp+4h] [ebp-4h]

    if (!dx.d3d9)
        MyAssertHandler(".\\r_init.cpp", 533, 0, "%s", "dx.d3d9");
    attempt = 0;
    while (1)
    {
        hr = ((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, _D3DCAPS9 *))dx.d3d9->GetDeviceCaps)(
            dx.d3d9,
            dx.d3d9,
            adapterIndex,
            1,
            caps);
        if (hr >= 0)
            break;
        Sleep(0x64u);
        if (++attempt == 20)
        {
            v2 = R_ErrorDescription(hr);
            v3 = va("GetDeviceCaps failed: %s", v2);
            R_FatalInitError(v3);
        }
    }
}

void __cdecl R_SetShadowmapFormats_DX(unsigned int adapterIndex)
{
    _D3DFORMAT colorFormat; // [esp+0h] [ebp-24h]
    unsigned int formatIndex; // [esp+4h] [ebp-20h]
    _D3DFORMAT depthFormat; // [esp+8h] [ebp-1Ch]
    _D3DFORMAT formats[3][2]; // [esp+Ch] [ebp-18h]

    *(_QWORD *)&formats[0][0] = 0x170000004BLL;
    *(_QWORD *)&formats[1][0] = 0x160000004BLL;
    *(_QWORD *)&formats[2][0] = 0x150000004BLL;
    for (formatIndex = 0; formatIndex < 3; ++formatIndex)
    {
        depthFormat = formats[formatIndex][0];
        colorFormat = formats[formatIndex][1];
        if (!((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, int, _D3DFORMAT, _D3DFORMAT))dx.d3d9->CheckDepthStencilMatch)(
            dx.d3d9,
            dx.d3d9,
            adapterIndex,
            1,
            22,
            colorFormat,
            depthFormat)
            && !((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, int, int, int, int, _D3DFORMAT))dx.d3d9->CheckDeviceFormat)(
                dx.d3d9,
                dx.d3d9,
                adapterIndex,
                1,
                22,
                2,
                3,
                depthFormat))
        {
            gfxMetrics.shadowmapFormatPrimary = depthFormat;
            gfxMetrics.shadowmapFormatSecondary = colorFormat;
            gfxMetrics.shadowmapBuildTechType = TECHNIQUE_BUILD_SHADOWMAP_DEPTH;
            gfxMetrics.hasHardwareShadowmap = 1;
            gfxMetrics.shadowmapSamplerState = 98;
            return;
        }
    }
    gfxMetrics.shadowmapFormatPrimary = D3DFMT_R32F;
    gfxMetrics.shadowmapFormatSecondary = D3DFMT_D24X8;
    gfxMetrics.shadowmapBuildTechType = TECHNIQUE_BUILD_SHADOWMAP_COLOR;
    gfxMetrics.hasHardwareShadowmap = 0;
    gfxMetrics.shadowmapSamplerState = 97;
}

unsigned int __cdecl R_ChooseAdapter()
{
    unsigned int foundAdapterIndex; // [esp+14h] [ebp-470h]
    unsigned int adapterIndex; // [esp+1Ch] [ebp-468h]
    HMONITOR__ *desiredMonitor; // [esp+20h] [ebp-464h]
    unsigned int adapterCount; // [esp+24h] [ebp-460h]
    _D3DADAPTER_IDENTIFIER9 id; // [esp+2Ch] [ebp-458h] BYREF

    desiredMonitor = R_ChooseMonitor();
    foundAdapterIndex = 0;
    adapterCount = dx.d3d9->GetAdapterCount(dx.d3d9);
    for (adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
    {
        if (desiredMonitor)
        {
            if (dx.d3d9->GetAdapterMonitor(dx.d3d9, adapterIndex) != desiredMonitor)
                continue;
            foundAdapterIndex = adapterIndex;
        }
        if (dx.d3d9->GetAdapterIdentifier(dx.d3d9, adapterIndex, 0, &id) >= 0
            && !strcmp(id.Description, "NVIDIA NVPerfHUD"))
        {
            return adapterIndex;
        }
    }
    return foundAdapterIndex;
}

char __cdecl R_CreateGameWindow(GfxWindowParms *wndParms)
{
    if (!R_CreateWindow(wndParms))
        return 0;
    if (!R_InitHardware(wndParms))
        return 0;
    dx.targetWindowIndex = 0;
    ShowWindow(wndParms->hwnd, 5);
    Sys_HideSplashWindow();
    return 1;
}

char __cdecl R_InitHardware(const GfxWindowParms *wndParms)
{
    unsigned int workerIndex; // [esp+4h] [ebp-4h]

    if (!R_CreateDevice(wndParms))
        return 0;
    if (useFastFile->current.enabled)
        R_LoadGraphicsAssets();
    R_UpdateGpuSyncType();
    R_StoreWindowSettings(wndParms);
    RB_InitSceneViewport();
    CL_ResetStats_f();
    if (!R_CreateForInitOrReset())
        return 0;
    R_Cinematic_Init();
    Com_Printf(8, "Setting initial state...\n");
    RB_SetInitialState();
    R_InitGamma();
    R_InitScene();
    R_InitSystems();
    CL_ResetStats_f();
    R_FinishAttachingToWindow(wndParms);
    for (workerIndex = 0; workerIndex < 2; ++workerIndex)
    {
        if (!r_smp_worker_thread[workerIndex])
            MyAssertHandler(".\\r_init.cpp", 1593, 0, "%s", "r_smp_worker_thread[workerIndex]");
        Dvar_ClearModified(r_smp_worker_thread[workerIndex]);
        if (r_smp_worker_thread[workerIndex]->current.enabled)
            Sys_ResumeThread(workerIndex + 2);
    }
    return 1;
}

void __cdecl R_StoreWindowSettings(const GfxWindowParms *wndParms)
{
    const char *v1; // eax
    float v2; // [esp+18h] [ebp-20h]
    int monitorHeight; // [esp+28h] [ebp-10h]
    int monitorWidth; // [esp+2Ch] [ebp-Ch]

    if (!r_aspectRatio)
        MyAssertHandler(".\\r_init.cpp", 425, 0, "%s", "r_aspectRatio");
    vidConfig.sceneWidth = wndParms->sceneWidth;
    vidConfig.sceneHeight = wndParms->sceneHeight;
    vidConfig.displayWidth = wndParms->displayWidth;
    vidConfig.displayHeight = wndParms->displayHeight;
    vidConfig.displayFrequency = wndParms->hz;
    vidConfig.isFullscreen = wndParms->fullscreen;
    switch (r_aspectRatio->current.integer)
    {
    case 0:
        if (vidConfig.isFullscreen && dx.adapterNativeIsValid)
        {
            monitorWidth = dx.adapterNativeWidth;
            monitorHeight = dx.adapterNativeHeight;
        }
        else
        {
            monitorWidth = vidConfig.displayWidth;
            monitorHeight = vidConfig.displayHeight;
        }
        v2 = (double)monitorHeight * 16.0 / (double)monitorWidth;
        if ((int)(v2 + 9.313225746154785e-10) == 10)
        {
            vidConfig.aspectRatioWindow = 1.6;
        }
        else if ((int)(v2 + 9.313225746154785e-10) >= 10)
        {
            vidConfig.aspectRatioWindow = 1.3333334;
        }
        else
        {
            vidConfig.aspectRatioWindow = 1.7777778;
        }
        break;
    case 1:
        vidConfig.aspectRatioWindow = 1.3333334;
        break;
    case 2:
        vidConfig.aspectRatioWindow = 1.6;
        break;
    case 3:
        vidConfig.aspectRatioWindow = 1.7777778;
        break;
    default:
        if (!alwaysfails)
        {
            v1 = va("unhandled case, aspectRatio = %i\n", r_aspectRatio->current.integer);
            MyAssertHandler(".\\r_init.cpp", 470, 1, v1);
        }
        break;
    }
    if (!com_wideScreen)
        MyAssertHandler(".\\r_init.cpp", 485, 0, "%s", "com_wideScreen");
    Dvar_SetBool((dvar_s *)com_wideScreen, vidConfig.aspectRatioWindow != 1.333333373069763);
    vidConfig.aspectRatioScenePixel = (double)vidConfig.sceneHeight
        * vidConfig.aspectRatioWindow
        / (double)vidConfig.sceneWidth;
    if (vidConfig.isFullscreen)
        vidConfig.aspectRatioDisplayPixel = (double)dx.adapterFullscreenHeight
        * vidConfig.aspectRatioWindow
        / (double)dx.adapterFullscreenWidth;
    else
        vidConfig.aspectRatioDisplayPixel = 1.0;
}

void R_InitGamma()
{
    Dvar_SetModified(r_gamma);
}

char __cdecl R_CreateForInitOrReset()
{
    const char *v0; // eax
    int hr; // [esp+8h] [ebp-8h]
    unsigned int fenceIter; // [esp+Ch] [ebp-4h]

    Com_Printf(8, "Initializing render targets...\n");
    R_InitRenderTargets();
    if (!g_allocateMinimalResources)
    {
        R_InitRenderBuffers();
        R_InitModelLightingImage();
        Com_Printf(8, "Initializing static model cache...\n");
        R_InitStaticModelCache();
    }
    Com_Printf(8, "Initializing dynamic buffers...\n");
    R_CreateDynamicBuffers();
    if (!g_allocateMinimalResources)
    {
        Com_Printf(8, "Initializing particle cloud buffer...\n");
        R_CreateParticleCloudBuffer();
    }
    Com_Printf(8, "Creating Direct3D queries...\n");
    dx.nextFence = 0;
    dx.flushGpuQueryIssued = 0;
    dx.flushGpuQueryCount = 0;
    hr = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, IDirect3DQuery9 **))dx.device->CreateQuery)(
        dx.device,
        dx.device,
        8,
        &dx.flushGpuQuery);
    if (hr >= 0)
    {
        for (fenceIter = 0; fenceIter < 8; ++fenceIter)
        {
            hr = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, unsigned int))dx.device->CreateQuery)(
                dx.device,
                dx.device,
                8,
                4 * fenceIter + 235334204);
            if (hr < 0)
                goto LABEL_6;
        }
        if (!g_allocateMinimalResources)
        {
            RB_AllocSunSpriteQueries();
            gfxAssets.pixelCountQuery = RB_HW_AllocOcclusionQuery();
        }
        return 1;
    }
    else
    {
    LABEL_6:
        v0 = R_ErrorDescription(hr);
        Com_Printf(8, "Event query creation failed: %s (0x%08x)\n", v0, hr);
        return 0;
    }
}

IDirect3DQuery9 *__cdecl RB_HW_AllocOcclusionQuery()
{
    const char *v0; // eax
    int hr; // [esp+0h] [ebp-8h]
    IDirect3DQuery9 *query; // [esp+4h] [ebp-4h] BYREF

    hr = dx.device->CreateQuery(dx.device, D3DQUERYTYPE_OCCLUSION, &query);
    if (hr >= 0)
        return query;
    v0 = R_ErrorDescription(hr);
    Com_Printf(8, "Occlusion query creation failed: %s (0x%08x)\n", v0, hr);
    return 0;
}

char __cdecl R_CreateDevice(const GfxWindowParms *wndParms)
{
    const char *v1; // eax
    _D3DPRESENT_PARAMETERS_ d3dpp; // [esp+0h] [ebp-44h] BYREF
    HWND__ *hwnd; // [esp+38h] [ebp-Ch]
    HRESULT hr; // [esp+3Ch] [ebp-8h]
    unsigned int behavior; // [esp+40h] [ebp-4h]

    if (!wndParms)
        MyAssertHandler(".\\r_init.cpp", 1473, 0, "%s", "wndParms");
    if (dx.windowCount)
        MyAssertHandler(".\\r_init.cpp", 1475, 0, "%s", "dx.windowCount == 0");
    if (!wndParms->hwnd)
        MyAssertHandler(".\\r_init.cpp", 1476, 0, "%s", "wndParms->hwnd");
    hwnd = wndParms->hwnd;
    if (dx.device)
        MyAssertHandler(".\\r_init.cpp", 1484, 0, "%s", "dx.device == NULL");
    dx.depthStencilFormat = R_GetDepthStencilFormat(D3DFMT_A8R8G8B8);
    R_SetD3DPresentParameters(&d3dpp, wndParms);
    behavior = 70;
    hr = R_CreateDeviceInternal(hwnd, 0x46u, &d3dpp);
    r_glob.haveThreadOwnership = 1;
    if (hr >= 0)
    {
        if (!dx.device)
            MyAssertHandler(".\\r_init.cpp", 1514, 1, "%s", "dx.device");
        dx.deviceLost = 0;
        return 1;
    }
    else
    {
        v1 = R_ErrorDescription(hr);
        Com_Printf(8, "Couldn't create a Direct3D device: %s\n", v1);
        return 0;
    }
}

void __cdecl R_SetD3DPresentParameters(_D3DPRESENT_PARAMETERS_ *d3dpp, const GfxWindowParms *wndParms)
{
    if (!d3dpp)
        MyAssertHandler(".\\r_init.cpp", 339, 0, "%s", "d3dpp");
    if (!wndParms)
        MyAssertHandler(".\\r_init.cpp", 340, 0, "%s", "wndParms");
    R_SetupAntiAliasing(wndParms);
    memset((unsigned __int8 *)d3dpp, 0, sizeof(_D3DPRESENT_PARAMETERS_));
    d3dpp->BackBufferWidth = wndParms->displayWidth;
    d3dpp->BackBufferHeight = wndParms->displayHeight;
    d3dpp->BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp->BackBufferCount = 1;
    d3dpp->MultiSampleType = dx.multiSampleType;
    d3dpp->MultiSampleQuality = dx.multiSampleQuality;
    d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp->EnableAutoDepthStencil = 0;
    d3dpp->AutoDepthStencilFormat = dx.depthStencilFormat;
    d3dpp->PresentationInterval = r_vsync->current.enabled ? 1 : 0x80000000;
    if (!wndParms->hwnd)
        MyAssertHandler(".\\r_init.cpp", 374, 0, "%s", "wndParms->hwnd");
    d3dpp->hDeviceWindow = wndParms->hwnd;
    d3dpp->Flags = 0;
    if (wndParms->fullscreen)
    {
        d3dpp->Windowed = 0;
        d3dpp->FullScreen_RefreshRateInHz = wndParms->hz;
    }
    else
    {
        d3dpp->Windowed = 1;
        d3dpp->FullScreen_RefreshRateInHz = 0;
    }
}

void __cdecl R_SetupAntiAliasing(const GfxWindowParms *wndParms)
{
    _D3DMULTISAMPLE_TYPE multiSampleCount; // [esp+0h] [ebp-Ch]
    unsigned int qualityLevels; // [esp+8h] [ebp-4h] BYREF

    if (!wndParms)
        MyAssertHandler(".\\r_init.cpp", 234, 0, "%s", "wndParms");
    if (wndParms->aaSamples < 1 || wndParms->aaSamples > 16)
        MyAssertHandler(
            ".\\r_init.cpp",
            248,
            0,
            "wndParms->aaSamples not in [1, 16]\n\t%i not in [%i, %i]",
            wndParms->aaSamples,
            1,
            16);
    if (r_reflectionProbeGenerate->current.enabled)
        multiSampleCount = D3DMULTISAMPLE_NONMASKABLE;
    else
        multiSampleCount = wndParms->aaSamples;
    while (multiSampleCount > D3DMULTISAMPLE_NONMASKABLE)
    {
        dx.multiSampleType = multiSampleCount;
        if (dx.d3d9->CheckDeviceMultiSampleType(
            dx.d3d9,
            0,
            D3DDEVTYPE_HAL,
            D3DFMT_A8R8G8B8,
            !wndParms->fullscreen,
            multiSampleCount,
            &qualityLevels) >= 0)
        {
            Com_Printf(8, "Using %ix anti-aliasing\n", multiSampleCount);
            dx.multiSampleQuality = qualityLevels - 1;
            return;
        }
        --multiSampleCount;
    }
    dx.multiSampleType = D3DMULTISAMPLE_NONE;
    dx.multiSampleQuality = 0;
}

HRESULT __cdecl R_CreateDeviceInternal(HWND__ *hwnd, unsigned int behavior, _D3DPRESENT_PARAMETERS_ *d3dpp)
{
    _D3DDEVTYPE DeviceType; // eax
    _D3DDISPLAYMODE getModeResult; // [esp+4h] [ebp-18h] BYREF
    HRESULT hr; // [esp+14h] [ebp-8h]
    int attempt; // [esp+18h] [ebp-4h]

    Com_Printf(8, "Creating Direct3D device...\n");
    attempt = 0;
    while (1)
    {
        dx.adapterNativeIsValid = R_GetMonitorDimensions(&dx.adapterNativeWidth, &dx.adapterNativeHeight);
        DeviceType = R_GetDeviceType();
        hr = dx.d3d9->CreateDevice(dx.d3d9, dx.adapterIndex, DeviceType, hwnd, behavior, d3dpp, &dx.device);
        if (hr >= 0)
            break;
        Sleep(0x64u);
        if (++attempt == 20)
        {
            if (!dx.adapterIndex)
                return hr;
            dx.adapterIndex = 0;
            return R_CreateDeviceInternal(hwnd, behavior, d3dpp);
        }
    }
    if (dx.d3d9->GetAdapterDisplayMode(dx.d3d9, dx.adapterIndex, &getModeResult) < 0)
    {
        dx.adapterFullscreenWidth = d3dpp->BackBufferWidth;
        dx.adapterFullscreenHeight = d3dpp->BackBufferHeight;
    }
    else
    {
        dx.adapterFullscreenWidth = getModeResult.Width;
        dx.adapterFullscreenHeight = getModeResult.Height;
    }
    return hr;
}

int __cdecl R_GetDeviceType()
{
    _D3DADAPTER_IDENTIFIER9 id; // [esp+18h] [ebp-458h] BYREF

    if (((int(__thiscall *)(IDirect3D9 *, IDirect3D9 *, unsigned int, unsigned int, _D3DADAPTER_IDENTIFIER9 *))dx.d3d9->GetAdapterIdentifier)(
        dx.d3d9,
        dx.d3d9,
        dx.adapterIndex,
        0,
        &id) >= 0
        && !strcmp(id.Description, "NVIDIA NVPerfHUD"))
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

void __cdecl R_SetWndParms(GfxWindowParms *wndParms)
{
    const char *resolutionString; // [esp+0h] [ebp-Ch]
    int refreshRate; // [esp+4h] [ebp-8h] BYREF
    const char *refreshRateString; // [esp+8h] [ebp-4h]

    wndParms->fullscreen = Dvar_GetBool("r_fullscreen");
    if (wndParms->fullscreen || !R_SetCustomResolution(wndParms))
    {
        resolutionString = Dvar_EnumToString(r_mode);
        sscanf(resolutionString, "%ix%i", &wndParms->displayWidth, &wndParms->displayHeight);
    }
    wndParms->sceneWidth = wndParms->displayWidth;
    wndParms->sceneHeight = wndParms->displayHeight;
    if (wndParms->fullscreen)
    {
        refreshRateString = Dvar_EnumToString(r_displayRefresh);
        sscanf(refreshRateString, "%i Hz", &refreshRate);
        wndParms->hz = (int)R_ClosestRefreshRateForMode(wndParms->displayWidth, wndParms->displayHeight, refreshRate);
    }
    else
    {
        wndParms->hz = 60;
    }
    wndParms->x = Dvar_GetInt("vid_xpos");
    wndParms->y = Dvar_GetInt("vid_ypos");
    wndParms->hwnd = 0;
    wndParms->aaSamples = r_aaSamples->current.integer;
}

void R_Register()
{
    R_RegisterDvars();
    R_RegisterCmds();
}

void R_InitGlobalStructs()
{
    memset((unsigned __int8 *)&rg, 0, sizeof(rg));
    memset((unsigned __int8 *)&rgp, 0, sizeof(rgp));
    RB_InitBackendGlobalStructs();
    rg.identityPlacement.base.quat[0] = 0.0;
    rg.identityPlacement.base.quat[1] = 0.0;
    rg.identityPlacement.base.quat[2] = 0.0;
    rg.identityPlacement.base.quat[3] = 1.0;
    rg.identityPlacement.base.origin[0] = 0.0;
    rg.identityPlacement.base.origin[1] = 0.0;
    rg.identityPlacement.base.origin[2] = 0.0;
    rg.identityPlacement.scale = 1.0;
    MatrixIdentity44(rg.identityViewParms.viewMatrix.m);
    MatrixIdentity44(rg.identityViewParms.projectionMatrix.m);
    MatrixIdentity44(rg.identityViewParms.viewProjectionMatrix.m);
    MatrixIdentity44(rg.identityViewParms.inverseViewProjectionMatrix.m);
}

void __cdecl R_EndRegistration()
{
    if (!rg.registered)
        MyAssertHandler(".\\r_init.cpp", 2511, 0, "%s", "rg.registered");
    CL_ResetStats_f();
    if (!useFastFile->current.enabled)
    {
        R_SyncRenderThread();
        RB_TouchAllImages();
    }
}

void __cdecl R_TrackStatistics(trStatistics_t *stats)
{
    rg.stats = stats;
}

void __cdecl R_UpdateTeamColors(int team, const float *color_allies, const float *color_axis)
{
    rg.team = team;
    Byte4PackRgba(color_allies, (unsigned __int8 *)&rg.color_allies);
    Byte4PackRgba(color_axis, (unsigned __int8 *)&rg.color_axis);
}

void __cdecl R_ConfigureRenderer(const GfxConfiguration *config)
{
    SetGfxConfig(config);
    R_InitRenderCommands();
}

void __cdecl R_ComErrorCleanup()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_init.cpp", 2819, 0, "%s", "Sys_IsMainThread()");
    R_AbortRenderCommands();
    R_SyncRenderThread();
    if (dx.inScene)
    {
        ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *))dx.device->EndScene)(dx.device, dx.device);
        dx.inScene = 0;
    }
}

