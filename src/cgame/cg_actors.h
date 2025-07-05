#pragma once
#include <bgame/bg_local.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

int __fastcall CG_GetEntityIndex(const centity_s *cent);
int __fastcall CG_GetLocalClientTime(int localClientNum);
void __fastcall CG_Actor_PreControllers(centity_s *cent);
void __fastcall CG_Actor(int localClientNum, centity_s *cent);
void __fastcall CG_ActorSpawner(centity_s *cent, int a2, int a3, const float *a4);
