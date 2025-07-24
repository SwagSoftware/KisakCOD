#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"
#include <qcommon/mem_track.h>
#include <script/scr_vm.h>
#include "g_utils_mp.h"
#include <server/sv_world.h>
#include <server_mp/server_mp.h>
#include <xanim/dobj.h>


// unsigned char *bulletPriorityMap  827b4d2c     g_combat_mp.obj
// unsigned char *riflePriorityMap   827b4d8c     g_combat_mp.obj
// unsigned short **modNames         827b4da0     g_combat_mp.obj
// float *g_fHitLocDamageMult 82cc4e00     g_combat_mp.obj


uint16_t g_HitLocConstNames[19]; // idb
float g_fHitLocDamageMult[19]; // idb
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
}; // idb

void __cdecl TRACK_g_combat()
{
    track_static_alloc_internal(g_fHitLocDamageMult, 76, "g_fHitLocDamageMult", 9);
    track_static_alloc_internal(g_HitLocNames, 76, "g_HitLocNames", 9);
    track_static_alloc_internal(g_HitLocConstNames, 38, "g_HitLocConstNames", 9);
}

void __cdecl G_ParseHitLocDmgTable()
{
    uint16_t prev; // ax
    char *pszBuffer; // [esp+4h] [ebp-20F4h]
    cspField_t pFieldList[19]; // [esp+8h] [ebp-20F0h] BYREF
    char loadBuffer[8192]; // [esp+F0h] [ebp-2008h] BYREF
    int32_t i; // [esp+20F4h] [ebp-4h]

    for (i = 0; i < 19; ++i)
    {
        g_fHitLocDamageMult[i] = 1.0;
        pFieldList[i].szName = g_HitLocNames[i];
        pFieldList[i].iOffset = 4 * i;
        pFieldList[i].iFieldType = 6;
        prev = Scr_AllocString((char *)g_HitLocNames[i], 1).prev;
        g_HitLocConstNames[i] = prev;
    }
    g_fHitLocDamageMult[18] = 0.0;
    pszBuffer = Com_LoadInfoString((char*)"info/mp_lochit_dmgtable", "hitloc damage table", "LOCDMGTABLE", loadBuffer);
    if (!ParseConfigStringToStruct(
        (uint8_t *)g_fHitLocDamageMult,
        pFieldList,
        19,
        pszBuffer,
        0,
        0,
        BG_StringCopy))
        Com_Error(ERR_DROP, "Error parsing hitloc damage table %s", "info/mp_lochit_dmgtable");
}

void __cdecl LookAtKiller(gentity_s *self, gentity_s *inflictor, gentity_s *attacker)
{
    float dir[3]; // [esp+0h] [ebp-18h] BYREF

    if (attacker && attacker != self)
    {
        Vec3Sub(attacker->r.currentOrigin, self->r.currentOrigin, dir);
    LABEL_10:
        self->client->ps.stats[1] = (int)vectoyaw(dir);
        vectoyaw(dir);
        return;
    }
    if (inflictor && inflictor != self)
    {
        Vec3Sub(inflictor->r.currentOrigin, self->r.currentOrigin, dir);
        goto LABEL_10;
    }
    if (!self->client)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 178, 0, "%s", "self->client");
    self->client->ps.stats[1] = (int)self->r.currentAngles[1];
}

int32_t __cdecl G_MeansOfDeathFromScriptParam(uint32_t scrParam)
{
    uint16_t modName; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    modName = Scr_GetConstString(scrParam);
    for (i = 0; i < 16; ++i)
    {
        if (*modNames[i] == modName)
            return i;
    }
    Scr_ParamError(scrParam, va("Unknown means of death \"%s\"\n", SL_ConvertToString(modName)));
    return 0;
}

