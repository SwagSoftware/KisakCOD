#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_cover.h"
#include <qcommon/mem_track.h>
#include "sentient.h"
#include <client/client.h>
#include "g_main.h"
#include "actor.h"
#include "g_local.h"
#include <universal/com_math.h>
#include "actor_senses.h"
#include "actor_threat.h"
#include <server/sv_game.h>
#include "actor_state.h"
#include "actor_team_move.h"

float debugCoverNodeColors[512][4];
char debugCoverNodeMsg[512][21];
pathnode_t *debugCoverNode[512];

unsigned int debugNodeIndex;

int debugNodeTimestamp;

void __cdecl TRACK_actor_cover()
{
    track_static_alloc_internal(debugCoverNodeColors, 0x2000, "debugCoverNodeColors", 5);
    track_static_alloc_internal(debugCoverNodeMsg, 10752, "debugCoverNodeMsg", 5);
    track_static_alloc_internal(debugCoverNode, 2048, "debugCoverNode", 5);
}

void __cdecl DebugDrawNodeSelectionOverlay()
{
    unsigned int v0; // r24
    float *v1; // r28
    const pathnode_t **v2; // r25
    const char *v3; // r30
    const pathnode_t *v4; // r31
    double v5; // fp0
    __int64 v6; // r11
    double v7; // fp31
    double v8; // fp30
    double v9; // fp29
    double v10; // r5
    const char *v11; // r5
    float v12[2]; // [sp+58h] [-B8h] BYREF
    float v13; // [sp+60h] [-B0h]
    float v14[20]; // [sp+68h] [-A8h] BYREF

    if (debugNodeIndex)
    {
        if (ai_debugCoverSelection->current.enabled)
        {
            CL_GetViewPos(v14);
            v0 = 0;
            if (debugNodeIndex)
            {
                v1 = debugCoverNodeColors[0];
                v2 = (const pathnode_t **)debugCoverNode;
                v3 = debugCoverNodeMsg[0];
                do
                {
                    v4 = *v2;
                    Path_DrawDebugNode(v14, *v2);
                    if (v3)
                    {
                        v12[0] = v4->constant.vOrigin[0];
                        v12[1] = v4->constant.vOrigin[1];
                        v13 = v4->constant.vOrigin[2];
                        if (strncmp(v3, "best", 4u))
                        {
                            if (strncmp(v3, "adv", 3u))
                                v5 = (float)(v13 + (float)16.0);
                            else
                                v5 = (float)(v13 + (float)32.0);
                        }
                        else
                        {
                            v5 = (float)(v13 + (float)48.0);
                        }
                        v13 = v5;
                        HIDWORD(v6) = level.time;
                        LODWORD(v6) = level.time - debugNodeTimestamp;
                        v7 = (float)(v4->constant.vOrigin[0] - v14[0]);
                        v8 = (float)(v4->constant.vOrigin[1] - v14[1]);
                        v9 = (float)(v4->constant.vOrigin[2] - v14[2]);
                        v10 = (float)((float)v6 * (float)0.001);
                        va("%s  (%2.1f)", HIDWORD(v10), LODWORD(v10));
                        G_AddDebugString(
                            v12,
                            v1,
                            (float)((float)sqrtf((float)((float)((float)v8 * (float)v8) + (float)((float)((float)v9 * (float)v9) + (float)((float)v7 * (float)v7)))) * (float)0.0022222223),
                            v11);
                    }
                    ++v0;
                    ++v2;
                    v3 += 21;
                    v1 += 4;
                } while (v0 < debugNodeIndex);
            }
        }
    }
}

// aislop
void DebugDrawNodePicking(const char *msg, actor_s *self, __int64 color, __int64 a4, __int64 a5)
{
    pathnode_t *node = (pathnode_t *)(color >> 32);
    float *colorVec = (float *)&color;

    if (ai_debugCoverSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number)
    {
        if (debugNodeTimestamp == level.time)
        {
            if (debugNodeIndex >= 512)
            {
                Com_PrintWarning(18, "DebugDrawNodePicking: Hit max [%d] debug nodes\n", 512);
                return;
            }
        }
        else
        {
            debugNodeIndex = 0;
        }

        if (msg)
        {
            size_t len = strlen(msg);
            if (len >= 20) // MAX_DEBUG_NODE_STRLEN assumed 20
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
                    113,
                    0,
                    "!msg || strlen(msg) < MAX_DEBUG_NODE_STRLEN");
            }
        }

        // Store node and color for debugging
        debugCoverNode[debugNodeIndex] = node;

        float *destColor = debugCoverNodeColors[debugNodeIndex];
        destColor[0] = colorVec[0];
        destColor[1] = colorVec[1];
        destColor[2] = colorVec[2];
        destColor[3] = colorVec[3];

        if (msg)
        {
            strncpy(debugCoverNodeMsg[debugNodeIndex], msg, 19);
            debugCoverNodeMsg[debugNodeIndex][19] = '\0'; // ensure null-termination
        }
        else
        {
            debugCoverNodeMsg[debugNodeIndex][0] = '\0';
        }

        debugNodeTimestamp = level.time;
        debugNodeIndex++;
    }
}


