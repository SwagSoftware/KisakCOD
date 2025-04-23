#include "phys_local.h"

#include "ode/common.h"
#include "ode/collision_kernel.h"
#include <ode/objects.h>
#include <universal/assertive.h>
#include <qcommon/qcommon.h>
#include <qcommon/threads.h>

// LWSS HACK - unfk some types
#define float dReal

typedef struct dxWorld *dWorldID;
typedef struct dxSpace *dSpaceID;
typedef struct dxBody *dBodyID;
typedef struct dxGeom *dGeomID;
typedef struct dxJoint *dJointID;
typedef struct dxJointGroup *dJointGroupID;

int __cdecl Phys_GetSurfaceFlagsFromBrush(const cbrush_t *brush, unsigned int brushSideIndex)
{
    if (!brush)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 37, 0, "%s", "brush");
    if (brushSideIndex >= brush->numsides + 6)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            38,
            0,
            "brushSideIndex doesn't index brush->numsides + 6\n\t%i not in [0, %i)",
            brushSideIndex,
            brush->numsides + 6);
    if ((int)brushSideIndex >= 6)
        return cm.materials[brush->sides[brushSideIndex - 6].materialNum].surfaceFlags;
    if (brushSideIndex >= 6)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\physics\\phys_coll_local.h",
            154,
            0,
            "axialSide doesn't index 6\n\t%i not in [0, %i)",
            brushSideIndex,
            6);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\physics\\phys_coll_local.h",
            161,
            0,
            "axialSide doesn't index 6\n\t%i not in [0, %i)",
            brushSideIndex,
            6);
    }
    return cm.materials[brush->axialMaterialNum[brushSideIndex & 1][brushSideIndex >> 1]].surfaceFlags;
}

void __cdecl CM_ForEachBrushInLeafBrushNode_r(
    cLeafBrushNode_s *node,
    const float *mins,
    const float *maxs,
    bool testMask,
    int clipMask,
    void(__cdecl *f)(const cbrush_t *, void *),
    void *userData)
{
    int k; // [esp+0h] [ebp-Ch]
    cbrush_t *b; // [esp+4h] [ebp-8h]

    if (!node)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 102, 0, "%s", "node");
    while (!testMask || (clipMask & node->contents) != 0)
    {
        if (node->leafBrushCount)
        {
            if (node->leafBrushCount > 0)
            {
                for (k = 0; k < node->leafBrushCount; ++k)
                {
                    b = &cm.brushes[node->data.leaf.brushes[k]];
                    if ((!testMask || (clipMask & b->contents) != 0)
                        && *mins <= (double)b->maxs[0]
                        && mins[1] <= (double)b->maxs[1]
                        && mins[2] <= (double)b->maxs[2]
                        && *maxs >= (double)b->mins[0]
                        && maxs[1] >= (double)b->mins[1]
                        && maxs[2] >= (double)b->mins[2])
                    {
                        f(b, userData);
                    }
                }
                return;
            }
            CM_ForEachBrushInLeafBrushNode_r(node + 1, mins, maxs, testMask, clipMask, f, userData);
        }
        if (node->data.children.dist >= (double)mins[node->axis])
        {
            if (node->data.children.dist <= (double)maxs[node->axis])
                CM_ForEachBrushInLeafBrushNode_r(
                    &node[node->data.children.childOffset[0]],
                    mins,
                    maxs,
                    testMask,
                    clipMask,
                    f,
                    userData);
            node += node->data.children.childOffset[1];
        }
        else
        {
            node += node->data.children.childOffset[0];
        }
    }
}

void __cdecl CM_MeshTestGeomInLeaf(cLeaf_t *leaf, const objInfo *input, Results *results)
{
    int k; // [esp+4h] [ebp-8h]
    CollisionAabbTree *aabbTree; // [esp+8h] [ebp-4h]

    for (k = 0; k < leaf->collAabbCount; ++k)
    {
        aabbTree = &cm.aabbTrees[k + leaf->firstCollAabbIndex];
        if ((cm.materials[aabbTree->materialIndex].contentFlags & input->clipMask) != 0)
            CM_PositionGeomTestInAabbTree_r(aabbTree, input, results);
    }
}

