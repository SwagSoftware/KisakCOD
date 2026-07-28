#include <universal/q_shared.h>
#include "fx_marks.h"
#include "fx_system.h"

#include <qcommon/mem_track.h>

#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/rb_tess.h>
#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_marks.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#endif

#include <win32/win_local.h>

#include <algorithm>
#include <aim_assist/aim_assist.h>
#include <universal/profile.h>

FxMarkPoint g_fxMarkPoints[765];

static uint g_markThread[1];

void __cdecl TRACK_fx_marks()
{
    track_static_alloc_internal(g_fxMarkPoints, 24480, "g_fxMarkPoints", 8);
}

uint16_t __cdecl FX_MarkToHandle(FxMarksSystem *marksSystem, FxMark *mark)
{
    iassert(marksSystem);
    iassert(mark);

    ushort handle = mark - marksSystem->marks;

    bcassert(handle, FX_MARKS_LIMIT);

    return handle;
}

FxMark *__cdecl FX_MarkFromHandle(FxMarksSystem *marksSystem, uint16_t handle)
{
    bcassert(handle, FX_MARKS_LIMIT);
    iassert(marksSystem);

    return &marksSystem->marks[handle];
}

static FxTriGroupPool *__cdecl FX_TriGroupFromHandle(FxMarksSystem *marksSystem, uint32_t handle)
{
    bcassert(handle, FX_TRI_GROUP_LIMIT * sizeof(FxTriGroup));
    iassert(marksSystem);

    return (FxTriGroupPool *)((char *)marksSystem->triGroups + handle);
}

static FxPointGroupPool *__cdecl FX_PointGroupFromHandle(FxMarksSystem *marksSystem, uint32_t handle)
{
    bcassert(handle, FX_POINT_GROUP_LIMIT * sizeof(FxPointGroup));
    iassert(marksSystem);
    return (FxPointGroupPool *)((char *)marksSystem->pointGroups + handle);
}

static int32_t __cdecl FX_TriGroupToHandle(FxMarksSystem *marksSystem, FxTriGroup *group)
{
    iassert(marksSystem);
    iassert(group);

    uint handle = (char *)group - (char *)marksSystem->triGroups;
    bcassert(handle, FX_TRI_GROUP_LIMIT * sizeof(FxTriGroup));

    return handle;
}

static int32_t __cdecl FX_PointGroupToHandle(FxMarksSystem *marksSystem, FxPointGroup *group)
{
    iassert(marksSystem);
    iassert(group);

    uint handle = (char *)group - (char *)marksSystem->pointGroups;
    bcassert(handle, FX_POINT_GROUP_LIMIT * sizeof(FxPointGroup));
    
    return (char *)group - (char *)marksSystem->pointGroups;
}

static int32_t __cdecl FX_MarkContextsCompare(const GfxMarkContext *context0, const GfxMarkContext *context1)
{
    int type0 = context0->modelTypeAndSurf & MARK_MODEL_TYPE_MASK;
    int type1 = context1->modelTypeAndSurf & MARK_MODEL_TYPE_MASK;

    if (type0 != type1)
        return type1 - type0;
    if (context0->primaryLightIndex != context1->primaryLightIndex)
        return context1->primaryLightIndex - context0->primaryLightIndex;
    if (context0->reflectionProbeIndex != context1->reflectionProbeIndex)
        return context1->reflectionProbeIndex - context0->reflectionProbeIndex;
    if (context0->lmapIndex != context1->lmapIndex)
        return context1->lmapIndex - context0->lmapIndex;
    if (context0->modelIndex == context1->modelIndex)
        return (context1->modelTypeAndSurf & MARK_MODEL_SURF_MASK) - (context0->modelTypeAndSurf & MARK_MODEL_SURF_MASK);
    return context1->modelIndex - context0->modelIndex;
}

static bool __cdecl FX_CompareMarkTris(const FxMarkTri &tri0, const FxMarkTri &tri1)
{
    int32_t contextCompareResult; // [esp+10h] [ebp-4h]

    contextCompareResult = FX_MarkContextsCompare(&tri0.context, &tri1.context);
    if (contextCompareResult)
        return contextCompareResult > 0;
    else
        return tri0.indices[0] < tri1.indices[0];
}

void __cdecl FX_InitMarksSystem(FxMarksSystem *marksSystem)
{
    int32_t pointIndex; // [esp+8h] [ebp-10h]
    uint32_t markIndex; // [esp+Ch] [ebp-Ch]
    int32_t triIndex; // [esp+10h] [ebp-8h]

    for (uint markHandleIndex = 0; markHandleIndex != MAX_GENTITIES; ++markHandleIndex)
        marksSystem->entFirstMarkHandles[markHandleIndex] = -1;

    marksSystem->firstFreeMarkHandle = FX_MarkToHandle(marksSystem, marksSystem->marks);
    for (markIndex = 0; markIndex < (FX_MARKS_LIMIT - 1); ++markIndex)
    {
        marksSystem->marks[markIndex].prevMark = -1;
        marksSystem->marks[markIndex].nextMark = FX_MarkToHandle(marksSystem, &marksSystem->marks[markIndex + 1]);
        marksSystem->marks[markIndex].frameCountDrawn = -1;
    }
    marksSystem->marks[markIndex].prevMark = -1;
    marksSystem->marks[markIndex].nextMark = -1;
    marksSystem->marks[markIndex].frameCountDrawn = -1;

    marksSystem->firstFreeTriGroup = marksSystem->triGroups;
    for (triIndex = 0; triIndex < (FX_TRI_GROUP_LIMIT - 1); ++triIndex)
        marksSystem->triGroups[triIndex].nextFreeTriGroup = &marksSystem->triGroups[triIndex + 1];
    marksSystem->triGroups[triIndex].nextFreeTriGroup = 0;

    marksSystem->firstFreePointGroup = marksSystem->pointGroups;
    for (pointIndex = 0; pointIndex < (FX_POINT_GROUP_LIMIT - 1); ++pointIndex)
        marksSystem->pointGroups[pointIndex].nextFreePointGroup = &marksSystem->pointGroups[pointIndex + 1];
    marksSystem->pointGroups[pointIndex].nextFreePointGroup = 0;

    marksSystem->firstActiveWorldMarkHandle = -1;
    marksSystem->allocedMarkCount = 0;
    marksSystem->freedMarkCount = 0;
}

static void __cdecl FX_FreeMarkTriGroups(FxMarksSystem *marksSystem, FxMark *mark)
{
    FxTriGroupPool *group;
    uint groupHandle = mark->tris;
    do
    {
        group = FX_TriGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->triGroup.next;
        group->nextFreeTriGroup = marksSystem->firstFreeTriGroup;
        marksSystem->firstFreeTriGroup = group;
    } while (groupHandle != FX_HANDLE_NONE);
}

static void __cdecl FX_FreeMarkPointGroups(FxMarksSystem *marksSystem, FxMark *mark)
{
    FxPointGroupPool *group; 
    uint groupHandle = mark->points;
    do
    {
        group = FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->pointGroup.next;
        group->nextFreePointGroup = marksSystem->firstFreePointGroup;
        marksSystem->firstFreePointGroup = group;
    } while (groupHandle != FX_HANDLE_NONE);
}

static void __cdecl FX_FreeMarkFromList(FxMarksSystem *marksSystem, FxMark *mark, uint16_t *listHead)
{
    ushort markHandle = FX_MarkToHandle(marksSystem, mark);
    FX_FreeMarkTriGroups(marksSystem, mark);
    FX_FreeMarkPointGroups(marksSystem, mark);

    if (mark->nextMark != FX_HANDLE_NONE)
        FX_MarkFromHandle(marksSystem, mark->nextMark)->prevMark = mark->prevMark;

    if (mark->prevMark == FX_HANDLE_NONE)
    {
        if (listHead)
        {
            iassert(*listHead == markHandle);
            *listHead = mark->nextMark;
        }
    }
    else
    {
        FX_MarkFromHandle(marksSystem, mark->prevMark)->nextMark = mark->nextMark;
    }
    mark->frameCountDrawn = -1;
    mark->nextMark = marksSystem->firstFreeMarkHandle;
    marksSystem->firstFreeMarkHandle = markHandle;
}

static void __cdecl FX_FreeMark(FxMarksSystem *marksSystem, FxMark *mark)
{
    switch (mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK)
    {
    case MARK_MODEL_TYPE_WORLD_BRUSH:
        FX_FreeMarkFromList(marksSystem, mark, &marksSystem->firstActiveWorldMarkHandle);
        break;
    case MARK_MODEL_TYPE_WORLD_MODEL:
        FX_FreeMarkFromList(marksSystem, mark, 0);
        break;
    case MARK_MODEL_TYPE_ENT_BRUSH:
    case MARK_MODEL_TYPE_ENT_MODEL:
        FX_FreeMarkFromList(marksSystem, mark, &marksSystem->entFirstMarkHandles[mark->context.modelIndex]);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 226, 0, "Unhandled case.\n");
        break;
    }
    ++marksSystem->freedMarkCount;
}

