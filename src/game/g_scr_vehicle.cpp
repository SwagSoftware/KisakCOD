#include "game_public.h"

#ifdef KISAK_MP
#include <game_mp/g_public_mp.h>
#include <game_mp/g_utils_mp.h>
#elif KISAK_SP
#include "g_main.h"
#include "g_local.h"
#include "g_vehicle_path.h"
#include "g_public.h"
#endif

#include <script/scr_const.h>
#include <script/scr_vm.h>

#include <server/sv_game.h>
#include <server/sv_world.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <xanim/xanim.h>

#include "bullet.h"
#include <universal/surfaceflags.h>

//Line 51763:  0006 : 00006554       unsigned short **s_flashTags      827b6554     g_scr_vehicle.obj

VehicleLocalPhysics s_phys_0;
VehiclePhysicsBackup s_backup_0;
vehicle_info_t s_vehicleInfos[32];
VehicleLocalPhysics s_phys;

VehiclePhysicsBackup s_backup;

#ifdef KISAK_SP
__int16 s_numVehicleInfos;

unsigned __int16 *s_flashTags[5] =
{
    &scr_const.tag_flash,
    &scr_const.tag_flash_11,
    &scr_const.tag_flash_2,
    &scr_const.tag_flash_22,
    &scr_const.tag_flash_3
};
unsigned __int16 *s_wheelTags[6] =
{
    &scr_const.tag_wheel_front_left,
    &scr_const.tag_wheel_front_right,
    &scr_const.tag_wheel_back_left,
    &scr_const.tag_wheel_back_right,
    &scr_const.tag_wheel_middle_left,
    &scr_const.tag_wheel_middle_right,
};
cspField_t s_vehicleFields[33] =
{
  { "type", 64, 12 },
  { "steerWheels", 68, 5 },
  { "texureScroll", 72, 5 },
  { "quadBarrel", 76, 5 },
  { "bulletDamage", 80, 5 },
  { "armorPiercingDamage", 84, 5 },
  { "grenadeDamage", 88, 5 },
  { "projectileDamage", 92, 5 },
  { "projectileSplashDamage", 96, 5 },
  { "heavyExplosiveDamage", 100, 5 },
  { "texureScrollScale", 104, 6 },
  { "maxSpeed", 108, 6 },
  { "accel", 112, 6 },
  { "rotRate", 116, 6 },
  { "rotAccel", 120, 6 },
  { "collisionDamage", 132, 6 },
  { "collisionSpeed", 136, 6 },
  { "suspensionTravel", 140, 6 },
  { "maxBodyPitch", 124, 6 },
  { "maxBodyRoll", 128, 6 },
  { "turretWeapon", 144, 0 },
  { "turretHorizSpanLeft", 208, 6 },
  { "turretHorizSpanRight", 212, 6 },
  { "turretVertSpanUp", 216, 6 },
  { "turretVertSpanDown", 220, 6 },
  { "turretRotRate", 224, 6 },
  { "lowIdleSnd", 228, 0 },
  { "highIdleSnd", 292, 0 },
  { "lowEngineSnd", 356, 0 },
  { "highEngineSnd", 420, 0 },
  { "turretSpinSnd", 484, 0 },
  { "turretStopSnd", 548, 0 },
  { "engineSndSpeed", 624, 6 }
};


#endif

gentity_s *__cdecl GScr_GetVehicle(scr_entref_t entref)
{
    if (!entref.classnum)
        return VEH_GetVehicle(entref.entnum);
    Scr_ObjectError("not an entity");
    return 0;
}

gentity_s *__cdecl VEH_GetVehicle(int32_t entNum)
{
    const char *v1; // eax
    const char *v2; // eax

    if (entNum >= 1024)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 453, 0, "%s", "entNum < MAX_GENTITIES");
    if (g_entities[entNum].classname != scr_const.script_vehicle)
    {
        v1 = va("entity %i is not a script_vehicle\n", entNum);
        Scr_Error(v1);
    }
    if (!g_entities[entNum].scr_vehicle)
    {
        v2 = va("entity %i doesn't have a script_vehicle\n", entNum);
        Scr_Error(v2);
    }
    return &g_entities[entNum];
}

void __cdecl VEH_InitEntity(gentity_s *ent, scr_vehicle_s *veh, int32_t infoIdx)
{
    ent->handler = 23;
    ent->r.svFlags = 4;
    ent->r.contents = 8320;
    ent->s.lerp.eFlags = 0;
    ent->s.lerp.pos.trType = TR_INTERPOLATE;
    ent->s.lerp.apos.trType = TR_INTERPOLATE;
    ent->s.lerp.u.vehicle.materialTime = 0;
    ent->s.time2 = 0;
    ent->s.loopSound = 0;
    ent->s.weapon = (uint8_t)G_GetWeaponIndexForName(s_vehicleInfos[infoIdx].turretWeapon);
    ent->s.weaponModel = 0;
    ent->s.lerp.u.turret.gunAngles[2] = 0.0;
    ent->s.lerp.u.turret.gunAngles[0] = 0.0;
    ent->s.lerp.u.turret.gunAngles[1] = 0.0;
    ent->s.lerp.u.primaryLight.cosHalfFovInner = 0.0;
    ent->s.lerp.u.vehicle.gunYaw = 0.0;
    ent->scr_vehicle = veh;
    ent->nextthink = level.time + 50;
    ent->takedamage = 1;
    ent->active = 1;
    ent->clipmask = 0x810211;
    SV_DObjGetBounds(ent, ent->r.mins, ent->r.maxs);
    SV_LinkEntity(ent);
}

void __cdecl VEH_InitVehicle(gentity_s *ent, scr_vehicle_s *veh, __int16 infoIdx)
{
    VEH_InitPhysics(ent);
    veh->entNum = ent->s.number;
    veh->infoIdx = infoIdx;
    veh->moveState = VEH_MOVESTATE_STOP;
    veh->waitNode = -1;
    veh->waitSpeed = -1.0f;
    veh->turret.fireTime = 0;
    veh->turret.fireBarrel = 0;
    veh->turret.turretState = VEH_TURRET_STOPPED;
    veh->turret.barrelOffset = 0.0f;
    veh->turret.barrelBlocked = 0;
    Com_Memset(&veh->jitter, 0, sizeof(VehicleJitter));
    veh->drawOnCompass = 0;
    veh->lookAtText0 = 0;
    veh->lookAtText1 = 0;
    veh->manualMode = 0;
    veh->manualSpeed = 0.0f;
    veh->manualAccel = 0.0f;
    veh->manualDecel = 0.0f;
    veh->manualTime = 0.0f;
    veh->speed = 0.0f;
    veh->maxDragSpeed = 17.6f * 60.0f;
    veh->turningAbility = 0.5f;
    veh->hasTarget = 0;
    veh->targetEnt = 1023;
    iassert(!veh->lookAtEnt.isDefined());
    veh->targetOrigin[0] = 0.0f;
    veh->targetOrigin[1] = 0.0f;
    veh->targetOrigin[2] = 0.0f;
    veh->targetOffset[0] = 0.0f;
    veh->targetOffset[1] = 0.0f;
    veh->targetOffset[2] = 0.0f;
    veh->joltDir[0] = 0.0f;
    veh->joltDir[1] = 0.0f;
    veh->joltTime = 0.0f;
    veh->joltWave = 0.0f;
    veh->joltSpeed = 0.0f;
    veh->joltDecel = 0.0f;
    veh->playEngineSound = 1;
    iassert(!veh->idleSndEnt.isDefined());
    iassert(!veh->engineSndEnt.isDefined());
    veh->idleSndLerp = 0.0f;
    veh->engineSndLerp = 0.0f;
    veh->turretHitNum = 0;
    veh->hover.hoverRadius = 30.0f;
    veh->hover.hoverSpeed = 17.6f * 0.800000011920929f;
    veh->hover.hoverAccel = 17.6f * 0.5f;
    veh->hover.useHoverAccelForAngles = 0;
    veh->yawSlowDown = 0;
    veh->yawOverShoot = 0.1f;
    veh->prevGoalYaw = -1.0f;
    veh->nearGoalNotifyDist = 0.0f;
    VEH_SetPosition(ent, ent->r.currentOrigin, vec3_origin, ent->r.currentAngles);
}

void __cdecl VEH_InitPhysics(gentity_s *ent)
{
    vehicle_physic_t *phys; // [esp+38h] [ebp-Ch]
    int32_t i; // [esp+40h] [ebp-4h]

    phys = &ent->scr_vehicle->phys;
    phys->origin[0] = ent->r.currentOrigin[0];
    phys->origin[1] = ent->r.currentOrigin[1];
    phys->origin[2] = ent->r.currentOrigin[2];
    phys->prevOrigin[0] = ent->r.currentOrigin[0];
    phys->prevOrigin[1] = ent->r.currentOrigin[1];
    phys->prevOrigin[2] = ent->r.currentOrigin[2];
    phys->angles[0] = ent->r.currentAngles[0];
    phys->angles[1] = ent->r.currentAngles[1];
    phys->angles[2] = ent->r.currentAngles[2];
    phys->prevAngles[0] = ent->r.currentAngles[0];
    phys->prevAngles[1] = ent->r.currentAngles[1];
    phys->prevAngles[2] = ent->r.currentAngles[2];
    phys->mins[0] = 0.0f;
    phys->mins[1] = 0.0f;
    phys->mins[2] = 0.0f;
    phys->maxs[0] = 0.0f;
    phys->maxs[1] = 0.0f;
    phys->maxs[2] = 0.0f;
    phys->vel[0] = 0.0f;
    phys->vel[1] = 0.0f;
    phys->vel[2] = 0.0f;
    phys->bodyVel[0] = 0.0f;
    phys->bodyVel[1] = 0.0f;
    phys->bodyVel[2] = 0.0f;
    phys->rotVel[0] = 0.0f;
    phys->rotVel[1] = 0.0f;
    phys->rotVel[2] = 0.0f;
    for (i = 0; i < 4; ++i)
    {
        phys->wheelZVel[i] = 0.0f;
        phys->wheelZPos[i] = 0.0f;
        phys->wheelSurfType[i] = 0;
    }
    phys->maxPitchAngle = 25.0f;
    phys->maxRollAngle = 25.0f;
    phys->maxAngleVel[1] = 90.0f;
    phys->maxAngleVel[0] = 45.0f;
    phys->maxAngleVel[2] = 45.0f;
    phys->yawAccel = 25.0f;
    phys->yawDecel = 15.0f;
    phys->worldTilt[0] = 0.0f;
    phys->worldTilt[1] = 0.0f;
    phys->worldTilt[2] = 0.0f;
    phys->worldTiltVel[0] = 0.0f;
    phys->worldTiltVel[1] = 0.0f;
    phys->worldTiltVel[2] = 0.0f;
}

const float s_correctSolidDeltas[26][3] =
{
  { 0.0f, 0.0f, 1.0f },
  { -1.0f, 0.0f, 1.0f },
  { 0.0f, -1.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f },
  { 0.0f, 1.0f, 1.0f },
  { -1.0f, 0.0f, 0.0f },
  { 0.0f, -1.0f, 0.0f },
  { 1.0f, 0.0f, 0.0f },
  { 0.0f, 1.0f, 0.0f },
  { 0.0f, 0.0f, -1.0f },
  { -1.0f, 0.0f, -1.0f },
  { 0.0f, -1.0f, -1.0f },
  { 1.0f, 0.0f, -1.0f },
  { 0.0f, 1.0f, -1.0f },
  { -1.0f, -1.0f, 1.0f },
  { 1.0f, -1.0f, 1.0f },
  { 1.0f, 1.0f, 1.0f },
  { -1.0f, 1.0f, 1.0f },
  { -1.0f, -1.0f, 0.0f },
  { 1.0f, -1.0f, 0.0f },
  { 1.0f, 1.0f, 0.0f },
  { -1.0f, 1.0f, 0.0f },
  { -1.0f, -1.0f, -1.0f },
  { 1.0f, -1.0f, -1.0f },
  { 1.0f, 1.0f, -1.0f },
  { -1.0f, 1.0f, -1.0f }
}; // idb
int32_t __cdecl VEH_CorrectAllSolid(gentity_s *ent, trace_t *trace)
{
    vehicle_physic_t *phys; // [esp+10h] [ebp-18h]
    uint32_t i; // [esp+18h] [ebp-10h]
    float point[3]; // [esp+1Ch] [ebp-Ch] BYREF

    phys = &ent->scr_vehicle->phys;
    for (i = 0; i < 0x1A; ++i)
    {
        Vec3Add(phys->origin, (const float*)&s_correctSolidDeltas[i], point);
        G_TraceCapsule(trace, point, phys->mins, phys->maxs, point, ent->s.number, ent->clipmask);
        if (!trace->startsolid)
        {
            phys->origin[0] = point[0];
            phys->origin[1] = point[1];
            phys->origin[2] = point[2];
            point[0] = phys->origin[0];
            point[1] = phys->origin[1];
            point[2] = phys->origin[2] - 1.0f;
            G_TraceCapsule(trace, phys->origin, phys->mins, phys->maxs, point, ent->s.number, ent->clipmask);
            memcpy(&s_phys, trace, 0x2Cu);
            Vec3Lerp(phys->origin, point, trace->fraction, phys->origin);
            return 1;
        }
    }
    return 0;
}

void __cdecl VEH_ClearGround()
{
    s_phys.hasGround = 0;
}

bool __cdecl VEH_SlideMove(gentity_s *ent, int32_t gravity)
{
    float *v3; // [esp+24h] [ebp-D4h]
    float timeLeft; // [esp+30h] [ebp-C8h]
    int32_t j; // [esp+34h] [ebp-C4h]
    vehicle_physic_t *phys; // [esp+38h] [ebp-C0h]
    float dir[3]; // [esp+3Ch] [ebp-BCh] BYREF
    int32_t bumpCount; // [esp+48h] [ebp-B0h]
    scr_vehicle_s *veh; // [esp+4Ch] [ebp-ACh]
    int32_t k; // [esp+50h] [ebp-A8h]
    float planes[5][3]; // [esp+54h] [ebp-A4h] BYREF
    float clipVel[3]; // [esp+90h] [ebp-68h] BYREF
    float end[3]; // [esp+9Ch] [ebp-5Ch] BYREF
    float endVel[3]; // [esp+A8h] [ebp-50h] BYREF
    int32_t numPlanes; // [esp+B4h] [ebp-44h]
    trace_t trace; // [esp+B8h] [ebp-40h] BYREF
    float endClipVel[3]; // [esp+E4h] [ebp-14h] BYREF
    int32_t i; // [esp+F0h] [ebp-8h]
    float dot; // [esp+F4h] [ebp-4h]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    timeLeft = 0.050000001f;
    endVel[0] = veh->phys.vel[0];
    endVel[1] = veh->phys.vel[1];
    endVel[2] = veh->phys.vel[2];
    if (gravity)
    {
        endVel[2] = endVel[2] - 40.0f;
        veh->phys.vel[2] = (veh->phys.vel[2] + endVel[2]) * 0.5f;
        if (s_phys.hasGround)
            VEH_ClipVelocity(phys->vel, s_phys.groundTrace.normal, phys->vel);
    }
    if (s_phys.hasGround)
    {
        numPlanes = 1;
        planes[0][0] = s_phys.groundTrace.normal[0];
        planes[0][1] = s_phys.groundTrace.normal[1];
        planes[0][2] = s_phys.groundTrace.normal[2];
    }
    else
    {
        numPlanes = 0;
    }
    Vec3NormalizeTo(phys->vel, planes[numPlanes++]);
    for (bumpCount = 0; bumpCount < 4; ++bumpCount)
    {
        Vec3Mad(phys->origin, timeLeft, phys->vel, end);
        G_TraceCapsule(&trace, phys->origin, phys->mins, phys->maxs, end, ent->s.number, ent->clipmask);
        if (trace.startsolid)
        {
            phys->vel[2] = 0.0f;
            return 1;
        }
        if (trace.fraction > 0.0f)
            Vec3Lerp(phys->origin, end, trace.fraction, phys->origin);
        if (trace.fraction == 1.0f)
            break;
        timeLeft = timeLeft - timeLeft * trace.fraction;
        if (numPlanes >= 5)
        {
            phys->vel[0] = 0.0f;
            phys->vel[1] = 0.0f;
            phys->vel[2] = 0.0f;
            return 1;
        }
        for (i = 0; i < numPlanes; ++i)
        {
            if (Vec3Dot(trace.normal, planes[i]) > 0.9900000095367432f)
            {
                Vec3Add(trace.normal, phys->vel, phys->vel);
                break;
            }
        }
        if (i >= numPlanes)
        {
            v3 = planes[numPlanes];
            *v3 = trace.normal[0];
            v3[1] = trace.normal[1];
            v3[2] = trace.normal[2];
            ++numPlanes;
            for (i = 0; i < numPlanes; ++i)
            {
                if (Vec3Dot(phys->vel, planes[i]) < 0.1000000014901161f)
                {
                    VEH_ClipVelocity(phys->vel, planes[i], clipVel);
                    VEH_ClipVelocity(endVel, planes[i], endClipVel);
                    for (j = 0; j < numPlanes; ++j)
                    {
                        if (j != i && Vec3Dot(clipVel, planes[j]) < 0.1000000014901161f)
                        {
                            VEH_ClipVelocity(clipVel, planes[j], clipVel);
                            VEH_ClipVelocity(endClipVel, planes[j], endClipVel);
                            if (Vec3Dot(clipVel, planes[i]) < 0.0f)
                            {
                                Vec3Cross(planes[i], planes[j], dir);
                                Vec3Normalize(dir);
                                dot = Vec3Dot(dir, phys->vel);
                                Vec3Scale(dir, dot, clipVel);
                                dot = Vec3Dot(dir, endVel);
                                Vec3Scale(dir, dot, endClipVel);
                                for (k = 0; k < numPlanes; ++k)
                                {
                                    if (k != i && k != j && Vec3Dot(clipVel, planes[k]) < 0.1000000014901161f)
                                    {
                                        phys->vel[0] = 0.0f;
                                        phys->vel[1] = 0.0f;
                                        phys->vel[2] = 0.0f;
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                    phys->vel[0] = clipVel[0];
                    phys->vel[1] = clipVel[1];
                    phys->vel[2] = clipVel[2];
                    endVel[0] = endClipVel[0];
                    endVel[1] = endClipVel[1];
                    endVel[2] = endClipVel[2];
                    break;
                }
            }
        }
    }
    if (gravity)
    {
        phys->vel[0] = endVel[0];
        phys->vel[1] = endVel[1];
        phys->vel[2] = endVel[2];
    }
    return bumpCount != 0;
}

void __cdecl VEH_ClipVelocity(float *in, float *normal, float *out)
{
    double v3; // st7
    float change; // [esp+0h] [ebp-Ch]
    int32_t i; // [esp+4h] [ebp-8h]
    float backoff; // [esp+8h] [ebp-4h]
    float backoffa; // [esp+8h] [ebp-4h]

    if (normal[2] < 0.699999988079071f || in[2] * in[2] > *in * *in + in[1] * in[1])
    {
        backoff = Vec3Dot(in, normal);
        if (backoff >= 0.0)
            v3 = backoff / 1.009999990463257f;
        else
            v3 = backoff * 1.009999990463257f;
        for (i = 0; i < 3; ++i)
        {
            backoffa = v3;
            change = normal[i] * backoffa;
            out[i] = in[i] - change;
        }
    }
    else
    {
        out[2] = -*in * *normal - in[1] * normal[1];
        *out = *in * normal[2];
        out[1] = in[1] * normal[2];
    }
}

void __cdecl Scr_Vehicle_Think(gentity_s *pSelf)
{
    vehicle_info_t *info; // [esp+4h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 4417, 0, "%s", "pSelf");
    if (!pSelf->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 4418, 0, "%s", "pSelf->scr_vehicle");
    veh = pSelf->scr_vehicle;
    info = &s_vehicleInfos[veh->infoIdx];
    VEH_BackupPosition(pSelf);
    memset((uint8_t *)&s_phys, 0, sizeof(s_phys));
    if (info->type != 4)
    {
        if ((veh->flags & 2) != 0)
        {
            VEH_UpdateAIMove(pSelf);
        }
        else if (info->type == 5)
        {
            VEH_UpdateClientChopper(pSelf);
        }
    }
    if (veh->speed < 0.0)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 4457, 0, "%s", "veh->speed >= 0.0f");
    VEH_SetPosition(pSelf, veh->phys.origin, veh->phys.vel, veh->phys.angles);
    if ((veh->flags & 2) != 0)
        VEH_MoveTrace(pSelf);
    VEH_TouchEntities(pSelf);
    veh->turret.barrelBlocked = 0;
    VEH_UpdateAim(pSelf);
    pSelf->s.time2 = (int)(info->suspensionTravel * 1000.0f);
    pSelf->nextthink = level.time + 50;
}

void __cdecl VEH_MoveTrace(gentity_s *ent)
{
    float diff[3]; // [esp+8h] [ebp-5Ch] BYREF
    int32_t contents; // [esp+14h] [ebp-50h]
    vehicle_physic_t *phys; // [esp+18h] [ebp-4Ch]
    float size[3]; // [esp+1Ch] [ebp-48h] BYREF
    int32_t dimIndex; // [esp+28h] [ebp-3Ch]
    trace_t trace; // [esp+2Ch] [ebp-38h] BYREF
    float maxs[3]; // [esp+58h] [ebp-Ch] BYREF

    if (Scr_IsSystemActive())
    {
        phys = &ent->scr_vehicle->phys;
        Vec3Sub(phys->origin, phys->prevOrigin, diff);
        if (Vec3LengthSq(diff) != 0.0)
        {
            contents = 8321;
            maxs[0] = ent->r.maxs[0];
            maxs[1] = ent->r.maxs[1];
            maxs[2] = ent->r.maxs[2];
            Vec3Sub(maxs, ent->r.mins, size);
            dimIndex = size[1] > (float)size[0];
            if (size[dimIndex] > (float)size[2])
                maxs[2] = size[dimIndex] - size[2] + maxs[2];
            G_TraceCapsule(&trace, phys->prevOrigin, ent->r.mins, maxs, phys->origin, ent->s.number, contents);
            if (trace.fraction < 1.0f && Trace_GetEntityHitId(&trace) == 1022)
            {
                Scr_AddVector(trace.normal);
                Scr_Notify(ent, scr_const.script_vehicle_collision, 1u);
            }
        }
    }
}

void __cdecl VEH_BackupPosition(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+18h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1708, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1709, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    veh->phys.prevOrigin[0] = ent->r.currentOrigin[0];
    veh->phys.prevOrigin[1] = ent->r.currentOrigin[1];
    veh->phys.prevOrigin[2] = ent->r.currentOrigin[2];
    veh->phys.prevAngles[0] = ent->r.currentAngles[0];
    veh->phys.prevAngles[1] = ent->r.currentAngles[1];
    veh->phys.prevAngles[2] = ent->r.currentAngles[2];
    memcpy(&s_backup, veh, 0xC0u);
    memcpy(&s_backup.phys, &veh->phys, sizeof(s_backup.phys));
}

void __cdecl VEH_TouchEntities(gentity_s *ent)
{
    int32_t v1; // [esp+Ch] [ebp-1080h]
    float maxs[3]; // [esp+18h] [ebp-1074h] BYREF
    DObj_s *obj; // [esp+24h] [ebp-1068h]
    scr_vehicle_s *scr_vehicle; // [esp+28h] [ebp-1064h]
    float b[3]; // [esp+2Ch] [ebp-1060h] BYREF
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+38h] [ebp-1054h]
    float v7; // [esp+3Ch] [ebp-1050h]
    gentity_s *target; // [esp+40h] [ebp-104Ch]
    float out[3]; // [esp+44h] [ebp-1048h] BYREF
    float v3[3]; // [esp+50h] [ebp-103Ch] BYREF
    float sum[3]; // [esp+5Ch] [ebp-1030h] BYREF
    void(__cdecl * v12)(gentity_s *, gentity_s *, int); // [esp+68h] [ebp-1024h]
    int32_t entityList[1025]; // [esp+6Ch] [ebp-1020h] BYREF
    int32_t i; // [esp+1070h] [ebp-1Ch]
    float mins[3]; // [esp+1074h] [ebp-18h] BYREF
    float diff[3]; // [esp+1080h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1916, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1917, 0, "%s", "ent->scr_vehicle");
    scr_vehicle = ent->scr_vehicle;
    touch = entityHandlers[ent->handler].touch;
    Vec3Sub(scr_vehicle->phys.origin, scr_vehicle->phys.prevOrigin, diff);
    AnglesSubtract(scr_vehicle->phys.angles, scr_vehicle->phys.prevAngles, v3);
    Vec3NormalizeTo(scr_vehicle->phys.vel, out);
    v7 = RadiusFromBounds(ent->r.mins, ent->r.maxs);
    b[0] = -v7;
    b[1] = b[0];
    b[2] = b[0];
    sum[0] = v7;
    sum[1] = v7;
    sum[2] = v7;
    Vec3Add(scr_vehicle->phys.prevOrigin, b, b);
    Vec3Add(scr_vehicle->phys.prevOrigin, sum, sum);
    ExtendBounds(b, sum, diff);
    v1 = CM_AreaEntities(b, sum, entityList, 1024, 0x2806081);
    for (i = 0; i < v1; ++i)
    {
        target = &g_entities[entityList[i]];
        v12 = entityHandlers[target->handler].touch;
        if (target->s.number != ent->s.number
            && (target->s.eType == 1 || target->s.eType == 6 || target->s.eType == 4 || target->s.eType == 14)
            && target->s.eType == 4)
        {
            if (target->s.groundEntityNum == ent->s.number)
            {
                VEH_PushEntity(ent, target, out, diff, v3);
                continue;
            }
            if (ent->r.bmodel)
            {
                if (target->classname == scr_const.script_model)
                {
                    if (!target->model)
                        continue;
                    obj = Com_GetServerDObj(target->s.number);
                    DObjPhysicsGetBounds(obj, mins, maxs);
                    Vec3Add(target->r.currentOrigin, mins, mins);
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
                    if (v12)
                        v12(target, ent, 1);
                    if (touch)
                        touch(ent, target, 1);
                    if (target->s.eType == 1)
                        VEH_PushEntity(ent, target, out, diff, v3);
                }
            }
        }
    }
}

void __cdecl VEH_PushEntity(gentity_s *ent, gentity_s *target, float *pushDir, float *deltaOrigin, float *deltaAngles)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1863, 0, "%s", "ent");
    if (!target)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1864, 0, "%s", "target");
    if (!target->tagInfo
        && (Vec3LengthSq(deltaOrigin) >= EQUAL_EPSILON || Vec3LengthSq(deltaAngles) >= EQUAL_EPSILON)
        && AttachedStickyMissile(ent, target))
    {
        PushAttachedStickyMissile(ent, target);
    }
}

