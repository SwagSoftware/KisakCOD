#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include <script/scr_vm.h>
#include <script/scr_const.h>
#include "g_main.h"
#include <server/sv_public.h>
#include "actor_senses.h"
#include "actor_events.h"

unsigned char *bulletPriorityMap;
unsigned char *riflePriorityMap;
float g_fHitLocDamageMult[19]{ 0.0f };

unsigned __int16 *modNames[16] =
{
    &scr_const.mod_unknown,
    &scr_const.mod_pistol_bullet,
    &scr_const.mod_rifle_bullet,
    &scr_const.mod_grenade,
    &scr_const.mod_grenade_splash,
    &scr_const.mod_projectile,
    &scr_const.mod_projectile_splash,
    &scr_const.mod_melee,
    &scr_const.mod_head_shot,
    &scr_const.mod_crush,
    &scr_const.mod_telefrag,
    &scr_const.mod_falling,
    &scr_const.mod_suicide,
    &scr_const.mod_trigger_hurt,
    &scr_const.mod_explosive,
    &scr_const.mod_impact,
};

const char *g_HitLocNames[19] =
{
  "none",
  "helmet",
  "head",
  "neck",
  "torso_upper",
  "torso_lower",
  "right_arm_upper",
  "left_arm_upper",
  "right_arm_lower",
  "left_arm_lower",
  "right_hand",
  "left_hand",
  "right_leg_upper",
  "left_leg_upper",
  "right_leg_lower",
  "left_leg_lower",
  "right_foot",
  "left_foot",
  "gun"
};

unsigned __int16 g_HitLocConstNames[19]{ 0 };

void __cdecl TRACK_g_combat()
{
    track_static_alloc_internal(g_fHitLocDamageMult, 76, "g_fHitLocDamageMult", 9);
    track_static_alloc_internal(g_HitLocNames, 76, "g_HitLocNames", 9);
    track_static_alloc_internal(g_HitLocConstNames, 38, "g_HitLocConstNames", 9);
}

void __cdecl G_HitLocStrcpy(unsigned __int8 *pMember, const char *pszKeyValue)
{
    int v2; // r10
    int v3; // r11

    v2 = pMember - (unsigned __int8 *)pszKeyValue;
    do
    {
        v3 = *(unsigned __int8 *)pszKeyValue;
        ((char*)pszKeyValue++)[v2] = v3;
    } while (v3);
}

void __cdecl G_ParseHitLocDmgTable()
{
    int v0; // r30
    unsigned __int16 *v1; // r29
    int *p_iOffset; // r31
    const char *v3; // r3
    const char *InfoString; // r3
    cspField_t v5[20]; // [sp+50h] [-2140h] BYREF
    char v6[80]; // [sp+140h] [-2050h] BYREF

    v0 = 0;
    v1 = g_HitLocConstNames;
    p_iOffset = &v5[0].iOffset;
    do
    {
        v3 = g_HitLocNames[v0];
        g_fHitLocDamageMult[v0] = 1.0;
        *p_iOffset = v0 * 4;
        p_iOffset[1] = 6;
        *(p_iOffset - 1) = (int)v3;
        *v1++ = Scr_AllocString((char*)v3, 1);
        p_iOffset += 3;
        ++v0;
    } while ((uintptr_t)v1 < (uintptr_t)&g_HitLocConstNames[19]);

    g_fHitLocDamageMult[18] = 0.0;
    InfoString = Com_LoadInfoString((char*)"info/ai_lochit_dmgtable", "hitloc damage table", "LOCDMGTABLE", v6);
    if (!ParseConfigStringToStruct((unsigned __int8 *)g_fHitLocDamageMult, v5, 19, (char*)InfoString, 0, 0, G_HitLocStrcpy))
        Com_Error(ERR_DROP, "Error parsing hitloc damage table %s", "info/ai_lochit_dmgtable");
}

void __cdecl TossClientItems(gentity_s *self)
{
    gclient_s *client; // r30
    unsigned int weapon; // r31
    int weaponstate; // r11
    gclient_s *v5; // r30
    int v6; // r5
    const gitem_s *ItemForWeapon; // r4
    gentity_s *v8; // r3

    client = self->client;
    weapon = self->s.weapon;
    weaponstate = client->ps.weaponstate;
    if (weaponstate == 3 || weaponstate == 4)
        weapon = client->pers.cmd.weapon;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, weapon, 16))
    {
        if (weapon)
        {
            if (weapon < BG_GetNumWeapons())
            {
                v5 = self->client;
                if (v5->ps.ammo[BG_AmmoForWeapon(weapon)])
                {
                    ItemForWeapon = BG_FindItemForWeapon(weapon, v5->ps.weaponmodels[weapon]);
                    if ((self->client->ps.eFlags & 0x300) == 0)
                    {
                        v8 = Drop_Item(self, ItemForWeapon, 0.0, v6);
                        if (v8)
                            v8->nextthink = 0;
                    }
                }
            }
        }
    }
}

void __cdecl LookAtKiller(gentity_s *self, gentity_s *inflictor, gentity_s *attacker)
{
    double v3; // fp0
    double v4; // fp12
    double v5; // fp13
    float v6[4]; // [sp+50h] [-20h] BYREF

    if (attacker && attacker != self)
    {
        v3 = (float)(attacker->r.currentOrigin[0] - self->r.currentOrigin[0]);
        v4 = attacker->r.currentOrigin[1];
        v5 = attacker->r.currentOrigin[2];
    LABEL_7:
        v6[0] = v3;
        v6[1] = (float)v4 - self->r.currentOrigin[1];
        v6[2] = (float)v5 - self->r.currentOrigin[2];
        self->client->ps.stats[1] = (int)vectoyaw(v6);
        vectoyaw(v6);
        return;
    }
    if (inflictor && inflictor != self)
    {
        v3 = (float)(inflictor->r.currentOrigin[0] - self->r.currentOrigin[0]);
        v4 = inflictor->r.currentOrigin[1];
        v5 = inflictor->r.currentOrigin[2];
        goto LABEL_7;
    }
    self->client->ps.stats[1] = (int)self->r.currentAngles[1];
}

int __cdecl G_MeansOfDeathFromScriptParam(unsigned int scrParam)
{
    unsigned __int16 ConstString; // r3
    int v3; // r10
    unsigned __int16 **v4; // r11

    ConstString = Scr_GetConstString(scrParam);
    v3 = 0;
    v4 = modNames;
    while (**v4 != ConstString)
    {
        ++v4;
        ++v3;
        //if ((int)v4 >= (int)WeaponStateNames_110)
        if ((int)v4 >= ARRAY_COUNT(WeaponStateNames)) // KISAKTODO: double check
        {
            Scr_ParamError(scrParam, va("Unknown means of death \"%s\"\n", SL_ConvertToString(ConstString)));
            return 0;
        }
    }
    return v3;
}

void use_trigger_use(gentity_s *ent, gentity_s *other, gentity_s *activator)
{
    ;
}

