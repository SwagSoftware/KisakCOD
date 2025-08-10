#include "bg_public.h"
#include "bg_local.h"
#include <universal/profile.h>
#include <xanim/dobj_utils.h>

#ifdef KISAK_MP
#include <game_mp/g_main_mp.h>
#elif KISAK_SP

#endif
#include <cgame/cg_local.h>

const scriptAnimMoveTypes_t moveAnimTable[6][2][2] =
{
  {
    { ANIM_MT_RUN, ANIM_MT_STUMBLE_FORWARD },
    { ANIM_MT_WALK, ANIM_MT_STUMBLE_WALK_FORWARD }
  },
  {
    { ANIM_MT_WALKPRONE, ANIM_MT_WALKPRONE },
    { ANIM_MT_WALKPRONE, ANIM_MT_WALKPRONE }
  },
  {
    { ANIM_MT_RUNCR, ANIM_MT_STUMBLE_CROUCH_FORWARD },
    { ANIM_MT_WALKCR, ANIM_MT_STUMBLE_CROUCH_FORWARD }
  },
  {
    { ANIM_MT_RUNBK, ANIM_MT_STUMBLE_BACKWARD },
    { ANIM_MT_WALKBK, ANIM_MT_STUMBLE_WALK_BACKWARD }
  },
  {
    { ANIM_MT_WALKPRONEBK, ANIM_MT_WALKPRONEBK },
    { ANIM_MT_WALKPRONEBK, ANIM_MT_WALKPRONEBK }
  },
  {
    { ANIM_MT_RUNCRBK, ANIM_MT_STUMBLE_CROUCH_BACKWARD },
    { ANIM_MT_WALKCRBK, ANIM_MT_STUMBLE_CROUCH_BACKWARD }
  }
}; // idb

const float bobFactorTable[6][2] =
{
  { 0.33500001f, 0.30500001f },
  { 0.25f, 0.23999999f },
  { 0.34f, 0.315f },
  { 0.36000001f, 0.32499999f },
  { 0.25f, 0.23999999f },
  { 0.34f, 0.315f }
}; // idb

const scriptAnimMoveTypes_t notMovingAnims[3][3] =
{
  { ANIM_MT_IDLE, ANIM_MT_TURNRIGHT, ANIM_MT_TURNLEFT },
  { ANIM_MT_IDLEPRONE, ANIM_MT_IDLEPRONE, ANIM_MT_IDLEPRONE },
  { ANIM_MT_IDLECR, ANIM_MT_TURNRIGHTCR, ANIM_MT_TURNLEFTCR }
}; // idb

const float CorrectSolidDeltas[26][3] =
{
  { 0.0f, 0.0f, 1.0f },
  { -1.0f, 0.0f, 1.0f },
  { 0.0f, -1.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f },
  { 0.0f, 1.0f, 1.0f },
  { -1.0f, 0.0f, 0.0f },
  { 0.0f, -1.0f, 0.0f },
  { 1.0f, 0.0f, 0.0f },
  { 0.0f, 1.0f, 0.0f },
  { 0.0f, 0.0f, -1.0f },
  { -1.0f, 0.0f, -1.0f },
  { 0.0f, -1.0f, -1.0f },
  { 1.0f, 0.0f, -1.0f },
  { 0.0f, 1.0f, -1.0f },
  { -1.0f, -1.0f, 1.0f },
  { 1.0f, -1.0f, 1.0f },
  { 1.0f, 1.0f, 1.0f },
  { -1.0f, 1.0f, 1.0f },
  { -1.0f, -1.0f, 0.0f },
  { 1.0f, -1.0f, 0.0f },
  { 1.0f, 1.0f, 0.0f },
  { -1.0f, 1.0f, 0.0f },
  { -1.0f, -1.0f, -1.0f },
  { 1.0f, -1.0f, -1.0f },
  { 1.0f, 1.0f, -1.0f },
  { -1.0f, 1.0f, -1.0f }
}; // idb

viewLerpWaypoint_s viewLerp_StandCrouch[9] =
{
  { 0, 60.0f, 0 },
  { 1, 59.5f, 0 },
  { 4, 58.5f, 0 },
  { 30, 56.0f, 0 },
  { 80, 44.0f, 0 },
  { 90, 41.5f, 0 },
  { 95, 40.5f, 0 },
  { 100, 40.0f, 0 },
  { -1, 0.0f, 0 }
}; // idb
viewLerpWaypoint_s viewLerp_CrouchStand[9] =
{
  { 0, 40.0f, 0 },
  { 5, 40.5f, 0 },
  { 10, 41.5f, 0 },
  { 20, 44.0f, 0 },
  { 70, 56.0f, 0 },
  { 96, 58.5f, 0 },
  { 99, 59.5f, 0 },
  { 100, 60.0f, 0 },
  { -1, 0.0f, 0 }
}; // idb
viewLerpWaypoint_s viewLerp_CrouchProne[11] =
{
  { 0, 40.0f, 0 },
  { 11, 38.0f, 0 },
  { 22, 33.0f, 0 },
  { 34, 25.0f, 0 },
  { 45, 16.0f, 0 },
  { 50, 15.0f, 0 },
  { 55, 16.0f, 0 },
  { 70, 18.0f, 0 },
  { 90, 17.0f, 0 },
  { 100, 11.0f, 0 },
  { -1, 0.0f, 0 }
}; // idb
viewLerpWaypoint_s viewLerp_ProneCrouch[8] =
{
  { 0, 11.0f, 0 },
  { 5, 10.0f, 0 },
  { 30, 21.0f, 0 },
  { 50, 25.0f, 0 },
  { 67, 31.0f, 0 },
  { 83, 34.0f, 0 },
  { 100, 40.0f, 0 },
  { -1, 0.0f, 0 }
}; // idb

void __cdecl PM_trace(
    pmove_t *pm,
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int32_t passEntityNum,
    int32_t contentMask)
{
    pmoveHandlers[pm->handler].trace(results, start, mins, maxs, end, passEntityNum, contentMask);
}

void __cdecl PM_playerTrace(
    pmove_t *pm,
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int32_t passEntityNum,
    int32_t contentMask)
{
    uint16_t EntityHitId; // ax

    pmoveHandlers[pm->handler].trace(results, start, mins, maxs, end, passEntityNum, contentMask);
    if (results->startsolid && (results->contents & 0x2000000) != 0)
    {
        EntityHitId = Trace_GetEntityHitId(results);
        PM_AddTouchEnt(pm, EntityHitId);
        pm->tracemask &= ~0x2000000u;
        pmoveHandlers[pm->handler].trace(results, start, mins, maxs, end, passEntityNum, contentMask & 0xFDFFFFFF);
    }
}

void __cdecl PM_AddEvent(playerState_s *ps, uint32_t newEvent)
{
    BG_AddPredictableEventToPlayerstate(newEvent, 0, ps);
}

void __cdecl PM_AddTouchEnt(pmove_t *pm, int32_t entityNum)
{
    int32_t i; // [esp+0h] [ebp-4h]

    if (entityNum != ENTITYNUM_WORLD)
    {
        if (!pm)
            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 277, 0, "%s", "pm");
        if (pm->numtouch != 32)
        {
            for (i = 0; i < pm->numtouch; ++i)
            {
                if (pm->touchents[i] == entityNum)
                    return;
            }
            pm->touchents[pm->numtouch++] = entityNum;
        }
    }
}

void __cdecl PM_ClipVelocity(const float *in, const float *normal, float *out)
{
    float scale; // [esp+0h] [ebp-14h]
    float v4; // [esp+Ch] [ebp-8h]
    float parallel; // [esp+10h] [ebp-4h]
    float parallela; // [esp+10h] [ebp-4h]

    parallel = Vec3Dot(in, normal);
    v4 = I_fabs(parallel);
    parallela = parallel - v4 * EQUAL_EPSILON;
    scale = -parallela;
    Vec3Mad(in, scale, normal, out);
}

void __cdecl PM_ProjectVelocity(const float *velIn, const float *normal, float *velOut)
{
    float v3; // [esp+8h] [ebp-3Ch]
    float v4; // [esp+Ch] [ebp-38h]
    float v5; // [esp+10h] [ebp-34h]
    float v6; // [esp+14h] [ebp-30h]
    float v7; // [esp+1Ch] [ebp-28h]
    float lengthSq2D; // [esp+24h] [ebp-20h]
    float adjusted[3]; // [esp+28h] [ebp-1Ch] BYREF
    float adjustedLengthSq; // [esp+34h] [ebp-10h]
    float newZ; // [esp+38h] [ebp-Ch]
    float lengthScale; // [esp+3Ch] [ebp-8h]
    float originalLengthSq; // [esp+40h] [ebp-4h]

    lengthSq2D = velIn[1] * velIn[1] + *velIn * *velIn;
    v6 = I_fabs(normal[2]);
    if (v6 < EQUAL_EPSILON || lengthSq2D == 0.0)
    {
        *velOut = *velIn;
        velOut[1] = velIn[1];
        velOut[2] = velIn[2];
    }
    else
    {
        v5 = normal[1] * velIn[1] + *normal * *velIn;
        newZ = -v5 / normal[2];
        v7 = velIn[1];
        adjusted[0] = *velIn;
        adjusted[1] = v7;
        adjusted[2] = newZ;
        originalLengthSq = velIn[2] * velIn[2] + lengthSq2D;
        adjustedLengthSq = newZ * newZ + lengthSq2D;
        v4 = originalLengthSq / adjustedLengthSq;
        v3 = sqrt(v4);
        lengthScale = v3;
        if (v3 < 1.0 || newZ < 0.0 || velIn[2] > 0.0)
            Vec3Scale(adjusted, lengthScale, velOut);
    }
}

int32_t __cdecl PM_GetEffectiveStance(const playerState_s *ps)
{
    if (ps->viewHeightTarget == 22)
        return 2;
    if (ps->viewHeightTarget == 40)
        return 2;
    return ps->viewHeightTarget == 11;
}

int32_t __cdecl PM_GetSprintLeft(const playerState_s *ps, int32_t gametime)
{
    int32_t v4; // [esp+8h] [ebp-10h]
    int32_t sprintLeft; // [esp+10h] [ebp-8h]
    int32_t maxSprintTime; // [esp+14h] [ebp-4h]

    maxSprintTime = BG_GetMaxSprintTime(ps);
    if (ps->sprintState.lastSprintStart)
    {
        if (ps->sprintState.lastSprintStart <= ps->sprintState.lastSprintEnd)
        {
            if (ps->sprintState.sprintDelay)
                sprintLeft = gametime
                + ps->sprintState.sprintStartMaxLength
                - (ps->sprintState.lastSprintEnd
                    - ps->sprintState.lastSprintStart)
                - ps->sprintState.lastSprintEnd
                - (int)(player_sprintRechargePause->current.value * 1000.0);
            else
                sprintLeft = gametime
                + ps->sprintState.sprintStartMaxLength
                - (ps->sprintState.lastSprintEnd
                    - ps->sprintState.lastSprintStart)
                - ps->sprintState.lastSprintEnd;
        }
        else
        {
            sprintLeft = ps->sprintState.sprintStartMaxLength - (gametime - ps->sprintState.lastSprintStart);
        }
    }
    else
    {
        sprintLeft = maxSprintTime;
    }
    if (sprintLeft < 0)
        v4 = 0;
    else
        v4 = sprintLeft;
    if (maxSprintTime < v4)
        return maxSprintTime;
    else
        return v4;
}

int32_t __cdecl PM_GetSprintLeftLastTime(const playerState_s *ps)
{
    int32_t v3; // [esp+4h] [ebp-10h]
    int32_t maxSprintTime; // [esp+10h] [ebp-4h]

    if (PM_IsSprinting(ps))
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 441, 0, "%s", "!PM_IsSprinting( ps )");
    maxSprintTime = BG_GetMaxSprintTime(ps);
    if (ps->sprintState.sprintStartMaxLength - (ps->sprintState.lastSprintEnd - ps->sprintState.lastSprintStart) < 0)
        v3 = 0;
    else
        v3 = ps->sprintState.sprintStartMaxLength - (ps->sprintState.lastSprintEnd - ps->sprintState.lastSprintStart);
    if (maxSprintTime < v3)
        return maxSprintTime;
    else
        return v3;
}

bool __cdecl PM_IsSprinting(const playerState_s *ps)
{
    return ps->sprintState.lastSprintStart && ps->sprintState.lastSprintStart > ps->sprintState.lastSprintEnd;
}

double __cdecl PM_DamageScale_Walk(int32_t damage_timer)
{
    float timer_gradiant; // [esp+0h] [ebp-10h]
    float timer_max; // [esp+4h] [ebp-Ch]

    if (!damage_timer)
        return 1.0;
    timer_max = player_dmgtimer_maxTime->current.value;
    if (timer_max == 0.0)
        return 1.0;
    timer_gradiant = -player_dmgtimer_minScale->current.value / timer_max;
    return (float)((double)damage_timer * timer_gradiant + 1.0);
}

uint32_t __cdecl PM_GroundSurfaceType(pml_t *pml)
{
    uint32_t iSurfType; // [esp+0h] [ebp-4h]

    if (!pml)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1115, 0, "%s", "pml");
    if ((pml->groundTrace.surfaceFlags & 0x2000) != 0)
        return 0;
    iSurfType = (pml->groundTrace.surfaceFlags & 0x1F00000) >> 20;
    if (iSurfType >= 0x1D)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1121, 0, "%s", "iSurfType >= 0 && iSurfType < SURF_TYPECOUNT");
    return iSurfType;
}

int32_t __cdecl PM_GetViewHeightLerpTime(const playerState_s *ps, int32_t iTarget, int32_t bDown)
{
    if (iTarget == 11)
        return 400;
    if (iTarget != 40)
        return 200;
    if (bDown)
        return 200;
    return 400;
}

bool __cdecl PlayerProneAllowed(pmove_t *pm)
{
    playerState_s *ps; // [esp+30h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2457, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2459, 0, "%s", "ps");
    if (BG_WeaponBlocksProne(pm->ps->weapon))
        return 0;
    if ((ps->pm_flags & 1) != 0)
        return 1;
    return ps->groundEntityNum != ENTITYNUM_NONE
        && BG_CheckProne(
            ps->clientNum,
            ps->origin,
            pm->maxs[0],
            30.0,
            ps->viewangles[1],
            &ps->fTorsoPitch,
            &ps->fWaistPitch,
            0,
            ps->groundEntityNum != ENTITYNUM_NONE,
            1,
            pm->handler,
            PCT_CLIENT,
            50.0);
}

void __cdecl PM_FootstepEvent(pmove_t *pm, pml_t *pml, char iOldBobCycle, char iNewBobCycle, int32_t bFootStep)
{
    uint32_t v5; // eax
    uint32_t v6; // [esp+4h] [ebp-70h]
    float mins[3]; // [esp+14h] [ebp-60h] BYREF
    float vEnd[3]; // [esp+20h] [ebp-54h] BYREF
    int32_t iClipMask; // [esp+2Ch] [ebp-48h]
    float fTraceDist; // [esp+30h] [ebp-44h]
    float maxs[3]; // [esp+34h] [ebp-40h] BYREF
    trace_t trace; // [esp+40h] [ebp-34h] BYREF
    int32_t iSurfaceType; // [esp+6Ch] [ebp-8h]
    playerState_s *ps; // [esp+70h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2993, 0, "%s", "ps");
    if ((((uint8_t)(iNewBobCycle + 64) ^ (uint8_t)(iOldBobCycle + 64)) & 0x80) != 0)
    {
        if (ps->groundEntityNum == ENTITYNUM_NONE)
        {
            if (bFootStep && (ps->pm_flags & 8) != 0)
            {
                mins[0] = pm->mins[0];
                mins[1] = pm->mins[1];
                mins[0] = mins[0] + 6.0;
                mins[1] = mins[1] + 6.0;
                mins[2] = 8.0;
                maxs[0] = pm->maxs[0];
                maxs[1] = pm->maxs[1];
                maxs[2] = pm->maxs[2];
                maxs[0] = maxs[0] - 6.0;
                maxs[1] = maxs[1] - 6.0;
                if ((float)8.0 > (double)maxs[2])
                    maxs[2] = mins[2];
                if (mins[0] > (double)maxs[0])
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3021, 0, "%s", "maxs[0] >= mins[0]");
                if (mins[1] > (double)maxs[1])
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3022, 0, "%s", "maxs[1] >= mins[1]");
                if (mins[2] > (double)maxs[2])
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3023, 0, "%s", "maxs[2] >= mins[2]");
                iClipMask = pm->tracemask & 0xFDFEFFFF;
                fTraceDist = -31.0;
                Vec3Mad(ps->origin, -31.0, ps->vLadderVec, vEnd);
                PM_playerTrace(pm, &trace, ps->origin, mins, maxs, vEnd, ps->clientNum, iClipMask);
                iSurfaceType = (trace.surfaceFlags & 0x1F00000) >> 20;
                if (trace.fraction == 1.0 || !iSurfaceType)
                    iSurfaceType = 21;
                BG_AddPredictableEventToPlayerstate(0x49u, iSurfaceType, ps);
            }
        }
        else if (bFootStep)
        {
            v6 = PM_GroundSurfaceType(pml);
            v5 = PM_FootstepType(ps, pml);
            BG_AddPredictableEventToPlayerstate(v5, v6, ps);
        }
    }
}

int32_t __cdecl PM_FootstepType(playerState_s *ps, pml_t *pml)
{
    int32_t iPMFlags; // [esp+0h] [ebp-8h]

    if (!PM_GroundSurfaceType(pml))
        return 0;
    iPMFlags = ps->pm_flags;
    if ((iPMFlags & 1) != 0)
        return 75;
    if ((iPMFlags & 0x40) != 0 || ps->leanf != 0.0)
        return 74;
    if (PM_IsSprinting(ps))
        return 72;
    return 73;
}

bool __cdecl PM_ShouldMakeFootsteps(pmove_t *pm)
{
    int32_t iStance; // [esp+8h] [ebp-Ch]
    int32_t bWalking; // [esp+Ch] [ebp-8h]
    playerState_s *ps; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3062, 0, "%s", "ps");
    bWalking = ps->pm_flags & 0x40;
    iStance = PM_GetEffectiveStance(ps);
    if (iStance == 1)
        return 0;
    if (iStance == 2)
        return 0;
    if ((ps->pm_flags & 0x20) != 0)
    {
        if (!bWalking)
            return player_footstepsThreshhold->current.value <= (double)pm->xyspeed;
    }
    else if (!bWalking)
    {
        return player_footstepsThreshhold->current.value <= (double)pm->xyspeed;
    }
    return 0;
}

void __cdecl PM_UpdateLean(
    playerState_s *ps,
    float msec,
    usercmd_s *cmd,
    void(__cdecl *capsuleTrace)(trace_t *, float *, float *, float *, float *, int32_t))
{
    float v4; // [esp+10h] [ebp-84h]
    float v5; // [esp+14h] [ebp-80h]
    float fLeanFrac; // [esp+18h] [ebp-7Ch]
    float fLean; // [esp+24h] [ebp-70h]
    float fLeanMax; // [esp+2Ch] [ebp-68h]
    float start[3]; // [esp+30h] [ebp-64h] BYREF
    float end[3]; // [esp+3Ch] [ebp-58h] BYREF
    float leanofs; // [esp+48h] [ebp-4Ch]
    trace_t trace; // [esp+4Ch] [ebp-48h] BYREF
    float tmins[3]; // [esp+78h] [ebp-1Ch] BYREF
    float tmaxs[3]; // [esp+84h] [ebp-10h] BYREF
    int32_t leaning; // [esp+90h] [ebp-4h]

    leaning = 0;
    leanofs = 0.0;
    if ((cmd->buttons & 0xC0) != 0
        && (ps->pm_flags & 0x800) == 0
        && ps->pm_type < PM_DEAD
        && (ps->groundEntityNum != ENTITYNUM_NONE || ps->pm_type == PM_NORMAL_LINKED))
    {
        if ((cmd->buttons & 0x40) != 0)
            --leaning;
        if ((cmd->buttons & 0x80) != 0)
            ++leaning;
    }
    if ((ps->eFlags & 0x300) != 0)
        leaning = 0;
    if (PM_GetEffectiveStance(ps) == 1)
        fLeanMax = 0.25;
    else
        fLeanMax = 0.5;
    leanofs = ps->leanf;
    if (leaning)
    {
        if (leaning <= 0)
        {
            if (leanofs > -fLeanMax)
                leanofs = leanofs - msec / 350.0 * fLeanMax;
            if (leanofs < -fLeanMax)
                leanofs = -fLeanMax;
        }
        else
        {
            if (fLeanMax > (double)leanofs)
                leanofs = msec / 350.0 * fLeanMax + leanofs;
            if (fLeanMax < (double)leanofs)
                leanofs = fLeanMax;
        }
    }
    else if (leanofs <= 0.0)
    {
        if (leanofs < 0.0)
        {
            leanofs = msec / 280.0 * fLeanMax + leanofs;
            if (leanofs > 0.0)
                leanofs = 0.0;
        }
    }
    else
    {
        leanofs = leanofs - msec / 280.0 * fLeanMax;
        if (leanofs < 0.0)
            leanofs = 0.0;
    }
    ps->leanf = leanofs;
    if (ps->leanf != 0.0)
    {
        fLeanFrac = ps->leanf < 0.0 ? -1.0 : 1.0;
        start[0] = ps->origin[0];
        start[1] = ps->origin[1];
        start[2] = ps->origin[2] + ps->viewHeightCurrent;
        end[0] = start[0];
        end[1] = start[1];
        end[2] = start[2];
        AddLeanToPosition(end, ps->viewangles[1], fLeanFrac, 16.0, 20.0);
        tmins[0] = -8.0;
        tmins[1] = -8.0;
        tmins[2] = -8.0;
        tmaxs[0] = 8.0;
        tmaxs[1] = 8.0;
        tmaxs[2] = 8.0;
        capsuleTrace(&trace, start, tmins, tmaxs, end, ps->clientNum);
        fLean = UnGetLeanFraction(trace.fraction);
        v5 = I_fabs(ps->leanf);
        if (fLean < (double)v5)
        {
            if (ps->leanf < 0.0)
                v4 = -1.0;
            else
                v4 = 1.0;
            ps->leanf = fLean * v4;
        }
    }
}

