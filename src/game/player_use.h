#pragma once
#include <universal/q_shared.h>

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct gentity_s;
struct gclient_s;

struct useList_t
{
    gentity_s *ent;
    float score;
};

void __cdecl Player_UseEntity(gentity_s *playerEnt, gentity_s *useEnt);
int __cdecl Player_ActivateCmd(gentity_s *ent);
void __cdecl Player_ActivateHoldCmd(gentity_s *ent);
void __cdecl Player_UpdateActivate(gentity_s *ent);
int __cdecl compare_use(float *pe1, float *pe2);
int __cdecl Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex);
void __cdecl G_UpdateFriendlyOverlay(gentity_s *ent);
int __cdecl Player_GetItemCursorHint(const gclient_s *client, const gentity_s *traceEnt);
void __cdecl Player_SetTurretDropHint(gentity_s *ent);
void __cdecl Player_UpdateCursorHints(gentity_s *ent);
gentity_s *__cdecl Player_UpdateLookAtEntityTrace(
    trace_t *trace,
    const float *start,
    const float *end,
    int entNum,
    int contentMask,
    unsigned __int8 *priorityMap,
    float *forward);
int __cdecl Player_CheckAlmostStationary(gentity_s *ent, float *dir);
void __cdecl Player_DebugDrawLOS(const float *center, const float *dir, double dist2D, int debugDrawDuration);
void __cdecl Player_BanNodesInFront(gentity_s *ent, float dist, const float *start, const float *dir);
void __cdecl Player_BlockFriendliesInADS(gentity_s *ent, float dist, const float *start, const float *dir);
void __cdecl Player_GrenadeThrowBlockFriendlies(
    gentity_s *ent,
    float dist,
    const float *start,
    const float *dir);
void __cdecl Player_UpdateLookAtEntity(gentity_s *ent);
