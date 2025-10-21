#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_threat.h"
#include <qcommon/mem_track.h>
#include "g_main.h"
#include "actor_state.h"
#include "game_public.h"
#include "g_local.h"
#include "actor_pain.h"
#include "actor_aim.h"
#include "actor_turret.h"

const float AI_THREAT_DISTANCE_RATE = 0.0008f;

threat_bias_t g_threatBias;
char g_threatDebugStrings[10][64]{ 0 };
const char *g_threatDebugLabels[10] =
{
  "Total:",
  "Flashed",
  "Suppressed",
  "Bias",
  "BiasGroup",
  "Attacker",
  "CurBonus",
  "Awareness",
  "Dist",
  "Scariness"
};

int g_skipDebugString;

void __cdecl TRACK_actor_threat()
{
    track_static_alloc_internal(&g_threatBias, 1060, "g_threatBias", 5);
    track_static_alloc_internal(g_threatDebugStrings, 640, "g_threatDebugStrings", 5);
    track_static_alloc_internal(g_threatDebugLabels, 40, "g_threatDebugLabels", 5);
}

void __cdecl Actor_InitThreatBiasGroups()
{
    threat_bias_t *v0; // r11
    int v1; // ctr

    v0 = &g_threatBias;
    v1 = 8;
    do
    {
        *(unsigned int *)v0->groupName = 0;
        v0 = (threat_bias_t *)((char *)v0 + 4);
        --v1;
    } while (v1);
    memset(g_threatBias.threatTable, 0, sizeof(g_threatBias.threatTable));
    g_threatBias.threatGroupCount = 1;
}

void __cdecl Actor_ClearThreatBiasGroups()
{
    threat_bias_t *v0; // r31

    v0 = &g_threatBias;
    do
    {
        if (v0->groupName[0])
            Scr_SetString(v0->groupName, 0);
        v0 = (threat_bias_t *)((char *)v0 + 2);
    } while ((int)v0 < (int)g_threatBias.threatTable);
    memset(&g_threatBias, 0, sizeof(g_threatBias));
}

int __cdecl Actor_FindThreatBiasGroupIndex(unsigned int name)
{
    int result; // r3
    threat_bias_t *v3; // r11

    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 163, 0, "%s", "name");
    result = 0;
    v3 = &g_threatBias;
    if (g_threatBias.threatGroupCount <= 0)
        return -1;
    while (v3->groupName[0] != name)
    {
        ++result;
        v3 = (threat_bias_t *)((char *)v3 + 2);
        if (result >= g_threatBias.threatGroupCount)
            return -1;
    }
    return result;
}

void __cdecl Actor_CreateThreatBiasGroup(unsigned int name)
{
    const char *v2; // r3

    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 183, 0, "%s", "name");
    if (g_threatBias.threatGroupCount < 16)
    {
        if (Actor_FindThreatBiasGroupIndex(name) < 0)
        {
            Scr_SetString(&g_threatBias.groupName[g_threatBias.threatGroupCount], name);
            ++g_threatBias.threatGroupCount;
        }
    }
    else
    {
        v2 = SL_ConvertToString(name);
        Com_PrintWarning(18, "Too many threat groups, can't create '%s'\n", v2);
    }
}

void __cdecl Actor_SetThreatBiasEntireGroup(int group, int threatBias)
{
    if (group < 0 || group >= g_threatBias.threatGroupCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            208,
            0,
            "%s",
            "group >= 0 && group < g_threatBias.threatGroupCount");
    memset(g_threatBias.threatTable[group], threatBias, sizeof(g_threatBias.threatTable[group]));
}

void __cdecl Actor_SetThreatBias(int groupSelf, int groupEnemy, int threatBias)
{
    int threatGroupCount; // r11

    if (groupSelf < 0 || (threatGroupCount = g_threatBias.threatGroupCount, groupSelf >= g_threatBias.threatGroupCount))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            223,
            0,
            "%s",
            "groupSelf >= 0 && groupSelf < g_threatBias.threatGroupCount");
        threatGroupCount = g_threatBias.threatGroupCount;
    }
    if (groupEnemy < 0 || groupEnemy >= threatGroupCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            224,
            0,
            "%s",
            "groupEnemy >= 0 && groupEnemy < g_threatBias.threatGroupCount");
    g_threatBias.threatTable[groupSelf][groupEnemy] = threatBias;
}

