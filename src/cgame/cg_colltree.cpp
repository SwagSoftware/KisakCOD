#include "cg_local.h"
#include "cg_public.h"

#include <qcommon/mem_track.h>
#include <qcommon/threads.h>

CgEntCollWorld cgEntCollWorld[1];
CgEntCollNode cgEntCollNodes[1][MAX_GENTITIES];
int32_t cgCollWorldLocalClientNum;

void __cdecl TRACK_CG_CollWorld()
{
    track_static_alloc_internal(cgEntCollWorld, 16412, "cgEntCollWorld", 25);
    track_static_alloc_internal(cgEntCollNodes, 20480, "cgEntCollNodes", 25);
}

void __cdecl CG_SetCollWorldLocalClientNum(int32_t localClientNum)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 61, 0, "%s", "Sys_IsMainThread()");
    cgCollWorldLocalClientNum = localClientNum;
}

int32_t __cdecl CG_GetCollWorldLocalClientNum()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 71, 0, "%s", "Sys_IsMainThread()");
    return cgCollWorldLocalClientNum;
}

void __cdecl CG_ClearEntityCollWorld(int32_t localClientNum)
{
    float worldSize; // [esp+8h] [ebp-14h]
    float worldSize_4; // [esp+Ch] [ebp-10h]
    uint16_t sectorIndex; // [esp+14h] [ebp-8h]
    CgEntCollWorld *world; // [esp+18h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_colltree.cpp",
            86,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    memset((uint8_t *)&cgEntCollWorld[localClientNum], 0, sizeof(CgEntCollWorld));
    memset((uint8_t *)cgEntCollNodes[localClientNum], 0, sizeof(CgEntCollNode[1024]));
    world = &cgEntCollWorld[localClientNum];
    CM_ModelBounds(0, world->mins, world->maxs);
    world->freeHead = 2;
    for (sectorIndex = 2; sectorIndex < 0x3FFu; ++sectorIndex)
        world->sectors[sectorIndex].tree.u.parent = sectorIndex + 1;
    worldSize = world->maxs[0] - world->mins[0];
    worldSize_4 = world->maxs[1] - world->mins[1];
    world->sectors[1].tree.axis = worldSize_4 >= (double)worldSize;
    world->sectors[1].tree.dist = (world->maxs[world->sectors[1].tree.axis] + world->mins[world->sectors[1].tree.axis])
        * 0.5;
}

const CgEntCollSector *__cdecl CG_GetEntityCollSector(int32_t localClientNum, uint16_t sectorIndex)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_colltree.cpp",
            117,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (!sectorIndex)
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 118, 0, "%s", "sectorIndex");
    if (sectorIndex >= 0x400u)
        MyAssertHandler(
            ".\\cgame\\cg_colltree.cpp",
            119,
            0,
            "%s\n\t(sectorIndex) = %i",
            "(sectorIndex < 1024)",
            sectorIndex);
    return &cgEntCollWorld[localClientNum].sectors[sectorIndex];
}

const CgEntCollNode *__cdecl CG_GetEntityCollNode(int32_t localClientNum, uint32_t entIndex)
{
    return CG_GetCollNode(localClientNum, entIndex);
}

CgEntCollNode *__cdecl CG_GetCollNode(int32_t localClientNum, uint32_t entIndex)
{
    //bcassert(localClientNum, STATIC_MAX_LOCAL_CLIENTS);
    bcassert(entIndex, MAX_GENTITIES);
    return &cgEntCollNodes[localClientNum][entIndex];
}

