#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_events.h"

float __cdecl Actor_EventDefaultRadiusSqrd(ai_event_t eType)
{
    ai_event_t v1; // r31
    const dvar_s *v2; // r31
    double v3; // fp1

    v1 = eType;
    if (!g_ai_event_info[eType].defaultDistDvar)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            65,
            0,
            "%s",
            "g_ai_event_info[eType].defaultDistDvar");
    v2 = *g_ai_event_info[v1].defaultDistDvar;
    if (!v2)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 68, 0, "%s", "dvar");
    v3 = (float)(v2->current.value * v2->current.value);
    return *((float *)&v3 + 1);
}

float __cdecl Actor_EventDefaultHeightDiff(ai_event_t eType)
{
    double defaultHeight; // fp1

    defaultHeight = g_ai_event_info[eType].defaultHeight;
    return *((float *)&defaultHeight + 1);
}

const char *__cdecl Actor_NameForEvent(ai_event_t eType)
{
    return g_ai_event_info[eType].name;
}

void __cdecl Actor_DumpEvents(actor_s *self, ai_event_t event, gentity_s *originator)
{
    int number; // r5
    int v7; // r7

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 90, 0, "%s", "self");
    number = self->ent->s.number;
    if (g_dumpAIEvents->current.integer == number)
    {
        if (originator)
            v7 = originator->s.number;
        else
            v7 = -1;
        Com_Printf(
            18,
            "%d ^3 %s^7:  from entity^5 %d ^7at time^5 %d\n",
            number,
            g_ai_event_info[event].name,
            v7,
            level.time);
    }
}

int __cdecl Actor_EventForName(const char *name)
{
    const $51E54BD14BE3E2EF9DA2A8BD94E6B80E *v2; // r31
    int v3; // r29
    unsigned int v4; // r30

    v2 = g_ai_event_info;
    v3 = 0;
    v4 = 0;
    while (!v2->name || I_stricmp(name, v2->name))
    {
        v4 += 12;
        ++v3;
        ++v2;
        if (v4 >= 0x114)
            return 0;
    }
    return v3;
}

// attributes: thunk
void __cdecl Actor_GetPerfectInfo(actor_s *self, sentient_s *pOther)
{
    Actor_UpdateLastKnownPos(self, pOther);
}

void __cdecl Actor_WasAttackedBy(actor_s *self, sentient_s *pOther)
{
    sentient_s *sentients; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 149, 0, "%s", "self");
    if (!pOther)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 150, 0, "%s", "pOther");
    sentients = level.sentients;
    if (pOther < level.sentients || pOther >= &level.sentients[33])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            151,
            0,
            "%s",
            "pOther >= level.sentients && pOther < level.sentients + MAX_SENTIENTS");
        sentients = level.sentients;
    }
    if (pOther != &sentients[pOther - sentients])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            152,
            0,
            "%s",
            "pOther == level.sentients + (pOther - level.sentients)");
        sentients = level.sentients;
    }
    self->sentientInfo[pOther - sentients].iLastAttackMeTime = level.time;
}

void __cdecl Actor_EventFootstep(actor_s *self, sentient_s *originator, const float *vOrigin)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 488, 0, "%s", "self");
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 489, 0, "%s", "originator");
    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 490, 0, "%s", "vOrigin");
    Actor_UpdateLastKnownPos(self, originator);
}

void __cdecl Actor_EventNewEnemy(actor_s *self, sentient_s *originator)
{
    sentient_s *sentient; // r4

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 505, 0, "%s", "self");
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 506, 0, "%s", "originator");
    if (!originator->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 507, 0, "%s", "originator->ent");
    if (!EntHandle::isDefined(&originator->targetEnt))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            508,
            0,
            "%s",
            "originator->targetEnt.isDefined()");
    if (!EntHandle::ent(&originator->targetEnt)->sentient)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            509,
            0,
            "%s",
            "originator->targetEnt.ent()->sentient");
    sentient = EntHandle::ent(&originator->targetEnt)->sentient;
    if (originator->ent->actor)
        SentientInfo_Copy(self, originator->ent->actor, sentient - level.sentients);
    else
        Actor_UpdateLastKnownPos(self, sentient);
}