void __cdecl Actor_SetIgnoreMeGroup(int groupSelf, int groupIgnoreMe)
{
    int threatGroupCount; // r11

    if (groupSelf < 0 || (threatGroupCount = g_threatBias.threatGroupCount, groupSelf >= g_threatBias.threatGroupCount))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            239,
            0,
            "%s",
            "groupSelf >= 0 && groupSelf < g_threatBias.threatGroupCount");
        threatGroupCount = g_threatBias.threatGroupCount;
    }
    if (groupIgnoreMe < 0 || groupIgnoreMe >= threatGroupCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            240,
            0,
            "%s",
            "groupIgnoreMe >= 0 && groupIgnoreMe < g_threatBias.threatGroupCount");
    g_threatBias.threatTable[groupSelf][groupIgnoreMe] = 0x80000000;
}

int __cdecl Actor_GetThreatBias(int groupSelf, int groupEnemy)
{
    int threatGroupCount; // r11

    if (groupSelf < 0 || (threatGroupCount = g_threatBias.threatGroupCount, groupSelf >= g_threatBias.threatGroupCount))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            255,
            0,
            "%s",
            "groupSelf >= 0 && groupSelf < g_threatBias.threatGroupCount");
        threatGroupCount = g_threatBias.threatGroupCount;
    }
    if (groupEnemy < 0 || groupEnemy >= threatGroupCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp",
            256,
            0,
            "%s",
            "groupEnemy >= 0 && groupEnemy < g_threatBias.threatGroupCount");
    return g_threatBias.threatTable[groupSelf][groupEnemy];
}

void __cdecl Actor_FlagEnemyUnattackable(actor_s *self)
{
    sentient_s *TargetSentient; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 271, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 272, 0, "%s", "self->sentient");
    TargetSentient = Actor_GetTargetSentient(self);
    if (TargetSentient)
    {
        if (self->eState[self->stateLevel] == AIS_EXPOSED)
        {
            self->sentientInfo[TargetSentient - level.sentients].attackTime = 0x7FFFFFFF;
            Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        }
    }
}

int __cdecl Actor_CaresAboutInfo(actor_s *self, sentient_s *pOther)
{
    int lastKnownPosTime; // r11
    int result; // r3

    if (!pOther)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 298, 0, "%s", "pOther");
    lastKnownPosTime = self->sentientInfo[pOther - level.sentients].lastKnownPosTime;
    if (lastKnownPosTime <= 0)
        return 1;
    result = 0;
    if (level.time - lastKnownPosTime >= 2000)
        return 1;
    return result;
}

int __cdecl DebugThreatInfoDuration()
{
    return ai_threatUpdateInterval->current.integer / 50;
}

void __cdecl DebugResetThreatStrings(const actor_s *self)
{
    char *v2; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 320, 0, "%s", "self");
    if (ai_debugThreatSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number)
    {
        g_skipDebugString = 0;
        v2 = g_threatDebugStrings[0];
        do
        {
            *v2 = 0;
            v2 += 64;
        } while ((int)v2 < (int)&g_skipDebugString);
    }
    else
    {
        g_skipDebugString = 1;
    }
}

void __cdecl DebugSetThreatString(ThreatDebugStringCategory category, int threat)
{
    if (!g_skipDebugString)
    {
        if (threat)
            sprintf(g_threatDebugStrings[category], "%s %d", g_threatDebugLabels[category], threat);
        else
            g_threatDebugStrings[category][0] = 0;
    }
}

void __cdecl DebugSetThreatStringFromString(ThreatDebugStringCategory category, const char *string)
{
    if (!g_skipDebugString)
    {
        if (string)
            sprintf(g_threatDebugStrings[category], "%s %s", g_threatDebugLabels[category], string);
        else
            g_threatDebugStrings[category][0] = 0;
    }
}

