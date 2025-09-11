#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"

#include <script/scr_vm.h>
#include <server/sv_game.h>
#include <server/sv_world.h>
#include "g_utils_mp.h"
#include <xanim/dobj.h>

void __cdecl PlayerCmd_giveWeapon(scr_entref_t entref)
{
    int32_t v2; // eax
    int32_t weaponModel; // [esp+0h] [ebp-60h]
    gentity_s *pSelf; // [esp+4h] [ebp-5Ch]
    const char *weaponName; // [esp+8h] [ebp-58h]
    bool hadWeapon; // [esp+Ch] [ebp-54h]
    char svcmd[64]; // [esp+10h] [ebp-50h] BYREF
    int32_t weaponIndex; // [esp+54h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+58h] [ebp-8h]
    playerState_s *ps; // [esp+5Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 111, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    if (!pSelf->client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 117, 0, "%s", "pSelf->client");
    ps = &pSelf->client->ps;
    if (!ps)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    hadWeapon = Com_BitCheckAssert(ps->weapons, weaponIndex, 16);
    if (Scr_GetNumParam() == 2)
    {
        weapDef = BG_GetWeaponDef(weaponIndex);
        weaponModel = Scr_GetInt(1);
        if ((uint32_t)weaponModel >= 0x100)
        {
            LOBYTE(weaponModel) = 0;
            v2 = G_GivePlayerWeapon(&pSelf->client->ps, weaponIndex, 0);
            goto LABEL_20;
        }
        if (!weapDef->gunXModel[weaponModel])
            LOBYTE(weaponModel) = 0;
    }
    else
    {
        LOBYTE(weaponModel) = 0;
    }
    v2 = G_GivePlayerWeapon(&pSelf->client->ps, weaponIndex, weaponModel);
LABEL_20:
    if (v2)
    {
        _snprintf(svcmd, 0x40u, "%c \"%i\"", 74, 1);
        svcmd[63] = 0;
        SV_GameSendServerCommand(pSelf - g_entities, SV_CMD_CAN_IGNORE, svcmd);
        G_InitializeAmmo(pSelf, weaponIndex, weaponModel, hadWeapon);
    }
}

void __cdecl G_InitializeAmmo(gentity_s *pSelf, int32_t weaponIndex, uint8_t weaponModel, int32_t hadWeapon)
{
    gclient_s *client; // [esp+0h] [ebp-14h]
    int32_t numWeapons; // [esp+4h] [ebp-10h]
    int32_t startWeapon; // [esp+8h] [ebp-Ch]
    int32_t ammoGive; // [esp+Ch] [ebp-8h]
    WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    startWeapon = weaponIndex;
    numWeapons = BG_GetNumWeapons();
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (!weapDef)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 77, 0, "%s", "weapDef");
    if (!pSelf)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 79, 0, "%s", "pSelf");
    if (!pSelf->client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 80, 0, "%s", "pSelf->client");
    do
    {
        ammoGive = G_GetNeededStartAmmo(pSelf, weapDef);
        if (ammoGive <= 0)
        {
            if (!hadWeapon)
                Fill_Clip(&pSelf->client->ps, weaponIndex);
        }
        else
        {
            Add_Ammo(pSelf, weaponIndex, weaponModel, ammoGive, hadWeapon == 0);
        }
        weaponIndex = weapDef->altWeaponIndex;
        weapDef = BG_GetWeaponDef(weaponIndex);
        if (!weapDef)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 92, 0, "%s", "weapDef");
        if (--numWeapons < 0)
            MyAssertHandler(
                ".\\game_mp\\g_client_script_cmd_mp.cpp",
                95,
                0,
                "%s\n\t(weapDef->szDisplayName) = %s",
                "(numWeapons >= 0)",
                weapDef->szDisplayName);
        if (!weaponIndex || weaponIndex == startWeapon || numWeapons < 0)
            break;
        client = pSelf->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    } while (Com_BitCheckAssert(client->ps.weapons, weaponIndex, 16));
}

int32_t __cdecl G_GetNeededStartAmmo(gentity_s *pSelf, WeaponDef *weapDef)
{
    WeaponDef *thisWeapDef; // [esp+0h] [ebp-14h]
    int32_t applicableOwnedAmmo; // [esp+8h] [ebp-Ch]
    uint32_t weapIndex; // [esp+Ch] [ebp-8h]
    gclient_s *ps; // [esp+10h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 34, 0, "%s", "pSelf");
    if (!pSelf->client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 35, 0, "%s", "pSelf->client");
    if (!weapDef)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 36, 0, "%s", "weapDef");
    ps = pSelf->client;
    applicableOwnedAmmo = ps->ps.ammo[weapDef->iAmmoIndex];
    for (weapIndex = 0; weapIndex <= bg_lastParsedWeaponIndex; ++weapIndex)
    {
        thisWeapDef = BG_GetWeaponDef(weapIndex);
        if (thisWeapDef->iAmmoIndex == weapDef->iAmmoIndex)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->ps.weapons, weapIndex, 16))
            {
                if (weapDef != thisWeapDef)
                    applicableOwnedAmmo -= thisWeapDef->iStartAmmo - ps->ps.ammoclip[thisWeapDef->iClipIndex];
            }
        }
    }
    if (applicableOwnedAmmo < 0)
        applicableOwnedAmmo = 0;
    return weapDef->iStartAmmo - applicableOwnedAmmo;
}

void __cdecl PlayerCmd_takeWeapon(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]
    int32_t iWeaponIndex; // [esp+4h] [ebp-8h]
    const char *pszWeaponName; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 151, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    BG_TakePlayerWeapon(&pSelf->client->ps, iWeaponIndex, 1);
}

void __cdecl PlayerCmd_takeAllWeapons(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-8h]
    uint32_t weapIndex; // [esp+4h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 164, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->ps.weapon = 0;
    for (weapIndex = 1; weapIndex < BG_GetNumWeapons(); ++weapIndex)
        BG_TakePlayerWeapon(&pSelf->client->ps, weapIndex, 1);
}

void __cdecl PlayerCmd_getCurrentWeapon(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-10h]
    int32_t weapon; // [esp+4h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+8h] [ebp-8h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 192, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (ClientPlaying(pSelf) && (weapon = pSelf->client->ps.weapon, weapon > 0))
    {
        weapDef = BG_GetWeaponDef(weapon);
        Scr_AddString((char *)weapDef->szInternalName);
    }
    else
    {
        Scr_AddString("none");
    }
}

bool __cdecl ClientPlaying(gentity_s *pSelf)
{
    if (!pSelf->client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 179, 0, "%s", "pSelf->client");
    if (pSelf->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            180,
            0,
            "%s",
            "pSelf->client->sess.connected != CON_DISCONNECTED");
    return pSelf->client->sess.sessionState == SESS_STATE_PLAYING;
}

void __cdecl PlayerCmd_getCurrentOffhand(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 218, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (ClientPlaying(pSelf) && pSelf->client->ps.offHandIndex > 0)
    {
        weapDef = BG_GetWeaponDef(pSelf->client->ps.offHandIndex);
        Scr_AddString((char *)weapDef->szInternalName);
    }
    else
    {
        Scr_AddString("none");
    }
}

void __cdecl PlayerCmd_setOffhandSecondaryClass(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+4h] [ebp-8h]
    int32_t sf_text; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 242, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        sf_text = Scr_GetConstString(0);
        if (sf_text == scr_const.flash)
        {
            pSelf->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_FLASH;
        }
        else if (sf_text == scr_const.smoke)
        {
            pSelf->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_SMOKE;
        }
        else
        {
            Scr_Error("Must specify either 'smoke' or 'flash' class to set secondary offhand to.\n");
        }
    }
    else
    {
        Scr_Error("Incorrect number of parameters.\n");
    }
}

void __cdecl PlayerCmd_getOffhandSecondaryClass(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 265, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (pSelf->client->ps.offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH)
    {
        Scr_AddConstString(scr_const.flash);
    }
    else
    {
        if (pSelf->client->ps.offhandSecondary)
            MyAssertHandler(
                ".\\game_mp\\g_client_script_cmd_mp.cpp",
                273,
                0,
                "%s",
                "pSelf->client->ps.offhandSecondary == PLAYER_OFFHAND_SECONDARY_SMOKE");
        Scr_AddConstString(scr_const.smoke);
    }
}

void __cdecl PlayerCmd_hasWeapon(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+0h] [ebp-10h]
    gentity_s *pSelf; // [esp+4h] [ebp-Ch]
    uint32_t iWeaponIndex; // [esp+8h] [ebp-8h]
    const char *pszWeaponName; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 284, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = BG_FindWeaponIndexForName(pszWeaponName);
    if (!iWeaponIndex)
        goto LABEL_13;
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, iWeaponIndex, 16))
        Scr_AddBool(1);
    else
        LABEL_13:
    Scr_AddBool(0);
}

void __cdecl PlayerCmd_switchToWeapon(scr_entref_t entref)
{
    gclient_s *client; // [esp+0h] [ebp-10h]
    gentity_s *pSelf; // [esp+4h] [ebp-Ch]
    uint32_t iWeaponIndex; // [esp+8h] [ebp-8h]
    const char *pszWeaponName; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 301, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    if (!iWeaponIndex)
    {
        Scr_ParamError(0, va("unknown weapon '%s'", pszWeaponName));
    }
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, iWeaponIndex, 16))
    {
        G_SelectWeaponIndex(entref.entnum, iWeaponIndex);
        Scr_AddBool(1);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void __cdecl PlayerCmd_switchToOffhand(scr_entref_t entref)
{
    gclient_s *client; // [esp+0h] [ebp-10h]
    gentity_s *pSelf; // [esp+4h] [ebp-Ch]
    int32_t iWeaponIndex; // [esp+8h] [ebp-8h]
    const char *pszWeaponName; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    if (!iWeaponIndex)
    {
        Scr_ParamError(0, va("unknown weapon '%s'", pszWeaponName));
    }
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, iWeaponIndex, 16))
    {
        G_SetEquippedOffHand(entref.entnum, iWeaponIndex);
        Scr_AddBool(1);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void __cdecl PlayerCmd_giveStartAmmo(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+0h] [ebp-10h]
    gentity_s *pSelf; // [esp+4h] [ebp-Ch]
    uint32_t iWeaponIndex; // [esp+8h] [ebp-8h]
    const char *pszWeaponName; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, iWeaponIndex, 16))
        G_InitializeAmmo(pSelf, iWeaponIndex, pSelf->client->ps.weaponmodels[iWeaponIndex], 0);
}

