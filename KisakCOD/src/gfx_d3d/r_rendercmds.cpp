#include "r_rendercmds.h"
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>
#include "rb_logfile.h"
#include "r_utils.h"
#include "r_model_lighting.h"
#include "r_scene.h"
#include "r_dpvs.h"
#include "r_meshdata.h"
#include "r_spotshadow.h"
#include "r_buffers.h"
#include "r_model.h"
#include "r_workercmds.h"
#include "r_cinematic.h"
#include "r_dvars.h"
#include <client/client.h>
#include "r_bsp.h"
#include "r_sky.h"
#include "r_draw_method.h"
#include <xanim/xmodel.h>
#include <win32/win_net.h>
#include <database/database.h>
#include <cgame/cg_local.h>
#include "r_drawsurf.h"
#include "rb_state.h"

//  struct GfxBackEndData *frontEndDataOut 85827c80     gfx_d3d : r_rendercmds.obj
GfxBackEndData *frontEndDataOut;
//  int marker_r_rendercmds  85827cb8     gfx_d3d : r_rendercmds.obj

struct GfxDebugFrameGlob // sizeof=0x11E880
{                                       // ...
    GfxCmdArray *restoreCmdList;        // ...
    GfxBackEndData *restoreFrontEndDataOut; // ...
    bool restoreSkinnedCache;           // ...
    GfxBackEndData frontEndDataOut;     // ...
    bool inFrame;                       // ...
};

GfxBackEndData s_backEndData[2];
GfxViewInfo g_viewInfo[2][4];
GfxCmdArray g_frontEndCmds[2];
GfxDebugFrameGlob s_debugFrameGlob;
GfxCmdArray g_debugFrontEndCmds;

static unsigned int s_renderCmdBufferSize;
static int s_renderCmdWarnSize;

void __cdecl TRACK_r_rendercmds()
{
    track_static_alloc_internal((void *)s_backEndData, 2346752, "s_backEndData", 18);
    track_static_alloc_internal(g_viewInfo, 212352, "g_viewInfo", 18);
    track_static_alloc_internal(g_frontEndCmds, 32, "g_frontEndCmds", 18);
    track_static_alloc_internal(&s_debugFrameGlob, 1173632, "s_debugFrameGlob", 0);
    track_static_alloc_internal(&g_debugFrontEndCmds, 16, "g_debugFrontEndCmds", 0);
}

void __cdecl R_FreeGlobalVariable(void *var)
{
    Z_VirtualFree(var);
}

void __cdecl R_ShutdownSceneBuffers()
{
    unsigned int localClientNum; // [esp+0h] [ebp-8h]
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    for (viewIndex = 0; viewIndex < 7; ++viewIndex)
        R_FreeGlobalVariable(scene.dpvs.entVisData[viewIndex]);
    R_FreeGlobalVariable(scene.dpvs.sceneXModelIndex);
    R_FreeGlobalVariable(scene.dpvs.sceneDObjIndex);
    for (localClientNum = 0; localClientNum < gfxCfg.maxClientViews; ++localClientNum)
    {
        R_FreeGlobalVariable(dpvsGlob.entVisBits[localClientNum]);
        R_FreeGlobalVariable(scene.dpvs.entInfo[localClientNum]);
    }
}

void __cdecl R_ShutdownRenderCommands()
{
    unsigned int dataIndex; // [esp+4h] [ebp-4h]

    R_ShutdownModelLightingGlobals();
    for (dataIndex = 0; dataIndex < 2; ++dataIndex)
    {
        R_FreeGlobalVariable(s_backEndData[dataIndex].commands->cmds);
        R_ShutdownDebugEntry(&s_backEndData[dataIndex].debugGlobals);
    }
    R_FreeGlobalVariable(g_debugFrontEndCmds.cmds);
    R_ShutdownSceneBuffers();
}

void __cdecl R_ShutdownRenderBuffers()
{
    GfxBackEndData *data; // [esp+0h] [ebp-10h]
    unsigned int partitionIndex; // [esp+4h] [ebp-Ch]
    int dataIndex; // [esp+8h] [ebp-8h]
    unsigned int viewIndex; // [esp+Ch] [ebp-4h]
    unsigned int viewIndexa; // [esp+Ch] [ebp-4h]

    for (dataIndex = 0; dataIndex < 2; ++dataIndex)
    {
        data = &s_backEndData[dataIndex];
        data->endFence = 0;
        data->preTessIb = 0;
        R_ShutdownDynamicMesh(&data->codeMesh);
        R_ShutdownDynamicMesh(&data->markMesh);
        for (viewIndex = 0; viewIndex < 4; ++viewIndex)
        {
            for (partitionIndex = 0; partitionIndex < 4; ++partitionIndex)
                R_ShutdownDynamicMesh(&g_viewInfo[dataIndex][viewIndex].pointLightMeshData[partitionIndex]);
        }
    }
    dx.swapFence = 0;
    for (viewIndexa = 0; viewIndexa < 4; ++viewIndexa)
        R_ShutdownDynamicMesh(&gfxMeshGlob.fullSceneViewMesh[viewIndexa].meshData);
    R_ShutdownSpotShadowMeshes();
    R_ShutdownDynamicIndices(&gfxBuf.smodelCache);
}

void __cdecl R_ShutdownDynamicMesh(GfxMeshData *mesh)
{
    IDirect3DVertexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    R_FreeGlobalVariable(mesh->indices);
    if (mesh->vb.buffer)
    {
        do
        {
            if (r_logFile)
            {
                if (r_logFile->current.integer)
                    RB_LogPrint("mesh->vb.buffer->Release()\n");
            }
            varCopy = mesh->vb.buffer;
            mesh->vb.buffer = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>((IDirect3DSurface9 *)varCopy, "mesh->vb.buffer", ".\\r_rendercmds.cpp", 246);
        } while (alwaysfails);
    }
}

void __cdecl R_InitRenderCommands()
{
    unsigned int dataIndex; // [esp+4h] [ebp-4h]

    s_renderCmdBufferSize = 98304 * gfxCfg.maxClientViews;
    s_renderCmdWarnSize = (signed int)(294912 * gfxCfg.maxClientViews) / 4;
    R_InitModelLightingGlobals();
    for (dataIndex = 0; dataIndex < 2; ++dataIndex)
    {
        g_frontEndCmds[dataIndex].cmds = (unsigned __int8 *)R_AllocGlobalVariable(s_renderCmdBufferSize, "rendercmds");
        R_InitDebugEntry(&s_backEndData[dataIndex].debugGlobals);
    }
    g_debugFrontEndCmds.cmds = (unsigned __int8 *)R_AllocGlobalVariable(s_renderCmdBufferSize, "rendercmds (debug)");
    s_debugFrameGlob.frontEndDataOut.commands = &g_debugFrontEndCmds;
    R_InitSceneBuffers();
    if (frontEndDataOut)
    {
        R_ClearCmdList();
    }
    else
    {
        R_UnlockSkinnedCache();
        R_ToggleSmpFrame();
    }
}

void __cdecl R_InitRenderBuffers()
{
    float w; // [esp+8h] [ebp-3Ch]
    float h; // [esp+Ch] [ebp-38h]
    unsigned int partitionIndex; // [esp+38h] [ebp-Ch]
    unsigned int dataIndex; // [esp+3Ch] [ebp-8h]
    unsigned int viewIndex; // [esp+40h] [ebp-4h]
    unsigned int viewIndexa; // [esp+40h] [ebp-4h]

    for (dataIndex = 0; dataIndex < 2; ++dataIndex)
    {
        R_InitDynamicMesh(&s_backEndData[dataIndex].codeMesh, 0x6000u, 0x4000u, 0x20u);
        R_InitDynamicMesh(&s_backEndData[dataIndex].markMesh, 0x2400u, 0x1800u, 0x2Cu);
        for (viewIndex = 0; viewIndex < 4; ++viewIndex)
        {
            for (partitionIndex = 0; partitionIndex < 4; ++partitionIndex)
                R_InitDynamicMesh(&g_viewInfo[dataIndex][viewIndex].pointLightMeshData[partitionIndex], 6u, 4u, 0x20u);
        }
    }
    for (viewIndexa = 0; viewIndexa < 4; ++viewIndexa)
    {
        R_InitDynamicMesh(&gfxMeshGlob.fullSceneViewMesh[viewIndexa].meshData, 6u, 4u, 0x20u);
        h = (float)vidConfig.sceneHeight;
        w = (float)vidConfig.sceneWidth;
        R_SetQuadMesh(&gfxMeshGlob.fullSceneViewMesh[viewIndexa], 0.0, 0.0, w, h, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF);
    }
    R_InitSpotShadowMeshes();
    R_InitDynamicIndices(&gfxBuf.smodelCache, 0x100000);
}

