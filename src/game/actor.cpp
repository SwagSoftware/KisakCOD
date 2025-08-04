#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include <xanim/xanim.h>
#include <bgame/bg_local.h>
#include <script/scr_const.h>
#include <bgame/bg_public.h>
#include "actor_event_listeners.h"
#include "g_scr_main.h"
#include "g_main.h"
#include "actor_state.h"
#include "g_local.h"
#include <script/scr_vm.h>
#include "sentient.h"
#include "actor_events.h"
#include "actor_senses.h"
#include "actor_threat.h"
#include <cgame/cg_pose.h>
#include <xanim/dobj_utils.h>
#include "actor_team_move.h"
#include <server/sv_game.h>
#include <qcommon/threads.h>
#include "actor_cover.h"
#include "g_save.h"
#include "actor_spawner.h"
#include "actor_aim.h"
#include "actor_orientation.h"
#include "actor_turret.h"
#include "actor_grenade.h"
#include "actor_lookat.h"
#include "actor_corpse.h"
#include "g_actor_prone.h"

const char *animModeNames[10] =
{
  "none",
  "(code)",
  "(pos deltas)",
  "angle_deltas",
  "gravity",
  "(noclip)",
  "nogravity",
  "zonly_physics",
  "(nophysics)",
  "point_relative"
};

const unsigned __int16 *g_AISpeciesNames[2] =
{ 
    &scr_const.human,
    &scr_const.dog
};

const char *g_entinfoAITextNames[6] = { "all", "brief", "combat", "movement", "state", NULL };
const char *ai_orient_mode_text[7] =
{
  "invalid",
  "dont_change",
  "motion",
  "enemy",
  "enemy_or_motion",
  "enemy_or_motion_sidestep",
  "goal"
};

float g_pathAttemptGoalPos[3] = { 0.0f, 0.0f, 0.0f };
AnimScriptList *g_animScriptTable[2] = { NULL, NULL };

int __cdecl Path_IsValidClaimNode(const pathnode_t *node)
{
    iassert(node);

    return node->constant.spawnflags & 0x8000;
}

int __cdecl Path_IsCoverNode(const pathnode_t *node)
{
    iassert(node);

    return (1 << node->constant.type) & 0x41FFC;
}

team_t __cdecl Sentient_EnemyTeam(unsigned int eTeam)
{
    unsigned int v3[8]; // [sp+50h] [-30h] BYREF

    v3[1] = 2;
    v3[0] = 0;
    v3[3] = 0;
    v3[4] = 0;
    v3[2] = 1;

    iassert(eTeam >= 0 && eTeam < TEAM_NUM_TEAMS);

    return (team_t)v3[eTeam];
}

void __cdecl TRACK_actor()
{
    track_static_alloc_internal((void *)actorMins, 12, "actorMins", 5);
    track_static_alloc_internal((void *)actorMaxs, 12, "actorMaxs", 5);
    track_static_alloc_internal((void *)g_actorAssumedSpeed, 8, "g_actorAssumedSpeed", 5);
    track_static_alloc_internal((void *)meleeAttackOffsets, 32, "meleeAttackOffsets", 5);
    track_static_alloc_internal(g_entinfoAITextNames, 24, "g_entinfoAITextNames", 0);
}

void __cdecl VisCache_Copy(vis_cache_t *pDstCache, const vis_cache_t *pSrcCache)
{
    iassert(pDstCache);
    iassert(pSrcCache);

    *(unsigned int *)&pDstCache->bVisible = *(unsigned int *)&pSrcCache->bVisible;
    pDstCache->iLastUpdateTime = pSrcCache->iLastUpdateTime;
    pDstCache->iLastVisTime = pSrcCache->iLastVisTime;
}

void __cdecl VisCache_Update(vis_cache_t *pCache, bool bVisible)
{
    iassert(pCache);

    pCache->bVisible = bVisible;
    pCache->iLastUpdateTime = level.time;
    if (bVisible)
        pCache->iLastVisTime = level.time;
}

void __cdecl SentientInfo_Clear(sentient_info_t *pInfo)
{
    sentient_info_t *v2; // r11
    int v3; // ctr

    iassert(pInfo);

    v2 = pInfo;
    v3 = 10;
    do
    {
        *(unsigned int *)&v2->VisCache.bVisible = 0;
        v2 = (sentient_info_t *)((char *)v2 + 4);
        --v3;
    } while (v3);
}

void __cdecl SentientInfo_ForceCopy(sentient_info_t *pTo, const sentient_info_t *pFrom)
{
    pTo->iLastAttackMeTime = 0;
    pTo->attackTime = 0;
    pTo->lastKnownPosTime = pFrom->lastKnownPosTime;
    pTo->vLastKnownPos[0] = pFrom->vLastKnownPos[0];
    pTo->vLastKnownPos[1] = pFrom->vLastKnownPos[1];
    pTo->vLastKnownPos[2] = pFrom->vLastKnownPos[2];
    pTo->pLastKnownNode = pFrom->pLastKnownNode;
}

int __cdecl Actor_droptofloor(gentity_s *ent)
{
    float *currentOrigin; // r31
    double v2; // fp12
    double v3; // fp0
    int result; // r3
    double fraction; // fp0
    float v6; // [sp+50h] [-70h] BYREF
    float v7; // [sp+54h] [-6Ch]
    float v8; // [sp+58h] [-68h]
    float v9[4]; // [sp+60h] [-60h] BYREF
    float v10[4]; // [sp+70h] [-50h] BYREF
    trace_t v11; // [sp+80h] [-40h] BYREF

    currentOrigin = ent->r.currentOrigin;
    v2 = (float)(ent->r.currentOrigin[2] + (float)1.0);
    v9[2] = 0.0;
    v10[2] = 30.0;
    v8 = ent->r.currentOrigin[2] - (float)128.0;
    v6 = ent->r.currentOrigin[0];
    v3 = ent->r.currentOrigin[1];
    ent->r.currentOrigin[2] = v2;
    v7 = v3;
    v9[0] = -15.0;
    v9[1] = -15.0;
    v10[0] = 15.0;
    v10[1] = 15.0;
    G_TraceCapsule(&v11, ent->r.currentOrigin, v9, v10, &v6, 2175, 42074129);
    if (v11.startsolid)
        return 1;
    result = 0;
    fraction = v11.fraction;
    *currentOrigin = (float)((float)(v6 - *currentOrigin) * v11.fraction) + *currentOrigin;
    currentOrigin[1] = (float)((float)(v7 - currentOrigin[1]) * (float)fraction) + currentOrigin[1];
    currentOrigin[2] = (float)((float)(v8 - currentOrigin[2]) * (float)fraction) + currentOrigin[2];
    return result;
}

int __cdecl Actor_IsDeletable(actor_s *actor)
{
    int number; // r10
    int result; // r3

    if ((actor->ent->spawnflags & 4) != 0)
        return 0;
    number = actor->ent->s.number;
    result = 1;
    if (number == level.currentEntityThink)
        return 0;
    return result;
}

void __cdecl G_InitActors()
{
    int i; // r11

    for (i = 0; i < 32; ++i)
        level.actors[i].inuse = 0;

    Actor_EventListener_Init();
}

unsigned int __cdecl G_GetActorIndex(actor_s *actor)
{
    actor_s *actors; // r11
    unsigned int v3; // r11

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 650, 0, "%s", "actor");
    actors = level.actors;
    if (actor - level.actors < 0)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 651, 0, "%s", "actor - level.actors >= 0");
        actors = level.actors;
    }
    if (actor - actors >= 32)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 652, 0, "%s", "actor - level.actors < MAX_ACTORS");
        actors = level.actors;
    }
    v3 = (int)((unsigned __int64)(2248490037LL * ((char *)actor - (char *)actors)) >> 32) >> 12;
    return v3 + (v3 >> 31);
}

XAnimTree_s *__cdecl G_GetActorAnimTree(actor_s *actor)
{
    return g_scr_data.actorXAnimTrees[G_GetActorIndex(actor)];
}

XAnimTree_s *__cdecl G_AllocAnimClientTree()
{
    int v0; // r9
    int actorFreeClientTree; // r11

    v0 = 0;
    actorFreeClientTree = g_scr_data.actorFreeClientTree;
    while (1)
    {
        g_scr_data.actorFreeClientTree = ++actorFreeClientTree;
        if (actorFreeClientTree == 64)
        {
            actorFreeClientTree = 0;
            g_scr_data.actorFreeClientTree = 0;
        }
        if (!g_scr_data.actorXAnimClientTreesInuse[actorFreeClientTree])
            break;
        if ((unsigned int)++v0 >= 0x40)
        {
            Com_Printf(18, "G_AllocAnimClientTree: failed allocation\n");
            return 0;
        }
    }
    g_scr_data.actorXAnimClientTreesInuse[actorFreeClientTree] = 1;
    return g_scr_data.actorXAnimClientTrees[g_scr_data.actorFreeClientTree];
}

void __cdecl G_FreeAnimClientTree(XAnimTree_s *tree)
{
    unsigned int v2; // r27
    XAnimTree_s **actorXAnimClientTrees; // r26

    XAnimClearTree(tree);
    v2 = 0;
    if (g_scr_data.actorXAnimClientTrees[0] != tree)
    {
        actorXAnimClientTrees = g_scr_data.actorXAnimClientTrees;
        do
        {
            ++v2;
            ++actorXAnimClientTrees;
            if (v2 >= 0x40)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    699,
                    0,
                    "%s",
                    "actorFreeClientTree < ARRAY_COUNT( g_scr_data.actorXAnimClientTrees )");
        } while (*actorXAnimClientTrees != tree);
    }
    g_scr_data.actorXAnimClientTreesInuse[v2] = 0;
}

void __cdecl Actor_SetDefaults(actor_s *actor)
{
    Actor_SetDefaultState(actor);
    actor->species = AI_SPECIES_HUMAN;
    actor->fovDot = 0.0099999998;
    actor->talkToSpecies = -1;
    actor->fMaxSightDistSqrd = 67108864.0;
    actor->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    actor->fWalkDist = 128.0;
    actor->CodeOrient.eMode = AI_ORIENT_DONT_CHANGE;
    actor->fInterval = 96.0;
    actor->ScriptOrient.eMode = AI_ORIENT_INVALID;
    actor->grenadeAwareness = 0.33000001;
    actor->iPacifistWait = 20000;
    actor->badPlaceAwareness = 0.75;
    actor->ignoreSuppression = 0;
    actor->suppressionWait = 2000;
    actor->suppressionDuration = 5000;
    actor->suppressionStartTime = 0;
    actor->iFollowMin = -5;
    actor->iFollowMax = -1;
    actor->goodShootPosValid = 0;
}

void __cdecl Actor_FinishSpawning(actor_s *self)
{
    gentity_s *ent; // r28
    gentity_s *v3; // r29
    const char *v4; // r29
    int *DataForFile; // r29
    unsigned __int16 v6; // r3

    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 747, 0, "%s", "self->ent");
    if (self->ent->actor != self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 748, 0, "%s", "self->ent->actor == self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 749, 0, "%s", "self->sentient");
    if (self->ent->sentient != self->sentient)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            750,
            0,
            "%s",
            "self->ent->sentient == self->sentient");
    if (self->sentient->ent != self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 751, 0, "%s", "self->sentient->ent == self->ent");
    ent = self->ent;
    v3 = G_Find(0, 284, scr_const.player);
    if (!v3)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 757, 0, "%s", "player");
    if (!v3->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 758, 0, "%s", "player->sentient");
    v4 = SL_ConvertToString(ent->classname);
    if (strncmp(v4, "actor_", 6u))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            762,
            0,
            "%s",
            "!strncmp( classname, ACTOR_CLASSNAME_PREFIX, ACTOR_CLASSNAME_PREFIX_LEN )");
    DataForFile = (int *)Hunk_FindDataForFile(0, v4 + 6);
    if (!DataForFile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 765, 0, "%s", "typeScript");
    if (!*DataForFile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 766, 0, "%s", "typeScript->main");
    v6 = Scr_ExecEntThread(ent, *DataForFile, 0);
    Scr_FreeThread(v6);
}

void __cdecl Actor_InitAnimScript(actor_s *self)
{
    gentity_s *ent; // r27
    unsigned __int16 v3; // r3
    unsigned int stateLevel; // r8
    ai_state_t v5; // r8
    ai_state_t v6; // r8
    int time; // r11

    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 777, 0, "%s", "self->ent");
    if (self->ent->actor != self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 778, 0, "%s", "self->ent->actor == self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 779, 0, "%s", "self->sentient");
    if (self->ent->sentient != self->sentient)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            780,
            0,
            "%s",
            "self->ent->sentient == self->sentient");
    if (self->sentient->ent != self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 781, 0, "%s", "self->sentient->ent == self->ent");
    ent = self->ent;
    v3 = Scr_ExecEntThread(self->ent, g_animScriptTable[self->species]->init.func, 0);
    Scr_FreeThread(v3);
    stateLevel = self->stateLevel;
    if (stateLevel)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            790,
            0,
            "%s\n\t(self->stateLevel) = %i",
            "(self->stateLevel == 0)",
            stateLevel);
    v5 = self->eState[0];
    if (v5 <= AIS_INVALID || v5 >= AIS_COUNT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            791,
            0,
            "%s\n\t(self->eState[0]) = %i",
            "(self->eState[0] > AIS_INVALID && self->eState[0] < AIS_COUNT)",
            v5);
    v6 = self->eState[0];
    if (!AIFuncTable[self->species][v6].pfnStart)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            792,
            0,
            "%s\n\t(self->eState[0]) = %i",
            "(AIFuncTable[self->species][self->eState[0]].pfnStart)",
            v6);
    AIFuncTable[self->species][self->eState[0]].pfnStart(self, AIS_INVALID);
    time = level.time;
    ent->handler = 2;
    ent->nextthink = time;
}

actor_s *__cdecl Actor_FirstActor(int iTeamFlags)
{
    int v2; // r25
    int v3; // r31
    actor_s *actors; // r11

    if (iTeamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            882,
            0,
            "%s",
            "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    v2 = 0;
    v3 = 0;
    actors = level.actors;
    while (1)
    {
        if (actors[v3].inuse)
        {
            if (!actors[v3].sentient)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 891, 0, "%s", "level.actors[i].sentient");
                actors = level.actors;
            }
            if (((1 << actors[v3].sentient->eTeam) & iTeamFlags) != 0)
                break;
        }
        ++v3;
        ++v2;
        if (v3 >= 32)
            return 0;
    }
    return &actors[v2];
}

actor_s *__cdecl Actor_NextActor(actor_s *pPrevActor, int iTeamFlags)
{
    actor_s *actors; // r11
    int v5; // r30
    int v6; // r31

    if (iTeamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            907,
            0,
            "%s",
            "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    if (!pPrevActor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 909, 0, "%s", "pPrevActor");
    actors = level.actors;
    if (pPrevActor < level.actors || pPrevActor >= &level.actors[32])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            910,
            0,
            "%s",
            "pPrevActor >= level.actors && pPrevActor < level.actors + MAX_ACTORS");
        actors = level.actors;
    }
    if (pPrevActor != &actors[pPrevActor - actors])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            911,
            0,
            "%s",
            "pPrevActor == level.actors + (pPrevActor - level.actors)");
        actors = level.actors;
    }
    v5 = pPrevActor - actors + 1;
    if (v5 >= 32)
        return 0;
    v6 = v5;
    while (1)
    {
        if (actors[v6].inuse)
        {
            if (!actors[v6].sentient)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 920, 0, "%s", "level.actors[i].sentient");
                actors = level.actors;
            }
            if (((1 << actors[v6].sentient->eTeam) & iTeamFlags) != 0)
                break;
        }
        ++v6;
        ++v5;
        if (v6 >= 32)
            return 0;
    }
    return &actors[v5];
}

void __cdecl Actor_ClearArrivalPos(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1013, 0, "%s", "self");
    self->arrivalInfo.animscriptOverrideRunTo = 0;
    self->arrivalInfo.arrivalNotifyRequested = 0;
}

void __cdecl Actor_PreThink(actor_s *self)
{
    int flashBanged; // r10
    sentient_s *v3; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1069, 0, "%s", "self");
    if (self->preThinkTime != level.time)
    {
        flashBanged = self->flashBanged;
        self->preThinkTime = level.time;
        if (!flashBanged)
        {
            //if (SentientHandle::isDefined(&self->pFavoriteEnemy))
            if (self->pFavoriteEnemy.isDefined())
            {
                //v3 = SentientHandle::sentient(&self->pFavoriteEnemy);
                Actor_GetPerfectInfo(self, self->pFavoriteEnemy.sentient());
            }
            Actor_UpdateSight(self);
            Actor_UpdateThreat(self);
            Actor_UpdateLastEnemySightPos(self);
        }
    }
}

void __cdecl Actor_ValidateReacquireNodes(actor_s *self)
{
    int v2; // r30
    pathnode_t **pPotentialReacquireNode; // r31

    v2 = 0;
    if (self->iPotentialReacquireNodeCount > 0)
    {
        pPotentialReacquireNode = self->pPotentialReacquireNode;
        do
        {
            Path_ConvertNodeToIndex(*pPotentialReacquireNode);
            ++v2;
            ++pPotentialReacquireNode;
        } while (v2 < self->iPotentialReacquireNodeCount);
    }
}

void __cdecl Actor_Touch(gentity_s *self, gentity_s *other, int bTouched)
{
    actor_s *actor; // r31
    unsigned int stateLevel; // r7
    int v6; // r8
    ai_state_t v7; // r8

    actor = self->actor;
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1490, 0, "%s", "actor");
    if (!actor->inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1491, 0, "%s", "actor->inuse");
    stateLevel = actor->stateLevel;
    if (stateLevel >= 5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1500,
            0,
            "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
            stateLevel,
            5);
    v6 = actor->eState[actor->stateLevel];
    if (v6 <= 0 || v6 >= 11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1501,
            0,
            "%s\n\t(actor->eState[actor->stateLevel]) = %i",
            "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
            v6);
    v7 = actor->eState[actor->stateLevel];
    if (!AIFuncTable[actor->species][v7].pfnTouch)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1502,
            0,
            "%s\n\t(actor->eState[actor->stateLevel]) = %i",
            "(AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnTouch)",
            v7);
    AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnTouch(actor, other);
}

bool __cdecl Actor_InScriptedState(const actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1516, 0, "%s", "self");
    return Actor_IsStateOnStack(self, AIS_SCRIPTEDANIM) || Actor_IsStateOnStack(self, AIS_NEGOTIATION) != 0;
}

int __cdecl Actor_CheckDeathAllowed(actor_s *self, int damage)
{
    int health; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1532, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1533, 0, "%s", "self->ent");
    if (damage < self->ent->health || self->allowDeath || !Actor_InScriptedState(self))
        return 0;
    health = self->ent->health;
    self->delayedDeath = 1;
    return damage - health + 1;
}

void __cdecl Actor_Pain(
    gentity_s *self,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    float *vDir,
    __int32 hitLoc,
    unsigned int weaponIdx)
{
    actor_s *actor; // r31
    unsigned int stateLevel; // r7
    int v18; // r8
    double v19; // fp1
    double v20; // fp31
    long double v21; // fp2
    long double v22; // fp2
    unsigned __int16 HitLocationString; // r3
    WeaponDef *WeaponDef; // r30
    sentient_s *sentient; // r4

    actor = self->actor;
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1568, 0, "%s", "actor");
    if (!actor->inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1569, 0, "%s", "actor->inuse");
    if (!vDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1570, 0, "%s", "vDir");
    if ((COERCE_UNSIGNED_INT(*vDir) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vDir[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vDir[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1571,
            0,
            "%s",
            "!IS_NAN((vDir)[0]) && !IS_NAN((vDir)[1]) && !IS_NAN((vDir)[2])");
    }
    stateLevel = actor->stateLevel;
    if (stateLevel >= 5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1590,
            0,
            "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
            stateLevel,
            5);
    v18 = actor->eState[actor->stateLevel];
    if (v18 <= 0 || v18 >= 11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1591,
            0,
            "%s\n\t(actor->eState[actor->stateLevel]) = %i",
            "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
            v18);
    actor->iDamageTaken = iDamage;
    v19 = vectoyaw(vDir);
    v20 = (float)((float)((float)v19 - self->r.currentAngles[1]) * (float)0.0027777778);
    *(double *)&v21 = (float)((float)((float)((float)v19 - self->r.currentAngles[1]) * (float)0.0027777778) + (float)0.5);
    v22 = floor(v21);
    actor->iDamageYaw = (int)(float)((float)((float)v20 - (float)*(double *)&v22) * (float)360.0);
    actor->damageDir[0] = *vDir;
    actor->damageDir[1] = vDir[1];
    actor->damageDir[2] = vDir[2];
    HitLocationString = G_GetHitLocationString((hitLocation_t)hitLoc);
    Scr_SetString(&actor->damageHitLoc, HitLocationString);
    if (pAttacker)
    {
        WeaponDef = BG_GetWeaponDef(weaponIdx);
        if (!WeaponDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1605, 0, "%s", "weapDef");
        if (!WeaponDef->szInternalName)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1606, 0, "%s", "weapDef->szInternalName");
        Scr_SetStringFromCharString(&actor->damageWeapon, WeaponDef->szInternalName);
    }
    if (!AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnPain)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1611,
            0,
            "%s",
            "AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnPain");
    AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnPain(
        actor,
        pAttacker,
        iDamage,
        vPoint,
        iMod,
        vDir,
        (const hitLocation_t)hitLoc);
    self->sentient->lastAttacker = pAttacker;
    sentient = pAttacker->sentient;
    if (sentient)
        Actor_WasAttackedBy(actor, sentient);
    if (!actor->sentient->syncedMeleeEnt.isDefined() && actor->allowPain)
    {
        if (Actor_PushState(actor, AIS_PAIN))
            Actor_KillAnimScript(actor);
    }
}

void __cdecl Actor_Die(
    gentity_s *self,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    int iWeapon,
    float *vDir,
    hitLocation_t hitLoc)
{
    actor_s *actor; // r31
    int health; // r8
    unsigned int stateLevel; // r7
    int v17; // r8
    double v18; // fp1
    double v19; // fp31
    long double v20; // fp2
    long double v21; // fp2
    unsigned __int16 HitLocationString; // r3
    WeaponDef *WeaponDef; // r29

    actor = self->actor;
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1643, 0, "%s", "actor");
    if (!actor->inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1644, 0, "%s", "actor->inuse");
    if (!vDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1645, 0, "%s", "vDir");
    if ((COERCE_UNSIGNED_INT(*vDir) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vDir[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vDir[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1646,
            0,
            "%s",
            "!IS_NAN((vDir)[0]) && !IS_NAN((vDir)[1]) && !IS_NAN((vDir)[2])");
    }
    if (!actor->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1659, 0, "%s", "actor->ent");
    health = actor->ent->health;
    if (health > 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1660,
            0,
            "%s\n\t(actor->ent->health) = %i",
            "(actor->ent->health <= 0)",
            health);
    stateLevel = actor->stateLevel;
    if (stateLevel >= 5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1661,
            0,
            "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
            stateLevel,
            5);
    v17 = actor->eState[actor->stateLevel];
    if (v17 <= 0 || v17 >= 11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1662,
            0,
            "%s\n\t(actor->eState[actor->stateLevel]) = %i",
            "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
            v17);
    if (actor->eState[actor->stateLevel] != AIS_DEATH)
    {
        actor->iDamageTaken = iDamage;
        actor->damageDir[0] = *vDir;
        actor->damageDir[1] = vDir[1];
        actor->damageDir[2] = vDir[2];
        v18 = vectoyaw(vDir);
        v19 = (float)((float)((float)v18 - self->r.currentAngles[1]) * (float)0.0027777778);
        *(double *)&v20 = (float)((float)((float)((float)v18 - self->r.currentAngles[1]) * (float)0.0027777778) + (float)0.5);
        v21 = floor(v20);
        actor->iDamageYaw = (int)(float)((float)((float)v19 - (float)*(double *)&v21) * (float)360.0);
        HitLocationString = G_GetHitLocationString(hitLoc);
        Scr_SetString(&actor->damageHitLoc, HitLocationString);
        if (pInflictor)
        {
            WeaponDef = BG_GetWeaponDef(pInflictor->s.weapon);
            if (!WeaponDef)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1681, 0, "%s", "weapDef");
            if (!WeaponDef->szInternalName)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1682, 0, "%s", "weapDef->szInternalName");
            Scr_SetStringFromCharString(&actor->damageWeapon, WeaponDef->szInternalName);
        }
        Actor_ForceState(actor, AIS_DEATH);
        actor->Physics.bIsAlive = 0;
        self->sentient->lastAttacker = pAttacker;
        Scr_SetString(&self->targetname, 0);
        Actor_KillAnimScript(actor);
    }
}

bool __cdecl Actor_IsDying(const actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1712, 0, "%s", "self");
    return self->eState[self->stateLevel] == AIS_DEATH;
}

bool __cdecl usingCodeGoal(actor_s *actor)
{
    return actor->codeGoalSrc
        && (actor->scriptGoalEnt.isDefined()
            || actor->scriptGoal.pos[0] != actor->codeGoal.pos[0]
            || actor->scriptGoal.pos[1] != actor->codeGoal.pos[1]
            || actor->scriptGoal.pos[2] != actor->codeGoal.pos[2]
            || actor->scriptGoal.radius != actor->codeGoal.radius);
}

gentity_s *__cdecl Actor_GetTargetEntity(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2308, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2309, 0, "%s", "self->sentient");
    if (self->sentient->targetEnt.isDefined())
        return self->sentient->targetEnt.ent();
    else
        return 0;
}

sentient_s *__cdecl Actor_GetTargetSentient(actor_s *self)
{
    iassert(self);
    iassert(self->sentient);

    if (self->sentient->targetEnt.isDefined())
        return self->sentient->targetEnt.ent()->sentient;
    else
        return 0;
}

void __cdecl Actor_GetTargetPosition(actor_s *self, float *position)
{
    gentity_s *v4; // r3

    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());
    v4 = self->sentient->targetEnt.ent();
    *position = v4->r.currentOrigin[0];
    position[1] = v4->r.currentOrigin[1];
    position[2] = v4->r.currentOrigin[2];
}

void __cdecl Actor_GetTargetLookPosition(actor_s *self, float *position)
{
    gentity_s *TargetEntity; // r3

    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());

    TargetEntity = Actor_GetTargetEntity(self);
    if (TargetEntity->sentient)
        Sentient_GetEyePosition(TargetEntity->sentient, position);
    else
        G_EntityCentroid(TargetEntity, position);
}

void __cdecl Actor_InitMove(actor_s *self)
{
    gentity_s *ent; // r11
    int number; // r7
    gentity_s *v4; // r11
    gentity_s *v5; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2367, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2368, 0, "%s", "self->ent");
    memset(&self->Physics, 0, sizeof(self->Physics));
    self->Physics.vOrigin[0] = 0.0;
    self->Physics.vOrigin[1] = 0.0;
    self->Physics.vOrigin[2] = 0.0;
    ent = self->ent;
    number = self->ent->s.number;
    self->Physics.ePhysicsType = AIPHYS_BAD;
    self->Physics.iMsec = 50;
    self->Physics.bIsAlive = 1;
    self->Physics.iEntNum = number;
    if (ent->health <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2377, 0, "%s", "self->ent->health > 0");
    v4 = self->ent;
    self->Physics.iHitEntnum = 2175;
    self->Physics.iTraceMask = 42074129;
    self->Physics.groundEntNum = 2174;
    self->Physics.vMins[0] = v4->r.mins[0];
    self->Physics.vMins[1] = v4->r.mins[1];
    self->Physics.vMins[2] = v4->r.mins[2];
    v5 = self->ent;
    self->Physics.vMaxs[0] = self->ent->r.maxs[0];
    self->Physics.vMaxs[1] = v5->r.maxs[1];
    self->Physics.vMaxs[2] = v5->r.maxs[2];
}

bool __cdecl Actor_IsDodgeEntity(actor_s *self, int entnum)
{
    gentity_s *gentities; // r11
    gentity_s *v5; // r11
    bool result; // r3

    gentities = level.gentities;
    if ((level.gentities[2175].flags & 0x400) == 0)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            2393,
            0,
            "%s",
            "level.gentities[ENTITYNUM_NONE].flags & FL_OBSTACLE");
        gentities = level.gentities;
    }
    v5 = &gentities[entnum];
    if (v5->sentient)
        return level.time < self->iTeamMoveDodgeTime;
    if ((v5->flags & 0x400) != 0)
        return 0;
    result = 1;
    if (self->Path.iPathTime <= v5->iDisconnectTime)
        return 0;
    return result;
}

