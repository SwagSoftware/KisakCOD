#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"

#include <game_mp/g_utils_mp.h>

#include <script/scr_const.h>
#include <script/scr_vm.h>

#include <server/sv_game.h>
#include <server/sv_world.h>

void __cdecl G_Trigger(gentity_s *self, gentity_s *other)
{
    trigger_info_t *trigger_info; // [esp+0h] [ebp-4h]

    if (!self->r.inuse)
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 8, 0, "%s", "self->r.inuse");
    if (!other->r.inuse)
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 9, 0, "%s", "other->r.inuse");
    if (Scr_IsSystemActive())
    {
        if (level.pendingTriggerListSize == 256)
        {
            Scr_AddEntity(other);
            Scr_Notify(self, scr_const.trigger, 1u);
        }
        else
        {
            trigger_info = &level.pendingTriggerList[level.pendingTriggerListSize++];
            trigger_info->entnum = self->s.number;
            trigger_info->otherEntnum = other->s.number;
            trigger_info->useCount = self->useCount;
            trigger_info->otherUseCount = other->useCount;
        }
    }
}

char __cdecl InitTrigger(gentity_s *self)
{
    if (SV_SetBrushModel(self))
    {
        self->r.contents = 1079771144;
        self->r.svFlags = 1;
        self->s.lerp.eFlags |= 1u;
        if (!self->model)
            self->s.lerp.eFlags |= 0x20u;
        return 1;
    }
    else
    {
        Com_PrintError(
            1,
            "Killing trigger at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
        return 0;
    }
}

void __cdecl InitSentientTrigger(gentity_s *self)
{
    if (!self)
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 73, 0, "%s", "self");
    self->r.contents = 0;
    if ((self->spawnflags & 8) == 0)
        self->r.contents |= 0x40000000u;
    if ((self->spawnflags & 1) != 0)
        self->r.contents |= 0x40000u;
    if ((self->spawnflags & 2) != 0)
        self->r.contents |= 0x80000u;
    if ((self->spawnflags & 4) != 0)
        self->r.contents |= 0x100000u;
}

void __cdecl multi_trigger(gentity_s *ent)
{
    if ((ent->spawnflags & 0x10) != 0)
        G_FreeEntityDelay(ent);
}

void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int32_t extra)
{
    G_Trigger(self, other);
    multi_trigger(self);
}

void __cdecl SP_trigger_multiple(gentity_s *ent)
{
    ent->handler = ENT_HANDLER_TRIGGER_MULTIPLE;
    InitTriggerWait(ent, 16);
    if (InitTrigger(ent))
    {
        InitSentientTrigger(ent);
        SV_LinkEntity(ent);
    }
}

void __cdecl InitTriggerWait(gentity_s *ent, int32_t spawnflag)
{
    float wait; // [esp+0h] [ebp-4h] BYREF

    if (level.spawnVar.spawnVarsValid && G_SpawnFloat("wait", "", &wait) && wait <= 0.0)
        ent->spawnflags |= spawnflag;
}

void __cdecl SP_trigger_radius(gentity_s *ent)
{
    const char *v1; // eax
    const char *v2; // eax
    float height; // [esp+18h] [ebp-8h] BYREF
    float radius; // [esp+1Ch] [ebp-4h] BYREF

    if (level.spawnVar.spawnVarsValid)
    {
        if (!G_SpawnFloat("radius", "", &radius))
        {
            v1 = va(
                "radius not specified for trigger_radius at (%g %g %g)",
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2]);
            Com_Error(ERR_DROP, v1);
        }
        if (!G_SpawnFloat("height", "", &height))
        {
            v2 = va(
                "height not specified for trigger_radius at (%g %g %g)",
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2]);
            Com_Error(ERR_DROP, v2);
        }
    }
    else
    {
        if (Scr_GetNumParam() < 5)
            Scr_Error("USAGE: spawn( \"trigger_radius\", <origin>, <spawnflags>, <radius>, <height> )");
        radius = Scr_GetFloat(3);
        height = Scr_GetFloat(4);
    }
    ent->handler = ENT_HANDLER_TRIGGER_MULTIPLE;
    ent->r.mins[0] = -radius;
    ent->r.mins[1] = -radius;
    ent->r.mins[2] = 0.0;
    ent->r.maxs[0] = radius;
    ent->r.maxs[1] = radius;
    ent->r.maxs[2] = height;
    ent->r.svFlags = 33;
    InitTriggerWait(ent, 16);
    InitSentientTrigger(ent);
    SV_LinkEntity(ent);
}

void __cdecl SP_trigger_disk(gentity_s *ent)
{
    const char *v1; // eax
    float radius; // [esp+18h] [ebp-4h] BYREF

    if (!G_SpawnFloat("radius", "", &radius))
    {
        v1 = va(
            "radius not specified for trigger_radius at (%g %g %g)",
            ent->r.currentOrigin[0],
            ent->r.currentOrigin[1],
            ent->r.currentOrigin[2]);
        Com_Error(ERR_DROP, v1);
    }
    ent->handler = ENT_HANDLER_TRIGGER_MULTIPLE;
    radius = radius + 64.0;
    ent->r.mins[0] = -radius;
    ent->r.mins[1] = -radius;
    ent->r.mins[2] = -100000.0;
    ent->r.maxs[0] = radius;
    ent->r.maxs[1] = radius;
    ent->r.maxs[2] = 100000.0;
    ent->r.svFlags = 65;
    InitTriggerWait(ent, 16);
    InitSentientTrigger(ent);
    SV_LinkEntity(ent);
}