void __cdecl Actor_EventPain(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 534, 0, "%s", "self");
    if (!pCasualty)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 535, 0, "%s", "pCasualty");
    if (!pAttacker)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 536, 0, "%s", "pAttacker");
    Actor_WasAttackedBy(self, pAttacker);
    Actor_UpdateLastKnownPos(self, pAttacker);
}

void __cdecl Actor_EventDeath(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 552, 0, "%s", "self");
    if (!pCasualty)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 553, 0, "%s", "pCasualty");
    if (!pAttacker)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 554, 0, "%s", "pAttacker");
    Actor_UpdateLastKnownPos(self, pAttacker);
}

void __cdecl Actor_EventExplosion(actor_s *self, gentity_s *originator, const float *vOrigin)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 567, 0, "%s", "self");
    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 568, 0, "%s", "vOrigin");
    if (originator)
        Scr_AddEntity(originator);
    else
        Scr_AddUndefined();
    Scr_AddVector(vOrigin);
    Scr_Notify(self->ent, scr_const.explode, 2u);
}

void __cdecl Actor_EventGrenadePing(actor_s *self, gentity_s *originator, const float *vOrigin)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 589, 0, "%s", "self");
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 590, 0, "%s", "originator");
    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 591, 0, "%s", "vOrigin");
    Actor_GrenadePing(self, originator);
}

void __cdecl Actor_EventGunshot(actor_s *self, sentient_s *originator, const float *vOrigin)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 606, 0, "%s", "self");
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 607, 0, "%s", "originator");
    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 608, 0, "%s", "vOrigin");
    Actor_UpdateLastKnownPos(self, originator);
}

void __cdecl Actor_EventBullet(
    actor_s *self,
    gentity_s *originator,
    const float *vStart,
    const float *vEnd,
    const float *vClosest,
    double fDistSqrd,
    double fRadiusSqrd,
    PARM_SUPPRESSION suppression,
    int a9,
    int a10)
{
    sentient_s *sentient; // r4

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 623, 0, "%s", "self");
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 624, 0, "%s", "originator");
    if (!vStart)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 625, 0, "%s", "vStart");
    if (!vEnd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 626, 0, "%s", "vEnd");
    if (!vClosest)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 627, 0, "%s", "vClosest");
    if (fDistSqrd < 0.0 || fDistSqrd > fRadiusSqrd)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            628,
            0,
            "%s",
            "fDistSqrd >= 0 && fDistSqrd <= fRadiusSqrd");
    sentient = originator->sentient;
    if (sentient)
    {
        Actor_WasAttackedBy(self, sentient);
        Actor_UpdateLastKnownPos(self, originator->sentient);
        if (!a10)
            Actor_AddSuppressionLine(self, originator->sentient, vStart, vEnd);
    }
}

