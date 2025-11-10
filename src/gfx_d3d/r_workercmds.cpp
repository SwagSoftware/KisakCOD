#include "r_workercmds.h"
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>
#include "r_scene.h"
#include "r_staticmodelcache.h"
#include "r_dobj_skin.h"
#include <universal/profile.h>
#include "r_workercmds_common.h"
#include "r_shadowcookie.h"
#include "r_spotshadow.h"
#include "r_dpvs.h"
#include <EffectsCore/fx_system.h>
#include "r_model_skin.h"
#include "r_dvars.h"
#include <win32/win_net.h>
#include <win32/win_local.h>
#include "rb_logfile.h"
#include "r_setstate_d3d.h"
#include "r_model_pose.h"

#include <setjmp.h>

void(__cdecl *g_cmdExecFailed[17])();
volatile WorkerCmdType g_waitTypeMainThread;

//long volatile g_workerCmdWaitCount 85b4fc54     gfx_d3d : r_workercmds.obj

GfxEntity *g_GfxEntityBoundsBuf[256];
GfxEntity *g_SkinGfxEntityBuf[1024];
FxCmd g_UpdateFxNonDependentBuf[1];
FxCmd g_UpdateFxRemainingBuf[1];
SkinCachedStaticModelCmd g_skinCachedStaticModelBuf[512];
SkinXModelCmd g_SkinXModelBuf[1024];
DpvsStaticCellCmd g_dpvsCellStaticBuf[256];
DpvsDynamicCellCmd g_dpvsCellSceneEntBuf[512];
DpvsDynamicCellCmd g_dpvsCellDynModelBuf[512];
DpvsDynamicCellCmd g_dpvsCellDynBrushBuf[512];
DpvsEntityCmd g_dpvsEntityBuf[2048];
FxCmd g_UpdateFxSpotLightBuf[1];
FxGenerateVertsCmd g_GenerateFxVertsBuf[2];
FxCmd g_GenerateMarkVertsBuf[1];
SceneEntCmd g_addSceneEntBuf[1];
GfxSpotShadowEntCmd g_spotShadowEntBuf[256];
ShadowCookieCmd g_shadowCookieBuf[1];

WorkerCmds g_workerCmds[WRKCMD_COUNT];


int __cdecl R_FXNonDependentOrSpotLightPending(void* args)
{
    return R_FXSpotLightPending() || R_FXNonDependentPending();
}

bool __cdecl R_FXSpotLightPending()
{
    return g_workerCmds[0].inSize > 0;
}

bool __cdecl R_FXNonDependentPending()
{
    return g_workerCmds[1].inSize > 0;
}

int __cdecl R_EndFenceBusy(void *args)
{
    return R_EndFencePending();
}

void __cdecl TRACK_r_workercmds()
{
    track_static_alloc_internal(g_GfxEntityBoundsBuf, 1024, "g_GfxEntityBoundsBuf", 18);
    track_static_alloc_internal(g_SkinGfxEntityBuf, 4096, "g_SkinGfxEntityBuf", 18);
    track_static_alloc_internal(g_UpdateFxNonDependentBuf, 12, "g_UpdateFxNonDependentBuf", 18);
    track_static_alloc_internal(g_UpdateFxRemainingBuf, 12, "g_UpdateFxRemainingBuf", 18);
    track_static_alloc_internal(g_skinCachedStaticModelBuf, 2048, "g_skinCachedStaticModelBuf", 18);
    track_static_alloc_internal(g_SkinXModelBuf, 28672, "g_SkinXModelBuf", 18);
    track_static_alloc_internal(g_workerCmds, 2176, "g_workerCmds", 18);
}

