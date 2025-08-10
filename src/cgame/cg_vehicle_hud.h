#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <gfx_d3d/r_material.h>
#include <qcommon/qcommon.h>

enum Clip_t : __int32
{
    CLIP_NONE = 0x0,
    CLIP_TOP = 0x1,
    CLIP_BOTTOM = 0x2,
    CLIP_RIGHT = 0x3,
    CLIP_LEFT = 0x4,
};

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
void __cdecl CG_VehicleHudRegisterDvars();
int __cdecl WorldDirToScreenPos(int localClientNum, const float *worldDir, float *outScreenPos);
void __cdecl CG_DrawVehicleTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial);
void __cdecl CG_DrawJavelinTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial);
void __cdecl CG_DrawPipOnAStickReticle(int localClientNum, rectDef_s *rect, float *color);
void __cdecl CG_InitVehicleReticle(int localClientNum);
void __cdecl CG_ReticleStartLockOn(int localClientNum, int targetEntNum, int msecDuration);
int __cdecl CG_GetTargetPos(int localClientNum, int targetEntNum, float *outPos);
void __cdecl CG_DrawBouncingDiamond(int localClientNum, rectDef_s *rect, float *color);
void __cdecl CG_DrawVehicleReticle(int localClientNum, rectDef_s *rect, float *color);
void __cdecl CG_TargetsChanged(int localClientNum, unsigned int num);
