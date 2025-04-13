#include "fx_system.h"

#include <qcommon/mem_track.h>

#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_marks.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>

#include <client_mp/client_mp.h>

#include <win32/win_local.h>

#include <algorithm>

FxMarkPoint g_fxMarkPoints[765];

static long g_markThread[1];

void __cdecl TRACK_fx_marks()
{
    track_static_alloc_internal(g_fxMarkPoints, 24480, "g_fxMarkPoints", 8);
}

void __cdecl FX_InitMarksSystem(FxMarksSystem *marksSystem)
{
    int pointIndex; // [esp+8h] [ebp-10h]
    unsigned int markIndex; // [esp+Ch] [ebp-Ch]
    int triIndex; // [esp+10h] [ebp-8h]
    unsigned int markHandleIndex; // [esp+14h] [ebp-4h]

    for (markHandleIndex = 0; markHandleIndex != 1024; ++markHandleIndex)
        marksSystem->entFirstMarkHandles[markHandleIndex] = -1;
    marksSystem->firstFreeMarkHandle = FX_MarkToHandle(marksSystem, marksSystem->marks);
    for (markIndex = 0; markIndex < 0x1FF; ++markIndex)
    {
        marksSystem->marks[markIndex].prevMark = -1;
        marksSystem->marks[markIndex].nextMark = FX_MarkToHandle(marksSystem, &marksSystem->marks[markIndex + 1]);
        marksSystem->marks[markIndex].frameCountDrawn = -1;
    }
    marksSystem->marks[markIndex].prevMark = -1;
    marksSystem->marks[markIndex].nextMark = -1;
    marksSystem->marks[markIndex].frameCountDrawn = -1;
    marksSystem->firstFreeTriGroup = marksSystem->triGroups;
    for (triIndex = 0; triIndex < 2047; ++triIndex)
        marksSystem->triGroups[triIndex].nextFreeTriGroup = &marksSystem->triGroups[triIndex + 1];
    marksSystem->triGroups[triIndex].nextFreeTriGroup = 0;
    marksSystem->firstFreePointGroup = marksSystem->pointGroups;
    for (pointIndex = 0; pointIndex < 3071; ++pointIndex)
        marksSystem->pointGroups[pointIndex].nextFreePointGroup = &marksSystem->pointGroups[pointIndex + 1];
    marksSystem->pointGroups[pointIndex].nextFreePointGroup = 0;
    marksSystem->firstActiveWorldMarkHandle = -1;
    marksSystem->allocedMarkCount = 0;
    marksSystem->freedMarkCount = 0;
}

unsigned __int16 __cdecl FX_MarkToHandle(FxMarksSystem *marksSystem, FxMark *mark)
{
    unsigned __int16 handle; // [esp+0h] [ebp-4h]

    if (!marksSystem)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 152, 0, "%s", "marksSystem");
    if (!mark)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 153, 0, "%s", "mark");
    handle = mark - marksSystem->marks;
    if (handle >= 0x200u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            156,
            0,
            "handle doesn't index FX_MARKS_LIMIT\n\t%i not in [0, %i)",
            handle,
            512);
    return mark - marksSystem->marks;
}

void __cdecl FX_BeginMarks(int clientIndex)
{
    if (clientIndex)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            clientIndex);
    if (++fx_marksSystemPool[0].frameCount <= 0)
        fx_marksSystemPool[0].frameCount = 1;
}

void __cdecl FX_CreateImpactMark(
    int localClientNum,
    const FxElemDef *elemDef,
    const FxSpatialFrame *spatialFrame,
    int randomSeed,
    unsigned int markEntnum)
{
    FxElemVisualState visState; // [esp+50h] [ebp-3Ch] BYREF
    FxElemPreVisualState preVisState; // [esp+6Ch] [ebp-20h] BYREF
    FxElemMarkVisuals *markVisuals; // [esp+88h] [ebp-4h]

    //Profile_Begin(204);
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
        spatialFrame->origin,
        spatialFrame->quat,
        visState.rotationTotal,
        visState.color,
        visState.size[0],
        markEntnum);
    //Profile_EndInternal(0);
}

void __cdecl FX_ImpactMark(
    int localClientNum,
    Material *worldMaterial,
    Material *modelMaterial,
    float *origin,
    const float *quat,
    float orientation,
    const unsigned __int8 *nativeColor,
    float radius,
    unsigned int markEntnum)
{
    float degrees; // [esp+10h] [ebp-70h]
    float axis[3][3]; // [esp+5Ch] [ebp-24h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    if (fx_marks->current.enabled && !fx_marksSystemPool[0].noMarks && radius >= 0.1000000014901161)
    {
        if (radius <= 0.0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 784, 0, "%s\n\t(radius) = %g", "(radius > 0)", radius);
        UnitQuatToAxis(quat, axis);
        degrees = orientation * 57.2957763671875;
        RotatePointAroundVector(axis[2], axis[0], axis[1], degrees);
        Vec3Cross(axis[0], axis[2], axis[1]);
        //Profile_Begin(205);
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
        //Profile_EndInternal(0);
        if (fx_marks_smodels->current.enabled || fx_marks_ents->current.enabled)
        {
            //Profile_Begin(206);
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
            //Profile_EndInternal(0);
        }
    }
}

struct FX_ImpactMark_Generate_CB
{
    int localClientNum;
    Material* material;
    float radius;
    const byte* nativeColor;
};

void __cdecl FX_ImpactMark_Generate(
    int localClientNum,
    MarkFragmentsAgainstEnum markAgainst,
    Material *material,
    float *origin,
    const float (*axis)[3],
    float orientation,
    const byte *nativeColor,
    float radius,
    unsigned int markEntnum)
{
    FxMarkTri tris; // [esp+230h] [ebp-1058h] BYREF
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
            if ((unsigned int)markAgainst > MARK_FRAGMENTS_AGAINST_MODELS)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_marks.cpp",
                    752,
                    0,
                    "%s\n\t(markAgainst) = %i",
                    "(markAgainst == MARK_FRAGMENTS_AGAINST_MODELS || markAgainst == MARK_FRAGMENTS_AGAINST_BRUSHES)",
                    markAgainst);
            if (markAgainst == MARK_FRAGMENTS_AGAINST_MODELS)
                FX_ImpactMark_Generate_AddEntityModel(localClientNum, &markInfo, markEntnum, origin, radius);
            else
                FX_ImpactMark_Generate_AddEntityBrush(localClientNum, &markInfo, markEntnum, origin, radius);
        }
        R_MarkFragments_Go(&markInfo, FX_ImpactMark_Generate_Callback, &callbackContext, 255, &tris, 765, g_fxMarkPoints);
    }
}

