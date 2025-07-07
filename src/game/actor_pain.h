#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __cdecl Actor_InPain(const actor_s *self);
int __cdecl Actor_Pain_Start(actor_s *self, ai_state_t ePrevState);
void __cdecl Actor_Pain_Finish(actor_s *self, ai_state_t eNextState);
int __cdecl Actor_Pain_Think(actor_s *self);
