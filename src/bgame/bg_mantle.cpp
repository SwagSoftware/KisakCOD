#include "bg_public.h"
#include "bg_local.h"
#include <aim_assist/aim_assist.h>
#include <xanim/xanim.h>


const dvar_t *mantle_enable;
const dvar_t *mantle_debug;
const dvar_t *mantle_check_range;
const dvar_t *mantle_check_radius;
const dvar_t *mantle_check_angle;
const dvar_t *mantle_view_yawcap;

XAnim_s *s_mantleAnims;

const char *s_mantleAnimNames[11] =
{
    "mp_mantle_root",
    "mp_mantle_up_57",
    "mp_mantle_up_51",
    "mp_mantle_up_45",
    "mp_mantle_up_39",
    "mp_mantle_up_33",
    "mp_mantle_up_27",
    "mp_mantle_up_21",
    "mp_mantle_over_high",
    "mp_mantle_over_mid",
    //"mp_mantle_over_low" // LWSS: WOW! This was a horrible typo!!!! !!! ! ! !! ! !! ! ! ! ! mp_mantle_over_low lacks bDelta which throws off the results and is wrong. This is likely a hack the devs did
    "player_mantle_over_low"
};

const MantleAnimTransition s_mantleTrans[7] =
{
    {1, 8, 57.0f},
    {2, 8, 51.0f},
    {3, 9, 45.0f},
    {4, 9, 39.0f},
    {5, 9, 33.0f},
    {6, 10, 27.0f},
    {7, 10, 21.0f}
};

void __cdecl Mantle_RegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-14h]
    DvarLimits mina; // [esp+4h] [ebp-14h]
    DvarLimits minb; // [esp+4h] [ebp-14h]
    DvarLimits minc; // [esp+4h] [ebp-14h]

    mantle_enable = Dvar_RegisterBool("mantle_enable", 1, DVAR_CHEAT | DVAR_TEMP, "Enable player mantling");
    mantle_debug = Dvar_RegisterBool("mantle_debug", 0, DVAR_CHEAT | DVAR_TEMP, "Show debug information for mantling");
    min.value.max = 128.0f;
    min.value.min = 0.0f;
    mantle_check_range = Dvar_RegisterFloat(
        "mantle_check_range",
        20.0f,
        min,
        DVAR_CHEAT | DVAR_TEMP,
        "The minimum distance from a player to a mantle surface to allow a mantle");
    mina.value.max = 15.0f;
    mina.value.min = 0.0f;
    mantle_check_radius = Dvar_RegisterFloat(
        "mantle_check_radius",
        0.1f,
        mina,
        DVAR_CHEAT | DVAR_TEMP,
        "The player radius to test against while mantling");
    minb.value.max = 180.0f;
    minb.value.min = 0.0f;
    mantle_check_angle = Dvar_RegisterFloat(
        "mantle_check_angle",
        60.0f,
        minb,
        DVAR_CHEAT | DVAR_TEMP,
        "The minimum angle from the player to a mantle surface to allow a mantle");
    minc.value.max = 180.0f;
    minc.value.min = 0.0f;
    mantle_view_yawcap = Dvar_RegisterFloat(
        "mantle_view_yawcap",
        60.0f,
        minc,
        DVAR_CHEAT | DVAR_TEMP,
        "The angle at which to restrict a sideways turn while mantling");
}

