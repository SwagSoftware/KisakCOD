#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include "actor_physics.h"
#include <qcommon/mem_track.h>
#include <universal/q_shared.h>
#include <universal/com_math.h>
#include "g_local.h"
#include "g_main.h"
#include "actor_events.h"
#include "sentient.h"
#include <universal/profile.h>

struct actor_physics_local_t
{
    float fFrameTime;
    int bIsWalking;
    int bGroundPlane;
    trace_t groundTrace;
    float fImpactSpeed;
    float vPrevOrigin[3];
    float vPrevVelocity[3];
    int iTraceMask;
    float stepheight;
};

actor_physics_local_t g_apl;
actor_physics_t *g_pPhys;

void __cdecl TRACK_actor_physics()
{
    track_static_alloc_internal(&g_apl, 92, "g_apl", 5);
}

void __cdecl AIPhys_AddTouchEnt(int entityNum)
{
    int i; // [esp+0h] [ebp-4h]

    if (entityNum != ENTITYNUM_WORLD && g_pPhys->iNumTouch != 32)
    {
        for (i = 0; i < g_pPhys->iNumTouch; ++i)
        {
            if (g_pPhys->iTouchEnts[i] == entityNum)
                return;
        }
        g_pPhys->iTouchEnts[g_pPhys->iNumTouch++] = entityNum;
    }
}

void __cdecl AIPhys_ClipVelocity(
    const float *in,
    const float *normal,
    bool isWalkable,
    float *out,
    float overbounce)
{

    int i; // [esp+4h] [ebp-8h]
    float backoff; // [esp+8h] [ebp-4h]

    if (isWalkable && (float)((float)(*in * *in) + (float)(in[1] * in[1])) >= (float)(in[2] * in[2]))
    {
        out[2] = (-(*in) * *normal) - (float)(in[1] * normal[1]);
        *out = *in * normal[2];
        out[1] = in[1] * normal[2];
    }
    else
    {
        backoff = (float)((float)(*in * *normal) + (float)(in[1] * normal[1])) + (float)(in[2] * normal[2]);
        if (backoff >= 0.0)
            backoff = backoff / overbounce;
        else
            backoff = backoff * overbounce;
        for (i = 0; i < 3; ++i)
            out[i] = in[i] - (float)(normal[i] * backoff);
    }

    //double v5; // fp0
    //
    //if (isWalkable && (float)((float)(*in * *in) + (float)(in[1] * in[1])) >= (double)(float)(in[2] * in[2]))
    //{
    //    out[2] = -(float)((float)(normal[1] * in[1]) - (float)-(float)(*normal * *in));
    //    *out = *in * normal[2];
    //    out[1] = in[1] * normal[2];
    //}
    //else
    //{
    //    if ((float)((float)(in[1] * normal[1]) + (float)((float)(*normal * *in) + (float)(in[2] * normal[2]))) >= 0.0)
    //        v5 = (float)((float)((float)(in[1] * normal[1]) + (float)((float)(*normal * *in) + (float)(in[2] * normal[2])))
    //            / (float)overbounce);
    //    else
    //        v5 = (float)((float)((float)(in[1] * normal[1]) + (float)((float)(*normal * *in) + (float)(in[2] * normal[2])))
    //            * (float)overbounce);
    //    *out = -(float)((float)(*normal * (float)v5) - *in);
    //    out[1] = -(float)((float)(normal[1] * (float)v5) - in[1]);
    //    out[2] = -(float)((float)((float)v5 * normal[2]) - in[2]);
    //}
}

