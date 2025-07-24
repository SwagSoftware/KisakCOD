#include "game_public.h"
#include <script/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include <server/sv_world.h>
#include <script/scr_vm.h>
#include "bullet.h"

struct AttractorRepulsor_t // sizeof=0x1C
{                                       // ...
    bool inUse;                         // ...
    bool isAttractor;                   // ...
    // padding byte
    // padding byte
    int32_t entnum;                         // ...
    float origin[3];                    // ...
    float strength;
    float maxDist;
};

struct $BC9161899B8BF9011D942B4F1507C18F // sizeof=0x380
{                                       // ...
    AttractorRepulsor_t attractors[32]; // ...
};

$BC9161899B8BF9011D942B4F1507C18F attrGlob;

const dvar_t *missileJavSpeedLimitDescend;
const dvar_t *missileJavClimbHeightTop;
const dvar_t *missileJavTurnRateDirect;
const dvar_t *missileJavClimbCeilingTop;
const dvar_t *missileJavTurnDecel;
const dvar_t *missileJavAccelDescend;
const dvar_t *missileHellfireMaxSlope;
const dvar_t *missileJavTurnRateTop;
const dvar_t *missileJavAccelClimb;
const dvar_t *missileDebugAttractors;
const dvar_t *missileDebugText;
const dvar_t *missileJavClimbToOwner;
const dvar_t *missileWaterMaxDepth;
const dvar_t *missileJavClimbAngleDirect;
const dvar_t *missileJavClimbHeightDirect;
const dvar_t *missileJavClimbCeilingDirect;
const dvar_t *missileJavClimbAngleTop;
const dvar_t *missileJavSpeedLimitClimb;
const dvar_t *missileHellfireUpAccel;
const dvar_t *missileDebugDraw;


void __cdecl G_RegisterMissileDvars()
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

    min.value.max = FLT_MAX;
    min.value.min = 0.0;
    missileHellfireMaxSlope = Dvar_RegisterFloat(
        "missileHellfireMaxSlope",
        0.5,
        min,
        DVAR_CHEAT,
        "This limits how steeply the hellfire missile can turn upward when climbing");
    mina.value.max = FLT_MAX;
    mina.value.min = 0.1f;
    missileHellfireUpAccel = Dvar_RegisterFloat(
        "missileHellfireUpAccel",
        1000.0f,
        mina,
        DVAR_CHEAT,
        "The rate at which the hellfire missile curves upward");
    minb.value.max = FLT_MAX;
    minb.value.min = 0.0f;
    missileJavClimbHeightDirect = Dvar_RegisterFloat(
        "missileJavClimbHeightDirect",
        10000.0f,
        minb,
        DVAR_CHEAT,
        "In direct-fire mode, how far above the target the rocket will aim for when climbing.");
    minc.value.max = FLT_MAX;
    minc.value.min = 0.0f;
    missileJavClimbHeightTop = Dvar_RegisterFloat(
        "missileJavClimbHeightTop",
        15000.0f,
        minc,
        DVAR_CHEAT,
        "In top-fire mode, how far above the target the rocket will aim for when climbing.");
    mind.value.max = FLT_MAX;
    mind.value.min = 0.0f;
    missileJavClimbAngleDirect = Dvar_RegisterFloat(
        "missileJavClimbAngleDirect",
        85.0f,
        mind,
        DVAR_CHEAT,
        "In direct-fire mode, the minimum angle between the rocket and target until the rocket s"
        "tops climbing.  Smaller angles make for higher climbs.");
    mine.value.max = FLT_MAX;
    mine.value.min = 0.0f;
    missileJavClimbAngleTop = Dvar_RegisterFloat(
        "missileJavClimbAngleTop",
        50.0f,
        mine,
        DVAR_CHEAT,
        "In top-fire mode, the minimum angle between the rocket and target until the rocket stops c"
        "limbing.  Smaller angles make for higher climbs.");
    minf.value.max = FLT_MAX;
    minf.value.min = 0.0f;
    missileJavClimbCeilingDirect = Dvar_RegisterFloat(
        "missileJavClimbCeilingDirect",
        0.0f,
        minf,
        DVAR_CHEAT,
        "In direct-fire mode, how high the missile needs to reach before it descends.");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0f;
    missileJavClimbCeilingTop = Dvar_RegisterFloat(
        "missileJavClimbCeilingTop",
        3000.0f,
        ming,
        DVAR_CHEAT,
        "In top-fire mode, how high the missile needs to reach before it descends.");
    minh.value.max = FLT_MAX;
    minh.value.min = 0.0f;
    missileJavTurnRateDirect = Dvar_RegisterFloat(
        "missileJavTurnRateDirect",
        60.0f,
        minh,
        DVAR_CHEAT,
        "In direct-fire mode, how sharp the rocket can turn, in angles/sec.");
    mini.value.max = FLT_MAX;
    mini.value.min = 0.0f;
    missileJavTurnRateTop = Dvar_RegisterFloat(
        "missileJavTurnRateTop",
        100.0f,
        mini,
        DVAR_CHEAT,
        "In top-fire mode, how sharp the rocket can turn, in angles/sec.");
    minj.value.max = FLT_MAX;
    minj.value.min = 0.0f;
    missileJavAccelClimb = Dvar_RegisterFloat(
        "missileJavAccelClimb",
        300.0f,
        minj,
        DVAR_CHEAT,
        "Rocket acceleration when climbing.");
    mink.value.max = FLT_MAX;
    mink.value.min = 0.0f;
    missileJavAccelDescend = Dvar_RegisterFloat(
        "missileJavAccelDescend",
        3000.0f,
        mink,
        DVAR_CHEAT,
        "Rocket acceleration when descending towards target.");
    minl.value.max = FLT_MAX;
    minl.value.min = 0.0f;
    missileJavSpeedLimitClimb = Dvar_RegisterFloat(
        "missileJavSpeedLimitClimb",
        1000.0f,
        minl,
        DVAR_CHEAT,
        "Rocket's speed limit when climbing.");
    minm.value.max = FLT_MAX;
    minm.value.min = 0.0f;
    missileJavSpeedLimitDescend = Dvar_RegisterFloat(
        "missileJavSpeedLimitDescend",
        6000.0f,
        minm,
        DVAR_CHEAT,
        "Rocket's speed limit when descending towards target.");
    minn.value.max = 1.0f;
    minn.value.min = 0.0f;
    missileJavTurnDecel = Dvar_RegisterFloat("missileJavTurnDecel", 0.050000001f, minn, DVAR_CHEAT, "");
    mino.value.max = FLT_MAX;
    mino.value.min = 0.0f;
    missileJavClimbToOwner = Dvar_RegisterFloat("missileJavClimbToOwner", 700.0f, mino, DVAR_CHEAT, "");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.0f;
    missileWaterMaxDepth = Dvar_RegisterFloat(
        "missileWaterMaxDepth",
        60.0f,
        minp,
        DVAR_CHEAT,
        "If a missile explodes deeper under water than this, they explosion effect/sound will not play.");
}

void __cdecl G_RegisterMissileDebugDvars()
{
    missileDebugDraw = Dvar_RegisterBool("missileDebugDraw", false, DVAR_NOFLAG, "Draw guided missile trajectories.");
    missileDebugText = Dvar_RegisterBool("missileDebugText", false, DVAR_NOFLAG, "Print debug missile info to console.");
    missileDebugAttractors = Dvar_RegisterBool(
        "missileDebugAttractors",
        false,
        DVAR_NOFLAG,
        "Draw the attractors and repulsors.  Attractors are green, and repulsors are yellow.");
}

void __cdecl G_TimedObjectThink(gentity_s *ent)
{
    ent->s.lerp.eFlags &= ~0x80000u;
}

void __cdecl G_ExplodeMissile(gentity_s *ent)
{
    int32_t v1; // eax
    uint8_t v2; // al
    uint8_t v3; // al
    uint8_t v4; // al
    uint8_t v5; // al
    float fInnerDamage; // [esp+0h] [ebp-E0h]
    float fOuterDamage; // [esp+4h] [ebp-DCh]
    float radius; // [esp+8h] [ebp-D8h]
    float scale; // [esp+10h] [ebp-D0h]
    float scale_4; // [esp+14h] [ebp-CCh]
    gentity_s *v11; // [esp+20h] [ebp-C0h]
    gentity_s *attacker; // [esp+24h] [ebp-BCh]
    float v13; // [esp+28h] [ebp-B8h]
    float v14; // [esp+2Ch] [ebp-B4h]
    float v15; // [esp+30h] [ebp-B0h]
    float v16; // [esp+50h] [ebp-90h]
    float origin[3]; // [esp+60h] [ebp-80h] BYREF
    float waterNormal[3]; // [esp+6Ch] [ebp-74h] BYREF
    float waterSurfacePos[3]; // [esp+78h] [ebp-68h] BYREF
    const float *normal; // [esp+84h] [ebp-5Ch]
    float end[3]; // [esp+88h] [ebp-58h] BYREF
    bool inWater; // [esp+97h] [ebp-49h]
    trace_t trace; // [esp+98h] [ebp-48h] BYREF
    bool doEvent; // [esp+C7h] [ebp-19h]
    float forwardDir[3]; // [esp+C8h] [ebp-18h] BYREF
    gentity_s *eventEnt; // [esp+D4h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+D8h] [ebp-8h]
    int32_t splashMethodOfDeath; // [esp+DCh] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 736, 0, "%s", "ent");
    if (!ent->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 737, 0, "%s", "ent->s.weapon");
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 739, 0, "%s", "weapDef");
    if (weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE && ent->s.groundEntityNum == 1023)
    {
        if (level.time - ent->item[0].clipAmmoCount <= 60000)
        {
            ent->nextthink = 50;
        }
        else
        {
            Scr_Notify(ent, scr_const.death, 0);
            G_FreeEntity(ent);
        }
    }
    else
    {
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, origin);
        origin[0] = (float)(int)origin[0];
        origin[1] = (float)(int)origin[1];
        origin[2] = (float)(int)origin[2];
        G_SetOrigin(ent, origin);
        doEvent = 1;
        v1 = SV_PointContents(ent->r.currentOrigin, -1, 32);
        inWater = v1 != 0;
        if (v1)
        {
            end[0] = ent->r.currentOrigin[0];
            end[1] = ent->r.currentOrigin[1];
            end[2] = ent->r.currentOrigin[2];
            end[2] = end[2] + missileWaterMaxDepth->current.value;
            G_TraceCapsule(&trace, end, (float *)vec3_origin, (float *)vec3_origin, ent->r.currentOrigin, ent->s.number, 32);
            if (trace.startsolid || trace.fraction >= 1.0)
            {
                doEvent = 0;
            }
            else
            {
                waterNormal[0] = trace.normal[0];
                waterNormal[1] = trace.normal[1];
                waterNormal[2] = trace.normal[2];
                Vec3Lerp(end, ent->r.currentOrigin, trace.fraction, waterSurfacePos);
            }
        }
        ent->s.lerp.eFlags |= 0x20u;
        Scr_AddVector(ent->r.currentOrigin);
        Scr_Notify(ent, scr_const.explode, 1u);
        eventEnt = 0;
        if (doEvent)
        {
            eventEnt = G_Spawn();
            eventEnt->s.eType = 0;
            eventEnt->s.lerp.eFlags = 32;
            eventEnt->s.weapon = ent->s.weapon;
            eventEnt->s.weaponModel = ent->s.weaponModel;
            eventEnt->r.contents = ent->r.contents;
            G_BroadcastEntity(eventEnt);
            if (inWater)
            {
                G_SetOrigin(eventEnt, waterSurfacePos);
                eventEnt->s.surfType = 20;
                normal = waterNormal;
            }
            else
            {
                G_SetOrigin(eventEnt, ent->r.currentOrigin);
                if (weapDef->stickiness == WEAPSTICKINESS_ALL && ent->s.groundEntityNum != 1023)
                {
                    Vec3Mad(ent->r.currentOrigin, -16.0, &ent->mover.aSpeed, end);
                }
                else
                {
                    end[0] = ent->r.currentOrigin[0];
                    end[1] = ent->r.currentOrigin[1];
                    end[2] = ent->r.currentOrigin[2];
                    end[2] = end[2] - 16.0;
                }
                G_TraceCapsule(
                    &trace,
                    ent->r.currentOrigin,
                    (float *)vec3_origin,
                    (float *)vec3_origin,
                    end,
                    ent->s.number,
                    2065);
                if (weapDef->projExplosionEffectForceNormalUp)
                    normal = up;
                else
                    normal = trace.normal;
                eventEnt->s.surfType = (trace.surfaceFlags & 0x1F00000) >> 20;
            }
            if (weapDef->projExplosion && weapDef->projExplosion != WEAPPROJEXP_HEAVY)
            {
                if (weapDef->projExplosion == WEAPPROJEXP_ROCKET)
                {
                    v3 = DirToByte(normal);
                    G_AddEvent(eventEnt, 0x2Eu, v3);
                }
                else if (weapDef->projExplosion == WEAPPROJEXP_FLASHBANG)
                {
                    v4 = DirToByte(normal);
                    G_AddEvent(eventEnt, 0x30u, v4);
                }
                else
                {
                    v5 = DirToByte(normal);
                    G_AddEvent(eventEnt, 0x31u, v5);
                    eventEnt->s.lerp.u.missile.launchTime = level.time;
                }
            }
            else
            {
                v2 = DirToByte(normal);
                G_AddEvent(eventEnt, 0x2Du, v2);
            }
            if (weapDef->projExplosion == WEAPPROJEXP_SMOKE && weapDef->projExplosionEffect)
            {
                eventEnt->s.lerp.pos.trBase[0] = (float)(int)eventEnt->s.lerp.pos.trBase[0];
                eventEnt->s.lerp.pos.trBase[1] = (float)(int)eventEnt->s.lerp.pos.trBase[1];
                eventEnt->s.lerp.pos.trBase[2] = (float)(int)eventEnt->s.lerp.pos.trBase[2];
                G_SetOrigin(eventEnt, eventEnt->s.lerp.pos.trBase);
                eventEnt->s.lerp.eFlags |= 0x10000u;
                eventEnt->s.lerp.u.missile.launchTime = level.time;
                eventEnt->s.time2 = level.time + 61000;
                eventEnt->s.lerp.eFlags |= 0x80000u;
                eventEnt->handler = 8;
                eventEnt->nextthink = level.time + 1;
                Com_Printf(
                    15,
                    "Sending smoke grenade that starts at %i and is at ( %f, %f, %f )\n",
                    level.time,
                    eventEnt->s.lerp.pos.trBase[0],
                    eventEnt->s.lerp.pos.trBase[1],
                    eventEnt->s.lerp.pos.trBase[2]);
            }
            else
            {
                G_FreeEntityAfterEvent(eventEnt);
            }
        }
        if (weapDef->iExplosionInnerDamage)
        {
            v16 = weapDef->damageConeAngle * 0.01745329238474369;
            v15 = cos(v16);
            v14 = weapDef->damageConeAngle - 180.0;
            if (v14 < 0.0)
                v13 = v15;
            else
                v13 = -1.0;
            AngleVectors(ent->r.currentAngles, forwardDir, 0, 0);
            splashMethodOfDeath = GetSplashMethodOfDeath(ent);

            if (ent->parent.isDefined())
                attacker = ent->parent.ent();
            else
                attacker = 0;

            radius = (float)weapDef->iExplosionRadius;
            fOuterDamage = (float)weapDef->iExplosionOuterDamage;
            fInnerDamage = (float)weapDef->iExplosionInnerDamage;
            G_RadiusDamage(
                ent->r.currentOrigin,
                ent,
                attacker,
                fInnerDamage,
                fOuterDamage,
                radius,
                v13,
                forwardDir,
                ent,
                splashMethodOfDeath,
                0xFFFFFFFF);
        }
        if (weapDef->projExplosion == WEAPPROJEXP_FLASHBANG)
        {
            if (ent->parent.isDefined())
                v11 = ent->parent.ent();
            else
                v11 = 0;
            scale_4 = (float)weapDef->iExplosionRadiusMin;
            scale = (float)weapDef->iExplosionRadius;
            G_FlashbangBlast(ent->r.currentOrigin, scale, scale_4, v11, ent->missile.team);
        }
        if (doEvent)
        {
            if (!eventEnt)
                MyAssertHandler(".\\game\\g_missile.cpp", 908, 1, "%s", "eventEnt");
            SV_LinkEntity(eventEnt);
        }
        Scr_Notify(ent, scr_const.death, 0);
        G_FreeEntity(ent);
    }
}

