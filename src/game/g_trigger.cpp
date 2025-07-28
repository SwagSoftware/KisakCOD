#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"

void __cdecl G_Trigger(gentity_s *self, gentity_s *other)
{
    int pendingTriggerListSize; // r11
    trigger_info_t *v5; // r11

    if (!self->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 10, 0, "%s", "self->r.inuse");
    if (!other->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 11, 0, "%s", "other->r.inuse");
    if (Scr_IsSystemActive(1u))
    {
        pendingTriggerListSize = level.pendingTriggerListSize;
        if (level.pendingTriggerListSize == 256)
        {
            Scr_AddEntity(other);
            Scr_Notify(self, scr_const.trigger, 1u);
        }
        else
        {
            ++level.pendingTriggerListSize;
            v5 = &level.pendingTriggerList[pendingTriggerListSize];
            v5->entnum = self->s.number;
            v5->otherEntnum = other->s.number;
            v5->useCount = self->s.lerp.useCount;
            v5->otherUseCount = other->s.lerp.useCount;
        }
    }
}

int __cdecl InitTrigger(gentity_s *self)
{
    int model; // r8
    int v4; // r11

    if (SV_SetBrushModel(self))
    {
        model = self->model;
        v4 = self->s.lerp.eFlags | 1;
        self->r.svFlags = 1;
        self->r.contents = 1079771144;
        self->s.lerp.eFlags = v4;
        if (!model)
            self->s.lerp.eFlags = v4 | 0x20;
        return 1;
    }
    else
    {
        Com_PrintError(
            1,
            (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[0])),
            (unsigned int)COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[0]),
            (unsigned int)COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[1]),
            (unsigned int)COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[2]));
        G_FreeEntity(self);
        return 0;
    }
}

void __cdecl InitTriggerWait(gentity_s *ent, int spawnflag)
{
    float v4[2]; // [sp+50h] [-20h] BYREF

    if (level.spawnVar.spawnVarsValid && G_SpawnFloat("wait", "", v4) && v4[0] <= 0.0)
        ent->spawnflags |= spawnflag;
}

void __cdecl InitSentientTrigger(gentity_s *self)
{
    int spawnflags; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 77, 0, "%s", "self");
    spawnflags = self->spawnflags;
    self->r.contents = 0;
    if ((spawnflags & 8) == 0)
        self->r.contents = 0x40000000;
    if ((spawnflags & 1) != 0)
        self->r.contents |= 0x40000u;
    if ((spawnflags & 2) != 0)
        self->r.contents |= 0x80000u;
    if ((spawnflags & 4) != 0)
        self->r.contents |= 0x100000u;
    if ((spawnflags & 0x10) != 0)
        self->r.contents |= 8u;
}

void __cdecl multi_trigger(gentity_s *ent, gentity_s *activator)
{
    if ((ent->spawnflags & 0x40) != 0)
        G_FreeEntityDelay(ent);
}

void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int bTouched)
{
    G_Trigger(self, other);
    if ((self->spawnflags & 0x40) != 0)
        G_FreeEntityDelay(self);
}

void __cdecl SP_trigger_multiple(gentity_s *ent)
{
    ent->handler = 4;
    InitTriggerWait(ent, 64);
    if ((unsigned __int8)InitTrigger(ent))
    {
        InitSentientTrigger(ent);
        SV_LinkEntity(ent);
    }
}

