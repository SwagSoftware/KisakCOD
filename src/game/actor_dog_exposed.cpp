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

int __cdecl Actor_SetMeleeAttackSpot(actor_s *self, const float *enemyPosition, float *attackPosition)
{
    sentient_s *TargetSentient; // r3
    gentity_s *ent; // r11
    sentient_s *v8; // r16
    double v9; // fp13
    int v10; // r20
    unsigned int *v11; // r7
    int v12; // r11
    int v13; // r6
    int v14; // r5
    int v15; // r10
    double v16; // fp0
    unsigned int *v17; // r11
    int v18; // r8
    int v19; // r9
    int v20; // r8
    int v21; // r9
    unsigned int *v22; // r11
    int v23; // r10
    int *meleeAttackerSpot; // r11
    int *v25; // r22
    int v26; // r30
    int v27; // r31
    double meleeAttackDist; // fp0
    float *v29; // r11
    int v30; // r31
    double v31; // fp29
    const float *v32; // r4
    double v33; // fp0
    const float *v34; // r4
    const char *v36; // r5
    float v37; // [sp+50h] [-130h] BYREF
    float v38; // [sp+54h] [-12Ch]
    float v39[4]; // [sp+58h] [-128h] BYREF
    float v40[6]; // [sp+68h] [-118h] BYREF
    float v41[4]; // [sp+80h] [-100h] BYREF
    int v42; // [sp+90h] [-F0h] BYREF
    unsigned int v43[3]; // [sp+94h] [-ECh] BYREF
    trace_t v44; // [sp+A0h] [-E0h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 130, 0, "%s", "self");
    if (!enemyPosition)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 131, 0, "%s", "enemyPosition");
    TargetSentient = Actor_GetTargetSentient(self);
    ent = self->ent;
    v8 = TargetSentient;
    v9 = enemyPosition[1];
    v37 = self->ent->r.currentOrigin[0] - *enemyPosition;
    v38 = ent->r.currentOrigin[1] - (float)v9;
    Vec2Normalize(&v37);
    v10 = 0;
    v11 = v43;
    v42 = 0;
    v43[0] = 1;
    v43[1] = 2;
    v41[0] = (float)((float)1.0 * v37) + (float)((float)0.0 * v38);
    v41[1] = (float)((float)0.0 * v37) + (float)((float)1.0 * v38);
    v43[2] = 3;
    v41[2] = (float)((float)-1.0 * v37) + (float)((float)0.0 * v38);
    v41[3] = (float)((float)0.0 * v37) + (float)((float)-1.0 * v38);
    v12 = 0;
    do
    {
        v13 = *v11;
        v14 = v12 + 1;
        v15 = v12;
        v16 = v41[*v11];
        if (v12 + 1 < 4)
        {
        LABEL_13:
            if (v15 >= 0)
            {
                v22 = &v43[v15 - 1];
                do
                {
                    if (v16 < v41[*v22])
                        break;
                    --v15;
                    v22[1] = *v22;
                    --v22;
                } while (v15 >= 0);
            }
        }
        else
        {
            v17 = v11 - 3;
            while (1)
            {
                v18 = v17[2];
                if (v16 < v41[v18])
                    break;
                v19 = v17[1];
                v17[3] = v18;
                if (v16 < v41[v19])
                {
                    --v15;
                    break;
                }
                v20 = *v17;
                v17[2] = v19;
                if (v16 < v41[v20])
                {
                    v15 -= 2;
                    break;
                }
                v21 = *(v17 - 1);
                v17[1] = v20;
                if (v16 < v41[v21])
                {
                    v15 -= 3;
                    break;
                }
                v15 -= 4;
                *v17 = v21;
                v17 -= 4;
                if (v15 < 3)
                    goto LABEL_13;
            }
        }
        v12 = v14;
        ++v11;
        v43[v15] = v13;
    } while (v14 + 1 < 4);
    v23 = 0;
    meleeAttackerSpot = v8->meleeAttackerSpot;
    while (*meleeAttackerSpot != self->ent->s.number)
    {
        ++v23;
        ++meleeAttackerSpot;
        if (v23 >= 4)
            goto LABEL_27;
    }
    v8->meleeAttackerSpot[v23] = 0;
