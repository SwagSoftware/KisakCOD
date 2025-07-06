#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall Actor_Death_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Death_Cleanup(actor_s *self);
int __fastcall Actor_Death_Think(actor_s *self);
