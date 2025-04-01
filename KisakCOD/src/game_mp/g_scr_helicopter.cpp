#include "g_public_mp.h"


//  struct dvar_s const *const vehHelicopterTiltFromControllerAxes 82e8a440     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltFromFwdAndYaw 82e8a444     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterJitterJerkyness 82e8a448     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltSpeed 82e8a44c     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterInvertUpDown 82e8a450     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxYawAccel 82e8a454     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterLookaheadTime 82e8a458     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxSpeedVertical 82e8a45c     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltFromDeceleration 82e8a460     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterRightStickDeadzone 82e8a464     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterSoftCollisions 82e8a468     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterYawOnLeftStick 82e8a46c     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterDecelerationFwd 82e8a470     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxAccelVertical 82e8a474     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxPitch 82e8a478     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltFromAcceleration 82e8a47c     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterStrafeDeadzone 82e8a480     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterHoverSpeedThreshold 82e8a484     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxYawRate 82e8a488     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxAccel 82e8a48c     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterYawAltitudeControls 82e8a490     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxRoll 82e8a494     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterScaleMovement 82e8a498     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterMaxSpeed 82e8a49c     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltFromVelocity 82e8a4a0     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltMomentum 82e8a4a4     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterHeadSwayDontSwayTheTurret 82e8a4a8     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt 82e8a4ac     g_scr_helicopter.obj
//  struct dvar_s const *const vehHelicopterDecelerationSide 82e8a4b0     g_scr_helicopter.obj

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
    ent->s.un1.scale = Scr_GetInt(0).intValue;
}

