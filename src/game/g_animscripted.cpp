#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"

#include <universal/com_math.h>
#include <xanim/xanim.h>
#include <script/scr_vm.h>
#include "g_scr_main.h"
#include <script/scr_memorytree.h>
#include "g_main.h"
#include <script/scr_animtree.h>
#include "actor_corpse.h"

void __cdecl LocalToWorldOriginAndAngles(
    const float (*matrix)[3],
    const float *trans,
    const float *rot,
    float *origin,
    float *angles)
{
    double v8; // fp1

    float v10[4][3]; // [sp+50h] [-80h] BYREF — YawToAxis output / MatrixMultiply in1
    float v11[6][3]; // [sp+80h] [-50h] BYREF — MatrixMultiply output

    MatrixTransformVector43(trans, (const mat4x3&)matrix, origin);
    v8 = RotationToYaw(rot);
    YawToAxis(v8, (mat3x3&)*v10);
    MatrixMultiply((const mat3x3&)*v10, (const mat3x3&)matrix, (mat3x3&)*v11);
    AxisToAngles((const mat3x3&)*v11, angles);
}

void __cdecl CalcDeltaOriginAndAngles(
    DObj_s *obj,
    unsigned int anim,
    const float (*matrix)[3],
    float *origin,
    float *angles)
{
    long double v8; // fp4
    double v9; // fp2
    double v10; // fp1
    double yaw; // fp1
    float (*v12)[3]; // r3
    float rot[2]; // [sp+50h] [-A0h] BYREF
    float trans[6]; // [sp+58h] [-98h] BYREF
    float localAxis[4][3]; // [sp+70h] [-80h] BYREF
    float resultAxis[4][3]; // [sp+A0h] [-50h] BYREF

    XAnimCalcAbsDelta(obj, anim, rot, trans);
    MatrixTransformVector43(trans, (const mat4x3&)*matrix, origin);
    yaw = RotationToYaw(rot);
    YawToAxis(yaw, (mat3x3&)*localAxis);
    MatrixMultiply((const mat3x3&)*localAxis, (const mat3x3&)*matrix, (mat3x3&)*resultAxis);
    AxisToAngles((const mat3x3&)*resultAxis, angles);
}

void __cdecl GetDeltaOriginAndAngles(
    const XAnim_s *anims,
    unsigned int anim,
    const float (*matrix)[3],
    float *trans,
    float *origin,
    float *angles)
{
    double v10; // fp1

    float v12[16]; // [sp+50h] [-A0h] BYREF — rot at v12[0..3], yawAxis at v12[4..12]
    float v13[8][3]; // [sp+90h] [-60h] BYREF — MatrixMultiply output

    XAnimGetAbsDelta(anims, anim, v12, trans, 1.0);
    MatrixTransformVector43(trans, (const mat4x3&)matrix, origin);
    v10 = RotationToYaw(v12);
    YawToAxis(v10, (mat3x3&)v12[4]);
    MatrixMultiply((const mat3x3&)v12[4], (const mat3x3&)matrix, (mat3x3&)*v13);
    AxisToAngles((const mat3x3&)*v13, angles);
}