WorkerCmdType g_workerCmdMinType;
int __cdecl R_ProcessWorkerCmdsWithTimeoutInternal(int(__cdecl *timeout)())
{
    int processed; // [esp+0h] [ebp-Ch]
    WorkerCmdType minType; // [esp+4h] [ebp-8h]
    WorkerCmdType type; // [esp+8h] [ebp-4h]

    if (timeout())
        return 1;
    Sys_ResetWorkerCmdEvent();
    do
    {
    restart_2:
        processed = 0;
        type = g_waitTypeMainThread;
        if (type >= 0 && g_workerCmds[type].outSize > 0)
        {
            while (R_ProcessWorkerCmd(type))
            {
                if (timeout())
                    return 1;
                processed = 1;
            }
        }
        minType = g_workerCmdMinType;
        if (g_workerCmdMinType == 0x7FFFFFFF)
            minType = WRKCMD_FIRST_FRONTEND;
        for (type = minType; type < WRKCMD_COUNT; ++type)
        {
            if (g_workerCmds[type].outSize > 0)
            {
                while (R_ProcessWorkerCmd(type))
                {
                    if (timeout())
                        return 1;
                    if (g_workerCmdMinType < type)
                        goto restart_2;
                    processed = 1;
                }
            }
            InterlockedCompareExchange((volatile unsigned int*)&g_workerCmdMinType, 0x7FFFFFFF, type);
        }
        if (timeout())
            return 1;
    } while (processed || minType);
    return 0;
}

LONG g_workerCmdWaitCount;
void __cdecl R_ProcessWorkerCmdsWithTimeout(int(__cdecl *timeout)(), int forever)
{
    while (!timeout() && !R_ProcessWorkerCmdsWithTimeoutInternal(timeout) && forever)
    {
        PROF_SCOPED("WaitForWorkerCmd");
        InterlockedIncrement(&g_workerCmdWaitCount);
        Sys_WaitForWorkerCmd();
        if (timeout())
        {
            InterlockedDecrement(&g_workerCmdWaitCount);
            return;
        }
        InterlockedDecrement(&g_workerCmdWaitCount);
    }
}

void __cdecl R_WaitWorkerCmdsOfType(WorkerCmdType type)
{
    iassert(Sys_IsMainThread());
    g_waitTypeMainThread = type;
    if (!R_WorkerCmdsFinished())
    {
        R_NotifyWorkerCmdType(type);
        KISAK_NULLSUB();
        R_ProcessWorkerCmdsWithTimeout(R_WorkerCmdsFinished, 1);
    }
    g_waitTypeMainThread = (WorkerCmdType)-1;
}

void __cdecl R_NotifyWorkerCmdType(WorkerCmdType type)
{
    if (g_workerCmdMinType > type)
        InterlockedCompareExchange((volatile unsigned int *)&g_workerCmdMinType, type, g_workerCmdMinType);
    if (g_workerCmdWaitCount)
        Sys_SetWorkerCmdEvent();
}

int __cdecl R_WorkerCmdsFinished()
{
    return g_workerCmds[g_waitTypeMainThread].inSize == 0;
}

void __cdecl R_ProcessWorkerCmds()
{
    int processed; // [esp+0h] [ebp-Ch]
    WorkerCmdType minType; // [esp+4h] [ebp-8h]
    WorkerCmdType type; // [esp+8h] [ebp-4h]

    Sys_ResetWorkerCmdEvent();
    do
    {
    restart_1:
        processed = 0;
        type = g_waitTypeMainThread;
        if (type >= 0 && g_workerCmds[type].outSize > 0)
        {
            while (R_ProcessWorkerCmd(type))
                processed = 1;
        }
        minType = g_workerCmdMinType;
        if (g_workerCmdMinType == 0x7FFFFFFF)
            minType = WRKCMD_FIRST_FRONTEND;
        for (type = minType; type < WRKCMD_COUNT; ++type)
        {
            if (g_workerCmds[type].outSize > 0)
            {
                while (R_ProcessWorkerCmd(type))
                {
                    if (g_workerCmdMinType < type)
                        goto restart_1;
                    processed = 1;
                }
            }
            InterlockedCompareExchange((volatile unsigned int*)&g_workerCmdMinType, 0x7FFFFFFF, type);
        }
    } while (processed || minType);
}