int __cdecl Actor_Physics_GetLeftOrRightDodge(actor_s *self, bool dodgeRight, double length)
{
    int flags; // r11
    int result; // r3
    double v8; // fp0
    double v9; // fp12
    double v10; // fp10
    double v11; // fp13
    double v12; // fp0

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2411, 0, "%s", "self");
    flags = self->ent->flags;
    if (dodgeRight)
    {
        if ((flags & 8) != 0)
        {
            v9 = (float)(self->Physics.vHitNormal[0] * (float)length);
            v10 = self->Physics.vWishDelta[1];
            result = 16;
            self->Physics.vWishDelta[0] = -(float)((float)((float)(self->Physics.vHitNormal[1] * (float)length)
                * (float)0.33000001)
                - self->Physics.vWishDelta[0]);
            self->Physics.vWishDelta[1] = (float)((float)v9 * (float)0.33000001) + (float)v10;
        }
        else
        {
            result = 16;
            v8 = (float)(self->Physics.vHitNormal[1] * (float)length);
            self->Physics.vWishDelta[1] = self->Physics.vHitNormal[0] * (float)length;
            self->Physics.vWishDelta[0] = -v8;
        }
    }
    else
    {
        v11 = (float)(self->Physics.vHitNormal[1] * (float)length);
        if ((flags & 0x10) != 0)
        {
            v11 = (float)((float)((float)(self->Physics.vHitNormal[1] * (float)length) * (float)0.33000001)
                + self->Physics.vWishDelta[0]);
            v12 = (float)-(float)((float)((float)(self->Physics.vHitNormal[0] * (float)length) * (float)0.33000001)
                - self->Physics.vWishDelta[1]);
        }
        else
        {
            v12 = -(float)(self->Physics.vHitNormal[0] * (float)length);
        }
        self->Physics.vWishDelta[1] = v12;
        result = 8;
        self->Physics.vWishDelta[0] = v11;
    }
    return result;
}

void __cdecl Actor_PhysicsBackupInputs(actor_s *self, PhysicsInputs *inputs)
{
    inputs->vVelocity[0] = self->Physics.vVelocity[0];
    inputs->vVelocity[1] = self->Physics.vVelocity[1];
    inputs->vVelocity[2] = self->Physics.vVelocity[2];
    inputs->groundEntNum = self->Physics.groundEntNum;
    inputs->bHasGroundPlane = self->Physics.bHasGroundPlane;
    inputs->groundplaneSlope = self->Physics.groundplaneSlope;
    inputs->iFootstepTimer = self->Physics.iFootstepTimer;
}

void __cdecl Actor_PhysicsRestoreInputs(actor_s *self, PhysicsInputs *inputs)
{
    self->Physics.vVelocity[0] = inputs->vVelocity[0];
    self->Physics.vVelocity[1] = inputs->vVelocity[1];
    self->Physics.vVelocity[2] = inputs->vVelocity[2];
    self->Physics.groundEntNum = inputs->groundEntNum;
    self->Physics.bHasGroundPlane = inputs->bHasGroundPlane;
    self->Physics.groundplaneSlope = inputs->groundplaneSlope;
    self->Physics.iFootstepTimer = inputs->iFootstepTimer;
}

bool __cdecl Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther)
{
    //return I_fabs((float)(vOrgSelf[2] - vOrgOther[2])) >= 80.0;
    return fabsf((float)(vOrgSelf[2] - vOrgOther[2])) >= 80.0f;
}

float __cdecl Actor_CalcultatePlayerPushDelta(const actor_s *self, const gentity_s *pusher, float *pushDir)
{
    double v6; // fp30
    double v7; // fp29
    double v8; // fp28
    double v9; // fp13
    double v10; // fp12
    double v11; // fp11
    double v12; // fp0
    double v13; // fp1
    float v15; // [sp+50h] [-70h] BYREF
    float v16; // [sp+54h] [-6Ch]
    float v17[14]; // [sp+58h] [-68h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2683, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2684, 0, "%s", "self->ent");
    if (!pusher)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2685, 0, "%s", "pusher");
    if (!pusher->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2686, 0, "%s", "pusher->client");
    v6 = (float)(self->ent->r.currentOrigin[0] - pusher->r.currentOrigin[0]);
    v7 = (float)(self->ent->r.currentOrigin[1] - pusher->r.currentOrigin[1]);
    v8 = Vec2NormalizeTo(pusher->client->ps.velocity, &v15);
    if (v8 == 0.0)
    {
        G_GetPlayerViewDirection(pusher, v17, 0, 0);
        Vec2NormalizeTo(v17, &v15);
    }
    v9 = v16;
    v10 = v15;
    v11 = (float)((float)v6 * v16);
    v12 = -v15;
    pushDir[1] = v12;
    *pushDir = v9;
    if ((float)((float)((float)v12 * (float)v7) + (float)v11) < 0.0)
    {
        pushDir[1] = v10;
        *pushDir = -v9;
    }
    if (v8 >= 60.0)
        v13 = (float)((float)v8 * 0.050000001);
    else
        v13 = (float)((float)60.0 * 0.050000001);
    return *((float *)&v13 + 1);
}

bool __cdecl Actor_ShouldMoveAwayFromCloseEnt(actor_s *self)
{
    bool result; // r3
    bool v3; // zf

    iassert(self);
    v3 = self->pCloseEnt.isDefined() == 0;
    result = 0;
    if (!v3 && self->Physics.ePhysicsType != AIPHYS_NOCLIP)
        return self->pushable;
    return result;
}

void __cdecl Actor_UpdateProneInformation(actor_s *self, int bDoProneCheck, float *a3, float *a4, long double a5)
{
    double v7; // fp31
    long double v8; // fp2
    double v9; // fp28
    double v10; // fp29
    double v11; // fp26
    double v12; // fp31
    double v13; // fp27
    DObj_s *ServerDObj; // r30
    int v15; // r7
    unsigned int v16; // r6
    unsigned int v17; // r5
    int v18; // r7
    unsigned int v19; // r6
    unsigned int v20; // r5
    int v21; // r7
    unsigned int v22; // r6
    unsigned int v23; // r5
    proneCheckType_t v24; // [sp+8h] [-B8h]

    if (!self->animSets.animProneLow || !self->animSets.animProneLevel || !self->animSets.animProneHigh)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            3029,
            0,
            "%s",
            "self->animSets.animProneLow && self->animSets.animProneLevel && self->animSets.animProneHigh");
    if (bDoProneCheck)
        self->bProneOK = BG_CheckProneValid(
            self->ent->s.number,
            self->ent->r.currentOrigin,
            15.0,
            48.0,
            self->ent->r.currentAngles[1],
            a3,
            a4,
            1,
            (_BYTE)self + 96,
            (_BYTE)self + 100,
            1u,
            v24,
            50.0);
    else
        self->fProneLastDiff = 360.0;
    v7 = (float)((float)(self->ProneInfo.fWaistPitch - self->ProneInfo.fTorsoPitch) * (float)0.0027777778);
    *(double *)&a5 = (float)((float)((float)(self->ProneInfo.fWaistPitch - self->ProneInfo.fTorsoPitch)
        * (float)0.0027777778)
        + (float)0.5);
    v8 = floor(a5);
    v9 = (float)((float)((float)v7 - (float)*(double *)&v8) * (float)360.0);
    if (self->fProneLastDiff != v9)
    {
        v10 = 0.0;
        v11 = 0.0;
        if (v9 >= 0.0)
        {
            v11 = (float)(self->fInvProneAnimHighPitch * (float)((float)((float)v7 - (float)*(double *)&v8) * (float)360.0));
            if (v11 > 0.99000001)
                v11 = 0.99000001;
            v12 = 1.0;
            v13 = (float)((float)1.0 - (float)v11);
        }
        else
        {
            v10 = (float)(self->fInvProneAnimLowPitch * (float)((float)((float)v7 - (float)*(double *)&v8) * (float)360.0));
            if (v10 > 0.99000001)
                v10 = 0.99000001;
            v12 = 1.0;
            v13 = (float)((float)1.0 - (float)v10);
        }
        ServerDObj = Com_GetServerDObj(self->ent->s.number);
        XAnimSetGoalWeight(ServerDObj, self->animSets.animProneLow, v10, 0.050000001, v12, v17, v16, v15);
        XAnimSetGoalWeight(ServerDObj, self->animSets.animProneLevel, v13, 0.050000001, v12, v20, v19, v18);
        XAnimSetGoalWeight(ServerDObj, self->animSets.animProneHigh, v11, 0.050000001, v12, v23, v22, v21);
        self->fProneLastDiff = v9;
    }
}

void __cdecl actor_controller(const gentity_s *self, int *partBits)
{
    actor_prone_info_s *p_ProneInfo; // r31
    double ActorProneFraction; // fp31
    DObjAnimMat *RotTransArray; // r3
    DObjAnimMat *v7; // r30

    if (!self->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3085, 0, "%s", "self->actor");
    p_ProneInfo = &self->actor->ProneInfo;
    if (BG_ActorIsProne(p_ProneInfo, level.time) && SV_DObjSetRotTransIndex(self, partBits, 0))
    {
        ActorProneFraction = BG_GetActorProneFraction(p_ProneInfo, level.time);
        RotTransArray = SV_DObjGetRotTransArray(self);
        v7 = RotTransArray;
        if (RotTransArray)
        {
            PitchToQuat(
                (float)(p_ProneInfo->fTorsoPitch * (float)ActorProneFraction),
                RotTransArray->quat,
                RotTransArray->quat);
            DObjSetTrans(v7, vec3_origin);
        }
    }
}

bool __cdecl Actor_PointNear(const float *vPoint, const float *vGoalPos)
{
    double v2; // fp0

    v2 = (float)(vPoint[2] - vGoalPos[2]);
    return (float)((float)v2 * (float)v2) <= 6400.0 && Vec2DistanceSq(vPoint, vGoalPos) <= 900.0;
}

bool __cdecl Actor_PointNearNode(const float *vPoint, const pathnode_t *node)
{
    double v2; // fp0

    v2 = (float)(vPoint[2] - node->constant.vOrigin[2]);
    return (float)((float)v2 * (float)v2) <= 6400.0 && Vec2DistanceSq(vPoint, node->constant.vOrigin) <= 225.0;
}

int __cdecl Actor_PointAtGoal(const float *vPoint, const actor_goal_s *goal)
{
    double v4; // fp13
    int result; // r3
    double radius; // fp31
    gentity_s *volume; // r5
    bool v8; // zf

    v4 = (float)(vPoint[2] - goal->pos[2]);
    if ((float)((float)v4 * (float)v4) > (double)(float)(goal->height * goal->height))
        return 0;
    radius = goal->radius;
    if (Vec2DistanceSq(vPoint, goal->pos) > (double)(float)((float)radius * (float)radius))
        return 0;
    volume = goal->volume;
    if (!volume)
        return 1;
    v8 = SV_EntityContact(vPoint, vPoint, volume) == 0;
    result = 0;
    if (!v8)
        return 1;
    return result;
}

bool __cdecl Actor_PointNearGoal(const float *vPoint, const actor_goal_s *goal, double buffer)
{
    double v3; // fp13
    double v5; // fp0

    v3 = (float)(vPoint[2] - goal->pos[2]);
    if ((float)((float)v3 * (float)v3) > (double)(float)(goal->height * goal->height))
        return 0;
    v5 = (float)(goal->radius + (float)buffer);
    return Vec2DistanceSq(vPoint, goal->pos) <= (double)(float)((float)v5 * (float)v5);
}

bool __cdecl Actor_PointNearPoint(const float *vPoint, const float *vGoalPos, double buffer)
{
    double v3; // fp0

    v3 = (float)(vPoint[2] - vGoalPos[2]);
    return (float)((float)v3 * (float)v3) <= 6400.0
        && Vec2DistanceSq(vPoint, vGoalPos) <= (double)(float)((float)buffer * (float)buffer);
}

bool __cdecl Actor_PointAt(const float *vPoint, const float *vGoalPos)
{
    double v2; // fp0

    v2 = (float)(vPoint[2] - vGoalPos[2]);
    return (float)((float)v2 * (float)v2) <= 6400.0 && Vec2DistanceSq(vPoint, vGoalPos) <= 4.0;
}

void __cdecl Actor_HandleInvalidPath(actor_s *self)
{
    bool useMeleeAttackSpot; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3398, 0, "%s", "self");
    useMeleeAttackSpot = self->useMeleeAttackSpot;
    self->pathWaitTime = level.time + 500;
    if (!useMeleeAttackSpot)
    {
        Actor_TeamMoveBlocked(self);
        if (Actor_IsSuppressedInAnyway(self))
        {
            if (ai_badPathSpam->current.enabled)
                Com_Printf(
                    18,
                    "AI (entity %d, origin %.1f %.1f %.1f) couldn't find path to goal. Maybe suppressed.\n",
                    0, // KISAKTODO ent id
                    self->ent->r.currentOrigin[0],
                    self->ent->r.currentOrigin[1],
                    self->ent->r.currentOrigin[2]);
        }
        else
        {
            Com_Printf(
                18,
                "AI (entity %d, origin %.1f %.1f %.1f) couldn't find path to goal.\n",
                0, // KISAKTODO ent id
                self->ent->r.currentOrigin[0],
                self->ent->r.currentOrigin[1],
                self->ent->r.currentOrigin[2]);
        }
        if (!self->ent)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3419, 0, "%s", "self->ent");
        Scr_AddVector(g_pathAttemptGoalPos);
        Scr_Notify(self->ent, scr_const.bad_path, 1u);
    }
}

pathnode_t *__cdecl Actor_FindClaimedNode(actor_s *self)
{
    pathnode_t *pClaimedNode; // r30
    char v3; // r29
    pathnode_t *result; // r3
    pathnode_t *node; // r4

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3503, 0, "%s", "self");
    if (self->ent->tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3504, 0, "%s", "!self->ent->tagInfo");
    pClaimedNode = self->sentient->pClaimedNode;
    if (pClaimedNode)
    {
        v3 = Actor_PointAtGoal(pClaimedNode->constant.vOrigin, &self->codeGoal);
        if (v3 && Actor_Cover_IsValidCover(self, pClaimedNode))
            return pClaimedNode;
    }
    else
    {
        v3 = 0;
    }
    if (Actor_GetTargetEntity(self) || (node = self->codeGoal.node) == 0)
    {
        result = Actor_Cover_FindBestCover(self);
        if (result)
            return result;
        if (v3)
            return self->sentient->pClaimedNode;
    }
    else if (Actor_Cover_IsValidCover(self, node)
        && (unsigned __int8)Actor_PointAtGoal(self->codeGoal.node->constant.vOrigin, &self->codeGoal))
    {
        return self->codeGoal.node;
    }
    return 0;
}

bool __cdecl Actor_EnemyInPathFightDist(actor_s *self, sentient_s *enemy)
{
    bool result; // r3
    double v5; // fp0
    double v6; // fp13

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3541, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3542, 0, "%s", "enemy");
    result = 0;
    v5 = (float)(enemy->ent->r.currentOrigin[0] - self->ent->r.currentOrigin[0]);
    v6 = (float)(enemy->ent->r.currentOrigin[1] - self->ent->r.currentOrigin[1]);
    if ((float)((float)((float)v6 * (float)v6) + (float)((float)v5 * (float)v5)) < (double)(float)(self->pathEnemyFightDist* self->pathEnemyFightDist))
        return I_fabs((float)(enemy->ent->r.currentOrigin[2] - self->ent->r.currentOrigin[2])) <= self->codeGoal.height;
        //return I_fabs((float)(enemy->ent->r.currentOrigin[2] - self->ent->r.currentOrigin[2])) <= self->codeGoal.height;
    return result;
}

bool __cdecl Actor_IsCloseToSegment(
    float *origin,
    float *pathPoint,
    double len,
    float *dir,
    double requiredDistFromPathSq,
    float *a6)
{
    double v6; // fp13
    double v7; // fp10
    double v9; // fp0
    double v10; // fp0

    v7 = (float)((float)(*a6 * (float)(*pathPoint - *origin)) + (float)(a6[1] * (float)(pathPoint[1] - origin[1])));
    if (v7 >= len)
        return 0;
    if (v7 > 0.0)
    {
        v10 = (float)((float)(a6[1] * (float)(*pathPoint - *origin)) - (float)(*a6 * (float)(pathPoint[1] - origin[1])));
        v9 = (float)((float)v10 * (float)v10);
    }
    else
    {
        v6 = (float)(pathPoint[1] - origin[1]);
        v9 = (float)((float)((float)v6 * (float)v6) + (float)((float)(*pathPoint - *origin) * (float)(*pathPoint - *origin)));
    }
    return v9 < requiredDistFromPathSq;
}

int __cdecl Actor_IsAlongPath(actor_s *self, float *origin, float *pathPoint, int hadPath)
{
    path_t *p_Path; // r29
    double pathEnemyLookahead; // fp0
    double v10; // fp31
    float *v11; // r5
    float *v12; // r5
    double v13; // fp2
    float *vOrigPoint; // r4
    int v15; // r9
    int v16; // r4
    double v17; // fp8
    double v18; // fp1
    double v19; // fp8
    double pathEnemyFightDist; // fp7
    float *v21; // r10
    double v22; // fp0
    double v23; // fp13
    double v24; // fp10
    bool v25; // r11
    double v26; // fp0
    double v27; // fp0

    p_Path = &self->Path;
    if (self == (actor_s *)-796)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3601, 0, "%s", "path");
    if (p_Path->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3602, 0, "%s", "path->wPathLen > 0");
    if ((unsigned __int16)p_Path->lookaheadNextNode >= 0x8000u)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3603, 0, "%s", "path->lookaheadNextNode >= 0");
    if (p_Path->lookaheadNextNode >= p_Path->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            3604,
            0,
            "%s",
            "path->lookaheadNextNode < path->wPathLen");
    pathEnemyLookahead = self->pathEnemyLookahead;
    if (!hadPath)
        pathEnemyLookahead = (float)(self->pathEnemyLookahead + (float)256.0);
    v10 = (float)((float)pathEnemyLookahead * (float)pathEnemyLookahead);
    if (Vec2DistanceSq(self->ent->r.currentOrigin, origin) < v10)
        return 1;
    if (Actor_IsCloseToSegment(origin, pathPoint, p_Path->fLookaheadDist, v11, v10, p_Path->lookaheadDir))
        return 1;
    vOrigPoint = p_Path->pts[p_Path->lookaheadNextNode].vOrigPoint;
    if (Actor_IsCloseToSegment(origin, vOrigPoint, p_Path->fLookaheadDistToNextNode, v12, v13, vOrigPoint + 3))
        return 1;
    v19 = (float)((float)v18 + (float)v17);
    pathEnemyFightDist = self->pathEnemyFightDist;
    if (v19 < pathEnemyFightDist)
    {
        v21 = (float *)(v16 + 16);
        do
        {
            --v15;
            v21 -= 7;
            if (v15 < 0)
                break;
            v24 = (float)((float)(*(v21 - 1) * (float)(*(v21 - 4) - *origin)) + (float)(*v21 * (float)(*(v21 - 3) - origin[1])));
            if (v24 < v21[1])
            {
                if (v24 > 0.0)
                {
                    v27 = (float)((float)(*v21 * (float)(*(v21 - 4) - *origin))
                        - (float)(*(v21 - 1) * (float)(*(v21 - 3) - origin[1])));
                    v26 = (float)((float)v27 * (float)v27);
                }
                else
                {
                    v23 = (float)(*(v21 - 3) - origin[1]);
                    v22 = (float)(*(v21 - 4) - *origin);
                    v26 = (float)((float)((float)v23 * (float)v23) + (float)((float)v22 * (float)v22));
                }
                v25 = v26 < v10;
            }
            else
            {
                v25 = 0;
            }
            if (v25)
                return 1;
            v19 = (float)(v21[1] + (float)v19);
        } while (v19 < pathEnemyFightDist);
    }
    return 0;
}

int __cdecl Actor_IsDoingCover(actor_s *self)
{
    scr_animscript_t *pAnimScriptFunc; // r11
    unsigned __int8 v2; // r11
    bool v3; // zf

    pAnimScriptFunc = self->pAnimScriptFunc;
    if (pAnimScriptFunc == &g_scr_data.anim.cover_left)
        return 1;
    if (pAnimScriptFunc == &g_scr_data.anim.cover_right)
        return 1;
    if (pAnimScriptFunc == &g_scr_data.anim.cover_stand)
        return 1;
    if (pAnimScriptFunc == &g_scr_data.anim.cover_crouch)
        return 1;
    if (pAnimScriptFunc == &g_scr_data.anim.cover_wide_left)
        return 1;
    v3 = pAnimScriptFunc != &g_scr_data.anim.cover_wide_right;
    v2 = 0;
    if (!v3)
        return 1;
    return v2;
}

bool __cdecl Actor_IsReactingToEnemyDuringReacquireMove(actor_s *self)
{
    unsigned int stateLevel; // r11

    stateLevel = self->stateLevel;
    return self->eState[stateLevel] == AIS_EXPOSED
        && self->eSubState[stateLevel] == STATE_EXPOSED_COMBAT
        && self->exposedDuration + self->exposedStartTime > level.time;
}

gentity_s *__cdecl Actor_IsKnownEnemyInRegion(
    const actor_s *self,
    const gentity_s *volume,
    const float *position,
    double radius)
{
    int v8; // r27
    unsigned int v9; // r30
    const float *i; // r31
    sentient_s *v11; // r11
    int v12; // r10
    __int64 v13; // r10
    gentity_s *ent; // r11
    const actor_s *actor; // r3
    bool v16; // cr58

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3772, 0, "%s", "self");
    if (!position)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3773, 0, "%s", "position");
    if (radius == 0.0 && !volume)
        return 0;
    v8 = 0;
    v9 = 0;
    for (i = self->sentientInfo[0].vLastKnownPos; ; i += 10)
    {
        v11 = &level.sentients[v9];
        if (!level.sentients[v9].inuse)
            goto LABEL_21;
        v12 = *((unsigned int *)i - 2);
        if (!v12)
            goto LABEL_21;
        if (level.time - v12 > 10000)
            goto LABEL_21;
        HIDWORD(v13) = v11->eTeam;
        if (HIDWORD(v13) == self->sentient->eTeam)
            goto LABEL_21;
        ent = v11->ent;
        LODWORD(v13) = ent->health;
        if ((float)v13 == 0.0)
            goto LABEL_21;
        actor = ent->actor;
        if (actor)
        {
            if (actor->ignoreForFixedNodeSafeCheck || Actor_IsDying(actor))
                goto LABEL_21;
        }
        if (!volume)
            break;
        if (self->fixedNodeSafeVolumeRadiusSq >= (double)Vec2DistanceSq(i, volume->r.currentOrigin))
        {
            v16 = SV_EntityContact(i, i, volume) == 0;
            goto LABEL_20;
        }
    LABEL_21:
        ++v9;
        ++v8;
        if (v9 >= 33)
            return 0;
    }
    v16 = !Actor_PointNearPoint(position, i, radius);
LABEL_20:
    if (v16)
        goto LABEL_21;
    return level.sentients[v8].ent;
}

int __cdecl Actor_InFixedNodeExposedCombat(actor_s *self)
{
    gentity_s *TargetEntity; // r3
    sentient_s *sentient; // r9
    bool v7; // r3
    unsigned __int8 v8; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3882, 0, "%s", "self");
    TargetEntity = Actor_GetTargetEntity(self);
    if (!TargetEntity)
        return 0;
    sentient = TargetEntity->sentient;
    if (sentient)
    {
        if (level.time - self->sentientInfo[sentient - level.sentients].lastKnownPosTime > 10000)
            return 0;
    }
    if ((AnimScriptList *)self->pAnimScriptFunc != &g_scr_data.anim)
        return 0;

    // aislop
    //_FP12 = (float)((float)64.0 - self->codeGoal.radius);
    //__asm { fsel      f1, f12, f13, f0# buffer }
    //v7 = Actor_PointNearPoint(self->ent->r.currentOrigin, self->codeGoal.pos, _FP1);

    float distance = 64.0f - self->codeGoal.radius;
    float clampedDistance = (distance >= 0.0f) ? distance : 0.0f;
    v7 = Actor_PointNearPoint(self->ent->r.currentOrigin, self->codeGoal.pos, clampedDistance);


    v8 = 1;
    if (!v7)
        return 0;
    return v8;
}

bool __cdecl Actor_HasPath(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    return Path_Exists(&self->Path);
}

void __cdecl Actor_InitPath(actor_s *self)
{
    Path_Begin(&self->Path);
    self->iTeamMoveDodgeTime = 0;
}

void __cdecl Actor_ClearPath(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4516, 0, "%s", "self");
    if (self->Path.wPathLen)
    {
        Path_AddTrimmedAmount(&self->Path, self->ent->r.currentOrigin);
        Path_Clear(&self->Path);
        self->iTeamMoveDodgeTime = 0;
    }
}

void __cdecl Actor_GetAnimDeltas(actor_s *self, float *rotation, float *translation)
{
    DObj_s *obj; // r28

    iassert(self);
    iassert(rotation);
    iassert(translation);

    obj = Com_GetServerDObj(self->ent->s.number);

    iassert(obj);

    XAnimCalcDelta(obj, 0, rotation, translation, self->bUseGoalWeight);

    if (ai_debugAnimDeltas->current.integer == self->ent->s.number)
        Com_Printf(
            18,
            "deltas = %g %g %g\n",
            translation[0],
            translation[1],
            translation[2]
        );
}

int __cdecl Actor_IsMovingToMeleeAttack(actor_s *self)
{
    unsigned __int8 v1; // r11

    if (self->meleeAttackDist == 0.0)
        return 0;
    if (!self->useEnemyGoal)
        return 0;
    v1 = 1;
    if (!self->useMeleeAttackSpot)
        return 0;
    return v1;
}

bool __cdecl Actor_SkipPathEndActions(actor_s *self)
{
    float *v3; // r10
    double v4; // fp31
    double v5; // fp30
    double v6; // fp29
    char v7; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4567, 0, "%s", "self");
    if (self->Path.iPathEndTime)
        return 0;
    if (!self->Physics.bHasGroundPlane && self->Physics.groundEntNum == 2175
        || !Path_AttemptedCompleteLookahead(&self->Path))
    {
        return 1;
    }
    if (Path_UsesObstacleNegotiation(&self->Path))
    {
        v3 = (float *)((char *)self + 28 * self->Path.wNegotiationStartNode);
        v4 = (float)(v3[199] - self->ent->r.currentOrigin[0]);
        v5 = (float)(v3[200] - self->ent->r.currentOrigin[1]);
        v6 = (float)(v3[201] - self->ent->r.currentOrigin[2]);
        if ((float)((float)((float)v5 * (float)v5) + (float)((float)v4 * (float)v4)) <= (double)(float)((float)((float)((float)(self->Physics.vVelocity[2] * self->Physics.vVelocity[2]) + (float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0]) + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1]))) * (float)0.0049999999) + (float)0.000001))
        {
            if (Actor_PushState(self, AIS_NEGOTIATION))
            {
                Path_GetObstacleNegotiationScript(&self->Path, &self->AnimScriptSpecific);
                self->Physics.vWishDelta[0] = v4;
                self->Physics.vWishDelta[1] = v5;
                self->Physics.vWishDelta[2] = v6;
            }
        }
        return 1;
    }
    if (self->Path.pathEndAnimNotified || self->arrivalInfo.animscriptOverrideRunTo)
        return 1;
    if (self->meleeAttackDist == 0.0 || !self->useEnemyGoal || (v7 = 1, !self->useMeleeAttackSpot))
        v7 = 0;
    return v7 != 0;
}