void __cdecl SP_trigger_radius(gentity_s *ent)
{
    const char *v2; // r3
    const char *v3; // r3
    double v4; // fp0
    double v5; // fp13
    double v6; // fp0
    float Float; // [sp+50h] [-20h] BYREF
    float v8; // [sp+54h] [-1Ch] BYREF

    if (level.spawnVar.spawnVarsValid)
    {
        if (!G_SpawnFloat("radius", "", &Float))
        {
            v2 = va(
                (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(ent->r.currentOrigin[0])),
                (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[0]),
                (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[1]),
                (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[2]));
            Com_Error(ERR_DROP, v2);
        }
        if (!G_SpawnFloat("height", "", &v8))
        {
            v3 = va(
                (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(ent->r.currentOrigin[0])),
                (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[0]),
                (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[1]),
                (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[2]));
            Com_Error(ERR_DROP, v3);
        }
    }
    else
    {
        if (Scr_GetNumParam() < 5)
            Scr_Error("USAGE: spawn( \"trigger_radius\", <origin>, <spawnflags>, <radius>, <height> )");
        Float = Scr_GetFloat(3u);
        v8 = Scr_GetFloat(4u);
    }
    v4 = Float;
    v5 = -Float;
    ent->r.maxs[0] = Float;
    ent->r.maxs[1] = v4;
    v6 = v8;
    ent->r.mins[2] = 0.0;
    ent->handler = 4;
    ent->r.maxs[2] = v6;
    ent->r.svFlags = 17;
    ent->r.mins[0] = v5;
    ent->r.mins[1] = v5;
    InitTriggerWait(ent, 64);
    InitSentientTrigger(ent);
    SV_LinkEntity(ent);
}

void __cdecl SP_trigger_disk(gentity_s *ent)
{
    const char *v2; // r3
    double v3; // fp0
    float v4[4]; // [sp+50h] [-20h] BYREF

    if (!G_SpawnFloat("radius", "", v4))
    {
        v2 = va(
            (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(ent->r.currentOrigin[0])),
            (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[0]),
            (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[1]),
            (unsigned int)COERCE_UNSIGNED_INT64(ent->r.currentOrigin[2]));
        Com_Error(ERR_DROP, v2);
    }
    v3 = (float)(v4[0] + (float)64.0);
    ent->handler = 4;
    v4[0] = v3;
    ent->r.svFlags = 33;
    ent->r.maxs[0] = v3;
    ent->r.maxs[1] = v3;
    ent->r.mins[2] = -100000.0;
    ent->r.maxs[2] = 100000.0;
    ent->r.mins[0] = -v3;
    ent->r.mins[1] = -v3;
    InitTriggerWait(ent, 64);
    InitSentientTrigger(ent);
    SV_LinkEntity(ent);
}

void __cdecl Touch_FriendlyChain(gentity_s *self, gentity_s *other, int bTouched)
{
    if (!other->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 239, 0, "%s", "other->sentient");
    Path_AttachSentientToChainNode(other->sentient, self->target);
    G_Trigger(self, other);
}

void __cdecl SP_trigger_friendlychain(gentity_s *ent)
{
    int target; // r10

    target = ent->target;
    ent->handler = 5;
    if (!target)
        Com_Error(ERR_DROP, byte_8203F0F8);
    if ((unsigned __int8)InitTrigger(ent))
    {
        InitSentientTrigger(ent);
        SV_LinkEntity(ent);
    }
}

void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int bTouched)
{
    int v5; // r11
    int v6; // [sp+8h] [-88h]
    hitLocation_t v7; // [sp+Ch] [-84h]
    unsigned int v8; // [sp+10h] [-80h]
    unsigned int v9; // [sp+14h] [-7Ch]
    int v10; // [sp+18h] [-78h]
    int v11; // [sp+1Ch] [-74h]
    int v12; // [sp+20h] [-70h]
    int v13; // [sp+24h] [-6Ch]
    int v14; // [sp+28h] [-68h]
    int v15; // [sp+2Ch] [-64h]
    int v16; // [sp+30h] [-60h]
    int v17; // [sp+34h] [-5Ch]
    int v18; // [sp+38h] [-58h]
    int v19; // [sp+3Ch] [-54h]
    int v20; // [sp+40h] [-50h]
    int v21; // [sp+44h] [-4Ch]
    int v22; // [sp+48h] [-48h]
    int v23; // [sp+4Ch] [-44h]
    int v24; // [sp+50h] [-40h]
    int v25; // [sp+58h] [-38h]
    int v26; // [sp+60h] [-30h]
    int v27; // [sp+68h] [-28h]

    if (other->takedamage && ((self->spawnflags & 2) == 0 || !other->actor) && self->item[0].index <= level.time)
    {
        G_Trigger(self, other);
        v5 = (self->spawnflags & 0x10) != 0 ? level.time + 1000 : level.time + 50;
        self->item[0].index = v5;
        G_Damage(
            other,
            self,
            self,
            0,
            0,
            self->damage,
            0,
            13,
            v6,
            v7,
            v8,
            v9,
            v10,
            v11,
            v12,
            v13,
            v14,
            v15,
            v16,
            v17,
            v18,
            v19,
            v20,
            v21,
            v22,
            v23,
            v24,
            0xFFFFFFFF,
            v25,
            0,
            v26,
            0,
            v27,
            0);
        if ((self->spawnflags & 0x20) != 0)
        {
            if (self->handler != 7)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp",
                    319,
                    0,
                    "%s",
                    "self->handler == ENT_HANDLER_TRIGGER_HURT_TOUCH");
            self->handler = 6;
        }
    }
}