void __cdecl R_InitDynamicMesh(
    GfxMeshData *mesh,
    unsigned int indexCount,
    unsigned int vertCount,
    unsigned int vertSize)
{
    mesh->indices = (unsigned __int16 *)R_AllocGlobalVariable(2 * indexCount, "R_InitDynamicMesh");
    mesh->totalIndexCount = indexCount;
    mesh->indexCount = 0;
    mesh->vertSize = vertSize;
    R_InitDynamicVertexBufferState(&mesh->vb, vertSize * vertCount);
}

void __cdecl R_InitRenderThread()
{
    if (!Sys_SpawnRenderThread((void(__cdecl *)(unsigned int))RB_RenderThread))
        Com_Error(ERR_FATAL, "Failed to create render thread");
}

void __cdecl R_SyncRenderThread()
{
    if (!Sys_IsRenderThread())
    {
        if (!Sys_IsMainThread())
            MyAssertHandler(".\\r_rendercmds.cpp", 605, 0, "%s", "Sys_IsMainThread()");
        if (rg.registered)
        {
            if (!dx.device)
                MyAssertHandler(".\\r_rendercmds.cpp", 613, 0, "%s", "dx.device");
            if (r_glob.remoteScreenUpdateNesting)
                MyAssertHandler(".\\r_rendercmds.cpp", 617, 0, "%s", "r_glob.remoteScreenUpdateNesting == 0");
            if (r_glob.startedRenderThread && !r_glob.haveThreadOwnership)
            {
                //Profile_Begin(173);
                Sys_FrontEndSleep();
                //Profile_EndInternal(0);
                r_glob.haveThreadOwnership = 1;
            }
        }
    }
}

GfxCmdArray *R_ClearCmdList()
{
    GfxCmdArray *commands; // edx
    GfxCmdArray *result; // eax

    commands = frontEndDataOut->commands;
    commands->usedTotal = 0;
    result = commands;
    commands->usedCritical = 0;
    commands->lastCmd = 0;
    return result;
}

void __cdecl R_ReleaseThreadOwnership()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 665, 0, "%s", "Sys_IsMainThread()");
    if (r_glob.startedRenderThread)
    {
        if (!frontEndDataOut)
            MyAssertHandler(".\\r_rendercmds.cpp", 670, 0, "%s", "frontEndDataOut");
        if (r_glob.haveThreadOwnership)
        {
            Sys_ReleaseThreadOwnership();
            r_glob.haveThreadOwnership = 0;
        }
    }
}

void __cdecl R_IssueRenderCommands(unsigned int type)
{
    bool v1; // [esp+1Eh] [ebp-16h]

    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 795, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    if (R_CheckLostDevice())
    {
        //Profile_Begin(15);
        frontEndDataOut->drawType = type;
        if (!R_HandOffToBackend(type))
        {
            if ((type & 2) != 0)
                R_PerformanceCounters();
            if (Sys_IsMainThread())
                R_WaitFrontendWorkerCmds();
            R_UpdateSkinCacheUsage();
            if (R_CheckLostDevice())
                v1 = g_disableRendering == 0;
            else
                v1 = 0;
            if (v1)
            {
                RB_BeginFrame(frontEndDataOut);
                RB_Draw3D();
                RB_CallExecuteRenderCommands();
                RB_EndFrame(frontEndDataOut->drawType);
            }
            R_UnlockSkinnedCache();
            R_ToggleSmpFrame();
        }
        //Profile_EndInternal(0);
    }
    else
    {
        if (Sys_IsMainThread())
            R_WaitFrontendWorkerCmds();
        R_Cinematic_UpdateFrame();
        R_UnlockSkinnedCache();
        R_ToggleSmpFrame();
    }
}

void R_PerformanceCounters()
{
    CL_ResetStats_f();
    //Profile_ResetCounters(0);
    if (rg.stats)
        RB_CopyBackendStats();
}

bool R_UpdateSkinCacheUsage()
{
    bool result; // eax

    if (!frontEndDataOut->skinnedCacheVb)
        MyAssertHandler(".\\r_rendercmds.cpp", 650, 0, "%s", "frontEndDataOut->skinnedCacheVb");
    result = frontEndDataOut->skinnedCacheVb->used >= 0x410000u;
    rg.skinnedCacheReachedThreshold = result;
    return result;
}

char __cdecl R_HandOffToBackend(char type)
{
    bool v2; // [esp+3h] [ebp-1h]

    if (r_smp_backend->current.enabled)
        v2 = sys_smp_allowed->current.enabled && !r_glob.isRenderingRemoteUpdate;
    else
        v2 = 0;
    if (v2)
    {
        R_ToggleSmpFrameCmd(type);
        return 1;
    }
    else
    {
        if (Sys_IsMainThread())
            R_SyncRenderThread();
        return 0;
    }
}

void __cdecl R_ToggleSmpFrameCmd(char type)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 687, 0, "%s", "Sys_IsMainThread()");
    R_ReleaseThreadOwnership();
    //Profile_Begin(133);
    CL_ResetStats_f();
    R_ProcessWorkerCmdsWithTimeout(Sys_IsRendererReady, 1);
    //Profile_EndInternal(0);
    if ((type & 2) != 0)
        R_PerformanceCounters();
    R_WaitFrontendWorkerCmds();
    R_UpdateSkinCacheUsage();
    R_UpdateActiveWorkerThreads();
    R_UnlockSkinnedCache();
    CL_ResetStats_f();
    Sys_WakeRenderer((void *)frontEndDataOut);
    if (r_glob.haveThreadOwnership)
        MyAssertHandler(".\\r_rendercmds.cpp", 729, 0, "%s", "!r_glob.haveThreadOwnership");
    R_ToggleSmpFrame();
}

void __cdecl R_AbortRenderCommands()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 851, 0, "%s", "Sys_IsMainThread()");
    if (rg.registered)
    {
        R_ClearCmdList();
        R_UnlockSkinnedCache();
        rg.viewInfoCount = 0;
        R_ToggleSmpFrame();
        rg.inFrame = 0;
    }
}

GfxCmdArray *s_cmdList;

void __cdecl R_BeginClientCmdList2D()
{
    frontEndDataOut->viewInfo[frontEndDataOut->viewInfoCount].cmds = &s_cmdList->cmds[s_cmdList->usedTotal];
}

void __cdecl R_ClearClientCmdList2D()
{
    frontEndDataOut->viewInfo[frontEndDataOut->viewInfoCount].cmds = 0;
}

void __cdecl R_BeginSharedCmdList()
{
    frontEndDataOut->cmds = &s_cmdList->cmds[s_cmdList->usedTotal];
}

void __cdecl R_AddCmdEndOfList()
{
    R_GetCommandBuffer(RC_END_OF_LIST, 4);
}

