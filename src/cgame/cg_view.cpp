#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_view.h"
#include <EffectsCore/fx_system.h>
#include "cg_main.h"
#include "cg_actors.h"
#include <qcommon/cmd.h>
#include <gfx_d3d/r_reflection_probe.h>
#include <script/scr_const.h>
#include "cg_ents.h"
#include "cg_draw.h"
#include <qcommon/threads.h>
#include <gfx_d3d/r_cinematic.h>
#include <gfx_d3d/r_workercmds_common.h>
#include <aim_assist/aim_assist.h>
#include <gfx_d3d/r_dpvs.h>
#include "cg_predict.h"
#include "cg_modelpreviewer.h"
#include <client/cl_input.h>
#include <game/g_local.h>
#include "cg_snapshot.h"

ClientViewParams clientViewParamsArray[1][1] = { { { 0.0, 0.0, 1.0, 1.0 } } };
TestEffect s_testEffect[1];

void __cdecl TRACK_cg_view()
{
    track_static_alloc_internal(clientViewParamsArray, 16, "clientViewParamsArray", 10);
    track_static_alloc_internal(s_testEffect, 84, "s_testEffect", 9);
}

void __cdecl CG_PlayTestFx(int localClientNum)
{
    TestEffect *v2; // r30
    const FxEffectDef *v3; // r28
    int time; // r29
    float v5[20]; // [sp+50h] [-50h] BYREF

    v2 = &s_testEffect[localClientNum];
    v3 = FX_Register(v2->name);
    v5[0] = 0.0;
    v5[1] = 0.0;
    v5[4] = 0.0;
    v5[5] = 0.0;
    v5[2] = 1.0;
    v5[3] = 1.0;
    v5[6] = 0.0;
    v5[7] = 1.0;
    v5[8] = 0.0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    time = cgArray[0].time;
    FX_PlayOrientedEffect(localClientNum, v3, cgArray[0].time, v2->pos, (const float (*)[3])v5);
    v2->time = time;
}

void __cdecl CG_UpdateTestFX(int localClientNum)
{
    TestEffect *v2; // r31

    v2 = &s_testEffect[localClientNum];
    if (v2->respawnTime >= 1 && CG_GetLocalClientTime(localClientNum) > v2->time + v2->respawnTime)
        CG_PlayTestFx(localClientNum);
}

void __cdecl CG_FxSetTestPosition()
{
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            191,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    if (cgArray[0].nextSnap)
    {
        Vec3Mad(cgArray[0].refdef.vieworg, 100.0, cgArray[0].refdef.viewaxis[0], s_testEffect[0].pos);
        Com_Printf(
            21,
            "\n\nFX Testing position set to: (%f, %f, %f)\n\n",
            s_testEffect[0].pos[0],
            s_testEffect[0].pos[1],
            s_testEffect[0].pos[2]);
    }
}

void __cdecl CG_FxTest()
{
    const char *v0; // r3
    const char *v1; // r3
    long double v2; // fp2

    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            191,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    if (cgArray[0].nextSnap)
    {
        if (Cmd_Argc() < 2)
            Com_Printf(21, "Must supply filename from base path.  Optional restart time.\n");
        v0 = Cmd_Argv(1);
        I_strncpyz(s_testEffect[0].name, v0, 64);
        if (I_strncmp(s_testEffect[0].name, "fx/", 3))
        {
            Com_Printf(21, "Spawning Fx %s\n", s_testEffect[0].name);
            CG_PlayTestFx(0);
            if (Cmd_Argc() == 3)
            {
                v1 = Cmd_Argv(2);
                v2 = atof(v1);
                s_testEffect[0].respawnTime = (int)(*(double *)&v2 * 1000.0);
            }
            else
            {
                s_testEffect[0].respawnTime = 0;
            }
        }
        else
        {
            Com_PrintError(1, "Fx path [%s] must not inclue \"fx/\" \n", s_testEffect[0].name);
        }
    }
}

void __cdecl CG_CalcVrect(int localClientNum)
{
    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    cgArray[0].refdef.x = cgsArray[0].viewX;
    cgArray[0].refdef.y = cgsArray[0].viewY;
    cgArray[0].refdef.width = cgsArray[0].viewWidth;
    cgArray[0].refdef.height = cgsArray[0].viewHeight;
    cgArray[0].refdef.useScissorViewport = 0;
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob)
{
    float *p_stepViewChange; // r28
    int *p_stepViewStart; // r31
    int *p_time; // r30
    __int64 v5; // r11 OVERLAPPED
    int v6; // r9 OVERLAPPED
    double v7; // fp0

    p_stepViewChange = &cgameGlob->stepViewChange;
    if (cgameGlob->stepViewChange != 0.0)
    {
        p_stepViewStart = &cgameGlob->stepViewStart;
        p_time = &cgameGlob->time;
        if (cgameGlob->time - cgameGlob->stepViewStart < 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
                216,
                0,
                "%s",
                "cgameGlob->time - cgameGlob->stepViewStart >= 0");
        LODWORD(v5) = *p_time - *p_stepViewStart;
        v6 = (int)(float)(cg_viewZSmoothingTime->current.value * (float)1000.0);
        if ((int)v5 < v6)
        {
            if ((int)v5 >= 0)
            {
                HIDWORD(v5) = (int)(float)(cg_viewZSmoothingTime->current.value * (float)1000.0);
                v7 = (float)((float)v5 / (float)*(__int64 *)((char *)&v5 + 4));
            }
            else
            {
                v7 = 0.0;
            }
        }
        else
        {
            v7 = 1.0;
        }
        cgameGlob->refdef.vieworg[2] = -(float)((float)((float)((float)1.0 - (float)v7) * *p_stepViewChange)
            - cgameGlob->refdef.vieworg[2]);
    }
}

void __cdecl CG_KickAngles(cg_s *cgameGlob)
{
    unsigned int ViewmodelWeaponIndex; // r22
    WeaponDef *WeaponDef; // r23
    int frametime; // r26
    __int64 v5; // r9
    int v6; // r31
    float *kickAngles; // r11
    int v8; // r10
    double v9; // fp13
    double v10; // fp0
    double v11; // fp0
    double v12; // fp0
    double v13; // fp0

    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    WeaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
    frametime = cgameGlob->frametime;
    if (frametime > 0)
    {
        HIDWORD(v5) = 0x82000000;
        do
        {
            if (frametime <= 5)
                v6 = frametime;
            else
                v6 = 5;
            LODWORD(v5) = v6;
            kickAngles = cgameGlob->kickAngles;
            v8 = 3;
            do
            {
                v9 = *kickAngles;
                if (*(kickAngles - 3) == 0.0)
                {
                    if (v9 == 0.0)
                        goto LABEL_30;
                LABEL_11:
                    if (v9 <= 0.0)
                        v10 = 1.0;
                    else
                        v10 = -1.0;
                    if (ViewmodelWeaponIndex)
                    {
                        if (cgameGlob->predictedPlayerState.fWeaponPosFrac <= 0.5)
                            v11 = (float)(WeaponDef->fHipViewKickCenterSpeed * (float)v10);
                        else
                            v11 = (float)(WeaponDef->fAdsViewKickCenterSpeed * (float)v10);
                    }
                    else
                    {
                        v11 = (float)((float)v10 * (float)2400.0);
                    }
                    *(kickAngles - 3) = (float)((float)v11 * (float)((float)v5 * (float)0.001)) + *(kickAngles - 3);
                    goto LABEL_20;
                }
                if (v9 != 0.0)
                    goto LABEL_11;
            LABEL_20:
                v12 = (float)(*(kickAngles - 3) * (float)((float)v5 * (float)0.001));
                if ((float)((float)v9 * (float)(*(kickAngles - 3) * (float)((float)v5 * (float)0.001))) < 0.0)
                    v12 = (float)((float)(*(kickAngles - 3) * (float)((float)v5 * (float)0.001)) * (float)0.059999999);
                v13 = (float)((float)v9 + (float)v12);
                if ((float)((float)v13 * (float)v9) < 0.0)
                {
                    *kickAngles = 0.0;
                }
                else
                {
                    *kickAngles = v13;
                    if (v13 != 0.0)
                    {
                        if (fabs(v13) <= 10.0)
                            goto LABEL_30;
                        if (v13 <= 0.0)
                            *kickAngles = -10.0;
                        else
                            *kickAngles = 10.0;
                    }
                }
                *(kickAngles - 3) = 0.0;
            LABEL_30:
                --v8;
                ++kickAngles;
            } while (v8);
            frametime -= 5;
        } while (frametime > 0);
    }
}

// local variable allocation has failed, the output may be wrong!
float __cdecl CG_GetVerticalBobFactor(
    const playerState_s *predictedPlayerState,
    double cycle,
    double speed,
    double maxAmp)
{
    int viewHeightTarget; // r11
    double v5; // fp1
    double v6; // fp29
    const dvar_s *v7; // r11
    double v8; // fp31
    double v9; // fp1
    double v10; // fp1
    long double v11; // fp2
    double v12; // fp30
    long double v13; // fp2
    double v14; // fp1

    viewHeightTarget = predictedPlayerState->viewHeightTarget;
    v6 = v5;
    if (viewHeightTarget == 11)
    {
        v7 = bg_bobAmplitudeProne;
    }
    else if (viewHeightTarget == 40)
    {
        v7 = bg_bobAmplitudeDucked;
    }
    else if ((predictedPlayerState->pm_flags & 0x8000) != 0)
    {
        v7 = bg_bobAmplitudeSprinting;
    }
    else
    {
        v7 = bg_bobAmplitudeStanding;
    }
    v8 = (float)(v7->current.vector[1] * (float)speed);
    if (v8 > maxAmp)
        v8 = maxAmp;
    v10 = (float)((float)v9 * (float)2.0);
    v11 = sin(*(long double *)&speed);
    v12 = (float)*(double *)&v11;
    *(double *)&v11 = (float)((float)((float)v6 * (float)4.0) + (float)1.5707964);
    v13 = sin(v11);
    v14 = (float)((float)((float)((float)((float)*(double *)&v13 * (float)0.2) + (float)v12) * (float)v8) * (float)0.75);
    return *((float *)&v14 + 1);
}

