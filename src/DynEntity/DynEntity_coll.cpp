#include "DynEntity_client.h"
#include <qcommon/mem_track.h>

DynEntityCollWorld dynEntCollWorlds[2];

void __cdecl TRACK_DynEntityCollWorld()
{
    int32_t collType; // [esp+0h] [ebp-4h]

    for (collType = 0; collType < 2; ++collType)
        track_static_alloc_internal(&dynEntCollWorlds[collType], 20508, "dynEntCollWorlds[collType]", 25);
}

DynEntityCollSector *__cdecl DynEnt_GetCollSector(DynEntityCollType collType, uint32_t sectorIndex)
{
    if (!sectorIndex)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 71, 0, "%s", "sectorIndex");
    if (sectorIndex >= 0x400)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            72,
            0,
            "%s\n\t(sectorIndex) = %i",
            "(sectorIndex < 1024)",
            sectorIndex);
    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    return &dynEntCollWorlds[collType].sectors[sectorIndex];
}

void __cdecl DynEnt_ClearCollWorld(DynEntityCollType collType)
{
    float worldSize; // [esp+8h] [ebp-14h]
    float worldSize_4; // [esp+Ch] [ebp-10h]
    uint16_t sectorIndex; // [esp+14h] [ebp-8h]
    DynEntityCollWorld *world; // [esp+18h] [ebp-4h]

    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    world = &dynEntCollWorlds[collType];
    memset((uint8_t *)world, 0, sizeof(DynEntityCollWorld));
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

void __cdecl DynEnt_UnlinkEntity(DynEntityCollType collType, uint16_t dynEntId)
{
    uint16_t EntityCount; // ax
    DynEntityClient *ClientEntity; // eax
    int32_t contents; // [esp+8h] [ebp-28h]
    DynEntityCollSector *sector; // [esp+Ch] [ebp-24h]
    DynEntityColl *scan; // [esp+14h] [ebp-1Ch]
    DynEntityColl *scana; // [esp+14h] [ebp-1Ch]
    uint16_t parentSectorIndex; // [esp+18h] [ebp-18h]
    uint16_t parentSectorIndexa; // [esp+18h] [ebp-18h]
    const DynEntityDef *dynEntDef; // [esp+1Ch] [ebp-14h]
    DynEntityColl *next; // [esp+20h] [ebp-10h]
    uint16_t sectorIndex; // [esp+24h] [ebp-Ch]
    DynEntityCollWorld *world; // [esp+28h] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+2Ch] [ebp-4h]

    if (dynEntId == 0xFFFF)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 410, 0, "%s", "dynEntId != DYNENT_INVALID_ID");
    if (dynEntId >= (uint32_t)DynEnt_GetEntityCount(collType))
    {
        EntityCount = DynEnt_GetEntityCount(collType);
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            411,
            0,
            "dynEntId doesn't index DynEnt_GetEntityCount( collType )\n\t%i not in [0, %i)",
            dynEntId,
            EntityCount);
    }
    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    world = &dynEntCollWorlds[collType];
    dynEntColl = DynEnt_GetEntityColl(collType, dynEntId);
    sectorIndex = dynEntColl->sector;
    if (dynEntColl->sector)
    {
        sector = &world->sectors[sectorIndex];
        dynEntColl->sector = 0;
        if (!world->sectors[sectorIndex].entListHead)
            MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 423, 0, "%s", "sector->entListHead");
        if (world->sectors[sectorIndex].entListHead - 1 == dynEntId)
        {
            world->sectors[sectorIndex].entListHead = dynEntColl->nextEntInSector;
        }
        else
        {
            for (scan = DynEnt_GetEntityColl(collType, world->sectors[sectorIndex].entListHead - 1); ; scan = next)
            {
                next = DynEnt_GetEntityColl(collType, scan->nextEntInSector - 1);
                if (next == dynEntColl)
                    break;
                if (!scan->nextEntInSector)
                    MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 441, 0, "%s", "scan->nextEntInSector");
            }
            scan->nextEntInSector = dynEntColl->nextEntInSector;
        }
        while (!sector->entListHead && !sector->tree.child[0] && !sector->tree.child[1])
        {
            sector->contents = 0;
            if (!sector->tree.u.parent)
            {
                if (sectorIndex != 1)
                    MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 451, 0, "%s", "sectorIndex == DYNENT_SECTOR_HEAD");
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
                    MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 468, 0, "%s", "sector->tree.child[1] == sectorIndex");
                world->sectors[parentSectorIndex].tree.child[1] = 0;
            }
            sectorIndex = parentSectorIndex;
        }
        dynEntDef = DynEnt_GetEntityDef(dynEntId, (DynEntityDrawType)(collType & 1));
        while (1)
        {
            contents = world->sectors[sector->tree.child[1]].contents | world->sectors[sector->tree.child[0]].contents;
            if (sector->entListHead)
            {
                for (scana = DynEnt_GetEntityColl(collType, sector->entListHead - 1);
                    ;
                    scana = DynEnt_GetEntityColl(collType, scana->nextEntInSector - 1))
                {
                    contents |= DynEnt_GetContents(dynEntDef);
                    if (!scana->nextEntInSector)
                        break;
                }
            }
            sector->contents = contents;
            parentSectorIndexa = sector->tree.u.parent;
            if (!parentSectorIndexa)
                break;
            sector = &world->sectors[parentSectorIndexa];
        }
        ClientEntity = DynEnt_GetClientEntity(dynEntId, (DynEntityDrawType)(collType & 1));
        ClientEntity->flags &= ~4u;
    }
}

