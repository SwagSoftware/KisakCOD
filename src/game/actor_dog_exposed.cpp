#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_dog_exposed.h"
#include "g_main.h"
#include "actor_state.h"
#include <universal/com_math.h>
#include "game_public.h"
#include "g_local.h"
#include "actor_corpse.h"
#include "actor_orientation.h"
#include "actor_team_move.h"
#include "actor_exposed.h"

bool __cdecl Actor_Dog_Exposed_Start(actor_s *self, ai_state_t ePrevState)
{
    int time; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 18, 0, "%s", "self");
    self->ProneInfo.prone = 1;
    self->ProneInfo.orientPitch = 1;
    time = level.time;
    self->ProneInfo.iProneTrans = 500;
    self->ProneInfo.iProneTime = time;
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    return 1;
}

void __cdecl Actor_Dog_Exposed_Finish(actor_s *self, ai_state_t eNextState)
{
    self->ProneInfo.fTorsoPitch = 0.0;
    self->ProneInfo.prone = 0;
    self->ProneInfo.orientPitch = 0;
}

void __cdecl Actor_Dog_Exposed_Suspend(actor_s *self, ai_state_t eNextState)
{
    self->ProneInfo.fTorsoPitch = 0.0;
    self->ProneInfo.prone = 0;
    self->ProneInfo.orientPitch = 0;
}

int __cdecl Actor_Dog_IsInSyncedMelee(actor_s *self, sentient_s *enemy)
{
    EntHandle *p_syncedMeleeEnt; // r31
    int result; // r3
    bool v6; // zf

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 63, 0, "%s", "self");
    if (!enemy)
        return 0;
    p_syncedMeleeEnt = &enemy->syncedMeleeEnt;
    if (!p_syncedMeleeEnt->isDefined())
        return 0;
    v6 = p_syncedMeleeEnt->ent() == self->ent;
    result = 1;
    if (!v6)
        return 0;
    return result;
}

void __cdecl Actor_Dog_Attack(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 77, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 78, 0, "%s", "self->sentient");
    if (self->sentient->targetEnt.isDefined())
    {
        if ((AnimScriptList *)self->pAnimScriptFunc == &g_scr_data.dogAnim && !Actor_IsAnimScriptAlive(self))
            Actor_KillAnimScript(self);
        Actor_SetAnimScript(self, &g_animScriptTable[self->species]->combat, 0, AI_ANIM_MOVE_CODE);
    }
}