int __cdecl R_ProcessWorkerCmd(WorkerCmdType type)
{
    int v2; // eax
    int v3; // eax
    unsigned int bufCount; // [esp+0h] [ebp-7A4h]
    unsigned __int8 data[1920]; // [esp+4h] [ebp-7A0h] BYREF
    int dataSize; // [esp+788h] [ebp-1Ch]
    WorkerCmds *workerCmds; // [esp+78Ch] [ebp-18h]
    unsigned int currentCount; // [esp+790h] [ebp-14h]
    unsigned int startPos; // [esp+794h] [ebp-10h]
    unsigned int newStartPos; // [esp+798h] [ebp-Ch]
    unsigned int i; // [esp+79Ch] [ebp-8h]
    unsigned int count; // [esp+7A0h] [ebp-4h]

    workerCmds = &g_workerCmds[type];
    dataSize = workerCmds->dataSize;
    bufCount = workerCmds->bufCount;
    iassert( !(workerCmds->bufSize % dataSize) );
    while (InterlockedExchangeAdd((LONG*)&workerCmds->outSize, -1) <= 0)
    {
        if (InterlockedExchangeAdd((LONG*)&workerCmds->outSize, 1) < 0)
            return 0;
    }
    if (g_cmdOutputBusy[type])
    {
        while (1)
        {
            startPos = workerCmds->startPos;
            memcpy(data, &workerCmds->buf[dataSize * startPos], dataSize);
            if (g_cmdOutputBusy[type](data))
            {
                InterlockedExchangeAdd((LONG*)&workerCmds->outSize, 1);
                return 0;
            }
            newStartPos = startPos + 1;
            if (startPos + 1 == bufCount)
                newStartPos = 0;
            v2 = InterlockedCompareExchange((LONG*)&workerCmds->startPos, newStartPos, startPos);
            if (v2 == startPos)
                break;
            if (g_cmdExecFailed[type])
                g_cmdExecFailed[type]();
        }
        KISAK_NULLSUB();
        R_ProcessWorkerCmdInternal(type, data);
        InterlockedExchangeAdd((LONG*)&workerCmds->inSize, -1);
        if (g_workerCmdWaitCount)
            Sys_SetWorkerCmdEvent();
    }
    else
    {
        iassert( !g_cmdExecFailed[type] );
        if (InterlockedExchangeAdd((LONG*)&workerCmds->outSize, -9) < 9)
        {
            InterlockedExchangeAdd((LONG*)&workerCmds->outSize, 9);
            count = 1;
        }
        else
        {
            count = 10;
        }
        do
        {
            startPos = workerCmds->startPos;
            currentCount = bufCount - startPos;
            if (count < bufCount - startPos)
            {
                currentCount = count;
                newStartPos = count + startPos;
            }
            else
            {
                memcpy(&data[dataSize * currentCount], workerCmds->buf, dataSize * (count - currentCount));
                newStartPos = count - currentCount;
            }
            memcpy(data, &workerCmds->buf[dataSize * startPos], dataSize * currentCount);
            v3 = InterlockedCompareExchange((LONG*)&workerCmds->startPos, newStartPos, startPos);
        } while (v3 != startPos);
        KISAK_NULLSUB();
        for (i = 0; i < count; ++i)
            R_ProcessWorkerCmdInternal(type, &data[dataSize * i]);
        //InterlockedExchangeAdd(&workerCmds->inSize, -count);
        InterlockedExchangeAdd((LONG*)&workerCmds->inSize, -(int)count);
        if (g_workerCmdWaitCount)
            Sys_SetWorkerCmdEvent();
    }
    return 1;
}

