#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"

void __fastcall LocalToWorldOriginAndAngles(
    const float (*matrix)[3],
    const float *trans,
    const float *rot,
    float *origin,
    float *angles)
{
    double v8; // fp1
    float (*v9)[3]; // r3
    float v10[4][3]; // [sp+50h] [-80h] BYREF
    float v11[6][3]; // [sp+80h] [-50h] BYREF

    MatrixTransformVector43(trans, matrix, origin);
    v8 = RotationToYaw(rot);
    YawToAxis(v8, v9);
    MatrixMultiply(v10, matrix, v11);
    AxisToAngles(v11, angles);
}

void __fastcall CalcDeltaOriginAndAngles(
    DObj_s *obj,
    unsigned int anim,
    const float (*matrix)[3],
    float *origin,
    float *angles)
{
    double v8; // fp1
    float (*v9)[3]; // r3
    float v10[2]; // [sp+50h] [-A0h] BYREF
    float v11[18]; // [sp+58h] [-98h] BYREF
    float v12[6][3]; // [sp+A0h] [-50h] BYREF

    XAnimCalcAbsDelta(obj, anim, v10, v11);
    MatrixTransformVector43(v11, matrix, origin);
    v8 = RotationToYaw(v10);
    YawToAxis(v8, v9);
    MatrixMultiply((const float (*)[3]) & v11[6], matrix, v12);
    AxisToAngles(v12, angles);
}

void __fastcall GetDeltaOriginAndAngles(
    const XAnim_s *anims,
    unsigned int anim,
    const float (*matrix)[3],
    float *trans,
    float *origin,
    float *angles)
{
    double v10; // fp1
    float (*v11)[3]; // r3
    float v12[16]; // [sp+50h] [-A0h] BYREF
    float v13[8][3]; // [sp+90h] [-60h] BYREF

    XAnimGetAbsDelta(anims, anim, v12, trans, 1.0);
    MatrixTransformVector43(trans, matrix, origin);
    v10 = RotationToYaw(v12);
    YawToAxis(v10, v11);
    MatrixMultiply((const float (*)[3]) & v12[4], matrix, v13);
    AxisToAngles(v13, angles);
}

void __fastcall G_Animscripted_DeathPlant(
    gentity_s *ent,
    const XAnim_s *anims,
    unsigned int anim,
    float *origin,
    const float *angles)
{
    double v8; // fp12
    animscripted_s *scripted; // r31
    int number; // r8
    double fraction; // fp0
    double v14; // fp0
    double v15; // fp1
    float (*v16)[3]; // r3
    int v17; // r8
    double v18; // fp0
    float *v19; // r6
    float *v20; // r3
    float v21; // [sp+50h] [-160h] BYREF
    float v22; // [sp+54h] [-15Ch]
    float v23; // [sp+58h] [-158h]
    float v24; // [sp+60h] [-150h] BYREF
    float v25; // [sp+64h] [-14Ch]
    float v26; // [sp+68h] [-148h]
    float v27; // [sp+70h] [-140h] BYREF
    float v28; // [sp+74h] [-13Ch]
    float v29; // [sp+78h] [-138h]
    float v30; // [sp+80h] [-130h] BYREF
    float v31; // [sp+84h] [-12Ch]
    float v32; // [sp+88h] [-128h]
    float v33[4]; // [sp+90h] [-120h] BYREF
    float v34; // [sp+A0h] [-110h] BYREF
    float v35; // [sp+A4h] [-10Ch]
    float v36; // [sp+A8h] [-108h]
    float v37[4]; // [sp+B0h] [-100h] BYREF
    trace_t v38[2]; // [sp+C0h] [-F0h] BYREF
    float v39[4][3]; // [sp+120h] [-90h] BYREF

    v21 = *origin;
    v24 = v21;
    v8 = origin[2];
    scripted = ent->scripted;
    number = ent->s.number;
    v22 = origin[1];
    v23 = (float)v8 + (float)36.0;
    v25 = v22;
    v26 = (float)v8 - (float)18.0;
    G_TraceCapsule(v38, &v21, actorMins, actorMaxs, &v24, number, 8519697);
    if (v38[0].allsolid || (fraction = v38[0].fraction, v38[0].fraction >= 1.0))
    {
        scripted->axis[3][0] = *origin;
        scripted->axis[3][1] = origin[1];
        v14 = origin[2];
    }
    else
    {
        scripted->axis[3][0] = (float)((float)(v24 - v21) * v38[0].fraction) + v21;
        scripted->axis[3][1] = (float)((float)(v25 - v22) * (float)fraction) + v22;
        v14 = (float)((float)((float)(v26 - v23) * (float)fraction) + v23);
    }
    scripted->axis[3][2] = v14;
    scripted->fHeightOfs = 0.0;
    v28 = angles[1];
    v29 = angles[2];
    v27 = 0.0;
    AnglesToAxis(&v27, scripted->axis);
    XAnimGetAbsDelta(anims, anim, v37, &v30, 1.0);
    MatrixTransformVector43(&v30, scripted->axis, &v34);
    v15 = RotationToYaw(v37);
    YawToAxis(v15, v16);
    MatrixMultiply((const float (*)[3])v38[1].normal, scripted->axis, v39);
    AxisToAngles(v39, &v27);
    v21 = v34;
    v24 = v34;
    v17 = ent->s.number;
    v22 = v35;
    v25 = v35;
    v18 = (float)((float)__fsqrts((float)((float)(v30 * v30) + (float)((float)(v31 * v31) + (float)(v32 * v32))))
        + (float)128.0);
    v23 = (float)v18 + v36;
    v26 = v36 - (float)v18;
    G_TraceCapsule(v38, &v21, actorMins, actorMaxs, &v24, v17, 8519697);
    if (v38[0].allsolid || v38[0].fraction >= 1.0)
    {
        scripted->fEndPitch = 0.0;
        scripted->fEndRoll = 0.0;
    }
    else
    {
        v20 = (float *)ent->s.number;
        v33[0] = (float)((float)(v24 - v21) * v38[0].fraction) + v21;
        v33[1] = (float)((float)(v25 - v22) * v38[0].fraction) + v22;
        v33[2] = (float)((float)(v26 - v23) * v38[0].fraction) + v23;
        Actor_GetBodyPlantAngles(v20, 8519697, v33, v28, v19, &scripted->fEndPitch, &scripted->fEndRoll, 0);
    }
    if (XAnimGetLength(anims, anim) >= 1.0)
        scripted->fOrientLerp = 0.0;
    else
        scripted->fOrientLerp = -1.0;
}

