#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <client_mp/client_mp.h>

#include <physics/phys_local.h>
#include <ragdoll/ragdoll.h>
#include <gfx_d3d/r_dpvs.h>
#include <gfx_d3d/r_model.h>
#include <EffectsCore/fx_system.h>
#include <aim_assist/aim_assist.h>
#include <universal/profile.h>

void __cdecl CG_ShutdownEntity(int localClientNum, centity_s *cent)
{
    trajectory_t *p_pos; // [esp+4h] [ebp-4h]

    if (cent->pose.isRagdoll)
        goto LABEL_9;
    p_pos = &cent->currentState.pos;
    if (cent == (centity_s *)-104)
        MyAssertHandler("c:\\trees\\cod3\\src\\cgame_mp\\../universal/q_shared.h", 1957, 0, "%s", "trajectory");
    if (p_pos->trType >= TR_FIRST_RAGDOLL && p_pos->trType <= TR_RAGDOLL_INTERPOLATE)
    {
    LABEL_9:
        cent->pose.isRagdoll = 0;
        if (cent->pose.ragdollHandle)
        {
            Ragdoll_Remove(cent->pose.ragdollHandle);
            cent->pose.ragdollHandle = 0;
        }
        if (cent->pose.killcamRagdollHandle)
        {
            Ragdoll_Remove(cent->pose.killcamRagdollHandle);
            cent->pose.killcamRagdollHandle = 0;
        }
        cent->currentState.pos.trType = TR_STATIONARY;
        cent->currentState.apos.trType = TR_STATIONARY;
    }
    if (cent->pose.physObjId && cent->pose.physObjId != -1 || cent->currentState.pos.trType == TR_PHYSICS)
    {
        if (cent->pose.physObjId != -1 && cent->pose.physObjId)
        {
            if (CG_IsEntityLinked(localClientNum, cent->nextState.number))
                CG_UnlinkEntity(localClientNum, cent->nextState.number);
            Phys_ObjDestroy(PHYS_WORLD_FX, (dxBody *)cent->pose.physObjId);
        }
        cent->currentState.pos.trType = TR_STATIONARY;
        cent->currentState.apos.trType = TR_STATIONARY;
        cent->pose.physObjId = 0;
    }
}

void __cdecl CG_SetInitialSnapshot(int localClientNum, snapshot_s *snap)
{
    float clientViewAxis[3][3]; // [esp+10h] [ebp-34h] BYREF
    float clientViewOrigin[3]; // [esp+34h] [ebp-10h] BYREF
    int i; // [esp+40h] [ebp-4h]

    R_InitSceneData(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_SetNextSnap(localClientNum, 0);
    if (cgArray[0].nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 248, 0, "%s", "!cgameGlob->nextSnap");
    cgArray[0].snap = snap;
    cgArray[0].nextSnap = snap;
    cgArray[0].time = snap->serverTime;
    cgArray[0].bgs.time = cgArray[0].time;
    cgArray[0].oldTime = cgArray[0].time;
    clientViewOrigin[0] = snap->ps.origin[0];
    clientViewOrigin[1] = snap->ps.origin[1];
    clientViewOrigin[2] = snap->ps.origin[2];
    clientViewOrigin[2] = clientViewOrigin[2] + snap->ps.viewHeightCurrent;
    AnglesToAxis(snap->ps.viewangles, clientViewAxis);
    SND_SetListener(localClientNum, snap->ps.clientNum, clientViewOrigin, clientViewAxis);
    for (i = 0; i < 1024; ++i)
    {
        if (CG_GetEntity(localClientNum, i)->nextValid)
            MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 265, 0, "%s", "!CG_GetEntity( localClientNum, i )->nextValid");
    }
    SND_FadeAllSounds(1.0, 0);
    CG_Respawn(localClientNum);
    CG_RestartSmokeGrenades(localClientNum);
    CG_ClearEntityCollWorld(localClientNum);
    CG_InitView(localClientNum);
    cgArray[0].nextSnap = 0;
}

