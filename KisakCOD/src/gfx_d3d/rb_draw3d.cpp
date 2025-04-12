#include "rb_draw3d.h"
#include "rb_logfile.h"
#include "r_dvars.h"
#include "r_state.h"
#include "r_draw_method.h"
#include "rb_state.h"
#include "r_cmdbuf.h"
#include "r_utils.h"
#include "rb_sky.h"

void __cdecl R_HW_InsertFence(IDirect3DQuery9 **fence)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-4h]

    *fence = dx.fencePool[dx.nextFence];
    dx.nextFence = (dx.nextFence + 1) % 8;
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("(*fence)->Issue( (1 << 0) )\n");
        hr = (*fence)->Issue(1);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_fence_pc.h (%i) (*fence)->Issue( (1 << 0) ) failed: %s\n",
                    8,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_ShowTris(GfxCmdBufContext context, const GfxDrawSurfListInfo *info)
{
    GfxDrawSurfListInfo debugInfo; // [esp+14h] [ebp-28h] BYREF

    if (r_showTris->current.integer)
    {
        if ((r_showTris->current.integer & 2) != 0)
            R_ClearScreen(context.state->prim.device, 6u, colorWhite, 1.0, 0, 0);
        memcpy(&debugInfo, info, sizeof(debugInfo));
        debugInfo.baseTechType = TECHNIQUE_WIREFRAME_SOLID;
        R_DrawSurfs(context, 0, &debugInfo);
    }
}

void __cdecl RB_Draw3DInternal(const GfxViewInfo *viewInfo)
{
    //Profile_Begin(141);
    if (!rgp.world)
        MyAssertHandler(".\\rb_draw3d.cpp", 1452, 0, "%s", "rgp.world");
    if (gfxDrawMethod.drawScene)
    {
        if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_FULLBRIGHT)
        {
            RB_FullbrightDrawCommands(viewInfo);
        }
        else if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_DEBUGSHADER)
        {
            RB_DebugShaderDrawCommands(viewInfo);
        }
        else
        {
            if (gfxDrawMethod.drawScene != GFX_DRAW_SCENE_STANDARD)
                MyAssertHandler(
                    ".\\rb_draw3d.cpp",
                    1472,
                    0,
                    "%s\n\t(gfxDrawMethod.drawScene) = %i",
                    "(gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)",
                    gfxDrawMethod.drawScene);
            RB_StandardDrawCommands(viewInfo);
        }
    }
    else
    {
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
        memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
        R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
        R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    }
    //Profile_EndInternal(0);
}

void __cdecl RB_FullbrightDrawCommands(const GfxViewInfo *viewInfo)
{
    const GfxBackEndData *data; // [esp+10h] [ebp-8h]
    GfxCmdBuf cmdBuf; // [esp+14h] [ebp-4h] BYREF

    data = backEndData;
    R_SetAndClearSceneTarget(0);
    R_InitContext(data, &cmdBuf);
    R_DrawFullbright(viewInfo, &gfxCmdBufContext.source->input, &cmdBuf);
    R_InitCmdBufSourceState(gfxCmdBufContext.source, &viewInfo->input, 0);
    memcpy(gfxCmdBufContext.state, &gfxCmdBufState, sizeof(GfxCmdBufState));
    memset(
        (unsigned __int8 *)gfxCmdBufContext.state->vertexShaderConstState,
        0,
        sizeof(gfxCmdBufContext.state->vertexShaderConstState));
    memset(
        (unsigned __int8 *)gfxCmdBufContext.state->pixelShaderConstState,
        0,
        sizeof(gfxCmdBufContext.state->pixelShaderConstState));
    R_SetRenderTargetSize(gfxCmdBufContext.source, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
    R_BeginView(gfxCmdBufContext.source, &viewInfo->sceneDef, &viewInfo->viewParms);
    R_SetViewportStruct(gfxCmdBufContext.source, &viewInfo->sceneViewport);
    RB_DrawSun(viewInfo->localClientNum);
    memcpy(&gfxCmdBufState, gfxCmdBufContext.state, sizeof(gfxCmdBufState));
    RB_EndSceneRendering(gfxCmdBufContext, &viewInfo->input, viewInfo);
}

void __cdecl RB_EndSceneRendering(GfxCmdBufContext context, const GfxCmdBufInput *input, const GfxViewInfo *viewInfo)
{
    R_HW_InsertFence(&backEndData->endFence);
    R_InitCmdBufSourceState(context.source, input, 0);
    memcpy(context.state, &gfxCmdBufState, sizeof(GfxCmdBufState));
    memset((unsigned __int8 *)context.state->vertexShaderConstState, 0, sizeof(context.state->vertexShaderConstState));
    memset((unsigned __int8 *)context.state->pixelShaderConstState, 0, sizeof(context.state->pixelShaderConstState));
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
    R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->viewParms);
    R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->sceneViewport);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
    if (developer->current.integer)
    {
        R_Set3D(&gfxCmdBufSourceState);
        if (gfxCmdBufSourceState.viewMode != VIEW_MODE_3D)
            MyAssertHandler(
                ".\\rb_draw3d.cpp",
                119,
                0,
                "%s\n\t(gfxCmdBufSourceState.viewMode) = %i",
                "(gfxCmdBufSourceState.viewMode == VIEW_MODE_3D)",
                gfxCmdBufSourceState.viewMode);
        RB_DrawDebug(&gfxCmdBufSourceState.viewParms);
        RB_ShowCollision(&gfxCmdBufSourceState.viewParms);
    }
    memcpy(&gfxCmdBufState, context.state, sizeof(gfxCmdBufState));
}

