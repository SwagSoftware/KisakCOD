#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall Actor_ScriptedAnim_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_ScriptedAnim_Finish(actor_s *self, ai_state_t eNextState);
actor_think_result_t __fastcall Actor_ScriptedAnim_Think(actor_s *self);
int __fastcall Actor_CustomAnim_Start(actor_s *self, ai_state_t ePrevState);
int __fastcall Actor_CustomAnim_Think(actor_s *self);
