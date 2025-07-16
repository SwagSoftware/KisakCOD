#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <bgame/bg_public.h>

int __cdecl Actor_SightTrace(actor_s *self, const float *start, const float *end, int passEntNum);
int __cdecl Actor_CanSeePointFrom(
    actor_s *self,
    const float *vStart,
    const float *vEnd,
    double fMaxDistSqrd,
    int ignoreEntityNum,
    int a6);
int __cdecl Actor_CanSeeEnemyViaClaimedNode(actor_s *self);
sentient_s *__cdecl Actor_KnowAboutEnemy(actor_s *self, int hadPath);
int __cdecl Actor_CanShootFrom(actor_s *self, const float *vTarget, const float *vFrom);
int __cdecl compare_sentient_sort(unsigned int *pe1, unsigned int *pe2);
void __cdecl Actor_UpdateLastKnownPos(actor_s *self, sentient_s *other);
void __cdecl Actor_UpdateLastEnemySightPos(actor_s *self);
void __cdecl Actor_UpdateEyeInformation(actor_s *self);
void __cdecl Actor_GetEyePosition(actor_s *self, float *vEyePosOut);
void __cdecl Actor_GetDebugEyePosition(actor_s *self, float *vEyePosOut);
void __cdecl Actor_GetEyeDirection(actor_s *self, float *vEyeDir);
void __cdecl Actor_GetEyeOffset(actor_s *self, float *vEyePosOut);
int __cdecl Actor_GetMuzzleInfo(actor_s *self, float *vOrigin, float *vForward);
bool __cdecl PointInFovAndRange(
    actor_s *self,
    const float *vEyePos,
    const float *vPoint,
    double fovDot,
    double fMaxDistSqrd);
int __cdecl Actor_CanSeePointExInternal(
    actor_s *self,
    const float *vPoint,
    double fovDot,
    double fMaxDistSqrd,
    int ignoreEntityNum,
    float *vViewPos,
    int a7,
    const float *a8);
int __cdecl Actor_CanSeePointEx(
    actor_s *self,
    const float *vPoint,
    double fovDot,
    double fMaxDistSqrd,
    int ignoreEntityNum,
    int a6,
    int a7);
void __cdecl Actor_UpdateVisCache(actor_s *self, const gentity_s *ent, sentient_info_t *pInfo, bool bVisible);
int __cdecl Actor_CanSeeEntityEx(actor_s *self, const gentity_s *ent, double fovDot, double fMaxDistSqrd);
int __cdecl Actor_CanSeeSentientEx(
    actor_s *self,
    sentient_s *sentient,
    double fovDot,
    double fMaxDistSqrd,
    int iMaxLatency,
    int a6,
    int a7);
int __cdecl Actor_CanShootEnemy(actor_s *self);
int __cdecl Actor_CanSeePoint(actor_s *self, const float *vPoint, int a3, int a4);
int __cdecl Actor_CanSeeEntityPoint(actor_s *self, const float *vPoint, const gentity_s *ent, int a4);
int __cdecl Actor_CanSeeEntity(actor_s *self, const gentity_s *ent);
int __cdecl Actor_CanSeeSentient(actor_s *self, sentient_s *sentient, int iMaxLatency);
int __cdecl Actor_CanSeeEnemy(actor_s *self);
int __cdecl Actor_CanSeeEnemyExtended(actor_s *self, int useClaimedNode);
void __cdecl Actor_UpdateSight(actor_s *self);
