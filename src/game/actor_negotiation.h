#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall Actor_Negotiation_Start(actor_s *pSelf, ai_state_t ePrevState);
int __fastcall Actor_Negotiation_Think(actor_s *pSelf);