void __cdecl R_SetAndClearSceneTarget(const GfxViewport *viewport)
{
    memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
    memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
    CL_ResetStats_f();
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
    R_ClearForFrameBuffer(gfxCmdBufState.prim.device, viewport);
    memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
}

void __cdecl R_ClearForFrameBuffer(IDirect3DDevice9 *device, const GfxViewport *viewport)
{
    float clearColor[4]; // [esp+10h] [ebp-10h] BYREF

    R_GetClearColor(clearColor);
    R_ClearScreen(device, 7u, clearColor, 1.0, 0, viewport);
}

void __cdecl R_DrawFullbright(const GfxViewInfo *viewInfo, GfxCmdBufInput *input, GfxCmdBuf *cmdBuf)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    R_DrawFullbrightOrDebugShader(
        (int)&savedregs,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawDebugShaderLitCallback,
        viewInfo,
        &viewInfo->litInfo,
        cmdBuf);
    R_DrawFullbrightOrDebugShader(
        (int)&savedregs,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawFullbrightDecalCallback,
        viewInfo,
        &viewInfo->decalInfo,
        cmdBuf);
    R_DrawFullbrightOrDebugShader(
        (int)&savedregs,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawDebugShaderEmissiveCallback,
        viewInfo,
        &viewInfo->emissiveInfo,
        cmdBuf);
}

void __cdecl R_DrawDebugShaderLitCallback(const GfxViewInfo *data, GfxCmdBufContext context)
{
    int height; // [esp+4h] [ebp-28h]
    int width; // [esp+8h] [ebp-24h]
    int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    unsigned int v6[4]; // [esp+18h] [ebp-14h] BYREF
    const GfxViewInfo *viewInfo; // [esp+28h] [ebp-4h]

    viewInfo = data;
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    height = data->scissorViewport.height;
    width = data->scissorViewport.width;
    y = data->scissorViewport.y;
    device = context.state->prim.device;
    v6[0] = data->scissorViewport.x;
    v6[1] = y;
    v6[2] = width + v6[0];
    v6[3] = height + y;
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int))device->SetRenderState)(
        device,
        device,
        174,
        1);
    device->SetScissorRect(device, (const tagRECT *)v6);
    R_DrawSurfs(context, 0, &viewInfo->litInfo);
    context.state->prim.device->SetRenderState(context.state->prim.device, D3DRS_SCISSORTESTENABLE, 0);
}

void __cdecl R_DrawFullbrightDecalCallback(const GfxViewInfo *data, GfxCmdBufContext context)
{
    int height; // [esp+4h] [ebp-28h]
    int width; // [esp+8h] [ebp-24h]
    int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    unsigned int v6[4]; // [esp+18h] [ebp-14h] BYREF
    const GfxViewInfo *viewInfo; // [esp+28h] [ebp-4h]

    viewInfo = data;
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    height = data->scissorViewport.height;
    width = data->scissorViewport.width;
    y = data->scissorViewport.y;
    device = context.state->prim.device;
    v6[0] = data->scissorViewport.x;
    v6[1] = y;
    v6[2] = width + v6[0];
    v6[3] = height + y;
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int))device->SetRenderState)(
        device,
        device,
        174,
        1);
    device->SetScissorRect(device, (const tagRECT *)v6);
    R_DrawSurfs(context, 0, &viewInfo->decalInfo);
    context.state->prim.device->SetRenderState(context.state->prim.device, D3DRS_SCISSORTESTENABLE, 0);
}

void __cdecl R_DrawDebugShaderEmissiveCallback(const GfxViewInfo *data, GfxCmdBufContext context)
{
    int height; // [esp+4h] [ebp-28h]
    int width; // [esp+8h] [ebp-24h]
    int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    unsigned int v6[4]; // [esp+18h] [ebp-14h] BYREF
    const GfxViewInfo *viewInfo; // [esp+28h] [ebp-4h]

    viewInfo = data;
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    height = data->scissorViewport.height;
    width = data->scissorViewport.width;
    y = data->scissorViewport.y;
    device = context.state->prim.device;
    v6[0] = data->scissorViewport.x;
    v6[1] = y;
    v6[2] = width + v6[0];
    v6[3] = height + y;
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int))device->SetRenderState)(
        device,
        device,
        174,
        1);
    device->SetScissorRect(device, (const tagRECT *)v6);
    R_DrawSurfs(context, 0, &viewInfo->emissiveInfo);
    R_ShowTris(context, &viewInfo->litInfo);
    R_ShowTris(context, &viewInfo->decalInfo);
    R_ShowTris(context, &viewInfo->emissiveInfo);
    context.state->prim.device->SetRenderState(context.state->prim.device, D3DRS_SCISSORTESTENABLE, 0);
}

void __cdecl RB_DebugShaderDrawCommands(const GfxViewInfo *viewInfo)
{
    const GfxBackEndData *data; // [esp+8h] [ebp-8h]
    GfxCmdBuf cmdBuf; // [esp+Ch] [ebp-4h] BYREF

    data = backEndData;
    R_SetAndClearSceneTarget(0);
    R_InitContext(data, &cmdBuf);
    R_DrawDebugShader(viewInfo, &cmdBuf);
    RB_EndSceneRendering(gfxCmdBufContext, &viewInfo->input, viewInfo);
}

void __cdecl R_DrawDebugShader(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    R_DrawFullbrightOrDebugShader(
        (int)&savedregs,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawDebugShaderLitCallback,
        viewInfo,
        &viewInfo->litInfo,
        cmdBuf);
    R_DrawFullbrightOrDebugShader(
        (int)&savedregs,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawDebugShaderDecalCallback,
        viewInfo,
        &viewInfo->decalInfo,
        cmdBuf);
    R_DrawFullbrightOrDebugShader(
        (int)&savedregs,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawDebugShaderEmissiveCallback,
        viewInfo,
        &viewInfo->emissiveInfo,
        cmdBuf);
}