void __cdecl PlayerCmd_giveMaxAmmo(scr_entref_t entref)
{
    gclient_s *client; // [esp+0h] [ebp-1Ch]
    gentity_s *pSelf; // [esp+4h] [ebp-18h]
    const char *weaponName; // [esp+8h] [ebp-14h]
    int32_t maxWeaponAmmo; // [esp+Ch] [ebp-10h]
    int32_t weaponIndex; // [esp+10h] [ebp-Ch]
    int32_t ammoGive; // [esp+14h] [ebp-8h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    weaponName = Scr_GetString(0);
    weaponIndex = G_GetWeaponIndexForName(weaponName);
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, weaponIndex, 16))
    {
        maxWeaponAmmo = BG_GetAmmoPlayerMax(&pSelf->client->ps, weaponIndex, 0);
        ammoGive = maxWeaponAmmo - pSelf->client->ps.ammo[BG_GetWeaponDef(weaponIndex)->iAmmoIndex];
        if (ammoGive > 0)
            Add_Ammo(pSelf, weaponIndex, pSelf->client->ps.weaponmodels[weaponIndex], ammoGive, 0);
    }
}

void __cdecl PlayerCmd_getFractionStartAmmo(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+4h] [ebp-18h]
    gentity_s *pSelf; // [esp+8h] [ebp-14h]
    int32_t iWeaponIndex; // [esp+Ch] [ebp-10h]
    const char *pszWeaponName; // [esp+10h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+14h] [ebp-8h]
    float fAmmoFrac; // [esp+18h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, iWeaponIndex, 16)
        && (weapDef = BG_GetWeaponDef(iWeaponIndex), weapDef->iStartAmmo >= 1))
    {
        if (pSelf->client->ps.ammo[weapDef->iAmmoIndex] >= 1)
        {
            fAmmoFrac = (double)pSelf->client->ps.ammo[weapDef->iAmmoIndex] / (double)weapDef->iStartAmmo;
            Scr_AddFloat(fAmmoFrac);
        }
        else
        {
            Scr_AddFloat(0.0f);
        }
    }
    else
    {
        Scr_AddFloat(1.0f);
    }
}

void __cdecl PlayerCmd_getFractionMaxAmmo(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+4h] [ebp-18h]
    gentity_s *pSelf; // [esp+8h] [ebp-14h]
    int32_t iWeaponIndex; // [esp+Ch] [ebp-10h]
    const char *pszWeaponName; // [esp+10h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+14h] [ebp-8h]
    float fAmmoFrac; // [esp+18h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    client = pSelf->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, iWeaponIndex, 16)
        && (weapDef = BG_GetWeaponDef(iWeaponIndex), weapDef->iMaxAmmo >= 1))
    {
        if (pSelf->client->ps.ammo[weapDef->iAmmoIndex] >= 1)
        {
            fAmmoFrac = (double)pSelf->client->ps.ammo[weapDef->iAmmoIndex] / (double)weapDef->iMaxAmmo;
            Scr_AddFloat(fAmmoFrac);
        }
        else
        {
            Scr_AddFloat(0.0f);
        }
    }
    else
    {
        Scr_AddFloat(1.0f);
    }
}

void __cdecl PlayerCmd_setOrigin(scr_entref_t entref)
{
    const char *v1; // eax
    float *v2; // [esp+4h] [ebp-18h]
    float *origin; // [esp+8h] [ebp-14h]
    gentity_s *pSelf; // [esp+Ch] [ebp-10h]
    float vNewOrigin[3]; // [esp+10h] [ebp-Ch] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 452, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, vNewOrigin);
    SV_UnlinkEntity(pSelf);
    origin = pSelf->client->ps.origin;
    *origin = vNewOrigin[0];
    origin[1] = vNewOrigin[1];
    origin[2] = vNewOrigin[2];
    pSelf->client->ps.origin[2] = pSelf->client->ps.origin[2] + 1.0;
    pSelf->client->ps.eFlags ^= 2u;
    BG_PlayerStateToEntityState(&pSelf->client->ps, &pSelf->s, 1, 1u);
    v2 = pSelf->client->ps.origin;
    pSelf->r.currentOrigin[0] = *v2;
    pSelf->r.currentOrigin[1] = v2[1];
    pSelf->r.currentOrigin[2] = v2[2];
    SV_LinkEntity(pSelf);
}

void __cdecl PlayerCmd_GetVelocity(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 476, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_AddVector(pSelf->client->ps.velocity);
}

void __cdecl PlayerCmd_setAngles(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-10h]
    float angles[3]; // [esp+4h] [ebp-Ch] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 486, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, angles);
    SetClientViewAngle(pSelf, angles);
    if (!pSelf->tagInfo)
        pSelf->r.currentAngles[0] = 0.0;
}

void __cdecl PlayerCmd_getAngles(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 498, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_AddVector(pSelf->client->ps.viewangles);
}

