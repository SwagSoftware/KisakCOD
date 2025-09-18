#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

struct TestEffect
{
    char name[64];
    float pos[3];
    int time;
    int respawnTime;
};

struct ClientViewParams
{
    float x;
    float y;
    float width;
    float height;
};

enum CubemapShot : __int32;

void __cdecl TRACK_cg_view();
void __cdecl CG_PlayTestFx(int localClientNum);
void __cdecl CG_UpdateTestFX(int localClientNum);
void __cdecl CG_FxSetTestPosition();
void __cdecl CG_FxTest();
void __cdecl CG_CalcVrect(int localClientNum);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob);
void __cdecl CG_KickAngles(cg_s *cgameGlob);
float __cdecl CG_GetVerticalBobFactor(
    const playerState_s *predictedPlayerState,
    float cycle,
    float speed,
    float maxAmp);
float __cdecl CG_GetHorizontalBobFactor(
    const playerState_s *predictedPlayerState,
    float cycle,
    float speed,
    float maxAmp);
void __cdecl CG_CalculateView_IdleAngles(cg_s *cgameGlob, float *angles);
void __cdecl CG_CalculateView_BobAngles(const cg_s *cgameGlob, float *angles);
void __cdecl CG_AddGroundTiltToAngles(int localClientNum, float *angles, const cg_s *cgameGlob);
void __cdecl OffsetFirstPersonView(int localClientNum, cg_s *cgameGlob);
float __cdecl CG_GetViewFov(int localClientNum);
void __cdecl CG_CalcFov(int localClientNum);
float __cdecl CG_GetViewZoomScale();
void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob);
void __cdecl CG_CalcVehicleViewValues(int localClientNum);
void __cdecl CalcTurretViewValues(int localClientNum);
void __cdecl CG_CalcLinkedViewValues(int localClientNum);
void __cdecl CG_ApplyViewAnimation(int localClientNum);
int __cdecl PausedClientFreeMove(int localClientNum);
void __cdecl CG_SetDebugOrigin(float *origin);
void __cdecl CG_SetDebugAngles(const float *angles);
void __cdecl CG_UpdateEntInfo(int localClientNum);
const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int localClientNum);
void __cdecl CG_ArchiveViewInfo(cg_s *cgameGlob, MemoryFile *memFile);
void __cdecl GetCeilingHeight(cg_s *cgameGlob);
void __cdecl DumpAnims(int localClientNum);
void __cdecl DrawShellshockBlend(int localClientNum);
void __cdecl CG_UpdateViewOffset(int localClientNum);
void __cdecl UpdateTurretScopeZoom(cg_s *cgameGlob);
void __cdecl CG_UpdateSceneDepthOfField(cg_s *cgameGlob);
void __cdecl CG_CalcViewValues(int localClientNum);
void __cdecl CG_InitView(int localClientNum);
int __cdecl CG_DrawActiveFrame(
    int localClientNum,
    int serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int cubemapSize,
    int animFrametime);
