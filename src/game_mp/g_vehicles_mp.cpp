#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

// This file contains MP-specific Vehicle functions.
//
// Note: There are a handful of _0 suffix'd functions
// These are functions that MP had 2 copies of, with the same name, and used separately.
// --

#include <universal/q_shared.h>
#include "g_public_mp.h"
#include <gfx_d3d/r_scene.h>
#include <EffectsCore/fx_system.h>
#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include "g_utils_mp.h"
#include <server/sv_world.h>
#include <script/scr_vm.h>
#include <server/sv_game.h>
#include <game/bullet.h>

const dvar_t *heli_barrelMaxVelocity;

vehicleEffects vehEffects[1][8];

scr_vehicle_s s_vehicles[8];

const dvar_t *vehDebugServer;
const dvar_t *vehTextureScrollScale;
const dvar_t *vehTestHorsepower;
const dvar_t *vehTestWeight;
const dvar_t *vehTestMaxMPH;

extern uint16_t *s_wheelTags[4];
extern uint16_t *s_flashTags[5];
extern short s_numVehicleInfos;
extern cspField_t s_vehicleFields[33];

void __cdecl CG_VehRegisterDvars();

clientInfo_t *__cdecl ClientInfoForLocalClient(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    
    bcassert(cgameGlob->predictedPlayerState.clientNum, MAX_CLIENTS);

    return &cgameGlob->bgs.clientinfo[cgameGlob->predictedPlayerState.clientNum];
}

vehicleEffects *__cdecl VehicleGetFxInfo(int32_t localClientNum, int32_t entityNum)
{
    vehicleEffects *v3; // edx
    int32_t veh; // [esp+4h] [ebp-8h]
    int32_t veha; // [esp+4h] [ebp-8h]
    int32_t vehb; // [esp+4h] [ebp-8h]
    int32_t oldest; // [esp+8h] [ebp-4h]

    for (veh = 0; veh < 8 && vehEffects[localClientNum][veh].active; ++veh)
    {
        if (vehEffects[localClientNum][veh].entityNum == entityNum)
        {
            vehEffects[localClientNum][veh].lastAccessed = Sys_Milliseconds();
            return &vehEffects[localClientNum][veh];
        }
    }
    for (veha = 0; veha < 8 && vehEffects[localClientNum][veha].active; ++veha)
        ;
    if (veha >= 8)
    {
        oldest = 0;
        for (vehb = 1; vehb < 8; ++vehb)
        {
            if (vehEffects[localClientNum][vehb].lastAccessed < vehEffects[localClientNum][oldest].lastAccessed)
                oldest = vehb;
        }
        veha = oldest;
    }
    v3 = &vehEffects[localClientNum][veha];
    *(_DWORD*)&v3->active = 0;
    v3->lastAccessed = 0;
    v3->entityNum = 0;
    v3->nextDustFx = 0;
    v3->nextSmokeFx = 0;
    *(_DWORD*)&v3->soundPlaying = 0;
    v3->barrelVelocity = 0.0;
    v3->barrelPos = 0.0;
    v3->lastBarrelUpdateTime = 0;
    *(_DWORD*)&v3->tag_engine_left = 0;
    v3->active = 1;
    vehEffects[localClientNum][veha].lastAccessed = Sys_Milliseconds();
    vehEffects[localClientNum][veha].entityNum = entityNum;
    return &vehEffects[localClientNum][veha];
}

void __cdecl Veh_IncTurretBarrelRoll(int32_t localClientNum, int32_t entityNum, float rotation)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float v5; // [esp+8h] [ebp-Ch]
    float value; // [esp+Ch] [ebp-8h]
    vehicleEffects *vehFx; // [esp+10h] [ebp-4h]

    vehFx = VehicleGetFxInfo(localClientNum, entityNum);
    v5 = rotation + vehFx->barrelVelocity;
    value = heli_barrelMaxVelocity->current.value;
    v4 = value - v5;
    if (v4 < 0.0)
        v3 = value;
    else
        v3 = rotation + vehFx->barrelVelocity;
    vehFx->barrelVelocity = v3;
}

uint16_t __cdecl CompressUnit(float unit)
{
    if (unit < 0.0 || unit > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../cgame_mp/cg_pose_mp.h",
            102,
            0,
            "%s\n\t(unit) = %g",
            "(unit >= 0.0f && unit <= 1.0f)",
            unit);
    return (int)(unit * 65535.0 + 0.5);
}

double __cdecl GetSpeed(int32_t localClientNum, centity_s *cent)
{
    int32_t serverTimeDelta; // [esp+Ch] [ebp-1Ch]
    float posDelta[3]; // [esp+10h] [ebp-18h] BYREF
    float len; // [esp+1Ch] [ebp-Ch]
    LerpEntityState *p_currentState; // [esp+20h] [ebp-8h]
    entityState_s *ns; // [esp+24h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    p_currentState = &cent->currentState;
    ns = &cent->nextState;
    serverTimeDelta = cgameGlob->nextSnap->serverTime - cgameGlob->snap->serverTime;
    Vec3Sub(cent->nextState.lerp.pos.trBase, cent->currentState.pos.trBase, posDelta);
    len = Vec3Length(posDelta);
    if (serverTimeDelta <= 0.0)
        return 0.0;
    return (len / serverTimeDelta);
}

void __cdecl G_VehRegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]

    vehDebugServer = Dvar_RegisterBool("vehDebugServer", 0, DVAR_CHEAT, "Turn on debug information for vehicles");
    min.value.max = FLT_MAX;
    min.value.min = 0.0;
    vehTextureScrollScale = Dvar_RegisterFloat(
        "vehTextureScrollScale",
        0.0,
        min,
        DVAR_CHEAT,
        "Scale vehicle texture scroll scale by this amount (debug only)");
    mina.value.max = FLT_MAX;
    mina.value.min = 0.0;
    vehTestHorsepower = Dvar_RegisterFloat("vehTestHorsepower", 200.0, mina, DVAR_CHEAT, "");
    minb.value.max = FLT_MAX;
    minb.value.min = 0.0;
    vehTestWeight = Dvar_RegisterFloat("vehTestWeight", 5200.0, minb, DVAR_CHEAT, "lbs");
    minc.value.max = FLT_MAX;
    minc.value.min = 0.0;
    vehTestMaxMPH = Dvar_RegisterFloat("vehTestMaxMPH", 40.0, minc, DVAR_CHEAT, "");
}


int32_t __cdecl G_VehPlayerRideSlot(gentity_s *vehicle, int32_t playerEntNum)
{
    iassert(vehicle->scr_vehicle);

    for (int i = 0; i < 3; ++i)
    {
        if (vehicle->scr_vehicle->boneIndex.riderSlots[i].entNum == playerEntNum)
            return i;
    }

    Com_Error(ERR_DROP, "VehicleGetPlayerRideSlot(): player ent #%i was not using vehicle.", playerEntNum);
    return 0;
}

void __cdecl VEH_DebugCapsule(float *pos, float rad, float height, float r, float g, float b)
{
    float top[3]; // [esp+14h] [ebp-1Ch] BYREF
    float color[4]; // [esp+20h] [ebp-10h] BYREF

    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0;
    top[0] = *pos;
    top[1] = pos[1];
    top[2] = pos[2] + height;
    G_DebugCircle(pos, rad, color, 1, 1, 1);
    G_DebugCircle(top, rad, color, 1, 1, 1);
}

// MP only
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *angles)
{
    if (Vec3Compare(origin, ent->r.currentOrigin)
        && Vec3Compare(origin, ent->s.lerp.pos.trBase)
        && Vec3Compare(angles, ent->r.currentAngles)
        && Vec3Compare(angles, ent->s.lerp.apos.trBase))
        return;

    G_SetOrigin(ent, origin);
    G_SetAngle(ent, angles);
    ent->s.lerp.pos.trType = TR_INTERPOLATE;
    ent->s.lerp.apos.trType = TR_INTERPOLATE;
    SV_LinkEntity(ent);
}