void __cdecl CG_UnlinkEntityColl(int32_t localClientNum, uint32_t entIndex)
{
    CgEntCollNode *node; // [esp+0h] [ebp-1Ch]
    CgEntCollSector *sector; // [esp+4h] [ebp-18h]
    CgEntCollNode *scan; // [esp+8h] [ebp-14h]
    uint16_t parentSectorIndex; // [esp+Ch] [ebp-10h]
    uint16_t parentSectorIndexa; // [esp+Ch] [ebp-10h]
    CgEntCollNode *next; // [esp+10h] [ebp-Ch]
    uint16_t sectorIndex; // [esp+14h] [ebp-8h]
    CgEntCollWorld *world; // [esp+18h] [ebp-4h]

    //bcassert(localClientNum, STATIC_MAX_LOCAL_CLIENTS);
    bcassert(entIndex, MAX_GENTITIES);

    world = &cgEntCollWorld[localClientNum];
    node = CG_GetCollNode(localClientNum, entIndex);
    sectorIndex = node->sector;
    if (node->sector)
    {
        sector = &world->sectors[sectorIndex];
        node->sector = 0;
        if (!world->sectors[sectorIndex].entListHead)
            MyAssertHandler(".\\cgame\\cg_colltree.cpp", 342, 0, "%s", "sector->entListHead");
        if (world->sectors[sectorIndex].entListHead - 1 == entIndex)
        {
            world->sectors[sectorIndex].entListHead = node->nextEntInSector;
        }
        else
        {
            for (scan = CG_GetCollNode(localClientNum, world->sectors[sectorIndex].entListHead - 1); ; scan = next)
            {
                next = CG_GetCollNode(localClientNum, scan->nextEntInSector - 1);
                if (next == node)
                    break;
                if (!scan->nextEntInSector)
                    MyAssertHandler(".\\cgame\\cg_colltree.cpp", 360, 0, "%s", "scan->nextEntInSector");
            }
            scan->nextEntInSector = node->nextEntInSector;
        }
        while (!sector->entListHead && !sector->tree.child[0] && !sector->tree.child[1])
        {
            if (!sector->tree.u.parent)
            {
                if (sectorIndex != 1)
                    MyAssertHandler(".\\cgame\\cg_colltree.cpp", 368, 0, "%s", "sectorIndex == CG_ENT_SECTOR_HEAD");
                break;
            }
            parentSectorIndex = sector->tree.u.parent;
            sector->tree.u.parent = world->freeHead;
            world->freeHead = sectorIndex;
            sector = &world->sectors[parentSectorIndex];
            if (world->sectors[parentSectorIndex].tree.child[0] == sectorIndex)
            {
                world->sectors[parentSectorIndex].tree.child[0] = 0;
            }
            else
            {
                if (world->sectors[parentSectorIndex].tree.child[1] != sectorIndex)
                    MyAssertHandler(".\\cgame\\cg_colltree.cpp", 385, 0, "%s", "sector->tree.child[1] == sectorIndex");
                world->sectors[parentSectorIndex].tree.child[1] = 0;
            }
            sectorIndex = parentSectorIndex;
        }
        while (1)
        {
            parentSectorIndexa = sector->tree.u.parent;
            if (!parentSectorIndexa)
                break;
            sector = &world->sectors[parentSectorIndexa];
        }
    }
}