int32_t __cdecl GetSplashMethodOfDeath(gentity_s *ent)
{
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 452, 0, "%s", "ent");
    if (!ent->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 453, 0, "%s", "ent->s.weapon");
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 456, 0, "%s", "weapDef");
    if (entityHandlers[ent->handler].splashMethodOfDeath == 4 && weapDef->projExplosion == WEAPPROJEXP_HEAVY)
        return 14;
    else
        return entityHandlers[ent->handler].splashMethodOfDeath;
}

void __cdecl G_MissileTrace(trace_t *results, float *start, float *end, int32_t passEntityNum, int32_t contentmask)
{
    float dir[3]; // [esp+0h] [ebp-Ch] BYREF

    G_LocationalTrace(results, start, end, passEntityNum, contentmask, bulletPriorityMap);
    if (results->startsolid)
    {
        results->fraction = 0.0;
        Vec3Sub(start, end, dir);
        Vec3NormalizeTo(dir, results->normal);
    }
}

void __cdecl TRACK_missile_attractors()
{
    track_static_alloc_internal(&attrGlob, 896, "attrGlob", 9);
}

void __cdecl Missile_InitAttractors()
{
    memset((uint8_t *)&attrGlob, 0, sizeof(attrGlob));
}

void __cdecl Missile_FreeAttractorRefs(gentity_s *ent)
{
    AttractorRepulsor_t *v1; // ecx
    uint32_t attractorIndex; // [esp+0h] [ebp-4h]

    for (attractorIndex = 0; attractorIndex < 0x20; ++attractorIndex)
    {
        if (attrGlob.attractors[attractorIndex].inUse && attrGlob.attractors[attractorIndex].entnum == ent->s.number)
        {
            v1 = &attrGlob.attractors[attractorIndex];
            *(uint32_t *)&v1->inUse = 0;
            v1->entnum = 0;
            v1->origin[0] = 0.0;
            v1->origin[1] = 0.0;
            v1->origin[2] = 0.0;
            v1->strength = 0.0;
            v1->maxDist = 0.0;
        }
    }
}

void __cdecl Scr_MissileCreateAttractorEnt()
{
    uint32_t attractorIndex; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 1;
    ent = Scr_GetEntity(0);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u);
    if (attrGlob.attractors[attractorIndex].maxDist <= 0.0)
        Scr_ParamError(2u, "maxDist must be greater than zero");
    attrGlob.attractors[attractorIndex].entnum = ent->s.number;
    ent->flags |= 0x800000u;
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex);
}

uint32_t __cdecl Missile_GetFreeAttractor()
{
    const char *v0; // eax
    uint32_t attractorIndex; // [esp+0h] [ebp-4h]

    for (attractorIndex = 0; attractorIndex < 0x20 && attrGlob.attractors[attractorIndex].inUse; ++attractorIndex)
        ;
    if (attractorIndex == 32)
    {
        v0 = va("Ran out of attractor/repulsors.  Max allowed: %i", 32);
        Scr_Error(v0);
    }
    return attractorIndex;
}

void __cdecl Scr_MissileCreateAttractorOrigin()
{
    uint32_t attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 1;
    attrGlob.attractors[attractorIndex].entnum = 1023;
    Scr_GetVector(0, attrGlob.attractors[attractorIndex].origin);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u);
    if (attrGlob.attractors[attractorIndex].maxDist <= 0.0)
        Scr_ParamError(2u, "maxDist must be greater than zero");
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex);
}

void __cdecl Scr_MissileCreateRepulsorEnt()
{
    uint32_t attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 0;
    attrGlob.attractors[attractorIndex].entnum = Scr_GetEntity(0)->s.number;
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u);
    if (attrGlob.attractors[attractorIndex].maxDist <= 0.0)
        Scr_ParamError(2u, "maxDist must be greater than zero");
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex);
}

void __cdecl Scr_MissileCreateRepulsorOrigin()
{
    uint32_t attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 0;
    attrGlob.attractors[attractorIndex].entnum = 1023;
    Scr_GetVector(0, attrGlob.attractors[attractorIndex].origin);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u);
    if (attrGlob.attractors[attractorIndex].maxDist <= 0.0)
        Scr_ParamError(2u, "maxDist must be greater than zero");
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex);
}

void __cdecl Scr_MissileDeleteAttractor()
{
    AttractorRepulsor_t *v0; // ecx
    uint32_t attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Scr_GetInt(0);
    if (attractorIndex >= 0x20)
        Scr_ParamError(0, "Invalid attractor or repulsor");
    v0 = &attrGlob.attractors[attractorIndex];
    *(uint32_t *)&v0->inUse = 0;
    v0->entnum = 0;
    v0->origin[0] = 0.0;
    v0->origin[1] = 0.0;
    v0->origin[2] = 0.0;
    v0->strength = 0.0;
    v0->maxDist = 0.0;
}

void __cdecl G_MakeMissilePickupItem(gentity_s *ent)
{
    const gitem_s *item; // [esp+8h] [ebp-8h]
    int32_t itemIndex; // [esp+Ch] [ebp-4h]

    ent->r.mins[0] = -1.0;
    ent->r.mins[1] = -1.0;
    ent->r.mins[2] = -1.0;

    ent->r.maxs[0] = 1.0;
    ent->r.maxs[1] = 1.0;
    ent->r.maxs[2] = 1.0;

    ent->r.contents |= 0x200000u;
    item = BG_FindItemForWeapon(ent->s.weapon, ent->s.weaponModel);
    iassert(item);

#ifdef KISAK_MP
    itemIndex = ((char *)item - (char *)bg_itemlist) >> 2;
    ent->s.index.brushmodel = (uint16_t)itemIndex;
    ent->s.clientNum = 64;
#elif KISAK_SP
    itemIndex = (int32_t)((char *)item - (char *)bg_itemlist);
    ent->s.index.item = (uint16_t) itemIndex;
#endif

    iassert(ent->s.index.item == itemIndex);
    iassert(item->giType == IT_WEAPON);
}

void __cdecl G_RunMissile(gentity_s *ent)
{
    const float *v1; // [esp+1Ch] [ebp-158h]
    float v7; // [esp+34h] [ebp-140h]
    int32_t passEntityNum; // [esp+38h] [ebp-13Ch]
    float v9; // [esp+48h] [ebp-12Ch]
    float diff[3]; // [esp+50h] [ebp-124h] BYREF
    float *v11; // [esp+5Ch] [ebp-118h]
    float *v12; // [esp+60h] [ebp-114h]
    float *v13; // [esp+64h] [ebp-110h]
    float *v14; // [esp+68h] [ebp-10Ch]
    float v15; // [esp+6Ch] [ebp-108h]
    float *v16; // [esp+7Ch] [ebp-F8h]
    float *trDelta; // [esp+80h] [ebp-F4h]
    float *trBase; // [esp+84h] [ebp-F0h]
    float *v19; // [esp+88h] [ebp-ECh]
    float *currentOrigin; // [esp+8Ch] [ebp-E8h]
    float circleDir3[3]; // [esp+90h] [ebp-E4h] BYREF
    float center; // [esp+9Ch] [ebp-D8h] BYREF
    float v23; // [esp+A0h] [ebp-D4h]
    float v24; // [esp+A4h] [ebp-D0h]
    float radius; // [esp+A8h] [ebp-CCh]
    float circleDir2[3]; // [esp+ACh] [ebp-C8h] BYREF
    float circleDir1[3]; // [esp+B8h] [ebp-BCh] BYREF
    const float *color; // [esp+C4h] [ebp-B0h]
    uint32_t attractorIndex; // [esp+C8h] [ebp-ACh]
    float originOffset[3]; // [esp+CCh] [ebp-A8h] BYREF
    float traceStart[3]; // [esp+D8h] [ebp-9Ch] BYREF
    gentity_s *groundEnt; // [esp+E4h] [ebp-90h]
    float vOldOrigin[3]; // [esp+E8h] [ebp-8Ch] BYREF
    float dir[3]; // [esp+F4h] [ebp-80h] BYREF
    float origin[3]; // [esp+100h] [ebp-74h] BYREF
    float endpos[3]; // [esp+10Ch] [ebp-68h] BYREF
    trace_t tr; // [esp+118h] [ebp-5Ch] BYREF
    trace_t trDown; // [esp+144h] [ebp-30h] BYREF
    WeaponDef *weapDef; // [esp+170h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 2080, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 2081, 0, "%s", "ent->s.eType == ET_MISSILE");
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 2088, 0, "%s", "weapDef");
    if (ent->s.groundEntityNum != 1023 && ent->s.groundEntityNum != 1022 && weapDef->stickiness == WEAPSTICKINESS_ALL)
    {
        groundEnt = &g_entities[ent->s.groundEntityNum];
        if (groundEnt->scr_vehicle)
        {
            ent->s.lerp.pos.trType = groundEnt->s.lerp.pos.trType;
            G_RunThink(ent);
            SV_LinkEntity(ent);
            return;
        }
    }
    if (ent->s.lerp.pos.trType == TR_STATIONARY && ent->s.groundEntityNum != 1022)
    {
        currentOrigin = ent->r.currentOrigin;
        origin[0] = ent->r.currentOrigin[0];
        origin[1] = ent->r.currentOrigin[1];
        origin[2] = ent->r.currentOrigin[2];
        Vec3Mad(origin, -1.635f, &ent->mover.aSpeed, origin);
        if (ent->r.ownerNum.isDefined())
        {
            passEntityNum = ent->r.ownerNum.entnum();
            G_MissileTrace(&tr, ent->r.currentOrigin, origin, passEntityNum, ent->clipmask);
        }
        else
        {
            G_MissileTrace(&tr, ent->r.currentOrigin, origin, 1023, ent->clipmask);
        }
        if (tr.fraction == 1.0f)
        {
            ent->s.lerp.pos.trType = TR_GRAVITY;
            ent->s.lerp.pos.trTime = level.time;
            ent->s.lerp.pos.trDuration = 0;
            trBase = ent->s.lerp.pos.trBase;
            v19 = ent->r.currentOrigin;
            ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
            trBase[1] = v19[1];
            trBase[2] = v19[2];
            trDelta = ent->s.lerp.pos.trDelta;
            ent->s.lerp.pos.trDelta[0] = 0.0f;
            trDelta[1] = 0.0f;
            trDelta[2] = 0.0f;
        }
    }
    v16 = ent->r.currentOrigin;
    vOldOrigin[0] = ent->r.currentOrigin[0];
    vOldOrigin[1] = ent->r.currentOrigin[1];
    vOldOrigin[2] = ent->r.currentOrigin[2];
    MissileTrajectory(ent, origin);
    Vec3Sub(origin, ent->r.currentOrigin, dir);
    if (Vec3Normalize(dir) < EQUAL_EPSILON)
    {
        G_RunThink(ent);
        return;
    }
    v15 = ent->s.lerp.pos.trDelta[2];
    v7 = fabs(v15);
    if (v7 <= 30.0 || SV_PointContents(ent->r.currentOrigin, -1, 32))
    {
        if (ent->r.ownerNum.isDefined())
        {
            G_MissileTrace(&tr, ent->r.currentOrigin, origin, ent->r.ownerNum.entnum(), ent->clipmask);
        }
        else
        {
            G_MissileTrace(&tr, ent->r.currentOrigin, origin, 1023, ent->clipmask);
        }
    }
    else if (ent->r.ownerNum.isDefined())
    {
        G_MissileTrace(&tr, ent->r.currentOrigin, origin, ent->r.ownerNum.entnum(), ent->clipmask | 0x20);
    }
    else
    {
        G_MissileTrace(&tr, ent->r.currentOrigin, origin, 1023, ent->clipmask | 0x20);
    }
    if ((tr.surfaceFlags & 0x1F00000) == 0x1400000)
    {
        RunMissile_CreateWaterSplash(ent, &tr);
        if (ent->r.ownerNum.isDefined())
        {
            G_MissileTrace(&tr, ent->r.currentOrigin, origin, ent->r.ownerNum.entnum(), ent->clipmask);
        }
        else
        {
            G_MissileTrace(&tr, ent->r.currentOrigin, origin, 1023, ent->clipmask);
        }
    }
    if ((char *)(tr.surfaceFlags & 0x1F00000) == " (%i) exceeded\n")
        Missile_PenetrateGlass(&tr, ent, ent->r.currentOrigin, origin, weapDef->damage, 0);
    Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
    DrawMissileDebug(ent->r.currentOrigin, endpos);
    v14 = ent->r.currentOrigin;
    ent->r.currentOrigin[0] = endpos[0];
    v14[1] = endpos[1];
    v14[2] = endpos[2];
    if ((ent->s.lerp.eFlags & 0x1000000) != 0)
    {
        if (weapDef->stickiness == WEAPSTICKINESS_ALL
            || (weapDef->stickiness == WEAPSTICKINESS_GROUND || weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW)
            && tr.normal[2] > 0.699999988079071f)
        {
            if (tr.fraction < 1.0f)
            {
                Vec3Mad(ent->r.currentOrigin, 0.13500001f, tr.normal, traceStart);
                Vec3Mad(ent->r.currentOrigin, -1.5f, tr.normal, origin);
                if (ent->r.ownerNum.isDefined())
                {
                    G_MissileTrace(&trDown, traceStart, origin, ent->r.ownerNum.entnum(), ent->clipmask);
                }
                else
                {
                    G_MissileTrace(&trDown, traceStart, origin, 1023, ent->clipmask);
                }
                if (trDown.fraction != 1.0f)
                {
                    memcpy(&tr, &trDown, sizeof(tr));
                    Vec3Lerp(traceStart, origin, tr.fraction, endpos);
                    Vec3Sub(endpos, origin, originOffset);
                    Vec3Add(ent->s.lerp.pos.trBase, originOffset, ent->s.lerp.pos.trBase);
                    Vec3Add(endpos, originOffset, ent->r.currentOrigin);
                }
            }
        }
        else if (tr.fraction == 1.0f || tr.fraction < 1.0f && tr.normal[2] > 0.699999988079071f)
        {
            v13 = ent->r.currentOrigin;
            traceStart[0] = ent->r.currentOrigin[0];
            traceStart[1] = ent->r.currentOrigin[1];
            traceStart[2] = ent->r.currentOrigin[2];
            v12 = ent->r.currentOrigin;
            origin[0] = ent->r.currentOrigin[0];
            origin[1] = ent->r.currentOrigin[1];
            origin[2] = ent->r.currentOrigin[2];
            traceStart[2] = traceStart[2] + 0.135000005364418f;
            origin[2] = origin[2] - 1.5f;
            if (ent->r.ownerNum.isDefined())
            {
                G_MissileTrace(&trDown, traceStart, origin, ent->r.ownerNum.entnum(), ent->clipmask);
            }
            else
            {
                G_MissileTrace(&trDown, traceStart, origin, 1023, ent->clipmask);
            }
            if (trDown.fraction != 1.0)
            {
                memcpy(&tr, &trDown, sizeof(tr));
                Vec3Lerp(traceStart, origin, tr.fraction, endpos);
                ent->s.lerp.pos.trBase[2] = endpos[2] + 1.5 - ent->r.currentOrigin[2] + ent->s.lerp.pos.trBase[2];
                v11 = ent->r.currentOrigin;
                ent->r.currentOrigin[0] = endpos[0];
                v11[1] = endpos[1];
                v11[2] = endpos[2];
                ent->r.currentOrigin[2] = ent->r.currentOrigin[2] + 1.5;
            }
        }
    }
    SV_LinkEntity(ent);
    if (weapDef->iProjectileActivateDist > 0)
    {
        Vec3Sub(endpos, vOldOrigin, diff);
        v9 = Vec3Length(diff);
        ent->mover.speed = ent->mover.speed + v9;
    }
    if (entityHandlers[ent->handler].methodOfDeath == 3)
        G_GrenadeTouchTriggerDamage(
            ent,
            vOldOrigin,
            ent->r.currentOrigin,
            weapDef->iExplosionInnerDamage,
            entityHandlers[ent->handler].methodOfDeath);
    if (tr.fraction == 1.0)
    {
        if (Vec3Length(ent->s.lerp.pos.trDelta) != 0.0)
        {
            ent->s.groundEntityNum = 1023;
            if (weapDef->weapType == WEAPTYPE_PROJECTILE
                && (ent->flags & 0x20000) == 0
                && weapDef->guidedMissileType == MISSILE_GUIDANCE_NONE)
            {
                RunMissile_Destabilize(ent);
            }
        }
    LABEL_70:
        if (missileDebugAttractors->current.enabled)
        {
            for (attractorIndex = 0; attractorIndex < 0x20; ++attractorIndex)
            {
                circleDir1[0] = 1.0f;
                circleDir1[1] = 0.0f;
                circleDir1[2] = 0.0f;
                circleDir2[0] = 0.0f;
                circleDir2[1] = 1.0f;
                circleDir2[2] = 0.0f;
                circleDir3[0] = 0.0f;
                circleDir3[1] = 0.0f;
                circleDir3[2] = 1.0f;
                radius = 10.0f;
                if (attrGlob.attractors[attractorIndex].inUse)
                {
                    if (attrGlob.attractors[attractorIndex].entnum == 1023)
                    {
                        center = attrGlob.attractors[attractorIndex].origin[0];
                        v23 = attrGlob.attractors[attractorIndex].origin[1];
                        v24 = attrGlob.attractors[attractorIndex].origin[2];
                    }
                    else
                    {
                        if (attrGlob.attractors[attractorIndex].entnum >= 1024)
                            MyAssertHandler(
                                ".\\game\\g_missile.cpp",
                                2265,
                                0,
                                "%s",
                                "attrGlob.attractors[attractorIndex].entnum < MAX_GENTITIES");
                        ent = &g_entities[attrGlob.attractors[attractorIndex].entnum];
                        center = ent->r.currentOrigin[0];
                        v23 = ent->r.currentOrigin[1];
                        v24 = ent->r.currentOrigin[2];
                    }
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                        v1 = colorGreen;
                    else
                        v1 = colorOrange;
                    color = v1;
                    G_DebugCircleEx(&center, radius, circleDir1, v1, 0, 1);
                    G_DebugCircleEx(&center, radius, circleDir2, color, 0, 1);
                    G_DebugCircleEx(&center, radius, circleDir3, color, 0, 1);
                }
            }
        }
        G_RunThink(ent);
        return;
    }
    if ((tr.surfaceFlags & 4) != 0)
    {
        Scr_Notify(ent, scr_const.death, 0);
        G_FreeEntity(ent);
        return;
    }
    MissileImpact(ent, &tr, dir, endpos);
    if (ent->s.eType == 4)
        goto LABEL_70;
}