void __cdecl Actor_PathEndActions(actor_s *self)
{
    double v2; // fp1
    double v3; // fp31
    char v4; // r11
    float *v5; // r10
    double v6; // fp31
    double v7; // fp30
    double v8; // fp28
    double v9; // fp29
    __int64 v10; // r11
    double v11; // fp12

    if (!self)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4621, 0, "%s", "self");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    }
    if (!Path_Exists(&self->Path))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4622, 0, "%s", "Actor_HasPath( self )");
    if (self->Path.pathEndAnimDistSq > 0.0)
    {
        v2 = Vec2DistanceSq(self->Path.vFinalGoal, self->ent->r.currentOrigin);
        v3 = v2;
        if (self->Path.pathEndAnimNotified)
        {
            if (v2 > self->Path.pathEndAnimDistSq)
            {
                Scr_Notify(self->ent, scr_const.run, 0);
                self->Path.pathEndAnimNotified = 0;
            }
        }
        else if (v2 < self->Path.pathEndAnimDistSq && Path_CompleteLookahead(&self->Path))
        {
            if (self->meleeAttackDist == 0.0 || !self->useEnemyGoal || (v4 = 1, !self->useMeleeAttackSpot))
                v4 = 0;
            if (!v4)
            {
                //Scr_AddFloat(sqrtf(v3));
                Scr_AddFloat(sqrtf(v3));
                Scr_Notify(self->ent, scr_const.stop_soon, 1u);
                self->Path.pathEndAnimNotified = 1;
            }
        }
    }
    if (!Actor_SkipPathEndActions(self) && self->Path.pathEndAnimDistSq <= 0.0)
    {
        v5 = (float *)((char *)self + 28 * self->Path.wNegotiationStartNode);
        v6 = (float)(v5[199] - self->ent->r.currentOrigin[0]);
        v7 = (float)(v5[200] - self->ent->r.currentOrigin[1]);
        v8 = (float)(v5[201] - self->ent->r.currentOrigin[2]);
        if (self->Path.iPathEndTime)
            goto LABEL_25;
        v9 = (float)((float)((float)v7 * (float)v7) + (float)((float)v6 * (float)v6));
        if ((v9 == 0.0 || Path_CompleteLookahead(&self->Path))
            && (float)((float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
                + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1]))
                * (float)0.0625) >= v9)
        {
            self->Path.iPathEndTime = level.time + 500;
        LABEL_25:
            LODWORD(v10) = self->Path.iPathEndTime - level.time;
            if ((int)v10 > 0)
            {
                HIDWORD(v10) = 0x82000000;
                if (self->Physics.bHasGroundPlane)
                    v11 = (float)(1.0 / self->Physics.groundplaneSlope);
                else
                    v11 = 1.0;
                self->Physics.vWishDelta[0] = (float)v6
                    * (float)((float)((float)((float)2.0 - (float)((float)50.0 / (float)v10))
                        * (float)((float)50.0 / (float)v10))
                        * (float)v11);
                self->Physics.vWishDelta[1] = (float)v7
                    * (float)((float)((float)((float)2.0 - (float)((float)50.0 / (float)v10))
                        * (float)((float)50.0 / (float)v10))
                        * (float)v11);
                self->Physics.vWishDelta[2] = (float)v8
                    * (float)((float)((float)((float)2.0 - (float)((float)50.0 / (float)v10))
                        * (float)((float)50.0 / (float)v10))
                        * (float)v11);
            }
            else
            {
                Actor_ClearPath(self);
                self->Path.iPathEndTime = 0;
                self->Physics.vWishDelta[0] = v6;
                self->Physics.vWishDelta[1] = v7;
                self->Physics.vWishDelta[2] = v8;
            }
        }
    }
}

void __cdecl Actor_PredictAnim(actor_s *self)
{
    gentity_s *ent; // r29
    int Int; // r3
    int updated; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4921, 0, "%s", "self");
    ent = self->ent;
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4923, 0, "%s", "ent");
    Int = Scr_GetInt(0);
    updated = G_DObjUpdateServerTime(ent, Int);
    Scr_AddInt(updated);
}

bool __cdecl Actor_AtClaimNode(actor_s *self)
{
    pathnode_t *pClaimedNode; // r4

    pClaimedNode = self->sentient->pClaimedNode;
    return pClaimedNode && Actor_PointNearNode(self->ent->r.currentOrigin, pClaimedNode);
}

bool __cdecl Actor_NearClaimNode(actor_s *self, double dist)
{
    pathnode_t *pClaimedNode; // r11

    pClaimedNode = self->sentient->pClaimedNode;
    return pClaimedNode && Actor_PointNearPoint(self->ent->r.currentOrigin, pClaimedNode->constant.vOrigin, dist);
}

void __cdecl Actor_CheckCollisions(actor_s *self)
{
    gentity_s *ent; // r11
    scr_animscript_t *pAnimScriptFunc; // r8
    AISpecies species; // r9
    AnimScriptList *v5; // r10
    int v6; // r26
    int v7; // r29
    actor_s *v8; // r31
    float v9[20]; // [sp+50h] [-50h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4962, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4963, 0, "%s", "self->sentient");
    if (!self->pCloseEnt.isDefined())
    {
        ent = self->ent;
        pAnimScriptFunc = self->pAnimScriptFunc;
        species = self->species;
        v9[0] = self->ent->r.currentOrigin[0];
        v5 = g_animScriptTable[species];
        v9[1] = ent->r.currentOrigin[1];
        v9[2] = ent->r.currentOrigin[2];
        if (pAnimScriptFunc == &v5->grenade_cower || (v6 = 0, Actor_AtClaimNode(self)))
            v6 = 1;
        v7 = 0;
        while (1)
        {
            v8 = &level.actors[v7];
            if (level.actors[v7].inuse && v8 != self && (!v6 || level.gentities[v8->Physics.iHitEntnum].sentient))
            {
                if (Actor_PointNear(v8->sentient->ent->r.currentOrigin, v9))
                    break;
            }
            if (++v7 >= 32)
                return;
        }
        self->pCloseEnt.setEnt(v8->ent);
    }
}

void __cdecl Actor_ClearPileUp(actor_s *self)
{
    self->pPileUpActor = 0;
    self->pPileUpEnt = 0;
}

void __cdecl Actor_ClipPathToGoal(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5045, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5046, 0, "%s", "self->sentient");
    if (!Path_ClipToGoal(&self->Path, &self->codeGoal))
        Actor_ClearPath(self);
}

void __cdecl Actor_BeginTrimPath(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5058, 0, "%s", "self");
    Path_BeginTrim(&self->Path, &self->TrimInfo);
}

int __cdecl Actor_TrimPathToAttack(actor_s *self)
{
    gentity_s *TargetEntity; // r28
    int v3; // r6
    float v5; // [sp+50h] [-40h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5072, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5073, 0, "%s", "self->sentient");
    TargetEntity = Actor_GetTargetEntity(self);
    if (!TargetEntity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5079, 0, "%s", "targetEnt");
    Actor_GetTargetLookPosition(self, &v5);
    return Path_TrimToSeePoint(
        &self->Path,
        &self->TrimInfo,
        self,
        self->fMaxSightDistSqrd,
        v3,
        (const float *)TargetEntity->s.number);
}

int __cdecl Actor_MayReacquireMove(actor_s *self)
{
    unsigned __int8 v1; // r11

    if (self->Path.wPathLen <= 0)
        return 1;
    v1 = 0;
    if (self->TrimInfo.iDelta)
        return 1;
    return v1;
}

void __cdecl Actor_ClearMoveHistory(actor_s *self)
{
    float *v1; // r11
    int v2; // r10

    v1 = &self->moveHistory[0][1];
    v2 = 10;
    do
    {
        --v2;
        *(v1 - 1) = 0.0;
        *v1 = 0.0;
        v1 += 2;
    } while (v2);
    self->moveHistoryIndex = 0;
}

void __cdecl Actor_GetMoveHistoryAverage(actor_s *self, float *vDir)
{
    *vDir = 0.0;
    vDir[1] = 0.0;
    vDir[2] = 0.0;
    *vDir = self->moveHistory[0][0];
    vDir[1] = self->moveHistory[0][1];
    *vDir = self->moveHistory[1][0] + *vDir;
    vDir[1] = self->moveHistory[1][1] + vDir[1];
    *vDir = self->moveHistory[2][0] + *vDir;
    vDir[1] = self->moveHistory[2][1] + vDir[1];
    *vDir = self->moveHistory[3][0] + *vDir;
    vDir[1] = self->moveHistory[3][1] + vDir[1];
    *vDir = self->moveHistory[4][0] + *vDir;
    vDir[1] = self->moveHistory[4][1] + vDir[1];
    *vDir = self->moveHistory[5][0] + *vDir;
    vDir[1] = self->moveHistory[5][1] + vDir[1];
    *vDir = self->moveHistory[6][0] + *vDir;
    vDir[1] = self->moveHistory[6][1] + vDir[1];
    *vDir = self->moveHistory[7][0] + *vDir;
    vDir[1] = self->moveHistory[7][1] + vDir[1];
    *vDir = self->moveHistory[8][0] + *vDir;
    vDir[1] = self->moveHistory[8][1] + vDir[1];
    *vDir = self->moveHistory[9][0] + *vDir;
    vDir[1] = self->moveHistory[9][1] + vDir[1];
}

void __cdecl Actor_UpdateMoveHistory(actor_s *self)
{
    int moveHistoryIndex; // r7
    int v2; // r11
    int v3; // r10
    int v4; // r11
    bool v5; // zf
    float *v6; // r11
    float *v7; // r9
    double v8; // fp0
    int v9; // r10
    float *v10; // r10
    int v11; // r10
    double v12; // fp0
    unsigned int v13; // r9
    float *v14; // r11
    double v15; // fp12
    double v16; // fp0
    double v17; // fp9
    double v18; // fp8
    double v19; // fp7
    double v20; // fp6
    double v21; // fp5
    double v22; // fp4
    double v23; // fp3
    double v24; // fp12
    double v25; // fp11
    double v26; // fp0
    double v27; // fp12
    double v28; // fp11
    double v29; // fp0
    double v30; // fp12
    double v31; // fp11
    int v32; // r10
    char *v33; // r11
    float *v34; // r11
    double v35; // fp12
    double v36; // fp10
    double v37; // fp12
    int v38; // r9
    float *v39; // r11
    unsigned int v40; // r10
    double v41; // fp12
    double v42; // fp0
    double v43; // fp9
    double v44; // fp8
    double v45; // fp7
    double v46; // fp6
    double v47; // fp5
    double v48; // fp4
    double v49; // fp3
    double v50; // fp12
    double v51; // fp11
    double v52; // fp0
    double v53; // fp12
    double v54; // fp11
    double v55; // fp0
    double v56; // fp12
    double v57; // fp11
    int v58; // r10
    float *v59; // r11
    double v60; // fp12
    double v61; // fp10
    double v62; // fp12

    moveHistoryIndex = self->moveHistoryIndex;
    v2 = moveHistoryIndex;
    if (!moveHistoryIndex)
        v2 = 10;
    v3 = v2 - 1;
    v5 = v2 != 1;
    v4 = v2 - 1;
    if (!v5)
        v4 = 10;
    v7 = self->moveHistory[v3];
    v8 = self->moveHistory[v3][1];
    v9 = v4 - 1 + 466;
    v6 = self->moveHistory[moveHistoryIndex];
    v10 = (float *)((char *)self + 8 * v9);
    if ((float)((float)((float)(v10[1] * (float)-*v7) + (float)(*v10 * (float)v8))
        * (float)((float)(v6[1] * (float)-*v7) + (float)(*v6 * (float)v8))) <= 0.0
        && (float)((float)(v7[1] * v6[1]) + (float)(*v7 * *v6)) >= 0.0
        && (float)((float)(v7[1] * v10[1]) + (float)(*v7 * *v10)) >= 0.0)
    {
        *v10 = *v6;
        v10[1] = v6[1];
    }
    v11 = moveHistoryIndex + 1;
    v12 = 1.0;
    if (10 - (moveHistoryIndex + 1) >= 4)
    {
        v13 = ((unsigned int)(6 - v11) >> 2) + 1;
        v14 = &self->moveHistory[v11][1];
        v11 += 4 * v13;
        do
        {
            v15 = v12;
            v16 = (float)((float)v12 + (float)1.0);
            v17 = *v14;
            v18 = v14[1];
            --v13;
            v19 = v14[2];
            v20 = v14[3];
            v21 = v14[4];
            v22 = v14[5];
            v23 = v14[6];
            v24 = (float)((float)v15 / (float)v16);
            v25 = v16;
            v26 = (float)((float)v16 + (float)1.0);
            *(v14 - 1) = *(v14 - 1) * (float)v24;
            *v14 = (float)v17 * (float)v24;
            v27 = (float)((float)v25 / (float)v26);
            v28 = v26;
            v29 = (float)((float)v26 + (float)1.0);
            v14[1] = (float)v27 * (float)v18;
            v14[2] = (float)v19 * (float)v27;
            v30 = (float)((float)v28 / (float)v29);
            v31 = v29;
            v12 = (float)((float)v29 + (float)1.0);
            v14[3] = (float)v20 * (float)v30;
            v14[4] = (float)v21 * (float)v30;
            v14[5] = (float)((float)v31 / (float)v12) * (float)v22;
            v14[6] = (float)v23 * (float)((float)v31 / (float)v12);
            v14 += 8;
        } while (v13);
    }
    if (v11 < 10)
    {
        v33 = (char *)self + 8 * v11;
        v32 = 10 - v11;
        v34 = (float *)(v33 + 3732);
        do
        {
            v35 = v12;
            v12 = (float)((float)v12 + (float)1.0);
            v36 = *v34;
            --v32;
            v37 = (float)((float)v35 / (float)v12);
            *(v34 - 1) = *(v34 - 1) * (float)v37;
            *v34 = (float)v36 * (float)v37;
            v34 += 2;
        } while (v32);
    }
    v38 = 0;
    if (moveHistoryIndex >= 4)
    {
        v39 = &self->moveHistory[0][1];
        v40 = ((unsigned int)(moveHistoryIndex - 4) >> 2) + 1;
        v38 = 4 * v40;
        do
        {
            v41 = v12;
            v42 = (float)((float)v12 + (float)1.0);
            v43 = *v39;
            v44 = v39[1];
            --v40;
            v45 = v39[2];
            v46 = v39[3];
            v47 = v39[4];
            v48 = v39[5];
            v49 = v39[6];
            v50 = (float)((float)v41 / (float)v42);
            v51 = v42;
            v52 = (float)((float)v42 + (float)1.0);
            *(v39 - 1) = *(v39 - 1) * (float)v50;
            *v39 = (float)v43 * (float)v50;
            v53 = (float)((float)v51 / (float)v52);
            v54 = v52;
            v55 = (float)((float)v52 + (float)1.0);
            v39[1] = (float)v53 * (float)v44;
            v39[2] = (float)v45 * (float)v53;
            v56 = (float)((float)v54 / (float)v55);
            v57 = v55;
            v12 = (float)((float)v55 + (float)1.0);
            v39[3] = (float)v46 * (float)v56;
            v39[4] = (float)v47 * (float)v56;
            v39[5] = (float)((float)v57 / (float)v12) * (float)v48;
            v39[6] = (float)v49 * (float)((float)v57 / (float)v12);
            v39 += 8;
        } while (v40);
    }
    if (v38 < moveHistoryIndex)
    {
        v58 = moveHistoryIndex - v38;
        v59 = &self->moveHistory[v38][1];
        do
        {
            v60 = v12;
            v12 = (float)((float)v12 + (float)1.0);
            v61 = *v59;
            --v58;
            v62 = (float)((float)v60 / (float)v12);
            *(v59 - 1) = *(v59 - 1) * (float)v62;
            *v59 = (float)v61 * (float)v62;
            v59 += 2;
        } while (v58);
    }
    self->moveHistoryIndex = (moveHistoryIndex + 1) % 10;
}

void __cdecl Path_UpdateLeanAmount(actor_s *self, float *vWishDir)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5186, 0, "%s", "self");
    if (!vWishDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5187, 0, "%s", "vWishDir");
    if (self->Path.wNegotiationStartNode == self->Path.wPathLen - 2
        && Actor_PointNearPoint(
            self->ent->r.currentOrigin,
            self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint,
            100.0))
    {
        self->leanAmount = (float)(self->leanAmount + (float)1.0) * (float)0.5;
        self->prevMoveDir[0] = 0.0;
        self->prevMoveDir[1] = 0.0;
        self->leanAmount = 0.0;
    }
    else
    {
        self->leanAmount = (float)(self->prevMoveDir[1] * vWishDir[1]) + (float)(*vWishDir * self->prevMoveDir[0]);
    }
}

float __cdecl Path_UpdateMomentum(actor_s *self, float *vWishDir, double fMoveDist)
{
    double leanAmount; // fp0
    double v7; // fp12
    double value; // fp0
    double v9; // fp1

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5225, 0, "%s", "self");
    if (!vWishDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5226, 0, "%s", "vWishDir");
    if (fMoveDist > 15.0)
    {
        Path_UpdateLeanAmount(self, vWishDir);
        leanAmount = self->leanAmount;
        fMoveDist = (float)((float)((float)((float)((float)(self->leanAmount + (float)1.0) * (float)0.5) + (float)1.0)
            * (float)fMoveDist)
            * (float)0.5);
        if (leanAmount > 0.0
            && (float)((float)(self->prevMoveDir[1] * *vWishDir) - (float)(self->prevMoveDir[0] * vWishDir[1])) < 0.0)
        {
            self->leanAmount = -leanAmount;
        }
        v7 = vWishDir[1];
        value = ai_pathMomentum->current.value;
        *vWishDir = (float)((float)(self->prevMoveDir[0] - *vWishDir) * ai_pathMomentum->current.value) + *vWishDir;
        vWishDir[1] = (float)((float)(self->prevMoveDir[1] - (float)v7) * (float)value) + (float)v7;
        Vec2Normalize(vWishDir);
    }
    v9 = fMoveDist;
    self->prevMoveDir[0] = *vWishDir;
    self->prevMoveDir[1] = vWishDir[1];
    return *((float *)&v9 + 1);
}

bool __cdecl Path_UseMomentum(actor_s *self)
{
    return self->species == AI_SPECIES_DOG;
}

void __cdecl Path_UpdateMovementDelta(actor_s *self, double fMoveDist)
{
    path_t *p_Path; // r30
    gentity_s *ent; // r11
    int iHitEntnum; // r4
    bool IsDodgeEntity; // r3
    long double v8; // fp2
    double v9; // fp12
    AISpecies species; // r9
    double v11; // fp11
    double v12; // fp10
    int moveHistoryIndex; // r29
    double v14; // fp0
    double v15; // fp10
    double v16; // fp0
    double sideMove; // fp11
    double v18; // fp0
    double v19; // fp12
    double v20; // fp7
    long double v21; // fp2
    long double v22; // fp2
    double v23; // fp0
    double fLookaheadDist; // fp1
    double v25; // fp13
    double v26; // fp11
    float *v27; // r11
    double v28; // fp12
    float v29; // [sp+50h] [-90h] BYREF
    float v30; // [sp+54h] [-8Ch]
    float v31; // [sp+58h] [-88h] BYREF
    float v32; // [sp+5Ch] [-84h]
    float v33; // [sp+60h] [-80h]
    float v34; // [sp+68h] [-78h] BYREF
    float v35; // [sp+6Ch] [-74h]
    float v36; // [sp+70h] [-70h]
    float v37; // [sp+78h] [-68h] BYREF
    float v38; // [sp+7Ch] [-64h]
    float v39; // [sp+80h] [-60h]
    float v40[6]; // [sp+88h] [-58h] BYREF

    p_Path = &self->Path;
    if (self == (actor_s *)-796)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5281, 0, "%s", "pPath");
    if (p_Path->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5282, 0, "%s", "pPath->wPathLen > 0");
    ent = self->ent;
    iHitEntnum = self->Physics.iHitEntnum;
    v34 = self->ent->r.currentOrigin[0];
    v35 = ent->r.currentOrigin[1];
    v36 = ent->r.currentOrigin[2];
    IsDodgeEntity = Actor_IsDodgeEntity(self, iHitEntnum);
    Path_UpdateLookahead(p_Path, &v34, IsDodgeEntity, 0, 1);
    v9 = p_Path->lookaheadDir[0];
    species = self->species;
    v11 = p_Path->lookaheadDir[1];
    v12 = p_Path->lookaheadDir[2];
    moveHistoryIndex = self->moveHistoryIndex;
    v31 = p_Path->lookaheadDir[0];
    v32 = v11;
    v33 = v12;
    v14 = (float)((float)((float)((float)fMoveDist * (float)20.0) - (float)(g_actorAssumedSpeed[species] * (float)0.5))
        / (float)(g_actorAssumedSpeed[species] * (float)0.5));
    if (v14 > 0.0)
    {
        if (v14 < 1.0)
        {
            v15 = (float)(self->Path.forwardLookaheadDir2D[1]
                * (float)((float)((float)((float)fMoveDist * (float)20.0)
                    - (float)(g_actorAssumedSpeed[species] * (float)0.5))
                    / (float)(g_actorAssumedSpeed[species] * (float)0.5)));
            v16 = (float)((float)1.0
                - (float)((float)((float)((float)fMoveDist * (float)20.0)
                    - (float)(g_actorAssumedSpeed[species] * (float)0.5))
                    / (float)(g_actorAssumedSpeed[species] * (float)0.5)));
            v29 = (float)((float)v9
                * (float)((float)1.0
                    - (float)((float)((float)((float)fMoveDist * (float)20.0)
                        - (float)(g_actorAssumedSpeed[species] * (float)0.5))
                        / (float)(g_actorAssumedSpeed[species] * (float)0.5))))
                + (float)(self->Path.forwardLookaheadDir2D[0]
                    * (float)((float)((float)((float)fMoveDist * (float)20.0)
                        - (float)(g_actorAssumedSpeed[species] * (float)0.5))
                        / (float)(g_actorAssumedSpeed[species] * (float)0.5)));
            v30 = (float)((float)v11 * (float)v16) + (float)v15;
            Vec2Normalize(&v29);
        }
        else
        {
            v29 = self->Path.forwardLookaheadDir2D[0];
            v30 = self->Path.forwardLookaheadDir2D[1];
        }
    }
    else
    {
        v29 = v9;
        v30 = v11;
    }
    if (self->sideMove != 0.0 && !Path_CompleteLookahead(p_Path))
    {
        sideMove = self->sideMove;
        v18 = (float)(p_Path->fLookaheadDist * (float)0.5);
        v19 = I_fabs(sideMove);
        v20 = (float)(p_Path->fLookaheadDist * v32);
        if (v18 > v19)
            v18 = v19;
        if (sideMove < 0.0)
            v18 = -v18;
        v37 = (float)(p_Path->fLookaheadDist * v31) + (float)((float)v18 * v32);
        v38 = (float)v20 + (float)((float)v18 * (float)-v31);
        Vec2Normalize(&v37);
        v40[0] = (float)(v37 * (float)60.0) + v34;
        v40[1] = (float)(v38 * (float)60.0) + v35;
        v39 = v33;
        v40[2] = (float)(v33 * (float)60.0) + v36;
        if (Path_LookaheadPredictionTrace(p_Path, &v34, v40))
        {
            v31 = v37;
            v32 = v38;
            v29 = v37;
            v30 = v38;
        }
    }
    *(double *)&v8 = (float)((float)(p_Path->lookaheadDir[1] * p_Path->forwardLookaheadDir2D[1])
        + (float)(p_Path->forwardLookaheadDir2D[0] * p_Path->lookaheadDir[0]));
    if (*(double *)&v8 < 0.000001)
        *(double *)&v8 = 0.000001;
    v21 = log(v8);
    *(double *)&v21 = (float)((float)*(double *)&v21 * (float)0.333);
    v22 = exp(v21);
    v23 = (float)*(double *)&v22;
    if (v23 < 0.60000002)
        v23 = 0.60000002;
    fLookaheadDist = (float)((float)v23 * (float)fMoveDist);
    if (fLookaheadDist > p_Path->fLookaheadDist)
        fLookaheadDist = p_Path->fLookaheadDist;
    if (self->species == AI_SPECIES_DOG && (self->Path.flags & 2) != 0)
        fLookaheadDist = Path_UpdateMomentum(self, &v31, fLookaheadDist);
    v25 = (float)(v32 * (float)fLookaheadDist);
    v26 = v29;
    v27 = self->moveHistory[moveHistoryIndex];
    v28 = (float)(v33 * (float)fLookaheadDist);
    self->Physics.vWishDelta[0] = v31 * (float)fLookaheadDist;
    self->Physics.vWishDelta[1] = v25;
    self->Physics.vWishDelta[2] = v28;
    *v27 = v26;
    v27[1] = v30;
    Actor_UpdateMoveHistory(self);
}

void __cdecl Actor_AddStationaryMoveHistory(actor_s *self)
{
    float *v1; // r11
    int v2; // r3
    float *v3; // r11

    if (level.time / 50 == 10 * (level.time / 50 / 10))
    {
        Actor_UpdateMoveHistory(self);
        v3 = (float *)(8 * (*(unsigned int *)(v2 + 3724) + 466) + v2);
        *v3 = *(float *)(v2 + 1716) * (float)0.1;
        v3[1] = *(float *)(v2 + 1720) * (float)0.1;
    }
    else
    {
        v1 = self->moveHistory[self->moveHistoryIndex];
        *v1 = (float)(self->Path.lookaheadDir[0] * (float)0.1) + *v1;
        v1[1] = (float)(self->Path.lookaheadDir[1] * (float)0.1) + v1[1];
    }
}

int __cdecl Actor_IsMoving(actor_s *self)
{
    unsigned __int8 v1; // r11

    if (self->eAnimMode != AI_ANIM_MOVE_CODE)
        return 0;
    v1 = 1;
    if (!self->moveMode)
        return 0;
    return v1;
}

unsigned int __cdecl G_GetActorFriendlyIndex(int iEntNum)
{
    gentity_s *v2; // r10
    actor_s *actor; // r11
    unsigned int v4; // r11

    if (!level.bDrawCompassFriendlies)
        return -1;
    if (iEntNum >= 2176)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5398, 0, "%s", "iEntNum < MAX_GENTITIES");
    v2 = &g_entities[iEntNum];
    actor = v2->actor;
    if (!actor || !actor->bDrawOnCompass || v2->sentient->eTeam != TEAM_ALLIES)
        return -1;
    v4 = (int)((unsigned __int64)(2248490037LL * ((char *)actor - (char *)level.actors)) >> 32) >> 12;
    return v4 + (v4 >> 31);
}

void __cdecl G_BypassForCG_GetClientActorIndexAndTeam(int iEntNum, int *actorIndex, int *team)
{
    gentity_s *v6; // r31
    double v7; // fp31
    double v8; // fp30
    double v9; // fp29
    const char *v10; // r3
    const char *v11; // r3

    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5415, 0, "%s", "Sys_IsMainThread()");
    if (!actorIndex)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5416, 0, "%s", "actorIndex");
    if (!team)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5417, 0, "%s", "team");
    *actorIndex = level.specialIndex[iEntNum];
    if (*actorIndex >= 32)
    {
        v6 = &level.gentities[iEntNum];
        v7 = v6->r.currentOrigin[2];
        v8 = v6->r.currentOrigin[1];
        v9 = v6->r.currentOrigin[0];
        v10 = SL_ConvertToString(v6->classname);
        v11 = va(
            "entnum: %d, inuse: %d, eType: %d, classname: %s, actorIndex: %d, origin: %.2f %.2f %.2f",
            v6->s.number,
            v6->r.inuse,
            v6->s.eType,
            v10,
            HIDWORD(v9),
            v9,
            v8,
            v7);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            5421,
            0,
            "%s\n\t%s",
            "*actorIndex < MAX_ACTORS",
            v11);
    }
    *team = level.cgData_actorTeam[*actorIndex];
}

unsigned int __cdecl G_BypassForCG_GetClientActorFriendlyIndex(int iEntNum)
{
    unsigned int v2; // r30
    gentity_s *v3; // r31
    double v4; // fp31
    double v5; // fp30
    double v6; // fp29
    const char *v7; // r3
    const char *v8; // r3
    unsigned int result; // r3

    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5431, 0, "%s", "Sys_IsMainThread()");
    v2 = level.specialIndex[iEntNum];
    if (v2 >= 0x20)
    {
        v3 = &level.gentities[iEntNum];
        v4 = v3->r.currentOrigin[2];
        v5 = v3->r.currentOrigin[1];
        v6 = v3->r.currentOrigin[0];
        v7 = SL_ConvertToString(v3->classname);
        v8 = va(
            "entnum: %d, inuse: %d, eType: %d, classname: %s, actorIndex: %d, origin: %.2f %.2f %.2f",
            v3->s.number,
            v3->r.inuse,
            v3->s.eType,
            v7,
            HIDWORD(v6),
            v6,
            v5,
            v4);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            5434,
            0,
            "%s\n\t%s",
            "actorIndex < MAX_ACTORS",
            v8);
    }
    result = v2;
    if (!level.cgData_actorOnCompass[v2])
        return -1;
    return result;
}

gentity_s *__cdecl G_GetFriendlyIndexActor(int iFriendlyIndex)
{
    if (iFriendlyIndex >= 32)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5451, 0, "%s", "iFriendlyIndex < MAX_ACTORS");
    return level.actors[iFriendlyIndex].ent;
}

void __cdecl Actor_SetFlashed(actor_s *self, int flashed, double strength)
{
    sentient_s *sentient; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5460, 0, "%s", "self");
    if (!flashed || self->flashBangImmunity)
    {
        self->flashBangedStrength = 0.0;
        self->flashBanged = 0;
    }
    else
    {
        if (self->flashBangedStrength < strength)
            self->flashBangedStrength = strength;
        sentient = self->sentient;
        self->flashBanged = 1;
        self->isInBadPlace = 0;
        Actor_ClearAllSuppressionFromEnemySentient(sentient);
    }
}

