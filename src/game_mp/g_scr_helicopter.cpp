#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"
#include <script/scr_vm.h>
#include <xanim/dobj.h>
#include "g_utils_mp.h"

const dvar_t *vehHelicopterTiltFromControllerAxes;
const dvar_t *vehHelicopterTiltFromFwdAndYaw;
const dvar_t *vehHelicopterJitterJerkyness;
const dvar_t *vehHelicopterTiltSpeed;
const dvar_t *vehHelicopterInvertUpDown;
const dvar_t *vehHelicopterMaxYawAccel;
const dvar_t *vehHelicopterLookaheadTime;
const dvar_t *vehHelicopterMaxSpeedVertical;
const dvar_t *vehHelicopterTiltFromDeceleration;
const dvar_t *vehHelicopterRightStickDeadzone;
const dvar_t *vehHelicopterSoftCollisions;
const dvar_t *vehHelicopterYawOnLeftStick;
const dvar_t *vehHelicopterDecelerationFwd;
const dvar_t *vehHelicopterMaxAccelVertical;
const dvar_t *vehHelicopterMaxPitch;
const dvar_t *vehHelicopterTiltFromAcceleration;
const dvar_t *vehHelicopterStrafeDeadzone;
const dvar_t *vehHelicopterHoverSpeedThreshold;
const dvar_t *vehHelicopterMaxYawRate;
const dvar_t *vehHelicopterMaxAccel;
const dvar_t *vehHelicopterYawAltitudeControls;
const dvar_t *vehHelicopterMaxRoll;
const dvar_t *vehHelicopterScaleMovement;
const dvar_t *vehHelicopterMaxSpeed;
const dvar_t *vehHelicopterTiltFromVelocity;
const dvar_t *vehHelicopterTiltMomentum;
const dvar_t *vehHelicopterHeadSwayDontSwayTheTurret;
const dvar_t *vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
const dvar_t *vehHelicopterDecelerationSide;

const BuiltinMethodDef s_methods[25] =
{
  { "freehelicopter", &CMD_Heli_FreeHelicopter, 0 },
  { "setspeed", &CMD_VEH_SetSpeed, 0 },
  { "getspeed", &CMD_VEH_GetSpeed, 0 },
  { "getspeedmph", &CMD_VEH_GetSpeedMPH, 0 },
  { "resumespeed", &CMD_VEH_ResumeSpeed, 0 },
  { "setyawspeed", &CMD_VEH_SetYawSpeed, 0 },
  { "setmaxpitchroll", &CMD_VEH_SetMaxPitchRoll, 0 },
  { "setturningability", &CMD_VEH_SetTurningAbility, 0 },
  { "setairresistance", &CMD_VEH_SetAirResitance, 0 },
  { "sethoverparams", &CMD_VEH_SetHoverParams, 0 },
  { "setneargoalnotifydist", &CMD_VEH_NearGoalNotifyDist, 0 },
  { "setvehgoalpos", &CMD_VEH_SetGoalPos, 0 },
  { "setgoalyaw", &CMD_VEH_SetGoalYaw, 0 },
  { "cleargoalyaw", &CMD_VEH_ClearGoalYaw, 0 },
  { "settargetyaw", &CMD_VEH_SetTargetYaw, 0 },
  { "cleartargetyaw", &CMD_VEH_ClearTargetYaw, 0 },
  { "setlookatent", &CMD_VEH_SetLookAtEnt, 0 },
  { "clearlookatent", &CMD_VEH_ClearLookAtEnt, 0 },
  { "setvehweapon", &CMD_VEH_SetWeapon, 0 },
  { "fireweapon", &CMD_VEH_FireWeapon, 0 },
  { "setturrettargetvec", &CMD_VEH_SetTurretTargetVec, 0 },
  { "setturrettargetent", &CMD_VEH_SetTurretTargetEnt, 0 },
  { "clearturrettarget", &CMD_VEH_ClearTurretTargetEnt, 0 },
  { "setvehicleteam", &CMD_VEH_SetVehicleTeam, 0 },
  { "setdamagestage", &CMD_Heli_SetDamageStage, 0 }
}; // idb

void __cdecl CMD_Heli_FreeHelicopter(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    G_VehFreeEntity(ent);
}

void __cdecl CMD_Heli_SetDamageStage(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    ent->s.un1.scale = Scr_GetInt(0);
}

