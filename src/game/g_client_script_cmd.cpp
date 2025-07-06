#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"

int __fastcall G_GetNeededStartAmmo(gentity_s *pSelf, WeaponDef *weapDef)
{
    gclient_s *client; // r29
    unsigned int v5; // r30
    int v6; // r28
    WeaponDef *WeaponDef; // r31

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 44, 0, "%s", "pSelf");
    if (!pSelf->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 45, 0, "%s", "pSelf->client");
    if (!weapDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 46, 0, "%s", "weapDef");
    client = pSelf->client;
    v5 = 0;
    v6 = client->ps.ammo[weapDef->iAmmoIndex];
    do
    {
        WeaponDef = BG_GetWeaponDef(v5);
        if (WeaponDef->iAmmoIndex == weapDef->iAmmoIndex)
        {
            if (!client)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(client->ps.weapons, v5, 16) && weapDef != WeaponDef)
                v6 += client->ps.ammoclip[WeaponDef->iClipIndex] - WeaponDef->iStartAmmo;
        }
        ++v5;
    } while (v5 <= bg_lastParsedWeaponIndex);
    if (v6 >= 0)
        return weapDef->iStartAmmo - v6;
    else
        return weapDef->iStartAmmo;
}

void __fastcall InitializeAmmo(gentity_s *pSelf, int weaponIndex, unsigned __int8 weaponModel, int hadWeapon)
{
    signed int altWeaponIndex; // r31
    int NumWeapons; // r14
    WeaponDef *WeaponDef; // r29
    int NeededStartAmmo; // r6
    gclient_s *client; // r30
    unsigned int *weapons; // r30

    altWeaponIndex = weaponIndex;
    NumWeapons = BG_GetNumWeapons();
    WeaponDef = BG_GetWeaponDef(altWeaponIndex);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 87, 0, "%s", "weapDef");
    do
    {
        NeededStartAmmo = G_GetNeededStartAmmo(pSelf, WeaponDef);
        if (NeededStartAmmo <= 0)
        {
            if (!hadWeapon)
                Fill_Clip(&pSelf->client->ps, altWeaponIndex);
        }
        else
        {
            Add_Ammo(pSelf, altWeaponIndex, weaponModel, NeededStartAmmo, hadWeapon == 0);
        }
        altWeaponIndex = WeaponDef->altWeaponIndex;
        WeaponDef = BG_GetWeaponDef(altWeaponIndex);
        if (!WeaponDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 99, 0, "%s", "weapDef");
        if (--NumWeapons < 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                102,
                0,
                "%s\n\t(weapDef->szDisplayName) = %s",
                "(numWeapons >= 0)",
                WeaponDef->szDisplayName);
        if (!altWeaponIndex || altWeaponIndex == weaponIndex || NumWeapons < 0)
            break;
        client = pSelf->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        weapons = client->ps.weapons;
        if (!weapons)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\universal\\q_shared.h", 696, 0, "%s", "array");
        if ((unsigned int)altWeaponIndex >= 0x80)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\universal\\q_shared.h",
                697,
                0,
                "bitNum doesn't index size * 8\n\t%i not in [0, %i)",
                altWeaponIndex,
                128);
    } while (((1 << (altWeaponIndex & 0x1F)) & weapons[altWeaponIndex >> 5]) != 0);
}

void __fastcall PlayerCmd_giveWeapon(scr_entref_t *entref)
{
    gentity_s *v1; // r29
    const char *v2; // r3
    const char *String; // r31
    int WeaponIndexForName; // r30
    gclient_s *client; // r31
    int v6; // r28
    WeaponDef *WeaponDef; // r31
    unsigned int Int; // r3
    unsigned __int8 v9; // r31
    unsigned __int16 v10; // [sp+94h] [+14h]

    v10 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                128,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v10];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v10);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    client = v1->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    v6 = Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16);
    if (Scr_GetNumParam() != 2
        || (WeaponDef = BG_GetWeaponDef(WeaponIndexForName), Int = Scr_GetInt(1u), Int > 0xFF)
        || !WeaponDef->gunXModel[Int])
    {
        LOBYTE(Int) = 0;
    }
    v9 = Int;
    G_GivePlayerWeapon(&v1->client->ps, WeaponIndexForName, Int);
    InitializeAmmo(v1, WeaponIndexForName, v9, v6);
}

void __fastcall PlayerCmd_takeWeapon(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const char *String; // r30
    int WeaponIndexForName; // r29
    unsigned __int16 v5; // [sp+84h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                170,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    BG_TakePlayerWeapon(&v1->client->ps, WeaponIndexForName, 1);
}

void __fastcall PlayerCmd_takeAllWeapons(scr_entref_t *entref)
{
    gentity_s *v1; // r30
    const char *v2; // r3
    unsigned int v3; // r31
    unsigned __int16 v4; // [sp+84h] [+14h]

    v4 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                194,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v4];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v4);
            Scr_ObjectError(v2);
        }
    }
    v3 = 1;
    for (v1->client->ps.weapon = 0; v3 < BG_GetNumWeapons(); ++v3)
        BG_TakePlayerWeapon(&v1->client->ps, v3, 1);
}

void __fastcall PlayerCmd_getCurrentWeapon(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gclient_s *client; // r11
    unsigned int weapon; // r3
    const char *szInternalName; // r3
    unsigned __int16 v6; // [sp+84h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                221,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    client = v1->client;
    weapon = client->ps.weapon;
    if (weapon || (weapon = client->pers.cmd.weapon) != 0)
        szInternalName = BG_GetWeaponDef(weapon)->szInternalName;
    else
        szInternalName = "none";
    Scr_AddString(szInternalName);
}

void __fastcall PlayerCmd_getCurrentWeaponClipAmmo(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gclient_s *client; // r31
    int v4; // r3
    unsigned __int16 v5; // [sp+84h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                256,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    client = v1->client;
    if (client->ps.weapon)
        v4 = client->ps.ammoclip[BG_ClipForWeapon(client->ps.weapon)];
    else
        v4 = 0;
    Scr_AddInt(v4);
}

void __fastcall PlayerCmd_getCurrentOffhand(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    signed int offHandIndex; // r3
    const char *szInternalName; // r3
    unsigned __int16 v5; // [sp+84h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                285,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    offHandIndex = v1->client->ps.offHandIndex;
    if (offHandIndex <= 0)
        szInternalName = "none";
    else
        szInternalName = BG_GetWeaponDef(offHandIndex)->szInternalName;
    Scr_AddString(szInternalName);
}

void __fastcall PlayerCmd_setOffhandSecondaryClass(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const char *v3; // r3
    unsigned int ConstString; // r3
    unsigned __int16 v5; // [sp+84h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                314,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        ConstString = Scr_GetConstString(0);
        if (ConstString == scr_const.flash)
        {
            v1->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_FLASH;
            return;
        }
        if (ConstString == scr_const.smoke)
        {
            v1->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_SMOKE;
            return;
        }
        v3 = "Must specify either 'smoke' or 'flash' to set toggle to.\n";
    }
    else
    {
        v3 = "Incorrect number of parameters.\n";
    }
    Scr_Error(v3);
}

void __fastcall PlayerCmd_getOffhandSecondaryClass(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    OffhandSecondaryClass offhandSecondary; // r11
    unsigned __int16 v4; // [sp+94h] [+14h]

    v4 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                346,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v4];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v4);
            Scr_ObjectError(v2);
        }
    }
    offhandSecondary = v1->client->ps.offhandSecondary;
    if (offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH)
    {
        Scr_AddConstString(scr_const.flash);
    }
    else
    {
        if (offhandSecondary)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                354,
                0,
                "%s",
                "pSelf->client->ps.offhandSecondary == PLAYER_OFFHAND_SECONDARY_SMOKE");
        Scr_AddConstString(scr_const.smoke);
    }
}

