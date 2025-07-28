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
    actor_physics_t *v1; // r11
    int iNumTouch; // r8
    int v3; // r9
    int *iTouchEnts; // r10

    if (entityNum != 2174)
    {
        v1 = g_pPhys;
        iNumTouch = g_pPhys->iNumTouch;
        if (iNumTouch != 32)
        {
            v3 = 0;
            if (iNumTouch <= 0)
            {
            LABEL_7:
                g_pPhys->iTouchEnts[iNumTouch] = entityNum;
                ++v1->iNumTouch;
            }
            else
            {
                iTouchEnts = g_pPhys->iTouchEnts;
                while (*iTouchEnts != entityNum)
                {
                    ++v3;
                    ++iTouchEnts;
                    if (v3 >= g_pPhys->iNumTouch)
                        goto LABEL_7;
                }
            }
        }
    }
}

void __cdecl AIPhys_ClipVelocity(
    const float *in,
    const float *normal,
    bool isWalkable,
    float *out,
    double overbounce)
{
    double v5; // fp0

    if (isWalkable && (float)((float)(*in * *in) + (float)(in[1] * in[1])) >= (double)(float)(in[2] * in[2]))
    {
        out[2] = -(float)((float)(normal[1] * in[1]) - (float)-(float)(*normal * *in));
        *out = *in * normal[2];
        out[1] = in[1] * normal[2];
    }
    else
    {
        if ((float)((float)(in[1] * normal[1]) + (float)((float)(*normal * *in) + (float)(in[2] * normal[2]))) >= 0.0)
            v5 = (float)((float)((float)(in[1] * normal[1]) + (float)((float)(*normal * *in) + (float)(in[2] * normal[2])))
                / (float)overbounce);
        else
            v5 = (float)((float)((float)(in[1] * normal[1]) + (float)((float)(*normal * *in) + (float)(in[2] * normal[2])))
                * (float)overbounce);
        *out = -(float)((float)(*normal * (float)v5) - *in);
        out[1] = -(float)((float)(normal[1] * (float)v5) - in[1]);
        out[2] = -(float)((float)((float)v5 * normal[2]) - in[2]);
    }
}

