#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"
#include "g_utils_mp.h"
#include <server/sv_world.h>
#include <script/scr_vm.h>
#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <game/bullet.h>


void __cdecl SP_info_notnull(gentity_s *self)
{
    G_SetOrigin(self, self->r.currentOrigin);
}

void __cdecl SP_light(gentity_s *self)
{
    float radius; // [esp+8h] [ebp-C4h]
    float v2; // [esp+14h] [ebp-B8h]
    int32_t primaryLightIndex; // [esp+A0h] [ebp-2Ch] BYREF
    const ComPrimaryLight *light; // [esp+A4h] [ebp-28h]
    float facingDir[3]; // [esp+A8h] [ebp-24h] BYREF
    float facingAngles[3]; // [esp+B4h] [ebp-18h] BYREF
    float normalizedColor[3]; // [esp+C0h] [ebp-Ch] BYREF

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 29, 0, "%s", "level.spawnVar.spawnVarsValid");
    if (G_SpawnInt("pl#", "0", &primaryLightIndex))
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        self->s.index.brushmodel = (uint16_t)primaryLightIndex;
        if (self->s.index.brushmodel != primaryLightIndex)
            MyAssertHandler(
                ".\\game_mp\\g_misc_mp.cpp",
                39,
                1,
                "self->s.index.primaryLight == primaryLightIndex\n\t%i, %i",
                self->s.index.brushmodel,
                primaryLightIndex);
        self->s.lerp.u.turret.gunAngles[1] = ColorNormalize((float*)light->color, normalizedColor);
        Byte4PackRgba(normalizedColor, (uint8_t *)&self->s.lerp.u);
        self->s.lerp.u.primaryLight.colorAndExp[3] = light->exponent;
        self->s.lerp.u.turret.gunAngles[2] = light->radius;
        self->s.lerp.u.primaryLight.cosHalfFovOuter = light->cosHalfFovOuter;
        self->s.lerp.u.primaryLight.cosHalfFovInner = light->cosHalfFovInner;
        facingDir[0] = -light->dir[0];
        facingDir[1] = -light->dir[1];
        facingDir[2] = -light->dir[2];
        vectoangles(facingDir, facingAngles);
        G_SetAngle(self, facingAngles);
        G_SetOrigin(self, light->origin);
        v2 = -light->radius;
        self->r.mins[0] = v2;
        self->r.mins[1] = v2;
        self->r.mins[2] = v2;
        radius = light->radius;
        self->r.maxs[0] = radius;
        self->r.maxs[1] = radius;
        self->r.maxs[2] = radius;
        self->s.eType = ET_PRIMARY_LIGHT;
        if (self->r.contents)
            MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 58, 1, "%s", "self->r.contents == 0");
        self->handler = 20;
        SV_LinkEntity(self);
    }
    else
    {
        G_FreeEntity(self);
    }
}

void __cdecl TeleportPlayer(gentity_s *player, float *origin, float *angles)
{
    float *v3; // [esp+4h] [ebp-Ch]
    float *v4; // [esp+8h] [ebp-8h]
    int32_t linked; // [esp+Ch] [ebp-4h]

    if (!player->client)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 75, 0, "%s", "player->client");
    if (player->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 76, 0, "%s", "player->client->sess.connected != CON_DISCONNECTED");
    linked = player->r.linked;
    SV_UnlinkEntity(player);
    v4 = player->client->ps.origin;
    *v4 = *origin;
    v4[1] = origin[1];
    v4[2] = origin[2];
    player->client->ps.origin[2] = player->client->ps.origin[2] + 1.0;
    player->client->ps.eFlags ^= 2u;
    SetClientViewAngle(player, angles);
    if (!player->tagInfo)
        player->r.currentAngles[0] = 0.0;
    BG_PlayerStateToEntityState(&player->client->ps, &player->s, 1, 1u);
    v3 = player->client->ps.origin;
    player->r.currentOrigin[0] = *v3;
    player->r.currentOrigin[1] = v3[1];
    player->r.currentOrigin[2] = v3[2];
    if (linked)
        SV_LinkEntity(player);
}

turretInfo_s turretInfo[32];

void __cdecl G_InitTurrets()
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 32; ++i)
        turretInfo[i].inuse = 0;
}

void __cdecl G_ClientStopUsingTurret(gentity_s *self)
{
    gentity_s *owner; // [esp+0h] [ebp-Ch]
    turretInfo_s *pTurretInfo; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    
    iassert(pTurretInfo);
    iassert(self->r.ownerNum.isDefined());

    owner = self->r.ownerNum.ent();

    iassert(owner->client);

    pTurretInfo->fireSndDelay = 0;
    self->s.loopSound = 0;
    if (pTurretInfo->prevStance != -1)
    {
        ps = &owner->client->ps;
        if (pTurretInfo->prevStance == 2)
        {
            ps->pm_flags &= ~2u;
            ps->pm_flags |= 1u;
            ps->viewHeightTarget = 11;
            G_AddEvent(owner, 8u, 0);
        }
        else if (pTurretInfo->prevStance == 1)
        {
            ps->pm_flags &= ~1u;
            ps->pm_flags |= 2u;
            ps->viewHeightTarget = 40;
            G_AddEvent(owner, 7u, 0);
        }
        else
        {
            ps->pm_flags &= 0xFFFFFFFC;
            ps->viewHeightTarget = 60;
            G_AddEvent(owner, 6u, 0);
        }
        pTurretInfo->prevStance = -1;
    }
    TeleportPlayer(owner, pTurretInfo->userOrigin, owner->r.currentAngles);
    owner->client->ps.eFlags &= 0xFFFFFCFF;
    owner->client->ps.viewlocked = PLAYERVIEWLOCK_NONE;
    owner->client->ps.viewlocked_entNum = ENTITYNUM_NONE;
    owner->active = 0;
    owner->s.otherEntityNum = 0;
    self->active = 0;
    self->r.ownerNum.setEnt(0);
    pTurretInfo->flags &= ~0x800u;
}

void __cdecl turret_think_client(gentity_s *self)
{
    gentity_s *owner; // [esp+0h] [ebp-4h]

    iassert(self->r.ownerNum.isDefined());
    owner = self->r.ownerNum.ent();
    if (!owner->client)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 596, 0, "%s", "owner->client");
    if (owner->active != 1 || owner->client->sess.sessionState || owner->client->ps.pm_type == PM_LASTSTAND)
    {
        G_ClientStopUsingTurret(self);
    }
    else
    {
        if (!self->active)
            MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 600, 0, "%s", "self->active");
        turret_track(self, owner);
        turret_UpdateSound(self);
    }
}

