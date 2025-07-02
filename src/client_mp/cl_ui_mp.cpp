#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "client_mp.h"
#include <cgame_mp/cg_local_mp.h>
#include <client/client.h>
#include <universal/com_sndalias.h>
#include <gfx_d3d/r_rendercmds.h>
#include <ui/ui.h>


void __cdecl CL_GetClientState(int localClientNum, uiClientState_s *state)
{
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    state->connectPacketCount = clc->connectPacketCount;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    state->connState = clientUIActives[0].connectionState;
    I_strncpyz(state->servername, cls.servername, 1024);
    I_strncpyz(state->updateInfoString, cls.updateInfoString, 1024);
    I_strncpyz(state->messageString, clc->serverMessage, 1024);
}

void __cdecl CL_SetDisplayHUDWithKeycatchUI(int localClientNum, bool display)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActives[0].displayHUDWithKeycatchUI = display;
}

bool __cdecl CL_AllowPopup(int localClientNum)
{
    connstate_t connstate; // [esp+0h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connstate = clientUIActives[0].connectionState;
    return !CL_GetLocalClientConnection(localClientNum)->demoplaying && connstate == CA_ACTIVE;
}

void __cdecl LAN_ResetPings(int source)
{
    int i; // [esp+4h] [ebp-Ch]
    int count; // [esp+8h] [ebp-8h]
    serverInfo_t *servers; // [esp+Ch] [ebp-4h]

    servers = 0;
    count = 0;
    if (source)
    {
        if (source == 1)
        {
            servers = cls.globalServers;
            count = cls.numglobalservers;
        }
        else if (source == 2)
        {
            servers = cls.favoriteServers;
            count = 128;
        }
    }
    else
    {
        servers = cls.localServers;
        count = 128;
    }
    if (servers)
    {
        for (i = 0; i < count; ++i)
            servers[i].ping = -1;
    }
}

int __cdecl LAN_GetServerCount(int source)
{
    switch (source)
    {
    case 0:
        return cls.numlocalservers;
    case 1:
        return cls.numglobalservers;
    case 2:
        return cls.numfavoriteservers;
    }
    return 0;
}

int __cdecl LAN_WaitServerResponse(int source)
{
    if (source == 1)
        return cls.waitglobalserverresponse;
    else
        return 0;
}

void __cdecl LAN_GetServerInfo(int source, unsigned int n, char *buf, int buflen)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    const char *v20; // eax
    char info[1024]; // [esp+4h] [ebp-408h] BYREF
    serverInfo_t *server; // [esp+408h] [ebp-4h]

    server = 0;
    info[0] = 0;
    if (source)
    {
        if (source == 1)
        {
            if ((n & 0x80000000) == 0 && (int)n < cls.numglobalservers)
                server = &cls.globalServers[n];
        }
        else if (source == 2 && n < 0x80)
        {
            server = &cls.favoriteServers[n];
        }
    }
    else if (n < 0x80)
    {
        server = &cls.localServers[n];
    }
    if (server && buf)
    {
        *buf = 0;
        Info_SetValueForKey(info, "hostname", server->hostName);
        Info_SetValueForKey(info, "mapname", server->mapName);
        v4 = va("%i", server->clients);
        Info_SetValueForKey(info, "clients", v4);
        v5 = va("%i", server->maxClients);
        Info_SetValueForKey(info, "sv_maxclients", v5);
        v6 = va("%i", server->ping);
        Info_SetValueForKey(info, "ping", v6);
        v7 = va("%i", server->minPing);
        Info_SetValueForKey(info, "minping", v7);
        v8 = va("%i", server->maxPing);
        Info_SetValueForKey(info, "maxping", v8);
        Info_SetValueForKey(info, "game", server->game);
        Info_SetValueForKey(info, "gametype", server->gameType);
        v9 = va("%i", server->netType);
        Info_SetValueForKey(info, "nettype", v9);
        v10 = NET_AdrToString(server->adr);
        Info_SetValueForKey(info, "addr", v10);
        v11 = va("%i", server->bPassword);
        Info_SetValueForKey(info, "pswrd", v11);
        v12 = va("%i", server->consoleDisabled);
        Info_SetValueForKey(info, "con_disabled", v12);
        v13 = va("%i", server->pure);
        Info_SetValueForKey(info, "pure", v13);
        v14 = va("%i", server->allowAnonymous);
        Info_SetValueForKey(info, "sv_allowAnonymous", v14);
        v15 = va("%i", server->friendlyfire);
        Info_SetValueForKey(info, "ff", v15);
        v16 = va("%i", server->killcam);
        Info_SetValueForKey(info, "kc", v16);
        v17 = va("%i", server->hardware);
        Info_SetValueForKey(info, "hw", v17);
        v18 = va("%i", server->mod);
        Info_SetValueForKey(info, "mod", v18);
        v19 = va("%i", server->voice);
        Info_SetValueForKey(info, "voice", v19);
        v20 = va("%i", server->punkbuster);
        Info_SetValueForKey(info, "pb", v20);
        I_strncpyz(buf, info, buflen);
    }
    else if (buf)
    {
        *buf = 0;
    }
}