void __cdecl G_VehUnlinkPlayer(gentity_s *ent, gentity_s *player)
{
    gclient_s *client; // [esp+8h] [ebp-24h]
    scr_vehicle_s *veh; // [esp+10h] [ebp-1Ch]
    float origin[3]; // [esp+14h] [ebp-18h] BYREF
    float angles[3]; // [esp+20h] [ebp-Ch] BYREF

    client = player->client;

    iassert(player->r.ownerNum.isDefined());
    iassert(ent == player->r.ownerNum.ent());

    if ((client->ps.pm_flags & PMF_VEHICLE_ATTACHED) == 0)
        Com_Error(ERR_DROP, "G_VehUnlinkPlayer: Player is not using a vehicle");

    veh = ent->scr_vehicle;

    iassert(veh);

    veh->flags &= ~1u;
    angles[0] = client->ps.viewangles[0];
    angles[1] = client->ps.viewangles[1];
    angles[2] = 0.0;
    origin[0] = player->r.currentOrigin[0];
    origin[1] = player->r.currentOrigin[1];
    origin[2] = player->r.currentOrigin[2];
    origin[2] = ent->r.currentOrigin[2] + veh->phys.maxs[2];
    angles[0] = 0.0;
    TeleportPlayer(player, origin, angles);
    VehicleClearRideSlotForPlayer(ent, player->s.number);
    player->r.ownerNum.setEnt(NULL);
    client->ps.pm_flags &= ~PMF_VEHICLE_ATTACHED;
    client->ps.weapFlags &= ~0x80u;
    client->ps.viewlocked_entNum = ENTITYNUM_NONE;
}

void __cdecl VehicleClearRideSlotForPlayer(gentity_s *ent, int32_t playerEntNum)
{
    int32_t i; // [esp+0h] [ebp-8h]

    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 306, 0, "%s", "ent->scr_vehicle");
    for (i = 0; i < 3; ++i)
    {
        if (ent->scr_vehicle->boneIndex.riderSlots[i].entNum == playerEntNum)
        {
            ent->scr_vehicle->boneIndex.riderSlots[i].entNum = ENTITYNUM_NONE;
            return;
        }
    }
    Com_Error(ERR_DROP, "VehicleClearRideSlotForPlayer(): player ent #%i was not using vehicle.", playerEntNum);
}

void __cdecl G_VehiclesInit(int32_t restarting)
{
    __int16 i; // [esp+0h] [ebp-4h]

    InitInfos(restarting);
    for (i = 0; i < 8; ++i)
        s_vehicles[i].entNum = ENTITYNUM_NONE;
    level.vehicles = s_vehicles;
}

void __cdecl InitInfos(int32_t restarting)
{
    int32_t vehIndex; // [esp+0h] [ebp-Ch]
    int32_t sndIndex; // [esp+4h] [ebp-8h]
    vehicle_info_t *vehInfo; // [esp+8h] [ebp-4h]

    if (restarting)
    {
        for (vehIndex = 0; vehIndex < s_numVehicleInfos; ++vehIndex)
        {
            vehInfo = &s_vehicleInfos[vehIndex];
            for (sndIndex = 0; sndIndex < 6; ++sndIndex)
            {
                if (vehInfo->sndIndices[sndIndex])
                    vehInfo->sndIndices[sndIndex] = G_SoundAliasIndex(vehInfo->sndNames[sndIndex]);
            }
        }
    }
    else
    {
        s_numVehicleInfos = 0;
    }
}

void __cdecl G_VehiclesSetupSpawnedEnts()
{
    gentity_s *ent; // [esp+4h] [ebp-8h]
    __int16 i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 8; ++i)
    {
        if (s_vehicles[i].entNum != ENTITYNUM_NONE)
        {
            ent = &g_entities[s_vehicles[i].entNum];
            if (ent->classname != scr_const.script_vehicle)
                MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2751, 0, "%s", "ent->classname == scr_const.script_vehicle");
            if (!ent->scr_vehicle)
                MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2752, 0, "%s", "ent->scr_vehicle");
            SetupCollisionMap(ent);
        }
    }
}

void __cdecl SetupCollisionMap(gentity_s *ent)
{
    gentity_s *cmEnt; // [esp+0h] [ebp-4h]

    cmEnt = GetCollisionMap(SL_ConvertToString(G_ModelName(ent->model)));
    if (cmEnt)
    {
        if (cmEnt->s.index.brushmodel)
        {
            ent->s.index.brushmodel = cmEnt->s.index.brushmodel;
            SV_SetBrushModel(ent);
            ent->r.contents = 0x800000;
            if ((ent->spawnflags & 1) != 0)
                ent->r.contents |= 0x200000u;
        }
        else
        {
            Com_PrintWarning(15, "WARNING: Cannot use empty vehicle collmap for [%s]\n", SL_ConvertToString(G_ModelName(ent->model)));
        }
    }
    else
    {
        Com_PrintWarning(15, "WARNING: Cannot find vehicle collmap for [%s]\n", SL_ConvertToString(G_ModelName(ent->model)));
    }
}

gentity_s *__cdecl GetCollisionMap(const char *modelname)
{
    const char *targetname; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int32_t i; // [esp+8h] [ebp-4h]

    for (i = 0; i < level.num_entities; ++i)
    {
        ent = &g_entities[i];
        if (ent->r.inuse && ent->classname == scr_const.script_vehicle_collmap)
        {
            targetname = SL_ConvertToString(ent->targetname);
            if (Com_IsLegacyXModelName(targetname))
                targetname += 7;
            if (!I_stricmp(targetname, modelname))
                return &g_entities[i];
        }
    }
    return 0;
}

void __cdecl SpawnVehicle(gentity_s *ent, const char *typeName)
{
    uint32_t WeaponIndexForName; // eax
    const char *v3; // eax
    vehicle_info_t *info; // [esp+0h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-Ch]
    int32_t infoIdx; // [esp+8h] [ebp-8h] BYREF
    int32_t i; // [esp+Ch] [ebp-4h]

    veh = 0;
    for (i = 0; i < 8; ++i)
    {
        veh = &s_vehicles[i];
        if (veh->entNum == ENTITYNUM_NONE)
            break;
    }
    if (i == 8)
        Com_Error(ERR_DROP, "Hit max vehicle count [%d]", 8);
    infoIdx = VEH_GetVehicleInfoFromName(typeName);
    if (infoIdx < 0)
        Com_Error(ERR_DROP, "Can't find info for script vehicle [%s]", typeName);
    ent->s.eType = ET_VEHICLE;
    VEH_InitModelAndValidateTags(ent, &infoIdx);
    if (!level.initializing)
    {
        info = &s_vehicleInfos[infoIdx];
        WeaponIndexForName = G_GetWeaponIndexForName(info->turretWeapon);
        if (!IsItemRegistered(WeaponIndexForName))
        {
            v3 = va("vehicle '%s' not precached", info->name);
            Scr_Error(v3);
        }
    }
    memset((uint8_t *)veh, 0, sizeof(scr_vehicle_s));
    InitEntityVars(ent, veh, infoIdx);
    InitEntityVehicleVars(ent, veh, infoIdx);
    InitVehicleTags(ent);
}


void __cdecl InitVehicleTags(gentity_s *ent)
{
    VehicleRideSlot_t *ridetag; // [esp+0h] [ebp-Ch]

    iassert(ent);
    iassert(ent->scr_vehicle);
    scr_vehicle_s *veh = ent->scr_vehicle;

    for (int i = 0; i < 3; ++i)
    {
        ridetag = &veh->boneIndex.riderSlots[i];
        ridetag->tagName = BG_VehiclesGetSlotTagName(i);
        ridetag->boneIdx = SV_DObjGetBoneIndex(ent, ridetag->tagName);
        ridetag->entNum = ENTITYNUM_NONE;
    }
    veh->boneIndex.detach = SV_DObjGetBoneIndex(ent, scr_const.tag_detach);
    veh->boneIndex.popout = SV_DObjGetBoneIndex(ent, scr_const.tag_popout);
    veh->boneIndex.body = SV_DObjGetBoneIndex(ent, scr_const.tag_body);
    veh->boneIndex.turret = SV_DObjGetBoneIndex(ent, scr_const.tag_turret);
    veh->boneIndex.barrel = SV_DObjGetBoneIndex(ent, scr_const.tag_barrel);
    for (int i = 0; i < 5; ++i)
        veh->boneIndex.flash[i] = SV_DObjGetBoneIndex(ent, *s_flashTags[i]);
    for (int i = 0; i < 4; ++i)
        veh->boneIndex.wheel[i] = SV_DObjGetBoneIndex(ent, *s_wheelTags[i]);
}