bool __cdecl AttachedStickyMissile(gentity_s *vehicle, gentity_s *missile)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1801, 0, "%s", "vehicle");
    if (!missile)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1802, 0, "%s", "missile");
    if (missile->s.groundEntityNum != vehicle->s.number)
        return 0;
    if (missile->s.eType != 4)
        return 0;
    weapDef = BG_GetWeaponDef(missile->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1810, 0, "%s", "weapDef");
    return weapDef->stickiness == WEAPSTICKINESS_ALL;
}

void __cdecl PushAttachedStickyMissile(gentity_s *vehicle, gentity_s *missile)
{
    scr_vehicle_s *scr_vehicle; // edx
    float newVehMat[3][3]; // [esp+10h] [ebp-F4h] BYREF
    vehicle_physic_t *phys; // [esp+34h] [ebp-D0h]
    float origin[3]; // [esp+38h] [ebp-CCh] BYREF
    float oldMissileMat[3][3]; // [esp+44h] [ebp-C0h] BYREF
    float newMissileMat[3][3]; // [esp+68h] [ebp-9Ch] BYREF
    float oldVehMatInv[3][3]; // [esp+8Ch] [ebp-78h] BYREF
    float deltaMat[3][3]; // [esp+B0h] [ebp-54h] BYREF
    float oldVehMat[3][3]; // [esp+D4h] [ebp-30h] BYREF
    float relativeOrig[3]; // [esp+F8h] [ebp-Ch] BYREF

    if (!vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1830, 0, "%s", "vehicle");
    if (!vehicle->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1831, 0, "%s", "vehicle->scr_vehicle");
    if (!missile)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1832, 0, "%s", "missile");
    scr_vehicle = vehicle->scr_vehicle;
    phys = &scr_vehicle->phys;
    if (scr_vehicle == (scr_vehicle_s *)-192)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 1835, 0, "%s", "phys");
    Vec3Sub(missile->r.currentOrigin, phys->prevOrigin, relativeOrig);
    AnglesToAxis(phys->angles, newVehMat);
    AnglesToAxis(phys->prevAngles, oldVehMat);
    MatrixTranspose(oldVehMat, oldVehMatInv);
    MatrixMultiply(oldVehMatInv, newVehMat, deltaMat);
    MatrixTransformVector(relativeOrig, deltaMat, origin);
    Vec3Add(origin, phys->origin, origin);
    missile->r.currentOrigin[0] = origin[0];
    missile->r.currentOrigin[1] = origin[1];
    missile->r.currentOrigin[2] = origin[2];
    missile->s.lerp.pos.trBase[0] = origin[0];
    missile->s.lerp.pos.trBase[1] = origin[1];
    missile->s.lerp.pos.trBase[2] = origin[2];
    AnglesToAxis(missile->r.currentAngles, oldMissileMat);
    MatrixMultiply(oldMissileMat, deltaMat, newMissileMat);
    AxisToAngles(newMissileMat, missile->r.currentAngles);
    missile->s.lerp.apos.trBase[0] = missile->r.currentAngles[0];
    missile->s.lerp.apos.trBase[1] = missile->r.currentAngles[1];
    missile->s.lerp.apos.trBase[2] = missile->r.currentAngles[2];
}

void __cdecl VEH_UpdateAim(gentity_s *ent)
{
    float v1; // [esp+10h] [ebp-180h]
    float v2; // [esp+14h] [ebp-17Ch]
    float v3; // [esp+18h] [ebp-178h]
    float v4; // [esp+1Ch] [ebp-174h]
    float v5; // [esp+20h] [ebp-170h]
    float v6; // [esp+24h] [ebp-16Ch]
    float v7; // [esp+28h] [ebp-168h]
    float v8; // [esp+2Ch] [ebp-164h]
    float gunYaw; // [esp+38h] [ebp-158h]
    float v10; // [esp+3Ch] [ebp-154h]
    float turretHorizSpanLeft; // [esp+40h] [ebp-150h]
    float v12; // [esp+44h] [ebp-14Ch]
    float cosHalfFovInner; // [esp+48h] [ebp-148h]
    float v14; // [esp+4Ch] [ebp-144h]
    float turretVertSpanDown; // [esp+50h] [ebp-140h]
    float v16; // [esp+54h] [ebp-13Ch]
    gentity_s *player; // [esp+80h] [ebp-110h]
    vehicle_info_t *info; // [esp+88h] [ebp-108h]
    float mtx[3][3]; // [esp+8Ch] [ebp-104h] BYREF
    scr_vehicle_s *veh; // [esp+B0h] [ebp-E0h]
    float tgtDir[3]; // [esp+B4h] [ebp-DCh] BYREF
    float tgtAngles[3]; // [esp+C0h] [ebp-D0h] BYREF
    float invMtx[3][3]; // [esp+CCh] [ebp-C4h] BYREF
    gentity_s *tgtEnt; // [esp+F0h] [ebp-A0h]
    float deltaAngles[3]; // [esp+F4h] [ebp-9Ch] BYREF
    float angles[3]; // [esp+100h] [ebp-90h] BYREF
    float stopAngles[3]; // [esp+10Ch] [ebp-84h]
    float turretBaseMat[4][3]; // [esp+118h] [ebp-78h] BYREF
    float barrelPos[3]; // [esp+148h] [ebp-48h] BYREF
    float prevAngles[3]; // [esp+154h] [ebp-3Ch] BYREF
    float axis[3][3]; // [esp+160h] [ebp-30h] BYREF
    float tgtPos[3]; // [esp+184h] [ebp-Ch] BYREF

    veh = ent->scr_vehicle;
    info = &s_vehicleInfos[veh->infoIdx];
    tgtEnt = 0;
    if (veh->hasTarget && ent->health > 0)
    {
        if (veh->targetEnt == 1023)
        {
            tgtEnt = 0;
            tgtPos[0] = veh->targetOrigin[0];
            tgtPos[1] = veh->targetOrigin[1];
            tgtPos[2] = veh->targetOrigin[2];
        }
        else
        {
            tgtEnt = &g_entities[veh->targetEnt];
            Vec3Add(tgtEnt->r.currentOrigin, veh->targetOffset, tgtPos);
        }
        if (veh->boneIndex.barrel >= 0)
        {
            G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.barrel, barrelPos);
            if (veh->boneIndex.turret_base == -1)
            {
                angles[0] = veh->phys.angles[0];
                angles[1] = veh->phys.angles[1];
                angles[2] = veh->phys.angles[2];
            }
            else
            {
                G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.turret_base, turretBaseMat);
                AxisToAngles(*(const mat3x3*)turretBaseMat, angles);
            }
            if (ent->r.ownerNum.isDefined() && veh->targetEnt == ENTITYNUM_NONE)
            {
                player = ent->r.ownerNum.ent();
                if (!player->client)
                    MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2081, 0, "%s", "player->client");
                Vec3Sub(veh->targetOrigin, barrelPos, tgtDir);
                Vec3Normalize(tgtDir);
                vectoangles(tgtDir, tgtAngles);
                tgtAngles[1] = player->client->ps.viewangles[1];
                tgtAngles[2] = 0.0f;
            }
            else
            {
                Vec3Sub(tgtPos, barrelPos, tgtDir);
                Vec3Normalize(tgtDir);
                vectoangles(tgtDir, tgtAngles);
            }
            AnglesToAxis(tgtAngles, axis);
            AnglesToAxis(angles, mtx);
            MatrixTranspose(mtx, invMtx);
            MatrixMultiply(axis, invMtx, mtx);
            AxisToAngles(mtx, tgtAngles);
            prevAngles[0] = ent->s.lerp.u.primaryLight.cosHalfFovInner;
            prevAngles[1] = ent->s.lerp.u.vehicle.gunYaw;
            prevAngles[2] = 0.0f;
            AnglesSubtract(tgtAngles, prevAngles, deltaAngles);
            v8 = I_fabs(deltaAngles[0]);
            deltaAngles[0] = v8;
            v7 = I_fabs(deltaAngles[1]);
            deltaAngles[1] = v7;
            ent->s.lerp.u.primaryLight.cosHalfFovInner = LinearTrackAngle(
                tgtAngles[0],
                prevAngles[0],
                info->turretRotRate,
                0.050000001f);
            ent->s.lerp.u.vehicle.gunYaw = LinearTrackAngle(tgtAngles[1], prevAngles[1], info->turretRotRate, 0.050000001f);
            stopAngles[0] = ent->s.lerp.u.primaryLight.cosHalfFovInner;
            stopAngles[1] = ent->s.lerp.u.vehicle.gunYaw;
            stopAngles[2] = 0.0f;
            cosHalfFovInner = ent->s.lerp.u.primaryLight.cosHalfFovInner;
            turretVertSpanDown = info->turretVertSpanDown;
            v6 = cosHalfFovInner - turretVertSpanDown;
            if (v6 < 0.0f)
                v16 = cosHalfFovInner;
            else
                v16 = turretVertSpanDown;
            v14 = -info->turretVertSpanUp;
            v5 = v14 - cosHalfFovInner;
            if (v5 < 0.0f)
                v4 = v16;
            else
                v4 = -info->turretVertSpanUp;
            ent->s.lerp.u.primaryLight.cosHalfFovInner = v4;
            gunYaw = ent->s.lerp.u.vehicle.gunYaw;
            turretHorizSpanLeft = info->turretHorizSpanLeft;
            v3 = gunYaw - turretHorizSpanLeft;
            if (v3 < 0.0f)
                v12 = gunYaw;
            else
                v12 = turretHorizSpanLeft;
            v10 = -info->turretHorizSpanRight;
            v2 = v10 - gunYaw;
            if (v2 < 0.0f)
                v1 = v12;
            else
                v1 = -info->turretHorizSpanRight;
            ent->s.lerp.u.vehicle.gunYaw = v1;
            stopAngles[0] = AngleDelta(stopAngles[0], ent->s.lerp.u.primaryLight.cosHalfFovInner);
            stopAngles[1] = AngleDelta(stopAngles[1], ent->s.lerp.u.vehicle.gunYaw);
            if (deltaAngles[0] >= 2.0f && stopAngles[0] == 0.0f || deltaAngles[1] >= 2.0f && stopAngles[1] == 0.0f)
            {
                veh->turret.turretState = VEH_TURRET_MOVING;
            }
            else if (veh->turret.turretState == VEH_TURRET_MOVING)
            {
                veh->turret.turretState = VEH_TURRET_STOPPING;
            }
            else if (veh->turret.turretState == VEH_TURRET_STOPPING)
            {
                Scr_Notify(ent, scr_const.turret_rotate_stopped, 0);
                veh->turret.turretState = VEH_TURRET_STOPPED;
            }
            if (deltaAngles[0] >= 1.0f || deltaAngles[1] >= 1.0f)
            {
                Scr_Notify(ent, scr_const.turret_not_on_target, 0);
                Scr_Notify(ent, scr_const.turret_no_vis, 0);
            }
            else
            {
                Scr_Notify(ent, scr_const.turret_on_target, 0);
                if (!tgtEnt
                    || (SV_SightTrace(
                        &veh->turretHitNum,
                        barrelPos,
                        (float *)vec3_origin,
                        (float *)vec3_origin,
                        tgtPos,
                        ent->s.number,
                        tgtEnt->s.number,
                        2049),
                        veh->turretHitNum))
                {
                    Scr_Notify(ent, scr_const.turret_no_vis, 0);
                }
                else
                {
                    Scr_Notify(ent, scr_const.turret_on_vistarget, 0);
                }
            }
        }
    }
    else if (veh->turret.turretState == VEH_TURRET_MOVING)
    {
        veh->turret.turretState = VEH_TURRET_STOPPING;
    }
    else if (veh->turret.turretState == VEH_TURRET_STOPPING)
    {
        veh->turret.turretState = VEH_TURRET_STOPPED;
    }
}

void __cdecl VEH_UpdateAIMove(gentity_s *ent)
{
    VehicleMoveState moveState; // [esp+0h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3448, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3449, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    moveState = veh->moveState;
    if (moveState == VEH_MOVESTATE_MOVE)
    {
        VEH_UpdateMoveToGoal(ent, veh->goalPosition);
    }
    else if (moveState == VEH_MOVESTATE_HOVER)
    {
        VEH_UpdateHover(ent);
    }
}

void __cdecl VEH_UpdateMoveToGoal(gentity_s *ent, const float *goalPos)
{
    double v2; // st7
    float v3; // [esp+10h] [ebp-B8h]
    float v4; // [esp+14h] [ebp-B4h]
    float v5; // [esp+18h] [ebp-B0h]
    float v6; // [esp+1Ch] [ebp-ACh]
    float v7; // [esp+20h] [ebp-A8h]
    float v8; // [esp+24h] [ebp-A4h]
    float v9; // [esp+28h] [ebp-A0h]
    float v10; // [esp+2Ch] [ebp-9Ch]
    float scale; // [esp+30h] [ebp-98h]
    float v12; // [esp+34h] [ebp-94h]
    float v13; // [esp+38h] [ebp-90h]
    float v14; // [esp+48h] [ebp-80h]
    float dotVec[2]; // [esp+54h] [ebp-74h] BYREF
    float dt; // [esp+5Ch] [ebp-6Ch]
    float desiredDir[3]; // [esp+60h] [ebp-68h] BYREF
    float desiredVel[3]; // [esp+6Ch] [ebp-5Ch] BYREF
    float newSpeed; // [esp+78h] [ebp-50h] BYREF
    float accelMax; // [esp+7Ch] [ebp-4Ch] BYREF
    float accelMaxDt; // [esp+80h] [ebp-48h]
    float prevVel[3]; // [esp+84h] [ebp-44h] BYREF
    float accelMagnitude; // [esp+90h] [ebp-38h]
    float averageVel[3]; // [esp+94h] [ebp-34h] BYREF
    float accelVec[3]; // [esp+A0h] [ebp-28h] BYREF
    vehicle_physic_t *phys; // [esp+ACh] [ebp-1Ch]
    scr_vehicle_s *veh; // [esp+B0h] [ebp-18h]
    float vecToGoal[3]; // [esp+B4h] [ebp-14h] BYREF
    float distToGoal; // [esp+C0h] [ebp-8h]
    int32_t hovering; // [esp+C4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3294, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3295, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    hovering = VEH_IsHovering(veh);
    Vec3Sub(goalPos, veh->phys.origin, vecToGoal);
    distToGoal = Vec3Length(vecToGoal);
    if (distToGoal > 0.0f)
    {
        dt = 0.050000001f;
        prevVel[0] = phys->vel[0];
        prevVel[1] = phys->vel[1];
        prevVel[2] = phys->vel[2];
        if (veh->stopAtGoal || hovering)
            dt = VEH_UpdateMove_CheckStop(veh, distToGoal);
        VEH_GetNewSpeedAndAccel(veh, dt, hovering, &newSpeed, &accelMax);
        scale = 1.0 / distToGoal;
        Vec3Scale(vecToGoal, scale, desiredDir);
        Vec3Scale(desiredDir, newSpeed, desiredVel);
        Vec3Sub(desiredVel, prevVel, accelVec);
        if (!veh->stopping && veh->manualSpeed >= (float)veh->speed)
        {
            v10 = desiredVel[1] * prevVel[1] + desiredVel[0] * prevVel[0];
            if (v10 > 0.0f)
            {
                v9 = accelVec[1] * prevVel[1] + accelVec[0] * prevVel[0];
                if (v9 < 0.0f)
                {
                    dotVec[0] = prevVel[0];
                    dotVec[1] = prevVel[1];
                    Vec2Normalize(dotVec);
                    v14 = accelVec[1] * dotVec[1] + accelVec[0] * dotVec[0];
                    dotVec[0] = v14 * dotVec[0];
                    dotVec[1] = v14 * dotVec[1];
                    accelVec[0] = accelVec[0] - dotVec[0];
                    accelVec[1] = accelVec[1] - dotVec[1];
                }
            }
        }
        accelMagnitude = Vec3Length(accelVec);
        accelMaxDt = accelMax * dt;
        if (accelMaxDt < (float)accelMagnitude)
        {
            if (accelMagnitude == 0.0)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3346, 0, "%s", "accelMagnitude");
            v8 = accelMaxDt / accelMagnitude;
            Vec3Scale(accelVec, v8, accelVec);
        }
        if (!hovering)
        {
            VEH_CheckHorizontalVelocityToGoal(veh, vecToGoal, accelMax, accelVec);
            if (vecToGoal[2] != 0.0f)
                VEH_CheckVerticalVelocityToGoal(veh, vecToGoal[2], accelVec);
        }
        Vec3Add(prevVel, accelVec, phys->vel);
        v2 = Vec3Length(phys->vel);
        veh->speed = v2;
        Vec3Lerp(phys->accel, accelVec, 0.5f, phys->accel);
        accelMagnitude = Vec3Length(phys->accel);
        if (accelMaxDt < (float)accelMagnitude && !veh->stopAtGoal)
        {
            v7 = accelMaxDt / accelMagnitude;
            Vec3Scale(phys->accel, v7, phys->accel);
        }
        Vec3Add(prevVel, phys->vel, averageVel);
        Vec3Scale(averageVel, 0.5f, averageVel);
        if (dt < 0.05000000074505806f)
        {
            v6 = 0.05000000074505806f - dt;
            Vec3Mad(phys->origin, v6, prevVel, phys->origin);
        }
        Vec3Mad(phys->origin, dt, averageVel, phys->origin);
        VEH_UpdateMoveOrientation(ent, desiredDir);
    }
    if (s_vehicleInfos[veh->infoIdx].engineSndSpeed == 0.0f)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3386, 0, "%s", "s_vehicleInfos[veh->infoIdx].engineSndSpeed");
    v12 = veh->speed / s_vehicleInfos[veh->infoIdx].engineSndSpeed;
    v5 = v12 - 1.0f;
    if (v5 < 0.0f)
        v13 = veh->speed / s_vehicleInfos[veh->infoIdx].engineSndSpeed;
    else
        v13 = 1.0f;
    v4 = 0.0f - v12;
    if (v4 < 0.0f)
        v3 = v13;
    else
        v3 = 0.0f;
    veh->engineSndLerp = v3;
    veh->idleSndLerp = 1.0f - veh->engineSndLerp;
    if (!hovering)
    {
        VEH_UpdateMove_CheckNearGoal(ent, distToGoal);
        VEH_UpdateMove_CheckGoalReached(ent, distToGoal);
    }
}

