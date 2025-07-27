#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <universal/q_shared.h>
#include "aim_assist.h"
#include <client/client.h>

void __cdecl TRACK_aim_target();
const dvar_s *__cdecl AimTarget_RegisterDvars(int a1, unsigned __int16 a2, const char *a3);
void __cdecl AimTarget_Init();
void __cdecl AimTarget_ClearTargetList();
int __cdecl AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB);
void __cdecl AimTarget_AddTargetToList(const AimTarget *target);
void __cdecl AimTarget_GetTargetBounds(const gentity_s *targetEnt, float *mins, float *maxs);
float __cdecl AimTarget_GetTargetRadius(const gentity_s *targetEnt);
void __cdecl AimTarget_GetTargetCenter(const gentity_s *targetEnt, float *center);
int __cdecl AimTarget_IsTargetValid(const gentity_s *targetEnt);
int __cdecl AimTarget_IsTargetVisible(const gentity_s *targetEnt, unsigned int visBone);
void __cdecl AimTarget_CreateTarget(const gentity_s *targetEnt, AimTarget *target);
bool __cdecl AimTarget_PlayerInValidState(const playerState_s *ps);
void __cdecl AimTarget_ProcessEntity(gentity_s *ent);
void __cdecl AimTarget_UpdateClientTargets();
void __cdecl AimTarget_GetClientTargetList(AimTarget **targetList, int *targetCount);
int __cdecl AimTarget_GetBestTarget(const float *start, const float *viewDir);
void __cdecl AimTarget_WriteSaveGame(SaveGame *save);
void __cdecl AimTarget_ReadSaveGame(SaveGame *save);
