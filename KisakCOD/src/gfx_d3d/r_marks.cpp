#include "r_marks.h"

#include "r_gfx.h"
#include "r_init.h"
#include <xanim/dobj_utils.h>
#include <cgame_mp/cg_local_mp.h>
#include <EffectsCore/fx_system.h>
#include "r_model_skin.h"

void  R_BoxSurfaces(
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    unsigned int surfListSize,
    unsigned int *surfCounts,
    unsigned int listCount)
{
    unsigned int j; // [esp-Ch] [ebp-A4h]
    unsigned int i; // [esp-8h] [ebp-A0h]
    unsigned int v11[2]; // [esp-4h] [ebp-9Ch] BYREF
    int v12; // [esp+8Ch] [ebp-Ch]
    void *v13; // [esp+90h] [ebp-8h]
    void *retaddr; // [esp+98h] [ebp+0h]

    //v12 = a1;
    v13 = retaddr;
    if (!rgp.world)
        MyAssertHandler(".\\r_marks.cpp", 944, 0, "%s", "rgp.world");
    if (rgp.world->dpvsPlanes.cellCount > 1024)
        MyAssertHandler(
            ".\\r_marks.cpp",
            945,
            0,
            "%s\n\t(rgp.world->dpvsPlanes.cellCount) = %i",
            "(rgp.world->dpvsPlanes.cellCount <= (1024))",
            rgp.world->dpvsPlanes.cellCount);
    if (rgp.world->cellBitsCount > 128)
        MyAssertHandler(
            ".\\r_marks.cpp",
            946,
            0,
            "%s\n\t(rgp.world->cellBitsCount) = %i",
            "(rgp.world->cellBitsCount <= ((1024) >> 3))",
            rgp.world->cellBitsCount);
    Com_Memset(v11, 0, rgp.world->cellBitsCount);
    for (i = 0; i < listCount; ++i)
        surfCounts[i] = 0;
    R_BoxSurfaces_r(
        (mnode_t *)rgp.world->dpvsPlanes.nodes,
        mins,
        maxs,
        allowSurf,
        callbackContext,
        surfLists,
        surfListSize,
        surfCounts,
        listCount,
        (unsigned __int8 *)v11);
    for (j = 0; j < listCount; ++j)
    {
        if (surfCounts[j] > surfListSize)
            MyAssertHandler(".\\r_marks.cpp", 956, 0, "surfCounts[i] <= surfListSize\n\t%i, %i", surfCounts[j], surfListSize);
    }
}

void __cdecl R_BoxSurfaces_r(
    mnode_t *node,
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    unsigned int surfListSize,
    unsigned int *surfCounts,
    unsigned int listCount,
    unsigned __int8 *cellBits)
{
    const cplane_s *side; // [esp+0h] [ebp-10h]
    int cellIndex; // [esp+4h] [ebp-Ch]
    int cellCount; // [esp+8h] [ebp-8h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while (1)
    {
        cellIndex = node->cellIndex;
        if (cellIndex - cellCount < 0)
            break;
        side = (const cplane_s *)BoxOnPlaneSide(mins, maxs, &rgp.world->dpvsPlanes.planes[cellIndex - cellCount], side);
        if (side == (const cplane_s *)1)
        {
            ++node;
        }
        else
        {
            if (side != (const cplane_s *)2)
                R_BoxSurfaces_r(
                    node + 1,
                    mins,
                    maxs,
                    allowSurf,
                    callbackContext,
                    surfLists,
                    surfListSize,
                    surfCounts,
                    listCount,
                    cellBits);
            node = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
        }
    }
    if (node->cellIndex)
    {
        if (listCount == 1)
        {
            R_CellSurfaces(
                cellIndex - 1,
                mins,
                maxs,
                *allowSurf,
                callbackContext,
                *surfLists,
                surfListSize,
                surfCounts,
                cellBits);
        }
        else
        {
            if (listCount != 2)
                MyAssertHandler(".\\r_marks.cpp", 792, 0, "%s", "listCount == 2");
            R_CellSurfacesTwoLists(
                cellIndex - 1,
                mins,
                maxs,
                allowSurf,
                callbackContext,
                surfLists,
                surfListSize,
                surfCounts,
                cellBits);
        }
    }
}

void __cdecl R_CellSurfaces(
    int cellIndex,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    unsigned int surfListSize,
    unsigned int *surfCount,
    unsigned __int8 *cellBits)
{
    GfxCell *cell; // [esp+0h] [ebp-24h]
    int surfIndex; // [esp+4h] [ebp-20h]
    int remappedSurfIndex; // [esp+Ch] [ebp-18h]
    int cullGroupIndex; // [esp+10h] [ebp-14h]
    GfxCullGroup *group; // [esp+14h] [ebp-10h]
    GfxSurface *surf; // [esp+18h] [ebp-Ch]
    int cellIndexBit; // [esp+1Ch] [ebp-8h]
    int triSurfIndex; // [esp+20h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if (((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0)
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        cell = &rgp.world->cells[cellIndex];
        for (cullGroupIndex = 0; cullGroupIndex < cell->cullGroupCount; ++cullGroupIndex)
        {
            group = &rgp.world->dpvs.cullGroups[cell->cullGroups[cullGroupIndex]];
            if (*mins <= (double)group->maxs[0]
                && *maxs >= (double)group->mins[0]
                && mins[1] <= (double)group->maxs[1]
                && maxs[1] >= (double)group->mins[1]
                && mins[2] <= (double)group->maxs[2]
                && maxs[2] >= (double)group->mins[2])
            {
                triSurfIndex = 0;
                surfIndex = group->startSurfIndex;
                while (triSurfIndex < group->surfaceCount && *surfCount < surfListSize)
                {
                    remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                    if (allowSurf(remappedSurfIndex, callbackContext))
                    {
                        surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                        if (*mins <= (double)surf->bounds[1][0]
                            && *maxs >= (double)surf->bounds[0][0]
                            && mins[1] <= (double)surf->bounds[1][1]
                            && maxs[1] >= (double)surf->bounds[0][1]
                            && mins[2] <= (double)surf->bounds[1][2]
                            && maxs[2] >= (double)surf->bounds[0][2])
                        {
                            R_AddSurfaceToList(surf, surfList, surfCount);
                        }
                    }
                    ++triSurfIndex;
                    ++surfIndex;
                }
            }
        }
        R_AABBTreeSurfaces_r(cell->aabbTree, mins, maxs, allowSurf, callbackContext, surfList, surfListSize, surfCount);
    }
}

void __cdecl R_AABBTreeSurfaces_r(
    GfxAabbTree *tree,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    unsigned int surfListSize,
    unsigned int *surfCount)
{
    unsigned int listIndex; // [esp+0h] [ebp-1Ch]
    GfxAabbTree *children; // [esp+4h] [ebp-18h]
    unsigned int surfIndex; // [esp+8h] [ebp-14h]
    unsigned int childIndex; // [esp+Ch] [ebp-10h]
    unsigned int remappedSurfIndex; // [esp+10h] [ebp-Ch]
    GfxSurface *surf; // [esp+14h] [ebp-8h]
    unsigned int surfNodeIndex; // [esp+18h] [ebp-4h]

    if (*mins <= (double)tree->maxs[0]
        && *maxs >= (double)tree->mins[0]
        && mins[1] <= (double)tree->maxs[1]
        && maxs[1] >= (double)tree->mins[1]
        && mins[2] <= (double)tree->maxs[2]
        && maxs[2] >= (double)tree->mins[2])
    {
        if (tree->childCount)
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
                R_AABBTreeSurfaces_r(
                    &children[childIndex],
                    mins,
                    maxs,
                    allowSurf,
                    callbackContext,
                    surfList,
                    surfListSize,
                    surfCount);
        }
        else
        {
            surfNodeIndex = 0;
            surfIndex = tree->startSurfIndex;
            while (surfNodeIndex < tree->surfaceCount && *surfCount < surfListSize)
            {
                remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                if (allowSurf(remappedSurfIndex, callbackContext))
                {
                    surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                    if (*mins <= (double)surf->bounds[1][0]
                        && *maxs >= (double)surf->bounds[0][0]
                        && mins[1] <= (double)surf->bounds[1][1]
                        && maxs[1] >= (double)surf->bounds[0][1]
                        && mins[2] <= (double)surf->bounds[1][2]
                        && maxs[2] >= (double)surf->bounds[0][2])
                    {
                        for (listIndex = 0; listIndex < *surfCount; ++listIndex)
                        {
                            if (surfList[listIndex] == surf)
                                MyAssertHandler(".\\r_marks.cpp", 397, 0, "%s", "surfList[listIndex] != surf");
                        }
                        surfList[(*surfCount)++] = surf;
                    }
                }
                ++surfNodeIndex;
                ++surfIndex;
            }
        }
    }
}

void __cdecl R_AddSurfaceToList(GfxSurface *surf, GfxSurface **surfList, unsigned int *surfCount)
{
    unsigned int listIndex; // [esp+0h] [ebp-4h]

    for (listIndex = 0; listIndex < *surfCount; ++listIndex)
    {
        if (surfList[listIndex] == surf)
            return;
    }
    surfList[(*surfCount)++] = surf;
}

void __cdecl R_CellSurfacesTwoLists(
    int cellIndex,
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    unsigned int surfListSize,
    unsigned int *surfCounts,
    unsigned __int8 *cellBits)
{
    GfxCell *cell; // [esp+0h] [ebp-24h]
    int surfIndex; // [esp+4h] [ebp-20h]
    int remappedSurfIndex; // [esp+Ch] [ebp-18h]
    int cullGroupIndex; // [esp+10h] [ebp-14h]
    GfxCullGroup *group; // [esp+14h] [ebp-10h]
    GfxSurface *surf; // [esp+18h] [ebp-Ch]
    int cellIndexBit; // [esp+1Ch] [ebp-8h]
    int triSurfIndex; // [esp+20h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if (((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0)
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        cell = &rgp.world->cells[cellIndex];
        for (cullGroupIndex = 0; cullGroupIndex < cell->cullGroupCount; ++cullGroupIndex)
        {
            group = &rgp.world->dpvs.cullGroups[cell->cullGroups[cullGroupIndex]];
            if (*mins <= (double)group->maxs[0]
                && *maxs >= (double)group->mins[0]
                && mins[1] <= (double)group->maxs[1]
                && maxs[1] >= (double)group->mins[1]
                && mins[2] <= (double)group->maxs[2]
                && maxs[2] >= (double)group->mins[2])
            {
                triSurfIndex = 0;
                surfIndex = group->startSurfIndex;
                while (triSurfIndex < group->surfaceCount && *surfCounts < surfListSize && surfCounts[1] < surfListSize)
                {
                    remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                    surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                    if (*mins <= (double)surf->bounds[1][0]
                        && *maxs >= (double)surf->bounds[0][0]
                        && mins[1] <= (double)surf->bounds[1][1]
                        && maxs[1] >= (double)surf->bounds[0][1]
                        && mins[2] <= (double)surf->bounds[1][2]
                        && maxs[2] >= (double)surf->bounds[0][2])
                    {
                        if ((*allowSurf)(remappedSurfIndex, callbackContext))
                            R_AddSurfaceToList(surf, *surfLists, surfCounts);
                        if (allowSurf[1](remappedSurfIndex, callbackContext))
                            R_AddSurfaceToList(surf, surfLists[1], surfCounts + 1);
                    }
                    ++triSurfIndex;
                    ++surfIndex;
                }
            }
        }
        R_AABBTreeSurfacesTwoLists_r(
            cell->aabbTree,
            mins,
            maxs,
            allowSurf,
            callbackContext,
            surfLists,
            surfListSize,
            surfCounts);
    }
}

void __cdecl R_AABBTreeSurfacesTwoLists_r(
    GfxAabbTree *tree,
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    unsigned int surfListSize,
    unsigned int *surfCounts)
{
    unsigned int i; // [esp+0h] [ebp-20h]
    unsigned int listIndex; // [esp+4h] [ebp-1Ch]
    GfxAabbTree *children; // [esp+8h] [ebp-18h]
    unsigned int surfIndex; // [esp+Ch] [ebp-14h]
    unsigned int childIndex; // [esp+10h] [ebp-10h]
    unsigned int remappedSurfIndex; // [esp+14h] [ebp-Ch]
    GfxSurface *surf; // [esp+18h] [ebp-8h]
    unsigned int surfNodeIndex; // [esp+1Ch] [ebp-4h]

    if (*mins <= (double)tree->maxs[0]
        && *maxs >= (double)tree->mins[0]
        && mins[1] <= (double)tree->maxs[1]
        && maxs[1] >= (double)tree->mins[1]
        && mins[2] <= (double)tree->maxs[2]
        && maxs[2] >= (double)tree->mins[2])
    {
        if (tree->childCount)
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
                R_AABBTreeSurfacesTwoLists_r(
                    &children[childIndex],
                    mins,
                    maxs,
                    allowSurf,
                    callbackContext,
                    surfLists,
                    surfListSize,
                    surfCounts);
        }
        else
        {
            surfNodeIndex = 0;
            surfIndex = tree->startSurfIndex;
            while (surfNodeIndex < tree->surfaceCount && *surfCounts < surfListSize && surfCounts[1] < surfListSize)
            {
                remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                if (*mins <= (double)surf->bounds[1][0]
                    && *maxs >= (double)surf->bounds[0][0]
                    && mins[1] <= (double)surf->bounds[1][1]
                    && maxs[1] >= (double)surf->bounds[0][1]
                    && mins[2] <= (double)surf->bounds[1][2]
                    && maxs[2] >= (double)surf->bounds[0][2])
                {
                    for (listIndex = 0; listIndex < *surfCounts; ++listIndex)
                    {
                        if ((*surfLists)[listIndex] == surf)
                            MyAssertHandler(".\\r_marks.cpp", 443, 0, "%s", "surfLists[0][listIndex] != surf");
                    }
                    for (i = 0; i < surfCounts[1]; ++i)
                    {
                        if (surfLists[1][i] == surf)
                            MyAssertHandler(".\\r_marks.cpp", 445, 0, "%s", "surfLists[1][listIndex] != surf");
                    }
                    if ((*allowSurf)(remappedSurfIndex, callbackContext))
                        (*surfLists)[(*surfCounts)++] = surf;
                    if (allowSurf[1](remappedSurfIndex, callbackContext))
                        surfLists[1][surfCounts[1]++] = surf;
                }
                ++surfNodeIndex;
                ++surfIndex;
            }
        }
    }
}