void __cdecl MissileImpact(gentity_s *ent, trace_t *trace, float *dir, float *endpos)
{
    uint8_t v4; // al
    bool v5; // eax
    int32_t v6; // eax
    gentity_s *v7; // eax
    uint8_t v8; // al
    uint8_t v9; // al
    uint8_t v10; // al
    uint8_t v11; // al
    uint8_t v12; // al
    uint8_t v13; // al
    float fInnerDamage; // [esp+0h] [ebp-F8h]
    float fOuterDamage; // [esp+4h] [ebp-F4h]
    float radius; // [esp+8h] [ebp-F0h]
    float radius_max; // [esp+10h] [ebp-E8h]
    gentity_s *radius_min; // [esp+14h] [ebp-E4h]
    float radius_mina; // [esp+14h] [ebp-E4h]
    int32_t fraction; // [esp+18h] [ebp-E0h]
    uint16_t fractiona; // [esp+18h] [ebp-E0h]
    gentity_s *v24; // [esp+20h] [ebp-D8h]
    float coneAngleCos; // [esp+24h] [ebp-D4h]
    float v26; // [esp+28h] [ebp-D0h]
    float v27; // [esp+2Ch] [ebp-CCh]
    uint8_t v28; // [esp+33h] [ebp-C5h]
    bool v29; // [esp+34h] [ebp-C4h]
    gentity_s *pActivator; // [esp+38h] [ebp-C0h]
    gentity_s *attacker; // [esp+40h] [ebp-B8h]
    hitLocation_t partGroup; // [esp+44h] [ebp-B4h]
    float v34; // [esp+4Ch] [ebp-ACh]
    float javNormal[3]; // [esp+58h] [ebp-A0h] BYREF
    float speed; // [esp+64h] [ebp-94h]
    float velocity[3]; // [esp+68h] [ebp-90h] BYREF
    int32_t damage; // [esp+74h] [ebp-84h]
    trace_t waterTrace; // [esp+78h] [ebp-80h] BYREF
    int32_t explodeOnImpact; // [esp+A4h] [ebp-54h]
    float waterNormal[3]; // [esp+A8h] [ebp-50h] BYREF
    float waterSurfacePos[3]; // [esp+B4h] [ebp-44h] BYREF
    const float *normal; // [esp+C0h] [ebp-38h]
    bool inWater; // [esp+C6h] [ebp-32h]
    bool waterExplodeAllowed; // [esp+C7h] [ebp-31h]
    gentity_s *other; // [esp+C8h] [ebp-30h]
    int32_t nomarks; // [esp+CCh] [ebp-2Ch]
    int32_t explosionType; // [esp+D0h] [ebp-28h]
    hitLocation_t hitLocation; // [esp+D4h] [ebp-24h]
    int32_t hitClient; // [esp+D8h] [ebp-20h]
    WeaponDef *weapDef; // [esp+DCh] [ebp-1Ch]
    int32_t methodOfDeath; // [esp+E0h] [ebp-18h]
    int32_t splashMethodOfDeath; // [esp+E4h] [ebp-14h]
    uint16_t hitEntId; // [esp+E8h] [ebp-10h]
    float traceStart[3]; // [esp+ECh] [ebp-Ch] BYREF

    hitClient = 0;
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 493, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 494, 0, "%s", "ent->s.eType == ET_MISSILE");
    if (!ent->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 495, 0, "%s", "ent->s.weapon");
    hitEntId = Trace_GetEntityHitId(trace);
    other = &g_entities[hitEntId];
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 501, 0, "%s", "weapDef");
    explodeOnImpact = weapDef->bProjImpactExplode;
    explosionType = weapDef->projExplosion;
    damage = weapDef->damage;
    ent->s.surfType = (trace->surfaceFlags & 0x1F00000) >> 20;
    if (GrenadeDud(ent, weapDef) || JavelinDud(ent, weapDef))
    {
        explosionType = 4;
        explodeOnImpact = 0;
        ent->mover.speed = -1.0e10f;
        methodOfDeath = 15;
    }
    else if (explodeOnImpact)
    {
        methodOfDeath = entityHandlers[ent->handler].methodOfDeath;
    }
    else
    {
        methodOfDeath = 15;
    }
    if (methodOfDeath == 15)
        partGroup = (hitLocation_t)trace->partGroup;
    else
        partGroup = HITLOC_NONE;
    hitLocation = partGroup;
    if (!other->takedamage
        && (ent->s.lerp.eFlags & 0x1000000) != 0
        && !explodeOnImpact
        && !CheckCrumpleMissile(ent, trace))
    {
        if (BounceMissile(ent, trace) && !trace->startsolid)
            G_AddEvent(ent, 0x2Cu, (trace->surfaceFlags & 0x1F00000) >> 20);
        if (weapDef->iProjectileActivateDist > 0 && ent->s.lerp.pos.trType == TR_STATIONARY)
        {
            v4 = DirToByte(trace->normal);
            G_AddEvent(ent, 0x33u, v4);
            G_FreeEntityAfterEvent(ent);
        }
        return;
    }
    if (other->takedamage)
    {
        if (damage)
        {
            if (ent->r.ownerNum.isDefined())
            {
                attacker = ent->r.ownerNum.ent();
                v5 = LogAccuracyHit(other, attacker);
            }
            else
            {
                v5 = LogAccuracyHit(other, &g_entities[1023]);
            }
            if (v5)
                hitClient = 1;
            BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, velocity);
            speed = Vec3Length(velocity);
            if (speed == 0.0f)
                velocity[2] = 1.0f;
            if (weapDef->weapType != WEAPTYPE_GRENADE || g_minGrenadeDamageSpeed->current.value < (double)speed)
            {
                if (ent->r.ownerNum.isDefined())
                {
                    G_Damage(
                        other,
                        ent,
                        ent->r.ownerNum.ent(),
                        velocity,
                        ent->r.currentOrigin,
                        damage,
                        0,
                        methodOfDeath,
                        ent->s.weapon,
                        hitLocation,
                        trace->modelIndex,
                        trace->partName,
                        0);
                }
                else
                {
                    G_Damage(
                        other,
                        ent,
                        0,
                        velocity,
                        ent->r.currentOrigin,
                        damage,
                        0,
                        methodOfDeath,
                        ent->s.weapon,
                        hitLocation,
                        trace->modelIndex,
                        trace->partName,
                        0);
                }
            }
        }
        if (!explodeOnImpact)
        {
            if (other->client && !trace->surfaceFlags)
                trace->surfaceFlags = 0x700000;
            if (!CheckCrumpleMissile(ent, trace))
            {
                if (BounceMissile(ent, trace) && !trace->startsolid)
                    G_AddEvent(ent, 0x2Cu, (trace->surfaceFlags & 0x1F00000) >> 20);
                return;
            }
        }
    }
    if (damage)
    {
        if (ent->r.ownerNum.isDefined())
        {
            pActivator = ent->r.ownerNum.ent();
            G_CheckHitTriggerDamage(pActivator, ent->r.currentOrigin, endpos, damage, methodOfDeath);
        }
        else
        {
            G_CheckHitTriggerDamage(&g_entities[1022], ent->r.currentOrigin, endpos, damage, methodOfDeath);
        }
    }
    v29 = hitClient || trace->partName;
    nomarks = v29;
    waterExplodeAllowed = 1;
    v6 = SV_PointContents(ent->r.currentOrigin, -1, 32);
    inWater = v6 != 0;
    if (v6)
    {
        traceStart[0] = *endpos;
        traceStart[1] = endpos[1];
        traceStart[2] = endpos[2];
        traceStart[2] = traceStart[2] + missileWaterMaxDepth->current.value;
        G_TraceCapsule(&waterTrace, traceStart, (float *)vec3_origin, (float *)vec3_origin, endpos, ent->s.number, 32);
        if (waterTrace.startsolid || waterTrace.fraction >= 1.0)
        {
            waterExplodeAllowed = 0;
        }
        else
        {
            waterNormal[0] = waterTrace.normal[0];
            waterNormal[1] = waterTrace.normal[1];
            waterNormal[2] = waterTrace.normal[2];
            Vec3Lerp(traceStart, endpos, waterTrace.fraction, waterSurfacePos);
        }
    }
    if (inWater && waterExplodeAllowed)
    {
        normal = waterNormal;
    }
    else if (ent->s.eType == 4
        && weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN
        && ent->missile.flightMode == MISSILEFLIGHTMODE_TOP
        && ent->missileTargetEnt.isDefined()
        && (v7 = ent->missileTargetEnt.ent(), v7 == other))
    {
        normal = up;
    }
    else if (weapDef->projExplosionEffectForceNormalUp)
    {
        normal = up;
    }
    else
    {
        normal = trace->normal;
    }
    if (inWater && !waterExplodeAllowed)
        goto LABEL_91;
    if (explosionType && explosionType != 6)
    {
        switch (explosionType)
        {
        case 1:
            v9 = DirToByte(normal);
            G_AddEvent(ent, (nomarks != 0) + 46, v9);
            goto LABEL_92;
        case 3:
            v10 = DirToByte(normal);
            G_AddEvent(ent, (nomarks != 0) + 49, v10);
            goto LABEL_92;
        case 2:
            v11 = DirToByte(normal);
            G_AddEvent(ent, 0x30u, v11);
            goto LABEL_92;
        }
        if (explosionType != 4)
            goto LABEL_92;
        if (JavelinProjectile(ent, weapDef))
        {
            Vec3NormalizeTo(ent->s.lerp.pos.trDelta, javNormal);
            javNormal[0] = -javNormal[0];
            javNormal[1] = -javNormal[1];
            javNormal[2] = -javNormal[2];
            v12 = DirToByte(javNormal);
            G_AddEvent(ent, 0x35u, v12);
            goto LABEL_92;
        }
    LABEL_91:
        v13 = DirToByte(normal);
        G_AddEvent(ent, 0x34u, v13);
        goto LABEL_92;
    }
    v8 = DirToByte(normal);
    G_AddEvent(ent, 0x2Du, v8);
