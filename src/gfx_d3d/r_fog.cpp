#include "r_fog.h"
#include "r_init.h"

#include <universal/memfile.h>

void __cdecl R_ClearFogs()
{
    memset(rg.fogSettings, 0, sizeof(rg.fogSettings));
    rg.fogIndex = 0;
}

void __cdecl R_SetFogFromServer(float start, uint8_t r, uint8_t g, uint8_t b, float density)
{
    rg.fogSettings[FOG_SERVER].color.packed = (r << 16) | (g << 8) | b | 0xFF000000;
    rg.fogSettings[FOG_SERVER].fogStart = start;
    rg.fogSettings[FOG_SERVER].density = density;
}

void __cdecl R_SwitchFog(uint32_t fogvar, int startTime, int transitionTime)
{
    iassert( (fogvar >= FOG_NONE && fogvar < FOG_COUNT) );
    rg.fogIndex = fogvar;
    if (rg.fogSettings[FOG_CURRENT].density == 0.0)
    {
        rg.fogSettings[FOG_LAST] = rg.fogSettings[rg.fogIndex];
        transitionTime = 0;
    }
    else
    {
        rg.fogSettings[FOG_LAST] = rg.fogSettings[2];
    }
    rg.fogSettings[FOG_TARGET] = rg.fogSettings[rg.fogIndex];
    if (transitionTime)
    {
        rg.fogSettings[FOG_TARGET].startTime = startTime;
        rg.fogSettings[FOG_TARGET].finishTime = transitionTime + startTime;
    }
    else
    {
        rg.fogSettings[FOG_TARGET].startTime = 0;
        rg.fogSettings[FOG_TARGET].finishTime = 0;
    }
}

void __cdecl R_ArchiveFogState(MemoryFile *memFile)
{
    static_assert(sizeof(GfxFog) * FOG_COUNT == 100);

    MemFile_ArchiveData(memFile, sizeof(GfxFog) * FOG_COUNT, rg.fogSettings);
    MemFile_ArchiveData(memFile, sizeof(int), &rg.fogIndex);
}

