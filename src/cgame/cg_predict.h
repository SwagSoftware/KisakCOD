#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

void __fastcall TRACK_cg_predict();
void __fastcall CG_InterpolatePlayerState(int localClientNum, int grabAngles, int grabStance);
void __fastcall CG_RestorePlayerOrientation(cg_s *cgameGlob);
void __fastcall CG_UpdateFreeMove(cg_s *cgameGlob);
void __fastcall CG_InterpolateGroundTilt(int localClientNum);
void __fastcall CG_PredictPlayerState_Internal(int localClientNum);
void __fastcall CG_PredictPlayerState(int localClientNum);