void(__cdecl *__cdecl Helicopter_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

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

    min.value.max = 3.4028235e38;
    min.value.min = 0.0099999998;
    vehHelicopterMaxSpeed = Dvar_RegisterFloat(
        "vehHelicopterMaxSpeed",
        150.0,
        min,
        0x80u,
        "Maximum horizontal speed of the player helicopter (in MPH)");
    mina.value.max = 3.4028235e38;
    mina.value.min = 0.0099999998;
    vehHelicopterMaxSpeedVertical = Dvar_RegisterFloat(
        "vehHelicopterMaxSpeedVertical",
        65.0,
        mina,
        0x80u,
        "Maximum vertical speed of the player helicopter (in MPH)");
    minb.value.max = 3.4028235e38;
    minb.value.min = 0.0099999998;
    vehHelicopterMaxAccel = Dvar_RegisterFloat(
        "vehHelicopterMaxAccel",
        45.0,
        minb,
        0x80u,
        "Maximum horizontal acceleration of the player helicopter (in MPH per second)");
    minc.value.max = 3.4028235e38;
    minc.value.min = 0.0099999998;
    vehHelicopterMaxAccelVertical = Dvar_RegisterFloat(
        "vehHelicopterMaxAccelVertical",
        30.0,
        minc,
        0x80u,
        "Maximum vertical acceleration of the player helicopter (in MPH per second)");
    mind.value.max = 3.4028235e38;
    mind.value.min = 0.0099999998;
    vehHelicopterMaxYawRate = Dvar_RegisterFloat(
        "vehHelicopterMaxYawRate",
        120.0,
        mind,
        0x80u,
        "Maximum yaw speed of the player helicopter");
    mine.value.max = 3.4028235e38;
    mine.value.min = 0.0099999998;
    vehHelicopterMaxYawAccel = Dvar_RegisterFloat(
        "vehHelicopterMaxYawAccel",
        90.0,
        mine,
        0x80u,
        "Maximum yaw acceleration of the player helicopter");
    minf.value.max = 3.4028235e38;
    minf.value.min = 0.0099999998;
    vehHelicopterMaxPitch = Dvar_RegisterFloat(
        "vehHelicopterMaxPitch",
        35.0,
        minf,
        0x80u,
        "Maximum pitch of the player helicopter");
    ming.value.max = 3.4028235e38;
    ming.value.min = 0.0099999998;
    vehHelicopterMaxRoll = Dvar_RegisterFloat(
        "vehHelicopterMaxRoll",
        35.0,
        ming,
        0x80u,
        "Maximum roll of the player helicopter");
    minh.value.max = 3.4028235e38;
    minh.value.min = 0.0099999998;
    vehHelicopterLookaheadTime = Dvar_RegisterFloat(
        "vehHelicopterLookaheadTime",
        1.0,
        minh,
        0x80u,
        "How far ahead (in seconds) the player helicopter looks ahead, to avoid hard collisions."
        "  (Like driving down the highway, you should keep 2 seconds distance between you and th"
        "e vehicle in front of you)");
    mini.value.max = 3.4028235e38;
    mini.value.min = 0.0099999998;
    vehHelicopterHoverSpeedThreshold = Dvar_RegisterFloat(
        "vehHelicopterHoverSpeedThreshold",
        400.0,
        mini,
        0x80u,
        "The speed below which the player helicopter begins to jitter the tilt, for hovering");
    minj.value.max = 1.0;
    minj.value.min = 0.0099999998;
    vehHelicopterRightStickDeadzone = Dvar_RegisterFloat(
        "vehHelicopterRightStickDeadzone",
        0.30000001,
        minj,
        0x80u,
        "Dead-zone for the axes of the right thumbstick.  This helps to better control the "
        "two axes separately.");
    mink.value.max = 1.0;
    mink.value.min = 0.0099999998;
    vehHelicopterStrafeDeadzone = Dvar_RegisterFloat(
        "vehHelicopterStrafeDeadzone",
        0.30000001,
        mink,
        0x80u,
        "Dead-zone so that you can fly straight forward easily without accidentally strafing (a"
        "nd thus rolling).");
    vehHelicopterScaleMovement = Dvar_RegisterBool(
        "vehHelicopterScaleMovement",
        1,
        0x80u,
        "Scales down the smaller of the left stick axes.");
    vehHelicopterSoftCollisions = Dvar_RegisterBool(
        "vehHelicopterSoftCollisions",
        0,
        0x80u,
        "Player helicopters have soft collisions (slow down before they collide).");
    minl.value.max = 3.4028235e38;
    minl.value.min = 0.0;
    vehHelicopterDecelerationFwd = Dvar_RegisterFloat(
        "vehHelicopterDecelerationFwd",
        0.5,
        minl,
        0x80u,
        "Set the deceleration of the player helicopter (as a fraction of acceleration) in the "
        "direction the chopper is facing.  So 1.0 makes it equal to the acceleration.");
    minm.value.max = 3.4028235e38;
    minm.value.min = 0.0;
    vehHelicopterDecelerationSide = Dvar_RegisterFloat(
        "vehHelicopterDecelerationSide",
        1.0,
        minm,
        0x80u,
        "Set the side-to-side deceleration of the player helicopter (as a fraction of acceler"
        "ation).  So 1.0 makes it equal to the acceleration.");
    vehHelicopterInvertUpDown = Dvar_RegisterBool(
        "vehHelicopterInvertUpDown",
        0,
        0x80u,
        "Invert the altitude control on the player helicopter.");
    minn.value.max = 3.4028235e38;
    minn.value.min = 0.0;
    vehHelicopterYawOnLeftStick = Dvar_RegisterFloat(
        "vehHelicopterYawOnLeftStick",
        5.0,
        minn,
        0x80u,
        "The yaw speed created by the left stick when pushing the stick diagonally (e.g., movin"
        "g forward and strafing slightly).");
    mino.value.max = 3.4028235e38;
    mino.value.min = 0.0099999998;
    vehHelicopterTiltSpeed = Dvar_RegisterFloat(
        "vehHelicopterTiltSpeed",
        1.2,
        mino,
        0x80u,
        "The rate at which the player helicopter's tilt responds");
    minp.value.max = 3.4028235e38;
    minp.value.min = 0.0099999998;
    vehHelicopterTiltFromAcceleration = Dvar_RegisterFloat(
        "vehHelicopterTiltFromAcceleration",
        2.0,
        minp,
        0x80u,
        "The amount of tilt caused by acceleration");
    minq.value.max = 3.4028235e38;
    minq.value.min = 0.0;
    vehHelicopterTiltFromDeceleration = Dvar_RegisterFloat(
        "vehHelicopterTiltFromDeceleration",
        2.0,
        minq,
        0x80u,
        "The amount of tilt caused by deceleration");
    minr.value.max = 3.4028235e38;
    minr.value.min = 0.0;
    vehHelicopterTiltFromVelocity = Dvar_RegisterFloat(
        "vehHelicopterTiltFromVelocity",
        1.0,
        minr,
        0x80u,
        "The amount of tilt caused by the current velocity");
    mins.value.max = 3.4028235e38;
    mins.value.min = 0.0;
    vehHelicopterTiltFromControllerAxes = Dvar_RegisterFloat(
        "vehHelicopterTiltFromControllerAxes",
        0.0,
        mins,
        0x80u,
        "The amount of tilt caused by the desired velocity (i.e., the amount of control"
        "ler stick deflection)");
    mint.value.max = 3.4028235e38;
    mint.value.min = 0.0;
    vehHelicopterTiltFromFwdAndYaw = Dvar_RegisterFloat(
        "vehHelicopterTiltFromFwdAndYaw",
        0.0,
        mint,
        0x80u,
        "The amount of roll caused by yawing while moving forward.");
    minu.value.max = 3.4028235e38;
    minu.value.min = 0.0;
    vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt = Dvar_RegisterFloat(
        "vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt",
        1.0,
        minu,
        0x80u,
        "The forward speed (as a fraction of top speed) at which the tilt due t"
        "o yaw reaches is maximum value.");
    minv.value.max = 3.4028235e38;
    minv.value.min = 0.000099999997;
    vehHelicopterJitterJerkyness = Dvar_RegisterFloat(
        "vehHelicopterJitterJerkyness",
        0.30000001,
        minv,
        0,
        "Specifies how jerky the tilt jitter should be");
    vehHelicopterHeadSwayDontSwayTheTurret = Dvar_RegisterBool(
        "vehHelicopterHeadSwayDontSwayTheTurret",
        1,
        0,
        "If set, the turret will not fire through the crosshairs, but straight ahead"
        " of the vehicle, when the player is not freelooking.");
    minw.value.max = 3.4028235e38;
    minw.value.min = 0.000099999997;
    vehHelicopterTiltMomentum = Dvar_RegisterFloat(
        "vehHelicopterTiltMomentum",
        0.40000001,
        minw,
        0x80u,
        "The amount of rotational momentum the helicopter has with regards to tilting.");
}

