#pragma once
#include "actor.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __cdecl Actor_CheckCoverLeave(actor_s *self, const float *exitPos);
int __cdecl Actor_CheckCoverApproach(actor_s *self);
void __cdecl Actor_CoverApproachNotify(actor_s *self);
bool __cdecl Actor_CoverArrival_Start(actor_s *self, ai_state_t ePrevState);
bool __cdecl Actor_CoverArrival_Resume(actor_s *self, ai_state_t ePrevState);
actor_think_result_t __cdecl Actor_CoverArrival_Think(actor_s *self);
