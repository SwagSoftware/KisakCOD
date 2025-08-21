#include "cg_local.h"
#include "cg_public.h"

#include <qcommon/threads.h>
#include <xanim/dobj.h>
#include <DynEntity/DynEntity_client.h>
#include <xanim/dobj_utils.h>
#include <universal/profile.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include "cg_main.h"
#include "cg_pose.h"
#endif

bool __cdecl CG_IsEntityLinked(int32_t localClientNum, uint32_t entIndex)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            183,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (entIndex >= 0x400)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            184,
            0,
            "entIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entIndex,
            1024);
    return CG_GetEntityCollNode(localClientNum, entIndex)->sector != 0;
}

bool __cdecl CG_EntityNeedsLinked(int32_t localClientNum, uint32_t entIndex)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            200,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (entIndex >= 0x400)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            201,
            0,
            "entIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entIndex,
            1024);
    return CG_GetEntity(localClientNum, entIndex)->nextState.solid
        || CG_LocationalTraceDObj(localClientNum, entIndex) != 0;
}

DObj_s *__cdecl CG_LocationalTraceDObj(int32_t localClientNum, uint32_t entIndex)
{
    centity_s *cent; // [esp+0h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            132,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (entIndex >= 0x400)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            133,
            0,
            "entIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entIndex,
            1024);
    cent = CG_GetEntity(localClientNum, entIndex);
    if (cent->nextState.solid)
        return 0;
    if (cent->nextState.eType == ET_SCRIPTMOVER || cent->nextState.eType == ET_VEHICLE || cent->nextState.eType == ET_HELICOPTER)
        return Com_GetClientDObj(entIndex, localClientNum);
    return 0;
}

void __cdecl CG_UnlinkEntity(int32_t localClientNum, uint32_t entIndex)
{
    PROF_SCOPED("CG_UnlinkEntity");
    CG_UnlinkEntityColl(localClientNum, entIndex);
}

void __cdecl CG_LinkEntity(int32_t localClientNum, uint32_t entIndex)
{
    float mins[3]; // [esp+4Ch] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+58h] [ebp-30h] BYREF
    DObj_s *dobj; // [esp+64h] [ebp-24h]
    float absMaxs[3]; // [esp+68h] [ebp-20h] BYREF
    centity_s *cent; // [esp+74h] [ebp-14h]
    entityState_s *p_nextState; // [esp+78h] [ebp-10h]
    float absMins[3]; // [esp+7Ch] [ebp-Ch] BYREF

    if (localClientNum)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            243,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (entIndex >= 0x400)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            244,
            0,
            "entIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entIndex,
            1024);

    PROF_SCOPED("CG_LinkEntity");
    cent = CG_GetEntity(localClientNum, entIndex);
    p_nextState = &cent->nextState;
    dobj = Com_GetClientDObj(entIndex, localClientNum);
    if (dobj)
    {
        CG_GetEntityDobjBounds(cent, dobj, absMins, absMaxs);
        CG_LinkEntityColl(localClientNum, entIndex, absMins, absMaxs);
    }
    else if (p_nextState->solid)
    {
        CG_GetEntityBModelBounds(cent, mins, maxs, absMins, absMaxs);
        CG_LinkEntityColl(localClientNum, entIndex, absMins, absMaxs);
    }
    else
    {
        CG_UnlinkEntity(localClientNum, entIndex);
    }
}

void __cdecl CG_GetEntityBModelBounds(const centity_s *cent, float *mins, float *maxs, float *absMins, float *absMaxs)
{
    int32_t zd; // [esp+0h] [ebp-14h]
    int32_t zu; // [esp+4h] [ebp-10h]
    float radius; // [esp+8h] [ebp-Ch]
    int32_t x; // [esp+10h] [ebp-4h]

    if (!cent)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 63, 0, "%s", "cent");
    if (!mins)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 64, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 65, 0, "%s", "maxs");
    if (!absMins)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 66, 0, "%s", "absMins");
    if (!absMaxs)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 67, 0, "%s", "absMaxs");
    if (!cent->nextState.solid)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 70, 0, "%s", "es->solid");
    if (cent->nextState.solid == 0xFFFFFF)
    {
        CM_ModelBounds(cent->nextState.index.brushmodel, mins, maxs);
    }
    else
    {
        x = (uint8_t)cent->nextState.solid;
        zd = (uint8_t)BYTE1(cent->nextState.solid) - 1;
        zu = (uint8_t)BYTE2(cent->nextState.solid) - 32;
        mins[1] = 1.0 - (double)x;
        *mins = mins[1];
        maxs[1] = (double)x - 1.0;
        *maxs = maxs[1];
        mins[2] = 1.0 - (double)zd;
        maxs[2] = (double)zu - 1.0;
    }
    radius = RadiusFromBounds(mins, maxs);
    *absMins = -radius;
    absMins[1] = -radius;
    absMins[2] = -radius;
    *absMaxs = radius;
    absMaxs[1] = radius;
    absMaxs[2] = radius;
    Vec3Add(cent->pose.origin, absMins, absMins);
    Vec3Add(cent->pose.origin, absMaxs, absMaxs);
}

