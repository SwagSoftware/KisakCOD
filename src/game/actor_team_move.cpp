#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_team_move.h"
#include "g_main.h"
#include <universal/com_math.h>
#include <server/server.h> // gentity_s 
#include "actor_cover_arrival.h"

bool __cdecl Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther)
{
    return fabsf((float)(vOrgSelf[2] - vOrgOther[2])) >= 80.0f;
}

void __cdecl Actor_TeamMoveBlocked(actor_s *self)
{
    self->iTeamMoveWaitTime = level.time + 500;
}

void __cdecl Actor_TeamMoveBlockedClear(actor_s *self)
{
    self->iTeamMoveWaitTime = 0;
}

int __cdecl Actor_TeamMoveCheckWaitTimer(actor_s *self, ai_teammove_t *result)
{
    int v4; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 69, 0, "%s", "self");
    if (!result)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 70, 0, "%s", "result");
    if (level.time >= self->iTeamMoveWaitTime)
    {
        if (self->pCloseEnt.isDefined())
        {
            v4 = 1;
            self->iTeamMoveWaitTime = level.time + 50;
            *result = AI_TEAMMOVE_WAIT;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        v4 = 1;
        *result = AI_TEAMMOVE_WAIT;
    }
    return v4;
}

bool __cdecl Actor_TeamMoveNeedToCheckWait(unsigned __int8 moveMode, path_t *pPath)
{
    int v2; // r11
    bool result; // r3
    bool v4; // zf

    v2 = pPath->flags & 2;
    if (!moveMode)
        return !v2 || pPath->fLookaheadDist < 60.0 && !Path_CompleteLookahead(pPath);
    if (v2)
        return 0;
    if (pPath->fLookaheadDist >= 60.0)
        return 0;
    v4 = Path_CompleteLookahead(pPath) == 0;
    result = 1;
    if (!v4)
        return 0;
    return result;
}

bool __cdecl Actor_IsEnemy(actor_s *self, sentient_s *other)
{
    return ((1 << other->eTeam) & ~(1 << Sentient_EnemyTeam(self->sentient->eTeam))) == 0;
}

void __cdecl Actor_CalcInterval(
    actor_s *self,
    bool bUseInterval,
    float *fIntervalSqrdOut,
    float *fWalkIntervalSqrdOut)
{
    double fInterval; // fp0
    double v5; // fp13

    if (bUseInterval)
    {
        fInterval = self->fInterval;
        v5 = (float)(self->fInterval * (float)0.5);
        if (v5 < 37.5)
        {
            v5 = 37.5;
            if (fInterval < 30.0)
                fInterval = 30.0;
        }
        *fIntervalSqrdOut = (float)fInterval * (float)fInterval;
        *fWalkIntervalSqrdOut = (float)v5 * (float)v5;
    }
    else
    {
        *fIntervalSqrdOut = 900.0;
        *fWalkIntervalSqrdOut = 1406.25;
    }
}

int __cdecl Actor_TeamMoveCalcMovementDir(team_move_context_t *context, ai_teammove_t *result)
{
    float *vVelDirSelf; // r30
    actor_s *self; // r29
    double v6; // fp1
    int v7; // r3
    double v8; // fp0

    vVelDirSelf = context->vVelDirSelf;
    self = context->self;
    *(double *)context->vVelSelf = *(double *)context->self->Physics.vVelocity;
    *(double *)context->vVelDirSelf = *(double *)context->vVelSelf;
    v6 = Vec2Normalize(context->vVelDirSelf);
    if (v6 >= 1.0)
        goto LABEL_8;
    *vVelDirSelf = self->Path.lookaheadDir[0];
    vVelDirSelf[1] = self->Path.lookaheadDir[1];
    if (*vVelDirSelf != 0.0 || context->vVelDirSelf[1] != 0.0)
    {
        v8 = g_actorAssumedSpeed[self->species];
        context->vVelSelf[0] = *vVelDirSelf * g_actorAssumedSpeed[self->species];
        context->vVelSelf[1] = vVelDirSelf[1] * (float)v8;
        v6 = g_actorAssumedSpeed[self->species];
    LABEL_8:
        v7 = 0;
        context->fVelSelfSqrd = (float)v6 * (float)v6;
        return v7;
    }
    if (Path_UsesObstacleNegotiation(&self->Path))
    {
        v7 = 1;
        *result = AI_TEAMMOVE_TRAVEL;
    }
    else
    {
        Actor_ClearPath(self);
        v7 = 1;
        *result = AI_TEAMMOVE_WAIT;
    }
    return v7;
}

float __cdecl Actor_TeamMoveDeltaCorrection(actor_s *self, double fVelSelfSqrd)
{
    double v2; // fp1

    if (self->moveMode)
        v2 = (float)((float)fVelSelfSqrd * (float)0.010000001);
    else
        v2 = 0.0;
    return *((float *)&v2 + 1);
}

void __cdecl Actor_AddToList(int *dodgeEntities, int *dodgeEntityCount, int arraysz, actor_s *pOtherActor)
{
    if (!pOtherActor->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 192, 0, "%s", "pOtherActor->ent");
    if (*dodgeEntityCount >= arraysz)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp",
            193,
            0,
            "%s",
            "(*dodgeEntityCount) < arraysz");
    dodgeEntities[(*dodgeEntityCount)++] = pOtherActor->ent->s.number;
}

void __cdecl Actor_TeamMoveSetDodge(team_move_context_t *context, team_move_other_context_t *context_other)
{
    context->fDodgePosDeltaLengthSqrd = context_other->fPosDeltaLengthSqrd;
    context->pDodgeOther = context_other->other;
}