void __cdecl DynEnt_LinkEntity(
    DynEntityCollType collType,
    uint16_t dynEntId,
    const float *absMins,
    const float *absMaxs)
{
    uint16_t EntityCount; // ax
    uint16_t *p_flags; // [esp+4h] [ebp-4Ch]
    int32_t contents; // [esp+1Ch] [ebp-34h]
    DynEntityCollSector *sector; // [esp+20h] [ebp-30h]
    float dist; // [esp+28h] [ebp-28h]
    float mins[2]; // [esp+2Ch] [ebp-24h] BYREF
    const DynEntityDef *dynEntDef; // [esp+34h] [ebp-1Ch]
    uint16_t sectorIndex; // [esp+38h] [ebp-18h]
    float maxs[2]; // [esp+3Ch] [ebp-14h] BYREF
    DynEntityCollWorld *world; // [esp+44h] [ebp-Ch]
    int32_t axis; // [esp+48h] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+4Ch] [ebp-4h]

    if (dynEntId == 0xFFFF)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 530, 0, "%s", "dynEntId != DYNENT_INVALID_ID");
    if (dynEntId >= (uint32_t)DynEnt_GetEntityCount(collType))
    {
        EntityCount = DynEnt_GetEntityCount(collType);
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            531,
            0,
            "dynEntId doesn't index DynEnt_GetEntityCount( collType )\n\t%i not in [0, %i)",
            dynEntId,
            EntityCount);
    }
    if (!absMins)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 532, 0, "%s", "absMins");
    if (!absMaxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 533, 0, "%s", "absMaxs");
    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    world = &dynEntCollWorlds[collType];
    dynEntDef = DynEnt_GetEntityDef(dynEntId, (DynEntityDrawType)(collType & 1));
    dynEntColl = DynEnt_GetEntityColl(collType, dynEntId);
    contents = DynEnt_GetContents(dynEntDef);
    if (!contents)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 542, 0, "%s", "contents");
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
                world->sectors[sectorIndex].contents |= contents;
                if (dist >= (double)absMins[axis])
                    break;
                mins[axis] = dist;
                if (!sector->tree.child[0])
                    goto LABEL_24;
                sectorIndex = sector->tree.child[0];
            }
            if (dist <= (double)absMaxs[axis])
                break;
            maxs[axis] = dist;
            if (!sector->tree.child[1])
                goto LABEL_24;
        }
        if (sectorIndex == dynEntColl->sector)
        {
            *(double *)dynEntColl->linkMins = *(double *)absMins;
            *(double *)dynEntColl->linkMaxs = *(double *)absMaxs;
            return;
        }
    LABEL_24:
        if (!dynEntColl->sector)
            break;
        if (sectorIndex == dynEntColl->sector)
            goto LABEL_29;
        DynEnt_UnlinkEntity(collType, dynEntId);
    }
    DynEnt_AddToCollSector(collType, dynEntId, sectorIndex);