void __cdecl Mantle_CreateAnims(void *(__cdecl *xanimAlloc)(int32_t))
{
    float v1; // [esp+10h] [ebp-50h]
    float v2; // [esp+14h] [ebp-4Ch]
    float v3; // [esp+18h] [ebp-48h]
    float v4; // [esp+1Ch] [ebp-44h]
    float v5; // [esp+20h] [ebp-40h]
    float v6; // [esp+24h] [ebp-3Ch]
    float v7; // [esp+28h] [ebp-38h]
    float v8; // [esp+30h] [ebp-30h]
    float v9; // [esp+34h] [ebp-2Ch]
    float v10; // [esp+3Ch] [ebp-24h]
    float delta[3]; // [esp+40h] [ebp-20h] BYREF
    int32_t animIndex; // [esp+4Ch] [ebp-14h]
    int32_t transIndex; // [esp+50h] [ebp-10h]
    float rot[3]; // [esp+54h] [ebp-Ch] BYREF

    if (!s_mantleAnims)
    {
        s_mantleAnims = XAnimCreateAnims("PLAYER_MANTLE", 0xBu, xanimAlloc);
        if (!s_mantleAnims)
            MyAssertHandler(".\\bgame\\bg_mantle.cpp", 530, 0, "%s", "s_mantleAnims");
        XAnimBlend(s_mantleAnims, 0, s_mantleAnimNames[0], 1u, 0xAu, 0);
        for (animIndex = 1; animIndex < 11; ++animIndex)
            BG_CreateXAnim(s_mantleAnims, animIndex, (char*)s_mantleAnimNames[animIndex]);
        for (transIndex = 0; transIndex < 7; ++transIndex)
        {
            animIndex = s_mantleTrans[transIndex].upAnimIndex;
            XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, 1.0);
            v10 = delta[0] - 16.0;
            v6 = I_fabs(v10);
            if (v6 > 1.0)
                Com_Error(ERR_DROP, "Mantle anim [%s] has X translation %f, should be %f\n", s_mantleAnimNames[animIndex], delta[0], 16.0);
            v5 = I_fabs(delta[1]);
            if (v5 > 1.0)
                Com_Error(ERR_DROP, "Mantle anim [%s] has Y translation %f, should be %f\n", s_mantleAnimNames[animIndex], delta[1], 0.0);
            v9 = delta[2] - s_mantleTrans[transIndex].height;
            v4 = I_fabs(v9);
            if (v4 > 1.0)
                Com_Error(ERR_DROP, "Mantle anim [%s] has Z translation %f, should be %f\n", s_mantleAnimNames[animIndex], delta[2], s_mantleTrans[transIndex].height);

            animIndex = s_mantleTrans[transIndex].overAnimIndex;
            XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, 1.0);
            v8 = delta[0] - 31.0;
            v3 = I_fabs(v8);
            if (v3 > 1.0)
                Com_Error(ERR_DROP, "Mantle anim [%s] has X translation %f, should be %f\n", s_mantleAnimNames[animIndex], delta[0], 31.0);
            v2 = I_fabs(delta[1]);
            if (v2 > 1.0)
                Com_Error(ERR_DROP, "Mantle anim [%s] has Y translation %f, should be %f\n", s_mantleAnimNames[animIndex], delta[1], 0.0);
            v7 = delta[2] - -18.0;
            v1 = I_fabs(v7);
            if (v1 > 1.0)
                Com_Error(ERR_DROP, "Mantle anim [%s] has Z translation %f, should be %f\n", s_mantleAnimNames[animIndex], delta[2], -18.0);
        }
    }
}

void __cdecl Mantle_ShutdownAnims()
{
    s_mantleAnims = 0;
}

void __cdecl Mantle_Check(pmove_t *pm, pml_t *pml)
{
    float mantleDir[3]; // [esp+8h] [ebp-74h] BYREF
    trace_t trace; // [esp+14h] [ebp-68h] BYREF
    MantleResults mresults; // [esp+40h] [ebp-3Ch] BYREF
    playerState_s *ps; // [esp+78h] [ebp-4h]

    Mantle_DebugPrint("====== Mantle Debug ======");
    pm->ps->mantleState.flags &= ~0x10u;
    if (mantle_enable->current.enabled)
    {
        ps = pm->ps;
        if (!ps)
            MyAssertHandler(".\\bgame\\bg_mantle.cpp", 682, 0, "%s", "ps");
        Mantle_ClearHint(ps);
        if (ps->pm_type < PM_DEAD)
        {
            if ((ps->pm_flags & 4) != 0)
            {
                Mantle_DebugPrint("Mantle Failed: Player already mantling");
            }
            else if ((ps->eFlags & 0xC) != 0)
            {
                Mantle_DebugPrint("Mantle Failed: Player not standing");
            }
            else if (Mantle_FindMantleSurface(pm, pml, &trace, mantleDir))
            {
                memset((unsigned __int8 *)&mresults, 0, sizeof(mresults));
                mresults.dir[0] = mantleDir[0];
                mresults.dir[1] = mantleDir[1];
                mresults.dir[2] = mantleDir[2];
                mresults.startPos[0] = ps->origin[0];
                mresults.startPos[1] = ps->origin[1];
                mresults.startPos[2] = ps->origin[2];
                if ((trace.surfaceFlags & 0x4000000) != 0)
                    mresults.flags |= 1u;
                if (!Mantle_CheckLedge(pm, pml, &mresults, 60.0) && !Mantle_CheckLedge(pm, pml, &mresults, 40.0))
                    Mantle_CheckLedge(pm, pml, &mresults, 20.0);
            }
        }
        else
        {
            Mantle_DebugPrint("Mantle Failed: Player is dead");
        }
    }
    else
    {
        Mantle_DebugPrint("Mantle Failed: Not enabled");
    }
}