int __cdecl Actor_TeamMoveShouldTryDodgeSentient(
    team_move_context_t *context,
    team_move_other_context_t *context_other)
{
    actor_s *self; // r27
    sentient_s *other; // r22
    char v6; // r29
    actor_s *actor; // r23
    int result; // r3
    gentity_s *ent; // r11
    double v10; // fp0
    double v11; // fp0
    double fPosDeltaLengthSqrd; // fp12
    bool v13; // r24
    double v14; // fp30
    gclient_s *client; // r11
    char v16; // r11
    char v17; // r28
    bool v18; // r30
    double v19; // fp0

    self = context->self;
    other = context_other->other;
    if (!context->self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 225, 0, "%s", "self");
    if (!other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 226, 0, "%s", "other");
    v6 = 1;
    actor = other->ent->actor;
    if (actor
        && (((1 << other->eTeam) & ~(1 << Sentient_EnemyTeam(self->sentient->eTeam))) == 0
            || other == self->sentient
            || actor->pPileUpEnt == self->ent))
    {
        return 0;
    }
    ent = other->ent;
    context_other->vOrgOther[0] = other->ent->s.lerp.pos.trBase[0];
    context_other->vOrgOther[1] = ent->s.lerp.pos.trBase[1];
    context_other->vOrgOther[2] = ent->s.lerp.pos.trBase[2];
    if (fabsf((float)(context->vOrgSelf[2] - context_other->vOrgOther[2])) >= 80.0)
        return 0;
    context_other->vDelta[0] = context_other->vOrgOther[0] - context->vOrgSelf[0];
    context_other->vDelta[1] = context_other->vOrgOther[1] - context->vOrgSelf[1];
    v10 = (float)((float)(context_other->vDelta[0] * context_other->vDelta[0])
        + (float)(context_other->vDelta[1] * context_other->vDelta[1]));
    context_other->fPosDeltaLengthSqrd = (float)(context_other->vDelta[0] * context_other->vDelta[0])
        + (float)(context_other->vDelta[1] * context_other->vDelta[1]);
    if (v10 >= context->fMaxIntervalSqrd)
        return 0;
    v11 = (float)((float)((float)(context->vVelSelf[1] * context_other->vDelta[1])
        + (float)(context->vVelSelf[0] * context_other->vDelta[0]))
        - context->fDeltaCorrection);
    context_other->fScale = (float)((float)(context->vVelSelf[1] * context_other->vDelta[1])
        + (float)(context->vVelSelf[0] * context_other->vDelta[0]))
        - context->fDeltaCorrection;
    if (v11 <= 0.0
        && (float)((float)(self->Path.forwardLookaheadDir2D[1] * context_other->vDelta[1])
            + (float)(self->Path.forwardLookaheadDir2D[0] * context_other->vDelta[0])) <= 0.0
        && !context->bFailedLookahead)
    {
        return 0;
    }
    fPosDeltaLengthSqrd = context_other->fPosDeltaLengthSqrd;
    v13 = fPosDeltaLengthSqrd < 900.0;
    v14 = (float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
        + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1]));
    if (actor)
    {
        if (actor->pPileUpActor || actor->pCloseEnt.isDefined())
        {
            context_other->vVelOther[0] = 0.0;
            context_other->vVelOther[1] = 0.0;
        }
        else
        {
            context_other->vVelOther[0] = actor->Physics.vWishDelta[0] * (float)20.0;
            context_other->vVelOther[1] = actor->Physics.vWishDelta[1] * (float)20.0;
        }
        if (v13 && !self->pCloseEnt.isDefined()  && level.gentities[actor->Physics.iHitEntnum].sentient)
            self->pCloseEnt.setEnt(actor->ent);
    }
    else
    {
        if (fPosDeltaLengthSqrd >= context->fDodgePosDeltaLengthSqrd || (self->Physics.iTraceMask & 0x2000000) == 0)
            return 0;
        if (Actor_IsEnemy(self, other))
            return 1;
        if (!other->ent->client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 290, 0, "%s", "other->ent->client");
        client = other->ent->client;
        context_other->vVelOther[0] = client->ps.velocity[0];
        context_other->vVelOther[1] = client->ps.velocity[1];
    }
    if ((float)((float)(context_other->vVelOther[0] * context_other->vVelOther[0])
        + (float)(context_other->vVelOther[1] * context_other->vVelOther[1])) < 1.0
        || (v16 = 1,
            v14 >= (float)((float)((float)(context_other->vVelOther[0] * context_other->vVelOther[0])
                + (float)(context_other->vVelOther[1] * context_other->vVelOther[1]))
                * (float)25.0)))
    {
        v16 = 0;
    }
    v17 = v16;
    v18 = self == self->pPileUpActor;
    if (self == self->pPileUpActor || !v16)
    {
    LABEL_46:
        if (actor)
            Actor_AddToList(context->dodgeEntities, &context->dodgeEntityCount, 33, actor);
        goto LABEL_48;
    }
    if (context_other->fPosDeltaLengthSqrd >= (double)context->fIntervalSqrd)
        return 0;
    if ((float)((float)(context->vVelSelf[1] * context_other->vVelOther[1])
        + (float)(context->vVelSelf[0] * context_other->vVelOther[0])) < 0.0)
    {
        if (!context->bPileUp && (v14 >= 1.0 || !v13))
        {
            if (self->ent->s.number <= (unsigned int)other->ent->s.number)
                return 0;
            v6 = 0;
        }
        goto LABEL_46;
    }
    if ((float)(context->fDeltaCorrection + context_other->fScale) <= 0.0)
        return 0;
    if (actor)
    {
        if (self->ent->s.number > (unsigned int)other->ent->s.number
            && (float)((float)(context_other->vVelOther[1] * context_other->vDelta[1])
                + (float)(context_other->vVelOther[0] * context_other->vDelta[0])) < (double)context->fDeltaCorrection)
        {
            return 0;
        }
        goto LABEL_46;
    }