LABEL_29:
    *(double *)dynEntColl->linkMins = *(double *)absMins;
    *(double *)dynEntColl->linkMaxs = *(double *)absMaxs;
    DynEnt_SortCollSector(collType, sectorIndex, mins, maxs);
    p_flags = &DynEnt_GetClientEntity(dynEntId, (DynEntityDrawType)(collType & 1))->flags;
    *p_flags |= 4u;
}

void __cdecl DynEnt_AddToCollSector(
    DynEntityCollType collType,
    uint16_t dynEntId,
    uint16_t sectorIndex)
{
    uint16_t EntityCount; // ax
    uint16_t *prevListIndex; // [esp+8h] [ebp-14h]
    DynEntityColl *dynEntColl; // [esp+18h] [ebp-4h]

    if (dynEntId == 0xFFFF)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 274, 0, "%s", "dynEntId != DYNENT_INVALID_ID");
    if (dynEntId >= (uint32_t)DynEnt_GetEntityCount(collType))
    {
        EntityCount = DynEnt_GetEntityCount(collType);
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            275,
            0,
            "dynEntId doesn't index DynEnt_GetEntityCount( collType )\n\t%i not in [0, %i)",
            dynEntId,
            EntityCount);
    }
    if (!sectorIndex)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 277, 0, "%s", "sectorIndex");
    if (sectorIndex >= 0x400u)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            278,
            0,
            "%s\n\t(sectorIndex) = %i",
            "(sectorIndex < 1024)",
            sectorIndex);
    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    dynEntColl = DynEnt_GetEntityColl(collType, dynEntId);
    for (prevListIndex = &dynEntCollWorlds[collType].sectors[sectorIndex].entListHead;
        (uint32_t)*prevListIndex - 1 <= dynEntId;
        prevListIndex = &DynEnt_GetEntityColl(collType, *prevListIndex - 1)->nextEntInSector)
    {
        ;
    }
    dynEntColl->sector = sectorIndex;
    dynEntColl->nextEntInSector = *prevListIndex;
    *prevListIndex = dynEntId + 1;
}

