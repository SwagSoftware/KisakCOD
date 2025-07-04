#include "bullet.h"

#include <cgame_mp/cg_local_mp.h>
#include <game_mp/g_main_mp.h>
#include "game_public.h"
#include <game_mp/g_public_mp.h>
#include <client/client.h>
#include <server_mp/server_mp.h>
#include <game_mp/g_utils_mp.h>

char __cdecl Bullet_Trace(
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    BulletTraceResults *br,
    uint32_t lastSurfaceType)
{
    gentity_s *v7; // [esp+Ch] [ebp-10h]
    uint16_t hitEntId; // [esp+18h] [ebp-4h]

    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 427, 0, "%s", "bp");
    if (!weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 428, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 429, 0, "%s", "attacker");
    if (!br)
        MyAssertHandler(".\\game\\bullet.cpp", 430, 0, "%s", "br");
    if (lastSurfaceType >= 0x1D)
        MyAssertHandler(
            ".\\game\\bullet.cpp",
            431,
            0,
            "lastSurfaceType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
            lastSurfaceType,
            29);
    Com_Memset((uint32_t *)br, 0, 68);
    if (weapDef->bRifleBullet)
        G_LocationalTraceAllowChildren(&br->trace, (float*)bp->start, (float *)bp->end, bp->ignoreEntIndex, 0x2806831, riflePriorityMap);
    else
        G_LocationalTraceAllowChildren(&br->trace, (float *)bp->start, (float *)bp->end, bp->ignoreEntIndex, 0x2806831, bulletPriorityMap);
    if (br->trace.hitType == TRACE_HITTYPE_NONE)
        return 0;
    hitEntId = Trace_GetEntityHitId(&br->trace);
    if (hitEntId == 1022)
        v7 = 0;
    else
        v7 = &g_entities[hitEntId];
    br->hitEnt = v7;
    Vec3Lerp(bp->start, bp->end, br->trace.fraction, br->hitPos);
    if (br->hitEnt)
    {
        if ((br->hitEnt->s.eType == 1 || br->hitEnt->s.eType == 2) && !br->trace.surfaceFlags)
            br->trace.surfaceFlags = 0x700000;
        br->ignoreHitEnt = Bullet_IgnoreHitEntity(bp, br, attacker);
    }
    br->depthSurfaceType = (br->trace.surfaceFlags & 0x1F00000) >> 20;
    if ((br->trace.surfaceFlags & 0x100) != 0)
    {
        br->depthSurfaceType = 0;
    }
    else if (!br->depthSurfaceType)
    {
        if (lastSurfaceType)
            br->depthSurfaceType = lastSurfaceType;
    }
    return 1;
}

float __cdecl G_GoodRandomFloat(int32_t *idum)
{
    double v4; // [esp+Ch] [ebp-90h]
    int32_t j; // [esp+14h] [ebp-88h]
    int32_t iv[32]; // [esp+1Ch] [ebp-80h]

    *idum = -*idum;
    for (j = 39; j >= 0; --j)
    {
        *idum = 16807 * (*idum % 127773) - 2836 * (*idum / 127773);
        if (*idum < 0)
            *idum += 0x7FFFFFFF;
        if (j < 32)
            iv[j] = *idum;
    }
    *idum = 16807 * (*idum % 127773) - 2836 * (*idum / 127773);
    if (*idum < 0)
        *idum += 0x7FFFFFFF;
    v4 = (float)iv[iv[0] / 0x4000000] * 4.656612875245797e-10f;
    if (0.99999988f - v4 < 0.0f)
        return 0.99999988f;
    else
        return v4;
}