void(__cdecl *__cdecl Helicopter_GetMethod(const char **pName))(scr_entref_t)
{
    uint32_t i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0x19; ++i)
    {
        if (!strcmp(*pName, s_methods[i].actionString))
        {
            *pName = s_methods[i].actionString;
            return s_methods[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl Helicopter_RegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]
    DvarLimits mine; // [esp+4h] [ebp-10h]
    DvarLimits minf; // [esp+4h] [ebp-10h]
    DvarLimits ming; // [esp+4h] [ebp-10h]
    DvarLimits minh; // [esp+4h] [ebp-10h]
    DvarLimits mini; // [esp+4h] [ebp-10h]
    DvarLimits minj; // [esp+4h] [ebp-10h]
    DvarLimits mink; // [esp+4h] [ebp-10h]
    DvarLimits minl; // [esp+4h] [ebp-10h]
    DvarLimits minm; // [esp+4h] [ebp-10h]
    DvarLimits minn; // [esp+4h] [ebp-10h]
    DvarLimits mino; // [esp+4h] [ebp-10h]
    DvarLimits minp; // [esp+4h] [ebp-10h]
    DvarLimits minq; // [esp+4h] [ebp-10h]
    DvarLimits minr; // [esp+4h] [ebp-10h]
    DvarLimits mins; // [esp+4h] [ebp-10h]
    DvarLimits mint; // [esp+4h] [ebp-10h]
    DvarLimits minu; // [esp+4h] [ebp-10h]
    DvarLimits minv; // [esp+4h] [ebp-10h]
    DvarLimits minw; // [esp+4h] [ebp-10h]

    min.value.max = FLT_MAX;
    min.value.min = 0.0099999998f;
    vehHelicopterMaxSpeed = Dvar_RegisterFloat(
        "vehHelicopterMaxSpeed",
        150.0f,
        min,
        DVAR_CHEAT,
        "Maximum horizontal speed of the player helicopter (in MPH)");
    mina.value.max = FLT_MAX;
    mina.value.min = 0.0099999998f;
    vehHelicopterMaxSpeedVertical = Dvar_RegisterFloat(
        "vehHelicopterMaxSpeedVertical",
        65.0f,
        mina,
        DVAR_CHEAT,
        "Maximum vertical speed of the player helicopter (in MPH)");
    minb.value.max = FLT_MAX;
    minb.value.min = 0.0099999998f;
    vehHelicopterMaxAccel = Dvar_RegisterFloat(
        "vehHelicopterMaxAccel",
        45.0f,
        minb,
        DVAR_CHEAT,
        "Maximum horizontal acceleration of the player helicopter (in MPH per second)");
    minc.value.max = FLT_MAX;
    minc.value.min = 0.0099999998f;
    vehHelicopterMaxAccelVertical = Dvar_RegisterFloat(
        "vehHelicopterMaxAccelVertical",
        30.0f,
        minc,
        DVAR_CHEAT,
        "Maximum vertical acceleration of the player helicopter (in MPH per second)");
    mind.value.max = FLT_MAX;
    mind.value.min = 0.0099999998f;
    vehHelicopterMaxYawRate = Dvar_RegisterFloat(
        "vehHelicopterMaxYawRate",
        120.0f,
        mind,
        DVAR_CHEAT,
        "Maximum yaw speed of the player helicopter");
    mine.value.max = FLT_MAX;
    mine.value.min = 0.0099999998f;
    vehHelicopterMaxYawAccel = Dvar_RegisterFloat(
        "vehHelicopterMaxYawAccel",
        90.0f,
        mine,
        DVAR_CHEAT,
        "Maximum yaw acceleration of the player helicopter");
    minf.value.max = FLT_MAX;
    minf.value.min = 0.0099999998f;
    vehHelicopterMaxPitch = Dvar_RegisterFloat(
        "vehHelicopterMaxPitch",
        35.0f,
        minf,
        DVAR_CHEAT,
        "Maximum pitch of the player helicopter");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0099999998f;
    vehHelicopterMaxRoll = Dvar_RegisterFloat(
        "vehHelicopterMaxRoll",
        35.0f,
        ming,
        DVAR_CHEAT,
        "Maximum roll of the player helicopter");
    minh.value.max = FLT_MAX;
    minh.value.min = 0.0099999998f;
    vehHelicopterLookaheadTime = Dvar_RegisterFloat(
        "vehHelicopterLookaheadTime",
        1.0f,
        minh,
        DVAR_CHEAT,
        "How far ahead (in seconds) the player helicopter looks ahead, to avoid hard collisions."
        "  (Like driving down the highway, you should keep 2 seconds distance between you and th"
        "e vehicle in front of you)");
    mini.value.max = FLT_MAX;
    mini.value.min = 0.0099999998f;
    vehHelicopterHoverSpeedThreshold = Dvar_RegisterFloat(
        "vehHelicopterHoverSpeedThreshold",
        400.0f,
        mini,
        DVAR_CHEAT,
        "The speed below which the player helicopter begins to jitter the tilt, for hovering");
    minj.value.max = 1.0f;
    minj.value.min = 0.0099999998f;
    vehHelicopterRightStickDeadzone = Dvar_RegisterFloat(
        "vehHelicopterRightStickDeadzone",
        0.30000001f,
        minj,
        DVAR_CHEAT,
        "Dead-zone for the axes of the right thumbstick.  This helps to better control the "
        "two axes separately.");
    mink.value.max = 1.0f;
    mink.value.min = 0.0099999998f;
    vehHelicopterStrafeDeadzone = Dvar_RegisterFloat(
        "vehHelicopterStrafeDeadzone",
        0.30000001f,
        mink,
        DVAR_CHEAT,
        "Dead-zone so that you can fly straight forward easily without accidentally strafing (a"
        "nd thus rolling).");
    vehHelicopterScaleMovement = Dvar_RegisterBool(
        "vehHelicopterScaleMovement",
        1,
        DVAR_CHEAT,
        "Scales down the smaller of the left stick axes.");
    vehHelicopterSoftCollisions = Dvar_RegisterBool(
        "vehHelicopterSoftCollisions",
        0,
        DVAR_CHEAT,
        "Player helicopters have soft collisions (slow down before they collide).");
    minl.value.max = FLT_MAX;
    minl.value.min = 0.0f;
    vehHelicopterDecelerationFwd = Dvar_RegisterFloat(
        "vehHelicopterDecelerationFwd",
        0.5f,
        minl,
        DVAR_CHEAT,
        "Set the deceleration of the player helicopter (as a fraction of acceleration) in the "
        "direction the chopper is facing.  So 1.0 makes it equal to the acceleration.");
    minm.value.max = FLT_MAX;
    minm.value.min = 0.0f;
    vehHelicopterDecelerationSide = Dvar_RegisterFloat(
        "vehHelicopterDecelerationSide",
        1.0f,
        minm,
        DVAR_CHEAT,
        "Set the side-to-side deceleration of the player helicopter (as a fraction of acceler"
        "ation).  So 1.0 makes it equal to the acceleration.");
    vehHelicopterInvertUpDown = Dvar_RegisterBool(
        "vehHelicopterInvertUpDown",
        0,
        DVAR_CHEAT,
        "Invert the altitude control on the player helicopter.");
    minn.value.max = FLT_MAX;
    minn.value.min = 0.0f;
    vehHelicopterYawOnLeftStick = Dvar_RegisterFloat(
        "vehHelicopterYawOnLeftStick",
        5.0f,
        minn,
        DVAR_CHEAT,
        "The yaw speed created by the left stick when pushing the stick diagonally (e.g., movin"
        "g forward and strafing slightly).");
    mino.value.max = FLT_MAX;
    mino.value.min = 0.0099999998f;
    vehHelicopterTiltSpeed = Dvar_RegisterFloat(
        "vehHelicopterTiltSpeed",
        1.2f,
        mino,
        DVAR_CHEAT,
        "The rate at which the player helicopter's tilt responds");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.0099999998f;
    vehHelicopterTiltFromAcceleration = Dvar_RegisterFloat(
        "vehHelicopterTiltFromAcceleration",
        2.0f,
        minp,
        DVAR_CHEAT,
        "The amount of tilt caused by acceleration");
    minq.value.max = FLT_MAX;
    minq.value.min = 0.0f;
    vehHelicopterTiltFromDeceleration = Dvar_RegisterFloat(
        "vehHelicopterTiltFromDeceleration",
        2.0f,
        minq,
        DVAR_CHEAT,
        "The amount of tilt caused by deceleration");
    minr.value.max = FLT_MAX;
    minr.value.min = 0.0f;
    vehHelicopterTiltFromVelocity = Dvar_RegisterFloat(
        "vehHelicopterTiltFromVelocity",
        1.0f,
        minr,
        DVAR_CHEAT,
        "The amount of tilt caused by the current velocity");
    mins.value.max = FLT_MAX;
    mins.value.min = 0.0f;
    vehHelicopterTiltFromControllerAxes = Dvar_RegisterFloat(
        "vehHelicopterTiltFromControllerAxes",
        0.0f,
        mins,
        DVAR_CHEAT,
        "The amount of tilt caused by the desired velocity (i.e., the amount of control"
        "ler stick deflection)");
    mint.value.max = FLT_MAX;
    mint.value.min = 0.0f;
    vehHelicopterTiltFromFwdAndYaw = Dvar_RegisterFloat(
        "vehHelicopterTiltFromFwdAndYaw",
        0.0f,
        mint,
        DVAR_CHEAT,
        "The amount of roll caused by yawing while moving forward.");
    minu.value.max = FLT_MAX;
    minu.value.min = 0.0f;
    vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt = Dvar_RegisterFloat(
        "vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt",
        1.0f,
        minu,
        DVAR_CHEAT,
        "The forward speed (as a fraction of top speed) at which the tilt due t"
        "o yaw reaches is maximum value.");
    minv.value.max = FLT_MAX;
    minv.value.min = 0.000099999997f;
    vehHelicopterJitterJerkyness = Dvar_RegisterFloat(
        "vehHelicopterJitterJerkyness",
        0.30000001f,
        minv,
        DVAR_NOFLAG,
        "Specifies how jerky the tilt jitter should be");
    vehHelicopterHeadSwayDontSwayTheTurret = Dvar_RegisterBool(
        "vehHelicopterHeadSwayDontSwayTheTurret",
        1,
        DVAR_NOFLAG,
        "If set, the turret will not fire through the crosshairs, but straight ahead"
        " of the vehicle, when the player is not freelooking.");
    minw.value.max = FLT_MAX;
    minw.value.min = 0.000099999997f;
    vehHelicopterTiltMomentum = Dvar_RegisterFloat(
        "vehHelicopterTiltMomentum",
        0.40000001f,
        minw,
        DVAR_CHEAT,
        "The amount of rotational momentum the helicopter has with regards to tilting.");
}

void __cdecl G_SpawnHelicopter(gentity_s *ent, gentity_s *owner, const char *vehicleInfoName, char *modelName)
{
    scr_vehicle_s *veh; // [esp+10h] [ebp-10h]
    team_t team; // [esp+14h] [ebp-Ch]

    G_SetModel(ent, modelName);
    SpawnVehicle(ent, vehicleInfoName);
    ent->s.eType = ET_HELICOPTER;
    veh = ent->scr_vehicle;
    s_vehicleInfos[veh->infoIdx].type = 5;
    veh->targetEnt = ENTITYNUM_NONE;

    iassert(!veh->lookAtEnt.isDefined());
    iassert(!veh->idleSndEnt.isDefined());
    iassert(!veh->engineSndEnt.isDefined());

    VEH_InitEntity(ent, veh, veh->infoIdx);
    VEH_InitVehicle(ent, veh, veh->infoIdx);
    veh->phys.mins[0] = -50.0f;
    veh->phys.mins[1] = -50.0f;
    veh->phys.mins[2] = -50.0f;
    veh->phys.maxs[0] = 50.0f;
    veh->phys.maxs[1] = 50.0f;
    veh->phys.maxs[2] = 50.0f;
    if (!owner->client)
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 282, 0, "%s", "owner->client");
    team = owner->client->sess.cs.team;
    if ((uint32_t)team >= TEAM_NUM_TEAMS)
        MyAssertHandler(
            ".\\game_mp\\g_scr_helicopter.cpp",
            285,
            0,
            "team doesn't index (1 << 2)\n\t%i not in [0, %i)",
            team,
            4);
    ent->s.lerp.u.vehicle.teamAndOwnerIndex = team | (4 * (owner->client - level.clients));
    ent->handler = ENT_HANDLER_HELICOPTER;
    ent->nextthink = level.time + 50;
    ent->r.svFlags |= 0x10u;
    Heli_InitFirstThink(ent);
}