void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int32_t damage,
    int32_t meansOfDeath,
    int32_t iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int32_t psTimeOffset)
{
    float *viewangles; // [esp+0h] [ebp-14h]
    gclient_s *client; // [esp+8h] [ebp-Ch]
    int32_t deathAnimDuration; // [esp+Ch] [ebp-8h]
    int32_t i; // [esp+10h] [ebp-4h]

    SV_CheckThread();
    if (!self->client)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 312, 0, "%s", "self->client");
    if (Com_GetServerDObj(self->client->ps.clientNum)
        && (self->client->ps.pm_type < PM_NOCLIP || self->client->ps.pm_type == PM_LASTSTAND)
        && (self->client->ps.otherFlags & 2) == 0)
    {
        if (bgs != &level_bgs)
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 328, 0, "%s\n\t(bgs) = %p", "(bgs == &level_bgs)", bgs);
        if (attacker->s.eType == 11 && attacker->r.ownerNum.isDefined())
            attacker = attacker->r.ownerNum.ent();
        DeathGrenadeDrop(self, meansOfDeath);
        Scr_AddEntity(attacker);
        Scr_Notify(self, scr_const.death, 1u);
        iassert((self->client->ps.pm_type == PM_NORMAL_LINKED || self->client->ps.pm_type == PM_NORMAL || self->client->ps.pm_type == PM_LASTSTAND));

        self->client->ps.pm_type = (self->client->ps.pm_type == PM_NORMAL_LINKED) ? PM_DEAD_LINKED : PM_DEAD;

        deathAnimDuration = BG_AnimScriptEvent(&self->client->ps, ANIM_ET_DEATH, 0, 1);
        self->client->ps.stats[0] = 0;
        Scr_PlayerKilled(
            self,
            inflictor,
            attacker,
            damage,
            meansOfDeath,
            iWeapon,
            vDir,
            hitLoc,
            psTimeOffset,
            deathAnimDuration);
        for (i = 0; i < level.maxclients; ++i)
        {
            client = &level.clients[i];
            if (client->sess.connected == CON_CONNECTED
                && client->sess.sessionState == SESS_STATE_SPECTATOR
                && client->spectatorClient == self->s.number)
            {
                Cmd_Score_f(&g_entities[i]);
            }
        }
        self->takedamage = 1;
        self->r.contents = 0x4000000;
        self->r.currentAngles[2] = 0.0;
        LookAtKiller(self, inflictor, attacker);
        viewangles = self->client->ps.viewangles;
        *viewangles = self->r.currentAngles[0];
        viewangles[1] = self->r.currentAngles[1];
        viewangles[2] = self->r.currentAngles[2];
        self->s.loopSound = 0;
        SV_UnlinkEntity(self);
        self->r.maxs[2] = 30.0;
        if (self->r.mins[2] > (double)self->r.maxs[2])
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 381, 0, "%s", "self->r.maxs[2] >= self->r.mins[2]");
        SV_LinkEntity(self);
        self->health = 0;
        self->handler = 12;
        if (bgs != &level_bgs)
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 389, 0, "%s\n\t(bgs) = %p", "(bgs == &level_bgs)", bgs);
    }
}

void __cdecl DeathGrenadeDrop(gentity_s *self, int32_t meansOfDeath)
{
    WeaponDef *weapDef; // [esp+14h] [ebp-20h]
    int32_t grenadeWeaponIndex; // [esp+18h] [ebp-1Ch]
    int32_t grenadeWeaponIndexa; // [esp+18h] [ebp-1Ch]
    float launchvel[3]; // [esp+1Ch] [ebp-18h] BYREF
    float launchspot[3]; // [esp+28h] [ebp-Ch] BYREF

    if (self->client->ps.grenadeTimeLeft)
    {
        if ((self->client->ps.weapFlags & 2) != 0)
            grenadeWeaponIndex = self->client->ps.offHandIndex;
        else
            grenadeWeaponIndex = self->client->ps.weapon;
        launchvel[0] = G_crandom();
        launchvel[1] = G_crandom();
        launchvel[2] = G_crandom();
        Vec3Scale(launchvel, 160.0, launchvel);
        launchspot[0] = self->r.currentOrigin[0];
        launchspot[1] = self->r.currentOrigin[1];
        launchspot[2] = self->r.currentOrigin[2];
        launchspot[2] = launchspot[2] + 40.0;
        G_FireGrenade(
            self,
            launchspot,
            launchvel,
            grenadeWeaponIndex,
            self->client->ps.weaponmodels[grenadeWeaponIndex],
            1,
            self->client->ps.grenadeTimeLeft);
    }
    if (meansOfDeath != 12 && (self->client->ps.perks & 0x40) != 0)
    {
        grenadeWeaponIndexa = BG_FindWeaponIndexForName(perk_grenadeDeath->current.string);
        if (grenadeWeaponIndexa)
        {
            weapDef = BG_GetWeaponDef(grenadeWeaponIndexa);
            launchvel[0] = G_crandom();
            launchvel[1] = G_crandom();
            launchvel[2] = G_crandom();
            Vec3Scale(launchvel, 160.0, launchvel);
            launchspot[0] = self->r.currentOrigin[0];
            launchspot[1] = self->r.currentOrigin[1];
            launchspot[2] = self->r.currentOrigin[2] + 40.0;
            G_FireGrenade(
                self,
                launchspot,
                launchvel,
                grenadeWeaponIndexa,
                self->client->ps.weaponmodels[grenadeWeaponIndexa],
                1,
                weapDef->fuseTime);
            BG_SetConditionValue(self->client->ps.clientNum, 9u, 2u);
        }
        else
        {
            Com_PrintWarning(14, "Unknown perk_grenadeDeath grenade: %s\n", perk_grenadeDeath->current.string);
        }
    }
}