void __cdecl InitEntityVehicleVars(gentity_s *ent, scr_vehicle_s *veh, __int16 infoIdx)
{
    VEH_InitPhysics(ent);
    veh->entNum = ent->s.number;
    veh->infoIdx = infoIdx;
    veh->moveState = VEH_MOVESTATE_STOP;
    veh->waitNode = -1;
    veh->waitSpeed = -1.0;
    veh->turret.fireTime = 0;
    veh->turret.fireBarrel = 0;
    veh->turret.turretState = VEH_TURRET_STOPPED;
    Com_Memset((uint32_t *)&veh->jitter, 0, 60);
    veh->drawOnCompass = 0;
    veh->lookAtText0 = 0;
    veh->lookAtText1 = 0;
    veh->manualMode = 0;
    veh->manualSpeed = 0.0;
    veh->manualAccel = 0.0;
    veh->manualDecel = 0.0;
    veh->manualTime = 0.0;
    veh->speed = 0.0;
    veh->turningAbility = 0.5;
    veh->joltDir[0] = 0.0;
    veh->joltDir[1] = 0.0;
    veh->joltTime = 0.0;
    veh->joltWave = 0.0;
    veh->joltSpeed = 0.0;
    veh->joltDecel = 0.0;
    veh->turretHitNum = 0;
    VEH_SetPosition(ent, ent->r.currentOrigin, ent->r.currentAngles);
}

void __cdecl InitEntityVars(gentity_s *ent, scr_vehicle_s *veh, int32_t infoIdx)
{
    ent->handler = ENT_HANDLER_VEHICLE;
    ent->r.svFlags = 4;
    ent->r.contents = 8320;
    if ((ent->spawnflags & 1) != 0)
    {
        if (!alwaysfails)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2874, 0, "Initializing a usable vehicle!");
        ent->r.contents |= 0x200000u;
    }
    ent->s.eType = ET_VEHICLE;
    ent->s.lerp.eFlags = 0;
    ent->s.lerp.pos.trType = TR_INTERPOLATE;
    ent->s.lerp.apos.trType = TR_INTERPOLATE;
    ent->s.time2 = 0;
    ent->s.loopSound = 0;
    ent->s.weapon = G_GetWeaponIndexForName(s_vehicleInfos[infoIdx].turretWeapon);
    ent->s.weaponModel = 0;
    ent->s.lerp.u.vehicle.bodyPitch = 0.0f;
    ent->s.lerp.u.vehicle.bodyRoll = 0.0f;
    ent->s.lerp.u.vehicle.steerYaw = 0.0f;
    ent->s.lerp.u.vehicle.materialTime = 0;
    ent->s.lerp.u.vehicle.gunPitch = 0.0f;
    ent->s.lerp.u.vehicle.gunYaw = 0.0f;
    ent->s.lerp.u.vehicle.teamAndOwnerIndex = 0;
    ent->scr_vehicle = veh;
    ent->nextthink = level.time + 50;
    ent->takedamage = 1;
    ent->active = 1;
    ent->clipmask = 0x810211;
    SV_DObjGetBounds(ent, ent->r.mins, ent->r.maxs);
    SV_LinkEntity(ent);
}

void __cdecl G_VehFreeEntity(gentity_s *vehEnt)
{
    scr_vehicle_s *scr_vehicle; // [esp+0h] [ebp-4h]

    scr_vehicle = vehEnt->scr_vehicle;
    iassert(scr_vehicle->entNum != ENTITYNUM_NONE);
    vehEnt->health = 0;
    VEH_UpdateSounds(vehEnt);
    Scr_SetString(&scr_vehicle->lookAtText0, 0);
    Scr_SetString(&scr_vehicle->lookAtText1, 0);
    scr_vehicle->lookAtEnt.setEnt(NULL);
    scr_vehicle->idleSndEnt.setEnt(NULL);
    scr_vehicle->engineSndEnt.setEnt(NULL);
    vehEnt->nextthink = 0;
    vehEnt->takedamage = 0;
    vehEnt->active = 0;
    vehEnt->s.lerp.eFlags = 0;
    vehEnt->s.lerp.pos.trType = TR_STATIONARY;
    vehEnt->s.lerp.apos.trType = TR_STATIONARY;
    scr_vehicle->entNum = ENTITYNUM_NONE;
    vehEnt->scr_vehicle = 0;
}

bool __cdecl G_VehUsable(gentity_s *vehicle, gentity_s *player)
{
    gclient_s *client; // [esp+0h] [ebp-4h]

    client = player->client;
    if (!client)
        return 0;
    if ((client->ps.pm_flags & PMF_VEHICLE_ATTACHED) != 0)
        return 0;
    if (player->r.ownerNum.isDefined())
        return 0;
    if (!VehicleHasSeatFree(vehicle))
        return 0;
    if (vehicle->scr_vehicle->speed > 1.0)
        return 0;
    if (vehicle->health > 0)
        return (vehicle->r.contents & 0x200000) != 0;
    return 0;
}

char __cdecl VehicleHasSeatFree(gentity_s *ent)
{
    int32_t i; // [esp+0h] [ebp-8h]

    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 266, 0, "%s", "ent->scr_vehicle");
    for (i = 0; i < 3; ++i)
    {
        if (ent->scr_vehicle->boneIndex.riderSlots[i].boneIdx != -1
            && ent->scr_vehicle->boneIndex.riderSlots[i].entNum == ENTITYNUM_NONE)
        {
            return 1;
        }
    }
    return 0;
}

bool __cdecl G_VehImmuneToDamage(gentity_s *ent, int32_t mod, char damageFlags, uint32_t weapon)
{
    bool result; // eax
    vehicle_info_t *info; // [esp+4h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = ent->scr_vehicle;
    if (!veh)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3023, 0, "%s", "veh");
    info = &s_vehicleInfos[veh->infoIdx];
    switch (mod)
    {
    case 1:
    case 2:
        if (info->bulletDamage)
            result = 0;
        else
            result = (damageFlags & DAMAGE_NO_ARMOR) == 0 || !info->armorPiercingDamage;
        break;
    case 3:
    case 4:
        if (BG_GetWeaponDef(weapon)->projExplosion == WEAPPROJEXP_HEAVY)
            result = info->heavyExplosiveDamage == 0;
        else
            result = info->grenadeDamage == 0;
        break;
    case 5:
        result = info->projectileDamage == 0;
        break;
    case 6:
        result = info->projectileSplashDamage == 0;
        break;
    case 14:
        result = 0;
        break;
    default:
        result = 1;
        break;
    }
    return result;
}

void __cdecl VEH_PushEntity_0(
    gentity_s *ent,
    gentity_s *target,
    float frameTime,
    float *pushDir,
    float *deltaOrigin,
    float *deltaAngles)
{
    float damagea; // [esp+4h] [ebp-10h]
    float damage; // [esp+4h] [ebp-10h]
    float dist; // [esp+8h] [ebp-Ch]
    float mph; // [esp+10h] [ebp-4h]

    iassert(ent);
    iassert(target);
    if (!target->tagInfo
        && (Vec3LengthSq(deltaOrigin) >= EQUAL_EPSILON || Vec3LengthSq(deltaAngles) >= EQUAL_EPSILON))
    {
        if (AttachedStickyMissile(ent, target))
        {
            PushAttachedStickyMissile(ent, target);
        }
        else if (G_TryPushingEntity(target, ent, deltaOrigin, deltaAngles))
        {
            dist = Vec3Length(deltaOrigin);
            mph = dist / frameTime * 0.056818184;
            if (target->s.eType == ET_PLAYER && target->s.groundEntityNum != ent->s.number && mph >= 10.0)
            {
                if (mph >= 50.0)
                {
                    damage = 100.0;
                }
                else
                {
                    damagea = (mph - 10.0) / (50.0 - 10.0);
                    damage = (100.0 - 5.0) * damagea;
                }
                InflictDamage(ent, target, pushDir, damage);
            }
        }
        else if (target->takedamage)
        {
            InflictDamage(ent, target, pushDir, 999999);
        }
    }
}