void __cdecl PM_UpdateViewAngles(playerState_s *ps, float msec, usercmd_s *cmd, uint8_t handler)
{
    float angle; // [esp+10h] [ebp-10h]
    float temp; // [esp+14h] [ebp-Ch]
    float oldViewYaw; // [esp+18h] [ebp-8h]

#ifdef KISAK_MP
    if (ps->pm_type == PM_INTERMISSION)
    {
        return;
    }
#endif
    if (ps->pm_type >= PM_DEAD)
    {
        if (ps->stats[1] == 999)
        {
            angle = (double)cmd->angles[1] * 0.0054931640625 + ps->delta_angles[1];
            temp = AngleNormalize360(angle);
            ps->stats[1] = (int)(temp * 0.0054931640625);
        }
    LABEL_21:
        PM_UpdateLean(
            ps,
            msec,
            cmd,
            (void(__cdecl *)(trace_t *, float *, float *, float *, float *, int))pmoveHandlers[handler].trace);
        return;
    }
    oldViewYaw = ps->viewangles[1];
    PM_UpdateViewAngles_Clamp(ps, cmd);
    if ((ps->eFlags & 0x300) != 0)
    {
        PM_UpdateViewAngles_RangeLimited(ps, oldViewYaw);
        return;
    }
    if ((ps->pm_flags & 4) != 0)
    {
        Mantle_CapView(ps);
        return;
    }
    if ((ps->pm_flags & 8) != 0 && ps->groundEntityNum == ENTITYNUM_NONE && bg_ladder_yawcap->current.value != 0.0)
        PM_UpdateViewAngles_LadderClamp(ps);
    if ((ps->pm_flags & 1) != 0)
    {
        if ((ps->eFlags & 0x300) != 0)
            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4241, 0, "%s", "!(ps->eFlags & EF_TURRET_ACTIVE)");
        PM_UpdateViewAngles_Prone(ps, msec, cmd, handler, oldViewYaw);
    }
#ifdef KISAK_MP
    if (ps->pm_type != PM_UFO && ps->pm_type != PM_NOCLIP && ps->pm_type != PM_SPECTATOR)
        goto LABEL_21;
#elif KISAK_SP
    if (ps->pm_type != PM_UFO && ps->pm_type != PM_NOCLIP && ps->pm_type != PM_MPVIEWER)
        goto LABEL_21;
#endif
}

void __cdecl PM_UpdateViewAngles_Clamp(playerState_s *ps, usercmd_s *cmd)
{
    float v2; // [esp+8h] [ebp-2Ch]
    float v3; // [esp+Ch] [ebp-28h]
    float v4; // [esp+10h] [ebp-24h]
    float v5; // [esp+14h] [ebp-20h]
    float v6; // [esp+18h] [ebp-1Ch]
    float v7; // [esp+1Ch] [ebp-18h]
    float v8; // [esp+20h] [ebp-14h]
    float temp; // [esp+24h] [ebp-10h]
    int32_t i; // [esp+28h] [ebp-Ch]
    float maxPitch; // [esp+2Ch] [ebp-8h]
    float minPitch; // [esp+30h] [ebp-4h]

    minPitch = player_view_pitch_up->current.value;
    maxPitch = player_view_pitch_down->current.value;
    for (i = 0; i < 3; ++i)
    {
        v7 = ps->delta_angles[i] + (double)cmd->angles[i] * 0.0054931640625;
        v8 = v7 * 0.002777777845039964;
        v5 = v8 + 0.5;
        v4 = floor(v5);
        temp = (v8 - v4) * 360.0;
        if (!i)
        {
            if (maxPitch >= (double)temp)
            {
                if (temp < -minPitch)
                {
                    ps->delta_angles[0] = -minPitch - (double)cmd->angles[0] * 0.0054931640625;
                    temp = -minPitch;
                }
            }
            else
            {
                ps->delta_angles[0] = maxPitch - (double)cmd->angles[0] * 0.0054931640625;
                temp = maxPitch;
            }
        }
        v6 = temp * 0.002777777845039964;
        v3 = v6 + 0.5;
        v2 = floor(v3);
        ps->viewangles[i] = (v6 - v2) * 360.0;
    }
}

void __cdecl PM_UpdateViewAngles_RangeLimited(playerState_s *ps, float oldYaw)
{
    double v2; // st7
    float v3; // [esp+8h] [ebp-10h]
    float v4; // [esp+Ch] [ebp-Ch]
    float deltaa; // [esp+10h] [ebp-8h]
    float delta; // [esp+10h] [ebp-8h]
    float deltab; // [esp+10h] [ebp-8h]
    int32_t i; // [esp+14h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        if (ps->viewAngleClampRange[i] < 180.0)
        {
            if (i == 1)
            {
                deltaa = AngleDelta(ps->viewAngleClampBase[1], oldYaw);
                delta = AngleDelta(oldYaw, ps->viewangles[1]) + deltaa;
            }
            else
            {
                delta = AngleDelta(ps->viewAngleClampBase[i], ps->viewangles[i]);
            }
            if (ps->viewAngleClampRange[i] < (double)delta || delta < -ps->viewAngleClampRange[i])
            {
                if (ps->viewAngleClampRange[i] >= (double)delta)
                    v2 = delta + ps->viewAngleClampRange[i];
                else
                    v2 = delta - ps->viewAngleClampRange[i];
                deltab = v2;
                ps->delta_angles[i] = ps->delta_angles[i] + deltab;
                if (deltab <= 0.0)
                {
                    v3 = ps->viewAngleClampBase[i] + ps->viewAngleClampRange[i];
                    ps->viewangles[i] = AngleNormalize360(v3);
                }
                else
                {
                    v4 = ps->viewAngleClampBase[i] - ps->viewAngleClampRange[i];
                    ps->viewangles[i] = AngleNormalize360(v4);
                }
            }
        }
    }
}

void __cdecl PM_UpdateViewAngles_LadderClamp(playerState_s *ps)
{
    double v1; // st7
    double v2; // st7
    float v3; // [esp+8h] [ebp-10h]
    float v4; // [esp+Ch] [ebp-Ch]
    float delta; // [esp+10h] [ebp-8h]
    float deltaa; // [esp+10h] [ebp-8h]
    float ladderFacing; // [esp+14h] [ebp-4h]

    ladderFacing = vectoyaw(ps->vLadderVec) + 180.0;
    delta = AngleDelta(ladderFacing, ps->viewangles[1]);
    if (bg_ladder_yawcap->current.value < (double)delta || delta < -bg_ladder_yawcap->current.value)
    {
        if (bg_ladder_yawcap->current.value >= (double)delta)
            v1 = delta + bg_ladder_yawcap->current.value;
        else
            v1 = delta - bg_ladder_yawcap->current.value;
        deltaa = v1;
        ps->delta_angles[1] = ps->delta_angles[1] + deltaa;
        if (deltaa <= 0.0)
        {
            v3 = ladderFacing + bg_ladder_yawcap->current.value;
            v2 = AngleNormalize360(v3);
        }
        else
        {
            v4 = ladderFacing - bg_ladder_yawcap->current.value;
            v2 = AngleNormalize360(v4);
        }
        ps->viewangles[1] = v2;
    }
}

void __cdecl PM_UpdateViewAngles_Prone(
    playerState_s *ps,
    float msec,
    usercmd_s *cmd,
    uint8_t handler,
    float oldViewYaw)
{
    double v5; // st7
    int32_t i; // eax
    uint8_t v7; // al
    uint8_t v8; // al
    uint8_t v9; // al
    float v10; // [esp+2Ch] [ebp-54h]
    float v11; // [esp+30h] [ebp-50h]
    float v12; // [esp+38h] [ebp-48h]
    float v13; // [esp+3Ch] [ebp-44h]
    float v14; // [esp+44h] [ebp-3Ch]
    float v15; // [esp+48h] [ebp-38h]
    bool v16; // [esp+4Ch] [ebp-34h]
    bool v17; // [esp+50h] [ebp-30h]
    float v18; // [esp+54h] [ebp-2Ch]
    float v19; // [esp+58h] [ebp-28h]
    float maxDeltaYaw; // [esp+5Ch] [ebp-24h]
    float delta; // [esp+60h] [ebp-20h]
    float deltaa; // [esp+60h] [ebp-20h]
    float deltab; // [esp+60h] [ebp-20h]
    float newViewYaw; // [esp+64h] [ebp-1Ch]
    float newProneYaw; // [esp+68h] [ebp-18h]
    float newProneYawa; // [esp+68h] [ebp-18h]
    int32_t bProneOK; // [esp+74h] [ebp-Ch]
    int32_t bProneOKa; // [esp+74h] [ebp-Ch]
    bool bRetry; // [esp+78h] [ebp-8h]
    bool bRetrya; // [esp+78h] [ebp-8h]
    int32_t proneBlocked; // [esp+7Ch] [ebp-4h]

    newViewYaw = ps->viewangles[1];
    proneBlocked = 0;
    delta = AngleDelta(ps->proneDirection, newViewYaw);
    v19 = bg_prone_yawcap->current.value - 5.0;
    v17 = 1;
    if (delta <= (double)v19)
    {
        v18 = -(bg_prone_yawcap->current.value - 5.0);
        if (delta >= (double)v18)
            v17 = 0;
    }
    v16 = (cmd->forwardmove || cmd->rightmove) && delta != 0.0;
    if (v17 || v16)
    {
        maxDeltaYaw = msec * 55.0 * EQUAL_EPSILON;
        v15 = I_fabs(delta);
        if (maxDeltaYaw <= (double)v15)
        {
            if (delta <= 0.0)
                v5 = ps->proneDirection + maxDeltaYaw;
            else
                v5 = ps->proneDirection - maxDeltaYaw;
            newProneYaw = v5;
        }
        else
        {
            newProneYaw = ps->viewangles[1];
        }
        bRetry = 1;
        for (i = BG_CheckProneTurned(ps, newProneYaw, handler); !i; i = BG_CheckProneTurned(ps, newProneYaw, handler))
        {
            if (!bRetry)
                goto LABEL_33;
            deltaa = AngleDelta(ps->proneDirection, newProneYaw);
            v14 = I_fabs(deltaa);
            bRetry = v14 > 1.0;
            if (v14 <= 1.0)
            {
                proneBlocked = 1;
            }
            else if (deltaa <= 0.0)
            {
                deltaa = -1.0;
            }
            else
            {
                deltaa = 1.0;
            }
            v13 = newProneYaw + deltaa;
            newProneYaw = AngleNormalize360(v13);
        }
        v7 = BG_CheckProne(
            ps->clientNum,
            ps->origin,
            15.0,
            30.0,
            ps->viewangles[1],
            0,
            0,
            1,
            ps->groundEntityNum != ENTITYNUM_NONE,
            1,
            handler,
            PCT_CLIENT,
            45.0);
        bProneOK = v7;
        if (v7)
        {
            v8 = BG_CheckProne(
                ps->clientNum,
                ps->origin,
                15.0,
                30.0,
                newProneYaw,
                0,
                0,
                1,
                ps->groundEntityNum != ENTITYNUM_NONE,
                1,
                handler,
                PCT_CLIENT,
                45.0);
            bProneOK = v8;
            if (v8)
                ps->proneDirection = newProneYaw;
        }
        if (!bProneOK)
            proneBlocked = 1;
    }
LABEL_33:
    deltab = AngleDelta(ps->proneDirection, ps->viewangles[1]);
    if (deltab != 0.0)
    {
        newProneYawa = ps->proneDirection;
        bRetrya = 1;
        while (1)
        {
            v9 = BG_CheckProne(
                ps->clientNum,
                ps->origin,
                15.0,
                30.0,
                newProneYawa,
                0,
                0,
                1,
                ps->groundEntityNum != ENTITYNUM_NONE,
                1,
                handler,
                PCT_CLIENT,
                45.0);
            bProneOKa = v9;
            if (v9)
            {
                if (BG_CheckProneTurned(ps, newProneYawa, handler))
                    break;
            }
            if (!bRetrya)
                goto LABEL_46;
            v12 = I_fabs(deltab);
            bRetrya = v12 > 1.0;
            if (v12 > 1.0)
            {
                if (deltab <= 0.0)
                    deltab = -1.0;
                else
                    deltab = 1.0;
            }
            proneBlocked = 1;
            ps->delta_angles[1] = ps->delta_angles[1] + deltab;
            v11 = ps->viewangles[1] + deltab;
            ps->viewangles[1] = AngleNormalize360(v11);
            deltab = AngleDelta(ps->proneDirection, ps->viewangles[1]);
            if (!bProneOKa)
            {
                v10 = newProneYawa + deltab;
                newProneYawa = AngleNormalize360(v10);
            }
        }
        ps->proneDirection = newProneYawa;
    }
LABEL_46:
    PM_UpdateViewAngles_ProneYawClamp(ps, deltab, proneBlocked, oldViewYaw, newViewYaw);
    PM_UpdateViewAngles_PronePitchClamp(ps);
}

int32_t __cdecl BG_CheckProneTurned(playerState_s *ps, float newProneYaw, uint8_t handler)
{
    float v4; // [esp+2Ch] [ebp-18h]
    float v5; // [esp+30h] [ebp-14h]
    float delta; // [esp+34h] [ebp-10h]
    float testYaw; // [esp+38h] [ebp-Ch]
    float fraction; // [esp+3Ch] [ebp-8h]
    float prone_feet_dist; // [esp+40h] [ebp-4h]

    delta = AngleDelta(newProneYaw, ps->viewangles[1]);
    v5 = I_fabs(delta);
    fraction = v5 / 240.0;
    v4 = newProneYaw - (1.0 - fraction) * delta;
    testYaw = AngleNormalize360(v4);
    prone_feet_dist = fraction * 45.0 + (1.0 - fraction) * 50.0;
    return (uint8_t)BG_CheckProne(
        ps->clientNum,
        ps->origin,
        15.0,
        30.0,
        testYaw,
        &ps->fTorsoPitch,
        &ps->fWaistPitch,
        1,
        ps->groundEntityNum != ENTITYNUM_NONE,
        1,
        handler,
        PCT_CLIENT,
        prone_feet_dist);
}

void __cdecl PM_UpdateViewAngles_ProneYawClamp(
    playerState_s *ps,
    float delta,
    int32_t proneBlocked,
    float oldViewYaw,
    float newViewYaw)
{
    double v5; // st7
    double v6; // st7
    float v7; // [esp+8h] [ebp-18h]
    float v8; // [esp+Ch] [ebp-14h]
    float v9; // [esp+10h] [ebp-10h]
    float angle; // [esp+14h] [ebp-Ch]
    float deltaYaw2; // [esp+18h] [ebp-8h]
    float deltaYaw1; // [esp+1Ch] [ebp-4h]
    float deltaYaw1a; // [esp+1Ch] [ebp-4h]
    float deltaa; // [esp+2Ch] [ebp+Ch]

    if (bg_prone_yawcap->current.value < (double)delta || delta < -bg_prone_yawcap->current.value)
    {
        if (bg_prone_yawcap->current.value >= (double)delta)
            v5 = delta + bg_prone_yawcap->current.value;
        else
            v5 = delta - bg_prone_yawcap->current.value;
        deltaa = v5;
        ps->delta_angles[1] = ps->delta_angles[1] + deltaa;
        if (deltaa <= 0.0)
        {
            v9 = ps->proneDirection + bg_prone_yawcap->current.value;
            v6 = AngleNormalize360(v9);
        }
        else
        {
            angle = ps->proneDirection - bg_prone_yawcap->current.value;
            v6 = AngleNormalize360(angle);
        }
        ps->viewangles[1] = v6;
    }
    if (proneBlocked)
    {
        ps->pm_flags |= 0x1000u;
        deltaYaw1 = AngleDelta(oldViewYaw, ps->viewangles[1]);
        v8 = I_fabs(deltaYaw1);
        if (v8 <= 1.0)
        {
            deltaYaw2 = AngleDelta(newViewYaw, ps->viewangles[1]);
            if (deltaYaw1 * deltaYaw2 > 0.0)
            {
                deltaYaw1a = deltaYaw1 * 0.9800000190734863;
                v7 = ps->viewangles[1] + deltaYaw1a;
                ps->viewangles[1] = AngleNormalize360(v7);
                ps->delta_angles[1] = ps->delta_angles[1] + deltaYaw1a;
            }
        }
    }
}

void __cdecl PM_UpdateViewAngles_PronePitchClamp(playerState_s *ps)
{
    double v1; // st7
    float v2; // [esp+8h] [ebp-24h]
    float v3; // [esp+Ch] [ebp-20h]
    float v4; // [esp+10h] [ebp-1Ch]
    float v5; // [esp+14h] [ebp-18h]
    float v6; // [esp+18h] [ebp-14h]
    float v7; // [esp+1Ch] [ebp-10h]
    float v8; // [esp+20h] [ebp-Ch]
    float v9; // [esp+24h] [ebp-8h]
    float delta; // [esp+28h] [ebp-4h]
    float deltaa; // [esp+28h] [ebp-4h]

    delta = AngleDelta(ps->proneTorsoPitch, ps->viewangles[0]);
    if (delta > 45.0 || delta < -45.0)
    {
        if (delta <= 45.0)
            v1 = delta + 45.0;
        else
            v1 = delta - 45.0;
        deltaa = v1;
        ps->delta_angles[0] = ps->delta_angles[0] + deltaa;
        if (deltaa <= 0.0)
        {
            v6 = ps->proneTorsoPitch + 45.0;
            v7 = v6 * 0.002777777845039964;
            v3 = v7 + 0.5;
            v2 = floor(v3);
            ps->viewangles[0] = (v7 - v2) * 360.0;
        }
        else
        {
            v8 = ps->proneTorsoPitch - 45.0;
            v9 = v8 * 0.002777777845039964;
            v5 = v9 + 0.5;
            v4 = floor(v5);
            ps->viewangles[0] = (v9 - v4) * 360.0;
        }
    }
}

