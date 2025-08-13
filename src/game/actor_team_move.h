#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include "sentient.h"

enum ai_teammove_t : __int32
{
    AI_TEAMMOVE_TRAVEL = 0x0,
    AI_TEAMMOVE_WAIT = 0x1,
    AI_TEAMMOVE_SLOW_DOWN = 0x2,
};

struct team_move_context_t
{
    actor_s *self;
    float vVelSelf[2];
    float vOrgSelf[3];
    float vVelDirSelf[2];
    float fVelSelfSqrd;
    float fDeltaCorrection;
    float fIntervalSqrd;
    float fWalkIntervalSqrd;
    float fMaxIntervalSqrd;
    int bFailedLookahead;
    float fDodgePosDeltaLengthSqrd;
    int dodgeEntities[33];
    int dodgeEntityCount;
    sentient_s *pDodgeOther;
    bool bPileUp;
    float fSlowDownPosDeltaLengthSqrd;
    sentient_s *pSlowDownOther;
    float vVelSlowDownOther[2];
};

struct team_move_other_context_t
{
    sentient_s *other;
    float vOrgOther[3];
    float vVelOther[2];
    float vDelta[2];
    float vPerp[2];
    float fPosDeltaLengthSqrd;
    float fScale;
};


void __cdecl Actor_TeamMoveBlocked(actor_s *self);
void __cdecl Actor_TeamMoveBlockedClear(actor_s *self);
int __cdecl Actor_TeamMoveCheckWaitTimer(actor_s *self, ai_teammove_t *result);
bool __cdecl Actor_TeamMoveNeedToCheckWait(unsigned __int8 moveMode, path_t *pPath);
bool __cdecl Actor_IsEnemy(actor_s *self, sentient_s *other);
void __cdecl Actor_CalcInterval(
    actor_s *self,
    bool bUseInterval,
    float *fIntervalSqrdOut,
    float *fWalkIntervalSqrdOut);
int __cdecl Actor_TeamMoveCalcMovementDir(team_move_context_t *context, ai_teammove_t *result);
float __cdecl Actor_TeamMoveDeltaCorrection(actor_s *self, double fVelSelfSqrd);
void __cdecl Actor_AddToList(int *dodgeEntities, int *dodgeEntityCount, int arraysz, actor_s *pOtherActor);
void __cdecl Actor_TeamMoveSetDodge(team_move_context_t *context, team_move_other_context_t *context_other);
int __cdecl Actor_TeamMoveShouldTryDodgeSentient(
    team_move_context_t *context,
    team_move_other_context_t *context_other);
int __cdecl Actor_TeamMoveTryDodge(team_move_context_t *context, team_move_other_context_t *context_other);
int __cdecl Actor_TeamMoveConsiderSlowDown(team_move_context_t *context, ai_teammove_t *eResult);
ai_teammove_t __cdecl Actor_TeamMoveNoDodge(team_move_context_t *context, ai_teammove_t eResult);
void __cdecl Actor_TeamMoveInitializeContext(
    actor_s *self,
    bool bUseInterval,
    bool bAllowGoalPileUp,
    team_move_context_t *context);
int __cdecl Actor_TeamMoveTrimPath(path_t *pPath, const team_move_context_t *context);
void __cdecl Actor_TeamMoveTooCloseMoveAway(const actor_s *self, int mask, team_move_context_t *context);
int __cdecl Actor_TeamMoveCheckPileup(actor_s *self, actor_s *pOtherActor);
void __cdecl Actor_DodgeDebug(
    actor_s *self,
    actor_s *otherActor,
    const char *debugString,
    int a4,
    int a5,
    int a6,
    int a7);
ai_teammove_t __cdecl Actor_GetTeamMoveStatus(actor_s *self, bool bUseInterval, bool bAllowGoalPileUp);
void __cdecl Actor_MoveAlongPathWithTeam(actor_s *self, bool bRun, bool bUseInterval, bool bAllowGoalPileUp);