void __cdecl Heli_InitFirstThink(gentity_s *pSelf)
{
    float *prevAngles; // [esp+0h] [ebp-28h]
    float *angles; // [esp+4h] [ebp-24h]
    float *prevOrigin; // [esp+8h] [ebp-20h]
    float pos[3]; // [esp+Ch] [ebp-1Ch] BYREF
    vehicle_physic_t *phys; // [esp+18h] [ebp-10h]
    vehicle_info_t *info; // [esp+1Ch] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+20h] [ebp-8h]
    int32_t wheelIndex; // [esp+24h] [ebp-4h]

    veh = pSelf->scr_vehicle;
    phys = &veh->phys;
    info = &s_vehicleInfos[veh->infoIdx];
    for (wheelIndex = 0; wheelIndex < 4; ++wheelIndex)
    {
        if (veh->boneIndex.wheel[wheelIndex] >= 0)
        {
            G_DObjGetWorldBoneIndexPos(pSelf, veh->boneIndex.wheel[wheelIndex], pos);
            phys->wheelZPos[wheelIndex] = pos[2];
        }
    }
    VEH_SetPosition(pSelf, phys->origin, phys->angles);
    prevOrigin = phys->prevOrigin;
    phys->prevOrigin[0] = phys->origin[0];
    prevOrigin[1] = phys->origin[1];
    prevOrigin[2] = phys->origin[2];
    prevAngles = phys->prevAngles;
    angles = phys->angles;
    phys->prevAngles[0] = phys->angles[0];
    prevAngles[1] = angles[1];
    prevAngles[2] = angles[2];
    pSelf->health = 99999;
    pSelf->handler = ENT_HANDLER_HELICOPTER;
    pSelf->nextthink = level.time + 50;
    veh->flags |= 8u;
}