// local variable allocation has failed, the output may be wrong!
float __cdecl CG_GetHorizontalBobFactor(
    const playerState_s *predictedPlayerState,
    double cycle,
    double speed,
    double maxAmp)
{
    int viewHeightTarget; // r11
    const dvar_s *v5; // r11
    double v6; // fp31
    long double v7; // fp2
    double v8; // fp1

    viewHeightTarget = predictedPlayerState->viewHeightTarget;
    if (viewHeightTarget == 11)
    {
        v5 = bg_bobAmplitudeProne;
    }
    else if (viewHeightTarget == 40)
    {
        v5 = bg_bobAmplitudeDucked;
    }
    else if ((predictedPlayerState->pm_flags & 0x8000) != 0)
    {
        v5 = bg_bobAmplitudeSprinting;
    }
    else
    {
        v5 = bg_bobAmplitudeStanding;
    }
    v6 = (float)(v5->current.value * (float)speed);
    if (v6 > maxAmp)
        v6 = maxAmp;
    v7 = sin(*(long double *)&speed);
    v8 = (float)((float)*(double *)&v7 * (float)v6);
    return *((float *)&v8 + 1);
}

void __cdecl CG_CalculateView_IdleAngles(cg_s *cgameGlob, float *angles)
{
    playerEntity_t *p_playerEntity; // r27
    unsigned int ViewmodelWeaponIndex; // r28
    WeaponDef *WeaponDef; // r30
    __int64 v7; // r9
    int IsAimDownSightWeapon; // r3
    long double v9; // fp2
    __int64 v10; // r11
    double fIdleProneFactor; // fp0
    double fHipIdleAmount; // fp10
    double hipIdleSpeed; // fp11
    int eFlags; // r11
    double fLastIdleFactor; // fp13
    double v16; // fp13
    double v17; // fp13
    __int64 v18; // r10
    double v19; // fp31
    long double v20; // fp2
    __int64 v21; // r11
    long double v22; // fp2

    p_playerEntity = &cgameGlob->playerEntity;
    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    WeaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
    if (WeaponDef->overlayReticle)
    {
        IsAimDownSightWeapon = BG_IsAimDownSightWeapon(ViewmodelWeaponIndex);
        HIDWORD(v10) = 108264;
        fIdleProneFactor = 1.0;
        if (IsAimDownSightWeapon)
        {
            fHipIdleAmount = (float)((float)((float)(WeaponDef->fAdsIdleAmount - WeaponDef->fHipIdleAmount)
                * cgameGlob->predictedPlayerState.fWeaponPosFrac)
                + WeaponDef->fHipIdleAmount);
            hipIdleSpeed = (float)((float)((float)(WeaponDef->adsIdleSpeed - WeaponDef->hipIdleSpeed)
                * cgameGlob->predictedPlayerState.fWeaponPosFrac)
                + WeaponDef->hipIdleSpeed);
        }
        else if (WeaponDef->fHipIdleAmount == 0.0)
        {
            hipIdleSpeed = 1.0;
            fHipIdleAmount = 80.0;
        }
        else
        {
            hipIdleSpeed = WeaponDef->hipIdleSpeed;
            fHipIdleAmount = WeaponDef->fHipIdleAmount;
        }
        eFlags = cgameGlob->predictedPlayerEntity.nextState.lerp.eFlags;
        if ((eFlags & 8) != 0)
        {
            fIdleProneFactor = WeaponDef->fIdleProneFactor;
        }
        else if ((eFlags & 4) != 0)
        {
            fIdleProneFactor = WeaponDef->fIdleCrouchFactor;
        }
        if (WeaponDef->overlayReticle == WEAPOVERLAYRETICLE_NONE)
            goto LABEL_19;
        if (cgameGlob->predictedPlayerState.fWeaponPosFrac == 0.0)
            goto LABEL_19;
        fLastIdleFactor = p_playerEntity->fLastIdleFactor;
        if (fIdleProneFactor == fLastIdleFactor)
            goto LABEL_19;
        if (fIdleProneFactor <= fLastIdleFactor)
        {
            LODWORD(v7) = cgameGlob->frametime;
            v17 = (float)-(float)((float)((float)v7 * (float)0.00050000002) - p_playerEntity->fLastIdleFactor);
            p_playerEntity->fLastIdleFactor = -(float)((float)((float)v7 * (float)0.00050000002)
                - p_playerEntity->fLastIdleFactor);
            if (v17 < fIdleProneFactor)
                LABEL_18:
            p_playerEntity->fLastIdleFactor = fIdleProneFactor;
        }
        else
        {
            LODWORD(v7) = cgameGlob->frametime;
            v16 = (float)((float)((float)v7 * (float)0.00050000002) + p_playerEntity->fLastIdleFactor);
            p_playerEntity->fLastIdleFactor = (float)((float)v7 * (float)0.00050000002) + p_playerEntity->fLastIdleFactor;
            if (v16 > fIdleProneFactor)
                goto LABEL_18;
        }
    LABEL_19:
        LODWORD(v10) = cgameGlob->frametime;
        HIDWORD(v18) = 109528;
        v19 = (float)((float)((float)(p_playerEntity->fLastIdleFactor * (float)fHipIdleAmount)
            * cgameGlob->predictedPlayerState.fWeaponPosFrac)
            * cgameGlob->predictedPlayerState.holdBreathScale);
        LODWORD(v18) = (int)(float)((float)((float)v10 * cgameGlob->predictedPlayerState.holdBreathScale)
            * (float)hipIdleSpeed)
            + cgameGlob->weapIdleTime;
        cgameGlob->weapIdleTime = v18;
        *(double *)&v9 = (float)((float)v18 * (float)0.00069999998);
        v20 = sin(v9);
        angles[1] = (float)((float)((float)*(double *)&v20 * (float)v19) * (float)0.0099999998) + angles[1];
        LODWORD(v21) = cgameGlob->weapIdleTime;
        *(double *)&v20 = (float)((float)v21 * (float)0.001);
        v22 = sin(v20);
        *angles = (float)((float)((float)*(double *)&v22 * (float)v19) * (float)0.0099999998) + *angles;
    }
}

void __cdecl CG_CalculateView_BobAngles(const cg_s *cgameGlob, float *angles)
{
    unsigned int ViewmodelWeaponIndex; // r3
    double v5; // fp10
    double v6; // fp9
    double v7; // fp12
    double v8; // fp0

    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    if (BG_GetWeaponDef(ViewmodelWeaponIndex)->overlayReticle)
    {
        v5 = angles[1];
        v6 = angles[2];
        v7 = (float)(cgameGlob->vAngOfs[1] * cgameGlob->predictedPlayerState.fWeaponPosFrac);
        v8 = (float)(cgameGlob->vAngOfs[2] * cgameGlob->predictedPlayerState.fWeaponPosFrac);
        *angles = *angles + (float)(cgameGlob->predictedPlayerState.fWeaponPosFrac * cgameGlob->vAngOfs[0]);
        angles[1] = (float)v5 + (float)v7;
        angles[2] = (float)v6 + (float)v8;
    }
}

void __cdecl CG_AddGroundTiltToAngles(int localClientNum, float *angles, const cg_s *cgameGlob)
{
    float v4[4][3]; // [sp+50h] [-A0h] BYREF
    float v5[4][3]; // [sp+80h] [-70h] BYREF
    float v6[4][3]; // [sp+B0h] [-40h] BYREF

    if (cgameGlob->predictedPlayerState.groundTiltAngles[0] != 0.0
        || cgameGlob->predictedPlayerState.groundTiltAngles[1] != 0.0
        || cgameGlob->predictedPlayerState.groundTiltAngles[2] != 0.0)
    {
        AnglesToAxis(cgameGlob->predictedPlayerState.groundTiltAngles, v4);
        AnglesToAxis(angles, v5);
        MatrixMultiply((const mat3x3&)v5, (const mat3x3 &)v4, (mat3x3 &)v6);
        AxisToAngles((const mat3x3 &)v6, angles);
    }
}

