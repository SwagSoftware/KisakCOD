#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_predict.h"
#include <client/client.h>
#include "cg_main.h"
#include "cg_ents.h"

pmove_t cg_pmove;

void __cdecl TRACK_cg_predict()
{
    track_static_alloc_internal(&cg_pmove, 292, "cg_pmove", 9);
}

void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles, int grabStance)
{
    snapshot_s *nextSnap; // r27
    snapshot_s *snap; // r31
    int CurrentCmdNumber; // r3
    usercmd_s *v9; // r4
    double frameInterpolation; // fp31
    int bobCycle; // r10
    int v12; // r11
    __int64 v13; // r10
    float *linkAngles; // r28
    float *origin; // r30
    int v16; // r24
    char *v17; // r23
    char *v18; // r25
    int v19; // r26
    float *viewAngleClampBase; // r30
    float *viewAngleClampRange; // r28
    int v22; // r29
    char *v23; // r31
    unsigned int v24[4]; // [sp+50h] [-C0h] BYREF
    usercmd_s v25; // [sp+60h] [-B0h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    nextSnap = cgArray[0].nextSnap;
    snap = cgArray[0].snap;
    if (!cgArray[0].nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_predict.cpp", 58, 0, "%s", "nextSnap");
    memcpy(&cgArray[0].predictedPlayerState, &nextSnap->ps, sizeof(cgArray[0].predictedPlayerState));
    if (grabAngles)
    {
        CurrentCmdNumber = CL_GetCurrentCmdNumber(localClientNum);
        CL_GetUserCmd(localClientNum, CurrentCmdNumber, &v25);
        PM_UpdateViewAngles(&cgArray[0].predictedPlayerState, 0.0, v9, (unsigned __int8)&v25);
    }
    if (nextSnap->serverTime > snap->serverTime)
    {
        frameInterpolation = cgArray[0].frameInterpolation;
        if (grabStance)
        {
            bobCycle = nextSnap->ps.bobCycle;
            if (bobCycle < snap->ps.bobCycle)
                bobCycle += 256;
            v12 = snap->ps.bobCycle;
            HIDWORD(v13) = v24;
            LODWORD(v13) = bobCycle - v12;
            v24[1] = v13;
            v24[0] = (int)(float)((float)v13 * cgArray[0].frameInterpolation);
            cgArray[0].predictedPlayerState.bobCycle = v24[0] + v12;
            cgArray[0].predictedPlayerState.leanf = (float)((float)(nextSnap->ps.leanf - snap->ps.leanf)
                * cgArray[0].frameInterpolation)
                + snap->ps.leanf;
            cgArray[0].predictedPlayerState.aimSpreadScale = (float)((float)(nextSnap->ps.aimSpreadScale
                - snap->ps.aimSpreadScale)
                * cgArray[0].frameInterpolation)
                + snap->ps.aimSpreadScale;
            cgArray[0].predictedPlayerState.fWeaponPosFrac = (float)((float)(nextSnap->ps.fWeaponPosFrac
                - snap->ps.fWeaponPosFrac)
                * cgArray[0].frameInterpolation)
                + snap->ps.fWeaponPosFrac;
            cgArray[0].predictedPlayerState.viewHeightCurrent = (float)((float)(nextSnap->ps.viewHeightCurrent
                - snap->ps.viewHeightCurrent)
                * cgArray[0].frameInterpolation)
                + snap->ps.viewHeightCurrent;
        }
        linkAngles = nextSnap->ps.linkAngles;
        origin = snap->ps.origin;
        v16 = (char *)nextSnap - (char *)snap;
        v17 = (char *)((char *)&cgArray[0].oldTime - (char *)snap);
        v18 = (char *)((char *)&cgArray[0].predictedPlayerState.weaponrechamber[1] - (char *)snap);
        v19 = 3;
        do
        {
            *(float *)&v17[(unsigned int)origin] = (float)((float)(*(float *)((char *)origin + v16) - *origin)
                * (float)frameInterpolation)
                + *origin;
            if (!grabAngles)
                *(float *)((char *)origin + (char *)&cgArray[0].predictedPlayerState.offhandSecondary - (char *)snap) = LerpAngle(origin[50], *(linkAngles - 294), frameInterpolation);
            *(float *)((char *)origin + (char *)&cgArray[0].predictedPlayerState.pm_type - (char *)snap) = (float)((float)(*(linkAngles - 341) - origin[3]) * (float)frameInterpolation) + origin[3];
            if (snap->ps.pm_type == 1)
                *(float *)&v18[(unsigned int)origin] = LerpAngle(origin[344], *linkAngles, frameInterpolation);
            else
                *(float *)&v18[(unsigned int)origin] = *linkAngles;
            if (nextSnap->ps.pm_type == 1 && (nextSnap->ps.eFlags & 0x20000) == 0)
                *(float *)((char *)origin + (char *)&cgArray[0].predictedPlayerState.weaponDelay - (char *)snap) = LerpAngle(origin[18], *(linkAngles - 326), frameInterpolation);
            --v19;
            ++origin;
            ++linkAngles;
        } while (v19);
        if (snap->ps.pm_type == 1 || (snap->ps.eFlags & 0x300) != 0)
        {
            viewAngleClampBase = snap->ps.viewAngleClampBase;
            viewAngleClampRange = nextSnap->ps.viewAngleClampRange;
            v22 = 2;
            v23 = (char *)((char *)&cgArray[0].predictedPlayerState - (char *)snap);
            do
            {
                *(float *)&v17[(unsigned int)viewAngleClampBase] = LerpAngle(
                    *viewAngleClampBase,
                    *(float *)((char *)viewAngleClampBase + v16),
                    frameInterpolation);
                --v22;
                *(float *)((char *)viewAngleClampBase + (unsigned int)v23) = (float)((float)(*viewAngleClampRange++
                    - viewAngleClampBase[2])
                    * (float)frameInterpolation)
                    + viewAngleClampBase[2];
                ++viewAngleClampBase;
            } while (v22);
        }
    }
}

void __cdecl CG_RestorePlayerOrientation(cg_s *cgameGlob)
{
    double v2; // fp0
    double v3; // fp13
    double v4; // fp12
    float v5[4]; // [sp+50h] [-30h] BYREF
    float v6[4]; // [sp+60h] [-20h] BYREF

    CL_GetViewForward(v6);
    vectoangles(v6, v5);
    v2 = v5[0];
    v3 = v5[1];
    v4 = v5[2];
    cgameGlob->predictedPlayerState.viewangles[0] = v5[0];
    cgameGlob->predictedPlayerState.viewangles[1] = v3;
    cgameGlob->predictedPlayerState.viewangles[2] = v4;
    cgameGlob->predictedPlayerState.delta_angles[0] = v2;
    cgameGlob->predictedPlayerState.delta_angles[1] = v3;
    cgameGlob->predictedPlayerState.delta_angles[2] = v4;
    CL_GetViewPos(cgameGlob->predictedPlayerState.origin);
    cgameGlob->predictedPlayerState.origin[2] = cgameGlob->predictedPlayerState.origin[2]
        - cgameGlob->predictedPlayerState.viewHeightCurrent;
}

void __cdecl CG_UpdateFreeMove(cg_s *cgameGlob)
{
    const dvar_s *v2; // r11
    int v3; // r10
    snapshot_s *snap; // r11
    snapshot_s *nextSnap; // r10
    double frameInterpolation; // fp3
    const float *v7; // r6
    double v8; // fp1
    float v9[6]; // [sp+50h] [-30h] BYREF

    v2 = cl_freemove;
    if (!cl_freemove->current.integer)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_predict.cpp",
            146,
            0,
            "%s",
            "cl_freemove->current.integer != FREEMOVE_NONE");
        v2 = cl_freemove;
    }
    v3 = 3;
    if (v2->current.integer == 1)
        v3 = 2;
    cgameGlob->predictedPlayerState.pm_type = v3;
    cgameGlob->predictedPlayerState.eFlags = 0;
    cgameGlob->predictedPlayerState.pm_flags = 0;
    cgameGlob->predictedPlayerState.aimSpreadScale = 0.0;
    cgameGlob->predictedPlayerState.weapFlags = 0;
    cgameGlob->predictedPlayerState.otherFlags = 0;
    if (cg_paused->current.integer == 1)
        Dvar_SetInt(cg_paused, 2);
    snap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;
    frameInterpolation = cgameGlob->frameInterpolation;
    v9[0] = (float)((float)(nextSnap->ps.origin[0] - snap->ps.origin[0]) * cgameGlob->frameInterpolation)
        + snap->ps.origin[0];
    v9[1] = (float)((float)(nextSnap->ps.origin[1] - snap->ps.origin[1]) * (float)frameInterpolation) + snap->ps.origin[1];
    v9[2] = (float)((float)(nextSnap->ps.origin[2] - snap->ps.origin[2]) * (float)frameInterpolation) + snap->ps.origin[2];
    v8 = LerpAngle(snap->ps.viewangles[1], nextSnap->ps.viewangles[1], frameInterpolation);
    if (cg_drawPlayerPosInFreeMove->current.enabled)
        CG_DebugBox(v9, cg_pmove.mins, cg_pmove.maxs, v8, v7, (int)colorRed, 1);
}