static void __cdecl FX_FreeLruMark(FxMarksSystem *marksSystem)
{
    FxMark *lruMark; // [esp+34h] [ebp-Ch]
    FxMark *mark; // [esp+38h] [ebp-8h]

    PROF_SCOPED("FX_FreeLruMark");

    iassert(marksSystem);

    lruMark = 0;
    for (mark = marksSystem->marks; mark != (FxMark *)marksSystem->triGroups; ++mark)
    {
        if (mark->frameCountDrawn != -1
            && (!lruMark
                || mark->frameCountDrawn < lruMark->frameCountDrawn
                || mark->frameCountDrawn == lruMark->frameCountDrawn && mark->frameCountAlloced < lruMark->frameCountAlloced))
        {
            lruMark = mark;
        }
    }

    FX_FreeMark(marksSystem, lruMark);
}

static int32_t __cdecl FX_AllocMarkTris(FxMarksSystem *marksSystem, const FxMarkTri *markTris, int32_t triCount)
{
    int32_t groupHandle; // [esp+14h] [ebp-Ch]
    int32_t usedCount; // [esp+18h] [ebp-8h]
    FxTriGroupPool *newGroup; // [esp+1Ch] [ebp-4h]

    groupHandle = FX_HANDLE_NONE;
    do
    {
        if (!marksSystem->firstFreeTriGroup)
        {
            FX_FreeLruMark(marksSystem);
            iassert(marksSystem->firstFreeTriGroup);
        }
        newGroup = marksSystem->firstFreeTriGroup;
        marksSystem->firstFreeTriGroup = newGroup->nextFreeTriGroup;
        newGroup->triGroup.next = groupHandle;
        groupHandle = FX_TriGroupToHandle(marksSystem, (FxTriGroup *)newGroup);
        if (triCount >= 2 && !memcmp((const char *)&markTris->context, (const char *)&markTris[1].context, sizeof(GfxMarkContext)))
            usedCount = 2;
        else
            usedCount = 1;
        markTris += usedCount;
        triCount -= usedCount;
    } while (triCount);
    return groupHandle;
}

static int32_t __cdecl FX_AllocMarkPoints(FxMarksSystem *marksSystem, int32_t pointCount)
{
    int groupHandle = FX_HANDLE_NONE;
    int pointGroupCount = (pointCount + 1) / 2;

    iassert(pointGroupCount >= 1);

    do
    {
        if (!marksSystem->firstFreePointGroup)
        {
            FX_FreeLruMark(marksSystem);
            iassert(marksSystem->firstFreePointGroup);
        }
        FxPointGroupPool *newGroup = marksSystem->firstFreePointGroup;
        marksSystem->firstFreePointGroup = newGroup->nextFreePointGroup;
        newGroup->pointGroup.next = groupHandle;
        groupHandle = FX_PointGroupToHandle(marksSystem, (FxPointGroup *)newGroup);
        --pointGroupCount;
    } while (pointGroupCount);

    return groupHandle;
}

static void __cdecl FX_LinkMarkIntoList(FxMarksSystem *marksSystem, uint16_t *head, FxMark *mark)
{
    float diff[3]; // [esp+14h] [ebp-28h] BYREF
    uint16_t iterMarkPrev; // [esp+20h] [ebp-1Ch]
    float radiusSum; // [esp+24h] [ebp-18h]
    uint16_t *iterHandlePrev; // [esp+28h] [ebp-14h]
    FxMark *nextMark; // [esp+2Ch] [ebp-10h]
    FxMark *iterMark; // [esp+30h] [ebp-Ch]
    float distSq; // [esp+34h] [ebp-8h]
    uint16_t markHandle; // [esp+38h] [ebp-4h]

    iterMarkPrev = -1;
    for (iterHandlePrev = head; *iterHandlePrev != FX_HANDLE_NONE; iterHandlePrev = &iterMark->nextMark)
    {
        iterMark = FX_MarkFromHandle(marksSystem, *iterHandlePrev);
        if (iterMark->material == mark->material
            && !memcmp((const char *)&iterMark->context, (const char *)&mark->context, 6))
        {
            break;
        }
        Vec3Sub(iterMark->origin, mark->origin, diff);
        distSq = Vec3LengthSq(diff);
        radiusSum = mark->radius + iterMark->radius;
        if (distSq < radiusSum * radiusSum)
        {
            iterMarkPrev = -1;
            iterHandlePrev = head;
            break;
        }
        iterMarkPrev = *iterHandlePrev;
    }
    markHandle = FX_MarkToHandle(marksSystem, mark);
    mark->nextMark = *iterHandlePrev;
    mark->prevMark = iterMarkPrev;
    if (mark->nextMark != FX_HANDLE_NONE)
    {
        nextMark = FX_MarkFromHandle(marksSystem, mark->nextMark);
        if (nextMark->prevMark != mark->prevMark)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 364, 0, "%s", "nextMark->prevMark == mark->prevMark");
        nextMark->prevMark = markHandle;
    }
    *iterHandlePrev = markHandle;
}

static void __cdecl FX_CopyMarkTris(
    FxMarksSystem *marksSystem,
    const FxMarkTri *srcTris,
    uint32_t dstGroupHandle,
    int32_t triCount)
{
    int32_t copyCount; // [esp+10h] [ebp-14h]
    int32_t copyIndex; // [esp+1Ch] [ebp-8h]
    FxTriGroupPool *dstGroup; // [esp+20h] [ebp-4h]

    do
    {
        dstGroup = FX_TriGroupFromHandle(marksSystem, dstGroupHandle);
        if (triCount < 2)
            copyCount = triCount;
        else
            copyCount = 2;
        copyIndex = 0;
        dstGroup->triGroup.context = srcTris->context;
        do
        {
            dstGroup->triGroup.indices[copyIndex][0] = srcTris[copyIndex].indices[0];
            dstGroup->triGroup.indices[copyIndex][1] = srcTris[copyIndex].indices[1];
            dstGroup->triGroup.indices[copyIndex][2] = srcTris[copyIndex].indices[2];
            ++copyIndex;
        } while (copyIndex != copyCount
            && !memcmp((const char *)&srcTris[copyIndex].context, (const char *)&dstGroup->triGroup.context, 6));
        dstGroupHandle = dstGroup->triGroup.next;
        dstGroup->triGroup.triCount = copyIndex;
        srcTris += dstGroup->triGroup.triCount;
        triCount -= dstGroup->triGroup.triCount;
    } while (triCount);

    iassert(dstGroupHandle == FX_HANDLE_NONE);
}

static void __cdecl FX_CopyMarkPoints(
    FxMarksSystem *marksSystem,
    const FxMarkPoint *srcPoints,
    uint32_t dstGroupHandle,
    int32_t pointCount)
{
    int32_t copyCount; // [esp+8h] [ebp-14h]
    int32_t copyIndex; // [esp+14h] [ebp-8h]
    FxPointGroupPool *dstGroup; // [esp+18h] [ebp-4h]

    do
    {
        dstGroup = FX_PointGroupFromHandle(marksSystem, dstGroupHandle);
        if (pointCount > 2)
            copyCount = 2;
        else
            copyCount = pointCount;

        iassert(copyCount > 0);
        copyIndex = 0;
        do
        {
            memcpy((char *)dstGroup + 32 * copyIndex, &srcPoints[copyIndex], 0x20u);
            ++copyIndex;
        } while (copyIndex < copyCount);
        dstGroupHandle = dstGroup->pointGroup.next;
        srcPoints += copyCount;
        pointCount -= copyCount;
    } while (pointCount);

    iassert(dstGroupHandle == FX_HANDLE_NONE);
}

static uint16_t __cdecl FX_FindModelHead(FxMarksSystem *marksSystem, uint16_t modelIndex, int32_t type)
{
    for (FxMark *mark = marksSystem->marks; mark != (FxMark *)marksSystem->triGroups; ++mark)
    {
        if (mark->frameCountDrawn != -1
            && mark->prevMark == FX_HANDLE_NONE
            && (mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == type
            && mark->context.modelIndex == modelIndex)
        {
            return FX_MarkToHandle(marksSystem, mark);
        }
    }
    return FX_MARK_FREE;
}

static void __cdecl FX_AllocAndConstructMark(
    int32_t localClientNum,
    int32_t triCount,
    int32_t pointCount,
    Material *material,
    FxMarkTri *markTris,
    const FxMarkPoint *markPoints,
    const float *origin,
    float radius,
    const float *texCoordAxis,
    const uint8_t *nativeColor)
{
    uint16_t staticModelMarkHead; // [esp+1Ch] [ebp-1Ch] BYREF
    uint16_t newMarkHandle; // [esp+20h] [ebp-18h]
    FxMarksSystem *marksSystem; // [esp+24h] [ebp-14h]
    int32_t points; // [esp+28h] [ebp-10h]
    int32_t modelType; // [esp+2Ch] [ebp-Ch]
    FxMark *newMark; // [esp+30h] [ebp-8h]
    int32_t tris; // [esp+34h] [ebp-4h]

    std::sort(markTris, &markTris[triCount], FX_CompareMarkTris);

    Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);

    iassert(Sys_InterlockedIncrement(&g_markThread[localClientNum]) == 1);

    marksSystem = FX_GetMarksSystem(localClientNum);
    tris = FX_AllocMarkTris(marksSystem, markTris, triCount);
    points = FX_AllocMarkPoints(marksSystem, pointCount);

    if (marksSystem->firstFreeMarkHandle == FX_HANDLE_NONE)
    {
        FX_FreeLruMark(marksSystem);
        iassert(marksSystem->firstFreeMarkHandle != FX_HANDLE_NONE);
    }
    newMarkHandle = marksSystem->firstFreeMarkHandle;
    newMark = FX_MarkFromHandle(marksSystem, newMarkHandle);

    iassert(newMark);

    marksSystem->firstFreeMarkHandle = newMark->nextMark;

    iassert(marksSystem->frameCount > 0);

    newMark->context = markTris->context;
    newMark->material = material;
    newMark->radius = radius;

    Vec3Copy(origin, newMark->origin);
    Vec3Copy(texCoordAxis, newMark->texCoordAxis);

    newMark->nativeColor[0] = nativeColor[0];
    newMark->nativeColor[1] = nativeColor[1];
    newMark->nativeColor[2] = nativeColor[2];
    newMark->nativeColor[3] = nativeColor[3];

    modelType = newMark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK;
    if (modelType == MARK_MODEL_TYPE_ENT_MODEL || modelType == MARK_MODEL_TYPE_ENT_BRUSH)
    {
        FX_LinkMarkIntoList(marksSystem, &marksSystem->entFirstMarkHandles[newMark->context.modelIndex], newMark);
    }
    else if (modelType == MARK_MODEL_TYPE_WORLD_MODEL)
    {
        staticModelMarkHead = FX_FindModelHead(marksSystem, newMark->context.modelIndex, MARK_MODEL_TYPE_WORLD_MODEL);

        iassert(staticModelMarkHead != newMarkHandle);
        FX_LinkMarkIntoList(marksSystem, &staticModelMarkHead, newMark);
    }
    else
    {
        iassert(modelType == MARK_MODEL_TYPE_WORLD_BRUSH);
        FX_LinkMarkIntoList(marksSystem, &marksSystem->firstActiveWorldMarkHandle, newMark);
    }

    iassert(newMark->frameCountDrawn == FX_MARK_FREE);

    newMark->frameCountDrawn = marksSystem->frameCount - 1;
    newMark->frameCountAlloced = marksSystem->frameCount;
    newMark->tris = tris;
    newMark->triCount = triCount;

    iassert(newMark->triCount == triCount);

    newMark->points = points;
    newMark->pointCount = pointCount;

    iassert(newMark->pointCount == pointCount);
    iassert(Sys_InterlockedDecrement(&g_markThread[localClientNum]) == 0);

    Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
    FX_CopyMarkTris(marksSystem, markTris, newMark->tris, triCount);
    FX_CopyMarkPoints(marksSystem, markPoints, newMark->points, pointCount);

    marksSystem->allocedMarkCount++;
}