void __cdecl Bullet_Endpos(int32_t randSeed, float spread, float *end, float *dir, const weaponParms *wp, float maxRange)
{
    float v6; // [esp+Ch] [ebp-84h]
    float v7; // [esp+10h] [ebp-80h]
    float right; // [esp+84h] [ebp-Ch] BYREF
    float aimOffset; // [esp+88h] [ebp-8h]
    float up; // [esp+8Ch] [ebp-4h] BYREF

    if ((LODWORD(spread) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\game\\bullet.cpp", 91, 0, "%s", "!IS_NAN(spread)");
    if (!end)
        MyAssertHandler(".\\game\\bullet.cpp", 94, 0, "%s", "end");
    if (!wp)
        MyAssertHandler(".\\game\\bullet.cpp", 95, 0, "%s", "wp");
    v7 = spread * 0.01745329238474369f;
    v6 = tan(v7);
    aimOffset = v6 * maxRange;
    if ((LODWORD(aimOffset) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\game\\bullet.cpp", 100, 0, "%s", "!IS_NAN(aimOffset)");
    Bullet_RandomDir(randSeed, &right, &up);
    right = right * aimOffset;
    up = up * aimOffset;
    if ((LODWORD(right) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\game\\bullet.cpp", 110, 0, "%s", "!IS_NAN(right)");
    if ((LODWORD(up) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\game\\bullet.cpp", 111, 0, "%s", "!IS_NAN(up)");
    if ((COERCE_UNSIGNED_INT(wp->muzzleTrace[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->muzzleTrace[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->muzzleTrace[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\bullet.cpp",
            112,
            0,
            "%s",
            "!IS_NAN((wp->muzzleTrace)[0]) && !IS_NAN((wp->muzzleTrace)[1]) && !IS_NAN((wp->muzzleTrace)[2])");
    }
    if ((COERCE_UNSIGNED_INT(wp->forward[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->forward[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->forward[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\bullet.cpp",
            113,
            0,
            "%s",
            "!IS_NAN((wp->forward)[0]) && !IS_NAN((wp->forward)[1]) && !IS_NAN((wp->forward)[2])");
    }
    if ((COERCE_UNSIGNED_INT(wp->right[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->right[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->right[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\bullet.cpp",
            114,
            0,
            "%s",
            "!IS_NAN((wp->right)[0]) && !IS_NAN((wp->right)[1]) && !IS_NAN((wp->right)[2])");
    }
    if ((COERCE_UNSIGNED_INT(wp->up[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->up[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->up[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\bullet.cpp",
            115,
            0,
            "%s",
            "!IS_NAN((wp->up)[0]) && !IS_NAN((wp->up)[1]) && !IS_NAN((wp->up)[2])");
    }
    Vec3Mad(wp->muzzleTrace, maxRange, wp->forward, end);
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(".\\game\\bullet.cpp", 122, 0, "%s", "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    Vec3Mad(end, right, wp->right, end);
    Vec3Mad(end, up, wp->up, end);
    if ((COERCE_UNSIGNED_INT(*end) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(".\\game\\bullet.cpp", 127, 0, "%s", "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])");
    }
    if (dir)
    {
        Vec3Sub(end, wp->muzzleTrace, dir);
        Vec3Normalize(dir);
        if ((COERCE_UNSIGNED_INT(*dir) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(dir[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(dir[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\bullet.cpp",
                136,
                0,
                "%s",
                "!IS_NAN((dir)[0]) && !IS_NAN((dir)[1]) && !IS_NAN((dir)[2])");
        }
    }
}

void __cdecl Bullet_RandomDir(int32_t time, float *x, float *y)
{
    float v3; // [esp+8h] [ebp-14h]
    float sinT; // [esp+Ch] [ebp-10h]
    float theta; // [esp+10h] [ebp-Ch]
    float r; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    if (!x)
        MyAssertHandler(".\\game\\bullet.cpp", 68, 0, "%s", "x");
    if (!y)
        MyAssertHandler(".\\game\\bullet.cpp", 69, 0, "%s", "y");
    theta = G_GoodRandomFloat(&time) * 360.0f;
    r = G_GoodRandomFloat(&time);
    v3 = theta * 0.01745329238474369f;
    cosT = cos(v3);
    sinT = sin(v3);
    *x = r * cosT;
    *y = r * sinT;
}

void __cdecl Bullet_Fire(
    gentity_s *attacker,
    float spread,
    const weaponParms *wp,
    const gentity_s *weaponEnt,
    int32_t gameTime)
{
    int32_t number; // [esp+14h] [ebp-3A4h]
    int32_t shotCount; // [esp+20h] [ebp-398h]
    float range; // [esp+24h] [ebp-394h]
    AntilagClientStore antilagClients; // [esp+28h] [ebp-390h] BYREF
    BulletFireParams v9; // [esp+370h] [ebp-48h] BYREF
    int32_t shotIndex; // [esp+3B4h] [ebp-4h]

    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 752, 0, "%s", "attacker");
    if (!wp)
        MyAssertHandler(".\\game\\bullet.cpp", 753, 0, "%s", "wp");
    if (!wp->weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 754, 0, "%s", "wp->weapDef");
    if (wp->weapDef->weapType)
        MyAssertHandler(".\\game\\bullet.cpp", 755, 0, "%s", "wp->weapDef->weapType == WEAPTYPE_BULLET");
    G_AntiLagRewindClientPos(gameTime, &antilagClients);
    if (wp->weapDef->weapClass == WEAPCLASS_SPREAD)
    {
        shotCount = wp->weapDef->shotCount;
        range = wp->weapDef->fMinDamageRange;
    }
    else
    {
        shotCount = 1;
        range = 8192.0f;
    }
    for (shotIndex = 0; shotIndex < shotCount; ++shotIndex)
    {
        if (weaponEnt)
            number = weaponEnt->s.number;
        else
            number = 1022;
        v9.weaponEntIndex = number;
        v9.ignoreEntIndex = number;
        v9.damageMultiplier = 1.0f;
        v9.methodOfDeath = (wp->weapDef->bRifleBullet != 0) + 1;
        v9.origStart[0] = wp->muzzleTrace[0];
        v9.origStart[1] = wp->muzzleTrace[1];
        v9.origStart[2] = wp->muzzleTrace[2];
        v9.start[0] = wp->muzzleTrace[0];
        v9.start[1] = wp->muzzleTrace[1];
        v9.start[2] = wp->muzzleTrace[2];
        Bullet_Endpos(shotIndex + gameTime, spread, v9.end, v9.dir, wp, range);
        if (bullet_penetrationEnabled->current.enabled && wp->weapDef->penetrateType)
            Bullet_FirePenetrate(&v9, wp->weapDef, attacker, gameTime);
        else
            Bullet_FireExtended(&v9, wp->weapDef, attacker, gameTime);
    }
    G_AntiLag_RestoreClientPos(&antilagClients);
}

void __cdecl Bullet_FireExtended(BulletFireParams *bp, const WeaponDef *weapDef, gentity_s *attacker, int32_t gameTime)
{
    int32_t extIndex; // [esp+4h] [ebp-50h]
    int32_t impactFlags; // [esp+8h] [ebp-4Ch] BYREF
    BulletTraceResults br; // [esp+Ch] [ebp-48h] BYREF

    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 565, 0, "%s", "bp");
    if (!weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 566, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 567, 0, "%s", "attacker");
    for (extIndex = 0; extIndex < 12 && Bullet_Trace(bp, weapDef, attacker, &br, 0); ++extIndex)
    {
        Bullet_Process(bp, &br, weapDef, attacker, 0, gameTime, &impactFlags, 1);
        if ((br.trace.contents & 0x10) != 0)
        {
            if (!BG_AdvanceTrace(bp, &br, 0.13500001f))
                return;
        }
        else
        {
            if (!br.hitEnt || !br.hitEnt->takedamage)
                return;
            if (br.ignoreHitEnt)
            {
                BG_AdvanceTrace(bp, &br, 0.0f);
            }
            else
            {
                if (!weapDef->bRifleBullet
                    || !br.hitEnt->client
                    || !Dvar_GetInt("scr_friendlyfire") && OnSameTeam(br.hitEnt, attacker))
                {
                    return;
                }
                bp->damageMultiplier = bp->damageMultiplier * 0.5f;
                BG_AdvanceTrace(bp, &br, 0.0f);
            }
        }
    }
}

bool __cdecl Bullet_IgnoreHitEntity(const BulletFireParams *bp, const BulletTraceResults *br, gentity_s *attacker)
{
    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 375, 0, "%s", "bp");
    if (!br)
        MyAssertHandler(".\\game\\bullet.cpp", 376, 0, "%s", "br");
    if (!br->hitEnt)
        MyAssertHandler(".\\game\\bullet.cpp", 377, 0, "%s", "br->hitEnt");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 378, 0, "%s", "attacker");
    return br->hitEnt == attacker;
}

void __cdecl Bullet_Process(
    const BulletFireParams *bp,
    BulletTraceResults *br,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    int32_t dFlags,
    int32_t gameTime,
    int32_t *outImpactFlags,
    bool processFx)
{
    bool v8; // [esp+0h] [ebp-24h]
    bool targetWasAlive; // [esp+Fh] [ebp-15h]
    hitLocation_t hitLoc; // [esp+10h] [ebp-14h]
    int32_t damage; // [esp+14h] [ebp-10h]
    DynEntityDrawType drawType; // [esp+18h] [ebp-Ch] BYREF
    gentity_s *bulletEffectTempEnt; // [esp+1Ch] [ebp-8h] BYREF
    uint16_t hitEntId; // [esp+20h] [ebp-4h]

    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 490, 0, "%s", "bp");
    if (!br)
        MyAssertHandler(".\\game\\bullet.cpp", 491, 0, "%s", "br");
    if (br->trace.hitType == TRACE_HITTYPE_NONE)
        MyAssertHandler(".\\game\\bullet.cpp", 492, 0, "%s", "br->trace.hitType != TRACE_HITTYPE_NONE");
    if (!weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 493, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 494, 0, "%s", "attacker");
    *outImpactFlags = 0;
    damage = Bullet_GetDamage(bp, br, weapDef, attacker);
    G_CheckHitTriggerDamage(attacker, (float*)bp->start, br->hitPos, damage, bp->methodOfDeath);
    bulletEffectTempEnt = 0;
    if (processFx)
        Bullet_ImpactEffect(bp, br, br->trace.normal, weapDef, attacker, 0, &bulletEffectTempEnt);
    hitEntId = Trace_GetDynEntHitId(&br->trace, &drawType);
    if (br->hitEnt && br->hitEnt->takedamage && !br->ignoreHitEnt)
    {
        if (weapDef->armorPiercing)
            dFlags |= 2u;
        if (!br->hitEnt->r.inuse)
            MyAssertHandler(".\\game\\bullet.cpp", 528, 0, "%s", "br->hitEnt->r.inuse");
        targetWasAlive = br->hitEnt->health > 0;
        hitLoc = (hitLocation_t)br->trace.partGroup;
        if (hitLoc == HITLOC_HEAD || hitLoc == HITLOC_HELMET)
            *outImpactFlags |= 1u;
        if (g_debugLocDamage->current.enabled)
            CL_AddDebugStar(br->hitPos, colorRed, 100, 1);
        G_Damage(
            br->hitEnt,
            attacker,
            attacker,
            (float*)bp->dir,
            br->hitPos,
            damage,
            dFlags,
            bp->methodOfDeath,
            0xFFFFFFFF,
            hitLoc,
            br->trace.modelIndex,
            br->trace.partName,
            level.time - gameTime);
        v8 = br->hitEnt->r.inuse && br->hitEnt->health > 0;
        if (targetWasAlive && !v8)
            *outImpactFlags |= 2u;
    }
    if (bulletEffectTempEnt)
        bulletEffectTempEnt->s.un1.scale |= (uint8_t)*outImpactFlags;
}

int32_t __cdecl Bullet_GetDamage(
    const BulletFireParams *bp,
    const BulletTraceResults *br,
    const WeaponDef *weapDef,
    gentity_s *attacker)
{
    float v5; // [esp+8h] [ebp-2Ch]
    float diff[3]; // [esp+10h] [ebp-24h] BYREF
    int32_t damage; // [esp+1Ch] [ebp-18h]
    float lerpAmount; // [esp+20h] [ebp-14h]
    int32_t baseDamage; // [esp+24h] [ebp-10h]
    float range; // [esp+28h] [ebp-Ch]
    float dist; // [esp+2Ch] [ebp-8h]
    int32_t minDamage; // [esp+30h] [ebp-4h]

    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 154, 0, "%s", "bp");
    if (!br)
        MyAssertHandler(".\\game\\bullet.cpp", 155, 0, "%s", "br");
    if (br->trace.hitType == TRACE_HITTYPE_NONE)
        MyAssertHandler(".\\game\\bullet.cpp", 156, 0, "%s", "br->trace.hitType != TRACE_HITTYPE_NONE");
    if (!weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 157, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 158, 0, "%s", "attacker");
    baseDamage = weapDef->damage;
    minDamage = weapDef->minDamage;
    damage = baseDamage;
    range = weapDef->fMinDamageRange - weapDef->fMaxDamageRange;
    if (baseDamage != minDamage && range != 0.0)
    {
        Vec3Sub(br->hitPos, bp->origStart, diff);
        dist = Vec3Length(diff);
        if (weapDef->fMaxDamageRange <= (double)dist)
        {
            if (weapDef->fMinDamageRange <= (double)dist)
            {
                damage = minDamage;
            }
            else
            {
                lerpAmount = (dist - weapDef->fMaxDamageRange) / range;
                if (lerpAmount < 0.0 || lerpAmount > 1.0)
                    MyAssertHandler(
                        ".\\game\\bullet.cpp",
                        202,
                        0,
                        "%s\n\t(lerpAmount) = %g",
                        "((lerpAmount >= 0.0) && (lerpAmount <= 1.0))",
                        lerpAmount);
                v5 = lerpAmount * (double)minDamage + (1.0 - lerpAmount) * (double)baseDamage;
                damage = (int)v5;
            }
        }
        else
        {
            damage = baseDamage;
        }
    }
    return (int)((double)damage * bp->damageMultiplier);
}

void __cdecl Bullet_ImpactEffect(
    const BulletFireParams *bp,
    const BulletTraceResults *br,
    const float *normal,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    uint8_t impactEffectFlags,
    gentity_s **outTempEnt)
{
    bool v7; // [esp+Ch] [ebp-1Ch]
    gentity_s *tempEnt; // [esp+10h] [ebp-18h]
    float reflect[3]; // [esp+14h] [ebp-14h] BYREF
    bool createEffect; // [esp+23h] [ebp-5h]
    float dot; // [esp+24h] [ebp-4h]

    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 272, 0, "%s", "bp");
    if (!br)
        MyAssertHandler(".\\game\\bullet.cpp", 273, 0, "%s", "br");
    if (br->trace.hitType == TRACE_HITTYPE_NONE)
        MyAssertHandler(".\\game\\bullet.cpp", 274, 0, "%s", "br->trace.hitType != TRACE_HITTYPE_NONE");
    if (!weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 275, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 276, 0, "%s", "attacker");
    if (!outTempEnt)
        MyAssertHandler(".\\game\\bullet.cpp", 277, 0, "%s", "outTempEnt");
    if (br->ignoreHitEnt)
    {
        *outTempEnt = 0;
    }
    else
    {
        v7 = (br->trace.surfaceFlags & 4) == 0 && br->trace.fraction < 1.0;
        createEffect = v7;
        if (br->hitEnt)
            createEffect &= br->hitEnt->client == 0;
        if (createEffect)
        {
            dot = Vec3Dot(bp->dir, br->trace.normal) * -2.0;
            Vec3Mad(bp->dir, dot, br->trace.normal, reflect);
            if (sv_clientSideBullets->current.enabled)
            {
                tempEnt = 0;
            }
            else
            {
                tempEnt = G_TempEntity(br->hitPos, 41);
                tempEnt->s.weapon = (uint8_t)BG_GetWeaponIndex(weapDef);
                tempEnt->s.eventParm = DirToByte(normal);
                tempEnt->s.un1.scale = impactEffectFlags;
                tempEnt->s.surfType = (br->trace.surfaceFlags & 0x1F00000) >> 20;
                tempEnt->s.otherEntityNum = bp->weaponEntIndex;
            }
            *outTempEnt = tempEnt;
        }
        else
        {
            *outTempEnt = 0;
        }
    }
}

void __cdecl Bullet_FirePenetrate(BulletFireParams *bp, const WeaponDef *weapDef, gentity_s *attacker, int32_t gameTime)
{
    float v4; // [esp+10h] [ebp-190h]
    float v5; // [esp+18h] [ebp-188h]
    float v6; // [esp+1Ch] [ebp-184h]
    float v7; // [esp+20h] [ebp-180h]
    float v8; // [esp+24h] [ebp-17Ch]
    float v9; // [esp+28h] [ebp-178h]
    double value; // [esp+2Ch] [ebp-174h]
    float v11; // [esp+34h] [ebp-16Ch]
    bool v12; // [esp+38h] [ebp-168h]
    float v13; // [esp+40h] [ebp-160h]
    float v14[3]; // [esp+44h] [ebp-15Ch] BYREF
    float v15; // [esp+50h] [ebp-150h]
    float v16; // [esp+54h] [ebp-14Ch]
    float v17[3]; // [esp+58h] [ebp-148h] BYREF
    float SurfacePenetrationDepth; // [esp+64h] [ebp-13Ch]
    float v19; // [esp+68h] [ebp-138h]
    int32_t v20; // [esp+6Ch] [ebp-134h]
    float v[4]; // [esp+74h] [ebp-12Ch] BYREF
    float diff[5]; // [esp+84h] [ebp-11Ch] BYREF
    int32_t perks; // [esp+98h] [ebp-108h]
    gentity_s *bulletEffectTempEnt; // [esp+9Ch] [ebp-104h] BYREF
    BulletTraceResults revBr; // [esp+A0h] [ebp-100h] BYREF
    float lastHitPos[3]; // [esp+E8h] [ebp-B8h] BYREF
    float depth; // [esp+F4h] [ebp-ACh]
    bool processFx; // [esp+FBh] [ebp-A5h]
    int32_t penetrateIndex; // [esp+FCh] [ebp-A4h]
    bool allSolid; // [esp+103h] [ebp-9Dh]
    int32_t impactFlags; // [esp+104h] [ebp-9Ch] BYREF
    BulletFireParams revBp; // [esp+108h] [ebp-98h] BYREF
    bool revTraceHit; // [esp+14Fh] [ebp-51h]
    BulletTraceResults br; // [esp+150h] [ebp-50h] BYREF
    float maxDepth; // [esp+198h] [ebp-8h]
    bool traceHit; // [esp+19Fh] [ebp-1h]

    if (!bp)
        MyAssertHandler(".\\game\\bullet.cpp", 630, 0, "%s", "bp");
    if (!weapDef)
        MyAssertHandler(".\\game\\bullet.cpp", 631, 0, "%s", "weapDef");
    if (!attacker)
        MyAssertHandler(".\\game\\bullet.cpp", 632, 0, "%s", "attacker");
    if (weapDef->penetrateType == PENETRATE_TYPE_NONE)
        MyAssertHandler(".\\game\\bullet.cpp", 633, 0, "%s", "weapDef->penetrateType != PENETRATE_TYPE_NONE");
    if (Bullet_Trace(bp, weapDef, attacker, &br, 0))
    {
        Bullet_Process(bp, &br, weapDef, attacker, 0, gameTime, &impactFlags, 1);
        for (penetrateIndex = 0; penetrateIndex < 5; ++penetrateIndex)
        {
            maxDepth = BG_GetSurfacePenetrationDepth(weapDef, br.depthSurfaceType);
            if (attacker->client)
            {
                perks = attacker->client->ps.perks;
                if ((perks & 0x20) != 0)
                    maxDepth = maxDepth * perk_bulletPenetrationMultiplier->current.value;
            }
            if (maxDepth <= 0.0)
                break;
            lastHitPos[0] = br.hitPos[0];
            lastHitPos[1] = br.hitPos[1];
            lastHitPos[2] = br.hitPos[2];
            if (!BG_AdvanceTrace(bp, &br, 0.13500001f))
                break;
            traceHit = Bullet_Trace(bp, weapDef, attacker, &br, br.depthSurfaceType);
            Com_Memcpy((char *)&revBp, (char *)bp, 64);
            diff[4] = bp->dir[0]; // KISAKTODO check float here
            revBp.dir[0] = -bp->dir[0];
            revBp.dir[1] = -bp->dir[1];
            revBp.dir[2] = -bp->dir[2];
            diff[3] = bp->end[0]; // KISAKTODO check float here
            revBp.start[0] = bp->end[0];
            revBp.start[1] = bp->end[1];
            revBp.start[2] = bp->end[2];
            Vec3Mad(lastHitPos, 0.0099999998f, revBp.dir, revBp.end);
            Com_Memcpy((char *)&revBr, (char *)&br, 68);
            revBr.trace.normal[0] = -revBr.trace.normal[0];
            revBr.trace.normal[1] = -revBr.trace.normal[1];
            revBr.trace.normal[2] = -revBr.trace.normal[2];
            if (traceHit)
                BG_AdvanceTrace(&revBp, &revBr, 0.0099999998f);
            revTraceHit = Bullet_Trace(&revBp, weapDef, attacker, &revBr, revBr.depthSurfaceType);
            v12 = revTraceHit && revBr.trace.allsolid || br.trace.startsolid && revBr.trace.startsolid;
            allSolid = v12;
            if (revTraceHit || allSolid)
            {
                if (allSolid)
                {
                    Vec3Sub(revBp.end, revBp.start, diff);
                    v11 = Vec3Length(diff);
                }
                else
                {
                    Vec3Sub(lastHitPos, revBr.hitPos, v);
                    v11 = Vec3Length(v);
                }
                depth = v11;
                if (v11 < 1.0f)
                    depth = 1.0f;
                if (revTraceHit)
                {
                    if (attacker->client && (v20 = attacker->client->ps.perks, (v20 & 0x20) != 0))
                    {
                        value = perk_bulletPenetrationMultiplier->current.value;
                        v19 = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType) * value;
                        v9 = v19 - maxDepth;
                        v8 = v9 < 0.0f ? v19 : maxDepth;
                        maxDepth = v8;
                    }
                    else
                    {
                        SurfacePenetrationDepth = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType);
                        v7 = SurfacePenetrationDepth - maxDepth;
                        v6 = v7 < 0.0f ? SurfacePenetrationDepth : maxDepth;
                        maxDepth = v6;
                    }
                    if (maxDepth <= 0.0f)
                        return;
                }
                bp->damageMultiplier = bp->damageMultiplier - depth / maxDepth;
                if (bp->damageMultiplier <= 0.0f)
                    return;
                if (!allSolid)
                {
                    Vec3Sub(revBr.hitPos, br.hitPos, v17);
                    v16 = Vec3LengthSq(v17);
                    v15 = bullet_penetrationMinFxDist->current.value;
                    v5 = v15 * v15;
                    processFx = v5 < (float)v16;
                    if (v5 < (float)v16 && (!traceHit || (br.trace.surfaceFlags & 4) == 0))
                        Bullet_ImpactEffect(&revBp, &revBr, bp->dir, weapDef, attacker, impactFlags | 4, &bulletEffectTempEnt);
                    if (traceHit)
                        Bullet_Process(bp, &br, weapDef, attacker, 8, gameTime, &impactFlags, processFx);
                }
            }
            else if (traceHit)
            {
                Vec3Sub(lastHitPos, br.hitPos, v14);
                v13 = Vec3LengthSq(v14);
                v4 = bullet_penetrationMinFxDist->current.value * bullet_penetrationMinFxDist->current.value;
                processFx = v4 < (float)v13;
                Bullet_Process(bp, &br, weapDef, attacker, 8, gameTime, &impactFlags, processFx);
            }
            if (!traceHit)
                return;
        }
    }
}