void __cdecl R_DrawDebugShaderDecalCallback(char *data, GfxCmdBufContext context)
{
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    R_DrawSurfs(context, 0, (const GfxDrawSurfListInfo *)(data + 25388));
}

void __cdecl RB_StandardDrawCommands(const GfxViewInfo *viewInfo)
{
    GfxRenderTargetId setupRenderTargetId; // [esp+44h] [ebp-2Ch]
    const GfxBackEndData *data; // [esp+48h] [ebp-28h]
    GfxCmdBuf cmdBuf; // [esp+4Ch] [ebp-24h] BYREF
    bool needsDepthPrepass; // [esp+53h] [ebp-1Dh]
    ShadowType dynamicShadowType; // [esp+54h] [ebp-1Ch]
    int isRenderingFullScreen; // [esp+58h] [ebp-18h]
    float clearColor[4]; // [esp+5Ch] [ebp-14h] BYREF
    unsigned __int8 whichToClearForScene; // [esp+6Eh] [ebp-2h]
    unsigned __int8 whichToClearForSetup; // [esp+6Fh] [ebp-1h]
    int savedregs; // [esp+70h] [ebp+0h] BYREF

    data = backEndData;
    dynamicShadowType = viewInfo->dynamicShadowType;
    isRenderingFullScreen = viewInfo->isRenderingFullScreen;
    if (dynamicShadowType == SHADOW_MAP)
    {
        if (Com_BitCheckAssert(backEndData->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32))
            RB_SunShadowMaps(data, viewInfo);
        RB_SpotShadowMaps(data, viewInfo);
    }
    needsDepthPrepass = r_depthPrepass->current.enabled;
    whichToClearForScene = 7;
    R_InitContext(data, &cmdBuf);
    if (viewInfo->needsFloatZ && !renderTarget.surface.color)
        MyAssertHandler(".\\rb_draw3d.cpp", 1084, 0, "%s", "!viewInfo->needsFloatZ || R_HaveFloatZ()");
    if (viewInfo->needsFloatZ || dynamicShadowType == SHADOW_COOKIE)
    {
        if (viewInfo->needsFloatZ)
        {
            setupRenderTargetId = R_RENDERTARGET_FLOAT_Z;
            whichToClearForSetup = 6;
        }
        else
        {
            setupRenderTargetId = R_RENDERTARGET_DYNAMICSHADOWS;
            whichToClearForSetup = 7;
        }
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
        memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
        R_SetRenderTargetSize(&gfxCmdBufSourceState, setupRenderTargetId);
        R_SetRenderTarget(gfxCmdBufContext, setupRenderTargetId);
        R_ClearScreen(gfxCmdBufState.prim.device, whichToClearForSetup, colorWhite, 1.0, 0, 0);
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        R_InitContext(data, &cmdBuf);
        R_DepthPrepass((int)&savedregs, setupRenderTargetId, viewInfo, &cmdBuf);
        if (dynamicShadowType == SHADOW_COOKIE)
            RB_DrawShadowCookies(viewInfo);
        if (dx.multiSampleType == D3DMULTISAMPLE_NONE)
        {
            whichToClearForScene = 1;
            needsDepthPrepass = 0;
        }
    }
    memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
    memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
    if (R_GetClearColor(clearColor) || (whichToClearForScene & 0xFE) != 0)
        R_ClearScreen(gfxCmdBufState.prim.device, whichToClearForScene, clearColor, 1.0, 0, 0);
    memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    if (needsDepthPrepass)
    {
        R_InitContext(data, &cmdBuf);
        R_DepthPrepass((int)&savedregs, R_RENDERTARGET_SCENE, viewInfo, &cmdBuf);
    }
    R_InitContext(data, &cmdBuf);
    R_DrawLit((int)&savedregs, viewInfo, &cmdBuf, 0);
    R_InitContext(data, &cmdBuf);
    R_DrawDecal((int)&savedregs, viewInfo, &cmdBuf, 0);
    CL_ResetStats_f();
    CL_ResetStats_f();
    if (viewInfo->isRenderingFullScreen != isRenderingFullScreen)
        MyAssertHandler(".\\rb_draw3d.cpp", 1170, 0, "%s", "viewInfo->isRenderingFullScreen == isRenderingFullScreen");
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
    memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
    memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
    R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->viewParms);
    R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->sceneViewport);
    RB_DrawSun(viewInfo->localClientNum);
    memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    R_InitContext(data, &cmdBuf);
    CL_ResetStats_f();
    R_DrawLights((int)&savedregs, viewInfo, &cmdBuf);
    if (rg.distortion)
    {
        //Profile_Begin(116);
        CL_ResetStats_f();
        R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
        R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
        R_Resolve(gfxCmdBufContext, dword_EA74F2C);
        //Profile_EndInternal(0);
    }
    R_InitContext(data, &cmdBuf);
    CL_ResetStats_f();
    R_DrawEmissive((int)&savedregs, viewInfo, &cmdBuf);
    RB_EndSceneRendering(gfxCmdBufContext, &viewInfo->input, viewInfo);
}

void  R_DrawLights(int a1@<ebp>, const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    _BYTE v3[12]; // [esp-Ch] [ebp-F1Ch] BYREF
    int v4; // [esp+F04h] [ebp-Ch]
    void *v5; // [esp+F08h] [ebp-8h]
    void *retaddr; // [esp+F10h] [ebp+0h]

    v4 = a1;
    v5 = retaddr;
    R_InitCmdBufSourceState((GfxCmdBufSourceState *)v3, &viewInfo->input, 1);
    R_SetRenderTargetSize((GfxCmdBufSourceState *)v3, R_RENDERTARGET_SCENE);
    R_SetViewportStruct((GfxCmdBufSourceState *)v3, &viewInfo->sceneViewport);
    R_DrawPointLitSurfs((GfxCmdBufSourceState *)v3, viewInfo, cmdBuf);
}

