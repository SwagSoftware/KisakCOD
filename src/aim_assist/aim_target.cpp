#include "aim_target.h"
#include <qcommon/cmd.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif
#include <game/g_local.h>
#include <script/scr_const.h>
#include <game/bullet.h>
#include <game/savememory.h>
#include <game/actor.h>
#include <game/g_main.h>
#include <universal/profile.h>

AimTargetGlob atGlob;

const dvar_s *aim_target_sentient_radius;

void __cdecl TRACK_aim_target()
{
    track_static_alloc_internal(&atGlob, 5640, "atGlob", 10);
}

const dvar_s *__cdecl AimTarget_RegisterDvars(int a1, unsigned __int16 a2, const char *a3)
{
    const dvar_s *result; // r3

    result = Dvar_RegisterFloat("aim_target_sentient_radius", 10.0, 0.0, 128.0, a2, a3);
    aim_target_sentient_radius = result;
    return result;
}

void __cdecl AimTarget_Init()
{
    memset(&atGlob, 0, sizeof(atGlob));
    aim_target_sentient_radius = Dvar_RegisterFloat("aim_target_sentient_radius", 10.0, 0.0, 128.0, 0x82/*?*/, "The radius used to calculate target bounds for a sentient(actor or player)");
    Cbuf_InsertText(0, "exec devgui_aimassist\n");
}

void __cdecl AimTarget_ClearTargetList()
{
    atGlob.targetCount = 0;
}

int __cdecl AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB)
{
    double worldDistSqr; // fp0
    double v5; // fp13
    int result; // r3

    if (!targetA)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 97, 0, "%s", "targetA");
    if (!targetB)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 98, 0, "%s", "targetB");
    worldDistSqr = targetA->worldDistSqr;
    v5 = targetB->worldDistSqr;
    if (worldDistSqr < v5)
        return 1;
    result = -1;
    if (worldDistSqr <= v5)
        return 0;
    return result;
}

void __cdecl AimTarget_AddTargetToList(const AimTarget *target)
{
    int v2; // r29
    int targetCount; // r11
    AimTargetGlob *v4; // r31
    int v5; // r29
    int v6; // r27
    int v7; // r31
    float *p_entIndex; // r28

    if (!target)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 119, 0, "%s", "target");
    v2 = 0;
    targetCount = atGlob.targetCount;
    if (atGlob.targetCount > 0)
    {
        v4 = &atGlob;
        do
        {
            if (target->entIndex == v4->targets[0].entIndex)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp",
                    124,
                    0,
                    "%s",
                    "target->entIndex != atGlob.targets[targetIndex].entIndex");
                targetCount = atGlob.targetCount;
            }
            ++v2;
            v4 = (AimTargetGlob *)((char *)v4 + 44);
        } while (v2 < targetCount);
    }
    v5 = 0;
    v6 = targetCount;
    if (targetCount <= 0)
        goto LABEL_19;
    do
    {
        v7 = (v6 + v5) / 2;
        p_entIndex = (float *)&atGlob.targets[v7].entIndex;
        if (!target)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 97, 0, "%s", "targetA");
        if (!p_entIndex)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 98, 0, "%s", "targetB");
        if (target->worldDistSqr >= (double)p_entIndex[1])
            v5 = v7 + 1;
        else
            v6 = (v6 + v5) / 2;
    } while (v5 < v6);
    if (v5 < 64)
    {
        targetCount = atGlob.targetCount;
    LABEL_19:
        if (targetCount == 64)
        {
            targetCount = 63;
            atGlob.targetCount = 63;
        }
        memmove(&atGlob.targets[v5 + 1], &atGlob.targets[v5], 44 * (targetCount - v5));
        memcpy(&atGlob.targets[v5], target, sizeof(atGlob.targets[v5]));
        ++atGlob.targetCount;
    }
}