void __cdecl CG_GetEntityDobjBounds(const centity_s *cent, const DObj_s *dobj, float *absMins, float *absMaxs)
{
    float v4; // [esp+0h] [ebp-1Ch]
    float v5; // [esp+4h] [ebp-18h]
    float v6; // [esp+8h] [ebp-14h]
    float v7; // [esp+Ch] [ebp-10h]
    float v8; // [esp+10h] [ebp-Ch]
    float v9; // [esp+14h] [ebp-8h]
    float radius; // [esp+18h] [ebp-4h]

    if (!cent)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 113, 0, "%s", "cent");
    if (!dobj)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 114, 0, "%s", "dobj");
    if (!absMins)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 115, 0, "%s", "absMins");
    if (!absMaxs)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 116, 0, "%s", "absMaxs");
    radius = DObjGetRadius(dobj);
    v7 = cent->pose.origin[0] - radius;
    v8 = cent->pose.origin[1] - radius;
    v9 = cent->pose.origin[2] - radius;
    *absMins = v7;
    absMins[1] = v8;
    absMins[2] = v9;
    v4 = radius + cent->pose.origin[0];
    v5 = radius + cent->pose.origin[1];
    v6 = radius + cent->pose.origin[2];
    *absMaxs = v4;
    absMaxs[1] = v5;
    absMaxs[2] = v6;
}

void __cdecl CG_LocationalTrace(trace_t *results, float *start, float *end, int32_t passEntityNum, int32_t contentMask)
{
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 843, 0, "%s", "results");
    if (!start)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 844, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 845, 0, "%s", "end");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_world.cpp", 846, 0, "%s", "Sys_IsMainThread()");
    KISAK_NULLSUB();
    PROF_SCOPED("CG_LocationalTrace");
    CG_Trace(results, start, (float *)vec3_origin, (float *)vec3_origin, end, passEntityNum, contentMask, 1, 1);
}