void __cdecl R_ProcessWorkerCmdInternal(WorkerCmdType type, void *data)
{
    R_NotifyWorkerCmdType(type);
    switch (type)
    {
    case WRKCMD_UPDATE_FX_SPOT_LIGHT:
        R_ProcessCmd_UpdateFxSpotLight((FxCmd *)data);
        break;
    case WRKCMD_UPDATE_FX_NON_DEPENDENT:
        R_ProcessCmd_UpdateFxNonDependent((FxCmd *)data);
        break;
    case WRKCMD_UPDATE_FX_REMAINING:
        R_ProcessCmd_UpdateFxRemaining((FxCmd *)data);
        break;
    case WRKCMD_DPVS_CELL_STATIC:
        R_AddCellStaticSurfacesInFrustumCmd((DpvsStaticCellCmd *)data);
        break;
    case WRKCMD_DPVS_CELL_SCENE_ENT:
        R_AddCellSceneEntSurfacesInFrustumCmd((GfxWorldDpvsPlanes *)data);
        break;
    case WRKCMD_DPVS_CELL_DYN_MODEL:
        R_AddCellDynModelSurfacesInFrustumCmd((const DpvsDynamicCellCmd *)data);
        break;
    case WRKCMD_DPVS_CELL_DYN_BRUSH:
        R_AddCellDynBrushSurfacesInFrustumCmd((const DpvsDynamicCellCmd *)data);
        break;
    case WRKCMD_DPVS_ENTITY:
        R_AddEntitySurfacesInFrustumCmd((unsigned __int16 *)data);
        break;
    case WRKCMD_ADD_SCENE_ENT:
        R_AddAllSceneEntSurfacesCamera(*(const GfxViewInfo **)data);
        break;
    case WRKCMD_SPOT_SHADOW_ENT:
        R_AddSpotShadowEntCmd((const GfxSpotShadowEntCmd *)data);
        break;
    case WRKCMD_SHADOW_COOKIE:
        R_GenerateShadowCookiesCmd((ShadowCookieCmd *)data);
        break;
    case WRKCMD_BOUNDS_ENT_DELAYED:
        R_UpdateGfxEntityBoundsCmd((GfxSceneEntity **)data);
        break;
    case WRKCMD_SKIN_ENT_DELAYED:
        R_SkinGfxEntityCmd((GfxSceneEntity **)data);
        break;
    case WRKCMD_GENERATE_FX_VERTS:
        if (!dx.deviceLost)
            FX_GenerateVerts((FxGenerateVertsCmd *)data);
        break;
    case WRKCMD_GENERATE_MARK_VERTS:
        if (!dx.deviceLost)
            FX_GenerateMarkVertsForWorld(((FxCmd *)data)->localClientNum);
        break;
    case WRKCMD_SKIN_CACHED_STATICMODEL:
        R_SkinCachedStaticModelCmd((SkinCachedStaticModelCmd *)data);
        break;
    case WRKCMD_SKIN_XMODEL:
        R_SkinXModelCmd((WORD*)data);
        break;
    default:
        if (!alwaysfails)
        {
            iassert(0);
            //MyAssertHandler(".\\r_workercmds.cpp", 635, 0, "unhandled case");
        }
        break;
    }
}

void R_InitWorkerThreads()
{
    unsigned int workerThreadIndexa; // [esp+0h] [ebp-4h]

    iassert( Sys_IsMainThread() );
    if (sys_smp_allowed->current.enabled)
    {
        R_InitWorkerCmds();
        for (workerThreadIndexa = 0; workerThreadIndexa < 2; ++workerThreadIndexa)
        {
            if (!Sys_SpawnWorkerThread((void(__cdecl *)(unsigned int))R_WorkerThread, workerThreadIndexa))
                Com_Error(ERR_FATAL, "Failed to create thread");
        }
    }
}