void __cdecl PM_UpdatePronePitch(pmove_t *pm, pml_t *pml)
{
    char v2; // al
    double v3; // st7
    double v4; // st7
    float v5; // [esp+2Ch] [ebp-54h]
    float v6; // [esp+30h] [ebp-50h]
    float v7; // [esp+34h] [ebp-4Ch]
    float v8; // [esp+38h] [ebp-48h]
    float v9; // [esp+3Ch] [ebp-44h]
    float v10; // [esp+40h] [ebp-40h]
    float v11; // [esp+44h] [ebp-3Ch]
    float v12; // [esp+48h] [ebp-38h]
    float v13; // [esp+4Ch] [ebp-34h]
    float v14; // [esp+50h] [ebp-30h]
    float v15; // [esp+5Ch] [ebp-24h]
    float v16; // [esp+68h] [ebp-18h]
    float fTargPitch; // [esp+74h] [ebp-Ch]
    float fTargPitcha; // [esp+74h] [ebp-Ch]
    float delta; // [esp+78h] [ebp-8h]
    float deltaa; // [esp+78h] [ebp-8h]
    playerState_s *ps; // [esp+7Ch] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4279, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4282, 0, "%s", "ps");
    if ((ps->pm_flags & 1) != 0)
    {
        if (ps->groundEntityNum == ENTITYNUM_NONE)
        {
            if (pml->groundPlane)
                v2 = BG_CheckProne(
                    ps->clientNum,
                    ps->origin,
                    15.0,
                    30.0,
                    ps->proneDirection,
                    &ps->fTorsoPitch,
                    &ps->fWaistPitch,
                    1,
                    ps->groundEntityNum != ENTITYNUM_NONE,
                    pml->groundTrace.walkable,
                    pm->handler,
                    PCT_CLIENT,
                    50.0);
            else
                v2 = BG_CheckProne(
                    ps->clientNum,
                    ps->origin,
                    15.0,
                    30.0,
                    ps->proneDirection,
                    &ps->fTorsoPitch,
                    &ps->fWaistPitch,
                    1,
                    ps->groundEntityNum != ENTITYNUM_NONE,
                    1,
                    pm->handler,
                    PCT_CLIENT,
                    50.0);
            if (!v2)
            {
                BG_AddPredictableEventToPlayerstate(7u, 0, ps);
                ps->pm_flags |= 0x1000u;
            }
        }
        else if (pml->groundPlane && !pml->groundTrace.walkable)
        {
            BG_AddPredictableEventToPlayerstate(7u, 0, ps);
        }
        if (pml->groundPlane)
        {
            fTargPitch = PitchForYawOnNormal(ps->proneDirection, pml->groundTrace.normal);
            v3 = AngleDelta(fTargPitch, ps->proneDirectionPitch);
        }
        else
        {
            v3 = AngleDelta(0.0, ps->proneDirectionPitch);
        }
        delta = v3;
        if (delta != 0.0)
        {
            v14 = I_fabs(delta);
            v13 = pml->frametime * 70.0;
            if (v14 <= (double)v13)
            {
                ps->proneDirectionPitch = ps->proneDirectionPitch + delta;
            }
            else
            {
                if (delta < 0.0)
                    v12 = -1.0;
                else
                    v12 = 1.0;
                ps->proneDirectionPitch = pml->frametime * 70.0 * v12 + ps->proneDirectionPitch;
            }
            v16 = ps->proneDirectionPitch * 0.002777777845039964;
            v11 = v16 + 0.5;
            v10 = floor(v11);
            ps->proneDirectionPitch = (v16 - v10) * 360.0;
        }
        if (pml->groundPlane)
        {
            fTargPitcha = PitchForYawOnNormal(ps->viewangles[1], pml->groundTrace.normal);
            v4 = AngleDelta(fTargPitcha, ps->proneTorsoPitch);
        }
        else
        {
            v4 = AngleDelta(0.0, ps->proneTorsoPitch);
        }
        deltaa = v4;
        if (deltaa != 0.0)
        {
            v9 = I_fabs(deltaa);
            v8 = pml->frametime * 70.0;
            if (v9 <= (double)v8)
            {
                ps->proneTorsoPitch = ps->proneTorsoPitch + deltaa;
            }
            else
            {
                if (deltaa < 0.0)
                    v7 = -1.0;
                else
                    v7 = 1.0;
                ps->proneTorsoPitch = pml->frametime * 70.0 * v7 + ps->proneTorsoPitch;
            }
            v15 = ps->proneTorsoPitch * 0.002777777845039964;
            v6 = v15 + 0.5;
            v5 = floor(v6);
            ps->proneTorsoPitch = (v15 - v5) * 360.0;
        }
    }
}

void __cdecl PM_SetProneMovementOverride(playerState_s *ps)
{
    if ((ps->pm_flags & 1) != 0)
        ps->pm_flags |= 0x200u;
}

void __cdecl PM_MeleeChargeStart(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4737, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4740, 0, "%s", "ps");
    if (pm->cmd.meleeChargeDist)
    {
        ps->pm_flags |= 0x20000u;
        ps->meleeChargeYaw = pm->cmd.meleeChargeYaw;
        ps->meleeChargeDist = pm->cmd.meleeChargeDist;
        ps->meleeChargeTime = 0;
    }
    else
    {
        PM_MeleeChargeClear(ps);
    }
}

void __cdecl PM_MeleeChargeClear(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4724, 0, "%s", "ps");
    ps->pm_flags &= ~0x20000u;
    ps->meleeChargeYaw = 0.0;
    ps->meleeChargeDist = 0;
    ps->meleeChargeTime = 0;
}

void __cdecl Pmove(pmove_t *pm)
{
    int32_t msec; // [esp+38h] [ebp-Ch]
    int32_t finalTime; // [esp+3Ch] [ebp-8h]
    playerState_s *ps; // [esp+40h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 5272, 0, "%s", "ps");
    finalTime = pm->cmd.serverTime;
    if (finalTime >= ps->commandTime)
    {
        if (finalTime > ps->commandTime + 1000)
            ps->commandTime = finalTime - 1000;
        pm->numtouch = 0;
        while (ps->commandTime != finalTime)
        {
            msec = finalTime - ps->commandTime;
            if (msec > 66)
                msec = 66;
            pm->cmd.serverTime = msec + ps->commandTime;
            {
                PROF_SCOPED("PmoveSingle");
                PmoveSingle(pm);
            }
            memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
        }
    }
}

void __cdecl PmoveSingle(pmove_t *pm)
{
    int32_t v1; // ecx
    double v2; // st7
    float msec; // [esp+0h] [ebp-1C4h]
    float v4; // [esp+Ch] [ebp-1B8h]
    float scale; // [esp+10h] [ebp-1B4h]
    float v6; // [esp+18h] [ebp-1ACh]
    float v7; // [esp+20h] [ebp-1A4h]
    float v8; // [esp+28h] [ebp-19Ch]
    float v9; // [esp+30h] [ebp-194h]
    float *oldVelocity; // [esp+38h] [ebp-18Ch]
    float *v11; // [esp+3Ch] [ebp-188h]
    float frametime; // [esp+40h] [ebp-184h]
    float *v13; // [esp+ECh] [ebp-D8h]
    float *v14; // [esp+F0h] [ebp-D4h]
    float v15; // [esp+FCh] [ebp-C8h]
    float rightmove; // [esp+104h] [ebp-C0h]
    float v17; // [esp+108h] [ebp-BCh]
    float forwardmove; // [esp+110h] [ebp-B4h]
    float *v19; // [esp+114h] [ebp-B0h]
    float *v20; // [esp+118h] [ebp-ACh]
    float *velocity; // [esp+11Ch] [ebp-A8h]
    float realVelSqrd; // [esp+120h] [ebp-A4h]
    float move[3]; // [esp+124h] [ebp-A0h] BYREF
    float supposedVelSqrd; // [esp+130h] [ebp-94h]
    pml_t pml; // [esp+134h] [ebp-90h] BYREF
    float velocityChange[2]; // [esp+1B4h] [ebp-10h]
    int32_t stance; // [esp+1BCh] [ebp-8h]
    playerState_s *ps; // [esp+1C0h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4839, 0, "%s", "ps");
    BG_AnimUpdatePlayerStateConditions(pm);
    if ((ps->pm_flags & 0x20000) != 0)
        pm->cmd.forwardmove = 127;
    if ((ps->pm_flags & 0x800) != 0)
    {
        pm->cmd.buttons &= 0x1300u;
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        velocity = ps->velocity;
        ps->velocity[0] = 0.0;
        velocity[1] = 0.0;
        velocity[2] = 0.0;
    }
    else if ((ps->pm_flags & 0x400) != 0)
    {
        pm->cmd.buttons &= 0x1301u;
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        v20 = ps->velocity;
        ps->velocity[0] = 0.0;
        v20[1] = 0.0;
        v20[2] = 0.0;
    }
    else if (PM_IsPlayerFrozenByWeapon(ps))
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        pm->cmd.buttons &= 0xFFFFFB3F;
        v19 = ps->velocity;
        ps->velocity[0] = 0.0;
        v19[1] = 0.0;
        v19[2] = 0.0;
    }
    if ((pm->cmd.buttons & 0x100000) != 0)
    {
        pm->cmd.buttons &= 0x101B02u;
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    ps->pm_flags &= ~0x1000u;
    if (ps->pm_type >= PM_DEAD)
        pm->tracemask &= ~0x2000000u;
    if ((ps->pm_flags & 1) == 0 || BG_UsingSniperScope(ps))
    {
        ps->pm_flags &= ~0x200u;
    }
    else
    {
        if (pm->cmd.forwardmove != pm->oldcmd.forwardmove)
        {
            forwardmove = (float)pm->cmd.forwardmove;
            v9 = I_fabs(forwardmove);
            v17 = (float)pm->oldcmd.forwardmove;
            v8 = I_fabs(v17);
            if (v8 < (double)v9)
                goto LABEL_90;
        }
        if (pm->cmd.rightmove == pm->oldcmd.rightmove
            || (rightmove = (float)pm->cmd.rightmove,
                v7 = I_fabs(rightmove),
                v15 = (float)pm->oldcmd.rightmove,
                v6 = I_fabs(v15),
                v6 >= (double)v7))
        {
            if ((ps->pm_flags & 0x10) == 0 && (ps->weaponstate <= 4u || ps->weaponstate == 7))
                ps->pm_flags &= ~0x200u;
        }
        else
        {
        LABEL_90:
            if (PM_InteruptWeaponWithProneMove(ps))
            {
                ps->pm_flags &= ~0x200u;
                PM_ExitAimDownSight(ps);
            }
        }
    }
    stance = PM_GetEffectiveStance(ps);
    if ((ps->pm_flags & 0x10) != 0 && stance == 1 && !BG_UsingSniperScope(ps))
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    if ((pm->cmd.buttons & 0x100000) != 0)
        v1 = ps->eFlags | 0x200000;
    else
        v1 = ps->eFlags & 0xFFDFFFFF;
    ps->eFlags = v1;
    ps->eFlags &= ~0x40u;

    if (
#ifdef KISAK_MP
        ps->pm_type != PM_INTERMISSION && 
#endif
        (ps->pm_flags & 0x400) == 0
        && (!ps->weaponstate || ps->weaponstate == 5)
        && PM_WeaponAmmoAvailable(ps)
        && (pm->cmd.buttons & 1) != 0)
    {
        ps->eFlags |= 0x40u;
    }

    if (ps->pm_type < PM_DEAD && (pm->cmd.buttons & 0x101) == 0)
        ps->pm_flags &= ~0x400u;
    memset((uint8_t *)&pml, 0, sizeof(pml));
    pml.msec = pm->cmd.serverTime - ps->commandTime;
    if (pml.msec >= 1)
    {
        if (pml.msec > 200)
            pml.msec = 200;
    }
    else
    {
        pml.msec = 1;
    }
    ps->commandTime = pm->cmd.serverTime;
    pml.previous_origin[0] = ps->origin[0];
    pml.previous_origin[1] = ps->origin[1];
    pml.previous_origin[2] = ps->origin[2];
    pml.previous_velocity[0] = ps->velocity[0];
    pml.previous_velocity[1] = ps->velocity[1];
    pml.previous_velocity[2] = ps->velocity[2];
    pml.frametime = (double)pml.msec * EQUAL_EPSILON;
    PM_AdjustAimSpreadScale(pm, &pml);
    msec = (float)pml.msec;
    PM_UpdateViewAngles(ps, msec, &pm->cmd, pm->handler);
    AngleVectors(ps->viewangles, pml.forward, pml.right, pml.up);
    if (pm->cmd.forwardmove >= 0)
    {
        if (pm->cmd.forwardmove > 0 || !pm->cmd.forwardmove && pm->cmd.rightmove)
            ps->pm_flags &= ~0x20u;
    }
    else
    {
        ps->pm_flags |= 0x20u;
    }
#ifdef KISAK_MP
    if (ps->pm_type >= PM_LASTSTAND)
#elif KISAK_SP
    if (ps->pm_type >= PM_DEAD)
#endif
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    if (stance == 1 && (ps->pm_flags & 0x200) != 0)
    {
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    Mantle_ClearHint(ps);
    PM_MeleeChargeUpdate(pm, &pml);
    switch (ps->pm_type)
    {
    case 1:
    case 8:
        PM_ClearLadderFlag(ps);
        ps->groundEntityNum = ENTITYNUM_NONE;
        memset(&pml.walking, 0, 12);
        v14 = ps->velocity;
        ps->velocity[0] = 0.0;
        v14[1] = 0.0;
        v14[2] = 0.0;
        PM_UpdateAimDownSightFlag(pm, &pml);
        PM_UpdateSprint(pm, &pml);
        PM_UpdatePlayerWalkingFlag(pm);
        PM_DropTimers(ps, &pml);
        PM_CheckDuck(pm, &pml);
        PM_Footsteps(pm, &pml);
        PM_Weapon(pm, &pml);
        break;
    case 2:
        PM_ClearLadderFlag(ps);
        PM_UpdateAimDownSightFlag(pm, &pml);
        PM_UpdateSprint(pm, &pml);
        PM_UpdatePlayerWalkingFlag(pm);
        PM_DropTimers(ps, &pml);
        PM_NoclipMove(pm, &pml);
        PM_UpdateAimDownSightLerp(pm, &pml);
        break;
    case 3:
        PM_ClearLadderFlag(ps);
        PM_UpdateAimDownSightFlag(pm, &pml);
        PM_UpdateSprint(pm, &pml);
        PM_UpdatePlayerWalkingFlag(pm);
        PM_DropTimers(ps, &pml);
        PM_UFOMove(pm, &pml);
        PM_UpdateAimDownSightLerp(pm, &pml);
        break;
    case 4:
        PM_ClearLadderFlag(ps);
        PM_UpdateAimDownSightFlag(pm, &pml);
        PM_UpdateSprint(pm, &pml);
        PM_UpdatePlayerWalkingFlag(pm);
        PM_CheckDuck(pm, &pml);
        PM_DropTimers(ps, &pml);
        PM_FlyMove(pm, &pml);
        PM_UpdateAimDownSightLerp(pm, &pml);
        break;
    case 5:
        PM_ClearLadderFlag(ps);
        PM_UpdateAimDownSightFlag(pm, &pml);
        PM_UpdateSprint(pm, &pml);
        PM_UpdateAimDownSightLerp(pm, &pml);
        break;
    case 6:
        PM_ClearLadderFlag(ps);
        ps->eFlags &= 0xFFFFFCFF;
        goto LABEL_69;
    default:
    LABEL_69:
        if ((ps->eFlags & 0x300) != 0)
        {
            PM_ClearLadderFlag(ps);
            ps->groundEntityNum = ENTITYNUM_NONE;
            memset(&pml.walking, 0, 12);
            v13 = ps->velocity;
            ps->velocity[0] = 0.0;
            v13[1] = 0.0;
            v13[2] = 0.0;
            PM_UpdateAimDownSightFlag(pm, &pml);
            PM_UpdateSprint(pm, &pml);
            PM_UpdatePlayerWalkingFlag(pm);
            TurretNVGTrigger(pm);
            PM_DropTimers(ps, &pml);
            PM_CheckDuck(pm, &pml);
            PM_UpdateAimDownSightLerp(pm, &pml);
            PM_Footsteps(pm, &pml);
            PM_ResetWeaponState(ps);
        }
        else
        {
            if ((ps->pm_flags & 4) == 0)
            {
                PM_UpdateAimDownSightFlag(pm, &pml);
                PM_UpdateSprint(pm, &pml);
                PM_UpdatePlayerWalkingFlag(pm);
                PM_CheckDuck(pm, &pml);
                {
                    PROF_SCOPED("PM_GroundTrace");
                    PM_GroundTrace(pm, &pml);
                }
            }
            {
                PROF_SCOPED("Mantle_Check");
                Mantle_Check(pm, &pml);
            }
            if ((ps->pm_flags & 4) != 0)
            {
                PM_ClearLadderFlag(ps);
                ps->groundEntityNum = ENTITYNUM_NONE;
                pml.groundPlane = 0;
                pml.walking = 0;
                PM_UpdateAimDownSightFlag(pm, &pml);
                PM_UpdateSprint(pm, &pml);
                PM_UpdatePlayerWalkingFlag(pm);
                PM_CheckDuck(pm, &pml);
                Mantle_Move(pm, ps, &pml);
                {
                    PROF_SCOPED("PM_Weapon");
                    PM_Weapon(pm, &pml);
                }
            }
            else
            {
                PM_UpdatePronePitch(pm, &pml);
                PM_DropTimers(ps, &pml);

#ifdef KISAK_MP
                if (ps->pm_type >= PM_LASTSTAND)
#elif KISAK_SP
                if (ps->pm_type >= PM_DEAD)
#endif
                {
                    PM_DeadMove(ps, &pml);
                }

                PM_CheckLadderMove(pm, &pml);

                {
                    PROF_SCOPED("PM_Move");
                    if ((ps->pm_flags & 8) != 0)
                    {
                        PM_LadderMove(pm, &pml);
                    }
                    else if (pml.walking)
                    {
                        PM_WalkMove(pm, &pml);
                    }
                    else
                    {
                        PM_AirMove(pm, &pml);
                    }
                }

                {
                    PROF_SCOPED("PM_GroundTrace");
                    PM_GroundTrace(pm, &pml);
                }
                {
                    PROF_SCOPED("PM_Footsteps");
                    PM_Footsteps(pm, &pml);
                }
                {
                    PROF_SCOPED("PM_Weapon");
                    PM_Weapon(pm, &pml);
                }
                {
                    PROF_SCOPED("PM_FoliageSounds");
                    PM_FoliageSounds(pm);
                }
                Vec3Sub(ps->origin, pml.previous_origin, move);
                v2 = Vec3LengthSq(move);
                realVelSqrd = v2 / (pml.frametime * pml.frametime);
                supposedVelSqrd = Vec3LengthSq(ps->velocity);
                if (realVelSqrd < supposedVelSqrd * 0.25)
                {
                    scale = 1.0 / pml.frametime;
                    Vec3Scale(move, scale, ps->velocity);
                }
                velocityChange[0] = ps->velocity[0] - ps->oldVelocity[0];
                velocityChange[1] = ps->velocity[1] - ps->oldVelocity[1];
                v4 = 1.0 - pml.frametime;
                if (v4 < 0.0)
                    frametime = 1.0;
                else
                    frametime = pml.frametime;
                velocityChange[0] = frametime * velocityChange[0];
                velocityChange[1] = frametime * velocityChange[1];
                oldVelocity = ps->oldVelocity;
                v11 = ps->oldVelocity;
                ps->oldVelocity[0] = ps->oldVelocity[0] + velocityChange[0];
                oldVelocity[1] = v11[1] + velocityChange[1];
                Sys_SnapVector(ps->velocity);
            }
        }
        break;
    }
}

void __cdecl PM_UpdateSprint(pmove_t *pm, const pml_t *pml)
{
    SprintState *p_sprintState; // [esp+4h] [ebp-10h]
    int32_t sprintLeft; // [esp+8h] [ebp-Ch]
    playerState_s *ps; // [esp+Ch] [ebp-8h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 570, 0, "%s", "ps");
    p_sprintState = &ps->sprintState;
    if (ps->sprintState.sprintButtonUpRequired && (pm->cmd.buttons & 2) == 0)
        p_sprintState->sprintButtonUpRequired = 0;
    if (ps->pm_type >= PM_NOCLIP || BG_GetMaxSprintTime(ps) <= 0)
    {
    LABEL_13:
        PM_EndSprint(ps, pm);
        return;
    }
    if ((ps->pm_flags & 0x8000) != 0)
    {
        if (pm->cmd.serverTime - ps->sprintState.lastSprintStart >= ps->sprintState.sprintStartMaxLength)
        {
            PM_EndSprint(ps, pm);
            ps->sprintState.sprintDelay = 1;
            return;
        }
        if (PM_SprintEndingButtons(ps, pm->cmd.forwardmove, pm->cmd.buttons))
            goto LABEL_13;
        if ((pm->oldcmd.buttons & 2) == 0 && (pm->cmd.buttons & 2) != 0)
        {
            PM_EndSprint(ps, pm);
            p_sprintState->sprintButtonUpRequired = 1;
        }
    }
    else if ((!ps->sprintState.sprintDelay
        || player_sprintRechargePause->current.value * 1000.0 <= (double)(pm->cmd.serverTime
            - ps->sprintState.lastSprintEnd))
        && (pm->cmd.buttons & 2) != 0
        && (ps->pm_flags & 0x40000) == 0
        && !p_sprintState->sprintButtonUpRequired
        && !PM_SprintStartInterferingButtons(ps, pm->cmd.forwardmove, pm->cmd.buttons)
        && PM_CanStand(ps, pm))
    {
        sprintLeft = PM_GetSprintLeft(ps, pm->cmd.serverTime);
        if (player_sprintMinTime->current.value * 1000.0 < (double)sprintLeft)
            PM_StartSprint(ps, pm, pml, sprintLeft);
    }
}

void __cdecl PM_StartSprint(playerState_s *ps, pmove_t *pm, const pml_t *pml, int32_t sprintLeft)
{
    if (ps->sprintState.lastSprintEnd && ps->sprintState.lastSprintEnd < ps->sprintState.lastSprintStart)
        MyAssertHandler(
            ".\\bgame\\bg_pmove.cpp",
            477,
            0,
            "%s",
            "ss->lastSprintEnd == 0 || ss->lastSprintEnd >= ss->lastSprintStart");
    ps->sprintState.sprintStartMaxLength = sprintLeft;
    ps->sprintState.lastSprintStart = pm->cmd.serverTime;
    ps->pm_flags |= 0x8000u;
}

void __cdecl PM_EndSprint(playerState_s *ps, pmove_t *pm)
{
    if ((ps->pm_flags & 0x8000) != 0)
    {
        ps->sprintState.sprintDelay = 0;
        ps->sprintState.lastSprintEnd = pm->cmd.serverTime;
        ps->pm_flags &= ~0x8000u;
        if ((pm->cmd.buttons & 2) != 0)
            ps->sprintState.sprintButtonUpRequired = 1;
    }
}

bool __cdecl PM_SprintStartInterferingButtons(const playerState_s *ps, int32_t forwardSpeed, int16_t buttons)
{
    if ((ps->pm_flags & 8) != 0)
        return 1;
    if (forwardSpeed <= player_sprintForwardMinimum->current.integer)
        return 1;
    if ((buttons & 0xC435) != 0)
        return 1;
    if (ps->leanf != 0.0)
        return 1;
    if ((ps->pm_flags & 0x1001C) != 0)
        return 1;
    if ((ps->pm_flags & 0x4000) != 0 && !ps->pm_time)
        return 0;
    return ps->weaponstate == 12
        || ps->weaponstate == 13
        || ps->weaponstate == 14
        || ps->weaponstate >= 15 && ps->weaponstate <= 20;
}

bool __cdecl PM_SprintEndingButtons(const playerState_s *ps, int32_t forwardSpeed, int16_t buttons)
{
    if ((ps->pm_flags & 0x10018) != 0)
        return 1;
    if (forwardSpeed <= player_sprintForwardMinimum->current.integer)
        return 1;
    if ((buttons & 0xC735) != 0)
        return 1;
    if (ps->leanf != 0.0)
        return 1;
    return ps->weaponstate == 12
        || ps->weaponstate == 13
        || ps->weaponstate == 14
        || ps->weaponstate >= 15 && ps->weaponstate <= 20
        || ps->weaponstate == 25
        || ps->weaponstate == 26;
}

bool __cdecl PM_CanStand(playerState_s *ps, pmove_t *pm)
{
    trace_t trace; // [esp+0h] [ebp-2Ch] BYREF

    if ((ps->pm_flags & 3) == 0)
        return 1;
    pmoveHandlers[pm->handler].trace(
        &trace,
        ps->origin,
        playerMins,
        playerMaxs,
        ps->origin,
        ps->clientNum,
        pm->tracemask & 0xFDFFFFFF);
    return !trace.allsolid;
}

void __cdecl PM_FlyMove(pmove_t *pm, pml_t *pml)
{
    float wishdir[3]; // [esp+20h] [ebp-40h] BYREF
    float wishvel[3]; // [esp+2Ch] [ebp-34h]
    float wishspeed; // [esp+38h] [ebp-28h]
    float forward[3]; // [esp+3Ch] [ebp-24h] BYREF
    float up[3]; // [esp+48h] [ebp-18h] BYREF
    int32_t i; // [esp+54h] [ebp-Ch]
    float scale; // [esp+58h] [ebp-8h]
    playerState_s *ps; // [esp+5Ch] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1148, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1151, 0, "%s", "ps");
    PM_Friction(ps, pml);
    scale = PM_CmdScale(ps, &pm->cmd);
    if (scale == 0.0)
    {
        wishvel[0] = 0.0;
        wishvel[1] = 0.0;
        wishvel[2] = 0.0;
    }
    else
    {
        up[1] = 0.0;
        up[0] = 0.0;
        up[2] = 1.0;
        Vec3Cross(up, pml->right, forward);
        for (i = 0; i < 3; ++i)
            wishvel[i] = ((double)pm->cmd.forwardmove * forward[i] + (double)pm->cmd.rightmove * pml->right[i]) * scale;
    }
    if (ps->speed)
    {
        scale = PM_MoveScale(ps, 0.0, 0.0, 127.0);
        if ((pm->cmd.buttons & 0x40) != 0)
            wishvel[2] = wishvel[2] - scale * 127.0;
        if ((pm->cmd.buttons & 0x80) != 0)
            wishvel[2] = scale * 127.0 + wishvel[2];
    }
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    PM_Accelerate(ps, pml, wishdir, wishspeed, 8.0);
    PM_StepSlideMove(pm, pml, 0);
}

void __cdecl PM_Friction(playerState_s *ps, pml_t *pml)
{
    float scale; // [esp+8h] [ebp-2Ch]
    float value; // [esp+Ch] [ebp-28h]
    float control; // [esp+14h] [ebp-20h]
    float *vel; // [esp+18h] [ebp-1Ch]
    float newspeed; // [esp+1Ch] [ebp-18h]
    float drop; // [esp+20h] [ebp-14h]
    float speed; // [esp+24h] [ebp-10h]
    float vec[3]; // [esp+28h] [ebp-Ch] BYREF

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 650, 0, "%s", "ps");
    vel = ps->velocity;
    vec[0] = ps->velocity[0];
    vec[1] = ps->velocity[1];
    vec[2] = ps->velocity[2];
    if (pml->walking)
        vec[2] = 0.0;
    speed = Vec3Length(vec);
    if (speed >= 1.0)
    {
        drop = 0.0;
        if ((ps->pm_flags & 0x20000) != 0)
        {
            drop = player_meleeChargeFriction->current.value * pml->frametime;
        }
        else if (pml->walking && (pml->groundTrace.surfaceFlags & 2) == 0 && (ps->pm_flags & 0x100) == 0)
        {
            if (stopspeed->current.value <= (double)speed)
                value = speed;
            else
                value = stopspeed->current.value;
            control = value;
            if ((ps->pm_flags & 0x80) != 0)
            {
                control = value * 0.300000011920929;
            }
            else if ((ps->pm_flags & 0x4000) != 0)
            {
                control = Jump_ReduceFriction(ps) * value;
            }
            drop = control * friction->current.value * pml->frametime + drop;
        }
#ifdef KISAK_MP
        if (ps->pm_type == PM_SPECTATOR)
            drop = speed * 5.0 * pml->frametime + drop;
#endif
        newspeed = speed - drop;
        if (newspeed < 0.0)
            newspeed = 0.0;
        scale = newspeed / speed;
        Vec3Scale(vel, scale, vel);
    }
    else
    {
        *vel = 0.0;
        ps->velocity[1] = 0.0;
        ps->velocity[2] = 0.0;
    }
}