void __cdecl FX_ImpactMark_Generate_AddEntityBrush(
    int localClientNum,
    MarkInfo *markInfo,
    unsigned int entityIndex,
    const float *origin,
    float radius)
{
    float s; // [esp+0h] [ebp-25Ch]
    int v6; // [esp+8h] [ebp-254h]
    int v7; // [esp+Ch] [ebp-250h]
    int v8; // [esp+10h] [ebp-24Ch]
    int v9; // [esp+14h] [ebp-248h]
    int v10; // [esp+18h] [ebp-244h]
    int v11; // [esp+1Ch] [ebp-240h]
    int v12; // [esp+20h] [ebp-23Ch]
    int v13; // [esp+24h] [ebp-238h]
    int v14; // [esp+28h] [ebp-234h]
    int v15; // [esp+2Ch] [ebp-230h]
    int v16; // [esp+30h] [ebp-22Ch]
    int v17; // [esp+34h] [ebp-228h]
    float v18; // [esp+D0h] [ebp-18Ch]
    float v19; // [esp+120h] [ebp-13Ch]
    float v20; // [esp+130h] [ebp-12Ch]
    float v21; // [esp+174h] [ebp-E8h]
    float v22; // [esp+178h] [ebp-E4h]
    float v23; // [esp+17Ch] [ebp-E0h]
    float v24; // [esp+180h] [ebp-DCh]
    float entAxis[3][3]; // [esp+1B8h] [ebp-A4h] BYREF
    unsigned __int16 entityIndexAsUnsignedShort; // [esp+1DCh] [ebp-80h]
    float4 worldModelBoundsFloat4[2]; // [esp+1E0h] [ebp-7Ch]
    float markMins[3]; // [esp+200h] [ebp-5Ch] BYREF
    float markMaxs[3]; // [esp+20Ch] [ebp-50h] BYREF
    float4 modelBounds[2]; // [esp+218h] [ebp-44h]
    GfxBrushModel *brushModel; // [esp+23Ch] [ebp-20h]
    centity_s *ent; // [esp+240h] [ebp-1Ch]
    float worldModelBoundsVec3[2][3]; // [esp+244h] [ebp-18h] BYREF

    if (entityIndex != 1023)
    {
        //Profile_Begin(207);
        s = -radius;
        Vec3AddScalar(origin, s, markMins);
        Vec3AddScalar(origin, radius, markMaxs);
        ent = CG_GetEntity(localClientNum, entityIndex);
        if (ent->nextValid
            && (clientActive_t *)ent->nextState.solid == (clientActive_t *)((char *)&clients[0].parseClients[238].attachTagIndex[4]
                + 3))
        {
            brushModel = R_GetBrushModel(ent->nextState.index.brushmodel);
            AnglesToAxis(ent->pose.angles, entAxis);
            modelBounds[0].v[0] = brushModel->bounds[0][0];
            modelBounds[0].v[1] = brushModel->bounds[0][1];
            modelBounds[0].v[2] = brushModel->bounds[0][2];
            modelBounds[0].v[3] = 0.0;
            modelBounds[1].v[0] = brushModel->bounds[1][0];
            modelBounds[1].v[1] = brushModel->bounds[1][1];
            modelBounds[1].v[2] = brushModel->bounds[1][2];
            modelBounds[1].v[3] = 0.0;
            v19 = 0.0;
            v18 = 0.0;
            v20 = 0.0;
            v21 = ent->pose.origin[0];
            v22 = ent->pose.origin[1];
            v23 = ent->pose.origin[2];
            v24 = 0.0;
            if (entAxis[0][0] >= 0.0)
                v17 = 0;
            else
                v17 = -1;
            if (entAxis[0][1] >= 0.0)
                v16 = 0;
            else
                v16 = -1;
            if (entAxis[0][2] >= 0.0)
                v15 = 0;
            else
                v15 = -1;
            if (v19 >= 0.0)
                v14 = 0;
            else
                v14 = -1;
            if (entAxis[1][0] >= 0.0)
                v13 = 0;
            else
                v13 = -1;
            if (entAxis[1][1] >= 0.0)
                v12 = 0;
            else
                v12 = -1;
            if (entAxis[1][2] >= 0.0)
                v11 = 0;
            else
                v11 = -1;
            if (v18 >= 0.0)
                v10 = 0;
            else
                v10 = -1;
            if (entAxis[2][0] >= 0.0)
                v9 = 0;
            else
                v9 = -1;
            if (entAxis[2][1] >= 0.0)
                v8 = 0;
            else
                v8 = -1;
            if (entAxis[2][2] >= 0.0)
                v7 = 0;
            else
                v7 = -1;
            if (v20 >= 0.0)
                v6 = 0;
            else
                v6 = -1;
            worldModelBoundsFloat4[0].v[0] = COERCE_FLOAT(modelBounds[1].u[0] & v17 | modelBounds[0].u[0] & ~v17)
                * entAxis[0][0]
                + v21;
            worldModelBoundsFloat4[0].v[1] = COERCE_FLOAT(modelBounds[1].u[0] & v16 | modelBounds[0].u[0] & ~v16)
                * entAxis[0][1]
                + v22;
            worldModelBoundsFloat4[0].v[2] = COERCE_FLOAT(modelBounds[1].u[0] & v15 | modelBounds[0].u[0] & ~v15)
                * entAxis[0][2]
                + v23;
            worldModelBoundsFloat4[0].v[3] = COERCE_FLOAT(modelBounds[1].u[0] & v14 | modelBounds[0].u[0] & ~v14) * v19 + v24;
            worldModelBoundsFloat4[0].v[0] = COERCE_FLOAT(modelBounds[1].u[1] & v13 | modelBounds[0].u[1] & ~v13)
                * entAxis[1][0]
                + worldModelBoundsFloat4[0].v[0];
            worldModelBoundsFloat4[0].v[1] = COERCE_FLOAT(modelBounds[1].u[1] & v12 | modelBounds[0].u[1] & ~v12)
                * entAxis[1][1]
                + worldModelBoundsFloat4[0].v[1];
            worldModelBoundsFloat4[0].v[2] = COERCE_FLOAT(modelBounds[1].u[1] & v11 | modelBounds[0].u[1] & ~v11)
                * entAxis[1][2]
                + worldModelBoundsFloat4[0].v[2];
            worldModelBoundsFloat4[0].v[3] = COERCE_FLOAT(modelBounds[1].u[1] & v10 | modelBounds[0].u[1] & ~v10) * v18
                + worldModelBoundsFloat4[0].v[3];
            worldModelBoundsFloat4[0].v[0] = COERCE_FLOAT(modelBounds[1].u[2] & v9 | modelBounds[0].u[2] & ~v9)
                * entAxis[2][0]
                + worldModelBoundsFloat4[0].v[0];
            worldModelBoundsFloat4[0].v[1] = COERCE_FLOAT(modelBounds[1].u[2] & v8 | modelBounds[0].u[2] & ~v8)
                * entAxis[2][1]
                + worldModelBoundsFloat4[0].v[1];
            worldModelBoundsFloat4[0].v[2] = COERCE_FLOAT(modelBounds[1].u[2] & v7 | modelBounds[0].u[2] & ~v7)
                * entAxis[2][2]
                + worldModelBoundsFloat4[0].v[2];
            worldModelBoundsFloat4[0].v[3] = COERCE_FLOAT(modelBounds[1].u[2] & v6 | modelBounds[0].u[2] & ~v6) * v20
                + worldModelBoundsFloat4[0].v[3];
            worldModelBoundsFloat4[1].v[0] = COERCE_FLOAT(modelBounds[0].u[0] & v17 | modelBounds[1].u[0] & ~v17)
                * entAxis[0][0]
                + v21;
            worldModelBoundsFloat4[1].v[1] = COERCE_FLOAT(modelBounds[0].u[0] & v16 | modelBounds[1].u[0] & ~v16)
                * entAxis[0][1]
                + v22;
            worldModelBoundsFloat4[1].v[2] = COERCE_FLOAT(modelBounds[0].u[0] & v15 | modelBounds[1].u[0] & ~v15)
                * entAxis[0][2]
                + v23;
            worldModelBoundsFloat4[1].v[3] = COERCE_FLOAT(modelBounds[0].u[0] & v14 | modelBounds[1].u[0] & ~v14) * v19 + v24;
            worldModelBoundsFloat4[1].v[0] = COERCE_FLOAT(modelBounds[0].u[1] & v13 | modelBounds[1].u[1] & ~v13)
                * entAxis[1][0]
                + worldModelBoundsFloat4[1].v[0];
            worldModelBoundsFloat4[1].v[1] = COERCE_FLOAT(modelBounds[0].u[1] & v12 | modelBounds[1].u[1] & ~v12)
                * entAxis[1][1]
                + worldModelBoundsFloat4[1].v[1];
            worldModelBoundsFloat4[1].v[2] = COERCE_FLOAT(modelBounds[0].u[1] & v11 | modelBounds[1].u[1] & ~v11)
                * entAxis[1][2]
                + worldModelBoundsFloat4[1].v[2];
            worldModelBoundsFloat4[1].v[3] = COERCE_FLOAT(modelBounds[0].u[1] & v10 | modelBounds[1].u[1] & ~v10) * v18
                + worldModelBoundsFloat4[1].v[3];
            worldModelBoundsFloat4[1].v[0] = COERCE_FLOAT(modelBounds[0].u[2] & v9 | modelBounds[1].u[2] & ~v9)
                * entAxis[2][0]
                + worldModelBoundsFloat4[1].v[0];
            worldModelBoundsFloat4[1].v[1] = COERCE_FLOAT(modelBounds[0].u[2] & v8 | modelBounds[1].u[2] & ~v8)
                * entAxis[2][1]
                + worldModelBoundsFloat4[1].v[1];
            worldModelBoundsFloat4[1].v[2] = COERCE_FLOAT(modelBounds[0].u[2] & v7 | modelBounds[1].u[2] & ~v7)
                * entAxis[2][2]
                + worldModelBoundsFloat4[1].v[2];
            worldModelBoundsFloat4[1].v[3] = COERCE_FLOAT(modelBounds[0].u[2] & v6 | modelBounds[1].u[2] & ~v6) * v20
                + worldModelBoundsFloat4[1].v[3];
            worldModelBoundsVec3[0][0] = worldModelBoundsFloat4[0].v[0];
            worldModelBoundsVec3[0][1] = worldModelBoundsFloat4[0].v[1];
            worldModelBoundsVec3[0][2] = worldModelBoundsFloat4[0].v[2];
            worldModelBoundsVec3[1][0] = worldModelBoundsFloat4[1].v[0];
            worldModelBoundsVec3[1][1] = worldModelBoundsFloat4[1].v[1];
            worldModelBoundsVec3[1][2] = worldModelBoundsFloat4[1].v[2];
            if (BoundsOverlap(markMins, markMaxs, worldModelBoundsVec3[0], worldModelBoundsVec3[1]))
            {
                entityIndexAsUnsignedShort = entityIndex;
                if ((unsigned __int16)entityIndex != entityIndex)
                    MyAssertHandler(
                        ".\\EffectsCore\\fx_marks.cpp",
                        653,
                        0,
                        "%s\n\t(entityIndex) = %i",
                        "(entityIndexAsUnsignedShort == entityIndex)",
                        entityIndex);
                R_MarkFragments_AddBModel(markInfo, brushModel, &ent->pose, entityIndexAsUnsignedShort);
            }
            //Profile_EndInternal(0);
        }
        else
        {
            //Profile_EndInternal(0);
        }
    }
}

void __cdecl FX_ImpactMark_Generate_AddEntityModel(
    int localClientNum,
    MarkInfo *markInfo,
    unsigned int entityIndex,
    const float *origin,
    float radius)
{
    double v5; // st7
    float diff[8]; // [esp+38h] [ebp-38h] BYREF
    unsigned __int16 entityIndexAsUnsignedShort; // [esp+58h] [ebp-18h]
    float dObjRadius; // [esp+5Ch] [ebp-14h]
    float summedRadiusSquared; // [esp+60h] [ebp-10h]
    centity_s *ent; // [esp+64h] [ebp-Ch]
    float summedRadius; // [esp+68h] [ebp-8h]
    DObj_s *dObj; // [esp+6Ch] [ebp-4h]

    if (entityIndex != 1023)
    {
        //Profile_Begin(207);
        ent = CG_GetEntity(localClientNum, entityIndex);
        if (ent->nextValid && (dObj = Com_GetClientDObj(ent->nextState.number, localClientNum)) != 0)
        {
            dObjRadius = DObjGetRadius(dObj);
            summedRadius = dObjRadius + radius;
            summedRadiusSquared = summedRadius * summedRadius;
            Vec3Sub(ent->pose.origin, origin, diff);
            v5 = Vec3LengthSq(diff);
            if (summedRadiusSquared >= v5)
            {
                entityIndexAsUnsignedShort = entityIndex;
                if ((unsigned __int16)entityIndex != entityIndex)
                    MyAssertHandler(
                        ".\\EffectsCore\\fx_marks.cpp",
                        703,
                        0,
                        "%s\n\t(entityIndex) = %i",
                        "(entityIndexAsUnsignedShort == entityIndex)",
                        entityIndex);
                R_MarkFragments_AddDObj(markInfo, dObj, &ent->pose, entityIndexAsUnsignedShort);
            }
            //Profile_EndInternal(0);
        }
        else
        {
            //Profile_EndInternal(0);
        }
    }
}

void __cdecl FX_ImpactMark_Generate_Callback(
    void *context_p,
    int triCount,
    FxMarkTri *tris,
    int pointCount,
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

void __cdecl FX_AllocAndConstructMark(
    int localClientNum,
    int triCount,
    int pointCount,
    Material *material,
    FxMarkTri *markTris,
    const FxMarkPoint *markPoints,
    const float *origin,
    float radius,
    const float *texCoordAxis,
    const unsigned __int8 *nativeColor)
{
    float *v10; // [esp+0h] [ebp-38h]
    float *v11; // [esp+4h] [ebp-34h]
    unsigned __int16 staticModelMarkHead; // [esp+1Ch] [ebp-1Ch] BYREF
    unsigned __int16 newMarkHandle; // [esp+20h] [ebp-18h]
    FxMarksSystem *marksSystem; // [esp+24h] [ebp-14h]
    int points; // [esp+28h] [ebp-10h]
    int modelType; // [esp+2Ch] [ebp-Ch]
    FxMark *newMark; // [esp+30h] [ebp-8h]
    int tris; // [esp+34h] [ebp-4h]

    std::sort(
        markTris,
        &markTris[triCount],
        FX_CompareMarkTris);
    Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);
    if (InterlockedIncrement(&g_markThread[localClientNum]) != 1)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            505,
            0,
            "%s",
            "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    marksSystem = fx_marksSystemPool;
    tris = FX_AllocMarkTris(fx_marksSystemPool, markTris, triCount);
    points = FX_AllocMarkPoints(fx_marksSystemPool, pointCount);
    if (fx_marksSystemPool[0].firstFreeMarkHandle == 0xFFFF)
    {
        FX_FreeLruMark(marksSystem);
        if (marksSystem->firstFreeMarkHandle == 0xFFFF)
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                517,
                0,
                "%s",
                "marksSystem->firstFreeMarkHandle != FX_HANDLE_NONE");
    }
    newMarkHandle = marksSystem->firstFreeMarkHandle;
    newMark = FX_MarkFromHandle(marksSystem, newMarkHandle);
    if (!newMark)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 521, 0, "%s", "newMark");
    marksSystem->firstFreeMarkHandle = newMark->nextMark;
    if (marksSystem->frameCount <= 0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            528,
            0,
            "%s\n\t(marksSystem->frameCount) = %i",
            "(marksSystem->frameCount > 0)",
            marksSystem->frameCount);
    newMark->context = markTris->context;
    newMark->material = material;
    newMark->radius = radius;
    v11 = newMark->origin;
    newMark->origin[0] = *origin;
    v11[1] = origin[1];
    v11[2] = origin[2];
    v10 = newMark->texCoordAxis;
    newMark->texCoordAxis[0] = *texCoordAxis;
    v10[1] = texCoordAxis[1];
    v10[2] = texCoordAxis[2];
    *(unsigned int *)newMark->nativeColor = *(unsigned int *)nativeColor;
    modelType = newMark->context.modelTypeAndSurf & 0xC0;
    if (modelType == 192 || modelType == 128)
    {
        FX_LinkMarkIntoList(marksSystem, &marksSystem->entFirstMarkHandles[newMark->context.modelIndex], newMark);
    }
    else if (modelType == 64)
    {
        staticModelMarkHead = FX_FindModelHead(marksSystem, newMark->context.modelIndex, 64);
        if (staticModelMarkHead == newMarkHandle)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 555, 0, "%s", "staticModelMarkHead != newMarkHandle");
        FX_LinkMarkIntoList(marksSystem, &staticModelMarkHead, newMark);
    }
    else
    {
        if (modelType)
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                566,
                0,
                "%s\n\t(modelType) = %i",
                "(modelType == MARK_MODEL_TYPE_WORLD_BRUSH)",
                modelType);
        FX_LinkMarkIntoList(marksSystem, &marksSystem->firstActiveWorldMarkHandle, newMark);
    }
    if (newMark->frameCountDrawn != -1)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 573, 0, "%s", "newMark->frameCountDrawn == FX_MARK_FREE");
    newMark->frameCountDrawn = marksSystem->frameCount - 1;
    newMark->frameCountAlloced = marksSystem->frameCount;
    newMark->tris = tris;
    newMark->triCount = triCount;
    if (newMark->triCount != triCount)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            580,
            0,
            "newMark->triCount == triCount\n\t%i, %i",
            newMark->triCount,
            triCount);
    newMark->points = points;
    newMark->pointCount = pointCount;
    if (newMark->pointCount != pointCount)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            584,
            0,
            "newMark->pointCount == pointCount\n\t%i, %i",
            newMark->pointCount,
            pointCount);
    if (InterlockedDecrement(&g_markThread[localClientNum]))
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            586,
            0,
            "%s",
            "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0");
    Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
    FX_CopyMarkTris(marksSystem, markTris, newMark->tris, triCount);
    FX_CopyMarkPoints(marksSystem, markPoints, newMark->points, pointCount);
    ++marksSystem->allocedMarkCount;
}