LABEL_27:
    attackPosition[2] = enemyPosition[2];
    v25 = &v42;
    v39[2] = 18.0;
    v39[0] = -15.0;
    v39[1] = -15.0;
    while (1)
    {
        v26 = *v25;
        v27 = v8->meleeAttackerSpot[*v25];
        if (v27 >= 2176)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp",
                194,
                0,
                "%s",
                "currentOccupier < MAX_GENTITIES");
        meleeAttackDist = self->meleeAttackDist;
        v29 = (float *)meleeAttackOffsets[v26];
        *attackPosition = (float)(*v29 * self->meleeAttackDist) + *enemyPosition;
        attackPosition[1] = (float)(v29[1] * (float)meleeAttackDist) + enemyPosition[1];
        if (v27 > 0)
        {
            v30 = v27;
            if (!g_entities[v30].r.inuse)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp",
                    200,
                    0,
                    "%s",
                    "g_entities[currentOccupier].r.inuse");
            v31 = Vec2DistanceSq(attackPosition, g_entities[v30].r.currentOrigin);
            if (v31 <= Vec2DistanceSq(attackPosition, self->ent->r.currentOrigin))
                goto LABEL_43;
        }
        G_TraceCapsule(&v44, attackPosition, v39, actorMaxs, enemyPosition, v8->ent->s.number, 42074129);
        if (v44.fraction == 1.0 && !v44.startsolid && !v44.allsolid)
            break;
        if (ai_debugMeleeAttackSpots->current.enabled)
        {
            v34 = enemyPosition;
            goto LABEL_42;
        }
    LABEL_43:
        ++v10;
        ++v25;
        if (v10 >= 4)
            goto LABEL_46;
    }
    v40[2] = attackPosition[2] - (float)50.0;
    v33 = attackPosition[1];
    v40[0] = *attackPosition;
    v40[1] = v33;
    G_TraceCapsule(&v44, attackPosition, v39, actorMaxs, v40, v8->ent->s.number, 42074129);
    if (v44.fraction >= 1.0)
    {
        if (ai_debugMeleeAttackSpots->current.enabled)
        {
            v34 = v40;
        LABEL_42:
            G_DebugLine(attackPosition, v34, colorRed, 0);
            goto LABEL_43;
        }
        goto LABEL_43;
    }
    v8->meleeAttackerSpot[v43[v10 - 1]] = self->ent->s.number;
LABEL_46:
    if (v10 == 4)
        return 0;
    if (ai_debugMeleeAttackSpots->current.enabled)
    {
        if (v8)
        {
            G_DebugCircle(attackPosition, 15.0, v32, (int)colorYellow, 0, 1);
            va("%i", self->ent->s.number);
            G_AddDebugString(attackPosition, colorYellow, 0.69999999, v36);
            G_DebugLine(attackPosition, enemyPosition, colorGreen, 0);
        }
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
    bool v6; // r29
    double EnemyPos; // fp31
    float *currentOrigin; // r3
    gentity_s *ent; // r11
    gentity_s *v11; // r8
    float *v12; // r4
    float v13; // [sp+50h] [-B0h] BYREF
    float v14; // [sp+54h] [-ACh]
    float v15; // [sp+58h] [-A8h] BYREF
    float v16; // [sp+5Ch] [-A4h]
    float v17; // [sp+60h] [-A0h] BYREF
    float v18; // [sp+64h] [-9Ch]
    float v19[4]; // [sp+70h] [-90h] BYREF
    float v20[4]; // [sp+80h] [-80h] BYREF
    trace_t v21; // [sp+90h] [-70h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 326, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 327, 0, "%s", "enemy");
    if (!goalPosSet)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_dog_exposed.cpp", 328, 0, "%s", "goalPosSet");
    v6 = 0;
    EnemyPos = Actor_Dog_GetEnemyPos(self, enemy, &v17);
    *goalPosSet = 0;
    if (!(unsigned __int8)Actor_PointAtGoal(&v17, &self->codeGoal))
        return v6;
    currentOrigin = self->ent->r.currentOrigin;
    self->useEnemyGoal = 1;
    self->useMeleeAttackSpot = 1;
    v6 = Actor_PointNearPoint(currentOrigin, &v17, EnemyPos);
    if ((unsigned __int8)Actor_SetMeleeAttackSpot(self, &v17, v19))
    {
        if (v6)
        {
            v13 = v19[0] - v17;
            v14 = v19[1] - v18;
            Vec2Normalize(&v13);
            ent = self->ent;
            v15 = self->ent->r.currentOrigin[0] - v17;
            v16 = ent->r.currentOrigin[1] - v18;
            Vec2Normalize(&v15);
            v6 = (float)((float)(v16 * v14) + (float)(v15 * v13)) > 0.70700002;
        }
        Actor_UpdateMeleeGoalPos(self, v19);
        *goalPosSet = 1;
        return v6;
    }
    if (!v6)
        return v6;
    v11 = enemy->ent;
    v12 = self->ent->r.currentOrigin;
    v20[0] = -15.0;
    v20[1] = -15.0;
    v20[2] = 18.0;
    G_TraceCapsule(&v21, v12, v20, actorMaxs, &v17, v11->s.number, 42074129);
    if (v21.fraction >= 1.0 && !v21.startsolid && !v21.allsolid)
        return v6;
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

