#include "qcommon.h"
#include "mem_track.h"
#include <server_mp/server_mp.h>
#include <server/sv_world.h>
#include <xanim/xmodel.h>
#include <server/sv_game.h>
#include <universal/profile.h>

#include <bgame/bg_public.h>
#include "cmd.h"

struct worldContents_s // sizeof=0x10
{                                       // ...
    int contentsStaticModels;           // ...
    int contentsEntities;               // ...
    int linkcontentsEntities;           // ...
    unsigned __int16 entities;          // ...
    unsigned __int16 staticModels;      // ...
};
union worldTree_s_u// sizeof=0x2
{                                       // ...
    unsigned __int16 parent;
    unsigned __int16 nextFree;
};
struct worldTree_s // sizeof=0xC
{                                       // ...
    float dist;                         // ...
    unsigned __int16 axis;              // ...
    worldTree_s_u u;    // ...
    unsigned __int16 child[2];          // ...
};
struct worldSector_s // sizeof=0x1C
{                                       // ...
    worldContents_s contents;           // ...
    worldTree_s tree;                   // ...
};

struct cm_world_t // sizeof=0x701C
{                                       // ...
    float mins[3];                      // ...
    float maxs[3];                      // ...
    unsigned __int16 freeHead;          // ...
    // padding byte
    // padding byte
    worldSector_s sectors[1024];        // ...
};

cm_world_t cm_world;

void __cdecl TRACK_cm_world()
{
    track_static_alloc_internal(&cm_world, 28700, "cm_world", 25);
}

void __cdecl CM_LinkWorld()
{
    ProfLoad_Begin("CM_LinkWorld");
    CM_ClearWorld();
    CM_LinkAllStaticModels();
    ProfLoad_End();
}

void CM_ClearWorld()
{
    float size; // [esp+4h] [ebp-Ch]
    float size_4; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    memset((unsigned __int8 *)&cm_world, 0, sizeof(cm_world));
    CM_ModelBounds(0, cm_world.mins, cm_world.maxs);
    cm_world.freeHead = 2;
    for (i = 2; i < 0x3FF; ++i)
        cm_world.sectors[i].tree.u.parent = i + 1;
    cm_world.sectors[1023].tree.u.parent = 0;
    size = cm_world.maxs[0] - cm_world.mins[0];
    size_4 = cm_world.maxs[1] - cm_world.mins[1];
    cm_world.sectors[1].tree.axis = size_4 >= (double)size;
    cm_world.sectors[1].tree.dist = (cm_world.maxs[size_4 >= (double)size] + cm_world.mins[size_4 >= (double)size]) * 0.5;
    if (cm_world.sectors[1].tree.child[0])
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 168, 0, "%s", "!cm_world.sectors[SECTOR_HEAD].tree.child[0]");
    if (cm_world.sectors[1].tree.child[1])
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 169, 0, "%s", "!cm_world.sectors[SECTOR_HEAD].tree.child[1]");
}

void __cdecl CM_UnlinkEntity(svEntity_s *ent)
{
    gentity_s *i; // eax
    worldSector_s *node; // [esp+0h] [ebp-18h]
    int contents; // [esp+4h] [ebp-14h]
    unsigned __int16 nodeIndex; // [esp+8h] [ebp-10h]
    svEntity_s *scan; // [esp+Ch] [ebp-Ch]
    svEntity_s *scana; // [esp+Ch] [ebp-Ch]
    int linkcontents; // [esp+10h] [ebp-8h]
    unsigned __int16 parentNodeIndex; // [esp+14h] [ebp-4h]
    unsigned __int16 parentNodeIndexa; // [esp+14h] [ebp-4h]

    nodeIndex = ent->worldSector;
    if (ent->worldSector)
    {
        node = &cm_world.sectors[nodeIndex];
        ent->worldSector = 0;
        if (!node->contents.entities)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 478, 0, "%s", "node->contents.entities");
        if (&sv.svEntities[node->contents.entities - 1] == ent)
        {
            node->contents.entities = ent->nextEntityInWorldSector;
        }
        else
        {
            for (scan = &sv.svEntities[node->contents.entities - 1];
                &sv.svEntities[scan->nextEntityInWorldSector - 1] != ent;
                scan = &sv.svEntities[scan->nextEntityInWorldSector - 1])
            {
                if (!scan->nextEntityInWorldSector)
                    MyAssertHandler(".\\qcommon\\cm_world.cpp", 493, 0, "%s", "scan->nextEntityInWorldSector");
            }
            scan->nextEntityInWorldSector = ent->nextEntityInWorldSector;
        }
        while (!node->contents.entities && !node->contents.staticModels && !node->tree.child[0] && !node->tree.child[1])
        {
            if (node->contents.contentsStaticModels)
                MyAssertHandler(".\\qcommon\\cm_world.cpp", 499, 0, "%s", "!node->contents.contentsStaticModels");
            node->contents.contentsEntities = 0;
            node->contents.linkcontentsEntities = 0;
            if (!node->tree.u.parent)
            {
                if (nodeIndex != 1)
                    MyAssertHandler(".\\qcommon\\cm_world.cpp", 505, 0, "%s", "nodeIndex == SECTOR_HEAD");
                goto LABEL_28;
            }
            parentNodeIndex = node->tree.u.parent;
            node->tree.u.parent = cm_world.freeHead;
            cm_world.freeHead = nodeIndex;
            node = &cm_world.sectors[parentNodeIndex];
            if (node->tree.child[0] == nodeIndex)
            {
                node->tree.child[0] = 0;
            }
            else
            {
                if (node->tree.child[1] != nodeIndex)
                    MyAssertHandler(".\\qcommon\\cm_world.cpp", 522, 0, "%s", "node->tree.child[1] == nodeIndex");
                node->tree.child[1] = 0;
            }
            nodeIndex = parentNodeIndex;
        }
        while (1)
        {
        LABEL_28:
            contents = cm_world.sectors[node->tree.child[1]].contents.contentsEntities
                | cm_world.sectors[node->tree.child[0]].contents.contentsEntities;
            linkcontents = cm_world.sectors[node->tree.child[1]].contents.linkcontentsEntities
                | cm_world.sectors[node->tree.child[0]].contents.linkcontentsEntities;
            if (node->contents.entities)
            {
                scana = &sv.svEntities[node->contents.entities - 1];
                for (i = SV_GEntityForSvEntity(scana); ; i = SV_GEntityForSvEntity(scana))
                {
                    contents |= i->r.contents;
                    linkcontents |= scana->linkcontents;
                    if (!scana->nextEntityInWorldSector)
                        break;
                    scana = &sv.svEntities[scana->nextEntityInWorldSector - 1];
                }
            }
            node->contents.contentsEntities = contents;
            node->contents.linkcontentsEntities = linkcontents;
            parentNodeIndexa = node->tree.u.parent;
            if (!parentNodeIndexa)
                break;
            node = &cm_world.sectors[parentNodeIndexa];
        }
    }
}