void __cdecl CM_PositionGeomTestInAabbTree_r(CollisionAabbTree *aabbTree, const objInfo *input, Results *results)
{
    int j; // [esp+6Ch] [ebp-24h]
    int i; // [esp+70h] [ebp-20h]
    int surfaceFlags; // [esp+74h] [ebp-1Ch]
    int childIndex; // [esp+78h] [ebp-18h]
    unsigned __int16 *indices; // [esp+7Ch] [ebp-14h]
    int partitionIndex; // [esp+80h] [ebp-10h]
    CollisionAabbTree *child; // [esp+84h] [ebp-Ch]
    int checkStamp; // [esp+88h] [ebp-8h]
    CollisionPartition *partition; // [esp+8Ch] [ebp-4h]

    if (!CM_CullBox2(input, aabbTree->origin, aabbTree->halfSize))
    {
        if (aabbTree->childCount)
        {
            childIndex = 0;
            child = &cm.aabbTrees[aabbTree->u.firstChildIndex];
            while (childIndex < aabbTree->childCount)
            {
                CM_PositionGeomTestInAabbTree_r(child, input, results);
                ++childIndex;
                ++child;
            }
        }
        else
        {
            partitionIndex = aabbTree->u.firstChildIndex;
            checkStamp = SLOWORD(input->threadInfo.checkcount.global);
            if (input->threadInfo.checkcount.partitions[partitionIndex] != checkStamp)
            {
                input->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
                partition = &cm.partitions[partitionIndex];
                indices = &cm.triIndices[3 * partition->firstTri];
                surfaceFlags = cm.materials[aabbTree->materialIndex].surfaceFlags;
                switch (input->type)
                {
                case PHYS_GEOM_BOX:
                    //Profile_Begin(372);
                    Phys_CollideBoxWithTriangleList(indices, cm.verts, partition->triCount, input, surfaceFlags, results);
                    //Profile_EndInternal(0);
                    break;
                case PHYS_GEOM_BRUSHMODEL:
                    //Profile_Begin(376);
                    Phys_CollideOrientedBrushModelWithTriangleList(
                        indices,
                        cm.verts,
                        partition->triCount,
                        input,
                        surfaceFlags,
                        results);
                    //Profile_EndInternal(0);
                    break;
                case PHYS_GEOM_BRUSH:
                    //Profile_Begin(376);
                    Phys_CollideOrientedBrushWithTriangleList(
                        input->u.brush,
                        indices,
                        cm.verts,
                        partition->triCount,
                        input,
                        surfaceFlags,
                        results);
                    //Profile_EndInternal(0);
                    break;
                case PHYS_GEOM_CYLINDER:
                    //Profile_Begin(378);
                    Phys_CollideCylinderWithTriangleList(indices, cm.verts, partition->triCount, input, surfaceFlags, results);
                    //Profile_EndInternal(0);
                    break;
                case PHYS_GEOM_CAPSULE:
                    //Profile_Begin(380);
                    Phys_CollideCapsuleWithTriangleList(indices, cm.verts, partition->triCount, input, surfaceFlags, results);
                    //Profile_EndInternal(0);
                    break;
                default:
                    break;
                }
                if (phys_drawCollisionWorld->current.enabled)
                {
                    for (i = 0; i < partition->triCount; ++i)
                    {
                        for (j = 0; j < 3; ++j)
                            CG_DebugLine(cm.verts[indices[3 * i + j]], cm.verts[indices[3 * i + (j + 1) % 3]], colorGreen, 0, 2);
                    }
                }
            }
        }
    }
}

bool __cdecl CM_CullBox2(const objInfo *input, const float *origin, const float *halfSize)
{
    float v4; // [esp+Ch] [ebp-48h]
    float v5; // [esp+10h] [ebp-44h]
    float v6; // [esp+14h] [ebp-40h]
    float centerDelta[3]; // [esp+30h] [ebp-24h] BYREF
    float size[3]; // [esp+3Ch] [ebp-18h] BYREF
    float halfBoxSize[3]; // [esp+48h] [ebp-Ch] BYREF

    Vec3Sub(input->pos, origin, centerDelta);
    Vec3Sub(input->bounds[1], input->bounds[0], size);
    Vec3Mad(halfSize, 0.5, size, halfBoxSize);
    v6 = fabs(centerDelta[0]);
    if (halfBoxSize[0] < (double)v6)
        return 1;
    v5 = fabs(centerDelta[1]);
    if (halfBoxSize[1] < (double)v5)
        return 1;
    v4 = fabs(centerDelta[2]);
    return halfBoxSize[2] < (double)v4;
}

