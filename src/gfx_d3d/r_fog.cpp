#include "r_fog.h"
#include "r_init.h"


void __cdecl R_ClearFogs()
{
    memset((unsigned __int8 *)rg.fogSettings, 0, sizeof(rg.fogSettings));
    rg.fogIndex = 0;
}

void __cdecl R_SetFogFromServer(float start, unsigned __int8 r, unsigned __int8 g, unsigned __int8 b, float density)
{
    rg.fogSettings[1].color.packed = (r << 16) | (g << 8) | b | 0xFF000000;
    rg.fogSettings[1].fogStart = start;
    rg.fogSettings[1].density = density;
}

void __cdecl R_SwitchFog(unsigned int fogvar, int startTime, int transitionTime)
{
    if (fogvar > 4)
        MyAssertHandler(".\\r_fog.cpp", 32, 0, "%s\n\t(fogvar) = %i", "(fogvar >= FOG_NONE && fogvar < FOG_COUNT)", fogvar);
    rg.fogIndex = fogvar;
    if (rg.fogSettings[2].density == 0.0)
    {
        rg.fogSettings[3] = rg.fogSettings[rg.fogIndex];
        transitionTime = 0;
    }
    else
    {
        rg.fogSettings[3] = rg.fogSettings[2];
    }
    rg.fogSettings[4] = rg.fogSettings[rg.fogIndex];
    if (transitionTime)
    {
        rg.fogSettings[4].startTime = startTime;
        rg.fogSettings[4].finishTime = transitionTime + startTime;
    }
    else
    {
        rg.fogSettings[4].startTime = 0;
        rg.fogSettings[4].finishTime = 0;
    }
}

void __cdecl R_ArchiveFogState(MemoryFile *memFile)
{
    MemFile_ArchiveData(memFile, 100, rg.fogSettings);
    MemFile_ArchiveData(memFile, 4, &rg.fogIndex);
}

