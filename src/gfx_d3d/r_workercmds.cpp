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
volatile int g_waitTypeMainThread;

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

WorkerCmds g_workerCmds[17];


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

LONG g_workerCmdMinType;
int __cdecl R_ProcessWorkerCmdsWithTimeoutInternal(int(__cdecl *timeout)())
{
    int processed; // [esp+0h] [ebp-Ch]
    int minType; // [esp+4h] [ebp-8h]
    LONG type; // [esp+8h] [ebp-4h]
    LONG typea; // [esp+8h] [ebp-4h]

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
            minType = 0;
        for (typea = minType; typea < 17; ++typea)
        {
            if (g_workerCmds[typea].outSize > 0)
            {
                while (R_ProcessWorkerCmd(typea))
                {
                    if (timeout())
                        return 1;
                    if (g_workerCmdMinType < typea)
                        goto restart_2;
                    processed = 1;
                }
            }
            InterlockedCompareExchange(&g_workerCmdMinType, 0x7FFFFFFF, typea);
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
        Profile_Begin(175);
        InterlockedIncrement(&g_workerCmdWaitCount);
        Sys_WaitForWorkerCmd();
        if (timeout())
        {
            InterlockedDecrement(&g_workerCmdWaitCount);
            Profile_EndInternal(0);
            return;
        }
        InterlockedDecrement(&g_workerCmdWaitCount);
        Profile_EndInternal(0);
    }
}

void __cdecl R_WaitWorkerCmdsOfType(int type)
{
    iassert(Sys_IsMainThread());
    g_waitTypeMainThread = type;
    if (!R_WorkerCmdsFinished())
    {
        R_NotifyWorkerCmdType(type);
        KISAK_NULLSUB();
        R_ProcessWorkerCmdsWithTimeout(R_WorkerCmdsFinished, 1);
    }
    g_waitTypeMainThread = -1;
}

void __cdecl R_NotifyWorkerCmdType(int type)
{
    if (g_workerCmdMinType > type)
        InterlockedCompareExchange(&g_workerCmdMinType, type, g_workerCmdMinType);
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
    int minType; // [esp+4h] [ebp-8h]
    int type; // [esp+8h] [ebp-4h]
    int typea; // [esp+8h] [ebp-4h]

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
            minType = 0;
        for (typea = minType; typea < 17; ++typea)
        {
            if (g_workerCmds[typea].outSize > 0)
            {
                while (R_ProcessWorkerCmd(typea))
                {
                    if (g_workerCmdMinType < typea)
                        goto restart_1;
                    processed = 1;
                }
            }
            InterlockedCompareExchange(&g_workerCmdMinType, 0x7FFFFFFF, typea);
        }
    } while (processed || minType);
}