void __cdecl PM_Accelerate(playerState_s *ps, const pml_t *pml, const float *wishdir, float wishspeed, float accel)
{
    float value; // [esp+Ch] [ebp-44h]
    float wishVelocity[3]; // [esp+1Ch] [ebp-34h] BYREF
    float pushDir[3]; // [esp+28h] [ebp-28h] BYREF
    float pushLen; // [esp+34h] [ebp-1Ch]
    float canPush; // [esp+38h] [ebp-18h]
    float inertiaspeed; // [esp+3Ch] [ebp-14h]
    float control; // [esp+40h] [ebp-10h]
    float addspeed; // [esp+44h] [ebp-Ch]
    float currentspeed; // [esp+48h] [ebp-8h]
    float accelspeed; // [esp+4Ch] [ebp-4h]

    if ((ps->pm_flags & 8) != 0)
    {
        Vec3Scale(wishdir, wishspeed, wishVelocity);
        Vec3Sub(wishVelocity, ps->velocity, pushDir);
        pushLen = Vec3Normalize(pushDir);
        canPush = accel * pml->frametime * wishspeed;
        if (pushLen < (double)canPush)
            canPush = pushLen;
        Vec3Mad(ps->velocity, canPush, pushDir, ps->velocity);
    }
    else
    {
        if (!ps)
            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 768, 0, "%s", "ps");
        currentspeed = Vec3Dot(ps->velocity, wishdir);
        addspeed = wishspeed - currentspeed;
        if (addspeed > 0.0)
        {
            if (stopspeed->current.value <= (double)wishspeed)
                value = wishspeed;
            else
                value = stopspeed->current.value;
            control = value;
            accelspeed = accel * pml->frametime * value;
            if (addspeed < (double)accelspeed)
                accelspeed = addspeed;
            inertiaspeed = PM_PlayerInertia(ps, accelspeed, wishdir);
            Vec3Mad(ps->velocity, inertiaspeed, wishdir, ps->velocity);
        }
    }
}

double __cdecl PM_PlayerInertia(const playerState_s *ps, float accelspeed, const float *wishdir)
{
    float v4; // [esp+8h] [ebp-8h]

    if (ps->pm_type == PM_NOCLIP)
        return accelspeed;

    if (accelspeed <= (double)inertiaMax->current.value)
        return accelspeed;

    v4 = ps->oldVelocity[1] * ps->oldVelocity[1] + ps->oldVelocity[0] * ps->oldVelocity[0];
    if (v4 < 0.0001)
        return accelspeed;

    if (PM_DoPlayerInertia(ps, accelspeed, wishdir))
        return inertiaMax->current.value;

    return accelspeed;
}

char __cdecl PM_DoPlayerInertia(const playerState_s *ps, float accelspeed, const float *wishdir)
{
    float v4; // [esp+28h] [ebp-34h]
    float v5; // [esp+2Ch] [ebp-30h]
    float v6; // [esp+30h] [ebp-2Ch]
    float scaledDotAngle; // [esp+3Ch] [ebp-20h]
    float velocity; // [esp+40h] [ebp-1Ch]
    float velocity_4; // [esp+44h] [ebp-18h]
    float oldVelLenSq; // [esp+48h] [ebp-14h]
    float newVelLenSq; // [esp+4Ch] [ebp-10h]
    float oldVelocity; // [esp+54h] [ebp-8h]
    float oldVelocity_4; // [esp+58h] [ebp-4h]

    velocity = accelspeed * *wishdir + ps->velocity[0];
    velocity_4 = accelspeed * wishdir[1] + ps->velocity[1];
    oldVelocity = ps->oldVelocity[0];
    oldVelocity_4 = ps->oldVelocity[1];
    oldVelLenSq = oldVelocity_4 * oldVelocity_4 + oldVelocity * oldVelocity;
    newVelLenSq = velocity_4 * velocity_4 + velocity * velocity;
    v6 = newVelLenSq * oldVelLenSq;
    v5 = sqrt(v6);
    scaledDotAngle = velocity_4 * oldVelocity_4 + velocity * oldVelocity;
    v4 = inertiaAngle->current.value * v5;
    if (scaledDotAngle >= (double)v4)
        return 0;
    if (inertiaDebug->current.enabled)
    {
        Com_Printf(
            17,
            "angle is %f (oldVel is (%f,%f), vel is (%f, %f))\n",
            scaledDotAngle / v5,
            oldVelocity,
            oldVelocity_4,
            velocity,
            velocity_4);
        Com_Printf(17, "clamping acceleration from %f to %f\n", accelspeed, inertiaMax->current.value);
    }
    return 1;
}

double __cdecl PM_MoveScale(playerState_s *ps, float fmove, float rmove, float umove)
{
    float v5; // [esp+0h] [ebp-28h]
    float v6; // [esp+4h] [ebp-24h]
    float v7; // [esp+8h] [ebp-20h]
    float v8; // [esp+Ch] [ebp-1Ch]
    float v9; // [esp+10h] [ebp-18h]
    float v10; // [esp+14h] [ebp-14h]
    float v11; // [esp+18h] [ebp-10h]
    float max; // [esp+1Ch] [ebp-Ch]
    float scale; // [esp+24h] [ebp-4h]
    float scalea; // [esp+24h] [ebp-4h]

    v11 = I_fabs(fmove);
    max = v11;
    v10 = I_fabs(rmove);
    if (v11 < (double)v10)
    {
        v9 = I_fabs(rmove);
        max = v9;
    }
    v8 = I_fabs(umove);
    if (max < (double)v8)
    {
        v7 = I_fabs(umove);
        max = v7;
    }
    if (max == 0.0)
        return 0.0;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 827, 0, "%s", "ps");
    v6 = umove * umove + rmove * rmove + fmove * fmove;
    v5 = sqrt(v6);
    scale = (double)ps->speed * max / (v5 * 127.0);
    if ((ps->pm_flags & 0x40) == 0 && ps->leanf == 0.0)
        scalea = scale * 1.0;
    else
        scalea = scale * 0.4000000059604645;

    if (ps->pm_type == PM_NOCLIP)
        scalea = scalea * 3.0;

    if (ps->pm_type == PM_UFO)
        scalea = scalea * 6.0;

#ifdef KISAK_MP
    if (ps->pm_type == PM_SPECTATOR)
        return (float)(scalea * player_spectateSpeedScale->current.value);
#endif

    return scalea;
}

double __cdecl PM_CmdScale(playerState_s *ps, usercmd_s *cmd)
{
    float v3; // [esp+0h] [ebp-18h]
    float v4; // [esp+8h] [ebp-10h]
    int32_t max; // [esp+Ch] [ebp-Ch]
    float scale; // [esp+14h] [ebp-4h]
    float scalea; // [esp+14h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 864, 0, "%s", "ps");
    v4 = (float)(cmd->rightmove * cmd->rightmove + cmd->forwardmove * cmd->forwardmove);
    v3 = sqrt(v4);
    max = abs8(cmd->forwardmove);
    if (abs8(cmd->rightmove) > max)
        max = abs8(cmd->rightmove);
    if (!max)
        return 0.0;
    scale = (double)ps->speed * (double)max / (v3 * 127.0);
    if ((ps->pm_flags & 0x40) == 0 && ps->leanf == 0.0)
        scalea = scale * 1.0;
    else
        scalea = scale * 0.4000000059604645;
    if (ps->pm_type == PM_NOCLIP)
        scalea = scalea * 3.0;
    if (ps->pm_type == PM_UFO)
        scalea = scalea * 6.0;
#ifdef KISAK_MP
    if (ps->pm_type == PM_SPECTATOR)
        return (float)(scalea * player_spectateSpeedScale->current.value);
#endif
    return scalea;
}

void __cdecl PM_AirMove(pmove_t *pm, pml_t *pml)
{
    float fmove; // [esp+3Ch] [ebp-54h]
    float wishdir[3]; // [esp+40h] [ebp-50h] BYREF
    float wishvel[3]; // [esp+4Ch] [ebp-44h]
    float wishspeed; // [esp+58h] [ebp-38h]
    float smove; // [esp+5Ch] [ebp-34h]
    int32_t i; // [esp+60h] [ebp-30h]
    float scale; // [esp+64h] [ebp-2Ch]
    playerState_s *ps; // [esp+68h] [ebp-28h]
    usercmd_s cmd; // [esp+6Ch] [ebp-24h] BYREF

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1223, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1226, 0, "%s", "ps");
    PM_Friction(ps, pml);
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    memcpy(&cmd, &pm->cmd, sizeof(cmd));
    scale = PM_CmdScale(ps, &cmd);
    pml->forward[2] = 0.0;
    pml->right[2] = 0.0;
    Vec3Normalize(pml->forward);
    Vec3Normalize(pml->right);
    for (i = 0; i < 2; ++i)
        wishvel[i] = pml->forward[i] * fmove + pml->right[i] * smove;
    wishvel[2] = 0.0;
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = 0.0;
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    PM_Accelerate(ps, pml, wishdir, wishspeed, 1.0);
    if (pml->groundPlane)
        PM_ClipVelocity(ps->velocity, pml->groundTrace.normal, ps->velocity);
    PM_StepSlideMove(pm, pml, 1);
    PM_SetMovementDir(pm, pml);
}

void __cdecl PM_SetMovementDir(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+8h] [ebp-3Ch]
    float v3; // [esp+Ch] [ebp-38h]
    float v4; // [esp+10h] [ebp-34h]
    float v5; // [esp+14h] [ebp-30h]
    float v6; // [esp+18h] [ebp-2Ch]
    float dir[3]; // [esp+20h] [ebp-24h] BYREF
    int32_t moveyaw; // [esp+2Ch] [ebp-18h]
    float moved[3]; // [esp+30h] [ebp-14h] BYREF
    float speed; // [esp+3Ch] [ebp-8h]
    playerState_s *ps; // [esp+40h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1036, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1039, 0, "%s", "ps");
    if ((ps->pm_flags & 1) == 0 || (ps->eFlags & 0x300) != 0)
    {
        if ((ps->pm_flags & 8) != 0)
        {
            speed = vectoyaw(ps->vLadderVec) + 180.0;
            moveyaw = (int)AngleDelta(speed, ps->viewangles[1]);
            if ((int)abs32(moveyaw) > 90)
            {
                if (moveyaw <= 0)
                    moveyaw = -90;
                else
                    moveyaw = 90;
            }
            ps->movementDir = (char)moveyaw;
        }
        else
        {
            Vec3Sub(ps->origin, pml->previous_origin, moved);
            if (!pm->cmd.forwardmove && !pm->cmd.rightmove
                || ps->groundEntityNum == ENTITYNUM_NONE
                || (speed = Vec3Length(moved), speed == 0.0)
                || speed <= pml->frametime * 5.0)
            {
                ps->movementDir = 0;
            }
            else
            {
                Vec3NormalizeTo(moved, dir);
                vectoangles(dir, dir);
                moveyaw = (int)AngleDelta(dir[1], ps->viewangles[1]);
                if (pm->cmd.forwardmove < 0)
                {
                    v5 = (double)moveyaw + 180.0;
                    v6 = v5 * 0.002777777845039964;
                    v4 = v6 + 0.5;
                    v3 = floor(v4);
                    v2 = (v6 - v3) * 360.0;
                    moveyaw = (int)v2;
                }
                if ((int)abs32(moveyaw) > 90)
                {
                    if (moveyaw <= 0)
                        moveyaw = -90;
                    else
                        moveyaw = 90;
                }
                ps->movementDir = (char)moveyaw;
            }
        }
    }
    else
    {
        moveyaw = (int)AngleDelta(ps->proneDirection, ps->viewangles[1]);
        if ((int)abs32(moveyaw) > 90)
        {
            if (moveyaw <= 0)
                moveyaw = -90;
            else
                moveyaw = 90;
        }
        ps->movementDir = (char)moveyaw;
    }
}