void __fastcall PlayerCmd_hasWeapon(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const char *String; // r29
    int WeaponIndexForName; // r30
    int v5; // r3
    bool v6; // zf
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                376,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = BG_FindWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    if (!WeaponIndexForName || (v6 = BG_PlayerHasWeapon(&v1->client->ps, WeaponIndexForName) != 0, v5 = 1, !v6))
        v5 = 0;
    Scr_AddBool(v5);
}

void __fastcall PlayerCmd_switchToWeapon(scr_entref_t *entref)
{
    unsigned __int16 v1; // r27
    gentity_s *v2; // r31
    const char *v3; // r3
    const char *String; // r30
    int WeaponIndexForName; // r29
    gclient_s *client; // r31

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v2 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                405,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v2 = &g_entities[v1];
        if (!v2->client)
        {
            v3 = va("entity %i is not a player", v1);
            Scr_ObjectError(v3);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    client = v2->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16))
    {
        G_SelectWeaponIndex(v1, WeaponIndexForName);
        Scr_AddBool(1);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void __fastcall PlayerCmd_switchToOffhand(scr_entref_t *entref)
{
    unsigned __int16 v1; // r27
    gentity_s *v2; // r29
    const char *v3; // r3
    const char *String; // r30
    int WeaponIndexForName; // r31
    WeaponDef *WeaponDef; // r3
    const char *v7; // r3
    gclient_s *client; // r30

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v2 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                438,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v2 = &g_entities[v1];
        if (!v2->client)
        {
            v3 = va("entity %i is not a player", v1);
            Scr_ObjectError(v3);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    if (!BG_ValidateWeaponNumberOffhand(WeaponIndexForName))
    {
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        v7 = va("%s is not a valid offhand weapon", WeaponDef->szInternalName);
        Scr_Error(v7);
    }
    client = v2->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16))
    {
        G_SetEquippedOffHand(v1, WeaponIndexForName);
        Scr_AddBool(1);
    }
    else
    {
        Scr_AddBool(0);
    }
}

void __fastcall PlayerCmd_giveStartAmmo(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const char *String; // r30
    int WeaponIndexForName; // r29
    gclient_s *client; // r30
    unsigned __int16 v6; // [sp+94h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                474,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    client = v1->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16))
        InitializeAmmo(v1, WeaponIndexForName, v1->client->ps.weaponmodels[WeaponIndexForName], 0);
}

void __fastcall PlayerCmd_giveMaxAmmo(scr_entref_t *entref)
{
    gentity_s *v1; // r30
    const char *v2; // r3
    const char *String; // r29
    int WeaponIndexForName; // r31
    gclient_s *client; // r29
    int AmmoPlayerMax; // r29
    WeaponDef *WeaponDef; // r3
    gclient_s *v8; // r11
    int v9; // r6
    unsigned __int16 v10; // [sp+94h] [+14h]

    v10 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                506,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v10];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v10);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    client = v1->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16))
    {
        AmmoPlayerMax = BG_GetAmmoPlayerMax(&v1->client->ps, WeaponIndexForName, 0);
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        v8 = v1->client;
        v9 = AmmoPlayerMax - v8->ps.ammo[WeaponDef->iAmmoIndex];
        if (v9 > 0)
            Add_Ammo(v1, WeaponIndexForName, v8->ps.weaponmodels[WeaponIndexForName], v9, 0);
    }
}

// local variable allocation has failed, the output may be wrong!
void __fastcall PlayerCmd_getFractionStartAmmo(scr_entref_t *entref)
{
    gentity_s *v1; // r29
    const char *v2; // r3
    const char *String; // r31
    int WeaponIndexForName; // r30
    gclient_s *client; // r31
    WeaponDef *WeaponDef; // r3
    __int64 v7; // r11 OVERLAPPED
    gclient_s *v8; // r9 OVERLAPPED
    unsigned __int16 v9; // [sp+A4h] [+14h]

    v9 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                540,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v9];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v9);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    client = v1->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16)
        && (WeaponDef = BG_GetWeaponDef(WeaponIndexForName), HIDWORD(v7) = WeaponDef->iStartAmmo, SHIDWORD(v7) >= 1))
    {
        v8 = v1->client;
        LODWORD(v7) = v8->ps.ammo[WeaponDef->iAmmoIndex];
        if ((int)v7 >= 1)
            Scr_AddFloat((float)((float)v7 / (float)*(__int64 *)((char *)&v7 + 4)));
        else
            Scr_AddFloat(0.0);
    }
    else
    {
        Scr_AddFloat(1.0);
    }
}

// local variable allocation has failed, the output may be wrong!
void __fastcall PlayerCmd_getFractionMaxAmmo(scr_entref_t *entref)
{
    gentity_s *v1; // r30
    const char *v2; // r3
    const char *String; // r31
    int WeaponIndexForName; // r29
    gclient_s *client; // r31
    WeaponDef *WeaponDef; // r3
    __int64 v7; // r11 OVERLAPPED
    gclient_s *v8; // r9 OVERLAPPED
    unsigned __int16 v9; // [sp+A4h] [+14h]

    v9 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                586,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v9];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v9);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    client = v1->client;
    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(client->ps.weapons, WeaponIndexForName, 16))
    {
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        if (WeaponDef->bClipOnly)
        {
            HIDWORD(v7) = WeaponDef->iClipSize;
            if (SHIDWORD(v7) >= 1)
            {
                v8 = v1->client;
                LODWORD(v7) = v8->ps.ammoclip[WeaponDef->iClipIndex];
                if ((int)v7 < 1)
                {
                LABEL_13:
                    Scr_AddFloat(0.0);
                    return;
                }
                goto LABEL_17;
            }
        }
        else
        {
            HIDWORD(v7) = WeaponDef->iMaxAmmo;
            if (SHIDWORD(v7) >= 1)
            {
                v8 = v1->client;
                LODWORD(v7) = v8->ps.ammo[WeaponDef->iAmmoIndex];
                if ((int)v7 < 1)
                    goto LABEL_13;
            LABEL_17:
                Scr_AddFloat((float)((float)v7 / (float)*(__int64 *)((char *)&v7 + 4)));
                return;
            }
        }
    }
    Scr_AddFloat(1.0);
}

void __fastcall PlayerCmd_setOrigin(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    float v3[6]; // [sp+50h] [-30h] BYREF
    unsigned __int16 v4; // [sp+94h] [+14h]

    v4 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                645,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v4];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v4);
            Scr_ObjectError(v2);
        }
    }
    Scr_GetVector(0, v3);
    SetClientOrigin(v1, v3);
}

void __fastcall PlayerCmd_SetVelocity(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    float *p_commandTime; // r11
    float v4[6]; // [sp+50h] [-30h] BYREF
    unsigned __int16 v5; // [sp+94h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                668,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    Scr_GetVector(0, v4);
    p_commandTime = (float *)&v1->client->ps.commandTime;
    p_commandTime[10] = v4[0];
    p_commandTime[11] = v4[1];
    p_commandTime[12] = v4[2];
}

void __fastcall PlayerCmd_GetVelocity(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                688,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddVector(v1->client->ps.velocity);
}

void __fastcall PlayerCmd_setAngles(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    long double v3; // fp2
    float v4[6]; // [sp+50h] [-30h] BYREF
    unsigned __int16 v5; // [sp+94h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                709,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    Scr_GetVector(0, v4);
    SetClientViewAngle(v1, v4, v3);
}

void __fastcall PlayerCmd_getAngles(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                729,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddVector(v1->client->ps.viewangles);
}

void __fastcall PlayerCmd_getViewHeight(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                747,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddFloat(v1->client->ps.viewHeightCurrent);
}

void __fastcall PlayerCmd_getNormalizedMovement(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gclient_s *client; // r11
    __int128 v4; // r11
    float v5[4]; // [sp+58h] [-28h] BYREF
    unsigned __int16 v6; // [sp+94h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                767,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    client = v1->client;
    DWORD1(v4) = 45811;
    DWORD2(v4) = client->pers.cmd.forwardmove;
    v5[0] = (float)*(__int64 *)((char *)&v4 + 4) * (float)0.0078740157;
    DWORD2(v4) = (unsigned __int8)client->pers.cmd.rightmove;
    LODWORD(v4) = SBYTE11(v4);
    v5[2] = 0.0;
    v5[1] = (float)(__int64)v4 * (float)0.0078740157;
    Scr_AddVector(v5);
}