void __cdecl CG_LinkEntityColl(int32_t localClientNum, uint32_t entIndex, const float *absMins, const float *absMaxs)
{
    CgEntCollNode *node; // [esp+14h] [ebp-28h]
    CgEntCollSector *sector; // [esp+18h] [ebp-24h]
    float dist; // [esp+1Ch] [ebp-20h]
    float mins[2]; // [esp+20h] [ebp-1Ch] BYREF
    uint16_t sectorIndex; // [esp+28h] [ebp-14h]
    float maxs[2]; // [esp+2Ch] [ebp-10h] BYREF
    CgEntCollWorld *world; // [esp+34h] [ebp-8h]
    int32_t axis; // [esp+38h] [ebp-4h]

    //bcassert(localClientNum, STATIC_MAX_LOCAL_CLIENTS);
    bcassert(entIndex, MAX_GENTITIES);

    iassert(absMins);
    iassert(absMaxs);
    
    world = &cgEntCollWorld[localClientNum];
    node = CG_GetCollNode(localClientNum, entIndex);
    while (1)
    {
        mins[0] = world->mins[0];
        mins[1] = world->mins[1];
        maxs[0] = world->maxs[0];
        maxs[1] = world->maxs[1];
        for (sectorIndex = 1; ; sectorIndex = sector->tree.child[1])
        {
            while (1)
            {
                sector = &world->sectors[sectorIndex];
                axis = world->sectors[sectorIndex].tree.axis;
                dist = sector->tree.dist;
                if (dist >= (double)absMins[axis])
                    break;
                mins[axis] = dist;
                if (!sector->tree.child[0])
                    goto LABEL_21;
                sectorIndex = sector->tree.child[0];
            }
            if (dist <= (double)absMaxs[axis])
                break;
            maxs[axis] = dist;
            if (!sector->tree.child[1])
                goto LABEL_21;
        }
        if (sectorIndex == node->sector)
        {
            node->linkMins[0] = *absMins;
            node->linkMins[1] = absMins[1];
            node->linkMaxs[0] = *absMaxs;
            node->linkMaxs[1] = absMaxs[1];
            return;
        }
    LABEL_21:
        if (!node->sector)
            break;
        if (sectorIndex == node->sector)
            goto LABEL_26;
        CG_UnlinkEntityColl(localClientNum, entIndex);
    }
    CG_AddEntityToCollSector(localClientNum, entIndex, sectorIndex);
LABEL_26:
    node->linkMins[0] = *absMins;
    node->linkMins[1] = absMins[1];
    node->linkMaxs[0] = *absMaxs;
    node->linkMaxs[1] = absMaxs[1];
    CG_SortEntityCollSector(localClientNum, sectorIndex, mins, maxs);
}

void __cdecl CG_AddEntityToCollSector(int32_t localClientNum, uint32_t entIndex, uint16_t sectorIndex)
{
    CgEntCollNode *node; // [esp+0h] [ebp-18h]
    uint16_t *prevListIndex; // [esp+Ch] [ebp-Ch]

    //bcassert(localClientNum, STATIC_MAX_LOCAL_CLIENTS);
    bcassert(entIndex, MAX_GENTITIES);
    iassert(sectorIndex);
    iassert(sectorIndex < 1024);
    node = CG_GetCollNode(localClientNum, entIndex);
    for (prevListIndex = &cgEntCollWorld[localClientNum].sectors[sectorIndex].entListHead;
        (uint32_t)*prevListIndex - 1 <= entIndex;
        prevListIndex = &CG_GetCollNode(localClientNum, *prevListIndex - 1)->nextEntInSector)
    {
        ;
    }
    node->sector = sectorIndex;
    node->nextEntInSector = *prevListIndex;
    *prevListIndex = entIndex + 1;
}