void __cdecl CG_Trace(
    trace_t *results,
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int32_t passEntityNum,
    int32_t contentMask,
    bool locational,
    bool staticModels)
{
    moveclip_t result; // [esp+70h] [ebp-A8h] BYREF
    pointtrace_t clip; // [esp+CCh] [ebp-4Ch] BYREF
    IgnoreEntParams ignoreEntParams; // [esp+100h] [ebp-18h] BYREF
    float delta[3]; // [esp+10Ch] [ebp-Ch] BYREF

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_world.cpp", 731, 0, "%s", "Sys_IsMainThread()");
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 733, 0, "%s", "results");
    if (!start)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 734, 0, "%s", "start");
    if (!mins)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 735, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 736, 0, "%s", "maxs");
    if (!end)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 737, 0, "%s", "end");
    if (*mins > (double)*maxs)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 739, 0, "%s", "maxs[0] >= mins[0]");
    if (mins[1] > (double)maxs[1])
        MyAssertHandler(".\\cgame\\cg_world.cpp", 740, 0, "%s", "maxs[1] >= mins[1]");
    if (mins[2] > (double)maxs[2])
        MyAssertHandler(".\\cgame\\cg_world.cpp", 741, 0, "%s", "maxs[2] >= mins[2]");
    if ((COERCE_UNSIGNED_INT(*start) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            743,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*mins) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mins[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            744,
            0,
            "%s",
            "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*maxs) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(maxs[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            745,
            0,
            "%s",
            "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            746,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    PROF_SCOPED("CG_Trace");
    CM_BoxTrace(results, start, end, mins, maxs, 0, contentMask);
    if ((COERCE_UNSIGNED_INT(results->fraction) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 752, 0, "%s", "!IS_NAN(results->fraction)");
    if (results->fraction == 1.0)
    {
        if (!results)
            MyAssertHandler("c:\\trees\\cod3\\src\\cgame\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
        results->hitType = TRACE_HITTYPE_NONE;
        results->hitId = 0;
    }
    else
    {
        if (!results)
            MyAssertHandler("c:\\trees\\cod3\\src\\cgame\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = ENTITYNUM_WORLD;
    }
    if (results->fraction == 0.0)
        goto LABEL_45;
    if (*maxs - *mins + maxs[1] - mins[1] + maxs[2] - mins[2] == 0.0)
    {
        if (staticModels)
        {
            CM_PointTraceStaticModels(results, start, end, contentMask);
            if ((COERCE_UNSIGNED_INT(results->fraction) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\cgame\\cg_world.cpp", 774, 0, "%s", "!IS_NAN(results->fraction)");
            if (results->fraction == 0.0)
            {
            LABEL_45:
                return;
            }
        }
        ignoreEntParams.baseEntity = passEntityNum;
        ignoreEntParams.parentEntity = -1;
        ignoreEntParams.ignoreSelf = 1;
        ignoreEntParams.ignoreParent = 0;
        ignoreEntParams.ignoreChildren = 0;
        ignoreEntParams.ignoreSiblings = 0;
        clip.extents.start[0] = *start;
        clip.extents.start[1] = start[1];
        clip.extents.start[2] = start[2];
        clip.extents.end[0] = *end;
        clip.extents.end[1] = end[1];
        clip.extents.end[2] = end[2];
        CM_CalcTraceExtents(&clip.extents);
        clip.ignoreEntParams = &ignoreEntParams;
        clip.contentmask = contentMask;
        clip.bLocational = locational;
        clip.priorityMap = 0;
        CG_PointTraceToEntities(&clip, results);
    }
    else
    {
        if (staticModels)
            MyAssertHandler(".\\cgame\\cg_world.cpp", 805, 0, "%s", "!staticModels");
        if (locational)
            MyAssertHandler(".\\cgame\\cg_world.cpp", 806, 0, "%s", "!locational");
        result.contentmask = contentMask;
        result.passEntityNum = passEntityNum;
        result.passOwnerNum = -1;
        Vec3Sub(maxs, mins, result.outerSize);
        Vec3Scale(result.outerSize, 0.5, result.outerSize);
        result.maxs[0] = result.outerSize[0];
        result.maxs[1] = result.outerSize[1];
        result.maxs[2] = result.outerSize[2];
        Vec3Scale(result.outerSize, -1.0, result.mins);
        result.outerSize[0] = result.outerSize[0] + 1.0;
        result.outerSize[1] = result.outerSize[1] + 1.0;
        result.outerSize[2] = result.outerSize[2] + 1.0;
        Vec3Add(maxs, mins, delta);
        Vec3Scale(delta, 0.5, delta);
        Vec3Add(start, delta, result.extents.start);
        Vec3Add(end, delta, result.extents.end);
        CM_CalcTraceExtents(&result.extents);
        CG_ClipMoveToEntities(&result, results);
        DynEntCl_ClipMoveTrace(&result, results);
    }
}

void __cdecl CG_ClipMoveToEntities(const moveclip_t *clip, trace_t *results)
{
    float start[4]; // [esp+40h] [ebp-20h] BYREF
    float end[4]; // [esp+50h] [ebp-10h] BYREF

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_world.cpp", 474, 0, "%s", "Sys_IsMainThread()");
    if (!clip)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 475, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 476, 0, "%s", "results");
    if (results->fraction <= 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            477,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction > 0.0f)",
            results->fraction);
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            478,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    PROF_SCOPED("CG_ClipMoveToEntities");
    start[0] = clip->extents.start[0];
    start[1] = clip->extents.start[1];
    start[2] = clip->extents.start[2];
    end[0] = clip->extents.end[0];
    end[1] = clip->extents.end[1];
    end[2] = clip->extents.end[2];
    start[3] = 0.0;
    end[3] = results->fraction;
    CG_ClipMoveToEntities_r(clip, 1u, start, end, results);
}

void __cdecl CG_ClipMoveToEntities_r(
    const moveclip_t *clip,
    uint16_t sectorIndex,
    const float *p1,
    const float *p2,
    trace_t *results)
{
    float v5; // [esp+8h] [ebp-98h]
    float v6; // [esp+Ch] [ebp-94h]
    float v7; // [esp+10h] [ebp-90h]
    float v8; // [esp+14h] [ebp-8Ch]
    float v9; // [esp+1Ch] [ebp-84h]
    float v10; // [esp+20h] [ebp-80h]
    float v11; // [esp+24h] [ebp-7Ch]
    float v12; // [esp+28h] [ebp-78h]
    float v13; // [esp+2Ch] [ebp-74h]
    float v14; // [esp+30h] [ebp-70h]
    const CgEntCollNode *node; // [esp+48h] [ebp-58h]
    bool side; // [esp+4Ch] [ebp-54h]
    float diff; // [esp+50h] [ebp-50h]
    const CgEntCollSector *sector; // [esp+58h] [ebp-48h]
    uint16_t listIndex; // [esp+5Ch] [ebp-44h]
    float t1; // [esp+60h] [ebp-40h]
    float frac; // [esp+64h] [ebp-3Ch]
    int32_t localClientNum; // [esp+68h] [ebp-38h]
    float offset; // [esp+6Ch] [ebp-34h]
    float t2; // [esp+70h] [ebp-30h]
    float frac2; // [esp+74h] [ebp-2Ch]
    float invDist; // [esp+7Ch] [ebp-24h]
    float p[4]; // [esp+80h] [ebp-20h] BYREF
    float mid[4]; // [esp+90h] [ebp-10h] BYREF

    if (!clip)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 374, 0, "%s", "clip");
    if (!p1)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 375, 0, "%s", "p1");
    if (!p2)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 376, 0, "%s", "p2");
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 377, 0, "%s", "results");
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            378,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    localClientNum = CG_GetCollWorldLocalClientNum();
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while (sectorIndex)
    {
        sector = CG_GetEntityCollSector(localClientNum, sectorIndex);
        for (listIndex = sector->entListHead; listIndex; listIndex = node->nextEntInSector)
        {
            node = CG_GetEntityCollNode(localClientNum, listIndex - 1);
            if (listIndex - 1 != clip->passEntityNum)
            {
                {
                    PROF_SCOPED("CG_ClipMoveToEntity");
                    CG_ClipMoveToEntity(clip, listIndex - 1, results);
                }
                if (results->allsolid || results->fraction == 0.0)
                    return;
            }
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        offset = clip->outerSize[sector->tree.axis];
        v14 = t2 - t1;
        if (v14 < 0.0)
            v13 = p2[sector->tree.axis] - sector->tree.dist;
        else
            v13 = p[sector->tree.axis] - sector->tree.dist;
        if (offset > (double)v13)
        {
            v12 = t1 - t2;
            if (v12 < 0.0)
                v11 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v11 = p[sector->tree.axis] - sector->tree.dist;
            if (v11 > -offset)
            {
                if (p[3] >= (double)results->fraction)
                    return;
                diff = t2 - t1;
                if (diff == 0.0)
                {
                    frac = 1.0;
                    frac2 = 0.0;
                    side = 0;
                }
                else
                {
                    v10 = I_fabs(diff);
                    if (diff < 0.0)
                        v9 = p[sector->tree.axis] - sector->tree.dist;
                    else
                        v9 = -t1;
                    invDist = 1.0 / v10;
                    frac = (v9 + offset) * invDist;
                    frac2 = (v9 - offset) * invDist;
                    side = diff >= 0.0;
                }
                if (frac < 0.0)
                    MyAssertHandler(".\\cgame\\cg_world.cpp", 441, 0, "%s", "frac >= 0.0f");
                v8 = 1.0 - frac;
                v7 = v8 < 0.0 ? 1.0 : frac;
                mid[0] = (*p2 - p[0]) * v7 + p[0];
                mid[1] = (p2[1] - p[1]) * v7 + p[1];
                mid[2] = (p2[2] - p[2]) * v7 + p[2];
                mid[3] = (p2[3] - p[3]) * v7 + p[3];
                CG_ClipMoveToEntities_r(clip, sector->tree.child[side], p, mid, results);
                if (results->fraction == 0.0)
                    return;
                if (frac2 > 1.0)
                    MyAssertHandler(".\\cgame\\cg_world.cpp", 453, 0, "%s\n\t(frac2) = %g", "(frac2 <= 1.0f)", frac2);
                v6 = frac2 - 0.0;
                if (v6 < 0.0)
                    v5 = 0.0;
                else
                    v5 = frac2;
                p[0] = (*p2 - p[0]) * v5 + p[0];
                p[1] = (p2[1] - p[1]) * v5 + p[1];
                p[2] = (p2[2] - p[2]) * v5 + p[2];
                p[3] = (p2[3] - p[3]) * v5 + p[3];
                sectorIndex = sector->tree.child[1 - side];
            }
            else
            {
                sectorIndex = sector->tree.child[1];
            }
        }
        else
        {
            sectorIndex = sector->tree.child[0];
        }
    }
}

void __cdecl CG_ClipMoveToEntity(const moveclip_t *clip, uint32_t entIndex, trace_t *results)
{
    int64_t v3; // [esp-Ch] [ebp-6Ch]
    uint16_t number; // [esp+6h] [ebp-5Ah]
    int32_t contents; // [esp+Ch] [ebp-54h]
    float mins[3]; // [esp+10h] [ebp-50h] BYREF
    float absMaxs[3]; // [esp+1Ch] [ebp-44h] BYREF
    const centity_s *cent; // [esp+28h] [ebp-38h]
    int32_t localClientNum; // [esp+2Ch] [ebp-34h]
    const entityState_s *p_nextState; // [esp+30h] [ebp-30h]
    float angles[3]; // [esp+34h] [ebp-2Ch] BYREF
    float maxs[3]; // [esp+40h] [ebp-20h] BYREF
    uint32_t cmodel; // [esp+4Ch] [ebp-14h]
    float oldFraction; // [esp+50h] [ebp-10h]
    float absMins[3]; // [esp+54h] [ebp-Ch] BYREF

    if (!clip)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 297, 0, "%s", "clip");
    if (entIndex >= 0x400)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            298,
            0,
            "entIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entIndex,
            1024);
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 299, 0, "%s", "results");
    localClientNum = CG_GetCollWorldLocalClientNum();
    cent = CG_GetEntity(localClientNum, entIndex);
    p_nextState = &cent->nextState;
    if (entIndex == clip->passEntityNum)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 306, 0, "%s", "entIndex != clip->passEntityNum");
    if (p_nextState->solid && (p_nextState->solid != 0xFFFFFF || (p_nextState->lerp.eFlags & 1) == 0))
    {
        contents = CG_GetEntityBModelContents(cent);
        if ((clip->contentmask & contents) != 0)
        {
            CG_GetEntityBModelBounds(cent, mins, maxs, absMins, absMaxs);
            Vec3Add(absMins, clip->mins, absMins);
            Vec3Add(absMaxs, clip->maxs, absMaxs);
            if (!CM_TraceBox(&clip->extents, absMins, absMaxs, results->fraction))
            {
                if (p_nextState->solid == 0xFFFFFF)
                {
                    cmodel = p_nextState->index.brushmodel;
                    angles[0] = cent->pose.angles[0];
                    angles[1] = cent->pose.angles[1];
                    angles[2] = cent->pose.angles[2];
                }
                else
                {
                    cmodel = CM_TempBoxModel(mins, maxs, contents);
                    angles[0] = 0.0;
                    angles[1] = 0.0;
                    angles[2] = 0.0;
                }
                oldFraction = results->fraction;
                HIDWORD(v3) = clip->contentmask;
                LODWORD(v3) = cmodel;
                CM_TransformedBoxTrace(
                    results,
                    clip->extents.start,
                    clip->extents.end,
                    clip->mins,
                    clip->maxs,
                    v3,
                    cent->pose.origin,
                    angles);
                if (oldFraction > (double)results->fraction)
                {
                    results->modelIndex = 0;
                    results->partName = 0;
                    results->partGroup = 0;
                    number = p_nextState->number;
                    if (!results)
                        MyAssertHandler("c:\\trees\\cod3\\src\\cgame\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
                    results->hitType = TRACE_HITTYPE_ENTITY;
                    results->hitId = number;
                }
            }
        }
    }
}