SlideMoveResult __cdecl AIPhys_SlideMove(int gravity, int zonly)
{
    float fTimeLeft; // fp20
    int iNumPlanes; // r31
    trace_t trace;

    float vEndVelocity[3];
    float vPrimalVelocity[3];
    float vEnd[3];

    bool isWalkable[8];
    float planes[5][3];

    vPrimalVelocity[0] = g_pPhys->vVelocity[0];
    vPrimalVelocity[1] = g_pPhys->vVelocity[1];
    vPrimalVelocity[2] = g_pPhys->vVelocity[2];
    vEndVelocity[0] = g_pPhys->vVelocity[0];
    vEndVelocity[1] = g_pPhys->vVelocity[1];
    vEndVelocity[2] = g_pPhys->vVelocity[2];

    if (gravity)
    {
        vEndVelocity[2] = vEndVelocity[2] - (float)(g_pPhys->fGravity * g_apl.fFrameTime);
        g_pPhys->vVelocity[2] = (float)(g_pPhys->vVelocity[2] + vEndVelocity[2]) * 0.5f;
        vPrimalVelocity[2] = vEndVelocity[2];
        if (g_apl.bGroundPlane)
        {
            if (!zonly)
                AIPhys_ClipVelocity(g_pPhys->vVelocity, g_apl.groundTrace.normal, g_apl.groundTrace.walkable, g_pPhys->vVelocity, 1.001);
        }
    }

    fTimeLeft = g_apl.fFrameTime;

    if (g_apl.bGroundPlane)
    {
        iNumPlanes = 1;

        planes[0][0] = g_apl.groundTrace.normal[0];
        planes[0][1] = g_apl.groundTrace.normal[1];
        planes[0][2] = g_apl.groundTrace.normal[2];

        isWalkable[0] = g_apl.groundTrace.walkable;
    }
    else
    {
        iNumPlanes = 0;
    }

    Vec3NormalizeTo(g_pPhys->vVelocity, planes[iNumPlanes]);

    isWalkable[iNumPlanes] = g_pPhys->vVelocity[2] >= 0.7f;
    iNumPlanes++;

    static const int iMaxBumps = 4;

    int i;
    int iBumpCount;

    for (iBumpCount = 0; ; ++iBumpCount)
    {
        if (iBumpCount >= iMaxBumps)
        {
        LABEL_66:
            if (gravity)
            {
                g_pPhys->vVelocity[0] = vEndVelocity[0];
                g_pPhys->vVelocity[0] = vEndVelocity[0];
                g_pPhys->vVelocity[0] = vEndVelocity[0];
            }
            return (SlideMoveResult)(iBumpCount != 0);
        }

        vEnd[0] = (float)(fTimeLeft * g_pPhys->vVelocity[0]) + g_pPhys->vOrigin[0];
        vEnd[1] = (float)(fTimeLeft * g_pPhys->vVelocity[1]) + g_pPhys->vOrigin[1];
        vEnd[2] = (float)(fTimeLeft * g_pPhys->vVelocity[2]) + g_pPhys->vOrigin[2];

        G_TraceCapsule(&trace, g_pPhys->vOrigin, g_pPhys->vMins, g_pPhys->vMaxs, vEnd, g_pPhys->iEntNum, g_apl.iTraceMask);

        if (trace.fraction <= 0.0f)
        {
            break;
        }

        Vec3Lerp(g_pPhys->vOrigin, vEnd, trace.fraction, g_pPhys->vOrigin);
        if (trace.fraction == 1.0f)
        {
            goto LABEL_66;
        }

    LABEL_23:
        g_pPhys->bDeflected = 1;
        if (!trace.walkable && g_pPhys->iHitEntnum == ENTITYNUM_NONE && (trace.normal[0] != 0.0f || trace.normal[1] != 0.0f))
        {
            g_pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
            g_pPhys->vHitOrigin[0] = g_pPhys->vOrigin[0];
            g_pPhys->vHitOrigin[1] = g_pPhys->vOrigin[1];
            g_pPhys->vHitNormal[0] = trace.normal[0];
            g_pPhys->vHitNormal[1] = trace.normal[1];
            g_pPhys->bStuck = 0;
        }

        AIPhys_AddTouchEnt(Trace_GetEntityHitId(&trace));
        fTimeLeft -= (fTimeLeft * trace.fraction);

        if (iNumPlanes >= 5)
        {
            g_pPhys->vVelocity[0] = 0.0f;
            g_pPhys->vVelocity[1] = 0.0f;
            g_pPhys->vVelocity[2] = 0.0f;
            return SLIDEMOVE_CLIPPED;
        }

        for (i = 0; i < iNumPlanes; i++)
        {
            if ((trace.normal[0] * planes[i][0])
                + (trace.normal[1] * planes[i][1])
                + (trace.normal[2] * planes[i][2]) > 0.99f)
            {
                if (trace.fraction == 0.0f && g_pPhys->iHitEntnum == ENTITYNUM_NONE && (trace.normal[0] != 0.0f || trace.normal[1] != 0.0f))
                {
                    g_pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
                    g_pPhys->vHitOrigin[0] = g_pPhys->vOrigin[0];
                    g_pPhys->vHitOrigin[1] = g_pPhys->vOrigin[1];
                    g_pPhys->vHitNormal[0] = trace.normal[0];
                    g_pPhys->vHitNormal[1] = trace.normal[1];
                    g_pPhys->bStuck = 0;
                }

                g_pPhys->vVelocity[0] = trace.normal[0] + g_pPhys->vVelocity[0];
                g_pPhys->vVelocity[1] = trace.normal[1] + g_pPhys->vVelocity[1];
                g_pPhys->vVelocity[2] = trace.normal[2] + g_pPhys->vVelocity[2];
                break;
            }
        }

        if (i >= iNumPlanes)
        {
            planes[iNumPlanes][0] = trace.normal[0];
            planes[iNumPlanes][1] = trace.normal[1];
            planes[iNumPlanes][2] = trace.normal[2];
            isWalkable[iNumPlanes++] = trace.walkable;

            for (i = 0; i < iNumPlanes; ++i)
            {
                float fInto = (g_pPhys->vVelocity[0] * planes[i][0]) + (g_pPhys->vVelocity[1] * planes[i][1]) + (g_pPhys->vVelocity[2] * planes[i][2]);

                float vDir[3];

                if (fInto < 0.1f)
                {
                    if (-fInto > g_apl.fImpactSpeed)
                    {
                        g_apl.fImpactSpeed = -fInto;
                    }

                    float vClipVelocity[3];
                    float vEndClipVelocity[3];

                    AIPhys_ClipVelocity(g_pPhys->vVelocity, planes[i], isWalkable[i], vClipVelocity, 1.001f);
                    AIPhys_ClipVelocity(vEndVelocity, planes[i], isWalkable[i], vEndClipVelocity, 1.001f);

                    for (int j = 0; j < iNumPlanes; ++j)
                    {
                        if (j == i)
                        {
                            continue;
                        }

                        if ((vClipVelocity[0] * planes[j][0]) + (vClipVelocity[1] * planes[j][1]) + (float)(vClipVelocity[2] * planes[j][2]) < 0.1f)
                        {
                            AIPhys_ClipVelocity(vClipVelocity, planes[j], isWalkable[j], vClipVelocity, 1.001);
                            AIPhys_ClipVelocity(vEndClipVelocity, planes[j], isWalkable[j], vEndClipVelocity, 1.001);

                            if ((vClipVelocity[0] * planes[i][0]) + (vClipVelocity[1] * planes[i][1]) + (vClipVelocity[2] * planes[i][2]) < 0.0f)
                            {
                                Vec3Cross(planes[i], planes[j], vDir);
                                Vec3Normalize(vDir);
                                float d = (vDir[0] * g_pPhys->vVelocity[0]) + (vDir[1] * g_pPhys->vVelocity[1]) + (vDir[2] * g_pPhys->vVelocity[2]);
                                Vec3Scale(vDir, d, vClipVelocity);


                                Vec3Cross(planes[i], planes[j], vDir);
                                Vec3Normalize(vDir);
                                d = (vDir[0] * vEndVelocity[0]) + (vDir[1] * vEndVelocity[1]) + (float)(vDir[2] * vEndVelocity[2]);
                                Vec3Scale(vDir, d, vEndClipVelocity);

                                for (int k = 0; k < iNumPlanes; ++k)
                                {
                                    if (k == i || k == j)
                                    {
                                        continue;
                                    }

                                    if (  (vClipVelocity[0] * planes[k][0])
                                        + (vClipVelocity[1] * planes[k][1])
                                        + (vClipVelocity[2] * planes[k][2]) < 0.1f)
                                    {
                                        g_pPhys->vVelocity[0] = 0.0f;
                                        g_pPhys->vVelocity[1] = 0.0f;
                                        g_pPhys->vVelocity[2] = 0.0f;
                                        return SLIDEMOVE_CLIPPED;
                                    }
                                }
                            }
                        }
                    }
                    if (g_pPhys->iHitEntnum == ENTITYNUM_NONE)
                    {
                        vDir[0] = vClipVelocity[0] - g_pPhys->vVelocity[0];
                        vDir[1] = vClipVelocity[1] - g_pPhys->vVelocity[1];

                        g_pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
                        g_pPhys->vHitOrigin[0] = g_pPhys->vOrigin[0];
                        g_pPhys->vHitOrigin[1] = g_pPhys->vOrigin[1];
                        g_pPhys->vHitNormal[0] = vDir[0];
                        g_pPhys->vHitNormal[1] = vDir[1];
                        g_pPhys->bStuck = 0;
                    }

                    g_pPhys->vVelocity[0] = vClipVelocity[0];
                    g_pPhys->vVelocity[1] = vClipVelocity[1];
                    g_pPhys->vVelocity[2] = vClipVelocity[2];

                    vEndVelocity[0] = vEndClipVelocity[0];
                    vEndVelocity[1] = vEndClipVelocity[1];
                    vEndVelocity[2] = vEndClipVelocity[2];
                    break;
                }
            }
        }
    }

    if (!trace.startsolid)
        goto LABEL_23;

    g_pPhys->vVelocity[2] = 0.0f;

    if (!iBumpCount && g_apl.groundTrace.startsolid)
    {
        return SLIDEMOVE_FAIL;
    }

    if (g_pPhys->iHitEntnum == ENTITYNUM_NONE)
    {
        g_pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
        g_pPhys->vHitOrigin[0] = g_pPhys->vOrigin[0];
        g_pPhys->vHitOrigin[1] = g_pPhys->vOrigin[1];
        g_pPhys->bStuck = 1;
    }

    return SLIDEMOVE_CLIPPED;

}

