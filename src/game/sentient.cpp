#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "sentient.h"
#include "g_main.h"
#include <server/server.h>
#include "actor.h"
#include <universal/com_math.h>
#include "actor_senses.h"
#include "g_local.h"
#include "actor_events.h"
#include <script/scr_vm.h>
#include <script/scr_const.h>
#include "savememory.h"
#include "actor_threat.h"
#include "g_save.h"

SentientGlobals glob;

sentient_s *__cdecl Sentient_Alloc()
{
    sentient_s *sentients; // r11
    int v1; // r10
    sentient_s *i; // r31
    sentient_s *result; // r3

    sentients = level.sentients;
    if (!level.sentients)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 39, 0, "%s", "level.sentients != NULL");
        sentients = level.sentients;
    }
    v1 = 0;
    for (i = sentients; i->inuse; ++i)
    {
        if (++v1 >= 33)
        {
            Com_DPrintf(15, "Sentient allocation failed\n");
            return 0;
        }
    }
    memset(i, 0, sizeof(sentient_s));
    result = i;
    i->inuse = 1;
    return result;
}

void __cdecl Sentient_DissociateSentient(sentient_s *self, sentient_s *other, team_t eOtherTeam)
{
    actor_s *actor; // r30
    gclient_s *client; // r11
    int *meleeAttackerSpot; // r11
    int v9; // r9

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 156, 0, "%s", "self");
    if (self == other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 157, 0, "%s", "self != other");
    if (!other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 158, 0, "%s", "other");
    if (other->eTeam != TEAM_DEAD)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 159, 0, "%s", "other->eTeam == TEAM_DEAD");
    actor = self->ent->actor;
    if (actor)
        Actor_DissociateSentient(self->ent->actor, other, eOtherTeam);
    client = self->ent->client;
    if (client && client->ps.throwBackGrenadeOwner == other->ent->s.number)
        client->ps.throwBackGrenadeOwner = ENTITYNUM_WORLD;
    if (self->targetEnt.isDefined() && self->targetEnt.ent()->sentient == other && actor)
        actor->lastEnemySightPosValid = 0;
    meleeAttackerSpot = self->meleeAttackerSpot;
    v9 = 4;
    do
    {
        if (*meleeAttackerSpot)
        {
            if (*meleeAttackerSpot == other->ent->s.number)
                *meleeAttackerSpot = 0;
        }
        --v9;
        ++meleeAttackerSpot;
    } while (v9);
}

void __cdecl Sentient_GetOrigin(const sentient_s *self, float *vOriginOut)
{
    gentity_s *ent; // r11

    iassert(self);
    iassert(self->ent);
    iassert(self->ent->actor || self->ent->client);
    iassert(vOriginOut);

    ent = self->ent;
    vOriginOut[0] = ent->r.currentOrigin[0];
    vOriginOut[1] = ent->r.currentOrigin[1];
    vOriginOut[2] = ent->r.currentOrigin[2];
}

void __cdecl Sentient_GetForwardDir(sentient_s *self, float *vDirOut)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 215, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 216, 0, "%s", "self->ent");
    if (!self->ent->actor && !self->ent->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            217,
            0,
            "%s",
            "self->ent->actor || self->ent->client");
    if (!vDirOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 218, 0, "%s", "vDirOut");
    YawVectors(self->ent->r.currentAngles[1], (float *)self, vDirOut);
}

void __cdecl Sentient_GetVelocity(const sentient_s *self, float *vVelOut)
{
    actor_s *actor; // r11
    gclient_s *client; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 233, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 234, 0, "%s", "self->ent");
    if (!self->ent->actor && !self->ent->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            235,
            0,
            "%s",
            "self->ent->actor || self->ent->client");
    if (!vVelOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 236, 0, "%s", "vVelOut");
    actor = self->ent->actor;
    if (actor)
    {
        *vVelOut = actor->Physics.vVelocity[0];
        vVelOut[1] = actor->Physics.vVelocity[1];
        vVelOut[2] = actor->Physics.vVelocity[2];
    }
    else
    {
        client = self->ent->client;
        *vVelOut = client->ps.velocity[0];
        vVelOut[1] = client->ps.velocity[1];
        vVelOut[2] = client->ps.velocity[2];
    }
}