int __cdecl R_ProcessWorkerCmd(LONG type)
{
    LONG v2; // eax
    LONG v3; // eax
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
    if (workerCmds->bufSize % dataSize)
        MyAssertHandler(".\\r_workercmds.cpp", 661, 0, "%s", "!(workerCmds->bufSize % dataSize)");
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
        if (g_cmdExecFailed[type])
            MyAssertHandler(".\\r_workercmds.cpp", 711, 0, "%s", "!g_cmdExecFailed[type]");
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

int __cdecl R_ProcessWorkerCmd(int type)
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
    if (workerCmds->bufSize % dataSize)
        MyAssertHandler(".\\r_workercmds.cpp", 661, 0, "%s", "!(workerCmds->bufSize % dataSize)");
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
        if (g_cmdExecFailed[type])
            MyAssertHandler(".\\r_workercmds.cpp", 711, 0, "%s", "!g_cmdExecFailed[type]");
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

void __cdecl R_ProcessWorkerCmdInternal(int type, void *data)
{
    R_NotifyWorkerCmdType(type);
    switch (type)
    {
    case 0:
        R_ProcessCmd_UpdateFxSpotLight((FxCmd *)data);
        break;
    case 1:
        R_ProcessCmd_UpdateFxNonDependent((FxCmd *)data);
        break;
    case 2:
        R_ProcessCmd_UpdateFxRemaining((FxCmd *)data);
        break;
    case 3:
        R_AddCellStaticSurfacesInFrustumCmd((DpvsStaticCellCmd *)data);
        break;
    case 4:
        R_AddCellSceneEntSurfacesInFrustumCmd((GfxWorldDpvsPlanes *)data);
        break;
    case 5:
        R_AddCellDynModelSurfacesInFrustumCmd((const DpvsDynamicCellCmd *)data);
        break;
    case 6:
        R_AddCellDynBrushSurfacesInFrustumCmd((const DpvsDynamicCellCmd *)data);
        break;
    case 7:
        R_AddEntitySurfacesInFrustumCmd((unsigned __int16 *)data);
        break;
    case 8:
        R_AddAllSceneEntSurfacesCamera(*(const GfxViewInfo **)data);
        break;
    case 9:
        R_AddSpotShadowEntCmd((const GfxSpotShadowEntCmd *)data);
        break;
    case 10:
        R_GenerateShadowCookiesCmd((ShadowCookieCmd *)data);
        break;
    case 11:
        R_UpdateGfxEntityBoundsCmd((GfxSceneEntity **)data);
        break;
    case 12:
        R_SkinGfxEntityCmd((GfxSceneEntity **)data);
        break;
    case 13:
        if (!dx.deviceLost)
            FX_GenerateVerts((FxGenerateVertsCmd *)data);
        break;
    case 14:
        if (!dx.deviceLost)
            FX_GenerateMarkVertsForWorld(((FxCmd *)data)->localClientNum);
        break;
    case 15:
        R_SkinCachedStaticModelCmd((SkinCachedStaticModelCmd *)data);
        break;
    case 16:
        R_SkinXModelCmd((WORD*)data);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\r_workercmds.cpp", 635, 0, "unhandled case");
        break;
    }
}

void R_InitWorkerThreads()
{
    unsigned int workerThreadIndexa; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_workercmds.cpp", 970, 0, "%s", "Sys_IsMainThread()");
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

int R_InitWorkerCmds()
{
    g_workerCmds[0].buf = (unsigned __int8 *)g_UpdateFxSpotLightBuf;
    g_workerCmds[0].bufSize = 12;
    g_workerCmds[0].dataSize = 12;

    g_workerCmds[1].buf = (unsigned __int8 *)g_UpdateFxNonDependentBuf;
    g_workerCmds[1].bufSize = 12;
    g_workerCmds[1].dataSize = 12;

    g_workerCmds[2].buf = (unsigned __int8 *)g_UpdateFxRemainingBuf;
    g_workerCmds[2].bufSize = 12;
    g_workerCmds[2].dataSize = 12;

    g_workerCmds[3].buf = (unsigned __int8 *)g_dpvsCellStaticBuf;
    g_workerCmds[3].bufSize = 3072;
    g_workerCmds[3].dataSize = 12;

    g_workerCmds[4].buf = (unsigned __int8 *)g_dpvsCellSceneEntBuf;
    g_workerCmds[4].bufSize = 6144;
    g_workerCmds[4].dataSize = 12;

    g_workerCmds[5].buf = (unsigned __int8 *)g_dpvsCellDynModelBuf;
    g_workerCmds[5].bufSize = 6144;
    g_workerCmds[5].dataSize = 12;

    g_workerCmds[6].buf = (unsigned __int8 *)g_dpvsCellDynBrushBuf;
    g_workerCmds[6].bufSize = 6144;
    g_workerCmds[6].dataSize = 12;

    g_workerCmds[7].buf = (unsigned __int8 *)g_dpvsEntityBuf;
    g_workerCmds[7].bufSize = 0x8000;
    g_workerCmds[7].dataSize = 16;

    g_workerCmds[8].buf = (unsigned __int8 *)g_addSceneEntBuf;
    g_workerCmds[8].bufSize = 4;
    g_workerCmds[8].dataSize = 4;

    g_workerCmds[9].buf = (unsigned __int8 *)g_spotShadowEntBuf;
    g_workerCmds[9].bufSize = 2048;
    g_workerCmds[9].dataSize = 8;

    g_workerCmds[10].buf = (unsigned __int8 *)g_shadowCookieBuf;
    g_workerCmds[10].bufSize = 16;
    g_workerCmds[10].dataSize = 16;

    g_workerCmds[11].buf = (unsigned __int8 *)g_GfxEntityBoundsBuf;
    g_workerCmds[11].bufSize = 1024;
    g_workerCmds[11].dataSize = 4;

    g_workerCmds[12].buf = (unsigned __int8 *)g_SkinGfxEntityBuf;
    g_workerCmds[12].bufSize = 4096;
    g_workerCmds[12].dataSize = 4;

    g_workerCmds[13].buf = (unsigned __int8 *)g_GenerateFxVertsBuf;
    g_workerCmds[13].bufSize = 136;
    g_workerCmds[13].dataSize = 68;

    g_workerCmds[14].buf = (unsigned __int8 *)g_GenerateMarkVertsBuf;
    g_workerCmds[14].bufSize = 12;
    g_workerCmds[14].dataSize = 12;

    g_workerCmds[15].buf = (unsigned __int8 *)g_skinCachedStaticModelBuf;
    g_workerCmds[15].bufSize = 2048;
    g_workerCmds[15].dataSize = 4;

    g_workerCmds[16].buf = (unsigned __int8 *)g_SkinXModelBuf;
    g_workerCmds[16].bufSize = 28672;
    g_workerCmds[16].dataSize = 28;

    return R_InitWorkerCmdsPos();
}

int R_InitWorkerCmdsPos()
{
    int result; // eax
    WorkerCmds *workerCmds; // [esp+4h] [ebp-8h]
    int type; // [esp+8h] [ebp-4h]

    for (type = 0; type < 17; ++type)
    {
        workerCmds = &g_workerCmds[type];
        workerCmds->startPos = 0;
        workerCmds->endPos = workerCmds->bufSize;
        workerCmds->syncedEndPos = 0;
        workerCmds->inSize = 0;
        workerCmds->outSize = 0;
        if (!workerCmds->dataSize)
            MyAssertHandler(".\\r_workercmds.cpp", 367, 0, "%s", "workerCmds->dataSize");
        workerCmds->bufCount = workerCmds->bufSize / workerCmds->dataSize;
        if (workerCmds->dataSize > 0xC0)
            MyAssertHandler(
                ".\\r_workercmds.cpp",
                369,
                0,
                "%s\n\t(workerCmds->dataSize) = %i",
                "(workerCmds->dataSize <= 192)",
                workerCmds->dataSize);
        result = type + 1;
    }
    return result;
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
        Profile_Begin(175);
        InterlockedIncrement(&g_workerCmdWaitCount);
        Sys_WaitForWorkerCmd();
        InterlockedDecrement(&g_workerCmdWaitCount);
        Profile_EndInternal(0);
        Profile_Begin(174);
        R_ProcessWorkerCmds();
        Profile_EndInternal(0);
    }
}

void __cdecl R_AddWorkerCmd(int type, unsigned __int8 *data)
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
        if (bufSize % dataSize)
            MyAssertHandler(".\\r_workercmds.cpp", 1007, 0, "%s", "!(bufSize % dataSize )");
        if (InterlockedExchangeAdd((LONG*)&workerCmds->inSize, 1) < bufCount)
        {
            endPos = InterlockedExchangeAdd((LONG*)&workerCmds->endPos, dataSize) % bufSize;
            if (endPos < 0)
                MyAssertHandler(".\\r_workercmds.cpp", 1014, 0, "%s\n\t(endPos) = %i", "(endPos >= 0)", endPos);
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
    Profile_Begin(176);
    if (g_cmdOutputBusy[type])
    {
        while (g_cmdOutputBusy[type](data))
            NET_Sleep(1u);
    }
    Profile_EndInternal(0);
    R_ProcessWorkerCmdInternal(type, (FxCmd*)data);
}

void __cdecl R_UpdateActiveWorkerThreads()
{
    char v0; // [esp+3h] [ebp-9h]
    unsigned int i; // [esp+4h] [ebp-8h]
    unsigned int workerIter; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_workercmds.cpp", 1119, 0, "%s", "Sys_IsMainThread()");
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
                    Sys_ResumeThread(workerIter + 2);
                else
                    Sys_SuspendThread(workerIter + 2);
            }
        }
        R_ReleaseThreadOwnership();
    }
}

void __cdecl R_WaitFrontendWorkerCmds()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_workercmds.cpp", 1146, 0, "%s", "Sys_IsMainThread()");
    Profile_Begin(176);
    KISAK_NULLSUB();
    R_ProcessWorkerCmdsWithTimeout(R_FinishedWorkerCmds, 1);
    Profile_EndInternal(0);
}

int __cdecl R_FinishedWorkerCmds()
{
    int type; // [esp+4h] [ebp-4h]

    for (type = 0; type < 17; ++type)
    {
        if (g_workerCmds[type].inSize > 0)
            return 0;
    }
    return 1;
}

void __cdecl R_WaitWorkerCmds()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_workercmds.cpp", 1162, 0, "%s", "Sys_IsMainThread()");
    Profile_Begin(176);
    KISAK_NULLSUB();
    R_ProcessWorkerCmdsWithTimeout(R_FinishedWorkerCmds, 1);
    Profile_EndInternal(0);
}

