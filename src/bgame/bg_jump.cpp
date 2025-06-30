#include <qcommon/qcommon.h>

#include "bg_public.h"
#include "bg_local.h"

const dvar_t *jump_height;
const dvar_t *jump_stepSize;
const dvar_t *jump_slowdownEnable;
const dvar_t *jump_ladderPushVel;
const dvar_t *jump_spreadAdd;

void __cdecl Jump_RegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-14h]
    DvarLimits mina; // [esp+4h] [ebp-14h]
    DvarLimits minb; // [esp+4h] [ebp-14h]
    DvarLimits minc; // [esp+4h] [ebp-14h]

    min.value.max = 1000.0;
    min.value.min = 0.0;
    jump_height = Dvar_RegisterFloat("jump_height", 39.0, min, DVAR_CHEAT | DVAR_TEMP, "The maximum height of a player's jump");
    mina.value.max = 64.0;
    mina.value.min = 0.0;
    jump_stepSize = Dvar_RegisterFloat(
        "jump_stepSize",
        18.0,
        mina,
        DVAR_CHEAT | DVAR_TEMP,
        "The maximum step up to the top of a jump arc");
    jump_slowdownEnable = Dvar_RegisterBool("jump_slowdownEnable", 1, DVAR_CHEAT | DVAR_TEMP, "Slow player movement after jumping");
    minb.value.max = 1024.0;
    minb.value.min = 0.0;
    jump_ladderPushVel = Dvar_RegisterFloat(
        "jump_ladderPushVel",
        128.0,
        minb,
        DVAR_CHEAT | DVAR_TEMP,
        "The velocity of a jump off of a ladder");
    minc.value.max = 512.0;
    minc.value.min = 0.0;
    jump_spreadAdd = Dvar_RegisterFloat(
        "jump_spreadAdd",
        64.0,
        minc,
        DVAR_CHEAT | DVAR_TEMP,
        "The amount of spread scale to add as a side effect of jumping");
}

void __cdecl Jump_ClearState(playerState_s *ps)
{
    ps->pm_flags &= ~0x4000u;
    ps->jumpOriginZ = 0.0;
}

char __cdecl Jump_GetStepHeight(playerState_s *ps, const float *origin, float *stepSize)
{
    float v4; // [esp+0h] [ebp-8h]
    float v5; // [esp+4h] [ebp-4h]

    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 84, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    if (!origin)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 85, 0, "%s", "origin");
    if (!stepSize)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 86, 0, "%s", "stepSize");
    v5 = ps->jumpOriginZ + jump_height->current.value;
    if (origin[2] >= (double)v5)
        return 0;
    *stepSize = jump_stepSize->current.value;
    v4 = ps->jumpOriginZ + jump_height->current.value;
    if (v4 < origin[2] + *stepSize)
        *stepSize = ps->jumpOriginZ + jump_height->current.value - origin[2];
    return 1;
}

bool __cdecl Jump_IsPlayerAboveMax(playerState_s *ps)
{
    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 108, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    return ps->origin[2] >= ps->jumpOriginZ + jump_height->current.value;
}

void __cdecl Jump_ActivateSlowdown(playerState_s *ps)
{
    if (!ps->pm_time)
    {
        ps->pm_flags |= 0x4000u;
        ps->pm_time = 1800;
    }
}

void __cdecl Jump_ApplySlowdown(playerState_s *ps)
{
    float scale; // [esp+8h] [ebp-4h]

    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 134, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    scale = 1.0;
    if (ps->pm_time <= 1800)
    {
        if (!ps->pm_time)
        {
            if (ps->origin[2] >= ps->jumpOriginZ + 18.0)
            {
                ps->pm_time = 1200;
                scale = 0.5;
            }
            else
            {
                ps->pm_time = 1800;
                scale = 0.64999998f;
            }
        }
    }
    else
    {
        Jump_ClearState(ps);
        scale = 0.64999998f;
    }
    if (jump_slowdownEnable->current.enabled)
        Vec3Scale(ps->velocity, scale, ps->velocity);
}

double __cdecl Jump_ReduceFriction(playerState_s *ps)
{
    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 187, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    if (ps->pm_time > 1800)
    {
        Jump_ClearState(ps);
        return (float)1.0;
    }
    else
    {
        return (float)Jump_GetSlowdownFriction(ps);
    }
}

double __cdecl Jump_GetSlowdownFriction(playerState_s *ps)
{
    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 167, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    if (ps->pm_time > 1800)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 168, 0, "%s", "ps->pm_time <= JUMP_LAND_SLOWDOWN_TIME");
    if (!jump_slowdownEnable->current.enabled)
        return 1.0;
    if (ps->pm_time >= 1700)
        return 2.5;
    return (float)((double)ps->pm_time * 1.5 * 0.0005882352706976235 + 1.0);
}

void __cdecl Jump_ClampVelocity(playerState_s *ps, const float *origin)
{
    float v2; // [esp+0h] [ebp-10h]
    float v3; // [esp+4h] [ebp-Ch]
    float heightDiff; // [esp+Ch] [ebp-4h]

    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 211, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    if (!origin)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 212, 0, "%s", "origin");
    if (ps->origin[2] - origin[2] > 0.0)
    {
        heightDiff = ps->jumpOriginZ + jump_height->current.value - ps->origin[2];
        if (heightDiff >= 0.1000000014901161)
        {
            v3 = (double)ps->gravity * (heightDiff + heightDiff);
            v2 = sqrt(v3);
            if (v2 < (double)ps->velocity[2])
                ps->velocity[2] = v2;
        }
        else
        {
            ps->velocity[2] = 0.0;
        }
    }
}