void __cdecl Sentient_GetCentroid(sentient_s *self, float *vCentroidOut)
{
    gentity_s *ent; // r11
    double v5; // fp13
    double v6; // fp12
    const float *v7; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 254, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 255, 0, "%s", "self->ent");
    if (!self->ent->actor && !self->ent->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            256,
            0,
            "%s",
            "self->ent->actor || self->ent->client");
    if (!vCentroidOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 257, 0, "%s", "vCentroidOut");
    ent = self->ent;
    *vCentroidOut = self->ent->r.currentOrigin[0];
    vCentroidOut[1] = ent->r.currentOrigin[1];
    v5 = ent->r.currentOrigin[2];
    vCentroidOut[2] = ent->r.currentOrigin[2];
    if (self->ent->actor)
    {
        v6 = 72.0;
        v7 = actorMins;
    }
    else
    {
        v6 = 70.0;
        v7 = playerMins;
    }
    vCentroidOut[2] = (float)((float)((float)((float)v6 - v7[2]) * (float)0.5) + (float)v5) + v7[2];
}

void __cdecl Sentient_GetEyePosition(const sentient_s *self, float *vEyePosOut)
{
    actor_s *actor; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 281, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 282, 0, "%s", "self->ent");
    if (!self->ent->actor && !self->ent->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            283,
            0,
            "%s",
            "self->ent->actor || self->ent->client");
    if (!vEyePosOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 284, 0, "%s", "vEyePosOut");
    actor = self->ent->actor;
    if (actor)
        Actor_GetEyePosition(actor, vEyePosOut);
    else
        G_GetPlayerViewOrigin(&self->ent->client->ps, vEyePosOut);
}

void __cdecl Sentient_GetDebugEyePosition(const sentient_s *self, float *vEyePosOut)
{
    actor_s *actor; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 310, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 311, 0, "%s", "self->ent");
    if (!self->ent->actor && !self->ent->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            312,
            0,
            "%s",
            "self->ent->actor || self->ent->client");
    if (!vEyePosOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 313, 0, "%s", "vEyePosOut");
    actor = self->ent->actor;
    if (actor)
        Actor_GetDebugEyePosition(actor, vEyePosOut);
    else
        G_GetPlayerViewOrigin(&self->ent->client->ps, vEyePosOut);
}

float __cdecl Sentient_GetHeadHeight(const sentient_s *self)
{
    double v2; // fp1

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 335, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 336, 0, "%s", "self->ent");
    if (!self->ent->actor && !self->ent->sentient)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            337,
            0,
            "%s",
            "self->ent->actor || self->ent->sentient");
    if (self->ent->actor)
        v2 = 6.0;
    else
        v2 = (float)((float)70.0 - (float)60.0);
    return *((float *)&v2 + 1);
}

void __cdecl Sentient_UpdateActualChainPos(sentient_s *self)
{
    pathnode_t *ChainPos; // r3
    float v3[4]; // [sp+50h] [-20h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 359, 0, "%s", "self");
    if (self->iActualChainPosTime != level.time)
    {
        self->iActualChainPosTime = level.time;
        Sentient_GetOrigin(self, v3);
        ChainPos = Path_FindChainPos(v3, self->pActualChainPos);
        if (ChainPos)
            self->pActualChainPos = ChainPos;
    }
}

pathnode_t *__cdecl Sentient_NearestNode(sentient_s *self)
{
    pathnode_t *node; // r3
    float vOrigin[3]; // [sp+68h] [-348h] BYREF
    int iNodeCount;
    pathsort_t nodes[64]; // [sp+80h] [-330h] BYREF

    iassert(self);

    if (!self->bNearestNodeValid)
    {
        Sentient_GetOrigin(self, vOrigin); // KISAKTODO: logic upgrade to blops? Logic wrong?
        node = Path_NearestNodeNotCrossPlanes(
            vOrigin,
            nodes,
            -2,
            192.0f,
            0,
            0,
            0,
            &iNodeCount,
            64,
            NEAREST_NODE_DO_HEIGHT_CHECK);

        if (node)
        {
            self->pNearestNode = node;
            self->bNearestNodeBad = 0;
        }
        else
        {
            if (iNodeCount)
            {
                iassert(nodes[0].node);
                self->pNearestNode = nodes[0].node;
            }
            self->bNearestNodeBad = 1;
        }
        self->bNearestNodeValid = 1;
    }
    iassert(self->bNearestNodeValid);

    return self->pNearestNode;
}