void __cdecl OffsetFirstPersonView(int localClientNum, cg_s *cgameGlob)
{
    unsigned int ViewmodelWeaponIndex; // r3
    WeaponDef *weapDef; // r3
    WeaponDef *v6; // r21
    float *refdefViewAngles; // r30
    __int64 v8; // r11
    int pm_type; // r11
    double v10; // fp13
    double v11; // fp12
    int damageTime; // r11
    double v13; // fp31
    __int64 v14; // r11
    double v15; // fp0
    double v16; // fp12
    float *p_fWeaponPosFrac; // r24
    double VerticalBobFactor; // fp1
    const dvar_s *v19; // r11
    double HorizontalBobFactor; // fp31
    double v21; // fp10
    double v22; // fp13
    double v23; // fp9
    double v24; // fp11
    __int64 v25; // r11
    double v26; // fp0
    float *v27; // r11
    double v28; // fp0
    float v29[6]; // [sp+58h] [-98h] BYREF

    if ((cgameGlob->predictedPlayerState.eFlags & 0x300) == 0)
    {
        ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
        weapDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
        HIDWORD(v8) = 108060;
        v6 = weapDef;
        refdefViewAngles = cgameGlob->refdefViewAngles;
        cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.viewHeightCurrent + cgameGlob->refdef.vieworg[2];
        LODWORD(v8) = cgameGlob->predictedPlayerState.pm_type;
        if ((_DWORD)v8 != 3 && (_DWORD)v8 != 2)
        {
            pm_type = cgameGlob->nextSnap->ps.pm_type;
            if (pm_type == 5)
            {
                cgameGlob->refdefViewAngles[2] = 40.0;
                *refdefViewAngles = -15.0;
                LODWORD(v8) = cgameGlob->nextSnap->ps.stats[1];
                cgameGlob->refdefViewAngles[1] = (float)v8;
                return;
            }
            if (pm_type != 6)
            {
                CG_KickAngles(cgameGlob);
                v10 = cgameGlob->refdefViewAngles[1];
                v11 = cgameGlob->refdefViewAngles[2];
                *refdefViewAngles = *refdefViewAngles + cgameGlob->kickAngles[0];
                cgameGlob->refdefViewAngles[1] = cgameGlob->kickAngles[1] + (float)v10;
                cgameGlob->refdefViewAngles[2] = cgameGlob->kickAngles[2] + (float)v11;
                CG_CalculateView_IdleAngles(cgameGlob, cgameGlob->refdefViewAngles);
                CG_CalculateView_BobAngles(cgameGlob, cgameGlob->refdefViewAngles);
                damageTime = cgameGlob->damageTime;
                if (damageTime)
                {
                    v13 = (float)((float)1.0 - (float)(cgameGlob->predictedPlayerState.fWeaponPosFrac * (float)0.5));
                    if (cgameGlob->predictedPlayerState.fWeaponPosFrac != 0.0 && v6->overlayReticle)
                        v13 = (float)((float)((float)(cgameGlob->predictedPlayerState.fWeaponPosFrac * (float)0.5) + (float)1.0)
                            * (float)((float)1.0 - (float)(cgameGlob->predictedPlayerState.fWeaponPosFrac * (float)0.5)));
                    HIDWORD(v14) = cgameGlob->time;
                    LODWORD(v14) = HIDWORD(v14) - damageTime;
                    if ((float)v14 < 100.0)
                    {
                        v15 = (float)(GetLeanFraction((float)((float)v14 * (float)0.0099999998)) * (float)v13);
                    LABEL_15:
                        v16 = cgameGlob->refdefViewAngles[2];
                        *refdefViewAngles = (float)(cgameGlob->v_dmg_pitch * (float)v15) + *refdefViewAngles;
                        cgameGlob->refdefViewAngles[2] = (float)(cgameGlob->v_dmg_roll * (float)v15) + (float)v16;
                        goto LABEL_16;
                    }
                    if ((float)-(float)((float)((float)((float)v14 - (float)100.0) * (float)0.0024999999) - (float)1.0) > 0.0)
                    {
                        v15 = (float)((float)((float)1.0
                            - GetLeanFraction((float)((float)1.0
                                - (float)-(float)((float)((float)((float)v14 - (float)100.0)
                                    * (float)0.0024999999)
                                    - (float)1.0))))
                            * (float)v13);
                        goto LABEL_15;
                    }
                }
            LABEL_16:
                p_fWeaponPosFrac = &cgameGlob->predictedPlayerState.fWeaponPosFrac;
                if (cgameGlob->predictedPlayerState.fWeaponPosFrac != 0.0 && v6->fAdsViewBobMult != 0.0)
                {
                    *refdefViewAngles = -(float)((float)((float)(CG_GetVerticalBobFactor(
                        &cgameGlob->predictedPlayerState,
                        cgameGlob->fBobCycle,
                        cgameGlob->xyspeed,
                        45.0)
                        * *p_fWeaponPosFrac)
                        * v6->fAdsViewBobMult)
                        - *refdefViewAngles);
                    cgameGlob->refdefViewAngles[1] = -(float)((float)((float)(CG_GetHorizontalBobFactor(
                        &cgameGlob->predictedPlayerState,
                        cgameGlob->fBobCycle,
                        cgameGlob->xyspeed,
                        45.0)
                        * *p_fWeaponPosFrac)
                        * v6->fAdsViewBobMult)
                        - cgameGlob->refdefViewAngles[1]);
                }
                VerticalBobFactor = CG_GetVerticalBobFactor(
                    &cgameGlob->predictedPlayerState,
                    cgameGlob->fBobCycle,
                    cgameGlob->xyspeed,
                    bg_bobMax->current.value);
                v19 = bg_bobMax;
                cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + (float)VerticalBobFactor;
                HorizontalBobFactor = CG_GetHorizontalBobFactor(
                    &cgameGlob->predictedPlayerState,
                    cgameGlob->fBobCycle,
                    cgameGlob->xyspeed,
                    v19->current.value);
                AngleVectors(cgameGlob->refdefViewAngles, 0, v29, 0);
                v21 = cgameGlob->refdef.vieworg[1];
                v22 = v29[1];
                cgameGlob->refdef.vieworg[0] = (float)(v29[0] * (float)HorizontalBobFactor) + cgameGlob->refdef.vieworg[0];
                v23 = cgameGlob->refdef.vieworg[2];
                v24 = v29[2];
                cgameGlob->refdef.vieworg[1] = (float)((float)v22 * (float)HorizontalBobFactor) + (float)v21;
                cgameGlob->refdef.vieworg[2] = (float)((float)v24 * (float)HorizontalBobFactor) + (float)v23;
                HIDWORD(v25) = cgameGlob->time;
                LODWORD(v25) = HIDWORD(v25) - cgameGlob->landTime;
                v26 = (float)v25;
                if (v26 <= 0.0)
                    goto LABEL_25;
                if (v26 >= 150.0)
                {
                    if (v26 >= 450.0)
                        goto LABEL_25;
                    v27 = &cgameGlob->refdef.vieworg[2];
                    v28 = (float)((float)((float)-(float)((float)((float)((float)v26 - (float)150.0) * (float)0.0033333334)
                        - (float)1.0)
                        * cgameGlob->landChange)
                        + cgameGlob->refdef.vieworg[2]);
                }
                else
                {
                    v27 = &cgameGlob->refdef.vieworg[2];
                    v28 = (float)((float)((float)((float)v26 * (float)0.0066666668) * cgameGlob->landChange)
                        + cgameGlob->refdef.vieworg[2]);
                }
                *v27 = v28;
            LABEL_25:
                AddLeanToPosition(
                    cgameGlob->refdef.vieworg,
                    cgameGlob->refdefViewAngles[1],
                    cgameGlob->predictedPlayerState.leanf,
                    16.0,
                    20.0);
                CG_AddGroundTiltToAngles(localClientNum, cgameGlob->refdefViewAngles, cgameGlob);
                if (cgameGlob->refdef.vieworg[2] < (double)(float)(cgameGlob->predictedPlayerState.origin[2] + (float)8.0))
                    cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2] + (float)8.0;
            }
        }
    }
}
float __cdecl CG_GetViewFov(int localClientNum)
{
    unsigned int ViewmodelWeaponIndex; // r30
    WeaponDef *WeaponDef; // r29
    double value; // fp30
    double v7; // fp13
    unsigned int v8; // r3
    double v9; // fp1

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
    WeaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
    value = cg_fov->current.value;
    if (value < 1.0 || value > 160.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            613,
            1,
            "%s\n\t(viewFov) = %g",
            HIDWORD(value),
            LODWORD(value));
    if (BG_IsAimDownSightWeapon(ViewmodelWeaponIndex))
    {
        _FP12 = (float)(WeaponDef->fAdsZoomFov - (float)value);
        __asm { fsel      f11, f12, f30, f13 }
        if (cgArray[0].predictedPlayerState.fWeaponPosFrac == 1.0)
        {
            value = _FP11;
        }
        else if (cgArray[0].predictedPlayerState.fWeaponPosFrac != 0.0)
        {
            v7 = cgArray[0].playerEntity.bPositionToADS ? WeaponDef->fAdsZoomInFrac : WeaponDef->fAdsZoomOutFrac;
            if ((float)(cgArray[0].predictedPlayerState.fWeaponPosFrac - (float)((float)1.0 - (float)v7)) > 0.0
                && (float)((float)(cgArray[0].predictedPlayerState.fWeaponPosFrac - (float)((float)1.0 - (float)v7)) / (float)v7) > 0.0)
            {
                value = (float)-(float)((float)((float)((float)value - (float)_FP11)
                    * (float)((float)(cgArray[0].predictedPlayerState.fWeaponPosFrac
                        - (float)((float)1.0 - (float)v7))
                        / (float)v7))
                    - (float)value);
            }
        }
    }
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) != 0)
    {
        v8 = CG_PlayerTurretWeaponIdx(localClientNum);
        if (BG_GetWeaponDef(v8)->overlayInterface == WEAPOVERLAYINTERFACE_TURRETSCOPE)
            v9 = turretScopeZoom->current.value;
        else
            v9 = 55.0;
    }
    else
    {
        v9 = value;
    }
    return *((float *)&v9 + 1);
}

void __cdecl CG_CalcFov(int localClientNum)
{
    long double v2; // fp2
    long double v3; // fp2
    double v4; // fp29
    long double v5; // fp2

    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    *(double *)&v2 = (float)((float)(CG_GetViewFov(localClientNum) * (float)0.017453292) * (float)0.5);
    v3 = tan(v2);
    v4 = (float)*(double *)&v3;
    cgArray[0].refdef.tanHalfFovX = cgsArray[0].viewAspect * (float)((float)*(double *)&v3 * (float)0.75);
    cgArray[0].refdef.tanHalfFovY = (float)*(double *)&v3 * (float)0.75;
    *(double *)&v3 = (float)((float)(cg_fov->current.value * (float)0.017453292) * (float)0.5);
    v5 = tan(v3);
    cgArray[0].zoomSensitivity = (float)v4 / (float)*(double *)&v5;
}

