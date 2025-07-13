#include "cg_local.h"
#include "cg_public.h"
#include <qcommon/mem_track.h>
#include <aim_assist/aim_assist.h>

#include <cgame_mp/cg_local_mp.h>
#include <universal/profile.h>

localEntity_s cg_localEntities[1][128];
localEntity_s cg_activeLocalEntities[1];
localEntity_s *cg_freeLocalEntities[1];

LONG g_localEntThread;

void __cdecl TRACK_cg_localents()
{
    track_static_alloc_internal(cg_localEntities, 10240, "cg_localEntities", 9);
    track_static_alloc_internal(cg_activeLocalEntities, 80, "cg_activeLocalEntities", 9);
}

void __cdecl CG_InitLocalEntities(int32_t localClientNum)
{
    int32_t entIter; // [esp+0h] [ebp-4h]

    memset((uint8_t *)cg_localEntities[localClientNum], 0, sizeof(localEntity_s[128]));
    cg_activeLocalEntities[localClientNum].next = &cg_activeLocalEntities[localClientNum];
    cg_activeLocalEntities[localClientNum].prev = &cg_activeLocalEntities[localClientNum];
    cg_freeLocalEntities[localClientNum] = cg_localEntities[localClientNum];
    for (entIter = 0; entIter < 127; ++entIter)
        cg_localEntities[localClientNum][entIter].next = &cg_localEntities[localClientNum][entIter + 1];
}

localEntity_s *__cdecl CG_AllocLocalEntity(int32_t localClientNum)
{
    localEntity_s *le; // [esp+0h] [ebp-4h]

    if (InterlockedIncrement(&g_localEntThread) != 1)
        MyAssertHandler(".\\cgame\\cg_localents.cpp", 69, 0, "%s", "Sys_InterlockedIncrement( &g_localEntThread ) == 1");
    if (!cg_freeLocalEntities[localClientNum])
        CG_FreeLocalEntity(localClientNum, cg_activeLocalEntities[localClientNum].prev);
    le = cg_freeLocalEntities[localClientNum];
    cg_freeLocalEntities[localClientNum] = le->next;
    memset((uint8_t *)le, 0, sizeof(localEntity_s));
    le->next = cg_activeLocalEntities[localClientNum].next;
    le->prev = &cg_activeLocalEntities[localClientNum];
    cg_activeLocalEntities[localClientNum].next->prev = le;
    cg_activeLocalEntities[localClientNum].next = le;
    if (InterlockedDecrement(&g_localEntThread))
        MyAssertHandler(".\\cgame\\cg_localents.cpp", 89, 0, "%s", "Sys_InterlockedDecrement( &g_localEntThread ) == 0");
    return le;
}

void __cdecl CG_FreeLocalEntity(int32_t localClientNum, localEntity_s *le)
{
    if (!le->prev)
        Com_Error(ERR_DROP, "CG_FreeLocalEntity: not active");
    le->prev->next = le->next;
    le->next->prev = le->prev;
    le->next = cg_freeLocalEntities[localClientNum];
    cg_freeLocalEntities[localClientNum] = le;
}

void __cdecl CG_AddLocalEntityTracerBeams(int32_t localClientNum)
{
    int32_t time; // [esp+38h] [ebp-10h]
    localEntity_s *prev; // [esp+3Ch] [ebp-Ch]
    localEntity_s *le; // [esp+40h] [ebp-8h]
    localEntity_s *activeLocalEntities; // [esp+44h] [ebp-4h]
    const cg_s *cgameGlob;

    PROF_SCOPED("CG_DAF_AddLocalEntities");

    if (InterlockedIncrement(&g_localEntThread) != 1)
        MyAssertHandler(".\\cgame\\cg_localents.cpp", 156, 0, "%s", "Sys_InterlockedIncrement( &g_localEntThread ) == 1");

    activeLocalEntities = &cg_activeLocalEntities[localClientNum];
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    time = cgameGlob->time;

    for (le = activeLocalEntities->prev; le != activeLocalEntities; le = prev)
    {
        prev = le->prev;
        iassert(le->leType == LE_MOVING_TRACER);
        iassert(le->pos.trType == TR_LINEAR);
        if (time >= le->endTime || time < le->pos.trTime)
            CG_FreeLocalEntity(localClientNum, le);
        else
            CG_AddMovingTracer(cgameGlob, le, &cgameGlob->refdef);
    }

    if (InterlockedDecrement(&g_localEntThread))
        MyAssertHandler(".\\cgame\\cg_localents.cpp", 178, 0, "%s", "Sys_InterlockedDecrement( &g_localEntThread ) == 0");
}

void __cdecl CG_AddMovingTracer(const cg_s *cgameGlob, localEntity_s *le, const refdef_s *refdef)
{
    float scale; // [esp+Ch] [ebp-54h]
    float v4; // [esp+10h] [ebp-50h]
    float value; // [esp+14h] [ebp-4Ch]
    float dir[3]; // [esp+24h] [ebp-3Ch] BYREF
    float lengthFromClip; // [esp+30h] [ebp-30h]
    float start[3]; // [esp+34h] [ebp-2Ch] BYREF
    float end[4]; // [esp+40h] [ebp-20h] BYREF
    float startFromBase[3]; // [esp+50h] [ebp-10h] BYREF
    float lengthFromBase; // [esp+5Ch] [ebp-4h]

    BG_EvaluateTrajectory(&le->pos, cgameGlob->time, start);
    if (!(!isnan((le->pos.trDelta)[0]) && !isnan((le->pos.trDelta)[1]) && !isnan((le->pos.trDelta)[2])))
    {
        MyAssertHandler(
            ".\\cgame\\cg_localents.cpp",
            105,
            0,
            "%s",
            "!IS_NAN((le->pos.trDelta)[0]) && !IS_NAN((le->pos.trDelta)[1]) && !IS_NAN((le->pos.trDelta)[2])");
    }
    Vec3NormalizeTo(le->pos.trDelta, dir);
    Vec3Sub(start, le->pos.trBase, startFromBase);
    lengthFromBase = Vec3Dot(startFromBase, dir);
    if (lengthFromBase < 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_localents.cpp",
            110,
            1,
            "%s\n\t(lengthFromBase) = %g",
            "(lengthFromBase >= 0.0f)",
            lengthFromBase);
    lengthFromClip = le->tracerClipDist - lengthFromBase;
    if (lengthFromClip < 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_localents.cpp",
            112,
            1,
            "%s\n\t(lengthFromClip) = %g",
            "(lengthFromClip >= 0.0f)",
            lengthFromClip);
    value = cg_tracerLength->current.value;
    v4 = value - lengthFromClip;
    if (v4 < 0.0)
        scale = value;
    else
        scale = lengthFromClip;
    end[3] = scale;
    Vec3Mad(start, scale, dir, end);
    CG_DrawTracer(start, end, refdef);
}