int __cdecl dCollideWorldGeom(dxGeom *o1, dxGeom *o2, unsigned __int16 flags, dContactGeomExt *contact, int skip)
{
    dxBody *Body; // eax
    float *Rotation; // eax
    float v8; // [esp+Ch] [ebp-AE0h]
    float v9; // [esp+10h] [ebp-ADCh]
    float v10; // [esp+14h] [ebp-AD8h]
    float v11; // [esp+1Ch] [ebp-AD0h]
    float v12; // [esp+20h] [ebp-ACCh]
    float v13; // [esp+28h] [ebp-AC4h]
    float v14; // [esp+2Ch] [ebp-AC0h]
    float v15; // [esp+30h] [ebp-ABCh]
    float v16; // [esp+34h] [ebp-AB8h]
    float v17; // [esp+38h] [ebp-AB4h]
    float v18; // [esp+3Ch] [ebp-AB0h]
    float v19; // [esp+40h] [ebp-AACh]
    float v20; // [esp+44h] [ebp-AA8h]
    float v21; // [esp+48h] [ebp-AA4h]
    float v22; // [esp+4Ch] [ebp-AA0h]
    float v23; // [esp+50h] [ebp-A9Ch]
    float v24; // [esp+54h] [ebp-A98h]
    bool v25; // [esp+5Ch] [ebp-A90h]
    float v26; // [esp+60h] [ebp-A8Ch]
    float v27; // [esp+F0h] [ebp-9FCh]
    float v28; // [esp+F4h] [ebp-9F8h]
    float v29; // [esp+FCh] [ebp-9F0h]
    float halfHeight; // [esp+100h] [ebp-9ECh]
    float *v31; // [esp+140h] [ebp-9ACh]
    dxBodyInfo *Position; // [esp+144h] [ebp-9A8h]
    int k; // [esp+170h] [ebp-97Ch]
    int j; // [esp+174h] [ebp-978h]
    int c; // [esp+178h] [ebp-974h]
    int r; // [esp+17Ch] [ebp-970h]
    leafList_s ll; // [esp+180h] [ebp-96Ch] BYREF
    const float *narrowLen; // [esp+1ACh] [ebp-940h]
    BrushInfo *brushInfo; // [esp+1B0h] [ebp-93Ch]
    float rotatedCenterOfMass[3]; // [esp+1B4h] [ebp-938h] BYREF
    float radius; // [esp+1C0h] [ebp-92Ch]
    unsigned __int16 leafs[1026]; // [esp+1C4h] [ebp-928h] BYREF
    float bounds[2][3]; // [esp+9CCh] [ebp-120h] BYREF
    objInfo input; // [esp+9E4h] [ebp-108h] BYREF
    float maxs[3]; // [esp+A8Ch] [ebp-60h] BYREF
    float absR[3][3]; // [esp+A98h] [ebp-54h] BYREF
    GeomStateCylinder *cyl; // [esp+ABCh] [ebp-30h]
    float lengths[5]; // [esp+AC0h] [ebp-2Ch] BYREF
    int i; // [esp+AD4h] [ebp-18h]
    Results results; // [esp+AD8h] [ebp-14h] BYREF
    TraceThreadInfo *value; // [esp+AE8h] [ebp-4h]

    LODWORD(lengths[4]) = 1024;
    narrowLen = &phys_narrowObjMaxLength->current.value;
    //Profile_Begin(368);
    //Profile_Begin(369);
    if (skip < 44)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 374, 0, "%s", "skip >= (int)sizeof(dContactGeom)");
    if (dGeomGetClass(o1) != 15)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 375, 0, "%s", "dGeomGetClass( o1 ) == GEOM_CLASS_WORLD");
    Body = dGeomGetBody(o2);
    Position = dBodyGetPosition(Body);
    input.bodyCenter[0] = Position->pos[0];
    input.bodyCenter[1] = Position->pos[1];
    input.bodyCenter[2] = Position->pos[2];
    v31 = dGeomGetPosition(o2);
    input.pos[0] = *v31;
    input.pos[1] = v31[1];
    input.pos[2] = v31[2];
    Rotation = dGeomGetRotation(o2);
    Phys_OdeMatrix3ToAxis(Rotation, input.R);
    MatrixTranspose(input.R, input.RTransposed);
    switch (dGeomGetClass(o2))
    {
    case 1:
        input.type = PHYS_GEOM_BOX;
        dGeomBoxGetLengths(o2, lengths);
        input.u.sideExtents[0] = lengths[0];
        input.u.sideExtents[1] = lengths[1];
        input.u.sideExtents[2] = lengths[2];
        Vec3Scale(input.u.sideExtents, 0.5, input.u.sideExtents);
        for (r = 0; r < 3; ++r)
        {
            for (c = 0; c < 3; ++c)
            {
                v26 = fabs(input.R[r][c]);
                absR[r][c] = v26;
            }
        }
        MatrixTransformVector(input.u.sideExtents, absR, maxs);
        v25 = *narrowLen > (double)lengths[0] || *narrowLen > (double)lengths[1] || *narrowLen > (double)lengths[2];
        input.isNarrow = v25;
        break;
    case 11:
        input.type = PHYS_GEOM_BRUSHMODEL;
        brushInfo = (BrushInfo *)dGeomGetClassData(o2);
        if (!brushInfo->u.brushModel)
            MyAssertHandler(".\\physics\\phys_world_collision.cpp", 402, 0, "%s", "brushInfo->u.brushModel");
        if (brushInfo->u.brushModel == 4095)
            MyAssertHandler(
                ".\\physics\\phys_world_collision.cpp",
                403,
                0,
                "%s",
                "brushInfo->u.brushModel != CAPSULE_MODEL_HANDLE");
        input.u.brushModel = CM_ClipHandleToModel(brushInfo->u.brushModel);
        MatrixTransformVector(brushInfo->centerOfMass, input.R, rotatedCenterOfMass);
        Vec3Sub(input.pos, rotatedCenterOfMass, input.pos);
        maxs[0] = input.u.brushModel->radius;
        maxs[1] = maxs[0];
        maxs[2] = maxs[0];
        input.isNarrow = *narrowLen > (double)input.u.brushModel->radius;
        break;
    case 12:
        input.type = PHYS_GEOM_BRUSH;
        brushInfo = (BrushInfo *)dGeomGetClassData(o2);
        if (!brushInfo->u.brush)
            MyAssertHandler(".\\physics\\phys_world_collision.cpp", 419, 0, "%s", "brushInfo->u.brush");
        input.u.brushModel = (const cmodel_t *)brushInfo->u.brush;
        MatrixTransformVector(brushInfo->centerOfMass, input.R, rotatedCenterOfMass);
        Vec3Sub(input.pos, rotatedCenterOfMass, input.pos);
        radius = 0.0;
        v24 = fabs(input.u.brushModel->maxs[1]);
        if ((float)0.0 < (double)v24)
        {
            v23 = fabs(input.u.brushModel->maxs[1]);
            radius = v23;
        }
        v22 = fabs(input.u.brushModel->maxs[2]);
        if (radius < (double)v22)
        {
            v21 = fabs(input.u.brushModel->maxs[2]);
            radius = v21;
        }
        v20 = fabs(input.u.brushModel->radius);
        if (radius < (double)v20)
        {
            v19 = fabs(input.u.brushModel->radius);
            radius = v19;
        }
        v18 = fabs(input.u.brushModel->mins[0]);
        if (radius < (double)v18)
        {
            v17 = fabs(input.u.brushModel->mins[0]);
            radius = v17;
        }
        v16 = fabs(input.u.brushModel->mins[1]);
        if (radius < (double)v16)
        {
            v15 = fabs(input.u.brushModel->mins[1]);
            radius = v15;
        }
        v14 = fabs(input.u.brushModel->mins[2]);
        if (radius < (double)v14)
        {
            v13 = fabs(input.u.brushModel->mins[2]);
            radius = v13;
        }
        maxs[0] = radius * 1.732050776481628;
        maxs[1] = maxs[0];
        maxs[2] = maxs[0];
        input.isNarrow = *narrowLen > (double)input.u.brushModel->radius;
        break;
    case 13:
        input.type = PHYS_GEOM_CYLINDER;
        cyl = (GeomStateCylinder *)dGeomGetClassData(o2);
        input.cylDirection = cyl->direction;
        input.u.sideExtents[0] = cyl->radius;
        input.u.sideExtents[1] = cyl->radius;
        input.u.sideExtents[2] = cyl->halfHeight;
        v29 = cyl->radius;
        halfHeight = cyl->halfHeight;
        v12 = v29 - halfHeight;
        if (v12 < 0.0)
            v11 = halfHeight;
        else
            v11 = v29;
        maxs[0] = v11 * 1.414214015007019;
        maxs[1] = maxs[0];
        maxs[2] = maxs[0];
        input.isNarrow = *narrowLen > cyl->radius + cyl->radius;
        break;
    case 14:
        input.type = PHYS_GEOM_CAPSULE;
        cyl = (GeomStateCylinder *)dGeomGetClassData(o2);
        input.cylDirection = cyl->direction;
        input.u.sideExtents[0] = cyl->radius;
        input.u.sideExtents[1] = cyl->radius;
        input.u.sideExtents[2] = cyl->halfHeight;
        for (j = 0; j < 3; ++j)
        {
            for (k = 0; k < 3; ++k)
            {
                v10 = fabs(input.R[j][k]);
                absR[j][k] = v10;
            }
        }
        v27 = cyl->radius;
        v28 = v27 + v27 + cyl->halfHeight;
        v9 = v27 - v28;
        if (v9 < 0.0)
            v8 = cyl->radius + cyl->radius + cyl->halfHeight;
        else
            v8 = v27;
        maxs[0] = v8;
        maxs[1] = v8;
        maxs[2] = v8;
        input.isNarrow = *narrowLen > cyl->radius + cyl->radius;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\physics\\phys_world_collision.cpp", 484, 0, "invalid geometry type");
        break;
    }
    ll.bounds[0][0] = input.pos[0];
    ll.bounds[0][1] = input.pos[1];
    ll.bounds[0][2] = input.pos[2];
    ll.bounds[1][0] = input.pos[0];
    ll.bounds[1][1] = input.pos[1];
    ll.bounds[1][2] = input.pos[2];
    for (i = 0; i < 3; ++i)
    {
        ll.bounds[0][i] = ll.bounds[0][i] - (maxs[i] + 1.0);
        ll.bounds[1][i] = maxs[i] + 1.0 + ll.bounds[1][i];
    }
    *(_QWORD *)&input.bounds[0][0] = *(_QWORD *)&ll.bounds[0][0];
    input.bounds[0][2] = ll.bounds[0][2];
    *(_QWORD *)&input.bounds[1][0] = *(_QWORD *)&ll.bounds[1][0];
    input.bounds[1][2] = ll.bounds[1][2];
    ll.count = 0;
    ll.maxcount = 1024;
    ll.list = leafs;
    ll.lastLeaf = 0;
    ll.overflowed = 0;
    CM_BoxLeafnums_r(&ll, 0);
    if (ll.count)
    {
        input.clipMask = 0x2806C91;
        results.contacts = contact;
        results.contactCount = 0;
        results.maxContacts = flags;
        results.stride = skip;
        value = (TraceThreadInfo *)Sys_GetValue(3);
        if (!value)
            MyAssertHandler(".\\physics\\phys_world_collision.cpp", 524, 0, "%s", "value");
        ++value->checkcount.global;
        input.threadInfo = *value;
        if (!input.threadInfo.checkcount.partitions && cm.partitionCount)
            MyAssertHandler(
                ".\\physics\\phys_world_collision.cpp",
                527,
                0,
                "%s",
                "input.threadInfo.checkcount.partitions || cm.partitionCount == 0");
        //Profile_EndInternal(0);
        //Profile_Begin(370);
        Vec3Sub(input.bounds[0], input.pos, bounds[0]);
        Vec3Sub(input.bounds[1], input.pos, bounds[1]);
        input.radius = RadiusFromBounds(bounds[0], bounds[1]);
        for (i = 0; i < ll.count; ++i)
            CM_TestGeomInLeaf(&cm.leafs[leafs[i]], &input, &results);
        if (phys_collUseEntities->current.enabled)
            Phys_TestAgainstEntities(&input, &results);
        for (i = 0; i < results.contactCount; ++i)
        {
            results.contacts[i].contact.g1 = o1;
            results.contacts[i].contact.g2 = o2;
        }
        //Profile_EndInternal(0);
        //Profile_EndInternal(0);
        return results.contactCount;
    }
    else
    {
        //Profile_EndInternal(0);
        //Profile_EndInternal(0);
        return 0;
    }
}

