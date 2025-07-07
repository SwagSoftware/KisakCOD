#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl DebugDrawSuppression(actor_s *self, const float *a2, int a3, int a4, int a5);
int __cdecl Actor_PickNewSuppressantEntry(actor_s *self, sentient_s *pSuppressor);
int __cdecl Actor_NearCoverNode(actor_s *self);
void __cdecl Actor_BulletWhizbyNotify(actor_s *self, sentient_s *pSuppressor);
void __cdecl Actor_AddSuppressionLine(
    actor_s *self,
    sentient_s *pSuppressor,
    const float *vStart,
    const float *vEnd);
void __cdecl Actor_ClearSuppressant(ai_suppression_t *suppressant);
void __cdecl Actor_DecaySuppressionLines(actor_s *self, const float *a2, int a3, int a4, int a5);
void __cdecl Actor_DissociateSuppressor(actor_s *self, sentient_s *pSuppressor);
int __cdecl Actor_IsSuppressedInAnyway(actor_s *self);
bool __cdecl Actor_IsSuppressed(actor_s *self);
int __cdecl Actor_IsMoveSuppressed(actor_s *self);
int __cdecl Actor_IsSuppressionWaiting(actor_s *self);
int __cdecl Actor_GetSuppressionPlanes(actor_s *self, float (*vNormalOut)[2], float *fDistOut);
int __cdecl Actor_GetMoveOnlySuppressionPlanes(actor_s *self, float (*vNormalOut)[2], float *fDistOut);
void __cdecl Actor_ClearAllSuppressionFromEnemySentient(sentient_s *pSuppressor);