void __cdecl R_DrawPointLitSurfs(GfxCmdBufSourceState *source, const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    double v3; // st7
    float s1; // [esp+18h] [ebp-144h]
    float v5; // [esp+24h] [ebp-138h]
    float v6; // [esp+28h] [ebp-134h]
    float v7; // [esp+2Ch] [ebp-130h]
    float v8; // [esp+30h] [ebp-12Ch]
    float v9; // [esp+3Ch] [ebp-120h]
    float v10; // [esp+40h] [ebp-11Ch]
    float v11; // [esp+44h] [ebp-118h]
    float v12; // [esp+48h] [ebp-114h]
    float v13; // [esp+50h] [ebp-10Ch]
    float v14; // [esp+58h] [ebp-104h]
    float v15; // [esp+5Ch] [ebp-100h]
    float v16; // [esp+60h] [ebp-FCh]
    float edgePoint[3]; // [esp+7Ch] [ebp-E0h] BYREF
    const PointLightPartition *pointLightPartitions; // [esp+88h] [ebp-D4h]
    const float *plane; // [esp+8Ch] [ebp-D0h]
    GfxPointLitSurfsInfo info; // [esp+90h] [ebp-CCh] BYREF
    int pointLightCount; // [esp+ACh] [ebp-B0h]
    const GfxBackEndData *data; // [esp+B0h] [ebp-ACh]
    float perp[3]; // [esp+B4h] [ebp-A8h] BYREF
    float dist[4]; // [esp+C0h] [ebp-9Ch]
    float halfWidth; // [esp+D0h] [ebp-8Ch]
    float width; // [esp+D4h] [ebp-88h]
    float height; // [esp+D8h] [ebp-84h]
    const GfxLight *light; // [esp+DCh] [ebp-80h]
    float sign; // [esp+E0h] [ebp-7Ch]
    const PointLightPartition *pointLightPartition; // [esp+E4h] [ebp-78h]
    float offset[3]; // [esp+E8h] [ebp-74h] BYREF
    float edgeGoalPoint[4]; // [esp+F4h] [ebp-68h] BYREF
    float edgeDir[3]; // [esp+104h] [ebp-58h] BYREF
    float planeDist; // [esp+110h] [ebp-4Ch]
    int partitionIndex; // [esp+114h] [ebp-48h]
    const GfxViewParms *viewParms; // [esp+118h] [ebp-44h]
    float tangentDistSq; // [esp+11Ch] [ebp-40h]
    GfxColor color; // [esp+120h] [ebp-3Ch] BYREF
    float x; // [esp+124h] [ebp-38h]
    float y; // [esp+128h] [ebp-34h]
    unsigned int axis; // [esp+12Ch] [ebp-30h]
    float offsetDistSq; // [esp+130h] [ebp-2Ch]
    GfxDrawPrimArgs args; // [esp+134h] [ebp-28h]
    unsigned int planeIndex; // [esp+140h] [ebp-1Ch]
    float halfHeight; // [esp+144h] [ebp-18h]
    float perpDir[3]; // [esp+148h] [ebp-14h] BYREF
    float perpDist; // [esp+154h] [ebp-8h]
    float w; // [esp+158h] [ebp-4h]
    int savedregs; // [esp+15Ch] [ebp+0h] BYREF

    if (!viewInfo)
        MyAssertHandler(".\\rb_draw3d.cpp", 597, 0, "%s", "viewInfo");
    pointLightCount = viewInfo->pointLightCount;
    if (pointLightCount)
    {
        pointLightPartitions = viewInfo->pointLightPartitions;
        R_ConvertColorToBytes(colorWhite, (unsigned __int8 *)&color);
        data = source->input.data;
        args.baseIndex = 0;
        args.vertexCount = 4;
        args.triCount = 2;
        viewParms = &viewInfo->viewParms;
        info.viewInfo = viewInfo;
        for (partitionIndex = 0; partitionIndex < pointLightCount; ++partitionIndex)
        {
            pointLightPartition = &pointLightPartitions[partitionIndex];
            light = &pointLightPartition->light;
            info.drawSurfInfo = &pointLightPartition->info;
            info.clearQuadMesh = &viewInfo->pointLightMeshData[partitionIndex];
            dist[0] = 1.0;
            dist[1] = -1.0;
            dist[2] = 1.0;
            dist[3] = -1.0;
            Vec3Sub(pointLightPartition->light.origin, viewParms->origin, offset);
            offsetDistSq = Vec3LengthSq(offset);
            tangentDistSq = offsetDistSq - light->radius * light->radius;
            if (tangentDistSq > 1.0)
            {
                v12 = sqrt(tangentDistSq);
                edgeGoalPoint[3] = v12;
                perpDist = offsetDistSq / v12;
                sign = 1.0;
                planeIndex = 0;
                while (planeIndex < 4)
                {
                    axis = planeIndex >> 1;
                    Vec3Scale(viewParms->axis[2 - (planeIndex >> 1)], sign, perp);
                    Vec3Cross(perp, offset, perpDir);
                    Vec3Normalize(perpDir);
                    s1 = -perpDist;
                    Vec3Mad(viewParms->origin, s1, perpDir, edgeGoalPoint);
                    Vec3Sub(edgeGoalPoint, light->origin, edgeDir);
                    Vec3Normalize(edgeDir);
                    Vec3Mad(light->origin, light->radius, edgeDir, edgePoint);
                    plane = viewInfo->frustumPlanes[planeIndex];
                    v3 = Vec3Dot(plane, edgePoint);
                    if (v3 + plane[3] > 0.0)
                    {
                        w = edgePoint[0] * viewParms->viewProjectionMatrix.m[0][3]
                            + edgePoint[1] * viewParms->viewProjectionMatrix.m[1][3]
                            + edgePoint[2] * viewParms->viewProjectionMatrix.m[2][3]
                            + viewParms->viewProjectionMatrix.m[3][3];
                        if (w > 0.0)
                        {
                            planeDist = edgePoint[0] * viewParms->viewProjectionMatrix.m[0][axis]
                                + edgePoint[1] * viewParms->viewProjectionMatrix.m[1][axis]
                                    + edgePoint[2] * viewParms->viewProjectionMatrix.m[2][axis]
                                        + viewParms->viewProjectionMatrix.m[3][axis];
                                        v15 = planeDist / w;
                                        v11 = v15 - 1.0;
                                        if (v11 < 0.0)
                                            v16 = planeDist / w;
                                        else
                                            v16 = 1.0;
                                        v10 = -1.0 - v15;
                                        if (v10 < 0.0)
                                            v9 = v16;
                                        else
                                            v9 = -1.0;
                                        dist[planeIndex] = v9;
                        }
                    }
                    ++planeIndex;
                    sign = -sign;
                }
            }
            halfWidth = (float)(viewInfo->sceneViewport.width >> 1);
            halfHeight = (float)(viewInfo->sceneViewport.height >> 1);
            x = (dist[1] + 1.0) * halfWidth;
            y = (1.0 - dist[2]) * halfHeight;
            width = (dist[0] - dist[1]) * halfWidth;
            height = (dist[2] - dist[3]) * halfHeight;
            v8 = floor(x);
            info.x = (int)v8;
            v7 = floor(y);
            info.y = (int)v7;
            v14 = (dist[0] + 1.0) * halfWidth;
            v6 = ceil(v14);
            info.w = (int)v6 - info.x;
            v13 = (1.0 - dist[3]) * halfHeight;
            v5 = ceil(v13);
            info.h = (int)v5 - info.y;
            info.x += viewInfo->sceneViewport.x;
            info.y += viewInfo->sceneViewport.y;
            R_SetQuadMeshData(info.clearQuadMesh, x, y, width, height, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF);
            R_DrawCall(
                (int)&savedregs,
                (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawPointLitSurfsCallback,
                &info,
                source,
                viewInfo,
                &pointLightPartition->info,
                &viewInfo->viewParms,
                cmdBuf,
                0);
        }
    }
}

void __cdecl R_DrawPointLitSurfsCallback(const GfxPointLitSurfsInfo *userData, GfxCmdBufContext context)
{
    unsigned int h; // [esp+4h] [ebp-28h]
    unsigned int w; // [esp+8h] [ebp-24h]
    unsigned int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    unsigned int v6[4]; // [esp+18h] [ebp-14h] BYREF
    const GfxPointLitSurfsInfo *info; // [esp+28h] [ebp-4h]

    info = userData;
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    R_Set2D(context.source);
    R_DrawQuadMesh(context, rgp.clearAlphaStencilMaterial, userData->clearQuadMesh);
    R_Set3D(context.source);
    h = userData->h;
    w = userData->w;
    y = userData->y;
    device = context.state->prim.device;
    v6[0] = userData->x;
    v6[1] = y;
    v6[2] = w + v6[0];
    v6[3] = h + y;
    device->SetRenderState(device, D3DRS_SCISSORTESTENABLE, 1u);
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int *))device->SetScissorRect)(device, device, v6);
    R_DrawSurfs(context, 0, info->drawSurfInfo);
    context.state->prim.device->SetRenderState(context.state->prim.device, D3DRS_SCISSORTESTENABLE, 0);
}