void DebugThreatStringAll(const actor_s *self, sentient_s *enemy, int threat)
{
    iassert(self);

    if (!ai_debugThreatSelection->current.enabled || ai_debugEntIndex->current.integer != self->ent->s.number)
    {
        return;
    }

    iassert(enemy);

    // Compute debug line color based on threat value
    //float normalized = (float)threat * 0.00014285714f;  // ~1 / 7000
    float normalized = (float)threat * (1.0f / 7000.0f);  // ~1 / 7000
    float colorValue = fminf(fmaxf(normalized, 0.0f), 1.0f);
    float color[4] = { (colorValue + 1.0f) * 0.5f, 0.0f, 0.0f, 1.0f };

    float start[3];
    float end[3];
    Sentient_GetDebugEyePosition(self->sentient, start);
    Sentient_GetDebugEyePosition(enemy, end);

    // Draw line between eye positions
    int duration = ai_threatUpdateInterval->current.integer / 50;
    G_DebugLineWithDuration(start, end, color, 0, duration);

    // Print debug threat strings
    float drawPos[3] = { end[0], end[1], end[2] + 32.0f };
    for (int i = 0; i < 5; ++i)
    {
        const char *str = g_threatDebugStrings[i];
        if (str[0])
        {
            G_AddDebugStringWithDuration(drawPos, color, 0.5f, str, 1); // KISAKTODO: check duration
        }
        drawPos[2] += 8.0f;
    }
}

void __cdecl DebugThreatStringSimple(const actor_s *self, gentity_s *enemy, const char *string, const float *color)
{
    float displayPos[3]; // [sp+50h] [-60h] BYREF
    float start[3]; // [sp+60h] [-50h] BYREF

    iassert(self);

    if (ai_debugThreatSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number)
    {
        iassert(enemy);
        iassert(string);

        Sentient_GetDebugEyePosition(self->ent->sentient, start);

        if (enemy->sentient)
        {
            Sentient_GetDebugEyePosition(enemy->sentient, displayPos);
        }
        else
        {
            displayPos[0] = enemy->r.currentOrigin[0];
            displayPos[1] = enemy->r.currentOrigin[1];
            displayPos[2] = enemy->r.currentOrigin[2];
        }
        G_DebugLineWithDuration(start, displayPos, color, 0, ai_threatUpdateInterval->current.integer / 50);
        displayPos[2] += 16.0f;
        G_AddDebugStringWithDuration(displayPos, color, 1.0, string, ai_threatUpdateInterval->current.integer / 50);
    }
}

void __cdecl DebugThreatNodes(
    const actor_s *self,
    sentient_s *enemy,
    pathnode_t *selfNode,
    pathnode_t *enemyNode,
    const float *color)
{
    float start[3]; // [sp+50h] [-60h] BYREF
    float end[3]; // [sp+60h] [-50h] BYREF
    float viewpos[16]; // [sp+70h] [-40h] BYREF

    if (ai_debugThreatSelection->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number)
    {
        CL_GetViewPos(viewpos);
        if (selfNode)
        {
            Path_DrawDebugNode(viewpos, selfNode);
            start[0] = selfNode->constant.vOrigin[0];
            start[1] = selfNode->constant.vOrigin[1];
            start[2] = selfNode->constant.vOrigin[2];
        }
        else
        {
            Sentient_GetDebugEyePosition(self->ent->sentient, start);
        }
        if (enemyNode)
        {
            Path_DrawDebugNode(viewpos, enemyNode);
            end[0] = enemyNode->constant.vOrigin[0];
            end[1] = enemyNode->constant.vOrigin[1];
            end[2] = enemyNode->constant.vOrigin[2];
        }
        else
        {
            Sentient_GetDebugEyePosition(enemy->ent->sentient, end);
        }

        start[2] += 16.0f;
        end[2] += 16.0f;
        G_DebugLineWithDuration(start, end, color, 1, ai_threatUpdateInterval->current.integer / 50);
    }
}

int __cdecl Actor_ThreatFromScariness(double fScariness)
{
    int v1; // r11
    int v2; // r31

    v1 = (int)(float)((float)fScariness * (float)-100.0);
    if (v1 < 1000)
    {
        v2 = (int)(float)((float)fScariness * (float)-100.0);
        if (v1 <= -500)
            v2 = -500;
    }
    else
    {
        v2 = 1000;
    }
    if (!g_skipDebugString)
    {
        if (v2)
            sprintf(g_threatDebugStrings[9], "%s %d", g_threatDebugLabels[9], v2);
        else
            g_threatDebugStrings[9][0] = 0;
    }
    return v2;
}