void __cdecl turret_track(gentity_s *self, gentity_s *other)
{
    turretInfo_s *turretInfo; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    turretInfo = self->pTurretInfo;
    iassert(turretInfo);
    iassert(self->active);
    iassert(self->r.ownerNum.isDefined());
    iassert(self->r.ownerNum.ent() == other);
    iassert(other->client);

    turret_clientaim(self, other);
    G_PlayerTurretPositionAndBlend(other, self);
    weapDef = BG_GetWeaponDef(self->s.weapon);
    other->client->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    self->s.lerp.eFlags &= ~0x40u;
    turretInfo->fireTime -= 50;
    if (turretInfo->fireTime <= 0)
    {
        turretInfo->fireTime = 0;
        if ((other->client->ps.pm_flags & 0x800) != 0 || (other->client->buttons & 1) == 0)
        {
            turretInfo->triggerDown = 0;
        }
        else if (weapDef->fireType != WEAPON_FIRETYPE_SINGLESHOT || !turretInfo->triggerDown)
        {
            turretInfo->triggerDown = 1;
            turretInfo->fireTime = weapDef->iFireTime;
            turret_shoot_internal(self, other);
            self->s.lerp.eFlags |= 0x40u;
        }
    }
}

void __cdecl G_PlayerTurretPositionAndBlend(gentity_s *ent, gentity_s *pTurretEnt)
{
    char *AnimDebugName; // eax
    char *v3; // eax
    int32_t v4; // eax
    double v5; // st7
    gclient_s *client; // edx
    float v7; // [esp+18h] [ebp-214h]
    float v8; // [esp+1Ch] [ebp-210h]
    float goalWeight; // [esp+20h] [ebp-20Ch]
    float *v10; // [esp+30h] [ebp-1FCh]
    float *v11; // [esp+38h] [ebp-1F4h]
    float *origin; // [esp+3Ch] [ebp-1F0h]
    float v13; // [esp+68h] [ebp-1C4h]
    float v14; // [esp+6Ch] [ebp-1C0h]
    float v15; // [esp+70h] [ebp-1BCh]
    float v16; // [esp+74h] [ebp-1B8h]
    float result[3]; // [esp+78h] [ebp-1B4h] BYREF
    float v18; // [esp+84h] [ebp-1A8h]
    float v19; // [esp+88h] [ebp-1A4h]
    float v20; // [esp+8Ch] [ebp-1A0h]
    float v21; // [esp+90h] [ebp-19Ch]
    float v22; // [esp+94h] [ebp-198h]
    float fHeightRatio; // [esp+98h] [ebp-194h]
    int32_t iPrevBlend; // [esp+9Ch] [ebp-190h]
    float fPrevTransZ; // [esp+A0h] [ebp-18Ch]
    DObj_s *obj; // [esp+A4h] [ebp-188h]
    int32_t numVertChildren; // [esp+A8h] [ebp-184h]
    float trans2[3]; // [esp+ACh] [ebp-180h] BYREF
    float yaw; // [esp+B8h] [ebp-174h]
    float trans[3]; // [esp+BCh] [ebp-170h] BYREF
    float start[3]; // [esp+C8h] [ebp-164h] BYREF
    float end[3]; // [esp+D4h] [ebp-158h] BYREF
    int32_t iBlend; // [esp+E0h] [ebp-14Ch]
    DObjAnimMat *tagMat; // [esp+E4h] [ebp-148h]
    uint32_t heightAnim; // [esp+E8h] [ebp-144h]
    float fDelta; // [esp+ECh] [ebp-140h]
    float fPrevBlend; // [esp+F0h] [ebp-13Ch]
    float rot[2]; // [esp+F4h] [ebp-138h] BYREF
    float tagAxis[3][3]; // [esp+FCh] [ebp-130h] BYREF
    float localAxis[4][3]; // [esp+120h] [ebp-10Ch] BYREF
    uint32_t leafAnim1; // [esp+150h] [ebp-DCh]
    trace_t trace; // [esp+154h] [ebp-D8h] BYREF
    float endpos[3]; // [esp+180h] [ebp-ACh] BYREF
    int32_t numHorChildren; // [esp+18Ch] [ebp-A0h]
    clientInfo_t *ci; // [esp+190h] [ebp-9Ch]
    float tagHeight; // [esp+194h] [ebp-98h]
    int32_t i; // [esp+198h] [ebp-94h]
    uint32_t baseAnim; // [esp+19Ch] [ebp-90h]
    int32_t clientNum; // [esp+1A0h] [ebp-8Ch]
    lerpFrame_t *pLerpAnim; // [esp+1A4h] [ebp-88h]
    WeaponDef *weapDef; // [esp+1A8h] [ebp-84h]
    float fBlend; // [esp+1ACh] [ebp-80h]
    float axis[4][3]; // [esp+1B0h] [ebp-7Ch] BYREF
    XAnimTree_s *pAnimTree; // [esp+1E0h] [ebp-4Ch]
    XAnim_s *pXAnims; // [esp+1E4h] [ebp-48h]
    uint32_t leafAnim2; // [esp+1E8h] [ebp-44h]
    float localYaw; // [esp+1ECh] [ebp-40h]
    float turretAxis[4][3]; // [esp+1F0h] [ebp-3Ch] BYREF
    float vDelta[3]; // [esp+220h] [ebp-Ch] BYREF

    clientNum = ent->s.clientNum;
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\game_mp\\g_misc_mp.cpp",
            230,
            0,
            "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            clientNum,
            64);
    ci = &level_bgs.clientinfo[clientNum];
    if (!ci->infoValid)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 232, 0, "%s", "ci->infoValid");
    pLerpAnim = &ci->legs;
    if (ci->legs.animationNumber && pLerpAnim->animation && (pLerpAnim->animation->flags & 4) != 0)
    {
        tagMat = G_DObjGetLocalTagMatrix(pTurretEnt, scr_const.tag_weapon);
        if (tagMat)
        {
            obj = Com_GetServerDObj(ent->s.number);
            if (obj)
            {
                if (!pTurretEnt->s.weapon)
                    MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 250, 0, "%s", "pTurretEnt->s.weapon");
                weapDef = BG_GetWeaponDef(pTurretEnt->s.weapon);
                if (weapDef->weapClass != WEAPCLASS_TURRET)
                    MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 252, 0, "%s", "weapDef->weapClass == WEAPCLASS_TURRET");
                if (weapDef->fAnimHorRotateInc == 0.0)
                    MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 253, 0, "%s", "weapDef->fAnimHorRotateInc");
                pAnimTree = ci->pXAnimTree;
                pXAnims = level_bgs.animScriptData.animTree.anims;
                baseAnim = pLerpAnim->animationNumber & 0xFFFFFDFF;
                if ((COERCE_UNSIGNED_INT(tagMat->quat[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(tagMat->quat[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(tagMat->quat[2]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(tagMat->quat[3]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                        432,
                        0,
                        "%s",
                        "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                }
                if ((COERCE_UNSIGNED_INT(tagMat->transWeight) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                        433,
                        0,
                        "%s",
                        "!IS_NAN(mat->transWeight)");
                Vec3Scale(tagMat->quat, tagMat->transWeight, result);
                v21 = result[0] * tagMat->quat[0];
                v14 = result[0] * tagMat->quat[1];
                v19 = result[0] * tagMat->quat[2];
                v22 = result[0] * tagMat->quat[3];
                v13 = result[1] * tagMat->quat[1];
                v20 = result[1] * tagMat->quat[2];
                v18 = result[1] * tagMat->quat[3];
                v15 = result[2] * tagMat->quat[2];
                v16 = result[2] * tagMat->quat[3];
                tagAxis[0][0] = 1.0 - (v13 + v15);
                tagAxis[0][1] = v14 + v16;
                tagAxis[0][2] = v19 - v18;
                tagAxis[1][0] = v14 - v16;
                tagAxis[1][1] = 1.0 - (v21 + v15);
                tagAxis[1][2] = v20 + v22;
                tagAxis[2][0] = v19 + v18;
                tagAxis[2][1] = v20 - v22;
                tagAxis[2][2] = 1.0 - (v21 + v13);
                localYaw = vectosignedyaw(tagAxis[0]);
                AnglesToAxis(pTurretEnt->r.currentAngles, turretAxis);
                turretAxis[3][0] = pTurretEnt->r.currentOrigin[0];
                turretAxis[3][1] = pTurretEnt->r.currentOrigin[1];
                turretAxis[3][2] = pTurretEnt->r.currentOrigin[2];
                Vec3Sub(ent->r.currentOrigin, turretAxis[3], vDelta);
                tagHeight = Vec3Dot(vDelta, turretAxis[2]);
                fDelta = tagHeight - tagMat->trans[2];
                numVertChildren = XAnimGetNumChildren(pXAnims, baseAnim);
                fPrevTransZ = 0.0;
                fPrevBlend = 0.0;
                iPrevBlend = 0;
                leafAnim2 = 0;
                if (!numVertChildren)
                {
                    AnimDebugName = XAnimGetAnimDebugName(pXAnims, baseAnim);
                    Com_Error(ERR_DROP, "Player anim %s has no children", AnimDebugName);
                }
                i = 0;
                do
                {
                    heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - 1 - i);
                    numHorChildren = XAnimGetNumChildren(pXAnims, heightAnim);
                    if (!numHorChildren)
                    {
                        v3 = XAnimGetAnimDebugName(pXAnims, heightAnim);
                        Com_Error(ERR_DROP, "Player anim %s has no children", v3);
                    }
                    fBlend = numHorChildren * 0.5 + localYaw / weapDef->fAnimHorRotateInc;
                    if (fBlend >= 0.0)
                    {
                        if (fBlend >= (numHorChildren - 1))
                            fBlend = (numHorChildren - 1);
                    }
                    else
                    {
                        fBlend = 0.0;
                    }
                    v4 = fBlend;
                    iBlend = v4;
                    fBlend = fBlend - v4;
                    leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, v4);
                    XAnimGetAbsDelta(pXAnims, leafAnim1, rot, trans, 0.0);
                    if (fBlend != 0.0)
                    {
                        leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iBlend + 1);
                        XAnimGetAbsDelta(pXAnims, leafAnim2, rot, trans2, 0.0);
                        Vec3Lerp(trans, trans2, fBlend, trans);
                    }
                    if (fDelta <= trans[2])
                        break;
                    fPrevTransZ = trans[2];
                    iPrevBlend = iBlend;
                    fPrevBlend = fBlend;
                    ++i;
                } while (i < numVertChildren);
                XAnimClearTreeGoalWeightsStrict(pAnimTree, baseAnim, 0.0);
                goalWeight = 1.0 - fBlend;
                XAnimSetGoalWeight(obj, leafAnim1, goalWeight, 0.0, 1.0, 0, 0, 0);
                if (fBlend != 0.0)
                    XAnimSetGoalWeight(obj, leafAnim2, fBlend, 0.0, 1.0, 0, 0, 0);
                if (i && i != numVertChildren)
                {
                    if (trans[2] - fPrevTransZ == 0.0)
                        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 329, 0, "%s", "trans[2] - fPrevTransZ");
                    fHeightRatio = (fDelta - fPrevTransZ) / (trans[2] - fPrevTransZ);
                    XAnimSetGoalWeight(obj, heightAnim, fHeightRatio, 0.0, 1.0, 0, 0, 0);
                    heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - i);
                    v8 = 1.0 - fHeightRatio;
                    XAnimSetGoalWeight(obj, heightAnim, v8, 0.0, 1.0, 0, 0, 0);
                    leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend);
                    v7 = 1.0 - fPrevBlend;
                    XAnimSetGoalWeight(obj, leafAnim1, v7, 0.0, 1.0, 0, 0, 0);
                    if (fPrevBlend != 0.0)
                    {
                        leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend + 1);
                        XAnimSetGoalWeight(obj, leafAnim2, fPrevBlend, 0.0, 1.0, 0, 0, 0);
                    }
                }
                else
                {
                    XAnimSetGoalWeight(obj, heightAnim, 1.0, 0.0, 1.0, 0, 0, 0);
                }
                XAnimCalcAbsDelta(obj, baseAnim, rot, trans);
                VectorAngleMultiply(trans, localYaw);
                localAxis[3][0] = trans[0] + tagMat->trans[0];
                localAxis[3][1] = trans[1] + tagMat->trans[1];
                localAxis[3][2] = tagHeight;
                v5 = RotationToYaw(rot);
                yaw = v5 + localYaw;
                YawToAxis(yaw, *(mat3x3*)localAxis);
                MatrixMultiply43(localAxis, turretAxis, axis);
                origin = ent->client->ps.origin;
                *origin = axis[3][0];
                origin[1] = axis[3][1];
                origin[2] = axis[3][2];
                v11 = ent->client->ps.origin;
                start[0] = *v11;
                start[1] = v11[1];
                start[2] = v11[2];
                client = ent->client;
                end[0] = client->ps.origin[0];
                end[1] = client->ps.origin[1];
                end[2] = client->ps.origin[2];
                start[2] = start[2] + ent->client->ps.viewHeightCurrent;
                end[2] = end[2] - 60.0;
                G_TraceCapsule(&trace, start, vec3_origin, vec3_origin, end, ent->s.number, 0x810011);
                if (trace.fraction < 1.0)
                {
                    Vec3Lerp(start, end, trace.fraction, endpos);
                    ent->client->ps.origin[2] = endpos[2];
                }
                BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, 1, 1u);
                v10 = ent->client->ps.origin;
                ent->r.currentOrigin[0] = *v10;
                ent->r.currentOrigin[1] = v10[1];
                ent->r.currentOrigin[2] = v10[2];
                AxisToAngles(*(const mat3x3*)axis, ent->r.currentAngles);
                SV_LinkEntity(ent);
            }
        }
        else
        {
            Com_PrintWarning(17, "WARNING: aborting player positioning on turret since 'tag_weapon' does not exist\n");
        }
    }
}
void __cdecl turret_clientaim(gentity_s *self, gentity_s *other)
{
    float v2; // [esp+8h] [ebp-48h]
    float v3; // [esp+Ch] [ebp-44h]
    float v4; // [esp+10h] [ebp-40h]
    float v5; // [esp+14h] [ebp-3Ch]
    float v6; // [esp+18h] [ebp-38h]
    float v7; // [esp+1Ch] [ebp-34h]
    float v8; // [esp+20h] [ebp-30h]
    float v9; // [esp+24h] [ebp-2Ch]
    float v10; // [esp+28h] [ebp-28h]
    float v11; // [esp+2Ch] [ebp-24h]
    float v12; // [esp+34h] [ebp-1Ch]
    float v13; // [esp+38h] [ebp-18h]
    float v14; // [esp+3Ch] [ebp-14h]
    float v15; // [esp+40h] [ebp-10h]
    turretInfo_s *pTurretInfo; // [esp+48h] [ebp-8h]
    gclient_s *ps; // [esp+4Ch] [ebp-4h]

    if (!self)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 392, 0, "%s", "self");
    if (self->s.eType != ET_MG42)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 393, 0, "%s", "self->s.eType == ET_MG42");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 396, 0, "%s", "pTurretInfo");
    if (!other)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 398, 0, "%s", "other");
    if (!other->client)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 399, 0, "%s", "other->client");
    ps = other->client;
    if (!self->active)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 403, 0, "%s", "self->active");

    iassert(self->r.ownerNum.isDefined());
    iassert(self->r.ownerNum.ent() == other);

    ps->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    ps->ps.viewlocked_entNum = self->s.number;
    self->s.lerp.u.turret.gunAngles[0] = AngleDelta(ps->ps.viewangles[0], self->r.currentAngles[0]);
    v12 = self->s.lerp.u.turret.gunAngles[0];
    v13 = pTurretInfo->arcmin[0];
    v14 = pTurretInfo->arcmax[0];
    v7 = v12 - v14;
    if (v7 < 0.0)
        v15 = v12;
    else
        v15 = v14;
    v6 = v13 - v12;
    if (v6 < 0.0)
        v5 = v15;
    else
        v5 = v13;
    self->s.lerp.u.turret.gunAngles[0] = v5;
    self->s.lerp.u.turret.gunAngles[1] = AngleDelta(ps->ps.viewangles[1], self->r.currentAngles[1]);
    v8 = self->s.lerp.u.turret.gunAngles[1];
    v9 = pTurretInfo->arcmin[1];
    v10 = pTurretInfo->arcmax[1];
    v4 = v8 - v10;
    if (v4 < 0.0)
        v11 = v8;
    else
        v11 = v10;
    v3 = v9 - v8;
    if (v3 < 0.0)
        v2 = v11;
    else
        v2 = v9;
    self->s.lerp.u.turret.gunAngles[1] = v2;
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    if ((pTurretInfo->flags & 0x800) != 0)
    {
        pTurretInfo->flags &= ~0x800u;
        self->s.lerp.eFlags ^= 2u;
    }
}