void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int32_t extra)
{
    if (other->takedamage && self->item[0].index <= level.time)
    {
        G_Trigger(self, other);
        self->item[0].index = (self->spawnflags & 0x10) != 0 ? level.time + 1000 : level.time + 50;
        G_Damage(other, self, self, 0, 0, self->damage, 0, 13, 0xFFFFFFFF, HITLOC_NONE, 0, 0, 0);
        if ((self->spawnflags & 0x20) != 0)
        {
            if (self->handler != 3)
                MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 236, 0, "%s", "self->handler == ENT_HANDLER_TRIGGER_HURT_TOUCH");
            self->handler = ENT_HANDLER_TRIGGER_HURT;
        }
    }
}

void __cdecl hurt_use(gentity_s *self, gentity_s *other, gentity_s *third)
{
    if (self->handler == ENT_HANDLER_TRIGGER_HURT_TOUCH)
    {
        self->handler = ENT_HANDLER_TRIGGER_HURT;
    }
    else
    {
        iassert(self->handler == ENT_HANDLER_TRIGGER_HURT);
        self->handler = ENT_HANDLER_TRIGGER_HURT_TOUCH;
    }
}

void __cdecl SP_trigger_hurt(gentity_s *self)
{
    const char *sound; // [esp+0h] [ebp-4h] BYREF

    if (InitTrigger(self))
    {
        G_LevelSpawnString("sound", "world_hurt_me", &sound);
        if (!self->damage)
            self->damage = 5;
        self->r.contents = 1079771144;
        if ((self->spawnflags & 1) != 0)
            self->handler = ENT_HANDLER_TRIGGER_HURT;
        else
            self->handler = ENT_HANDLER_TRIGGER_HURT_TOUCH;
    }
}

void __cdecl SP_trigger_once(gentity_s *ent)
{
    ent->handler = ENT_HANDLER_TRIGGER_MULTIPLE;
    ent->spawnflags |= 0x10u;
    if (InitTrigger(ent))
    {
        InitSentientTrigger(ent);
        SV_LinkEntity(ent);
    }
}

bool __cdecl Respond_trigger_damage(gentity_s *pEnt, int32_t iMOD)
{
    if ((pEnt->spawnflags & 1) != 0 && iMOD == 1)
        return 0;
    if ((pEnt->spawnflags & 2) != 0 && iMOD == 2)
        return 0;
    if ((pEnt->spawnflags & 4) != 0 && iMOD >= 3 && iMOD <= 6)
        return 0;
    if ((pEnt->spawnflags & 8) != 0 && iMOD >= 3 && (iMOD <= 6 || iMOD == 14))
        return 0;
    if ((pEnt->spawnflags & 0x10) != 0 && (iMOD == 4 || iMOD == 6))
        return 0;
    if ((pEnt->spawnflags & 0x20) != 0 && iMOD == 7)
        return 0;
    return (pEnt->spawnflags & 0x100) == 0 || iMOD && (iMOD <= 8 || iMOD > 13);
}

void __cdecl Activate_trigger_damage(gentity_s *pEnt, gentity_s *pOther, int32_t iDamage, int32_t iMOD)
{
    if ((pEnt->item[0].ammoCount <= 0 || iDamage >= pEnt->item[0].ammoCount)
        && Respond_trigger_damage(pEnt, iMOD)
        && (!pEnt->item[0].clipAmmoCount || 32000 - pEnt->health >= pEnt->item[0].clipAmmoCount))
    {
        if (iMOD != -1)
            G_Trigger(pEnt, pOther);
        pEnt->health = 32000;
        if ((pEnt->spawnflags & 0x200) != 0)
            G_FreeEntityDelay(pEnt);
    }
}

void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *third)
{
    Activate_trigger_damage(pEnt, pOther, pEnt->item[0].clipAmmoCount + 1, -1);
}

void __cdecl Pain_trigger_damage(gentity_s *pSelf, gentity_s *pAttacker, int32_t iDamage, const float *vPoint, int32_t iMod, const float* idk, hitLocation_t hit, int32_t swag)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    if (!pSelf->item[0].clipAmmoCount)
        pSelf->health = 32000;
}

void Die_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int32_t iDamage,
    int32_t iMod,
    int32_t iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc,
    int32_t timeOffset)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    if (!pSelf->item[0].clipAmmoCount)
        pSelf->health = 32000;
}