float __cdecl CG_GetViewZoomScale()
{
    double v0; // fp1

    v0 = (float)(cgArray[0].refdef.tanHalfFovY * (float)1.5890048);
    return *((float *)&v0 + 1);
}

void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob)
{
    playerState_s *p_predictedPlayerState; // r10
    int *p_fromAlignOrg; // r3
    int cubemapSize; // r5
    CubemapShot cubemapShot; // r4

    p_predictedPlayerState = &cgameGlob->predictedPlayerState;
    cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
    cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
    cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
    cubemapSize = cgameGlob->cubemapSize;
    cubemapShot = cgameGlob->cubemapShot;
    p_fromAlignOrg = &cgameGlob->predictedPlayerState.hud.elem[123].fromAlignOrg;
    *((float *)p_fromAlignOrg + 5823) = p_predictedPlayerState->viewHeightCurrent + *((float *)p_fromAlignOrg + 5823);
    R_CalcCubeMapViewValues((refdef_s *)(p_fromAlignOrg + 5815), cubemapShot, cubemapSize);
}

void __cdecl CG_CalcVehicleViewValues(int localClientNum)
{
    centity_s *Entity; // r30
    DObj_s *ClientDObj; // r3
    DObj_s *v4; // r29
    double v5; // fp31
    double v6; // fp11
    double v7; // fp0
    __int64 v8; // r10
    double v9; // fp30
    double v10; // fp31
    float *v11; // r5
    double v12; // fp13
    float v13; // [sp+50h] [-200h] BYREF
    float v14; // [sp+54h] [-1FCh]
    float v15; // [sp+58h] [-1F8h]
    float v16; // [sp+60h] [-1F0h] BYREF
    float v17; // [sp+64h] [-1ECh]
    float v18[12]; // [sp+70h] [-1E0h] BYREF
    float v19[4]; // [sp+A0h] [-1B0h] BYREF
    float v20[4]; // [sp+B0h] [-1A0h] BYREF
    float v21[4]; // [sp+C0h] [-190h] BYREF
    __int64 v22; // [sp+D0h] [-180h]
    float v23[9]; // [sp+E0h] [-170h] BYREF
    float v24[3]; // [sp+104h] [-14Ch] BYREF
    float v25[4][3]; // [sp+110h] [-140h] BYREF
    float v26[4][3]; // [sp+140h] [-110h] BYREF
    float v27[21]; // [sp+170h] [-E0h] BYREF
    float v28[3]; // [sp+1C4h] [-8Ch] BYREF
    float v29[4]; // [sp+1D0h] [-80h] BYREF
    float v30[4][3]; // [sp+1E0h] [-70h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Entity = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum);
    ClientDObj = Com_GetClientDObj(Entity->nextState.number, 0);
    v4 = ClientDObj;
    if (ClientDObj
        && CG_DObjGetWorldTagMatrix(&Entity->pose, ClientDObj, scr_const.tag_body, (float (*)[3])v23, v24)
        && CG_DObjGetWorldTagMatrix(&Entity->pose, v4, scr_const.tag_player, (float (*)[3]) & v27[12], v28))
    {
        v18[0] = v23[0];
        v18[1] = v23[1];
        v18[2] = v23[2];
        v18[3] = v23[3];
        v18[4] = v23[4];
        v18[5] = v23[5];
        v18[6] = v23[6];
        v18[7] = v23[7];
        v18[8] = v23[8];
        v18[9] = v28[0];
        v18[10] = v28[1];
        v18[11] = v28[2];
        if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0
            || cgArray[0].predictedPlayerState.viewlocked_entNum == 2175)
        {
            if (cgArray[0].predictedPlayerState.vehicleType != 5)
                MatrixTranspose((const float (*)[3])v18, cgArray[0].prevVehicleInvAxis);
        }
        else
        {
            if (cgArray[0].vehicleInitView)
            {
                MatrixTranspose((const float (*)[3])v18, cgArray[0].prevVehicleInvAxis);
                cgArray[0].vehicleInitView = 0;
            }
            MatrixMultiply(cgArray[0].prevVehicleInvAxis, (const float (*)[3])v18, (float (*)[3])v27);
            MatrixTranspose((const float (*)[3])v18, cgArray[0].prevVehicleInvAxis);
            if (cgArray[0].predictedPlayerState.vehicleType == 5)
            {
                if ((cgArray[0].predictedPlayerState.eFlags & 0x40000) != 0)
                {
                    AxisToAngles((const float (*)[3])v18, &v16);
                    if (!cgArray[0].vehicleViewLocked)
                    {
                        cgArray[0].vehicleViewLockedAngles[0] = AngleSubtract(cgArray[0].predictedPlayerState.viewangles[0], v16);
                        cgArray[0].vehicleViewLockedAngles[1] = AngleSubtract(cgArray[0].predictedPlayerState.viewangles[1], v17);
                        cgArray[0].vehicleViewLocked = 1;
                    }
                    v5 = (float)(AngleSubtract(Entity->nextState.lerp.apos.trBase[1], Entity->currentState.apos.trBase[1])
                        * (float)20.0);
                    v19[0] = Entity->nextState.lerp.pos.trBase[0] - Entity->currentState.pos.trBase[0];
                    v19[1] = Entity->nextState.lerp.pos.trBase[1] - Entity->currentState.pos.trBase[1];
                    v19[2] = Entity->nextState.lerp.pos.trBase[2] - Entity->currentState.pos.trBase[2];
                    MatrixTransposeTransformVector(v19, (const float (*)[3])v23, v29);
                    v6 = Entity->pose.angles[0];
                    v7 = Entity->pose.angles[2];
                    LODWORD(v8) = cgArray[0].frametime;
                    v22 = v8;
                    v9 = (float)-(float)((float)(vehHelicopterHeadSwayOnRollHorz->current.value * (float)v7)
                        - (float)(vehHelicopterHeadSwayOnYaw->current.value * (float)v5));
                    v10 = (float)((float)v8 * (float)0.001);
                    cgArray[0].vehicleViewLockedAngles[0] = DiffTrackAngle(
                        (float)((float)((float)((float)(vehHelicopterHeadSwayOnRollVert->current.value
                            * (float)v7)
                            * (float)v7)
                            * (float)0.001)
                            + (float)-(float)(vehHelicopterHeadSwayOnPitch->current.value
                                * (float)v6)),
                        cgArray[0].vehicleViewLockedAngles[0],
                        vehHelicopterFreeLookReleaseSpeed->current.value,
                        v10);
                    cgArray[0].vehicleViewLockedAngles[1] = DiffTrackAngle(
                        v9,
                        cgArray[0].vehicleViewLockedAngles[1],
                        vehHelicopterFreeLookReleaseSpeed->current.value,
                        v10);
                    cgArray[0].predictedPlayerState.viewangles[0] = cgArray[0].vehicleViewLockedAngles[0] + v16;
                    cgArray[0].predictedPlayerState.viewangles[1] = cgArray[0].vehicleViewLockedAngles[1] + v17;
                    v13 = (float)(cgArray[0].vehicleViewLockedAngles[0] + v16) - cgArray[0].predictedPlayerState.delta_angles[0];
                    v14 = cgArray[0].predictedPlayerState.viewangles[1] - cgArray[0].predictedPlayerState.delta_angles[1];
                    v15 = cgArray[0].predictedPlayerState.viewangles[2] - cgArray[0].predictedPlayerState.delta_angles[2];
                    CL_SetViewAngles(localClientNum, &v13);
                    MatrixTranspose((const float (*)[3])v18, v30);
                    AnglesToAxis(cgArray[0].predictedPlayerState.viewangles, v26);
                    MatrixMultiply(v26, v30, v25);
                    AxisToAngles(v25, &v13);
                    cgArray[0].predictedPlayerState.viewangles[2] = -v15;
                    return;
                }
                cgArray[0].vehicleViewLocked = 0;
            }
            v20[0] = 0.0;
            v20[1] = 0.0;
            v20[2] = 0.0;
            v20[3] = 1.0;
            AxisToQuat((const float (*)[3])v27, v21);
            QuatLerp(v20, v21, cg_viewVehicleInfluence->current.value, v11);
            QuatToAxis(v21, (float (*)[3])v27);
            AnglesToAxis(cgArray[0].predictedPlayerState.viewangles, v26);
            MatrixMultiply(v26, (const float (*)[3])v27, v25);
            AxisToAngles(v25, &v13);
            cgArray[0].predictedPlayerState.viewangles[0] = v13;
            if (cgArray[0].predictedPlayerState.vehicleType == 5 || (cgArray[0].predictedPlayerState.eFlags & 0x40000) != 0)
            {
                v12 = v14;
                cgArray[0].predictedPlayerState.viewangles[1] = v14;
            }
            else
            {
                v12 = cgArray[0].predictedPlayerState.viewangles[1];
            }
            v13 = v13 - cgArray[0].predictedPlayerState.delta_angles[0];
            v14 = (float)v12 - cgArray[0].predictedPlayerState.delta_angles[1];
            v15 = cgArray[0].predictedPlayerState.viewangles[2] - cgArray[0].predictedPlayerState.delta_angles[2];
            CL_SetViewAngles(localClientNum, &v13);
            MatrixMultiply(v26, cgArray[0].prevVehicleInvAxis, v25);
            AxisToAngles(v25, &v13);
            cgArray[0].predictedPlayerState.viewangles[2] = -v15;
        }
    }
}