pathnode_t *__cdecl Sentient_NearestNodeSuppressed(
    sentient_s *self,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount)
{
    float vOrigin[3]; // [sp+68h] [-348h] BYREF
    pathsort_t nodes[64]; // [sp+80h] [-330h] BYREF
    int iNodeCount;

    iassert(self);

    Sentient_GetOrigin(self, vOrigin);

    return Path_NearestNodeNotCrossPlanes(
        vOrigin,
        nodes,
        -2,
        192.0,
        vNormal,
        fDist,
        iPlaneCount,
        &iNodeCount,
        64,
        NEAREST_NODE_DO_HEIGHT_CHECK);
}

void __cdecl Sentient_InvalidateNearestNode(sentient_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 448, 0, "%s", "self");
    self->bNearestNodeValid = 0;
}

void __cdecl Sentient_SetEnemy(sentient_s *self, gentity_s *enemy, int bNotify)
{
    sentient_s *sentient; // r30
    int iEnemyNotifyTime; // r11
    actor_s *actor; // r30
    sentient_s *v9; // r11
    bool provideCoveringFire; // r11

    iassert(self);
    iassert(self->ent != enemy);

    if (!self->targetEnt.isDefined())
    {
        if (!enemy)
            return;
        goto LABEL_18;
    }
    sentient = self->targetEnt.ent()->sentient;
    if (bNotify)
    {
        if (sentient)
        {
            iEnemyNotifyTime = self->iEnemyNotifyTime;
            if (iEnemyNotifyTime)
            {
                if (level.time >= iEnemyNotifyTime)
                {
                    Actor_BroadcastTeamEvent(self, 4);
                    self->iEnemyNotifyTime = 0;
                }
            }
        }
    }
    if (self->targetEnt.ent() != enemy)
    {
        if (sentient)
        {
            if (bNotify && self->iEnemyNotifyTime)
            {
                Actor_BroadcastTeamEvent(self, 4);
                self->iEnemyNotifyTime = 0;
            }
            --sentient->attackerCount;
        }
    LABEL_18:
        actor = self->ent->actor;
        if (enemy && (v9 = enemy->sentient) != 0)
        {
            ++v9->attackerCount;
            self->iEnemyNotifyTime = level.time + 1000;
        }
        else
        {
            self->iEnemyNotifyTime = 0;
        }
        self->targetEnt.setEnt(enemy);
        if (bNotify && Scr_IsSystemActive())
            Scr_Notify(self->ent, scr_const.enemy, 0);
        if (actor)
        {
            if (actor->useEnemyGoal)
            {
                actor->useEnemyGoal = 0;
                Actor_UpdateGoalPos(actor);
            }
            provideCoveringFire = actor->provideCoveringFire;
            actor->iPotentialCoverNodeCount = 0;
            actor->iPotentialReacquireNodeCount = 0;
            if (provideCoveringFire && enemy && enemy->sentient)
            {
                actor->lastEnemySightPosValid = 1;
                Sentient_GetEyePosition(enemy->sentient, actor->lastEnemySightPos);
            }
            else
            {
                actor->lastEnemySightPosValid = 0;
                Actor_UpdateLastEnemySightPos(actor);
            }
        }
    }
}