void __cdecl AimTarget_GetTargetBounds(const gentity_s *targetEnt, float *mins, float *maxs)
{
    const dvar_s *v6; // r11
    double v7; // fp13
    float v8[16]; // [sp+50h] [-40h] BYREF

    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 160, 0, "%s", "targetEnt");
    if (!mins)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 161, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 162, 0, "%s", "maxs");
    if (targetEnt->s.eType == 14)
    {
        G_DObjGetWorldTagPos_CheckTagExists(targetEnt, scr_const.aim_highest_bone, v8);
        v6 = aim_target_sentient_radius;
        *mins = -aim_target_sentient_radius->current.value;
        v7 = -v6->current.value;
        mins[2] = 0.0;
        mins[1] = v7;
        *maxs = v6->current.value;
        maxs[1] = v6->current.value;
        maxs[2] = v8[2] - targetEnt->r.currentOrigin[2];
    }
    else
    {
        if (targetEnt->s.solid != 0xFFFFFF)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp",
                178,
                0,
                "%s",
                "targetEnt->s.solid == SOLID_BMODEL");
        CM_ModelBounds(targetEnt->s.index.item, mins, maxs);
    }
}

float __cdecl AimTarget_GetTargetRadius(const gentity_s *targetEnt)
{
    double value; // fp1
    float v4[4]; // [sp+50h] [-30h] BYREF
    float v5[4]; // [sp+60h] [-20h] BYREF

    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 194, 0, "%s", "targetEnt");
    if (targetEnt->s.eType == 14)
    {
        value = aim_target_sentient_radius->current.value;
    }
    else
    {
        AimTarget_GetTargetBounds(targetEnt, v5, v4);
        value = RadiusFromBounds(v5, v4);
    }
    return *((float *)&value + 1);
}

void __cdecl AimTarget_GetTargetCenter(const gentity_s *targetEnt, float *center)
{
    double v4; // fp12
    double v5; // fp11
    double v6; // fp13
    double v7; // fp12
    float v8[4]; // [sp+50h] [-50h] BYREF
    float v9[16]; // [sp+60h] [-40h] BYREF

    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 214, 0, "%s", "targetEnt");
    if (!center)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 215, 0, "%s", "center");
    AimTarget_GetTargetBounds(targetEnt, v9, v8);
    v4 = (float)(v8[1] + v9[1]);
    v5 = (float)(v8[2] + v9[2]);
    v6 = (float)((float)(v8[0] + v9[0]) * (float)0.5);
    *center = (float)(v8[0] + v9[0]) * (float)0.5;
    v7 = (float)((float)v4 * (float)0.5);
    center[1] = v7;
    center[2] = (float)v5 * (float)0.5;
    *center = targetEnt->r.currentOrigin[0] + (float)v6;
    center[1] = targetEnt->r.currentOrigin[1] + (float)v7;
    center[2] = targetEnt->r.currentOrigin[2] + (float)((float)v5 * (float)0.5);
}

int __cdecl AimTarget_IsTargetValid(const gentity_s *targetEnt)
{
    gentity_s *Player; // r28
    double v4; // fp31
    double v5; // fp30
    double v6; // fp29
    double TargetRadius; // fp1
    float v8[4]; // [sp+50h] [-50h] BYREF

    //Profile_Begin(59);
    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 238, 0, "%s", "targetEnt");
    Player = G_GetPlayer();
    if (!Player)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 241, 0, "%s", "playerEnt");
    if (Player->s.number == targetEnt->s.number)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp",
            242,
            0,
            "%s",
            "playerEnt->s.number != targetEnt->s.number");
    if (!targetEnt->r.inuse || !targetEnt->r.linked)
        goto LABEL_16;
    if (targetEnt->s.eType == 14)
    {
        iassert(targetEnt->actor);
        iassert(targetEnt->actor->sentient);
        
        if (targetEnt->health <= 0 || targetEnt->actor->sentient->eTeam != TEAM_AXIS)
            goto LABEL_16;
    }
    else if ((targetEnt->s.lerp.eFlags & 0x800) == 0 || targetEnt->s.solid != 0xFFFFFF)
    {
        goto LABEL_16;
    }
    v4 = (float)(targetEnt->r.currentOrigin[0] - Player->r.currentOrigin[0]);
    v5 = (float)(targetEnt->r.currentOrigin[1] - Player->r.currentOrigin[1]);
    v6 = (float)(targetEnt->r.currentOrigin[2] - Player->r.currentOrigin[2]);
    G_GetPlayerViewDirection(Player, v8, 0, 0);
    TargetRadius = AimTarget_GetTargetRadius(targetEnt);
    if ((float)((float)((float)(v8[1] * (float)v5) + (float)((float)(v8[2] * (float)v6) + (float)(v8[0] * (float)v4)))
        + (float)TargetRadius) < 0.0)
    {
    LABEL_16:
        //Profile_EndInternal(0);
        return 0;
    }
    //Profile_EndInternal(0);
    return 1;
}