char __cdecl Jump_Check(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    if (!pm)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 368, 0, "%s", "pm");
    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 371, 0, "%s", "ps");
    if ((ps->pm_flags & 0x80000) != 0)
        return 0;
    if (pm->cmd.serverTime - ps->jumpTime < 500)
        return 0;
    if ((ps->pm_flags & 0x400) != 0)
        return 0;
    if ((ps->pm_flags & 4) != 0)
        return 0;
    if (ps->pm_type >= 7)
        return 0;
    if (PM_GetEffectiveStance(ps))
        return 0;
    if ((pm->cmd.buttons & 0x400) == 0)
        return 0;
    if ((pm->oldcmd.buttons & 0x400) != 0)
    {
        pm->cmd.buttons &= ~0x400u;
        return 0;
    }
    else
    {
        Jump_Start(pm, pml, jump_height->current.value);
        Jump_AddSurfaceEvent(ps, pml);
        if ((ps->pm_flags & 8) != 0)
            Jump_PushOffLadder(ps, pml);
        if (pm->cmd.forwardmove < 0)
            BG_AnimScriptEvent(ps, ANIM_ET_JUMPBK, 0, 1);
        else
            BG_AnimScriptEvent(ps, ANIM_ET_JUMP, 0, 1);
        return 1;
    }
}

void __cdecl Jump_Start(pmove_t *pm, pml_t *pml, float height)
{
    float v3; // [esp+0h] [ebp-10h]
    float factor; // [esp+4h] [ebp-Ch]
    float velocitySqrd; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 261, 0, "%s", "ps");
    velocitySqrd = (height + height) * (double)ps->gravity;
    if ((ps->pm_flags & 0x4000) != 0 && ps->pm_time <= 1800)
    {
        factor = Jump_GetLandFactor(ps);
        if (factor == 0.0)
            MyAssertHandler(".\\bgame\\bg_jump.cpp", 270, 0, "%s", "factor");
        velocitySqrd = velocitySqrd / factor;
    }
    pml->groundPlane = 0;
    pml->almostGroundPlane = 0;
    pml->walking = 0;
    ps->groundEntityNum = 1023;
    ps->jumpTime = pm->cmd.serverTime;
    ps->jumpOriginZ = ps->origin[2];
    v3 = sqrt(velocitySqrd);
    ps->velocity[2] = v3;
    ps->pm_flags &= 0xFFFFFE7F;
    ps->pm_flags |= 0x4000u;
    ps->pm_time = 0;
    ps->sprintState.sprintButtonUpRequired = 0;
    ps->aimSpreadScale = ps->aimSpreadScale + jump_spreadAdd->current.value;
    if (ps->aimSpreadScale > 255.0)
        ps->aimSpreadScale = 255.0;
}

double __cdecl Jump_GetLandFactor(playerState_s *ps)
{
    if ((ps->pm_flags & 0x4000) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 238, 0, "%s", "ps->pm_flags & PMF_JUMPING");
    if (ps->pm_time > 1800)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 239, 0, "%s", "ps->pm_time <= JUMP_LAND_SLOWDOWN_TIME");
    if (!jump_slowdownEnable->current.enabled)
        return 1.0;
    if (ps->pm_time >= 1700)
        return 2.5;
    return (float)((double)ps->pm_time * 1.5 * 0.0005882352706976235 + 1.0);
}

void __cdecl Jump_PushOffLadder(playerState_s *ps, pml_t *pml)
{
    float scale; // [esp+Ch] [ebp-48h]
    float value; // [esp+14h] [ebp-40h]
    float v4; // [esp+34h] [ebp-20h]
    float flatForward[3]; // [esp+38h] [ebp-1Ch] BYREF
    float pushOffDir[3]; // [esp+44h] [ebp-10h] BYREF
    float dot; // [esp+50h] [ebp-4h]

    if ((ps->pm_flags & 8) == 0)
        MyAssertHandler(".\\bgame\\bg_jump.cpp", 309, 0, "%s", "ps->pm_flags & PMF_LADDER");
    ps->velocity[2] = ps->velocity[2] * 0.75;
    v4 = pml->forward[1];
    flatForward[0] = pml->forward[0];
    flatForward[1] = v4;
    flatForward[2] = 0.0;
    Vec3Normalize(flatForward);
    dot = Vec3Dot(ps->vLadderVec, pml->forward);
    if (dot >= 0.0)
    {
        pushOffDir[0] = flatForward[0];
        pushOffDir[1] = flatForward[1];
        pushOffDir[2] = flatForward[2];
    }
    else
    {
        dot = Vec3Dot(flatForward, ps->vLadderVec);
        scale = dot * -2.0;
        Vec3Mad(flatForward, scale, ps->vLadderVec, pushOffDir);
        Vec3Normalize(pushOffDir);
    }
    value = jump_ladderPushVel->current.value;
    ps->velocity[0] = value * pushOffDir[0];
    ps->velocity[1] = value * pushOffDir[1];
    ps->pm_flags &= ~8u;
}

void __cdecl Jump_AddSurfaceEvent(playerState_s *ps, pml_t *pml)
{
    int surfType; // [esp+0h] [ebp-4h]

    if ((ps->pm_flags & 8) != 0)
    {
        BG_AddPredictableEventToPlayerstate(0x4Cu, 0x15u, ps);
    }
    else
    {
        surfType = PM_GroundSurfaceType(pml);
        if (surfType)
            BG_AddPredictableEventToPlayerstate(0x4Cu, surfType, ps);
    }
}