sentient_s *__cdecl Sentient_FirstSentient(int iTeamFlags)
{
    int v2; // r10
    team_t *i; // r11

    if (iTeamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            564,
            0,
            "%s",
            "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    v2 = 0;
    for (i = &level.sentients->eTeam; !*((_BYTE *)i + 106) || ((1 << *i) & iTeamFlags) == 0; i += 29)
    {
        if (++v2 >= 33)
            return 0;
    }
    return &level.sentients[v2];
}

sentient_s *__cdecl Sentient_NextSentient(sentient_s *pPrevSentient, int iTeamFlags)
{
    sentient_s *sentients; // r11
    int v5; // r10
    int i; // r9

    if (iTeamFlags > 31)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            593,
            0,
            "%s",
            "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1");
    if (!pPrevSentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 595, 0, "%s", "pPrevSentient");
    sentients = level.sentients;
    if (pPrevSentient < level.sentients || pPrevSentient >= &level.sentients[33])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            596,
            0,
            "%s",
            "pPrevSentient >= level.sentients && pPrevSentient < level.sentients + MAX_SENTIENTS");
        sentients = level.sentients;
    }
    if (pPrevSentient != &sentients[pPrevSentient - sentients])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            597,
            0,
            "%s",
            "pPrevSentient == level.sentients + (pPrevSentient - level.sentients)");
        sentients = level.sentients;
    }
    v5 = pPrevSentient - sentients + 1;
    if (v5 >= 33)
        return 0;
    for (i = (int)&sentients[v5].eTeam; !*(_BYTE *)(i + 106) || ((1 << *(unsigned int *)i) & iTeamFlags) == 0; i += 116)
    {
        if (++v5 >= 33)
            return 0;
    }
    return &sentients[v5];
}

const char *pszTeamName[5] =
{ "invalid", "axis", "allies", "neutral", "dead" };

const char *__cdecl Sentient_NameForTeam(unsigned int eTeam)
{
    if (eTeam > 4)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            625,
            0,
            "%s",
            "eTeam >= 0 && eTeam < TEAM_NUM_TEAMS");
    return pszTeamName[eTeam];
}

void __cdecl Sentient_SetTeam(sentient_s *self, team_t eTeam)
{
    if (eTeam <= TEAM_FREE || eTeam >= TEAM_NUM_TEAMS)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            652,
            0,
            "%s",
            "eTeam > TEAM_BAD && eTeam < TEAM_NUM_TEAMS");
    if (self->eTeam != eTeam)
        self->eTeam = eTeam;
}

void __cdecl Sentient_ClaimNode(sentient_s *self, pathnode_t *node)
{
    actor_s *actor; // r3
    pathnode_t *pClaimedNode; // r11
    char v6; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 673, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 674, 0, "%s", "self->ent");
    actor = self->ent->actor;
    if (actor && (unsigned __int8)Actor_KeepClaimedNode(actor))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            675,
            0,
            "%s",
            "!self->ent->actor || !Actor_KeepClaimedNode(self->ent->actor)");
    pClaimedNode = self->pClaimedNode;
    if (pClaimedNode != node)
    {
        self->pPrevClaimedNode = pClaimedNode;
        if (pClaimedNode)
            Path_RelinquishNodeNow(self);
        if (node)
        {
            if ((node->constant.spawnflags & 0x40) == 0 || (v6 = 1, level.time > node->dynamic.iFreeTime))
                v6 = 0;
            if (v6)
                Path_ForceClaimNode(node, self);
            else
                Path_ClaimNode(node, self);
        }
    }
}

void __cdecl Sentient_NodeClaimRevoked(sentient_s *self, pathnode_t *node)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 705, 0, "%s", "self");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 706, 0, "%s", "node");
    if (node != self->pClaimedNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 707, 0, "%s", "node == self->pClaimedNode");
    if (!self->ent->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 709, 0, "%s", "self->ent->actor");
    Actor_NodeClaimRevoked(self->ent->actor, 5000);
    if (self->pClaimedNode == node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 713, 0, "%s", "self->pClaimedNode != node");
}

void __cdecl Sentient_StealClaimNode(sentient_s *self, sentient_s *other)
{
    pathnode_t *pClaimedNode; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 726, 0, "%s", "self");
    if (!other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 727, 0, "%s", "other");
    if (!other->ent->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 728, 0, "%s", "other->ent->actor");
    pClaimedNode = other->pClaimedNode;
    if (pClaimedNode)
        Path_ForceClaimNode(pClaimedNode, self);
}

