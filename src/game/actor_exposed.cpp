#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_exposed.h"
#include "actor_state.h"
#include "g_main.h"
#include "actor_cover.h"
#include "actor_orientation.h"
#include "actor_senses.h"
#include "g_local.h"
#include "actor_team_move.h"
#include "actor_events.h"

void __cdecl Actor_Exposed_CheckLockGoal(actor_s *self)
{
    ai_animmode_t eAnimMode; // r11
    double v3; // fp31

    if (!Actor_IsAtGoal(self))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 35, 0, "%s", "Actor_IsAtGoal( self )");
    if (!self->useEnemyGoal)
    {
        eAnimMode = self->eAnimMode;
        if (eAnimMode != AI_ANIM_MOVE_CODE
            && eAnimMode != AI_ANIM_USE_ANGLE_DELTAS
            && level.time >= self->mayMoveTime + 1000
            && !(unsigned __int8)Actor_KeepClaimedNode(self))
        {
            v3 = (float)(self->codeGoal.radius - (float)30.0);
            if (v3 <= 0.0
                || Vec2DistanceSq(self->ent->r.currentOrigin, self->codeGoal.pos) >= (double)(float)((float)v3 * (float)v3))
            {
                self->moveMode = 0;
                self->eAnimMode = AI_ANIM_MOVE_CODE;
            }
        }
    }
}

void __cdecl Actor_Exposed_Combat(actor_s *self)
{
    sentient_s *TargetSentient; // r30
    int iLastVisTime; // r11
    sentient_s *sentient; // r11
    const pathnode_t *pClaimedNode; // r30
    scr_animscript_t *v6; // [sp+50h] [-30h] BYREF

    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());
    
    Actor_AnimCombat(self);
    TargetSentient = Actor_GetTargetSentient(self);
    if (Actor_IsAtGoal(self)
        && TargetSentient
        && ((Actor_Exposed_CheckLockGoal(self),
            (iLastVisTime = self->sentientInfo[TargetSentient - level.sentients].VisCache.iLastVisTime) == 0)
            || level.time - iLastVisTime >= 10000)
        && (sentient = self->sentient, (pClaimedNode = sentient->pClaimedNode) != 0)
        && Path_IsValidClaimNode(sentient->pClaimedNode)
        && (!Actor_Cover_PickAttackScript(self, pClaimedNode, 1, &v6) || v6))
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_GOAL);
    }
    else
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
    }
}

bool Actor_Exposed_Start(actor_s *self, ai_state_t ePrevState)
{
    iassert(self);
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    return true;
}

void __cdecl Actor_Exposed_Finish(actor_s *self, ai_state_t eNextState)
{
    ;
}

void __cdecl Actor_Exposed_Suspend(actor_s *self, ai_state_t eNextState)
{
    ;
}

bool __cdecl Actor_Exposed_Resume(actor_s *self, ai_state_t ePrevState)
{
    return true;
}

void __cdecl Actor_Exposed_DecideSubState(actor_s *self)
{
    ai_substate_t v2; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 173, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 174, 0, "%s", "self->ent");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 175, 0, "%s", "self->sentient");
    if (self->flashBanged)
    {
        Actor_SetSubState(self, STATE_EXPOSED_FLASHBANGED);
        return;
    }
    if (!Actor_GetTargetEntity(self))
    {
        Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        return;
    }
    v2 = self->eSubState[self->stateLevel];
    if (v2 == STATE_EXPOSED_NONCOMBAT)
    {
    LABEL_16:
        if (Actor_CanSeeEnemyExtended(self, 1))
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
        return;
    }
    if (v2 != STATE_EXPOSED_REACQUIRE_MOVE)
    {
        if (v2 != STATE_EXPOSED_FLASHBANGED)
            return;
        if (!self->flashBanged)
            Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        goto LABEL_16;
    }
    if (!Actor_HasPath(self) || self->pPileUpActor || self->pCloseEnt.isDefined())
        Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    if (Actor_CanSeeEnemy(self) && Actor_CanShootEnemy(self))
    {
        Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
        self->exposedStartTime = level.time;
    }
}

void __cdecl Actor_Exposed_FindReacquireNode(actor_s *self)
{
    if (Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED)
        self->iPotentialReacquireNodeCount = Actor_Cover_FindBestCoverList(self, self->pPotentialReacquireNode, 10);
    else
        self->iPotentialReacquireNodeCount = 0;
}