int __cdecl Actor_Cover_IsWithinNodeAngle(
    const float *pos,
    const pathnode_t *node,
    const pathnodeRange_t *range,
    double a4,
    double a5,
    long double a6)
{
    long double v9; // fp2
    long double v10; // fp2
    double v11; // fp1
    double fAngleMin; // fp0
    double v13; // fp30
    double fAngleMax; // fp1
    double v15; // fp0
    double v16; // fp13

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 141, 0, "%s", "node");
    if (!range)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 142, 0, "%s", "range");
    *((double *)&v9 + 1) = (float)(*pos - node->constant.vOrigin[0]);
    *(double *)&v9 = (float)(pos[1] - node->constant.vOrigin[1]);
    v10 = atan2(v9, a6);
    v11 = AngleNormalize360((float)((float)((float)*(double *)&v10 * (float)57.295776) - node->constant.fAngle));
    fAngleMin = range->fAngleMin;
    v13 = v11;
    if (fAngleMin < 0.0 || fAngleMin >= 360.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
            150,
            0,
            "%s\n\t(range->fAngleMin) = %g",
            HIDWORD(fAngleMin),
            LODWORD(fAngleMin));
    fAngleMax = range->fAngleMax;
    if (fAngleMax < 0.0 || fAngleMax >= 360.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
            151,
            0,
            "%s\n\t(range->fAngleMax) = %g",
            "(range->fAngleMax >= 0 && range->fAngleMax < 360)",
            fAngleMax);
    if (v13 < 0.0 || v13 >= 360.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
            152,
            0,
            "%s\n\t(fAngle) = %g",
            HIDWORD(v13),
            LODWORD(v13));
    v15 = range->fAngleMin;
    v16 = range->fAngleMax;
    if (v15 <= v16)
    {
        if (v13 < v15 || v13 > v16)
            return 0;
    }
    else if (v13 < v15 && v13 > v16)
    {
        return 0;
    }
    return 1;
}

int __cdecl Actor_Cover_NodeRangeValid(const float *pos, const pathnode_t *node, pathnodeRange_t *range)
{
    long double v6; // fp4
    double v7; // fp2
    double v8; // fp1

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 177, 0, "%s", "node");
    if (!range)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 178, 0, "%s", "range");
    v8 = Vec2DistanceSq(pos, node->constant.vOrigin);
    if (v8 >= range->minSqDist)
        return Actor_Cover_IsWithinNodeAngle(pos, node, range, v8, v7, v6);
    else
        return 0;
}

void __cdecl Actor_Cover_InitRange(pathnodeRange_t *rangeOut, const pathnode_t *node)
{
    if (!rangeOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 195, 0, "%s", "rangeOut");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 196, 0, "%s", "node");
    switch (node->constant.type)
    {
    case NODE_COVER_STAND:
    case NODE_COVER_CROUCH:
    case NODE_CONCEALMENT_STAND:
    case NODE_CONCEALMENT_CROUCH:
        rangeOut->fAngleMin = 315.0;
        rangeOut->fAngleMax = 45.0;
        rangeOut->minSqDist = 81225.0;
        break;
    case NODE_COVER_CROUCH_WINDOW:
        rangeOut->fAngleMin = 350.0;
        rangeOut->fAngleMax = 10.0;
        rangeOut->minSqDist = 81225.0;
        break;
    case NODE_COVER_PRONE:
        rangeOut->fAngleMin = 330.0;
        rangeOut->fAngleMax = 30.0;
        rangeOut->minSqDist = 640000.0;
        break;
    case NODE_COVER_RIGHT:
    case NODE_COVER_WIDE_RIGHT:
        rangeOut->fAngleMin = 300.0;
        rangeOut->fAngleMax = 14.0;
        rangeOut->minSqDist = 81225.0;
        break;
    case NODE_COVER_LEFT:
    case NODE_COVER_WIDE_LEFT:
        rangeOut->fAngleMin = 348.0;
        rangeOut->fAngleMax = 60.0;
        rangeOut->minSqDist = 81225.0;
        break;
    case NODE_CONCEALMENT_PRONE:
        rangeOut->fAngleMin = 330.0;
        rangeOut->fAngleMax = 30.0;
        rangeOut->minSqDist = 81225.0;
        break;
    case NODE_TURRET:
        TurretNode_GetAngles(node, &rangeOut->fAngleMin, &rangeOut->fAngleMax);
        rangeOut->minSqDist = 81225.0;
        break;
    default:
        rangeOut->fAngleMin = 305.0;
        rangeOut->fAngleMax = 55.0;
        rangeOut->minSqDist = 0.0;
        break;
    }
}