int  R_BoxStaticModels(
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    unsigned __int16 *smodelList,
    int smodelListSize)
{
    int v7; // [esp-Ch] [ebp-A0h] BYREF
    unsigned int v8[2]; // [esp-8h] [ebp-9Ch] BYREF
    int v9; // [esp+88h] [ebp-Ch]
    void *v10; // [esp+8Ch] [ebp-8h]
    void *retaddr; // [esp+94h] [ebp+0h]

    //v9 = a1;
    v10 = retaddr;
    if (!rgp.world)
        MyAssertHandler(".\\r_marks.cpp", 966, 0, "%s", "rgp.world");
    if (rgp.world->dpvsPlanes.cellCount > 1024)
        MyAssertHandler(
            ".\\r_marks.cpp",
            967,
            0,
            "%s\n\t(rgp.world->dpvsPlanes.cellCount) = %i",
            "(rgp.world->dpvsPlanes.cellCount <= (1024))",
            rgp.world->dpvsPlanes.cellCount);
    if (rgp.world->cellBitsCount > 128)
        MyAssertHandler(
            ".\\r_marks.cpp",
            968,
            0,
            "%s\n\t(rgp.world->cellBitsCount) = %i",
            "(rgp.world->cellBitsCount <= ((1024) >> 3))",
            rgp.world->cellBitsCount);
    //Profile_Begin(215);
    Com_Memset(v8, 0, rgp.world->cellBitsCount);
    v7 = 0;
    R_BoxStaticModels_r(
        (mnode_t *)rgp.world->dpvsPlanes.nodes,
        mins,
        maxs,
        allowSModel,
        smodelList,
        smodelListSize,
        &v7,
        (unsigned __int8 *)v8);
    if (v7 > smodelListSize)
        MyAssertHandler(".\\r_marks.cpp", 975, 0, "smodelCount <= smodelListSize\n\t%i, %i", v7, smodelListSize);
    //Profile_EndInternal(0);
    return v7;
}

void __cdecl R_BoxStaticModels_r(
    mnode_t *node,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    unsigned __int16 *smodelList,
    int smodelListSize,
    int *smodelCount,
    unsigned __int8 *cellBits)
{
    const cplane_s *side; // [esp+0h] [ebp-10h]
    int cellIndex; // [esp+4h] [ebp-Ch]
    int cellCount; // [esp+8h] [ebp-8h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while (1)
    {
        cellIndex = node->cellIndex;
        if (cellIndex - cellCount < 0)
            break;
        side = (const cplane_s *)BoxOnPlaneSide(mins, maxs, &rgp.world->dpvsPlanes.planes[cellIndex - cellCount], side);
        if (side == (const cplane_s *)1)
        {
            ++node;
        }
        else
        {
            if (side != (const cplane_s *)2)
                R_BoxStaticModels_r(node + 1, mins, maxs, allowSModel, smodelList, smodelListSize, smodelCount, cellBits);
            node = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
        }
    }
    if (node->cellIndex)
        R_CellStaticModels(cellIndex - 1, mins, maxs, allowSModel, smodelList, smodelListSize, smodelCount, cellBits);
}

void __cdecl R_CellStaticModels(
    int cellIndex,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    unsigned __int16 *smodelList,
    int smodelListSize,
    int *smodelCount,
    unsigned __int8 *cellBits)
{
    int cellIndexBit; // [esp+8h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if (((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0)
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        R_AABBTreeStaticModels_r(
            rgp.world->cells[cellIndex].aabbTree,
            mins,
            maxs,
            allowSModel,
            smodelList,
            smodelListSize,
            smodelCount);
    }
}

void __cdecl R_AABBTreeStaticModels_r(
    GfxAabbTree *tree,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    unsigned __int16 *smodelList,
    int smodelListSize,
    int *smodelCount)
{
    GfxAabbTree *children; // [esp+0h] [ebp-14h]
    int smodelIndexIter; // [esp+4h] [ebp-10h]
    int childIndex; // [esp+8h] [ebp-Ch]
    const GfxStaticModelInst *smodelInst; // [esp+Ch] [ebp-8h]
    int smodelIndex; // [esp+10h] [ebp-4h]

    if (*mins <= (double)tree->maxs[0]
        && *maxs >= (double)tree->mins[0]
        && mins[1] <= (double)tree->maxs[1]
        && maxs[1] >= (double)tree->mins[1]
        && mins[2] <= (double)tree->maxs[2]
        && maxs[2] >= (double)tree->mins[2])
    {
        if (tree->childCount)
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
                R_AABBTreeStaticModels_r(
                    &children[childIndex],
                    mins,
                    maxs,
                    allowSModel,
                    smodelList,
                    smodelListSize,
                    smodelCount);
        }
        else
        {
            for (smodelIndexIter = 0;
                smodelIndexIter < tree->smodelIndexCount && *smodelCount < smodelListSize;
                ++smodelIndexIter)
            {
                smodelIndex = tree->smodelIndexes[smodelIndexIter];
                if (allowSModel(smodelIndex))
                {
                    smodelInst = &rgp.world->dpvs.smodelInsts[smodelIndex];
                    if (*mins <= (double)smodelInst->maxs[0]
                        && *maxs >= (double)smodelInst->mins[0]
                        && mins[1] <= (double)smodelInst->maxs[1]
                        && maxs[1] >= (double)smodelInst->mins[1]
                        && mins[2] <= (double)smodelInst->maxs[2]
                        && maxs[2] >= (double)smodelInst->mins[2])
                    {
                        R_AddStaticModelToList(smodelIndex, smodelList, smodelCount);
                    }
                }
            }
        }
    }
}

void __cdecl R_AddStaticModelToList(int smodelIndex, unsigned __int16 *smodelList, int *smodelCount)
{
    int listIndex; // [esp+0h] [ebp-4h]

    for (listIndex = 0; listIndex < *smodelCount; ++listIndex)
    {
        if (smodelList[listIndex] == smodelIndex)
            return;
    }
    smodelList[(*smodelCount)++] = smodelIndex;
}

unsigned int  R_CylinderSurfaces(
    const float *start,
    const float *end,
    float radius,
    const DpvsPlane *planes,
    unsigned int planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    unsigned int surfListSize)
{
    unsigned int v11; // [esp+30h] [ebp-A0h] BYREF
    unsigned int v12[2]; // [esp+34h] [ebp-9Ch] BYREF
    int v13; // [esp+C4h] [ebp-Ch]
    void *v14; // [esp+C8h] [ebp-8h]
    void *retaddr; // [esp+D0h] [ebp+0h]

    //v13 = a1;
    v14 = retaddr;
    if (!rgp.world)
        MyAssertHandler(".\\r_marks.cpp", 986, 0, "%s", "rgp.world");
    if (rgp.world->dpvsPlanes.cellCount > 1024)
        MyAssertHandler(
            ".\\r_marks.cpp",
            987,
            0,
            "%s\n\t(rgp.world->dpvsPlanes.cellCount) = %i",
            "(rgp.world->dpvsPlanes.cellCount <= (1024))",
            rgp.world->dpvsPlanes.cellCount);
    if (rgp.world->cellBitsCount > 128)
        MyAssertHandler(
            ".\\r_marks.cpp",
            988,
            0,
            "%s\n\t(rgp.world->cellBitsCount) = %i",
            "(rgp.world->cellBitsCount <= ((1024) >> 3))",
            rgp.world->cellBitsCount);
    Com_Memset(v12, 0, rgp.world->cellBitsCount);
    v11 = 0;
    R_CylinderSurfaces_r(
        (mnode_t *)rgp.world->dpvsPlanes.nodes,
        start,
        end,
        radius,
        planes,
        planeCount,
        allowSurf,
        callbackContext,
        surfList,
        surfListSize,
        &v11,
        (unsigned __int8 *)v12);
    if (v11 > surfListSize)
        MyAssertHandler(".\\r_marks.cpp", 994, 0, "surfCount <= surfListSize\n\t%i, %i", v11, surfListSize);
    return v11;
}

void __cdecl R_CylinderSurfaces_r(
    mnode_t *node,
    const float *start,
    const float *end,
    float radius,
    const DpvsPlane *planes,
    unsigned int planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    unsigned int surfListSize,
    unsigned int *surfCount,
    unsigned __int8 *cellBits)
{
    float v12; // [esp+24h] [ebp-64h]
    float v13; // [esp+28h] [ebp-60h]
    float endDist; // [esp+30h] [ebp-58h]
    cplane_s *plane; // [esp+34h] [ebp-54h]
    float delta[3]; // [esp+38h] [ebp-50h] BYREF
    int cellIndex; // [esp+44h] [ebp-44h]
    float startDist; // [esp+48h] [ebp-40h]
    float dist; // [esp+4Ch] [ebp-3Ch]
    float invDeltaDist; // [esp+50h] [ebp-38h]
    float fraction; // [esp+54h] [ebp-34h]
    float deltaDist; // [esp+58h] [ebp-30h]
    float end2[3]; // [esp+5Ch] [ebp-2Ch] BYREF
    int cellCount; // [esp+68h] [ebp-20h]
    float start2[3]; // [esp+6Ch] [ebp-1Ch] BYREF
    int planeIndex; // [esp+78h] [ebp-10h]
    float mid[3]; // [esp+7Ch] [ebp-Ch] BYREF

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    start2[0] = *start;
    start2[1] = start[1];
    start2[2] = start[2];
    end2[0] = *end;
    end2[1] = end[1];
    end2[2] = end[2];
    while (1)
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if (cellIndex - cellCount < 0)
            break;
        plane = &rgp.world->dpvsPlanes.planes[planeIndex];
        startDist = Vec3Dot(start2, plane->normal) - plane->dist;
        endDist = Vec3Dot(end2, plane->normal) - plane->dist;
        if (startDist * endDist <= 0.0
            || (v13 = fabs(startDist), radius > (double)v13)
            || (v12 = fabs(endDist), radius > (double)v12))
        {
            Vec3Sub(end2, start2, delta);
            deltaDist = endDist - startDist;
            if (deltaDist == 0.0)
            {
                R_CylinderSurfaces_r(
                    node + 1,
                    start2,
                    end2,
                    radius,
                    planes,
                    planeCount,
                    allowSurf,
                    callbackContext,
                    surfList,
                    surfListSize,
                    surfCount,
                    cellBits);
            }
            else
            {
                invDeltaDist = 1.0 / deltaDist;
                dist = -radius;
                fraction = (dist - startDist) * invDeltaDist;
                if (fraction <= 0.0 || fraction >= 1.0)
                {
                    R_CylinderSurfaces_r(
                        node + 1,
                        start2,
                        end2,
                        radius,
                        planes,
                        planeCount,
                        allowSurf,
                        callbackContext,
                        surfList,
                        surfListSize,
                        surfCount,
                        cellBits);
                }
                else
                {
                    Vec3Mad(start2, fraction, delta, mid);
                    if (dist > (double)startDist)
                    {
                        if (dist > (double)endDist)
                            MyAssertHandler(".\\r_marks.cpp", 894, 0, "%s", "endDist >= dist");
                        R_CylinderSurfaces_r(
                            node + 1,
                            mid,
                            end2,
                            radius,
                            planes,
                            planeCount,
                            allowSurf,
                            callbackContext,
                            surfList,
                            surfListSize,
                            surfCount,
                            cellBits);
                    }
                    else
                    {
                        if (dist <= (double)endDist)
                            MyAssertHandler(".\\r_marks.cpp", 889, 0, "%s", "endDist < dist");
                        R_CylinderSurfaces_r(
                            node + 1,
                            start2,
                            mid,
                            radius,
                            planes,
                            planeCount,
                            allowSurf,
                            callbackContext,
                            surfList,
                            surfListSize,
                            surfCount,
                            cellBits);
                    }
                }
                dist = radius;
                fraction = (radius - startDist) * invDeltaDist;
                if (fraction > 0.0 && fraction < 1.0)
                {
                    Vec3Mad(start2, fraction, delta, mid);
                    if (dist < (double)startDist)
                    {
                        if (dist < (double)endDist)
                            MyAssertHandler(".\\r_marks.cpp", 915, 0, "%s", "endDist <= dist");
                        start2[0] = mid[0];
                        start2[1] = mid[1];
                        start2[2] = mid[2];
                    }
                    else
                    {
                        if (dist >= (double)endDist)
                            MyAssertHandler(".\\r_marks.cpp", 910, 0, "%s", "endDist > dist");
                        end2[0] = mid[0];
                        end2[1] = mid[1];
                        end2[2] = mid[2];
                    }
                }
            }
            node = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
        }
        else
        {
            node = (mnode_t *)((char *)node + 2 * (startDist < 0.0) * (node->rightChildOffset - 2) + 4);
        }
    }
    if (cellIndex)
        R_CellCylinderSurfaces(
            cellIndex - 1,
            planes,
            planeCount,
            allowSurf,
            callbackContext,
            surfList,
            surfListSize,
            surfCount,
            cellBits);
}

void __cdecl R_CellCylinderSurfaces(
    int cellIndex,
    const DpvsPlane *planes,
    unsigned int planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    unsigned int surfListSize,
    unsigned int *surfCount,
    unsigned __int8 *cellBits)
{
    GfxCell *cell; // [esp+10h] [ebp-24h]
    unsigned int surfIndex; // [esp+14h] [ebp-20h]
    unsigned int remappedSurfIndex; // [esp+1Ch] [ebp-18h]
    int cullGroupIndex; // [esp+20h] [ebp-14h]
    GfxCullGroup *group; // [esp+24h] [ebp-10h]
    GfxSurface *surf; // [esp+28h] [ebp-Ch]
    unsigned int cellIndexBit; // [esp+2Ch] [ebp-8h]
    int triSurfIndex; // [esp+30h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if (((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0)
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        cell = &rgp.world->cells[cellIndex];
        for (cullGroupIndex = 0; cullGroupIndex < cell->cullGroupCount; ++cullGroupIndex)
        {
            group = &rgp.world->dpvs.cullGroups[cell->cullGroups[cullGroupIndex]];
            if (!R_OutsideFrustumPlanes(planes, planeCount, group->mins))
            {
                triSurfIndex = 0;
                surfIndex = group->startSurfIndex;
                while (triSurfIndex < group->surfaceCount && *surfCount < surfListSize)
                {
                    remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                    if (allowSurf(remappedSurfIndex, callbackContext))
                    {
                        surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                        if (!R_OutsideFrustumPlanes(planes, planeCount, surf->bounds[0]))
                            R_AddSurfaceToList(surf, surfList, surfCount);
                    }
                    ++triSurfIndex;
                    ++surfIndex;
                }
            }
        }
        R_AABBTreeCylinderSurfaces_r(
            cell->aabbTree,
            planes,
            planeCount,
            allowSurf,
            callbackContext,
            surfList,
            surfListSize,
            surfCount);
    }
}