bool __cdecl VEH_IsHovering(scr_vehicle_s *veh)
{
    if (!veh)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2635, 0, "%s", "veh");
    return veh->moveState == VEH_MOVESTATE_HOVER;
}

void __cdecl VEH_UpdateMoveOrientation(gentity_s *ent, float *desiredDir)
{
    float v2; // [esp+14h] [ebp-7Ch]
    float desiredAngle; // [esp+18h] [ebp-78h]
    float timeToGoal; // [esp+3Ch] [ebp-54h]
    float stoppingTime; // [esp+40h] [ebp-50h]
    vehicle_physic_t *phys; // [esp+44h] [ebp-4Ch]
    float angularDecel; // [esp+48h] [ebp-48h]
    scr_vehicle_s *veh; // [esp+4Ch] [ebp-44h]
    float angularAccel; // [esp+50h] [ebp-40h]
    float accelFraction; // [esp+54h] [ebp-3Ch]
    float accelFractiona; // [esp+54h] [ebp-3Ch]
    float angle; // [esp+58h] [ebp-38h]
    float horizontalAccel; // [esp+5Ch] [ebp-34h]
    float bodyDir; // [esp+60h] [ebp-30h]
    float bodyDir_4; // [esp+64h] [ebp-2Ch]
    float desiredYaw; // [esp+68h] [ebp-28h]
    float accelVec[3]; // [esp+6Ch] [ebp-24h] BYREF
    float accelFactor; // [esp+78h] [ebp-18h]
    float perpDir[2]; // [esp+7Ch] [ebp-14h]
    float dot; // [esp+84h] [ebp-Ch]
    float accel; // [esp+88h] [ebp-8h]
    float stoppingFactor; // [esp+8Ch] [ebp-4h]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    desiredYaw = VEH_UpdateMove_GetDesiredYaw(veh, desiredDir);
    VEH_UpdateYawAndNotify(ent, desiredYaw);
    accelVec[0] = veh->phys.accel[0];
    accelVec[1] = veh->phys.accel[1];
    accelVec[2] = veh->phys.accel[2];
    VEH_AddFakeDrag(veh->phys.vel, veh->maxDragSpeed, accelVec);
    horizontalAccel = Vec2Length(accelVec) / 0.05000000074505806f;
    Vec3Normalize(accelVec);
    angle = veh->phys.angles[1] * 0.01745329238474369f;
    bodyDir = cos(angle);
    bodyDir_4 = sin(angle);
    accelFraction = VEH_CalcAccelFraction(horizontalAccel, veh->infoIdx);
    stoppingFactor = 1.0f;
    if (veh->stopping && horizontalAccel > 0.0f)
    {
        if (horizontalAccel == 0.0f)
            MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2984, 0, "%s", "horizontalAccel");
        timeToGoal = Vec2Length(veh->phys.vel) / horizontalAccel;
        stoppingTime = VEH_CalcStoppingTime(horizontalAccel, accelFraction);
        if (stoppingTime == 0.0f)
            MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2988, 0, "%s", "stoppingTime");
        if (stoppingTime > timeToGoal)
            stoppingFactor = timeToGoal / stoppingTime;
    }
    dot = accelVec[1] * bodyDir_4 + accelVec[0] * bodyDir;
    dot = dot * stoppingFactor;
    accelFactor = VEH_CalcAngleForAccel(horizontalAccel, accelFraction);
    accel = VEH_GetAccelForAngles(veh);
    accelFractiona = VEH_CalcAccelFraction(accel, veh->infoIdx);
    angularAccel = VEH_CalcAngularAccel(accel, accelFractiona);
    angularDecel = angularAccel * 0.4000000059604645f;
    desiredAngle = veh->phys.maxPitchAngle * dot * accelFactor;
    VEH_UpdateAngleAndAngularVel(0, desiredAngle, angularAccel, angularDecel, 0.0f, phys);
    perpDir[0] = bodyDir_4;
    perpDir[1] = -bodyDir;
    dot = accelVec[1] * perpDir[1] + accelVec[0] * bodyDir_4;
    dot = dot * stoppingFactor;
    v2 = veh->phys.maxRollAngle * dot * accelFactor;
    VEH_UpdateAngleAndAngularVel(2, v2, angularAccel, angularDecel, 0.0f, phys);
}

void __cdecl VEH_UpdateAngleAndAngularVel(
    int32_t index,
    float desiredAngle,
    float accel,
    float decel,
    float overShoot,
    vehicle_physic_t *phys)
{
    float v6; // [esp+10h] [ebp-40h]
    float v7; // [esp+14h] [ebp-3Ch]
    float v8; // [esp+18h] [ebp-38h]
    float v9; // [esp+1Ch] [ebp-34h]
    float v10; // [esp+20h] [ebp-30h]
    float v11; // [esp+24h] [ebp-2Ch]
    float v12; // [esp+2Ch] [ebp-24h]
    float stopAngle; // [esp+38h] [ebp-18h]
    float stopAnglea; // [esp+38h] [ebp-18h]
    float stopTime; // [esp+3Ch] [ebp-14h]
    float effectiveAccel; // [esp+44h] [ebp-Ch]
    float targetAngleVel; // [esp+48h] [ebp-8h]
    float angleDiff; // [esp+4Ch] [ebp-4h]

    angleDiff = AngleDelta(desiredAngle, phys->angles[index]);
    if (angleDiff != 0.0f || phys->rotVel[index] != 0.0f)
    {
        v11 = I_fabs(phys->rotVel[index]);
        targetAngleVel = phys->maxAngleVel[index];
        if (angleDiff * phys->rotVel[index] < 0.0f)
        {
            effectiveAccel = accel;
        }
        else
        {
            if (decel == 0.0f)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2662, 0, "%s", "decel");
            if (overShoot < 0.0f || overShoot >= 1.0f)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2666, 0, "%s", "overShoot >= 0.f && overShoot < 1.f");
            stopTime = v11 / decel;
            stopAngle = v11 * 0.5f * stopTime;
            stopAnglea = (1.0f - overShoot) * stopAngle;
            v10 = I_fabs(angleDiff);
            if (stopAnglea < v10)
            {
                effectiveAccel = accel;
            }
            else
            {
                targetAngleVel = 0.0f;
                effectiveAccel = decel;
            }
        }
        if (angleDiff < 0.0f)
            targetAngleVel = -targetAngleVel;
        if (v11 >= effectiveAccel * 0.05000000074505806f
            || (v9 = v11 * 0.05000000074505806f, v8 = I_fabs(angleDiff), v8 >= v9))
        {
            phys->rotVel[index] = VEH_AccelerateSpeed(phys->rotVel[index], targetAngleVel, effectiveAccel, 0.050000001f);
            phys->angles[index] = phys->rotVel[index] * 0.05000000074505806f + phys->angles[index];
            v12 = phys->angles[index] * 0.002777777845039964f;
            v7 = v12 + 0.5f;
            v6 = floor(v7);
            phys->angles[index] = (v12 - v6) * 360.0f;
        }
        else
        {
            phys->angles[index] = desiredAngle;
            phys->rotVel[index] = 0.0f;
        }
    }
}

float __cdecl VEH_AccelerateSpeed(float speed, float tgtSpeed, float accel, float dt)
{
    float speeda; // [esp+8h] [ebp+8h]

    if (tgtSpeed <= (double)speed)
    {
        speeda = speed - accel * dt;
        if (tgtSpeed > (double)speeda)
            return tgtSpeed;
    }
    else
    {
        speeda = accel * dt + speed;
        if (tgtSpeed < (double)speeda)
            return tgtSpeed;
    }
    return speeda;
}

float __cdecl VEH_UpdateMove_GetDesiredYaw(scr_vehicle_s *veh, float *desiredDir)
{
    double v2; // st7
    float v4; // [esp+8h] [ebp-44h]
    float v5; // [esp+Ch] [ebp-40h]
    float v6; // [esp+18h] [ebp-34h]
    float v7; // [esp+1Ch] [ebp-30h]
    float v8; // [esp+20h] [ebp-2Ch]
    float stopAngle; // [esp+24h] [ebp-28h]
    float timeToTurn; // [esp+28h] [ebp-24h]
    float timeToStop; // [esp+2Ch] [ebp-20h]
    float angleDiff; // [esp+30h] [ebp-1Ch]
    float diff[3]; // [esp+34h] [ebp-18h] BYREF
    gentity_s *lookAtEnt; // [esp+40h] [ebp-Ch]
    vehicle_physic_t *phys; // [esp+44h] [ebp-8h]
    float desiredYaw; // [esp+48h] [ebp-4h]

    phys = &veh->phys;
    if (veh->lookAtEnt.isDefined())
    {
        lookAtEnt = veh->lookAtEnt.ent();
        Vec3Sub(lookAtEnt->r.currentOrigin, phys->origin, diff);
        return (float)vectoyaw(diff);
    }
    else
    {
        if (veh->hasGoalYaw && (veh->stopping || VEH_IsHovering(veh)))
        {
            desiredYaw = veh->goalYaw;
            if (phys->maxAngleVel[1] == 0.0)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2745, 0, "%s", "phys->maxAngleVel[YAW]");
            if (veh->manualDecel == 0.0)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2746, 0, "%s", "veh->manualDecel");
            timeToStop = veh->speed / veh->manualDecel;
            v2 = phys->maxAngleVel[1] / phys->yawAccel;
            timeToTurn = v2 + v2;
            stopAngle = phys->maxAngleVel[1] * 0.5 * timeToTurn;
            angleDiff = desiredYaw - phys->angles[1];
            v8 = angleDiff * 0.002777777845039964f;
            v6 = v8 + 0.5f;
            v5 = floor(v6);
            v7 = (v8 - v5) * 360.0f;
            v4 = I_fabs(v7);
            if (timeToStop > timeToTurn && v4 > stopAngle)
                timeToTurn = (v4 - stopAngle) / phys->maxAngleVel[1] + timeToTurn;
            if (timeToStop <= timeToTurn)
                return desiredYaw;
        }
        if (veh->hasTargetYaw)
        {
            return veh->targetYaw;
        }
        else if (VEH_IsHovering(veh))
        {
            return phys->angles[1];
        }
        else
        {
            return vectoyaw(desiredDir);
        }
    }
}

float __cdecl VEH_CalcAccelFraction(float accel, int32_t infoIdx)
{
    float v4; // [esp+4h] [ebp-1Ch]
    float v5; // [esp+8h] [ebp-18h]
    float v6; // [esp+Ch] [ebp-14h]
    float v7; // [esp+10h] [ebp-10h]
    float minAccel; // [esp+14h] [ebp-Ch]
    float maxAccel; // [esp+1Ch] [ebp-4h]

    minAccel = 17.6f * 0.0f;
    maxAccel = s_vehicleInfos[infoIdx].accel;
    if (minAccel >= maxAccel)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2784, 0, "%s", "maxAccel > minAccel");
    v6 = accel - maxAccel;
    if (v6 < 0.0f)
        v7 = accel;
    else
        v7 = maxAccel;
    v5 = minAccel - accel;
    if (v5 < 0.0f)
        v4 = v7;
    else
        v4 = 17.6f * 0.0f;
    return (v4 - minAccel) / (maxAccel - minAccel);
}

float __cdecl VEH_CalcAngularAccel(float accel, float accelFraction)
{
    return accelFraction * 45.0f + (1.0f - accelFraction) * 1.0f;
}

float __cdecl VEH_CalcAngleForAccel(float accel, float accelFraction)
{
    return accelFraction * 1.0f + (1.0f - accelFraction) * 0.1f;
}

float __cdecl VEH_CalcStoppingTime(float accel, float accelFraction)
{
    float v4; // [esp+4h] [ebp-Ch]

    v4 = 1.0f - accelFraction;
    return v4 * 3.5f + (1.0f - v4) * 2.5f;
}

void __cdecl VEH_UpdateYawAndNotify(gentity_s *ent, float desiredYaw)
{
    float v2; // [esp+14h] [ebp-38h]
    bool v3; // [esp+18h] [ebp-34h]
    float v4; // [esp+1Ch] [ebp-30h]
    float finalYawDiff; // [esp+20h] [ebp-2Ch]
    float yawDecel; // [esp+28h] [ebp-24h]
    scr_vehicle_s *veh; // [esp+30h] [ebp-1Ch]
    float initialYawDiff; // [esp+34h] [ebp-18h]
    float overshoot; // [esp+38h] [ebp-14h]
    float yawAccel; // [esp+3Ch] [ebp-10h]
    float initalVel; // [esp+40h] [ebp-Ch]
    float EPSILON; // [esp+48h] [ebp-4h]

    EPSILON = 0.001f;
    veh = ent->scr_vehicle;
    initialYawDiff = veh->phys.angles[1] - veh->goalYaw;
    v3 = 0;
    if (veh->hasGoalYaw)
    {
        v4 = I_fabs(initialYawDiff);
        if (EPSILON < v4)
            v3 = 1;
    }
    initalVel = veh->phys.rotVel[1];
    yawAccel = veh->phys.yawAccel;
    yawDecel = veh->phys.yawDecel;
    if (veh->prevGoalYaw != desiredYaw)
    {
        veh->yawSlowDown = 0;
        veh->prevGoalYaw = desiredYaw;
    }
    if (veh->yawSlowDown)
    {
        yawAccel = yawAccel * 0.2f;
        yawDecel = yawDecel * 0.2f;
    }
    if (veh->hasGoalYaw || veh->hasTargetYaw)
        overshoot = veh->yawOverShoot;
    else
        overshoot = 0.0f;
    VEH_UpdateAngleAndAngularVel(1, desiredYaw, yawAccel, yawDecel, overshoot, &veh->phys);
    if (veh->hasGoalYaw && veh->phys.rotVel[1] * initalVel < 0.0f)
        veh->yawSlowDown = 1;
    if (v3)
    {
        finalYawDiff = veh->phys.angles[1] - veh->goalYaw;
        if (finalYawDiff * initialYawDiff < 0.0f || (v2 = I_fabs(finalYawDiff), EPSILON > v2))
            Scr_Notify(ent, scr_const.goal_yaw, 0);
    }
}

float __cdecl VEH_GetAccelForAngles(scr_vehicle_s *veh)
{
    float v2; // [esp+0h] [ebp-28h]
    float v3; // [esp+4h] [ebp-24h]
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+Ch] [ebp-1Ch]
    float MAX_HOVER_ANGLE_VEL; // [esp+20h] [ebp-8h]
    float MAX_HOVER_ANGLE; // [esp+24h] [ebp-4h]

    if (!veh)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2878, 0, "%s", "veh");
    if (VEH_IsHovering(veh))
    {
        MAX_HOVER_ANGLE = 5.0f;
        MAX_HOVER_ANGLE_VEL = 3.0f;
        if (veh->hover.useHoverAccelForAngles)
        {
            return veh->hover.hoverAccel;
        }
        else
        {
            v5 = I_fabs(veh->phys.angles[0]);
            if (MAX_HOVER_ANGLE >= v5 && (v4 = I_fabs(veh->phys.angles[2]), MAX_HOVER_ANGLE >= v4))
            {
                v3 = I_fabs(veh->phys.rotVel[0]);
                if (MAX_HOVER_ANGLE_VEL >= v3 && (v2 = I_fabs(veh->phys.rotVel[2]), MAX_HOVER_ANGLE_VEL >= v2))
                {
                    veh->hover.useHoverAccelForAngles = 1;
                    return veh->hover.hoverAccel;
                }
                else
                {
                    return veh->manualAccel;
                }
            }
            else
            {
                return veh->manualAccel;
            }
        }
    }
    else
    {
        veh->hover.useHoverAccelForAngles = 0;
        return veh->manualAccel;
    }
}

void __cdecl VEH_AddFakeDrag(const float *velocity, float maxDragSpeed, float *accelVec)
{
    float v3; // [esp+0h] [ebp-24h]
    float v4; // [esp+4h] [ebp-20h]
    float v5; // [esp+8h] [ebp-1Ch]
    float fakeDrag; // [esp+14h] [ebp-10h]
    float fakeDraga; // [esp+14h] [ebp-10h]
    float horizontalVel; // [esp+18h] [ebp-Ch]
    float velocityVec[2]; // [esp+1Ch] [ebp-8h] BYREF

    velocityVec[0] = *velocity;
    velocityVec[1] = velocity[1];
    horizontalVel = Vec2Length(velocityVec);
    v4 = maxDragSpeed - horizontalVel;
    if (v4 < 0.0f)
        v3 = maxDragSpeed;
    else
        v3 = horizontalVel;
    if (maxDragSpeed == 0.0f)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 2925, 0, "%s", "maxDragSpeed");
    fakeDrag = v3 / maxDragSpeed;
    fakeDraga = fakeDrag * fakeDrag;
    Vec2Normalize(velocityVec);
    v5 = fakeDraga * 5.0f;
    velocityVec[0] = v5 * velocityVec[0];
    velocityVec[1] = v5 * velocityVec[1];
    *accelVec = *accelVec + velocityVec[0];
    accelVec[1] = accelVec[1] + velocityVec[1];
}

void __cdecl VEH_CheckHorizontalVelocityToGoal(
    scr_vehicle_s *veh,
    const float *vecToGoal,
    float accelMax,
    float *accelVec)
{
    float turningAbility; // [esp+0h] [ebp-98h]
    float v5; // [esp+4h] [ebp-94h]
    float v6; // [esp+8h] [ebp-90h]
    float v7; // [esp+Ch] [ebp-8Ch]
    float v8; // [esp+10h] [ebp-88h]
    float *v9; // [esp+14h] [ebp-84h]
    float *v10; // [esp+18h] [ebp-80h]
    float v11; // [esp+20h] [ebp-78h]
    float v12; // [esp+28h] [ebp-70h]
    float *vel; // [esp+2Ch] [ebp-6Ch]
    float v14; // [esp+30h] [ebp-68h]
    float breakVec; // [esp+48h] [ebp-50h]
    float breakVec_4; // [esp+4Ch] [ebp-4Ch]
    float breakingAccel; // [esp+50h] [ebp-48h]
    float breakFactor; // [esp+58h] [ebp-40h]
    float radius; // [esp+5Ch] [ebp-3Ch]
    float oldSpeed; // [esp+60h] [ebp-38h]
    float requiredDecel; // [esp+64h] [ebp-34h]
    float newSpeed; // [esp+6Ch] [ebp-2Ch]
    float newVel[3]; // [esp+70h] [ebp-28h] BYREF
    vehicle_physic_t *phys; // [esp+7Ch] [ebp-1Ch]
    float horizontalDist; // [esp+80h] [ebp-18h]
    float horizontalSpeed; // [esp+84h] [ebp-14h]
    float perpDir[2]; // [esp+88h] [ebp-10h]
    float radiusVec[2]; // [esp+90h] [ebp-8h]

    phys = &veh->phys;
    horizontalDist = Vec2Length(vecToGoal);
    if (horizontalDist >= 1.0f)
    {
        horizontalSpeed = Vec2Length(phys->vel);
        if (veh->stopAtGoal
            && (newVel[0] = phys->vel[0] + *accelVec,
                newVel[1] = phys->vel[1] + accelVec[1],
                newSpeed = Vec2Length(newVel),
                newSpeed > 0.0f))
        {
            oldSpeed = Vec2Length(phys->vel);
            v8 = newSpeed - oldSpeed;
            v7 = I_fabs(v8);
            requiredDecel = horizontalSpeed * horizontalSpeed / (horizontalDist * 2.0f) * 0.05000000074505806f;
            if (v7 < requiredDecel)
            {
                v14 = (oldSpeed - requiredDecel) / newSpeed;
                newVel[0] = v14 * newVel[0];
                newVel[1] = v14 * newVel[1];
                vel = phys->vel;
                *accelVec = newVel[0] - phys->vel[0];
                accelVec[1] = newVel[1] - vel[1];
            }
        }
        else
        {
            perpDir[0] = phys->vel[1];
            perpDir[1] = -phys->vel[0];
            if (horizontalDist == 0.0)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3061, 0, "%s", "horizontalDist");
            v12 = perpDir[0] * *vecToGoal + perpDir[1] * vecToGoal[1];
            v6 = I_fabs(v12);
            radiusVec[0] = v6;
            if (horizontalDist < v6)
            {
                radiusVec[1] = vecToGoal[1] * phys->vel[1] + *vecToGoal * phys->vel[0];
                v11 = 1.0 / horizontalSpeed;
                radiusVec[0] = v11 * radiusVec[0];
                radiusVec[1] = v11 * radiusVec[1];
                v5 = radiusVec[1] * radiusVec[1] + radiusVec[0] * radiusVec[0];
                radius = v5 / (radiusVec[0] * 2.0f);
                if (radius <= 0.0f)
                    MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3073, 0, "%s", "radius > 0.f");
                if (radius > 1.0f && accelMax * radius < horizontalSpeed * horizontalSpeed)
                {
                    if (radius == 0.0f)
                        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3083, 0, "%s", "radius");
                    breakingAccel = horizontalSpeed * horizontalSpeed / radius;
                    if (horizontalSpeed < breakingAccel)
                        breakingAccel = horizontalSpeed;
                    if (veh->stopAtGoal)
                        turningAbility = 1.0f;
                    else
                        turningAbility = veh->turningAbility;
                    breakFactor = -turningAbility * breakingAccel / horizontalSpeed * 0.05000000074505806f;
                    breakVec = breakFactor * phys->vel[0];
                    breakVec_4 = breakFactor * phys->vel[1];
                    v9 = phys->vel;
                    v10 = phys->vel;
                    phys->vel[0] = phys->vel[0] + breakVec;
                    v9[1] = v10[1] + breakVec_4;
                    *accelVec = *accelVec + breakVec;
                    accelVec[1] = accelVec[1] + breakVec_4;
                }
            }
        }
    }
}