void __fastcall G_AnimScripted_ClearAnimWeights(
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
        XAnimSetGoalWeightKnob(obj, animLookAtLeft, v11, 0.2, 1.0, v10, v9);
        goto LABEL_9;
    }
    v11 = XAnimGetWeight(pAnimTree, pActor->lookAtInfo.animLookAtRight);
    if (v11 > 0.0)
    {
        animLookAtLeft = pActor->lookAtInfo.animLookAtRight;
        goto LABEL_8;
    }
LABEL_9:
    XAnimSetCompleteGoalWeight(obj, pActor->lookAtInfo.animLookAtStraight, Weight, 0.2, 1.0, v10, v9, v13);
    if (!root)
        return;
    v14 = 0.2;
LABEL_12:
    XAnimClearTreeGoalWeightsStrict(pAnimTree, root, v14);
}

void __fastcall G_Animscripted(
    gentity_s *ent,
    float *origin,
    const float *angles,
    unsigned int anim,
    unsigned int root,
    unsigned int notifyName,
    unsigned __int8 animMode)
{
    const char *v13; // r3
    const char *v14; // r3
    XAnimTree_s *EntAnimTree; // r26
    animscripted_s *scripted; // r30
    const XAnim_s *Anims; // r3
    const XAnim_s *v18; // r27
    DObj_s *ServerDObj; // r29
    int v20; // r7
    unsigned int v21; // r6
    unsigned int v22; // r5
    double v23; // fp1
    float (*v24)[3]; // r3
    float v25[2]; // [sp+50h] [-110h] BYREF
    float v26[4]; // [sp+58h] [-108h] BYREF
    float v27[4]; // [sp+68h] [-F8h] BYREF
    float v28[18]; // [sp+78h] [-E8h] BYREF
    float v29[4][3]; // [sp+C0h] [-A0h] BYREF

    if ((ent->flags & 0x2000) == 0)
    {
        v13 = SL_ConvertToString(ent->classname);
        v14 = va("entity (classname: '%s') does not currently support animscripted", v13);
        Scr_ObjectError(v14);
    }
    EntAnimTree = GScr_GetEntAnimTree(ent);
    if (!EntAnimTree)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_animscripted.cpp", 163, 0, "%s", "pAnimTree");
    scripted = ent->scripted;
    if (!scripted)
    {
        scripted = (animscripted_s *)MT_Alloc(96, 18);
        ent->scripted = scripted;
    }
    scripted->anim = anim;
    scripted->root = root;
    scripted->mode = animMode;
    scripted->bStarted = 0;
    Anims = XAnimGetAnims(EntAnimTree);
    v18 = Anims;
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
    G_AnimScripted_ClearAnimWeights(ServerDObj, EntAnimTree, root, ent->actor);
    if (g_dumpAnimsCommands->current.integer == ent->s.number)
        DumpAnimCommand("animscripted(internal)", EntAnimTree, ent->scripted->anim, -1, 1.0, 0.2, 1.0);
    XAnimIsLooped(v18, anim);
    XAnimSetCompleteGoalWeight(ServerDObj, anim, 1.0, 0.2, 1.0, v22, v21, v20);
    G_FlagAnimForUpdate(ent);
    XAnimCalcAbsDelta(ServerDObj, anim, v25, v27);
    MatrixTransformVector43(v27, scripted->axis, v26);
    v23 = RotationToYaw(v25);
    YawToAxis(v23, v24);
    MatrixMultiply((const float (*)[3]) & v28[6], scripted->axis, v29);
    AxisToAngles(v29, v28);
    scripted->originError[0] = ent->r.currentOrigin[0] - v26[0];
    scripted->originError[1] = ent->r.currentOrigin[1] - v26[1];
    scripted->originError[2] = ent->r.currentOrigin[2] - v26[2];
    AnglesSubtract(ent->r.currentAngles, v28, scripted->anglesError);
}