int32_t __cdecl CG_GetEntityBModelContents(const centity_s *cent)
{
    if (!cent)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 25, 0, "%s", "cent");
    if (!cent->nextState.solid)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 28, 0, "%s", "es->solid");
    if (cent->nextState.solid == 0xFFFFFF)
        return CM_ContentsOfModel(cent->nextState.index.brushmodel);
    if (cent->nextState.eType == ET_PLAYER)
        return 0x2000000;
    return 1;
}

void __cdecl CG_PointTraceToEntities(const pointtrace_t *clip, trace_t *results)
{
    float start[4]; // [esp+3Ch] [ebp-20h] BYREF
    float end[4]; // [esp+4Ch] [ebp-10h] BYREF

    PROF_SCOPED("CG_PointTraceToEntities");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_world.cpp", 707, 0, "%s", "Sys_IsMainThread()");
    if (!clip)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 708, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 709, 0, "%s", "results");
    if (results->fraction <= 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            710,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction > 0.0f)",
            results->fraction);
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            711,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    start[0] = clip->extents.start[0];
    start[1] = clip->extents.start[1];
    start[2] = clip->extents.start[2];
    end[0] = clip->extents.end[0];
    end[1] = clip->extents.end[1];
    end[2] = clip->extents.end[2];
    start[3] = 0.0;
    end[3] = results->fraction;
    CG_PointTraceToEntities_r(clip, 1u, start, end, results);
}