LABEL_48:
    v19 = context_other->fPosDeltaLengthSqrd;
    if (v19 >= context->fDodgePosDeltaLengthSqrd)
        return 0;
    if (v18 || !v17)
        return 1;
    if (v19 >= context->fSlowDownPosDeltaLengthSqrd)
        return 0;
    if (v6)
    {
        context->fSlowDownPosDeltaLengthSqrd = context_other->fPosDeltaLengthSqrd;
        context->pSlowDownOther = other;
        result = 0;
        context->vVelSlowDownOther[0] = context_other->vVelOther[0];
        context->vVelSlowDownOther[1] = context_other->vVelOther[1];
        return result;
    }
    return 1;
}

int __cdecl Actor_TeamMoveTryDodge(team_move_context_t *context, team_move_other_context_t *context_other)
{
    actor_s *self; // r10
    sentient_s *other; // r9
    path_t *p_Path; // r29
    double v7; // fp0
    double v8; // fp13
    double v9; // fp0
    char v10; // r28
    double v11; // fp0
    gentity_s *ent; // r11
    int result; // r3
    double v14; // fp0
    double v15; // fp0
    float v16; // [sp+50h] [-50h] BYREF
    float v17; // [sp+54h] [-4Ch]

    self = context->self;
    other = context_other->other;
    p_Path = &context->self->Path;
    v7 = (float)((float)(context_other->vDelta[1] * context->vVelDirSelf[0])
        - (float)(context->vVelDirSelf[1] * context_other->vDelta[0]));
    v8 = (float)-(float)((float)((float)v7 * (float)v7) - (float)900.0);
    if (v8 <= 0.0)
        return 0;
    v9 = (float)((float)((float)(context->vVelDirSelf[1] * context_other->vDelta[1])
        + (float)(context->vVelDirSelf[0] * context_other->vDelta[0]))
        - context->self->Path.fLookaheadDist);
    if (v9 > 0.0 && (float)((float)v9 * (float)v9) >= (double)(float)((float)((float)v8 + (float)1406.25) - (float)900.0))
        return 0;
    if (!context->bFailedLookahead)
    {
        v10 = 0;
        v11 = (float)(context_other->fScale / context->fVelSelfSqrd);
        context_other->fScale = context_other->fScale / context->fVelSelfSqrd;
        ent = other->ent;
        context_other->vPerp[0] = (float)(context->vVelSelf[0] * (float)-v11) + context_other->vDelta[0];
        context_other->vPerp[1] = (float)(context->vVelSelf[1] * (float)-v11) + context_other->vDelta[1];
        if (ent->s.number == p_Path->wDodgeEntity)
        {
            if (context->bPileUp)
            {
                if ((p_Path->wDodgeCount || (p_Path->flags & 2) != 0)
                    && !Path_CompleteLookahead(p_Path)
                    && p_Path->fLookaheadDist < (double)(float)((float)sqrtf(context_other->fPosDeltaLengthSqrd) + (float)37.5))
                {
                    return 1;
                }
            }
            else if (self->moveMode && level.time < self->iTeamMoveDodgeTime)
            {
                if (context_other->fScale <= 0.0
                    || (float)((float)(context_other->vPerp[0] * context_other->vPerp[0])
                        + (float)(context_other->vPerp[1] * context_other->vPerp[1])) >= 900.0)
                {
                    return 0;
                }
                v10 = 1;
            }
        }
        if (v10)
            goto LABEL_33;
        if ((float)((float)(context_other->vPerp[0] * context_other->vPerp[0])
            + (float)(context_other->vPerp[1] * context_other->vPerp[1])) >= 1406.25)
        {
            if (context_other->fPosDeltaLengthSqrd >= 3600.0 || !Path_GetForwardStartPos(p_Path, context->vOrgSelf, &v16))
                return 0;
        LABEL_25:
            context_other->vDelta[0] = context_other->vOrgOther[0] - v16;
            context_other->vDelta[1] = context_other->vOrgOther[1] - v17;
            goto LABEL_28;
        }
        if (context_other->fScale > 0.0)
        {
        LABEL_33:
            if (Path_GetForwardStartPos(p_Path, context->vOrgSelf, &v16))
            {
                context_other->vDelta[0] = context_other->vOrgOther[0] - v16;
                context_other->vDelta[1] = context_other->vOrgOther[1] - v17;
                if ((float)((float)(context->vVelSelf[1] * context_other->vDelta[1])
                    + (float)(context->vVelSelf[0] * context_other->vDelta[0])) > 0.0)
                {
                LABEL_28:
                    v14 = (float)((float)(p_Path->forwardLookaheadDir2D[1] * context_other->vDelta[1])
                        + (float)(p_Path->forwardLookaheadDir2D[0] * context_other->vDelta[0]));
                    context_other->fScale = (float)(p_Path->forwardLookaheadDir2D[1] * context_other->vDelta[1])
                        + (float)(p_Path->forwardLookaheadDir2D[0] * context_other->vDelta[0]);
                    if (v14 > 0.0)
                    {
                        v15 = -v14;
                        result = 0;
                        context_other->vPerp[0] = (float)((float)v15 * p_Path->forwardLookaheadDir2D[0]) + context_other->vDelta[0];
                        context_other->vPerp[1] = (float)(p_Path->forwardLookaheadDir2D[1] * (float)v15) + context_other->vDelta[1];
                        if ((float)((float)(context_other->vPerp[0] * context_other->vPerp[0])
                            + (float)(context_other->vPerp[1] * context_other->vPerp[1])) >= 900.0)
                            return result;
                        return 1;
                    }
                    return 0;
                }
            }
        }
        else
        {
            if (context_other->fPosDeltaLengthSqrd >= 3600.0)
                return 0;
            if (Path_GetForwardStartPos(p_Path, context->vOrgSelf, &v16))
                goto LABEL_25;
        }
    }
    return 1;
}

