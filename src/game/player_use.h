#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

// LWSS: unsure this .h is supposed to be here.

void __fastcall Player_UseEntity(gentity_s *playerEnt, gentity_s *useEnt);
int __fastcall Player_ActivateCmd(gentity_s *ent);
void __fastcall Player_ActivateHoldCmd(gentity_s *ent);
void __fastcall Player_UpdateActivate(gentity_s *ent);
int __fastcall compare_use(float *pe1, float *pe2);
int __fastcall Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex);
void __fastcall G_UpdateFriendlyOverlay(gentity_s *ent);
int __fastcall Player_GetItemCursorHint(const gclient_s *client, const gentity_s *traceEnt);
void __fastcall Player_SetTurretDropHint(gentity_s *ent);
void __fastcall Player_UpdateCursorHints(gentity_s *ent);
gentity_s *__fastcall Player_UpdateLookAtEntityTrace(
    trace_t *trace,
    const float *start,
    const float *end,
    int entNum,
    int contentMask,
    unsigned __int8 *priorityMap,
    float *forward);
int __fastcall Player_CheckAlmostStationary(gentity_s *ent, float *dir);
void __fastcall Player_DebugDrawLOS(const float *center, const float *dir, double dist2D, int debugDrawDuration);
void __fastcall Player_BanNodesInFront(gentity_s *ent, double dist, const float *start, const float *dir, int a5);
void __fastcall Player_BlockFriendliesInADS(gentity_s *ent, double dist, const float *start, const float *dir, int a5);
void __fastcall Player_GrenadeThrowBlockFriendlies(
    gentity_s *ent,
    double dist,
    const float *start,
    const float *dir,
    float *a5);
void __fastcall Player_UpdateLookAtEntity(gentity_s *ent);