GfxCmdHeader *__cdecl R_GetCommandBuffer(GfxRenderCommand renderCmd, int bytes)
{
    const char *v2; // eax
    GfxCmdHeader *header; // [esp+8h] [ebp-8h]
    int sizeLimit; // [esp+Ch] [ebp-4h]

    if ((unsigned int)renderCmd >= RC_COUNT)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            881,
            0,
            "%s\n\t(renderCmd) = %i",
            "(renderCmd >= 0 && renderCmd < RC_COUNT)",
            renderCmd);
    if ((bytes & 3) != 0)
        MyAssertHandler(".\\r_rendercmds.cpp", 882, 0, "%s\n\t(bytes) = %i", "((bytes & 3) == 0)", bytes);
    if (bytes >= (int)s_renderCmdBufferSize)
        MyAssertHandler(".\\r_rendercmds.cpp", 883, 0, "%s\n\t(bytes) = %i", "(bytes < s_renderCmdBufferSize)", bytes);
    if (bytes != (unsigned __int16)bytes)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            884,
            0,
            "%s\n\t(bytes) = %i",
            "(bytes == static_cast< unsigned short >( bytes ))",
            bytes);
    if (!s_cmdList)
        MyAssertHandler(".\\r_rendercmds.cpp", 885, 0, "%s", "s_cmdList");
    if (!s_cmdList->cmds)
        MyAssertHandler(".\\r_rendercmds.cpp", 886, 0, "%s", "s_cmdList->cmds");
    if (!rg.inFrame)
        MyAssertHandler(".\\r_rendercmds.cpp", 887, 0, "%s", "rg.inFrame");
    if (renderCmd < RC_FIRST_NONCRITICAL && s_cmdList->usedCritical < 7680 && bytes + s_cmdList->usedCritical >= 7680)
        Com_PrintWarning(8, "RENDERCOMMAND_CRITICAL_WARN_SIZE (%i bytes) reached\n", 7680);
    if (s_cmdList->usedTotal < s_renderCmdWarnSize && bytes + s_cmdList->usedTotal >= s_renderCmdWarnSize)
        Com_PrintWarning(8, "RENDERCOMMAND_WARN_SIZE (%.0f KB) reached\n", (double)s_renderCmdWarnSize / 1024.0);
    sizeLimit = s_renderCmdBufferSize - s_cmdList->usedTotal;
    if (renderCmd >= RC_FIRST_NONCRITICAL)
        sizeLimit -= 0x2000 - s_cmdList->usedCritical;
    if (bytes <= sizeLimit)
    {
        header = (GfxCmdHeader *)&s_cmdList->cmds[s_cmdList->usedTotal];
        s_cmdList->usedTotal += bytes;
        s_cmdList->usedCritical += renderCmd >= RC_FIRST_NONCRITICAL ? 0 : bytes;
        s_cmdList->lastCmd = header;
        header->id = renderCmd;
        header->byteCount = bytes;
        return header;
    }
    else
    {
        if (renderCmd < RC_FIRST_NONCRITICAL)
        {
            v2 = va("rc %i used %i critical %i bytes %i", renderCmd, s_cmdList->usedTotal, s_cmdList->usedCritical, bytes);
            MyAssertHandler(".\\r_rendercmds.cpp", 904, 0, "%s\n\t%s", "renderCmd >= RC_FIRST_NONCRITICAL", v2);
        }
        s_cmdList->lastCmd = 0;
        return 0;
    }
}

void R_FreeTempSkinBuffer()
{
    if (frontEndDataOut->tempSkinPos)
    {
        Z_VirtualDecommit(frontEndDataOut->tempSkinBuf, frontEndDataOut->tempSkinPos);
        frontEndDataOut->tempSkinPos = 0;
    }
}

unsigned int s_smpFrame;
unsigned int g_frameIndex;
DebugGlobals *R_ToggleSmpFrame()
{
    DebugGlobals *result; // eax
    volatile int surfPos; // [esp+0h] [ebp-Ch]
    DebugGlobals *debugGlobalsEntry; // [esp+8h] [ebp-4h]

    if (!rg.viewInfoCount)
        CG_CalculateFPS();
    s_smpFrame = (s_smpFrame + 1) % 2;
    ++rg.frontEndFrameCount;
    gfxBuf.dynamicBufferFrame = (gfxBuf.dynamicBufferFrame + 1) % 2;
    gfxBuf.preTessBufferFrame = (gfxBuf.preTessBufferFrame + 1) % 2;
    frontEndDataOut = &s_backEndData[s_smpFrame];
    if (!rg.frontEndFrameCount)
        MyAssertHandler(".\\r_rendercmds.cpp", 993, 0, "%s", "rg.frontEndFrameCount > 0");
    R_FreeTempSkinBuffer();
    if (!frontEndDataOut)
        MyAssertHandler(".\\r_rendercmds.cpp", 999, 0, "%s", "frontEndDataOut");
    frontEndDataOut->frameCount = rg.frontEndFrameCount;
    frontEndDataOut->viewInfoCount = rg.viewInfoCount;
    frontEndDataOut->viewInfo = g_viewInfo[g_frameIndex];
    frontEndDataOut->commands = &g_frontEndCmds[g_frameIndex];
    if (!rg.viewInfoCount)
        R_ToggleModelLightingFrame();
    frontEndDataOut->skinnedCacheVb = &gfxBuf.skinnedCacheVbPool[gfxBuf.dynamicBufferFrame];
    gfxBuf.preTessIndexBuffer = &gfxBuf.preTessIndexBufferPool[gfxBuf.preTessBufferFrame];
    frontEndDataOut->preTessIb = gfxBuf.preTessIndexBufferPool[gfxBuf.preTessBufferFrame].buffer;
    frontEndDataOut->smcPatchCount = 0;
    frontEndDataOut->smcPatchVertsUsed = 0;
    frontEndDataOut->modelLightingPatchCount = 0;
    frontEndDataOut->skinnedCacheVb->used = 0;
    s_cmdList = frontEndDataOut->commands;
    CL_ResetStats_f();
    if (frontEndDataOut->drawSurfCount > 0x8000u)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            1037,
            0,
            "frontEndDataOut->drawSurfCount not in [0, static_cast< int >( ARRAY_COUNT( frontEndDataOut->drawSurfs ) )]\n"
            "\t%i not in [%i, %i]",
            frontEndDataOut->drawSurfCount,
            0,
            0x8000);
    if (frontEndDataOut->surfPos < 0)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            1038,
            0,
            "%s\n\t(frontEndDataOut->surfPos) = %i",
            "(frontEndDataOut->surfPos >= 0)",
            frontEndDataOut->surfPos);
    if (frontEndDataOut->surfPos > 0x20000)
        surfPos = 0x20000;
    else
        surfPos = frontEndDataOut->surfPos;
    frontEndDataOut->surfPos = surfPos;
    if (frontEndDataOut->cloudCount > 0x100u)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            1040,
            0,
            "frontEndDataOut->cloudCount not in [0, GFX_PARTICLE_CLOUD_LIMIT]\n\t%i not in [%i, %i]",
            frontEndDataOut->cloudCount,
            0,
            256);
    Com_Memset(frontEndDataOut->drawSurfs, 176, 8 * frontEndDataOut->drawSurfCount);
    Com_Memset(frontEndDataOut->surfsBuffer, 176, frontEndDataOut->surfPos);
    Com_Memset(frontEndDataOut->clouds, 176, frontEndDataOut->cloudCount << 6);
    Com_Memset(&frontEndDataOut->codeMeshes[0].triCount, 176, 0x8000);
    Com_Memset(frontEndDataOut->primDrawSurfsBuf, 176, 4 * frontEndDataOut->primDrawSurfPos);
    Com_Memset(&frontEndDataOut->fogSettings, 176, 20);
    frontEndDataOut->drawSurfCount = 0;
    frontEndDataOut->primDrawSurfPos = 0;
    frontEndDataOut->surfPos = 0;
    frontEndDataOut->gfxEntCount = 1;
    frontEndDataOut->cloudCount = 0;
    frontEndDataOut->codeMeshCount = 0;
    frontEndDataOut->codeMeshArgsCount = 0;
    R_ResetMesh(&frontEndDataOut->codeMesh);
    frontEndDataOut->markMeshCount = 0;
    R_ResetMesh(&frontEndDataOut->markMesh);
    frontEndDataOut->viewParmCount = 0;
    frontEndDataOut->cmds = 0;
    debugGlobalsEntry = &frontEndDataOut->debugGlobals;
    frontEndDataOut->debugGlobals.lineCount = 0;
    debugGlobalsEntry->stringCount = 0;
    result = debugGlobalsEntry;
    debugGlobalsEntry->vertCount = 0;
    return result;
}