void __cdecl PlayerCmd_useButtonPressed(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 516, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (((pSelf->client->buttons | pSelf->client->buttonsSinceLastFrame) & 0x28) != 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_attackButtonPressed(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 537, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (((pSelf->client->buttons | pSelf->client->buttonsSinceLastFrame) & 1) != 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_adsButtonPressed(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 558, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (((pSelf->client->buttons | pSelf->client->buttonsSinceLastFrame) & 0x800) != 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_meleeButtonPressed(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 579, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (((pSelf->client->buttons | pSelf->client->buttonsSinceLastFrame) & 4) != 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_fragButtonPressed(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 600, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (((pSelf->client->buttons | pSelf->client->buttonsSinceLastFrame) & 0x4000) != 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_secondaryOffhandButtonPressed(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 621, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (((pSelf->client->buttons | pSelf->client->buttonsSinceLastFrame) & 0x8000) != 0)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_buttonPressedDEVONLY(scr_entref_t entref)
{
    Scr_AddInt(0);
}

void __cdecl PlayerCmd_playerADS(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+4h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 674, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_AddFloat(pSelf->client->ps.fWeaponPosFrac);
}

void __cdecl PlayerCmd_isOnGround(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 682, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if ((pSelf->client->ps.eFlags & 0x300) != 0 || pSelf->client->ps.groundEntityNum != ENTITYNUM_NONE)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl PlayerCmd_pingPlayer(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 710, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->ps.eFlags |= 0x400000u;
    pSelf->client->compassPingTime = level.time + 3000;
}

void __cdecl PlayerCmd_SetViewmodel(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]
    const char *modelName; // [esp+4h] [ebp-8h]
    int32_t modelIndex; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    modelName = Scr_GetString(0);
    if (!modelName || !*modelName)
        Scr_ParamError(0, "usage: setviewmodel(<model name>)");
    modelIndex = G_ModelIndex((char*)modelName);
    if (modelIndex != (uint16_t)modelIndex)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            730,
            0,
            "%s",
            "modelIndex == (modelNameIndex_t) modelIndex");
    pSelf->client->sess.viewmodelIndex = modelIndex;
}

void __cdecl PlayerCmd_GetViewmodel(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-8h]
    uint32_t modelName; // [esp+4h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 750, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    modelName = G_ModelName(pSelf->client->sess.viewmodelIndex);
    Scr_AddConstString(modelName);
}

void __cdecl PlayerCmd_showScoreboard(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 768, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Cmd_Score_f(pSelf);
}

void __cdecl PlayerCmd_setSpawnWeapon(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]
    int32_t iWeaponIndex; // [esp+4h] [ebp-8h]
    const char *pszWeaponName; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszWeaponName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszWeaponName);
    if (BG_IsWeaponValid(&pSelf->client->ps, iWeaponIndex))
    {
        pSelf->client->ps.weapon = iWeaponIndex;
        pSelf->client->ps.weaponstate = 0;
        Com_BitSetAssert(pSelf->client->ps.weaponold, iWeaponIndex, 16);
        G_SelectWeaponIndex(entref.entnum, iWeaponIndex);
    }
}

void __cdecl PlayerCmd_dropItem(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *v2; // eax
    gentity_s *pSelf; // [esp+8h] [ebp-18h]
    uint32_t iWeaponIndex; // [esp+Ch] [ebp-14h]
    const gitem_s *pItem; // [esp+10h] [ebp-10h]
    uint32_t dropTag; // [esp+14h] [ebp-Ch]
    gentity_s *pDroppedItem; // [esp+18h] [ebp-8h]
    const char *pszItemName; // [esp+1Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pszItemName = Scr_GetString(0);
    iWeaponIndex = G_GetWeaponIndexForName(pszItemName);
    if (iWeaponIndex)
    {
        if (Scr_GetNumParam() <= 1)
        {
            v2 = Drop_Weapon(pSelf, iWeaponIndex, pSelf->client->ps.weaponmodels[iWeaponIndex], scr_const.tag_weapon_right);
        }
        else
        {
            dropTag = Scr_GetConstLowercaseString(1);
            v2 = Drop_Weapon(pSelf, iWeaponIndex, pSelf->client->ps.weaponmodels[iWeaponIndex], dropTag);
        }
        GScr_AddEntity(v2);
    }
    else
    {
        pItem = G_FindItem(pszItemName, 0);
        if (pItem)
        {
            pDroppedItem = Drop_Item(pSelf, pItem, 0.0, 0);
            GScr_AddEntity(pDroppedItem);
        }
        else
        {
            GScr_AddEntity(0);
        }
    }
}

void __cdecl PlayerCmd_finishPlayerDamage(scr_entref_t entref)
{
    uint16_t floatValue; // ax
    uint8_t v4; // al
    WeaponDef *WeaponDef; // eax
    float scale; // [esp+10h] [ebp-A4h]
    float *v7; // [esp+18h] [ebp-9Ch]
    float *damage_from; // [esp+20h] [ebp-94h]
    gentity_s *tent; // [esp+24h] [ebp-90h]
    gentity_s *attacker; // [esp+28h] [ebp-8Ch]
    int32_t damage; // [esp+2Ch] [ebp-88h]
    meansOfDeath_t mod; // [esp+30h] [ebp-84h]
    float damage_time; // [esp+34h] [ebp-80h]
    gentity_s *pSelf; // [esp+38h] [ebp-7Ch]
    int32_t knockback; // [esp+3Ch] [ebp-78h]
    int32_t t; // [esp+40h] [ebp-74h]
    float *dir; // [esp+44h] [ebp-70h]
    float localdir[3]; // [esp+48h] [ebp-6Ch] BYREF
    float vDir[3]; // [esp+54h] [ebp-60h] BYREF
    float knockbackMod; // [esp+60h] [ebp-54h]
    float vPoint[3]; // [esp+64h] [ebp-50h] BYREF
    float player_yaw; // [esp+70h] [ebp-44h]
    gentity_s *tempBulletHitEntity; // [esp+74h] [ebp-40h]
    int32_t iWeapon; // [esp+78h] [ebp-3Ch]
    int32_t psTimeOffset; // [esp+7Ch] [ebp-38h]
    int32_t dflags; // [esp+80h] [ebp-34h]
    float mass; // [esp+84h] [ebp-30h]
    float flinchYawDir; // [esp+88h] [ebp-2Ch]
    gentity_s *inflictor; // [esp+8Ch] [ebp-28h]
    hitLocation_t hitLoc; // [esp+90h] [ebp-24h]
    void(__cdecl * die)(gentity_s *, gentity_s *, gentity_s *, int, int, const int, const float *, const hitLocation_t, int); // [esp+94h] [ebp-20h]
    void(__cdecl * pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int); // [esp+98h] [ebp-1Ch]
    float kvel[3]; // [esp+9Ch] [ebp-18h] BYREF
    float max_damage_time; // [esp+A8h] [ebp-Ch]
    const float *point; // [esp+ACh] [ebp-8h]
    float time_per_point; // [esp+B0h] [ebp-4h]

    inflictor = &g_entities[ENTITYNUM_WORLD];
    attacker = &g_entities[ENTITYNUM_WORLD];
    dir = 0;
    point = 0;
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    tempBulletHitEntity = 0;
    if (!pSelf->client->lastStand || pSelf->client->lastStandTime <= level.time)
    {
        damage = Scr_GetInt(2);
        if (damage > 0)
        {
            if (Scr_GetType(0) && Scr_GetPointerType(0) == 20)
                inflictor = Scr_GetEntity(0);
            if (Scr_GetType(1) && Scr_GetPointerType(1) == 20)
                attacker = Scr_GetEntity(1);
            dflags = Scr_GetInt(3);
            mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(4);
            iWeapon = G_GetWeaponIndexForName(Scr_GetString(5));
            if (Scr_GetType(6))
            {
                Scr_GetVector(6u, vPoint);
                point = vPoint;
            }
            if (Scr_GetType(7))
            {
                Scr_GetVector(7u, vDir);
                dir = vDir;
            }
            floatValue = Scr_GetConstString(8);
            hitLoc = (hitLocation_t)G_GetHitLocationIndexFromString(floatValue);
            psTimeOffset = Scr_GetInt(9);
            if (pSelf->client->ps.pm_type == PM_DEAD)
            {
                Scr_Error("Trying to do damage to a client that is already dead");
                return;
            }
            if (dir)
            {
                Vec3NormalizeTo(dir, localdir);
            }
            else
            {
                localdir[0] = 0.0f;
                localdir[1] = 0.0f;
                localdir[2] = 0.0f;
            }
            if ((pSelf->flags & 8) == 0 && (dflags & 4) == 0)
            {
                knockbackMod = 0.30000001f;
                if ((pSelf->client->ps.pm_flags & PMF_PRONE) != 0)
                {
                    knockbackMod = 0.02f;
                }
                else if ((pSelf->client->ps.pm_flags & 2) != 0)
                {
                    knockbackMod = 0.15000001f;
                }
                knockback = (int)((float)damage * knockbackMod);
                if (knockback > 60)
                    knockback = 60;
                if (knockback)
                {
                    if ((pSelf->client->ps.eFlags & 0x300) == 0)
                    {
                        mass = 250.0f;
                        scale = (float)knockback * g_knockback->current.value / 250.0f;
                        Vec3Scale(localdir, scale, kvel);
                        Vec3Add(pSelf->client->ps.velocity, kvel, pSelf->client->ps.velocity);
                        if (!pSelf->client->ps.pm_time)
                        {
                            t = 2 * knockback;
                            if (2 * knockback < 50)
                                t = 50;
                            if (t > 200)
                                t = 200;
                            pSelf->client->ps.pm_time = t;
                            pSelf->client->ps.pm_flags |= 0x100u;
                        }
                    }
                }
            }
            if ((pSelf->flags & 1) == 0)
            {
                if (iWeapon && BG_GetWeaponDef(iWeapon)->weapType == WEAPTYPE_BULLET && IsBulletImpactMOD(mod))
                {
                    tempBulletHitEntity = G_TempEntity(vPoint, 41);
                    tempBulletHitEntity->s.weapon = iWeapon;
                    v4 = DirToByte(localdir);
                    tempBulletHitEntity->s.eventParm = v4;
                    tempBulletHitEntity->s.un1.scale = hitLoc == HITLOC_HEAD;
                    tempBulletHitEntity->s.surfType = 7;
                    tempBulletHitEntity->s.otherEntityNum = attacker->s.number;
                    if (tempBulletHitEntity->r.clientMask[0])
                        MyAssertHandler(
                            ".\\game_mp\\g_client_script_cmd_mp.cpp",
                            1002,
                            0,
                            "%s",
                            "tempBulletHitEntity->r.clientMask[ 0 ] == 0");
                    if (tempBulletHitEntity->r.clientMask[1])
                        MyAssertHandler(
                            ".\\game_mp\\g_client_script_cmd_mp.cpp",
                            1003,
                            0,
                            "%s",
                            "tempBulletHitEntity->r.clientMask[ 1 ] == 0");
                    tempBulletHitEntity->r.clientMask[pSelf->client->ps.clientNum >> 5] |= 1 << (pSelf->client->ps.clientNum & 0x1F);
                    WeaponDef = BG_GetWeaponDef(iWeapon);
                    tent = G_TempEntity(vPoint, (WeaponDef->bRifleBullet != 0) + 42);
                    tent->s.weapon = iWeapon;
                    tent->s.surfType = 7;
                    tent->s.otherEntityNum = attacker->s.number;
                    tent->s.clientNum = pSelf->client->ps.clientNum;
                    tent->r.clientMask[0] = -1;
                    tent->r.clientMask[1] = -1;
                    tent->r.clientMask[pSelf->client->ps.clientNum >> 5] &= ~(1 << (pSelf->client->ps.clientNum & 0x1F));
                }
                pSelf->client->damage_blood += damage;
                if (dir)
                {
                    damage_from = pSelf->client->damage_from;
                    *damage_from = localdir[0];
                    damage_from[1] = localdir[1];
                    damage_from[2] = localdir[2];
                    pSelf->client->damage_fromWorld = 0;
                }
                else
                {
                    v7 = pSelf->client->damage_from;
                    *v7 = pSelf->r.currentOrigin[0];
                    v7[1] = pSelf->r.currentOrigin[1];
                    v7[2] = pSelf->r.currentOrigin[2];
                    pSelf->client->damage_fromWorld = 1;
                }
                if ((pSelf->flags & 2) != 0 && pSelf->health - damage <= 0)
                    damage = pSelf->health - 1;
                time_per_point = player_dmgtimer_timePerPoint->current.value;
                max_damage_time = player_dmgtimer_maxTime->current.value;
                damage_time = (double)damage * time_per_point;
                pSelf->client->ps.damageTimer += (int)damage_time;
                if (dir)
                {
                    flinchYawDir = vectoyaw(dir);
                    player_yaw = pSelf->client->ps.viewangles[1];
                    if (player_yaw < 0.0)
                        player_yaw = player_yaw + 360.0;
                    flinchYawDir = flinchYawDir - (double)(int)player_yaw;
                    if (flinchYawDir < 0.0)
                        flinchYawDir = flinchYawDir + 360.0;
                }
                else
                {
                    flinchYawDir = 0.0;
                }
                if (flinchYawDir < 315.0 && flinchYawDir >= 45.0)
                {
                    if (flinchYawDir < 135.0 || flinchYawDir >= 225.0)
                    {
                        if (flinchYawDir < 45.0 || flinchYawDir >= 135.0)
                            pSelf->client->ps.flinchYawAnim = 3;
                        else
                            pSelf->client->ps.flinchYawAnim = 2;
                    }
                    else
                    {
                        pSelf->client->ps.flinchYawAnim = 1;
                    }
                }
                else
                {
                    pSelf->client->ps.flinchYawAnim = 0;
                }
                if (max_damage_time < (double)pSelf->client->ps.damageTimer)
                    pSelf->client->ps.damageTimer = (int)max_damage_time;
                pSelf->client->ps.damageDuration = pSelf->client->ps.damageTimer;
                pSelf->health -= damage;
                Scr_AddEntity(attacker);
                Scr_AddInt(damage);
                Scr_Notify(pSelf, scr_const.damage, 2u);
                if (!entityHandlers[pSelf->handler].die)
                    Com_Printf(1, "No die handler for player entity type %i", pSelf->handler);
                if (pSelf->health > 0)
                {
                    pain = entityHandlers[pSelf->handler].pain;
                    if (pain)
                        pain(pSelf, attacker, damage, point, mod, localdir, hitLoc, iWeapon);
                    goto LABEL_93;
                }
                if (!pSelf->client->lastStand && (pSelf->client->ps.perks & 0x80) != 0)
                {
                    pSelf->client->lastStand = 1;
                    pSelf->client->lastStandTime = level.time + 500;
                    Scr_PlayerLastStand(pSelf, inflictor, attacker, damage, mod, iWeapon, localdir, hitLoc, psTimeOffset);
                LABEL_93:
                    if (!pSelf->r.inuse)
                        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1166, 0, "%s", "pSelf->r.inuse");
                    pSelf->client->ps.stats[0] = pSelf->health;
                    return;
                }
                if (tempBulletHitEntity)
                    tempBulletHitEntity->s.un1.scale |= 2u;
                if (pSelf->health <= 0xFFFFFC18)
                    pSelf->health = -999;
                die = entityHandlers[pSelf->handler].die;
                if (die)
                    die(pSelf, inflictor, attacker, damage, mod, iWeapon, localdir, hitLoc, psTimeOffset);
                if (pSelf->r.inuse)
                    goto LABEL_93;
            }
        }
    }
}

bool __cdecl IsBulletImpactMOD(meansOfDeath_t mod)
{
    if ((uint32_t)mod >= MOD_NUM)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\game_mp\\../bgame/bg_public.h",
            961,
            0,
            "mod doesn't index MOD_NUM\n\t%i not in [0, %i)",
            mod,
            16);
    return mod == MOD_PISTOL_BULLET || mod == MOD_RIFLE_BULLET || mod == MOD_HEAD_SHOT;
}

void __cdecl PlayerCmd_Suicide(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1184, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->flags &= 0xFFFFFFFC;
    pSelf->health = 0;
    pSelf->client->ps.stats[0] = 0;
    player_die(pSelf, pSelf, pSelf, 100000, 12, 0, 0, HITLOC_NONE, 0);
}

void __cdecl PlayerCmd_OpenMenu(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-50h]
    uint32_t iMenuIndex; // [esp+4h] [ebp-4Ch]
    char svcmd[68]; // [esp+8h] [ebp-48h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    if (pSelf->client->sess.connected == CON_CONNECTED)
    {
        iMenuIndex = GScr_GetScriptMenuIndex(Scr_GetString(0));
        if (iMenuIndex >= 0x20)
            MyAssertHandler(
                ".\\game_mp\\g_client_script_cmd_mp.cpp",
                1203,
                0,
                "%s",
                "(iMenuIndex >= 0) && (iMenuIndex < MAX_SCRIPT_MENUS)");
        _snprintf(svcmd, 0x40u, "%c %i", 116, iMenuIndex);
        svcmd[63] = 0;
        SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, svcmd);
        Scr_AddInt(1);
    }
    else
    {
        Scr_AddInt(0);
    }
}

void __cdecl PlayerCmd_OpenMenuNoMouse(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-50h]
    uint32_t iMenuIndex; // [esp+4h] [ebp-4Ch]
    char svcmd[68]; // [esp+8h] [ebp-48h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    if (pSelf->client->sess.connected == CON_CONNECTED)
    {
        iMenuIndex = GScr_GetScriptMenuIndex(Scr_GetString(0));
        if (iMenuIndex >= 0x20)
            MyAssertHandler(
                ".\\game_mp\\g_client_script_cmd_mp.cpp",
                1227,
                0,
                "%s",
                "(iMenuIndex >= 0) && (iMenuIndex < MAX_SCRIPT_MENUS)");
        _snprintf(svcmd, 0x40u, "%c %i 1", 116, iMenuIndex);
        svcmd[63] = 0;
        SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, svcmd);
        Scr_AddInt(1);
    }
    else
    {
        Scr_AddInt(0);
    }
}

void __cdecl PlayerCmd_CloseMenu(scr_entref_t entref)
{
    const char *v1; // eax
    char svcmd[68]; // [esp+4h] [ebp-48h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1245, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    _snprintf(svcmd, 0x40u, "%c", 117);
    svcmd[63] = 0;
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, svcmd);
}

