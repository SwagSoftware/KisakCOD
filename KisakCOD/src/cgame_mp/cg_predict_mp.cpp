#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <bgame/bg_public.h>
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>
#include <universal/profile.h>

static pmove_t cg_pmove[1];
static centity_s* cg_itemEntities[512];
static int cg_itemEntityCount;
static int cg_itemLocalClientNum;

void __cdecl TRACK_cg_predict()
{
    track_static_alloc_internal(cg_pmove, 272, "cg_pmove", 9);
    track_static_alloc_internal(cg_itemEntities, 2048, "cg_itemEntities", 9);
}

int __cdecl CG_ItemListLocalClientNum()
{
    return cg_itemLocalClientNum;
}

void __cdecl CG_ClearItemList()
{
    cg_itemEntityCount = 0;
}

void __cdecl CG_BuildItemList(int localClientNum, const snapshot_s *nextSnap)
{
    centity_s *cent; // [esp+0h] [ebp-Ch]
    int entIndex; // [esp+4h] [ebp-8h]

    if (!nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 70, 0, "%s", "nextSnap");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 71, 0, "%s", "Sys_IsMainThread()");
    CG_ClearItemList();
    cg_itemLocalClientNum = localClientNum;
    for (entIndex = 0; entIndex < nextSnap->numEntities; ++entIndex)
    {
        cent = CG_GetEntity(localClientNum, nextSnap->entities[entIndex].number);
        if (cent->nextState.eType == 3)
            cg_itemEntities[cg_itemEntityCount++] = cent;
    }
}

void __cdecl CG_PredictPlayerState(int localClientNum)
{
    centity_s* Entity; // eax
    double v2; // st7
    float v3; // [esp+10h] [ebp-3Ch]
    float v4; // [esp+14h] [ebp-38h]
    float* origin; // [esp+24h] [ebp-28h]
    float angle; // [esp+2Ch] [ebp-20h]
    float size; // [esp+34h] [ebp-18h]
    float fZoom; // [esp+38h] [ebp-14h] BYREF
    WeaponDef* weapDef; // [esp+3Ch] [ebp-10h]
    float fCos; // [esp+40h] [ebp-Ch]
    playerState_s* ps; // [esp+44h] [ebp-8h]
    float fSin; // [esp+48h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ps = &cgArray[0].predictedPlayerState;
    cgArray[0].lastFrame.aimSpreadScale = cgArray[0].predictedPlayerState.aimSpreadScale;
    CG_PredictPlayerState_Internal(localClientNum);
    Entity = CG_GetEntity(localClientNum, ps->clientNum);
    origin = ps->origin;
    Entity->pose.origin[0] = ps->origin[0];
    Entity->pose.origin[1] = origin[1];
    Entity->pose.origin[2] = origin[2];
    BG_EvaluateTrajectory(&Entity->currentState.apos, cgArray[0].time, Entity->pose.angles);
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ((cgArray[0].nextSnap->ps.otherFlags & 4) != 0 && CG_GetWeapReticleZoom(cgArray, &fZoom))
    {
        if (weapDef->adsViewErrorMax != 0.0 && !cgArray[0].adsViewErrorDone)
        {
            cgArray[0].adsViewErrorDone = 1;
            size = flrand(weapDef->adsViewErrorMin, weapDef->adsViewErrorMax);
            v2 = random();
            angle = (v2 + v2) * 3.141592741012573;
            fCos = cos(angle);
            fSin = sin(angle);
            v4 = fSin * size + cgArray[0].offsetAngles[0];
            cgArray[0].offsetAngles[0] = AngleNormalize360(v4);
            v3 = fCos * size + cgArray[0].offsetAngles[1];
            cgArray[0].offsetAngles[1] = AngleNormalize360(v3);
        }
    }
    else
    {
        cgArray[0].adsViewErrorDone = 0;
    }
    cgArray[0].predictedPlayerEntity.nextState.number = LOWORD(ps->clientNum);
    BG_PlayerStateToEntityState(ps, &cgArray[0].predictedPlayerEntity.nextState, 0, 0);
    memcpy(
        &cgArray[0].predictedPlayerEntity.currentState,
        &cgArray[0].predictedPlayerEntity.nextState.lerp,
        sizeof(cgArray[0].predictedPlayerEntity.currentState));
    CL_SetUserCmdOrigin(localClientNum, ps->origin, ps->velocity, ps->viewangles, ps->bobCycle, ps->movementDir);
    CL_SendCmd(localClientNum);
}