void __fastcall G_ReduceOriginError(float *origin, float *originError, double maxChange)
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

void __fastcall G_ReduceAnglesError(float *angles, float *anglesError, double maxChange)
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

void __fastcall G_AnimScripted_Think_DeathPlant(gentity_s *ent, XAnimTree_s *tree, float *origin, float *angles)
{
    animscripted_s *scripted; // r30
    int number; // r8
    double v8; // fp12
    double v9; // fp0
    double v10; // fp13
    double v12; // fp11
    double v13; // fp13
    double v14; // fp12
    double v15; // fp0
    double fOrientLerp; // fp1
    double v17; // fp0
    float v18; // [sp+50h] [-80h] BYREF
    float v19; // [sp+54h] [-7Ch]
    float v20; // [sp+58h] [-78h]
    float v21; // [sp+60h] [-70h] BYREF
    float v22; // [sp+64h] [-6Ch]
    float v23; // [sp+68h] [-68h]
    trace_t v24; // [sp+70h] [-60h] BYREF

    scripted = ent->scripted;
    number = ent->s.number;
    v8 = origin[2];
    v9 = *origin;
    v10 = origin[1];
    v18 = *origin;
    v19 = v10;
    v20 = v8;
    v12 = (float)(scripted->fHeightOfs + (float)v8);
    v21 = v9;
    v22 = v10;
    v23 = v8;
    v20 = (float)v12 + (float)18.0;
    v23 = (float)(scripted->fHeightOfs - (float)18.0) + (float)v8;
    G_TraceCapsule(&v24, &v18, actorMins, actorMaxs, &v21, number, 8519697);
    if (!v24.allsolid && v24.fraction < 1.0)
    {
        v13 = (float)((float)((float)(v23 - v20) * v24.fraction) + v20);
        v14 = (float)((float)((float)(v21 - v18) * v24.fraction) + v18);
        v15 = (float)((float)((float)(v22 - v19) * v24.fraction) + v19);
        scripted->fHeightOfs = (float)((float)((float)(v23 - v20) * v24.fraction) + v20) - origin[2];
        origin[2] = v13;
        *origin = v14;
        origin[1] = v15;
    }
    if (scripted->fOrientLerp >= 0.0)
    {
        v17 = (float)(scripted->fOrientLerp + (float)0.050000001);
        scripted->fOrientLerp = scripted->fOrientLerp + (float)0.050000001;
        if (v17 > 1.0)
            scripted->fOrientLerp = 1.0;
        fOrientLerp = scripted->fOrientLerp;
    }
    else
    {
        fOrientLerp = XAnimGetTime(tree, scripted->anim);
    }
    *angles = (float)(scripted->fEndPitch * (float)fOrientLerp) + *angles;
    angles[2] = (float)(scripted->fEndRoll * (float)fOrientLerp) + angles[2];
}

void __fastcall G_AnimScripted_UpdateEntityOriginAndAngles(gentity_s *ent, float *origin, const float *angles)
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
        MatrixMultiply43((const float (*)[3])v9, v10, (float (*)[3])v8);
        v6 = v8[10];
        v7 = v8[11];
        ent->r.currentOrigin[0] = v8[9];
        ent->r.currentOrigin[1] = v6;
        ent->r.currentOrigin[2] = v7;
        AxisToAngles((const float (*)[3])v8, ent->r.currentAngles);
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