void __cdecl Actor_ReceivePointEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *vOrigin,
    double fDistSqrd,
    double fRadiusSqrd)
{
    sentient_s *sentient; // r9
    sentient_s *v13; // r4
    sentient_s *v14; // r4

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 650, 0, "%s", "self");
    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 651, 0, "%s", "vOrigin");
    if (fDistSqrd < 0.0 || fDistSqrd > fRadiusSqrd)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            652,
            0,
            "%s",
            "fDistSqrd >= 0 && fDistSqrd <= fRadiusSqrd");
    if (eType <= AI_EV_FIRST_POINT_EVENT || eType >= AI_EV_LAST_POINT_EVENT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            653,
            0,
            "%s",
            "eType > AI_EV_FIRST_POINT_EVENT && eType < AI_EV_LAST_POINT_EVENT");
    switch (eType)
    {
    case AI_EV_FOOTSTEP:
    case AI_EV_FOOTSTEP_LITE:
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 659, 0, "%s", "originator");
        if (!originator->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 660, 0, "%s", "originator->sentient");
        if (Actor_CaresAboutInfo(self, originator->sentient))
        {
            if (!Actor_IsUsingTurret(self)
                || !turret_IsFiring(self->pTurret)
                || (sentient = originator->sentient) == 0
                || level.time - self->sentientInfo[sentient - level.sentients].lastKnownPosTime < 3000)
            {
                Actor_EventFootstep(self, originator->sentient, vOrigin);
                Actor_DumpEvents(self, eType, originator);
            }
        }
        break;
    case AI_EV_NEW_ENEMY:
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 673, 0, "%s", "originator");
        if (!originator->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 674, 0, "%s", "originator->sentient");
        Actor_EventNewEnemy(self, originator->sentient);
        Actor_DumpEvents(self, eType, originator);
        break;
    case AI_EV_PAIN:
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 680, 0, "%s", "originator");
        if (!originator->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 681, 0, "%s", "originator->sentient");
        if (!originator->sentient->lastAttacker)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
                682,
                0,
                "%s",
                "originator->sentient->lastAttacker");
        v13 = originator->sentient->lastAttacker->sentient;
        if (v13 && Actor_CaresAboutInfo(self, v13))
        {
            Actor_EventPain(self, originator->sentient, originator->sentient->lastAttacker->sentient);
            Actor_DumpEvents(self, eType, originator);
        }
        break;
    case AI_EV_DEATH:
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 692, 0, "%s", "originator");
        if (!originator->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 693, 0, "%s", "originator->sentient");
        if (!originator->sentient->lastAttacker)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
                694,
                0,
                "%s",
                "originator->sentient->lastAttacker");
        v14 = originator->sentient->lastAttacker->sentient;
        if (v14 && Actor_CaresAboutInfo(self, v14))
        {
            Actor_EventDeath(self, originator->sentient, originator->sentient->lastAttacker->sentient);
            Actor_DumpEvents(self, eType, originator);
        }
        break;
    case AI_EV_EXPLOSION:
        Actor_EventExplosion(self, originator, vOrigin);
        Actor_DumpEvents(self, eType, originator);
        break;
    case AI_EV_GRENADE_PING:
        goto LABEL_50;
    case AI_EV_PROJECTILE_PING:
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 712, 0, "%s", "originator");
        if (!EntHandle::isDefined(&originator->parent) || EntHandle::ent(&originator->parent) != self->ent)
        {
        LABEL_50:
            Actor_EventGrenadePing(self, originator, vOrigin);
            Actor_DumpEvents(self, eType, originator);
        }
        break;
    case AI_EV_GUNSHOT:
    case AI_EV_SILENCED_SHOT:
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 721, 0, "%s", "originator");
        if (!originator->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 722, 0, "%s", "originator->sentient");
        if (originator != self->ent && Actor_CaresAboutInfo(self, originator->sentient))
        {
            Actor_EventGunshot(self, originator->sentient, vOrigin);
            Actor_DumpEvents(self, eType, originator);
        }
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
                734,
                0,
                "Actor_ReceivePointEvent: unhandled case\n");
        break;
    }
}

void __cdecl Actor_ReceiveLineEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *vStart,
    const float *vEnd,
    const float *vClosest,
    double fDistSqrd,
    double fRadiusSqrd,
    int a9)
{
    int v17; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 751, 0, "%s", "self");
    if (!vStart)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 752, 0, "%s", "vStart");
    if (!vEnd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 753, 0, "%s", "vEnd");
    if (!vClosest)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 754, 0, "%s", "vClosest");
    if (fDistSqrd < 0.0 || fDistSqrd > fRadiusSqrd)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            755,
            0,
            "%s",
            "fDistSqrd >= 0 && fDistSqrd <= fRadiusSqrd");
    if (eType <= AI_EV_FIRST_LINE_EVENT || eType >= AI_EV_LAST_LINE_EVENT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            756,
            0,
            "%s",
            "eType > AI_EV_FIRST_LINE_EVENT && eType < AI_EV_LAST_LINE_EVENT");
    if (eType != AI_EV_BULLET)
    {
        if (eType != AI_EV_PROJECTILE_IMPACT)
        {
            if (!alwaysfails)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
                    773,
                    0,
                    "Actor_ReceiveLineEvent: unhandled case\n");
            return;
        }
        v17 = 1;
    LABEL_24:
        Actor_EventBullet(
            self,
            originator,
            vStart,
            vEnd,
            vClosest,
            fDistSqrd,
            fRadiusSqrd,
            (PARM_SUPPRESSION)vClosest,
            a9,
            v17);
        Actor_DumpEvents(self, eType, originator);
        return;
    }
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 761, 0, "%s", "originator");
    if (originator != self->ent)
    {
        v17 = 0;
        goto LABEL_24;
    }
}

