#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "server_mp.h"
#include <qcommon/files.h>
#include <qcommon/cmd.h>
#include <win32/win_net_debug.h>
#include <universal/com_files.h>
#include <qcommon/com_playerprofile.h>
#include <script/scr_debugger.h>
#include <qcommon/mem_track.h>
#include <server/sv_game.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_public_mp.h>
#include <script/scr_vm.h>
#include <script/scr_memorytree.h>
#include <database/database.h>

#ifdef WIN32
#include <win32/win_steam.h>
#else
#error Steam auth for Arch(Server)
#endif

int sv_serverId_value;

// Jesus christ these are ugly, koutsie go fix this
const char ERRMSG_PLURAL[85] =
{
  '\x15',
  '\"',
  '%',
  's',
  '\"',
  '\n',
  '-',
  '\n',
  'T',
  'h',
  'e',
  'r',
  'e',
  ' ',
  'w',
  'e',
  'r',
  'e',
  ' ',
  '%',
  'u',
  ' ',
  'e',
  'r',
  'r',
  'o',
  'r',
  's',
  ' ',
  'w',
  'h',
  'e',
  'n',
  ' ',
  'l',
  'o',
  'a',
  'd',
  'i',
  'n',
  'g',
  ' ',
  't',
  'h',
  'i',
  's',
  ' ',
  'm',
  'a',
  'p',
  '.',
  '\n',
  'S',
  'e',
  'e',
  ' ',
  't',
  'h',
  'e',
  ' ',
  'c',
  'o',
  'n',
  's',
  'o',
  'l',
  'e',
  ' ',
  'l',
  'o',
  'g',
  ' ',
  'f',
  'o',
  'r',
  ' ',
  'd',
  'e',
  't',
  'a',
  'i',
  'l',
  's',
  '.',
  '\0'
}; // idb
const char ERRMSG_SINGLE[83] =
{
  '\x15',
  '\"',
  '%',
  's',
  '\"',
  '\n',
  '-',
  '\n',
  'T',
  'h',
  'e',
  'r',
  'e',
  ' ',
  'w',
  'a',
  's',
  ' ',
  '%',
  'u',
  ' ',
  'e',
  'r',
  'r',
  'o',
  'r',
  ' ',
  'w',
  'h',
  'e',
  'n',
  ' ',
  'l',
  'o',
  'a',
  'd',
  'i',
  'n',
  'g',
  ' ',
  't',
  'h',
  'i',
  's',
  ' ',
  'm',
  'a',
  'p',
  '.',
  '\n',
  'S',
  'e',
  'e',
  ' ',
  't',
  'h',
  'e',
  ' ',
  'c',
  'o',
  'n',
  's',
  'o',
  'l',
  'e',
  ' ',
  'l',
  'o',
  'g',
  ' ',
  'f',
  'o',
  'r',
  ' ',
  'd',
  'e',
  't',
  'a',
  'i',
  'l',
  's',
  '.',
  '\0'
}; // idb

char *__cdecl SV_GetMapBaseName(char *mapname)
{
    return FS_GetMapBaseName(mapname);
}

void __cdecl SV_Heartbeat_f()
{
    svs.nextHeartbeatTime = 0x80000000;
}

void __cdecl SV_GameCompleteStatus_f()
{
    SV_MasterGameCompleteStatus();
}

