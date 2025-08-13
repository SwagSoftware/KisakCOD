#include "qcommon.h"
#include "mem_track.h"
#include <universal/com_memory.h>
#include <database/database.h>
#include <win32/win_local.h>
#include "com_bsp.h"
#include <gfx_d3d/rb_backend.h>

//Line 53199 : 0006 : 006e75b8       struct clipMap_t cm        82e975b8     cm_load.obj
clipMap_t cm;
cbrush_t g_box_brush[4];
cmodel_t g_box_model[4];

void __cdecl TRACK_cm_load()
{
    track_static_alloc_internal(&cm, 284, "cm", 25);
    track_static_alloc_internal(g_box_brush, 320, "g_box_brush", 25);
    track_static_alloc_internal(g_box_model, 288, "g_box_model", 25);
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

void CM_InitAllThreadData()
{
    unsigned int workerIndex; // [esp+0h] [ebp-4h]

    CM_InitThreadData(0);
    CM_InitThreadData(1);
    for (workerIndex = 0; workerIndex < 2; ++workerIndex)
        CM_InitThreadData(workerIndex + 2);
}

extern TraceThreadInfo g_traceThreadInfo[THREAD_CONTEXT_COUNT];
void __cdecl CM_InitThreadData(unsigned int threadContext)
{
    TraceThreadInfo *traceThreadInfo; // [esp+8h] [ebp-4h]

    if (threadContext >= 4)
        MyAssertHandler(
            ".\\qcommon\\cm_load.cpp",
            60,
            0,
            "threadContext doesn't index THREAD_CONTEXT_TRACE_COUNT\n\t%i not in [0, %i)",
            threadContext,
            4);
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
    if (useFastFile->current.enabled)
        CM_LoadMapData_FastFile(name);
    else
        CM_LoadMapData_LoadObj(name);
}

void __cdecl CM_LoadMapData_FastFile(const char *name)
{
    if (DB_FindXAssetHeader(ASSET_TYPE_CLIPMAP_PVS, name).clipMap != &cm)
        MyAssertHandler(".\\qcommon\\cm_load.cpp", 133, 0, "%s", "clipMap == &cm");
}

void __cdecl CM_Shutdown()
{
    const char *savedName; // [esp+0h] [ebp-4h]

    savedName = cm.name;
    Com_Memset((unsigned int *)&cm, 0, 284);
    cm.name = savedName;
    if (cm.isInUse)
        MyAssertHandler(".\\qcommon\\cm_load.cpp", 177, 0, "%s", "!cm.isInUse");
}

void __cdecl CM_Unload()
{
    if (!useFastFile->current.enabled)
        MyAssertHandler(".\\qcommon\\cm_load.cpp", 190, 0, "%s", "IsFastFileLoad()");
    if (cm.isInUse)
        Sys_Error("Cannot unload collision while it is in use");
}

int __cdecl CM_LeafCluster(unsigned int leafnum)
{
    if (leafnum >= cm.numLeafs)
        MyAssertHandler(
            ".\\qcommon\\cm_load.cpp",
            200,
            0,
            "leafnum doesn't index cm.numLeafs\n\t%i not in [0, %i)",
            leafnum,
            cm.numLeafs);
    return cm.leafs[leafnum].cluster;
}

void __cdecl CM_ModelBounds(unsigned int model, float *mins, float *maxs)
{
    cmodel_t *v3; // eax

    v3 = CM_ClipHandleToModel(model);
    *mins = v3->mins[0];
    mins[1] = v3->mins[1];
    mins[2] = v3->mins[2];
    *maxs = v3->maxs[0];
    maxs[1] = v3->maxs[1];
    maxs[2] = v3->maxs[2];
}

