#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "turret.h"

void __cdecl TRACK_turret()
{
    track_static_alloc_internal(turretInfoStore, 6016, "turretInfoStore", 9);
}

void __cdecl G_InitTurrets()
{
    TurretInfo *v0; // r11

    v0 = turretInfoStore;
    do
    {
        v0->inuse = 0;
        ++v0;
    } while ((int)v0 < (int)&marker_turret);
    level.turrets = turretInfoStore;
}

int __cdecl Turret_GetRemainingConvergenceTime(const TurretInfo *turretInfo, unsigned int type)
{
    if (type >= 2)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 65, 0, "%s", "type == PITCH || type == YAW");
    return turretInfo->convergenceTime[type] + turretInfo->targetTime - level.time;
}

void __cdecl Turret_FillWeaponParms(const gentity_s *ent, const gentity_s *activator, weaponParms *wp)
{
    const char *v6; // r3
    gclient_s *client; // r3
    double v8; // fp10
    double v9; // fp13
    double v10; // fp11
    double v11; // fp0
    double v12; // fp9
    double v13; // fp12
    double v14; // fp0
    double v15; // fp12
    double v16; // fp11
    double v17; // fp13
    double v18; // fp12
    double v19; // fp11
    double v20; // fp10
    double v21; // fp9
    float v22; // [sp+50h] [-70h] BYREF
    float v23; // [sp+54h] [-6Ch]
    float v24; // [sp+58h] [-68h]
    float v25[13]; // [sp+60h] [-60h] BYREF
    float v26; // [sp+94h] [-2Ch]
    float v27; // [sp+98h] [-28h]
    float v28; // [sp+9Ch] [-24h]

    if (!G_DObjGetWorldTagMatrix(ent, scr_const.tag_flash, (float (*)[3]) & v25[4]))
    {
        v6 = SL_ConvertToString(ent->classname);
        Com_Error(ERR_DROP, byte_82044300, "tag_flash", ent->s.number, v6);
    }
    client = activator->client;
    if (client)
    {
        G_GetPlayerViewOrigin(&client->ps, &v22);
        G_GetPlayerViewDirection(activator, wp->forward, wp->right, wp->up);
        v8 = v23;
        v9 = (float)(v27 - v23);
        v10 = v24;
        v11 = (float)(v28 - v24);
        v12 = v22;
        v13 = (float)(v26 - v22);
        wp->gunForward[0] = wp->forward[0];
        wp->gunForward[1] = wp->forward[1];
        wp->gunForward[2] = wp->forward[2];
        v14 = __fsqrts((float)((float)((float)v13 * (float)v13)
            + (float)((float)((float)v11 * (float)v11) + (float)((float)v9 * (float)v9))));
        wp->muzzleTrace[0] = (float)(wp->forward[0] * (float)v14) + (float)v12;
        wp->muzzleTrace[1] = (float)((float)v14 * wp->forward[1]) + (float)v8;
        wp->muzzleTrace[2] = (float)(wp->forward[2] * (float)v14) + (float)v10;
    }
    else
    {
        v15 = ent->r.currentAngles[0];
        v16 = ent->s.lerp.u.turret.gunAngles[0];
        v25[1] = ent->r.currentAngles[1] + ent->s.lerp.u.turret.gunAngles[1];
        v25[2] = ent->r.currentAngles[2];
        v25[0] = (float)v15 + (float)v16;
        AngleVectors(v25, wp->forward, wp->right, wp->up);
        v17 = v25[5];
        v18 = v25[6];
        v19 = v26;
        v20 = v27;
        v21 = v28;
        wp->gunForward[0] = v25[4];
        wp->gunForward[1] = v17;
        wp->gunForward[2] = v18;
        wp->muzzleTrace[0] = v19;
        wp->muzzleTrace[1] = v20;
        wp->muzzleTrace[2] = v21;
    }
}

bool __cdecl MayHitTarget(const weaponParms *weapon, const gentity_s *target, const float *forward)
{
    double v6; // fp0
    double v7; // fp13
    double v8; // fp12
    double v9; // fp11
    double v10; // fp7
    double v11; // fp10
    double v12; // fp6
    double v13; // fp9
    int number; // r7
    double v15; // fp0
    float v17[4]; // [sp+50h] [-50h] BYREF
    float v18[16]; // [sp+60h] [-40h] BYREF

    if (!target)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 116, 0, "%s", "target");
    if (!weapon)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 117, 0, "%s", "weapon");
    v6 = (float)(target->r.currentOrigin[1] - weapon->muzzleTrace[1]);
    v7 = (float)(target->r.currentOrigin[2] - weapon->muzzleTrace[2]);
    v8 = (float)(target->r.currentOrigin[0] - weapon->muzzleTrace[0]);
    v9 = *forward;
    v10 = weapon->muzzleTrace[1];
    v11 = forward[1];
    v17[0] = weapon->muzzleTrace[0];
    v12 = weapon->muzzleTrace[2];
    v13 = forward[2];
    number = target->s.number;
    v17[2] = weapon->muzzleTrace[2];
    v17[1] = v10;
    v15 = __fsqrts((float)((float)((float)v8 * (float)v8)
        + (float)((float)((float)v7 * (float)v7) + (float)((float)v6 * (float)v6))));
    v18[0] = (float)((float)v9 * (float)v15) + v17[0];
    v18[1] = (float)((float)v11 * (float)v15) + (float)v10;
    v18[2] = (float)((float)v13 * (float)v15) + (float)v12;
    return (_cntlzw(SV_SightTraceToEntity(v17, vec3_origin, vec3_origin, v18, number, -1)) & 0x20) == 0;
}

void __cdecl Fire_Lead(gentity_s *ent, gentity_s *activator, int bUseAccuracy)
{
    gentity_s *v6; // r25
    weaponParms *v7; // r5
    const weaponParms *v8; // r4
    WeaponDef *WeaponDef; // r3
    TurretInfo *pTurretInfo; // r31
    double v11; // fp12
    double v12; // fp13
    double v13; // fp0
    double v14; // fp0
    double v15; // fp13
    double v16; // fp12
    int flags; // r11
    unsigned int v20; // r11
    double Visibility; // fp31
    int v22; // r29
    const gentity_s *v23; // r3
    char v24; // r11
    char v25; // r29
    gentity_s *v26; // r3
    char v27; // r11
    float v28; // [sp+50h] [-B0h] BYREF
    float v29; // [sp+54h] [-ACh]
    float v30; // [sp+58h] [-A8h]
    float v31[4]; // [sp+60h] [-A0h] BYREF
    weaponParms v32; // [sp+70h] [-90h] BYREF

    //Profile_Begin(239);
    if (!activator)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 146, 0, "%s", "activator");
    v6 = activator;
    if (activator == &g_entities[2175])
        v6 = ent;
    Turret_FillWeaponParms(ent, v6, &v32);
    WeaponDef = BG_GetWeaponDef(ent->s.weapon);
    pTurretInfo = ent->pTurretInfo;
    v32.weapDef = WeaponDef;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 155, 0, "%s", "turretInfo");
    v11 = pTurretInfo->targetPos[0];
    v31[0] = pTurretInfo->targetPos[0];
    v12 = pTurretInfo->targetPos[1];
    v31[1] = pTurretInfo->targetPos[1];
    v13 = pTurretInfo->targetPos[2];
    v31[2] = pTurretInfo->targetPos[2];
    if (bUseAccuracy)
    {
        v14 = (float)((float)v13 - v32.muzzleTrace[2]);
        v15 = (float)((float)v12 - v32.muzzleTrace[1]);
        v16 = (float)((float)v11 - v32.muzzleTrace[0]);
        flags = pTurretInfo->flags;
        _FP6 = -__fsqrts((float)((float)((float)v16 * (float)v16)
            + (float)((float)((float)v14 * (float)v14) + (float)((float)v15 * (float)v15))));
        __asm { fsel      f11, f6, f10, f11 }
        v20 = (float)((float)(v32.forward[0] * (float)((float)((float)1.0 / (float)_FP11) * (float)v16))
            + (float)((float)(v32.forward[2] * (float)((float)v14 * (float)((float)1.0 / (float)_FP11)))
                + (float)(v32.forward[1] * (float)((float)v15 * (float)((float)1.0 / (float)_FP11))))) > (double)pTurretInfo->forwardAngleDot
            ? flags & 0xFFFFFF7F
            : flags | 0x80;
        pTurretInfo->flags = v20;
        Visibility = SV_FX_GetVisibility(v32.muzzleTrace, v31);
        v22 = pTurretInfo->convergenceTime[1] + pTurretInfo->targetTime - level.time;
        if (EntHandle::isDefined(&pTurretInfo->target))
        {
            v23 = EntHandle::ent(&pTurretInfo->target);
            if (MayHitTarget(&v32, v23, v32.forward))
            {
                if (v22 > 0 || (v24 = 1, Visibility < 0.2))
                    v24 = 0;
                v25 = v24;
                if (EntHandle::ent(&pTurretInfo->target)->sentient)
                {
                    if (!v25 || (v26 = EntHandle::ent(&pTurretInfo->target), v27 = 1, v26->sentient->turretInvulnerability))
                        v27 = 0;
                    v25 = v27;
                }
                if (v25)
                    Actor_HitTarget(&v32, v31, &v28);
                else
                    Actor_MissTarget(&v32, v31, &v28);
                if ((float)((float)(v28 * v32.forward[0])
                    + (float)((float)(v29 * v32.forward[1]) + (float)(v30 * v32.forward[2]))) > 0.94999999)
                {
                    v32.forward[2] = v30;
                    v32.forward[1] = v29;
                    v32.forward[0] = v28;
                }
            }
        }
    }
    if (v32.weapDef->weapType)
    {
        Weapon_RocketLauncher_Fire(ent, ent->s.weapon, 0.0, v7, v32.forward, (gentity_s *)vec3_origin, 0);
    }
    else if (activator->client)
    {
        Bullet_Fire(v6, pTurretInfo->playerSpread, v8, (const gentity_s *)&v32);
    }
    else
    {
        Bullet_Fire(v6, pTurretInfo->aiSpread, v8, (const gentity_s *)&v32);
    }
    G_AddEvent(ent, 0x26u, v6->s.number);
    //Profile_EndInternal(0);
}

void __cdecl clamp_playerbehindgun(gentity_s *self, gentity_s *other)
{
    double v4; // fp12
    double v5; // fp11
    double v6; // fp31
    double v7; // fp30
    double v8; // fp29
    gclient_s *client; // r11
    gclient_s *v10; // r11
    float v11[4]; // [sp+50h] [-50h] BYREF
    float v12[4]; // [sp+60h] [-40h] BYREF

    v4 = self->r.currentAngles[1];
    v5 = self->s.lerp.u.turret.gunAngles[1];
    v11[0] = self->r.currentAngles[0] + self->s.lerp.u.turret.gunAngles[0];
    v11[2] = self->r.currentAngles[2];
    v11[1] = (float)v4 + (float)v5;
    AngleVectors(v11, v12, 0, 0);
    v6 = other->r.currentOrigin[2];
    v7 = (float)-(float)((float)(v12[0] * (float)34.0) - self->r.currentOrigin[0]);
    v8 = (float)-(float)((float)(v12[1] * (float)34.0) - self->r.currentOrigin[1]);
    SV_UnlinkEntity(other);
    client = other->client;
    client->ps.origin[0] = v7;
    client->ps.origin[1] = v8;
    client->ps.origin[2] = v6;
    BG_PlayerStateToEntityState(&other->client->ps, &other->s, 1, 1u);
    v10 = other->client;
    other->r.currentOrigin[0] = v10->ps.origin[0];
    other->r.currentOrigin[1] = v10->ps.origin[1];
    other->r.currentOrigin[2] = v10->ps.origin[2];
    SV_LinkEntity(other);
}

void __cdecl turret_clientaim(gentity_s *self, gentity_s *other)
{
    TurretInfo *pTurretInfo; // r27
    gclient_s *client; // r30
    long double v6; // fp2
    double v7; // fp0
    double v8; // fp0
    double v9; // fp30
    long double v10; // fp2
    double v11; // fp11
    double v16; // fp31
    long double v17; // fp2
    double v18; // fp13
    double v19; // fp12
    int flags; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 263, 0, "%s", "self");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 266, 0, "%s", "pTurretInfo");
    if (!other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 268, 0, "%s", "other");
    if (!other->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 269, 0, "%s", "other->client");
    client = other->client;
    if (!self->active)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 273, 0, "%s", "self->active");
    if (!EntHandle::isDefined(&self->r.ownerNum) || EntHandle::entnum(&self->r.ownerNum) != other->s.number)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp",
            274,
            0,
            "%s",
            "self->r.ownerNum.isDefined() && (self->r.ownerNum.entnum() == other->s.number)");
    v7 = client->ps.viewangles[0];
    client->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    client->ps.viewlocked_entNum = self->s.number;
    v8 = (float)((float)v7 - self->r.currentAngles[0]);
    v9 = (float)((float)v8 * (float)0.0027777778);
    *(double *)&v6 = (float)((float)((float)v8 * (float)0.0027777778) + (float)0.5);
    v10 = floor(v6);
    v11 = self->r.currentAngles[1];
    self->s.lerp.u.turret.gunAngles[0] = (float)((float)v9 - (float)*(double *)&v10) * (float)360.0;
    _FP10 = (float)((float)((float)((float)v9 - (float)*(double *)&v10) * (float)360.0) - pTurretInfo->arcmax[0]);
    _FP9 = (float)(pTurretInfo->arcmin[0] - (float)((float)((float)v9 - (float)*(double *)&v10) * (float)360.0));
    __asm
    {
        fsel      f0, f10, f13, f0
        fsel      f0, f9, f12, f0
    }
    self->s.lerp.u.turret.gunAngles[0] = _FP0;
    v16 = (float)((float)(client->ps.viewangles[1] - (float)v11) * (float)0.0027777778);
    *(double *)&v10 = (float)((float)((float)(client->ps.viewangles[1] - (float)v11) * (float)0.0027777778) + (float)0.5);
    v17 = floor(v10);
    self->s.lerp.u.turret.gunAngles[1] = (float)((float)v16 - (float)*(double *)&v17) * (float)360.0;
    v18 = pTurretInfo->arcmax[1];
    v19 = pTurretInfo->arcmin[1];
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    _FP11 = (float)((float)((float)((float)v16 - (float)*(double *)&v17) * (float)360.0) - (float)v18);
    _FP10 = (float)((float)v19 - (float)((float)((float)v16 - (float)*(double *)&v17) * (float)360.0));
    __asm
    {
        fsel      f0, f11, f13, f0
        fsel      f0, f10, f12, f0
    }
    self->s.lerp.u.turret.gunAngles[1] = _FP0;
    flags = pTurretInfo->flags;
    if ((flags & 0x800) != 0)
    {
        pTurretInfo->flags = flags & 0xFFFFF7FF;
        self->s.lerp.eFlags ^= 2u;
    }
}