void __cdecl PlayerCmd_CloseInGameMenu(scr_entref_t entref)
{
    const char *v1; // eax
    char svcmd[32]; // [esp+4h] [ebp-24h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1268, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    _snprintf(svcmd, 0x20u, "%c", 76);
    svcmd[31] = 0;
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, svcmd);
}

void __cdecl PlayerCmd_SetWeaponAmmoClip(scr_entref_t entref)
{
    const char *v1; // eax
    int32_t ammoCount; // [esp+0h] [ebp-18h]
    gentity_s *pSelf; // [esp+4h] [ebp-14h]
    const char *weapName; // [esp+8h] [ebp-10h]
    int32_t clipIndex; // [esp+Ch] [ebp-Ch]
    int32_t weapIndex; // [esp+10h] [ebp-8h]
    WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    weapName = Scr_GetString(0);
    ammoCount = Scr_GetInt(1);
    weapIndex = G_GetWeaponIndexForName(weapName);
    if (weapIndex)
    {
        clipIndex = BG_ClipForWeapon(weapIndex);
        if (clipIndex)
        {
            weapDef = BG_GetWeaponDef(weapIndex);
            if (ammoCount >= 0)
            {
                if (ammoCount > weapDef->iClipSize)
                    ammoCount = weapDef->iClipSize;
            }
            else
            {
                ammoCount = 0;
            }
            pSelf->client->ps.ammoclip[clipIndex] = ammoCount;
        }
    }
    else
    {
        Scr_AddInt(0);
    }
}

void __cdecl PlayerCmd_SetWeaponAmmoStock(scr_entref_t entref)
{
    VariableUnion v2; // [esp+0h] [ebp-38h]
    VariableUnion v3; // [esp+4h] [ebp-34h]
    VariableUnion v4; // [esp+8h] [ebp-30h]
    VariableUnion v5; // [esp+10h] [ebp-28h]
    int32_t ammoIdx; // [esp+14h] [ebp-24h]
    int32_t maxAmmo; // [esp+18h] [ebp-20h]
    int32_t clipIdx; // [esp+1Ch] [ebp-1Ch]
    gentity_s *pSelf; // [esp+20h] [ebp-18h]
    const char *weapName; // [esp+24h] [ebp-14h]
    int32_t weapIdx; // [esp+28h] [ebp-10h]
    playerState_s *ps; // [esp+2Ch] [ebp-Ch]
    WeaponDef *weapDef; // [esp+30h] [ebp-8h]
    int32_t newAmmoCnt; // [esp+34h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    weapName = Scr_GetString(0);
    newAmmoCnt = Scr_GetInt(1);
    weapIdx = G_GetWeaponIndexForName(weapName);
    ps = &pSelf->client->ps;
    if (weapIdx)
    {
        weapDef = BG_GetWeaponDef(weapIdx);
        if (BG_WeaponIsClipOnly(weapIdx))
        {
            clipIdx = BG_ClipForWeapon(weapIdx);
            if (clipIdx)
            {
                if (newAmmoCnt < weapDef->iClipSize)
                    v5.intValue = newAmmoCnt;
                else
                    v5.intValue = weapDef->iClipSize;
                if (v5.intValue > 0)
                    v3.intValue = v5.intValue;
                else
                    v3.intValue = 0;
                ps->ammoclip[clipIdx] = v3.intValue;
            }
        }
        else
        {
            ammoIdx = BG_AmmoForWeapon(weapIdx);
            if (ammoIdx)
            {
                maxAmmo = BG_GetAmmoPlayerMax(ps, weapIdx, 0);
                if (newAmmoCnt < maxAmmo)
                    v4.intValue = newAmmoCnt;
                else
                    v4.intValue = maxAmmo;
                if (v4.intValue > 0)
                    v2.intValue = v4.intValue;
                else
                    v2.intValue = 0;
                ps->ammo[ammoIdx] = v2.intValue;
            }
        }
    }
}

void __cdecl PlayerCmd_GetWeaponAmmoClip(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-10h]
    const char *weapName; // [esp+4h] [ebp-Ch]
    int32_t weapIdx; // [esp+8h] [ebp-8h]
    int32_t clipIdx; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    weapName = Scr_GetString(0);
    weapIdx = G_GetWeaponIndexForName(weapName);
    if (weapIdx)
    {
        clipIdx = BG_ClipForWeapon(weapIdx);
        Scr_AddInt(pSelf->client->ps.ammoclip[clipIdx]);
    }
    else
    {
        Scr_AddInt(0);
    }
}

void __cdecl PlayerCmd_GetWeaponAmmoStock(scr_entref_t entref)
{
    int32_t ammoIdx; // [esp+0h] [ebp-14h]
    int32_t clipIdx; // [esp+4h] [ebp-10h]
    gentity_s *pSelf; // [esp+8h] [ebp-Ch]
    const char *weapName; // [esp+Ch] [ebp-8h]
    int32_t weapIdx; // [esp+10h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    weapName = Scr_GetString(0);
    weapIdx = G_GetWeaponIndexForName(weapName);
    if (weapIdx)
    {
        if (BG_WeaponIsClipOnly(weapIdx))
        {
            clipIdx = BG_ClipForWeapon(weapIdx);
            Scr_AddInt(pSelf->client->ps.ammoclip[clipIdx]);
        }
        else
        {
            ammoIdx = BG_AmmoForWeapon(weapIdx);
            Scr_AddInt(pSelf->client->ps.ammo[ammoIdx]);
        }
    }
    else
    {
        Scr_AddInt(0);
    }
}

void __cdecl PlayerCmd_AnyAmmoForWeaponModes(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-14h]
    const char *weapName; // [esp+4h] [ebp-10h]
    int32_t weapIdx; // [esp+8h] [ebp-Ch]
    uint32_t altWeapIdx; // [esp+Ch] [ebp-8h]
    int32_t totalAmmo; // [esp+10h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    weapName = Scr_GetString(0);
    weapIdx = G_GetWeaponIndexForName(weapName);
    Scr_VerifyWeaponIndex(weapIdx, weapName);
    totalAmmo = BG_WeaponAmmo(&pSelf->client->ps, weapIdx);
    altWeapIdx = BG_GetWeaponDef(weapIdx)->altWeaponIndex;
    if (altWeapIdx)
        totalAmmo += BG_WeaponAmmo(&pSelf->client->ps, altWeapIdx);
    if (totalAmmo)
        Scr_AddInt(1);
    else
        Scr_AddInt(0);
}

void __cdecl iclientprintln(scr_entref_t entref)
{
    const char *v1; // eax
    char svcmd[32]; // [esp+4h] [ebp-24h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1435, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    _snprintf(svcmd, 0x20u, "%c", 102);
    svcmd[31] = 0;
    Scr_MakeGameMessage(entref.entnum, svcmd);
}

void __cdecl iclientprintlnbold(scr_entref_t entref)
{
    const char *v1; // eax
    char svcmd[32]; // [esp+4h] [ebp-24h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1448, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    _snprintf(svcmd, 0x20u, "%c", 103);
    svcmd[31] = 0;
    Scr_MakeGameMessage(entref.entnum, svcmd);
}

void __cdecl PlayerCmd_spawn(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-1Ch]
    float spawn_angles[3]; // [esp+4h] [ebp-18h] BYREF
    float spawn_origin[3]; // [esp+10h] [ebp-Ch] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1462, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, spawn_origin);
    Scr_GetVector(1u, spawn_angles);
    ClientSpawn(pSelf, spawn_origin, spawn_angles);
}