void __cdecl CM_TestGeomInLeaf(cLeaf_t *leaf, const objInfo *input, Results *results)
{
    if ((input->clipMask & leaf->brushContents) != 0)
        CM_TestGeomInLeafBrushNode(leaf, input, results);
    if ((input->clipMask & leaf->terrainContents) != 0)
        CM_MeshTestGeomInLeaf(leaf, input, results);
}

void __cdecl CM_TestGeomInLeafBrushNode(cLeaf_t *leaf, const objInfo *input, Results *results)
{
    int i; // [esp+0h] [ebp-Ch]
    InputOutput io; // [esp+4h] [ebp-8h] BYREF

    if (!leaf->leafBrushNode)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 162, 0, "%s", "leaf->leafBrushNode");
    for (i = 0; i < 3; ++i)
    {
        if (leaf->mins[i] >= (double)input->bounds[1][i] || leaf->maxs[i] <= (double)input->bounds[0][i])
            return;
    }
    io.Input = input;
    io.Output = results;
    CM_ForEachBrushInLeafBrushNode_r(
        &cm.leafbrushNodes[leaf->leafBrushNode],
        input->bounds[0],
        input->bounds[1],
        1,
        input->clipMask,
        (void(__cdecl *)(const cbrush_t *, void *))Phys_TestGeomInBrush,
        &io);
}