void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    const int damage,
    const int meansOfDeath,
    const int iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc)
{
    gclient_s *client; // r11
    double v15; // fp1
    gclient_s *v16; // r11
    int offHandIndex; // r6
    WeaponDef *WeaponDef; // r3
    const char **p_szInternalName; // r24
    int v20; // r27
    int number; // r26
    gclient_s *v22; // r11
    gclient_s *v23; // r11
    double v24; // fp13
    const char *v25; // r29
    gclient_s *v26; // r30
    const char *v27; // r3
    const char *v28; // r3
    float v29; // [sp+50h] [-70h] BYREF
    float v30; // [sp+54h] [-6Ch]
    float v31; // [sp+58h] [-68h]
    float v32[24]; // [sp+60h] [-60h] BYREF

    client = self->client;
    if (client->ps.pm_type < 5)
    {
        if (client->ps.grenadeTimeLeft)
        {
            v29 = G_crandom();
            v30 = G_crandom();
            v15 = G_crandom();
            v16 = self->client;
            v29 = v29 * (float)160.0;
            v30 = v30 * (float)160.0;
            v31 = (float)v15 * (float)160.0;
            v32[0] = self->r.currentOrigin[0];
            v32[2] = self->r.currentOrigin[2] + (float)40.0;
            v32[1] = self->r.currentOrigin[1];
            if ((v16->ps.weapFlags & 2) != 0)
                offHandIndex = v16->ps.offHandIndex;
            else
                offHandIndex = v16->ps.weapon;
            G_FireGrenade(self, v32, &v29, offHandIndex, v16->ps.weaponmodels[offHandIndex], 1, v16->ps.grenadeTimeLeft);
        }
        WeaponDef = BG_GetWeaponDef(iWeapon);
        p_szInternalName = &WeaponDef->szInternalName;
        if (iWeapon)
            Scr_AddString(WeaponDef->szInternalName);
        else
            Scr_AddUndefined();
        v20 = meansOfDeath;
        Scr_AddConstString(*modNames[meansOfDeath]);
        Scr_AddEntity(attacker);
        Scr_Notify(self, scr_const.death, 3u);
        self->client->ps.pm_type = (pmtype_t)((self->client->ps.pm_type == 1) + 5);
        if (!attacker || (number = attacker->s.number) != 0)
            number = ENTITYNUM_WORLD;
        self->sentient->lastAttacker = attacker;
        v22 = self->client;
        self->takedamage = 1;
        self->r.contents = 0x4000000;
        self->s.weapon = 0;
        if (v22->ps.pm_type != 6)
        {
            self->r.currentAngles[0] = 0.0;
            self->r.currentAngles[2] = 0.0;
            LookAtKiller(self, inflictor, attacker);
            v23 = self->client;
            v23->ps.viewangles[0] = self->r.currentAngles[0];
            v23->ps.viewangles[1] = self->r.currentAngles[1];
            v23->ps.viewangles[2] = self->r.currentAngles[2];
        }
        v24 = self->r.mins[2];
        self->s.loopSound = 0;
        self->r.maxs[2] = 16.0;
        if (v24 > 16.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                358,
                0,
                "%s",
                "self->r.maxs[2] >= self->r.mins[2]");
        self->client->respawnTime = level.time + 1700;
        if (self->handler != 17)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                364,
                0,
                "%s",
                "self->handler == ENT_HANDLER_CLIENT");
        self->handler = ENT_HANDLER_CLIENT_DEAD;
        SV_TrackPlayerDied();
        if (p_szInternalName)
            v25 = *p_szInternalName;
        else
            v25 = "NONE";
        v26 = self->client;
        v27 = SL_ConvertToStringSafe(*modNames[v20]);
        v28 = va(
            "died at %.1f %.1f %.1f from %s (weapon:%s) - hitloc %i, killer was %i",
            v26->ps.origin[0],
            v26->ps.origin[1],
            v26->ps.origin[2],
            v27,
            v25,
            hitLoc,
            number);
        //LSP_LogString(cl_controller_in_use, v28);
        //LSP_SendLogRequest(cl_controller_in_use);
        SV_LinkEntity(self);
    }
}

float __cdecl G_GetWeaponHitLocationMultiplier(unsigned int hitLoc, unsigned int weapon)
{
    WeaponDef *WeaponDef; // r3
    double v5; // fp1

    if (hitLoc > 0x12)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
            389,
            0,
            "%s",
            "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)");
    if (weapon && (WeaponDef = BG_GetWeaponDef(weapon)) != 0 && WeaponDef->weapType == WEAPTYPE_BULLET)
        v5 = WeaponDef->locationDamageMultipliers[hitLoc];
    else
        v5 = g_fHitLocDamageMult[hitLoc];
    return *((float *)&v5 + 1);
}

void __cdecl handleDeathInvulnerability(gentity_s *targ, int prevHealth, int mod)
{
    gclient_s *client; // r28
    int health; // r11
    char v8; // r10
    char v9; // r9
    bool v10; // r11
    bool v11; // r10
    bool v12; // r11
    bool *p_invulnerableActivated; // r10

    if (!targ)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 417, 0, "%s", "targ");
    if (targ->health > prevHealth)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 418, 0, "%s", "targ->health <= prevHealth");
    client = targ->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 422, 0, "%s", "client");
    if (!client->invulnerableEnabled)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 423, 0, "%s", "client->invulnerableEnabled");
    if (prevHealth == client->ps.stats[2])
        client->invulnerableActivated = 0;
    health = targ->health;
    if (prevHealth != health && health <= 0 && prevHealth > 0)
    {
        if (mod == 1 || mod == 2 || (v8 = 0, mod == 8))
            v8 = 1;
        v9 = v8;
        v10 = player_deathInvulnerableToProjectile->current.enabled && (mod == 5 || mod == 6);
        v11 = v10;
        v12 = player_deathInvulnerableToMelee->current.enabled && mod == 7;
        if (v9 || v11 || v12)
        {
            p_invulnerableActivated = &client->invulnerableActivated;
            if (!client->invulnerableActivated)
            {
                *p_invulnerableActivated = 1;
                client->invulnerableExpireTime = player_deathInvulnerableTime->current.integer + level.time;
            }
            if (*p_invulnerableActivated && level.time < client->invulnerableExpireTime)
                targ->health = 1;
        }
    }
}

void __cdecl G_DamageNotify(
    unsigned __int16 notify,
    gentity_s *targ,
    gentity_s *attacker,
    const float *dir,
    const float *point,
    int damage,
    int mod,
    unsigned int modelIndex,
    unsigned int partName)
{
    unsigned int v36; // r27
    unsigned int modelName; // r31
    const float *v38; // r3
    const float *v39; // r3

    if (partName)
        Scr_AddConstString(partName);
    else
        Scr_AddString("");

    if (modelIndex)
    {
        v36 = 2 * (modelIndex + 272);
        if (!*(_WORD *)(&targ->s.eType + v36))
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                466,
                0,
                "%s",
                "targ->attachTagNames[modelIndex - 1]");
        modelName = SV_GetConfigstringConst(*((unsigned __int16 *)&targ->scripted + modelIndex + 1) + 1155);
        if (!modelName)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 469, 1, "%s", "modelName");
        Scr_AddConstString(*(unsigned __int16 *)(&targ->s.eType + v36));
        Scr_AddConstString(modelName);
    }
    else
    {
        Scr_AddString("");
        Scr_AddString("");
    }
    Scr_AddConstString(*modNames[mod]);
    v38 = point;
    if (!point)
        v38 = vec3_origin;
    Scr_AddVector(v38);
    v39 = dir;
    if (!dir)
        v39 = vec3_origin;
    Scr_AddVector(v39);
    Scr_AddEntity(attacker);
    Scr_AddInt(damage);
    Scr_Notify(targ, notify, 8u);
}

int __cdecl G_GetWeaponIndexForEntity(const gentity_s *ent)
{
    gclient_s *client; // r28
    unsigned int viewlocked_entNum; // r7
    int result; // r3

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 501, 0, "%s", "ent");
    client = ent->client;
    if (client)
    {
        if ((client->ps.eFlags & 0x20300) != 0)
        {
            if (client->ps.viewlocked_entNum == ENTITYNUM_NONE)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                    509,
                    0,
                    "%s",
                    "client->ps.viewlocked_entNum != ENTITYNUM_NONE");
            viewlocked_entNum = client->ps.viewlocked_entNum;
            if (viewlocked_entNum >= 0x880)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                    510,
                    0,
                    "client->ps.viewlocked_entNum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    viewlocked_entNum,
                    2176);
            return 0;
        }
        else
        {
            return BG_GetViewmodelWeaponIndex(&ent->client->ps);
        }
    }
    else
    {
        result = 0;
        if (!ent->pTurretInfo)
            return ent->s.weapon;
    }
    return result;
}