void __cdecl PlayerCmd_setEnterTime(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1472, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->sess.enterTime = Scr_GetInt(0);
}

void __cdecl BodyEnd(gentity_s *ent)
{
    ent->s.lerp.eFlags &= ~0x80000u;
    ent->r.contents = 0x4000000;
    ent->r.svFlags = 0;
}

void __cdecl PlayerCmd_ClonePlayer(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+48h] [ebp-24h]
    gentity_s *pSelf; // [esp+4Ch] [ebp-20h]
    const DObj_s *dobj; // [esp+50h] [ebp-1Ch]
    XAnimTree_s *tree; // [esp+54h] [ebp-18h]
    gentity_s *body; // [esp+58h] [ebp-14h]
    int32_t deathAnimDuration; // [esp+5Ch] [ebp-10h]
    corpseInfo_t *corpseInfo; // [esp+64h] [ebp-8h]
    int32_t axis; // [esp+68h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1510, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    deathAnimDuration = Scr_GetInt(0);
    client = pSelf->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1515, 0, "%s", "client");
    if (client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            1516,
            0,
            "%s",
            "client->sess.connected != CON_DISCONNECTED");
    body = G_SpawnPlayerClone();
    body->s.clientNum = client->ps.clientNum;
    body->s.lerp.eFlags = body->s.lerp.eFlags & 2 | client->ps.eFlags & 0xFFFFFFFD | 0xA0000;
    G_SetOrigin(body, client->ps.origin);
    G_SetAngle(body, pSelf->r.currentAngles);
    body->s.lerp.pos.trType = TR_GRAVITY;
    body->s.lerp.pos.trTime = level.time;
    body->s.lerp.pos.trDelta[0] = client->ps.velocity[0];
    body->s.lerp.pos.trDelta[1] = client->ps.velocity[1];
    body->s.lerp.pos.trDelta[2] = client->ps.velocity[2];
    if ((COERCE_UNSIGNED_INT(body->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(body->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(body->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            1529,
            0,
            "%s",
            "!IS_NAN((body->s.lerp.pos.trDelta)[0]) && !IS_NAN((body->s.lerp.pos.trDelta)[1]) && !IS_NAN((body->s.lerp.pos.trDelta)[2])");
    }
    body->s.eType = ET_PLAYER_CORPSE;
    body->physicsObject = 1;
    dobj = Com_GetServerDObj(client->ps.clientNum);
    tree = DObjGetTree(dobj);
    for (axis = 0; axis < 2; ++axis)
    {
        if (g_clonePlayerMaxVelocity->current.value < (double)body->s.lerp.pos.trDelta[axis])
            body->s.lerp.pos.trDelta[axis] = g_clonePlayerMaxVelocity->current.value;
    }
    if ((COERCE_UNSIGNED_INT(body->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(body->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(body->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            1543,
            0,
            "%s",
            "!IS_NAN((body->s.lerp.pos.trDelta)[0]) && !IS_NAN((body->s.lerp.pos.trDelta)[1]) && !IS_NAN((body->s.lerp.pos.trDelta)[2])");
    }
    body->item[0].ammoCount = level.time;
    corpseInfo = &g_scr_data.playerCorpseInfo[G_GetFreePlayerCorpseIndex()];
    corpseInfo->entnum = body->s.number;
    corpseInfo->time = level.time;
    corpseInfo->falling = 1;
    if (client->ps.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            1551,
            0,
            "client->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client->ps.clientNum,
            64);
    memcpy(&corpseInfo->ci, &level_bgs.clientinfo[client->ps.clientNum], sizeof(corpseInfo->ci));
    corpseInfo->ci.pXAnimTree = corpseInfo->tree;
    XAnimCloneAnimTree(tree, corpseInfo->tree);
    body->s.groundEntityNum = ENTITYNUM_NONE;
    if (body->r.svFlags)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1559, 0, "%s", "!body->r.svFlags");
    body->r.svFlags = 2;
    body->r.mins[0] = pSelf->r.mins[0];
    body->r.mins[1] = pSelf->r.mins[1];
    body->r.mins[2] = pSelf->r.mins[2];
    body->r.maxs[0] = pSelf->r.maxs[0];
    body->r.maxs[1] = pSelf->r.maxs[1];
    body->r.maxs[2] = pSelf->r.maxs[2];
    body->r.absmin[0] = pSelf->r.absmin[0];
    body->r.absmin[1] = pSelf->r.absmin[1];
    body->r.absmin[2] = pSelf->r.absmin[2];
    body->r.absmax[0] = pSelf->r.absmax[0];
    body->r.absmax[1] = pSelf->r.absmax[1];
    body->r.absmax[2] = pSelf->r.absmax[2];
    body->s.legsAnim = client->ps.legsAnim;
    body->s.torsoAnim = client->ps.torsoAnim;
    body->clipmask = 65537;
    body->r.contents = 0x4004000;
    SV_LinkEntity(body);
    body->nextthink = deathAnimDuration + level.time;
    body->handler = ENT_HANDLER_PLAYER_CLONE;
    GScr_AddEntity(body);
}

void __cdecl PlayerCmd_SetClientDvar(scr_entref_t entref)
{
    uint32_t NumParam; // eax
    const char *v3; // eax
    char v4; // al
    const char *v5; // eax
    const char *pszDvar; // [esp+18h] [ebp-818h]
    const char *pszText; // [esp+1Ch] [ebp-814h]
    int32_t i; // [esp+24h] [ebp-80Ch]
    char szString[1024]; // [esp+28h] [ebp-808h] BYREF
    char szOutString[1024]; // [esp+428h] [ebp-408h] BYREF
    char *pCh; // [esp+82Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        if (!g_entities[entref.entnum].client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    pszDvar = Scr_GetString(0);
    if (Scr_GetType(1) == 3)
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(1, NumParam - 1, "Client Dvar Value", szString, 0x400u);
        pszText = szString;
    }
    else
    {
        pszText = Scr_GetString(1);
    }
    strlen(pszText);
    if (Dvar_IsValidName(pszDvar))
    {
        pCh = szOutString;
        memset((uint8_t *)szOutString, 0, sizeof(szOutString));
        for (i = 0; i < 1023 && pszText[i]; ++i)
        {
            v4 = I_CleanChar(pszText[i]);
            *pCh = v4;
            if (*pCh == 34)
                *pCh = 39;
            ++pCh;
        }
        v5 = va("%c %s \"%s\"", 118, pszDvar, szOutString);
        SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, v5);
    }
    else
    {
        v3 = va("Dvar %s has an invalid dvar name", pszDvar);
        Scr_Error(v3);
    }
}

void __cdecl PlayerCmd_SetClientDvars(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    uint8_t *c; // [esp+4h] [ebp-814h]
    char finalString[1024]; // [esp+8h] [ebp-810h] BYREF
    char tempString[1024]; // [esp+408h] [ebp-410h] BYREF
    const char *dvarName; // [esp+80Ch] [ebp-Ch]
    uint32_t i; // [esp+810h] [ebp-8h]
    const char *value; // [esp+814h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1636, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam() % 2)
        Scr_Error(
            "Not enough parameters to setclientdvar() - must be an even number of parameters (dvar, value, dvar, value, etc.)\n");
    strcpy(finalString, "v");
    for (i = 0; i < Scr_GetNumParam(); i += 2)
    {
        dvarName = Scr_GetString(i);
        value = Scr_GetString(i + 1);
        if (!Dvar_IsValidName(dvarName))
        {
            v2 = va("Dvar %s has an invalid dvar name", dvarName);
            Scr_Error(v2);
            return;
        }
        I_strncpyz(tempString, (char *)value, 1024);
        for (c = (uint8_t *)tempString; *c; ++c)
        {
            *c = I_CleanChar(*c);
            if (*c == 34)
                *c = 39;
        }
        I_strncat(finalString, 1024, " ");
        I_strncat(finalString, 1024, (char *)dvarName);
        I_strncat(finalString, 1024, " \"");
        I_strncat(finalString, 1024, tempString);
        I_strncat(finalString, 1024, "\"");
    }
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, finalString);
}

void __cdecl PlayerCmd_IsTalking(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-8h]
    int32_t elapsedTime; // [esp+4h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1688, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    elapsedTime = level.time - pSelf->client->lastVoiceTime;
    if (elapsedTime < 0 || elapsedTime >= g_voiceChatTalkingDuration->current.integer)
        Scr_AddInt(0);
    else
        Scr_AddInt(1);
}

void __cdecl PlayerCmd_FreezeControls(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1700, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->bFrozen = Scr_GetInt(0);
}

void __cdecl PlayerCmd_DisableWeapons(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1707, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->ps.weapFlags |= 0x80u;
}

void __cdecl PlayerCmd_EnableWeapons(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1714, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->ps.weapFlags &= ~0x80u;
}

void __cdecl PlayerCmd_SetReverb(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float drylevel; // [esp+20h] [ebp-18h]
    float fadetime; // [esp+24h] [ebp-14h]
    float wetlevel; // [esp+28h] [ebp-10h]
    const char *pszReverb; // [esp+2Ch] [ebp-Ch]
    uint16_t prio_name; // [esp+30h] [ebp-8h]
    int32_t prio; // [esp+34h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1728, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    fadetime = 0.0;
    drylevel = 1.0;
    wetlevel = 0.5;
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto $LN6_34;
    case 3u:
        goto $LN7_31;
    case 4u:
        goto $LN8_23;
    case 5u:
        fadetime = Scr_GetFloat(4);
    $LN8_23:
        wetlevel = Scr_GetFloat(3);
    $LN7_31:
        drylevel = Scr_GetFloat(2);
    $LN6_34:
        pszReverb = Scr_GetString(1);
        prio_name = Scr_GetConstString(0);
        prio = 1;
        if (prio_name == scr_const.snd_enveffectsprio_level)
        {
            prio = 1;
        }
        else if (prio_name == scr_const.snd_enveffectsprio_shellshock)
        {
            prio = 2;
        }
        else
        {
            Scr_Error("priority must be 'snd_enveffectsprio_level' or 'snd_enveffectsprio_shellshock'\n");
        }
        v2 = va("%c %i \"%s\" %g %g %g", 114, prio, pszReverb, drylevel, wetlevel, fadetime);
        SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, v2);
        break;
    default:
        Scr_Error(
            "USAGE: player setReverb(\"priority\", \"roomtype\", drylevel = 1.0, wetlevel = 0.5, fadetime = 0);\n"
            "Valid priorities are \"snd_enveffectsprio_level\" or \"snd_enveffectsprio_shellshock\", dry level is a float fro"
            "m 0 (no source sound) to 1 (full source sound), wetlevel is a float from 0 (no effect) to 1 (full effect), fadet"
            "ime is in sec and modifies drylevel and wetlevel\n");
        break;
    }
}