bool __cdecl Actor_Cover_GetAttackScript(
    actor_s *self,
    const pathnode_t *node,
    scr_animscript_t **pAttackScriptFunc)
{
    bool result; // r3

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 297, 0, "%s", "node");
    switch (node->constant.type)
    {
    case NODE_COVER_STAND:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_stand;
        break;
    case NODE_COVER_CROUCH:
    case NODE_COVER_CROUCH_WINDOW:
    case NODE_TURRET:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_crouch;
        break;
    case NODE_COVER_PRONE:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_prone;
        break;
    case NODE_COVER_RIGHT:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_right;
        break;
    case NODE_COVER_LEFT:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_left;
        break;
    case NODE_COVER_WIDE_RIGHT:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_wide_right;
        break;
    case NODE_COVER_WIDE_LEFT:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.cover_wide_left;
        break;
    case NODE_CONCEALMENT_STAND:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.concealment_stand;
        break;
    case NODE_CONCEALMENT_CROUCH:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.concealment_crouch;
        break;
    case NODE_CONCEALMENT_PRONE:
        result = 1;
        *pAttackScriptFunc = &g_scr_data.anim.concealment_prone;
        break;
    case NODE_GUARD:
        if ((node->constant.spawnflags & 0x80) == 0 || Actor_GetTargetEntity(self) && self->hasThreateningEnemy)
        {
            result = 1;
            *pAttackScriptFunc = &g_scr_data.anim.combat;
        }
        else
        {
            result = 1;
            *pAttackScriptFunc = &g_scr_data.anim.stop;
        }
        break;
    default:
        result = (node->constant.spawnflags & 0x8000) != 0;
        *pAttackScriptFunc = 0;
        break;
    }
    return result;
}

bool __cdecl Actor_Cover_CheckWithEnemy(actor_s *self, const pathnode_t *node, bool checkEnemyRange)
{
    gentity_s *TargetEntity; // r31
    sentient_s *TargetSentient; // r3
    sentient_s *v8; // r29
    bool result; // r3
    int v10; // r11
    sentient_info_t *v11; // r31
    float *vLastKnownPos; // r3
    bool v13; // zf
    pathnodeRange_t nodeRange[5]; // [sp+50h] [-40h] BYREF

    iassert(self);
    iassert(self->sentient);
    iassert(node);

    Actor_Cover_InitRange(nodeRange, node);
    TargetEntity = Actor_GetTargetEntity(self);
    TargetSentient = Actor_GetTargetSentient(self);
    v8 = TargetSentient;

    if (TargetEntity)
    {
        if (!TargetSentient)
            return !checkEnemyRange && !Actor_CanSeeEntity(self, TargetEntity) || Actor_Cover_NodeRangeValid(TargetEntity->r.currentOrigin, node, nodeRange);
    LABEL_14:
        if ((TargetSentient->ent->flags & 4) == 0 && !Actor_CheckIgnore(self->sentient, TargetSentient))
        {
            v10 = v8 - level.sentients;
            v11 = &self->sentientInfo[v10];
            if (self->sentientInfo[v10].lastKnownPosTime > 0
                && (checkEnemyRange || Actor_CanSeeEnemy(self))
                && !(unsigned __int8)Actor_Cover_NodeRangeValid(v8->ent->r.currentOrigin, node, nodeRange))
            {
                vLastKnownPos = v11->vLastKnownPos;
                goto LABEL_24;
            }
        }
        return 1;
    }
    if (TargetSentient)
        goto LABEL_14;
    if (checkEnemyRange && self->pGrenade.isDefined())
    {
        vLastKnownPos = self->pGrenade.ent()->r.currentOrigin;
    LABEL_24:
        v13 = (unsigned __int8)Actor_Cover_NodeRangeValid(vLastKnownPos, node, nodeRange) == 0;
        result = 0;
        if (v13)
            return result;
    }
    return 1;
}

bool Actor_Cover_PickAttackScript(
    actor_s *self,
    const pathnode_t *node,
    bool checkEnemyRange,
    scr_animscript_t **pAttackScriptFunc)
{
    iassert(self);
    iassert(self->sentient);
    iassert(node);
    //iassert(Actor_UsingCoverNodes(self));

    if (!Actor_Cover_GetAttackScript(self, node, pAttackScriptFunc))
    {
        return false;
    }

    if (Actor_Cover_CheckWithEnemy(self, node, checkEnemyRange))
    {
        return *pAttackScriptFunc != NULL;
    }

    *pAttackScriptFunc = NULL;
    return true;
}


// aislop
float Actor_Cover_ScoreOnDistance(actor_s *self, const pathnode_t *node)
{
    float dx = self->ent->r.currentOrigin[0] - node->constant.vOrigin[0];
    float dy = self->ent->r.currentOrigin[1] - node->constant.vOrigin[1];
    float dz = self->ent->r.currentOrigin[2] - node->constant.vOrigin[2];

    float distSq = dx * dx + dy * dy + dz * dz;

    // Clamp to max distance threshold (here: 1440000 == 1200^2)
    float clampedDist = distSq - 1440000.0f;

    if (clampedDist < 0.0f)
    {
        clampedDist = 0.0f;
    }

    // Inverse linear falloff: score = 1.0 - (clampedDist * scale)
    // scale = 1 / 1,440,000 = ~0.00000069444445
    float score = 1.0f - (clampedDist * 0.00000069444445f);

    if (score < 0.0f)
    {
        score = 0.0f;
    }

    return score;
}