void __cdecl VEH_TouchEntities_0(gentity_s *ent, float frameTime)
{
    scr_vehicle_s *v2; // eax
    scr_vehicle_s *ecx26; // ecx
    float v4; // [esp+10h] [ebp-10D4h]
    float scale; // [esp+14h] [ebp-10D0h]
    float v[3]; // [esp+38h] [ebp-10ACh] BYREF
    float v7; // [esp+44h] [ebp-10A0h]
    float v8[3]; // [esp+48h] [ebp-109Ch] BYREF
    float result[3]; // [esp+54h] [ebp-1090h] BYREF
    float *origin; // [esp+60h] [ebp-1084h]
    int32_t contentmask; // [esp+64h] [ebp-1080h]
    int32_t v12; // [esp+68h] [ebp-107Ch]
    float *a; // [esp+6Ch] [ebp-1078h]
    vehicle_info_t *v14; // [esp+70h] [ebp-1074h]
    float maxs[3]; // [esp+74h] [ebp-1070h] BYREF
    scr_vehicle_s *scr_vehicle; // [esp+80h] [ebp-1064h]
    float b[3]; // [esp+84h] [ebp-1060h] BYREF
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+90h] [ebp-1054h]
    float v19; // [esp+94h] [ebp-1050h]
    gentity_s *target; // [esp+98h] [ebp-104Ch]
    float out[3]; // [esp+9Ch] [ebp-1048h] BYREF
    float v3[3]; // [esp+A8h] [ebp-103Ch] BYREF
    float sum[3]; // [esp+B4h] [ebp-1030h] BYREF
    void(__cdecl * v24)(gentity_s *, gentity_s *, int); // [esp+C0h] [ebp-1024h]
    int32_t entityList[1025]; // [esp+C4h] [ebp-1020h] BYREF
    int32_t i; // [esp+10C8h] [ebp-1Ch]
    float mins[3]; // [esp+10CCh] [ebp-18h] BYREF
    float diff[3]; // [esp+10D8h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1481, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1482, 0, "%s", "ent->scr_vehicle");
    if (ent->r.bmodel)
    {
        scr_vehicle = ent->scr_vehicle;
        a = scr_vehicle->phys.origin;
        v14 = &s_vehicleInfos[scr_vehicle->infoIdx];
        touch = entityHandlers[ent->handler].touch;
        Vec3Sub(scr_vehicle->phys.origin, scr_vehicle->phys.prevOrigin, diff);
        AnglesSubtract(scr_vehicle->phys.angles, scr_vehicle->phys.prevAngles, v3);
        Vec3NormalizeTo(scr_vehicle->phys.vel, out);
        v19 = RadiusFromBounds(ent->r.mins, ent->r.maxs);
        b[0] = -v19;
        b[1] = b[0];
        b[2] = b[0];
        sum[0] = v19;
        sum[1] = v19;
        sum[2] = v19;
        Vec3Add(scr_vehicle->phys.prevOrigin, b, b);
        Vec3Add(scr_vehicle->phys.prevOrigin, sum, sum);
        ExtendBounds(b, sum, diff);
        contentmask = 0x2806081;
        v12 = CM_AreaEntities(b, sum, entityList, 1024, 0x2806081);
        for (i = 0; ; ++i)
        {
            if (i >= v12)
                return;
            target = &g_entities[entityList[i]];
            v24 = entityHandlers[target->handler].touch;
            if (target->s.number != ent->s.number
                && (target->s.eType == ET_PLAYER || target->s.eType == ET_SCRIPTMOVER || target->s.eType == ET_VEHICLE || target->s.eType == ET_MISSILE)
                && (!target->r.ownerNum.isDefined() || target->s.eType == ET_MISSILE && target->r.ownerNum.ent() != ent))
            {
                if (target->s.groundEntityNum == ent->s.number)
                    goto LABEL_18;
                if (target->classname == scr_const.script_model)
                {
                    if (!target->model)
                        continue;
                    SV_DObjGetBounds(target, mins, maxs);
                    Vec3Add(target->r.currentOrigin, mins, mins);
                    Vec3Add(target->r.currentOrigin, maxs, maxs);
                }
                else if (target->classname == scr_const.script_vehicle)
                {
                    if (!target->model)
                        continue;
                    if (!target->scr_vehicle)
                        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1553, 0, "%s", "hit->scr_vehicle");
                    v2 = target->scr_vehicle;
                    mins[0] = v2->phys.mins[0];
                    mins[1] = v2->phys.mins[1];
                    mins[2] = v2->phys.mins[2];
                    Vec3Scale(mins, 2.0, mins);
                    Vec3Add(target->r.currentOrigin, mins, mins);
                    ecx26 = target->scr_vehicle;
                    maxs[0] = ecx26->phys.maxs[0];
                    maxs[1] = ecx26->phys.maxs[1];
                    maxs[2] = ecx26->phys.maxs[2];
                    Vec3Scale(maxs, 2.0, maxs);
                    Vec3Add(target->r.currentOrigin, maxs, maxs);
                }
                else
                {
                    mins[0] = target->r.absmin[0];
                    mins[1] = target->r.absmin[1];
                    mins[2] = target->r.absmin[2];
                    maxs[0] = target->r.absmax[0];
                    maxs[1] = target->r.absmax[1];
                    maxs[2] = target->r.absmax[2];
                }
                ExpandBoundsToWidth(mins, maxs);
                if (SV_EntityContact(mins, maxs, ent))
                {
                    if (Scr_IsSystemActive())
                    {
                        Scr_AddEntity(ent);
                        Scr_Notify(target, scr_const.touch, 1u);
                        Scr_AddEntity(target);
                        Scr_Notify(ent, scr_const.touch, 1u);
                    }
                    if (v24)
                        v24(target, ent, 1);
                    if (touch)
                        touch(ent, target, 1);
                    if (target->s.eType == ET_PLAYER)
                    {
                    LABEL_18:
                        VEH_PushEntity_0(ent, target, frameTime, out, diff, v3);
                        continue;
                    }
                    if (target->s.eType == ET_VEHICLE)
                    {
                        if (!target->scr_vehicle)
                            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1604, 0, "%s", "hit->scr_vehicle");
                        origin = target->scr_vehicle->phys.origin;
                        v7 = Vec3Length(a + 30);
                        Vec3Sub(origin, a, v);
                        Vec3Normalize(v);
                        scale = -v7 * 0.80000001;
                        Vec3Scale(v, scale, result);
                        result[2] = v7 * 0.1 + result[2];
                        Vec3Add(a + 21, result, a + 21);
                        v4 = v7 * 0.15000001;
                        Vec3Scale(v, v4, v8);
                        Vec3Add(origin + 21, v8, origin + 21);
                    }
                }
            }
        }
    }
}

void __cdecl G_VehEntHandler_Think(gentity_s *pSelf)
{
    float frameTime; // [esp+10h] [ebp-14h]
    vehicle_info_t *info; // [esp+18h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+1Ch] [ebp-8h]
    VehicleTags *rideTag; // [esp+20h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3153, 0, "%s", "pSelf");
    if (!pSelf->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3154, 0, "%s", "pSelf->scr_vehicle");
    veh = pSelf->scr_vehicle;
    info = &s_vehicleInfos[veh->infoIdx];
    frameTime = (double)level.frametime * EQUAL_EPSILON;
    if ((veh->flags & 8) != 0)
    {
        VEH_BackupPosition(pSelf);
        memset((uint8_t *)&s_phys, 0, sizeof(s_phys));
        for (rideTag = RideTagFirst(pSelf); rideTag; rideTag = RideTagNext(pSelf, rideTag->riderSlots))
        {
            if (rideTag->riderSlots[0].entNum != ENTITYNUM_NONE && g_entities[rideTag->riderSlots[0].entNum].health <= 0)
                G_EntUnlink(&g_entities[rideTag->riderSlots[0].entNum]);
        }
        VEH_UpdateClients(pSelf);
        UpdateSimulation(pSelf);
        if (veh->speed < 0.0)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3185, 0, "%s", "veh->speed >= 0.0f");
        VEH_SetPosition(pSelf, veh->phys.origin, veh->phys.angles);
        VEH_TouchEntities_0(pSelf, frameTime);
        if (vehDebugServer->current.enabled)
            VEH_DebugBox(veh->phys.origin, 4.0, 1.0, 1.0, 0.0);
        if ((veh->flags & 1) != 0)
            VEH_UpdateWeapon(pSelf);
        UpdateTurret(pSelf);
        VEH_UpdateBody(pSelf, frameTime);
        VEH_UpdateSteering(pSelf);
        VEH_UpdateMaterialTime(pSelf, frameTime);
        VEH_UpdateSounds(pSelf);
        pSelf->s.time2 = (int)(info->suspensionTravel * 1000.0);
        pSelf->nextthink = level.time + 50;
    }
    else
    {
        InitFirstThink(pSelf);
    }
}

