#pragma once
#include <bgame/bg_local.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

enum CompassType : __int32;

struct __declspec(align(4)) CompassActor
{
    int lastUpdate;
    float lastPos[2];
    float lastYaw;
    int beginFadeTime;
    bool enemy;
};

struct CompassVehicle
{
    int entityNum;
    int lastUpdate;
    float lastPos[2];
    float lastYaw;
};

void __cdecl TRACK_cg_compassfriendlies();
void __cdecl CG_ClearCompassPingData();
void __cdecl CG_CompassAddWeaponPingInfo(int localClientNum, centity_s *cent, float *origin, int msec);
void __cdecl CG_CompassApplyPointerRadiusScale(float *radiusScale);
void __cdecl CG_CalcCompassPointerRadius(float *radius, double dist);
void __cdecl CG_CompassUpdateActorInfo(int localClientNum, int entityIndex);
CompassVehicle *__cdecl Compass_GetVehicle(int localClientNum, int entityNum);
void __cdecl CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex);
void __cdecl CG_CompassDrawActors(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawVehicles(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color,
    unsigned __int8 vehicleCompassType);