float __cdecl Actor_Cover_ScoreOnEngagement(actor_s *self, const pathnode_t *node)
{
    gentity_s *TargetEntity; // r3
    double v5; // fp0
    double v6; // fp13
    double v7; // fp11
    double engageMinFalloffDist; // fp12
    double v9; // fp0
    double engageMaxFalloffDist; // fp13
    double v11; // fp31
    double v12; // fp1
    double v13; // fp31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 481, 0, "%s", "self");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 482, 0, "%s", "node");
    if (self->engageMinDist < (double)self->engageMinFalloffDist)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
            484,
            0,
            "%s",
            "self->engageMinDist >= self->engageMinFalloffDist");
    if (self->engageMaxDist > (double)self->engageMaxFalloffDist)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
            485,
            0,
            "%s",
            "self->engageMaxDist <= self->engageMaxFalloffDist");
    TargetEntity = Actor_GetTargetEntity(self);
    if (!TargetEntity)
        goto LABEL_25;
    v5 = (float)(TargetEntity->r.currentOrigin[0] - node->constant.vOrigin[0]);
    v6 = (float)(TargetEntity->r.currentOrigin[2] - node->constant.vOrigin[2]);
    v7 = (float)(TargetEntity->r.currentOrigin[1] - node->constant.vOrigin[1]);
    engageMinFalloffDist = self->engageMinFalloffDist;
    //v9 = sqrtf((float)((float)((float)v7 * (float)v7) + (float)((float)((float)v5 * (float)v5) + (float)((float)v6 * (float)v6))));
    v9 = sqrtf((float)((float)((float)v7 * (float)v7) + (float)((float)((float)v5 * (float)v5) + (float)((float)v6 * (float)v6))));
    if (v9 < engageMinFalloffDist)
        goto LABEL_25;
    engageMaxFalloffDist = self->engageMaxFalloffDist;
    if (v9 > engageMaxFalloffDist)
        goto LABEL_25;
    if (v9 < self->engageMinDist)
    {
        if (v9 >= engageMinFalloffDist)
        {
            v11 = (float)((float)1.0
                - (float)((float)(self->engageMinDist - (float)v9)
                    / (float)(self->engageMinDist - self->engageMinFalloffDist)));
            if (v11 < 0.0 || v11 > 1.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp",
                    502,
                    1,
                    (const char *)HIDWORD(v11),
                    0,
                    0,
                    0);
            v12 = v11;
            return *((float *)&v12 + 1);
        }
    LABEL_25:
        v12 = 0.0;
        return *((float *)&v12 + 1);
    }
    if (v9 <= self->engageMaxDist)
    {
        v12 = 1.0;
        return *((float *)&v12 + 1);
    }
    if (v9 > engageMaxFalloffDist)
        goto LABEL_25;
    v13 = (float)((float)(self->engageMaxFalloffDist - (float)v9)
        / (float)(self->engageMaxFalloffDist - self->engageMaxDist));
    if (v13 < 0.0 || v13 > 1.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 513, 1, (const char *)HIDWORD(v13), 0, 0, 0);
    v12 = v13;
    return *((float *)&v12 + 1);
}

float __cdecl Actor_Cover_ScoreOnNodeAngle(actor_s *self, const pathnode_t *node)
{
    bool PotentialThreat; // r28
    gentity_s *TargetEntity; // r29
    long double v6; // fp4
    double v7; // fp2
    double v8; // fp1
    double v9; // fp1
    float v11[2]; // [sp+50h] [-50h] BYREF
    float v12[4]; // [sp+58h] [-48h] BYREF
    pathnodeRange_t v13[4]; // [sp+68h] [-38h] BYREF

    PotentialThreat = Actor_GetPotentialThreat(&self->potentialThreat, v11);
    Actor_Cover_InitRange(v13, node);
    TargetEntity = Actor_GetTargetEntity(self);
    if (!TargetEntity || !Actor_CanSeeEnemy(self) && PotentialThreat)
    {
        if (PotentialThreat)
        {
            v12[2] = node->constant.vOrigin[2];
            v12[0] = node->constant.vOrigin[0] + v11[0];
            v12[1] = node->constant.vOrigin[1] + v11[1];
            if ((unsigned __int8)Actor_Cover_IsWithinNodeAngle(v12, node, v13, v8, v7, v6))
            {
                v9 = 1.0;
                return *((float *)&v9 + 1);
            }
        }
    LABEL_9:
        v9 = 0.0;
        return *((float *)&v9 + 1);
    }
    if (!(unsigned __int8)Actor_Cover_IsWithinNodeAngle(TargetEntity->r.currentOrigin, node, v13, v8, v7, v6))
        goto LABEL_9;
    v9 = 1.0;
    return *((float *)&v9 + 1);
}

float __cdecl Actor_Cover_ScoreOnTargetDir(actor_s *self, const pathnode_t *node)
{
    gentity_s *TargetEntity; // r29
    double v5; // fp1
    gentity_s *ent; // r11
    double v7; // fp13
    float v9; // [sp+50h] [-30h] BYREF
    float v10; // [sp+54h] [-2Ch]
    float v11; // [sp+58h] [-28h] BYREF
    float v12; // [sp+5Ch] [-24h]

    TargetEntity = Actor_GetTargetEntity(self);
    if (TargetEntity)
    {
        if (Actor_PointNearNode(self->ent->r.currentOrigin, node))
        {
            v5 = 1.0;
        }
        else
        {
            v9 = TargetEntity->r.currentOrigin[0] - node->constant.vOrigin[0];
            v10 = TargetEntity->r.currentOrigin[1] - node->constant.vOrigin[1];
            Vec2Normalize(&v9);
            ent = self->ent;
            v7 = node->constant.vOrigin[1];
            v11 = node->constant.vOrigin[0] - self->ent->r.currentOrigin[0];
            v12 = (float)v7 - ent->r.currentOrigin[1];
            Vec2Normalize(&v11);
            v5 = (float)((float)((float)((float)(v11 * v9) + (float)(v12 * v10)) + (float)1.0) * (float)0.5);
        }
    }
    else
    {
        v5 = 0.0;
    }
    return *((float *)&v5 + 1);
}