void __cdecl CM_LinkEntity(svEntity_s *ent, float *absmin, float *absmax, unsigned int clipHandle)
{
    bool v4; // zf
    worldSector_s *node; // [esp+10h] [ebp-30h]
    int contents; // [esp+14h] [ebp-2Ch]
    unsigned __int16 nodeIndex; // [esp+18h] [ebp-28h]
    float dist; // [esp+1Ch] [ebp-24h]
    float mins[2]; // [esp+20h] [ebp-20h] BYREF
    cmodel_t *cmod; // [esp+28h] [ebp-18h]
    cLeaf_t *leaf; // [esp+2Ch] [ebp-14h]
    float maxs[2]; // [esp+30h] [ebp-10h] BYREF
    int axis; // [esp+38h] [ebp-8h]
    int linkcontents; // [esp+3Ch] [ebp-4h]

    cmod = CM_ClipHandleToModel(clipHandle);
    leaf = &cmod->leaf;
    contents = SV_GEntityForSvEntity(ent)->r.contents;
    if (!contents)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 761, 0, "%s", "contents");
    v4 = *(_QWORD *)&leaf->brushContents == 0;
    linkcontents = leaf->terrainContents | leaf->brushContents;
    if (v4)
    {
        CM_UnlinkEntity(ent);
    }
    else
    {
        while (1)
        {
            mins[0] = cm_world.mins[0];
            mins[1] = cm_world.mins[1];
            maxs[0] = cm_world.maxs[0];
            maxs[1] = cm_world.maxs[1];
            for (nodeIndex = 1; ; nodeIndex = node->tree.child[1])
            {
                while (1)
                {
                    cm_world.sectors[nodeIndex].contents.contentsEntities |= contents;
                    cm_world.sectors[nodeIndex].contents.linkcontentsEntities |= linkcontents;
                    node = &cm_world.sectors[nodeIndex];
                    axis = node->tree.axis;
                    dist = node->tree.dist;
                    if (dist >= (double)absmin[axis])
                        break;
                    mins[axis] = dist;
                    if (!node->tree.child[0])
                        goto LABEL_17;
                    nodeIndex = node->tree.child[0];
                }
                if (dist <= (double)absmax[axis])
                    break;
                maxs[axis] = dist;
                if (!node->tree.child[1])
                    goto LABEL_17;
            }
            if (nodeIndex == ent->worldSector && (ent->linkcontents & ~linkcontents) == 0)
            {
                ent->linkcontents = linkcontents;
                *(double *)ent->linkmin = *(double *)absmin;
                *(double *)ent->linkmax = *(double *)absmax;
                return;
            }
        LABEL_17:
            if (!ent->worldSector)
                break;
            if (nodeIndex == ent->worldSector && (ent->linkcontents & ~linkcontents) == 0)
                goto LABEL_23;
            CM_UnlinkEntity(ent);
        }
        CM_AddEntityToNode(ent, nodeIndex);
    LABEL_23:
        ent->linkcontents = linkcontents;
        *(double *)ent->linkmin = *(double *)absmin;
        *(double *)ent->linkmax = *(double *)absmax;
        CM_SortNode(nodeIndex, mins, maxs);
    }
}

void __cdecl CM_AddEntityToNode(svEntity_s *ent, unsigned __int16 childNodeIndex)
{
    unsigned __int16 *prevEnt; // [esp+0h] [ebp-8h]
    unsigned int entnum; // [esp+4h] [ebp-4h]

    entnum = ent - sv.svEntities;
    for (prevEnt = &cm_world.sectors[childNodeIndex].contents.entities;
        (unsigned int)*prevEnt - 1 <= entnum;
        prevEnt = &sv.configstrings[188 * *prevEnt + 2256])
    {
        ;
    }
    ent->worldSector = childNodeIndex;
    ent->nextEntityInWorldSector = *prevEnt;
    *prevEnt = entnum + 1;
}