void __cdecl DynEnt_SortCollSector(
    DynEntityCollType collType,
    uint16_t sectorIndex,
    const float *mins,
    const float *maxs)
{
    uint16_t listIndex; // [esp+8h] [ebp-28h]
    float dist; // [esp+10h] [ebp-20h]
    const DynEntityDef *dynEntDef; // [esp+14h] [ebp-1Ch]
    DynEntityCollWorld *world; // [esp+18h] [ebp-18h]
    uint16_t dynEntId; // [esp+1Ch] [ebp-14h]
    DynEntityColl *prevDynEntColl; // [esp+20h] [ebp-10h]
    int32_t axis; // [esp+24h] [ebp-Ch]
    DynEntityColl *dynEntColl; // [esp+28h] [ebp-8h]
    uint16_t childSectorIndex; // [esp+2Ch] [ebp-4h]

    if (!sectorIndex)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 321, 0, "%s", "sectorIndex");
    if (sectorIndex >= 0x400u)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            322,
            0,
            "%s\n\t(sectorIndex) = %i",
            "(sectorIndex < 1024)",
            sectorIndex);
    if (!mins)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 323, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 324, 0, "%s", "maxs");
    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    world = &dynEntCollWorlds[collType];
    axis = world->sectors[sectorIndex].tree.axis;
    dist = world->sectors[sectorIndex].tree.dist;
    prevDynEntColl = 0;
    listIndex = world->sectors[sectorIndex].entListHead;
    while (listIndex)
    {
        dynEntId = listIndex - 1;
        dynEntDef = DynEnt_GetEntityDef(listIndex - 1, (DynEntityDrawType)(collType & 1));
        dynEntColl = DynEnt_GetEntityColl(collType, listIndex - 1);
        if (dist >= (double)dynEntColl->linkMins[axis])
        {
            if (dist > (double)dynEntColl->linkMaxs[axis])
            {
                childSectorIndex = dynEntCollWorlds[collType].sectors[sectorIndex].tree.child[1];
                if (childSectorIndex)
                    goto LABEL_24;
                childSectorIndex = DynEnt_AllocCollSector(collType, mins, maxs);
                if (childSectorIndex)
                {
                    dynEntCollWorlds[collType].sectors[sectorIndex].tree.child[1] = childSectorIndex;
                    world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                    goto LABEL_24;
                }
            }
            goto skipEntity_0;
        }
        childSectorIndex = dynEntCollWorlds[collType].sectors[sectorIndex].tree.child[0];
        if (childSectorIndex)
        {
        LABEL_24:
            listIndex = dynEntColl->nextEntInSector;
            if (!prevDynEntColl
                && DynEnt_GetEntityColl(collType, dynEntCollWorlds[collType].sectors[sectorIndex].entListHead - 1) != dynEntColl)
            {
                MyAssertHandler(
                    ".\\DynEntity\\DynEntity_coll.cpp",
                    379,
                    0,
                    "%s",
                    "prevDynEntColl || (DynEnt_GetEntityColl( collType, sector->entListHead - 1 ) == dynEntColl)");
            }
            if (prevDynEntColl && DynEnt_GetEntityColl(collType, prevDynEntColl->nextEntInSector - 1) != dynEntColl)
                MyAssertHandler(
                    ".\\DynEntity\\DynEntity_coll.cpp",
                    380,
                    0,
                    "%s",
                    "!prevDynEntColl || (DynEnt_GetEntityColl( collType, prevDynEntColl->nextEntInSector - 1) == dynEntColl)");
            DynEnt_AddToCollSector(collType, dynEntId, childSectorIndex);
            world->sectors[childSectorIndex].contents |= DynEnt_GetContents(dynEntDef);
            if (prevDynEntColl)
                prevDynEntColl->nextEntInSector = listIndex;
            else
                dynEntCollWorlds[collType].sectors[sectorIndex].entListHead = listIndex;
        }
        else
        {
            childSectorIndex = DynEnt_AllocCollSector(collType, mins, maxs);
            if (childSectorIndex)
            {
                dynEntCollWorlds[collType].sectors[sectorIndex].tree.child[0] = childSectorIndex;
                world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                goto LABEL_24;
            }
        skipEntity_0:
            prevDynEntColl = dynEntColl;
            listIndex = dynEntColl->nextEntInSector;
        }
    }
}

uint16_t __cdecl DynEnt_AllocCollSector(DynEntityCollType collType, const float *mins, const float *maxs)
{
    DynEntityCollSector *sector; // [esp+4h] [ebp-18h]
    float size[2]; // [esp+8h] [ebp-14h]
    uint16_t sectorIndex; // [esp+10h] [ebp-Ch]
    DynEntityCollWorld *world; // [esp+14h] [ebp-8h]
    uint16_t axis; // [esp+18h] [ebp-4h]

    if (!mins)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 230, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 231, 0, "%s", "maxs");
    if ((uint32_t)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            59,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    world = &dynEntCollWorlds[collType];
    sectorIndex = world->freeHead;
    if (!sectorIndex)
        return 0;
    size[0] = *maxs - *mins;
    size[1] = maxs[1] - mins[1];
    axis = size[1] >= (double)size[0];
    if (size[size[1] >= (double)size[0]] <= 512.0)
        return 0;
    sector = &world->sectors[sectorIndex];
    if (world->sectors[sectorIndex].contents)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 247, 0, "%s", "!sector->contents");
    if (sector->entListHead)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 248, 0, "%s", "!sector->entListHead");
    world->freeHead = sector->tree.u.parent;
    sector->tree.axis = axis;
    sector->tree.dist = (maxs[axis] + mins[axis]) * 0.5;
    if (sector->tree.child[0])
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 255, 0, "%s", "!sector->tree.child[0]");
    if (sector->tree.child[1])
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 256, 0, "%s", "!sector->tree.child[1]");
    return sectorIndex;
}