float __cdecl Actor_Cover_ScoreOnPriority(actor_s *self, const pathnode_t *node)
{
    double v2; // fp1

    if ((node->constant.spawnflags & 0x40) != 0)
        v2 = 1.0;
    else
        v2 = 0.0;
    return *((float *)&v2 + 1);
}

float __cdecl Actor_Cover_ScoreOnPlayerLOS(actor_s *self, const pathnode_t *node)
{
    double v2; // fp1

    if (node->dynamic.inPlayerLOSTime >= level.time && self->sentient->eTeam == TEAM_ALLIES)
        v2 = 0.0;
    else
        v2 = 1.0;
    return *((float *)&v2 + 1);
}

float __cdecl Actor_Cover_ScoreOnVisibility(actor_s *self, const pathnode_t *node)
{
    sentient_s *TargetSentient; // r3
    const pathnode_t *v4; // r4
    double v5; // fp1

    TargetSentient = Actor_GetTargetSentient(self);
    if (TargetSentient && (v4 = Sentient_NearestNode(TargetSentient)) != 0 && !Path_NodesVisible(node, v4))
        v5 = 0.0;
    else
        v5 = 1.0;
    return *((float *)&v5 + 1);
}

float __cdecl Actor_Cover_ScoreOnCoverType(actor_s *self, const pathnode_t *node)
{
    double v2; // fp1

    if (((1 << node->constant.type) & 0x1C00) != 0)
        v2 = 0.0;
    else
        v2 = 1.0;
    return *((float *)&v2 + 1);
}

// aislop
float Actor_Cover_GetNodeDistMetric(actor_s *self, const pathnode_t *node)
{
    iassert(self);
    iassert(self->sentient);
    iassert(node);

    float dx = self->ent->r.currentOrigin[0] - node->constant.vOrigin[0];
    float dy = self->ent->r.currentOrigin[1] - node->constant.vOrigin[1];
    float dz = self->ent->r.currentOrigin[2] - node->constant.vOrigin[2];

    float distSq = dx * dx + dy * dy + dz * dz;
    float distDelta = distSq - 1440000.0f;

    float score = 0.0f;
    if (distDelta >= 0.0f)
    {
        score = distDelta * 0.00000069444445f;
    }
    else
    {
        score = -distSq * 0.00000069444445f;
    }

    return (1.0f - score) * ai_coverScore_distance->current.value;
}


float __cdecl Actor_Cover_GetNodeMetric(actor_s *self, const pathnode_t *node)
{
    double NodeDistMetric; // fp31
    double v5; // fp31
    double v6; // fp1
    double v7; // fp0
    double v8; // fp31
    double v9; // fp31
    double v10; // fp1
    double v11; // fp0
    double v12; // fp13
    double v13; // fp0
    double v14; // fp1

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 668, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 669, 0, "%s", "self->sentient");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 670, 0, "%s", "node");
    NodeDistMetric = Actor_Cover_GetNodeDistMetric(self, node);
    v5 = (float)((float)(Actor_Cover_ScoreOnEngagement(self, node) * ai_coverScore_engagement->current.value)
        + (float)NodeDistMetric);
    v6 = Actor_Cover_ScoreOnVisibility(self, node);
    if (((1 << node->constant.type) & 0x1C00) != 0)
        v7 = 0.0;
    else
        v7 = 1.0;
    v8 = (float)((float)(ai_coverScore_coverType->current.value * (float)v7)
        + (float)((float)((float)v6 * ai_coverScore_visibility->current.value) + (float)v5));
    v9 = (float)((float)(Actor_Cover_ScoreOnNodeAngle(self, node) * ai_coverScore_nodeAngle->current.value) + (float)v8);
    v10 = Actor_Cover_ScoreOnTargetDir(self, node);
    if (node->dynamic.inPlayerLOSTime >= level.time && self->sentient->eTeam == TEAM_ALLIES)
        v11 = 0.0;
    else
        v11 = 1.0;
    v12 = (float)((float)(ai_coverScore_playerLos->current.value * (float)v11)
        + (float)((float)((float)v10 * ai_coverScore_targetDir->current.value) + (float)v9));
    if ((node->constant.spawnflags & 0x40) != 0)
        v13 = 1.0;
    else
        v13 = 0.0;
    v14 = (float)((float)(ai_coverScore_priority->current.value * (float)v13) + (float)v12);
    return *((float *)&v14 + 1);
}