int AIPhys_StepSlideMove(int gravity, int zonly)
{
    float start_o[3];
    float start_v[3];

    float down[3];
    float up[3];

    trace_t trace;

    start_o[0] = g_pPhys->vOrigin[0];
    start_o[1] = g_pPhys->vOrigin[1];
    start_o[2] = g_pPhys->vOrigin[2];

    start_v[0] = g_pPhys->vVelocity[0];
    start_v[1] = g_pPhys->vVelocity[1];
    start_v[2] = g_pPhys->vVelocity[2];

    SlideMoveResult moveResult = AIPhys_SlideMove(gravity, zonly);

    if (moveResult == SLIDEMOVE_COMPLETE)
    {
        return 1;
    }
    if (moveResult == SLIDEMOVE_FAIL)
    {
        return 0;
    }

    iassert(moveResult == SLIDEMOVE_CLIPPED);


    float stepheight = g_apl.stepheight;

    if (g_pPhys->vVelocity[2] > 0.0 && !g_apl.bIsWalking)
    {
        down[0] = start_o[0];
        down[1] = start_o[1];
        down[2] = start_o[2] - stepheight;
        G_TraceCapsule(&trace, start_o, g_pPhys->vMins, g_pPhys->vMaxs, down, g_pPhys->iEntNum, g_apl.iTraceMask);
        if (trace.fraction == 1.0 || !trace.walkable)
        {
             return 1;
        }
    }

    up[0] = start_o[0];
    up[1] = start_o[1];
    up[2] = start_o[2] + stepheight;
    G_TraceCapsule(&trace, start_o, g_pPhys->vMins, g_pPhys->vMaxs, up, g_pPhys->iEntNum, g_apl.iTraceMask);
    if (trace.allsolid)
    {
        return trace.fraction != 0.0;
    }

    actor_physics_t phys;
    actor_physics_local_t localPhys;

    memcpy(&phys, g_pPhys, sizeof(actor_physics_t));
    memcpy(&localPhys, &g_apl, sizeof(actor_physics_local_t));
    
    g_pPhys->iHitEntnum = ENTITYNUM_NONE;
    g_apl.bGroundPlane = 0;

    float endpos[3];
    Vec3Lerp(start_o, up, trace.fraction, endpos);

    g_pPhys->vOrigin[0] = endpos[0];
    g_pPhys->vOrigin[1] = endpos[1];
    g_pPhys->vOrigin[2] = endpos[2];

    g_pPhys->vVelocity[0] = start_v[0];
    g_pPhys->vVelocity[1] = start_v[1];
    g_pPhys->vVelocity[2] = start_v[2];

    moveResult = AIPhys_SlideMove(gravity, zonly);

    float diff2_4;

    if (moveResult == SLIDEMOVE_FAIL)
    {
        goto LABEL_22;
    }

    diff2_4 = (float)((float)(g_apl.fFrameTime * start_v[1]) + start_o[1]) - g_pPhys->vOrigin[1];

    // Im leaving this if-statement intact for your enjoyment
    if ((float)((float)((float)((float)((float)(g_apl.fFrameTime * start_v[0]) + start_o[0]) - g_pPhys->vOrigin[0])
        * (float)((float)((float)(g_apl.fFrameTime * start_v[0]) + start_o[0]) - g_pPhys->vOrigin[0]))
        + (float)(diff2_4 * diff2_4)) > (float)((float)((float)((float)((float)(g_apl.fFrameTime * start_v[0])
            + start_o[0])
            - phys.vOrigin[0])
            * (float)((float)((float)(g_apl.fFrameTime * start_v[0])
                + start_o[0])
                - phys.vOrigin[0]))
            + (float)((float)((float)((float)(g_apl.fFrameTime * start_v[1])
                + start_o[1])
                - phys.vOrigin[1])
                * (float)((float)((float)(g_apl.fFrameTime * start_v[1])
                    + start_o[1])
                    - phys.vOrigin[1]))))
    {
        goto LABEL_22;
    }

    down[0] = g_pPhys->vOrigin[0];
    down[1] = g_pPhys->vOrigin[1];
    down[2] = g_pPhys->vOrigin[2];
    down[2] = (start_o[2] - endpos[2]) + down[2];

    G_TraceCapsule(&trace, g_pPhys->vOrigin, g_pPhys->vMins, g_pPhys->vMaxs, down, g_pPhys->iEntNum, g_apl.iTraceMask);

    if (!trace.startsolid)
    {
        Vec3Lerp(g_pPhys->vOrigin, down, trace.fraction, g_pPhys->vOrigin);
    }

    if (trace.fraction < 1.0f)
    {
        if (trace.normal[2] < 0.3f)
        {
LABEL_22:
            memcpy(g_pPhys, &phys, sizeof(actor_physics_t));
            memcpy(&g_apl, &localPhys, sizeof(actor_physics_local_t));
            return 1;
        }

        AIPhys_ClipVelocity(g_pPhys->vVelocity, trace.normal, trace.walkable, g_pPhys->vVelocity, 1.001f);
        float v4;
        if ((float)(start_o[2] - phys.vOrigin[2]) < 0.0)
            v4 = phys.vOrigin[2];
        else
            v4 = start_o[2];
        if (g_pPhys->vOrigin[2] > (float)(v4 + 0.1))
            g_pPhys->iHitEntnum = ENTITYNUM_NONE;
    }

    return 1;
}