double __cdecl G_GetWeaponHitLocationMultiplier(hitLocation_t hitLoc, uint32_t weapon)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ((uint32_t)hitLoc > HITLOC_GUN)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 396, 0, "%s", "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)");
    if (!weapon)
        return g_fHitLocDamageMult[hitLoc];
    weapDef = BG_GetWeaponDef(weapon);
    if (!weapDef || weapDef->weapType || weapDef->weapClass == WEAPCLASS_TURRET)
        return g_fHitLocDamageMult[hitLoc];
    else
        return weapDef->locationDamageMultipliers[hitLoc];
}

void __cdecl G_DamageClient(
    gentity_s *targ,
    gentity_s *inflictor,
    gentity_s *attacker,
    const float *dir,
    const float *point,
    int32_t damage,
    int32_t dflags,
    uint32_t mod,
    uint32_t weapon,
    hitLocation_t hitLoc,
    int32_t timeOffset)
{
    uint32_t NumWeapons; // eax

    if (targ->takedamage
        && damage > 0
        && !targ->client->noclip
        && !targ->client->ufo
        && targ->client->sess.connected == CON_CONNECTED
        && targ->client->ps.pm_type != PM_DEAD)
    {
        if (weapon == -1)
        {
            if (inflictor)
            {
                weapon = G_GetWeaponIndexForEntity(inflictor);
            }
            else if (attacker)
            {
                weapon = G_GetWeaponIndexForEntity(attacker);
            }
            else
            {
                weapon = 0;
            }
        }
        if (weapon >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            MyAssertHandler(
                ".\\game_mp\\g_combat_mp.cpp",
                486,
                0,
                "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                weapon,
                NumWeapons);
        }
        if ((uint32_t)hitLoc > HITLOC_GUN)
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 489, 0, "%s", "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)");
        if (mod != 7)
            damage = (int)(G_GetWeaponHitLocationMultiplier(hitLoc, weapon) * (double)damage);
        if (damage <= 0)
            damage = 1;
        Scr_PlayerDamage(targ, inflictor, attacker, damage, dflags, mod, weapon, point, dir, hitLoc, timeOffset);
    }
}

uint32_t __cdecl G_GetWeaponIndexForEntity(const gentity_s *ent)
{
    gclient_s *client; // [esp+8h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 415, 0, "%s", "ent");
    client = ent->client;
    if (!client)
        return ent->s.weapon;
    if ((client->ps.eFlags & 0x300) == 0 && (client->ps.pm_flags & 0x100000) == 0)
        return BG_GetViewmodelWeaponIndex(&client->ps);
    if (client->ps.viewlocked == PLAYERVIEWLOCK_NONE)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 429, 0, "%s", "ps->viewlocked");
    if (client->ps.viewlocked_entNum == 1023)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 430, 0, "%s", "ps->viewlocked_entNum != ENTITYNUM_NONE");
    return g_entities[client->ps.viewlocked_entNum].s.weapon;
}