int __cdecl R_OutsideFrustumPlanes(const DpvsPlane *planes, unsigned int planeCount, const float *minmax)
{
    unsigned int plane; // [esp+8h] [ebp-4h]

    for (plane = 0; plane < planeCount; ++plane)
    {
        if (*(const float *)((char *)minmax + planes[plane].side[0]) * planes[plane].coeffs[0]
            + planes[plane].coeffs[3]
            + *(const float *)((char *)minmax + planes[plane].side[1]) * planes[plane].coeffs[1]
            + *(const float *)((char *)minmax + planes[plane].side[2]) * planes[plane].coeffs[2] <= 0.0)
            return 1;
    }
    return 0;
}

void __cdecl R_AABBTreeCylinderSurfaces_r(
    GfxAabbTree *tree,
    const DpvsPlane *planes,
    unsigned int planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    unsigned int surfListSize,
    unsigned int *surfCount)
{
    unsigned int listIndex; // [esp+10h] [ebp-1Ch]
    GfxAabbTree *children; // [esp+14h] [ebp-18h]
    unsigned int surfIndex; // [esp+18h] [ebp-14h]
    unsigned int childIndex; // [esp+1Ch] [ebp-10h]
    unsigned int remappedSurfIndex; // [esp+20h] [ebp-Ch]
    GfxSurface *surf; // [esp+24h] [ebp-8h]
    unsigned int surfNodeIndex; // [esp+28h] [ebp-4h]

    if (!R_OutsideFrustumPlanes(planes, planeCount, tree->mins))
    {
        if (tree->childCount)
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
                R_AABBTreeCylinderSurfaces_r(
                    &children[childIndex],
                    planes,
                    planeCount,
                    allowSurf,
                    callbackContext,
                    surfList,
                    surfListSize,
                    surfCount);
        }
        else
        {
            surfNodeIndex = 0;
            surfIndex = tree->startSurfIndex;
            while (surfNodeIndex < tree->surfaceCount && *surfCount < surfListSize)
            {
                remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                if (allowSurf(remappedSurfIndex, callbackContext))
                {
                    surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                    if (!R_OutsideFrustumPlanes(planes, planeCount, surf->bounds[0]))
                    {
                        for (listIndex = 0; listIndex < *surfCount; ++listIndex)
                        {
                            if (surfList[listIndex] == surf)
                                MyAssertHandler(".\\r_marks.cpp", 507, 0, "%s", "surfList[listIndex] != surf");
                        }
                        surfList[(*surfCount)++] = surf;
                    }
                }
                ++surfNodeIndex;
                ++surfIndex;
            }
        }
    }
}

void __cdecl R_MarkUtil_GetDObjAnimMatAndHideParts(
    const DObj_s *dobj,
    const cpose_t *pose,
    const DObjAnimMat **outBoneMtxList,
    unsigned int *outHidePartBits)
{
    char zeroLods[32]; // [esp+30h] [ebp-38h] BYREF
    int partBits[4]; // [esp+58h] [ebp-10h] BYREF

    //Profile_Begin(217);
    memset(zeroLods, 0, sizeof(zeroLods));
    if (!DObjGetSurfaces(dobj, partBits, zeroLods))
        MyAssertHandler(".\\r_marks.cpp", 1715, 0, "%s", "surfaceCount");
    DObjLock((DObj_s*)dobj);
    *outBoneMtxList = CG_DObjCalcPose(pose, dobj, partBits);
    DObjUnlock((DObj_s*)dobj);
    if (!DObjSkelAreBonesUpToDate(dobj, partBits))
        MyAssertHandler(".\\r_marks.cpp", 1726, 0, "%s", "DObjSkelAreBonesUpToDate( dobj, partBits )");
    if (!*outBoneMtxList)
        MyAssertHandler(".\\r_marks.cpp", 1728, 0, "%s", "*outBoneMtxList");
    DObjGetHidePartBits(dobj, outHidePartBits);
    //Profile_EndInternal(0);
}

void __cdecl R_MarkFragments_Begin(
    MarkInfo *markInfo,
    MarkFragmentsAgainstEnum markAgainst,
    const float *origin,
    const float (*axis)[3],
    float radius,
    const float *viewOffset,
    Material *material)
{
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    markInfo->origin[0] = *origin;
    markInfo->origin[1] = origin[1];
    markInfo->origin[2] = origin[2];
    AxisCopy(axis, markInfo->axis);
    markInfo->viewOffset[0] = *viewOffset;
    markInfo->viewOffset[1] = viewOffset[1];
    markInfo->viewOffset[2] = viewOffset[2];
    markInfo->radius = radius;
    markInfo->material = material;
    markInfo->markHasLightmap = (material->info.gameFlags & 2) != 0;
    markInfo->markHasReflection = (material->info.gameFlags & 0x10) != 0;
    markInfo->markAgainst = markAgainst;
    R_GetMarkFragmentBounds(markInfo->origin, markInfo->axis, markInfo->radius, markInfo->mins, markInfo->maxs);
    R_GetMarkFragmentClipPlanes(markInfo->origin, markInfo->axis, markInfo->radius, markInfo->planes);
    if (markAgainst)
    {
        if (markAgainst != MARK_FRAGMENTS_AGAINST_MODELS)
            MyAssertHandler(".\\r_marks.cpp", 1867, 0, "%s", "markAgainst == MARK_FRAGMENTS_AGAINST_MODELS");
        markInfo->smodelCollidedCount = R_BoxStaticModels(
            markInfo->mins,
            markInfo->maxs,
            (int(__cdecl *)(int))CL_GetLocalClientActiveCount,
            markInfo->smodelsCollided,
            32);
        markInfo->sceneDObjCollidedCount = 0;
    }
    else
    {
        markInfo->sceneBModelCollidedCount = 0;
    }
}

void __cdecl R_GetMarkFragmentBounds(
    const float *origin,
    const float (*axis)[3],
    float radius,
    float *mins,
    float *maxs)
{
    float v5; // [esp+0h] [ebp-28h]
    float v6; // [esp+4h] [ebp-24h]
    float v7; // [esp+8h] [ebp-20h]
    int coord; // [esp+20h] [ebp-8h]
    float offset; // [esp+24h] [ebp-4h]

    for (coord = 0; coord < 3; ++coord)
    {
        v7 = fabs((*axis)[coord]);
        v6 = fabs((*axis)[coord + 3]);
        v5 = fabs((*axis)[coord + 6]);
        offset = (v7 + v6 + v5) * radius;
        mins[coord] = origin[coord] - offset;
        maxs[coord] = origin[coord] + offset;
    }
}

void __cdecl R_GetMarkFragmentClipPlanes(const float *origin, const float (*axis)[3], float radius, float (*planes)[4])
{
    float *v4; // [esp+0h] [ebp-18h]
    float *v5; // [esp+8h] [ebp-10h]
    float *v6; // [esp+Ch] [ebp-Ch]
    int axisIndex; // [esp+10h] [ebp-8h]
    int planeIndex; // [esp+14h] [ebp-4h]
    int planeIndexa; // [esp+14h] [ebp-4h]

    planeIndex = 0;
    for (axisIndex = 0; axisIndex < 3; ++axisIndex)
    {
        v5 = &(*planes)[4 * planeIndex];
        v6 = (float *)&(*axis)[3 * axisIndex];
        *v5 = *v6;
        v5[1] = v6[1];
        v5[2] = v6[2];
        v5[3] = Vec3Dot(v5, origin) - radius;
        planeIndexa = planeIndex + 1;
        v4 = &(*planes)[4 * planeIndexa];
        *v4 = -*v6;
        v4[1] = -v6[1];
        v4[2] = -v6[2];
        v4[3] = Vec3Dot(v4, origin) - radius;
        planeIndex = planeIndexa + 1;
    }
}

char __cdecl R_MarkFragments_AddDObj(MarkInfo *markInfo, DObj_s *dObj, cpose_t *pose, unsigned __int16 entityIndex)
{
    MarkInfoCollidedDObj *collidedDObj; // [esp+0h] [ebp-4h]

    if (markInfo->sceneDObjCollidedCount == 32)
        return 0;
    collidedDObj = &markInfo->sceneDObjsCollided[markInfo->sceneDObjCollidedCount];
    collidedDObj->dObj = dObj;
    collidedDObj->pose = pose;
    collidedDObj->entnum = entityIndex;
    ++markInfo->sceneDObjCollidedCount;
    return 1;
}

char __cdecl R_MarkFragments_AddBModel(
    MarkInfo *markInfo,
    GfxBrushModel *brushModel,
    cpose_t *pose,
    unsigned __int16 entityIndex)
{
    MarkInfoCollidedBModel *collidedBModel; // [esp+0h] [ebp-4h]

    if (markInfo->sceneBModelCollidedCount == 32)
        return 0;
    collidedBModel = &markInfo->sceneBModelsCollided[markInfo->sceneBModelCollidedCount];
    collidedBModel->brushModel = brushModel;
    collidedBModel->pose = pose;
    collidedBModel->entnum = entityIndex;
    ++markInfo->sceneBModelCollidedCount;
    return 1;
}

void __cdecl R_MarkFragments_Go(
    MarkInfo *markInfo,
    void(__cdecl *callback)(void *, int, FxMarkTri *, int, FxMarkPoint *, const float *, const float *),
    void *callbackContext,
    int maxTris,
    FxMarkTri *tris,
    int maxPoints,
    FxMarkPoint *points)
{
    bool v7; // [esp+B7h] [ebp-419h]
    bool error; // [esp+4CFh] [ebp-1h]

    if (!markInfo->material)
        MyAssertHandler(".\\r_marks.cpp", 1910, 0, "%s", "markInfo->material");
    markInfo->maxTris = maxTris;
    markInfo->tris = tris;
    markInfo->maxPoints = maxPoints;
    markInfo->points = points;
    markInfo->usedTriCount = 0;
    markInfo->usedPointCount = 0;
    markInfo->callback = callback;
    markInfo->callbackContext = callbackContext;
    error = 0;
    if (markInfo->markAgainst)
    {
        if (markInfo->markAgainst != MARK_FRAGMENTS_AGAINST_MODELS)
            MyAssertHandler(".\\r_marks.cpp", 1930, 0, "%s", "markInfo->markAgainst == MARK_FRAGMENTS_AGAINST_MODELS");
        if (!R_MarkFragments_Models(markInfo))
            error = 1;
    }
    else
    {
        if (R_MarkFragments_WorldBrushes(markInfo))
            v7 = R_MarkFragments_EntBrushes(markInfo) != 0;
        else
            v7 = 0;
        if (!v7)
            error = 1;
    }
    if (error)
        Com_Printf(
            14,
            "R_MarkFragments: Too many triangles to mark, max %d radius %.2f at %.2f %.2f %.2f\n",
            markInfo->maxTris,
            markInfo->radius,
            markInfo->origin[0],
            markInfo->origin[1],
            markInfo->origin[2]);
}

int(__cdecl *allowSurf[1])(int, void *) = { (int(*)(int, void*))R_AllowMarks };
char __cdecl R_MarkFragments_WorldBrushes(MarkInfo *markInfo)
{
    GfxSurface **surfacesArray[1]; // [esp+298h] [ebp-41Ch] BYREF
    unsigned int surfIndex; // [esp+29Ch] [ebp-418h]
    unsigned int surfCount; // [esp+2A0h] [ebp-414h] BYREF
    GfxSurface *surfaces[256]; // [esp+2A4h] [ebp-410h] BYREF
    GfxMarkContext markContext; // [esp+6A8h] [ebp-Ch] BYREF
    bool anyMarks; // [esp+6B3h] [ebp-1h] BYREF
    int savedregs; // [esp+6B4h] [ebp+0h] BYREF

    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1297, 0, "%s", "!markInfo->usedTriCount && !markInfo->usedPointCount");
    markContext.modelTypeAndSurf = 0;
    markContext.modelIndex = 0;
    surfacesArray[0] = surfaces;
    R_BoxSurfaces(
        markInfo->mins,
        markInfo->maxs,
        allowSurf,
        markInfo->material,
        surfacesArray,
        0x100u,
        &surfCount,
        1u);
    anyMarks = 0;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        if (!R_MarkFragments_BrushSurface(
            markInfo,
            &markContext,
            markInfo->planes,
            markInfo->axis[0],
            surfaces[surfIndex],
            &anyMarks))
            return 0;
    }
    if (anyMarks)
    {
        markInfo->callback(
            markInfo->callbackContext,
            markInfo->usedTriCount,
            markInfo->tris,
            markInfo->usedPointCount,
            markInfo->points,
            (const float *)markInfo,
            markInfo->axis[1]);
        markInfo->usedTriCount = 0;
        markInfo->usedPointCount = 0;
    }
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1319, 0, "%s", "!markInfo->usedTriCount && !markInfo->usedPointCount");
    return 1;
}

bool __cdecl R_AllowMarks(int surfIndex, const Material *markMaterialAsVoid)
{
    return R_Mark_MaterialAllowsMarks(rgp.world->dpvs.surfaces[surfIndex].material, markMaterialAsVoid);
}

bool __cdecl R_Mark_MaterialAllowsMarks(const Material *markReceiverMaterialHandle, const Material *markMaterialHandle)
{
    if ((markReceiverMaterialHandle->stateFlags & 4) != 0)
        return 0;
    if ((markReceiverMaterialHandle->info.gameFlags & 4) != 0)
        return 0;
    return (markReceiverMaterialHandle->info.surfaceTypeBits & markMaterialHandle->info.surfaceTypeBits) == markMaterialHandle->info.surfaceTypeBits;
}