void __cdecl Actor_ReceiveArcEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *origin,
    double distSqrd,
    double radiusSqrd,
    double angle0,
    double angle1,
    double halfHeight)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 790, 0, "%s", "self");
    if (!origin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 791, 0, "%s", "origin");
    if (eType == AI_EV_BADPLACE_ARC)
    {
        Actor_Badplace_Ping(self);
        Actor_DumpEvents(self, AI_EV_BADPLACE_ARC, originator);
    }
    else
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            792,
            0,
            "%s",
            "eType > AI_EV_FIRST_ARC_EVENT && eType < AI_EV_LAST_ARC_EVENT");
        if (!alwaysfails)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
                802,
                0,
                "Actor_ReceiveArcEvent: unhandled case\n");
    }
}

void __cdecl Actor_ReceiveVolumeEvent(actor_s *self, gentity_s *originator, ai_event_t eType)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 819, 0, "%s", "self");
    if (eType == AI_EV_BADPLACE_VOLUME)
    {
        Actor_Badplace_Ping(self);
        Actor_DumpEvents(self, AI_EV_BADPLACE_VOLUME, originator);
    }
    else
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            820,
            0,
            "%s",
            "eType > AI_EV_FIRST_VOLUME_EVENT && eType < AI_EV_LAST_VOLUME_EVENT");
        if (!alwaysfails)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
                829,
                0,
                "Actor_ReceiveVolumeEvent: unhandled case\n");
    }
}

void __cdecl Actor_BroadcastPointEvent(
    gentity_s *originator,
    int eType,
    int teamFlags,
    const float *vOrigin,
    double fRadiusSqrd)
{
    int v6; // r27
    sentient_s *sentient; // r11
    team_t v11; // r3
    actor_s *i; // r31
    double v13; // fp13
    double v14; // fp0
    double v15; // fp12
    double v16; // fp1
    int j; // r31
    gentity_s *Entity; // r3
    double v19; // fp13
    double v20; // fp0
    double v21; // fp12

    v6 = teamFlags;
    if (teamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            186,
            0,
            "%s",
            "teamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    if (eType <= 1 || eType >= 12)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            187,
            0,
            "%s",
            "eType > AI_EV_FIRST_POINT_EVENT && eType < AI_EV_LAST_POINT_EVENT");
    if (v6)
        goto LABEL_12;
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 194, 0, "%s", "originator");
    sentient = originator->sentient;
    if (sentient)
    {
        v11 = Sentient_EnemyTeam(sentient->eTeam);
        if (v11)
        {
            v6 = 1 << v11;
        LABEL_12:
            if (fRadiusSqrd == 0.0)
                fRadiusSqrd = Actor_EventDefaultRadiusSqrd((ai_event_t)eType);
            for (i = Actor_FirstActor(v6); i; i = Actor_NextActor(i, v6))
            {
                v13 = (float)(vOrigin[2] - i->ent->r.currentOrigin[2]);
                v14 = (float)(*vOrigin - i->ent->r.currentOrigin[0]);
                v15 = (float)(vOrigin[1] - i->ent->r.currentOrigin[1]);
                v16 = (float)((float)((float)v15 * (float)v15)
                    + (float)((float)((float)v14 * (float)v14) + (float)((float)v13 * (float)v13)));
                if (v16 <= fRadiusSqrd)
                    Actor_ReceivePointEvent(i, originator, (ai_event_t)eType, vOrigin, v16, fRadiusSqrd);
            }
            for (j = Actor_EventListener_First(eType, v6); j >= 0; j = Actor_EventListener_Next(j, eType, v6))
            {
                Entity = Actor_EventListener_GetEntity(j);
                v19 = (float)(vOrigin[2] - Entity->r.currentOrigin[2]);
                v20 = (float)(*vOrigin - Entity->r.currentOrigin[0]);
                v21 = (float)(vOrigin[1] - Entity->r.currentOrigin[1]);
                if ((float)((float)((float)v21 * (float)v21)
                    + (float)((float)((float)v20 * (float)v20) + (float)((float)v19 * (float)v19))) <= fRadiusSqrd)
                    Actor_EventListener_NotifyToListener(Entity, originator, (ai_event_t)eType, vOrigin);
            }
        }
    }
}