void __cdecl G_Animscripted_DeathPlant(
    gentity_s *ent,
    const XAnim_s *anims,
    unsigned int anim,
    float *origin,
    const float *angles)
{
    animscripted_s *scripted; // r31
    int number; // r8
    float fraction; // fp0
    int v17; // r8
    double v18; // fp0
    float *pPitch; // r6
    int num; // r3
    float traceStart[3]; // was v21 (BYREF) + v22 + v23
    float traceEnd[3];   // was v24 (BYREF) + v25 + v26
    float anglesV[3];    // was v27 (BYREF) + yaw + v29
    float trans[3]; // [sp+80h] [-130h] BYREF
    float vOrigin[4]; // [sp+90h] [-120h] BYREF
    float transformedPos[3]; // [sp+A0h] [-110h] BYREF
    float rot[4]; // [sp+B0h] [-100h] BYREF
    trace_t trace; // [sp+C0h] [-F0h] BYREF
    mat3x3 newMat;

    scripted = ent->scripted;
    number = ent->s.number;

    traceStart[0] = origin[0];
    traceStart[1] = origin[1];
    traceStart[2] = origin[2] + 36.0f;

    traceEnd[0] = origin[0];
    traceEnd[1] = origin[1];
    traceEnd[2] = origin[2] - 18.0f;

    G_TraceCapsule(&trace, traceStart, actorMins, actorMaxs, traceEnd, number, 8519697);

    if (trace.allsolid || (fraction = trace.fraction, trace.fraction >= 1.0))
    {
        scripted->axis[3][0] = origin[0];
        scripted->axis[3][1] = origin[1];
        scripted->axis[3][2] = origin[2];
    }
    else
    {
        scripted->axis[3][0] = ((traceEnd[0] - traceStart[0]) * trace.fraction) + traceStart[0];
        scripted->axis[3][1] = ((traceEnd[1] - traceStart[1]) * fraction) + traceStart[1];
        scripted->axis[3][2] = (((traceEnd[2] - traceStart[2]) * fraction) + traceStart[2]);
    }
    scripted->fHeightOfs = 0.0;
    anglesV[1] = angles[1];
    anglesV[2] = angles[2];
    anglesV[0] = 0.0;
    AnglesToAxis(anglesV, scripted->axis);
    XAnimGetAbsDelta(anims, anim, rot, trans, 1.0);
    MatrixTransformVector43(trans, scripted->axis, transformedPos);
    mat3x3 yawAxis;
    YawToAxis(RotationToYaw(rot), yawAxis);
    MatrixMultiply(yawAxis, (const mat3x3&)scripted->axis, newMat);
    AxisToAngles(newMat, anglesV);
    traceStart[0] = transformedPos[0];
    traceEnd[0] = transformedPos[0];
    v17 = ent->s.number;
    traceStart[1] = transformedPos[1];
    traceEnd[1] = transformedPos[1];
    v18 = Vec3Length(trans) + 128.0f;
    traceStart[2] = (float)v18 + transformedPos[2];
    traceEnd[2] = transformedPos[2] - (float)v18;
    G_TraceCapsule(&trace, traceStart, actorMins, actorMaxs, traceEnd, v17, 8519697);
    if (trace.allsolid || trace.fraction >= 1.0)
    {
        scripted->fEndPitch = 0.0;
        scripted->fEndRoll = 0.0;
    }
    else
    {
        num = ent->s.number;
        vOrigin[0] = (float)((float)(traceEnd[0] - traceStart[0]) * trace.fraction) + traceStart[0];
        vOrigin[1] = (float)((float)(traceEnd[1] - traceStart[1]) * trace.fraction) + traceStart[1];
        vOrigin[2] = (float)((float)(traceEnd[2] - traceStart[2]) * trace.fraction) + traceStart[2];
        Actor_GetBodyPlantAngles(num, 8519697, vOrigin, anglesV[1], &scripted->fEndPitch, &scripted->fEndRoll, NULL);
    }
    if (XAnimGetLength(anims, anim) >= 1.0)
        scripted->fOrientLerp = 0.0;
    else
        scripted->fOrientLerp = -1.0;
}