void __cdecl G_DamageKnockback(
    gentity_s *targ,
    const gentity_s *attacker,
    const float *dir,
    float *scaledDir,
    __int64 dflags,
    int mod)
{
    double v10; // fp31
    unsigned int v11; // r28
    char v12; // r27
    gclient_s *client; // r11
    int pm_flags; // r10
    int v16; // r9
    double v17; // fp13
    double v18; // fp12
    double v19; // fp11
    double v20; // fp12
    double v21; // fp0
    int v22; // r10
    int v23; // r9
    __int64 v24; // [sp+50h] [-50h] BYREF

    v10 = 0.050000001;
    v11 = HIDWORD(dflags);
    v12 = dflags;
    if (!targ)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 534, 0, "%s", "targ");
    if (dir)
        Vec3NormalizeTo(dir, scaledDir);
    else
        v12 |= 4u;
    client = targ->client;
    if (client)
    {
        pm_flags = client->ps.pm_flags;
        if ((pm_flags & 1) != 0)
        {
            v10 = 0.0099999998;
        }
        else if ((pm_flags & 2) != 0)
        {
            v10 = 0.025;
        }
    }
    LODWORD(v24) = v11;
    HIDWORD(v24) = (int)(float)((float)__SPAIR64__(&v24, v11) * (float)v10);
    v16 = HIDWORD(v24);
    if (SHIDWORD(v24) > 60)
        v16 = 60;
    if ((targ->flags & 0x20) != 0)
        v16 = 0;
    if ((v12 & 4) == 0 && v16 && client && (client->ps.eFlags & 0x300) == 0)
    {
        LODWORD(dflags) = v16;
        v17 = *scaledDir;
        v18 = scaledDir[1];
        v19 = scaledDir[2];
        v24 = dflags;
        v20 = (float)((float)v18 * (float)((float)(g_knockback->current.value * (float)dflags) * (float)0.0040000002));
        v21 = (float)((float)v19 * (float)((float)(g_knockback->current.value * (float)dflags) * (float)0.0040000002));
        client->ps.velocity[0] = client->ps.velocity[0]
            + (float)((float)v17
                * (float)((float)(g_knockback->current.value * (float)dflags) * (float)0.0040000002));
        client->ps.velocity[1] = client->ps.velocity[1] + (float)v20;
        client->ps.velocity[2] = client->ps.velocity[2] + (float)v21;
        if (targ == attacker && (mod == 5 || mod == 6 || mod == 3 || mod == 4))
            client->ps.velocity[2] = client->ps.velocity[2] * 0.25;
        if (!client->ps.pm_time)
        {
            v22 = 2 * v16;
            if (2 * v16 < 200)
            {
                if (v22 <= 50)
                    v22 = 50;
            }
            else
            {
                v22 = 200;
            }
            v23 = client->ps.pm_flags;
            client->ps.pm_time = v22;
            client->ps.pm_flags = v23 | 0x100;
        }
    }
}

bool __cdecl G_ShouldTakeBulletDamage(gentity_s *targ, gentity_s *attacker)
{
    sentient_s *sentient; // r11
    actor_s *actor; // r11

    iassert(targ);
    iassert(attacker);
    sentient = targ->sentient;
    if (!sentient || !sentient->ignoreRandomBulletDamage || !attacker->actor)
        return 1;
    iassert(attacker->sentient);
    if (attacker->sentient->targetEnt.isDefined() && attacker->sentient->targetEnt.ent() == targ)
        return 1;
    actor = targ->actor;
    return actor && actor->eState[actor->stateLevel] == AIS_SCRIPTEDANIM
        || attacker->actor->eState[attacker->actor->stateLevel] == AIS_SCRIPTEDANIM;
}

