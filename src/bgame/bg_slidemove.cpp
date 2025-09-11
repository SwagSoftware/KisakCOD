#include "bg_public.h"
#include "bg_local.h"
#include <aim_assist/aim_assist.h>
#include <universal/com_math.h>

void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int32_t gravity)
{
    float v3; // [esp+8h] [ebp-140h]
    float v4; // [esp+Ch] [ebp-13Ch]
    float v5; // [esp+14h] [ebp-134h]
    float v6; // [esp+1Ch] [ebp-12Ch]
    float v7; // [esp+20h] [ebp-128h]
    float v8; // [esp+30h] [ebp-118h]
    float *v9; // [esp+34h] [ebp-114h]
    float *v10; // [esp+3Ch] [ebp-10Ch]
    float *v11; // [esp+40h] [ebp-108h]
    float v12; // [esp+44h] [ebp-104h]
    float *v13; // [esp+4Ch] [ebp-FCh]
    float *v14; // [esp+50h] [ebp-F8h]
    float *v15; // [esp+54h] [ebp-F4h]
    float *v16; // [esp+58h] [ebp-F0h]
    float *velocity; // [esp+60h] [ebp-E8h]
    float *origin; // [esp+64h] [ebp-E4h]
    float v19; // [esp+6Ch] [ebp-DCh]
    float v20; // [esp+70h] [ebp-D8h]
    int32_t old; // [esp+84h] [ebp-C4h]
    float bobmove; // [esp+88h] [ebp-C0h]
    float fSpeedScale; // [esp+90h] [ebp-B8h]
    int32_t iDelta; // [esp+98h] [ebp-B0h]
    int32_t iDeltaa; // [esp+98h] [ebp-B0h]
    float flatDelta; // [esp+9Ch] [ebp-ACh]
    float flatDelta_4; // [esp+A0h] [ebp-A8h]
    float stepDelta; // [esp+A4h] [ebp-A4h]
    float stepDelta_4; // [esp+A8h] [ebp-A0h]
    float down_v; // [esp+ACh] [ebp-9Ch]
    float down_v_4; // [esp+B0h] [ebp-98h]
    float down_v_8; // [esp+B4h] [ebp-94h]
    int32_t jumping; // [esp+B8h] [ebp-90h]
    bool iBumps; // [esp+C0h] [ebp-88h]
    float start_o[3]; // [esp+C4h] [ebp-84h] BYREF
    trace_t trace; // [esp+D0h] [ebp-78h] BYREF
    float endpos[3]; // [esp+FCh] [ebp-4Ch] BYREF
    float start_v[3]; // [esp+108h] [ebp-40h] BYREF
    float down_o[3]; // [esp+114h] [ebp-34h] BYREF
    float up[3]; // [esp+120h] [ebp-28h] BYREF
    int bHadGround; // [esp+12Ch] [ebp-1Ch]
    float down[3]; // [esp+130h] [ebp-18h] BYREF
    playerState_s *ps; // [esp+13Ch] [ebp-Ch]
    float fStepSize; // [esp+140h] [ebp-8h] BYREF
    float fStepAmount; // [esp+144h] [ebp-4h]

    fStepAmount = 0.0;
    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_slidemove.cpp", 310, 0, "%s", "ps");
    jumping = 0;
    if ((ps->pm_flags & PMF_LADDER) != 0)
    {
        bHadGround = 0;
        Jump_ClearState(ps);
    }
    else if (pml->groundPlane)
    {
        bHadGround = 1;
    }
    else
    {
        bHadGround = 0;
        if ((ps->pm_flags & PMF_JUMPING) != 0 && ps->pm_time)
            Jump_ClearState(ps);
    }
    start_o[0] = ps->origin[0];
    start_o[1] = ps->origin[1];
    start_o[2] = ps->origin[2];
    start_v[0] = ps->velocity[0];
    start_v[1] = ps->velocity[1];
    start_v[2] = ps->velocity[2];
    iBumps = PM_SlideMove(pm, pml, gravity);
    if ((ps->pm_flags & PMF_PRONE) != 0)
        fStepSize = 10.0;
    else
        fStepSize = 18.0;
    if (ps->groundEntityNum != ENTITYNUM_NONE)
        goto LABEL_26;
    if ((ps->pm_flags & PMF_JUMPING) != 0 && ps->pm_time)
        Jump_ClearState(ps);
    if (iBumps && (ps->pm_flags & PMF_JUMPING) != 0 && Jump_GetStepHeight(ps, start_o, &fStepSize))
    {
        if (fStepSize < 1.0)
            return;
        jumping = 1;
    }
    if (jumping || (ps->pm_flags & PMF_LADDER) != 0 && ps->velocity[2] > 0.0)
    {
    LABEL_26:
        down_o[0] = ps->origin[0];
        down_o[1] = ps->origin[1];
        down_o[2] = ps->origin[2];
        down_v = ps->velocity[0];
        down_v_4 = ps->velocity[1];
        down_v_8 = ps->velocity[2];
        flatDelta = down_o[0] - start_o[0];
        flatDelta_4 = down_o[1] - start_o[1];
        if (iBumps || pml->groundPlane && pml->groundTrace.normal[2] < 0.8999999761581421)
        {
            up[0] = start_o[0];
            up[1] = start_o[1];
            up[2] = fStepSize + 1.0 + start_o[2];
            PM_playerTrace(pm, &trace, start_o, pm->mins, pm->maxs, up, ps->clientNum, pm->tracemask);
            fStepAmount = (fStepSize + 1.0) * trace.fraction - 1.0;
            if (fStepAmount >= 1.0)
            {
                origin = ps->origin;
                v19 = up[1];
                v20 = fStepAmount + start_o[2];
                ps->origin[0] = up[0];
                origin[1] = v19;
                origin[2] = v20;
                velocity = ps->velocity;
                ps->velocity[0] = start_v[0];
                velocity[1] = start_v[1];
                velocity[2] = start_v[2];
                PM_SlideMove(pm, pml, gravity);
            }
            else
            {
                fStepAmount = 0.0;
            }
        }
        if (bHadGround || fStepAmount != 0.0)
        {
            down[0] = ps->origin[0];
            down[1] = ps->origin[1];
            down[2] = ps->origin[2];
            down[2] = down[2] - fStepAmount;
            if (bHadGround)
                down[2] = down[2] - 9.0;
            PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, down, ps->clientNum, pm->tracemask);
            if (Trace_GetEntityHitId(&trace) < 64u)
            {
                v16 = ps->origin;
                ps->origin[0] = down_o[0];
                v16[1] = down_o[1];
                v16[2] = down_o[2];
                v15 = ps->velocity;
                ps->velocity[0] = down_v;
                v15[1] = down_v_4;
                v15[2] = down_v_8;
                return;
            }
            if (trace.fraction >= 1.0)
            {
                if (fStepAmount != 0.0)
                    ps->origin[2] = ps->origin[2] - fStepAmount;
            }
            else
            {
                if (!trace.walkable && trace.normal[2] < 0.300000011920929)
                {
                    v14 = ps->origin;
                    ps->origin[0] = down_o[0];
                    v14[1] = down_o[1];
                    v14[2] = down_o[2];
                    v13 = ps->velocity;
                    ps->velocity[0] = down_v;
                    v13[1] = down_v_4;
                    v13[2] = down_v_8;
                    return;
                }
                Vec3Lerp(ps->origin, down, trace.fraction, ps->origin);
                PM_ProjectVelocity(ps->velocity, trace.normal, ps->velocity);
            }
        }
        stepDelta = ps->origin[0] - start_o[0];
        stepDelta_4 = ps->origin[1] - start_o[1];
        v12 = stepDelta * start_v[0] + stepDelta_4 * start_v[1];
        v5 = start_v[1] * flatDelta_4 + start_v[0] * flatDelta;
        if (v12 <= v5 + EQUAL_EPSILON || jumping && Jump_IsPlayerAboveMax(ps))
        {
            v11 = ps->origin;
            ps->origin[0] = down_o[0];
            v11[1] = down_o[1];
            v11[2] = down_o[2];
            v10 = ps->velocity;
            ps->velocity[0] = down_v;
            v10[1] = down_v_4;
            v10[2] = down_v_8;
            fStepAmount = 0.0;
            if (bHadGround)
            {
                down[0] = ps->origin[0];
                down[1] = ps->origin[1];
                down[2] = ps->origin[2];
                down[2] = down[2] - 9.0;
                PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, down, ps->clientNum, pm->tracemask);
                if (trace.fraction < 1.0)
                {
                    Vec3Lerp(ps->origin, down, trace.fraction, endpos);
                    fStepAmount = endpos[2] - ps->origin[2];
                    v9 = ps->origin;
                    ps->origin[0] = endpos[0];
                    v9[1] = endpos[1];
                    v9[2] = endpos[2];
                    PM_ClipVelocity(ps->velocity, trace.normal, ps->velocity);
                }
            }
        }
        if (jumping)
            Jump_ClampVelocity(ps, down_o);
        if (bHadGround)
        {
            if (ps->pm_type < PM_DEAD)
            {
                if (PM_VerifyPronePosition(pm, start_o, start_v))
                {
                    v8 = ps->origin[2] - down_o[2];
                    v4 = I_fabs(v8);
                    if (v4 > 0.5)
                    {
                        iDelta = (int)(ps->origin[2] - down_o[2]);
                        if (iDelta)
                        {
                            if (pm->viewChangeTime < ps->commandTime)
                            {
                                pm->viewChange = ps->origin[2] - down_o[2] + pm->viewChange;
                                pm->viewChangeTime = ps->commandTime;
                            }
                            v6 = ps->origin[2] - start_o[2];
                            v3 = I_fabs(v6);
                            fSpeedScale = 1.0 - (float)0.80000001 + (1.0 - v3 / fStepSize) * (float)0.80000001;
                            Vec3Scale(ps->velocity, fSpeedScale, ps->velocity);
                            pm->xyspeed = Vec2Length(ps->velocity);
                            if ((int)abs(iDelta) > 3 && ps->groundEntityNum != ENTITYNUM_NONE && PM_ShouldMakeFootsteps(pm))
                            {
                                iDeltaa = (int)abs(iDelta) / 2;
                                if (iDeltaa > 4)
                                    iDeltaa = 4;
                                bobmove = (double)iDeltaa * 1.25 + 7.0;
                                old = ps->bobCycle;
                                ps->bobCycle = (unsigned __int8)(int)((double)old + bobmove);
                                PM_FootstepEvent(pm, pml, old, ps->bobCycle, 1);
                            }
                        }
                    }
                }
            }
        }
    }
}