void __cdecl G_AnimScripted_ClearAnimWeights(
    DObj_s *obj,
    XAnimTree_s *pAnimTree,
    unsigned int root,
    actor_s *pActor)
{
    double Weight; // fp28
    unsigned int v9; // r6
    unsigned int v10; // r5
    double v11; // fp1
    unsigned int animLookAtLeft; // r4
    int v13; // r7
    double v14; // fp1

    if (!pActor || pActor->lookAtInfo.fLookAtTurnAngle == 0.0 || !pActor->lookAtInfo.bLookAtSetup)
    {
        v14 = 0.2;
        goto LABEL_12;
    }
    Weight = XAnimGetWeight(pAnimTree, pActor->lookAtInfo.animLookAtStraight);
    v11 = XAnimGetWeight(pAnimTree, pActor->lookAtInfo.animLookAtLeft);
    if (v11 > 0.0)
    {
        animLookAtLeft = pActor->lookAtInfo.animLookAtLeft;
    LABEL_8:
        XAnimSetGoalWeightKnob(obj, animLookAtLeft, v11, 0.2, 1.0, 0, 0, 0);
        goto LABEL_9;
    }
    v11 = XAnimGetWeight(pAnimTree, pActor->lookAtInfo.animLookAtRight);
    if (v11 > 0.0)
    {
        animLookAtLeft = pActor->lookAtInfo.animLookAtRight;
        goto LABEL_8;
    }
LABEL_9:
    XAnimSetCompleteGoalWeight(obj, pActor->lookAtInfo.animLookAtStraight, Weight, 0.2, 1.0, 0, 0, 0);
    if (!root)
        return;
    v14 = 0.2;
LABEL_12:
    XAnimClearTreeGoalWeightsStrict(pAnimTree, root, v14);
}

void __cdecl G_Animscripted(
    gentity_s *ent,
    float *origin,
    const float *angles,
    unsigned int anim,
    unsigned int root,
    unsigned int notifyName,
    unsigned __int8 animMode)
{
    XAnimTree_s *pAnimTree; // r26
    animscripted_s *scripted; // r30
    const XAnim_s *Anims; // r3
    const XAnim_s *v20; // r27
    DObj_s *ServerDObj; // r29
    bool IsLooped; // r3
    int v23; // r7
    unsigned int v24; // r6
    unsigned int v25; // r5
    double yaw; // fp1
    float (*v27)[3]; // r3
    float rot[2]; // [sp+50h] [-110h] BYREF
    float transformedPos[4]; // [sp+58h] [-108h] BYREF
    float trans[4]; // [sp+68h] [-F8h] BYREF
    float axisAngles[6]; // [sp+78h] [-E8h] BYREF
    float localAxis[4][3]; // [sp+90h] [-D0h] BYREF
    float resultAxis[4][3]; // [sp+C0h] [-A0h] BYREF

    if ((ent->flags & 0x2000) == 0)
    {
        Scr_ObjectError(va("entity (classname: '%s') does not currently support animscripted", SL_ConvertToString(ent->classname)));
    }
    pAnimTree = GScr_GetEntAnimTree(ent);
    iassert(pAnimTree);
    scripted = ent->scripted;
    if (!scripted)
    {
        scripted = (animscripted_s *)MT_Alloc(sizeof(animscripted_s), 18);
        ent->scripted = scripted;
    }
    scripted->anim = anim;
    scripted->root = root;
    scripted->mode = animMode;
    scripted->bStarted = 0;
    Anims = XAnimGetAnims(pAnimTree);
    v20 = Anims;
    if (animMode == 1)
    {
        G_Animscripted_DeathPlant(ent, Anims, anim, origin, angles);
    }
    else
    {
        scripted->axis[3][0] = *origin;
        scripted->axis[3][1] = origin[1];
        scripted->axis[3][2] = origin[2];
        AnglesToAxis(angles, scripted->axis);
    }
    ServerDObj = Com_GetServerDObj(ent->s.number);
    G_AnimScripted_ClearAnimWeights(ServerDObj, pAnimTree, root, ent->actor);
    if (g_dumpAnimsCommands->current.integer == ent->s.number)
        DumpAnimCommand("animscripted(internal)", pAnimTree, ent->scripted->anim, -1, 1.0, 0.2, 1.0);
    IsLooped = XAnimIsLooped(v20, anim);
    XAnimSetCompleteGoalWeight(ServerDObj, anim, 1.0, 0.2, 1.0, notifyName, 0, !IsLooped);
    G_FlagAnimForUpdate(ent);
    XAnimCalcAbsDelta(ServerDObj, anim, rot, trans);
    MatrixTransformVector43(trans, scripted->axis, transformedPos);
    yaw = RotationToYaw(rot);
    YawToAxis(yaw, (mat3x3&)*localAxis);
    MatrixMultiply((const mat3x3&)*localAxis, (const mat3x3&)*scripted->axis, (mat3x3&)*resultAxis);
    AxisToAngles((const mat3x3&)*resultAxis, axisAngles);
    scripted->originError[0] = ent->r.currentOrigin[0] - transformedPos[0];
    scripted->originError[1] = ent->r.currentOrigin[1] - transformedPos[1];
    scripted->originError[2] = ent->r.currentOrigin[2] - transformedPos[2];
    AnglesSubtract(ent->r.currentAngles, axisAngles, scripted->anglesError);
}

