#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <gfx_d3d/r_material.h>
#include <qcommon/qcommon.h>

bool __cdecl ClampScreenPosToEdges(
    const float *localClientNum,
    Material *point,
    double padLeft,
    double padRight,
    double padTop,
    double padBottom,
    float *resultNormal,
    float *resultDist,
    int a9,
    int a10,
    float *a11,
    float *a12);
void __cdecl CG_VehicleHudRegisterDvars(int a1, unsigned __int16 a2, const char *a3);
int __cdecl WorldDirToScreenPos(int localClientNum, const float *worldDir, float *outScreenPos);
void __cdecl CG_DrawVehicleTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial);
void __cdecl CG_DrawJavelinTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial);
void __cdecl CG_DrawPipOnAStickReticle(int localClientNum, rectDef_s *rect, float *color);
void __cdecl CG_InitVehicleReticle(int localClientNum);
void __cdecl CG_ReticleStartLockOn(int localClientNum, int targetEntNum, int msecDuration);
int __cdecl CG_GetTargetPos(int localClientNum, int targetEntNum, float *outPos);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawBouncingDiamond(int localClientNum, rectDef_s *rect, float *color, const float *a4);
void __cdecl CG_DrawVehicleReticle(int localClientNum, rectDef_s *rect, float *color);
void __cdecl CG_TargetsChanged(int localClientNum, unsigned int num);