void __cdecl Actor_UpdateDesiredChainPosInternal(
    actor_s *self,
    int iFollowMin,
    int iFollowMax,
    sentient_s *pGoalSentient)
{
    sentient_s *sentient; // r27
    team_t eTeam; // r11
    pathnode_t *pDesiredChainPos; // r3
    pathnode_t *pActualChainPos; // r30
    gentity_s *ent; // r31
    double v13; // fp29
    double v14; // fp28
    double v15; // fp27
    double v16; // fp26
    pathnode_t *v17; // r11
    int v18; // r11
    pathnode_t *v19; // r11
    float v20; // [sp+60h] [-80h] BYREF
    float v21; // [sp+64h] [-7Ch]
    float v22; // [sp+68h] [-78h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5487, 0, "%s", "self");
    if (!pGoalSentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5488, 0, "%s", "pGoalSentient");
    if (iFollowMin > iFollowMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5489, 0, "%s", "iFollowMin <= iFollowMax");
    sentient = self->sentient;
    eTeam = sentient->eTeam;
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            5493,
            0,
            "%s",
            "sentient->eTeam == TEAM_AXIS || sentient->eTeam == TEAM_ALLIES || sentient->eTeam == TEAM_NEUTRAL");
    pDesiredChainPos = self->pDesiredChainPos;
    if (pDesiredChainPos && !sentient->pClaimedNode && !Path_CanClaimNode(pDesiredChainPos, sentient))
        self->pDesiredChainPos = 0;
    Sentient_UpdateActualChainPos(pGoalSentient);
    pActualChainPos = pGoalSentient->pActualChainPos;
    if (!pActualChainPos)
    {
        ent = self->ent;
        v13 = pGoalSentient->ent->r.currentOrigin[0];
        SL_ConvertToString(pGoalSentient->ent->classname);
        v14 = ent->r.currentOrigin[2];
        v15 = ent->r.currentOrigin[1];
        v16 = ent->r.currentOrigin[0];
        SL_ConvertToString(ent->classname);
        //Com_Error(ERR_DROP, byte_82021D00, HIDWORD(v16), HIDWORD(v15), v14, v13);
        Com_Error(ERR_DROP, "sentient following someone else not on a friendly chain"); // KISAKTODO: not proper
    }
    v17 = self->pDesiredChainPos;
    if (!v17
        || v17->constant.wChainId != pActualChainPos->constant.wChainId
        || (v18 = v17->constant.wChainDepth - pActualChainPos->constant.wChainDepth, v18 < iFollowMin)
        || v18 > iFollowMax
        || (Sentient_GetOrigin(sentient, &v20),
            v19 = self->pDesiredChainPos,
            (float)((float)((float)(v21 - v19->constant.vOrigin[1]) * (float)(v21 - v19->constant.vOrigin[1]))
                + (float)((float)((float)(v20 - v19->constant.vOrigin[0]) * (float)(v20 - v19->constant.vOrigin[0]))
                    + (float)((float)(v22 - v19->constant.vOrigin[2]) * (float)(v22 - v19->constant.vOrigin[2])))) > 4096.0))
    {
        self->pDesiredChainPos = Path_ChooseChainPos(pActualChainPos, iFollowMin, iFollowMax, self, self->chainFallback);
    }
}

void __cdecl Actor_UpdateDesiredChainPos(actor_s *self)
{
    sentient_s *sentient; // r27
    sentient_s *v3; // r28
    int iFollowMin; // r4
    int iFollowMax; // r5
    pathnode_t *pDesiredChainPos; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5540, 0, "%s", "self");
    sentient = self->sentient;
    if (!self->scriptGoalEnt.isDefined())
        goto LABEL_16;
    iassert(self->scriptGoalEnt.ent()->r.inuse);
    v3 = self->scriptGoalEnt.ent()->sentient;
    if (!v3)
        goto LABEL_16;
    iassert(sentient);
    if (v3->eTeam == Sentient_EnemyTeam(sentient->eTeam))
    {
        Actor_GetPerfectInfo(self, v3);
        goto LABEL_16;
    }
    iFollowMin = self->iFollowMin;
    iFollowMax = self->iFollowMax;
    if (iFollowMin > iFollowMax)
    {
    LABEL_16:
        self->pDesiredChainPos = 0;
        return;
    }
    Actor_UpdateDesiredChainPosInternal(self, iFollowMin, iFollowMax, v3);
    pDesiredChainPos = self->pDesiredChainPos;
    if (pDesiredChainPos && !sentient->pClaimedNode)
    {
        if (Path_CanClaimNode(pDesiredChainPos, sentient))
            Sentient_ClaimNode(sentient, pDesiredChainPos);
    }
}

void __cdecl Actor_CheckOverridePos(actor_s *self, const float *prevGoalPos)
{
    if (self->arrivalInfo.animscriptOverrideRunTo
        && (self->codeGoal.pos[0] != *prevGoalPos
            || self->codeGoal.pos[1] != prevGoalPos[1]
            || self->codeGoal.pos[2] != prevGoalPos[2]))
    {
        self->arrivalInfo.animscriptOverrideRunTo = 0;
    }
}

void __cdecl Actor_SetGoalRadius(actor_goal_s *goal, double radius)
{
    if (!goal)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5686, 0, "%s", "goal");
    if (radius < 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5687, 0, "%s", "radius >= 0");
    if (radius >= 4.0)
        goal->radius = radius;
    else
        goal->radius = 4.0;
}

void __cdecl Actor_SetGoalHeight(actor_goal_s *goal, double height)
{
    if (!goal)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5700, 0, "%s", "goal");
    if (height < 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5701, 0, "%s", "height >= 0");
    if (height >= 80.0)
        goal->height = height;
    else
        goal->height = 80.0;
}

bool __cdecl Actor_IsInsideArc(
    actor_s *self,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight)
{
    iassert(self);

    //return (_cntlzw(IsPosInsideArc(self->ent->r.currentOrigin, 15.0, origin, radius, angle0, angle1, halfHeight)) & 0x20) == 0;
    return IsPosInsideArc(self->ent->r.currentOrigin, 15.0, origin, radius, angle0, angle1, halfHeight);
}

void __cdecl SentientInfo_Copy(actor_s *pTo, const actor_s *pFrom, int index)
{
    char *v6; // r9
    char *v7; // r11

    if (!pTo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 153, 0, "%s", "pTo");
    if (!pFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 154, 0, "%s", "pFrom");
    if (((1 << pTo->species) & pFrom->talkToSpecies) != 0)
    {
        v6 = (char *)pTo + 40 * index;
        v7 = (char *)pFrom + 40 * index;
        if (*((unsigned int *)v6 + 529) < *((unsigned int *)v7 + 529))
        {
            *((unsigned int *)v6 + 528) = 0;
            *((unsigned int *)v6 + 530) = 0;
            *((unsigned int *)v6 + 529) = *((unsigned int *)v7 + 529);
            *((float *)v6 + 531) = *((float *)v7 + 531);
            *((float *)v6 + 532) = *((float *)v7 + 532);
            *((float *)v6 + 533) = *((float *)v7 + 533);
            *((unsigned int *)v6 + 534) = *((unsigned int *)v7 + 534);
        }
    }
}

actor_s *__cdecl Actor_Alloc()
{
    actor_s *actors; // r31
    int v1; // r11

    actors = level.actors;
    if (!level.actors)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 375, 0, "%s", "level.actors != NULL");
        actors = level.actors;
    }
    v1 = 0;
    while (actors->inuse)
    {
        ++v1;
        ++actors;
        if (v1 >= 32)
        {
            Com_DPrintf(18, "Actor allocation failed\n");
            return 0;
        }
    }
    memset(actors, 0, sizeof(actor_s));
    actors->inuse = 1;
    Actor_SetDefaults(actors);
    return actors;
}

void __cdecl Actor_Free(actor_s *actor)
{
    actor_s *actors; // r11
    gentity_s *ent; // r27
    gentity_s *v4; // r3
    gentity_s *v5; // r3
    int number; // r9
    int i; // r10
    actor_s *v8; // r11

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 404, 0, "%s", "actor");
    actors = level.actors;
    if (!level.actors)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 405, 0, "%s", "level.actors");
        actors = level.actors;
    }
    if (actor < actors || actor >= &actors[32])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            406,
            0,
            "%s",
            "actor >= level.actors && actor < level.actors + MAX_ACTORS");
        actors = level.actors;
    }
    if (&actors[actor - actors] != actor)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            407,
            0,
            "%s",
            "&level.actors[actor - level.actors] == actor");
    if (!actor->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 408, 0, "%s", "actor->sentient");
    if (!actor->inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 409, 0, "%s", "actor->inuse");
    ent = actor->ent;
    Actor_ClearPath(actor);
    Actor_StopUseTurret(actor);
    if (Scr_IsSystemActive())
        Actor_KillAnimScript(actor);
    if (actor->pGrenade.isDefined())
    {
        if (actor->pGrenade.ent()->activator == ent)
        {
            v4 = actor->pGrenade.ent();
            if (Actor_Grenade_InActorHands(v4))
            {
                v5 = actor->pGrenade.ent();
                G_FreeEntity(v5);
                iassert(!actor->pGrenade.isDefined());
            }
        }
    }
    Sentient_Dissociate(actor->sentient);
    number = ent->s.number;
    for (i = 0; i < 32; ++i)
    {
        v8 = &level.actors[i];
        if (level.actors[i].inuse)
        {
            if (v8->Path.wDodgeEntity == number)
                v8->Path.wDodgeEntity = 2175;
            if (v8->Physics.iHitEntnum == number)
                v8->Physics.iHitEntnum = 2175;
            if (v8->pPileUpActor == actor)
            {
                v8->pPileUpActor = 0;
                v8->pPileUpEnt = 0;
            }
        }
    }
    ent->actor = 0;
    Sentient_Free(actor->sentient);
    actor->sentient = 0;
    Scr_FreeActorFields(actor);
    memset(actor, 240, sizeof(actor_s));
    actor->inuse = 0;
}

void __cdecl Actor_FreeExpendable()
{
    gentity_s *v0; // r30
    actor_s *v1; // r28
    double v2; // fp31
    actor_s *Actor; // r31
    gentity_s *ent; // r11
    double v5; // fp30
    actor_s *v6; // r3
    gentity_s *v7; // r11
    actor_s *v8; // r3
    gentity_s *v9; // r11
    double v10; // fp0
    actor_s *v11; // r3
    gentity_s *v12; // r11
    gentity_s *v13; // r30
    sentient_s *sentient; // r29
    float v15; // [sp+50h] [-80h] BYREF
    float v16; // [sp+54h] [-7Ch]
    float v17; // [sp+58h] [-78h]
    float v18; // [sp+60h] [-70h] BYREF
    float v19; // [sp+64h] [-6Ch]
    float v20; // [sp+68h] [-68h]

    Com_Printf(18, "^3trying to delete somebody to make room for spawned AI (time %d)\n", level.time);
    if (level.loading)
        Com_Error(ERR_DROP, "too many actors in BSP file");
    v0 = G_Find(0, 284, scr_const.player);
    if (!v0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 500, 0, "%s", "player");
    if (!v0->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 501, 0, "%s", "player->client");
    if (!v0->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 502, 0, "%s", "player->sentient");
    Sentient_GetEyePosition(v0->sentient, &v15);
    v1 = 0;
    v2 = 0.0;
    Actor = Actor_FirstActor(-1);
    if (!Actor)
        goto LABEL_19;
    do
    {
        ent = Actor->ent;
        if ((Actor->ent->spawnflags & 4) == 0 && ent->s.number != level.currentEntityThink)
        {
            v5 = (float)((float)((float)(ent->r.currentOrigin[0] - v15) * (float)(ent->r.currentOrigin[0] - v15))
                + (float)((float)((float)(ent->r.currentOrigin[2] - v17) * (float)(ent->r.currentOrigin[2] - v17))
                    + (float)((float)(ent->r.currentOrigin[1] - v16) * (float)(ent->r.currentOrigin[1] - v16))));
            if (v2 < v5 && !PointCouldSeeSpawn(&v15, ent->r.currentOrigin, v0->s.number, ent->s.number))
            {
                v2 = v5;
                v1 = Actor;
            }
        }
        Actor = Actor_NextActor(Actor, -1);
    } while (Actor);
    if (!v1)
    {
        if (v2 != 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 530, 0, "%s", "fMaxDistSqrd == 0");
    LABEL_19:
        G_GetPlayerViewDirection(v0, &v18, 0, 0);
        v6 = Actor_FirstActor(-1);
        if (!v6)
            goto LABEL_27;
        do
        {
            v7 = v6->ent;
            if ((v6->ent->spawnflags & 4) == 0
                && v7->s.number != level.currentEntityThink
                && (float)((float)(v18 * (float)(v7->r.currentOrigin[0] - v15))
                    + (float)((float)(v20 * (float)(v7->r.currentOrigin[2] - v17))
                        + (float)(v19 * (float)(v7->r.currentOrigin[1] - v16)))) < 0.0
                && v2 < (float)((float)((float)(v7->r.currentOrigin[0] - v15) * (float)(v7->r.currentOrigin[0] - v15))
                    + (float)((float)((float)(v7->r.currentOrigin[2] - v17) * (float)(v7->r.currentOrigin[2] - v17))
                        + (float)((float)(v7->r.currentOrigin[1] - v16) * (float)(v7->r.currentOrigin[1] - v16)))))
            {
                v2 = (float)((float)((float)(v7->r.currentOrigin[0] - v15) * (float)(v7->r.currentOrigin[0] - v15))
                    + (float)((float)((float)(v7->r.currentOrigin[2] - v17) * (float)(v7->r.currentOrigin[2] - v17))
                        + (float)((float)(v7->r.currentOrigin[1] - v16) * (float)(v7->r.currentOrigin[1] - v16))));
                v1 = v6;
            }
            v6 = Actor_NextActor(v6, -1);
        } while (v6);
        if (!v1)
        {
        LABEL_27:
            if (v2 != 0.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 556, 0, "%s", "fMaxDistSqrd == 0");
            v8 = Actor_FirstActor(-1);
            if (!v8)
                goto LABEL_37;
            do
            {
                v9 = v8->ent;
                if ((v8->ent->spawnflags & 4) == 0
                    && v9->s.number != level.currentEntityThink
                    && v2 < (float)((float)((float)(v9->r.currentOrigin[0] - v15) * (float)(v9->r.currentOrigin[0] - v15))
                        + (float)((float)((float)(v9->r.currentOrigin[2] - v17) * (float)(v9->r.currentOrigin[2] - v17))
                            + (float)((float)(v9->r.currentOrigin[1] - v16) * (float)(v9->r.currentOrigin[1] - v16)))))
                {
                    v10 = (float)((float)(v18 * (float)(v9->r.currentOrigin[0] - v15))
                        + (float)((float)(v20 * (float)(v9->r.currentOrigin[2] - v17))
                            + (float)(v19 * (float)(v9->r.currentOrigin[1] - v16))));
                    if ((float)((float)v10 * (float)v10) < (double)(float)((float)((float)((float)(v9->r.currentOrigin[0] - v15)
                        * (float)(v9->r.currentOrigin[0] - v15))
                        + (float)((float)((float)(v9->r.currentOrigin[2]
                            - v17)
                            * (float)(v9->r.currentOrigin[2]
                                - v17))
                            + (float)((float)(v9->r.currentOrigin[1]
                                - v16)
                                * (float)(v9->r.currentOrigin[1]
                                    - v16))))
                        * (float)0.5))
                    {
                        v2 = (float)((float)((float)(v9->r.currentOrigin[0] - v15) * (float)(v9->r.currentOrigin[0] - v15))
                            + (float)((float)((float)(v9->r.currentOrigin[2] - v17) * (float)(v9->r.currentOrigin[2] - v17))
                                + (float)((float)(v9->r.currentOrigin[1] - v16) * (float)(v9->r.currentOrigin[1] - v16))));
                        v1 = v8;
                    }
                }
                v8 = Actor_NextActor(v8, -1);
            } while (v8);
            if (!v1)
            {
            LABEL_37:
                if (v2 != 0.0)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 583, 0, "%s", "fMaxDistSqrd == 0");
                G_GetPlayerViewDirection(v0, &v18, 0, 0);
                v11 = Actor_FirstActor(-1);
                if (!v11)
                    goto LABEL_46;
                do
                {
                    v12 = v11->ent;
                    if ((v11->ent->spawnflags & 4) == 0
                        && v12->s.number != level.currentEntityThink
                        && v2 <= (float)((float)((float)(v12->r.currentOrigin[0] - v15) * (float)(v12->r.currentOrigin[0] - v15))
                            + (float)((float)((float)(v12->r.currentOrigin[2] - v17)
                                * (float)(v12->r.currentOrigin[2] - v17))
                                + (float)((float)(v12->r.currentOrigin[1] - v16)
                                    * (float)(v12->r.currentOrigin[1] - v16)))))
                    {
                        v2 = (float)((float)((float)(v12->r.currentOrigin[0] - v15) * (float)(v12->r.currentOrigin[0] - v15))
                            + (float)((float)((float)(v12->r.currentOrigin[2] - v17) * (float)(v12->r.currentOrigin[2] - v17))
                                + (float)((float)(v12->r.currentOrigin[1] - v16) * (float)(v12->r.currentOrigin[1] - v16))));
                        v1 = v11;
                    }
                    v11 = Actor_NextActor(v11, -1);
                } while (v11);
                if (!v1)
                    LABEL_46:
                Com_Error(ERR_DROP, "Tried to force spawning of an AI when all AI slots are used by undeletable AI");
            }
        }
    }
    Com_Printf(18, "^3deleting entity %i\n", v1->ent->s.number);
    v13 = v1->ent;
    sentient = v1->sentient;
    if (v1->Path.wPathLen)
    {
        Path_AddTrimmedAmount(&v1->Path, v13->r.currentOrigin);
        Path_Clear(&v1->Path);
        v1->iTeamMoveDodgeTime = 0;
    }
    Scr_Notify(v1->ent, scr_const.death, 0);
    G_FreeEntity(v1->ent);
    if (v13->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 624, 0, "%s", "ent->actor == NULL");
    if (v13->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 625, 0, "%s", "ent->sentient == NULL");
    if (v1->inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 627, 0, "%s", "!pExpendable->inuse");
    if (sentient->inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 628, 0, "%s", "!sentient->inuse");
}

void __cdecl Actor_FinishSpawningAll()
{
    gentity_s *v0; // r28
    int i; // r22
    int eType; // r11
    const char *v3; // r31
    unsigned int *DataForFile; // r31
    unsigned __int16 v5; // r3
    int v6; // r3
    unsigned __int16 v7; // r3
    actor_s *j; // r31

    v0 = g_entities;
    for (i = 0; i < level.num_entities; ++v0)
    {
        if (v0->r.inuse)
        {
            eType = v0->s.eType;
            if (eType == 14 || eType == 15)
            {
                v3 = SL_ConvertToString(v0->classname);
                if (strncmp(v3, "actor_", 6u))
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                        825,
                        0,
                        "%s",
                        "!strncmp( classname, ACTOR_CLASSNAME_PREFIX, ACTOR_CLASSNAME_PREFIX_LEN )");
                DataForFile = (unsigned int*)Hunk_FindDataForFile(0, v3 + 6);
                if (!DataForFile)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 829, 0, "%s", "typeScript");
                if (!*DataForFile)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 830, 0, "%s", "typeScript->main");
                if (!DataForFile[2])
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 831, 0, "%s", "typeScript->spawner");
                if (v0->s.eType == 15)
                {
                    v5 = Scr_ExecEntThread(v0, DataForFile[2], 0);
                    Scr_FreeThread(v5);
                }
                v6 = DataForFile[1];
                if (v6)
                {
                    v7 = Scr_ExecThread(v6, 0);
                    Scr_FreeThread(v7);
                    DataForFile[1] = 0;
                }
            }
        }
        ++i;
    }
    for (j = Actor_FirstActor(-1); j; j = Actor_NextActor(j, -1))
    {
        Actor_FinishSpawning(j);
        Actor_InitAnimScript(j);
    }
}

void __cdecl Actor_DissociateSentient(actor_s *self, sentient_s *other, team_t eOtherTeam)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 860, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 861, 0, "%s", "self->sentient");
    if (self->sentient == other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 862, 0, "%s", "self->sentient != other");
    if (other->eTeam != TEAM_DEAD)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 863, 0, "%s", "other->eTeam == TEAM_DEAD");
    SentientInfo_Clear(&self->sentientInfo[other - level.sentients]);
    Actor_DissociateSuppressor(self, other);
    if (other == Actor_GetTargetSentient(self))
        Sentient_SetEnemy(self->sentient, 0, 1);
}

void __cdecl Actor_NodeClaimRevoked(actor_s *self, int invalidTime)
{
    pathnode_t *pClaimedNode; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 937, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 938, 0, "%s", "self->sentient");
    pClaimedNode = self->sentient->pClaimedNode;
    if (pClaimedNode)
    {
        if (Path_Exists(&self->Path))
        {
            if (Actor_PointAt(self->Path.vFinalGoal, pClaimedNode->constant.vOrigin))
                Actor_ClearPath(self);
        }
        Path_MarkNodeInvalid(pClaimedNode, self->sentient->eTeam);
    }
}

void __cdecl Actor_CheckClearNodeClaimCloseEnt(actor_s *self)
{
    pathnode_t *pClaimedNode; // r29
    gentity_s *v3; // r3

    pClaimedNode = self->sentient->pClaimedNode;
    if (pClaimedNode && self->pCloseEnt.isDefined())
    {
        v3 = self->pCloseEnt.ent();
        if (Actor_PointNearNode(v3->r.currentOrigin, pClaimedNode))
            Actor_NodeClaimRevoked(self, 1000);
    }
}

int __cdecl Actor_KeepClaimedNode(actor_s *self)
{
    pathnode_t *pClaimedNode; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 974, 0, "%s", "self");
    if (self->keepClaimedNode)
        return 1;
    if (self->keepClaimedNodeInGoal)
    {
        pClaimedNode = self->sentient->pClaimedNode;
        if (pClaimedNode && (unsigned __int8)Actor_PointAtGoal(pClaimedNode->constant.vOrigin, &self->codeGoal))
            return 1;
        self->keepClaimedNodeInGoal = 0;
    }
    return 0;
}

void __cdecl Actor_ClearKeepClaimedNode(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1003, 0, "%s", "self");
    self->keepClaimedNode = 0;
    self->keepClaimedNodeInGoal = 0;
    self->arrivalInfo.animscriptOverrideRunTo = 0;
    self->arrivalInfo.arrivalNotifyRequested = 0;
}

void __cdecl Actor_CheckNodeClaim(actor_s *self)
{
    float *vFinalGoal; // r29
    pathnode_t *pClaimedNode; // r4
    const pathnode_t *v4; // r3
    pathnode_t *v5; // r30

    if (self->Physics.bIsAlive
        && !self->fixedNode
        && !self->arrivalInfo.animscriptOverrideRunTo
        && !(unsigned __int8)Actor_KeepClaimedNode(self))
    {
        if (Actor_HasPath(self))
            vFinalGoal = self->Path.vFinalGoal;
        else
            vFinalGoal = self->ent->r.currentOrigin;
        pClaimedNode = self->sentient->pClaimedNode;
        if (pClaimedNode)
        {
            if (Actor_PointNearNode(vFinalGoal, pClaimedNode) || level.time < self->iTeamMoveWaitTime)
                return;
            Sentient_ClaimNode(self->sentient, 0);
        }
        v4 = Sentient_NearestNode(self->sentient);
        v5 = (pathnode_t *)v4;
        if (v4 && Path_IsCoverNode(v4) && Path_IsValidClaimNode(v5) && Actor_PointNearNode(vFinalGoal, v5))
        {
            if (Path_CanClaimNode(v5, self->sentient))
                Sentient_ClaimNode(self->sentient, v5);
            else
                self->sentient->bNearestNodeValid = 0;
        }
    }
}

void __cdecl Actor_UpdatePlayerPush(actor_s *self, gentity_s *player)
{
    gclient_s *client; // r11
    gentity_s *ent; // r5
    double v6; // fp13
    double v7; // fp12
    double v8; // fp0
    float v9[4]; // [sp+50h] [-50h] BYREF
    float v10[16]; // [sp+60h] [-40h] BYREF

    iassert(self);
    iassert(self->ent);
    iassert(self->sentient);
    iassert(player);
    iassert(player->client);
    iassert(player->sentient);

    if (!self->bDontAvoidPlayer
        && (self->Physics.iTraceMask & 0x2000000) != 0
        && self->eState[self->stateLevel] != AIS_TURRET
        && ((1 << self->sentient->eTeam) & ~(1 << Sentient_EnemyTeam(player->sentient->eTeam))) != 0)
    {
        client = player->client;
        ent = self->ent;
        v6 = (float)((float)(client->ps.velocity[0] * (float)0.1) + player->r.currentOrigin[0]);
        v7 = (float)((float)(client->ps.velocity[1] * (float)0.1) + player->r.currentOrigin[1]);
        v8 = (float)((float)(client->ps.velocity[2] * (float)0.1) + player->r.currentOrigin[2]);
        v9[0] = (float)-15.0 + (float)((float)(client->ps.velocity[0] * (float)0.1) + player->r.currentOrigin[0]);
        v10[0] = (float)15.0 + (float)v6;
        v9[1] = (float)-15.0 + (float)v7;
        v9[2] = (float)0.0 + (float)v8;
        v10[1] = (float)15.0 + (float)v7;
        v10[2] = (float)70.0 + (float)v8;
        if (SV_EntityContact(v9, v10, ent))
        {
            if (Actor_AtClaimNode(self))
            {
                if (player->client->playerMoved)
                    return;
                Sentient_StealClaimNode(player->sentient, self->sentient);
            }
            self->pCloseEnt.setEnt(player);
        }
    }
}

void __cdecl Actor_UpdateCloseEnt(actor_s *self)
{
    actor_s *actor; // r29
    gentity_s *v3; // r3
    EntHandle *p_pCloseEnt; // r29
    gentity_s *Player; // r29
    gentity_s *v6; // r3
    gentity_s *v7; // r3

    iassert(self);

    if (self->pCloseEnt.isDefined())
    {
        actor = self->pCloseEnt.ent()->actor;
        v3 = self->pCloseEnt.ent();
        if (Vec2DistanceSq(self->ent->r.currentOrigin, v3->r.currentOrigin) >= 1406.25)
        {
            if (actor)
            {
                p_pCloseEnt = &actor->pCloseEnt;
                if (p_pCloseEnt->isDefined())
                {
                    if (p_pCloseEnt->ent() == self->ent)
                        p_pCloseEnt->setEnt(NULL);
                }
            }
            goto LABEL_11;
        }
        if (actor && actor->pPileUpEnt == self->ent)
            LABEL_11:
        self->pCloseEnt.setEnt(NULL);
    }
    Player = G_GetPlayer();
    if (self->pCloseEnt.isDefined())
    {
        if (self->pCloseEnt.ent() != Player)
            Actor_UpdatePlayerPush(self, Player);

        iassert(self->pCloseEnt.isDefined());

        v6 = self->pCloseEnt.ent();
        if (Vec2DistanceSq(self->ent->r.currentOrigin, v6->r.currentOrigin) >= 900.0
            && !self->pPileUpActor
            && self->eAnimMode == AI_ANIM_MOVE_CODE
            && Actor_HasPath(self))
        {
            v7 = self->pCloseEnt.ent();
            if ((float)((float)(self->Path.lookaheadDir[1] * (float)(self->ent->r.currentOrigin[1] - v7->r.currentOrigin[1]))
                + (float)(self->Path.lookaheadDir[0] * (float)(self->ent->r.currentOrigin[0] - v7->r.currentOrigin[0]))) > 0.0)
                self->pCloseEnt.setEnt(NULL);
        }
    }
}