void __fastcall PlayerCmd_useButtonPressed(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                789,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddInt((((unsigned __int8)v1->client->buttonsSinceLastFrame | (unsigned __int8)v1->client->buttons) & 0x28) != 0);
}

void __fastcall PlayerCmd_attackButtonPressed(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                810,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddInt((((unsigned __int8)v1->client->buttonsSinceLastFrame | (unsigned __int8)v1->client->buttons) & 1) != 0);
}

void __fastcall PlayerCmd_adsButtonPressed(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                831,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddInt((((unsigned __int16)v1->client->buttonsSinceLastFrame | (unsigned __int16)v1->client->buttons) & 0x800) != 0);
}

void __fastcall PlayerCmd_meleeButtonPressed(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                852,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddInt((((unsigned __int8)v1->client->buttonsSinceLastFrame | (unsigned __int8)v1->client->buttons) & 4) != 0);
}

int __fastcall PlayerCmd_CheckButtonPressed()
{
    int result; // r3
    bool IsActive; // zf
    const char *String; // r3
    const char *v3; // r31

    IsActive = DevGui_IsActive();
    result = 0;
    if (!IsActive)
    {
        String = Scr_GetString(0);
        v3 = String;
        if (!String || !*String)
            Scr_ParamError(0, "usage: buttonPressed(<button name>)");
        return CL_IsKeyPressed(0, v3);
    }
    return result;
}

void __fastcall PlayerCmd_buttonPressed(scr_entref_t *entref)
{
    int v1; // r31
    int v2; // r3

    if (level.loading)
        goto LABEL_6;
    if (sv.demo.playing)
    {
        v1 = SV_DemoButtonPressed();
    }
    else
    {
        v1 = PlayerCmd_CheckButtonPressed();
        SV_RecordButtonPressed(v1);
    }
    v2 = 1;
    if (!v1)
        LABEL_6:
    v2 = 0;
    Scr_AddInt(v2);
}

void __fastcall G_FlushCommandNotifies()
{
    unsigned int v0; // r30
    unsigned int v1; // r3

    __lwsync();
    while (s_cmdNotify.read != s_cmdNotify.write)
    {
        if (s_cmdNotify.write - s_cmdNotify.read < 2)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                934,
                0,
                "s_cmdNotify.write - s_cmdNotify.read >= 2\n\t%i, %i",
                s_cmdNotify.write - s_cmdNotify.read,
                2);
        ++s_cmdNotify.read;
        v0 = *(unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * s_cmdNotify.read++) & 0x7E));
        if (!v0 || v0 > 0xA)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                944,
                0,
                "argc not in [1, G_CMD_NOTIFY_ARGC_LIMIT]\n\t%i not in [%i, %i]",
                v0,
                1,
                10);
        if (v0 > s_cmdNotify.write - s_cmdNotify.read)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                945,
                0,
                "argc <= s_cmdNotify.write - s_cmdNotify.read\n\t%i, %i",
                v0,
                s_cmdNotify.write - s_cmdNotify.read);
        for (; v0; --v0)
        {
            v1 = *(unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * s_cmdNotify.read++) & 0x7E));
            SL_RemoveRefToString(v1);
        }
        __lwsync();
    }
}

void __fastcall G_ProcessCommandNotifies()
{
    unsigned __int16 v0; // r26
    unsigned int v1; // r28
    unsigned int v2; // r30
    unsigned int v3; // r29

    __lwsync();
    if (Scr_IsSystemActive(1u))
    {
        while (s_cmdNotify.read != s_cmdNotify.write)
        {
            if (s_cmdNotify.write - s_cmdNotify.read < 2)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                    979,
                    0,
                    "s_cmdNotify.write - s_cmdNotify.read >= 2\n\t%i, %i",
                    s_cmdNotify.write - s_cmdNotify.read,
                    2);
            v0 = *(volatile unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * s_cmdNotify.read++) & 0x7E));
            v1 = *(unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * s_cmdNotify.read++) & 0x7E));
            if (!v1 || v1 > 0xA)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                    989,
                    0,
                    "argc not in [1, G_CMD_NOTIFY_ARGC_LIMIT]\n\t%i not in [%i, %i]",
                    v1,
                    1,
                    10);
            if (v1 > s_cmdNotify.write - s_cmdNotify.read)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                    990,
                    0,
                    "argc <= s_cmdNotify.write - s_cmdNotify.read\n\t%i, %i",
                    v1,
                    s_cmdNotify.write - s_cmdNotify.read);
            if (v1)
            {
                v2 = v1;
                do
                {
                    v3 = *(unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * s_cmdNotify.read++) & 0x7E));
                    Scr_AddConstString(v3);
                    SL_RemoveRefToString(v3);
                    --v2;
                } while (v2);
            }
            Scr_Notify(g_entities, v0, v1);
            __lwsync();
            if (!Scr_IsSystemActive(1u))
                goto LABEL_14;
        }
    }
    else
    {
    LABEL_14:
        s_cmdNotify.read = s_cmdNotify.write;
    }
}

void __fastcall PlayerCmd_notifyOnCommand(scr_entref_t *entref)
{
    const char *String; // r31
    const char *v2; // r3

    if (Scr_GetNumParam() != 2)
        Scr_Error("USAGE: <player> notifyOnCommand( <notify>, <command> )\n");
    String = Scr_GetString(0);
    v2 = Scr_GetString(1u);
    Cmd_RegisterNotification(v2, String);
}

void __fastcall PlayerCmd_playerADS(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1085,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    Scr_AddFloat(v1->client->ps.fWeaponPosFrac);
}

void __fastcall PlayerCmd_isOnGround(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gclient_s *client; // r11
    bool v4; // r3
    unsigned __int16 v5; // [sp+84h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1103,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    client = v1->client;
    v4 = 1;
    if ((client->ps.eFlags & 0x300) == 0)
        v4 = client->ps.groundEntityNum != 2175;
    Scr_AddInt(v4);
}

void __fastcall PlayerCmd_SetViewmodel(scr_entref_t *entref)
{
    gentity_s *v1; // r30
    const char *v2; // r3
    const char *String; // r3
    const char *v4; // r31
    int v5; // r31
    unsigned __int16 v6; // [sp+94h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1135,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    v4 = String;
    if (!String || !*String)
        Scr_ParamError(0, "usage: setviewmodel(<model name>)");
    v5 = G_ModelIndex(v4);
    if (v5 != (unsigned __int16)v5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1144,
            0,
            "%s",
            "modelIndex == (modelNameIndex_t) modelIndex");
    v1->client->ps.viewmodelIndex = v5;
}

void __fastcall PlayerCmd_AllowADS(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int weapFlags; // r10
    unsigned __int16 v6; // [sp+84h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1163,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        Int = Scr_GetInt(0);
        client = v1->client;
        weapFlags = client->ps.weapFlags;
        if (Int)
        {
            client->ps.weapFlags = weapFlags & 0xFFFFFFDF;
        }
        else
        {
            client->ps.weapFlags = weapFlags | 0x20;
            PM_ExitAimDownSight(&v1->client->ps);
        }
    }
    else
    {
        Scr_Error("USAGE: <player> allowads( <boolean> )\n");
    }
}