static void __cdecl FX_ImpactMark_Generate_Callback(
    void *context_p,
    int32_t triCount,
    FxMarkTri *tris,
    int32_t pointCount,
    FxMarkPoint *points,
    const float *markOrigin,
    const float *markTexCoordAxis)
{
    FX_ImpactMark_Generate_CB* context = (FX_ImpactMark_Generate_CB *)context_p;
    FX_AllocAndConstructMark(
        context->localClientNum,
        triCount,
        pointCount,
        context->material,
        tris,
        points,
        markOrigin,
        context->radius,
        markTexCoordAxis,
        context->nativeColor);
}

static void __cdecl FX_ImpactMark_Generate_AddEntityBrush(
    int32_t localClientNum,
    MarkInfo* markInfo,
    uint32_t entityIndex,
    const float* origin,
    float radius)
{
    if (entityIndex == ENTITYNUM_NONE)
        return;

    PROF_SCOPED("FX_ImpactMark_Generate_AddEntityModels");

    float markMins[3];
    float markMaxs[3];
    Vec3AddScalar(origin, -radius, markMins);
    Vec3AddScalar(origin, radius, markMaxs);

    centity_s* ent = CG_GetEntity(localClientNum, entityIndex);
    if (!ent->nextValid || ent->nextState.solid != SOLID_BMODEL)
        return;

    GfxBrushModel* brushModel = R_GetBrushModel(ent->nextState.index.brushmodel);
    float entAxis[3][3];
    AnglesToAxis(ent->pose.angles, entAxis);

    float worldModelBounds[2][3];
    for (int32_t worldAxis = 0; worldAxis < 3; ++worldAxis)
    {
        const float firstAxisComponent = entAxis[0][worldAxis];
        const int32_t firstMinBound = firstAxisComponent >= 0.0f ? 0 : 1;
        const int32_t firstMaxBound = firstMinBound ^ 1;

        worldModelBounds[0][worldAxis] = brushModel->bounds[firstMinBound][0] * firstAxisComponent + ent->pose.origin[worldAxis];
        worldModelBounds[1][worldAxis] = brushModel->bounds[firstMaxBound][0] * firstAxisComponent + ent->pose.origin[worldAxis];

        for (int32_t modelAxis = 1; modelAxis < 3; ++modelAxis)
        {
            const float axisComponent = entAxis[modelAxis][worldAxis];
            const int32_t minBound = axisComponent >= 0.0f ? 0 : 1;
            const int32_t maxBound = minBound ^ 1;

            worldModelBounds[0][worldAxis] += brushModel->bounds[minBound][modelAxis] * axisComponent;
            worldModelBounds[1][worldAxis] += brushModel->bounds[maxBound][modelAxis] * axisComponent;
        }
    }

    if (!BoundsOverlap(markMins, markMaxs, worldModelBounds[0], worldModelBounds[1]))
        return;

    const uint16_t entityIndexAsUnsignedShort = truncate_cast<uint16_t>(entityIndex);
    R_MarkFragments_AddBModel(markInfo, brushModel, &ent->pose, entityIndexAsUnsignedShort);
}

static void __cdecl FX_ImpactMark_Generate_AddEntityModel(
    int32_t localClientNum,
    MarkInfo *markInfo,
    uint32_t entityIndex,
    const float *origin,
    float radius)
{
    float diff[8]; // [esp+38h] [ebp-38h] BYREF
    uint16_t entityIndexAsUnsignedShort; // [esp+58h] [ebp-18h]
    float dObjRadius; // [esp+5Ch] [ebp-14h]
    float summedRadiusSquared; // [esp+60h] [ebp-10h]
    centity_s *ent; // [esp+64h] [ebp-Ch]
    float summedRadius; // [esp+68h] [ebp-8h]
    DObj_s *dObj; // [esp+6Ch] [ebp-4h]

    if (entityIndex != ENTITYNUM_NONE)
    {
        PROF_SCOPED("FX_ImpactMark_Generate_AddEntityModels");

        ent = CG_GetEntity(localClientNum, entityIndex);
        if (ent->nextValid && (dObj = Com_GetClientDObj(ent->nextState.number, localClientNum)) != 0)
        {
            dObjRadius = DObjGetRadius(dObj);
            summedRadius = dObjRadius + radius;
            summedRadiusSquared = summedRadius * summedRadius;
            Vec3Sub(ent->pose.origin, origin, diff);
            if (summedRadiusSquared >= Vec3LengthSq(diff))
            {
                entityIndexAsUnsignedShort = truncate_cast<ushort>(entityIndex);
                iassert(entityIndexAsUnsignedShort == entityIndex);
                R_MarkFragments_AddDObj(markInfo, dObj, &ent->pose, entityIndexAsUnsignedShort);
            }
        }
    }
}

static void __cdecl FX_ImpactMark_Generate(
    int32_t localClientNum,
    MarkFragmentsAgainstEnum markAgainst,
    Material *material,
    float *origin,
    const float (*axis)[3],
    float orientation,
    const byte *nativeColor,
    float radius,
    uint32_t markEntnum)
{
    FxMarkTri tris[256]; // [esp+230h] [ebp-1058h] BYREF
    MarkInfo markInfo; // [esp+E28h] [ebp-460h] BYREF
    FxSystem *System; // [esp+1274h] [ebp-14h]

    struct FX_ImpactMark_Generate_CB callbackContext
    {
        .localClientNum = localClientNum,
        .material = material,
        .radius = radius,
        .nativeColor = nativeColor
    };

    System = FX_GetSystem(localClientNum);

    if (fx_marks->current.enabled
        && (markAgainst != MARK_FRAGMENTS_AGAINST_MODELS
            || fx_marks_ents->current.enabled
            || fx_marks_smodels->current.enabled))
    {
        R_MarkFragments_Begin(&markInfo, markAgainst, origin, axis, radius, System->camera.viewOffset, material);

        if (fx_marks_ents->current.enabled)
        {
            iassert(markAgainst == MARK_FRAGMENTS_AGAINST_MODELS || markAgainst == MARK_FRAGMENTS_AGAINST_BRUSHES);

            if (markAgainst == MARK_FRAGMENTS_AGAINST_MODELS)
                FX_ImpactMark_Generate_AddEntityModel(localClientNum, &markInfo, markEntnum, origin, radius);
            else
                FX_ImpactMark_Generate_AddEntityBrush(localClientNum, &markInfo, markEntnum, origin, radius);
        }

        R_MarkFragments_Go(&markInfo, 
            FX_ImpactMark_Generate_Callback, 
            &callbackContext, 
            ARRAY_COUNT(tris) - 1, 
            &tris[0], 
            ARRAY_COUNT(g_fxMarkPoints), 
            g_fxMarkPoints
        );
    }
}

