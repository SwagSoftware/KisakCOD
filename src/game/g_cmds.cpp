#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include "g_main.h"
#include <server/sv_game.h>
#include <qcommon/cmd.h>
#include "actor_corpse.h"

#include <xanim/xanim.h>
#include <script/scr_vm.h>
#include <server/sv_public.h>
#include <script/scr_const.h>
#include "actor_fields.h"

int g_allowRemoveCorpse;
bool g_godModeRemoteInputValid;

int __cdecl CheatsOkInternal(gentity_s *ent)
{
    const char *v2; // r3
    const char *v4; // r3

    if (g_cheats->current.enabled)
    {
        if (ent->health > 0)
        {
            return 1;
        }
        else
        {
            v4 = va("print \"GAME_MUSTBEALIVECOMMAND\"");
            SV_GameSendServerCommand(ent - g_entities, v4);
            return 0;
        }
    }
    else
    {
        v2 = va("print \"GAME_CHEATSNOTENABLED\"");
        SV_GameSendServerCommand(ent - g_entities, v2);
        return 0;
    }
}

int __cdecl CheatsOk(gentity_s *ent)
{
    int ok; // r31

    if (level.demoplaying)
        return SV_DemoCheatsOk();
    ok = CheatsOkInternal(ent);
    SV_RecordCheatsOk(ok);
    return ok;
}

char line[1024];
char *__cdecl ConcatArgs(int start)
{
    size_t v2; // r30
    int nesting; // r7
    int i; // r27
    char *v5; // r11
    size_t v7; // r5
    size_t v8; // r31
    char *result; // r3
    char v10[1088]; // [sp+50h] [-440h] BYREF

    v2 = 0;
    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    for (i = sv_cmd_args.argc[nesting]; start < i; ++start)
    {
        SV_Cmd_ArgvBuffer(start, v10, 1024);
        v5 = v10;
        while (*v5++)
            ;
        v7 = v5 - v10 - 1;
        v8 = v7 + v2;
        if ((int)(v7 + v2) >= 1023)
            break;
        memcpy(&line[v2], v10, v7);
        v2 = v8;
        if (start != i - 1)
        {
            line[v8] = 32;
            v2 = v8 + 1;
        }
    }
    result = line;
    line[v2] = 0;
    return result;
}

void __cdecl SanitizeString(char *in, char *out)
{
    char *v2; // r31
    char v4; // r11
    int v5; // r3

    v2 = in;
    v4 = *in;
    v5 = *in;
    if (v4)
    {
        do
        {
            if (v5 == 27)
            {
                v2 += 2;
            }
            else
            {
                if (v5 >= 32)
                    *out++ = tolower(v5);
                ++v2;
            }
            v5 = *v2;
        } while (*v2);
    }
    *out = 0;
}

void __cdecl G_setfog(const char *fogstring)
{
    float fDensity; // [esp+0h] [ebp-1Ch] BYREF
    float clr[3]; // [esp+4h] [ebp-18h] BYREF
    float fFar; // [esp+10h] [ebp-Ch] BYREF
    float fNear; // [esp+14h] [ebp-8h] BYREF
    int32_t time; // [esp+18h] [ebp-4h] BYREF

    SV_GameSendServerCommand(-1, va("fog %s", fogstring));

    level.fFogOpaqueDist = FLT_MAX;
    level.fFogOpaqueDistSqrd = FLT_MAX;

    if (sscanf(fogstring, "%f %f %f %f %f %f %i", &fNear, &fFar, &fDensity, clr, &clr[1], &clr[2], &time) == 7
        && fDensity >= 1.0)
    {
        level.fFogOpaqueDist = (fFar - fNear) * 1.0 + fNear;
        level.fFogOpaqueDistSqrd = level.fFogOpaqueDist * level.fFogOpaqueDist;
    }
}

void __cdecl Cmd_Fogswitch_f()
{
    const char *v0; // r3

    v0 = ConcatArgs(1);
    G_setfog(v0);
}

