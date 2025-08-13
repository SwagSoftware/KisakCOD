#include "r_marks.h"

#include "r_gfx.h"
#include "r_init.h"
#include <xanim/dobj_utils.h>
#include <EffectsCore/fx_system.h>
#include "r_model_skin.h"
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_pose.h>
#include <cgame/cg_ents.h>
#endif


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
    unsigned __int8 cellBits[128]; // [esp-4h] [ebp-9Ch] BYREF

    iassert(rgp.world);
    iassert(rgp.world->dpvsPlanes.cellCount <= (1024));
    iassert(rgp.world->cellBitsCount <= ((1024) >> 3));

    Com_Memset(cellBits, 0, rgp.world->cellBitsCount);
    for (unsigned int i = 0; i < listCount; ++i)
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
        (unsigned __int8 *)cellBits);

    for (unsigned int i = 0; i < listCount; ++i)
    {
        iassert(surfCounts[i] <= surfListSize);
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
    int side; // [esp+0h] [ebp-10h]
    int cellIndex; // [esp+4h] [ebp-Ch]
    int cellCount; // [esp+8h] [ebp-8h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while (1)
    {
        cellIndex = node->cellIndex;
        if (cellIndex - cellCount < 0)
            break;
        side = BoxOnPlaneSide(mins, maxs, &rgp.world->dpvsPlanes.planes[cellIndex - cellCount]);
        if (side == 1)
        {
            ++node;
        }
        else
        {
            if (side != 2)
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
            node = (mnode_t*)((char*)node + 2 * node->rightChildOffset);
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
    unsigned __int8 v8[128]; // [esp-8h] [ebp-9Ch] BYREF

    //v9 = a1;
    //v10 = retaddr;
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

    PROF_SCOPED("R_BoxStaticModels");
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
        v8);
    if (v7 > smodelListSize)
        MyAssertHandler(".\\r_marks.cpp", 975, 0, "smodelCount <= smodelListSize\n\t%i, %i", v7, smodelListSize);
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
    int side; // [esp+0h] [ebp-10h]
    int cellIndex; // [esp+4h] [ebp-Ch]
    int cellCount; // [esp+8h] [ebp-8h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while (1)
    {
        cellIndex = node->cellIndex;
        if (cellIndex - cellCount < 0)
            break;
        side = BoxOnPlaneSide(mins, maxs, &rgp.world->dpvsPlanes.planes[cellIndex - cellCount]);
        if (side == 1)
        {
            ++node;
        }
        else
        {
            if (side != 2)
                R_BoxStaticModels_r(node + 1, mins, maxs, allowSModel, smodelList, smodelListSize, smodelCount, cellBits);
            node = (mnode_t*)((char*)node + 2 * node->rightChildOffset);
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
    unsigned __int8 v12[128]; // [esp+34h] [ebp-9Ch] BYREF

    //v13 = a1;
    //v14 = retaddr;
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
        v12);
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
            || (v13 = I_fabs(startDist), radius > (double)v13)
            || (v12 = I_fabs(endDist), radius > (double)v12))
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

    PROF_SCOPED("R_MarkUtil_GetDObjAnimMatAndHideParts");
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
    AxisCopy(*(const mat3x3*)axis, markInfo->axis);
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
        iassert(markAgainst == MARK_FRAGMENTS_AGAINST_MODELS);
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
        v7 = I_fabs((*axis)[coord]);
        v6 = I_fabs((*axis)[coord + 3]);
        v5 = I_fabs((*axis)[coord + 6]);
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

    iassert(markInfo->material);

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
        iassert(markInfo->markAgainst == MARK_FRAGMENTS_AGAINST_MODELS);
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

    iassert(!markInfo->usedTriCount && !markInfo->usedPointCount);

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

bool __cdecl R_MarkFragments_BrushSurface(
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
        if (!R_MarkFragment_IsTriangleRejected(markDir, triVerts0[indices[0]].xyz, triVerts0[indices[1]].xyz, triVerts0[indices[2]].xyz))
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

                iassert(fragmentPointCount <= 3 + 6);
                pingPong ^= 1u;
            }
            if (fragmentPointCount > markInfo->maxPoints - markInfo->usedPointCount || 3 * (fragmentPointCount - 2) > markInfo->maxTris - markInfo->usedTriCount)
            {
                return false;
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
            triVert1[0] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[0]];
            triVert1[1] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[1]];
            triVert1[2] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[2]];

            lmapCoord[0][0] = triVert1[0]->lmapCoord[0];
            lmapCoord[0][1] = triVert1[0]->lmapCoord[1];

            lmapCoord[1][0] = triVert1[1]->lmapCoord[0];
            lmapCoord[1][1] = triVert1[1]->lmapCoord[1];

            lmapCoord[2][0] = triVert1[2]->lmapCoord[0];
            lmapCoord[2][1] = triVert1[2]->lmapCoord[1];

            float tmp[3];
            Vec3UnpackUnitVec(triVert1[0]->normal, tmp);
            normal[0][0] = tmp[0];
            normal[0][1] = tmp[1];
            normal[0][2] = tmp[2];

            Vec3UnpackUnitVec(triVert1[1]->normal, tmp);
            normal[1][0] = tmp[0];
            normal[1][1] = tmp[1];
            normal[1][2] = tmp[2];

            Vec3UnpackUnitVec(triVert1[2]->normal, tmp);
            normal[2][0] = tmp[0];
            normal[2][1] = tmp[1];
            normal[2][2] = tmp[2];

            points = &markInfo->points[markInfo->usedPointCount];

            for (pointIndex = 0; pointIndex < fragmentPointCount; ++pointIndex)
            {
                point = &points[pointIndex];
                clipPoint = &clipPoints[pingPong][pointIndex];

                point->xyz[0] = clipPoint->xyz[0];
                point->xyz[1] = clipPoint->xyz[1];
                point->xyz[2] = clipPoint->xyz[2];

                point->lmapCoord[0] = (clipPoint->vertWeights[0] * lmapCoord[0][0]) + (clipPoint->vertWeights[1] * lmapCoord[1][0]) + (clipPoint->vertWeights[2] * lmapCoord[2][0]);
                point->lmapCoord[1] = (clipPoint->vertWeights[0] * lmapCoord[0][1]) + (clipPoint->vertWeights[1] * lmapCoord[1][1]) + (clipPoint->vertWeights[2] * lmapCoord[2][1]);

                point->normal[0] = (clipPoint->vertWeights[0] * normal[0][0]) + (clipPoint->vertWeights[1] * normal[1][0]) + (clipPoint->vertWeights[2] * normal[2][0]);
                point->normal[1] = (clipPoint->vertWeights[0] * normal[0][1]) + (clipPoint->vertWeights[1] * normal[1][1]) + (clipPoint->vertWeights[2] * normal[2][1]);
                point->normal[2] = (clipPoint->vertWeights[0] * normal[0][2]) + (clipPoint->vertWeights[1] * normal[1][2]) + (clipPoint->vertWeights[2] * normal[2][2]);
            }
            iassert(fragmentPointCount >= 3);
            markInfo->usedPointCount += fragmentPointCount;
            markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
            *anyMarks = 1;
        }
    LABEL_6:
        ++triIndex;
        indices += 3;
    }
    return true;
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
        {
            PROF_SCOPED("R_memcpy");
            memcpy((unsigned __int8 *)outPoints, (unsigned __int8 *)inPoints, 24 * inPointCount);
        }
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
        MatrixTransposeTransformVector(markInfo->axis[0], *(const mat3x3*)poseMatrix, markDir);
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
            MatrixTranspose(*(const mat3x3 *)poseMatrix, *(mat3x3*)invPoseMatrix);
            negatedPoseOrigin[0] = -poseMatrix[3][0];
            negatedPoseOrigin[1] = -poseMatrix[3][1];
            negatedPoseOrigin[2] = -poseMatrix[3][2];
            MatrixTransformVector(negatedPoseOrigin, *(const mat3x3 *)invPoseMatrix, invPoseMatrix[3]);
            MatrixTransformVector43(markInfo->origin, invPoseMatrix, transformedOrigin);
            MatrixTransposeTransformVector(markInfo->axis[1], *(const mat3x3 *)poseMatrix, transformedTexCoordAxis);
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

    PROF_SCOPED("R_MarkFragments_SceneDObjs");

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
                return 0;
            }
            boneIndex += model->numBones;
            boneMtxList += model->numBones;
        }
    }
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

    PROF_SCOPED("R_MarkFragments_AnimatedXModel");

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

    return 1;
}