void __cdecl CG_ExtractTransPlayerState(const playerState_s *ps, transPlayerState_t *transPs)
{
    transPs->damageEvent = ps->damageEvent;
    transPs->eventSequence = ps->eventSequence;
    transPs->events[0] = ps->events[0];
    transPs->events[1] = ps->events[1];
    transPs->events[2] = ps->events[2];
    transPs->events[3] = ps->events[3];
}

void __cdecl CG_SetNextSnap(int localClientNum, snapshot_s *snap)
{
    char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    XModel *v5; // eax
    int v6; // eax
    char *name; // [esp-4h] [ebp-22Ch]
    team_t team; // [esp+58h] [ebp-1D0h]
    float *predictedError; // [esp+5Ch] [ebp-1CCh]
    float *v10; // [esp+60h] [ebp-1C8h]
    transPlayerState_t transPs; // [esp+64h] [ebp-1C4h] BYREF
    const char *tagName; // [esp+7Ch] [ebp-1ACh]
    unsigned int centInPrevSnapshot[32]; // [esp+80h] [ebp-1A8h] BYREF
    cg_s *cgameGlob; // [esp+100h] [ebp-128h]
    centity_s *cent; // [esp+104h] [ebp-124h]
    const char *modelName; // [esp+108h] [ebp-120h]
    entityState_s *v17; // [esp+10Ch] [ebp-11Ch]
    clientState_s *clientState; // [esp+110h] [ebp-118h]
    clientInfo_t *ci; // [esp+114h] [ebp-114h]
    int clientIndex[64]; // [esp+118h] [ebp-110h] BYREF
    int num; // [esp+21Ch] [ebp-Ch]
    int entnum; // [esp+220h] [ebp-8h]
    int i; // [esp+224h] [ebp-4h]

    memset((unsigned __int8 *)centInPrevSnapshot, 0, sizeof(centInPrevSnapshot));
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    if (cgArray[0].nextSnap)
    {
        for (num = 0; num < cgameGlob->nextSnap->numEntities; ++num)
        {
            v17 = &cgameGlob->nextSnap->entities[num];
            entnum = v17->number;
            cent = CG_GetEntity(localClientNum, entnum);
            if (!cent->nextValid)
                MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 445, 0, "%s", "cent->nextValid");
            cent->nextValid = 0;
            centInPrevSnapshot[entnum >> 5] |= 0x80000000 >> (entnum & 0x1F);
        }
        entnum = cgameGlob->nextSnap->ps.clientNum;
        cent = CG_GetEntity(localClientNum, entnum);
        if (cent->nextValid)
        {
            cent->nextValid = 0;
            centInPrevSnapshot[entnum >> 5] |= 0x80000000 >> (entnum & 0x1F);
        }
    }
    cgameGlob->nextSnap = snap;
    if (snap)
    {
        if (!cgameGlob->snap)
            MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 479, 0, "%s", "cgameGlob->snap");
        CG_UpdateViewOffset(localClientNum);
        CG_SetFrameInterpolation(localClientNum);
        CG_ExecuteNewServerCommands(localClientNum, snap->serverCommandSequence);
        CG_CheckOpenWaitingScriptMenu(localClientNum);
        memset((unsigned __int8 *)clientIndex, 0, sizeof(clientIndex));
        for (num = 0; num < snap->numClients; ++num)
        {
            clientState = &snap->clients[num];
            if (clientIndex[clientState->clientIndex])
                MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 499, 0, "%s", "!clientIndex[clientState->clientIndex]");
            clientIndex[clientState->clientIndex] = 1;
            if (clientState->clientIndex >= 0x40u)
                MyAssertHandler(
                    ".\\cgame_mp\\cg_snapshot_mp.cpp",
                    502,
                    0,
                    "clientState->clientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientState->clientIndex,
                    64);
            ci = &cgameGlob->bgs.clientinfo[clientState->clientIndex];
            if (ci->infoValid)
                team = ci->team;
            else
                team = clientState->team;
            ci->oldteam = team;
            ci->infoValid = 1;
            ci->nextValid = 1;
            ci->clientNum = clientState->clientIndex;
            ci->team = clientState->team;
            ci->rank = clientState->rank;
            ci->prestige = clientState->prestige;
            ci->perks = clientState->perks;
            ci->attachedVehEntNum = clientState->attachedVehEntNum;
            ci->attachedVehSlotIndex = clientState->attachedVehSlotIndex;
            if (strcmp(ci->name, clientState->name))
            {
                if (ci->name[0])
                {
                    name = clientState->name;
                    v2 = UI_SafeTranslateString("CGAME_PLAYERRENAMES");
                    v3 = va("%s^7 %s %s", ci->name, v2, name);
                    CG_GameMessage(localClientNum, v3);
                }
                I_strncpyz(ci->name, clientState->name, 16);
            }
            modelName = CL_GetConfigString(localClientNum, clientState->modelindex + 830);
            if (strcmp(ci->model, modelName))
            {
                I_strncpyz(ci->model, (char *)modelName, 64);
                ci->dobjDirty = 1;
            }
            for (i = 0; i < 6; ++i)
            {
                modelName = CL_GetConfigString(localClientNum, clientState->attachModelIndex[i] + 830);
                if (strcmp(ci->attachModelNames[i], modelName))
                {
                    I_strncpyz(ci->attachModelNames[i], (char *)modelName, 64);
                    ci->dobjDirty = 1;
                }
                tagName = CL_GetConfigString(localClientNum, clientState->attachTagIndex[i] + 2282);
                if (strcmp(ci->attachTagNames[i], tagName))
                {
                    I_strncpyz(ci->attachTagNames[i], (char *)tagName, 64);
                    ci->dobjDirty = 1;
                }
            }
        }
        cgameGlob->identifyClientNum = snap->ps.stats[3];
        entnum = snap->ps.clientNum;
        cgameGlob->playerTeleported = 0;
        if ((snap->ps.otherFlags & 6) != 0)
        {
            cent = CG_GetEntity(localClientNum, entnum);
            cent->nextState.number = (unsigned __int16)entnum;
            BG_PlayerStateToEntityState(&snap->ps, &cent->nextState, 0, 0);
            cent->nextValid = 1;
            if (!cgameGlob->mapRestart
                && snap->ps.stats[4] == cgameGlob->snap->ps.stats[4]
                && entnum == cgameGlob->snap->ps.clientNum)
            {
                if ((centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 && !cgameGlob->playerTeleported)
                {
                    if (((cent->nextState.lerp.eFlags ^ cent->currentState.eFlags) & 2) != 0)
                    {
                        memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
                        CG_ResetEntity(localClientNum, cent, 0);
                        predictedError = cgameGlob->predictedError;
                        cgameGlob->predictedError[0] = 0.0;
                        predictedError[1] = 0.0;
                        predictedError[2] = 0.0;
                    }
                }
                else
                {
                    memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
                    CG_ResetEntity(localClientNum, cent, 1);
                    v10 = cgameGlob->predictedError;
                    cgameGlob->predictedError[0] = 0.0;
                    v10[1] = 0.0;
                    v10[2] = 0.0;
                }
            }
            else
            {
                cgameGlob->playerTeleported = 1;
                centInPrevSnapshot[cgameGlob->snap->ps.clientNum >> 5] &= ~(0x80000000 >> (cgameGlob->snap->ps.clientNum & 0x1F));
                memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
                CG_ResetEntity(localClientNum, cent, 1);
                CG_Respawn(localClientNum);
            }
        }
        else if (snap->ps.pm_type == 4)
        {
            cent = CG_GetEntity(localClientNum, entnum);
            centInPrevSnapshot[cgameGlob->snap->ps.clientNum >> 5] &= ~(0x80000000 >> (cgameGlob->snap->ps.clientNum & 0x1F));
            CG_ResetEntity(localClientNum, cent, 1);
        }
        else if (cgameGlob->mapRestart
            || snap->ps.stats[4] != cgameGlob->snap->ps.stats[4]
            || entnum != cgameGlob->snap->ps.clientNum)
        {
            memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
            CG_Respawn(localClientNum);
        }
        for (num = 0; num < snap->numEntities; ++num)
        {
            v17 = &snap->entities[num];
            entnum = v17->number;
            cent = CG_GetEntity(localClientNum, entnum);
            memcpy(&cent->nextState, v17, sizeof(cent->nextState));
            if (cent->nextValid)
            {
                v4 = va(
                    "entnum %d num %d numEntities %d clientNum %d flags 0x%x",
                    entnum,
                    num,
                    snap->numEntities,
                    snap->ps.clientNum,
                    snap->ps.pm_flags);
                MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 619, 0, "%s\n\t%s", "!cent->nextValid", v4);
            }
            cent->nextValid = 1;
            if ((centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0)
            {
                if (((v17->lerp.eFlags ^ cent->currentState.eFlags) & 2) != 0)
                    CG_ResetEntity(localClientNum, cent, 0);
            }
            else
            {
                CG_ResetEntity(localClientNum, cent, 1);
            }
            centInPrevSnapshot[entnum >> 5] &= ~(0x80000000 >> (entnum & 0x1F));
        }
        for (num = 0; num < cgameGlob->snap->numEntities; ++num)
        {
            v17 = &cgameGlob->snap->entities[num];
            entnum = v17->number;
            if ((centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0)
            {
                R_UnlinkEntity(localClientNum, entnum);
                CG_UnlinkEntity(localClientNum, entnum);
                FX_MarkEntDetachAll(localClientNum, entnum);
                cent = CG_GetEntity(localClientNum, entnum);
                CG_ShutdownEntity(localClientNum, cent);
                if (cent->nextState.eType == 8 || cent->nextState.eType == 9)
                {
                    if (cent->pose.fx.effect)
                    {
                        if (cgameGlob->mapRestart)
                            MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 651, 0, "%s", "!cgameGlob->mapRestart");
                        FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect);
                        cent->pose.fx.effect = 0;
                        cent->pose.fx.triggerTime = 0;
                    }
                    if (cent->pose.fx.triggerTime)
                        MyAssertHandler(
                            ".\\cgame_mp\\cg_snapshot_mp.cpp",
                            656,
                            0,
                            "%s\n\t(cent->pose.fx.triggerTime) = %i",
                            "(cent->pose.fx.triggerTime == 0)",
                            cent->pose.fx.triggerTime);
                }
            }
        }
        for (num = 0; num < snap->numClients; ++num)
        {
            cent = CG_GetEntity(localClientNum, snap->clients[num].clientIndex);
            CG_UpdatePlayerDObj(localClientNum, cent);
        }
        CG_UpdateWeaponViewmodels(localClientNum);
        if (snap->ps.viewmodelIndex > 0)
        {
            modelName = CL_GetConfigString(localClientNum, snap->ps.viewmodelIndex + 830);
            if (!modelName || !*modelName)
                MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 672, 0, "%s", "modelName && modelName[0]");
            v5 = R_RegisterModel(modelName);
            CG_UpdateHandViewmodels(localClientNum, v5);
        }
        CG_BuildItemList(localClientNum, cgameGlob->nextSnap);
        CG_TransitionKillcam(localClientNum);
        for (num = 0; num < snap->numEntities; ++num)
        {
            cent = CG_GetEntity(localClientNum, snap->entities[num].number);
            CG_CheckEvents(localClientNum, cent);
        }
        if (cgameGlob->demoType
            || (cgameGlob->nextSnap->ps.otherFlags & 2) != 0
            || cg_nopredict->current.enabled
            || cg_synchronousClients->current.enabled)
        {
            CG_ExtractTransPlayerState(&cgameGlob->snap->ps, &transPs);
            v6 = CG_TransitionPlayerState(localClientNum, &cgameGlob->nextSnap->ps, &transPs);
            cgameGlob->snap->ps.eventSequence = v6;
        }
        cgameGlob->mapRestart = 0;
        for (i = 0; i < 64; ++i)
        {
            ci = &cgameGlob->bgs.clientinfo[i];
            if (!ci->infoValid && Com_GetClientDObj(i, localClientNum))
                MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 706, 0, "%s", "!Com_GetClientDObj( i, localClientNum )");
        }
    }
    else
    {
        if (cgameGlob->snap)
        {
            for (num = 0; num < cgameGlob->snap->numEntities; ++num)
            {
                v17 = &cgameGlob->snap->entities[num];
                entnum = v17->number;
                cent = CG_GetEntity(localClientNum, entnum);
                CG_ShutdownEntity(localClientNum, cent);
                CG_ClearUnion(localClientNum, cent);
            }
            cgameGlob->snap = 0;
        }
        CG_ClearItemList();
    }
}