void __cdecl Cmd_SetSoundLength_f()
{
    int nesting; // r7
    int v1; // r5
    unsigned int v2; // r31
    int v3; // r3
    gentity_s *v4; // r9
    char v5[256]; // [sp+50h] [-110h] BYREF

    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] == 3)
    {
        SV_Cmd_ArgvBuffer(1, v5, 256);
        v2 = atol(v5);
        SV_Cmd_ArgvBuffer(2, v5, 256);
        v3 = atol(v5);
        if (v2 > 0x87F)
        {
            Com_Error(ERR_DROP, "client command received with entnum %i", v2);
        }
        else
        {
            v4 = &level.gentities[v2];
            if (v4->snd_wait.duration < v3)
            {
                v4->snd_wait.duration = v3;
                level.gentities[v2].snd_wait.basetime = level.time;
            }
        }
    }
    else
    {
        v1 = SV_Cmd_Argc();
        Com_Error(ERR_DROP, "client command received with %i parameters instead of 2", v1);
    }
}

void __cdecl Cmd_RemoveCorpse_f()
{
    G_GetFreeActorCorpseIndex(0);
}

void __cdecl Cmd_Give_f(gentity_s *ent)
{
    int ok; // r31
    int v3; // r3
    const char *v4; // r3
    int v5; // r27
    const char *v6; // r3
    const char *v7; // r30
    const char *v8; // r11
    int v10; // r28
    int v11; // r11
    unsigned int i; // r31
    unsigned int weapon; // r4
    unsigned int k; // r31
    unsigned int j; // r31
    const gitem_s *Item; // r30
    gentity_s *v17; // r31
    OffhandClass offhandClass; // r11
    int inuse; // r11

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (!v3)
        return;
    v4 = ConcatArgs(2);
    v5 = atol(v4);
    v6 = ConcatArgs(1);
    v7 = v6;
    if (!v6)
        return;
    v8 = v6;
    while (*(unsigned __int8 *)v8++)
        ;
    if (v8 - v6 == 1)
        return;
    if (!I_stricmp(v6, "all"))
    {
        v10 = 1;
        goto LABEL_12;
    }
    v10 = 0;
    if (!I_strnicmp(v7, "health", 6))
    {
    LABEL_12:
        if (v5)
        {
            v11 = ent->health + v5;
        }
        else
        {
            ent->client->ps.stats[2] = g_player_maxhealth->current.integer;
            v11 = ent->client->ps.stats[2];
        }
        ent->health = v11;
        if (!v10)
            return;
    LABEL_18:
        for (i = 1; i < BG_GetNumWeapons(); ++i)
        {
            if (BG_CanPlayerHaveWeapon(i))
            {
                BG_TakePlayerWeapon(&ent->client->ps, i, 1);
                G_GivePlayerWeapon(&ent->client->ps, i, 0);
            }
        }
        if (!v10)
            return;
        goto LABEL_25;
    }
    if (!I_stricmp(v7, "weapons"))
        goto LABEL_18;
    if (I_strnicmp(v7, "ammo", 4))
    {
    LABEL_31:
        if (I_strnicmp(v7, "allammo", 7) || !v5)
        {
            if (!v10)
            {
                level.initializing = 1;
                Item = G_FindItem(v7, 0);
                if (Item)
                {
                    v17 = G_Spawn();
                    v17->r.currentOrigin[0] = ent->r.currentOrigin[0];
                    v17->r.currentOrigin[1] = ent->r.currentOrigin[1];
                    v17->r.currentOrigin[2] = ent->r.currentOrigin[2];
                    G_GetItemClassname(Item, &v17->classname);
                    G_SpawnItem(v17, Item);
                    v17->active = 1;
                    if (Item->giType == IT_WEAPON)
                    {
                        offhandClass = BG_GetWeaponDef(v17->item[0].index % 128)->offhandClass;
                        if (offhandClass == OFFHAND_CLASS_FLASH_GRENADE)
                        {
                            ent->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_FLASH;
                        }
                        else if (offhandClass == OFFHAND_CLASS_SMOKE_GRENADE)
                        {
                            ent->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_SMOKE;
                        }
                    }
                    Touch_Item(v17, ent, 0);
                    inuse = v17->r.inuse;
                    v17->active = 0;
                    if (inuse)
                        G_FreeEntity(v17);
                }
                level.initializing = 0;
            }
        }
        else
        {
            for (j = 1; j < BG_GetNumWeapons(); ++j)
                Add_Ammo(ent, j, 0, v5, 1);
        }
        return;
    }
LABEL_25:
    if (v5)
    {
        weapon = ent->client->ps.weapon;
        if (weapon)
            Add_Ammo(ent, weapon, 0, v5, 1);
    }
    else
    {
        for (k = 1; k < BG_GetNumWeapons(); ++k)
            Add_Ammo(ent, k, 0, 998, 1);
    }
    if (v10)
        goto LABEL_31;
}