int __cdecl Actor_TeamMoveConsiderSlowDown(team_move_context_t *context, ai_teammove_t *eResult)
{
    double fSlowDownPosDeltaLengthSqrd; // fp0
    sentient_s *pSlowDownOther; // r8
    actor_s *actor; // r10
    char v5; // r11

    fSlowDownPosDeltaLengthSqrd = context->fSlowDownPosDeltaLengthSqrd;
    if (fSlowDownPosDeltaLengthSqrd >= context->fDodgePosDeltaLengthSqrd)
        return 1;
    pSlowDownOther = context->pSlowDownOther;
    actor = pSlowDownOther->ent->actor;
    if (actor
        && (float)((float)(context->vVelSelf[1] * context->vVelSlowDownOther[1])
            + (float)(context->vVelSelf[0] * context->vVelSlowDownOther[0])) >= 0.0)
    {
        if (actor->moveMode != 2 || (v5 = 1, fSlowDownPosDeltaLengthSqrd < context->fWalkIntervalSqrd))
            v5 = 0;
        if (actor->eAnimMode == AI_ANIM_MOVE_CODE && (actor->moveMode == 3 || v5))
        {
            *eResult = AI_TEAMMOVE_SLOW_DOWN;
            return 1;
        }
        context->fDodgePosDeltaLengthSqrd = context->fSlowDownPosDeltaLengthSqrd;
        context->pDodgeOther = pSlowDownOther;
        return 1;
    }
    else
    {
        context->self->iTeamMoveWaitTime = level.time + 500;
        return 0;
    }
}

ai_teammove_t __cdecl Actor_TeamMoveNoDodge(team_move_context_t *context, ai_teammove_t eResult)
{
    actor_s *self; // r29
    path_t *p_Path; // r31
    sentient_s *pSlowDownOther; // r11
    ai_teammove_t result; // r3
    int wDodgeEntity; // r9
    char v8; // r11

    self = context->self;
    p_Path = &context->self->Path;
    if (context->bFailedLookahead
        || !context->pSlowDownOther
        || (float)((float)(context->vVelSlowDownOther[0] * context->vVelSlowDownOther[0])
            + (float)(context->vVelSlowDownOther[1] * context->vVelSlowDownOther[1])) >= (double)context->fVelSelfSqrd)
    {
        if (!context->self->Path.wDodgeCount)
            goto LABEL_14;
        wDodgeEntity = context->self->Path.wDodgeEntity;
        if (wDodgeEntity == 2175)
            goto LABEL_14;
        if (context->self->Path.wDodgeCount >= 0 || (v8 = 1, context->self->Path.fLookaheadDist >= 37.5))
            v8 = 0;
        if (v8 || Vec2DistanceSq(level.gentities[wDodgeEntity].r.currentOrigin, context->vOrgSelf) >= 3600.0)
        {
        LABEL_14:
            if ((unsigned __int16)p_Path->wDodgeCount >= 0x8000u)
                p_Path->wDodgeCount = 0;
            if (p_Path->wDodgeEntity != 2175)
            {
                p_Path->wDodgeEntity = 2175;
                if (!self->moveMode && level.time >= self->iTeamMoveDodgeTime)
                    return AI_TEAMMOVE_WAIT;
            }
            self->iTeamMoveDodgeTime = 0;
        }
        return eResult;
    }
    else
    {
        if (!context->self->Path.wDodgeCount)
            context->self->Path.wDodgeCount = -1;
        pSlowDownOther = context->pSlowDownOther;
        result = AI_TEAMMOVE_TRAVEL;
        p_Path->wDodgeEntity = pSlowDownOther->ent->s.number;
    }
    return result;
}

void __cdecl Actor_TeamMoveInitializeContext(
    actor_s *self,
    bool bUseInterval,
    bool bAllowGoalPileUp,
    team_move_context_t *context)
{
    double fInterval; // fp0
    double v7; // fp13
    double v8; // fp0
    bool v11; // r11
    double fMaxIntervalSqrd; // fp0
    double fIntervalSqrd; // fp13

    if (bUseInterval)
    {
        fInterval = self->fInterval;
        v7 = (float)(self->fInterval * (float)0.5);
        if (v7 < 37.5)
        {
            v7 = 37.5;
            if (fInterval < 30.0)
                fInterval = 30.0;
        }
        context->fIntervalSqrd = (float)fInterval * (float)fInterval;
        v8 = (float)((float)v7 * (float)v7);
    }
    else
    {
        context->fIntervalSqrd = 900.0;
        v8 = 1406.25;
    }
    context->fWalkIntervalSqrd = v8;

    //_FP12 = (float)(context->fIntervalSqrd - context->fVelSelfSqrd);
    //__asm { fsel      f0, f12, f13, f0 }
    //context->fMaxIntervalSqrd = _FP0;

    float diff = context->fIntervalSqrd - context->fVelSelfSqrd;
    context->fMaxIntervalSqrd = (diff < 0.0f) ? diff : context->fMaxIntervalSqrd;

    v11 = self->pPileUpActor || bAllowGoalPileUp && Actor_IsAtGoal(self);
    fMaxIntervalSqrd = context->fMaxIntervalSqrd;
    fIntervalSqrd = context->fIntervalSqrd;
    context->bPileUp = v11;
    context->fDodgePosDeltaLengthSqrd = fMaxIntervalSqrd;
    context->dodgeEntityCount = 0;
    context->fSlowDownPosDeltaLengthSqrd = fIntervalSqrd;
    context->pDodgeOther = 0;
    context->pSlowDownOther = 0;
    context->bFailedLookahead = Path_FailedLookahead(&self->Path);
    if (self->moveMode)
        context->fDeltaCorrection = context->fVelSelfSqrd * (float)0.010000001;
    else
        context->fDeltaCorrection = 0.0;
}

