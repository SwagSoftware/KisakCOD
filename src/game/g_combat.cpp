#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"

unsigned char *bulletPriorityMap;
unsigned char *riflePriorityMap;
unsigned short **modNames;
float *g_fHitLocDamageMult;

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
        (pszKeyValue++)[v2] = v3;
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
        *v1++ = Scr_AllocString(v3, 1);
        p_iOffset += 3;
        ++v0;
    } while ((int)v1 < (int)&word_82C43A02);
    g_fHitLocDamageMult[18] = 0.0;
    InfoString = Com_LoadInfoString("info/ai_lochit_dmgtable", "hitloc damage table", "LOCDMGTABLE", v6);
    if (!ParseConfigStringToStruct((unsigned __int8 *)g_fHitLocDamageMult, v5, 19, InfoString, 0, 0, G_HitLocStrcpy))
        Com_Error(ERR_DROP, byte_8202D650, "info/ai_lochit_dmgtable");
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
    const char *v5; // r3
    const char *v6; // r3

    ConstString = Scr_GetConstString(scrParam);
    v3 = 0;
    v4 = modNames;
    while (**v4 != ConstString)
    {
        ++v4;
        ++v3;
        if ((int)v4 >= (int)WeaponStateNames_110)
        {
            v5 = SL_ConvertToString(ConstString);
            v6 = va("Unknown means of death \"%s\"\n", v5);
            Scr_ParamError(scrParam, v6);
            return 0;
        }
    }
    return v3;
}