void __cdecl VEH_CheckVerticalVelocityToGoal(scr_vehicle_s *veh, float verticalDist, float *accelVec)
{
    float v3; // [esp+0h] [ebp-3Ch]
    float v4; // [esp+4h] [ebp-38h]
    float v5; // [esp+8h] [ebp-34h]
    float v6; // [esp+Ch] [ebp-30h]
    float v7; // [esp+10h] [ebp-2Ch]
    float v8; // [esp+14h] [ebp-28h]
    float v9; // [esp+18h] [ebp-24h]
    float accelerationCap; // [esp+20h] [ebp-1Ch]
    float breakingAccel; // [esp+24h] [ebp-18h]
    float desiredStoppingTime; // [esp+28h] [ebp-14h]
    float currentStoppingTime; // [esp+2Ch] [ebp-10h]
    float verticalSpeed; // [esp+38h] [ebp-4h]

    verticalSpeed = veh->phys.vel[2];
    v7 = I_fabs(accelVec[2]);
    if (0.001f <= v7)
    {
        v6 = I_fabs(verticalSpeed);
        if (0.001f <= v6 && verticalSpeed * accelVec[2] < 0.0 && verticalDist * verticalSpeed > 0.0f)
        {
            currentStoppingTime = verticalSpeed * 0.05000000074505806f / -accelVec[2];
            desiredStoppingTime = verticalDist / (verticalSpeed * 0.5f);
            if (desiredStoppingTime < currentStoppingTime)
            {
                if (desiredStoppingTime == 0.0f)
                    MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3124, 0, "%s", "desiredStoppingTime");
                breakingAccel = -verticalSpeed * 0.05000000074505806f / desiredStoppingTime;
                if (accelVec[2] * accelVec[2] < breakingAccel * breakingAccel)
                {
                    accelerationCap = veh->manualAccel * 0.05000000074505806f * 3.0f;
                    v5 = breakingAccel - accelerationCap;
                    if (v5 < 0.0f)
                        v9 = -verticalSpeed * 0.05000000074505806f / desiredStoppingTime;
                    else
                        v9 = veh->manualAccel * 0.05000000074505806f * 3.0f;
                    v8 = -accelerationCap;
                    v4 = v8 - breakingAccel;
                    if (v4 < 0.0f)
                        v3 = v9;
                    else
                        v3 = -accelerationCap;
                    veh->phys.vel[2] = v3 - accelVec[2] + veh->phys.vel[2];
                    accelVec[2] = v3;
                }
            }
        }
    }
}

int32_t __cdecl VEH_UpdateMove_CheckGoalReached(gentity_s *ent, float distToGoal)
{
    bool v3; // [esp+0h] [ebp-1Ch]
    bool v4; // [esp+4h] [ebp-18h]
    float MAX_SPEED_FOR_HOVER; // [esp+10h] [ebp-Ch]
    bool goalReached; // [esp+17h] [ebp-5h]
    scr_vehicle_s *veh; // [esp+18h] [ebp-4h]

    veh = ent->scr_vehicle;
    if (veh->stopAtGoal)
    {
        if (veh->hover.hoverRadius == 0.0f)
        {
            v3 = (veh->stopping || distToGoal == 0.0f) && veh->speed == 0.0f;
            goalReached = v3;
        }
        else
        {
            v4 = 0;
            if (veh->hover.hoverRadius >= distToGoal)
            {
                MAX_SPEED_FOR_HOVER = 17.6f * 2.0f;
                if (MAX_SPEED_FOR_HOVER > veh->speed)
                    v4 = 1;
            }
            goalReached = v4;
        }
        if (goalReached)
        {
            veh->moveState = VEH_MOVESTATE_HOVER;
            if (veh->hover.hoverRadius == 0.0f)
            {
                veh->phys.accel[0] = 0.0f;
                veh->phys.accel[1] = 0.0f;
                veh->phys.accel[2] = 0.0f;
                veh->phys.vel[0] = 0.0f;
                veh->phys.vel[1] = 0.0f;
                veh->phys.vel[2] = 0.0f;
            }
            goto LABEL_20;
        }
    }
    else if (distToGoal <= veh->speed * 0.05000000074505806f)
    {
    LABEL_20:
        Scr_Notify(ent, scr_const.goal, 0);
        return 1;
    }
    return 0;
}

double __cdecl VEH_UpdateMove_CheckStop(scr_vehicle_s *veh, float distToGoal)
{
    float v3; // [esp+10h] [ebp-24h]
    float v4; // [esp+14h] [ebp-20h]
    float v5; // [esp+18h] [ebp-1Ch]
    float v6; // [esp+1Ch] [ebp-18h]
    float dt; // [esp+20h] [ebp-14h]
    float dta; // [esp+20h] [ebp-14h]
    float checkDist; // [esp+24h] [ebp-10h]
    float newSpeed; // [esp+28h] [ebp-Ch]
    float time; // [esp+2Ch] [ebp-8h]
    float stopDist; // [esp+30h] [ebp-4h]

    dt = 0.050000001f;
    newSpeed = VEH_AccelerateSpeed(veh->speed, veh->manualSpeed, veh->manualAccel, 0.050000001f);
    if (veh->manualDecel == 0.0f)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3211, 0, "%s", "veh->manualDecel");
    time = newSpeed / veh->manualDecel;
    stopDist = newSpeed * 0.5f * time;
    checkDist = distToGoal - newSpeed * 0.05000000074505806f;
    if (stopDist < checkDist || veh->speed <= 0.0f)
    {
        veh->stopping = 0;
    }
    else
    {
        if (stopDist < distToGoal)
        {
            if (veh->speed == 0.0f)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3223, 0, "%s", "veh->speed");
            dta = 0.05000000074505806f - (distToGoal - stopDist) / veh->speed;
            v5 = dta - 0.050000001f;
            if (v5 < 0.0f)
                v6 = 0.05000000074505806f - (distToGoal - stopDist) / veh->speed;
            else
                v6 = 0.050000001f;
            v4 = 0.0f - dta;
            if (v4 < 0.0f)
                v3 = v6;
            else
                v3 = 0.0f;
            dt = v3;
        }
        veh->stopping = 1;
    }
    return dt;
}

void __cdecl VEH_UpdateMove_CheckNearGoal(gentity_s *ent, float distToGoal)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3241, 0, "%s", "ent");
    veh = ent->scr_vehicle;
    if (veh->nearGoalNotifyDist != 0.0f && veh->nearGoalNotifyDist > (double)distToGoal)
        Scr_Notify(ent, scr_const.near_goal, 0);
}

void __cdecl VEH_GetNewSpeedAndAccel(scr_vehicle_s *veh, float dt, int32_t hovering, float *newSpeed, float *accelMax)
{
    float decel; // [esp+10h] [ebp-Ch]
    float speed; // [esp+14h] [ebp-8h]
    float accel; // [esp+18h] [ebp-4h]

    if (!veh)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3255, 0, "%s", "veh");
    if (hovering)
    {
        speed = veh->hover.hoverSpeed;
        accel = veh->hover.hoverAccel;
        decel = accel * 0.5f;
    }
    else
    {
        speed = veh->manualSpeed;
        accel = veh->manualAccel;
        decel = veh->manualDecel;
    }
    if (veh->stopping)
    {
        *newSpeed = VEH_AccelerateSpeed(veh->speed, 0.0, decel, dt);
        *accelMax = decel;
    }
    else
    {
        if (speed < 0.0)
            MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3277, 0, "%s", "speed >= 0.0f");
        *newSpeed = VEH_AccelerateSpeed(veh->speed, speed, accel, dt);
        *accelMax = accel;
    }
}

void __cdecl VEH_UpdateHover(gentity_s *ent)
{
    double v1; // [esp+0h] [ebp-2Ch]
    float diff[3]; // [esp+8h] [ebp-24h] BYREF
    scr_vehicle_s *veh; // [esp+14h] [ebp-18h]
    float NEW_HOVER_DIST_FACTOR; // [esp+18h] [ebp-14h]
    float hoverPos[3]; // [esp+1Ch] [ebp-10h] BYREF
    float newHoverDist; // [esp+28h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3400, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 3401, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    Vec3Add(veh->goalPosition, veh->hover.hoverGoalPos, hoverPos);
    VEH_UpdateMoveToGoal(ent, hoverPos);
    NEW_HOVER_DIST_FACTOR = 0.25f;
    newHoverDist = veh->hover.hoverRadius * 0.25f;
    Vec3Sub(hoverPos, veh->phys.origin, diff);
    v1 = newHoverDist * newHoverDist;
    if (Vec3LengthSq(diff) < v1)
        VEH_SetHoverGoal(ent);
}

void __cdecl VEH_SetHoverGoal(gentity_s *ent)
{
    float min; // [esp+4h] [ebp-20h]
    int32_t i; // [esp+10h] [ebp-14h]
    scr_vehicle_s *veh; // [esp+14h] [ebp-10h]
    float randomOffset[3]; // [esp+18h] [ebp-Ch] BYREF

    veh = ent->scr_vehicle;
    if (veh->hover.hoverRadius == 0.0f)
    {
        veh->hover.hoverGoalPos[0] = 0.0f;
        veh->hover.hoverGoalPos[1] = 0.0f;
        veh->hover.hoverGoalPos[2] = 0.0f;
    }
    else
    {
        for (i = 0; i < 3; ++i)
        {
            min = -veh->hover.hoverRadius;
            randomOffset[i] = G_flrand(min, veh->hover.hoverRadius);
        }
        Vec3Mad(randomOffset, -0.5f, veh->hover.hoverGoalPos, veh->hover.hoverGoalPos);
    }
}

void __cdecl CMD_VEH_SetSpeed(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    CMD_VEH_Script_SetSpeed(ent);
}

void __cdecl CMD_VEH_Script_SetSpeed(gentity_s *ent)
{
    vehicle_info_t *info; // [esp+0h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 4868, 0, "%s", "ent");
    veh = ent->scr_vehicle;
    info = &s_vehicleInfos[veh->infoIdx];
    if (!veh)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 4873, 0, "%s", "veh");
    if (!info)
        MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 4874, 0, "%s", "info");
    veh->manualMode = 1;
    veh->manualSpeed = Scr_GetFloat(0) * 17.6f;
    if (veh->manualSpeed < 0.0f)
    {
        Scr_ParamError(0, "Cannot set negative speed on vehicle");
        veh->manualSpeed = 0.0f;
    }
    if (Scr_GetNumParam() > 1)
        veh->manualAccel = Scr_GetFloat(1) * 17.6f;
    if (info->type == 5 && veh->speed < veh->manualSpeed && veh->manualAccel > veh->manualSpeed)
    {
        Com_PrintWarning(15, "WARNING: capping acceleration to speed / sec for vehicle '%d'\n", ent->s.number);
        veh->manualAccel = veh->manualSpeed;
    }
    if (Scr_GetNumParam() <= 2)
        veh->manualDecel = veh->manualAccel * 0.5f;
    else
        veh->manualDecel = Scr_GetFloat(2) * 17.6f;
    if (veh->manualAccel <= 0.0 || veh->manualDecel <= 0.0f)
    {
        Scr_ParamError(1u, "Acceleration/deceleration must be > 0");
        veh->manualAccel = 17.6f * 1.0f;
        veh->manualDecel = 17.6f * 1.0f;
    }
}

void __cdecl CMD_VEH_GetSpeed(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    Scr_AddFloat(veh->speed);
}

void __cdecl CMD_VEH_GetSpeedMPH(scr_entref_t entref)
{
    Scr_AddFloat(GScr_GetVehicle(entref)->scr_vehicle->speed / 17.6f);
}

void __cdecl CMD_VEH_ResumeSpeed(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->manualMode = 2;
    veh->manualAccel = Scr_GetFloat(0) * 17.6f;
    if (veh->manualAccel < 0.0f)
        Scr_ParamError(0, "Cannot set negative acceleration on vehicle");
}

void __cdecl CMD_VEH_SetYawSpeed(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->phys.maxAngleVel[1] = Scr_GetFloat(0);
    veh->phys.yawAccel = Scr_GetFloat(1);
    if (Scr_GetNumParam() <= 2)
        veh->phys.yawDecel = veh->phys.yawAccel;
    else
        veh->phys.yawDecel = Scr_GetFloat(2);
    if (Scr_GetNumParam() > 3)
    {
        veh->yawOverShoot = Scr_GetFloat(3);
        if (veh->yawOverShoot < 0.0f || veh->yawOverShoot > 1.0f)
            Scr_ParamError(3u, "Overshoot must be in 0 to 1 range");
    }
    if (veh->phys.maxAngleVel[1] < 0.0f)
        Scr_ParamError(0, "Cannot set negative yaw speed on vehicle");
    if (veh->phys.yawAccel < 0.0f)
        Scr_ParamError(1u, "Cannot set negative yaw acceleration on vehicle");
}

void __cdecl CMD_VEH_SetMaxPitchRoll(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if (s_vehicleInfos[veh->infoIdx].type != 5)
        Scr_Error("Max Pitch Roll only valid for helicopters");
    veh->phys.maxPitchAngle = Scr_GetFloat(0);
    veh->phys.maxRollAngle = Scr_GetFloat(1);
    if (veh->phys.maxPitchAngle < 0.0f)
        Scr_ParamError(0, "Cannot set negative max pitch");
    if (veh->phys.maxRollAngle < 0.0f)
        Scr_ParamError(1u, "Cannot set negative max roll");
}

void __cdecl CMD_VEH_SetAirResitance(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->maxDragSpeed = Scr_GetFloat(0) * 17.6f;
}

void __cdecl CMD_VEH_SetTurningAbility(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+14h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->turningAbility = Scr_GetFloat(0);
}

void __cdecl CMD_VEH_SetHoverParams(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->hover.hoverRadius = Scr_GetFloat(0);
    if (Scr_GetNumParam() > 1)
    {
        veh->hover.hoverSpeed = Scr_GetFloat(1);
        if (Scr_GetNumParam() > 2)
            veh->hover.hoverAccel = Scr_GetFloat(2);
    }
}

void __cdecl CMD_VEH_SetVehicleTeam(scr_entref_t entref)
{
    const char *team; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]
    scr_vehicle_s *veh;

    ent = GScr_GetVehicle(entref);
    iassert(ent);
    veh = ent->scr_vehicle;
    iassert(veh);

    team = Scr_GetString(0);

    if (I_stricmp(team, "axis"))
    {
        if (I_stricmp(team, "allies"))
        {
            if (I_stricmp(team, "none"))
                Scr_Error("setVehicleTeam: invalid team used must be 'axis', 'allies', or 'none'\n");
            else
                veh->team = 0;
        }
        else
        {
            veh->team = 2;
        }
    }
    else
    {
        veh->team = 1;
    }
}

void __cdecl CMD_VEH_NearGoalNotifyDist(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->nearGoalNotifyDist = Scr_GetFloat(0);
}

void __cdecl CMD_VEH_SetGoalPos(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->manualMode = 1;
    Scr_GetVector(0, veh->goalPosition);
    veh->stopAtGoal = Scr_GetNumParam() > 1 && Scr_GetInt(1) != 0;
    if (veh->manualSpeed == 0.0f || veh->manualAccel == 0.0f || veh->manualDecel == 0.0f)
        Scr_Error("Speed and acceleration must not be zero before setting goal pos");
    veh->flags |= 2u;
    veh->moveState = VEH_MOVESTATE_MOVE;
    veh->stopping = 0;
}

void __cdecl CMD_VEH_SetGoalYaw(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->goalYaw = Scr_GetFloat(0);
    veh->hasGoalYaw = 1;
}

void __cdecl CMD_VEH_ClearGoalYaw(scr_entref_t entref)
{
    scr_vehicle_s *scr_vehicle; // eax

    scr_vehicle = GScr_GetVehicle(entref)->scr_vehicle;
    scr_vehicle->goalYaw = 0.0f;
    scr_vehicle->hasGoalYaw = 0;
}

void __cdecl CMD_VEH_SetTargetYaw(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->targetYaw = Scr_GetFloat(0);
    veh->hasTargetYaw = 1;
}

void __cdecl CMD_VEH_ClearTargetYaw(scr_entref_t entref)
{
    scr_vehicle_s *scr_vehicle; // eax

    scr_vehicle = GScr_GetVehicle(entref)->scr_vehicle;
    scr_vehicle->targetYaw = 0.0;
    scr_vehicle->hasTargetYaw = 0;
}

void __cdecl CMD_VEH_SetTurretTargetVec(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    scr_vehicle_s *veh; // [esp+8h] [ebp-14h]
    gentity_s *ent; // [esp+Ch] [ebp-10h]
    float tgtPos[3]; // [esp+10h] [ebp-Ch] BYREF

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if ((veh->flags & 1) != 0)
    {
        v1 = va("Can't set target position on player's vehicle");
        Scr_Error(v1);
    }
    if ((float)ent->health <= 0.0f)
    {
        v2 = va("Vehicle must have health to control the turret");
        Scr_Error(v2);
    }
    veh->hasTarget = 1;
    veh->targetEnt = 1023;
    Scr_GetVector(0, tgtPos);
    veh->targetOrigin[0] = tgtPos[0];
    veh->targetOrigin[1] = tgtPos[1];
    veh->targetOrigin[2] = tgtPos[2];
    veh->targetOffset[0] = 0.0f;
    veh->targetOffset[1] = 0.0f;
    veh->targetOffset[2] = 0.0f;
}

void __cdecl CMD_VEH_SetTurretTargetEnt(scr_entref_t entref)
{
    gentity_s *Vehicle; // ecx
    const char *v2; // eax
    int32_t number; // [esp+0h] [ebp-18h]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-Ch]
    gentity_s *tgtEnt; // [esp+10h] [ebp-8h]

    Vehicle = GScr_GetVehicle(entref);
    veh = Vehicle->scr_vehicle;
    if ((float)Vehicle->health <= 0.0f)
    {
        v2 = va("Vehicle must have health to control the turret");
        Scr_Error(v2);
    }
    tgtEnt = Scr_GetEntity(0);
    veh->hasTarget = 1;
    if (tgtEnt)
        number = tgtEnt->s.number;
    else
        number = 1023;
    veh->targetEnt = number;
    if (tgtEnt)
        tgtEnt->flags |= 0x80000u;
    if (Scr_GetNumParam() <= 1)
    {
        veh->targetOffset[0] = 0.0f;
        veh->targetOffset[1] = 0.0f;
        veh->targetOffset[2] = 0.0f;
    }
    else
    {
        Scr_GetVector(1u, veh->targetOffset);
    }
    veh->targetOrigin[0] = 0.0f;
    veh->targetOrigin[1] = 0.0f;
    veh->targetOrigin[2] = 0.0f;
}

void __cdecl CMD_VEH_ClearTurretTargetEnt(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ((veh->flags & 1) == 0)
        veh->hasTarget = 0;
    veh->targetEnt = 1023;
    veh->targetOrigin[0] = 0.0f;
    veh->targetOrigin[1] = 0.0f;
    veh->targetOrigin[2] = 0.0f;
    veh->targetOffset[0] = 0.0f;
    veh->targetOffset[1] = 0.0f;
    veh->targetOffset[2] = 0.0f;
}

void __cdecl CMD_VEH_SetLookAtEnt(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    scr_vehicle_s *veh; // [esp+0h] [ebp-Ch]
    gentity_s *tgtEnt; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if ((veh->flags & 1) != 0)
    {
        v1 = va("Can't set target on player's vehicle");
        Scr_Error(v1);
    }
    if ((float)ent->health <= 0.0f)
    {
        v2 = va("Vehicle must have health to control");
        Scr_Error(v2);
    }
    tgtEnt = Scr_GetEntity(0);
    if (!tgtEnt)
    {
        v3 = va("Invalid entity");
        Scr_Error(v3);
    }
    veh->lookAtEnt.setEnt(tgtEnt);
}

void __cdecl CMD_VEH_ClearLookAtEnt(scr_entref_t entref)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    ent->scr_vehicle->lookAtEnt.setEnt(NULL);
}