void __cdecl CM_SortNode(unsigned __int16 nodeIndex, float *mins, float *maxs)
{
    worldSector_s *node; // [esp+4h] [ebp-28h]
    unsigned __int16 modelnum; // [esp+8h] [ebp-24h]
    svEntity_s *prevEnt; // [esp+Ch] [ebp-20h]
    float dist; // [esp+10h] [ebp-1Ch]
    svEntity_s *ent; // [esp+14h] [ebp-18h]
    cStaticModel_s *staticModel; // [esp+18h] [ebp-14h]
    unsigned __int16 entnum; // [esp+1Ch] [ebp-10h]
    int axis; // [esp+20h] [ebp-Ch]
    cStaticModel_s *prevStaticModel; // [esp+24h] [ebp-8h]
    unsigned __int16 childNodeIndex; // [esp+28h] [ebp-4h]
    unsigned __int16 childNodeIndexa; // [esp+28h] [ebp-4h]

    node = &cm_world.sectors[nodeIndex];
    axis = node->tree.axis;
    dist = node->tree.dist;
    prevEnt = 0;
    entnum = node->contents.entities;
    while (entnum)
    {
        ent = &sv.svEntities[entnum - 1];
        if (dist >= (double)ent->linkmin[axis])
        {
            if (dist > (double)ent->linkmax[axis])
            {
                childNodeIndex = node->tree.child[1];
                if (childNodeIndex)
                    goto LABEL_14;
                childNodeIndex = CM_AllocWorldSector(mins, maxs);
                if (childNodeIndex)
                {
                    node->tree.child[1] = childNodeIndex;
                    cm_world.sectors[childNodeIndex].tree.u.parent = nodeIndex;
                    goto LABEL_14;
                }
            }
            goto skipEntity_1;
        }
        childNodeIndex = node->tree.child[0];
        if (childNodeIndex)
        {
        LABEL_14:
            entnum = ent->nextEntityInWorldSector;
            if (!prevEnt && &sv.svEntities[node->contents.entities - 1] != ent)
                MyAssertHandler(
                    ".\\qcommon\\cm_world.cpp",
                    671,
                    0,
                    "%s",
                    "prevEnt || (&sv.svEntities[node->contents.entities - 1] == ent)");
            if (prevEnt && &sv.svEntities[prevEnt->nextEntityInWorldSector - 1] != ent)
                MyAssertHandler(
                    ".\\qcommon\\cm_world.cpp",
                    672,
                    0,
                    "%s",
                    "!prevEnt || (&sv.svEntities[prevEnt->nextEntityInWorldSector - 1] == ent)");
            CM_AddEntityToNode(ent, childNodeIndex);
            cm_world.sectors[childNodeIndex].contents.contentsEntities |= SV_GEntityForSvEntity(ent)->r.contents;
            cm_world.sectors[childNodeIndex].contents.linkcontentsEntities |= ent->linkcontents;
            if (prevEnt)
                prevEnt->nextEntityInWorldSector = entnum;
            else
                node->contents.entities = entnum;
        }
        else
        {
            childNodeIndex = CM_AllocWorldSector(mins, maxs);
            if (childNodeIndex)
            {
                node->tree.child[0] = childNodeIndex;
                cm_world.sectors[childNodeIndex].tree.u.parent = nodeIndex;
                goto LABEL_14;
            }
        skipEntity_1:
            prevEnt = &sv.svEntities[entnum - 1];
            entnum = ent->nextEntityInWorldSector;
        }
    }
    prevStaticModel = 0;
    modelnum = node->contents.staticModels;
    while (modelnum)
    {
        staticModel = &cm.staticModelList[modelnum - 1];
        if (dist >= (double)staticModel->absmin[axis])
        {
            if (dist > (double)staticModel->absmax[axis])
            {
                childNodeIndexa = node->tree.child[1];
                if (childNodeIndexa)
                    goto LABEL_38;
                childNodeIndexa = CM_AllocWorldSector(mins, maxs);
                if (childNodeIndexa)
                {
                    node->tree.child[1] = childNodeIndexa;
                    cm_world.sectors[childNodeIndexa].tree.u.parent = nodeIndex;
                    goto LABEL_38;
                }
            }
            goto skipStaticModel;
        }
        childNodeIndexa = node->tree.child[0];
        if (childNodeIndexa)
        {
        LABEL_38:
            modelnum = staticModel->writable.nextModelInWorldSector;
            if (!prevStaticModel && &cm.staticModelList[node->contents.staticModels - 1] != staticModel)
                MyAssertHandler(
                    ".\\qcommon\\cm_world.cpp",
                    724,
                    0,
                    "%s",
                    "prevStaticModel || (&cm.staticModelList[node->contents.staticModels - 1] == staticModel)");
            if (prevStaticModel && &cm.staticModelList[prevStaticModel->writable.nextModelInWorldSector - 1] != staticModel)
                MyAssertHandler(
                    ".\\qcommon\\cm_world.cpp",
                    725,
                    0,
                    "%s",
                    "!prevStaticModel || (&cm.staticModelList[prevStaticModel->writable.nextModelInWorldSector - 1] == staticModel)");
            CM_AddStaticModelToNode(staticModel, childNodeIndexa);
            cm_world.sectors[childNodeIndexa].contents.contentsStaticModels |= XModelGetContents(staticModel->xmodel);
            if (prevStaticModel)
                prevStaticModel->writable.nextModelInWorldSector = modelnum;
            else
                node->contents.staticModels = modelnum;
        }
        else
        {
            childNodeIndexa = CM_AllocWorldSector(mins, maxs);
            if (childNodeIndexa)
            {
                node->tree.child[0] = childNodeIndexa;
                cm_world.sectors[childNodeIndexa].tree.u.parent = nodeIndex;
                goto LABEL_38;
            }
        skipStaticModel:
            prevStaticModel = staticModel;
            modelnum = staticModel->writable.nextModelInWorldSector;
        }
    }
}

unsigned __int16 __cdecl CM_AllocWorldSector(float *mins, float *maxs)
{
    worldSector_s *node; // [esp+4h] [ebp-14h]
    unsigned __int16 nodeIndex; // [esp+8h] [ebp-10h]
    float size[2]; // [esp+Ch] [ebp-Ch]
    unsigned __int16 axis; // [esp+14h] [ebp-4h]

    nodeIndex = cm_world.freeHead;
    if (!cm_world.freeHead)
        return 0;
    size[0] = *maxs - *mins;
    size[1] = maxs[1] - mins[1];
    axis = size[1] >= (double)size[0];
    if (size[size[1] >= (double)size[0]] <= 512.0)
        return 0;
    node = &cm_world.sectors[cm_world.freeHead];
    if (node->contents.contentsStaticModels)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 124, 0, "%s", "!node->contents.contentsStaticModels");
    if (node->contents.contentsEntities)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 125, 0, "%s", "!node->contents.contentsEntities");
    if (node->contents.linkcontentsEntities)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 126, 0, "%s", "!node->contents.linkcontentsEntities");
    if (node->contents.entities)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 127, 0, "%s", "!node->contents.entities");
    if (node->contents.staticModels)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 128, 0, "%s", "!node->contents.staticModels");
    cm_world.freeHead = node->tree.u.parent;
    node->tree.axis = axis;
    node->tree.dist = (maxs[axis] + mins[axis]) * 0.5;
    if (node->tree.child[0])
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 135, 0, "%s", "!node->tree.child[0]");
    if (node->tree.child[1])
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 136, 0, "%s", "!node->tree.child[1]");
    return nodeIndex;
}