cmd_function_s SV_Heartbeat_f_VAR;
cmd_function_s SV_Heartbeat_f_VAR_SERVER;
cmd_function_s SV_Drop_f_VAR;
cmd_function_s SV_Drop_f_VAR_SERVER;
cmd_function_s SV_Ban_f_VAR;
cmd_function_s SV_Ban_f_VAR_SERVER;
cmd_function_s SV_BanNum_f_VAR;
cmd_function_s SV_BanNum_f_VAR_SERVER;
cmd_function_s SV_TempBan_f_VAR_0;
cmd_function_s SV_TempBan_f_VAR_SERVER_0;
cmd_function_s SV_TempBan_f_VAR;
cmd_function_s SV_TempBan_f_VAR_SERVER;
cmd_function_s SV_TempBanNum_f_VAR;
cmd_function_s SV_TempBanNum_f_VAR_SERVER;
cmd_function_s SV_Unban_f_VAR;
cmd_function_s SV_Unban_f_VAR_SERVER;
cmd_function_s SV_DropNum_f_VAR;
cmd_function_s SV_DropNum_f_VAR_SERVER;
cmd_function_s SV_Status_f_VAR;
cmd_function_s SV_Status_f_VAR_SERVER;
cmd_function_s SV_Serverinfo_f_VAR;
cmd_function_s SV_Serverinfo_f_VAR_SERVER;
cmd_function_s SV_Systeminfo_f_VAR;
cmd_function_s SV_Systeminfo_f_VAR_SERVER;
cmd_function_s SV_DumpUser_f_VAR;
cmd_function_s SV_DumpUser_f_VAR_SERVER;
cmd_function_s SV_MapRestart_f_VAR;
cmd_function_s SV_MapRestart_f_VAR_SERVER;
cmd_function_s SV_FastRestart_f_VAR;
cmd_function_s SV_FastRestart_f_VAR_SERVER;
cmd_function_s SV_Map_f_VAR_0;
cmd_function_s SV_Map_f_VAR_SERVER_0;
cmd_function_s SV_MapRotate_f_VAR;
cmd_function_s SV_MapRotate_f_VAR_SERVER;
cmd_function_s SV_GameCompleteStatus_f_VAR;
cmd_function_s SV_GameCompleteStatus_f_VAR_SERVER;
cmd_function_s SV_Map_f_VAR;
cmd_function_s SV_Map_f_VAR_SERVER;
cmd_function_s SV_KillServer_f_VAR;
cmd_function_s SV_KillServer_f_VAR_SERVER;
cmd_function_s SV_ScriptUsage_f_VAR;
cmd_function_s SV_ScriptUsage_f_VAR_SERVER;
cmd_function_s SV_ScriptDebugger_f_VAR;
cmd_function_s SV_ScriptDebugger_f_VAR_SERVER;
cmd_function_s SV_ScriptVarUsage_f_VAR;
cmd_function_s SV_ScriptVarUsage_f_VAR_SERVER;
cmd_function_s SV_StringUsage_f_VAR;
cmd_function_s SV_StringUsage_f_VAR_SERVER;
cmd_function_s SV_ScriptProfile_f_VAR;
cmd_function_s SV_ScriptProfile_f_VAR_SERVER;
cmd_function_s SV_ScriptBuiltin_f_VAR;
cmd_function_s SV_ScriptBuiltin_f_VAR_SERVER;
cmd_function_s SV_SetPerk_f_VAR;
cmd_function_s SV_SetPerk_f_VAR_SERVER;


void __cdecl SV_ScriptDebugger_f()
{
    if (Sys_IsRemoteDebugClient())
    {
        Scr_RunDebuggerRemote();
    }
    else if (sv.state == SS_GAME)
    {
        Scr_RunDebugger();
    }
}