void __cdecl CG_SortEntityCollSector(
    int32_t localClientNum,
    uint16_t sectorIndex,
    const float *mins,
    const float *maxs)
{
    CgEntCollNode *node; // [esp+0h] [ebp-24h]
    uint16_t listIndex; // [esp+8h] [ebp-1Ch]
    float dist; // [esp+Ch] [ebp-18h]
    CgEntCollNode *prevNode; // [esp+10h] [ebp-14h]
    CgEntCollWorld *world; // [esp+14h] [ebp-10h]
    uint32_t entIndex; // [esp+18h] [ebp-Ch]
    int32_t axis; // [esp+1Ch] [ebp-8h]
    uint16_t childSectorIndex; // [esp+20h] [ebp-4h]

    //bcassert(localClientNum, STATIC_MAX_LOCAL_CLIENTS);
    iassert(sectorIndex);
    iassert(sectorIndex < 1024);
    iassert(mins);
    iassert(maxs);

    world = &cgEntCollWorld[localClientNum];
    axis = world->sectors[sectorIndex].tree.axis;
    dist = world->sectors[sectorIndex].tree.dist;
    prevNode = 0;
    listIndex = world->sectors[sectorIndex].entListHead;
    while (listIndex)
    {
        entIndex = listIndex - 1;
        node = CG_GetCollNode(localClientNum, entIndex);
        if (dist >= (double)node->linkMins[axis])
        {
            if (dist > (double)node->linkMaxs[axis])
            {
                childSectorIndex = cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[1];
                if (childSectorIndex)
                    goto LABEL_24;
                childSectorIndex = CG_AllocEntityCollSector(localClientNum, mins, maxs);
                if (childSectorIndex)
                {
                    cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[1] = childSectorIndex;
                    world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                    goto LABEL_24;
                }
            }
            goto skipEntity;
        }
        childSectorIndex = cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[0];
        if (childSectorIndex)
        {
        LABEL_24:
            listIndex = node->nextEntInSector;
            if (!prevNode
                && CG_GetCollNode(localClientNum, cgEntCollWorld[localClientNum].sectors[sectorIndex].entListHead - 1) != node)
            {
                MyAssertHandler(
                    ".\\cgame\\cg_colltree.cpp",
                    303,
                    0,
                    "%s",
                    "prevNode || (CG_GetCollNode( localClientNum, sector->entListHead - 1 ) == node)");
            }
            if (prevNode && CG_GetCollNode(localClientNum, prevNode->nextEntInSector - 1) != node)
                MyAssertHandler(
                    ".\\cgame\\cg_colltree.cpp",
                    304,
                    0,
                    "%s",
                    "!prevNode || (CG_GetCollNode( localClientNum, prevNode->nextEntInSector - 1) == node)");
            CG_AddEntityToCollSector(localClientNum, entIndex, childSectorIndex);
            if (prevNode)
                prevNode->nextEntInSector = listIndex;
            else
                cgEntCollWorld[localClientNum].sectors[sectorIndex].entListHead = listIndex;
        }
        else
        {
            childSectorIndex = CG_AllocEntityCollSector(localClientNum, mins, maxs);
            if (childSectorIndex)
            {
                cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[0] = childSectorIndex;
                world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                goto LABEL_24;
            }
        skipEntity:
            prevNode = node;
            listIndex = node->nextEntInSector;
        }
    }
}

uint16_t __cdecl CG_AllocEntityCollSector(int32_t localClientNum, const float *mins, const float *maxs)
{
    CgEntCollSector *sector; // [esp+4h] [ebp-18h]
    float size[2]; // [esp+8h] [ebp-14h]
    uint16_t sectorIndex; // [esp+10h] [ebp-Ch]
    CgEntCollWorld *world; // [esp+14h] [ebp-8h]
    uint16_t axis; // [esp+18h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_colltree.cpp",
            200,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (!mins)
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 201, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 202, 0, "%s", "maxs");
    world = &cgEntCollWorld[localClientNum];
    sectorIndex = world->freeHead;
    if (!sectorIndex)
        return 0;
    size[0] = *maxs - *mins;
    size[1] = maxs[1] - mins[1];
    axis = size[1] >= (double)size[0];
    if (size[size[1] >= (double)size[0]] <= 512.0)
        return 0;
    sector = &world->sectors[sectorIndex];
    if (world->sectors[sectorIndex].entListHead)
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 218, 0, "%s", "!sector->entListHead");
    world->freeHead = sector->tree.u.parent;
    sector->tree.axis = axis;
    sector->tree.dist = (maxs[axis] + mins[axis]) * 0.5;
    if (sector->tree.child[0])
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 225, 0, "%s", "!sector->tree.child[0]");
    if (sector->tree.child[1])
        MyAssertHandler(".\\cgame\\cg_colltree.cpp", 226, 0, "%s", "!sector->tree.child[1]");
    return sectorIndex;
}