float __cdecl Actor_Cover_FromPoint_GetNodeMetric(actor_s *self, const pathnode_t *node)
{
    double NodeDistMetric; // fp31
    double v5; // fp1
    double v6; // fp0
    double v7; // fp31
    double v8; // fp1
    double v9; // fp0
    double v10; // fp1

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 692, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 693, 0, "%s", "self->sentient");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 694, 0, "%s", "node");
    NodeDistMetric = Actor_Cover_GetNodeDistMetric(self, node);
    v5 = Actor_Cover_ScoreOnVisibility(self, node);
    if (((1 << node->constant.type) & 0x1C00) != 0)
        v6 = 0.0;
    else
        v6 = 1.0;
    v7 = (float)((float)(ai_coverScore_coverType->current.value * (float)v6)
        + (float)((float)((float)v5 * ai_coverScore_visibility->current.value) + (float)NodeDistMetric));
    v8 = Actor_Cover_ScoreOnNodeAngle(self, node);
    if ((node->constant.spawnflags & 0x40) != 0)
        v9 = 1.0;
    else
        v9 = 0.0;
    v10 = (float)((float)(ai_coverScore_priority->current.value * (float)v9)
        + (float)((float)((float)v8 * ai_coverScore_nodeAngle->current.value) + (float)v7));
    return *((float *)&v10 + 1);
}

int __cdecl Actor_Cover_IsValidReacquire(actor_s *self, const pathnode_t *node)
{
    int result; // r3
    sentient_s *TargetSentient; // r3
    const pathnode_t *v6; // r4
    int v7; // r3
    unsigned __int8 v8; // r11

    result = Path_CanClaimNode(node, self->sentient);
    if (result)
    {
        TargetSentient = Actor_GetTargetSentient(self);
        v6 = Sentient_NearestNode(TargetSentient);
        if (!v6)
            return 1;
        v7 = Path_ExpandedNodeVisible(node, v6);
        v8 = 0;
        if (v7)
            return 1;
        return v8;
    }
    return result;
}

int __cdecl Actor_Cover_IsValidCoverDir(actor_s *self, const pathnode_t *node)
{
    gentity_s *TargetEntity; // r3
    int result; // r3
    double v5; // fp13

    TargetEntity = Actor_GetTargetEntity(self);
    if (!TargetEntity)
        return 1;
    v5 = (float)((float)(node->constant.forward[1] * (float)(TargetEntity->r.currentOrigin[1] - node->constant.vOrigin[1]))
        + (float)(node->constant.forward[0] * (float)(TargetEntity->r.currentOrigin[0] - node->constant.vOrigin[0])));
    result = 0;
    if (v5 >= 0.0)
        return 1;
    return result;
}

int __cdecl Actor_Cover_IsValidCover(actor_s *self, const pathnode_t *node)
{
    sentient_s *TargetSentient; // r3
    scr_animscript_t *v6[12]; // [sp+50h] [-30h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 761, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 762, 0, "%s", "self->sentient");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 763, 0, "%s", "node");
    if (!Path_CanClaimNode(node, self->sentient))
    {
        DebugDrawNodePicking("clm", self, node, (float *)colorRed);
        return 0;
    }
    TargetSentient = Actor_GetTargetSentient(self);
    if (TargetSentient)
    {
        Sentient_NearestNode(TargetSentient);
        if (!(unsigned __int8)Actor_Cover_IsValidCoverDir(self, node))
        {
            DebugDrawNodePicking("dir", self, node, (float *)colorRed);
            return 0;
        }
        if (Actor_Cover_PickAttackScript(self, node, 1, v6) && !v6[0])
            goto LABEL_14;
    }
    else if (Actor_Cover_GetAttackScript(self, node, v6) && !v6[0])
    {
    LABEL_14:
        DebugDrawNodePicking("scr", self, node, (float *)colorRed);
        return 0;
    }
    return 1;
}

float __cdecl Actor_Cover_MinHeightAtCover(pathnode_t *node)
{
    double v2; // fp1

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 821, 0, "%s", "node");
    if (((1 << node->constant.type) & 0x1020) != 0 || (node->constant.spawnflags & 8) != 0)
        v2 = 20.0;
    else
        v2 = 36.0;
    return *((float *)&v2 + 1);
}

pathnode_t *__cdecl Actor_Cover_FindCoverFromPoint(actor_s *self, const float *vPoint, double fMinSafeDist)
{
    int v6; // r5
    pathsort_t *v7; // r4
    int v8; // r3
    pathnode_t *v9; // r23
    double v10; // fp30
    pathnode_t **v11; // r27
    int v12; // r24
    pathnode_t *v13; // r31
    const float *vOrigin; // r30
    double v15; // fp1
    bool v16; // r8
    float *v17; // r7
    double NodeMetric; // fp1
    _BYTE v20[3104]; // [sp+60h] [-C70h] BYREF

    //Profile_Begin(356);
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 860, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 861, 0, "%s", "self->sentient");
    Actor_HasPath(self);
    v8 = Path_NodesInCylinder(self->ent->r.currentOrigin, 512.0, 80.0, v7, v6, (int)v20);
    v9 = 0;
    v10 = 0.0;
    if (v8 > 0)
    {
        v11 = (pathnode_t **)v20;
        v12 = v8;
        do
        {
            v13 = *v11;
            vOrigin = (*v11)->constant.vOrigin;
            if (Vec2DistanceSq(vOrigin, vPoint) > (double)(float)((float)fMinSafeDist * (float)fMinSafeDist)
                || (v15 = Actor_Cover_MinHeightAtCover(v13),
                    !G_CanRadiusDamageFromPos(self->ent, vOrigin, 0, vPoint, fMinSafeDist, 1.0, v17, v15, v16, 0)))
            {
                if ((unsigned __int8)Actor_Cover_IsValidCover(self, v13))
                {
                    NodeMetric = Actor_Cover_FromPoint_GetNodeMetric(self, v13);
                    if (NodeMetric > v10)
                    {
                        v10 = NodeMetric;
                        v9 = v13;
                    }
                }
            }
            --v12;
            v11 += 3;
        } while (v12);
    }
    //Profile_EndInternal(0);
    return v9;
}

