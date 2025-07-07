#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl TRACK_actor_state();
void __cdecl Actor_SetDefaultState(actor_s *actor);
int __cdecl Actor_StartState(actor_s *self, ai_state_t eStartedState);
void __cdecl Actor_FinishState(actor_s *self, ai_state_t eNextState);
void __cdecl Actor_SuspendState(actor_s *self, ai_state_t eNextState);
int __cdecl Actor_GetNextPopedState(actor_s *self);
int __cdecl Actor_ResumeState(actor_s *self, ai_state_t ePrevState);
void __cdecl Actor_StartDefaultState(actor_s *self);
void __cdecl Actor_ResumePopedState(actor_s *self, ai_state_t state);
void __cdecl Actor_ThinkStateTransitions(actor_s *self);
void __cdecl Actor_SetSubState(actor_s *self, ai_substate_t eSubState);
int __cdecl Actor_IsStateOnStack(const actor_s *self, ai_state_t eState);
int __cdecl Actor_PendingTransitionTo(actor_s *self, ai_state_t eState);
void __cdecl Actor_SimplifyStateTransitions(actor_s *self);
int __cdecl Actor_AllowedToPushState(actor_s *self, int eState);
void __cdecl Actor_OnStateChange(actor_s *self);
void __cdecl Actor_SetState(actor_s *self, ai_state_t eState);
void __cdecl Actor_PopState(actor_s *self);
void __cdecl Actor_ForceState(actor_s *self, ai_state_t eState);
void __cdecl Actor_PrepareToPushState(actor_s *self, int eState);
int __cdecl Actor_PushState(actor_s *self, ai_state_t eState);
