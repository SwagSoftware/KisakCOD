#include <universal/q_shared.h>
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
#include "r_state.h"
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
#include <universal/profile.h>

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

static uint32_t s_renderCmdBufferSize;
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
    uint32_t localClientNum; // [esp+0h] [ebp-8h]
    uint32_t viewIndex; // [esp+4h] [ebp-4h]

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
    uint32_t dataIndex; // [esp+4h] [ebp-4h]

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
    uint32_t partitionIndex; // [esp+4h] [ebp-Ch]
    int dataIndex; // [esp+8h] [ebp-8h]
    uint32_t viewIndex; // [esp+Ch] [ebp-4h]
    uint32_t viewIndexa; // [esp+Ch] [ebp-4h]

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
    uint32_t dataIndex; // [esp+4h] [ebp-4h]

#ifdef KISAK_RADIANT
    s_renderCmdBufferSize = 48u * 1024u * 1024u;   // 48 MB
    s_renderCmdWarnSize   = (int)( 48u * 1024u * 1024u * 3 / 4 );
#else
    s_renderCmdBufferSize = 98304 * gfxCfg.maxClientViews;
    s_renderCmdWarnSize = (signed int)(294912 * gfxCfg.maxClientViews) / 4;
#endif
    R_InitModelLightingGlobals();
    for (dataIndex = 0; dataIndex < 2; ++dataIndex)
    {
        g_frontEndCmds[dataIndex].cmds = (uint8_t *)R_AllocGlobalVariable(s_renderCmdBufferSize, "rendercmds");
        R_InitDebugEntry(&s_backEndData[dataIndex].debugGlobals);
    }
    g_debugFrontEndCmds.cmds = (uint8_t *)R_AllocGlobalVariable(s_renderCmdBufferSize, "rendercmds (debug)");
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
    uint32_t partitionIndex; // [esp+38h] [ebp-Ch]
    uint32_t dataIndex; // [esp+3Ch] [ebp-8h]
    uint32_t viewIndex; // [esp+40h] [ebp-4h]
    uint32_t viewIndexa; // [esp+40h] [ebp-4h]

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
    uint32_t indexCount,
    uint32_t vertCount,
    uint32_t vertSize)
{
    mesh->indices = (uint16_t *)R_AllocGlobalVariable(2 * indexCount, "R_InitDynamicMesh");
    mesh->totalIndexCount = indexCount;
    mesh->indexCount = 0;
    mesh->vertSize = vertSize;
    R_InitDynamicVertexBufferState(&mesh->vb, vertSize * vertCount);
}

void __cdecl R_InitRenderThread()
{
    if (!Sys_SpawnRenderThread((void(__cdecl *)(uint32_t))RB_RenderThread))
        Com_Error(ERR_FATAL, "Failed to create render thread");
}

void __cdecl R_SyncRenderThread()
{
    if (!Sys_IsRenderThread())
    {
#ifndef KISAK_SP // called in Debug_Frame from script debugger (SP = SERVER thread)
        iassert( Sys_IsMainThread() );
#endif
        if (rg.registered)
        {
            iassert( dx.device );
            iassert( r_glob.remoteScreenUpdateNesting == 0 );
            if (r_glob.startedRenderThread && !r_glob.haveThreadOwnership)
            {
                PROF_SCOPED("FrontEndSleep");
                Sys_FrontEndSleep();
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
    iassert( Sys_IsMainThread() );
    if (r_glob.startedRenderThread)
    {
        iassert( frontEndDataOut );
        if (r_glob.haveThreadOwnership)
        {
            Sys_ReleaseThreadOwnership();
            r_glob.haveThreadOwnership = 0;
        }
    }
}

void __cdecl R_IssueRenderCommands(uint32_t type)
{
    bool v1; // [esp+1Eh] [ebp-16h]

    iassert( Sys_IsMainThread() || Sys_IsRenderThread() );
    if (R_CheckLostDevice())
    {
        PROF_SCOPED("R_IssueRenderCommands");
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
    KISAK_NULLSUB();
    Profile_ResetCounters(0);
    if (rg.stats)
        RB_CopyBackendStats();
}

bool R_UpdateSkinCacheUsage()
{
    bool result; // eax

    iassert( frontEndDataOut->skinnedCacheVb );
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
    iassert( Sys_IsMainThread() );
    R_ReleaseThreadOwnership();
    {
        PROF_SCOPED("WaitRenderer");
        //KISAK_NULLSUB();
        R_ProcessWorkerCmdsWithTimeout(Sys_IsRendererReady, 1);
    }
    if ((type & 2) != 0)
        R_PerformanceCounters();
    R_WaitFrontendWorkerCmds();
    R_UpdateSkinCacheUsage();
    R_UpdateActiveWorkerThreads();
    R_UnlockSkinnedCache();
    KISAK_NULLSUB();
    Sys_WakeRenderer((void *)frontEndDataOut);
    iassert( !r_glob.haveThreadOwnership );
    R_ToggleSmpFrame();
}

void __cdecl R_AbortRenderCommands()
{
    iassert( Sys_IsMainThread() );
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

    if ((uint32_t)renderCmd >= RC_COUNT)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            881,
            0,
            "%s\n\t(renderCmd) = %i",
            "(renderCmd >= 0 && renderCmd < RC_COUNT)",
            renderCmd);
    iassert( ((bytes & 3) == 0) );
    iassert( (bytes < s_renderCmdBufferSize) );
    if (bytes != (uint16_t)bytes)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            884,
            0,
            "%s\n\t(bytes) = %i",
            "(bytes == static_cast< unsigned short >( bytes ))",
            bytes);
    iassert( s_cmdList );
    iassert( s_cmdList->cmds );
    iassert( rg.inFrame );
    if (renderCmd < RC_FIRST_NONCRITICAL && s_cmdList->usedCritical < 7680 && bytes + s_cmdList->usedCritical >= 7680)
        Com_PrintWarning(CON_CHANNEL_GFX, "RENDERCOMMAND_CRITICAL_WARN_SIZE (%i bytes) reached\n", 7680);
    if (s_cmdList->usedTotal < s_renderCmdWarnSize && bytes + s_cmdList->usedTotal >= s_renderCmdWarnSize)
        Com_PrintWarning(CON_CHANNEL_GFX, "RENDERCOMMAND_WARN_SIZE (%.0f KB) reached\n", (double)s_renderCmdWarnSize / 1024.0);
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

uint32_t s_smpFrame;
uint32_t g_frameIndex;
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
    iassert( rg.frontEndFrameCount > 0 );
    R_FreeTempSkinBuffer();
    iassert( frontEndDataOut );
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
    KISAK_NULLSUB();
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
    iassert( frontEndDataOut );
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
                CON_CHANNEL_GFX,
                "R_AddCmdDrawStretchPic: NOT DRAWING WITH MATERIAL \"%s\", because it uses the depth buffer. Set materialType to 2d.\n",
                Name);
            actualMaterial = rgp.defaultMaterial;
        }
    }
    else
    {
        v10 = Material_GetName(material);
        Com_PrintWarning(
            CON_CHANNEL_GFX,
            "R_AddCmdDrawStretchPic: NOT DRAWING WITH MATERIAL \"%s\", because it has a fogable technique.\n",
            v10);
        actualMaterial = rgp.defaultMaterial;
    }
    iassert( !Material_UsesDepthBuffer( actualMaterial ) );
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
    iassert( techSet );
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
    iassert( material );
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
                CON_CHANNEL_GFX,
                "R_AddCmdDrawStretchPicFlipST: NOT DRAWING WITH MATERIAL \"%s\", because it uses the depth buffer. Set materialType to 2d.\n",
                Name);
            actualMaterial = rgp.defaultMaterial;
        }
    }
    else
    {
        v10 = Material_GetName(material);
        Com_PrintWarning(
            CON_CHANNEL_GFX,
            "R_AddCmdDrawStretchPicFlipST: NOT DRAWING WITH MATERIAL \"%s\", because it has a fogable technique.\n",
            v10);
        actualMaterial = rgp.defaultMaterial;
    }
    iassert( !Material_UsesDepthBuffer( actualMaterial ) );
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
        R_ConvertColorToBytes(color, (uint32_t *)&cmd->color);
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
        R_ConvertColorToBytes(color, (uint32_t *)&cmd->color);
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
        R_ConvertColorToBytes(color, (uint32_t *)&cmd->color);
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
    uint32_t v13; // [esp+0h] [ebp-4Ch]
    GfxCmdDrawText2D *cmd; // [esp+48h] [ebp-4h]

    iassert( maxChars > 0 );
    iassert( text );
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
    R_ConvertColorToBytes(color, (uint32_t *)&cmd->color);
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
    {
        PROF_SCOPED("R_memcpy");
        memcpy((uint8_t *)cmd->text, (uint8_t *)text, v13);
    }
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
    R_ConvertColorToBytes(scaledGlowColor, (uint8_t *)&cmd->glowForceColor);
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

    iassert( charCount >= 0 );
    iassert( textPool );
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
    R_ConvertColorToBytes(color, (uint8_t *)&cmd->color);
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
    uint32_t poolRemaining; // [esp+30h] [ebp-4h]

    iassert(cmd);
    
    PROF_SCOPED("R_memcpy");
    poolRemaining = poolSize - firstChar;

    if (charCount > poolSize - firstChar)
    {
        memcpy((uint8_t *)cmd->text, (uint8_t *)&textPool[firstChar], poolRemaining);
        memcpy((uint8_t *)&cmd->text[poolRemaining], (uint8_t *)textPool, charCount - poolRemaining);
    }
    else
    {
        memcpy((uint8_t *)cmd->text, (uint8_t *)&textPool[firstChar], charCount);
    }

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
        R_ConvertColorToBytes(color, (uint8_t *)&cmd->color);
    }
}