void __cdecl turret_shoot_internal(gentity_s *self, gentity_s *other)
{
    if (!self->pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 428, 0, "%s", "self->pTurretInfo");
    if (!other)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 429, 0, "%s", "other");
    self->pTurretInfo->fireSndDelay = 3 * BG_GetWeaponDef(self->s.weapon)->iFireTime;
    if (other->client)
    {
        Fire_Lead(self, other);
        other->client->ps.viewlocked = PLAYERVIEWLOCK_WEAPONJITTER;
    }
}

void __cdecl Fire_Lead(gentity_s *ent, gentity_s *activator)
{
    gentity_s *v2; // [esp+14h] [ebp-4Ch]
    weaponParms wp; // [esp+20h] [ebp-40h] BYREF


    iassert(activator);
    if (activator == &g_entities[ENTITYNUM_NONE])
        v2 = ent;
    else
        v2 = activator;
    Turret_FillWeaponParms(ent, v2, &wp);
    wp.weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (wp.weapDef->weapType)
        Weapon_RocketLauncher_Fire(ent, ent->s.weapon, 0.0, &wp, vec3_origin, 0, 0);
    else
        Bullet_Fire(v2, ent->pTurretInfo->playerSpread, &wp, ent, level.time);
    G_AddEvent(ent, 0x26u, v2->s.number);
}