int __cdecl AIPhys_AirMove()
{
    if (g_apl.bGroundPlane)
        AIPhys_ClipVelocity(
            g_pPhys->vVelocity,
            g_apl.groundTrace.normal,
            g_apl.groundTrace.walkable,
            g_pPhys->vVelocity,
            1.001);
    return AIPhys_StepSlideMove(1, 0);
}

int __cdecl AIPhys_WalkMove()
{
    g_pPhys->vVelocity[0] = (1.0f / g_apl.fFrameTime) * g_pPhys->vWishDelta[0];
    g_pPhys->vVelocity[1] = (1.0f / g_apl.fFrameTime) * g_pPhys->vWishDelta[1];
    g_pPhys->vVelocity[2] = 0.0f;

    float vel = Vec2Length(g_pPhys->vVelocity);
    float oldVel[2];
    oldVel[0] = g_pPhys->vVelocity[0];
    oldVel[1] = g_pPhys->vVelocity[1];

    AIPhys_ClipVelocity(g_pPhys->vVelocity, g_apl.groundTrace.normal, g_apl.groundTrace.walkable, g_pPhys->vVelocity, (1.0f + EQUAL_EPSILON));
    
    if (((g_pPhys->vVelocity[0] * oldVel[0]) + (g_pPhys->vVelocity[1] * oldVel[1])) > 0.0f)
    {
        Vec3Normalize(g_pPhys->vVelocity);
        Vec3Scale(g_pPhys->vVelocity, vel, g_pPhys->vVelocity);
    }

    if (g_pPhys->vVelocity[0] == 0.0f && g_pPhys->vVelocity[1] == 0.0f && g_apl.bGroundPlane)
    {
        return 1;
    }
    else
    {
        return AIPhys_StepSlideMove(0, 0);
    }
}