void __cdecl Mantle_DebugPrint(const char *msg)
{
    if (!msg)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 185, 0, "%s", "msg");
    if (mantle_debug->current.enabled)
        Com_Printf(17, "%s\n", msg);
}

char __cdecl Mantle_CheckLedge(pmove_t *pm, pml_t *pml, MantleResults *mresults, float height)
{
    const char *v4; // eax
    float mins[3]; // [esp+18h] [ebp-60h] BYREF
    float start[3]; // [esp+24h] [ebp-54h] BYREF
    float end[3]; // [esp+30h] [ebp-48h] BYREF
    trace_t trace; // [esp+3Ch] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+68h] [ebp-10h] BYREF
    playerState_s *ps; // [esp+74h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 429, 0, "%s", "ps");
    v4 = va("Checking for ledge at %f units", height);
    Mantle_DebugPrint(v4);
    mins[0] = -(float)15.0;
    mins[1] = mins[0];
    mins[2] = 0.0;
    maxs[0] = 15.0;
    maxs[1] = 15.0;
    maxs[2] = (float)15.0 + (float)15.0;
    start[0] = mresults->startPos[0];
    start[1] = mresults->startPos[1];
    start[2] = mresults->startPos[2];
    start[2] = start[2] + height;
    Vec3Mad(start, 16.0, mresults->dir, end);
    PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, pm->tracemask);
    if (trace.startsolid || trace.fraction < 1.0)
    {
        Mantle_DebugPrint("Mantle Failed: Forward movement is blocked");
        return 0;
    }
    else
    {
        start[0] = end[0];
        start[1] = end[1];
        start[2] = end[2];
        end[2] = mresults->startPos[2] + 18.0;
        PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, pm->tracemask);
        if (trace.startsolid || trace.fraction == 1.0)
        {
            Mantle_DebugPrint("Mantle Failed: Can't find ledge");
            return 0;
        }
        else if (trace.walkable)
        {
            mresults->ledgePos[0] = end[0];
            mresults->ledgePos[1] = end[1];
            mresults->ledgePos[2] = end[2];
            mresults->ledgePos[2] = (end[2] - start[2]) * trace.fraction + start[2];
            if (mresults->ledgePos[2] - mresults->startPos[2] <= 0.0)
                MyAssertHandler(
                    ".\\bgame\\bg_mantle.cpp",
                    474,
                    0,
                    "%s\n\t((mresults->ledgePos[2] - mresults->startPos[2])) = %g",
                    "(((mresults->ledgePos[2] - mresults->startPos[2]) > 0.0f))",
                    mresults->ledgePos[2] - mresults->startPos[2]);
            maxs[2] = 50.0;
            PM_trace(pm, &trace, mresults->ledgePos, mins, maxs, mresults->ledgePos, ps->clientNum, pm->tracemask);
            if (trace.startsolid)
            {
                Mantle_DebugPrint("Mantle Failed: Player can't fit crouched on ledge");
                return 0;
            }
            else
            {
                ps->mantleState.flags |= 8u;
                mresults->flags |= 8u;
                Mantle_DebugPrint("Mantle available!");
                if ((pm->cmd.buttons & 0x400) != 0)
                {
                    Mantle_CalcEndPos(pm, mresults);
                    if ((ps->eFlags & 4) == 0)
                    {
                        PM_trace(
                            pm,
                            &trace,
                            mresults->ledgePos,
                            playerMins,
                            playerMaxs,
                            mresults->ledgePos,
                            ps->clientNum,
                            pm->tracemask);
                        if (trace.startsolid)
                            mresults->flags |= 2u;
                        PM_trace(
                            pm,
                            &trace,
                            mresults->endPos,
                            playerMins,
                            playerMaxs,
                            mresults->endPos,
                            ps->clientNum,
                            pm->tracemask);
                        if (!trace.startsolid)
                            mresults->flags |= 4u;
                    }
                    Mantle_Start(pm, ps, mresults);
                    return 1;
                }
                else
                {
                    return 1;
                }
            }
        }
        else
        {
            Mantle_DebugPrint("Mantle Failed: Ledge is too steep");
            return 0;
        }
    }
}

