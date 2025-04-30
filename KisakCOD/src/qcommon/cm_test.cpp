#include "qcommon.h"
#include <xanim/xanim.h>

int __cdecl CM_PointLeafnum_r(const float *p, int num)
{
    cNode_t *node; // [esp+0h] [ebp-Ch]
    cplane_s *plane; // [esp+4h] [ebp-8h]
    float d; // [esp+8h] [ebp-4h]

    while (num >= 0)
    {
        node = &cm.nodes[num];
        plane = node->plane;
        if (node->plane->type >= 3u)
            d = Vec3Dot(plane->normal, p) - plane->dist;
        else
            d = p[plane->type] - plane->dist;
        if (d >= 0.0)
            num = node->children[0];
        else
            num = node->children[1];
    }
    return -1 - num;
}

int __cdecl CM_PointLeafnum(const float *p)
{
    if (!cm.numNodes)
        MyAssertHandler(".\\qcommon\\cm_test.cpp", 37, 0, "%s", "cm.numNodes");
    return CM_PointLeafnum_r(p, 0);
}

void __cdecl CM_BoxLeafnums_r(leafList_s *ll, int nodenum)
{
    cNode_t *node; // [esp+0h] [ebp-Ch]
    int s; // [esp+8h] [ebp-4h]

    if (!cm.nodes)
        MyAssertHandler(".\\qcommon\\cm_test.cpp", 86, 0, "%s", "cm.nodes");
    if (!ll)
        MyAssertHandler(".\\qcommon\\cm_test.cpp", 87, 0, "%s", "ll");
    while (nodenum >= 0)
    {
        node = &cm.nodes[nodenum];
        s = BoxOnPlaneSide(ll->bounds[0], ll->bounds[1], node->plane);
        if (s == 1)
        {
            nodenum = node->children[0];
        }
        else
        {
            if (s != 2)
                CM_BoxLeafnums_r(ll, node->children[0]);
            nodenum = node->children[1];
        }
    }
    CM_StoreLeafs(ll, nodenum);
}

void __cdecl CM_StoreLeafs(leafList_s *ll, int nodenum)
{
    int leafNum; // [esp+0h] [ebp-4h]

    leafNum = -1 - nodenum;
    if (cm.leafs[-1 - nodenum].cluster != -1)
        ll->lastLeaf = leafNum;
    if (ll->count < ll->maxcount)
    {
        if ((unsigned __int16)leafNum != leafNum)
            MyAssertHandler(".\\qcommon\\cm_test.cpp", 68, 0, "%s", "static_cast< ushort >( leafNum ) == leafNum");
        ll->list[ll->count++] = leafNum;
    }
    else
    {
        ll->overflowed = 1;
    }
}

int __cdecl CM_BoxLeafnums(const float *mins, const float *maxs, unsigned __int16 *list, int listsize, int *lastLeaf)
{
    leafList_s ll; // [esp+4h] [ebp-2Ch] BYREF

    ll.bounds[0][0] = *mins;
    ll.bounds[0][1] = mins[1];
    ll.bounds[0][2] = mins[2];
    ll.bounds[1][0] = *maxs;
    ll.bounds[1][1] = maxs[1];
    ll.bounds[1][2] = maxs[2];
    ll.count = 0;
    ll.maxcount = listsize;
    ll.list = list;
    ll.lastLeaf = 0;
    ll.overflowed = 0;
    CM_BoxLeafnums_r(&ll, 0);
    *lastLeaf = ll.lastLeaf;
    return ll.count;
}

int __cdecl CM_PointContents(const float *p, unsigned int model)
{
    cLeaf_t *leaf; // [esp+0h] [ebp-10h]
    int i; // [esp+Ch] [ebp-4h]

    if (!cm.numNodes)
        MyAssertHandler(".\\qcommon\\cm_test.cpp", 281, 0, "%s", "cm.numNodes");
    if (model)
        leaf = &CM_ClipHandleToModel(model)->leaf;
    else
        leaf = &cm.leafs[CM_PointLeafnum_r(p, 0)];
    if (!leaf->leafBrushNode)
        return 0;
    for (i = 0; i < 3; ++i)
    {
        if (leaf->mins[i] >= (double)p[i])
            return 0;
        if (leaf->maxs[i] <= (double)p[i])
            return 0;
    }
    return CM_PointContentsLeafBrushNode_r(p, &cm.leafbrushNodes[leaf->leafBrushNode]);
}

int __cdecl CM_PointContentsLeafBrushNode_r(const float *p, cLeafBrushNode_s *node)
{
    cbrushside_t *side; // [esp+4h] [ebp-18h]
    int contents; // [esp+8h] [ebp-14h]
    int k; // [esp+Ch] [ebp-10h]
    cbrush_t *b; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    int ia; // [esp+14h] [ebp-8h]

    if (!node)
        MyAssertHandler(".\\qcommon\\cm_test.cpp", 220, 0, "%s", "node");
    contents = 0;
    while (1)
    {
        if (!node->leafBrushCount)
            goto LABEL_22;
        if (node->leafBrushCount > 0)
            break;
        contents |= CM_PointContentsLeafBrushNode_r(p, node + 1);
    LABEL_22:
        node += node->data.children.childOffset[node->data.children.dist >= (double)p[node->axis]];
    }
    for (k = 0; k < node->leafBrushCount; ++k)
    {
        b = &cm.brushes[node->data.leaf.brushes[k]];
        for (i = 0; i < 3; ++i)
        {
            if (b->mins[i] > (double)p[i] || b->maxs[i] < (double)p[i])
                goto miss;
        }
        side = b->sides;
        ia = b->numsides;
        if (ia < 0)
            MyAssertHandler(".\\qcommon\\cm_test.cpp", 244, 0, "%s", "i >= 0");
        while (ia)
        {
            if (side->plane->dist < Vec3Dot(p, side->plane->normal))
                goto miss;
            --ia;
            ++side;
        }
        contents |= b->contents;
    miss:
        ;
    }
    return contents;
}

int __cdecl CM_TransformedPointContents(const float *p, unsigned int model, const float *origin, const float *angles)
{
    float temp[3]; // [esp+0h] [ebp-3Ch] BYREF
    float axis[3][3]; // [esp+Ch] [ebp-30h] BYREF
    float p_l[3]; // [esp+30h] [ebp-Ch] BYREF

    Vec3Sub(p, origin, p_l);
    if (*angles != 0.0 || angles[1] != 0.0 || angles[2] != 0.0)
    {
        AnglesToAxis(angles, axis);
        temp[0] = p_l[0];
        temp[1] = p_l[1];
        temp[2] = p_l[2];
        p_l[0] = Vec3Dot(temp, axis[0]);
        p_l[1] = Vec3Dot(temp, axis[1]);
        p_l[2] = Vec3Dot(temp, axis[2]);
    }
    return CM_PointContents(p_l, model);
}

unsigned __int8 *__cdecl CM_ClusterPVS(int cluster)
{
    if (!cm.vised)
        return cm.visibility;
    if (cluster < 0 || cluster >= cm.numClusters)
        MyAssertHandler(
            ".\\qcommon\\cm_test.cpp",
            355,
            0,
            "%s\n\t(cluster) = %i",
            "(cluster >= 0 && cluster < cm.numClusters)",
            cluster);
    return &cm.visibility[cm.clusterBytes * cluster];
}