void __cdecl G_Damage(
    gentity_s *targ,
    gentity_s *inflictor,
    gentity_s *attacker,
    float *dir,
    float *point,
    int32_t damage,
    int32_t dFlags,
    int32_t mod,
    uint32_t weapon,
    hitLocation_t hitLoc,
    uint32_t modelIndex,
    uint32_t partName,
    int32_t timeOffset)
{
    uint32_t NumWeapons; // eax
    float localdir[3]; // [esp+0h] [ebp-14h] BYREF
    void(__cdecl * die)(gentity_s *, gentity_s *, gentity_s *, int, int, const int, const float *, const hitLocation_t, int); // [esp+Ch] [ebp-8h]
    void(__cdecl * pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int); // [esp+10h] [ebp-4h]

    if (targ->client)
    {
        G_DamageClient(targ, inflictor, attacker, dir, point, damage, dFlags, mod, weapon, hitLoc, timeOffset);
    }
    else if (targ->takedamage)
    {
        if (!inflictor)
            inflictor = &g_entities[1022];
        if (!attacker)
            attacker = &g_entities[1022];
        if (weapon == -1)
            weapon = G_GetWeaponIndexForEntity(inflictor);
        if (weapon >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            MyAssertHandler(
                ".\\game_mp\\g_combat_mp.cpp",
                593,
                0,
                "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                weapon,
                NumWeapons);
        }
        if (!targ->scr_vehicle || !G_VehImmuneToDamage(targ, mod, dFlags, weapon))
        {
            if (!targ->r.inuse)
                MyAssertHandler(
                    ".\\game_mp\\g_combat_mp.cpp",
                    603,
                    0,
                    "%s\n\t(targ->s.number) = %i",
                    "(targ->r.inuse)",
                    targ->s.number);
            if (!attacker->r.inuse)
                MyAssertHandler(
                    ".\\game_mp\\g_combat_mp.cpp",
                    604,
                    0,
                    "%s\n\t(attacker->s.number) = %i",
                    "(attacker->r.inuse)",
                    attacker->s.number);
            Vec3NormalizeTo(dir, localdir);
            if ((targ->flags & 1) == 0)
            {
                if (damage < 1)
                    damage = 1;
                if ((targ->flags & 2) != 0 && targ->health - damage <= 0)
                    damage = targ->health - 1;
                if (g_debugDamage->current.enabled)
                    Com_Printf(15, "target:%i health:%i damage:%i\n", targ->s.number, targ->health, damage);
                targ->health -= damage;
                DamageNotify(scr_const.damage, targ, attacker, dir, point, damage, mod, dFlags, modelIndex, partName);
                if (targ->health > 0)
                {
                    pain = entityHandlers[targ->handler].pain;
                    if (pain)
                        pain(targ, attacker, damage, point, mod, localdir, hitLoc, weapon);
                }
                else
                {
                    if (targ->health < -999)
                        targ->health = -999;
                    Scr_AddEntity(attacker);
                    Scr_Notify(targ, scr_const.death, 1u);
                    die = entityHandlers[targ->handler].die;
                    if (die)
                        die(targ, inflictor, attacker, damage, mod, weapon, localdir, hitLoc, timeOffset);
                }
            }
        }
    }
}

void __cdecl DamageNotify(
    uint16_t notify,
    gentity_s *targ,
    gentity_s *attacker,
    float *dir,
    float *point,
    int32_t damage,
    int32_t mod,
    int32_t dFlags,
    uint32_t modelIndex,
    uint32_t partName)
{
    uint32_t modelName; // [esp+0h] [ebp-4h]

    Scr_AddInt(dFlags);
    if (partName)
        Scr_AddConstString(partName);
    else
        Scr_AddString((char *)"");
    if (modelIndex)
    {
        if (!targ->attachModelNames[modelIndex + 18])
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 515, 0, "%s", "targ->attachTagNames[modelIndex - 1]");
        modelName = SV_GetConfigstringConst(*((uint16_t *)&targ->tagChildren + modelIndex + 1) + 830);
        if (!modelName)
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 518, 1, "%s", "modelName");
        Scr_AddConstString(targ->attachModelNames[modelIndex + 18]);
        Scr_AddConstString(modelName);
    }
    else
    {
        Scr_AddString((char *)"");
        Scr_AddString((char *)"");
    }
    Scr_AddConstString(*modNames[mod]);
    if (point)
        Scr_AddVector(point);
    else
        Scr_AddVector((float *)vec3_origin);
    if (dir)
        Scr_AddVector(dir);
    else
        Scr_AddVector((float *)vec3_origin);
    Scr_AddEntity(attacker);
    Scr_AddInt(damage);
    Scr_Notify(targ, notify, 9u);
}