void __cdecl CM_AddStaticModelToNode(cStaticModel_s *staticModel, unsigned __int16 childNodeIndex)
{
    unsigned int modelnum; // [esp+0h] [ebp-8h]
    cStaticModel_s *prevStaticModel; // [esp+4h] [ebp-4h]

    modelnum = staticModel - cm.staticModelList;
    for (prevStaticModel = (cStaticModel_s *)&cm_world.sectors[childNodeIndex].contents.staticModels;
        (unsigned int)prevStaticModel->writable.nextModelInWorldSector - 1 <= modelnum;
        prevStaticModel = &cm.staticModelList[prevStaticModel->writable.nextModelInWorldSector - 1])
    {
        ;
    }
    staticModel->writable.nextModelInWorldSector = prevStaticModel->writable.nextModelInWorldSector;
    prevStaticModel->writable.nextModelInWorldSector = modelnum + 1;
}

unsigned int CM_LinkAllStaticModels()
{
    unsigned int result; // eax
    cStaticModel_s *staticModel; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    i = 0;
    for (staticModel = cm.staticModelList; ; ++staticModel)
    {
        result = i;
        if (i >= cm.numStaticModels)
            break;
        if (!staticModel->xmodel)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 913, 0, "%s", "staticModel->xmodel");
        if (XModelGetContents(staticModel->xmodel))
            CM_LinkStaticModel(staticModel);
        ++i;
    }
    return result;
}

void __cdecl CM_LinkStaticModel(cStaticModel_s *staticModel)
{
    int contents; // [esp+0h] [ebp-24h]
    worldSector_s *node; // [esp+4h] [ebp-20h]
    unsigned __int16 nodeIndex; // [esp+8h] [ebp-1Ch]
    float dist; // [esp+Ch] [ebp-18h]
    float mins[2]; // [esp+10h] [ebp-14h] BYREF
    float maxs[2]; // [esp+18h] [ebp-Ch] BYREF
    int axis; // [esp+20h] [ebp-4h]

    contents = XModelGetContents(staticModel->xmodel);
    if (!contents)
        MyAssertHandler(".\\qcommon\\cm_world.cpp", 855, 0, "%s", "contents");
    mins[0] = cm_world.mins[0];
    mins[1] = cm_world.mins[1];
    maxs[0] = cm_world.maxs[0];
    maxs[1] = cm_world.maxs[1];
    for (nodeIndex = 1; ; nodeIndex = node->tree.child[1])
    {
        while (1)
        {
            cm_world.sectors[nodeIndex].contents.contentsStaticModels |= contents;
            node = &cm_world.sectors[nodeIndex];
            axis = node->tree.axis;
            dist = node->tree.dist;
            if (dist >= (double)staticModel->absmin[axis])
                break;
            mins[axis] = dist;
            if (!node->tree.child[0])
                goto LABEL_11;
            nodeIndex = node->tree.child[0];
        }
        if (dist <= (double)staticModel->absmax[axis])
            break;
        maxs[axis] = dist;
        if (!node->tree.child[1])
            break;
    }
LABEL_11:
    CM_AddStaticModelToNode(staticModel, nodeIndex);
    CM_SortNode(nodeIndex, mins, maxs);
}

int __cdecl CM_AreaEntities(const float *mins, const float *maxs, int *entityList, int maxcount, int contentmask)
{
    areaParms_t ap; // [esp+30h] [ebp-18h] BYREF

    PROF_SCOPED("CM_AreaEntities");

    ap.mins = mins;
    ap.maxs = maxs;
    ap.list = entityList;
    ap.count = 0;
    ap.maxcount = maxcount;
    ap.contentmask = contentmask;
    CM_AreaEntities_r(1u, &ap);

    return ap.count;
}

void __cdecl CM_AreaEntities_r(unsigned int nodeIndex, areaParms_t *ap)
{
    worldSector_s *node; // [esp+0h] [ebp-14h]
    unsigned int nextNodeIndex; // [esp+4h] [ebp-10h]
    gentity_s *gcheck; // [esp+8h] [ebp-Ch]
    unsigned int entnum; // [esp+Ch] [ebp-8h]

    while (1)
    {
        node = &cm_world.sectors[nodeIndex];
        if ((ap->contentmask & node->contents.contentsEntities) == 0)
            break;
        for (entnum = node->contents.entities; entnum; entnum = sv.configstrings[188 * entnum + 2256])
        {
            gcheck = SV_GEntityForSvEntity(&sv.svEntities[entnum - 1]);
            if ((ap->contentmask & gcheck->r.contents) != 0
                && *ap->maxs >= (double)gcheck->r.absmin[0]
                && *ap->mins <= (double)gcheck->r.absmax[0]
                && *((float *)ap->maxs + 1) >= (double)gcheck->r.absmin[1]
                && *((float *)ap->mins + 1) <= (double)gcheck->r.absmax[1]
                && *((float *)ap->maxs + 2) >= (double)gcheck->r.absmin[2]
                && *((float *)ap->mins + 2) <= (double)gcheck->r.absmax[2])
            {
                if (ap->count == ap->maxcount)
                {
                    Com_DPrintf(16, "CM_AreaEntities: MAXCOUNT\n");
                    return;
                }
                ap->list[ap->count++] = (int)(376 * (entnum - 1)) / 376;
            }
        }
        if (node->tree.dist >= (double)ap->maxs[node->tree.axis])
        {
            if (node->tree.dist <= (double)ap->mins[node->tree.axis])
                return;
            nodeIndex = node->tree.child[1];
        }
        else if (node->tree.dist <= (double)ap->mins[node->tree.axis])
        {
            nodeIndex = node->tree.child[0];
        }
        else
        {
            nextNodeIndex = node->tree.child[1];
            CM_AreaEntities_r(node->tree.child[0], ap);
            nodeIndex = nextNodeIndex;
        }
    }
}

void __cdecl CM_PointTraceStaticModels(trace_t *results, const float *start, const float *end, int contentmask)
{
    locTraceWork_t tw; // [esp+30h] [ebp-48h] BYREF
    float start_[4]; // [esp+58h] [ebp-20h] BYREF
    float end_[4]; // [esp+68h] [ebp-10h] BYREF

    PROF_SCOPED("staticModelTrace");

    tw.contents = contentmask;
    tw.extents.start[0] = *start;
    tw.extents.start[1] = start[1];
    tw.extents.start[2] = start[2];
    tw.extents.end[0] = *end;
    tw.extents.end[1] = end[1];
    tw.extents.end[2] = end[2];
    CM_CalcTraceExtents(&tw.extents);

    start_[0] = tw.extents.start[0];
    start_[1] = tw.extents.start[1];
    start_[2] = tw.extents.start[2];
    start_[3] = 0.0;
    end_[0] = tw.extents.end[0];
    end_[1] = tw.extents.end[1];
    end_[2] = tw.extents.end[2];
    end_[3] = results->fraction;
    CM_PointTraceStaticModels_r(&tw, 1u, start_, end_, results);
}

