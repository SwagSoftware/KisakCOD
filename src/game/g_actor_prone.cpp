#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_actor_prone.h"

actor_prone_info_s *__cdecl G_GetActorProneInfo(actor_s *actor)
{
    if ((unsigned int)(actor - level.actors) >= 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp",
            14,
            0,
            "%s",
            "(iIndex >= 0) && (iIndex < MAX_ACTORS)");
    return &actor->ProneInfo;
}

actor_prone_info_s *__cdecl G_GetActorProneInfoFromEntNum(int iEntNum)
{
    gentity_s *v2; // r31
    actor_s *actor; // r3
    int ActorCorpseIndex; // r3
    int number; // r11
    int v6; // r31

    if (iEntNum >= 2176)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp", 24, 0, "%s", "iEntNum < MAX_GENTITIES");
    v2 = &g_entities[iEntNum];
    actor = v2->actor;
    if (actor)
        return G_GetActorProneInfo(actor);
    if (v2->s.eType != 16)
        return 0;
    ActorCorpseIndex = G_GetActorCorpseIndex(v2);
    number = v2->s.number;
    v6 = ActorCorpseIndex;
    if (g_scr_data.actorCorpseInfo[ActorCorpseIndex].entnum != number)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp",
            33,
            0,
            "%s",
            "g_scr_data.actorCorpseInfo[iCorpseIndex].entnum == ent->s.number");
    return &g_scr_data.actorCorpseInfo[v6].proneInfo;
}

actor_prone_info_s *__cdecl G_BypassForCG_GetClientActorProneInfo(int iEntNum)
{
    unsigned int v1; // r30
    gentity_s *v2; // r31
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29
    const char *v6; // r3
    const char *v7; // r3

    v1 = level.specialIndex[iEntNum];
    if (v1 >= 0x30)
    {
        v2 = &level.gentities[iEntNum];
        v3 = v2->r.currentOrigin[2];
        v4 = v2->r.currentOrigin[1];
        v5 = v2->r.currentOrigin[0];
        v6 = SL_ConvertToString(v2->classname);
        v7 = va(
            "entnum: %d, inuse: %d, eType: %d, classname: %s, actorIndex: %d, origin: %.2f %.2f %.2f",
            v2->s.number,
            v2->r.inuse,
            v2->s.eType,
            v6,
            HIDWORD(v5),
            v5,
            v4,
            v3);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp",
            49,
            0,
            "%s\n\t%s",
            "actorIndex < NUM_PRONE_INFO",
            v7);
    }
    return &level.cgData_actorProneInfo[v1];
}

void __cdecl G_InitActorProneInfo(actor_s *actor)
{
    actor_prone_info_s *p_ProneInfo; // r11
    int v3; // ctr

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp", 56, 0, "%s", "actor");
    p_ProneInfo = &actor->ProneInfo;
    v3 = 6;
    do
    {
        *(unsigned int *)&p_ProneInfo->bCorpseOrientation = 0;
        p_ProneInfo = (actor_prone_info_s *)((char *)p_ProneInfo + 4);
        --v3;
    } while (v3);
}

void __cdecl G_ActorEnterProne(actor_s *actor, unsigned int iTransTime)
{
    actor_prone_info_s *p_ProneInfo; // r29
    float *v5; // r5
    int iProneTrans; // r11
    double ActorProneFraction; // fp1
    bool prone; // r9
    gentity_s *ent; // r7
    float *currentOrigin; // r4
    int time; // r8
    proneCheckType_t v12; // [sp+8h] [-B8h]
    unsigned int v13[16]; // [sp+80h] [-40h] BYREF

    p_ProneInfo = &actor->ProneInfo;
    if (BG_ActorIsProne(&actor->ProneInfo, level.time))
    {
        iProneTrans = actor->ProneInfo.iProneTrans;
        if (iProneTrans && iTransTime != iProneTrans)
        {
            ActorProneFraction = BG_GetActorProneFraction(p_ProneInfo, level.time);
            if (ActorProneFraction < 1.0)
            {
                prone = actor->ProneInfo.prone;
                v13[1] = iTransTime;
                v13[0] = (int)(float)((float)__SPAIR64__(v13, iTransTime) * (float)ActorProneFraction);
                actor->ProneInfo.iProneTime = level.time - v13[0];
                if (!prone)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp",
                        75,
                        0,
                        "%s",
                        "actor->ProneInfo.prone");
            }
            actor->ProneInfo.iProneTrans = iTransTime;
        }
    }
    else
    {
        ent = actor->ent;
        currentOrigin = actor->ent->r.currentOrigin;
        p_ProneInfo->bCorpseOrientation = 0;
        time = level.time;
        actor->ProneInfo.prone = 1;
        actor->ProneInfo.iProneTrans = iTransTime;
        actor->Physics.prone = 1;
        actor->ProneInfo.iProneTime = time;
        actor->bProneOK = BG_CheckProneValid(
            ent->s.number,
            currentOrigin,
            15.0,
            48.0,
            ent->r.currentAngles[1],
            v5,
            (float *)".0f, 1.0f]\n\t%g not in [%g, %g]",
            (bool)ent,
            (_BYTE)actor + 96,
            (_BYTE)actor + 100,
            0,
            v12,
            50.0);
    }
    if (!BG_ActorGoalIsProne(p_ProneInfo))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp",
            92,
            0,
            "%s",
            "BG_ActorGoalIsProne( &actor->ProneInfo )");
}

void __cdecl G_ActorExitProne(actor_s *actor, unsigned int iTransTime)
{
    actor_prone_info_s *p_ProneInfo; // r29
    int iProneTrans; // r11
    int time; // r11
    unsigned int v7[16]; // [sp+50h] [-40h] BYREF

    p_ProneInfo = &actor->ProneInfo;
    if (BG_ActorIsProne(&actor->ProneInfo, level.time))
    {
        iProneTrans = actor->ProneInfo.iProneTrans;
        if (!iProneTrans || iTransTime == iProneTrans)
        {
            time = level.time;
        }
        else
        {
            v7[1] = iTransTime;
            v7[0] = (int)(float)((float)__SPAIR64__(v7, iTransTime) * BG_GetActorProneFraction(p_ProneInfo, level.time));
            time = level.time - v7[0];
        }
        actor->ProneInfo.iProneTime = time;
        if (!actor->ProneInfo.prone)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp", 113, 0, "%s", "actor->ProneInfo.prone");
        actor->ProneInfo.iProneTrans = -iTransTime;
    }
    if (BG_ActorGoalIsProne(p_ProneInfo))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_actor_prone.cpp",
            117,
            0,
            "%s",
            "!BG_ActorGoalIsProne( &actor->ProneInfo )");
}

