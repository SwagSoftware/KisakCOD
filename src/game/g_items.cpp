#include "game_public.h"
#include <server/sv_game.h>

#include <server/sv_world.h>
#include <script/scr_vm.h>
#include <script/scr_const.h>

#ifdef KISAK_MP
#include <game_mp/g_utils_mp.h>
#elif KISAK_SP
#include <server/sv_public.h>
#include "g_local.h"
#include "g_main.h"
#endif


//Line 53050:  0006 : 0053ff60       int32_t *itemRegistered     82ceff60     g_items.obj

void __cdecl Fill_Clip(playerState_s *ps, uint32_t weapon)
{
    int32_t ammomove; // [esp+0h] [ebp-18h]
    int32_t iClipIndex; // [esp+8h] [ebp-10h]
    int32_t iAmmoIndex; // [esp+Ch] [ebp-Ch]
    int32_t inclip; // [esp+14h] [ebp-4h]

    iAmmoIndex = BG_AmmoForWeapon(weapon);
    iClipIndex = BG_ClipForWeapon(weapon);
    if (weapon && weapon < BG_GetNumWeapons())
    {
        inclip = ps->ammoclip[iClipIndex];
        ammomove = BG_GetWeaponDef(weapon)->iClipSize - inclip;
        if (ammomove > ps->ammo[iAmmoIndex])
            ammomove = ps->ammo[iAmmoIndex];
        if (ammomove)
        {
            ps->ammo[iAmmoIndex] -= ammomove;
            ps->ammoclip[iClipIndex] += ammomove;
        }
    }
}

int32_t __cdecl Add_Ammo(gentity_s *ent, uint32_t weaponIndex, uint8_t weaponModel, int32_t count, int32_t fillClip)
{
    int32_t ammoIndex; // [esp+0h] [ebp-20h]
    int32_t oldClip; // [esp+4h] [ebp-1Ch]
    int32_t clipIndex; // [esp+8h] [ebp-18h]
    bool clipOnly; // [esp+Fh] [ebp-11h]
    int32_t maxWeaponAmmo; // [esp+10h] [ebp-10h]
    int32_t oldAmmo; // [esp+14h] [ebp-Ch]
    gclient_s *ps; // [esp+18h] [ebp-8h]
    WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]
    int32_t counta; // [esp+34h] [ebp+14h]

    if (!ent)
        MyAssertHandler(".\\game\\g_items.cpp", 82, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game\\g_items.cpp", 83, 0, "%s", "ent->client");
    ps = ent->client;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (!Com_BitCheckAssert(ps->ps.weapons, weaponIndex, 16) && !BG_PlayerHasCompatibleWeapon(&ps->ps, weaponIndex))
        return 0;
    clipOnly = 0;
    weapDef = BG_GetWeaponDef(weaponIndex);
    ammoIndex = BG_AmmoForWeapon(weaponIndex);
    clipIndex = BG_ClipForWeapon(weaponIndex);
    oldAmmo = ps->ps.ammo[ammoIndex];
    oldClip = ps->ps.ammoclip[clipIndex];
    maxWeaponAmmo = BG_GetAmmoPlayerMax(&ps->ps, weaponIndex, 0);
    ps->ps.ammo[ammoIndex] += count;
    if (BG_WeaponIsClipOnly(weaponIndex))
    {
        G_GivePlayerWeapon(&ps->ps, weaponIndex, weaponModel);
        clipOnly = 1;
    }
    if (fillClip || clipOnly)
        Fill_Clip(&ps->ps, weaponIndex);
    if (clipOnly)
    {
        ps->ps.ammo[ammoIndex] = 0;
    }
    else if (ps->ps.ammo[ammoIndex] > maxWeaponAmmo)
    {
        ps->ps.ammo[ammoIndex] = maxWeaponAmmo;
    }
    if (ps->ps.ammoclip[clipIndex] > weapDef->iClipSize)
        ps->ps.ammoclip[clipIndex] = weapDef->iClipSize;
    if (BG_GetWeaponDef(weaponIndex)->iSharedAmmoCapIndex >= 0)
    {
        counta = BG_GetMaxPickupableAmmo(&ps->ps, weaponIndex);
        if (counta < 0)
        {
            if (BG_WeaponIsClipOnly(weaponIndex))
            {
                ps->ps.ammoclip[clipIndex] += counta;
                if (ps->ps.ammoclip[clipIndex] <= 0)
                {
                    BG_TakePlayerWeapon(&ps->ps, weaponIndex, 1);
                    return 0;
                }
            }
            else
            {
                ps->ps.ammo[ammoIndex] += counta;
                if (ps->ps.ammo[ammoIndex] < 0)
                    ps->ps.ammo[ammoIndex] = 0;
            }
        }
    }
    return ps->ps.ammoclip[clipIndex] - oldClip + ps->ps.ammo[ammoIndex] - oldAmmo;
}

void __cdecl Touch_Item_Auto(gentity_s *ent, gentity_s *other, int32_t bTouched)
{
    ent->active = 1;
    Touch_Item(ent, other, bTouched);
}

void __cdecl Touch_Item(gentity_s *ent, gentity_s *other, int32_t touched)
{
    char *Guid; // eax
    int32_t number; // [esp-Ch] [ebp-68h]
    const char *szInternalName; // [esp-4h] [ebp-60h]
    gitem_s *item; // [esp+0h] [ebp-5Ch]
    char cleanname[68]; // [esp+4h] [ebp-58h] BYREF
    int32_t pickupEvent; // [esp+50h] [ebp-Ch] BYREF
    int32_t pickedUp; // [esp+54h] [ebp-8h]
    int32_t weapIndex; // [esp+58h] [ebp-4h]

    if (ent->active)
    {
        pickupEvent = 0;
        ent->active = 0;
        if (other->client)
        {
            if (other->health >= 1 && !level.clientIsSpawning)
            {
                weapIndex = ent->s.index.brushmodel % 128;
                item = &bg_itemlist[ent->s.index.brushmodel];
                if (BG_CanItemBeGrabbed(&ent->s, &other->client->ps, touched))
                {
#ifdef KISAK_MP
                    I_strncpyz(cleanname, other->client->sess.cs.name, 64);
                    I_CleanStr(cleanname);
                    szInternalName = BG_GetWeaponDef(weapIndex)->szInternalName;
                    number = other->s.number;
                    Guid = SV_GetGuid(other->s.number);
                    G_LogPrintf("Weapon;%s;%d;%s;%s\n", Guid, number, cleanname, szInternalName);
#endif
                    iassert(item);  
                    iassert(item->giType == IT_WEAPON);
                    pickedUp = WeaponPickup(ent, other, &pickupEvent, touched);

#ifdef KISAK_MP
                    if (pickupEvent)
                    {
                        if (other->client->sess.predictItemPickup)
                            G_AddPredictableEvent(other, pickupEvent, weapIndex);
                        else
                            G_AddEvent(other, pickupEvent, weapIndex);
                    }
                    if (pickedUp)
                    {
                        if (ent->s.eType == 4)
                            Scr_Notify(ent, scr_const.death, 0);
                        G_FreeEntity(ent);
                    }
#elif KISAK_SP
                    if (pickupEvent)
                        G_AddEvent(other, pickupEvent, weapIndex);
                    if (pickedUp)
                    {
                        if (ent->s.eType == 3)
                            Scr_Notify(ent, scr_const.death, 0);
                        G_FreeEntity(ent);
                    }
#endif
                }
                else
                {
                    PrintMessage_CannotGrabItem(ent, other, touched, item, weapIndex);
                }
            }
        }
    }
}