void CalcTurretViewValues(int localClientNum)
{
    int viewlocked_entNum; // r4
    centity_s *Entity; // r30
    DObj_s *ClientDObj; // r4
    WeaponDef *weapDef; // r29
    WeaponDef *v6; // r30

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) != 0)
    {
        if (cgArray[0].predictedPlayerState.viewlocked == PLAYERVIEWLOCK_NONE)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp", 860, 0, "%s", "ps->viewlocked");
        viewlocked_entNum = cgArray[0].predictedPlayerState.viewlocked_entNum;
        if (cgArray[0].predictedPlayerState.viewlocked_entNum == 2175)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
                861,
                0,
                "%s",
                "ps->viewlocked_entNum != ENTITYNUM_NONE");
            viewlocked_entNum = cgArray[0].predictedPlayerState.viewlocked_entNum;
        }
        Entity = CG_GetEntity(localClientNum, viewlocked_entNum);
        ClientDObj = Com_GetClientDObj(Entity->nextState.number, 0);
        if (ClientDObj)
        {
            if (!CG_DObjGetWorldTagPos(&Entity->pose, ClientDObj, scr_const.tag_player, cgArray[0].refdef.vieworg))
                Com_Error(ERR_DROP, "Turret has no bone: tag_player");
            if (cgArray[0].predictedPlayerState.viewlocked == PLAYERVIEWLOCK_WEAPONJITTER && !cg_paused->current.integer)
            {
                weapDef = BG_GetWeaponDef(Entity->nextState.weapon);
                cgArray[0].refdefViewAngles[0] = (float)(crandom() * weapDef->vertViewJitter) + cgArray[0].refdefViewAngles[0];
                v6 = BG_GetWeaponDef(Entity->nextState.weapon);
                cgArray[0].refdefViewAngles[1] = (float)(crandom() * v6->horizViewJitter) + cgArray[0].refdefViewAngles[1];
            }
        }
    }
}

void __cdecl CG_CalcLinkedViewValues(int localClientNum)
{
    float v1[4]; // [sp+50h] [-E0h] BYREF
    float v2[4][3]; // [sp+60h] [-D0h] BYREF
    float v3[4][3]; // [sp+90h] [-A0h] BYREF
    float v4[4][3]; // [sp+C0h] [-70h] BYREF
    float v5[4][3]; // [sp+F0h] [-40h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.pm_type == 1
        && (cgArray[0].predictedPlayerState.eFlags & 0x20300) == 0
        && (cgArray[0].predictedPlayerState.pm_flags & 0x1000000) == 0)
    {
        AnglesToAxis(cgArray[0].predictedPlayerState.linkAngles, v4);
        MatrixTranspose((const mat3x3&)v4, (mat3x3 &)v2);
        AnglesToAxis(cgArray[0].predictedPlayerState.viewangles, v3);
        MatrixMultiply((const mat3x3 &)v3, (const mat3x3 &)v2, (mat3x3 &)v5);
        AxisToAngles((const mat3x3 &)v5, v1);
        cgArray[0].predictedPlayerState.viewangles[2] = -v1[2];
    }
}

void __cdecl CG_ApplyViewAnimation(int localClientNum)
{
    int ViewmodelWeaponIndex; // r4
    weaponInfo_s *LocalClientWeaponInfo; // r3
    DObj_s **p_viewModelDObj; // r30

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.pm_type != 3
        && cgArray[0].predictedPlayerState.pm_type != 2
        && (cgArray[0].predictedPlayerState.eFlags & 0x300) == 0
        && (cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0)
    {
        ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
        if (ViewmodelWeaponIndex > 0)
        {
            LocalClientWeaponInfo = CG_GetLocalClientWeaponInfo(localClientNum, ViewmodelWeaponIndex);
            p_viewModelDObj = &LocalClientWeaponInfo->viewModelDObj;
            if (LocalClientWeaponInfo->viewModelDObj)
            {
                cgArray[0].viewModelAxis[0][0] = cgArray[0].refdef.viewaxis[0][0];
                cgArray[0].viewModelAxis[0][1] = cgArray[0].refdef.viewaxis[0][1];
                cgArray[0].viewModelAxis[0][2] = cgArray[0].refdef.viewaxis[0][2];
                cgArray[0].viewModelAxis[1][0] = cgArray[0].refdef.viewaxis[1][0];
                cgArray[0].viewModelAxis[1][1] = cgArray[0].refdef.viewaxis[1][1];
                cgArray[0].viewModelAxis[1][2] = cgArray[0].refdef.viewaxis[1][2];
                cgArray[0].viewModelAxis[2][0] = cgArray[0].refdef.viewaxis[2][0];
                cgArray[0].viewModelAxis[2][1] = cgArray[0].refdef.viewaxis[2][1];
                cgArray[0].viewModelAxis[2][2] = cgArray[0].refdef.viewaxis[2][2];
                cgArray[0].viewModelAxis[3][0] = cgArray[0].refdef.vieworg[0];
                cgArray[0].viewModelAxis[3][1] = cgArray[0].refdef.vieworg[1];
                cgArray[0].viewModelAxis[3][2] = cgArray[0].refdef.vieworg[2];
                CG_UpdateViewModelPose(LocalClientWeaponInfo->viewModelDObj);
                if (CG_DObjGetWorldTagMatrix(
                    &cgArray[0].viewModelPose,
                    *p_viewModelDObj,
                    scr_const.tag_camera,
                    cgArray[0].refdef.viewaxis,
                    cgArray[0].refdef.vieworg))
                {
                    AxisToAngles(cgArray[0].refdef.viewaxis, cgArray[0].refdefViewAngles);
                }
            }
        }
    }
}

float angles[3];
int oldTime;
int oldMsec;
int moveMsec;
int __cdecl PausedClientFreeMove(int localClientNum)
{
    int integer; // r11
    int CurrentCmdNumber; // r3
    cg_s *LocalClientGlobals; // r30
    int v6; // r26
    double value; // fp12
    __int64 v8; // r11
    double v9; // fp31
    __int64 v10; // r9
    double v11; // fp12
    double v12; // fp11
    double v13; // fp10
    double v14; // fp11
    double v15; // fp10
    unsigned __int64 v16; // [sp+50h] [-C0h] BYREF
    float v17; // [sp+58h] [-B8h]
    unsigned __int64 v18; // [sp+60h] [-B0h] BYREF
    float v19; // [sp+68h] [-A8h]
    __int64 v20; // [sp+70h] [-A0h] BYREF
    float v21; // [sp+78h] [-98h]
    usercmd_s v22; // [sp+80h] [-90h] BYREF

    integer = cg_paused->current.integer;
    if (!integer || integer != 2 && !cl_freemove->current.integer)
        return 0;
    CurrentCmdNumber = CL_GetCurrentCmdNumber(localClientNum);
    if (CL_GetUserCmd(localClientNum, CurrentCmdNumber, &v22))
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        v16 = __PAIR64__(v22.angles[1], v22.angles[0]);
        v18 = __PAIR64__(v22.angles[2], v22.angles[1]);
        v20 = (unsigned int)v22.angles[2] | 0x1000000000000LL;
        angles[0] = (float)__SPAIR64__(v22.angles[1], v22.angles[0]) * (float)0.0054931641;
        angles[1] = (float)__SPAIR64__(v22.angles[2], v22.angles[1]) * (float)0.0054931641;
        angles[2] = (float)v20 * (float)0.0054931641;
        angles[0] = LocalClientGlobals->predictedPlayerState.delta_angles[0] + angles[0];
        angles[1] = LocalClientGlobals->predictedPlayerState.delta_angles[1] + angles[1];
        angles[2] = LocalClientGlobals->predictedPlayerState.delta_angles[2] + angles[2];
        if (oldTime != LocalClientGlobals->time)
        {
            oldTime = LocalClientGlobals->time;
            oldMsec = Sys_Milliseconds();
            moveMsec = 0;
        }
        v6 = Sys_Milliseconds();
        value = cl_freemoveScale->current.value;
        LocalClientGlobals->refdefViewAngles[0] = angles[0];
        LocalClientGlobals->refdefViewAngles[1] = angles[1];
        LocalClientGlobals->refdefViewAngles[2] = angles[2];
        *((float *)&v16 + 1) = 0.0;
        *(float *)&v16 = 0.0;
        v17 = 1.0;
        LODWORD(v8) = v6 - oldMsec;
        oldMsec = v6;
        v20 = v8;
        v9 = (float)((float)((float)v8 * (float)value) * (float)0.050000001);
        AnglesToAxis(LocalClientGlobals->refdefViewAngles, LocalClientGlobals->refdef.viewaxis);
        *(float *)&v18 = LocalClientGlobals->refdef.viewaxis[0][0];
        *((float *)&v18 + 1) = LocalClientGlobals->refdef.viewaxis[0][1];
        v19 = LocalClientGlobals->refdef.viewaxis[0][2];
        Vec3Cross((const float *)&v16, (const float *)&v18, (float *)&v20);
        Vec3Cross((const float *)&v20, (const float *)&v16, (float *)&v18);
        if (v22.rightmove || v22.forwardmove || v22.upmove)
        {
            HIDWORD(v10) = moveMsec;
            if (!moveMsec)
            {
                HIDWORD(v10) = v6;
                moveMsec = v6;
            }
            if (v22.rightmove)
            {
                v11 = *(float *)&v20;
                v12 = *((float *)&v20 + 1);
                LODWORD(v10) = -v22.rightmove;
                v13 = v21;
                v20 = v10;
                LocalClientGlobals->predictedPlayerState.origin[0] = (float)((float)v11
                    * (float)((float)((float)v10 * (float)0.2) * (float)v9))
                    + LocalClientGlobals->predictedPlayerState.origin[0];
                LocalClientGlobals->predictedPlayerState.origin[1] = (float)((float)v12
                    * (float)((float)((float)v10 * (float)0.2) * (float)v9))
                    + LocalClientGlobals->predictedPlayerState.origin[1];
                LocalClientGlobals->predictedPlayerState.origin[2] = (float)((float)v13
                    * (float)((float)((float)v10 * (float)0.2) * (float)v9))
                    + LocalClientGlobals->predictedPlayerState.origin[2];
            }
            if (v22.forwardmove)
            {
                LODWORD(v10) = v22.forwardmove;
                v14 = *((float *)&v18 + 1);
                v15 = v19;
                v20 = v10;
                LocalClientGlobals->predictedPlayerState.origin[0] = (float)((float)((float)((float)v10 * (float)0.2) * (float)v9)
                    * *(float *)&v18)
                    + LocalClientGlobals->predictedPlayerState.origin[0];
                LocalClientGlobals->predictedPlayerState.origin[1] = (float)((float)v14
                    * (float)((float)((float)v10 * (float)0.2) * (float)v9))
                    + LocalClientGlobals->predictedPlayerState.origin[1];
                LocalClientGlobals->predictedPlayerState.origin[2] = (float)((float)v15
                    * (float)((float)((float)v10 * (float)0.2) * (float)v9))
                    + LocalClientGlobals->predictedPlayerState.origin[2];
            }
            if (v6 - HIDWORD(v10) < 250)
                v9 = (float)((float)v9 * (float)0.25);
            if (v22.upmove)
            {
                LODWORD(v10) = v22.upmove;
                v20 = v10;
                LocalClientGlobals->predictedPlayerState.origin[0] = (float)((float)((float)((float)v10 * (float)0.2) * (float)v9)
                    * (float)0.0)
                    + LocalClientGlobals->predictedPlayerState.origin[0];
                LocalClientGlobals->predictedPlayerState.origin[1] = LocalClientGlobals->predictedPlayerState.origin[1]
                    + (float)((float)((float)((float)v10 * (float)0.2) * (float)v9)
                        * (float)0.0);
                LocalClientGlobals->predictedPlayerState.origin[2] = LocalClientGlobals->predictedPlayerState.origin[2]
                    + (float)((float)((float)v10 * (float)0.2) * (float)v9);
            }
        }
        else
        {
            moveMsec = v22.rightmove;
        }
        LocalClientGlobals->refdef.vieworg[0] = LocalClientGlobals->predictedPlayerState.origin[0];
        LocalClientGlobals->refdef.vieworg[1] = LocalClientGlobals->predictedPlayerState.origin[1];
        LocalClientGlobals->refdef.vieworg[2] = LocalClientGlobals->predictedPlayerState.origin[2];
        LocalClientGlobals->refdef.vieworg[2] = LocalClientGlobals->predictedPlayerState.viewHeightCurrent
            + LocalClientGlobals->refdef.vieworg[2];
        LocalClientGlobals->refdefViewAngles[0] = angles[0];
        LocalClientGlobals->refdefViewAngles[1] = angles[1];
        LocalClientGlobals->refdefViewAngles[2] = angles[2];
    }
    return 1;
}