void __cdecl CMD_VEH_SetWeapon(scr_entref_t entref)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    ent->s.weapon = (uint8_t)BG_FindWeaponIndexForName(Scr_GetString(0));
    ent->s.weaponModel = 0;
}

void __cdecl CMD_VEH_FireWeapon(scr_entref_t entref)
{
    const char *v4; // eax
    float intensity; // [esp+8h] [ebp-168h]
    gentity_s *Entity; // [esp+14h] [ebp-15Ch]
    float v9; // [esp+18h] [ebp-158h]
    float v10; // [esp+1Ch] [ebp-154h]
    float v11; // [esp+20h] [ebp-150h]
    float v12; // [esp+24h] [ebp-14Ch]
    float v13; // [esp+28h] [ebp-148h]
    float v14; // [esp+2Ch] [ebp-144h]
    float v15; // [esp+3Ch] [ebp-134h]
    float v16; // [esp+40h] [ebp-130h]
    float v17; // [esp+44h] [ebp-12Ch]
    float v18; // [esp+48h] [ebp-128h]
    float aimPadding; // [esp+4Ch] [ebp-124h]
    float v20; // [esp+50h] [ebp-120h]
    float joltDir[3]; // [esp+60h] [ebp-110h] BYREF
    int32_t numBarrels; // [esp+6Ch] [ebp-104h]
    int32_t boneIndex; // [esp+70h] [ebp-100h]
    vehicle_info_t *info; // [esp+74h] [ebp-FCh]
    scr_vehicle_s *veh; // [esp+78h] [ebp-F8h]
    float diffAngles[3]; // [esp+7Ch] [ebp-F4h] BYREF
    int32_t event; // [esp+88h] [ebp-E8h]
    gentity_s *target; // [esp+8Ch] [ebp-E4h]
    int32_t startBarrel; // [esp+90h] [ebp-E0h]
    float gunAngles[3]; // [esp+94h] [ebp-DCh] BYREF
    weaponParms wp; // [esp+A0h] [ebp-D0h] BYREF
    gentity_s *missile; // [esp+E4h] [ebp-8Ch]
    float targetOffset[3]; // [esp+E8h] [ebp-88h] BYREF
    gentity_s *ent; // [esp+F4h] [ebp-7Ch]
    float bulletAngles[3]; // [esp+F8h] [ebp-78h] BYREF
    int32_t i; // [esp+104h] [ebp-6Ch]
    float barrelMtx[4][3]; // [esp+108h] [ebp-68h] BYREF
    int32_t barrel; // [esp+138h] [ebp-38h]
    gentity_s *player; // [esp+13Ch] [ebp-34h]
    float flashMtx[4][3]; // [esp+140h] [ebp-30h] BYREF

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    info = &s_vehicleInfos[veh->infoIdx];
    if ((float)ent->health <= 0.0f)
    {
        Scr_Error(va("Vehicle must have health to control the turret"));
    }
    if (!ent->s.weapon)
    {
        Scr_Error(va("Invalid weapon specified for [%s]\n", SL_ConvertToString(ent->targetname)));
    }
    wp.weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (wp.weapDef->weapType && wp.weapDef->weapType != WEAPTYPE_PROJECTILE)
    {
        v4 = va("Vehicles only support bullet and projectile weapons\n");
        Scr_Error(v4);
    }
    if (info->type != 2 && info->type != 5)
    {
        if (veh->boneIndex.barrel < 0)
        {
            Scr_Error(va("No tag_barrel for [%s]\n", SL_ConvertToString(ent->targetname)));
        }
        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.barrel, barrelMtx);
        joltDir[0] = -barrelMtx[0][0];
        joltDir[1] = -barrelMtx[0][1];
        joltDir[2] = -barrelMtx[0][2];
        VEH_JoltBody(ent, joltDir, 1.0f, 0.0f, 0.0f);
    }
    if (!Scr_GetNumParam() && info->quadBarrel)
    {
        numBarrels = 2;
        startBarrel = veh->turret.fireBarrel != 0 ? 2 : 0;
        event = (veh->turret.fireBarrel != 0) + 39;
    }
    else
    {
        numBarrels = 1;
        startBarrel = 0;
        event = 26;
    }
    for (i = 0; i < numBarrels; ++i)
    {
        barrel = i + startBarrel;
        boneIndex = VEH_GetTagBoneIndex(ent, i + startBarrel);
        if (boneIndex < 0)
            MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 6189, 1, "%s", "boneIndex >= 0");
        G_DObjGetWorldBoneIndexMatrix(ent, boneIndex, flashMtx);
        if (ent->r.ownerNum.isDefined()
            && numBarrels == 1
            && !veh->turret.barrelBlocked
            && veh->targetEnt == 1023)
        {
            player = ent->r.ownerNum.ent();
            if (!player->client)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 6196, 0, "%s", "player->client");
            Vec3Sub(veh->targetOrigin, flashMtx[3], wp.gunForward);
            Vec3Normalize(wp.gunForward);
            vectoangles(flashMtx[0], gunAngles);
            vectoangles(wp.gunForward, bulletAngles);
            AnglesSubtract(gunAngles, bulletAngles, diffAngles);
            aimPadding = wp.weapDef->aimPadding;
            v14 = diffAngles[0] - aimPadding;
            if (v14 < 0.0)
                v20 = diffAngles[0];
            else
                v20 = aimPadding;
            v18 = -wp.weapDef->aimPadding;
            v13 = v18 - diffAngles[0];
            if (v13 < 0.0)
                v12 = v20;
            else
                v12 = -wp.weapDef->aimPadding;
            diffAngles[0] = v12;
            v16 = wp.weapDef->aimPadding;
            v11 = diffAngles[1] - v16;
            if (v11 < 0.0)
                v17 = diffAngles[1];
            else
                v17 = v16;
            v15 = -wp.weapDef->aimPadding;
            v10 = v15 - diffAngles[1];
            if (v10 < 0.0)
                v9 = v17;
            else
                v9 = -wp.weapDef->aimPadding;
            diffAngles[1] = v9;
            diffAngles[2] = 0.0;
            AnglesSubtract(gunAngles, diffAngles, gunAngles);
            AngleVectors(gunAngles, wp.gunForward, 0, 0);
        }
        else
        {
            wp.gunForward[0] = flashMtx[0][0];
            wp.gunForward[1] = flashMtx[0][1];
            wp.gunForward[2] = flashMtx[0][2];
        }
        wp.forward[0] = wp.gunForward[0];
        wp.forward[1] = wp.gunForward[1];
        wp.forward[2] = wp.gunForward[2];
        wp.right[0] = flashMtx[1][0];
        wp.right[1] = flashMtx[1][1];
        wp.right[2] = flashMtx[1][2];
        wp.up[0] = flashMtx[2][0];
        wp.up[1] = flashMtx[2][1];
        wp.up[2] = flashMtx[2][2];
        if (veh->turret.barrelBlocked)
        {
            intensity = -veh->turret.barrelOffset;
            Vec3Mad(flashMtx[3], intensity, wp.gunForward, wp.muzzleTrace);
        }
        else
        {
            wp.muzzleTrace[0] = flashMtx[3][0];
            wp.muzzleTrace[1] = flashMtx[3][1];
            wp.muzzleTrace[2] = flashMtx[3][2];
        }
        if (wp.weapDef->weapType)
        {
            if (Scr_GetNumParam() <= 1)
                Entity = 0;
            else
                Entity = Scr_GetEntity(1);
            target = Entity;
            if (Scr_GetNumParam() <= 2)
            {
                targetOffset[0] = 0.0f;
                targetOffset[1] = 0.0f;
                targetOffset[2] = 0.0f;
            }
            else
            {
                Scr_GetVector(2u, targetOffset);
            }
            missile = Weapon_RocketLauncher_Fire(
                ent,
                ent->s.weapon,
                wp.weapDef->fAdsSpread,
                &wp,
                veh->phys.vel,
                target,
                targetOffset);
            if (!missile)
                MyAssertHandler(".\\game\\g_scr_vehicle.cpp", 6250, 0, "%s", "missile");
            Scr_AddEntity(missile);
        }
        else
        {
            Bullet_Fire(ent, 0.0f, &wp, ent, level.time);
        }
    }
    G_AddEvent(ent, event, 0);
    Scr_Notify(ent, scr_const.weapon_fired, 0);
    veh->turret.fireTime = wp.weapDef->iFireTime;
    veh->turret.fireBarrel = veh->turret.fireBarrel == 0;
}

int32_t __cdecl VEH_GetTagBoneIndex(gentity_s *ent, int32_t barrel)
{
    const char *boneName; // [esp+0h] [ebp-Ch]
    int32_t boneIndex; // [esp+4h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-4h]

    boneIndex = -1;
    veh = ent->scr_vehicle;
    if (Scr_GetNumParam())
    {
        boneName = Scr_GetString(0);
        if (*boneName)
        {
            boneIndex = SV_DObjGetBoneIndex(ent, Scr_GetConstLowercaseString(0));
            if (boneIndex < 0)
            {
                Scr_Error(va("No tag %s for [%s]\n", boneName, SL_ConvertToString(ent->targetname)));
            }
        }
        else
        {
            boneIndex = -1;
        }
    }
    if (!Scr_GetNumParam() || boneIndex < 0)
    {
        if (veh->boneIndex.flash[barrel] < 0)
        {
            Scr_Error(va("No %s for [%s]\n", SL_ConvertToString(*s_flashTags[barrel]), SL_ConvertToString(ent->targetname)));
        }
        else
        {
            return veh->boneIndex.flash[barrel];
        }
    }
    return boneIndex;
}

gentity_s *G_IsVehicleUnusable(gentity_s *player)
{
    gclient_s *client; // r11
    EntHandle *p_ownerNum; // r31
    gentity_s *result; // r3

    client = player->client;
    if (!client)
        return 0;
    if ((client->ps.eFlags & 0x20000) == 0)
        return 0;

    p_ownerNum = &player->r.ownerNum;

    //if (!EntHandle::isDefined(&player->r.ownerNum))
    if (!player->r.ownerNum.isDefined())
        return 0;

    result = p_ownerNum->ent();
    if ((result->r.contents & 0x200000) == 0)
        return 0;
    return result;
}

bool G_IsVehicleImmune(gentity_s *ent, int mod, char damageFlags, unsigned int weapon)
{
    vehicle_info_t *v4 = &s_vehicleInfos[ent->scr_vehicle->infoIdx];
    int result;
    unsigned int damageValue;

    switch (mod)
    {
    case 1:
    case 2:
        if (v4->bulletDamage || ((damageFlags & 2) != 0 && v4->armorPiercingDamage))
            goto LABEL_3;
        goto LABEL_6;

    case 3:
    case 4:
    {

        if (BG_GetWeaponDef(weapon)->projExplosion == WEAPPROJEXP_HEAVY)
            damageValue = v4->heavyExplosiveDamage;
        else
            damageValue = v4->grenadeDamage;

        result = (damageValue == 0) ? 1 : 0;
        break;
    }

    case 5:
        result = (v4->projectileDamage == 0);
        break;

    case 6:
        result = (v4->projectileSplashDamage == 0);
        break;

    case 14:
    LABEL_3:
        result = 0;
        break;

    default:
    LABEL_6:
        result = 1;
        break;
    }

    return result;
}

bool G_IsPlayerDrivingVehicle(const gentity_s *player)
{
    gclient_s *client; // r11
    int eFlags; // r11
    const EntHandle *p_ownerNum; // r31
    scr_vehicle_s *scr_vehicle; // r11

    iassert(player);

    client = player->client;
    if (client
        && (eFlags = client->ps.eFlags, (eFlags & 0x20000) != 0)
        && (eFlags & 0x80000) == 0
        && (p_ownerNum = &player->r.ownerNum, p_ownerNum->isDefined())
        && (scr_vehicle = p_ownerNum->ent()->scr_vehicle) != 0
        && (scr_vehicle->flags & 0xA) == 0)
    {
        return s_vehicleInfos[scr_vehicle->infoIdx].type != 4;
    }
    else
    {
        return 0;
    }
}

#ifdef KISAK_SP

scr_vehicle_s s_vehicles[64]{ 0 };

gentity_s * VEH_GetCollMap(const char *modelname)
{
    const char *targetname; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

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

void VEH_SetupCollmap(gentity_s *ent)
{
    gentity_s *CollMap; // r3

    CollMap = VEH_GetCollMap(SL_ConvertToString(G_ModelName(ent->model)));
    if (CollMap)
    {
        if (CollMap->s.index.item)
        {
            ent->s.index.item = CollMap->s.index.item;
            SV_SetBrushModel(ent);
            ent->r.contents = 0x800000;
            if (ent->spawnflags & 1)
                ent->r.contents = 0xA00000;
        }
        else
        {
            Com_PrintWarning(15, "WARNING: Cannot use empty vehicle collmap for [%s]\n", SL_ConvertToString(G_ModelName(ent->model)));
        }
    }
}

#ifdef KISAK_SP
static void G_UpdateVehicleTags(gentity_s *ent)
{
    scr_vehicle_s *scr_vehicle; // r30
    int *flash; // r28
    unsigned __int16 **v4; // r29
    int BoneIndex; // r3
    int *wheel; // r29
    unsigned __int16 **v7; // r30
    int v8; // r3

    iassert(ent);
    iassert(ent->scr_vehicle);

    scr_vehicle = ent->scr_vehicle;
    scr_vehicle->boneIndex.player = SV_DObjGetBoneIndex(ent, scr_const.tag_player);
    scr_vehicle->boneIndex.detach = SV_DObjGetBoneIndex(ent, scr_const.tag_detach);
    scr_vehicle->boneIndex.popout = SV_DObjGetBoneIndex(ent, scr_const.tag_popout);
    scr_vehicle->boneIndex.body = SV_DObjGetBoneIndex(ent, scr_const.tag_body);
    scr_vehicle->boneIndex.turret = SV_DObjGetBoneIndex(ent, scr_const.tag_turret);
    scr_vehicle->boneIndex.barrel = SV_DObjGetBoneIndex(ent, scr_const.tag_barrel);
    scr_vehicle->boneIndex.turret_base = SV_DObjGetBoneIndex(ent, scr_const.tag_turret_base);
    flash = scr_vehicle->boneIndex.flash;
    v4 = s_flashTags;
    do
    {
        BoneIndex = SV_DObjGetBoneIndex(ent, **v4++);
        *flash++ = BoneIndex;
    } while ((int)v4 < (int)s_vehicleFields);
    wheel = scr_vehicle->boneIndex.wheel;
    v7 = s_wheelTags;
    do
    {
        v8 = SV_DObjGetBoneIndex(ent, **v7++);
        *wheel++ = v8;
    } while ((int)v7 < (int)s_flashTags);
}

static const char *s_vehicleTypeNames[6] = { "4 wheel", "tank", "plane", "boat", "artillery", "helicopter" };

int VEH_ParseSpecificField(unsigned __int8 *pStruct, const char *pValue, int fieldType)
{
    const char *v6; // r3
    int v8; // r30
    const char **v9; // r31

    if (fieldType == 12)
    {
        v8 = 0;
        v9 = (const char **)s_vehicleTypeNames;
        while (I_stricmp(pValue, *v9))
        {
            ++v9;
            ++v8;
            if ((int)v9 >= (int)s_wheelTags)
                goto LABEL_10;
        }
        *((_WORD *)pStruct + 32) = v8;
    LABEL_10:
        if (v8 == 6)
            Com_Error(ERR_DROP, "Unknown vehicle type [%s]", pValue); // lwss: omg iw forgot the % here. wow what an epic find.
        return 1;
    }
    else
    {
        if (!alwaysfails)
        {
            v6 = va("Bad vehicle field type %i\n", fieldType);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 437, 0, v6);
        }
        Com_Error(ERR_DROP, "Bad vehicle field type %i", fieldType);
        return 0;
    }
}

static void VEH_Strcpy(unsigned char *pMember, const char *pKeyValue)
{
    int v2; // r10
    int v3; // r11

    v2 = pMember - (unsigned char *)pKeyValue;
    do
    {
        v3 = *(unsigned char *)pKeyValue;
        ((char*)pKeyValue++)[v2] = v3;
    } while (v3);
}

int __fastcall G_LoadVehicle(const char *name)
{
    const char *InfoString; // r28
    int v3; // r27
    vehicle_info_t *v4; // r31
    char *v5; // r11
    int v6; // r9
    int result; // r3
    double collisionSpeed; // fp12
    unsigned __int16 *sndIndices; // r30
    double maxSpeed; // fp11
    int v11; // r28
    double engineSndSpeed; // fp10
    const char *v13; // r31
    char v14[64]; // [sp+50h] [-2080h] BYREF
    char v15[64]; // [sp+90h] [-2040h] BYREF

    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 489, 0, "%s", "name");
    sprintf(v14, "vehicles/%s", name);
    InfoString = Com_LoadInfoString(v14, "vehicle file", "VEHICLEFILE", v15);
    v3 = s_numVehicleInfos;
    v4 = &s_vehicleInfos[s_numVehicleInfos];
    memset(v4, 0, sizeof(vehicle_info_t));
    v5 = (char*)name;
    do
    {
        v6 = *(unsigned __int8 *)v5;
        (v5++)[(char *)v4 - name] = v6;
    } while (v6);
    if (!ParseConfigStringToStruct(
        (unsigned __int8 *)v4,
        s_vehicleFields,
        33,
        (char*)InfoString,
        13,
        VEH_ParseSpecificField,
        VEH_Strcpy))
        return -1;
    collisionSpeed = v4->collisionSpeed;
    sndIndices = (unsigned short*)v4->sndIndices;
    maxSpeed = v4->maxSpeed;
    v11 = 6;
    engineSndSpeed = v4->engineSndSpeed;
    v4->accel = v4->accel * (float)17.6;
    v4->collisionSpeed = (float)collisionSpeed * (float)17.6;
    v4->maxSpeed = (float)maxSpeed * (float)17.6;
    v4->engineSndSpeed = (float)engineSndSpeed * (float)17.6;
    v13 = v4->sndNames[0];
    do
    {
        if (*v13)
            *sndIndices = G_SoundAliasIndexPermanent(v13);
        else
            *sndIndices = 0;
        --v11;
        ++sndIndices;
        v13 += 64;
    } while (v11);
    result = v3;
    ++s_numVehicleInfos;
    return result;
}
int __fastcall VEH_GetVehicleInfoFromName(const char *name)
{
    int v2; // r30
    vehicle_info_t *v3; // r29
    int result; // r3
    vehicle_info_t *v5; // r29

    if (!name || !*name)
        return -1;
    v2 = 0;
    if (s_numVehicleInfos > 0)
    {
        v3 = s_vehicleInfos;
        while (I_stricmp(name, v3->name))
        {
            ++v2;
            ++v3;
            if (v2 >= s_numVehicleInfos)
                goto LABEL_7;
        }
        return v2;
    }
LABEL_7:
    result = G_LoadVehicle(name);
    if (result < 0)
    {
        Com_PrintWarning(15, "WARNING: couldn't find vehicle info for '%s', attempting to use 'defaultvehicle'.\n", name);
        v2 = 0;
        if (s_numVehicleInfos > 0)
        {
            v5 = s_vehicleInfos;
            while (I_stricmp("defaultvehicle", v5->name))
            {
                ++v2;
                ++v5;
                if (v2 >= s_numVehicleInfos)
                    goto LABEL_12;
            }
            return v2;
        }
    LABEL_12:
        result = G_LoadVehicle("defaultvehicle");
        if (result < 0)
        {
            Com_Error(ERR_DROP, "Cannot find vehicle info for 'defaultvehicle'");
            return -1;
        }
    }
    return result;
}

static int VEH_DObjHasRequiredTags(gentity_s *ent, int infoIdx)
{
    int type; // r11
    int v4; // r30
    int v5; // r29
    unsigned __int16 **i; // r31

    type = s_vehicleInfos[infoIdx].type;
    if (s_vehicleInfos[infoIdx].type && type != 1)
        return 1;
    v4 = 0;
    v5 = type == 0 ? 4 : 6;
    if (v5 <= 0)
        return 1;
    for (i = s_wheelTags; SV_DObjGetBoneIndex(ent, **i) >= 0; ++i)
    {
        if (++v4 >= v5)
            return 1;
    }
    return 0;
}
static void VEH_InitModelAndValidateTags(gentity_s *ent, int *infoIdx)
{
    int VehicleInfoFromName; // r28
    char v5; // r27
    unsigned int v6; // r3
    const char *v7; // r3

    VehicleInfoFromName = VEH_GetVehicleInfoFromName("defaultvehicle");
    v5 = 0;
    if (*infoIdx == VehicleInfoFromName)
    {
        v5 = 1;
        G_SetModel(ent, "defaultvehicle");
    LABEL_6:
        *infoIdx = VehicleInfoFromName;
        goto LABEL_7;
    }
    if (ent->model && G_XModelBad(ent->model))
    {
        v5 = 1;
        G_OverrideModel(ent->model, "defaultvehicle");
        goto LABEL_6;
    }
LABEL_7:
    G_DObjUpdate(ent);
    if (!VEH_DObjHasRequiredTags(ent, *infoIdx))
    {
        if (v5)
            Com_Error(ERR_DROP, "ERROR: default vehicle is missing a required tag!");
        v6 = G_ModelName(ent->model);
        v7 = SL_ConvertToString(v6);
        Com_PrintWarning(
            15,
            "WARNING: vehicle '%s' is missing a required tag! switching to default vehicle model and info.\n",
            v7);
        G_SetModel(ent, "defaultvehicle");
        *infoIdx = VehicleInfoFromName;
        G_DObjUpdate(ent);
        if (!VEH_DObjHasRequiredTags(ent, *infoIdx))
            Com_Error(ERR_DROP, "ERROR: default vehicle is missing a required tag!");
    }
}
#endif