LABEL_92:
    if (inWater)
        v28 = 20;
    else
        v28 = (trace->surfaceFlags & 0x1F00000) >> 20;
    ent->s.surfType = v28;
    Scr_Notify(ent, scr_const.death, 0);
    G_FreeEntityAfterEvent(ent);
    ent->s.lerp.eFlags ^= 2u;
    ent->s.lerp.eFlags |= 0x20u;
    if (inWater && waterExplodeAllowed)
        G_SetOrigin(ent, waterSurfacePos);
    else
        G_SetOrigin(ent, endpos);
    if (explodeOnImpact)
    {
        if (weapDef->iExplosionInnerDamage)
        {
            splashMethodOfDeath = GetSplashMethodOfDeath(ent);
            if (ent->parent.isDefined())
            {
                v34 = weapDef->damageConeAngle * 0.01745329238474369f;
                v27 = cos(v34);
                v26 = weapDef->damageConeAngle - 180.0f;
                if (v26 < 0.0f)
                    coneAngleCos = v27;
                else
                    coneAngleCos = -1.0f;
                fraction = splashMethodOfDeath;
                radius_min = other;
                radius = (float)weapDef->iExplosionRadius;
                fOuterDamage = (float)weapDef->iExplosionOuterDamage;
                fInnerDamage = (float)weapDef->iExplosionInnerDamage;
                G_RadiusDamage(
                    endpos,
                    ent,
                    ent->parent.ent(),
                    fInnerDamage,
                    fOuterDamage,
                    radius,
                    coneAngleCos,
                    dir,
                    radius_min,
                    fraction,
                    0xFFFFFFFF);
                Scr_AddInt(weapDef->iExplosionRadius);
                Scr_AddVector(endpos);
                Scr_AddString((char *)weapDef->szInternalName);
                fractiona = scr_const.projectile_impact;
                Scr_Notify(ent->parent.ent(), fractiona, 3u);
            }
        }
    }
    if (explosionType == 2)
    {
        if (ent->parent.isDefined())
            v24 = ent->parent.ent();
        else
            v24 = 0;
        radius_mina = (float)weapDef->iExplosionRadiusMin;
        radius_max = (float)weapDef->iExplosionRadius;
        G_FlashbangBlast(endpos, radius_max, radius_mina, v24, ent->missile.team);
    }
    SV_LinkEntity(ent);
}

bool __cdecl CheckCrumpleMissile(gentity_s *ent, trace_t *trace)
{
    double v3; // st7
    float scale; // [esp+Ch] [ebp-2Ch]
    float velocity[3]; // [esp+18h] [ebp-20h] BYREF
    int32_t hitTime; // [esp+24h] [ebp-14h]
    float MIN_CRUMPLE_SPEED; // [esp+28h] [ebp-10h]
    float cos45; // [esp+2Ch] [ebp-Ch]
    float speed; // [esp+30h] [ebp-8h]
    WeaponDef *weapDef; // [esp+34h] [ebp-4h]

    if (!ent->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 233, 0, "%s", "ent->s.weapon");
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 235, 0, "%s", "weapDef");
    if (weapDef->weapType != WEAPTYPE_PROJECTILE)
        return 0;
    if (trace->surfaceFlags == 0x700000)
        return 1;
    hitTime = level.previousTime + (int)((double)(level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, hitTime, velocity);
    speed = Vec3Length(velocity);
    MIN_CRUMPLE_SPEED = 500.0f;
    if ((float)500.0f > (double)speed)
        return 0;
    scale = -1.0f / speed;
    Vec3Scale(velocity, scale, velocity);
    cos45 = 0.70700002f;
    v3 = Vec3Dot(velocity, trace->normal);
    return cos45 < v3;
}

bool __cdecl BounceMissile(gentity_s *ent, trace_t *trace)
{
    bool v3; // [esp+Ch] [ebp-84h]
    float scale; // [esp+10h] [ebp-80h]
    float velocity[3]; // [esp+50h] [ebp-40h] BYREF
    float vAngles[3]; // [esp+5Ch] [ebp-34h] BYREF
    int32_t hitTime; // [esp+68h] [ebp-28h]
    int32_t contents; // [esp+6Ch] [ebp-24h]
    bool mayStop; // [esp+73h] [ebp-1Dh]
    int32_t surfType; // [esp+74h] [ebp-1Ch]
    float bounceFactor; // [esp+78h] [ebp-18h]
    WeaponDef *weapDef; // [esp+7Ch] [ebp-14h]
    float dot; // [esp+80h] [ebp-10h]
    float vDelta[3]; // [esp+84h] [ebp-Ch] BYREF

    if (!ent->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 308, 0, "%s", "ent->s.weapon");
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 310, 0, "%s", "weapDef");
    contents = SV_PointContents(ent->r.currentOrigin, -1, 32);
    surfType = (trace->surfaceFlags & 0x1F00000) >> 20;
    hitTime = level.previousTime + (int)((double)(level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, hitTime, velocity);
    dot = Vec3Dot(velocity, trace->normal);
    scale = dot * -2.0;
    Vec3Mad(velocity, scale, trace->normal, ent->s.lerp.pos.trDelta);
    if ((COERCE_UNSIGNED_INT(ent->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(ent->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(ent->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_missile.cpp",
            320,
            0,
            "%s",
            "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])");
    }
    v3 = weapDef->stickiness == WEAPSTICKINESS_NONE || Trace_GetEntityHitId(trace) >= 0x40u;
    mayStop = v3;
    if (g_entities[Trace_GetEntityHitId(trace)].s.eType == 4)
        mayStop = 0;
    if (mayStop && (weapDef->stickiness == WEAPSTICKINESS_ALL || trace->normal[2] > 0.699999988079071))
    {
        ent->s.groundEntityNum = Trace_GetEntityHitId(trace);
        g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
    }
    if ((ent->s.lerp.eFlags & 0x1000000) == 0)
        goto LABEL_40;
    bounceFactor = Vec3Length(velocity);
    if (bounceFactor > 0.0f && dot <= 0.0f)
    {
        dot = dot / -bounceFactor;
        bounceFactor = (weapDef->perpendicularBounce[surfType] - weapDef->parallelBounce[surfType]) * dot
            + weapDef->parallelBounce[surfType];
        Vec3Scale(ent->s.lerp.pos.trDelta, bounceFactor, ent->s.lerp.pos.trDelta);
        if ((COERCE_UNSIGNED_INT(ent->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(ent->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(ent->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\g_missile.cpp",
                347,
                0,
                "%s",
                "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])");
        }
    }
    if (mayStop
        && (weapDef->stickiness == WEAPSTICKINESS_ALL
            || trace->normal[2] > 0.699999988079071f
            && (weapDef->stickiness == WEAPSTICKINESS_GROUND
                || weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW
                || Vec3Length(ent->s.lerp.pos.trDelta) < 20.0f)))
    {
        G_SetOrigin(ent, ent->r.currentOrigin);
        if (weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW)
        {
            MissileLandAnglesFlatMaintainingDirection(ent, trace, vAngles);
        }
        else if (weapDef->stickiness)
        {
            MissileLandAnglesFlat(ent, trace, vAngles);
        }
        else
        {
            MissileLandAngles(ent, trace, vAngles, 1);
        }
        G_SetAngle(ent, vAngles);
        ent->item[1] = *(item_ent_t *)trace->normal;
        if (!weapDef->timedDetonation)
            ent->nextthink = 0;
        CheckGrenadeDanger(ent);
        return 1;
    }
    else
    {
    LABEL_40:
        Vec3Scale(trace->normal, 0.1f, vDelta);
        if (vDelta[2] > 0.0f)
            vDelta[2] = 0.0f;
        Vec3Add(ent->r.currentOrigin, vDelta, ent->r.currentOrigin);
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
        ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
        ent->s.lerp.pos.trTime = level.time;
        MissileLandAngles(ent, trace, vAngles, 0);
        ent->s.lerp.apos.trBase[0] = vAngles[0];
        ent->s.lerp.apos.trBase[1] = vAngles[1];
        ent->s.lerp.apos.trBase[2] = vAngles[2];
        ent->s.lerp.apos.trTime = level.time;
        if (contents)
        {
            return 0;
        }
        else
        {
            Vec3Sub(ent->s.lerp.pos.trDelta, velocity, velocity);
            dot = Vec3Length(velocity);
            return dot > 100.0f;
        }
    }
}

void __cdecl MissileLandAngles(gentity_s *ent, trace_t *trace, float *vAngles, int32_t bForceAlign)
{
    double v4; // st7
    double v5; // st7
    double v6; // st7
    float v7; // [esp+8h] [ebp-5Ch]
    float v8; // [esp+10h] [ebp-54h]
    float v9; // [esp+14h] [ebp-50h]
    float v10; // [esp+18h] [ebp-4Ch]
    float v11; // [esp+1Ch] [ebp-48h]
    float v12; // [esp+20h] [ebp-44h]
    float v13; // [esp+24h] [ebp-40h]
    float v14; // [esp+34h] [ebp-30h]
    int32_t hitTime; // [esp+40h] [ebp-24h]
    float fSurfacePitch; // [esp+44h] [ebp-20h]
    float fAdjustPitchDiff; // [esp+50h] [ebp-14h]
    float fAngleDelta; // [esp+60h] [ebp-4h]

    fAdjustPitchDiff = 80.0f;
    hitTime = level.previousTime + (int)((float)(level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectory(&ent->s.lerp.apos, hitTime, vAngles);
    if (trace->normal[2] <= 0.1000000014901161f)
    {
        if (!bForceAlign)
        {
            v7 = ((G_rand() & 0x7F) - 63) + ent->s.lerp.apos.trDelta[0];
            ent->s.lerp.apos.trDelta[0] = AngleNormalize360(v7);
        }
    }
    else
    {
        fSurfacePitch = PitchForYawOnNormal(vAngles[1], trace->normal);
        fAngleDelta = AngleDelta(fSurfacePitch, *vAngles);
        v13 = fabs(fAngleDelta);
        if (!bForceAlign)
        {
            ent->s.lerp.apos.trBase[0] = *vAngles;
            ent->s.lerp.apos.trBase[1] = vAngles[1];
            ent->s.lerp.apos.trBase[2] = vAngles[2];
            ent->s.lerp.apos.trTime = hitTime;
            v4 = G_random();
            if (fAdjustPitchDiff <= v13)
                ent->s.lerp.apos.trDelta[0] = (v4 * 0.300000011920929f + 0.8500000238418579f) * ent->s.lerp.apos.trDelta[0];
            else
                ent->s.lerp.apos.trDelta[0] = (v4 * 0.300000011920929f + 0.8500000238418579f) * ent->s.lerp.apos.trDelta[0] * -1.0;
        }
        v14 = *vAngles * 0.002777777845039964f;
        v12 = v14 + 0.5;
        v11 = floor(v12);
        *vAngles = (v14 - v11) * 360.0f;
        if (bForceAlign || 45.0f > v13)
        {
            v10 = fabs(*vAngles);
            if (v10 <= 90.0f)
            {
                v5 = AngleNormalize360(fSurfacePitch);
            }
            else
            {
                v9 = fSurfacePitch + 180.0f;
                v5 = AngleNormalize360(v9);
            }
            *vAngles = v5;
        }
        else
        {
            if (fAdjustPitchDiff <= v13)
            {
                v6 = AngleNormalize360(*vAngles);
            }
            else
            {
                v8 = fAngleDelta * 0.25f + *vAngles;
                v6 = AngleNormalize360(v8);
            }
            *vAngles = v6;
        }
    }
}

void __cdecl MissileLandAnglesFlat(gentity_s *ent, trace_t *trace, float *angles)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float normalUpComponent; // [esp+Ch] [ebp-20h]
    float right[3]; // [esp+10h] [ebp-1Ch] BYREF
    float normalRightComponent; // [esp+1Ch] [ebp-10h]
    float up[3]; // [esp+20h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 181, 0, "%s", "ent");
    if (!trace)
        MyAssertHandler(".\\game\\g_missile.cpp", 182, 0, "%s", "trace");
    BG_EvaluateTrajectory(
        &ent->s.lerp.apos,
        level.previousTime + (int)((double)(level.time - level.previousTime) * trace->fraction),
        angles);
    NearestPitchAndYawOnPlane(angles, trace->normal, angles);
    if (angles[2] != 0.0f)
        MyAssertHandler(".\\game\\g_missile.cpp", 191, 0, "%s", "angles[ROLL] == 0.f");
    AngleVectors(angles, 0, right, up);
    normalRightComponent = Vec3Dot(right, trace->normal);
    normalUpComponent = Vec3Dot(up, trace->normal);
    v3 = atan2(normalRightComponent, normalUpComponent);
    angles[2] = v3 * 180.0f / 3.141592741012573f;
}

void __cdecl MissileLandAnglesFlatMaintainingDirection(gentity_s *ent, trace_t *trace, float *angles)
{
    float v3; // [esp+8h] [ebp-2Ch]
    float normalUpComponent; // [esp+14h] [ebp-20h]
    float right[3]; // [esp+18h] [ebp-1Ch] BYREF
    float normalRightComponent; // [esp+24h] [ebp-10h]
    float up[3]; // [esp+28h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 207, 0, "%s", "ent");
    if (!trace)
        MyAssertHandler(".\\game\\g_missile.cpp", 208, 0, "%s", "trace");
    BG_EvaluateTrajectory(
        &ent->s.lerp.apos,
        level.previousTime + (int)((double)(level.time - level.previousTime) * trace->fraction),
        angles);
    *angles = PitchForYawOnNormal(angles[1], trace->normal);
    angles[2] = 0.0f;
    AngleVectors(angles, 0, right, up);
    normalRightComponent = Vec3Dot(right, trace->normal);
    normalUpComponent = Vec3Dot(up, trace->normal);
    v3 = atan2(normalRightComponent, normalUpComponent);
    angles[2] = v3 * 180.0f / 3.141592741012573f;
}

void __cdecl CheckGrenadeDanger(gentity_s *grenadeEnt)
{
    gentity_s *v1; // eax
    float iExplosionRadius; // [esp+8h] [ebp-14h]
    float damageRadiusSquared; // [esp+Ch] [ebp-10h]
    gentity_s *ent; // [esp+10h] [ebp-Ch]
    int32_t i; // [esp+14h] [ebp-8h]
    WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    if (!grenadeEnt)
        MyAssertHandler(".\\game\\g_missile.cpp", 267, 0, "%s", "grenadeEnt");
    if (!grenadeEnt->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 268, 0, "%s", "grenadeEnt->s.weapon != WP_NONE");
    weapDef = BG_GetWeaponDef(grenadeEnt->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 270, 0, "%s", "weapDef");
    iExplosionRadius = (float)weapDef->iExplosionRadius;
    ent = g_entities;
    for (i = 0; i < level.maxclients; ++i)
    {
        if (ent->r.inuse)
        {
            if (!ent->client)
                MyAssertHandler(".\\game\\g_missile.cpp", 278, 0, "%s", "ent->client");
            damageRadiusSquared = iExplosionRadius * iExplosionRadius;
            if (G_WithinDamageRadius(grenadeEnt->r.currentOrigin, damageRadiusSquared, ent))
            {
                Scr_AddString((char *)weapDef->szInternalName);
                if (grenadeEnt->parent.isDefined())
                {
                    Scr_AddEntity(grenadeEnt->parent.ent());
                }
                else
                {
                    Scr_AddUndefined();
                }
                Scr_AddEntity(grenadeEnt);
                Scr_Notify(ent, scr_const.grenadedanger, 3u);
            }
        }
        ++ent;
    }
}

bool __cdecl GrenadeDud(gentity_s *ent, WeaponDef *weapDef)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 406, 0, "%s", "ent");
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 407, 0, "%s", "weapDef");
    return weapDef->iProjectileActivateDist > 0 && ent->mover.speed < (double)weapDef->iProjectileActivateDist;
}

bool __cdecl JavelinProjectile(gentity_s *ent, WeaponDef *weapDef)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 421, 0, "%s", "ent");
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 422, 0, "%s", "weapDef");
    return ent->s.eType == 4 && weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN;
}

bool __cdecl JavelinDud(gentity_s *ent, WeaponDef *weapDef)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 435, 0, "%s", "ent");
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 436, 0, "%s", "weapDef");
    return JavelinProjectile(ent, weapDef) && ent->missile.stage == MISSILESTAGE_SOFTLAUNCH;
}