void __cdecl Cmd_Take_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    const char *v4; // r3
    int v5; // r23
    const char *v6; // r3
    const char *v7; // r25
    const char *v8; // r11
    unsigned int v10; // r30
    int v11; // r24
    int v12; // r11
    unsigned int i; // r29
    gclient_s *client; // r10
    gclient_s *v15; // r11
    int weapon; // r29
    int v17; // r3
    int v18; // r3
    gclient_s *v19; // r27
    unsigned int j; // r29
    int v21; // r3
    int v22; // r3
    int v23; // r3
    gclient_s *v24; // r28

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (!v3)
        return;
    v4 = ConcatArgs(2);
    v5 = atol(v4);
    v6 = ConcatArgs(1);
    v7 = v6;
    if (!v6)
        return;
    v8 = v6;
    while (*(unsigned __int8 *)v8++)
        ;
    if (v8 - v6 == 1)
        return;
    v10 = 1;
    if (!I_stricmp(v6, "all"))
    {
        v11 = 1;
    LABEL_12:
        if (!v5 || (v12 = ent->health - v5, ent->health = v12, v12 < 1))
            ent->health = 1;
        if (!v11)
            return;
        goto LABEL_18;
    }
    v11 = 0;
    if (!I_strnicmp(v7, "health", 6))
        goto LABEL_12;
    if (!I_stricmp(v7, "weapons"))
    {
    LABEL_18:
        for (i = 1; i < BG_GetNumWeapons(); ++i)
            BG_TakePlayerWeapon(&ent->client->ps, i, 1);
        client = ent->client;
        if (client->ps.weapon)
        {
            client->ps.weapon = 0;
            G_SelectWeaponIndex(ent - g_entities, 0);
        }
        if (!v11)
            return;
        goto LABEL_25;
    }
    if (I_strnicmp(v7, "ammo", 4))
    {
    LABEL_33:
        if (!I_strnicmp(v7, "allammo", 7) && v5 && BG_GetNumWeapons() > 1)
        {
            do
            {
                v22 = BG_AmmoForWeapon(v10);
                ent->client->ps.ammo[v22] -= v5;
                if (ent->client->ps.ammo[BG_AmmoForWeapon(v10)] < 0)
                {
                    v23 = BG_ClipForWeapon(v10);
                    v24 = ent->client;
                    v24->ps.ammoclip[v23] += v24->ps.ammo[BG_AmmoForWeapon(v10)];
                    ent->client->ps.ammo[BG_AmmoForWeapon(v10)] = 0;
                    if (ent->client->ps.ammoclip[BG_ClipForWeapon(v10)] < 0)
                        ent->client->ps.ammoclip[BG_ClipForWeapon(v10)] = 0;
                }
                ++v10;
            } while (v10 < BG_GetNumWeapons());
        }
        return;
    }
LABEL_25:
    if (v5)
    {
        v15 = ent->client;
        weapon = v15->ps.weapon;
        if (weapon)
        {
            v17 = BG_AmmoForWeapon(v15->ps.weapon);
            ent->client->ps.ammo[v17] -= v5;
            if (ent->client->ps.ammo[BG_AmmoForWeapon(weapon)] < 0)
            {
                v18 = BG_ClipForWeapon(weapon);
                v19 = ent->client;
                v19->ps.ammoclip[v18] += v19->ps.ammo[BG_AmmoForWeapon(weapon)];
                ent->client->ps.ammo[BG_AmmoForWeapon(weapon)] = 0;
                if (ent->client->ps.ammoclip[BG_ClipForWeapon(weapon)] < 0)
                    ent->client->ps.ammoclip[BG_ClipForWeapon(weapon)] = 0;
            }
        }
    }
    else
    {
        for (j = 1; j < BG_GetNumWeapons(); ent->client->ps.ammoclip[v21] = 0)
        {
            ent->client->ps.ammo[BG_AmmoForWeapon(j)] = 0;
            v21 = BG_ClipForWeapon(j++);
        }
    }
    if (v11)
        goto LABEL_33;
}

void __cdecl Cmd_God_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    gentityFlags_t v4; // r11
    const char *v5; // r4

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        v4 = ent->flags ^ FL_GODMODE;
        ent->flags = v4;
        if ((v4 & FL_GODMODE) != 0)
            v5 = "GAME_GODMODE_ON";
        else
            v5 = "GAME_GODMODE_OFF";
        SV_GameSendServerCommand(ent - g_entities, va("print \"%s\"", v5));
        g_godModeRemoteInputValid = HIBYTE(ent->flags) & 1;
    }
}

