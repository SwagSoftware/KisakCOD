#include "cg_local.h"
#include "cg_public.h"

#include <bgame/bg_public.h>

#include <cgame_mp/cg_local_mp.h>
#include <script/scr_const.h>
#include <xanim/dobj.h>
#include <EffectsCore/fx_system.h>
#include <bgame/bg_public.h>
#include <DynEntity/DynEntity_client.h>
#include <ragdoll/ragdoll.h>
#include <client/client.h>
#include <server_mp/server.h>


int __cdecl CG_GetBoneIndex(
    int localClientNum,
    unsigned int dobjHandle,
    unsigned int boneName,
    unsigned __int8 *boneIndex)
{
    const DObj_s *obj; // [esp+0h] [ebp-4h]

    if (dobjHandle >= 0x480)
        MyAssertHandler(
            ".\\cgame\\cg_event.cpp",
            307,
            0,
            "dobjHandle doesn't index CLIENT_DOBJ_HANDLE_MAX\n\t%i not in [0, %i)",
            dobjHandle,
            1152);
    obj = Com_GetClientDObj(dobjHandle, localClientNum);
    if (obj)
        return DObjGetBoneIndex(obj, boneName, boneIndex);
    else
        return 0;
}

void __cdecl CG_PlayBoltedEffect(
    int localClientNum,
    const FxEffectDef *fxDef,
    unsigned int dobjHandle,
    unsigned int boneName)
{
    unsigned __int8 boneIndex; // [esp+3h] [ebp-5h] BYREF
    int time; // [esp+4h] [ebp-4h]

    boneIndex = -2;
    if (CG_GetBoneIndex(localClientNum, dobjHandle, boneName, &boneIndex))
    {
        iassert(localClientNum == 0);
        time = cgArray[0].time;
        FX_PlayBoltedEffect(localClientNum, fxDef, cgArray[0].time, dobjHandle, boneIndex);
    }
}