static void __cdecl FX_ImpactMark(
    int32_t localClientNum,
    Material *worldMaterial,
    Material *modelMaterial,
    float *origin,
    const float *quat,
    float orientation,
    const uint8_t *nativeColor,
    float radius,
    uint32_t markEntnum)
{
    float axis[3][3]; // [esp+5Ch] [ebp-24h] BYREF

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);

    if (fx_marks->current.enabled && !marksSystem->noMarks && radius >= 0.1f)
    {
        iassert(radius > 0);
        UnitQuatToAxis(quat, axis);
        RotatePointAroundVector(axis[2], axis[0], axis[1], RAD2DEG(orientation));
        Vec3Cross(axis[0], axis[2], axis[1]);

        {
            PROF_SCOPED("FX_ImpactMark_World");
            FX_ImpactMark_Generate(
                localClientNum,
                MARK_FRAGMENTS_AGAINST_BRUSHES,
                worldMaterial,
                origin,
                axis,
                orientation,
                nativeColor,
                radius,
                markEntnum);
        }
        
        if (fx_marks_smodels->current.enabled || fx_marks_ents->current.enabled)
        {
            PROF_SCOPED("FX_ImpactMark_Models");
            FX_ImpactMark_Generate(
                localClientNum,
                MARK_FRAGMENTS_AGAINST_MODELS,
                modelMaterial,
                origin,
                axis,
                orientation,
                nativeColor,
                radius,
                markEntnum);
        }
    }
}

void __cdecl FX_CreateImpactMark(
    int32_t localClientNum,
    const FxElemDef *elemDef,
    const FxSpatialFrame *spatialFrame,
    int32_t randomSeed,
    uint32_t markEntnum)
{
    FxElemVisualState visState; // [esp+50h] [ebp-3Ch] BYREF
    FxElemPreVisualState preVisState; // [esp+6Ch] [ebp-20h] BYREF
    FxElemMarkVisuals *markVisuals; // [esp+88h] [ebp-4h]

    PROF_SCOPED("FX_CreateImpactMark");

    FX_SetupVisualState(elemDef, 0, randomSeed, 0.0, &preVisState);
    visState.size[0] = FX_InterpolateSize(
        preVisState.refState,
        randomSeed,
        FXRAND_SIZE_0,
        preVisState.sampleLerp,
        preVisState.sampleLerpInv,
        0);
    FX_EvaluateVisualState(&preVisState, 1.0, &visState);
    markVisuals = &elemDef->visuals.markArray[(elemDef->visualCount * LOWORD(fx_randomTable[randomSeed + 21])) >> 16];
    FX_ImpactMark(
        localClientNum,
        markVisuals->materials[1],
        markVisuals->materials[0],
        (float*)spatialFrame->origin,
        spatialFrame->quat,
        visState.rotationTotal,
        visState.color,
        visState.size[0],
        markEntnum);
}

void __cdecl FX_BeginMarks(int32_t clientIndex)
{
    FxMarksSystem *marksSystem = FX_GetMarksSystem(clientIndex);

    if (++marksSystem->frameCount <= 0)
    {
        marksSystem->frameCount = 1;
    }
}

static void __cdecl FX_MarkEntDetachMatchingBones(
    FxMarksSystem *marksSystem,
    int32_t entnum,
    const uint32_t *unsetHidePartBits)
{
    uint16_t handle; // [esp+18h] [ebp-Ch]
    FxMark *mark; // [esp+1Ch] [ebp-8h]
    int32_t markBoneIndex; // [esp+20h] [ebp-4h]

    handle = marksSystem->entFirstMarkHandles[entnum];
    while (handle != FX_HANDLE_NONE)
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ((mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_MASK)
        {
            markBoneIndex = mark->context.lmapIndex;

            iassert((markBoneIndex >> 5) < DOBJ_MAX_PART_BITS);
            iassert((mark->context.modelTypeAndSurf & MARK_MODEL_SURF_MASK) == 0);

            if ((unsetHidePartBits[markBoneIndex >> 5] & (0x80000000 >> (markBoneIndex & 0x1F))) != 0)
                FX_FreeMark(marksSystem, mark);
        }
    }
}

void __cdecl FX_MarkEntDetachAll(int32_t localClientNum, int32_t entnum)
{
    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    
    while (marksSystem->entFirstMarkHandles[entnum] != FX_HANDLE_NONE)
    {
        FX_FreeMark(marksSystem, FX_MarkFromHandle(marksSystem, marksSystem->entFirstMarkHandles[entnum]));
    }
}

void __cdecl FX_MarkEntUpdateHidePartBits(
    const uint32_t *oldHidePartBits,
    const uint32_t *newHidePartBits,
    int32_t localClientNum,
    int32_t entnum)
{
    uint32_t v4; // edx
    uint32_t unsetHidePartBits[4]; // [esp+8h] [ebp-18h] BYREF
    int32_t hidePartIntIndex; // [esp+18h] [ebp-8h]
    uint32_t oredUnsetHidePartBits; // [esp+1Ch] [ebp-4h]

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    
    oredUnsetHidePartBits = 0;
    for (hidePartIntIndex = 0; hidePartIntIndex != 4; ++hidePartIntIndex)
    {
        v4 = newHidePartBits[hidePartIntIndex] & ~oldHidePartBits[hidePartIntIndex];
        unsetHidePartBits[hidePartIntIndex] = v4;
        oredUnsetHidePartBits |= v4;
    }
    if (oredUnsetHidePartBits)
        FX_MarkEntDetachMatchingBones(marksSystem, entnum, unsetHidePartBits);
}

static void __cdecl FX_MarkEntDetachAllOfType(int32_t localClientNum, int32_t entnum, int32_t markType)
{
    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    ushort handle = marksSystem->entFirstMarkHandles[entnum];
    while (handle != FX_HANDLE_NONE)
    {
        FxMark *mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ((mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == markType)
            FX_FreeMark(marksSystem, mark);
    }
}

static void __cdecl FX_MarkEntDetachModel(FxMarksSystem *marksSystem, int32_t entnum, int32_t oldModelIndex)
{
    uint16_t handle; // [esp+18h] [ebp-Ch]
    FxMark *mark; // [esp+1Ch] [ebp-8h]
    int32_t markModelIndex; // [esp+20h] [ebp-4h]

    handle = marksSystem->entFirstMarkHandles[entnum];
    while (handle != FX_HANDLE_NONE)
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ((mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == 0xC0)
        {
            markModelIndex = mark->context.modelTypeAndSurf & MARK_MODEL_SURF_MASK;
            if (markModelIndex == oldModelIndex)
            {
                if (mark->frameCountDrawn == -1)
                    MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 972, 0, "%s", "mark->frameCountDrawn != FX_MARK_FREE");
                FX_FreeMark(marksSystem, mark);
                if (mark->frameCountDrawn != -1)
                    MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 974, 0, "%s", "mark->frameCountDrawn == FX_MARK_FREE");
            }
            else if (markModelIndex > oldModelIndex)
            {
                --mark->context.modelTypeAndSurf;
            }
        }
    }
}

static void __cdecl FX_MarkEntUpdateEndDObj(FxMarkDObjUpdateContext *context, int32_t localClientNum, int32_t entnum, DObj_s *obj)
{
    int32_t oldModelCount; // [esp+4h] [ebp-14h]
    int32_t removedModelCount; // [esp+8h] [ebp-10h]
    int32_t oldModelIndex; // [esp+Ch] [ebp-Ch]
    int32_t modelCount; // [esp+10h] [ebp-8h]
    int32_t modelIndex; // [esp+14h] [ebp-4h]

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);

    oldModelCount = context->modelCount;
    modelCount = DObjGetNumModels(obj);
    modelIndex = 0;
    removedModelCount = 0;
    for (oldModelIndex = 0; oldModelIndex != oldModelCount; ++oldModelIndex)
    {
        if (modelIndex == modelCount)
        {
            while (oldModelIndex != oldModelCount)
            {
                FX_MarkEntDetachModel(marksSystem, entnum, oldModelIndex - removedModelCount++);
                ++oldModelIndex;
            }
            return;
        }
        if (DObjGetModel(obj, modelIndex) == context->models[oldModelIndex]
            && DObjGetModelParentBoneName(obj, modelIndex) == context->modelParentBones[oldModelIndex])
        {
            ++modelIndex;
        }
        else
        {
            FX_MarkEntDetachModel(marksSystem, entnum, oldModelIndex - removedModelCount++);
        }
    }
}

void __cdecl FX_MarkEntUpdateBegin(
    FxMarkDObjUpdateContext *context,
    DObj_s *obj,
    bool isBrush,
    uint16_t brushIndex)
{
    int32_t modelCount; // [esp+0h] [ebp-8h]
    int32_t modelIndex; // [esp+4h] [ebp-4h]

    if (obj)
    {
        modelCount = DObjGetNumModels(obj);
        context->modelCount = modelCount;
        for (modelIndex = 0; modelIndex != modelCount; ++modelIndex)
        {
            context->models[modelIndex] = DObjGetModel(obj, modelIndex);
            context->modelParentBones[modelIndex] = DObjGetModelParentBoneName(obj, modelIndex);
        }
    }
    else
    {
        context->modelCount = 0;
    }
    context->isBrush = isBrush;
    context->brushIndex = brushIndex;
}

void __cdecl FX_MarkEntUpdateEnd(
    FxMarkDObjUpdateContext *context,
    int32_t localClientNum,
    int32_t entnum,
    DObj_s *obj,
    bool isBrush,
    uint16_t brushIndex)
{
    if (context->isBrush && (!isBrush || context->brushIndex != brushIndex))
        FX_MarkEntDetachAllOfType(localClientNum, entnum, MARK_MODEL_TYPE_ENT_BRUSH);
    if (context->modelCount)
    {
        if (obj)
            FX_MarkEntUpdateEndDObj(context, localClientNum, entnum, obj);
        else
            FX_MarkEntDetachAllOfType(localClientNum, entnum, MARK_MODEL_TYPE_ENT_MODEL);
    }
}