void __cdecl CG_ResetEntity(int localClientNum, centity_s *cent, int newEntity)
{
    float *v3; // [esp+Ch] [ebp-30h]
    DObj_s *obj; // [esp+1Ch] [ebp-20h]
    DObj_s *obja; // [esp+1Ch] [ebp-20h]
    XAnimTree_s *pXAnimTree; // [esp+24h] [ebp-18h]
    unsigned int corpseIndex; // [esp+2Ch] [ebp-10h]
    clientInfo_t *cia; // [esp+30h] [ebp-Ch]
    clientInfo_t *ci; // [esp+30h] [ebp-Ch]
    int i; // [esp+34h] [ebp-8h]
    clientInfo_t *corpseInfo; // [esp+38h] [ebp-4h]
    int savedregs; // [esp+3Ch] [ebp+0h] BYREF

    cent->lightingOrigin[0] = 0.0;
    cent->lightingOrigin[1] = 0.0;
    cent->lightingOrigin[2] = 0.0;
    CG_ShutdownEntity(localClientNum, cent);
    CG_ClearUnion(localClientNum, cent);
    AimAssist_ClearEntityReference(localClientNum, cent->nextState.number);
    qmemcpy(&cent->currentState, &cent->nextState.lerp, sizeof(cent->currentState));
    cent->bTrailMade = 0;
    cent->pose.cullIn = 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgArray[0].time, cent->pose.origin);
    BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgArray[0].time, cent->pose.angles);
    if (cent->pose.localClientNum != localClientNum)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 120, 0, "%s", "cent->pose.localClientNum == localClientNum");
    cent->pose.eType = cent->nextState.eType;
    if (cent->pose.eType != cent->nextState.eType)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 123, 0, "%s", "cent->pose.eType == cent->nextState.eType");
    CG_UnlinkEntity(localClientNum, cent->nextState.number);
    switch (cent->nextState.eType)
    {
    case 0:
    case 4:
        if ((cent->nextState.lerp.eFlags & 0x10000) != 0
            && cgArray[0].time - cent->nextState.lerp.u.missile.launchTime > 200)
        {
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
        else
        {
            cent->previousEventSequence = 0;
        }
        goto LABEL_43;
    case 1:
        for (i = 0; i < 6; ++i)
            cent->pose.player.tag[i] = -2;
        cent->previousEventSequence = cent->nextState.eventSequence;
        if (cent->nextState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_snapshot_mp.cpp",
                144,
                0,
                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                64);
        cia = &cgArray[0].bgs.clientinfo[cent->nextState.clientNum];
        cia->lerpMoveDir = (float)cent->nextState.lerp.u.loopFx.period;
        cia->lerpLean = cent->nextState.lerp.u.turret.gunAngles[0];
        cia->playerAngles[0] = cent->pose.angles[0];
        cia->playerAngles[1] = cent->pose.angles[1];
        cia->playerAngles[2] = cent->pose.angles[2];
        cent->pose.angles[0] = 0.0;
        cent->pose.angles[2] = 0.0;
        CG_ResetPlayerEntity(localClientNum, cgArray, cent, newEntity);
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obj)
            CG_Player_PreControllers(obj, cent);
        goto LABEL_43;
    case 2:
        corpseIndex = cent->nextState.number - 64;
        if (cent->nextState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_snapshot_mp.cpp",
                160,
                0,
                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                64);
        ci = &cgArray[0].bgs.clientinfo[cent->nextState.clientNum];
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (corpseIndex >= 8)
            MyAssertHandler(
                ".\\cgame_mp\\cg_snapshot_mp.cpp",
                163,
                0,
                "corpseIndex doesn't index MAX_CLIENT_CORPSES\n\t%i not in [0, %i)",
                corpseIndex,
                8);
        corpseInfo = &cgsArray[0].corpseinfo[corpseIndex];
        pXAnimTree = cgsArray[0].corpseinfo[corpseIndex].pXAnimTree;
        if ((cent->nextState.lerp.eFlags & 0x80000) != 0)
        {
            CG_CopyCorpseInfo(corpseInfo, ci);
            cgsArray[0].corpseinfo[corpseIndex].pXAnimTree = pXAnimTree;
            XAnimCloneAnimTree(ci->pXAnimTree, pXAnimTree);
            cent->previousEventSequence = 0;
        }
        else
        {
            if (!cgsArray[0].corpseinfo[corpseIndex].model[0]
                || cgsArray[0].corpseinfo[corpseIndex].clientNum != ci->clientNum)
            {
                CG_CopyCorpseInfo(corpseInfo, ci);
                cgsArray[0].corpseinfo[corpseIndex].pXAnimTree = pXAnimTree;
            }
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
        cgsArray[0].corpseinfo[corpseIndex].dobjDirty = 1;
        goto LABEL_43;
    case 6:
    case 0xD:
        cent->previousEventSequence = cent->nextState.eventSequence;
        if (cent->nextState.solid != 0xFFFFFF)
            goto LABEL_43;
        CG_UpdateBModelWorldBounds(localClientNum, cent, 1);
        break;
    case 8:
    case 9:
        if (cent->pose.fx.effect)
            MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 204, 0, "%s", "!cent->pose.fx.effect");
        goto LABEL_43;
    case 0xB:
        cent->previousEventSequence = cent->nextState.eventSequence;
        cent->pose.turret.tag_aim = -2;
        cent->pose.turret.tag_aim_animated = -2;
        cent->pose.turret.tag_flash = -2;
        obja = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obja)
            CG_mg42_PreControllers(obja, cent);
        goto LABEL_43;
    default:
        cent->previousEventSequence = cent->nextState.eventSequence;
    LABEL_43:
        v3 = cg_entityOriginArray[localClientNum][cent->nextState.number];
        *v3 = 131072.0;
        v3[1] = 131072.0;
        v3[2] = 131072.0;
        break;
    }
}