int __cdecl AIPhys_ZOnlyPhysicsMove()
{
    actor_physics_t *v0; // r11
    double v1; // fp0

    v0 = g_pPhys;
    v1 = (float)((float)1.0 / g_apl.fFrameTime);
    g_pPhys->vVelocity[0] = g_pPhys->vWishDelta[0] * (float)((float)1.0 / g_apl.fFrameTime);
    v0->vVelocity[1] = v0->vWishDelta[1] * (float)v1;
    v0->vVelocity[2] = 0.0;
    return AIPhys_StepSlideMove(1, 1);
}

void AIPhys_NoClipMove()
{
    actor_physics_t *v0; // r11

    v0 = g_pPhys;
    g_pPhys->vOrigin[0] = g_pPhys->vWishDelta[0] + g_pPhys->vOrigin[0];
    v0->vOrigin[1] = v0->vWishDelta[1] + v0->vOrigin[1];
    v0->vOrigin[2] = v0->vWishDelta[2] + v0->vOrigin[2];
}

SlideMoveResult AIPhys_NoGravityMove()
{
    actor_physics_t *v0; // r11
    double v1; // fp0

    v0 = g_pPhys;
    v1 = (float)((float)1.0 / g_apl.fFrameTime);
    g_pPhys->vVelocity[0] = g_pPhys->vWishDelta[0] * (float)((float)1.0 / g_apl.fFrameTime);
    v0->vVelocity[1] = v0->vWishDelta[1] * (float)v1;
    v0->vVelocity[2] = v0->vWishDelta[2] * (float)v1;
    return AIPhys_SlideMove(0, 1);
}