int __cdecl isNodeInRegion(pathnode_t *node, gentity_s *volume)
{
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 908, 0, "%s", "node");
    if (!volume)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 909, 0, "%s", "volume");
    return SV_EntityContact(node->constant.vOrigin, node->constant.vOrigin, volume);
}

int __cdecl Actor_Cover_FindBestCoverListInList(actor_s *self, pathsort_t *nodes, int iNodeCount, gentity_s *volume)
{
    int v4; // r25
    int v7; // r18
    pathsort_t *v8; // r29
    pathsort_t *v9; // r22
    const pathnode_t *node; // r31
    bool v11; // r30
    const char *v12; // r3
    double NodeMetric; // [sp+18h] [-B8h]

    v4 = iNodeCount;
    v7 = 0;
    if (iNodeCount > 0)
    {
        v8 = nodes;
        v9 = &nodes[iNodeCount];
        do
        {
            node = v8->node;
            v11 = (unsigned __int8)Actor_Cover_IsValidCover(self, v8->node) != 0;
            if (v11 && volume)
            {
                if (!node)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 908, 0, "%s", "node");
                if (!SV_EntityContact(node->constant.vOrigin, node->constant.vOrigin, volume))
                {
                    DebugDrawNodePicking("vol", self, node, (float *)colorRed);
                    v11 = 0;
                }
            }
            if (v11)
            {
                v8->distMetric = Actor_Cover_GetNodeDistMetric(self, node);
                NodeMetric = Actor_Cover_GetNodeMetric(self, node);
                v8->metric = NodeMetric;
                v12 = va((const char *)HIDWORD(NodeMetric), LODWORD(NodeMetric));
                DebugDrawNodePicking(v12, self, node, (float *)colorWhite);
                ++v7;
                ++v8;
            }
            else
            {
                --v9;
                --v4;
                v8->node = v9->node;
                v8->metric = v9->metric;
                v8->distMetric = v9->distMetric;
            }
        } while (v7 < v4);
    }
    return v4;
}

int __cdecl compare_node_sort(float *pe1, float *pe2)
{
    double v2; // fp0

    v2 = (float)(pe1[1] - pe2[1]);
    if (v2 >= 0.0)
        return v2 > 0.0;
    else
        return -1;
}

int __cdecl Actor_Cover_FindBestCoverList(actor_s *self, pathnode_t **bestNodes, int bestNodesInList)
{
    int v6; // r5
    pathsort_t *v7; // r4
    int v8; // r3
    int BestCoverListInList; // r3
    signed int v10; // r31
    int v11; // r10
    float *p_metric; // r11
    unsigned int v13; // r9
    double v14; // fp13
    double v15; // fp12
    double v16; // fp11
    int v17; // r9
    float *v18; // r11
    signed int v19; // r29
    pathsort_t *v20; // r30
    signed int v21; // r11
    int v22; // r30
    int v23; // r10
    pathnode_t **v24; // r9
    pathsort_t *v25; // r11
    const pathnode_t *v26; // r29
    const char *v27; // r3
    pathsort_t v29[262]; // [sp+50h] [-C50h] BYREF

    //Profile_Begin(356);
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1003, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1004, 0, "%s", "self->sentient");
    v8 = Path_NodesInCylinder(self->codeGoal.pos, self->codeGoal.radius, self->codeGoal.height, v7, v6, (int)v29);
    BestCoverListInList = Actor_Cover_FindBestCoverListInList(self, v29, v8, self->codeGoal.volume);
    v10 = BestCoverListInList;
    v11 = 0;
    self->numCoverNodesInGoal = BestCoverListInList;
    if (BestCoverListInList >= 4)
    {
        p_metric = &v29[0].metric;
        v13 = ((unsigned int)(BestCoverListInList - 4) >> 2) + 1;
        v11 = 4 * v13;
        do
        {
            --v13;
            v14 = (float)(p_metric[3] - p_metric[4]);
            v15 = (float)(p_metric[6] - p_metric[7]);
            v16 = (float)(p_metric[9] - p_metric[10]);
            *p_metric = *p_metric - p_metric[1];
            p_metric[3] = v14;
            p_metric[6] = v15;
            p_metric[9] = v16;
            p_metric += 12;
        } while (v13);
    }
    if (v11 < BestCoverListInList)
    {
        v17 = BestCoverListInList - v11;
        v18 = &v29[v11].metric;
        do
        {
            --v17;
            *v18 = *v18 - v18[1];
            v18 += 3;
        } while (v17);
    }
    qsort(v29, BestCoverListInList, 0xCu, (int(__cdecl *)(const void *, const void *))compare_node_sort);
    v19 = 0;
    if (v10 > 0)
    {
        v20 = v29;
        do
        {
            if (v20->node != self->sentient->pClaimedNode || v19 == v10 - 1)
                v20->metric = v20->distMetric + v20->metric;
            else
                DebugDrawNodePicking("adv", self, v20->node, (float *)colorRed);
            ++v19;
            ++v20;
        } while (v19 < v10);
    }
    qsort(v29, v10, 0xCu, (int(__cdecl *)(const void *, const void *))compare_node_sort);
    v21 = 0;
    if (v10 > bestNodesInList)
        v21 = v10 - bestNodesInList;
    v22 = 0;
    if (v21 < v10)
    {
        v23 = v10 - v21;
        v24 = bestNodes;
        v22 = v10 - v21;
        v25 = &v29[v21];
        do
        {
            --v23;
            *v24 = v25->node;
            ++v25;
            ++v24;
        } while (v23);
        if (v22 > 0)
        {
            v26 = bestNodes[v22 - 1];
            v27 = va("best%d", v22);
            DebugDrawNodePicking(v27, self, v26, (float *)colorGreen);
        }
    }
    //Profile_EndInternal(0);
    return v22;
}