void __cdecl CG_EntityEvent(int localClientNum, centity_s *cent, int event)
{
    char *ConfigString; // eax
    char *v4; // eax
    float innerRadius_4; // [esp+4h] [ebp-150h]
    float innerRadius_4a; // [esp+4h] [ebp-150h]
    float innerRadius_4b; // [esp+4h] [ebp-150h]
    float innerRadius_4c; // [esp+4h] [ebp-150h]
    float innerRadius_4d; // [esp+4h] [ebp-150h]
    float innerRadius_4e; // [esp+4h] [ebp-150h]
    float p_4; // [esp+Ch] [ebp-148h]
    float p_4a; // [esp+Ch] [ebp-148h]
    float p_4b; // [esp+Ch] [ebp-148h]
    float p_4c; // [esp+Ch] [ebp-148h]
    float p_4d; // [esp+Ch] [ebp-148h]
    float p_4e; // [esp+Ch] [ebp-148h]
    float p_4f; // [esp+Ch] [ebp-148h]
    bool v23; // [esp+ACh] [ebp-A8h]
    snapshot_s *v24; // [esp+B0h] [ebp-A4h]
    snapshot_s *v25; // [esp+B4h] [ebp-A0h]
    snapshot_s *v26; // [esp+B8h] [ebp-9Ch]
    snapshot_s *v27; // [esp+BCh] [ebp-98h]
    snapshot_s *v28; // [esp+C0h] [ebp-94h]
    snapshot_s *nextSnap; // [esp+C8h] [ebp-8Ch]
    snd_alias_list_t *v30; // [esp+CCh] [ebp-88h] BYREF
    FxEffectDef *def; // [esp+D0h] [ebp-84h] BYREF
    snd_alias_list_t *outSnd; // [esp+D4h] [ebp-80h] BYREF
    FxEffectDef *outFx; // [esp+D8h] [ebp-7Ch] BYREF
    snd_alias_list_t *snd; // [esp+DCh] [ebp-78h] BYREF
    const FxEffectDef *fx; // [esp+E0h] [ebp-74h] BYREF
    centity_s *attackerCent; // [esp+E4h] [ebp-70h]
    entityType_t eType; // [esp+E8h] [ebp-6Ch]
    WeaponDef *itemWeapDef; // [esp+ECh] [ebp-68h]
    int index; // [esp+F0h] [ebp-64h]
    float fallHeight; // [esp+F4h] [ebp-60h]
    float dir[3]; // [esp+F8h] [ebp-5Ch] BYREF
    bool isPlayerView; // [esp+107h] [ebp-4Dh]
    cg_s *cgameGlob; // [esp+108h] [ebp-4Ch]
    int viewDip; // [esp+10Ch] [ebp-48h]
    const WeaponDef *weaponDef; // [esp+110h] [ebp-44h]
    int offset; // [esp+114h] [ebp-40h]
    entityState_s *ent; // [esp+118h] [ebp-3Ch]
    int eventParm; // [esp+11Ch] [ebp-38h]
    float axis[3][3]; // [esp+120h] [ebp-34h] BYREF
    int clientNum; // [esp+144h] [ebp-10h]
    float *position; // [esp+148h] [ebp-Ch]
    const playerState_s *ps; // [esp+14Ch] [ebp-8h]
    unsigned int weaponIdx; // [esp+150h] [ebp-4h]

    if (event)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgameGlob = cgArray;
        position = cent->pose.origin;
        ps = &cgArray[0].nextSnap->ps;
        ent = &cent->nextState;
        eventParm = cent->nextState.eventParm;
        nextSnap = cgArray[0].nextSnap;
        v23 = (nextSnap->ps.otherFlags & 6) != 0 && ent->number == nextSnap->ps.clientNum;
        isPlayerView = v23;
        if (cg_debugEvents->current.enabled)
            Com_Printf(21, "ent:%3i  event:%3i ", ent->number, event);
        iassert(event > 0 && event < EV_MAX_EVENTS);
        if (cg_debugEvents->current.enabled)
            Com_Printf(21, "CG_EntityEvent:%s\n", eventnames[event]);

        if (isPlayerView)
            weaponIdx = cgameGlob->predictedPlayerState.weapon;
        else
            weaponIdx = ent->weapon;
        clientNum = ent->clientNum;

        if ((unsigned int)clientNum >= 64)
            clientNum = 0;

        weaponDef = BG_GetWeaponDef(weaponIdx);
        if ((cgameGlob->bgs.clientinfo[clientNum].perks & 0x100) != 0)
            offset = 29;
        else
            offset = 0;

        if (event > EV_LANDING_FIRST && event < EV_LANDING_LAST)
        {
            if (isPlayerView)
                CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.landSound[event - 19] + offset));
            else
                CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepProneSoundPlayer[event - 19] + offset));
            if (clientNum == cgameGlob->predictedPlayerState.clientNum)
            {
                cgameGlob->landChange = 0.0 - (double)eventParm;
                cgameGlob->landTime = cgameGlob->time;
            }
        }
        else if (event > EV_LANDING_PAIN_FIRST && event < EV_LANDING_PAIN_LAST)
        {
            if (isPlayerView)
                CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.landSound[event - 48] + offset));
            else
                CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepProneSoundPlayer[event - 48] + offset));
            CG_PlayEntitySoundAlias(localClientNum, ent->number, cgMedia.landDmgSound);
            if (clientNum == cgameGlob->predictedPlayerState.clientNum)
            {
                fallHeight = (double)eventParm
                    * 0.009999999776482582
                    * (bg_fallDamageMaxHeight->current.value - bg_fallDamageMinHeight->current.value)
                    + bg_fallDamageMinHeight->current.value;
                if (fallHeight > 12.0)
                    viewDip = (int)((fallHeight - 12.0) / 26.0 * 4.0 + 4.0);
                else
                    viewDip = 0;
                if (viewDip > 24)
                    viewDip = 24;
                if (viewDip > 0)
                {
                    cgameGlob->landChange = 0.0 - (double)viewDip;
                    cgameGlob->landTime = cgameGlob->time;
                }
            }
        }
        else
        {
            switch (event)
            {
            case EV_FOLIAGE_SOUND:
                CG_PlayEntitySoundAlias(localClientNum, ent->number, cgMedia.foliageMovement);
                return;
            case EV_STOP_WEAPON_SOUND:
                CG_StopWeaponSound(localClientNum, isPlayerView, weaponDef, ent->number, (weaponstate_t)eventParm);
                return;
            case EV_SOUND_ALIAS:
                if (ent->eventParm)
                {
                    ConfigString = CL_GetConfigString(localClientNum, ent->eventParm + 1342);
                    CG_PlaySoundAliasByName(localClientNum, ent->number, ent->lerp.pos.trBase, ConfigString);
                }
                return;
            case EV_SOUND_ALIAS_AS_MASTER:
                if (ent->eventParm)
                {
                    v4 = CL_GetConfigString(localClientNum, ent->eventParm + 1342);
                    CG_PlaySoundAliasAsMasterByName(localClientNum, ent->number, ent->lerp.pos.trBase, v4);
                }
                return;
            case EV_STOPSOUNDS:
                CG_StopSoundsOnEnt(localClientNum, ent->number);
                return;
            case EV_STANCE_FORCE_STAND:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                CL_SetStance(localClientNum, CL_STANCE_STAND);
                return;
            case EV_STANCE_FORCE_CROUCH:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                CL_SetStance(localClientNum, CL_STANCE_CROUCH);
                return;
            case EV_STANCE_FORCE_PRONE:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                CL_SetStance(localClientNum, CL_STANCE_PRONE);
                return;
            case EV_ITEM_PICKUP:
            case EV_AMMO_PICKUP:
                index = ent->eventParm;
                if (index >= 1 && index < 128)
                {
                    itemWeapDef = BG_GetWeaponDef(index);
                    if (event == EV_ITEM_PICKUP)
                    {
                        if (isPlayerView)
                            CG_PlayEntitySoundAlias(localClientNum, ent->number, itemWeapDef->pickupSoundPlayer);
                        else
                            CG_PlayEntitySoundAlias(localClientNum, ent->number, itemWeapDef->pickupSound);
                    }
                    else if (event == EV_AMMO_PICKUP)
                    {
                        if (isPlayerView)
                            CG_PlayEntitySoundAlias(localClientNum, ent->number, itemWeapDef->ammoPickupSoundPlayer);
                        else
                            CG_PlayEntitySoundAlias(localClientNum, ent->number, itemWeapDef->ammoPickupSound);
                    }
                    if (isPlayerView)
                        CG_ItemPickup(localClientNum, index);
                }
                return;
            case EV_NOAMMO:
                if (!BG_WeaponIsClipOnly(weaponIdx) && !weaponDef->cancelAutoHolsterWhenEmpty)
                {
                    if (isPlayerView)
                        CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->emptyFireSoundPlayer);
                    else
                        CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->emptyFireSound);
                }
                if (isPlayerView)
                    CG_OutOfAmmoChange(localClientNum);
                return;
            case EV_EMPTYCLIP:
            case EV_RELOAD_START_NOTIFY:
            case EV_RELOAD_ADDAMMO:
            case EV_GRENADE_SUICIDE:
                return;
            case EV_EMPTY_OFFHAND:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                if (isPlayerView)
                {
                    CG_MenuShowNotify(localClientNum, 4);
                    CG_SwitchOffHandCmd(localClientNum);
                }
                return;
            case EV_RESET_ADS:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                if (isPlayerView)
                    CL_SetADS(localClientNum, 0);
                return;
            case EV_RELOAD:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadSound);
                return;
            case EV_RELOAD_FROM_EMPTY:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadEmptySoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadEmptySound);
                return;
            case EV_RELOAD_START:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadStartSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadStartSound);
                return;
            case EV_RELOAD_END:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadEndSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->reloadEndSound);
                return;
            case EV_RAISE_WEAPON:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->raiseSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->raiseSound);
                return;
            case EV_FIRST_RAISE_WEAPON:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->firstRaiseSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->firstRaiseSound);
                return;
            case EV_PUTAWAY_WEAPON:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->putawaySoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->putawaySound);
                return;
            case EV_WEAPON_ALT:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->altSwitchSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->altSwitchSound);
                return;
            case EV_PULLBACK_WEAPON:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->pullbackSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->pullbackSound);
                return;
            case EV_FIRE_WEAPON:
            case EV_FIRE_WEAPON_LASTSHOT:
                CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash, 0, &cgameGlob->predictedPlayerState);
                return;
            case EV_RECHAMBER_WEAPON:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->rechamberSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->rechamberSound);
                return;
            case EV_EJECT_BRASS:
                CG_EjectWeaponBrass(localClientNum, ent, event);
                return;
            case EV_MELEE_SWIPE:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->meleeSwipeSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->meleeSwipeSound);
                return;
            case EV_FIRE_MELEE:
                DynEntCl_MeleeEvent(localClientNum, ent->number);
                return;
            case EV_PREP_OFFHAND:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                if (isPlayerView)
                    CG_PrepOffHand(localClientNum, ent, ent->eventParm);
                return;
            case EV_USE_OFFHAND:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                if (isPlayerView)
                    CG_UseOffHand(localClientNum, cent, ent->eventParm);
                return;
            case EV_SWITCH_OFFHAND:
                if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                    goto LABEL_216;
                if (isPlayerView)
                    CG_SetEquippedOffHand(localClientNum, ent->eventParm);
                return;
            case EV_MELEE_HIT:
                if (ent->eventParm)
                {
                    CG_PlayEntitySoundAlias(localClientNum, ent->otherEntityNum, cgMedia.meleeKnifeHit);
                }
                else if (weaponDef->meleeHitSound)
                {
                    CG_PlayEntitySoundAlias(localClientNum, ent->otherEntityNum, weaponDef->meleeHitSound);
                }
                else
                {
                    CG_PlayEntitySoundAlias(localClientNum, ent->otherEntityNum, cgMedia.meleeHit);
                }
                return;
            case EV_MELEE_MISS:
                if (ent->eventParm)
                {
                    CG_PlaySoundAlias(localClientNum, ent->otherEntityNum, position, cgMedia.meleeKnifeHitOther);
                }
                else if (weaponDef->meleeMissSound)
                {
                    CG_PlaySoundAlias(localClientNum, ent->otherEntityNum, position, weaponDef->meleeMissSound);
                }
                else
                {
                    CG_PlaySoundAlias(localClientNum, ent->otherEntityNum, position, cgMedia.meleeHitOther);
                }
                return;
            case EV_MELEE_BLOOD:
                if (clientNum == cgameGlob->predictedPlayerState.clientNum)
                {
                    if (isPlayerView)
                        CG_MeleeBloodEvent(localClientNum, cent);
                }
                else
                {
                LABEL_216:
                    Com_DPrintf(21, "Event %s just for client %i was sent to other clients\n", eventnames[event], clientNum);
                }
                return;
            case EV_FIRE_WEAPON_MG42:
                eType = ET_PLAYER;
                CG_StartShakeCamera(localClientNum, 0.050000001f, 100, cent->pose.origin, 100.0f);
                CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash, 0, &cgameGlob->nextSnap->ps);
                attackerCent = CG_GetEntity(localClientNum, eventParm);
                if (attackerCent->nextValid
                    && attackerCent->nextState.eType == eType
                    && ps->viewlocked_entNum != cent->nextState.number)
                {
                    CG_CompassAddWeaponPingInfo(localClientNum, attackerCent, cent->pose.origin, 50);
                }
                return;
            case EV_FIRE_QUADBARREL_1:
                CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash, 0, &cgameGlob->nextSnap->ps);
                CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_11, 0, &cgameGlob->nextSnap->ps);
                return;
            case EV_FIRE_QUADBARREL_2:
                CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_2, 0, &cgameGlob->nextSnap->ps);
                CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_22, 0, &cgameGlob->nextSnap->ps);
                return;
            case EV_BULLET_HIT:
                ByteToDir(ent->eventParm, dir);
                CG_BulletHitEvent(
                    localClientNum,
                    ent->otherEntityNum,
                    ent->groundEntityNum,
                    ent->weapon,
                    ent->lerp.u.turret.gunAngles,
                    cent->pose.origin,
                    dir,
                    ent->surfType,
                    event,
                    ent->un1.scale,
                    weaponDef->damage,
                    0);
                return;
            case EV_BULLET_HIT_CLIENT_SMALL:
            case EV_BULLET_HIT_CLIENT_LARGE:
                CG_BulletHitClientEvent(
                    localClientNum,
                    ent->otherEntityNum,
                    ent->lerp.u.turret.gunAngles,
                    cent->pose.origin,
                    ent->surfType,
                    event,
                    weaponDef->damage);
                return;
            case EV_GRENADE_BOUNCE:
                if (ent->surfType >= 0x1Du)
                    MyAssertHandler(
                        ".\\cgame\\cg_event.cpp",
                        971,
                        0,
                        "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                        ent->surfType,
                        29);
                if (weaponDef->bounceSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->bounceSound[ent->surfType]);
                if (cgMedia.fx->table[8].nonflesh[ent->surfType])
                {
                    ByteToDir(ent->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    FX_PlayOrientedEffect(
                        localClientNum,
                        cgMedia.fx->table[8].nonflesh[ent->surfType],
                        cgameGlob->time,
                        position,
                        axis);
                }
                return;
            case EV_GRENADE_EXPLODE:
                p_4 = (float)weaponDef->iExplosionRadius;
                Ragdoll_ExplosionEvent(localClientNum, 0, position, 0.0, p_4, vec3_origin, 1.0);
                innerRadius_4 = (float)weaponDef->iExplosionRadius;
                DynEntCl_ExplosionEvent(
                    localClientNum,
                    0,
                    position,
                    0.0,
                    innerRadius_4,
                    (float *)vec3_origin,
                    1.0,
                    weaponDef->iExplosionInnerDamage,
                    weaponDef->iExplosionOuterDamage);
                if (ent->surfType >= 0x1Du)
                    MyAssertHandler(
                        ".\\cgame\\cg_event.cpp",
                        992,
                        0,
                        "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                        ent->surfType,
                        29);
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                CG_ImpactEffectForWeapon(weaponIdx, ent->surfType, 0, &fx, &snd);
                if (fx)
                    FX_PlayOrientedEffect(localClientNum, fx, cgameGlob->time, position, axis);
                if (snd)
                    CG_PlaySoundAlias(localClientNum, 1022, position, snd);
                if (weaponDef->projExplosionEffect)
                    FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                if (weaponDef->projExplosionSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projExplosionSound);
                return;
            case EV_ROCKET_EXPLODE:
                goto $LN48_1;
            case EV_ROCKET_EXPLODE_NOMARKS:
                cgameGlob->nomarks = 1;
            $LN48_1:
                p_4a = (float)weaponDef->iExplosionRadius;
                Ragdoll_ExplosionEvent(localClientNum, 0, position, p_4a, p_4a, vec3_origin, 1.0);
                innerRadius_4a = (float)weaponDef->iExplosionRadius;
                DynEntCl_ExplosionEvent(
                    localClientNum,
                    0,
                    position,
                    innerRadius_4a,
                    innerRadius_4a,
                    (float *)vec3_origin,
                    1.0,
                    weaponDef->iExplosionInnerDamage,
                    weaponDef->iExplosionOuterDamage);
                if (ent->surfType >= 0x1Du)
                    MyAssertHandler(
                        ".\\cgame\\cg_event.cpp",
                        1022,
                        0,
                        "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                        ent->surfType,
                        29);
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                CG_ImpactEffectForWeapon(weaponIdx, ent->surfType, 0, (const FxEffectDef **)&outFx, &outSnd);
                if (outFx)
                    FX_PlayOrientedEffect(localClientNum, outFx, cgameGlob->time, position, axis);
                if (outSnd)
                    CG_PlaySoundAlias(localClientNum, 1022, position, outSnd);
                if (weaponDef->projExplosionEffect)
                    FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                if (weaponDef->projExplosionSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projExplosionSound);
                cgameGlob->nomarks = 0;
                return;
            case EV_FLASHBANG_EXPLODE:
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                if (weaponDef->projExplosionEffect)
                    FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                if (weaponDef->projExplosionSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projExplosionSound);
                return;
            case EV_CUSTOM_EXPLODE:
                goto $LN37_1;
            case EV_CUSTOM_EXPLODE_NOMARKS:
                cgameGlob->nomarks = 1;
            $LN37_1:
                p_4b = (float)weaponDef->iExplosionRadius;
                Ragdoll_ExplosionEvent(localClientNum, 0, position, p_4b, p_4b, vec3_origin, 1.0);
                innerRadius_4b = (float)weaponDef->iExplosionRadius;
                DynEntCl_ExplosionEvent(
                    localClientNum,
                    0,
                    position,
                    innerRadius_4b,
                    innerRadius_4b,
                    (float *)vec3_origin,
                    1.0,
                    weaponDef->iExplosionInnerDamage,
                    weaponDef->iExplosionOuterDamage);
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                CG_ImpactEffectForWeapon(weaponIdx, ent->surfType, 0, (const FxEffectDef **)&def, &v30);
                if (def)
                    FX_PlayOrientedEffect(localClientNum, def, cgameGlob->time, position, axis);
                if (v30)
                    CG_PlaySoundAlias(localClientNum, 1022, position, v30);
                if (weaponDef->projExplosionEffect)
                {
                    Com_Printf(
                        14,
                        "Playing smoke grenade at %i at ( %f, %f, %f )\n",
                        ent->lerp.u.missile.launchTime,
                        ent->lerp.pos.trBase[0],
                        ent->lerp.pos.trBase[1],
                        ent->lerp.pos.trBase[2]);
                    FX_PlayOrientedEffect(
                        localClientNum,
                        weaponDef->projExplosionEffect,
                        ent->lerp.u.missile.launchTime,
                        ent->lerp.pos.trBase,
                        axis);
                }
                if (weaponDef->projExplosionSound
                    && ((ent->lerp.eFlags & 0x10000) == 0 || cgameGlob->time - ent->lerp.u.missile.launchTime < 200))
                {
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projExplosionSound);
                }
                cgameGlob->nomarks = 0;
                return;
            case EV_CHANGE_TO_DUD:
                cgameGlob->nomarks = 1;
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                if (cgMedia.fx->table[11].nonflesh[0])
                    FX_PlayOrientedEffect(localClientNum, cgMedia.fx->table[11].nonflesh[0], cgameGlob->time, position, axis);
                cgameGlob->nomarks = 0;
                return;
            case EV_DUD_EXPLODE:
                if (ent->surfType >= 0x1Du)
                    MyAssertHandler(
                        ".\\cgame\\cg_event.cpp",
                        1103,
                        0,
                        "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                        ent->surfType,
                        29);
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                CG_PlaySoundAlias(localClientNum, 1022, position, cgMedia.bulletHitLargeSound[ent->surfType]);
                if (cgMedia.fx->table[11].nonflesh[ent->surfType])
                    FX_PlayOrientedEffect(
                        localClientNum,
                        cgMedia.fx->table[11].nonflesh[ent->surfType],
                        cgameGlob->time,
                        position,
                        axis);
                if (weaponDef->projExplosionEffect)
                    FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                if (weaponDef->projExplosionSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projExplosionSound);
                if (weaponDef->projDudEffect)
                    FX_PlayOrientedEffect(localClientNum, weaponDef->projDudEffect, cgameGlob->time, position, axis);
                if (weaponDef->projDudSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projDudSound);
                return;
            case EV_DUD_IMPACT:
                if (ent->surfType >= 0x1Du)
                    MyAssertHandler(
                        ".\\cgame\\cg_event.cpp",
                        1122,
                        0,
                        "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                        ent->surfType,
                        29);
                ByteToDir(ent->eventParm, axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                CG_PlaySoundAlias(localClientNum, 1022, position, cgMedia.bulletHitLargeSound[ent->surfType]);
                if (cgMedia.fx->table[11].nonflesh[ent->surfType])
                    FX_PlayOrientedEffect(
                        localClientNum,
                        cgMedia.fx->table[11].nonflesh[ent->surfType],
                        cgameGlob->time,
                        position,
                        axis);
                if (weaponDef->projDudEffect)
                    FX_PlayOrientedEffect(localClientNum, weaponDef->projDudEffect, cgameGlob->time, position, axis);
                if (weaponDef->projDudSound)
                    CG_PlaySoundAlias(localClientNum, 1022, position, weaponDef->projDudSound);
                return;
            case EV_PLAY_FX:
                CG_PlayFx(localClientNum, cent, ent->lerp.apos.trBase);
                return;
            case EV_PLAY_FX_ON_TAG:
                CG_PlayFxOnTag(localClientNum, cent, ent->eventParm);
                return;
            case EV_PHYS_EXPLOSION_SPHERE:
                p_4c = (float)ent->eventParm;
                Ragdoll_ExplosionEvent(
                    localClientNum,
                    0,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    p_4c,
                    vec3_origin,
                    ent->lerp.u.turret.gunAngles[1]);
                innerRadius_4c = (float)ent->eventParm;
                DynEntCl_ExplosionEvent(
                    localClientNum,
                    0,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    innerRadius_4c,
                    (float *)vec3_origin,
                    ent->lerp.u.turret.gunAngles[1],
                    0,
                    0);
                return;
            case EV_PHYS_EXPLOSION_CYLINDER:
                p_4d = (float)ent->eventParm;
                Ragdoll_ExplosionEvent(
                    localClientNum,
                    1,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    p_4d,
                    vec3_origin,
                    ent->lerp.u.turret.gunAngles[1]);
                innerRadius_4d = (float)ent->eventParm;
                DynEntCl_ExplosionEvent(
                    localClientNum,
                    1,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    innerRadius_4d,
                    (float *)vec3_origin,
                    ent->lerp.u.turret.gunAngles[1],
                    0,
                    0);
                return;
            case EV_PHYS_EXPLOSION_JOLT:
                p_4e = (float)ent->eventParm;
                Ragdoll_ExplosionEvent(
                    localClientNum,
                    1,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    p_4e,
                    &ent->lerp.u.turret.gunAngles[1],
                    1.0);
                innerRadius_4e = (float)ent->eventParm;
                DynEntCl_ExplosionEvent(
                    localClientNum,
                    1,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    innerRadius_4e,
                    &ent->lerp.u.turret.gunAngles[1],
                    1.0,
                    0,
                    0);
                return;
            case EV_PHYS_JITTER:
                p_4f = (float)ent->eventParm;
                DynEntCl_JitterEvent(
                    localClientNum,
                    position,
                    ent->lerp.u.turret.gunAngles[0],
                    p_4f,
                    ent->lerp.u.turret.gunAngles[1],
                    ent->lerp.u.turret.gunAngles[2]);
                return;
            case EV_EARTHQUAKE:
                CG_StartShakeCamera(
                    localClientNum,
                    ent->lerp.u.turret.gunAngles[0],
                    ent->lerp.u.earthquake.duration,
                    cent->pose.origin,
                    ent->lerp.u.turret.gunAngles[1]);
                return;
            case EV_DETONATE:
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->detonateSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->detonateSound);
                return;
            case EV_NIGHTVISION_WEAR:
                if (isPlayerView && ((ps->eFlags & 0x300) != 0 || !*weaponDef->szXAnims[26]))
                    CG_PlayClientSoundAlias(localClientNum, cgMedia.nightVisionOn);
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->nightVisionWearSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->nightVisionWearSound);
                return;
            case EV_NIGHTVISION_REMOVE:
                if (isPlayerView && ((ps->eFlags & 0x300) != 0 || !*weaponDef->szXAnims[27]))
                    CG_PlayClientSoundAlias(localClientNum, cgMedia.nightVisionOff);
                if (isPlayerView)
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->nightVisionRemoveSoundPlayer);
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, weaponDef->nightVisionRemoveSound);
                return;
            case EV_OBITUARY:
                CG_Obituary(localClientNum, ent);
                return;
            case EV_NO_FRAG_GRENADE_HINT:
                v28 = cgameGlob->nextSnap;
                if ((v28->ps.otherFlags & 6) != 0 && ent->number == v28->ps.clientNum)
                    CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_NO_AMMO_FRAG_GRENADE);
                return;
            case EV_NO_SPECIAL_GRENADE_HINT:
                v27 = cgameGlob->nextSnap;
                if ((v27->ps.otherFlags & 6) != 0 && ent->number == v27->ps.clientNum)
                    CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_NO_AMMO_SPECIAL_GRENADE);
                return;
            case EV_TARGET_TOO_CLOSE_HINT:
                v26 = cgameGlob->nextSnap;
                if ((v26->ps.otherFlags & 6) != 0 && ent->number == v26->ps.clientNum)
                    CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_TARGET_TOO_CLOSE);
                return;
            case EV_TARGET_NOT_ENOUGH_CLEARANCE:
                v25 = cgameGlob->nextSnap;
                if ((v25->ps.otherFlags & 6) != 0 && ent->number == v25->ps.clientNum)
                    CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_NOT_ENOUGH_CLEARANCE);
                return;
            case EV_LOCKON_REQUIRED_HINT:
                v24 = cgameGlob->nextSnap;
                if ((v24->ps.otherFlags & 6) != 0 && ent->number == v24->ps.clientNum)
                    CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_LOCKON_REQUIRED);
                return;
            case EV_FOOTSTEP_SPRINT:
                if (cg_footsteps->current.enabled)
                {
                    if (isPlayerView)
                        CG_PlayEntitySoundAlias(
                            localClientNum,
                            ent->number,
                            *(&cgMedia.stepSprintSoundPlayer[offset] + eventParm));
                    else
                        CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepSprintSound[offset] + eventParm));
                }
                CG_EquipmentSound(
                    localClientNum,
                    ent->number,
                    isPlayerView,
                    (EquipmentSound_t)(offset != 0 ? EQS_QSPRINTING : EQS_SPRINTING));
                return;
            case EV_FOOTSTEP_RUN:
                if (!cg_footsteps->current.enabled)
                    goto LABEL_72;
                if (isPlayerView)
                    goto LABEL_70;
                CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepRunSound[offset] + eventParm));
                goto LABEL_72;
            case EV_FOOTSTEP_WALK:
                if (cg_footsteps->current.enabled)
                {
                    if (isPlayerView)
                        CG_PlayEntitySoundAlias(
                            localClientNum,
                            ent->number,
                            *(&cgMedia.stepWalkSoundPlayer[offset] + eventParm));
                    else
                        CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepWalkSound[offset] + eventParm));
                }
                goto LABEL_63;
            case EV_FOOTSTEP_PRONE:
                if (cg_footsteps->current.enabled)
                {
                    if (isPlayerView)
                        CG_PlayEntitySoundAlias(
                            localClientNum,
                            ent->number,
                            *(&cgMedia.stepProneSoundPlayer[offset] + eventParm));
                    else
                        CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepProneSound[offset] + eventParm));
                }
            LABEL_63:
                CG_EquipmentSound(localClientNum, ent->number, isPlayerView, offset != 0 ? EQS_QWALKING : EQS_WALKING);
                return;
            case EV_JUMP:
                if (isPlayerView)
                    LABEL_70 :
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepRunSoundPlayer[offset] + eventParm));
                else
                    CG_PlayEntitySoundAlias(localClientNum, ent->number, *(&cgMedia.stepRunSound[offset] + eventParm));
            LABEL_72:
                CG_EquipmentSound(
                    localClientNum,
                    ent->number,
                    isPlayerView,
                    (EquipmentSound_t)(offset != 0 ? EQS_QRUNNING : EQS_RUNNING));
                break;
            default:
                Com_Error(ERR_DROP, "Unknown event: %s", eventnames[event]);
                break;
            }
        }
    }
    else if (cg_debugEvents->current.enabled)
    {
        Com_Printf(21, "CG_EntityEvent:ZERO EVENT\n");
    }
}