//void __cdecl PlayerCmd_DeactivateReverb(scr_entref_t *entref)
void __cdecl PlayerCmd_DeactivateReverb(scr_entref_t e)
{
    scr_entref_t *entref = &e; // HACK

    uint16_t v1; // r30
    const char *v2; // r3
    double Float; // fp31
    uint32_t NumParam; // r3
    int32_t ConstString; // r10
    const char *v6; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
    }
    else
    {
        if (g_entities[HIWORD(entref)].client)
            goto LABEL_6;
        v2 = va("entity %i is not a player", HIWORD(entref));
    }
    Scr_ObjectError(v2);
LABEL_6:
    Float = 0.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error(
                "USAGE: player deactivateReverb(\"priority\", fadetime = 0);\n"
                "Valid priorities are \"snd_enveffectsprio_level\" or \"snd_enveffectsprio_shellshock\", fadetime is the time spe"
                "nt fading to the next lowest active reverb priority level in seconds\n");
            return;
        }
        Float = Scr_GetFloat(1);
    }
    ConstString = Scr_GetConstString(0);
    if (ConstString != scr_const.snd_enveffectsprio_level && ConstString != scr_const.snd_enveffectsprio_shellshock)
        Scr_Error("priority must be 'snd_enveffectsprio_level' or 'snd_enveffectsprio_shellshock'\n");
    v6 = va("%c %i \"%s\" %g %g %g", 68, Float);
    SV_GameSendServerCommand(v1, SV_CMD_RELIABLE, v6);
}

void __cdecl PlayerCmd_SetChannelVolumes(scr_entref_t entref)
{
    uint32_t NumParam; // [esp+8h] [ebp-18h]
    float fadetime; // [esp+10h] [ebp-10h]
    int32_t shockIndex; // [esp+14h] [ebp-Ch]
    uint16_t prio_name; // [esp+18h] [ebp-8h]
    int32_t prio; // [esp+1Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1806, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    fadetime = 0.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 2)
    {
        if (NumParam != 3)
        {
            Scr_Error(
                "USAGE: player setchannelvolumes(\"priority\", \"shock name\", fadetime = 0);\n"
                "Valid priorities are \"snd_channelvolprio_holdbreath\", \"snd_channelvolprio_pain\", or \"snd_channelvolprio_she"
                "llshock\", fadetime is in sec\n");
            return;
        }
        fadetime = Scr_GetFloat(2);
    }
    shockIndex = G_FindConfigstringIndex((char*)Scr_GetString(1), 1954, 16, 0, 0);
    prio_name = Scr_GetConstString(0);
    prio = 1;
    if (prio_name == scr_const.snd_channelvolprio_holdbreath)
    {
        prio = 1;
    }
    else if (prio_name == scr_const.snd_channelvolprio_pain)
    {
        prio = 2;
    }
    else if (prio_name == scr_const.snd_channelvolprio_shellshock)
    {
        prio = 3;
    }
    else
    {
        Scr_Error(
            "priority must be 'snd_channelvolprio_holdbreath', 'snd_channelvolprio_pain', or 'snd_channelvolprio_shellshock'\n");
    }

    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c %i %i %g", 69, prio, shockIndex, fadetime));
}

//void __cdecl PlayerCmd_DeactivateChannelVolumes(scr_entref_t *entref)
void __cdecl PlayerCmd_DeactivateChannelVolumes(scr_entref_t e)
{
    scr_entref_t *entref = &e; // HACK

    uint16_t v1; // r30
    const char *v2; // r3
    double Float; // fp31
    uint32_t NumParam; // r3
    int32_t ConstString; // r10
    const char *v6; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
    }
    else
    {
        if (g_entities[HIWORD(entref)].client)
            goto LABEL_6;
        v2 = va("entity %i is not a player", HIWORD(entref));
    }
    Scr_ObjectError(v2);
LABEL_6:
    Float = 0.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error(
                "USAGE: player deactivatechannelvolumes(\"priority\", fadetime = 0);\n"
                "Valid priorities are \"snd_channelvolprio_holdbreath\", \"snd_channelvolprio_pain\", or \"snd_channelvolprio_she"
                "llshock\", fadetime is the time spent fading to the next lowest active reverb priority level in seconds\n");
            return;
        }
        Float = Scr_GetFloat(1);
    }
    ConstString = Scr_GetConstString(0);
    if (ConstString != scr_const.snd_channelvolprio_holdbreath
        && ConstString != scr_const.snd_channelvolprio_pain
        && ConstString != scr_const.snd_channelvolprio_shellshock)
    {
        Scr_Error(
            "priority must be 'snd_channelvolprio_holdbreath', 'snd_channelvolprio_pain', or 'snd_channelvolprio_shellshock'\n");
    }
    v6 = va("%c %i \"%s\" %g %g %g", 70, Float);
    SV_GameSendServerCommand(v1, SV_CMD_RELIABLE, v6);
}

void __cdecl ScrCmd_PlayLocalSound(scr_entref_t entref)
{
    const char *pszSoundName; // [esp+4h] [ebp-4Ch]
    char svcmd[64]; // [esp+8h] [ebp-48h] BYREF
    uint8_t soundIndex; // [esp+4Fh] [ebp-1h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        if (!g_entities[entref.entnum].client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    pszSoundName = Scr_GetString(0);
    soundIndex = G_SoundAliasIndex((char*)pszSoundName);
    _snprintf(svcmd, 0x40u, "%c %i", 115, soundIndex);
    svcmd[63] = 0;
    SV_GameSendServerCommand(entref.entnum, SV_CMD_CAN_IGNORE, svcmd);
}

void __cdecl ScrCmd_StopLocalSound(scr_entref_t entref)
{
    const char *pszSoundName; // [esp+4h] [ebp-4Ch]
    char svcmd[64]; // [esp+8h] [ebp-48h] BYREF
    uint8_t soundIndex; // [esp+4Fh] [ebp-1h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        if (!g_entities[entref.entnum].client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    pszSoundName = Scr_GetString(0);
    soundIndex = G_SoundAliasIndex((char*)pszSoundName);
    _snprintf(svcmd, 0x40u, "%c %i", 107, soundIndex);
    svcmd[63] = 0;
    SV_GameSendServerCommand(entref.entnum, SV_CMD_CAN_IGNORE, svcmd);
}

void __cdecl PlayerCmd_SayAll(scr_entref_t entref)
{
    const char *v1; // eax
    uint32_t NumParam; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-40Ch]
    char szString[1028]; // [esp+4h] [ebp-408h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1934, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Client Chat Message", &szString[1], 0x3FFu);
    szString[0] = 20;
    G_Say(pSelf, 0, 0, szString);
}

void __cdecl PlayerCmd_SayTeam(scr_entref_t entref)
{
    const char *v1; // eax
    uint32_t NumParam; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-40Ch]
    char szString[1028]; // [esp+4h] [ebp-408h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1958, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    NumParam = Scr_GetNumParam();
    Scr_ConstructMessageString(0, NumParam - 1, "Client Chat Message", &szString[1], 0x3FFu);
    szString[0] = 20;
    G_Say(pSelf, 0, 1, szString);
}

void __cdecl PlayerCmd_AllowADS(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1969, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        if (Scr_GetInt(0))
        {
            pSelf->client->ps.weapFlags &= ~0x20u;
        }
        else
        {
            pSelf->client->ps.weapFlags |= 0x20u;
            PM_ExitAimDownSight(&pSelf->client->ps);
        }
    }
    else
    {
        Scr_Error("USAGE: <player> allowads( <boolean> )\n");
    }
}

void __cdecl PlayerCmd_AllowJump(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 1991, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetInt(0))
        pSelf->client->ps.pm_flags &= ~0x80000u;
    else
        pSelf->client->ps.pm_flags |= 0x80000u;
}

void __cdecl PlayerCmd_AllowSprint(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2002, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetInt(0))
        pSelf->client->ps.pm_flags &= ~0x40000u;
    else
        pSelf->client->ps.pm_flags |= 0x40000u;
}

void __cdecl PlayerCmd_SetSpreadOverride(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-8h]
    int32_t value; // [esp+4h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2015, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        value = Scr_GetInt(0);
        if (value > 0)
        {
            if (value < 64)
            {
                pSelf->client->ps.spreadOverride = value;
                pSelf->client->ps.spreadOverrideState = 2;
            }
            else
            {
                v2 = va("setspreadoverride: spread must be < %d", 64);
                Scr_ParamError(0, v2);
            }
        }
        else
        {
            Scr_ParamError(0, "setspreadoverride: spread must be > 0");
        }
    }
    else
    {
        Scr_Error("USAGE: <player> setspreadoverride( <spread> )\n");
    }
}

void __cdecl PlayerCmd_ResetSpreadOverride(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2043, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    pSelf->client->ps.spreadOverrideState = 1;
    pSelf->client->ps.aimSpreadScale = 255.0;
    if (Scr_GetNumParam())
        Scr_Error("USAGE: <player> resetspreadoverride()\n");
}

void __cdecl PlayerCmd_AllowSpectateTeam(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-10h]
    uint16_t teamString; // [esp+8h] [ebp-8h]
    int32_t teamBit; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2071, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    teamString = Scr_GetConstString(0);
    teamBit = 0;
    if (teamString == scr_const.axis)
    {
        teamBit = 2;
    }
    else if (teamString == scr_const.allies)
    {
        teamBit = 4;
    }
    else if (teamString == scr_const.none)
    {
        teamBit = 1;
    }
    else if (teamString == scr_const.freelook)
    {
        teamBit = 16;
    }
    else
    {
        Scr_ParamError(0, "team must be \"axis\", \"allies\", \"none\", or \"freelook\"");
    }
    if (Scr_GetInt(1))
        pSelf->client->sess.noSpectate &= ~teamBit;
    else
        pSelf->client->sess.noSpectate |= teamBit;
}

