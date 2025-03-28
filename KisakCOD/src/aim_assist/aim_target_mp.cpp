#include "aim_assist.h"
#include <qcommon/mem_track.h>
#include <qcommon/cmd.h>
#include <bgame/bg_local.h>

AimTargetGlob atGlobArray[1];

const dvar_t *aim_target_sentient_radius;

int __cdecl AimTarget_GetTagPos(int localClientNum, const centity_s *cent, unsigned int tagName, float *pos)
{
    char *v5; // eax
    DObj_s *dobj; // [esp+0h] [ebp-4h]

    if (!cent)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 646, 0, "%s", "cent");
    if (!pos)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 647, 0, "%s", "pos");
    dobj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if (!dobj)
        return 0;
    if (!CG_DObjGetWorldTagPos(&cent->pose, dobj, tagName, pos))
    {
        v5 = SL_ConvertToString(tagName);
        Com_Error(ERR_DROP, "AimTarget_GetTagPos: Cannot find tag [%s] on entity\n", v5);
    }
    return 1;
}

void __cdecl TRACK_aim_target()
{
    track_static_alloc_internal(atGlobArray, 5640, "atGlobArray", 10);
}

void __cdecl AimTarget_Init(int localClientNum)
{
    memset((unsigned __int8 *)&atGlobArray[localClientNum], 0, sizeof(AimTargetGlob));
    AimTarget_RegisterDvars();
    Cbuf_InsertText(0, "exec devgui_aimassist\n");
}

const dvar_s *AimTarget_RegisterDvars()
{
    const dvar_s *result; // eax
    DvarLimits min; // [esp+4h] [ebp-10h]

    min.value.max = 128.0;
    min.value.min = 0.0;
    result = Dvar_RegisterFloat(
        "aim_target_sentient_radius",
        10.0,
        min,
        0x80u,
        "The radius used to calculate target bounds for a sentient(actor or player)");
    aim_target_sentient_radius = result;
    return result;
}

void __cdecl AimTarget_ClearTargetList(int localClientNum)
{
    atGlobArray[localClientNum].targetCount = 0;
}

void __cdecl AimTarget_ProcessEntity(int localClientNum, const centity_s *ent)
{
    AimTarget target; // [esp+50h] [ebp-30h] BYREF
    unsigned int visBone; // [esp+7Ch] [ebp-4h]

    //Profile_Begin(58);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\aim_assist\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!ent)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 466, 0, "%s", "ent");
    if (!ent->nextValid)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 467, 0, "%s", "ent->nextValid");
    if (ent->nextState.number == cgArray[0].predictedPlayerState.clientNum)
        MyAssertHandler(
            ".\\aim_assist\\aim_target_mp.cpp",
            468,
            0,
            "%s",
            "ent->nextState.number != cgameGlob->predictedPlayerState.clientNum");
    if (!AimTarget_PlayerInValidState(&cgArray[0].predictedPlayerState))
        goto LABEL_20;
    if (ent->nextState.eType == 1)
    {
        visBone = scr_const.aim_vis_bone;
    }
    else
    {
        if ((ent->nextState.lerp.eFlags & 0x800) == 0)
            MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 478, 0, "%s", "ent->nextState.lerp.eFlags & EF_AIM_ASSIST");
        if ((clientActive_t *)ent->nextState.solid != (clientActive_t *)((char *)&clients[0].parseClients[238].attachTagIndex[4]
            + 3))
            MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 479, 0, "%s", "ent->nextState.solid == SOLID_BMODEL");
        visBone = 0;
    }
    if (AimTarget_IsTargetValid(cgArray, ent) && AimTarget_IsTargetVisible(localClientNum, ent, visBone))
    {
        AimTarget_CreateTarget(localClientNum, ent, &target);
        //Profile_EndInternal(0);
    }
    else
    {
    LABEL_20:
        //Profile_EndInternal(0);
    }
}