void __cdecl Sentient_BanNearNodes(sentient_s *self)
{
    pathnode_t *pClaimedNode; // r31
    int v3; // r5
    pathsort_t *v4; // r4
    int iNodeCount; // r3
    team_t eTeam; // r27
    double v7; // fp31
    double v8; // fp30
    double v9; // fp29
    double v10; // fp28
    pathsort_t *v11; // r30
    int v12; // r29
    pathnode_t *node; // r31
    float v14; // [sp+50h] [-370h] BYREF
    float v15; // [sp+54h] [-36Ch]
    pathsort_t nodes[64]; // [sp+60h] [-360h] BYREF

    if (level.time - self->banNodeTime >= 0)
    {
        pClaimedNode = self->pClaimedNode;
        self->banNodeTime = level.time + 4950;
        if (pClaimedNode)
        {
            if (Path_IsValidClaimNode(pClaimedNode))
            {
                iNodeCount = Path_NodesInCylinder(pClaimedNode->constant.vOrigin, 80.0, 80.0, nodes, 64, 270332);
                eTeam = self->eTeam;
                v7 = pClaimedNode->constant.forward[0];
                v8 = pClaimedNode->constant.forward[1];
                v9 = pClaimedNode->constant.vOrigin[0];
                v10 = pClaimedNode->constant.vOrigin[1];
                if (iNodeCount > 0)
                {
                    v11 = nodes;
                    v12 = iNodeCount;
                    do
                    {
                        node = v11->node;
                        if ((v11->node->constant.spawnflags & 0x8000) != 0 && node->dynamic.iFreeTime != 0x7FFFFFFF)
                        {
                            v14 = (float)v9 - node->constant.vOrigin[0];
                            v15 = (float)v10 - node->constant.vOrigin[1];
                            Vec2Normalize(&v14);
                            if ((float)((float)(v14 * (float)v7) + (float)(v15 * (float)v8)) <= -0.5
                                && (float)((float)(node->constant.forward[1] * v15) + (float)(node->constant.forward[0] * v14)) >= 0.5)
                            {
                                Path_MarkNodeInvalid(node, eTeam);
                            }
                        }
                        --v12;
                        ++v11;
                    } while (v12);
                }
            }
        }
    }
}

void __cdecl Sentient_UpdatePlayerTrail(gentity_s *ent)
{
    int lastTime; // r11
    int time; // r10
    int v4; // r10
    double v5; // fp13
    double v6; // fp12
    float *v7; // r11
    float v8[12]; // [sp+50h] [-30h] BYREF

    lastTime = glob.lastTime;
    time = level.time;
    if (level.time - glob.lastTime < 0)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            802,
            0,
            "%s",
            "level.time - glob.lastTime >= 0");
        time = level.time;
        lastTime = glob.lastTime;
    }
    if (time - lastTime > g_playerTrailTime->current.integer)
    {
        glob.lastTime = time;
        glob.lastSample = (glob.lastSample + 1) % 2;
        Sentient_GetEyePosition(ent->sentient, v8);
        v4 = level.time;
        v5 = v8[1];
        v6 = v8[2];
        v7 = glob.playerTrail[glob.lastSample];
        *v7 = v8[0];
        v7[1] = v5;
        v7[2] = v6;
        glob.sampleTime[glob.lastSample] = v4;
    }
}

void __cdecl Sentient_WriteGlob(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 819, 0, "%s", "save");
    SaveMemory_SaveWrite(&glob, 40, save);
}

void __cdecl Sentient_ReadGlob(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 826, 0, "%s", "save");
    SaveMemory_LoadRead(&glob, 40, save);
}

void __cdecl Sentient_GetTrailPos(sentient_s *self, float *pos, int *time)
{
    int v6; // r11
    int v7; // r8
    float *v8; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 838, 0, "%s", "self");
    if (!self->ent->actor && !self->ent->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            839,
            0,
            "%s",
            "self->ent->actor || self->ent->client");
    if (self->ent->client)
    {
        v6 = glob.lastSample - 1;
        if (glob.lastSample - 1 < 0)
            v6 = 1;
        v7 = v6;
        v8 = glob.playerTrail[v6];
        *pos = *v8;
        pos[1] = v8[1];
        pos[2] = v8[2];
        *time = glob.sampleTime[v7];
    }
    else
    {
        Sentient_GetEyePosition(self, pos);
        *time = level.time;
    }
}