void __cdecl PM_WalkMove(pmove_t *pm, pml_t *pml)
{
    float fmove; // [esp+20h] [ebp-58h]
    float wishdir[3]; // [esp+24h] [ebp-54h] BYREF
    float wishvel[3]; // [esp+30h] [ebp-48h] BYREF
    int32_t iStance; // [esp+3Ch] [ebp-3Ch]
    float wishspeed; // [esp+40h] [ebp-38h]
    float acceleration; // [esp+44h] [ebp-34h]
    float smove; // [esp+48h] [ebp-30h]
    float scale; // [esp+4Ch] [ebp-2Ch]
    playerState_s *ps; // [esp+50h] [ebp-28h]
    usercmd_s cmd; // [esp+54h] [ebp-24h] BYREF

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1284, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1287, 0, "%s", "ps");
    if ((ps->pm_flags & 0x4000) != 0)
        Jump_ApplySlowdown(ps);
    if ((ps->pm_flags & 0x8000) != 0)
        pm->cmd.rightmove = (int)((double)pm->cmd.rightmove * player_sprintStrafeSpeedScale->current.value);
    if (Jump_Check(pm, pml))
    {
        PM_AirMove(pm, pml);
    }
    else
    {
        PM_Friction(ps, pml);
        fmove = (float)pm->cmd.forwardmove;
        smove = (float)pm->cmd.rightmove;
        memcpy(&cmd, &pm->cmd, sizeof(cmd));
        scale = PM_CmdScale_Walk(pm, &cmd);
#ifdef KISAK_MP
        scale = PM_DamageScale_Walk(ps->damageTimer) * scale;
        ps->damageTimer -= (int)(pml->frametime * 1000.0);
        if (ps->damageTimer <= 0)
            ps->damageTimer = 0;
#endif
        pml->forward[2] = 0.0;
        pml->right[2] = 0.0;
        Vec2Normalize(pml->forward);
        Vec2Normalize(pml->right);
        wishvel[0] = fmove * pml->forward[0] + smove * pml->right[0];
        wishvel[1] = fmove * pml->forward[1] + smove * pml->right[1];
        wishvel[2] = 0.0;
        wishspeed = Vec3NormalizeTo(wishvel, wishdir);
        wishspeed = wishspeed * scale;
        PM_ProjectVelocity(wishdir, pml->groundTrace.normal, wishdir);
        iStance = PM_GetEffectiveStance(ps);
        if ((pml->groundTrace.surfaceFlags & 2) != 0 || (ps->pm_flags & 0x100) != 0)
        {
            acceleration = 1.0;
        }
        else if (iStance == 1)
        {
            acceleration = 19.0;
        }
        else if (iStance == 2)
        {
            acceleration = 12.0;
        }
        else
        {
            acceleration = 9.0;
        }
        if ((ps->pm_flags & 0x80) != 0)
            acceleration = acceleration * 0.25;
        PM_Accelerate(ps, pml, wishdir, wishspeed, acceleration);
        if ((pml->groundTrace.surfaceFlags & 2) != 0 || (ps->pm_flags & 0x100) != 0)
            ps->velocity[2] = ps->velocity[2] - (double)ps->gravity * pml->frametime;
        PM_ProjectVelocity(ps->velocity, pml->groundTrace.normal, ps->velocity);
        if (ps->velocity[0] != 0.0 || ps->velocity[1] != 0.0)
            PM_StepSlideMove(pm, pml, 0);
        PM_SetMovementDir(pm, pml);
    }
}

double __cdecl PM_CmdScale_Walk(pmove_t *pm, usercmd_s *cmd)
{
    float v3; // [esp+0h] [ebp-5Ch]
    float v4; // [esp+4h] [ebp-58h]
    float v5; // [esp+8h] [ebp-54h]
    float v6; // [esp+10h] [ebp-4Ch]
    float v7; // [esp+18h] [ebp-44h]
    float v8; // [esp+20h] [ebp-3Ch]
    bool v9; // [esp+28h] [ebp-34h]
    float v10; // [esp+2Ch] [ebp-30h]
    float forwardmove; // [esp+30h] [ebp-2Ch]
    float v12; // [esp+34h] [ebp-28h]
    float v13; // [esp+38h] [ebp-24h]
    float fmove; // [esp+40h] [ebp-1Ch]
    WeaponDef *weapon; // [esp+44h] [ebp-18h]
    float scale; // [esp+54h] [ebp-8h]
    float scalea; // [esp+54h] [ebp-8h]
    float scaleb; // [esp+54h] [ebp-8h]
    playerState_s *ps; // [esp+58h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 941, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 943, 0, "%s", "ps");
    v9 = (ps->pm_flags & 1) != 0 && ps->fWeaponPosFrac > 0.0;
    v13 = (float)(cmd->rightmove * cmd->rightmove + cmd->forwardmove * cmd->forwardmove);
    v8 = sqrt(v13);
    if (cmd->forwardmove >= 0)
    {
        forwardmove = (float)cmd->forwardmove;
        v6 = I_fabs(forwardmove);
        fmove = v6;
    }
    else
    {
        v12 = player_backSpeedScale->current.value * (double)cmd->forwardmove;
        v7 = I_fabs(v12);
        fmove = v7;
    }
    v10 = player_strafeSpeedScale->current.value * (double)cmd->rightmove;
    v5 = I_fabs(v10);
    v4 = fmove - v5;
    if (v4 < 0.0)
        v3 = v5;
    else
        v3 = fmove;
    if (v3 == 0.0)
        return 0.0;
    scale = (double)ps->speed * v3 / (v8 * 127.0);
    if ((ps->pm_flags & 0x40) != 0 || ps->leanf != 0.0 || v9)
        scalea = scale * 0.4000000059604645;
    else
        scalea = scale * 1.0;
    if ((ps->pm_flags & 0x8000) != 0)
        scalea = scalea * player_sprintSpeedScale->current.value;
    if (ps->pm_type == PM_NOCLIP)
    {
        scaleb = scalea * 3.0;
    }
    else if (ps->pm_type == PM_UFO)
    {
        scaleb = scalea * 6.0;
    }
    else
    {
        scaleb = PM_CmdScaleForStance(pm) * scalea;
    }
    weapon = BG_GetWeaponDef(ps->weapon);
    if (!ps->weapon || weapon->moveSpeedScale <= 0.0 || (ps->pm_flags & 0x40) != 0 || v9)
    {
        if (ps->weapon && weapon->adsMoveSpeedScale > 0.0)
            scaleb = scaleb * weapon->adsMoveSpeedScale;
    }
    else
    {
        scaleb = scaleb * weapon->moveSpeedScale;
    }
    if ((ps->pm_flags & 0x10000) != 0 && BG_GetShellshockParms(ps->shellshockIndex)->movement.affect)
        scaleb = scaleb * 0.40000001;
    return (float)(scaleb * ps->moveSpeedScaleMultiplier);
}

double __cdecl PM_CmdScaleForStance(const pmove_t *pm)
{
    int32_t stance; // [esp+8h] [ebp-8h]
    float lerpFrac; // [esp+Ch] [ebp-4h]
    float lerpFraca; // [esp+Ch] [ebp-4h]

    lerpFrac = PM_GetViewHeightLerp(pm, 40, 11);
    if (lerpFrac == 0.0)
    {
        lerpFraca = PM_GetViewHeightLerp(pm, 11, 40);
        if (lerpFraca == 0.0)
        {
            if (!pm->ps)
                MyAssertHandler(".\\bgame\\bg_pmove.cpp", 908, 0, "%s", "pm->ps");
            stance = PM_GetEffectiveStance(pm->ps);
            if (stance == 1)
            {
                return 0.15000001;
            }
            else if (stance == 2)
            {
                return 0.64999998;
            }
            else
            {
                return 1.0;
            }
        }
        else
        {
            return (float)(lerpFraca * 0.6499999761581421 + (1.0 - lerpFraca) * 0.1500000059604645);
        }
    }
    else
    {
        return (float)(lerpFrac * 0.1500000059604645 + (1.0 - lerpFrac) * 0.6499999761581421);
    }
}

void __cdecl PM_DeadMove(playerState_s *ps, pml_t *pml)
{
    float forwarda; // [esp+1Ch] [ebp-4h]
    float forward; // [esp+1Ch] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1379, 0, "%s", "ps");
    if (pml->walking)
    {
        forwarda = Vec3Length(ps->velocity);
        forward = forwarda - 20.0;
        if (forward > 0.0)
        {
            Vec3Normalize(ps->velocity);
            Vec3Scale(ps->velocity, forward, ps->velocity);
        }
        else
        {
            ps->velocity[0] = 0.0;
            ps->velocity[1] = 0.0;
            ps->velocity[2] = 0.0;
        }
    }
}

void __cdecl PM_NoclipMove(pmove_t *pm, pml_t *pml)
{
    float value; // [esp+14h] [ebp-60h]
    float *velocity; // [esp+24h] [ebp-50h]
    float fmove; // [esp+2Ch] [ebp-48h]
    float wishdir[4]; // [esp+30h] [ebp-44h] BYREF
    float wishvel[3]; // [esp+40h] [ebp-34h]
    float wishspeed; // [esp+4Ch] [ebp-28h]
    float curFriction; // [esp+50h] [ebp-24h]
    float newspeed; // [esp+54h] [ebp-20h]
    float drop; // [esp+58h] [ebp-1Ch]
    float speed; // [esp+5Ch] [ebp-18h]
    float smove; // [esp+60h] [ebp-14h]
    int32_t i; // [esp+64h] [ebp-10h]
    float scale; // [esp+68h] [ebp-Ch]
    playerState_s *ps; // [esp+6Ch] [ebp-8h]
    float umove; // [esp+70h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1422, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1425, 0, "%s", "ps");
    ps->viewHeightTarget = 60;
    speed = Vec3Length(ps->velocity);
    if (speed >= 1.0)
    {
        drop = 0.0;
        curFriction = friction->current.value * 1.5;
        if (stopspeed->current.value <= (double)speed)
            value = speed;
        else
            value = stopspeed->current.value;
        wishdir[3] = value;
        drop = value * curFriction * pml->frametime + drop;
        newspeed = speed - drop;
        if (newspeed < 0.0)
            newspeed = 0.0;
        newspeed = newspeed / speed;
        Vec3Scale(ps->velocity, newspeed, ps->velocity);
    }
    else
    {
        velocity = ps->velocity;
        ps->velocity[0] = 0.0;
        velocity[1] = 0.0;
        velocity[2] = 0.0;
    }
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    umove = 0.0;
    if ((pm->cmd.buttons & 0x80) != 0)
        umove = umove + 127.0;
    if ((pm->cmd.buttons & 0x40) != 0)
        umove = umove - 127.0;
    scale = PM_MoveScale(ps, fmove, smove, umove);
    for (i = 0; i < 3; ++i)
        wishvel[i] = pml->forward[i] * fmove + pml->right[i] * smove + pml->up[i] * umove;
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    PM_Accelerate(ps, pml, wishdir, wishspeed, 9.0);
    Vec3Mad(ps->origin, pml->frametime, ps->velocity, ps->origin);
}

void __cdecl PM_UFOMove(pmove_t *pm, pml_t *pml)
{
    float value; // [esp+Ch] [ebp-80h]
    float *velocity; // [esp+24h] [ebp-68h]
    float forward[3]; // [esp+2Ch] [ebp-60h] BYREF
    float up[3]; // [esp+38h] [ebp-54h] BYREF
    int32_t i; // [esp+44h] [ebp-48h]
    float fmove; // [esp+48h] [ebp-44h]
    float wishdir[4]; // [esp+4Ch] [ebp-40h] BYREF
    float wishvel[3]; // [esp+5Ch] [ebp-30h]
    float wishspeed; // [esp+68h] [ebp-24h]
    float curFriction; // [esp+6Ch] [ebp-20h]
    float newspeed; // [esp+70h] [ebp-1Ch]
    float drop; // [esp+74h] [ebp-18h]
    float speed; // [esp+78h] [ebp-14h]
    float smove; // [esp+7Ch] [ebp-10h]
    float scale; // [esp+80h] [ebp-Ch]
    playerState_s *ps; // [esp+84h] [ebp-8h]
    float umove; // [esp+88h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1500, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1503, 0, "%s", "ps");
    ps->viewHeightTarget = 60;
    fmove = (float)pm->cmd.forwardmove;
    smove = (float)pm->cmd.rightmove;
    umove = 0.0;
    if ((pm->cmd.buttons & 0x80) != 0)
        umove = umove + 127.0;
    if ((pm->cmd.buttons & 0x40) != 0)
        umove = umove - 127.0;
    if (fmove == 0.0 && smove == 0.0 && umove == 0.0)
        speed = 0.0;
    else
        speed = Vec3Length(ps->velocity);
    if (speed >= 1.0)
    {
        drop = 0.0;
        curFriction = friction->current.value * 1.5;
        if (stopspeed->current.value <= (double)speed)
            value = speed;
        else
            value = stopspeed->current.value;
        wishdir[3] = value;
        drop = value * curFriction * pml->frametime + drop;
        newspeed = speed - drop;
        if (newspeed < 0.0)
            newspeed = 0.0;
        newspeed = newspeed / speed;
        Vec3Scale(ps->velocity, newspeed, ps->velocity);
    }
    else
    {
        velocity = ps->velocity;
        ps->velocity[0] = 0.0;
        velocity[1] = 0.0;
        velocity[2] = 0.0;
    }
    scale = PM_MoveScale(ps, fmove, smove, umove);
    up[1] = 0.0;
    up[0] = 0.0;
    up[2] = 1.0;
    Vec3Cross(up, pml->right, forward);
    for (i = 0; i < 3; ++i)
        wishvel[i] = forward[i] * fmove + pml->right[i] * smove + up[i] * umove;
    wishdir[0] = wishvel[0];
    wishdir[1] = wishvel[1];
    wishdir[2] = wishvel[2];
    wishspeed = Vec3Normalize(wishdir);
    wishspeed = wishspeed * scale;
    PM_Accelerate(ps, pml, wishdir, wishspeed, 9.0);
    Vec3Mad(ps->origin, pml->frametime, ps->velocity, ps->origin);
}

void __cdecl PM_GroundTrace(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    uint16_t EntityHitId; // ax
    float start[3]; // [esp+8h] [ebp-48h] BYREF
    trace_t trace; // [esp+14h] [ebp-3Ch] BYREF
    playerState_s *ps; // [esp+40h] [ebp-10h]
    float point[3]; // [esp+44h] [ebp-Ch] BYREF

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1989, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1992, 0, "%s", "ps");
    start[0] = ps->origin[0];
    start[1] = ps->origin[1];
    point[0] = ps->origin[0];
    point[1] = ps->origin[1];
    if ((ps->eFlags & 0x300) != 0)
    {
        start[2] = ps->origin[2];
        v2 = ps->origin[2] - 1.0;
    }
    else
    {
        start[2] = ps->origin[2] + 0.25;
        v2 = ps->origin[2] - 0.25;
    }
    point[2] = v2;
    PM_playerTrace(pm, &trace, start, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
    memcpy(&pml->groundTrace, &trace, sizeof(pml->groundTrace));
    if (!trace.allsolid || PM_CorrectAllSolid(pm, pml, &trace))
    {
        if (trace.startsolid)
        {
            start[2] = ps->origin[2] - EQUAL_EPSILON;
            PM_playerTrace(pm, &trace, start, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
            if (trace.startsolid)
            {
                ps->groundEntityNum = ENTITYNUM_NONE;
                pml->groundPlane = 0;
                pml->almostGroundPlane = 0;
                pml->walking = 0;
                return;
            }
            memcpy(&pml->groundTrace, &trace, sizeof(pml->groundTrace));
        }
        if (trace.fraction == 1.0)
        {
            PM_GroundTraceMissed(pm, pml);
        }
        else
        {
            if (trace.normal[0] == 0.0 && trace.normal[1] == 0.0 && trace.normal[2] == 0.0)
                MyAssertHandler(
                    ".\\bgame\\bg_pmove.cpp",
                    2047,
                    0,
                    "%s",
                    "trace.normal[0] || trace.normal[1] || trace.normal[2]");
            if ((ps->pm_flags & 8) != 0 || ps->velocity[2] <= 0.0 || Vec3Dot(ps->velocity, trace.normal) <= 10.0)
            {
                if (trace.walkable)
                {
                    pml->groundPlane = 1;
                    pml->almostGroundPlane = 1;
                    pml->walking = 1;
                    if (ps->groundEntityNum == ENTITYNUM_NONE)
                        PM_CrashLand(ps, pml);
                    EntityHitId = Trace_GetEntityHitId(&trace);
                    ps->groundEntityNum = EntityHitId;
                    PM_AddTouchEnt(pm, ps->groundEntityNum);
                }
                else
                {
                    ps->groundEntityNum = ENTITYNUM_NONE;
                    pml->groundPlane = 1;
                    pml->almostGroundPlane = 1;
                    pml->walking = 0;
                    Jump_ClearState(ps);
                }
            }
            else
            {
                if (pm->cmd.forwardmove < 0)
                    BG_AnimScriptEvent(ps, ANIM_ET_JUMPBK, 0, 0);
                else
                    BG_AnimScriptEvent(ps, ANIM_ET_JUMP, 0, 0);
                pml->almostGroundPlane = 0;
                ps->groundEntityNum = ENTITYNUM_NONE;
                pml->groundPlane = 0;
                pml->walking = 0;
            }
        }
    }
}

void __cdecl PM_CrashLand(playerState_s *ps, pml_t *pml)
{
    uint32_t v2; // eax
    uint32_t v3; // eax
    uint32_t v4; // eax
    uint32_t v5; // eax
    int32_t v6; // [esp+8h] [ebp-50h]
    float v7; // [esp+Ch] [ebp-4Ch]
    int32_t v8; // [esp+18h] [ebp-40h]
    int32_t damage; // [esp+1Ch] [ebp-3Ch]
    float fallHeight; // [esp+20h] [ebp-38h]
    float t; // [esp+24h] [ebp-34h]
    float vel; // [esp+28h] [ebp-30h]
    float dist; // [esp+30h] [ebp-28h]
    float landVel; // [esp+34h] [ebp-24h]
    int32_t viewDip; // [esp+38h] [ebp-20h]
    int32_t stunTime; // [esp+3Ch] [ebp-1Ch]
    float acc; // [esp+44h] [ebp-14h]
    int32_t surfaceType; // [esp+48h] [ebp-10h]
    float a; // [esp+4Ch] [ebp-Ch]
    float den; // [esp+50h] [ebp-8h]
    float fSpeedMult; // [esp+54h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1723, 0, "%s", "ps");
    dist = pml->previous_origin[2] - ps->origin[2];
    vel = pml->previous_velocity[2];
    acc = -(float)ps->gravity;
    a = acc * 0.5;
    den = vel * vel - a * 4.0 * dist;
    if (den >= 0.0)
    {
        v7 = sqrt(den);
        t = (-vel - v7) / (a * 2.0);
        landVel = (t * acc + vel) * -1.0;
        fallHeight = landVel * landVel / ((float)ps->gravity * 2.0);
        if (bg_fallDamageMinHeight->current.value <= 0.0)
            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1743, 0, "%s", "bg_fallDamageMinHeight->current.value > 0");
        if (bg_fallDamageMinHeight->current.value < (float)bg_fallDamageMaxHeight->current.value)
        {
            if (bg_fallDamageMinHeight->current.value >= (float)fallHeight
                || (pml->groundTrace.surfaceFlags & 1) != 0
                || ps->pm_type >= PM_DEAD)
            {
                damage = 0;
            }
            else if (bg_fallDamageMaxHeight->current.value > (float)fallHeight)
            {
                if ((int)((fallHeight - bg_fallDamageMinHeight->current.value)
                    / (bg_fallDamageMaxHeight->current.value - bg_fallDamageMinHeight->current.value)
                    * 100.0f) < 100)
                    v8 = (int)((fallHeight - bg_fallDamageMinHeight->current.value)
                        / (bg_fallDamageMaxHeight->current.value - bg_fallDamageMinHeight->current.value)
                        * 100.0);
                else
                    v8 = 100;
                if (v8 > 0)
                    v6 = v8;
                else
                    v6 = 0;
                damage = v6;
            }
            else
            {
                damage = 100;
            }
        }
        else
        {
            Com_Printf(17, "bg_fallDamageMaxHeight must be greater than bg_fallDamageMinHeight\n");
            damage = 0;
        }
        if (fallHeight > 12.0f)
        {
            viewDip = (int)((fallHeight - 12.0f) / 26.0f * 4.0f + 4.0f);
            if (viewDip > 24)
                viewDip = 24;
            BG_AnimScriptEvent(ps, ANIM_ET_LAND, 0, 1);
        }
        else
        {
            viewDip = 0;
        }
        surfaceType = PM_GroundSurfaceType(pml);
        if (damage)
        {
            if (damage >= 100 || (pml->groundTrace.surfaceFlags & 2) != 0)
            {
                Vec3Scale(ps->velocity, 0.67000002f, ps->velocity);
            }
            else
            {
                stunTime = 35 * damage + 500;
                if (stunTime > 2000)
                    stunTime = 2000;
                if (stunTime > 500)
                {
                    if (stunTime < 1500)
                        fSpeedMult = 0.5f - ((float)stunTime - 500.0f) / 1000.0f * 0.300000011920929f;
                    else
                        fSpeedMult = 0.2f;
                }
                else
                {
                    fSpeedMult = 0.5f;
                }
                ps->pm_time = stunTime;
                ps->pm_flags |= 0x80u;
                Vec3Scale(ps->velocity, fSpeedMult, ps->velocity);
            }
            v2 = PM_DamageLandingForSurface(pml);
            BG_AddPredictableEventToPlayerstate(v2, damage, ps);
        }
        else if (fallHeight > 4.0f)
        {
            if (fallHeight >= 8.0f)
            {
                if (fallHeight >= 12.0f)
                {
                    Vec3Scale(ps->velocity, 0.67000002f, ps->velocity);
                    v5 = PM_HardLandingForSurface(pml);
                    BG_AddPredictableEventToPlayerstate(v5, viewDip, ps);
                }
                else
                {
                    v4 = PM_MediumLandingForSurface(pml);
                    BG_AddPredictableEventToPlayerstate(v4, surfaceType, ps);
                }
            }
            else
            {
                v3 = PM_LightLandingForSurface(pml);
                BG_AddPredictableEventToPlayerstate(v3, surfaceType, ps);
            }
        }
    }
}