char __cdecl AimTarget_IsTargetValid(const cg_s *cgameGlob, const centity_s *targetEnt)
{
    double v3; // st7
    float targetDir[3]; // [esp+50h] [ebp-28h] BYREF
    const clientInfo_t *playerInfo; // [esp+5Ch] [ebp-1Ch]
    const clientInfo_t *targetInfo; // [esp+60h] [ebp-18h]
    float radius; // [esp+64h] [ebp-14h]
    float playerDir[3]; // [esp+68h] [ebp-10h] BYREF
    float dot; // [esp+74h] [ebp-4h]

    //Profile_Begin(59);
    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 262, 0, "%s", "targetEnt");
    if (!targetEnt->nextValid)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 263, 0, "%s", "targetEnt->nextValid");
    if (targetEnt->nextState.number == cgameGlob->predictedPlayerState.clientNum)
        MyAssertHandler(
            ".\\aim_assist\\aim_target_mp.cpp",
            264,
            0,
            "%s",
            "targetEnt->nextState.number != cgameGlob->predictedPlayerState.clientNum");
    if (targetEnt->nextState.eType == 1)
    {
        if ((targetEnt->nextState.lerp.eFlags & 0x20000) != 0)
            MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 268, 0, "%s", "!(targetEnt->nextState.lerp.eFlags & EF_DEAD)");
        if (targetEnt->nextState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\aim_assist\\aim_target_mp.cpp",
                270,
                0,
                "targetEnt->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                targetEnt->nextState.clientNum,
                64);
        targetInfo = &cgameGlob->bgs.clientinfo[targetEnt->nextState.clientNum];
        if (cgameGlob->predictedPlayerState.clientNum >= 0x40u)
            MyAssertHandler(
                ".\\aim_assist\\aim_target_mp.cpp",
                273,
                0,
                "cgameGlob->predictedPlayerState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cgameGlob->predictedPlayerState.clientNum,
                64);
        playerInfo = &cgameGlob->bgs.clientinfo[cgameGlob->predictedPlayerState.clientNum];
        if (targetInfo->infoValid && targetInfo->model[0])
        {
            if (!Com_GetClientDObj(targetEnt->nextState.number, targetEnt->pose.localClientNum))
                MyAssertHandler(
                    ".\\aim_assist\\aim_target_mp.cpp",
                    282,
                    0,
                    "%s",
                    "Com_GetClientDObj( targetEnt->nextState.number, targetEnt->pose.localClientNum )");
            if (targetInfo->team != playerInfo->team || playerInfo->team == TEAM_FREE)
                goto LABEL_26;
        }
    LABEL_25:
        //Profile_EndInternal(0);
        return 0;
    }
    if ((targetEnt->nextState.lerp.eFlags & 0x800) == 0
        || (clientActive_t *)targetEnt->nextState.solid != (clientActive_t *)((char *)&clients[0].parseClients[238].attachTagIndex[4]
            + 3))
    {
        goto LABEL_25;
    }
LABEL_26:
    Vec3Sub(targetEnt->pose.origin, cgameGlob->predictedPlayerState.origin, targetDir);
    playerDir[0] = cgameGlob->refdef.viewaxis[0][0];
    playerDir[1] = cgameGlob->refdef.viewaxis[0][1];
    playerDir[2] = cgameGlob->refdef.viewaxis[0][2];
    radius = AimTarget_GetTargetRadius(targetEnt);
    v3 = Vec3Dot(playerDir, targetDir);
    dot = v3 + radius;
    if (dot >= 0.0)
    {
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
        //Profile_EndInternal(0);
        return 0;
    }
}

double __cdecl AimTarget_GetTargetRadius(const centity_s *targetEnt)
{
    float mins[3]; // [esp+0h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+10h] [ebp-Ch] BYREF

    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 217, 0, "%s", "targetEnt");
    if (targetEnt->nextState.eType == 1)
        return aim_target_sentient_radius->current.value;
    AimTarget_GetTargetBounds(targetEnt, mins, maxs);
    return (float)RadiusFromBounds(mins, maxs);
}

void __cdecl AimTarget_GetTargetBounds(const centity_s *targetEnt, float *mins, float *maxs)
{
    float highBonePos[3]; // [esp+0h] [ebp-Ch] BYREF

    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 183, 0, "%s", "targetEnt");
    if (!mins)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 184, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 185, 0, "%s", "maxs");
    if (targetEnt->nextState.eType == 1)
    {
        AimTarget_GetTagPos(targetEnt, scr_const.aim_highest_bone, highBonePos);
        *mins = -aim_target_sentient_radius->current.value;
        mins[1] = -aim_target_sentient_radius->current.value;
        mins[2] = 0.0;
        *maxs = aim_target_sentient_radius->current.value;
        maxs[1] = aim_target_sentient_radius->current.value;
        maxs[2] = highBonePos[2] - targetEnt->pose.origin[2];
    }
    else
    {
        if ((clientActive_t *)targetEnt->nextState.solid != (clientActive_t *)((char *)&clients[0].parseClients[238].attachTagIndex[4]
            + 3))
            MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 201, 0, "%s", "targetEnt->nextState.solid == SOLID_BMODEL");
        CM_ModelBounds(targetEnt->nextState.index.brushmodel, mins, maxs);
    }
}

