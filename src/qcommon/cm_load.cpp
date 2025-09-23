#include "qcommon.h"
#include "mem_track.h"
#include <universal/com_memory.h>
#include <database/database.h>
#include <win32/win_local.h>
#include "com_bsp.h"
#include <gfx_d3d/rb_backend.h>

//Line 53199 : 0006 : 006e75b8       struct clipMap_t cm        82e975b8     cm_load.obj
clipMap_t cm;
cbrush_t g_box_brush[THREAD_CONTEXT_TRACE_COUNT];
cmodel_t g_box_model[THREAD_CONTEXT_TRACE_COUNT];

void __cdecl TRACK_cm_load()
{
    //track_static_alloc_internal(&cm, 284, "cm", 25);
    //track_static_alloc_internal(g_box_brush, 320, "g_box_brush", 25);
    //track_static_alloc_internal(g_box_model, 288, "g_box_model", 25);
}

static void CM_InitAllThreadData()
{
    unsigned int workerIndex; // [esp+0h] [ebp-4h]

    CM_InitThreadData(THREAD_CONTEXT_MAIN);
    CM_InitThreadData(THREAD_CONTEXT_BACKEND);

    // r_smp_worker threads
    for (workerIndex = 0; workerIndex < 2; ++workerIndex)
        CM_InitThreadData(workerIndex + 2);

#ifdef KISAK_SP
    CM_InitThreadData(THREAD_CONTEXT_SERVER);
#endif
}

void __cdecl CM_LoadMap(const char *name, int *checksum)
{
    if (!name || !*name)
        Com_Error(ERR_DROP, "CM_LoadMap: NULL name");
    CM_LoadMapData(name);
    CM_InitAllThreadData();
    cm.isInUse = 1;
    *checksum = cm.checksum;
}

extern TraceThreadInfo g_traceThreadInfo[THREAD_CONTEXT_COUNT];
void __cdecl CM_InitThreadData(unsigned int threadContext)
{
    TraceThreadInfo *traceThreadInfo; // [esp+8h] [ebp-4h]

    bcassert(threadContext, THREAD_CONTEXT_TRACE_COUNT);

    traceThreadInfo = &g_traceThreadInfo[threadContext];
    traceThreadInfo->checkcount.global = 0;
    traceThreadInfo->checkcount.partitions = (int *)Hunk_Alloc(4 * cm.partitionCount, "CM_InitThreadData", 28);
    traceThreadInfo->box_brush = &g_box_brush[threadContext];
    memcpy(traceThreadInfo->box_brush, cm.box_brush, sizeof(cbrush_t));
    traceThreadInfo->box_model = &g_box_model[threadContext];
    memcpy(traceThreadInfo->box_model, &cm.box_model, sizeof(cmodel_t));
}

void __cdecl CM_LoadMapData(const char *name)
{
    if (IsFastFileLoad())
        CM_LoadMapData_FastFile(name);
    else
        CM_LoadMapData_LoadObj(name);
}

void __cdecl CM_LoadMapData_FastFile(const char *name)
{
#ifdef KISAK_MP
    if (DB_FindXAssetHeader(ASSET_TYPE_CLIPMAP_PVS, name).clipMap != &cm)
        MyAssertHandler(".\\qcommon\\cm_load.cpp", 133, 0, "%s", "clipMap == &cm");
#elif KISAK_SP
    if (DB_FindXAssetHeader(ASSET_TYPE_CLIPMAP, name).clipMap != &cm)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\cm_load.cpp", 133, 0, "%s", "clipMap == &cm");
#endif
}

void __cdecl CM_Shutdown()
{
    const char *savedName; // [esp+0h] [ebp-4h]

    savedName = cm.name;
    Com_Memset((unsigned int *)&cm, 0, 284);
    cm.name = savedName;
    iassert( !cm.isInUse );
}

void __cdecl CM_Unload()
{
    iassert( IsFastFileLoad() );
    if (cm.isInUse)
        Sys_Error("Cannot unload collision while it is in use");
}

int __cdecl CM_LeafCluster(unsigned int leafnum)
{
    bcassert(leafnum, cm.numLeafs);
    return cm.leafs[leafnum].cluster;
}

void __cdecl CM_ModelBounds(unsigned int model, float *mins, float *maxs)
{
    cmodel_t *cmodel; // eax

    cmodel = CM_ClipHandleToModel(model);

    mins[0] = cmodel->mins[0];
    mins[1] = cmodel->mins[1];
    mins[2] = cmodel->mins[2];

    maxs[0] = cmodel->maxs[0];
    maxs[1] = cmodel->maxs[1];
    maxs[2] = cmodel->maxs[2];
}