void __cdecl G_Damage(
    gentity_s *targ,
    gentity_s *inflictor,
    gentity_s *attacker,
    const float *dir,
    const float *point,
    int damage,
    unsigned int dflags,
    int mod,
    int weapon,
    hitLocation_t hitLoc,
    unsigned int modelIndex,
    unsigned int partName)
{
    unsigned int WeaponIndexForEntity; // r19
    const gentity_s *v43; // r3
    unsigned int NumWeapons; // r3
    gclient_s *client; // r24
    __int64 v46; // r10
    const dvar_s *dmgDvar; // r11
    __int64 v48; // fp13
    unsigned int v49; // r20
    double WeaponHitLocationMultiplier; // fp1
    actor_s *actor; // r3
    int v54; // r3
    int v55; // r30
    int health; // r11
    int v57; // r4
    int v58; // r11
    WeaponDef *WeaponDef; // r3
    sentient_s *sentient; // r11
    int v61; // r3
    sentient_s *v62; // r11
    void(__cdecl * die)(gentity_s *, gentity_s *, gentity_s *, const int, const int, const int, const float *, const hitLocation_t); // r30
    int v64; // r3
    void(__cdecl * pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int); // r11
    gclient_s *v66; // r11
    unsigned int v67; // [sp+8h] [-F8h]
    int v68; // [sp+Ch] [-F4h]
    int v69; // [sp+10h] [-F0h]
    int v70; // [sp+14h] [-ECh]
    int v71; // [sp+18h] [-E8h]
    int v72; // [sp+1Ch] [-E4h]
    int v73; // [sp+20h] [-E0h]
    int v74; // [sp+24h] [-DCh]
    int v75; // [sp+28h] [-D8h]
    int v76; // [sp+2Ch] [-D4h]
    int v77; // [sp+30h] [-D0h]
    int v78; // [sp+34h] [-CCh]
    int v79; // [sp+38h] [-C8h]
    int v80; // [sp+3Ch] [-C4h]
    int v81; // [sp+40h] [-C0h]
    int v82; // [sp+44h] [-BCh]
    int v83; // [sp+48h] [-B8h]
    int v84; // [sp+4Ch] [-B4h]
    int v85; // [sp+50h] [-B0h]
    float v86; // [sp+68h] [-98h] BYREF
    float v87; // [sp+6Ch] [-94h]
    float v88; // [sp+70h] [-90h]

    damage = damage;

    iassert(targ);

    if (targ->takedamage)
    {
        if (!inflictor)
            inflictor = &g_entities[ENTITYNUM_WORLD];
        if (!attacker)
            attacker = &g_entities[ENTITYNUM_WORLD];

        iassert(targ->r.inuse);
        iassert(attacker->r.inuse);
        
        WeaponIndexForEntity = weapon;

        if (weapon == -1)
        {
            v43 = inflictor;
            if (!inflictor)
                v43 = attacker;
            WeaponIndexForEntity = G_GetWeaponIndexForEntity(v43);
        }
        if (WeaponIndexForEntity >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                692,
                0,
                "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                WeaponIndexForEntity,
                NumWeapons);
        }
        if (!targ->scr_vehicle || !G_IsVehicleImmune(targ, mod, dflags, WeaponIndexForEntity))
        {
            client = targ->client;
            if (client)
            {
                if (G_IsPlayerDrivingVehicle(targ)
                    || (client->ps.otherFlags & 1) != 0
                    || client->noclip
                    || client->ufo
                    || level.mpviewer)
                {
                    return;
                }
                if (mod != 11)
                {
                    LODWORD(v46) = damage;
                    if ((dflags & 1) != 0)
                    {
                        dmgDvar = player_radiusDamageMultiplier;
                        v48 = v46;
                    }
                    else
                    {
                        if (mod == 7)
                            dmgDvar = player_meleeDamageMultiplier;
                        else
                            dmgDvar = player_damageMultiplier;
                        v48 = v46;
                    }
                    //a22 = (int)(float)(dmgDvar->current.value * (float)v48);
                    //damage = a22;
                    damage = (int)(float)(dmgDvar->current.value * (float)v48);
                }
                if (damage <= 0)
                {
                    damage = 1;
                    //a22 = 1;
                }
            }
            if (!G_ShouldTakeBulletDamage(targ, attacker))
                return;
            v49 = hitLoc;
            if (targ->actor && mod != 7)
            {
                if (hitLoc > 0x12)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                        744,
                        0,
                        "%s",
                        "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)");
                WeaponHitLocationMultiplier = G_GetWeaponHitLocationMultiplier(v49, WeaponIndexForEntity);
                if (WeaponHitLocationMultiplier == 0.0)
                    return;
                //a22 = (int)(float)((float)__SPAIR64__(&a22, damage) * (float)WeaponHitLocationMultiplier);
                damage = damage * WeaponHitLocationMultiplier;
                if (damage <= 0)
                    damage = 1;
            }
            v86 = 0.0;
            v87 = 0.0;
            v88 = 0.0;
            G_DamageKnockback(targ, attacker, dir, &v86, __SPAIR64__(damage, dflags), mod);
            if ((targ->flags & 1) == 0)
            {
                if (damage < 1)
                    damage = 1;
                actor = targ->actor;
                //v52 = a34;
                //v53 = a32;
                if (actor)
                {
                    v54 = Actor_CheckDeathAllowed(actor, damage);
                    v55 = v54;
                    damage -= v54;
                    if (g_debugDamage->current.enabled)
                        Com_Printf(15, "client:%i health:%i damage:%i armor:%i\n", targ->s.number, targ->health, damage, v54);
                    if (v55)
                        G_DamageNotify(
                            scr_const.damage_notdone,
                            targ,
                            attacker,
                            dir,
                            point,
                            damage,
                            mod,
                            modelIndex,
                            partName);
                }
                if (client)
                {
                    client->damage_blood += damage;
                    if (dir)
                    {
                        client->damage_from[0] = v86;
                        client->damage_from[1] = v87;
                        client->damage_from[2] = v88;
                        client->damage_fromWorld = 0;
                    }
                    else
                    {
                        client->damage_from[0] = targ->r.currentOrigin[0];
                        client->damage_from[1] = targ->r.currentOrigin[1];
                        client->damage_from[2] = targ->r.currentOrigin[2];
                        client->damage_fromWorld = 1;
                    }
                }
                if (damage)
                {
                    if ((targ->flags & 2) != 0)
                    {
                        health = targ->health;
                        if (health - damage <= 0)
                            damage = health - 1;
                    }
                    v57 = targ->health;
                    targ->health = v57 - damage;
                    if (client && client->invulnerableEnabled)
                        handleDeathInvulnerability(targ, v57, mod);
                    G_DamageNotify(
                        scr_const.damage,
                        targ,
                        attacker,
                        dir,
                        point,
                        damage,
                        mod,
                        modelIndex,
                        partName
                    );
                    v58 = targ->health;
                    if (v58 > 0)
                    {
                        pain = entityHandlers[targ->handler].pain;
                        if (pain)
                            pain(targ, attacker, damage, point, mod, &v86, (const hitLocation_t)v49, WeaponIndexForEntity);
                        goto LABEL_85;
                    }
                    if (client)
                        targ->flags |= 0x20u;
                    if (v58 < -999)
                        targ->health = -999;
                    if (!client)
                    {
                        if (WeaponIndexForEntity)
                        {
                            WeaponDef = BG_GetWeaponDef(WeaponIndexForEntity);
                            Scr_AddString(WeaponDef->szInternalName);
                        }
                        else
                        {
                            Scr_AddUndefined();
                        }
                        Scr_AddConstString(*modNames[mod]);
                        Scr_AddEntity(attacker);
                        Scr_Notify(targ, scr_const.death, 3u);
                        if (attacker->client)
                        {
                            sentient = targ->sentient;
                            if (sentient)
                            {
                                v61 = 1;
                                if (sentient->eTeam != TEAM_AXIS)
                                    v61 = -1;
                                SV_AddToPlayerScore(v61);
                            }
                        }
                    }
                    v62 = targ->sentient;
                    if (v62)
                        v62->lastAttacker = attacker;
                    die = entityHandlers[targ->handler].die;
                    if (die)
                    {
                        v64 = G_GetWeaponIndexForEntity(inflictor);
                        die(targ, inflictor, attacker, damage, mod, v64, &v86, (const hitLocation_t)v49);
                    }
                    if (targ->r.inuse)
                    {
                    LABEL_85:
                        v66 = targ->client;
                        if (v66)
                        {
                            if (targ->health < 0)
                                targ->health = 0;
                            v66->ps.stats[0] = targ->health;
                        }
                    }
                }
            }
        }
    }
}

