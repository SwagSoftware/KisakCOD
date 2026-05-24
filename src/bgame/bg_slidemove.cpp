#include "bg_public.h"
#include "bg_local.h"
#include <aim_assist/aim_assist.h>
#include <universal/com_math.h>

#define	MAX_CLIP_PLANES	8

void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int32_t gravity)
{
    float v3; // [esp+8h] [ebp-140h]
    float v4; // [esp+Ch] [ebp-13Ch]
    float v5; // [esp+14h] [ebp-134h]
    float v6; // [esp+1Ch] [ebp-12Ch]
    float v7; // [esp+20h] [ebp-128h]
    float v8; // [esp+30h] [ebp-118h]
    float v12; // [esp+44h] [ebp-104h]
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
    float down_v[3]; // [esp+ACh] [ebp-9Ch]
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
    iassert(ps);

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

    Vec3Copy(ps->origin, start_o);
    Vec3Copy(ps->velocity, start_v);

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
        Vec3Copy(ps->origin, down_o);
        Vec3Copy(ps->velocity, down_v);
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
            if (Trace_GetEntityHitId(&trace) < MAX_CLIENTS)
            {
                Vec3Copy(down_o, ps->origin);
                Vec3Copy(down_v, ps->velocity);
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
                    Vec3Copy(down_o, ps->origin);
                    Vec3Copy(down_v, ps->velocity);
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
            Vec3Copy(down_o, ps->origin);
            Vec3Copy(down_v, ps->velocity);

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
                    Vec3Copy(endpos, ps->origin);
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
                        iDelta = SnapFloatToInt(ps->origin[2] - down_o[2]);                        if (iDelta)
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
                                ps->bobCycle = (uint8_t)(int)((double)old + bobmove);
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

    playerState_s* ps = pm->ps;
    iassert(ps);

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

    return (uint8_t)result;
}

/*
==================
PM_SlideMove

Returns qtrue if the velocity was clipped in some way
==================
*/
bool __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int32_t gravity)
{
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
    iassert(ps);

    numbumps = 4;

    Vec3Copy(ps->velocity, primal_velocity);

    endVelocity[0] = ps->velocity[0];
    endVelocity[1] = ps->velocity[1];
    endVelocity[2] = ps->velocity[2];

    if (gravity)
    {
        endVelocity[2] = endVelocity[2] - (double)ps->gravity * pml->frametime;
        ps->velocity[2] = (ps->velocity[2] + endVelocity[2]) * 0.5;
        primal_velocity[2] = endVelocity[2];
        if (pml->groundPlane)
        {
            // slide along the ground plane
            PM_ClipVelocity(ps->velocity, pml->groundTrace.normal, ps->velocity);
        }
    }

    time_left = pml->frametime;

    // never turn against the ground plane
    if (pml->groundPlane)
    {
        numplanes = 1;

        Vec3Copy(pml->groundTrace.normal, planes[0]);
        planes[0][0] = pml->groundTrace.normal[0];
        planes[0][1] = pml->groundTrace.normal[1];
        planes[0][2] = pml->groundTrace.normal[2];
    }
    else
    {
        numplanes = 0;
    }

    // never turn against original velocity
    Vec3NormalizeTo(ps->velocity, planes[numplanes]);
    numplanes++;

    for (bumpcount = 0; bumpcount < numbumps; ++bumpcount)
    {
        // calculate position we are trying to move to
        Vec3Mad(ps->origin, time_left, ps->velocity, end);

        // see if we can make it there
        PM_playerTrace(pm, &trace, ps->origin, pm->mins, pm->maxs, end, ps->clientNum, pm->tracemask);

        // entity is completely trapped in another solid
        if (trace.allsolid)
        {
            ps->velocity[2] = 0.0;// don't build up falling damage, but allow sideways acceleration
            return true;
        }

        // actually covered some distance
        if (trace.fraction > 0.0)
        {
            Vec3Lerp(ps->origin, end, trace.fraction, ps->origin);
        }

        if (trace.fraction == 1.0)
        {
            break; // moved the entire distance
        }

        // save entity for contact
        PM_AddTouchEnt(pm, Trace_GetEntityHitId(&trace));

        time_left -= time_left * trace.fraction;

        if (numplanes >= MAX_CLIP_PLANES)
        { // this shouldn't really happen
            Vec3Clear(ps->velocity);
            return true;
        }

        // no sliding if stuck to wall
        for (i = 0; i < numplanes; ++i)
        {
            if (Vec3Dot(trace.normal, planes[i]) > 0.999)
            {
                PM_ClipVelocity(ps->velocity, trace.normal, ps->velocity);
                Vec3Add(trace.normal, ps->velocity, ps->velocity);
                break;
            }
        }

        // modify velocity so it parallels all of the clip planes

        // find a plane that it enters
        if (i >= numplanes)
        {
            planes[numplanes][0] = trace.normal[0];
            planes[numplanes][1] = trace.normal[1];
            planes[numplanes][2] = trace.normal[2];
            into = PM_PermuteRestrictiveClipPlanes(ps->velocity, ++numplanes, planes, permutation);
            if (into < 0.1000000014901161)
            {
                // see how hard we are hitting things
                if (pml->impactSpeed < -into)
                {
                    pml->impactSpeed = -into;
                }
                
                // slide along the plane
                PM_ClipVelocity(ps->velocity, planes[permutation[0]], clipVelocity);
                // slide along the plane
                PM_ClipVelocity(endVelocity, planes[permutation[0]], endClipVelocity);

                // see if there is a second plane that the new move enters
                for (j = 1; j < numplanes; ++j)
                {
                    if (Vec3Dot(clipVelocity, planes[permutation[j]]) >= 0.1)
                    {
                        continue;
                    }

                    // try clipping the move to the plane
                    PM_ClipVelocity(clipVelocity, planes[permutation[j]], clipVelocity);
                    PM_ClipVelocity(endClipVelocity, planes[permutation[j]], endClipVelocity);

                    // see if it goes back into the first clip plane
                    if (Vec3Dot(clipVelocity, planes[permutation[0]]) >= 0)
                    {
                        continue;
                    }

                    // slide the original velocity along the crease
                    Vec3Cross(planes[permutation[0]], planes[permutation[j]], dir);
                    Vec3Normalize(dir);
                    d = Vec3Dot(dir, ps->velocity);
                    Vec3Scale(dir, d, clipVelocity);

                    d = Vec3Dot(dir, endVelocity);
                    Vec3Scale(dir, d, endClipVelocity);

                    // see if there is a third plane that the new move enters
                    for (k = 1; k < numplanes; ++k)
                    {
                        if (k == j)
                        {
                            continue;
                        }
                        if (Vec3Dot(clipVelocity, planes[permutation[k]]) >= 0.1)
                        {
                            continue;
                        }

                        // stop dead at a triple plane intersection
                        Vec3Clear(ps->velocity);
                        return true;
                    }
                }

                // if we have fixed all interactions, try another move
                Vec3Copy(clipVelocity, ps->velocity);
                Vec3Copy(endClipVelocity, endVelocity);
            }
        }
    }

    if (gravity)
    {
        Vec3Copy(endVelocity, ps->velocity);
    }
    if (ps->pm_time)
    {
        Vec3Copy(primal_velocity, ps->velocity);
    }

    return bumpcount != 0;
}

double __cdecl PM_PermuteRestrictiveClipPlanes(
    const float *velocity,
    int32_t planeCount,
    const float (*planes)[3],
    int32_t*permutation)
{
    int32_t permutedIndex; // [esp+4h] [ebp-28h]
    float parallel[8]; // [esp+8h] [ebp-24h]
    int32_t planeIndex; // [esp+28h] [ebp-4h]

    iassert(velocity);
    iassert(planeCount > 0 && planeCount <= 8);
    iassert(planes);
    iassert(permutation);

    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        parallel[planeIndex] = Vec3Dot(velocity, &(*planes)[3 * planeIndex]);
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