void __cdecl AimTarget_GetTagPos(const centity_s *ent, unsigned int tagName, float *pos)
{
    char *v3; // eax
    DObj_s *dobj; // [esp+0h] [ebp-4h]

    dobj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
    if (!dobj)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 169, 0, "%s", "dobj");
    if (!CG_DObjGetWorldTagPos(&ent->pose, dobj, tagName, pos))
    {
        v3 = SL_ConvertToString(tagName);
        Com_Error(ERR_DROP, "AimTarget_GetTagPos: Cannot find tag [%s] on entity\n", v3);
    }
}

char __cdecl AimTarget_IsTargetVisible(int localClientNum, const centity_s *targetEnt, unsigned int visBone)
{
    float endPos[4]; // [esp+58h] [ebp-58h] BYREF
    trace_t trace; // [esp+68h] [ebp-48h] BYREF
    float playerEyePos[3]; // [esp+94h] [ebp-1Ch] BYREF
    float targetEyePos[3]; // [esp+A0h] [ebp-10h] BYREF
    float visibility; // [esp+ACh] [ebp-4h]

    //Profile_Begin(60);
    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 337, 0, "%s", "targetEnt");
    if (visBone)
        AimTarget_GetTagPos(targetEnt, visBone, targetEyePos);
    else
        AimTarget_GetTargetCenter(targetEnt, targetEyePos);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\aim_assist\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    LODWORD(endPos[3]) = cgArray;
    playerEyePos[0] = flt_9D8718[0];
    playerEyePos[1] = flt_9D8718[1];
    playerEyePos[2] = flt_9D8718[2];
    CG_TraceCapsule(
        &trace,
        playerEyePos,
        (float *)vec3_origin,
        (float *)vec3_origin,
        targetEyePos,
        unk_9D5650,
        (int)&loc_803003);
    if (trace.fraction != 1.0 && Trace_GetEntityHitId(&trace) != targetEnt->nextState.number)
    {
        if ((clientActive_t *)targetEnt->nextState.solid != (clientActive_t *)((char *)&clients[0].parseClients[238].attachTagIndex[4]
            + 3))
        {
            //Profile_EndInternal(0);
            return 0;
        }
        Vec3Lerp(playerEyePos, targetEyePos, trace.fraction, endPos);
        if (!CM_TransformedPointContents(
            endPos,
            targetEnt->nextState.index.brushmodel,
            targetEnt->pose.origin,
            targetEnt->pose.angles))
        {
        LABEL_12:
            //Profile_EndInternal(0);
            return 0;
        }
    }
    visibility = FX_GetClientVisibility(localClientNum, playerEyePos, targetEyePos);
    if (visibility <= 0.00009999999747378752)
        goto LABEL_12;
    //Profile_EndInternal(0);
    return 1;
}

void __cdecl AimTarget_GetTargetCenter(const centity_s *targetEnt, float *center)
{
    float mins[3]; // [esp+8h] [ebp-18h] BYREF
    float maxs[3]; // [esp+14h] [ebp-Ch] BYREF

    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 237, 0, "%s", "targetEnt");
    if (!center)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 238, 0, "%s", "center");
    AimTarget_GetTargetBounds(targetEnt, mins, maxs);
    Vec3Add(mins, maxs, center);
    Vec3Scale(center, 0.5, center);
    Vec3Add(targetEnt->pose.origin, center, center);
}

void __cdecl Vec3Scale(const float *v, float scale, float *result)
{
    *result = scale * *v;
    result[1] = scale * v[1];
    result[2] = scale * v[2];
}