int __cdecl G_CanRadiusDamageFromPos(
    gentity_s *targ,
    const float *targetPos,
    gentity_s *inflictor,
    const float *centerPos,
    double radius,
    double coneAngleCos,
    const float *coneDirection,
    double maxHeight,
    bool useEyeOffset,
    int contentMask)
{
    double v15; // fp27
    double v17; // fp13
    double v18; // fp12
    double v19; // fp25
    int number; // r21
    sentient_s *sentient; // r3
    double v22; // fp13
    double v25; // fp12
    double v26; // fp30
    double v27; // fp29
    double v28; // fp28
    double v29; // fp0
    double v30; // fp13
    double v31; // fp12
    double v32; // fp10
    double v33; // fp11
    float *v34; // r30
    int v35; // r28
    char inCone; // r11
    const float *v37; // r29
    double v38; // fp13
    double v39; // fp12
    int v42; // r29
    float *i; // r30
    double v44; // fp13
    double v45; // fp12
    const DObj_s *ServerDObj; // r3
    double v50; // fp27
    double v51; // fp26
    double v52; // fp25
    double v53; // fp13
    double v56; // fp11
    double v57; // fp12
    double v58; // fp0
    double v59; // fp12
    double v62; // fp13
    double v63; // fp29
    double v64; // fp30
    double v65; // fp28
    double v66; // fp8
    double v67; // fp7
    double v68; // fp8
    double v69; // fp12
    double v70; // fp9
    double v71; // fp0
    double v72; // fp7
    double v73; // fp5
    double v74; // fp10
    double v75; // fp0
    double v76; // fp9
    double v77; // fp13
    double v78; // fp12
    float *v79; // r30
    int v80; // r28
    char v81; // r11
    const float *v82; // r29
    double v83; // fp0
    double v84; // fp13
    int v87; // r29
    float *j; // r30
    double v89; // fp0
    double v90; // fp13
    float v93; // [sp+50h] [-140h] BYREF
    float v94; // [sp+54h] [-13Ch]
    float v95; // [sp+58h] [-138h]
    float v96; // [sp+5Ch] [-134h]
    float v97; // [sp+60h] [-130h]
    float v98; // [sp+64h] [-12Ch]
    float v99; // [sp+68h] [-128h]
    float v100; // [sp+6Ch] [-124h]
    float v101; // [sp+70h] [-120h]
    float v102; // [sp+74h] [-11Ch]
    float v103; // [sp+78h] [-118h]
    float v104; // [sp+7Ch] [-114h]
    float v105; // [sp+80h] [-110h]
    float v106; // [sp+84h] [-10Ch]
    float v107; // [sp+88h] [-108h]
    float v108[4]; // [sp+90h] [-100h] BYREF
    float v109[2]; // [sp+A0h] [-F0h] BYREF
    float v110; // [sp+A8h] [-E8h]
    float v111[4]; // [sp+B0h] [-E0h] BYREF
    float v112[4]; // [sp+C0h] [-D0h] BYREF
    float v113; // [sp+D0h] [-C0h] BYREF
    float v114; // [sp+D4h] [-BCh]
    float v115; // [sp+D8h] [-B8h]
    float v116[20]; // [sp+E0h] [-B0h] BYREF
    int v118; // [sp+1ECh] [+5Ch]

    v15 = 15.0;
    if ((targ->r.contents & 0x405C0008) == 0)
    {
        v17 = (float)(centerPos[2] - targetPos[2]);
        v18 = (float)(centerPos[1] - targetPos[1]);
        if ((float)((float)((float)v18 * (float)v18)
            + (float)((float)((float)(*centerPos - *targetPos) * (float)(*centerPos - *targetPos))
                + (float)((float)v17 * (float)v17))) >= (double)(float)((float)radius * (float)radius))
            return 0;
    }
    v19 = targ->r.currentOrigin[2];
    if (inflictor)
        number = inflictor->s.number;
    else
        number = ENTITYNUM_NONE;
    sentient = targ->sentient;
    if (sentient)
    {
        if (targ->client)
            v15 = 8.0;
        //v22 = (float)(centerPos[1] - targetPos[1]);
        //_FP10 = -sqrtf((float)((float)((float)(*centerPos - *targetPos) * (float)(*centerPos - *targetPos)) + (float)((float)v22 * (float)v22)));
        //__asm { fsel      f12, f10, f31, f12 }
        //v25 = (float)((float)1.0 / (float)_FP12);

        float dx = centerPos[0] - targetPos[0];
        float dy = centerPos[1] - targetPos[1];
        float dist2D = sqrtf(dx * dx + dy * dy);

        v25 = (1.0f / dist2D);

        v26 = (float)((float)v25 * (float)(*centerPos - *targetPos));
        v27 = (float)((float)v25 * (float)0.0);
        v28 = -(float)((float)v25 * (float)(centerPos[1] - targetPos[1]));
        if (maxHeight == 0.0)
        {
            Sentient_GetEyePosition(sentient, v109);
            v29 = 0.5;
            v30 = (float)((float)(v110 - (float)v19) * (float)0.5);
        }
        else
        {
            iassert(!useEyeOffset);
            v29 = 0.5;
            v30 = (float)((float)maxHeight * (float)0.5);
        }
        v31 = *targetPos;
        if (useEyeOffset)
        {
            v31 = (float)((float)(v109[0] + *targetPos) * (float)v29);
            v32 = (float)((float)(targetPos[1] + v109[1]) * (float)v29);
            v33 = (float)((float)(targetPos[2] + v110) * (float)v29);
        }
        else
        {
            v32 = targetPos[1];
            v33 = (float)(targetPos[2] + (float)v30);
        }
        v95 = v33;
        v96 = (float)((float)v28 * (float)v15) + (float)v31;
        v99 = v96;
        v93 = v31;
        v94 = v32;
        v97 = (float)((float)v26 * (float)v15) + (float)v32;
        v100 = v97;
        v98 = (float)((float)((float)v27 * (float)v15) + (float)v33) + (float)v30;
        v101 = (float)((float)((float)v27 * (float)v15) + (float)v33) - (float)v30;
        v102 = (float)((float)-v15 * (float)v28) + (float)v31;
        v103 = (float)((float)-v15 * (float)v26) + (float)v32;
        v105 = v102;
        v106 = v103;
        v104 = (float)((float)((float)-v15 * (float)v27) + (float)v33) + (float)v30;
        v107 = (float)((float)((float)-v15 * (float)v27) + (float)v33) - (float)v30;
        //if (radius_damage_debug->current.enabled)
        //{
        //    v34 = &v93;
        //    v35 = 5;
        //    do
        //    {
        //        inCone = 1;
        //        v37 = colorWhite;
        //        if (coneAngleCos != -1.0)
        //        {
        //            if (contentMask)
        //            {
        //                v38 = (float)(v34[2] - centerPos[2]);
        //                v39 = (float)(v34[1] - centerPos[1]);
        //                _FP7 = -sqrtf((float)((float)((float)v39 * (float)v39)
        //                    + (float)((float)((float)(*v34 - *centerPos) * (float)(*v34 - *centerPos))
        //                        + (float)((float)v38 * (float)v38))));
        //                __asm { fsel      f11, f7, f31, f11 }
        //                if ((float)((float)(*(float *)contentMask
        //                    * (float)((float)((float)1.0 / (float)_FP11) * (float)(*v34 - *centerPos)))
        //                    + (float)((float)(*(float *)(contentMask + 8)
        //                        * (float)((float)(v34[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
        //                        + (float)(*(float *)(contentMask + 4)
        //                            * (float)((float)(v34[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) < coneAngleCos)
        //                {
        //                    inCone = 0;
        //                    v37 = colorOrange;
        //                }
        //            }
        //        }
        //        if (inCone && !G_LocationalTracePassed(centerPos, v34, targ->s.number, number, v118, 0))
        //            v37 = colorRed;
        //        G_DebugLineWithDuration(centerPos, v34, v37, 1, 200);
        //        --v35;
        //        v34 += 3;
        //    } while (v35);
        //}

        if (radius_damage_debug->current.enabled)
        {
            float *sample = &v93;
            int sampleCount = 5;

            for (int i = 0; i < sampleCount; ++i, sample += 3)
            {
                bool inCone = true;
                const float *color = colorWhite;

                if (coneAngleCos != -1.0f && contentMask)
                {
                    float dx = sample[0] - centerPos[0];
                    float dy = sample[1] - centerPos[1];
                    float dz = sample[2] - centerPos[2];

                    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
                    if (dist > 0.0001f)
                    {
                        const float *coneDir = coneDirection;

                        float dot = (dx * coneDir[0] + dy * coneDir[1] + dz * coneDir[2]) / dist;
                        if (dot < coneAngleCos)
                        {
                            inCone = false;
                            color = colorOrange;
                        }
                    }
                }

                if (inCone && !G_LocationalTracePassed(centerPos, sample, targ->s.number, number, v118, 0))
                {
                    color = colorRed;
                }

                G_DebugLineWithDuration(centerPos, sample, color, 1, 200);
            }
        }

        //v42 = 0;
        //for (i = &v93; ; i += 3)
        //{
        //    if (coneAngleCos == -1.0)
        //        goto LABEL_58;
        //    if (!contentMask)
        //        goto LABEL_58;
        //    v44 = (float)(i[2] - centerPos[2]);
        //    v45 = (float)(i[1] - centerPos[1]);
        //    _FP7 = -sqrtf((float)((float)((float)v45 * (float)v45)
        //        + (float)((float)((float)(*i - *centerPos) * (float)(*i - *centerPos))
        //            + (float)((float)v44 * (float)v44))));
        //    __asm { fsel      f11, f7, f31, f11 }
        //    if ((float)((float)(*(float *)contentMask * (float)((float)((float)1.0 / (float)_FP11) * (float)(*i - *centerPos)))
        //        + (float)((float)(*(float *)(contentMask + 8)
        //            * (float)((float)(i[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
        //            + (float)(*(float *)(contentMask + 4)
        //                * (float)((float)(i[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) >= coneAngleCos)
        //    {
        //    LABEL_58:
        //        if (G_LocationalTracePassed(centerPos, i, targ->s.number, number, v118, 0))
        //            break;
        //    }
        //    if (++v42 >= 5)
        //        return 0;
        //}

        int passed = 0;
        for (int idx = 0; idx < 5; ++idx) {
            float *point = &v93 + idx * 3;

            bool withinCone = true;

            if (coneAngleCos != -1.0f && contentMask) {
                float dx = point[0] - centerPos[0];
                float dy = point[1] - centerPos[1];
                float dz = point[2] - centerPos[2];

                float len = sqrtf(dx * dx + dy * dy + dz * dz);

                if (len > 0.0001f) {
                    float invLen = 1.0f / len;

                    float dot = ((float *)contentMask)[0] * dx * invLen +
                        ((float *)contentMask)[1] * dy * invLen +
                        ((float *)contentMask)[2] * dz * invLen;

                    if (dot < coneAngleCos)
                        withinCone = false;
                }
                else {
                    withinCone = false;
                }
            }

            if (withinCone && G_LocationalTracePassed(centerPos, point, targ->s.number, number, v118, 0)) {
                passed = 1;
                break;
            }
        }

        if (!passed)
            return 0;
    }
    else
    {
        if (targ->classname == scr_const.script_model && targ->model)
        {
            ServerDObj = Com_GetServerDObj(targ->s.number);
            DObjPhysicsGetBounds(ServerDObj, v116, v108);
            G_EntityCentroidWithBounds(targ, v116, v108, &v93);
            v50 = (float)(v108[0] + targ->r.currentOrigin[0]);
            v51 = (float)(targ->r.currentOrigin[1] + v108[1]);
            v52 = (float)(targ->r.currentOrigin[2] + v108[2]);
        }
        else
        {
            G_EntityCentroid(targ, &v93);
            v50 = targ->r.absmax[0];
            v51 = targ->r.absmax[1];
            v52 = targ->r.absmax[2];
        }
        v53 = (float)(centerPos[2] - v95);

        //_FP10 = -sqrtf((float)((float)((float)(centerPos[1] - v94) * (float)(centerPos[1] - v94))
        //    + (float)((float)((float)(centerPos[2] - v95) * (float)(centerPos[2] - v95))
        //        + (float)((float)(*centerPos - v93) * (float)(*centerPos - v93)))));
        //__asm { fsel      f11, f10, f31, f11 }
        //v56 = (float)((float)1.0 / (float)_FP11);


        {
            float dx = centerPos[0] - v93;
            float dy = centerPos[1] - v94;
            float dz = centerPos[2] - v95;

            float len = sqrtf(dx * dx + dy * dy + dz * dz);
            v56 = (1.0f / len);
        }


        v57 = (float)((float)v56 * (float)(centerPos[1] - v94));
        v58 = (float)((float)v56 * (float)(*centerPos - v93));
        v112[1] = (float)v56 * (float)(centerPos[1] - v94);
        v112[0] = v58;
        v112[2] = (float)v53 * (float)v56;
        v59 = -v57;

        //_FP11 = -sqrtf((float)((float)((float)v58 * (float)v58) + (float)((float)v59 * (float)v59)));
        //__asm { fsel      f13, f11, f31, f13 }
        //v62 = (float)((float)1.0 / (float)_FP13);

        v62 = 1.0f / sqrtf((v58 * v58) + (v59 * v59));


        v63 = (float)((float)v62 * (float)v58);
        v64 = (float)((float)v62 * (float)v59);
        v111[1] = (float)v62 * (float)v58;
        v65 = (float)((float)v62 * (float)0.0);
        v111[0] = (float)v62 * (float)v59;
        v111[2] = (float)v62 * (float)0.0;
        Vec3Cross(v112, v111, &v113);
        v66 = I_fabs((float)((float)((float)v50 - v93) * (float)v64));
        v67 = I_fabs((float)((float)((float)v51 - v94) * (float)v63));
        v69 = (float)((float)((float)fabs((float)(v114 * (float)((float)v51 - v94)))
            + (float)fabs((float)(v113 * (float)((float)v50 - v93))))
            + (float)fabs((float)((float)((float)v52 - v95) * v115)));
        v70 = (float)((float)((float)v66 + (float)v67) * (float)v64);
        v71 = (float)((float)((float)v66 + (float)v67) * (float)v65);
        v68 = (float)((float)((float)v66 + (float)v67) * (float)v63);
        v72 = (float)(v93 + (float)v70);
        v73 = (float)((float)v71 + v95);
        v74 = (float)(v95 - (float)v71);
        v75 = (float)(v113 * (float)v69);
        v77 = (float)(v114 * (float)v69);
        v96 = (float)(v93 + (float)v70) + (float)(v113 * (float)v69);
        v76 = (float)(v93 - (float)v70);
        v97 = (float)(v94 + (float)v68) + (float)(v114 * (float)v69);
        v78 = (float)(v115 * (float)v69);
        v98 = (float)v78 + (float)v73;
        v99 = (float)v76 + (float)v75;
        v100 = (float)(v94 - (float)v68) + (float)v77;
        v101 = (float)v78 + (float)v74;
        v102 = (float)v72 - (float)v75;
        v105 = (float)v76 - (float)v75;
        v103 = (float)(v94 + (float)v68) - (float)v77;
        v106 = (float)(v94 - (float)v68) - (float)v77;
        v104 = (float)v73 - (float)v78;
        v107 = (float)v74 - (float)v78;
        if (radius_damage_debug->current.enabled)
        {
            v79 = &v93;
            v80 = 5;
            do
            {
                v81 = 1;
                v82 = colorWhite;
                if (coneAngleCos != -1.0)
                {
                    if (contentMask)
                    {
                        //v83 = (float)(v79[1] - centerPos[1]);
                        //v84 = (float)(v79[2] - centerPos[2]);
                        //_FP7 = -sqrtf((float)((float)((float)(*v79 - *centerPos) * (float)(*v79 - *centerPos))
                        //    + (float)((float)((float)v84 * (float)v84) + (float)((float)v83 * (float)v83))));
                        //__asm { fsel      f11, f7, f31, f11 }
                        //if ((float)((float)(*(float *)contentMask
                        //    * (float)((float)((float)1.0 / (float)_FP11) * (float)(*v79 - *centerPos)))
                        //    + (float)((float)(*(float *)(contentMask + 8)
                        //        * (float)((float)(v79[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
                        //        + (float)(*(float *)(contentMask + 4)
                        //            * (float)((float)(v79[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) < coneAngleCos)
                        //{
                        //    v81 = 0;
                        //    v82 = colorOrange;
                        //}

                        float dx = v79[0] - centerPos[0];
                        float dy = v79[1] - centerPos[1];
                        float dz = v79[2] - centerPos[2];

                        float len = sqrtf(dx * dx + dy * dy + dz * dz);
                        float invLen = (len > 0.0001f) ? (1.0f / len) : 0.0f;

                        float dot = (*(float *)contentMask * (dx * invLen)) +
                            (*(float *)(contentMask + 8) * (dz * invLen)) +
                            (*(float *)(contentMask + 4) * (dy * invLen));

                        if (dot < coneAngleCos)
                        {
                            v81 = 0;
                            v82 = colorOrange;
                        }

                    }
                }
                if (v81 && !G_LocationalTracePassed(centerPos, v79, targ->s.number, number, v118, 0))
                    v82 = colorRed;
                G_DebugLineWithDuration(centerPos, v79, v82, 1, 200);
                --v80;
                v79 += 3;
            } while (v80);
        }
        //v87 = 0;
        //for (j = &v93; ; j += 3)
        //{
        //    if (coneAngleCos == -1.0)
        //        goto LABEL_59;
        //    if (!contentMask)
        //        goto LABEL_59;
        //    v89 = (float)(j[1] - centerPos[1]);
        //    v90 = (float)(j[2] - centerPos[2]);
        //    _FP7 = -sqrtf((float)((float)((float)(*j - *centerPos) * (float)(*j - *centerPos))
        //        + (float)((float)((float)v90 * (float)v90) + (float)((float)v89 * (float)v89))));
        //    __asm { fsel      f11, f7, f31, f11 }
        //    if ((float)((float)(*(float *)contentMask * (float)((float)((float)1.0 / (float)_FP11) * (float)(*j - *centerPos)))
        //        + (float)((float)(*(float *)(contentMask + 8)
        //            * (float)((float)(j[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
        //            + (float)(*(float *)(contentMask + 4)
        //                * (float)((float)(j[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) >= coneAngleCos)
        //    {
        //    LABEL_59:
        //        if (G_LocationalTracePassed(centerPos, j, targ->s.number, number, v118, 0))
        //            break;
        //    }
        //    if (++v87 >= 5)
        //        return 0;
        //}

        v87 = 0;
        for (float *j = &v93; ; j += 3)
        {
            if (coneAngleCos == -1.0 || !contentMask)
            {
                // Skip the cone angle/content mask check
                if (G_LocationalTracePassed(centerPos, j, targ->s.number, number, v118, 0))
                    break;
            }
            else
            {
                float dx = j[0] - centerPos[0];
                float dy = j[1] - centerPos[1];
                float dz = j[2] - centerPos[2];

                float len = sqrtf(dx * dx + dy * dy + dz * dz);
                float invLen = (len > 0.0001f) ? (1.0f / len) : 0.0f;

                float dot = (*(float *)contentMask * (dx * invLen)) +
                    (*(float *)(contentMask + 8) * (dz * invLen)) +
                    (*(float *)(contentMask + 4) * (dy * invLen));

                if (dot >= coneAngleCos)
                {
                    if (G_LocationalTracePassed(centerPos, j, targ->s.number, number, v118, 0))
                        break;
                }
            }

            if (++v87 >= 5)
                return 0;
        }

    }
    return 1;
}