void __fastcall PlayerCmd_AllowJump(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1196,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFFF7FFFF;
    else
        v6 = pm_flags | 0x80000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_AllowSprint(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1218,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFFFBFFFF;
    else
        v6 = pm_flags | 0x40000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_AllowMelee(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1240,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFDFFFFFF;
    else
        v6 = pm_flags | 0x2000000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_SetSpreadOverride(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    const char *v4; // r3
    unsigned __int16 v5; // [sp+84h] [+14h]

    v5 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1264,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v5];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v5);
            Scr_ObjectError(v2);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        Int = Scr_GetInt(0);
        if (Int > 0)
        {
            if (Int < 64)
            {
                v1->client->ps.spreadOverride = Int;
                v1->client->ps.spreadOverrideState = 2;
            }
            else
            {
                v4 = va("setspreadoverride: spread must be < %d", 64);
                Scr_ParamError(0, v4);
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

void __fastcall PlayerCmd_ResetSpreadOverride(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1302,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.spreadOverrideState = 1;
    v1->client->ps.aimSpreadScale = 255.0;
    if (Scr_GetNumParam())
        Scr_Error("USAGE: <player> resetspreadoverride()\n");
}

void __fastcall PlayerCmd_ShowViewmodel(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1324,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    SV_GameSendServerCommand(v1, "showViewModel");
}

void __fastcall PlayerCmd_HideViewmodel(scr_entref_t *entref)
{
    unsigned __int16 v1; // r29
    gentity_s *v2; // r31
    const char *v3; // r3
    playerState_s *p_ps; // r31
    unsigned int weaponstate; // r4

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v2 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1342,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v2 = &g_entities[v1];
        if (!v2->client)
        {
            v3 = va("entity %i is not a player", v1);
            Scr_ObjectError(v3);
        }
    }
    p_ps = &v2->client->ps;
    weaponstate = p_ps->weaponstate;
    if (weaponstate == 7 || weaponstate == 9 || weaponstate == 11 || weaponstate == 10 || weaponstate == 8)
        BG_AddPredictableEventToPlayerstate(2, weaponstate, p_ps);
    PM_ResetWeaponState(p_ps);
    SV_GameSendServerCommand(v1, "hideViewModel");
}

void __fastcall PlayerCmd_AllowStand(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1369,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFFEFFFFF;
    else
        v6 = pm_flags | 0x100000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_AllowCrouch(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1391,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFFDFFFFF;
    else
        v6 = pm_flags | 0x200000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_AllowProne(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1413,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFFBFFFFF;
    else
        v6 = pm_flags | 0x400000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_AllowLean(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int pm_flags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1435,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    pm_flags = client->ps.pm_flags;
    if (Int)
        v6 = pm_flags & 0xFF7FFFFF;
    else
        v6 = pm_flags | 0x800000;
    client->ps.pm_flags = v6;
}

void __fastcall PlayerCmd_OpenMenu(scr_entref_t *entref)
{
    unsigned __int16 v1; // r27
    gentity_s *v2; // r31
    const char *v3; // r3
    int v4; // r3
    const char *String; // r3
    unsigned int ScriptMenuIndex; // r31
    const char *v7; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v2 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1459,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v2 = &g_entities[v1];
        if (!v2->client)
        {
            v3 = va("entity %i is not a player", v1);
            Scr_ObjectError(v3);
        }
    }
    v4 = 0;
    if (v2->client->pers.connected == CON_CONNECTED)
    {
        String = Scr_GetString(0);
        ScriptMenuIndex = GScr_GetScriptMenuIndex(String);
        if (ScriptMenuIndex >= 0x20)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1465,
                0,
                "%s",
                "(iMenuIndex >= 0) && (iMenuIndex < MAX_SCRIPT_MENUS)");
        v7 = va("popupopen %i", ScriptMenuIndex);
        SV_GameSendServerCommand(v1, v7);
        v4 = 1;
    }
    Scr_AddInt(v4);
}

void __fastcall PlayerCmd_OpenMenuNoMouse(scr_entref_t *entref)
{
    unsigned __int16 v1; // r27
    gentity_s *v2; // r31
    const char *v3; // r3
    int v4; // r3
    const char *String; // r3
    unsigned int ScriptMenuIndex; // r31
    const char *v7; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v2 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1489,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v2 = &g_entities[v1];
        if (!v2->client)
        {
            v3 = va("entity %i is not a player", v1);
            Scr_ObjectError(v3);
        }
    }
    v4 = 0;
    if (v2->client->pers.connected == CON_CONNECTED)
    {
        String = Scr_GetString(0);
        ScriptMenuIndex = GScr_GetScriptMenuIndex(String);
        if (ScriptMenuIndex >= 0x20)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1495,
                0,
                "%s",
                "(iMenuIndex >= 0) && (iMenuIndex < MAX_SCRIPT_MENUS)");
        v7 = va("popupopen %i 1", ScriptMenuIndex);
        SV_GameSendServerCommand(v1, v7);
        v4 = 1;
    }
    Scr_AddInt(v4);
}

void __fastcall PlayerCmd_CloseMenu(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1517,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    SV_GameSendServerCommand(v1, "popupclose");
}

void __fastcall PlayerCmd_FreezeControls(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1536,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->bFrozen = Scr_GetInt(0);
}

void __fastcall PlayerCmd_SetEQLerp(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3
    unsigned int NumParam; // r31
    double Float; // fp1
    double v6; // fp31
    unsigned int Int; // r3
    const char *v8; // r3
    const char *v9; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1559,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    NumParam = Scr_GetNumParam();
    if (NumParam - 1 > 1)
    {
        v9 = "Incorrect number of parameters\n";
    }
    else
    {
        Float = Scr_GetFloat(0);
        v6 = Float;
        if (Float < 0.0 || Float > 1.0)
        {
            v9 = "eq lerp must be between 0 and 1";
        }
        else
        {
            if (NumParam != 2)
            {
            LABEL_15:
                v8 = va("eqLerp %g", v6);
                SV_GameSendServerCommand(v1, v8);
                return;
            }
            Int = Scr_GetInt(1u);
            if (Int <= 1)
            {
                if (Int == 1)
                    v6 = (float)((float)1.0 - (float)v6);
                goto LABEL_15;
            }
            v9 = va("eqIndex must be between 0 and %i", 2);
        }
    }
    Scr_Error(v9);
}

void __fastcall PlayerCmd_SetEQ(scr_entref_t *entref)
{
    unsigned __int16 v1; // r26
    const char *v2; // r3
    const char *String; // r29
    int Int; // r28
    int v5; // r27
    const char *v6; // r31
    double Float; // fp31
    const char *v8; // r3
    double v9; // [sp+48h] [-58h]

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
        goto LABEL_7;
    }
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1621,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v1].client)
    {
        v2 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v2);
    }
    if (Scr_GetNumParam() == 7)
    {
        String = Scr_GetString(0);
        Int = Scr_GetInt(1u);
        v5 = Scr_GetInt(2u);
        v6 = Scr_GetString(3u);
        if (I_stricmp(v6, "lowpass")
            && I_stricmp(v6, "highpass")
            && I_stricmp(v6, "lowshelf")
            && I_stricmp(v6, "highshelf"))
        {
            if (I_stricmp(v6, "bell"))
                Scr_Error("Unknown eq filter type\n");
        }
        Float = Scr_GetFloat(4u);
        Scr_GetFloat(5u);
        v9 = Scr_GetFloat(6u);
        v8 = va("eq \"%s\" %i %i %i %g %g %g", String, Int, v5, Float, v9);
        SV_GameSendServerCommand(v1, v8);
    }
    else
    {
        Scr_Error("Incorrect number of parameters\n");
    }
}

void __fastcall PlayerCmd_DeactivateEq(scr_entref_t *entref)
{
    unsigned __int16 v1; // r29
    const char *v2; // r3
    unsigned int NumParam; // r30
    int Int; // r3
    int v5; // r31
    const char *v6; // r3
    int v7; // r30
    const char *v8; // r3
    const char *String; // r3
    const char *v10; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
        goto LABEL_7;
    }
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1691,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v1].client)
    {
        v2 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v2);
    }
    NumParam = Scr_GetNumParam();
    if (NumParam - 1 > 2)
    {
        Scr_Error("Incorrect number of parameters\n");
    }
    else
    {
        Int = Scr_GetInt(0);
        v5 = Int;
        if (NumParam == 2)
        {
            String = Scr_GetString(1u);
            v10 = va("deactivateeq %i \"%s\"", v5, String);
            SV_GameSendServerCommand(v1, v10);
        }
        else
        {
            if (NumParam == 3)
            {
                v7 = Scr_GetInt(2u);
                v8 = Scr_GetString(1u);
                v6 = va("deactivateeq %i \"%s\" %i", v5, v8, v7);
            }
            else
            {
                v6 = va("deactivateeq %i", Int);
            }
            SV_GameSendServerCommand(v1, v6);
        }
    }
}