void __cdecl Missile_PenetrateGlass(
    trace_t *results,
    gentity_s *ent,
    float *start,
    float *end,
    int32_t damage,
    bool predicted)
{
    int32_t passEntityNum; // [esp+0h] [ebp-28h]
    gentity_s *v7; // [esp+4h] [ebp-24h]
    int32_t contents; // [esp+8h] [ebp-20h]
    float vel[4]; // [esp+Ch] [ebp-1Ch] BYREF
    gentity_s *hitEnt; // [esp+1Ch] [ebp-Ch]
    hitLocation_t hitLoc; // [esp+20h] [ebp-8h]
    uint16_t hitEntId; // [esp+24h] [ebp-4h]

    if (!results)
        MyAssertHandler(".\\game\\g_missile.cpp", 943, 0, "%s", "results");
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 944, 0, "%s", "ent");
    if (!start)
        MyAssertHandler(".\\game\\g_missile.cpp", 945, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\game\\g_missile.cpp", 946, 0, "%s", "end");
    hitEntId = Trace_GetEntityHitId(results);
    if (hitEntId < 0x3FEu)
    {
        hitEnt = &g_entities[hitEntId];
        if (hitEnt->takedamage)
        {
            if (damage)
            {
                BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, vel);
                if (Vec3LengthSq(vel) >= 1.0f)
                {
                    if (!predicted)
                    {
                        if (ent->r.ownerNum.isDefined())
                            v7 = ent->r.ownerNum.ent();
                        else
                            v7 = 0;
                        //LODWORD(vel[3]) = v7;
                        hitLoc = (hitLocation_t)results->partGroup;
                        G_Damage(
                            hitEnt,
                            ent,
                            v7,
                            vel,
                            ent->r.currentOrigin,
                            damage,
                            0,
                            15,
                            ent->s.weapon,
                            hitLoc,
                            results->modelIndex,
                            results->partName,
                            0);
                    }
                    contents = hitEnt->r.contents;
                    hitEnt->r.contents = 0;
                    if (ent->r.ownerNum.isDefined())
                    {
                        passEntityNum = ent->r.ownerNum.entnum();
                        G_MissileTrace(results, start, end, passEntityNum, ent->clipmask);
                    }
                    else
                    {
                        G_MissileTrace(results, start, end, 1023, ent->clipmask);
                    }
                    hitEnt->r.contents = contents;
                }
            }
        }
    }
}

void __cdecl DrawMissileDebug(float *start, float *end)
{
    float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

    if (g_debugBullets->current.integer >= 5)
        G_DebugLineWithDuration(start, end, color, 1, 1000);
}

void __cdecl RunMissile_Destabilize(gentity_s *missile)
{
    double v1; // st7
    double v2; // st7
    float min; // [esp+4h] [ebp-50h]
    float perturbationMax; // [esp+24h] [ebp-30h]
    WeaponDef *weaponDef; // [esp+28h] [ebp-2Ch]
    float newAngleAccel[3]; // [esp+2Ch] [ebp-28h]
    float direction[3]; // [esp+38h] [ebp-1Ch] BYREF
    int32_t axis; // [esp+44h] [ebp-10h]
    float newAPos[3]; // [esp+48h] [ebp-Ch] BYREF

    if (!missile)
        MyAssertHandler(".\\game\\g_missile.cpp", 1335, 0, "%s", "missile");
    if (missile->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1336, 0, "%s", "missile->s.eType == ET_MISSILE");
    if ((missile->flags & 0x20000) != 0)
        MyAssertHandler(".\\game\\g_missile.cpp", 1337, 0, "%s", "!(missile->flags & FL_STABLE_MISSILES)");
    weaponDef = BG_GetWeaponDef(missile->s.weapon);
    if (missile->s.lerp.pos.trTime + (int)missile->mover.decelTime >= level.time)
    {
        if ((missile->flags & 0x10000) == 0)
            return;
    }
    else
    {
        perturbationMax = RunMissile_GetPerturbation(weaponDef->destabilizationCurvatureMax);
        if ((missile->flags & 0x10000) != 0)
        {
            for (axis = 0; axis < 3; ++axis)
            {
                v1 = G_flrand(0.0f, 1.0f);
                newAngleAccel[axis] = v1 * perturbationMax;
            }
            if (missile->mover.pos1[2] < 0.0f)
                newAngleAccel[0] = -newAngleAccel[0];
            if (missile->mover.pos1[1] > 0.0f)
                newAngleAccel[1] = -newAngleAccel[1];
        }
        else
        {
            for (axis = 0; axis < 3; ++axis)
            {
                v2 = G_flrand(-1.0f, 1.0f);
                newAngleAccel[axis] = v2 * perturbationMax;
            }
        }
        missile->mover.pos1[1] = newAngleAccel[0];
        missile->mover.pos1[2] = newAngleAccel[1];
        missile->mover.pos2[0] = newAngleAccel[2];
        missile->s.lerp.pos.trTime = level.time;
        missile->mover.decelTime = weaponDef->destabilizationRateTime * 1000.0f;
        missile->flags |= 0x10000u;
    }
    Vec3Mad(missile->s.lerp.apos.trBase, 0.050000001f, &missile->mover.pos1[1], newAPos);
    G_SetAngle(missile, newAPos);
    AngleVectors(newAPos, direction, 0, 0);
    min = (float)weaponDef->iProjectileSpeed;
    Vec3Scale(direction, min, missile->s.lerp.pos.trDelta);
    if ((COERCE_UNSIGNED_INT(missile->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(missile->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(missile->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_missile.cpp",
            1379,
            0,
            "%s",
            "!IS_NAN((missile->s.lerp.pos.trDelta)[0]) && !IS_NAN((missile->s.lerp.pos.trDelta)[1]) && !IS_NAN((missile->s.lerp"
            ".pos.trDelta)[2])");
    }
    missile->s.lerp.pos.trBase[0] = missile->r.currentOrigin[0];
    missile->s.lerp.pos.trBase[1] = missile->r.currentOrigin[1];
    missile->s.lerp.pos.trBase[2] = missile->r.currentOrigin[2];
    missile->s.lerp.pos.trType = TR_INTERPOLATE;
    missile->s.lerp.apos.trType = TR_INTERPOLATE;
    Missile_ApplyAttractorsRepulsors(missile);
}

double __cdecl RunMissile_GetPerturbation(float destabilizationCurvatureMax)
{
    if (destabilizationCurvatureMax >= 1000000000.0f || destabilizationCurvatureMax < 0.0f)
        MyAssertHandler(
            ".\\game\\g_missile.cpp",
            997,
            0,
            "%s\n\t(destabilizationCurvatureMax) = %g",
            "(destabilizationCurvatureMax < 1000000000.0f && destabilizationCurvatureMax >= 0.0f)",
            destabilizationCurvatureMax);
    return destabilizationCurvatureMax;
}

void __cdecl Missile_ApplyAttractorsRepulsors(gentity_s *missile)
{
    float scale; // [esp+0h] [ebp-ACh]
    float iProjectileSpeed; // [esp+4h] [ebp-A8h]
    float v4; // [esp+10h] [ebp-9Ch]
    float v5; // [esp+14h] [ebp-98h]
    double v6; // [esp+18h] [ebp-94h]
    float v7; // [esp+20h] [ebp-8Ch]
    float v8; // [esp+24h] [ebp-88h]
    float v9; // [esp+28h] [ebp-84h]
    float maxCorrectingAccel; // [esp+3Ch] [ebp-70h]
    float maxCorrectingVel; // [esp+40h] [ebp-6Ch]
    float perpDelta[3]; // [esp+44h] [ebp-68h] BYREF
    float delta[3]; // [esp+50h] [ebp-5Ch] BYREF
    float forwardDist; // [esp+5Ch] [ebp-50h]
    float forceVector[3]; // [esp+60h] [ebp-4Ch] BYREF
    float force; // [esp+6Ch] [ebp-40h]
    WeaponDef *weaponDef; // [esp+70h] [ebp-3Ch]
    float totalDist; // [esp+74h] [ebp-38h]
    uint32_t attractorIndex; // [esp+78h] [ebp-34h]
    float attractorOrigin[3]; // [esp+7Ch] [ebp-30h] BYREF
    float forwardDir[3]; // [esp+88h] [ebp-24h] BYREF
    gentity_s *ent; // [esp+94h] [ebp-18h]
    float angleToAttractor; // [esp+98h] [ebp-14h]
    float perpDir[3]; // [esp+9Ch] [ebp-10h] BYREF
    float perpDist; // [esp+A8h] [ebp-4h]

    weaponDef = BG_GetWeaponDef(missile->s.weapon);
    if (Vec3NormalizeTo(missile->s.lerp.pos.trDelta, forwardDir) < 0.000009999999747378752f)
        return;
    forceVector[0] = 0.0f;
    forceVector[1] = 0.0f;
    forceVector[2] = 0.0f;
    for (attractorIndex = 0; attractorIndex < 0x20; ++attractorIndex)
    {
        if (attrGlob.attractors[attractorIndex].inUse)
        {
            if (attrGlob.attractors[attractorIndex].entnum == 1023)
            {
                attractorOrigin[0] = attrGlob.attractors[attractorIndex].origin[0];
                attractorOrigin[1] = attrGlob.attractors[attractorIndex].origin[1];
                attractorOrigin[2] = attrGlob.attractors[attractorIndex].origin[2];
            }
            else
            {
                if (attrGlob.attractors[attractorIndex].entnum >= 1024)
                    MyAssertHandler(
                        ".\\game\\g_missile.cpp",
                        1262,
                        0,
                        "%s",
                        "attrGlob.attractors[attractorIndex].entnum < MAX_GENTITIES");
                ent = &g_entities[attrGlob.attractors[attractorIndex].entnum];
                attractorOrigin[0] = ent->r.currentOrigin[0];
                attractorOrigin[1] = ent->r.currentOrigin[1];
                attractorOrigin[2] = ent->r.currentOrigin[2];
            }
            Vec3Sub(attractorOrigin, missile->s.lerp.pos.trBase, delta);
            forwardDist = Vec3Dot(delta, forwardDir);
            if (forwardDist > 0.0f)
            {
                scale = -forwardDist;
                Vec3Mad(delta, scale, forwardDir, perpDelta);
                perpDist = Vec3NormalizeTo(perpDelta, perpDir);
                if (perpDist < 0.000009999999747378752f)
                {
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                        continue;
                    perpDir[0] = 0.0f;
                    perpDir[1] = 0.0f;
                    perpDir[2] = -1.0f;
                }
                if (!attrGlob.attractors[attractorIndex].isAttractor && perpDir[2] > 0.0f)
                {
                    perpDir[0] = 0.0f;
                    perpDir[1] = 0.0f;
                    perpDir[2] = -1.0f;
                    perpDist = 0.0f;
                }
                totalDist = Vec3Length(delta);
                if (attrGlob.attractors[attractorIndex].maxDist >= (double)totalDist)
                {
                    if (attrGlob.attractors[attractorIndex].maxDist <= 0.0)
                        MyAssertHandler(".\\game\\g_missile.cpp", 1294, 0, "%s", "attrGlob.attractors[attractorIndex].maxDist > 0");
                    force = 1.0 - totalDist / attrGlob.attractors[attractorIndex].maxDist;
                    force = force * attrGlob.attractors[attractorIndex].strength;
                    v9 = fabs(perpDist);
                    v8 = v9 / forwardDist;
                    v7 = atan(v8);
                    angleToAttractor = v7 * 0.6366202831268311f;
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                        v6 = angleToAttractor;
                    else
                        v6 = angleToAttractor - 1.0f;
                    force = force * v6;
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                    {
                        maxCorrectingVel = (float)weaponDef->iProjectileSpeed * perpDist / forwardDist;
                        maxCorrectingAccel = maxCorrectingVel * 20.0f;
                        v5 = force - maxCorrectingAccel;
                        if (v5 < 0.0f)
                            v4 = force;
                        else
                            v4 = maxCorrectingVel * 20.0f;
                        force = v4;
                    }
                    Vec3Mad(forceVector, force, perpDir, forceVector);
                }
            }
        }
    }
    if (0.0f != forceVector[0] || 0.0f != forceVector[1] || 0.0f != forceVector[2])
    {
        Vec3Mad(missile->s.lerp.pos.trDelta, 0.050000001f, forceVector, missile->s.lerp.pos.trDelta);
        Vec3NormalizeTo(missile->s.lerp.pos.trDelta, forwardDir);
        iProjectileSpeed = (float)weaponDef->iProjectileSpeed;
        Vec3Scale(forwardDir, iProjectileSpeed, missile->s.lerp.pos.trDelta);
        vectoangles(forwardDir, missile->s.lerp.apos.trBase);
    }
}

void __cdecl RunMissile_CreateWaterSplash(const gentity_s *missile, const trace_t *trace)
{
    gentity_s *tent; // [esp+0h] [ebp-10h]
    float reflect[3]; // [esp+4h] [ebp-Ch] BYREF

    if (!missile)
        MyAssertHandler(".\\game\\g_missile.cpp", 1393, 0, "%s", "missile");
    if (!trace)
        MyAssertHandler(".\\game\\g_missile.cpp", 1394, 0, "%s", "trace");
    Vec3NormalizeTo(missile->s.lerp.pos.trDelta, reflect);
    if (reflect[2] < 0.0f)
        reflect[2] = reflect[2] * -1.0f;
    tent = G_TempEntity(missile->r.currentOrigin, 44);
    tent->s.eventParm = DirToByte(trace->normal);
    tent->s.un1.scale = 0;
    tent->s.surfType = (trace->surfaceFlags & 0x1F00000) >> 20;
    tent->s.otherEntityNum = missile->s.number;
    tent->s.weapon = missile->s.weapon;
}

void __cdecl MissileTrajectory(gentity_s *ent, float *result)
{
    float v2; // [esp+Ch] [ebp-34h]
    float scale; // [esp+10h] [ebp-30h]
    float dir[3]; // [esp+1Ch] [ebp-24h] BYREF
    float dbgStart[3]; // [esp+28h] [ebp-18h] BYREF
    float forwardSpeed; // [esp+34h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+38h] [ebp-8h]
    float accel; // [esp+3Ch] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1980, 0, "%s", "ent");
    if (!result)
        MyAssertHandler(".\\game\\g_missile.cpp", 1981, 0, "%s", "result");
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 1984, 0, "%s", "weapDef");
    if (level.time > ent->s.lerp.u.missile.launchTime && ent->s.lerp.pos.trType != TR_LINEAR && ent->handler == 9)
    {
        if (weapDef->timeToAccelerate > 0.0)
        {
            AngleVectors(ent->r.currentAngles, dir, 0, 0);
            forwardSpeed = Vec3Dot(dir, ent->s.lerp.pos.trDelta);
            if (forwardSpeed < (float)weapDef->iProjectileSpeed)
            {
                accel = (float)weapDef->iProjectileSpeed / weapDef->timeToAccelerate;
                v2 = accel * 0.05000000074505806f;
                Vec3Mad(ent->s.lerp.pos.trDelta, v2, dir, ent->s.lerp.pos.trDelta);
            }
            else
            {
                scale = (float)weapDef->iProjectileSpeed - forwardSpeed;
                Vec3Mad(ent->s.lerp.pos.trDelta, scale, dir, ent->s.lerp.pos.trDelta);
            }
        }
        if (weapDef->projectileCurvature > 0.0f)
            Vec3Mad(ent->s.lerp.pos.trDelta, 0.050000001f, &ent->mover.pos1[1], ent->s.lerp.pos.trDelta);
        if (missileDebugDraw->current.enabled)
        {
            dbgStart[0] = ent->r.currentOrigin[0];
            dbgStart[1] = ent->r.currentOrigin[1];
            dbgStart[2] = ent->r.currentOrigin[2];
        }
        GuidedMissileSteering(ent);
        if (weapDef->guidedMissileType != MISSILE_GUIDANCE_JAVELIN || ent->missile.stage)
        {
            Vec3Mad(ent->s.lerp.pos.trBase, 0.050000001f, ent->s.lerp.pos.trDelta, ent->s.lerp.pos.trBase);
            *result = ent->s.lerp.pos.trBase[0];
            result[1] = ent->s.lerp.pos.trBase[1];
            result[2] = ent->s.lerp.pos.trBase[2];
        }
        else
        {
            BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, result);
        }
        if (missileDebugDraw->current.enabled)
        {
            if (weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN && ent->missile.stage == MISSILESTAGE_ASCENT)
                G_DebugLineWithDuration(dbgStart, result, colorOrange, 0, 200);
            else
                G_DebugLineWithDuration(dbgStart, result, colorRed, 0, 200);
        }
        if (ent->s.lerp.apos.trType == TR_INTERPOLATE && MissileIsReadyForSteering(ent))
        {
            vectoangles(ent->s.lerp.pos.trDelta, ent->r.currentAngles);
            G_SetAngle(ent, ent->r.currentAngles);
            ent->s.lerp.apos.trType = TR_INTERPOLATE;
        }
    }
    else
    {
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, result);
    }
}