double __cdecl CanDamage(
    gentity_s *targ,
    gentity_s *inflictor,
    float *centerPos,
    float coneAngleCos,
    float *coneDirection,
    int32_t contentMask)
{
    float v20[3]; // [esp+A0h] [ebp-108h] BYREF
    float a[3]; // [esp+ACh] [ebp-FCh] BYREF
    char v22; // [esp+BBh] [ebp-EDh]
    float *v23; // [esp+BCh] [ebp-ECh]
    float centerToCorner[3]; // [esp+C0h] [ebp-E8h] BYREF
    DObj_s *obj; // [esp+CCh] [ebp-DCh]
    float radiusUp; // [esp+D0h] [ebp-D8h]
    float absMaxs[3]; // [esp+D4h] [ebp-D4h] BYREF
    float v1[3]; // [esp+E0h] [ebp-C8h] BYREF
    float v[3]; // [esp+ECh] [ebp-BCh] BYREF
    float up[3]; // [esp+F8h] [ebp-B0h] BYREF
    float radiusRight; // [esp+104h] [ebp-A4h]
    float absMins[3]; // [esp+108h] [ebp-A0h] BYREF
    float diff[3]; // [esp+114h] [ebp-94h] BYREF
    float dir[3]; // [esp+120h] [ebp-88h] BYREF
    bool success; // [esp+12Fh] [ebp-79h]
    const float *color; // [esp+130h] [ebp-78h]
    float halfWidth; // [esp+134h] [ebp-74h]
    float right[3]; // [esp+138h] [ebp-70h] BYREF
    float forward[3]; // [esp+144h] [ebp-64h] BYREF
    float eyeOrigin[3]; // [esp+150h] [ebp-58h] BYREF
    float halfHeight; // [esp+15Ch] [ebp-4Ch]
    int32_t hits; // [esp+160h] [ebp-48h]
    int32_t inflictorNum; // [esp+164h] [ebp-44h]
    float dest[5][3]; // [esp+168h] [ebp-40h] BYREF
    int32_t i; // [esp+1A4h] [ebp-4h]

    if (inflictor)
        inflictorNum = inflictor->s.number;
    else
        inflictorNum = 1023;
    if (targ->client)
    {
        halfWidth = 15.0;
        G_GetPlayerViewOrigin(&targ->client->ps, eyeOrigin);
        halfHeight = (eyeOrigin[2] - targ->r.currentOrigin[2]) * 0.5;
        Vec3Sub(centerPos, targ->r.currentOrigin, forward);
        forward[2] = 0.0;
        Vec3Normalize(forward);
        right[0] = -forward[1];
        right[1] = forward[0];
        right[2] = forward[2];
        Vec3Add(eyeOrigin, targ->r.currentOrigin, dest[0]);
        Vec3Scale(dest[0], 0.5, dest[0]);
        Vec3Mad(dest[0], 15.0, right, dest[1]);
        dest[1][2] = dest[1][2] + halfHeight;
        Vec3Mad(dest[0], 15.0, right, dest[2]);
        dest[2][2] = dest[2][2] - halfHeight;
        Vec3Mad(dest[0], -15.0f, right, dest[3]);
        dest[3][2] = dest[3][2] + halfHeight;
        Vec3Mad(dest[0], -15.0f, right, dest[4]);
        dest[4][2] = dest[4][2] - halfHeight;
        if (radius_damage_debug->current.enabled)
        {
            for (i = 0; i < 5; ++i)
            {
                success = 1;
                color = colorWhite;
                if (coneAngleCos != -1.0)
                {
                    if (coneDirection)
                    {
                        Vec3Sub(dest[i], centerPos, dir);
                        Vec3Normalize(dir);
                        if (coneAngleCos > Vec3Dot(dir, coneDirection))
                        {
                            success = 0;
                            color = colorOrange;
                        }
                    }
                }
                if (success && !G_LocationalTracePassed(centerPos, dest[i], targ->s.number, inflictorNum, contentMask, 0))
                    color = colorRed;
                G_DebugLineWithDuration(centerPos, dest[i], color, 1, 300);
            }
        }
        hits = 0;
        for (i = 0; i < 5; ++i)
        {
            if (coneAngleCos != -1.0)
            {
                if (coneDirection)
                {
                    Vec3Sub(dest[i], centerPos, diff);
                    Vec3Normalize(diff);
                    if (coneAngleCos > Vec3Dot(diff, coneDirection))
                        continue;
                }
            }
            if (G_LocationalTracePassed(centerPos, dest[i], targ->s.number, inflictorNum, contentMask, 0))
                ++hits;
        }
        if (hits)
        {
            if (hits <= 3)
                return (float)((double)hits / 3.0);
            else
                return 1.0;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        if (targ->classname == scr_const.script_model && targ->model)
        {
            obj = Com_GetServerDObj(targ->s.number);
            DObjPhysicsGetBounds(obj, absMins, absMaxs);
            Vec3Add(targ->r.currentOrigin, absMins, absMins);
            Vec3Add(targ->r.currentOrigin, absMaxs, absMaxs);
        }
        else
        {
            absMins[0] = targ->r.absmin[0];
            absMins[1] = targ->r.absmin[1];
            absMins[2] = targ->r.absmin[2];

            absMaxs[0] = targ->r.absmax[0];
            absMaxs[1] = targ->r.absmax[1];
            absMaxs[2] = targ->r.absmax[2];
        }
        Vec3Add(absMins, absMaxs, dest[0]);
        Vec3Scale(dest[0], 0.5, dest[0]);
        Vec3Sub(centerPos, dest[0], v);
        Vec3Normalize(v);
        v1[0] = -v[1];
        v1[1] = v[0];
        v1[2] = 0.0;
        Vec3Normalize(v1);
        Vec3Cross(v, v1, up);
        Vec3Sub(absMaxs, dest[0], centerToCorner);
        radiusRight = I_fabs((centerToCorner[0] * v1[0])) + I_fabs((centerToCorner[1] * v1[1]));
        radiusUp = I_fabs((centerToCorner[0] * up[0])) + I_fabs((centerToCorner[1] * up[1])) + I_fabs((centerToCorner[2] * up[2]));
        Vec3Scale(v1, radiusRight, v1);
        Vec3Scale(up, radiusUp, up);
        Vec3Add(dest[0], v1, dest[1]);
        Vec3Add(dest[1], up, dest[1]);
        Vec3Mad(dest[0], -1.0, v1, dest[2]);
        Vec3Add(dest[2], up, dest[2]);
        Vec3Add(dest[0], v1, dest[3]);
        Vec3Mad(dest[3], -1.0, up, dest[3]);
        Vec3Mad(dest[0], -1.0, v1, dest[4]);
        Vec3Mad(dest[4], -1.0, up, dest[4]);
        if (radius_damage_debug->current.enabled)
        {
            for (i = 0; i < 5; ++i)
            {
                v22 = 1;
                v23 = (float *)colorWhite;
                if (coneAngleCos != -1.0)
                {
                    if (coneDirection)
                    {
                        Vec3Sub(dest[i], centerPos, a);
                        Vec3Normalize(a);
                        if (coneAngleCos > Vec3Dot(a, coneDirection))
                        {
                            v22 = 0;
                            v23 = (float *)colorOrange;
                        }
                    }
                }
                if (v22 && !G_LocationalTracePassed(centerPos, dest[i], targ->s.number, inflictorNum, contentMask, 0))
                    v23 = (float *)colorRed;
                G_DebugLineWithDuration(centerPos, dest[i], v23, 1, 300);
            }
        }
        for (i = 0; i < 5; ++i)
        {
            if (coneAngleCos != -1.0)
            {
                if (coneDirection)
                {
                    Vec3Sub(dest[i], centerPos, v20);
                    Vec3Normalize(v20);
                    if (coneAngleCos > Vec3Dot(v20, coneDirection))
                        continue;
                }
            }
            if (G_LocationalTracePassed(centerPos, dest[i], targ->s.number, inflictorNum, contentMask, 0))
                return 1.0;
        }
        return 0.0;
    }
}

void __cdecl G_FlashbangBlast(float *origin, float radius_max, float radius_min, gentity_s *attacker, team_t team)
{
    int32_t i; // [esp+7Ch] [ebp-100Ch]
    int32_t entList[1024]; // [esp+80h] [ebp-1008h] BYREF
    gentity_s *ent; // [esp+1080h] [ebp-8h]
    int32_t entListCount; // [esp+1084h] [ebp-4h] BYREF

    if (radius_min < 1.0)
        radius_min = 1.0;
    if (radius_min > (double)radius_max)
        radius_max = radius_min;
    GetEntListForRadius(origin, radius_max, radius_min, entList, &entListCount);
    for (i = 0; i < entListCount; ++i)
    {
        ent = &g_entities[entList[i]];
        if (!ent)
            MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 1014, 0, "%s", "ent");
        FlashbangBlastEnt(ent, origin, radius_max, radius_min, attacker, team);
    }
}