void __cdecl G_ReduceOriginError(float *origin, float *originError, double maxChange)
{
    double v6; // fp1
    double v7; // fp1
    double v8; // fp9
    double v9; // fp8
    double v10; // fp7
    double v11; // fp11
    double v12; // fp10
    double v13; // fp12

    v6 = (float)((float)(originError[2] * originError[2])
        + (float)((float)(*originError * *originError) + (float)(originError[1] * originError[1])));
    if (v6 != 0.0)
    {
        v7 = I_rsqrt(v6);
        if ((float)((float)v7 * (float)maxChange) >= 1.0)
        {
            *origin = *origin + *originError;
            origin[1] = origin[1] + originError[1];
            origin[2] = origin[2] + originError[2];
            *originError = 0.0;
            originError[1] = 0.0;
            originError[2] = 0.0;
        }
        else
        {
            v8 = (float)(*origin + *originError);
            v9 = originError[1];
            v10 = originError[2];
            v11 = origin[1];
            *origin = (float)(*originError * (float)((float)v7 * (float)maxChange)) + *origin;
            v12 = origin[2];
            origin[1] = (float)((float)((float)v7 * (float)maxChange) * originError[1]) + (float)v11;
            v13 = *origin;
            origin[2] = (float)((float)((float)v7 * (float)maxChange) * originError[2]) + (float)v12;
            *originError = (float)v8 - (float)v13;
            originError[1] = (float)((float)v11 + (float)v9) - origin[1];
            originError[2] = (float)((float)v12 + (float)v10) - origin[2];
        }
    }
}

void __cdecl G_ReduceAnglesError(float *angles, float *anglesError, double maxChange)
{
    float *v4; // r31
    int v5; // r30
    int v6; // r29
    double v7; // fp0

    v4 = anglesError;
    v5 = (char *)angles - (char *)anglesError;
    v6 = 3;
    do
    {
        v7 = *v4;
        if (v7 != 0.0)
        {
            if (v7 <= maxChange)
            {
                if (v7 >= -maxChange)
                {
                    *(float *)((char *)v4 + v5) = *v4 + *(float *)((char *)v4 + v5);
                    *v4 = 0.0;
                }
                else
                {
                    *v4 = *v4 + (float)maxChange;
                    *(float *)((char *)v4 + v5) = AngleNormalize360((float)(*(float *)((char *)v4 + v5) - (float)maxChange));
                }
            }
            else
            {
                *v4 = *v4 - (float)maxChange;
                *(float *)((char *)v4 + v5) = AngleNormalize360((float)((float)maxChange + *(float *)((char *)v4 + v5)));
            }
        }
        --v6;
        ++v4;
    } while (v6);
}