void AIPhys_GroundTrace()
{
    float stepheight; // fp0
    unsigned __int16 EntityHitId; // r3
    //float v9; // [sp+50h] [-70h] BYREF
    //float v10; // [sp+54h] [-6Ch]
    //float v11; // [sp+58h] [-68h]
    float start[3];
    float end[3];
    //float v12[2]; // [sp+60h] [-60h] BYREF
    //float v13; // [sp+68h] [-58h]
    trace_t trace; // [sp+70h] [-50h] BYREF

    start[0] = g_pPhys->vOrigin[0];
    start[1] = g_pPhys->vOrigin[1];
    start[2] = g_pPhys->vOrigin[2] + 0.25;

    end[0] = g_pPhys->vOrigin[0];
    end[1] = g_pPhys->vOrigin[1];

    bool use_big_trace = g_pPhys->vVelocity[2] <= 0.0f && g_pPhys->groundEntNum != ENTITYNUM_NONE;
    if (g_pPhys->vVelocity[2] <= 0.0 && g_pPhys->groundEntNum != ENTITYNUM_NONE)
        stepheight = g_apl.stepheight;
    else
        stepheight = 0.25f;

    end[2] = g_pPhys->vOrigin[2] - stepheight;

    G_TraceCapsule(&trace, start, g_pPhys->vMins, g_pPhys->vMaxs, end, g_pPhys->iEntNum, g_apl.iTraceMask);

    memcpy(&g_apl.groundTrace, &trace, sizeof(g_apl.groundTrace));
    if (!trace.startsolid)
        goto LABEL_10;
    if ((trace.contents & 0x2000000) != 0)
    {
        EntityHitId = Trace_GetEntityHitId(&trace);
        AIPhys_AddTouchEnt(EntityHitId);
        g_apl.iTraceMask &= ~0x2000000u;
        G_TraceCapsule(&trace, start, g_pPhys->vMins, g_pPhys->vMaxs, end, g_pPhys->iEntNum, g_apl.iTraceMask);
        memcpy(&g_apl.groundTrace, &trace, sizeof(g_apl.groundTrace));
    }
    if (trace.startsolid
        && (start[0] = g_pPhys->vOrigin[0],
            start[1]= g_pPhys->vOrigin[1],
            start[2]= g_pPhys->vOrigin[2],
            G_TraceCapsule(&trace, start, g_pPhys->vMins, g_pPhys->vMaxs, end, g_pPhys->iEntNum, g_apl.iTraceMask),
            memcpy(&g_apl.groundTrace, &trace, sizeof(g_apl.groundTrace)),
            trace.startsolid))
    {
        g_pPhys->groundEntNum = Trace_GetEntityHitId(&trace);
        g_apl.bGroundPlane = 0;
        g_apl.bIsWalking = 1;
    }
    else
    {
    LABEL_10:
        if (trace.fraction == 1.0
            || g_pPhys->vVelocity[2] > 0.0
            && ((trace.normal[0] * g_pPhys->vVelocity[0])
                + ((g_pPhys->vVelocity[2] * trace.normal[2]) + (g_pPhys->vVelocity[1] * trace.normal[1]))) > 10.0)
        {
            g_pPhys->groundEntNum = ENTITYNUM_NONE;
            g_apl.bGroundPlane = 0;
            g_apl.bIsWalking = 0;
        }
        else if (trace.walkable)
        {
            if ((((end[2] - start[2]) * trace.fraction) + start[2]) < g_pPhys->vOrigin[2] || g_pPhys->ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE)
            {
                g_pPhys->vOrigin[2] = ((end[2] - start[2]) * trace.fraction) + start[2];
            }
            g_apl.bGroundPlane = 1;
            g_apl.bIsWalking = 1;
            g_pPhys->groundEntNum = Trace_GetEntityHitId(&trace);
            AIPhys_AddTouchEnt(g_pPhys->groundEntNum);
        }
        else
        {
            g_pPhys->groundEntNum = ENTITYNUM_NONE;
            g_apl.bGroundPlane = 1;
            g_apl.bIsWalking = 0;
            if (g_pPhys->iHitEntnum == ENTITYNUM_NONE && (trace.normal[0] != 0.0 || trace.normal[1] != 0.0))
            {
                g_pPhys->iHitEntnum = Trace_GetEntityHitId(&trace);
                g_pPhys->vHitOrigin[0] = g_pPhys->vOrigin[0];
                g_pPhys->vHitOrigin[1] = g_pPhys->vOrigin[1];
                g_pPhys->vHitNormal[0] = trace.normal[0];
                g_pPhys->vHitNormal[1] = trace.normal[1];
                g_pPhys->bStuck = 0;
            }
        }
    }
}