void  R_DrawEmissive(int a1@<ebp>, const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    _BYTE v3[12]; // [esp-Ch] [ebp-F1Ch] BYREF
    unsigned int v4[3]; // [esp+F04h] [ebp-Ch] BYREF
    _UNKNOWN *retaddr; // [esp+F10h] [ebp+0h]

    v4[0] = a1;
    v4[1] = retaddr;
    R_InitCmdBufSourceState((GfxCmdBufSourceState *)v3, &viewInfo->input, 1);
    R_SetRenderTargetSize((GfxCmdBufSourceState *)v3, R_RENDERTARGET_SCENE);
    R_SetViewportStruct((GfxCmdBufSourceState *)v3, &viewInfo->sceneViewport);
    R_DrawCall(
        (int)v4,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DrawEmissiveCallback,
        viewInfo,
        (GfxCmdBufSourceState *)v3,
        viewInfo,
        &viewInfo->emissiveInfo,
        &viewInfo->viewParms,
        cmdBuf,
        0);
}

void __cdecl R_DrawEmissiveCallback(const GfxViewInfo *userData, GfxCmdBufContext context)
{
    int height; // [esp+4h] [ebp-28h]
    int width; // [esp+8h] [ebp-24h]
    int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    unsigned int v6[4]; // [esp+18h] [ebp-14h] BYREF
    const GfxViewInfo *viewInfo; // [esp+28h] [ebp-4h]

    viewInfo = userData;
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    R_Set3D(context.source);
    height = userData->scissorViewport.height;
    width = userData->scissorViewport.width;
    y = userData->scissorViewport.y;
    device = context.state->prim.device;
    v6[0] = userData->scissorViewport.x;
    v6[1] = y;
    v6[2] = width + v6[0];
    v6[3] = height + y;
    device->SetRenderState(device, D3DRS_SCISSORTESTENABLE, 1u);
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int *))device->SetScissorRect)(device, device, v6);
    CL_ResetStats_f();
    R_DrawSurfs(context, 0, &viewInfo->emissiveInfo);
    R_ShowTris(context, &viewInfo->litInfo);
    R_ShowTris(context, &viewInfo->decalInfo);
    R_ShowTris(context, &viewInfo->emissiveInfo);
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, unsigned int))context.state->prim.device->SetRenderState)(
        context.state->prim.device,
        context.state->prim.device,
        174,
        0);
}