int __cdecl PM_VerifyPronePosition(pmove_t *pm, float *vFallbackOrg, float *vFallbackVel)
{
    int result; // eax
    playerState_s *ps; // [esp+38h] [ebp-4h]

    ps = pm->ps;
    if (!pm->ps)
        MyAssertHandler(".\\bgame\\bg_slidemove.cpp", 30, 0, "%s", "ps");
    if ((ps->pm_flags & PMF_PRONE) == 0)
        return 1;
    result = BG_CheckProne(
        ps->clientNum,
        ps->origin,
        15.0,
        30.0,
        ps->proneDirection,
        &ps->fTorsoPitch,
        &ps->fWaistPitch,
        1,
        1,
        1,
        pm->handler,
        PCT_CLIENT,
        50.0);
    if (!(_BYTE)result)
    {
        ps->origin[0] = *vFallbackOrg;
        ps->origin[1] = vFallbackOrg[1];
        ps->origin[2] = vFallbackOrg[2];
        ps->velocity[0] = *vFallbackVel;
        ps->velocity[1] = vFallbackVel[1];
        ps->velocity[2] = vFallbackVel[2];
    }
    return (unsigned __int8)result;
}

bool __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int32_t gravity)
{
    uint16_t EntityHitId; // ax
    float *v5; // [esp+Ch] [ebp-150h]
    float *v6; // [esp+10h] [ebp-14Ch]
    float *v7; // [esp+14h] [ebp-148h]
    float *v8; // [esp+18h] [ebp-144h]
    float *v9; // [esp+28h] [ebp-134h]
    float *velocity; // [esp+2Ch] [ebp-130h]
    int32_t j; // [esp+3Ch] [ebp-120h]
    float dir[3]; // [esp+40h] [ebp-11Ch] BYREF
    float d; // [esp+4Ch] [ebp-110h]
    int32_t numbumps; // [esp+50h] [ebp-10Ch]
    float endClipVelocity[3]; // [esp+54h] [ebp-108h] BYREF
    int32_t k; // [esp+60h] [ebp-FCh]
    float planes[8][3]; // [esp+64h] [ebp-F8h] BYREF
    int32_t permutation[8]; // [esp+C8h] [ebp-94h] BYREF
    float time_left; // [esp+E8h] [ebp-74h]
    float end[3]; // [esp+ECh] [ebp-70h] BYREF
    int32_t numplanes; // [esp+F8h] [ebp-64h]
    int32_t bumpcount; // [esp+FCh] [ebp-60h]
    float primal_velocity[3]; // [esp+100h] [ebp-5Ch]
    trace_t trace; // [esp+10Ch] [ebp-50h] BYREF
    float endVelocity[3]; // [esp+138h] [ebp-24h] BYREF
    float clipVelocity[3]; // [esp+144h] [ebp-18h] BYREF
    int32_t i; // [esp+150h] [ebp-Ch]
    playerState_s *ps; // [esp+154h] [ebp-8h]
    float into; // [esp+158h] [ebp-4h]

    ps = pm->ps;
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_slidemove.cpp", 112, 0, "%s", "ps");
    numbumps = 4;
    primal_velocity[0] = ps->velocity[0];
    primal_velocity[1] = ps->velocity[1];
    primal_velocity[2] = ps->velocity[2];
    endVelocity[0] = ps->velocity[0];
    endVelocity[1] = ps->velocity[1];
    endVelocity[2] = ps->velocity[2];
    if (gravity)
    {
        endVelocity[2] = endVelocity[2] - (double)ps->gravity * pml->frametime;
        ps->velocity[2] = (ps->velocity[2] + endVelocity[2]) * 0.5;
        primal_velocity[2] = endVelocity[2];
        if (pml->groundPlane)
            PM_ClipVelocity(ps->velocity, pml->groundTrace.normal, ps->velocity);
    }
    time_left = pml->frametime;
    if (pml->groundPlane)
    {
        planes[0][0] = pml->groundTrace.normal[0];
        planes[0][1] = pml->groundTrace.normal[1];
        planes[0][2] = pml->groundTrace.normal[2];
        numplanes = 1;
    }
    else
    {
        numplanes = 0;
    }
    Vec3NormalizeTo(ps->velocity, planes[numplanes++]);
    for (bumpcount = 0; bumpcount < numbumps; ++bumpcount)
    {
        Vec3Mad(ps->origin, time_left, ps->velocity, end);
        PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, end, ps->clientNum, pm->tracemask);
        if (trace.allsolid)
        {
            ps->velocity[2] = 0.0;
            return 1;
        }
        if (trace.fraction > 0.0)
            Vec3Lerp(ps->origin, end, trace.fraction, ps->origin);
        if (trace.fraction == 1.0)
            break;
        EntityHitId = Trace_GetEntityHitId(&trace);
        PM_AddTouchEnt(pm, EntityHitId);
        time_left = time_left - time_left * trace.fraction;
        if (numplanes >= 8)
        {
            velocity = ps->velocity;
            ps->velocity[0] = 0.0;
            velocity[1] = 0.0;
            velocity[2] = 0.0;
            return 1;
        }
        for (i = 0; i < numplanes; ++i)
        {
            if (Vec3Dot(trace.normal, planes[i]) > 0.9990000128746033)
            {
                PM_ClipVelocity(ps->velocity, trace.normal, ps->velocity);
                Vec3Add(trace.normal, ps->velocity, ps->velocity);
                break;
            }
        }
        if (i >= numplanes)
        {
            v9 = planes[numplanes];
            *v9 = trace.normal[0];
            v9[1] = trace.normal[1];
            v9[2] = trace.normal[2];
            into = PM_PermuteRestrictiveClipPlanes(ps->velocity, ++numplanes, planes, permutation);
            if (into < 0.1000000014901161)
            {
                if (pml->impactSpeed < -into)
                    pml->impactSpeed = -into;
                PM_ClipVelocity(ps->velocity, planes[permutation[0]], clipVelocity);
                PM_ClipVelocity(endVelocity, planes[permutation[0]], endClipVelocity);
                for (j = 1; j < numplanes; ++j)
                {
                    if (Vec3Dot(clipVelocity, planes[permutation[j]]) < 0.1000000014901161)
                    {
                        PM_ClipVelocity(clipVelocity, planes[permutation[j]], clipVelocity);
                        PM_ClipVelocity(endClipVelocity, planes[permutation[j]], endClipVelocity);
                        if (Vec3Dot(clipVelocity, planes[permutation[0]]) < 0.0)
                        {
                            Vec3Cross(planes[permutation[0]], planes[permutation[j]], dir);
                            Vec3Normalize(dir);
                            d = Vec3Dot(dir, ps->velocity);
                            Vec3Scale(dir, d, clipVelocity);
                            d = Vec3Dot(dir, endVelocity);
                            Vec3Scale(dir, d, endClipVelocity);
                            for (k = 1; k < numplanes; ++k)
                            {
                                if (k != j && Vec3Dot(clipVelocity, planes[permutation[k]]) < 0.1000000014901161)
                                {
                                    v8 = ps->velocity;
                                    ps->velocity[0] = 0.0;
                                    v8[1] = 0.0;
                                    v8[2] = 0.0;
                                    return 1;
                                }
                            }
                        }
                    }
                }
                v7 = ps->velocity;
                ps->velocity[0] = clipVelocity[0];
                v7[1] = clipVelocity[1];
                v7[2] = clipVelocity[2];
                endVelocity[0] = endClipVelocity[0];
                endVelocity[1] = endClipVelocity[1];
                endVelocity[2] = endClipVelocity[2];
            }
        }
    }
    if (gravity)
    {
        v6 = ps->velocity;
        ps->velocity[0] = endVelocity[0];
        v6[1] = endVelocity[1];
        v6[2] = endVelocity[2];
    }
    if (ps->pm_time)
    {
        v5 = ps->velocity;
        ps->velocity[0] = primal_velocity[0];
        v5[1] = primal_velocity[1];
        v5[2] = primal_velocity[2];
    }
    return bumpcount != 0;
}