void __cdecl Phys_TestGeomInBrush(const cbrush_t *brush, unsigned int *userData)
{
    Results *results; // [esp+68h] [ebp-8h]

    results = (Results *)userData[1];
    if (results->contactCount < results->maxContacts)
    {
        switch (*(unsigned int *)(*userData + 52))
        {
        case 1:
            //Profile_Begin(374);
            Phys_CollideBoxWithBrush(brush, (const objInfo *)*userData, results);
            //Profile_EndInternal(0);
            break;
        case 2:
            //Profile_Begin(375);
            Phys_CollideOrientedBrushModelWithBrush(brush, (const objInfo *)*userData, results);
            //Profile_EndInternal(0);
            break;
        case 3:
            //Profile_Begin(375);
            Phys_CollideOrientedBrushWithBrush(
                *(const cbrush_t **)(*userData + 140),
                brush,
                (const objInfo *)*userData,
                results);
            //Profile_EndInternal(0);
            break;
        case 4:
            //Profile_Begin(377);
            Phys_CollideCylinderWithBrush(brush, (const objInfo *)*userData, results);
            //Profile_EndInternal(0);
            break;
        case 5:
            //Profile_Begin(379);
            Phys_CollideCapsuleWithBrush(brush, (const objInfo *)*userData, results);
            //Profile_EndInternal(0);
            break;
        default:
            return;
        }
    }
}

void __cdecl Phys_TestAgainstEntities(const objInfo *input, Results *results)
{
    float capRadius; // [esp+14h] [ebp-10C4h]
    int entityList[1024]; // [esp+28h] [ebp-10B0h] BYREF
    float v4; // [esp+1028h] [ebp-B0h]
    float out[9]; // [esp+102Ch] [ebp-ACh] BYREF
    float result[3]; // [esp+1050h] [ebp-88h] BYREF
    float sum[3]; // [esp+105Ch] [ebp-7Ch] BYREF
    float capHalfHeight; // [esp+1068h] [ebp-70h]
    float boxRot[12]; // [esp+106Ch] [ebp-6Ch] BYREF
    int i; // [esp+109Ch] [ebp-3Ch]
    gentity_s *v11; // [esp+10A0h] [ebp-38h]
    float outMatrix[12]; // [esp+10A4h] [ebp-34h] BYREF
    int v13; // [esp+10D4h] [ebp-4h]

    v13 = CM_AreaEntities(input->bounds[0], input->bounds[1], entityList, 1024, input->clipMask);
    MatrixIdentity33((float (*)[3])out);
    for (i = 0; i < v13; ++i)
    {
        v11 = &g_entities[entityList[i]];
        Vec3Avg(v11->r.absmax, v11->r.absmin, sum);
        v4 = v11->r.absmax[0] - v11->r.absmin[0];
        capHalfHeight = v11->r.absmax[2] - v11->r.absmin[2] - v4;
        Vec3Scale(input->u.sideExtents, 2.0, result);
        Phys_AxisToOdeMatrix3((const float (*)[3])out, outMatrix);
        Phys_AxisToOdeMatrix3(input->R, boxRot);
        capRadius = v4 * 0.5;
        results->contactCount += ODE_CollideCapsuleBox(
            input->pos,
            boxRot,
            result,
            sum,
            outMatrix,
            capRadius,
            capHalfHeight,
            results->maxContacts - results->contactCount,
            &results->contacts[results->contactCount].contact,
            results->stride);
    }
}