void __cdecl CG_InterpolateGroundTilt(int localClientNum)
{
    snapshot_s *nextSnap; // r28
    snapshot_s *snap; // r29
    float *groundTiltAngles; // r31
    int v4; // r28
    double frameInterpolation; // fp31
    int v6; // r30
    char *v7; // r29

    if (cg_paused->current.integer != 2)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        nextSnap = cgArray[0].nextSnap;
        snap = cgArray[0].snap;
        if (!cgArray[0].nextSnap)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_predict.cpp", 191, 0, "%s", "nextSnap");
        if (nextSnap->serverTime > snap->serverTime)
        {
            groundTiltAngles = snap->ps.groundTiltAngles;
            v4 = (char *)nextSnap - (char *)snap;
            frameInterpolation = cgArray[0].frameInterpolation;
            v6 = 3;
            v7 = (char *)((char *)&cgArray[0].oldTime - (char *)snap);
            do
            {
                --v6;
                *(float *)((char *)groundTiltAngles + (unsigned int)v7) = LerpAngle(
                    *groundTiltAngles,
                    *(float *)((char *)groundTiltAngles + v4),
                    frameInterpolation);
                ++groundTiltAngles;
            } while (v6);
        }
    }
}

void __cdecl CG_PredictPlayerState_Internal(int localClientNum)
{
    playerState_s *Buf; // r20
    int v3; // r11
    snapshot_s *nextSnap; // r11
    int CurrentCmdNumber; // r27
    int v6; // r28
    int i; // r30
    const dvar_s *v8; // r11
    double v9; // fp30
    double v10; // fp29
    double v11; // fp28
    double v12; // fp1
    __int64 v13; // r10
    double v14; // fp31
    double v15; // r5
    double v16; // fp0
    double v17; // fp13
    double v18; // fp12
    const dvar_s *v19; // r11
    __int64 v20; // r8
    _BYTE v21[12]; // r11 OVERLAPPED
    double v22; // fp0
    double v23; // fp13
    double v24; // fp0
    __int64 v27; // r11
    double v28; // fp0
    __int64 v29; // fp13
    __int64 v30; // r11
    LargeLocal v31; // [sp+58h] [-118h] BYREF
    __int64 v32; // [sp+60h] [-110h]
    float v33; // [sp+68h] [-108h] BYREF
    float v34; // [sp+6Ch] [-104h]
    float v35; // [sp+70h] [-100h]
    float v36[4]; // [sp+78h] [-F8h] BYREF
    float v37[6]; // [sp+88h] [-E8h] BYREF
    usercmd_s v38; // [sp+A0h] [-D0h] BYREF

    LargeLocal::LargeLocal(&v31, 45784);
    Buf = (playerState_s *)LargeLocal::GetBuf(&v31);
    //Profile_Begin(326);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].validPPS)
    {
        cgArray[0].validPPS = 1;
        memcpy(&cgArray[0].predictedPlayerState, &cgArray[0].nextSnap->ps, sizeof(cgArray[0].predictedPlayerState));
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                917,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (cl_freemove->current.integer && cgsArray[0].started)
            CG_RestorePlayerOrientation(cgArray);
        cgsArray[0].started = 1;
    }
    if (cgArray[0].demoType && cg_paused->current.integer != 2 && !cl_freemove->current.integer)
    {
        CG_InterpolatePlayerState(localClientNum, 0, 1);
        v36[0] = cgArray[0].predictedPlayerState.viewangles[0] - cgArray[0].predictedPlayerState.delta_angles[0];
        v36[1] = cgArray[0].predictedPlayerState.viewangles[1] - cgArray[0].predictedPlayerState.delta_angles[1];
        v36[2] = cgArray[0].predictedPlayerState.viewangles[2] - cgArray[0].predictedPlayerState.delta_angles[2];
        CL_SetViewAngles(localClientNum, v36);
        goto LABEL_81;
    }
    if (cg_nopredict->current.enabled)
    {
        CG_InterpolatePlayerState(localClientNum, 1, 1);
        goto LABEL_81;
    }
    cg_pmove.handler = 0;
    cg_pmove.ps = &cgArray[0].predictedPlayerState;
    if (cgArray[0].predictedPlayerState.pm_type < 5)
        v3 = 42057745;
    else
        v3 = 8454161;
    cg_pmove.tracemask = v3;
    cg_pmove.viewChange = 0.0;
    cg_pmove.viewChangeTime = cgArray[0].stepViewStart;
    memcpy(Buf, &cgArray[0].predictedPlayerState, sizeof(playerState_s));
    if (cl_freemove->current.integer)
    {
        CG_UpdateFreeMove(cgArray);
    }
    else if (cg_paused->current.integer != 2)
    {
        nextSnap = cgArray[0].nextSnap;
        if (!cgArray[0].nextSnap)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_predict.cpp", 309, 0, "%s", "cgameGlob->nextSnap");
            nextSnap = cgArray[0].nextSnap;
        }
        memcpy(&cgArray[0].predictedPlayerState, &nextSnap->ps, sizeof(cgArray[0].predictedPlayerState));
    }
    cgArray[0].physicsTime = cgArray[0].nextSnap->serverTime;
    CurrentCmdNumber = CL_GetCurrentCmdNumber(localClientNum);
    if (CL_GetUserCmd(localClientNum, CurrentCmdNumber, &v38))
    {
        if ((cgArray[0].predictedPlayerState.pm_type == 1 || cgArray[0].predictedPlayerState.pm_type == 6)
            && cg_paused->current.integer != 2)
        {
            CG_InterpolatePlayerState(localClientNum, 0, 0);
        }
        CG_InterpolateGroundTilt(localClientNum);
        CG_AdjustPositionForMover(
            localClientNum,
            cgArray[0].predictedPlayerState.origin,
            cgArray[0].predictedPlayerState.groundEntityNum,
            cgArray[0].physicsTime,
            cgArray[0].time,
            cgArray[0].predictedPlayerState.origin,
            v37);
        v6 = CurrentCmdNumber - 63;
        for (i = 0; v6 <= CurrentCmdNumber; ++v6)
        {
            if (CL_GetUserCmd(localClientNum, v6, &cg_pmove.cmd)
                && cg_pmove.cmd.serverTime > cgArray[0].predictedPlayerState.commandTime
                && cg_pmove.cmd.serverTime <= v38.serverTime
                && CL_GetUserCmd(localClientNum, v6 - 1, &cg_pmove.oldcmd))
            {
                if (cgArray[0].predictedPlayerState.commandTime == Buf->commandTime)
                {
                    CG_AdjustPositionForMover(
                        localClientNum,
                        cgArray[0].predictedPlayerState.origin,
                        cgArray[0].predictedPlayerState.groundEntityNum,
                        cgArray[0].time,
                        cgArray[0].oldTime,
                        &v33,
                        v37);
                    cgArray[0].predictedPlayerState.delta_angles[1] = cgArray[0].predictedPlayerState.delta_angles[1] + v37[1];
                    v8 = cg_showmiss;
                    if (cg_showmiss->current.integer && (Buf->origin[0] != v33 || Buf->origin[1] != v34 || Buf->origin[2] != v35))
                    {
                        Com_PrintError(17, "prediction error\n");
                        v8 = cg_showmiss;
                    }
                    v9 = (float)(Buf->origin[1] - v34);
                    v10 = (float)(Buf->origin[2] - v35);
                    v11 = (float)(Buf->origin[0] - v33);
                    v12 = sqrtf((float)((float)((float)(Buf->origin[0] - v33) * (float)(Buf->origin[0] - v33))
                        + (float)((float)((float)(Buf->origin[2] - v35) * (float)(Buf->origin[2] - v35))
                            + (float)((float)(Buf->origin[1] - v34) * (float)(Buf->origin[1] - v34)))));
                    if (v12 > 0.1)
                    {
                        if (v8->current.integer)
                        {
                            Com_Printf(17, (const char *)HIDWORD(v12), LODWORD(v12));
                            v8 = cg_showmiss;
                        }
                        if (cg_errorDecay->current.value == 0.0)
                        {
                            v16 = 0.0;
                            v17 = 0.0;
                            v18 = 0.0;
                        }
                        else
                        {
                            HIDWORD(v13) = cgArray[0].time;
                            LODWORD(v13) = cgArray[0].time - cgArray[0].predictedErrorTime;
                            v14 = (float)((float)(cg_errorDecay->current.value - (float)v13) / cg_errorDecay->current.value);
                            if (v14 >= 0.0)
                            {
                                if (v14 > 0.0 && v8->current.integer)
                                {
                                    v15 = (float)((float)(cg_errorDecay->current.value - (float)v13) / cg_errorDecay->current.value);
                                    Com_Printf(17, (const char *)HIDWORD(v15), LODWORD(v15));
                                }
                            }
                            else
                            {
                                v14 = 0.0;
                            }
                            v16 = (float)(cgArray[0].predictedError[0] * (float)v14);
                            v17 = (float)(cgArray[0].predictedError[1] * (float)v14);
                            v18 = (float)(cgArray[0].predictedError[2] * (float)v14);
                        }
                        cgArray[0].predictedError[0] = (float)v16 + (float)v11;
                        cgArray[0].predictedErrorTime = cgArray[0].oldTime;
                        cgArray[0].predictedError[1] = (float)v17 + (float)v9;
                        cgArray[0].predictedError[2] = (float)v18 + (float)v10;
                    }
                }
                //Profile_Begin(26);
                Pmove(&cg_pmove);
                //Profile_EndInternal(0);
                i = 1;
            }
        }
        v19 = cg_showmiss;
        if (cg_showmiss->current.integer > 1)
        {
            Com_Printf(17, "[%i : %i] ", cg_pmove.cmd.serverTime, cgArray[0].time);
            v19 = cg_showmiss;
        }
        if (!i && v19->current.integer)
            Com_Printf(17, "no prediction run\n");
        CG_TransitionPlayerState(localClientNum, &cgArray[0].predictedPlayerState, Buf);
        if ((cgArray[0].predictedPlayerState.pm_flags & 0x400) != 0)
            CL_SetStance(localClientNum, CL_STANCE_STAND);
        if (cg_pmove.viewChange == 0.0
            || (*(unsigned int *)v21 = cg_pmove.viewChangeTime, cg_pmove.viewChangeTime == cgArray[0].stepViewStart)
            || cgArray[0].playerTeleported
            || cgArray[0].predictedPlayerState.pm_type
            && cgArray[0].predictedPlayerState.pm_type != 2
            && cgArray[0].predictedPlayerState.pm_type != 3)
        {
            LODWORD(v30) = cgArray[0].time - cgArray[0].stepViewStart;
            HIDWORD(v30) = cgArray[0].time - cgArray[0].stepViewStart;
            v28 = (float)(cg_viewZSmoothingTime->current.value * (float)1000.0);
            v32 = v30;
            v29 = v30;
        }
        else
        {
            if (I_fabs(cg_pmove.viewChange) >= cg_viewZSmoothingMin->current.value)
            {
                LODWORD(v20) = cgArray[0].time - cgArray[0].stepViewStart;
                v22 = 0.0;
                v23 = (float)(cg_viewZSmoothingTime->current.value * (float)1000.0);
                if ((float)v20 < v23)
                {
                    *(unsigned int *)&v21[8] = cg_pmove.viewChangeTime - cgArray[0].stepViewStart;
                    if (cg_pmove.viewChangeTime - cgArray[0].stepViewStart >= 0)
                    {
                        *(unsigned int *)&v21[4] = (int)v23;
                        if (*(int *)&v21[8] < (int)v23)
                        {
                            v32 = *(_QWORD *)v21;
                            v22 = (float)((float)((float)1.0 - (float)((float)*(__int64 *)&v21[4] / (float)*(__int64 *)v21))
                                * cgArray[0].stepViewChange);
                        }
                    }
                }
                v24 = (float)(cg_pmove.viewChange + (float)v22);
                if (v24 <= 0.0)
                    _FP12 = (float)((float)-cg_viewZSmoothingMax->current.value - (float)v24);
                else
                    _FP12 = (float)((float)v24 - cg_viewZSmoothingMax->current.value);
                __asm { fsel      f0, f12, f13, f0 }
                cgArray[0].stepViewChange = _FP0;
                cgArray[0].stepViewStart = cg_pmove.viewChangeTime;
                goto LABEL_81;
            }
            HIDWORD(v27) = cgArray[0].time - cgArray[0].stepViewStart;
            LODWORD(v27) = cgArray[0].time - cgArray[0].stepViewStart;
            v28 = (float)(cg_viewZSmoothingTime->current.value * (float)1000.0);
            v32 = v27;
            v29 = v27;
        }
        if ((float)v29 > v28)
            cgArray[0].stepViewChange = 0.0;
    }