static void __cdecl FX_EmitMarkTri(
    FxMarksSystem *marksSystem,
    const uint16_t *indices,
    const GfxMarkContext *markContext,
    uint16_t baseVertex,
    FxActiveMarkSurf *outSurf)
{
    r_double_index_t *pIndex; // [esp+10h] [ebp-8h]
    r_double_index_t index; // [esp+14h] [ebp-4h]

    if (memcmp((const char *)&outSurf->context, (const char *)markContext, 6))
    {
        if (outSurf->indexCount)
        {
            R_AddMarkMeshDrawSurf(outSurf->material, &outSurf->context, outSurf->indices, outSurf->indexCount);
            outSurf->indices += outSurf->indexCount;
            outSurf->indexCount = 0;
        }
        vassert(outSurf->context.modelIndex == markContext->modelIndex, "outSurf->context.modelIndex = %hu, markContext->modelIndex = %hu", outSurf->context.modelIndex, markContext->modelIndex);
        vassert((outSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == (markContext->modelTypeAndSurf & MARK_MODEL_TYPE_MASK), "(outSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) = %x, (markContext->modelTypeAndSurf & MARK_MODEL_TYPE_MASK) = %x", outSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK, markContext->modelTypeAndSurf & MARK_MODEL_TYPE_MASK);
        outSurf->context = *markContext;
    }
    if (marksSystem->hasCarryIndex)
    {
        index.value[0] = marksSystem->carryIndex;
        index.value[1] = *indices + baseVertex;
        pIndex = (r_double_index_t *)&outSurf->indices[outSurf->indexCount - 1];
        iassert(!((uint)pIndex & 3));
        *pIndex = index;
        index.value[0] = indices[1] + baseVertex;
        index.value[1] = indices[2] + baseVertex;
        pIndex = pIndex + 1;
        iassert(!((uint)pIndex & 3));
        *pIndex = index;
        marksSystem->hasCarryIndex = 0;
    }
    else
    {
        index.value[0] = *indices + baseVertex;
        index.value[1] = indices[1] + baseVertex;
        pIndex = (r_double_index_t *)&outSurf->indices[outSurf->indexCount];
        if (((uint8_t)pIndex & 3) != 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1255, 0, "%s", "!((uint)pIndex & 3)");
        *pIndex = index;
        marksSystem->hasCarryIndex = 1;
        marksSystem->carryIndex = indices[2] + baseVertex;
    }
    outSurf->indexCount += 3;
}

static void __cdecl FX_DrawMarkTris(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    uint16_t baseVertex,
    uint16_t *indices,
    FxActiveMarkSurf *outSurf)
{
    uint32_t groupHandle; // [esp+Ch] [ebp-10h]
    int32_t triCount; // [esp+10h] [ebp-Ch]
    FxTriGroupPool *group; // [esp+14h] [ebp-8h]
    int32_t triIndex; // [esp+18h] [ebp-4h]

    iassert(mark);
    groupHandle = mark->tris;
    triCount = mark->triCount;
    outSurf->material = mark->material;
    outSurf->context.lmapIndex = -1;
    outSurf->context.primaryLightIndex = 0;
    outSurf->context.reflectionProbeIndex = -1;
    outSurf->context.modelTypeAndSurf = mark->context.modelTypeAndSurf;
    outSurf->context.modelIndex = mark->context.modelIndex;
    outSurf->indices = indices;
    outSurf->indexCount = 0;

    do
    {
        group = FX_TriGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->triGroup.next;
        vassert(triCount >= group->triGroup.triCount, "%i < %i", triCount, group->triGroup.triCount);
        vassert(group->triGroup.triCount > 0, "(group->triCount) = %i", group->triGroup.triCount);
        triCount -= group->triGroup.triCount;
        triIndex = 0;
        do
        {
            FX_EmitMarkTri(
                marksSystem,
                (const uint16_t *)group + 3 * triIndex++,
                &group->triGroup.context,
                baseVertex,
                outSurf);
        }
        while (triIndex != group->triGroup.triCount);
    } while (triCount);

    iassert(groupHandle == FX_HANDLE_NONE);
    iassert(outSurf->indexCount);
}

static char __cdecl FX_GenerateMarkVertsForMark_Begin(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    uint32_t *indexCount,
    uint16_t *outBaseVertex,
    FxActiveMarkSurf *outDrawSurf)
{
    uint32_t newIndexCount; // [esp+18h] [ebp-10h]
    uint32_t reserveIndexCount; // [esp+1Ch] [ebp-Ch]
    uint16_t *indices; // [esp+20h] [ebp-8h]
    r_double_index_t *doubleIndices; // [esp+24h] [ebp-4h] BYREF

    newIndexCount = *indexCount + 3 * mark->triCount;
    reserveIndexCount = ((newIndexCount + 1) & 0xFFFFFFFE) - ((*indexCount + 1) & 0xFFFFFFFE);

    if (R_ReserveMarkMeshVerts(mark->pointCount, outBaseVertex) && R_ReserveMarkMeshIndices(reserveIndexCount, &doubleIndices))
    {
        indices = (uint16_t *)doubleIndices - (*indexCount & 1);
        *indexCount = newIndexCount;
        iassert(mark->frameCountDrawn != FX_MARK_FREE);
        mark->frameCountDrawn = marksSystem->frameCount;
        FX_DrawMarkTris(marksSystem, mark, *outBaseVertex, indices, outDrawSurf);
        return 1;
    }
    else
    {
        FX_FreeMark(marksSystem, mark);
        return 0;
    }
}

static void __cdecl FX_GenerateMarkVertsForMark_SetLightHandle(
    FxActiveMarkSurf *drawSurf,
    uint16_t lightHandleOverride)
{
    iassert(((drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL || (drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH));
    iassert(lightHandleOverride != GFX_ML_HANDLE_NONE);

    drawSurf->context.modelIndex = lightHandleOverride;
}

static void __cdecl FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(
    FxActiveMarkSurf *drawSurf,
    uint8_t reflectionProbeIndexOverride)
{
    iassert(((drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL || (drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH));
    iassert(reflectionProbeIndexOverride != REFLECTION_PROBE_INVALID);

    drawSurf->context.reflectionProbeIndex = reflectionProbeIndexOverride;
}

static void __cdecl setTexCoordAndLMap_GfxPackedVertex_(GfxPackedVertex *outVert, const float *texCoord)
{
    outVert->texCoord = Vec2PackTexCoords(texCoord);
}

static void __cdecl FX_ExpandMarkVerts_Transform_GfxWorldVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    uint16_t baseVertex,
    const float (*matrixTransform)[3])
{
    double v4; // st7
    double v5; // st7
    PackedUnitVec v7; // [esp+54h] [ebp-BCh]
    PackedUnitVec v8; // [esp+74h] [ebp-9Ch]
    float *lmapCoord; // [esp+94h] [ebp-7Ch]
    GfxWorldVertex *castOutVert; // [esp+A8h] [ebp-68h]
    uint32_t groupHandle; // [esp+ACh] [ebp-64h]
    float delta[3]; // [esp+B0h] [ebp-60h] BYREF
    float transformedNormal[3]; // [esp+BCh] [ebp-54h] BYREF
    float texCoordScale; // [esp+C8h] [ebp-48h]
    float binormal[3]; // [esp+CCh] [ebp-44h] BYREF
    const FxMarkPoint *markPoint; // [esp+D8h] [ebp-38h]
    GfxWorldVertex *verts; // [esp+DCh] [ebp-34h]
    float transformedDelta[3]; // [esp+E0h] [ebp-30h] BYREF
    //__int64 texCoord; // [esp+ECh] [ebp-24h]
    float texCoord[2];
    int32_t pointCount; // [esp+F4h] [ebp-1Ch]
    int32_t loopCount; // [esp+F8h] [ebp-18h]
    float transformedTexCoordAxis[3]; // [esp+FCh] [ebp-14h] BYREF
    const FxPointGroup *group; // [esp+108h] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+10Ch] [ebp-4h]

    iassert(mark);
    verts = R_GetMarkMeshVerts(baseVertex);
    iassert(mark->radius >= 0.1f);

    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    pointCount = mark->pointCount;
    outVert = verts;

    do
    {
        iassert(groupHandle != FX_HANDLE_NONE);
        group = &FX_PointGroupFromHandle(marksSystem, groupHandle)->pointGroup;
        groupHandle = group->next;
        if (pointCount > 2)
            loopCount = 2;
        else
            loopCount = pointCount;

        iassert(loopCount > 0);
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint * )group;
        do
        {
            castOutVert = outVert;
            Vec3Sub(markPoint->xyz, mark->origin, delta);
            MatrixTransformVector(delta, *(mat3x3*)matrixTransform, transformedDelta);
            MatrixTransformVector(mark->texCoordAxis, *(mat3x3 *)matrixTransform, transformedTexCoordAxis);
            MatrixTransformVector(markPoint->normal, *(mat3x3 *)matrixTransform, transformedNormal);
            Vec3Cross(transformedTexCoordAxis, transformedNormal, binormal);
            MatrixTransformVector43(markPoint->xyz, *(mat4x3 *)matrixTransform, castOutVert->xyz);
            castOutVert->binormalSign = -1.0;
            castOutVert->color.array[0] = mark->nativeColor[0];
            castOutVert->color.array[1] = mark->nativeColor[1];
            castOutVert->color.array[2] = mark->nativeColor[2];
            castOutVert->color.array[3] = mark->nativeColor[3];
            v4 = Vec3Dot(transformedDelta, transformedTexCoordAxis);
            //*&texCoord = v4 * texCoordScale + 0.5;
            texCoord[0] = v4 * texCoordScale + 0.5f;
            v5 = Vec3Dot(transformedDelta, binormal);
            //*(&texCoord + 1) = v5 * texCoordScale + 0.5;
            texCoord[1] = v5 * texCoordScale + 0.5f;
            lmapCoord = (float*)markPoint->lmapCoord;
            castOutVert->texCoord[0] = texCoord[0];
            castOutVert->texCoord[1] = texCoord[1];
            castOutVert->lmapCoord[0] = lmapCoord[0];
            castOutVert->lmapCoord[1] = lmapCoord[1];
            v8.array[0] = (transformedNormal[0] * 127.0 + 127.5);
            v8.array[1] = (transformedNormal[1] * 127.0 + 127.5);
            v8.array[2] = (transformedNormal[2] * 127.0 + 127.5);
            v8.array[3] = 63;
            castOutVert->normal = v8;
            v7.array[0] = (transformedTexCoordAxis[0] * 127.0 + 127.5);
            v7.array[1] = (transformedTexCoordAxis[1] * 127.0 + 127.5);
            v7.array[2] = (transformedTexCoordAxis[2] * 127.0 + 127.5);
            v7.array[3] = 63;
            castOutVert->tangent = v7;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);

    iassert(groupHandle == FX_HANDLE_NONE);
}

static void __cdecl FX_ExpandMarkVerts_Transform_GfxPackedVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    uint16_t baseVertex,
    const float (*matrixTransform)[3])
{
    double v4; // st7
    double v5; // st7
    PackedUnitVec v7; // [esp+54h] [ebp-ECh]
    PackedUnitVec v8; // [esp+74h] [ebp-CCh]
    GfxPackedVertex *castOutVert; // [esp+D8h] [ebp-68h]
    uint32_t groupHandle; // [esp+DCh] [ebp-64h]
    float delta[3]; // [esp+E0h] [ebp-60h] BYREF
    float transformedNormal[3]; // [esp+ECh] [ebp-54h] BYREF
    float texCoordScale; // [esp+F8h] [ebp-48h]
    float binormal[3]; // [esp+FCh] [ebp-44h] BYREF
    const FxMarkPoint *markPoint; // [esp+108h] [ebp-38h]
    GfxWorldVertex *verts; // [esp+10Ch] [ebp-34h]
    float transformedDelta[3]; // [esp+110h] [ebp-30h] BYREF
    float texCoord[2]; // [esp+11Ch] [ebp-24h] BYREF
    int32_t pointCount; // [esp+124h] [ebp-1Ch]
    int32_t loopCount; // [esp+128h] [ebp-18h]
    float transformedTexCoordAxis[3]; // [esp+12Ch] [ebp-14h] BYREF
    const FxPointGroup *group; // [esp+138h] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+13Ch] [ebp-4h]

    iassert(mark);
    verts = R_GetMarkMeshVerts(baseVertex);
    iassert(mark->radius >= 0.1f);
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    pointCount = mark->pointCount;
    outVert = verts;
    do
    {
        iassert(groupHandle != FX_HANDLE_NONE);
        group = (const FxPointGroup*)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            loopCount = 2;
        else
            loopCount = pointCount;

        iassert(loopCount > 0);

        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            castOutVert = (GfxPackedVertex*)outVert;
            Vec3Sub(markPoint->xyz, mark->origin, delta);
            MatrixTransformVector(delta, *(const mat3x3*)matrixTransform, transformedDelta);
            MatrixTransformVector(mark->texCoordAxis, *(const mat3x3*)matrixTransform, transformedTexCoordAxis);
            MatrixTransformVector(markPoint->normal, *(const mat3x3*)matrixTransform, transformedNormal);
            Vec3Cross(transformedTexCoordAxis, transformedNormal, binormal);
            MatrixTransformVector43(markPoint->xyz, *(const mat4x3*)matrixTransform, castOutVert->xyz);
            castOutVert->binormalSign = -1.0;
            castOutVert->color.array[0] = mark->nativeColor[0];
            castOutVert->color.array[1] = mark->nativeColor[1];
            castOutVert->color.array[2] = mark->nativeColor[2];
            castOutVert->color.array[3] = mark->nativeColor[3];
            v4 = Vec3Dot(transformedDelta, transformedTexCoordAxis);
            texCoord[0] = v4 * texCoordScale + 0.5;
            v5 = Vec3Dot(transformedDelta, binormal);
            texCoord[1] = v5 * texCoordScale + 0.5;
            setTexCoordAndLMap_GfxPackedVertex_(castOutVert, texCoord);
            v8.array[0] = (transformedNormal[0] * 127.0 + 127.5);
            v8.array[1] = (transformedNormal[1] * 127.0 + 127.5);
            v8.array[2] = (transformedNormal[2] * 127.0 + 127.5);
            v8.array[3] = 63;
            castOutVert->normal = v8;
            v7.array[0] = (transformedTexCoordAxis[0] * 127.0 + 127.5);
            v7.array[1] = (transformedTexCoordAxis[1] * 127.0 + 127.5);
            v7.array[2] = (transformedTexCoordAxis[2] * 127.0 + 127.5);
            v7.array[3] = 63;
            castOutVert->tangent = v7;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);

    iassert(groupHandle == FX_HANDLE_NONE);
}

static void __cdecl FX_GenerateMarkVertsForMark_FinishAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    uint16_t baseVertex,
    FxActiveMarkSurf *drawSurf,
    const float (*transform)[3])
{
    uint type = drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK;

    iassert(type == MARK_MODEL_TYPE_ENT_BRUSH || type == MARK_MODEL_TYPE_ENT_MODEL);
    iassert(transform);

    R_AddMarkMeshDrawSurf(drawSurf->material, &drawSurf->context, drawSurf->indices, drawSurf->indexCount);
    if (type == MARK_MODEL_TYPE_ENT_BRUSH)
        FX_ExpandMarkVerts_Transform_GfxWorldVertex_(marksSystem, mark, baseVertex, transform);
    else
        FX_ExpandMarkVerts_Transform_GfxPackedVertex_(marksSystem, mark, baseVertex, transform);
}

static void FX_GenerateMarkVertsForMark_MatrixFromPlacement(
    const GfxPlacement* placement,
    const float* viewOffset,
    float (*outTransform)[3])
{
    DObjAnimMat mat; // [sp+50h] [-80h] BYREF
    DObjSkelMat skelMat; // [sp+70h] [-60h] BYREF

    Vec4Copy(placement->quat, mat.quat);
    mat.transWeight = 2.0f;
    Vec3Copy(placement->origin, mat.trans);
    ConvertQuatToSkelMat(&mat, &skelMat);
    DObjSkelMatToMatrix43(&skelMat, outTransform);
    (*outTransform)[9] = (*outTransform)[9] + *viewOffset;
    (*outTransform)[10] = (*outTransform)[10] + viewOffset[1];
    (*outTransform)[11] = (*outTransform)[11] + viewOffset[2];
}

static void __cdecl FX_GenerateMarkVertsForMark_MatrixFromScaledPlacement(
    const GfxScaledPlacement* placement,
    const float* viewOffset,
    float (*outTransform)[3])
{
    iassert(placement->scale == 1.0f);
    FX_GenerateMarkVertsForMark_MatrixFromPlacement(&placement->base, viewOffset, outTransform);
}

static void __cdecl FX_GenerateMarkVertsForMark_MatrixFromAnim(
    FxMark *mark,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList,
    const vec3r viewOffset,
    mat4x3 &outTransform)
{
    iassert(dobj);
    iassert(boneMtxList);

    const int32_t boneIndex = mark->context.lmapIndex;
    const int32_t modelIndex = mark->context.modelTypeAndSurf & MARK_MODEL_SURF_MASK;

    iassert(modelIndex < DObjGetNumModels(dobj));

    int32_t modelBoneOffset = 0;
    for (int32_t modelIndexIter = 0; modelIndexIter < modelIndex; ++modelIndexIter)
    {
        const XModel *model = DObjGetModel(dobj, modelIndexIter);
        modelBoneOffset += XModelNumBones(model);
    }

    DObjSkelMat animatedBoneSkelMat;
    mat4x3 animatedBoneMatrix;
    ConvertQuatToSkelMat(&boneMtxList[modelBoneOffset + boneIndex], &animatedBoneSkelMat);
    DObjSkelMatToMatrix43(&animatedBoneSkelMat, animatedBoneMatrix);
    Vec3Add(animatedBoneMatrix[3], viewOffset, animatedBoneMatrix[3]);

    const XModel *model = DObjGetModel(dobj, modelIndex);
    const DObjAnimMat *basePose = XModelGetBasePose(model);

    DObjSkelMat inverseBasePoseSkelMat;
    mat4x3 inverseBasePoseMatrix;
    ConvertQuatToInverseSkelMat(&basePose[boneIndex], &inverseBasePoseSkelMat);
    DObjSkelMatToMatrix43(&inverseBasePoseSkelMat, inverseBasePoseMatrix);

    MatrixMultiply43(inverseBasePoseMatrix, animatedBoneMatrix, outTransform);
}

static void __cdecl FX_FinishGeneratingMarkVerts(FxMarksSystem *marksSystem)
{
    r_double_index_t *doubleIndices; // [esp+8h] [ebp-4h] BYREF

    if (marksSystem->hasCarryIndex)
    {
        bool allocSuccessed = R_ReserveMarkMeshIndices(0, &doubleIndices);
        iassert(allocSuccessed);
        (*--doubleIndices).value[0] = marksSystem->carryIndex;
    }
}

static char __cdecl FX_GenerateMarkVertsForList_EntXModel(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount,
    uint16_t lightHandleOverride,
    uint8_t reflectionProbeIndexOverride,
    const GfxScaledPlacement *placement)
{
    FxMark *mark; // [esp+F4h] [ebp-50h]
    FxActiveMarkSurf drawSurf; // [esp+F8h] [ebp-4Ch] BYREF
    uint16_t markHandle; // [esp+10Ch] [ebp-38h]
    uint16_t baseVertex; // [esp+110h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+114h] [ebp-30h] BYREF

    FX_GenerateMarkVertsForMark_MatrixFromScaledPlacement(placement, vec3_origin, (float (*)[3])transformMatrix);
    for (markHandle = head; markHandle != FX_HANDLE_NONE; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if ((mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == 0xC0)
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return 0;
            FX_GenerateMarkVertsForMark_SetLightHandle(&drawSurf, lightHandleOverride);
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndexOverride);
            FX_GenerateMarkVertsForMark_FinishAnimated(marksSystem, mark, baseVertex, &drawSurf, (const float(*)[3])transformMatrix);
        }
    }
    return 1;
}

static char __cdecl FX_GenerateMarkVertsForList_EntDObj(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount,
    uint16_t lightHandleOverride,
    uint8_t reflectionProbeIndexOverride,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList)
{
    FxMark *mark; // [esp+214h] [ebp-50h]
    FxActiveMarkSurf drawSurf; // [esp+218h] [ebp-4Ch] BYREF
    uint16_t markHandle; // [esp+22Ch] [ebp-38h]
    uint16_t baseVertex; // [esp+230h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+234h] [ebp-30h] BYREF
    int32_t savedregs; // [esp+264h] [ebp+0h] BYREF

    for (markHandle = head; markHandle != FX_HANDLE_NONE; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if ((mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL)
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return 0;
            FX_GenerateMarkVertsForMark_SetLightHandle(&drawSurf, lightHandleOverride);
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndexOverride);
            FX_GenerateMarkVertsForMark_MatrixFromAnim(
                mark,
                dobj,
                boneMtxList,
                camera->viewOffset,
                transformMatrix);
            FX_GenerateMarkVertsForMark_FinishAnimated(marksSystem, mark, baseVertex, &drawSurf, transformMatrix);
        }
    }
    return 1;
}