void __cdecl CG_PredictPlayerState_Internal(int localClientNum)
{
    float v1; // [esp+10h] [ebp-188h]
    float v2; // [esp+14h] [ebp-184h]
    float v3; // [esp+18h] [ebp-180h]
    float v4; // [esp+1Ch] [ebp-17Ch]
    float v5; // [esp+24h] [ebp-174h]
    float v7; // [esp+48h] [ebp-150h]
    float v8; // [esp+4Ch] [ebp-14Ch]
    float value; // [esp+50h] [ebp-148h]
    float v10; // [esp+54h] [ebp-144h]
    float* viewangles; // [esp+58h] [ebp-140h]
    float* predictedError; // [esp+8Ch] [ebp-10Ch]
    int timeSinceStart; // [esp+B8h] [ebp-E0h]
    int smoothingDuration; // [esp+BCh] [ebp-DCh]
    float lerp; // [esp+C0h] [ebp-D8h]
    float diff; // [esp+C4h] [ebp-D4h]
    float stepRemaining; // [esp+C8h] [ebp-D0h]
    float f; // [esp+D4h] [ebp-C4h]
    float adjusted[3]; // [esp+D8h] [ebp-C0h] BYREF
    float delta[3]; // [esp+E4h] [ebp-B4h] BYREF
    float len; // [esp+F0h] [ebp-A8h]
    transPlayerState_t oldTransPlayerState; // [esp+F4h] [ebp-A4h] BYREF
    cg_s* cgameGlob; // [esp+10Ch] [ebp-8Ch]
    int bPredictionRun; // [esp+110h] [ebp-88h]
    usercmd_s latestCmd; // [esp+114h] [ebp-84h] BYREF
    float oldViewangles[3]; // [esp+134h] [ebp-64h]
    float deltaAngles[3]; // [esp+140h] [ebp-58h] BYREF
    int cmdNum; // [esp+14Ch] [ebp-4Ch]
    const playerState_s* oldPlayerState; // [esp+150h] [ebp-48h]
    float oldOrigin[3]; // [esp+154h] [ebp-44h] BYREF
    usercmd_s oldestCmd; // [esp+160h] [ebp-38h] BYREF
    int oldCommandTime; // [esp+184h] [ebp-14h]
    const playerState_s* ps; // [esp+188h] [ebp-10h]
    float oldVelocity[2]; // [esp+18Ch] [ebp-Ch]
    int current; // [esp+194h] [ebp-4h]

    PROF_SCOPED("CG_PredictPlayerState");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    if (!cgArray[0].nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 280, 0, "%s", "cgameGlob->nextSnap");
    ps = &cgameGlob->nextSnap->ps;
    if (cgameGlob->demoType || (ps->otherFlags & 2) != 0)
    {
        CG_InterpolatePlayerState(localClientNum, 0);
    }
    else if (cg_nopredict->current.enabled || cg_synchronousClients->current.enabled)
    {
        CG_InterpolatePlayerState(localClientNum, 1);
    }
    else
    {
        cg_pmove[localClientNum].ps = &cgameGlob->predictedPlayerState;
        cg_pmove[localClientNum].handler = 0;
        if (cg_pmove[localClientNum].ps->pm_type < 7)
            cg_pmove[localClientNum].tracemask = 0x2810011;
        else
            cg_pmove[localClientNum].tracemask = 0x810011;
        if (ps->pm_type == 4)
            cg_pmove[localClientNum].tracemask &= 0xFDFEFFFF;
        cg_pmove[localClientNum].viewChange = 0.0;
        cg_pmove[localClientNum].viewChangeTime = cgameGlob->stepViewStart;
        current = CL_GetCurrentCmdNumber(localClientNum);
        cmdNum = current - 127;
        if (CL_GetUserCmd(localClientNum, current - 127, &oldestCmd))
        {
            oldPlayerState = &cgameGlob->predictedPlayerState;
            oldCommandTime = cgameGlob->predictedPlayerState.commandTime;
            oldOrigin[0] = cgameGlob->predictedPlayerState.origin[0];
            oldOrigin[1] = cgameGlob->predictedPlayerState.origin[1];
            oldOrigin[2] = cgameGlob->predictedPlayerState.origin[2];
            oldVelocity[0] = cgameGlob->predictedPlayerState.oldVelocity[0];
            oldVelocity[1] = cgameGlob->predictedPlayerState.oldVelocity[1];
            oldViewangles[0] = cgameGlob->predictedPlayerState.viewangles[0];
            oldViewangles[1] = cgameGlob->predictedPlayerState.viewangles[1];
            oldViewangles[2] = cgameGlob->predictedPlayerState.viewangles[2];
            CG_ExtractTransPlayerState(&cgameGlob->predictedPlayerState, &oldTransPlayerState);
            CL_GetUserCmd(localClientNum, current, &latestCmd);
            memcpy(
                (unsigned __int8*)&cgameGlob->predictedPlayerState,
                (unsigned __int8*)ps,
                sizeof(cgameGlob->predictedPlayerState));
            cgameGlob->physicsTime = cgameGlob->nextSnap->serverTime;
            //*(_QWORD*)cgameGlob->predictedPlayerState.oldVelocity = oldVelocity;
            cgameGlob->predictedPlayerState.oldVelocity[0] = oldVelocity[0];
            cgameGlob->predictedPlayerState.oldVelocity[1] = oldVelocity[1];
            if (cgameGlob->predictedPlayerState.pm_type == 1 || cgameGlob->predictedPlayerState.pm_type == 8)
                CG_InterpolatePlayerState(localClientNum, 0);
            CG_AdjustPositionForMover(
                localClientNum,
                cgameGlob->predictedPlayerState.origin,
                cgameGlob->predictedPlayerState.groundEntityNum,
                cgameGlob->physicsTime,
                cgameGlob->time,
                cgameGlob->predictedPlayerState.origin,
                deltaAngles);
            bPredictionRun = 0;
            if (ps->commandTime >= latestCmd.serverTime)
            {
                viewangles = cgameGlob->predictedPlayerState.viewangles;
                cgameGlob->predictedPlayerState.viewangles[0] = oldViewangles[0];
                viewangles[1] = oldViewangles[1];
                viewangles[2] = oldViewangles[2];
            }
            else
            {
                for (cmdNum = current - 127; cmdNum <= current; ++cmdNum)
                {
                    CL_GetUserCmd(localClientNum, cmdNum, &cg_pmove[localClientNum].cmd);
                    if (cg_pmove[localClientNum].cmd.serverTime > cgameGlob->predictedPlayerState.commandTime
                        && cg_pmove[localClientNum].cmd.serverTime <= latestCmd.serverTime
                        && CL_GetUserCmd(localClientNum, cmdNum - 1, &cg_pmove[localClientNum].oldcmd))
                    {
                        if (cgameGlob->predictedPlayerState.commandTime == oldCommandTime)
                        {
                            CG_AdjustPositionForMover(
                                localClientNum,
                                cgameGlob->predictedPlayerState.origin,
                                cgameGlob->predictedPlayerState.groundEntityNum,
                                cgameGlob->time,
                                cgameGlob->oldTime,
                                adjusted,
                                deltaAngles);
                            cgameGlob->predictedPlayerState.delta_angles[1] = cgameGlob->predictedPlayerState.delta_angles[1]
                                + deltaAngles[1];
                            if (cg_showmiss->current.integer)
                            {
                                if (adjusted[0] != oldOrigin[0] || adjusted[1] != oldOrigin[1] || adjusted[2] != oldOrigin[2])
                                    Com_PrintError(17, "prediction error\n");
                            }
                            Vec3Sub(oldOrigin, adjusted, delta);
                            len = Vec3Length(delta);
                            if (len > 0.1000000014901161)
                            {
                                if (cg_showmiss->current.integer)
                                    Com_Printf(17, "Prediction miss: %f\n", len);
                                if (cg_errorDecay->current.value == 0.0)
                                {
                                    predictedError = cgameGlob->predictedError;
                                    cgameGlob->predictedError[0] = 0.0;
                                    predictedError[1] = 0.0;
                                    predictedError[2] = 0.0;
                                }
                                else
                                {
                                    f = (cg_errorDecay->current.value - (double)(cgameGlob->time - cgameGlob->predictedErrorTime))
                                        / cg_errorDecay->current.value;
                                    if (f < 0.0)
                                        f = 0.0;
                                    if (f > 0.0 && cg_showmiss->current.integer)
                                        Com_Printf(17, "Double prediction decay: %f\n", f);
                                    Vec3Scale(cgameGlob->predictedError, f, cgameGlob->predictedError);
                                }
                                Vec3Add(delta, cgameGlob->predictedError, cgameGlob->predictedError);
                                cgameGlob->predictedErrorTime = cgameGlob->oldTime;
                            }
                        }
                        {
                            PROF_SCOPED("CG_Pmove");
                            Pmove(&cg_pmove[localClientNum]);
                        }
                        CG_TouchItemPrediction(localClientNum);
                        bPredictionRun = 1;
                    }
                }
            }
            if (cg_showmiss->current.integer > 1)
                Com_Printf(17, "[%i : %i] ", cg_pmove[localClientNum].cmd.serverTime, cgameGlob->time);
            if (!bPredictionRun && cg_showmiss->current.integer)
                Com_Printf(17, "no prediction run\n");
            CG_TransitionPlayerState(localClientNum, &cgameGlob->predictedPlayerState, &oldTransPlayerState);
            if ((cgameGlob->predictedPlayerState.pm_flags & 0x400) != 0)
                CL_SetStance(localClientNum, CL_STANCE_STAND);
            if (!cgameGlob->predictedPlayerState.cursorHint)
            {
                cgameGlob->predictedPlayerState.cursorHintString = -1;
                cgameGlob->predictedPlayerState.cursorHintEntIndex = 1023;
            }
            if (cg_pmove[localClientNum].viewChange == 0.0
                || cg_pmove[localClientNum].viewChangeTime == cgameGlob->stepViewStart
                || cgameGlob->playerTeleported
                || ps->pm_type && ps->pm_type != 2 && ps->pm_type != 3)
            {
                if (cg_viewZSmoothingTime->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->stepViewStart))
                    cgameGlob->stepViewChange = 0.0;
            }
            else
            {
                diff = cg_pmove[localClientNum].viewChange;
                v5 = fabs(diff);
                if (cg_viewZSmoothingMin->current.value > (double)v5)
                {
                    if (cg_viewZSmoothingTime->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->stepViewStart))
                        cgameGlob->stepViewChange = 0.0;
                }
                else
                {
                    stepRemaining = 0.0;
                    if (cg_viewZSmoothingTime->current.value * 1000.0 > (double)(cgameGlob->time - cgameGlob->stepViewStart))
                    {
                        timeSinceStart = cg_pmove[localClientNum].viewChangeTime - cgameGlob->stepViewStart;
                        smoothingDuration = (int)(cg_viewZSmoothingTime->current.value * 1000.0);
                        if (timeSinceStart >= 0 && timeSinceStart < smoothingDuration)
                        {
                            lerp = (double)timeSinceStart * 1.0 / (double)smoothingDuration;
                            stepRemaining = (1.0 - lerp) * cgameGlob->stepViewChange;
                        }
                    }
                    if (stepRemaining + diff <= 0.0)
                    {
                        v7 = -cg_viewZSmoothingMax->current.value;
                        v8 = diff + stepRemaining;
                        v2 = v7 - v8;
                        if (v2 < 0.0)
                            v1 = diff + stepRemaining;
                        else
                            v1 = -cg_viewZSmoothingMax->current.value;
                        cgameGlob->stepViewChange = v1;
                    }
                    else
                    {
                        value = cg_viewZSmoothingMax->current.value;
                        v10 = diff + stepRemaining;
                        v4 = v10 - value;
                        if (v4 < 0.0)
                            v3 = diff + stepRemaining;
                        else
                            v3 = value;
                        cgameGlob->stepViewChange = v3;
                    }
                    cgameGlob->stepViewStart = cg_pmove[localClientNum].viewChangeTime;
                }
            }
        }
        else
        {
            if (cg_showmiss->current.integer)
                Com_Printf(17, "exceeded PACKET_BACKUP on commands\n");
        }
    }
}