int32_t __cdecl PM_LightLandingForSurface(pml_t *pml)
{
    if (PM_GroundSurfaceType(pml))
        return 74;
    else
        return 0;
}

int32_t __cdecl PM_MediumLandingForSurface(pml_t *pml)
{
    if (PM_GroundSurfaceType(pml))
        return 73;
    else
        return 0;
}

uint32_t __cdecl PM_HardLandingForSurface(pml_t *pml)
{
    uint32_t iSurfType; // [esp+0h] [ebp-4h]

    iSurfType = PM_GroundSurfaceType(pml);
    if (iSurfType)
        return iSurfType + 77;
    else
        return 0;
}

uint32_t __cdecl PM_DamageLandingForSurface(pml_t *pml)
{
    uint32_t iSurfType; // [esp+0h] [ebp-4h]

    iSurfType = PM_GroundSurfaceType(pml);
    if (iSurfType)
        return iSurfType + 106;
    else
        return 0;
}

int32_t __cdecl PM_CorrectAllSolid(pmove_t *pm, pml_t *pml, trace_t *trace)
{
    uint32_t i; // [esp+14h] [ebp-14h]
    playerState_s *ps; // [esp+18h] [ebp-10h]
    float point[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1875, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1878, 0, "%s", "ps");
    for (i = 0; i < 0x1A; ++i)
    {
        Vec3Add(ps->origin, CorrectSolidDeltas[i], point);
        PM_playerTrace(pm, trace, point, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
        if (!trace->startsolid)
        {
            ps->origin[0] = point[0];
            ps->origin[1] = point[1];
            ps->origin[2] = point[2];
            point[2] = ps->origin[2] - 1.0 - 0.25;
            PM_playerTrace(pm, trace, ps->origin, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
            memcpy(&pml->groundTrace, trace, sizeof(pml->groundTrace));
            Vec3Lerp(ps->origin, point, trace->fraction, ps->origin);
            return 1;
        }
    }
    ps->groundEntityNum = ENTITYNUM_NONE;
    pml->groundPlane = 0;
    pml->almostGroundPlane = 0;
    pml->walking = 0;
    Jump_ClearState(ps);
    return 0;
}

void __cdecl PM_GroundTraceMissed(pmove_t *pm, pml_t *pml)
{
    trace_t trace; // [esp+10h] [ebp-3Ch] BYREF
    playerState_s *ps; // [esp+3Ch] [ebp-10h]
    float point[3]; // [esp+40h] [ebp-Ch] BYREF

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1931, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 1934, 0, "%s", "ps");
    if (ps->groundEntityNum == ENTITYNUM_NONE)
    {
        point[0] = ps->origin[0];
        point[1] = ps->origin[1];
        point[2] = ps->origin[2];
        point[2] = point[2] - 1.0;
        pmoveHandlers[pm->handler].trace(&trace, ps->origin, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
        pml->almostGroundPlane = trace.fraction != 1.0;
    }
    else
    {
        point[0] = ps->origin[0];
        point[1] = ps->origin[1];
        point[2] = ps->origin[2];
        point[2] = point[2] - 64.0;
        pmoveHandlers[pm->handler].trace(&trace, ps->origin, pm->mins, pm->maxs, point, ps->clientNum, pm->tracemask);
        if (trace.fraction == 1.0)
        {
            if (pm->cmd.forwardmove < 0)
                BG_AnimScriptEvent(ps, ANIM_ET_JUMPBK, 0, 1);
            else
                BG_AnimScriptEvent(ps, ANIM_ET_JUMP, 0, 1);
            pml->almostGroundPlane = 0;
        }
        else
        {
            pml->almostGroundPlane = trace.fraction < 0.015625;
        }
    }
    ps->groundEntityNum = ENTITYNUM_NONE;
    pml->groundPlane = 0;
    pml->walking = 0;
}

double __cdecl PM_GetViewHeightLerp(const pmove_t *pm, int32_t iFromHeight, int32_t iToHeight)
{
    float fLerpFrac; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps->viewHeightLerpTime)
        return 0.0;
    if (iFromHeight != -1
        && iToHeight != -1
        && (iToHeight != ps->viewHeightLerpTarget
            || iToHeight == 40
            && (iFromHeight != 11 || ps->viewHeightLerpDown)
            && (iFromHeight != 60 || !ps->viewHeightLerpDown)))
    {
        return 0.0;
    }
    fLerpFrac = (double)(pm->cmd.serverTime - ps->viewHeightLerpTime)
        / (double)PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
    if (fLerpFrac >= 0.0)
    {
        if (fLerpFrac > 1.0)
            return (float)1.0;
    }
    else
    {
        return (float)0.0;
    }
    return fLerpFrac;
}

bool __cdecl PM_IsPlayerFrozenByWeapon(const playerState_s *ps)
{
    return ps->weaponstate == 5 && ps->weapon && BG_GetWeaponDef(ps->weapon)->freezeMovementWhenFiring != 0;
}

void __cdecl PM_CheckDuck(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    float delta; // [esp+18h] [ebp-58h]
    float vPoint[3]; // [esp+1Ch] [ebp-54h] BYREF
    float vEnd[3]; // [esp+28h] [ebp-48h] BYREF
    int32_t iStance; // [esp+34h] [ebp-3Ch]
    int32_t bWasProne; // [esp+38h] [ebp-38h]
    int32_t bWasStanding; // [esp+3Ch] [ebp-34h]
    trace_t trace; // [esp+40h] [ebp-30h] BYREF
    playerState_s *ps; // [esp+6Ch] [ebp-4h]

    iassert(pm);
    ps = pm->ps;
    iassert(ps);

#ifdef KISAK_MP
    pm->proneChange = 0;
#endif

#ifdef KISAK_MP
    if (ps->pm_type == PM_SPECTATOR)
#elif KISAK_SP
    if (ps->pm_type == PM_DEAD)
#endif
    {
        pm->mins[0] = -8.0;
        pm->mins[1] = -8.0;
        pm->mins[2] = -8.0;
        pm->maxs[0] = 8.0;
        pm->maxs[1] = 8.0;
        pm->maxs[2] = 16.0;
        ps->pm_flags &= 0xFFFFFFFC;
        if ((pm->cmd.buttons & 0x100) != 0)
        {
            pm->cmd.buttons &= ~0x100u;
            BG_AddPredictableEventToPlayerstate(6, 0, ps);
        }
        ps->viewHeightTarget = 0;
        ps->viewHeightCurrent = 0.0;
    }
    else
    {
        bWasProne = (ps->pm_flags & 1) != 0;
        bWasStanding = (ps->pm_flags & 3) == 0;
        pm->mins[0] = -15.0;
        pm->mins[1] = -15.0;
        pm->mins[2] = 0.0;
        pm->maxs[0] = 15.0;
        pm->maxs[1] = 15.0;
        pm->maxs[2] = 70.0;
        if (ps->pm_type == PM_DEAD)
        {
            ps->viewHeightTarget = 8;
            PM_ViewHeightAdjust(pm, pml);
        }
        else if ((ps->pm_flags & 0x100000) != 0)
        {
            ps->viewHeightTarget = 60;
            ps->pm_flags &= 0xFFFFFFFC;
            BG_AddPredictableEventToPlayerstate(6u, 0, ps);
            PM_ViewHeightAdjust(pm, pml);
        }
        else if ((ps->pm_flags & 0x8000) != 0)
        {
            ps->viewHeightTarget = 60;
            ps->eFlags &= 0xFFFFFFF3;
            ps->pm_flags &= 0xFFFFFFFC;
            BG_AddPredictableEventToPlayerstate(6u, 0, ps);
            PM_ViewHeightAdjust(pm, pml);
        }
        else
        {
            if ((ps->eFlags & 0x300) != 0)
            {
                if ((ps->eFlags & 0x100) == 0 || (ps->eFlags & 0x200) != 0)
                {
                    if ((ps->eFlags & 0x200) == 0 || (ps->eFlags & 0x100) != 0)
                    {
                        ps->pm_flags &= 0xFFFFFFFC;
                    }
                    else
                    {
                        ps->pm_flags |= 2u;
                        ps->pm_flags &= ~1u;
                    }
                }
                else
                {
                    ps->pm_flags |= 1u;
                    ps->pm_flags &= ~2u;
                }
            }
            else if ((ps->pm_flags & 0xC00) == 0 && !PM_IsPlayerFrozenByWeapon(ps))
            {
#ifdef KISAK_MP
                if (ps->pm_type == PM_LASTSTAND)
                {
                    ps->pm_flags &= ~1u;
                    ps->pm_flags |= 2u;
                }
                else
#endif
                {
                    if ((ps->pm_flags & 8) != 0 && (pm->cmd.buttons & 0x300) != 0)
                    {
                        pm->cmd.buttons &= 0xFFFFFCFF;
                        BG_AddPredictableEventToPlayerstate(6u, 0, ps);
                    }
                    if ((pm->cmd.buttons & 0x100) == 0 || (ps->pm_flags & 0x400) != 0)
                    {
                        if ((pm->cmd.buttons & 0x200) != 0)
                        {
                            if ((ps->pm_flags & 1) != 0)
                            {
                                pm->maxs[2] = 50.0;
                                pmoveHandlers[pm->handler].trace(
                                    &trace,
                                    ps->origin,
                                    pm->mins,
                                    pm->maxs,
                                    ps->origin,
                                    ps->clientNum,
                                    pm->tracemask & 0xFDFFFFFF);
                                if (trace.allsolid)
                                {
                                    if ((pm->cmd.buttons & 0x1000) == 0)
                                        BG_AddPredictableEventToPlayerstate(8u, 2u, ps);
                                }
                                else
                                {
                                    BG_AnimScriptEvent(ps, ANIM_ET_PRONE_TO_CROUCH, 0, 0);
                                    ps->pm_flags &= ~1u;
                                    ps->pm_flags |= 2u;
                                }
                            }
                            else
                            {
                                BG_AnimScriptEvent(ps, ANIM_ET_STAND_TO_CROUCH, 0, 0);
                                ps->pm_flags |= 2u;
                            }
                        }
                        else if ((ps->pm_flags & 1) != 0)
                        {
                            pmoveHandlers[pm->handler].trace(
                                &trace,
                                ps->origin,
                                pm->mins,
                                pm->maxs,
                                ps->origin,
                                ps->clientNum,
                                pm->tracemask & 0xFDFFFFFF);
                            if (trace.allsolid)
                            {
                                pm->maxs[2] = 50.0;
                                pmoveHandlers[pm->handler].trace(
                                    &trace,
                                    ps->origin,
                                    pm->mins,
                                    pm->maxs,
                                    ps->origin,
                                    ps->clientNum,
                                    pm->tracemask & 0xFDFFFFFF);
                                if (trace.allsolid)
                                {
                                    if ((pm->cmd.buttons & 0x1000) == 0)
                                        BG_AddPredictableEventToPlayerstate(8u, 1u, ps);
                                }
                                else
                                {
                                    ps->pm_flags &= ~1u;
                                    ps->pm_flags |= 2u;
                                }
                            }
                            else
                            {
                                BG_AnimScriptEvent(ps, ANIM_ET_PRONE_TO_STAND, 0, 0);
                                ps->pm_flags &= 0xFFFFFFFC;
                            }
                        }
                        else if ((ps->pm_flags & 2) != 0)
                        {
                            pmoveHandlers[pm->handler].trace(
                                &trace,
                                ps->origin,
                                pm->mins,
                                pm->maxs,
                                ps->origin,
                                ps->clientNum,
                                pm->tracemask & 0xFDFFFFFF);
                            if (trace.allsolid)
                            {
                                if ((pm->cmd.buttons & 0x1000) == 0)
                                    BG_AddPredictableEventToPlayerstate(7u, 1u, ps);
                            }
                            else
                            {
                                BG_AnimScriptEvent(ps, ANIM_ET_CROUCH_TO_STAND, 0, 0);
                                ps->pm_flags &= ~2u;
                            }
                        }
                    }
                    else if (PlayerProneAllowed(pm))
                    {
                        ps->pm_flags |= 1u;
                        ps->pm_flags &= ~2u;
                    }
                    else if (ps->groundEntityNum != ENTITYNUM_NONE)
                    {
                        ps->pm_flags |= 0x1000u;
                        if ((pm->cmd.buttons & 0x1000) == 0)
                        {
                            if ((ps->pm_flags & 1) != 0 || (ps->pm_flags & 2) != 0)
                                BG_AddPredictableEventToPlayerstate(7u, 0, ps);
                            else
                                BG_AddPredictableEventToPlayerstate(6u, 0, ps);
                        }
                    }
                }
            }
            if (!ps->viewHeightLerpTime)
            {
#ifdef KISAK_MP
                if (ps->pm_type == PM_LASTSTAND)
                {
                    ps->viewHeightTarget = 22;
                }
                else if ((ps->pm_flags & 1) != 0)
#elif KISAK_SP
                if ((ps->pm_flags & 1) != 0)
#endif
                {
                    if (ps->viewHeightTarget == 60)
                    {
                        ps->viewHeightTarget = 40;
                    }
                    else if (ps->viewHeightTarget != 11)
                    {
                        ps->viewHeightTarget = 11;
#ifdef KISAK_MP
                        pm->proneChange = 1;
#endif
                        BG_PlayAnim(ps, 0, ANIM_BP_TORSO, 0, 0, 1, 1);
                        Jump_ActivateSlowdown(ps);
                    }
                }
                else if (ps->viewHeightTarget == 11)
                {
                    ps->viewHeightTarget = 40;
#ifdef KISAK_MP
                    pm->proneChange = 1;
#endif
                    BG_PlayAnim(ps, 0, ANIM_BP_TORSO, 0, 0, 1, 1);
                }
                else if ((ps->pm_flags & 2) != 0)
                {
                    ps->viewHeightTarget = 40;
                }
                else
                {
                    ps->viewHeightTarget = 60;
                }
            }
            PM_ViewHeightAdjust(pm, pml);
            iStance = PM_GetEffectiveStance(ps);
            if (iStance == 1)
            {
                pm->maxs[2] = 30.0;
                ps->eFlags |= 8u;
                ps->eFlags &= ~4u;
                ps->pm_flags |= 1u;
                ps->pm_flags &= ~2u;
            }
            else if (iStance == 2)
            {
                pm->maxs[2] = 50.0;
                ps->eFlags |= 4u;
                ps->eFlags &= ~8u;
                ps->pm_flags |= 2u;
                ps->pm_flags &= ~1u;
            }
            else
            {
                pm->maxs[2] = 70.0;
                ps->eFlags &= 0xFFFFFFF3;
                ps->pm_flags &= 0xFFFFFFFC;
            }
            if ((ps->pm_flags & 1) != 0 && !bWasProne)
            {
                if (pm->cmd.forwardmove || pm->cmd.rightmove)
                {
                    ps->pm_flags &= ~0x200u;
                    PM_ExitAimDownSight(ps);
                }
                vEnd[0] = ps->origin[0];
                vEnd[1] = ps->origin[1];
                vEnd[2] = ps->origin[2];
                vEnd[2] = vEnd[2] + 10.0;
                pmoveHandlers[pm->handler].trace(
                    &trace,
                    ps->origin,
                    pm->mins,
                    pm->maxs,
                    vEnd,
                    ps->clientNum,
                    pm->tracemask & 0xFDFFFFFF);
                Vec3Lerp(ps->origin, vEnd, trace.fraction, vEnd);
                pmoveHandlers[pm->handler].trace(
                    &trace,
                    vEnd,
                    pm->mins,
                    pm->maxs,
                    ps->origin,
                    ps->clientNum,
                    pm->tracemask & 0xFDFFFFFF);
                Vec3Lerp(vEnd, ps->origin, trace.fraction, ps->origin);
                ps->proneDirection = ps->viewangles[1];
                vPoint[0] = ps->origin[0];
                vPoint[1] = ps->origin[1];
                vPoint[2] = ps->origin[2];
                vPoint[2] = vPoint[2] - 0.25;
                pmoveHandlers[pm->handler].trace(
                    &trace,
                    ps->origin,
                    pm->mins,
                    pm->maxs,
                    vPoint,
                    ps->clientNum,
                    pm->tracemask & 0xFDFFFFFF);
                if (trace.startsolid || trace.fraction >= 1.0)
                {
                    ps->proneDirectionPitch = 0.0;
                }
                else
                {
                    if (trace.normal[0] == 0.0 && trace.normal[1] == 0.0 && trace.normal[2] == 0.0)
                        MyAssertHandler(
                            ".\\bgame\\bg_pmove.cpp",
                            2960,
                            0,
                            "%s",
                            "trace.normal[0] || trace.normal[1] || trace.normal[2]");
                    v2 = PitchForYawOnNormal(ps->proneDirection, trace.normal);
                    ps->proneDirectionPitch = v2;
                }
                delta = AngleDelta(ps->proneDirectionPitch, ps->viewangles[0]);
                if (delta >= -45.0)
                {
                    if (delta <= 45.0)
                        ps->proneTorsoPitch = ps->proneDirectionPitch;
                    else
                        ps->proneTorsoPitch = ps->viewangles[0] + 45.0;
                }
                else
                {
                    ps->proneTorsoPitch = ps->viewangles[0] - 45.0;
                }
            }
        }
    }
}

void __cdecl PM_ViewHeightAdjust(pmove_t *pm, pml_t *pml)
{
    double v2; // st7
    double v3; // st7
    double v4; // st7
    const char *v5; // eax
    float fNewPosOfs; // [esp+0h] [ebp-10h] BYREF
    int32_t iLerpFrac; // [esp+4h] [ebp-Ch]
    int32_t iLerpTime; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2249, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2252, 0, "%s", "ps");
    if (ps->viewHeightTarget && ps->viewHeightCurrent != 0.0)
    {
        if (ps->viewHeightCurrent != (double)ps->viewHeightTarget || ps->viewHeightLerpTime)
        {
            iLerpFrac = 0;
            if (ps->viewHeightTarget == 11 || ps->viewHeightTarget == 40 || ps->viewHeightTarget == 60)
            {
                if (ps->viewHeightLerpTime)
                {
                    iLerpTime = PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
                    iLerpFrac = 100 * (pm->cmd.serverTime - ps->viewHeightLerpTime) / iLerpTime;
                    if (iLerpFrac >= 0)
                    {
                        if (iLerpFrac > 100)
                            iLerpFrac = 100;
                    }
                    else
                    {
                        iLerpFrac = 0;
                    }
                    if (iLerpFrac == 100)
                    {
                        ps->viewHeightCurrent = (float)ps->viewHeightLerpTarget;
                        ps->viewHeightLerpTime = 0;
                    }
                    else
                    {
                        if (ps->viewHeightLerpTarget == 11)
                        {
                            v2 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchProne, &fNewPosOfs);
                            ps->viewHeightCurrent = v2;
                        }
                        else if (ps->viewHeightLerpTarget == 40)
                        {
                            if (ps->viewHeightLerpDown)
                                v3 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_StandCrouch, &fNewPosOfs);
                            else
                                v3 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_ProneCrouch, &fNewPosOfs);
                            ps->viewHeightCurrent = v3;
                        }
                        else
                        {
                            v4 = PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchStand, &fNewPosOfs);
                            ps->viewHeightCurrent = v4;
                        }
                        if (fNewPosOfs != 0.0 && !alwaysfails)
                            MyAssertHandler(
                                ".\\bgame\\bg_pmove.cpp",
                                2329,
                                0,
                                "Wow, someone added view height offsets back in!  You need to restore this code and the viewHeightLerpPosAdj field");
                    }
                }
                if (ps->viewHeightLerpTime)
                {
                    if (ps->viewHeightTarget != ps->viewHeightLerpTarget
                        && (ps->viewHeightTarget < ps->viewHeightLerpTarget && !ps->viewHeightLerpDown
                            || ps->viewHeightTarget > ps->viewHeightLerpTarget && ps->viewHeightLerpDown))
                    {
                        iLerpFrac = 100 - iLerpFrac;
                        ps->viewHeightLerpDown ^= 1u;
                        if (ps->viewHeightLerpDown)
                        {
                            if (ps->viewHeightLerpTarget == 60)
                            {
                                ps->viewHeightLerpTarget = 40;
                            }
                            else if (ps->viewHeightLerpTarget == 40)
                            {
                                ps->viewHeightLerpTarget = 11;
                            }
                        }
                        else if (ps->viewHeightLerpTarget == 11)
                        {
                            ps->viewHeightLerpTarget = 40;
                        }
                        else if (ps->viewHeightLerpTarget == 40)
                        {
                            ps->viewHeightLerpTarget = 60;
                        }
                        if (iLerpFrac == 100)
                        {
                            ps->viewHeightCurrent = (float)ps->viewHeightLerpTarget;
                            ps->viewHeightLerpTime = 0;
                        }
                        else
                        {
                            iLerpTime = PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
                            ps->viewHeightLerpTime = pm->cmd.serverTime
                                - (int)((double)iLerpFrac * 0.009999999776482582 * (double)iLerpTime);
                            if (ps->viewHeightLerpTarget == 11)
                            {
                                PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchProne, &fNewPosOfs);
                            }
                            else if (ps->viewHeightLerpTarget == 40)
                            {
                                if (ps->viewHeightLerpDown)
                                    PM_ViewHeightTableLerp(iLerpFrac, viewLerp_StandCrouch, &fNewPosOfs);
                                else
                                    PM_ViewHeightTableLerp(iLerpFrac, viewLerp_ProneCrouch, &fNewPosOfs);
                            }
                            else
                            {
                                PM_ViewHeightTableLerp(iLerpFrac, viewLerp_CrouchStand, &fNewPosOfs);
                            }
                            if (fNewPosOfs != 0.0 && !alwaysfails)
                                MyAssertHandler(
                                    ".\\bgame\\bg_pmove.cpp",
                                    2400,
                                    0,
                                    "Wow, someone added view height offsets back in!  You need to restore the viewHeightLerpPosAdj field back in!");
                        }
                    }
                }
                else if (ps->viewHeightCurrent != (double)ps->viewHeightTarget)
                {
                    ps->viewHeightLerpTime = pm->cmd.serverTime;
                    switch (ps->viewHeightTarget)
                    {
                    case 0xB:
                        ps->viewHeightLerpDown = 1;
                        if (ps->viewHeightCurrent <= 40.0)
                            ps->viewHeightLerpTarget = 11;
                        else
                            ps->viewHeightLerpTarget = 40;
                        break;
                    case 0x28:
                        ps->viewHeightLerpDown = ps->viewHeightCurrent > (double)ps->viewHeightTarget;
                        ps->viewHeightLerpTarget = 40;
                        break;
                    case 0x3C:
                        ps->viewHeightLerpDown = 0;
                        if (ps->viewHeightCurrent >= 40.0)
                            ps->viewHeightLerpTarget = 60;
                        else
                            ps->viewHeightLerpTarget = 40;
                        break;
                    default:
                        if (!alwaysfails)
                        {
                            v5 = va("View height lerp to %i reached bad place\n", ps->viewHeightTarget);
                            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2441, 0, v5);
                        }
                        break;
                    }
                }
            }
            else
            {
                ps->viewHeightLerpTime = 0;
                if (ps->viewHeightCurrent >= (double)ps->viewHeightTarget)
                {
                    ps->viewHeightCurrent = ps->viewHeightCurrent - pml->frametime * 180.0;
                    if (ps->viewHeightCurrent <= (double)ps->viewHeightTarget)
                        ps->viewHeightCurrent = (float)ps->viewHeightTarget;
                }
                else
                {
                    ps->viewHeightCurrent = pml->frametime * 180.0 + ps->viewHeightCurrent;
                    if (ps->viewHeightCurrent >= (double)ps->viewHeightTarget)
                        ps->viewHeightCurrent = (float)ps->viewHeightTarget;
                }
            }
        }
    }