void __cdecl Turret_FillWeaponParms(gentity_s *ent, gentity_s *activator, weaponParms *wp)
{
    float diff[3]; // [esp+1Ch] [ebp-4Ch] BYREF
    float playerPos[3]; // [esp+28h] [ebp-40h] BYREF
    float len; // [esp+34h] [ebp-34h]
    float flashTag[4][3]; // [esp+38h] [ebp-30h] BYREF

    if (!G_DObjGetWorldTagMatrix(ent, scr_const.tag_flash, flashTag))
    {
        Com_Error(ERR_DROP, "Couldn't find %s on turret (entity %d, classname %s )", "tag_flash", ent->s.number, SL_ConvertToString(ent->classname));
    }
    if (!activator->client)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 137, 0, "%s", "activator->client");
    G_GetPlayerViewOrigin(&activator->client->ps, playerPos);
    BG_GetPlayerViewDirection(&activator->client->ps, wp->forward, wp->right, wp->up);
    wp->gunForward[0] = wp->forward[0];
    wp->gunForward[1] = wp->forward[1];
    wp->gunForward[2] = wp->forward[2];
    Vec3Sub(flashTag[3], playerPos, diff);
    len = Vec3Normalize(diff);
    Vec3Mad(playerPos, len, wp->forward, wp->muzzleTrace);
}