void __cdecl CG_PointTraceToEntities_r(
    const pointtrace_t *clip,
    uint16_t sectorIndex,
    const float *p1,
    const float *p2,
    trace_t *results)
{
    float v5; // [esp+10h] [ebp-5Ch]
    float v6; // [esp+14h] [ebp-58h]
    const CgEntCollNode *node; // [esp+2Ch] [ebp-40h]
    const CgEntCollSector *sector; // [esp+34h] [ebp-38h]
    uint16_t listIndex; // [esp+38h] [ebp-34h]
    float t1; // [esp+3Ch] [ebp-30h]
    float frac; // [esp+40h] [ebp-2Ch]
    int32_t localClientNum; // [esp+44h] [ebp-28h]
    float t2; // [esp+48h] [ebp-24h]
    float p[4]; // [esp+4Ch] [ebp-20h] BYREF
    float mid[4]; // [esp+5Ch] [ebp-10h] BYREF

    if (!clip)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 635, 0, "%s", "clip");
    if (!p1)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 636, 0, "%s", "p1");
    if (!p2)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 637, 0, "%s", "p2");
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 638, 0, "%s", "results");
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            639,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    localClientNum = CG_GetCollWorldLocalClientNum();
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while (sectorIndex)
    {
        sector = CG_GetEntityCollSector(localClientNum, sectorIndex);
        for (listIndex = sector->entListHead; listIndex; listIndex = node->nextEntInSector)
        {
            node = CG_GetEntityCollNode(localClientNum, listIndex - 1);
            if (listIndex - 1 != clip->ignoreEntParams->baseEntity)
            {
                {
                    PROF_SCOPED("CG_PointTraceToEntity");
                    CG_PointTraceToEntity(clip, listIndex - 1, results);
                }
                if (results->fraction == 0.0)
                    return;
            }
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        if (t1 * t2 < 0.0)
        {
            if (p[3] >= (double)results->fraction)
                return;
            frac = t1 / (t1 - t2);
            if (frac < 0.0)
                MyAssertHandler(".\\cgame\\cg_world.cpp", 677, 0, "%s", "frac >= 0.0f");
            if (frac > 1.0)
                MyAssertHandler(".\\cgame\\cg_world.cpp", 678, 0, "%s", "frac <= 1.0f");
            mid[0] = (*p2 - p[0]) * frac + p[0];
            mid[1] = (p2[1] - p[1]) * frac + p[1];
            mid[2] = (p2[2] - p[2]) * frac + p[2];
            mid[3] = (p2[3] - p[3]) * frac + p[3];
            CG_PointTraceToEntities_r(clip, sector->tree.child[t2 >= 0.0], p, mid, results);
            if (results->fraction == 0.0)
                return;
            sectorIndex = sector->tree.child[t2 < 0.0];
            p[0] = mid[0];
            p[1] = mid[1];
            p[2] = mid[2];
            p[3] = mid[3];
        }
        else
        {
            v5 = t2 - t1;
            if (v5 < 0.0)
                v6 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v6 = p[sector->tree.axis] - sector->tree.dist;
            sectorIndex = sector->tree.child[v6 < 0.0];
        }
    }
}

