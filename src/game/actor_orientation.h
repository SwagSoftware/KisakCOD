#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl Actor_SetDesiredLookAngles(ai_orient_t *pOrient, double fPitch, double fYaw);
void __cdecl Actor_SetDesiredBodyAngle(ai_orient_t *pOrient, double fAngle);
void __cdecl Actor_SetDesiredAngles(ai_orient_t *pOrient, double fPitch, double fYaw);
void __cdecl Actor_SetLookAngles(actor_s *self, double fPitch, double fYaw);
void __cdecl Actor_SetBodyAngle(actor_s *self, double fAngle);
void __cdecl Actor_ChangeAngles(actor_s *self, double fPitch, double fYaw);
void __cdecl Actor_UpdateLookAngles(actor_s *self, long double a2);
void __cdecl Actor_UpdateBodyAngle(actor_s *self, long double a2);
void __cdecl Actor_FaceVector(ai_orient_t *pOrient, const float *v);
void __cdecl Actor_FaceMotion(actor_s *self, ai_orient_t *pOrient);
void __cdecl Actor_SetAnglesToLikelyEnemyPath(actor_s *self);
const pathnode_t *__cdecl Actor_GetAnglesToLikelyEnemyPath(actor_s *self);
void __cdecl Actor_FaceLikelyEnemyPath(actor_s *self, ai_orient_t *pOrient);
// local variable allocation has failed, the output may be wrong!
void __cdecl Actor_FaceEnemy(actor_s *self, ai_orient_t *pOrient);
int __cdecl Actor_FaceGoodShootPos(actor_s *self, ai_orient_t *pOrient);
void __cdecl Actor_FaceEnemyOrMotion(actor_s *self, ai_orient_t *pOrient);
void __cdecl Actor_FaceEnemyOrMotionSidestep(actor_s *self, ai_orient_t *pOrient);
void __cdecl Actor_DecideOrientation(actor_s *self);
void __cdecl Actor_SetOrientMode(actor_s *self, ai_orient_mode_t eMode);
void __cdecl Actor_ClearScriptOrient(actor_s *self);