FxMark *__cdecl FX_MarkFromHandle(FxMarksSystem *marksSystem, unsigned __int16 handle)
{
    if (handle >= 0x200u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            169,
            0,
            "handle doesn't index FX_MARKS_LIMIT\n\t%i not in [0, %i)",
            handle,
            512);
    if (!marksSystem)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 170, 0, "%s", "marksSystem");
    return &marksSystem->marks[handle];
}

void __cdecl FX_FreeLruMark(FxMarksSystem *marksSystem)
{
    FxMark *lruMark; // [esp+34h] [ebp-Ch]
    FxMark *mark; // [esp+38h] [ebp-8h]

    //Profile_Begin(208);
    if (!marksSystem)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 241, 0, "%s", "marksSystem");
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
    //Profile_EndInternal(0);
}

void __cdecl FX_FreeMark(FxMarksSystem *marksSystem, FxMark *mark)
{
    switch (mark->context.modelTypeAndSurf & 0xC0)
    {
    case 0:
        FX_FreeMarkFromList(marksSystem, mark, &marksSystem->firstActiveWorldMarkHandle);
        break;
    case 0x40:
        FX_FreeMarkFromList(marksSystem, mark, 0);
        break;
    case 0x80:
    case 0xC0:
        FX_FreeMarkFromList(marksSystem, mark, &marksSystem->entFirstMarkHandles[mark->context.modelIndex]);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 226, 0, "Unhandled case.\n");
        break;
    }
    ++marksSystem->freedMarkCount;
}

void __cdecl FX_FreeMarkFromList(FxMarksSystem *marksSystem, FxMark *mark, unsigned __int16 *listHead)
{
    const char *v3; // eax
    unsigned __int16 markHandle; // [esp+4h] [ebp-4h]

    markHandle = FX_MarkToHandle(marksSystem, mark);
    FX_FreeMarkTriGroups(marksSystem, mark);
    FX_FreeMarkPointGroups(marksSystem, mark);
    if (mark->nextMark != 0xFFFF)
        FX_MarkFromHandle(marksSystem, mark->nextMark)->prevMark = mark->prevMark;
    if (mark->prevMark == 0xFFFF)
    {
        if (listHead)
        {
            if (*listHead != markHandle)
            {
                v3 = va("%i %i", *listHead, markHandle);
                MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 195, 0, "%s\n\t%s", "*listHead == markHandle", v3);
            }
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

void __cdecl FX_FreeMarkTriGroups(FxMarksSystem *marksSystem, FxMark *mark)
{
    unsigned int groupHandle; // [esp+4h] [ebp-Ch]
    FxTriGroupPool *group; // [esp+Ch] [ebp-4h]

    groupHandle = mark->tris;
    do
    {
        group = FX_TriGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->triGroup.next;
        group->nextFreeTriGroup = marksSystem->firstFreeTriGroup;
        marksSystem->firstFreeTriGroup = group;
    } while (groupHandle != 0xFFFF);
}

FxTriGroupPool *__cdecl FX_TriGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle)
{
    if (handle >= 0xC000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            193,
            0,
            "handle doesn't index FX_TRI_GROUP_LIMIT * sizeof( FxTriGroup )\n\t%i not in [0, %i)",
            handle,
            49152);
    if (!marksSystem)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 194, 0, "%s", "marksSystem");
    return (FxTriGroupPool *)((char *)marksSystem->triGroups + handle);
}

void __cdecl FX_FreeMarkPointGroups(FxMarksSystem *marksSystem, FxMark *mark)
{
    unsigned int groupHandle; // [esp+4h] [ebp-Ch]
    FxPointGroupPool *group; // [esp+Ch] [ebp-4h]

    groupHandle = mark->points;
    do
    {
        group = FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->pointGroup.next;
        group->nextFreePointGroup = marksSystem->firstFreePointGroup;
        marksSystem->firstFreePointGroup = group;
    } while (groupHandle != 0xFFFF);
}

FxPointGroupPool *__cdecl FX_PointGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle)
{
    if (handle >= 0x33000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            218,
            0,
            "handle doesn't index FX_POINT_GROUP_LIMIT * sizeof( FxPointGroup )\n\t%i not in [0, %i)",
            handle,
            208896);
    if (!marksSystem)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 219, 0, "%s", "marksSystem");
    return (FxPointGroupPool *)((char *)marksSystem->pointGroups + handle);
}

int __cdecl FX_AllocMarkTris(FxMarksSystem *marksSystem, const FxMarkTri *markTris, int triCount)
{
    int groupHandle; // [esp+14h] [ebp-Ch]
    int usedCount; // [esp+18h] [ebp-8h]
    FxTriGroupPool *newGroup; // [esp+1Ch] [ebp-4h]

    groupHandle = 0xFFFF;
    do
    {
        if (!marksSystem->firstFreeTriGroup)
        {
            FX_FreeLruMark(marksSystem);
            if (!marksSystem->firstFreeTriGroup)
                MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 272, 0, "%s", "marksSystem->firstFreeTriGroup");
        }
        newGroup = marksSystem->firstFreeTriGroup;
        marksSystem->firstFreeTriGroup = newGroup->nextFreeTriGroup;
        newGroup->triGroup.next = groupHandle;
        groupHandle = FX_TriGroupToHandle(marksSystem, (FxTriGroup *)newGroup);
        if (triCount >= 2 && !memcmp((const char *)&markTris->context, (const char *)&markTris[1].context, 6))
            usedCount = 2;
        else
            usedCount = 1;
        markTris += usedCount;
        triCount -= usedCount;
    } while (triCount);
    return groupHandle;
}

int __cdecl FX_TriGroupToHandle(FxMarksSystem *marksSystem, FxTriGroup *group)
{
    unsigned int handle; // [esp+0h] [ebp-4h]

    if (!marksSystem)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 180, 0, "%s", "marksSystem");
    if (!group)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 181, 0, "%s", "group");
    handle = (char *)group - (char *)marksSystem->triGroups;
    if (handle >= 0xC000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            184,
            0,
            "handle doesn't index FX_TRI_GROUP_LIMIT * sizeof( FxTriGroup )\n\t%i not in [0, %i)",
            handle,
            49152);
    return (char *)group - (char *)marksSystem->triGroups;
}

int __cdecl FX_AllocMarkPoints(FxMarksSystem *marksSystem, int pointCount)
{
    int groupHandle; // [esp+4h] [ebp-Ch]
    FxPointGroupPool *newGroup; // [esp+8h] [ebp-8h]
    int pointGroupCount; // [esp+Ch] [ebp-4h]

    groupHandle = 0xFFFF;
    pointGroupCount = (pointCount + 1) / 2;
    if (pointGroupCount < 1)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            300,
            0,
            "%s\n\t(pointGroupCount) = %i",
            "(pointGroupCount >= 1)",
            pointGroupCount);
    do
    {
        if (!marksSystem->firstFreePointGroup)
        {
            FX_FreeLruMark(marksSystem);
            if (!marksSystem->firstFreePointGroup)
                MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 306, 0, "%s", "marksSystem->firstFreePointGroup");
        }
        newGroup = marksSystem->firstFreePointGroup;
        marksSystem->firstFreePointGroup = newGroup->nextFreePointGroup;
        newGroup->pointGroup.next = groupHandle;
        groupHandle = FX_PointGroupToHandle(marksSystem, (FxPointGroup *)newGroup);
        --pointGroupCount;
    } while (pointGroupCount);
    return groupHandle;
}

int __cdecl FX_PointGroupToHandle(FxMarksSystem *marksSystem, FxPointGroup *group)
{
    unsigned int handle; // [esp+0h] [ebp-4h]

    if (!marksSystem)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 205, 0, "%s", "marksSystem");
    if (!group)
        MyAssertHandler("c:\\trees\\cod3\\src\\effectscore\\fx_marks.h", 206, 0, "%s", "group");
    handle = (char *)group - (char *)marksSystem->pointGroups;
    if (handle >= 0x33000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            209,
            0,
            "handle doesn't index FX_POINT_GROUP_LIMIT * sizeof( FxPointGroup )\n\t%i not in [0, %i)",
            handle,
            208896);
    return (char *)group - (char *)marksSystem->pointGroups;
}

void __cdecl FX_LinkMarkIntoList(FxMarksSystem *marksSystem, unsigned __int16 *head, FxMark *mark)
{
    float diff[3]; // [esp+14h] [ebp-28h] BYREF
    unsigned __int16 iterMarkPrev; // [esp+20h] [ebp-1Ch]
    float radiusSum; // [esp+24h] [ebp-18h]
    unsigned __int16 *iterHandlePrev; // [esp+28h] [ebp-14h]
    FxMark *nextMark; // [esp+2Ch] [ebp-10h]
    FxMark *iterMark; // [esp+30h] [ebp-Ch]
    float distSq; // [esp+34h] [ebp-8h]
    unsigned __int16 markHandle; // [esp+38h] [ebp-4h]

    iterMarkPrev = -1;
    for (iterHandlePrev = head; *iterHandlePrev != 0xFFFF; iterHandlePrev = &iterMark->nextMark)
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
    if (mark->nextMark != 0xFFFF)
    {
        nextMark = FX_MarkFromHandle(marksSystem, mark->nextMark);
        if (nextMark->prevMark != mark->prevMark)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 364, 0, "%s", "nextMark->prevMark == mark->prevMark");
        nextMark->prevMark = markHandle;
    }
    *iterHandlePrev = markHandle;
}