void R_InitWorkerCmdsPos()
{
    WorkerCmds *workerCmds; // [esp+4h] [ebp-8h]
    int type; // [esp+8h] [ebp-4h]

    for (type = 0; type < WRKCMD_COUNT; ++type)
    {
        workerCmds = &g_workerCmds[type];
        workerCmds->startPos = 0;
        workerCmds->endPos = workerCmds->bufSize;
        workerCmds->syncedEndPos = 0;
        workerCmds->inSize = 0;
        workerCmds->outSize = 0;
        iassert(workerCmds->dataSize);
        workerCmds->bufCount = workerCmds->bufSize / workerCmds->dataSize;
        iassert(workerCmds->dataSize <= 192);
    }
}

void R_InitWorkerCmds()
{
    g_workerCmds[WRKCMD_UPDATE_FX_SPOT_LIGHT].buf = (unsigned __int8 *)g_UpdateFxSpotLightBuf;
    g_workerCmds[WRKCMD_UPDATE_FX_SPOT_LIGHT].bufSize = 12;
    g_workerCmds[WRKCMD_UPDATE_FX_SPOT_LIGHT].dataSize = 12;

    g_workerCmds[WRKCMD_UPDATE_FX_NON_DEPENDENT].buf = (unsigned __int8 *)g_UpdateFxNonDependentBuf;
    g_workerCmds[WRKCMD_UPDATE_FX_NON_DEPENDENT].bufSize = 12;
    g_workerCmds[WRKCMD_UPDATE_FX_NON_DEPENDENT].dataSize = 12;

    g_workerCmds[WRKCMD_UPDATE_FX_REMAINING].buf = (unsigned __int8 *)g_UpdateFxRemainingBuf;
    g_workerCmds[WRKCMD_UPDATE_FX_REMAINING].bufSize = 12;
    g_workerCmds[WRKCMD_UPDATE_FX_REMAINING].dataSize = 12;

    g_workerCmds[WRKCMD_DPVS_CELL_STATIC].buf = (unsigned __int8 *)g_dpvsCellStaticBuf;
    g_workerCmds[WRKCMD_DPVS_CELL_STATIC].bufSize = 3072;
    g_workerCmds[WRKCMD_DPVS_CELL_STATIC].dataSize = 12;

    g_workerCmds[WRKCMD_DPVS_CELL_SCENE_ENT].buf = (unsigned __int8 *)g_dpvsCellSceneEntBuf;
    g_workerCmds[WRKCMD_DPVS_CELL_SCENE_ENT].bufSize = 6144;
    g_workerCmds[WRKCMD_DPVS_CELL_SCENE_ENT].dataSize = 12;

    g_workerCmds[WRKCMD_DPVS_CELL_DYN_MODEL].buf = (unsigned __int8 *)g_dpvsCellDynModelBuf;
    g_workerCmds[WRKCMD_DPVS_CELL_DYN_MODEL].bufSize = 6144;
    g_workerCmds[WRKCMD_DPVS_CELL_DYN_MODEL].dataSize = 12;

    g_workerCmds[WRKCMD_DPVS_CELL_DYN_BRUSH].buf = (unsigned __int8 *)g_dpvsCellDynBrushBuf;
    g_workerCmds[WRKCMD_DPVS_CELL_DYN_BRUSH].bufSize = 6144;
    g_workerCmds[WRKCMD_DPVS_CELL_DYN_BRUSH].dataSize = 12;

    g_workerCmds[WRKCMD_DPVS_ENTITY].buf = (unsigned __int8 *)g_dpvsEntityBuf;
    g_workerCmds[WRKCMD_DPVS_ENTITY].bufSize = 0x8000;
    g_workerCmds[WRKCMD_DPVS_ENTITY].dataSize = 16;

    g_workerCmds[WRKCMD_ADD_SCENE_ENT].buf = (unsigned __int8 *)g_addSceneEntBuf;
    g_workerCmds[WRKCMD_ADD_SCENE_ENT].bufSize = 4;
    g_workerCmds[WRKCMD_ADD_SCENE_ENT].dataSize = 4;

    g_workerCmds[WRKCMD_SPOT_SHADOW_ENT].buf = (unsigned __int8 *)g_spotShadowEntBuf;
    g_workerCmds[WRKCMD_SPOT_SHADOW_ENT].bufSize = 2048;
    g_workerCmds[WRKCMD_SPOT_SHADOW_ENT].dataSize = 8;

    g_workerCmds[WRKCMD_SHADOW_COOKIE].buf = (unsigned __int8 *)g_shadowCookieBuf;
    g_workerCmds[WRKCMD_SHADOW_COOKIE].bufSize = 16;
    g_workerCmds[WRKCMD_SHADOW_COOKIE].dataSize = 16;

    g_workerCmds[WRKCMD_BOUNDS_ENT_DELAYED].buf = (unsigned __int8 *)g_GfxEntityBoundsBuf;
    g_workerCmds[WRKCMD_BOUNDS_ENT_DELAYED].bufSize = 1024;
    g_workerCmds[WRKCMD_BOUNDS_ENT_DELAYED].dataSize = 4;

    g_workerCmds[WRKCMD_SKIN_ENT_DELAYED].buf = (unsigned __int8 *)g_SkinGfxEntityBuf;
    g_workerCmds[WRKCMD_SKIN_ENT_DELAYED].bufSize = 4096;
    g_workerCmds[WRKCMD_SKIN_ENT_DELAYED].dataSize = 4;

    g_workerCmds[WRKCMD_GENERATE_FX_VERTS].buf = (unsigned __int8 *)g_GenerateFxVertsBuf;
    g_workerCmds[WRKCMD_GENERATE_FX_VERTS].bufSize = 136;
    g_workerCmds[WRKCMD_GENERATE_FX_VERTS].dataSize = 68;

    g_workerCmds[WRKCMD_GENERATE_MARK_VERTS].buf = (unsigned __int8 *)g_GenerateMarkVertsBuf;
    g_workerCmds[WRKCMD_GENERATE_MARK_VERTS].bufSize = 12;
    g_workerCmds[WRKCMD_GENERATE_MARK_VERTS].dataSize = 12;

    g_workerCmds[WRKCMD_SKIN_CACHED_STATICMODEL].buf = (unsigned __int8 *)g_skinCachedStaticModelBuf;
    g_workerCmds[WRKCMD_SKIN_CACHED_STATICMODEL].bufSize = 2048;
    g_workerCmds[WRKCMD_SKIN_CACHED_STATICMODEL].dataSize = 4;

    g_workerCmds[WRKCMD_SKIN_XMODEL].buf = (unsigned __int8 *)g_SkinXModelBuf;
    g_workerCmds[WRKCMD_SKIN_XMODEL].bufSize = 28672;
    g_workerCmds[WRKCMD_SKIN_XMODEL].dataSize = 28;

    R_InitWorkerCmdsPos();
}

