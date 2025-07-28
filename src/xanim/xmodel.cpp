#include "xmodel.h"

#include "dobj.h"
#include "dobj_utils.h"

#include <qcommon/qcommon.h>
#include <qcommon/mem_track.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <database/database.h>
#include <devgui/devgui.h>
#include <physics/phys_local.h>

void __cdecl XModelPartsFree(XModelPartsLoad *modelParts)
{
    int size; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+8h] [ebp-4h]

    iassert(modelParts);

    boneNames = modelParts->boneNames;
    size = modelParts->numBones;
    for (i = 0; i < size; ++i)
        SL_RemoveRefToString(boneNames[i]);
}

bool __cdecl XModelBad(const XModel *model)
{
    iassert(model);

    if (useFastFile->current.enabled)
        return DB_IsXAssetDefault(ASSET_TYPE_XMODEL, model->name);
    else
        return model->bad;
}

XModel *__cdecl XModelPrecache(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    if (useFastFile->current.enabled)
        return XModelPrecache_FastFile(name);
    else
        return XModelPrecache_LoadObj(name, Alloc, AllocColl);
}

XModel *__cdecl XModelPrecache_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_XMODEL, name).model;
}

double __cdecl XModelGetRadius(const XModel *model)
{
    return model->radius;
}

void __cdecl XModelGetBounds(const XModel *model, float *mins, float *maxs)
{
    mins[0] = model->mins[0];
    mins[1] = model->mins[1];
    mins[2] = model->mins[2];

    maxs[0] = model->maxs[0];
    maxs[1] = model->maxs[1];
    maxs[2] = model->maxs[2];
}

int __cdecl XModelGetMemUsage(const XModel *model)
{
    return model->memUsage;
}