void __cdecl CG_CopyCorpseInfo(clientInfo_t *corpseInfo, const clientInfo_t *ci)
{
    int attachIndex; // [esp+8h] [ebp-4h]

    memcpy(corpseInfo, ci, sizeof(clientInfo_t));
    for (attachIndex = 0; attachIndex < 6; ++attachIndex)
    {
        if (!I_stricmp(corpseInfo->attachTagNames[attachIndex], "J_Spine4"))
        {
            corpseInfo->attachModelNames[attachIndex][0] = 0;
            corpseInfo->attachTagNames[attachIndex][0] = 0;
        }
    }
}

void __cdecl CG_TransitionKillcam(int localClientNum)
{
    XAnimTree_s *pXAnimTree; // [esp+Ch] [ebp-24h]
    centity_s *cent; // [esp+10h] [ebp-20h]
    unsigned int corpseIndex; // [esp+18h] [ebp-18h]
    int anim; // [esp+20h] [ebp-10h]
    int i; // [esp+24h] [ebp-Ch]
    XAnim_s *anims; // [esp+28h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].inKillCam && cgArray[0].nextSnap->ps.deltaTime)
    {
        cgArray[0].inKillCam = 1;
        CG_SetEquippedOffHand(localClientNum, 0);
        CG_RestartSmokeGrenades(localClientNum);
    }
    if (cgArray[0].inKillCam && !cgArray[0].nextSnap->ps.deltaTime)
    {
        cgArray[0].inKillCam = 0;
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        CG_RestartSmokeGrenades(localClientNum);
        for (i = 0; i < cgArray[0].nextSnap->numEntities; ++i)
        {
            cent = CG_GetEntity(localClientNum, cgArray[0].nextSnap->entities[i].number);
            CG_ResetEntity(localClientNum, cent, 1);
            if (cent->nextState.eType == 2)
            {
                corpseIndex = cent->nextState.number - 64;
                if (corpseIndex >= 8)
                    MyAssertHandler(
                        ".\\cgame_mp\\cg_snapshot_mp.cpp",
                        327,
                        0,
                        "corpseIndex doesn't index MAX_CLIENT_CORPSES\n\t%i not in [0, %i)",
                        corpseIndex,
                        8);
                pXAnimTree = cgsArray[0].corpseinfo[corpseIndex].pXAnimTree;
                anim = cgsArray[0].corpseinfo[corpseIndex].legs.animationNumber & 0xFFFFFDFF;
                anims = XAnimGetAnims(pXAnimTree);
                if (anim && !XAnimIsLooped(anims, anim) && !XAnimGetNumChildren(anims, anim))
                    XAnimSetTime(pXAnimTree, anim, 1.0);
            }
        }
    }
}