void __cdecl CM_PointTraceStaticModels_r(
    locTraceWork_t *tw,
    unsigned __int16 nodeIndex,
    const float *p1_,
    const float *p2,
    trace_t *trace)
{
    float v5; // [esp+Ch] [ebp-44h]
    float v6; // [esp+10h] [ebp-40h]
    bool side; // [esp+14h] [ebp-3Ch]
    worldSector_s *node; // [esp+18h] [ebp-38h]
    unsigned __int16 modelnum; // [esp+1Ch] [ebp-34h]
    float t1; // [esp+20h] [ebp-30h]
    float frac; // [esp+24h] [ebp-2Ch]
    float p1[4]; // [esp+28h] [ebp-28h] BYREF
    float t2; // [esp+38h] [ebp-18h]
    float mid[4]; // [esp+3Ch] [ebp-14h] BYREF
    cStaticModel_s *check; // [esp+4Ch] [ebp-4h]

    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    p1[3] = p1_[3];
    while (1)
    {
        node = &cm_world.sectors[nodeIndex];
        if ((tw->contents & node->contents.contentsStaticModels) == 0)
            break;
        for (modelnum = node->contents.staticModels; modelnum; modelnum = check->writable.nextModelInWorldSector)
        {
            check = &cm.staticModelList[modelnum - 1];
            if ((tw->contents & XModelGetContents(check->xmodel)) != 0
                && !CM_TraceBox(&tw->extents, check->absmin, check->absmax, trace->fraction))
            {
                CM_TraceStaticModel(check, trace, tw->extents.start, tw->extents.end, tw->contents);
            }
        }
        t1 = p1[node->tree.axis] - node->tree.dist;
        t2 = p2[node->tree.axis] - node->tree.dist;
        if (t1 * t2 < 0.0)
        {
            if (p1[3] >= (double)trace->fraction)
                return;
            if (t1 - t2 == 0.0)
                MyAssertHandler(".\\qcommon\\cm_world.cpp", 1082, 0, "%s", "t1 - t2");
            frac = t1 / (t1 - t2);
            if (frac < 0.0)
                MyAssertHandler(".\\qcommon\\cm_world.cpp", 1084, 0, "%s", "frac >= 0");
            if (frac > 1.0)
                MyAssertHandler(".\\qcommon\\cm_world.cpp", 1085, 0, "%s", "frac <= 1.f");
            mid[0] = (*p2 - p1[0]) * frac + p1[0];
            mid[1] = (p2[1] - p1[1]) * frac + p1[1];
            mid[2] = (p2[2] - p1[2]) * frac + p1[2];
            mid[3] = (p2[3] - p1[3]) * frac + p1[3];
            side = t2 >= 0.0;
            CM_PointTraceStaticModels_r(tw, node->tree.child[side], p1, mid, trace);
            nodeIndex = node->tree.child[1 - side];
            p1[0] = mid[0];
            p1[1] = mid[1];
            p1[2] = mid[2];
            p1[3] = mid[3];
        }
        else
        {
            v5 = t2 - t1;
            if (v5 < 0.0)
                v6 = t2;
            else
                v6 = t1;
            nodeIndex = node->tree.child[v6 < 0.0];
        }
    }
}

int __cdecl CM_PointTraceStaticModelsComplete(const float *start, const float *end, int contentmask)
{
    staticmodeltrace_t clip; // [esp+30h] [ebp-2Ch] BYREF
    int result; // [esp+58h] [ebp-4h]

    PROF_SCOPED("staticModelTrace");

    clip.contents = contentmask;
    clip.extents.start[0] = *start;
    clip.extents.start[1] = start[1];
    clip.extents.start[2] = start[2];
    clip.extents.end[0] = *end;
    clip.extents.end[1] = end[1];
    clip.extents.end[2] = end[2];
    CM_CalcTraceExtents(&clip.extents);
    result = CM_PointTraceStaticModelsComplete_r(&clip, 1u, clip.extents.start, clip.extents.end);

    return result;
}

int __cdecl CM_PointTraceStaticModelsComplete_r(
    const staticmodeltrace_t *clip,
    unsigned __int16 nodeIndex,
    const float *p1_,
    const float *p2)
{
    float v5; // [esp+Ch] [ebp-3Ch]
    float v6; // [esp+10h] [ebp-38h]
    bool side; // [esp+14h] [ebp-34h]
    worldSector_s *node; // [esp+18h] [ebp-30h]
    unsigned __int16 modelnum; // [esp+1Ch] [ebp-2Ch]
    float t1; // [esp+20h] [ebp-28h]
    float frac; // [esp+24h] [ebp-24h]
    float p1[3]; // [esp+28h] [ebp-20h] BYREF
    float t2; // [esp+34h] [ebp-14h]
    float mid[3]; // [esp+38h] [ebp-10h] BYREF
    cStaticModel_s *check; // [esp+44h] [ebp-4h]

    p1[0] = *p1_;
    p1[1] = p1_[1];
    for (p1[2] = p1_[2]; ; p1[2] = mid[2])
    {
        while (1)
        {
            node = &cm_world.sectors[nodeIndex];
            if ((clip->contents & node->contents.contentsStaticModels) == 0)
                return 1;
            for (modelnum = node->contents.staticModels; modelnum; modelnum = check->writable.nextModelInWorldSector)
            {
                check = &cm.staticModelList[modelnum - 1];
                if ((clip->contents & XModelGetContents(check->xmodel)) != 0
                    && !CM_TraceBox(&clip->extents, check->absmin, check->absmax, 1.0)
                    && !CM_TraceStaticModelComplete(check, clip->extents.start, clip->extents.end, clip->contents))
                {
                    return 0;
                }
            }
            t1 = p1[node->tree.axis] - node->tree.dist;
            t2 = p2[node->tree.axis] - node->tree.dist;
            if (t1 * t2 < 0.0)
                break;
            v5 = t2 - t1;
            if (v5 < 0.0)
                v6 = t2;
            else
                v6 = t1;
            nodeIndex = node->tree.child[v6 < 0.0];
        }
        if (t1 - t2 == 0.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1176, 0, "%s", "t1 - t2");
        frac = t1 / (t1 - t2);
        if (frac < 0.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1178, 0, "%s", "frac >= 0");
        if (frac > 1.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1179, 0, "%s", "frac <= 1.f");
        mid[0] = (*p2 - p1[0]) * frac + p1[0];
        mid[1] = (p2[1] - p1[1]) * frac + p1[1];
        mid[2] = (p2[2] - p1[2]) * frac + p1[2];
        side = t2 >= 0.0;
        if (!CM_PointTraceStaticModelsComplete_r(clip, node->tree.child[side], p1, mid))
            break;
        nodeIndex = node->tree.child[1 - side];
        p1[0] = mid[0];
        p1[1] = mid[1];
    }
    return 0;
}