bool __cdecl MissileIsReadyForSteering(gentity_s *ent)
{
    float timeLeft; // [esp+8h] [ebp-8h]

    timeLeft = BG_GetWeaponDef(ent->s.weapon)->timeToAccelerate
        - (float)(level.time - ent->s.lerp.pos.trTime) * EQUAL_EPSILON;
    return timeLeft <= 0.0f;
}

void __cdecl GuidedMissileSteering(gentity_s *ent)
{
    float v1; // [esp+10h] [ebp-4Ch]
    float currentHorzSpeed; // [esp+14h] [ebp-48h]
    float currentRight[2]; // [esp+18h] [ebp-44h] BYREF
    float toTarget[3]; // [esp+20h] [ebp-3Ch] BYREF
    float currentHorzDir[2]; // [esp+2Ch] [ebp-30h] BYREF
    float steer[3]; // [esp+34h] [ebp-28h] BYREF
    float targetPos[3]; // [esp+40h] [ebp-1Ch] BYREF
    WeaponDef *weapDef; // [esp+4Ch] [ebp-10h]
    float toTargetRelative[3]; // [esp+50h] [ebp-Ch] BYREF

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (weapDef->guidedMissileType
        && ent->missileTargetEnt.isDefined()
        && IsMissileLockedOn(ent)
        && MissileIsReadyForSteering(ent))
    {
        if (weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN)
        {
            JavelinSteering(ent, weapDef);
        }
        else
        {
            currentHorzSpeed = Vec2NormalizeTo(ent->s.lerp.pos.trDelta, currentHorzDir);
            v1 = -currentHorzDir[0];
            currentRight[0] = currentHorzDir[1];
            currentRight[1] = v1;
            GetTargetPosition(ent, targetPos);
            if (missileDebugDraw->current.enabled)
                G_DebugLineWithDuration(ent->s.lerp.pos.trBase, targetPos, colorGreen, 0, 200);
            Vec3Sub(targetPos, ent->s.lerp.pos.trBase, toTarget);
            toTargetRelative[0] = currentHorzDir[1] * toTarget[1] + currentHorzDir[0] * toTarget[0];
            toTargetRelative[1] = currentRight[1] * toTarget[1] + currentRight[0] * toTarget[0];
            toTargetRelative[2] = toTarget[2];
            steer[0] = 0.0f;
            steer[1] = 0.0f;
            steer[2] = 0.0f;
            MissileHorzSteerToTarget(ent, currentRight, toTargetRelative, currentHorzSpeed, steer);
            MissileVerticalSteering(ent, toTargetRelative, currentHorzSpeed, steer);
            Vec3Mad(ent->s.lerp.pos.trDelta, 0.050000001f, steer, ent->s.lerp.pos.trDelta);
        }
    }
}

bool IsMissileLockedOn(gentity_s *ent)
{
    if (!ent->missileTargetEnt.isDefined())
        return 0;

    gentity_s *target = ent->missileTargetEnt.ent();

    iassert(target->r.inuse);

    return 1;
}

void __cdecl MissileHorzSteerToTarget(
    gentity_s *ent,
    const float *currentRight,
    const float *toTargetRelative,
    float currentHorzSpeed,
    float *steer)
{
    double v5; // st7
    float v6; // [esp+0h] [ebp-3Ch]
    float v7; // [esp+4h] [ebp-38h]
    float v8; // [esp+8h] [ebp-34h]
    float v9; // [esp+Ch] [ebp-30h]
    float v10; // [esp+10h] [ebp-2Ch]
    float v11; // [esp+14h] [ebp-28h]
    float v12; // [esp+18h] [ebp-24h]
    float maxSteeringAccel; // [esp+1Ch] [ebp-20h]
    float v14; // [esp+20h] [ebp-1Ch]
    float v15; // [esp+24h] [ebp-18h]
    float v16; // [esp+28h] [ebp-14h]
    float radius; // [esp+2Ch] [ebp-10h]
    float tightestRadius; // [esp+30h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+34h] [ebp-8h]
    float accel; // [esp+38h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (toTargetRelative[1] == 0.0)
    {
        radius = FLT_MAX;
    }
    else
    {
        v11 = toTargetRelative[1] * toTargetRelative[1] + *toTargetRelative * *toTargetRelative;
        radius = v11 / (toTargetRelative[1] * 2.0);
    }
    if (weapDef->maxSteeringAccel <= 0.0)
        MyAssertHandler(".\\game\\g_missile.cpp", 1471, 0, "%s", "weapDef->maxSteeringAccel > 0");
    tightestRadius = (float)weapDef->iProjectileSpeed * (float)weapDef->iProjectileSpeed / weapDef->maxSteeringAccel;
    if (*toTargetRelative <= 0.0f)
    {
        v6 = fabs(radius);
        if (v6 >= tightestRadius + 60.0f)
        {
            if (toTargetRelative[1] <= 0.0f)
            {
                v12 = -weapDef->maxSteeringAccel;
                *steer = v12 * *currentRight;
                v5 = v12 * currentRight[1];
            }
            else
            {
                maxSteeringAccel = weapDef->maxSteeringAccel;
                *steer = maxSteeringAccel * *currentRight;
                v5 = maxSteeringAccel * currentRight[1];
            }
            steer[1] = v5;
        }
    }
    else if (toTargetRelative[1] != 0.0f)
    {
        v10 = fabs(radius);
        if (tightestRadius <= v10)
        {
            if (radius == 0.0f)
                MyAssertHandler(".\\game\\g_missile.cpp", 1482, 0, "%s", "radius != 0");
            accel = currentHorzSpeed * 2.0f * currentHorzSpeed / radius;
            v15 = weapDef->maxSteeringAccel;
            v9 = accel - v15;
            if (v9 < 0.0f)
                v16 = currentHorzSpeed * 2.0f * currentHorzSpeed / radius;
            else
                v16 = v15;
            v14 = -weapDef->maxSteeringAccel;
            v8 = v14 - accel;
            if (v8 < 0.0f)
                v7 = v16;
            else
                v7 = -weapDef->maxSteeringAccel;
            *steer = v7 * *currentRight;
            steer[1] = v7 * currentRight[1];
        }
    }
}

void __cdecl MissileVerticalSteering(
    gentity_s *ent,
    const float *toTargetRelative,
    float currentHorzSpeed,
    float *steer)
{
    float v4; // [esp+10h] [ebp-38h]
    float v5; // [esp+14h] [ebp-34h]
    float v6; // [esp+18h] [ebp-30h]
    guidedMissileType_t guidedMissileType; // [esp+1Ch] [ebp-2Ch]
    float v8; // [esp+20h] [ebp-28h]
    float v9; // [esp+24h] [ebp-24h]
    float value; // [esp+28h] [ebp-20h]
    float v11; // [esp+2Ch] [ebp-1Ch]
    float toTargetHorzRelativeDir[2]; // [esp+30h] [ebp-18h] BYREF
    float horzDistToTarg; // [esp+38h] [ebp-10h]
    float minTimeToTarg; // [esp+3Ch] [ebp-Ch]
    float maxVertSpeedAtMaxAccel; // [esp+40h] [ebp-8h]
    WeaponDef *weapDef; // [esp+44h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    horzDistToTarg = Vec2NormalizeTo(toTargetRelative, toTargetHorzRelativeDir);
    steer[2] = 0.0f;
    if (horzDistToTarg != 0.0f)
    {
        guidedMissileType = weapDef->guidedMissileType;
        if (guidedMissileType == MISSILE_GUIDANCE_SIDEWINDER)
            goto LABEL_7;
        if (guidedMissileType != MISSILE_GUIDANCE_HELLFIRE)
            return;
        if (ent->s.lerp.pos.trDuration)
        {
        LABEL_7:
            MissileVerticalSteerToTarget(
                ent,
                toTargetHorzRelativeDir,
                horzDistToTarg,
                toTargetRelative[2],
                currentHorzSpeed,
                steer);
            return;
        }
        if (*toTargetRelative <= 0.0f || currentHorzSpeed == 0.0f)
        {
            if (!weapDef->iProjectileSpeed)
                MyAssertHandler(".\\game\\g_missile.cpp", 1552, 0, "%s", "weapDef->iProjectileSpeed != 0");
            minTimeToTarg = horzDistToTarg / (float)weapDef->iProjectileSpeed;
        }
        else
        {
            minTimeToTarg = *toTargetRelative / (float)weapDef->iProjectileSpeed;
        }
        if (minTimeToTarg <= 0.0f)
            MyAssertHandler(".\\game\\g_missile.cpp", 1556, 0, "%s", "minTimeToTarg > 0");
        maxVertSpeedAtMaxAccel = toTargetRelative[2] / minTimeToTarg;
        maxVertSpeedAtMaxAccel = weapDef->maxSteeringAccel * 0.5f * minTimeToTarg + maxVertSpeedAtMaxAccel;
        maxVertSpeedAtMaxAccel = maxVertSpeedAtMaxAccel * 0.8999999761581421f;
        if (maxVertSpeedAtMaxAccel > currentHorzSpeed * missileHellfireMaxSlope->current.value)
            maxVertSpeedAtMaxAccel = currentHorzSpeed * missileHellfireMaxSlope->current.value;
        if (ent->s.lerp.pos.trDelta[2] >= (float)maxVertSpeedAtMaxAccel)
        {
            ent->s.lerp.pos.trDuration = 1;
        }
        else
        {
            steer[2] = (maxVertSpeedAtMaxAccel - ent->s.lerp.pos.trDelta[2]) * 20.0f;
            v8 = steer[2];
            value = missileHellfireUpAccel->current.value;
            v6 = v8 - value;
            if (v6 < 0.0f)
                v11 = v8;
            else
                v11 = value;
            v9 = -weapDef->maxSteeringAccel;
            v5 = v9 - v8;
            if (v5 < 0.0f)
                v4 = v11;
            else
                v4 = -weapDef->maxSteeringAccel;
            steer[2] = v4;
        }
    }
}

void __cdecl MissileVerticalSteerToTarget(
    gentity_s *ent,
    const float *toTargetHorzRelDir,
    float horzDistToTarg,
    float vertDistToTarg,
    float currentHorzSpeed,
    float *steer)
{
    float v6; // [esp+0h] [ebp-2Ch]
    float v7; // [esp+4h] [ebp-28h]
    float v8; // [esp+8h] [ebp-24h]
    float v9; // [esp+Ch] [ebp-20h]
    float v10; // [esp+10h] [ebp-1Ch]
    float v11; // [esp+14h] [ebp-18h]
    float maxSteeringAccel; // [esp+18h] [ebp-14h]
    float v13; // [esp+1Ch] [ebp-10h]
    float wishVertSpeed; // [esp+20h] [ebp-Ch]
    float horzSpeedToTarg; // [esp+24h] [ebp-8h]
    WeaponDef *weapDef; // [esp+28h] [ebp-4h]

    if (*toTargetHorzRelDir >= 0.0f)
    {
        weapDef = BG_GetWeaponDef(ent->s.weapon);
        horzSpeedToTarg = *toTargetHorzRelDir * currentHorzSpeed;
        v9 = fabs(horzSpeedToTarg);
        wishVertSpeed = v9 * vertDistToTarg / horzDistToTarg;
        steer[2] = (wishVertSpeed - ent->s.lerp.pos.trDelta[2]) * 20.0f;
        v10 = steer[2];
        maxSteeringAccel = weapDef->maxSteeringAccel;
        v8 = v10 - maxSteeringAccel;
        if (v8 < 0.0f)
            v13 = v10;
        else
            v13 = maxSteeringAccel;
        v11 = -weapDef->maxSteeringAccel;
        v7 = v11 - v10;
        if (v7 < 0.0f)
            v6 = v13;
        else
            v6 = -weapDef->maxSteeringAccel;
        steer[2] = v6;
    }
}

void __cdecl GetTargetPosition(gentity_s *ent, float *result)
{
    gentity_s *target; // [esp+0h] [ebp-4h]

    iassert(ent);
    iassert(ent->s.eType == ET_MISSILE);
    iassert(ent->missileTargetEnt.isDefined());

    target = ent->missileTargetEnt.ent();
    Vec3Add(target->r.currentOrigin, &ent->mover.pos2[1], result);
}