char __cdecl R_MarkFragments_BrushSurface(
    MarkInfo *markInfo,
    GfxMarkContext *markContext,
    const float (*clipPlanes)[4],
    const float *markDir,
    const GfxSurface *surface,
    bool *anyMarks)
{
    PackedUnitVec v7; // [esp+8h] [ebp-2D0h] BYREF
    float v8; // [esp+Ch] [ebp-2CCh]
    float v9; // [esp+10h] [ebp-2C8h]
    PackedUnitVec v10; // [esp+14h] [ebp-2C4h]
    float *v11; // [esp+1Ch] [ebp-2BCh]
    PackedUnitVec v12; // [esp+20h] [ebp-2B8h]
    PackedUnitVec v13; // [esp+24h] [ebp-2B4h] BYREF
    float v14; // [esp+28h] [ebp-2B0h]
    float v15; // [esp+2Ch] [ebp-2ACh]
    PackedUnitVec v16; // [esp+30h] [ebp-2A8h]
    PackedUnitVec v17; // [esp+38h] [ebp-2A0h]
    PackedUnitVec out; // [esp+3Ch] [ebp-29Ch] BYREF
    float v19; // [esp+40h] [ebp-298h]
    float v20; // [esp+44h] [ebp-294h]
    PackedUnitVec in; // [esp+48h] [ebp-290h]
    float *v22; // [esp+50h] [ebp-288h]
    float *v23; // [esp+54h] [ebp-284h]
    float *v24; // [esp+58h] [ebp-280h]
    float *v25; // [esp+5Ch] [ebp-27Ch]
    float *v26; // [esp+60h] [ebp-278h]
    const unsigned __int8 *triVerts1; // [esp+A0h] [ebp-238h]
    FxMarkPoint *points; // [esp+A4h] [ebp-234h]
    FxWorldMarkPoint clipPoints[2][9]; // [esp+A8h] [ebp-230h] BYREF
    int baseIndex; // [esp+25Ch] [ebp-7Ch]
    FxMarkTri *tris; // [esp+260h] [ebp-78h]
    float normal[3][3]; // [esp+264h] [ebp-74h] BYREF
    FxWorldMarkPoint *clipPoint; // [esp+288h] [ebp-50h]
    int pointIndex; // [esp+28Ch] [ebp-4Ch]
    const unsigned __int16 *indices; // [esp+290h] [ebp-48h]
    unsigned int triVerts1Stride; // [esp+294h] [ebp-44h]
    int fragmentPointCount; // [esp+298h] [ebp-40h]
    const GfxWorldVertex *triVert1[3]; // [esp+29Ch] [ebp-3Ch]
    const GfxWorldVertex *triVerts0; // [esp+2A8h] [ebp-30h]
    int triIndex; // [esp+2ACh] [ebp-2Ch]
    int pingPong; // [esp+2B0h] [ebp-28h]
    const srfTriangles_t *triSurf; // [esp+2B4h] [ebp-24h]
    float lmapCoord[3][2]; // [esp+2B8h] [ebp-20h] BYREF
    int planeIndex; // [esp+2D0h] [ebp-8h]
    FxMarkPoint *point; // [esp+2D4h] [ebp-4h]

    markContext->lmapIndex = surface->lightmapIndex;
    if (markInfo->markHasLightmap != (markContext->lmapIndex != 31))
        return 1;
    markContext->reflectionProbeIndex = surface->reflectionProbeIndex;
    if (markInfo->markHasReflection != (markContext->reflectionProbeIndex != 0))
        return 1;
    markContext->primaryLightIndex = surface->primaryLightIndex;
    triSurf = &surface->tris;
    triVerts0 = &rgp.world->vd.vertices[surface->tris.firstVertex];
    indices = &rgp.world->indices[surface->tris.baseIndex];
    triVerts1 = (const unsigned __int8 *)triVerts0;
    triVerts1Stride = 44;
    triIndex = 0;
    while (triIndex < triSurf->triCount)
    {
        if (!R_MarkFragment_IsTriangleRejected(
            markDir,
            triVerts0[*indices].xyz,
            triVerts0[indices[1]].xyz,
            triVerts0[indices[2]].xyz))
        {
            R_MarkFragment_SetupWorldClipPoints(triVerts0, indices, clipPoints);
            pingPong = 0;
            fragmentPointCount = 3;
            for (planeIndex = 0; planeIndex < 6; ++planeIndex)
            {
                fragmentPointCount = R_ChopWorldPolyBehindPlane(
                    fragmentPointCount,
                    clipPoints[pingPong],
                    clipPoints[pingPong == 0],
                    &(*clipPlanes)[4 * planeIndex]);
                if (!fragmentPointCount)
                    goto LABEL_6;
                if (fragmentPointCount > 9)
                    MyAssertHandler(
                        ".\\r_marks.cpp",
                        1227,
                        0,
                        "%s\n\t(fragmentPointCount) = %i",
                        "(fragmentPointCount <= 3 + 6)",
                        fragmentPointCount);
                pingPong ^= 1u;
            }
            if (fragmentPointCount > markInfo->maxPoints - markInfo->usedPointCount
                || 3 * (fragmentPointCount - 2) > markInfo->maxTris - markInfo->usedTriCount)
            {
                return 0;
            }
            tris = &markInfo->tris[markInfo->usedTriCount];
            baseIndex = markInfo->usedPointCount;
            for (pointIndex = 2; pointIndex < fragmentPointCount; ++pointIndex)
            {
                tris->indices[0] = baseIndex + pointIndex - 1;
                tris->indices[1] = pointIndex + baseIndex;
                tris->indices[2] = baseIndex;
                tris->context = *markContext;
                ++tris;
            }
            triVert1[0] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * *indices];
            triVert1[1] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[1]];
            triVert1[2] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[2]];
            v26 = (float*)triVert1[0]->lmapCoord;
            lmapCoord[0][0] = triVert1[0]->lmapCoord[0];
            lmapCoord[0][1] = triVert1[0]->lmapCoord[1];
            v24 = lmapCoord[1];
            v25 = (float*)triVert1[1]->lmapCoord;
            *(_QWORD *)&lmapCoord[1][0] = *(_QWORD *)triVert1[1]->lmapCoord;
            v22 = lmapCoord[2];
            v23 = (float*)triVert1[2]->lmapCoord;
            *(_QWORD *)&lmapCoord[2][0] = *(_QWORD *)triVert1[2]->lmapCoord;
            v17.packed = triVert1[0]->normal.packed;
            out.packed = v17.packed;
            v19 = *(float *)&v17.packed;
            v20 = *(float *)&v17.packed;
            in.packed = v17.packed;
            Vec3UnpackUnitVec(v17, (float *)&out.packed);
            *(float *)&in.packed = 0.0;
            normal[0][0] = *(float *)&out.packed;
            normal[0][1] = v19;
            normal[0][2] = v20;
            v11 = normal[1];
            v12.packed = (unsigned int)triVert1[1]->normal.packed;
            v13.packed = v12.packed;
            v14 = *(float *)&v12.packed;
            v15 = *(float *)&v12.packed;
            v16.packed = v12.packed;
            Vec3UnpackUnitVec(v12, (float *)&v13.packed);
            *(float *)&v16.packed = 0.0;
            *v11 = *(float *)&v13.packed;
            v11[1] = v14;
            v11[2] = v15;
            v7.packed = (unsigned int)triVert1[2]->normal.packed;
            v8 = *(float *)&v7.packed;
            v9 = *(float *)&v7.packed;
            v10.packed = v7.packed;
            Vec3UnpackUnitVec(v7, (float *)&v7.packed);
            *(float *)&v10.packed = 0.0;
            normal[2][0] = *(float *)&v7.packed;
            normal[2][1] = v8;
            normal[2][2] = v9;
            points = &markInfo->points[markInfo->usedPointCount];
            for (pointIndex = 0; pointIndex < fragmentPointCount; ++pointIndex)
            {
                point = &points[pointIndex];
                clipPoint = &clipPoints[pingPong][pointIndex];
                point->xyz[0] = clipPoint->xyz[0];
                point->xyz[1] = clipPoint->xyz[1];
                point->xyz[2] = clipPoint->xyz[2];
                point->lmapCoord[0] = clipPoint->vertWeights[0] * lmapCoord[0][0]
                    + clipPoint->vertWeights[1] * lmapCoord[1][0]
                    + clipPoint->vertWeights[2] * lmapCoord[2][0];
                point->lmapCoord[1] = clipPoint->vertWeights[0] * lmapCoord[0][1]
                    + clipPoint->vertWeights[1] * lmapCoord[1][1]
                    + clipPoint->vertWeights[2] * lmapCoord[2][1];
                point->normal[0] = clipPoint->vertWeights[0] * normal[0][0]
                    + clipPoint->vertWeights[1] * normal[1][0]
                    + clipPoint->vertWeights[2] * normal[2][0];
                point->normal[1] = clipPoint->vertWeights[0] * normal[0][1]
                    + clipPoint->vertWeights[1] * normal[1][1]
                    + clipPoint->vertWeights[2] * normal[2][1];
                point->normal[2] = clipPoint->vertWeights[0] * normal[0][2]
                    + clipPoint->vertWeights[1] * normal[1][2]
                    + clipPoint->vertWeights[2] * normal[2][2];
            }
            if (fragmentPointCount < 3)
                MyAssertHandler(
                    ".\\r_marks.cpp",
                    1277,
                    0,
                    "%s\n\t(fragmentPointCount) = %i",
                    "(fragmentPointCount >= 3)",
                    fragmentPointCount);
            markInfo->usedPointCount += fragmentPointCount;
            markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
            *anyMarks = 1;
        }
    LABEL_6:
        ++triIndex;
        indices += 3;
    }
    return 1;
}

int __cdecl R_ChopWorldPolyBehindPlane(
    int inPointCount,
    FxWorldMarkPoint *inPoints,
    FxWorldMarkPoint *outPoints,
    const float *plane)
{
    double v4; // st7
    const FxWorldMarkPoint *v6; // eax
    FxWorldMarkPoint *v7; // ecx
    const FxWorldMarkPoint *v8; // ecx
    FxWorldMarkPoint *v9; // edx
    int sideCount[3]; // [esp+38h] [ebp-6Ch] BYREF
    float dists[10]; // [esp+44h] [ebp-60h]
    int nextIndex; // [esp+6Ch] [ebp-38h]
    float lerp; // [esp+70h] [ebp-34h]
    int pointIndex; // [esp+74h] [ebp-30h]
    int sides[10]; // [esp+78h] [ebp-2Ch]
    int outPointCount; // [esp+A0h] [ebp-4h]

    if (inPointCount > 9)
        MyAssertHandler(".\\r_marks.cpp", 250, 0, "%s\n\t(inPointCount) = %i", "(inPointCount <= 3 + 6)", inPointCount);
    memset(sideCount, 0, sizeof(sideCount));
    for (pointIndex = 0; pointIndex < inPointCount; ++pointIndex)
    {
        v4 = Vec3Dot(inPoints[pointIndex].xyz, plane);
        dists[pointIndex] = v4 - plane[3];
        if (dists[pointIndex] <= 0.5)
        {
            if (dists[pointIndex] >= -0.5)
                sides[pointIndex] = 2;
            else
                sides[pointIndex] = 1;
        }
        else
        {
            sides[pointIndex] = 0;
        }
        ++sideCount[sides[pointIndex]];
    }
    sides[pointIndex] = sides[0];
    dists[pointIndex] = dists[0];
    if (!sideCount[0])
        return 0;
    if (sideCount[1])
    {
        outPointCount = 0;
        for (pointIndex = 0; pointIndex < inPointCount; ++pointIndex)
        {
            if (sides[pointIndex] == 2)
            {
                if (outPointCount >= 9)
                    MyAssertHandler(
                        ".\\r_marks.cpp",
                        291,
                        0,
                        "%s\n\t(outPointCount) = %i",
                        "(outPointCount < 3 + 6)",
                        outPointCount);
                v6 = &inPoints[pointIndex];
                v7 = &outPoints[outPointCount];
                v7->xyz[0] = v6->xyz[0];
                v7->xyz[1] = v6->xyz[1];
                v7->xyz[2] = v6->xyz[2];
                v7->vertWeights[0] = v6->vertWeights[0];
                v7->vertWeights[1] = v6->vertWeights[1];
                v7->vertWeights[2] = v6->vertWeights[2];
                ++outPointCount;
            }
            else
            {
                if (!sides[pointIndex])
                {
                    if (outPointCount >= 9)
                        MyAssertHandler(
                            ".\\r_marks.cpp",
                            299,
                            0,
                            "%s\n\t(outPointCount) = %i",
                            "(outPointCount < 3 + 6)",
                            outPointCount);
                    v8 = &inPoints[pointIndex];
                    v9 = &outPoints[outPointCount];
                    v9->xyz[0] = v8->xyz[0];
                    v9->xyz[1] = v8->xyz[1];
                    v9->xyz[2] = v8->xyz[2];
                    v9->vertWeights[0] = v8->vertWeights[0];
                    v9->vertWeights[1] = v8->vertWeights[1];
                    v9->vertWeights[2] = v8->vertWeights[2];
                    ++outPointCount;
                }
                if (sides[pointIndex + 1] != 2 && sides[pointIndex + 1] != sides[pointIndex])
                {
                    if (outPointCount >= 9)
                        MyAssertHandler(
                            ".\\r_marks.cpp",
                            308,
                            0,
                            "%s\n\t(outPointCount) = %i",
                            "(outPointCount < 3 + 6)",
                            outPointCount);
                    if (dists[pointIndex + 1] == dists[pointIndex])
                        MyAssertHandler(
                            ".\\r_marks.cpp",
                            309,
                            0,
                            "%s\n\t(dists[pointIndex]) = %g",
                            "(dists[pointIndex] != dists[pointIndex + 1])",
                            dists[pointIndex]);
                    lerp = dists[pointIndex] / (dists[pointIndex] - dists[pointIndex + 1]);
                    nextIndex = (pointIndex + 1) % inPointCount;
                    R_LerpModelMarkPoints(
                        (const FxModelMarkPoint *)&inPoints[pointIndex],
                        (const FxModelMarkPoint *)&inPoints[nextIndex],
                        lerp,
                        (FxModelMarkPoint *)&outPoints[outPointCount++]);
                }
            }
        }
        return outPointCount;
    }
    else
    {
        //Profile_Begin(166);
        memcpy((unsigned __int8 *)outPoints, (unsigned __int8 *)inPoints, 24 * inPointCount);
        //Profile_EndInternal(0);
        return inPointCount;
    }
}

bool __cdecl R_MarkFragment_IsTriangleRejected(
    const float *markNormal,
    const float *xyz0,
    const float *xyz1,
    const float *xyz2)
{
    double v5; // [esp+0h] [ebp-30h]
    float edge01[3]; // [esp+8h] [ebp-28h] BYREF
    float scaledDot; // [esp+14h] [ebp-1Ch]
    float scaledNormal[3]; // [esp+18h] [ebp-18h] BYREF
    float edge21[3]; // [esp+24h] [ebp-Ch] BYREF

    Vec3Sub(xyz0, xyz1, edge01);
    Vec3Sub(xyz2, xyz1, edge21);
    Vec3Cross(edge01, edge21, scaledNormal);
    scaledDot = Vec3Dot(scaledNormal, markNormal);
    if (scaledDot < 0.0)
        return 1;
    v5 = scaledDot * scaledDot;
    return Vec3LengthSq(scaledNormal) * 0.25 > v5;
}