void __cdecl CG_PointTraceToEntity(const pointtrace_t *clip, uint32_t entIndex, trace_t *results)
{
    int64_t v3; // [esp-Ch] [ebp-C4h]
    DObj_s *v4; // [esp+8h] [ebp-B0h]
    uint16_t number; // [esp+Eh] [ebp-AAh]
    float mins[3]; // [esp+18h] [ebp-A0h] BYREF
    float angles[3]; // [esp+24h] [ebp-94h] BYREF
    float maxs[3]; // [esp+30h] [ebp-88h] BYREF
    uint32_t cmodel; // [esp+3Ch] [ebp-7Ch]
    float oldFraction; // [esp+40h] [ebp-78h]
    float localStart[3]; // [esp+44h] [ebp-74h] BYREF
    cg_s *cgameGlob; // [esp+50h] [ebp-68h]
    DObjTrace_s objTrace; // [esp+54h] [ebp-64h] BYREF
    float localEnd[3]; // [esp+70h] [ebp-48h] BYREF
    int32_t contents; // [esp+7Ch] [ebp-3Ch]
    DObj_s *dobj; // [esp+80h] [ebp-38h]
    float absMaxs[3]; // [esp+84h] [ebp-34h] BYREF
    const centity_s *cent; // [esp+90h] [ebp-28h]
    int32_t localClientNum; // [esp+94h] [ebp-24h]
    const entityState_s *p_nextState; // [esp+98h] [ebp-20h]
    int32_t partBits[4]; // [esp+9Ch] [ebp-1Ch] BYREF
    float absMins[3]; // [esp+ACh] [ebp-Ch] BYREF

    if (!clip)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 507, 0, "%s", "clip");
    if (entIndex >= 0x400)
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            508,
            0,
            "entIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entIndex,
            1024);
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 509, 0, "%s", "results");
    localClientNum = CG_GetCollWorldLocalClientNum();
    cent = CG_GetEntity(localClientNum, entIndex);
    p_nextState = &cent->nextState;
    if (entIndex == clip->ignoreEntParams->baseEntity)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 516, 0, "%s", "entIndex != clip->ignoreEntParams->baseEntity");
    if (clip->bLocational)
        v4 = CG_LocationalTraceDObj(localClientNum, entIndex);
    else
        v4 = 0;
    dobj = v4;
    if (p_nextState->solid || dobj)
    {
        if (dobj)
        {
            if (DObjHasContents(dobj, clip->contentmask))
            {
                contents = DObjGetContents(dobj);
                if ((clip->contentmask & contents) != 0)
                {
                    CG_GetEntityDobjBounds(cent, dobj, absMins, absMaxs);
                    if (!CM_TraceBox(&clip->extents, absMins, absMaxs, results->fraction))
                    {
                        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                        Vec3Sub(clip->extents.start, cgameGlob->refdef.viewOffset, localStart);
                        Vec3Sub(clip->extents.end, cgameGlob->refdef.viewOffset, localEnd);
                        objTrace.fraction = results->fraction;
                        DObjGeomTracelinePartBits(dobj, clip->contentmask, partBits);
                        CG_LocationTraceDobjCalcPose(dobj, &cent->pose, partBits);
                        DObjGeomTraceline(dobj, localStart, localEnd, clip->contentmask, &objTrace);
                        if (results->fraction > (double)objTrace.fraction)
                        {
                            results->fraction = objTrace.fraction;
                            results->surfaceFlags = objTrace.surfaceflags;
                            results->modelIndex = objTrace.modelIndex;
                            results->partName = objTrace.partName;
                            results->partGroup = objTrace.partGroup;
                            results->normal[0] = objTrace.normal[0];
                            results->normal[1] = objTrace.normal[1];
                            results->normal[2] = objTrace.normal[2];
                            results->walkable = results->normal[2] >= 0.699999988079071;
                        LABEL_33:
                            number = p_nextState->number;
                            if (!results)
                                MyAssertHandler("c:\\trees\\cod3\\src\\cgame\\../qcommon/cm_public.h", 135, 0, "%s", "trace");
                            results->hitType = TRACE_HITTYPE_ENTITY;
                            results->hitId = number;
                            results->contents = contents;
                            results->material = 0;
                        }
                    }
                }
            }
        }
        else
        {
            if (!p_nextState->solid)
                MyAssertHandler(".\\cgame\\cg_world.cpp", 572, 0, "%s", "es->solid");
            if (p_nextState->solid != 0xFFFFFF || (p_nextState->lerp.eFlags & 1) == 0)
            {
                contents = CG_GetEntityBModelContents(cent);
                if ((clip->contentmask & contents) != 0)
                {
                    CG_GetEntityBModelBounds(cent, mins, maxs, absMins, absMaxs);
                    if (!CM_TraceBox(&clip->extents, absMins, absMaxs, results->fraction))
                    {
                        if (p_nextState->solid == 0xFFFFFF)
                        {
                            cmodel = p_nextState->index.brushmodel;
                            angles[0] = cent->pose.angles[0];
                            angles[1] = cent->pose.angles[1];
                            angles[2] = cent->pose.angles[2];
                        }
                        else
                        {
                            cmodel = CM_TempBoxModel(mins, maxs, contents);
                            angles[0] = 0.0;
                            angles[1] = 0.0;
                            angles[2] = 0.0;
                        }
                        oldFraction = results->fraction;
                        HIDWORD(v3) = clip->contentmask;
                        LODWORD(v3) = cmodel;
                        CM_TransformedBoxTrace(
                            results,
                            clip->extents.start,
                            clip->extents.end,
                            vec3_origin,
                            vec3_origin,
                            v3,
                            cent->pose.origin,
                            angles);
                        if (oldFraction > (double)results->fraction)
                        {
                            results->modelIndex = 0;
                            results->partName = 0;
                            results->partGroup = 0;
                            goto LABEL_33;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_LocationTraceDobjCalcPose(const DObj_s *dobj, const cpose_t *pose, int32_t *partBits)
{
    if (!dobj)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 159, 0, "%s", "dobj");
    if (!pose)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 160, 0, "%s", "pose");
    DObjLock((DObj_s * )dobj);
    CG_DObjCalcPose(pose, dobj, partBits);
    DObjUnlock((DObj_s * )dobj);
    if (!DObjSkelAreBonesUpToDate(dobj, partBits))
        MyAssertHandler(".\\cgame\\cg_world.cpp", 172, 0, "%s", "DObjSkelAreBonesUpToDate( dobj, partBits )");
}

void __cdecl CG_LocationalTraceEntitiesOnly(
    trace_t *results,
    float *start,
    float *end,
    int32_t passEntityNum,
    int32_t contentMask)
{
    pointtrace_t clip; // [esp+48h] [ebp-40h] BYREF
    IgnoreEntParams ignoreEntParams; // [esp+7Ch] [ebp-Ch] BYREF

    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 864, 0, "%s", "results");
    if (!start)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 865, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 866, 0, "%s", "end");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_world.cpp", 867, 0, "%s", "Sys_IsMainThread()");
    if ((COERCE_UNSIGNED_INT(*start) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            868,
            0,
            "%s",
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_world.cpp",
            869,
            0,
            "%s",
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    KISAK_NULLSUB();
    {
        PROF_SCOPED("CG_LocationalTrace");
        ignoreEntParams.baseEntity = passEntityNum;
        ignoreEntParams.parentEntity = -1;
        ignoreEntParams.ignoreSelf = 1;
        ignoreEntParams.ignoreParent = 0;
        ignoreEntParams.ignoreChildren = 0;
        ignoreEntParams.ignoreSiblings = 0;
        clip.extents.start[0] = *start;
        clip.extents.start[1] = start[1];
        clip.extents.start[2] = start[2];
        clip.extents.end[0] = *end;
        clip.extents.end[1] = end[1];
        clip.extents.end[2] = end[2];
        CM_CalcTraceExtents(&clip.extents);
        clip.ignoreEntParams = &ignoreEntParams;
        clip.contentmask = contentMask;
        clip.bLocational = 1;
        clip.priorityMap = 0;
        CG_PointTraceToEntities(&clip, results);
    }
}

void __cdecl CG_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int32_t passEntityNum,
    int32_t contentMask)
{
    if (!results)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 904, 0, "%s", "results");
    if (!start)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 905, 0, "%s", "start");
    if (!mins)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 906, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 907, 0, "%s", "maxs");
    if (!end)
        MyAssertHandler(".\\cgame\\cg_world.cpp", 908, 0, "%s", "end");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame\\cg_world.cpp", 909, 0, "%s", "Sys_IsMainThread()");

    {
        PROF_SCOPED("CG_TraceCapsule");
        CG_Trace(results, (float *)start, (float *)mins, (float *)maxs, (float *)end, passEntityNum, contentMask, 0, 0);
    }
}