void __cdecl Cmd_DemiGod_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    gentityFlags_t v4; // r11
    const char *v5; // r4

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        v4 = ent->flags ^ FL_DEMI_GODMODE;
        ent->flags = v4;
        if ((v4 & FL_DEMI_GODMODE) != 0)
            v5 = "GAME_DEMI_GODMODE_ON";
        else
            v5 = "GAME_DEMI_GODMODE_OFF";
        SV_GameSendServerCommand(ent - g_entities, va("print \"%s\"", v5));
    }
}

void __cdecl Cmd_Notarget_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    gentityFlags_t v4; // r11
    const char *v5; // r4

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        v4 = ent->flags ^ FL_NOTARGET;
        ent->flags = v4;
        if ((v4 & 4) != 0)
            v5 = "GAME_NOTARGETON";
        else
            v5 = "GAME_NOTARGETOFF";
        SV_GameSendServerCommand(ent - g_entities, va("print \"%s\"", v5));
    }
}

void __cdecl Cmd_Noclip_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    int *p_noclip; // r10
    const char *v5; // r4
    const char *v6; // r3

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        p_noclip = &ent->client->noclip;
        if (*p_noclip)
            v5 = "GAME_NOCLIPOFF";
        else
            v5 = "GAME_NOCLIPON";
        *p_noclip = *p_noclip == 0;
        v6 = va("print \"%s\"", v5);
        SV_GameSendServerCommand(ent - g_entities, v6);
    }
}

void __cdecl Cmd_UFO_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    int *p_ufo; // r10
    const char *v5; // r4
    const char *v6; // r3

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        p_ufo = &ent->client->ufo;
        if (*p_ufo)
            v5 = "GAME_UFOOFF";
        else
            v5 = "GAME_UFOON";
        *p_ufo = *p_ufo == 0;
        v6 = va("print \"%s\"", v5);
        SV_GameSendServerCommand(ent - g_entities, v6);
    }
}

void __cdecl Cmd_Kill_f(gentity_s *ent)
{
    gclient_s *client; // r5
    gentityFlags_t v2; // r6

    if (!g_reloading->current.integer)
    {
        client = ent->client;
        v2 = ent->flags & ~(FL_GODMODE | FL_DEMI_GODMODE);
        ent->health = 0;
        ent->flags = v2;
        client->ps.stats[0] = 0;
        player_die(ent, ent, ent, 100000, 12, 0, 0, HITLOC_NONE);
    }
}

void __cdecl Cmd_Where_f(gentity_s *ent)
{
    gclient_s *client; // r11
    char *v3; // r3
    const char *v4; // r3

    client = ent->client;
    if (client)
    {
        v3 = vtos(client->ps.origin);
        v4 = va("print \"%s\"", v3);
        SV_GameSendServerCommand(ent - g_entities, v4);
    }
}

const char *aPrintGameUsage_0 = "print \"GAME_USAGE\x15: setviewpos x y z [yaw] [pitch]\"";
void __cdecl Cmd_SetViewpos_f(gentity_s *ent)
{
    int ok; // r31
    int v3; // r3
    const char *v4; // r4
    int v5; // r11
    float *v6; // r31
    int v7; // r30
    long double v8; // fp2
    int v9; // r3
    long double v10; // fp2
    long double v11; // fp2
    float v12; // [sp+50h] [-440h] BYREF
    float v13; // [sp+54h] [-43Ch]
    float v14; // [sp+58h] [-438h]
    float v15[2]; // [sp+60h] [-430h] BYREF
    float v16; // [sp+68h] [-428h]
    char v17[1056]; // [sp+70h] [-420h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_cmds.cpp", 636, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_cmds.cpp", 637, 0, "%s", "ent->client");
    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        if (SV_Cmd_Argc() < 4 || SV_Cmd_Argc() > 6)
        {
            v4 = aPrintGameUsage_0;
            goto LABEL_20;
        }
        v5 = 0;
        v6 = v15;
        do
        {
            v7 = v5 + 1;
            SV_Cmd_ArgvBuffer(v5 + 1, v17, 1024);
            v8 = atof(v17);
            v5 = v7;
            *v6++ = *(double *)&v8;
        } while (v7 < 3);
        v16 = v16 - ent->client->ps.viewHeightCurrent;
        v12 = 0.0;
        v13 = 0.0;
        v14 = 0.0;
        v9 = SV_Cmd_Argc();
        if (v9 != 5)
        {
            if (v9 != 6)
            {
            LABEL_18:
                TeleportPlayer(ent, v15, &v12);
                return;
            }
            SV_Cmd_ArgvBuffer(5, v17, 1024);
            v10 = atof(v17);
            v12 = *(double *)&v10;
        }
        SV_Cmd_ArgvBuffer(4, v17, 1024);
        v11 = atof(v17);
        v13 = *(double *)&v11;
        goto LABEL_18;
    }
    v4 = "print \"GAME_CHEATSNOTENABLED\"";