void __cdecl PlayerCmd_GetGuid(scr_entref_t entref)
{
    const char *v1; // eax
    char *Guid; // eax

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2098, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam())
        Scr_Error("USAGE: self getGuid()\n");
    Guid = SV_GetGuid(entref.entnum);
    Scr_AddString(Guid);
}

void __cdecl PlayerCmd_GetXuid(scr_entref_t entref)
{
    const char *v1; // eax

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2113, 0, "%s", "entref.entnum < MAX_GENTITIES");
        if (!g_entities[entref.entnum].client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam())
        Scr_Error("USAGE: self getXuid()\n");
    Scr_AddString("0");
}

void __cdecl PlayerCmd_BeginLocationSelection(scr_entref_t entref)
{
    const char *v1; // eax
    float v2; // [esp+0h] [ebp-24h]
    gentity_s *pSelf; // [esp+10h] [ebp-14h]
    float radius; // [esp+14h] [ebp-10h]
    float radiusa; // [esp+14h] [ebp-10h]
    int32_t locSelIndex; // [esp+18h] [ebp-Ch]
    const char *locSelName; // [esp+1Ch] [ebp-8h]
    uint32_t radiusBits; // [esp+20h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2143, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (!pSelf->client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2145, 0, "%s", "pSelf->client");
    if (pSelf->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            2146,
            0,
            "%s",
            "pSelf->client->sess.connected != CON_DISCONNECTED");
    locSelName = Scr_GetString(0);
    locSelIndex = GScr_GetLocSelIndex(locSelName);
    if (locSelIndex < 1 || locSelIndex > 4)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            2150,
            0,
            "locSelIndex not in [1, MAX_LOC_SEL_MTLS + 1]\n\t%i not in [%i, %i]",
            locSelIndex,
            1,
            4);
    if (Scr_GetNumParam() < 2)
    {
        radiusa = 0.15000001f;
    }
    else
    {
        radius = Scr_GetFloat(1);
        if (radius <= 0.0f)
            Scr_ParamError(1u, "Radius of location selector must be greater than zero\n");
        if (level.compassMapWorldSize[1] <= 0.0f || radius <= 0.0f)
            radiusa = radius / 1000.0f;
        else
            radiusa = radius / level.compassMapWorldSize[1];
        if (0.0f >= 1.0f) // ?????
            MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 533, 0, "%s", "min < max");
        if (radiusa >= 0.0f)
        {
            if (radiusa > 1.0f)
                radiusa = 1.0f;
        }
        else
        {
            radiusa = 0.0f;
        }
    }
    radiusBits = (int)(radiusa * 63.0f);
    if (radiusBits >= 0x40)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            2173,
            0,
            "radiusBits doesn't index (1 << LOC_SEL_RADIUS_BITS)\n\t%i not in [0, %i)",
            radiusBits,
            64);
    pSelf->client->ps.locationSelectionInfo = locSelIndex | (4 * radiusBits);
}

void __cdecl PlayerCmd_EndLocationSelection(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2181, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (!pSelf->client)
        MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2183, 0, "%s", "pSelf->client");
    if (pSelf->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(
            ".\\game_mp\\g_client_script_cmd_mp.cpp",
            2184,
            0,
            "%s",
            "pSelf->client->sess.connected != CON_DISCONNECTED");
    pSelf->client->ps.locationSelectionInfo = 0;
}

void __cdecl PlayerCmd_SetActionSlot(scr_entref_t entref)
{
    uint32_t weaponIdx; // [esp+0h] [ebp-10h]
    const char *str; // [esp+4h] [ebp-Ch]
    const char *stra; // [esp+4h] [ebp-Ch]
    gentity_s *pSelf; // [esp+8h] [ebp-8h]
    int32_t slot; // [esp+Ch] [ebp-4h]
    int32_t slota; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    slot = Scr_GetInt(0);
    if (slot >= 1 && slot <= 4)
    {
        slota = slot - 1;
        str = Scr_GetString(1);
        if (I_stricmp(str, "weapon"))
        {
            if (I_stricmp(str, "altmode"))
            {
                if (I_stricmp(str, "nightvision"))
                {
                    if (I_stricmp(str, ""))
                        Scr_Error("Invalid option: expected \"weapon\", \"altweapon\", or \"nightvision\".\n");
                    else
                        pSelf->client->ps.actionSlotType[slota] = ACTIONSLOTTYPE_DONOTHING;
                }
                else
                {
                    pSelf->client->ps.actionSlotType[slota] = ACTIONSLOTTYPE_NIGHTVISION;
                }
            }
            else
            {
                pSelf->client->ps.actionSlotType[slota] = ACTIONSLOTTYPE_ALTWEAPONTOGGLE;
            }
        }
        else
        {
            stra = Scr_GetString(2);
            weaponIdx = BG_FindWeaponIndexForName(stra);
            if (weaponIdx)
            {
                pSelf->client->ps.actionSlotType[slota] = ACTIONSLOTTYPE_SPECIFYWEAPON;
                pSelf->client->ps.actionSlotParam[slota].specifyWeapon.index = weaponIdx;
            }
            else
            {
                Scr_ParamError(2, va("Unknown weapon name \"%s\".\n", stra));
            }
        }
    }
    else
    {
        Scr_Error(va("Invalid slot (%i) given, expecting 1 - %i\n", slot, 4));
    }
}

void __cdecl PlayerCmd_GetWeaponsList(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+0h] [ebp-14h]
    WeaponDef *weapDef; // [esp+4h] [ebp-10h]
    gentity_s *pSelf; // [esp+8h] [ebp-Ch]
    uint32_t weapCount; // [esp+Ch] [ebp-8h]
    uint32_t idx; // [esp+10h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2247, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    weapCount = BG_GetNumWeapons();
    Scr_MakeArray();
    for (idx = 1; idx < weapCount; ++idx)
    {
        client = pSelf->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(client->ps.weapons, idx, 16))
        {
            weapDef = BG_GetWeaponDef(idx);
            Scr_AddString((char *)weapDef->szInternalName);
            Scr_AddArray();
        }
    }
}

void __cdecl PlayerCmd_GetWeaponsListPrimaries(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // [esp+0h] [ebp-14h]
    WeaponDef *weapDef; // [esp+4h] [ebp-10h]
    gentity_s *pSelf; // [esp+8h] [ebp-Ch]
    uint32_t weapCount; // [esp+Ch] [ebp-8h]
    uint32_t idx; // [esp+10h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2271, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    weapCount = BG_GetNumWeapons();
    Scr_MakeArray();
    for (idx = 1; idx < weapCount; ++idx)
    {
        client = pSelf->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(client->ps.weapons, idx, 16))
        {
            weapDef = BG_GetWeaponDef(idx);
            if (weapDef->inventoryType == WEAPINVENTORY_PRIMARY)
            {
                Scr_AddString((char *)weapDef->szInternalName);
                Scr_AddArray();
            }
        }
    }
}

void __cdecl PlayerCmd_SetPerk(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]
    const char *perkName; // [esp+4h] [ebp-8h]
    uint32_t perkIndex; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    perkName = Scr_GetString(0);
    perkIndex = BG_GetPerkIndexForName(perkName);
    if (perkIndex == 20)
    {
        Scr_Error(va("Unknown perk: %s\n", perkName));
    }
    BG_SetPerk(&pSelf->client->ps.perks, perkIndex);
    BG_SetPerk(&pSelf->client->sess.cs.perks, perkIndex);
}

void __cdecl BG_SetPerk(int32_t *perks, uint32_t perkIndex)
{
    iassert(perks);
    bcassert(perkIndex, PERK_COUNT);

    *perks |= 1 << perkIndex;
}

void __cdecl PlayerCmd_HasPerk(scr_entref_t entref)
{
    int32_t perks; // [esp+0h] [ebp-10h]
    gentity_s *pSelf; // [esp+4h] [ebp-Ch]
    const char *perkName; // [esp+8h] [ebp-8h]
    uint32_t perkIndex; // [esp+Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2337, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    perkName = Scr_GetString(0);
    perkIndex = BG_GetPerkIndexForName(perkName);
    if (perkIndex == 20)
    {
        Scr_Error(va("Unknown perk: %s\n", perkName));
    }
    perks = pSelf->client->ps.perks;
    if (perkIndex >= 0x14)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../bgame/bg_perks_mp.h",
            40,
            0,
            "perkIndex doesn't index PERK_COUNT\n\t%i not in [0, %i)",
            perkIndex,
            20);
    Scr_AddBool((perks & (1 << perkIndex)) != 0);
}

void __cdecl PlayerCmd_UnsetPerk(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]
    const char *perkName; // [esp+4h] [ebp-8h]
    uint32_t perkIndex; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);

        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            Scr_ObjectError(va("entity %i is not a player", entref.entnum));
        }
    }
    perkName = Scr_GetString(0);
    perkIndex = BG_GetPerkIndexForName(perkName);
    if (perkIndex == 20)
    {
        Scr_Error(va("Unknown perk: %s\n", perkName));
    }
    BG_UnsetPerk(&pSelf->client->ps.perks, perkIndex);
    BG_UnsetPerk(&pSelf->client->sess.cs.perks, perkIndex);
}

void __cdecl BG_UnsetPerk(int32_t *perks, uint32_t perkIndex)
{
    if (!perks)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_perks_mp.h", 55, 0, "%s", "perks");
    if (perkIndex >= 0x14)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../bgame/bg_perks_mp.h",
            56,
            0,
            "perkIndex doesn't index PERK_COUNT\n\t%i not in [0, %i)",
            perkIndex,
            20);
    *perks &= ~(1 << perkIndex);
}

void __cdecl PlayerCmd_ClearPerks(scr_entref_t entref)
{
    const char *v1; // eax
    gclient_s *client; // eax
    gclient_s *v3; // eax
    int32_t *v4; // [esp+0h] [ebp-Ch]
    int32_t *p_perks; // [esp+4h] [ebp-8h]
    gentity_s *pSelf; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2388, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    client = pSelf->client;
    p_perks = &client->ps.perks;
    if (client == (gclient_s *)-1532)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_perks_mp.h", 63, 0, "%s", "perks");
    *p_perks = 0;
    v3 = pSelf->client;
    v4 = &v3->sess.cs.perks;
    if (v3 == (gclient_s *)-12388)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_perks_mp.h", 63, 0, "%s", "perks");
    *v4 = 0;
}

