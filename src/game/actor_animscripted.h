#pragma once
#include "actor.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __cdecl Actor_ScriptedAnim_Start(actor_s *self, ai_state_t ePrevState);
void __cdecl Actor_ScriptedAnim_Finish(actor_s *self, ai_state_t eNextState);
actor_think_result_t __cdecl Actor_ScriptedAnim_Think(actor_s *self);
bool __cdecl Actor_CustomAnim_Start(actor_s *self, ai_state_t ePrevState);
actor_think_result_t __cdecl Actor_CustomAnim_Think(actor_s *self);