pathnode_t *__cdecl Actor_Exposed_GetReacquireNode(actor_s *self)
{
    int v2; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 257, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 258, 0, "%s", "self->sentient");
    if (!Actor_GetTargetEntity(self) || self->eState[self->stateLevel] != AIS_EXPOSED)
    {
    LABEL_11:
        self->iPotentialReacquireNodeCount = 0;
        return 0;
    }
    if (self->iPotentialReacquireNodeCount)
    {
        if (Actor_HasPath(self) || level.time >= self->iTeamMoveWaitTime)
        {
            v2 = self->iPotentialReacquireNodeCount - 1;
            self->iPotentialReacquireNodeCount = v2;
            return self->pPotentialReacquireNode[v2];
        }
        goto LABEL_11;
    }
    return 0;
}

int __cdecl Actor_Exposed_UseReacquireNode(actor_s *self, pathnode_t *pNode)
{
    int v4; // r6
    float v6[4]; // [sp+50h] [-50h] BYREF
    float v7[16]; // [sp+60h] [-40h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 291, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 292, 0, "%s", "self->sentient");
    if ((unsigned __int8)Actor_KeepClaimedNode(self))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp",
            293,
            0,
            "%s",
            "!Actor_KeepClaimedNode( self )");
    if (!Actor_GetTargetEntity(self))
        return 0;
    if (self->eState[self->stateLevel] != AIS_EXPOSED)
        return 0;
    if (!Path_CanClaimNode(pNode, self->sentient))
        return 0;
    v6[0] = pNode->constant.vOrigin[0];
    v6[2] = pNode->constant.vOrigin[2] + (float)64.0;
    v6[1] = pNode->constant.vOrigin[1];
    Actor_GetTargetLookPosition(self, v7);
    if (!Actor_CanSeePointFrom(self, v6, v7, 0.0, v4) || !Actor_FindPathToNode(self, pNode, 1))
        return 0;
    Sentient_ClaimNode(self->sentient, pNode);
    self->iPotentialReacquireNodeCount = 0;
    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    return 1;
}

static const float fSign[2] = { -1.0, 1.0 };