void __cdecl hurt_use(gentity_s *self, gentity_s *other, gentity_s *activator)
{
    int handler; // r11
    unsigned __int8 v5; // r11

    handler = self->handler;
    if (handler == 7)
    {
        v5 = 6;
    }
    else
    {
        if (handler != 6)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp",
                333,
                0,
                "%s",
                "self->handler == ENT_HANDLER_TRIGGER_HURT");
        v5 = 7;
    }
    self->handler = v5;
}

void __cdecl SP_trigger_hurt(gentity_s *self)
{
    bool v2; // cr58
    unsigned __int8 v3; // r11

    if ((unsigned __int8)InitTrigger(self))
    {
        if (!self->damage)
            self->damage = 5;
        v2 = (self->spawnflags & 1) == 0;
        v3 = 7;
        self->r.contents = 1079771144;
        if (!v2)
            v3 = 6;
        self->handler = v3;
    }
}

void __cdecl SP_trigger_once(gentity_s *ent)
{
    int spawnflags; // r11

    spawnflags = ent->spawnflags;
    ent->handler = 4;
    ent->spawnflags = spawnflags | 0x40;
    if ((unsigned __int8)InitTrigger(ent))
    {
        InitSentientTrigger(ent);
        SV_LinkEntity(ent);
    }
}

bool __cdecl Respond_trigger_damage(gentity_s *trigger, int damageType)
{
    int spawnflags; // r11

    if (!trigger)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 434, 0, "%s", "trigger");
    spawnflags = trigger->spawnflags;
    return ((spawnflags & 1) == 0 || damageType != 1)
        && ((spawnflags & 2) == 0 || damageType != 2)
        && ((spawnflags & 4) == 0 || damageType < 3 || damageType > 6)
        && ((spawnflags & 8) == 0 || damageType != 14)
        && ((spawnflags & 0x10) == 0 || damageType != 4 && damageType != 6)
        && ((spawnflags & 0x20) == 0 || damageType != 7)
        && ((spawnflags & 0x100) == 0 || damageType && (damageType <= 8 || damageType > 13));
}

void __cdecl Activate_trigger_damage(gentity_s *pEnt, gentity_s *pOther, int iDamage, int iMOD)
{
    int ammoCount; // r11
    int clipAmmoCount; // r11
    int v9; // r11

    ammoCount = pEnt->item[0].ammoCount;
    if (ammoCount <= 0 || iDamage >= ammoCount)
    {
        if (Respond_trigger_damage(pEnt, iMOD))
        {
            clipAmmoCount = pEnt->item[0].clipAmmoCount;
            if (!clipAmmoCount || 32000 - pEnt->health >= clipAmmoCount)
            {
                if (iMOD != -1)
                    G_Trigger(pEnt, pOther);
                v9 = pEnt->spawnflags & 0x200;
                pEnt->health = 32000;
                if (v9)
                    G_FreeEntityDelay(pEnt);
            }
        }
    }
}