void __cdecl Actor_FindPathToGoalNearestNode(actor_s *self)
{
    pathnode_t *nodeTo; // r30
    pathnode_t *nodeFrom; // r5
    _BYTE v7[16]; // [sp+50h] [-330h] BYREF
    pathsort_t nodes[64]; // [sp+60h] [-320h] BYREF
    int iNodeCount;

    nodeTo = Path_NearestNode(self->codeGoal.pos, nodes, -2, self->codeGoal.radius, &iNodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if (nodeTo)
    {
        nodeFrom = Sentient_NearestNode(self->sentient);
        if (nodeFrom)
            Path_FindPathGetCloseAsPossible(
                &self->Path,
                self->sentient->eTeam,
                nodeFrom,
                self->ent->r.currentOrigin,
                nodeTo,
                nodeTo->constant.vOrigin,
                1);
    }
    else
    {
        Actor_FindPathToGoalDirect(self);
    }
}

// From blops with love
int __cdecl Actor_SetMeleeAttackSpot(actor_s *self, const float *enemyPosition, float *attackPosition)
{
    float dirFromEnemy[2]; // [sp+50h] [-130h] BYREF // v37-v38


    iassert(self);
    iassert(enemyPosition);

    float bestFraction = 0.0f;
    float currentValue;
    int currentIndex;
    int indices[4];
    float dotProducts[4];
    float mins[3];
    float maxs[3];

    float *v11;
    float meleeAttackDist;
    float endPos[3];

    trace_t trace;

    sentient_s *enemy = Actor_GetTargetSentient(self);
    dirFromEnemy[0] = self->ent->r.currentOrigin[0] - enemyPosition[0];
    dirFromEnemy[1] = self->ent->r.currentOrigin[1] - enemyPosition[1];
    Vec2Normalize(dirFromEnemy);

    int i;
    int j;
    for (i = 0; i < 4; ++i)
    {
        dotProducts[i] = (float)(dirFromEnemy[0] * (float)meleeAttackOffsets[i][0])
            + (float)(dirFromEnemy[1] * (float)meleeAttackOffsets[i][1]);
        indices[i] = i;
    }
    for (i = 1; i < 4; ++i)
    {
        currentValue = dotProducts[indices[i]];
        currentIndex = indices[i];
        for (j = i - 1; j >= 0 && dotProducts[indices[j]] <= currentValue; --j)
            indices[j + 1] = indices[j];
        indices[j + 1] = currentIndex;
    }
    for (i = 0; i < 4; ++i)
    {
        if (enemy->meleeAttackerSpot[i] == self->ent->s.number)
        {
            enemy->meleeAttackerSpot[i] = 0;
            break;
        }
    }
    
    attackPosition[2] = enemyPosition[2];
    mins[0] = actorMins[0];
    mins[1] = -15.0;
    mins[2] = 18.0f;
    maxs[0] = actorMaxs[0];
    maxs[1] = 15.0;
    maxs[2] = 48.0;

    float v7;
    float v8;
    float v9;
    float v10;
    float bestPosition[3];
    float dropPosition[3];

    //static const float DROP_AMOUNT = 90.0f; // 90 in blops
    static const float DROP_AMOUNT = 50.0f;

    for (i = 0; i < 4; ++i)
    {
        int currentOccupier = enemy->meleeAttackerSpot[indices[i]];

        iassert(currentOccupier < MAX_GENTITIES);

        v11 = (float *)meleeAttackOffsets[indices[i]];
        meleeAttackDist = self->meleeAttackDist;
        *attackPosition = (float)(meleeAttackDist * *v11) + *enemyPosition;
        attackPosition[1] = (float)(meleeAttackDist * v11[1]) + enemyPosition[1];
        if (currentOccupier <= 0)
            goto LABEL_33;

        iassert(g_entities[currentOccupier].r.inuse);

        v9 = g_entities[currentOccupier].r.currentOrigin[0] - *attackPosition;
        v10 = g_entities[currentOccupier].r.currentOrigin[1] - attackPosition[1];
        v7 = self->ent->r.currentOrigin[0] - *attackPosition;
        v8 = self->ent->r.currentOrigin[1] - attackPosition[1];

        if ((float)((float)(v7 * v7) + (float)(v8 * v8)) < (float)((float)(v9 * v9) + (float)(v10 * v10)))
        {
        LABEL_33:
            // blops mod (the flags arent the same, KISAKTODO: restore this part when flags are decoded)
            //G_TraceCapsule(&trace, attackPosition, mins, maxs, attackPosition, enemy->ent->s.number, 0x20000);
            //if (!trace.startsolid && !trace.allsolid)
            //    G_TraceCapsule(&trace, attackPosition, mins, maxs, enemyPosition, enemy->ent->s.number, 0x2820011);
            G_TraceCapsule(&trace, attackPosition, mins, maxs, enemyPosition, enemy->ent->s.number, 0x2820011);
            if (trace.fraction < 1.0 && ai_debugMeleeAttackSpots->current.enabled)
            {
                Vec3Lerp(attackPosition, enemyPosition, trace.fraction, endPos);
                G_DebugStar(endPos, colorYellow);
            }
            if (trace.fraction != 1.0 || trace.startsolid || trace.allsolid)
            {
                if (trace.fraction > bestFraction)
                {
                    bestFraction = trace.fraction;
                    bestPosition[0] = *attackPosition;
                    bestPosition[1] = attackPosition[1];
                    bestPosition[2] = attackPosition[2];
                }
                if (ai_debugMeleeAttackSpots->current.enabled)
                    G_DebugLine(attackPosition, enemyPosition, colorRed, 0);
            }
            else
            {
                dropPosition[0] = *attackPosition;
                dropPosition[1] = attackPosition[1];
                dropPosition[2] = attackPosition[2];
                dropPosition[2] = dropPosition[2] - DROP_AMOUNT;
                G_TraceCapsule(&trace, attackPosition, mins, actorMaxs, dropPosition, enemy->ent->s.number, 0x2820011);
                if (trace.fraction < 1.0 && !trace.allsolid && !trace.startsolid)
                {
                    attackPosition[2] = (float)(attackPosition[2] - (float)(DROP_AMOUNT * trace.fraction)) + 1.0;
                    enemy->meleeAttackerSpot[indices[i]] = self->ent->s.number;
                    break;
                }
                if (ai_debugMeleeAttackSpots->current.enabled)
                    G_DebugLine(attackPosition, dropPosition, colorRed, 0);
            }
        }
    }
    if (i == 4)
    {
        if (bestFraction <= 0.94999999)
            return 0;
        *attackPosition = bestPosition[0];
        attackPosition[1] = bestPosition[1];
        attackPosition[2] = bestPosition[2];
    }
    if (ai_debugMeleeAttackSpots->current.enabled && enemy)
    {
        G_DebugCircle(attackPosition, 15.0, colorYellow, 0, 1, 0);
        G_AddDebugString(attackPosition, colorYellow, 0.69999999, va("%i", self->ent->s.number));
        G_DebugLine(attackPosition, enemyPosition, colorGreen, 0);
    }
    return 1;
}

void __cdecl Actor_UpdateMeleeGoalPos(actor_s *self, float *goalPos)
{
    double pathEnemyFightDist; // fp1

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 262, 0, "%s", "self");
    if (!goalPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 263, 0, "%s", "goalPos");
    self->codeGoal.pos[0] = goalPos[0];
    self->codeGoal.pos[1] = goalPos[1];
    self->codeGoal.pos[2] = goalPos[2];
    self->codeGoalSrc = AI_GOAL_SRC_ENEMY;
    pathEnemyFightDist = self->pathEnemyFightDist;
    self->codeGoal.node = 0;
    self->codeGoal.volume = 0;
    Actor_SetGoalRadius(&self->codeGoal, pathEnemyFightDist);
}