void __fastcall PlayerCmd_SetReverb(scr_entref_t *entref)
{
    unsigned __int16 v1; // r29
    const char *v2; // r3
    double Float; // fp29
    double v4; // fp31
    double v5; // fp30
    unsigned int ConstString; // r3
    int v7; // r31
    const char *v8; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
        goto LABEL_7;
    }
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1747,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v1].client)
    {
        v2 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v2);
    }
    Float = 0.0;
    v4 = 1.0;
    v5 = 0.5;
    switch (Scr_GetNumParam())
    {
    case 2u:
        goto LABEL_12;
    case 3u:
        goto LABEL_11;
    case 4u:
        goto LABEL_10;
    case 5u:
        Float = Scr_GetFloat(4u);
    LABEL_10:
        v5 = Scr_GetFloat(3u);
    LABEL_11:
        v4 = Scr_GetFloat(2u);
    LABEL_12:
        Scr_GetString(1u);
        ConstString = Scr_GetConstString(0);
        v7 = 1;
        if (ConstString != scr_const.snd_enveffectsprio_level)
        {
            if (ConstString == scr_const.snd_enveffectsprio_shellshock)
                v7 = 2;
            else
                Scr_Error("priority must be 'snd_enveffectsprio_level' or 'snd_enveffectsprio_shellshock'\n");
        }
        v8 = va("reverb %i \"%s\" %g %g %g", v7, v4, LODWORD(v5), LODWORD(Float));
        SV_GameSendServerCommand(v1, v8);
        break;
    default:
        Scr_Error("Incorrect number of parameters\n");
        break;
    }
}

void __fastcall PlayerCmd_DeactivateReverb(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3
    double Float; // fp31
    unsigned int NumParam; // r3
    unsigned int ConstString; // r3
    const char *v7; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1802,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    Float = 0.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error("Incorrect number of parameters\n");
            return;
        }
        Float = Scr_GetFloat(1u);
    }
    ConstString = Scr_GetConstString(0);
    if (ConstString != scr_const.snd_enveffectsprio_level && ConstString != scr_const.snd_enveffectsprio_shellshock)
        Scr_Error("priority must be 'snd_enveffectsprio_level' or 'snd_enveffectsprio_shellshock'\n");
    v7 = va("deactivatereverb %i %g", HIDWORD(Float), LODWORD(Float));
    SV_GameSendServerCommand(v1, v7);
}

void __fastcall PlayerCmd_SetChannelVolumes(scr_entref_t *entref)
{
    unsigned __int16 v1; // r29
    const char *v2; // r3
    double Float; // fp31
    unsigned int NumParam; // r3
    const char *String; // r3
    unsigned int ConstString; // r3
    int v7; // r31
    const char *v8; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
        goto LABEL_7;
    }
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1850,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v1].client)
    {
        v2 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v2);
    }
    Float = 0.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 2)
    {
        if (NumParam != 3)
        {
            Scr_Error("Incorrect number of parameters\n");
            return;
        }
        Float = Scr_GetFloat(2u);
    }
    String = Scr_GetString(1u);
    G_FindConfigstringIndex(String, 2535, 16, 0, 0);
    ConstString = Scr_GetConstString(0);
    v7 = 2;
    if (ConstString != scr_const.snd_channelvolprio_holdbreath)
    {
        if (ConstString == scr_const.snd_channelvolprio_pain)
        {
            v7 = 3;
        }
        else if (ConstString == scr_const.snd_channelvolprio_shellshock)
        {
            v7 = 4;
        }
        else
        {
            Scr_Error(
                "priority must be 'snd_channelvolprio_holdbreath', 'snd_channelvolprio_pain', or 'snd_channelvolprio_shellshock'\n");
        }
    }
    v8 = va("setchannelvol %i %i %g", v7, Float);
    SV_GameSendServerCommand(v1, v8);
}

void __fastcall PlayerCmd_DeactivateChannelVolumes(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3
    double Float; // fp31
    unsigned int NumParam; // r3
    unsigned int ConstString; // r3
    const char *v7; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            1900,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    Float = 0.0;
    NumParam = Scr_GetNumParam();
    if (NumParam != 1)
    {
        if (NumParam != 2)
        {
            Scr_Error("Incorrect number of parameters\n");
            return;
        }
        Float = Scr_GetFloat(1u);
    }
    ConstString = Scr_GetConstString(0);
    if (ConstString != scr_const.snd_channelvolprio_holdbreath
        && ConstString != scr_const.snd_channelvolprio_pain
        && ConstString != scr_const.snd_channelvolprio_shellshock)
    {
        Scr_Error(
            "priority must be 'snd_channelvolprio_holdbreath', 'snd_channelvolprio_pain', or 'snd_channelvolprio_shellshock'\n");
    }
    v7 = va("deactivatechannelvol %i %g", HIDWORD(Float), LODWORD(Float));
    SV_GameSendServerCommand(v1, v7);
}

void __fastcall ScrCmd_IsLookingAt(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gentity_s *v3; // r31
    int v4; // r3
    bool v5; // zf
    unsigned __int16 v6; // [sp+84h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1943,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    if (!EntHandle::isDefined(&v1->client->pLookatEnt)
        || (v3 = EntHandle::ent(&v1->client->pLookatEnt), v5 = v3 == Scr_GetEntity(0), v4 = 1, !v5))
    {
        v4 = 0;
    }
    Scr_AddInt(v4);
}

void __fastcall PlayerCmd_IsFiring(scr_entref_t *entref)
{
    gentity_s *v1; // r29
    const char *v2; // r3
    int weaponstate; // r11
    unsigned __int8 v4; // r11
    bool v5; // zf
    unsigned __int16 v6; // [sp+94h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 1964, 0, "%s", "pSelf");
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1963,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    if (!v1->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 1965, 0, "%s", "pSelf->client");
    weaponstate = v1->client->ps.weaponstate;
    if (weaponstate == 5
        || weaponstate == 6
        || weaponstate == 12
        || weaponstate == 13
        || (v5 = weaponstate != 14, v4 = 0, !v5))
    {
        v4 = 1;
    }
    Scr_AddBool(v4);
}

void __fastcall PlayerCmd_IsThrowingGrenade(scr_entref_t *entref)
{
    gentity_s *v1; // r29
    const char *v2; // r3
    int weaponstate; // r11
    unsigned __int8 v4; // r11
    bool v5; // zf
    unsigned __int16 v6; // [sp+94h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 1986, 0, "%s", "pSelf");
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                1985,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    if (!v1->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 1987, 0, "%s", "pSelf->client");
    weaponstate = v1->client->ps.weaponstate;
    if (weaponstate < 15 || (v5 = weaponstate <= 20, v4 = 1, !v5))
        v4 = 0;
    Scr_AddBool(v4);
}

void __fastcall PlayerCmd_IsMeleeing(scr_entref_t *entref)
{
    gentity_s *v1; // r29
    const char *v2; // r3
    int weaponstate; // r11
    unsigned __int8 v4; // r11
    bool v5; // zf
    unsigned __int16 v6; // [sp+94h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 2008, 0, "%s", "pSelf");
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2007,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    if (!v1->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 2009, 0, "%s", "pSelf->client");
    weaponstate = v1->client->ps.weaponstate;
    if (weaponstate == 12 || weaponstate == 13 || (v5 = weaponstate != 14, v4 = 0, !v5))
        v4 = 1;
    Scr_AddBool(v4);
}