GfxViewParms *__cdecl R_AllocViewParms()
{
    if (!frontEndDataOut)
        MyAssertHandler(".\\r_rendercmds.cpp", 1128, 0, "%s", "frontEndDataOut");
    if (frontEndDataOut->viewParmCount >= 0x1Cu)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            1129,
            0,
            "frontEndDataOut->viewParmCount doesn't index ARRAY_COUNT( frontEndDataOut->viewParms )\n\t%i not in [0, %i)",
            frontEndDataOut->viewParmCount,
            28);
    return &frontEndDataOut->viewParms[frontEndDataOut->viewParmCount++];
}

void __cdecl R_AddCmdDrawStretchPic(
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    const float *color,
    Material *material)
{
    const char *v10; // eax
    const char *Name; // eax
    Material *defaultMaterial; // [esp+0h] [ebp-1Ch]
    Material *actualMaterial; // [esp+14h] [ebp-8h]
    GfxCmdStretchPic *cmd; // [esp+18h] [ebp-4h]

    if (material)
        defaultMaterial = (Material *)Material_FromHandle(material);
    else
        defaultMaterial = rgp.defaultMaterial;
    actualMaterial = defaultMaterial;
    if (!Material_HasAnyFogableTechnique(defaultMaterial) || Material_IsDefault(defaultMaterial))
    {
        if ((defaultMaterial->stateFlags & 0x10) != 0)
        {
            Name = Material_GetName(material);
            Com_PrintWarning(
                8,
                "R_AddCmdDrawStretchPic: NOT DRAWING WITH MATERIAL \"%s\", because it uses the depth buffer. Set materialType to 2d.\n",
                Name);
            actualMaterial = rgp.defaultMaterial;
        }
    }
    else
    {
        v10 = Material_GetName(material);
        Com_PrintWarning(
            8,
            "R_AddCmdDrawStretchPic: NOT DRAWING WITH MATERIAL \"%s\", because it has a fogable technique.\n",
            v10);
        actualMaterial = rgp.defaultMaterial;
    }
    if ((actualMaterial->stateFlags & 0x10) != 0)
        MyAssertHandler(".\\r_rendercmds.cpp", 1160, 0, "%s", "!Material_UsesDepthBuffer( actualMaterial )");
    cmd = (GfxCmdStretchPic *)R_GetCommandBuffer(RC_FIRST_NONCRITICAL, 44);
    if (cmd)
    {
        cmd->material = actualMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->s0 = s0;
        cmd->t0 = t0;
        cmd->s1 = s1;
        cmd->t1 = t1;
        R_ConvertColorToBytes(color, &cmd->color);
    }
}

bool __cdecl Material_HasAnyFogableTechnique(const Material *material)
{
    return Material_GetTechnique(material, TECHNIQUE_LIT_BEGIN) || Material_GetTechnique(material, TECHNIQUE_EMISSIVE);
}

const MaterialTechnique *__cdecl Material_GetTechnique(const Material *material, MaterialTechniqueType techType)
{
    const MaterialTechnique *technique; // [esp+0h] [ebp-8h]
    const MaterialTechniqueSet *techSet; // [esp+4h] [ebp-4h]

    techSet = Material_GetTechniqueSet(material);
    if (!techSet)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h", 316, 0, "%s", "techSet");
    technique = techSet->techniques[techType];
    if (technique
        && technique->passArray[0].pixelShader->prog.loadDef.loadForRenderer != r_rendererInUse->current.integer)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h",
            320,
            0,
            "technique->passArray[0].pixelShader->prog.loadDef.loadForRenderer == r_rendererInUse->current.integer\n\t%i, %i",
            technique->passArray[0].pixelShader->prog.loadDef.loadForRenderer,
            r_rendererInUse->current.integer);
    }
    return technique;
}

MaterialTechniqueSet *__cdecl Material_GetTechniqueSet(const Material *material)
{
    if (!material)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h", 299, 0, "%s", "material");
    if (!material->techniqueSet)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h",
            300,
            0,
            "%s\n\t(material->info.name) = %s",
            "(material->techniqueSet)",
            material->info.name);
    return material->techniqueSet->remappedTechniqueSet;
}

void __cdecl R_AddCmdDrawStretchPicFlipST(
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    const float *color,
    Material *material)
{
    const char *v10; // eax
    const char *Name; // eax
    Material *defaultMaterial; // [esp+0h] [ebp-1Ch]
    Material *actualMaterial; // [esp+14h] [ebp-8h]
    GfxCmdStretchPic *cmd; // [esp+18h] [ebp-4h]

    if (material)
        defaultMaterial = (Material *)Material_FromHandle(material);
    else
        defaultMaterial = rgp.defaultMaterial;
    actualMaterial = defaultMaterial;
    if (!Material_HasAnyFogableTechnique(defaultMaterial) || Material_IsDefault(defaultMaterial))
    {
        if ((defaultMaterial->stateFlags & 0x10) != 0)
        {
            Name = Material_GetName(material);
            Com_PrintWarning(
                8,
                "R_AddCmdDrawStretchPicFlipST: NOT DRAWING WITH MATERIAL \"%s\", because it uses the depth buffer. Set materialType to 2d.\n",
                Name);
            actualMaterial = rgp.defaultMaterial;
        }
    }
    else
    {
        v10 = Material_GetName(material);
        Com_PrintWarning(
            8,
            "R_AddCmdDrawStretchPicFlipST: NOT DRAWING WITH MATERIAL \"%s\", because it has a fogable technique.\n",
            v10);
        actualMaterial = rgp.defaultMaterial;
    }
    if ((actualMaterial->stateFlags & 0x10) != 0)
        MyAssertHandler(".\\r_rendercmds.cpp", 1198, 0, "%s", "!Material_UsesDepthBuffer( actualMaterial )");
    cmd = (GfxCmdStretchPic *)R_GetCommandBuffer(RC_STRETCH_PIC_FLIP_ST, 44);
    if (cmd)
    {
        cmd->material = actualMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->s0 = s0;
        cmd->t0 = t0;
        cmd->s1 = s1;
        cmd->t1 = t1;
        R_ConvertColorToBytes(color, (unsigned int *)&cmd->color);
    }
}

void __cdecl R_AddCmdDrawStretchPicRotateXY(
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    float angle,
    const float *color,
    Material *material)
{
    Material *defaultMaterial; // [esp+4h] [ebp-8h]
    GfxCmdStretchPicRotateXY *cmd; // [esp+8h] [ebp-4h]

    cmd = (GfxCmdStretchPicRotateXY *)R_GetCommandBuffer(RC_STRETCH_PIC_ROTATE_XY, 48);
    if (cmd)
    {
        if (material)
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->s0 = s0;
        cmd->t0 = t0;
        cmd->s1 = s1;
        cmd->t1 = t1;
        R_ConvertColorToBytes(color, (unsigned int *)&cmd->color);
        cmd->rotation = AngleNormalize360(angle);
    }
}

void __cdecl R_AddCmdDrawStretchPicRotateST(
    float x,
    float y,
    float w,
    float h,
    float centerS,
    float centerT,
    float radiusST,
    float scaleFinalS,
    float scaleFinalT,
    float angle,
    const float *color,
    Material *material)
{
    Material *defaultMaterial; // [esp+4h] [ebp-8h]
    GfxCmdStretchPicRotateST *cmd; // [esp+8h] [ebp-4h]

    cmd = (GfxCmdStretchPicRotateST *)R_GetCommandBuffer(RC_STRETCH_PIC_ROTATE_ST, 52);
    if (cmd)
    {
        if (material)
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->centerS = centerS;
        cmd->centerT = centerT;
        cmd->radiusST = radiusST;
        cmd->scaleFinalS = scaleFinalS;
        cmd->scaleFinalT = scaleFinalT;
        R_ConvertColorToBytes(color, (unsigned int *)&cmd->color);
        cmd->rotation = AngleNormalize360(angle);
    }
}