void __cdecl turret_shoot_internal(gentity_s *self, gentity_s *other)
{
    if (!self->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 297, 0, "%s", "self->pTurretInfo");
    if (!other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 298, 0, "%s", "other");
    if ((self->pTurretInfo->flags & 0x4000) == 0)
    {
        self->pTurretInfo->fireSndDelay = 3 * BG_GetWeaponDef(self->s.weapon)->iFireTime;
        if (other->client)
        {
            Fire_Lead(self, other, 0);
            other->client->ps.viewlocked = PLAYERVIEWLOCK_WEAPONJITTER;
        }
        else
        {
            Fire_Lead(self, other, self->pTurretInfo->flags & 0x40);
        }
    }
}

void __cdecl turret_track(gentity_s *self, gentity_s *other)
{
    TurretInfo *pTurretInfo; // r31
    WeaponDef *WeaponDef; // r3
    int v6; // r11
    gclient_s *client; // r11

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 323, 0, "%s", "turretInfo");
    if (!self->active)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 325, 0, "%s", "self->active");
    if (!EntHandle::isDefined(&self->r.ownerNum) || EntHandle::ent(&self->r.ownerNum) != other)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp",
            326,
            0,
            "%s",
            "self->r.ownerNum.isDefined() && (self->r.ownerNum.ent() == other)");
    if (!other->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 327, 0, "%s", "other->client");
    turret_clientaim(self, other);
    clamp_playerbehindgun(self, other);
    WeaponDef = BG_GetWeaponDef(self->s.weapon);
    other->client->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    v6 = pTurretInfo->fireTime - 50;
    pTurretInfo->fireTime = v6;
    if (v6 <= 0)
    {
        pTurretInfo->fireTime = 0;
        client = other->client;
        if ((client->ps.pm_flags & 0x800) != 0 || (client->buttons & 1) == 0)
        {
            pTurretInfo->triggerDown = 0;
        }
        else if (WeaponDef->fireType != WEAPON_FIRETYPE_SINGLESHOT || !pTurretInfo->triggerDown)
        {
            pTurretInfo->triggerDown = 1;
            pTurretInfo->fireTime = WeaponDef->iFireTime;
            turret_shoot_internal(self, other);
        }
    }
}

void __cdecl turret_UpdateSound(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r29
    gentity_s *v3; // r3
    char v4; // r11
    char v5; // r10
    unsigned __int16 fireSndPlayer; // r11
    int v7; // r11
    unsigned __int16 stopSndPlayer; // r4

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 371, 0, "%s", "pTurretInfo");
    if (!EntHandle::isDefined(&self->r.ownerNum) || (v3 = EntHandle::ent(&self->r.ownerNum), v4 = 1, !v3->client))
        v4 = 0;
    v5 = v4;
    self->s.loopSound = 0;
    if (pTurretInfo->fireSndDelay > 0)
    {
        fireSndPlayer = pTurretInfo->fireSndPlayer;
        if (!fireSndPlayer || !v5)
            fireSndPlayer = pTurretInfo->fireSnd;
        self->s.loopSound = fireSndPlayer;
        v7 = pTurretInfo->fireSndDelay - 50;
        pTurretInfo->fireSndDelay = v7;
        if (v7 <= 0 && pTurretInfo->stopSnd)
        {
            self->s.loopSound = 0;
            stopSndPlayer = pTurretInfo->stopSndPlayer;
            if (!stopSndPlayer || !v5)
                stopSndPlayer = pTurretInfo->stopSnd;
            G_PlaySoundAlias(self, stopSndPlayer);
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl turret_CalculateConvergenceAngularVelocity(
    const gentity_s *self,
    const float *desiredAngles,
    float *angularVelocity,
    long double a4)
{
    TurretInfo *pTurretInfo; // r30
    int v8; // r7
    int targetTime; // r10
    int v10; // r29
    int v11; // r30
    int v12; // r26
    double v13; // fp30
    long double v14; // fp2
    double v15; // fp31
    double v16; // fp28
    long double v17; // fp2
    __int64 v18; // r10 OVERLAPPED
    double v19; // fp12
    int v20; // r11
    int v21; // r11

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 463, 0, "%s", "turretInfo");
    if (!angularVelocity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 464, 0, "%s", "angularVelocity");
    v8 = pTurretInfo->convergenceTime[0];
    targetTime = pTurretInfo->targetTime;
    v10 = self->nextthink - level.time;
    v11 = pTurretInfo->convergenceTime[1] - level.time + targetTime;
    v12 = v8 - level.time + targetTime;
    v13 = (float)((float)(self->s.lerp.u.turret.gunAngles[0] - *desiredAngles) * (float)0.0027777778);
    *(double *)&a4 = (float)((float)((float)(self->s.lerp.u.turret.gunAngles[0] - *desiredAngles) * (float)0.0027777778)
        + (float)0.5);
    v14 = floor(a4);
    v15 = (float)((float)(self->s.lerp.u.turret.gunAngles[1] - desiredAngles[1]) * (float)0.0027777778);
    v16 = __fabs((float)((float)((float)v13 - (float)*(double *)&v14) * (float)360.0));
    *(double *)&v14 = (float)((float)((float)(self->s.lerp.u.turret.gunAngles[1] - desiredAngles[1]) * (float)0.0027777778)
        + (float)0.5);
    v17 = floor(v14);
    LODWORD(v18) = v10;
    v19 = __fabs((float)((float)((float)v15 - (float)*(double *)&v17) * (float)360.0));
    *angularVelocity = (float)((float)v16 * (float)1000.0) / (float)v18;
    angularVelocity[1] = (float)((float)v19 * (float)1000.0) / (float)v18;
    if (v11 > 0 && v10 < v11)
    {
        v20 = v11;
        angularVelocity[1] = (float)((float)v18 / (float)*(__int64 *)((char *)&v18 - 4))
            * (float)((float)((float)v19 * (float)1000.0) / (float)v18);
    }
    if (v12 > 0 && v10 < v12)
    {
        v21 = v12;
        *angularVelocity = (float)((float)v18 / (float)*(__int64 *)((char *)&v18 - 4))
            * (float)((float)((float)v16 * (float)1000.0) / (float)v18);
    }
}

int __cdecl turret_UpdateTargetAngles(gentity_s *self, const float *desiredAngles, int bManned, long double a4)
{
    TurretInfo *pTurretInfo; // r23
    int v8; // r22
    long double v9; // fp2
    WeaponDef *WeaponDef; // r3
    int flags; // r11
    int v20; // r27
    LerpEntityStateTypeUnion *p_u; // r25
    float *v22; // r30
    int v23; // r29
    LerpEntityStateTypeUnion *v24; // r31
    double v25; // fp31
    double v26; // fp30
    double v27; // fp0
    int v28; // r11
    int result; // r3
    double v30; // fp0
    double pitchCap; // fp13
    float v32; // [sp+50h] [-A0h] BYREF
    float v33; // [sp+54h] [-9Ch]

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 523, 0, "%s", "pTurretInfo");
    v8 = 1;
    turret_CalculateConvergenceAngularVelocity(self, desiredAngles, &v32, a4);
    if (bManned)
    {
        WeaponDef = BG_GetWeaponDef(self->s.weapon);
        _FP8 = (float)(WeaponDef->maxTurnSpeed[0] - v32);
        _FP7 = (float)(WeaponDef->maxTurnSpeed[1] - v33);
        __asm
        {
            fsel      f0, f8, f10, f0
            fsel      f13, f7, f9, f13
        }
        _FP10 = (float)((float)_FP0 - WeaponDef->minTurnSpeed[0]);
        _FP9 = (float)((float)_FP13 - WeaponDef->minTurnSpeed[1]);
        __asm { fsel      f0, f10, f0, f12 }
        v32 = _FP0;
        __asm { fsel      f0, f9, f13, f11 }
    }
    else
    {
        _FP0 = 200.0;
        v32 = 200.0;
    }
    flags = pTurretInfo->flags;
    v33 = _FP0;
    if ((flags & 0x200) != 0 && (flags & 0x100) != 0)
        v32 = 360.0;
    v20 = (char *)desiredAngles - (char *)&v32;
    p_u = &self->s.lerp.u;
    v22 = &v32;
    v23 = 2;
    v24 = &self->s.lerp.u;
    do
    {
        v25 = (float)(*v22 * (float)0.050000001);
        *v22 = *v22 * (float)0.050000001;
        if (v25 < 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 556, 0, "%s", "fSpeed[i] >= 0");
        v26 = (float)((float)(*(float *)((char *)v22 + v20) - v24->turret.gunAngles[0]) * (float)0.0027777778);
        *(double *)&v9 = (float)((float)((float)(*(float *)((char *)v22 + v20) - v24->turret.gunAngles[0])
            * (float)0.0027777778)
            + (float)0.5);
        v9 = floor(v9);
        v27 = (float)((float)((float)v26 - (float)*(double *)&v9) * 360.0);
        if (v27 <= v25)
        {
            if (v27 >= -v25)
                goto LABEL_17;
            v27 = -v25;
        }
        else
        {
            v27 = v25;
        }
        v8 = 0;
    LABEL_17:
        --v23;
        v24->turret.gunAngles[0] = v24->turret.gunAngles[0] + (float)v27;
        ++v22;
        v24 = (LerpEntityStateTypeUnion *)((char *)v24 + 4);
    } while (v23);
    v28 = pTurretInfo->flags;
    result = v8;
    if ((v28 & 0x200) != 0)
    {
        v30 = p_u->turret.gunAngles[0];
        pitchCap = pTurretInfo->pitchCap;
        if ((v28 & 0x400) != 0)
        {
            if (v30 < pitchCap)
                goto LABEL_23;
        LABEL_21:
            pTurretInfo->flags = v28 & 0xFFFFFEFF;
        }
        else
        {
            if (v30 <= pitchCap)
                goto LABEL_21;
        LABEL_23:
            self->s.lerp.u.turret.gunAngles[2] = p_u->turret.gunAngles[0] - pTurretInfo->pitchCap;
        }
    }
    return result;
}

bool __cdecl turret_IsFiringInternal(int state)
{
    return state != 0;
}

bool __cdecl turret_IsFiring(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r31

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 604, 0, "%s", "pTurretInfo");
    return pTurretInfo->state != 0;
}

void __cdecl turret_SetState(gentity_s *self, unsigned int state)
{
    TurretInfo *pTurretInfo; // r30
    TurretInfo *v5; // r25

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 615, 0, "%s", "pTurretInfo");
    v5 = self->pTurretInfo;
    if (!v5)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 604, 0, "%s", "pTurretInfo");
    if (((_cntlzw(v5->state) & 0x20) == 0) != ((_cntlzw(state) & 0x20) == 0))
        Scr_Notify(self, scr_const.turretstatechange, 0);
    pTurretInfo->state = state;
}

void __cdecl turret_ClearTargetEnt(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r31

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 629, 0, "%s", "pTurretInfo");
    EntHandle::setEnt(&pTurretInfo->target, 0);
    pTurretInfo->flags &= 0xFFFFFFB7;
    turret_SetState(self, 0);
}

int __cdecl turret_ReturnToDefaultPos(gentity_s *self, int bManned)
{
    TurretInfo *pTurretInfo; // r29
    long double v5; // fp2
    float v7[12]; // [sp+50h] [-30h] BYREF

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 651, 0, "%s", "pTurretInfo");
    if (bManned)
        v7[0] = 0.0;
    else
        v7[0] = pTurretInfo->dropPitch;
    v7[1] = 0.0;
    turret_SetState(self, 0);
    return turret_UpdateTargetAngles(self, v7, bManned, v5);
}

void __cdecl turret_SetDefaultDropPitch(gentity_s *self, double pitch)
{
    TurretInfo *pTurretInfo; // r31

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 666, 0, "%s", "pTurretInfo");
    pTurretInfo->dropPitch = pitch;
}

void __cdecl turret_shoot(gentity_s *self)
{
    gentity_s *v2; // r4

    //Profile_Begin(236);
    if (EntHandle::isDefined(&self->r.ownerNum))
        v2 = EntHandle::ent(&self->r.ownerNum);
    else
        v2 = &g_entities[2175];
    turret_shoot_internal(self, v2);
    //Profile_EndInternal(0);
}