actor_think_result_t __cdecl Actor_CallThink(actor_s *self)
{
    unsigned int stateLevel; // r7
    ai_state_t v3; // r8
    actor_think_result_t v4; // r27
    int wPathLen; // r11
    double fCurrLength; // fp1
    double v7; // fp2
    const char *v8; // r3
    int *v9; // r11
    float *v10; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1225, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1226, 0, "%s", "self->sentient");
    stateLevel = self->stateLevel;
    if (stateLevel >= 5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1227,
            0,
            "self->stateLevel doesn't index ARRAY_COUNT( self->eState )\n\t%i not in [0, %i)",
            stateLevel,
            5);
    v3 = self->eState[self->stateLevel];
    if (!AIFuncTable[self->species][v3].pfnThink)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1228,
            0,
            "%s\n\t(self->eState[self->stateLevel]) = %i",
            "(AIFuncTable[self->species][self->eState[self->stateLevel]].pfnThink)",
            v3);
    Actor_ValidateReacquireNodes(self);
    if (self->transitionCount)
    {
        Actor_ThinkStateTransitions(self);
        if (self->transitionCount)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1240, 0, "%s", "self->transitionCount == 0");
    }
    Actor_UpdateCloseEnt(self);
    v4 = AIFuncTable[self->species][self->eState[self->stateLevel]].pfnThink(self);
    if ((unsigned int)v4 > ACTOR_THINK_MOVE_TO_BODY_QUEUE)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1247,
            0,
            "%s",
            "eThinkResult == ACTOR_THINK || eThinkResult == ACTOR_THINK_REPEAT || eThinkResult == ACTOR_THINK_MOVE_TO_BODY_QUEUE");
    wPathLen = self->Path.wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = self->Path.fCurrLength;
        v7 = *(float *)&self->Physics.iTouchEnts[7 * wPathLen + 29];
        if (fCurrLength > v7)
        {
            v8 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v7));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                1249,
                0,
                "%s\n\t%s",
                "self->Path.wPathLen <= 1 || self->Path.fCurrLength <= self->Path.pts[self->Path.wPathLen - 2].fOrigLength",
                v8);
        }
    }
    if (Path_HasNegotiationNode(&self->Path))
    {
        v9 = (int *)((char *)self + 28 * self->Path.wNegotiationStartNode);
        if (v9[205] < 0 || v9[198] < 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                1250,
                0,
                "%s",
                "!Path_HasNegotiationNode( &self->Path ) || (self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum >= 0 && se"
                "lf->Path.pts[self->Path.wNegotiationStartNode - 1].iNodeNum >= 0)");
    }
    if (self->Path.wPathLen)
    {
        if (self->Path.wNegotiationStartNode == self->Path.wPathLen - 1)
        {
            v10 = (float *)((char *)self + 28 * self->Path.wNegotiationStartNode);
            if (v10[199] != self->Path.vCurrPoint[0]
                || v10[200] != self->Path.vCurrPoint[1]
                || v10[201] != self->Path.vCurrPoint[2])
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    1251,
                    0,
                    "%s",
                    "!self->Path.wPathLen || !Path_AtEndOrNegotiation( &self->Path ) || Vec3Compare( self->Path.pts[self->Path.wNeg"
                    "otiationStartNode].vOrigPoint, self->Path.vCurrPoint )");
            }
        }
    }
    if (self->Path.wPathLen && self->Path.lookaheadNextNode >= self->Path.wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1252,
            0,
            "%s",
            "!self->Path.wPathLen || (self->Path.lookaheadNextNode < self->Path.wPathLen)");
    if (self->Path.wPathLen
        && self->Path.fLookaheadDistToNextNode != 0.0
        && self->Path.lookaheadNextNode >= self->Path.wPathLen - 1)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1253,
            0,
            "%s",
            "!self->Path.wPathLen || !self->Path.fLookaheadDistToNextNode || (self->Path.lookaheadNextNode < self->Path.wPathLen - 1)");
    }
    Actor_ValidateReacquireNodes(self);
    return v4;
}

int endTime;
int direction;
static const float colorTeam[5][4] =
{
  { 1.0f, 1.0f, 1.0f, 1.0f },
  { 1.0f, 0.4f, 0.4f, 1.0f },
  { 0.5f, 0.5f, 1.0f, 1.0f },
  { 0.0f, 1.0f, 0.0f, 1.0f },
  { 0.5f, 0.5f, 0.5f, 1.0f }
};

void __cdecl Actor_EntInfo(gentity_s *self, float *source)
{
    unsigned __int8 v4; // r29
    char v5; // r19
    int integer; // r11
    actor_s *actor; // r31
    gentity_s *TargetEntity; // r18
    const sentient_s *TargetSentient; // r30
    int v10; // r24
    char *v11; // r11
    const float *v12; // r29
    const float *v13; // r5
    const float *currentOrigin; // r4
    double v15; // fp0
    double v16; // fp13
    double v17; // fp12
    double value; // fp11
    double v19; // fp31
    const float *v20; // r6
    double v21; // fp25
    double v22; // fp12
    double v23; // fp30
    double v24; // fp1
    double v25; // fp1
    int v26; // r6
    int v27; // r5
    const float *v28; // r4
    double v29; // fp2
    double v30; // fp3
    gentity_s *v31; // r3
    gentity_s *v32; // r3
    const float *v33; // r4
    gentity_s *v34; // r3
    WeaponDef *WeaponDef; // r28
    __int64 v36; // r11
    double v37; // fp31
    gentity_s *v38; // r3
    const float *v39; // r4
    int v40; // r11
    __int64 v41; // r11
    double v42; // fp0
    const float *v43; // r4
    const float *v44; // r4
    int number; // r11
    int v46; // r10
    gentity_s *v47; // r3
    const float *v48; // r4
    aiGoalSources codeGoalSrc; // r11
    double v50; // fp0
    gentity_s *volume; // r3
    int v52; // r11
    int v53; // r10
    pathnode_t *pDesiredChainPos; // r11
    const float *v55; // r5
    unsigned int eTeam; // r10
    const float *v57; // r29
    int v58; // r4
    const char *v59; // r5
    const float *v60; // r4
    double v61; // fp1
    const dvar_s *v62; // r11
    const char *v63; // r3
    const char *v64; // r5
    double v65; // fp31
    const char *v66; // r5
    const char *v67; // r5
    int v68; // r25
    const char *v69; // r5
    const char *v70; // r5
    double v71; // fp28
    int health; // r4
    const char *v73; // r5
    const float *v74; // r30
    int v75; // r4
    const char *v76; // r5
    const char *v77; // r5
    unsigned int missCount; // r11
    unsigned int hitCount; // r7
    const char *v80; // r6
    const char *v81; // r5
    const char *v82; // r5
    const char *v83; // r5
    scr_animscript_t *pAnimScriptFunc; // r11
    const char *v85; // r30
    const char *v86; // r3
    const char *v87; // r5
    const char *v88; // r30
    const char *v89; // r3
    const char *v90; // r5
    ai_orient_mode_t eMode; // r10
    const float *v92; // r30
    ai_orient_mode_t v93; // r9
    const char *v94; // r5
    const char *v95; // r5
    const char *v96; // r5
    const char *v97; // r5
    const char *v98; // r5
    gentity_s *v99; // r3
    const char *v100; // r5
    const char *v101; // r5
    unsigned int eAnimMode; // r7
    const char *v103; // r5
    float v104; // [sp+50h] [-1A0h] BYREF
    float v105; // [sp+54h] [-19Ch]
    float v106; // [sp+58h] [-198h]
    float v107; // [sp+60h] [-190h] BYREF
    float v108; // [sp+64h] [-18Ch]
    float v109; // [sp+68h] [-188h]
    float v110; // [sp+70h] [-180h] BYREF
    float v111; // [sp+74h] [-17Ch]
    float v112; // [sp+78h] [-178h]
    float v113; // [sp+7Ch] [-174h]
    float v114; // [sp+80h] [-170h] BYREF
    float v115; // [sp+84h] [-16Ch]
    float v116; // [sp+88h] [-168h]
    float v117; // [sp+90h] [-160h] BYREF
    float v118; // [sp+94h] [-15Ch]
    float v119; // [sp+98h] [-158h]
    float v120; // [sp+A0h] [-150h] BYREF
    float v121; // [sp+A4h] [-14Ch]
    float v122; // [sp+A8h] [-148h]
    float v123; // [sp+ACh] [-144h]
    __int64 v124; // [sp+B0h] [-140h] BYREF
    const char *v125; // [sp+BCh] [-134h]
    float v126; // [sp+C0h] [-130h] BYREF
    float v127; // [sp+C4h] [-12Ch]
    float v128; // [sp+C8h] [-128h]
    float v129[4]; // [sp+D0h] [-120h] BYREF
    float v130; // [sp+E0h] [-110h] BYREF
    float v131; // [sp+E4h] [-10Ch]
    float v132; // [sp+E8h] [-108h]
    float v133[4]; // [sp+F0h] [-100h] BYREF
    float v134[22]; // [sp+100h] [-F0h] BYREF

    v4 = 1;
    v5 = 0;
    v125 = "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp";
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1811, 0, "%s", "self");
    if (!SV_DObjExists(self))
        return;
    integer = g_entinfo->current.integer;
    if (integer == 3)
        v4 = 0;
    if (integer == 4 || integer == 5)
    {
        v4 = 0;
        v5 = 1;
    }
    actor = self->actor;
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1825, 0, "%s", "actor");
    TargetEntity = Actor_GetTargetEntity(actor);
    TargetSentient = Actor_GetTargetSentient(actor);
    Sentient_GetDebugEyePosition(actor->ent->sentient, &v104);
    v10 = v4;
    if (v4)
    {
        if (TargetSentient)
        {
            v11 = (char *)actor + 40 * (TargetSentient - level.sentients);
            if (v11[2100])
            {
                v12 = colorGreen;
                if (level.time - *((unsigned int *)v11 + 526) > 250)
                    v12 = colorYellow;
            }
            else
            {
                v12 = colorRed;
            }
            Sentient_GetDebugEyePosition(TargetSentient, v134);
            v13 = v12;
            currentOrigin = v134;
            goto LABEL_20;
        }
        if (TargetEntity)
        {
            v13 = colorBlue;
            currentOrigin = TargetEntity->r.currentOrigin;
        LABEL_20:
            G_DebugLine(&v104, currentOrigin, v13, 1);
        }
    }
    v15 = (float)(*source - self->r.currentOrigin[0]);
    v16 = (float)(source[2] - self->r.currentOrigin[2]);
    v17 = (float)(source[1] - self->r.currentOrigin[1]);
    value = g_entinfo_maxdist->current.value;
    //v19 = sqrtf((float)((float)((float)v17 * (float)v17) + (float)((float)((float)v16 * (float)v16) + (float)((float)v15 * (float)v15))));
    v19 = sqrtf((float)((float)((float)v17 * (float)v17) + (float)((float)((float)v16 * (float)v16) + (float)((float)v15 * (float)v15))));
    if (value > 0.0 && v19 > value)
        return;
    v21 = (float)((float)(G_GetEntInfoScale() * (float)v19) * (float)0.0026041667);
    if (g_entinfo->current.integer != 3)
        G_DebugBox(self->r.currentOrigin, self->r.mins, self->r.maxs, self->r.currentAngles[1], v20, (int)colorMagenta, 1);
    v114 = actor->eyeInfo.dir[0];
    v115 = actor->eyeInfo.dir[1];
    v22 = actor->eyeInfo.dir[2];
    v114 = (float)(v114 * (float)48.0) + v104;
    v115 = (float)(v115 * (float)48.0) + v105;
    v116 = (float)((float)v22 * (float)48.0) + v106;
    if (v10)
        G_DebugLine(&v104, &v114, colorBlue, 1);
    if (ai_debugEntIndex->current.integer == self->s.number)
        DebugDrawNodeSelectionOverlay();
    v126 = (float)(actor->Path.lookaheadDir[0] * (float)48.0) + v104;
    v127 = (float)(actor->Path.lookaheadDir[1] * (float)48.0) + v105;
    v128 = v106;
    G_DebugLine(&v104, &v126, colorMagenta, 1);
    v126 = (float)(actor->prevMoveDir[0] * (float)48.0) + v104;
    v127 = (float)(actor->prevMoveDir[1] * (float)48.0) + v105;
    v128 = v106;
    G_DebugLine(&v104, &v126, colorOrange, 1);
    if (v10)
    {
        if (actor->lookAtInfo.bDoLookAt || actor->lookAtInfo.fLookAtTurnAngle != 0.0)
        {
            v129[0] = actor->lookAtInfo.vLookAtPos[0] - v104;
            v129[1] = actor->lookAtInfo.vLookAtPos[1] - v105;
            v23 = self->r.currentAngles[1];
            v129[2] = actor->lookAtInfo.vLookAtPos[2] - v106;
            vectoangles(v129, (float *)&v124);
            v120 = 1.0;
            v121 = 1.0;
            v122 = 0.25;
            v107 = actor->lookAtInfo.vLookAtPos[0];
            v108 = actor->lookAtInfo.vLookAtPos[1];
            v109 = actor->lookAtInfo.vLookAtPos[2] - (float)2.0;
            v123 = 0.5;
            G_DebugLine(&v104, &v107, &v120, 1);
            v24 = (float)((float)v23 - actor->lookAtInfo.fLookAtTurnAngle);
            v123 = 0.75;
            v25 = AngleNormalize360(v24);
            v107 = 0.0;
            v109 = 0.0;
            v108 = v25;
            AngleVectors(&v107, &v114, 0, 0);
            v107 = (float)(v114 * (float)20.0) + v104;
            v108 = (float)(v115 * (float)20.0) + v105;
            v109 = (float)(v116 * (float)20.0) + v106;
            G_DebugLine(&v104, &v107, &v120, 1);
            v120 = 0.5;
            v121 = 0.5;
            v122 = 0.0;
            v123 = 0.75;
            v107 = 0.0;
            v108 = *((float *)&v124 + 1);
            v109 = 0.0;
            AngleVectors(&v107, &v114, 0, 0);
            v107 = (float)(v114 * (float)20.0) + v104;
            v108 = (float)(v115 * (float)20.0) + v105;
            v109 = (float)(v116 * (float)20.0) + v106;
            G_DebugLine(&v104, &v107, &v120, 1);
            v107 = 0.0;
            v108 = v23;
            v109 = 0.0;
            AngleVectors(&v107, &v114, 0, 0);
            v107 = (float)(v114 * (float)20.0) + v104;
            v108 = (float)(v115 * (float)20.0) + v105;
            v109 = (float)(v116 * (float)20.0) + v106;
            G_DebugLine(&v104, &v107, &v120, 1);
            if (AngleSubtract(v23, *((float *)&v124 + 1)) <= 0.0)
            {
                v30 = *((float *)&v124 + 1);
                v29 = v23;
            }
            else
            {
                v29 = *((float *)&v124 + 1);
                v30 = v23;
            }
            G_DebugArc(&v104, 16.0, v29, v30, v28, v27, v26);
        }
        if (actor->pGrenade.isDefined())
        {
            v31 = actor->pGrenade.ent();
            G_DebugLine(&v104, v32->missile.predictLandPos, colorOrange, 1);
            v32 = actor->pGrenade.ent();
            G_DebugCircle(v32->missile.predictLandPos, 8.0, colorOrange, 0, 1, 1);// KISAKTODO: argcheck
            v34 = actor->pGrenade.ent();
            WeaponDef = BG_GetWeaponDef(v34->s.weapon);
            if (!WeaponDef)
                MyAssertHandler(v125, 1956, 0, "%s", "weapDef");
            LODWORD(v36) = WeaponDef->iExplosionRadius;
            v124 = v36;
            v37 = (float)v36;
            v38 = actor->pGrenade.ent();
            G_DebugCircle(v38->missile.predictLandPos, v37, colorOrange, 0, 1, 1); // KISAKTODO: argcheck
        }
    }
    else if (!v5)
    {
        goto LABEL_80;
    }
    v113 = 1.0;
    v40 = endTime;
    if (level.time <= endTime)
    {
        if (endTime - level.time > 1000)
        {
            v40 = 0;
            endTime = 0;
        }
    }
    else
    {
        v40 = level.time + 1000;
        endTime = level.time + 1000;
        direction = direction == 0;
    }
    LODWORD(v41) = v40 - level.time;
    HIDWORD(v41) = direction;
    v124 = v41;
    v42 = (float)((float)v41 * (float)0.001);
    if (!direction)
        v42 = (float)((float)1.0 - (float)((float)(unsigned int)v41 * (float)0.001));
    v110 = v42;
    v111 = v42;
    v112 = v42;
    if (!actor->scriptGoalEnt.isDefined())
    {
        v117 = actor->scriptGoal.pos[0];
        v118 = actor->scriptGoal.pos[1];
        v119 = actor->scriptGoal.pos[2] + (float)16.0;
        if (!actor->pDesiredChainPos)
            G_DebugLine(&v104, &v117, &v110, 0);
        G_DebugCircle(&v117, actor->scriptGoal.radius, v43, (int)&v110, 0, 1);
    }
    if (actor->fixedNode)
    {
        v117 = actor->scriptGoal.pos[0];
        v118 = actor->scriptGoal.pos[1];
        v119 = actor->scriptGoal.pos[2] + (float)16.0;
        v112 = 0.0;
        v110 = 0.0;
        G_DebugLine(&v104, &v117, &v110, 0);
        G_DebugCircle(&v117, actor->fixedNodeSafeRadius, v44, (int)&v110, 0, 1);
        if (actor->fixedNodeSafeVolume.isDefined())
        {
            number = self->s.number;
            v46 = ai_showVolume->current.integer;
            if (v46 == number || v46 > 0 && ai_debugEntIndex->current.integer == number)
            {
                v47 = actor->fixedNodeSafeVolume.ent();
                G_DebugDrawBrushModel(v47, colorGreenFaded, 0, 0);
            }
        }
    }
    if (usingCodeGoal(actor))
    {
        v117 = actor->codeGoal.pos[0];
        v118 = actor->codeGoal.pos[1];
        v119 = actor->codeGoal.pos[2] + (float)16.0;
        G_DebugLine(&v104, &v117, colorMagenta, 0);
        codeGoalSrc = actor->codeGoalSrc;
        if (codeGoalSrc == AI_GOAL_SRC_SCRIPT_ENTITY_GOAL)
        {
            v110 = 0.0;
            v111 = 0.0;
            v112 = 1.0;
            v50 = 1.0;
        }
        else if (codeGoalSrc == AI_GOAL_SRC_FRIENDLY_CHAIN)
        {
            v110 = 0.0;
            v111 = 1.0;
            v112 = 0.0;
            v50 = 1.0;
        }
        else
        {
            if (codeGoalSrc == AI_GOAL_SRC_ENEMY)
            {
                v110 = 1.0;
                v111 = 0.0;
                v112 = 1.0;
            }
            else
            {
                v110 = 0.0;
                v111 = 0.0;
                v112 = 0.0;
            }
            v50 = 1.0;
        }
        v113 = v50;
        G_DebugCircle(&v117, actor->codeGoal.radius, v48, (int)&v110, 0, 1);
    }
    volume = actor->codeGoal.volume;
    if (volume)
    {
        v52 = self->s.number;
        v53 = ai_showVolume->current.integer;
        if (v53 == v52 || v53 > 0 && ai_debugEntIndex->current.integer == v52)
            G_DebugDrawBrushModel(volume, colorWhiteFaded, 1, 0);
        if (ai_showRegion->current.enabled)
            Actor_DebugDrawNodesInVolume(actor);
    }
    if (self->sentient->pClaimedNode
        && ai_showClaimedNode->current.enabled
        && !ai_debugCoverSelection->current.enabled
        && !ai_debugThreatSelection->current.enabled)
    {
        CL_GetViewPos((float *)&v124);
        G_DebugLine(&v104, self->sentient->pClaimedNode->constant.vOrigin, colorBlue, 0);
        Path_DrawDebugNode((const float *)&v124, self->sentient->pClaimedNode);
    }
LABEL_80:
    if (v10)
    {
        if (self->sentient->pClaimedNode)
        {
            pDesiredChainPos = actor->pDesiredChainPos;
            if (pDesiredChainPos)
            {
                v55 = colorYellow;
            LABEL_86:
                G_DebugLine(&v104, pDesiredChainPos->constant.vOrigin, v55, 0);
                goto LABEL_87;
            }
        }
        pDesiredChainPos = actor->pDesiredChainPos;
        if (pDesiredChainPos)
        {
            v55 = colorCyan;
            goto LABEL_86;
        }
    }
LABEL_87:
    eTeam = actor->sentient->eTeam;
    if (eTeam > 4)
        v57 = colorYellow;
    else
        v57 = colorTeam[eTeam];
    if (v5)
    {
        v58 = self->s.number;
        v106 = (float)((float)v21 * (float)3.5) + v106;
        va("%i", v58);
        v60 = v57;
        v61 = (float)((float)v21 * (float)0.60000002);
    LABEL_171:
        G_AddDebugString(&v104, v60, v61, v59);
        return;
    }
    v106 = (float)((float)v21 * (float)70.0) + v106;
    if (ai_debugAccuracy->current.enabled && ai_debugEntIndex->current.integer == self->s.number)
    {
        Actor_DrawDebugAccuracy(&v104, (float)((float)v21 * (float)0.60000002), (float)((float)v21 * (float)7.0));
        return;
    }
    CL_GetViewForward(v133);
    v104 = (float)((float)((float)v21 * (float)-70.0) * v133[1]) + v104;
    v105 = (float)((float)((float)v21 * (float)-70.0) * (float)-v133[0]) + v105;
    if (g_entinfo->current.integer == 2 || (v62 = g_entinfo_AItext, !g_entinfo_AItext->current.integer))
    {
        if (self->targetname)
            v69 = SL_ConvertToString(self->targetname);
        else
            v69 = "<noname>";
        v65 = (float)((float)v21 * (float)0.60000002);
        va("%i : %s", self->s.number, v69);
        G_AddDebugString(&v104, v57, v65, v70);
        //LOBYTE(v68) = 30;
        v68 = 30;
    }
    else
    {
        if (self->targetname)
        {
            v63 = SL_ConvertToString(self->targetname);
            v62 = g_entinfo_AItext;
            v64 = v63;
        }
        else
        {
            v64 = "<noname>";
        }
        v65 = (float)((float)v21 * (float)0.60000002);
        va("%i : %s (%s)", self->s.number, v64, g_entinfoAITextNames[v62->current.integer]);
        G_AddDebugString(&v104, v57, v65, v66);
        v68 = 1 << g_entinfo_AItext->current.integer;
    }
    if ((v68 & 6) != 0)
    {
        v71 = 0.0;
        if ((v68 & 4) != 0)
        {
            health = self->health;
            v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
            va("health: %i", health);
            G_AddDebugString(&v104, v57, v65, v73);
        }
        if (TargetEntity)
        {
            Sentient_GetOrigin(actor->sentient, &v130);
            v74 = colorRed;
            v71 = sqrtf((float)((float)((float)(v130 - TargetEntity->r.currentOrigin[0])
                * (float)(v130 - TargetEntity->r.currentOrigin[0]))
                + (float)((float)((float)(v132 - TargetEntity->r.currentOrigin[2])
                    * (float)(v132 - TargetEntity->r.currentOrigin[2]))
                    + (float)((float)(v131 - TargetEntity->r.currentOrigin[1])
                        * (float)(v131 - TargetEntity->r.currentOrigin[1])))));
        }
        else
        {
            v74 = colorYellow;
        }
        v106 = v106 - (float)((float)v21 * (float)7.0);
        if (TargetEntity)
        {
            if (TargetEntity->targetname)
                v76 = SL_ConvertToString(TargetEntity->targetname);
            else
                v76 = "<noname target>";
            v75 = TargetEntity->s.number;
        }
        else
        {
            v75 = 0;
            v76 = "no target";
        }
        va("%i : %s", v75, v76);
        G_AddDebugString(&v104, v74, v65, v77);
        if ((v68 & 4) != 0)
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            missCount = actor->missCount;
            hitCount = missCount;
            if (missCount)
            {
                v80 = "MISS";
            }
            else
            {
                hitCount = actor->hitCount;
                v80 = "HIT";
            }
            //va(
            //    range: %.2f ac: %.2f %s %u
            //    LODWORD(v71),
            //    (unsigned int)COERCE_UNSIGNED_INT64(actor->debugLastAccuracy),
            //    v80,
            //    hitCount);
            G_AddDebugString(&v104, colorWhite, v65, v81);
            v106 = v106 - (float)((float)v21 * (float)7.0);
            va("talkto: %d", actor->talkToSpecies);
            G_AddDebugString(&v104, colorWhite, v65, v82);
        }
    }
    if ((v68 & 0x10) != 0)
    {
        v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
        va(
            "(%i)%i:%i = %s",
            actor->stateLevel,
            actor->eState[actor->stateLevel],
            actor->eSubState[actor->stateLevel],
            actor->pszDebugInfo);
        G_AddDebugString(&v104, colorWhite, v65, v83);
    }
    if ((v68 & 0x12) != 0)
    {
        v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
        pAnimScriptFunc = actor->pAnimScriptFunc;
        if (pAnimScriptFunc)
            v85 = SL_ConvertToString(pAnimScriptFunc->name);
        else
            v85 = "<none>";
        v86 = SL_ConvertToString(actor->scriptState);
        va("%s [%s]", v85, v86);
        G_AddDebugString(&v104, colorWhite, v65, v87);
    }
    if ((v68 & 0x10) != 0)
    {
        v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
        v88 = SL_ConvertToString(actor->stateChangeReason);
        v89 = SL_ConvertToString(actor->lastScriptState);
        va("<-  %s [%s]", v89, v88);
        G_AddDebugString(&v104, colorWhite, v65, v90);
    }
    if ((v68 & 8) != 0)
    {
        v106 = v106 - (float)((float)v21 * (float)7.0);
        eMode = actor->ScriptOrient.eMode;
        if (eMode)
        {
            if (eMode == actor->CodeOrient.eMode)
                v92 = colorYellow;
            else
                v92 = colorRed;
        }
        else
        {
            v92 = colorGreen;
        }
        v93 = actor->CodeOrient.eMode;
        if (eMode)
            va("orient: %s (%s <- script)", ai_orient_mode_text[v93], ai_orient_mode_text[eMode]);
        else
            va("orient: %s", ai_orient_mode_text[v93]);
        G_AddDebugString(&v104, v92, v65, v94);
        if (Actor_HasPath(actor))
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            G_AddDebugString(&v104, colorWhite, v65, v95);
        }
        if (actor->Path.wPathLen > 0 && !Path_DistanceGreaterThan(&actor->Path, 128.0))
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            G_AddDebugString(&v104, colorWhite, v65, v96);
        }
        if (actor->pPileUpActor)
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            va("blockee: %d, blocker: %d", actor->pPileUpActor->ent->s.number, actor->pPileUpEnt->s.number);
            G_AddDebugString(&v104, colorWhite, v65, v97);
        }
        if (actor->pCloseEnt.isDefined())
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            v99 = actor->pCloseEnt.ent();
            va("closeEnt: %d", v99->s.number);
            G_AddDebugString(&v104, colorWhite, v65, v100);
        }
        if (actor->bDontAvoidPlayer)
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            G_AddDebugString(&v104, colorWhite, v65, v98);
        }
        if ((actor->Physics.iTraceMask & 0x2000000) == 0)
        {
            v106 = v106 - (float)((float)v21 * (float)7.0);
            G_AddDebugString(&v104, colorWhite, v65, v98);
        }
        v106 = v106 - (float)((float)v21 * (float)7.0);
        va("physics %d", actor->Physics.ePhysicsType);
        G_AddDebugString(&v104, colorWhite, v65, v101);
    }
    if ((v68 & 0xA) != 0)
    {
        eAnimMode = actor->eAnimMode;
        if (eAnimMode >= 0xA)
            MyAssertHandler(
                v125,
                2228,
                0,
                "actor->eAnimMode doesn't index ARRAY_COUNT( animModeNames )\n\t%i not in [0, %i)",
                eAnimMode,
                10);
        v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
        va("animmode %s script: %s", animModeNames[actor->eAnimMode], animModeNames[actor->eScriptSetAnimMode]);
        G_AddDebugString(&v104, colorWhite, v65, v103);
    }
    if ((v68 & 4) != 0)
    {
        if (!actor->ent->takedamage)
        {
            v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
            G_AddDebugString(&v104, colorRed, v65, v67);
        }
        if (actor->provideCoveringFire)
        {
            v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
            G_AddDebugString(&v104, colorWhite, v65, v67);
        }
        if (actor->ignoreSuppression)
        {
            v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
            G_AddDebugString(&v104, colorRed, v65, v67);
        }
        if (Actor_IsSuppressed(actor) || actor->suppressionMeter > 0.0)
        {
            v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
            //va(
            //    (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(actor->suppressionMeter)),
            //    (unsigned int)COERCE_UNSIGNED_INT64(actor->suppressionMeter));
            v61 = v65;
            v60 = colorRed;
        }
        else
        {
            if (!Actor_IsMoveSuppressed(actor))
                return;
            v60 = colorCyan;
            v106 = -(float)((float)((float)v21 * (float)7.0) - v106);
            v61 = v65;
        }
        goto LABEL_171;
    }
}

int __cdecl Actor_MoveAwayNoWorse(actor_s *self)
{
    int v2; // r26
    int i; // r28
    actor_s *v4; // r31

    v2 = 1;
    for (i = 0; i < 32; ++i)
    {
        v4 = &level.actors[i];
        if (level.actors[i].inuse
            && v4 != self
            && (!self->pCloseEnt.isDefined() || v4->ent != self->pCloseEnt.ent())
            && Vec2DistanceSq(self->Physics.vOrigin, v4->ent->r.currentOrigin) < 900.0
            && Vec2DistanceSq(self->ent->r.currentOrigin, v4->ent->r.currentOrigin) >= 900.0
            //&& I_fabs((float)(self->ent->r.currentOrigin[2] - v4->ent->r.currentOrigin[2])) < 80.0)
            && I_fabs((float)(self->ent->r.currentOrigin[2] - v4->ent->r.currentOrigin[2])) < 80.0)
        {
            if (!v4->pCloseEnt.isDefined() && !Actor_AtClaimNode(v4))
                v4->pCloseEnt.setEnt(self->ent);
            v2 = 0;
        }
    }
    return v2;
}