void __cdecl SV_AddOperatorCommands()
{
    static int initialized_0 = 0;
    if (!initialized_0)
    {
        initialized_0 = 1;
        Cmd_AddCommandInternal("heartbeat", Cbuf_AddServerText_f, &SV_Heartbeat_f_VAR);
        Cmd_AddServerCommandInternal("heartbeat", SV_Heartbeat_f, &SV_Heartbeat_f_VAR_SERVER);
        Cmd_AddCommandInternal("onlykick", Cbuf_AddServerText_f, &SV_Drop_f_VAR);
        Cmd_AddServerCommandInternal("onlykick", SV_Drop_f, &SV_Drop_f_VAR_SERVER);
        Cmd_AddCommandInternal("banUser", Cbuf_AddServerText_f, &SV_Ban_f_VAR);
        Cmd_AddServerCommandInternal("banUser", SV_Ban_f, &SV_Ban_f_VAR_SERVER);
        Cmd_AddCommandInternal("banClient", Cbuf_AddServerText_f, &SV_BanNum_f_VAR);
        Cmd_AddServerCommandInternal("banClient", SV_BanNum_f, &SV_BanNum_f_VAR_SERVER);
        Cmd_AddCommandInternal("kick", Cbuf_AddServerText_f, &SV_TempBan_f_VAR_0);
        Cmd_AddServerCommandInternal("kick", SV_TempBan_f, &SV_TempBan_f_VAR_SERVER_0);
        Cmd_AddCommandInternal("tempBanUser", Cbuf_AddServerText_f, &SV_TempBan_f_VAR);
        Cmd_AddServerCommandInternal("tempBanUser", SV_TempBan_f, &SV_TempBan_f_VAR_SERVER);
        Cmd_AddCommandInternal("tempBanClient", Cbuf_AddServerText_f, &SV_TempBanNum_f_VAR);
        Cmd_AddServerCommandInternal("tempBanClient", SV_TempBanNum_f, &SV_TempBanNum_f_VAR_SERVER);
        Cmd_AddCommandInternal("unbanUser", Cbuf_AddServerText_f, &SV_Unban_f_VAR);
        Cmd_AddServerCommandInternal("unbanUser", SV_Unban_f, &SV_Unban_f_VAR_SERVER);
        Cmd_AddCommandInternal("clientkick", Cbuf_AddServerText_f, &SV_DropNum_f_VAR);
        Cmd_AddServerCommandInternal("clientkick", SV_DropNum_f, &SV_DropNum_f_VAR_SERVER);
        Cmd_AddCommandInternal("status", Cbuf_AddServerText_f, &SV_Status_f_VAR);
        Cmd_AddServerCommandInternal("status", SV_Status_f, &SV_Status_f_VAR_SERVER);
        Cmd_AddCommandInternal("serverinfo", Cbuf_AddServerText_f, &SV_Serverinfo_f_VAR);
        Cmd_AddServerCommandInternal("serverinfo", SV_Serverinfo_f, &SV_Serverinfo_f_VAR_SERVER);
        Cmd_AddCommandInternal("systeminfo", Cbuf_AddServerText_f, &SV_Systeminfo_f_VAR);
        Cmd_AddServerCommandInternal("systeminfo", SV_Systeminfo_f, &SV_Systeminfo_f_VAR_SERVER);
        Cmd_AddCommandInternal("dumpuser", Cbuf_AddServerText_f, &SV_DumpUser_f_VAR);
        Cmd_AddServerCommandInternal("dumpuser", SV_DumpUser_f, &SV_DumpUser_f_VAR_SERVER);
        Cmd_AddCommandInternal("map_restart", Cbuf_AddServerText_f, &SV_MapRestart_f_VAR);
        Cmd_AddServerCommandInternal("map_restart", SV_MapRestart_f, &SV_MapRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("fast_restart", Cbuf_AddServerText_f, &SV_FastRestart_f_VAR);
        Cmd_AddServerCommandInternal("fast_restart", SV_FastRestart_f, &SV_FastRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("map", Cbuf_AddServerText_f, &SV_Map_f_VAR_0);
        Cmd_AddServerCommandInternal("map", SV_Map_f, &SV_Map_f_VAR_SERVER_0);
        Cmd_SetAutoComplete("map", "maps/mp", "d3dbsp");
        Cmd_AddCommandInternal("map_rotate", Cbuf_AddServerText_f, &SV_MapRotate_f_VAR);
        Cmd_AddServerCommandInternal("map_rotate", SV_MapRotate_f, &SV_MapRotate_f_VAR_SERVER);
        Cmd_AddCommandInternal("gameCompleteStatus", Cbuf_AddServerText_f, &SV_GameCompleteStatus_f_VAR);
        Cmd_AddServerCommandInternal("gameCompleteStatus", SV_GameCompleteStatus_f, &SV_GameCompleteStatus_f_VAR_SERVER);
        Cmd_AddCommandInternal("devmap", Cbuf_AddServerText_f, &SV_Map_f_VAR);
        Cmd_AddServerCommandInternal("devmap", SV_Map_f, &SV_Map_f_VAR_SERVER);
        Cmd_SetAutoComplete("devmap", "maps/mp", "d3dbsp");
        Cmd_AddCommandInternal("killserver", Cbuf_AddServerText_f, &SV_KillServer_f_VAR);
        Cmd_AddServerCommandInternal("killserver", SV_KillServer_f, &SV_KillServer_f_VAR_SERVER);
        if (com_dedicated->current.integer)
        {
            SV_AddDedicatedCommands();
            // LWSS ADD
            Steam_SV_AddTestCommands();
            // LWSS END
        }
        Cmd_AddCommandInternal("scriptUsage", Cbuf_AddServerText_f, &SV_ScriptUsage_f_VAR);
        Cmd_AddServerCommandInternal("scriptUsage", SV_ScriptUsage_f, &SV_ScriptUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("scriptDebugger", Cbuf_AddServerText_f, &SV_ScriptDebugger_f_VAR);
        Cmd_AddServerCommandInternal("scriptDebugger", SV_ScriptDebugger_f, &SV_ScriptDebugger_f_VAR_SERVER);
        Cmd_AddCommandInternal("scriptVarUsage", Cbuf_AddServerText_f, &SV_ScriptVarUsage_f_VAR);
        Cmd_AddServerCommandInternal("scriptVarUsage", SV_ScriptVarUsage_f, &SV_ScriptVarUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("stringUsage", Cbuf_AddServerText_f, &SV_StringUsage_f_VAR);
        Cmd_AddServerCommandInternal("stringUsage", SV_StringUsage_f, &SV_StringUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("scriptProfile", Cbuf_AddServerText_f, &SV_ScriptProfile_f_VAR);
        Cmd_AddServerCommandInternal("scriptProfile", SV_ScriptProfile_f, &SV_ScriptProfile_f_VAR_SERVER);
        Cmd_AddCommandInternal("scriptBuiltin", Cbuf_AddServerText_f, &SV_ScriptBuiltin_f_VAR);
        Cmd_AddServerCommandInternal("scriptBuiltin", SV_ScriptBuiltin_f, &SV_ScriptBuiltin_f_VAR_SERVER);
        Cmd_AddCommandInternal("setPerk", Cbuf_AddServerText_f, &SV_SetPerk_f_VAR);
        Cmd_AddServerCommandInternal("setPerk", SV_SetPerk_f, &SV_SetPerk_f_VAR_SERVER);
    }
}

char __cdecl SV_CheckMapExists(const char *map)
{
    char expanded[68]; // [esp+0h] [ebp-48h] BYREF

    Com_GetBspFilename(expanded, 0x40u, map);
    if (FS_ReadFile(expanded, 0) != -1)
        return 1;
    Com_PrintError(1, "Can't find map %s\n", expanded);
    return 0;
}

void __cdecl SV_Map_f()
{
    const char *v0; // eax
    char *v1; // eax
    const char *v2; // eax
    const char *v3; // [esp-4h] [ebp-58h]
    char *map; // [esp+0h] [ebp-54h]
    char mapname[68]; // [esp+4h] [ebp-50h] BYREF
    bool cheat; // [esp+4Fh] [ebp-5h]
    const char *basename; // [esp+50h] [ebp-4h]

    map = (char *)SV_Cmd_Argv(1);
    iassert(map);
    if (*map)
    {
        com_errorPrintsCount = 0;
        if (!Com_HasPlayerProfile() && !com_dedicated->current.enabled)
        {
            Com_Error(ERR_DROP, "PLATFORM_NOTSIGNEDINTOPROFILE");
            return;
        }
        Cbuf_ExecuteBuffer(0, 0, "selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
        if (com_dedicated->latched.integer != com_dedicated->current.integer)
        {
            v3 = SV_Cmd_Argv(1);
            v0 = SV_Cmd_Argv(0);
            v1 = va("wait; wait; %s %s;", v0, v3);
            Cbuf_InsertText(0, v1);
            return;
        }
        basename = SV_GetMapBaseName(map);
        I_strncpyz(mapname, (char *)basename, 64);
        I_strlwr(mapname);
        if (useFastFile->current.enabled)
        {
            if (!fs_gameDirVar)
                MyAssertHandler(".\\server_mp\\sv_ccmds_mp.cpp", 239, 0, "%s", "fs_gameDirVar");
            if (!DB_FileSize(mapname, 0) && (!*(_BYTE *)fs_gameDirVar->current.integer || !DB_FileSize(mapname, 1)))
            {
                Com_PrintError(1, "Can't find map \"%s\".\n", mapname);
                return;
            }
        }
        else if (!SV_CheckMapExists(mapname))
        {
            return;
        }
        FS_ConvertPath(mapname);
        SV_SpawnServer(mapname);
        v2 = SV_Cmd_Argv(0);
        cheat = I_stricmp(v2, "devmap") == 0;
        Dvar_SetBool((dvar_s *)sv_cheats, cheat);
        ShowLoadErrorsSummary(map, com_errorPrintsCount);
    }
}

void __cdecl ShowLoadErrorsSummary(const char *mapName, unsigned int count)
{
    if (com_errorPrintsCount)
    {
        if (com_dedicated->current.enabled)
        {
            if (count == 1)
                Com_PrintError(16, (char *)ERRMSG_SINGLE, mapName, com_errorPrintsCount);
            else
                Com_PrintError(16, (char *)ERRMSG_PLURAL, mapName, com_errorPrintsCount);
        }
        else if (count == 1)
        {
            Com_Error(ERR_MAPLOADERRORSUMMARY, ERRMSG_SINGLE, mapName, com_errorPrintsCount);
        }
        else
        {
            Com_Error(ERR_MAPLOADERRORSUMMARY, ERRMSG_PLURAL, mapName, com_errorPrintsCount);
        }
    }
}

void __cdecl SV_MapRestart_f()
{
    SV_MapRestart(0);
}

void __cdecl SV_MapRestart(int fast_restart)
{
    char *String; // eax
    char *v2; // eax
    client_t *client; // [esp+0h] [ebp-58h]
    client_t *clienta; // [esp+0h] [ebp-58h]
    const char *denied; // [esp+4h] [ebp-54h]
    int savepersist; // [esp+8h] [ebp-50h]
    int i; // [esp+Ch] [ebp-4Ch]
    int ia; // [esp+Ch] [ebp-4Ch]
    signed int ib; // [esp+Ch] [ebp-4Ch]
    char mapname[68]; // [esp+10h] [ebp-48h] BYREF

    Com_SyncThreads();
    track_hunk_ClearToStart();
    if (com_sv_running->current.enabled)
    {
        SV_SetGametype();
        I_strncpyz(sv.gametype, (char *)sv_gametype->current.integer, 64);
        savepersist = G_GetSavePersist();
        if (sv_maxclients->modified || I_stricmp(sv.gametype, sv_gametype->current.string) || !fast_restart)
        {
            G_SetSavePersist(0);
            String = (char *)Dvar_GetString("mapname");
            I_strncpyz(mapname, String, 64);
            FS_ConvertPath(mapname);
            SV_SpawnServer(mapname);
        }
        else if (com_frameTime != sv.start_frameTime)
        {
            for (i = 0; i < sv_maxclients->current.integer; ++i)
            {
                client = &svs.clients[i];
                if (client->header.state >= 2)
                    NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "fastrestart");
            }
            SV_InitDvar();
            SV_InitArchivedSnapshot();
            SV_InitSnapshot();
            svs.snapFlagServerBit ^= 4u;
            sv_serverId_value = (((_BYTE)sv_serverId_value + 1) & 0xF) + (sv_serverId_value & 0xF0);
            Dvar_SetInt((dvar_s *)sv_serverid, sv_serverId_value);
            sv.start_frameTime = com_frameTime;
            sv.state = SS_LOADING;
            sv.restarting = 1;
            SV_RestartGameProgs(savepersist);
            for (ia = 0; ia < 3; ++ia)
            {
                svs.time += 100;
                SV_RunFrame();
            }
            for (ib = 0; ib < sv_maxclients->current.integer; ++ib)
            {
                clienta = &svs.clients[ib];
                if (clienta->header.state >= 2)
                {
                    v2 = va("%c", savepersist != 0 ? 110 : 66);
                    SV_AddServerCommand(clienta, SV_CMD_RELIABLE, v2);
                    denied = ClientConnect(ib, clienta->scriptId);
                    if (denied)
                    {
                        SV_DropClient(clienta, denied, 1);
                        Com_Printf(0, "SV_MapRestart_f: dropped client %i - denied!\n", ib);
                    }
                    else if (clienta->header.state == 4)
                    {
                        SV_ClientEnterWorld(clienta, &clienta->lastUsercmd);
                    }
                }
            }
            sv.state = SS_GAME;
            sv.restarting = 0;
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_FastRestart_f()
{
    SV_MapRestart(1);
}

parseInfo_t *__cdecl SV_GetMapRotationToken()
{
    parseInfo_t *token; // [esp+0h] [ebp-8h]
    const char *value; // [esp+4h] [ebp-4h] BYREF

    value = sv_mapRotationCurrent->current.string;
    token = Com_Parse(&value);
    if (value)
    {
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, (char *)value);
        return token;
    }
    else
    {
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, (char *)"");
        return 0;
    }
}

void __cdecl SV_MapRotate_f()
{
    char *v0; // eax
    parseInfo_t *token; // [esp+0h] [ebp-4h]
    parseInfo_t *tokena; // [esp+0h] [ebp-4h]
    parseInfo_t *tokenb; // [esp+0h] [ebp-4h]

    Com_Printf(0, "map_rotate...\n\n");
    Com_Printf(0, "\"sv_mapRotation\" is:\"%s\"\n\n", sv_mapRotation->current.string);
    Com_Printf(0, "\"sv_mapRotationCurrent\" is:\"%s\"\n\n", sv_mapRotationCurrent->current.string);
    if (!*(_BYTE *)sv_mapRotationCurrent->current.integer)
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, (char *)sv_mapRotation->current.integer);
    token = SV_GetMapRotationToken();
    if (!token)
    {
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, (char *)sv_mapRotation->current.integer);
        token = SV_GetMapRotationToken();
    }
    while (1)
    {
        if (!token)
        {
            Com_Printf(0, "No map specified in sv_mapRotation - forcing map_restart.\n");
            SV_FastRestart_f();
            return;
        }
        if (I_stricmp(token->token, "gametype"))
            break;
        tokena = SV_GetMapRotationToken();
        if (!tokena)
        {
            Com_Printf(0, "No gametype specified after 'gametype' keyword in sv_mapRotation - forcing map_restart.\n");
            SV_FastRestart_f();
            return;
        }
        Com_Printf(0, "Setting g_gametype: %s.\n", tokena->token);
        if (com_sv_running->current.enabled)
        {
            if (I_stricmp(sv_gametype->current.string, tokena->token))
                G_SetSavePersist(0);
        }
        Dvar_SetString((dvar_s *)sv_gametype, tokena->token);
    LABEL_19:
        token = SV_GetMapRotationToken();
    }
    if (I_stricmp(token->token, "map"))
    {
        Com_Printf(0, "Unknown keyword '%s' in sv_mapRotation.\n", token->token);
        goto LABEL_19;
    }
    tokenb = SV_GetMapRotationToken();
    if (tokenb)
    {
        Com_Printf(0, "Setting map: %s.\n", tokenb->token);
        v0 = va("map %s\n", tokenb->token);
        Cmd_ExecuteSingleCommand(0, 0, v0);
    }
    else
    {
        Com_Printf(0, "No map specified after 'map' keyword in sv_mapRotation - forcing map_restart.\n");
        SV_FastRestart_f();
    }
}

void __cdecl SV_TempBan_f()
{
    char cdkeyHash[36]; // [esp+0h] [ebp-6Ch] BYREF
    char playerName[68]; // [esp+24h] [ebp-48h] BYREF

    if (SV_KickUser_f(playerName, 64, cdkeyHash))
    {
        Com_Printf(0, "%s (guid \"%s\") was kicked for cheating\n", playerName, cdkeyHash);
        SV_BanGuidBriefly(cdkeyHash);
    }
}

int __cdecl SV_KickUser_f(char *playerName, int maxPlayerNameLen, char *cdkeyHash)
{
    const char *v4; // eax
    client_t *PlayerByName; // [esp+0h] [ebp-Ch]
    client_t *clients; // [esp+0h] [ebp-Ch]
    int clientNum; // [esp+4h] [ebp-8h]
    const char *cmdName; // [esp+8h] [ebp-4h]

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() == 2)
        {
            PlayerByName = SV_GetPlayerByName();
            if (PlayerByName)
            {
                return SV_KickClient(PlayerByName, playerName, maxPlayerNameLen, cdkeyHash);
            }
            else
            {
                v4 = SV_Cmd_Argv(1);
                if (!I_stricmp(v4, "all"))
                {
                    clientNum = 0;
                    clients = svs.clients;
                    while (clientNum < sv_maxclients->current.integer)
                    {
                        if (clients->header.state)
                            SV_KickClient(clients, 0, 0, cdkeyHash);
                        ++clientNum;
                        ++clients;
                    }
                }
                return 0;
            }
        }
        else
        {
            cmdName = SV_Cmd_Argv(0);
            Com_Printf(0, "Usage: %s <player name>\n%s all = kick everyone\n", cmdName, cmdName);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
        return 0;
    }
}

client_t *__cdecl SV_GetPlayerByName()
{
    client_t *clients; // [esp+0h] [ebp-54h]
    const char *s; // [esp+4h] [ebp-50h]
    int i; // [esp+8h] [ebp-4Ch]
    char cleanName[68]; // [esp+Ch] [ebp-48h] BYREF

    if (!com_sv_running->current.enabled)
        return 0;
    if (SV_Cmd_Argc() >= 2)
    {
        s = SV_Cmd_Argv(1);
        i = 0;
        clients = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (clients->header.state)
            {
                if (!I_stricmp(clients->name, s))
                    return clients;
                I_strncpyz(cleanName, clients->name, 64);
                I_CleanStr(cleanName);
                if (!I_stricmp(cleanName, s))
                    return clients;
            }
            ++i;
            ++clients;
        }
        Com_Printf(0, "Player %s is not on the server\n", s);
        return 0;
    }
    else
    {
        Com_Printf(0, "No player specified.\n");
        return 0;
    }
}

int __cdecl SV_KickClient(client_t *cl, char *playerName, int maxPlayerNameLen, char *cdkeyHash)
{
    if (!cl)
        MyAssertHandler(".\\server_mp\\sv_ccmds_mp.cpp", 537, 0, "%s", "cl");
    if (cl->header.netchan.remoteAddress.type == NA_LOOPBACK)
    {
        SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c \"EXE_CANNOTKICKHOSTPLAYER\"", 101);
        return 0;
    }
    else
    {
        if (playerName)
        {
            I_strncpyz(playerName, cl->name, maxPlayerNameLen);
            I_CleanStr(playerName);
        }
        memcpy(cdkeyHash, cl->cdkeyHash, 33);
        SV_DropClient(cl, "EXE_PLAYERKICKED", 1);
        cl->lastPacketTime = svs.time;
        return 1;
    }
}

void __cdecl SV_Ban_f()
{
    client_t *PlayerByName; // [esp+0h] [ebp-4h]

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() == 2)
        {
            PlayerByName = SV_GetPlayerByName();
            if (PlayerByName)
                SV_BanClient(PlayerByName);
        }
        else
        {
            Com_Printf(0, "Usage: banUser <player name>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_BanNum_f()
{
    client_t *PlayerByNum; // [esp+0h] [ebp-4h]

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() == 2)
        {
            PlayerByNum = SV_GetPlayerByNum();
            if (PlayerByNum)
                SV_BanClient(PlayerByNum);
        }
        else
        {
            Com_Printf(0, "Usage: banClient <client number>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

client_t *__cdecl SV_GetPlayerByNum()
{
    int idnum; // [esp+4h] [ebp-Ch]
    const char *s; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    if (!com_sv_running->current.enabled)
        return 0;
    if (SV_Cmd_Argc() >= 2)
    {
        s = SV_Cmd_Argv(1);
        for (i = 0; s[i]; ++i)
        {
            if (s[i] < 48 || s[i] > 57)
            {
                Com_Printf(0, "Bad slot number: %s\n", s);
                return 0;
            }
        }
        idnum = atoi(s);
        if (idnum >= 0 && idnum < sv_maxclients->current.integer)
        {
            if (svs.clients[idnum].header.state)
            {
                return &svs.clients[idnum];
            }
            else
            {
                Com_Printf(0, "Client %i is not active\n", idnum);
                return 0;
            }
        }
        else
        {
            Com_Printf(0, "Bad client slot: %i\n", idnum);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "No player specified.\n");
        return 0;
    }
}

void __cdecl SV_Unban_f()
{
    char *v0; // eax

    if (SV_Cmd_Argc() == 2)
    {
        v0 = (char *)SV_Cmd_Argv(1);
        SV_UnbanClient(v0);
    }
    else
    {
        Com_Printf(0, "Usage: unban <client name>\n");
    }
}

void __cdecl SV_Drop_f()
{
    char cdkeyHash[36]; // [esp+0h] [ebp-28h] BYREF

    SV_KickUser_f(0, 0, cdkeyHash);
}

void __cdecl SV_DropNum_f()
{
    char cdkeyHash[36]; // [esp+0h] [ebp-28h] BYREF

    SV_KickClient_f(0, 0, cdkeyHash);
}

int __cdecl SV_KickClient_f(char *playerName, int maxPlayerNameLen, char *cdkeyHash)
{
    const char *v4; // eax
    client_t *PlayerByNum; // [esp+0h] [ebp-4h]

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() == 2)
        {
            PlayerByNum = SV_GetPlayerByNum();
            if (PlayerByNum)
                return SV_KickClient(PlayerByNum, playerName, maxPlayerNameLen, cdkeyHash);
            else
                return 0;
        }
        else
        {
            v4 = SV_Cmd_Argv(0);
            Com_Printf(0, "Usage: %s <client number>\n", v4);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
        return 0;
    }
}

void __cdecl SV_TempBanNum_f()
{
    char cdkeyHash[36]; // [esp+0h] [ebp-6Ch] BYREF
    char playerName[68]; // [esp+24h] [ebp-48h] BYREF

    if (SV_KickClient_f(playerName, 64, cdkeyHash))
    {
        Com_Printf(0, "%s (guid \"%s\") was kicked for cheating\n", playerName, cdkeyHash);
        SV_BanGuidBriefly(cdkeyHash);
    }
}

void __cdecl SV_Status_f()
{
    int ClientScore; // eax
    unsigned int v1; // kr00_4
    int j; // [esp+14h] [ebp-1Ch]
    int ja; // [esp+14h] [ebp-1Ch]
    client_t *clients; // [esp+18h] [ebp-18h]
    int l; // [esp+1Ch] [ebp-14h]
    const char *s; // [esp+24h] [ebp-Ch]
    int i; // [esp+28h] [ebp-8h]

    if (com_sv_running->current.enabled)
    {
        Com_Printf(0, "map: %s\n", sv_mapname->current.string);
        Com_Printf(
            0,
            "num score ping guid                             name            lastmsg address               qport rate\n");
        Com_Printf(
            0,
            "--- ----- ---- -------------------------------- --------------- ------- --------------------- ----- -----\n");
        i = 0;
        clients = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (clients->header.state)
            {
                Com_Printf(0, "%3i ", i);
                SV_GameClientNum(i);
                ClientScore = G_GetClientScore(clients - svs.clients);
                Com_Printf(0, "%5i ", ClientScore);
                if (clients->header.state == 2)
                {
                    Com_Printf(0, "CNCT ");
                }
                else if (clients->header.state == 1)
                {
                    Com_Printf(0, "ZMBI ");
                }
                else if (clients->ping >= 9999)
                {
                    Com_Printf(0, "%4i ", 9999);
                }
                else
                {
                    Com_Printf(0, "%4i ", clients->ping);
                }
                Com_Printf(0, "%32s ", clients->cdkeyHash);
                Com_Printf(0, "%s^7", clients->name);
                l = 16 - I_DrawStrlen(clients->name);
                for (j = 0; j < l; ++j)
                    Com_Printf(0, " ");
                Com_Printf(0, "%7i ", svs.time - clients->lastPacketTime);
                s = NET_AdrToString(clients->header.netchan.remoteAddress);
                Com_Printf(0, "%s", s);
                v1 = strlen(s);
                for (ja = 0; ja < (int)(22 - v1); ++ja)
                    Com_Printf(0, " ");
                Com_Printf(0, "%5i", clients->header.netchan.qport);
                Com_Printf(0, " %5i", clients->rate);
                Com_Printf(0, "\n");
            }
            ++i;
            ++clients;
        }
        Com_Printf(0, "\n");
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_Serverinfo_f()
{
    char *v0; // eax

    Com_Printf(0, "Server info settings:\n");
    v0 = Dvar_InfoString(0, 4);
    Info_Print(v0);
}

void __cdecl SV_Systeminfo_f()
{
    char *v0; // eax

    Com_Printf(0, "System info settings:\n");
    v0 = Dvar_InfoString(0, 8);
    Info_Print(v0);
}

void __cdecl SV_DumpUser_f()
{
    client_t *PlayerByName; // [esp+0h] [ebp-4h]

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() == 2)
        {
            PlayerByName = SV_GetPlayerByName();
            if (PlayerByName)
            {
                Com_Printf(0, "userinfo\n");
                Com_Printf(0, "--------\n");
                Info_Print(PlayerByName->userinfo);
            }
        }
        else
        {
            Com_Printf(0, "Usage: info <userid>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_KillServer_f()
{
    Com_Shutdown("EXE_SERVERKILLED");
}

void __cdecl SV_ScriptUsage_f()
{
    Scr_DumpScriptThreads();
}

void __cdecl SV_ScriptVarUsage_f()
{
    Scr_DumpScriptVariablesDefault();
}

void __cdecl SV_ScriptProfile_f()
{
  const char *v0; // eax
  float minTime; // [esp+4h] [ebp-4h]

  v0 = SV_Cmd_Argv(1);
  minTime = atof(v0);
  Scr_DoProfile(minTime);
}

void __cdecl SV_ScriptBuiltin_f()
{
    const char *v0; // eax
    float minTime; // [esp+4h] [ebp-4h]

    v0 = SV_Cmd_Argv(1);
    minTime = atof(v0);
    Scr_DoProfileBuiltin(minTime);
}

void __cdecl SV_StringUsage_f()
{
    MT_DumpTree();
}

void __cdecl SV_SetPerk_f()
{
    clientState_s *ClientState; // eax
    client_t *PlayerByName; // [esp+0h] [ebp-18h]
    const char *perkName; // [esp+4h] [ebp-14h]
    unsigned int perkIndex; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    playerState_s *ps; // [esp+10h] [ebp-8h]
    client_t *clIdx; // [esp+14h] [ebp-4h]

    PlayerByName = SV_GetPlayerByName();
    if (PlayerByName)
    {
        perkName = SV_Cmd_Argv(2);
        perkIndex = BG_GetPerkIndexForName(perkName);
        if (perkIndex < 0x14)
        {
            i = 0;
            for (clIdx = svs.clients; i < sv_maxclients->current.integer && clIdx != PlayerByName; ++clIdx)
                ++i;
            if ((unsigned int)i >= sv_maxclients->current.integer)
                MyAssertHandler(
                    ".\\server_mp\\sv_ccmds_mp.cpp",
                    1130,
                    0,
                    "i doesn't index sv_maxclients->current.integer\n\t%i not in [0, %i)",
                    i,
                    sv_maxclients->current.integer);
            ps = SV_GameClientNum(i);
            BG_SetPerk(&ps->perks, perkIndex);
            ClientState = G_GetClientState(i);
            BG_SetPerk(&ClientState->perks, perkIndex);
        }
        else
        {
            Com_DPrintf(0, "Unknown perk: %s\n", perkName);
        }
    }
}

cmd_function_s SV_ConSay_f_VAR;
cmd_function_s SV_ConSay_f_VAR_SERVER;
cmd_function_s SV_ConTell_f_VAR;
cmd_function_s SV_ConTell_f_VAR_SERVER;

void __cdecl SV_AddDedicatedCommands()
{
    SV_RemoveDedicatedCommands();
    Cmd_AddCommandInternal("say", Cbuf_AddServerText_f, &SV_ConSay_f_VAR);
    Cmd_AddServerCommandInternal("say", SV_ConSay_f, &SV_ConSay_f_VAR_SERVER);
    Cmd_AddCommandInternal("tell", Cbuf_AddServerText_f, &SV_ConTell_f_VAR);
    Cmd_AddServerCommandInternal("tell", SV_ConTell_f, &SV_ConTell_f_VAR_SERVER);
}

const char aC_5[] = "%c \""; // idb
void __cdecl SV_ConSay_f()
{
    char text[1028]; // [esp+0h] [ebp-408h] BYREF

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() >= 2)
        {
            SV_AssembleConSayMessage(1, text, 1024);
            SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_5, 104, text);
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_AssembleConSayMessage(int firstArg, char *text, int sizeofText)
{
    unsigned int textLen; // [esp+10h] [ebp-4h]

    strcpy(text, "console: ");
    textLen = 9;
    if (strlen(text) != 9)
        MyAssertHandler(".\\server_mp\\sv_ccmds_mp.cpp", 827, 1, "%s", "textLen == strlen( text )");
    Cmd_ArgsBuffer(firstArg, text + 9, sizeofText - 9);
    if (text[9] == 34)
    {
        while (text[textLen + 1])
        {
            text[textLen - 1] = text[textLen];
            ++textLen;
        }
        text[textLen] = 0;
    }
}

void __cdecl SV_ConTell_f()
{
    const char *v0; // eax
    int clientNum; // [esp+4h] [ebp-40Ch]
    char text[1028]; // [esp+8h] [ebp-408h] BYREF

    if (com_sv_running->current.enabled)
    {
        if (SV_Cmd_Argc() >= 3)
        {
            v0 = SV_Cmd_Argv(1);
            clientNum = atoi(v0);
            if (clientNum >= 0 && clientNum < sv_maxclients->current.integer && svs.clients[clientNum].header.state == 4)
            {
                SV_AssembleConSayMessage(2, text, 1024);
                SV_SendServerCommand(&svs.clients[clientNum], SV_CMD_CAN_IGNORE, aC_5, 104, text);
            }
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_RemoveDedicatedCommands()
{
    Cmd_RemoveCommand("say");
    Cmd_RemoveCommand("tell");
}