int __cdecl turret_CanTargetPoint(const gentity_s *self, const float *vPoint, float *vSource, float *localAngles)
{
    int result; // r3
    TurretInfo *pTurretInfo; // r27
    long double v10; // fp2
    float *arcmin; // r30
    int v12; // r27
    int v13; // r28
    float *currentAngles; // r29
    double v15; // fp31
    double v16; // fp0
    double v17; // fp0
    float v18; // [sp+50h] [-90h] BYREF
    float v19; // [sp+54h] [-8Ch]
    float v20; // [sp+58h] [-88h]
    float v21[4]; // [sp+60h] [-80h] BYREF
    float v22[14]; // [sp+70h] [-70h] BYREF

    if (G_DObjGetWorldTagPos(self, scr_const.tag_flash, &v18))
    {
        pTurretInfo = self->pTurretInfo;
        if (!pTurretInfo)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 693, 0, "%s", "pTurretInfo");
        v21[0] = *vPoint - v18;
        v21[1] = vPoint[1] - v19;
        v21[2] = vPoint[2] - v20;
        vectosignedangles(v21, v22);
        arcmin = pTurretInfo->arcmin;
        v12 = (char *)v22 - (char *)localAngles;
        v13 = 0;
        currentAngles = self->r.currentAngles;
        while (1)
        {
            v15 = (float)((float)(*(float *)((char *)localAngles + v12) - *currentAngles) * (float)0.0027777778);
            *(double *)&v10 = (float)((float)((float)(*(float *)((char *)localAngles + v12) - *currentAngles)
                * (float)0.0027777778)
                + (float)0.5);
            v10 = floor(v10);
            v16 = (float)((float)((float)v15 - (float)*(double *)&v10) * (float)360.0);
            *localAngles = (float)((float)v15 - (float)*(double *)&v10) * (float)360.0;
            if (v16 > arcmin[2] || v16 < *arcmin)
                break;
            ++v13;
            ++currentAngles;
            ++localAngles;
            ++arcmin;
            if (v13 >= 2)
            {
                result = 1;
                *vSource = v18;
                v17 = v20;
                vSource[1] = v19;
                vSource[2] = v17;
                return result;
            }
        }
    }
    return 0;
}

int __cdecl turret_CanTargetSentient(
    const gentity_s *self,
    const sentient_s *sentient,
    float *targetPosition,
    float *muzzlePosition,
    float *localAngles)
{
    TurretInfo *pTurretInfo; // r30
    double v11; // fp31
    long double v12; // fp2
    long double v13; // fp2
    double v14; // fp0
    double v15; // fp0
    double v16; // fp13
    double v17; // fp12
    gentity_s *ent; // r11
    double v19; // fp30
    double v20; // fp31
    double v21; // fp1
    double v22; // fp1
    int v23; // r27
    double v24; // fp1
    int result; // r3
    gentity_s *v26; // r11
    double v27; // fp1
    int v28; // r26
    gentity_s *v29; // r11
    double v30; // fp30
    double v31; // fp1
    double v32; // fp1
    float v33; // [sp+50h] [-80h] BYREF
    float v34; // [sp+54h] [-7Ch]
    float v35; // [sp+58h] [-78h]
    float v36; // [sp+60h] [-70h] BYREF
    float v37; // [sp+64h] [-6Ch]
    float v38; // [sp+68h] [-68h]
    float v39; // [sp+70h] [-60h] BYREF
    float v40; // [sp+74h] [-5Ch]

    Sentient_GetEyePosition(sentient, targetPosition);
    if (G_DObjGetWorldTagPos(self, scr_const.tag_flash, &v33))
    {
        pTurretInfo = self->pTurretInfo;
        if (!pTurretInfo)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 727, 0, "%s", "pTurretInfo");
        v36 = *targetPosition - v33;
        v37 = targetPosition[1] - v34;
        v38 = targetPosition[2] - v35;
        vectosignedangles(&v36, &v39);
        v11 = (float)((float)(v40 - self->r.currentAngles[1]) * (float)0.0027777778);
        *(double *)&v12 = (float)((float)((float)(v40 - self->r.currentAngles[1]) * (float)0.0027777778) + (float)0.5);
        v13 = floor(v12);
        v14 = (float)((float)((float)v11 - (float)*(double *)&v13) * (float)360.0);
        localAngles[1] = (float)((float)v11 - (float)*(double *)&v13) * (float)360.0;
        if (v14 <= pTurretInfo->arcmax[1] && v14 >= pTurretInfo->arcmin[1])
        {
            v15 = v33;
            v16 = v34;
            v17 = v35;
            *muzzlePosition = v33;
            muzzlePosition[1] = v16;
            muzzlePosition[2] = v17;
            if (pTurretInfo->state != 1)
            {
                ent = sentient->ent;
                v19 = self->r.currentAngles[0];
                v36 = sentient->ent->r.currentOrigin[0] - (float)v15;
                v37 = ent->r.currentOrigin[1] - (float)v16;
                v20 = 2.0;
                v38 = (float)(ent->r.currentOrigin[2] - (float)v17) + (float)2.0;
                v21 = vectosignedpitch(&v36);
                v22 = AngleSubtract(v21, v19);
                *localAngles = v22;
                if (v22 > pTurretInfo->arcmax[0])
                {
                    v23 = 1;
                LABEL_11:
                    v24 = AngleSubtract(v39, self->r.currentAngles[0]);
                    *localAngles = v24;
                    if (v24 > pTurretInfo->arcmax[0])
                    {
                        if (!v23)
                        {
                            result = 1;
                            *localAngles = 0.0;
                            targetPosition[2] = v35;
                            return result;
                        }
                        return 0;
                    }
                    if (v24 < pTurretInfo->arcmin[0])
                    {
                        if (v23)
                        {
                            result = 1;
                            *localAngles = 0.0;
                            targetPosition[2] = v35;
                            return result;
                        }
                        return 0;
                    }
                    return 1;
                }
                if (v22 < pTurretInfo->arcmin[0])
                {
                    v23 = 0;
                    goto LABEL_11;
                }
            LABEL_18:
                v26 = sentient->ent;
                result = 1;
                *targetPosition = sentient->ent->r.currentOrigin[0];
                targetPosition[1] = v26->r.currentOrigin[1];
                targetPosition[2] = v26->r.currentOrigin[2] + (float)v20;
                return result;
            }
            v27 = AngleSubtract(v39, self->r.currentAngles[0]);
            *localAngles = v27;
            if (v27 <= pTurretInfo->arcmax[0])
            {
                if (v27 >= pTurretInfo->arcmin[0])
                    return 1;
                v28 = 0;
            }
            else
            {
                v28 = 1;
            }
            v29 = sentient->ent;
            v30 = self->r.currentAngles[0];
            v36 = sentient->ent->r.currentOrigin[0] - v33;
            v37 = v29->r.currentOrigin[1] - v34;
            v20 = 2.0;
            v38 = (float)(v29->r.currentOrigin[2] - v35) + (float)2.0;
            v31 = vectosignedpitch(&v36);
            v32 = AngleSubtract(v31, v30);
            *localAngles = v32;
            if (v32 <= pTurretInfo->arcmax[0])
            {
                if (v32 >= pTurretInfo->arcmin[0])
                    goto LABEL_18;
                if (v28)
                {
                LABEL_25:
                    result = 1;
                    *localAngles = 0.0;
                    targetPosition[2] = v35;
                    return result;
                }
            }
            else if (!v28)
            {
                goto LABEL_25;
            }
        }
    }
    return 0;
}

void __cdecl turret_aimat_vector_internal(gentity_s *self, float *origin, int bShoot, const float *desiredAngles)
{
    TurretInfo *pTurretInfo; // r31
    int targetTime; // r11
    int state; // r9
    int v11; // r29
    unsigned int v12; // r4

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 826, 0, "%s", "turretInfo");
    if (!bShoot)
    {
        v12 = 0;
        goto LABEL_14;
    }
    targetTime = pTurretInfo->targetTime;
    state = pTurretInfo->state;
    v11 = pTurretInfo->convergenceTime[1] - level.time + targetTime;
    if (state)
    {
        if (state != 2)
            goto LABEL_15;
    }
    else
    {
        if (level.time < targetTime + 250
            && (__fabs(AngleSubtract(self->s.lerp.u.turret.gunAngles[0], *desiredAngles)) >= 5.0
                || __fabs(AngleSubtract(self->s.lerp.u.turret.gunAngles[1], desiredAngles[1])) >= 5.0))
        {
            goto LABEL_15;
        }
        turret_SetState(self, 2u);
    }
    BG_GetWeaponDef(self->s.weapon);
    if (v11 < 0)
    {
        v12 = 1;
    LABEL_14:
        turret_SetState(self, v12);
    }
LABEL_15:
    pTurretInfo->flags |= 0x40u;
    pTurretInfo->targetPos[0] = *origin;
    pTurretInfo->targetPos[1] = origin[1];
    pTurretInfo->targetPos[2] = origin[2];
}

int __cdecl turret_aimat_vector(gentity_s *self, float *origin, int bShoot, float *desiredAngles)
{
    int result; // r3
    float v9; // [sp+50h] [-40h] BYREF

    if (turret_CanTargetPoint(self, origin, &v9, desiredAngles))
    {
        turret_aimat_vector_internal(self, origin, bShoot, desiredAngles);
        return 1;
    }
    else
    {
        if (!self->pTurretInfo)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 879, 0, "%s", "self->pTurretInfo");
        result = 0;
        self->pTurretInfo->flags &= ~0x40u;
    }
    return result;
}

void __cdecl turret_SetTargetEnt(gentity_s *self, gentity_s *ent)
{
    TurretInfo *pTurretInfo; // r31
    gentity_s *v4; // r3
    int time; // r11

    pTurretInfo = self->pTurretInfo;
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 902, 0, "%s", "ent");
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 903, 0, "%s", "pTurretInfo");
    if (EntHandle::isDefined(&pTurretInfo->target))
        v4 = EntHandle::ent(&pTurretInfo->target);
    else
        v4 = 0;
    if (v4 != ent)
    {
        EntHandle::setEnt(&pTurretInfo->target, ent);
        time = level.time;
        pTurretInfo->flags &= ~8u;
        pTurretInfo->targetTime = time;
    }
}

// local variable allocation has failed, the output may be wrong!
int __cdecl turret_aimat_Sentient_Internal(
    gentity_s *self,
    sentient_s *enemy,
    int bShoot,
    int missTime,
    float *desiredAngles)
{
    int result; // r3
    TurretInfo *pTurretInfo; // r31
    int targetTime; // r11
    __int64 v13; // r10 OVERLAPPED
    double v14; // fp0
    double v15; // fp13
    double v16; // fp12
    double v17; // fp31
    double v18; // fp30
    gentity_s *ent; // r11
    double v20; // fp29
    double v21; // fp13
    double v22; // fp12
    double v25; // fp11
    int v26; // r11
    double v27; // fp13
    double v28; // fp12
    double v29; // fp0
    float v30; // [sp+50h] [-D0h] BYREF
    float v31; // [sp+54h] [-CCh]
    __int64 v32; // [sp+58h] [-C8h]
    __int64 v33; // [sp+60h] [-C0h] BYREF
    float v34; // [sp+68h] [-B8h]
    float v35; // [sp+70h] [-B0h] BYREF
    float v36; // [sp+74h] [-ACh]
    float v37; // [sp+78h] [-A8h]
    float v38[22]; // [sp+80h] [-A0h] BYREF

    if (missTime < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 935, 0, "%s", "missTime >= 0");
    turret_SetTargetEnt(self, enemy->ent);
    if (!turret_CanTargetSentient(self, enemy, &v35, (float *)&v33, desiredAngles))
        return 0;
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 943, 0, "%s", "pTurretInfo");
    targetTime = pTurretInfo->targetTime;
    if (pTurretInfo->convergenceTime[1] - level.time + targetTime > 0)
    {
        HIDWORD(v13) = pTurretInfo->flags;
        if ((v13 & 0x800000000LL) != 0)
        {
            if (missTime <= 0)
            {
                v27 = 0.0;
            }
            else
            {
                LODWORD(v13) = missTime;
                v26 = targetTime - level.time + missTime;
                v32 = v13;
                v33 = *(__int64 *)((char *)&v13 - 4);
                v27 = (float)((float)*(__int64 *)((char *)&v13 - 4) / (float)v13);
            }
            v28 = (float)((float)(pTurretInfo->missOffsetNormalized[1] * (float)33.0)
                + (float)(pTurretInfo->missOffsetNormalized[1] * (float)((float)v27 * (float)30.0)));
            v29 = (float)((float)(pTurretInfo->missOffsetNormalized[2] * (float)33.0)
                + (float)(pTurretInfo->missOffsetNormalized[2] * (float)((float)v27 * (float)30.0)));
            *(float *)&v33 = (float)((float)(pTurretInfo->missOffsetNormalized[0] * (float)33.0)
                + (float)((float)((float)v27 * (float)30.0) * pTurretInfo->missOffsetNormalized[0]))
                + v35;
            *((float *)&v33 + 1) = (float)v28 + v36;
            v34 = (float)v29 + v37;
            if (turret_aimat_vector(self, (float *)&v33, bShoot, desiredAngles))
                return 1;
        }
        else
        {
            pTurretInfo->flags = HIDWORD(v13) | 8;
            if (!G_DObjGetWorldTagMatrix(self, scr_const.tag_aim, (float (*)[3])v38))
                return 0;
            v30 = v36 - v38[10];
            v31 = v38[9] - v35;
            Vec2Normalize(&v30);
            v14 = (float)(v30 * (float)63.0);
            v15 = (float)(v31 * (float)63.0);
            v16 = (float)((float)(v38[1] * (float)(v31 * (float)63.0)) + (float)(v38[0] * (float)(v30 * (float)63.0)));
            v31 = v31 * (float)63.0;
            v30 = v30 * (float)63.0;
            if (v16 < 0.0)
            {
                v17 = (float)(v35 - (float)v14);
                v18 = (float)(v36 - (float)v15);
            }
            else
            {
                v17 = (float)(v35 + (float)v14);
                v18 = (float)((float)v15 + v36);
            }
            ent = enemy->ent;
            *((float *)&v33 + 1) = v18;
            *(float *)&v33 = v17;
            v20 = ent->r.currentOrigin[2];
            v34 = ent->r.currentOrigin[2];
            if (turret_aimat_vector(self, (float *)&v33, bShoot, desiredAngles)
                || (v20 = v37, v34 = v37, turret_aimat_vector(self, (float *)&v33, bShoot, desiredAngles)))
            {
                result = 1;
                pTurretInfo->missOffsetNormalized[0] = (float)v17 - v35;
                pTurretInfo->missOffsetNormalized[1] = (float)v18 - v36;
                pTurretInfo->missOffsetNormalized[2] = (float)v20 - v37;
                v21 = pTurretInfo->missOffsetNormalized[1];
                v22 = pTurretInfo->missOffsetNormalized[2];
                _FP9 = -__fsqrts((float)((float)(pTurretInfo->missOffsetNormalized[2] * pTurretInfo->missOffsetNormalized[2])
                    + (float)((float)(pTurretInfo->missOffsetNormalized[0]
                        * pTurretInfo->missOffsetNormalized[0])
                        + (float)(pTurretInfo->missOffsetNormalized[1]
                            * pTurretInfo->missOffsetNormalized[1]))));
                __asm { fsel      f11, f9, f10, f11 }
                v25 = (float)((float)1.0 / (float)_FP11);
                pTurretInfo->missOffsetNormalized[0] = pTurretInfo->missOffsetNormalized[0] * (float)v25;
                pTurretInfo->missOffsetNormalized[1] = (float)v21 * (float)v25;
                pTurretInfo->missOffsetNormalized[2] = (float)v22 * (float)v25;
                return result;
            }
            pTurretInfo->missOffsetNormalized[0] = 0.0;
            pTurretInfo->missOffsetNormalized[1] = 0.0;
            pTurretInfo->missOffsetNormalized[2] = 0.0;
        }
    }
    turret_aimat_vector_internal(self, &v35, bShoot, desiredAngles);
    return 1;
}