void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *pActivator)
{
    int ammoCount; // r11
    int clipAmmoCount; // r11
    int v5; // r11

    ammoCount = pEnt->item[0].ammoCount;
    if (ammoCount <= 0 || pEnt->item[0].clipAmmoCount + 1 >= ammoCount)
    {
        clipAmmoCount = pEnt->item[0].clipAmmoCount;
        if (!clipAmmoCount || 32000 - pEnt->health >= clipAmmoCount)
        {
            v5 = pEnt->spawnflags & 0x200;
            pEnt->health = 32000;
            if (v5)
                G_FreeEntityDelay(pEnt);
        }
    }
}

void __cdecl Pain_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    const float *vDir,
    const hitLocation_t hitLoc,
    const int weaponIdx)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    if (!pSelf->item[0].clipAmmoCount)
        pSelf->health = 32000;
}

void __cdecl Die_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    int iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    if (!pSelf->item[0].clipAmmoCount)
        pSelf->health = 32000;
}

void __cdecl SP_trigger_damage(gentity_s *pSelf)
{
    G_SpawnInt("accumulate", "0", &pSelf->item[0].clipAmmoCount);
    G_SpawnInt("threshold", "0", (int *)&pSelf->352);
    pSelf->health = 32000;
    pSelf->takedamage = 1;
    pSelf->handler = 8;
    InitTriggerWait(pSelf, 512);
    if ((unsigned __int8)InitTrigger(pSelf))
        SV_LinkEntity(pSelf);
}

void __cdecl G_CheckHitTriggerDamage(
    gentity_s *pActivator,
    const float *vStart,
    const float *vEnd,
    int iDamage,
    unsigned int iMOD)
{
    double v10; // fp0
    double v11; // fp13
    double v12; // fp12
    double v13; // fp0
    double v14; // fp13
    double v17; // fp11
    double v18; // fp12
    int v19; // r3
    int *v20; // r27
    int v21; // r20
    gentity_s *v22; // r31
    float v23[4]; // [sp+50h] [-22B0h] BYREF
    float v24[4]; // [sp+60h] [-22A0h] BYREF
    float v25[4]; // [sp+70h] [-2290h] BYREF
    int v26[160]; // [sp+80h] [-2280h] BYREF

    if (iMOD >= 0x10)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp",
            600,
            0,
            "iMOD doesn't index MOD_NUM\n\t%i not in [0, %i)",
            iMOD,
            16);
    if (!*modNames[iMOD])
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 601, 0, "%s", "*modNames[iMOD]");
    if (!vStart)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 602, 0, "%s", "vStart");
    if (!vEnd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_trigger.cpp", 603, 0, "%s", "vEnd");
    v10 = *vStart;
    v11 = vStart[1];
    v12 = vStart[2];
    v23[1] = vStart[1];
    v23[0] = v10;
    v23[2] = v12;
    v24[0] = v10;
    v24[1] = v11;
    v24[2] = v12;
    AddPointToBounds(vEnd, v23, v24);
    v13 = (float)(vEnd[1] - vStart[1]);
    v14 = (float)(vEnd[2] - vStart[2]);
    _FP9 = -sqrtf((float)((float)((float)(*vEnd - *vStart) * (float)(*vEnd - *vStart))
        + (float)((float)((float)v14 * (float)v14) + (float)((float)v13 * (float)v13))));
    __asm { fsel      f11, f9, f10, f11 }
    v17 = (float)((float)1.0 / (float)_FP11);
    v18 = (float)((float)v17 * (float)(*vEnd - *vStart));
    v25[1] = (float)(vEnd[1] - vStart[1]) * (float)v17;
    v25[0] = v18;
    v25[2] = (float)v14 * (float)v17;
    v19 = CM_AreaEntities(v23, v24, v26, 2176, 0x400000);
    if (v19 > 0)
    {
        v20 = v26;
        v21 = v19;
        do
        {
            v22 = &g_entities[*v20];
            if (v22->classname == scr_const.trigger_damage
                && SV_SightTraceToEntity(vStart, vec3_origin, vec3_origin, vEnd, v22->s.number, -1))
            {
                Scr_AddConstString(*modNames[iMOD]);
                Scr_AddVector(vec3_origin);
                Scr_AddVector(v25);
                Scr_AddEntity(pActivator);
                Scr_AddInt(iDamage);
                Scr_Notify(v22, scr_const.damage, 5u);
                Activate_trigger_damage(v22, pActivator, iDamage, iMOD);
                if (!v22->item[0].clipAmmoCount)
                    v22->health = 32000;
            }
            --v21;
            ++v20;
        } while (v21);
    }
}