void __cdecl GetEntListForRadius(
    const float *origin,
    float radius_max,
    float radius_min,
    int32_t *entList,
    int32_t *entListCount)
{
    float mins[3]; // [esp+0h] [ebp-20h] BYREF
    float boxradius; // [esp+Ch] [ebp-14h]
    float maxs[3]; // [esp+10h] [ebp-10h] BYREF
    int32_t i; // [esp+1Ch] [ebp-4h]

    boxradius = radius_max * 1.414213538169861;
    for (i = 0; i < 3; ++i)
    {
        mins[i] = origin[i] - boxradius;
        maxs[i] = origin[i] + boxradius;
    }
    *entListCount = CM_AreaEntities(mins, maxs, entList, 1024, -1);
}

void __cdecl FlashbangBlastEnt(
    gentity_s *ent,
    float *blastOrigin,
    float radius_max,
    float radius_min,
    gentity_s *attacker,
    team_t team)
{
    gclient_s *client; // eax
    float percent_distance; // [esp+40h] [ebp-34h]
    float percent_angle; // [esp+44h] [ebp-30h]
    float forward[3]; // [esp+48h] [ebp-2Ch] BYREF
    float viewOrigin[3]; // [esp+54h] [ebp-20h] BYREF
    float toBlast[3]; // [esp+60h] [ebp-14h] BYREF
    float dist; // [esp+6Ch] [ebp-8h]
    float damageScale; // [esp+70h] [ebp-4h]

    if (ent->client)
    {
        if (ent->takedamage)
        {
            dist = EntDistToPoint(blastOrigin, ent);
            if (radius_max >= (double)dist)
            {
                damageScale = CanDamage(ent, attacker, blastOrigin, 1.0, 0, 2049);
                if (damageScale > 0.0)
                {
                    if (radius_min < (double)dist)
                        percent_distance = 1.0 - (dist - radius_min) / (radius_max - radius_min);
                    else
                        percent_distance = 1.0;
                    AngleVectors(ent->client->ps.viewangles, forward, 0, 0);
                    client = ent->client;
                    viewOrigin[0] = client->ps.origin[0];
                    viewOrigin[1] = client->ps.origin[1];
                    viewOrigin[2] = client->ps.origin[2];
                    viewOrigin[2] = viewOrigin[2] + ent->client->ps.viewHeightCurrent;
                    Vec3Sub(blastOrigin, viewOrigin, toBlast);
                    Vec3NormalizeFast(toBlast);
                    percent_angle = (Vec3Dot(forward, toBlast) + 1.0) * 0.5;
                    AddScrTeamName(team);
                    if (attacker)
                        Scr_AddEntity(attacker);
                    else
                        Scr_AddUndefined();
                    Scr_AddFloat(percent_angle);
                    Scr_AddFloat(percent_distance);
                    Scr_Notify(ent, scr_const.flashbang, 4u);
                }
            }
        }
    }
}

