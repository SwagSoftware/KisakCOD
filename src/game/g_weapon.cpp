#include "game_public.h"
#include <server/sv_world.h>
#include <DynEntity/DynEntity_client.h>
#include <script/scr_const.h>
#include <server/sv_game.h>
#include "bullet.h"

#ifdef KISAK_MP
#include <game_mp/g_utils_mp.h>
#include <server_mp/server_mp.h>
#elif KISAK_SP
#include "g_main.h"
#include "g_local.h"
#endif


#ifdef KISAK_MP
void __cdecl G_AntiLagRewindClientPos(int gameTime, AntilagClientStore *antilagStore)
{
    const char *v2; // eax
    float *currentOrigin; // edx
    float *v4; // eax
    const char *v5; // eax
    float *v6; // edx
    int client; // [esp+34h] [ebp-14h]
    float clientPosition[4]; // [esp+38h] [ebp-10h] BYREF

    if (!antilagStore)
        MyAssertHandler(".\\game\\g_weapon.cpp", 30, 0, "%s", "antilagStore");
    if (g_antilag->current.enabled)
    {
        memset((unsigned __int8 *)antilagStore, 0, sizeof(AntilagClientStore));
        if (gameTime <= 0)
            MyAssertHandler(".\\game\\g_weapon.cpp", 36, 0, "%s", "gameTime > 0");
        if (level.time - gameTime <= 400 && level.time - gameTime > 1000 / sv_fps->current.integer)
        {
            for (client = 0; client < level.maxclients; ++client)
            {
                if (level.clients[client].sess.connected == CON_CONNECTED
                    && level.clients[client].sess.sessionState == SESS_STATE_PLAYING
                    && SV_GetClientPositionAtTime(client, gameTime, clientPosition))
                {
                    if ((LODWORD(clientPosition[0]) & 0x7F800000) == 0x7F800000
                        || (LODWORD(clientPosition[1]) & 0x7F800000) == 0x7F800000
                        || (LODWORD(clientPosition[2]) & 0x7F800000) == 0x7F800000)
                    {
                        v2 = va(
                            "client %i's antilag origin is invalid - (%f, %f, %f)",
                            client,
                            clientPosition[0],
                            clientPosition[1],
                            clientPosition[2]);
                        MyAssertHandler(
                            ".\\game\\g_weapon.cpp",
                            51,
                            0,
                            "%s\n\t%s",
                            "!IS_NAN(clientPosition[0]) && !IS_NAN(clientPosition[1]) && !IS_NAN(clientPosition[2])",
                            v2);
                    }
                    LODWORD(clientPosition[3]) = gameTime;
                    currentOrigin = g_entities[client].r.currentOrigin;
                    v4 = antilagStore->realClientPositions[client];
                    *v4 = *currentOrigin;
                    v4[1] = currentOrigin[1];
                    v4[2] = currentOrigin[2];
                    if ((COERCE_UNSIGNED_INT(*v4) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(antilagStore->realClientPositions[client][1]) & 0x7F800000) == 0x7F800000
                        || (COERCE_UNSIGNED_INT(antilagStore->realClientPositions[client][2]) & 0x7F800000) == 0x7F800000)
                    {
                        v5 = va(
                            "client %i's origin is invalid - (%f, %f, %f)",
                            client,
                            antilagStore->realClientPositions[client][0],
                            antilagStore->realClientPositions[client][1],
                            antilagStore->realClientPositions[client][2]);
                        MyAssertHandler(
                            ".\\game\\g_weapon.cpp",
                            58,
                            0,
                            "%s\n\t%s",
                            "!IS_NAN( antilagStore->realClientPositions[client][0]) && !IS_NAN(antilagStore->realClientPositions[client"
                            "][1]) && !IS_NAN(antilagStore->realClientPositions[client][2])",
                            v5);
                    }
                    SV_UnlinkEntity(&g_entities[client]);
                    v6 = g_entities[client].r.currentOrigin;
                    *v6 = clientPosition[0];
                    v6[1] = clientPosition[1];
                    v6[2] = clientPosition[2];
                    SV_LinkEntity(&g_entities[client]);
                    antilagStore->clientMoved[client] = 1;
                }
            }
        }
    }
}


