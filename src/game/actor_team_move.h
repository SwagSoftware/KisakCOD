#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __fastcall Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther);
void __fastcall Actor_TeamMoveBlocked(actor_s *self);
void __fastcall Actor_TeamMoveBlockedClear(actor_s *self);
int __fastcall Actor_TeamMoveCheckWaitTimer(actor_s *self, ai_teammove_t *result);
bool __fastcall Actor_TeamMoveNeedToCheckWait(unsigned __int8 moveMode, path_t *pPath);
bool __fastcall Actor_IsEnemy(actor_s *self, sentient_s *other);
void __fastcall Actor_CalcInterval(
    actor_s *self,
    bool bUseInterval,
    float *fIntervalSqrdOut,
    float *fWalkIntervalSqrdOut);
int __fastcall Actor_TeamMoveCalcMovementDir(team_move_context_t *context, ai_teammove_t *result);
float __fastcall Actor_TeamMoveDeltaCorrection(actor_s *self, double fVelSelfSqrd);
void __fastcall Actor_AddToList(int *dodgeEntities, int *dodgeEntityCount, int arraysz, actor_s *pOtherActor);
void __fastcall Actor_TeamMoveSetDodge(team_move_context_t *context, team_move_other_context_t *context_other);
int __fastcall Actor_TeamMoveShouldTryDodgeSentient(
    team_move_context_t *context,
    team_move_other_context_t *context_other);
int __fastcall Actor_TeamMoveTryDodge(team_move_context_t *context, team_move_other_context_t *context_other);
int __fastcall Actor_TeamMoveConsiderSlowDown(team_move_context_t *context, ai_teammove_t *eResult);
ai_teammove_t __fastcall Actor_TeamMoveNoDodge(team_move_context_t *context, ai_teammove_t eResult);
void __fastcall Actor_TeamMoveInitializeContext(
    actor_s *self,
    bool bUseInterval,
    bool bAllowGoalPileUp,
    team_move_context_t *context);
int __fastcall Actor_TeamMoveTrimPath(path_t *pPath, const team_move_context_t *context);
void __fastcall Actor_TeamMoveTooCloseMoveAway(const actor_s *self, int mask, team_move_context_t *context);
int __fastcall Actor_TeamMoveCheckPileup(actor_s *self, actor_s *pOtherActor);
void __fastcall Actor_DodgeDebug(
    actor_s *self,
    actor_s *otherActor,
    const char *debugString,
    int a4,
    int a5,
    int a6,
    int a7);
ai_teammove_t __fastcall Actor_GetTeamMoveStatus(actor_s *self, bool bUseInterval, bool bAllowGoalPileUp);
void __fastcall Actor_MoveAlongPathWithTeam(actor_s *self, bool bRun, bool bUseInterval, bool bAllowGoalPileUp);
