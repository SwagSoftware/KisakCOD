#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_actor_state();
void __fastcall Actor_SetDefaultState(actor_s *actor);
int __fastcall Actor_StartState(actor_s *self, ai_state_t eStartedState);
void __fastcall Actor_FinishState(actor_s *self, ai_state_t eNextState);
void __fastcall Actor_SuspendState(actor_s *self, ai_state_t eNextState);
int __fastcall Actor_GetNextPopedState(actor_s *self);
int __fastcall Actor_ResumeState(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_StartDefaultState(actor_s *self);
void __fastcall Actor_ResumePopedState(actor_s *self, ai_state_t state);
void __fastcall Actor_ThinkStateTransitions(actor_s *self);
void __fastcall Actor_SetSubState(actor_s *self, ai_substate_t eSubState);
int __fastcall Actor_IsStateOnStack(const actor_s *self, ai_state_t eState);
int __fastcall Actor_PendingTransitionTo(actor_s *self, ai_state_t eState);
void __fastcall Actor_SimplifyStateTransitions(actor_s *self);
int __fastcall Actor_AllowedToPushState(actor_s *self, int eState);
void __fastcall Actor_OnStateChange(actor_s *self);
void __fastcall Actor_SetState(actor_s *self, ai_state_t eState);
void __fastcall Actor_PopState(actor_s *self);
void __fastcall Actor_ForceState(actor_s *self, ai_state_t eState);
void __fastcall Actor_PrepareToPushState(actor_s *self, int eState);
int __fastcall Actor_PushState(actor_s *self, ai_state_t eState);