int __cdecl Actor_TeamMoveTrimPath(path_t *pPath, const team_move_context_t *context)
{
    __int16 wDodgeCount; // r9
    int wPathLen; // r10
    int lookaheadNextNode; // r31
    const float *vOrigPoint; // r30

    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 557, 0, "%s", "pPath->wPathLen > 0");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp",
            558,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    wDodgeCount = pPath->wDodgeCount;
    wPathLen = pPath->wPathLen;
    lookaheadNextNode = wPathLen - 1;
    if (wDodgeCount > 0)
        lookaheadNextNode -= wDodgeCount;
    if (lookaheadNextNode > pPath->lookaheadNextNode)
        lookaheadNextNode = pPath->lookaheadNextNode;
    if (lookaheadNextNode >= wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp",
            567,
            0,
            "%s",
            "startIndex < pPath->wPathLen");
    if (lookaheadNextNode == pPath->wPathLen - 1)
    {
        if (Vec2DistanceSq(pPath->vCurrPoint, context->vOrgSelf) >= (double)context->fDodgePosDeltaLengthSqrd)
            return lookaheadNextNode;
        --lookaheadNextNode;
    }
    if (lookaheadNextNode >= pPath->wNegotiationStartNode)
    {
        vOrigPoint = pPath->pts[lookaheadNextNode].vOrigPoint;
        do
        {
            if (Vec2DistanceSq(vOrigPoint, context->vOrgSelf) >= (double)context->fDodgePosDeltaLengthSqrd)
                break;
            --lookaheadNextNode;
            vOrigPoint -= 7;
        } while (lookaheadNextNode >= pPath->wNegotiationStartNode);
    }
    return lookaheadNextNode;
}

void __cdecl Actor_TeamMoveTooCloseMoveAway(const actor_s *self, int mask, team_move_context_t *context)
{
    gentity_s *ent; // r11
    double v7; // fp31
    double v8; // fp30
    double v9; // fp29
    double v10; // fp0
    gentity_s *v11; // r11
    float v12; // [sp+50h] [-60h] BYREF
    float v13; // [sp+54h] [-5Ch]
    float v14[6]; // [sp+58h] [-58h] BYREF

    if (context->fDodgePosDeltaLengthSqrd < 1406.25)
    {
        if (!context->pDodgeOther)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 597, 0, "%s", "context.pDodgeOther");
        ent = context->pDodgeOther->ent;
        v7 = ent->r.currentOrigin[0];
        v8 = ent->r.currentOrigin[1];
        v9 = ent->r.currentOrigin[2];
        v10 = (float)(ent->r.currentOrigin[1] - context->vOrgSelf[1]);
        v12 = ent->r.currentOrigin[0] - context->vOrgSelf[0];
        v13 = v10;
        if (Vec2Normalize(&v12) < 37.5)
        {
            v11 = self->ent;
            v14[2] = v9;
            v14[0] = -(float)((float)(v12 * (float)37.5) - (float)v7);
            v14[1] = -(float)((float)(v13 * (float)37.5) - (float)v8);
            Path_PredictionTraceCheckForEntities(
                context->vOrgSelf,
                v14,
                context->dodgeEntities,
                context->dodgeEntityCount,
                v11->s.number,
                mask,
                context->vOrgSelf);
        }
    }
}

int __cdecl Actor_TeamMoveCheckPileup(actor_s *self, actor_s *pOtherActor)
{
    actor_s *pPileUpActor; // r11
    int result; // r3
    int time; // r10

    if (!pOtherActor)
        return 0;
    Actor_UpdatePileUp(pOtherActor);
    pPileUpActor = pOtherActor->pPileUpActor;
    if (!pPileUpActor
        || pPileUpActor == self
        || pOtherActor->pPileUpEnt == self->ent
        || self->eState[self->stateLevel] == AIS_GRENADE_RESPONSE)
    {
        return 0;
    }
    if ((unsigned __int16)self->Path.wDodgeCount >= 0x8000u)
        self->Path.wDodgeCount = 0;
    result = 1;
    self->pPileUpActor = pOtherActor->pPileUpActor;
    self->pPileUpEnt = pOtherActor->pPileUpEnt;
    time = level.time;
    self->iTeamMoveDodgeTime = 0;
    self->iTeamMoveWaitTime = time + 500;
    return result;
}

void __cdecl Actor_DodgeDebug(
    actor_s *self,
    actor_s *otherActor,
    const char *debugString,
    int a4,
    int a5,
    int a6,
    int a7)
{
    int number; // r6

    if (ai_showDodge->current.enabled)
    {
        if (otherActor)
            number = otherActor->ent->s.number;
        else
            number = 2175;
        Com_Printf(18, debugString, self->ent->s.number, number, a5, a6, a7, otherActor);
    }
}