float __cdecl EntDistToPoint(float *origin, gentity_s *ent)
{
    float *v2; // r11
    double v3; // fp0
    double v4; // fp13
    double v5; // fp0
    double v6; // fp13
    double v7; // fp1
    float *absmax; // r10
    int v9; // r8
    int v10; // r9
    double v11; // fp0
    float back_chain; // [sp+0h] [-10h] BYREF
    float v14; // [sp+4h] [-Ch]
    float v15; // [sp+8h] [-8h]

    v2 = origin;
    if (ent->r.bmodel)
    {
        absmax = ent->r.absmax;
        v9 = (char *)&back_chain - (char *)origin;
        v10 = 3;
        do
        {
            v11 = *v2;
            if (v11 >= *(absmax - 3))
            {
                if (v11 <= *absmax)
                    *(float *)((char *)v2 + v9) = 0.0;
                else
                    *(float *)((char *)v2 + v9) = *v2 - *absmax;
            }
            else
            {
                *(float *)((char *)v2 + v9) = *(absmax - 3) - *v2;
            }
            --v10;
            ++v2;
            ++absmax;
        } while (v10);
        //v7 = sqrtf((float)((float)(v14 * v14) + (float)((float)(v15 * v15) + (float)(back_chain * back_chain))));
        v7 = sqrtf((float)((float)(v14 * v14) + (float)((float)(v15 * v15) + (float)(back_chain * back_chain))));
    }
    else
    {
        v3 = (float)(ent->r.currentOrigin[0] - *origin);
        v4 = (float)(ent->r.currentOrigin[2] - origin[2]);
        v6 = (float)((float)((float)v4 * (float)v4) + (float)((float)v3 * (float)v3));
        v5 = (float)(ent->r.currentOrigin[1] - origin[1]);
        //v7 = sqrtf((float)((float)((float)v5 * (float)v5) + (float)v6));
        v7 = sqrtf((float)((float)((float)v5 * (float)v5) + (float)v6));
    }
    return *((float *)&v7 + 1);
}