void __cdecl AimTarget_CreateTarget(int localClientNum, const centity_s *targetEnt, AimTarget *target)
{
    float scale; // [esp+8h] [ebp-70h]
    float diff[8]; // [esp+30h] [ebp-48h] BYREF
    const cg_s *cgameGlob; // [esp+50h] [ebp-28h]
    const snapshot_s *nextSnap; // [esp+54h] [ebp-24h]
    const snapshot_s *snap; // [esp+58h] [ebp-20h]
    float currentPos[3]; // [esp+5Ch] [ebp-1Ch] BYREF
    float nextPos[3]; // [esp+68h] [ebp-10h] BYREF
    float deltaTime; // [esp+74h] [ebp-4h]

    //Profile_Begin(61);
    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 393, 0, "%s", "targetEnt");
    if (!target)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 394, 0, "%s", "target");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\aim_assist\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    target->entIndex = targetEnt->nextState.number;
    Vec3Sub(targetEnt->pose.origin, cgameGlob->predictedPlayerState.origin, diff);
    target->worldDistSqr = Vec3LengthSq(diff);
    AimTarget_GetTargetBounds(targetEnt, target->mins, target->maxs);
    snap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;
    deltaTime = (double)(nextSnap->serverTime - snap->serverTime) * 0.001000000047497451;
    if (deltaTime <= 0.0)
    {
        target->velocity[0] = 0.0;
        target->velocity[1] = 0.0;
        target->velocity[2] = 0.0;
    }
    else
    {
        BG_EvaluateTrajectory(&targetEnt->currentState.pos, snap->serverTime, currentPos);
        BG_EvaluateTrajectory(&targetEnt->nextState.lerp.pos, nextSnap->serverTime, nextPos);
        Vec3Sub(nextPos, currentPos, target->velocity);
        scale = 1.0 / deltaTime;
        Vec3Scale(target->velocity, scale, target->velocity);
    }
    AimTarget_AddTargetToList(localClientNum, target);
    //Profile_EndInternal(0);
}

double __cdecl Vec3LengthSq(const float *v)
{
    return (float)(*v * *v + v[1] * v[1] + v[2] * v[2]);
}

void __cdecl AimTarget_AddTargetToList(int localClientNum, const AimTarget *target)
{
    int targetIndex; // [esp+8h] [ebp-14h]
    int low; // [esp+Ch] [ebp-10h]
    AimTargetGlob *atGlob; // [esp+14h] [ebp-8h]
    int high; // [esp+18h] [ebp-4h]

    if (!target)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 125, 0, "%s", "target");
    atGlob = &atGlobArray[localClientNum];
    for (targetIndex = 0; targetIndex < atGlob->targetCount; ++targetIndex)
    {
        if (target->entIndex == atGlob->targets[targetIndex].entIndex)
            MyAssertHandler(
                ".\\aim_assist\\aim_target_mp.cpp",
                132,
                0,
                "%s",
                "target->entIndex != atGlob->targets[targetIndex].entIndex");
    }
    low = 0;
    high = atGlob->targetCount;
    while (low < high)
    {
        if (AimTarget_CompareTargets(target, &atGlob->targets[(high + low) / 2]) <= 0)
            low = (high + low) / 2 + 1;
        else
            high = (high + low) / 2;
    }
    if (low < 64)
    {
        if (atGlob->targetCount == 64)
            --atGlob->targetCount;
        memmove(
            (unsigned __int8 *)&atGlob->targets[low + 1],
            (unsigned __int8 *)&atGlob->targets[low],
            44 * (atGlob->targetCount - low));
        memcpy(&atGlob->targets[low], target, sizeof(atGlob->targets[low]));
        ++atGlob->targetCount;
    }
}

int __cdecl AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB)
{
    if (!targetA)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 102, 0, "%s", "targetA");
    if (!targetB)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 103, 0, "%s", "targetB");
    if (targetB->worldDistSqr > (double)targetA->worldDistSqr)
        return 1;
    if (targetB->worldDistSqr >= (double)targetA->worldDistSqr)
        return 0;
    return -1;
}

bool __cdecl AimTarget_PlayerInValidState(const playerState_s *ps)
{
    bool result; // al

    if (!ps)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 430, 0, "%s", "ps");
    if ((ps->otherFlags & 2) != 0)
        return 0;
    switch (ps->pm_type)
    {
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
    case 8:
        result = 0;
        break;
    default:
        result = 1;
        break;
    }
    return result;
}

void __cdecl AimTarget_UpdateClientTargets(int localClientNum)
{
    atGlobArray[localClientNum].clientTargetCount = atGlobArray[localClientNum].targetCount;
    memcpy(
        (unsigned __int8 *)atGlobArray[localClientNum].clientTargets,
        (unsigned __int8 *)&atGlobArray[localClientNum],
        44 * atGlobArray[localClientNum].targetCount);
}

void __cdecl AimTarget_GetClientTargetList(int localClientNum, AimTarget **targetList, int *targetCount)
{
    if (!targetList)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 527, 0, "%s", "targetList");
    if (!targetCount)
        MyAssertHandler(".\\aim_assist\\aim_target_mp.cpp", 528, 0, "%s", "targetCount");
    *targetList = atGlobArray[localClientNum].clientTargets;
    *targetCount = atGlobArray[localClientNum].clientTargetCount;
}