void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int meansOfDeath,
    unsigned int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc)
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
        self->client->ps.pm_type = (self->client->ps.pm_type == 1) + 5;
        if (!attacker || (number = attacker->s.number) != 0)
            number = 2174;
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
        self->handler = 18;
        SV_TrackPlayerDied();
        if (p_szInternalName)
            v25 = *p_szInternalName;
        else
            v25 = "NONE";
        v26 = self->client;
        v27 = SL_ConvertToStringSafe(*modNames[v20]);
        v28 = va(
            (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(v26->ps.origin[0])),
            (unsigned int)COERCE_UNSIGNED_INT64(v26->ps.origin[0]),
            (unsigned int)COERCE_UNSIGNED_INT64(v26->ps.origin[1]),
            (unsigned int)COERCE_UNSIGNED_INT64(v26->ps.origin[2]),
            v27,
            v25,
            hitLoc,
            number);
        LSP_LogString(cl_controller_in_use, v28);
        LSP_SendLogRequest(cl_controller_in_use);
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
    unsigned int partName,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    unsigned int a28)
{
    unsigned int v36; // r27
    unsigned int ConfigstringConst; // r31
    const float *v38; // r3
    const float *v39; // r3

    if (a28)
        Scr_AddConstString(a28);
    else
        Scr_AddString(byte_82003CDD);
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
        ConfigstringConst = SV_GetConfigstringConst(*((unsigned __int16 *)&targ->scripted + modelIndex + 1) + 1155);
        if (!ConfigstringConst)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 469, 1, "%s", "modelName");
        Scr_AddConstString(*(unsigned __int16 *)(&targ->s.eType + v36));
        Scr_AddConstString(ConfigstringConst);
    }
    else
    {
        Scr_AddString(byte_82003CDD);
        Scr_AddString(byte_82003CDD);
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
            if (client->ps.viewlocked_entNum == 2175)
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

    if (!targ)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 602, 0, "%s", "targ");
    if (!attacker)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 603, 0, "%s", "attacker");
    sentient = targ->sentient;
    if (!sentient || !sentient->ignoreRandomBulletDamage || !attacker->actor)
        return 1;
    if (!attacker->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 614, 0, "%s", "attacker->sentient");
    if (EntHandle::isDefined(&attacker->sentient->targetEnt) && EntHandle::ent(&attacker->sentient->targetEnt) == targ)
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
    unsigned int partName,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    unsigned int a28,
    int a29,
    unsigned int a30,
    int a31,
    unsigned int a32,
    int a33,
    unsigned int a34)
{
    int v34; // r29
    unsigned int WeaponIndexForEntity; // r19
    const gentity_s *v43; // r3
    unsigned int NumWeapons; // r3
    gclient_s *client; // r24
    __int64 v46; // r10
    const dvar_s *v47; // r11
    __int64 v48; // fp13
    unsigned int v49; // r20
    double WeaponHitLocationMultiplier; // fp1
    actor_s *actor; // r3
    unsigned int v52; // r27
    unsigned int v53; // r26
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

    v34 = damage;
    a22 = damage;
    if (!targ)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 667, 0, "%s", "targ");
    if (targ->takedamage)
    {
        if (!inflictor)
            inflictor = &g_entities[2174];
        if (!attacker)
            attacker = &g_entities[2174];
        if (!targ->r.inuse)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                680,
                0,
                "%s\n\t(targ->s.number) = %i",
                "(targ->r.inuse)",
                targ->s.number);
        if (!attacker->r.inuse)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                681,
                0,
                "%s\n\t(attacker - g_entities) = %i",
                "(attacker->r.inuse)",
                attacker - g_entities);
        WeaponIndexForEntity = a28;
        if (a28 == -1)
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
                    LODWORD(v46) = v34;
                    if ((dflags & 1) != 0)
                    {
                        v47 = player_radiusDamageMultiplier;
                        v48 = v46;
                    }
                    else
                    {
                        if (mod == 7)
                            v47 = player_meleeDamageMultiplier;
                        else
                            v47 = player_damageMultiplier;
                        v48 = v46;
                    }
                    a22 = (int)(float)(v47->current.value * (float)v48);
                    v34 = a22;
                }
                if (v34 <= 0)
                {
                    v34 = 1;
                    a22 = 1;
                }
            }
            if (!G_ShouldTakeBulletDamage(targ, attacker))
                return;
            v49 = a30;
            if (targ->actor && mod != 7)
            {
                if (a30 > 0x12)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp",
                        744,
                        0,
                        "%s",
                        "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)");
                WeaponHitLocationMultiplier = G_GetWeaponHitLocationMultiplier(v49, WeaponIndexForEntity);
                if (WeaponHitLocationMultiplier == 0.0)
                    return;
                a22 = (int)(float)((float)__SPAIR64__(&a22, v34) * (float)WeaponHitLocationMultiplier);
                v34 = a22;
                if (a22 <= 0)
                    v34 = 1;
            }
            v86 = 0.0;
            v87 = 0.0;
            v88 = 0.0;
            G_DamageKnockback(targ, attacker, dir, &v86, __SPAIR64__(v34, dflags), mod);
            if ((targ->flags & 1) == 0)
            {
                if (v34 < 1)
                    v34 = 1;
                actor = targ->actor;
                v52 = a34;
                v53 = a32;
                if (actor)
                {
                    v54 = Actor_CheckDeathAllowed(actor, v34);
                    v55 = v54;
                    v34 -= v54;
                    if (g_debugDamage->current.enabled)
                        Com_Printf(15, "client:%i health:%i damage:%i armor:%i\n", targ->s.number, targ->health, v34, v54);
                    if (v55)
                        G_DamageNotify(
                            scr_const.damage_notdone,
                            targ,
                            attacker,
                            dir,
                            point,
                            v55,
                            mod,
                            v53,
                            v67,
                            v68,
                            v69,
                            v70,
                            v71,
                            v72,
                            v73,
                            v74,
                            v75,
                            v76,
                            v77,
                            v78,
                            v79,
                            v80,
                            v81,
                            v82,
                            v83,
                            v84,
                            v85,
                            v52);
                }
                if (client)
                {
                    client->damage_blood += v34;
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
                if (v34)
                {
                    if ((targ->flags & 2) != 0)
                    {
                        health = targ->health;
                        if (health - v34 <= 0)
                            v34 = health - 1;
                    }
                    v57 = targ->health;
                    targ->health = v57 - v34;
                    if (client && client->invulnerableEnabled)
                        handleDeathInvulnerability(targ, v57, mod);
                    G_DamageNotify(
                        scr_const.damage,
                        targ,
                        attacker,
                        dir,
                        point,
                        v34,
                        mod,
                        v53,
                        v67,
                        v68,
                        v69,
                        v70,
                        v71,
                        v72,
                        v73,
                        v74,
                        v75,
                        v76,
                        v77,
                        v78,
                        v79,
                        v80,
                        v81,
                        v82,
                        v83,
                        v84,
                        v85,
                        v52);
                    v58 = targ->health;
                    if (v58 > 0)
                    {
                        pain = entityHandlers[targ->handler].pain;
                        if (pain)
                            pain(targ, attacker, v34, point, mod, &v86, (const hitLocation_t)v49, WeaponIndexForEntity);
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
                        die(targ, inflictor, attacker, v34, mod, v64, &v86, (const hitLocation_t)v49);
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
    float *coneDirection,
    double maxHeight,
    bool useEyeOffset,
    float *contentMask,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    char a31,
    int a32,
    int a33)
{
    double v38; // fp27
    double v40; // fp13
    double v41; // fp12
    double v42; // fp25
    int number; // r21
    sentient_s *sentient; // r3
    double v45; // fp13
    double v48; // fp12
    double v49; // fp30
    double v50; // fp29
    double v51; // fp28
    double v52; // fp0
    double v53; // fp13
    double v54; // fp12
    double v55; // fp10
    double v56; // fp11
    float *v57; // r30
    int v58; // r28
    char v59; // r11
    const float *v60; // r29
    double v61; // fp13
    double v62; // fp12
    int v65; // r29
    float *i; // r30
    double v67; // fp13
    double v68; // fp12
    const DObj_s *ServerDObj; // r3
    double v73; // fp27
    double v74; // fp26
    double v75; // fp25
    double v76; // fp13
    double v79; // fp11
    double v80; // fp12
    double v81; // fp0
    double v82; // fp12
    double v85; // fp13
    double v86; // fp29
    double v87; // fp30
    double v88; // fp28
    double v89; // fp8
    double v90; // fp7
    double v91; // fp8
    double v92; // fp12
    double v93; // fp9
    double v94; // fp0
    double v95; // fp7
    double v96; // fp5
    double v97; // fp10
    double v98; // fp0
    double v99; // fp9
    double v100; // fp13
    double v101; // fp12
    float *v102; // r30
    int v103; // r28
    char v104; // r11
    const float *v105; // r29
    double v106; // fp0
    double v107; // fp13
    int v110; // r29
    float *j; // r30
    double v112; // fp0
    double v113; // fp13
    float v116; // [sp+50h] [-140h] BYREF
    float v117; // [sp+54h] [-13Ch]
    float v118; // [sp+58h] [-138h]
    float v119; // [sp+5Ch] [-134h]
    float v120; // [sp+60h] [-130h]
    float v121; // [sp+64h] [-12Ch]
    float v122; // [sp+68h] [-128h]
    float v123; // [sp+6Ch] [-124h]
    float v124; // [sp+70h] [-120h]
    float v125; // [sp+74h] [-11Ch]
    float v126; // [sp+78h] [-118h]
    float v127; // [sp+7Ch] [-114h]
    float v128; // [sp+80h] [-110h]
    float v129; // [sp+84h] [-10Ch]
    float v130; // [sp+88h] [-108h]
    float v131[4]; // [sp+90h] [-100h] BYREF
    float v132[2]; // [sp+A0h] [-F0h] BYREF
    float v133; // [sp+A8h] [-E8h]
    float v134[4]; // [sp+B0h] [-E0h] BYREF
    float v135[4]; // [sp+C0h] [-D0h] BYREF
    float v136; // [sp+D0h] [-C0h] BYREF
    float v137; // [sp+D4h] [-BCh]
    float v138; // [sp+D8h] [-B8h]
    float v139[20]; // [sp+E0h] [-B0h] BYREF

    v38 = 15.0;
    if ((targ->r.contents & 0x405C0008) == 0)
    {
        v40 = (float)(centerPos[2] - targetPos[2]);
        v41 = (float)(centerPos[1] - targetPos[1]);
        if ((float)((float)((float)v41 * (float)v41)
            + (float)((float)((float)(*centerPos - *targetPos) * (float)(*centerPos - *targetPos))
                + (float)((float)v40 * (float)v40))) >= (double)(float)((float)radius * (float)radius))
            return 0;
    }
    v42 = targ->r.currentOrigin[2];
    if (inflictor)
        number = inflictor->s.number;
    else
        number = 2175;
    sentient = targ->sentient;
    if (sentient)
    {
        if (targ->client)
            v38 = 8.0;
        v45 = (float)(centerPos[1] - targetPos[1]);
        _FP10 = -__fsqrts((float)((float)((float)(*centerPos - *targetPos) * (float)(*centerPos - *targetPos))
            + (float)((float)v45 * (float)v45)));
        __asm { fsel      f12, f10, f31, f12 }
        v48 = (float)((float)1.0 / (float)_FP12);
        v49 = (float)((float)v48 * (float)(*centerPos - *targetPos));
        v50 = (float)((float)v48 * (float)0.0);
        v51 = -(float)((float)v48 * (float)(centerPos[1] - targetPos[1]));
        if (maxHeight == 0.0)
        {
            Sentient_GetEyePosition(sentient, v132);
            v52 = 0.5;
            v53 = (float)((float)(v133 - (float)v42) * (float)0.5);
        }
        else
        {
            if (a31)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 952, 0, "%s", "!useEyeOffset");
            v52 = 0.5;
            v53 = (float)((float)maxHeight * (float)0.5);
        }
        v54 = *targetPos;
        if (a31)
        {
            v54 = (float)((float)(v132[0] + *targetPos) * (float)v52);
            v55 = (float)((float)(targetPos[1] + v132[1]) * (float)v52);
            v56 = (float)((float)(targetPos[2] + v133) * (float)v52);
        }
        else
        {
            v55 = targetPos[1];
            v56 = (float)(targetPos[2] + (float)v53);
        }
        v118 = v56;
        v119 = (float)((float)v51 * (float)v38) + (float)v54;
        v122 = v119;
        v116 = v54;
        v117 = v55;
        v120 = (float)((float)v49 * (float)v38) + (float)v55;
        v123 = v120;
        v121 = (float)((float)((float)v50 * (float)v38) + (float)v56) + (float)v53;
        v124 = (float)((float)((float)v50 * (float)v38) + (float)v56) - (float)v53;
        v125 = (float)((float)-v38 * (float)v51) + (float)v54;
        v126 = (float)((float)-v38 * (float)v49) + (float)v55;
        v128 = v125;
        v129 = v126;
        v127 = (float)((float)((float)-v38 * (float)v50) + (float)v56) + (float)v53;
        v130 = (float)((float)((float)-v38 * (float)v50) + (float)v56) - (float)v53;
        if (radius_damage_debug->current.enabled)
        {
            v57 = &v116;
            v58 = 5;
            do
            {
                v59 = 1;
                v60 = colorWhite;
                if (coneAngleCos != -1.0)
                {
                    if (contentMask)
                    {
                        v61 = (float)(v57[2] - centerPos[2]);
                        v62 = (float)(v57[1] - centerPos[1]);
                        _FP7 = -__fsqrts((float)((float)((float)v62 * (float)v62)
                            + (float)((float)((float)(*v57 - *centerPos) * (float)(*v57 - *centerPos))
                                + (float)((float)v61 * (float)v61))));
                        __asm { fsel      f11, f7, f31, f11 }
                        if ((float)((float)(*contentMask * (float)((float)((float)1.0 / (float)_FP11) * (float)(*v57 - *centerPos)))
                            + (float)((float)(contentMask[2]
                                * (float)((float)(v57[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
                                + (float)(contentMask[1]
                                    * (float)((float)(v57[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) < coneAngleCos)
                        {
                            v59 = 0;
                            v60 = colorOrange;
                        }
                    }
                }
                if (v59 && !G_LocationalTracePassed(centerPos, v57, targ->s.number, number, a33, 0))
                    v60 = colorRed;
                G_DebugLineWithDuration(centerPos, v57, v60, 1, 200);
                --v58;
                v57 += 3;
            } while (v58);
        }
        v65 = 0;
        for (i = &v116; ; i += 3)
        {
            if (coneAngleCos == -1.0)
                goto LABEL_58;
            if (!contentMask)
                goto LABEL_58;
            v67 = (float)(i[2] - centerPos[2]);
            v68 = (float)(i[1] - centerPos[1]);
            _FP7 = -__fsqrts((float)((float)((float)v68 * (float)v68)
                + (float)((float)((float)(*i - *centerPos) * (float)(*i - *centerPos))
                    + (float)((float)v67 * (float)v67))));
            __asm { fsel      f11, f7, f31, f11 }
            if ((float)((float)(*contentMask * (float)((float)((float)1.0 / (float)_FP11) * (float)(*i - *centerPos)))
                + (float)((float)(contentMask[2]
                    * (float)((float)(i[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
                    + (float)(contentMask[1]
                        * (float)((float)(i[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) >= coneAngleCos)
            {
            LABEL_58:
                if (G_LocationalTracePassed(centerPos, i, targ->s.number, number, a33, 0))
                    break;
            }
            if (++v65 >= 5)
                return 0;
        }
    }
    else
    {
        if (targ->classname == scr_const.script_model && targ->model)
        {
            ServerDObj = Com_GetServerDObj(targ->s.number);
            DObjPhysicsGetBounds(ServerDObj, v139, v131);
            G_EntityCentroidWithBounds(targ, v139, v131, &v116);
            v73 = (float)(v131[0] + targ->r.currentOrigin[0]);
            v74 = (float)(targ->r.currentOrigin[1] + v131[1]);
            v75 = (float)(targ->r.currentOrigin[2] + v131[2]);
        }
        else
        {
            G_EntityCentroid(targ, &v116);
            v73 = targ->r.absmax[0];
            v74 = targ->r.absmax[1];
            v75 = targ->r.absmax[2];
        }
        v76 = (float)(centerPos[2] - v118);
        _FP10 = -__fsqrts((float)((float)((float)(centerPos[1] - v117) * (float)(centerPos[1] - v117))
            + (float)((float)((float)(centerPos[2] - v118) * (float)(centerPos[2] - v118))
                + (float)((float)(*centerPos - v116) * (float)(*centerPos - v116)))));
        __asm { fsel      f11, f10, f31, f11 }
        v79 = (float)((float)1.0 / (float)_FP11);
        v80 = (float)((float)v79 * (float)(centerPos[1] - v117));
        v81 = (float)((float)v79 * (float)(*centerPos - v116));
        v135[1] = (float)v79 * (float)(centerPos[1] - v117);
        v135[0] = v81;
        v135[2] = (float)v76 * (float)v79;
        v82 = -v80;
        _FP11 = -__fsqrts((float)((float)((float)v81 * (float)v81) + (float)((float)v82 * (float)v82)));
        __asm { fsel      f13, f11, f31, f13 }
        v85 = (float)((float)1.0 / (float)_FP13);
        v86 = (float)((float)v85 * (float)v81);
        v87 = (float)((float)v85 * (float)v82);
        v134[1] = (float)v85 * (float)v81;
        v88 = (float)((float)v85 * (float)0.0);
        v134[0] = (float)v85 * (float)v82;
        v134[2] = (float)v85 * (float)0.0;
        Vec3Cross(v135, v134, &v136);
        v89 = __fabs((float)((float)((float)v73 - v116) * (float)v87));
        v90 = __fabs((float)((float)((float)v74 - v117) * (float)v86));
        v92 = (float)((float)((float)__fabs((float)(v137 * (float)((float)v74 - v117)))
            + (float)__fabs((float)(v136 * (float)((float)v73 - v116))))
            + (float)__fabs((float)((float)((float)v75 - v118) * v138)));
        v93 = (float)((float)((float)v89 + (float)v90) * (float)v87);
        v94 = (float)((float)((float)v89 + (float)v90) * (float)v88);
        v91 = (float)((float)((float)v89 + (float)v90) * (float)v86);
        v95 = (float)(v116 + (float)v93);
        v96 = (float)((float)v94 + v118);
        v97 = (float)(v118 - (float)v94);
        v98 = (float)(v136 * (float)v92);
        v100 = (float)(v137 * (float)v92);
        v119 = (float)(v116 + (float)v93) + (float)(v136 * (float)v92);
        v99 = (float)(v116 - (float)v93);
        v120 = (float)(v117 + (float)v91) + (float)(v137 * (float)v92);
        v101 = (float)(v138 * (float)v92);
        v121 = (float)v101 + (float)v96;
        v122 = (float)v99 + (float)v98;
        v123 = (float)(v117 - (float)v91) + (float)v100;
        v124 = (float)v101 + (float)v97;
        v125 = (float)v95 - (float)v98;
        v128 = (float)v99 - (float)v98;
        v126 = (float)(v117 + (float)v91) - (float)v100;
        v129 = (float)(v117 - (float)v91) - (float)v100;
        v127 = (float)v96 - (float)v101;
        v130 = (float)v97 - (float)v101;
        if (radius_damage_debug->current.enabled)
        {
            v102 = &v116;
            v103 = 5;
            do
            {
                v104 = 1;
                v105 = colorWhite;
                if (coneAngleCos != -1.0)
                {
                    if (contentMask)
                    {
                        v106 = (float)(v102[1] - centerPos[1]);
                        v107 = (float)(v102[2] - centerPos[2]);
                        _FP7 = -__fsqrts((float)((float)((float)(*v102 - *centerPos) * (float)(*v102 - *centerPos))
                            + (float)((float)((float)v107 * (float)v107) + (float)((float)v106 * (float)v106))));
                        __asm { fsel      f11, f7, f31, f11 }
                        if ((float)((float)(*contentMask * (float)((float)((float)1.0 / (float)_FP11) * (float)(*v102 - *centerPos)))
                            + (float)((float)(contentMask[2]
                                * (float)((float)(v102[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
                                + (float)(contentMask[1]
                                    * (float)((float)(v102[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) < coneAngleCos)
                        {
                            v104 = 0;
                            v105 = colorOrange;
                        }
                    }
                }
                if (v104 && !G_LocationalTracePassed(centerPos, v102, targ->s.number, number, a33, 0))
                    v105 = colorRed;
                G_DebugLineWithDuration(centerPos, v102, v105, 1, 200);
                --v103;
                v102 += 3;
            } while (v103);
        }
        v110 = 0;
        for (j = &v116; ; j += 3)
        {
            if (coneAngleCos == -1.0)
                goto LABEL_59;
            if (!contentMask)
                goto LABEL_59;
            v112 = (float)(j[1] - centerPos[1]);
            v113 = (float)(j[2] - centerPos[2]);
            _FP7 = -__fsqrts((float)((float)((float)(*j - *centerPos) * (float)(*j - *centerPos))
                + (float)((float)((float)v113 * (float)v113) + (float)((float)v112 * (float)v112))));
            __asm { fsel      f11, f7, f31, f11 }
            if ((float)((float)(*contentMask * (float)((float)((float)1.0 / (float)_FP11) * (float)(*j - *centerPos)))
                + (float)((float)(contentMask[2]
                    * (float)((float)(j[2] - centerPos[2]) * (float)((float)1.0 / (float)_FP11)))
                    + (float)(contentMask[1]
                        * (float)((float)(j[1] - centerPos[1]) * (float)((float)1.0 / (float)_FP11))))) >= coneAngleCos)
            {
            LABEL_59:
                if (G_LocationalTracePassed(centerPos, j, targ->s.number, number, a33, 0))
                    break;
            }
            if (++v110 >= 5)
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
        v7 = __fsqrts((float)((float)(v14 * v14) + (float)((float)(v15 * v15) + (float)(back_chain * back_chain))));
    }
    else
    {
        v3 = (float)(ent->r.currentOrigin[0] - *origin);
        v4 = (float)(ent->r.currentOrigin[2] - origin[2]);
        v6 = (float)((float)((float)v4 * (float)v4) + (float)((float)v3 * (float)v3));
        v5 = (float)(ent->r.currentOrigin[1] - origin[1]);
        v7 = __fsqrts((float)((float)((float)v5 * (float)v5) + (float)v6));
    }
    return *((float *)&v7 + 1);
}

void __cdecl GetEntListForRadius(
    const float *origin,
    double radius_max,
    double radius_min,
    int *entList,
    int *entListCount,
    int *a6,
    unsigned int *a7)
{
    double v7; // fp13
    double v8; // fp12
    double v9; // fp11
    float v10[4]; // [sp+50h] [-30h] BYREF
    float v11[4]; // [sp+60h] [-20h] BYREF

    v7 = *origin;
    v8 = origin[1];
    v9 = origin[2];
    v10[0] = *origin - (float)((float)radius_max * (float)1.4142135);
    v11[0] = (float)v7 + (float)((float)radius_max * (float)1.4142135);
    v10[1] = (float)v8 - (float)((float)radius_max * (float)1.4142135);
    v11[1] = (float)v8 + (float)((float)radius_max * (float)1.4142135);
    v10[2] = (float)v9 - (float)((float)radius_max * (float)1.4142135);
    v11[2] = (float)v9 + (float)((float)radius_max * (float)1.4142135);
    *a7 = CM_AreaEntities(v10, v11, a6, 2176, -1);
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

bool __cdecl G_WithinDamageRadius(float *damageOrigin, double radiusSquared, gentity_s *ent, gentity_s *a4)
{
    if (!a4)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 1333, 0, "%s", "ent");
    return G_GetRadiusDamageDistanceSquared(damageOrigin, a4) < radiusSquared;
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
    float *contentMask,
    float *a8,
    int a9)
{
    int v17; // [sp+8h] [-98h]
    int v18; // [sp+Ch] [-94h]
    int v19; // [sp+10h] [-90h]
    int v20; // [sp+14h] [-8Ch]
    int v21; // [sp+18h] [-88h]
    int v22; // [sp+1Ch] [-84h]
    int v23; // [sp+20h] [-80h]
    int v24; // [sp+24h] [-7Ch]
    int v25; // [sp+28h] [-78h]
    int v26; // [sp+2Ch] [-74h]
    int v27; // [sp+30h] [-70h]
    int v28; // [sp+34h] [-6Ch]
    int v29; // [sp+38h] [-68h]
    int v30; // [sp+3Ch] [-64h]
    int v31; // [sp+40h] [-60h]
    int v32; // [sp+44h] [-5Ch]
    int v33; // [sp+48h] [-58h]
    int v34; // [sp+4Ch] [-54h]
    int v35; // [sp+50h] [-50h]
    int v36; // [sp+58h] [-48h]

    if (!targ)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 895, 0, "%s", "targ");
    return G_CanRadiusDamageFromPos(
        targ,
        targ->r.currentOrigin,
        inflictor,
        centerPos,
        radius,
        coneAngleCos,
        contentMask,
        0.0,
        (bool)a8,
        a8,
        1,
        v17,
        v18,
        v19,
        v20,
        v21,
        v22,
        v23,
        v24,
        v25,
        v26,
        v27,
        v28,
        v29,
        v30,
        v31,
        v32,
        v33,
        v34,
        v35,
        1,
        v36,
        a9);
}

void __cdecl FlashbangBlastEnt(
    gentity_s *ent,
    float *blastOrigin,
    double radius_max,
    double radius_min,
    gentity_s *attacker,
    team_t team,
    gentity_s *a7,
    team_t a8)
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
            if (G_CanRadiusDamage(ent, a7, blastOrigin, radius_max, 1.0, v15, v14, 0, 10241))
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
                AddScrTeamName(a8);
                if (a7)
                    Scr_AddEntity(a7);
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
    int *team,
    gentity_s *a6,
    team_t a7)
{
    double v7; // fp31
    double v9; // fp30
    team_t v12; // r6
    gentity_s *v13; // r5
    int v14; // r29
    int *v15; // r30
    gentity_s *v16; // r31
    int v17[4]; // [sp+50h] [-2280h] BYREF
    int v18[156]; // [sp+60h] [-2270h] BYREF

    v7 = radius_min;
    v9 = radius_max;
    if (radius_min < 1.0)
        v7 = 1.0;
    if (radius_max < v7)
        v9 = v7;
    GetEntListForRadius(origin, v9, v7, (int *)attacker, team, v18, v17);
    v14 = v17[0];
    if (v17[0] > 0)
    {
        v15 = v18;
        do
        {
            v16 = &g_entities[*v15];
            if (!v16)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_combat.cpp", 1286, 0, "%s", "ent");
            FlashbangBlastEnt(v16, origin, v9, v7, v13, v12, a6, a7);
            --v14;
            ++v15;
        } while (v14);
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
    int weapon,
    float *a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    int a31,
    gentity_s *a32,
    int a33,
    int a34,
    int a35,
    unsigned int a36)
{
    int v44; // r18
    double v45; // fp12
    double v46; // fp11
    double v47; // fp13
    int v48; // r3
    int *v49; // r23
    int v50; // r19
    gentity_s *v51; // r31
    float *v52; // r7
    float *v53; // r6
    double RadiusDamageDistanceSquared; // fp1
    double v55; // fp30
    double v56; // fp11
    double v57; // fp10
    double v58; // fp0
    int v60; // [sp+8h] [-2368h]
    hitLocation_t v61; // [sp+Ch] [-2364h]
    unsigned int v62; // [sp+10h] [-2360h]
    unsigned int v63; // [sp+14h] [-235Ch]
    int v64; // [sp+18h] [-2358h]
    int v65; // [sp+1Ch] [-2354h]
    int v66; // [sp+20h] [-2350h]
    int v67; // [sp+24h] [-234Ch]
    int v68; // [sp+28h] [-2348h]
    int v69; // [sp+2Ch] [-2344h]
    int v70; // [sp+30h] [-2340h]
    int v71; // [sp+34h] [-233Ch]
    int v72; // [sp+38h] [-2338h]
    int v73; // [sp+3Ch] [-2334h]
    int v74; // [sp+40h] [-2330h]
    int v75; // [sp+44h] [-232Ch]
    int v76; // [sp+48h] [-2328h]
    int v77; // [sp+4Ch] [-2324h]
    int v78; // [sp+50h] [-2320h]
    int v79; // [sp+58h] [-2318h]
    int v80; // [sp+60h] [-2310h]
    int v81; // [sp+68h] [-2308h]
    float v82[4]; // [sp+78h] [-22F8h] BYREF
    float v83[4]; // [sp+88h] [-22E8h] BYREF
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
    v48 = CM_AreaEntities(v84, v82, v85, 2176, -1);
    if (v48 > 0)
    {
        v49 = v85;
        v50 = v48;
        do
        {
            v51 = &g_entities[*v49];
            if (v51 != a32 && v51->takedamage && (!v51->client || !level.bPlayerIgnoreRadiusDamage))
            {
                RadiusDamageDistanceSquared = G_GetRadiusDamageDistanceSquared(origin, v51);
                if (RadiusDamageDistanceSquared < (float)((float)radius * (float)radius))
                {
                    v55 = (float)((float)((float)((float)1.0
                        - (float)((float)__fsqrts(RadiusDamageDistanceSquared) / (float)radius))
                        * (float)((float)fInnerDamage - (float)fOuterDamage))
                        + (float)fOuterDamage);
                    if (G_CanRadiusDamage(v51, inflictor, origin, radius, coneAngleCos, v53, v52, a12, 8396817))
                    {
                        if (attacker && LogAccuracyHit(v51, attacker))
                            v44 = 1;
                        v56 = origin[1];
                        v57 = v51->r.currentOrigin[1];
                        v58 = (float)(v51->r.currentOrigin[2] - origin[2]);
                        v83[0] = v51->r.currentOrigin[0] - *origin;
                        v83[1] = (float)v57 - (float)v56;
                        v83[2] = (float)v58 + (float)24.0;
                        G_Damage(
                            v51,
                            inflictor,
                            attacker,
                            v83,
                            origin,
                            (int)v55,
                            5u,
                            a34,
                            v60,
                            v61,
                            v62,
                            v63,
                            v64,
                            v65,
                            v66,
                            v67,
                            v68,
                            v69,
                            v70,
                            v71,
                            v72,
                            v73,
                            v74,
                            v75,
                            v76,
                            v77,
                            v78,
                            a36,
                            v79,
                            0,
                            v80,
                            0,
                            v81,
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