ai_teammove_t __cdecl Actor_GetTeamMoveStatus(actor_s *self, bool bUseInterval, bool bAllowGoalPileUp)
{
    gentity_s *ent; // r11
    ai_teammove_t result; // r3
    sentient_s *sentients; // r11
    sentient_s *i; // r31
    sentient_s *pDodgeOther; // r4
    sentient_s *TargetSentient; // r3
    actor_s *actor; // r27
    bool v13; // r25
    int v14; // r3
    int v15; // r28
    int v16; // r29
    int iTraceMask; // r11
    int v18; // r31
    gentity_s *v19; // r10
    double v20; // fp31
    double v21; // fp30
    double v22; // fp11
    double v23; // fp10
    double v24; // fp13
    int number; // r6
    gentity_s *v26; // r11
    int v27; // r6
    unsigned int v28; // r11
    ai_animmode_t eAnimMode; // r11
    ai_state_t v30; // r11
    int time; // r11
    unsigned int v32; // r10
    ai_state_t v33; // r11
    int moveMode; // r10
    sentient_s *v35; // r11
    int v36; // r6
    int v37; // r6
    int v38; // r10
    int v39; // [sp+8h] [-288h]
    int v40; // [sp+8h] [-288h]
    int v41; // [sp+Ch] [-284h]
    int v42; // [sp+Ch] [-284h]
    int v43; // [sp+10h] [-280h]
    int v44; // [sp+10h] [-280h]
    int v45; // [sp+14h] [-27Ch]
    int v46; // [sp+14h] [-27Ch]
    int v47; // [sp+18h] [-278h]
    int v48; // [sp+18h] [-278h]
    int v49; // [sp+1Ch] [-274h]
    int v50; // [sp+1Ch] [-274h]
    int v51; // [sp+20h] [-270h]
    int v52; // [sp+20h] [-270h]
    int v53; // [sp+24h] [-26Ch]
    int v54; // [sp+24h] [-26Ch]
    int v55; // [sp+28h] [-268h]
    int v56; // [sp+28h] [-268h]
    int v57; // [sp+2Ch] [-264h]
    int v58; // [sp+2Ch] [-264h]
    int v59; // [sp+30h] [-260h]
    int v60; // [sp+30h] [-260h]
    int v61; // [sp+34h] [-25Ch]
    int v62; // [sp+34h] [-25Ch]
    int v63; // [sp+38h] [-258h]
    int v64; // [sp+38h] [-258h]
    int v65; // [sp+3Ch] [-254h]
    int v66; // [sp+3Ch] [-254h]
    int v67; // [sp+40h] [-250h]
    int v68; // [sp+40h] [-250h]
    int v69; // [sp+44h] [-24Ch]
    int v70; // [sp+44h] [-24Ch]
    int v71; // [sp+48h] [-248h]
    int v72; // [sp+48h] [-248h]
    int v73; // [sp+4Ch] [-244h]
    int v74; // [sp+4Ch] [-244h]
    int v75; // [sp+50h] [-240h]
    int v76; // [sp+50h] [-240h]
    int v77; // [sp+58h] [-238h]
    int v78; // [sp+58h] [-238h]
    ai_teammove_t v79[2]; // [sp+60h] [-230h] BYREF
    float v80; // [sp+68h] [-228h] BYREF
    float v81; // [sp+6Ch] [-224h]
    float v82; // [sp+70h] [-220h]
    float v83; // [sp+78h] [-218h] BYREF
    float v84; // [sp+7Ch] [-214h]
    float v85; // [sp+80h] [-210h]
    const char *v86; // [sp+84h] [-20Ch]
    const char *v87; // [sp+88h] [-208h]
    team_move_context_t v88; // [sp+90h] [-200h] BYREF
    team_move_other_context_t v89[2]; // [sp+170h] [-120h] BYREF

    v86 = "c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp";
    v87 = "%s";
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 667, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 668, 0, "%s", "self->sentient");
    if (!Actor_HasPath(self))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 669, 0, "%s", "Actor_HasPath( self )");
    if (self->eAnimMode != AI_ANIM_MOVE_CODE)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp",
            670,
            0,
            "%s",
            "self->eAnimMode == AI_ANIM_MOVE_CODE");
    ent = self->ent;
    v88.vOrgSelf[0] = self->ent->r.currentOrigin[0];
    v79[0] = AI_TEAMMOVE_TRAVEL;
    v88.vOrgSelf[1] = ent->r.currentOrigin[1];
    v88.vOrgSelf[2] = ent->r.currentOrigin[2];
    if ((unsigned __int8)Actor_TeamMoveCheckWaitTimer(self, v79))
        return v79[0];
    v88.self = self;
    if ((unsigned __int8)Actor_TeamMoveCalcMovementDir(&v88, v79))
        return v79[0];
    Actor_TeamMoveInitializeContext(self, bUseInterval, bAllowGoalPileUp, &v88);
    sentients = level.sentients;
    for (i = level.sentients; i < &sentients[33]; ++i)
    {
        if (i->inuse)
        {
            v89[0].other = i;
            if ((unsigned __int8)Actor_TeamMoveShouldTryDodgeSentient(&v88, v89)
                && (unsigned __int8)Actor_TeamMoveTryDodge(&v88, v89))
            {
                v88.fDodgePosDeltaLengthSqrd = v89[0].fPosDeltaLengthSqrd;
                v88.pDodgeOther = v89[0].other;
            }
            sentients = level.sentients;
        }
    }
    if (v88.pSlowDownOther && !(unsigned __int8)Actor_TeamMoveConsiderSlowDown(&v88, v79))
        return AI_TEAMMOVE_WAIT;
    Actor_ClearPileUp(self);
    pDodgeOther = v88.pDodgeOther;
    if (!v88.pDodgeOther || self->noDodgeMove || ai_noDodge->current.enabled)
        return Actor_TeamMoveNoDodge(&v88, v79[0]);
    if (self->Path.wDodgeEntity != v88.pDodgeOther->ent->s.number)
    {
        self->iTeamMoveDodgeTime = 0;
        if ((unsigned __int16)self->Path.wDodgeCount >= 0x8000u)
            self->Path.wDodgeCount = 0;
        self->Path.wDodgeEntity = pDodgeOther->ent->s.number;
    }
    if (Actor_IsEnemy(self, pDodgeOther))
    {
        if (v88.fDodgePosDeltaLengthSqrd < 1406.25)
        {
            TargetSentient = Actor_GetTargetSentient(self);
            if (TargetSentient == v88.pDodgeOther)
                Actor_ClearPath(self);
            result = AI_TEAMMOVE_WAIT;
            self->iTeamMoveWaitTime = level.time + 500;
            return result;
        }
        return v79[0];
    }
    actor = v88.pDodgeOther->ent->actor;
    v13 = Actor_TeamMoveNeedToCheckWait(self->moveMode, &self->Path);
    if (v13)
        goto checkwait;
    while (1)
    {
        if (level.time < self->iTeamMoveDodgeTime)
            return v79[0];
        v14 = Actor_TeamMoveTrimPath(&self->Path, &v88);
        v15 = v14;
        if (self->Path.fLookaheadDistToNextNode == 0.0
            || self->Path.fLookaheadDist < (double)(float)((float)sqrtf(v88.fDodgePosDeltaLengthSqrd) + (float)37.5))
        {
            v16 = 0;
            if (v14 < self->Path.wNegotiationStartNode)
                goto failed_dodge;
        }
        else
        {
            v16 = 1;
            if (self->Path.lookaheadNextNode >= self->Path.wPathLen - 1)
                MyAssertHandler(v86, 753, 0, v87, "!bCheckLookahead || pPath->lookaheadNextNode < pPath->wPathLen - 1");
        }
        if (v88.vVelDirSelf[0] == 0.0 && v88.vVelDirSelf[1] == 0.0)
            MyAssertHandler(v86, 758, 0, v87, "context.vVelDirSelf[0] || context.vVelDirSelf[1]");
        iTraceMask = self->Physics.iTraceMask;
        v18 = iTraceMask | 4;
        if (!actor)
            v18 = iTraceMask | 0x4004;
        Actor_TeamMoveTooCloseMoveAway(self, v18, &v88);
        v19 = self->ent;
        v20 = (float)(v88.vVelDirSelf[1] * (float)37.5);
        v21 = (float)(v88.vVelDirSelf[0] * (float)-37.5);
        v22 = (float)-(float)((float)(v88.vVelDirSelf[0] * (float)37.5) - v88.pDodgeOther->ent->r.currentOrigin[0]);
        v23 = (float)(v88.pDodgeOther->ent->r.currentOrigin[1] - (float)(v88.vVelDirSelf[1] * (float)37.5));
        v24 = v88.pDodgeOther->ent->r.currentOrigin[2];
        v85 = v88.pDodgeOther->ent->r.currentOrigin[2];
        v82 = v24;
        v80 = (float)v22 + (float)(v88.vVelDirSelf[1] * (float)37.5);
        v81 = (float)v23 + (float)(v88.vVelDirSelf[0] * (float)-37.5);
        v83 = (float)(v88.vVelDirSelf[0] * (float)75.0) + v80;
        v84 = (float)(v88.vVelDirSelf[1] * (float)75.0) + v81;
        if ((unsigned __int8)Path_AttemptDodge(
            &self->Path,
            v88.vOrgSelf,
            &v80,
            &v83,
            v15,
            v88.dodgeEntities,
            v88.dodgeEntityCount,
            v19->s.number,
            v39,
            v41,
            v43,
            v45,
            v47,
            v49,
            v51,
            v53,
            v55,
            v57,
            v59,
            v61,
            v63,
            v65,
            v67,
            v69,
            v71,
            v73,
            v75,
            v18,
            v77,
            v16))
            break;
        if (ai_showDodge->current.enabled)
        {
            if (actor)
                number = actor->ent->s.number;
            else
                number = 2175;
            Com_Printf(18, "AI %d failed right dodge pathing AI %d\n", self->ent->s.number, number);
        }
        v26 = self->ent;
        v80 = v80 - (float)((float)v20 * (float)2.0);
        v81 = v81 - (float)((float)v21 * (float)2.0);
        v83 = v83 - (float)((float)v20 * (float)2.0);
        v84 = v84 - (float)((float)v21 * (float)2.0);
        if ((unsigned __int8)Path_AttemptDodge(
            &self->Path,
            v88.vOrgSelf,
            &v80,
            &v83,
            v15,
            v88.dodgeEntities,
            v88.dodgeEntityCount,
            v26->s.number,
            v40,
            v42,
            v44,
            v46,
            v48,
            v50,
            v52,
            v54,
            v56,
            v58,
            v60,
            v62,
            v64,
            v66,
            v68,
            v70,
            v72,
            v74,
            v76,
            v18,
            v78,
            v16))
        {
            if (ai_showDodge->current.enabled)
            {
                if (actor)
                    v36 = actor->ent->s.number;
                else
                    v36 = 2175;
                Com_Printf(18, "AI %d left dodge succeed AI %d\n", self->ent->s.number, v36);
            }
            goto LABEL_114;
        }
        if (ai_showDodge->current.enabled)
        {
            if (actor)
                v27 = actor->ent->s.number;
            else
                v27 = 2175;
            Com_Printf(18, "AI %d failed left dodge pathing AI %d\n", self->ent->s.number, v27);
        }
    failed_dodge:
        if (actor)
        {
            v28 = 4 * (self->stateLevel + 8);
            if (*(gentity_s **)((char *)&self->ent + v28) == (gentity_s *)100
                && (*(gentity_s **)((char *)&actor->ent + v28) != (gentity_s *)100 || !Actor_HasPath(actor)))
            {
                goto dodge;
            }
        }
        if (v88.fDodgePosDeltaLengthSqrd >= 1406.25)
        {
            if (self->moveMode)
                return v79[0];
            if (v88.fDodgePosDeltaLengthSqrd >= 3600.0
                && (self->Path.fLookaheadDist >= 60.0 || Path_CompleteLookahead(&self->Path)))
            {
                goto dodge;
            }
        }
        if (self->Path.wDodgeCount >= 0)
        {
            if (!self->Path.wDodgeCount && actor)
            {
                eAnimMode = actor->eAnimMode;
                if ((eAnimMode == AI_ANIM_MOVE_CODE || eAnimMode == AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS)
                    && !actor->moveMode
                    && Actor_IsAtGoal(actor))
                {
                    if (ai_showDodge->current.enabled)
                        Com_Printf(18, "AI %d failed to dodge stationary AI %d\n", self->ent->s.number, actor->ent->s.number);
                    goto dodge;
                }
                v30 = actor->eState[actor->stateLevel];
                if (v30 == AIS_SCRIPTEDANIM || v30 == AIS_CUSTOMANIM)
                    goto LABEL_88;
            }
        }
        else if (self->Path.fLookaheadDist < 60.0 && !Path_CompleteLookahead(&self->Path))
        {
            goto failsafe;
        }
        time = level.time;
        //__twllei(0x1F4u, 0);
        //__twllei(0x1F4u, 0);
        //v32 = ~(__ROL4__(level.time, 1) - 1) & 0x1F4;
        //__twllei(4u, 0);
        //__twlgei(v32, 0xFFFFFFFF);
        //__twlgei(~(__ROL4__(time / 500, 1) - 1) & 4, 0xFFFFFFFF);

        v32 = (~((level.time << 1) - 1)) & 0x1F4;

        if (time / 500 % 4)
            goto LABEL_100;
        if ((unsigned __int16)self->Path.wDodgeCount < 0x8000u)
        {
            if (self->Path.wDodgeCount || v88.bFailedLookahead)
                Actor_ClearPath(self);
            else
                self->Path.wDodgeCount = -1;
            goto LABEL_100;
        }
    failsafe:
        if (!actor)
            goto LABEL_95;
        if (actor->eAnimMode != AI_ANIM_MOVE_CODE)
        {
            v33 = actor->eState[actor->stateLevel];
            if (v33 != AIS_SCRIPTEDANIM && v33 != AIS_CUSTOMANIM)
            {
            LABEL_95:
                self->Path.wDodgeCount = 0;
            LABEL_100:
                if (!actor || actor->pPileUpEnt != self->ent)
                {
                    v35 = v88.pDodgeOther;
                    self->pPileUpActor = self;
                    self->pPileUpEnt = v35->ent;
                }
                result = AI_TEAMMOVE_WAIT;
                self->iTeamMoveWaitTime = level.time + 500;
                return result;
            }
        LABEL_88:
            if (ai_showDodge->current.enabled)
                Com_Printf(18, "AI %d failed to dodge scripted AI %d\n", self->ent->s.number, actor->ent->s.number);
            goto dodge;
        }
        if (ai_showDodge->current.enabled)
            Com_Printf(18, "AI %d failed to dodge pathing AI %d\n", self->ent->s.number, actor->ent->s.number);
    dodge:
        moveMode = self->moveMode;
        self->iTeamMoveDodgeTime = level.time + 1000;
        if (moveMode)
            return v79[0];
        if (v13)
            return AI_TEAMMOVE_WAIT;
        v79[0] = AI_TEAMMOVE_WAIT;
    checkwait:
        if ((unsigned __int8)Actor_TeamMoveCheckPileup(self, actor))
            return AI_TEAMMOVE_WAIT;
    }
    if (ai_showDodge->current.enabled)
    {
        if (actor)
            v37 = actor->ent->s.number;
        else
            v37 = 2175;
        Com_Printf(18, "AI %d right dodge succeed AI %d\n", self->ent->s.number, v37);
    }