int __cdecl Actor_Exposed_ReacquireStepMove(actor_s *self, double fDist)
{
    gentity_s *targetEnt; // r23
    int iOrder; // r24
    bool prone; // r11
    float forward[3]; // [sp+50h] [-110h] BYREF // v19
    float vStepDir[3]; // [sp+60h] [-100h] BYREF // v22
    float vStartPos[3]; // [sp+80h] [-E0h] BYREF // v26
    float vEyePos[4]; // [sp+90h] [-D0h] BYREF
    float vEnemyPos[4]; // [sp+B0h] [-B0h] BYREF
    float vCheckPos[3];
    float vMovePos[3];
    float vTraceEndPos[3];

    iassert(self);
    iassert(self->sentient);
    targetEnt = Actor_GetTargetEntity(self);

    if (!targetEnt)
        return 0;

    if (self->eState[self->stateLevel] != AIS_EXPOSED)
        return 0;

    Actor_GetTargetLookPosition(self, vEnemyPos);
    YawVectors((self->fDesiredBodyYaw + 90.0f), vStepDir, 0);

    forward[0] = targetEnt->r.currentOrigin[0];
    forward[1] = targetEnt->r.currentOrigin[1];
    forward[2] = targetEnt->r.currentOrigin[2];

    forward[0] -= self->ent->r.currentOrigin[0];
    forward[1] -= self->ent->r.currentOrigin[1];
    forward[2] -= self->ent->r.currentOrigin[2];

    if (((forward[0] * forward[0]) + (forward[1] * forward[1])) <= 1.0f)
        return 0;

    Vec2Normalize(forward);

    forward[2] = 0.0;

    vStepDir[0] = forward[1];
    vStepDir[1] = -forward[0];
    vStepDir[2] = 0.0f;

    Actor_GetEyePosition(self, vEyePos);

    vStartPos[0] = self->ent->r.currentOrigin[0];
    vStartPos[1] = self->ent->r.currentOrigin[1];
    vStartPos[2] = self->ent->r.currentOrigin[2];

    iassert(!vStepDir[2]);

    iOrder = G_rand() & 1;

    for (int i = 0; i < 2; ++i)
    {
        float fScale = fSign[iOrder ^ i] * fDist;
        vCheckPos[0] = (float)(fScale * vStepDir[0]) + vEyePos[0];
        vCheckPos[1] = (float)(fScale * vStepDir[1]) + vEyePos[1];
        vCheckPos[2] = vEyePos[2];
        if (Actor_CanSeePointFrom(self, vCheckPos, vEnemyPos, self->fMaxSightDistSqrd, targetEnt->s.number))
        {
            vMovePos[0] = (float)(fScale * vStepDir[0]) + vStartPos[0];
            vMovePos[1] = (float)(fScale * vStepDir[1]) + vStartPos[1];
            vMovePos[2] = vStartPos[2];
            float stepheight = self->Physics.prone ? 10.0f : 18.0f;
            if (Path_PredictionTrace(vStartPos, vMovePos, 1023, self->Physics.iTraceMask | 4, vTraceEndPos, stepheight, 1)
                && Actor_PointAtGoal(vTraceEndPos, &self->codeGoal))
            {
                Actor_FindPath(self, vTraceEndPos, 0, 0);
                if (Actor_HasPath(self))
                {
                    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
                    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
                    return 1;
                }
            }
        }
    }

    return 0;

    //v11 = 0;
    //while (1)
    //{
    //    v25[2] = vEyePos[2];
    //    v12 = self->ent;
    //    v13 = (fSign[v11 ^ iOrder] * fDist);
    //    v14 = ((v23 * (fSign[v11 ^ iOrder] * fDist)) + vEyePos[1]);
    //    v25[0] = (vStepDir * (fSign[v11 ^ iOrder] * fDist)) + vEyePos[0];
    //    v25[1] = v14;
    //    if (ai_debugCoverEntityNum->current.integer == v12->s.number)
    //    {
    //        v33[3] = 1.0;
    //        v33[2] = 1.0;
    //        v33[1] = 1.0;
    //        v33[0] = 1.0;
    //        CL_AddDebugLine(v25, vEnemyPos, v33, 0, 30, 1);
    //        CL_AddDebugLine(v25, targetEnt->r.currentOrigin, colorRed, 0, 30, 1);
    //        CL_AddDebugLine(v25, self->ent->r.currentOrigin, colorBlue, 0, 30, 1);
    //    }
    //    if (Actor_CanSeePointFrom(self, v25, vEnemyPos, self->fMaxSightDistSqrd, v10))
    //    {
    //        prone = self->Physics.prone;
    //        v30[2] = v28;
    //        v30[0] = (float)(vStepDir * (float)v13) + vStartPos;
    //        v30[1] = (float)(v23 * (float)v13) + v27;
    //        v17 = prone ? 10.0 : 18.0;
    //        if (Path_PredictionTrace(&vStartPos, v30, ENTITYNUM_NONE, self->Physics.iTraceMask | 4, v32, v17, v15))
    //        {
    //            if ((unsigned __int8)Actor_PointAtGoal(v32, &self->codeGoal))
    //            {
    //                Actor_FindPath(self, v32, 0, 0);
    //                if (Actor_HasPath(self))
    //                    break;
    //            }
    //        }
    //    }
    //    if (++v11 >= 2)
    //        return 0;
    //}
    //Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    //Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
    //return 1;
}

void __cdecl Actor_Exposed_FindReacquireDirectPath(actor_s *self, bool ignoreSuppression)
{
    sentient_s *TargetSentient; // r4
    float v5[16]; // [sp+50h] [-40h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 448, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 449, 0, "%s", "self->sentient");
    if (Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED)
    {
        Actor_GetTargetPosition(self, v5);
        TargetSentient = Actor_GetTargetSentient(self);
        if (TargetSentient)
            Actor_FindPathToSentient(self, TargetSentient, !ignoreSuppression);
        else
            Actor_FindPath(self, v5, 1, ignoreSuppression);
        if (!(unsigned __int8)Actor_PointAtGoal(v5, &self->codeGoal))
            Actor_ClipPathToGoal(self);
        Actor_BeginTrimPath(self);
    }
    else
    {
        Actor_ClearPath(self);
    }
}