void __cdecl PlayerCmd_UpdateScores(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4Ch]
    char svcmd[68]; // [esp+4h] [ebp-48h] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2409, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    _snprintf(svcmd, 0x40u, "%c %i", 72, level.teamScores[2]);
    svcmd[63] = 0;
    SV_GameSendServerCommand(pSelf - g_entities, SV_CMD_CAN_IGNORE, svcmd);
    _snprintf(svcmd, 0x40u, "%c %i", 71, level.teamScores[1]);
    svcmd[63] = 0;
    SV_GameSendServerCommand(pSelf - g_entities, SV_CMD_CAN_IGNORE, svcmd);
}

void __cdecl PlayerCmd_UpdateDMScores(scr_entref_t entref)
{
    const char *v1; // eax
    int32_t i; // [esp+4h] [ebp-58h]
    gentity_s *pSelf; // [esp+8h] [ebp-54h]
    char svcmd[64]; // [esp+Ch] [ebp-50h] BYREF
    int32_t numSorted; // [esp+50h] [ebp-Ch]
    int32_t nextBestClientIndex; // [esp+54h] [ebp-8h]
    int32_t selfClientIndex; // [esp+58h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2437, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    selfClientIndex = pSelf - g_entities;
    nextBestClientIndex = -1;
    numSorted = level.numConnectedClients;
    for (i = 0; i < numSorted; ++i)
    {
        if (level.sortedClients[i] != selfClientIndex)
        {
            nextBestClientIndex = level.sortedClients[i];
            break;
        }
    }
    _snprintf(svcmd, 0x40u, "%c %i %i", 73, selfClientIndex, level.clients[selfClientIndex].sess.score);
    svcmd[63] = 0;
    SV_GameSendServerCommand(pSelf - g_entities, SV_CMD_CAN_IGNORE, svcmd);
    if (nextBestClientIndex >= 0)
    {
        _snprintf(svcmd, 0x40u, "%c %i %i", 73, nextBestClientIndex, level.clients[nextBestClientIndex].sess.score);
        svcmd[63] = 0;
        SV_GameSendServerCommand(pSelf - g_entities, SV_CMD_CAN_IGNORE, svcmd);
    }
}

void __cdecl PlayerCmd_SetRank(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]
    int32_t prestige; // [esp+4h] [ebp-8h]
    int32_t rank; // [esp+8h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_client_script_cmd_mp.cpp", 2483, 0, "%s", "entref.entnum < MAX_GENTITIES");
        pSelf = &g_entities[entref.entnum];
        if (!pSelf->client)
        {
            v1 = va("entity %i is not a player", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    rank = Scr_GetInt(0);
    if ((uint32_t)rank < 0x100)
    {
        pSelf->client->sess.cs.rank = rank;
        if (Scr_GetNumParam() >= 2)
        {
            prestige = Scr_GetInt(1);
            if ((uint32_t)prestige < 0x100)
            {
                pSelf->client->sess.cs.prestige = prestige;
            }
            else
            {
                v3 = va("'%i' is an illegal prestige value.  Must be less than %i.\n", prestige, 256);
                Scr_Error(v3);
            }
        }
    }
    else
    {
        v2 = va("'%i' is an illegal rank value.  Must be less than %i.\n", rank, 256);
        Scr_Error(v2);
    }
}

const BuiltinMethodDef methods[83] =
{
  { "giveweapon", &PlayerCmd_giveWeapon, 0 },
  { "takeweapon", &PlayerCmd_takeWeapon, 0 },
  { "takeallweapons", &PlayerCmd_takeAllWeapons, 0 },
  { "getcurrentweapon", &PlayerCmd_getCurrentWeapon, 0 },
  { "getcurrentoffhand", &PlayerCmd_getCurrentOffhand, 0 },
  { "hasweapon", &PlayerCmd_hasWeapon, 0 },
  { "switchtoweapon", &PlayerCmd_switchToWeapon, 0 },
  { "switchtooffhand", &PlayerCmd_switchToOffhand, 0 },
  { "givestartammo", &PlayerCmd_giveStartAmmo, 0 },
  { "givemaxammo", &PlayerCmd_giveMaxAmmo, 0 },
  { "getfractionstartammo", &PlayerCmd_getFractionStartAmmo, 0 },
  { "getfractionmaxammo", &PlayerCmd_getFractionMaxAmmo, 0 },
  { "setorigin", &PlayerCmd_setOrigin, 0 },
  { "getvelocity", &PlayerCmd_GetVelocity, 0 },
  { "setplayerangles", &PlayerCmd_setAngles, 0 },
  { "getplayerangles", &PlayerCmd_getAngles, 0 },
  { "usebuttonpressed", &PlayerCmd_useButtonPressed, 0 },
  { "attackbuttonpressed", &PlayerCmd_attackButtonPressed, 0 },
  { "adsbuttonpressed", &PlayerCmd_adsButtonPressed, 0 },
  { "meleebuttonpressed", &PlayerCmd_meleeButtonPressed, 0 },
  { "fragbuttonpressed", &PlayerCmd_fragButtonPressed, 0 },
  {
    "secondaryoffhandbuttonpressed",
    &PlayerCmd_secondaryOffhandButtonPressed,
    0
  },
  { "playerads", &PlayerCmd_playerADS, 0 },
  { "isonground", &PlayerCmd_isOnGround, 0 },
  { "pingplayer", &PlayerCmd_pingPlayer, 0 },
  { "setviewmodel", &PlayerCmd_SetViewmodel, 0 },
  { "getviewmodel", &PlayerCmd_GetViewmodel, 0 },
  { "setoffhandsecondaryclass", &PlayerCmd_setOffhandSecondaryClass, 0 },
  { "getoffhandsecondaryclass", &PlayerCmd_getOffhandSecondaryClass, 0 },
  { "beginlocationselection", &PlayerCmd_BeginLocationSelection, 0 },
  { "endlocationselection", &PlayerCmd_EndLocationSelection, 0 },
  { "buttonpressed", &PlayerCmd_buttonPressedDEVONLY, 0 },
  { "sayall", &PlayerCmd_SayAll, 0 },
  { "sayteam", &PlayerCmd_SayTeam, 0 },
  { "showscoreboard", &PlayerCmd_showScoreboard, 0 },
  { "setspawnweapon", &PlayerCmd_setSpawnWeapon, 0 },
  { "dropitem", &PlayerCmd_dropItem, 0 },
  { "finishplayerdamage", &PlayerCmd_finishPlayerDamage, 0 },
  { "suicide", &PlayerCmd_Suicide, 0 },
  { "openmenu", &PlayerCmd_OpenMenu, 0 },
  { "openmenunomouse", &PlayerCmd_OpenMenuNoMouse, 0 },
  { "closemenu", &PlayerCmd_CloseMenu, 0 },
  { "closeingamemenu", &PlayerCmd_CloseInGameMenu, 0 },
  { "freezecontrols", &PlayerCmd_FreezeControls, 0 },
  { "disableweapons", &PlayerCmd_DisableWeapons, 0 },
  { "enableweapons", &PlayerCmd_EnableWeapons, 0 },
  { "setreverb", &PlayerCmd_SetReverb, 0 },
  { "deactivatereverb", &PlayerCmd_DeactivateReverb, 0 },
  { "setchannelvolumes", &PlayerCmd_SetChannelVolumes, 0 },
  { "deactivatechannelvolumes", &PlayerCmd_DeactivateChannelVolumes, 0 },
  { "setweaponammoclip", &PlayerCmd_SetWeaponAmmoClip, 0 },
  { "setweaponammostock", &PlayerCmd_SetWeaponAmmoStock, 0 },
  { "getweaponammoclip", &PlayerCmd_GetWeaponAmmoClip, 0 },
  { "getweaponammostock", &PlayerCmd_GetWeaponAmmoStock, 0 },
  { "anyammoforweaponmodes", &PlayerCmd_AnyAmmoForWeaponModes, 0 },
  { "iprintln", &iclientprintln, 0 },
  { "iprintlnbold", &iclientprintlnbold, 0 },
  { "spawn", &PlayerCmd_spawn, 0 },
  { "setentertime", &PlayerCmd_setEnterTime, 0 },
  { "cloneplayer", &PlayerCmd_ClonePlayer, 0 },
  { "setclientdvar", &PlayerCmd_SetClientDvar, 0 },
  { "setclientdvars", &PlayerCmd_SetClientDvars, 0 },
  { "playlocalsound", &ScrCmd_PlayLocalSound, 0 },
  { "stoplocalsound", &ScrCmd_StopLocalSound, 0 },
  { "istalking", &PlayerCmd_IsTalking, 0 },
  { "allowspectateteam", &PlayerCmd_AllowSpectateTeam, 0 },
  { "getguid", &PlayerCmd_GetGuid, 0 },
  { "getxuid", &PlayerCmd_GetXuid, 0 },
  { "allowads", &PlayerCmd_AllowADS, 0 },
  { "allowjump", &PlayerCmd_AllowJump, 0 },
  { "allowsprint", &PlayerCmd_AllowSprint, 0 },
  { "setspreadoverride", &PlayerCmd_SetSpreadOverride, 0 },
  { "resetspreadoverride", &PlayerCmd_ResetSpreadOverride, 0 },
  { "setactionslot", &PlayerCmd_SetActionSlot, 0 },
  { "getweaponslist", &PlayerCmd_GetWeaponsList, 0 },
  { "getweaponslistprimaries", &PlayerCmd_GetWeaponsListPrimaries, 0 },
  { "setperk", &PlayerCmd_SetPerk, 0 },
  { "hasperk", &PlayerCmd_HasPerk, 0 },
  { "clearperks", &PlayerCmd_ClearPerks, 0 },
  { "unsetperk", &PlayerCmd_UnsetPerk, 0 },
  { "updatescores", &PlayerCmd_UpdateScores, 0 },
  { "updatedmscores", &PlayerCmd_UpdateDMScores, 0 },
  { "setrank", &PlayerCmd_SetRank, 0 }
}; // idb

void(__cdecl *__cdecl Player_GetMethod(const char **pName))(scr_entref_t)
{
    uint32_t i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0x53; ++i)
    {
        if (!strcmp(*pName, methods[i].actionString))
        {
            *pName = methods[i].actionString;
            return methods[i].actionFunc;
        }
    }
    return 0;
}