void __cdecl G_AnimScripted_Think_DeathPlant(gentity_s *ent, XAnimTree_s *tree, float *origin, float *angles)
{
    animscripted_s *scripted; // r30
    int number; // r8
    float fOrientLerp; // fp1

    float traceStart[3]; // was v18 (BYREF) + v19 + v20
    float traceEnd[3];   // was v21 (BYREF) + v22 + v23
    trace_t trace; // [sp+70h] [-60h] BYREF

    scripted = ent->scripted;
    number = ent->s.number;

    Vec3Copy(origin, traceStart);
    Vec3Copy(origin, traceEnd);

    traceStart[2] = scripted->fHeightOfs + origin[2] + 18.0f;
    traceEnd[2] = (float)(scripted->fHeightOfs - (float)18.0) + (float)origin[2];

    G_TraceCapsule(&trace, traceStart, actorMins, actorMaxs, traceEnd, number, 8519697);
    if (!trace.allsolid && trace.fraction < 1.0)
    {
        scripted->fHeightOfs = (((traceEnd[2] - traceStart[2]) * trace.fraction) + traceStart[2]) - origin[2];

        float delta[3];
        delta[0] = traceEnd[0] - traceStart[0];
        delta[1] = traceEnd[1] - traceStart[1];
        delta[2] = traceEnd[2] - traceStart[2];
        Vec3Mad(traceStart, trace.fraction, delta, origin);
        //origin[0] = (((traceEnd[0] - traceStart[0]) * trace.fraction) + traceStart[0]);
        //origin[1] = (((traceEnd[1] - traceStart[1]) * trace.fraction) + traceStart[1]);
        //origin[2] = (((traceEnd[2] - traceStart[2]) * trace.fraction) + traceStart[2]);
    }
    if (scripted->fOrientLerp >= 0.0)
    {
        scripted->fOrientLerp = scripted->fOrientLerp + 0.05f;
        if (scripted->fOrientLerp > 1.0)
            scripted->fOrientLerp = 1.0;
        fOrientLerp = scripted->fOrientLerp;
    }
    else
    {
        fOrientLerp = XAnimGetTime(tree, scripted->anim);
    }
    angles[0] = (scripted->fEndPitch * fOrientLerp) + angles[0];
    angles[2] = (scripted->fEndRoll * fOrientLerp) + angles[2];
}

void __cdecl G_AnimScripted_UpdateEntityOriginAndAngles(gentity_s *ent, float *origin, const float *angles)
{
    double v6; // fp13
    double v7; // fp12
    float v8[12]; // [sp+50h] [-B0h] BYREF
    float v9[12]; // [sp+80h] [-80h] BYREF
    float v10[6][3]; // [sp+B0h] [-50h] BYREF

    if (ent->tagInfo)
    {
        G_CalcTagParentRelAxis(ent, v10);
        v9[9] = *origin;
        v9[10] = origin[1];
        v9[11] = origin[2];
        AnglesToAxis(angles, (float (*)[3])v9);
        MatrixMultiply43((const mat4x3&)v9, (const mat4x3&)v10,(mat4x3&)v8);
        v6 = v8[10];
        v7 = v8[11];
        ent->r.currentOrigin[0] = v8[9];
        ent->r.currentOrigin[1] = v6;
        ent->r.currentOrigin[2] = v7;
        AxisToAngles((const mat3x3&)v8, ent->r.currentAngles);
        G_CalcTagAxis(ent, 0);
    }
    else
    {
        ent->r.currentOrigin[0] = *origin;
        ent->r.currentOrigin[1] = origin[1];
        ent->r.currentOrigin[2] = origin[2];
        ent->r.currentAngles[0] = *angles;
        ent->r.currentAngles[1] = angles[1];
        ent->r.currentAngles[2] = angles[2];
    }
}

void __cdecl G_Animscripted_Think(gentity_s *ent)
{
    animscripted_s *scripted; // r31
    XAnimTree_s *EntAnimTree; // r28
    DObj_s *ServerDObj; // r29
    float v8[4]; // [sp+50h] [-50h] BYREF
    float v9[16]; // [sp+60h] [-40h] BYREF

    scripted = ent->scripted;
    if (scripted)
    {
        EntAnimTree = G_GetEntAnimTree(ent);
        if (EntAnimTree && scripted->anim)
        {
            ServerDObj = Com_GetServerDObj(ent->s.number);
            CalcDeltaOriginAndAngles(ServerDObj, scripted->anim, scripted->axis, v9, v8);
            if (scripted->mode == 1)
                G_AnimScripted_Think_DeathPlant(ent, EntAnimTree, v9, v8);
            G_ReduceOriginError(v9, scripted->originError, 0.25);
            G_ReduceAnglesError(v8, scripted->anglesError, (float)(ent->angleLerpRate * 0.05f));
            G_AnimScripted_UpdateEntityOriginAndAngles(ent, v9, v8);
            if (scripted->bStarted)
            {
                if (XAnimHasFinished(EntAnimTree, scripted->anim))
                {
                    XAnimSetCompleteGoalWeight(ServerDObj, scripted->anim, 1.0, 0.2, 1.0, 0, 0, 0);
                    scripted->anim = 0;
                }
            }
            else
            {
                scripted->bStarted = 1;
            }
        }
        else
        {
            MT_Free((unsigned char*)scripted, 96);
            ent->scripted = 0;
        }
    }
}

