#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_predict.h"
#include <client/client.h>
#include "cg_main.h"
#include "cg_ents.h"

#include <bgame/bg_public.h>
#include <client/cl_input.h>

pmove_t cg_pmove;

void __cdecl TRACK_cg_predict()
{
    track_static_alloc_internal(&cg_pmove, 292, "cg_pmove", 9);
}

char __cdecl CG_ShouldInterpolatePlayerStateViewClamp(int localClientNum, const snapshot_s *prevSnap)
{
    //const vehicle_info_t *vehInfo; // [esp+8h] [ebp-8h]
    centity_s *cent; // [esp+Ch] [ebp-4h]

    if (prevSnap->ps.pm_type == 1)
        return 1;
    if ((prevSnap->ps.eFlags & 0x300) != 0)
        return 1;
    //if ((prevSnap->ps.eFlags & 0x4000) != 0)
    //{
    //    cent = CG_GetEntity(localClientNum, prevSnap->ps.viewlocked_entNum);
    //    if (cent)
    //    {
    //        if (((*((_DWORD *)cent + 201) >> 1) & 1) != 0)
    //        {
    //            vehInfo = CG_GetVehicleInfo(cent->nextState.un2.vehicleState.vehicleInfoIndex);
    //            if (!vehInfo
    //                && !Assert_MyHandler(
    //                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp",
    //                    171,
    //                    0,
    //                    "%s",
    //                    "vehInfo"))
    //            {
    //                __debugbreak();
    //            }
    //            if (vehInfo->turretClampPlayerView)
    //                return 1;
    //        }
    //    }
    //}
    return 0;
}