#ifdef KISAK_MP
    else if (ps->pm_type == PM_SPECTATOR)
    {
        ps->viewHeightCurrent = 0.0;
    }
#endif
    else
    {
        ps->viewHeightCurrent = (float)ps->viewHeightTarget;
    }
}

double __cdecl PM_ViewHeightTableLerp(int32_t iFrac, viewLerpWaypoint_s *pTable, float *pfPosOfs)
{
    float fEntryFrac; // [esp+10h] [ebp-10h]
    viewLerpWaypoint_s *pCurr; // [esp+14h] [ebp-Ch]
    int32_t i; // [esp+18h] [ebp-8h]
    viewLerpWaypoint_s *pPrev; // [esp+1Ch] [ebp-4h]

    if (iFrac)
    {
        if (iFrac >= 100)
            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 2152, 0, "%s", "iFrac < 100");
        pCurr = pTable + 1;
        i = 1;
        do
        {
            if (iFrac == pCurr->iFrac)
            {
                *pfPosOfs = (float)pCurr->iOffset;
                return pCurr->fViewHeight;
            }
            if (pCurr->iFrac > iFrac)
            {
                pPrev = &pTable[i - 1];
                iassert((pCurr->iFrac - pPrev->iFrac) > 0);
                fEntryFrac = (double)(iFrac - pPrev->iFrac) / (double)(pCurr->iFrac - pPrev->iFrac);
                *pfPosOfs = (double)pPrev->iOffset + (double)(pCurr->iOffset - pPrev->iOffset) * fEntryFrac;
                return (float)((pCurr->fViewHeight - pPrev->fViewHeight) * fEntryFrac + pPrev->fViewHeight);
            }
            pCurr = &pTable[++i];
        } while (pCurr->iFrac != -1);
        if (!va("No encapsulating table entries found for fraction %i", iFrac))
            MyAssertHandler(
                ".\\bgame\\bg_pmove.cpp",
                2187,
                0,
                "%s",
                "va( \"No encapsulating table entries found for fraction %i\", iFrac )");
        *pfPosOfs = (float)pTable->iOffset;
        return pTable->fViewHeight;
    }
    else
    {
        *pfPosOfs = (float)pTable->iOffset;
        return pTable->fViewHeight;
    }
}

void __cdecl PM_Footsteps(pmove_t *pm, pml_t *pml)
{
    scriptAnimMoveTypes_t StanceIdleAnim; // eax
    int32_t v3; // ecx
    bool Footsteps; // eax
    bool v5; // [esp+10h] [ebp-30h]
    scriptAnimMoveTypes_t moveAnim; // [esp+18h] [ebp-28h]
    float fMaxSpeed; // [esp+1Ch] [ebp-24h]
    int32_t iStance; // [esp+20h] [ebp-20h]
    bool walking; // [esp+24h] [ebp-1Ch]
    bool sprinting; // [esp+28h] [ebp-18h]
    playerState_s *ps; // [esp+30h] [ebp-10h]
    float bobmove; // [esp+34h] [ebp-Ch]
    int32_t old; // [esp+38h] [ebp-8h]
    PmStanceFrontBack stanceFrontBack; // [esp+3Ch] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3526, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3529, 0, "%s", "ps");
    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3537, 0, "%s", "bgs");
    if (ps->pm_type < PM_DEAD)
    {
        pm->xyspeed = Vec2Length(ps->velocity);
        if ((ps->eFlags & 0x300) != 0)
        {
            StanceIdleAnim = (scriptAnimMoveTypes_t)PM_GetStanceIdleAnim(ps->pm_flags);
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, StanceIdleAnim, 0);
        }
        else
        {
            iStance = PM_GetEffectiveStance(ps);
            if (ps->groundEntityNum != ENTITYNUM_NONE || ps->pm_type == PM_NORMAL_LINKED)
            {
                v5 = (ps->pm_flags & 0x40) != 0 || ps->leanf != 0.0;
                walking = v5;
                v3 = ps->pm_flags & 0x8000;
                sprinting = v3 != 0;
                if (v3 && v5)
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3569, 0, "%s", "!sprinting || !walking");
                if (player_moveThreshhold->current.value > (double)pm->xyspeed || ps->pm_type == PM_NORMAL_LINKED)
                {
                    PM_Footsteps_NotMoving(pm, iStance);
                }
                else if (pm->cmd.forwardmove || pm->cmd.rightmove)
                {
                    stanceFrontBack = (PmStanceFrontBack)PM_GetStanceEx(iStance, ps->pm_flags & 0x20);
                    PM_SetStrafeCondition(pm);
                    moveAnim = (scriptAnimMoveTypes_t)PM_GetMoveAnim(ps, stanceFrontBack, walking, sprinting);
                    if (BG_AnimScriptAnimation(ps, AISTATE_COMBAT, moveAnim, 0) < 0)
                        BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_IDLE, 0);
                    fMaxSpeed = PM_GetMaxSpeed(pm, walking, sprinting);
                    bobmove = PM_GetBobMove(stanceFrontBack, pm->xyspeed, fMaxSpeed, walking, sprinting);
                    old = ps->bobCycle;
                    ps->bobCycle = (uint8_t)(int)((double)old + (double)pml->msec * bobmove);
                    Footsteps = PM_ShouldMakeFootsteps(pm);
                    PM_FootstepEvent(pm, pml, old, ps->bobCycle, Footsteps);
                }
                else
                {
                    PM_Footstep_NotTryingToMove(pm);
                }
            }
            else
            {
                PM_Footstep_LadderMove(pm, pml);
                if (iStance != (ps->pm_flags & 3))
                    MyAssertHandler(
                        ".\\bgame\\bg_pmove.cpp",
                        3562,
                        0,
                        "%s",
                        "iStance == (ps->pm_flags & (PMF_DUCKED | PMF_PRONE))");
            }
        }
    }
}

int32_t __cdecl PM_GetStanceEx(int32_t stance, int32_t backward)
{
    if (stance >= 3)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 388, 0, "%s", "stance < PM_STANCE_BACKWARD_FIRST");
    if (backward)
        return stance + 3;
    else
        return stance;
}

void __cdecl PM_Footstep_LadderMove(pmove_t *pm, pml_t *pml)
{
    float fMaxSpeed; // [esp+0h] [ebp-14h]
    float fLadderSpeed; // [esp+4h] [ebp-10h]
    playerState_s *ps; // [esp+8h] [ebp-Ch]
    float bobmove; // [esp+Ch] [ebp-8h]
    int32_t old; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if ((pm->ps->pm_flags & 8) != 0 && pm->cmd.serverTime - ps->jumpTime >= 300)
    {
        fLadderSpeed = ps->velocity[2];
        fMaxSpeed = 0.5 * 1.5 * 127.0;
        if ((ps->pm_flags & 0x40) == 0 && ps->leanf == 0.0)
            bobmove = fLadderSpeed / (fMaxSpeed * 1.0) * 0.449999988079071;
        else
            bobmove = fLadderSpeed / (fMaxSpeed * 0.4000000059604645) * 0.3499999940395355;
        if (fLadderSpeed < 0.0)
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_CLIMBDOWN, 0);
        else
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_CLIMBUP, 0);
        old = ps->bobCycle;
        ps->bobCycle = (uint8_t)(int)((double)old + (double)pml->msec * bobmove);
        PM_FootstepEvent(pm, pml, old, ps->bobCycle, 1);
    }
}

void __cdecl PM_Footsteps_NotMoving(pmove_t *pm, int32_t stance)
{
#ifdef KISAK_SP
    if (pm->xyspeed < 1.0)
        pm->ps->bobCycle = 0;
#elif KISAK_MP
    int32_t EffectiveStance; // eax
    scriptAnimMoveTypes_t flinch_anim; // [esp+0h] [ebp-18h]
    int32_t turnAdjust; // [esp+4h] [ebp-14h]
    int32_t animResult; // [esp+8h] [ebp-10h]
    clientInfo_t *ci; // [esp+Ch] [ebp-Ch]
    scriptAnimMoveTypes_t anim; // [esp+10h] [ebp-8h]
    playerState_s *ps; // [esp+14h] [ebp-4h]

    ps = pm->ps;
    if (pm->xyspeed < 1.0)
        ps->bobCycle = 0;
    turnAdjust = 0;
    if (ps->clientNum >= 64)
        ci = 0;
    else
        ci = &bgs->clientinfo[ps->clientNum];
    if (ci && player_turnAnims->current.enabled && stance != 1)
        turnAdjust = PM_Footsteps_TurnAnim(ci);
    EffectiveStance = PM_GetEffectiveStance(ps);
    anim = PM_GetNotMovingAnim(EffectiveStance, turnAdjust);
    if (turnAdjust)
    {
        animResult = BG_AnimScriptAnimation(ps, AISTATE_COMBAT, anim, 0);
        if (animResult > 0 && !ci->turnAnimEndTime)
        {
            if (ps->legsAnimDuration <= 0)
                MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3278, 0, "%s", "ps->legsAnimDuration > 0");
            ci->turnAnimEndTime = ps->legsAnimDuration + bgs->time;
            if (xanim_debug->current.enabled)
                Com_Printf(17, "[%i] turn anim should end at %i\n", bgs->time, ps->legsAnimDuration + bgs->time);
        }
    }
    else
    {
        if (PM_ShouldFlinch(ps) && ps->viewHeightTarget == 60)
        {
            flinch_anim = (scriptAnimMoveTypes_t)PM_GetFlinchAnim(ps->flinchYawAnim);
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, flinch_anim, 0);
            return;
        }
        animResult = BG_AnimScriptAnimation(ps, AISTATE_COMBAT, anim, 0);
    }
    if (animResult < 0)
    {
        if (ps->viewHeightTarget == 40)
        {
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_IDLECR, 0);
        }
        else if (PM_ShouldFlinch(ps))
        {
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_FLINCH_FORWARD, 0);
        }
        else
        {
            BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_IDLE, 0);
        }
    }
#endif
}

uint32_t __cdecl PM_GetFlinchAnim(uint32_t flinchAnimDir)
{
    if (flinchAnimDir >= 4)
        MyAssertHandler(
            ".\\bgame\\bg_pmove.cpp",
            3100,
            0,
            "%s\n\t(flinchAnimDir) = %i",
            "(flinchAnimDir >= 0 && flinchAnimDir <= 3)",
            flinchAnimDir);
    return flinchAnimDir + 32;
}

int32_t __cdecl PM_Footsteps_TurnAnim(clientInfo_t *ci)
{
    int32_t turnAdjust; // [esp+0h] [ebp-4h]

    turnAdjust = 0;
    if (ci->turnAnimType && ci->turnAnimEndTime)
        Com_DPrintf(17, "turn anim end time is %i, time is %i\n", ci->turnAnimEndTime, bgs->time);
    if (ci->legs.yawing)
    {
        if (ci->torso.yawAngle >= (double)ci->legs.yawAngle)
            turnAdjust = 15;
        else
            turnAdjust = 14;
        ci->legs.yawAngle = ci->torso.yawAngle;
        ci->turnAnimType = turnAdjust;
        if (ci->turnAnimEndTime < bgs->time)
        {
            if (xanim_debug->current.enabled)
                Com_Printf(17, "Restarting turn animation because it is done playing\n");
            ci->turnAnimEndTime = 0;
        }
    }
    else
    {
        if (ci->turnAnimEndTime > bgs->time)
        {
            ci->legs.yawAngle = ci->torso.yawAngle;
            return 0;
        }
        if (ci->turnAnimEndTime)
        {
            ci->turnAnimEndTime = 0;
            ci->legs.yawAngle = ci->torso.yawAngle;
            if (xanim_debug->current.enabled)
                Com_Printf(17, "[%i] playing idle anim after turn anim\n", bgs->time);
        }
    }
    return turnAdjust;
}

scriptAnimMoveTypes_t __cdecl PM_GetNotMovingAnim(int32_t stance, int32_t turnAdjust)
{
    int32_t turn; // [esp+0h] [ebp-4h]

    if (turnAdjust)
        turn = (turnAdjust != 14) + 1;
    else
        turn = 0;
    return notMovingAnims[stance][turn];
}

#ifdef KISAK_MP
bool __cdecl PM_ShouldFlinch(playerState_s *ps)
{
    int32_t flinch_end_time; // [esp+0h] [ebp-4h]

    flinch_end_time = ps->damageDuration - player_dmgtimer_flinchTime->current.integer;
    if (flinch_end_time < 0)
        flinch_end_time = 0;
    return ps->damageTimer > flinch_end_time;
}
#endif

double __cdecl PM_GetMaxSpeed(pmove_t *pm, int32_t walking, int32_t sprinting)
{
    double v3; // st7
    float fMaxSpeed; // [esp+0h] [ebp-8h]
    float fMaxSpeeda; // [esp+0h] [ebp-8h]
    WeaponDef *weapon; // [esp+4h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3321, 0, "%s", "pm");
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3322, 0, "%s", "pm->ps");
    fMaxSpeed = (float)pm->ps->speed;
    if (pm->cmd.forwardmove)
    {
        if (pm->cmd.rightmove)
        {
            fMaxSpeed = ((player_strafeSpeedScale->current.value - 1.0) * 0.75 + 1.0 + 1.0) * 0.5 * fMaxSpeed;
            if (pm->cmd.forwardmove < 0)
                fMaxSpeed = (player_backSpeedScale->current.value + 1.0) * 0.5 * fMaxSpeed;
        }
        else if (pm->cmd.forwardmove < 0)
        {
            fMaxSpeed = fMaxSpeed * player_backSpeedScale->current.value;
        }
    }
    else if (pm->cmd.rightmove)
    {
        fMaxSpeed = ((player_strafeSpeedScale->current.value - 1.0) * 0.75 + 1.0) * fMaxSpeed;
    }
    if (walking)
    {
        fMaxSpeeda = fMaxSpeed * 0.4000000059604645;
    }
    else
    {
        if (sprinting)
            v3 = fMaxSpeed * player_sprintSpeedScale->current.value;
        else
            v3 = fMaxSpeed * 1.0;
        fMaxSpeeda = v3;
    }
    if (pm->ps->weapon)
    {
        weapon = BG_GetWeaponDef(pm->ps->weapon);
        if (weapon->moveSpeedScale <= 0.0 || (pm->ps->pm_flags & 0x40) != 0)
        {
            if (weapon->adsMoveSpeedScale > 0.0)
                fMaxSpeeda = fMaxSpeeda * weapon->adsMoveSpeedScale;
        }
        else
        {
            fMaxSpeeda = fMaxSpeeda * weapon->moveSpeedScale;
        }
    }
    return (float)(PM_CmdScaleForStance(pm) * fMaxSpeeda);
}

double __cdecl PM_GetBobMove(PmStanceFrontBack stance, float xyspeed, float fMaxSpeed, int32_t walking, int32_t sprinting)
{
    float bobFactor; // [esp+4h] [ebp-4h]

    if (stance || !sprinting)
        bobFactor = bobFactorTable[stance][walking];
    else
        bobFactor = player_sprintCameraBob->current.value;
    return (float)(xyspeed / fMaxSpeed * bobFactor);
}

int32_t __cdecl PM_GetStanceIdleAnim(char stanceFlag)
{
    if ((stanceFlag & 1) != 0)
        return 3;
    if ((stanceFlag & 2) != 0)
        return 2;
    return 1;
}

#ifdef KISAK_MP
int32_t __cdecl PM_GetMoveAnim(playerState_s *ps, PmStanceFrontBack stance, int32_t walking, int32_t sprinting)
{
    scriptAnimMoveTypes_t moveAnim; // [esp+0h] [ebp-Ch]
    int32_t stumble_end_time; // [esp+4h] [ebp-8h]

    stumble_end_time = ps->damageDuration - player_dmgtimer_stumbleTime->current.integer;
    if (stumble_end_time < 0)
        stumble_end_time = 0;
    moveAnim = moveAnimTable[stance][walking][ps->damageTimer > stumble_end_time];
    if (stance == PM_STANCE_STAND && sprinting)
        return ps->damageTimer > stumble_end_time ? 42 : 20;
    return moveAnim;
}
#endif

void __cdecl PM_SetStrafeCondition(pmove_t *pm)
{
    float moveVec[2]; // [esp+8h] [ebp-Ch] BYREF
    scriptAnimStrafeStates_t strafeState; // [esp+10h] [ebp-4h]

    moveVec[0] = (float)pm->cmd.rightmove;
    moveVec[1] = (float)pm->cmd.forwardmove;
    Vec2Normalize(moveVec);
    if (player_strafeAnimCosAngle->current.value >= (double)moveVec[1]
        && moveVec[1] >= -player_strafeAnimCosAngle->current.value)
    {
        if (pm->cmd.rightmove <= 0)
            strafeState = ANIM_STRAFE_LEFT;
        else
            strafeState = ANIM_STRAFE_RIGHT;
    }
    else
    {
        strafeState = ANIM_STRAFE_NOT;
    }
    BG_SetConditionValue(pm->ps->clientNum, 8u, strafeState);
}