void __cdecl G_SpawnHelicopter(gentity_s *ent, gentity_s *owner, const char *vehicleInfoName, char *modelName)
{
    scr_vehicle_s *veh; // [esp+10h] [ebp-10h]
    team_t team; // [esp+14h] [ebp-Ch]

    G_SetModel(ent, modelName);
    SpawnVehicle(ent, vehicleInfoName);
    ent->s.eType = 12;
    veh = ent->scr_vehicle;
    s_vehicleInfos[veh->infoIdx].type = 5;
    veh->targetEnt = 1023;
    if (EntHandle::isDefined(&veh->lookAtEnt))
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 267, 0, "%s", "!veh->lookAtEnt.isDefined()");
    if (EntHandle::isDefined(&veh->idleSndEnt))
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 268, 0, "%s", "!veh->idleSndEnt.isDefined()");
    if (EntHandle::isDefined(&veh->engineSndEnt))
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 269, 0, "%s", "!veh->engineSndEnt.isDefined()");
    VEH_InitEntity(ent, veh, veh->infoIdx);
    VEH_InitVehicle(ent, veh, veh->infoIdx);
    veh->phys.mins[0] = -(float)50.0;
    veh->phys.mins[1] = -(float)50.0;
    veh->phys.mins[2] = -(float)50.0;
    veh->phys.maxs[0] = 50.0;
    veh->phys.maxs[1] = 50.0;
    veh->phys.maxs[2] = 50.0;
    if (!owner->client)
        MyAssertHandler(".\\game_mp\\g_scr_helicopter.cpp", 282, 0, "%s", "owner->client");
    team = owner->client->sess.cs.team;
    if ((unsigned int)team >= TEAM_NUM_TEAMS)
        MyAssertHandler(
            ".\\game_mp\\g_scr_helicopter.cpp",
            285,
            0,
            "team doesn't index (1 << 2)\n\t%i not in [0, %i)",
            team,
            4);
    ent->s.lerp.u.vehicle.teamAndOwnerIndex = team | (4 * (owner->client - level.clients));
    ent->handler = 23;
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
    int wheelIndex; // [esp+24h] [ebp-4h]

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
    pSelf->handler = 23;
    pSelf->nextthink = level.time + 50;
    veh->flags |= 8u;
}

void __cdecl Helicopter_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int damage,
    const float *point,
    int mod,
    const float *dir)
{
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (pAttacker)
    {
        if (pAttacker->s.weapon)
        {
            weapDef = BG_GetWeaponDef(pAttacker->s.weapon);
            if (weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->weapType == WEAPTYPE_GRENADE)
                VEH_JoltBody(pSelf, dir, 1.0, 0.0, 0.0);
        }
    }
}

void __cdecl Helicopter_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int damage,
    int mod,
    int weapon,
    const float *dir)
{
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (pAttacker)
    {
        if (pAttacker->s.weapon)
        {
            weapDef = BG_GetWeaponDef(pAttacker->s.weapon);
            if (weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->weapType == WEAPTYPE_GRENADE)
                VEH_JoltBody(pSelf, dir, 1.0, 0.0, 0.0);
        }
    }
}

void __cdecl Helicopter_Controller(const gentity_s *pSelf, int *partBits)
{
    float gunYaw; // [esp+4h] [ebp-38h]
    float v3; // [esp+Ch] [ebp-30h]
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
    v3 = pSelf->s.lerp.u.turret.gunAngles[1];
    bodyAngles[0] = pSelf->s.lerp.u.turret.gunAngles[0];
    bodyAngles[1] = 0.0;
    bodyAngles[2] = v3;
    if (veh->boneIndex.body >= 0)
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.body, vec3_origin, bodyAngles);
    gunYaw = pSelf->s.lerp.u.vehicle.gunYaw;
    turretAngles[0] = 0.0;
    turretAngles[1] = gunYaw;
    turretAngles[2] = 0.0;
    barrelAngles[0] = pSelf->s.lerp.u.primaryLight.cosHalfFovInner;
    barrelAngles[1] = 0.0;
    barrelAngles[2] = 0.0;
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