int __cdecl Actor_PhysicsCheckMoveAwayNoWorse(
    actor_s *self,
    gentity_s *other,
    int flags,
    double distanceSqrd,
    double lengthSqrd)
{
    actor_physics_t *p_Physics; // r30
    int result; // r3

    p_Physics = &self->Physics;
    if (Vec2DistanceSq(self->Physics.vOrigin, other->r.currentOrigin) <= distanceSqrd
        || Vec2DistanceSq(p_Physics->vOrigin, self->ent->r.currentOrigin) <= lengthSqrd
        || !Actor_MoveAwayNoWorse(self))
    {
        return 0;
    }
    result = 1;
    self->ent->flags &= 0xFFFFFFE7;
    self->ent->flags |= flags;
    return result;
}

int __cdecl Actor_PhysicsMoveAway(actor_s *self)
{
    gentity_s *v2; // r3
    gentity_s *ent; // r11
    actor_physics_t *p_Physics; // r30
    unsigned __int16 groundEntNum; // r27
    int bHasGroundPlane; // r26
    double groundplaneSlope; // fp26
    int iFootstepTimer; // r25
    double v9; // fp25
    double v10; // fp24
    gentity_s *v11; // r28
    double v12; // fp23
    double v13; // fp31
    double v14; // fp27
    double v15; // fp0
    double v16; // fp31
    double v17; // fp11
    double v18; // fp30
    int result; // r3
    int i; // r24
    int LeftOrRightDodge; // r3
    int v22; // r29
    gentity_s *v23; // r11
    actor_s *actor; // r11
    gentity_s *v25; // r11
    float v26; // [sp+50h] [-B0h] BYREF
    float v27; // [sp+54h] [-ACh]
    float v28[2]; // [sp+58h] [-A8h] BYREF
    float v29; // [sp+60h] [-A0h] BYREF
    float v30; // [sp+64h] [-9Ch]

    iassert(Actor_ShouldMoveAwayFromCloseEnt(self));
    iassert(self->pCloseEnt.isDefined());

    v2 = self->pCloseEnt.ent();
    ent = self->ent;
    p_Physics = &self->Physics;
    groundEntNum = self->Physics.groundEntNum;
    bHasGroundPlane = self->Physics.bHasGroundPlane;
    groundplaneSlope = self->Physics.groundplaneSlope;
    iFootstepTimer = self->Physics.iFootstepTimer;
    v9 = self->Physics.vVelocity[0];
    v10 = self->Physics.vVelocity[1];
    v11 = v2;
    v12 = self->Physics.vVelocity[2];
    self->Physics.vOrigin[0] = self->ent->r.currentOrigin[0];
    self->Physics.vOrigin[1] = ent->r.currentOrigin[1];
    self->Physics.vOrigin[2] = ent->r.currentOrigin[2];
    if (v2->client)
    {
        v14 = Vec2DistanceSq(self->Physics.vOrigin, v2->r.currentOrigin);
        v16 = Actor_CalcultatePlayerPushDelta(self, v11, &v26);
        v15 = (float)((float)v16 * (float)v16);
    }
    else
    {
        v26 = p_Physics->vOrigin[0] - v2->r.currentOrigin[0];
        v27 = self->Physics.vOrigin[1] - v2->r.currentOrigin[1];
        v13 = Vec2Normalize(&v26);
        if (v13 == 0.0)
        {
            v26 = G_crandom();
            v27 = G_crandom();
            Vec2Normalize(&v26);
        }
        v14 = (float)((float)v13 * (float)v13);
        Actor_GetAnimDeltas(self, v28, &v29);
        v15 = (float)((float)(v30 * v30) + (float)(v29 * v29));
        if (v15 < 2.2500002)
            v15 = 2.2500002;
        //v16 = sqrtf(v15);
        v16 = sqrtf(v15);
    }
    v17 = (float)((float)v16 * v27);
    self->Physics.vWishDelta[0] = (float)v16 * v26;
    self->Physics.vWishDelta[1] = v17;
    v18 = (float)((float)v15 * (float)0.25);
    if (Actor_Physics(&self->Physics))
    {
        if (!Actor_PhysicsCheckMoveAwayNoWorse(self, v11, 0, v14, v18) && self->Physics.groundEntNum != 2175)
        {
            for (i = 0; i < 2; ++i)
            {
                if (self->Physics.iHitEntnum == 2175)
                    break;
                Vec2Normalize(self->Physics.vHitNormal);
                v16 = (float)((float)v16 * (float)0.75);
                v18 = (float)((float)v18 * (float)0.5625);
                LeftOrRightDodge = Actor_Physics_GetLeftOrRightDodge(
                    self,
                    (float)(self->Physics.vHitNormal[0] * v27) >= (double)(float)(self->Physics.vHitNormal[1]
                        * v26),
                    v16);
                self->Physics.vVelocity[0] = v9;
                self->Physics.vVelocity[1] = v10;
                v22 = LeftOrRightDodge;
                self->Physics.vVelocity[2] = v12;
                v23 = self->ent;
                self->Physics.groundplaneSlope = groundplaneSlope;
                self->Physics.groundEntNum = groundEntNum;
                self->Physics.bHasGroundPlane = bHasGroundPlane;
                self->Physics.iFootstepTimer = iFootstepTimer;
                p_Physics->vOrigin[0] = v23->r.currentOrigin[0];
                self->Physics.vOrigin[1] = v23->r.currentOrigin[1];
                self->Physics.vOrigin[2] = v23->r.currentOrigin[2];
                if (!Actor_Physics(&self->Physics))
                    goto LABEL_13;
                if (Actor_PhysicsCheckMoveAwayNoWorse(self, v11, v22, v14, v18))
                    return 1;
            }
            self->Physics.iHitEntnum = v11->s.number;
            actor = v11->actor;
            if (actor)
            {
                if (!actor->pCloseEnt.isDefined())
                    v11->actor->pCloseEnt.setEnt(self->ent);
            }
            self->ent->flags &= 0xFFFFFFE7;
            self->Physics.vVelocity[0] = v9;
            self->Physics.vVelocity[1] = v10;
            self->Physics.vVelocity[2] = v12;
            v25 = self->ent;
            self->Physics.groundplaneSlope = groundplaneSlope;
            self->Physics.groundEntNum = groundEntNum;
            self->Physics.bHasGroundPlane = bHasGroundPlane;
            self->Physics.iFootstepTimer = iFootstepTimer;
            p_Physics->vOrigin[0] = v25->r.currentOrigin[0];
            self->Physics.vOrigin[1] = v25->r.currentOrigin[1];
            self->Physics.vOrigin[2] = v25->r.currentOrigin[2];
        }
        return 1;
    }
    else
    {
    LABEL_13:
        result = 0;
        self->ent->flags &= 0xFFFFFFE7;
    }
    return result;
}

int __cdecl Actor_IsAtScriptGoal(actor_s *self)
{
    iassert(self);
    iassert(self->ent);
    iassert(self->sentient);

    if (!Actor_PointAtGoal(self->ent->r.currentOrigin, &self->scriptGoal))
        return 0;

    if (Path_Exists(&self->Path))
        return Actor_PointAtGoal(self->Path.vFinalGoal, &self->scriptGoal);

    return 1;
}

bool __cdecl Actor_IsNearClaimedNode(actor_s *self)
{
    iassert(self);
    iassert(self->ent);
    iassert(self->sentient);
    iassert(self->sentient->pClaimedNode);

    return Actor_KeepClaimedNode(self) || Actor_PointNearNode(self->ent->r.currentOrigin, self->sentient->pClaimedNode);
}

int __cdecl Actor_IsFixedNodeUseable(actor_s *self)
{
    pathnode_t *node; // r28
    actor_goal_s *p_codeGoal; // r3
    bool v4; // zf
    int result; // r3
    sentient_s *v6; // r3
    const gentity_s *v7; // r4
    gentity_s *v8; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3830, 0, "%s", "self");
    node = self->codeGoal.node;
    v4 = !Path_Exists(&self->Path);
    p_codeGoal = &self->codeGoal;
    if (v4)
    {
        if (Actor_PointNear(p_codeGoal->pos, self->ent->r.currentOrigin))
        {
            result = 1;
            self->commitToFixedNode = 0;
            return result;
        }
    }
    else if (Actor_PointNear(p_codeGoal->pos, self->Path.vFinalGoal)
        || self->arrivalInfo.animscriptOverrideRunTo
        && Actor_PointNear(self->arrivalInfo.animscriptOverrideRunToPos, self->Path.vFinalGoal))
    {
        return 1;
    }
    if (!node || Path_CanClaimNode(node, self->sentient))
    {
        if (self->fixedNodeSafeVolume.isDefined())
            v7 = self->fixedNodeSafeVolume.ent();
        else
            v7 = 0;
        v8 = Actor_IsKnownEnemyInRegion(self, v7, self->codeGoal.pos, self->fixedNodeSafeRadius);
        if (!v8)
            return 1;
        Scr_AddEntity(v8);
        Scr_Notify(self->ent, scr_const.node_not_safe, 1u);
        return 0;
    }
    else
    {
        if (node->dynamic.pOwner.isDefined())
        {
            v6 = node->dynamic.pOwner.sentient();
            Scr_AddEntity(v6->ent);
            Scr_Notify(self->ent, scr_const.node_taken, 1u);
        }
        return 0;
    }
}

bool __cdecl Actor_FindPath(
    actor_s *self,
    const float *vGoalPos,
    int bAllowNegotiationLinks,
    bool ignoreSuppression)
{
    int SuppressionPlanes; // r30
    sentient_s *sentient; // r3
    pathnode_t *v11; // r5
    pathnode_t *v12; // r5
    int v13; // [sp+8h] [-C8h]
    float v14[4]; // [sp+60h] [-70h] BYREF
    float v15[12][2]; // [sp+70h] [-60h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4175, 0, "%s", "self");
    if (!vGoalPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4176, 0, "%s", "vGoalPos");
    if (self->ent->tagInfo)
    {
        Actor_ClearPath(self);
        return 1;
    }
    if (Path_Exists(&self->Path))
    {
        if (!Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, vGoalPos))
            return 1;
        Actor_ClearPath(self);
    }
    else if (Actor_PointAt(self->ent->r.currentOrigin, vGoalPos))
    {
        return 1;
    }
    if (ignoreSuppression)
        SuppressionPlanes = 0;
    else
        SuppressionPlanes = Actor_GetSuppressionPlanes(self, v15, v14);
    Sentient_InvalidateNearestNode(self->sentient);
    sentient = self->sentient;
    if (SuppressionPlanes)
    {
        v11 = Sentient_NearestNodeSuppressed(sentient, v15, v14, SuppressionPlanes);
        if (!v11)
            return 0;
        Path_FindPathFromNotCrossPlanes(
            &self->Path,
            self->sentient->eTeam,
            v11,
            self->ent->r.currentOrigin,
            vGoalPos,
            v15,
            v14,
            SuppressionPlanes,
            v13);
        return Actor_HasPath(self);
    }
    else
    {
        v12 = Sentient_NearestNode(sentient);
        if (!v12)
            return 0;
        Path_FindPathFrom(
            &self->Path,
            self->sentient->eTeam,
            v12,
            self->ent->r.currentOrigin,
            vGoalPos,
            bAllowNegotiationLinks);
        return Actor_HasPath(self);
    }
}

void __cdecl Actor_RecalcPath(actor_s *self)
{
    int v2; // r29
    sentient_s *sentient; // r3
    pathnode_t *v4; // r5

    if (!self)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4241, 0, "%s", "self");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    }
    if (Path_Exists(&self->Path))
    {
        v2 = Path_AllowsObstacleNegotiation(&self->Path);
        Actor_ClearPath(self);
        sentient = self->sentient;
        self->Path.flags |= 0x80u;
        Sentient_InvalidateNearestNode(sentient);
        v4 = Sentient_NearestNode(self->sentient);
        if (v4)
        {
            Path_FindPathFrom(&self->Path, self->sentient->eTeam, v4, self->ent->r.currentOrigin, self->Path.vFinalGoal, v2);
            self->Path.flags |= 0x80u;
        }
    }
}

bool __cdecl Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable)
{
    int SuppressionPlanes; // r28
    int *v8; // r6
    pathnode_t *v9; // r3
    pathnode_t *v10; // r29
    gentity_s *ent; // r11
    float *vOrigin; // r8
    path_t *p_Path; // r3
    int v14; // [sp+8h] [-3D8h]
    int v15; // [sp+Ch] [-3D4h]
    _BYTE v16[16]; // [sp+60h] [-380h] BYREF
    float v17[4]; // [sp+70h] [-370h] BYREF
    float v18[4][2]; // [sp+80h] [-360h] BYREF
    pathsort_t v19[69]; // [sp+A0h] [-340h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4268, 0, "%s", "self");
    if (!pGoalNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4269, 0, "%s", "pGoalNode");
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    if (Path_Exists(&self->Path))
    {
        if (!Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, pGoalNode->constant.vOrigin))
            return 1;
        Actor_ClearPath(self);
    }
    else if (Actor_PointAt(self->ent->r.currentOrigin, pGoalNode->constant.vOrigin))
    {
        return 1;
    }
    Sentient_InvalidateNearestNode(self->sentient);
    if (bSuppressable)
    {
        SuppressionPlanes = Actor_GetSuppressionPlanes(self, v18, v17);
        v9 = Sentient_NearestNodeSuppressed(self->sentient, v18, v17, SuppressionPlanes);
    }
    else
    {
        SuppressionPlanes = 0;
        v9 = Sentient_NearestNode(self->sentient);
    }
    v10 = v9;
    if (!v9)
        return 0;
    if ((pGoalNode->constant.spawnflags & 1) != 0)
    {
        pGoalNode = Path_NearestNode(pGoalNode->constant.vOrigin, v19, -2, 192.0, v8, (int)v16, (nearestNodeHeightCheck)64);
        if (!pGoalNode)
            return 0;
    }
    ent = self->ent;
    vOrigin = pGoalNode->constant.vOrigin;
    p_Path = &self->Path;
    if (SuppressionPlanes)
        Path_FindPathFromToNotCrossPlanes(
            p_Path,
            self->sentient->eTeam,
            v10,
            ent->r.currentOrigin,
            pGoalNode,
            vOrigin,
            v18,
            v17,
            v14,
            v15);
    else
        Path_FindPathFromTo(p_Path, self->sentient->eTeam, v10, ent->r.currentOrigin, pGoalNode, vOrigin, 1);
    return Actor_HasPath(self);
}

bool __cdecl Actor_FindPathToSentient(actor_s *self, sentient_s *pGoalEnt, int bSuppressable)
{
    int SuppressionPlanes; // r30
    pathnode_t *v8; // r29
    pathnode_t *v9; // r7
    pathnode_t *v10; // r30
    pathnode_t *v11; // r7
    int v12; // [sp+8h] [-D8h]
    int v13; // [sp+Ch] [-D4h]
    float v14[4]; // [sp+60h] [-80h] BYREF
    float v15[4]; // [sp+70h] [-70h] BYREF
    float v16[12][2]; // [sp+80h] [-60h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4333, 0, "%s", "self");
    if (!pGoalEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4334, 0, "%s", "pGoalEnt");
    Sentient_GetOrigin(pGoalEnt, v14);
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    if (Path_Exists(&self->Path))
    {
        if (!Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, v14))
            return 1;
        Actor_ClearPath(self);
    }
    Sentient_InvalidateNearestNode(self->sentient);
    if (bSuppressable)
    {
        SuppressionPlanes = Actor_GetSuppressionPlanes(self, v16, v15);
        v8 = Sentient_NearestNodeSuppressed(self->sentient, v16, v15, SuppressionPlanes);
        if (v8)
        {
            v9 = Sentient_NearestNodeSuppressed(pGoalEnt, v16, v15, SuppressionPlanes);
            if (v9)
            {
                Path_FindPathFromToNotCrossPlanes(
                    &self->Path,
                    self->sentient->eTeam,
                    v8,
                    self->ent->r.currentOrigin,
                    v9,
                    v14,
                    v16,
                    v15,
                    v12,
                    v13);
                return Actor_HasPath(self);
            }
        }
        return 0;
    }
    v10 = Sentient_NearestNode(self->sentient);
    if (!v10)
        return 0;
    v11 = Sentient_NearestNode(pGoalEnt);
    if (!v11)
        return 0;
    Path_FindPathFromTo(&self->Path, self->sentient->eTeam, v10, self->ent->r.currentOrigin, v11, v14, 1);
    return Actor_HasPath(self);
}

void __cdecl Actor_FindPathInGoalWithLOS(
    actor_s *self,
    const float *vGoalPos,
    double fWithinDistSqrd,
    bool ignoreSuppression)
{
    int v9; // r8
    int v10; // [sp+8h] [-C8h]
    float fDists[4]; // [sp+60h] [-70h] BYREF
    float vNormals[5][2]; // [sp+70h] [-60h] BYREF

    iassert(self);
    iassert(vGoalPos);

    Actor_ClearPath(self);

    if (ignoreSuppression)
        Path_FindPathInCylinderWithLOS(
            &self->Path,
            self->sentient->eTeam,
            self->ent->r.currentOrigin,
            (float*)vGoalPos,
            &self->codeGoal,
            fWithinDistSqrd,
            false);
    else
    {
        int iPlaneCount = Actor_GetSuppressionPlanes(self, vNormals, fDists);
        if (!iPlaneCount)
        {
            Path_FindPathInCylinderWithLOS(
                &self->Path,
                self->sentient->eTeam,
                self->ent->r.currentOrigin,
                (float *)vGoalPos,
                &self->codeGoal,
                fWithinDistSqrd,
                false);
            return;
        }
        else
        {
            Path_FindPathInCylinderWithLOSNotCrossPlanes(
                &self->Path,
                self->sentient->eTeam,
                self->ent->r.currentOrigin,
                (float *)vGoalPos,
                &self->codeGoal,
                fWithinDistSqrd,
                (float(*)[2])vNormals[0],
                fDists,
                iPlaneCount,
                true); // KISAKTODO: idk if 'true'
        }
    }
}

void __cdecl Actor_FindPathAway(
    actor_s *self,
    const float *vBadPos,
    double fMinSafeDist,
    int bAllowNegotiationLinks)
{
    int v7; // r7

    iassert(self);
    iassert(vBadPos);

    Actor_ClearPath(self);
    Path_FindPathAway(&self->Path, self->sentient->eTeam, self->ent->r.currentOrigin, (float*)vBadPos, fMinSafeDist, v7);
}

void __cdecl Actor_FindPathAwayNotCrossPlanes(
    actor_s *self,
    const float *vBadPos,
    double fMinSafeDist,
    float *normal,
    double dist,
    float *bSuppressable,
    int bAllowNegotiationLinks)
{
    int SuppressionPlanes; // r10
    double v15; // fp13
    float *v16; // r11
    float vDists[8]; // [sp+60h] [-A0h] BYREF
    float vNormals[6][2]; // [sp+80h] [-80h] BYREF

    iassert(self);
    iassert(vBadPos);

    Actor_ClearPath(self);

    if (bAllowNegotiationLinks)
        SuppressionPlanes = Actor_GetSuppressionPlanes(self, vNormals, vDists);
    else
        SuppressionPlanes = 0;

    if (dist != 0.0)
    {
        v15 = bSuppressable[1];
        v16 = vNormals[SuppressionPlanes];
        *v16 = *bSuppressable;
        vDists[SuppressionPlanes++] = dist;
        v16[1] = v15;
    }
    Path_FindPathAwayNotCrossPlanes(
        &self->Path,
        self->sentient->eTeam,
        self->ent->r.currentOrigin,
        (float*)vBadPos,
        fMinSafeDist,
        (float(*)[2])vNormals[0],
        vDists,
        SuppressionPlanes,
        bAllowNegotiationLinks);
}

void __cdecl Actor_BadPlacesChanged()
{
    actor_s *i; // r30

    for (i = Actor_FirstActor(-1); i; i = Actor_NextActor(i, -1))
    {
        if (Path_Exists(&i->Path))
            Path_TrimToBadPlaceLink(&i->Path, i->sentient->eTeam);
    }
}

void __cdecl Actor_UpdateAnglesAndDelta(actor_s *self)
{
    gentity_s *ent; // r28
    int iPathEndTime; // r10
    ai_animmode_t eAnimMode; // r4
    double v5; // fp13
    double v6; // fp0
    double v7; // fp30
    bool IsDodgeEntity; // r3
    double v9; // fp0
    double v10; // fp13
    double v11; // fp0
    double v12; // fp13
    double v13; // fp0
    double v14; // fp13
    gentity_s *v15; // r3
    const char *v16; // r3
    float v17[2]; // [sp+50h] [-50h] BYREF
    float v18; // [sp+58h] [-48h] BYREF
    float v19; // [sp+5Ch] [-44h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4725, 0, "%s", "self");
    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4727, 0, "%s", "ent");
    if (self->eAnimMode != AI_ANIM_MOVE_CODE)
    {
        iPathEndTime = self->Path.iPathEndTime;
        if (iPathEndTime)
        {
            if (iPathEndTime - level.time <= 0)
            {
                Actor_ClearPath(self);
                self->Path.iPathEndTime = 0;
            }
        }
    }
    eAnimMode = self->eAnimMode;
    switch (eAnimMode)
    {
    case AI_ANIM_MOVE_CODE:
        if (self->moveMode && Actor_HasPath(self) && !self->pCloseEnt.isDefined())
        {
            Actor_GetAnimDeltas(self, v17, &v18);
            if (!Actor_HasPath(self))
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4748, 0, "%s", "Actor_HasPath( self )");
            v5 = (float)(v18 * v18);
            v6 = v19;
            self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
            Path_UpdateMovementDelta(self, sqrtf((float)((float)((float)v6 * (float)v6) + (float)v5)));
            Actor_PathEndActions(self);
            v7 = 0.0;
            if (ai_showPaths->current.integer)
                Path_DebugDraw(&self->Path, ent->r.currentOrigin, 1);
        }
        else
        {
            if (Actor_HasPath(self))
            {
                IsDodgeEntity = Actor_IsDodgeEntity(self, self->Physics.iHitEntnum);
                Path_UpdateLookahead(&self->Path, ent->r.currentOrigin, IsDodgeEntity, 0, 1);
                Actor_AddStationaryMoveHistory(self);
            }
            v7 = 0.0;
            self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
            self->Physics.vWishDelta[0] = 0.0;
            self->Physics.vWishDelta[1] = 0.0;
            self->Physics.vWishDelta[2] = 0.0;
            if (ai_showPaths->current.integer == 2)
                Path_DebugDraw(&self->Path, ent->r.currentOrigin, 1);
        }
        goto LABEL_33;
    case AI_ANIM_USE_POS_DELTAS:
        Actor_GetAnimDeltas(self, v17, &v18);
        v9 = v18;
        v10 = v19;
        self->Physics.ePhysicsType = AIPHYS_NORMAL_RELATIVE;
        self->Physics.vWishDelta[0] = v9;
        self->Physics.vWishDelta[1] = v10;
        self->Physics.vWishDelta[2] = 0.0;
        goto LABEL_32;
    case AI_ANIM_USE_ANGLE_DELTAS:
        Actor_GetAnimDeltas(self, v17, &v18);
        self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
        self->Physics.vWishDelta[0] = 0.0;
        self->Physics.vWishDelta[1] = 0.0;
        self->Physics.vWishDelta[2] = 0.0;
        v7 = RotationToYaw(v17);
        goto LABEL_33;
    case AI_ANIM_USE_BOTH_DELTAS:
        Actor_GetAnimDeltas(self, v17, &v18);
        v11 = v18;
        v12 = v19;
        self->Physics.ePhysicsType = AIPHYS_NORMAL_RELATIVE;
        self->Physics.vWishDelta[0] = v11;
        self->Physics.vWishDelta[1] = v12;
        self->Physics.vWishDelta[2] = 0.0;
        v7 = RotationToYaw(v17);
        goto LABEL_33;
    case AI_ANIM_USE_BOTH_DELTAS_NOCLIP:
        Actor_GetAnimDeltas(self, v17, self->Physics.vWishDelta);
        self->Physics.ePhysicsType = AIPHYS_NOCLIP;
        v7 = RotationToYaw(v17);
        goto LABEL_33;
    case AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY:
        Actor_GetAnimDeltas(self, v17, self->Physics.vWishDelta);
        self->Physics.ePhysicsType = AIPHYS_NOGRAVITY;
        v7 = RotationToYaw(v17);
        goto LABEL_33;
    case AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS:
        Actor_GetAnimDeltas(self, v17, &v18);
        v13 = v18;
        v14 = v19;
        self->Physics.ePhysicsType = AIPHYS_ZONLY_PHYSICS_RELATIVE;
        self->Physics.vWishDelta[0] = v13;
        self->Physics.vWishDelta[1] = v14;
        self->Physics.vWishDelta[2] = 0.0;
        v7 = RotationToYaw(v17);
        goto LABEL_33;
    case AI_ANIM_POINT_RELATIVE:
        v15 = self->ent;
        self->Physics.ePhysicsType = AIPHYS_ZONLY_PHYSICS_RELATIVE;
        if (!v15->tagInfo)
            G_Animscripted_Think(v15);
        Actor_SetDesiredAngles(&self->CodeOrient, self->ent->r.currentAngles[0], self->ent->r.currentAngles[1]);
        self->Physics.vVelocity[0] = 0.0;
        self->Physics.vVelocity[1] = 0.0;
        self->Physics.vVelocity[2] = 0.0;
        self->Physics.vWishDelta[0] = 0.0;
        self->Physics.vWishDelta[1] = 0.0;
        self->Physics.vWishDelta[2] = 0.0;
        return;
    default:
        if (!alwaysfails)
        {
            v16 = va("bad eAnimMode value: %d", eAnimMode);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4841, 0, v16);
        }
    LABEL_32:
        v7 = 0.0;
    LABEL_33:
        if (ai_debugAnimDeltas->current.integer == ent->s.number)
            Com_Printf(18, (const char *)HIDWORD(v7), LODWORD(v7));
        if (v7 != 0.0)
            Actor_ChangeAngles(self, 0.0, v7);
        Actor_DecideOrientation(self);
        Actor_UpdateBodyAngle(self);
        Actor_UpdateLookAngles(self);
        return;
    }
}

void __cdecl Actor_UpdatePileUp(actor_s *self)
{
    actor_s *v2; // r31
    int iTeamMoveWaitTime; // r10

    if (self->eAnimMode == AI_ANIM_MOVE_CODE)
    {
        if (self->pPileUpActor == self)
            return;
        v2 = self;
        while (1)
        {
            v2 = v2->pPileUpActor;
            if (!v2)
                break;
            if (v2->pPileUpActor == v2)
            {
                if (!v2->pPileUpEnt)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5015, 0, "%s", "other->pPileUpEnt");
                if (v2 == self)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5016, 0, "%s", "other != self");
                iTeamMoveWaitTime = v2->iTeamMoveWaitTime;
                if (level.time >= v2->ent->nextthink)
                    iTeamMoveWaitTime += 50;
                if (level.time < iTeamMoveWaitTime && self->ent != v2->pPileUpEnt)
                {
                    self->pPileUpActor = v2;
                    self->pPileUpEnt = v2->pPileUpEnt;
                    return;
                }
                break;
            }
        }
    }
    self->pPileUpActor = 0;
    self->pPileUpEnt = 0;
}