void __cdecl turret_UpdateSound(gentity_s *self)
{
    turretInfo_s *pTurretInfo; // [esp+0h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 498, 0, "%s", "pTurretInfo");
    self->s.loopSound = 0;
    if (pTurretInfo->fireSndDelay > 0)
    {
        self->s.loopSound = pTurretInfo->fireSnd;
        pTurretInfo->fireSndDelay -= 50;
        if (pTurretInfo->fireSndDelay <= 0)
        {
            if (pTurretInfo->stopSnd)
            {
                self->s.loopSound = 0;
                G_PlaySoundAlias(self, pTurretInfo->stopSnd);
            }
        }
    }
}

void __cdecl turret_think(gentity_s *self)
{
    gentity_s *v1; // [esp+0h] [ebp-Ch]

    if (!self->pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 745, 0, "%s", "pTurretInfo");
    self->nextthink = level.time + 50;
    if (self->tagInfo)
        G_GeneralLink(self);
    if (self->r.ownerNum.isDefined())
        v1 = self->r.ownerNum.ent();
    else
        v1 = &g_entities[ENTITYNUM_NONE];
    if (!v1->client)
    {
        turret_UpdateSound(self);
        self->s.lerp.eFlags &= ~0x40u;
        turret_ReturnToDefaultPos(self, 0);
    }
}

int32_t __cdecl turret_ReturnToDefaultPos(gentity_s *self, int32_t bManned)
{
    float dropPitch; // [esp+0h] [ebp-10h]
    float desiredAngles[2]; // [esp+4h] [ebp-Ch] BYREF
    turretInfo_s *pTurretInfo; // [esp+Ch] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 724, 0, "%s", "pTurretInfo");
    if (bManned)
        dropPitch = 0.0;
    else
        dropPitch = pTurretInfo->dropPitch;
    desiredAngles[0] = dropPitch;
    desiredAngles[1] = 0.0;
    return turret_UpdateTargetAngles(self, desiredAngles, bManned);
}

int32_t __cdecl turret_UpdateTargetAngles(gentity_s *self, float *desiredAngles, int32_t bManned)
{
    double v3; // st7
    float desiredPitch; // [esp+10h] [ebp-20h]
    float fDelta; // [esp+14h] [ebp-1Ch]
    float fDeltaa; // [esp+14h] [ebp-1Ch]
    turretInfo_s *pTurretInfo; // [esp+18h] [ebp-18h]
    int32_t bComplete; // [esp+1Ch] [ebp-14h]
    float pitch; // [esp+20h] [ebp-10h]
    float fSpeed[2]; // [esp+24h] [ebp-Ch]
    int32_t i; // [esp+2Ch] [ebp-4h]

    if (self->s.eType != ET_MG42)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 629, 0, "%s", "self->s.eType == ET_MG42");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 632, 0, "%s", "pTurretInfo");
    bComplete = 1;
    pitch = self->s.lerp.u.turret.gunAngles[0];
    self->s.lerp.u.turret.gunAngles[0] = pitch + self->s.lerp.u.turret.gunAngles[2];
    if (bManned)
    {
        fSpeed[0] = BG_GetWeaponDef(self->s.weapon)->maxTurnSpeed[0];
        fSpeed[1] = BG_GetWeaponDef(self->s.weapon)->maxTurnSpeed[1];
    }
    else
    {
        fSpeed[0] = 200.0;
        fSpeed[1] = 200.0;
    }
    if ((pTurretInfo->flags & 0x200) != 0 && (pTurretInfo->flags & 0x100) != 0 && fSpeed[0] < 360.0)
        fSpeed[0] = 360.0;
    for (i = 0; i < 2; ++i)
    {
        fSpeed[i] = fSpeed[i] * 0.05000000074505806;
        if (fSpeed[i] <= 0.0)
            MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 656, 0, "%s", "fSpeed[i] > 0");
        fDelta = AngleDelta(desiredAngles[i], self->s.lerp.u.turret.gunAngles[i]);
        if (fSpeed[i] >= (double)fDelta)
        {
            if (fDelta < -fSpeed[i])
            {
                bComplete = 0;
                fDelta = -fSpeed[i];
            }
        }
        else
        {
            bComplete = 0;
            fDelta = fSpeed[i];
        }
        self->s.lerp.u.turret.gunAngles[i] = self->s.lerp.u.turret.gunAngles[i] + fDelta;
    }
    desiredPitch = self->s.lerp.u.turret.gunAngles[0];
    self->s.lerp.u.turret.gunAngles[2] = desiredPitch;
    if ((pTurretInfo->flags & 0x200) != 0)
    {
        if ((pTurretInfo->flags & 0x400) != 0)
        {
            if (pTurretInfo->pitchCap > (double)self->s.lerp.u.turret.gunAngles[0])
                goto LABEL_24;
            pTurretInfo->flags &= ~0x100u;
        }
        else
        {
            if (pTurretInfo->pitchCap < (double)self->s.lerp.u.turret.gunAngles[0])
            {
            LABEL_24:
                v3 = AngleDelta(pTurretInfo->pitchCap, pitch);
                goto LABEL_29;
            }
            pTurretInfo->flags &= ~0x100u;
        }
    }
    v3 = AngleDelta(desiredPitch, pitch);
LABEL_29:
    fDeltaa = v3;
    if (fSpeed[0] >= (double)fDeltaa)
    {
        if (fDeltaa < -fSpeed[0])
        {
            bComplete = 0;
            fDeltaa = -fSpeed[0];
        }
    }
    else
    {
        bComplete = 0;
        fDeltaa = fSpeed[0];
    }
    self->s.lerp.u.turret.gunAngles[0] = pitch + fDeltaa;
    self->s.lerp.u.turret.gunAngles[2] = self->s.lerp.u.turret.gunAngles[2] - self->s.lerp.u.turret.gunAngles[0];
    return bComplete;
}