void __cdecl R_MarkFragment_SetupWorldClipPoints(
    const GfxWorldVertex *triVerts0,
    const unsigned __int16 *indices,
    FxWorldMarkPoint(*clipPoints)[9])
{
    FxWorldMarkPoint *v3; // [esp+4h] [ebp-10h]
    const GfxWorldVertex *v4; // [esp+8h] [ebp-Ch]
    unsigned int pointIndex; // [esp+Ch] [ebp-8h]

    for (pointIndex = 0; pointIndex < 3; ++pointIndex)
    {
        v3 = &(*clipPoints)[pointIndex];
        v4 = &triVerts0[indices[pointIndex]];
        v3->xyz[0] = v4->xyz[0];
        v3->xyz[1] = v4->xyz[1];
        v3->xyz[2] = v4->xyz[2];
        v3->vertWeights[0] = 0.0;
        v3->vertWeights[1] = 0.0;
        v3->vertWeights[2] = 0.0;
        v3->vertWeights[pointIndex] = 1.0;
    }
}

char __cdecl R_MarkFragments_EntBrushes(MarkInfo *markInfo)
{
    float invPoseMatrix[4][3]; // [esp+10h] [ebp-128h] BYREF
    float negatedPoseOrigin[3]; // [esp+40h] [ebp-F8h] BYREF
    float transformedTexCoordAxis[3]; // [esp+4Ch] [ebp-ECh] BYREF
    int surfaceBegin; // [esp+58h] [ebp-E0h]
    int surfaceIndex; // [esp+5Ch] [ebp-DCh]
    int surfaceEnd; // [esp+60h] [ebp-D8h]
    int brushModelCollidedIndex; // [esp+64h] [ebp-D4h]
    float clipPlanes[6][4]; // [esp+68h] [ebp-D0h] BYREF
    int brushModelCollidedCount; // [esp+C8h] [ebp-70h]
    MarkInfoCollidedBModel *brushModelCollided; // [esp+CCh] [ebp-6Ch]
    float poseMatrix[4][3]; // [esp+D0h] [ebp-68h] BYREF
    GfxBrushModel *brushModel; // [esp+100h] [ebp-38h]
    GfxMarkContext markContext; // [esp+104h] [ebp-34h] BYREF
    const GfxSurface *surface; // [esp+10Ch] [ebp-2Ch]
    float transformedOrigin[3]; // [esp+110h] [ebp-28h] BYREF
    bool anyMarks; // [esp+11Fh] [ebp-19h] BYREF
    float markDir[3]; // [esp+120h] [ebp-18h] BYREF
    float poseAngles[3]; // [esp+12Ch] [ebp-Ch] BYREF

    brushModelCollidedCount = markInfo->sceneBModelCollidedCount;
    for (brushModelCollidedIndex = 0; brushModelCollidedIndex != brushModelCollidedCount; ++brushModelCollidedIndex)
    {
        brushModelCollided = &markInfo->sceneBModelsCollided[brushModelCollidedIndex];
        markContext.modelTypeAndSurf = 0x80;
        markContext.modelIndex = markInfo->sceneBModelsCollided[brushModelCollidedIndex].entnum;
        CG_GetPoseAngles(markInfo->sceneBModelsCollided[brushModelCollidedIndex].pose, poseAngles);
        AnglesToAxis(poseAngles, poseMatrix);
        CG_GetPoseOrigin(brushModelCollided->pose, poseMatrix[3]);
        R_Mark_TransformClipPlanes(markInfo->planes, poseMatrix, clipPlanes);
        MatrixTransposeTransformVector(markInfo->axis[0], poseMatrix, markDir);
        anyMarks = 0;
        brushModel = brushModelCollided->brushModel;
        surfaceBegin = brushModel->startSurfIndex;
        surfaceEnd = brushModel->surfaceCount + brushModel->startSurfIndex;
        for (surfaceIndex = surfaceBegin; surfaceIndex != surfaceEnd; ++surfaceIndex)
        {
            surface = &rgp.world->dpvs.surfaces[surfaceIndex];
            if (R_Mark_MaterialAllowsMarks(surface->material, markInfo->material)
                && !R_MarkFragments_BrushSurface(markInfo, &markContext, clipPlanes, markDir, surface, &anyMarks))
            {
                return 0;
            }
        }
        if (anyMarks)
        {
            MatrixTranspose(poseMatrix, invPoseMatrix);
            negatedPoseOrigin[0] = -poseMatrix[3][0];
            negatedPoseOrigin[1] = -poseMatrix[3][1];
            negatedPoseOrigin[2] = -poseMatrix[3][2];
            MatrixTransformVector(negatedPoseOrigin, invPoseMatrix, invPoseMatrix[3]);
            MatrixTransformVector43(markInfo->origin, invPoseMatrix, transformedOrigin);
            MatrixTransposeTransformVector(markInfo->axis[1], poseMatrix, transformedTexCoordAxis);
            markInfo->callback(
                markInfo->callbackContext,
                markInfo->usedTriCount,
                markInfo->tris,
                markInfo->usedPointCount,
                markInfo->points,
                transformedOrigin,
                transformedTexCoordAxis);
            markInfo->usedTriCount = 0;
            markInfo->usedPointCount = 0;
        }
    }
    return 1;
}

void __cdecl R_Mark_TransformClipPlanes(const float (*inClipPlanes)[4], float (*matrix)[3], float (*outClipPlanes)[4])
{
    float v3; // [esp+4h] [ebp-8h]
    int planeIndex; // [esp+8h] [ebp-4h]

    for (planeIndex = 0; planeIndex != 6; ++planeIndex)
    {
        (*outClipPlanes)[4 * planeIndex] = Vec3Dot(&(*inClipPlanes)[4 * planeIndex], (const float *)matrix);
        (*outClipPlanes)[4 * planeIndex + 1] = Vec3Dot(&(*inClipPlanes)[4 * planeIndex], &(*matrix)[3]);
        (*outClipPlanes)[4 * planeIndex + 2] = Vec3Dot(&(*inClipPlanes)[4 * planeIndex], &(*matrix)[6]);
        v3 = (float)(*inClipPlanes)[4 * planeIndex + 3] - Vec3Dot(&(*inClipPlanes)[4 * planeIndex], &(*matrix)[9]);
        (*outClipPlanes)[4 * planeIndex + 3] = v3;
    }
}

char __cdecl R_MarkFragments_Models(MarkInfo *markInfo)
{
    if (markInfo->markHasLightmap)
        MyAssertHandler(".\\r_marks.cpp", 1835, 0, "%s", "!markInfo->markHasLightmap");
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1836, 0, "%s", "!( markInfo->usedTriCount || markInfo->usedPointCount )");
    if (!R_MarkFragments_StaticModels(markInfo))
        return 0;
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1839, 0, "%s", "!( markInfo->usedTriCount || markInfo->usedPointCount )");
    if (!R_MarkFragments_SceneDObjs(markInfo))
        return 0;
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1842, 0, "%s", "!( markInfo->usedTriCount || markInfo->usedPointCount )");
    return 1;
}

char __cdecl R_MarkFragments_SceneDObjs(MarkInfo *markInfo)
{
    int v1; // eax
    int submodelIndex; // [esp+6Ch] [ebp-3Ch]
    int boneIndex; // [esp+70h] [ebp-38h]
    DObj_s *dobj; // [esp+74h] [ebp-34h]
    const XModel *model; // [esp+7Ch] [ebp-2Ch]
    int submodelCount; // [esp+80h] [ebp-28h]
    unsigned int hidePartBits[4]; // [esp+84h] [ebp-24h] BYREF
    const DObjAnimMat *boneMtxList; // [esp+94h] [ebp-14h] BYREF
    GfxMarkContext markContext; // [esp+98h] [ebp-10h] BYREF
    unsigned __int16 entnum; // [esp+A0h] [ebp-8h]
    int sceneDObjCollidedIndex; // [esp+A4h] [ebp-4h]

    //Profile_Begin(216);
    for (sceneDObjCollidedIndex = 0; sceneDObjCollidedIndex != markInfo->sceneDObjCollidedCount; ++sceneDObjCollidedIndex)
    {
        dobj = markInfo->sceneDObjsCollided[sceneDObjCollidedIndex].dObj;
        entnum = markInfo->sceneDObjsCollided[sceneDObjCollidedIndex].entnum;
        R_MarkUtil_GetDObjAnimMatAndHideParts(
            dobj,
            markInfo->sceneDObjsCollided[sceneDObjCollidedIndex].pose,
            &boneMtxList,
            hidePartBits);
        submodelCount = DObjGetNumModels(dobj);
        boneIndex = 0;
        for (submodelIndex = 0; submodelIndex != submodelCount; ++submodelIndex)
        {
            markContext.reflectionProbeIndex = 0;
            markContext.primaryLightIndex = 0;
            markContext.modelIndex = entnum;
            markContext.modelTypeAndSurf = submodelIndex | 0xC0;
            if ((submodelIndex & 0x3F) != submodelIndex)
                MyAssertHandler(
                    ".\\r_marks.cpp",
                    1768,
                    0,
                    "%s\n\t(submodelIndex) = %i",
                    "((markContext.modelTypeAndSurf & MARK_MODEL_SURF_MASK ) == submodelIndex)",
                    submodelIndex);
            model = DObjGetModel(dobj, submodelIndex);
            v1 = DObjNumBones(dobj);
            if (!R_MarkFragments_AnimatedXModel(markInfo, model, hidePartBits, boneIndex, boneMtxList, v1, &markContext))
            {
                //Profile_EndInternal(0);
                return 0;
            }
            boneIndex += model->numBones;
            boneMtxList += model->numBones;
        }
    }
    //Profile_EndInternal(0);
    return 1;
}

char __cdecl R_MarkFragments_AnimatedXModel(
    MarkInfo* markInfo,
    const XModel* model,
    const unsigned int* hidePartBits,
    int boneIndex,
    const DObjAnimMat* boneMtxList,
    int boneCount,
    GfxMarkContext* markContext)
{
    int boneOffset; // [esp+528h] [ebp-30h]
    const DObjAnimMat* baseMatList; // [esp+53Ch] [ebp-1Ch]
    int surfIndex; // [esp+540h] [ebp-18h]
    unsigned int vertListIndex; // [esp+544h] [ebp-14h]
    int surfCount; // [esp+548h] [ebp-10h]
    Material** materials; // [esp+54Ch] [ebp-Ch]
    XSurface* surfaces; // [esp+550h] [ebp-8h] BYREF
    XSurface* surface; // [esp+554h] [ebp-4h]
    int savedregs; // [esp+558h] [ebp+0h] BYREF

    surfCount = XModelGetSurfaces(model, &surfaces, 0);
    baseMatList = XModelGetBasePose(model);
    materials = XModelGetSkins(model, 0);
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1639, 0, "%s", "!markInfo->usedTriCount && !markInfo->usedPointCount");
    //Profile_Begin(218);
    for (surfIndex = 0; surfIndex != surfCount; ++surfIndex)
    {
        if (R_Mark_MaterialAllowsMarks(materials[surfIndex], markInfo->material))
        {
            surface = &surfaces[surfIndex];
            if (!surface->deformed)
            {
                if (!surface->vertListCount)
                    MyAssertHandler(".\\r_marks.cpp", 1654, 0, "%s", "surface->vertListCount > 0");
                for (vertListIndex = 0; vertListIndex != surface->vertListCount; ++vertListIndex)
                {
                    boneOffset = surface->vertList[vertListIndex].boneOffset >> 6;
                    if (boneOffset + boneIndex >= boneCount)
                        MyAssertHandler(".\\r_marks.cpp", 1669, 0, "%s", "(boneIndex + boneOffset) < boneCount");
                    if ((hidePartBits[(boneOffset + boneIndex) >> 5] & (0x80000000 >> ((boneOffset + boneIndex) & 0x1F))) == 0)
                    {
                        markContext->lmapIndex = boneOffset;
                        if (markContext->lmapIndex != boneOffset)
                            MyAssertHandler(
                                ".\\r_marks.cpp",
                                1680,
                                0,
                                "%s\n\t(boneOffset) = %i",
                                "(markContext->lmapIndex == boneOffset)",
                                boneOffset);
                        if (!R_MarkFragments_AnimatedXModel_VertList(
                            markInfo,
                            vertListIndex,
                            &boneMtxList[boneOffset],
                            &baseMatList[boneOffset],
                            markContext,
                            surface))
                        {
                            //Profile_EndInternal(0);
                            return 0;
                        }
                        if (markInfo->usedTriCount || markInfo->usedPointCount)
                        {
                            if (markInfo->usedTriCount)
                            {
                                if (markInfo->usedPointCount)
                                    markInfo->callback(
                                        markInfo->callbackContext,
                                        markInfo->usedTriCount,
                                        markInfo->tris,
                                        markInfo->usedPointCount,
                                        markInfo->points,
                                        markInfo->localOrigin,
                                        markInfo->localTexCoordAxis);
                            }
                            markInfo->usedTriCount = 0;
                            markInfo->usedPointCount = 0;
                        }
                    }
                }
            }
        }
    }
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1700, 0, "%s", "!markInfo->usedTriCount && !markInfo->usedPointCount");
    //Profile_EndInternal(0);
    return 1;
}