void __cdecl Mantle_CalcEndPos(pmove_t *pm, MantleResults *mresults)
{
    float mins[3]; // [esp+2Ch] [ebp-60h] BYREF
    float start[3]; // [esp+38h] [ebp-54h] BYREF
    float end[3]; // [esp+44h] [ebp-48h] BYREF
    trace_t trace; // [esp+50h] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+7Ch] [ebp-10h] BYREF
    playerState_s *ps; // [esp+88h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 335, 0, "%s", "ps");
    if ((mresults->flags & 1) != 0)
    {
        mins[0] = -15.0;
        mins[1] = -15.0;
        mins[2] = 0.0;
        maxs[0] = 15.0;
        maxs[1] = 15.0;
        maxs[2] = 50.0;
        start[0] = mresults->ledgePos[0];
        start[1] = mresults->ledgePos[1];
        start[2] = mresults->ledgePos[2];
        Vec3Mad(start, 31.0, mresults->dir, end);
        PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, pm->tracemask);
        if (trace.startsolid || trace.fraction < 1.0)
        {
            mresults->flags &= ~1u;
            mresults->endPos[0] = mresults->ledgePos[0];
            mresults->endPos[1] = mresults->ledgePos[1];
            mresults->endPos[2] = mresults->ledgePos[2];
        }
        else
        {
            start[0] = end[0];
            start[1] = end[1];
            start[2] = end[2];
            end[2] = end[2] - 18.0;
            PM_trace(pm, &trace, start, mins, maxs, end, ps->clientNum, pm->tracemask);
            if (trace.startsolid || trace.fraction < 1.0)
            {
                mresults->flags &= ~1u;
                mresults->endPos[0] = mresults->ledgePos[0];
                mresults->endPos[1] = mresults->ledgePos[1];
                mresults->endPos[2] = mresults->ledgePos[2];
            }
            else
            {
                mresults->endPos[0] = end[0];
                mresults->endPos[1] = end[1];
                mresults->endPos[2] = end[2];
                mresults->endPos[2] = (end[2] - start[2]) * trace.fraction + start[2];
            }
        }
    }
    else
    {
        mresults->endPos[0] = mresults->ledgePos[0];
        mresults->endPos[1] = mresults->ledgePos[1];
        mresults->endPos[2] = mresults->ledgePos[2];
    }
}

void __cdecl Mantle_Start(pmove_t *pm, playerState_s *ps, MantleResults *mresults)
{
    int UpLength; // esi
    float trans[3]; // [esp+14h] [ebp-14h] BYREF
    MantleState *mstate; // [esp+20h] [ebp-8h]
    int mantleTime; // [esp+24h] [ebp-4h]

    mstate = &ps->mantleState;
    ps->mantleState.yaw = vectoyaw(mresults->dir);
    mstate->timer = 0;
    mstate->transIndex = Mantle_FindTransition(mresults->startPos[2], mresults->ledgePos[2]);
    mstate->flags = mresults->flags;
    UpLength = Mantle_GetUpLength(mstate);
    mantleTime = Mantle_GetOverLength(mstate) + UpLength;
    Mantle_GetAnimDelta(mstate, mantleTime, trans);
    Vec3Sub(mresults->endPos, trans, ps->origin);
    if ((ps->pm_flags & 4) != 0)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 401, 0, "%s", "!( ps->pm_flags & PMF_MANTLE )");
    ps->pm_flags |= 4u;
    ps->eFlags |= 0x8000u;
    pm->mantleEndPos[0] = mresults->endPos[0];
    pm->mantleEndPos[1] = mresults->endPos[1];
    pm->mantleEndPos[2] = mresults->endPos[2];
    pm->mantleDuration = mantleTime;
    pm->mantleStarted = 1;
}