void __cdecl turret_think_init(gentity_s *self)
{
    DObjAnimMat *weaponMtx; // [esp+8h] [ebp-D0h]
    float mtx[3][3]; // [esp+Ch] [ebp-CCh] BYREF
    float dir[3]; // [esp+30h] [ebp-A8h] BYREF
    float transDir[3]; // [esp+3Ch] [ebp-9Ch] BYREF
    float start[3]; // [esp+48h] [ebp-90h] BYREF
    float end[3]; // [esp+54h] [ebp-84h] BYREF
    turretInfo_s *pTurretInfo; // [esp+60h] [ebp-78h]
    DObjAnimMat *aimMtx; // [esp+64h] [ebp-74h]
    float angles[3]; // [esp+68h] [ebp-70h] BYREF
    float baseMtx[4][3]; // [esp+74h] [ebp-64h] BYREF
    trace_t trace; // [esp+A4h] [ebp-34h] BYREF
    int32_t i; // [esp+D0h] [ebp-8h]
    int32_t numSteps; // [esp+D4h] [ebp-4h]

    numSteps = 30;
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 781, 0, "%s", "pTurretInfo");
    if (self->handler != 14)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 783, 0, "%s", "self->handler == ENT_HANDLER_TURRET_INIT");
    self->handler = 15;
    self->nextthink = level.time + 50;
    if (!self->pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 790, 0, "%s", "self->pTurretInfo");
    if (self->pTurretInfo->dropPitch == -90.0)
    {
        aimMtx = G_DObjGetLocalTagMatrix(self, scr_const.tag_aim);
        if (aimMtx)
        {
            weaponMtx = G_DObjGetLocalTagMatrix(self, scr_const.tag_butt);
            if (weaponMtx)
            {
                AnglesToAxis(self->r.currentAngles, baseMtx);
                baseMtx[3][0] = self->r.currentOrigin[0];
                baseMtx[3][1] = self->r.currentOrigin[1];
                baseMtx[3][2] = self->r.currentOrigin[2];
                Vec3Sub(weaponMtx->trans, aimMtx->trans, dir);
                MatrixTransformVector43(aimMtx->trans, baseMtx, start);
                for (i = 0; i <= 30; ++i)
                {
                    angles[0] = -90.0 / 30.0 * (double)i;
                    angles[1] = 0.0;
                    angles[2] = 0.0;
                    AnglesToAxis(angles, mtx);
                    MatrixTransformVector(dir, mtx, transDir);
                    Vec3Add(aimMtx->trans, transDir, transDir);
                    MatrixTransformVector43(transDir, baseMtx, end);
                    G_LocationalTrace(&trace, start, end, self->s.number, 2065, bulletPriorityMap);
                    if (trace.fraction < 1.0)
                    {
                        pTurretInfo->dropPitch = angles[0];
                        return;
                    }
                }
            }
        }
    }
}

void __cdecl turret_controller(const gentity_s *self, int32_t *partBits)
{
    uint8_t boneIndex; // [esp+3h] [ebp-11h] BYREF
    DObj_s *obj; // [esp+4h] [ebp-10h]
    float angles[3]; // [esp+8h] [ebp-Ch] BYREF

    if (self->s.eType != ET_MG42)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 836, 0, "%s", "self->s.eType == ET_MG42");
    angles[1] = self->s.lerp.u.turret.gunAngles[1];
    angles[0] = self->s.lerp.u.turret.gunAngles[0];
    angles[2] = 0.0;
    obj = Com_GetServerDObj(self->s.number);
    if (!obj)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 843, 0, "%s", "obj");
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_aim, &boneIndex);
    DObjSetControlTagAngles(obj, partBits, boneIndex, angles);
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_aim_animated, &boneIndex);
    DObjSetControlTagAngles(obj, partBits, boneIndex, angles);
    angles[0] = self->s.lerp.u.turret.gunAngles[2];
    angles[1] = 0.0;
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_flash, &boneIndex);
    DObjSetControlTagAngles(obj, partBits, boneIndex, angles);
}

void __cdecl G_FreeTurret(gentity_s *self)
{
    gentity_s *v1; // [esp+0h] [ebp-Ch]

    if (self->r.ownerNum.isDefined())
        v1 = self->r.ownerNum.ent();
    else
        v1 = &g_entities[ENTITYNUM_NONE];
    if (!self->pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 900, 0, "%s", "pTurretInfo");
    if (v1->client)
        G_ClientStopUsingTurret(self);
    self->active = 0;
    self->pTurretInfo->inuse = 0;
    self->pTurretInfo = 0;
}

bool __cdecl G_IsTurretUsable(gentity_s *self, gentity_s *owner)
{
    if (self->active || !self->pTurretInfo)
        return 0;
    if (!turret_behind(self, owner))
        return 0;
    if (owner->client->ps.grenadeTimeLeft)
        return 0;
    return owner->client->ps.groundEntityNum != ENTITYNUM_NONE;
}

bool __cdecl turret_behind(gentity_s *self, gentity_s *other)
{
    double v2; // st7
    float v4; // [esp+10h] [ebp-78h]
    float v5; // [esp+14h] [ebp-74h]
    float v6; // [esp+18h] [ebp-70h]
    float v7; // [esp+1Ch] [ebp-6Ch]
    float v8; // [esp+20h] [ebp-68h]
    float v9; // [esp+24h] [ebp-64h]
    float v10; // [esp+28h] [ebp-60h]
    float v11; // [esp+2Ch] [ebp-5Ch]
    float v12; // [esp+48h] [ebp-40h]
    float dir[3]; // [esp+58h] [ebp-30h] BYREF
    float centerYaw; // [esp+64h] [ebp-24h]
    float minYaw; // [esp+68h] [ebp-20h]
    float angle; // [esp+6Ch] [ebp-1Ch]
    float forward[3]; // [esp+70h] [ebp-18h] BYREF
    turretInfo_s *pTurretInfo; // [esp+7Ch] [ebp-Ch]
    float yawSpan; // [esp+80h] [ebp-8h]
    float dot; // [esp+84h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if (!other->client)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 869, 0, "%s", "other->client");
    minYaw = self->r.currentAngles[1] + pTurretInfo->arcmin[1];
    v10 = I_fabs(pTurretInfo->arcmin[1]);
    v9 = I_fabs(pTurretInfo->arcmax[1]);
    yawSpan = (v10 + v9) * 0.5;
    v8 = yawSpan + minYaw;
    v12 = v8 * 0.002777777845039964;
    v7 = v12 + 0.5;
    v6 = floor(v7);
    centerYaw = (v12 - v6) * 360.0;
    YawVectors(centerYaw, forward, 0);
    Vec3Normalize(forward);
    Vec3Sub(self->r.currentOrigin, other->r.currentOrigin, dir);
    dir[2] = 0.0;
    Vec3Normalize(dir);
    dot = Vec3Dot(forward, dir);
    v5 = dot - 1.0;
    if (v5 < 0.0)
        v11 = dot;
    else
        v11 = 1.0;
    v4 = -1.0 - dot;
    if (v4 < 0.0)
        v2 = Q_acos(v11);
    else
        v2 = Q_acos(-1.0);
    angle = v2 * 57.2957763671875;
    return yawSpan >= (double)angle;
}