// local variable allocation has failed, the output may be wrong!
int __cdecl Actor_ThreatFromDistance(double fDistance)
{
    long double v3; // fp2
    long double v4; // fp2
    int v5; // r31
    const char *v6; // r3

    if (fDistance < 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 486, 0, "%s", "fDistance >= 0");
    if (fDistance >= 2500.0)
        return 0;
    *(double *)&v3 = (float)((float)((float)(AI_THREAT_DISTANCE_RATE * (float)((float)2500.0 - (float)fDistance))
        * (float)((float)2500.0 - (float)fDistance))
        + (float)0.5);
    v4 = floor(v3);
    v5 = (int)(float)*(double *)&v4;
    v6 = va("%d (%0.1f)", v5, fDistance);
    if (!g_skipDebugString)
    {
        if (v6)
            sprintf(g_threatDebugStrings[8], "%s %s", g_threatDebugLabels[8], v6);
        else
            g_threatDebugStrings[8][0] = 0;
    }
    return v5;
}

bool __cdecl Actor_IsFullyAware(actor_s *self, sentient_s *enemy, int isCurrentEnemy)
{
    double v6; // fp0
    double v7; // fp13
    double v8; // fp12
    sentient_s *sentient; // r3
    pathnode_t *pClaimedNode; // r29
    const pathnode_t *v11; // r3
    pathnode_t *v12; // r28

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 512, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 513, 0, "%s", "enemy");
    if (isCurrentEnemy)
    {
        v6 = (float)(enemy->ent->r.currentOrigin[0] - self->sentientInfo[enemy - level.sentients].vLastKnownPos[0]);
        v7 = (float)(enemy->ent->r.currentOrigin[2] - self->sentientInfo[enemy - level.sentients].vLastKnownPos[2]);
        v8 = (float)(enemy->ent->r.currentOrigin[1] - self->sentientInfo[enemy - level.sentients].vLastKnownPos[1]);
        if ((float)((float)((float)v8 * (float)v8)
            + (float)((float)((float)v6 * (float)v6) + (float)((float)v7 * (float)v7))) < 4096.0)
        {
            sentient = self->sentient;
            pClaimedNode = sentient->pClaimedNode;
            if (!pClaimedNode)
                pClaimedNode = Sentient_NearestNode(sentient);
            v11 = Sentient_NearestNode(enemy);
            v12 = (pathnode_t *)v11;
            if (pClaimedNode)
            {
                if (v11 && Path_NodesVisible(pClaimedNode, v11))
                {
                    DebugThreatNodes(self, enemy, pClaimedNode, v12, colorBlue);
                    return 1;
                }
            }
        }
        return 0;
    }
    else
    {
        return level.time - self->sentientInfo[enemy - level.sentients].lastKnownPosTime < 10000;
    }
}

int __cdecl Actor_ThreatFromVisibilityAndAwareness(int isVisible, int isFullyAware, int friendlyTimingOut)
{
    int v3; // r31
    char *v4; // r3

    v3 = 0;
    if (isVisible)
    {
        v3 = 1000;
        v4 = va("%d (visible)", 1000);
        if (!g_skipDebugString)
        {
            if (v4)
            {
                sprintf(g_threatDebugStrings[7], "%s %s", g_threatDebugLabels[7], v4);
                return v3;
            }
        LABEL_13:
            g_threatDebugStrings[7][0] = 0;
        }
    }
    else
    {
        if (isFullyAware)
        {
            v3 = 500;
        }
        else if (friendlyTimingOut)
        {
            v3 = 250;
        }
        if (!g_skipDebugString)
        {
            if (v3)
            {
                sprintf(g_threatDebugStrings[7], "%s %d", g_threatDebugLabels[7], v3);
                return v3;
            }
            goto LABEL_13;
        }
    }
    return v3;
}