// taken from blops, modified
static void __cdecl CG_InterpolatePlayerStateViewAngles(int localClientNum, playerState_s *out, usercmd_s *cmd)
{
    const char *v3; // eax
    float v4; // [esp+10h] [ebp-F0h]
    float v5; // [esp+20h] [ebp-E0h]
    float frameInterpolation; // [esp+28h] [ebp-D8h]
    float resultMat[3][3]; // [esp+30h] [ebp-D0h] BYREF
    float centerAngles[3]; // [esp+54h] [ebp-ACh] BYREF
    float viewYawAngles[3]; // [esp+60h] [ebp-A0h] BYREF
    float viewMat[3][3]; // [esp+6Ch] [ebp-94h] BYREF
    float vehMat[3][3]; // [esp+90h] [ebp-70h] BYREF
    float v12; // [esp+B4h] [ebp-4Ch]
    float fc; // [esp+B8h] [ebp-48h]
    int clampCount; // [esp+BCh] [ebp-44h]
    float yawOffset; // [esp+C0h] [ebp-40h]
    float pitchClamp; // [esp+C4h] [ebp-3Ch]
    float (*gunnerClamps)[2]; // [esp+C8h] [ebp-38h]
    centity_s *vehCent; // [esp+CCh] [ebp-34h]
    float vehAngles[3]; // [esp+D0h] [ebp-30h] BYREF
    float yawFrac; // [esp+DCh] [ebp-24h]
    int vehicleType; // [esp+E0h] [ebp-20h]
    unsigned __int16 vehType; // [esp+E4h] [ebp-1Ch]
    const char *vehicleTypeStr[2]; // [esp+E8h] [ebp-18h]
    cg_s *cgameGlob; // [esp+F0h] [ebp-10h]
    const snapshot_s *prevSnap; // [esp+F4h] [ebp-Ch]
    const snapshot_s *nextSnap; // [esp+F8h] [ebp-8h]
    int i; // [esp+FCh] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    prevSnap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;

    iassert(prevSnap);
    iassert(nextSnap);

    if (CG_ShouldInterpolatePlayerStateViewClamp(localClientNum, prevSnap))
    {
        for (i = 0; i < 2; ++i)
        {
            v5 = prevSnap->ps.viewAngleClampBase[i];
            frameInterpolation = cgameGlob->frameInterpolation;
            out->viewAngleClampBase[i] = AngleNormalize180(nextSnap->ps.viewAngleClampBase[i] - v5) * frameInterpolation + v5;
            out->viewAngleClampRange[i] = (float)((float)(nextSnap->ps.viewAngleClampRange[i]
                - prevSnap->ps.viewAngleClampRange[i])
                * cgameGlob->frameInterpolation)
                + prevSnap->ps.viewAngleClampRange[i];
        }
    }

    //if (cmd)
    //{
    //    if ((prevSnap->ps.eFlags & 0x4000) != 0
    //        && prevSnap->ps.vehiclePos == nextSnap->ps.vehiclePos
    //        && prevSnap->ps.vehiclePos >= 1
    //        && prevSnap->ps.vehiclePos <= 4)
    //    {
    //        vehCent = CG_GetEntity(localClientNum, out->viewlocked_entNum);
    //        vehicleTypeStr[0] = "t34";
    //        vehicleTypeStr[1] = "panzer";
    //        vehType = CG_GetVehicleTypeString(cgameGlob->clientNum, out->viewlocked_entNum);
    //        if (vehType)
    //        {
    //            for (vehicleType = 0; vehicleType < 2; ++vehicleType)
    //            {
    //                v3 = SL_ConvertToString(vehType);
    //                if (!I_stricmp(vehicleTypeStr[vehicleType], v3))
    //                    break;
    //            }
    //            if (vehicleType < 2)
    //            {
    //                pitchClamp = 360.0f;
    //                vehAngles[0] = vehCent->pose.angles[0];
    //                vehAngles[1] = vehCent->pose.angles[1];
    //                vehAngles[2] = vehCent->pose.angles[2];
    //                if (prevSnap->ps.vehiclePos)
    //                {
    //                    gunnerClamps = gunnerClampArrays[vehicleType];
    //                    vehAngles[1] = (float)((float)vehCent->pose.vehicle.yaw * 0.0054931641) + vehAngles[1];
    //                }
    //                else
    //                {
    //                    gunnerClamps = driverClampArrays[vehicleType];
    //                }
    //                yawOffset = AngleNormalize360((float)((float)((float)cmd->angles[1] * 0.0054931641) + out->delta_angles[1]) - vehAngles[1]);
    //                if (prevSnap->ps.vehiclePos)
    //                    clampCount = gunnerClampCounts[vehicleType];
    //                else
    //                    clampCount = driverClampCounts[vehicleType];
    //                for (i = 0; i < clampCount && gunnerClamps[i][1] <= yawOffset; ++i)
    //                    ;
    //                v4 = (float)((float)((float)(yawOffset - gunnerClamps[i - 1][1])
    //                    / (float)(gunnerClamps[i][1] - gunnerClamps[i - 1][1]))
    //                    * 3.1415927)
    //                    - 1.5707964;
    //                fc = cos(v4);
    //                v12 = sin(v4);
    //                yawFrac = (float)(v12 + 1.0) / 2.0;
    //                pitchClamp = (float)((float)(gunnerClamps[i][0] - gunnerClamps[i - 1][0]) * yawFrac) + gunnerClamps[i - 1][0];
    //                if (prevSnap->ps.vehiclePos)
    //                {
    //                    out->viewAngleClampBase[0] = (float)(pitchClamp - out->viewAngleClampRange[0]) / 2.0;
    //                    out->viewAngleClampRange[0] = (float)(pitchClamp + out->viewAngleClampRange[0]) / 2.0;
    //                    out->viewAngleClampBase[0] = out->viewAngleClampBase[0] + cgameGlob->gunnerPitchOffset;
    //                }
    //                else
    //                {
    //                    AnglesToAxis(vehAngles, vehMat);
    //                    viewYawAngles[0] = 0.0f;
    //                    viewYawAngles[1] = yawOffset;
    //                    viewYawAngles[2] = 0.0f;
    //                    AnglesToAxis(viewYawAngles, viewMat);
    //                    MatrixMultiply(viewMat, vehMat, resultMat);
    //                    AxisToAngles(resultMat, centerAngles);
    //                    out->viewAngleClampBase[0] = (float)((float)(pitchClamp - out->viewAngleClampRange[0]) / 2.0)
    //                        + centerAngles[0];
    //                    out->viewAngleClampRange[0] = (float)(pitchClamp + out->viewAngleClampRange[0]) / 2.0;
    //                }
    //            }
    //        }
    //    }
    //}
}

