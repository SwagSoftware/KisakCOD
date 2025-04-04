#include "r_staticmodel.h"
#include <qcommon/mem_track.h>
#include "r_init.h"
#include "r_rendercmds.h"


void __cdecl TRACK_r_staticmodelcache()
{
	track_static_alloc_internal(&s_cache, 266464, "s_cache", 18);
}

void __cdecl R_StaticModelWriteInfoHeader(int fileHandle)
{
    char dest; // [esp+10h] [ebp-1008h] BYREF
    _BYTE v2[4103]; // [esp+11h] [ebp-1007h] BYREF

    *(_DWORD *)&v2[4099] = 4096;
    Com_sprintf(
        &dest,
        0x1000u,
        "index,name,radius,numLods,lodDist,lodPixels720p,1PixelDist720p,scaledDist,posx,posy,posz,pixels\n");
    FS_Write(&dest, &v2[strlen(&dest)] - v2, fileHandle);
}

void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, const float dist)
{
    float v3; // [esp+5Ch] [ebp-101Ch]
    char dest; // [esp+60h] [ebp-1018h] BYREF
    _BYTE v5[4103]; // [esp+61h] [ebp-1017h] BYREF
    float v6; // [esp+1068h] [ebp-10h]
    XModel *model; // [esp+106Ch] [ebp-Ch]
    float v8; // [esp+1070h] [ebp-8h]
    float v9; // [esp+1074h] [ebp-4h]

    if ((_S1_0 & 1) == 0)
    {
        _S1_0 |= 1u;
        v3 = tan(22.5);
        radius2pixels = 720.0 / v3;
    }
    *(_DWORD *)&v5[4099] = 4096;
    model = smodelDrawInst->model;
    if (!model)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 538, 0, "%s", "xmodel");
    if (!model->name)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 539, 0, "%s", "xmodel->name");
    if (model->numLods <= 0)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 540, 0, "%s", "xmodel->numLods > 0");
    v9 = *((float *)&model->parentList + 7 * model->numLods);
    if (v9 <= 0.0)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 544, 0, "%s", "lodDist > 0.0f");
    v6 = radius2pixels * model->radius / v9;
    v8 = radius2pixels * model->radius;
    ++g_dumpStaticModelCount;
    if (smodelDrawInst->placement.scale > 0.0 && dist > 0.0)
        Com_sprintf(
            &dest,
            0x1000u,
            "%d,%s,%.1f,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
            g_dumpStaticModelCount,
            model->name,
            model->radius,
            model->numLods,
            v9,
            v6,
            v8,
            dist / smodelDrawInst->placement.scale,
            smodelDrawInst->placement.origin[0],
            smodelDrawInst->placement.origin[1],
            smodelDrawInst->placement.origin[2],
            v8 / (dist / smodelDrawInst->placement.scale));
    FS_Write(&dest, &v5[strlen(&dest)] - v5, fileHandle);
}

BOOL __cdecl R_StaticModelHasLighting(unsigned int smodelIndex)
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

BOOL __cdecl R_StaticModelCompare(
    const GfxStaticModelCombinedInst *smodelInst0,
    const GfxStaticModelCombinedInst *smodelInst1)
{
    const ComPrimaryLight *primaryLight; // [esp+0h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-4h]
    int comparisona; // [esp+8h] [ebp-4h]
    int comparisonb; // [esp+8h] [ebp-4h]

    primaryLight = Com_GetPrimaryLight(smodelInst0->smodelDrawInst.primaryLightIndex);
    comparison = primaryLight->type - Com_GetPrimaryLight(smodelInst1->smodelDrawInst.primaryLightIndex)->type;
    if (comparison)
        return comparison < 0;
    comparisona = smodelInst0->smodelDrawInst.primaryLightIndex - smodelInst1->smodelDrawInst.primaryLightIndex;
    if (comparisona)
        return comparisona < 0;
    comparisonb = smodelInst0->smodelDrawInst.model - smodelInst1->smodelDrawInst.model;
    if (!comparisonb)
        comparisonb = smodelInst0->smodelDrawInst.reflectionProbeIndex - smodelInst1->smodelDrawInst.reflectionProbeIndex;
    return comparisonb < 0;
}

void __cdecl R_StaticModelCacheStats_f()
{
    unsigned int usedCount; // [esp+24h] [ebp-24h]
    unsigned int lodIter; // [esp+28h] [ebp-20h]
    unsigned int allocCount; // [esp+2Ch] [ebp-1Ch]
    unsigned int surfCount; // [esp+30h] [ebp-18h]
    unsigned int smodelIter; // [esp+34h] [ebp-14h]
    unsigned int lodCount; // [esp+38h] [ebp-10h]
    const GfxStaticModelDrawInst *drawInst; // [esp+3Ch] [ebp-Ch]
    unsigned int surfIter; // [esp+40h] [ebp-8h]
    XSurface *surfs; // [esp+44h] [ebp-4h] BYREF

    if (rgp.world)
    {
        allocCount = 0;
        usedCount = 0;
        for (smodelIter = 0; smodelIter < rgp.world->dpvs.smodelCount; ++smodelIter)
        {
            drawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIter];
            lodCount = XModelGetNumLods(drawInst->model);
            for (lodIter = 0; lodIter < lodCount; ++lodIter)
            {
                if (drawInst->smodelCacheIndex[lodIter])
                {
                    allocCount += 1 << XModelGetLodInfo(drawInst->model, lodIter)->smcAllocBits;
                    surfCount = XModelGetSurfCount(drawInst->model, lodIter);
                    XModelGetSurfaces(drawInst->model, &surfs, lodIter);
                    for (surfIter = 0; surfIter < surfCount; ++surfIter)
                        usedCount += XSurfaceGetNumVerts(&surfs[surfIter]);
                }
            }
        }
        Com_Printf(8, "%.2f%% of cache is currently allocated.\n", (double)allocCount * 100.0 / 262144.0);
        if (allocCount)
            Com_Printf(8, "%.2f%% allocated cache vertices are used.\n", (double)usedCount * 100.0 / (double)allocCount);
    }
}

void __cdecl R_StaticModelCacheFlush_f()
{
    R_SyncRenderThread();
    R_ClearAllStaticModelCacheRefs();
}

void __cdecl R_ClearAllStaticModelCacheRefs()
{
    unsigned int smodelCount; // [esp+0h] [ebp-8h]
    unsigned int smodelIndex; // [esp+4h] [ebp-4h]

    if (rgp.world)
    {
        smodelCount = rgp.world->dpvs.smodelCount;
        for (smodelIndex = 0; smodelIndex < smodelCount; ++smodelIndex)
            R_UncacheStaticModel(smodelIndex);
    }
}