static char __cdecl FX_GenerateMarkVertsForList_EntBrush(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount,
    const GfxPlacement *placement,
    uint8_t reflectionProbeIndex)
{
    FxMark *mark; // [esp+F4h] [ebp-50h]
    FxActiveMarkSurf drawSurf; // [esp+F8h] [ebp-4Ch] BYREF
    uint16_t markHandle; // [esp+10Ch] [ebp-38h]
    uint16_t baseVertex; // [esp+110h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+114h] [ebp-30h] BYREF

    FX_GenerateMarkVertsForMark_MatrixFromPlacement(placement, vec3_origin, (float(*)[3])transformMatrix);
    for (markHandle = head; markHandle != FX_HANDLE_NONE; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if ((mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH)
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return 0;
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndex);
            FX_GenerateMarkVertsForMark_FinishAnimated(marksSystem, mark, baseVertex, &drawSurf, transformMatrix);
        }
    }
    return 1;
}

void __cdecl FX_BeginGeneratingMarkVertsForEntModels(int32_t localClientNum, uint32_t *indexCount)
{
    if (!fx_marks->current.enabled || !fx_marks_ents->current.enabled)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1633,
            0,
            "%s",
            "fx_marks->current.enabled && fx_marks_ents->current.enabled");
    PROF_SCOPED("FX_GenMarkVertsEnt");
    R_BeginMarkMeshVerts();
    if (InterlockedIncrement((LONG*) & g_markThread[localClientNum]) != 1)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1638,
            0,
            "%s",
            "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1");

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    marksSystem->hasCarryIndex = 0;
    *indexCount = 0;
}

