#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall Actor_SetDesiredLookAngles(ai_orient_t *pOrient, double fPitch, double fYaw);
void __fastcall Actor_SetDesiredBodyAngle(ai_orient_t *pOrient, double fAngle);
void __fastcall Actor_SetDesiredAngles(ai_orient_t *pOrient, double fPitch, double fYaw);
void __fastcall Actor_SetLookAngles(actor_s *self, double fPitch, double fYaw);
void __fastcall Actor_SetBodyAngle(actor_s *self, double fAngle);
void __fastcall Actor_ChangeAngles(actor_s *self, double fPitch, double fYaw);
void __fastcall Actor_UpdateLookAngles(actor_s *self, long double a2);
void __fastcall Actor_UpdateBodyAngle(actor_s *self, long double a2);
void __fastcall Actor_FaceVector(ai_orient_t *pOrient, const float *v);
void __fastcall Actor_FaceMotion(actor_s *self, ai_orient_t *pOrient);
void __fastcall Actor_SetAnglesToLikelyEnemyPath(actor_s *self);
const pathnode_t *__fastcall Actor_GetAnglesToLikelyEnemyPath(actor_s *self);
void __fastcall Actor_FaceLikelyEnemyPath(actor_s *self, ai_orient_t *pOrient);
// local variable allocation has failed, the output may be wrong!
void __fastcall Actor_FaceEnemy(actor_s *self, ai_orient_t *pOrient);
int __fastcall Actor_FaceGoodShootPos(actor_s *self, ai_orient_t *pOrient);
void __fastcall Actor_FaceEnemyOrMotion(actor_s *self, ai_orient_t *pOrient);
void __fastcall Actor_FaceEnemyOrMotionSidestep(actor_s *self, ai_orient_t *pOrient);
void __fastcall Actor_DecideOrientation(actor_s *self);
void __fastcall Actor_SetOrientMode(actor_s *self, ai_orient_mode_t eMode);
void __fastcall Actor_ClearScriptOrient(actor_s *self);