void AIPhys_Footsteps()
{
    actor_physics_t *v0; // r11
    int groundEntNum; // r8
    int iFootstepTimer; // r9
    aiphys_t ePhysicsType; // r10
    int v4; // r10
    gentity_s *v5; // r31
    char v6; // r3

    v0 = g_pPhys;
    groundEntNum = g_pPhys->groundEntNum;
    iFootstepTimer = g_pPhys->iFootstepTimer;
    g_pPhys->iFootstepTimer = 0;
    if (groundEntNum != ENTITYNUM_NONE)
    {
        ePhysicsType = v0->ePhysicsType;
        if (ePhysicsType == AIPHYS_NORMAL_ABSOLUTE
            || ePhysicsType == AIPHYS_NORMAL_RELATIVE
            || ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE)
        {
            v4 = v0->iMsec + iFootstepTimer;
            v0->iFootstepTimer = v4;
            if (v4 >= 500)
            {
                v5 = &level.gentities[v0->iEntNum];
                if (!v5->r.inuse)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_physics.cpp", 766, 0, "%s", "ent->r.inuse");
                if (!v5->actor)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_physics.cpp", 767, 0, "%s", "ent->actor");
                if (!v5->sentient)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_physics.cpp", 768, 0, "%s", "ent->sentient");
                v6 = Sentient_EnemyTeam(v5->sentient->eTeam);
                Actor_BroadcastPointEvent(v5, 2, 1 << v6, g_pPhys->vOrigin, 0.0);
                g_pPhys->iFootstepTimer %= 500;
            }
        }
    }
}

void AIPhys_FoliageSounds(void)
{
    actor_physics_t *phys = g_pPhys;
    float speed2d = sqrtf(phys->vVelocity[0] * phys->vVelocity[0]
        + phys->vVelocity[1] * phys->vVelocity[1]);
    const dvar_s *minD = bg_foliagesnd_minspeed;
    const dvar_s *maxD = bg_foliagesnd_maxspeed;

    if (speed2d >= minD->current.value)
    {
        float range = maxD->current.value - minD->current.value;
        if (range <= 0.0f)
        {
            MyAssertHandler("actor_physics.cpp", 808, 1,
                "bg_foliagesnd_maxspeed->current.value - bg_foliagesnd_minspeed->current.value > 0");
            range = 0.0f;  // prevent divide by zero
        }

        float t = (speed2d - minD->current.value) / range;
        if (t > 1.0f)
            t = 1.0f;

        unsigned slow = bg_foliagesnd_slowinterval->current.integer;
        unsigned fast = bg_foliagesnd_fastinterval->current.integer;
        unsigned since = phys->foliageSoundTime;
        unsigned interval = slow + (unsigned)((fast - slow) * t);

        if (since + interval < level.time)
        {
            float mins[3], maxs[3];
            mins[0] = phys->vMins[0] * 0.75f;
            mins[1] = phys->vMins[1] * 0.75f;
            mins[2] = phys->vMins[2] * 0.75f;

            maxs[0] = phys->vMaxs[0] * 0.75f;
            maxs[1] = phys->vMaxs[1] * 0.75f;
            maxs[2] = phys->vMaxs[2] * 0.9f;

            trace_t tr;
            G_TraceCapsule(&tr, phys->vOrigin, mins, maxs, phys->vOrigin, phys->iEntNum, 2);
            if (tr.startsolid)
            {
                G_AddEvent(&g_entities[phys->iEntNum], 1, 0);
                phys->foliageSoundTime = level.time;
            }
        }
    }
    else if (phys->foliageSoundTime + bg_foliagesnd_resetinterval->current.integer < level.time)
    {
        phys->foliageSoundTime = 0;
    }
}