void __cdecl Actor_Cover_FindCoverNode(actor_s *self)
{
    if (Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED)
        self->iPotentialCoverNodeCount = Actor_Cover_FindBestCoverList(self, self->pPotentialCoverNode, 1000);
    else
        self->iPotentialCoverNodeCount = 0;
}

pathnode_t *__cdecl Actor_Cover_GetCoverNode(actor_s *self)
{
    int v2; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1090, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1091, 0, "%s", "self->sentient");
    if (!Actor_GetTargetEntity(self) || self->eState[self->stateLevel] != AIS_EXPOSED)
    {
    LABEL_11:
        self->iPotentialCoverNodeCount = 0;
        return 0;
    }
    if (self->iPotentialCoverNodeCount)
    {
        if (Actor_HasPath(self) || level.time >= self->iTeamMoveWaitTime)
        {
            v2 = self->iPotentialCoverNodeCount - 1;
            self->iPotentialCoverNodeCount = v2;
            return self->pPotentialCoverNode[v2];
        }
        goto LABEL_11;
    }
    return 0;
}

int __cdecl Actor_Cover_UseCoverNode(actor_s *self, pathnode_t *node)
{
    ai_state_t v4; // r11
    unsigned int v6; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1122, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1123, 0, "%s", "self->sentient");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1124, 0, "%s", "node");
    if (self->keepClaimedNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1125, 0, "%s", "!self->keepClaimedNode");
    v4 = self->eState[self->stateLevel];
    if (v4 != AIS_EXPOSED && v4 != AIS_TURRET || !(unsigned __int8)Actor_Cover_IsValidCover(self, node))
        return 0;
    if (!self->ent->tagInfo && !(unsigned __int8)Actor_FindPathToClaimNode(self, node))
    {
        Actor_TeamMoveBlocked(self);
        return 0;
    }
    Sentient_ClaimNode(self->sentient, node);
    v6 = 4 * (self->stateLevel + 3);
    self->iPotentialCoverNodeCount = 0;
    if (*(gentity_s **)((char *)&self->ent + v6) != (gentity_s *)1)
        Actor_SetState(self, AIS_EXPOSED);
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    return 1;
}

void __cdecl Actor_DebugDrawNodesInVolume(actor_s *self)
{
    int v2; // r5
    pathsort_t *v3; // r4
    gentity_s *volume; // r28
    int v5; // r3
    const pathnode_t **v6; // r27
    int v7; // r26
    const pathnode_t *v8; // r31
    float v9[4]; // [sp+50h] [-1860h] BYREF
    _BYTE v10[2128]; // [sp+60h] [-1850h] BYREF

    CL_GetViewPos(v9);
    volume = self->codeGoal.volume;
    if (!volume)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 1166, 0, "%s", "volume");
    v5 = Path_NodesInCylinder(self->codeGoal.pos, self->codeGoal.radius, self->codeGoal.height, v3, v2, (int)v10);
    if (v5 > 0)
    {
        v6 = (const pathnode_t **)v10;
        v7 = v5;
        do
        {
            v8 = *v6;
            if (!*v6)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 908, 0, "%s", "node");
            if (!volume)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover.cpp", 909, 0, "%s", "volume");
            if (SV_EntityContact(v8->constant.vOrigin, v8->constant.vOrigin, volume))
                Path_DrawDebugNode(v9, v8);
            --v7;
            v6 += 3;
        } while (v7);
    }
}

pathnode_t *__cdecl Actor_Cover_FindBestCover(actor_s *self)
{
    pathnode_t *result; // r3
    bool v2; // zf
    pathnode_t *v3; // [sp+50h] [-10h] BYREF

    v2 = Actor_Cover_FindBestCoverList(self, &v3, 1) != 0;
    result = v3;
    if (!v2)
        return 0;
    return result;
}