int __cdecl Mantle_GetUpLength(MantleState *mstate)
{
    return XAnimGetLengthMsec(s_mantleAnims, s_mantleTrans[mstate->transIndex].upAnimIndex);
}

int __cdecl Mantle_GetOverLength(MantleState *mstate)
{
    if ((mstate->flags & 1) != 0)
        return XAnimGetLengthMsec(s_mantleAnims, s_mantleTrans[mstate->transIndex].overAnimIndex);
    else
        return 0;
}

void __cdecl Mantle_GetAnimDelta(MantleState *mstate, int32_t time, float *delta)
{
    const char *v3; // eax
    float frac; // [esp+8h] [ebp-24h]
    float fraca; // [esp+8h] [ebp-24h]
    int32_t upTime; // [esp+Ch] [ebp-20h]
    float trans[3]; // [esp+10h] [ebp-1Ch] BYREF
    int32_t animIndex; // [esp+1Ch] [ebp-10h]
    float rot[2]; // [esp+20h] [ebp-Ch] BYREF
    int32_t overTime; // [esp+28h] [ebp-4h]

    upTime = Mantle_GetUpLength(mstate);
    overTime = Mantle_GetOverLength(mstate);
    if (time < 0 || time > overTime + upTime)
    {
        v3 = va("time: %i, upTime: %i, overTime: %i", time, upTime, overTime);
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 265, 0, "%s\n\t%s", "(time >= 0) && (time <= (upTime + overTime))", v3);
    }
    if (time > upTime)
    {
        fraca = (double)(time - upTime) / (double)overTime;
        animIndex = s_mantleTrans[mstate->transIndex].upAnimIndex;
        XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, trans, 1.0);
        animIndex = s_mantleTrans[mstate->transIndex].overAnimIndex;
        XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, fraca);
        Vec3Add(delta, trans, delta);
    }
    else
    {
        frac = (double)time / (double)upTime;
        animIndex = s_mantleTrans[mstate->transIndex].upAnimIndex;
        XAnimGetAbsDelta(s_mantleAnims, animIndex, rot, delta, frac);
    }
    VectorAngleMultiply(delta, mstate->yaw);
}

int __cdecl Mantle_FindTransition(float curHeight, float goalHeight)
{
    float v3; // [esp+8h] [ebp-24h]
    float v4; // [esp+Ch] [ebp-20h]
    float v5; // [esp+10h] [ebp-1Ch]
    float v6; // [esp+14h] [ebp-18h]
    float height; // [esp+1Ch] [ebp-10h]
    int bestIndex; // [esp+20h] [ebp-Ch]
    int transIndex; // [esp+24h] [ebp-8h]
    float bestDiff; // [esp+28h] [ebp-4h]

    height = goalHeight - curHeight;
    if (height <= 0.0)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 303, 0, "%s\n\t(height) = %g", "((height > 0))", height);
    bestIndex = 0;
    v6 = s_mantleTrans[0].height - height;
    v4 = I_fabs(v6);
    bestDiff = v4;
    for (transIndex = 1; transIndex < 7; ++transIndex)
    {
        v5 = s_mantleTrans[transIndex].height - height;
        v3 = I_fabs(v5);
        if (bestDiff > (double)v3)
        {
            bestIndex = transIndex;
            bestDiff = v3;
        }
    }
    return bestIndex;
}