void __cdecl GetEntListForRadius(
    const float *origin,
    float radius_max,
    float radius_min,
    int *entList,
    int *entListCount)
{
    float mins[3]; // [esp+0h] [ebp-20h] BYREF
    float boxradius; // [esp+Ch] [ebp-14h]
    float maxs[3]; // [esp+10h] [ebp-10h] BYREF
    int i; // [esp+1Ch] [ebp-4h]

    boxradius = 1.4142135 * radius_max;
    for (i = 0; i < 3; ++i)
    {
        mins[i] = origin[i] - boxradius;
        maxs[i] = origin[i] + boxradius;
    }
    *entListCount = CM_AreaEntities(mins, maxs, entList, MAX_GENTITIES, -1);
}

void __cdecl AddScrTeamName(team_t team)
{
    switch (team)
    {
    case TEAM_FREE:
        Scr_AddConstString(scr_const.free);
        break;
    case TEAM_AXIS:
        Scr_AddConstString(scr_const.axis);
        break;
    case TEAM_ALLIES:
        Scr_AddConstString(scr_const.allies);
        break;
    case TEAM_NEUTRAL:
        Scr_AddConstString(scr_const.neutral);
        break;
    case TEAM_DEAD:
        Scr_AddConstString(scr_const.dead);
        break;
    default:
        Com_PrintWarning(15, "AddScrTeamName(): Unhandled team name %i.\n", team);
        Scr_AddUndefined();
        break;
    }
}

float __cdecl G_GetRadiusDamageDistanceSquared(float *damageOrigin, gentity_s *ent)
{
    double v4; // fp12
    double v5; // fp13
    double v6; // fp0
    float *v7; // r11
    int v8; // r8
    float *absmax; // r10
    int v10; // r9
    double v11; // fp0
    double v12; // fp1
    float v14[6]; // [sp+50h] [-30h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 1301, 0, "%s", "ent");
    if (ent->r.bmodel)
    {
        v7 = damageOrigin;
        v8 = (char *)v14 - (char *)damageOrigin;
        absmax = ent->r.absmax;
        v10 = 3;
        do
        {
            v11 = *v7;
            if (v11 >= *(absmax - 3))
            {
                if (v11 <= *absmax)
                    *(float *)((char *)v7 + v8) = 0.0;
                else
                    *(float *)((char *)v7 + v8) = *v7 - *absmax;
            }
            else
            {
                *(float *)((char *)v7 + v8) = *(absmax - 3) - *v7;
            }
            --v10;
            ++v7;
            ++absmax;
        } while (v10);
        v6 = v14[2];
        v5 = v14[1];
        v4 = v14[0];
    }
    else
    {
        v4 = (float)(ent->r.currentOrigin[0] - *damageOrigin);
        v5 = (float)(ent->r.currentOrigin[1] - damageOrigin[1]);
        v6 = (float)(ent->r.currentOrigin[2] - damageOrigin[2]);
    }
    v12 = (float)((float)((float)v5 * (float)v5) + (float)((float)((float)v4 * (float)v4) + (float)((float)v6 * (float)v6)));
    return *((float *)&v12 + 1);
}

bool __cdecl G_WithinDamageRadius(float *damageOrigin, double radiusSquared, gentity_s *ent)
{
    iassert(ent);
    return G_GetRadiusDamageDistanceSquared(damageOrigin, ent) < radiusSquared;
}

bool __cdecl G_ClientFlashbanged(gclient_s *client)
{
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 1436, 0, "%s", "client");
    return (client->ps.pm_flags & 0x10000) != 0
        && level.time < client->ps.shellshockDuration + client->ps.shellshockTime
        && BG_GetShellshockParms(client->ps.shellshockIndex)->screenBlend.type == SHELLSHOCK_VIEWTYPE_FLASHED;
}