void __fastcall ScrCmd_PlayLocalSound(scr_entref_t *entref)
{
    unsigned __int16 v1; // r28
    gentity_s *v2; // r30
    const char *v3; // r3
    const char *String; // r31
    const char *v5; // r3
    unsigned __int16 v6; // r31
    int Int; // r29
    unsigned int NumParam; // r3
    unsigned int ConstString; // r3
    const char *v10; // r3
    const char *v11; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v2 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2035,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v2 = &g_entities[v1];
        if (!v2->client)
        {
            v3 = va("entity %i is not a player", v1);
            Scr_ObjectError(v3);
        }
    }
    String = Scr_GetString(0);
    if (!Com_FindSoundAlias(String))
    {
        v5 = va("unknown sound alias '%s'", String);
        Scr_ParamError(0, v5);
    }
    v6 = G_SoundAliasIndexTransient(String);
    Int = 0;
    NumParam = Scr_GetNumParam();
    if (NumParam == 1)
    {
        v11 = va("ls %i", v6);
        SV_GameSendServerCommand(v1, v11);
    }
    else
    {
        if (NumParam != 2)
        {
            if (NumParam != 3)
            {
                Scr_Error("Sound Error");
                return;
            }
            Int = Scr_GetInt(2u);
        }
        ConstString = Scr_GetConstString(1u);
        G_RegisterSoundWait(v2, v6, ConstString, Int);
        v10 = va("ls %i %i", v6, v2->s.number);
        SV_GameSendServerCommand(v1, v10);
    }
}

void __fastcall ScrCmd_StopLocalSound(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3
    const char *String; // r31
    const char *v5; // r3
    int v6; // r31
    const char *v7; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2082,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    String = Scr_GetString(0);
    if (!Com_FindSoundAlias(String))
    {
        v5 = va("unknown sound alias '%s'", String);
        Scr_ParamError(0, v5);
    }
    v6 = G_SoundAliasIndexTransient(String);
    if (!v6)
        Scr_Error("Unknown soundalias.");
    v7 = va("ls_stop %i", v6);
    SV_GameSendServerCommand(v1, v7);
}

void __fastcall ScrCmd_SetAutoPickup(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2112,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->bDisableAutoPickup = Scr_GetInt(0) == 0;
}

void __fastcall PlayerCmd_SetWeaponAmmoClip(scr_entref_t *entref)
{
    gentity_s *v1; // r28
    const char *v2; // r3
    const char *String; // r29
    int Int; // r31
    int WeaponIndexForName; // r30
    int v6; // r29
    WeaponDef *WeaponDef; // r3
    unsigned __int16 v8; // [sp+94h] [+14h]

    v8 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2141,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v8];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v8);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    Int = Scr_GetInt(1u);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    v6 = BG_ClipForWeapon(WeaponIndexForName);
    if (v6)
    {
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        if (Int >= 0)
        {
            if (Int > WeaponDef->iClipSize)
                Int = WeaponDef->iClipSize;
        }
        else
        {
            Int = 0;
        }
        v1->client->ps.ammoclip[v6] = Int;
    }
}

void __fastcall PlayerCmd_SetWeaponAmmoStock(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const playerState_s *p_ps; // r28
    const char *String; // r30
    int Int; // r29
    int WeaponIndexForName; // r31
    WeaponDef *WeaponDef; // r30
    int v8; // r3
    int iClipSize; // r11
    int v10; // r30
    int AmmoPlayerMax; // r11
    unsigned __int16 v12; // [sp+94h] [+14h]

    v12 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2182,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v12];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v12);
            Scr_ObjectError(v2);
        }
    }
    p_ps = &v1->client->ps;
    String = Scr_GetString(0);
    Int = Scr_GetInt(1u);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
    if (BG_WeaponIsClipOnly(WeaponIndexForName))
    {
        v8 = BG_ClipForWeapon(WeaponIndexForName);
        if (v8)
        {
            iClipSize = WeaponDef->iClipSize;
            if (Int < iClipSize)
                iClipSize = Int;
            if (iClipSize <= 0)
                iClipSize = 0;
            p_ps->ammoclip[v8] = iClipSize;
        }
    }
    else
    {
        v10 = BG_AmmoForWeapon(WeaponIndexForName);
        if (v10)
        {
            AmmoPlayerMax = BG_GetAmmoPlayerMax(p_ps, WeaponIndexForName, 0);
            if (Int < AmmoPlayerMax)
                AmmoPlayerMax = Int;
            if (AmmoPlayerMax <= 0)
                AmmoPlayerMax = 0;
            p_ps->ammo[v10] = AmmoPlayerMax;
        }
    }
}

void __fastcall PlayerCmd_GetWeaponAmmoClip(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const char *String; // r30
    int WeaponIndexForName; // r29
    int v5; // r3
    unsigned __int16 v6; // [sp+84h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2231,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    v5 = BG_ClipForWeapon(WeaponIndexForName);
    Scr_AddInt(v1->client->ps.ammoclip[v5]);
}

void __fastcall PlayerCmd_GetWeaponAmmoStock(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    const char *String; // r29
    int WeaponIndexForName; // r30
    int v5; // r10
    unsigned __int16 v6; // [sp+84h] [+14h]

    v6 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2258,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v6];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v6);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    if (BG_WeaponIsClipOnly(WeaponIndexForName))
        v5 = BG_ClipForWeapon(WeaponIndexForName) + 205;
    else
        v5 = BG_AmmoForWeapon(WeaponIndexForName) + 77;
    Scr_AddInt(*(&v1->client->ps.commandTime + v5));
}

void __fastcall PlayerCmd_AnyAmmoForWeaponModes(scr_entref_t *entref)
{
    gentity_s *v1; // r30
    const char *v2; // r3
    const char *String; // r29
    int WeaponIndexForName; // r31
    int v5; // r29
    unsigned int altWeaponIndex; // r4
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2297,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    v5 = BG_WeaponAmmo(&v1->client->ps, WeaponIndexForName);
    altWeaponIndex = BG_GetWeaponDef(WeaponIndexForName)->altWeaponIndex;
    if (altWeaponIndex)
        v5 += BG_WeaponAmmo(&v1->client->ps, altWeaponIndex);
    Scr_AddInt(v5 != 0);
}

void __fastcall PlayerCmd_EnableHealthShield(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2328,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->invulnerableEnabled = Scr_GetInt(0) != 0;
}

void __fastcall PlayerCmd_SetClientDvar(scr_entref_t *entref)
{
    unsigned __int16 v1; // r29
    const char *v2; // r3
    const char *String; // r30
    unsigned int NumParam; // r3
    const char *v5; // r31
    const char *v6; // r3
    const char *v7; // r3
    char v8[1072]; // [sp+50h] [-430h] BYREF

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
        goto LABEL_7;
    }
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2356,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v1].client)
    {
        v2 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v2);
    }
    String = Scr_GetString(0);
    if (Scr_GetType(1u) == 3)
    {
        NumParam = Scr_GetNumParam();
        Scr_ConstructMessageString(1, NumParam - 1, "Client Dvar Value", v8, 0x400u);
        v5 = v8;
    }
    else
    {
        v5 = Scr_GetString(1u);
    }
    if (Dvar_IsValidName(String))
    {
        v7 = va("setclientdvar \"%s\" \"%s\"", String, v5);
        SV_GameSendServerCommand(v1, v7);
    }
    else
    {
        v6 = va("%s is an invalid dvar name", String);
        Scr_Error(v6);
    }
}

void __fastcall PlayerCmd_SetClientDvars(scr_entref_t *entref)
{
    unsigned __int16 v1; // r27
    const char *v2; // r3
    unsigned int v3; // r31
    const char *String; // r30
    const char *v5; // r29
    const char *v6; // r3
    const char *v7; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v2 = "not an entity";
        goto LABEL_7;
    }
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2399,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v1].client)
    {
        v2 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v2);
    }
    if ((Scr_GetNumParam() & 1) != 0)
        Scr_Error(
            "Not enough parameters to setclientdvar() - must be an even number of parameters (dvar, value, dvar, value, etc.)\n");
    v3 = 0;
    if (Scr_GetNumParam())
    {
        while (1)
        {
            String = Scr_GetString(v3);
            v5 = Scr_GetString(v3 + 1);
            if (!Dvar_IsValidName(String))
                break;
            v6 = va("setclientdvar \"%s\" \"%s\"", String, v5);
            SV_GameSendServerCommand(v1, v6);
            v3 += 2;
            if (v3 >= Scr_GetNumParam())
                return;
        }
        v7 = va("Dvar %s has an invalid dvar name", String);
        Scr_Error(v7);
    }
}