void __cdecl CG_SetDebugOrigin(float *origin)
{
    if (cl_freemove->current.integer)
    {
        cgArray[0].predictedPlayerState.origin[0] = *origin;
        cgArray[0].predictedPlayerState.origin[1] = origin[1];
        cgArray[0].predictedPlayerState.origin[2] = origin[2];
    }
}

void __cdecl CG_SetDebugAngles(const float *angles)
{
    double v1; // fp12
    double v2; // fp0

    if (cl_freemove->current.integer)
    {
        v1 = (float)(angles[2] - cgArray[0].refdefViewAngles[2]);
        v2 = (float)(cgArray[0].predictedPlayerState.delta_angles[1] + (float)(angles[1] - cgArray[0].refdefViewAngles[1]));
        cgArray[0].predictedPlayerState.delta_angles[0] = cgArray[0].predictedPlayerState.delta_angles[0]
            + (float)(*angles - cgArray[0].refdefViewAngles[0]);
        cgArray[0].predictedPlayerState.delta_angles[1] = v2;
        cgArray[0].predictedPlayerState.delta_angles[2] = cgArray[0].predictedPlayerState.delta_angles[2] + (float)v1;
    }
}

void __cdecl CG_UpdateEntInfo(int localClientNum)
{
    snapshot_s *nextSnap; // r11
    unsigned int numEntities; // r7
    int v4; // r28
    int v5; // r30
    unsigned int v6; // r31
    DObj_s *ClientDObj; // r4

    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "update ent info");
    //Profile_Begin(12);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    nextSnap = cgArray[0].nextSnap;
    numEntities = cgArray[0].nextSnap->numEntities;
    if (numEntities > 0x800)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1188,
            0,
            "cgameGlob->nextSnap->numEntities not in [0, MAX_ENTITIES_IN_SNAPSHOT]\n\t%i not in [%i, %i]",
            numEntities,
            0,
            2048);
        nextSnap = cgArray[0].nextSnap;
    }
    v4 = 0;
    if (nextSnap->numEntities > 0)
    {
        v5 = 45796;
        do
        {
            v6 = *(int *)((char *)&nextSnap->snapFlags + v5);
            if (v6 >= 0x880)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
                    1193,
                    0,
                    "entnum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    *(int *)((char *)&nextSnap->snapFlags + v5),
                    2176);
            ClientDObj = Com_GetClientDObj(v6, localClientNum);
            if (ClientDObj)
                CG_DObjUpdateInfo(cgArray, ClientDObj, 0);
            nextSnap = cgArray[0].nextSnap;
            ++v4;
            v5 += 4;
        } while (v4 < cgArray[0].nextSnap->numEntities);
    }
    //Profile_EndInternal(0);
    //PIXEndNamedEvent();
}

const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp", 1241, 0, "%s", "localClientNum == 0");
    return clientViewParamsArray[0];
}

void __cdecl CG_ArchiveViewInfo(cg_s *cgameGlob, MemoryFile *memFile)
{
    if (!memFile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp", 1248, 0, "%s", "memFile");
    MemFile_ArchiveData(memFile, 4, &cgameGlob->vehicleInitView);
    MemFile_ArchiveData(memFile, 36, cgameGlob->prevVehicleInvAxis);
    MemFile_ArchiveData(memFile, 1, &cgameGlob->vehicleViewLocked);
    MemFile_ArchiveData(memFile, 12, cgameGlob->vehicleViewLockedAngles);
}

void __cdecl GetCeilingHeight(cg_s *cgameGlob)
{
    float *origin; // r31
    double v3; // fp0
    float v4[2]; // [sp+50h] [-60h] BYREF
    float v5; // [sp+58h] [-58h]
    trace_t v6; // [sp+60h] [-50h] BYREF

    origin = cgameGlob->predictedPlayerState.origin;
    v4[0] = cgameGlob->predictedPlayerState.origin[0];
    v5 = cgameGlob->predictedPlayerState.origin[2] + (float)1024.0;
    v4[1] = cgameGlob->predictedPlayerState.origin[1];
    CG_TraceCapsule(&v6, cgameGlob->predictedPlayerState.origin, playerMins, playerMaxs, v4, 2175, 1);
    if (v6.fraction < 1.0)
        v3 = (float)((float)((float)((float)(v5 - origin[2]) * v6.fraction) + origin[2])
            - cgameGlob->predictedPlayerState.origin[2]);
    else
        v3 = FLT_MAX;
    cgameGlob->heightToCeiling = v3;
}

void __cdecl DumpAnims(int localClientNum)
{
    const dvar_s *v2; // r11
    int integer; // r8
    int v4; // r3
    const DObj_s *ClientDObj; // r3

    v2 = cg_dumpAnims;
    integer = cg_dumpAnims->current.integer;
    if (integer < -1 || integer >= 2176)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1285,
            0,
            "%s\n\t(cg_dumpAnims->current.integer) = %i",
            "(cg_dumpAnims->current.integer >= -1 && cg_dumpAnims->current.integer < (2176))",
            integer);
        v2 = cg_dumpAnims;
    }
    v4 = v2->current.integer;
    if (v4 >= 0 && !cg_paused->current.integer)
    {
        if (v4)
        {
            ClientDObj = Com_GetClientDObj(v4, localClientNum);
            if (ClientDObj)
                DObjDisplayAnim(ClientDObj, "client:\n");
        }
        else
        {
            CG_DisplayViewmodelAnim(localClientNum);
        }
    }
}

void __cdecl DrawShellshockBlend(int localClientNum)
{
    ShockViewTypes type; // r10

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cg_drawShellshock->current.enabled)
    {
        type = cgArray[0].shellshock.parms->screenBlend.type;
        if (type)
        {
            if (type == SHELLSHOCK_VIEWTYPE_FLASHED)
            {
                CG_DrawShellShockSavedScreenBlendFlashed(
                    localClientNum,
                    cgArray[0].shellshock.parms,
                    cgArray[0].shellshock.startTime,
                    cgArray[0].shellshock.duration);
            }
            else if (type != SHELLSHOCK_VIEWTYPE_NONE)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
                    1321,
                    0,
                    "%s",
                    "cgameGlob->shellshock.parms->screenBlend.type == SHELLSHOCK_VIEWTYPE_NONE");
            }
        }
        else
        {
            CG_DrawShellShockSavedScreenBlendBlurred(
                localClientNum,
                cgArray[0].shellshock.parms,
                cgArray[0].shellshock.startTime,
                cgArray[0].shellshock.duration);
        }
    }
}

void __cdecl CG_UpdateViewOffset(int localClientNum)
{
    double v1; // fp0

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].refdef.viewOffset[0] = (float)((float)(cgArray[0].nextSnap->ps.origin[0] - cgArray[0].snap->ps.origin[0])
        * cgArray[0].frameInterpolation)
        + cgArray[0].snap->ps.origin[0];
    cgArray[0].refdef.viewOffset[1] = (float)((float)(cgArray[0].nextSnap->ps.origin[1] - cgArray[0].snap->ps.origin[1])
        * cgArray[0].frameInterpolation)
        + cgArray[0].snap->ps.origin[1];
    v1 = (float)((float)((float)(cgArray[0].nextSnap->ps.origin[2] - cgArray[0].snap->ps.origin[2])
        * cgArray[0].frameInterpolation)
        + cgArray[0].snap->ps.origin[2]);
    cgArray[0].refdef.viewOffset[2] = (float)((float)(cgArray[0].nextSnap->ps.origin[2] - cgArray[0].snap->ps.origin[2])
        * cgArray[0].frameInterpolation)
        + cgArray[0].snap->ps.origin[2];
    cgArray[0].refdef.viewOffset[2] = cgArray[0].nextSnap->ps.viewHeightCurrent + (float)v1;
    CL_ResetSkeletonCache();
}