void __cdecl R_AddCmdDrawTextWithCursor(
    const char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    int cursorPos,
    char cursor)
{
    AddBaseDrawTextCmd(text, maxChars, font, x, y, xScale, yScale, rotation, color, style, cursorPos, cursor);
}

GfxCmdDrawText2D *__cdecl AddBaseDrawTextCmd(
    const char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    int cursorPos,
    char cursor)
{
    unsigned int v13; // [esp+0h] [ebp-4Ch]
    GfxCmdDrawText2D *cmd; // [esp+48h] [ebp-4h]

    if (maxChars <= 0)
        MyAssertHandler(".\\r_rendercmds.cpp", 1293, 0, "%s", "maxChars > 0");
    if (!text)
        MyAssertHandler(".\\r_rendercmds.cpp", 1294, 0, "%s", "text");
    if (!*text && cursorPos < 0)
        return 0;
    v13 = strlen(text);
    cmd = (GfxCmdDrawText2D *)R_GetCommandBuffer(RC_DRAW_TEXT_2D, (v13 + 84) & 0xFFFFFFFC);
    if (!cmd)
        return 0;
    cmd->x = x;
    cmd->y = y;
    cmd->rotation = rotation;
    cmd->font = font;
    cmd->xScale = xScale;
    cmd->yScale = yScale;
    R_ConvertColorToBytes(color, (unsigned int *)&cmd->color);
    cmd->maxChars = maxChars;
    cmd->renderFlags = 0;
    switch (style)
    {
    case 3:
        cmd->renderFlags |= 4u;
        break;
    case 6:
        cmd->renderFlags |= 0xCu;
        break;
    case 128:
        cmd->renderFlags |= 1u;
        break;
    }
    if (cursorPos > -1)
    {
        cmd->renderFlags |= 2u;
        cmd->cursorPos = cursorPos;
        cmd->cursorLetter = cursor;
    }
    //Profile_Begin(166);
    memcpy((unsigned __int8 *)cmd->text, (unsigned __int8 *)text, v13);
    //Profile_EndInternal(0);
    cmd->text[v13] = 0;
    return cmd;
}

void __cdecl R_AddCmdDrawText(
    const char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, rotation, color, style, -1, 0);
}

void __cdecl R_AddCmdDrawTextSubtitle(
    const char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    const float *glowColor,
    bool cinematic)
{
    GfxCmdDrawText2D *cmd; // [esp+24h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, xScale, yScale, rotation, color, style, -1, 0);
    if (cmd)
    {
        if (cinematic)
            cmd->renderFlags |= 0x200u;
        cmd->renderFlags |= 0x100u;
        SetDrawText2DGlowParms(cmd, color, glowColor);
    }
}

char __cdecl SetDrawText2DGlowParms(GfxCmdDrawText2D *cmd, const float *color, const float *glowColor)
{
    float scaledGlowColor[4]; // [esp+0h] [ebp-10h] BYREF

    if (!glowColor)
        return 0;
    if (glowColor[3] == 0.0)
        return 0;
    cmd->renderFlags |= 0x30u;
    scaledGlowColor[0] = *glowColor * 0.1000000014901161;
    scaledGlowColor[1] = glowColor[1] * 0.1000000014901161;
    scaledGlowColor[2] = glowColor[2] * 0.1000000014901161;
    scaledGlowColor[3] = glowColor[3] * color[3];
    R_ConvertColorToBytes(scaledGlowColor, (unsigned __int8 *)&cmd->glowForceColor);
    return 1;
}

void __cdecl R_AddCmdDrawTextWithEffects(
    const char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    float rotation,
    const float *color,
    int style,
    const float *glowColor,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration)
{
    Material *v17; // [esp+24h] [ebp-30h]
    Material *defaultMaterial; // [esp+28h] [ebp-2Ch]
    const Material *actualMaterial; // [esp+4Ch] [ebp-8h]
    GfxCmdDrawText2D *cmd; // [esp+50h] [ebp-4h]

    if (fxMaterial)
        defaultMaterial = (Material *)Material_FromHandle(fxMaterial);
    else
        defaultMaterial = rgp.defaultMaterial;
    actualMaterial = defaultMaterial;
    if (Material_HasAnyFogableTechnique(defaultMaterial) && !Material_IsDefault(defaultMaterial)
        || (!fxMaterialGlow ? (v17 = rgp.defaultMaterial) : (v17 = (Material *)Material_FromHandle(fxMaterialGlow)),
            (actualMaterial = v17, Material_HasAnyFogableTechnique(v17)) && !Material_IsDefault(v17)))
    {
        R_WarnOncePerFrame(R_WARN_FOGABLE_2DTEXT, actualMaterial->info.name, text);
    }
    else
    {
        cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, xScale, yScale, rotation, color, style, -1, 0);
        if (cmd)
        {
            SetDrawText2DGlowParms(cmd, color, glowColor);
            SetDrawText2DPulseFXParms(
                cmd,
                fxMaterial,
                fxMaterialGlow,
                fxBirthTime,
                fxLetterTime,
                fxDecayStartTime,
                fxDecayDuration);
        }
    }
}

char __cdecl SetDrawText2DPulseFXParms(
    GfxCmdDrawText2D *cmd,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration)
{
    if (!fxMaterial)
        return 0;
    if (!fxMaterialGlow)
        return 0;
    if (!fxBirthTime)
        return 0;
    cmd->renderFlags |= 0xC0u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->padding = 0.0;
    return 1;
}

void __cdecl R_AddCmdDrawConsoleText(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style)
{
    AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
}

GfxCmdDrawText2D *__cdecl AddBaseDrawConsoleTextCmd(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style)
{
    GfxCmdDrawText2D *cmd; // [esp+4h] [ebp-4h]

    if (charCount < 0)
        MyAssertHandler(".\\r_rendercmds.cpp", 1501, 0, "%s", "charCount >= 0");
    if (!textPool)
        MyAssertHandler(".\\r_rendercmds.cpp", 1502, 0, "%s", "textPool");
    if (!charCount)
        return 0;
    cmd = (GfxCmdDrawText2D *)R_GetCommandBuffer(RC_DRAW_TEXT_2D, (charCount + 84) & 0xFFFFFFFC);
    if (!cmd)
        return 0;
    cmd->x = x;
    cmd->y = y;
    cmd->rotation = 0.0;
    cmd->font = font;
    cmd->xScale = xScale;
    cmd->yScale = yScale;
    R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    cmd->maxChars = 0x7FFFFFFF;
    cmd->renderFlags = 0;
    switch (style)
    {
    case 3:
        cmd->renderFlags |= 4u;
        break;
    case 6:
        cmd->renderFlags |= 0xCu;
        break;
    case 128:
        cmd->renderFlags |= 1u;
        break;
    }
    CopyPoolTextToCmd(textPool, poolSize, firstChar, charCount, cmd);
    return cmd;
}

void __cdecl CopyPoolTextToCmd(char *textPool, int poolSize, int firstChar, int charCount, GfxCmdDrawText2D *cmd)
{
    unsigned int poolRemaining; // [esp+30h] [ebp-4h]

    if (!cmd)
        MyAssertHandler(".\\r_rendercmds.cpp", 1477, 0, "%s", "cmd");
    //Profile_Begin(166);
    poolRemaining = poolSize - firstChar;
    if (charCount > poolSize - firstChar)
    {
        memcpy((unsigned __int8 *)cmd->text, (unsigned __int8 *)&textPool[firstChar], poolRemaining);
        memcpy((unsigned __int8 *)&cmd->text[poolRemaining], (unsigned __int8 *)textPool, charCount - poolRemaining);
    }
    else
    {
        memcpy((unsigned __int8 *)cmd->text, (unsigned __int8 *)&textPool[firstChar], charCount);
    }
    //Profile_EndInternal(0);
    cmd->text[charCount] = 0;
}