void __cdecl turret_use(gentity_s *self, gentity_s *owner, gentity_s* activator)
{
    uint32_t v2; // ecx
    float v3; // [esp+8h] [ebp-60h]
    float v4; // [esp+Ch] [ebp-5Ch]
    float v5; // [esp+10h] [ebp-58h]
    float v6; // [esp+14h] [ebp-54h]
    float v7; // [esp+18h] [ebp-50h]
    float v8; // [esp+1Ch] [ebp-4Ch]
    float v9; // [esp+20h] [ebp-48h]
    float v10; // [esp+24h] [ebp-44h]
    float v11; // [esp+30h] [ebp-38h]
    float v12; // [esp+34h] [ebp-34h]
    float v13; // [esp+38h] [ebp-30h]
    float v14; // [esp+3Ch] [ebp-2Ch]
    float v15; // [esp+44h] [ebp-24h]
    float v16; // [esp+48h] [ebp-20h]
    float v17; // [esp+4Ch] [ebp-1Ch]
    float v18; // [esp+50h] [ebp-18h]
    turretInfo_s *pTurretInfo; // [esp+60h] [ebp-8h]
    gclient_s *ps; // [esp+64h] [ebp-4h]

    if (!self)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 934, 0, "%s", "self");
    if (self->s.eType != ET_MG42)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 935, 0, "%s", "self->s.eType == ET_MG42");
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 938, 0, "%s", "pTurretInfo");
    if (!owner)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 940, 0, "%s", "owner");
    if (!owner->client)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 941, 0, "%s", "owner->client");
    if (owner->s.number < 0)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 942, 0, "%s", "owner->s.number >= 0");
    if (owner->s.number >= level.maxclients)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 943, 0, "%s", "owner->s.number < level.maxclients");
    ps = owner->client;
    owner->active = 1;
    self->active = 1;
    self->r.ownerNum.setEnt(owner);
    owner->flags |= 0x400000u;
    ps->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    ps->ps.viewlocked_entNum = self->s.number;
    pTurretInfo->flags |= 0x800u;
    pTurretInfo->userOrigin[0] = owner->r.currentOrigin[0];
    pTurretInfo->userOrigin[1] = owner->r.currentOrigin[1];
    pTurretInfo->userOrigin[2] = owner->r.currentOrigin[2];
    owner->s.otherEntityNum = self->s.number;
    self->s.otherEntityNum = owner->s.number;
    if ((ps->ps.pm_flags & 1) != 0)
        pTurretInfo->prevStance = 2;
    else
        pTurretInfo->prevStance = (ps->ps.pm_flags & 2) != 0;
    if (pTurretInfo->stance == 2)
    {
        ps->ps.eFlags |= 0x100u;
        ps->ps.eFlags &= ~0x200u;
    }
    else
    {
        if (pTurretInfo->stance == 1)
        {
            ps->ps.eFlags |= 0x200u;
            v2 = ps->ps.eFlags & 0xFFFFFEFF;
        }
        else
        {
            v2 = ps->ps.eFlags | 0x300;
        }
        ps->ps.eFlags = v2;
    }
    self->s.lerp.u.turret.gunAngles[0] = AngleDelta(ps->ps.viewangles[0], self->r.currentAngles[0]);
    v15 = self->s.lerp.u.turret.gunAngles[0];
    v16 = pTurretInfo->arcmin[0];
    v17 = pTurretInfo->arcmax[0];
    v10 = v15 - v17;
    if (v10 < 0.0)
        v18 = v15;
    else
        v18 = v17;
    v9 = v16 - v15;
    if (v9 < 0.0)
        v8 = v18;
    else
        v8 = v16;
    self->s.lerp.u.turret.gunAngles[0] = v8;
    self->s.lerp.u.turret.gunAngles[1] = AngleDelta(ps->ps.viewangles[1], self->r.currentAngles[1]);
    v11 = self->s.lerp.u.turret.gunAngles[1];
    v12 = pTurretInfo->arcmin[1];
    v13 = pTurretInfo->arcmax[1];
    v7 = v11 - v13;
    if (v7 < 0.0)
        v14 = v11;
    else
        v14 = v13;
    v6 = v12 - v11;
    if (v6 < 0.0)
        v5 = v14;
    else
        v5 = v12;
    self->s.lerp.u.turret.gunAngles[1] = v5;
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    ps->ps.viewAngleClampRange[0] = AngleDelta(pTurretInfo->arcmax[0], pTurretInfo->arcmin[0]) * 0.5;
    ps->ps.viewAngleClampBase[0] = self->r.currentAngles[0] + pTurretInfo->arcmax[0];
    v4 = ps->ps.viewAngleClampBase[0] - ps->ps.viewAngleClampRange[0];
    ps->ps.viewAngleClampBase[0] = AngleNormalize360(v4);
    ps->ps.viewAngleClampRange[1] = AngleDelta(pTurretInfo->arcmax[1], pTurretInfo->arcmin[1]) * 0.5;
    ps->ps.viewAngleClampBase[1] = self->r.currentAngles[1] + pTurretInfo->arcmax[1];
    v3 = ps->ps.viewAngleClampBase[1] - ps->ps.viewAngleClampRange[1];
    ps->ps.viewAngleClampBase[1] = AngleNormalize360(v3);
}