int32_t __cdecl DynEnt_GetContents(const DynEntityDef *dynEntDef)
{
    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 625, 0, "%s", "dynEntDef");
    return dynEntDef->contents;
}

void __cdecl DynEnt_GetLocalBounds(const DynEntityDef *dynEntDef, float *mins, float *maxs)
{
    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 636, 0, "%s", "dynEntDef");
    if (!dynEntDef->xModel && !dynEntDef->brushModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 637, 0, "%s", "dynEntDef->xModel || dynEntDef->brushModel");
    if (!mins)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 638, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 639, 0, "%s", "maxs");
    if (dynEntDef->xModel)
        XModelGetBounds(dynEntDef->xModel, mins, maxs);
    else
        CM_ModelBounds(dynEntDef->brushModel, mins, maxs);
}

void __cdecl DynEnt_GetWorldBounds(const DynEntityPose *dynEntPose, float *mins, float *maxs)
{
    float s; // [esp+0h] [ebp-Ch]
    float radius; // [esp+8h] [ebp-4h]

    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 655, 0, "%s", "dynEntPose");
    if (!mins)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 656, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 657, 0, "%s", "maxs");
    radius = dynEntPose->radius;
    s = -radius;
    Vec3AddScalar(dynEntPose->pose.origin, s, mins);
    Vec3AddScalar(dynEntPose->pose.origin, radius, maxs);
}

double __cdecl DynEnt_GetRadiusDistSqr(const DynEntityPose *dynEntPose, const float *origin)
{
    float absMaxs[3]; // [esp+0h] [ebp-28h] BYREF
    float offset[3]; // [esp+Ch] [ebp-1Ch] BYREF
    int32_t vecIndex; // [esp+18h] [ebp-10h]
    float absMins[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 676, 0, "%s", "dynEntPose");
    if (!origin)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 677, 0, "%s", "origin");
    DynEnt_GetWorldBounds(dynEntPose, absMins, absMaxs);
    for (vecIndex = 0; vecIndex < 3; ++vecIndex)
    {
        if (absMins[vecIndex] <= (double)origin[vecIndex])
        {
            if (absMaxs[vecIndex] >= (double)origin[vecIndex])
                offset[vecIndex] = 0.0;
            else
                offset[vecIndex] = origin[vecIndex] - absMaxs[vecIndex];
        }
        else
        {
            offset[vecIndex] = absMins[vecIndex] - origin[vecIndex];
        }
    }
    return Vec3LengthSq(offset);
}

double __cdecl DynEnt_GetCylindricalRadiusDistSqr(const DynEntityPose *dynEntPose, const float *origin)
{
    float absMaxs[3]; // [esp+4h] [ebp-24h] BYREF
    float offset[2]; // [esp+10h] [ebp-18h]
    int32_t vecIndex; // [esp+18h] [ebp-10h]
    float absMins[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 717, 0, "%s", "dynEntPose");
    if (!origin)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 718, 0, "%s", "origin");
    DynEnt_GetWorldBounds(dynEntPose, absMins, absMaxs);
    for (vecIndex = 0; vecIndex < 2; ++vecIndex)
    {
        if (absMins[vecIndex] <= (double)origin[vecIndex])
        {
            if (absMaxs[vecIndex] >= (double)origin[vecIndex])
                offset[vecIndex] = 0.0;
            else
                offset[vecIndex] = origin[vecIndex] - absMaxs[vecIndex];
        }
        else
        {
            offset[vecIndex] = absMins[vecIndex] - origin[vecIndex];
        }
    }
    return (float)(offset[1] * offset[1] + offset[0] * offset[0]);
}