void G_SpawnVehicle(gentity_s *ent, const char *typeName, int load)
{
    int vehCount; // r9
    //int *i; // r11
    int v9; // r24
    int WeaponIndexForName; // r3
    int number; // r11
    int v13; // r11
    int v14; // r11
    int VehicleInfoFromName; // [sp+50h] [-60h] BYREF

    vehCount = 0;

    scr_vehicle_s *veh;
    int i;

    for (i = 0; i < ARRAY_COUNT(s_vehicles); i++)
    {
        veh = &s_vehicles[i];
        if (veh->entNum == 2175)
        {
            break;
        }
    }   

    if (vehCount >= 64)
        Com_Error(ERR_DROP, "Hit max vehicle count [%d]", 64);

    VehicleInfoFromName = VEH_GetVehicleInfoFromName(typeName);

    if (VehicleInfoFromName < 0)
        Com_Error(ERR_DROP, "Can't find info for script vehicle [%s]", typeName);

    VEH_InitModelAndValidateTags(ent, &VehicleInfoFromName);
    v9 = VehicleInfoFromName;
    if (!level.initializing)
    {
        WeaponIndexForName = G_GetWeaponIndexForName(s_vehicleInfos[VehicleInfoFromName].turretWeapon);
        if (!IsItemRegistered(WeaponIndexForName))
        {
            Scr_Error(va("vehicle '%s' not precached", s_vehicleInfos[v9].name));
        }
    }
    memset(veh, 0, sizeof(scr_vehicle_s));
    veh->targetEnt = 2175;
    number = veh->lookAtEnt.number;
    iassert(!number || g_entities[number - 1].r.inuse);
    iassert(!veh->lookAtEnt.isDefined());

    number = veh->idleSndEnt.number;

    iassert(!number || g_entities[number - 1].r.inuse);
    iassert(!veh->idleSndEnt.isDefined());

    number = veh->engineSndEnt.number;

    iassert(!number || g_entities[number - 1].r.inuse);
    iassert(!veh->engineSndEnt.isDefined());

    VEH_InitEntity(ent, veh, v9);
    VEH_InitVehicle(ent, veh, v9);

    if (!load)
        VEH_SetupCollmap(ent);

    G_UpdateVehicleTags(ent);
}

const char* G_GetVehicleInfoName(__int16 index)
{
    iassert(index >= 0);
    iassert(index < s_numVehicleInfos);
    
    return s_vehicleInfos[index].name;
}

int G_GetVehicleInfoIndex(const char *name)
{
    int index; // r29

    iassert(name && name[0]);

    index = VEH_GetVehicleInfoFromName(name);

    iassert(index != -1);

    return index;
}

bool G_IsVehicleUsable(gentity_s *ent, gentity_s *player)
{
    gclient_s *client; // r11

    client = player->client;
    if (!client
        || (client->ps.eFlags & 0x20000) != 0
        || player->r.ownerNum.isDefined()
        || ent->r.ownerNum.isDefined()
        || ent->scr_vehicle->speed > 1.0
        || ent->health <= 0)
    {
        return 0;
    }
    else
    {
        return ((unsigned int)ent->r.contents >> 21) & 1;
    }
}

void G_PrecacheDefaultVehicle()
{
    int VehicleInfoFromName; // r3

    G_ModelIndex("defaultvehicle");
    VehicleInfoFromName = VEH_GetVehicleInfoFromName("defaultvehicle");
    G_GetWeaponIndexForName(s_vehicleInfos[VehicleInfoFromName].turretWeapon);
}

void G_FreeVehicleRefs(gentity_s *ent)
{
    int *p_targetEnt; // r11

    if ((ent->flags & 0x800000) != 0)
    {
        p_targetEnt = &s_vehicles[0].targetEnt;
        do
        {
            if (*(p_targetEnt - 49) != 2175 && *p_targetEnt == ent->s.number)
            {
                *p_targetEnt = 2175;
                *(p_targetEnt - 5) = 0;
            }
            if (p_targetEnt[157] != 2175 && p_targetEnt[206] == ent->s.number)
            {
                p_targetEnt[206] = 2175;
                p_targetEnt[201] = 0;
            }
            if (p_targetEnt[363] != 2175 && p_targetEnt[412] == ent->s.number)
            {
                p_targetEnt[412] = 2175;
                p_targetEnt[407] = 0;
            }
            if (p_targetEnt[569] != 2175 && p_targetEnt[618] == ent->s.number)
            {
                p_targetEnt[618] = 2175;
                p_targetEnt[613] = 0;
            }
            p_targetEnt += 824;
        } while ((uintptr_t)p_targetEnt < (uintptr_t)&s_vehicles[64]);
    }
}

gentity_s *G_GetPlayerVehicle(const gentity_s *player)
{
    gclient_s *client; // r28
    const EntHandle *p_ownerNum; // r29
    gentity_s *v5; // r29

    if (!player)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 3976, 0, "%s", "player");
    client = player->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 3979, 0, "%s", "client");
    if ((client->ps.eFlags & 0x20000) == 0)
        return 0;
    p_ownerNum = &player->r.ownerNum;
    iassert(player->r.ownerNum.isDefined());
    v5 = p_ownerNum->ent();
    if (!v5->scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 3988, 0, "%s", "vehEnt->scr_vehicle");
    if ((v5->scr_vehicle->flags & 1) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp",
            3989,
            0,
            "%s",
            "vehEnt->scr_vehicle->flags & VEHFLAG_PLAYER");
    return v5;
}

void G_InitScrVehicles()
{
    int i; // r31

    for (i = 0; i < 64; i = (__int16)(i + 1))
    {
        G_VehInitPathPos(&s_vehicles[i].pathPos);
        s_vehicles[i].entNum = 2175;
    }
    level.vehicles = s_vehicles;
}

void G_SetupScrVehicles()
{
    int i; // r31
    int entNum; // r3
    gentity_s *Vehicle; // r3

    for (i = 0; i < 64; i = (__int16)(i + 1))
    {
        entNum = s_vehicles[i].entNum;
        if (entNum != 2175)
        {
            Vehicle = VEH_GetVehicle(entNum);
            VEH_SetupCollmap(Vehicle);
        }
    }
}

void G_FreeScrVehicles()
{
    int i; // r31

    for (i = 0; i < 64; i = (__int16)(i + 1))
        G_VehFreePathPos(&s_vehicles[i].pathPos);
}

void G_RestartScrVehicleInfo()
{
    int v0; // r27
    char *v1; // r28
    _WORD *v2; // r31
    const char *v3; // r30
    int v4; // r29

    v0 = 0;
    if (s_numVehicleInfos > 0)
    {
        v1 = s_vehicleInfos[0].sndNames[0];
        do
        {
            //v2 = v1 + 384;
            v2 = (_WORD*)(v1 + 380);
            v3 = v1;
            v4 = 6;
            do
            {
                if (*v2)
                    *v2 = G_SoundAliasIndexPermanent(v3);
                --v4;
                ++v2;
                v3 += 64;
            } while (v4);
            ++v0;
            //v1 += 628;
            v1 += sizeof(vehicle_info_t);
        } while (v0 < s_numVehicleInfos);
    }
}

void G_ParseScrVehicleInfo()
{
    s_numVehicleInfos = 0;
}

void  VEH_ResetWheels(gentity_s *ent, vehicle_physic_t *phys)
{
    float *wheelZPos; // r31
    int v5; // r29
    int *wheel; // r30
    float v7[16]; // [sp+50h] [-40h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 643, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 644, 0, "%s", "ent->scr_vehicle");
    if (!phys)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 645, 0, "%s", "phys");
    wheelZPos = phys->wheelZPos;
    v5 = 6;
    wheel = ent->scr_vehicle->boneIndex.wheel;
    do
    {
        if (*wheel >= 0)
        {
            G_DObjGetWorldBoneIndexPos(ent, *wheel, v7);
            *wheelZPos = v7[2];
        }
        --v5;
        ++wheel;
        ++wheelZPos;
    } while (v5);
}

void __fastcall VEH_GetWheelOrigin(gentity_s *ent, int idx, float *origin)
{
    int v5; // r28
    scr_vehicle_s *scr_vehicle; // r29
    DObjAnimMat *mtx; // r30
    double v10; // fp11
    int flags; // r11
    double v12; // fp12
    double v13; // fp0
    double v15; // fp9
    double v17; // fp0
    double v18; // fp13
    double v19; // fp12
    double v20; // fp0
    double v21; // fp11
    double v22; // fp0

    v5 = 4 * (idx + 198);
    scr_vehicle = ent->scr_vehicle;
    if (*(int *)((char *)&scr_vehicle->pathPos.nodeIdx + v5) < 0)
    {
        Com_Error(ERR_DROP, "Script vehicle [%s] needs [%s]", SL_ConvertToString(ent->targetname), SL_ConvertToString(*s_wheelTags[idx]));
    }
    mtx = G_DObjGetLocalBoneIndexMatrix(ent, *(_DWORD *)((char *)&scr_vehicle->pathPos.nodeIdx + v5));
    if (!mtx)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 619, 0, "%s", "mtx");
    *origin = mtx->trans[0];
    origin[1] = mtx->trans[1];
    v10 = mtx->trans[2];
    origin[2] = mtx->trans[2];
    flags = scr_vehicle->flags;
    if ((flags & 8) == 0 && (flags & 1) != 0)
    {
        v12 = origin[1];
        v13 = (float)((float)((float)v10 * (float)v10) + (float)((float)(*origin * *origin) + (float)(origin[1] * origin[1])));
        if (v13 > (float)(scr_vehicle->phys.maxs[0] * scr_vehicle->phys.maxs[0]))
        {
            // aislop
            //float _FP11 = -__fsqrts(v13);
            //__asm { fsel      f0, f11, f13, f0 }
            //v17 = (float)((float)1.0 / (float)_FP0);
            {
                float _FP0, _FP11, v17;
                float f13 = 1.0f;

                _FP11 = -sqrtf(v13);  // same as -__fsqrts(v13)
                _FP0 = (_FP11 >= 0.0f) ? f13 : _FP11;  // emulate fsel
                v17 = 1.0f / _FP0;
            }

            v15 = origin[2];
            v18 = (float)(*origin * (float)v17);
            *origin = *origin * (float)v17;
            v19 = (float)((float)v12 * (float)v17);
            origin[1] = v19;
            v20 = (float)((float)v17 * (float)v15);
            origin[2] = v20;
            v21 = v20;
            v22 = (float)(scr_vehicle->phys.maxs[0] - (float)2.0);
            *origin = (float)v18 * (float)(scr_vehicle->phys.maxs[0] - (float)2.0);
            origin[1] = (float)v19 * (float)v22;
            origin[2] = (float)v22 * (float)v21;
        }
    }
}

// KISAKTODO: my god, just merge these with the MP version
void __fastcall VEH_GroundPlant(gentity_s *ent, vehicle_physic_t *phys, int gravity)
{
    scr_vehicle_s *scr_vehicle; // r19
    vehicle_info_t *v7; // r20
    int v8; // r23
    int v9; // r28
    float *angles; // r24
    int v11; // r29
    float *v12; // r30
    float *wheelZVel; // r31
    const float *v14; // r6
    const dvar_s *v15; // r11
    const float *v16; // r6
    double fraction; // fp0
    double v18; // fp13
    double v19; // fp10
    double v20; // fp12
    double v21; // fp9
    double v22; // fp11
    double v23; // fp8
    double v24; // fp10
    double v25; // fp12
    double v26; // fp0
    double v27; // fp13
    double v28; // fp11
    double v29; // fp13
    const dvar_s *v30; // r11
    double v31; // fp0
    double v32; // fp12
    double v33; // fp13
    double v34; // fp9
    double v35; // fp11
    double v36; // fp8
    double v39; // fp10
    double v40; // fp0
    double v43; // fp0
    double v44; // fp27
    int v45; // r10
    double v46; // fp29
    double v47; // fp28
    double v48; // fp30
    double suspensionTravel; // fp0
    float *v50; // r11
    unsigned int v51; // r9
    int v52; // r9
    float *v53; // r11
    double v56; // fp0
    double v59; // fp0
    double v60; // fp1
    int flags; // r11
    const dvar_s *v70; // r11
    float v71; // [sp+50h] [-2A0h] BYREF
    float v72; // [sp+54h] [-29Ch]
    float v73; // [sp+58h] [-298h]
    float v74; // [sp+60h] [-290h] BYREF
    float v75; // [sp+64h] [-28Ch]
    float v76; // [sp+68h] [-288h]
    float v77; // [sp+70h] [-280h] BYREF
    float v78; // [sp+74h] [-27Ch]
    float v79; // [sp+78h] [-278h]
    float v80; // [sp+7Ch] [-274h]
    float v81; // [sp+80h] [-270h] BYREF
    float v82; // [sp+84h] [-26Ch]
    float v83; // [sp+88h] [-268h]
    float v84; // [sp+90h] [-260h] BYREF
    float v85; // [sp+94h] [-25Ch]
    float v86; // [sp+98h] [-258h]
    float v87; // [sp+A0h] [-250h] BYREF
    float v88; // [sp+A4h] [-24Ch]
    float v89; // [sp+A8h] [-248h]
    float v90[3]; // [sp+B0h] [-240h] BYREF
    float v91; // [sp+BCh] [-234h] BYREF
    float v92; // [sp+C0h] [-230h]
    float v93; // [sp+C4h] [-22Ch]
    float v94; // [sp+D4h] [-21Ch]
    float v95; // [sp+D8h] [-218h]
    float v96; // [sp+DCh] [-214h]
    float v97[3]; // [sp+E0h] [-210h] BYREF
    float v98[3]; // [sp+ECh] [-204h] BYREF
    float v99[3]; // [sp+F8h] [-1F8h] BYREF
    float v100[3]; // [sp+104h] [-1ECh] BYREF
    float v101; // [sp+110h] [-1E0h] BYREF
    float v102; // [sp+114h] [-1DCh]
    float v103; // [sp+118h] [-1D8h] BYREF
    float v104; // [sp+11Ch] [-1D4h]
    float v105; // [sp+120h] [-1D0h]
    float v106; // [sp+124h] [-1CCh] BYREF
    float v107; // [sp+128h] [-1C8h]
    float v108; // [sp+12Ch] [-1C4h]
    float v109; // [sp+130h] [-1C0h]
    float v110; // [sp+134h] [-1BCh]
    float v111; // [sp+138h] [-1B8h]
    float v112; // [sp+13Ch] [-1B4h]
    float v113[6]; // [sp+158h] [-198h] BYREF
    float v114[4]; // [sp+170h] [-180h] BYREF
    float v115[4]; // [sp+180h] [-170h] BYREF
    float v116[4]; // [sp+190h] [-160h] BYREF
    float v117[4]; // [sp+1A0h] [-150h] BYREF
    float v118[4]; // [sp+1B0h] [-140h] BYREF
    float v119[4]; // [sp+1C0h] [-130h] BYREF
    float v120[4]; // [sp+1D0h] [-120h] BYREF
    float v121[4]; // [sp+1E0h] [-110h] BYREF
    float v122[4]; // [sp+1F0h] [-100h] BYREF
    trace_t v123; // [sp+200h] [-F0h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 1548, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 1549, 0, "%s", "ent->scr_vehicle");
    if (!phys)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 1550, 0, "%s", "phys");
    scr_vehicle = ent->scr_vehicle;
    v7 = &s_vehicleInfos[scr_vehicle->infoIdx];
    if (v7->type && v7->type != 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp",
            1555,
            0,
            "%s",
            "(info->type == VEH_WHEELS_4) || (info->type == VEH_TANK)");
    v8 = 529;
    v9 = v7->type == 0 ? 4 : 6;
    if ((scr_vehicle->flags & 1) != 0)
        v8 = 66065;
    angles = phys->angles;
    v94 = phys->origin[0];
    v95 = phys->origin[1];
    v96 = phys->prevOrigin[2];
    AnglesToAxis(phys->angles, (float (*)[3])v90);
    v11 = 0;
    if (v9 > 0)
    {
        v12 = &v101;
        wheelZVel = phys->wheelZVel;
        do
        {
            VEH_GetWheelOrigin(ent, v11, v122);
            MatrixTransformVector43(v122, (const mat4x3&)v90, &v74);
            v15 = g_vehicleDebug;
            if (g_vehicleDebug->current.enabled)
            {
                v114[0] = 1.0;
                v114[1] = 0.0;
                v114[2] = 0.0;
                v114[3] = 1.0;
                v113[0] = -2.0;
                v113[1] = -2.0;
                v113[2] = -2.0;
                v77 = 2.0;
                v78 = 2.0;
                v79 = 2.0;
                G_DebugBox(&v74, v113, &v77, 0.0, v14, (int)v114, 1);
                v15 = g_vehicleDebug;
            }
            v87 = v74;
            v71 = v74;
            v88 = v75;
            v72 = v75;
            v89 = v76 + (float)64.0;
            v73 = v76 - (float)256.0;
            if (v15->current.enabled)
            {
                v120[0] = 0.0;
                v120[1] = 0.0;
                v120[2] = 1.0;
                v120[3] = 1.0;
                G_DebugLine(&v87, &v71, v120, 1);
            }
            G_TraceCapsule(&v123, &v87, vec3_origin, vec3_origin, &v71, ent->s.number, v8);
            fraction = v123.fraction;
            if (v123.fraction >= 1.0)
            {
                v24 = v71;
                wheelZVel[12] = 0.0;
                v25 = v72;
                v26 = v73;
            }
            else
            {
                v18 = v87;
                v19 = (float)(v71 - v87);
                v20 = v88;
                v21 = (float)(v72 - v88);
                v22 = v89;
                v23 = (float)(v73 - v89);
                *((_DWORD *)wheelZVel + 12) = (v123.surfaceFlags >> 20) & 0x1F;
                v24 = (float)((float)((float)v19 * (float)fraction) + (float)v18);
                v25 = (float)((float)((float)v21 * (float)fraction) + (float)v20);
                v26 = (float)((float)((float)v23 * (float)fraction) + (float)v22);
            }
            v76 = v26;
            v75 = v25;
            v74 = v24;
            if (!gravity
                || (v27 = (float)(*wheelZVel - (float)40.0),
                    v28 = wheelZVel[6],
                    *wheelZVel = *wheelZVel - (float)40.0,
                    v29 = (float)((float)((float)v27 * (float)0.050000001) + (float)v28),
                    wheelZVel[6] = v29,
                    v29 < v26))
            {
                *wheelZVel = 0.0;
                wheelZVel[6] = v26;
            }
            v30 = g_vehicleDebug;
            v31 = wheelZVel[6];
            *v12 = v24;
            v12[1] = v25;
            v12[2] = v31;
            if (v30->current.enabled)
            {
                v116[0] = 0.0;
                v116[1] = 1.0;
                v116[2] = 0.0;
                v116[3] = 1.0;
                v84 = -2.0;
                v85 = -2.0;
                v86 = -2.0;
                v81 = 2.0;
                v82 = 2.0;
                v83 = 2.0;
                G_DebugBox(v12, &v84, &v81, 0.0, v16, (int)v116, 1);
            }
            ++v11;
            ++wheelZVel;
            v12 += 3;
        } while (v11 < v9);
    }

    // aislop
    //v32 = (float)((float)((float)(v112 + v106) * (float)0.5) - (float)((float)(v109 + v103) * (float)0.5));
    //v33 = (float)((float)((float)(v104 + v110) * (float)0.5) - (float)((float)(v107 + v101) * (float)0.5));
    //v34 = (float)((float)((float)(v106 + v103) * (float)0.5) - (float)((float)(v109 + v112) * (float)0.5));
    //v35 = (float)((float)((float)(v111 + v105) * (float)0.5) - (float)((float)(v108 + v102) * (float)0.5));
    //v36 = (float)((float)((float)(v105 + v102) * (float)0.5) - (float)((float)(v108 + v111) * (float)0.5));
    //_FP0 = -__fsqrts((float)((float)((float)v35 * (float)v35)
    //    + (float)((float)((float)v33 * (float)v33) + (float)((float)v32 * (float)v32))));
    //__asm { fsel      f0, f0, f31, f10 }
    //v39 = (float)((float)((float)(v104 + v101) * (float)0.5) - (float)((float)(v107 + v110) * (float)0.5));
    //v40 = (float)((float)1.0 / (float)_FP0);
    //v81 = (float)v40 * (float)((float)((float)(v104 + v110) * (float)0.5) - (float)((float)(v107 + v101) * (float)0.5));
    //v82 = (float)((float)((float)(v111 + v105) * (float)0.5) - (float)((float)(v108 + v102) * (float)0.5)) * (float)v40;
    //v83 = (float)((float)((float)(v112 + v106) * (float)0.5) - (float)((float)(v109 + v103) * (float)0.5)) * (float)v40;
    //_FP13 = -__fsqrts((float)((float)((float)v36 * (float)v36)
    //    + (float)((float)((float)v39 * (float)v39) + (float)((float)v34 * (float)v34))));
    //__asm { fsel      f0, f13, f31, f0 }
    //v43 = (float)((float)1.0 / (float)_FP0);
    //v84 = (float)v43 * (float)((float)((float)(v104 + v101) * (float)0.5) - (float)((float)(v107 + v110) * (float)0.5));
    //v85 = (float)((float)((float)(v105 + v102) * (float)0.5) - (float)((float)(v108 + v111) * (float)0.5)) * (float)v43;
    //v86 = (float)((float)((float)(v106 + v103) * (float)0.5) - (float)((float)(v109 + v112) * (float)0.5)) * (float)v43;
    //Vec3Cross(&v81, &v84, &v77);
    {
        // Compute vector components
        v32 = ((v112 + v106) * 0.5f) - ((v109 + v103) * 0.5f);
        v33 = ((v104 + v110) * 0.5f) - ((v107 + v101) * 0.5f);
        v34 = ((v106 + v103) * 0.5f) - ((v109 + v112) * 0.5f);
        v35 = ((v111 + v105) * 0.5f) - ((v108 + v102) * 0.5f);
        v36 = ((v105 + v102) * 0.5f) - ((v108 + v111) * 0.5f);

        // Length of first vector
        float _FP0 = -sqrtf(v35 * v35 + v33 * v33 + v32 * v32);

        // Replace fsel: f0 = (f0 >= 0.0f) ? f31 : f10
        // Assuming f31 = 0.0f and f10 = _FP0 from previous step
        _FP0 = (_FP0 >= 0.0f) ? 0.0f : _FP0;

        // Normalize factor
        v39 = ((v104 + v101) * 0.5f) - ((v107 + v110) * 0.5f);
        v40 = 1.0f / _FP0;

        // Normalized components (v81, v82, v83)
        v81 = v40 * (((v104 + v110) * 0.5f) - ((v107 + v101) * 0.5f));
        v82 = v40 * (((v111 + v105) * 0.5f) - ((v108 + v102) * 0.5f));
        v83 = v40 * (((v112 + v106) * 0.5f) - ((v109 + v103) * 0.5f));

        // Length of second vector
        float _FP13 = -sqrtf(v36 * v36 + v39 * v39 + v34 * v34);

        // Replace fsel: f0 = (f13 >= 0.0f) ? f31 : f0
        // Assuming f31 = 0.0f
        _FP0 = (_FP13 >= 0.0f) ? 0.0f : _FP0;

        // Normalize again using updated _FP0
        v43 = 1.0f / _FP0;

        // Normalized components (v84, v85, v86)
        v84 = v43 * (((v104 + v101) * 0.5f) - ((v107 + v110) * 0.5f));
        v85 = v43 * (((v105 + v102) * 0.5f) - ((v108 + v111) * 0.5f));
        v86 = v43 * (((v106 + v103) * 0.5f) - ((v109 + v112) * 0.5f));

        // Cross product of vectors
        Vec3Cross(&v81, &v84, &v77);

    }

    v44 = v79;
    v45 = 1;
    v46 = v78;
    v47 = v77;
    v48 = (float)((float)(v101 * v77) + (float)((float)(v102 * v78) + (float)(v103 * v79)));
    v80 = (float)(v101 * v77) + (float)((float)(v102 * v78) + (float)(v103 * v79));
    if (v9 - 1 >= 4)
    {
        suspensionTravel = v7->suspensionTravel;
        v50 = &v106;
        v51 = ((unsigned int)(v9 - 5) >> 2) + 1;
        v45 = 4 * v51 + 1;
        do
        {
            if ((float)((float)((float)(*v50 * v79) + (float)((float)(*(v50 - 1) * v78) + (float)(*(v50 - 2) * v77)))
                - (float)v48) > suspensionTravel)
                v48 = (float)((float)((float)(*v50 * v79) + (float)((float)(*(v50 - 1) * v78) + (float)(*(v50 - 2) * v77)))
                    - v7->suspensionTravel);
            if ((float)((float)((float)(v50[3] * v79) + (float)((float)(v50[2] * v78) + (float)(v50[1] * v77))) - (float)v48) > suspensionTravel)
                v48 = (float)((float)((float)(v50[3] * v79) + (float)((float)(v50[2] * v78) + (float)(v50[1] * v77)))
                    - v7->suspensionTravel);
            if ((float)((float)((float)(v50[6] * v79) + (float)((float)(v50[5] * v78) + (float)(v50[4] * v77))) - (float)v48) > suspensionTravel)
                v48 = (float)((float)((float)(v50[6] * v79) + (float)((float)(v50[5] * v78) + (float)(v50[4] * v77)))
                    - v7->suspensionTravel);
            if ((float)((float)((float)(v50[9] * v79) + (float)((float)(v50[8] * v78) + (float)(v50[7] * v77))) - (float)v48) > suspensionTravel)
                v48 = (float)((float)((float)(v50[9] * v79) + (float)((float)(v50[8] * v78) + (float)(v50[7] * v77)))
                    - v7->suspensionTravel);
            --v51;
            v50 += 12;
        } while (v51);
        v80 = v48;
    }
    if (v45 < v9)
    {
        v52 = v9 - v45;
        v53 = &v103 + 3 * v45;
        do
        {
            if ((float)((float)((float)(*v53 * v79) + (float)((float)(*(v53 - 1) * v78) + (float)(*(v53 - 2) * v77)))
                - (float)v48) > (double)v7->suspensionTravel)
                v48 = (float)((float)((float)(*v53 * v79) + (float)((float)(*(v53 - 1) * v78) + (float)(*(v53 - 2) * v77)))
                    - v7->suspensionTravel);
            --v52;
            v53 += 3;
        } while (v52);
        v80 = v48;
    }
    // aislop
    //Vec3Cross(&v77, v90, &v91);
    //_FP10 = -__fsqrts((float)((float)(v93 * v93) + (float)((float)(v91 * v91) + (float)(v92 * v92))));
    //__asm { fsel      f0, f10, f31, f0 }
    //v56 = (float)((float)1.0 / (float)_FP0);
    //v91 = v91 * (float)v56;
    //v92 = v92 * (float)v56;
    //v93 = v93 * (float)v56;
    //Vec3Cross(&v91, &v77, v90);
    //_FP10 = -__fsqrts((float)((float)(v90[1] * v90[1]) + (float)((float)(v90[0] * v90[0]) + (float)(v90[2] * v90[2]))));
    //__asm { fsel      f0, f10, f31, f0 }
    //v59 = (float)((float)1.0 / (float)_FP0);
    //v90[0] = v90[0] * (float)v59;
    //v90[1] = v90[1] * (float)v59;
    //v90[2] = v90[2] * (float)v59;
    //AxisToAngles((const float (*)[3])v90, v121);
    //*angles = DiffTrackAngle(v121[0], phys->prevAngles[0], 6.0, 0.050000001);
    //v60 = DiffTrackAngle(v121[2], phys->prevAngles[2], 6.0, 0.050000001);
    //_FP11 = (float)(*angles - (float)60.0);
    //_FP10 = (float)((float)-60.0 - *angles);
    //__asm { fsel      f12, f11, f0, f12 }
    //_FP11 = (float)((float)-60.0 - (float)v60);
    //__asm { fsel      f12, f10, f13, f12 }
    //*angles = _FP12;
    //_FP12 = (float)((float)v60 - (float)60.0);
    //__asm
    //{
    //    fsel      f0, f12, f0, f1
    //    fsel      f0, f11, f13, f0
    //}
    //phys->angles[2] = _FP0;

    {
        Vec3Cross(&v77, v90, &v91);

        // Replace __fsqrts with sqrtf
        float _FP10 = -sqrtf((v93 * v93) + (v91 * v91) + (v92 * v92));

        // Replace fsel with ternary: f0 = (f10 >= 0) ? f31 : f0
        float _FP0 = (_FP10 >= 0.0f) ? 0.0f : _FP10;

        v56 = 1.0f / _FP0;
        v91 = v91 * v56;
        v92 = v92 * v56;
        v93 = v93 * v56;

        Vec3Cross(&v91, &v77, v90);

        _FP10 = -sqrtf((v90[1] * v90[1]) + (v90[0] * v90[0]) + (v90[2] * v90[2]));
        _FP0 = (_FP10 >= 0.0f) ? 0.0f : _FP10;

        v59 = 1.0f / _FP0;
        v90[0] = v90[0] * v59;
        v90[1] = v90[1] * v59;
        v90[2] = v90[2] * v59;

        AxisToAngles((const mat3x3&)v90, v121);

        *angles = DiffTrackAngle(v121[0], phys->prevAngles[0], 6.0f, 0.05f);
        v60 = DiffTrackAngle(v121[2], phys->prevAngles[2], 6.0f, 0.05f);

        float _FP11 = *angles - 60.0f;
        _FP10 = -60.0f - *angles;

        float _FP12 = (_FP11 >= 0.0f) ? 0.0f : ((_FP10 >= 0.0f) ? -60.0f : *angles);
        *angles = _FP12;

        _FP12 = v60 - 60.0f;
        _FP11 = -60.0f - v60;

        _FP0 = (_FP12 >= 0.0f) ? 0.0f : ((_FP11 >= 0.0f) ? -60.0f : v60);

        phys->angles[2] = _FP0;
    }

    flags = scr_vehicle->flags;
    if (((flags & 8) != 0 || (flags & 1) == 0) && v44 != 0.0)
        phys->origin[2] = (float)((float)((float)(phys->origin[0] * (float)v47) + (float)((float)v46 * phys->origin[1]))
            - (float)v48)
        * (float)((float)-1.0 / (float)v44);
    AnglesSubtract(phys->angles, phys->prevAngles, phys->rotVel);
    v70 = g_vehicleDebug;
    phys->rotVel[0] = phys->rotVel[0] * (float)20.0;
    phys->rotVel[1] = phys->rotVel[1] * (float)20.0;
    phys->rotVel[2] = phys->rotVel[2] * (float)20.0;
    if (v70->current.enabled)
    {
        v97[1] = v102;
        v97[0] = v101;
        v98[1] = v105;
        v98[0] = v104;
        v99[0] = v107;
        v99[1] = v108;
        v100[0] = v110;
        v100[1] = v111;
        v117[0] = 1.0;
        v117[1] = 1.0;
        v117[2] = 0.0;
        v117[3] = 1.0;
        v97[2] = (float)((float)((float)(v102 * (float)v46) + (float)(v101 * (float)v47)) - (float)v48)
            * (float)((float)-1.0 / (float)v44);
        v98[2] = (float)((float)((float)(v105 * (float)v46) + (float)(v104 * (float)v47)) - (float)v48)
            * (float)((float)-1.0 / (float)v44);
        v99[2] = (float)((float)((float)(v108 * (float)v46) + (float)(v107 * (float)v47)) - (float)v48)
            * (float)((float)-1.0 / (float)v44);
        v100[2] = (float)((float)((float)(v111 * (float)v46) + (float)(v110 * (float)v47)) - (float)v48)
            * (float)((float)-1.0 / (float)v44);
        G_DebugLine(v97, v98, v117, 1);
        v118[0] = 1.0;
        v118[1] = 1.0;
        v118[2] = 0.0;
        v118[3] = 1.0;
        G_DebugLine(v98, v100, v118, 1);
        v119[0] = 1.0;
        v119[1] = 1.0;
        v119[2] = 0.0;
        v119[3] = 1.0;
        G_DebugLine(v100, v99, v119, 1);
        v115[0] = 1.0;
        v115[1] = 1.0;
        v115[2] = 0.0;
        v115[3] = 1.0;
        G_DebugLine(v99, v97, v115, 1);
    }
}