char __cdecl Mantle_FindMantleSurface(pmove_t *pm, pml_t *pml, trace_t *trace, float *mantleDir)
{
    float scale; // [esp+0h] [ebp-A4h]
    float v6; // [esp+Ch] [ebp-98h]
    float v7; // [esp+10h] [ebp-94h]
    float v8; // [esp+14h] [ebp-90h]
    float v9; // [esp+44h] [ebp-60h]
    float traceDir[3]; // [esp+50h] [ebp-54h] BYREF
    float playerRadius; // [esp+5Ch] [ebp-48h]
    float mins[3]; // [esp+60h] [ebp-44h] BYREF
    float start[3]; // [esp+6Ch] [ebp-38h] BYREF
    float end[3]; // [esp+78h] [ebp-2Ch] BYREF
    float len; // [esp+84h] [ebp-20h]
    float maxs[3]; // [esp+88h] [ebp-1Ch] BYREF
    float innerDist; // [esp+94h] [ebp-10h]
    float traceDist; // [esp+98h] [ebp-Ch]
    playerState_s *ps; // [esp+9Ch] [ebp-8h]
    float dot; // [esp+A0h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 596, 0, "%s", "ps");
    playerRadius = 15.0;
    v9 = -mantle_check_radius->current.value;
    mins[0] = v9;
    mins[1] = v9;
    mins[2] = 0.0;
    maxs[0] = mantle_check_radius->current.value;
    maxs[1] = maxs[0];
    maxs[2] = 70.0;
    v8 = 70.0 - 0.0;
    if (v8 < maxs[0] - v9)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 604, 0, "%s", "(maxs[0] - mins[0]) <= (playerMaxs[2] - playerMins[2])");
    v7 = 70.0 - 0.0;
    if (v7 < maxs[1] - mins[1])
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 605, 0, "%s", "(maxs[1] - mins[1]) <= (playerMaxs[2] - playerMins[2])");
    innerDist = playerRadius - mantle_check_radius->current.value;
    traceDist = mantle_check_range->current.value + innerDist;
    traceDir[0] = pml->forward[0];
    traceDir[1] = pml->forward[1];
    traceDir[2] = 0.0;
    Vec3Normalize(traceDir);
    scale = -innerDist;
    Vec3Mad(ps->origin, scale, traceDir, start);
    Vec3Mad(ps->origin, traceDist, traceDir, end);
    PM_trace(pm, trace, start, mins, maxs, end, ps->clientNum, 0x1000000);
    if (trace->startsolid || trace->allsolid)
    {
        Mantle_DebugPrint("Mantle Failed: Mantle brush is too thick");
        return 0;
    }
    else if (trace->fraction == 1.0)
    {
        Mantle_DebugPrint("Mantle Failed: No mantle surface found");
        return 0;
    }
    else if ((trace->surfaceFlags & 0x6000000) != 0)
    {
        *mantleDir = -trace->normal[0];
        mantleDir[1] = -trace->normal[1];
        mantleDir[2] = -trace->normal[2];
        mantleDir[2] = 0.0;
        len = Vec3Normalize(mantleDir);
        if (len >= 0.00009999999747378752)
        {
            dot = Vec3Dot(traceDir, mantleDir);
            v6 = acos(dot);
            if (mantle_check_angle->current.value >= v6 * 57.2957763671875)
            {
                return 1;
            }
            else
            {
                Mantle_DebugPrint("Mantle Failed: Player is not facing mantle surface");
                return 0;
            }
        }
        else
        {
            Mantle_DebugPrint("Mantle Failed: Mantle surface has vertical normal");
            return 0;
        }
    }
    else
    {
        Mantle_DebugPrint("Mantle Failed: No mantle surface with MANTLE_ON or MANTLE_OVER found");
        return 0;
    }
}