void __cdecl  R_WorkerThread()
{
    void *Value; // eax

    Value = Sys_GetValue(2);
    //if (_setjmp3(Value, 0))
    if (_setjmp(*(jmp_buf *)Value))
        Com_ErrorAbort();
    Profile_Guard(1);

    while (1)
    {
        {
            PROF_SCOPED("WaitForWorkerCmd");
            InterlockedIncrement(&g_workerCmdWaitCount);
            Sys_WaitForWorkerCmd();
            InterlockedDecrement(&g_workerCmdWaitCount);
        }
        {
            PROF_SCOPED("WorkerThread");
            R_ProcessWorkerCmds();
        }
    }
}

void __cdecl R_AddWorkerCmd(WorkerCmdType type, unsigned __int8 *data)
{
    LONG* Destination; // [esp+30h] [ebp-20h]
    int bufCount; // [esp+34h] [ebp-1Ch]
    int endPos; // [esp+38h] [ebp-18h]
    int bufSize; // [esp+40h] [ebp-10h]
    int dataSize; // [esp+44h] [ebp-Ch]
    WorkerCmds *workerCmds; // [esp+48h] [ebp-8h]

    if (r_smp_worker->current.enabled && sys_smp_allowed->current.enabled)
    {
        workerCmds = &g_workerCmds[type];
        dataSize = workerCmds->dataSize;
        bufSize = workerCmds->bufSize;
        bufCount = workerCmds->bufCount;
        iassert( !(bufSize % dataSize ) );
        if (InterlockedExchangeAdd((LONG*)&workerCmds->inSize, 1) < bufCount)
        {
            endPos = InterlockedExchangeAdd((LONG*)&workerCmds->endPos, dataSize) % bufSize;
            iassert( (endPos >= 0) );
            if (!endPos)
                InterlockedExchangeAdd((LONG*)&workerCmds->endPos, -bufSize);
            memcpy(&workerCmds->buf[endPos], data, dataSize);
            Destination = (LONG*)&workerCmds->syncedEndPos;
            do
            {
                while (*Destination != endPos)
                    ;
            } while (InterlockedCompareExchange(Destination, (dataSize + endPos) % bufSize, endPos) != endPos);
            InterlockedExchangeAdd((LONG*)&workerCmds->outSize, 1);
            R_NotifyWorkerCmdType(type);
            return;
        }
        if (type != 15)
            R_WarnOncePerFrame(R_WARN_WORKER_CMD_SIZE, type);
        InterlockedExchangeAdd((LONG*)&workerCmds->inSize, -1);
    }

    {
        PROF_SCOPED("WaitWorkerCmds");
        if (g_cmdOutputBusy[type])
        {
            while (g_cmdOutputBusy[type](data))
                NET_Sleep(1);
        }
    }

    R_ProcessWorkerCmdInternal(type, data);
}

