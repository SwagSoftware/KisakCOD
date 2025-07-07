#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __cdecl Actor_ScriptedAnim_Start(actor_s *self, ai_state_t ePrevState);
void __cdecl Actor_ScriptedAnim_Finish(actor_s *self, ai_state_t eNextState);
actor_think_result_t __cdecl Actor_ScriptedAnim_Think(actor_s *self);
int __cdecl Actor_CustomAnim_Start(actor_s *self, ai_state_t ePrevState);
int __cdecl Actor_CustomAnim_Think(actor_s *self);