int __cdecl AimTarget_IsTargetVisible(const gentity_s *targetEnt, unsigned int visBone)
{
    gentity_s *Player; // r28
    unsigned int index; // r4
    long double *v6; // r3
    double Visibility; // fp1
    float v9[4]; // [sp+50h] [-90h] BYREF
    float v10; // [sp+60h] [-80h] BYREF
    float v11; // [sp+64h] [-7Ch]
    float v12; // [sp+68h] [-78h]
    float v13[4]; // [sp+70h] [-70h] BYREF
    trace_t v14[2]; // [sp+80h] [-60h] BYREF

    //Profile_Begin(60);
    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 314, 0, "%s", "targetEnt");
    Player = G_GetPlayer();
    if (!Player)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 317, 0, "%s", "playerEnt");
    if (visBone)
        G_DObjGetWorldTagPos_CheckTagExists(targetEnt, visBone, v9);
    else
        AimTarget_GetTargetCenter(targetEnt, v9);
    if (!Player->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 324, 0, "%s", "playerEnt->client");
    G_GetPlayerViewOrigin(&Player->client->ps, &v10);
    if (targetEnt->s.solid == 0xFFFFFF)
    {
        G_LocationalTrace(v14, &v10, v9, Player->s.number, 8396803, bulletPriorityMap);
        if (v14[0].fraction != 1.0 && Trace_GetEntityHitId(v14) != targetEnt->s.number)
        {
            index = targetEnt->s.index.item;
            v13[0] = (float)((float)(v9[0] - v10) * v14[0].fraction) + v10;
            v13[1] = (float)((float)(v9[1] - v11) * v14[0].fraction) + v11;
            v13[2] = (float)((float)(v9[2] - v12) * v14[0].fraction) + v12;
            v6 = (long double *)CM_TransformedPointContents(
                v13,
                index,
                targetEnt->r.currentOrigin,
                targetEnt->r.currentAngles);
            if (!v6)
                goto LABEL_14;
        }
    }
    else if (!G_LocationalTracePassed(&v10, v9, Player->s.number, targetEnt->s.number, 8396803, bulletPriorityMap))
    {
        //Profile_EndInternal(0);
        return 0;
    }
    Visibility = SV_FX_GetVisibility(&v10, v9);
    v6 = 0;
    if (Visibility <= 0.000099999997)
    {
    LABEL_14:
        //Profile_EndInternal(v6);
        return 0;
    }
    //Profile_EndInternal(0);
    return 1;
}

void __cdecl AimTarget_CreateTarget(const gentity_s *targetEnt, AimTarget *target)
{
    gentity_s *Player; // r27
    double v5; // fp0
    double v6; // fp13
    double v7; // fp12
    actor_s *actor; // r11
    double v9; // fp0
    const trajectory_t *p_pos; // r30
    double v11; // fp0
    float v12[4]; // [sp+50h] [-50h] BYREF
    float v13[16]; // [sp+60h] [-40h] BYREF

    //Profile_Begin(61);
    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 372, 0, "%s", "targetEnt");
    if (!target)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 373, 0, "%s", "target");
    Player = G_GetPlayer();
    if (!Player)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 376, 0, "%s", "playerEnt");
    target->entIndex = targetEnt->s.number;
    v5 = (float)(targetEnt->r.currentOrigin[1] - Player->r.currentOrigin[1]);
    v6 = (float)(targetEnt->r.currentOrigin[2] - Player->r.currentOrigin[2]);
    v7 = (float)(targetEnt->r.currentOrigin[0] - Player->r.currentOrigin[0]);
    target->worldDistSqr = (float)((float)v7 * (float)v7)
        + (float)((float)((float)v6 * (float)v6) + (float)((float)v5 * (float)v5));
    AimTarget_GetTargetBounds(targetEnt, target->mins, target->maxs);
    if (targetEnt->s.eType == 14)
    {
        if (!targetEnt->actor)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 385, 0, "%s", "targetEnt->actor");
        actor = targetEnt->actor;
        target->velocity[0] = actor->Physics.vVelocity[0];
        target->velocity[1] = actor->Physics.vVelocity[1];
        v9 = actor->Physics.vVelocity[2];
    }
    else
    {
        p_pos = &targetEnt->s.lerp.pos;
        BG_EvaluateTrajectory(p_pos, level.time, v12);
        BG_EvaluateTrajectory(p_pos, level.time + 50, v13);
        v11 = (float)(v13[0] - v12[0]);
        target->velocity[1] = v13[1] - v12[1];
        target->velocity[0] = v11;
        target->velocity[2] = v13[2] - v12[2];
        target->velocity[0] = (float)v11 * (float)20.0;
        target->velocity[1] = target->velocity[1] * (float)20.0;
        v9 = (float)(target->velocity[2] * (float)20.0);
    }
    target->velocity[2] = v9;
    AimTarget_AddTargetToList(target);
    //Profile_EndInternal(0);
}