void __cdecl R_BeginFrame()
{
    bool v0; // [esp+0h] [ebp-Ch]
    bool v1; // [esp+4h] [ebp-8h]

    if (rg.registered)
    {
        iassert( !rg.inFrame );
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
        v1 = r_skinCache->current.enabled && IsFastFileLoad();
        gfxBuf.skinCache = v1;
        v0 = v1 && r_fastSkin->current.enabled;
        gfxBuf.fastSkin = v0;
        if (gfxBuf.skinCache)
            R_LockSkinnedCache();
    }
}

int __cdecl R_GpuSyncModified()
{
    iassert( r_gpuSync );
    iassert( r_multiGpu );

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
    R_SetInputCodeConstantFromVec4(&gfxCmdBufInput, CONST_SRC_CODE_DEBUG_BUMPMAP, (float*)s_debugShaderConsts[r_debugShader->current.integer]);
    if (R_CheckDvarModified(r_envMapOverride)
        || R_CheckDvarModified(r_envMapMinIntensity)
        || R_CheckDvarModified(r_envMapMaxIntensity)
        || R_CheckDvarModified(r_envMapExponent)
        || R_CheckDvarModified(r_envMapSunIntensity))
    {
        R_EnvMapOverrideConstants();
    }
    v0 = r_distortion->current.enabled && RETURN_ONE() == 1;
    if (rg.distortion != v0)
        R_SyncRenderThread();
    rg.distortion = v0;
    R_SetInputCodeImageTexture(&gfxCmdBufInput, TEXTURE_SRC_CODE_RESOLVED_POST_SUN, v0 ? gfxRenderTargets[R_RENDERTARGET_RESOLVED_POST_SUN].image : 0);
    rg.drawWorld = r_drawWorld->current.enabled;
    rg.drawBModels = r_drawBModels->current.enabled;
    rg.drawSModels = r_drawSModels->current.enabled;
    rg.drawXModels = r_drawXModels->current.enabled;
}

void __cdecl R_SetInputCodeConstantFromVec4(GfxCmdBufInput *input, CodeConstant constant, const float *value)
{
    bcassert(constant, CONST_SRC_CODE_COUNT_FLOAT4);
    iassert(s_codeConstUpdateFreq[constant] == MTL_UPDATE_RARELY);

    if (constant < 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            487,
            0,
            "%s\n\t(constant) = %i",
            "(!R_IsChangeablePixelShaderConst( constant ))",
            constant);

    input->consts[constant][0] = value[0];
    input->consts[constant][1] = value[1];
    input->consts[constant][2] = value[2];
    input->consts[constant][3] = value[3];
}

