#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

struct ClientViewParams
{
    float x;
    float y;
    float width;
    float height;
};

void __fastcall TRACK_cg_view();
void __fastcall CG_PlayTestFx(int localClientNum);
void __fastcall CG_UpdateTestFX(int localClientNum);
void __fastcall CG_FxSetTestPosition();
void __fastcall CG_FxTest();
void __fastcall CG_CalcVrect(int localClientNum);
// local variable allocation has failed, the output may be wrong!
void __fastcall CG_SmoothCameraZ(cg_s *cgameGlob);
void __fastcall CG_KickAngles(cg_s *cgameGlob);
// local variable allocation has failed, the output may be wrong!
float __fastcall CG_GetVerticalBobFactor(
    const playerState_s *predictedPlayerState,
    double cycle,
    double speed,
    double maxAmp);
// local variable allocation has failed, the output may be wrong!
float __fastcall CG_GetHorizontalBobFactor(
    const playerState_s *predictedPlayerState,
    double cycle,
    double speed,
    double maxAmp);
void __fastcall CG_CalculateView_IdleAngles(cg_s *cgameGlob, float *angles);
void __fastcall CG_CalculateView_BobAngles(const cg_s *cgameGlob, float *angles);
void __fastcall CG_AddGroundTiltToAngles(int localClientNum, float *angles, const cg_s *cgameGlob);
void __fastcall OffsetFirstPersonView(int localClientNum, cg_s *cgameGlob);
float __fastcall CG_GetViewFov(int localClientNum);
void __fastcall CG_CalcFov(int localClientNum);
float __fastcall CG_GetViewZoomScale();
void __fastcall CG_CalcCubemapViewValues(cg_s *cgameGlob);
void __fastcall CG_CalcVehicleViewValues(int localClientNum);
void __fastcall CalcTurretViewValues(int localClientNum);
void __fastcall CG_CalcLinkedViewValues(int localClientNum);
void __fastcall CG_ApplyViewAnimation(int localClientNum);
int __fastcall PausedClientFreeMove(int localClientNum);
void __fastcall CG_SetDebugOrigin(float *origin);
void __fastcall CG_SetDebugAngles(const float *angles);
void __fastcall CG_UpdateEntInfo(int localClientNum);
const ClientViewParams *__fastcall CG_GetLocalClientViewParams(int localClientNum);
void __fastcall CG_ArchiveViewInfo(cg_s *cgameGlob, MemoryFile *memFile);
void __fastcall GetCeilingHeight(cg_s *cgameGlob);
void __fastcall DumpAnims(int localClientNum);
void __fastcall DrawShellshockBlend(int localClientNum);
void __fastcall CG_UpdateViewOffset(int localClientNum);
void __fastcall UpdateTurretScopeZoom(cg_s *cgameGlob);
void __fastcall CG_UpdateSceneDepthOfField(cg_s *cgameGlob);
void __fastcall CG_CalcViewValues(int localClientNum);
void __fastcall CG_InitView(int localClientNum);
int __fastcall CG_DrawActiveFrame(
    int localClientNum,
    int serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int cubemapSize,
    int animFrametime);