void __cdecl Mantle_Move(pmove_t *pm, playerState_s *ps, pml_t *pml)
{
    int UpLength; // esi
    float scale; // [esp+Ch] [ebp-30h]
    float trans[3]; // [esp+10h] [ebp-2Ch] BYREF
    float prevTrans[3]; // [esp+1Ch] [ebp-20h] BYREF
    int prevTime; // [esp+28h] [ebp-14h]
    MantleState *mstate; // [esp+2Ch] [ebp-10h]
    int animIndex; // [esp+30h] [ebp-Ch]
    int deltaTime; // [esp+34h] [ebp-8h]
    int mantleLength; // [esp+38h] [ebp-4h]

    if ((ps->pm_flags & 4) == 0)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 745, 0, "%s", "ps->pm_flags & PMF_MANTLE");
    if (pml->msec < 0)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 746, 0, "%s\n\t(pml->msec) = %i", "(pml->msec >= 0)", pml->msec);
    if (mantle_enable->current.enabled)
    {
        mstate = &ps->mantleState;
        ps->mantleState.flags &= ~8u;
        if ((mstate->flags & 2) != 0)
            BG_AddPredictableEventToPlayerstate(7u, 0, ps);
        UpLength = Mantle_GetUpLength(mstate);
        mantleLength = Mantle_GetOverLength(mstate) + UpLength;
        prevTime = mstate->timer;
        mstate->timer += pml->msec;
        if (mstate->timer > mantleLength)
            mstate->timer = mantleLength;
        deltaTime = mstate->timer - prevTime;
        Mantle_GetAnimDelta(mstate, prevTime, prevTrans);
        Mantle_GetAnimDelta(mstate, mstate->timer, trans);
        animIndex = Mantle_GetAnim(mstate);
        BG_AnimScriptAnimation(ps, AISTATE_COMBAT, (scriptAnimMoveTypes_t)(animIndex + 21), 1);
        Vec3Sub(trans, prevTrans, trans);
        Vec3Add(trans, ps->origin, ps->origin);
        scale = 1.0 / ((double)deltaTime * EQUAL_EPSILON);
        Vec3Scale(trans, scale, ps->velocity);
        if (mstate->timer == mantleLength)
        {
            if ((ps->pm_flags & 4) == 0)
                MyAssertHandler(".\\bgame\\bg_mantle.cpp", 786, 0, "%s", "( ps->pm_flags & PMF_MANTLE )");
            ps->pm_flags &= ~4u;
            pm->mantleStarted = 0;
            if ((mstate->flags & 1) != 0)
                BG_AnimScriptEvent(ps, ANIM_ET_JUMP, 0, 1);
            if ((mstate->flags & 4) != 0)
            {
                BG_AddPredictableEventToPlayerstate(6u, 0, ps);
                ps->eFlags &= ~0x8000u;
            }
            mstate->flags |= 0x10u;
        }
    }
}

int __cdecl Mantle_GetAnim(MantleState *mstate)
{
    int upTime; // [esp+0h] [ebp-Ch]
    int overTime; // [esp+8h] [ebp-4h]

    upTime = Mantle_GetUpLength(mstate);
    overTime = Mantle_GetOverLength(mstate);
    if (mstate->timer < 0 || mstate->timer > overTime + upTime)
        MyAssertHandler(
            ".\\bgame\\bg_mantle.cpp",
            239,
            0,
            "%s",
            "(mstate->timer >= 0) && (mstate->timer <= (upTime + overTime))");
    if (mstate->timer > upTime)
        return s_mantleTrans[mstate->transIndex].overAnimIndex;
    else
        return s_mantleTrans[mstate->transIndex].upAnimIndex;
}

void __cdecl Mantle_CapView(playerState_s *ps)
{
    float v1; // [esp+8h] [ebp-10h]
    float value; // [esp+Ch] [ebp-Ch]
    float delta; // [esp+10h] [ebp-8h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 823, 0, "%s", "ps");
    if ((ps->pm_flags & 4) == 0)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 824, 0, "%s", "ps->pm_flags & PMF_MANTLE");
    if (mantle_enable->current.enabled)
    {
        delta = AngleDelta(ps->mantleState.yaw, ps->viewangles[1]);
        if (delta < -mantle_view_yawcap->current.value || mantle_view_yawcap->current.value < (double)delta)
        {
            while (delta < -mantle_view_yawcap->current.value)
                delta = delta + mantle_view_yawcap->current.value;
            while (mantle_view_yawcap->current.value < (double)delta)
                delta = delta - mantle_view_yawcap->current.value;
            if (delta <= 0.0)
                value = mantle_view_yawcap->current.value;
            else
                value = -mantle_view_yawcap->current.value;
            ps->delta_angles[1] = ps->delta_angles[1] + delta;
            v1 = ps->mantleState.yaw + value;
            ps->viewangles[1] = AngleNormalize360(v1);
        }
    }
}

void __cdecl Mantle_ClearHint(playerState_s *ps)
{
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_mantle.cpp", 852, 0, "%s", "ps");
    ps->mantleState.flags &= ~8u;
}

bool __cdecl Mantle_IsWeaponInactive(playerState_s *ps)
{
    if (!mantle_enable->current.enabled)
        return 0;
    if ((ps->pm_flags & 4) != 0)
        return s_mantleTrans[ps->mantleState.transIndex].overAnimIndex != 10;
    return 0;
}

void __cdecl Vec3Negate(const float *from, float *to)
{
    *to = -*from;
    to[1] = -from[1];
    to[2] = -from[2];
}