void __cdecl G_InitSentients()
{
    SentientGlobals *v0; // r11
    int v1; // ctr
    int i; // r11

    v0 = &glob;
    v1 = 10;
    do
    {
        v0->lastTime = 0;
        v0 = (SentientGlobals *)((char *)v0 + 4);
        --v1;
    } while (v1);
    for (i = 0; i < 33; ++i)
        level.sentients[i].inuse = 0;
    Actor_InitThreatBiasGroups();
}

int __cdecl Sentient_NearestNodeDirty(sentient_s *self, bool originChanged)
{
    int result; // r3
    pathnode_t *pNearestNode; // r11
    unsigned __int8 v5; // r11
    bool v6; // zf

    if (originChanged)
        self->originChanged = 1;
    if ((50 * self->ent->s.number + level.time) % 1000)
        return 0;
    if (self->originChanged)
    {
        result = 1;
        self->originChanged = 0;
    }
    else
    {
        pNearestNode = self->pNearestNode;
        if (!pNearestNode)
            return 1;
        v6 = pNearestNode->dynamic.wLinkCount != 0;
        v5 = 0;
        if (!v6)
            return 1;
        return v5;
    }
    return result;
}

void __cdecl Sentient_Dissociate(sentient_s *self)
{
    sentient_s *sentient; // r29
    actor_s *actor; // r31
    team_t eTeam; // r29
    sentient_s *i; // r31

    iassert(self);
    iassert(self->ent);
    
    if (self->targetEnt.isDefined())
    {
        sentient = self->targetEnt.ent()->sentient;
        if (self->targetEnt.ent())
        {
            if (sentient)
                --sentient->attackerCount;
            actor = self->ent->actor;
            self->iEnemyNotifyTime = 0;
            self->targetEnt.setEnt(NULL);
            if (actor)
            {
                if (actor->useEnemyGoal)
                {
                    actor->useEnemyGoal = 0;
                    Actor_UpdateGoalPos(actor);
                }
                actor->iPotentialCoverNodeCount = 0;
                actor->iPotentialReacquireNodeCount = 0;
                actor->lastEnemySightPosValid = 0;
                Actor_UpdateLastEnemySightPos(actor);
            }
        }
    }
    if (self->pClaimedNode)
        Path_RelinquishNodeNow(self);
    eTeam = self->eTeam;
    self->eTeam = TEAM_DEAD;
    for (i = Sentient_FirstSentient(-1); i; i = Sentient_NextSentient(i, -1))
    {
        if (i != self)
            Sentient_DissociateSentient(i, self, eTeam);
    }
}

void __cdecl Sentient_Free(sentient_s *sentient)
{
    sentient_s *sentients; // r11
    gentity_s *ent; // r28
    int i; // r31
    actor_s *v5; // r3

    if (!sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 73, 0, "%s", "sentient");
    sentients = level.sentients;
    if (!level.sentients)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 74, 0, "%s", "level.sentients");
        sentients = level.sentients;
    }
    if (sentient < sentients || sentient >= &sentients[33])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            75,
            0,
            "%s",
            "sentient >= level.sentients && sentient < level.sentients + MAX_SENTIENTS");
        sentients = level.sentients;
    }
    if (&sentients[sentient - sentients] != sentient)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp",
            76,
            0,
            "%s",
            "&level.sentients[sentient - level.sentients] == sentient");
    if (!sentient->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 77, 0, "%s", "sentient->ent");
    if (sentient->ent->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient.cpp", 78, 0, "%s", "sentient->ent->actor == NULL");
    ent = sentient->ent;
    for (i = 0; i < 32; ++i)
    {
        v5 = &level.actors[i];
        if (level.actors[i].inuse && v5->pPileUpEnt == ent)
            Actor_ClearPileUp(v5);
    }
    G_FreeEntityRefs(sentient->ent);
    Sentient_Dissociate(sentient);
    sentient->ent->sentient = 0;
    SentientHandleDissociate(sentient);
    Scr_FreeSentientFields(sentient);
    memset(sentient, 240, sizeof(sentient_s));
    sentient->inuse = 0;
}