void __cdecl FX_CopyMarkTris(
    FxMarksSystem *marksSystem,
    const FxMarkTri *srcTris,
    unsigned int dstGroupHandle,
    int triCount)
{
    int v4; // [esp+10h] [ebp-14h]
    int copyIndex; // [esp+1Ch] [ebp-8h]
    FxTriGroupPool *dstGroup; // [esp+20h] [ebp-4h]

    do
    {
        dstGroup = FX_TriGroupFromHandle(marksSystem, dstGroupHandle);
        if (triCount < 2)
            v4 = triCount;
        else
            v4 = 2;
        copyIndex = 0;
        dstGroup->triGroup.context = srcTris->context;
        do
        {
            dstGroup->triGroup.indices[copyIndex][0] = srcTris[copyIndex].indices[0];
            dstGroup->triGroup.indices[copyIndex][1] = srcTris[copyIndex].indices[1];
            dstGroup->triGroup.indices[copyIndex][2] = srcTris[copyIndex].indices[2];
            ++copyIndex;
        } while (copyIndex != v4
            && !memcmp((const char *)&srcTris[copyIndex].context, (const char *)&dstGroup->triGroup.context, 6));
        dstGroupHandle = dstGroup->triGroup.next;
        dstGroup->triGroup.triCount = copyIndex;
        srcTris += dstGroup->triGroup.triCount;
        triCount -= dstGroup->triGroup.triCount;
    } while (triCount);
    if (dstGroupHandle != 0xFFFF)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            399,
            0,
            "%s\n\t(dstGroupHandle) = %i",
            "(dstGroupHandle == 0xffff)",
            dstGroupHandle);
}

void __cdecl FX_CopyMarkPoints(
    FxMarksSystem *marksSystem,
    const FxMarkPoint *srcPoints,
    unsigned int dstGroupHandle,
    int pointCount)
{
    int v4; // [esp+8h] [ebp-14h]
    int copyIndex; // [esp+14h] [ebp-8h]
    FxPointGroupPool *dstGroup; // [esp+18h] [ebp-4h]

    do
    {
        dstGroup = FX_PointGroupFromHandle(marksSystem, dstGroupHandle);
        if (pointCount > 2)
            v4 = 2;
        else
            v4 = pointCount;
        if (v4 <= 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 414, 0, "%s\n\t(copyCount) = %i", "(copyCount > 0)", v4);
        copyIndex = 0;
        do
        {
            memcpy((char *)dstGroup + 32 * copyIndex, &srcPoints[copyIndex], 0x20u);
            ++copyIndex;
        } while (copyIndex < v4);
        dstGroupHandle = dstGroup->pointGroup.next;
        srcPoints += v4;
        pointCount -= v4;
    } while (pointCount);
    if (dstGroupHandle != 0xFFFF)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            426,
            0,
            "%s\n\t(dstGroupHandle) = %i",
            "(dstGroupHandle == 0xffff)",
            dstGroupHandle);
}

unsigned __int16 __cdecl FX_FindModelHead(FxMarksSystem *marksSystem, unsigned __int16 modelIndex, int type)
{
    FxMark *mark; // [esp+4h] [ebp-8h]

    for (mark = marksSystem->marks; mark != (FxMark *)marksSystem->triGroups; ++mark)
    {
        if (mark->frameCountDrawn != -1
            && mark->prevMark == 0xFFFF
            && (mark->context.modelTypeAndSurf & 0xC0) == type
            && mark->context.modelIndex == modelIndex)
        {
            return FX_MarkToHandle(marksSystem, mark);
        }
    }
    return -1;
}

bool __cdecl FX_CompareMarkTris(const FxMarkTri *tri0, const FxMarkTri *tri1)
{
    int contextCompareResult; // [esp+10h] [ebp-4h]

    contextCompareResult = FX_MarkContextsCompare(&tri0->context, &tri1->context);
    if (contextCompareResult)
        return contextCompareResult > 0;
    else
        return tri0->indices[0] < (int)tri1->indices[0];
}

int __cdecl FX_MarkContextsCompare(const GfxMarkContext *context0, const GfxMarkContext *context1)
{
    int type1; // [esp+0h] [ebp-10h]
    int type0; // [esp+8h] [ebp-8h]

    type0 = context0->modelTypeAndSurf & 0xC0;
    type1 = context1->modelTypeAndSurf & 0xC0;
    if (type0 != type1)
        return type1 - type0;
    if (context0->primaryLightIndex != context1->primaryLightIndex)
        return context1->primaryLightIndex - context0->primaryLightIndex;
    if (context0->reflectionProbeIndex != context1->reflectionProbeIndex)
        return context1->reflectionProbeIndex - context0->reflectionProbeIndex;
    if (context0->lmapIndex != context1->lmapIndex)
        return context1->lmapIndex - context0->lmapIndex;
    if (context0->modelIndex == context1->modelIndex)
        return (context1->modelTypeAndSurf & 0x3F) - (context0->modelTypeAndSurf & 0x3F);
    return context1->modelIndex - context0->modelIndex;
}

void __cdecl FX_MarkEntDetachAll(int localClientNum, int entnum)
{
    FxMark *v2; // eax

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    while (fx_marksSystemPool[0].entFirstMarkHandles[entnum] != 0xFFFF)
    {
        v2 = FX_MarkFromHandle(fx_marksSystemPool, fx_marksSystemPool[0].entFirstMarkHandles[entnum]);
        FX_FreeMark(fx_marksSystemPool, v2);
    }
}

void __cdecl FX_MarkEntUpdateHidePartBits(
    const unsigned int *oldHidePartBits,
    const unsigned int *newHidePartBits,
    int localClientNum,
    int entnum)
{
    unsigned int v4; // edx
    unsigned int unsetHidePartBits[4]; // [esp+8h] [ebp-18h] BYREF
    int hidePartIntIndex; // [esp+18h] [ebp-8h]
    unsigned int oredUnsetHidePartBits; // [esp+1Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    oredUnsetHidePartBits = 0;
    for (hidePartIntIndex = 0; hidePartIntIndex != 4; ++hidePartIntIndex)
    {
        v4 = newHidePartBits[hidePartIntIndex] & ~oldHidePartBits[hidePartIntIndex];
        unsetHidePartBits[hidePartIntIndex] = v4;
        oredUnsetHidePartBits |= v4;
    }
    if (oredUnsetHidePartBits)
        FX_MarkEntDetachMatchingBones(fx_marksSystemPool, entnum, unsetHidePartBits);
}

void __cdecl FX_MarkEntDetachMatchingBones(
    FxMarksSystem *marksSystem,
    int entnum,
    const unsigned int *unsetHidePartBits)
{
    unsigned __int16 handle; // [esp+18h] [ebp-Ch]
    FxMark *mark; // [esp+1Ch] [ebp-8h]
    int markBoneIndex; // [esp+20h] [ebp-4h]

    handle = marksSystem->entFirstMarkHandles[entnum];
    while (handle != 0xFFFF)
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ((mark->context.modelTypeAndSurf & 0xC0) == 0xC0)
        {
            markBoneIndex = mark->context.lmapIndex;
            if (markBoneIndex >> 5 >= 4)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_marks.cpp",
                    895,
                    0,
                    "%s\n\t(markBoneIndex) = %i",
                    "((markBoneIndex >> 5) < DOBJ_MAX_PART_BITS)",
                    markBoneIndex);
            if ((mark->context.modelTypeAndSurf & 0x3F) != 0)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_marks.cpp",
                    898,
                    0,
                    "%s\n\t((mark->context.modelTypeAndSurf & MARK_MODEL_SURF_MASK)) = %i",
                    "((mark->context.modelTypeAndSurf & MARK_MODEL_SURF_MASK) == 0)",
                    mark->context.modelTypeAndSurf & 0x3F);
            if ((unsetHidePartBits[markBoneIndex >> 5] & (0x80000000 >> (markBoneIndex & 0x1F))) != 0)
                FX_FreeMark(marksSystem, mark);
        }
    }
}

void __cdecl FX_MarkEntUpdateBegin(
    FxMarkDObjUpdateContext *context,
    DObj_s *obj,
    bool isBrush,
    unsigned __int16 brushIndex)
{
    int modelCount; // [esp+0h] [ebp-8h]
    int modelIndex; // [esp+4h] [ebp-4h]

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
    int localClientNum,
    int entnum,
    DObj_s *obj,
    bool isBrush,
    unsigned __int16 brushIndex)
{
    if (context->isBrush && (!isBrush || context->brushIndex != brushIndex))
        FX_MarkEntDetachAllOfType(localClientNum, entnum, 128);
    if (context->modelCount)
    {
        if (obj)
            FX_MarkEntUpdateEndDObj(context, localClientNum, entnum, obj);
        else
            FX_MarkEntDetachAllOfType(localClientNum, entnum, 192);
    }
}

void __cdecl FX_MarkEntDetachAllOfType(int localClientNum, int entnum, int markType)
{
    unsigned __int16 handle; // [esp+1Ch] [ebp-8h]
    FxMark *mark; // [esp+20h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    handle = fx_marksSystemPool[0].entFirstMarkHandles[entnum];
    while (handle != 0xFFFF)
    {
        mark = FX_MarkFromHandle(fx_marksSystemPool, handle);
        handle = mark->nextMark;
        if ((mark->context.modelTypeAndSurf & 0xC0) == markType)
            FX_FreeMark(fx_marksSystemPool, mark);
    }
}

void __cdecl FX_MarkEntUpdateEndDObj(FxMarkDObjUpdateContext *context, int localClientNum, int entnum, DObj_s *obj)
{
    int oldModelCount; // [esp+4h] [ebp-14h]
    int removedModelCount; // [esp+8h] [ebp-10h]
    int oldModelIndex; // [esp+Ch] [ebp-Ch]
    int modelCount; // [esp+10h] [ebp-8h]
    int modelIndex; // [esp+14h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
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
                FX_MarkEntDetachModel(fx_marksSystemPool, entnum, oldModelIndex - removedModelCount++);
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
            FX_MarkEntDetachModel(fx_marksSystemPool, entnum, oldModelIndex - removedModelCount++);
        }
    }
}