void __cdecl UpdateTurretScopeZoom(cg_s *cgameGlob)
{
    unsigned int v2; // r3
    int CurrentCmdNumber; // r3
    __int128 v4; // r11
    double value; // fp31
    const dvar_s *v6; // r3
    double v7; // fp0
    double v8; // fp1
    unsigned int v9; // r3
    snd_alias_list_t *fireStopSoundPlayer; // r4
    usercmd_s v11; // [sp+60h] [-60h] BYREF

    if (CG_PlayerUsingScopedTurret(cgameGlob->localClientNum))
    {
        v2 = CG_PlayerTurretWeaponIdx(cgameGlob->localClientNum);
        if (BG_GetWeaponDef(v2)->overlayInterface == WEAPOVERLAYINTERFACE_TURRETSCOPE)
        {
            CurrentCmdNumber = CL_GetCurrentCmdNumber(cgameGlob->localClientNum);
            CL_GetUserCmd(cgameGlob->localClientNum, CurrentCmdNumber, &v11);
            DWORD1(v4) = (unsigned __int8)v11.forwardmove;
            DWORD2(v4) = cgameGlob->frametime;
            value = turretScopeZoom->current.value;
            LODWORD(v4) = v11.forwardmove;
            Dvar_SetFloat(
                turretScopeZoom,
                (float)((float)((float)((float)((float)*(__int64 *)((char *)&v4 + 4) * (float)0.001)
                    * (float)((float)(__int64)v4 * (float)-0.0078740157))
                    * turretScopeZoomRate->current.value)
                    + turretScopeZoom->current.value));
            v6 = turretScopeZoom;
            v7 = turretScopeZoom->current.value;
            v8 = turretScopeZoomMin->current.value;
            if (v7 < v8 || (v8 = turretScopeZoomMax->current.value, v7 > v8))
            {
                Dvar_SetFloat(turretScopeZoom, v8);
                v6 = turretScopeZoom;
            }
            if (fabs((float)(v6->current.value - (float)value)) > 0.0)
            {
                v9 = CG_PlayerTurretWeaponIdx(cgameGlob->localClientNum);
                fireStopSoundPlayer = BG_GetWeaponDef(v9)->fireStopSoundPlayer;
                if (fireStopSoundPlayer)
                    CG_PlayClientSoundAlias(cgameGlob->localClientNum, fireStopSoundPlayer);
            }
        }
    }
}

void __cdecl CG_UpdateSceneDepthOfField(cg_s *cgameGlob)
{
    snapshot_s *snap; // r11

    snap = cgameGlob->snap;
    cgameGlob->refdef.dof.nearStart = snap->ps.dofNearStart;
    cgameGlob->refdef.dof.nearEnd = snap->ps.dofNearEnd;
    cgameGlob->refdef.dof.farStart = snap->ps.dofFarStart;
    cgameGlob->refdef.dof.farEnd = snap->ps.dofFarEnd;
    cgameGlob->refdef.dof.nearBlur = snap->ps.dofNearBlur;
    cgameGlob->refdef.dof.farBlur = snap->ps.dofFarBlur;
}

void __cdecl CG_CalcViewValues(int localClientNum)
{
    double BlurRadius; // fp31
    double MenuBlurRadius; // fp1
    __int64 v4; // r10
    double v5; // fp0
    double v6; // fp8
    double v7; // fp9
    double v8; // fp10
    int pm_type; // r11
    __int64 v10; // r11
    double v11; // fp0

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].refdef.zNear = cgArray[0].zNear;
    cgArray[0].refdef.time = cgArray[0].time;
    cgArray[0].refdef.localClientNum = localClientNum;
    BlurRadius = CG_GetBlurRadius(localClientNum);
    if (cg_drawpaused->current.enabled)
        MenuBlurRadius = CL_GetMenuBlurRadius(localClientNum);
    else
        MenuBlurRadius = 0.0;
    cgArray[0].refdef.blurRadius = sqrtf((float)((float)((float)BlurRadius * (float)BlurRadius) + (float)((float)(cgDC.blurRadiusOut * cgDC.blurRadiusOut) + (float)((float)MenuBlurRadius * (float)MenuBlurRadius))));
    CG_VisionSetApplyToRefdef(localClientNum);
    if (cgArray[0].cubemapShot)
    {
        cgArray[0].refdef.vieworg[0] = cgArray[0].predictedPlayerState.origin[0];
        cgArray[0].refdef.vieworg[1] = cgArray[0].predictedPlayerState.origin[1];
        cgArray[0].refdef.vieworg[2] = cgArray[0].predictedPlayerState.viewHeightCurrent
            + cgArray[0].predictedPlayerState.origin[2];
        R_CalcCubeMapViewValues(&cgArray[0].refdef, cgArray[0].cubemapShot, cgArray[0].cubemapSize);
        return;
    }
    CG_CalcVrect(localClientNum);
    LODWORD(v4) = LOBYTE(cgArray[0].predictedPlayerState.bobCycle);
    cgArray[0].fBobCycle = (float)((float)v4 * (float)0.024639944) + (float)6.2831855;
    if ((cgArray[0].predictedPlayerState.pm_flags & 8) == 0)
    {
        v5 = sqrtf((float)((float)(cgArray[0].predictedPlayerState.velocity[0]
            * cgArray[0].predictedPlayerState.velocity[0])
            + (float)(cgArray[0].predictedPlayerState.velocity[1]
                * cgArray[0].predictedPlayerState.velocity[1])));
        goto LABEL_13;
    }
    if (cgArray[0].time - cgArray[0].predictedPlayerState.jumpTime >= 500)
    {
        v5 = cgArray[0].predictedPlayerState.velocity[2];
    LABEL_13:
        cgArray[0].xyspeed = v5;
        goto LABEL_14;
    }
    cgArray[0].xyspeed = 0.0;
LABEL_14:
    if (!(unsigned __int8)PausedClientFreeMove(localClientNum))
    {
        CG_CalcVehicleViewValues(localClientNum);
        CG_CalcLinkedViewValues(localClientNum);
        v6 = cgArray[0].predictedPlayerState.origin[0];
        cgArray[0].refdef.vieworg[0] = cgArray[0].predictedPlayerState.origin[0];
        v7 = cgArray[0].predictedPlayerState.origin[1];
        cgArray[0].refdef.vieworg[1] = cgArray[0].predictedPlayerState.origin[1];
        v8 = cgArray[0].predictedPlayerState.origin[2];
        cgArray[0].refdef.vieworg[2] = cgArray[0].predictedPlayerState.origin[2];
        if (!cgArray[0].playerTeleported)
        {
            pm_type = cgArray[0].nextSnap->ps.pm_type;
            if (!pm_type || pm_type == 2 || pm_type == 3)
            {
                CG_SmoothCameraZ(cgArray);
                v8 = cgArray[0].refdef.vieworg[2];
                v7 = cgArray[0].refdef.vieworg[1];
                v6 = cgArray[0].refdef.vieworg[0];
            }
        }
        cgArray[0].lastVieworg[0] = v6;
        cgArray[0].lastVieworg[1] = v7;
        cgArray[0].lastVieworg[2] = v8;
        cgArray[0].refdefViewAngles[0] = cgArray[0].predictedPlayerState.viewangles[0];
        cgArray[0].refdefViewAngles[1] = cgArray[0].predictedPlayerState.viewangles[1];
        cgArray[0].refdefViewAngles[2] = cgArray[0].predictedPlayerState.viewangles[2];
        if (cgArray[0].predictedPlayerState.pm_type != 1
            && cgArray[0].predictedPlayerState.pm_type != 6
            && cg_errorDecay->current.value > 0.0)
        {
            HIDWORD(v10) = cgArray[0].time;
            LODWORD(v10) = cgArray[0].time - cgArray[0].predictedErrorTime;
            v11 = (float)((float)(cg_errorDecay->current.value - (float)v10) / cg_errorDecay->current.value);
            if (v11 <= 0.0 || v11 >= 1.0)
            {
                cgArray[0].predictedErrorTime = 0;
            }
            else
            {
                cgArray[0].refdef.vieworg[0] = (float)(cgArray[0].predictedError[0]
                    * (float)((float)(cg_errorDecay->current.value - (float)v10)
                        / cg_errorDecay->current.value))
                    + (float)v6;
                cgArray[0].refdef.vieworg[1] = (float)(cgArray[0].predictedError[1] * (float)v11) + (float)v7;
                cgArray[0].refdef.vieworg[2] = (float)(cgArray[0].predictedError[2] * (float)v11) + (float)v8;
            }
        }
        CalcTurretViewValues(localClientNum);
        OffsetFirstPersonView(localClientNum, cgArray);
        CG_ShakeCamera(localClientNum);
    }
    AnglesToAxis(cgArray[0].refdefViewAngles, cgArray[0].refdef.viewaxis);
    CG_ApplyViewAnimation(localClientNum);
    CG_PerturbCamera(cgArray);
    CG_CalcFov(localClientNum);
    if (cgArray[0].predictedPlayerState.pm_type == 4)
        CG_ModelPreviewerUpdateView(
            cgArray[0].refdef.vieworg,
            cgArray[0].refdef.viewaxis,
            cgArray[0].refdefViewAngles,
            &cgArray[0].refdef.zNear);
}