void __cdecl Helicopter_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int32_t damage,
    const float *point,
    const int32_t mod,
    const float *dir,
    const hitLocation_t hitLoc,
    const int32_t weaponIdx)
{
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (pAttacker)
    {
        if (pAttacker->s.weapon)
        {
            weapDef = BG_GetWeaponDef(pAttacker->s.weapon);
            if (weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->weapType == WEAPTYPE_GRENADE)
                VEH_JoltBody(pSelf, dir, 1.0f, 0.0f, 0.0f);
        }
    }
}

void __cdecl Helicopter_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    const int32_t damage,
    const int32_t mod,
    const int32_t weapon,
    const float *dir,
    const hitLocation_t hitLoc,
    int32_t psTimeOffset)
{
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (pAttacker)
    {
        if (pAttacker->s.weapon)
        {
            weapDef = BG_GetWeaponDef(pAttacker->s.weapon);
            if (weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->weapType == WEAPTYPE_GRENADE)
                VEH_JoltBody(pSelf, dir, 1.0f, 0.0f, 0.0f);
        }
    }
}

void __cdecl Helicopter_Controller(const gentity_s *pSelf, int32_t *partBits)
{
    //float gunYaw; // [esp+4h] [ebp-38h]
    //float v3; // [esp+Ch] [ebp-30h]
    float barrelAngles[3]; // [esp+10h] [ebp-2Ch] BYREF
    DObj_s *obj; // [esp+1Ch] [ebp-20h]
    scr_vehicle_s *veh; // [esp+20h] [ebp-1Ch]
    float bodyAngles[3]; // [esp+24h] [ebp-18h] BYREF
    float turretAngles[3]; // [esp+30h] [ebp-Ch] BYREF

    if (!pSelf)
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 346, 0, "%s", "pSelf");
    if (!pSelf->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 347, 0, "%s", "pSelf->scr_vehicle");
    veh = pSelf->scr_vehicle;
    obj = Com_GetServerDObj(pSelf->s.number);
    if (!obj)
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 351, 0, "%s", "obj");
    //v3 = pSelf->s.lerp.u.turret.gunAngles[1];
    bodyAngles[0] = pSelf->s.lerp.u.vehicle.bodyPitch;
    bodyAngles[1] = 0.0f;
    bodyAngles[2] = pSelf->s.lerp.u.vehicle.bodyRoll;
    if (veh->boneIndex.body >= 0)
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.body, vec3_origin, bodyAngles);
    //gunYaw = pSelf->s.lerp.u.vehicle.gunYaw;
    turretAngles[0] = 0.0f;
    turretAngles[1] = pSelf->s.lerp.u.vehicle.gunYaw;
    turretAngles[2] = 0.0f;
    barrelAngles[0] = pSelf->s.lerp.u.vehicle.gunPitch;
    barrelAngles[1] = 0.0f;
    barrelAngles[2] = 0.0f;
    if (veh->boneIndex.turret >= 0)
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.turret, vec3_origin, turretAngles);
    if (veh->boneIndex.barrel >= 0)
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.barrel, vec3_origin, barrelAngles);
}

void __cdecl Helicopter_Think(gentity_s *ent)
{
    Scr_Vehicle_Think(ent);
    ent->nextthink = level.time + 50;
}