void __cdecl FX_MarkEntDetachModel(FxMarksSystem *marksSystem, int entnum, int oldModelIndex)
{
    unsigned __int16 handle; // [esp+18h] [ebp-Ch]
    FxMark *mark; // [esp+1Ch] [ebp-8h]
    int markModelIndex; // [esp+20h] [ebp-4h]

    handle = marksSystem->entFirstMarkHandles[entnum];
    while (handle != 0xFFFF)
    {
        mark = FX_MarkFromHandle(marksSystem, handle);
        handle = mark->nextMark;
        if ((mark->context.modelTypeAndSurf & 0xC0) == 0xC0)
        {
            markModelIndex = mark->context.modelTypeAndSurf & 0x3F;
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

void __cdecl FX_BeginGeneratingMarkVertsForEntModels(int localClientNum, unsigned int *indexCount)
{
    if (!fx_marks->current.enabled || !fx_marks_ents->current.enabled)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1633,
            0,
            "%s",
            "fx_marks->current.enabled && fx_marks_ents->current.enabled");
    //Profile_Begin(210);
    R_BeginMarkMeshVerts();
    if (InterlockedIncrement(&g_markThread[localClientNum]) != 1)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1638,
            0,
            "%s",
            "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    fx_marksSystemPool[0].hasCarryIndex = 0;
    *indexCount = 0;
    //Profile_EndInternal(0);
}

void __cdecl FX_GenerateMarkVertsForEntXModel(
    int localClientNum,
    int entId,
    unsigned int *indexCount,
    unsigned __int16 lightHandle,
    unsigned __int8 reflectionProbeIndex,
    const GfxScaledPlacement *placement)
{
    FxSystem *camera; // [esp+94h] [ebp-Ch]
    unsigned __int16 entMarkListHead; // [esp+9Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    entMarkListHead = fx_marksSystemPool[0].entFirstMarkHandles[entId];
    if (entMarkListHead != 0xFFFF)
    {
        //Profile_Begin(210);
        camera = FX_GetSystem(localClientNum);
        FX_GenerateMarkVertsForList_EntXModel(
            fx_marksSystemPool,
            entMarkListHead,
            &camera->camera,
            indexCount,
            lightHandle,
            reflectionProbeIndex,
            placement);
        //Profile_EndInternal(0);
    }
}

char __cdecl FX_GenerateMarkVertsForList_EntXModel(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount,
    unsigned __int16 lightHandleOverride,
    unsigned __int8 reflectionProbeIndexOverride,
    const GfxScaledPlacement *placement)
{
    __int64 v8; // [esp-8h] [ebp-14Ch]
    FxMark *mark; // [esp+F4h] [ebp-50h]
    FxActiveMarkSurf drawSurf; // [esp+F8h] [ebp-4Ch] BYREF
    unsigned __int16 markHandle; // [esp+10Ch] [ebp-38h]
    unsigned __int16 baseVertex; // [esp+110h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+114h] [ebp-30h] BYREF

    FX_GenerateMarkVertsForMark_MatrixFromScaledPlacement(placement, vec3_origin, (float (*)[3][3])transformMatrix);
    for (markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if ((mark->context.modelTypeAndSurf & 0xC0) == 0xC0)
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return 0;
            FX_GenerateMarkVertsForMark_SetLightHandle(&drawSurf, lightHandleOverride);
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndexOverride);
            FX_GenerateMarkVertsForMark_FinishAnimated(marksSystem, mark, baseVertex, &drawSurf, transformMatrix);
        }
    }
    return 1;
}

char __cdecl FX_GenerateMarkVertsForMark_Begin(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    unsigned int *indexCount,
    unsigned __int16 *outBaseVertex,
    FxActiveMarkSurf *outDrawSurf)
{
    unsigned int newIndexCount; // [esp+18h] [ebp-10h]
    unsigned int reserveIndexCount; // [esp+1Ch] [ebp-Ch]
    unsigned __int16 *indices; // [esp+20h] [ebp-8h]
    r_double_index_t *doubleIndices; // [esp+24h] [ebp-4h] BYREF

    newIndexCount = *indexCount + 3 * mark->triCount;
    reserveIndexCount = ((newIndexCount + 1) & 0xFFFFFFFE) - ((*indexCount + 1) & 0xFFFFFFFE);
    if (R_ReserveMarkMeshVerts(mark->pointCount, outBaseVertex)
        && R_ReserveMarkMeshIndices(reserveIndexCount, &doubleIndices))
    {
        indices = (unsigned __int16 *)doubleIndices - (*indexCount & 1);
        *indexCount = newIndexCount;
        if (mark->frameCountDrawn == -1)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1348, 0, "%s", "mark->frameCountDrawn != FX_MARK_FREE");
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

void __cdecl FX_DrawMarkTris(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex,
    unsigned __int16 *indices,
    FxActiveMarkSurf *outSurf)
{
    unsigned int groupHandle; // [esp+Ch] [ebp-10h]
    int triCount; // [esp+10h] [ebp-Ch]
    FxTriGroupPool *group; // [esp+14h] [ebp-8h]
    int triIndex; // [esp+18h] [ebp-4h]

    if (!mark)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1291, 0, "%s", "mark");
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
        if (triCount < group->triGroup.triCount)
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                1309,
                0,
                "triCount >= group->triCount\n\t%i, %i",
                triCount,
                group->triGroup.triCount);
        if (!group->triGroup.triCount)
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                1310,
                0,
                "%s\n\t(group->triCount) = %i",
                "(group->triCount > 0)",
                group->triGroup.triCount);
        triCount -= group->triGroup.triCount;
        triIndex = 0;
        do
            FX_EmitMarkTri(
                marksSystem,
                (const unsigned __int16 *)group + 3 * triIndex++,
                &group->triGroup.context,
                baseVertex,
                outSurf);
        while (triIndex != group->triGroup.triCount);
    } while (triCount);
    if (groupHandle != 0xFFFF)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1320,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle);
    if (!outSurf->indexCount)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1322, 0, "%s", "outSurf->indexCount");
}

void __cdecl FX_EmitMarkTri(
    FxMarksSystem *marksSystem,
    const unsigned __int16 *indices,
    const GfxMarkContext *markContext,
    unsigned __int16 baseVertex,
    FxActiveMarkSurf *outSurf)
{
    r_double_index_t *pIndex; // [esp+10h] [ebp-8h]
    r_double_index_t *pIndexa; // [esp+10h] [ebp-8h]
    r_double_index_t *pIndexb; // [esp+10h] [ebp-8h]
    r_double_index_t index; // [esp+14h] [ebp-4h]
    r_double_index_t indexa; // [esp+14h] [ebp-4h]
    r_double_index_t indexb; // [esp+14h] [ebp-4h]

    if (memcmp((const char *)&outSurf->context, (const char *)markContext, 6))
    {
        if (outSurf->indexCount)
        {
            R_AddMarkMeshDrawSurf(outSurf->material, &outSurf->context, outSurf->indices, outSurf->indexCount);
            outSurf->indices += outSurf->indexCount;
            outSurf->indexCount = 0;
        }
        if (outSurf->context.modelIndex != markContext->modelIndex)
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                1243,
                0,
                "%s",
                "outSurf->context.modelIndex == markContext->modelIndex");
        if ((outSurf->context.modelTypeAndSurf & 0xC0) != (markContext->modelTypeAndSurf & 0xC0))
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                1244,
                0,
                "%s",
                "(outSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == (markContext->modelTypeAndSurf & MARK_MODEL_TYPE_MASK)");
        outSurf->context = *markContext;
    }
    if (marksSystem->hasCarryIndex)
    {
        indexa.value[0] = marksSystem->carryIndex;
        indexa.value[1] = *indices + baseVertex;
        pIndexa = (r_double_index_t *)&outSurf->indices[outSurf->indexCount - 1];
        if (((unsigned __int8)pIndexa & 3) != 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1267, 0, "%s", "!((uint)pIndex & 3)");
        *pIndexa = indexa;
        indexb.value[0] = indices[1] + baseVertex;
        indexb.value[1] = indices[2] + baseVertex;
        pIndexb = pIndexa + 1;
        if (((unsigned __int8)pIndexb & 3) != 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1274, 0, "%s", "!((uint)pIndex & 3)");
        *pIndexb = indexb;
        marksSystem->hasCarryIndex = 0;
    }
    else
    {
        index.value[0] = *indices + baseVertex;
        index.value[1] = indices[1] + baseVertex;
        pIndex = (r_double_index_t *)&outSurf->indices[outSurf->indexCount];
        if (((unsigned __int8)pIndex & 3) != 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1255, 0, "%s", "!((uint)pIndex & 3)");
        *pIndex = index;
        marksSystem->hasCarryIndex = 1;
        marksSystem->carryIndex = indices[2] + baseVertex;
    }
    outSurf->indexCount += 3;
}

void __cdecl FX_GenerateMarkVertsForMark_SetLightHandle(
    FxActiveMarkSurf *drawSurf,
    unsigned __int16 lightHandleOverride)
{
    if ((drawSurf->context.modelTypeAndSurf & 0xC0) != 0xC0 && (drawSurf->context.modelTypeAndSurf & 0xC0) != 0x80)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1357,
            0,
            "%s\n\t(drawSurf->context.modelTypeAndSurf) = %i",
            "((drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL || (drawSurf->context.mo"
            "delTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH)",
            drawSurf->context.modelTypeAndSurf);
    if (!lightHandleOverride)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1358, 0, "%s", "lightHandleOverride != GFX_ML_HANDLE_NONE");
    drawSurf->context.modelIndex = lightHandleOverride;
}

void __cdecl FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(
    FxActiveMarkSurf *drawSurf,
    unsigned __int8 reflectionProbeIndexOverride)
{
    if ((drawSurf->context.modelTypeAndSurf & 0xC0) != 0xC0 && (drawSurf->context.modelTypeAndSurf & 0xC0) != 0x80)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1365,
            0,
            "%s\n\t(drawSurf->context.modelTypeAndSurf) = %i",
            "((drawSurf->context.modelTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_MODEL || (drawSurf->context.mo"
            "delTypeAndSurf & MARK_MODEL_TYPE_MASK) == MARK_MODEL_TYPE_ENT_BRUSH)",
            drawSurf->context.modelTypeAndSurf);
    if (reflectionProbeIndexOverride == 255)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1366,
            0,
            "%s",
            "reflectionProbeIndexOverride != REFLECTION_PROBE_INVALID");
    drawSurf->context.reflectionProbeIndex = reflectionProbeIndexOverride;
}

void __cdecl FX_GenerateMarkVertsForMark_FinishAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    unsigned __int16 baseVertex,
    FxActiveMarkSurf *drawSurf,
    const float (*transform)[3])
{
    unsigned int type; // [esp+19Ch] [ebp-4h]

    type = drawSurf->context.modelTypeAndSurf & 0xC0;
    if (type != 128 && type != 192)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1396,
            0,
            "%s\n\t(type) = %i",
            "(type == MARK_MODEL_TYPE_ENT_BRUSH || type == MARK_MODEL_TYPE_ENT_MODEL)",
            type);
    if (!transform)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1397, 0, "%s", "transform");
    R_AddMarkMeshDrawSurf(drawSurf->material, &drawSurf->context, drawSurf->indices, drawSurf->indexCount);
    if (type == 128)
        FX_ExpandMarkVerts_Transform_GfxWorldVertex_(marksSystem, mark, baseVertex, transform);
    else
        FX_ExpandMarkVerts_Transform_GfxPackedVertex_(marksSystem, mark, baseVertex, transform);
}

void __cdecl FX_GenerateMarkVertsForMark_MatrixFromScaledPlacement(
    const GfxScaledPlacement* placement,
    const float* viewOffset,
    float (*outTransform)[3][3])
{
    if (placement->scale != 1.0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1434,
            0,
            "%s\n\t(placement->scale) = %g",
            "(placement->scale == 1.0f)",
            placement->scale);
    FX_GenerateMarkVertsForMark_MatrixFromPlacement(&placement->base, viewOffset, outTransform);
}