int __cdecl turret_aimat_Sentient(gentity_s *self, sentient_s *enemy, int bShoot, int missTime)
{
    int result; // r3
    long double v6; // fp2
    float v7[4]; // [sp+50h] [-20h] BYREF

    result = turret_aimat_Sentient_Internal(self, enemy, bShoot, missTime, v7);
    if (result)
    {
        turret_UpdateTargetAngles(self, v7, 1, v6);
        return 1;
    }
    return result;
}

int __cdecl turret_aimat_Ent(gentity_s *self, gentity_s *ent, int bShoot)
{
    long double v6; // fp2
    float v8[12]; // [sp+50h] [-30h] BYREF

    turret_SetTargetEnt(self, ent);
    if (!turret_aimat_vector(self, ent->r.currentOrigin, bShoot, v8))
        return 0;
    turret_UpdateTargetAngles(self, v8, 1, v6);
    return 1;
}

bool __cdecl turret_SightTrace(const float *start, const float *end, int passEnt1, int passEnt2)
{
    bool v9; // [sp+50h] [-30h] BYREF

    v9 = 0;
    //Profile_Begin(228);
    SV_SightTrace(&v9, start, vec3_origin, vec3_origin, end, passEnt1, passEnt2, 8394755);
    if (!v9)
        v9 = SV_FX_GetVisibility(start, end) < 0.2;
    //Profile_EndInternal(0);
    return v9;
}

int __cdecl turret_isTargetTooCloseToPlayer(
    const float *flashOrigin,
    const gentity_s *turret,
    const gentity_s *target)
{
    gentity_s *Player; // r31
    double v7; // fp13
    double v8; // fp12
    double v9; // fp11
    double v10; // fp30
    double v13; // fp10
    double v14; // fp6
    double v15; // fp5
    double v16; // fp4
    int result; // r3
    long double v18; // fp2
    double v19; // fp13
    double v20; // fp12
    double v21; // fp11
    double v24; // fp0
    long double v25; // fp2
    double v26; // fp12
    double v27; // fp30
    long double v28; // fp2

    Player = G_GetPlayer();
    if (!Player->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1099, 0, "%s", "player->sentient");
    if (!turret->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1100, 0, "%s", "turret->pTurretInfo");
    if (Player->sentient->eTeam != turret->pTurretInfo->eTeam)
        return 0;
    v7 = (float)((float)(Player->r.currentOrigin[1] + (float)((float)(Player->r.maxs[1] + Player->r.mins[1]) * (float)0.5))
        - flashOrigin[1]);
    v8 = (float)((float)(Player->r.currentOrigin[2] + (float)((float)(Player->r.maxs[2] + Player->r.mins[2]) * (float)0.5))
        - flashOrigin[2]);
    v9 = (float)((float)(Player->r.currentOrigin[0] + (float)((float)(Player->r.mins[0] + Player->r.maxs[0]) * (float)0.5))
        - *flashOrigin);
    v10 = __fsqrts((float)((float)((float)v9 * (float)v9)
        + (float)((float)((float)v8 * (float)v8) + (float)((float)v7 * (float)v7))));
    _FP10 = -v10;
    __asm { fsel      f10, f10, f0, f30 }
    v13 = (float)((float)1.0 / (float)_FP10);
    v14 = (float)((float)v13
        * (float)((float)(Player->r.currentOrigin[0]
            + (float)((float)(Player->r.mins[0] + Player->r.maxs[0]) * (float)0.5))
            - *flashOrigin));
    v15 = (float)((float)((float)(Player->r.currentOrigin[1]
        + (float)((float)(Player->r.maxs[1] + Player->r.mins[1]) * (float)0.5))
        - flashOrigin[1])
        * (float)v13);
    v16 = (float)((float)((float)(Player->r.currentOrigin[2]
        + (float)((float)(Player->r.maxs[2] + Player->r.mins[2]) * (float)0.5))
        - flashOrigin[2])
        * (float)v13);
    if (v10 < 0.0000099999997)
        return 1;
    *((double *)&v18 + 1) = target->r.currentOrigin[0];
    v19 = (float)((float)(target->r.currentOrigin[1] + (float)((float)(target->r.maxs[1] + target->r.mins[1]) * (float)0.5))
        - flashOrigin[1]);
    v20 = (float)((float)(target->r.currentOrigin[2] + (float)((float)(target->r.maxs[2] + target->r.mins[2]) * (float)0.5))
        - flashOrigin[2]);
    v21 = (float)((float)(target->r.currentOrigin[0] + (float)((float)(target->r.maxs[0] + target->r.mins[0]) * (float)0.5))
        - *flashOrigin);
    _FP9 = -__fsqrts((float)((float)((float)v21 * (float)v21)
        + (float)((float)((float)v20 * (float)v20) + (float)((float)v19 * (float)v19))));
    __asm { fsel      f10, f9, f0, f10 }
    v24 = (float)((float)1.0 / (float)_FP10);
    *(double *)&v18 = (float)((float)((float)((float)v24
        * (float)((float)(target->r.currentOrigin[0]
            + (float)((float)(target->r.maxs[0] + target->r.mins[0])
                * (float)0.5))
            - *flashOrigin))
        * (float)v14)
        + (float)((float)((float)((float)((float)(target->r.currentOrigin[2]
            + (float)((float)(target->r.maxs[2] + target->r.mins[2])
                * (float)0.5))
            - flashOrigin[2])
            * (float)v24)
            * (float)v16)
            + (float)((float)((float)((float)(target->r.currentOrigin[1]
                + (float)((float)(target->r.maxs[1] + target->r.mins[1])
                    * (float)0.5))
                - flashOrigin[1])
                * (float)v24)
                * (float)v15)));
    v25 = acos(v18);
    v26 = (float)(turretPlayerAvoidScale->current.value / (float)v10);
    v27 = (float)*(double *)&v25;
    *(double *)&v25 = (float)((float)((float)((float)(Player->r.maxs[2] - Player->r.mins[2])
        + (float)(target->r.maxs[2] - target->r.mins[2]))
        * (float)0.5)
        * (float)v26);
    v28 = atan(v25);
    result = 1;
    if (v27 > (float)*(double *)&v28)
        return 0;
    return result;
}

int __cdecl turret_isTargetVisible(gentity_s *self, const gentity_s *target, float *distSqr)
{
    TurretInfo *pTurretInfo; // r27
    double v8; // fp0
    double v9; // fp13
    double v10; // fp12
    double v11; // fp31
    sentient_s *sentient; // r4
    float v13[4]; // [sp+50h] [-70h] BYREF
    float v14[4]; // [sp+60h] [-60h] BYREF
    float v15[4]; // [sp+70h] [-50h] BYREF

    if (!target)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1143, 0, "%s", "target");
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1144, 0, "%s", "self");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1147, 0, "%s", "turretInfo");
    if (target->client && G_IsPlayerDrivingVehicle(target))
        return 0;
    v8 = (float)(target->r.currentOrigin[0] - self->r.currentOrigin[0]);
    v9 = (float)(target->r.currentOrigin[2] - self->r.currentOrigin[2]);
    v10 = (float)(target->r.currentOrigin[1] - self->r.currentOrigin[1]);
    v11 = (float)((float)((float)v10 * (float)v10)
        + (float)((float)((float)v9 * (float)v9) + (float)((float)v8 * (float)v8)));
    if (v11 >= pTurretInfo->maxRangeSquared)
        return 0;
    sentient = target->sentient;
    if (!sentient)
    {
        if (turret_CanTargetPoint(self, target->r.currentOrigin, v14, v15))
        {
            v13[0] = target->r.currentOrigin[0];
            v13[1] = target->r.currentOrigin[1];
            v13[2] = target->r.currentOrigin[2];
            goto LABEL_16;
        }
        return 0;
    }
    if (!turret_CanTargetSentient(self, sentient, v13, v14, v15))
        return 0;
    Sentient_GetEyePosition(target->sentient, v13);
LABEL_16:
    if (turret_isTargetTooCloseToPlayer(v14, self, target)
        || turret_SightTrace(v14, v13, self->s.number, target->s.number))
    {
        return 0;
    }
    if (distSqr)
        *distSqr = v11;
    return 1;
}

sentient_s *__cdecl turret_findBestTarget(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r30
    sentient_s *sentient; // r28
    EntHandle *p_target; // r3
    char v5; // r3
    int prevSentTarget; // r11
    sentient_s *v7; // r3
    sentient_s *v9; // r31
    double v10; // fp0
    double v11; // fp13
    double v12; // fp12
    float v13[16]; // [sp+50h] [-40h] BYREF

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1205, 0, "%s", "pTurretInfo");
    sentient = 0;
    if ((pTurretInfo->flags & 0x40) != 0
        && EntHandle::isDefined(&pTurretInfo->target)
        && EntHandle::ent(&pTurretInfo->target)->sentient)
    {
        p_target = &pTurretInfo->target;
        if (level.time <= pTurretInfo->targetTime + 4000)
            return EntHandle::ent(p_target)->sentient;
        sentient = EntHandle::ent(p_target)->sentient;
        if (sentient && (sentient->bIgnoreMe || !turret_isTargetVisible(self, sentient->ent, v13)))
            sentient = 0;
    }
    v5 = Sentient_EnemyTeam(pTurretInfo->eTeam);
    prevSentTarget = pTurretInfo->prevSentTarget;
    if (prevSentTarget <= -1)
        v7 = Sentient_FirstSentient(1 << v5);
    else
        v7 = Sentient_NextSentient(&level.sentients[prevSentTarget], 1 << v5);
    v9 = v7;
    if (v7)
    {
        pTurretInfo->prevSentTarget = v7 - level.sentients;
        if (!v7->bIgnoreMe)
        {
            if (turret_isTargetVisible(self, v7->ent, v13))
            {
                if (!sentient)
                    return v9;
                v10 = (float)(sentient->ent->r.currentOrigin[0] - self->r.currentOrigin[0]);
                v11 = (float)(sentient->ent->r.currentOrigin[2] - self->r.currentOrigin[2]);
                v12 = (float)(sentient->ent->r.currentOrigin[1] - self->r.currentOrigin[1]);
                if (v13[0] < (double)(float)((float)((float)v12 * (float)v12)
                    + (float)((float)((float)v10 * (float)v10) + (float)((float)v11 * (float)v11))))
                    return v9;
            }
        }
    }
    else
    {
        pTurretInfo->prevSentTarget = -1;
    }
    return sentient;
}