void __cdecl JavelinSteering(gentity_s *ent, WeaponDef *weapDef)
{
    const char *v2; // [esp+20h] [ebp-50h]
    float diff[6]; // [esp+28h] [ebp-48h] BYREF
    float *trBase; // [esp+40h] [ebp-30h]
    float *currentOrigin; // [esp+44h] [ebp-2Ch]
    float limit; // [esp+48h] [ebp-28h]
    float distance2D; // [esp+4Ch] [ebp-24h]
    float height; // [esp+50h] [ebp-20h]
    float distance3D; // [esp+54h] [ebp-1Ch]
    float toTarget[3]; // [esp+58h] [ebp-18h] BYREF
    float targetPos[3]; // [esp+64h] [ebp-Ch] BYREF

    iassert(ent);
    iassert(ent->missileTargetEnt.isDefined());

    if (ent->missile.stage == MISSILESTAGE_SOFTLAUNCH)
    {
        if (level.time - ent->s.lerp.u.missile.launchTime < weapDef->projIgnitionDelay)
        {
            if (missileDebugText->current.enabled)
                Com_Printf(15, "Javelin: softlaunch\n");
            return;
        }
        ent->missile.stage = MISSILESTAGE_ASCENT;
        ent->s.lerp.pos.trType = TR_INTERPOLATE;
        trBase = ent->s.lerp.pos.trBase;
        currentOrigin = ent->r.currentOrigin;
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        trBase[1] = currentOrigin[1];
        trBase[2] = currentOrigin[2];
    }
    GetTargetPosition(ent, targetPos);
    if (ent->missile.stage == MISSILESTAGE_ASCENT)
    {
        if (JavelinClimbEnd(ent, targetPos))
            ent->missile.stage = MISSILESTAGE_DESCENT;
        else
            JavelinClimbOffset(ent, targetPos);
    }
    Vec3Sub(targetPos, ent->s.lerp.pos.trBase, toTarget);
    Vec3Normalize(toTarget);
    JavelinRotateVelocity(ent, ent->s.lerp.pos.trDelta, toTarget, ent->s.lerp.pos.trDelta);
    if (missileDebugText->current.enabled)
    {
        height = ent->s.lerp.pos.trBase[2] - targetPos[2] - ent->mover.pos3[0];
        limit = JavelinClimbCeiling(ent);
        distance2D = Vec2Distance(ent->s.lerp.pos.trBase, targetPos);
        Vec3Sub(targetPos, ent->s.lerp.pos.trBase, diff);
        distance3D = Vec3Length(diff);
        if (ent->missile.stage == MISSILESTAGE_ASCENT)
            v2 = "A";
        else
            v2 = "D";
        Com_Printf(15, "Jav:%s h:%.0f/%.0f dist 2d:%.0f 3d:%.0f\n", v2, height, limit, distance2D, distance3D);
    }
}

void __cdecl JavelinClimbOffset(gentity_s *ent, float *targetPos)
{
    gentity_s *v2; // [esp+0h] [ebp-20h]
    float value; // [esp+4h] [ebp-1Ch]
    gentity_s *target; // [esp+14h] [ebp-Ch]
    float ownerDir[2]; // [esp+18h] [ebp-8h] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1673, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1674, 0, "%s", "ent->s.eType == ET_MISSILE");
    if (ent->missile.flightMode)
    {
        if (ent->missile.flightMode != MISSILEFLIGHTMODE_DIRECT)
            MyAssertHandler(".\\game\\g_missile.cpp", 1682, 0, "%s", "ent->missile.flightMode == MISSILEFLIGHTMODE_DIRECT");
        targetPos[2] = targetPos[2] + missileJavClimbHeightDirect->current.value;

        if (ent->r.ownerNum.isDefined())
            v2 = ent->r.ownerNum.ent();
        else
            v2 = &g_entities[1023];

        iassert(ent->missileTargetEnt.isDefined());

        target = ent->missileTargetEnt.ent();
        ownerDir[0] = v2->s.lerp.pos.trBase[0] - target->s.lerp.pos.trBase[0];
        ownerDir[1] = v2->s.lerp.pos.trBase[1] - target->s.lerp.pos.trBase[1];
        Vec2Normalize(ownerDir);
        value = missileJavClimbToOwner->current.value;
        ownerDir[0] = value * ownerDir[0];
        ownerDir[1] = value * ownerDir[1];
        *targetPos = *targetPos + ownerDir[0];
        targetPos[1] = targetPos[1] + ownerDir[1];
    }
    else
    {
        targetPos[2] = targetPos[2] + missileJavClimbHeightTop->current.value;
    }
}

void __cdecl JavelinRotateVelocity(gentity_s *ent, const float *currentVel, const float *targetDir, float *resultVel)
{
    float currentDir[3]; // [esp+8h] [ebp-20h] BYREF
    float len; // [esp+14h] [ebp-14h]
    float resultDir[3]; // [esp+18h] [ebp-10h] BYREF
    float turnDiff; // [esp+24h] [ebp-4h]

    len = Vec3NormalizeTo(currentVel, currentDir);
    turnDiff = JavelinRotateDir(ent, currentDir, targetDir, resultDir);
    if (turnDiff < 30.0f)
    {
        if (ent->missile.stage == MISSILESTAGE_ASCENT || currentVel[2] > 0.0f)
        {
            len = missileJavAccelClimb->current.value * 0.05000000074505806f + len;
            if (missileJavSpeedLimitClimb->current.value < len)
                len = missileJavSpeedLimitClimb->current.value;
        }
        else
        {
            if (ent->missile.stage != MISSILESTAGE_DESCENT)
                MyAssertHandler(".\\game\\g_missile.cpp", 1804, 0, "%s", "ent->missile.stage == MISSILESTAGE_DESCENT");
            len = missileJavAccelDescend->current.value * 0.05000000074505806f + len;
            if (missileJavSpeedLimitDescend->current.value < len)
                len = missileJavSpeedLimitDescend->current.value;
        }
    }
    len = (1.0 - turnDiff / 180.0 * missileJavTurnDecel->current.value) * len;
    Vec3Scale(resultDir, len, resultVel);
}

double __cdecl JavelinRotateDir(gentity_s *ent, const float *currentDir, const float *targetDir, float *resultDir)
{
    float end[3]; // [esp+20h] [ebp-4Ch] BYREF
    float maxDPS; // [esp+2Ch] [ebp-40h]
    float targetQuat[4]; // [esp+30h] [ebp-3Ch] BYREF
    float frac; // [esp+40h] [ebp-2Ch]
    float resultQuat[4]; // [esp+44h] [ebp-28h] BYREF
    float targetDPS; // [esp+54h] [ebp-18h]
    float currentQuat[4]; // [esp+58h] [ebp-14h] BYREF
    float dot; // [esp+68h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1730, 0, "%s", "ent");
    maxDPS = JavelinMaxDPS(ent);
    dot = Vec3Dot(targetDir, currentDir);
    dot = (1.0 - (dot + 1.0) * 0.5) * 180.0;
    if (dot > 0.1)
    {
        targetDPS = dot / 0.05000000074505806f;
        if (maxDPS <= targetDPS)
        {
            frac = maxDPS / targetDPS;
            if (missileDebugText->current.enabled)
                Com_Printf(15, "dot:%.2f frac:%.2f =%.0f/%.0f ", dot, frac, maxDPS, targetDPS);
            VecToQuat(currentDir, currentQuat);
            VecToQuat(targetDir, targetQuat);
            QuatSlerp(currentQuat, targetQuat, frac, resultQuat);
            Vec4Normalize(resultQuat);
            UnitQuatToForward(resultQuat, resultDir);
            if (missileDebugDraw->current.enabled)
            {
                Vec3Mad(ent->s.lerp.pos.trBase, 100.0f, currentDir, end);
                G_DebugLineWithDuration(ent->s.lerp.pos.trBase, end, colorBlue, 0, 200);
                Vec3Mad(ent->s.lerp.pos.trBase, 100.0f, targetDir, end);
                G_DebugLineWithDuration(ent->s.lerp.pos.trBase, end, colorLtGrey, 0, 200);
                Vec3Mad(ent->s.lerp.pos.trBase, 100.0f, resultDir, end);
                G_DebugLineWithDuration(ent->s.lerp.pos.trBase, end, colorGreen, 0, 200);
            }
            return dot;
        }
        else
        {
            *resultDir = *targetDir;
            resultDir[1] = targetDir[1];
            resultDir[2] = targetDir[2];
            if (missileDebugText->current.enabled)
                Com_Printf(15, "dot:%.2f (%.0f > %.0f) ", dot, maxDPS, targetDPS);
            return 0.0;
        }
    }
    else
    {
        *resultDir = *targetDir;
        resultDir[1] = targetDir[1];
        resultDir[2] = targetDir[2];
        if (missileDebugText->current.enabled)
            Com_Printf(15, "dot:%.2f ", dot);
        return 0.0;
    }
}

double __cdecl JavelinMaxDPS(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1698, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1699, 0, "%s", "ent->s.eType == ET_MISSILE");
    if (ent->missile.flightMode == MISSILEFLIGHTMODE_TOP)
        return missileJavTurnRateTop->current.value;
    if (ent->missile.flightMode != MISSILEFLIGHTMODE_DIRECT)
        MyAssertHandler(".\\game\\g_missile.cpp", 1703, 0, "%s", "ent->missile.flightMode == MISSILEFLIGHTMODE_DIRECT");
    return missileJavTurnRateDirect->current.value;
}

void __cdecl VecToQuat(const float *vec, float *quat)
{
    float angles[3]; // [esp+0h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    vectoangles(vec, angles);
    AnglesToAxis(angles, axis);
    AxisToQuat(axis, quat);
}

double __cdecl JavelinClimbCeiling(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1819, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1820, 0, "%s", "ent->s.eType == ET_MISSILE");
    if (ent->missile.flightMode == MISSILEFLIGHTMODE_DIRECT)
        return missileJavClimbCeilingDirect->current.value;
    if (ent->missile.flightMode)
        MyAssertHandler(".\\game\\g_missile.cpp", 1828, 0, "%s", "ent->missile.flightMode == MISSILEFLIGHTMODE_TOP");
    return missileJavClimbCeilingTop->current.value;
}

bool __cdecl JavelinClimbEnd(gentity_s *ent, const float *targetPos)
{
    if (!JavelinClimbIsAboveCeiling(ent, targetPos))
        return 0;
    if (JavelinClimbExceededAngle(ent, targetPos))
        return 1;
    return JavelinClimbWithinDistance(ent, targetPos) != 0;
}

char __cdecl JavelinClimbExceededAngle(gentity_s *ent, const float *targetPos)
{
    float v3; // [esp+0h] [ebp-30h]
    float v4; // [esp+4h] [ebp-2Ch]
    float v5; // [esp+Ch] [ebp-24h]
    float limit; // [esp+10h] [ebp-20h]
    float toTarget[3]; // [esp+14h] [ebp-1Ch] BYREF
    float currentHorzDir[2]; // [esp+20h] [ebp-10h] BYREF
    float deltaHorz; // [esp+28h] [ebp-8h]
    float deg; // [esp+2Ch] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1615, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1616, 0, "%s", "ent->s.eType == ET_MISSILE");
    if (ent->missile.flightMode)
    {
        if (ent->missile.flightMode != MISSILEFLIGHTMODE_DIRECT)
            MyAssertHandler(".\\game\\g_missile.cpp", 1624, 0, "%s", "ent->missile.flightMode == MISSILEFLIGHTMODE_DIRECT");
        limit = missileJavClimbAngleDirect->current.value;
    }
    else
    {
        limit = missileJavClimbAngleTop->current.value;
    }
    Vec2NormalizeTo(ent->s.lerp.pos.trDelta, currentHorzDir);
    Vec3Sub(targetPos, ent->s.lerp.pos.trBase, toTarget);
    deltaHorz = currentHorzDir[1] * toTarget[1] + currentHorzDir[0] * toTarget[0];
    v5 = deltaHorz / toTarget[2];
    v4 = atan(v5);
    deg = v4 * 57.2957763671875f;
    v3 = fabs(deg);
    deg = v3;
    if (limit <= (double)v3)
        return 0;
    if (missileDebugText->current.enabled)
        Com_Printf(15, "Javelin: *** Exceeded climb angle ***\n");
    return 1;
}

char __cdecl JavelinClimbWithinDistance(gentity_s *ent, const float *targetPos)
{
    float distance; // [esp+0h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1651, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1652, 0, "%s", "ent->s.eType == ET_MISSILE");
    distance = Vec2Distance(ent->s.lerp.pos.trBase, targetPos);
    if (distance >= 400.0)
        return 0;
    if (missileDebugText->current.enabled)
        Com_Printf(15, "Javelin: *** Exceeded climb distance ***\n");
    return 1;
}

bool __cdecl JavelinClimbIsAboveCeiling(gentity_s *ent, const float *targetPos)
{
    float limit; // [esp+0h] [ebp-8h]
    float height; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_missile.cpp", 1839, 0, "%s", "ent");
    if (ent->s.eType != 4)
        MyAssertHandler(".\\game\\g_missile.cpp", 1840, 0, "%s", "ent->s.eType == ET_MISSILE");
    height = ent->s.lerp.pos.trBase[2] - targetPos[2] - ent->mover.pos3[0];
    limit = JavelinClimbCeiling(ent);
    return limit < (double)height;
}

void __cdecl G_InitGrenadeEntity(gentity_s *parent, gentity_s *grenade)
{
    gentity_s *ent; // [esp+0h] [ebp-10h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (!parent)
        MyAssertHandler(".\\game\\g_missile.cpp", 2496, 0, "%s", "parent");
    if (!grenade)
        MyAssertHandler(".\\game\\g_missile.cpp", 2497, 0, "%s", "grenade");
    if (!grenade->s.weapon)
        MyAssertHandler(".\\game\\g_missile.cpp", 2499, 0, "%s", "grenade->s.weapon");
    weapDef = BG_GetWeaponDef(grenade->s.weapon);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 2501, 0, "%s", "weapDef");
    grenade->s.eType = 4;
    grenade->s.lerp.eFlags = 0x1000000;
    grenade->s.lerp.u.missile.launchTime = level.time;
    grenade->r.contents = 256;
    grenade->r.contents |= 0x2000u;
    grenade->r.mins[0] = -1.5f;
    grenade->r.mins[1] = -1.5f;
    grenade->r.mins[2] = -1.5f;
    grenade->r.maxs[0] = 1.5f;
    grenade->r.maxs[1] = 1.5f;
    grenade->r.maxs[2] = 1.5f;
    if (!parent->client || parent->client->ps.grenadeTimeLeft >= 0 || parent->client->ps.throwBackGrenadeOwner == 1023)
    {
        grenade->r.ownerNum.setEnt(parent);
        grenade->parent.setEnt(parent);
    }
    else
    {
        grenade->r.ownerNum.setEnt(&g_entities[parent->client->ps.throwBackGrenadeOwner]);
        if (grenade->r.ownerNum.isDefined())
        {
            ent = grenade->r.ownerNum.ent();
            grenade->parent.setEnt(ent);
        }
        else
        {
            grenade->parent.setEnt(NULL);
        }
    }
    grenade->clipmask = 0x2806891;
    grenade->handler = 7;
    grenade->missileTargetEnt.setEnt(NULL);
    G_BroadcastEntity(grenade);
    grenade->r.svFlags = 4;
    if (weapDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
        G_MakeMissilePickupItem(grenade);
    if (parent->client)
        grenade->missile.team = parent->client->sess.cs.team;
    else
        grenade->missile.team = TEAM_FREE;
}