LABEL_20:
    SV_GameSendServerCommand(ent - g_entities, v4);
}

const char *aPrintGameUsage = "print \"GAME_USAGE\x15: jumptonode nodenum\n\"";
const char *aPrint = "print \"\x15nodenum must be >= 0 and < %i\n\"";

void __cdecl Cmd_JumpToNode_f(gentity_s *ent)
{
    int ok; // r30
    int v3; // r3
    const char *v4; // r4
    int v5; // r11
    int v6; // r30
    pathnode_t *v7; // r3
    int v8; // r3
    float v9[4]; // [sp+50h] [-430h] BYREF
    char v10[1032]; // [sp+60h] [-420h] BYREF

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(ent);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        if (SV_Cmd_Argc() == 2)
        {
            SV_Cmd_ArgvBuffer(1, v10, 1024);
            v6 = atol(v10);
            if (v6 >= 0 && v6 < Path_NodeCount())
            {
                v7 = Path_ConvertIndexToNode(v6);
                v9[0] = 0.0;
                v9[1] = v7->constant.fAngle;
                v9[2] = 0.0;
                TeleportPlayer(ent, v7->constant.vOrigin, v9);
                return;
            }
            v8 = Path_NodeCount();
            v4 = va(aPrint, v8);
            v5 = (unsigned __int64)(875407347LL * ((char *)ent - (char *)g_entities)) >> 32;
        }
        else
        {
            v4 = aPrintGameUsage;
            v5 = (unsigned __int64)(875407347LL * ((char *)ent - (char *)g_entities)) >> 32;
        }
    }
    else
    {
        v4 = "print \"GAME_CHEATSNOTENABLED\"";
        v5 = (unsigned __int64)(875407347LL * ((char *)ent - (char *)g_entities)) >> 32;
    }
    SV_GameSendServerCommand((v5 >> 7) + ((unsigned int)v5 >> 31), v4);
}

void __cdecl Cmd_InterruptCamera_f(gentity_s *ent)
{
    ;
}

void __cdecl Cmd_DropWeapon_f(gentity_s *pSelf)
{
    int ok; // r31
    int v3; // r3

    if (level.demoplaying)
    {
        v3 = SV_DemoCheatsOk();
    }
    else
    {
        ok = CheatsOkInternal(pSelf);
        SV_RecordCheatsOk(ok);
        v3 = ok;
    }
    if (v3)
    {
        if (Drop_Weapon(pSelf, pSelf->s.weapon, pSelf->s.weaponModel, 0))
            G_AddEvent(pSelf, 11, 0);
    }
}

void __cdecl Cmd_MenuResponse_f(gentity_s *pEnt)
{
    int nesting; // r7
    unsigned int v3; // r3
    char v4[1024]; // [sp+50h] [-820h] BYREF
    char v5[1032]; // [sp+450h] [-420h] BYREF

    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] == 3)
    {
        SV_Cmd_ArgvBuffer(1, v4, 1024);
        v3 = atol(v4);
        if (v3 <= 0x1F)
            SV_GetConfigstring(v3 + 2551, v4, 1024);
        SV_Cmd_ArgvBuffer(2, v5, 1024);
    }
    else
    {
        v4[0] = 0;
        strcpy(v5, "bad");
    }
    Scr_AddString(v5);
    Scr_AddString(v4);
    Scr_Notify(pEnt, scr_const.menuresponse, 2u);
}

// attributes: thunk
void __cdecl Cmd_PrintEntities_f()
{
    G_PrintEntities();
}

