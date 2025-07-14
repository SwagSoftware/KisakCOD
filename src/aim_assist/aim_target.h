#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <universal/q_shared.h>
#include "aim_assist.h"
#include <client/client.h>

void __fastcall TRACK_aim_target();
const dvar_s *__fastcall AimTarget_RegisterDvars(int a1, unsigned __int16 a2, const char *a3);
void __fastcall AimTarget_Init();
void __fastcall AimTarget_ClearTargetList();
int __fastcall AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB);
void __fastcall AimTarget_AddTargetToList(const AimTarget *target);
void __fastcall AimTarget_GetTargetBounds(const gentity_s *targetEnt, float *mins, float *maxs);
float __fastcall AimTarget_GetTargetRadius(const gentity_s *targetEnt);
void __fastcall AimTarget_GetTargetCenter(const gentity_s *targetEnt, float *center);
int __fastcall AimTarget_IsTargetValid(const gentity_s *targetEnt);
int __fastcall AimTarget_IsTargetVisible(const gentity_s *targetEnt, unsigned int visBone);
void __fastcall AimTarget_CreateTarget(const gentity_s *targetEnt, AimTarget *target);
bool __fastcall AimTarget_PlayerInValidState(const playerState_s *ps);
void __fastcall AimTarget_ProcessEntity(gentity_s *ent);
void __fastcall AimTarget_UpdateClientTargets();
void __fastcall AimTarget_GetClientTargetList(AimTarget **targetList, int *targetCount);
int __fastcall AimTarget_GetBestTarget(const float *start, const float *viewDir);
void __fastcall AimTarget_WriteSaveGame(SaveGame *save);
void __fastcall AimTarget_ReadSaveGame(SaveGame *save);