void __cdecl PM_Footstep_NotTryingToMove(pmove_t *pm)
{
    int32_t animResult; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    animResult = -1;
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3487, 0, "%s", "ps");
    if (pm->xyspeed <= 120.0)
    {
        if (ps->viewHeightTarget == 11)
        {
            animResult = BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_IDLEPRONE, 0);
        }
        else if (ps->viewHeightTarget == 40)
        {
            animResult = BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_IDLECR, 0);
        }
        if (animResult < 0)
        {
            if (PM_ShouldFlinch(ps))
                BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_FLINCH_FORWARD, 0);
            else
                BG_AnimScriptAnimation(ps, AISTATE_COMBAT, ANIM_MT_IDLE, 0);
        }
    }
}

void __cdecl PM_FoliageSounds(pmove_t *pm)
{
    float mins[3]; // [esp+Ch] [ebp-50h] BYREF
    float speedFrac; // [esp+18h] [ebp-44h]
    trace_t trace; // [esp+1Ch] [ebp-40h] BYREF
    float maxs[3]; // [esp+48h] [ebp-14h] BYREF
    int32_t interval; // [esp+54h] [ebp-8h]
    playerState_s *ps; // [esp+58h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3645, 0, "%s", "ps");
    if (bg_foliagesnd_minspeed->current.value <= (double)pm->xyspeed)
    {
        if (bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value <= 0.0)
            MyAssertHandler(
                ".\\bgame\\bg_pmove.cpp",
                3655,
                1,
                "%s",
                "bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value > 0");
        speedFrac = (pm->xyspeed - bg_foliagesnd_minspeed->current.value)
            / (bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value);
        if (speedFrac > 1.0)
            speedFrac = 1.0;
        interval = (int)((double)(bg_foliagesnd_fastinterval->current.integer - bg_foliagesnd_slowinterval->current.integer)
            * speedFrac
            + (double)bg_foliagesnd_slowinterval->current.integer);
        if (interval + ps->foliageSoundTime < pm->cmd.serverTime)
        {
            Vec3Scale(pm->mins, 0.75, mins);
            Vec3Scale(pm->maxs, 0.75, maxs);
            maxs[2] = pm->maxs[2] * 0.8999999761581421;
            PM_playerTrace(pm, &trace, ps->origin, mins, maxs, ps->origin, ps->clientNum, 2);
            if (trace.startsolid)
            {
                PM_AddEvent(ps, 1u);
                ps->foliageSoundTime = pm->cmd.serverTime;
            }
        }
    }
    else if (bg_foliagesnd_resetinterval->current.integer + ps->foliageSoundTime < pm->cmd.serverTime)
    {
        ps->foliageSoundTime = 0;
    }
}

void __cdecl PM_DropTimers(playerState_s *ps, pml_t *pml)
{
#ifdef KISAK_SP
    int pm_time; // r11
    int msec; // r10
    unsigned int v6; // r11

    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\bg_pmove.cpp", 3679, 0, "%s", "ps");
    pm_time = ps->pm_time;
    if (pm_time)
    {
        msec = pml->msec;
        if (msec < pm_time)
        {
            ps->pm_time = pm_time - msec;
        }
        else
        {
            if ((ps->pm_flags & 0x4000) != 0)
                Jump_ClearState(ps);
            v6 = ps->pm_flags & 0xFFFFBE7F;
            ps->pm_time = 0;
            ps->pm_flags = v6;
        }
    }
#elif KISAK_MP
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 3686, 0, "%s", "ps");
    if (ps->pm_time)
    {
        if (pml->msec < ps->pm_time)
        {
            ps->pm_time -= pml->msec;
        }
        else
        {
            if ((ps->pm_flags & 0x4000) != 0)
                Jump_ClearState(ps);
            ps->pm_flags &= 0xFFFFBE7F;
            ps->pm_time = 0;
        }
    }
    if (ps->legsTimer > 0)
    {
        ps->legsTimer -= pml->msec;
        if (ps->legsTimer < 0)
        {
            ps->legsTimer = 0;
            if (G_IsServerGameSystem(ps->clientNum))
                Com_Printf(19, "end legs\n");
        }
    }
    if (ps->torsoTimer > 0)
    {
        ps->torsoTimer -= pml->msec;
        if (ps->torsoTimer < 0)
        {
            ps->torsoTimer = 0;
            if (G_IsServerGameSystem(ps->clientNum))
                Com_Printf(19, "end torso\n");
        }
    }
#endif
}

void __cdecl PM_UpdatePlayerWalkingFlag(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4379, 0, "%s", "ps");
    ps->pm_flags &= ~0x40u;
    if (ps->pm_type < PM_DEAD
        && (pm->cmd.buttons & 0x800) != 0
        && (ps->pm_flags & 1) == 0
        && (ps->pm_flags & 0x10) != 0
        && ps->weaponstate != 7
        && ps->weaponstate != 9
        && ps->weaponstate != 11
        && ps->weaponstate != 10
        && ps->weaponstate != 8)
    {
        ps->pm_flags |= 0x40u;
        if ((ps->otherFlags & 4) == 0)
            MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4404, 0, "%s", "ps->otherFlags & POF_PLAYER");
    }
}

void __cdecl PM_ClearLadderFlag(playerState_s *ps)
{
    if ((ps->pm_flags & 8) != 0)
    {
        ps->pm_flags |= 0x2000u;
        ps->pm_flags &= ~8u;
    }
}

void __cdecl PM_CheckLadderMove(pmove_t *pm, pml_t *pml)
{
    bool v2; // [esp+Ch] [ebp-8Ch]
    float *vLadderVec; // [esp+14h] [ebp-84h]
    float vLadderCheckDir[3]; // [esp+30h] [ebp-68h] BYREF
    float mins[3]; // [esp+3Ch] [ebp-5Ch] BYREF
    float tracedist; // [esp+48h] [ebp-50h]
    float spot[3]; // [esp+4Ch] [ebp-4Ch] BYREF
    int32_t fellOffLadderInAir; // [esp+58h] [ebp-40h]
    float maxs[3]; // [esp+5Ch] [ebp-3Ch] BYREF
    trace_t trace; // [esp+68h] [ebp-30h] BYREF
    playerState_s *ps; // [esp+94h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4443, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4446, 0, "%s", "ps");
    if (pml->walking)
        ps->pm_flags &= ~0x2000u;
    if (!ps->pm_time || (ps->pm_flags & 8) != 0 || (ps->pm_flags & 0x180) == 0)
    {
        if (pml->walking)
            tracedist = 8.0;
        else
            tracedist = 30.0;
        v2 = (ps->pm_flags & 8) != 0 && ps->groundEntityNum == ENTITYNUM_NONE;
        fellOffLadderInAir = v2;
        if (v2)
        {
            vLadderCheckDir[0] = -ps->vLadderVec[0];
            vLadderCheckDir[1] = -ps->vLadderVec[1];
            vLadderCheckDir[2] = -ps->vLadderVec[2];
        }
        else
        {
            vLadderCheckDir[0] = pml->forward[0];
            vLadderCheckDir[1] = pml->forward[1];
            vLadderCheckDir[2] = 0.0;
            Vec3Normalize(vLadderCheckDir);
        }
        if (ps->pm_type < PM_DEAD)
        {
            if ((ps->pm_flags & 0x2000) != 0 || PM_GetEffectiveStance(ps) == 1 || pm->cmd.serverTime - ps->jumpTime < 300)
            {
                PM_ClearLadderFlag(ps);
            }
            else
            {
                mins[0] = pm->mins[0];
                mins[1] = pm->mins[1];
                mins[0] = mins[0] + 6.0;
                mins[1] = mins[1] + 6.0;
                mins[2] = 8.0;
                maxs[0] = pm->maxs[0];
                maxs[1] = pm->maxs[1];
                maxs[2] = pm->maxs[2];
                maxs[0] = maxs[0] - 6.0;
                maxs[1] = maxs[1] - 6.0;
                if ((float)8.0 > (double)maxs[2])
                    maxs[2] = mins[2];
                if (mins[0] > (double)maxs[0])
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4524, 0, "%s", "maxs[0] >= mins[0]");
                if (mins[1] > (double)maxs[1])
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4525, 0, "%s", "maxs[1] >= mins[1]");
                if (mins[2] > (double)maxs[2])
                    MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4526, 0, "%s", "maxs[2] >= mins[2]");
                Vec3Mad(ps->origin, tracedist, vLadderCheckDir, spot);
                PM_playerTrace(pm, &trace, ps->origin, mins, maxs, spot, ps->clientNum, pm->tracemask);
                if (trace.fraction >= 1.0 || (trace.surfaceFlags & 8) == 0 || pml->walking && pm->cmd.forwardmove <= 0)
                    goto LABEL_45;
                if ((ps->pm_flags & 8) != 0)
                    goto LABEL_42;
                vLadderVec = ps->vLadderVec;
                ps->vLadderVec[0] = trace.normal[0];
                vLadderVec[1] = trace.normal[1];
                vLadderVec[2] = trace.normal[2];
                vLadderCheckDir[0] = -ps->vLadderVec[0];
                vLadderCheckDir[1] = -ps->vLadderVec[1];
                vLadderCheckDir[2] = -ps->vLadderVec[2];
                Vec3Mad(ps->origin, tracedist, vLadderCheckDir, spot);
                PM_playerTrace(pm, &trace, ps->origin, mins, maxs, spot, ps->clientNum, pm->tracemask);
                if (trace.fraction >= 1.0)
                    goto LABEL_45;
                if ((trace.surfaceFlags & 8) != 0)
                {
                LABEL_42:
                    PM_SetLadderFlag(ps);
                }
                else
                {
                LABEL_45:
                    PM_ClearLadderFlag(ps);
                    if (fellOffLadderInAir)
                        BG_AnimScriptEvent(ps, ANIM_ET_JUMP, 0, 1);
                }
            }
        }
        else
        {
            ps->groundEntityNum = ENTITYNUM_NONE;
            pml->groundPlane = 0;
            pml->almostGroundPlane = 0;
            pml->walking = 0;
            PM_ClearLadderFlag(ps);
        }
    }
}

void __cdecl PM_SetLadderFlag(playerState_s *ps)
{
    ps->pm_flags |= 8u;
}

void __cdecl PM_LadderMove(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+Ch] [ebp-C4h]
    float v3; // [esp+10h] [ebp-C0h]
    float v4; // [esp+14h] [ebp-BCh]
    float v5; // [esp+18h] [ebp-B8h]
    float v6; // [esp+1Ch] [ebp-B4h]
    float v7; // [esp+20h] [ebp-B0h]
    float *v8; // [esp+2Ch] [ebp-A4h]
    float *v9; // [esp+30h] [ebp-A0h]
    float *v10; // [esp+34h] [ebp-9Ch]
    float v11; // [esp+3Ch] [ebp-94h]
    float *v12; // [esp+44h] [ebp-8Ch]
    float *vLadderVec; // [esp+48h] [ebp-88h]
    float v14; // [esp+4Ch] [ebp-84h]
    float *v15; // [esp+50h] [ebp-80h]
    float *v16; // [esp+5Ch] [ebp-74h]
    float *v17; // [esp+60h] [ebp-70h]
    float *velocity; // [esp+68h] [ebp-68h]
    float v19; // [esp+6Ch] [ebp-64h]
    float *v20; // [esp+70h] [ebp-60h]
    float wishdir[3]; // [esp+88h] [ebp-48h] BYREF
    float fSideSpeed; // [esp+94h] [ebp-3Ch]
    float wishvel[3]; // [esp+98h] [ebp-38h] BYREF
    float vTempRight[3]; // [esp+A4h] [ebp-2Ch] BYREF
    float vSideDir[2]; // [esp+B0h] [ebp-20h] BYREF
    float fSpeedDrop; // [esp+B8h] [ebp-18h]
    float wishspeed; // [esp+BCh] [ebp-14h]
    float upscale; // [esp+C0h] [ebp-10h]
    int32_t moveyaw; // [esp+C4h] [ebp-Ch]
    float scale; // [esp+C8h] [ebp-8h]
    playerState_s *ps; // [esp+CCh] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4582, 0, "%s", "pm");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4585, 0, "%s", "ps");
    if (Jump_Check(pm, pml))
    {
        PM_AirMove(pm, pml);
    }
    else
    {
        upscale = (pml->forward[2] + 0.25) * 2.5;
        if (upscale <= 1.0)
        {
            if (upscale < -1.0)
                upscale = -1.0;
        }
        else
        {
            upscale = 1.0;
        }
        pml->forward[2] = 0.0;
        Vec3Normalize(pml->forward);
        pml->right[2] = 0.0;
        Vec3NormalizeTo(pml->right, vTempRight);
        ProjectPointOnPlane(vTempRight, ps->vLadderVec, pml->right);
        scale = PM_CmdScale(ps, &pm->cmd);
        wishvel[0] = 0.0;
        wishvel[1] = 0.0;
        wishvel[2] = 0.0;
        if (pm->cmd.forwardmove)
            wishvel[2] = 0.5 * upscale * scale * (double)pm->cmd.forwardmove;
        if (pm->cmd.rightmove)
        {
            v7 = scale * 0.2000000029802322 * (double)pm->cmd.rightmove;
            Vec3Mad(wishvel, v7, pml->right, wishvel);
        }
        wishspeed = Vec3NormalizeTo(wishvel, wishdir);
        PM_Accelerate(ps, pml, wishdir, wishspeed, 9.0);
        if (!pm->cmd.forwardmove)
        {
            if (ps->velocity[2] <= 0.0)
            {
                ps->velocity[2] = (double)ps->gravity * pml->frametime + ps->velocity[2];
                if (ps->velocity[2] > 0.0)
                    ps->velocity[2] = 0.0;
            }
            else
            {
                ps->velocity[2] = ps->velocity[2] - (double)ps->gravity * pml->frametime;
                if (ps->velocity[2] < 0.0)
                    ps->velocity[2] = 0.0;
            }
        }
        if (!pm->cmd.rightmove)
        {
            vSideDir[0] = pml->right[0];
            vSideDir[1] = pml->right[1];
            Vec2Normalize(vSideDir);
            fSideSpeed = ps->velocity[1] * vSideDir[1] + ps->velocity[0] * vSideDir[0];
            if (fSideSpeed != 0.0)
            {
                velocity = ps->velocity;
                v19 = -fSideSpeed;
                v20 = ps->velocity;
                ps->velocity[0] = v19 * vSideDir[0] + ps->velocity[0];
                velocity[1] = v19 * vSideDir[1] + v20[1];
                fSpeedDrop = fSideSpeed * pml->frametime * 16.0;
                v6 = I_fabs(fSideSpeed);
                v5 = I_fabs(fSpeedDrop);
                if (v5 < (double)v6)
                {
                    v4 = I_fabs(fSpeedDrop);
                    if (v4 < 1.0)
                    {
                        if (fSpeedDrop < 0.0)
                            v3 = -1.0;
                        else
                            v3 = 1.0;
                        fSpeedDrop = v3;
                    }
                    fSideSpeed = fSideSpeed - fSpeedDrop;
                    v16 = ps->velocity;
                    v17 = ps->velocity;
                    ps->velocity[0] = fSideSpeed * vSideDir[0] + ps->velocity[0];
                    v16[1] = fSideSpeed * vSideDir[1] + v17[1];
                }
            }
        }
        if (!pml->walking)
        {
            fSideSpeed = ps->velocity[1] * ps->vLadderVec[1] + ps->velocity[0] * ps->vLadderVec[0];
            v12 = ps->velocity;
            vLadderVec = ps->vLadderVec;
            v14 = -fSideSpeed;
            v15 = ps->velocity;
            ps->velocity[0] = v14 * ps->vLadderVec[0] + ps->velocity[0];
            v12[1] = v14 * vLadderVec[1] + v15[1];
            if (ps->velocity[0] != 0.0 || ps->velocity[1] != 0.0 || ps->velocity[2] != 0.0)
            {
                v11 = ps->velocity[0] * ps->velocity[0] + ps->velocity[1] * ps->velocity[1];
                v2 = ps->velocity[2] * ps->velocity[2];
                if (v2 >= (double)v11)
                {
                    fSideSpeed = -50.0;
                    v8 = ps->velocity;
                    v9 = ps->vLadderVec;
                    v10 = ps->velocity;
                    ps->velocity[0] = (float)-50.0 * ps->vLadderVec[0] + ps->velocity[0];
                    v8[1] = fSideSpeed * v9[1] + v10[1];
                }
            }
        }
        PM_StepSlideMove(pm, pml, 0);
        scale = vectoyaw(ps->vLadderVec) + 180.0;
        moveyaw = (int)AngleDelta(scale, ps->viewangles[1]);
        if ((int)abs32(moveyaw) > 75)
        {
            if (moveyaw <= 0)
                moveyaw = -75;
            else
                moveyaw = 75;
        }
        ps->movementDir = (char)moveyaw;
    }
}

void __cdecl PM_MeleeChargeUpdate(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+Ch] [ebp-30h]
    bool v3; // [esp+10h] [ebp-2Ch]
    bool v4; // [esp+14h] [ebp-28h]
    bool v5; // [esp+18h] [ebp-24h]
    float *velocity; // [esp+1Ch] [ebp-20h]
    float v7; // [esp+20h] [ebp-1Ch]
    float chargeDir[2]; // [esp+24h] [ebp-18h] BYREF
    float chargeVel; // [esp+2Ch] [ebp-10h]
    float chargeTime; // [esp+30h] [ebp-Ch]
    playerState_s *ps; // [esp+34h] [ebp-8h]
    bool chargeValid; // [esp+3Bh] [ebp-1h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4760, 0, "%s", "pm");
    if (!pml)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4761, 0, "%s", "pml");
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4764, 0, "%s", "ps");
    chargeValid = (ps->pm_flags & 0x20000) != 0;
    v5 = chargeValid && !ps->pm_type;
    chargeValid = v5;
    v4 = v5 && (ps->eFlags & 0x300) == 0;
    chargeValid = v4;
    v3 = v4 && (ps->pm_flags & 0xC) == 0;
    chargeValid = v3;
    if (v3)
    {
        if (!ps->meleeChargeTime)
        {
            if (player_meleeChargeFriction->current.value <= 0.0)
                MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4784, 0, "%s", "player_meleeChargeFriction->current.value > 0.0f");
            YawVectors2D(ps->meleeChargeYaw, chargeDir, 0);
            v7 = player_meleeChargeFriction->current.value * ((double)ps->meleeChargeDist + (double)ps->meleeChargeDist);
            v2 = sqrt(v7);
            chargeVel = v2;
            velocity = ps->velocity;
            ps->velocity[0] = v2 * chargeDir[0];
            velocity[1] = chargeVel * chargeDir[1];
            chargeTime = chargeVel / player_meleeChargeFriction->current.value * 1000.0;
            ps->meleeChargeTime = (int)chargeTime;
        }
        ps->meleeChargeTime -= pml->msec;
        if (ps->meleeChargeTime <= 0)
            PM_MeleeChargeClear(ps);
    }
    else
    {
        PM_MeleeChargeClear(ps);
    }
}

void __cdecl TurretNVGTrigger(pmove_t *pm)
{
    playerState_s *ps; // [esp+8h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4805, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_pmove.cpp", 4807, 0, "%s", "ps");
    if ((pm->oldcmd.buttons & 0x40000) == 0 && (pm->cmd.buttons & 0x40000) != 0)
    {
        if ((ps->weapFlags & 0x40) != 0)
        {
            ps->weapFlags &= ~0x40u;
            PM_AddEvent(ps, 0x41u);
        }
        else
        {
            ps->weapFlags |= 0x40u;
            PM_AddEvent(ps, 0x40u);
        }
    }
}

double __cdecl BG_GetSpeed(const playerState_s *ps, int32_t time)
{
    if ((ps->pm_flags & 8) == 0)
        return Vec2Length(ps->velocity);
    if (time - ps->jumpTime >= 500)
        return ps->velocity[2];
    return 0.0;
}

// LWSS: might not be right file for this function
void __cdecl BG_Player_DoControllers(const CEntPlayerInfo *player, const DObj_s *obj, int32_t *partBits)
{
    clientControllers_t *control; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    control = player->control;
    for (i = 0; i < 6; ++i)
        DObjSetControlTagAngles((DObj_s *)obj, partBits, player->tag[i], control->angles[i]);
    DObjSetLocalTag((DObj_s *)obj, partBits, 0, control->tag_origin_offset, control->tag_origin_angles);
}