int __cdecl XModelTraceLine(
    const XModel *model,
    trace_t *results,
    const float *localStart,
    const float *localEnd,
    int contentmask)
{
    double v6; // st7
    double v7; // st7
    double v8; // st7
    double v9; // st7
    float v10; // [esp+Ch] [ebp-74h]
    float v11; // [esp+10h] [ebp-70h]
    float endDist; // [esp+18h] [ebp-68h]
    int j; // [esp+1Ch] [ebp-64h]
    TraceExtents boneExtents; // [esp+20h] [ebp-60h] BYREF
    float t; // [esp+44h] [ebp-3Ch]
    float delta[3]; // [esp+48h] [ebp-38h] BYREF
    float hitFrac; // [esp+54h] [ebp-2Ch]
    const XModelCollTri_s *ctri; // [esp+58h] [ebp-28h]
    float frac; // [esp+5Ch] [ebp-24h]
    int partIndex; // [esp+60h] [ebp-20h]
    float startDist; // [esp+64h] [ebp-1Ch]
    const XModelCollSurf_s *csurf; // [esp+68h] [ebp-18h]
    float hit[3]; // [esp+6Ch] [ebp-14h] BYREF
    float s; // [esp+78h] [ebp-8h]
    unsigned int i; // [esp+7Ch] [ebp-4h]

    if (model->collLod < 0)
        return -1;
    partIndex = -1;
    boneExtents.start[0] = *localStart;
    boneExtents.start[1] = localStart[1];
    boneExtents.start[2] = localStart[2];
    boneExtents.end[0] = *localEnd;
    boneExtents.end[1] = localEnd[1];
    boneExtents.end[2] = localEnd[2];
    CM_CalcTraceExtents(&boneExtents);
    Vec3Sub(boneExtents.end, boneExtents.start, delta);
    for (i = 0; i < model->numCollSurfs; ++i)
    {
        csurf = &model->collSurfs[i];
        if ((contentmask & csurf->contents) != 0 && !CM_TraceBox(&boneExtents, (float*)csurf->mins, (float *)csurf->maxs, results->fraction))
        {
            for (j = 0; j < csurf->numCollTris; ++j)
            {
                ctri = &csurf->collTris[j];
                v6 = Vec3Dot(boneExtents.end, ctri->plane);
                endDist = v6 - ctri->plane[3];
                if (endDist < 0.0)
                {
                    v7 = Vec3Dot(boneExtents.start, ctri->plane);
                    startDist = v7 - ctri->plane[3];
                    if (startDist > 0.0)
                    {
                        frac = (startDist - 0.125) / (startDist - endDist);
                        v11 = frac - 0.0;
                        v10 = v11 < 0.0 ? 0.0 : frac;
                        frac = v10;
                        if (results->fraction > (double)v10)
                        {
                            hitFrac = startDist / (startDist - endDist);
                            Vec3Mad(boneExtents.start, hitFrac, delta, hit);
                            v8 = Vec3Dot(hit, ctri->svec);
                            s = v8 - ctri->svec[3];
                            if (s >= -EQUAL_EPSILON && s <= (1.0f + EQUAL_EPSILON))
                            {
                                v9 = Vec3Dot(hit, ctri->tvec);
                                t = v9 - ctri->tvec[3];
                                if (t >= -EQUAL_EPSILON && s + t <= (1.0f + EQUAL_EPSILON))
                                {
                                    partIndex = csurf->boneIdx;
                                    results->startsolid = 0;
                                    results->allsolid = 0;
                                    results->fraction = frac;
                                    iassert(results->fraction >= 0 && results->fraction <= 1.0f);
                                    results->surfaceFlags = csurf->surfFlags;
                                    results->contents = csurf->contents;
                                    results->normal[0] = ctri->plane[0];
                                    results->normal[1] = ctri->plane[1];
                                    results->normal[2] = ctri->plane[2];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return partIndex;
}

int __cdecl XModelTraceLineAnimated(
    const DObj_s *obj,
    unsigned int modelIndex,
    int baseBoneIndex,
    trace_t *results,
    const DObjAnimMat *boneMtxList,
    float *localStart,
    float *localEnd,
    int contentmask)
{
    float v13; // [esp+Ch] [ebp-1E0h]
    float v14; // [esp+10h] [ebp-1DCh]
    int v15; // [esp+14h] [ebp-1D8h]
    const float *trans; // [esp+ACh] [ebp-140h]
    const float *v42; // [esp+B0h] [ebp-13Ch]
    float endDist; // [esp+B4h] [ebp-138h]
    int j; // [esp+B8h] [ebp-134h]
    TraceExtents boneExtents; // [esp+BCh] [ebp-130h] BYREF
    const DObjAnimMat *baseMatList; // [esp+E0h] [ebp-10Ch]
    float t; // [esp+E4h] [ebp-108h]
    float delta[3]; // [esp+E8h] [ebp-104h] BYREF
    float invBaseMat[4][3]; // [esp+F4h] [ebp-F8h] BYREF
    float hitFrac; // [esp+124h] [ebp-C8h]
    float localStart2[3]; // [esp+128h] [ebp-C4h] BYREF
    const XModelCollTri_s *ctri; // [esp+134h] [ebp-B8h]
    float frac; // [esp+138h] [ebp-B4h]
    const XModel *model; // [esp+13Ch] [ebp-B0h]
    int partIndex; // [esp+140h] [ebp-ACh]
    float startDist; // [esp+144h] [ebp-A8h]
    const XModelCollSurf_s *csurf; // [esp+148h] [ebp-A4h]
    float localEnd2[3]; // [esp+14Ch] [ebp-A0h] BYREF
    int globalBoneIndex; // [esp+158h] [ebp-94h]
    float hit[3]; // [esp+15Ch] [ebp-90h] BYREF
    float mat[4][3];
    unsigned int hidePartBits[4]; // [esp+198h] [ebp-54h] BYREF
    const DObjAnimMat *baseMat; // [esp+1A8h] [ebp-44h]
    float s; // [esp+1ACh] [ebp-40h]
    unsigned int i; // [esp+1B0h] [ebp-3Ch]
    int boneIdx; // [esp+1B4h] [ebp-38h]
    float axis[4][3]; // [esp+1B8h] [ebp-34h] BYREF
    const DObjAnimMat *boneMtx; // [esp+1E8h] [ebp-4h]

    model = obj->models[modelIndex];
    if (model->collLod < 0)
        return -1;
    partIndex = -1;
    baseMatList = XModelGetBasePose(model);
    DObjGetHidePartBits(obj, hidePartBits);
    for (i = 0; i < model->numCollSurfs; ++i)
    {
        csurf = &model->collSurfs[i];
        if ((contentmask & csurf->contents) != 0)
        {
            boneIdx = csurf->boneIdx;
            globalBoneIndex = boneIdx + baseBoneIndex;
            if ((hidePartBits[(boneIdx + baseBoneIndex) >> 5] & (0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F))) == 0)
            {
                boneMtx = &boneMtxList[boneIdx];
                baseMat = &baseMatList[boneIdx];
                if (Vec4Compare(boneMtx->quat, baseMat->quat)
                    && ((trans = baseMat->trans, v42 = boneMtx->trans, baseMat->trans[0] != boneMtx->trans[0])
                        || trans[1] != v42[1]
                        || trans[2] != v42[2]
                        ? (v15 = 0)
                        : (v15 = 1),
                        v15))
                {
                    boneExtents.start[0] = localStart[0];
                    boneExtents.start[1] = localStart[1];
                    boneExtents.start[2] = localStart[2];
                    boneExtents.end[0] = localEnd[0];
                    boneExtents.end[1] = localEnd[1];
                    boneExtents.end[2] = localEnd[2];
                }
                else
                {
                    ConvertQuatToInverseMat(baseMat, invBaseMat);
                    ConvertQuatToMat(boneMtx, mat);
                    mat[3][0] = boneMtx->trans[0];
                    mat[3][1] = boneMtx->trans[1];
                    mat[3][2] = boneMtx->trans[2];
                    MatrixMultiply43(invBaseMat, mat, axis);
                    Vec3Sub(localStart, axis[3], localStart2);
                    Vec3Sub(localEnd, axis[3], localEnd2);
                    MatrixTransposeTransformVector(localStart2, *(mat3x3 *)axis, boneExtents.start);
                    MatrixTransposeTransformVector(localEnd2, *(mat3x3 *)axis, boneExtents.end);
                }
                CM_CalcTraceExtents(&boneExtents);
                if (!CM_TraceBox(&boneExtents, (float*)csurf->mins, (float*)csurf->maxs, results->fraction))
                {
                    Vec3Sub(boneExtents.end, boneExtents.start, delta);
                    for (j = 0; j < csurf->numCollTris; ++j)
                    {
                        ctri = &csurf->collTris[j];
                        endDist = Vec3Dot(boneExtents.end, ctri->plane) - ctri->plane[3];
                        if (endDist < 0.0)
                        {
                            startDist = Vec3Dot(boneExtents.start, ctri->plane) - ctri->plane[3];
                            if (startDist > 0.0)
                            {
                                frac = (startDist - 0.125) / (startDist - endDist);
                                v14 = frac - 0.0;
                                v13 = v14 < 0.0 ? 0.0 : frac;
                                frac = v13;
                                if (results->fraction > v13)
                                {
                                    hitFrac = startDist / (startDist - endDist);
                                    Vec3Mad(boneExtents.start, hitFrac, delta, hit);
                                    s = Vec3Dot(hit, ctri->svec) - ctri->svec[3];
                                    if (s >= -EQUAL_EPSILON && s <= (1.0f + EQUAL_EPSILON))
                                    {
                                        t = Vec3Dot(hit, ctri->tvec) - ctri->tvec[3];
                                        if (t >= -EQUAL_EPSILON && s + t <= (1.0f + EQUAL_EPSILON))
                                        {
                                            partIndex = csurf->boneIdx;
                                            results->startsolid = 0;
                                            results->allsolid = 0;
                                            results->fraction = frac;
                                            iassert(results->fraction >= 0 && results->fraction <= 1.0f);
                                            results->surfaceFlags = csurf->surfFlags;
                                            results->contents = csurf->contents;
                                            Vec3Copy(ctri->plane, results->normal);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return partIndex;
}

void __cdecl XModelTraceLineAnimatedPartBits(
    const DObj_s *obj,
    unsigned int modelIndex,
    int baseBoneIndex,
    int contentmask,
    int *partBits)
{
    const XModel *model; // [esp+0h] [ebp-24h]
    const XModelCollSurf_s *csurf; // [esp+4h] [ebp-20h]
    unsigned int hidePartBits[4]; // [esp+Ch] [ebp-18h] BYREF
    unsigned int i; // [esp+1Ch] [ebp-8h]
    int boneIdx; // [esp+20h] [ebp-4h]

    model = obj->models[modelIndex];
    if (model->collLod >= 0)
    {
        DObjGetHidePartBits(obj, hidePartBits);
        for (i = 0; i < model->numCollSurfs; ++i)
        {
            csurf = &model->collSurfs[i];
            if ((contentmask & csurf->contents) != 0)
            {
                boneIdx = csurf->boneIdx;
                if ((hidePartBits[(boneIdx + baseBoneIndex) >> 5] & (0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F))) == 0)
                    partBits[(boneIdx + baseBoneIndex) >> 5] |= 0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F);
            }
        }
    }
}

void __cdecl PrefetchArray_XSurfaceCollisionNode_(const XSurfaceCollisionNode *mem, unsigned int elementCount)
{
    const unsigned __int8 *memIter; // [esp+0h] [ebp-8h]

    if (!elementCount)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 883, 0, "%s", "elementCount");
    for (memIter = (const unsigned __int8 *)((uintptr_t)mem & 0xFFFFFF80); 
        memIter <= (const unsigned __int8 *)(((uintptr_t) & mem[elementCount - 1].childCount + 1) & 0xFFFFFF80);
        memIter += 128)
        ;
}

char __cdecl XSurfaceVisitTrianglesInAabb(
    const XSurface *surface,
    unsigned int vertListIndex,
    const float *aabbMins,
    const float *aabbMaxs,
    bool(__cdecl *visitorFunc)(void *, const GfxPackedVertex **, const GfxPackedVertex **),
    void *visitorContext)
{
    XSurfaceGetTriCandidatesLocals locals; // [esp+6Ch] [ebp-2B0h] BYREF
    XRigidVertList *vertList; // [esp+318h] [ebp-4h]

    if (surface->deformed)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1093, 0, "%s", "!surface->deformed");
    if (vertListIndex >= surface->vertListCount)
        MyAssertHandler(
            ".\\xanim\\xmodel.cpp",
            1095,
            0,
            "%s\n\t(vertListIndex) = %i",
            "(vertListIndex >= 0 && vertListIndex < surface->vertListCount)",
            vertListIndex);
    vertList = &surface->vertList[vertListIndex];
    locals.tree = vertList->collisionTree;
    if (!locals.tree)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 1098, 0, "%s", "locals.tree");
    PrefetchArray_XSurfaceCollisionNode_(locals.tree->nodes, 1u);
    locals.visitorFunc = visitorFunc;
    locals.visitorContext = visitorContext;
    locals.inIndices = surface->triIndices;
    locals.inVertices0 = surface->verts0;
    XSurfaceVisitTrianglesInAabb_ConvertAabb(locals.tree, aabbMins, aabbMaxs, locals.mins, locals.maxs);
    locals.nodeQueue[0].count = 1;
    locals.nodeQueueBegin = 0;
    locals.nodeQueueEnd = 1;
    memset(&locals.leafQueueBegin, 0, 28);
    while (locals.nodeQueueBegin != locals.nodeQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessNode(&locals))
            return 0;
    }
    while (locals.leafQueueBegin != locals.leafQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessLeaf(&locals))
            return 0;
    }
    while (locals.triangleQueueBegin != locals.triangleQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessTriangles(&locals))
            return 0;
    }
    while (locals.vertexQueueBegin != locals.vertexQueueEnd)
    {
        if (!XSurfaceVisitTrianglesInAabb_ProcessVertices(&locals))
            return 0;
    }
    return 1;
}

void __cdecl XSurfaceVisitTrianglesInAabb_ConvertAabb(
    const XSurfaceCollisionTree *tree,
    const float *aabbMins,
    const float *aabbMaxs,
    int *mins,
    int *maxs)
{
    float translatedMaxs[3]; // [esp+90h] [ebp-30h] BYREF
    float translatedMins[3]; // [esp+9Ch] [ebp-24h] BYREF
    float transformedMins[3]; // [esp+A8h] [ebp-18h] BYREF
    float transformedMaxs[3]; // [esp+B4h] [ebp-Ch] BYREF

    Vec3Add(aabbMins, tree->trans, translatedMins);
    Vec3Add(aabbMaxs, tree->trans, translatedMaxs);
    Vec3Mul(translatedMins, tree->scale, transformedMins);
    Vec3Mul(translatedMaxs, tree->scale, transformedMaxs);

    static const float minfloat = -1000000.0f; // LWSS ADD
    static const float maxfloat = 1000000.0f; // LWSS ADD

    mins[0] = CLAMP((transformedMins[0] - 0.5), minfloat, maxfloat);
    mins[1] = CLAMP((transformedMins[1] - 0.5), minfloat, maxfloat);
    mins[2] = CLAMP((transformedMins[2] - 0.5), minfloat, maxfloat);

    maxs[0] = CLAMP((transformedMaxs[0] + 0.5), minfloat, maxfloat);
    maxs[1] = CLAMP((transformedMaxs[1] + 0.5), minfloat, maxfloat);
    maxs[2] = CLAMP((transformedMaxs[2] + 0.5), minfloat, maxfloat);
}

bool __cdecl XSurfaceVisitTrianglesInAabb_ProcessVertices(XSurfaceGetTriCandidatesLocals *locals)
{
    const GfxPackedVertex *verts0[3]; // [esp+4h] [ebp-10h] BYREF
    unsigned int vertIter; // [esp+10h] [ebp-4h]

    for (vertIter = 0; vertIter != 3; ++vertIter)
        verts0[vertIter] = &locals->inVertices0[locals->vertexQueue[locals->vertexQueueBegin][vertIter]];
    locals->vertexQueueBegin = ((unsigned __int8)locals->vertexQueueBegin + 1) & 3;

    return locals->visitorFunc(locals->visitorContext,verts0,verts0);
}

void __cdecl PrefetchArray_GfxPackedVertex_(const GfxPackedVertex *mem, unsigned int elementCount)
{
    const unsigned __int8 *memIter; // [esp+0h] [ebp-8h]

    if (!elementCount)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 883, 0, "%s", "elementCount");
    for (memIter = (const unsigned __int8*)((uintptr_t)mem & 0xFFFFFF80); 
        memIter <= (const unsigned __int8*)(((uintptr_t) & mem[elementCount - 1].tangent + 3) & 0xFFFFFF80);
        memIter += 128)
        ;
}
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessTriangles(XSurfaceGetTriCandidatesLocals *locals)
{
    unsigned __int16 index; // [esp+1Ch] [ebp-18h]
    unsigned int vertIter; // [esp+20h] [ebp-14h]
    unsigned int triangleIter; // [esp+24h] [ebp-10h]
    const unsigned __int16 *indexPtr; // [esp+28h] [ebp-Ch]
    unsigned int triangleCount; // [esp+2Ch] [ebp-8h]
    unsigned int triangleBegin; // [esp+30h] [ebp-4h]

    triangleBegin = locals->triangleQueue[locals->triangleQueueBegin].beginIndex;
    triangleCount = locals->triangleQueue[locals->triangleQueueBegin].count;
    locals->triangleQueueBegin = ((unsigned __int8)locals->triangleQueueBegin + 1) & 3;
    indexPtr = &locals->inIndices[3 * triangleBegin];
    for (triangleIter = 0; triangleIter != triangleCount; ++triangleIter)
    {
        for (vertIter = 0; vertIter != 3; ++vertIter)
        {
            index = *indexPtr;
            PrefetchArray_GfxPackedVertex_(&locals->inVertices0[*indexPtr++], 1u);
            locals->vertexQueue[locals->vertexQueueEnd][vertIter] = index;
        }
        locals->vertexQueueEnd = ((unsigned __int8)locals->vertexQueueEnd + 1) & 3;
        if (locals->vertexQueueBegin == locals->vertexQueueEnd && !XSurfaceVisitTrianglesInAabb_ProcessVertices(locals))
        {
            return 0;
        }
    }
    return 1;
}

void __cdecl PrefetchArray_XSurfaceCollisionLeaf_(const XSurfaceCollisionLeaf *mem, unsigned int elementCount)
{
    const unsigned __int8 *memIter; // [esp+0h] [ebp-8h]

    if (!elementCount)
        MyAssertHandler(".\\xanim\\xmodel.cpp", 883, 0, "%s", "elementCount");
    for (memIter = (const unsigned __int8*)((uintptr_t)mem & 0xFFFFFF80);
        memIter <= (const unsigned __int8*)(((uintptr_t) & mem[elementCount - 1].triangleBeginIndex + 1) & 0xFFFFFF80);
        memIter += 128)
    {
        ;
    }
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessLeaf(XSurfaceGetTriCandidatesLocals *locals)
{
    XSurfaceCollisionLeaf *leaf; // [esp+3Ch] [ebp-18h]
    unsigned int indexCount; // [esp+40h] [ebp-14h]
    int indexBeginIndex; // [esp+44h] [ebp-10h]
    unsigned int leafBeginIndex; // [esp+48h] [ebp-Ch]
    unsigned int leafIndex; // [esp+4Ch] [ebp-8h]
    unsigned int leafEndIndex; // [esp+50h] [ebp-4h]

    leafBeginIndex = locals->leafQueue[locals->leafQueueBegin].beginIndex;
    leafEndIndex = locals->leafQueue[locals->leafQueueBegin].count + leafBeginIndex;
    locals->leafQueueBegin = ((unsigned __int8)locals->leafQueueBegin + 1) & 3;
    for (leafIndex = leafBeginIndex; leafIndex != leafEndIndex; ++leafIndex)
    {
        leaf = &locals->tree->leafs[leafIndex];
        if (leaf->triangleBeginIndex < 0x8000u)
        {
            indexBeginIndex = leaf->triangleBeginIndex;
            indexCount = 1;
        }
        else
        {
            indexBeginIndex = leaf->triangleBeginIndex - 0x8000;
            indexCount = 2;
        }
        PrefetchArray_XSurfaceCollisionLeaf_(
            (const XSurfaceCollisionLeaf *)&locals->inIndices[3 * indexBeginIndex],
            3 * indexCount);
        locals->triangleQueue[locals->triangleQueueEnd].beginIndex = indexBeginIndex;
        locals->triangleQueue[locals->triangleQueueEnd].count = indexCount;
        locals->triangleQueueEnd = ((unsigned __int8)locals->triangleQueueEnd + 1) & 3;
        if (locals->triangleQueueBegin == locals->triangleQueueEnd
            && !XSurfaceVisitTrianglesInAabb_ProcessTriangles(locals))
        {
            return 0;
        }
    }
    return 1;
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessNode(XSurfaceGetTriCandidatesLocals *locals)
{
    unsigned int childBeginIndex; // [esp+48h] [ebp-1Ch]
    unsigned int childCount; // [esp+4Ch] [ebp-18h]
    unsigned int childCounta; // [esp+4Ch] [ebp-18h]
    XSurfaceCollisionNode *node; // [esp+50h] [ebp-14h]
    unsigned int nodeIndex; // [esp+58h] [ebp-Ch]
    unsigned int nodeEndIndex; // [esp+5Ch] [ebp-8h]
    unsigned int nodeBeginIndex; // [esp+60h] [ebp-4h]

    nodeBeginIndex = locals->nodeQueue[locals->nodeQueueBegin].beginIndex;
    nodeEndIndex = locals->nodeQueue[locals->nodeQueueBegin].count + nodeBeginIndex;
    locals->nodeQueueBegin = ((unsigned __int8)locals->nodeQueueBegin + 1) & 0x3F;
    for (nodeIndex = nodeBeginIndex; nodeIndex != nodeEndIndex; ++nodeIndex)
    {
        node = &locals->tree->nodes[nodeIndex];
        if (locals->maxs[0] >= node->aabb.mins[0]
            && locals->mins[0] <= node->aabb.maxs[0]
            && locals->maxs[1] >= node->aabb.mins[1]
            && locals->mins[1] <= node->aabb.maxs[1]
            && locals->maxs[2] >= node->aabb.mins[2]
            && locals->mins[2] <= node->aabb.maxs[2])
        {
            childBeginIndex = node->childBeginIndex;
            childCount = node->childCount;
            if (childCount < 0x8000)
            {
                PrefetchArray_XSurfaceCollisionNode_(&locals->tree->nodes[childBeginIndex], childCount);
                locals->nodeQueue[locals->nodeQueueEnd].beginIndex = childBeginIndex;
                locals->nodeQueue[locals->nodeQueueEnd].count = childCount;
                locals->nodeQueueEnd = ((unsigned __int8)locals->nodeQueueEnd + 1) & 0x3F;
                if (locals->nodeQueueBegin == locals->nodeQueueEnd)
                    MyAssertHandler(".\\xanim\\xmodel.cpp", 1079, 0, "%s", "locals.nodeQueueBegin != locals.nodeQueueEnd");
            }
            else
            {
                childCounta = childCount - 0x8000;
                PrefetchArray_XSurfaceCollisionLeaf_(&locals->tree->leafs[childBeginIndex], childCounta);
                locals->leafQueue[locals->leafQueueEnd].beginIndex = childBeginIndex;
                locals->leafQueue[locals->leafQueueEnd].count = childCounta;
                locals->leafQueueEnd = ((unsigned __int8)locals->leafQueueEnd + 1) & 3;
                if (locals->leafQueueBegin == locals->leafQueueEnd && !XSurfaceVisitTrianglesInAabb_ProcessLeaf(locals))
                    return 0;
            }
        }
    }
    return 1;
}

int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned int offset, unsigned __int8 *index)
{
    unsigned int numBones; // [esp+0h] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+4h] [ebp-8h]
    unsigned const __int16 *boneNames; // [esp+8h] [ebp-4h]
   
    iassert(index);

    boneNames = model->boneNames;
    numBones = model->numBones;

    iassert(numBones < DOBJ_MAX_PARTS);

    for (localBoneIndex = 0; ; ++localBoneIndex)
    {
        if (localBoneIndex >= numBones)
            return 0;
        if (name == boneNames[localBoneIndex])
            break;
    }

    *index = localBoneIndex + offset;
    iassert(*index == offset + localBoneIndex);
    return 1;
}

int __cdecl XModelGetStaticBounds(const XModel *model, mat3x3 &axis, float *mins, float *maxs)
{
    float v5; // [esp+0h] [ebp-34h]
    float v6; // [esp+4h] [ebp-30h]
    float v7; // [esp+8h] [ebp-2Ch]
    int j; // [esp+Ch] [ebp-28h]
    int k; // [esp+10h] [ebp-24h]
    const XModelCollSurf_s *csurf; // [esp+14h] [ebp-20h]
    float rotated[3]; // [esp+18h] [ebp-1Ch] BYREF
    int i; // [esp+24h] [ebp-10h]
    float corner[3]; // [esp+28h] [ebp-Ch] BYREF

    if (model->numCollSurfs)
    {

        mins[0] = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;

        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        maxs[0] = -FLT_MAX;

        for (i = 0; i < model->numCollSurfs; ++i)
        {
            csurf = &model->collSurfs[i];
            for (k = 0; k < 8; ++k)
            {
                if ((k & 1) != 0)
                    v7 = csurf->mins[0];
                else
                    v7 = csurf->maxs[0];
                corner[0] = v7;

                if ((k & 2) != 0)
                    v6 = csurf->mins[1];
                else
                    v6 = csurf->maxs[1];
                corner[1] = v6;

                if ((k & 4) != 0)
                    v5 = csurf->mins[2];
                else
                    v5 = csurf->maxs[2];
                corner[2] = v5;

                MatrixTransformVector(corner, axis, rotated);
                for (j = 0; j < 3; ++j)
                {
                    if (rotated[j] < (double)mins[j])
                        mins[j] = rotated[j];
                    if (rotated[j] > (double)maxs[j])
                        maxs[j] = rotated[j];
                }
            }
        }
        return 1;
    }
    else
    {
        iassert(!model->contents);
        return 0;
    }
}

XModel* XModelFindExisting(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_XMODEL, name).model;
}

unsigned __int16 *XModelBoneNames(XModel *model)
{
    return model->boneNames;
}

void XModelDumpInfo()
{
    // KISAKTODO
    //void *v0; // r3
    //const char *RemotePCPath; // r3
    //void *v2; // [sp+50h] [-20h] BYREF
    //char v3[16]; // [sp+58h] [-18h] BYREF
    //
    //strcpy(v3, "modelInfo.csv");
    //v0 = FS_FOpenTextFileWrite(v3);
    //v2 = v0;
    //if (v0)
    //{
    //    XModelWriteInfoHeader(v0);
    //    DB_EnumXAssets(ASSET_TYPE_XMODEL, XModelWriteInfo, &v2, 0);
    //    FS_FCloseFile(v2);
    //    if (FS_IsUsingRemotePCSharing())
    //        RemotePCPath = FS_GetRemotePCPath(0);
    //    else
    //        RemotePCPath = Sys_DefaultInstallPath();
    //    Com_Printf(18, "^7Successfully wrote model info [%s\\%s].\n", RemotePCPath, v3);
    //}
    //else
    //{
    //    Com_PrintError(1, "Could not dump model info.\n");
    //}
}