double __cdecl EntDistToPoint(const float *origin, gentity_s *ent)
{
    uint32_t i; // [esp+8h] [ebp-10h]
    float v[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (ent->r.bmodel)
    {
        for (i = 0; i < 3; ++i)
        {
            if (ent->r.absmin[i] <= (double)origin[i])
            {
                if (ent->r.absmax[i] >= (double)origin[i])
                    v[i] = 0.0;
                else
                    v[i] = origin[i] - ent->r.absmax[i];
            }
            else
            {
                v[i] = ent->r.absmin[i] - origin[i];
            }
        }
        return Vec3Length(v);
    }
    else
    {
        Vec3Sub(ent->r.currentOrigin, origin, v);
        return Vec3Length(v);
    }
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
    case TEAM_SPECTATOR:
        Scr_AddConstString(scr_const.spectator);
        break;
    default:
        Com_PrintWarning(15, "AddScrTeamName(): Unhandled team name %i.\n", team);
        Scr_AddUndefined();
        break;
    }
}

bool __cdecl G_WithinDamageRadius(const float *damageOrigin, float radiusSquared, gentity_s *ent)
{
    float distSqrd; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 1061, 0, "%s", "ent");
    distSqrd = G_GetRadiusDamageDistanceSquared(damageOrigin, ent);
    return radiusSquared > (double)distSqrd;
}