void __cdecl SP_trigger_damage(gentity_s *pSelf)
{
    G_SpawnInt("accumulate", "0", &pSelf->item[0].clipAmmoCount);
    //G_SpawnInt("threshold", "0", (int32_t *)&pSelf->436);
    G_SpawnInt("threshold", "0", &pSelf->item[0].ammoCount);
    pSelf->health = 32000;
    pSelf->takedamage = 1;
    pSelf->handler = ENT_HANDLER_TRIGGER_DAMAGE;
    InitTriggerWait(pSelf, 512);
    if (InitTrigger(pSelf))
        SV_LinkEntity(pSelf);
}

void __cdecl G_CheckHitTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int32_t iDamage, uint32_t  iMOD)
{
    int32_t v5; // [esp+Ch] [ebp-1034h]
    int32_t entityList[1025]; // [esp+10h] [ebp-1030h] BYREF
    float diff[3]; // [esp+1014h] [ebp-2Ch] BYREF
    float mins[3]; // [esp+1020h] [ebp-20h] BYREF
    int32_t i; // [esp+102Ch] [ebp-14h]
    gentity_s *pEnt; // [esp+1030h] [ebp-10h]
    float maxs[3]; // [esp+1034h] [ebp-Ch] BYREF

    if (iMOD >= 0x10)
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 505, 0, "iMOD doesn't index MOD_NUM\n\t%i not in [0, %i)", iMOD, 16);
    if (!*modNames[iMOD])
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 506, 0, "%s", "*modNames[iMOD]");
    if (!vStart)
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 507, 0, "%s", "vStart");
    if (!vEnd)
        MyAssertHandler(".\\game_mp\\g_trigger_mp.cpp", 508, 0, "%s", "vEnd");
    mins[0] = *vStart;
    mins[1] = vStart[1];
    mins[2] = vStart[2];
    maxs[0] = *vStart;
    maxs[1] = vStart[1];
    maxs[2] = vStart[2];
    AddPointToBounds(vEnd, mins, maxs);
    Vec3Sub(vEnd, vStart, diff);
    Vec3Normalize(diff);
    v5 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x400000);
    for (i = 0; i < v5; ++i)
    {
        pEnt = &g_entities[entityList[i]];
        if (pEnt->classname == scr_const.trigger_damage
            && SV_SightTraceToEntity(vStart, (float *)vec3_origin, (float *)vec3_origin, vEnd, pEnt->s.number, -1))
        {
            Scr_AddConstString(*modNames[iMOD]);
            Scr_AddVector((float *)vec3_origin);
            Scr_AddVector(diff);
            Scr_AddEntity(pActivator);
            Scr_AddInt(iDamage);
            Scr_Notify(pEnt, scr_const.damage, 5u);
            Activate_trigger_damage(pEnt, pActivator, iDamage, iMOD);
            if (!pEnt->item[0].clipAmmoCount)
                pEnt->health = 32000;
        }
    }
}

void __cdecl G_GrenadeTouchTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int32_t iDamage, int32_t iMOD)
{
    int32_t v5; // [esp+Ch] [ebp-1034h]
    int32_t entityList[1025]; // [esp+10h] [ebp-1030h] BYREF
    float diff[3]; // [esp+1014h] [ebp-2Ch] BYREF
    float mins[3]; // [esp+1020h] [ebp-20h] BYREF
    int32_t i; // [esp+102Ch] [ebp-14h]
    gentity_s *pEnt; // [esp+1030h] [ebp-10h]
    float maxs[3]; // [esp+1034h] [ebp-Ch] BYREF

    mins[0] = *vStart;
    mins[1] = vStart[1];
    mins[2] = vStart[2];
    maxs[0] = *vStart;
    maxs[1] = vStart[1];
    maxs[2] = vStart[2];
    AddPointToBounds(vEnd, mins, maxs);
    Vec3Sub(vEnd, vStart, diff);
    Vec3Normalize(diff);
    v5 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x400000);
    for (i = 0; i < v5; ++i)
    {
        pEnt = &g_entities[entityList[i]];
        if (pEnt->classname == scr_const.trigger_damage
            && (pEnt->flags & 0x4000) != 0
            && SV_SightTraceToEntity(vStart, (float *)vec3_origin, (float *)vec3_origin, vEnd, pEnt->s.number, -1))
        {
            Scr_AddConstString(*modNames[iMOD]);
            Scr_AddVector((float *)vec3_origin);
            Scr_AddVector(diff);
            Scr_AddEntity(pActivator);
            Scr_AddInt(iDamage);
            Scr_Notify(pEnt, scr_const.damage, 5u);
            Activate_trigger_damage(pEnt, pActivator, iDamage, iMOD);
            if (!pEnt->item[0].clipAmmoCount)
                pEnt->health = 32000;
        }
    }
}

void __cdecl SP_trigger_lookat(gentity_s *self)
{
    if (SV_SetBrushModel(self))
    {
        self->r.contents = 0x20000000;
        self->r.svFlags = 1;
        self->s.lerp.eFlags |= 1u;
        if (!self->model)
            self->s.lerp.eFlags |= 0x20u;
    }
    else
    {
        Com_PrintError(
            1,
            "Killing trigger_lookat at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
    }
}