void __cdecl R_SetInputCodeImageTexture(GfxCmdBufInput *input, MaterialTextureSource codeTexture, const GfxImage *image)
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
    iassert( r_lightTweakAmbient );
    iassert( r_lightTweakDiffuseFraction );
    iassert( r_lightTweakSunLight );
    iassert( r_lightTweakAmbientColor );
    iassert( r_lightTweakSunColor );
    iassert( r_lightTweakSunDiffuseColor );
    iassert( r_lightTweakSunDirection );
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
                dist = 0.0f;
            else
                dist = 0.001f;
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
        CONST_SRC_CODE_OUTDOOR_FEATHER_PARMS,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value);
}

void __cdecl R_SetInputCodeConstant(GfxCmdBufInput *input, CodeConstant constant, float x, float y, float z, float w)
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
    iassert( r_envMapMaxIntensity->current.value > 0.0f );
    if (r_envMapOverride->current.enabled)
        R_SetInputCodeConstant(
            &gfxCmdBufInput,
            CONST_SRC_CODE_ENVMAP_PARMS,
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
        iassert( rg.inFrame );
        rg.inFrame = 0;
    }
    else if (rg.inFrame)
    {
        MyAssertHandler(".\\r_rendercmds.cpp", 1923, 0, "%s", "!rg.inFrame");
    }
}

void __cdecl R_AddCmdClearScreen(int whichToClear, const float *color, float depth, uint8_t stencil)
{
    GfxCmdClearScreen *cmd; // [esp+Ch] [ebp-4h]

    iassert( whichToClear );
    if ((whichToClear & 0xFFFFFFF8) != 0)
        MyAssertHandler(
            ".\\r_rendercmds.cpp",
            2044,
            0,
            "%s\n\t(whichToClear) = %i",
            "((whichToClear & ~(0x00000001l | 0x00000002l | 0x00000004l)) == 0)",
            whichToClear);
    iassert( color );
    iassert( (depth >= 0.0f && depth <= 1.0f) );
    cmd = (GfxCmdClearScreen *)R_GetCommandBuffer(RC_CLEAR_SCREEN, 28);
    iassert( cmd );
    cmd->whichToClear = whichToClear;
    iassert( cmd->whichToClear == whichToClear );
    cmd->stencil = stencil;
    cmd->depth = depth;
    cmd->color[0] = *color;
    cmd->color[1] = color[1];
    cmd->color[2] = color[2];
    cmd->color[3] = color[3];
}

void __cdecl R_AddCmdSaveScreen(uint32_t screenTimerId)
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
    iassert( cmd );
    cmd->screenTimerId = screenTimerId;
}

void __cdecl R_AddCmdSaveScreenSection(
    float viewX,
    float viewY,
    float viewWidth,
    float viewHeight,
    uint32_t screenTimerId)
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
    iassert( cmd );
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
    uint32_t screenTimerId)
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

void __cdecl R_AddCmdProjectionSet3D()
{
    R_AddCmdProjectionSet(GFX_PROJECTION_3D);
}

void __cdecl R_AddCmdProjectionSet(GfxProjectionTypes projection)
{
    GfxCmdProjectionSet *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdProjectionSet *)R_GetCommandBuffer(RC_PROJECTION_SET, 8);
    if (cmd)
        cmd->projection = projection;
}

#ifdef KISAK_RADIANT
bool __cdecl CL_IsLocalClientInGame(int32_t localClientNum)
{
    return true;
}
#endif

void __cdecl R_BeginRemoteScreenUpdate()
{
    if (IsFastFileLoad() && Sys_IsMainThread())
    {
        iassert( r_glob.remoteScreenUpdateNesting >= 0 );
        if (r_glob.startedRenderThread && !CL_IsLocalClientInGame(0))
        {
            if (r_glob.remoteScreenUpdateNesting)
            {
                ++r_glob.remoteScreenUpdateNesting;
            }
            else
            {
                iassert( !r_glob.screenUpdateNotify );
                ++r_glob.remoteScreenUpdateNesting;
                R_ReleaseThreadOwnership();
                Sys_NotifyRenderer();
            }
        }
    }
}