void __cdecl R_AddCmdDrawConsoleTextSubtitle(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    const float *glowColor)
{
    GfxCmdDrawText2D *cmd; // [esp+18h] [ebp-4h]

    cmd = AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
    if (cmd)
    {
        cmd->renderFlags |= 0x100u;
        SetDrawText2DGlowParms(cmd, color, glowColor);
    }
}

void __cdecl R_AddCmdDrawConsoleTextPulseFX(
    char *textPool,
    int poolSize,
    int firstChar,
    int charCount,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    const float *glowColor,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration,
    Material *fxMaterial,
    Material *fxMaterialGlow)
{
    Material *v18; // [esp+18h] [ebp-30h]
    Material *defaultMaterial; // [esp+1Ch] [ebp-2Ch]
    const Material *actualMaterial; // [esp+40h] [ebp-8h]
    GfxCmdDrawText2D *cmd; // [esp+44h] [ebp-4h]

    if (fxMaterial)
        defaultMaterial = (Material *)Material_FromHandle(fxMaterial);
    else
        defaultMaterial = rgp.defaultMaterial;
    actualMaterial = defaultMaterial;
    if (Material_HasAnyFogableTechnique(defaultMaterial) && !Material_IsDefault(defaultMaterial)
        || (!fxMaterialGlow ? (v18 = rgp.defaultMaterial) : (v18 = (Material *)Material_FromHandle(fxMaterialGlow)),
            (actualMaterial = v18, Material_HasAnyFogableTechnique(v18)) && !Material_IsDefault(v18)))
    {
        R_WarnOncePerFrame(R_WARN_FOGABLE_2DTEXT, actualMaterial->info.name, textPool);
    }
    else
    {
        cmd = AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
        if (cmd)
        {
            SetDrawText2DGlowParms(cmd, color, glowColor);
            SetDrawText2DPulseFXParms(
                cmd,
                fxMaterial,
                fxMaterialGlow,
                fxBirthTime,
                fxLetterTime,
                fxDecayStartTime,
                fxDecayDuration);
        }
    }
}

void __cdecl R_AddCmdDrawQuadPic(const float (*verts)[2], const float *color, Material *material)
{
    Material *defaultMaterial; // [esp+0h] [ebp-14h]
    int cornerIndex; // [esp+Ch] [ebp-8h]
    GfxCmdDrawQuadPic *cmd; // [esp+10h] [ebp-4h]

    cmd = (GfxCmdDrawQuadPic *)R_GetCommandBuffer(RC_DRAW_QUAD_PIC, 44);
    if (cmd)
    {
        if (material)
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        for (cornerIndex = 0; cornerIndex < 4; ++cornerIndex)
        {
            cmd->verts[cornerIndex][0] = (*verts)[2 * cornerIndex];
            cmd->verts[cornerIndex][1] = (*verts)[2 * cornerIndex + 1];
        }
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    }
}

void __cdecl R_BeginFrame()
{
    bool v0; // [esp+0h] [ebp-Ch]
    bool v1; // [esp+4h] [ebp-8h]

    if (rg.registered)
    {
        if (rg.inFrame)
            MyAssertHandler(".\\r_rendercmds.cpp", 1878, 0, "%s", "!rg.inFrame");
        rg.inFrame = 1;
        rg.lodParms.valid = 0;
        rg.correctedLodParms.valid = 0;
        if (Sys_IsMainThread())
            R_UpdateFrontEndDvarOptions();
        Material_OverrideTechniqueSets();
        if (rgp.world && rgp.needSortMaterials)
        {
            rgp.needSortMaterials = 0;
            if (Sys_IsMainThread())
                R_SyncRenderThread();
            Material_Sort();
            R_SortWorldSurfaces();
        }
        CL_FlushDebugClientData();
        v1 = r_skinCache->current.enabled && useFastFile->current.enabled;
        gfxBuf.skinCache = v1;
        v0 = v1 && r_fastSkin->current.enabled;
        gfxBuf.fastSkin = v0;
        if (gfxBuf.skinCache)
            R_LockSkinnedCache();
    }
}

int __cdecl R_GpuSyncModified()
{
    if (!r_gpuSync)
        MyAssertHandler(".\\r_rendercmds.cpp", 1723, 0, "%s", "r_gpuSync");
    if (!r_multiGpu)
        MyAssertHandler(".\\r_rendercmds.cpp", 1724, 0, "%s", "r_multiGpu");

    return R_CheckDvarModified(r_gpuSync) || R_CheckDvarModified(r_multiGpu);
}

const float s_debugShaderConsts[5][4] =
{
  { 0.0, 0.0, 0.0, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0, 0.0 }
}; // idb
void R_UpdateFrontEndDvarOptions()
{
    bool v0; // [esp+0h] [ebp-Ch]

    if (R_LightTweaksModified())
        R_UpdateLightsFromDvars();
    if (r_sun_from_dvars->current.enabled && rgp.world)
        R_SetSunFromDvars(&rgp.world->sun);
    if (R_GpuSyncModified())
        R_UpdateGpuSyncType();
    R_SetTestLods();
    rg.hasAnyImageOverrides = R_AreAnyImageOverridesActive();
    if (R_CheckDvarModified(r_showMissingLightGrid))
    {
        R_SyncRenderThread();
        R_ResetModelLighting();
    }
    if (r_fullbright->modified || r_debugShader->modified)
    {
        Dvar_ClearModified((dvar_s*)r_fullbright);
        Dvar_ClearModified((dvar_s*)r_debugShader);
        R_SyncRenderThread();
        R_InitDrawMethod();
    }
    if (R_CheckDvarModified(r_outdoorFeather))
        R_SetOutdoorFeatherConst();
    R_SetInputCodeConstantFromVec4(&gfxCmdBufInput, 0x27u, (float*)s_debugShaderConsts[r_debugShader->current.integer]);
    if (R_CheckDvarModified(r_envMapOverride)
        || R_CheckDvarModified(r_envMapMinIntensity)
        || R_CheckDvarModified(r_envMapMaxIntensity)
        || R_CheckDvarModified(r_envMapExponent)
        || R_CheckDvarModified(r_envMapSunIntensity))
    {
        R_EnvMapOverrideConstants();
    }
    v0 = r_distortion->current.enabled && CL_GetLocalClientActiveCount() == 1;
    if (rg.distortion != v0)
        R_SyncRenderThread();
    rg.distortion = v0;
    R_SetInputCodeImageTexture(&gfxCmdBufInput, 0xAu, v0 ? gfxRenderTargets[3].image : 0);
    rg.drawWorld = r_drawWorld->current.enabled;
    rg.drawBModels = r_drawBModels->current.enabled;
    rg.drawSModels = r_drawSModels->current.enabled;
    rg.drawXModels = r_drawXModels->current.enabled;
}

void __cdecl R_SetInputCodeConstantFromVec4(GfxCmdBufInput *input, unsigned int constant, float *value)
{
    float *v3; // [esp+0h] [ebp-4h]

    if (constant >= 0x3A)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            485,
            0,
            "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
            constant,
            58);
    if (s_codeConstUpdateFreq[constant] != 2)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            486,
            0,
            "%s\n\t(constant) = %i",
            "(s_codeConstUpdateFreq[constant] == MTL_UPDATE_RARELY)",
            constant);
    if (constant < 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            487,
            0,
            "%s\n\t(constant) = %i",
            "(!R_IsChangeablePixelShaderConst( constant ))",
            constant);
    v3 = input->consts[constant];
    *v3 = *value;
    v3[1] = value[1];
    v3[2] = value[2];
    v3[3] = value[3];
}

void __cdecl R_SetInputCodeImageTexture(GfxCmdBufInput *input, unsigned int codeTexture, const GfxImage *image)
{
    iassert(input);
    bcassert(codeTexture, TEXTURE_SRC_CODE_COUNT);
    input->codeImages[codeTexture] = image;
}