void __cdecl turret_think_auto_nonai(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r28
    sentient_s *BestTarget; // r29
    gentity_s *v4; // r30
    long double v5; // fp2
    int v6; // r3
    float v7[16]; // [sp+50h] [-40h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1272, 0, "%s", "self");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1275, 0, "%s", "turretInfo");
    BestTarget = turret_findBestTarget(self);
    if (EntHandle::isDefined(&pTurretInfo->manualTarget))
        v4 = EntHandle::ent(&pTurretInfo->manualTarget);
    else
        v4 = 0;
    if (!BestTarget)
    {
    LABEL_11:
        if (v4)
        {
            if (!v4->sentient)
            {
                v6 = turret_aimat_Ent(self, v4, 1);
            LABEL_16:
                if (v6)
                    return;
                goto LABEL_17;
            }
            if (turret_isTargetVisible(self, v4, 0))
            {
                v6 = turret_aimat_Sentient(self, v4->sentient, 1, pTurretInfo->convergenceTime[1]);
                goto LABEL_16;
            }
        }
    LABEL_17:
        turret_ClearTargetEnt(self);
        if (!self->pTurretInfo)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 651, 0, "%s", "pTurretInfo");
        v7[0] = 0.0;
        v7[1] = 0.0;
        turret_SetState(self, 0);
        goto LABEL_20;
    }
    if (!turret_aimat_Sentient_Internal(self, BestTarget, 1, pTurretInfo->convergenceTime[1], v7))
    {
        if (v4 == BestTarget->ent)
            goto LABEL_17;
        goto LABEL_11;
    }
LABEL_20:
    turret_UpdateTargetAngles(self, v7, 1, v5);
}

int __cdecl turret_think_auto(gentity_s *self, actor_s *actor)
{
    TurretInfo *pTurretInfo; // r24
    sentient_s *TargetSentient; // r26
    gentity_s *v6; // r20
    sentient_info_t *v7; // r25
    double v8; // fp31
    double v9; // fp29
    double v10; // fp30
    double v11; // fp28
    int result; // r3
    bool v13; // r28
    bool v16; // r23
    sentient_s *v17; // r3
    int time; // r11
    sentient_s *v19; // r3
    SentientHandle *p_detachSentient; // r3
    sentient_info_t *v21; // r28
    int lastKnownPosTime; // r10
    sentient_s *v23; // r3
    const sentient_s *v24; // r3
    sentient_s *v25; // r3
    bool v26; // zf
    sentient_s *sentient; // r29
    char *v28; // r11
    double v29; // fp0
    long double v30; // fp2
    double v31; // fp0
    double v32; // fp13
    double v33; // fp12
    sentient_s *v34; // r4
    TurretInfo *v35; // r28
    long double v36; // fp2
    float v37[2]; // [sp+50h] [-C0h] BYREF
    float v38[4]; // [sp+58h] [-B8h] BYREF
    float v39[4]; // [sp+68h] [-A8h] BYREF
    float v40[12]; // [sp+78h] [-98h] BYREF

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1337, 0, "%s", "pTurretInfo");
    if ((pTurretInfo->flags & 0x20) == 0
        || pTurretInfo->originError[0] != 0.0
        || pTurretInfo->originError[1] != 0.0
        || pTurretInfo->originError[2] != 0.0)
    {
        v35 = self->pTurretInfo;
        if (!v35)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 651, 0, "%s", "pTurretInfo");
        v37[0] = v35->dropPitch;
        v37[1] = 0.0;
        turret_SetState(self, 0);
        turret_UpdateTargetAngles(self, v37, 0, v36);
        return 1;
    }
    TargetSentient = Actor_GetTargetSentient(actor);
    if ((pTurretInfo->flags & 0x10) != 0 && SentientHandle::isDefined(&pTurretInfo->detachSentient))
        TargetSentient = SentientHandle::sentient(&pTurretInfo->detachSentient);
    if (EntHandle::isDefined(&pTurretInfo->manualTarget))
        v6 = EntHandle::ent(&pTurretInfo->manualTarget);
    else
        v6 = 0;
    if (!TargetSentient)
    {
        v7 = 0;
        if (SentientHandle::isDefined(&pTurretInfo->detachSentient)
            && actor->sentientInfo[SentientHandle::sentient(&pTurretInfo->detachSentient) - level.sentients].attackTime <= level.time)
        {
            SentientHandle::setSentient(&pTurretInfo->detachSentient, 0);
        }
        Actor_CanAttackAll(actor);
        goto LABEL_53;
    }
    v7 = &actor->sentientInfo[TargetSentient - level.sentients];
    actor->sentientInfo[TargetSentient - level.sentients].attackTime = level.time + 2000;
    v8 = (float)(TargetSentient->ent->r.currentOrigin[1] - self->r.currentOrigin[1]);
    v9 = (float)(TargetSentient->ent->r.currentOrigin[0] - self->r.currentOrigin[0]);
    v10 = (float)(TargetSentient->ent->r.currentOrigin[2] - self->r.currentOrigin[2]);
    v11 = (float)((float)((float)v9 * (float)v9)
        + (float)((float)((float)v10 * (float)v10) + (float)((float)v8 * (float)v8)));
    if (TargetSentient->bIgnoreMe || v11 >= pTurretInfo->maxRangeSquared)
    {
        v7 = 0;
        goto LABEL_51;
    }
    if (v7->VisCache.bVisible && turret_aimat_Sentient(self, TargetSentient, 1, pTurretInfo->convergenceTime[1]))
    {
        result = 1;
        pTurretInfo->flags &= ~0x10u;
        v7->attackTime = 0;
        return result;
    }
    v13 = level.time - actor->iStateTime >= 1000;
    if (level.time - actor->iStateTime < 1000)
        goto LABEL_51;
    AngleVectors(self->r.currentAngles, v39, 0, 0);
    _FP12 = -__fsqrts(v11);
    __asm { fsel      f0, f12, f13, f0 }
    v16 = (float)((float)(v39[0] * (float)((float)((float)1.0 / (float)_FP0) * (float)v9))
        + (float)((float)(v39[1] * (float)((float)v8 * (float)((float)1.0 / (float)_FP0)))
            + (float)(v39[2] * (float)((float)v10 * (float)((float)1.0 / (float)_FP0))))) >= (double)pTurretInfo->forwardAngleDot;
    if (SentientHandle::isDefined(&pTurretInfo->detachSentient))
    {
        if (SentientHandle::sentient(&pTurretInfo->detachSentient) == TargetSentient)
            goto LABEL_36;
        v17 = SentientHandle::sentient(&pTurretInfo->detachSentient);
        time = level.time;
        if (actor->sentientInfo[v17 - level.sentients].attackTime > level.time)
            goto LABEL_36;
    }
    else
    {
        time = level.time;
    }
    if ((v7->VisCache.bVisible || !v16)
        && time - v7->lastKnownPosTime < pTurretInfo->suppressTime
        && Vec2DistanceSq(TargetSentient->ent->r.currentOrigin, v7->vLastKnownPos) < 4096.0)
    {
        Sentient_GetEyePosition(actor->sentient, v40);
        Sentient_GetEyePosition(TargetSentient, v38);
        if (!turret_SightTrace(v40, v38, actor->ent->s.number, TargetSentient->ent->s.number))
        {
            SentientHandle::setSentient(&pTurretInfo->detachSentient, TargetSentient);
            v13 = 0;
            if (v11 < 65536.0)
                v7->attackTime = 0;
        }
    }
LABEL_36:
    if (v13 && SentientHandle::isDefined(&pTurretInfo->detachSentient))
    {
        v19 = SentientHandle::sentient(&pTurretInfo->detachSentient);
        v21 = &actor->sentientInfo[v19 - level.sentients];
        lastKnownPosTime = actor->sentientInfo[v19 - level.sentients].lastKnownPosTime;
        p_detachSentient = &pTurretInfo->detachSentient;
        if (level.time - lastKnownPosTime >= pTurretInfo->suppressTime)
        {
        LABEL_41:
            SentientHandle::setSentient(p_detachSentient, 0);
            goto LABEL_51;
        }
        v23 = SentientHandle::sentient(p_detachSentient);
        if (Vec2DistanceSq(v23->ent->r.currentOrigin, v21->vLastKnownPos) < 4096.0)
        {
            Sentient_GetEyePosition(actor->sentient, v40);
            v24 = SentientHandle::sentient(&pTurretInfo->detachSentient);
            Sentient_GetEyePosition(v24, v38);
            v25 = SentientHandle::sentient(&pTurretInfo->detachSentient);
            v26 = !turret_SightTrace(v40, v38, actor->ent->s.number, v25->ent->s.number);
            p_detachSentient = &pTurretInfo->detachSentient;
            if (v26)
            {
                if (SentientHandle::sentient(p_detachSentient) == TargetSentient)
                {
                    if ((pTurretInfo->flags & 0x10) != 0 || !v16 || turret_ReturnToDefaultPos(self, 1))
                    {
                        result = 0;
                        v7->attackTime = 0;
                        return result;
                    }
                    if (v11 < 65536.0)
                    {
                        v7->attackTime = 0;
                        return 1;
                    }
                    return 1;
                }
                goto LABEL_51;
            }
            goto LABEL_41;
        }
        SentientHandle::setSentient(&pTurretInfo->detachSentient, 0);
    }
LABEL_51:
    if (v6 == TargetSentient->ent)
        v6 = 0;
LABEL_53:
    if (EntHandle::isDefined(&pTurretInfo->target) && EntHandle::ent(&pTurretInfo->target)->sentient)
    {
        sentient = EntHandle::ent(&pTurretInfo->target)->sentient;
        v28 = (char *)actor + 40 * (sentient - level.sentients);
        v7 = (sentient_info_t *)(v28 + 2100);
        if (!sentient->bIgnoreMe && level.time - *((unsigned int *)v28 + 529) < pTurretInfo->suppressTime)
        {
            v38[0] = *((float *)v28 + 531);
            v38[1] = *((float *)v28 + 532);
            if ((pTurretInfo->flags & 0x40) != 0)
                v29 = pTurretInfo->targetPos[2];
            else
                v29 = (float)(*((float *)v28 + 533) + (float)32.0);
            v38[2] = v29;
            if (turret_aimat_vector(self, v38, 1, v37))
            {
                turret_UpdateTargetAngles(self, v37, 1, v30);
                v7->attackTime = 0;
                if (SentientHandle::isDefined(&pTurretInfo->detachSentient)
                    && SentientHandle::sentient(&pTurretInfo->detachSentient) == sentient)
                {
                    SentientHandle::setSentient(&pTurretInfo->detachSentient, 0);
                    return 1;
                }
                return 1;
            }
            goto LABEL_66;
        }
    }
    else if (!v7)
    {
        goto LABEL_66;
    }
    v7->lastKnownPosTime = 0;
LABEL_66:
    if (v6)
    {
        v31 = (float)(self->r.currentOrigin[1] - v6->r.currentOrigin[1]);
        v32 = (float)(self->r.currentOrigin[2] - v6->r.currentOrigin[2]);
        v33 = (float)(self->r.currentOrigin[0] - v6->r.currentOrigin[0]);
        if ((float)((float)((float)v33 * (float)v33)
            + (float)((float)((float)v32 * (float)v32) + (float)((float)v31 * (float)v31))) < (double)pTurretInfo->maxRangeSquared)
        {
            v34 = v6->sentient;
            if (!v34)
            {
                if (turret_aimat_Ent(self, v6, 1))
                    return 1;
                goto LABEL_73;
            }
            if (actor->sentientInfo[v34 - level.sentients].VisCache.bVisible
                && turret_aimat_Sentient(self, v34, 1, pTurretInfo->convergenceTime[1]))
            {
                return 1;
            }
        }
    }
LABEL_73:
    if (!v7 || !v7->VisCache.bVisible)
        turret_ClearTargetEnt(self);
    turret_ReturnToDefaultPos(self, 1);
    return 1;
}

int __cdecl turret_think_manual(gentity_s *self, actor_s *actor)
{
    TurretInfo *pTurretInfo; // r30
    sentient_s *TargetSentient; // r3
    double v7; // fp0
    double v8; // fp13
    double v9; // fp12
    double v10; // fp0
    double v11; // fp13
    double v12; // fp12
    int v13; // r28
    gentity_s *v14; // r3
    gentity_s *v15; // r31
    double v16; // fp0
    double v17; // fp13
    double v18; // fp12
    sentient_s *sentient; // r4
    TurretInfo *v20; // r30
    long double v21; // fp2
    float v22[4]; // [sp+50h] [-50h] BYREF

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1543, 0, "%s", "pTurretInfo");
    if (!actor)
        goto LABEL_15;
    if ((pTurretInfo->flags & 0x20) != 0
        && pTurretInfo->originError[0] == 0.0
        && pTurretInfo->originError[1] == 0.0
        && pTurretInfo->originError[2] == 0.0)
    {
        if (SentientHandle::isDefined(&pTurretInfo->detachSentient))
            return turret_think_auto(self, actor);
        TargetSentient = Actor_GetTargetSentient(actor);
        if (TargetSentient)
        {
            v7 = (float)(self->r.currentOrigin[1] - TargetSentient->ent->r.currentOrigin[1]);
            v8 = (float)(self->r.currentOrigin[2] - TargetSentient->ent->r.currentOrigin[2]);
            v9 = (float)(self->r.currentOrigin[0] - TargetSentient->ent->r.currentOrigin[0]);
            if ((float)((float)((float)v9 * (float)v9)
                + (float)((float)((float)v8 * (float)v8) + (float)((float)v7 * (float)v7))) < (double)pTurretInfo->maxRangeSquared)
            {
                v10 = (float)(actor->ent->r.currentOrigin[1] - TargetSentient->ent->r.currentOrigin[1]);
                v11 = (float)(actor->ent->r.currentOrigin[2] - TargetSentient->ent->r.currentOrigin[2]);
                v12 = (float)(actor->ent->r.currentOrigin[0] - TargetSentient->ent->r.currentOrigin[0]);
                if ((float)((float)((float)v12 * (float)v12)
                    + (float)((float)((float)v11 * (float)v11) + (float)((float)v10 * (float)v10))) < 65536.0)
                    return turret_think_auto(self, actor);
            }
        }
        pTurretInfo->flags &= ~0x10u;
        if (TargetSentient)
            actor->sentientInfo[TargetSentient - level.sentients].attackTime = level.time + 2000;
        else
            Actor_CanAttackAll(actor);
    LABEL_15:
        v13 = ((unsigned int)pTurretInfo->flags >> 2) & 1;
        if (EntHandle::isDefined(&pTurretInfo->manualTarget))
        {
            v14 = EntHandle::ent(&pTurretInfo->manualTarget);
            v15 = v14;
            if (v14)
            {
                v16 = (float)(v14->r.currentOrigin[1] - self->r.currentOrigin[1]);
                v17 = (float)(v14->r.currentOrigin[2] - self->r.currentOrigin[2]);
                v18 = (float)(v14->r.currentOrigin[0] - self->r.currentOrigin[0]);
                if ((float)((float)((float)v18 * (float)v18)
                    + (float)((float)((float)v17 * (float)v17) + (float)((float)v16 * (float)v16))) < (double)pTurretInfo->maxRangeSquared)
                {
                    sentient = v14->sentient;
                    if (!sentient)
                    {
                        turret_aimat_Ent(self, v14, v13);
                        return 1;
                    }
                    if (turret_aimat_Sentient(self, sentient, v13, 0))
                        return 1;
                }
            }
        }
        else
        {
            v15 = 0;
        }
        if (actor && v15)
        {
            turret_ReturnToDefaultPos(self, 1);
            return 1;
        }
        turret_ClearTargetEnt(self);
        return 1;
    }
    v20 = self->pTurretInfo;
    if (!v20)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 651, 0, "%s", "pTurretInfo");
    v22[0] = v20->dropPitch;
    v22[1] = 0.0;
    turret_SetState(self, 0);
    turret_UpdateTargetAngles(self, v22, 0, v21);
    return 1;
}