int __cdecl Actor_Dog_IsAttackScriptRunning(actor_s *self)
{
    unsigned __int8 v2; // r11

    if ((AnimScriptList *)self->pAnimScriptFunc != &g_scr_data.dogAnim)
        return 0;
    if (!Actor_IsAnimScriptAlive(self))
        return 0;
    v2 = 1;
    if (self->safeToChangeScript)
        return 0;
    return v2;
}

float __cdecl Actor_Dog_GetEnemyPos(actor_s *self, sentient_s *enemy, float *enemyPos)
{
    gentity_s *ent; // r11
    float *p_commandTime; // r10
    double v8; // fp31
    double v9; // fp11
    double v10; // fp10
    int isDefined; // r3
    double v12; // fp1

    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 295, 0, "%s", "enemy");
    if (!enemyPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 296, 0, "%s", "enemyPos");
    ent = enemy->ent;
    *enemyPos = enemy->ent->r.currentOrigin[0];
    enemyPos[1] = ent->r.currentOrigin[1];
    enemyPos[2] = ent->r.currentOrigin[2];
    p_commandTime = (float *)&ent->client->ps.commandTime;
    v8 = (float)(self->meleeAttackDist + (float)15.0);
    if (p_commandTime)
    {
        if ((float)((float)(p_commandTime[10] * p_commandTime[10]) + (float)(p_commandTime[11] * p_commandTime[11])) > 1.0)
        {
            v8 = (float)((float)(self->meleeAttackDist + (float)15.0) + (float)15.0);
            v9 = enemyPos[1];
            v10 = enemyPos[2];
            *enemyPos = (float)(p_commandTime[10] * (float)0.25) + *enemyPos;
            enemyPos[1] = (float)(p_commandTime[11] * (float)0.25) + (float)v9;
            enemyPos[2] = (float)(p_commandTime[12] * (float)0.25) + (float)v10;
        }
        isDefined = ent->sentient->syncedMeleeEnt.isDefined();
        v12 = v8;
        if (isDefined)
            enemyPos[2] = enemyPos[2] + (float)64.0;
    }
    else
    {
        v12 = (float)(self->meleeAttackDist + (float)15.0);
    }
    return *((float *)&v12 + 1);
}

bool __cdecl Actor_Dog_IsEnemyInAttackRange(actor_s *self, sentient_s *enemy, int *goalPosSet)
{
    bool enemyInAttackRange; // r29
    float bufferedAttackDist; // fp31
    float *currentOrigin; // r3
    float mins[4]; // [sp+80h] [-80h] BYREF
    trace_t trace; // [sp+90h] [-70h] BYREF

    float enemyPos[3]; // v17
    float attackPos[3]; // v19
    float enemyToAttackSpot[2]; // v13, v14
    float enemyToMe[2]; // v15, v16

    iassert(self);
    iassert(enemy);
    iassert(goalPosSet);

    enemyInAttackRange = 0;
    bufferedAttackDist = Actor_Dog_GetEnemyPos(self, enemy, enemyPos);
    *goalPosSet = 0;
    if (!Actor_PointAtGoal(enemyPos, &self->codeGoal))
        return enemyInAttackRange;
    currentOrigin = self->ent->r.currentOrigin;
    self->useEnemyGoal = 1;
    self->useMeleeAttackSpot = 1;
    enemyInAttackRange = Actor_PointNearPoint(currentOrigin, enemyPos, bufferedAttackDist);

    if (Actor_SetMeleeAttackSpot(self, enemyPos, attackPos))
    {
        if (enemyInAttackRange)
        {
            enemyToAttackSpot[0] = attackPos[0] - enemyPos[0];
            enemyToAttackSpot[1] = attackPos[1] - enemyPos[1];
            Vec2Normalize(enemyToAttackSpot);

            enemyToMe[0] = self->ent->r.currentOrigin[0] - enemyPos[0];
            enemyToMe[1] = self->ent->r.currentOrigin[1] - enemyPos[1];
            Vec2Normalize(enemyToMe);
            //enemyInAttackRange = ((enemyToAttackSpot[0] * enemyToMe[0]) + (enemyToAttackSpot[1] * enemyToMe[1])) > 0.707f;
            enemyInAttackRange = ((enemyToAttackSpot[0] * enemyToMe[0]) + (enemyToAttackSpot[1] * enemyToMe[1])) > cosf(DEG2RAD(45.0f));
        }
        Actor_UpdateMeleeGoalPos(self, attackPos);
        *goalPosSet = 1;
        return enemyInAttackRange;
    }

    if (!enemyInAttackRange)
        return enemyInAttackRange;

    mins[0] = actorMins[0];
    mins[1] = actorMins[1];
    mins[2] = 18.0f;
    G_TraceCapsule(&trace, self->ent->r.currentOrigin, mins, actorMaxs, enemyPos, enemy->ent->s.number, 42074129);
    if (trace.fraction >= 1.0 && !trace.startsolid && !trace.allsolid)
        return enemyInAttackRange;
    return 0;
}