int32_t __cdecl WeaponPickup(gentity_s *weaponEnt, gentity_s *player, int32_t *pickupEvent, int32_t touched)
{
    int32_t weapIdx; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4h] [ebp-8h]

    if (!weaponEnt)
        MyAssertHandler(".\\game\\g_items.cpp", 503, 0, "%s", "weaponEnt");
    if (!player)
        MyAssertHandler(".\\game\\g_items.cpp", 504, 0, "%s", "player");
    if (!player->client)
        MyAssertHandler(".\\game\\g_items.cpp", 505, 0, "%s", "player->client");
    weapIdx = weaponEnt->s.index.brushmodel % 128;
    weapDef = BG_GetWeaponDef(weapIdx);
    if (!BG_PlayerCanPickUpWeaponType(weapDef, &player->client->ps))
        return 0;
    if (touched)
        return WeaponPickup_Touch(weaponEnt, player, weapIdx, pickupEvent);
    return WeaponPickup_Grab(weaponEnt, player, weapIdx, pickupEvent);
}

int32_t __cdecl WeaponPickup_Grab(gentity_s *weaponEnt, gentity_s *player, int32_t weapIdx, int32_t *pickupEvent)
{
    uint8_t weaponModel; // [esp+3h] [ebp-Dh]
    int32_t gotWeapon; // [esp+4h] [ebp-Ch]
    gentity_s *droppedEnt; // [esp+8h] [ebp-8h] BYREF
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if (!weaponEnt)
        MyAssertHandler(".\\game\\g_items.cpp", 429, 0, "%s", "weaponEnt");
    if (!player)
        MyAssertHandler(".\\game\\g_items.cpp", 430, 0, "%s", "player");
    if (!player->client)
        MyAssertHandler(".\\game\\g_items.cpp", 431, 0, "%s", "player->client");
    droppedEnt = 0;
    weaponModel = weaponEnt->s.index.brushmodel / 128;
    weapDef = BG_GetWeaponDef(weapIdx);
    if (weaponEnt->s.eType == 4)
    {
        *pickupEvent = 10;
        WeaponPickup_Notifies(weaponEnt, 0, player, weapDef);
        return 1;
    }
    else
    {
        if (!weaponEnt->r.inuse)
            MyAssertHandler(".\\game\\g_items.cpp", 444, 0, "%s", "weaponEnt->r.inuse");
        weaponEnt->flags |= 0x1000000u;
        gotWeapon = WeaponPickup_AddWeapon(weaponEnt, player, weapIdx, weaponModel, &droppedEnt);
        if (!weaponEnt->r.inuse)
            MyAssertHandler(".\\game\\g_items.cpp", 449, 0, "%s", "weaponEnt->r.inuse");
        weaponEnt->flags &= ~0x1000000u;
        if (gotWeapon)
        {
            *pickupEvent = 9;
            WeaponPickup_AddAmmoForNewWeapon(weaponEnt, player);
            if (droppedEnt)
                WeaponPickup_LeechFromWeaponEnt(droppedEnt, player, 0, 0, 1);
            WeaponPickup_Notifies(weaponEnt, droppedEnt, player, weapDef);
            if (weapDef->weapType != WEAPTYPE_GRENADE || weapDef->offhandClass == OFFHAND_CLASS_NONE)
                G_SelectWeaponIndex(player - g_entities, weapIdx);
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

int32_t __cdecl WeaponPickup_AddWeapon(
    gentity_s *ent,
    gentity_s *other,
    int32_t weapon,
    uint8_t weaponModel,
    gentity_s **pDroppedWeapon)
{
    const char *v6; // eax
    int32_t passEntityNum; // [esp+Ch] [ebp-58h]
    uint32_t bitNum; // [esp+14h] [ebp-50h]
    gentity_s *droppedEnt; // [esp+18h] [ebp-4Ch]
    trace_t trace; // [esp+1Ch] [ebp-48h] BYREF
    int32_t mask; // [esp+48h] [ebp-1Ch]
    float up[3]; // [esp+4Ch] [ebp-18h] BYREF
    playerState_s *ps; // [esp+58h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+5Ch] [ebp-8h]
    int32_t playerWeapIdx; // [esp+60h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game\\g_items.cpp", 192, 0, "%s", "ent");
    if (!other)
        MyAssertHandler(".\\game\\g_items.cpp", 193, 0, "%s", "other");
    ps = &other->client->ps;
    weapDef = BG_GetWeaponDef(weapon);
    if (weapDef->inventoryType)
        goto LABEL_6;
    if (!ps->weapon)
        goto LABEL_12;
    bitNum = ps->weapon;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (!Com_BitCheckAssert(ps->weapons, bitNum, 16))
        return 0;
LABEL_12:
    if (!BG_PlayerWeaponsFull_Primaries(ps))
    {
    LABEL_6:
        G_GivePlayerWeapon(ps, weapon, weaponModel);
        return 1;
    }
    playerWeapIdx = CurrentPrimaryWeapon(ps);
    if (playerWeapIdx)
    {
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (!Com_BitCheckAssert(ps->weapons, playerWeapIdx, 16))
            MyAssertHandler(".\\game\\g_items.cpp", 227, 0, "%s", "BG_PlayerHasWeapon( ps, playerWeapIdx )");
        droppedEnt = Drop_Weapon(other, playerWeapIdx, ps->weaponmodels[playerWeapIdx], 0);
        if (droppedEnt)
        {
            droppedEnt->spawnflags = ent->spawnflags & 0xFFFFFFFE;
            droppedEnt->s.groundEntityNum = ent->s.groundEntityNum;
            if ((droppedEnt->flags & 0x1000) != 0 && ent->tagInfo && ent->tagInfo->parent)
            {
                G_SetOrigin(droppedEnt, ent->r.currentOrigin);
                G_SetAngle(droppedEnt, ent->r.currentAngles);
                G_EntLinkTo(droppedEnt, ent->tagInfo->parent, ent->tagInfo->name);
            }
            else
            {
                up[0] = ent->r.currentOrigin[0];
                up[1] = ent->r.currentOrigin[1];
                up[2] = ent->r.currentOrigin[2] + 2.0;
                mask = G_ItemClipMask(ent);
                if (droppedEnt->r.ownerNum.isDefined())
                {
                    passEntityNum = droppedEnt->r.ownerNum.entnum();
                    G_TraceCapsule(&trace, ent->r.currentOrigin, droppedEnt->r.mins, droppedEnt->r.maxs, up, passEntityNum, mask);
                }
                else
                {
                    G_TraceCapsule(&trace, ent->r.currentOrigin, droppedEnt->r.mins, droppedEnt->r.maxs, up, 1023, mask);
                }
                if (trace.allsolid)
                {
                    G_SetOrigin(droppedEnt, ent->r.currentOrigin);
                }
                else
                {
                    Vec3Lerp(ent->r.currentOrigin, up, trace.fraction, up);
                    if (droppedEnt->r.ownerNum.isDefined())
                    {
                        G_TraceCapsule(&trace, up, droppedEnt->r.mins, droppedEnt->r.maxs, ent->r.currentOrigin, droppedEnt->r.ownerNum.entnum(), mask);
                    }
                    else
                    {
                        G_TraceCapsule(&trace, up, droppedEnt->r.mins, droppedEnt->r.maxs, ent->r.currentOrigin, 1023, mask);
                    }
                    Vec3Lerp(up, ent->r.currentOrigin, trace.fraction, up);
                    G_SetOrigin(droppedEnt, up);
                }
                G_SetAngle(droppedEnt, ent->r.currentAngles);
            }
            SV_LinkEntity(droppedEnt);
        }
        *pDroppedWeapon = droppedEnt;
        G_GivePlayerWeapon(ps, weapon, weaponModel);
        return 1;
    }
    else
    {
#ifdef KISAK_MP
        v6 = va("%c \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"", 102);
        SV_GameSendServerCommand(other - g_entities, SV_CMD_CAN_IGNORE, v6);
        return 0;
#elif KISAK_SP
        SV_GameSendServerCommand(other - g_entities, va("gm \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\""));
        return 0;
#endif
    }
}

int32_t __cdecl CurrentPrimaryWeapon(playerState_s *ps)
{
    int32_t weapIdx; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!ps)
        MyAssertHandler(".\\game\\g_items.cpp", 148, 0, "%s", "ps");
    if (!ps->weapon)
        return 0;
    weapIdx = ps->weapon;
    weapDef = BG_GetWeaponDef(weapIdx);
    if (weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
    {
        weapIdx = weapDef->altWeaponIndex;
        weapDef = BG_GetWeaponDef(weapIdx);
    }
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (!Com_BitCheckAssert(ps->weapons, weapIdx, 16))
        return 0;
    if (weapDef->inventoryType)
        return 0;
    return weapIdx;
}

int32_t __cdecl G_ItemClipMask(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game\\g_items.cpp", 176, 0, "%s", "ent");
    if (ent->clipmask)
        return ent->clipmask;
    else
        return 1169;
}

bool __cdecl WeaponPickup_LeechFromWeaponEnt(
    gentity_s *weaponEnt,
    gentity_s *player,
    int32_t haveExactWeapon,
    int32_t *pickupEvent,
    bool suppressNotifies)
{
    int32_t v5; // eax
    uint8_t weaponModel; // [esp+3h] [ebp-1Dh]
    int32_t ammoTaken; // [esp+4h] [ebp-1Ch]
    uint32_t weapIdx; // [esp+8h] [ebp-18h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-14h]
    int32_t ammoAvailable; // [esp+10h] [ebp-10h]
    item_ent_t *itemEnt; // [esp+14h] [ebp-Ch]
    int32_t i; // [esp+18h] [ebp-8h]
    bool removedAnyAmmo; // [esp+1Fh] [ebp-1h]

    if (!weaponEnt)
        MyAssertHandler(".\\game\\g_items.cpp", 295, 0, "%s", "weaponEnt");
    if (!player)
        MyAssertHandler(".\\game\\g_items.cpp", 296, 0, "%s", "player");
    removedAnyAmmo = 0;
    for (i = 0; i < 2; ++i)
    {
        v5 = weaponEnt->item[i].index % 128;
        weapIdx = v5;
        if (v5 > 0)
        {
            weaponModel = weaponEnt->item[i].index / 128;
            weapDef = BG_GetWeaponDef(v5);
            itemEnt = &weaponEnt->item[i];
            ammoAvailable = itemEnt->ammoCount;
            if (haveExactWeapon)
                ammoAvailable += weaponEnt->item[i].clipAmmoCount;
            ammoTaken = Add_Ammo(player, weapIdx, weaponModel, ammoAvailable, 0);
            if (ammoTaken)
            {
                removedAnyAmmo = 1;
                if (!suppressNotifies && pickupPrints->current.enabled)
                    PrintPlayerPickupMessage(player, weapIdx, weapDef);
                itemEnt->ammoCount -= ammoTaken;
                if (itemEnt->ammoCount < 0)
                {
                    weaponEnt->item[i].clipAmmoCount += weaponEnt->item[i].ammoCount;
                    itemEnt->ammoCount = 0;
                    if (weaponEnt->item[i].clipAmmoCount < 0)
                        weaponEnt->item[i].clipAmmoCount = 0;
                }
            }
        }
    }
    if (!suppressNotifies && removedAnyAmmo)
    {
        if (!pickupEvent)
            MyAssertHandler(".\\game\\g_items.cpp", 345, 0, "%s", "pickupEvent");
        *pickupEvent = 9;
    }
    return haveExactWeapon && removedAnyAmmo;
}

void __cdecl PrintPlayerPickupMessage(gentity_s *player, uint32_t weapIdx, WeaponDef *weapDef)
{
    const char *v3; // eax

    iassert(player);
    iassert(weapDef);

#ifdef KISAK_MP
    if (BG_WeaponIsClipOnly(weapIdx))
        v3 = va("*WARNING* One or more selections were skipped as they could not be interpreted as c data", 102, weapDef->szDisplayName);
    else
        v3 = va("*WARNING* One or more selections were skipped as they could not be interpreted as c data", 102, weapDef->szDisplayName);
    SV_GameSendServerCommand(player - g_entities, SV_CMD_CAN_IGNORE, v3);
#elif KISAK_SP
    const char *v8;
    if (BG_WeaponIsClipOnly(weapIdx))
        v8 = va("gm \"GAME_PICKUP_CLIPONLY_AMMO %s\"", weapDef->szDisplayName);
    else
        v8 = va("gm \"GAME_PICKUP_AMMO %s\"", weapDef->szDisplayName);
    SV_GameSendServerCommand(player - g_entities, v8);
#endif
}

void __cdecl WeaponPickup_AddAmmoForNewWeapon(gentity_s *weaponEnt, gentity_s *player)
{
    gclient_s *client; // esi
    int32_t i; // [esp+4h] [ebp-18h]
    uint8_t weaponModel; // [esp+Bh] [ebp-11h]
    int32_t weapon; // [esp+10h] [ebp-Ch]
    int32_t clipAmmo; // [esp+14h] [ebp-8h]
    WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        weapon = weaponEnt->item[i].index % 128;
        weaponModel = weaponEnt->item[i].index / 128;
        if (weapon > 0)
        {
            weapDef = BG_GetWeaponDef(weapon);
            clipAmmo = weaponEnt->item[i].clipAmmoCount;
            if (clipAmmo >= 0)
            {
                if (clipAmmo > weapDef->iClipSize)
                    MyAssertHandler(".\\game\\g_items.cpp", 379, 0, "%s", "clipAmmo <= weapDef->iClipSize");
                client = player->client;
                client->ps.ammoclip[BG_ClipForWeapon(weapon)] = clipAmmo;
            }
            Add_Ammo(player, weapon, weaponModel, weaponEnt->item[i].ammoCount, clipAmmo == -1);
        }
    }
}

void __cdecl WeaponPickup_Notifies(
    gentity_s *thisItem,
    gentity_s *newDroppedItem,
    gentity_s *player,
    WeaponDef *weapDef)
{
    if (!thisItem)
        MyAssertHandler(".\\game\\g_items.cpp", 392, 0, "%s", "thisItem");
    if (!player)
        MyAssertHandler(".\\game\\g_items.cpp", 393, 0, "%s", "player");
    if (!weapDef)
        MyAssertHandler(".\\game\\g_items.cpp", 394, 0, "%s", "weapDef");
    if (newDroppedItem)
        Scr_AddEntity(newDroppedItem);
    else
        Scr_AddUndefined();
    Scr_AddEntity(player);
    Scr_Notify(thisItem, scr_const.trigger, 2u);
}

bool __cdecl WeaponPickup_Touch(gentity_s *weaponEnt, gentity_s *player, int32_t weapIdx, int32_t *pickupEvent)
{
    gclient_s *client; // [esp+0h] [ebp-14h]
    bool haveExactWeapon; // [esp+8h] [ebp-Ch]
    bool removeWeaponFromWorld; // [esp+Fh] [ebp-5h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if (!weaponEnt)
        MyAssertHandler(".\\game\\g_items.cpp", 474, 0, "%s", "weaponEnt");
    if (!player)
        MyAssertHandler(".\\game\\g_items.cpp", 475, 0, "%s", "player");
    if (!player->client)
        MyAssertHandler(".\\game\\g_items.cpp", 476, 0, "%s", "player->client");
    if (!pickupEvent)
        MyAssertHandler(".\\game\\g_items.cpp", 477, 0, "%s", "pickupEvent");
    weapDef = BG_GetWeaponDef(weapIdx);
    client = player->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    haveExactWeapon = Com_BitCheckAssert(client->ps.weapons, weapIdx, 16);
    if (!haveExactWeapon && !BG_PlayerHasCompatibleWeapon(&player->client->ps, weapIdx))
        return 0;
    removeWeaponFromWorld = WeaponPickup_LeechFromWeaponEnt(weaponEnt, player, haveExactWeapon, pickupEvent, 0);
    if (*pickupEvent)
        WeaponPickup_Notifies(weaponEnt, 0, player, weapDef);
    return removeWeaponFromWorld;
}

void __cdecl PrintMessage_CannotGrabItem(gentity_s *ent, gentity_s *player, int32_t touched, gitem_s *item, int32_t weapIndex)
{
    WeaponDef *WeaponDef; // eax
    const char *v6; // eax
    gclient_s *ps; // [esp+0h] [ebp-4h]

    iassert(ent);
    iassert(player);
    iassert(player->client);
    iassert(item);

    if (!touched)
    {
#ifdef KISAK_MP
        if (ent->s.clientNum != player->s.number)
#endif
        {
            iassert(item->giType == IT_WEAPON);

#ifdef KISAK_MP
            ps = player->client;
            iassert(ps);
            if (Com_BitCheckAssert(ps->ps.weapons, weapIndex, 16))
            {
                WeaponDef = BG_GetWeaponDef(weapIndex);
                v6 = va("%c \"GAME_PICKUP_CANTCARRYMOREAMMO", 102, WeaponDef->szDisplayName);
            }
            else
            {
                v6 = va("%c \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"", 102);
            }
            SV_GameSendServerCommand(player - g_entities, SV_CMD_CAN_IGNORE, v6);
#elif KISAK_SP
            const char *v10;
            if (BG_PlayerHasWeapon(&player->client->ps, weapIndex))
            {
                WeaponDef = BG_GetWeaponDef(weapIndex);
                v10 = va("gm \"GAME_PICKUP_CANTCARRYMOREAMMO %s\"", WeaponDef->szDisplayName);
            }
            else
            {
                v10 = va("gm \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"");
            }
            SV_GameSendServerCommand(player - g_entities, v10);
#endif
        }
    }
}

#ifdef KISAK_MP
void __cdecl DroppedItemClearOwner(gentity_s *pSelf)
{
    pSelf->s.clientNum = 64;
}
#endif

void __cdecl G_GetItemClassname(const gitem_s *item, uint16_t *out)
{
    char classname[256]; // [esp+4h] [ebp-108h] BYREF
    WeaponDef *weapDef; // [esp+108h] [ebp-4h]

    weapDef = BG_GetWeaponDef((((char *)item - (char *)bg_itemlist) >> 2) % 128);
    Com_sprintf(classname, 0x100u, "weapon_%s", weapDef->szInternalName);
    G_SetConstString(out, classname);
}

gentity_s *__cdecl Drop_Item(gentity_s *ent, const gitem_s *item, float angle, int32_t novelocity)
{
    float velocity[3]; // [esp+10h] [ebp-24h] BYREF
    float angles[3]; // [esp+1Ch] [ebp-18h] BYREF
    float vPos[3]; // [esp+28h] [ebp-Ch] BYREF

    angles[0] = ent->r.currentAngles[0];
    angles[1] = ent->r.currentAngles[1] + angle;

    angles[0] = 0.0;
    angles[2] = 0.0;

    if (novelocity)
    {
        velocity[0] = 0.0;
        velocity[1] = 0.0;
        velocity[2] = 0.0;
    }
    else
    {
        AngleVectors(angles, velocity, 0, 0);
#ifdef KISAK_MP
        Vec3Scale(velocity, g_dropForwardSpeed->current.value, velocity);
        velocity[2] = crandom() * g_dropUpSpeedRand->current.value + g_dropUpSpeedBase->current.value + velocity[2];
        velocity[0] = crandom() * g_dropHorzSpeedRand->current.value + velocity[0];
        velocity[1] = crandom() * g_dropHorzSpeedRand->current.value + velocity[1];
#elif KISAK_SP
        Vec3Scale(velocity, 150.0f, velocity);
        velocity[2] = (float)((float)((float)(G_crandom() * (float)50.0) + velocity[2]) + (float)200.0);
#endif
    }

#ifdef KISAK_MP
    vPos[0] = ent->r.currentOrigin[0];
    vPos[1] = ent->r.currentOrigin[1];
    vPos[2] = ent->r.currentOrigin[2];
    vPos[2] = (ent->r.maxs[2] - ent->r.mins[2]) * 0.5 + vPos[2];
#elif KISAK_SP
    float v8 = (float)(ent->r.maxs[2] - ent->r.mins[2]);
    vPos[0] = ent->r.currentOrigin[0];
    vPos[2] = (float)((float)((float)v8 * (float)0.5) + ent->r.currentOrigin[2]);
    vPos[1] = ent->r.currentOrigin[1];
#endif

    return LaunchItem(item, vPos, angles, velocity, ent->s.number);
}

gentity_s *__cdecl LaunchItem(const gitem_s *item, float *origin, float *angles, float *velocity, int32_t ownerNum)
{
    char *Name; // eax
    gentity_s *dropped; // [esp+30h] [ebp-18h]
    int32_t dropIdx; // [esp+34h] [ebp-14h]
    int32_t itemIndex; // [esp+38h] [ebp-10h]
    uint8_t weapModel; // [esp+43h] [ebp-5h]
    WeaponDef *weapDef; // [esp+44h] [ebp-4h]

    iassert(item);
    iassert(!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2]));
    iassert(!IS_NAN((velocity)[0]) && !IS_NAN((velocity)[1]) && !IS_NAN((velocity)[2]));

    itemIndex = ((char *)item - (char *)bg_itemlist) >> 2;
    dropped = G_Spawn();
    dropIdx = GetFreeDropCueIdx();
    level.droppedWeaponCue[dropIdx].setEnt(dropped);
    dropped->s.eType = ET_ITEM;
    dropped->s.index.brushmodel = itemIndex;
    G_GetItemClassname(item, &dropped->classname);
    dropped->item[0].index = itemIndex;

    iassert(item->giType == IT_WEAPON);

    dropped->r.mins[0] = -1.0;
    dropped->r.mins[1] = -1.0;
    dropped->r.mins[2] = -1.0;

    dropped->r.maxs[0] = 1.0;
    dropped->r.maxs[1] = 1.0;
    dropped->r.maxs[2] = 1.0;

#ifdef KISAK_MP
    dropped->r.contents = 1079771400;
    dropped->r.contents |= 0x200000u;
    dropped->s.clientNum = ownerNum;
#elif KISAK_SP
    dropped->r.contents = 1081868552;
#endif
    weapModel = itemIndex / 128;
    weapDef = BG_GetWeaponDef(itemIndex % 128);

    iassert(weapDef);

    if (weapDef->worldModel[weapModel])
    {
        Name = (char *)XModelGetName(weapDef->worldModel[weapModel]);
        G_SetModel(dropped, Name);
    }

    G_DObjUpdate(dropped);
#ifdef KISAK_MP
    dropped->handler = 16;
#elif KISAK_SP
    dropped->handler = 12;
#endif
    G_SetOrigin(dropped, origin);
    dropped->s.lerp.pos.trType = TR_GRAVITY;
    dropped->s.lerp.pos.trTime = level.time;
    dropped->s.lerp.pos.trDelta[0] = velocity[0];
    dropped->s.lerp.pos.trDelta[1] = velocity[1];
    dropped->s.lerp.pos.trDelta[2] = velocity[2];

    G_SetAngle(dropped, angles);

    iassert(!IS_NAN((dropped->s.lerp.pos.trDelta)[0]) && !IS_NAN((dropped->s.lerp.pos.trDelta)[1]) && !IS_NAN((dropped->s.lerp.pos.trDelta)[2]));

#ifdef KISAK_MP
    dropped->flags = 16;
    dropped->nextthink = level.time + 1000;
#elif KISAK_SP
    dropped->flags = 2112;
#endif
    SV_LinkEntity(dropped);
    return dropped;
}

int32_t __cdecl GetFreeDropCueIdx()
{
    float diff[3]; // [esp+0h] [ebp-38h] BYREF
    int32_t weapIndex; // [esp+Ch] [ebp-2Ch]
    WeaponDef *weapDef; // [esp+10h] [ebp-28h]
    int32_t iBest; // [esp+14h] [ebp-24h]
    int32_t j; // [esp+18h] [ebp-20h]
    int32_t maxDroppedWeapon; // [esp+1Ch] [ebp-1Ch]
    float fDistSqrd; // [esp+20h] [ebp-18h]
    float fBestDistSqrd; // [esp+24h] [ebp-14h]
    gclient_s *pClient; // [esp+28h] [ebp-10h]
    float fClientDistSqrd; // [esp+2Ch] [ebp-Ch]
    gentity_s *ent; // [esp+30h] [ebp-8h]

#ifdef KISAK_MP
    maxDroppedWeapon = g_maxDroppedWeapons->current.integer;
    iassert(maxDroppedWeapon >= 1 && maxDroppedWeapon <= 32);
#elif KISAK_SP
    maxDroppedWeapon = 32; // LWSS Hack. There is no dvar on SP, but the array is len 32
#endif

    iBest = -1;
    fBestDistSqrd = -1.0;
    for (int i = 0; i < maxDroppedWeapon; ++i)
    {
        if (!level.droppedWeaponCue[i].isDefined())
            return i;

        ent = level.droppedWeaponCue[i].ent();
        if ((ent->flags & 0x1000000) == 0)
        {
            if (bg_itemlist[ent->s.index.brushmodel].giType != IT_WEAPON)
                MyAssertHandler(".\\game\\g_items.cpp", 670, 0, "%s", "bg_itemlist[ ent->s.index.item ].giType == IT_WEAPON");
            weapIndex = ent->item[0].index % 128;
            weapDef = BG_GetWeaponDef(weapIndex);
            if (!weapDef->avoidDropCleanup)
            {
                fDistSqrd = 9.9999803e11f;
                for (j = 0; j < level.maxclients; ++j)
                {
                    pClient = &level.clients[j];
#ifdef KISAK_MP
                    if (pClient->sess.connected == CON_CONNECTED && pClient->sess.sessionState == SESS_STATE_PLAYING)
#endif
                    {
                        Vec3Sub(ent->r.currentOrigin, g_entities[j].r.currentOrigin, diff);
                        fClientDistSqrd = Vec3LengthSq(diff);
                        if (fDistSqrd > fClientDistSqrd)
                            fDistSqrd = fClientDistSqrd;
                    }
                }
                if (fBestDistSqrd < fDistSqrd)
                {
                    fBestDistSqrd = fDistSqrd;
                    iBest = i;
                }
            }
        }
    }
    if (iBest == -1)
    {
        Com_PrintWarning(16, "Could not find a suitable weapon entity to free out of %i possible.  Using index zero.\n", maxDroppedWeapon);
        iBest = 0;
    }
    ent = level.droppedWeaponCue[iBest].ent();

    iassert(ent);

    G_FreeEntity(ent);
    level.droppedWeaponCue[iBest].setEnt(NULL);
    return iBest;
}

bool __cdecl PlayerHasAnyAmmoToTransferToWeapon(gentity_s *player, uint32_t transferWeapon)
{
    if (!player->client)
        MyAssertHandler(".\\game\\g_items.cpp", 890, 0, "%s", "player->client");
    return player->client->ps.ammoclip[BG_ClipForWeapon(transferWeapon)] > 0
        || GetNonClipAmmoToTransferToWeaponEntity(player, transferWeapon) > 0;
}

int32_t __cdecl GetNonClipAmmoToTransferToWeaponEntity(gentity_s *player, uint32_t transferWeapon)
{
    int32_t ammoIndex; // [esp+0h] [ebp-10h]
    int32_t ammoDrop; // [esp+Ch] [ebp-4h]

    ammoIndex = BG_AmmoForWeapon(transferWeapon);
    BG_ClipForWeapon(transferWeapon);
    ammoDrop = player->client->ps.ammo[ammoIndex]
        - BG_GetAmmoPlayerMax(&player->client->ps, transferWeapon, transferWeapon);
        if (ammoDrop < 0)
            return 0;
        return ammoDrop;
}

gentity_s *__cdecl Drop_Weapon(gentity_s *ent, int32_t weapIdx, uint8_t weaponModel, uint32_t tag)
{
    WeaponDef *WeaponDef; // eax
    gclient_s *v7; // esi
    int32_t v8; // eax
    float *currentOrigin; // [esp+Ch] [ebp-80h]
    float *trBase; // [esp+10h] [ebp-7Ch]
    gclient_s *client; // [esp+14h] [ebp-78h]
    trace_t trace; // [esp+18h] [ebp-74h] BYREF
    float vCenter[3]; // [esp+44h] [ebp-48h] BYREF
    mat4x3 tagMat; // [esp+50h] [ebp-3Ch] BYREF
    uint32_t altWeaponIndex; // [esp+80h] [ebp-Ch]
    gentity_s *dropEnt; // [esp+84h] [ebp-8h]
    const gitem_s *weapItem; // [esp+88h] [ebp-4h]

    if (BG_GetWeaponDef(weapIdx)->inventoryType == WEAPINVENTORY_ALTMODE)
    {
        altWeaponIndex = BG_GetWeaponDef(weapIdx)->altWeaponIndex;
        if (!altWeaponIndex)
        {
            WeaponDef = BG_GetWeaponDef(weapIdx);
            Com_PrintError(
                15,
                "Drop_Weapon(): Trying to drop alt-type weapon, \"%s\", but it has no corresponding altWeapon set.\n",
                WeaponDef->szInternalName);
            return 0;
        }
        weapIdx = altWeaponIndex;
    }
    weapItem = &bg_itemlist[128 * weaponModel + weapIdx];
    iassert(weapItem->giType == IT_WEAPON);
    if (ent->client)
    {
        client = ent->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (!Com_BitCheckAssert(client->ps.weapons, weapIdx, 16) || !PlayerHasAnyAmmoToTransferToWeapon(ent, weapIdx))
            goto LABEL_15;
    }
    if (!BG_GetWeaponDef(weapIdx)->bClipOnly || (v7 = ent->client, v7->ps.ammoclip[BG_ClipForWeapon(weapIdx)]))
    {
        dropEnt = Drop_Item(ent, weapItem, 0.0, 0);
        if (ent->client)
        {
            TransferPlayerAmmoToWeaponEntity(ent, dropEnt, weapIdx);
            BG_TakePlayerWeapon(&ent->client->ps, weapIdx, 1);
        }
        else
        {
            TransferRandomAmmoToWeaponEntity(dropEnt, weapIdx);
        }
        if (tag)
        {
            if (G_DObjGetWorldTagMatrix(ent, tag, tagMat))
            {
                Vec3Avg(ent->r.mins, ent->r.maxs, vCenter);
                Vec3Add(vCenter, ent->r.currentOrigin, vCenter);
                v8 = G_ItemClipMask(dropEnt);
                G_TraceCapsule(&trace, vCenter, dropEnt->r.mins, dropEnt->r.maxs, tagMat[3], ent->s.number, v8);
                Vec3Lerp(vCenter, tagMat[3], trace.fraction, dropEnt->s.lerp.pos.trBase);
                currentOrigin = dropEnt->r.currentOrigin;
                trBase = dropEnt->s.lerp.pos.trBase;
                dropEnt->r.currentOrigin[0] = dropEnt->s.lerp.pos.trBase[0];
                currentOrigin[1] = trBase[1];
                currentOrigin[2] = trBase[2];
                dropEnt->s.lerp.pos.trTime = level.time;
            }
            G_SetAngle(dropEnt, ent->r.currentAngles);
        }
        return dropEnt;
    }
    else
    {
    LABEL_15:
        BG_TakePlayerWeapon(&ent->client->ps, weapIdx, 1);
        return 0;
    }
}

int32_t __cdecl TransferPlayerAmmoToWeaponEntity(gentity_s *player, gentity_s *weaponEnt, int32_t transferWeapon)
{
    int32_t NonClipAmmoToTransferToWeaponEntity; // eax
    int32_t clipIndex; // [esp+4h] [ebp-14h]
    int32_t i; // [esp+Ch] [ebp-Ch]
    int32_t total; // [esp+10h] [ebp-8h]

    total = 0;
    for (i = 0; i < 2; ++i)
    {
        if (transferWeapon > 0)
        {
            clipIndex = BG_ClipForWeapon(transferWeapon);
            NonClipAmmoToTransferToWeaponEntity = GetNonClipAmmoToTransferToWeaponEntity(player, transferWeapon);
            weaponEnt->item[i].index = transferWeapon;
            weaponEnt->item[i].ammoCount = NonClipAmmoToTransferToWeaponEntity;
            weaponEnt->item[i].clipAmmoCount = player->client->ps.ammoclip[clipIndex];
            total += weaponEnt->item[i].clipAmmoCount + weaponEnt->item[i].ammoCount;
            transferWeapon = BG_GetWeaponDef(transferWeapon)->altWeaponIndex;
        }
    }
    return total;
}

int32_t __cdecl TransferRandomAmmoToWeaponEntity(gentity_s *weaponEnt, int32_t transferWeapon)
{
    int32_t i; // [esp+0h] [ebp-20h]
    int32_t iDropAmmo; // [esp+4h] [ebp-1Ch]
    int32_t iDropAmmoa; // [esp+4h] [ebp-1Ch]
    int32_t iMax; // [esp+8h] [ebp-18h]
    int32_t total; // [esp+Ch] [ebp-14h]
    int32_t iMin; // [esp+10h] [ebp-10h]
    int32_t iDropClip; // [esp+18h] [ebp-8h]
    WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    total = 0;
    for (i = 0; i < 2; ++i)
    {
        if (transferWeapon > 0)
        {
            weapDef = BG_GetWeaponDef(transferWeapon);
            if (weapDef->iSharedAmmoCapIndex >= 0 && i)
                return total;
            iMax = weapDef->iDropAmmoMax;
            iMin = weapDef->iDropAmmoMin;
            if (iMax < iMin)
            {
                iMax = weapDef->iDropAmmoMin;
                iMin = weapDef->iDropAmmoMax;
            }
            if (iMax >= 0)
            {
                if (iMax < iMin)
                    MyAssertHandler(".\\game\\g_items.cpp", 976, 0, "%s", "iMax >= iMin");
                iDropAmmoa = iMin + G_rand() % (iMax - iMin + 1);
                if (iDropAmmoa > 0)
                {
                    if (weapDef->iClipSize < 0)
                        MyAssertHandler(".\\game\\g_items.cpp", 986, 0, "%s", "weapDef->iClipSize >= 0");
                    if (weapDef->iClipSize == 1)
                        iDropClip = 1;
                    else
                        iDropClip = G_rand() % (weapDef->iClipSize + 1);
                    if (iDropClip < iDropAmmoa)
                    {
                        iDropAmmo = iDropAmmoa - iDropClip;
                    }
                    else
                    {
                        iDropClip = iDropAmmoa;
                        iDropAmmo = 0;
                    }
                }
                else
                {
                    iDropAmmo = 0;
                    iDropClip = 0;
                }
            }
            else
            {
                iDropAmmo = 0;
                iDropClip = 0;
            }
            weaponEnt->item[i].index = transferWeapon;
            weaponEnt->item[i].ammoCount = iDropAmmo;
            weaponEnt->item[i].clipAmmoCount = iDropClip;
            transferWeapon = BG_GetWeaponDef(transferWeapon)->altWeaponIndex;
            total += weaponEnt->item[i].clipAmmoCount + weaponEnt->item[i].ammoCount;
        }
    }
    return total;
}

void __cdecl FinishSpawningItem(gentity_s *ent)
{
    float v4; // [esp+10h] [ebp-BCh]
    float v5; // [esp+14h] [ebp-B8h]
    float v6; // [esp+20h] [ebp-ACh]
    float v7; // [esp+24h] [ebp-A8h]
    float vAngles[3]; // [esp+2Ch] [ebp-A0h] BYREF
    float vAxis[3][3]; // [esp+38h] [ebp-94h] BYREF
    gitem_s *item; // [esp+5Ch] [ebp-70h]
    float mins[3]; // [esp+60h] [ebp-6Ch] BYREF
    float start[3]; // [esp+6Ch] [ebp-60h] BYREF
    float maxs[3]; // [esp+78h] [ebp-54h] BYREF
    float endpos[3]; // [esp+84h] [ebp-48h] BYREF
    trace_t tr; // [esp+90h] [ebp-3Ch] BYREF
    float dest[3]; // [esp+BCh] [ebp-10h] BYREF
    int32_t clipMask; // [esp+C8h] [ebp-4h]

    ent->handler = 18;
    if ((ent->spawnflags & 1) != 0)
    {
        G_SetOrigin(ent, ent->r.currentOrigin);
    }
    else
    {
        item = &bg_itemlist[ent->s.index.brushmodel];
        iassert(item->giType == IT_WEAPON);
        mins[0] = -1.0;
        mins[1] = -1.0;
        mins[2] = -1.0;
        maxs[0] = 1.0;
        maxs[1] = 1.0;
        maxs[2] = 1.0;
        clipMask = G_ItemClipMask(ent);
        start[0] = ent->r.currentOrigin[0];
        start[1] = ent->r.currentOrigin[1];
        start[2] = ent->r.currentOrigin[2];
        v6 = ent->r.currentOrigin[1];
        v7 = ent->r.currentOrigin[2] - 4096.0;
        dest[0] = ent->r.currentOrigin[0];
        dest[1] = v6;
        dest[2] = v7;
        G_TraceCapsule(&tr, start, mins, maxs, dest, ent->s.number, clipMask);
        if (tr.startsolid)
        {
            start[0] = ent->r.currentOrigin[0];
            start[1] = ent->r.currentOrigin[1];
            start[2] = ent->r.currentOrigin[2];
            start[2] = start[2] - 15.0;
            v4 = ent->r.currentOrigin[1];
            v5 = ent->r.currentOrigin[2] - 4096.0;
            dest[0] = ent->r.currentOrigin[0];
            dest[1] = v4;
            dest[2] = v5;
            G_TraceCapsule(&tr, start, mins, maxs, dest, ent->s.number, clipMask);
        }
        if (tr.startsolid)
        {
            Com_Printf(15, "FinishSpawningItem: %s startsolid at %s\n", SL_ConvertToString(ent->classname), vtos(ent->r.currentOrigin));
            G_FreeEntity(ent);
            return;
        }
        ent->s.groundEntityNum = Trace_GetEntityHitId(&tr);
        g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
        Vec3Lerp(start, dest, tr.fraction, endpos);
        G_SetOrigin(ent, endpos);
        if (tr.fraction < 1.0)
        {
            vAxis[2][0] = tr.normal[0];
            vAxis[2][1] = tr.normal[1];
            vAxis[2][2] = tr.normal[2];
            AngleVectors(ent->r.currentAngles, vAxis[0], 0, 0);
            Vec3Cross(vAxis[2], vAxis[0], vAxis[1]);
            Vec3Cross(vAxis[1], vAxis[2], vAxis[0]);
            AxisToAngles(vAxis, vAngles);
            if (bg_itemlist[ent->s.index.brushmodel].giType != IT_WEAPON)
                MyAssertHandler(".\\game\\g_items.cpp", 1176, 0, "%s", "bg_itemlist[ent->s.index.item].giType == IT_WEAPON");
            vAngles[2] = vAngles[2] + 90.0;
            G_SetAngle(ent, vAngles);
        }
    }
    SV_LinkEntity(ent);
}

void __cdecl ClearRegisteredItems()
{
    memset((uint8_t *)itemRegistered, 0, sizeof(itemRegistered));
    itemRegistered[0] = 1;
}

#ifdef KISAK_MP
void __cdecl SaveRegisteredWeapons()
{
    char dest[8196]; // [esp+0h] [ebp-2010h] BYREF
    uint32_t weaponIndex; // [esp+2008h] [ebp-8h]
    WeaponDef *WeaponDef; // [esp+200Ch] [ebp-4h]

    level.registerWeapons = 0;
    dest[0] = 0;
    WeaponDef = 0;
    for (weaponIndex = 1; weaponIndex < BG_GetNumWeapons(); ++weaponIndex)
    {
        if (WeaponDef)
            I_strncat(dest, 0x2000, " ");
        WeaponDef = BG_GetWeaponDef(weaponIndex);
        I_strncat(dest, 0x2000, (char *)WeaponDef->szInternalName);
    }
    SV_SetConfigstring(2258, dest);
}
#endif

void __cdecl SaveRegisteredItems()
{
    char string[128]; // [esp+0h] [ebp-98h] BYREF
    int32_t modelIdx; // [esp+84h] [ebp-14h]
    int32_t bits; // [esp+88h] [ebp-10h]
    int32_t weapIdx; // [esp+8Ch] [ebp-Ch]
    int32_t n; // [esp+90h] [ebp-8h]
    int32_t digit; // [esp+94h] [ebp-4h]

    level.bRegisterItems = 0;
    n = 0;
    digit = 0;
    bits = 0;
    for (weapIdx = 0; weapIdx < 128; ++weapIdx)
    {
        for (modelIdx = 0; modelIdx < 16; ++modelIdx)
        {
            if (itemRegistered[128 * modelIdx + weapIdx])
            {
                digit += 1 << bits;
                break;
            }
        }
        if (++bits == 4)
        {
            string[n++] = digit + (digit >= 10 ? 87 : 48);
            digit = 0;
            bits = 0;
        }
    }
    if (bits)
        string[n++] = digit + (digit >= 10 ? 87 : 48);
    string[n] = 0;
    SV_SetConfigstring(2314, string);
}

void __cdecl G_RegisterWeapon(uint32_t weapIndex)
{
    char *Name; // eax
    char *v2; // eax
    uint32_t modelindex; // [esp+0h] [ebp-Ch]
    uint8_t weapModel; // [esp+7h] [ebp-5h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if (itemRegistered[weapIndex])
        MyAssertHandler(".\\game\\g_items.cpp", 1301, 0, "%s", "!itemRegistered[weapIndex]");
    itemRegistered[weapIndex] = 1;
    level.bRegisterItems = 1;
#ifdef KISAK_MP
    level.registerWeapons = 1;
#endif

    weapDef = BG_GetWeaponDef(weapIndex);
    if (*weapDef->szUseHintString
        && !G_GetHintStringIndex(&weapDef->iUseHintStringIndex, (char *)weapDef->szUseHintString))
    {
        Com_Error(ERR_DROP, "Too many different hintstring values on weapons. Max allowed is %i different strings", 32);
    }
    if (*weapDef->dropHintString && !G_GetHintStringIndex(&weapDef->dropHintStringIndex, (char *)weapDef->dropHintString))
        Com_Error(ERR_DROP, "Too many different hintstring values on weapons. Max allowed is %i different strings", 32);
    for (weapModel = 0; weapModel < 0x10u; ++weapModel)
    {
        if (weapDef->worldModel[weapModel])
        {
            Name = (char *)XModelGetName(weapDef->worldModel[weapModel]);
            modelindex = G_ModelIndex(Name);
            if (XModelBad(weapDef->worldModel[weapModel]))
                G_OverrideModel(modelindex, (char*)"defaultweapon");
        }
    }
    if (weapDef->projectileModel)
    {
        v2 = (char *)XModelGetName(weapDef->projectileModel);
        G_ModelIndex(v2);
    }
    //fireRumble = v4->fireRumble;
    //if (fireRumble && *fireRumble)
    //    G_RumbleIndex(fireRumble);
    //meleeImpactRumble = v4->meleeImpactRumble;
    //if (meleeImpactRumble)
    //{
    //    if (*meleeImpactRumble)
    //        G_RumbleIndex(meleeImpactRumble);
    //}
}

int32_t __cdecl IsItemRegistered(uint32_t iItemIndex)
{
    if (iItemIndex >= 0x80)
        MyAssertHandler(".\\game\\g_items.cpp", 1349, 0, "%s", "(iItemIndex >= 0) && (iItemIndex < MAX_WEAPONS)");
    return itemRegistered[iItemIndex];
}

void __cdecl G_SpawnItem(gentity_s *ent, const gitem_s *item)
{
    char *Name; // eax
    uint32_t weapIndex; // [esp+8h] [ebp-Ch]
    uint8_t weapModel; // [esp+Fh] [ebp-5h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    ent->item[0].index = ((char *)item - (char *)bg_itemlist) >> 2;
    weapIndex = ent->item[0].index % 128;
    weapModel = ent->item[0].index / 128;
    weapDef = BG_GetWeaponDef(weapIndex);
    
    iassert(weapDef);

    if (weapDef->worldModel[weapModel])
    {
        Name = (char *)XModelGetName(weapDef->worldModel[weapModel]);
        G_SetModel(ent, Name);
    }

    iassert(item->giType == IT_WEAPON);

    ent->r.mins[0] = -1.0;
    ent->r.mins[1] = -1.0;
    ent->r.mins[2] = -1.0;

    ent->r.maxs[0] = 1.0;
    ent->r.maxs[1] = 1.0;
    ent->r.maxs[2] = 1.0;

    ent->s.eType = ET_ITEM;
#ifdef KISAK_MP
    ent->r.contents = 1079771400;
    ent->r.contents |= 0x200000u;
    ent->s.index.brushmodel = LOWORD(ent->missile.travelDist);
#elif KISAK_SP
    ent->r.contents = 1081868552;
    ent->s.index.item = ent->item->index;
#endif
    TransferRandomAmmoToWeaponEntity(ent, weapIndex);
    G_DObjUpdate(ent);
#ifdef KISAK_MP
    ent->s.clientNum = 64;
    ent->flags |= 0x1000u;
#elif KISAK_SP
    ent->flags |= 0x800;
#endif
    if (level.spawnVar.spawnVarsValid)
    {
        G_SetAngle(ent, ent->r.currentAngles);
        ent->nextthink = level.time + 100;
        ent->handler = 17;
    }
    else
    {
        ent->handler = 18;
        if ((ent->spawnflags & 1) == 0)
        {
            ent->s.groundEntityNum = 1023;
            iassert(item->giType == IT_WEAPON);
            ent->r.currentAngles[2] = ent->r.currentAngles[2] + 90.0;
        }
        G_SetAngle(ent, ent->r.currentAngles);
        G_SetOrigin(ent, ent->r.currentOrigin);
        SV_LinkEntity(ent);
    }
}

void __cdecl G_RunItem(gentity_s *ent)
{
    int32_t passEntityNum; // [esp+10h] [ebp-ACh]
    float diff[6]; // [esp+24h] [ebp-98h] BYREF
    float v4; // [esp+3Ch] [ebp-80h]
    float v5; // [esp+40h] [ebp-7Ch]
    float v6; // [esp+44h] [ebp-78h]
    float v7; // [esp+48h] [ebp-74h]
    float v8; // [esp+4Ch] [ebp-70h]
    float v9; // [esp+50h] [ebp-6Ch]
    float *trDelta; // [esp+54h] [ebp-68h]
    float *trBase; // [esp+58h] [ebp-64h]
    float *currentOrigin; // [esp+5Ch] [ebp-60h]
    int32_t contents; // [esp+60h] [ebp-5Ch]
    float delta[3]; // [esp+64h] [ebp-58h] BYREF
    float origin[3]; // [esp+70h] [ebp-4Ch] BYREF
    float endpos[3]; // [esp+7Ch] [ebp-40h] BYREF
    trace_t tr; // [esp+88h] [ebp-34h] BYREF
    int32_t mask; // [esp+B4h] [ebp-8h]
    float dot; // [esp+B8h] [ebp-4h]

    if (ent->s.eType == 2)
        MyAssertHandler(".\\game\\g_items.cpp", 1465, 0, "%s", "ent->s.eType != ET_PLAYER_CORPSE");
    if ((ent->s.groundEntityNum == 1023 || level.gentities[ent->s.groundEntityNum].s.lerp.pos.trType)
        && ent->s.lerp.pos.trType != TR_GRAVITY
        && ent->s.lerp.pos.trType != TR_RAGDOLL_GRAVITY
        && (ent->spawnflags & 1) == 0)
    {
        ent->s.lerp.pos.trType = TR_GRAVITY;
        ent->s.lerp.pos.trTime = level.time;
        trBase = ent->s.lerp.pos.trBase;
        currentOrigin = ent->r.currentOrigin;
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        trBase[1] = currentOrigin[1];
        trBase[2] = currentOrigin[2];
        trDelta = ent->s.lerp.pos.trDelta;
        ent->s.lerp.pos.trDelta[0] = 0.0;
        trDelta[1] = 0.0;
        trDelta[2] = 0.0;
    }
    v9 = ent->s.lerp.pos.trBase[0];
    if ((LODWORD(v9) & 0x7F800000) == 0x7F800000
        || (v8 = ent->s.lerp.pos.trBase[1], (LODWORD(v8) & 0x7F800000) == 0x7F800000)
        || (v7 = ent->s.lerp.pos.trBase[2], (LODWORD(v7) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            ".\\game\\g_items.cpp",
            1492,
            0,
            "%s",
            "!IS_NAN((ent->s.lerp.pos.trBase)[0]) && !IS_NAN((ent->s.lerp.pos.trBase)[1]) && !IS_NAN((ent->s.lerp.pos.trBase)[2])");
    }
    v6 = ent->s.lerp.pos.trDelta[0];
    if ((LODWORD(v6) & 0x7F800000) == 0x7F800000
        || (v5 = ent->s.lerp.pos.trDelta[1], (LODWORD(v5) & 0x7F800000) == 0x7F800000)
        || (v4 = ent->s.lerp.pos.trDelta[2], (LODWORD(v4) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            ".\\game\\g_items.cpp",
            1493,
            0,
            "%s",
            "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])");
    }
    if (ent->s.lerp.pos.trType && !ent->tagInfo)
    {
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time + 50, origin);
        diff[5] = origin[0];
        if ((LODWORD(origin[0]) & 0x7F800000) == 0x7F800000
            || (diff[4] = origin[1], (LODWORD(origin[1]) & 0x7F800000) == 0x7F800000)
            || (diff[3] = origin[2], (LODWORD(origin[2]) & 0x7F800000) == 0x7F800000))
        {
            MyAssertHandler(
                ".\\game\\g_items.cpp",
                1505,
                0,
                "%s",
                "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
        }
        mask = G_ItemClipMask(ent);
        if ((mask & ent->r.contents) != 0)
            MyAssertHandler(".\\game\\g_items.cpp", 1511, 0, "%s", "!( ent->r.contents & mask )");
        Vec3Sub(origin, ent->r.currentOrigin, diff);
        if (Vec3LengthSq(diff) < 0.1000000014901161)
            origin[2] = origin[2] - 1.0;
        if (ent->r.ownerNum.isDefined())
        {
            passEntityNum = ent->r.ownerNum.entnum();
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passEntityNum, mask);
        }
        else
        {
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 1023, mask);
        }
        if (tr.fraction >= 1.0)
        {
            ent->r.currentOrigin[0] = origin[0];
            ent->r.currentOrigin[1] = origin[1];
            ent->r.currentOrigin[2] = origin[2];
        }
        else
        {
            Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
            if (!tr.startsolid && tr.fraction < 0.009999999776482582 && tr.normal[2] < 0.5)
            {
                Vec3Sub(origin, ent->r.currentOrigin, delta);
                dot = 1.0 - Vec3Dot(delta, tr.normal);
                Vec3Mad(origin, dot, tr.normal, origin);
                if (ent->r.ownerNum.isDefined())
                {
                    G_TraceCapsule(&tr, endpos, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum.entnum(), mask);
                }
                else
                {
                    G_TraceCapsule(&tr, endpos, ent->r.mins, ent->r.maxs, origin, 1023, mask);
                }
                Vec3Lerp(endpos, origin, tr.fraction, endpos);
            }
            ent->s.lerp.pos.trType = TR_LINEAR_STOP;
            ent->s.lerp.pos.trTime = level.time;
            ent->s.lerp.pos.trDuration = 50;
            ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
            ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
            ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
            Vec3Sub(endpos, ent->r.currentOrigin, ent->s.lerp.pos.trDelta);
            Vec3Scale(ent->s.lerp.pos.trDelta, 20.0, ent->s.lerp.pos.trDelta);
            ent->r.currentOrigin[0] = endpos[0];
            ent->r.currentOrigin[1] = endpos[1];
            ent->r.currentOrigin[2] = endpos[2];
        }
        SV_LinkEntity(ent);
        G_RunThink(ent);
        if (ent->r.inuse && tr.fraction < 0.009999999776482582)
        {
            if (tr.normal[2] <= 0.0 || (contents = SV_PointContents(ent->r.currentOrigin, -1, 0x80000000)) != 0)
            {
                G_FreeEntity(ent);
            }
            else
            {
                G_OrientItemToGround(ent, &tr);
                G_SetOrigin(ent, endpos);
                ent->s.groundEntityNum = Trace_GetEntityHitId(&tr);
                g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
                SV_LinkEntity(ent);
            }
        }
    }
    else
    {
        G_RunThink(ent);
    }
}

void __cdecl G_OrientItemToGround(gentity_s *ent, trace_t *trace)
{
    float vAngles[3]; // [esp+8h] [ebp-30h] BYREF
    float vAxis[3][3]; // [esp+14h] [ebp-24h] BYREF

    vAxis[2][0] = trace->normal[0];
    vAxis[2][1] = trace->normal[1];
    vAxis[2][2] = trace->normal[2];
    AngleVectors(ent->r.currentAngles, vAxis[0], 0, 0);
    Vec3Cross(vAxis[2], vAxis[0], vAxis[1]);
    Vec3Cross(vAxis[1], vAxis[2], vAxis[0]);
    AxisToAngles(vAxis, vAngles);
    if (ent->s.index.brushmodel)
    {
        if (bg_itemlist[ent->s.index.brushmodel].giType != IT_WEAPON)
            MyAssertHandler(".\\game\\g_items.cpp", 1443, 0, "%s", "bg_itemlist[ent->s.index.item].giType == IT_WEAPON");
        vAngles[2] = vAngles[2] + 90.0;
    }
    G_SetAngle(ent, vAngles);
}