int __cdecl Actor_ThreatFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy)
{
    int attackerCount; // r11
    int v7; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 584, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 585, 0, "%s", "enemy");
    attackerCount = enemy->attackerCount;
    if (isCurrentEnemy)
        --attackerCount;
    v7 = -150 * attackerCount;
    if (-150 * attackerCount < -1000)
        v7 = -1000;
    if (enemy->syncedMeleeEnt.isDefined())
    {
        if (self->ent != enemy->syncedMeleeEnt.ent())
            v7 -= 10000;
    }
    if (!g_skipDebugString)
    {
        if (v7)
        {
            sprintf(g_threatDebugStrings[5], "%s %d", g_threatDebugLabels[5], v7);
            return v7;
        }
        g_threatDebugStrings[5][0] = 0;
    }
    return v7;
}

int __cdecl Actor_ThreatBonusForCurrentEnemy(
    int isCurrentEnemy,
    int isFullyAware,
    int friendlyTimingOut,
    int isPlayer,
    int isDamaged)
{
    int v5; // r31

    v5 = 0;
    if (isCurrentEnemy)
    {
        if (isFullyAware)
        {
            if (isPlayer && isDamaged)
                v5 = 1000;
            else
                v5 = 500;
        }
        else
        {
            v5 = friendlyTimingOut == 0 ? 100 : 200;
        }
    }
    if (!g_skipDebugString)
    {
        if (v5)
            sprintf(g_threatDebugStrings[6], "%s %d", g_threatDebugLabels[6], v5);
        else
            g_threatDebugStrings[6][0] = 0;
    }
    return v5;
}

int __cdecl Actor_ThreatCoveringFire(actor_s *self, sentient_s *enemy)
{
    actor_s *actor; // r30
    pathnode_t *pClaimedNode; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 633, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 634, 0, "%s", "enemy");
    if (!self->provideCoveringFire || (unsigned __int8)Actor_IsMoving(self))
        return 0;
    actor = enemy->ent->actor;
    if (actor && !actor->ignoreSuppression && !Actor_InPain(enemy->ent->actor) && !Actor_IsDying(actor))
    {
        if (actor->suppressionMeter <= 0.25)
            return 0;
        pClaimedNode = enemy->pClaimedNode;
        if (pClaimedNode)
        {
            if (Actor_IsNearClaimedNode(actor)
                && (float)((float)(pClaimedNode->constant.forward[1]
                    * (float)(self->ent->r.currentOrigin[1] - pClaimedNode->constant.vOrigin[1]))
                    + (float)(pClaimedNode->constant.forward[0]
                        * (float)(self->ent->r.currentOrigin[0] - pClaimedNode->constant.vOrigin[0]))) < 0.0)
            {
                return 0;
            }
        }
        if (!Actor_IsSuppressed(actor) && (unsigned __int8)Actor_IsMoving(actor))
            return 0;
    }
    if (!g_skipDebugString)
        sprintf(g_threatDebugStrings[2], "%s %d", g_threatDebugLabels[2], -3000);
    return -3000;
}

int __cdecl Actor_ThreatFlashed(sentient_s *enemy)
{
    actor_s *actor; // r11
    int flashBanged; // r11

    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 681, 0, "%s", "enemy");
    actor = enemy->ent->actor;
    if (actor)
    {
        flashBanged = actor->flashBanged;
    }
    else
    {
        if (!enemy->ent->client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 693, 0, "%s", "enemy->ent->client");
        flashBanged = G_ClientFlashbanged(enemy->ent->client);
    }
    if (!flashBanged)
        return 0;
    if (!g_skipDebugString)
        sprintf(g_threatDebugStrings[1], "%s %d", g_threatDebugLabels[1], 200);
    return 200;
}