void __cdecl Phys_InitWorldCollision()
{
    dGeomClass gclass; // [esp+0h] [ebp-18h] BYREF
    int classID; // [esp+14h] [ebp-4h]

    gclass.bytes = 0;
    gclass.aabb_test = 0;
    gclass.isPlaceable = 0;
    gclass.collider = (int(__cdecl * (__cdecl *)(int))(dxGeom *, dxGeom *, int, dContactGeom *, int))dGetColliderWorld;
    gclass.aabb = dInfiniteAABB;
    classID = dCreateGeomClass(&gclass);
    if (classID != 15)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            576,
            0,
            "%s\n\t(classID) = %i",
            "(classID == GEOM_CLASS_WORLD)",
            classID);
    physGlob.worldGeom = Phys_GetWorldGeom();
    dInitUserGeom((dxUserGeom *)physGlob.worldGeom, 15, 0, 0);
}

int(__cdecl *__cdecl dGetColliderWorld(
    int classnum))(dxGeom *o1, dxGeom *o2, unsigned __int16 flags, dContactGeomExt *contact, int skip)
{
    if (classnum != 1 && classnum != 11 && classnum != 12 && classnum != 13 && classnum != 14)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            560,
            0,
            "%s",
            "classnum == dBoxClass || classnum == GEOM_CLASS_BRUSHMODEL || classnum == GEOM_CLASS_BRUSH || classnum == GEOM_CLA"
            "SS_CYLINDER || classnum == GEOM_CLASS_CAPSULE");
    return dCollideWorldGeom;
}

void __cdecl Phys_InitBrushmodelGeomClass()
{
    dGeomClass gclass; // [esp+0h] [ebp-18h] BYREF
    int classID; // [esp+14h] [ebp-4h]

    gclass.aabb_test = 0;
    *(unsigned int *)&gclass.isPlaceable = 1;
    gclass.collider = (int(__cdecl * (__cdecl *)(int))(dxGeom *, dxGeom *, int, dContactGeom *, int))jpeg_mem_init;
    gclass.aabb = Phys_GetBrushmodelAABB;
    gclass.bytes = 16;
    classID = dCreateGeomClass(&gclass);
    if (classID != 11)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            708,
            0,
            "%s\n\t(classID) = %i",
            "(classID == GEOM_CLASS_BRUSHMODEL)",
            classID);
}

void __cdecl Phys_GetBrushmodelAABB(dxGeom *geom, float *aabb)
{
    cmodel_t *v2; // eax
    float radius; // [esp+Ch] [ebp-18h]
    float v4; // [esp+10h] [ebp-14h]
    BrushInfo *brushInfo; // [esp+1Ch] [ebp-8h]

    if (!geom)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 588, 0, "%s", "geom");
    if (dGeomGetClass(geom) != 11)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            589,
            0,
            "%s",
            "dGeomGetClass( geom ) == GEOM_CLASS_BRUSHMODEL");
    brushInfo = (BrushInfo *)dGeomGetClassData(geom);
    if (!brushInfo->u.brushModel)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 592, 0, "%s", "brushInfo->u.brushModel");
    if (brushInfo->u.brushModel == 4095)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            593,
            0,
            "%s",
            "brushInfo->u.brushModel != CAPSULE_MODEL_HANDLE");
    v2 = CM_ClipHandleToModel(brushInfo->u.brushModel);
    v4 = -v2->radius;
    *aabb = v4;
    aabb[1] = v4;
    aabb[2] = v4;
    radius = v2->radius;
    aabb[3] = radius;
    aabb[4] = radius;
    aabb[5] = radius;
}

dColliderFn* Phys_GetColliderNull(int num)
{
    return NULL;
}

void __cdecl Phys_InitBrushGeomClass()
{
    dGeomClass gclass; // [esp+0h] [ebp-18h] BYREF
    int classID; // [esp+14h] [ebp-4h]

    gclass.aabb_test = 0;
    gclass.isPlaceable = 1;
    gclass.collider = Phys_GetColliderNull;
    gclass.aabb = Phys_GetBrushAABB;
    gclass.bytes = 16;
    classID = dCreateGeomClass(&gclass);
    if (classID != 12)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            723,
            0,
            "%s\n\t(classID) = %i",
            "(classID == GEOM_CLASS_BRUSH)",
            classID);
}

void __cdecl Phys_GetBrushAABB(dxGeom *geom, float *aabb)
{
    if (!geom)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 605, 0, "%s", "geom");
    if (dGeomGetClass(geom) != 12)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 606, 0, "%s", "dGeomGetClass( geom ) == GEOM_CLASS_BRUSH");
    *aabb = -3.4028235e38;
    aabb[1] = -3.4028235e38;
    aabb[2] = -3.4028235e38;
    aabb[3] = 3.4028235e38;
    aabb[4] = 3.4028235e38;
    aabb[5] = 3.4028235e38;
}

void __cdecl Phys_InitCylinderGeomClass()
{
    dGeomClass gclass; // [esp+0h] [ebp-18h] BYREF
    int classID; // [esp+14h] [ebp-4h]

    gclass.aabb_test = 0;
    *(unsigned int *)&gclass.isPlaceable = 1;
    gclass.collider = (int(__cdecl * (__cdecl *)(int))(dxGeom *, dxGeom *, int, dContactGeom *, int))jpeg_mem_init;
    gclass.aabb = Phys_GetCylinderAABB;
    gclass.bytes = 12;
    classID = dCreateGeomClass(&gclass);
    if (classID != 13)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            738,
            0,
            "%s\n\t(classID) = %i",
            "(classID == GEOM_CLASS_CYLINDER)",
            classID);
}