void __cdecl CG_Obituary(int localClientNum, const entityState_s *ent)
{
    const char *v2; // eax
    weaponIconRatioType_t killIconRatio; // [esp+10h] [ebp-A0h]
    unsigned int mod; // [esp+14h] [ebp-9Ch]
    unsigned int attacker; // [esp+18h] [ebp-98h]
    bool iconHorzFlip; // [esp+1Fh] [ebp-91h]
    float iconWidth; // [esp+24h] [ebp-8Ch]
    unsigned int target; // [esp+2Ch] [ebp-84h]
    float iconHeight; // [esp+30h] [ebp-80h]
    char targetName[40]; // [esp+34h] [ebp-7Ch] BYREF
    char attackerName[40]; // [esp+5Ch] [ebp-54h] BYREF
    const clientInfo_t *attackerCI; // [esp+88h] [ebp-28h]
    const clientInfo_t *victimCI; // [esp+8Ch] [ebp-24h]
    char attackerColor; // [esp+93h] [ebp-1Dh]
    float baseIconSize; // [esp+94h] [ebp-1Ch]
    const char *s; // [esp+98h] [ebp-18h]
    char victimColor; // [esp+9Fh] [ebp-11h]
    const clientInfo_t *playerCI; // [esp+A0h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+A4h] [ebp-Ch]
    const playerState_s *ps; // [esp+A8h] [ebp-8h]
    Material *iconShader; // [esp+ACh] [ebp-4h]

    baseIconSize = 1.4f;
    target = ent->otherEntityNum;
    attacker = ent->attackerEntityNum;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    iconWidth = baseIconSize;
    iconHeight = baseIconSize;
    iconHorzFlip = 0;
    if ((ent->eventParm & 0x80) != 0)
    {
        mod = ent->eventParm & 0xFFFFFF7F;
        weapDef = 0;
        iconShader = Material_RegisterHandle("killicondied", 7);
    }
    else
    {
        mod = 0;
        weapDef = BG_GetWeaponDef(ent->eventParm);
        if (weapDef->killIcon)
        {
            iconShader = weapDef->killIcon;
            iconHorzFlip = weapDef->flipKillIcon != 0;
            killIconRatio = weapDef->killIconRatio;
            if (killIconRatio)
            {
                if (killIconRatio == WEAPON_ICON_RATIO_2TO1)
                {
                    iconWidth = baseIconSize + baseIconSize;
                }
                else
                {
                    if (weapDef->killIconRatio != WEAPON_ICON_RATIO_4TO1)
                    {
                        v2 = va("killIconRatio %d, weapon %s", weapDef->killIconRatio, weapDef->szInternalName);
                        MyAssertHandler(
                            ".\\cgame\\cg_event.cpp",
                            93,
                            0,
                            "%s\n\t%s",
                            "weapDef->killIconRatio == WEAPON_ICON_RATIO_4TO1",
                            v2);
                    }
                    iconWidth = baseIconSize + baseIconSize;
                    iconHeight = baseIconSize * 0.5;
                }
            }
        }
        else
        {
            iconShader = Material_RegisterHandle("killicondied", 7);
        }
    }
    switch (mod)
    {
    case 7u:
        iconShader = Material_RegisterHandle("killiconmelee", 7);
        iconWidth = baseIconSize;
        break;
    case 8u:
        iconShader = Material_RegisterHandle("killiconheadshot", 7);
        iconWidth = baseIconSize;
        break;
    case 9u:
        iconShader = Material_RegisterHandle("killiconcrush", 7);
        iconWidth = baseIconSize;
        break;
    case 0xBu:
        iconShader = Material_RegisterHandle("killiconfalling", 7);
        iconWidth = baseIconSize;
        break;
    case 0xCu:
        iconShader = Material_RegisterHandle("killiconsuicide", 7);
        iconWidth = baseIconSize;
        break;
    case 0xDu:
        iconShader = Material_RegisterHandle("killicondied", 7);
        iconWidth = baseIconSize;
        break;
    case 0xFu:
        iconShader = Material_RegisterHandle("killiconimpact", 7);
        iconWidth = baseIconSize;
        break;
    default:
        break;
    }
    if (target >= 0x40)
    {
        Com_Error(ERR_DROP, "CG_Obituary: target out of range");
        MyAssertHandler(
            ".\\cgame\\cg_event.cpp",
            139,
            0,
            "target doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            target,
            64);
    }
    victimCI = &cgArray[0].bgs.clientinfo[target];
    if (victimCI->infoValid)
    {
        CL_GetClientName(localClientNum, target, targetName, 38);
        victimColor = CG_DrawScoreboard_GetTeamColorIndex(victimCI->oldteam, localClientNum);
        if (cgArray[0].clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame\\cg_event.cpp",
                147,
                0,
                "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cgArray[0].clientNum,
                64);
        playerCI = &cgArray[0].bgs.clientinfo[cgArray[0].clientNum];
        if (playerCI->infoValid)
        {
            if (attacker < 0x40)
            {
                attackerCI = &cgArray[0].bgs.clientinfo[attacker];
                if (!attackerCI->infoValid)
                    return;
                CL_GetClientName(localClientNum, attacker, attackerName, 38);
                attackerColor = CG_DrawScoreboard_GetTeamColorIndex(attackerCI->oldteam, localClientNum);
            }
            else
            {
                attacker = 1022;
                attackerCI = 0;
                attackerName[0] = 0;
                attackerColor = 55;
            }
            ps = &cgArray[0].nextSnap->ps;
            if (attacker == target)
            {
                attackerName[0] = 0;
            }
            else if (attacker == ps->clientNum)
            {
                if (!cgArray[0].inKillCam)
                {
                    if (attackerCI->oldteam && victimCI->oldteam == attackerCI->oldteam)
                        s = va("CGAME_YOUKILLED", targetName, "CGAME_TEAMMATE");
                    else
                        s = va("CGAME_YOUKILLED", targetName);
                    CG_PriorityCenterPrint(localClientNum, s, 0);
                }
            }
            else if (target == ps->clientNum && attackerCI && !cgArray[0].inKillCam)
            {
                // KISAKTODO: double check the string literals here in va() `CGAME_...`
                if (attackerCI->oldteam && victimCI->oldteam == attackerCI->oldteam)
                    s = va("CGAME_YOUWEREKILLED", attackerName, "CGAME_TEAMMATE");
                else
                    s = va("CGAME_YOUWEREKILLED", attackerName);
                CG_PriorityCenterPrint(localClientNum, s, 0);
            }
            if (!cgArray[0].inKillCam)
                CL_DeathMessagePrint(
                    localClientNum,
                    attackerName,
                    attackerColor,
                    targetName,
                    victimColor,
                    iconShader,
                    iconWidth,
                    iconHeight,
                    iconHorzFlip);
        }
    }
}