void __cdecl G_AntiLag_RestoreClientPos(AntilagClientStore *antilagStore)
{
    const char *v1; // eax
    float *v2; // edx
    float *currentOrigin; // eax
    int client; // [esp+24h] [ebp-4h]

    if (!antilagStore)
        MyAssertHandler(".\\game\\g_weapon.cpp", 80, 0, "%s", "antilagStore");
    if (g_antilag->current.enabled)
    {
        for (client = 0; client < level.maxclients; ++client)
        {
            if (antilagStore->clientMoved[client])
            {
                SV_UnlinkEntity(&g_entities[client]);
                if ((COERCE_UNSIGNED_INT(antilagStore->realClientPositions[client][0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(antilagStore->realClientPositions[client][1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(antilagStore->realClientPositions[client][2]) & 0x7F800000) == 0x7F800000)
                {
                    v1 = va(
                        "client %i's origin is invalid - (%f, %f, %f)",
                        client,
                        antilagStore->realClientPositions[client][0],
                        antilagStore->realClientPositions[client][1],
                        antilagStore->realClientPositions[client][2]);
                    MyAssertHandler(
                        ".\\game\\g_weapon.cpp",
                        92,
                        0,
                        "%s\n\t%s",
                        "!IS_NAN(antilagStore->realClientPositions[client][0]) && !IS_NAN(antilagStore->realClientPositions[client][1"
                        "]) && !IS_NAN(antilagStore->realClientPositions[client][2])",
                        v1);
                }
                v2 = antilagStore->realClientPositions[client];
                currentOrigin = g_entities[client].r.currentOrigin;
                *currentOrigin = *v2;
                currentOrigin[1] = v2[1];
                currentOrigin[2] = v2[2];
                SV_LinkEntity(&g_entities[client]);
            }
        }
    }
}
#endif

gentity_s *__cdecl Weapon_Melee(gentity_s *ent, weaponParms *wp, float range, float width, float height, int gametime)
{
    AntilagClientStore antilagClients; // [esp+Ch] [ebp-348h] BYREF
    gentity_s *traceEnt; // [esp+350h] [ebp-4h]

    G_AntiLagRewindClientPos(gametime, &antilagClients);
    traceEnt = Weapon_Melee_internal(ent, wp, range, width, height);
    G_AntiLag_RestoreClientPos(&antilagClients);
    return traceEnt;
}

gentity_s *__cdecl Weapon_Melee_internal(gentity_s *ent, weaponParms *wp, float range, float width, float height)
{
    int v6; // eax
    hitLocation_t partGroup; // [esp+4h] [ebp-5Ch]
    unsigned int modelIndex; // [esp+8h] [ebp-58h]
    unsigned int partName; // [esp+Ch] [ebp-54h]
    bool v10; // [esp+14h] [ebp-4Ch]
    int damage; // [esp+18h] [ebp-48h]
    gentity_s *tent; // [esp+1Ch] [ebp-44h]
    float endpos[3]; // [esp+20h] [ebp-40h] BYREF
    trace_t tr; // [esp+2Ch] [ebp-34h] BYREF
    unsigned __int16 hitEntId; // [esp+58h] [ebp-8h]
    gentity_s *traceEnt; // [esp+5Ch] [ebp-4h]

    if (!wp)
        MyAssertHandler(".\\game\\g_weapon.cpp", 190, 0, "%s", "wp");
    if (!wp->weapDef)
        MyAssertHandler(".\\game\\g_weapon.cpp", 191, 0, "%s", "wp->weapDef");
    damage = wp->weapDef->iMeleeDamage;
    if (!Melee_Trace(ent, wp, damage, range, width, height, &tr, endpos))
        return 0;
    hitEntId = Trace_GetEntityHitId(&tr);
    traceEnt = &g_entities[hitEntId];
    if (ent->client && traceEnt->client)
        G_AddEvent(ent, 0x25u, 0);
    if (traceEnt->client)
        tent = G_TempEntity(endpos, 35);
    else
        tent = G_TempEntity(endpos, 36);
    tent->s.otherEntityNum = traceEnt->s.number;
    v10 = wp->weapDef->knifeModel && ent->client;
    tent->s.eventParm = v10;
    tent->s.weapon = ent->s.weapon;
    tent->s.weaponModel = ent->s.weaponModel;
    if (traceEnt->s.number == 1022)
        return 0;
    if (!traceEnt->takedamage)
        return 0;
    partName = tr.partName;
    modelIndex = tr.modelIndex;
    partGroup = (hitLocation_t)tr.partGroup;
    v6 = G_rand();
    G_Damage(
        traceEnt,
        ent,
        ent,
        wp->forward,
        endpos,
        damage + v6 % 5,
        0,
        7,
        0xFFFFFFFF,
        partGroup,
        modelIndex,
        partName
#ifdef KISAK_MP
        , 0);
#elif KISAK_SP
        );
#endif
    return traceEnt;
}

char __cdecl Melee_Trace(
    gentity_s *ent,
    weaponParms *wp,
    int damage,
    float range,
    float width,
    float height,
    trace_t *trace,
    float *endPos)
{
    float v9; // [esp+Ch] [ebp-3Ch]
    float v10; // [esp+10h] [ebp-38h]
    float v11; // [esp+14h] [ebp-34h]
    float v12; // [esp+18h] [ebp-30h]
    float v13; // [esp+1Ch] [ebp-2Ch]
    float scale; // [esp+20h] [ebp-28h]
    int v15; // [esp+24h] [ebp-24h]
    float end[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float start[3]; // [esp+38h] [ebp-10h] BYREF
    int traceIndex; // [esp+44h] [ebp-4h]

    if (width > 0.0 || height > 0.0)
        v15 = 5;
    else
        v15 = 1;
    for (traceIndex = 0; traceIndex < v15; ++traceIndex)
    {
        Vec3Mad(wp->muzzleTrace, range, wp->forward, end);
        scale = width * (float)traceOffsets[traceIndex][0];
        Vec3Mad(end, scale, wp->right, end);
        v13 = height * (float)traceOffsets[traceIndex][1];
        Vec3Mad(end, v13, wp->up, end);
        if (melee_debug->current.enabled)
            G_DebugLineWithDuration(wp->muzzleTrace, end, colorRed, 1, 200);
        G_LocationalTrace(trace, wp->muzzleTrace, end, ent->s.number, 0x2806891, bulletPriorityMap);
        Vec3Lerp(wp->muzzleTrace, end, trace->fraction, endPos);
        if (!traceIndex)
            G_CheckHitTriggerDamage(ent, wp->muzzleTrace, endPos, damage, 7u);
        if ((trace->surfaceFlags & 0x10) == 0 && trace->fraction != 1.0)
        {
            if (melee_debug->current.enabled)
                G_DebugLineWithDuration(wp->muzzleTrace, endPos, colorGreen, 1, 200);
            return 1;
        }
    }
    for (traceIndex = 1; traceIndex + 1 < v15; traceIndex += 2)
    {
        Vec3Mad(wp->muzzleTrace, range, wp->forward, end);
        v12 = width * (float)traceOffsets[traceIndex][0];
        Vec3Mad(end, v12, wp->right, start);
        v11 = height * (float)traceOffsets[traceIndex][1];
        Vec3Mad(start, v11, wp->up, start);
        v10 = width * (float)traceOffsets[traceIndex + 1][0];
        Vec3Mad(end, v10, wp->right, end);
        v9 = height * (float)traceOffsets[traceIndex + 1][1];
        Vec3Mad(end, v9, wp->up, end);
        if (melee_debug->current.enabled)
            G_DebugLineWithDuration(start, end, colorRed, 1, 200);
        G_LocationalTrace(trace, start, end, ent->s.number, 0x2806891, bulletPriorityMap);
        Vec3Lerp(start, end, trace->fraction, endPos);
        if ((trace->surfaceFlags & 0x10) == 0 && !trace->startsolid && trace->fraction != 1.0)
            return 1;
    }
    return 0;
}

gentity_s *__cdecl Weapon_Throw_Grenade(
    gentity_s *ent,
    unsigned int grenType,
    unsigned __int8 grenModel,
    weaponParms *wp)
{
    float scale; // [esp+4h] [ebp-50h]
    int fuseTime; // [esp+Ch] [ebp-48h]
    float iProjectileSpeedForward; // [esp+28h] [ebp-2Ch]
    gentity_s *m; // [esp+34h] [ebp-20h]
    float fAddVel; // [esp+38h] [ebp-1Ch]
    float vTossVel[3]; // [esp+3Ch] [ebp-18h] BYREF
    float forwardHoriz[3]; // [esp+48h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_weapon.cpp", 302, 0, "%s", "ent");
    if (!wp)
        MyAssertHandler(".\\game\\g_weapon.cpp", 303, 0, "%s", "wp");
    scale = (float)wp->weapDef->iProjectileSpeed;
    Vec3Scale(wp->forward, scale, vTossVel);
    vTossVel[2] = (double)wp->weapDef->iProjectileSpeedUp + vTossVel[2];
    if (wp->weapDef->iProjectileSpeedForward)
    {
        forwardHoriz[0] = wp->forward[0];
        forwardHoriz[1] = wp->forward[1];
        Vec2NormalizeFast(forwardHoriz);
        iProjectileSpeedForward = (float)wp->weapDef->iProjectileSpeedForward;
        vTossVel[0] = iProjectileSpeedForward * forwardHoriz[0] + vTossVel[0];
        vTossVel[1] = iProjectileSpeedForward * forwardHoriz[1] + vTossVel[1];
    }
    if (ent->client)
        fuseTime = wp->weapDef->fuseTime;
    else
        fuseTime = wp->weapDef->aiFuseTime;
    LODWORD(forwardHoriz[2]) = fuseTime;
    m = G_FireGrenade(ent, wp->muzzleTrace, vTossVel, grenType, grenModel, 1, fuseTime);
    Vec3Normalize(vTossVel);
    fAddVel = Vec3Dot(ent->client->ps.velocity, vTossVel);
    Vec3Mad(m->s.lerp.pos.trDelta, fAddVel, vTossVel, m->s.lerp.pos.trDelta);
    if ((COERCE_UNSIGNED_INT(m->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(m->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(m->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_weapon.cpp",
            322,
            0,
            "%s",
            "!IS_NAN((m->s.lerp.pos.trDelta)[0]) && !IS_NAN((m->s.lerp.pos.trDelta)[1]) && !IS_NAN((m->s.lerp.pos.trDelta)[2])");
    }
    return m;
}

void __cdecl Vec2NormalizeFast(float *v)
{
    int number; // [esp+18h] [ebp-8h]
    float invLength; // [esp+1Ch] [ebp-4h]

    *(float *)&number = *v * *v + v[1] * v[1];
    invLength = I_rsqrt(number);
    *v = *v * invLength;
    v[1] = v[1] * invLength;
}

gentity_s *__cdecl Weapon_GrenadeLauncher_Fire(
    gentity_s *ent,
    unsigned int grenType,
    unsigned __int8 grenModel,
    weaponParms *wp)
{
    float scale; // [esp+4h] [ebp-34h]
    gentity_s *m; // [esp+24h] [ebp-14h]
    float fAddVel; // [esp+28h] [ebp-10h]
    float vTossVel[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_weapon.cpp", 334, 0, "%s", "ent");
    if (!wp)
        MyAssertHandler(".\\game\\g_weapon.cpp", 335, 0, "%s", "wp");
    scale = (float)wp->weapDef->iProjectileSpeed;
    Vec3Scale(wp->forward, scale, vTossVel);
    vTossVel[2] = (double)wp->weapDef->iProjectileSpeedUp + vTossVel[2];
    m = G_FireGrenade(ent, wp->muzzleTrace, vTossVel, grenType, grenModel, 0, 0);
    m->flags |= 0x20000u;
    Vec3Normalize(vTossVel);
    fAddVel = Vec3Dot(ent->client->ps.velocity, vTossVel);
    Vec3Mad(m->s.lerp.pos.trDelta, fAddVel, vTossVel, m->s.lerp.pos.trDelta);
    if ((COERCE_UNSIGNED_INT(m->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(m->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(m->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_weapon.cpp",
            347,
            0,
            "%s",
            "!IS_NAN((m->s.lerp.pos.trDelta)[0]) && !IS_NAN((m->s.lerp.pos.trDelta)[1]) && !IS_NAN((m->s.lerp.pos.trDelta)[2])");
    }
    return m;
}

gentity_s *__cdecl Weapon_RocketLauncher_Fire(
    gentity_s *ent,
    unsigned int weaponIndex,
    float spread,
    weaponParms *wp,
    const float *gunVel,
    gentity_s *target,
    const float *targetOffset)
{
    float v8; // [esp+Ch] [ebp-44h]
    float v9; // [esp+10h] [ebp-40h]
    float fAimOffset; // [esp+28h] [ebp-28h]
    gentity_s *m; // [esp+2Ch] [ebp-24h]
    float r; // [esp+30h] [ebp-20h] BYREF
    float dir[3]; // [esp+34h] [ebp-1Ch] BYREF
    float launchpos[3]; // [esp+40h] [ebp-10h] BYREF
    float u; // [esp+4Ch] [ebp-4h] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_weapon.cpp", 362, 0, "%s", "ent");
    if (!wp)
        MyAssertHandler(".\\game\\g_weapon.cpp", 363, 0, "%s", "wp");
    v9 = spread * 0.01745329238474369;
    v8 = tan(v9);
    fAimOffset = v8 * 16.0;
    gunrandom(&r, &u);
    r = r * fAimOffset;
    u = u * fAimOffset;
    Vec3Scale(wp->forward, 16.0, dir);
    Vec3Mad(dir, r, wp->right, dir);
    Vec3Mad(dir, u, wp->up, dir);
    Vec3Normalize(dir);
    launchpos[0] = wp->muzzleTrace[0];
    launchpos[1] = wp->muzzleTrace[1];
    launchpos[2] = wp->muzzleTrace[2];
    m = G_FireRocket(ent, weaponIndex, launchpos, dir, gunVel, target, targetOffset);
    if (ent->client)
        Vec3Mad(ent->client->ps.velocity, -64.0, wp->forward, ent->client->ps.velocity);
    return m;
}

void __cdecl gunrandom(float *x, float *y)
{
    float v2; // [esp+8h] [ebp-14h]
    float sinT; // [esp+Ch] [ebp-10h]
    float theta; // [esp+10h] [ebp-Ch]
    float r; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    theta = G_random() * 360.0;
    r = G_random();
    v2 = theta * 0.01745329238474369;
    cosT = cos(v2);
    sinT = sin(v2);
    *x = r * cosT;
    *y = r * sinT;
}

bool __cdecl LogAccuracyHit(gentity_s *target, gentity_s *attacker)
{
    iassert(target);
    iassert(attacker);

    if (!target->takedamage)
        return 0;
    if (target == attacker)
        return 0;
    if (!target->client)
        return 0;
    if (!attacker->client)
        return 0;
#ifdef KISAK_MP
    if (target->client->ps.pm_type < PM_DEAD)
        return !OnSameTeam(target, attacker);
#elif KISAK_SP
    if (target->client->ps.stats[0] <= 0)
    {
        return 0;
    }
#endif
    return 0;
}

void __cdecl FireWeapon(gentity_s *ent, int gametime)
{
    double v2; // st7
    float offset[3]; // [esp+18h] [ebp-60h] BYREF
    float fAimSpreadAmount; // [esp+24h] [ebp-54h]
    float minSpread; // [esp+28h] [ebp-50h] BYREF
    float maxSpread; // [esp+2Ch] [ebp-4Ch] BYREF
    weaponParms wp; // [esp+30h] [ebp-48h] BYREF
    float aimSpreadScale; // [esp+74h] [ebp-4h]

    if ((ent->client->ps.eFlags & 0x300) == 0 || !ent->active)
    {
        Scr_Notify(ent, scr_const.weapon_fired, 0);
        wp.weapDef = BG_GetWeaponDef(ent->s.weapon);
        CalcMuzzlePoints(ent, &wp);
        aimSpreadScale = ent->client->currentAimSpreadScale;
        BG_GetSpreadForWeapon(&ent->client->ps, wp.weapDef, &minSpread, &maxSpread);
        if (ent->client->ps.fWeaponPosFrac == 1.0)
            v2 = (maxSpread - wp.weapDef->fAdsSpread) * aimSpreadScale + wp.weapDef->fAdsSpread;
        else
            v2 = (maxSpread - minSpread) * aimSpreadScale + minSpread;
        fAimSpreadAmount = v2;
        if (wp.weapDef->weapType)
        {
            if (wp.weapDef->weapType == WEAPTYPE_GRENADE)
            {
                Weapon_Throw_Grenade(ent, ent->s.weapon, ent->s.weaponModel, &wp);
            }
            else if (wp.weapDef->weapType == WEAPTYPE_PROJECTILE)
            {
                if (wp.weapDef->weapClass == WEAPCLASS_GRENADE)
                {
                    Weapon_GrenadeLauncher_Fire(ent, ent->s.weapon, ent->s.weaponModel, &wp);
                }
                else
                {
                    offset[0] = 0.0;
                    offset[1] = 0.0;
                    offset[2] = 0.0;
                    Weapon_RocketLauncher_Fire(ent, ent->s.weapon, fAimSpreadAmount, &wp, vec3_origin, 0, offset);
                }
            }
            else
            {
                Com_Error(ERR_DROP, "Unknown weapon type %i for %s", wp.weapDef->weapType, wp.weapDef->szInternalName);
            }
        }
        else
        {
            Bullet_Fire(ent, fAimSpreadAmount, &wp, ent, gametime);
        }
    }
}

void __cdecl CalcMuzzlePoints(const gentity_s *ent, weaponParms *wp)
{
    float *viewangles; // [esp+0h] [ebp-10h]
    float viewang[3]; // [esp+4h] [ebp-Ch] BYREF

    if (!ent->client)
        MyAssertHandler(".\\game\\g_weapon.cpp", 437, 0, "%s", "ent->client");
    viewangles = ent->client->ps.viewangles;
    viewang[0] = *viewangles;
    viewang[1] = viewangles[1];
    viewang[2] = viewangles[2];
    viewang[0] = ent->client->fGunPitch;
    viewang[1] = ent->client->fGunYaw;
    AngleVectors(viewang, wp->forward, wp->right, wp->up);
    G_GetPlayerViewOrigin(&ent->client->ps, wp->muzzleTrace);
}

void __cdecl G_UseOffHand(gentity_s *ent)
{
    weaponParms wp; // [esp+0h] [ebp-40h] BYREF

    if (!ent->client)
        MyAssertHandler(".\\game\\g_weapon.cpp", 545, 0, "%s", "ent->client");
    if (!ent->client->ps.offHandIndex)
        MyAssertHandler(".\\game\\g_weapon.cpp", 546, 0, "%s", "ent->client->ps.offHandIndex != WP_NONE");
    wp.weapDef = BG_GetWeaponDef(ent->client->ps.offHandIndex);
    if (wp.weapDef->weapType != WEAPTYPE_GRENADE)
        MyAssertHandler(".\\game\\g_weapon.cpp", 550, 0, "%s", "wp.weapDef->weapType == WEAPTYPE_GRENADE");
    CalcMuzzlePoints(ent, &wp);
    Weapon_Throw_Grenade(
        ent,
        ent->client->ps.offHandIndex,
        ent->client->ps.weaponmodels[ent->client->ps.offHandIndex],
        &wp);
}

void __cdecl FireWeaponMelee(gentity_s *ent, int gametime)
{
    weaponParms wp; // [esp+10h] [ebp-40h] BYREF

    if ((ent->client->ps.eFlags & 0x300) == 0 || !ent->active)
    {
        wp.weapDef = BG_GetWeaponDef(ent->s.weapon);
        G_GetPlayerViewOrigin(&ent->client->ps, wp.muzzleTrace);
        BG_GetPlayerViewDirection(&ent->client->ps, wp.forward, wp.right, wp.up);
        if (!player_meleeRange)
            MyAssertHandler(".\\game\\g_weapon.cpp", 577, 0, "%s", "player_meleeRange");
        if (!player_meleeWidth)
            MyAssertHandler(".\\game\\g_weapon.cpp", 578, 0, "%s", "player_meleeWidth");
        if (!player_meleeHeight)
            MyAssertHandler(".\\game\\g_weapon.cpp", 579, 0, "%s", "player_meleeHeight");
        Weapon_Melee(
            ent,
            &wp,
            player_meleeRange->current.value,
            player_meleeWidth->current.value,
            player_meleeHeight->current.value,
            gametime);
    }
}

int __cdecl G_GivePlayerWeapon(playerState_s *pPS, int iWeaponIndex, unsigned __int8 altModelIndex)
{
    int iCurrIndex; // [esp+0h] [ebp-10h]
    int newOffHandIndex; // [esp+4h] [ebp-Ch]
    WeaponDef *oldWeapDef; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (!pPS)
    {
        MyAssertHandler(".\\game\\g_weapon.cpp", 597, 0, "%s", "pPS");
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    }
    if (Com_BitCheckAssert(pPS->weapons, iWeaponIndex, 16))
        return 0;
    weapDef = BG_GetWeaponDef(iWeaponIndex);
    if (weapDef->weapClass == WEAPCLASS_TURRET)
        return 0;
    if (weapDef->weapClass == WEAPCLASS_NON_PLAYER)
        return 0;
    if (!weapDef->gunXModel[altModelIndex])
        return 0;
    if (level.clientIsSpawning)
        MyAssertHandler(
            ".\\game\\g_weapon.cpp",
            612,
            0,
            "%s\n\t(weapDef->szDisplayName) = %s",
            "(!level.clientIsSpawning)",
            weapDef->szDisplayName);
    Com_BitSetAssert(pPS->weapons, iWeaponIndex, 16);
    Com_BitClearAssert(pPS->weaponrechamber, iWeaponIndex, 16);
    Com_BitClearAssert(pPS->weaponold, iWeaponIndex, 16);
    pPS->weaponmodels[iWeaponIndex] = altModelIndex;
    if (weapDef->weapClass == WEAPCLASS_ITEM)
        return 1;
    if (weapDef->offhandClass)
    {
        if (pPS->offHandIndex)
        {
            if (BG_WeaponAmmo(pPS, pPS->offHandIndex) <= 0)
            {
                oldWeapDef = BG_GetWeaponDef(pPS->offHandIndex);
                if (!oldWeapDef)
                    MyAssertHandler(".\\game\\g_weapon.cpp", 634, 0, "%s", "oldWeapDef");
                newOffHandIndex = BG_GetFirstAvailableOffhand(pPS, oldWeapDef->offhandClass);
                if (newOffHandIndex)
                    pPS->offHandIndex = newOffHandIndex;
                else
                    pPS->offHandIndex = iWeaponIndex;
                G_SetEquippedOffHand(pPS->clientNum, pPS->offHandIndex);
            }
        }
        else
        {
            pPS->offHandIndex = iWeaponIndex;
            G_SetEquippedOffHand(pPS->clientNum, pPS->offHandIndex);
        }
        return 1;
    }
    else
    {
        for (iCurrIndex = weapDef->altWeaponIndex; iCurrIndex; iCurrIndex = BG_GetWeaponDef(iCurrIndex)->altWeaponIndex)
        {
            if (!pPS)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(pPS->weapons, iCurrIndex, 16))
                break;
            Com_BitSetAssert(pPS->weapons, iCurrIndex, 16);
            Com_BitClearAssert(pPS->weaponrechamber, iWeaponIndex, 16);
            pPS->weaponmodels[iCurrIndex] = altModelIndex;
        }
        return 1;
    }
}

void __cdecl G_SetupWeaponDef()
{
    Com_DPrintf(17, "----------------------\n");
    Com_DPrintf(17, "Game: G_SetupWeaponDef\n");
    if (!bg_lastParsedWeaponIndex)
    {
        Com_SetWeaponInfoMemory(1);
        ClearRegisteredItems();
        BG_ClearWeaponDef();
        G_GetWeaponIndexForName("defaultweapon_mp");
    }
    Com_DPrintf(17, "----------------------\n");
}

unsigned int __cdecl G_GetWeaponIndexForName(const char *name)
{
    if (level.initializing)
        return BG_GetWeaponIndexForName(name, G_RegisterWeapon);
    else
        return BG_FindWeaponIndexForName(name);
}

void __cdecl G_SelectWeaponIndex(int clientNum, int iWeaponIndex)
{
#ifdef KISAK_MP
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, va("%c %i", 97, iWeaponIndex));
#elif KISAK_SP
    SV_GameSendServerCommand(clientNum, va("sw %i", iWeaponIndex));
#endif
}

void __cdecl G_SetEquippedOffHand(int clientNum, unsigned int offHandIndex)
{
#ifdef KISAK_MP
    BG_AssertOffhandIndexOrNone(offHandIndex);
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, va("%c %i", 67, offHandIndex));
#elif KISAK_SP
    BG_AssertOffhandIndexOrNone(offHandIndex);
    SV_GameSendServerCommand(clientNum, va("offhand %i", offHandIndex));
#endif
}