bool __cdecl R_LightTweaksModified()
{
    char v1; // bl
    char v2; // bl
    char v3; // bl
    char v4; // bl
    char v5; // bl
    char v6; // bl

    if (!rgp.world)
        return 0;
    if (!r_lightTweakAmbient)
        MyAssertHandler(".\\r_rendercmds.cpp", 1698, 0, "%s", "r_lightTweakAmbient");
    if (!r_lightTweakDiffuseFraction)
        MyAssertHandler(".\\r_rendercmds.cpp", 1699, 0, "%s", "r_lightTweakDiffuseFraction");
    if (!r_lightTweakSunLight)
        MyAssertHandler(".\\r_rendercmds.cpp", 1700, 0, "%s", "r_lightTweakSunLight");
    if (!r_lightTweakAmbientColor)
        MyAssertHandler(".\\r_rendercmds.cpp", 1701, 0, "%s", "r_lightTweakAmbientColor");
    if (!r_lightTweakSunColor)
        MyAssertHandler(".\\r_rendercmds.cpp", 1702, 0, "%s", "r_lightTweakSunColor");
    if (!r_lightTweakSunDiffuseColor)
        MyAssertHandler(".\\r_rendercmds.cpp", 1703, 0, "%s", "r_lightTweakSunDiffuseColor");
    if (!r_lightTweakSunDirection)
        MyAssertHandler(".\\r_rendercmds.cpp", 1704, 0, "%s", "r_lightTweakSunDirection");
    v1 = R_CheckDvarModified(r_lightTweakAmbient);
    v2 = R_CheckDvarModified(r_lightTweakDiffuseFraction) | v1;
    v3 = R_CheckDvarModified(r_lightTweakSunLight) | v2;
    v4 = R_CheckDvarModified(r_lightTweakAmbientColor) | v3;
    v5 = R_CheckDvarModified(r_lightTweakSunColor) | v4;
    v6 = R_CheckDvarModified(r_lightTweakSunDiffuseColor) | v5;
    return R_CheckDvarModified((const dvar_s *)r_lightTweakSunDirection) | v6;
}

void R_SetTestLods()
{
    float dist; // [esp+4h] [ebp-8h]
    signed int i; // [esp+8h] [ebp-4h]

    if (r_forceLod->current.integer == r_forceLod->reset.integer)
    {
        XModelSetTestLods(0, r_highLodDist->current.value);
        XModelSetTestLods(1u, r_mediumLodDist->current.value);
        XModelSetTestLods(2u, r_lowLodDist->current.value);
        XModelSetTestLods(3u, r_lowestLodDist->current.value);
    }
    else
    {
        for (i = 0; i < 4; ++i)
        {
            if (i == r_forceLod->current.integer)
                dist = 0.0;
            else
                dist = 0.001;
            XModelSetTestLods(i, dist);
        }
    }
}

bool __cdecl R_AreAnyImageOverridesActive()
{
    if (r_colorMap->current.integer != 1)
        return 1;
    if (r_normalMap->current.integer == 1)
        return r_specularMap->current.integer != 1;
    return 1;
}

void R_SetOutdoorFeatherConst()
{
    R_SetInputCodeConstant(
        &gfxCmdBufInput,
        0x30u,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value);
}

void __cdecl R_SetInputCodeConstant(GfxCmdBufInput *input, unsigned int constant, float x, float y, float z, float w)
{
    float *v6; // [esp+0h] [ebp-4h]

    if (constant >= 0x3A)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            475,
            0,
            "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
            constant,
            58);
    if (s_codeConstUpdateFreq[constant] != 2)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            476,
            0,
            "%s\n\t(constant) = %i",
            "(s_codeConstUpdateFreq[constant] == MTL_UPDATE_RARELY)",
            constant);
    if (constant < 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            477,
            0,
            "%s\n\t(constant) = %i",
            "(!R_IsChangeablePixelShaderConst( constant ))",
            constant);
    v6 = input->consts[constant];
    *v6 = x;
    v6[1] = y;
    v6[2] = z;
    v6[3] = w;
}

void R_EnvMapOverrideConstants()
{
    if (r_envMapMaxIntensity->current.value <= 0.0)
        MyAssertHandler(".\\r_rendercmds.cpp", 1777, 0, "%s", "r_envMapMaxIntensity->current.value > 0.0f");
    if (r_envMapOverride->current.enabled)
        R_SetInputCodeConstant(
            &gfxCmdBufInput,
            0x31u,
            r_envMapMinIntensity->current.value,
            r_envMapMaxIntensity->current.value,
            r_envMapExponent->current.value,
            r_envMapSunIntensity->current.value);
}

void __cdecl R_EndFrame()
{
    if (rg.registered)
    {
        CL_UpdateDebugClientData();
        R_TransferDebugGlobals(&frontEndDataOut->debugGlobals);
        R_AddCmdEndOfList();
        R_ClearCmdList();
        rg.viewInfoCount = 0;
        g_frameIndex = (g_frameIndex + 1) % 2;
        if (!rg.inFrame)
            MyAssertHandler(".\\r_rendercmds.cpp", 1947, 0, "%s", "rg.inFrame");
        rg.inFrame = 0;
    }
    else if (rg.inFrame)
    {
        MyAssertHandler(".\\r_rendercmds.cpp", 1923, 0, "%s", "!rg.inFrame");
    }
}

void __cdecl R_AddCmdClearScreen(int whichToClear, const float *color, float depth, unsigned __int8 stencil)
{
    GfxCmdClearScreen *cmd; // [esp+Ch] [ebp-4h]

    if (!whichToClear)
        MyAssertHandler(".\\r_rendercmds.cpp", 2043, 0, "%s", "whichToClear");
    if ((whichToClear & 0xFFFFFFF8) != 0)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2044,
            0,
            "%s\n\t(whichToClear) = %i",
            "((whichToClear & ~(0x00000001l | 0x00000002l | 0x00000004l)) == 0)",
            whichToClear);
    if (!color)
        MyAssertHandler(".\\r_rendercmds.cpp", 2045, 0, "%s", "color");
    if (depth < 0.0 || depth > 1.0)
        MyAssertHandler(".\\r_rendercmds.cpp", 2046, 0, "%s\n\t(depth) = %g", "(depth >= 0.0f && depth <= 1.0f)", depth);
    cmd = (GfxCmdClearScreen *)R_GetCommandBuffer(RC_CLEAR_SCREEN, 28);
    if (!cmd)
        MyAssertHandler(".\\r_rendercmds.cpp", 2050, 0, "%s", "cmd");
    cmd->whichToClear = whichToClear;
    if (cmd->whichToClear != whichToClear)
        MyAssertHandler(".\\r_rendercmds.cpp", 2053, 1, "%s", "cmd->whichToClear == whichToClear");
    cmd->stencil = stencil;
    cmd->depth = depth;
    cmd->color[0] = *color;
    cmd->color[1] = color[1];
    cmd->color[2] = color[2];
    cmd->color[3] = color[3];
}

void __cdecl R_AddCmdSaveScreen(unsigned int screenTimerId)
{
    GfxCmdSaveScreen *cmd; // [esp+0h] [ebp-4h]

    if (screenTimerId >= 4)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2064,
            0,
            "screenTimerId not in [0, 3]\n\t%i not in [%i, %i]",
            screenTimerId,
            0,
            3);
    cmd = (GfxCmdSaveScreen *)R_GetCommandBuffer(RC_SAVE_SCREEN, 8);
    if (!cmd)
        MyAssertHandler(".\\r_rendercmds.cpp", 2068, 0, "%s", "cmd");
    cmd->screenTimerId = screenTimerId;
}