void __cdecl CG_InitView(int localClientNum)
{
    double FarPlaneDist; // fp1

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_UpdateViewOffset(localClientNum);
    CG_PredictPlayerState(localClientNum);
    CG_UpdateViewWeaponAnim(localClientNum);
    CG_CalcViewValues(localClientNum);
    FX_SetNextUpdateTime(localClientNum, cgArray[0].time);
    FarPlaneDist = R_GetFarPlaneDist();
    FX_SetNextUpdateCamera(localClientNum, &cgArray[0].refdef, FarPlaneDist);
}

int __cdecl CG_DrawActiveFrame(
    int localClientNum,
    int serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int cubemapSize,
    int animFrametime)
{
    int v12; // r4
    int v13; // r7
    int time; // r5
    int frametime; // r6
    bool v16; // zf
    const char *v17; // r3
    const char *v18; // r3
    int shellshockIndex; // r3
    int shellshockDuration; // r29
    int shellshockTime; // r28
    const shellshock_parms_t *ShellshockParms; // r3
    unsigned int viewlocked_entNum; // r28
    __int64 v25; // r10
    int v26; // r8
    int v27; // r7
    int v28; // r6
    int v29; // r5
    int v30; // r4
    double FarPlaneDist; // fp1
    FxCmd v32[9]; // [sp+50h] [-70h] BYREF

    //Profile_Begin(13);
    R_ClearScene(localClientNum);
    FX_BeginUpdate(localClientNum);
    CG_SetCollWorldLocalClientNum(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v12 = serverTime;
    v13 = animFrametime;
    time = cgArray[0].time;
    frametime = serverTime - cgArray[0].time;
    cgArray[0].oldTime = cgArray[0].time;
    cgArray[0].frametime = serverTime - cgArray[0].time;
    cgArray[0].demoType = demoType;
    cgArray[0].cubemapShot = cubemapShot;
    cgArray[0].cubemapSize = cubemapSize;
    cgArray[0].animFrametime = animFrametime;
    v16 = serverTime - cgArray[0].time >= 0;
    cgArray[0].time = serverTime;
    if (!v16)
    {
        v17 = va("cgameGlob->time: %d, cgameGlob->oldTime: %d, cgameGlob->frametime: %d", serverTime, time, frametime);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1419,
            0,
            "%s\n\t%s",
            "cgameGlob->frametime >= 0",
            v17);
        time = cgArray[0].oldTime;
        v12 = cgArray[0].time;
        v13 = cgArray[0].animFrametime;
        frametime = cgArray[0].frametime;
    }
    if (frametime - v13 < 0)
    {
        v18 = va(
            "cgameGlob->time: %d, cgameGlob->oldTime: %d, cgameGlob->frametime: %d, cgameGlob->animFrametime: %d",
            v12,
            time,
            frametime,
            v13);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1420,
            0,
            "%s\n\t%s",
            "cgameGlob->frametime - cgameGlob->animFrametime >= 0",
            v18);
    }
    CG_ProcessSnapshots(localClientNum);
    if (!cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp", 1425, 0, "%s", "cgameGlob->snap");
    if (!cgArray[0].nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp", 1426, 0, "%s", "cgameGlob->nextSnap");
    if (cgArray[0].nextSnap->serverTime != G_GetServerSnapTime())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1427,
            0,
            "%s",
            "cgameGlob->nextSnap->serverTime == G_GetServerSnapTime()");
    if (CL_SkipRendering())
    {
        CG_PredictPlayerState(localClientNum);
        //Profile_EndInternal(0);
        return 0;
    }
    CG_VisionSetsUpdate(localClientNum);
    CG_UpdateViewOffset(localClientNum);
    if (!CG_ModelPreviewerNeedsVieworgInterpSkipped(localClientNum))
    {
        cgArray[0].refdef.vieworg[0] = cgArray[0].refdef.viewOffset[0];
        cgArray[0].refdef.vieworg[1] = cgArray[0].refdef.viewOffset[1];
        cgArray[0].refdef.vieworg[2] = cgArray[0].refdef.viewOffset[2];
    }
    cgArray[0].refdef.time = cgArray[0].time;
    R_SetLodOrigin(&cgArray[0].refdef);
    FX_SetNextUpdateTime(localClientNum, cgArray[0].time);
    FX_FillUpdateCmd(localClientNum, v32);
    //Profile_Begin(22);
    R_UpdateNonDependentEffects(v32);
    //Profile_EndInternal(0);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    shellshockIndex = cgArray[0].snap->ps.shellshockIndex;
    if (shellshockIndex)
    {
        shellshockDuration = cgArray[0].snap->ps.shellshockDuration;
        shellshockTime = cgArray[0].snap->ps.shellshockTime;
    }
    else
    {
        shellshockDuration = cgArray[0].testShock.duration;
        shellshockTime = cgArray[0].testShock.time;
    }
    ShellshockParms = BG_GetShellshockParms(shellshockIndex);
    CG_StartShellShock(cgArray, ShellshockParms, shellshockTime, shellshockDuration);
    UpdateTurretScopeZoom(cgArray);
    CG_UpdateShellShock(
        localClientNum,
        cgArray[0].shellshock.parms,
        cgArray[0].shellshock.startTime,
        cgArray[0].shellshock.duration);
    CG_ClearHudGrenades();
    CG_UpdateEntInfo(localClientNum);
    if (CG_AddPacketEntities(localClientNum))
        viewlocked_entNum = cgArray[0].predictedPlayerState.viewlocked_entNum;
    else
        viewlocked_entNum = 2175;
    CG_UpdateRumble(localClientNum);
    if (!cgArray[0].predictedPlayerState.locationSelectionInfo)
    {
        Key_RemoveCatcher(localClientNum, -9);
        goto LABEL_29;
    }
    if (!Key_IsCatcherActive(localClientNum, 8))
    {
        Key_AddCatcher(localClientNum, 8);
    LABEL_29:
        cgArray[0].selectedLocation[0] = 0.5;
        cgArray[0].selectedLocation[1] = 0.5;
    }
    CL_Input(localClientNum);
    CG_ModelPreviewerFrame(cgArray);
    CG_AddModelPreviewerModel(cgArray[0].frametime, v30, v29, v28, v27, v26, v25);
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "player state");
    CG_PredictPlayerState(localClientNum);
    //PIXEndNamedEvent();
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "view anim");
    CG_UpdateViewWeaponAnim(localClientNum);
    //PIXEndNamedEvent();
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "view values");
    CG_CalcViewValues(localClientNum);
    //PIXEndNamedEvent();
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "player entity");
    if (cl_freemove->current.integer)
        R_SetLodOrigin(&cgArray[0].refdef);
    FarPlaneDist = R_GetFarPlaneDist();
    FX_SetNextUpdateCamera(localClientNum, &cgArray[0].refdef, FarPlaneDist);
    R_UpdateSpotLightEffect(v32);
    SND_SetListener(
        localClientNum,
        cgArray[0].nextSnap->ps.clientNum,
        cgArray[0].refdef.vieworg,
        cgArray[0].refdef.viewaxis);
    CG_SetRumbleReceiver(localClientNum, cgArray[0].nextSnap->ps.clientNum, cgArray[0].refdef.vieworg);
    CG_AddViewWeapon(localClientNum);
    CG_UpdateTestFX(localClientNum);
    if (cgArray[0].nextSnap->serverTime != G_GetServerSnapTime())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1566,
            0,
            "%s",
            "cgameGlob->nextSnap->serverTime == G_GetServerSnapTime()");
    CG_ProcessEntity(localClientNum, &cgArray[0].predictedPlayerEntity);
    if (viewlocked_entNum != 2175)
        CG_AddPacketEntity(localClientNum, viewlocked_entNum);
    GetCeilingHeight(cgArray);
    DumpAnims(localClientNum);
    //PIXEndNamedEvent();
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "remaining fx");
    R_UpdateRemainingEffects(v32);
    //PIXEndNamedEvent();
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "aim assist");
    AimAssist_UpdateScreenTargets(
        localClientNum,
        cgArray[0].refdef.vieworg,
        cgArray[0].refdefViewAngles,
        cgArray[0].refdef.tanHalfFovX,
        cgArray[0].refdef.tanHalfFovY);
    //PIXEndNamedEvent();
    cgArray[0].refdef.dof.nearStart = cgArray[0].snap->ps.dofNearStart;
    cgArray[0].refdef.dof.nearEnd = cgArray[0].snap->ps.dofNearEnd;
    cgArray[0].refdef.dof.farStart = cgArray[0].snap->ps.dofFarStart;
    cgArray[0].refdef.dof.farEnd = cgArray[0].snap->ps.dofFarEnd;
    cgArray[0].refdef.dof.nearBlur = cgArray[0].snap->ps.dofNearBlur;
    cgArray[0].refdef.dof.farBlur = cgArray[0].snap->ps.dofFarBlur;
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "draw 2D");
    R_AddCmdProjectionSet2D();
    DrawShellshockBlend(localClientNum);
    //Profile_Begin(350);
    CG_Draw2D(localClientNum);
    //Profile_EndInternal(0);
    //PIXEndNamedEvent();
    if (cgArray[0].nextSnap->serverTime != G_GetServerSnapTime())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_view.cpp",
            1612,
            0,
            "%s",
            "cgameGlob->nextSnap->serverTime == G_GetServerSnapTime()");
    Sys_AllowSendClientMessages();
    R_Cinematic_SetPaused((CinematicEnum)(cg_paused->current.integer != 0));
    //Profile_Begin(25);
    CG_DrawActive(localClientNum);
    //Profile_EndInternal(0);
    //Profile_EndInternal(0);
    return 1;
}