int __cdecl R_AddMarkFragment_1_(
    FxModelMarkPoint(*clipPoints)[9],
    const float (*planes)[4],
    const GfxMarkContext *markContext,
    unsigned __int16 baseIndex,
    int maxTris,
    FxMarkTri *tris,
    int maxPoints,
    const GfxPackedVertex **triVerts,
    const float (*transformNormalMatrix)[3],
    FxMarkPoint *points)
{
    float out3[3]; // [esp+54h] [ebp-8Ch] BYREF
    PackedUnitVec v12; // [esp+60h] [ebp-80h]
    PackedUnitVec v13; // [esp+68h] [ebp-78h]
    float out2[3]; // [esp+6Ch] [ebp-74h] BYREF
    PackedUnitVec v15; // [esp+78h] [ebp-68h]
    PackedUnitVec v16; // [esp+80h] [ebp-60h]
    float out[3]; // [esp+84h] [ebp-5Ch] BYREF
    PackedUnitVec in; // [esp+90h] [ebp-50h]
    float tempNormal[3]; // [esp+98h] [ebp-48h] BYREF
    float normal[3][3]; // [esp+A4h] [ebp-3Ch] BYREF
    FxModelMarkPoint *clipPoint; // [esp+C8h] [ebp-18h]
    int pointIndex; // [esp+CCh] [ebp-14h]
    int pingPong; // [esp+D0h] [ebp-10h]
    int clipPointCount; // [esp+D4h] [ebp-Ch]
    int planeIndex; // [esp+D8h] [ebp-8h]
    FxMarkPoint *point; // [esp+DCh] [ebp-4h]

    pingPong = 0;
    clipPointCount = 3;
    for (planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        clipPointCount = R_ChopPolyBehindPlane(
            clipPointCount,
            &(*clipPoints)[9 * pingPong],
            &(*clipPoints)[9 * (pingPong == 0)],
            &(*planes)[4 * planeIndex]);
        if (clipPointCount > 9)
            MyAssertHandler(
                ".\\r_marks.cpp",
                1016,
                0,
                "%s\n\t(clipPointCount) = %i",
                "(clipPointCount <= 3 + 6)",
                clipPointCount);
        pingPong ^= 1u;
        if (!clipPointCount)
            return 0;
    }
    if (clipPointCount > maxPoints || 3 * (clipPointCount - 2) > maxTris)
        return -1;

    Vec3UnpackUnitVec(triVerts[0]->normal, out);
    tempNormal[0] = out[0];
    tempNormal[1] = out[1];
    tempNormal[2] = out[2];
    MatrixTransformVector(tempNormal, *(const mat3x3 *)transformNormalMatrix, normal[0]);

    Vec3UnpackUnitVec(triVerts[1]->normal, out2);
    tempNormal[0] = out2[0];
    tempNormal[1] = out2[1];
    tempNormal[2] = out2[2];
    MatrixTransformVector(tempNormal, *(const mat3x3 *)transformNormalMatrix, normal[1]);

    Vec3UnpackUnitVec(triVerts[2]->normal, out3);
    tempNormal[0] = out3[0];
    tempNormal[1] = out3[1];
    tempNormal[2] = out3[2];
    MatrixTransformVector(tempNormal, *(const mat3x3*)transformNormalMatrix, normal[2]);

    for (pointIndex = 0; pointIndex < clipPointCount; ++pointIndex)
    {
        point = &points[pointIndex];
        clipPoint = &(*clipPoints)[9 * pingPong + pointIndex];
        point->xyz[0] = clipPoint->xyz[0];
        point->xyz[1] = clipPoint->xyz[1];
        point->xyz[2] = clipPoint->xyz[2];
        point->lmapCoord[0] = 0.0;
        point->lmapCoord[1] = 0.0;
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
    for (pointIndex = 2; pointIndex < clipPointCount; ++pointIndex)
    {
        tris->indices[0] = baseIndex + pointIndex - 1;
        tris->indices[1] = pointIndex + baseIndex;
        tris->indices[2] = baseIndex;
        tris->context = *markContext;
        ++tris;
    }
    return clipPointCount;
}

char __cdecl R_MarkFragment_DoTriangle_1_(
    MarkInfo *markInfo,
    const float (*clipPlanes)[4],
    const GfxMarkContext *markContext,
    const GfxPackedVertex **triVerts,
    const float (*transformNormalMatrix)[3],
    FxModelMarkPoint(*clipPoints)[9])
{
    int fragmentPointCount; // [esp+0h] [ebp-4h]

    fragmentPointCount = R_AddMarkFragment_1_(
        clipPoints,
        clipPlanes,
        markContext,
        markInfo->usedPointCount,
        markInfo->maxTris - markInfo->usedTriCount,
        &markInfo->tris[markInfo->usedTriCount],
        markInfo->maxPoints - markInfo->usedPointCount,
        triVerts,
        transformNormalMatrix,
        &markInfo->points[markInfo->usedPointCount]);
    if (fragmentPointCount == -1)
        return 0;
    if (fragmentPointCount)
    {
        if (fragmentPointCount < 3)
            MyAssertHandler(
                ".\\r_marks.cpp",
                1161,
                0,
                "%s\n\t(fragmentPointCount) = %i",
                "(fragmentPointCount >= 3)",
                fragmentPointCount);
        markInfo->usedPointCount += fragmentPointCount;
        markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
    }
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
        MatrixTransformVector43(pos, *(const mat4x3 *)context->transformMatrix, clipPoints[0][vertIndex].xyz);
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

int __cdecl R_AddMarkFragment_0_(
    FxModelMarkPoint(*clipPoints)[9],
    const float (*planes)[4],
    const GfxMarkContext *markContext,
    unsigned __int16 baseIndex,
    int maxTris,
    FxMarkTri *tris,
    int maxPoints,
    const GfxPackedVertex **triVerts,
    const float (*transformNormalMatrix)[3],
    FxMarkPoint *points)
{
    float out3[3]; // [esp+8h] [ebp-D8h] BYREF
    PackedUnitVec v12; // [esp+14h] [ebp-CCh]
    float *v13; // [esp+1Ch] [ebp-C4h]
    PackedUnitVec v14; // [esp+20h] [ebp-C0h]
    float out2[3]; // [esp+24h] [ebp-BCh] BYREF
    PackedUnitVec v16; // [esp+30h] [ebp-B0h]
    PackedUnitVec v17; // [esp+38h] [ebp-A8h]
    float out[3]; // [esp+3Ch] [ebp-A4h] BYREF
    PackedUnitVec v19; // [esp+48h] [ebp-98h]
    float normal[3][3]; // [esp+A4h] [ebp-3Ch] BYREF
    FxModelMarkPoint *clipPoint; // [esp+C8h] [ebp-18h]
    int pointIndex; // [esp+CCh] [ebp-14h]
    int pingPong; // [esp+D0h] [ebp-10h]
    int clipPointCount; // [esp+D4h] [ebp-Ch]
    int planeIndex; // [esp+D8h] [ebp-8h]
    FxMarkPoint *point; // [esp+DCh] [ebp-4h]

    pingPong = 0;
    clipPointCount = 3;
    for (planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        clipPointCount = R_ChopPolyBehindPlane(
            clipPointCount,
            &(*clipPoints)[9 * pingPong],
            &(*clipPoints)[9 * (pingPong == 0)],
            &(*planes)[4 * planeIndex]);
        if (clipPointCount > 9)
            MyAssertHandler(
                ".\\r_marks.cpp",
                1016,
                0,
                "%s\n\t(clipPointCount) = %i",
                "(clipPointCount <= 3 + 6)",
                clipPointCount);
        pingPong ^= 1u;
        if (!clipPointCount)
            return 0;
    }
    if (clipPointCount > maxPoints || 3 * (clipPointCount - 2) > maxTris)
        return -1;

    Vec3UnpackUnitVec(triVerts[0]->normal, out);
    normal[0][0] = out[0];
    normal[0][1] = out[1];
    normal[0][2] = out[2];

    Vec3UnpackUnitVec(triVerts[1]->normal, out2);
    normal[1][0] = out2[0];
    normal[1][1] = out2[1];
    normal[1][2] = out2[2];

    Vec3UnpackUnitVec(triVerts[2]->normal, out3);
    normal[2][0] = out3[0];
    normal[2][1] = out3[1];
    normal[2][2] = out3[2];

    for (pointIndex = 0; pointIndex < clipPointCount; ++pointIndex)
    {
        point = &points[pointIndex];
        clipPoint = &(*clipPoints)[9 * pingPong + pointIndex];
        point->xyz[0] = clipPoint->xyz[0];
        point->xyz[1] = clipPoint->xyz[1];
        point->xyz[2] = clipPoint->xyz[2];
        point->lmapCoord[0] = 0.0;
        point->lmapCoord[1] = 0.0;
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
    for (pointIndex = 2; pointIndex < clipPointCount; ++pointIndex)
    {
        tris->indices[0] = baseIndex + pointIndex - 1;
        tris->indices[1] = pointIndex + baseIndex;
        tris->indices[2] = baseIndex;
        tris->context = *markContext;
        ++tris;
    }
    return clipPointCount;
}

char __cdecl R_MarkFragment_DoTriangle_0_(
    MarkInfo *markInfo,
    const float (*clipPlanes)[4],
    const GfxMarkContext *markContext,
    const GfxPackedVertex **triVerts,
    const float (*transformNormalMatrix)[3],
    FxModelMarkPoint(*clipPoints)[9])
{
    int fragmentPointCount; // [esp+0h] [ebp-4h]

    fragmentPointCount = R_AddMarkFragment_0_(
        clipPoints,
        clipPlanes,
        markContext,
        markInfo->usedPointCount,
        markInfo->maxTris - markInfo->usedTriCount,
        &markInfo->tris[markInfo->usedTriCount],
        markInfo->maxPoints - markInfo->usedPointCount,
        triVerts,
        transformNormalMatrix,
        &markInfo->points[markInfo->usedPointCount]);
    if (fragmentPointCount == -1)
        return 0;
    if (fragmentPointCount)
    {
        if (fragmentPointCount < 3)
            MyAssertHandler(
                ".\\r_marks.cpp",
                1161,
                0,
                "%s\n\t(fragmentPointCount) = %i",
                "(fragmentPointCount >= 3)",
                fragmentPointCount);
        markInfo->usedPointCount += fragmentPointCount;
        markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
    }
    return 1;
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
        *(const float**)((uintptr_t)contextAsVoid + 12),
        clipPoints[0][0].xyz,
        clipPoints[0][1].xyz,
        clipPoints[0][2].xyz)
        || R_MarkFragment_DoTriangle_0_(
            *(MarkInfo**)contextAsVoid,
            *(const float(**)[4])((uintptr_t)contextAsVoid + 16),
            *(const GfxMarkContext**)((uintptr_t)contextAsVoid + 4),
            triVerts1,
            *(const float(**)[3])((uintptr_t)contextAsVoid + 24),
            clipPoints) != 0;
}

void __cdecl DObjSkelMatToMatrix43(const DObjSkelMat *inSkelMat, float (*outMatrix)[3])
{
    float *v2; // r10
    const float *v3; // r11
    int v4; // r9
    double v5; // fp0
    double v6; // fp13
    double v7; // fp0
    double v8; // fp13

    v2 = &(*outMatrix)[2];
    v3 = &inSkelMat->axis[0][1];
    v4 = 3;
    do
    {
        --v4;
        v5 = v3[1];
        v6 = *v3;
        *(v2 - 2) = *(v3 - 1);
        v3 += 4;
        *v2 = v5;
        *(v2 - 1) = v6;
        v2 += 3;
    } while (v4);
    v7 = inSkelMat->origin[2];
    v8 = inSkelMat->origin[1];
    (*outMatrix)[9] = inSkelMat->origin[0];
    (*outMatrix)[10] = v8;
    (*outMatrix)[11] = v7;
}

char  R_MarkFragments_AnimatedXModel_VertList(
    MarkInfo *markInfo,
    unsigned int vertListIndex,
    const DObjAnimMat *poseBone,
    const DObjAnimMat *baseBone,
    GfxMarkContext *markContext,
    XSurface *surface)
{
    DObjAnimMat *v12; // r10
    int v13; // ctr
    double v14; // fp13
    double radius; // fp0
    double v16; // fp11
    double v17; // fp9
    double v18; // fp10
    float v20[4]; // [sp+50h] [-2B0h] BYREF
    float v21[4]; // [sp+60h] [-2A0h] BYREF
    float v22[4]; // [sp+70h] [-290h] BYREF
    DObjAnimMat v23; // [sp+80h] [-280h] BYREF
    void* v24[8]; // [sp+A0h] [-260h] BYREF
    float v25[4]; // [sp+C0h] [-240h] BYREF
    float v26[4][3]; // [sp+D0h] [-230h] BYREF
    DObjSkelMat v27; // [sp+100h] [-200h] BYREF
    DObjSkelMat v28; // [sp+140h] [-1C0h] BYREF
    float v29[4][3]; // [sp+180h] [-180h] BYREF
    float v30[4][3]; // [sp+1B0h] [-150h] BYREF
    float v31[4][3]; // [sp+1E0h] [-120h] BYREF
    float v32[4][3]; // [sp+210h] [-F0h] BYREF
    float v33[4][3]; // [sp+240h] [-C0h] BYREF
    float v34[9][4]; // [sp+270h] [-90h] BYREF

    v12 = &v23;
    v13 = 8;
    do
    {
        v12->quat[0] = poseBone->quat[0];
        poseBone = (const DObjAnimMat *)((char *)poseBone + 4);
        v12 = (DObjAnimMat *)((char *)v12 + 4);
        --v13;
    } while (v13);
    v14 = (float)(markInfo->viewOffset[1] + v23.trans[1]);
    v23.trans[0] = markInfo->viewOffset[0] + v23.trans[0];
    v23.trans[1] = v14;
    v23.trans[2] = markInfo->viewOffset[2] + v23.trans[2];
    ConvertQuatToSkelMat(&v23, &v28);
    ConvertQuatToInverseSkelMat(baseBone, &v27);
    DObjSkelMatToMatrix43(&v28, v29);
    DObjSkelMatToMatrix43(&v27, v30);
    MatrixMultiply43(v30, v29, v26);
    R_Mark_TransformClipPlanes(markInfo->planes, v26, v34);
    MatrixTransposeTransformVector(markInfo->axis[0], *(const mat3x3*)&v26[0][0], v25);
    ConvertQuatToSkelMat(baseBone, &v28);
    ConvertQuatToInverseSkelMat(&v23, &v27);
    DObjSkelMatToMatrix43(&v27, v31);
    DObjSkelMatToMatrix43(&v28, v32);
    MatrixMultiply43(v31, v32, v33);
    v22[0] = markInfo->origin[0];
    v22[1] = markInfo->origin[1];
    v22[2] = markInfo->origin[2];
    MatrixTransformVector43(v22, v33, markInfo->localOrigin);
    radius = markInfo->radius;
    v16 = markInfo->localOrigin[1];
    v17 = markInfo->localOrigin[0];
    v18 = markInfo->localOrigin[2];

    v24[0] = markInfo;
    v24[1] = markContext;
    v24[2] = markInfo->localOrigin;
    v24[3] = v25;
    v24[4] = v34;

    v21[0] = (float)v17 + (float)-radius;
    v21[2] = (float)v18 + (float)-radius;
    v20[0] = (float)v17 + (float)radius;
    v21[1] = (float)v16 + (float)-radius;
    v20[1] = (float)v16 + (float)radius;
    v20[2] = (float)v18 + (float)radius;
    if (!(unsigned __int8)XSurfaceVisitTrianglesInAabb(surface, vertListIndex, v21, v20, R_MarkModelCoreCallback_0_, v24))
        return 0;
    if (markInfo->usedPointCount)
        MatrixTransposeTransformVector(markInfo->axis[1], *(const mat3x3*)&v26[0][0], markInfo->localTexCoordAxis);
    return 1;
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
    MatrixTransposeTransformVector(localOriginTranslated, *(const mat3x3*)modelAxis, localOriginRotated);
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
        {
            PROF_SCOPED("R_memcpy");
            memcpy((unsigned __int8 *)outPoints, (unsigned __int8 *)inPoints, 24 * inPointCount);
        }
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