void __cdecl GScr_GetStartOrigin()
{
    XAnimTree_s *v0; // r5
    const XAnim_s *Anims; // r3
    scr_anim_s Anim; // [sp+50h] [-A0h]
    float v3[2]; // [sp+58h] [-98h] BYREF
    float v4[4]; // [sp+60h] [-90h] BYREF
    float v5[4]; // [sp+70h] [-80h] BYREF
    float v6[4]; // [sp+80h] [-70h] BYREF
    float v7[4]; // [sp+90h] [-60h] BYREF
    float v8[12]; // [sp+A0h] [-50h] BYREF

    Scr_GetVector(0, v4);
    Scr_GetVector(1u, v5);
    Anim = Scr_GetAnim(2, NULL);
    Anims = Scr_GetAnims(Anim.tree);
    XAnimGetAbsDelta(Anims, Anim.index, v3, v6, 0.0);
    v8[9] = v4[0];
    v8[10] = v4[1];
    v8[11] = v4[2];
    AnglesToAxis(v5, (float (*)[3])v8);
    MatrixTransformVector43(v6, (const mat4x3&)v8, v7);
    Scr_AddVector(v7);
}

void __cdecl GScr_GetStartAngles()
{
    float pos[3];
    float angles[3];

    Scr_GetVector(0, pos);
    Scr_GetVector(1, angles);

    scr_anim_s anim = Scr_GetAnim(2, NULL);
    const XAnim_s *anims = Scr_GetAnims(anim.tree);

    float rot[3];
    float trans[3];
    XAnimGetAbsDelta(anims, anim.index, rot, trans, 0.0f);

    float axis[3][3];
    axis[2][0] = pos[0];
    axis[2][1] = pos[1];
    axis[2][2] = pos[2];

    AnglesToAxis(angles, axis);

    float yaw = RotationToYaw(rot);

    mat3x3 yawAxis;
    YawToAxis(yaw, yawAxis);

    mat3x3 result;
    MatrixMultiply(yawAxis, axis, result);

    float finalAngles[3];
    AxisToAngles(result, finalAngles);

    Scr_AddVector(finalAngles);
}

void __cdecl GScr_GetCycleOriginOffset()
{
    XAnimTree_s *v0; // r5
    const XAnim_s *Anims; // r31
    scr_anim_s Anim; // [sp+50h] [-B0h]
    float v3[2]; // [sp+58h] [-A8h] BYREF
    float v4[4]; // [sp+60h] [-A0h] BYREF
    float v5[4]; // [sp+70h] [-90h] BYREF
    float v6[4]; // [sp+80h] [-80h] BYREF
    float v7[4]; // [sp+90h] [-70h] BYREF
    float v8[4]; // [sp+A0h] [-60h] BYREF
    float v9[4][3]; // [sp+B0h] [-50h] BYREF

    Scr_GetVector(0, v7);
    Anim = Scr_GetAnim(1, NULL);
    AnglesToAxis(v7, v9);
    Anims = Scr_GetAnims(Anim.tree);
    XAnimGetAbsDelta(Anims, Anim.index, v3, v4, 0.0);
    XAnimGetAbsDelta(Anims, Anim.index, v3, v5, 1.0);
    v6[0] = v5[0] - v4[0];
    v6[1] = v5[1] - v4[1];
    v6[2] = v5[2] - v4[2];
    MatrixTransformVector(v6, (const mat3x3&)v9, v8);
    Scr_AddVector(v8);
}