void __cdecl Actor_BroadcastLineEvent(
    gentity_s *originator,
    int eType,
    int teamFlags,
    const float *vStart,
    const float *vEnd,
    double fRadiusSqrd)
{
    int v7; // r25
    sentient_s *sentient; // r11
    team_t v13; // r3
    double v14; // fp30
    double v15; // fp29
    double v16; // fp28
    double defaultHeight; // fp23
    double v18; // fp26
    actor_s *i; // r31
    int v20; // r9
    double v21; // fp31
    double v22; // fp13
    double v23; // fp12
    double v24; // fp11
    double v25; // fp0
    double v26; // fp13
    double v27; // fp1
    int j; // r29
    gentity_s *Entity; // r31
    double v30; // fp31
    double v31; // fp13
    double v32; // fp12
    double v33; // fp11
    double v34; // fp0
    double v35; // fp13
    float v36; // [sp+50h] [-C0h] BYREF
    float v37; // [sp+54h] [-BCh]
    float v38; // [sp+58h] [-B8h]

    v7 = teamFlags;
    if (teamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            249,
            0,
            "%s",
            "teamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    if (eType <= 13 || eType >= 16)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            250,
            0,
            "%s",
            "eType > AI_EV_FIRST_LINE_EVENT && eType < AI_EV_LAST_LINE_EVENT");
    if (!v7)
    {
        sentient = originator->sentient;
        if (!sentient)
            return;
        v13 = Sentient_EnemyTeam(sentient->eTeam);
        if (v13 == TEAM_FREE)
            return;
        v7 = 1 << v13;
    }
    if (fRadiusSqrd == 0.0)
        fRadiusSqrd = Actor_EventDefaultRadiusSqrd((ai_event_t)eType);
    v14 = (float)(vEnd[1] - vStart[1]);
    v15 = (float)(vEnd[2] - vStart[2]);
    v16 = (float)(*vEnd - *vStart);
    defaultHeight = g_ai_event_info[eType].defaultHeight;
    v18 = (float)((float)((float)(*vEnd - *vStart) * (float)(*vEnd - *vStart))
        + (float)((float)((float)v15 * (float)v15) + (float)((float)v14 * (float)v14)));
    for (i = Actor_FirstActor(v7); i; i = Actor_NextActor(i, v7))
    {
        v21 = (float)((float)((float)(i->ent->r.currentOrigin[0] - *vStart) * (float)v16)
            + (float)((float)((float)(i->ent->r.currentOrigin[1] - vStart[1]) * (float)v14)
                + (float)((float)v15 * (float)(i->ent->r.currentOrigin[2] - vStart[2]))));
        if (v21 >= 0.0)
        {
            if (v21 < v18)
            {
                if (v18 == 0.0)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 297, 0, "%s", "fLineLenSqrd");
                v22 = (float)((float)((float)((float)v21 / (float)v18) * (float)v16) + *vStart);
                v23 = (float)((float)((float)v14 * (float)((float)v21 / (float)v18)) + vStart[1]);
                v24 = (float)((float)((float)v15 * (float)((float)v21 / (float)v18)) + vStart[2]);
            }
            else
            {
                v22 = *vEnd;
                v23 = vEnd[1];
                v24 = vEnd[2];
            }
            v37 = v23;
            v36 = v22;
            v38 = v24;
            v25 = (float)(i->ent->r.currentOrigin[0] - (float)v22);
            v26 = (float)(i->ent->r.currentOrigin[1] - (float)v23);
            v27 = (float)((float)((float)v26 * (float)v26) + (float)((float)v25 * (float)v25));
            if (v27 <= fRadiusSqrd
                && __fabs((float)((float)((float)(72.0 + 0.0) * (float)0.5) + (float)(i->ent->r.currentOrigin[2] - (float)v24))) < __fabs(defaultHeight))
            {
                Actor_ReceiveLineEvent(i, originator, (ai_event_t)eType, vStart, vEnd, &v36, v27, fRadiusSqrd, v20);
            }
        }
    }
    for (j = Actor_EventListener_First(eType, v7); j >= 0; j = Actor_EventListener_Next(j, eType, v7))
    {
        Entity = Actor_EventListener_GetEntity(j);
        v30 = (float)((float)((float)(Entity->r.currentOrigin[0] - *vStart) * (float)v16)
            + (float)((float)((float)(Entity->r.currentOrigin[1] - vStart[1]) * (float)v14)
                + (float)((float)v15 * (float)(Entity->r.currentOrigin[2] - vStart[2]))));
        if (v30 >= 0.0)
        {
            if (v30 < v18)
            {
                if (v18 == 0.0)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 332, 0, "%s", "fLineLenSqrd");
                v31 = (float)((float)((float)((float)v30 / (float)v18) * (float)v16) + *vStart);
                v32 = (float)((float)((float)v14 * (float)((float)v30 / (float)v18)) + vStart[1]);
                v33 = (float)((float)((float)v15 * (float)((float)v30 / (float)v18)) + vStart[2]);
            }
            else
            {
                v31 = *vEnd;
                v32 = vEnd[1];
                v33 = vEnd[2];
            }
            v37 = v32;
            v36 = v31;
            v38 = v33;
            v34 = (float)(Entity->r.currentOrigin[0] - (float)v31);
            v35 = (float)(Entity->r.currentOrigin[1] - (float)v32);
            if ((float)((float)((float)v35 * (float)v35) + (float)((float)v34 * (float)v34)) <= fRadiusSqrd
                && __fabs((float)(Entity->r.currentOrigin[2] - (float)v33)) < __fabs(defaultHeight))
            {
                Actor_EventListener_NotifyToListener(Entity, originator, (ai_event_t)eType, &v36);
            }
        }
    }
}