void __cdecl FX_GenerateMarkVertsForEntXModel(
    int32_t localClientNum,
    int32_t entId,
    uint32_t *indexCount,
    uint16_t lightHandle,
    uint8_t reflectionProbeIndex,
    const GfxScaledPlacement *placement)
{
    FxSystem *camera; // [esp+94h] [ebp-Ch]
    uint16_t entMarkListHead; // [esp+9Ch] [ebp-4h]

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    entMarkListHead = marksSystem->entFirstMarkHandles[entId];
    if (entMarkListHead != FX_HANDLE_NONE)
    {
        PROF_SCOPED("FX_GenMarkVertsEnt");
        camera = FX_GetSystem(localClientNum);
        FX_GenerateMarkVertsForList_EntXModel(
            marksSystem,
            entMarkListHead,
            &camera->camera,
            indexCount,
            lightHandle,
            reflectionProbeIndex,
            placement);
    }
}

void __cdecl FX_GenerateMarkVertsForEntDObj(
    int32_t localClientNum,
    int32_t entId,
    uint32_t *indexCount,
    uint16_t lightHandle,
    uint8_t reflectionProbeIndex,
    const DObj_s *dobj,
    const cpose_t *pose)
{
    FxSystem *camera; // [esp+94h] [ebp-20h]
    uint32_t hidePartBits[4]; // [esp+98h] [ebp-1Ch] BYREF
    FxSystem *system; // [esp+A8h] [ebp-Ch]
    const DObjAnimMat *boneMtxList; // [esp+ACh] [ebp-8h] BYREF
    uint16_t entMarkListHead; // [esp+B0h] [ebp-4h]
    
    iassert(dobj);
    iassert(pose);

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    entMarkListHead = marksSystem->entFirstMarkHandles[entId];
    if (entMarkListHead != FX_HANDLE_NONE)
    {
        PROF_SCOPED("FX_GenMarkVertsEnt");
        system = FX_GetSystem(localClientNum);
        camera = system;
        R_MarkUtil_GetDObjAnimMatAndHideParts(dobj, pose, &boneMtxList, hidePartBits);
        FX_GenerateMarkVertsForList_EntDObj(
            marksSystem,
            entMarkListHead,
            &camera->camera,
            indexCount,
            lightHandle,
            reflectionProbeIndex,
            dobj,
            boneMtxList);
    }
}

void __cdecl FX_GenerateMarkVertsForEntBrush(
    int32_t localClientNum,
    int32_t entId,
    uint32_t *indexCount,
    uint8_t reflectionProbeIndex,
    const GfxPlacement *placement)
{
    iassert(placement);

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    ushort entMarkListHead = marksSystem->entFirstMarkHandles[entId];

    if (entMarkListHead != FX_HANDLE_NONE)
    {
        PROF_SCOPED("FX_GenMarkVertsEnt");
        FxSystem *camera = FX_GetSystem(localClientNum);
        FX_GenerateMarkVertsForList_EntBrush(
            marksSystem,
            entMarkListHead,
            &camera->camera,
            indexCount,
            placement,
            reflectionProbeIndex);
    }
}

void __cdecl FX_EndGeneratingMarkVertsForEntModels(int32_t localClientNum)
{
    PROF_SCOPED("FX_GenMarkVertsEnt");
    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    FX_FinishGeneratingMarkVerts(marksSystem);

    iassert(Sys_InterlockedDecrement(&g_markThread[localClientNum]) == 0);
    R_EndMarkMeshVerts();
}

static void __cdecl FX_ExpandMarkVerts_NoTransform_GfxPackedVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    uint16_t baseVertex)
{
    int32_t v5; // [esp+50h] [ebp-D4h]
    PackedUnitVec v6; // [esp+58h] [ebp-CCh]
    PackedUnitVec v7; // [esp+7Ch] [ebp-A8h]
    GfxPackedVertex *castOutVert; // [esp+E0h] [ebp-44h]
    uint32_t groupHandle; // [esp+E4h] [ebp-40h]
    float delta[3]; // [esp+E8h] [ebp-3Ch] BYREF
    float texCoordScale; // [esp+F4h] [ebp-30h]
    float binormal[3]; // [esp+F8h] [ebp-2Ch] BYREF
    const FxMarkPoint *markPoint; // [esp+104h] [ebp-20h]
    GfxWorldVertex *verts; // [esp+108h] [ebp-1Ch]
    float texCoord[2]; // [esp+10Ch] [ebp-18h] BYREF
    int32_t pointCount; // [esp+114h] [ebp-10h]
    int32_t loopCount; // [esp+118h] [ebp-Ch]
    const FxPointGroup *group; // [esp+11Ch] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+120h] [ebp-4h]

    iassert(mark);
    verts = R_GetMarkMeshVerts(baseVertex);
    pointCount = mark->pointCount;
    iassert(mark->radius >= 0.1f);
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    outVert = verts;
    do
    {
        iassert(groupHandle != FX_HANDLE_NONE);
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            loopCount = 2;
        else
            loopCount = pointCount;
        iassert(loopCount > 0);
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            castOutVert = (GfxPackedVertex *)outVert;
            Vec3Sub(markPoint->xyz, mark->origin, delta);
            Vec3Cross(mark->texCoordAxis, markPoint->normal, binormal);
            castOutVert->xyz[0] = markPoint->xyz[0];
            castOutVert->xyz[1] = markPoint->xyz[1];
            castOutVert->xyz[2] = markPoint->xyz[2];
            castOutVert->binormalSign = -1.0;
            castOutVert->color.packed = *(_DWORD *)mark->nativeColor;
            texCoord[0] = Vec3Dot(delta, mark->texCoordAxis) * texCoordScale + 0.5;
            texCoord[1] = Vec3Dot(delta, binormal) * texCoordScale + 0.5;
            setTexCoordAndLMap_GfxPackedVertex_(castOutVert, texCoord);
            v7.array[0] = (int)(markPoint->normal[0] * 127.0 + 127.5);
            v7.array[1] = (int)(markPoint->normal[1] * 127.0 + 127.5);
            v7.array[2] = (int)(markPoint->normal[2] * 127.0 + 127.5);
            v7.array[3] = 63;
            castOutVert->normal = v7;
            v6.array[0] = (int)(mark->texCoordAxis[0] * 127.0 + 127.5);
            v6.array[1] = (int)(mark->texCoordAxis[1] * 127.0 + 127.5);
            v6.array[2] = (int)(mark->texCoordAxis[2] * 127.0 + 127.5);
            v6.array[3] = 63;
            castOutVert->tangent = v6;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);

    iassert(groupHandle == FX_HANDLE_NONE);
}