bool __cdecl R_MarkModelCoreCallback_1_(
    MarkModelCoreContext *contextAsVoid,
    const GfxPackedVertex **triVerts0,
    const GfxPackedVertex **triVerts1)
{
    float *vertWeights; // [esp+2Ch] [ebp-1D8h]
    const GfxPackedVertex *v5; // [esp+38h] [ebp-1CCh]
    float pos[3]; // [esp+3Ch] [ebp-1C8h] BYREF
    MarkModelCoreContext *context; // [esp+48h] [ebp-1BCh]
    FxModelMarkPoint clipPoints[2][9]; // [esp+4Ch] [ebp-1B8h] BYREF
    int vertIndex; // [esp+200h] [ebp-4h]

    context = contextAsVoid;
    for (vertIndex = 0; vertIndex != 3; ++vertIndex)
    {
        v5 = triVerts0[vertIndex];
        pos[0] = v5->xyz[0];
        pos[1] = v5->xyz[1];
        pos[2] = v5->xyz[2];
        MatrixTransformVector43(pos, context->transformMatrix, clipPoints[0][vertIndex].xyz);
        vertWeights = clipPoints[0][vertIndex].vertWeights;
        *vertWeights = 0.0;
        vertWeights[1] = 0.0;
        vertWeights[2] = 0.0;
        clipPoints[0][vertIndex].vertWeights[vertIndex] = 1.0;
    }
    return R_MarkFragment_IsTriangleRejected(
        context->markDir,
        clipPoints[0][0].xyz,
        clipPoints[0][1].xyz,
        clipPoints[0][2].xyz)
        || R_MarkFragment_DoTriangle_1_(
            context->markInfo,
            context->clipPlanes,
            context->markContext,
            triVerts1,
            context->transformNormalMatrix,
            clipPoints) != 0;
}

bool __cdecl R_MarkModelCoreCallback_0_(
    void *contextAsVoid,
    const GfxPackedVertex **triVerts0,
    const GfxPackedVertex **triVerts1)
{
    float *vertWeights; // [esp+2Ch] [ebp-1D8h]
    float *xyz; // [esp+30h] [ebp-1D4h]
    const GfxPackedVertex *v6; // [esp+34h] [ebp-1D0h]
    FxModelMarkPoint clipPoints[2][9]; // [esp+4Ch] [ebp-1B8h] BYREF
    int vertIndex; // [esp+200h] [ebp-4h]

    for (vertIndex = 0; vertIndex != 3; ++vertIndex)
    {
        xyz = clipPoints[0][vertIndex].xyz;
        v6 = triVerts0[vertIndex];
        *xyz = v6->xyz[0];
        xyz[1] = v6->xyz[1];
        xyz[2] = v6->xyz[2];
        vertWeights = clipPoints[0][vertIndex].vertWeights;
        *vertWeights = 0.0;
        vertWeights[1] = 0.0;
        vertWeights[2] = 0.0;
        clipPoints[0][vertIndex].vertWeights[vertIndex] = 1.0;
    }
    return R_MarkFragment_IsTriangleRejected(
        *(contextAsVoid + 3),
        clipPoints[0][0].xyz,
        clipPoints[0][1].xyz,
        clipPoints[0][2].xyz)
        || R_MarkFragment_DoTriangle_0_(
            *contextAsVoid,
            *(contextAsVoid + 4),
            *(contextAsVoid + 1),
            triVerts1,
            *(contextAsVoid + 6),
            clipPoints) != 0;
}