void __cdecl Phys_GetCylinderAABB(dxGeom *geom, float *aabb)
{
    float *Rotation; // eax
    float v3; // [esp+0h] [ebp-78h]
    float v4; // [esp+4h] [ebp-74h]
    float v5; // [esp+8h] [ebp-70h]
    float v6; // [esp+Ch] [ebp-6Ch]
    float v7; // [esp+14h] [ebp-64h]
    float v8; // [esp+1Ch] [ebp-5Ch]
    float *Position; // [esp+20h] [ebp-58h]
    float pos[3]; // [esp+2Ch] [ebp-4Ch]
    int axisIdx; // [esp+38h] [ebp-40h]
    float R[4][3]; // [esp+3Ch] [ebp-3Ch] BYREF
    GeomStateCylinder *cyl; // [esp+6Ch] [ebp-Ch]
    int i; // [esp+70h] [ebp-8h]
    float axisRange; // [esp+74h] [ebp-4h]

    if (!geom)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 623, 0, "%s", "geom");
    if (dGeomGetClass(geom) != 13)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            624,
            0,
            "%s",
            "dGeomGetClass( geom ) == GEOM_CLASS_CYLINDER");
    cyl = (GeomStateCylinder *)dGeomGetClassData(geom);
    Rotation = dGeomGetRotation(geom);
    Phys_OdeMatrix3ToAxis(Rotation, R);
    Position = dGeomGetPosition(geom);
    pos[0] = *Position;
    pos[1] = Position[1];
    pos[2] = Position[2];
    axisIdx = cyl->direction - 1;
    if ((unsigned int)axisIdx > 2)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            632,
            0,
            "axisIdx not in [0, 2]\n\t%i not in [%i, %i]",
            axisIdx,
            0,
            2);
    R[3][axisIdx] = cyl->halfHeight;
    R[3][(axisIdx + 1) % 3] = cyl->radius;
    R[3][(axisIdx + 2) % 3] = cyl->radius;
    for (i = 0; i < 3; ++i)
    {
        v8 = R[3][i] * R[0][axisIdx];
        v5 = fabs(v8);
        v7 = R[3][i] * R[1][axisIdx];
        v4 = fabs(v7);
        v6 = R[3][i] * R[2][axisIdx];
        v3 = fabs(v6);
        axisRange = v5 + v4 + v3;
        aabb[2 * i] = pos[i] - axisRange;
        aabb[2 * i + 1] = pos[i] + axisRange;
        axisIdx = (axisIdx + 1) % 3;
    }
}

void __cdecl Phys_InitCapsuleGeomClass()
{
    dGeomClass gclass; // [esp+0h] [ebp-18h] BYREF
    int classID; // [esp+14h] [ebp-4h]

    gclass.aabb_test = 0;
    *(unsigned int *)&gclass.isPlaceable = 1;
    gclass.collider = (int(__cdecl * (__cdecl *)(int))(dxGeom *, dxGeom *, int, dContactGeom *, int))jpeg_mem_init;
    gclass.aabb = Phys_GetCapsuleAABB;
    gclass.bytes = 12;
    classID = dCreateGeomClass(&gclass);
    if (classID != 14)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            753,
            0,
            "%s\n\t(classID) = %i",
            "(classID == GEOM_CLASS_CAPSULE)",
            classID);
}

void __cdecl Phys_GetCapsuleAABB(dxGeom *geom, float *aabb)
{
    const float *Rotation; // eax
    float v3; // [esp+0h] [ebp-78h]
    float v4; // [esp+4h] [ebp-74h]
    float v5; // [esp+8h] [ebp-70h]
    float v6; // [esp+Ch] [ebp-6Ch]
    float v7; // [esp+14h] [ebp-64h]
    float v8; // [esp+1Ch] [ebp-5Ch]
    const float *Position; // [esp+20h] [ebp-58h]
    float pos[3]; // [esp+2Ch] [ebp-4Ch]
    int axisIdx; // [esp+38h] [ebp-40h]
    float R[4][3]; // [esp+3Ch] [ebp-3Ch] BYREF
    GeomStateCylinder *cyl; // [esp+6Ch] [ebp-Ch]
    int i; // [esp+70h] [ebp-8h]
    float axisRange; // [esp+74h] [ebp-4h]

    if (!geom)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 660, 0, "%s", "geom");
    if (dGeomGetClass(geom) != 14)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 661, 0, "%s", "dGeomGetClass( geom ) == GEOM_CLASS_CAPSULE");
    cyl = (GeomStateCylinder *)dGeomGetClassData(geom);
    Rotation = dGeomGetRotation(geom);
    Phys_OdeMatrix3ToAxis(Rotation, R);
    Position = dGeomGetPosition(geom);
    pos[0] = *Position;
    pos[1] = Position[1];
    pos[2] = Position[2];
    axisIdx = cyl->direction - 1;
    if ((unsigned int)axisIdx > 2)
        MyAssertHandler(
            ".\\physics\\phys_world_collision.cpp",
            669,
            0,
            "axisIdx not in [0, 2]\n\t%i not in [%i, %i]",
            axisIdx,
            0,
            2);
    R[3][axisIdx] = cyl->radius + cyl->radius + cyl->halfHeight;
    R[3][(axisIdx + 1) % 3] = cyl->radius;
    R[3][(axisIdx + 2) % 3] = cyl->radius;
    for (i = 0; i < 3; ++i)
    {
        v8 = R[3][i] * R[0][axisIdx];
        v5 = fabs(v8);
        v7 = R[3][i] * R[1][axisIdx];
        v4 = fabs(v7);
        v6 = R[3][i] * R[2][axisIdx];
        v3 = fabs(v6);
        axisRange = v5 + v4 + v3;
        aabb[2 * i] = pos[i] - axisRange;
        aabb[2 * i + 1] = pos[i] + axisRange;
        axisIdx = (axisIdx + 1) % 3;
    }
}