void __cdecl CM_ClipMoveToEntities(moveclip_t *clip, trace_t *trace)
{
    float start[4]; // [esp+40h] [ebp-20h] BYREF
    float end[4]; // [esp+50h] [ebp-10h] BYREF

    PROF_SCOPED("CM_ClipMoveToEntities");

    if (trace->fraction > 1.0)
        MyAssertHandler(
            ".\\qcommon\\cm_world.cpp",
            1334,
            0,
            "%s\n\t(trace->fraction) = %g",
            "(trace->fraction <= 1.f)",
            trace->fraction);
    start[0] = clip->extents.start[0];
    start[1] = clip->extents.start[1];
    start[2] = clip->extents.start[2];
    end[0] = clip->extents.end[0];
    end[1] = clip->extents.end[1];
    end[2] = clip->extents.end[2];
    start[3] = 0.0;
    end[3] = trace->fraction;
    CM_ClipMoveToEntities_r(clip, 1u, start, end, trace);
}

void __cdecl CM_ClipMoveToEntities_r(
    const moveclip_t *clip,
    unsigned __int16 nodeIndex,
    const float *p1,
    const float *p2,
    trace_t *trace)
{
    float v5; // [esp+8h] [ebp-80h]
    float v6; // [esp+Ch] [ebp-7Ch]
    float v7; // [esp+10h] [ebp-78h]
    float v8; // [esp+14h] [ebp-74h]
    float v9; // [esp+1Ch] [ebp-6Ch]
    float v10; // [esp+20h] [ebp-68h]
    float v11; // [esp+24h] [ebp-64h]
    float v12; // [esp+28h] [ebp-60h]
    float v13; // [esp+2Ch] [ebp-5Ch]
    float v14; // [esp+30h] [ebp-58h]
    bool side; // [esp+34h] [ebp-54h]
    worldSector_s *node; // [esp+38h] [ebp-50h]
    float diff; // [esp+3Ch] [ebp-4Ch]
    float t1; // [esp+44h] [ebp-44h]
    float frac; // [esp+48h] [ebp-40h]
    float offset; // [esp+4Ch] [ebp-3Ch]
    float t2; // [esp+50h] [ebp-38h]
    float frac2; // [esp+54h] [ebp-34h]
    float invDist; // [esp+5Ch] [ebp-2Ch]
    unsigned int entnum; // [esp+60h] [ebp-28h]
    float mid[4]; // [esp+64h] [ebp-24h] BYREF
    svEntity_s *check; // [esp+74h] [ebp-14h]
    float p[4]; // [esp+78h] [ebp-10h] BYREF

    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while (1)
    {
        node = &cm_world.sectors[nodeIndex];
        if ((clip->contentmask & node->contents.contentsEntities) == 0
            || (clip->contentmask & node->contents.linkcontentsEntities) == 0)
        {
            break;
        }
        for (entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector)
        {
            check = &sv.svEntities[entnum - 1];
            if ((check->linkcontents & clip->contentmask) != 0)
                SV_ClipMoveToEntity(clip, check, trace);
        }
        t1 = p[node->tree.axis] - node->tree.dist;
        t2 = p2[node->tree.axis] - node->tree.dist;
        offset = clip->outerSize[node->tree.axis];
        v14 = t2 - t1;
        if (v14 < 0.0)
            v13 = p2[node->tree.axis] - node->tree.dist;
        else
            v13 = p[node->tree.axis] - node->tree.dist;
        if (offset > (double)v13)
        {
            v12 = t1 - t2;
            if (v12 < 0.0)
                v11 = p2[node->tree.axis] - node->tree.dist;
            else
                v11 = p[node->tree.axis] - node->tree.dist;
            if (v11 > -offset)
            {
                if (p[3] >= (double)trace->fraction)
                    return;
                diff = t2 - t1;
                if (diff == 0.0)
                {
                    side = 0;
                    frac = 1.0;
                    frac2 = 0.0;
                }
                else
                {
                    v10 = I_fabs(diff);
                    if (diff < 0.0)
                        v9 = p[node->tree.axis] - node->tree.dist;
                    else
                        v9 = -t1;
                    invDist = 1.0 / v10;
                    frac2 = (v9 - offset) * invDist;
                    frac = (v9 + offset) * invDist;
                    side = diff >= 0.0;
                }
                if (frac < 0.0)
                    MyAssertHandler(".\\qcommon\\cm_world.cpp", 1299, 0, "%s", "frac >= 0");
                v8 = 1.0 - frac;
                if (v8 < 0.0)
                    v7 = 1.0;
                else
                    v7 = frac;
                mid[0] = (*p2 - p[0]) * v7 + p[0];
                mid[1] = (p2[1] - p[1]) * v7 + p[1];
                mid[2] = (p2[2] - p[2]) * v7 + p[2];
                mid[3] = (p2[3] - p[3]) * v7 + p[3];
                CM_ClipMoveToEntities_r(clip, node->tree.child[side], p, mid, trace);
                if (frac2 > 1.0)
                    MyAssertHandler(".\\qcommon\\cm_world.cpp", 1309, 0, "%s\n\t(frac2) = %g", "(frac2 <= 1.f)", frac2);
                v6 = frac2 - 0.0;
                if (v6 < 0.0)
                    v5 = 0.0;
                else
                    v5 = frac2;
                p[0] = (*p2 - p[0]) * v5 + p[0];
                p[1] = (p2[1] - p[1]) * v5 + p[1];
                p[2] = (p2[2] - p[2]) * v5 + p[2];
                p[3] = (p2[3] - p[3]) * v5 + p[3];
                nodeIndex = node->tree.child[1 - side];
            }
            else
            {
                nodeIndex = node->tree.child[1];
            }
        }
        else
        {
            nodeIndex = node->tree.child[0];
        }
    }
}