void __cdecl CG_ProcessSnapshots(int localClientNum)
{
    snapshot_s *snap; // [esp+34h] [ebp-8h]
    snapshot_s *snapa; // [esp+34h] [ebp-8h]
    int n; // [esp+38h] [ebp-4h] BYREF

    KISAK_NULLSUB();
    PROF_SCOPED("CG_ProcessSnapshots");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CL_GetCurrentSnapshotNumber(localClientNum, &n, &cgArray[0].latestSnapshotTime);
    if (n != cgArray[0].latestSnapshotNum)
    {
        if (n < cgArray[0].latestSnapshotNum)
            Com_Error(ERR_DROP, "G_ProcessSnapshots: n < cgameGlob->latestSnapshotNum");
        cgArray[0].latestSnapshotNum = n;
    }
    cgArray[0].bgs.latestSnapshotTime = cgArray[0].latestSnapshotTime;
    while (!cgArray[0].snap)
    {
        snap = CG_ReadNextSnapshot(localClientNum);
        if (!snap)
            return;
        if ((snap->snapFlags & 2) == 0)
        {
            CG_SetInitialSnapshot(localClientNum, snap);
            CG_SetNextSnap(localClientNum, snap);
            CG_TransitionSnapshot(localClientNum);
            AimAssist_Setup(localClientNum);
            if (!cg_fs_debug->current.integer)
                Dvar_SetInt(cg_fs_debug, 2);
        }
    }
    CG_SetFrameInterpolation(localClientNum);
    while (1)
    {
        while (1)
        {
            if (cgArray[0].nextSnap != cgArray[0].snap && cgArray[0].cubemapShot == CUBEMAPSHOT_NONE)
                goto LABEL_25;
            snapa = CG_ReadNextSnapshot(localClientNum);
            if (!snapa)
                goto LABEL_28;
            if (!cgArray[0].nextSnap)
                MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 865, 0, "%s", "cgameGlob->nextSnap");
            if (((cgArray[0].nextSnap->snapFlags ^ snapa->snapFlags) & 4) == 0)
                break;
            CG_SetInitialSnapshot(localClientNum, snapa);
            CG_SetNextSnap(localClientNum, snapa);
            CG_TransitionSnapshot(localClientNum);
        }
        if (snapa->serverTime - cgArray[0].nextSnap->serverTime < 0)
            Com_Error(ERR_DROP, "CG_ProcessSnapshots: Server time went backwards");
        CG_SetNextSnap(localClientNum, snapa);
    LABEL_25:
        if (cgArray[0].time - cgArray[0].snap->serverTime >= 0 && cgArray[0].time - cgArray[0].nextSnap->serverTime < 0)
            break;
        CG_TransitionSnapshot(localClientNum);
    }