LABEL_81:
    //Profile_EndInternal(0);
    //LargeLocal::~LargeLocal(&v31);
}

void __cdecl CG_PredictPlayerState(int localClientNum)
{
    centity_s *Entity; // r3

    CG_PredictPlayerState_Internal(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Entity = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.clientNum);
    Entity->pose.origin[0] = cgArray[0].predictedPlayerState.origin[0];
    Entity->pose.origin[1] = cgArray[0].predictedPlayerState.origin[1];
    Entity->pose.origin[2] = cgArray[0].predictedPlayerState.origin[2];
    BG_EvaluateTrajectory(&Entity->currentState.apos, cgArray[0].time, Entity->pose.angles);
    cgArray[0].predictedPlayerEntity.nextState.number = cgArray[0].predictedPlayerState.clientNum;
    BG_PlayerStateToEntityState(&cgArray[0].predictedPlayerState, &cgArray[0].predictedPlayerEntity.nextState, 0, 0);
    memcpy(
        &cgArray[0].predictedPlayerEntity.currentState,
        &cgArray[0].predictedPlayerEntity.nextState.lerp,
        sizeof(cgArray[0].predictedPlayerEntity.currentState));
    cgArray[0].predictedPlayerEntity.oldEType = cgArray[0].predictedPlayerEntity.nextState.eType;
    CG_CalcEntityLerpPositions(localClientNum, &cgArray[0].predictedPlayerEntity);
}