void __fastcall G_Animscripted_Think(gentity_s *ent)
{
    animscripted_s *scripted; // r31
    XAnimTree_s *EntAnimTree; // r28
    DObj_s *ServerDObj; // r29
    int v5; // r7
    unsigned int v6; // r6
    unsigned int v7; // r5
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
            G_ReduceAnglesError(v8, scripted->anglesError, (float)(ent->angleLerpRate * (float)0.050000001));
            G_AnimScripted_UpdateEntityOriginAndAngles(ent, v9, v8);
            if (scripted->bStarted)
            {
                if (XAnimHasFinished(EntAnimTree, scripted->anim))
                {
                    XAnimSetCompleteGoalWeight(ServerDObj, scripted->anim, 1.0, 0.2, 1.0, v7, v6, v5);
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
            MT_Free(scripted, 96);
            ent->scripted = 0;
        }
    }
}

void __fastcall GScr_GetStartOrigin()
{
    XAnimTree_s *v0; // r5
    const XAnim_s *Anims; // r3
    scr_anim_s *Anim; // [sp+50h] [-A0h]
    float v3[2]; // [sp+58h] [-98h] BYREF
    float v4[4]; // [sp+60h] [-90h] BYREF
    float v5[4]; // [sp+70h] [-80h] BYREF
    float v6[4]; // [sp+80h] [-70h] BYREF
    float v7[4]; // [sp+90h] [-60h] BYREF
    float v8[12]; // [sp+A0h] [-50h] BYREF

    Scr_GetVector(0, v4);
    Scr_GetVector(1u, v5);
    Anim = Scr_GetAnim((scr_anim_s *)2, 0, v0);
    Anims = Scr_GetAnims((unsigned __int16)Anim);
    XAnimGetAbsDelta(Anims, HIWORD(Anim), v3, v6, 0.0);
    v8[9] = v4[0];
    v8[10] = v4[1];
    v8[11] = v4[2];
    AnglesToAxis(v5, (float (*)[3])v8);
    MatrixTransformVector43(v6, (const float (*)[3])v8, v7);
    Scr_AddVector(v7);
}

void __fastcall GScr_GetStartAngles()
{
    XAnimTree_s *v0; // r5
    const XAnim_s *Anims; // r3
    double v2; // fp1
    float (*v3)[3]; // r3
    scr_anim_s *Anim; // [sp+50h] [-100h]
    float v5[4]; // [sp+58h] [-F8h] BYREF
    float v6[2]; // [sp+68h] [-E8h] BYREF
    float v7[4]; // [sp+70h] [-E0h] BYREF
    float v8[4]; // [sp+80h] [-D0h] BYREF
    float v9[12]; // [sp+90h] [-C0h] BYREF
    float v10[16]; // [sp+C0h] [-90h] BYREF
    float v11[4][3]; // [sp+100h] [-50h] BYREF

    Scr_GetVector(0, v5);
    Scr_GetVector(1u, v8);
    Anim = Scr_GetAnim((scr_anim_s *)2, 0, v0);
    Anims = Scr_GetAnims((unsigned __int16)Anim);
    XAnimGetAbsDelta(Anims, HIWORD(Anim), v6, v10, 0.0);
    v9[9] = v5[0];
    v9[10] = v5[1];
    v9[11] = v5[2];
    AnglesToAxis(v8, (float (*)[3])v9);
    v2 = RotationToYaw(v6);
    YawToAxis(v2, v3);
    MatrixMultiply((const float (*)[3]) & v10[4], (const float (*)[3])v9, v11);
    AxisToAngles(v11, v7);
    Scr_AddVector(v7);
}

void __fastcall GScr_GetCycleOriginOffset()
{
    XAnimTree_s *v0; // r5
    const XAnim_s *Anims; // r31
    scr_anim_s *Anim; // [sp+50h] [-B0h]
    float v3[2]; // [sp+58h] [-A8h] BYREF
    float v4[4]; // [sp+60h] [-A0h] BYREF
    float v5[4]; // [sp+70h] [-90h] BYREF
    float v6[4]; // [sp+80h] [-80h] BYREF
    float v7[4]; // [sp+90h] [-70h] BYREF
    float v8[4]; // [sp+A0h] [-60h] BYREF
    float v9[4][3]; // [sp+B0h] [-50h] BYREF

    Scr_GetVector(0, v7);
    Anim = Scr_GetAnim((scr_anim_s *)1, 0, v0);
    AnglesToAxis(v7, v9);
    Anims = Scr_GetAnims((unsigned __int16)Anim);
    XAnimGetAbsDelta(Anims, HIWORD(Anim), v3, v4, 0.0);
    XAnimGetAbsDelta(Anims, HIWORD(Anim), v3, v5, 1.0);
    v6[0] = v5[0] - v4[0];
    v6[1] = v5[1] - v4[1];
    v6[2] = v5[2] - v4[2];
    MatrixTransformVector(v6, v9, v8);
    Scr_AddVector(v8);
}