bool __cdecl AimTarget_PlayerInValidState(const playerState_s *ps)
{
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 410, 0, "%s", "ps");
    return (unsigned int)(ps->pm_type - 2) > 4;
}

void __cdecl AimTarget_ProcessEntity(gentity_s *ent)
{
    gentity_s *playerEnt; // r29
    gclient_s *client; // r29
    unsigned int aim_vis_bone; // r4
    int v5; // r11
    char v6; // r3
    gentityFlags_t flags; // r11
    gentityFlags_t v8; // r11
    char IsTargetVisible; // r3
    AimTarget v10[2]; // [sp+50h] [-60h] BYREF

    PROF_SCOPED("AimTarget_ProcessEntity");

    iassert(ent);
    iassert(ent->r.inuse);
    playerEnt = G_GetPlayer();
    iassert(playerEnt);
    iassert(playerEnt->client);
    iassert(ent->s.number != playerEnt->s.number);
    client = playerEnt->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 410, 0, "%s", "ps");
    if ((unsigned int)(client->ps.pm_type - 2) <= 4)
        goto LABEL_35;
    if (!ent->r.linked || !(unsigned __int8)AimTarget_IsTargetValid(ent))
    {
        ent->flags &= ~(FL_VISIBLE_AIMTARGET);
        goto LABEL_35;
    }
    if (ent->s.eType == ET_ACTOR)
    {
        aim_vis_bone = scr_const.aim_vis_bone;
    }
    else
    {
        iassert(ent->s.lerp.eFlags & EF_AIM_ASSIST);
        iassert(ent->s.solid == SOLID_BMODEL);
        aim_vis_bone = 0;
    }
    v5 = ent->s.number - level.time / 50;
    if ((ent->flags & FL_VISIBLE_AIMTARGET) == 0)
    {
        if (v5 % 2)
            goto LABEL_32;
        IsTargetVisible = AimTarget_IsTargetVisible(ent, aim_vis_bone);
        flags = ent->flags;
        if (IsTargetVisible)
        {
            v8 = flags | FL_VISIBLE_AIMTARGET;
            goto LABEL_31;
        }
    LABEL_30:
        v8 = flags & ~(FL_VISIBLE_AIMTARGET);
        goto LABEL_31;
    }
    if (v5 % 8)
        goto LABEL_32;
    v6 = AimTarget_IsTargetVisible(ent, aim_vis_bone);
    flags = ent->flags;
    if (!v6)
        goto LABEL_30;
    v8 = flags | FL_VISIBLE_AIMTARGET;
LABEL_31:
    ent->flags = v8;
LABEL_32:
    if ((ent->flags & FL_VISIBLE_AIMTARGET) != 0)
    {
        AimTarget_CreateTarget(ent, v10);
        return;
    }
LABEL_35:
    ; // LWSS: this is only here so the goto works
}

void __cdecl AimTarget_UpdateClientTargets()
{
    atGlob.clientTargetCount = atGlob.targetCount;
    memcpy(atGlob.clientTargets, &atGlob, 44 * atGlob.targetCount);
}

