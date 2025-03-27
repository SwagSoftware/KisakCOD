#include "qcommon.h"
#include "mem_track.h"

statmonitor_s stats[7];
int statCount;

void __cdecl TRACK_statmonitor()
{
    track_static_alloc_internal(stats, 56, "stats", 10);
}

void __cdecl StatMon_Warning(int type, int duration, const char *materialName)
{
    if (com_statmon->current.enabled)
    {
        if ((unsigned int)type >= 7)
            MyAssertHandler(
                ".\\qcommon\\statmonitor.cpp",
                41,
                0,
                "type doesn't index ARRAY_COUNT( stats )\n\t%i not in [0, %i)",
                type,
                7);
        Sys_EnterCriticalSection(CRITSECT_STATMON);
        stats[type].endtime = duration + Sys_Milliseconds();
        if (!stats[type].material && cls.rendererStarted)
            stats[type].material = Material_RegisterHandle(materialName, 1);
        if (type >= statCount)
            statCount = type + 1;
        Sys_LeaveCriticalSection(CRITSECT_STATMON);
    }
}

void __cdecl StatMon_GetStatsArray(const statmonitor_s **array, int *count)
{
    *array = stats;
    *count = statCount;
}

void __cdecl StatMon_Reset()
{
    memset((unsigned __int8 *)stats, 0, sizeof(stats));
    statCount = 0;
}