static void __cdecl FX_ExpandMarkVerts_NoTransform_GfxWorldVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    uint16_t baseVertex)
{
    PackedUnitVec v6; // [esp+58h] [ebp-9Ch]
    PackedUnitVec v7; // [esp+7Ch] [ebp-78h]
    GfxWorldVertex *castOutVert; // [esp+B0h] [ebp-44h]
    uint32_t groupHandle; // [esp+B4h] [ebp-40h]
    float delta[3]; // [esp+B8h] [ebp-3Ch] BYREF
    float texCoordScale; // [esp+C4h] [ebp-30h]
    float binormal[3]; // [esp+C8h] [ebp-2Ch] BYREF
    const FxMarkPoint *markPoint; // [esp+D4h] [ebp-20h]
    GfxWorldVertex *verts; // [esp+D8h] [ebp-1Ch]
    int32_t pointCount; // [esp+E4h] [ebp-10h]
    int32_t loopCount; // [esp+E8h] [ebp-Ch]
    const FxPointGroup *group; // [esp+ECh] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+F0h] [ebp-4h]

    iassert(mark);
    verts = R_GetMarkMeshVerts(baseVertex);
    pointCount = mark->pointCount;
    iassert(mark->radius >= 0.1f);
    
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    outVert = verts;
    do
    {
        iassert(groupHandle != FX_HANDLE_NONE);
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            loopCount = 2;
        else
            loopCount = pointCount;
        iassert(loopCount > 0);
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            castOutVert = outVert;
            Vec3Sub(markPoint->xyz, mark->origin, delta);
            Vec3Cross(mark->texCoordAxis, markPoint->normal, binormal);
            Vec3Copy(markPoint->xyz, castOutVert->xyz);
            castOutVert->binormalSign = -1.0;
            castOutVert->color.array[0] = mark->nativeColor[0];
            castOutVert->color.array[1] = mark->nativeColor[1];
            castOutVert->color.array[2] = mark->nativeColor[2];
            castOutVert->color.array[3] = mark->nativeColor[3];
            castOutVert->texCoord[0] = Vec3Dot(delta, mark->texCoordAxis) * texCoordScale + 0.5;
            castOutVert->texCoord[1] = Vec3Dot(delta, binormal) * texCoordScale + 0.5;
            castOutVert->lmapCoord[0] = markPoint->lmapCoord[0];
            castOutVert->lmapCoord[1] = markPoint->lmapCoord[1];
            v7.array[0] = (int)(markPoint->normal[0] * 127.0 + 127.5);
            v7.array[1] = (int)(markPoint->normal[1] * 127.0 + 127.5);
            v7.array[2] = (int)(markPoint->normal[2] * 127.0 + 127.5);
            v7.array[3] = 63;
            castOutVert->normal = v7;
            v6.array[0] = (int)(mark->texCoordAxis[0] * 127.0 + 127.5);
            v6.array[1] = (int)(mark->texCoordAxis[1] * 127.0 + 127.5);
            v6.array[2] = (int)(mark->texCoordAxis[2] * 127.0 + 127.5);
            v6.array[3] = 63;
            castOutVert->tangent = v6;
            ++markPoint;
            ++outVert;
            --loopCount;
        } while (loopCount);
    } while (pointCount);

    iassert(groupHandle == FX_HANDLE_NONE);
}

static void __cdecl FX_GenerateMarkVertsForMark_FinishNonAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    uint16_t baseVertex,
    FxActiveMarkSurf *drawSurf)
{
    uint type = drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK;

    iassert(type == MARK_MODEL_TYPE_WORLD_BRUSH || type == MARK_MODEL_TYPE_WORLD_MODEL);

    R_AddMarkMeshDrawSurf(drawSurf->material, &drawSurf->context, drawSurf->indices, drawSurf->indexCount);

    if (type != MARK_MODEL_TYPE_WORLD_BRUSH)
        FX_ExpandMarkVerts_NoTransform_GfxPackedVertex_(marksSystem, mark, baseVertex);
    else
        FX_ExpandMarkVerts_NoTransform_GfxWorldVertex_(marksSystem, mark, baseVertex);
}

static char __cdecl FX_GenerateMarkVertsForList_WorldXModel(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount)
{
    FxMark *mark; // [esp+B0h] [ebp-20h]
    FxActiveMarkSurf drawSurf; // [esp+B4h] [ebp-1Ch] BYREF
    uint16_t baseVertex; // [esp+CCh] [ebp-4h] BYREF

    for (ushort markHandle = head; markHandle != FX_HANDLE_NONE; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
            return 0;
        FX_GenerateMarkVertsForMark_FinishNonAnimated(marksSystem, mark, baseVertex, &drawSurf);
    }
    return 1;
}

static bool FX_GenerateMarkVertsForList_WorldBrush(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount)
{
    FxMark *mark; // [esp+B4h] [ebp-20h]
    FxActiveMarkSurf drawSurf; // [esp+B8h] [ebp-1Ch] BYREF
    uint16_t markHandle; // [esp+CCh] [ebp-8h]
    uint16_t baseVertex; // [esp+D0h] [ebp-4h] BYREF

    for (markHandle = head; markHandle != FX_HANDLE_NONE; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if (!FX_CullSphere(camera, camera->frustumPlaneCount, mark->origin, mark->radius))
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return false;

            FX_GenerateMarkVertsForMark_FinishNonAnimated(marksSystem, mark, baseVertex, &drawSurf);
        }
    }

    return true;
}

void __cdecl FX_GenerateMarkVertsForStaticModels(
    int32_t localClientNum,
    int32_t smodelCount,
    const uint8_t *smodelVisLods)
{
    FxMark *mark; // [esp+64h] [ebp-14h]
    FxSystem *camera; // [esp+68h] [ebp-10h]
    uint32_t indexCount; // [esp+6Ch] [ebp-Ch] BYREF
    FxSystem *system; // [esp+70h] [ebp-8h]
    FxMark *markEnd; // [esp+74h] [ebp-4h]

    iassert(fx_marks->current.enabled && fx_marks_smodels->current.enabled);

    PROF_SCOPED("FX_GenMarkVertsStaticModel");
    R_BeginMarkMeshVerts();
    iassert(Sys_InterlockedIncrement(&g_markThread[localClientNum]) == 1);

    FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);
    system = FX_GetSystem(localClientNum);
    camera = system;
    marksSystem->hasCarryIndex = 0;
    indexCount = 0;
    markEnd = (FxMark *)marksSystem->triGroups;
    for (mark = marksSystem->marks; mark != markEnd; ++mark)
    {
        if (mark->frameCountDrawn != -1 && mark->prevMark == FX_HANDLE_NONE && (mark->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == 0x40)
        {
            if (smodelVisLods[mark->context.modelIndex])
            {
                FX_GenerateMarkVertsForList_WorldXModel(marksSystem, FX_MarkToHandle(marksSystem, mark), &camera->camera, &indexCount);
            }
        }
    }

    FX_FinishGeneratingMarkVerts(marksSystem);
    iassert(Sys_InterlockedDecrement(&g_markThread[localClientNum]) == 0);
    R_EndMarkMeshVerts();
}

void __cdecl FX_GenerateMarkVertsForWorld(int32_t localClientNum)
{
    if (fx_marks->current.enabled)
    {
        PROF_SCOPED("FX_GenMarkVertsWorld");
        R_BeginMarkMeshVerts();
        iassert(Sys_InterlockedIncrement(&g_markThread[localClientNum]) == 1);

        FxMarksSystem *marksSystem = FX_GetMarksSystem(localClientNum);

        marksSystem->hasCarryIndex = 0;
        uint indexCount = 0;
        FX_GenerateMarkVertsForList_WorldBrush(
            marksSystem,
            marksSystem->firstActiveWorldMarkHandle,
            &FX_GetSystem(localClientNum)->camera,
            &indexCount);
        FX_FinishGeneratingMarkVerts(marksSystem);

        iassert(Sys_InterlockedDecrement(&g_markThread[localClientNum]) == 0);
        R_EndMarkMeshVerts();
    }
}