// aislop
int Actor_UpdateSingleThreat(actor_s *self, sentient_s *enemy)
{
    iassert(self);
    iassert(enemy);

    char *info = (char *)self + 40 * (enemy - level.sentients);
    float *enemyData = (float *)(info + 2100);

    // Pacifist check
    if (self->bPacifist)
    {
        int lastThreat = ((unsigned int *)info)[528];
        if (!lastThreat || level.time - lastThreat >= self->iPacifistWait)
        {
            DebugThreatStringSimple(self, enemy->ent, "pacifist", colorRed);
            return INT_MIN;
        }
    }

    // Threat bias
    int bias = Actor_GetThreatBias(enemy->iThreatBiasGroupIndex, self->sentient->iThreatBiasGroupIndex);
    if (bias == INT_MIN)
    {
        return INT_MIN;
    }

    // Compute distance to enemy
    float dx = enemyData[6] - self->ent->r.currentOrigin[0];
    float dy = enemyData[7] - self->ent->r.currentOrigin[1];
    float dz = enemyData[8] - self->ent->r.currentOrigin[2];
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    sentient_s *currentTarget = Actor_GetTargetSentient(self);
    bool aware = false;
    int idxDiff = (((uintptr_t)currentTarget - (uintptr_t)enemy) >> 5) & 1;

    if (enemyData[0] != 0 ||
        Actor_IsFullyAware(self, enemy, idxDiff))
    {
        aware = true;
    }

    bool isDamaged =  enemy->ent->health < enemy->ent->maxHealth * 0.8f; // KISAKTODO: double check logic
    bool isPlayer = (enemy->ent->client != NULL);

    int proximityFlag = 0;
    if (!aware && idxDiff != 0 && level.time - ((unsigned int *)info)[4] < 10000)
    {
        proximityFlag = 1;
    }

    // Scariness difference
    float scarinessMe = Sentient_GetScarinessForDistance(self->sentient, enemy, dist);
    float scarinessOther = Sentient_GetScarinessForDistance(enemy, self->sentient, dist);
    float scarinessDiff = aware || idxDiff || proximityFlag ? (scarinessMe - scarinessOther) : 0.0f;

    DebugResetThreatStrings(self);
    if (!g_skipDebugString)
    {
        if (bias)
        {
            sprintf(g_threatDebugStrings[4], "%s %d",
                g_threatDebugLabels[4], bias);
        }
        else
        {
            g_threatDebugStrings[4][0] = '\0';
        }

        if (enemy->iThreatBias)
        {
            sprintf(g_threatDebugStrings[3], "%s %d",
                g_threatDebugLabels[3], enemy->iThreatBias);
        }
        else
        {
            g_threatDebugStrings[3][0] = '\0';
        }
    }

    int threatValue = bias + enemy->iThreatBias;
    threatValue += Actor_ThreatFromVisibilityAndAwareness((int)enemyData[0], aware, proximityFlag);
    threatValue += Actor_ThreatFromScariness(scarinessDiff);
    threatValue += Actor_ThreatFromDistance(dist);
    threatValue += Actor_ThreatFromAttackerCount(self, enemy, idxDiff);
    threatValue += Actor_ThreatBonusForCurrentEnemy(idxDiff, aware, proximityFlag, isPlayer, isDamaged);

    if (dist > 256.0f)
    {
        threatValue += Actor_ThreatCoveringFire(self, enemy);
    }

    threatValue += Actor_ThreatFlashed(enemy);

    float scaledThreat = threatValue * 0.00014285714f; // scale by ~1/7000

    if (!g_skipDebugString)
    {
        char *threatStr = va("%d (%.3f)", threatValue, scaledThreat);
        if (threatStr)
        {
            sprintf(g_threatDebugStrings[0], "%s %s",
                g_threatDebugLabels[0], threatStr);
        }
        else
        {
            g_threatDebugStrings[0][0] = '\0';
        }
    }

    DebugThreatStringAll(self, enemy, threatValue);
    return threatValue;
}

void Actor_InitThreatUpdateInterval(actor_s *self)
{
    iassert(self);

    int interval = ai_threatUpdateInterval->current.integer;

    if (interval > 0)
    {
        int alignedTime = (level.time / interval) * interval;
        int randomOffset = G_irand(0, interval);
        self->threatUpdateTime = alignedTime + randomOffset;
    }
    else
    {
        self->threatUpdateTime = 0;
    }
}


// aislop
void Actor_IncrementThreatTime(actor_s *self)
{
    iassert(self);
    iassert(ai_threatUpdateInterval->current.integer);

    int interval = ai_threatUpdateInterval->current.integer;

    self->threatUpdateTime += interval;

    if (self->threatUpdateTime <= level.time)
    {
        // Wrap-around handling
        self->threatUpdateTime %= interval;

        int cyclesAhead = (level.time / interval) + 1;
        self->threatUpdateTime += cyclesAhead * interval;
    }

    iassert(level.time < self->threatUpdateTime);
}