void FX_GenerateMarkVertsForMark_MatrixFromPlacement(
    const GfxPlacement* placement,
    const float* viewOffset,
    float (*outTransform)[3][3])
{
    int v3; // ebp
    float v4; // [esp+58h] [ebp-BCh]
    float v5; // [esp+5Ch] [ebp-B8h]
    float v6; // [esp+60h] [ebp-B4h]
    float skelMat_4; // [esp+68h] [ebp-ACh]
    float skelMat_8; // [esp+6Ch] [ebp-A8h]
    float skelMat_12; // [esp+70h] [ebp-A4h]
    float skelMat_20; // [esp+78h] [ebp-9Ch]
    float skelMat_24; // [esp+7Ch] [ebp-98h]
    float skelMat_28; // [esp+80h] [ebp-94h]
    float skelMat_36; // [esp+88h] [ebp-8Ch]
    float skelMat_40; // [esp+8Ch] [ebp-88h]
    float skelMat_44; // [esp+90h] [ebp-84h]
    float v16; // [esp+A4h] [ebp-70h]
    float v17; // [esp+A8h] [ebp-6Ch]
    float v18; // [esp+ACh] [ebp-68h]
    float v19; // [esp+B0h] [ebp-64h]
    float v20; // [esp+B4h] [ebp-60h]
    float v21; // [esp+B8h] [ebp-5Ch]
    float v22; // [esp+BCh] [ebp-58h]
    float v23; // [esp+C0h] [ebp-54h]
    float v24; // [esp+C4h] [ebp-50h]
    float v25; // [esp+C8h] [ebp-4Ch] BYREF
    float v26; // [esp+CCh] [ebp-48h]
    float v27; // [esp+D0h] [ebp-44h]
    float transWeight; // [esp+D4h] [ebp-40h]
    float v29; // [esp+D8h] [ebp-3Ch]
    float v30; // [esp+DCh] [ebp-38h]
    float v31; // [esp+E0h] [ebp-34h]
    float v32; // [esp+E4h] [ebp-30h]
    DObjAnimMat scale; // [esp+E8h] [ebp-2Ch] BYREF
    int animMat_20; // [esp+108h] [ebp-Ch]
    void* animMat_24; // [esp+10Ch] [ebp-8h]
    void* retaddr; // [esp+114h] [ebp+0h]

    animMat_20 = v3;
    animMat_24 = retaddr;
    scale.quat[0] = placement->quat[0];
    scale.quat[1] = placement->quat[1];
    scale.quat[2] = placement->quat[2];
    scale.quat[3] = placement->quat[3];
    scale.transWeight = 2.0;
    scale.trans[0] = placement->origin[0];
    scale.trans[1] = placement->origin[1];
    scale.trans[2] = placement->origin[2];
    v32 = scale.quat[0];
    if ((LODWORD(scale.quat[0]) & 0x7F800000) == 0x7F800000
        || (v31 = scale.quat[1], (LODWORD(scale.quat[1]) & 0x7F800000) == 0x7F800000)
        || (v30 = scale.quat[2], (LODWORD(scale.quat[2]) & 0x7F800000) == 0x7F800000)
        || (v29 = scale.quat[3], (LODWORD(scale.quat[3]) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            473,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    transWeight = scale.transWeight;
    if ((LODWORD(scale.transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            474,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(scale.quat, scale.transWeight, &v25);
    v24 = v25 * scale.quat[0];
    v23 = v25 * scale.quat[1];
    v22 = v25 * scale.quat[2];
    v21 = v25 * scale.quat[3];
    v20 = v26 * scale.quat[1];
    v19 = v26 * scale.quat[2];
    v18 = v26 * scale.quat[3];
    v17 = v27 * scale.quat[2];
    v16 = v27 * scale.quat[3];
    v4 = 1.0 - (v20 + v17);
    v5 = v23 + v16;
    v6 = v22 - v18;
    skelMat_4 = v23 - v16;
    skelMat_8 = 1.0 - (v24 + v17);
    skelMat_12 = v19 + v21;
    skelMat_20 = v22 + v18;
    skelMat_24 = v19 - v21;
    skelMat_28 = 1.0 - (v24 + v20);
    skelMat_36 = scale.trans[0];
    skelMat_40 = scale.trans[1];
    skelMat_44 = scale.trans[2];
    (*outTransform)[0][0] = v4;
    (*outTransform)[0][1] = v5;
    (*outTransform)[0][2] = v6;
    (*outTransform)[1][0] = skelMat_4;
    (*outTransform)[1][1] = skelMat_8;
    (*outTransform)[1][2] = skelMat_12;
    (*outTransform)[2][0] = skelMat_20;
    (*outTransform)[2][1] = skelMat_24;
    (*outTransform)[2][2] = skelMat_28;
    (*outTransform)[3][0] = skelMat_36;
    (*outTransform)[3][1] = skelMat_40;
    (*outTransform)[3][2] = skelMat_44;
    Vec3Add((*outTransform)[3], viewOffset, (*outTransform)[3]);
}

void __cdecl FX_GenerateMarkVertsForEntDObj(
    int localClientNum,
    int entId,
    unsigned int *indexCount,
    unsigned __int16 lightHandle,
    unsigned __int8 reflectionProbeIndex,
    const DObj_s *dobj,
    const cpose_t *pose)
{
    FxSystem *camera; // [esp+94h] [ebp-20h]
    unsigned int hidePartBits[4]; // [esp+98h] [ebp-1Ch] BYREF
    FxSystem *system; // [esp+A8h] [ebp-Ch]
    const DObjAnimMat *boneMtxList; // [esp+ACh] [ebp-8h] BYREF
    unsigned __int16 entMarkListHead; // [esp+B0h] [ebp-4h]

    if (!dobj)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1679, 0, "%s", "dobj");
    if (!pose)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1680, 0, "%s", "pose");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    entMarkListHead = fx_marksSystemPool[0].entFirstMarkHandles[entId];
    if (entMarkListHead != 0xFFFF)
    {
        //Profile_Begin(210);
        system = FX_GetSystem(localClientNum);
        camera = system;
        R_MarkUtil_GetDObjAnimMatAndHideParts(dobj, pose, &boneMtxList, hidePartBits);
        FX_GenerateMarkVertsForList_EntDObj(
            fx_marksSystemPool,
            entMarkListHead,
            &camera->camera,
            indexCount,
            lightHandle,
            reflectionProbeIndex,
            dobj,
            boneMtxList);
        //Profile_EndInternal(0);
    }
}

char __cdecl FX_GenerateMarkVertsForList_EntDObj(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount,
    unsigned __int16 lightHandleOverride,
    unsigned __int8 reflectionProbeIndexOverride,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList)
{
    FxMark *mark; // [esp+214h] [ebp-50h]
    FxActiveMarkSurf drawSurf; // [esp+218h] [ebp-4Ch] BYREF
    unsigned __int16 markHandle; // [esp+22Ch] [ebp-38h]
    unsigned __int16 baseVertex; // [esp+230h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+234h] [ebp-30h] BYREF
    int savedregs; // [esp+264h] [ebp+0h] BYREF

    for (markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if ((mark->context.modelTypeAndSurf & 0xC0) == 0xC0)
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

void __cdecl FX_GenerateMarkVertsForMark_MatrixFromAnim(
    FxMark *mark,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList,
    const vec3r viewOffset,
    mat4x3 &outTransform)
{
    int v5; // ebp
    float v6[4][3]; // [esp+3Ch] [ebp-1E8h] BYREF
    float v7; // [esp+6Ch] [ebp-1B8h]
    float v8; // [esp+70h] [ebp-1B4h]
    float v9; // [esp+74h] [ebp-1B0h]
    float v10; // [esp+78h] [ebp-1ACh]
    float v11; // [esp+7Ch] [ebp-1A8h]
    float v12; // [esp+80h] [ebp-1A4h]
    float v13; // [esp+84h] [ebp-1A0h]
    float v14; // [esp+88h] [ebp-19Ch]
    float v15; // [esp+8Ch] [ebp-198h]
    float v16; // [esp+90h] [ebp-194h]
    float v17; // [esp+94h] [ebp-190h]
    float v18; // [esp+98h] [ebp-18Ch]
    float v19; // [esp+9Ch] [ebp-188h]
    float v20; // [esp+A0h] [ebp-184h]
    float v21; // [esp+A4h] [ebp-180h]
    float v22; // [esp+A8h] [ebp-17Ch]
    float v23; // [esp+ACh] [ebp-178h]
    float v24; // [esp+B0h] [ebp-174h]
    float v25; // [esp+B4h] [ebp-170h]
    float v26; // [esp+B8h] [ebp-16Ch]
    float v27; // [esp+BCh] [ebp-168h]
    float v28; // [esp+C0h] [ebp-164h]
    float v29; // [esp+C4h] [ebp-160h]
    float v30; // [esp+C8h] [ebp-15Ch]
    float v31; // [esp+CCh] [ebp-158h]
    float v32; // [esp+D0h] [ebp-154h]
    float v33; // [esp+D4h] [ebp-150h]
    float v34; // [esp+D8h] [ebp-14Ch]
    float v35; // [esp+DCh] [ebp-148h] BYREF
    float v36; // [esp+E0h] [ebp-144h]
    float v37; // [esp+E4h] [ebp-140h]
    float v38; // [esp+E8h] [ebp-13Ch]
    float v39; // [esp+ECh] [ebp-138h]
    float v40; // [esp+F0h] [ebp-134h]
    float v41; // [esp+F4h] [ebp-130h]
    float v42; // [esp+F8h] [ebp-12Ch]
    const DObjAnimMat *v43; // [esp+FCh] [ebp-128h]
    const DObjAnimMat *BasePose; // [esp+100h] [ebp-124h]
    float v45; // [esp+104h] [ebp-120h]
    float v46; // [esp+108h] [ebp-11Ch]
    float v47; // [esp+10Ch] [ebp-118h]
    float *v48; // [esp+110h] [ebp-114h]
    float v49; // [esp+114h] [ebp-110h]
    float v50; // [esp+118h] [ebp-10Ch]
    float v51; // [esp+11Ch] [ebp-108h]
    float *v52; // [esp+120h] [ebp-104h]
    float v53; // [esp+124h] [ebp-100h]
    float v54; // [esp+128h] [ebp-FCh]
    float v55; // [esp+12Ch] [ebp-F8h]
    float *v56; // [esp+130h] [ebp-F4h]
    float v57[4][3]; // [esp+134h] [ebp-F0h] BYREF
    float v58; // [esp+164h] [ebp-C0h]
    float v59; // [esp+168h] [ebp-BCh]
    float v60; // [esp+16Ch] [ebp-B8h]
    const float *trans; // [esp+170h] [ebp-B4h]
    float *v62; // [esp+174h] [ebp-B0h]
    float v63; // [esp+178h] [ebp-ACh]
    float v64; // [esp+17Ch] [ebp-A8h]
    float v65; // [esp+180h] [ebp-A4h]
    float v66; // [esp+184h] [ebp-A0h]
    float v67; // [esp+188h] [ebp-9Ch]
    float v68; // [esp+18Ch] [ebp-98h]
    float v69; // [esp+190h] [ebp-94h]
    float v70; // [esp+194h] [ebp-90h]
    float v71; // [esp+198h] [ebp-8Ch]
    float v72; // [esp+19Ch] [ebp-88h]
    float v73; // [esp+1A0h] [ebp-84h]
    float v74; // [esp+1A4h] [ebp-80h]
    float v75; // [esp+1A8h] [ebp-7Ch] BYREF
    float v76; // [esp+1ACh] [ebp-78h]
    float v77; // [esp+1B0h] [ebp-74h]
    float v78; // [esp+1B4h] [ebp-70h]
    float v79; // [esp+1BCh] [ebp-68h]
    float v80; // [esp+1C0h] [ebp-64h]
    float v81; // [esp+1C4h] [ebp-60h]
    float v82; // [esp+1C8h] [ebp-5Ch]
    float v83; // [esp+1CCh] [ebp-58h]
    float v84; // [esp+1D0h] [ebp-54h]
    float v85; // [esp+1D4h] [ebp-50h]
    float v86; // [esp+1D8h] [ebp-4Ch]
    float v87; // [esp+1DCh] [ebp-48h]
    float v88; // [esp+1E0h] [ebp-44h] BYREF
    float v89; // [esp+1E4h] [ebp-40h]
    float v90; // [esp+1E8h] [ebp-3Ch]
    float transWeight; // [esp+1ECh] [ebp-38h]
    float v92; // [esp+1F0h] [ebp-34h]
    float v93; // [esp+1F4h] [ebp-30h]
    float v94; // [esp+1F8h] [ebp-2Ch]
    float v95; // [esp+1FCh] [ebp-28h]
    const DObjAnimMat *v96; // [esp+200h] [ebp-24h]
    const XModel *Model; // [esp+204h] [ebp-20h]
    int i; // [esp+208h] [ebp-1Ch]
    int v99; // [esp+20Ch] [ebp-18h]
    int v100; // [esp+210h] [ebp-14h]
    int lmapIndex; // [esp+214h] [ebp-10h]
    void *v103; // [esp+21Ch] [ebp-8h]

    if (!dobj)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1452, 0, "%s", "dobj");
    if (!boneMtxList)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1453, 0, "%s", "boneMtxList");
    lmapIndex = mark->context.lmapIndex;
    v100 = mark->context.modelTypeAndSurf & 0x3F;
    if (v100 >= DObjGetNumModels(dobj))
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1458,
            0,
            "%s\n\t(dObjModelIndexForMark) = %i",
            "(dObjModelIndexForMark < DObjGetNumModels( dobj ))",
            v100);
    v99 = 0;
    for (i = 0; i != v100; ++i)
    {
        Model = DObjGetModel(dobj, i);
        v99 += XModelNumBones(Model);
    }
    v96 = &boneMtxList[lmapIndex + v99];
    v95 = v96->quat[0];
    if ((LODWORD(v95) & 0x7F800000) == 0x7F800000
        || (v94 = v96->quat[1], (LODWORD(v94) & 0x7F800000) == 0x7F800000)
        || (v93 = v96->quat[2], (LODWORD(v93) & 0x7F800000) == 0x7F800000)
        || (v92 = v96->quat[3], (LODWORD(v92) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            473,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    transWeight = v96->transWeight;
    if ((LODWORD(transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            474,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(v96->quat, v96->transWeight, &v88);
    v87 = v88 * v96->quat[0];
    v86 = v88 * v96->quat[1];
    v85 = v88 * v96->quat[2];
    v84 = v88 * v96->quat[3];
    v83 = v89 * v96->quat[1];
    v82 = v89 * v96->quat[2];
    v81 = v89 * v96->quat[3];
    v80 = v90 * v96->quat[2];
    v79 = v90 * v96->quat[3];
    v63 = 1.0 - (v83 + v80);
    v64 = v86 + v79;
    v65 = v85 - v81;
    v66 = 0.0;
    v67 = v86 - v79;
    v68 = 1.0 - (v87 + v80);
    v69 = v82 + v84;
    v70 = 0.0;
    v71 = v85 + v81;
    v72 = v82 - v84;
    v73 = 1.0 - (v87 + v83);
    v74 = 0.0;
    v62 = &v75;
    trans = v96->trans;
    v75 = v96->trans[0];
    v76 = v96->trans[1];
    v77 = v96->trans[2];
    v78 = 1.0;
    v60 = v63;
    v59 = v64;
    v58 = v65;
    v57[0][0] = v63;
    v57[0][1] = v64;
    v57[0][2] = v65;
    v56 = v57[1];
    v55 = v67;
    v54 = v68;
    v53 = v69;
    v57[1][0] = v67;
    v57[1][1] = v68;
    v57[1][2] = v69;
    v52 = v57[2];
    v51 = v71;
    v50 = v72;
    v49 = v73;
    v57[2][0] = v71;
    v57[2][1] = v72;
    v57[2][2] = v73;
    v48 = v57[3];
    v47 = v75;
    v46 = v76;
    v45 = v77;
    v57[3][0] = v75;
    v57[3][1] = v76;
    v57[3][2] = v77;
    Vec3Add(v57[3], (const float *)viewOffset, v57[3]);
    Model = DObjGetModel(dobj, v100);
    BasePose = XModelGetBasePose(Model);
    v43 = &BasePose[lmapIndex];
    v42 = v43->quat[0];
    if ((LODWORD(v42) & 0x7F800000) == 0x7F800000
        || (v41 = v43->quat[1], (LODWORD(v41) & 0x7F800000) == 0x7F800000)
        || (v40 = v43->quat[2], (LODWORD(v40) & 0x7F800000) == 0x7F800000)
        || (v39 = v43->quat[3], (LODWORD(v39) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            581,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    v38 = v43->transWeight;
    if ((LODWORD(v38) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
            582,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(v43->quat, v43->transWeight, &v35);
    v34 = v35 * v43->quat[0];
    v33 = v35 * v43->quat[1];
    v32 = v35 * v43->quat[2];
    v31 = v35 * v43->quat[3];
    v30 = v36 * v43->quat[1];
    v29 = v36 * v43->quat[2];
    v28 = v36 * v43->quat[3];
    v27 = v37 * v43->quat[2];
    v26 = v37 * v43->quat[3];
    v10 = 1.0 - (v30 + v27);
    v11 = v33 - v26;
    v12 = v32 + v28;
    v13 = 0.0;
    v14 = v33 + v26;
    v15 = 1.0 - (v34 + v27);
    v16 = v29 - v31;
    v17 = 0.0;
    v18 = v32 - v28;
    v19 = v29 + v31;
    v20 = 1.0 - (v34 + v30);
    v21 = 0.0;
    v22 = -(v43->trans[0] * v10 + v43->trans[1] * v14 + v43->trans[2] * v18);
    v23 = -(v43->trans[0] * v11 + v43->trans[1] * v15 + v43->trans[2] * v19);
    v24 = -(v43->trans[0] * v12 + v43->trans[1] * v16 + v43->trans[2] * v20);
    v25 = 1.0;
    v9 = v10;
    v8 = v11;
    v7 = v12;
    v6[0][0] = v10;
    v6[0][1] = v11;
    v6[0][2] = v12;
    v6[1][0] = v14;
    v6[1][1] = v15;
    v6[1][2] = v16;
    v6[2][0] = v18;
    v6[2][1] = v19;
    v6[2][2] = v20;
    v6[3][0] = v22;
    v6[3][1] = v23;
    v6[3][2] = v24;
    MatrixMultiply43(v6, v57, outTransform);
}

void __cdecl FX_GenerateMarkVertsForEntBrush(
    int localClientNum,
    int entId,
    unsigned int *indexCount,
    unsigned __int8 reflectionProbeIndex,
    const GfxPlacement *placement)
{
    FxSystem *camera; // [esp+164h] [ebp-Ch]
    unsigned __int16 entMarkListHead; // [esp+16Ch] [ebp-4h]

    if (!placement)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1707, 0, "%s", "placement");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    entMarkListHead = fx_marksSystemPool[0].entFirstMarkHandles[entId];
    if (entMarkListHead != 0xFFFF)
    {
        //Profile_Begin(210);
        camera = FX_GetSystem(localClientNum);
        FX_GenerateMarkVertsForList_EntBrush(
            fx_marksSystemPool,
            entMarkListHead,
            &camera->camera,
            indexCount,
            placement,
            reflectionProbeIndex);
        //Profile_EndInternal(0);
    }
}

char __cdecl FX_GenerateMarkVertsForList_EntBrush(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount,
    const GfxPlacement *placement,
    unsigned __int8 reflectionProbeIndex)
{
    FxMark *mark; // [esp+F4h] [ebp-50h]
    FxActiveMarkSurf drawSurf; // [esp+F8h] [ebp-4Ch] BYREF
    unsigned __int16 markHandle; // [esp+10Ch] [ebp-38h]
    unsigned __int16 baseVertex; // [esp+110h] [ebp-34h] BYREF
    float transformMatrix[4][3]; // [esp+114h] [ebp-30h] BYREF
    int savedregs; // [esp+144h] [ebp+0h] BYREF

    FX_GenerateMarkVertsForMark_MatrixFromPlacement(placement, vec3_origin, (float(*)[3][3])transformMatrix);
    for (markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if ((mark->context.modelTypeAndSurf & 0xC0) == 0x80)
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return 0;
            FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(&drawSurf, reflectionProbeIndex);
            FX_GenerateMarkVertsForMark_FinishAnimated(marksSystem, mark, baseVertex, &drawSurf, transformMatrix);
        }
    }
    return 1;
}

void __cdecl FX_EndGeneratingMarkVertsForEntModels(int localClientNum)
{
    //Profile_Begin(210);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    FX_FinishGeneratingMarkVerts(fx_marksSystemPool);
    if (InterlockedDecrement(&g_markThread[localClientNum]))
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1734,
            0,
            "%s",
            "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0");
    R_EndMarkMeshVerts();
    //Profile_EndInternal(0);
}

void __cdecl FX_FinishGeneratingMarkVerts(FxMarksSystem *marksSystem)
{
    r_double_index_t *doubleIndices; // [esp+8h] [ebp-4h] BYREF

    if (marksSystem->hasCarryIndex)
    {
        if (!R_ReserveMarkMeshIndices(0, &doubleIndices))
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1616, 0, "%s", "allocSuccessed");
        (*--doubleIndices).value[0] = marksSystem->carryIndex;
    }
}

void __cdecl FX_GenerateMarkVertsForStaticModels(
    int localClientNum,
    int smodelCount,
    const unsigned __int8 *smodelVisLods)
{
    unsigned __int16 v3; // ax
    FxMark *mark; // [esp+64h] [ebp-14h]
    FxSystem *camera; // [esp+68h] [ebp-10h]
    unsigned int indexCount; // [esp+6Ch] [ebp-Ch] BYREF
    FxSystem *system; // [esp+70h] [ebp-8h]
    FxMark *markEnd; // [esp+74h] [ebp-4h]

    if (!fx_marks->current.enabled || !fx_marks_smodels->current.enabled)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1751,
            0,
            "%s",
            "fx_marks->current.enabled && fx_marks_smodels->current.enabled");
    //Profile_Begin(209);
    R_BeginMarkMeshVerts();
    if (InterlockedIncrement(&g_markThread[localClientNum]) != 1)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1756,
            0,
            "%s",
            "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
            139,
            0,
            "%s\n\t(clientIndex) = %i",
            "(clientIndex == 0)",
            localClientNum);
    system = FX_GetSystem(localClientNum);
    camera = system;
    fx_marksSystemPool[0].hasCarryIndex = 0;
    indexCount = 0;
    markEnd = (FxMark *)fx_marksSystemPool[0].triGroups;
    for (mark = fx_marksSystemPool[0].marks; mark != markEnd; ++mark)
    {
        if (mark->frameCountDrawn != -1 && mark->prevMark == 0xFFFF && (mark->context.modelTypeAndSurf & 0xC0) == 0x40)
        {
            if (smodelVisLods[mark->context.modelIndex])
            {
                v3 = FX_MarkToHandle(fx_marksSystemPool, mark);
                FX_GenerateMarkVertsForList_WorldXModel(fx_marksSystemPool, v3, &camera->camera, &indexCount);
            }
        }
    }
    FX_FinishGeneratingMarkVerts(fx_marksSystemPool);
    if (InterlockedDecrement(&g_markThread[localClientNum]))
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1788,
            0,
            "%s",
            "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0");
    R_EndMarkMeshVerts();
    //Profile_EndInternal(0);
}

void __cdecl FX_ExpandMarkVerts_NoTransform_GfxPackedVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex)
{
    double v3; // st7
    double v4; // st7
    int v5; // [esp+50h] [ebp-D4h]
    PackedUnitVec v6; // [esp+58h] [ebp-CCh]
    PackedUnitVec v7; // [esp+7Ch] [ebp-A8h]
    GfxPackedVertex *castOutVert; // [esp+E0h] [ebp-44h]
    unsigned int groupHandle; // [esp+E4h] [ebp-40h]
    float delta[3]; // [esp+E8h] [ebp-3Ch] BYREF
    float texCoordScale; // [esp+F4h] [ebp-30h]
    float binormal[3]; // [esp+F8h] [ebp-2Ch] BYREF
    const FxMarkPoint *markPoint; // [esp+104h] [ebp-20h]
    GfxWorldVertex *verts; // [esp+108h] [ebp-1Ch]
    float texCoord[2]; // [esp+10Ch] [ebp-18h] BYREF
    int pointCount; // [esp+114h] [ebp-10h]
    int loopCount; // [esp+118h] [ebp-Ch]
    const FxPointGroup *group; // [esp+11Ch] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+120h] [ebp-4h]

    if (!mark)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1082, 0, "%s", "mark");
    verts = R_GetMarkMeshVerts(baseVertex);
    pointCount = mark->pointCount;
    if (mark->radius < 0.1000000014901161)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1097,
            0,
            "%s\n\t(mark->radius) = %g",
            "(mark->radius >= 0.1f)",
            mark->radius);
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    outVert = verts;
    do
    {
        if (groupHandle == 0xFFFF)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1103, 0, "%s", "groupHandle != FX_HANDLE_NONE");
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            v5 = 2;
        else
            v5 = pointCount;
        loopCount = v5;
        if (v5 <= 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1108, 0, "%s\n\t(loopCount) = %i", "(loopCount > 0)", loopCount);
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
            v3 = Vec3Dot(delta, mark->texCoordAxis);
            texCoord[0] = v3 * texCoordScale + 0.5;
            v4 = Vec3Dot(delta, binormal);
            texCoord[1] = v4 * texCoordScale + 0.5;
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
    if (groupHandle != 0xFFFF)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1135,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle);
}

char __cdecl FX_GenerateMarkVertsForList_WorldXModel(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount)
{
    FxMark *mark; // [esp+B0h] [ebp-20h]
    FxActiveMarkSurf drawSurf; // [esp+B4h] [ebp-1Ch] BYREF
    unsigned __int16 markHandle; // [esp+C8h] [ebp-8h]
    unsigned __int16 baseVertex; // [esp+CCh] [ebp-4h] BYREF

    for (markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
            return 0;
        FX_GenerateMarkVertsForMark_FinishNonAnimated(marksSystem, mark, baseVertex, &drawSurf);
    }
    return 1;
}

void __cdecl FX_ExpandMarkVerts_NoTransform_GfxWorldVertex_(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex)
{
    double v3; // st7
    double v4; // st7
    int v5; // [esp+50h] [ebp-A4h]
    PackedUnitVec v6; // [esp+58h] [ebp-9Ch]
    PackedUnitVec v7; // [esp+7Ch] [ebp-78h]
    float *lmapCoord; // [esp+9Ch] [ebp-58h]
    GfxWorldVertex *castOutVert; // [esp+B0h] [ebp-44h]
    unsigned int groupHandle; // [esp+B4h] [ebp-40h]
    float delta[3]; // [esp+B8h] [ebp-3Ch] BYREF
    float texCoordScale; // [esp+C4h] [ebp-30h]
    float binormal[3]; // [esp+C8h] [ebp-2Ch] BYREF
    const FxMarkPoint *markPoint; // [esp+D4h] [ebp-20h]
    GfxWorldVertex *verts; // [esp+D8h] [ebp-1Ch]
    __int64 texCoord; // [esp+DCh] [ebp-18h]
    int pointCount; // [esp+E4h] [ebp-10h]
    int loopCount; // [esp+E8h] [ebp-Ch]
    const FxPointGroup *group; // [esp+ECh] [ebp-8h]
    GfxWorldVertex *outVert; // [esp+F0h] [ebp-4h]

    if (!mark)
        MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1082, 0, "%s", "mark");
    verts = R_GetMarkMeshVerts(baseVertex);
    pointCount = mark->pointCount;
    if (mark->radius < 0.1000000014901161)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1097,
            0,
            "%s\n\t(mark->radius) = %g",
            "(mark->radius >= 0.1f)",
            mark->radius);
    texCoordScale = 0.5 / mark->radius;
    groupHandle = mark->points;
    outVert = verts;
    do
    {
        if (groupHandle == 0xFFFF)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1103, 0, "%s", "groupHandle != FX_HANDLE_NONE");
        group = (const FxPointGroup *)FX_PointGroupFromHandle(marksSystem, groupHandle);
        groupHandle = group->next;
        if (pointCount > 2)
            v5 = 2;
        else
            v5 = pointCount;
        loopCount = v5;
        if (v5 <= 0)
            MyAssertHandler(".\\EffectsCore\\fx_marks.cpp", 1108, 0, "%s\n\t(loopCount) = %i", "(loopCount > 0)", loopCount);
        pointCount -= loopCount;
        markPoint = (const FxMarkPoint *)group;
        do
        {
            castOutVert = outVert;
            Vec3Sub(markPoint->xyz, mark->origin, delta);
            Vec3Cross(mark->texCoordAxis, markPoint->normal, binormal);
            castOutVert->xyz[0] = markPoint->xyz[0];
            castOutVert->xyz[1] = markPoint->xyz[1];
            castOutVert->xyz[2] = markPoint->xyz[2];
            castOutVert->binormalSign = -1.0;
            castOutVert->color.packed = *(_DWORD *)mark->nativeColor;
            v3 = Vec3Dot(delta, mark->texCoordAxis);
            *(float *)&texCoord = v3 * texCoordScale + 0.5;
            v4 = Vec3Dot(delta, binormal);
            *((float *)&texCoord + 1) = v4 * texCoordScale + 0.5;
            lmapCoord = markPoint->lmapCoord;
            *(_QWORD *)castOutVert->texCoord = texCoord;
            *(double *)castOutVert->lmapCoord = *(double *)lmapCoord;
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
    if (groupHandle != 0xFFFF)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1135,
            0,
            "%s\n\t(groupHandle) = %i",
            "(groupHandle == 0xffff)",
            groupHandle);
}

void __cdecl FX_GenerateMarkVertsForMark_FinishNonAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    unsigned __int16 baseVertex,
    FxActiveMarkSurf *drawSurf)
{
    unsigned int type; // [esp+168h] [ebp-4h]

    type = drawSurf->context.modelTypeAndSurf & 0xC0;
    if ((drawSurf->context.modelTypeAndSurf & 0xC0) != 0 && type != 64)
        MyAssertHandler(
            ".\\EffectsCore\\fx_marks.cpp",
            1376,
            0,
            "%s\n\t(type) = %i",
            "(type == MARK_MODEL_TYPE_WORLD_BRUSH || type == MARK_MODEL_TYPE_WORLD_MODEL)",
            type);
    R_AddMarkMeshDrawSurf(drawSurf->material, &drawSurf->context, drawSurf->indices, drawSurf->indexCount);
    if (type)
        FX_ExpandMarkVerts_NoTransform_GfxPackedVertex_(marksSystem, mark, baseVertex);
    else
        FX_ExpandMarkVerts_NoTransform_GfxWorldVertex_(marksSystem, mark, baseVertex);
}