VehicleTags *__cdecl RideTagFirst(gentity_s *ent)
{
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 225, 0, "%s", "ent->scr_vehicle");
    return &ent->scr_vehicle->boneIndex;
}

VehicleTags *__cdecl RideTagNext(gentity_s *ent, VehicleRideSlot_t *inTag)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]
    int32_t i; // [esp+8h] [ebp-4h]
    int32_t ia; // [esp+8h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 235, 0, "%s", "ent");
    if (!inTag)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 236, 0, "%s", "inTag");
    veh = ent->scr_vehicle;
    if (!veh)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 239, 0, "%s", "veh");
    for (i = 0; i < 3; ++i)
    {
        if (&veh->boneIndex.riderSlots[i] == inTag)
        {
            ia = i + 1;
            if (ia == 3)
                return 0;
            else
                return (VehicleTags *)((char *)&veh->boneIndex + 12 * ia);
        }
    }
    if (!alwaysfails)
        MyAssertHandler(
            ".\\game_mp\\g_vehicles_mp.cpp",
            256,
            0,
            "RideTagNext() ran through riderSlots without finding a match.  inTag does not belong to veh.");
    return 0;
}

void __cdecl VEH_DebugBox(float *pos, float width, float r, float g, float b)
{
    float mins[3]; // [esp+10h] [ebp-28h] BYREF
    float maxs[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float color[4]; // [esp+28h] [ebp-10h] BYREF

    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0;
    mins[0] = -(width * 0.5);
    mins[1] = mins[0];
    mins[2] = mins[0];
    maxs[0] = width * 0.5;
    maxs[1] = maxs[0];
    maxs[2] = maxs[0];
    G_DebugBox(pos, mins, maxs, 0.0, color, 1, 1);
}

void __cdecl InflictDamage(gentity_s *vehEnt, gentity_s *target, float *dir, int32_t damage)
{
    int32_t attackerNum; // [esp+4h] [ebp-4h]

    if (!vehEnt)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1327, 0, "%s", "vehEnt");
    if (!target)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1328, 0, "%s", "target");
    attackerNum = VehicleEntDriver(vehEnt);
    if (attackerNum == ENTITYNUM_NONE)
        attackerNum = ENTITYNUM_WORLD;
    if (vehDebugServer->current.enabled)
    {
        float dmg = (float)damage;
        Com_Printf(16, "Vehicle damage to ent #%i: %.2f\n", target->s.number, dmg);
    }
    G_Damage(
        target,
        vehEnt,
        &g_entities[attackerNum],
        dir,
        target->r.currentOrigin,
        damage,
        DAMAGE_NOFLAG,
        9,
        0xFFFFFFFF,
        HITLOC_NONE,
        0,
        0,
        0);
}

int32_t __cdecl VehicleEntDriver(gentity_s *ent)
{
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 282, 0, "%s", "ent->scr_vehicle");
    return ent->scr_vehicle->boneIndex.riderSlots[0].entNum;
}

void __cdecl UpdateTurret(gentity_s *ent)
{
    int32_t playerEntNum; // [esp+0h] [ebp-8h]
    gentity_s *player; // [esp+4h] [ebp-4h]

    playerEntNum = VehicleEntGunner(ent);
    if (playerEntNum == ENTITYNUM_NONE)
    {
        ent->s.lerp.u.vehicle.gunYaw = 0.0;
        ent->s.lerp.u.vehicle.gunPitch = 0.0;
    }
    else
    {
        player = &g_entities[playerEntNum];
        if (!player->client)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1635, 0, "%s", "player->client");
        ent->s.lerp.u.vehicle.gunYaw = player->client->ps.viewangles[1];
        ent->s.lerp.u.vehicle.gunPitch = player->client->ps.viewangles[0];
    }
}

int32_t __cdecl VehicleEntGunner(gentity_s *ent)
{
    iassert(ent->scr_vehicle);
    return ent->scr_vehicle->boneIndex.riderSlots[2].entNum;
}

void __cdecl FireTurret(gentity_s *ent, gentity_s *player)
{
    scr_vehicle_s *veh; // [esp+14h] [ebp-44h]
    weaponParms wp; // [esp+18h] [ebp-40h] BYREF

    if (ent->s.weapon)
    {
        veh = ent->scr_vehicle;
        if (!veh)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1750, 0, "%s", "veh");
        FillWeaponParms(ent, player, &wp);
        if (wp.weapDef->weapType)
            Com_Error(ERR_DROP, "FireTurret(): WeapDef is not a bullet type.");
        else
            Bullet_Fire(player, wp.weapDef->fAdsSpread, &wp, ent, level.time);
        G_AddEvent(ent, EV_FIRE_WEAPON, 0);
        veh->turret.fireTime = wp.weapDef->iFireTime;
    }
}

void __cdecl FillWeaponParms(gentity_s *vehEnt, gentity_s *player, weaponParms *wp)
{
    float endpoint[3]; // [esp+18h] [ebp-4Ch] BYREF
    float flashTag[4][3]; // [esp+28h] [ebp-3Ch] BYREF
    float flashAngles[3]; // [esp+58h] [ebp-Ch] BYREF

    wp->weapDef = BG_GetWeaponDef(vehEnt->s.weapon);
    if (G_DObjGetWorldTagMatrix(vehEnt, scr_const.tag_flash, flashTag))
    {
        AxisToAngles(*(const mat3x3*)&flashTag, flashAngles);
        AngleVectors(flashAngles, wp->forward, wp->right, wp->up);
        wp->gunForward[0] = wp->forward[0];
        wp->gunForward[1] = wp->forward[1];
        wp->gunForward[2] = wp->forward[2];
        wp->muzzleTrace[0] = flashTag[3][0];
        wp->muzzleTrace[1] = flashTag[3][1];
        wp->muzzleTrace[2] = flashTag[3][2];
        if (vehDebugServer->current.enabled)
        {
            Vec3Mad(flashTag[3], 1000.0, wp->forward, endpoint);
            G_DebugLine(flashTag[3], endpoint, colorRed, 0);
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Couldn't find tag %s on vehicle (entity %d).", "tag_flash", vehEnt->s.number);
    }
}

void __cdecl VEH_UpdateClients(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1956, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1957, 0, "%s", "ent->scr_vehicle");
    VEH_UpdateClientDriver(ent);
    VEH_UpdateClientPassenger(ent);
    VEH_UpdateClientGunner(ent);
}

void __cdecl VEH_UpdateClientPassenger(gentity_s *ent)
{
    VehicleEntPassenger(ent);
}

int32_t __cdecl VehicleEntPassenger(gentity_s *ent)
{
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 289, 0, "%s", "ent->scr_vehicle");
    return ent->scr_vehicle->boneIndex.riderSlots[1].entNum;
}

void __cdecl VEH_UpdateClientGunner(gentity_s *ent)
{
    VehicleEntGunner(ent);
}