void __cdecl Actor_UpdateGoalPos(actor_s *self)
{
    actor_goal_s *p_codeGoal; // r30
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29
    sentient_s *TargetSentient; // r27
    float *v7; // r11
    double pathEnemyFightDist; // fp1
    sentient_s *sentient; // r28
    pathnode_t *pDesiredChainPos; // r11
    gentity_s *ent; // r11
    double fRadius; // fp1
    gentity_s *v13; // r3
    pathnode_t *node; // r11
    gentity_s *volume; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5599, 0, "%s", "self");
    p_codeGoal = &self->codeGoal;
    v3 = self->codeGoal.pos[0];
    v4 = self->codeGoal.pos[1];
    v5 = self->codeGoal.pos[2];
    if (!self->useEnemyGoal)
    {
        if (self->scriptGoalEnt.isDefined())
        {
            iassert(self->scriptGoalEnt.ent()->r.inuse);
            iassert(!self->scriptGoal.node);
            iassert(!self->scriptGoal.volume);
            self->codeGoal.node = 0;
            self->codeGoal.volume = 0;
            sentient = self->scriptGoalEnt.ent()->sentient;
            if (sentient
                && sentient->eTeam != Sentient_EnemyTeam(self->sentient->eTeam)
                && self->iFollowMin <= self->iFollowMax)
            {
                pDesiredChainPos = self->pDesiredChainPos;
                self->codeGoalSrc = AI_GOAL_SRC_FRIENDLY_CHAIN;
                if (!pDesiredChainPos)
                {
                    ent = self->ent;
                    p_codeGoal->pos[0] = self->ent->r.currentOrigin[0];
                    self->codeGoal.pos[1] = ent->r.currentOrigin[1];
                    self->codeGoal.pos[2] = ent->r.currentOrigin[2];
                    goto LABEL_25;
                }
                p_codeGoal->pos[0] = pDesiredChainPos->constant.vOrigin[0];
                self->codeGoal.pos[1] = pDesiredChainPos->constant.vOrigin[1];
                self->codeGoal.pos[2] = pDesiredChainPos->constant.vOrigin[2];
                fRadius = pDesiredChainPos->constant.fRadius;
                if (fRadius == 0.0)
                    goto LABEL_25;
            LABEL_24:
                Actor_SetGoalRadius(&self->codeGoal, fRadius);
                Actor_SetGoalHeight(&self->codeGoal, self->scriptGoal.height);
                goto LABEL_25;
            }
            v13 = self->scriptGoalEnt.ent();
            p_codeGoal->pos[0] = v13->r.currentOrigin[0];
            self->codeGoal.pos[1] = v13->r.currentOrigin[1];
            self->codeGoal.pos[2] = v13->r.currentOrigin[2];
            self->codeGoalSrc = AI_GOAL_SRC_SCRIPT_ENTITY_GOAL;
        }
        else
        {
            p_codeGoal->pos[0] = self->scriptGoal.pos[0];
            self->codeGoal.pos[1] = self->scriptGoal.pos[1];
            self->codeGoal.pos[2] = self->scriptGoal.pos[2];
            node = self->scriptGoal.node;
            volume = self->scriptGoal.volume;
            self->codeGoalSrc = AI_GOAL_SRC_SCRIPT_GOAL;
            self->codeGoal.node = node;
            self->codeGoal.volume = volume;
        }
        fRadius = self->scriptGoal.radius;
        goto LABEL_24;
    }
    TargetSentient = Actor_GetTargetSentient(self);
    if (!TargetSentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 5606, 0, "%s", "enemy");
    v7 = (float *)((char *)self + 40 * (TargetSentient - level.sentients));
    p_codeGoal->pos[0] = v7[531];
    self->codeGoal.pos[1] = v7[532];
    self->codeGoal.pos[2] = v7[533];
    self->codeGoalSrc = AI_GOAL_SRC_ENEMY;
    pathEnemyFightDist = self->pathEnemyFightDist;
    self->codeGoal.node = 0;
    self->codeGoal.volume = 0;
    Actor_SetGoalRadius(&self->codeGoal, pathEnemyFightDist);
LABEL_25:
    if (self->arrivalInfo.animscriptOverrideRunTo
        && (p_codeGoal->pos[0] != v3 || self->codeGoal.pos[1] != v4 || self->codeGoal.pos[2] != v5))
    {
        self->arrivalInfo.animscriptOverrideRunTo = 0;
    }
}

int __cdecl SP_actor(gentity_s *ent)
{
    actor_s *v3; // r31
    sentient_s *v4; // r3
    sentient_s *v5; // r27
    int eFlags; // r11
    unsigned __int8 v7; // r10
    int v8; // r9
    float *currentOrigin; // r26
    float *v10; // r11
    int v11; // r10
    actor_goal_s *p_scriptGoal; // r28
    actor_goal_s *p_codeGoal; // r11
    int v14; // ctr

    ent->model = 0;
    if (!g_spawnai->current.enabled)
        goto LABEL_8;
    if ((ent->spawnflags & 1) != 0)
        return SP_actor_spawner(ent);
    Actor_droptofloor(ent);
    v3 = Actor_Alloc();
    if (!v3)
    {
        if ((ent->spawnflags & 2) == 0)
        {
        LABEL_8:
            G_FreeEntity(ent);
            return 0;
        }
        Actor_FreeExpendable();
        v3 = Actor_Alloc();
        if (!v3)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 233, 0, "%s", "actor");
            goto LABEL_8;
        }
    }
    v4 = Sentient_Alloc();
    v5 = v4;
    if (v4)
    {
        eFlags = ent->s.lerp.eFlags;
        ent->actor = v3;
        ent->sentient = v4;
        ent->nextthink = 0;
        ent->handler = 2;
        if (eFlags)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 259, 0, "%s", "!ent->s.lerp.eFlags");
        v7 = ent->r.svFlags | 2;
        v8 = ent->flags | 0x2800;
        ent->takedamage = 1;
        ent->r.svFlags = v7;
        ent->flags = v8;
        currentOrigin = ent->r.currentOrigin;
        ent->maxHealth = 100;
        ent->health = 100;
        ent->r.mins[2] = 0.0;
        ent->r.mins[0] = -15.0;
        ent->r.mins[1] = -15.0;
        ent->r.maxs[0] = 15.0;
        ent->r.maxs[1] = 15.0;
        ent->r.maxs[2] = 72.0;
        ent->clipmask = 42074129;
        ent->r.contents = 0x4000;
        ent->s.eType = ET_ACTOR;
        G_SetOrigin(ent, ent->r.currentOrigin);
        v3->ent = ent;
        v3->sentient = v5;
        Actor_InitMove(v3);
        Actor_InitAnim(v3);
        Path_Begin(&v3->Path);
        v3->sideMove = 0.0;
        v3->iTeamMoveDodgeTime = 0;
        v3->safeToChangeScript = 1;
        v3->ignoreTriggers = 0;
        v3->accuracy = 0.2;
        v3->missCount = 4;
        v3->eAllowedStances = STANCE_ANY;
        v3->pushable = 1;
        v3->bDropWeapon = 1;
        v3->playerSightAccuracy = 1.0;
        v3->bDrawOnCompass = 1;
        v3->allowPain = 1;
        v3->allowDeath = 0;
        v3->debugLastAccuracy = -6969.0;
        v3->iUseHintString = -1;
        Actor_SetDesiredAngles(&v3->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        G_SetAngle(ent, ent->r.currentAngles);
        Actor_SetLookAngles(v3, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        v10 = &v3->moveHistory[0][1];
        v11 = 10;
        do
        {
            --v11;
            *(v10 - 1) = 0.0;
            *v10 = 0.0;
            v10 += 2;
        } while (v11);
        p_scriptGoal = &v3->scriptGoal;
        v3->moveHistoryIndex = 0;
        v3->scriptGoal.pos[0] = *currentOrigin;
        v3->scriptGoal.pos[1] = ent->r.currentOrigin[1];
        v3->scriptGoal.pos[2] = ent->r.currentOrigin[2];
        if (v3->scriptGoal.node)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 306, 0, "%s", "!actor->scriptGoal.node");
        if (v3->scriptGoal.volume)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 307, 0, "%s", "!actor->scriptGoal.volume");
        Actor_SetGoalRadius(&v3->scriptGoal, 0.0);
        Actor_SetGoalHeight(&v3->scriptGoal, 0.0);
        p_codeGoal = &v3->codeGoal;
        v14 = 7;
        do
        {
            p_codeGoal->pos[0] = p_scriptGoal->pos[0];
            p_scriptGoal = (actor_goal_s *)((char *)p_scriptGoal + 4);
            p_codeGoal = (actor_goal_s *)((char *)p_codeGoal + 4);
            --v14;
        } while (v14);
        v3->keepClaimedNode = 0;
        v3->keepClaimedNodeInGoal = 0;
        v3->noDodgeMove = 0;
        v3->goalPosChanged = 0;
        v3->fixedNode = 0;
        v3->fixedNodeSafeRadius = 64.0;
        v3->arrivalInfo.animscriptOverrideRunTo = 0;
        v3->arrivalInfo.arrivalNotifyRequested = 0;
        v3->exposedDuration = 5000;
        v3->exposedStartTime = 0x80000000;
        Actor_InitThreatUpdateInterval(v3);
        v5->attackerAccuracy = 1.0;
        v5->ent = ent;
        v5->iThreatBias = 0;
        v5->iThreatBiasGroupIndex = 0;
        v5->pPrevClaimedNode = 0;
        v5->maxVisibleDist = 8192.0;
        v5->eTeam = TEAM_NEUTRAL;
        v5->oldOrigin[0] = *currentOrigin;
        v5->oldOrigin[1] = ent->r.currentOrigin[1];
        v5->oldOrigin[2] = ent->r.currentOrigin[2];
        G_InitActorProneInfo(v3);
        v3->fInvProneAnimLowPitch = 0.0;
        v3->fInvProneAnimHighPitch = 0.0;
        v3->fProneLastDiff = 0.0;
        Actor_InitLookAt(v3);
        SV_LinkEntity(ent);
        Sentient_NearestNode(v5);
        return 1;
    }
    else
    {
        v3->inuse = 0;
        G_FreeEntity(ent);
        return 0;
    }
}

int __cdecl Actor_CheckGoalNotify(actor_s *self)
{
    sentient_s *sentient; // r11
    const pathnode_t *pClaimedNode; // r31
    double v5; // fp1
    unsigned __int8 v6; // r11

    if (!(unsigned __int8)Actor_IsAtScriptGoal(self))
        return 0;
    if (self->ScriptOrient.eMode)
        return 1;
    if (self->CodeOrient.eMode != AI_ORIENT_TO_GOAL)
        return 1;
    sentient = self->sentient;
    pClaimedNode = sentient->pClaimedNode;
    if (!pClaimedNode || Path_IsCoverNode(sentient->pClaimedNode) || !Path_IsValidClaimNode(pClaimedNode))
        return 1;
    v5 = AngleSubtract(pClaimedNode->constant.fAngle, self->ent->r.currentAngles[1]);
    if (v5 < -20.0)
        return 0;
    v6 = 1;
    if (v5 > 20.0)
        return 0;
    return v6;
}

void __cdecl Actor_CheckNotify(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1289, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1290, 0, "%s", "self->sentient");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1291, 0, "%s", "self->ent");
    if ((unsigned __int8)Actor_CheckGoalNotify(self))
        Scr_Notify(self->ent, scr_const.goal, 0);
    if (self->arrivalInfo.animscriptOverrideRunTo)
    {
        if (Actor_PointAt(self->ent->r.currentOrigin, self->arrivalInfo.animscriptOverrideRunToPos))
        {
            Scr_Notify(self->ent, scr_const.runto_arrived, 0);
            self->arrivalInfo.animscriptOverrideRunTo = 0;
        }
    }
}

void __cdecl Actor_Think(gentity_s *self)
{
    int v2; // r27
    int time; // r11
    actor_s *actor; // r26
    const char *v5; // r3
    unsigned int stateLevel; // r7
    int v7; // r8
    actor_think_result_t v8; // r25
    float *sentient; // r11
    float *currentOrigin; // r30
    int v11; // r10
    bool v12; // r27
    int v13; // r11
    char v14; // r11
    bool v15; // zf
    char v16; // r28
    float *v17; // r6
    float *v18; // r5
    long double v19; // fp2
    long double v20; // fp2
    actor_s *v21; // r11
    long double v22; // fp2
    int v23; // [sp+8h] [-B8h]
    hitLocation_t v24; // [sp+Ch] [-B4h]
    unsigned int v25; // [sp+10h] [-B0h]
    unsigned int v26; // [sp+14h] [-ACh]

    v2 = 0;
    //Profile_Begin(224);
    if (g_ai->current.enabled)
    {
        if (!self)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1334, 0, "%s", "self");
        actor = self->actor;
        if (!actor)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1337, 0, "%s", "actor");
        if (!actor->inuse)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1338, 0, "%s", "actor->inuse");
        if (Com_GetServerDObj(self->s.number))
        {
            if (!actor->ent)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1360, 0, "%s", "actor->ent");
            if (actor->ent != self)
            {
                v5 = va("actor->ent->s.number: %d, self->s.number: %d", actor->ent->s.number, self->s.number);
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1361, 0, "%s\n\t%s", "actor->ent == self", v5);
            }
            if (!actor->sentient)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1362, 0, "%s", "actor->sentient");
            if (actor->sentient->ent != self)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1363, 0, "%s", "actor->sentient->ent == self");
            if (self->sentient != actor->sentient)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    1364,
                    0,
                    "%s",
                    "self->sentient == actor->sentient");
            if (self->client)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1365, 0, "%s", "self->client == NULL");
            stateLevel = actor->stateLevel;
            if (stateLevel >= 5)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    1366,
                    0,
                    "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
                    stateLevel,
                    5);
            v7 = actor->eState[actor->stateLevel];
            if (v7 <= 0 || v7 >= 11)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    1367,
                    0,
                    "%s\n\t(actor->eState[actor->stateLevel]) = %i",
                    "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
                    v7);
            if (actor->Path.iPathEndTime < 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    1370,
                    0,
                    "%s",
                    "actor->Path.iPathEndTime >= 0");
            if (actor->Path.iPathEndTime > level.time + 500)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    1371,
                    0,
                    "%s",
                    "actor->Path.iPathEndTime <= level.time + ACTOR_STOP_TIME");
            Actor_DecaySuppressionLines(actor);
            if (actor->Physics.bIsAlive)
                Actor_UpdatePileUp(actor);
            do
            {
                ++v2;
                v8 = Actor_CallThink(actor);
                if (v2 >= 10)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                        1394,
                        0,
                        "%s",
                        "callThinkCounter < ACTOR_CALL_THINK_REPEAT_MAX");
            } while (v8 == ACTOR_THINK_REPEAT);
            sentient = (float *)actor->sentient;
            currentOrigin = self->r.currentOrigin;
            if (self->r.currentOrigin[0] != sentient[5]
                || self->r.currentOrigin[1] != sentient[6]
                || (v11 = 1, self->r.currentOrigin[2] != sentient[7]))
            {
                v11 = 0;
            }
            sentient[5] = *currentOrigin;
            v12 = v11 == 0;
            sentient[6] = self->r.currentOrigin[1];
            sentient[7] = self->r.currentOrigin[2];
            self->s.lerp.pos.trType = (unsigned __int8)Com_IsRagdollTrajectory(&self->s.lerp.pos) == 0
                ? TR_INTERPOLATE
                : TR_RAGDOLL_INTERPOLATE;
            self->s.lerp.pos.trBase[0] = *currentOrigin;
            self->s.lerp.pos.trBase[1] = self->r.currentOrigin[1];
            self->s.lerp.pos.trBase[2] = self->r.currentOrigin[2];
            if (self->r.currentAngles[0] != self->s.lerp.apos.trBase[0]
                || self->r.currentAngles[1] != self->s.lerp.apos.trBase[1]
                || (v13 = 1, self->r.currentAngles[2] != self->s.lerp.apos.trBase[2]))
            {
                v13 = 0;
            }
            if (v12 || (v15 = v13 != 0, v14 = 0, !v15))
                v14 = 1;
            v16 = v14;
            self->s.lerp.apos.trType = (unsigned __int8)Com_IsRagdollTrajectory(&self->s.lerp.apos) == 0
                ? TR_INTERPOLATE
                : TR_RAGDOLL_INTERPOLATE;
            self->s.lerp.apos.trBase[0] = self->r.currentAngles[0];
            self->s.lerp.apos.trBase[1] = self->r.currentAngles[1];
            self->s.lerp.apos.trBase[2] = self->r.currentAngles[2];
            if (v12)
            {
                SV_LinkEntity(self);
                Actor_PostPhysics(&actor->Physics);
            }
            if (Sentient_NearestNodeDirty(actor->sentient, v12))
                Sentient_InvalidateNearestNode(actor->sentient);
            Sentient_BanNearNodes(actor->sentient);
            if (BG_ActorIsProne(&actor->ProneInfo, level.time))
            {
                if (v16)
                    Actor_UpdateProneInformation(actor, 1, v18, v17, v19);
            }
            else
            {
                actor->Physics.prone = 0;
            }
            Actor_UpdateLookAt(actor);
            if (actor->delayedDeath && !Actor_InScriptedState(actor))
                G_Damage(self, 0, 0, 0, self->r.currentOrigin, self->health + 1, 0, 0, v23, v24, v25, v26);
            v21 = self->actor;
            if (v21->Physics.bIsAlive && !v21->ignoreTriggers)
                G_DoTouchTriggers(self);
            if (v8 != ACTOR_THINK_MOVE_TO_BODY_QUEUE)
                Actor_CheckNotify(actor);
            *(double *)&v20 = actor->fovDot;
            v22 = acos(v20);
            self->s.lerp.u.turret.gunAngles[0] = (float)*(double *)&v22 * (float)57.295776;
            self->s.lerp.u.turret.gunAngles[1] = sqrtf(actor->fMaxSightDistSqrd);
            if (v8 != ACTOR_THINK_MOVE_TO_BODY_QUEUE || Actor_BecomeCorpse(self))
            {
                time = level.time;
                goto LABEL_68;
            }
        }
        else
        {
            Com_Printf(18, "^3Deleting AI without a model.\n");
        }
        G_FreeEntity(self);
        //Profile_EndInternal(0);
        return;
    }
    time = level.time;
LABEL_68:
    self->nextthink = time + 50;
    //Profile_EndInternal(0);
}

int __cdecl Actor_PhysicsAndDodge(actor_s *self)
{
    gentity_s *ent; // r11
    unsigned __int16 groundEntNum; // r22
    int bHasGroundPlane; // r21
    double groundplaneSlope; // fp28
    int iFootstepTimer; // r20
    double v7; // fp27
    double v8; // fp26
    double v9; // fp25
    int result; // r3
    int iHitEntnum; // r19
    int v12; // r28
    float *v13; // r25
    int v14; // r23
    double v15; // fp31
    double v16; // fp30
    double v17; // fp29
    int LeftOrRightDodge; // r3
    gentity_s *v19; // r11
    int v20; // r11
    int v21; // r3
    double v22; // fp1
    int wPathLen; // r30
    int v24; // r11
    int wDodgeCount; // r10
    float v26[20]; // [sp+50h] [-D0h] BYREF

    if (self->eAnimMode != AI_ANIM_MOVE_CODE || !Path_Exists(&self->Path))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            2505,
            0,
            "%s",
            "self->eAnimMode == AI_ANIM_MOVE_CODE && Actor_HasPath( self )");
    ent = self->ent;
    groundEntNum = self->Physics.groundEntNum;
    bHasGroundPlane = self->Physics.bHasGroundPlane;
    groundplaneSlope = self->Physics.groundplaneSlope;
    iFootstepTimer = self->Physics.iFootstepTimer;
    v7 = self->Physics.vVelocity[0];
    v8 = self->Physics.vVelocity[1];
    v9 = self->Physics.vVelocity[2];
    self->Physics.vOrigin[0] = self->ent->r.currentOrigin[0];
    self->Physics.vOrigin[1] = ent->r.currentOrigin[1];
    self->Physics.vOrigin[2] = ent->r.currentOrigin[2];
    if (!Actor_Physics(&self->Physics))
    {
    LABEL_5:
        result = 0;
        self->ent->flags &= 0xFFFFFFE7;
        return result;
    }
    iHitEntnum = self->Physics.iHitEntnum;
    if (iHitEntnum == 2175)
    {
        result = 1;
        self->ent->flags &= 0xFFFFFFE7;
        return result;
    }
    v12 = 0;
    if (!self->Physics.bStuck && Actor_IsDodgeEntity(self, self->Physics.iHitEntnum) && Path_IsTrimmed(&self->Path))
    {
        if ((unsigned __int16)self->Path.wNegotiationStartNode >= 0x8000u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                2529,
                0,
                "%s",
                "self->Path.wNegotiationStartNode >= 0");
        if (self->Path.wPathLen - 2 < self->Path.wNegotiationStartNode)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                2530,
                0,
                "%s",
                "self->Path.wPathLen - 2 >= self->Path.wNegotiationStartNode");
        v13 = (float *)&self->Physics.iTouchEnts[7 * self->Path.wPathLen + 27];
        v14 = 0;
        v15 = sqrtf((float)((float)(self->Physics.vWishDelta[0] * self->Physics.vWishDelta[0])
            + (float)(self->Physics.vWishDelta[1] * self->Physics.vWishDelta[1])));
        while (1)
        {
            v16 = (float)(self->Path.vCurrPoint[0] - self->Physics.vHitOrigin[0]);
            v17 = (float)(self->Path.vCurrPoint[1] - self->Physics.vHitOrigin[1]);
            Vec2Normalize(self->Physics.vHitNormal);
            v15 = (float)((float)v15 * (float)0.75);
            LeftOrRightDodge = Actor_Physics_GetLeftOrRightDodge(
                self,
                (float)(v13[1] * (float)v16) >= (double)(float)(*v13 * (float)v17),
                v15);
            self->Physics.vVelocity[0] = v7;
            self->Physics.vVelocity[1] = v8;
            v12 = LeftOrRightDodge;
            self->Physics.vVelocity[2] = v9;
            v19 = self->ent;
            self->Physics.groundplaneSlope = groundplaneSlope;
            self->Physics.groundEntNum = groundEntNum;
            self->Physics.bHasGroundPlane = bHasGroundPlane;
            self->Physics.iFootstepTimer = iFootstepTimer;
            self->Physics.vOrigin[0] = v19->r.currentOrigin[0];
            self->Physics.vOrigin[1] = v19->r.currentOrigin[1];
            self->Physics.vOrigin[2] = v19->r.currentOrigin[2];
            if (!Actor_Physics(&self->Physics))
                goto LABEL_5;
            if (!self->Physics.bStuck)
            {
                if (Actor_IsDodgeEntity(self, self->Physics.iHitEntnum))
                {
                    if (++v14 < 2)
                        continue;
                }
            }
            break;
        }
    }
    v20 = self->ent->flags & 0x18;
    if (v20)
    {
        if (!v12 || (v21 = 1, v12 == v20))
            v21 = 0;
        self->ent->flags &= 0xFFFFFFE7;
    }
    else
    {
        v21 = 0;
    }
    self->ent->flags |= v12;
    if (self->Physics.iHitEntnum != 2175)
    {
        if (!v21)
        {
            v21 = Path_FailedLookahead(&self->Path);
            if (!v21)
            {
                v22 = Vec2NormalizeTo(self->Physics.vWishDelta, v26);
                v21 = (float)((float)(v26[1] * (float)(self->Physics.vOrigin[1] - self->ent->r.currentOrigin[1]))
                    + (float)(v26[0] * (float)(self->Physics.vOrigin[0] - self->ent->r.currentOrigin[0]))) < (double)(float)((float)v22 * (float)0.25);
            }
        }
        if (level.gentities[iHitEntnum].sentient)
        {
            //LOWORD(v24) = iHitEntnum;
            v24 = iHitEntnum;
        }
        else
        {
            if (!level.gentities[self->Physics.iHitEntnum].sentient)
            {
                if (v21)
                {
                    wPathLen = self->Path.wPathLen;
                    Actor_RecalcPath(self);
                    if (wPathLen == self->Path.wPathLen)
                        return 0;
                }
                goto LABEL_44;
            }
            v24 = self->Physics.iHitEntnum;
        }
        wDodgeCount = (unsigned __int16)self->Path.wDodgeCount;
        self->Path.wDodgeEntity = v24;
        if (!wDodgeCount)
            self->Path.wDodgeCount = -1;
        if (!v21)
        {
            result = 1;
            self->Physics.iHitEntnum = 2175;
            return result;
        }
        if (!level.gentities[iHitEntnum].sentient)
        {
            if (!level.gentities[self->Physics.iHitEntnum].sentient)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                    2618,
                    0,
                    "%s",
                    "level.gentities[self->Physics.iHitEntnum].sentient");
                return 1;
            }
            return 1;
        }
    }
LABEL_44:
    self->Physics.iHitEntnum = iHitEntnum;
    return 1;
}