void __cdecl RB_Draw3DCommon()
{
    //Profile_Begin(141);
    if (!rgp.world)
        MyAssertHandler(".\\rb_draw3d.cpp", 1485, 0, "%s", "rgp.world");
    if (gfxDrawMethod.drawScene)
    {
        if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_FULLBRIGHT || gfxDrawMethod.drawScene == GFX_DRAW_SCENE_DEBUGSHADER)
        {
            RB_DebugShaderDrawCommandsCommon();
        }
        else
        {
            if (gfxDrawMethod.drawScene != GFX_DRAW_SCENE_STANDARD)
                MyAssertHandler(
                    ".\\rb_draw3d.cpp",
                    1501,
                    0,
                    "%s\n\t(gfxDrawMethod.drawScene) = %i",
                    "(gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)",
                    gfxDrawMethod.drawScene);
            RB_StandardDrawCommandsCommon();
        }
    }
    //Profile_EndInternal(0);
}

void __cdecl R_SetResolvedScene(GfxCmdBufContext context)
{
    R_SetRenderTargetSize(context.source, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
}

GfxCmdBufSourceState *RB_DebugShaderDrawCommandsCommon()
{
    GfxCmdBufSourceState *result; // eax
    const GfxBackEndData *data; // [esp+10h] [ebp-Ch]
    const GfxViewInfo *viewInfo; // [esp+14h] [ebp-8h]
    unsigned int viewInfoIndex; // [esp+18h] [ebp-4h]

    result = gfxCmdBufContext.source;
    data = backEndData;
    for (viewInfoIndex = 0; viewInfoIndex < data->viewInfoCount; ++viewInfoIndex)
    {
        viewInfo = &data->viewInfo[viewInfoIndex];
        viewInfo->input.data = data;
        R_InitCmdBufSourceState(gfxCmdBufContext.source, &gfxCmdBufInput, 0);
        gfxCmdBufContext.source->input.data = data;
        memcpy(gfxCmdBufContext.state, &gfxCmdBufState, sizeof(GfxCmdBufState));
        memset(
            (unsigned __int8 *)gfxCmdBufContext.state->vertexShaderConstState,
            0,
            sizeof(gfxCmdBufContext.state->vertexShaderConstState));
        memset(
            (unsigned __int8 *)gfxCmdBufContext.state->pixelShaderConstState,
            0,
            sizeof(gfxCmdBufContext.state->pixelShaderConstState));
        R_SetResolvedScene(gfxCmdBufContext);
        R_BeginView(gfxCmdBufContext.source, &viewInfo->sceneDef, &viewInfo->viewParms);
        R_SetViewportStruct(gfxCmdBufContext.source, &viewInfo->displayViewport);
        if (viewInfo->cmds)
            RB_ExecuteRenderCommandsLoop(viewInfo->cmds);
        memcpy(&gfxCmdBufState, gfxCmdBufContext.state, sizeof(gfxCmdBufState));
        result = (GfxCmdBufSourceState *)(viewInfoIndex + 1);
    }
    return result;
}

void RB_StandardDrawCommandsCommon()
{
    const GfxBackEndData *data; // [esp+1Ch] [ebp-Ch]
    const GfxViewInfo *viewInfo; // [esp+20h] [ebp-8h]
    GfxViewInfo *viewInfoa; // [esp+20h] [ebp-8h]
    unsigned int viewInfoIndex; // [esp+24h] [ebp-4h]
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    data = backEndData;
    if (backEndData->viewInfoCount)
    {
        CL_ResetStats_f();
        for (viewInfoIndex = 0; viewInfoIndex < data->viewInfoCount; ++viewInfoIndex)
        {
            viewInfo = &data->viewInfo[viewInfoIndex];
            viewInfo->input.data = data;
            R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
            memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
            memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
            memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
            R_SetResolvedScene(gfxCmdBufContext);
            R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->viewParms);
            R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->displayViewport);
            if (viewInfo->isRenderingFullScreen)
            {
                RB_ApplyLatePostEffects(viewInfo);
            }
            else
            {
                //Profile_Begin(116);
                if (RB_UsingColorManipulation(viewInfo))
                    RB_ApplyColorManipulationSplitscreen(viewInfo);
                //Profile_EndInternal(0);
            }
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
            RB_DrawSunPostEffects(viewInfo->localClientNum);
            memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
            R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
            gfxCmdBufSourceState.input.data = backEndData;
            memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
            memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
            memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
            R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->viewParms);
            R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->displayViewport);
            if (viewInfo->cmds)
                RB_ExecuteRenderCommandsLoop(viewInfo->cmds);
            memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        }
        viewInfoa = backEndData->viewInfo;
        R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfoa->input, 0);
        R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
        R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
        R_BeginView(&gfxCmdBufSourceState, &viewInfoa->sceneDef, &viewInfoa->viewParms);
        R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfoa->displayViewport);
        R_Set2D(&gfxCmdBufSourceState);
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
        memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
        if (viewInfoa->dynamicShadowType == SHADOW_COOKIE && sc_showOverlay->current.enabled)
            RB_ShadowCookieOverlay((int)&savedregs);
        if (viewInfoa->dynamicShadowType == SHADOW_MAP && sm_showOverlay->current.integer)
        {
            if (sm_showOverlay->current.integer != 1 && sm_showOverlay->current.integer != 2)
                MyAssertHandler(
                    ".\\rb_draw3d.cpp",
                    1432,
                    0,
                    "%s\n\t(sm_showOverlay->current.integer) = %i",
                    "(sm_showOverlay->current.integer == GFX_SM_OVERLAY_SUN || sm_showOverlay->current.integer == GFX_SM_OVERLAY_SPOT)",
                    sm_showOverlay->current.integer);
            if (sm_showOverlay->current.integer == 1)
                RB_DrawSunShadowOverlay();
            else
                RB_DrawSpotShadowOverlay();
        }
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    }
}