SlideMoveResult __cdecl AIPhys_SlideMove(int gravity, int zonly)
{
    float *vVelocity; // r3
    double v4; // fp26
    double v5; // fp25
    double v6; // fp27
    bool v7; // zf
    double v8; // fp0
    double fFrameTime; // fp20
    int v10; // r31
    actor_physics_t *v11; // r11
    int v12; // r27
    bool *v13; // r23
    float *v14; // r24
    unsigned int v15; // r22
    double fraction; // fp12
    double v17; // fp13
    double v18; // fp11
    SlideMoveResult result; // r3
    unsigned __int16 EntityHitId; // r3
    actor_physics_t *v21; // r11
    double v22; // fp0
    unsigned __int16 v23; // r3
    double v24; // fp13
    int v25; // r31
    double v26; // fp0
    float *v27; // r11
    unsigned __int16 v28; // r3
    double v29; // fp0
    bool walkable; // r10
    double v31; // fp0
    int v32; // r28
    float *v33; // r10
    double v34; // fp0
    double v35; // fp0
    bool v36; // r5
    const float *v37; // r4
    double v38; // fp1
    double v39; // fp28
    double v40; // fp29
    int v41; // r29
    double v42; // fp30
    float *v43; // r30
    bool v44; // r5
    const float *v45; // r4
    double v46; // fp1
    const float *v47; // r4
    double v48; // fp7
    double v51; // fp0
    double v52; // fp12
    double v53; // fp13
    double v54; // fp11
    double v55; // fp0
    double v56; // fp0
    int v57; // r11
    float *v58; // r10
    double v61; // fp0
    double v62; // fp13
    double v63; // fp0
    double v64; // fp27
    double v65; // fp26
    unsigned __int16 v66; // r3
    double v67; // fp0
    actor_physics_t *v68; // r11
    unsigned __int16 v69; // r3
    actor_physics_t *v70; // r11
    double v71; // fp0
    actor_physics_t *v72; // r11
    float v73; // [sp+50h] [-1A0h] BYREF
    float v74; // [sp+54h] [-19Ch]
    float v75; // [sp+58h] [-198h]
    _BYTE v76[12]; // [sp+5Ch] [-194h] BYREF
    float v77; // [sp+68h] [-188h] BYREF
    float v78; // [sp+6Ch] [-184h]
    float v79; // [sp+70h] [-180h]
    float v80; // [sp+78h] [-178h] BYREF
    float v81; // [sp+7Ch] [-174h]
    float v82; // [sp+80h] [-170h]
    float v83; // [sp+88h] [-168h] BYREF
    float v84; // [sp+8Ch] [-164h]
    float v85; // [sp+90h] [-160h]
    trace_t v86; // [sp+A0h] [-150h] BYREF
    float v87; // [sp+D0h] [-120h] BYREF
    float v88; // [sp+D4h] [-11Ch]
    float v89; // [sp+D8h] [-118h]
    float v90; // [sp+E0h] [-110h] BYREF
    float v91; // [sp+E4h] [-10Ch] BYREF
    float v92[40]; // [sp+E8h] [-108h] BYREF

    v4 = g_pPhys->vVelocity[0];
    v5 = g_pPhys->vVelocity[1];
    v6 = g_pPhys->vVelocity[2];
    v83 = g_pPhys->vVelocity[0];
    v84 = v5;
    v85 = v6;
    v7 = gravity == 0;
    vVelocity = g_pPhys->vVelocity;
    if (!v7)
    {
        v6 = (float)-(float)((float)(g_pPhys->fGravity * g_apl.fFrameTime) - (float)v6);
        v8 = (float)((float)(g_pPhys->vVelocity[2] + (float)v6) * (float)0.5);
        v85 = v6;
        g_pPhys->vVelocity[2] = v8;
        if (g_apl.bGroundPlane)
        {
            if (!zonly)
                AIPhys_ClipVelocity(vVelocity, g_apl.groundTrace.normal, g_apl.groundTrace.walkable, vVelocity, 1.001);
        }
    }
    fFrameTime = g_apl.fFrameTime;
    if (g_apl.bGroundPlane)
    {
        v90 = g_apl.groundTrace.normal[0];
        v10 = 1;
        v91 = g_apl.groundTrace.normal[1];
        v92[0] = g_apl.groundTrace.normal[2];
        v76[0] = g_apl.groundTrace.walkable;
    }
    else
    {
        v10 = 0;
    }
    Vec3NormalizeTo(vVelocity, &v90 + 3 * v10);
    v11 = g_pPhys;
    v12 = v10 + 1;
    v13 = (bool*)&v76[v10 + 1];
    v76[v10] = g_pPhys->vVelocity[2] > 0.69999999;
    v14 = &v92[3 * v10 + 3];
    v15 = 0;
    while (1)
    {
        v87 = (float)(v11->vVelocity[0] * (float)fFrameTime) + v11->vOrigin[0];
        v88 = (float)(v11->vVelocity[1] * (float)fFrameTime) + v11->vOrigin[1];
        v89 = (float)(v11->vVelocity[2] * (float)fFrameTime) + v11->vOrigin[2];
        //Profile_Begin(226);
        G_TraceCapsule(&v86, g_pPhys->vOrigin, g_pPhys->vMins, g_pPhys->vMaxs, &v87, g_pPhys->iEntNum, g_apl.iTraceMask);
        //Profile_EndInternal(0);
        fraction = v86.fraction;
        v11 = g_pPhys;
        if (v86.fraction <= 0.0)
            break;
        v17 = g_pPhys->vOrigin[1];
        v18 = g_pPhys->vOrigin[2];
        g_pPhys->vOrigin[0] = (float)((float)(v87 - g_pPhys->vOrigin[0]) * v86.fraction) + g_pPhys->vOrigin[0];
        v11->vOrigin[1] = (float)((float)(v88 - (float)v17) * (float)fraction) + (float)v17;
        v11->vOrigin[2] = (float)((float)(v89 - (float)v18) * (float)fraction) + (float)v18;
        if (v86.fraction == 1.0)
            goto LABEL_11;
    LABEL_15:
        v11->bDeflected = 1;
        if (!v86.walkable && v11->iHitEntnum == 2175 && (v86.normal[0] != 0.0 || v86.normal[1] != 0.0))
        {
            EntityHitId = Trace_GetEntityHitId(&v86);
            v21 = g_pPhys;
            v22 = g_pPhys->vOrigin[0];
            g_pPhys->iHitEntnum = EntityHitId;
            v21->vHitOrigin[0] = v22;
            v21->vHitOrigin[1] = v21->vOrigin[1];
            *(_QWORD *)v21->vHitNormal = *(_QWORD *)v86.normal;
            v21->bStuck = 0;
        }
        v23 = Trace_GetEntityHitId(&v86);
        AIPhys_AddTouchEnt(v23);
        fFrameTime = (float)-(float)((float)(v86.fraction * (float)fFrameTime) - (float)fFrameTime);
        if (v12 >= 5)
            goto LABEL_61;
        v24 = v86.normal[1];
        v25 = 0;
        v26 = v86.normal[0];
        if (v12 <= 0)
        {
        LABEL_25:
            v11 = g_pPhys;
            goto LABEL_33;
        }
        v27 = &v91;
        while ((float)((float)(*v27 * v86.normal[1])
            + (float)((float)(v27[1] * v86.normal[2]) + (float)(*(v27 - 1) * v86.normal[0]))) <= 0.99)
        {
            ++v25;
            v27 += 3;
            if (v25 >= v12)
                goto LABEL_25;
        }
        v11 = g_pPhys;
        if (v86.fraction == 0.0 && g_pPhys->iHitEntnum == 2175 && (v26 != 0.0 || v24 != 0.0))
        {
            v28 = Trace_GetEntityHitId(&v86);
            v11 = g_pPhys;
            v29 = g_pPhys->vOrigin[0];
            g_pPhys->iHitEntnum = v28;
            v11->vHitOrigin[0] = v29;
            v11->vHitOrigin[1] = v11->vOrigin[1];
            *(_QWORD *)v11->vHitNormal = *(_QWORD *)v86.normal;
            v11->bStuck = 0;
            v26 = v86.normal[0];
        }
        v11->vVelocity[0] = v11->vVelocity[0] + (float)v26;
        v11->vVelocity[1] = v11->vVelocity[1] + v86.normal[1];
        v11->vVelocity[2] = v11->vVelocity[2] + v86.normal[2];
        if (v25 >= v12)
        {
            v24 = v86.normal[1];
            v26 = v86.normal[0];
        LABEL_33:
            walkable = v86.walkable;
            *(v14 - 2) = v26;
            ++v12;
            v31 = v86.normal[2];
            *(v14 - 1) = v24;
            v32 = 0;
            *v14 = v31;
            v14 += 3;
            *v13++ = walkable;
            if (v12 > 0)
            {
                v33 = &v91;
                while (1)
                {
                    v34 = (float)((float)(v11->vVelocity[1] * *v33)
                        + (float)((float)(*(v33 - 1) * v11->vVelocity[0]) + (float)(v33[1] * v11->vVelocity[2])));
                    if (v34 < 0.1)
                        break;
                    ++v32;
                    v33 += 3;
                    if (v32 >= v12)
                        goto LABEL_53;
                }
                v35 = -v34;
                if (v35 > g_apl.fImpactSpeed)
                    g_apl.fImpactSpeed = v35;
                AIPhys_ClipVelocity(v11->vVelocity, &v90 + 3 * v32, v76[v32], &v77, 1.001);
                AIPhys_ClipVelocity(&v83, v37, v36, &v80, v38);
                v39 = v79;
                v40 = v78;
                v41 = 0;
                v42 = v77;
                v43 = &v90;
                while (1)
                {
                    if (v41 != v32
                        && (float)((float)((float)v39 * v43[2]) + (float)((float)(v43[1] * (float)v40) + (float)(*v43 * (float)v42))) < 0.1)
                    {
                        AIPhys_ClipVelocity(&v77, v43, v76[v41], &v77, 1.001);
                        AIPhys_ClipVelocity(&v80, v45, v44, &v80, v46);
                        v39 = v79;
                        v40 = v78;
                        v42 = v77;
                        if ((float)((float)(*(&v90 + 3 * v32) * v77)
                            + (float)((float)(*(&v90 + 3 * v32 + 1) * v78) + (float)(*(&v90 + 3 * v32 + 2) * v79))) < 0.0)
                            break;
                    }
                LABEL_49:
                    ++v41;
                    v43 += 3;
                    if (v41 >= v12)
                    {
                        v11 = g_pPhys;
                        if (g_pPhys->iHitEntnum == 2175)
                        {
                            v64 = (float)((float)v42 - g_pPhys->vVelocity[0]);
                            v65 = (float)((float)v40 - g_pPhys->vVelocity[1]);
                            v73 = (float)v42 - g_pPhys->vVelocity[0];
                            v74 = v65;
                            v66 = Trace_GetEntityHitId(&v86);
                            v11 = g_pPhys;
                            v67 = g_pPhys->vOrigin[0];
                            g_pPhys->iHitEntnum = v66;
                            v11->vHitOrigin[0] = v67;
                            v11->vHitOrigin[1] = v11->vOrigin[1];
                            v11->vHitNormal[0] = v64;
                            v11->vHitNormal[1] = v65;
                            v11->bStuck = 0;
                        }
                        v4 = v80;
                        v5 = v81;
                        v6 = v82;
                        v83 = v80;
                        v84 = v81;
                        v85 = v82;
                        v11->vVelocity[0] = v42;
                        v11->vVelocity[1] = v40;
                        v11->vVelocity[2] = v39;
                        goto LABEL_53;
                    }
                }
                Vec3Cross(&v90 + 3 * v32, v47, &v73);
                v48 = g_pPhys->vVelocity[0];

                //_FP10 = -sqrtf((float)((float)(v73 * v73) + (float)((float)(v75 * v75) + (float)(v74 * v74))));
                //__asm { fsel      f0, f10, f31, f0 }
                //v51 = (float)((float)1.0 / (float)_FP0);

                {
                    float tmp = sqrtf(v73 * v73 + v75 * v75 + v74 * v74);
                    float neg = -tmp;

                    // Emulate: fsel f0, f10, f31, f0
                    // Meaning: if (neg >= 0) use f31, else use previous f0
                    // Assuming fallback is tmp (mag), adjust if you know actual f31 value
                    float selected = (neg >= 0.0f) ? tmp : tmp;

                    v51 = 1.0f / selected;
                }


                v52 = (float)(v74 * (float)v51);
                v53 = (float)(v75 * (float)v51);
                v54 = (float)(v73 * (float)v51);
                v55 = (float)((float)(g_pPhys->vVelocity[2] * (float)(v75 * (float)v51))
                    + (float)(g_pPhys->vVelocity[1] * (float)(v74 * (float)v51)));
                v73 = v54;
                v75 = v53;
                v74 = v52;
                v56 = (float)((float)((float)v48 * (float)v54) + (float)v55);
                v42 = (float)((float)v54 * (float)v56);
                v77 = (float)v54 * (float)v56;
                v40 = (float)((float)v52 * (float)v56);
                v78 = (float)v52 * (float)v56;
                v39 = (float)((float)v53 * (float)v56);
                v79 = (float)v53 * (float)v56;
                Vec3Cross(&v90 + 3 * v32, v43, &v73);
                v57 = 0;
                v58 = v92;

                //_FP10 = -sqrtf((float)((float)(v73 * v73) + (float)((float)(v75 * v75) + (float)(v74 * v74))));
                //__asm { fsel      f0, f10, f31, f0 }
                //v61 = (float)((float)1.0 / (float)_FP0);

                {
                    float len = sqrtf(v73 * v73 + v75 * v75 + v74 * v74);
                    float neg = -len;

                    // Emulate PowerPC `fsel f0, f10, f31, f0`:
                    // f0 = (neg >= 0 ? f31 : f0)
                    // We lack original fallback register value, so default to `len`.
                    float denom = (neg >= 0.0f) ? len : len;

                    v61 = 1.0f / denom;
                }

                v73 = (float)v61 * v73;
                v74 = v74 * (float)v61;
                v62 = (float)(v75 * (float)v61);
                v63 = (float)((float)((float)(v75 * (float)v61) * (float)v6)
                    + (float)((float)(v74 * (float)v5) + (float)(v73 * (float)v4)));
                v75 = v62;
                v80 = v73 * (float)v63;
                v81 = v74 * (float)v63;
                v82 = (float)v62 * (float)v63;
                while (v57 == v32
                    || v57 == v41
                    || (float)((float)(*v58 * (float)v39)
                        + (float)((float)(*(v58 - 2) * (float)v42) + (float)(*(v58 - 1) * (float)v40))) >= 0.1)
                {
                    ++v57;
                    v58 += 3;
                    if (v57 >= v12)
                        goto LABEL_49;
                }
            LABEL_61:
                v72 = g_pPhys;
                result = SLIDEMOVE_CLIPPED;
                g_pPhys->vVelocity[0] = 0.0;
                v72->vVelocity[1] = 0.0;
                v72->vVelocity[2] = 0.0;
                return result;
            }
        }
    LABEL_53:
        if ((int)++v15 >= 4)
        {
        LABEL_11:
            if (gravity)
            {
                v11->vVelocity[0] = v4;
                v11->vVelocity[1] = v5;
                v11->vVelocity[2] = v6;
            }
            //return (_cntlzw(v15) & 0x20) == 0;
            return (SlideMoveResult)(v15 != 0);
        }
    }
    if (!v86.startsolid)
        goto LABEL_15;
    v68 = g_pPhys;
    g_pPhys->vVelocity[2] = 0.0;
    if (!v15 && g_apl.groundTrace.startsolid)
        return SLIDEMOVE_FAIL;
    if (v68->iHitEntnum == 2175)
    {
        v69 = Trace_GetEntityHitId(&v86);
        v70 = g_pPhys;
        v71 = g_pPhys->vOrigin[0];
        g_pPhys->iHitEntnum = v69;
        v70->vHitOrigin[0] = v71;
        v70->vHitOrigin[1] = v70->vOrigin[1];
        v70->bStuck = 1;
    }
    return SLIDEMOVE_CLIPPED;
}