void __cdecl Actor_CanAttackAll(actor_s *self)
{
    team_t v2; // r3
    int v3; // r29
    sentient_s *i; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 1012, 0, "%s", "self");
    v2 = Sentient_EnemyTeam(self->sentient->eTeam);
    if (v2)
    {
        v3 = 1 << v2;
        for (i = Sentient_FirstSentient(1 << v2); i; i = Sentient_NextSentient(i, v3))
            self->sentientInfo[i - level.sentients].attackTime = 0;
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl Actor_SetPotentialThreat(potential_threat_t *self, double yaw)
{
    double v3; // fp31
    long double v4; // fp2
    long double v5; // fp2
    long double v6; // fp2

    v3 = (float)((float)yaw * (float)0.017453292);
    self->isEnabled = 1;
    *(double *)&v4 = v3;
    v5 = cos(v4);
    self->direction[0] = *(double *)&v5;
    *(double *)&v5 = v3;
    v6 = sin(v5);
    self->direction[1] = *(double *)&v6;
}

void __cdecl Actor_ClearPotentialThreat(potential_threat_t *self)
{
    self->isEnabled = 0;
}

bool __cdecl Actor_GetPotentialThreat(potential_threat_t *self, float *potentialThreatDir)
{
    if (self->isEnabled)
    {
        potentialThreatDir[0] = self->direction[0];
        potentialThreatDir[1] = self->direction[1];
    }

    return self->isEnabled;
}

void __cdecl Actor_PotentialThreat_Debug(actor_s *self)
{
    double v3; // fp0
    float xyz[4]; // [sp+50h] [-30h] BYREF
    float v5[4]; // [sp+60h] [-20h] BYREF

    Sentient_GetDebugEyePosition(self->ent->sentient, xyz);
    if (self->potentialThreat.isEnabled)
    {
        v5[2] = xyz[2];
        v3 = (float)((float)(self->potentialThreat.direction[1] * (float)32.0) + xyz[1]);
        v5[0] = (float)(self->potentialThreat.direction[0] * (float)32.0) + xyz[0];
        v5[1] = v3;
        G_DebugLine(xyz, v5, colorRed, 0);
    }
    else
    {
        G_AddDebugString(xyz, colorWhite, 1.0, "No Threat");
    }
}

int __cdecl Actor_CheckIgnore(sentient_s *self, sentient_s *enemy)
{
    int result; // r3
    bool v5; // zf

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 114, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 115, 0, "%s", "enemy");
    if (enemy->bIgnoreMe)
        return 1;
    v5 = Actor_GetThreatBias(enemy->iThreatBiasGroupIndex, self->iThreatBiasGroupIndex) != 0x80000000;
    result = 0;
    if (!v5)
        return 1;
    return result;
}