void __cdecl turret_RestoreDefaultDropPitch(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r27
    DObjAnimMat *LocalTagMatrix; // r28
    DObjAnimMat *v4; // r29
    double v5; // fp0
    __int64 v6; // r11
    int v7; // r29
    float v8; // [sp+50h] [-140h] BYREF
    float v9; // [sp+54h] [-13Ch]
    float v10; // [sp+58h] [-138h]
    float v11[4]; // [sp+60h] [-130h] BYREF
    float v12[4]; // [sp+70h] [-120h] BYREF
    __int64 v13; // [sp+80h] [-110h]
    float v14[4]; // [sp+88h] [-108h] BYREF
    float v15[6]; // [sp+98h] [-F8h] BYREF
    float v16[12]; // [sp+B0h] [-E0h] BYREF
    float v17[4][3]; // [sp+E0h] [-B0h] BYREF
    trace_t v18; // [sp+110h] [-80h] BYREF

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1637, 0, "%s", "pTurretInfo");
    LocalTagMatrix = G_DObjGetLocalTagMatrix(self, scr_const.tag_aim);
    if (LocalTagMatrix)
    {
        v4 = G_DObjGetLocalTagMatrix(self, scr_const.tag_butt);
        if (v4)
        {
            AnglesToAxis(self->r.currentAngles, (float (*)[3])v16);
            v16[9] = self->r.currentOrigin[0];
            v16[10] = self->r.currentOrigin[1];
            v16[11] = self->r.currentOrigin[2];
            v12[0] = v4->trans[0] - LocalTagMatrix->trans[0];
            v5 = (float)(v4->trans[2] - LocalTagMatrix->trans[2]);
            v12[1] = v4->trans[1] - LocalTagMatrix->trans[1];
            v12[2] = v5;
            MatrixTransformVector43(LocalTagMatrix->trans, (const float (*)[3])v16, v15);
            HIDWORD(v6) = "erverTime - cgameGlob->time > 0";
            v7 = 0;
            while (1)
            {
                LODWORD(v6) = v7;
                v11[1] = 0.0;
                v11[2] = 0.0;
                v13 = v6;
                v11[0] = (float)v6 * (float)-3.0;
                AnglesToAxis(v11, v17);
                MatrixTransformVector(v12, v17, &v8);
                v8 = LocalTagMatrix->trans[0] + v8;
                v9 = LocalTagMatrix->trans[1] + v9;
                v10 = LocalTagMatrix->trans[2] + v10;
                MatrixTransformVector43(&v8, (const float (*)[3])v16, v14);
                G_LocationalTrace(&v18, v15, v14, self->s.number, 8390673, bulletPriorityMap);
                if (v18.fraction < 1.0)
                    break;
                if (++v7 > 30)
                    return;
            }
            pTurretInfo->dropPitch = self->s.lerp.u.turret.gunAngles[0] + v11[0];
        }
    }
}

void __cdecl turret_think(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r29
    tagInfo_s *tagInfo; // r9
    gentity_s *v4; // r30
    int flags; // r11
    actor_s *actor; // r30

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1688, 0, "%s", "pTurretInfo");
    tagInfo = self->tagInfo;
    self->nextthink = level.time + 50;
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    if (tagInfo)
        G_GeneralLink(self);
    if (EntHandle::isDefined(&self->r.ownerNum))
        v4 = EntHandle::ent(&self->r.ownerNum);
    else
        v4 = &g_entities[2175];
    if (v4->client)
        return;
    turret_UpdateSound(self);
    flags = pTurretInfo->flags;
    if ((flags & 2) != 0 && (flags & 1) == 0)
    {
        turret_think_auto_nonai(self);
        return;
    }
    actor = v4->actor;
    if (actor)
    {
        if (actor->eState[0] == AIS_TURRET)
        {
            if ((flags & 2) != 0)
            {
                if (turret_think_auto(self, actor))
                    return;
                goto LABEL_24;
            }
            goto LABEL_22;
        }
        actor = 0;
    }
    if ((flags & 1) != 0)
    {
        turret_ReturnToDefaultPos(self, 0);
        return;
    }
    if ((flags & 2) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp",
            1726,
            0,
            "%s",
            "!(pTurretInfo->flags & TURRET_AUTO)");
LABEL_22:
    if (!turret_think_manual(self, actor) && actor)
        LABEL_24 :
        Actor_StopUseTurret(actor);
}

bool __cdecl Actor_IsTurretCloserThenCurrent(actor_s *actor, gentity_s *turret)
{
    double v4; // fp31

    if (!turret)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1745, 0, "%s", "turret");
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1746, 0, "%s", "actor");
    if (!actor->pTurret)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1747, 0, "%s", "actor->pTurret");
    if (actor->pTurret == turret)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1748, 0, "%s", "actor->pTurret != turret");
    v4 = Vec2DistanceSq(actor->ent->r.currentOrigin, turret->r.currentOrigin);
    return Vec2DistanceSq(actor->ent->r.currentOrigin, actor->pTurret->r.currentOrigin) < v4;
}

bool __cdecl turret_canuse_auto(gentity_s *self, actor_s *actor)
{
    TurretInfo *pTurretInfo; // r29
    sentient_s *TargetSentient; // r3
    const sentient_s *v7; // r30
    int v8; // r10
    sentient_info_t *v9; // r31
    const float *currentOrigin; // r3
    double v11; // fp31
    double v12; // fp29
    double v13; // fp30
    double v14; // fp28
    float v17; // [sp+50h] [-C0h] BYREF
    float v18; // [sp+54h] [-BCh]
    float v19[2]; // [sp+58h] [-B8h] BYREF
    float v20; // [sp+60h] [-B0h] BYREF
    float v21; // [sp+64h] [-ACh]
    float v22; // [sp+68h] [-A8h]
    float v23; // [sp+70h] [-A0h] BYREF
    float v24; // [sp+74h] [-9Ch]
    float v25; // [sp+78h] [-98h]
    float v26[4]; // [sp+80h] [-90h] BYREF
    float v27[4]; // [sp+90h] [-80h] BYREF
    float v28[4]; // [sp+A0h] [-70h] BYREF
    float v29[12]; // [sp+B0h] [-60h] BYREF

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1778, 0, "%s", "actor");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1781, 0, "%s", "pTurretInfo");
    if ((unsigned __int8)Actor_IsUsingTurret(actor)
        && actor->pTurret != self
        && !Actor_IsTurretCloserThenCurrent(actor, self))
    {
        return 0;
    }
    TargetSentient = Actor_GetTargetSentient(actor);
    v7 = TargetSentient;
    if (!TargetSentient)
        return 1;
    v8 = TargetSentient - level.sentients;
    v9 = &actor->sentientInfo[v8];
    if (level.time - actor->sentientInfo[v8].lastKnownPosTime >= pTurretInfo->suppressTime)
        return 1;
    currentOrigin = TargetSentient->ent->r.currentOrigin;
    v11 = (float)(*currentOrigin - self->r.currentOrigin[0]);
    v12 = (float)(v7->ent->r.currentOrigin[1] - self->r.currentOrigin[1]);
    v13 = (float)(v7->ent->r.currentOrigin[2] - self->r.currentOrigin[2]);
    v14 = (float)((float)((float)v12 * (float)v12)
        + (float)((float)((float)v13 * (float)v13) + (float)((float)v11 * (float)v11)));
    if (v14 >= pTurretInfo->maxRangeSquared
        || Vec2DistanceSq(currentOrigin, actor->sentientInfo[v8].vLastKnownPos) >= 4096.0)
    {
        return 1;
    }
    if (v9->VisCache.bVisible)
    {
        Sentient_GetEyePosition(v7, v28);
        if (turret_CanTargetSentient(self, v7, v29, &v20, v19))
            return 1;
    }
    else
    {
        AngleVectors(self->r.currentAngles, v26, 0, 0);
        _FP12 = -__fsqrts(v14);
        __asm { fsel      f0, f12, f13, f0 }
        if ((float)((float)(v26[1] * (float)((float)((float)1.0 / (float)_FP0) * (float)v12))
            + (float)((float)(v26[2] * (float)((float)v13 * (float)((float)1.0 / (float)_FP0)))
                + (float)(v26[0] * (float)((float)((float)1.0 / (float)_FP0) * (float)v11)))) >= (double)pTurretInfo->forwardAngleDot)
            return 1;
    }
    if (!G_DObjGetWorldTagPos(self, scr_const.tag_weapon, &v23))
        return 0;
    v20 = v23;
    v21 = v24;
    if (!G_DObjGetWorldTagPos(self, scr_const.tag_aim, &v23))
        return 0;
    v22 = v25;
    v17 = v20 - v23;
    v18 = v21 - v24;
    Vec2Normalize(&v17);
    v20 = (float)(v17 * (float)30.0) + v20;
    v21 = (float)(v18 * (float)30.0) + v21;
    Sentient_GetEyePosition(v7, v27);
    return (_cntlzw(turret_SightTrace(&v20, v27, actor->ent->s.number, v7->ent->s.number)) & 0x20) == 0;
}

bool __cdecl turret_canuse_manual(gentity_s *self, actor_s *actor)
{
    sentient_s *TargetSentient; // r3
    bool result; // r3
    double v6; // fp0
    double v7; // fp13
    double v8; // fp12

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1852, 0, "%s", "actor");
    if (!self->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1855, 0, "%s", "pTurretInfo");
    TargetSentient = Actor_GetTargetSentient(actor);
    result = !TargetSentient
        || (v6 = (float)(actor->ent->r.currentOrigin[0] - TargetSentient->ent->r.currentOrigin[0]),
            v7 = (float)(actor->ent->r.currentOrigin[2] - TargetSentient->ent->r.currentOrigin[2]),
            v8 = (float)(actor->ent->r.currentOrigin[1] - TargetSentient->ent->r.currentOrigin[1]),
            (float)((float)((float)v8 * (float)v8)
                + (float)((float)((float)v6 * (float)v6) + (float)((float)v7 * (float)v7))) >= 65536.0)
        || turret_canuse_auto(self, actor);
    return result;
}

bool __cdecl turret_canuse(actor_s *actor, gentity_s *pTurret)
{
    TurretInfo *pTurretInfo; // r27

    if (!pTurret)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1878, 0, "%s", "pTurret");
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1879, 0, "%s", "actor");
    pTurretInfo = pTurret->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1882, 0, "%s", "pTurretInfo");
    if ((unsigned __int8)Actor_IsUsingTurret(actor) && actor->pTurret == pTurret)
        return 1;
    if (pTurret->active)
        return 0;
    if ((pTurretInfo->flags & 2) != 0)
        return turret_canuse_auto(pTurret, actor);
    return turret_canuse_manual(pTurret, actor);
}

void __cdecl turret_controller(const gentity_s *self, int *partBits)
{
    int number; // r3
    double v5; // fp12
    double v6; // fp0
    const DObj_s *ServerDObj; // r31
    unsigned __int8 v8[8]; // [sp+50h] [-50h] BYREF
    float v9; // [sp+58h] [-48h] BYREF
    float v10; // [sp+5Ch] [-44h]
    float v11; // [sp+60h] [-40h]

    v5 = self->s.lerp.u.turret.gunAngles[1];
    v6 = (float)(self->s.lerp.u.turret.gunAngles[0] - self->s.lerp.u.turret.gunAngles[2]);
    number = self->s.number;
    v10 = v5;
    v9 = v6;
    v11 = 0.0;
    ServerDObj = Com_GetServerDObj(number);
    if (!ServerDObj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1906, 0, "%s", "obj");
    v8[0] = -2;
    DObjGetBoneIndex(ServerDObj, scr_const.tag_aim, v8);
    DObjSetControlTagAngles(ServerDObj, partBits, v8[0], &v9);
    v8[0] = -2;
    DObjGetBoneIndex(ServerDObj, scr_const.tag_aim_animated, v8);
    DObjSetControlTagAngles(ServerDObj, partBits, v8[0], &v9);
    v9 = self->s.lerp.u.turret.gunAngles[2];
    v10 = 0.0;
    v8[0] = -2;
    DObjGetBoneIndex(ServerDObj, scr_const.tag_flash, v8);
    DObjSetControlTagAngles(ServerDObj, partBits, v8[0], &v9);
}