void __cdecl R_EndRemoteScreenUpdate()
{
    if (IsFastFileLoad() && Sys_IsMainThread())
    {
        iassert( r_glob.remoteScreenUpdateNesting >= 0 );
        if (r_glob.startedRenderThread && !CL_IsLocalClientInGame(0))
        {
            iassert( r_glob.remoteScreenUpdateNesting > 0 );
            if (r_glob.remoteScreenUpdateNesting == 1)
            {
                while (!r_glob.screenUpdateNotify)
                    NET_Sleep(1);
                r_glob.screenUpdateNotify = 0;
                iassert( r_glob.remoteScreenUpdateNesting > 0 );
                --r_glob.remoteScreenUpdateNesting;
                while (!r_glob.screenUpdateNotify)
                {
                    ++g_mainThreadBlocked;
                    NET_Sleep(1);
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
    iassert( IsFastFileLoad() || remoteScreenUpdateNesting == 0 );
    iassert( Sys_IsMainThread() );
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

    iassert( IsFastFileLoad() || r_glob.remoteScreenUpdateNesting == 0 );
#ifndef KISAK_SP
    iassert( Sys_IsMainThread() );
#endif
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
    iassert( Sys_IsRenderThread() );
    return r_glob.isRenderingRemoteUpdate;
}

void __cdecl R_InitTempSkinBuf()
{
    GfxBackEndData *data; // [esp+0h] [ebp-8h]
    uint32_t i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        data = &s_backEndData[i];
        iassert( !data->tempSkinPos );
        iassert( !data->tempSkinBuf );
#ifdef KISAK_RADIANT
        data->tempSkinBuf = (uint8_t *)Z_VirtualReserve(0x5000000);
#else
		data->tempSkinBuf = (uint8_t *)Z_VirtualReserve(0x480000);
#endif
    }
}

void R_AddCmdSetViewportValues(int x, int y, int width, int height)
{
    GfxCmdHeader *cmd; // r30

    iassert(width > 0);
    iassert(height > 0);
    
    cmd = R_GetCommandBuffer(RC_SET_VIEWPORT, 20);

    iassert(cmd);

    _DWORD *writer = (_DWORD *)cmd; // hack

    writer[1] = x;
    writer[2] = y;
    writer[3] = width;
    writer[4] = height;
}

void __cdecl R_BeginDebugFrame()
{
    iassert(s_debugFrameGlob.restoreCmdList == NULL);
    iassert(s_debugFrameGlob.restoreFrontEndDataOut == NULL);

    if (rg.registered)
    {
        Com_SyncThreads();
        s_debugFrameGlob.inFrame = rg.inFrame;
        rg.inFrame = 1;
        s_debugFrameGlob.restoreCmdList = s_cmdList;
        s_debugFrameGlob.restoreFrontEndDataOut = frontEndDataOut;
        if (gfxBuf.skinnedCacheLockAddr)
        {
            iassert(!s_debugFrameGlob.restoreSkinnedCache);
            s_debugFrameGlob.restoreSkinnedCache = 1;
            R_UnlockSkinnedCache();
        }
        s_cmdList = s_debugFrameGlob.frontEndDataOut.commands;
        frontEndDataOut = &s_debugFrameGlob.frontEndDataOut;
        R_BeginSharedCmdList();
    }
}

void __cdecl R_EndDebugFrame()
{
    bool v0; // [esp+1h] [ebp-1h]

    if (rg.registered)
    {
        R_AddCmdEndOfList();
        R_ClearCmdList();
        R_SyncRenderThread();
        frontEndDataOut->drawType = -1;
#ifndef KISAK_SP
        iassert(Sys_IsMainThread());
#endif
        if (R_CheckLostDevice())
            v0 = g_disableRendering == 0;
        else
            v0 = 0;
        if (v0)
        {
            RB_BeginFrame(frontEndDataOut);
            RB_Draw3D();
            RB_CallExecuteRenderCommands();
            RB_EndFrame(-1);
        }
        s_cmdList = s_debugFrameGlob.restoreCmdList;
        frontEndDataOut = s_debugFrameGlob.restoreFrontEndDataOut;
        if (s_debugFrameGlob.restoreSkinnedCache)
        {
            s_debugFrameGlob.restoreSkinnedCache = 0;
            R_LockSkinnedCache();
        }
        s_debugFrameGlob.restoreCmdList = 0;
        s_debugFrameGlob.restoreFrontEndDataOut = 0;
        iassert(rg.inFrame);
        rg.inFrame = s_debugFrameGlob.inFrame;
    }
}

#ifdef KISAK_RADIANT
// ─────────────────────────────────────────────────────────────────────────────
// Editor render-command additions — cod3src\src\gfx_d3d\r_rendercmds.cpp in the
// CoD4Radiant binary (IDB port 13343). The editor's line bridge (draw.cpp
// R_Add3DLine/Draw_02 and the XY/Z/Cam view draws) batches line geometry into
// GfxCmdDrawLines render commands through these. KisakCOD's CoD3-era
// r_rendercmds.cpp lacks them; ported verbatim from the IDB.
//
// SIGNATURE / §11 NOTES (validated against the kisak callsites above):
//  • kisak R_GetCommandBuffer(renderCmd, bytes). The IDB pseudocode lists the
//    args swapped — R_GetCommandBuffer(bytes, RC_DRAW_LINES) — a __usercall
//    normalization artifact. kisak's own callers use (renderCmd, bytes); we follow
//    the kisak prototype, NOT the IDB literal order.
//  • The IDB compares lastCmd->header.id == 20 (the CoD4 RC_DRAW_LINES enum value).
//    kisak's RC_DRAW_LINES == 0x12. We use the SYMBOL, never the literal.
//  • GfxCmdDrawLines layout (rb_backend.h): header(4) + lineCount(2)+width(1)+
//    dimensions(1) = 8-byte prefix, then verts. A line command of N line-segments
//    is 8 + 0x20*N bytes (0x20 = two GfxPointVertex per segment).
// ─────────────────────────────────────────────────────────────────────────────
#include "rb_backend.h"   // GfxCmdDrawLines
#include <string.h>       // memcpy

// IDB R_AddMultipleRendercommands @ 0x4fb0d0 — extend the last command in place:
// rewind usedTotal to the start of lastCmd, re-acquire the buffer at the bigger
// byte count (same id), and hand back the old end-of-buffer where the new data
// gets appended.
void *__cdecl R_AddMultipleRendercommands(int bytes)
{
    GfxCmdHeader *lastCmd = s_cmdList->lastCmd;
    uint8_t *cmds = s_cmdList->cmds;
    int usedTotal = s_cmdList->usedTotal;
    s_cmdList->usedTotal = (int)((uint8_t *)lastCmd - cmds);
    uint8_t *appendPos = &cmds[usedTotal];
    GfxCmdHeader *bufferStart =
        R_GetCommandBuffer((GfxRenderCommand)lastCmd->id, bytes + lastCmd->byteCount);
    iassert( s_cmdList->lastCmd == bufferStart );
    return bufferStart ? appendPos : nullptr;
}

// IDB R_AddLineCmd @ 0x4fd0a0 — append `count` line-segments of `dimension`-D
// geometry, merging into the previous RC_DRAW_LINES command when the widths and
// dimensions match and the byte/line caps still fit.
void __cdecl R_AddLineCmd(short count, char width, char dimension, GfxPointVertex *verts)
{
    iassert( (count > 0) );

    GfxCmdDrawLines *lastCmd = (GfxCmdDrawLines *)s_cmdList->lastCmd;
    if ( lastCmd
        && lastCmd->header.id == RC_DRAW_LINES
        && 0x20u * (unsigned)count + (unsigned)lastCmd->header.byteCount <= 0xFFFF
        && lastCmd->width == (uint8_t)width
        && lastCmd->dimensions == (uint8_t)dimension
        && count + lastCmd->lineCount <= 0x7FFF )
    {
        void *dst = R_AddMultipleRendercommands(0x20 * count);
        if ( dst )
        {
            memcpy(dst, verts, 0x20 * count);
            lastCmd->lineCount += count;
        }
    }
    else
    {
        int byteCount = 0x20 * count;
        GfxCmdDrawLines *cmd =
            (GfxCmdDrawLines *)R_GetCommandBuffer(RC_DRAW_LINES, byteCount + 8);
        if ( cmd )
        {
            cmd->lineCount = count;
            cmd->width = width;
            cmd->dimensions = dimension;
            memcpy(cmd->verts, verts, byteCount);
        }
    }
}

#ifdef KISAK_RADIANT
// Editor-only: the last MATERIAL_COLOR pushed via RC_SET_MATERIAL_COLOR. R_AddCmd_Line3D sets the
// $line colour per batch; without deduping, DrawGeo's one-Line3D-per-brush emits a redundant
// SetMaterialColor EVERY brush AND breaks R_AddLineCmd's RC_DRAW_LINES merge (lastCmd becomes the
// SetMaterialColor, so each brush starts a fresh DrawLines). On a large map (blackout) the per-brush
// command pairs overflow s_renderCmdBufferSize -> R_GetCommandBuffer line-904 assert. Tracking the
// last colour (updated by EVERY R_AddCmdSetMaterialColor caller, incl. the per-paint s_edWhite seed)
// lets us skip the emit when unchanged, so same-colour line batches merge into one DrawLines.
// Behaviourally identical: MATERIAL_COLOR persists in backend state until the next set.
static float s_edLastMatColor[4] = { -2.0f, -2.0f, -2.0f, -2.0f };   // sentinel: never matches a real colour

// TASK-1 EXPERIMENT (temporary, env-gated): RADIANT_LINEVCOL=1 pushes the BINARY's neutral
// MATERIAL_COLOR {0,0,0,0} for every editor line batch instead of the port's first-vertex
// override.  The binary never sets MATERIAL_COLOR per line batch — it leaves the value that
// R_SetMaterialColor(NULL) parked (Cam_Draw 0x4080f7/0x408115, XY_Draw 0x46d8fa) — so if the
// $line 'vertcol_shaded_tools' pixel shader really is
//     rgb = lerp( sample(colorMap=$white) * vertexColour, matColor.rgb, matColor.w )
// then w==0 reproduces the binary exactly and the PER-VERTEX colour drives the line.  If
// instead the lines go black/white, the shader ignores vertex colour and the port's override
// is a required adaptation.  One run decides it.
static bool Ed_LineVertexColorMode()
{
    static const bool s_on = []{ const char *e = getenv( "RADIANT_LINEVCOL" );
                                 return e && *e && *e != '0'; }();
    return s_on;
}
static const float s_edNeutralMatColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

// ─────────────────────────────────────────────────────────────────────────────
// Ed_EmitLineBatch — the port's MATERIAL_COLOR line adaptation, split PER COLOUR RUN.
//
// WHY THE ADAPTATION EXISTS AT ALL:  $line resolves (LINEPROBE, mp_backlot) to
//   mtl='$line' techSet='tools' tech='vertcol_shaded_tools' entry=0
//   loadBits=18128812/0000000d  DEV[blendEn=0 src=2 dst=1 zwrite=1 ztest=1 zfunc=4 cwe=15]
// i.e. the SHARED vertcol_shaded pixel shader, whose editor form is
//   rgb = lerp( sample(colorMap) * vertexColour, materialColor.rgb, materialColor.w ).
// The binary never sets MATERIAL_COLOR per line batch: it parks the neutral {0,0,0,0}
// (R_SetMaterialColor(NULL) — Cam_Draw 0x4080f7 / 0x408115, XY_Draw 0x46d8fa) so w==0 and
// the PER-VERTEX colour drives every line.  kisak's editor bring-up could not rely on that
// (the grid draws outside a full scene render, so CONST_SRC_CODE_MATERIAL_COLOR has no
// version and r_shade.cpp asserts), hence the per-batch push with w==1 = flat override.
//
// THE BUG THIS FIXES (TASK 1, 2026-07-26):  the push used verts[0]'s colour for the WHOLE
// batch, so any batch carrying more than one colour was FLATTENED to its first colour.
// DrawAngles (0x479d50) builds exactly such a batch: with drawType == -1 (the camera pass —
// DrawGeneralWorld_ 0x407b9d and DrawBrush_SunPreview 0x406999 both pass 0xFFFFFFFF) it
// emits base->tip and tip->wing1 in flt_6DE220 GREY (0.5,0.5,0.5) and tip->wing2 in
// colorWhite, then ONE R_AddCmd_Line3D(3, 2, verts).  The port painted all three segments
// grey; the binary paints two grey and one WHITE.  Measured on mp_backlot's camera pane:
// 13695 px of flat 128,128,128 before, 10967 grey + 2728 pure white after — and the 2728
// are exactly the pixels that flipped, i.e. the arrowhead barb the binary draws white.
//
// Splitting the emit at every colour change reproduces the binary's per-vertex result
// through the mechanism kisak already needs, WITHOUT depending on the $white colorMap
// sample being 1.0 (which the RADIANT_LINEVCOL experiment showed cannot be assumed: with
// w==0 the XY brush wireframes came back at ~0.32x, so the neutral-matColor route needs the
// colorMap binding verified first — see PROGRESS.md).  Single-colour batches (the common
// case: the grid, one brush's wireframe) still emit exactly one SetMaterialColor+DrawLines
// pair, so the command-buffer pressure the dedup was added for is unchanged.
//
// LIMITATION (documented, not a divergence in practice): a single line whose two endpoints
// carry DIFFERENT colours would interpolate in the binary; here it takes its first vertex's
// colour.  No editor line builder emits such a gradient (R_Add3DLine stamps one colour into
// both endpoints; Draw_02 / the wireframe builders do the same).
static void Ed_EmitLineBatch(short count, char width, char dimension, GfxPointVertex *verts)
{
    if ( count <= 0 || !verts )
        { R_AddLineCmd(count, width, dimension, verts); return; }

    const bool neutral = Ed_LineVertexColorMode();
    int runStart = 0;
    for ( int i = 0; i <= (int)count; ++i )
    {
        // GfxPointVertex.color is packed BGRA (Byte4PackVertexColor / Byte4PackPixelColor:
        // array[2]=R, [1]=G, [0]=B, [3]=A) — unpack in that order or red<->blue swaps.
        const bool last = ( i == (int)count );
        const bool changed = last
            || ( *(const unsigned int *)verts[2 * i].color
              != *(const unsigned int *)verts[2 * runStart].color );
        if ( !changed )
            continue;

        const unsigned char *c = (const unsigned char *)verts[2 * runStart].color;
        float rgba[4] = { c[2] * (1.0f / 255.0f), c[1] * (1.0f / 255.0f),
                          c[0] * (1.0f / 255.0f), c[3] * (1.0f / 255.0f) };
        const float *push = neutral ? s_edNeutralMatColor : rgba;
        // Only push when the colour CHANGES vs the last emitted one — keeps same-colour
        // batches (hundreds of unselected brushes) merging into one RC_DRAW_LINES instead of
        // a SetMaterialColor+DrawLines pair per brush (which overflowed s_renderCmdBufferSize
        // on blackout).
        if ( push[0] != s_edLastMatColor[0] || push[1] != s_edLastMatColor[1]
          || push[2] != s_edLastMatColor[2] || push[3] != s_edLastMatColor[3] )
            R_AddCmdSetMaterialColor( push );
        R_AddLineCmd((short)(i - runStart), width, dimension, &verts[2 * runStart]);
        runStart = i;
    }
}
#endif

// IDB R_AddCmd_Line3D @ 0x4fd1a0 — thin 3D wrapper over R_AddLineCmd.
void __cdecl R_AddCmd_Line3D(short count, char width, GfxPointVertex *verts)
{
#ifdef KISAK_RADIANT
    Ed_EmitLineBatch(count, width, 3, verts);
#else
    R_AddLineCmd(count, width, 3, verts);
#endif
}

#ifdef KISAK_RADIANT
void __cdecl R_AddCmd_Line3DNoDepth(short count, char width, GfxPointVertex *verts)
{
    Ed_EmitLineBatch(count, width, 4, verts);
}
#endif

// IDB R_AddCmd_Line2D @ 0x4fd180 — thin 2D wrapper over R_AddLineCmd (the exact 2D
// sibling of R_AddCmd_Line3D: dimension=2 vs 3).  Editor-only (the 2D pane line
// renderer used by the texture window's layered-material outline rects).  The binary
// is a pure forwarder; like Line3D we additionally push the first vertex's colour as
// the MATERIAL colour under KISAK_RADIANT, because kisak's editor $line technique
// colours from CODE_MATERIAL_COLOR (not the per-vertex GfxPointVertex.color the binary
// relies on) — without it a 2D line batch would render whatever material colour was
// last set instead of its own packed colour.
void __cdecl R_AddCmd_Line2D(short count, char width, GfxPointVertex *verts)
{
#ifdef KISAK_RADIANT
    Ed_EmitLineBatch(count, width, 2, verts);
#else
    R_AddLineCmd(count, width, 2, verts);
#endif
}

// IDB R_AddPointCmd @ 0x4fcf60 — append `pointCount` points (each a 16-byte
// GfxPointVertex) of `dimension`-D into the last RC_DRAW_POINTS command, or start a
// fresh one. Mirrors R_AddLineCmd exactly (the merge path uses
// R_AddMultipleRendercommands; the fresh path R_GetCommandBuffer). kisak's
// GfxCmdDrawPoints names the per-point field `size` (the IDB calls it `width`) and the
// flexible array `verts`. The clipper's DrawClipper draws its placed clip points here.
static GfxCmdDrawPoints *__cdecl R_AddPointCmdRaw(short pointCount, char size, char dimension, const GfxPointVertex *verts)
{
    iassert( (pointCount > 0) );
    iassert( (size > 0) );

    GfxCmdDrawPoints *lastCmd = (GfxCmdDrawPoints *)s_cmdList->lastCmd;
    if ( lastCmd
        && lastCmd->header.id == RC_DRAW_POINTS
        && 16u * (unsigned)pointCount + (unsigned)lastCmd->header.byteCount <= 0xFFFF
        && lastCmd->size == (uint8_t)size
        && lastCmd->dimensions == (uint8_t)dimension
        && pointCount + lastCmd->pointCount <= 0x7FFF )
    {
        GfxCmdDrawPoints *dst = (GfxCmdDrawPoints *)R_AddMultipleRendercommands(16 * pointCount);
        if ( dst )
        {
            memcpy(dst, verts, 16 * pointCount);
            lastCmd->pointCount += pointCount;
        }
        return dst;
    }

    int byteCount = 16 * pointCount;
    GfxCmdDrawPoints *cmd =
        (GfxCmdDrawPoints *)R_GetCommandBuffer(RC_DRAW_POINTS, byteCount + 8);
    if ( cmd )
    {
        cmd->pointCount = pointCount;
        cmd->size = size;
        cmd->dimensions = dimension;
        memcpy(cmd->verts, verts, byteCount);
    }
    return cmd;
}

// Keep the IDA point-command batching in R_AddPointCmdRaw; Radiant wraps it below
// to map per-vertex point colours onto kisak's material-colour point technique.
GfxCmdDrawPoints *__cdecl R_AddPointCmd(short pointCount, char size, char dimension, const GfxPointVertex *verts)
{
#ifdef KISAK_RADIANT
    iassert( (pointCount > 0) );
    iassert( (size > 0) );

    GfxCmdDrawPoints *lastCmd = nullptr;
    int first = 0;

    while ( first < pointCount )
    {
        const unsigned int packed = *(const unsigned int *)verts[first].color;
        int runCount = 1;

        while ( first + runCount < pointCount
             && *(const unsigned int *)verts[first + runCount].color == packed )
        {
            ++runCount;
        }

        const unsigned char *c = (const unsigned char *)&packed;
        float rgba[4] =
        {
            c[2] * ( 1.0f / 255.0f ),
            c[1] * ( 1.0f / 255.0f ),
            c[0] * ( 1.0f / 255.0f ),
            c[3] * ( 1.0f / 255.0f )
        };

        if ( rgba[0] != s_edLastMatColor[0] || rgba[1] != s_edLastMatColor[1]
          || rgba[2] != s_edLastMatColor[2] || rgba[3] != s_edLastMatColor[3] )
            R_AddCmdSetMaterialColor( rgba );

        lastCmd = R_AddPointCmdRaw( (short)runCount, size, dimension, &verts[first] );
        first += runCount;
    }

    return lastCmd;
#else
    return R_AddPointCmdRaw( pointCount, size, dimension, verts );
#endif
}

// IDB R_AddPointCmd_W @ 0x4fd080 - thin 3D wrapper over R_AddPointCmd.
GfxCmdDrawPoints *__cdecl R_AddPointCmd_W(short pointCount, char size, const GfxPointVertex *verts)
{
    return R_AddPointCmd(pointCount, size, 3, verts);
}

// IDB R_AddRenderCmdDrawTris @ 0x4fd1c0 — immediate-mode textured/lit triangle
// soup with a material + technique. The editor camera view draws each face's
// triangle fan through this. kisak ALREADY has the consumer (RB_DrawTrianglesCmd,
// rb_backend.cpp) and the GfxCmdDrawTriangles layout (xyzw@16, normal, color, st,
// indices) — verified byte-identical to the IDB. §11: kisak
// R_GetCommandBuffer(renderCmd, bytes) (IDB lists them swapped); RC_DRAW_TRIANGLES
// symbol (kisak 0x13), never a literal.
void __cdecl R_AddRenderCmdDrawTris(
    Material *material, MaterialTechniqueType techType, short indexCount,
    const uint16_t *indices, short vertexCount,
    const float (*xyzw)[4], const float (*normal)[3], float *color,
    const float (*st)[2])
{
    const Material *handle = material ? Material_FromHandle(material) : rgp.defaultMaterial;
    iassert(handle);
    if (!Material_GetTechnique((Material *)handle, techType))
        return;                                  // material lacks this technique → skip

    int vc           = vertexCount;
    int xyzwOffset   = 16;                        // after the 16-byte GfxCmdDrawTriangles
    int normalOffset = xyzwOffset   + 16 * vc;
    int colorOffset  = normalOffset + 12 * vc;
    int stOffset     = colorOffset  +  4 * vc;
    int indexOffset  = stOffset     +  8 * vc;
    int indexBytes   = 2 * ((indexCount + 1) & ~1);
    int total        = indexOffset  + indexBytes;

    GfxCmdDrawTriangles *cmd =
        (GfxCmdDrawTriangles *)R_GetCommandBuffer(RC_DRAW_TRIANGLES, total);
    if (!cmd)
        return;
    cmd->material    = handle;
    cmd->techType    = techType;
    cmd->indexCount  = indexCount;
    cmd->vertexCount = vertexCount;
    memcpy((char *)cmd + xyzwOffset,   xyzw,    16 * vc);
    memcpy((char *)cmd + normalOffset, normal,  12 * vc);
    memcpy((char *)cmd + colorOffset,  color,    4 * vc);
    memcpy((char *)cmd + stOffset,     st,       8 * vc);
    memcpy((char *)cmd + indexOffset,  indices, indexBytes);
}

// IDB R_AddBeginViewCmd @ 0x4fc3a0 — emit RC_BEGIN_VIEW carrying the scene def
// and the GfxViewParms* that RB_BeginViewCmd will install as the active view.
// The IDB's frontEndDataOut->viewParms[] bounds asserts are dropped: the editor
// scene setup (R_Ed_SetSceneParms) uses a single static GfxViewParms (kisak's
// GfxBackEndData has only one viewParms, and editor rendering is synchronous —
// no SMP workers — so the pointer is valid front-end → back-end).
// §11: kisak R_GetCommandBuffer(renderCmd, bytes); IDB lists them swapped.
void __cdecl R_AddBeginViewCmd(const GfxSceneDef *sceneDef, const GfxViewParms *viewParms)
{
    GfxCmdBeginView *cmd = (GfxCmdBeginView *)R_GetCommandBuffer(RC_BEGIN_VIEW, sizeof(GfxCmdBeginView));
    iassert( cmd );
    cmd->sceneDef = *sceneDef;
    cmd->viewParms = viewParms;
}

// Emit RC_SET_MATERIAL_COLOR → RB_SetMaterialColorCmd sets CONST_SRC_CODE_MATERIAL_COLOR.
// The editor draws the grid via a bare RC_DRAW_LINES outside a full scene render, so the
// per-material code-constant setup a normal scene does (which feeds MATERIAL_COLOR to the
// $line UNLIT pixel shader) never runs → constVersions[MATERIAL_COLOR]==0 →
// r_shade.cpp:282 (iassert newState.fields.version) + garbage (white) line output. Setting
// it white before the lines (matching rb_sky.cpp's unlit-draw pattern) bumps the version
// and gives the shader its colour. kisak ships only the RB handler, no emitter — add one.
void __cdecl R_AddCmdSetMaterialColor(const float *color)
{
    GfxCmdSetMaterialColor *cmd = (GfxCmdSetMaterialColor *)R_GetCommandBuffer(RC_SET_MATERIAL_COLOR, sizeof(GfxCmdSetMaterialColor));
#ifdef KISAK_RADIANT
    // OVERFLOW GUARD (blackout crash): the editor's immediate-mode world draw + per-entity
    // DrawAngles emit an RC_SET_MATERIAL_COLOR per brush/entity, so on a huge map the render
    // command buffer fills and R_GetCommandBuffer returns NULL (its 904 overflow assert
    // logs+continues in the non-fatal editor build).  R_AddLineCmd already null-guards its
    // writes; this emitter did not, so line 2087 dereferenced NULL -> access violation
    // (Cam_Draw -> DrawBrush -> DrawAngles -> R_AddCmd_Line3D -> here).  Drop the colour set on
    // overflow (the batch draws with the persisted colour — cosmetic) and DON'T advance
    // s_edLastMatColor, so a later successful emit re-pushes it.  The buffer is also grown for
    // the editor in R_InitRenderCommands so this rarely fires.
    if ( !cmd )
        return;
#else
    iassert( cmd );
#endif
    cmd->color[0] = color[0];
    cmd->color[1] = color[1];
    cmd->color[2] = color[2];
    cmd->color[3] = color[3];
#ifdef KISAK_RADIANT
    // Keep R_AddCmd_Line3D's per-batch colour dedup in sync (see s_edLastMatColor): every caller
    // updates it, so the line path can skip redundant colour sets and let DrawLines batches merge.
    s_edLastMatColor[0] = color[0]; s_edLastMatColor[1] = color[1];
    s_edLastMatColor[2] = color[2]; s_edLastMatColor[3] = color[3];
#endif
}

#ifdef KISAK_RADIANT
// IDB R_AddCmdSetCustomShaderConstant @ 0x4fd330 — emit RC_SET_CUSTOM_CONSTANT carrying a
// (constantType, vec4) pair. The sun-light preview (#26 layer C2, R_SunPrev_SetSunConstants)
// uses it to inject the parsed worldspawn sun direction/colour into the active source-state
// (the editor has no scene sun / rgp.world->sunLight, so the in-scene R_SetSunConstants never
// runs). RB_SetCustomConstantCmd (rb_backend.cpp) consumes it. §11: kisak R_GetCommandBuffer
// (renderCmd, bytes) — the IDB lists the args swapped (R_GetCommandBuffer(24, cmd)).
void __cdecl R_AddCmdSetCustomShaderConstant(unsigned int constant, float x, float y, float z, float w)
{
    GfxCmdSetCustomConstant *cmd =
        (GfxCmdSetCustomConstant *)R_GetCommandBuffer(RC_SET_CUSTOM_CONSTANT, sizeof(GfxCmdSetCustomConstant));
    if ( !cmd )
        return;
    cmd->type   = constant;
    cmd->vec[0] = x;
    cmd->vec[1] = y;
    cmd->vec[2] = z;
    cmd->vec[3] = w;
}

// IDB R_AddCmdDrawFullScreenColoredQuad @ 0x4fc260 — emit RC_DRAW_FULL_SCREEN_COLORED_QUAD.
// The backend handler (RB_DrawFullScreenColoredQuadCmd -> RB_DrawFullScreenColoredQuad),
// the GfxCmdDrawFullScreenColoredQuad struct, and the dispatch-table entry ALL already
// exist in kisak's CoD3 base (rb_backend.cpp) — only the EDITOR's front-end emitter was
// missing (the game draws its full-screen colored quads via RB_FullScreenColoredFilter /
// rb_sky.cpp's glare-blind, never through this command). The #26 sun-light preview
// (R_SunPrev_Main 0x4069c0) uses it for the two full-screen passes that frame the lit
// draw: the BLACK-WORLD multiply quad (mat_white_multiply, colour = worldspawn ambient)
// that resets the already-textured framebuffer to its unlit/ambient state so the
// SUNLIGHT_PREVIEW pass ADDS sun light rather than double-brightening, and the
// CLEAR-STENCIL quad (rgp.clearAlphaStencilMaterial) that zeroes the stencil before the
// shadow volumes build. The IDB takes a Material HANDLE (Material_FromHandle); kisak's
// editor passes a resolved Material* directly (matching every other kisak render-command
// material field), so no handle indirection is needed. §11: R_GetCommandBuffer is
// (renderCmd, bytes) in kisak — the IDB lists them swapped.
void __cdecl R_AddCmdDrawFullScreenColoredQuad(
    float s0, float t0, float s1, float t1, const float *color, const Material *material)
{
    GfxCmdDrawFullScreenColoredQuad *cmd = (GfxCmdDrawFullScreenColoredQuad *)
        R_GetCommandBuffer(RC_DRAW_FULL_SCREEN_COLORED_QUAD, sizeof(GfxCmdDrawFullScreenColoredQuad));
    if ( !cmd )
        return;
    cmd->material = material ? material : rgp.defaultMaterial;   // IDB: NULL -> rgp.defaultMaterial
    cmd->s0 = s0;
    cmd->t0 = t0;
    cmd->s1 = s1;
    cmd->t1 = t1;
    R_ConvertColorToBytes( color, &cmd->color );
}
#endif

// IDB R_AddCmdDrawTextAtPosition @ 0x4fbe20 — emit an RC_DRAW_TEXT_3D command for a
// string positioned in world space. `origin` is the world-space anchor; `xPixelStep`
// and `yPixelStep` are the per-text-pixel world-space basis vectors (so the glyphs are
// screen-aligned at the view's scale). The XY view uses this for its coordinate labels,
// view-name hint, and entity-name labels (P5.5 text path). kisak ALREADY has the
// consumer (RB_DrawText3DCmd, rb_backend.cpp, dispatch index RC_DRAW_TEXT_3D=0xE) and
// the GfxCmdDrawText3D layout — only the editor's emitter was missing.
// §11: kisak R_GetCommandBuffer(renderCmd, bytes) (IDB lists them swapped); the byte
// size is (strlen(text)+sizeof(GfxCmdDrawText3D)) rounded DOWN to a multiple of 4,
// matching the IDB's `(t_size + 0x34) & ~3` (0x34 == sizeof(GfxCmdDrawText3D)).
void __cdecl R_AddCmdDrawTextAtPosition(
    const char *text, Font_s *font, const float *origin,
    const float *xPixelStep, const float *yPixelStep, const float *color)
{
    iassert( text );
    if ( !*text )
        return;
    size_t tlen = strlen(text);
    GfxCmdDrawText3D *cmd =
        (GfxCmdDrawText3D *)R_GetCommandBuffer(RC_DRAW_TEXT_3D, (tlen + sizeof(GfxCmdDrawText3D)) & ~3u);
    if ( !cmd )
        return;
    cmd->org[0] = origin[0];
    cmd->org[1] = origin[1];
    cmd->org[2] = origin[2];
    cmd->font = font;
    cmd->xPixelStep[0] = xPixelStep[0];
    cmd->xPixelStep[1] = xPixelStep[1];
    cmd->xPixelStep[2] = xPixelStep[2];
    cmd->yPixelStep[0] = yPixelStep[0];
    cmd->yPixelStep[1] = yPixelStep[1];
    cmd->yPixelStep[2] = yPixelStep[2];
    R_ConvertColorToBytes(color, (uint32_t *)&cmd->color);
    memcpy(cmd->text, text, tlen);
    cmd->text[tlen] = 0;
}

// IDB R_AddCmdDraw2DImage @ 0x4fb5e0 — the editor's OWN 2D image emitter, used by
// the texture-window thumbnail grid (TexWnd_DrawMaterials @ 0x45cc40). It is a
// STRIPPED R_AddCmdDrawStretchPic (above): it emits the SAME RC_STRETCH_PIC command
// (44 bytes, consumed by the existing RB_DrawStretchPic which forces TECHNIQUE_UNLIT)
// but WITHOUT the game version's depth-buffer / fogable-technique rejection. The game
// R_AddCmdDrawStretchPic refuses any material whose stateFlags&0x10 ("uses the depth
// buffer") and substitutes rgp.defaultMaterial — which is exactly why P5.7's StretchPic
// path drew the default checkerboard for every wc/ world material instead of the real
// texture (hence the name-list fallback). The editor draws world ("wc/<name>") materials
// straight into the 2D pass: RB_DrawStretchPic selects each material's UNLIT technique,
// which samples the colormap, so the thumbnail shows the actual texture image.
// §11: kisak R_GetCommandBuffer(renderCmd, bytes) (IDB lists them swapped); RC_STRETCH_PIC
// symbol (== RC_FIRST_NONCRITICAL), never a literal.
void __cdecl R_AddCmdDraw2DImage(
    float x, float y, float w, float h,
    float s0, float t0, float s1, float t1,
    const float *color, Material *material)
{
    Material *actualMaterial = material ? (Material *)Material_FromHandle(material) : rgp.defaultMaterial;
    GfxCmdStretchPic *cmd = (GfxCmdStretchPic *)R_GetCommandBuffer(RC_STRETCH_PIC, 44);
    if ( cmd )
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
#endif // KISAK_RADIANT