void __cdecl Actor_DoMove(actor_s *self)
{
    gentity_s *ent; // r11
    gentity_s *v3; // r11
    unsigned __int16 groundEntNum; // r29
    double v5; // fp0
    double v6; // fp12
    double v7; // fp13
    double v8; // fp10
    double v9; // fp9
    double v10; // fp0
    ai_animmode_t eAnimMode; // r10
    sentient_s *sentient; // r3
    gentity_s *v13; // r11
    double fLookaheadAmount; // fp13
    gentity_s *v15; // r11
    float *currentOrigin; // r3
    double v17; // fp31
    double v18; // fp30
    double v19; // fp29
    int v20; // r5
    pathsort_t *v21; // r4
    int v22; // r3
    int v23; // r30
    int v24; // r8
    double v25; // fp0
    char *v26; // r10
    unsigned int v27; // r9
    float *v28; // r11
    double v29; // fp13
    double v30; // fp12
    double v31; // fp11
    double v32; // fp13
    double v33; // fp13
    double v34; // fp12
    double v35; // fp11
    double v36; // fp13
    float *v37; // r11
    double v38; // fp13
    double v39; // fp12
    double v40; // fp11
    double v41; // fp13
    float *v42; // r11
    double v43; // fp13
    double v44; // fp12
    double v45; // fp11
    double v46; // fp13
    int v47; // r9
    unsigned int *v48; // r10
    double v49; // fp13
    double v50; // fp12
    double v51; // fp11
    double v52; // fp13
    gentity_s *v53; // r11
    double v54; // fp0
    double v55; // fp13
    gentity_s *v56; // r11
    gentity_s *v57; // r11
    unsigned __int16 v58; // r9
    float v59[8]; // [sp+50h] [-360h] BYREF
    unsigned int v60[3]; // [sp+70h] [-340h] BYREF
    char v61; // [sp+7Ch] [-334h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2885, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 2886, 0, "%s", "self->ent");
    if (self->ent->s.number != self->Physics.iEntNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            2887,
            0,
            "%s",
            "self->ent->s.number == self->Physics.iEntNum");
    ent = self->ent;
    if (self->ent->r.mins[0] != self->Physics.vMins[0]
        || ent->r.mins[1] != self->Physics.vMins[1]
        || ent->r.mins[2] != self->Physics.vMins[2])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            2888,
            0,
            "%s",
            "Vec3Compare( self->ent->r.mins, self->Physics.vMins )");
    }
    v3 = self->ent;
    if (self->ent->r.maxs[0] != self->Physics.vMaxs[0]
        || v3->r.maxs[1] != self->Physics.vMaxs[1]
        || v3->r.maxs[2] != self->Physics.vMaxs[2])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            2889,
            0,
            "%s",
            "Vec3Compare( self->ent->r.maxs, self->Physics.vMaxs )");
    }
    if (self->Physics.ePhysicsType == AIPHYS_BAD)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            2890,
            0,
            "%s",
            "self->Physics.ePhysicsType != AIPHYS_BAD");
    groundEntNum = self->Physics.groundEntNum;
    if (self->Physics.ePhysicsType != AIPHYS_NORMAL_ABSOLUTE)
    {
        YawVectors(self->fDesiredBodyYaw, (float *)self, v59);
        v5 = self->Physics.vWishDelta[0];
        v6 = self->Physics.vWishDelta[2];
        v7 = -self->Physics.vWishDelta[1];
        self->Physics.vWishDelta[0] = v59[0] * self->Physics.vWishDelta[0];
        self->Physics.vWishDelta[1] = v59[1] * (float)v5;
        self->Physics.vWishDelta[2] = v59[2] * (float)v5;
        v8 = v59[5];
        v9 = v59[6];
        self->Physics.vWishDelta[0] = (float)(v59[4] * (float)v7) + self->Physics.vWishDelta[0];
        self->Physics.vWishDelta[1] = (float)((float)v8 * (float)v7) + self->Physics.vWishDelta[1];
        v10 = (float)((float)((float)v9 * (float)v7) + self->Physics.vWishDelta[2]);
        self->Physics.vWishDelta[2] = (float)((float)v9 * (float)v7) + self->Physics.vWishDelta[2];
        self->Physics.vWishDelta[2] = (float)v6 + (float)v10;
    }
    eAnimMode = self->eAnimMode;
    self->Physics.fGravity = g_gravity->current.value;
    if (eAnimMode == AI_ANIM_MOVE_CODE
        && self->moveMode
        && Actor_HasPath(self)
        && !self->pCloseEnt.isDefined())
    {
    try_path:
        if (!Actor_PhysicsAndDodge(self)
            || self->Path.lookaheadDir[2] > 4.0
            && self->Physics.vWishDelta[2] > 0.0
            && self->Physics.vOrigin[2] <= (double)self->ent->r.currentOrigin[2]
            || (sentient = self->sentient, sentient->bNearestNodeBad)
            && (Sentient_InvalidateNearestNode(sentient), Sentient_NearestNode(self->sentient),
                self->sentient->bNearestNodeBad))
        {
            v13 = self->ent;
            self->Physics.vOrigin[0] = self->ent->r.currentOrigin[0] + self->Physics.vWishDelta[0];
            self->Physics.vOrigin[1] = v13->r.currentOrigin[1] + self->Physics.vWishDelta[1];
            self->Physics.vOrigin[2] = v13->r.currentOrigin[2] + self->Physics.vWishDelta[2];
            fLookaheadAmount = self->Path.fLookaheadAmount;
            self->Path.wDodgeEntity = 2175;
            self->Physics.vVelocity[2] = 0.0;
            if (fLookaheadAmount < 64.0)
                self->Path.fLookaheadAmount = 64.0;
        }
        goto LABEL_60;
    }
    if (Actor_ShouldMoveAwayFromCloseEnt(self))
    {
        if (Actor_PhysicsMoveAway(self))
            goto LABEL_60;
        if (self->eAnimMode == AI_ANIM_MOVE_CODE && Actor_HasPath(self))
            goto try_path;
    }
    else
    {
        self->ent->flags &= 0xFFFFFFE7;
        v15 = self->ent;
        self->Physics.vOrigin[0] = self->ent->r.currentOrigin[0];
        self->Physics.vOrigin[1] = v15->r.currentOrigin[1];
        self->Physics.vOrigin[2] = v15->r.currentOrigin[2];
        if (Actor_Physics(&self->Physics))
            goto LABEL_60;
    }
    //Profile_Begin(232);
    currentOrigin = self->ent->r.currentOrigin;
    v17 = *currentOrigin;
    v18 = self->ent->r.currentOrigin[1];
    v19 = self->ent->r.currentOrigin[2];
    v22 = Path_NodesInCylinder(currentOrigin, 384.0, 128.0, v21, v20, (int)v60);
    v23 = 0;
    v24 = 0;
    v25 = FLT_MAX;
    if (v22 >= 4)
    {
        v26 = &v61;
        v27 = ((unsigned int)(v22 - 4) >> 2) + 1;
        v24 = 4 * v27;
        do
        {
            v28 = (float *)*((unsigned int *)v26 - 3);
            v29 = (float)(v28[6] - (float)v18);
            v30 = (float)(v28[7] - (float)v19);
            v31 = (float)(v28[5] - (float)v17);
            v32 = (float)((float)((float)v31 * (float)v31)
                + (float)((float)((float)v30 * (float)v30) + (float)((float)v29 * (float)v29)));
            if (v25 >= v32)
            {
                v25 = v32;
                v23 = *((unsigned int *)v26 - 3);
            }
            v33 = (float)(*(float *)(*(unsigned int *)v26 + 24) - (float)v18);
            v34 = (float)(*(float *)(*(unsigned int *)v26 + 28) - (float)v19);
            v35 = (float)(*(float *)(*(unsigned int *)v26 + 20) - (float)v17);
            v36 = (float)((float)((float)v35 * (float)v35)
                + (float)((float)((float)v34 * (float)v34) + (float)((float)v33 * (float)v33)));
            if (v25 >= v36)
            {
                v25 = v36;
                v23 = *(unsigned int *)v26;
            }
            v37 = (float *)*((unsigned int *)v26 + 3);
            v38 = (float)(v37[6] - (float)v18);
            v39 = (float)(v37[7] - (float)v19);
            v40 = (float)(v37[5] - (float)v17);
            v41 = (float)((float)((float)v40 * (float)v40)
                + (float)((float)((float)v39 * (float)v39) + (float)((float)v38 * (float)v38)));
            if (v25 >= v41)
            {
                v25 = v41;
                v23 = *((unsigned int *)v26 + 3);
            }
            v42 = (float *)*((unsigned int *)v26 + 6);
            v43 = (float)(v42[6] - (float)v18);
            v44 = (float)(v42[7] - (float)v19);
            v45 = (float)(v42[5] - (float)v17);
            v46 = (float)((float)((float)v45 * (float)v45)
                + (float)((float)((float)v44 * (float)v44) + (float)((float)v43 * (float)v43)));
            if (v25 >= v46)
            {
                v25 = v46;
                v23 = *((unsigned int *)v26 + 6);
            }
            --v27;
            v26 += 48;
        } while (v27);
    }
    if (v24 < v22)
    {
        v47 = v22 - v24;
        v48 = &v60[3 * v24];
        do
        {
            v49 = (float)(*(float *)(*v48 + 24) - (float)v18);
            v50 = (float)(*(float *)(*v48 + 28) - (float)v19);
            v51 = (float)(*(float *)(*v48 + 20) - (float)v17);
            v52 = (float)((float)((float)v51 * (float)v51)
                + (float)((float)((float)v50 * (float)v50) + (float)((float)v49 * (float)v49)));
            if (v25 >= v52)
            {
                v25 = v52;
                v23 = *v48;
            }
            --v47;
            v48 += 3;
        } while (v47);
    }
    //Profile_EndInternal(0);
    if (v23)
    {
        v53 = self->ent;
        v54 = (float)(*(float *)(v23 + 28) - self->ent->r.currentOrigin[2]);
        if (v54 <= 8.0)
        {
            if (v54 < 0.0)
            {
                if (v54 >= -18.0)
                    v54 = 0.0;
                else
                    v54 = -8.0;
            }
        }
        else
        {
            v54 = 8.0;
        }
        self->Physics.vOrigin[0] = v53->r.currentOrigin[0];
        self->Physics.vOrigin[1] = v53->r.currentOrigin[1];
        v55 = v53->r.currentOrigin[2];
        self->Physics.vOrigin[2] = v53->r.currentOrigin[2];
        self->Physics.vVelocity[2] = 0.0;
        self->Physics.vOrigin[2] = (float)v55 + (float)v54;
    }
LABEL_60:
    v56 = self->ent;
    v56->r.currentOrigin[0] = self->Physics.vOrigin[0];
    v56->r.currentOrigin[1] = self->Physics.vOrigin[1];
    v56->r.currentOrigin[2] = self->Physics.vOrigin[2];
    v57 = self->ent;
    v58 = self->Physics.groundEntNum;
    self->Physics.ePhysicsType = AIPHYS_BAD;
    v57->s.groundEntityNum = v58;
    if (groundEntNum != self->Physics.groundEntNum)
        Scr_Notify(self->ent, scr_const.groundEntChanged, 0);
}

bool __cdecl Actor_IsAtGoal(actor_s *self)
{
    gentity_s *ent; // r28

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3217, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3218, 0, "%s", "self->sentient");
    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3221, 0, "%s", "ent");
    if (!(unsigned __int8)Actor_PointAtGoal(ent->r.currentOrigin, &self->codeGoal))
        return 0;
    if (Path_Exists(&self->Path))
    {
        if (!(unsigned __int8)Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal))
            return 0;
        if (self->sentient->pClaimedNode && !(unsigned __int8)Actor_KeepClaimedNode(self))
            return Actor_PointAt(ent->r.currentOrigin, self->sentient->pClaimedNode->constant.vOrigin);
    }
    else if (self->sentient->pClaimedNode)
    {
        return Actor_IsNearClaimedNode(self);
    }
    return 1;
}

bool __cdecl Actor_FindPathToGoalDirectInternal(actor_s *self)
{
    double v3; // fp31
    int *SuppressionPlanes; // r25
    sentient_s *sentient; // r3
    pathnode_t *v6; // r24
    actor_goal_s *p_codeGoal; // r30
    pathnode_t *v8; // r3
    int *v9; // r6
    pathnode_t *v10; // r29
    double v11; // fp0
    double v12; // fp13
    sentient_s *v13; // r11
    bool prone; // r10
    double v15; // fp0
    team_t eTeam; // r26
    double v17; // fp31
    pathnode_t *CloseNode; // r27
    const float *vOrigin; // r28
    int v20; // r8
    double v21; // fp0
    path_t *p_Path; // r3
    float *currentOrigin; // r6
    nearestNodeHeightCheck v24; // [sp+8h] [-408h]
    int v25; // [sp+Ch] [-404h]
    float v26; // [sp+60h] [-3B0h] BYREF
    float v27; // [sp+64h] [-3ACh]
    float v28; // [sp+68h] [-3A8h]
    _BYTE v29[4]; // [sp+6Ch] [-3A4h] BYREF
    float v30[4]; // [sp+70h] [-3A0h] BYREF
    float v31[4]; // [sp+80h] [-390h] BYREF
    float v32[4][2]; // [sp+90h] [-380h] BYREF
    pathsort_t v33[64]; // [sp+B0h] [-360h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3285, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3286, 0, "%s", "self->sentient");
    if (Path_Exists(&self->Path))
    {
        if (!Path_NeedsReevaluation(&self->Path)
            && (unsigned __int8)Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal))
        {
            return 1;
        }
        Actor_ClearPath(self);
    }
    else if (self->meleeAttackDist == 0.0
        && (unsigned __int8)Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal))
    {
        return 1;
    }
    v3 = I_fabs(self->sideMove);
    if (v3 > (float)(self->codeGoal.radius - (float)15.0))
        v3 = (float)(self->codeGoal.radius - (float)15.0);
    if (v3 <= 0.0)
        return Actor_FindPath(self, self->codeGoal.pos, 1, 0);
    SuppressionPlanes = (int *)Actor_GetSuppressionPlanes(self, v32, v31);
    Sentient_InvalidateNearestNode(self->sentient);
    sentient = self->sentient;
    if (SuppressionPlanes)
    {
        v6 = Sentient_NearestNodeSuppressed(sentient, v32, v31, (int)SuppressionPlanes);
        if (!v6)
            return 0;
        p_codeGoal = &self->codeGoal;
        v8 = Path_NearestNodeNotCrossPlanes(
            self->codeGoal.pos,
            v33,
            -2,
            192.0,
            (float (*)[2])0x40,
            v32[0],
            (int)v31,
            SuppressionPlanes,
            (int)v29,
            v24);
    }
    else
    {
        v6 = Sentient_NearestNode(sentient);
        if (!v6)
            return 0;
        p_codeGoal = &self->codeGoal;
        v8 = Path_NearestNode(self->codeGoal.pos, v33, -2, 192.0, v9, (int)v29, (nearestNodeHeightCheck)64);
    }
    v10 = v8;
    if (!v8)
        return 0;
    v11 = -self->Path.lookaheadDir[0];
    v12 = self->Path.lookaheadDir[1];
    if (self->sideMove < 0.0)
        v3 = -v3;
    v13 = self->sentient;
    v30[2] = self->codeGoal.pos[2];
    prone = self->Physics.prone;
    v15 = (float)((float)((float)v11 * (float)v3) + p_codeGoal->pos[1]);
    v30[0] = (float)((float)v12 * (float)v3) + p_codeGoal->pos[0];
    v30[1] = v15;
    eTeam = v13->eTeam;
    if (prone)
        v17 = 10.0;
    else
        v17 = 18.0;
    CloseNode = Path_FindCloseNode(eTeam, v8, v30, 1);
    vOrigin = CloseNode->constant.vOrigin;
    Path_PredictionTrace(CloseNode->constant.vOrigin, v30, 2175, 8519697, &v26, v17, v20);
    if ((unsigned __int8)Actor_PointAtGoal(&v26, p_codeGoal))
    {
        v10 = CloseNode;
    }
    else
    {
        if ((unsigned __int8)Actor_PointAtGoal(vOrigin, p_codeGoal))
        {
            v10 = CloseNode;
            v26 = *vOrigin;
            v27 = CloseNode->constant.vOrigin[1];
            v21 = CloseNode->constant.vOrigin[2];
        }
        else
        {
            v26 = p_codeGoal->pos[0];
            v27 = p_codeGoal->pos[1];
            v21 = p_codeGoal->pos[2];
        }
        v28 = v21;
    }
    p_Path = &self->Path;
    currentOrigin = self->ent->r.currentOrigin;
    if (SuppressionPlanes)
        Path_FindPathFromToNotCrossPlanes(p_Path, eTeam, v6, currentOrigin, v10, &v26, v32, v31, v24, v25);
    else
        Path_FindPathFromTo(p_Path, eTeam, v6, currentOrigin, v10, &v26, 1);
    return Actor_HasPath(self);
}

void __cdecl Actor_FindPathToGoalDirect(actor_s *self)
{
    bool PathToGoalDirectInternal; // r27

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    if (Path_Exists(&self->Path) || level.time >= self->pathWaitTime)
    {
        PathToGoalDirectInternal = Actor_FindPathToGoalDirectInternal(self);
        if (!self)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
        if (!Path_Exists(&self->Path))
        {
            self->pPileUpActor = 0;
            self->pPileUpEnt = 0;
            if (!PathToGoalDirectInternal)
                Actor_HandleInvalidPath(self);
        }
    }
    else
    {
        self->pPileUpActor = 0;
        self->pPileUpEnt = 0;
    }
}

int __cdecl Actor_FindPathToClaimNode(actor_s *self, pathnode_t *node)
{
    bool v4; // r25
    int result; // r3
    bool Path; // r28
    float *currentOrigin; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3452, 0, "%s", "self");
    if (self->ent->tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3453, 0, "%s", "!self->ent->tagInfo");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3454, 0, "%s", "node");
    v4 = Path_Exists(&self->Path);
    if (!v4 && level.time < self->pathWaitTime)
    {
        result = 0;
        self->pPileUpEnt = 0;
        self->pPileUpActor = 0;
        return result;
    }
    Path = Actor_FindPath(self, node->constant.vOrigin, 1, 0);
    if (Path_Exists(&self->Path))
    {
        if (!v4)
        {
            currentOrigin = self->ent->r.currentOrigin;
            if (((1 << node->constant.type) & 0x3C0) != 0)
            {
                if (Actor_PointAt(currentOrigin, node->constant.vOrigin))
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
                        3478,
                        0,
                        "%s",
                        "!Actor_PointAt( self->ent->r.currentOrigin, node->constant.vOrigin )");
            }
            else if (Actor_PointNearNode(currentOrigin, node)
                && (unsigned __int8)Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal))
            {
                Actor_ClearPath(self);
                result = 1;
                self->pPileUpEnt = 0;
                self->pPileUpActor = 0;
                return result;
            }
        }
        return 1;
    }
    self->pPileUpActor = 0;
    self->pPileUpEnt = 0;
    if (!Path)
        Actor_HandleInvalidPath(self);
    return Path;
}

int __cdecl Actor_CheckStop(actor_s *self, bool canUseEnemyGoal, pathnode_t *node, int hadPath)
{
    sentient_s *TargetSentient; // r29
    double v10; // fp9
    float *currentOrigin; // r3
    double v12; // fp8
    double v13; // fp11
    double v14; // fp12
    double v15; // fp10
    double v16; // fp1
    pathnode_t *pClaimedNode; // r11
    char v18; // r3
    unsigned __int8 v19; // r11
    float v20[20]; // [sp+50h] [-50h] BYREF

    TargetSentient = Actor_GetTargetSentient(self);
    if (!TargetSentient)
        return 0;
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4490, 0, "%s", "self");
    if (!Path_Exists(&self->Path)
        || !Actor_EnemyInPathFightDist(self, TargetSentient)
        || node && Actor_PointNearNode(self->ent->r.currentOrigin, node)
        || self->arrivalInfo.animscriptOverrideRunTo
        && Actor_PointNear(self->ent->r.currentOrigin, self->arrivalInfo.animscriptOverrideRunToPos)
        || !Actor_CanSeeEnemy(self))
    {
        return 0;
    }
    v10 = self->Path.lookaheadDir[0];
    currentOrigin = self->ent->r.currentOrigin;
    v12 = self->Path.lookaheadDir[1];
    v13 = TargetSentient->ent->r.currentOrigin[0];
    v14 = (float)((float)(self->Path.lookaheadDir[1] * self->Path.fLookaheadDist) + self->ent->r.currentOrigin[1]);
    v15 = TargetSentient->ent->r.currentOrigin[1];
    v20[0] = (float)(self->Path.lookaheadDir[0] * self->Path.fLookaheadDist) + *currentOrigin;
    v20[1] = v14;
    if ((float)((float)((float)v12 * (float)((float)v14 - (float)v15))
        + (float)((float)v10 * (float)(v20[0] - (float)v13))) > 0.0
        && (float)((float)((float)((float)v14 - (float)v15) * (float)((float)v14 - (float)v15))
            + (float)((float)(v20[0] - (float)v13) * (float)(v20[0] - (float)v13))) < (double)(float)(self->Path.fLookaheadDist * self->Path.fLookaheadDist))
    {
        if (canUseEnemyGoal && !self->useEnemyGoal)
        {
            self->useEnemyGoal = 1;
            Actor_UpdateGoalPos(self);
        }
        return 1;
    }
    if ((unsigned __int8)Actor_PointAtGoal(currentOrigin, &self->codeGoal))
    {
        if (Actor_CanSeeEnemy(self))
        {
            if (!node)
                return 1;
            v16 = Vec2Distance(TargetSentient->ent->r.currentOrigin, node->constant.vOrigin);
            if (Path_DistanceGreaterThan(&self->Path, v16))
                return 1;
        }
    }
    if (!canUseEnemyGoal
        || self->useEnemyGoal
        || (unsigned __int8)Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal)
        || !(unsigned __int8)Actor_IsAlongPath(self, TargetSentient->ent->r.currentOrigin, v20, hadPath))
    {
        return 0;
    }
    self->useEnemyGoal = 1;
    Actor_UpdateGoalPos(self);
    pClaimedNode = self->sentient->pClaimedNode;
    if (pClaimedNode && (unsigned __int8)Actor_PointAtGoal(pClaimedNode->constant.vOrigin, &self->codeGoal))
        return ((char *)self->sentient->pClaimedNode - (char *)node) != 0;
        //return (_cntlzw((char *)self->sentient->pClaimedNode - (char *)node) & 0x20) == 0;
    if (!node)
        return 1;
    v18 = Actor_PointAtGoal(node->constant.vOrigin, &self->codeGoal);
    v19 = 0;
    if (!v18)
        return 1;
    return v19;
}

void __cdecl Actor_TryPathToArrivalPos(actor_s *self)
{
    path_t *p_Path; // r30

    p_Path = &self->Path;
    Path_Backup(&self->Path);
    if (!Actor_FindPath(self, self->arrivalInfo.animscriptOverrideRunToPos, 1, 0))
    {
        Actor_ClearPath(self);
        Path_Restore(p_Path);
        self->arrivalInfo.animscriptOverrideRunTo = 0;
        self->arrivalInfo.arrivalNotifyRequested = 0;
    }
}

void __cdecl Actor_FindPathToFixedNode(actor_s *self)
{
    double v2; // fp31
    double v3; // fp30
    double v4; // fp29
    int v5; // r11
    bool v6; // r11
    char v7; // r3
    AISpecies species; // r11
    sentient_s *sentient; // r29
    pathnode_t *node; // r30
    pathnode_t *pClaimedNode; // r27
    pathnode_t *v12; // r11
    bool v13; // r11
    bool v14; // zf

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3921, 0, "%s", "self");
    self->useEnemyGoal = 0;
    v2 = self->codeGoal.pos[0];
    v3 = self->codeGoal.pos[1];
    v4 = self->codeGoal.pos[2];
    Actor_UpdateGoalPos(self);
    if (self->codeGoal.pos[0] != v2 || self->codeGoal.pos[1] != v3 || (v5 = 1, self->codeGoal.pos[2] != v4))
        v5 = 0;
    v6 = v5 == 0;
    self->goalPosChanged = v6;
    if (v6)
    {
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
        self->commitToFixedNode = 0;
    }
    if ((unsigned __int8)Actor_InFixedNodeExposedCombat(self))
        return;
    v7 = Actor_KeepClaimedNode(self);
    species = self->species;
    if (!v7)
    {
        if (species)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3951, 0, "%s", "Actor_UsingCoverNodes( self )");
        if ((unsigned __int8)Actor_IsDoingCover(self))
            self->Path.flags |= 0x100u;
        sentient = self->sentient;
        node = self->codeGoal.node;
        pClaimedNode = sentient->pClaimedNode;
        if (!(unsigned __int8)Actor_IsFixedNodeUseable(self) && !self->commitToFixedNode)
            goto LABEL_14;
        if (self->arrivalInfo.animscriptOverrideRunTo)
        {
            Actor_TryPathToArrivalPos(self);
        }
        else
        {
            if (!node)
            {
                Actor_FindPathToGoalDirect(self);
                if (Actor_HasPath(self) && !self->commitToFixedNode)
                {
                    self->commitToFixedNode = 1;
                    Scr_Notify(self->ent, scr_const.start_move, 0);
                }
                goto LABEL_24;
            }
            if (!(unsigned __int8)Actor_FindPathToClaimNode(self, node))
            {
                Actor_TeamMoveBlocked(self);
                node = 0;
                goto LABEL_24;
            }
            if (!self->commitToFixedNode && Actor_HasPath(self))
            {
                self->commitToFixedNode = 1;
                Scr_Notify(self->ent, scr_const.start_move, 0);
            }
        }
        if (node)
        {
            if (!Path_CanClaimNode(node, sentient))
                goto LABEL_37;
            goto LABEL_36;
        }
    LABEL_24:
        if (!Actor_HasPath(self))
        {
        LABEL_37:
            if (!self->goalPosChanged)
            {
                v12 = sentient->pClaimedNode;
                if (!v12 || (v14 = pClaimedNode != v12, v13 = 1, !v14))
                    v13 = 0;
                self->goalPosChanged = v13;
                if (v13)
                {
                    Scr_Notify(self->ent, scr_const.goal_changed, 0);
                    self->commitToFixedNode = 0;
                }
            }
            return;
        }
    LABEL_36:
        Sentient_ClaimNode(sentient, node);
        goto LABEL_37;
    }
    if (species)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 3942, 0, "%s", "Actor_UsingCoverNodes( self )");
    if (Actor_HasPath(self))
    {
    LABEL_14:
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
    }
}

void __cdecl Actor_FindPathToGoal(actor_s *self)
{
    int v2; // r24
    bool v3; // r26
    double v4; // fp31
    double v5; // fp30
    double v6; // fp29
    int v7; // r11
    bool v8; // r11
    char v9; // r3
    AISpecies species; // r11
    pathnode_t *ClaimedNode; // r30
    sentient_s *sentient; // r28
    pathnode_t *pClaimedNode; // r27
    pathnode_t *v14; // r11
    bool v15; // r11
    bool v16; // zf

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4032, 0, "%s", "self");
    if (self->ent->tagInfo)
    {
        Actor_ClearPath(self);
        return;
    }
    v2 = Path_Exists(&self->Path);
    if (!v2 && level.time < self->iTeamMoveWaitTime)
        return;
    Actor_UpdateDesiredChainPos(self);
    if (self->fixedNode)
    {
        Actor_FindPathToFixedNode(self);
        return;
    }
    v3 = Actor_KnowAboutEnemy(self, v2);
    if (!v3)
        self->useEnemyGoal = 0;
    v4 = self->codeGoal.pos[0];
    v5 = self->codeGoal.pos[1];
    v6 = self->codeGoal.pos[2];
    Actor_UpdateGoalPos(self);
    if (self->codeGoal.pos[0] != v4 || self->codeGoal.pos[1] != v5 || (v7 = 1, self->codeGoal.pos[2] != v6))
        v7 = 0;
    v8 = v7 == 0;
    self->goalPosChanged = v8;
    if (v8)
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
    v9 = Actor_KeepClaimedNode(self);
    species = self->species;
    if (v9)
    {
        if (species)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4070, 0, "%s", "Actor_UsingCoverNodes( self )");
        if (Actor_HasPath(self))
            goto LABEL_21;
        return;
    }
    if (species)
    {
        ClaimedNode = 0;
    }
    else
    {
        if (Actor_IsReactingToEnemyDuringReacquireMove(self) && !self->arrivalInfo.animscriptOverrideRunTo)
            return;
        if ((unsigned __int8)Actor_IsDoingCover(self))
            self->Path.flags |= 0x100u;
        ClaimedNode = Actor_FindClaimedNode(self);
    }
    sentient = self->sentient;
    pClaimedNode = sentient->pClaimedNode;
    if (self->arrivalInfo.animscriptOverrideRunTo)
    {
        Actor_TryPathToArrivalPos(self);
        goto LABEL_46;
    }
    if (!ClaimedNode)
    {
        if (!self->useEnemyGoal)
        {
            Actor_FindPathToGoalDirect(self);
            goto LABEL_46;
        }
        self->useEnemyGoal = 0;
        Actor_UpdateGoalPos(self);
        if (!Actor_HasPath(self))
            goto LABEL_46;
        Actor_ClearPath(self);
    LABEL_45:
        Actor_TeamMoveBlocked(self);
        goto LABEL_46;
    }
    if (!(unsigned __int8)Actor_FindPathToClaimNode(self, ClaimedNode))
    {
        if (!Actor_IsSuppressedInAnyway(self))
            Path_MarkNodeInvalid(ClaimedNode, sentient->eTeam);
        Actor_TeamMoveBlocked(self);
        ClaimedNode = 0;
        goto LABEL_46;
    }
    if (!Actor_HasPath(self) && self->useEnemyGoal)
    {
        if (!(unsigned __int8)Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal)
            || !Actor_PointNearNode(self->ent->r.currentOrigin, ClaimedNode))
        {
            self->useEnemyGoal = 0;
            Actor_UpdateGoalPos(self);
        }
        goto LABEL_45;
    }
LABEL_46:
    if ((unsigned __int8)Actor_CheckStop(self, v3, ClaimedNode, v2))
    {
    LABEL_21:
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
        return;
    }
    if (ClaimedNode || Actor_HasPath(self))
        Sentient_ClaimNode(sentient, ClaimedNode);
    if (!self->goalPosChanged)
    {
        v14 = sentient->pClaimedNode;
        if (!v14 || (v16 = pClaimedNode != v14, v15 = 1, !v16))
            v15 = 0;
        self->goalPosChanged = v15;
        if (v15)
            Scr_Notify(self->ent, scr_const.goal_changed, 0);
    }
}

void __cdecl Actor_UpdateOriginAndAngles(actor_s *self)
{
    gentity_s *ent; // r28

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4866, 0, "%s", "self");
    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4868, 0, "%s", "ent");
    if (self->eAnimMode != AI_ANIM_NOPHYSICS)
    {
        if (ent->tagInfo)
        {
            G_SetFixedLink(ent, 1);
            Actor_ClearPath(self);
            Actor_UpdateAnglesAndDelta(self);
            self->Physics.vVelocity[0] = 0.0;
            self->Physics.vVelocity[1] = 0.0;
            self->Physics.vVelocity[2] = 0.0;
            self->Physics.vWishDelta[0] = 0.0;
            self->Physics.vWishDelta[1] = 0.0;
            self->Physics.vWishDelta[2] = 0.0;
            G_CalcTagAxis(ent, 1);
        }
        else
        {
            Actor_UpdateAnglesAndDelta(self);
            Actor_DoMove(self);
            if (level.gentities[self->Physics.iHitEntnum].sentient)
            {
                if (!self->noDodgeMove)
                    Actor_TeamMoveBlocked(self);
            }
            G_TouchEnts(ent, self->Physics.iNumTouch, self->Physics.iTouchEnts);
        }
    }
}

void __cdecl Actor_PredictOriginAndAngles(actor_s *self)
{
    gentity_s *ent; // r28

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4901, 0, "%s", "self");
    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4903, 0, "%s", "ent");
    if (ent->tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 4905, 0, "%s", "!ent->tagInfo");
    if (self->eAnimMode == AI_ANIM_MOVE_CODE)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            4906,
            0,
            "%s",
            "self->eAnimMode != AI_ANIM_MOVE_CODE");
    Actor_UpdateAnglesAndDelta(self);
    Actor_DoMove(self);
    SV_DObjInitServerTime(ent, 0.050000001);
}

void __cdecl Actor_PostThink(actor_s *self)
{
    pathnode_t *pClaimedNode; // r29
    gentity_s *v3; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp", 1091, 0, "%s", "self");
    if (self->eAnimMode == AI_ANIM_UNKNOWN)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor.cpp",
            1092,
            0,
            "%s",
            "self->eAnimMode != AI_ANIM_UNKNOWN");
    Actor_UpdateOriginAndAngles(self);
    if (self->eAnimMode != AI_ANIM_MOVE_CODE)
    {
        if (ai_showPaths->current.integer)
            Path_DebugDraw(&self->Path, self->ent->r.currentOrigin, 1);
        if (Path_Exists(&self->Path))
            Path_UpdateLookahead_NonCodeMove(&self->Path, self->sentient->oldOrigin, self->ent->r.currentOrigin);
    }
    Actor_CheckNodeClaim(self);
    pClaimedNode = self->sentient->pClaimedNode;
    if (pClaimedNode && self->pCloseEnt.isDefined())
    {
        v3 = self->pCloseEnt.ent();
        if (Actor_PointNearNode(v3->r.currentOrigin, pClaimedNode))
            Actor_NodeClaimRevoked(self, 1000);
    }
}

