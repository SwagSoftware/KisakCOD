#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"
#include <server/sv_game.h>
#include <qcommon/cmd.h>
#include "g_utils_mp.h"
#include <script/scr_vm.h>


void __cdecl SendScoreboard(gentity_s *ent)
{
    int32_t v1; // eax
    int32_t ping; // [esp+0h] [ebp-B3Ch]
    gclient_s *v3; // [esp+4h] [ebp-B38h]
    int32_t msgLen; // [esp+8h] [ebp-B34h]
    char entry[1432]; // [esp+Ch] [ebp-B30h] BYREF
    int32_t entryLen; // [esp+5A4h] [ebp-598h]
    int32_t scoreLimit; // [esp+5A8h] [ebp-594h]
    char msg[1404]; // [esp+5ACh] [ebp-590h] BYREF
    int32_t numSorted; // [esp+B2Ch] [ebp-10h]
    clientState_s *clientState; // [esp+B30h] [ebp-Ch]
    int32_t i; // [esp+B34h] [ebp-8h]
    int32_t clientNum; // [esp+B38h] [ebp-4h]

    msg[0] = 0;
    msgLen = 0;
    numSorted = level.numConnectedClients;
    if (level.numConnectedClients > 64)
        numSorted = 64;
    for (i = 0; i < numSorted; ++i)
    {
        clientNum = level.sortedClients[i];
        v3 = &level.clients[clientNum];
        clientState = &v3->sess.cs;
        if (v3->sess.connected == CON_CONNECTING)
        {
            v1 = Com_sprintf(
                entry,
                0x598u,
                " %i %i %i %i %i %i %i",
                level.sortedClients[i],
                v3->sess.score,
                -1,
                v3->sess.deaths,
                v3->sess.status_icon,
                v3->sess.kills,
                v3->sess.assists);
        }
        else
        {
            ping = SV_GetClientPing(clientNum);
            v1 = Com_sprintf(
                entry,
                0x598u,
                " %i %i %i %i %i %i %i",
                level.sortedClients[i],
                v3->sess.score,
                ping,
                v3->sess.deaths,
                v3->sess.status_icon,
                v3->sess.kills,
                v3->sess.assists);
        }
        entryLen = v1;
        if (v1 < 0 || entryLen + msgLen > 1400)
        {
            Com_PrintError(15, "Scoreboard message too large: %i\n", entryLen + msgLen);
            Com_DPrintf(6, "%s\n%s\n", entry, msg);
            break;
        }
        I_strncpyz(&msg[msgLen], entry, 1400 - msgLen);
        msgLen += entryLen;
    }
    Com_sprintf(entry, 0x598u, "scr_%s_scorelimit", g_gametype->current.string);
    scoreLimit = Dvar_GetInt(entry);
    if (!scoreLimit)
    {
        Com_sprintf(entry, 0x598u, "scr_%s_roundlimit", g_gametype->current.string);
        scoreLimit = Dvar_GetInt(entry);
    }
    entryLen = Com_sprintf(
        entry,
        0x598u,
        "%c %i %i %i %i%s",
        98,
        i,
        level.teamScores[1],
        level.teamScores[2],
        scoreLimit,
        msg);
    if (entryLen >= 0)
    {
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_RELIABLE, entry);
    }
    else
    {
        Com_PrintError(15, "Scoreboard message too large > %i.  Message not sent.\n", 1432);
        Com_DPrintf(6, "%c %i %i %i %i%s", 98, i, level.teamScores[1], level.teamScores[2], scoreLimit, msg);
    }
}

void __cdecl Cmd_Score_f(gentity_s *ent)
{
    SendScoreboard(ent);
}