bool __cdecl DynEnt_EntityInArea(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const float *mins,
    const float *maxs,
    int32_t contentMask)
{
    float absMaxs[3]; // [esp+0h] [ebp-18h] BYREF
    float absMins[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 742, 0, "%s", "dynEntDef");
    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 743, 0, "%s", "dynEntPose");
    if (!mins)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 744, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 745, 0, "%s", "maxs");
    if ((contentMask & DynEnt_GetContents(dynEntDef)) == 0)
        return 0;
    DynEnt_GetWorldBounds(dynEntPose, absMins, absMaxs);
    if (*maxs < (double)absMins[0] || maxs[1] < (double)absMins[1] || maxs[2] < (double)absMins[2])
        return 0;
    return *mins <= (double)absMaxs[0] && mins[1] <= (double)absMaxs[1] && mins[2] <= (double)absMaxs[2];
}

void __cdecl DynEnt_PointTraceToModel(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const pointtrace_t *clip,
    trace_t *results)
{
    uint16_t Id; // [esp+Ah] [ebp-5Eh]
    float dynEntAxis[4][3]; // [esp+14h] [ebp-54h] BYREF
    float localStart[3]; // [esp+44h] [ebp-24h] BYREF
    float normal[3]; // [esp+50h] [ebp-18h] BYREF
    float localEnd[3]; // [esp+5Ch] [ebp-Ch] BYREF

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 772, 0, "%s", "dynEntDef");
    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 773, 0, "%s", "dynEntPose");
    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 774, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 775, 0, "%s", "results");
    if (!dynEntDef->xModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 776, 0, "%s", "dynEntDef->xModel");
    if (results->fraction <= 0.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            778,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction > 0.0f)",
            results->fraction);
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            779,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    if (clip->bLocational)
    {
        dynEntAxis[3][0] = dynEntPose->pose.origin[0];
        dynEntAxis[3][1] = dynEntPose->pose.origin[1];
        dynEntAxis[3][2] = dynEntPose->pose.origin[2];
        UnitQuatToAxis(dynEntPose->pose.quat, *(mat3x3*)dynEntAxis);
        MatrixTransposeTransformVector43(clip->extents.start, dynEntAxis, localStart);
        MatrixTransposeTransformVector43(clip->extents.end, dynEntAxis, localEnd);
        if (!XModelTraceLine(dynEntDef->xModel, results, localStart, localEnd, clip->contentmask))
        {
            MatrixTransformVector(results->normal, *(const mat3x3*)dynEntAxis, normal);
            Vec3NormalizeTo(normal, results->normal);
            Id = DynEnt_GetId(dynEntDef, DYNENT_DRAW_MODEL);
            if (!results)
                MyAssertHandler("c:\\trees\\cod3\\src\\qcommon\\cm_public.h", 135, 0, "%s", "trace");
            results->hitType = TRACE_HITTYPE_DYNENT_MODEL;
            results->hitId = Id;
        }
    }
}