void __cdecl VEH_UpdateClientDriver(gentity_s *ent)
{
    char sign; // [esp+Ah] [ebp-16h]
    char accela; // [esp+Bh] [ebp-15h]
    char accel; // [esp+Bh] [ebp-15h]
    scr_vehicle_s *veh; // [esp+14h] [ebp-Ch]
    int32_t playerEntNum; // [esp+18h] [ebp-8h]
    gentity_s *player; // [esp+1Ch] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1883, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1884, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    veh->phys.driverPedal = 0.0;
    veh->phys.driverSteer = 0.0;
    veh->phys.inputAccelerationOLD = 0;
    veh->phys.inputTurning = 0;
    playerEntNum = VehicleEntDriver(ent);
    if (playerEntNum == ENTITYNUM_NONE)
    {
        veh->phys.inputAccelerationOLD = 0;
        veh->phys.inputTurning = 0;
        veh->phys.driverPedal = 0.0;
        veh->phys.driverSteer = 0.0;
    }
    else
    {
        player = &g_entities[playerEntNum];
        if (!player->client)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 1906, 0, "%s", "player->client");
        if ((player->client->ps.pm_flags & (PMF_RESPAWNED | PMF_FROZEN)) == 0)
        {
            veh->phys.inputAccelerationOLD = player->client->sess.cmd.forwardmove;
            veh->phys.inputTurning = VEH_PlayerRotation(player, &veh->phys);
            veh->phys.driverPedal = (double)player->client->sess.cmd.forwardmove / 126.0;
            veh->phys.driverSteer = (double)VEH_PlayerRotation(player, &veh->phys) / 127.0;
            accela = veh->phys.inputAccelerationOLD;
            sign = 2 * (accela >= 0) - 1;
            accel = sign * accela;
            if (accel >= 10)
            {
                if (accel > 115)
                    accel = 115;
            }
            else
            {
                accel = 0;
            }
            veh->phys.inputAccelerationOLD = sign * accel;
            if (veh->phys.driverPedal > 1.0)
                veh->phys.driverPedal = 1.0;
            if (veh->phys.driverPedal < -1.0)
                veh->phys.driverPedal = -1.0;
            if (veh->phys.driverSteer > 1.0)
                veh->phys.driverSteer = 1.0;
            if (veh->phys.driverSteer < -1.0)
                veh->phys.driverSteer = -1.0;
        }
    }
}

char __cdecl VEH_PlayerRotation(gentity_s *player, vehicle_physic_t *phys)
{
    float v3; // [esp+Ch] [ebp-2Ch]
    float v4; // [esp+10h] [ebp-28h]
    float v5; // [esp+14h] [ebp-24h]
    float v6; // [esp+18h] [ebp-20h]
    float v7; // [esp+1Ch] [ebp-1Ch]
    float angle; // [esp+20h] [ebp-18h]
    float v9; // [esp+24h] [ebp-14h]
    float yawCurrent; // [esp+28h] [ebp-10h]
    float yawDelta; // [esp+2Ch] [ebp-Ch]
    char sign; // [esp+33h] [ebp-5h]
    float yawWanted; // [esp+34h] [ebp-4h]

    angle = (double)player->client->sess.cmd.angles[1] * 0.0054931640625 + 180.0;
    yawWanted = AngleNormalize360(angle);
    v7 = phys->angles[1] + 90.0;
    yawCurrent = AngleNormalize360(v7);
    v6 = yawWanted - yawCurrent;
    v9 = v6 * 0.002777777845039964;
    v5 = v9 + 0.5;
    v4 = floor(v5);
    yawDelta = (v9 - v4) * 360.0;
    if (yawDelta >= 0.0)
        sign = -1;
    else
        sign = 1;
    v3 = I_fabs(yawDelta);
    if (v3 >= 20.0)
        return 127 * sign;
    if (v3 >= 0.0099999998)
        return (int)(v3 / 20.0 * 127.0) * sign;
    return 0;
}

void __cdecl UpdateSimulation(gentity_s *ent)
{
    IntegratePosAndRot(ent);
}

void __cdecl IntegratePosAndRot(gentity_s *ent)
{
    double v1; // st7
    char *v2; // eax
    double scale; // [esp+8h] [ebp-38h]
    float *vel; // [esp+14h] [ebp-2Ch]
    float accelPos[3]; // [esp+20h] [ebp-20h] BYREF
    vehicle_physic_t *phys; // [esp+2Ch] [ebp-14h]
    scr_vehicle_s *veh; // [esp+30h] [ebp-10h]
    float frameTimea; // [esp+4Ch] [ebp+Ch]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2330, 0, "%s", "ent");
    veh = ent->scr_vehicle;
    if (!veh)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2332, 0, "%s", "veh");
    phys = &veh->phys;
    frameTimea = (double)level.frametime * EQUAL_EPSILON;
    AdvanceVehicleRotation(ent, frameTimea);
    AdvanceVehiclePosition(ent, frameTimea);
    GetAccelerationForces(ent, frameTimea, accelPos);
    Vec3Mad(phys->vel, frameTimea, accelPos, phys->vel);
    CapMaxMPH(phys);
    if (vehDebugServer->current.enabled)
    {
        scale = phys->rotVel[1];
        v1 = Vec3Length(phys->vel);
        v2 = va("%.1f MPH, %.2f DPS", v1 * 0.056818184, scale);
        G_DebugStarWithText(phys->origin, colorRed, colorRed, v2, 1.0);
    }
    if (Vec3Length(phys->vel) < 5.0)
    {
        vel = phys->vel;
        phys->vel[0] = 0.0;
        vel[1] = 0.0;
        vel[2] = 0.0;
    }
    if (vehDebugServer->current.enabled)
    {
        VEH_DebugCapsule(phys->origin, phys->maxs[0], phys->maxs[2], 1.0, 1.0, 0.0);
        DebugDrawVelocity(phys->origin, phys->vel, colorYellow);
    }
}

void __cdecl DebugDrawVelocity(const float *origin, const float *velocity, const float *color)
{
    float endpoint[3]; // [esp+0h] [ebp-Ch] BYREF

    Vec3Add(origin, velocity, endpoint);
    G_DebugLine(origin, endpoint, color, 0);
    G_DebugStar(endpoint, color);
}

void __cdecl GetAccelerationForces(gentity_s *ent, float frameTime, float *resultPosition)
{
    scr_vehicle_s *veh; // [esp+10h] [ebp-8h]
    float driverAccel; // [esp+14h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2223, 0, "%s", "ent");
    veh = ent->scr_vehicle;
    if (!veh)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2225, 0, "%s", "veh");
    driverAccel = veh->phys.driverPedal * vehTestHorsepower->current.value * 6600.0;
    PositionAccelForces(&veh->phys, driverAccel, resultPosition);
    KISAK_NULLSUB();
}

void __cdecl PositionAccelForces(vehicle_physic_t *phys, float driverAccel, float *result)
{
    float scale; // [esp+4h] [ebp-70h]
    float scalea; // [esp+4h] [ebp-70h]
    float v5; // [esp+Ch] [ebp-68h]
    float dir[3]; // [esp+24h] [ebp-50h] BYREF
    float v7; // [esp+30h] [ebp-44h]
    float breakAccel[3]; // [esp+34h] [ebp-40h] BYREF
    float mod; // [esp+40h] [ebp-34h]
    float rollResist[3]; // [esp+44h] [ebp-30h] BYREF
    float oldVelDir[3]; // [esp+50h] [ebp-24h] BYREF
    float dot; // [esp+5Ch] [ebp-18h]
    float forward[3]; // [esp+60h] [ebp-14h] BYREF
    bool breaking; // [esp+6Fh] [ebp-5h]
    float speed; // [esp+70h] [ebp-4h]

    *result = 0.0f;
    result[1] = 0.0f;
    result[2] = 0.0f;
    speed = Vec3Length(phys->vel);
    if (speed != 0.0f)
    {
        AngleVectors(phys->angles, forward, 0, 0);
        if (!CarTravelingForward(phys))
            Vec3Scale(forward, -1.0, forward);
        Vec3NormalizeTo(phys->vel, oldVelDir);
        Vec3Lerp(oldVelDir, forward, 0.60000002f, phys->vel);
        Vec3Normalize(phys->vel);
        dot = Vec3Dot(oldVelDir, phys->vel);
        speed = dot * dot * speed;
        Vec3Scale(phys->vel, speed, phys->vel);
    }
    speed = Vec3Length(phys->vel);
    breaking = DriverBreaking(phys, driverAccel);
    if (speed != 0.0f)
    {
        mod = 0.1f * vehTestWeight->current.value;
        scale = -mod;
        Vec3Scale(phys->vel, scale, rollResist);
        Vec3Add(rollResist, result, result);
    }
    if (breaking)
    {
        v7 = 1.0f * vehTestWeight->current.value;
        scalea = -v7;
        Vec3Scale(phys->vel, scalea, breakAccel);
        Vec3Add(breakAccel, result, result);
    }
    if (driverAccel != 0.0f)
    {
        AngleVectors(phys->angles, dir, 0, 0);
        Vec3Mad(result, driverAccel, dir, result);
    }
    if (!phys->onGround)
        result[2] = result[2] - vehTestWeight->current.value * 800.0f;
    v5 = 1.0f / vehTestWeight->current.value;
    Vec3Scale(result, v5, result);
}