void __cdecl RB_ApplyLatePostEffects(const GfxViewInfo *viewInfo)
{
    //Profile_Begin(116);
    RB_ProcessPostEffects(viewInfo);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    RB_DrawDebugPostEffects();
    //Profile_EndInternal(0);
}

void RB_DrawDebugPostEffects()
{
    if (!r_showFbColorDebug)
        MyAssertHandler(".\\rb_draw3d.cpp", 845, 0, "%s", "r_showFbColorDebug");
    if (!r_showFloatZDebug)
        MyAssertHandler(".\\rb_draw3d.cpp", 846, 0, "%s", "r_showFloatZDebug");
    if (!sc_showDebug)
        MyAssertHandler(".\\rb_draw3d.cpp", 847, 0, "%s", "sc_showDebug");
    if (r_showFbColorDebug->current.integer == 1)
    {
        RB_ShowFbColorDebug_Screen();
    }
    else if (r_showFbColorDebug->current.integer == 2)
    {
        RB_ShowFbColorDebug_Feedback();
    }
    else if (r_showFloatZDebug->current.enabled)
    {
        RB_ShowFloatZDebug();
    }
    else if (sc_showDebug->current.enabled)
    {
        RB_ShowShadowsDebug();
    }
}

void RB_ShowFbColorDebug_Screen()
{
    float quarterScreenHeight; // [esp+34h] [ebp-14h]
    float halfScreenWidth; // [esp+38h] [ebp-10h]
    float halfScreenHeight; // [esp+3Ch] [ebp-Ch]
    float quarterScreenWidth; // [esp+40h] [ebp-8h]

    if (tess.indexCount)
        RB_EndTessSurface();
    R_Set2D(&gfxCmdBufSourceState);
    halfScreenWidth = (double)gfxCmdBufSourceState.renderTargetWidth * 0.5;
    halfScreenHeight = (double)gfxCmdBufSourceState.renderTargetHeight * 0.5;
    quarterScreenWidth = halfScreenWidth * 0.5;
    quarterScreenHeight = halfScreenHeight * 0.5;
    RB_DrawStretchPic(
        rgp.frameColorDebugMaterial,
        quarterScreenWidth,
        quarterScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.0,
        0.0,
        1.0,
        1.0,
        0xFFFF0000,
        GFX_PRIM_STATS_CODE);
    RB_DrawStretchPic(
        rgp.frameColorDebugMaterial,
        halfScreenWidth,
        quarterScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.0,
        0.0,
        1.0,
        1.0,
        0xFF00FF00,
        GFX_PRIM_STATS_CODE);
    RB_DrawStretchPic(
        rgp.frameColorDebugMaterial,
        quarterScreenWidth,
        halfScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.0,
        0.0,
        1.0,
        1.0,
        0xFF0000FF,
        GFX_PRIM_STATS_CODE);
    RB_DrawStretchPic(
        rgp.frameAlphaDebugMaterial,
        halfScreenWidth,
        halfScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.0,
        0.0,
        1.0,
        1.0,
        0xFFFFFFFF,
        GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
}

void RB_ShowFbColorDebug_Feedback()
{
    float quarterScreenHeight; // [esp+44h] [ebp-10h]
    float halfScreenWidth; // [esp+48h] [ebp-Ch]
    float halfScreenHeight; // [esp+4Ch] [ebp-8h]
    float quarterScreenWidth; // [esp+50h] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    halfScreenWidth = (double)gfxCmdBufSourceState.renderTargetWidth * 0.5;
    halfScreenHeight = (double)gfxCmdBufSourceState.renderTargetHeight * 0.5;
    quarterScreenWidth = halfScreenWidth * 0.5;
    quarterScreenHeight = halfScreenHeight * 0.5;
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, (const GfxImage *)dword_EA74F68);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    R_Set2D(&gfxCmdBufSourceState);
    gfxCmdBufSourceState.input.consts[29][0] = 1.0;
    gfxCmdBufSourceState.input.consts[29][1] = 0.0;
    gfxCmdBufSourceState.input.consts[29][2] = 0.0;
    gfxCmdBufSourceState.input.consts[29][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Du);
    gfxCmdBufSourceState.input.consts[30][0] = 0.0;
    gfxCmdBufSourceState.input.consts[30][1] = 0.0;
    gfxCmdBufSourceState.input.consts[30][2] = 0.0;
    gfxCmdBufSourceState.input.consts[30][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Eu);
    gfxCmdBufSourceState.input.consts[31][0] = 0.0;
    gfxCmdBufSourceState.input.consts[31][1] = 0.0;
    gfxCmdBufSourceState.input.consts[31][2] = 0.0;
    gfxCmdBufSourceState.input.consts[31][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Fu);
    RB_DrawStretchPic(
        rgp.colorChannelMixerMaterial,
        quarterScreenWidth,
        quarterScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.25,
        0.25,
        0.5,
        0.5,
        0xFFFFFFFF,
        GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
    gfxCmdBufSourceState.input.consts[29][0] = 0.0;
    gfxCmdBufSourceState.input.consts[29][1] = 0.0;
    gfxCmdBufSourceState.input.consts[29][2] = 0.0;
    gfxCmdBufSourceState.input.consts[29][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Du);
    gfxCmdBufSourceState.input.consts[30][0] = 0.0;
    gfxCmdBufSourceState.input.consts[30][1] = 1.0;
    gfxCmdBufSourceState.input.consts[30][2] = 0.0;
    gfxCmdBufSourceState.input.consts[30][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Eu);
    gfxCmdBufSourceState.input.consts[31][0] = 0.0;
    gfxCmdBufSourceState.input.consts[31][1] = 0.0;
    gfxCmdBufSourceState.input.consts[31][2] = 0.0;
    gfxCmdBufSourceState.input.consts[31][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Fu);
    RB_DrawStretchPic(
        rgp.colorChannelMixerMaterial,
        halfScreenWidth,
        quarterScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.5,
        0.25,
        0.75,
        0.5,
        0xFFFFFFFF,
        GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
    Vec4Set(gfxCmdBufSourceState.input.consts[29], 0.0, 0.0, 0.0, 0.0);
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Du);
    Vec4Set(gfxCmdBufSourceState.input.consts[30], 0.0, 0.0, 0.0, 0.0);
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Eu);
    Vec4Set(gfxCmdBufSourceState.input.consts[31], 0.0, 0.0, 1.0, 0.0);
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Fu);
    RB_DrawStretchPic(
        rgp.colorChannelMixerMaterial,
        quarterScreenWidth,
        halfScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.25,
        0.5,
        0.5,
        0.75,
        0xFFFFFFFF,
        GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
    R_SetCodeConstant(&gfxCmdBufSourceState, 0x1Du, 0.0, 0.0, 0.0, 1.0);
    R_SetCodeConstant(&gfxCmdBufSourceState, 0x1Eu, 0.0, 0.0, 0.0, 1.0);
    R_SetCodeConstant(&gfxCmdBufSourceState, 0x1Fu, 0.0, 0.0, 0.0, 1.0);
    RB_DrawStretchPic(
        rgp.colorChannelMixerMaterial,
        halfScreenWidth,
        halfScreenHeight,
        quarterScreenWidth,
        quarterScreenHeight,
        0.5,
        0.5,
        0.75,
        0.75,
        0xFFFFFFFF,
        GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
}

void RB_ShowFloatZDebug()
{
    float quarterScreenHeight; // [esp+28h] [ebp-10h]
    float halfScreenWidth; // [esp+2Ch] [ebp-Ch]
    float halfScreenHeight; // [esp+30h] [ebp-8h]
    float quarterScreenWidth; // [esp+34h] [ebp-4h]

    if (renderTarget.surface.color)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        halfScreenWidth = (double)gfxCmdBufSourceState.renderTargetWidth * 0.5;
        halfScreenHeight = (double)gfxCmdBufSourceState.renderTargetHeight * 0.5;
        quarterScreenWidth = halfScreenWidth * 0.5;
        quarterScreenHeight = halfScreenHeight * 0.5;
        R_Set2D(&gfxCmdBufSourceState);
        RB_DrawStretchPic(
            rgp.floatZDisplayMaterial,
            quarterScreenWidth,
            quarterScreenHeight,
            halfScreenWidth,
            halfScreenHeight,
            0.25,
            0.25,
            0.75,
            0.75,
            0xFFFFFFFF,
            GFX_PRIM_STATS_CODE);
        RB_EndTessSurface();
    }
}