void __cdecl G_SpawnTurret(gentity_s *self, const char *weaponinfoname)
{
    const char *v2; // eax
    const char *v3; // eax
    turretInfo_s *pTurretInfo; // [esp+Ch] [ebp-Ch]
    int32_t i; // [esp+10h] [ebp-8h]
    WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    pTurretInfo = 0;
    for (i = 0; i < 32; ++i)
    {
        pTurretInfo = &turretInfo[i];
        if (!pTurretInfo->inuse)
            break;
    }
    if (i == 32)
        Com_Error(ERR_DROP, "G_SpawnTurret: max number of turrets (%d) exceeded", 32);
    memset((uint8_t *)pTurretInfo, 0, sizeof(turretInfo_s));
    self->pTurretInfo = pTurretInfo;
    pTurretInfo->inuse = 1;
    self->s.weapon = G_GetWeaponIndexForName(weaponinfoname);
    self->s.weaponModel = 0;
    if (!self->s.weapon)
        Com_Error(ERR_DROP, "bad weaponinfo ',27h,'%s',27h,' specified for turret", weaponinfoname);
    weapDef = BG_GetWeaponDef(self->s.weapon);
    if (weapDef->weapClass != WEAPCLASS_TURRET)
    {
        v2 = va(
            "G_SpawnTurret: weapon '%s' isn't a turret. This usually indicates that the weapon failed to load.",
            weaponinfoname);
        Scr_Error(v2);
    }
    if (weapDef->weapClass != WEAPCLASS_TURRET)
        MyAssertHandler(".\\game_mp\\g_misc_mp.cpp", 1036, 0, "%s", "weapDef->weapClass == WEAPCLASS_TURRET");
    if (!level.initializing && !IsItemRegistered(self->s.weapon))
    {
        v3 = va("turret '%s' not precached", weaponinfoname);
        Scr_Error(v3);
    }
    pTurretInfo->fireTime = 0;
    pTurretInfo->stance = weapDef->stance;
    pTurretInfo->prevStance = -1;
    pTurretInfo->fireSndDelay = 0;
    if (weapDef->fireLoopSound)
        pTurretInfo->fireSnd = G_SoundAliasIndex((char *)weapDef->fireLoopSound->aliasName);
    else
        pTurretInfo->fireSnd = 0;
    if (weapDef->fireLoopSoundPlayer)
        pTurretInfo->fireSndPlayer = G_SoundAliasIndex((char *)weapDef->fireLoopSoundPlayer->aliasName);
    else
        pTurretInfo->fireSndPlayer = 0;
    if (weapDef->fireStopSound)
        pTurretInfo->stopSnd = G_SoundAliasIndex((char *)weapDef->fireStopSound->aliasName);
    else
        pTurretInfo->stopSnd = 0;
    if (weapDef->fireStopSoundPlayer)
        pTurretInfo->stopSndPlayer = G_SoundAliasIndex((char *)weapDef->fireStopSoundPlayer->aliasName);
    else
        pTurretInfo->stopSndPlayer = 0;
    if (!level.spawnVar.spawnVarsValid || !G_SpawnFloat("rightarc", "", &pTurretInfo->arcmin[1]))
        pTurretInfo->arcmin[1] = weapDef->rightArc;
    pTurretInfo->arcmin[1] = pTurretInfo->arcmin[1] * -1.0;
    if (pTurretInfo->arcmin[1] > 0.0)
        pTurretInfo->arcmin[1] = 0.0;
    if (!level.spawnVar.spawnVarsValid || !G_SpawnFloat("leftarc", "", &pTurretInfo->arcmax[1]))
        pTurretInfo->arcmax[1] = weapDef->leftArc;
    if (pTurretInfo->arcmax[1] < 0.0)
        pTurretInfo->arcmax[1] = 0.0;
    if (!level.spawnVar.spawnVarsValid || !G_SpawnFloat("toparc", "", pTurretInfo->arcmin))
        pTurretInfo->arcmin[0] = weapDef->topArc;
    pTurretInfo->arcmin[0] = pTurretInfo->arcmin[0] * -1.0;
    if (pTurretInfo->arcmin[0] > 0.0)
        pTurretInfo->arcmin[0] = 0.0;
    if (!level.spawnVar.spawnVarsValid || !G_SpawnFloat("bottomarc", "", pTurretInfo->arcmax))
        pTurretInfo->arcmax[0] = weapDef->bottomArc;
    if (pTurretInfo->arcmax[0] < 0.0)
        pTurretInfo->arcmax[0] = 0.0;
    pTurretInfo->dropPitch = -90.0;
    if (!self->health)
        self->health = 100;
    if (!level.spawnVar.spawnVarsValid || !G_SpawnInt("damage", "0", &self->damage))
        self->damage = weapDef->damage;
    if (self->damage < 0)
        self->damage = 0;
    if (!level.spawnVar.spawnVarsValid || !G_SpawnFloat("playerSpread", "1", &pTurretInfo->playerSpread))
        pTurretInfo->playerSpread = weapDef->playerSpread;
    if (pTurretInfo->playerSpread < 0.0)
        pTurretInfo->playerSpread = 0.0;
    pTurretInfo->flags = 3;
    self->clipmask = 1;
    self->r.contents = 2097156;
    self->r.svFlags = 0;
    self->s.eType = ET_MG42;
    self->flags |= 0x1000u;
    G_DObjUpdate(self);
    self->r.mins[0] = -32.0;
    self->r.mins[1] = -32.0;
    self->r.mins[2] = 0.0;
    self->r.maxs[0] = 32.0;
    self->r.maxs[1] = 32.0;
    self->r.maxs[2] = 56.0;
    G_SetOrigin(self, self->r.currentOrigin);
    G_SetAngle(self, self->r.currentAngles);
    self->s.lerp.u.turret.gunAngles[0] = 0.0;
    self->s.lerp.u.turret.gunAngles[1] = 0.0;
    self->s.lerp.u.turret.gunAngles[2] = 0.0;
    self->handler = 14;
    self->nextthink = level.time + 50;
    self->s.lerp.apos.trType = TR_LINEAR_STOP;
    self->takedamage = 0;

    iassert(!self->r.ownerNum.isDefined());

    SV_LinkEntity(self);
}

void __cdecl SP_turret(gentity_s *self)
{
    const char *weaponinfoname; // [esp+0h] [ebp-4h] BYREF

    if (!G_LevelSpawnString("weaponinfo", "", &weaponinfoname))
        Com_Error(ERR_DROP, "no weaponinfo specified for turret");
    G_SpawnTurret(self, weaponinfoname);
}