bool __cdecl CarTravelingForward(vehicle_physic_t *phys)
{
    float velDir[3]; // [esp+4h] [ebp-1Ch] BYREF
    float forwardDir[3]; // [esp+10h] [ebp-10h] BYREF
    float speed; // [esp+1Ch] [ebp-4h]

    speed = Vec3Length(phys->vel);
    if (speed < 0.0099999998f)
        return 1;
    AngleVectors(phys->angles, forwardDir, 0, 0);
    Vec3NormalizeTo(phys->vel, velDir);
    return Vec3Dot(forwardDir, velDir) >= 0.0f;
}

bool __cdecl DriverBreaking(vehicle_physic_t *phys, float driverAccel)
{
    float v3; // [esp+4h] [ebp-Ch]

    v3 = I_fabs(driverAccel);
    return v3 < 0.0099999998f || CarTravelingForward(phys) != driverAccel >= 0.0f;
}

void __cdecl AdvanceVehiclePosition(gentity_s *ent, float frameTime)
{
    float v2; // [esp+4h] [ebp-20h]
    scr_vehicle_s *veh; // [esp+20h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2241, 0, "%s", "ent");
    veh = ent->scr_vehicle;
    if (!veh)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2243, 0, "%s", "veh");
    VEH_GroundTrace(ent);
    veh->phys.onGround = s_phys.onGround;
    Vec3Add(veh->phys.vel, veh->phys.colVelDelta, veh->phys.vel);
    veh->phys.colVelDelta[0] = 0.0f;
    veh->phys.colVelDelta[1] = 0.0f;
    veh->phys.colVelDelta[2] = 0.0f;
    if (0.0f == veh->phys.vel[0] && 0.0f == veh->phys.vel[1] && 0.0f == veh->phys.vel[2])
    {
        VEH_GroundPlant(ent, 1, frameTime);
    }
    else
    {
        if (veh->phys.onGround)
            VEH_GroundMove(ent, frameTime);
        else
            VEH_AirMove(ent, 1, frameTime);
        VEH_GroundPlant(ent, 1, frameTime);
        v2 = I_fabs(veh->phys.bodyVel[0]);
        ent->scr_vehicle->speed = v2;
        if (veh->speed < 0.0f)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2269, 0, "%s", "veh->speed >= 0.0f");
    }
}

void __cdecl VEH_DebugLine(float *start, float *end, float r, float g, float b)
{
    float color[4]; // [esp+0h] [ebp-10h] BYREF

    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0f;
    G_DebugLineWithDuration(start, end, color, 1, 1);
}

void __cdecl AdvanceVehicleRotation(gentity_s *ent, float frameTime)
{
    float v2; // [esp+8h] [ebp-24h]
    float v3; // [esp+Ch] [ebp-20h]
    float v4; // [esp+10h] [ebp-1Ch]
    float v5; // [esp+14h] [ebp-18h]
    float turningMod; // [esp+1Ch] [ebp-10h]
    float mph; // [esp+20h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+28h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2278, 0, "%s", "ent");
    veh = ent->scr_vehicle;
    if (!veh)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2280, 0, "%s", "veh");
    mph = Vec3Length(veh->phys.vel) * 0.056818184f;
    if (mph > 0.1f)
    {
        if (mph < 30.0f)
            turningMod = (mph - 0.1f) / (30.0f - 0.1f);
        else
            turningMod = 1.0f;
    }
    else
    {
        turningMod = 0.0f;
    }
    veh->phys.rotVel[1] = -veh->phys.driverSteer * 90.0f * turningMod;
    v4 = frameTime * veh->phys.rotVel[1] + veh->phys.prevAngles[1];
    v5 = v4 * 0.002777777845039964f;
    v3 = v5 + 0.5f;
    v2 = floor(v3);
    veh->phys.angles[1] = (v5 - v2) * 360.0f;
    veh->phys.angles[0] = 0.0f;
    veh->phys.angles[2] = 0.0f;
}

void __cdecl CapMaxMPH(vehicle_physic_t *phys)
{
    float maxIPS; // [esp+18h] [ebp-8h]
    float speed; // [esp+1Ch] [ebp-4h]

    speed = Vec3Length(phys->vel);
    maxIPS = vehTestMaxMPH->current.value * MPH_TO_INCHES_PER_SEC;
    if (maxIPS < speed)
    {
        Vec3Normalize(phys->vel);
        Vec3Scale(phys->vel, maxIPS, phys->vel);
    }
}

void __cdecl InitFirstThink(gentity_s *pSelf)
{
    float *prevAngles; // [esp+4h] [ebp-5Ch]
    float *angles; // [esp+8h] [ebp-58h]
    float *prevOrigin; // [esp+Ch] [ebp-54h]
    float *maxs; // [esp+10h] [ebp-50h]
    float *mins; // [esp+14h] [ebp-4Ch]
    float diff[3]; // [esp+1Ch] [ebp-44h] BYREF
    float wheelRight[3]; // [esp+28h] [ebp-38h] BYREF
    float wheelLeft[3]; // [esp+34h] [ebp-2Ch] BYREF
    float pos[3]; // [esp+40h] [ebp-20h] BYREF
    vehicle_physic_t *phys; // [esp+4Ch] [ebp-14h]
    vehicle_info_t *info; // [esp+50h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+54h] [ebp-Ch]
    float radius; // [esp+58h] [ebp-8h]
    int32_t wheelIndex; // [esp+5Ch] [ebp-4h]

    veh = pSelf->scr_vehicle;
    phys = &veh->phys;
    info = &s_vehicleInfos[veh->infoIdx];
    if (!info->type || info->type == 1)
    {
        if (!alwaysfails)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3077, 0, "Initializing a driveable vehicle!");
        if (info->type)
            MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3081, 0, "%s", "info->type == VEH_WHEELS_4");
        VEH_GetWheelOrigin(pSelf, 0, wheelLeft);
        VEH_GetWheelOrigin(pSelf, 1, wheelRight);
        Vec3Sub(wheelRight, wheelLeft, diff);
        radius = Vec3Length(diff) * 0.5f;
        radius = radius * 1.5f;
        mins = phys->mins;
        phys->mins[0] = -radius;
        mins[1] = -radius;
        mins[2] = 0.0f;
        maxs = phys->maxs;
        phys->maxs[0] = radius;
        maxs[1] = radius;
        maxs[2] = radius + radius;
    }
    for (wheelIndex = 0; wheelIndex < 4; ++wheelIndex)
    {
        if (veh->boneIndex.wheel[wheelIndex] >= 0)
        {
            G_DObjGetWorldBoneIndexPos(pSelf, veh->boneIndex.wheel[wheelIndex], pos);
            phys->wheelZPos[wheelIndex] = pos[2];
        }
    }
    VEH_GroundPlant(pSelf, 0, 0.05f);
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
    VEH_TouchEntities_0(pSelf, 0.05f);
    pSelf->handler = ENT_HANDLER_VEHICLE;
    pSelf->nextthink = level.time + 50;
    veh->flags |= 8u;
}