void __cdecl Actor_BroadcastArcEvent(
    gentity_s *originator,
    __int32 eventType,
    int teamFlags,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight)
{
    int v9; // r27
    sentient_s *sentient; // r11
    team_t v17; // r3
    double v18; // fp28
    actor_s *i; // r31
    double v20; // fp0
    double v21; // fp13
    double v22; // fp12
    double v23; // fp0
    int j; // r31
    gentity_s *Entity; // r29
    const float *v26; // r4

    v9 = teamFlags;
    if (teamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            358,
            0,
            "%s",
            "teamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    if (eventType != 18)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            359,
            0,
            "%s",
            "eventType > AI_EV_FIRST_ARC_EVENT && eventType < AI_EV_LAST_ARC_EVENT");
    if (angle0 != angle1)
    {
        if (v9)
        {
        LABEL_12:
            v18 = (float)((float)radius * (float)radius);
            if (v18 == 0.0)
                v18 = Actor_EventDefaultRadiusSqrd((ai_event_t)eventType);
            for (i = Actor_FirstActor(v9); i; i = Actor_NextActor(i, v9))
            {
                if (Actor_IsInsideArc(i, origin, radius, angle0, angle1, halfHeight))
                {
                    v20 = (float)(i->ent->r.currentOrigin[0] - *origin);
                    v21 = (float)(i->ent->r.currentOrigin[2] - origin[2]);
                    v22 = (float)(i->ent->r.currentOrigin[1] - origin[1]);
                    v23 = __fsqrts((float)((float)((float)v22 * (float)v22)
                        + (float)((float)((float)v21 * (float)v21) + (float)((float)v20 * (float)v20))));
                    Actor_ReceiveArcEvent(
                        i,
                        originator,
                        (ai_event_t)eventType,
                        origin,
                        (float)((float)v23 * (float)v23),
                        v18,
                        angle0,
                        angle1,
                        halfHeight);
                }
            }
            for (j = Actor_EventListener_First(eventType, v9); j >= 0; j = Actor_EventListener_Next(j, eventType, v9))
            {
                Entity = Actor_EventListener_GetEntity(j);
                if (IsPosInsideArc(Entity->r.currentOrigin, 15.0, v26, radius, angle0, angle1, halfHeight))
                    Actor_EventListener_NotifyToListener(Entity, originator, (ai_event_t)eventType, origin);
            }
            return;
        }
        if (!originator)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 373, 0, "%s", "originator");
        sentient = originator->sentient;
        if (sentient)
        {
            v17 = Sentient_EnemyTeam(sentient->eTeam);
            if (v17)
            {
                v9 = 1 << v17;
                goto LABEL_12;
            }
        }
    }
}

