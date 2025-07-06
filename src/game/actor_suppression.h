#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall DebugDrawSuppression(actor_s *self, const float *a2, int a3, int a4, int a5);
int __fastcall Actor_PickNewSuppressantEntry(actor_s *self, sentient_s *pSuppressor);
int __fastcall Actor_NearCoverNode(actor_s *self);
void __fastcall Actor_BulletWhizbyNotify(actor_s *self, sentient_s *pSuppressor);
void __fastcall Actor_AddSuppressionLine(
    actor_s *self,
    sentient_s *pSuppressor,
    const float *vStart,
    const float *vEnd);
void __fastcall Actor_ClearSuppressant(ai_suppression_t *suppressant);
void __fastcall Actor_DecaySuppressionLines(actor_s *self, const float *a2, int a3, int a4, int a5);
void __fastcall Actor_DissociateSuppressor(actor_s *self, sentient_s *pSuppressor);
int __fastcall Actor_IsSuppressedInAnyway(actor_s *self);
bool __fastcall Actor_IsSuppressed(actor_s *self);
int __fastcall Actor_IsMoveSuppressed(actor_s *self);
int __fastcall Actor_IsSuppressionWaiting(actor_s *self);
int __fastcall Actor_GetSuppressionPlanes(actor_s *self, float (*vNormalOut)[2], float *fDistOut);
int __fastcall Actor_GetMoveOnlySuppressionPlanes(actor_s *self, float (*vNormalOut)[2], float *fDistOut);
void __fastcall Actor_ClearAllSuppressionFromEnemySentient(sentient_s *pSuppressor);