void __cdecl CG_TouchItemPrediction(int localClientNum)
{
    centity_s *cent; // [esp+4h] [ebp-8h]
    int entIndex; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.pm_type <= 1u)
    {
        for (entIndex = 0; entIndex < cg_itemEntityCount; ++entIndex)
        {
            cent = cg_itemEntities[entIndex];
            if (cent->nextState.eType != 3)
                MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 145, 0, "%s", "cent->nextState.eType == ET_ITEM");
            CG_TouchItem(cgArray, cent);
        }
    }
}

void __cdecl CG_TouchItem(cg_s *cgameGlob, centity_s *cent)
{
    int bitNum; // [esp+0h] [ebp-8h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 96, 0, "%s", "cgameGlob");
    if (!cent)
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 97, 0, "%s", "cent");
    if (cg_predictItems->current.enabled
        && BG_PlayerTouchesItem(&cgameGlob->predictedPlayerState, &cent->nextState, cgameGlob->time)
        && cent->miscTime != cgameGlob->time)
    {
        bitNum = cent->nextState.index.brushmodel % 128;
        if (cgameGlob == (cg_s *)-287036)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(cgameGlob->predictedPlayerState.weapons, bitNum, 16)
            && BG_CanItemBeGrabbed(&cent->nextState, &cgameGlob->predictedPlayerState, 1))
        {
            if (BG_PlayerHasRoomForEntAllAmmoTypes(&cent->nextState, &cgameGlob->predictedPlayerState))
            {
                cent->nextState.lerp.eFlags |= 0x20u;
                cent->miscTime = cgameGlob->time;
                BG_AddPredictableEventToPlayerstate(
                    9u,
                    cent->nextState.index.brushmodel % 128,
                    &cgameGlob->predictedPlayerState);
            }
        }
    }
}