void __cdecl R_UpdateActiveWorkerThreads()
{
    char v0; // [esp+3h] [ebp-9h]
    unsigned int i; // [esp+4h] [ebp-8h]
    unsigned int workerIter; // [esp+8h] [ebp-4h]

    iassert( Sys_IsMainThread() );
    for (i = 0; i < 2; ++i)
    {
        if (r_smp_worker_thread[i]->modified)
        {
            v0 = 1;
            goto LABEL_9;
        }
    }
    v0 = 0;
LABEL_9:
    if (v0)
    {
        Com_SyncThreads();
        for (workerIter = 0; workerIter < 2; ++workerIter)
        {
            if (r_smp_worker_thread[workerIter]->modified)
            {
                Dvar_ClearModified((dvar_s*)r_smp_worker_thread[workerIter]);
                if (r_smp_worker_thread[workerIter]->current.enabled)
                    Sys_ResumeThread((ThreadContext_t)(workerIter + 2));
                else
                    Sys_SuspendThread((ThreadContext_t)(workerIter + 2));
            }
        }
        R_ReleaseThreadOwnership();
    }
}

void __cdecl R_WaitFrontendWorkerCmds()
{
    iassert(Sys_IsMainThread());

    PROF_SCOPED("R_WaitFrontendWorkerCmds");
    //KISAK_NULLSUB();

    R_ProcessWorkerCmdsWithTimeout(R_FinishedWorkerCmds, 1);
}

int __cdecl R_FinishedWorkerCmds()
{
    int type; // [esp+4h] [ebp-4h]

    for (type = 0; type < WRKCMD_COUNT; ++type)
    {
        if (g_workerCmds[type].inSize > 0)
            return 0;
    }
    return 1;
}

void __cdecl R_WaitWorkerCmds()
{
#ifndef KISAK_SP
    iassert(Sys_IsMainThread());
#endif

    PROF_SCOPED("R_WaitWorkerCmds");
    //KISAK_NULLSUB();

    R_ProcessWorkerCmdsWithTimeout(R_FinishedWorkerCmds, 1);
}