void RB_ShowShadowsDebug()
{
    float quarterScreenHeight; // [esp+38h] [ebp-10h]
    float halfScreenWidth; // [esp+3Ch] [ebp-Ch]
    float halfScreenHeight; // [esp+40h] [ebp-8h]
    float quarterScreenWidth; // [esp+44h] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    halfScreenWidth = (double)gfxCmdBufSourceState.renderTargetWidth * 0.5;
    halfScreenHeight = (double)gfxCmdBufSourceState.renderTargetHeight * 0.5;
    quarterScreenWidth = halfScreenWidth * 0.5;
    quarterScreenHeight = halfScreenHeight * 0.5;
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    R_Set2D(&gfxCmdBufSourceState);
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, (const GfxImage *)dword_EA74F68);
    gfxCmdBufSourceState.input.consts[29][0] = 1.0;
    gfxCmdBufSourceState.input.consts[29][1] = 0.0;
    gfxCmdBufSourceState.input.consts[29][2] = 0.0;
    gfxCmdBufSourceState.input.consts[29][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Du);
    gfxCmdBufSourceState.input.consts[30][0] = 0.0;
    gfxCmdBufSourceState.input.consts[30][1] = 1.0;
    gfxCmdBufSourceState.input.consts[30][2] = 0.0;
    gfxCmdBufSourceState.input.consts[30][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Eu);
    gfxCmdBufSourceState.input.consts[31][0] = 0.0;
    gfxCmdBufSourceState.input.consts[31][1] = 0.0;
    gfxCmdBufSourceState.input.consts[31][2] = 1.0;
    gfxCmdBufSourceState.input.consts[31][3] = 0.0;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 0x1Fu);
    RB_DrawStretchPic(
        rgp.colorChannelMixerMaterial,
        quarterScreenWidth,
        quarterScreenHeight,
        halfScreenWidth,
        halfScreenHeight,
        0.25,
        0.25,
        0.75,
        0.75,
        0xFFFFFFFF,
        GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
}