int __cdecl LAN_GetServerPing(int source, unsigned int n)
{
    serverInfo_t *server; // [esp+4h] [ebp-4h]

    server = 0;
    if (source)
    {
        if (source == 1)
        {
            if ((n & 0x80000000) == 0 && (int)n < cls.numglobalservers)
                server = &cls.globalServers[n];
        }
        else if (source == 2 && n < 0x80)
        {
            server = &cls.favoriteServers[n];
        }
    }
    else if (n < 0x80)
    {
        server = &cls.localServers[n];
    }
    if (server)
        return server->ping;
    else
        return -1;
}

serverInfo_t *__cdecl LAN_GetServerPtr(int source, unsigned int n)
{
    if (source)
    {
        if (source == 1)
        {
            if ((n & 0x80000000) == 0 && (int)n < cls.numglobalservers)
                return &cls.globalServers[n];
        }
        else if (source == 2 && n < 0x80)
        {
            return &cls.favoriteServers[n];
        }
    }
    else if (n < 0x80)
    {
        return &cls.localServers[n];
    }
    return 0;
}

void __cdecl LAN_CleanHostname(const char *hostName, char *cleanHostName)
{
    unsigned __int8 c; // [esp+3h] [ebp-1h]

    while (1)
    {
        c = *hostName;
        if (!*hostName)
            break;
        if (isalpha(c))
            *cleanHostName++ = c;
        ++hostName;
    }
    *cleanHostName = 0;
}

int __cdecl LAN_CompareHostname(const char *hostName1, const char *hostName2)
{
    char cleanHostName2[32]; // [esp+4h] [ebp-48h] BYREF
    char cleanHostName1[32]; // [esp+24h] [ebp-28h] BYREF
    int res; // [esp+48h] [ebp-4h]

    LAN_CleanHostname(hostName1, cleanHostName1);
    LAN_CleanHostname(hostName2, cleanHostName2);
    res = I_stricmp(cleanHostName1, cleanHostName2);
    if (res)
        return res;
    else
        return I_stricmp(hostName1, hostName2);
}

int __cdecl LAN_CompareServers(int source, int sortKey, int sortDir, unsigned int s1, unsigned int s2)
{
    char *v6; // eax
    char *v7; // eax
    char *MapDisplayName; // [esp-4h] [ebp-18h]
    char *GameTypeDisplayName; // [esp-4h] [ebp-18h]
    serverInfo_t *server1; // [esp+8h] [ebp-Ch]
    serverInfo_t *server2; // [esp+Ch] [ebp-8h]
    int res; // [esp+10h] [ebp-4h]

    server1 = LAN_GetServerPtr(source, s1);
    server2 = LAN_GetServerPtr(source, s2);
    if (!server1 || !server2)
        return 0;
    res = 0;
    switch (sortKey)
    {
    case 0:
        res = server1->bPassword - server2->bPassword;
        if (!res)
            goto sort_ping;
        break;
    case 1:
        res = server1->hardware - server2->hardware;
        if (!res)
            goto sort_ping;
        if (res >= 0)
        {
            if (!server2->hardware)
                res = -1;
        }
        else if (!server1->hardware)
        {
            res = 1;
        }
        break;
    case 2:
        res = LAN_CompareHostname(server1->hostName, server2->hostName);
        if (!res)
            goto sort_ping;
        break;
    case 3:
        MapDisplayName = UI_GetMapDisplayName(server2->mapName);
        v6 = UI_GetMapDisplayName(server1->mapName);
        res = I_stricmp(v6, MapDisplayName);
        if (!res)
            goto sort_ping;
        break;
    case 4:
        res = server1->clients - server2->clients;
        if (!res)
            goto sort_ping;
        break;
    case 5:
        GameTypeDisplayName = UI_GetGameTypeDisplayName(server2->gameType);
        v7 = UI_GetGameTypeDisplayName(server1->gameType);
        res = I_stricmp(v7, GameTypeDisplayName);
        if (!res)
            goto sort_ping;
        break;
    case 6:
        res = server1->voice - server2->voice;
        if (!res)
            goto sort_ping;
        break;
    case 7:
        res = server1->pure - server2->pure;
        if (!res)
            goto sort_ping;
        break;
    case 8:
        res = server1->mod - server2->mod;
        if (!res)
            goto sort_ping;
        break;
    case 9:
        res = server1->punkbuster - server2->punkbuster;
        if (!res)
        {
        sort_ping:
            sortDir = 0;
            goto $LN3_17;
        }
        break;
    case 10:
    $LN3_17:
        res = server1->ping - server2->ping;
        if (!res)
        {
            res = I_stricmp(server1->gameType, server2->gameType);
            if (!res)
                res = LAN_CompareHostname(server1->hostName, server2->hostName);
        }
        break;
    default:
        break;
    }
    if (sortDir)
        return -res;
    else
        return res;
}