void __cdecl FX_GenerateMarkVertsForWorld(int localClientNum)
{
    FxSystem *System; // eax
    unsigned int indexCount[2]; // [esp+64h] [ebp-8h] BYREF

    if (fx_marks->current.enabled)
    {
        //Profile_Begin(211);
        R_BeginMarkMeshVerts();
        if (InterlockedIncrement(&g_markThread[localClientNum]) != 1)
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                1810,
                0,
                "%s",
                "Sys_InterlockedIncrement( &g_markThread[localClientNum] ) == 1");
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\effectscore\\fx_marks.h",
                139,
                0,
                "%s\n\t(clientIndex) = %i",
                "(clientIndex == 0)",
                localClientNum);
        System = FX_GetSystem(localClientNum);
        indexCount[1] = (unsigned int)System;
        fx_marksSystemPool[0].hasCarryIndex = 0;
        indexCount[0] = 0;
        FX_GenerateMarkVertsForList_WorldBrush(
            fx_marksSystemPool,
            fx_marksSystemPool[0].firstActiveWorldMarkHandle,
            &System->camera,
            indexCount);
        FX_FinishGeneratingMarkVerts(fx_marksSystemPool);
        if (InterlockedDecrement(&g_markThread[localClientNum]))
            MyAssertHandler(
                ".\\EffectsCore\\fx_marks.cpp",
                1820,
                0,
                "%s",
                "Sys_InterlockedDecrement( &g_markThread[localClientNum] ) == 0");
        R_EndMarkMeshVerts();
        //Profile_EndInternal(0);
    }
}

char __cdecl FX_GenerateMarkVertsForList_WorldBrush(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount)
{
    FxMark *mark; // [esp+B4h] [ebp-20h]
    FxActiveMarkSurf drawSurf; // [esp+B8h] [ebp-1Ch] BYREF
    unsigned __int16 markHandle; // [esp+CCh] [ebp-8h]
    unsigned __int16 baseVertex; // [esp+D0h] [ebp-4h] BYREF

    for (markHandle = head; markHandle != 0xFFFF; markHandle = mark->nextMark)
    {
        mark = FX_MarkFromHandle(marksSystem, markHandle);
        if (!FX_CullSphere(camera, camera->frustumPlaneCount, mark->origin, mark->radius))
        {
            if (!FX_GenerateMarkVertsForMark_Begin(marksSystem, mark, indexCount, &baseVertex, &drawSurf))
                return 0;
            FX_GenerateMarkVertsForMark_FinishNonAnimated(marksSystem, mark, baseVertex, &drawSurf);
        }
    }
    return 1;
}