void CMD_VEH_AttachPath(scr_entref_t entref)
{
    gentity_s *Vehicle; // r28
    scr_vehicle_s *scr_vehicle; // r30
    vehicle_info_t *v4; // r26
    __int16 VehicleNodeIndex; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    v4 = &s_vehicleInfos[scr_vehicle->infoIdx];
    VehicleNodeIndex = GScr_GetVehicleNodeIndex(0);
    G_VehSetUpPathPos(&scr_vehicle->pathPos, VehicleNodeIndex);
    scr_vehicle->phys.origin[0] = scr_vehicle->pathPos.origin[0];
    scr_vehicle->phys.origin[1] = scr_vehicle->pathPos.origin[1];
    scr_vehicle->phys.origin[2] = scr_vehicle->pathPos.origin[2];
    scr_vehicle->phys.angles[0] = scr_vehicle->pathPos.angles[0];
    scr_vehicle->phys.angles[1] = scr_vehicle->pathPos.angles[1];
    scr_vehicle->phys.angles[2] = scr_vehicle->pathPos.angles[2];
    VEH_SetPosition(Vehicle, scr_vehicle->phys.origin, scr_vehicle->phys.vel, scr_vehicle->phys.angles);
    scr_vehicle->phys.prevOrigin[0] = scr_vehicle->phys.origin[0];
    scr_vehicle->phys.prevOrigin[1] = scr_vehicle->phys.origin[1];
    scr_vehicle->phys.prevOrigin[2] = scr_vehicle->phys.origin[2];
    scr_vehicle->phys.prevAngles[0] = scr_vehicle->phys.angles[0];
    scr_vehicle->phys.prevAngles[1] = scr_vehicle->phys.angles[1];
    scr_vehicle->phys.prevAngles[2] = scr_vehicle->phys.angles[2];
    VEH_ResetWheels(Vehicle, &scr_vehicle->phys);
    if (!v4->type || v4->type == 1)
        VEH_GroundPlant(Vehicle, &scr_vehicle->phys, 0);
    VEH_SetPosition(Vehicle, scr_vehicle->phys.origin, scr_vehicle->phys.vel, scr_vehicle->phys.angles);
    scr_vehicle->phys.prevOrigin[0] = scr_vehicle->phys.origin[0];
    scr_vehicle->phys.prevOrigin[1] = scr_vehicle->phys.origin[1];
    scr_vehicle->phys.prevOrigin[2] = scr_vehicle->phys.origin[2];
    scr_vehicle->phys.prevAngles[0] = scr_vehicle->phys.angles[0];
    scr_vehicle->phys.prevAngles[1] = scr_vehicle->phys.angles[1];
    scr_vehicle->phys.prevAngles[2] = scr_vehicle->phys.angles[2];
}

void CMD_VEH_GetAttachPos(scr_entref_t entref)
{
    gentity_s *Vehicle; // r30
    unsigned int v2; // r4
    scr_vehicle_s *scr_vehicle; // r31
    vehicle_info_t *v4; // r29
    __int16 VehicleNodeIndex; // r3
    vehicle_physic_t v6; // [sp+50h] [-1E0h] BYREF
    vehicle_pathpos_t v7; // [sp+150h] [-E0h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    v4 = &s_vehicleInfos[scr_vehicle->infoIdx];
    VehicleNodeIndex = GScr_GetVehicleNodeIndex(0);
    G_VehSetUpPathPos(&v7, VehicleNodeIndex);
    Com_Memcpy(&v6, &scr_vehicle->phys, 244);
    v6.origin[0] = v7.origin[0];
    v6.origin[1] = v7.origin[1];
    v6.origin[2] = v7.origin[2];
    v6.angles[0] = v7.angles[0];
    v6.angles[1] = v7.angles[1];
    v6.angles[2] = v7.angles[2];
    VEH_ResetWheels(Vehicle, &v6);
    if (!v4->type || v4->type == 1)
        VEH_GroundPlant(Vehicle, &v6, 0);
    Scr_MakeArray();
    Scr_AddVector(v6.origin);
    Scr_AddArray();
    Scr_AddVector(v6.angles);
    Scr_AddArray();
}

void CMD_VEH_StartPath(scr_entref_t entref)
{
    gentity_s *Vehicle; // r31
    scr_vehicle_s *scr_vehicle; // r30
    const char *v3; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    if ((unsigned __int16)scr_vehicle->pathPos.nodeIdx >= 0x8000u)
    {
        v3 = va("Can't start path on a vehicle that hasn't been attached");
        Scr_Error(v3);
    }
    scr_vehicle->flags |= 8u;
    Vehicle->s.lerp.eFlags |= 0x100000u;
}

void CMD_VEH_SetSwitchNode(scr_entref_t entref)
{
    gentity_s *Vehicle; // r11
    vehicle_pathpos_t *p_pathPos; // r31
    __int16 VehicleNodeIndex; // r30
    __int16 v6; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    p_pathPos = &Vehicle->scr_vehicle->pathPos;
    VehicleNodeIndex = GScr_GetVehicleNodeIndex(0);
    v6 = GScr_GetVehicleNodeIndex(1);
    G_VehSetSwitchNode(p_pathPos, VehicleNodeIndex, v6);
}

void CMD_VEH_SetWaitNode(scr_entref_t entref)
{
    gentity_s *Vehicle; // r11
    unsigned int v2; // r4
    scr_vehicle_s *scr_vehicle; // r31

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    scr_vehicle->waitNode = GScr_GetVehicleNodeIndex(0);
}

void CMD_VEH_SetWaitSpeed(scr_entref_t entref)
{
    gentity_s *Vehicle; // r11
    scr_vehicle_s *scr_vehicle; // r31
    double volume; // fp1

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    volume = Scr_GetFloat(0);
    scr_vehicle->waitSpeed = (float)volume * (float)17.6;
    if ((float)((float)volume * (float)17.6) < 0.0)
        Scr_ParamError(0, "Cannot have a negative wait speed on a vehicle");
}

void CMD_VEH_SetSpeedImmediate(scr_entref_t entref)
{
    gentity_s *Vehicle; // r30
    float *p_nodeIdx; // r31
    double v3; // fp12
    double _FP8; // fp8
    double _FP11; // fp11
    double v6; // fp11
    double v7; // fp0
    double v8; // fp13
    double v9; // fp12
    double v10; // fp0
    float v11; // [sp+50h] [-30h] BYREF
    float v12; // [sp+54h] [-2Ch]
    float v13; // [sp+58h] [-28h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    p_nodeIdx = (float *)&Vehicle->scr_vehicle->pathPos.nodeIdx;
    CMD_VEH_Script_SetSpeed(Vehicle);
    v11 = 0.0;
    v12 = 0.0;
    v13 = 0.0;
    if (p_nodeIdx[150] <= 0.0 && p_nodeIdx[167] == 0.0 && p_nodeIdx[168] == 0.0 && p_nodeIdx[169] == 0.0)
        goto LABEL_11;
    v11 = p_nodeIdx[167] - Vehicle->r.currentOrigin[0];
    v12 = p_nodeIdx[168] - Vehicle->r.currentOrigin[1];
    v3 = (float)(p_nodeIdx[169] - Vehicle->r.currentOrigin[2]);
    _FP8 = -sqrtf((float)((float)(v11 * v11) + (float)((float)((float)v3 * (float)v3) + (float)(v12 * v12))));
    //__asm { fsel      f11, f8, f10, f11 }
    if (_FP8 >= 0.0f)
    {
        _FP11 = 1.0f;
    }
    else
    {
        _FP11 = _FP8;
    }
    v6 = (float)((float)1.0 / (float)_FP11);
    v7 = (float)((float)v6 * v11);
    v11 = (float)v6 * v11;
    v8 = (float)(v12 * (float)v6);
    v12 = v12 * (float)v6;
    v9 = (float)((float)v3 * (float)v6);
    v13 = v9;
    if (v7 == 0.0 && v8 == 0.0 && v9 == 0.0)
        LABEL_11:
    AngleVectors(Vehicle->r.currentAngles, &v11, 0, 0);
    v10 = p_nodeIdx[146];
    p_nodeIdx[150] = p_nodeIdx[146];
    p_nodeIdx[71] = (float)v10 * v11;
    p_nodeIdx[72] = v12 * (float)v10;
    p_nodeIdx[73] = v13 * (float)v10;
}

void CMD_VEH_GetGoalSpeedMPH(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    Scr_AddFloat((float)(Vehicle->scr_vehicle->manualSpeed * (float)0.05681818));
}

void CMD_VEH_SetAcceleration(scr_entref_t entref)
{
    gentity_s *Vehicle; // r29
    scr_vehicle_s *scr_vehicle; // r29

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
        if (Vehicle)
            goto LABEL_4;
    }
    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 5044, 0, "%s", "ent");
LABEL_4:
    scr_vehicle = Vehicle->scr_vehicle;
    if (!scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 5047, 0, "%s", "veh");
    scr_vehicle->manualAccel = Scr_GetFloat(0) * (float)17.6;
}

void CMD_VEH_SetDeceleration(scr_entref_t entref)
{
    gentity_s *Vehicle; // r29
    scr_vehicle_s *scr_vehicle; // r29

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
        if (Vehicle)
            goto LABEL_4;
    }
    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 5067, 0, "%s", "ent");
LABEL_4:
    scr_vehicle = Vehicle->scr_vehicle;
    if (!scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 5070, 0, "%s", "veh");
    scr_vehicle->manualDecel = Scr_GetFloat(0) * (float)17.6;
}

void CMD_VEH_SetJitterParams(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    scr_vehicle_s *scr_vehicle; // r31
    double volume; // fp1
    double v4; // fp1

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    Scr_GetVector(0, scr_vehicle->jitter.jitterOffsetRange);
    if (Scr_GetNumParam() <= 1)
        volume = 0.5;
    else
        volume = Scr_GetFloat(1u);
    scr_vehicle->jitter.jitterPeriodMin = (int)(float)((float)volume * (float)1000.0);
    if (Scr_GetNumParam() <= 2)
        v4 = 1.0;
    else
        v4 = Scr_GetFloat(2u);
    scr_vehicle->jitter.jitterEndTime = 0;
    scr_vehicle->jitter.jitterPeriodMax = (int)(float)((float)v4 * (float)1000.0);
}

