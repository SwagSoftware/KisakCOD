#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

void __cdecl TRACK_cg_predict();
void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles, int grabStance);
void __cdecl CG_RestorePlayerOrientation(cg_s *cgameGlob);
void __cdecl CG_UpdateFreeMove(cg_s *cgameGlob);
void __cdecl CG_InterpolateGroundTilt(int localClientNum);
void __cdecl CG_PredictPlayerState_Internal(int localClientNum);
void __cdecl CG_PredictPlayerState(int localClientNum);