void __cdecl AimTarget_GetClientTargetList(int32_t localClientNum, AimTarget **targetList, int *targetCount)
{
    iassert(targetList);
    iassert(targetCount);

    *targetList = atGlob.clientTargets;
    *targetCount = atGlob.clientTargetCount;
}

int __cdecl AimTarget_GetBestTarget(const float *start, const float *viewDir)
{
    int result; // r3
    double v5; // fp4
    AimTargetGlob *v6; // r8
    int targetCount; // r6
    float *currentOrigin; // r11
    double v9; // fp13
    double v10; // fp12
    double v13; // fp11

    if (!start)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 546, 0, "%s", "start");
    if (!viewDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 547, 0, "%s", "viewDir");
    if (fabs((float)((float)((float)(viewDir[2] * viewDir[2])
        + (float)((float)(*viewDir * *viewDir) + (float)(viewDir[1] * viewDir[1])))
        - (float)1.0)) >= 0.0020000001)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp",
            548,
            0,
            "%s",
            "Vec3IsNormalized( viewDir )");
    result = ENTITYNUM_NONE;
    v5 = 0.9659;
    v6 = &atGlob;
    if (atGlob.targetCount > 0)
    {
        targetCount = atGlob.targetCount;
        do
        {
            currentOrigin = g_entities[v6->targets[0].entIndex].r.currentOrigin;
            v9 = (float)(currentOrigin[2] - start[2]);
            v10 = (float)(currentOrigin[1] - start[1]);

            //_FP3 = -sqrtf((float)((float)((float)v10 * (float)v10)
            //    + (float)((float)((float)v9 * (float)v9)
            //        + (float)((float)(*currentOrigin - *start) * (float)(*currentOrigin - *start)))));
            //__asm { fsel      f11, f3, f31, f11 }
            //v13 = (float)((float)1.0 / (float)_FP11);

            float dx = currentOrigin[0] - start[0];
            float temp = v10 * v10 + v9 * v9 + dx * dx;

            float _FP3 = -sqrtf(temp > 0.0f ? temp : 0.0f);
            float _FP11 = -_FP3;
            v13 = 1.0f / _FP11;

            if ((float)((float)(*viewDir * (float)((float)v13 * (float)(*currentOrigin - *start)))
                + (float)((float)(viewDir[2] * (float)((float)(currentOrigin[2] - start[2]) * (float)v13))
                    + (float)(viewDir[1] * (float)((float)v13 * (float)(currentOrigin[1] - start[1]))))) > v5)
            {
                result = v6->targets[0].entIndex;
                v5 = (float)((float)(*viewDir * (float)((float)v13 * (float)(*currentOrigin - *start)))
                    + (float)((float)(viewDir[2] * (float)((float)(currentOrigin[2] - start[2]) * (float)v13))
                        + (float)(viewDir[1] * (float)((float)v13 * (float)(currentOrigin[1] - start[1])))));
            }
            --targetCount;
            v6 = (AimTargetGlob *)((char *)v6 + 44);
        } while (targetCount);
    }
    return result;
}

void __cdecl AimTarget_WriteSaveGame(SaveGame *save)
{
    int v2; // r31
    AimTargetGlob *v3; // r30

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 581, 0, "%s", "save");
    SaveMemory_SaveWrite(&atGlob.targetCount, 4, save);
    v2 = 0;
    if (atGlob.targetCount > 0)
    {
        v3 = &atGlob;
        do
        {
            SaveMemory_SaveWrite(v3, 44, save);
            ++v2;
            v3 = (AimTargetGlob *)((char *)v3 + 44);
        } while (v2 < atGlob.targetCount);
    }
}

void __cdecl AimTarget_ReadSaveGame(SaveGame *save)
{
    int v2; // r31
    AimTargetGlob *v3; // r30

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\aim_assist\\aim_target.cpp", 597, 0, "%s", "save");
    SaveMemory_LoadRead(&atGlob.targetCount, 4, save);
    v2 = 0;
    if (atGlob.targetCount > 0)
    {
        v3 = &atGlob;
        do
        {
            SaveMemory_LoadRead(v3, 44, save);
            ++v2;
            v3 = (AimTargetGlob *)((char *)v3 + 44);
        } while (v2 < atGlob.targetCount);
    }
}