int __cdecl G_GetHitLocationString(unsigned int hitLoc)
{
    if (hitLoc >= 0x13)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 1459, 0, "%s", "(unsigned)hitLoc < HITLOC_NUM");
    return g_HitLocConstNames[hitLoc];
}

int __cdecl G_CanRadiusDamage(
    gentity_s *targ,
    gentity_s *inflictor,
    const float *centerPos,
    double radius,
    double coneAngleCos,
    float *coneDirection,
    int contentMask)
{
    iassert(targ);

    return G_CanRadiusDamageFromPos(
        targ,
        targ->r.currentOrigin,
        inflictor,
        centerPos,
        radius,
        coneAngleCos,
        coneDirection,
        0.0,
        true,
        contentMask);
}

void __cdecl FlashbangBlastEnt(
    gentity_s *ent,
    float *blastOrigin,
    double radius_max,
    double radius_min,
    gentity_s *attacker,
    team_t team)
{
    float *v14; // r7
    float *v15; // r6
    double v16; // fp29
    double v17; // fp30
    actor_s *actor; // r3
    double v19; // fp13
    double v20; // fp31
    float v21; // [sp+50h] [-80h] BYREF
    float v22; // [sp+54h] [-7Ch]
    float v23; // [sp+58h] [-78h]
    float v24[4]; // [sp+60h] [-70h] BYREF
    float v25[14]; // [sp+70h] [-60h] BYREF

    if (ent->takedamage)
    {
        v16 = EntDistToPoint(blastOrigin, ent);
        if (v16 <= radius_max)
        {
            if (G_CanRadiusDamage(ent, attacker, blastOrigin, radius_max, 1.0, 0, 10241))
            {
                if (v16 > radius_min)
                    v17 = (float)((float)1.0
                        - (float)((float)((float)v16 - (float)radius_min) / (float)((float)radius_max - (float)radius_min)));
                else
                    v17 = 1.0;
                if (ent->client)
                {
                    G_GetPlayerViewDirection(ent, v25, 0, 0);
                    G_GetPlayerViewOrigin(&ent->client->ps, v24);
                    Actor_ClearAllSuppressionFromEnemySentient(ent->sentient);
                }
                else
                {
                    actor = ent->actor;
                    if (actor)
                    {
                        Actor_GetEyeDirection(actor, v25);
                        Actor_GetEyePosition(ent->actor, v24);
                    }
                    else
                    {
                        AngleVectors(ent->r.currentAngles, v25, 0, 0);
                        G_EntityCentroid(ent, v24);
                    }
                }
                v19 = (float)(blastOrigin[1] - v24[1]);
                v21 = *blastOrigin - v24[0];
                v22 = v19;
                v23 = blastOrigin[2] - v24[2];
                Vec3NormalizeFast(&v21);
                v20 = (float)((float)((float)((float)(v21 * v25[0]) + (float)((float)(v25[2] * v23) + (float)(v25[1] * v22)))
                    + (float)1.0)
                    * (float)0.5);
                AddScrTeamName(team);
                if (attacker)
                    Scr_AddEntity(attacker);
                else
                    Scr_AddUndefined();
                Scr_AddFloat(v20);
                Scr_AddFloat(v17);
                Scr_Notify(ent, scr_const.flashbang, 4u);
            }
        }
    }
}

void __cdecl G_FlashbangBlast(
    float *origin,
    double radius_max,
    double radius_min,
    gentity_s *attacker,
    team_t team)
{
    int entList[MAX_GENTITIES];
    int entListCount;

    if (radius_min < 1.0)
        radius_min = 1.0f;
    if (radius_min > radius_max)
        radius_max = radius_min;

    GetEntListForRadius(origin, radius_max, radius_min, entList, &entListCount);

    for (int i = 0; i < entListCount; i++)
    {
        gentity_s *ent = &g_entities[entList[i]];
        iassert(ent);

        FlashbangBlastEnt(ent, origin, radius_max, radius_min, attacker, team);
    }
}

int __cdecl G_RadiusDamage(
    float *origin,
    gentity_s *inflictor,
    gentity_s *attacker,
    double fInnerDamage,
    double fOuterDamage,
    double radius,
    double coneAngleCos,
    float *coneDirection,
    gentity_s *ignore,
    int mod,
    int weapon)
{
    int v44; // r18
    double v45; // fp12
    double v46; // fp11
    double v47; // fp13
    int v48; // r3
    int *v49; // r23
    int v50; // r19
    gentity_s *ent; // r31
    float *v52; // r7
    float *v53; // r6
    double RadiusDamageDistanceSquared; // fp1
    double damage; // fp30
    double v56; // fp11
    double v57; // fp10
    double v58; // fp0
    int v60; // [sp+8h] [-2368h]
    hitLocation_t v61; // [sp+Ch] [-2364h]
    unsigned int v62; // [sp+10h] [-2360h]
    unsigned int v63; // [sp+14h] [-235Ch]
    float v82[4]; // [sp+78h] [-22F8h] BYREF
    float dir[4]; // [sp+88h] [-22E8h] BYREF
    float v84[6]; // [sp+98h] [-22D8h] BYREF
    int v85[176]; // [sp+B0h] [-22C0h] BYREF

    v44 = 0;
    Actor_BroadcastPointEvent(attacker, 7, 14, origin, 0.0);
    if (radius < 1.0)
        radius = 1.0;
    v45 = origin[1];
    v46 = origin[2];
    v47 = (float)(*origin + (float)((float)radius * (float)1.4142135));
    v84[0] = *origin - (float)((float)radius * (float)1.4142135);
    v82[0] = v47;
    v84[1] = (float)v45 - (float)((float)radius * (float)1.4142135);
    v82[1] = (float)v45 + (float)((float)radius * (float)1.4142135);
    v84[2] = (float)v46 - (float)((float)radius * (float)1.4142135);
    v82[2] = (float)v46 + (float)((float)radius * (float)1.4142135);
    v48 = CM_AreaEntities(v84, v82, v85, MAX_GENTITIES, -1);
    if (v48 > 0)
    {
        v49 = v85;
        v50 = v48;
        do
        {
            ent = &g_entities[*v49];
            if (ent != ignore && ent->takedamage && (!ent->client || !level.bPlayerIgnoreRadiusDamage))
            {
                RadiusDamageDistanceSquared = G_GetRadiusDamageDistanceSquared(origin, ent);
                if (RadiusDamageDistanceSquared < (radius * radius))
                {
                    damage = (float)((float)((float)((float)1.0
                        //- (float)((float)sqrtf(RadiusDamageDistanceSquared) / (float)radius))
                        - (float)((float)sqrtf(RadiusDamageDistanceSquared) / (float)radius))
                        * (float)((float)fInnerDamage - (float)fOuterDamage))
                        + (float)fOuterDamage);
                    if (G_CanRadiusDamage(ent, inflictor, origin, radius, coneAngleCos, coneDirection, 8396817))
                    {
                        if (attacker && LogAccuracyHit(ent, attacker))
                            v44 = 1;
                        v56 = origin[1];
                        v57 = ent->r.currentOrigin[1];
                        v58 = (float)(ent->r.currentOrigin[2] - origin[2]);

                        dir[0] = ent->r.currentOrigin[0] - *origin;
                        dir[1] = (float)v57 - (float)v56;
                        dir[2] = (float)v58 + (float)24.0;

                        G_Damage(
                            ent,
                            inflictor,
                            attacker,
                            dir,
                            origin,
                            (int)damage,
                            5, // dflags
                            mod,
                            weapon,
                            HITLOC_NONE,
                            0,
                            0);
                    }
                }
            }
            --v50;
            ++v49;
        } while (v50);
    }
    return v44;
}