void __cdecl LAN_MarkServerDirty(int source, unsigned int n, unsigned __int8 dirty)
{
    serverInfo_t *server; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]
    int na; // [esp+1Ch] [ebp+Ch]

    if (n == -1)
    {
        count = 128;
        server = 0;
        if (source)
        {
            if (source == 1)
            {
                server = cls.globalServers;
                count = cls.numglobalservers;
            }
            else if (source == 2)
            {
                server = cls.favoriteServers;
            }
        }
        else
        {
            server = cls.localServers;
        }
        if (server)
        {
            for (na = 0; na < count; ++na)
                server[na].dirty = dirty;
        }
    }
    else if (source)
    {
        if (source == 1)
        {
            if ((n & 0x80000000) == 0 && (int)n < cls.numglobalservers)
                cls.globalServers[n].dirty = dirty;
        }
        else if (source == 2 && n < 0x80)
        {
            cls.favoriteServers[n].dirty = dirty;
        }
    }
    else if (n < 0x80)
    {
        cls.localServers[n].dirty = dirty;
    }
}

int __cdecl LAN_ServerIsDirty(int source, unsigned int n)
{
    if (source)
    {
        if (source == 1)
        {
            if ((n & 0x80000000) == 0 && (int)n < cls.numglobalservers)
                return cls.globalServers[n].dirty;
        }
        else if (source == 2 && n < 0x80)
        {
            return cls.favoriteServers[n].dirty;
        }
    }
    else if (n < 0x80)
    {
        return cls.localServers[n].dirty;
    }
    return 0;
}

int __cdecl LAN_UpdateDirtyPings(netsrc_t localClientNum, unsigned int source)
{
    return CL_UpdateDirtyPings(localClientNum, source);
}

void __cdecl Key_KeynumToStringBuf(int keynum, char *buf, int buflen)
{
    char *v3; // eax

    v3 = (char *)Key_KeynumToString(keynum, 1);
    I_strncpyz(buf, v3, buflen);
}

int __cdecl CL_GetClientName(int localClientNum, int index, char *buf, int size)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    *buf = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (!LocalClientGlobals->snap.valid)
        return 0;
    for (i = 0; i < LocalClientGlobals->snap.numClients; ++i)
    {
        if (LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
            & 0x7FF].clientIndex == index)
        {
            I_strncpyz(
                buf,
                LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum) & 0x7FF].name,
                size);
            return 1;
        }
    }
    return 0;
}

int __cdecl CL_ShutdownUI()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    if (!cls.uiStarted)
        return 0;
    Com_UnloadSoundAliases(SASYS_UI);
    Key_RemoveCatcher(0, -17);
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        UI_Shutdown(localClientNum);
    cls.uiStarted = 0;
    return 1;
}

void __cdecl CL_InitUI()
{
    int localClientNum; // [esp+0h] [ebp-8h]
    int remoteScreenUpdateNesting; // [esp+4h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        UI_Init(localClientNum);
    UI_Component_Init();
    remoteScreenUpdateNesting = R_PopRemoteScreenUpdate();
    cls.uiStarted = 1;
    R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
}