void __cdecl CG_ItemPickup(int localClientNum, int weapIndex)
{
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    if (weapDef->weapClass != WEAPCLASS_ITEM)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (weapDef->offhandClass)
        {
            if (!cgArray[0].equippedOffHand)
                CG_SetEquippedOffHand(localClientNum, weapIndex);
        }
        else if (!cgArray[0].weaponSelect)
        {
            CG_SelectWeaponIndex(localClientNum, weapIndex);
        }
    }
}

void __cdecl CG_EquipmentSound(int localClientNum, int entNum, bool isPlayerView, EquipmentSound_t type)
{
    if (isPlayerView)
    {
        switch (type)
        {
        case EQS_RUNNING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.runningEquipmentSoundPlayer);
            break;
        case EQS_SPRINTING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.sprintingEquipmentSoundPlayer);
            break;
        case EQS_QWALKING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.qwalkingEquipmentSoundPlayer);
            break;
        case EQS_QRUNNING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.qrunningEquipmentSoundPlayer);
            break;
        case EQS_QSPRINTING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.qsprintingEquipmentSoundPlayer);
            break;
        default:
            if (type)
                MyAssertHandler(".\\cgame\\cg_event.cpp", 265, 0, "%s", "type == EQS_WALKING");
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.walkingEquipmentSoundPlayer);
            break;
        }
    }
    else
    {
        switch (type)
        {
        case EQS_RUNNING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.runningEquipmentSound);
            break;
        case EQS_SPRINTING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.sprintingEquipmentSound);
            break;
        case EQS_QWALKING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.qwalkingEquipmentSound);
            break;
        case EQS_QRUNNING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.qrunningEquipmentSound);
            break;
        case EQS_QSPRINTING:
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.qsprintingEquipmentSound);
            break;
        default:
            if (type)
                MyAssertHandler(".\\cgame\\cg_event.cpp", 296, 0, "%s", "type == EQS_WALKING");
            CG_PlayEntitySoundAlias(localClientNum, entNum, cgMedia.walkingEquipmentSound);
            break;
        }
    }
}