double __cdecl G_GetRadiusDamageDistanceSquared(const float *damageOrigin, gentity_s *ent)
{
    int32_t i; // [esp+0h] [ebp-10h]
    float v[3]; // [esp+4h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 1029, 0, "%s", "ent");
    if (ent->r.bmodel)
    {
        for (i = 0; i < 3; ++i)
        {
            if (ent->r.absmin[i] <= (double)damageOrigin[i])
            {
                if (ent->r.absmax[i] >= (double)damageOrigin[i])
                    v[i] = 0.0;
                else
                    v[i] = damageOrigin[i] - ent->r.absmax[i];
            }
            else
            {
                v[i] = ent->r.absmin[i] - damageOrigin[i];
            }
        }
    }
    else
    {
        Vec3Sub(ent->r.currentOrigin, damageOrigin, v);
    }
    return Vec3LengthSq(v);
}

int32_t __cdecl G_RadiusDamage(
    float *origin,
    gentity_s *inflictor,
    gentity_s *attacker,
    float fInnerDamage,
    float fOuterDamage,
    float radius,
    float coneAngleCos,
    float *coneDirection,
    gentity_s *ignore,
    int32_t mod,
    uint32_t weapon)
{
    float v12; // [esp+Ch] [ebp-1058h]
    int32_t j; // [esp+10h] [ebp-1054h]
    float v14; // [esp+14h] [ebp-1050h]
    float diff[3]; // [esp+18h] [ebp-104Ch] BYREF
    float v16; // [esp+24h] [ebp-1040h]
    float mins[3]; // [esp+28h] [ebp-103Ch] BYREF
    float v18; // [esp+34h] [ebp-1030h]
    float v19; // [esp+38h] [ebp-102Ch]
    float RadiusDamageDistanceSquared; // [esp+3Ch] [ebp-1028h]
    int32_t v21; // [esp+40h] [ebp-1024h]
    float maxs[3]; // [esp+44h] [ebp-1020h] BYREF
    float v23; // [esp+50h] [ebp-1014h]
    int32_t entityList[1025]; // [esp+54h] [ebp-1010h] BYREF
    gentity_s *ent; // [esp+1058h] [ebp-Ch]
    int32_t i; // [esp+105Ch] [ebp-8h]
    int32_t v27; // [esp+1060h] [ebp-4h]

    v27 = 0;
    if (!attacker)
        return 0;
    if (radius < 1.0)
        radius = 1.0;
    v18 = radius * radius;
    v19 = radius * 1.414213538169861;
    for (i = 0; i < 3; ++i)
    {
        mins[i] = origin[i] - v19;
        maxs[i] = origin[i] + v19;
    }
    v21 = CM_AreaEntities(mins, maxs, entityList, 1024, -1);
    for (j = 0; j < v21; ++j)
    {
        ent = &g_entities[entityList[j]];
        if (ent != ignore && ent->takedamage && (!ent->client || !level.bPlayerIgnoreRadiusDamage))
        {
            RadiusDamageDistanceSquared = G_GetRadiusDamageDistanceSquared(origin, ent);
            if (v18 > RadiusDamageDistanceSquared)
            {
                v12 = sqrt(RadiusDamageDistanceSquared);
                v16 = v12;
                v23 = CanDamage(ent, inflictor, origin, coneAngleCos, coneDirection, 0x802011);
                if (v23 > 0.0)
                {
                    if (LogAccuracyHit(ent, attacker))
                        v27 = 1;
                    Vec3Sub(ent->r.currentOrigin, origin, diff);
                    diff[2] = diff[2] + 24.0;
                    v14 = (fInnerDamage - fOuterDamage) * (1.0 - v12 / radius) + fOuterDamage;
                    G_Damage(ent, inflictor, attacker, diff, origin, (v14 * v23), 5, mod, weapon, HITLOC_NONE, 0, 0, 0);
                }
            }
        }
    }
    return v27;
}

uint16_t __cdecl G_GetHitLocationString(hitLocation_t hitLoc)
{
    if ((uint32_t)hitLoc >= HITLOC_NUM)
        MyAssertHandler(".\\game_mp\\g_combat_mp.cpp", 1165, 0, "%s", "(unsigned)hitLoc < HITLOC_NUM");
    return g_HitLocConstNames[hitLoc];
}

int32_t __cdecl G_GetHitLocationIndexFromString(uint16_t sString)
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 19; ++i)
    {
        if (g_HitLocConstNames[i] == sString)
            return i;
    }
    return 0;
}