void __cdecl G_InitGrenadeMovement(gentity_s *grenade, const float *start, const float *dir, int32_t rotate)
{
    double v4; // [esp+Ch] [ebp-4Ch]
    double v5; // [esp+18h] [ebp-40h]
    float angle; // [esp+20h] [ebp-38h]

    if (!grenade)
        MyAssertHandler(".\\game\\g_missile.cpp", 2548, 0, "%s", "grenade");
    grenade->mover.speed = 0.0;
    grenade->s.lerp.pos.trType = TR_GRAVITY;
    grenade->s.lerp.pos.trTime = level.time;
    grenade->r.currentOrigin[0] = *start;
    grenade->r.currentOrigin[1] = start[1];
    grenade->r.currentOrigin[2] = start[2];
    grenade->s.lerp.pos.trBase[0] = *start;
    grenade->s.lerp.pos.trBase[1] = start[1];
    grenade->s.lerp.pos.trBase[2] = start[2];
    grenade->s.lerp.pos.trDelta[0] = *dir;
    grenade->s.lerp.pos.trDelta[1] = dir[1];
    grenade->s.lerp.pos.trDelta[2] = dir[2];
    if ((COERCE_UNSIGNED_INT(grenade->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(grenade->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(grenade->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_missile.cpp",
            2558,
            0,
            "%s",
            "!IS_NAN((grenade->s.lerp.pos.trDelta)[0]) && !IS_NAN((grenade->s.lerp.pos.trDelta)[1]) && !IS_NAN((grenade->s.lerp"
            ".pos.trDelta)[2])");
    }
    vectoangles(dir, grenade->r.currentAngles);
    grenade->s.lerp.pos.trDelta[0] = (float)(int)grenade->s.lerp.pos.trDelta[0];
    grenade->s.lerp.pos.trDelta[1] = (float)(int)grenade->s.lerp.pos.trDelta[1];
    grenade->s.lerp.pos.trDelta[2] = (float)(int)grenade->s.lerp.pos.trDelta[2];
    if (rotate)
    {
        grenade->s.lerp.apos.trType = TR_LINEAR;
        grenade->s.lerp.apos.trTime = level.time;
        grenade->s.lerp.apos.trBase[0] = grenade->r.currentAngles[0];
        grenade->s.lerp.apos.trBase[1] = grenade->r.currentAngles[1];
        grenade->s.lerp.apos.trBase[2] = grenade->r.currentAngles[2];
        angle = grenade->s.lerp.apos.trBase[0] - 120.0f;
        grenade->s.lerp.apos.trBase[0] = AngleNormalize360(angle);
        v5 = G_flrand(320.0f, 800.0f);
        grenade->s.lerp.apos.trDelta[0] = (float)(2 * G_irand(0, 2) - 1) * v5;
        grenade->s.lerp.apos.trDelta[1] = 0.0;
        v4 = G_flrand(180.0f, 540.0f);
        grenade->s.lerp.apos.trDelta[2] = (float)(2 * G_irand(0, 2) - 1) * v4;
        grenade->r.currentAngles[0] = grenade->s.lerp.apos.trBase[0];
        grenade->r.currentAngles[1] = grenade->s.lerp.apos.trBase[1];
        grenade->r.currentAngles[2] = grenade->s.lerp.apos.trBase[2];
    }
    else
    {
        G_SetAngle(grenade, grenade->r.currentAngles);
    }
}

gentity_s *__cdecl G_FireGrenade(
    gentity_s *parent,
    float *start,
    float *dir,
    uint32_t grenadeWPID,
    uint8_t grenModel,
    int32_t rotate,
    int32_t time)
{
    char *Name; // eax
    float speed; // [esp+0h] [ebp-14h]
    gentity_s *grenade; // [esp+Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if (!parent)
        MyAssertHandler(".\\game\\g_missile.cpp", 2638, 0, "%s", "parent");
    weapDef = BG_GetWeaponDef(grenadeWPID);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 2641, 0, "%s", "weapDef");
    grenade = G_Spawn();
    Scr_SetString(&grenade->classname, scr_const.grenade);
    grenade->s.weapon = grenadeWPID;
    grenade->s.weaponModel = grenModel;
    G_InitGrenadeEntity(parent, grenade);
    if (rotate && weapDef->rotate)
        G_InitGrenadeMovement(grenade, start, dir, 1);
    else
        G_InitGrenadeMovement(grenade, start, dir, 0);
    if (parent->client)
    {
        speed = Vec3Length(dir);
        grenade->s.lerp.u.missile.launchTime += CalcMissileNoDrawTime(speed);
    }
    InitGrenadeTimer(parent, grenade, weapDef, time);
    if (grenade->model)
        MyAssertHandler(".\\game\\g_missile.cpp", 2662, 0, "%s", "!grenade->model");
    if (weapDef->projectileModel)
    {
        Name = (char *)XModelGetName(weapDef->projectileModel);
        G_SetModel(grenade, Name);
    }
    G_DObjUpdate(grenade);
    if (!weapDef->iProjectileActivateDist)
    {
        Scr_AddString((char *)weapDef->szInternalName);
        Scr_AddEntity(grenade);
        Scr_Notify(parent, scr_const.grenade_fire, 2u);
    }
    return grenade;
}

int32_t __cdecl CalcMissileNoDrawTime(float speed)
{
    int32_t v3; // [esp+4h] [ebp-8h]

    if ((int)(speed * -35.0f / 600.0f + 85.0f) < 50)
        v3 = (int)(speed * -35.0f / 600.0f + 85.0f);
    else
        v3 = 50;
    if (v3 > 20)
        return v3;
    else
        return 20;
}

void __cdecl InitGrenadeTimer(const gentity_s *parent, gentity_s *grenade, const WeaponDef *weapDef, int32_t time)
{
    if (!parent)
        MyAssertHandler(".\\game\\g_missile.cpp", 2590, 0, "%s", "parent");
    if (!grenade)
        MyAssertHandler(".\\game\\g_missile.cpp", 2591, 0, "%s", "grenade");
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 2592, 0, "%s", "weapDef");
    if (!weapDef->iProjectileActivateDist || time > 0)
    {
        if (!parent->client || weapDef->timedDetonation)
        {
            if (parent->client && parent->client->ps.grenadeTimeLeft)
            {
                grenade->nextthink = parent->client->ps.grenadeTimeLeft + level.time;
                parent->client->ps.grenadeTimeLeft = 0;
            }
            else
            {
                grenade->nextthink = time + level.time;
            }
        }
        else
        {
            parent->client->ps.grenadeTimeLeft = 0;
        }
    }
    if (grenade->handler != 7)
        MyAssertHandler(".\\game\\g_missile.cpp", 2615, 0, "%s", "grenade->handler == ENT_HANDLER_GRENADE");
    grenade->item[0].clipAmmoCount = level.time;
    if (!grenade->nextthink)
        grenade->nextthink = level.time + 30000;
    if (grenade->nextthink > level.time + 60000)
        grenade->nextthink = level.time + 60000;
}

float MYJAVELINOFFSET = 0.3f;
float MYJAVELINOFFSET_RIGHT = 10.0f;

gentity_s *__cdecl G_FireRocket(
    gentity_s *parent,
    uint32_t weaponIndex,
    float *start,
    float *dir,
    const float *gunVel,
    gentity_s *target,
    const float *targetOffset)
{
    int32_t v7; // eax
    int32_t v8; // ecx
    float iProjectileSpeed; // [esp+4h] [ebp-A8h]
    float speed; // [esp+8h] [ebp-A4h]
    float scale; // [esp+Ch] [ebp-A0h]
    float v13; // [esp+10h] [ebp-9Ch]
    float v14; // [esp+34h] [ebp-78h]
    float *currentOrigin; // [esp+38h] [ebp-74h]
    float *trBase; // [esp+48h] [ebp-64h]
    float *trDelta; // [esp+4Ch] [ebp-60h]
    float *v18; // [esp+50h] [ebp-5Ch]
    float *v19; // [esp+54h] [ebp-58h]
    float sinT; // [esp+70h] [ebp-3Ch]
    float theta; // [esp+74h] [ebp-38h]
    float r; // [esp+78h] [ebp-34h]
    float v[3]; // [esp+7Ch] [ebp-30h] BYREF
    float cosT; // [esp+88h] [ebp-24h]
    float up[3]; // [esp+8Ch] [ebp-20h] BYREF
    float right[3]; // [esp+98h] [ebp-14h] BYREF
    gentity_s *bolt; // [esp+A4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+A8h] [ebp-4h]

    if (!parent)
        MyAssertHandler(".\\game\\g_missile.cpp", 2700, 0, "%s", "parent");
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 2703, 0, "%s", "weapDef");
    Vec3Normalize(dir);
    if (weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN)
    {
        dir[2] = dir[2] + MYJAVELINOFFSET;
        Vec3Normalize(dir);
        AngleVectors(parent->s.lerp.apos.trBase, 0, right, 0);
        Vec3Mad(start, MYJAVELINOFFSET_RIGHT, right, start);
    }
    bolt = G_Spawn();
    Scr_SetString(&bolt->classname, scr_const.rocket);
    bolt->s.eType = 4;
    bolt->s.lerp.eFlags |= 0x400u;
    bolt->s.weapon = (uint8_t)weaponIndex;
    bolt->s.weaponModel = 0;
    bolt->s.lerp.u.missile.launchTime = level.time;
    if (parent->client)
    {
        speed = (float)weapDef->iProjectileSpeed;
        v7 = CalcMissileNoDrawTime(speed);
        bolt->s.lerp.u.missile.launchTime += v7;
    }
    bolt->r.ownerNum.setEnt(parent);
    bolt->parent.setEnt(parent);
    bolt->clipmask = 0x2806891;
    bolt->handler = 9;
    InitRocketTimer(bolt, weapDef);
    bolt->mover.speed = 0.0;
    bolt->missileTargetEnt.setEnt(target);
    if (targetOffset)
    {
        v19 = &bolt->mover.pos2[1];
        bolt->mover.pos2[1] = *targetOffset;
        v19[1] = targetOffset[1];
        v19[2] = targetOffset[2];
    }
    else
    {
        v18 = &bolt->mover.pos2[1];
        bolt->mover.pos2[1] = 0.0;
        v18[1] = 0.0;
        v18[2] = 0.0;
    }
    if (parent->client)
        bolt->missile.team = parent->client->sess.cs.team;
    else
        bolt->missile.team = TEAM_FREE;
    G_BroadcastEntity(bolt);
    bolt->s.lerp.pos.trType = TR_LINEAR;
    if (weapDef->timeToAccelerate <= 0.0)
    {
        iProjectileSpeed = (float)weapDef->iProjectileSpeed;
        Vec3Scale(dir, iProjectileSpeed, bolt->s.lerp.pos.trDelta);
    }
    else
    {
        bolt->s.lerp.pos.trType = TR_INTERPOLATE;
        trDelta = bolt->s.lerp.pos.trDelta;
        bolt->s.lerp.pos.trDelta[0] = 0.0;
        trDelta[1] = 0.0;
        trDelta[2] = 0.0;
    }
    bolt->s.lerp.pos.trTime = level.time;
    trBase = bolt->s.lerp.pos.trBase;
    bolt->s.lerp.pos.trBase[0] = *start;
    trBase[1] = start[1];
    trBase[2] = start[2];
    Vec3Add(bolt->s.lerp.pos.trDelta, gunVel, bolt->s.lerp.pos.trDelta);
    if ((COERCE_UNSIGNED_INT(bolt->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(bolt->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(bolt->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_missile.cpp",
            2770,
            0,
            "%s",
            "!IS_NAN((bolt->s.lerp.pos.trDelta)[0]) && !IS_NAN((bolt->s.lerp.pos.trDelta)[1]) && !IS_NAN((bolt->s.lerp.pos.trDelta)[2])");
    }
    bolt->s.lerp.pos.trDelta[0] = (float)(int)bolt->s.lerp.pos.trDelta[0];
    bolt->s.lerp.pos.trDelta[1] = (float)(int)bolt->s.lerp.pos.trDelta[1];
    bolt->s.lerp.pos.trDelta[2] = (float)(int)bolt->s.lerp.pos.trDelta[2];
    currentOrigin = bolt->r.currentOrigin;
    bolt->r.currentOrigin[0] = *start;
    currentOrigin[1] = start[1];
    currentOrigin[2] = start[2];
    vectoangles(dir, bolt->r.currentAngles);
    G_SetAngle(bolt, bolt->r.currentAngles);
    if (weapDef->projectileCurvature > 0.0)
    {
        bolt->s.lerp.pos.trType = TR_INTERPOLATE;
        AngleVectors(bolt->r.currentAngles, 0, v, up);
        theta = G_random() * 360.0;
        r = G_random() * weapDef->projectileCurvature;
        v14 = theta * 0.01745329238474369;
        cosT = cos(v14);
        sinT = sin(v14);
        v13 = r * cosT;
        Vec3Scale(v, v13, &bolt->mover.pos1[1]);
        scale = r * sinT;
        Vec3Mad(&bolt->mover.pos1[1], scale, up, &bolt->mover.pos1[1]);
        if ((COERCE_UNSIGNED_INT(bolt->mover.pos1[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(bolt->mover.pos1[2]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(bolt->mover.pos2[0]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\g_missile.cpp",
                2797,
                0,
                "%s",
                "!IS_NAN((bolt->missile.curvature)[0]) && !IS_NAN((bolt->missile.curvature)[1]) && !IS_NAN((bolt->missile.curvature)[2])");
        }
    }
    if (weapDef->guidedMissileType)
    {
        bolt->s.lerp.pos.trType = TR_INTERPOLATE;
        bolt->s.lerp.apos.trType = TR_INTERPOLATE;
        bolt->s.lerp.pos.trDuration = 0;
        if (weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN)
        {
            bolt->s.lerp.pos.trType = TR_GRAVITY;
            bolt->s.lerp.pos.trTime = level.time;
            bolt->missile.stage = MISSILESTAGE_SOFTLAUNCH;
            bolt->missile.flightMode = MISSILEFLIGHTMODE_DIRECT;
        }
    }
    if (!weapDef->iProjectileSpeed)
        MyAssertHandler(".\\game\\g_missile.cpp", 2827, 0, "%s", "weapDef->iProjectileSpeed");
    bolt->mover.decelTime = (double)weapDef->destabilizeDistance / (double)weapDef->iProjectileSpeed * 1000.0;
    if (weapDef->destabilizationRateTime == 0.0)
        v8 = bolt->flags | 0x20000;
    else
        v8 = bolt->flags | parent->flags & 0x20000;
    bolt->flags = v8;
    SV_LinkEntity(bolt);
    return bolt;
}

void __cdecl InitRocketTimer(gentity_s *bolt, WeaponDef *weapDef)
{
    if (!bolt)
        MyAssertHandler(".\\game\\g_missile.cpp", 2684, 0, "%s", "bolt");
    if (!weapDef)
        MyAssertHandler(".\\game\\g_missile.cpp", 2685, 0, "%s", "weapDef");
    bolt->nextthink = level.time + (int)(weapDef->projLifetime * 1000.0);
    if (bolt->nextthink > level.time + 60000)
        bolt->nextthink = level.time + 60000;
}

