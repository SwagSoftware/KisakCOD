#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall Actor_Exposed_CheckLockGoal(actor_s *self);
void __fastcall Actor_Exposed_Combat(actor_s *self);
int __fastcall Actor_Exposed_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Exposed_Finish(actor_s *self, ai_state_t eNextState);
void __fastcall Actor_Exposed_Suspend(actor_s *self, ai_state_t eNextState);
int __fastcall Actor_Exposed_Resume(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Exposed_DecideSubState(actor_s *self);
void __fastcall Actor_Exposed_FindReacquireNode(actor_s *self);
pathnode_t *__fastcall Actor_Exposed_GetReacquireNode(actor_s *self);
int __fastcall Actor_Exposed_UseReacquireNode(actor_s *self, pathnode_t *pNode);
int __fastcall Actor_Exposed_ReacquireStepMove(actor_s *self, double fDist);
void __fastcall Actor_Exposed_FindReacquireDirectPath(actor_s *self, bool ignoreSuppression);
void __fastcall Actor_Exposed_FindReacquireProximatePath(actor_s *self, char ignoreSuppression);
int __fastcall Actor_Exposed_StartReacquireMove(actor_s *self);
void __fastcall Actor_Exposed_Reacquire_Move(actor_s *self);
void __fastcall Actor_Exposed_MoveToGoal_Move(actor_s *self);
bool __fastcall Actor_Exposed_IsShortMove(actor_s *self);
void __fastcall Actor_Exposed_NonCombat_Think(actor_s *self);
void __fastcall Actor_Exposed_FlashBanged(actor_s *self);
bool __fastcall Actor_Exposed_CheckStopMovingAndStartCombat(actor_s *self);
actor_think_result_t __fastcall Actor_Exposed_Think(actor_s *self);
void __fastcall Actor_Exposed_Touch(actor_s *self, gentity_s *pOther);