LABEL_28:
    if (CG_ItemListLocalClientNum() != localClientNum)
        CG_BuildItemList(localClientNum, cgArray[0].nextSnap);
    if (!cgArray[0].snap)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 892, 0, "%s", "cgameGlob->snap");
    if (cgArray[0].time - cgArray[0].snap->serverTime < 0)
    {
        cgArray[0].time = cgArray[0].snap->serverTime;
        cgArray[0].bgs.time = cgArray[0].time;
    }
    if (!cgArray[0].nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 900, 0, "%s", "cgameGlob->nextSnap");
    if (cgArray[0].nextSnap != cgArray[0].snap && cgArray[0].nextSnap->serverTime - cgArray[0].time <= 0)
        MyAssertHandler(
            ".\\cgame_mp\\cg_snapshot_mp.cpp",
            901,
            0,
            "%s",
            "cgameGlob->nextSnap == cgameGlob->snap || cgameGlob->nextSnap->serverTime - cgameGlob->time > 0");
}

void __cdecl CG_TransitionSnapshot(int localClientNum)
{
    centity_s *cent; // [esp+Ch] [ebp-18h]
    centity_s *centa; // [esp+Ch] [ebp-18h]
    XAnimTree_s *pXAnimTree; // [esp+10h] [ebp-14h]
    clientState_s *clientState; // [esp+14h] [ebp-10h]
    clientInfo_t *ci; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]
    signed int ia; // [esp+1Ch] [ebp-8h]
    int ib; // [esp+1Ch] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].snap)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 360, 0, "%s", "cgameGlob->snap");
    if (!cgArray[0].nextSnap)
        MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 361, 0, "%s", "cgameGlob->nextSnap");
    for (i = 0; i < cgArray[0].snap->numClients; ++i)
    {
        clientState = &cgArray[0].snap->clients[i];
        if (clientState->clientIndex >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_snapshot_mp.cpp",
                366,
                0,
                "clientState->clientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                clientState->clientIndex,
                64);
        ci = &cgArray[0].bgs.clientinfo[clientState->clientIndex];
        if (cgArray[0].bgs.clientinfo[clientState->clientIndex].nextValid)
        {
            cgArray[0].bgs.clientinfo[clientState->clientIndex].nextValid = 0;
        }
        else
        {
            pXAnimTree = cgArray[0].bgs.clientinfo[clientState->clientIndex].pXAnimTree;
            memset((unsigned __int8 *)ci, 0, sizeof(clientInfo_t));
            ci->pXAnimTree = pXAnimTree;
            XAnimClearTree(ci->pXAnimTree);
            CG_SafeDObjFree(localClientNum, clientState->clientIndex);
        }
    }
    for (ia = 0; ia < 64; ++ia)
    {
        if (!cgArray[0].bgs.clientinfo[ia].infoValid && Com_GetClientDObj(ia, localClientNum))
            MyAssertHandler(".\\cgame_mp\\cg_snapshot_mp.cpp", 386, 0, "%s", "!Com_GetClientDObj( i, localClientNum )");
    }
    cgArray[0].snap = cgArray[0].nextSnap;
    if ((cgArray[0].nextSnap->ps.otherFlags & 6) != 0)
    {
        cent = CG_GetEntity(localClientNum, cgArray[0].nextSnap->ps.clientNum);
        qmemcpy(&cent->currentState, &cent->nextState.lerp, sizeof(cent->currentState));
    }
    for (ib = 0; ib < cgArray[0].nextSnap->numEntities; ++ib)
    {
        centa = CG_GetEntity(localClientNum, cgArray[0].nextSnap->entities[ib].number);
        qmemcpy(&centa->currentState, &centa->nextState.lerp, sizeof(centa->currentState));
    }
}

snapshot_s *__cdecl CG_ReadNextSnapshot(int localClientNum)
{
    snapshot_s *dest; // [esp+Ch] [ebp-4h]

    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    if (cgArray[0].latestSnapshotNum > cgsArray[0].processedSnapshotNum + 1000)
        Com_PrintWarning(
            14,
            "WARNING: CG_ReadNextSnapshot: way out of range, %i > %i\n",
            cgArray[0].latestSnapshotNum,
            cgsArray[0].processedSnapshotNum);
    while (cgsArray[0].processedSnapshotNum < cgArray[0].latestSnapshotNum)
    {
        dest = &cgArray[0].activeSnapshots[cgArray[0].snap == cgArray[0].activeSnapshots];
        if (CL_GetSnapshot(localClientNum, ++cgsArray[0].processedSnapshotNum, dest))
        {
            CG_AddLagometerSnapshotInfo(dest);
            return dest;
        }
        CG_AddLagometerSnapshotInfo(0);
    }
    return 0;
}