void __fastcall PlayerCmd_BeginLocationSelection(scr_entref_t *entref)
{
    gentity_s *v1; // r28
    const char *v2; // r3
    const char *String; // r3
    int LocSelIndex; // r3
    int v5; // r30
    long double v6; // fp2
    double Float; // fp31
    long double v8; // fp2
    unsigned int v9; // r31
    unsigned __int16 v10; // [sp+A4h] [+14h]

    v10 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2439,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v10];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v10);
            Scr_ObjectError(v2);
        }
    }
    if (!v1->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 2441, 0, "%s", "pSelf->client");
    String = Scr_GetString(0);
    LocSelIndex = GScr_GetLocSelIndex(String);
    v5 = LocSelIndex;
    if (LocSelIndex < 1 || LocSelIndex > 4)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2445,
            0,
            "locSelIndex not in [1, MAX_LOC_SEL_MTLS + 1]\n\t%i not in [%i, %i]",
            LocSelIndex,
            1,
            4);
    if (Scr_GetNumParam() < 2)
    {
        *(double *)&v6 = 0.15000001;
    }
    else
    {
        Float = Scr_GetFloat(1u);
        if (Float <= 0.0)
            Scr_ParamError(1u, "Radius of location selector must be greater than zero\n");
        if (level.compassMapWorldSize[1] <= 0.0)
            *(double *)&v6 = ClampFloat((float)((float)Float * (float)0.001), 0.0, 1.0);
        else
            *(double *)&v6 = ClampFloat((float)((float)Float / level.compassMapWorldSize[1]), 0.0, 1.0);
    }
    *(double *)&v6 = (float)((float)((float)*(double *)&v6 * (float)63.0) + (float)0.5);
    v8 = floor(v6);
    v9 = (int)(float)*(double *)&v8;
    if (v9 >= 0x40)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2468,
            0,
            "radiusBits doesn't index (1 << LOC_SEL_RADIUS_BITS)\n\t%i not in [0, %i)",
            (int)(float)*(double *)&v8,
            64);
    v1->client->ps.locationSelectionInfo = (4 * v9) | v5;
}

void __fastcall PlayerCmd_EndLocationSelection(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+94h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2486,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    if (!v1->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 2488, 0, "%s", "pSelf->client");
    v1->client->ps.locationSelectionInfo = 0;
}

void __fastcall PlayerCmd_WeaponLockStart(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gentity_s *Entity; // r3
    unsigned __int16 v4; // [sp+84h] [+14h]

    v4 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2507,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v4];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v4);
            Scr_ObjectError(v2);
        }
    }
    if (Scr_GetNumParam())
    {
        Entity = Scr_GetEntity(0);
        v1->client->ps.weapLockFlags |= 1u;
        v1->client->ps.weapLockFlags &= ~2u;
        v1->client->ps.weapLockedEntnum = Entity->s.number;
        v1->client->ps.weapLockFlags &= ~4u;
        v1->client->ps.weapLockFlags &= ~8u;
    }
    else
    {
        Scr_Error("Incorrect number of parameters.\n");
    }
}

void __fastcall PlayerCmd_WeaponLockFinalize(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    gentity_s *Entity; // r3
    bool v4; // r3
    gclient_s *client; // r11
    int weapLockFlags; // r10
    int v7; // r10
    unsigned __int16 v8; // [sp+84h] [+14h]

    v8 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2539,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v8];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v8);
            Scr_ObjectError(v2);
        }
    }
    if (Scr_GetNumParam())
    {
        Entity = Scr_GetEntity(0);
        v1->client->ps.weapLockFlags |= 1u;
        v1->client->ps.weapLockFlags |= 2u;
        v1->client->ps.weapLockedEntnum = Entity->s.number;
        v4 = G_TargetAttackProfileTop(Entity);
        client = v1->client;
        weapLockFlags = client->ps.weapLockFlags;
        if (v4)
            v7 = weapLockFlags | 4;
        else
            v7 = weapLockFlags | 8;
        client->ps.weapLockFlags = v7;
    }
    else
    {
        Scr_Error("Incorrect number of parameters.\n");
    }
}

void __fastcall PlayerCmd_WeaponLockFree(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2573,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.weapLockFlags &= ~1u;
    v1->client->ps.weapLockFlags &= ~2u;
    v1->client->ps.weapLockedEntnum = 2175;
    v1->client->ps.weapLockFlags &= ~4u;
    v1->client->ps.weapLockFlags &= ~8u;
}

void __fastcall PlayerCmd_WeaponLockTargetTooClose(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int weapLockFlags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2596,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    weapLockFlags = client->ps.weapLockFlags;
    if (Int)
        v6 = weapLockFlags | 0x10;
    else
        v6 = weapLockFlags & 0xFFFFFFEF;
    client->ps.weapLockFlags = v6;
}

void __fastcall PlayerCmd_WeaponLockNoClearance(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    int Int; // r3
    gclient_s *client; // r11
    int weapLockFlags; // r10
    unsigned int v6; // r10
    unsigned __int16 v7; // [sp+84h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2618,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    client = v1->client;
    weapLockFlags = client->ps.weapLockFlags;
    if (Int)
        v6 = weapLockFlags | 0x20;
    else
        v6 = weapLockFlags & 0xFFFFFFDF;
    client->ps.weapLockFlags = v6;
}

void __fastcall PlayerCmd_SetActionSlot(scr_entref_t *entref)
{
    gentity_s *v1; // r28
    const char *v2; // r3
    int Int; // r4
    int v4; // r29
    const char *String; // r31
    const char *v6; // r31
    int WeaponIndexForName; // r30
    const char *v8; // r3
    unsigned __int16 v9; // [sp+94h] [+14h]

    v9 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2648,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v9];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v9);
            Scr_ObjectError(v2);
        }
    }
    Int = Scr_GetInt(0);
    if ((unsigned int)(Int - 1) > 3)
    {
        v8 = va("Invalid slot (%i) given, expecting 1 - %i\n", Int, 4);
        Scr_Error(v8);
    }
    else
    {
        v4 = Int - 1;
        String = Scr_GetString(1u);
        if (I_stricmp(String, "weapon"))
        {
            if (I_stricmp(String, "altmode"))
            {
                if (I_stricmp(String, "nightvision"))
                {
                    if (I_stricmp(String, byte_82003CDD))
                        Scr_Error("Invalid option: expected \"weapon\", \"altweapon\", or \"nightvision\".\n");
                    else
                        v1->client->ps.actionSlotType[v4] = ACTIONSLOTTYPE_DONOTHING;
                }
                else
                {
                    v1->client->ps.actionSlotType[v4] = ACTIONSLOTTYPE_NIGHTVISION;
                }
            }
            else
            {
                v1->client->ps.actionSlotType[v4] = ACTIONSLOTTYPE_ALTWEAPONTOGGLE;
            }
        }
        else
        {
            v6 = Scr_GetString(2u);
            WeaponIndexForName = BG_GetWeaponIndexForName(v6, 0);
            Scr_VerifyWeaponIndex(WeaponIndexForName, v6);
            v1->client->ps.actionSlotType[v4] = ACTIONSLOTTYPE_SPECIFYWEAPON;
            v1->client->ps.actionSlotParam[v4].specifyWeapon.index = WeaponIndexForName;
        }
    }
}

void __fastcall PlayerCmd_DisableWeapons(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2703,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.weapFlags |= 0x80u;
}

void __fastcall PlayerCmd_EnableWeapons(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2720,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.weapFlags &= ~0x80u;
}

void __fastcall PlayerCmd_NightVisionForceOff(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2737,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.weapFlags &= ~0x40u;
}

