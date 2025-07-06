#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall Actor_Turret_Touch(actor_s *self, gentity_s *pOther);
int __fastcall Actor_IsUsingTurret(actor_s *self);
int __fastcall Actor_UseTurret(actor_s *self, gentity_s *pTurret);
int __fastcall Actor_Turret_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_DetachTurret(actor_s *self);
void __fastcall Actor_Turret_Finish(actor_s *self, ai_state_t eNextState);
void __fastcall Actor_Turret_Suspend(actor_s *self, ai_state_t eNextState);
void __fastcall Actor_StopUseTurret(actor_s *self);
actor_think_result_t __fastcall Actor_Turret_PostThink(actor_s *self);
int __fastcall Actor_Turret_Think(actor_s *self);
void __fastcall Actor_Turret_Pain(
    actor_s *self,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    const int iMod,
    const float *vDir,
    const hitLocation_t hitLoc);