void __cdecl G_VehEntHandler_Touch(gentity_s *pSelf, gentity_s *pOther, int32_t bTouched)
{
    int32_t damage; // [esp+8h] [ebp-30h]
    float moveLen; // [esp+Ch] [ebp-2Ch]
    vehicle_info_t *info; // [esp+14h] [ebp-24h]
    float hitDir[2]; // [esp+18h] [ebp-20h] BYREF
    scr_vehicle_s *veh; // [esp+20h] [ebp-18h]
    float damageScale; // [esp+24h] [ebp-14h]
    float dot; // [esp+28h] [ebp-10h]
    float moveDir[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if (!pSelf)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3222, 0, "%s", "pSelf");
    if (!pSelf->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3223, 0, "%s", "pSelf->scr_vehicle");
    if (!pOther)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3224, 0, "%s", "pOther");
    veh = pSelf->scr_vehicle;
    info = &s_vehicleInfos[veh->infoIdx];
    if (pOther->s.eType == ET_PLAYER || pOther->s.eType == ET_SCRIPTMOVER)
    {
        if (pOther->takedamage)
        {
            if (!pOther->tagInfo && info->collisionDamage > 0.0f)
            {
                moveLen = Vec3NormalizeTo(veh->phys.vel, moveDir);
                if (moveLen >= EQUAL_EPSILON)
                {
                    if (pOther->s.eType == ET_SCRIPTMOVER)
                    {
                        InflictDamage(pSelf, pOther, moveDir, 999999);
                    }
                    else
                    {
                        hitDir[0] = pOther->r.currentOrigin[0] - pSelf->r.currentOrigin[0];
                        hitDir[1] = pOther->r.currentOrigin[1] - pSelf->r.currentOrigin[1];
                        Vec2Normalize(hitDir);
                        dot = hitDir[1] * moveDir[1] + hitDir[0] * moveDir[0];
                        if (dot >= 0.800000011920929f)
                        {
                            if (pOther->client && (pOther->client->ps.pm_flags & PMF_PRONE) != 0)
                            {
                                InflictDamage(pSelf, pOther, moveDir, 999999);
                            }
                            else
                            {
                                damageScale = veh->speed / info->collisionSpeed;
                                if (damageScale > 1.0f)
                                    damageScale = 1.0f;
                                damageScale = (dot - 0.800000011920929f) / 0.199999988079071f * damageScale;
                                damage = (int)(damageScale * info->collisionDamage);
                                if (damage > 0)
                                    InflictDamage(pSelf, pOther, moveDir, damage);
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl G_VehEntHandler_Use(gentity_s *pEnt, gentity_s *pOther, gentity_s *pActivator)
{
    if (pOther->client)
    {
        if ((pOther->client->ps.pm_flags & PMF_VEHICLE_ATTACHED) != 0)
            G_EntUnlink(pOther);
        else
            LinkPlayerToVehicle(pEnt, pOther);
    }
}

void __cdecl LinkPlayerToVehicle(gentity_s *ent, gentity_s *player)
{
    float diff[3]; // [esp+Ch] [ebp-7Ch] BYREF
    float pos[3]; // [esp+18h] [ebp-70h] BYREF
    float dist; // [esp+24h] [ebp-64h]
    VehicleRideSlot_t *rideTag; // [esp+28h] [ebp-60h]
    gclient_s *client; // [esp+2Ch] [ebp-5Ch]
    float playerAngles[3]; // [esp+30h] [ebp-58h] BYREF
    float originOffset[3]; // [esp+3Ch] [ebp-4Ch] BYREF
    scr_vehicle_s *veh; // [esp+48h] [ebp-40h]
    float bestRiderDist; // [esp+4Ch] [ebp-3Ch]
    VehicleRideSlot_t *bestRiderTag; // [esp+50h] [ebp-38h]
    int32_t i; // [esp+54h] [ebp-34h]
    float playerMtx[4][3]; // [esp+58h] [ebp-30h] BYREF

    veh = ent->scr_vehicle;
    client = player->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2487, 0, "%s", "client");
    if (!alwaysfails)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 2489, 0, "Trying to attach a player to a vehicle!");
    if ((client->ps.pm_flags & PMF_VEHICLE_ATTACHED) != 0)
        Com_Error(ERR_DROP, "LinkPlayerToVehicle: Player is already using a vehicle");
    if (player->r.ownerNum.isDefined())
        Com_Error(ERR_DROP, "LinkPlayerToVehicle: Player already has an owner");
    if (!VehicleHasSeatFree(ent))
        Com_Error(ERR_DROP, "LinkPlayerToVehicle: Vehicle has all seats filled");
    bestRiderTag = 0;
    bestRiderDist = 999999.0f;
    for (i = 0; i < 3; ++i)
    {
        rideTag = &veh->boneIndex.riderSlots[i];
        if (veh->boneIndex.riderSlots[i].boneIdx != -1 && rideTag->entNum == ENTITYNUM_NONE)
        {
            G_DObjGetWorldBoneIndexPos(ent, rideTag->boneIdx, pos);
            Vec3Sub(pos, player->r.currentOrigin, diff);
            dist = Vec3Length(diff);
            if (bestRiderDist > dist)
            {
                bestRiderTag = rideTag;
                bestRiderDist = dist;
            }
        }
    }
    if (!bestRiderTag)
        Com_Error(ERR_DROP, "LinkPlayerToVehicle: Tried to mount player on a full vehicle.");
    G_DObjGetWorldBoneIndexMatrix(ent, bestRiderTag->boneIdx, playerMtx);
    AxisToAngles(*(const mat3x3*)&playerMtx, playerAngles);
    playerAngles[2] = 0.0f;
    player->r.currentOrigin[0] = playerMtx[3][0];
    player->r.currentOrigin[1] = playerMtx[3][1];
    player->r.currentOrigin[2] = playerMtx[3][2];
    originOffset[0] = 0.0f;
    originOffset[1] = 0.0f;
    originOffset[2] = 0.0f;
    if (bestRiderTag->tagName == scr_const.tag_driver || bestRiderTag->tagName == scr_const.tag_passenger)
        originOffset[2] = -35.0f;
    if (!G_EntLinkToWithOffset(player, ent, bestRiderTag->tagName, originOffset, vec3_origin))
    {
        Com_Error(ERR_DROP, "LinkPlayerToVehicle: Cannot link to vehicle bone %s", SL_ConvertToString(bestRiderTag->tagName));
    }
    veh->flags |= 1u;
    bestRiderTag->entNum = player->s.number;
    player->r.ownerNum.setEnt(ent);
    client->ps.pm_flags |= PMF_VEHICLE_ATTACHED;
    if (bestRiderTag->tagName != scr_const.tag_passenger)
        client->ps.weapFlags |= 0x80u;
    client->ps.viewlocked_entNum = ent->s.number;
}

void __cdecl G_VehEntHandler_Die(
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
    WeaponDef *weapDef; // [esp+Ch] [ebp-8h]
    VehicleTags *rideTag; // [esp+10h] [ebp-4h]

    for (rideTag = RideTagFirst(pSelf); rideTag; rideTag = RideTagNext(pSelf, rideTag->riderSlots))
    {
        if (rideTag->riderSlots[0].entNum != ENTITYNUM_NONE)
            G_EntUnlink(&g_entities[rideTag->riderSlots[0].entNum]);
    }
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

void __cdecl G_VehEntHandler_Controller(const gentity_s *pSelf, int32_t *partBits)
{
    //float gunYaw; // [esp+4h] [ebp-38h]
    //float v3; // [esp+Ch] [ebp-30h]
    float barrelAngles[3]; // [esp+10h] [ebp-2Ch] BYREF
    DObj_s *obj; // [esp+1Ch] [ebp-20h]
    scr_vehicle_s *veh; // [esp+20h] [ebp-1Ch]
    float bodyAngles[3]; // [esp+24h] [ebp-18h] BYREF
    float turretAngles[3]; // [esp+30h] [ebp-Ch] BYREF

    if (!pSelf)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3336, 0, "%s", "pSelf");
    if (!pSelf->scr_vehicle)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3337, 0, "%s", "pSelf->scr_vehicle");
    veh = pSelf->scr_vehicle;
    obj = Com_GetServerDObj(pSelf->s.number);
    if (!obj)
        MyAssertHandler(".\\game_mp\\g_vehicles_mp.cpp", 3341, 0, "%s", "obj");
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

void __cdecl G_VehSpawner(gentity_s *pSelf)
{
    const char *typeName; // [esp+0h] [ebp-4h] BYREF

    G_LevelSpawnString("vehicletype", 0, &typeName);
    SpawnVehicle(pSelf, typeName);
}

void __cdecl G_VehCollmapSpawner(gentity_s *pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = ET_VEHICLE_COLLMAP;
}