void Cmd_VisionSetNaked_f()
{
    unsigned int nesting; // r7
    int v1; // r11
    const char *v2; // r3
    long double v3; // fp2
    long double v4; // fp2
    const char *v5; // r4
    const char *v6; // r3
    int v7; // [sp+50h] [-30h]

    v7 = 1000;
    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    v1 = sv_cmd_args.argc[nesting];
    if (v1 != 2)
    {
        if (v1 != 3)
        {
            Com_Printf(0, "USAGE: visionSetNaked <name> <duration>\n");
            return;
        }
        v2 = SV_Cmd_Argv(2);
        v3 = atof(v2);
        *(double *)&v3 = (float)((float)((float)*(double *)&v3 * (float)1000.0) + (float)0.5);
        v4 = floor(v3);
        nesting = sv_cmd_args.nesting;
        v7 = (int)(float)*(double *)&v4;
    }
    if (nesting >= 8)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            182,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] <= 1)
        v5 = "";
    else
        v5 = (const char *)*((unsigned int *)sv_cmd_args.argv[nesting] + 1);
    v6 = va("\"%s\" %i", v5, v7);
    SV_SetConfigstring(1149, v6);
}

void Cmd_VisionSetNight_f()
{
    unsigned int nesting; // r7
    int v1; // r11
    const char *v2; // r3
    long double v3; // fp2
    long double v4; // fp2
    const char *v5; // r4
    const char *v6; // r3
    int v7; // [sp+50h] [-30h]

    v7 = 1000;
    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    v1 = sv_cmd_args.argc[nesting];
    if (v1 != 2)
    {
        if (v1 != 3)
        {
            Com_Printf(0, "USAGE: visionSetNight <name> <duration>\n");
            return;
        }
        v2 = SV_Cmd_Argv(2);
        v3 = atof(v2);
        *(double *)&v3 = (float)((float)((float)*(double *)&v3 * (float)1000.0) + (float)0.5);
        v4 = floor(v3);
        nesting = sv_cmd_args.nesting;
        v7 = (int)(float)*(double *)&v4;
    }
    if (nesting >= 8)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            182,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] <= 1)
        v5 = "";
    else
        v5 = (const char *)*((unsigned int *)sv_cmd_args.argv[nesting] + 1);
    v6 = va("\"%s\" %i", v5, v7);
    SV_SetConfigstring(1150, v6);
}

void __cdecl ClientCommand(int clientNum, const char *s)
{
    gentity_s *v3; // r30
    const char *v5; // r3
    const char *v6; // r3
    char v7[1056]; // [sp+50h] [-420h] BYREF

    v3 = &g_entities[clientNum];
    g_allowRemoveCorpse = 1;
    if (!v3->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_cmds.cpp", 883, 0, "%s", "ent->client");
    SV_Cmd_TokenizeString((char*)s);
    SV_Cmd_ArgvBuffer(0, v7, 1024);
    if (!I_stricmp(v7, "fogswitch"))
    {
        v5 = ConcatArgs(1);
        G_setfog(v5);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "mr"))
    {
        Cmd_MenuResponse_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "sl"))
    {
        Cmd_SetSoundLength_f();
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "removecorpse"))
    {
        G_GetFreeActorCorpseIndex(0);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "give"))
    {
        Cmd_Give_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "take"))
    {
        Cmd_Take_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "god"))
    {
        Cmd_God_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "demigod"))
    {
        Cmd_DemiGod_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "notarget"))
    {
        Cmd_Notarget_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "noclip"))
    {
        Cmd_Noclip_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "ufo"))
    {
        Cmd_UFO_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "kill"))
    {
        Cmd_Kill_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (!I_stricmp(v7, "where"))
    {
        Cmd_Where_f(v3);
        SV_Cmd_EndTokenizedString();
        return;
    }
    if (I_stricmp(v7, "cameraInterrupt"))
    {
        if (!I_stricmp(v7, "setviewpos"))
        {
            Cmd_SetViewpos_f(v3);
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "jumptonode"))
        {
            Cmd_JumpToNode_f(v3);
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "ai") && CheatsOk(v3))
        {
            Cmd_AI_f();
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "echo"))
        {
            Cmd_Echo_f();
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "printentities"))
        {
            G_PrintEntities();
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "visionsetnaked"))
        {
            Cmd_VisionSetNaked_f();
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "visionsetnight"))
        {
            Cmd_VisionSetNight_f();
            SV_Cmd_EndTokenizedString();
            return;
        }
        if (!I_stricmp(v7, "dropweapon"))
        {
            Cmd_DropWeapon_f(v3);
            SV_Cmd_EndTokenizedString();
            return;
        }
        v6 = va("print \"GAME_UNKNOWNCLIENTCOMMAND %s\"", v7);
        SV_GameSendServerCommand(clientNum, v6);
    }
    SV_Cmd_EndTokenizedString();
}