void __cdecl Actor_Exposed_FindReacquireProximatePath(actor_s *self, char ignoreSuppression)
{
    double v5; // fp0
    double v6; // fp13
    double v7; // fp12
    double v8; // fp1
    float v9[16]; // [sp+50h] [-40h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 485, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 486, 0, "%s", "self->sentient");
    if (Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED)
    {
        Actor_GetTargetPosition(self, v9);
        v5 = (float)(v9[0] - self->ent->r.currentOrigin[0]);
        v6 = (float)(v9[2] - self->ent->r.currentOrigin[2]);
        v7 = (float)(v9[1] - self->ent->r.currentOrigin[1]);
        v8 = (float)((float)((float)((float)v7 * (float)v7)
            + (float)((float)((float)v5 * (float)v5) + (float)((float)v6 * (float)v6)))
            * (float)0.25);
        if (v8 > (float)(self->fMaxSightDistSqrd * (float)0.0625))
            v8 = (float)(self->fMaxSightDistSqrd * (float)0.0625);
        Actor_FindPathInGoalWithLOS(self, v9, v8, ignoreSuppression);
        Actor_BeginTrimPath(self);
    }
    else
    {
        Actor_ClearPath(self);
    }
}

int __cdecl Actor_Exposed_StartReacquireMove(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 521, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 522, 0, "%s", "self->sentient");
    self->TrimInfo.iIndex = 0;
    self->TrimInfo.iDelta = 0;
    if (!Actor_GetTargetEntity(self) || self->eState[self->stateLevel] != AIS_EXPOSED || !Actor_HasPath(self))
        return 0;
    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    return 1;
}

void __cdecl Actor_Exposed_Reacquire_Move(actor_s *self)
{
    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());

    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP);
    Actor_MoveAlongPathWithTeam(self, 1, 0, 1);
}

void __cdecl Actor_Exposed_MoveToGoal_Move(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 568, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 569, 0, "%s", "self->sentient");
    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
    Actor_MoveAlongPathWithTeam(self, 1, 1, 1);
}

bool __cdecl Actor_Exposed_IsShortMove(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 585, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 586, 0, "%s", "self->sentient");
    return Actor_HasPath(self)
        && level.time - self->Path.iPathTime < 2000
        && Path_DistanceGreaterThan(&self->Path, 60.0) == 0;
}

void __cdecl Actor_Exposed_NonCombat_Think(actor_s *self)
{
    pathnode_t *pClaimedNode; // r29
    gentity_s *ent; // r11
    double v4; // fp11
    double v5; // fp0
    double v6; // fp13
    double v7; // fp12
    ai_orient_mode_t integer; // r30
    scr_animscript_t *StopAnim; // r3
    scr_animscript_t *v10; // r3

    pClaimedNode = self->sentient->pClaimedNode;
    Actor_FindPathToGoal(self);
    if (Actor_HasPath(self))
    {
        ent = self->ent;
        v4 = (float)(self->fWalkDist * self->fWalkDist);
        self->pszDebugInfo = "noncombat_move";
        v5 = (float)(ent->r.currentOrigin[0] - self->Path.vFinalGoal[0]);
        v6 = (float)(ent->r.currentOrigin[2] - self->Path.vFinalGoal[2]);
        v7 = (float)(ent->r.currentOrigin[1] - self->Path.vFinalGoal[1]);
        Actor_MoveAlongPathWithTeam(
            self,
            (float)((float)((float)v7 * (float)v7) + (float)((float)((float)v6 * (float)v6) + (float)((float)v5 * (float)v5))) >= v4,
            1,
            1);
        if (ai_moveOrientMode->current.integer)
        {
            integer = (ai_orient_mode_t)ai_moveOrientMode->current.integer;
        }
        else if (!pClaimedNode
            || !Path_IsValidClaimNode(pClaimedNode)
            || !Actor_Exposed_IsShortMove(self)
            || (integer = AI_ORIENT_TO_GOAL, self->arrivalInfo.animscriptOverrideRunTo))
        {
            integer = AI_ORIENT_TO_MOTION;
        }
        if (!Actor_IsAtGoal(self) || pClaimedNode && Path_IsValidClaimNode(pClaimedNode))
            Actor_SetOrientMode(self, integer);
        else
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
    }
    else
    {
        self->pszDebugInfo = "noncombat_stop";
        if (Actor_IsAtGoal(self))
        {
            if (pClaimedNode && Path_IsValidClaimNode(pClaimedNode))
            {
                StopAnim = Actor_GetStopAnim(self);
                Actor_AnimStop(self, StopAnim);
                Actor_SetOrientMode(self, AI_ORIENT_TO_GOAL);
                return;
            }
            Actor_Exposed_CheckLockGoal(self);
        }
        if (Actor_GetTargetEntity(self))
        {
            Actor_AnimCombat(self);
        }
        else
        {
            v10 = Actor_GetStopAnim(self);
            Actor_AnimStop(self, v10);
        }
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
    }
}