int __cdecl Actor_Physics(actor_physics_t *pPhys)
{
    int groundEntNum; // r9
    actor_physics_t *v3; // r29
    __int64 v5; // r11
    double v6; // fp0
    aiphys_t ePhysicsType; // r11
    int v8; // r3
    int v9; // r31
    actor_physics_t *v10; // r11

    PROF_SCOPED("Actor_Physics");

    groundEntNum = pPhys->groundEntNum;
    v3 = pPhys;
    pPhys->iNumTouch = 0;
    pPhys->bDeflected = 0;
    pPhys->iHitEntnum = ENTITYNUM_NONE;
    g_pPhys = pPhys;
    if (groundEntNum == ENTITYNUM_WORLD && pPhys->vWishDelta[0] == 0.0 && pPhys->vWishDelta[1] == 0.0 && pPhys->vWishDelta[2] == 0.0)
    {
        pPhys->vVelocity[0] = 0.0;
        pPhys->vVelocity[1] = 0.0;
        pPhys->vVelocity[2] = 0.0;
        return 1;
    }
    else
    {
        memset(&g_apl, 0, sizeof(g_apl));
        g_apl.iTraceMask = pPhys->iTraceMask;
        g_apl.vPrevOrigin[0] = pPhys->vOrigin[0];
        g_apl.vPrevOrigin[1] = pPhys->vOrigin[1];
        g_apl.vPrevOrigin[2] = pPhys->vOrigin[2];
        g_apl.vPrevVelocity[0] = pPhys->vVelocity[0];
        g_apl.vPrevVelocity[1] = pPhys->vVelocity[1];
        g_apl.vPrevVelocity[2] = pPhys->vVelocity[2];
        LODWORD(v5) = pPhys->iMsec;
        g_apl.fFrameTime = (float)v5 * (float)0.001;
        if (pPhys->iMsec <= 0)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_physics.cpp", 876, 0, "%s", "g_pPhys->iMsec > 0");
            v3 = g_pPhys;
        }
        if (pPhys->prone)
            v6 = 10.0;
        else
            v6 = 18.0;
        g_apl.stepheight = v6;
        ePhysicsType = v3->ePhysicsType;
        v3->groundplaneSlope = 0.0;
        v3->bHasGroundPlane = 0;
        if (ePhysicsType == AIPHYS_NOCLIP)
        {
            v3->vOrigin[0] = v3->vOrigin[0] + v3->vWishDelta[0];
            v3->vOrigin[1] = v3->vWishDelta[1] + v3->vOrigin[1];
            v3->vOrigin[2] = v3->vWishDelta[2] + v3->vOrigin[2];
            return 1;
        }
        else if (ePhysicsType == AIPHYS_NOGRAVITY)
        {
            AIPhys_NoGravityMove();
            return 1;
        }
        else
        {
            AIPhys_GroundTrace();
            if (g_pPhys->ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE)
            {
                v8 = AIPhys_ZOnlyPhysicsMove();
            }
            else if (g_apl.bIsWalking)
            {
                v8 = AIPhys_WalkMove();
            }
            else
            {
                v8 = AIPhys_AirMove();
            }
            v9 = v8;
            if (g_apl.bGroundPlane && g_apl.groundTrace.normal[2] >= 0.30000001)
            {
                v10 = g_pPhys;
                g_pPhys->bHasGroundPlane = 1;
                v10->groundplaneSlope = g_apl.groundTrace.normal[2];
                v10->iSurfaceType = (g_apl.groundTrace.surfaceFlags >> 20) & 0x1F;
            }
            else
            {
                g_pPhys->iSurfaceType = 0;
            }
            //Profile_EndInternal(0);
            return v9;
        }
    }
}

void __cdecl Actor_PostPhysics(actor_physics_t *pPhys)
{
    int bIsAlive; // r10

    //Profile_Begin(225);
    bIsAlive = pPhys->bIsAlive;
    g_pPhys = pPhys;
    if (bIsAlive)
        AIPhys_Footsteps();
    AIPhys_FoliageSounds();
    //Profile_EndInternal(0);
}

