#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

XAnimTree_s *__fastcall G_GetActorCorpseIndexAnimTree(unsigned int index);
int __fastcall G_GetActorCorpseIndex(gentity_s *ent);
int __fastcall G_GetFreeActorCorpseIndex(int reuse);
void __fastcall G_RemoveActorCorpses(unsigned int allowedCorpseCount);
void __fastcall G_UpdateActorCorpses();
int __fastcall G_PruneCorpsesSortCmp(int *a, int *b);
void __fastcall G_PruneLoadedCorpses();
void __fastcall ActorCorpse_Free(gentity_s *ent);
void __fastcall Actor_CorpseThink(gentity_s *self);
float __fastcall Actor_SetBodyPlantAngle(
    int iEntNum,
    int iClipMask,
    float *vOrigin,
    const float *vCenter,
    const float *vDir,
    float *pfAngle);
void __fastcall Actor_GetBodyPlantAngles(
    float *iEntNum,
    int iClipMask,
    float *vOrigin,
    double fYaw,
    float *pfPitch,
    float *pfRoll,
    float *pfHeight,
    float *a8);
float __fastcall Actor_Orient_LerpWithLimit(double current, double newValue, double delta, double rate);
void __fastcall Actor_OrientCorpseToGround(gentity_s *self, int bLerp, int a3, float *a4);
void __fastcall Actor_OrientPitchToGround(gentity_s *self, int bLerp, int a3, float *a4);
int __fastcall Actor_BecomeCorpse(gentity_s *self);
XAnimTree_s *__fastcall G_GetActorCorpseAnimTree(gentity_s *ent);