void __cdecl CG_PlayFx(int localClientNum, centity_s *cent, const float *angles)
{
    const FxEffectDef *fxDef; // [esp+4h] [ebp-30h]
    signed int fxId; // [esp+Ch] [ebp-28h]
    float axis[3][3]; // [esp+10h] [ebp-24h] BYREF

    fxId = cent->nextState.eventParm;
    if (fxId > 0 && fxId < 100)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        fxDef = cgsArray[0].fxs[fxId];
        if (!fxDef)
            MyAssertHandler(".\\cgame\\cg_event.cpp", 360, 0, "%s", "fxDef");
        AnglesToAxis(angles, axis);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        FX_PlayOrientedEffect(localClientNum, fxDef, cgArray[0].time, cent->pose.origin, axis);
    }
    else
    {
        Com_PrintError(21, "ERROR: CG_PlayFx called with invalid effect id %i\n", fxId);
    }
}

void __cdecl CG_PlayFxOnTag(int localClientNum, centity_s *cent, int eventParm)
{
    unsigned int ConfigstringConst; // eax
    char *v4; // eax
    const char *v5; // eax
    unsigned int v6; // eax
    char *v7; // eax
    const char *v8; // eax
    unsigned __int16 tagName; // [esp+0h] [ebp-1Ch] BYREF
    int dobjHandle; // [esp+4h] [ebp-18h]
    const char *tagAndEffect; // [esp+8h] [ebp-14h]
    const FxEffectDef *fxDef; // [esp+Ch] [ebp-10h]
    const cgs_t *cgs; // [esp+10h] [ebp-Ch]
    int fxId; // [esp+14h] [ebp-8h]
    int csIndex; // [esp+18h] [ebp-4h]

    csIndex = eventParm + 1698;
    tagAndEffect = CL_GetConfigString(localClientNum, eventParm + 1698);
    if (!*tagAndEffect)
    {
        ConfigstringConst = SV_GetConfigstringConst(csIndex);
        v4 = SL_ConvertToString(ConfigstringConst);
        v5 = va("tagAndEffect: '%s', server: '%s'", tagAndEffect, v4);
        MyAssertHandler(".\\cgame\\cg_event.cpp", 387, 0, "%s\n\t%s", "tagAndEffect[0]", v5);
    }
    if (!tagAndEffect[1])
    {
        v6 = SV_GetConfigstringConst(csIndex);
        v7 = SL_ConvertToString(v6);
        v8 = va("tagAndEffect: '%s', server: '%s'", tagAndEffect, v7);
        MyAssertHandler(".\\cgame\\cg_event.cpp", 388, 0, "%s\n\t%s", "tagAndEffect[1]", v8);
    }
    fxId = 10 * (*tagAndEffect - 48) + tagAndEffect[1] - 48;
    if (fxId <= 0 || fxId >= 100)
        MyAssertHandler(".\\cgame\\cg_event.cpp", 390, 0, "%s", "fxId > 0 && fxId < MAX_EFFECT_NAMES");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgs = cgsArray;
    fxDef = cgsArray[0].fxs[fxId];
    if (!fxDef)
        MyAssertHandler(".\\cgame\\cg_event.cpp", 393, 0, "%s", "fxDef");
    dobjHandle = cent->nextState.number;
    tagName = SL_GetString((char *)tagAndEffect + 2, 0);
    CG_PlayBoltedEffect(localClientNum, fxDef, dobjHandle, tagName);
    Scr_SetString(&tagName, 0);
}