void __cdecl SP_turret_XAnimPrecache(ScriptFunctions *functions, const char *classname)
{
    unsigned int WeaponIndexForName; // r31
    WeaponDef *WeaponDef; // r28
    const char *szScript; // r5
    unsigned int v7; // r11
    const char *v8; // [sp+50h] [-30h] BYREF

    G_LevelSpawnString("weaponinfo", byte_82003CDD, &v8);
    WeaponIndexForName = G_GetWeaponIndexForName(v8);
    if (!WeaponIndexForName)
        Com_Error(ERR_DROP, byte_820444E4, v8, classname);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    if (WeaponDef->weapClass != WEAPCLASS_TURRET)
        Com_Error(ERR_DROP, byte_820444AC, v8, classname);
    szScript = WeaponDef->szScript;
    if (*szScript)
    {
        GScr_SetSingleAnimScript(functions, &g_scr_data.anim.weapons[WeaponIndexForName], szScript);
    }
    else
    {
        v7 = WeaponIndexForName;
        g_scr_data.anim.weapons[v7].func = 0;
        g_scr_data.anim.weapons[v7].name = 0;
    }
}

bool __cdecl turret_behind(gentity_s *self, gentity_s *other, long double a3)
{
    TurretInfo *pTurretInfo; // r29
    double v6; // fp13
    double v7; // fp31
    double v8; // fp30
    long double v9; // fp2
    float *v10; // r3
    double v12; // fp13
    double v13; // fp12
    double v15; // fp11
    double v18; // fp12
    float v20; // [sp+50h] [-40h] BYREF
    float v21; // [sp+54h] [-3Ch]
    float v22; // [sp+58h] [-38h]

    pTurretInfo = self->pTurretInfo;
    if (!other->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1963, 0, "%s", "other->client");
    v6 = (float)((float)__fabs(pTurretInfo->arcmax[1]) + (float)__fabs(pTurretInfo->arcmin[1]));
    v7 = (float)((float)v6 * (float)0.5);
    v8 = (float)((float)((float)(self->r.currentAngles[1] + pTurretInfo->arcmin[1]) + (float)((float)v6 * (float)0.5))
        * (float)0.0027777778);
    *(double *)&a3 = (float)((float)((float)((float)(self->r.currentAngles[1] + pTurretInfo->arcmin[1])
        + (float)((float)v6 * (float)0.5))
        * (float)0.0027777778)
        + (float)0.5);
    v9 = floor(a3);
    YawVectors((float)((float)((float)v8 - (float)*(double *)&v9) * (float)360.0), v10, &v20);
    _FP7 = -__fsqrts((float)((float)(v21 * v21) + (float)((float)(v22 * v22) + (float)(v20 * v20))));
    v12 = (float)(self->r.currentOrigin[0] - other->r.currentOrigin[0]);
    v13 = (float)(self->r.currentOrigin[1] - other->r.currentOrigin[1]);
    __asm { fsel      f11, f7, f0, f11 }
    v15 = (float)((float)1.0 / (float)_FP11);
    v20 = v20 * (float)v15;
    _FP6 = -__fsqrts((float)((float)((float)v13 * (float)v13) + (float)((float)v12 * (float)v12)));
    v22 = v22 * (float)v15;
    __asm { fsel      f10, f6, f0, f9 }
    v18 = (float)((float)((float)v13 * (float)((float)1.0 / (float)_FP10)) * (float)((float)v15 * v21));
    v21 = (float)v15 * v21;
    return (float)(Q_acos((float)((float)(v22 * (float)((float)((float)1.0 / (float)_FP10) * (float)0.0))
        + (float)((float)((float)((float)((float)1.0 / (float)_FP10) * (float)v12) * v20)
            + (float)v18)))
        * (float)57.295776) <= v7;
}

bool __cdecl G_IsTurretUsable(gentity_s *self, gentity_s *owner, long double a3)
{
    TurretInfo *pTurretInfo; // r30
    gclient_s *client; // r11

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2015, 0, "%s", "pTurretInfo");
    if ((pTurretInfo->flags & 0x1000) != 0
        && !self->active
        && self->pTurretInfo
        && turret_behind(self, owner, a3)
        && (client = owner->client, !client->ps.grenadeTimeLeft))
    {
        return (_cntlzw(client->ps.groundEntityNum - 2175) & 0x20) == 0;
    }
    else
    {
        return 0;
    }
}

void __cdecl G_DeactivateTurret(gentity_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2037, 0, "%s", "self");
    if (self->active == 1 && Scr_IsSystemActive(1u))
        Scr_Notify(self, scr_const.turret_deactivate, 0);
    self->active = 0;
}

void __cdecl turret_use(gentity_s *self, gentity_s *owner, gentity_s *activator)
{
    TurretInfo *pTurretInfo; // r31
    gclient_s *client; // r30
    long double v7; // fp2
    int flags; // r11
    double initialYawmax; // fp13
    int pm_flags; // r11
    int v11; // r11
    int stance; // r11
    unsigned int v13; // r11
    int eFlags; // r11
    bool v15; // zf
    double v16; // fp29
    long double v17; // fp2
    double v18; // fp11
    double v23; // fp30
    long double v24; // fp2
    double v25; // fp13
    double v26; // fp12
    double v31; // fp0
    double v32; // fp13
    double v33; // fp0
    double v34; // fp13

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2052, 0, "%s", "self");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2055, 0, "%s", "pTurretInfo");
    if (!owner)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2057, 0, "%s", "owner");
    if (!owner->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2058, 0, "%s", "owner->client");
    if (owner->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2059, 0, "%s", "!owner->actor");
    if (owner->s.number >= level.maxclients)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp",
            2060,
            0,
            "%s",
            "owner->s.number < level.maxclients");
    client = owner->client;
    owner->active = 1;
    self->active = 1;
    EntHandle::setEnt(&self->r.ownerNum, owner);
    client->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    client->ps.viewlocked_entNum = self->s.number;
    flags = pTurretInfo->flags;
    initialYawmax = pTurretInfo->initialYawmax;
    pTurretInfo->arcmin[1] = pTurretInfo->initialYawmin;
    pTurretInfo->arcmax[1] = initialYawmax;
    pTurretInfo->flags = flags | 0x800;
    pTurretInfo->userOrigin[0] = owner->r.currentOrigin[0];
    pTurretInfo->userOrigin[1] = owner->r.currentOrigin[1];
    pTurretInfo->userOrigin[2] = owner->r.currentOrigin[2];
    pm_flags = client->ps.pm_flags;
    if ((pm_flags & 1) != 0)
    {
        v11 = 2;
    }
    else
    {
        if ((pm_flags & 2) != 0)
        {
            pTurretInfo->prevStance = 1;
            goto LABEL_19;
        }
        v11 = 0;
    }
    pTurretInfo->prevStance = v11;
LABEL_19:
    stance = pTurretInfo->stance;
    if (stance == 2)
    {
        v13 = client->ps.eFlags & 0xFFFFFCFF | 0x100;
    }
    else
    {
        v15 = stance != 1;
        eFlags = client->ps.eFlags;
        if (v15)
            v13 = eFlags | 0x300;
        else
            v13 = __ROL4__(1, 9) & 0x300 | eFlags & 0xFFFFFCFF;
    }
    client->ps.eFlags = v13;
    v16 = (float)((float)(client->ps.viewangles[0] - self->r.currentAngles[0]) * (float)0.0027777778);
    *(double *)&v7 = (float)((float)((float)(client->ps.viewangles[0] - self->r.currentAngles[0]) * (float)0.0027777778)
        + (float)0.5);
    v17 = floor(v7);
    v18 = self->r.currentAngles[1];
    self->s.lerp.u.turret.gunAngles[0] = (float)((float)v16 - (float)*(double *)&v17) * (float)360.0;
    _FP10 = (float)((float)((float)((float)v16 - (float)*(double *)&v17) * (float)360.0) - pTurretInfo->arcmax[0]);
    _FP9 = (float)(pTurretInfo->arcmin[0] - (float)((float)((float)v16 - (float)*(double *)&v17) * (float)360.0));
    __asm
    {
        fsel      f0, f10, f13, f0
        fsel      f0, f9, f12, f0
    }
    self->s.lerp.u.turret.gunAngles[0] = _FP0;
    v23 = (float)((float)(client->ps.viewangles[1] - (float)v18) * (float)0.0027777778);
    *(double *)&v17 = (float)((float)((float)(client->ps.viewangles[1] - (float)v18) * (float)0.0027777778) + (float)0.5);
    v24 = floor(v17);
    self->s.lerp.u.turret.gunAngles[1] = (float)((float)v23 - (float)*(double *)&v24) * (float)360.0;
    v25 = pTurretInfo->arcmax[1];
    v26 = pTurretInfo->arcmin[1];
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    _FP11 = (float)((float)((float)((float)v23 - (float)*(double *)&v24) * (float)360.0) - (float)v25);
    _FP10 = (float)((float)v26 - (float)((float)((float)v23 - (float)*(double *)&v24) * (float)360.0));
    __asm
    {
        fsel      f0, f11, f13, f0
        fsel      f0, f10, f12, f0
    }
    self->s.lerp.u.turret.gunAngles[1] = _FP0;
    client->ps.viewAngleClampRange[0] = (float)(pTurretInfo->arcmax[0] - pTurretInfo->arcmin[0]) * (float)0.5;
    v31 = (float)(self->r.currentAngles[0] + pTurretInfo->arcmax[0]);
    v32 = client->ps.viewAngleClampRange[0];
    client->ps.viewAngleClampBase[0] = self->r.currentAngles[0] + pTurretInfo->arcmax[0];
    client->ps.viewAngleClampBase[0] = AngleNormalize360((float)((float)v31 - (float)v32));
    client->ps.viewAngleClampRange[1] = (float)(pTurretInfo->arcmax[1] - pTurretInfo->arcmin[1]) * (float)0.5;
    v33 = (float)(self->r.currentAngles[1] + pTurretInfo->arcmax[1]);
    v34 = client->ps.viewAngleClampRange[1];
    client->ps.viewAngleClampBase[1] = self->r.currentAngles[1] + pTurretInfo->arcmax[1];
    client->ps.viewAngleClampBase[1] = AngleNormalize360((float)((float)v33 - (float)v34));
    Scr_Notify(self, scr_const.turretownerchange, 0);
}

int __cdecl G_CanSpawnTurret()
{
    TurretInfo *v0; // r11

    v0 = turretInfoStore;
    while (v0->inuse)
    {
        if ((int)++v0 >= (int)&marker_turret)
            return 0;
    }
    return 1;
}