void __cdecl Actor_BroadcastVolumeEvent(
    gentity_s *originator,
    __int32 eventType,
    int teamFlags,
    gentity_s *volumeEnt,
    double radius)
{
    int v6; // r28
    sentient_s *sentient; // r11
    team_t v11; // r3
    double v12; // fp31
    float *currentOrigin; // r31
    actor_s *i; // r30
    const float *v15; // r4
    double v16; // fp13
    double v17; // fp12
    int j; // r30
    gentity_s *Entity; // r3
    gentity_s *v20; // r29
    double v21; // fp13
    double v22; // fp0
    double v23; // fp12

    v6 = teamFlags;
    if (teamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            427,
            0,
            "%s",
            "teamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    if (eventType != 21)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp",
            428,
            0,
            "%s",
            "eventType > AI_EV_FIRST_VOLUME_EVENT && eventType < AI_EV_LAST_VOLUME_EVENT");
    if (v6)
        goto LABEL_11;
    if (!originator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 438, 0, "%s", "originator");
    sentient = originator->sentient;
    if (sentient)
    {
        v11 = Sentient_EnemyTeam(sentient->eTeam);
        if (v11)
        {
            v6 = 1 << v11;
        LABEL_11:
            v12 = (float)((float)radius * (float)radius);
            currentOrigin = volumeEnt->r.currentOrigin;
            for (i = Actor_FirstActor(v6); i; i = Actor_NextActor(i, v6))
            {
                v15 = i->ent->r.currentOrigin;
                v16 = (float)(i->ent->r.currentOrigin[2] - volumeEnt->r.currentOrigin[2]);
                v17 = (float)(i->ent->r.currentOrigin[1] - volumeEnt->r.currentOrigin[1]);
                if ((float)((float)((float)v17 * (float)v17)
                    + (float)((float)((float)(*v15 - *currentOrigin) * (float)(*v15 - *currentOrigin))
                        + (float)((float)v16 * (float)v16))) < v12
                    && SV_EntityContact(i->ent->r.currentOrigin, v15, volumeEnt))
                {
                    Actor_ReceiveVolumeEvent(i, originator, (ai_event_t)eventType);
                }
            }
            for (j = Actor_EventListener_First(eventType, v6); j >= 0; j = Actor_EventListener_Next(j, eventType, v6))
            {
                Entity = Actor_EventListener_GetEntity(j);
                v20 = Entity;
                v21 = (float)(Entity->r.currentOrigin[2] - volumeEnt->r.currentOrigin[2]);
                v22 = (float)(Entity->r.currentOrigin[0] - *currentOrigin);
                v23 = (float)(Entity->r.currentOrigin[1] - volumeEnt->r.currentOrigin[1]);
                if ((float)((float)((float)v23 * (float)v23)
                    + (float)((float)((float)v22 * (float)v22) + (float)((float)v21 * (float)v21))) < v12)
                {
                    if (SV_EntityContact(Entity->r.currentOrigin, Entity->r.currentOrigin, volumeEnt))
                        Actor_EventListener_NotifyToListener(v20, originator, (ai_event_t)eventType, volumeEnt->r.currentOrigin);
                }
            }
        }
    }
}

void __cdecl Actor_BroadcastTeamEvent(sentient_s *sentient, int eType)
{
    if (!sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 168, 0, "%s", "sentient");
    if (!sentient->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_events.cpp", 169, 0, "%s", "sentient->ent");
    Actor_BroadcastPointEvent(sentient->ent, eType, 1 << sentient->eTeam, sentient->ent->r.currentOrigin, 0.0);
}

