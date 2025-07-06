#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall Actor_InitLookAt(actor_s *self);
void __fastcall Actor_SetLookAtAnimNodes(
    actor_s *self,
    unsigned __int16 animStraight,
    unsigned __int16 animLeft,
    unsigned __int16 animRight);
void __fastcall Actor_SetLookAt(actor_s *self, float *vPosition, double fTurnAccel);
float __fastcall Actor_CurrentLookAtAnimYawMax(actor_s *self);
float __fastcall Actor_CurrentLookAtYawMax(actor_s *self);
void __fastcall Actor_SetLookAtYawLimits(actor_s *self, double fAnimYawLimit, double fYawLimit, double fBlendTime);
void __fastcall Actor_StopLookAt(actor_s *self, double fTurnAccel);
void __fastcall Actor_UpdateLookAt(actor_s *self);