void __fastcall PlayerCmd_GetWeaponsList(scr_entref_t *entref)
{
    gentity_s *v1; // r27
    const char *v2; // r3
    unsigned int NumWeapons; // r26
    unsigned int i; // r31
    gclient_s *client; // r30
    WeaponDef *WeaponDef; // r3
    unsigned __int16 v7; // [sp+A4h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2757,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    NumWeapons = BG_GetNumWeapons();
    Scr_MakeArray();
    for (i = 1; i < NumWeapons; ++i)
    {
        client = v1->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(client->ps.weapons, i, 16))
        {
            WeaponDef = BG_GetWeaponDef(i);
            Scr_AddString(WeaponDef->szInternalName);
            Scr_AddArray();
        }
    }
}

void __fastcall PlayerCmd_GetWeaponsListPrimaries(scr_entref_t *entref)
{
    gentity_s *v1; // r27
    const char *v2; // r3
    unsigned int NumWeapons; // r26
    unsigned int i; // r31
    gclient_s *client; // r30
    WeaponDef *WeaponDef; // r3
    unsigned __int16 v7; // [sp+A4h] [+14h]

    v7 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2791,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v7];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v7);
            Scr_ObjectError(v2);
        }
    }
    NumWeapons = BG_GetNumWeapons();
    Scr_MakeArray();
    for (i = 1; i < NumWeapons; ++i)
    {
        client = v1->client;
        if (!client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(client->ps.weapons, i, 16))
        {
            WeaponDef = BG_GetWeaponDef(i);
            if (WeaponDef->inventoryType == WEAPINVENTORY_PRIMARY)
            {
                Scr_AddString(WeaponDef->szInternalName);
                Scr_AddArray();
            }
        }
    }
}

void __fastcall PlayerCmd_EnableInvulnerability(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2825,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.otherFlags |= 1u;
}

void __fastcall PlayerCmd_DisableInvulnerability(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2842,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.otherFlags &= ~1u;
}

void __fastcall PlayerCmd_ForceViewmodelAnimation(scr_entref_t *entref)
{
    gentity_s *v1; // r30
    const char *v2; // r3
    const char *String; // r31
    int WeaponIndexForName; // r29
    const char *v5; // r31
    int v6; // r10
    const char *v7; // r3
    unsigned __int16 v8; // [sp+84h] [+14h]

    v8 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2866,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v8];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v8);
            Scr_ObjectError(v2);
        }
    }
    String = Scr_GetString(0);
    WeaponIndexForName = G_GetWeaponIndexForName(String);
    Scr_VerifyWeaponIndex(WeaponIndexForName, String);
    v5 = Scr_GetString(1u);
    if (!I_stricmp(v5, "reload"))
    {
        v6 = 7;
    LABEL_15:
        v1->client->ps.weapFlags |= 0x400u;
        v1->client->ps.forcedViewAnimWeaponIdx = WeaponIndexForName;
        v1->client->ps.forcedViewAnimWeaponState = v6;
        return;
    }
    if (!I_stricmp(v5, "fire"))
    {
        v6 = 5;
        goto LABEL_15;
    }
    if (!I_stricmp(v5, "NVG_up"))
    {
        v6 = 26;
        goto LABEL_15;
    }
    if (!I_stricmp(v5, "NVG_down"))
    {
        v6 = 25;
        goto LABEL_15;
    }
    v7 = va("Animation name \"%s\" is not supported by this function.\n", v5);
    Scr_Error(v7);
}

void __fastcall PlayerCmd_DisableTurretDismount(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2905,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.weapFlags |= 0x800u;
}

void __fastcall PlayerCmd_EnableTurretDismount(scr_entref_t *entref)
{
    gentity_s *v1; // r31
    const char *v2; // r3
    unsigned __int16 v3; // [sp+84h] [+14h]

    v3 = HIWORD(entref);
    if ((_WORD)entref)
    {
        Scr_ObjectError("not an entity");
        v1 = 0;
    }
    else
    {
        if (HIWORD(entref) >= 0x880u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
                2922,
                0,
                "%s",
                "entref.entnum < MAX_GENTITIES");
        v1 = &g_entities[v3];
        if (!v1->client)
        {
            v2 = va("entity %i is not a player", v3);
            Scr_ObjectError(v2);
        }
    }
    v1->client->ps.weapFlags &= ~0x800u;
}

void __fastcall PlayerCmd_UploadScore(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3
    int Int; // r31
    int v5; // r3
    const char *v6; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2942,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    if (Scr_GetNumParam() == 2)
    {
        Int = Scr_GetInt(0);
        v5 = Scr_GetInt(1u);
        v6 = va("upscore %i %i", Int, v5);
        SV_GameSendServerCommand(v1, v6);
    }
    else
    {
        Scr_Error("Incorrect number of parameters\n");
    }
}

void __fastcall PlayerCmd_UploadTime(scr_entref_t *entref)
{
    unsigned __int16 v1; // r30
    int v2; // r31
    const char *v3; // r3
    int Int; // r31
    double Float; // fp1
    const char *v6; // r3

    v1 = HIWORD(entref);
    if ((_WORD)entref)
    {
        v3 = "not an entity";
        goto LABEL_7;
    }
    v2 = HIWORD(entref);
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp",
            2978,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    if (!g_entities[v2].client)
    {
        v3 = va("entity %i is not a player", v1);
    LABEL_7:
        Scr_ObjectError(v3);
    }
    if (Scr_GetNumParam() == 2)
    {
        Int = Scr_GetInt(0);
        Float = Scr_GetFloat(1u);
        if (Float >= 0.0)
        {
            v6 = va("upscore %i %i", Int, (int)(float)((float)Float * (float)-100.0));
            SV_GameSendServerCommand(v1, v6);
        }
        else
        {
            Scr_Error("Negative time is invalid\n");
        }
    }
    else
    {
        Scr_Error("Incorrect number of parameters\n");
    }
}

void(__fastcall *__fastcall Player_GetMethod(const char **pName))(scr_entref_t)
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinMethodDef *i; // r7
    const char *actionString; // r10
    _BYTE *v5; // r11
    int v6; // r8

    v1 = 0;
    v2 = 0;
    for (i = methods_0; ; ++i)
    {
        actionString = i->actionString;
        v5 = *pName;
        do
        {
            v6 = (unsigned __int8)*v5 - *(unsigned __int8 *)actionString;
            if (!*v5)
                break;
            ++v5;
            ++actionString;
        } while (!v6);
        if (!v6)
            break;
        v2 += 12;
        ++v1;
        if (v2 >= 0x42C)
            return 0;
    }
    *pName = methods_0[v1].actionString;
    return methods_0[v1].actionFunc;
}

void __fastcall G_AddCommandNotify(volatile unsigned __int16 notify)
{
    int nesting; // r7
    int v3; // r29
    unsigned int v4; // r30
    const char *v5; // r3
    unsigned int String; // r7
    volatile unsigned __int16 v7; // r28

    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 1012, 0, "%s", "Sys_IsMainThread()");
    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    v3 = cmd_args.argc[nesting];
    if (v3 > 10)
        v3 = 10;
    while (v3 - s_cmdNotify.read + s_cmdNotify.write + 2 > 0x40)
    {
        if (!alwaysfails)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client_script_cmd.cpp", 1022, 1, "inconceivable");
        __lwsync();
    }
    *(volatile unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * s_cmdNotify.write) & 0x7E)) = notify;
    if (v3 != (unsigned __int16)v3)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            v3,
            (unsigned __int16)v3);
    v4 = 0;
    for (*(volatile unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * (s_cmdNotify.write + 1)) & 0x7E)) = v3;
        v4 < v3;
        ++v4)
    {
        v5 = Cmd_Argv(v4);
        String = SL_GetString(v5, 0);
        v7 = String;
        if (String != (unsigned __int16)String)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                String,
                (unsigned __int16)String);
        *(volatile unsigned __int16 *)((char *)s_cmdNotify.data + ((2 * (s_cmdNotify.write + v4 + 2)) & 0x7E)) = v7;
    }
    __lwsync();
    s_cmdNotify.write += v3 + 2;
}