void __cdecl G_SpawnTurret(gentity_s *self, const char *weaponinfoname)
{
    int v4; // r10
    TurretInfo *i; // r11
    TurretInfo *v6; // r31
    unsigned __int8 WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r27
    long double v9; // fp2
    const char *v10; // r3
    const char *v11; // r3
    weapStance_t stance; // r11
    const char **p_aliasName; // r11
    const char **v14; // r11
    const char **v15; // r11
    const char **v16; // r11
    float *v17; // r29
    double v18; // fp0
    bool spawnVarsValid; // r11
    float *v20; // r29
    double v21; // fp0
    long double v26; // fp2
    bool v27; // r11
    float *arcmin; // r29
    double v29; // fp0
    float *arcmax; // r29
    double yawConvergenceTime; // fp0
    bool v32; // cr58
    double pitchConvergenceTime; // fp0
    bool v34; // r10
    double suppressTime; // fp0
    bool v36; // r10
    double maxRange; // fp0
    double v38; // fp0
    float *p_accuracy; // r29
    double v40; // fp0
    float *p_aiSpread; // r29
    float *p_playerSpread; // r29
    int flags; // r9
    int time; // r11
    int number; // r11
    float v46; // [sp+50h] [-80h] BYREF
    float v47; // [sp+54h] [-7Ch] BYREF
    float v48[4]; // [sp+58h] [-78h] BYREF

    v4 = 0;
    for (i = &turretInfoStore[1]; (int)i < (int)&unk_832A3074; i += 4)
    {
        v6 = i - 1;
        if (!i[-1].inuse)
            break;
        v6 = i;
        if (!i->inuse)
        {
            ++v4;
            break;
        }
        v6 = i + 1;
        if (!i[1].inuse)
        {
            v4 += 2;
            break;
        }
        v6 = i + 2;
        if (!i[2].inuse)
        {
            v4 += 3;
            break;
        }
        v4 += 4;
    }
    if (v4 == 32)
        Com_Error(ERR_DROP, byte_820446CC, 32);
    memset(v6, 0, sizeof(TurretInfo));
    self->pTurretInfo = v6;
    v6->inuse = 1;
    WeaponIndexForName = G_GetWeaponIndexForName(weaponinfoname);
    self->s.weaponModel = 0;
    self->s.weapon = WeaponIndexForName;
    if (!WeaponIndexForName)
        Com_Error(ERR_DROP, byte_820446A0, weaponinfoname);
    WeaponDef = BG_GetWeaponDef(self->s.weapon);
    if (WeaponDef->weapClass != WEAPCLASS_TURRET)
    {
        v10 = va("Could not load turret '%s'", weaponinfoname);
        Scr_Error(v10);
    }
    if (!level.initializing && !IsItemRegistered(self->s.weapon))
    {
        v11 = va("turret '%s' not precached", weaponinfoname);
        Scr_Error(v11);
    }
    if (*WeaponDef->szScript && !*(int *)((char *)&g_scr_data.anim.weapons[0].func + __ROL4__(self->s.weapon, 3)))
        Com_Error(ERR_DROP, byte_82044618, weaponinfoname);
    v6->fireTime = 0;
    stance = WeaponDef->stance;
    v6->fireSndDelay = 0;
    v6->prevStance = -1;
    v6->stance = stance;
    p_aliasName = &WeaponDef->fireLoopSound->aliasName;
    if (p_aliasName)
        v6->fireSnd = G_SoundAliasIndexPermanent(*p_aliasName);
    else
        v6->fireSnd = 0;
    v14 = &WeaponDef->fireLoopSoundPlayer->aliasName;
    if (v14)
        v6->fireSndPlayer = G_SoundAliasIndexPermanent(*v14);
    else
        v6->fireSndPlayer = 0;
    v15 = &WeaponDef->fireStopSound->aliasName;
    if (v15)
        v6->stopSnd = G_SoundAliasIndexPermanent(*v15);
    else
        v6->stopSnd = 0;
    v16 = &WeaponDef->fireStopSoundPlayer->aliasName;
    if (v16)
        v6->stopSndPlayer = G_SoundAliasIndexPermanent(*v16);
    else
        v6->stopSndPlayer = 0;
    if (!level.spawnVar.spawnVarsValid
        || (v17 = &v6->arcmin[1], !G_SpawnFloat("rightarc", byte_82003CDD, &v6->arcmin[1])))
    {
        v17 = &v6->arcmin[1];
        v6->arcmin[1] = WeaponDef->rightArc;
    }
    v18 = (float)(*v17 * (float)-1.0);
    *v17 = *v17 * (float)-1.0;
    if (v18 > 0.0)
        *v17 = 0.0;
    spawnVarsValid = level.spawnVar.spawnVarsValid;
    v6->initialYawmin = *v17;
    if (!spawnVarsValid || (v20 = &v6->arcmax[1], !G_SpawnFloat("leftarc", byte_82003CDD, &v6->arcmax[1])))
    {
        v20 = &v6->arcmax[1];
        v6->arcmax[1] = WeaponDef->leftArc;
    }
    if (*v20 < 0.0)
        *v20 = 0.0;
    v21 = (float)(*v20 - v6->initialYawmin);
    v6->initialYawmax = *v20;
    v6->forwardAngleDot = v21;
    _FP11 = (float)((float)v21 - (float)180.0);
    _FP10 = (float)((float)90.0 - (float)v21);
    __asm { fsel      f0, f11, f13, f0 }
    __asm { fsel      f0, f10, f12, f0 }
    v6->forwardAngleDot = _FP0;
    *(double *)&v9 = (float)((float)_FP0 * (float)0.017453292);
    v26 = cos(v9);
    v27 = level.spawnVar.spawnVarsValid;
    v6->forwardAngleDot = *(double *)&v26;
    if (!v27 || (arcmin = v6->arcmin, !G_SpawnFloat("toparc", byte_82003CDD, v6->arcmin)))
    {
        arcmin = v6->arcmin;
        v6->arcmin[0] = WeaponDef->topArc;
    }
    v29 = (float)(*arcmin * (float)-1.0);
    *arcmin = *arcmin * (float)-1.0;
    if (v29 > 0.0)
        *arcmin = 0.0;
    if (!level.spawnVar.spawnVarsValid || (arcmax = v6->arcmax, !G_SpawnFloat("bottomarc", byte_82003CDD, v6->arcmax)))
    {
        arcmax = v6->arcmax;
        v6->arcmax[0] = WeaponDef->bottomArc;
    }
    if (*arcmax < 0.0)
        *arcmax = 0.0;
    if (level.spawnVar.spawnVarsValid && G_SpawnFloat("yawconvergencetime", byte_82003CDD, &v46)
        || level.spawnVar.spawnVarsValid && G_SpawnFloat("convergencetime", byte_82003CDD, &v46))
    {
        yawConvergenceTime = v46;
    }
    else
    {
        yawConvergenceTime = WeaponDef->yawConvergenceTime;
        v46 = WeaponDef->yawConvergenceTime;
    }
    if (yawConvergenceTime < 0.0)
    {
        yawConvergenceTime = 0.0;
        v46 = 0.0;
    }
    v32 = !level.spawnVar.spawnVarsValid;
    v6->convergenceTime[1] = (int)(float)((float)((float)yawConvergenceTime * (float)1000.0) + (float)0.5);
    if (!v32 && G_SpawnFloat("pitchconvergencetime", byte_82003CDD, &v46))
    {
        pitchConvergenceTime = v46;
    }
    else
    {
        pitchConvergenceTime = WeaponDef->pitchConvergenceTime;
        v46 = WeaponDef->pitchConvergenceTime;
    }
    if (pitchConvergenceTime < 0.0)
    {
        pitchConvergenceTime = 0.0;
        v46 = 0.0;
    }
    v34 = level.spawnVar.spawnVarsValid;
    v6->convergenceTime[0] = (int)(float)((float)((float)pitchConvergenceTime * (float)1000.0) + (float)0.5);
    if (v34 && G_SpawnFloat("suppressionTime", byte_82003CDD, &v47))
    {
        suppressTime = v47;
    }
    else
    {
        suppressTime = WeaponDef->suppressTime;
        v47 = WeaponDef->suppressTime;
    }
    if (suppressTime < 0.0)
    {
        suppressTime = 0.0;
        v47 = 0.0;
    }
    v36 = level.spawnVar.spawnVarsValid;
    v6->suppressTime = (int)(float)((float)((float)suppressTime * (float)1000.0) + (float)0.5);
    if (v36 && G_SpawnFloat("maxrange", byte_82003CDD, v48))
    {
        maxRange = v48[0];
    }
    else
    {
        maxRange = WeaponDef->maxRange;
        v48[0] = WeaponDef->maxRange;
    }
    if (maxRange <= 0.0)
        v38 = 3.4028235e38;
    else
        v38 = (float)((float)maxRange * (float)maxRange);
    v6->maxRangeSquared = v38;
    v6->dropPitch = -90.0;
    if (!self->health)
        self->health = 100;
    if (!level.spawnVar.spawnVarsValid || (p_accuracy = &v6->accuracy, !G_SpawnFloat("accuracy", "1", &v6->accuracy)))
    {
        p_accuracy = &v6->accuracy;
        v6->accuracy = WeaponDef->accuracy;
    }
    v40 = *p_accuracy;
    if (v40 >= 0.0)
    {
        if (v40 > 1.0)
            *p_accuracy = 1.0;
    }
    else
    {
        *p_accuracy = 0.0;
    }
    if (!level.spawnVar.spawnVarsValid || (p_aiSpread = &v6->aiSpread, !G_SpawnFloat("aiSpread", "1", &v6->aiSpread)))
    {
        p_aiSpread = &v6->aiSpread;
        v6->aiSpread = WeaponDef->aiSpread;
    }
    if (*p_aiSpread < 0.0)
        *p_aiSpread = 0.0;
    if (!level.spawnVar.spawnVarsValid
        || (p_playerSpread = &v6->playerSpread, !G_SpawnFloat("playerSpread", "1", &v6->playerSpread)))
    {
        p_playerSpread = &v6->playerSpread;
        v6->playerSpread = WeaponDef->playerSpread;
    }
    if (*p_playerSpread < 0.0)
        *p_playerSpread = 0.0;
    v6->state = 0;
    v6->prevSentTarget = -1;
    v6->flags = 4099;
    v6->eTeam = TEAM_NEUTRAL;
    flags = self->flags;
    self->clipmask = 1;
    self->r.svFlags = 0;
    self->s.eType = 10;
    self->r.contents = 2097156;
    self->flags = flags | 0x800;
    G_DObjUpdate(self);
    self->r.mins[2] = 0.0;
    self->r.mins[0] = -32.0;
    self->r.mins[1] = -32.0;
    self->r.maxs[0] = 32.0;
    self->r.maxs[1] = 32.0;
    self->r.maxs[2] = 56.0;
    G_SetOrigin(self, self->r.currentOrigin);
    G_SetAngle(self, self->r.currentAngles);
    self->s.lerp.u.turret.gunAngles[0] = 0.0;
    self->s.lerp.u.turret.gunAngles[1] = 0.0;
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    self->handler = 20;
    time = level.time;
    self->s.lerp.apos.trType = TR_LINEAR_STOP;
    self->takedamage = 0;
    self->nextthink = time + 50;
    number = self->r.ownerNum.number;
    if (self->r.ownerNum.number && !g_entities[number - 1].r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_public.h",
            336,
            0,
            "%s\n\t(number - 1) = %i",
            "(!number || g_entities[number - 1].r.inuse)",
            number - 1);
    if (self->r.ownerNum.number)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 2308, 0, "%s", "!self->r.ownerNum.isDefined()");
    SV_LinkEntity(self);
}

void __cdecl SP_turret(gentity_s *self)
{
    const char *v2; // [sp+50h] [-20h] BYREF

    if (!G_LevelSpawnString("weaponinfo", byte_82003CDD, &v2))
        Com_Error(ERR_DROP, byte_82044700);
    G_SpawnTurret(self, v2);
}

void __cdecl G_ClientStopUsingTurret(gentity_s *self)
{
    TurretInfo *pTurretInfo; // r30
    gentity_s *v3; // r31
    int prevStance; // r11
    unsigned int v5; // r4

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 409, 0, "%s", "pTurretInfo");
    if (!EntHandle::isDefined(&self->r.ownerNum))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 411, 0, "%s", "self->r.ownerNum.isDefined()");
    v3 = EntHandle::ent(&self->r.ownerNum);
    if (!v3->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 414, 0, "%s", "owner->client");
    pTurretInfo->fireSndDelay = 0;
    self->s.loopSound = 0;
    prevStance = pTurretInfo->prevStance;
    if (prevStance != -1)
    {
        if (prevStance == 2)
        {
            v5 = 8;
        }
        else
        {
            v5 = 7;
            if (prevStance != 1)
                v5 = 6;
        }
        G_AddEvent(v3, v5, 0);
        pTurretInfo->prevStance = -1;
    }
    TeleportPlayer(v3, pTurretInfo->userOrigin, v3->r.currentAngles);
    v3->client->ps.eFlags &= 0xFFFFFCFF;
    v3->client->ps.viewlocked = PLAYERVIEWLOCK_NONE;
    v3->client->ps.viewlocked_entNum = 2175;
    v3->active = 0;
    G_DeactivateTurret(self);
    EntHandle::setEnt(&self->r.ownerNum, 0);
    pTurretInfo->flags &= ~0x800u;
    Scr_Notify(self, scr_const.turretownerchange, 0);
}

void __cdecl turret_think_client(gentity_s *self)
{
    EntHandle *p_ownerNum; // r29
    gentity_s *v3; // r29

    p_ownerNum = &self->r.ownerNum;
    if (!EntHandle::isDefined(&self->r.ownerNum))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 493, 0, "%s", "self->r.ownerNum.isDefined()");
    v3 = EntHandle::ent(p_ownerNum);
    if (!v3->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 497, 0, "%s", "owner->client");
    if (v3->active == 1 && v3->health > 0)
    {
        if (!self->active)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 501, 0, "%s", "self->active");
        turret_track(self, v3);
        turret_UpdateSound(self);
    }
    else
    {
        G_ClientStopUsingTurret(self);
    }
}

void __cdecl turret_think_init(gentity_s *self)
{
    tagInfo_s *tagInfo; // r9
    int time; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1611, 0, "%s", "self");
    if (self->handler != 20)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp",
            1612,
            0,
            "%s",
            "self->handler == ENT_HANDLER_TURRET_INIT");
    tagInfo = self->tagInfo;
    self->handler = 21;
    time = level.time;
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    self->nextthink = time + 50;
    if (tagInfo)
        G_GeneralLink(self);
    if (!self->pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1621, 0, "%s", "self->pTurretInfo");
    if (self->pTurretInfo->dropPitch == -90.0)
        turret_RestoreDefaultDropPitch(self);
}

void __cdecl G_FreeTurret(gentity_s *self)
{
    EntHandle *p_ownerNum; // r31
    gentity_s *v3; // r30
    TurretInfo *pTurretInfo; // r31
    actor_s *actor; // r3

    p_ownerNum = &self->r.ownerNum;
    if (EntHandle::isDefined(&self->r.ownerNum))
        v3 = EntHandle::ent(p_ownerNum);
    else
        v3 = &g_entities[2175];
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\turret.cpp", 1992, 0, "%s", "pTurretInfo");
    if (v3->client)
    {
        G_ClientStopUsingTurret(self);
    }
    else
    {
        actor = v3->actor;
        if (actor)
            Actor_StopUseTurret(actor);
    }
    G_DeactivateTurret(self);
    EntHandle::setEnt(&pTurretInfo->manualTarget, 0);
    EntHandle::setEnt(&pTurretInfo->target, 0);
    SentientHandle::setSentient(&pTurretInfo->detachSentient, 0);
    pTurretInfo->inuse = 0;
    self->pTurretInfo = 0;
}