dxGeom *__cdecl Phys_CreateBrushmodelGeom(
    dxSpace *space,
    dxBody *body,
    unsigned __int16 brushModel,
    const float *centerOfMass)
{
    GeomStateBrush *ClassData; // eax
    dxGeom *geom; // [esp+8h] [ebp-8h]
    const cmodel_t *cmod; // [esp+Ch] [ebp-4h]

    if (!space)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 762, 0, "%s", "space");
    if (!body)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 763, 0, "%s", "body");
    if (!brushModel)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 764, 0, "%s", "brushModel");
    if (!centerOfMass)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 765, 0, "%s", "centerOfMass");
    cmod = CM_ClipHandleToModel(brushModel);
    if (cmod->mins[0] > (double)cmod->maxs[0])
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 770, 0, "%s", "cmod->maxs[0] >= cmod->mins[0]");
    if (cmod->mins[1] > (double)cmod->maxs[1])
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 771, 0, "%s", "cmod->maxs[1] >= cmod->mins[1]");
    if (cmod->mins[2] > (double)cmod->maxs[2])
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 772, 0, "%s", "cmod->maxs[2] >= cmod->mins[2]");
    geom = ODE_CreateGeom(11, space, body);
    if (!geom)
        return 0;
    ClassData = (GeomStateBrush *)dGeomGetClassData(geom);
    ClassData->u.brushModel = brushModel;
    ClassData->momentsOfInertia[0] = *centerOfMass;
    ClassData->momentsOfInertia[1] = centerOfMass[1];
    ClassData->momentsOfInertia[2] = centerOfMass[2];
    return geom;
}

dxGeom *__cdecl Phys_CreateBrushGeom(dxSpace *space, dxBody *body, const cbrush_t *brush, const float *centerOfMass)
{
    GeomStateBrush *ClassData; // eax
    dxGeom *geom; // [esp+8h] [ebp-4h]

    if (!space)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 792, 0, "%s", "space");
    if (!body)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 793, 0, "%s", "body");
    if (!brush)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 794, 0, "%s", "brush");
    if (!centerOfMass)
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 795, 0, "%s", "centerOfMass");
    if (brush->mins[0] > (double)brush->maxs[0])
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 796, 0, "%s", "brush->maxs[0] >= brush->mins[0]");
    if (brush->mins[1] > (double)brush->maxs[1])
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 797, 0, "%s", "brush->maxs[1] >= brush->mins[1]");
    if (brush->mins[2] > (double)brush->maxs[2])
        MyAssertHandler(".\\physics\\phys_world_collision.cpp", 798, 0, "%s", "brush->maxs[2] >= brush->mins[2]");
    geom = ODE_CreateGeom(12, space, body);
    if (!geom)
        return 0;
    ClassData = (GeomStateBrush *)dGeomGetClassData(geom);
    ClassData->u.brush = brush;
    ClassData->momentsOfInertia[0] = *centerOfMass;
    ClassData->momentsOfInertia[1] = centerOfMass[1];
    ClassData->momentsOfInertia[2] = centerOfMass[2];
    return geom;
}

dxGeom *__cdecl Phys_CreateCylinderGeom(dxSpace *space, dxBody *body, const GeomStateCylinder *cyl)
{
    GeomStateCylinder *ClassData; // eax
    dxGeom *geom; // [esp+0h] [ebp-8h]

    geom = ODE_CreateGeom(13, space, body);
    if (!geom)
        return 0;
    ClassData = (GeomStateCylinder *)dGeomGetClassData(geom);
    ClassData->direction = cyl->direction;
    ClassData->radius = cyl->radius;
    ClassData->halfHeight = cyl->halfHeight;
    return geom;
}

dxGeom *__cdecl Phys_CreateCapsuleGeom(dxSpace *space, dxBody *body, const GeomStateCylinder *cyl)
{
    GeomStateCylinder *ClassData; // eax
    dxGeom *geom; // [esp+0h] [ebp-8h]

    geom = ODE_CreateGeom(14, space, body);
    if (!geom)
        return 0;
    ClassData = (GeomStateCylinder *)dGeomGetClassData(geom);
    ClassData->direction = cyl->direction;
    ClassData->radius = cyl->radius;
    ClassData->halfHeight = cyl->halfHeight;
    return geom;
}