int32_t __cdecl CheatsOk(gentity_s *ent)
{
    const char *v1; // eax
    const char *v3; // eax

    if (g_cheats->current.enabled)
    {
        if (ent->health > 0)
        {
            return 1;
        }
        else
        {
            v3 = va("%c \"GAME_MUSTBEALIVECOMMAND\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v3);
            return 0;
        }
    }
    else
    {
        v1 = va("%c \"GAME_CHEATSNOTENABLED\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
        return 0;
    }
}

char line[1024];
char *__cdecl ConcatArgs(int32_t start)
{
    uint32_t v1; // kr00_4
    int32_t c; // [esp+10h] [ebp-418h]
    int32_t len; // [esp+14h] [ebp-414h]
    char arg[1028]; // [esp+20h] [ebp-408h] BYREF

    len = 0;
    c = SV_Cmd_Argc();
    while (start < c)
    {
        SV_Cmd_ArgvBuffer(start, arg, 1024);
        v1 = strlen(arg);
        if ((int)(v1 + len) >= 1023)
            break;
        memcpy((uint8_t *)&line[len], (uint8_t *)arg, v1);
        len += v1;
        if (start != c - 1)
            line[len++] = 32;
        ++start;
    }
    line[len] = 0;
    return line;
}

void __cdecl G_setfog(const char *fogstring)
{
    float fDensity; // [esp+0h] [ebp-1Ch] BYREF
    float clr[3]; // [esp+4h] [ebp-18h] BYREF
    float fFar; // [esp+10h] [ebp-Ch] BYREF
    float fNear; // [esp+14h] [ebp-8h] BYREF
    int32_t time; // [esp+18h] [ebp-4h] BYREF

    SV_SetConfigstring(9, (char*)fogstring);
    level.fFogOpaqueDist = FLT_MAX;
    level.fFogOpaqueDistSqrd = FLT_MAX;
    if (sscanf(fogstring, "%f %f %f %f %f %f %i", &fNear, &fFar, &fDensity, clr, &clr[1], &clr[2], &time) == 7
        && fDensity >= 1.0)
    {
        level.fFogOpaqueDist = (fFar - fNear) * 1.0 + fNear;
        level.fFogOpaqueDistSqrd = level.fFogOpaqueDist * level.fFogOpaqueDist;
    }
}

void __cdecl Cmd_Give_f(gentity_s *ent)
{
    WeaponDef *weapDef; // [esp+18h] [ebp-20h]
    gentity_s *it_ent; // [esp+1Ch] [ebp-1Ch]
    char *name; // [esp+20h] [ebp-18h]
    bool give_all; // [esp+24h] [ebp-14h]
    char *amt; // [esp+28h] [ebp-10h]
    int32_t amount; // [esp+2Ch] [ebp-Ch]
    uint32_t weapIndex; // [esp+30h] [ebp-8h]
    uint32_t weapIndexa; // [esp+30h] [ebp-8h]
    uint32_t weapIndexb; // [esp+30h] [ebp-8h]
    const gitem_s *it; // [esp+34h] [ebp-4h]

    if (CheatsOk(ent))
    {
        amt = ConcatArgs(2);
        amount = atoi(amt);
        name = ConcatArgs(1);
        if (name)
        {
            if (strlen(name))
            {
                if (!(give_all = I_stricmp(name, "all") == 0) && I_strnicmp(name, "health", 6)
                    || (!amount ? (ent->health = ent->client->ps.stats[2]) : (ent->health += amount), give_all))
                {
                    if (!give_all && I_stricmp(name, "weapons"))
                        goto LABEL_49;
                    for (weapIndex = 1; weapIndex < BG_GetNumWeapons(); ++weapIndex)
                    {
                        if (BG_CanPlayerHaveWeapon(weapIndex))
                        {
                            BG_TakePlayerWeapon(&ent->client->ps, weapIndex, 1);
                            G_GivePlayerWeapon(&ent->client->ps, weapIndex, 0);
                        }
                    }
                    if (give_all)
                    {
                    LABEL_49:
                        if (!give_all && I_strnicmp(name, "ammo", 4))
                            goto LABEL_35;
                        if (amount)
                        {
                            if (ent->client->ps.weapon)
                                Add_Ammo(ent, ent->client->ps.weapon, ent->client->ps.weaponmodels[ent->client->ps.weapon], amount, 1);
                        }
                        else
                        {
                            for (weapIndexa = 1; weapIndexa < BG_GetNumWeapons(); ++weapIndexa)
                                Add_Ammo(ent, weapIndexa, 0, 998, 1);
                        }
                        if (give_all)
                        {
                        LABEL_35:
                            if (I_strnicmp(name, "allammo", 7))
                                goto LABEL_48;
                            if (!amount)
                                goto LABEL_48;
                            for (weapIndexb = 1; weapIndexb < BG_GetNumWeapons(); Add_Ammo(ent, weapIndexb++, 0, amount, 1))
                                ;
                            if (give_all)
                            {
                            LABEL_48:
                                if (!give_all)
                                {
                                    level.initializing = 1;
                                    it = G_FindItem(name, 0);
                                    if (it)
                                    {
                                        it_ent = G_Spawn();
                                        it_ent->r.currentOrigin[0] = ent->r.currentOrigin[0];
                                        it_ent->r.currentOrigin[1] = ent->r.currentOrigin[1];
                                        it_ent->r.currentOrigin[2] = ent->r.currentOrigin[2];
                                        G_GetItemClassname(it, &it_ent->classname);
                                        G_SpawnItem(it_ent, it);
                                        it_ent->active = 1;
                                        if (it->giType == IT_WEAPON)
                                        {
                                            weapDef = BG_GetWeaponDef(it_ent->item[0].index % 128);
                                            if (weapDef->offhandClass == OFFHAND_CLASS_FLASH_GRENADE)
                                            {
                                                ent->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_FLASH;
                                            }
                                            else if (weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE)
                                            {
                                                ent->client->ps.offhandSecondary = PLAYER_OFFHAND_SECONDARY_SMOKE;
                                            }
                                        }
                                        Touch_Item(it_ent, ent, 0);
                                        it_ent->active = 0;
                                        if (it_ent->r.inuse)
                                            G_FreeEntity(it_ent);
                                        level.initializing = 0;
                                    }
                                    else
                                    {
                                        level.initializing = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Cmd_Take_f(gentity_s *ent)
{
    gclient_s *client; // esi
    int32_t v2; // eax
    gclient_s *v3; // esi
    gclient_s *v4; // esi
    gclient_s *v5; // esi
    gclient_s *v6; // esi
    gclient_s *v7; // esi
    gclient_s *v8; // esi
    gclient_s *v9; // esi
    gclient_s *v10; // esi
    gclient_s *v11; // esi
    int32_t v12; // eax
    gclient_s *v13; // esi
    gclient_s *v14; // esi
    gclient_s *v15; // esi
    gclient_s *v16; // esi
    gclient_s *v17; // esi
    gclient_s *v18; // esi
    int32_t *v19; // [esp+4h] [ebp-34h]
    int32_t *v20; // [esp+Ch] [ebp-2Ch]
    char *name; // [esp+24h] [ebp-14h]
    char *amt; // [esp+28h] [ebp-10h]
    int32_t amount; // [esp+2Ch] [ebp-Ch]
    uint32_t weapIndex; // [esp+30h] [ebp-8h]
    uint32_t weapIndexa; // [esp+30h] [ebp-8h]
    uint32_t weapIndexb; // [esp+30h] [ebp-8h]
    uint32_t weapIndexc; // [esp+30h] [ebp-8h]
    bool take_all; // [esp+34h] [ebp-4h]

    if (CheatsOk(ent))
    {
        amt = ConcatArgs(2);
        amount = atoi(amt);
        name = ConcatArgs(1);
        if (name)
        {
            if (strlen(name))
            {
                take_all = I_stricmp(name, "all") == 0;
                if (!take_all)
                {
                    if (I_strnicmp(name, "health", 6))
                        goto LABEL_20;
                }
                if (amount)
                {
                    ent->health -= amount;
                    if (ent->health < 1)
                        ent->health = 1;
                }
                else
                {
                    ent->health = 1;
                }
                if (take_all)
                {
                LABEL_20:
                    if (!take_all && I_stricmp(name, "weapons"))
                        goto LABEL_32;
                    for (weapIndex = 1; weapIndex < BG_GetNumWeapons(); ++weapIndex)
                        BG_TakePlayerWeapon(&ent->client->ps, weapIndex, 1);
                    if (ent->client->ps.weapon)
                    {
                        ent->client->ps.weapon = 0;
                        G_SelectWeaponIndex(ent - g_entities, 0);
                    }
                    if (take_all)
                    {
                    LABEL_32:
                        if (!take_all && I_strnicmp(name, "ammo", 4))
                            goto LABEL_34;
                        if (amount)
                        {
                            if (ent->client->ps.weapon)
                            {
                                weapIndexa = ent->client->ps.weapon;
                                client = ent->client;
                                v2 = BG_AmmoForWeapon(weapIndexa);
                                client->ps.ammo[v2] -= amount;
                                v3 = ent->client;
                                if (v3->ps.ammo[BG_AmmoForWeapon(weapIndexa)] < 0)
                                {
                                    v4 = ent->client;
                                    v20 = &v4->ps.ammoclip[BG_ClipForWeapon(weapIndexa)];
                                    v5 = ent->client;
                                    *v20 += v5->ps.ammo[BG_AmmoForWeapon(weapIndexa)];
                                    v6 = ent->client;
                                    v6->ps.ammo[BG_AmmoForWeapon(weapIndexa)] = 0;
                                    v7 = ent->client;
                                    if (v7->ps.ammoclip[BG_ClipForWeapon(weapIndexa)] < 0)
                                    {
                                        v8 = ent->client;
                                        v8->ps.ammoclip[BG_ClipForWeapon(weapIndexa)] = 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            for (weapIndexb = 1; weapIndexb < BG_GetNumWeapons(); ++weapIndexb)
                            {
                                v9 = ent->client;
                                v9->ps.ammo[BG_AmmoForWeapon(weapIndexb)] = 0;
                                v10 = ent->client;
                                v10->ps.ammoclip[BG_ClipForWeapon(weapIndexb)] = 0;
                            }
                        }
                        if (take_all)
                        {
                        LABEL_34:
                            if (!I_strnicmp(name, "allammo", 7) && amount)
                            {
                                for (weapIndexc = 1; weapIndexc < BG_GetNumWeapons(); ++weapIndexc)
                                {
                                    v11 = ent->client;
                                    v12 = BG_AmmoForWeapon(weapIndexc);
                                    v11->ps.ammo[v12] -= amount;
                                    v13 = ent->client;
                                    if (v13->ps.ammo[BG_AmmoForWeapon(weapIndexc)] < 0)
                                    {
                                        v14 = ent->client;
                                        v19 = &v14->ps.ammoclip[BG_ClipForWeapon(weapIndexc)];
                                        v15 = ent->client;
                                        *v19 += v15->ps.ammo[BG_AmmoForWeapon(weapIndexc)];
                                        v16 = ent->client;
                                        v16->ps.ammo[BG_AmmoForWeapon(weapIndexc)] = 0;
                                        v17 = ent->client;
                                        if (v17->ps.ammoclip[BG_ClipForWeapon(weapIndexc)] < 0)
                                        {
                                            v18 = ent->client;
                                            v18->ps.ammoclip[BG_ClipForWeapon(weapIndexc)] = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Cmd_God_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *v2; // [esp+0h] [ebp-8h]

    if (CheatsOk(ent))
    {
        ent->flags ^= 1u;
        if ((ent->flags & 1) != 0)
            v2 = "GAME_GODMODE_ON";
        else
            v2 = "GAME_GODMODE_OFF";
        v1 = va("%c \"%s\"", 101, v2);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
    }
}

void __cdecl Cmd_DemiGod_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *v2; // [esp+0h] [ebp-8h]

    if (CheatsOk(ent))
    {
        ent->flags ^= 2u;
        if ((ent->flags & 2) != 0)
            v2 = "GAME_DEMI_GODMODE_ON";
        else
            v2 = "GAME_DEMI_GODMODE_OFF";
        v1 = va("%c \"%s\"", 101, v2);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
    }
}

void __cdecl Cmd_Notarget_f(gentity_s *ent)
{
    const char *v1; // eax

    if (CheatsOk(ent))
    {
        ent->flags ^= 4u;
        if ((ent->flags & 4) != 0)
            v1 = va("%c \"%s\"", 101, "GAME_NOTARGETON");
        else
            v1 = va("%c \"%s\"", 101, "GAME_NOTARGETOFF");
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
    }
}

void __cdecl Cmd_Noclip_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *msg; // [esp+0h] [ebp-4h]

    if (CheatsOk(ent))
    {
        if (ent->client->noclip)
            msg = "GAME_NOCLIPOFF";
        else
            msg = "GAME_NOCLIPON";
        ent->client->noclip = ent->client->noclip == 0;
        v1 = va("%c \"%s\"", 101, msg);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
    }
}

void __cdecl Cmd_UFO_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *msg; // [esp+0h] [ebp-4h]

    if (CheatsOk(ent))
    {
        if (ent->client->ufo)
            msg = "GAME_UFOOFF";
        else
            msg = "GAME_UFOON";
        ent->client->ufo = ent->client->ufo == 0;
        v1 = va("%c \"%s\"", 101, msg);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
    }
}

void __cdecl Cmd_Kill_f(gentity_s *ent)
{
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 679, 0, "%s", "ent->client");
    if (ent->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 680, 0, "%s", "ent->client->sess.connected != CON_DISCONNECTED");
    if (ent->client->sess.sessionState == SESS_STATE_PLAYING && CheatsOk(ent))
    {
        if (bgs)
            MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 688, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
        bgs = &level_bgs;
        ent->flags &= 0xFFFFFFFC;
        ent->health = 0;
        ent->client->ps.stats[0] = 0;
        player_die(ent, ent, ent, 100000, 12, 0, 0, HITLOC_NONE, 0);
        if (bgs != &level_bgs)
            MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 695, 0, "%s\n\t(bgs) = %p", "(bgs == &level_bgs)", bgs);
        bgs = 0;
    }
}

void __cdecl StopFollowing(gentity_s *ent)
{
    gclient_s *client; // [esp+14h] [ebp-84h]
    float vAngles[3]; // [esp+18h] [ebp-80h] BYREF
    float vEnd[3]; // [esp+24h] [ebp-74h] BYREF
    float vMins[3]; // [esp+30h] [ebp-68h] BYREF
    trace_t trace; // [esp+3Ch] [ebp-5Ch] BYREF
    float vForward[3]; // [esp+68h] [ebp-30h] BYREF
    float vPos[3]; // [esp+74h] [ebp-24h] BYREF
    float vUp[3]; // [esp+80h] [ebp-18h] BYREF
    float vMaxs[3]; // [esp+8Ch] [ebp-Ch] BYREF

    client = ent->client;
    if (!client)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 721, 0, "%s", "client");
    client->sess.forceSpectatorClient = -1;
    client->sess.killCamEntity = -1;
    client->spectatorClient = -1;
    if ((client->ps.otherFlags & 2) != 0)
    {
        G_GetPlayerViewOrigin(&client->ps, vPos);
        BG_GetPlayerViewDirection(&client->ps, vForward, 0, vUp);
        vAngles[0] = client->ps.viewangles[0];
        vAngles[1] = client->ps.viewangles[1];
        vAngles[2] = client->ps.viewangles[2];
        vAngles[0] = vAngles[0] + 15.0;
        Vec3Mad(vPos, -40.0, vForward, vEnd);
        Vec3Mad(vEnd, 10.0, vUp, vEnd);
        vMins[0] = -8.0;
        vMins[1] = -8.0;
        vMins[2] = -8.0;
        vMaxs[0] = 8.0;
        vMaxs[1] = 8.0;
        vMaxs[2] = 8.0;
        G_TraceCapsule(&trace, vPos, vMins, vMaxs, vEnd, 1023, 0x810011);
        Vec3Lerp(vPos, vEnd, trace.fraction, vPos);
        client->ps.clientNum = ent - g_entities;
        client->ps.eFlags &= 0xFFFFFCFF;
        client->ps.viewlocked = PLAYERVIEWLOCK_NONE;
        client->ps.viewlocked_entNum = 1023;
        client->ps.pm_flags &= 0xFFFEFFEF;
        client->ps.weapFlags &= ~0x40u;
        client->ps.otherFlags &= ~2u;
        client->ps.fWeaponPosFrac = 0.0;
        G_SetOrigin(ent, vPos);
        client->ps.origin[0] = vPos[0];
        client->ps.origin[1] = vPos[1];
        client->ps.origin[2] = vPos[2];
        SetClientViewAngle(ent, vAngles);
        if (!ent->tagInfo)
            ent->r.currentAngles[0] = 0.0;
        client->ps.shellshockIndex = 0;
        client->ps.shellshockTime = 0;
        client->ps.shellshockDuration = 0;
    }
}

int32_t __cdecl Cmd_FollowCycle_f(gentity_s *ent, int32_t dir)
{
    int32_t v3; // [esp+0h] [ebp-2FE0h]
    int32_t clientNum; // [esp+4h] [ebp-2FDCh]
    clientState_s v5; // [esp+8h] [ebp-2FD8h] BYREF
    playerState_s ps; // [esp+70h] [ebp-2F70h] BYREF

    if (dir != 1 && dir != -1)
        Com_Error(ERR_DROP, "Cmd_FollowCycle_f: bad dir %i", dir);
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 793, 0, "%s", "ent->client");
    if (ent->client->sess.sessionState != SESS_STATE_SPECTATOR)
        return 0;
    if (ent->client->sess.forceSpectatorClient >= 0)
        return 0;
    clientNum = ent->client->spectatorClient;
    if (clientNum < 0)
        clientNum = 0;
    v3 = clientNum;
    do
    {
        clientNum += dir;
        if (clientNum >= level.maxclients)
            clientNum = 0;
        if (clientNum < 0)
            clientNum = level.maxclients - 1;
        if (SV_GetArchivedClientInfo(clientNum, &ent->client->sess.archiveTime, &ps, &v5))
        {
            if ((ps.otherFlags & 4) == 0)
                MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 823, 0, "%s", "ps.otherFlags & POF_PLAYER");
            if (G_ClientCanSpectateTeam(ent->client, v5.team))
            {
                ent->client->spectatorClient = clientNum;
                ent->client->sess.sessionState = SESS_STATE_SPECTATOR;
                return 1;
            }
        }
    } while (clientNum != v3);
    return 0;
}

bool __cdecl G_IsPlaying(gentity_s *ent)
{
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 845, 0, "%s", "ent->client");
    if (ent->client->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 846, 0, "%s", "ent->client->sess.connected != CON_DISCONNECTED");
    return ent->client->sess.sessionState == SESS_STATE_PLAYING;
}

void __cdecl G_Say(gentity_s *ent, gentity_s *target, int32_t mode, char *chatText)
{
    char *v4; // eax
    char *Guid; // eax
    int32_t v6; // [esp-Ch] [ebp-104h]
    int32_t number; // [esp-Ch] [ebp-104h]
    int32_t j; // [esp+4h] [ebp-F4h]
    char cleanname[68]; // [esp+8h] [ebp-F0h] BYREF
    gentity_s *other; // [esp+4Ch] [ebp-ACh]
    const char *pszTeamString; // [esp+50h] [ebp-A8h]
    int32_t color; // [esp+54h] [ebp-A4h]
    char text[156]; // [esp+58h] [ebp-A0h] BYREF

    pszTeamString = "";
    if (mode == 1 && ent->client->sess.cs.team != TEAM_AXIS)
        mode = ent->client->sess.cs.team == TEAM_ALLIES;
    I_strncpyz(cleanname, ent->client->sess.cs.name, 64);
    I_CleanStr(cleanname);
    if (mode == 1)
    {
        if (ent->client->sess.cs.team != TEAM_AXIS && ent->client->sess.cs.team != TEAM_ALLIES)
            MyAssertHandler(
                ".\\game_mp\\g_cmds_mp.cpp",
                965,
                0,
                "%s",
                "(ent->client->sess.cs.team == TEAM_AXIS) || (ent->client->sess.cs.team == TEAM_ALLIES)");
        number = ent->s.number;
        Guid = SV_GetGuid(ent->s.number);
        G_LogPrintf("sayteam;%s;%d;%s;%s\n", Guid, number, cleanname, chatText);
        if (ent->client->sess.cs.team == TEAM_AXIS)
            pszTeamString = Dvar_GetString("g_TeamName_Axis");
        else
            pszTeamString = Dvar_GetString("g_TeamName_Allies");
        color = 53;
    }
    else if (mode == 2)
    {
        color = 51;
    }
    else
    {
        v6 = ent->s.number;
        v4 = SV_GetGuid(ent->s.number);
        G_LogPrintf("say;%s;%d;%s;%s\n", v4, v6, cleanname, chatText);
        color = 55;
    }
    I_strncpyz(text, chatText, 150);
    if (target)
    {
        G_SayTo(ent, target, mode, color, pszTeamString, cleanname, text);
    }
    else
    {
        if (g_dedicated->current.integer)
            Com_Printf(15, "%s%s\n", cleanname, text);
        for (j = 0; j < level.maxclients; ++j)
        {
            other = &g_entities[j];
            G_SayTo(ent, other, mode, color, pszTeamString, cleanname, text);
        }
    }
}

void __cdecl G_SayTo(
    gentity_s *ent,
    gentity_s *other,
    int32_t mode,
    int32_t color,
    const char *teamString,
    const char *cleanname,
    const char *message)
{
    const char *v7; // eax
    team_t team; // [esp+4h] [ebp-D4h]
    char szStateString[68]; // [esp+8h] [ebp-D0h] BYREF
    const char *team_color; // [esp+4Ch] [ebp-8Ch]
    char name[132]; // [esp+50h] [ebp-88h] BYREF

    if (other
        && other->r.inuse
        && other->client
        && other->client->sess.connected == CON_CONNECTED
        && (mode != 1 || OnSameTeam(ent, other))
        && (g_deadChat->current.enabled || G_IsPlaying(ent) || !G_IsPlaying(other)))
    {
        team = ent->client->sess.cs.team;
        if (team <= TEAM_FREE || team > TEAM_ALLIES)
        {
            team_color = "";
        }
        else if (ent->client->sess.cs.team == other->client->sess.cs.team)
        {
            team_color = "^8";
        }
        else
        {
            team_color = "^9";
        }
        if (ent->client->sess.cs.team == TEAM_SPECTATOR)
        {
            // KISAKTODO: cleaner strings here
            Com_sprintf(szStateString, 0x40u, (const char*)"\x15\x28\x14\x47\x41\x4d\x45\x5f\x53\x50\x45\x43\x54\x41\x54\x4f\x52\x15\x29");
        }
        else if (ent->client->sess.sessionState)
        {
            Com_sprintf(szStateString, 0x40u, (const char*)"\x15\x25\x73\x28\x14\x47\x41\x4d\x45\x5f\x44\x45\x41\x44\x15\x29", team_color);
        }
        else
        {
            Com_sprintf(szStateString, 0x40u, (const char*)"\x15\x25\x73\x0", team_color);
        }
        if (mode == 1)
        {
            Com_sprintf(name, 0x80u, (const char*)"%s(\x14\x25\x73\x15\x29\x25\x73\x25\x73\x3a\x20", szStateString, teamString, cleanname, "^7");
        }
        else if (mode == 2)
        {
            Com_sprintf(name, 0x80u, "%s[%s]%s: ", szStateString, cleanname, "^7");
        }
        else
        {
            Com_sprintf(name, 0x80u, "%s%s%s: ", szStateString, cleanname, "^7");
        }
        v7 = va("%c \"\x15\x25\x73\x25\x63\x25\x63\x25\x73\x22", (char)((mode == 1) + 104), name, 94, color, message);
        SV_GameSendServerCommand(other - g_entities, SV_CMD_CAN_IGNORE, v7);
    }
}

void __cdecl Cmd_Where_f(gentity_s *ent)
{
    char *v1; // eax
    const char *v2; // eax

    v1 = vtos(ent->r.currentOrigin);
    v2 = va("%c \"\x15\x25\x73\x0a\x22", 101, v1);
    SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v2);
}

void __cdecl Cmd_CallVote_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int32_t v6; // eax
    int32_t v7; // eax
    int32_t v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    char *GameTypeNameForScript; // eax
    char *v17; // eax
    char *v18; // eax
    const char *v19; // eax
    const char *v20; // eax
    int32_t Int; // eax
    char *v22; // eax
    char *v23; // eax
    char *v24; // eax
    int32_t j; // [esp+4h] [ebp-35Ch]
    uint32_t kicknum; // [esp+8h] [ebp-358h]
    const dvar_s *mapname; // [esp+Ch] [ebp-354h]
    char arg1[256]; // [esp+10h] [ebp-350h] BYREF
    char arg2[256]; // [esp+110h] [ebp-250h] BYREF
    char cleanName[68]; // [esp+210h] [ebp-150h] BYREF
    int32_t i; // [esp+254h] [ebp-10Ch]
    char arg3[260]; // [esp+258h] [ebp-108h] BYREF

    const char aCGameCalledavo[] = "%c \"GAME_CALLEDAVOTE"; // idb
    const char aGameVoteKick[] = "GAME_VOTE_KICK"; // idb
    const char aGameVoteGamety[] = "GAME_VOTE_GAMETYPE"; // idb
    const char aGameVoteMap[] = "GAME_VOTE_MAP"; // idb
    const char aGameVoteGamety_0[] = "GAME_VOTE_GAMETYPE"; // idb

    const char aC_7[] = "%c \""; // idb

    if (!g_allowVote->current.enabled)
    {
        v1 = va("%c \"GAME_VOTINGNOTENABLED\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
        return;
    }
    if (level.numConnectedClients < 2)
    {
        v2 = va("%c \"GAME_VOTINGNOTENOUGHPLAYERS\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v2);
        return;
    }
    if (g_oldVoting->current.enabled)
    {
        if (level.voteTime)
        {
            v3 = va("%c \"GAME_VOTEALREADYINPROGRESS\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v3);
            return;
        }
        if (ent->client->sess.voteCount >= 3)
        {
            v4 = va("%c \"GAME_MAXVOTESCALLED\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v4);
            return;
        }
        if (ent->client->sess.cs.team == TEAM_SPECTATOR)
        {
            v5 = va("%c \"GAME_NOSPECTATORCALLVOTE\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v5);
            return;
        }
    }
    SV_Cmd_ArgvBuffer(1, arg1, 256);
    SV_Cmd_ArgvBuffer(2, arg2, 256);
    SV_Cmd_ArgvBuffer(3, arg3, 256);
    //if (strchr(arg1, 0x3Bu) || (strchr(arg2, 0x3Bu), v7) || (strchr(arg3, 0x3Bu), v8))
    if (strchr(arg1, 0x3Bu) || (strchr(arg2, 0x3Bu) || (strchr(arg3, 0x3Bu))))
    {
        v9 = va("%c \"GAME_INVALIDVOTESTRING\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v9);
        return;
    }
    if (!g_oldVoting->current.enabled)
    {
        Scr_VoteCalled(ent, arg1, arg2, arg3);
        return;
    }
    if (I_stricmp(arg1, "map_restart")
        && I_stricmp(arg1, "map_rotate")
        && I_stricmp(arg1, "typemap")
        && I_stricmp(arg1, "map")
        && I_stricmp(arg1, "g_gametype")
        && I_stricmp(arg1, "kick")
        && I_stricmp(arg1, "clientkick")
        && I_stricmp(arg1, "tempBanUser")
        && I_stricmp(arg1, "tempBanClient"))
    {
        v10 = va("%c \"GAME_INVALIDVOTESTRING\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v10);
        const char aCGameVotecomma[] = "%c \"GAME_VOTECOMMANDSARE"; // idb
        v11 = va(aCGameVotecomma, 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v11);
        return;
    }
    if (level.voteExecuteTime)
    {
        level.voteExecuteTime = 0;
        v12 = va("%s\n", level.voteString);
        Cbuf_AddText(0, v12);
    }
    if (!I_stricmp(arg1, "typemap"))
    {
        if (!Scr_IsValidGameType(arg2))
        {
        LABEL_31:
            v13 = va("%c \"GAME_INVALIDGAMETYPE\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v13);
            return;
        }
        if (!I_stricmp(arg2, g_gametype->current.string))
            arg2[0] = 0;
        SV_Cmd_ArgvBuffer(3, arg3, 256);
        if (!useFastFile->current.enabled && !SV_MapExists(arg3))
            goto LABEL_36;
        mapname = Dvar_RegisterString("mapname", (char *)"", DVAR_SERVERINFO | DVAR_ROM, "Current map name");
        if (!I_stricmp(arg3, mapname->current.string))
            arg3[0] = 0;
        if (!arg2[0] && !arg3[0])
        {
            v15 = va("%c \"GAME_TYPEMAP_NOCHANGE\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v15);
            return;
        }
        if (arg3[0])
        {
            if (arg2[0])
                Com_sprintf(level.voteString, 0x400u, "g_gametype %s; map %s", arg2, arg3);
            else
                Com_sprintf(level.voteString, 0x400u, "map %s", arg3);
            if (arg2[0])
            {
                GameTypeNameForScript = Scr_GetGameTypeNameForScript(arg2);
                Com_sprintf(level.voteDisplayString, 0x400u, aGameVoteGamety_0, GameTypeNameForScript, arg3);
            }
            else
            {
                Com_sprintf(level.voteDisplayString, 0x400u, aGameVoteMap, arg3);
            }
        }
        else
        {
            Com_sprintf(level.voteString, 0x400u, "g_gametype %s; map_restart", arg2);
            v17 = Scr_GetGameTypeNameForScript(arg2);
            Com_sprintf(level.voteDisplayString, 0x400u, aGameVoteGamety, v17);
        }
        goto LABEL_91;
    }
    if (!I_stricmp(arg1, "g_gametype"))
    {
        if (!Scr_IsValidGameType(arg2))
            goto LABEL_31;
        Com_sprintf(level.voteString, 0x400u, "%s %s; map_restart", arg1, arg2);
        v18 = Scr_GetGameTypeNameForScript(arg2);
        Com_sprintf(level.voteDisplayString, 0x400u, aGameVoteGamety, v18);
        goto LABEL_91;
    }
    if (!I_stricmp(arg1, "map_restart"))
    {
        Com_sprintf(level.voteString, 0x400u, "fast_restart");
        Com_sprintf(level.voteDisplayString, 0x400u, "GAME_VOTE_MAPRESTART");
        goto LABEL_91;
    }
    if (!I_stricmp(arg1, "map_rotate"))
    {
        Com_sprintf(level.voteString, 0x400u, "%s", arg1);
        Com_sprintf(level.voteDisplayString, 0x400u, "GAME_VOTE_NEXTMAP");
    LABEL_91:
        v20 = va(aCGameCalledavo, 101, ent->client->sess.cs.name);
        SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v20);
        level.voteTime = level.time + 30000;
        level.voteYes = 1;
        level.voteNo = 0;
        for (i = 0; i < level.maxclients; ++i)
            level.clients[i].ps.eFlags &= ~0x100000u;
        ent->client->ps.eFlags |= 0x100000u;
        Int = Dvar_GetInt("sv_serverId");
        v22 = va("%i %i", level.voteTime, Int);
        SV_SetConfigstring(13, v22);
        SV_SetConfigstring(14, level.voteDisplayString);
        v23 = va("%i", level.voteYes);
        SV_SetConfigstring(15, v23);
        v24 = va("%i", level.voteNo);
        SV_SetConfigstring(16, v24);
        return;
    }
    if (!I_stricmp(arg1, "map"))
    {
        if (!useFastFile->current.enabled && !SV_MapExists(arg2))
        {
        LABEL_36:
            v14 = va(aC_7, 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v14);
            return;
        }
        Com_sprintf(level.voteString, 0x400u, "%s %s", arg1, arg2);
        Com_sprintf(level.voteDisplayString, 0x400u, aGameVoteMap, arg2);
        goto LABEL_91;
    }
    if (!I_stricmp(arg1, "kick")
        || !I_stricmp(arg1, "clientkick")
        || !I_stricmp(arg1, "tempBanUser")
        || !I_stricmp(arg1, "tempBanClient"))
    {
        kicknum = 64;
        if (I_stricmp(arg1, "kick") && I_stricmp(arg1, "tempBanUser"))
        {
            kicknum = atoi(arg2);
            if ((kicknum || !I_stricmp(arg2, "0"))
                && kicknum < 0x40
                && level.clients[kicknum].sess.connected == CON_CONNECTED)
            {
                I_strncpyz(cleanName, level.clients[kicknum].sess.cs.name, 64);
                I_CleanStr(cleanName);
            }
            else
            {
                kicknum = 64;
            }
        }
        else
        {
            for (j = 0; j < 64; ++j)
            {
                if (level.clients[j].sess.connected == CON_CONNECTED)
                {
                    I_strncpyz(cleanName, level.clients[j].sess.cs.name, 64);
                    I_CleanStr(cleanName);
                    if (!I_stricmp(cleanName, arg2))
                        kicknum = j;
                }
            }
        }
        if (kicknum == 64)
        {
            v19 = va("%c \"GAME_CLIENTNOTONSERVER\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v19);
            return;
        }
        if (arg1[0] == 116 || arg1[0] == 84)
            Com_sprintf(level.voteString, 0x400u, "%s \"%d\"", "tempBanClient", kicknum);
        else
            Com_sprintf(level.voteString, 0x400u, "%s \"%d\"", "clientkick", kicknum);
        Com_sprintf(level.voteDisplayString, 0x400u, aGameVoteKick, kicknum, level.clients[kicknum].sess.cs.name);
        goto LABEL_91;
    }
    if (!alwaysfails)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 1271, 0, "unhandled callvote");
}

void __cdecl Cmd_Vote_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char msg[68]; // [esp+0h] [ebp-48h] BYREF

    if (g_oldVoting->current.enabled)
    {
        if (!level.voteTime)
        {
            v1 = va("%c \"GAME_NOVOTEINPROGRESS\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
            return;
        }
        if ((ent->client->ps.eFlags & 0x100000) != 0)
        {
            v2 = va("%c \"GAME_VOTEALREADYCAST\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v2);
            return;
        }
        if (ent->client->sess.cs.team == TEAM_SPECTATOR)
        {
            v3 = va("%c \"GAME_NOSPECTATORVOTE\"", 101);
            SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v3);
            return;
        }
        v4 = va("%c \"GAME_VOTECAST\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v4);
        ent->client->ps.eFlags |= 0x100000u;
    }
    SV_Cmd_ArgvBuffer(1, msg, 64);
    if (msg[0] == 121 || msg[1] == 89 || msg[1] == 49)
    {
        if (g_oldVoting->current.enabled)
        {
            v5 = va("%i", ++level.voteYes);
            SV_SetConfigstring(15, v5);
        }
        else
        {
            Scr_PlayerVote(ent, (char *)"yes");
        }
    }
    else if (g_oldVoting->current.enabled)
    {
        v6 = va("%i", ++level.voteNo);
        SV_SetConfigstring(16, v6);
    }
    else
    {
        Scr_PlayerVote(ent, (char*)"no");
    }
}

const char aCGameUsage[] = "%c \"GAME_USAGE"; // idb

void __cdecl Cmd_SetViewpos_f(gentity_s *ent)
{
    const char *v1; // eax
    const char *v2; // eax
    long double v3; // st7
    int32_t v4; // [esp+0h] [ebp-424h]
    char buffer[1024]; // [esp+4h] [ebp-420h] BYREF
    float origin[3]; // [esp+408h] [ebp-1Ch] BYREF
    float angles[3]; // [esp+414h] [ebp-10h] BYREF
    int32_t i; // [esp+420h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 1372, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler(".\\game_mp\\g_cmds_mp.cpp", 1373, 0, "%s", "ent->client");
    if (!g_cheats->current.enabled)
    {
        v1 = va("%c \"GAME_CHEATSNOTENABLED\"", 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v1);
        return;
    }
    if (SV_Cmd_Argc() < 4 || SV_Cmd_Argc() > 6)
    {
        v2 = va(aCGameUsage, 101);
        SV_GameSendServerCommand(ent - g_entities, SV_CMD_CAN_IGNORE, v2);
        return;
    }
    for (i = 0; i < 3; ++i)
    {
        SV_Cmd_ArgvBuffer(i + 1, buffer, 1024);
        v3 = atof(buffer);
        origin[i] = v3;
    }
    origin[2] = origin[2] - ent->client->ps.viewHeightCurrent;
    angles[0] = 0.0;
    angles[1] = 0.0;
    angles[2] = 0.0;
    v4 = SV_Cmd_Argc();
    if (v4 == 5)
        goto LABEL_16;
    if (v4 == 6)
    {
        SV_Cmd_ArgvBuffer(5, buffer, 1024);
        angles[0] = atof(buffer);
    LABEL_16:
        SV_Cmd_ArgvBuffer(4, buffer, 1024);
        angles[1] = atof(buffer);
    }
    TeleportPlayer(ent, origin, angles);
}

void __cdecl Cmd_EntityCount_f()
{
    if (g_cheats->current.enabled)
        Com_Printf(0, "entity count = %i\n", level.num_entities);
}

void __cdecl Cmd_MenuResponse_f(gentity_s *pEnt)
{
    int32_t v1; // esi
    char szServerId[1024]; // [esp+4h] [ebp-C10h] BYREF
    char szMenuName[1028]; // [esp+404h] [ebp-810h] BYREF
    int32_t iMenuIndex; // [esp+808h] [ebp-40Ch]
    char szResponse[1028]; // [esp+80Ch] [ebp-408h] BYREF

    iMenuIndex = -1;
    if (SV_Cmd_Argc() == 4)
    {
        SV_Cmd_ArgvBuffer(1, szServerId, 1024);
        v1 = atoi(szServerId);
        if (v1 != Dvar_GetInt("sv_serverId"))
            return;
        SV_Cmd_ArgvBuffer(2, szMenuName, 1024);
        iMenuIndex = atoi(szMenuName);
        if ((uint32_t)iMenuIndex < 0x20)
            SV_GetConfigstring(iMenuIndex + 1970, szMenuName, 1024);
        SV_Cmd_ArgvBuffer(3, szResponse, 1024);
    }
    else
    {
        szMenuName[0] = 0;
        strcpy(szResponse, "bad");
    }
    Scr_AddString(szResponse);
    Scr_AddString(szMenuName);
    Scr_Notify(pEnt, scr_const.menuresponse, 2u);
}

const char aCGameUnknowncl[] = "%c \"GAME_UNKNOWNCLIENTCOMMAND"; // idb

void __cdecl ClientCommand(int32_t clientNum)
{
    const char *v1; // eax
    gentity_s *ent; // [esp+0h] [ebp-40Ch]
    char cmd[1028]; // [esp+4h] [ebp-408h] BYREF

    ent = &g_entities[clientNum];
    if (ent->client)
    {
        SV_Cmd_ArgvBuffer(0, cmd, 1024);
        if (I_stricmp(cmd, "say"))
        {
            if (I_stricmp(cmd, "say_team"))
            {
                if (I_stricmp(cmd, "score"))
                {
                    if (ent->client->ps.pm_type != PM_INTERMISSION)
                    {
                        if (I_stricmp(cmd, "mr"))
                        {
                            if (I_stricmp(cmd, "give"))
                            {
                                if (I_stricmp(cmd, "take"))
                                {
                                    if (I_stricmp(cmd, "god"))
                                    {
                                        if (I_stricmp(cmd, "demigod"))
                                        {
                                            if (I_stricmp(cmd, "notarget"))
                                            {
                                                if (I_stricmp(cmd, "noclip"))
                                                {
                                                    if (I_stricmp(cmd, "ufo"))
                                                    {
                                                        if (I_stricmp(cmd, "kill"))
                                                        {
                                                            if (I_stricmp(cmd, "follownext"))
                                                            {
                                                                if (I_stricmp(cmd, "followprev"))
                                                                {
                                                                    if (I_stricmp(cmd, "where"))
                                                                    {
                                                                        if (I_stricmp(cmd, "callvote"))
                                                                        {
                                                                            if (I_stricmp(cmd, "vote"))
                                                                            {
                                                                                if (I_stricmp(cmd, "setviewpos"))
                                                                                {
                                                                                    if (I_stricmp(cmd, "entitycount"))
                                                                                    {
                                                                                        if (I_stricmp(cmd, "printentities"))
                                                                                        {
                                                                                            if (I_stricmp(cmd, "visionsetnaked"))
                                                                                            {
                                                                                                if (I_stricmp(cmd, "visionsetnight"))
                                                                                                {
                                                                                                    v1 = va(aCGameUnknowncl, 101, cmd);
                                                                                                    SV_GameSendServerCommand(clientNum, SV_CMD_CAN_IGNORE, v1);
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    Cmd_VisionSetNight_f();
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                Cmd_VisionSetNaked_f();
                                                                                            }
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            Cmd_PrintEntities_f();
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        Cmd_EntityCount_f();
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    Cmd_SetViewpos_f(ent);
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                Cmd_Vote_f(ent);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            Cmd_CallVote_f(ent);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        Cmd_Where_f(ent);
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    Cmd_FollowCycle_f(ent, -1);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                Cmd_FollowCycle_f(ent, 1);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            Cmd_Kill_f(ent);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        Cmd_UFO_f(ent);
                                                    }
                                                }
                                                else
                                                {
                                                    Cmd_Noclip_f(ent);
                                                }
                                            }
                                            else
                                            {
                                                Cmd_Notarget_f(ent);
                                            }
                                        }
                                        else
                                        {
                                            Cmd_DemiGod_f(ent);
                                        }
                                    }
                                    else
                                    {
                                        Cmd_God_f(ent);
                                    }
                                }
                                else
                                {
                                    Cmd_Take_f(ent);
                                }
                            }
                            else
                            {
                                Cmd_Give_f(ent);
                            }
                        }
                        else
                        {
                            Cmd_MenuResponse_f(ent);
                        }
                    }
                }
                else
                {
                    Cmd_Score_f(ent);
                }
            }
            else
            {
                Cmd_Say_f(ent, 1, 0);
            }
        }
        else
        {
            Cmd_Say_f(ent, 0, 0);
        }
    }
}

void __cdecl Cmd_Say_f(gentity_s *ent, int32_t mode, int32_t arg0)
{
    char *p; // [esp+0h] [ebp-4h]

    if (SV_Cmd_Argc() >= 2 || arg0)
    {
        if (arg0)
            p = ConcatArgs(0);
        else
            p = ConcatArgs(1);
        G_Say(ent, 0, mode, p);
    }
}

void Cmd_PrintEntities_f()
{
    G_PrintEntities();
}

void Cmd_VisionSetNaked_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char *v2; // eax
    float v3; // [esp+0h] [ebp-1Ch]
    int32_t v4; // [esp+4h] [ebp-18h]
    float v5; // [esp+8h] [ebp-14h]
    int32_t duration; // [esp+18h] [ebp-4h]

    duration = 1000;
    v4 = SV_Cmd_Argc();
    if (v4 == 2)
        goto LABEL_4;
    if (v4 == 3)
    {
        v0 = SV_Cmd_Argv(2);
        v3 = atof(v0);
        duration = (int)(v3 * 1000.0f);
    LABEL_4:
        v1 = SV_Cmd_Argv(1);
        v2 = va("\"%s\" %i", v1, duration);
        SV_SetConfigstring(824, v2);
        return;
    }
    Com_Printf(0, "USAGE: visionSetNaked <name> <duration>\n");
}

void Cmd_VisionSetNight_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char *v2; // eax
    float v3; // [esp+0h] [ebp-1Ch]
    int32_t v4; // [esp+4h] [ebp-18h]
    float v5; // [esp+8h] [ebp-14h]
    int32_t duration; // [esp+18h] [ebp-4h]

    duration = 1000;
    v4 = SV_Cmd_Argc();
    if (v4 == 2)
        goto LABEL_4;
    if (v4 == 3)
    {
        v0 = SV_Cmd_Argv(2);
        v3 = atof(v0);
        duration = (int)(v3 * 1000.0f);
    LABEL_4:
        v1 = SV_Cmd_Argv(1);
        v2 = va("\"%s\" %i", v1, duration);
        SV_SetConfigstring(825, v2);
        return;
    }
    Com_Printf(0, "USAGE: visionSetNight <name> <duration>\n");
}