void __cdecl CG_SetInvalidCmdHint(cg_s *cgameGlob, InvalidCmdHintType hintType)
{
    if (cgameGlob->invalidCmdHintType != hintType)
    {
        cgameGlob->invalidCmdHintType = hintType;
        cgameGlob->invalidCmdHintTime = cgameGlob->time;
    }
}

void __cdecl CG_StopWeaponSound(
    int localClientNum,
    bool isPlayerView,
    const WeaponDef *weaponDef,
    int entitynum,
    weaponstate_t weaponstate)
{
    switch (weaponstate)
    {
    case WEAPON_RELOADING:
    case WEAPON_RELOADING_INTERUPT:
        if (isPlayerView)
        {
            if (weaponDef->reloadEmptySoundPlayer)
                CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadEmptySoundPlayer);
            if (weaponDef->reloadSoundPlayer)
                CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadSoundPlayer);
        }
        else
        {
            if (weaponDef->reloadEmptySound)
                CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadEmptySound);
            if (weaponDef->reloadSound)
                CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadSound);
        }
        break;
    case WEAPON_RELOAD_START:
    case WEAPON_RELOAD_START_INTERUPT:
        if (isPlayerView)
            CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadStartSoundPlayer);
        else
            CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadStartSound);
        break;
    case WEAPON_RELOAD_END:
        if (isPlayerView)
            CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadEndSoundPlayer);
        else
            CG_StopSoundAlias(localClientNum, entitynum, weaponDef->reloadEndSound);
        break;
    default:
        return;
    }
}

