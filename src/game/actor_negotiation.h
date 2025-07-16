#pragma once
#include "actor.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __cdecl Actor_Negotiation_Start(actor_s *pSelf, ai_state_t ePrevState);
actor_think_result_t __cdecl Actor_Negotiation_Think(actor_s *pSelf);
