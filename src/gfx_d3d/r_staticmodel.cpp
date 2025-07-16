#include "r_staticmodel.h"
#include <qcommon/mem_track.h>
#include "r_init.h"
#include "r_rendercmds.h"
#include <universal/com_files.h>
#include <xanim/xmodel.h>
#include "r_xsurface.h"
#include <cgame_mp/cg_local_mp.h>


void __cdecl R_StaticModelWriteInfoHeader(int fileHandle)
{
    char dest[4104]; // [esp+10h] [ebp-1008h] BYREF

    *(DWORD*)&dest[4100] = 4096;
    Com_sprintf(
        dest,
        0x1000u,
        "index,name,radius,numLods,lodDist,lodPixels720p,1PixelDist720p,scaledDist,posx,posy,posz,pixels\n");
    FS_Write(dest, &dest[strlen(dest) + 1] - &dest[1], fileHandle);
}

bool __cdecl R_StaticModelHasLighting(unsigned int smodelIndex)
{
    return rgp.world->dpvs.smodelDrawInsts[smodelIndex].lightingHandle != 0;
}

int __cdecl R_StaticModelGetMemoryUsageInst()
{
    return 76;
}

int __cdecl R_StaticModelGetMemoryUsage(XModel *model, int *modelCount)
{
    int cellIndex; // [esp+0h] [ebp-14h]
    GfxAabbTree *tree; // [esp+8h] [ebp-Ch]
    int smodelIndexIter; // [esp+Ch] [ebp-8h]
    int usage; // [esp+10h] [ebp-4h]

    usage = 0;
    *modelCount = 0;
    if (!rgp.world)
        return 0;
    for (cellIndex = 0; cellIndex < rgp.world->dpvsPlanes.cellCount; ++cellIndex)
    {
        tree = rgp.world->cells[cellIndex].aabbTree;
        for (smodelIndexIter = 0; smodelIndexIter < tree->smodelIndexCount; ++smodelIndexIter)
        {
            if (rgp.world->dpvs.smodelDrawInsts[tree->smodelIndexes[smodelIndexIter]].model == model)
            {
                usage += R_StaticModelGetMemoryUsageInst();
                ++*modelCount;
            }
        }
    }
    return usage;
}

bool __cdecl R_StaticModelCompare(
    const GfxStaticModelCombinedInst &smodelInst0,
    const GfxStaticModelCombinedInst &smodelInst1)
{
    const ComPrimaryLight *primaryLight; // [esp+0h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-4h]
    int comparisona; // [esp+8h] [ebp-4h]
    int comparisonb; // [esp+8h] [ebp-4h]

    primaryLight = Com_GetPrimaryLight(smodelInst0.smodelDrawInst.primaryLightIndex);
    comparison = primaryLight->type - Com_GetPrimaryLight(smodelInst1.smodelDrawInst.primaryLightIndex)->type;
    if (comparison)
        return comparison < 0;
    comparisona = smodelInst0.smodelDrawInst.primaryLightIndex - smodelInst1.smodelDrawInst.primaryLightIndex;
    if (comparisona)
        return comparisona < 0;
    comparisonb = smodelInst0.smodelDrawInst.model - smodelInst1.smodelDrawInst.model;
    if (!comparisonb)
        comparisonb = smodelInst0.smodelDrawInst.reflectionProbeIndex - smodelInst1.smodelDrawInst.reflectionProbeIndex;
    return comparisonb < 0;
}
