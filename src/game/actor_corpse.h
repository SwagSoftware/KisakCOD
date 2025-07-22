#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct XAnimTree_s;
struct gentity_s;

XAnimTree_s *__cdecl G_GetActorCorpseIndexAnimTree(unsigned int index);
int __cdecl G_GetActorCorpseIndex(gentity_s *ent);
int __cdecl G_GetFreeActorCorpseIndex(int reuse);
void __cdecl G_RemoveActorCorpses(unsigned int allowedCorpseCount);
void __cdecl G_UpdateActorCorpses();
int __cdecl G_PruneCorpsesSortCmp(int *a, int *b);
void __cdecl G_PruneLoadedCorpses();
void __cdecl ActorCorpse_Free(gentity_s *ent);
void __cdecl Actor_CorpseThink(gentity_s *self);
float __cdecl Actor_SetBodyPlantAngle(
    int iEntNum,
    int iClipMask,
    float *vOrigin,
    const float *vCenter,
    const float *vDir,
    float *pfAngle);
void __cdecl Actor_GetBodyPlantAngles(
    float *iEntNum,
    int iClipMask,
    float *vOrigin,
    double fYaw,
    float *pfPitch,
    float *pfRoll,
    float *pfHeight);
float Actor_Orient_LerpWithLimit(float current, float newValue, float delta, float rate);
void __cdecl Actor_OrientCorpseToGround(gentity_s *self, int bLerp);
void __cdecl Actor_OrientPitchToGround(gentity_s *self, int bLerp);
int __cdecl Actor_BecomeCorpse(gentity_s *self);
XAnimTree_s *__cdecl G_GetActorCorpseAnimTree(gentity_s *ent);

extern float playerEyePos[3];