double __cdecl PM_PermuteRestrictiveClipPlanes(
    const float *velocity,
    int32_t planeCount,
    const float (*planes)[3],
    int32_t*permutation)
{
    double v4; // st7
    int32_t permutedIndex; // [esp+4h] [ebp-28h]
    float parallel[8]; // [esp+8h] [ebp-24h]
    int32_t planeIndex; // [esp+28h] [ebp-4h]

    if (!velocity)
        MyAssertHandler(".\\bgame\\bg_slidemove.cpp", 57, 0, "%s", "velocity");
    if (planeCount <= 0 || planeCount > 8)
        MyAssertHandler(
            ".\\bgame\\bg_slidemove.cpp",
            58,
            0,
            "%s\n\t(planeCount) = %i",
            "(planeCount > 0 && planeCount <= 8)",
            planeCount);
    if (!planes)
        MyAssertHandler(".\\bgame\\bg_slidemove.cpp", 59, 0, "%s", "planes");
    if (!permutation)
        MyAssertHandler(".\\bgame\\bg_slidemove.cpp", 60, 0, "%s", "permutation");
    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        v4 = Vec3Dot(velocity, &(*planes)[3 * planeIndex]);
        parallel[planeIndex] = v4;
        for (permutedIndex = planeIndex;
            permutedIndex && parallel[planeIndex] <= (double)parallel[permutation[permutedIndex - 1]];
            --permutedIndex)
        {
            permutation[permutedIndex] = permutation[permutedIndex - 1];
        }
        permutation[permutedIndex] = planeIndex;
    }
    return parallel[*permutation];
}