void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles)
{
    float v2; // [esp+Ch] [ebp-7Ch]
    float v3; // [esp+10h] [ebp-78h]
    float v4; // [esp+14h] [ebp-74h]
    float v5; // [esp+18h] [ebp-70h]
    float v6; // [esp+1Ch] [ebp-6Ch]
    float v7; // [esp+20h] [ebp-68h]
    float v8; // [esp+24h] [ebp-64h]
    float v9; // [esp+28h] [ebp-60h]
    float v10; // [esp+30h] [ebp-58h]
    float v11; // [esp+38h] [ebp-50h]
    float v12; // [esp+3Ch] [ebp-4Ch]
    float v13; // [esp+44h] [ebp-44h]
    int cmdNum; // [esp+48h] [ebp-40h]
    usercmd_s cmd; // [esp+4Ch] [ebp-3Ch] BYREF
    playerState_s* out; // [esp+70h] [ebp-18h]
    cg_s* cgameGlob; // [esp+74h] [ebp-14h]
    const snapshot_s* prevSnap; // [esp+78h] [ebp-10h]
    const snapshot_s* nextSnap; // [esp+7Ch] [ebp-Ch]
    float f; // [esp+80h] [ebp-8h]
    int i; // [esp+84h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    out = &cgArray[0].predictedPlayerState;
    prevSnap = cgArray[0].snap;
    nextSnap = cgArray[0].nextSnap;
    if (!prevSnap)
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 173, 0, "%s", "prevSnap");
    if (!nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_predict_mp.cpp", 174, 0, "%s", "nextSnap");
    memcpy((unsigned __int8*)out, (unsigned __int8*)&nextSnap->ps, sizeof(playerState_s));
    if (grabAngles)
    {
        cmdNum = CL_GetCurrentCmdNumber(localClientNum);
        CL_GetUserCmd(localClientNum, cmdNum, &cmd);
        PM_UpdateViewAngles(out, 0.0, &cmd, 0);
    }
    if (nextSnap->serverTime > prevSnap->serverTime)
    {
        f = cgameGlob->frameInterpolation;
        i = nextSnap->ps.bobCycle;
        if (i < prevSnap->ps.bobCycle)
            i += 256;
        out->bobCycle = prevSnap->ps.bobCycle + (int)((double)(i - prevSnap->ps.bobCycle) * f);
        out->aimSpreadScale = (nextSnap->ps.aimSpreadScale - prevSnap->ps.aimSpreadScale) * f + prevSnap->ps.aimSpreadScale;
        Vec3Lerp(prevSnap->ps.origin, nextSnap->ps.origin, f, out->origin);
        Vec3Lerp(prevSnap->ps.velocity, nextSnap->ps.velocity, f, out->velocity);
        if (!grabAngles)
        {
            for (i = 0; i < 3; ++i)
            {
                v12 = prevSnap->ps.viewangles[i];
                v9 = nextSnap->ps.viewangles[i] - v12;
                v13 = v9 * 0.002777777845039964;
                v8 = v13 + 0.5;
                v7 = floor(v8);
                v6 = (v13 - v7) * 360.0;
                out->viewangles[i] = v6 * f + v12;
                v10 = prevSnap->ps.delta_angles[i];
                v5 = nextSnap->ps.delta_angles[i] - v10;
                v11 = v5 * 0.002777777845039964;
                v4 = v11 + 0.5;
                v3 = floor(v4);
                v2 = (v11 - v3) * 360.0;
                out->delta_angles[i] = v2 * f + v10;
            }
        }
        out->viewHeightCurrent = (nextSnap->ps.viewHeightCurrent - prevSnap->ps.viewHeightCurrent) * f
            + prevSnap->ps.viewHeightCurrent;
        out->leanf = (nextSnap->ps.leanf - prevSnap->ps.leanf) * f + prevSnap->ps.leanf;
        out->fWeaponPosFrac = (nextSnap->ps.fWeaponPosFrac - prevSnap->ps.fWeaponPosFrac) * f + prevSnap->ps.fWeaponPosFrac;
        if (!nextSnap->ps.cursorHint)
        {
            out->cursorHintString = -1;
            out->cursorHintEntIndex = 1023;
        }
    }
}