void __cdecl R_AddCmdSaveScreenSection(
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight,
    unsigned int screenTimerId)
{
    GfxCmdSaveScreenSection *cmd; // [esp+0h] [ebp-4h]

    if (screenTimerId >= 4)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2078,
            0,
            "screenTimerId not in [0, 3]\n\t%i not in [%i, %i]",
            screenTimerId,
            0,
            3);
    cmd = (GfxCmdSaveScreenSection *)R_GetCommandBuffer(RC_SAVE_SCREEN_SECTION, 24);
    if (!cmd)
        MyAssertHandler(".\\r_rendercmds.cpp", 2082, 0, "%s", "cmd");
    cmd->s0 = viewX;
    cmd->t0 = viewY;
    cmd->ds = viewWidth;
    cmd->dt = viewHeight;
    cmd->screenTimerId = screenTimerId;
}

void __cdecl R_AddCmdBlendSavedScreenShockBlurred(
    int fadeMsec,
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight,
    unsigned int screenTimerId)
{
    GfxCmdBlendSavedScreenBlurred *cmd; // [esp+0h] [ebp-4h]

    if (screenTimerId >= 4)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2096,
            0,
            "screenTimerId not in [0, 3]\n\t%i not in [%i, %i]",
            screenTimerId,
            0,
            3);
    if (fadeMsec > 0)
    {
        cmd = (GfxCmdBlendSavedScreenBlurred *)R_GetCommandBuffer(RC_BLEND_SAVED_SCREEN_BLURRED, 28);
        if (cmd)
        {
            cmd->fadeMsec = fadeMsec;
            cmd->s0 = viewX;
            cmd->t0 = viewY;
            cmd->ds = viewWidth;
            cmd->dt = viewHeight;
            cmd->screenTimerId = screenTimerId;
        }
    }
}

void __cdecl R_AddCmdBlendSavedScreenShockFlashed(
    float intensityWhiteout,
    float intensityScreengrab,
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight)
{
    GfxCmdBlendSavedScreenFlashed *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdBlendSavedScreenFlashed *)R_GetCommandBuffer(RC_BLEND_SAVED_SCREEN_FLASHED, 28);
    if (cmd)
    {
        cmd->intensityWhiteout = intensityWhiteout;
        cmd->intensityScreengrab = intensityScreengrab;
        cmd->s0 = viewX;
        cmd->t0 = viewY;
        cmd->ds = viewWidth;
        cmd->dt = viewHeight;
    }
}

void __cdecl R_AddCmdDrawProfile()
{
    R_GetCommandBuffer(RC_DRAW_PROFILE, 4);
}

void __cdecl R_AddCmdProjectionSet2D()
{
    R_AddCmdProjectionSet(GFX_PROJECTION_2D);
}

void __cdecl R_AddCmdProjectionSet(GfxProjectionTypes projection)
{
    GfxCmdProjectionSet *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdProjectionSet *)R_GetCommandBuffer(RC_PROJECTION_SET, 8);
    if (cmd)
        cmd->projection = projection;
}

void __cdecl R_BeginRemoteScreenUpdate()
{
    if (useFastFile->current.enabled && Sys_IsMainThread())
    {
        if (r_glob.remoteScreenUpdateNesting < 0)
            MyAssertHandler(".\\r_rendercmds.cpp", 2370, 0, "%s", "r_glob.remoteScreenUpdateNesting >= 0");
        if (r_glob.startedRenderThread && !CL_IsLocalClientInGame(0))
        {
            if (r_glob.remoteScreenUpdateNesting)
            {
                ++r_glob.remoteScreenUpdateNesting;
            }
            else
            {
                if (r_glob.screenUpdateNotify)
                    MyAssertHandler(".\\r_rendercmds.cpp", 2381, 0, "%s", "!r_glob.screenUpdateNotify");
                ++r_glob.remoteScreenUpdateNesting;
                R_ReleaseThreadOwnership();
                Sys_NotifyRenderer();
            }
        }
    }
}

void __cdecl R_EndRemoteScreenUpdate()
{
    if (useFastFile->current.enabled && Sys_IsMainThread())
    {
        if (r_glob.remoteScreenUpdateNesting < 0)
            MyAssertHandler(".\\r_rendercmds.cpp", 2398, 0, "%s", "r_glob.remoteScreenUpdateNesting >= 0");
        if (r_glob.startedRenderThread && !CL_IsLocalClientInGame(0))
        {
            if (r_glob.remoteScreenUpdateNesting <= 0)
                MyAssertHandler(".\\r_rendercmds.cpp", 2406, 0, "%s", "r_glob.remoteScreenUpdateNesting > 0");
            if (r_glob.remoteScreenUpdateNesting == 1)
            {
                while (!r_glob.screenUpdateNotify)
                    NET_Sleep(1u);
                r_glob.screenUpdateNotify = 0;
                if (r_glob.remoteScreenUpdateNesting <= 0)
                    MyAssertHandler(".\\r_rendercmds.cpp", 2421, 0, "%s", "r_glob.remoteScreenUpdateNesting > 0");
                --r_glob.remoteScreenUpdateNesting;
                while (!r_glob.screenUpdateNotify)
                {
                    ++g_mainThreadBlocked;
                    NET_Sleep(1u);
                    --g_mainThreadBlocked;
                }
                r_glob.screenUpdateNotify = 0;
            }
            else
            {
                --r_glob.remoteScreenUpdateNesting;
            }
        }
        else if (r_glob.remoteScreenUpdateNesting)
        {
            MyAssertHandler(".\\r_rendercmds.cpp", 2402, 0, "%s", "r_glob.remoteScreenUpdateNesting == 0");
        }
    }
}

void __cdecl R_PushRemoteScreenUpdate(int remoteScreenUpdateNesting)
{
    if (!useFastFile->current.enabled && remoteScreenUpdateNesting)
        MyAssertHandler(".\\r_rendercmds.cpp", 2439, 0, "%s", "IsFastFileLoad() || remoteScreenUpdateNesting == 0");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 2440, 0, "%s", "Sys_IsMainThread()");
    if (remoteScreenUpdateNesting < 0)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2441,
            0,
            "%s\n\t(remoteScreenUpdateNesting) = %i",
            "(remoteScreenUpdateNesting >= 0)",
            remoteScreenUpdateNesting);
    while (remoteScreenUpdateNesting)
    {
        R_BeginRemoteScreenUpdate();
        --remoteScreenUpdateNesting;
    }
}

int __cdecl R_PopRemoteScreenUpdate()
{
    volatile int remoteScreenUpdateNesting; // [esp+4h] [ebp-4h]

    if (!useFastFile->current.enabled && r_glob.remoteScreenUpdateNesting)
        MyAssertHandler(".\\r_rendercmds.cpp", 2455, 0, "%s", "IsFastFileLoad() || r_glob.remoteScreenUpdateNesting == 0");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 2456, 0, "%s", "Sys_IsMainThread()");
    remoteScreenUpdateNesting = r_glob.remoteScreenUpdateNesting;
    while (r_glob.remoteScreenUpdateNesting)
        R_EndRemoteScreenUpdate();
    if (remoteScreenUpdateNesting < 0)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2463,
            0,
            "%s\n\t(remoteScreenUpdateNesting) = %i",
            "(remoteScreenUpdateNesting >= 0)",
            remoteScreenUpdateNesting);
    return remoteScreenUpdateNesting;
}

bool __cdecl R_IsInRemoteScreenUpdate()
{
    if (!Sys_IsRenderThread())
        MyAssertHandler(".\\r_rendercmds.cpp", 2472, 0, "%s", "Sys_IsRenderThread()");
    return r_glob.isRenderingRemoteUpdate;
}

void __cdecl R_InitTempSkinBuf()
{
    GfxBackEndData *data; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        data = &s_backEndData[i];
        if (data->tempSkinPos)
            MyAssertHandler(".\\r_rendercmds.cpp", 552, 0, "%s", "!data->tempSkinPos");
        if (data->tempSkinBuf)
            MyAssertHandler(".\\r_rendercmds.cpp", 553, 0, "%s", "!data->tempSkinBuf");
        data->tempSkinBuf = (unsigned __int8 *)Z_VirtualReserve(0x480000);
    }
}