void __cdecl CG_CheckEvents(int localClientNum, centity_s *cent)
{
    int v2; // [esp+0h] [ebp-14h]
    int previousEventSequence; // [esp+4h] [ebp-10h]
    int event; // [esp+8h] [ebp-Ch]
    unsigned __int8 oldEventParm; // [esp+Fh] [ebp-5h]
    int i; // [esp+10h] [ebp-4h]

    if (cent->nextState.eType <= 17)
    {
        if (cent->nextState.eventSequence)
        {
            previousEventSequence = cent->previousEventSequence;
            if (previousEventSequence <= cent->nextState.eventSequence + 64)
                v2 = cent->previousEventSequence;
            else
                v2 = previousEventSequence - 256;
            cent->previousEventSequence = v2;
            if (cent->nextState.eventSequence - cent->previousEventSequence > 4)
                cent->previousEventSequence = cent->nextState.eventSequence - 4;
            if (cent->previousEventSequence < cent->nextState.eventSequence)
            {
                CG_CalcEntityLerpPositions(localClientNum, cent);
                oldEventParm = cent->nextState.eventParm;
                for (i = cent->previousEventSequence; i != cent->nextState.eventSequence; ++i)
                {
                    event = cent->nextState.events[i & 3];
                    cent->nextState.eventParm = cent->nextState.eventParms[i & 3];
                    BG_GetEntityTypeName(event + 17);
                    KISAK_NULLSUB();
                    CG_EntityEvent(localClientNum, cent, event);
                }
                cent->nextState.eventParm = oldEventParm;
                cent->previousEventSequence = cent->nextState.eventSequence;
            }
            else
            {
                cent->previousEventSequence = cent->nextState.eventSequence;
            }
        }
        else
        {
            cent->previousEventSequence = 0;
        }
    }
    else
    {
        if (cent->nextState.eventSequence)
            MyAssertHandler(".\\cgame\\cg_event.cpp", 1277, 0, "%s", "!cent->nextState.eventSequence");
        if (!cent->previousEventSequence)
        {
            cent->previousEventSequence = 1;
            BG_GetEntityTypeName(cent->nextState.eType);
            KISAK_NULLSUB();
            CG_CalcEntityLerpPositions(localClientNum, cent);
            CG_EntityEvent(localClientNum, cent, cent->nextState.eType - 17);
        }
    }
}