char  R_MarkFragments_AnimatedXModel_VertList(
    MarkInfo *markInfo,
    unsigned int vertListIndex,
    const DObjAnimMat *poseBone,
    const DObjAnimMat *baseBone,
    GfxMarkContext *markContext,
    XSurface *surface)
{
    float scale; // [esp+0h] [ebp-534h]
    unsigned int v9[3]; // [esp+10h] [ebp-524h] BYREF
    MarkModelCoreContext markModelCoreContext; // [esp+1Ch] [ebp-518h] BYREF
    float localMaxs[3]; // [esp+38h] [ebp-4FCh] BYREF
    float localMins[3]; // [esp+44h] [ebp-4F0h] BYREF
    float originalOrigin[27]; // [esp+50h] [ebp-4E4h] BYREF
    float baseBoneMatrix[4][3]; // [esp+BCh] [ebp-478h] BYREF
    float v15; // [esp+ECh] [ebp-448h]
    float v16; // [esp+F0h] [ebp-444h]
    float v17; // [esp+F4h] [ebp-440h]
    float *v18; // [esp+F8h] [ebp-43Ch]
    float v19; // [esp+FCh] [ebp-438h]
    float v20; // [esp+100h] [ebp-434h]
    float v21; // [esp+104h] [ebp-430h]
    float *v22; // [esp+108h] [ebp-42Ch]
    float v23; // [esp+10Ch] [ebp-428h]
    float v24; // [esp+110h] [ebp-424h]
    float v25; // [esp+114h] [ebp-420h]
    float *v26; // [esp+118h] [ebp-41Ch]
    float v27[3]; // [esp+11Ch] [ebp-418h] BYREF
    float invPoseBoneMatrix[4][3]; // [esp+128h] [ebp-40Ch] BYREF
    float v29; // [esp+158h] [ebp-3DCh]
    float v30; // [esp+15Ch] [ebp-3D8h]
    float v31; // [esp+160h] [ebp-3D4h]
    DObjSkelMat invPoseBoneSkelMat; // [esp+164h] [ebp-3D0h]
    float v33; // [esp+1A4h] [ebp-390h]
    float v34; // [esp+1A8h] [ebp-38Ch]
    float v35; // [esp+1ACh] [ebp-388h]
    float v36; // [esp+1B0h] [ebp-384h]
    float v37; // [esp+1B4h] [ebp-380h]
    float v38; // [esp+1B8h] [ebp-37Ch]
    float v39; // [esp+1BCh] [ebp-378h]
    float v40; // [esp+1C0h] [ebp-374h]
    float v41; // [esp+1C4h] [ebp-370h]
    float v42; // [esp+1C8h] [ebp-36Ch] BYREF
    float v43; // [esp+1CCh] [ebp-368h]
    float v44; // [esp+1D0h] [ebp-364h]
    float v45; // [esp+1D4h] [ebp-360h]
    float v46; // [esp+1D8h] [ebp-35Ch]
    float v47; // [esp+1DCh] [ebp-358h]
    float v48; // [esp+1E0h] [ebp-354h]
    float v49; // [esp+1E4h] [ebp-350h]
    float v50; // [esp+1E8h] [ebp-34Ch]
    float v51; // [esp+1ECh] [ebp-348h]
    float v52; // [esp+1F0h] [ebp-344h]
    DObjSkelMat baseBoneSkelMat; // [esp+1F4h] [ebp-340h] BYREF
    float v54; // [esp+234h] [ebp-300h]
    float v55; // [esp+238h] [ebp-2FCh]
    float v56; // [esp+23Ch] [ebp-2F8h]
    float v57; // [esp+240h] [ebp-2F4h]
    float v58; // [esp+244h] [ebp-2F0h]
    float v59; // [esp+248h] [ebp-2ECh]
    float v60; // [esp+24Ch] [ebp-2E8h]
    float v61; // [esp+250h] [ebp-2E4h]
    float v62; // [esp+254h] [ebp-2E0h] BYREF
    float v63; // [esp+258h] [ebp-2DCh]
    float v64; // [esp+25Ch] [ebp-2D8h]
    float v65; // [esp+260h] [ebp-2D4h]
    float v66; // [esp+264h] [ebp-2D0h]
    float v67; // [esp+268h] [ebp-2CCh]
    float v68; // [esp+26Ch] [ebp-2C8h]
    float v69; // [esp+270h] [ebp-2C4h]
    float v70[3]; // [esp+274h] [ebp-2C0h] BYREF
    float markDir[24]; // [esp+280h] [ebp-2B4h] BYREF
    float clipPlanes_84[27]; // [esp+2E0h] [ebp-254h] BYREF
    float invBaseBoneMatrix[4][3]; // [esp+34Ch] [ebp-1E8h] BYREF
    float v74; // [esp+37Ch] [ebp-1B8h]
    float v75; // [esp+380h] [ebp-1B4h]
    float v76; // [esp+384h] [ebp-1B0h]
    float *v77; // [esp+388h] [ebp-1ACh]
    float v78; // [esp+38Ch] [ebp-1A8h]
    float v79; // [esp+390h] [ebp-1A4h]
    float v80; // [esp+394h] [ebp-1A0h]
    float *v81; // [esp+398h] [ebp-19Ch]
    float v82; // [esp+39Ch] [ebp-198h]
    float v83; // [esp+3A0h] [ebp-194h]
    float v84; // [esp+3A4h] [ebp-190h]
    float *v85; // [esp+3A8h] [ebp-18Ch]
    float v86[3]; // [esp+3ACh] [ebp-188h] BYREF
    float poseBoneMatrix[4][3]; // [esp+3B8h] [ebp-17Ch] BYREF
    float v88; // [esp+3E8h] [ebp-14Ch]
    float v89; // [esp+3ECh] [ebp-148h]
    float v90; // [esp+3F0h] [ebp-144h]
    DObjSkelMat invBaseBoneSkelMat; // [esp+3F4h] [ebp-140h]
    float v92; // [esp+434h] [ebp-100h]
    float v93; // [esp+438h] [ebp-FCh]
    float v94; // [esp+43Ch] [ebp-F8h]
    float v95; // [esp+440h] [ebp-F4h]
    float v96; // [esp+444h] [ebp-F0h]
    float v97; // [esp+448h] [ebp-ECh]
    float v98; // [esp+44Ch] [ebp-E8h]
    float v99; // [esp+450h] [ebp-E4h]
    float v100; // [esp+454h] [ebp-E0h]
    float v101; // [esp+458h] [ebp-DCh] BYREF
    float v102; // [esp+45Ch] [ebp-D8h]
    float v103; // [esp+460h] [ebp-D4h]
    float transWeight; // [esp+464h] [ebp-D0h]
    float v105; // [esp+468h] [ebp-CCh]
    float v106; // [esp+46Ch] [ebp-C8h]
    float v107; // [esp+470h] [ebp-C4h]
    float v108; // [esp+474h] [ebp-C0h]
    float v109; // [esp+478h] [ebp-BCh]
    float v110; // [esp+47Ch] [ebp-B8h]
    float v111; // [esp+480h] [ebp-B4h]
    DObjSkelMat poseBoneSkelMat; // [esp+484h] [ebp-B0h] BYREF
    float v113; // [esp+4C4h] [ebp-70h]
    float v114; // [esp+4C8h] [ebp-6Ch]
    float v115; // [esp+4CCh] [ebp-68h]
    float v116; // [esp+4D0h] [ebp-64h]
    float v117; // [esp+4D4h] [ebp-60h]
    float v118; // [esp+4D8h] [ebp-5Ch]
    float v119; // [esp+4DCh] [ebp-58h]
    float v120; // [esp+4E0h] [ebp-54h]
    float v121; // [esp+4E4h] [ebp-50h]
    float v122; // [esp+4E8h] [ebp-4Ch] BYREF
    float v123; // [esp+4ECh] [ebp-48h]
    float v124; // [esp+4F0h] [ebp-44h]
    float v125; // [esp+4F4h] [ebp-40h]
    float v126; // [esp+4F8h] [ebp-3Ch]
    float v127; // [esp+4FCh] [ebp-38h]
    float v128; // [esp+500h] [ebp-34h]
    float v129; // [esp+504h] [ebp-30h]
    float v130[11]; // [esp+508h] [ebp-2Ch] BYREF
    float retaddr; // [esp+534h] [ebp+0h]

    v130[8] = a1;
    v130[9] = retaddr;
    //Profile_Begin(219);
    memcpy(v130, poseBone, 0x20u);
    Vec3Add(&v130[4], markInfo->viewOffset, &v130[4]);
    v129 = v130[0];
    if ((LODWORD(v130[0]) & 0x7F800000) == 0x7F800000
        || (v128 = v130[1], (LODWORD(v130[1]) & 0x7F800000) == 0x7F800000)
        || (v127 = v130[2], (LODWORD(v130[2]) & 0x7F800000) == 0x7F800000)
        || (v126 = v130[3], (LODWORD(v130[3]) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            473,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    v125 = v130[7];
    if ((LODWORD(v130[7]) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            474,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(v130, v130[7], &v122);
    v121 = v122 * v130[0];
    v120 = v122 * v130[1];
    v119 = v122 * v130[2];
    v118 = v122 * v130[3];
    v117 = v123 * v130[1];
    v116 = v123 * v130[2];
    v115 = v123 * v130[3];
    v114 = v124 * v130[2];
    v113 = v124 * v130[3];
    v109 = 1.0 - (v117 + v114);
    v110 = v120 + v113;
    v111 = v119 - v115;
    poseBoneSkelMat.axis[0][0] = 0.0;
    poseBoneSkelMat.axis[0][1] = v120 - v113;
    poseBoneSkelMat.axis[0][2] = 1.0 - (v121 + v114);
    poseBoneSkelMat.axis[0][3] = v116 + v118;
    poseBoneSkelMat.axis[1][0] = 0.0;
    poseBoneSkelMat.axis[1][1] = v119 + v115;
    poseBoneSkelMat.axis[1][2] = v116 - v118;
    poseBoneSkelMat.axis[1][3] = 1.0 - (v121 + v117);
    poseBoneSkelMat.axis[2][0] = 0.0;
    Vec3Copy(&v130[4], &poseBoneSkelMat.axis[2][1]);
    poseBoneSkelMat.origin[0] = 1.0;
    v108 = baseBone->quat[0];
    if ((LODWORD(v108) & 0x7F800000) == 0x7F800000
        || (v107 = baseBone->quat[1], (LODWORD(v107) & 0x7F800000) == 0x7F800000)
        || (v106 = baseBone->quat[2], (LODWORD(v106) & 0x7F800000) == 0x7F800000)
        || (v105 = baseBone->quat[3], (LODWORD(v105) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            581,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    transWeight = baseBone->transWeight;
    if ((LODWORD(transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            582,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(baseBone->quat, baseBone->transWeight, &v101);
    v100 = v101 * baseBone->quat[0];
    v99 = v101 * baseBone->quat[1];
    v98 = v101 * baseBone->quat[2];
    v97 = v101 * baseBone->quat[3];
    v96 = v102 * baseBone->quat[1];
    v95 = v102 * baseBone->quat[2];
    v94 = v102 * baseBone->quat[3];
    v93 = v103 * baseBone->quat[2];
    v92 = v103 * baseBone->quat[3];
    v88 = 1.0 - (v96 + v93);
    v89 = v99 - v92;
    v90 = v98 + v94;
    invBaseBoneSkelMat.axis[0][0] = 0.0;
    invBaseBoneSkelMat.axis[0][1] = v99 + v92;
    invBaseBoneSkelMat.axis[0][2] = 1.0 - (v100 + v93);
    invBaseBoneSkelMat.axis[0][3] = v95 - v97;
    invBaseBoneSkelMat.axis[1][0] = 0.0;
    invBaseBoneSkelMat.axis[1][1] = v98 - v94;
    invBaseBoneSkelMat.axis[1][2] = v95 + v97;
    invBaseBoneSkelMat.axis[1][3] = 1.0 - (v100 + v96);
    invBaseBoneSkelMat.axis[2][0] = 0.0;
    invBaseBoneSkelMat.axis[2][1] = -(baseBone->trans[0] * v88
        + baseBone->trans[1] * invBaseBoneSkelMat.axis[0][1]
        + baseBone->trans[2] * invBaseBoneSkelMat.axis[1][1]);
    invBaseBoneSkelMat.axis[2][2] = -(baseBone->trans[0] * v89
        + baseBone->trans[1] * invBaseBoneSkelMat.axis[0][2]
        + baseBone->trans[2] * invBaseBoneSkelMat.axis[1][2]);
    invBaseBoneSkelMat.axis[2][3] = -(baseBone->trans[0] * v90
        + baseBone->trans[1] * invBaseBoneSkelMat.axis[0][3]
        + baseBone->trans[2] * invBaseBoneSkelMat.axis[1][3]);
    invBaseBoneSkelMat.origin[0] = 1.0;
    poseBoneMatrix[3][2] = v109;
    poseBoneMatrix[3][1] = v110;
    poseBoneMatrix[3][0] = v111;
    v86[0] = v109;
    v86[1] = v110;
    v86[2] = v111;
    v85 = poseBoneMatrix[0];
    v84 = poseBoneSkelMat.axis[0][1];
    v83 = poseBoneSkelMat.axis[0][2];
    v82 = poseBoneSkelMat.axis[0][3];
    poseBoneMatrix[0][0] = poseBoneSkelMat.axis[0][1];
    poseBoneMatrix[0][1] = poseBoneSkelMat.axis[0][2];
    poseBoneMatrix[0][2] = poseBoneSkelMat.axis[0][3];
    v81 = poseBoneMatrix[1];
    v80 = poseBoneSkelMat.axis[1][1];
    v79 = poseBoneSkelMat.axis[1][2];
    v78 = poseBoneSkelMat.axis[1][3];
    poseBoneMatrix[1][0] = poseBoneSkelMat.axis[1][1];
    poseBoneMatrix[1][1] = poseBoneSkelMat.axis[1][2];
    poseBoneMatrix[1][2] = poseBoneSkelMat.axis[1][3];
    v77 = poseBoneMatrix[2];
    v76 = poseBoneSkelMat.axis[2][1];
    v75 = poseBoneSkelMat.axis[2][2];
    v74 = poseBoneSkelMat.axis[2][3];
    poseBoneMatrix[2][0] = poseBoneSkelMat.axis[2][1];
    poseBoneMatrix[2][1] = poseBoneSkelMat.axis[2][2];
    poseBoneMatrix[2][2] = poseBoneSkelMat.axis[2][3];
    invBaseBoneMatrix[3][2] = v88;
    invBaseBoneMatrix[3][1] = v89;
    invBaseBoneMatrix[3][0] = v90;
    clipPlanes_84[24] = v88;
    clipPlanes_84[25] = v89;
    clipPlanes_84[26] = v90;
    LODWORD(clipPlanes_84[23]) = invBaseBoneMatrix;
    clipPlanes_84[22] = invBaseBoneSkelMat.axis[0][1];
    clipPlanes_84[21] = invBaseBoneSkelMat.axis[0][2];
    clipPlanes_84[20] = invBaseBoneSkelMat.axis[0][3];
    invBaseBoneMatrix[0][0] = invBaseBoneSkelMat.axis[0][1];
    invBaseBoneMatrix[0][1] = invBaseBoneSkelMat.axis[0][2];
    invBaseBoneMatrix[0][2] = invBaseBoneSkelMat.axis[0][3];
    LODWORD(clipPlanes_84[19]) = invBaseBoneMatrix[1];
    clipPlanes_84[18] = invBaseBoneSkelMat.axis[1][1];
    clipPlanes_84[17] = invBaseBoneSkelMat.axis[1][2];
    clipPlanes_84[16] = invBaseBoneSkelMat.axis[1][3];
    invBaseBoneMatrix[1][0] = invBaseBoneSkelMat.axis[1][1];
    invBaseBoneMatrix[1][1] = invBaseBoneSkelMat.axis[1][2];
    invBaseBoneMatrix[1][2] = invBaseBoneSkelMat.axis[1][3];
    LODWORD(clipPlanes_84[15]) = invBaseBoneMatrix[2];
    clipPlanes_84[14] = invBaseBoneSkelMat.axis[2][1];
    clipPlanes_84[13] = invBaseBoneSkelMat.axis[2][2];
    clipPlanes_84[12] = invBaseBoneSkelMat.axis[2][3];
    invBaseBoneMatrix[2][0] = invBaseBoneSkelMat.axis[2][1];
    invBaseBoneMatrix[2][1] = invBaseBoneSkelMat.axis[2][2];
    invBaseBoneMatrix[2][2] = invBaseBoneSkelMat.axis[2][3];
    MatrixMultiply43((const float (*)[3]) & clipPlanes_84[24], (const float (*)[3])v86, (float (*)[3])clipPlanes_84);
    if (markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1586, 0, "%s", "markInfo->usedPointCount == 0");
    R_Mark_TransformClipPlanes(markInfo->planes, (float (*)[3])clipPlanes_84, (float (*)[4])markDir);
    MatrixTransposeTransformVector(markInfo->axis[0], (const float (*)[3])clipPlanes_84, v70);
    v69 = baseBone->quat[0];
    if ((LODWORD(v69) & 0x7F800000) == 0x7F800000
        || (v68 = baseBone->quat[1], (LODWORD(v68) & 0x7F800000) == 0x7F800000)
        || (v67 = baseBone->quat[2], (LODWORD(v67) & 0x7F800000) == 0x7F800000)
        || (v66 = baseBone->quat[3], (LODWORD(v66) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            473,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    v65 = baseBone->transWeight;
    if ((LODWORD(v65) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            474,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(baseBone->quat, baseBone->transWeight, &v62);
    v61 = v62 * baseBone->quat[0];
    v60 = v62 * baseBone->quat[1];
    v59 = v62 * baseBone->quat[2];
    v58 = v62 * baseBone->quat[3];
    v57 = v63 * baseBone->quat[1];
    v56 = v63 * baseBone->quat[2];
    v55 = v63 * baseBone->quat[3];
    v54 = v64 * baseBone->quat[2];
    baseBoneSkelMat.origin[3] = v64 * baseBone->quat[3];
    v50 = 1.0 - (v57 + v54);
    v51 = v60 + baseBoneSkelMat.origin[3];
    v52 = v59 - v55;
    baseBoneSkelMat.axis[0][0] = 0.0;
    baseBoneSkelMat.axis[0][1] = v60 - baseBoneSkelMat.origin[3];
    baseBoneSkelMat.axis[0][2] = 1.0 - (v61 + v54);
    baseBoneSkelMat.axis[0][3] = v56 + v58;
    baseBoneSkelMat.axis[1][0] = 0.0;
    baseBoneSkelMat.axis[1][1] = v59 + v55;
    baseBoneSkelMat.axis[1][2] = v56 - v58;
    baseBoneSkelMat.axis[1][3] = 1.0 - (v61 + v57);
    baseBoneSkelMat.axis[2][0] = 0.0;
    Vec3Copy(baseBone->trans, &baseBoneSkelMat.axis[2][1]);
    baseBoneSkelMat.origin[0] = 1.0;
    v49 = v130[0];
    if ((LODWORD(v130[0]) & 0x7F800000) == 0x7F800000
        || (v48 = v130[1], (LODWORD(v130[1]) & 0x7F800000) == 0x7F800000)
        || (v47 = v130[2], (LODWORD(v130[2]) & 0x7F800000) == 0x7F800000)
        || (v46 = v130[3], (LODWORD(v130[3]) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            581,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    v45 = v130[7];
    if ((LODWORD(v130[7]) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            582,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(v130, v130[7], &v42);
    v41 = v42 * v130[0];
    v40 = v42 * v130[1];
    v39 = v42 * v130[2];
    v38 = v42 * v130[3];
    v37 = v43 * v130[1];
    v36 = v43 * v130[2];
    v35 = v43 * v130[3];
    v34 = v44 * v130[2];
    v33 = v44 * v130[3];
    v29 = 1.0 - (v37 + v34);
    v30 = v40 - v33;
    v31 = v39 + v35;
    invPoseBoneSkelMat.axis[0][0] = 0.0;
    invPoseBoneSkelMat.axis[0][1] = v40 + v33;
    invPoseBoneSkelMat.axis[0][2] = 1.0 - (v41 + v34);
    invPoseBoneSkelMat.axis[0][3] = v36 - v38;
    invPoseBoneSkelMat.axis[1][0] = 0.0;
    invPoseBoneSkelMat.axis[1][1] = v39 - v35;
    invPoseBoneSkelMat.axis[1][2] = v36 + v38;
    invPoseBoneSkelMat.axis[1][3] = 1.0 - (v41 + v37);
    invPoseBoneSkelMat.axis[2][0] = 0.0;
    invPoseBoneSkelMat.axis[2][1] = -(v130[4] * v29
        + v130[5] * invPoseBoneSkelMat.axis[0][1]
        + v130[6] * invPoseBoneSkelMat.axis[1][1]);
    invPoseBoneSkelMat.axis[2][2] = -(v130[4] * v30
        + v130[5] * invPoseBoneSkelMat.axis[0][2]
        + v130[6] * invPoseBoneSkelMat.axis[1][2]);
    invPoseBoneSkelMat.axis[2][3] = -(v130[4] * v31
        + v130[5] * invPoseBoneSkelMat.axis[0][3]
        + v130[6] * invPoseBoneSkelMat.axis[1][3]);
    invPoseBoneSkelMat.origin[0] = 1.0;
    invPoseBoneMatrix[3][2] = v29;
    invPoseBoneMatrix[3][1] = v30;
    invPoseBoneMatrix[3][0] = v31;
    v27[0] = v29;
    v27[1] = v30;
    v27[2] = v31;
    v26 = invPoseBoneMatrix[0];
    v25 = invPoseBoneSkelMat.axis[0][1];
    v24 = invPoseBoneSkelMat.axis[0][2];
    v23 = invPoseBoneSkelMat.axis[0][3];
    invPoseBoneMatrix[0][0] = invPoseBoneSkelMat.axis[0][1];
    invPoseBoneMatrix[0][1] = invPoseBoneSkelMat.axis[0][2];
    invPoseBoneMatrix[0][2] = invPoseBoneSkelMat.axis[0][3];
    v22 = invPoseBoneMatrix[1];
    v21 = invPoseBoneSkelMat.axis[1][1];
    v20 = invPoseBoneSkelMat.axis[1][2];
    v19 = invPoseBoneSkelMat.axis[1][3];
    invPoseBoneMatrix[1][0] = invPoseBoneSkelMat.axis[1][1];
    invPoseBoneMatrix[1][1] = invPoseBoneSkelMat.axis[1][2];
    invPoseBoneMatrix[1][2] = invPoseBoneSkelMat.axis[1][3];
    v18 = invPoseBoneMatrix[2];
    v17 = invPoseBoneSkelMat.axis[2][1];
    v16 = invPoseBoneSkelMat.axis[2][2];
    v15 = invPoseBoneSkelMat.axis[2][3];
    invPoseBoneMatrix[2][0] = invPoseBoneSkelMat.axis[2][1];
    invPoseBoneMatrix[2][1] = invPoseBoneSkelMat.axis[2][2];
    invPoseBoneMatrix[2][2] = invPoseBoneSkelMat.axis[2][3];
    baseBoneMatrix[3][2] = v50;
    baseBoneMatrix[3][1] = v51;
    baseBoneMatrix[3][0] = v52;
    originalOrigin[24] = v50;
    originalOrigin[25] = v51;
    originalOrigin[26] = v52;
    LODWORD(originalOrigin[23]) = baseBoneMatrix;
    originalOrigin[22] = baseBoneSkelMat.axis[0][1];
    originalOrigin[21] = baseBoneSkelMat.axis[0][2];
    originalOrigin[20] = baseBoneSkelMat.axis[0][3];
    baseBoneMatrix[0][0] = baseBoneSkelMat.axis[0][1];
    baseBoneMatrix[0][1] = baseBoneSkelMat.axis[0][2];
    baseBoneMatrix[0][2] = baseBoneSkelMat.axis[0][3];
    LODWORD(originalOrigin[19]) = baseBoneMatrix[1];
    originalOrigin[18] = baseBoneSkelMat.axis[1][1];
    originalOrigin[17] = baseBoneSkelMat.axis[1][2];
    originalOrigin[16] = baseBoneSkelMat.axis[1][3];
    baseBoneMatrix[1][0] = baseBoneSkelMat.axis[1][1];
    baseBoneMatrix[1][1] = baseBoneSkelMat.axis[1][2];
    baseBoneMatrix[1][2] = baseBoneSkelMat.axis[1][3];
    LODWORD(originalOrigin[15]) = baseBoneMatrix[2];
    originalOrigin[14] = baseBoneSkelMat.axis[2][1];
    originalOrigin[13] = baseBoneSkelMat.axis[2][2];
    originalOrigin[12] = baseBoneSkelMat.axis[2][3];
    baseBoneMatrix[2][0] = baseBoneSkelMat.axis[2][1];
    baseBoneMatrix[2][1] = baseBoneSkelMat.axis[2][2];
    baseBoneMatrix[2][2] = baseBoneSkelMat.axis[2][3];
    MatrixMultiply43((const float (*)[3])v27, (const float (*)[3]) & originalOrigin[24], (float (*)[3])originalOrigin);
    localMins[0] = markInfo->origin[0];
    localMins[1] = markInfo->origin[1];
    localMins[2] = markInfo->origin[2];
    MatrixTransformVector43(localMins, (const float (*)[3])originalOrigin, markInfo->localOrigin);
    scale = -markInfo->radius;
    Vec3AddScalar(markInfo->localOrigin, scale, localMaxs);
    Vec3AddScalar(markInfo->localOrigin, markInfo->radius, (float *)&markModelCoreContext.clipPlanes);
    v9[0] = markInfo;
    v9[1] = markContext;
    v9[2] = markInfo->localOrigin;
    markModelCoreContext.markInfo = (MarkInfo *)v70;
    markModelCoreContext.markContext = (GfxMarkContext *)markDir;
    if (XSurfaceVisitTrianglesInAabb(
        surface,
        vertListIndex,
        localMaxs,
        (const float *)&markModelCoreContext.clipPlanes,
        R_MarkModelCoreCallback_0_,
        v9))
    {
        if (markInfo->usedPointCount)
            MatrixTransposeTransformVector(markInfo->axis[1], (const float (*)[3])clipPlanes_84, markInfo->localTexCoordAxis);
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
        //Profile_EndInternal(0);
        return 0;
    }
}

char __cdecl R_MarkFragments_StaticModels(MarkInfo *markInfo)
{
    float v2; // [esp+E8h] [ebp-6Ch]
    float v3; // [esp+ECh] [ebp-68h]
    float v4; // [esp+F4h] [ebp-60h]
    float v5; // [esp+F8h] [ebp-5Ch]
    float v6; // [esp+FCh] [ebp-58h]
    float v7; // [esp+104h] [ebp-50h]
    float v8; // [esp+108h] [ebp-4Ch]
    float v9; // [esp+10Ch] [ebp-48h]
    int smodelCollidedIndex; // [esp+118h] [ebp-3Ch]
    const GfxStaticModelDrawInst *smodelDraw; // [esp+11Ch] [ebp-38h]
    GfxMarkContext markContext; // [esp+120h] [ebp-34h] BYREF
    const XModel *xmodel; // [esp+128h] [ebp-2Ch]
    float modelAxis[3][3]; // [esp+12Ch] [ebp-28h] BYREF
    int smodelIndex; // [esp+150h] [ebp-4h]

    for (smodelCollidedIndex = 0; smodelCollidedIndex != markInfo->smodelCollidedCount; ++smodelCollidedIndex)
    {
        if (markInfo->usedTriCount || markInfo->usedPointCount)
            MyAssertHandler(".\\r_marks.cpp", 1799, 0, "%s", "!markInfo->usedTriCount && !markInfo->usedPointCount");
        smodelIndex = markInfo->smodelsCollided[smodelCollidedIndex];
        smodelDraw = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
        xmodel = smodelDraw->model;
        markContext.lmapIndex = 31;
        markContext.reflectionProbeIndex = smodelDraw->reflectionProbeIndex;
        if (markContext.reflectionProbeIndex != smodelDraw->reflectionProbeIndex)
            MyAssertHandler(
                ".\\r_marks.cpp",
                1809,
                0,
                "%s\n\t(smodelDraw->reflectionProbeIndex) = %i",
                "(markContext.reflectionProbeIndex == smodelDraw->reflectionProbeIndex)",
                smodelDraw->reflectionProbeIndex);
        markContext.primaryLightIndex = smodelDraw->primaryLightIndex;
        markContext.modelIndex = smodelIndex;
        if ((unsigned __int16)smodelIndex != smodelIndex)
            MyAssertHandler(
                ".\\r_marks.cpp",
                1812,
                0,
                "%s\n\t(smodelIndex) = %i",
                "(markContext.modelIndex == smodelIndex)",
                smodelIndex);
        v2 = smodelDraw->placement.axis[0][1];
        v3 = smodelDraw->placement.axis[0][2];
        v4 = smodelDraw->placement.axis[1][0];
        v5 = smodelDraw->placement.axis[1][1];
        v6 = smodelDraw->placement.axis[1][2];
        v7 = smodelDraw->placement.axis[2][0];
        v8 = smodelDraw->placement.axis[2][1];
        v9 = smodelDraw->placement.axis[2][2];
        modelAxis[0][0] = smodelDraw->placement.axis[0][0];
        modelAxis[0][1] = v2;
        modelAxis[0][2] = v3;
        modelAxis[1][0] = v4;
        modelAxis[1][1] = v5;
        modelAxis[1][2] = v6;
        modelAxis[2][0] = v7;
        modelAxis[2][1] = v8;
        modelAxis[2][2] = v9;
        if (!R_MarkFragments_EntirelyRigidXModel(
            markInfo,
            xmodel,
            modelAxis,
            smodelDraw->placement.origin,
            smodelDraw->placement.scale,
            &markContext))
            return 0;
        if (markInfo->usedTriCount || markInfo->usedPointCount)
        {
            if (markInfo->usedTriCount)
            {
                if (markInfo->usedPointCount)
                    markInfo->callback(
                        markInfo->callbackContext,
                        markInfo->usedTriCount,
                        markInfo->tris,
                        markInfo->usedPointCount,
                        markInfo->points,
                        (const float *)markInfo,
                        markInfo->axis[1]);
            }
            markInfo->usedTriCount = 0;
            markInfo->usedPointCount = 0;
        }
    }
    if (markInfo->usedTriCount || markInfo->usedPointCount)
        MyAssertHandler(".\\r_marks.cpp", 1828, 0, "%s", "!markInfo->usedTriCount && !markInfo->usedPointCount");
    return 1;
}

char __cdecl R_MarkFragments_EntirelyRigidXModel(
    MarkInfo *markInfo,
    const XModel *xmodel,
    const float (*modelAxis)[3],
    const float *modelOrigin,
    float modelScale,
    GfxMarkContext *markContext)
{
    int surfIndex; // [esp+B0h] [ebp-10h]
    int surfCount; // [esp+B4h] [ebp-Ch]
    Material **materials; // [esp+B8h] [ebp-8h]
    XSurface *surfaces; // [esp+BCh] [ebp-4h] BYREF

    surfCount = XModelGetSurfaces(xmodel, &surfaces, 0);
    materials = XModelGetSkins(xmodel, 0);
    for (surfIndex = 0; surfIndex != surfCount && surfIndex <= 63; ++surfIndex)
    {
        if (R_Mark_MaterialAllowsMarks(materials[surfIndex], markInfo->material))
        {
            if ((surfIndex & 0x3F) != surfIndex)
                MyAssertHandler(
                    ".\\r_marks.cpp",
                    1541,
                    0,
                    "%s\n\t(surfIndex) = %i",
                    "((surfIndex & MARK_MODEL_SURF_MASK) == surfIndex)",
                    surfIndex);
            markContext->modelTypeAndSurf = surfIndex | 0x40;
            if (markContext->modelTypeAndSurf != (surfIndex | 0x40))
                MyAssertHandler(
                    ".\\r_marks.cpp",
                    1543,
                    0,
                    "%s\n\t(surfIndex | MARK_MODEL_TYPE_WORLD_MODEL) = %i",
                    "(markContext->modelTypeAndSurf == (surfIndex | MARK_MODEL_TYPE_WORLD_MODEL))",
                    surfIndex | 0x40);
            if (!R_MarkFragments_XModelSurface_Basic(
                markInfo,
                &surfaces[surfIndex],
                modelAxis,
                modelOrigin,
                modelScale,
                markContext))
                return 0;
        }
    }
    return 1;
}

char __cdecl R_MarkFragments_XModelSurface_Basic(
    MarkInfo *markInfo,
    const XSurface *surface,
    const float (*modelAxis)[3],
    const float *modelOrigin,
    float modelScale,
    GfxMarkContext *markContext)
{
    float scale; // [esp+0h] [ebp-A4h]
    float localRadius; // [esp+Ch] [ebp-98h]
    MarkModelCoreContext markModelCoreContext; // [esp+10h] [ebp-94h] BYREF
    float invModelScale; // [esp+2Ch] [ebp-78h]
    float localOrigin[3]; // [esp+30h] [ebp-74h] BYREF
    unsigned int vertListIndex; // [esp+3Ch] [ebp-68h]
    float localMaxs[3]; // [esp+40h] [ebp-64h] BYREF
    float localOriginRotated[3]; // [esp+4Ch] [ebp-58h] BYREF
    int dim; // [esp+58h] [ebp-4Ch]
    float localMins[3]; // [esp+5Ch] [ebp-48h] BYREF
    float surfTransform[4][3]; // [esp+68h] [ebp-3Ch] BYREF
    float localOriginTranslated[3]; // [esp+98h] [ebp-Ch] BYREF

    for (dim = 0; dim != 3; ++dim)
        Vec3Scale(&(*modelAxis)[3 * dim], modelScale, surfTransform[dim]);
    surfTransform[3][0] = *modelOrigin;
    surfTransform[3][1] = modelOrigin[1];
    surfTransform[3][2] = modelOrigin[2];
    invModelScale = 1.0 / modelScale;
    Vec3Sub(markInfo->origin, modelOrigin, localOriginTranslated);
    MatrixTransposeTransformVector(localOriginTranslated, modelAxis, localOriginRotated);
    Vec3Scale(localOriginRotated, invModelScale, localOrigin);
    localRadius = markInfo->radius * invModelScale;
    scale = -localRadius;
    Vec3AddScalar(localOrigin, scale, localMins);
    Vec3AddScalar(localOrigin, localRadius, localMaxs);
    markModelCoreContext.markInfo = markInfo;
    markModelCoreContext.markContext = markContext;
    markModelCoreContext.markOrigin = (const float *)markInfo;
    markModelCoreContext.markDir = markInfo->axis[0];
    markModelCoreContext.clipPlanes = markInfo->planes;
    markModelCoreContext.transformMatrix = surfTransform;
    markModelCoreContext.transformNormalMatrix = modelAxis;
    for (vertListIndex = 0; vertListIndex != surface->vertListCount; ++vertListIndex)
    {
        if (!XSurfaceVisitTrianglesInAabb(
            surface,
            vertListIndex,
            localMins,
            localMaxs,
            (bool(__cdecl *)(void *, const GfxPackedVertex **, const GfxPackedVertex **))R_MarkModelCoreCallback_1_,
            &markModelCoreContext))
            return 0;
    }
    return 1;
}

int __cdecl R_ChopPolyBehindPlane(
    int inPointCount,
    FxModelMarkPoint *inPoints,
    FxModelMarkPoint *outPoints,
    const float *plane)
{
    double v4; // st7
    const FxModelMarkPoint *v6; // eax
    FxModelMarkPoint *v7; // ecx
    const FxModelMarkPoint *v8; // ecx
    FxModelMarkPoint *v9; // edx
    int sideCount[3]; // [esp+38h] [ebp-6Ch] BYREF
    float dists[10]; // [esp+44h] [ebp-60h]
    int nextIndex; // [esp+6Ch] [ebp-38h]
    float lerp; // [esp+70h] [ebp-34h]
    int pointIndex; // [esp+74h] [ebp-30h]
    int sides[10]; // [esp+78h] [ebp-2Ch]
    int outPointCount; // [esp+A0h] [ebp-4h]

    if (inPointCount > 9)
        MyAssertHandler(".\\r_marks.cpp", 158, 0, "%s\n\t(inPointCount) = %i", "(inPointCount <= 3 + 6)", inPointCount);
    memset(sideCount, 0, sizeof(sideCount));
    for (pointIndex = 0; pointIndex < inPointCount; ++pointIndex)
    {
        v4 = Vec3Dot(inPoints[pointIndex].xyz, plane);
        dists[pointIndex] = v4 - plane[3];
        if (dists[pointIndex] <= 0.5)
        {
            if (dists[pointIndex] >= -0.5)
                sides[pointIndex] = 2;
            else
                sides[pointIndex] = 1;
        }
        else
        {
            sides[pointIndex] = 0;
        }
        ++sideCount[sides[pointIndex]];
    }
    sides[pointIndex] = sides[0];
    dists[pointIndex] = dists[0];
    if (!sideCount[0])
        return 0;
    if (sideCount[1])
    {
        outPointCount = 0;
        for (pointIndex = 0; pointIndex < inPointCount; ++pointIndex)
        {
            if (sides[pointIndex] == 2)
            {
                if (outPointCount >= 9)
                    MyAssertHandler(
                        ".\\r_marks.cpp",
                        199,
                        0,
                        "%s\n\t(outPointCount) = %i",
                        "(outPointCount < 3 + 6)",
                        outPointCount);
                v6 = &inPoints[pointIndex];
                v7 = &outPoints[outPointCount];
                v7->xyz[0] = v6->xyz[0];
                v7->xyz[1] = v6->xyz[1];
                v7->xyz[2] = v6->xyz[2];
                v7->vertWeights[0] = v6->vertWeights[0];
                v7->vertWeights[1] = v6->vertWeights[1];
                v7->vertWeights[2] = v6->vertWeights[2];
                ++outPointCount;
            }
            else
            {
                if (!sides[pointIndex])
                {
                    if (outPointCount >= 9)
                        MyAssertHandler(
                            ".\\r_marks.cpp",
                            207,
                            0,
                            "%s\n\t(outPointCount) = %i",
                            "(outPointCount < 3 + 6)",
                            outPointCount);
                    v8 = &inPoints[pointIndex];
                    v9 = &outPoints[outPointCount];
                    v9->xyz[0] = v8->xyz[0];
                    v9->xyz[1] = v8->xyz[1];
                    v9->xyz[2] = v8->xyz[2];
                    v9->vertWeights[0] = v8->vertWeights[0];
                    v9->vertWeights[1] = v8->vertWeights[1];
                    v9->vertWeights[2] = v8->vertWeights[2];
                    ++outPointCount;
                }
                if (sides[pointIndex + 1] != 2 && sides[pointIndex + 1] != sides[pointIndex])
                {
                    if (outPointCount >= 9)
                        MyAssertHandler(
                            ".\\r_marks.cpp",
                            216,
                            0,
                            "%s\n\t(outPointCount) = %i",
                            "(outPointCount < 3 + 6)",
                            outPointCount);
                    if (dists[pointIndex + 1] == dists[pointIndex])
                        MyAssertHandler(
                            ".\\r_marks.cpp",
                            217,
                            0,
                            "%s\n\t(dists[pointIndex]) = %g",
                            "(dists[pointIndex] != dists[pointIndex + 1])",
                            dists[pointIndex]);
                    lerp = dists[pointIndex] / (dists[pointIndex] - dists[pointIndex + 1]);
                    nextIndex = (pointIndex + 1) % inPointCount;
                    R_LerpModelMarkPoints(&inPoints[pointIndex], &inPoints[nextIndex], lerp, &outPoints[outPointCount++]);
                }
            }
        }
        return outPointCount;
    }
    else
    {
        //Profile_Begin(166);
        memcpy((unsigned __int8 *)outPoints, (unsigned __int8 *)inPoints, 24 * inPointCount);
        //Profile_EndInternal(0);
        return inPointCount;
    }
}

void __cdecl R_LerpModelMarkPoints(
    const FxModelMarkPoint *from,
    const FxModelMarkPoint *to,
    float lerp,
    FxModelMarkPoint *out)
{
    Vec3Lerp(from->xyz, to->xyz, lerp, out->xyz);
    Vec3Lerp(from->vertWeights, to->vertWeights, lerp, out->vertWeights);
}