void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles, int grabStance)
{
    snapshot_s *nextSnap; // r27
    snapshot_s *prevSnap; // r31
    int CurrentCmdNumber; // r3
    double f; // fp31
    int i; // r10
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
    usercmd_s cmd; // [sp+60h] [-B0h] BYREF

    cg_s *cgameGlob;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    playerState_s *out;
    out = &cgameGlob->predictedPlayerState;

    nextSnap = cgameGlob->nextSnap;
    prevSnap = cgameGlob->snap;

    iassert(nextSnap);

    memcpy(&cgArray[0].predictedPlayerState, &nextSnap->ps, sizeof(cgArray[0].predictedPlayerState));

    if (grabAngles)
    {
        CurrentCmdNumber = CL_GetCurrentCmdNumber(localClientNum);
        CL_GetUserCmd(localClientNum, CurrentCmdNumber, &cmd);
        PM_UpdateViewAngles(&cgArray[0].predictedPlayerState, 0.0, &cmd, 0);
    }

    if (nextSnap->serverTime > prevSnap->serverTime)
    {
        f = cgArray[0].frameInterpolation;

        if (grabStance)
        {
            i = nextSnap->ps.bobCycle;
            if (i < prevSnap->ps.bobCycle)
                i += 256;

            //v12 = prevSnap->ps.bobCycle;
            //HIDWORD(v13) = v24;
            //LODWORD(v13) = i - v12;
            //v24[1] = v13;
            //v24[0] = (int)(float)((float)v13 * cgArray[0].frameInterpolation);
            //cgArray[0].predictedPlayerState.bobCycle = v24[0] + v12;
            out->bobCycle = prevSnap->ps.bobCycle + (int)((double)(i - prevSnap->ps.bobCycle) * f);
            out->leanf = (float)((float)(nextSnap->ps.leanf - prevSnap->ps.leanf)
                * cgArray[0].frameInterpolation)
                + prevSnap->ps.leanf;
            out->aimSpreadScale = (float)((float)(nextSnap->ps.aimSpreadScale
                - prevSnap->ps.aimSpreadScale)
                * cgArray[0].frameInterpolation)
                + prevSnap->ps.aimSpreadScale;
            out->fWeaponPosFrac = (float)((float)(nextSnap->ps.fWeaponPosFrac
                - prevSnap->ps.fWeaponPosFrac)
                * cgArray[0].frameInterpolation)
                + prevSnap->ps.fWeaponPosFrac;
            out->viewHeightCurrent = (float)((float)(nextSnap->ps.viewHeightCurrent
                - prevSnap->ps.viewHeightCurrent)
                * cgArray[0].frameInterpolation)
                + prevSnap->ps.viewHeightCurrent;
        }

        //linkAngles = nextSnap->ps.linkAngles;
        //origin = prevSnap->ps.origin;
        //v16 = (char *)nextSnap - (char *)prevSnap;
        //v17 = (char *)((char *)&cgArray[0].oldTime - (char *)prevSnap);
        //v18 = (char *)((char *)&out->weaponrechamber[1] - (char *)prevSnap);
        //v19 = 3;
        //do
        //{
        //    *(float *)&v17[(unsigned int)origin] = (float)((float)(*(float *)((char *)origin + v16) - *origin)
        //        * (float)f)
        //        + *origin;
        //    if (!grabAngles)
        //        *(float *)((char *)origin + (char *)&out->offhandSecondary - (char *)prevSnap) = LerpAngle(origin[50], *(linkAngles - 294), f);
        //    *(float *)((char *)origin + (char *)&out->pm_type - (char *)prevSnap) = (float)((float)(*(linkAngles - 341) - origin[3]) * (float)f) + origin[3];
        //    if (prevSnap->ps.pm_type == 1)
        //        *(float *)&v18[(unsigned int)origin] = LerpAngle(origin[344], *linkAngles, f);
        //    else
        //        *(float *)&v18[(unsigned int)origin] = *linkAngles;
        //    if (nextSnap->ps.pm_type == 1 && (nextSnap->ps.eFlags & 0x20000) == 0)
        //        *(float *)((char *)origin + (char *)&out->weaponDelay - (char *)prevSnap) = LerpAngle(origin[18], *(linkAngles - 326), f);
        //    --v19;
        //    ++origin;
        //    ++linkAngles;
        //} while (v19);

        for (int link = 0; link < 3; ++link)
        {
            float v7 = prevSnap->ps.viewangles[link];
            float v2 = AngleNormalize180(nextSnap->ps.viewangles[link] - v7);
            out->viewangles[link] = v2 * f + v7;

            float v6 = prevSnap->ps.delta_angles[link];
            float v3 = AngleNormalize180(nextSnap->ps.delta_angles[link] - v6);
            out->delta_angles[link] = v3 * f + v6;

            if (prevSnap->ps.pm_type == 1)
            {
                float v5 = prevSnap->ps.linkAngles[link];
                float v4 = AngleNormalize180(nextSnap->ps.linkAngles[link] - v5);
                out->linkAngles[link] = v4 * f + v5;
            }
            else
            {
                out->linkAngles[link] = nextSnap->ps.linkAngles[link];
            }
        }

        CG_InterpolatePlayerStateViewAngles(localClientNum, out, 0);
        //if (prevSnap->ps.pm_type == PM_NORMAL_LINKED || (prevSnap->ps.eFlags & 0x300) != 0)
        //{
        //    viewAngleClampBase = prevSnap->ps.viewAngleClampBase;
        //    viewAngleClampRange = nextSnap->ps.viewAngleClampRange;
        //    v22 = 2;
        //    v23 = (char *)((char *)&cgArray[0].predictedPlayerState - (char *)prevSnap);
        //    do
        //    {
        //        *(float *)&v17[(unsigned int)viewAngleClampBase] = LerpAngle(
        //            *viewAngleClampBase,
        //            *(float *)((char *)viewAngleClampBase + v16),
        //            f);
        //        --v22;
        //        *(float *)((char *)viewAngleClampBase + (unsigned int)v23) = (float)((float)(*viewAngleClampRange++
        //            - viewAngleClampBase[2])
        //            * (float)f)
        //            + viewAngleClampBase[2];
        //        ++viewAngleClampBase;
        //    } while (v22);
        //}
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
    double yaw; // fp1
    float origin[6]; // [sp+50h] [-30h] BYREF

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
    cgameGlob->predictedPlayerState.pm_type = (pmtype_t)v3;
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
    origin[0] = (float)((float)(nextSnap->ps.origin[0] - snap->ps.origin[0]) * cgameGlob->frameInterpolation)
        + snap->ps.origin[0];
    origin[1] = (float)((float)(nextSnap->ps.origin[1] - snap->ps.origin[1]) * (float)frameInterpolation) + snap->ps.origin[1];
    origin[2] = (float)((float)(nextSnap->ps.origin[2] - snap->ps.origin[2]) * (float)frameInterpolation) + snap->ps.origin[2];
    yaw = LerpAngle(snap->ps.viewangles[1], nextSnap->ps.viewangles[1], frameInterpolation);
    if (cg_drawPlayerPosInFreeMove->current.enabled)
        CG_DebugBox(origin, cg_pmove.mins, cg_pmove.maxs, yaw, colorRed, 1, 0);
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

void __cdecl CG_PredictPlayerState_Internal(int localClientNum) // KISAKTODO: use the MP version to further clean this up
{
    playerState_s *Buf; // r20
    int v3; // r11
    snapshot_s *nextSnap; // r11
    int CurrentCmdNumber; // r27
    int v6; // r28
    int i; // r30
    double v9; // fp30
    double v10; // fp29
    double v11; // fp28
    double len; // fp1
    __int64 v13; // r10
    double f; // fp31
    double v15; // r5
    double v16; // fp0
    double v17; // fp13
    double v18; // fp12
    __int64 v20; // r8
    _BYTE v21[12]; // r11 OVERLAPPED
    double v22; // fp0
    double v23; // fp13
    double v24; // fp0
    __int64 v27; // r11
    double v28; // fp0
    __int64 v29; // fp13
    __int64 v30; // r11
    LargeLocal v31(45784); // [sp+58h] [-118h] BYREF
    __int64 v32; // [sp+60h] [-110h]
    float adjusted[3];
    float deltaAngles[3];
    //float v33; // [sp+68h] [-108h] BYREF
    //float v34; // [sp+6Ch] [-104h]
    //float v35; // [sp+70h] [-100h]
    float v36[4]; // [sp+78h] [-F8h] BYREF
    //float v37[6]; // [sp+88h] [-E8h] BYREF
    usercmd_s v38; // [sp+A0h] [-D0h] BYREF

    //LargeLocal::LargeLocal(&v31, 45784);
    //Buf = (playerState_s *)LargeLocal::GetBuf(&v31);
    Buf = (playerState_s *)v31.GetBuf();
    
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
            deltaAngles);
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
                        adjusted,
                        deltaAngles);
                    cgArray[0].predictedPlayerState.delta_angles[1] = cgArray[0].predictedPlayerState.delta_angles[1] + deltaAngles[1];
                    if (cg_showmiss->current.integer && (Buf->origin[0] != adjusted[0] || Buf->origin[1] != adjusted[1]|| Buf->origin[2] != adjusted[2]))
                    {
                        Com_PrintError(17, "prediction error\n");
                    }
                    v9 = (float)(Buf->origin[1] - adjusted[1]);
                    v10 = (float)(Buf->origin[2] - adjusted[2]);
                    v11 = (float)(Buf->origin[0] - adjusted[0]);
                    len = sqrtf((((Buf->origin[0] - adjusted[0]) * (Buf->origin[0] - adjusted[0]))
                        + (((Buf->origin[2] - adjusted[2]) * (Buf->origin[2] - adjusted[2]))
                            + ((Buf->origin[1] - adjusted[1]) * (Buf->origin[1] - adjusted[1])))));
                    if (len > 0.1)
                    {
                        if (cg_showmiss->current.integer)
                        {
                            Com_Printf(17, "Prediction miss: %f\n", len);
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
                            f = (float)((float)(cg_errorDecay->current.value - (float)v13) / cg_errorDecay->current.value);
                            if (f >= 0.0)
                            {
                                if (f > 0.0 && cg_showmiss->current.integer)
                                {
                                    Com_Printf(17, "Double prediction decay: %f\n", f);
                                }
                            }
                            else
                            {
                                f = 0.0;
                            }
                            v16 = (float)(cgArray[0].predictedError[0] * (float)f);
                            v17 = (float)(cgArray[0].predictedError[1] * (float)f);
                            v18 = (float)(cgArray[0].predictedError[2] * (float)f);
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
        if (cg_showmiss->current.integer > 1)
        {
            Com_Printf(17, "[%i : %i] ", cg_pmove.cmd.serverTime, cgArray[0].time);
        }
        if (!i && cg_showmiss->current.integer)
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

                //v24 = (float)(cg_pmove.viewChange + (float)v22);
                //if (v24 <= 0.0)
                //    _FP12 = (float)((float)-cg_viewZSmoothingMax->current.value - (float)v24);
                //else
                //    _FP12 = (float)((float)v24 - cg_viewZSmoothingMax->current.value);
                //__asm { fsel      f0, f12, f13, f0 }
                cgArray[0].stepViewChange = max(cg_pmove.viewChange + v22, cg_viewZSmoothingMax->current.value);

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
    ; // goto
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