LABEL_114:
    v38 = self->moveMode;
    result = v79[0];
    self->iTeamMoveDodgeTime = level.time + 1000;
    if (v38)
        return result;
    return AI_TEAMMOVE_WAIT;
}

void __cdecl Actor_MoveAlongPathWithTeam(actor_s *self, bool bRun, bool bUseInterval, bool bAllowGoalPileUp)
{
    char IsMoving; // r26
    ai_teammove_t TeamMoveStatus; // r4
    unsigned int stateLevel; // r11
    const char *v11; // r3

    if (!Actor_HasPath(self))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 925, 0, "%s", "Actor_HasPath( self )");
    if (self->eAnimMode == AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS)
    {
        IsMoving = 0;
        TeamMoveStatus = AI_TEAMMOVE_TRAVEL;
    }
    else
    {
        IsMoving = Actor_IsMoving(self);
        if (!IsMoving)
        {
            stateLevel = self->stateLevel;
            self->moveMode = 0;
            self->eAnimMode = AI_ANIM_MOVE_CODE;
            if (stateLevel < 5 && self->eState[stateLevel + 1] != AIS_NEGOTIATION)
            {
                self->prevMoveDir[0] = 0.0;
                self->prevMoveDir[1] = 0.0;
            }
        }
        TeamMoveStatus = Actor_GetTeamMoveStatus(self, bUseInterval, bAllowGoalPileUp);
    }
    if (TeamMoveStatus)
    {
        if (TeamMoveStatus == AI_TEAMMOVE_WAIT)
        {
            Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
            self->arrivalInfo.animscriptOverrideRunTo = 0;
            return;
        }
        if ((unsigned int)TeamMoveStatus >= (AI_TEAMMOVE_SLOW_DOWN | AI_TEAMMOVE_WAIT))
        {
            if (!alwaysfails)
            {
                v11 = va("unhandled case %i for Actor_MoveAlongPathWithTeam", TeamMoveStatus);
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_team_move.cpp", 980, 0, v11);
            }
            return;
        }
        bRun = 0;
    }
    if (bRun)
        Actor_AnimTryRun(self);
    else
        Actor_AnimTryWalk(self);
    if (self->goalPosChanged && IsMoving || self->arrivalInfo.arrivalNotifyRequested)
    {
        if (Actor_CheckCoverApproach(self))
            Actor_CoverApproachNotify(self);
        self->arrivalInfo.arrivalNotifyRequested = 0;
    }
    if (!IsMoving && (unsigned __int8)Actor_IsMoving(self))
    {
        Actor_ClearMoveHistory(self);
        self->ent->flags &= 0xFFFFFFE7;
        self->Path.iPathEndTime = 0;
    }
}

