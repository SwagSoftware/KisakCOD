#pragma once
#include <bgame/bg_local.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

enum CompassType : __int32
{
    COMPASS_TYPE_PARTIAL = 0x0,
    COMPASS_TYPE_FULL = 0x1,
};

struct CompassVehicle
{
    int entityNum;
    int lastUpdate;
    float lastPos[2];
    float lastYaw;
};

void __fastcall TRACK_cg_compassfriendlies();
void __fastcall CG_ClearCompassPingData();
void __fastcall CG_CompassAddWeaponPingInfo(int localClientNum, centity_s *cent, float *origin, int msec);
void __fastcall CG_CompassApplyPointerRadiusScale(float *radiusScale);
void __fastcall CG_CalcCompassPointerRadius(float *radius, double dist);
void __fastcall CG_CompassUpdateActorInfo(int localClientNum, int entityIndex);
CompassVehicle *__fastcall Compass_GetVehicle(int localClientNum, int entityNum);
void __fastcall CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex);
void __fastcall CG_CompassDrawActors(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __fastcall CG_CompassDrawVehicles(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color,
    unsigned __int8 vehicleCompassType);