int __cdecl CM_ClipSightTraceToEntities(sightclip_t *clip)
{
    int hitNum; // [esp+30h] [ebp-4h]

    PROF_SCOPED("CM_ClipSightTraceToEntities");

    hitNum = CM_ClipSightTraceToEntities_r(clip, 1u, clip->start, clip->end);
    return hitNum;
}

int __cdecl CM_ClipSightTraceToEntities_r(
    const sightclip_t *clip,
    unsigned __int16 nodeIndex,
    const float *p1,
    const float *p2)
{
    float v5; // [esp+8h] [ebp-7Ch]
    float v6; // [esp+Ch] [ebp-78h]
    float v7; // [esp+10h] [ebp-74h]
    float v8; // [esp+14h] [ebp-70h]
    float v9; // [esp+1Ch] [ebp-68h]
    float v10; // [esp+20h] [ebp-64h]
    float v11; // [esp+24h] [ebp-60h]
    float v12; // [esp+28h] [ebp-5Ch]
    float v13; // [esp+2Ch] [ebp-58h]
    float v14; // [esp+30h] [ebp-54h]
    bool side; // [esp+34h] [ebp-50h]
    worldSector_s *node; // [esp+38h] [ebp-4Ch]
    float diff; // [esp+3Ch] [ebp-48h]
    float t1; // [esp+44h] [ebp-40h]
    float frac; // [esp+48h] [ebp-3Ch]
    float offset; // [esp+4Ch] [ebp-38h]
    float t2; // [esp+50h] [ebp-34h]
    float frac2; // [esp+54h] [ebp-30h]
    float invDist; // [esp+5Ch] [ebp-28h]
    unsigned int entnum; // [esp+60h] [ebp-24h]
    int hitNum; // [esp+64h] [ebp-20h]
    int hitNuma; // [esp+64h] [ebp-20h]
    float mid[3]; // [esp+68h] [ebp-1Ch] BYREF
    svEntity_s *check; // [esp+74h] [ebp-10h]
    float p[3]; // [esp+78h] [ebp-Ch] BYREF

    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    while (1)
    {
        while (1)
        {
            while (1)
            {
                node = &cm_world.sectors[nodeIndex];
                if ((clip->contentmask & node->contents.contentsEntities) == 0)
                    return 0;
                if ((clip->contentmask & node->contents.linkcontentsEntities) == 0)
                    return 0;
                for (entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector)
                {
                    check = &sv.svEntities[entnum - 1];
                    hitNum = SV_ClipSightToEntity(clip, check);
                    if (hitNum)
                        return hitNum;
                }
                t1 = p[node->tree.axis] - node->tree.dist;
                t2 = p2[node->tree.axis] - node->tree.dist;
                offset = clip->outerSize[node->tree.axis];
                v14 = t2 - t1;
                v13 = v14 < 0.0 ? p2[node->tree.axis] - node->tree.dist : p[node->tree.axis] - node->tree.dist;
                if (offset > (double)v13)
                    break;
                nodeIndex = node->tree.child[0];
            }
            v12 = t1 - t2;
            v11 = v12 < 0.0 ? p2[node->tree.axis] - node->tree.dist : p[node->tree.axis] - node->tree.dist;
            if (v11 > -offset)
                break;
            nodeIndex = node->tree.child[1];
        }
        diff = t2 - t1;
        if (diff == 0.0)
        {
            side = 0;
            frac = 1.0;
            frac2 = 0.0;
        }
        else
        {
            v10 = I_fabs(diff);
            if (diff < 0.0)
                v9 = p[node->tree.axis] - node->tree.dist;
            else
                v9 = -t1;
            invDist = 1.0 / v10;
            frac2 = (v9 - offset) * invDist;
            frac = (v9 + offset) * invDist;
            side = diff >= 0.0;
        }
        if (frac < 0.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1422, 0, "%s", "frac >= 0");
        v8 = 1.0 - frac;
        v7 = v8 < 0.0 ? 1.0 : frac;
        mid[0] = (*p2 - p[0]) * v7 + p[0];
        mid[1] = (p2[1] - p[1]) * v7 + p[1];
        mid[2] = (p2[2] - p[2]) * v7 + p[2];
        hitNuma = CM_ClipSightTraceToEntities_r(clip, node->tree.child[side], p, mid);
        if (hitNuma)
            break;
        if (frac2 > 1.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1433, 0, "%s\n\t(frac2) = %g", "(frac2 <= 1.f)", frac2);
        v6 = frac2 - 0.0;
        if (v6 < 0.0)
            v5 = 0.0;
        else
            v5 = frac2;
        p[0] = (*p2 - p[0]) * v5 + p[0];
        p[1] = (p2[1] - p[1]) * v5 + p[1];
        p[2] = (p2[2] - p[2]) * v5 + p[2];
        nodeIndex = node->tree.child[1 - side];
    }
    return hitNuma;
}

void __cdecl CM_PointTraceToEntities(pointtrace_t *clip, trace_t *trace)
{
    float start[4]; // [esp+3Ch] [ebp-20h] BYREF
    float end[4]; // [esp+4Ch] [ebp-10h] BYREF

    PROF_SCOPED("CM_PointTraceToEntities");

    iassert(trace->fraction <= 1.f);

    start[0] = clip->extents.start[0];
    start[1] = clip->extents.start[1];
    start[2] = clip->extents.start[2];
    start[3] = 0.0;

    end[0] = clip->extents.end[0];
    end[1] = clip->extents.end[1];
    end[2] = clip->extents.end[2];
    end[3] = trace->fraction;

    CM_PointTraceToEntities_r(clip, 1u, start, end, trace);
}