void __cdecl G_GrenadeTouchTriggerDamage(
    gentity_s *pActivator,
    const float *vStart,
    const float *vEnd,
    int iDamage,
    int iMOD)
{
    double v8; // fp0
    double v9; // fp13
    double v11; // fp12
    double v13; // fp0
    double v14; // fp13
    double v17; // fp11
    double v18; // fp12
    int v19; // r3
    int *v20; // r21
    int v21; // r20
    gentity_s *v22; // r31
    float v23[4]; // [sp+50h] [-22A0h] BYREF
    float v24[4]; // [sp+60h] [-2290h] BYREF
    float v25[4]; // [sp+70h] [-2280h] BYREF
    int v26[156]; // [sp+80h] [-2270h] BYREF

    v8 = *vStart;
    v9 = vStart[1];
    v11 = vStart[2];
    v23[0] = *vStart;
    v23[1] = v9;
    v23[2] = v11;
    v24[0] = v8;
    v24[1] = v9;
    v24[2] = v11;
    AddPointToBounds(vEnd, v23, v24);
    v13 = (float)(vEnd[1] - vStart[1]);
    v14 = (float)(vEnd[2] - vStart[2]);
    _FP9 = -sqrtf((float)((float)((float)(*vEnd - *vStart) * (float)(*vEnd - *vStart))
        + (float)((float)((float)v14 * (float)v14) + (float)((float)v13 * (float)v13))));
    __asm { fsel      f11, f9, f10, f11 }
    v17 = (float)((float)1.0 / (float)_FP11);
    v18 = (float)((float)v17 * (float)(*vEnd - *vStart));
    v25[1] = (float)(vEnd[1] - vStart[1]) * (float)v17;
    v25[0] = v18;
    v25[2] = (float)v14 * (float)v17;
    v19 = CM_AreaEntities(v23, v24, v26, 2176, 0x400000);
    if (v19 > 0)
    {
        v20 = v26;
        v21 = v19;
        do
        {
            v22 = &g_entities[*v20];
            if (v22->classname == scr_const.trigger_damage
                && (v22->flags & 0x4000) != 0
                && SV_SightTraceToEntity(vStart, vec3_origin, vec3_origin, vEnd, v22->s.number, -1))
            {
                Scr_AddConstString(*modNames[iMOD]);
                Scr_AddVector(vec3_origin);
                Scr_AddVector(v25);
                Scr_AddEntity(pActivator);
                Scr_AddInt(iDamage);
                Scr_Notify(v22, scr_const.damage, 5u);
                Activate_trigger_damage(v22, pActivator, iDamage, iMOD);
                if (!v22->item[0].clipAmmoCount)
                    v22->health = 32000;
            }
            --v21;
            ++v20;
        } while (v21);
    }
}

void __cdecl SP_trigger_lookat(gentity_s *self)
{
    int model; // r8
    int v3; // r11

    if (SV_SetBrushModel(self))
    {
        model = self->model;
        v3 = self->s.lerp.eFlags | 1;
        self->r.contents = 0x20000000;
        self->r.svFlags = 1;
        self->s.lerp.eFlags = v3;
        if (!model)
            self->s.lerp.eFlags = v3 | 0x20;
        SV_LinkEntity(self);
    }
    else
    {
        Com_PrintError(
            1,
            (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[0])),
            (unsigned int)COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[0]),
            (unsigned int)COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[1]),
            (unsigned int)COERCE_UNSIGNED_INT64(self->s.lerp.pos.trBase[2]));
        G_FreeEntity(self);
    }
}