void __cdecl DynEnt_PointTraceToBrush(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const pointtrace_t *clip,
    trace_t *results)
{
    uint16_t Id; // [esp+Ah] [ebp-2Ah]
    float oldFraction; // [esp+Ch] [ebp-28h]
    float axis[3][3]; // [esp+10h] [ebp-24h] BYREF

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 808, 0, "%s", "dynEntDef");
    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 809, 0, "%s", "dynEntPose");
    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 810, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 811, 0, "%s", "results");
    if (dynEntDef->xModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 812, 0, "%s", "!dynEntDef->xModel");
    if (!dynEntDef->brushModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 813, 0, "%s", "dynEntDef->brushModel");
    if (results->fraction <= 0.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            815,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction > 0.0f)",
            results->fraction);
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            816,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    oldFraction = results->fraction;
    UnitQuatToAxis(dynEntPose->pose.quat, axis);
    CM_TransformedBoxTraceRotated(
        results,
        clip->extents.start,
        clip->extents.end,
        vec3_origin,
        vec3_origin,
        dynEntDef->brushModel,
        clip->contentmask,
        dynEntPose->pose.origin,
        axis);
    if (oldFraction > (double)results->fraction)
    {
        Id = DynEnt_GetId(dynEntDef, DYNENT_DRAW_BRUSH);
        if (!results)
            MyAssertHandler("c:\\trees\\cod3\\src\\qcommon\\cm_public.h", 135, 0, "%s", "trace");
        results->hitType = TRACE_HITTYPE_DYNENT_BRUSH;
        results->hitId = Id;
    }
}

void __cdecl DynEnt_ClipMoveTraceToBrush(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const moveclip_t *clip,
    trace_t *results)
{
    uint16_t Id; // [esp+Ah] [ebp-2Eh]
    float oldFraction; // [esp+10h] [ebp-28h]
    float axis[3][3]; // [esp+14h] [ebp-24h] BYREF

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 838, 0, "%s", "dynEntDef");
    if (!dynEntPose)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 839, 0, "%s", "dynEntPose");
    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 840, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 841, 0, "%s", "results");
    if (results->fraction <= 0.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            843,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction > 0.0f)",
            results->fraction);
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_coll.cpp",
            844,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    if (!dynEntDef->brushModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_coll.cpp", 845, 0, "%s", "dynEntDef->brushModel");
    if ((clip->contentmask & DynEnt_GetContents(dynEntDef)) != 0
        && DynEnt_GetEntityProps(dynEntDef->type)->clipMove
        && !CM_TraceSphere(&clip->extents, dynEntPose->pose.origin, dynEntPose->radius, results->fraction))
    {
        oldFraction = results->fraction;
        UnitQuatToAxis(dynEntPose->pose.quat, axis);
        CM_TransformedBoxTraceRotated(
            results,
            clip->extents.start,
            clip->extents.end,
            clip->mins,
            clip->maxs,
            dynEntDef->brushModel,
            clip->contentmask,
            dynEntPose->pose.origin,
            axis);
        if (oldFraction > (double)results->fraction)
        {
            Id = DynEnt_GetId(dynEntDef, DYNENT_DRAW_BRUSH);
            if (!results)
                MyAssertHandler("c:\\trees\\cod3\\src\\qcommon\\cm_public.h", 135, 0, "%s", "trace");
            results->hitType = TRACE_HITTYPE_DYNENT_BRUSH;
            results->hitId = Id;
        }
    }
}

void __cdecl DynEnt_SetPhysObjCollision(const DynEntityDef *dynEntDef, dxBody *physId)
{
    float mins[3]; // [esp+0h] [ebp-18h] BYREF
    float maxs[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (dynEntDef->xModel && dynEntDef->xModel->physGeoms)
    {
        Phys_ObjSetCollisionFromXModel(dynEntDef->xModel, PHYS_WORLD_DYNENT, physId);
    }
    else if (dynEntDef->physicsBrushModel)
    {
        Phys_ObjAddGeomBrushModel(PHYS_WORLD_DYNENT, physId, dynEntDef->physicsBrushModel, &dynEntDef->mass);
    }
    else
    {
        DynEnt_GetLocalBounds(dynEntDef, mins, maxs);
        if (dynEntDef->physPreset->tempDefaultToCylinder)
            Phys_ObjAddGeomCylinder(PHYS_WORLD_DYNENT, physId, mins, maxs);
        else
            Phys_ObjAddGeomBox(PHYS_WORLD_DYNENT, physId, mins, maxs);
    }
}