void __cdecl Actor_Exposed_FlashBanged(actor_s *self)
{
    if (!self->flashBanged)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 684, 0, "%s", "self->flashBanged");
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->flashed, 0, AI_ANIM_USE_BOTH_DELTAS);
    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
}

// aislop
bool Actor_Exposed_CheckStopMovingAndStartCombat(actor_s *self)
{
    iassert(self);

    // If the actor is in a pile-up or has a close entity defined, stop moving and start combat.
    if (self->pPileUpActor || self->pCloseEnt.isDefined())
        return true;

    // If the actor can't point at the goal, don't stop or start combat.
    if (!Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal))
        return false;

    // If the actor has a claimed node and doesn't want to keep it, point at the node origin.
    if (self->sentient->pClaimedNode && !Actor_KeepClaimedNode(self))
        return Actor_PointAt(self->ent->r.currentOrigin, self->sentient->pClaimedNode->constant.vOrigin);

    // Compute distance to goal with a fallback to 64 unit offset.
    float nearDistance = (self->codeGoal.radius * 0.5f) - 64.0f;

    // Clamp the value: if negative, use 0 instead (fsel emulation).
    float finalDistance = (nearDistance >= 0.0f) ? nearDistance : 0.0f;

    // Check if we're near the goal within the calculated distance.
    return Actor_PointNearGoal(self->ent->r.currentOrigin, &self->codeGoal, -finalDistance);
}


actor_think_result_t __cdecl Actor_Exposed_Think(actor_s *self)
{
    actor_think_result_t result; // r3
    gentity_s *pTurret; // r11
    ai_substate_t v4; // r4
    const char *v5; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 736, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 737, 0, "%s", "self->sentient");
    self->pszDebugInfo = "exposed";
    if (self->pGrenade.isDefined() && !self->flashBanged)
    {
        Actor_SetState(self, AIS_GRENADE_RESPONSE);
        return ACTOR_THINK_REPEAT;
    }
    if (self->isInBadPlace && !self->flashBanged)
    {
        Actor_SetState(self, AIS_BADPLACE_FLEE);
        return ACTOR_THINK_REPEAT;
    }
    pTurret = self->pTurret;
    if (pTurret)
    {
        if (!pTurret->active)
        {
            Actor_SetState(self, AIS_TURRET);
            return ACTOR_THINK_REPEAT;
        }
        self->pTurret = 0;
    }
    Actor_PreThink(self);
    Actor_Exposed_DecideSubState(self);
    v4 = self->eSubState[self->stateLevel];
    switch (v4)
    {
    case STATE_EXPOSED_COMBAT:
        self->pszDebugInfo = "exposed_combat";
        Actor_FindPathToGoal(self);
        if (Actor_HasPath(self) && Actor_CanSeeEnemy(self) && Actor_Exposed_CheckStopMovingAndStartCombat(self))
            Actor_ClearPath(self);
        if (Actor_HasPath(self))
            Actor_Exposed_MoveToGoal_Move(self);
        else
            Actor_Exposed_Combat(self);
        goto LABEL_28;
    case STATE_EXPOSED_NONCOMBAT:
        Actor_Exposed_NonCombat_Think(self);
        Actor_PostThink(self);
        result = ACTOR_THINK_DONE;
        break;
    case STATE_EXPOSED_REACQUIRE_MOVE:
        self->pszDebugInfo = "exposed_reacquire_move";
        Actor_Exposed_Reacquire_Move(self);
        Actor_PostThink(self);
        result = ACTOR_THINK_DONE;
        break;
    case STATE_EXPOSED_FLASHBANGED:
        self->pszDebugInfo = "exposed_flashbanged";
        Actor_Exposed_FlashBanged(self);
        Actor_PostThink(self);
        result = ACTOR_THINK_DONE;
        break;
    default:
        if (!alwaysfails)
        {
            v5 = va("Actor_Exposed_Think: unhandled substate %i\n", v4);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 805, 0, v5);
        }
    LABEL_28:
        Actor_PostThink(self);
        result = ACTOR_THINK_DONE;
        break;
    }
    return result;
}

void __cdecl Actor_Exposed_Touch(actor_s *self, gentity_s *pOther)
{
    sentient_s *sentient; // r4

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 823, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 824, 0, "%s", "self->sentient");
    if (!pOther)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_exposed.cpp", 825, 0, "%s", "pOther");
    sentient = pOther->sentient;
    if (sentient)
        Actor_GetPerfectInfo(self, sentient);
}