actor_think_result_t __cdecl Actor_Dog_Exposed_Think(actor_s *self)
{
    bool IsEnemyInAttackRange; // r28
    bool v3; // r29
    sentient_s *TargetSentient; // r30
    float *v5; // r6
    int v6; // r5
    unsigned __int8 v7; // r11
    int v8; // r26
    int *v9; // r6
    int v10; // r5
    int v11; // r4
    int *v12; // r6
    int v13; // r5
    int v14; // r4
    int flashBanged; // r11
    AISpecies species; // r10
    int v18; // [sp+50h] [-40h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 394, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 395, 0, "%s", "self->sentient");
    IsEnemyInAttackRange = 0;
    v3 = 1;
    v18 = 0;
    self->pszDebugInfo = "exposed";
    Actor_PreThink(self);
    TargetSentient = Actor_GetTargetSentient(self);
    if ((AnimScriptList *)self->pAnimScriptFunc != &g_scr_data.dogAnim
        || !Actor_IsAnimScriptAlive(self)
        || (v7 = 1, self->safeToChangeScript))
    {
        v7 = 0;
    }
    v8 = v7;
    if (!v7)
    {
        Actor_OrientPitchToGround(self->ent, 1);
        if (!TargetSentient)
        {
        LABEL_14:
            self->useEnemyGoal = 0;
            self->useMeleeAttackSpot = 0;
            goto LABEL_15;
        }
        IsEnemyInAttackRange = Actor_Dog_IsEnemyInAttackRange(self, TargetSentient, &v18);
    }
    if (!TargetSentient || !(unsigned __int8)Actor_PointAtGoal(TargetSentient->ent->r.currentOrigin, &self->codeGoal))
        goto LABEL_14;
LABEL_15:
    if (!v18)
        Actor_UpdateGoalPos(self);
    if (Actor_HasPath(self) && Vec2DistanceSq(self->codeGoal.pos, self->Path.vFinalGoal) > 225.0)
        Actor_ClearPath(self);
    if (!TargetSentient || self->useEnemyGoal)
    {
        Actor_FindPathToGoalDirect(self);
    }
    else if (!Actor_HasPath(self))
    {
        Actor_FindPathToGoalNearestNode(self);
        v3 = 0;
    }
    if (!Actor_HasPath(self))
    {
        if (self->useMeleeAttackSpot)
        {
            Actor_UpdateGoalPos(self);
            Actor_FindPathToGoalDirect(self);
        }
        if (TargetSentient && !Actor_HasPath(self))
        {
            Actor_FindPathToGoalNearestNode(self);
            v3 = 0;
        }
    }
    flashBanged = self->flashBanged;
    self->noDodgeMove = v3;
    if (!flashBanged || Actor_Dog_IsInSyncedMelee(self, TargetSentient))
    {
        if (v8 || IsEnemyInAttackRange)
        {
            Actor_Dog_Attack(self);
            Actor_PostThink(self);
            return ACTOR_THINK_DONE;
        }
        else
        {
            if (Actor_HasPath(self))
            {
                Actor_SetOrientMode(self, AI_ORIENT_TO_MOTION);
                Actor_MoveAlongPathWithTeam(self, 1, 0, 0);
            }
            else
            {
                species = self->species;
                self->useMeleeAttackSpot = 0;
                Actor_AnimStop(self, &g_animScriptTable[species]->stop);
            }
            Actor_PostThink(self);
            return ACTOR_THINK_DONE;
        }
    }
    else
    {
        Actor_Exposed_FlashBanged(self);
        Actor_PostThink(self);
        return ACTOR_THINK_DONE;
    }
}