void __cdecl Actor_UpdateThreat(actor_s *self)
{
    sentient_s *sentient; // r11
    const char *v3; // r29
    gentity_s *v4; // r3
    gentity_s *v5; // r3
    team_t v6; // r3
    int v7; // r20
    gentity_s *ent; // r24
    int v9; // r14
    bool IsUsingTurret; // r16
    bool v11; // r22
    sentient_s *i; // r29
    gentity_s **v13; // r30
    const char *v14; // r5
    gentity_s *v15; // r11
    bool v16; // r10
    gentity_s *v17; // r11
    char v18; // r11
    bool v19; // zf
    unsigned __int8 v20; // r11
    unsigned __int8 v21; // r27
    int v22; // r30
    int updated; // r3
    int v24; // r28
    __int64 v25; // r10
    sentient_s *v26; // r11
    double entityTargetThreat; // fp1
    double v28; // r4
    const char *v29; // r5
    const char *v30; // r29
    gentity_s *v31; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 855, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_threat.cpp", 856, 0, "%s", "self->sentient");
    if (ai_threatUpdateInterval->current.integer)
    {
        if (level.time < self->threatUpdateTime)
            return;
        Actor_IncrementThreatTime(self);
    }
    if (ai_showPotentialThreatDir->current.enabled)
        Actor_PotentialThreat_Debug(self);
    sentient = self->sentient;
    if (sentient->bIgnoreAll)
    {
        if (Actor_GetTargetEntity(self))
            Sentient_SetEnemy(self->sentient, 0, 1);
        return;
    }
    if (sentient->scriptTargetEnt.isDefined()  && self->sentient->entityTargetThreat == 1.0)
    {
        v3 = va((const char *)0x3FF00000, 0);
        v4 = self->sentient->scriptTargetEnt.ent();
        DebugThreatStringSimple(self, v4, v3, colorGreen);
        v5 = self->sentient->scriptTargetEnt.ent();
        Sentient_SetEnemy(self->sentient, v5, 1);
        return;
    }
    v6 = Sentient_EnemyTeam(self->sentient->eTeam);
    if (v6)
    {
        v7 = -2147483647;
        ent = 0;
        v9 = 1 << v6;
        IsUsingTurret = Actor_IsUsingTurret(self);
        self->hasThreateningEnemy = 0;
        v11 = IsUsingTurret;
        for (i = Sentient_FirstSentient(v9); i; i = Sentient_NextSentient(i, v9))
        {
            v13 = &self->ent + 10 * (i - level.sentients);
            if ((int)v13[529] > 0)
            {
                if ((i->ent->flags & 4) != 0 || Actor_CheckIgnore(self->sentient, i))
                    goto LABEL_50;
                v16 = 1;
                if (!IsUsingTurret)
                {
                    v15 = v13[527];
                    if (!v15 || level.time - (int)v15 >= 10000)
                        v16 = 0;
                }
                v17 = v13[528];
                if (!v17 || (v19 = level.time - (int)v17 < 10000, v18 = 1, !v19))
                    v18 = 0;
                if (!v16 && !v18 || (v20 = 1, (int)v13[530] > level.time))
                    v20 = 0;
                v21 = v20;
                v22 = v20;
                self->hasThreateningEnemy = v20;
                if (v20 || !v11)
                {
                    if (!self->pTurret || !i->turretInvulnerability)
                    {
                        updated = Actor_UpdateSingleThreat(self, i);
                        v24 = updated;
                        if (updated != 0x80000000)
                        {
                            if (v7 < updated
                                || !v11 && v22
                                || self->pFavoriteEnemy.isDefined()
                                && i == self->pFavoriteEnemy.sentient())
                            {
                                if (self->pFavoriteEnemy.isDefined()
                                    && i == self->pFavoriteEnemy.sentient())
                                {
                                    ent = i->ent;
                                    v11 = v21;
                                    v7 = 2147483646;
                                }
                                else
                                {
                                    ent = i->ent;
                                    v7 = v24;
                                    v11 = v21;
                                }
                            }
                            continue;
                        }
                    LABEL_50:
                        v14 = "ignoreme";
                        goto LABEL_51;
                    }
                    v14 = "turret invul";
                }
                else
                {
                    v14 = "goodOnly";
                }
            }
            else
            {
                v14 = "unaware";
            }
        LABEL_51:
            DebugThreatStringSimple(self, i->ent, v14, colorRed);
        }
        if (self->sentient->scriptTargetEnt.isDefined())
        {
            LODWORD(v25) = v7;
            v26 = self->sentient;
            entityTargetThreat = v26->entityTargetThreat;
            if (entityTargetThreat > (float)((float)v25 * (float)0.00014285714))
            {
                ent = v26->scriptTargetEnt.ent();
                v28 = self->sentient->entityTargetThreat;
                v29 = va((const char *)HIDWORD(v28), LODWORD(v28));
                goto LABEL_59;
            }
            v30 = va("enemy (%0.3f)", entityTargetThreat);
            v31 = self->sentient->scriptTargetEnt.ent();
            DebugThreatStringSimple(self, v31, v30, colorYellow);
        }
        if (!ent)
        {
        LABEL_60:
            Sentient_SetEnemy(self->sentient, ent, 1);
            return;
        }
        v29 = "enemy";
    LABEL_59:
        DebugThreatStringSimple(self, ent, v29, colorGreen);
        goto LABEL_60;
    }
}