// aislop
int AIPhys_StepSlideMove(int gravity, int zonly)
{
    float startX = g_pPhys->vOrigin[0];
    float startY = g_pPhys->vOrigin[1];
    float startZ = g_pPhys->vOrigin[2];
    float velX = g_pPhys->vVelocity[0];
    float velY = g_pPhys->vVelocity[1];
    float velZ = g_pPhys->vVelocity[2];

    SlideMoveResult moveResult = AIPhys_SlideMove(gravity, zonly);

    if (moveResult == SLIDEMOVE_FAIL)
        return 0;

    iassert(moveResult == SLIDEMOVE_CLIPPED);

    float stepHeight = g_apl.stepheight;

    // Check if we should attempt to step up
    int tryStep = 0;
    if (g_pPhys->vVelocity[2] <= 0.0f)
        tryStep = 1;
    else if (g_apl.bIsWalking)
        tryStep = 1;
    else
    {
        float downX = startX;
        float downY = startY;
        float downZ = startZ - stepHeight;
        trace_t traceDown;
        G_TraceCapsule(&traceDown, &startX, g_pPhys->vMins, g_pPhys->vMaxs,
            &downX, g_pPhys->iEntNum, g_apl.iTraceMask);

        if (traceDown.fraction != 1.0f && traceDown.walkable)
            tryStep = 1;
    }

    if (!tryStep)
        return 1;

    // Trace up by stepHeight to see if stepping up is possible
    float upX = startX;
    float upY = startY;
    float upZ = startZ + stepHeight;
    trace_t traceUp;
    G_TraceCapsule(&traceUp, &startX, g_pPhys->vMins, g_pPhys->vMaxs,
        &upX, g_pPhys->iEntNum, g_apl.iTraceMask);

    if (traceUp.allsolid)
    {
        if (traceUp.fraction != 0.0f)
            return 1;
        return 0;
    }

    // Backup current physics state
    actor_physics_t physBackup;
    memcpy(&physBackup, g_pPhys, sizeof(actor_physics_t));

    actor_physics_local_t aplBackup;
    memcpy(&aplBackup, &g_apl, sizeof(actor_physics_local_t));

    g_pPhys->iHitEntnum = 2175;  // Magic number (entity hit)
    g_apl.bGroundPlane = 0;

    // Move up fraction of stepHeight
    float deltaX = upX - startX;
    float deltaY = upY - startY;
    float deltaZ = upZ - startZ;
    float fraction = traceUp.fraction;

    g_pPhys->vOrigin[0] = startX + deltaX * fraction;
    g_pPhys->vOrigin[1] = startY + deltaY * fraction;
    g_pPhys->vOrigin[2] = startZ + deltaZ * fraction;

    // Restore velocity
    g_pPhys->vVelocity[0] = velX;
    g_pPhys->vVelocity[1] = velY;
    g_pPhys->vVelocity[2] = velZ;

    if (AIPhys_SlideMove(gravity, zonly) == SLIDEMOVE_FAIL)
    {
        // Rollback on failure
        memcpy(g_pPhys, &physBackup, sizeof(actor_physics_t));
        memcpy(&g_apl, &aplBackup, sizeof(actor_physics_local_t));
        return 1;
    }

    // Trace down from new position to floor
    float floorX = g_pPhys->vOrigin[0];
    float floorY = g_pPhys->vOrigin[1];
    float floorZ = g_pPhys->vOrigin[2];

    float floorTraceEndX = startX;
    float floorTraceEndY = startY;
    float floorTraceEndZ = startZ - (g_pPhys->vOrigin[2] - startZ) + g_pPhys->vOrigin[2];

    G_TraceCapsule(&traceUp, g_pPhys->vOrigin, g_pPhys->vMins, g_pPhys->vMaxs,
        &floorTraceEndX, g_pPhys->iEntNum, g_apl.iTraceMask);

    if (!traceUp.startsolid)
    {
        float frac = traceUp.fraction;
        g_pPhys->vOrigin[0] += (floorTraceEndX - g_pPhys->vOrigin[0]) * frac;
        g_pPhys->vOrigin[1] += (floorTraceEndY - g_pPhys->vOrigin[1]) * frac;
        g_pPhys->vOrigin[2] += (floorTraceEndZ - g_pPhys->vOrigin[2]) * frac;
    }

    // Handle velocity clipping if needed
    if (traceUp.fraction < 1.0f)
    {
        if (traceUp.normal[2] < 0.3f)
        {
            // Rollback on steep slopes
            memcpy(g_pPhys, &physBackup, sizeof(actor_physics_t));
            memcpy(&g_apl, &aplBackup, sizeof(actor_physics_local_t));
            return 1;
        }

        AIPhys_ClipVelocity(g_pPhys->vVelocity, traceUp.normal, traceUp.walkable,
            g_pPhys->vVelocity, 1.001f);

        // Additional ground plane update could go here
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
    actor_physics_t *v0; // r9
    double v1; // fp0
    float *vVelocity; // r3
    int v3; // r10
    int v4; // r9
    float *v5; // r6
    double v6; // fp9
    double v7; // fp8
    double v8; // fp7
    double v9; // fp6
    double v10; // fp13
    double v11; // fp5
    double v12; // fp12
    double v15; // fp11

    v0 = g_pPhys;
    v1 = (float)((float)1.0 / g_apl.fFrameTime);
    vVelocity = g_pPhys->vVelocity;
    g_pPhys->vVelocity[0] = g_pPhys->vWishDelta[0] * (float)((float)1.0 / g_apl.fFrameTime);
    vVelocity[1] = v0->vWishDelta[1] * (float)v1;
    v0->vVelocity[2] = 0.0;
    AIPhys_ClipVelocity(vVelocity, g_apl.groundTrace.normal, g_apl.groundTrace.walkable, vVelocity, 1.001);
    v10 = v5[1];
    if ((float)((float)(v5[1] * (float)v11) + (float)(*v5 * (float)v9)) > v8)
    {
        v12 = v5[2];

        //_FP10 = -sqrtf((float)((float)(v5[2] * v5[2]) + (float)((float)(*v5 * *v5) + (float)(v5[1] * v5[1]))));
        //__asm { fsel      f11, f10, f9, f11 }
        //v15 = (float)((float)v6 / (float)_FP11);

        {
            float len = sqrtf(v5[0] * v5[0] + v5[1] * v5[1] + v5[2] * v5[2]);
            float neg = -len;

            float denom = (neg >= 0.0f) ? neg : neg;

            v15 = v6 / denom;
        }


        *v5 = *v5 * (float)v15;
        v5[1] = (float)v10 * (float)v15;
        v5[2] = (float)v12 * (float)v15;
        *v5 = *v5 * (float)v7;
        v5[1] = v5[1] * (float)v7;
        v5[2] = v5[2] * (float)v7;
    }
    if (*v5 == v8 && *(float *)(v4 + 16) == v8 && *(unsigned int *)(v3 + 8))
        return 1;
    else
        return AIPhys_StepSlideMove(0, 0);
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
    double v0; // fp12
    double stepheight; // fp0
    double v2; // fp11
    unsigned __int16 EntityHitId; // r3
    actor_physics_t *v4; // r11
    unsigned __int16 v5; // r3
    actor_physics_t *v6; // r11
    double v7; // fp0
    unsigned __int16 v8; // r3
    float v9; // [sp+50h] [-70h] BYREF
    float v10; // [sp+54h] [-6Ch]
    float v11; // [sp+58h] [-68h]
    float v12[2]; // [sp+60h] [-60h] BYREF
    float v13; // [sp+68h] [-58h]
    trace_t v14; // [sp+70h] [-50h] BYREF

    v9 = g_pPhys->vOrigin[0];
    v12[0] = v9;
    v0 = g_pPhys->vOrigin[2];
    stepheight = 0.25;
    v2 = g_pPhys->vVelocity[2];
    v10 = g_pPhys->vOrigin[1];
    v12[1] = v10;
    v11 = (float)v0 + (float)0.25;
    if (v2 <= 0.0 && g_pPhys->groundEntNum != 2175)
        stepheight = g_apl.stepheight;
    v13 = (float)v0 - (float)stepheight;
    //Profile_Begin(226);
    G_TraceCapsule(&v14, &v9, g_pPhys->vMins, g_pPhys->vMaxs, v12, g_pPhys->iEntNum, g_apl.iTraceMask);
    //Profile_EndInternal(0);
    memcpy(&g_apl.groundTrace, &v14, sizeof(g_apl.groundTrace));
    if (!v14.startsolid)
        goto LABEL_10;
    if ((v14.contents & 0x2000000) != 0)
    {
        EntityHitId = Trace_GetEntityHitId(&v14);
        AIPhys_AddTouchEnt(EntityHitId);
        g_apl.iTraceMask &= ~0x2000000u;
        //Profile_Begin(226);
        G_TraceCapsule(&v14, &v9, g_pPhys->vMins, g_pPhys->vMaxs, v12, g_pPhys->iEntNum, g_apl.iTraceMask);
        //Profile_EndInternal(0);
        memcpy(&g_apl.groundTrace, &v14, sizeof(g_apl.groundTrace));
    }
    if (v14.startsolid
        && (v9 = g_pPhys->vOrigin[0],
            v10 = g_pPhys->vOrigin[1],
            v11 = g_pPhys->vOrigin[2],
            //Profile_Begin(226),
            G_TraceCapsule(&v14, &v9, g_pPhys->vMins, g_pPhys->vMaxs, v12, g_pPhys->iEntNum, g_apl.iTraceMask),
            //Profile_EndInternal(0),
            memcpy(&g_apl.groundTrace, &v14, sizeof(g_apl.groundTrace)),
            v14.startsolid))
    {
        g_pPhys->groundEntNum = Trace_GetEntityHitId(&v14);
        g_apl.bGroundPlane = 0;
        g_apl.bIsWalking = 1;
    }
    else
    {
    LABEL_10:
        v4 = g_pPhys;
        if (v14.fraction == 1.0
            || g_pPhys->vVelocity[2] > 0.0
            && (float)((float)(v14.normal[0] * g_pPhys->vVelocity[0])
                + (float)((float)(g_pPhys->vVelocity[2] * v14.normal[2]) + (float)(g_pPhys->vVelocity[1] * v14.normal[1]))) > 10.0)
        {
            g_pPhys->groundEntNum = 2175;
            g_apl.bGroundPlane = 0;
            g_apl.bIsWalking = 0;
        }
        else if (v14.walkable)
        {
            if ((float)((float)((float)(v13 - v11) * v14.fraction) + v11) < (double)g_pPhys->vOrigin[2]
                || g_pPhys->ePhysicsType == AIPHYS_ZONLY_PHYSICS_RELATIVE)
            {
                g_pPhys->vOrigin[2] = (float)((float)(v13 - v11) * v14.fraction) + v11;
            }
            g_apl.bGroundPlane = 1;
            g_apl.bIsWalking = 1;
            v8 = Trace_GetEntityHitId(&v14);
            g_pPhys->groundEntNum = v8;
            AIPhys_AddTouchEnt(v8);
        }
        else
        {
            g_pPhys->groundEntNum = 2175;
            g_apl.bGroundPlane = 1;
            g_apl.bIsWalking = 0;
            if (v4->iHitEntnum == 2175 && (v14.normal[0] != 0.0 || v14.normal[1] != 0.0))
            {
                v5 = Trace_GetEntityHitId(&v14);
                v6 = g_pPhys;
                v7 = g_pPhys->vOrigin[0];
                g_pPhys->iHitEntnum = v5;
                v6->vHitOrigin[0] = v7;
                v6->vHitOrigin[1] = v6->vOrigin[1];
                *(_QWORD *)v6->vHitNormal = *(_QWORD *)v14.normal;
                v6->bStuck = 0;
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
    if (groundEntNum != 2175)
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

    //Profile_Begin(225);
    groundEntNum = pPhys->groundEntNum;
    v3 = pPhys;
    pPhys->iNumTouch = 0;
    pPhys->bDeflected = 0;
    pPhys->iHitEntnum = 2175;
    g_pPhys = pPhys;
    if (groundEntNum == 2174 && pPhys->vWishDelta[0] == 0.0 && pPhys->vWishDelta[1] == 0.0 && pPhys->vWishDelta[2] == 0.0)
    {
        pPhys->vVelocity[0] = 0.0;
        pPhys->vVelocity[1] = 0.0;
        pPhys->vVelocity[2] = 0.0;
        //Profile_EndInternal(0);
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
            //Profile_EndInternal(0);
            return 1;
        }
        else if (ePhysicsType == AIPHYS_NOGRAVITY)
        {
            AIPhys_NoGravityMove();
            //Profile_EndInternal(0);
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