void VEH_UpdateSounds(gentity_s *ent)
{
    scr_vehicle_s *scr_vehicle; // r31
    unsigned __int16 *sndIndices; // r28
    BOOL v4; // r27
    gentity_s *v5; // r3
    double idleSndLerp; // fp1
    unsigned __int16 v7; // r5
    unsigned __int16 v8; // r4
    gentity_s *v9; // r3
    VehicleTurretState turretState; // r10

    scr_vehicle = ent->scr_vehicle;
    sndIndices = (unsigned short*)s_vehicleInfos[scr_vehicle->infoIdx].sndIndices;
    v4 = ent->health <= 0;
    iassert(ent->r.inuse);
    ent->s.loopSound = 0;
    if (scr_vehicle->idleSndEnt.isDefined())
    {
        v5 = scr_vehicle->idleSndEnt.ent();
        if (v4 || !scr_vehicle->playEngineSound)
        {
            v7 = 0;
            idleSndLerp = 0.0;
            v8 = 0;
        }
        else
        {
            idleSndLerp = scr_vehicle->idleSndLerp;
            v7 = sndIndices[1];
            v8 = *sndIndices;
        }
        G_SetSoundBlend(v5, v8, v7, idleSndLerp);
    }
    if (!scr_vehicle->engineSndEnt.isDefined())
    {
    LABEL_14:
        if (v4)
            return;
        goto LABEL_15;
    }
    v9 = scr_vehicle->engineSndEnt.ent();
    if (v4 || !scr_vehicle->playEngineSound)
    {
        G_SetSoundBlend(v9, 0, 0, 0.0);
        goto LABEL_14;
    }
    G_SetSoundBlend(v9, sndIndices[2], sndIndices[3], scr_vehicle->engineSndLerp);
LABEL_15:
    turretState = scr_vehicle->turret.turretState;
    if (turretState == VEH_TURRET_MOVING && sndIndices[4])
    {
        ent->s.loopSound = sndIndices[4];
    }
    else if (turretState == VEH_TURRET_STOPPING)
    {
        if (sndIndices[5])
            G_PlaySoundAlias(ent, sndIndices[5]);
    }
}

void G_FreeVehicle(gentity_s *ent)
{
    scr_vehicle_s *scr_vehicle; // r11

    iassert(ent->scr_vehicle->entNum != ENTITYNUM_NONE);
    ent->health = 0;
    VEH_UpdateSounds(ent);

    if (ent->scr_vehicle->idleSndEnt.isDefined())
    {
        G_FreeEntity(ent->scr_vehicle->idleSndEnt.ent());
    }
    if (ent->scr_vehicle->engineSndEnt.isDefined())
    {
        G_FreeEntity(ent->scr_vehicle->engineSndEnt.ent());
    }
    Scr_SetString(&ent->scr_vehicle->lookAtText0, 0);
    Scr_SetString(&ent->scr_vehicle->lookAtText1, 0);
    ent->scr_vehicle->lookAtEnt.setEnt(NULL);
    ent->scr_vehicle->idleSndEnt.setEnt(NULL);
    ent->scr_vehicle->engineSndEnt.setEnt(NULL);
    scr_vehicle = ent->scr_vehicle;
    ent->nextthink = 0;
    ent->takedamage = 0;
    ent->active = 0;
    ent->handler = 11;
    ent->s.lerp.eFlags = 0;
    ent->s.lerp.pos.trType = TR_STATIONARY;
    ent->s.lerp.apos.trType = TR_STATIONARY;
    VP_ClearNode(scr_vehicle->pathPos.switchNode);
    VP_ClearNode(&ent->scr_vehicle->pathPos.switchNode[1]);
    ent->scr_vehicle->entNum = 2175;
    ent->scr_vehicle = 0;
}

void CMD_VEH_JoltBody(scr_entref_t entref)
{
    gentity_s *Vehicle; // r30
    int NumParam; // r31
    double volume; // fp27
    double v4; // fp1
    double v5; // fp30
    double v6; // fp31
    double v7; // fp13
    double v10; // fp11
    double v11; // fp12
    float v12; // [sp+50h] [-60h] BYREF
    float v13; // [sp+54h] [-5Ch]
    float v14; // [sp+58h] [-58h]
    float v15[14]; // [sp+60h] [-50h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    NumParam = Scr_GetNumParam();
    Scr_GetVector(0, &v12);
    volume = Scr_GetFloat(1u);
    if (NumParam <= 2)
    {
        v5 = 0.0;
        v6 = 0.0;
    }
    else
    {
        v4 = Scr_GetFloat(2);
        v5 = v4;
        if (v4 < 0.0 || v4 > 1.0)
            Scr_ParamError(2u, "Speed fraction must be between [0,1]");
        v6 = (float)(Scr_GetFloat(3u) * (float)17.6);
        if (v6 < 0.0)
            Scr_ParamError(3u, "Deceleration can't be negative");
    }
    v7 = (float)(Vehicle->r.currentOrigin[2] - v14);
    float _FP10 = -sqrtf((float)((float)((float)(Vehicle->r.currentOrigin[0] - v12)
        * (float)(Vehicle->r.currentOrigin[0] - v12))
        + (float)((float)((float)(Vehicle->r.currentOrigin[2] - v14)
            * (float)(Vehicle->r.currentOrigin[2] - v14))
            + (float)((float)(Vehicle->r.currentOrigin[1] - v13)
                * (float)(Vehicle->r.currentOrigin[1] - v13)))));
    float _FP11;
    //__asm { fsel      f11, f10, f29, f11 }
    if (_FP10 >= 0.0f)
    {
        _FP11 = 1.0f;
    }
    else
    {
        _FP11 = _FP10;
    }
    v10 = (float)((float)1.0 / (float)_FP11);
    v11 = (float)((float)v10 * (float)(Vehicle->r.currentOrigin[0] - v12));
    v15[1] = (float)(Vehicle->r.currentOrigin[1] - v13) * (float)v10;
    v15[0] = v11;
    v15[2] = (float)v7 * (float)v10;
    VEH_JoltBody(Vehicle, v15, volume, v5, v6);
}

void CMD_VEH_FreeVehicle(scr_entref_t entref)
{
    gentity_s *Vehicle; // r31
    const char *v2; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    if ((Vehicle->scr_vehicle->flags & 1) != 0)
    {
        v2 = va("Can't free vehicle that a player is using");
        Scr_Error(v2);
    }
    G_FreeVehicle(Vehicle);
    Vehicle->s.eType = ET_VEHICLE_CORPSE;
    Scr_SetString(&Vehicle->classname, scr_const.script_vehicle_corpse);
    Scr_Notify(Vehicle, scr_const.death, 0);
}

void CMD_VEH_GetWheelSurface(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    scr_vehicle_s *scr_vehicle; // r28
    vehicle_info_t *v3; // r30
    unsigned int ConstString; // r31
    int v5; // r29
    const char *v6; // r3
    int v7; // r3
    const char *v8; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    v3 = &s_vehicleInfos[scr_vehicle->infoIdx];
    ConstString = Scr_GetConstString(0);
    v5 = 0;
    if (v3->type != 1 && v3->type)
    {
        v6 = va("Vehicle type [%s] has no wheels\n", v3->name);
        Scr_Error(v6);
    }
    if (ConstString == scr_const.front_left)
    {
        v5 = 0;
    }
    else if (ConstString == scr_const.front_right)
    {
        v5 = 1;
    }
    else if (ConstString == scr_const.back_left)
    {
        v5 = 2;
    }
    else if (ConstString == scr_const.back_right)
    {
        v5 = 3;
    }
    else if (ConstString == scr_const.middle_left)
    {
        v5 = 4;
    }
    else if (ConstString == scr_const.middle_right)
    {
        v5 = 5;
    }
    else
    {
        Scr_ParamError(
            0,
            "Valid wheel names are: [front_left, front_right, back_left, back_right, middle_left, middle_right]\n");
    }
    if (!v3->type && v5 > 3)
        Scr_ParamError(0, "Vehicle has no middle wheels\n");
    v7 = scr_vehicle->phys.wheelSurfType[v5];
    if (v7)
    {
        v8 = Com_SurfaceTypeToName(v7);
        Scr_AddString(v8);
    }
    else
    {
        Scr_AddString("none");
    }
}

void CMD_VEH_GetVehicleOwner(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    EntHandle *p_ownerNum; // r31
    gentity_s *v3; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    p_ownerNum = &Vehicle->r.ownerNum;
    if (Vehicle->r.ownerNum.isDefined())
    {
        v3 = p_ownerNum->ent();
        Scr_AddEntity(v3);
    }
}

void CMD_VEH_StartEngineSound(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    Vehicle->scr_vehicle->playEngineSound = 1;
}

void CMD_VEH_StopEngineSound(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    Vehicle->scr_vehicle->playEngineSound = 0;
}

void CMD_VEH_SetEngineVolume(scr_entref_t entref)
{
    gentity_s *Vehicle; // r11
    scr_vehicle_s *scr_vehicle; // r30
    float volume; // fp1
    float _FP12; // fp12
    float _FP13; // fp13
    float _FP31; // fp31
    gentity_s *v8; // r3
    gentity_s *v9; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    volume = Scr_GetFloat(0);

    _FP12 = -volume;
    _FP13 = (float)((float)volume - (float)1.0);

    if (_FP13 >= 0.0f)
    {
        _FP13 = 1.0f;
    }
    else
    {
        _FP13 = volume;
    }
    //__asm { fsel      f13, f13, f0, f1 }

    if (_FP12 >= 0.0f)
    {
        _FP31 = 1.0f;
    }
    else
    {
        _FP31 = _FP13;
    }

    //__asm { fsel      f31, f12, f0, f13 }

    if (scr_vehicle->idleSndEnt.isDefined())
    {
        v8 = scr_vehicle->idleSndEnt.ent();
        G_SetSoundBlendVolumeScale(v8, _FP31);
    }
    if (scr_vehicle->engineSndEnt.isDefined())
    {
        v9 = scr_vehicle->engineSndEnt.ent();
        G_SetSoundBlendVolumeScale(v9, _FP31);
    }
}

void CMD_VEH_GetEngineVolume(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    scr_vehicle_s *scr_vehicle; // r31
    double SoundBlendVolumeScale; // fp31
    EntHandle *v4; // r3
    EntHandle *v5; // r31
    gentity_s *v6; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    SoundBlendVolumeScale = 0.0;
    if (scr_vehicle->engineSndEnt.isDefined())
    {
        v4 = &scr_vehicle->engineSndEnt;
    LABEL_8:
        v6 = v4->ent();
        SoundBlendVolumeScale = G_GetSoundBlendVolumeScale(v6);
        goto LABEL_9;
    }
    v5 = &scr_vehicle->idleSndEnt;
    if (v5->isDefined())
    {
        v4 = v5;
        goto LABEL_8;
    }
LABEL_9:
    Scr_AddFloat(SoundBlendVolumeScale);
}

void CMD_VEH_MakeVehicleUsable(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    int v2; // r10

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    v2 = Vehicle->r.contents | 0x200000;
    Vehicle->spawnflags |= 1u;
    Vehicle->r.contents = v2;
}

void CMD_VEH_MakeVehicleUnusable(scr_entref_t entref)
{
    gentity_s *Vehicle; // r31
    unsigned int v3; // r10

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    if (Vehicle->r.ownerNum.isDefined())
    {
        Scr_Error(va("Vehicle is already in use"));
    }
    v3 = Vehicle->r.contents & 0xFFDFFFFF;
    Vehicle->spawnflags &= ~1u;
    Vehicle->r.contents = v3;
}

void CMD_VEH_AddVehicleToCompass(scr_entref_t entref)
{
    gentity_s *Vehicle; // r30
    const char *String; // r31
    scr_vehicle_s *scr_vehicle; // r11
    const char *v4; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    if (!Scr_GetNumParam())
    {
        Com_PrintWarning(24, "Script AddVehicleToCompass(); Was not passed a compassIconType, defaulting to \"tank\".");
        Vehicle->s.un1.scale = 1;
        return;
    }
    String = Scr_GetString(0);
    if (!I_stricmp(String, "tank"))
        goto LABEL_15;
    if (!I_stricmp(String, "helicopter"))
    {
        Vehicle->s.un1.scale = 2;
        Vehicle->scr_vehicle->drawOnCompass = 1;
        return;
    }
    if (!I_stricmp(String, "plane"))
    {
        Vehicle->s.un1.scale = 3;
        Vehicle->scr_vehicle->drawOnCompass = 1;
        return;
    }
    if (!I_stricmp(String, "automobile"))
    {
        Vehicle->s.un1.scale = 4;
        Vehicle->scr_vehicle->drawOnCompass = 1;
        return;
    }
    if (!*String)
    {
        Com_PrintWarning(24, "Script AddVehicleToCompass(); Was not passed a compassIconType, defaulting to \"tank\".");
    LABEL_15:
        scr_vehicle = Vehicle->scr_vehicle;
        Vehicle->s.un1.scale = 1;
        scr_vehicle->drawOnCompass = 1;
        return;
    }
    v4 = va("Unrecognized vehicle type given, \"%s\".", String);
    Scr_Error(v4);
    Vehicle->scr_vehicle->drawOnCompass = 1;
}

void CMD_VEH_RemoveVehicleFromCompass(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    Vehicle->scr_vehicle->drawOnCompass = 0;
    Vehicle->s.un1.scale = 0;
}

void CMD_VEH_SetVehicleLookatText(scr_entref_t entref)
{
    gentity_s *Vehicle; // r11
    scr_vehicle_s *scr_vehicle; // r31
    unsigned int ConstString; // r3
    unsigned int ConstIString; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    ConstString = Scr_GetConstString(0);
    Scr_SetString(&scr_vehicle->lookAtText0, ConstString);
    if (Scr_GetNumParam() > 1)
    {
        ConstIString = Scr_GetConstIString(1u);
        Scr_SetString(&scr_vehicle->lookAtText1, ConstIString);
    }
}

vehicle_info_t *VEH_GetVehicleInfo(__int16 index)
{
    int v1; // r31

    v1 = index;
    if (index < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 530, 0, "%s", "index >= 0");
    if (v1 >= s_numVehicleInfos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 531, 0, "%s", "index < s_numVehicleInfos");
    return &s_vehicleInfos[v1];
}

void HELI_CancelAIMove(gentity_s *ent)
{
    scr_vehicle_s *scr_vehicle; // r11
    float *origin; // r31
    double value; // fp0
    double v5; // fp13
    double v6; // fp11
    float v7[4]; // [sp+50h] [-80h] BYREF
    float v8[4]; // [sp+60h] [-70h] BYREF
    float v9[3]; // [sp+70h] [-60h] BYREF
    float v10[3]; // [sp+7Ch] [-54h] BYREF
    float v11[8]; // [sp+88h] [-48h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_helicopter.cpp", 341, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_helicopter.cpp", 342, 0, "%s", "ent->scr_vehicle");
    scr_vehicle = ent->scr_vehicle;
    origin = scr_vehicle->phys.origin;
    VEH_GetVehicleInfo(scr_vehicle->infoIdx);
    v7[0] = 0.0;
    v7[1] = origin[7];
    v7[2] = 0.0;
    AngleVectors(v7, v9, v10, v11);
    v11[3] = 0.0;
    v11[4] = 0.0;
    v11[5] = 0.0;
    value = vehHelicopterMaxPitch->current.value;
    if (value <= 0.0)
        value = 1.0;
    v5 = vehHelicopterMaxRoll->current.value;
    if (v5 <= 0.0)
        v5 = 1.0;
    v6 = origin[8];
    v8[0] = origin[6] / (float)value;
    v8[1] = (float)v6 / (float)v5;
    v8[2] = 0.0;
    MatrixTransformVector(v8, (const mat3x3&)v9, origin + 55);
    origin[58] = 0.0;
    origin[59] = 0.0;
    origin[60] = 0.0;
}

void VEH_CancelAIMove(gentity_s *ent)
{
    scr_vehicle_s *scr_vehicle; // r11
    int infoIdx; // r9

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 3146, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 3147, 0, "%s", "ent->scr_vehicle");
    scr_vehicle = ent->scr_vehicle;
    infoIdx = scr_vehicle->infoIdx;
    scr_vehicle->flags &= ~2u;
    if (s_vehicleInfos[infoIdx].type == 5)
        HELI_CancelAIMove(ent);
}

void CMD_VEH_ReturnPlayerControl(scr_entref_t entref)
{
    gentity_s *Vehicle; // r31

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    if (Vehicle->r.ownerNum.isDefined())
        VEH_CancelAIMove(Vehicle);
}

void CMD_VEH_IsTurretReady(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    scr_vehicle_s *scr_vehicle; // r31
    const char *v3; // r3

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    if ((scr_vehicle->flags & 1) == 0)
    {
        v3 = va("Must be called on a player controlled vehicle");
        Scr_Error(v3);
    }
    Scr_AddInt(scr_vehicle->turret.fireTime <= 0);
}

void CMD_VEH_ForceMaterialSpeed(scr_entref_t entref)
{
    gentity_s *Vehicle; // r3
    scr_vehicle_s *scr_vehicle; // r31
    unsigned int v3; // r11
    double Float; // fp1
    int flags; // r11

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        Vehicle = 0;
    }
    else
    {
        Vehicle = VEH_GetVehicle(entref.entnum);
    }
    scr_vehicle = Vehicle->scr_vehicle;
    if (!scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_vehicle.cpp", 6301, 0, "%s", "veh");
    if (Scr_GetInt(0))
    {
        Float = Scr_GetFloat(1u);
        flags = scr_vehicle->flags;
        scr_vehicle->forcedMaterialSpeed = Float;
        v3 = flags | 4;
    }
    else
    {
        v3 = scr_vehicle->flags & 0xFFFFFFFB;
    }
    scr_vehicle->flags = v3;
}

const BuiltinMethodDef s_methods[50] =
{
  { "attachpath", CMD_VEH_AttachPath, 0 },
  { "getattachpos", CMD_VEH_GetAttachPos, 0 },
  { "startpath", CMD_VEH_StartPath, 0 },
  { "setswitchnode", CMD_VEH_SetSwitchNode, 0 },
  { "setwaitnode", CMD_VEH_SetWaitNode, 0 },
  { "setwaitspeed", CMD_VEH_SetWaitSpeed, 0 },
  { "setspeed", CMD_VEH_SetSpeed, 0 },
  { "setspeedimmediate", CMD_VEH_SetSpeedImmediate, 0 },
  { "getspeed", CMD_VEH_GetSpeed, 0 },
  { "getspeedmph", CMD_VEH_GetSpeedMPH, 0 },
  { "getgoalspeedmph", CMD_VEH_GetGoalSpeedMPH, 0 },
  { "setacceleration", CMD_VEH_SetAcceleration, 0 },
  { "setdeceleration", CMD_VEH_SetDeceleration, 0 },
  { "resumespeed", CMD_VEH_ResumeSpeed, 0 },
  { "setyawspeed", CMD_VEH_SetYawSpeed, 0 },
  { "setmaxpitchroll", CMD_VEH_SetMaxPitchRoll, 0 },
  { "setturningability", CMD_VEH_SetTurningAbility, 0 },
  { "setairresistance", CMD_VEH_SetAirResitance, 0 },
  { "setjitterparams", CMD_VEH_SetJitterParams, 0 },
  { "sethoverparams", CMD_VEH_SetHoverParams, 0 },
  { "joltbody", CMD_VEH_JoltBody, 0 },
  { "freevehicle", CMD_VEH_FreeVehicle, 0 },
  { "getwheelsurface", CMD_VEH_GetWheelSurface, 0 },
  { "getvehicleowner", CMD_VEH_GetVehicleOwner, 0 },
  { "startenginesound", CMD_VEH_StartEngineSound, 0 },
  { "stopenginesound", CMD_VEH_StopEngineSound, 0 },
  { "setenginevolume", CMD_VEH_SetEngineVolume, 0 },
  { "getenginevolume", CMD_VEH_GetEngineVolume, 0 },
  { "makevehicleusable", CMD_VEH_MakeVehicleUsable, 0 },
  { "makevehicleunusable", CMD_VEH_MakeVehicleUnusable, 0 },
  { "addvehicletocompass", CMD_VEH_AddVehicleToCompass, 0 },
  { "removevehiclefromcompass", CMD_VEH_RemoveVehicleFromCompass, 0 },
  { "setvehiclelookattext", CMD_VEH_SetVehicleLookatText, 0 },
  { "setvehicleteam", CMD_VEH_SetVehicleTeam, 0 },
  { "setneargoalnotifydist", CMD_VEH_NearGoalNotifyDist, 0 },
  { "setvehgoalpos", CMD_VEH_SetGoalPos, 0 },
  { "setgoalyaw", CMD_VEH_SetGoalYaw, 0 },
  { "cleargoalyaw", CMD_VEH_ClearGoalYaw, 0 },
  { "settargetyaw", CMD_VEH_SetTargetYaw, 0 },
  { "cleartargetyaw", CMD_VEH_ClearTargetYaw, 0 },
  { "setlookatent", CMD_VEH_SetLookAtEnt, 0 },
  { "clearlookatent", CMD_VEH_ClearLookAtEnt, 0 },
  { "returnplayercontrol", CMD_VEH_ReturnPlayerControl, 0 },
  { "setturrettargetvec", CMD_VEH_SetTurretTargetVec, 0 },
  { "setturrettargetent", CMD_VEH_SetTurretTargetEnt, 0 },
  { "clearturrettarget", CMD_VEH_ClearTurretTargetEnt, 0 },
  { "setvehweapon", CMD_VEH_SetWeapon, 0 },
  { "fireweapon", CMD_VEH_FireWeapon, 0 },
  { "isturretready", CMD_VEH_IsTurretReady, 0 },
  { "vehforcematerialspeed", CMD_VEH_ForceMaterialSpeed, 0 }
};


void(* ScriptVehicle_GetMethod(const char **pName))(scr_entref_t)
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinMethodDef *i; // r7
    const char *actionString; // r10
    const char *v5; // r11
    int v6; // r8

    v1 = 0;
    v2 = 0;
    for (i = s_methods; ; ++i)
    {
        actionString = i->actionString;
        v5 = *pName;
        do
        {
            v6 = (unsigned __int8)*v5 - *(unsigned __int8 *)actionString;
            if (!*v5)
                break;
            ++v5;
            ++actionString;
        } while (!v6);
        if (!v6)
            break;
        v2 += 12;
        ++v1;
        if (v2 >= 0x258)
            return 0;
    }
    *pName = s_methods[v1].actionString;
    return s_methods[v1].actionFunc;
}

#endif // KISAK_SP