void __cdecl CM_PointTraceToEntities_r(
    pointtrace_t *clip,
    unsigned __int16 nodeIndex,
    const float *p1,
    const float *p2,
    trace_t *trace)
{
    float v5; // [esp+8h] [ebp-44h]
    float v6; // [esp+Ch] [ebp-40h]
    worldSector_s *node; // [esp+14h] [ebp-38h]
    float t1; // [esp+18h] [ebp-34h]
    float frac; // [esp+1Ch] [ebp-30h]
    float t2; // [esp+20h] [ebp-2Ch]
    unsigned int entnum; // [esp+24h] [ebp-28h]
    float mid[4]; // [esp+28h] [ebp-24h] BYREF
    svEntity_s *check; // [esp+38h] [ebp-14h]
    float p[4]; // [esp+3Ch] [ebp-10h] BYREF

    p[0] = p1[0];
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];

    while (1)
    {
        node = &cm_world.sectors[nodeIndex];
        if ((clip->contentmask & node->contents.contentsEntities) == 0 || (clip->contentmask & node->contents.linkcontentsEntities) == 0)
        {
            break;
        }
        for (entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector)
        {
            check = &sv.svEntities[entnum - 1];
            SV_PointTraceToEntity(clip, check, trace);
        }
        t1 = p[node->tree.axis] - node->tree.dist;
        t2 = p2[node->tree.axis] - node->tree.dist;
        if (t1 * t2 < 0.0)
        {
            if (p[3] >= (double)trace->fraction)
                return;
            frac = t1 / (t1 - t2);

            iassert(frac >= 0);
            iassert(frac <= 1.f);

            mid[0] = (p2[0] - p[0]) * frac + p[0];
            mid[1] = (p2[1] - p[1]) * frac + p[1];
            mid[2] = (p2[2] - p[2]) * frac + p[2];
            mid[3] = (p2[3] - p[3]) * frac + p[3];
            CM_PointTraceToEntities_r(clip, node->tree.child[t2 >= 0.0], p, mid, trace);
            nodeIndex = node->tree.child[t2 < 0.0];
            p[0] = mid[0];
            p[1] = mid[1];
            p[2] = mid[2];
            p[3] = mid[3];
        }
        else
        {
            v5 = t2 - t1;
            if (v5 < 0.0)
                v6 = p2[node->tree.axis] - node->tree.dist;
            else
                v6 = p[node->tree.axis] - node->tree.dist;
            nodeIndex = node->tree.child[v6 < 0.0];
        }
    }
}

int __cdecl CM_PointSightTraceToEntities(sightpointtrace_t *clip)
{
    int hitNum; // [esp+30h] [ebp-4h]

    PROF_SCOPED("CM_PointSightTraceToEntities");

    hitNum = CM_PointSightTraceToEntities_r(clip, 1u, clip->start, clip->end);
    return hitNum;
}

int __cdecl CM_PointSightTraceToEntities_r(
    sightpointtrace_t *clip,
    unsigned __int16 nodeIndex,
    const float *p1,
    const float *p2)
{
    float v5; // [esp+8h] [ebp-34h]
    float v6; // [esp+Ch] [ebp-30h]
    worldSector_s *node; // [esp+14h] [ebp-28h]
    float t1; // [esp+18h] [ebp-24h]
    float frac; // [esp+1Ch] [ebp-20h]
    float t2; // [esp+20h] [ebp-1Ch]
    unsigned int entnum; // [esp+24h] [ebp-18h]
    int hitNum; // [esp+28h] [ebp-14h]
    int hitNuma; // [esp+28h] [ebp-14h]
    int hitNumb; // [esp+28h] [ebp-14h]
    int hitNumc; // [esp+28h] [ebp-14h]
    float mid[3]; // [esp+2Ch] [ebp-10h] BYREF
    svEntity_s *check; // [esp+38h] [ebp-4h]

    node = &cm_world.sectors[nodeIndex];
    if ((clip->contentmask & node->contents.contentsEntities) == 0)
        return 0;
    if ((clip->contentmask & node->contents.linkcontentsEntities) == 0)
        return 0;
    t1 = p1[node->tree.axis] - node->tree.dist;
    t2 = p2[node->tree.axis] - node->tree.dist;
    if (t1 * t2 < 0.0)
    {
        frac = t1 / (t1 - t2);
        if (frac < 0.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1586, 0, "%s", "frac >= 0");
        if (frac > 1.0)
            MyAssertHandler(".\\qcommon\\cm_world.cpp", 1587, 0, "%s", "frac <= 1.f");
        mid[0] = (*p2 - *p1) * frac + *p1;
        mid[1] = (p2[1] - p1[1]) * frac + p1[1];
        mid[2] = (p2[2] - p1[2]) * frac + p1[2];
        hitNuma = CM_PointSightTraceToEntities_r(clip, node->tree.child[t2 >= 0.0], p1, mid);
        if (hitNuma)
            return hitNuma;
        hitNumb = CM_PointSightTraceToEntities_r(clip, node->tree.child[t2 < 0.0], mid, p2);
        if (hitNumb)
            return hitNumb;
    }
    else
    {
        v5 = t2 - t1;
        if (v5 < 0.0)
            v6 = p2[node->tree.axis] - node->tree.dist;
        else
            v6 = p1[node->tree.axis] - node->tree.dist;
        hitNum = CM_PointSightTraceToEntities_r(clip, node->tree.child[v6 < 0.0], p1, p2);
        if (hitNum)
            return hitNum;
    }
    for (entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector)
    {
        check = &sv.svEntities[entnum - 1];
        hitNumc = SV_PointSightTraceToEntity(clip, check);
        if (hitNumc)
            return hitNumc;
    }
    return 0;
}

int CM_SaveWorld(unsigned __int8 *buf)
{
    unsigned __int8 *v1; // r30
    worldTree_s *p_tree; // r31

    if (buf)
    {
        v1 = buf + 2;
        p_tree = &cm_world.sectors[0].tree;
        *(_WORD *)buf = cm_world.freeHead;
        do
        {
            memcpy(v1, p_tree, 0xCu);
            p_tree = (worldTree_s *)((char *)p_tree + 28);
            v1 += 12;
        } while ((int)p_tree < (int)&cmd_args.localClientNum[2]);
    }
    return 12290;
}