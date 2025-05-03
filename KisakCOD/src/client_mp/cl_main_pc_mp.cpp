#include "client_mp.h"
#include <qcommon/cmd.h>
#include <qcommon/com_playerprofile.h>
#include <sound/snd_public.h>
#include <server_mp/server.h>
#include <client/client.h>
#include <cgame_mp/cg_local_mp.h>

bool s_playerMute[64];

int __cdecl CL_ServerStatus(char *serverAddress, char *serverStatusString, int maxLen)
{
    serverStatus_s *serverStatus; // [esp+0h] [ebp-20h]
    int i; // [esp+4h] [ebp-1Ch]
    netadr_t to; // [esp+8h] [ebp-18h] BYREF

    if (serverAddress)
    {
        if (!NET_StringToAdr(serverAddress, &to))
            return 0;
        serverStatus = CL_GetServerStatus(to);
        if (!serverStatusString)
        {
            serverStatus->retrieved = 1;
            return 0;
        }
        if (NET_CompareAdr(to, serverStatus->address))
        {
            if (!serverStatus->pending)
            {
                I_strncpyz(serverStatusString, serverStatus->string, maxLen);
                serverStatus->retrieved = 1;
                serverStatus->startTime = 0;
                return 1;
            }
            if (serverStatus->startTime < (Sys_Milliseconds() - cl_serverStatusResendTime->current.integer))
            {
                serverStatus->print = 0;
                serverStatus->pending = 1;
                serverStatus->retrieved = 0;
                serverStatus->time = 0;
                serverStatus->startTime = Sys_Milliseconds();
                NET_OutOfBandPrint(NS_CLIENT1, to, "getstatus");
                return 0;
            }
        }
        else if (serverStatus->retrieved)
        {
            serverStatus->address = to;
            serverStatus->print = 0;
            serverStatus->pending = 1;
            serverStatus->retrieved = 0;
            serverStatus->startTime = Sys_Milliseconds();
            serverStatus->time = 0;
            NET_OutOfBandPrint(NS_CLIENT1, to, "getstatus");
            return 0;
        }
        return 0;
    }
    else
    {
        for (i = 0; i < 16; ++i)
        {
            cl_serverStatusList[i].address.port = 0;
            cl_serverStatusList[i].retrieved = 1;
        }
        return 0;
    }
}

void __cdecl CL_SetServerInfoByAddress(netadr_t from, char *info, __int16 ping)
{
    int cmp; // [esp+0h] [ebp-14h]
    int low; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int ia; // [esp+8h] [ebp-Ch]
    int ib; // [esp+8h] [ebp-Ch]
    int ic; // [esp+8h] [ebp-Ch]
    int high; // [esp+Ch] [ebp-8h]

    for (i = 0; i < 128; ++i)
    {
        if (NET_CompareAdr(from, cls.localServers[i].adr))
            CL_SetServerInfo(&cls.localServers[i], info, ping);
    }
    low = 0;
    high = cls.numglobalservers;
    while (low < high)
    {
        ia = (high + low) / 2;
        cmp = NET_CompareAdrSigned(&from, &cls.globalServers[ia].adr);
        if (cmp >= 0)
        {
            if (cmp <= 0)
            {
                do
                    --ia;
                while (ia >= 0 && !NET_CompareAdrSigned(&from, &cls.globalServers[ia].adr));
                ib = ia + 1;
                do
                    CL_SetServerInfo(&cls.globalServers[ib++], info, ping);
                while (ib < cls.numglobalservers && !NET_CompareAdrSigned(&from, &cls.globalServers[ib].adr));
                break;
            }
            low = ia + 1;
        }
        else
        {
            high = (high + low) / 2;
        }
    }
    for (ic = 0; ic < 128; ++ic)
    {
        if (NET_CompareAdr(from, cls.favoriteServers[ic].adr))
            CL_SetServerInfo(&cls.favoriteServers[ic], info, ping);
    }
}

void __cdecl CL_SetServerInfo(serverInfo_t *server, char *info, __int16 ping)
{
    const char *v3; // eax
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
    const char *v21; // eax

    if (server)
    {
        if (info)
        {
            v3 = Info_ValueForKey(info, "clients");
            server->clients = atoi(v3);
            v4 = Info_ValueForKey(info, "hostname");
            I_strncpyz(server->hostName, v4, 32);
            v5 = Info_ValueForKey(info, "mapname");
            I_strncpyz(server->mapName, v5, 32);
            v6 = Info_ValueForKey(info, "sv_maxclients");
            server->maxClients = atoi(v6);
            v7 = Info_ValueForKey(info, "game");
            I_strncpyz(server->game, v7, 24);
            v8 = Info_ValueForKey(info, "gametype");
            I_strncpyz(server->gameType, v8, 16);
            v9 = Info_ValueForKey(info, "nettype");
            server->netType = atoi(v9);
            v10 = Info_ValueForKey(info, "minping");
            server->minPing = atoi(v10);
            v11 = Info_ValueForKey(info, "maxping");
            server->maxPing = atoi(v11);
            v12 = Info_ValueForKey(info, "sv_allowAnonymous");
            server->allowAnonymous = atoi(v12);
            v13 = Info_ValueForKey(info, "con_disabled");
            server->consoleDisabled = atoi(v13);
            v14 = Info_ValueForKey(info, "pswrd");
            server->bPassword = atoi(v14);
            v15 = Info_ValueForKey(info, "pure");
            server->pure = atoi(v15);
            v16 = Info_ValueForKey(info, "ff");
            server->friendlyfire = atoi(v16);
            v17 = Info_ValueForKey(info, "kc");
            server->killcam = atoi(v17);
            v18 = Info_ValueForKey(info, "hw");
            server->hardware = atoi(v18);
            v19 = Info_ValueForKey(info, "mod");
            server->mod = atoi(v19);
            v20 = Info_ValueForKey(info, "voice");
            server->voice = atoi(v20);
            v21 = Info_ValueForKey(info, "pb");
            server->punkbuster = atoi(v21) > 0;
        }
        server->ping = ping;
    }
}

void __cdecl CL_ServerInfoPacket(netadr_t from, msg_t *msg, int time)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *String; // eax
    const char *v7; // eax
    int v8; // [esp+24h] [ebp-424h]
    int prot; // [esp+2Ch] [ebp-41Ch]
    char info[1028]; // [esp+30h] [ebp-418h] BYREF
    const char *ptr; // [esp+434h] [ebp-14h]
    int type; // [esp+438h] [ebp-10h]
    int i; // [esp+43Ch] [ebp-Ch]
    char *infoString; // [esp+440h] [ebp-8h]

    infoString = MSG_ReadString(msg);
    v3 = Info_ValueForKey(infoString, "protocol");
    prot = atoi(v3);
    ptr = Dvar_GetString("debug_protocol");
    if (*ptr)
        v8 = atoi(ptr);
    else
        v8 = 1;
    if (prot == v8)
    {
        for (i = 0; i < 16; ++i)
        {
            if (cl_pinglist[i].adr.port && !cl_pinglist[i].time && NET_CompareAdr(from, cl_pinglist[i].adr))
            {
                cl_pinglist[i].time = time - cl_pinglist[i].start + 1;
                v4 = NET_AdrToString(from);
                Com_DPrintf(14, "ping time %dms from %s\n", cl_pinglist[i].time, v4);
                I_strncpyz(cl_pinglist[i].info, infoString, 1024);
                switch (from.type)
                {
                case NA_BROADCAST:
                case NA_IP:
                    type = 1;
                    break;
                case NA_IPX:
                case NA_BROADCAST_IPX:
                    type = 2;
                    break;
                default:
                    type = 0;
                    break;
                }
                v5 = va("%d", type);
                Info_SetValueForKey(cl_pinglist[i].info, "nettype", v5);
                CL_SetServerInfoByAddress(from, infoString, cl_pinglist[i].time);
                return;
            }
        }
        if (!cls.pingUpdateSource)
        {
            for (i = 0; i < 128 && cls.localServers[i].adr.port; ++i)
            {
                if (NET_CompareAdr(from, cls.localServers[i].adr))
                    return;
            }
            if (i == 128)
            {
                Com_DPrintf(14, "MAX_OTHER_SERVERS hit, dropping infoResponse\n");
            }
            else
            {
                cls.numlocalservers = i + 1;
                cls.localServers[i].adr = from;
                cls.localServers[i].clients = 0;
                cls.localServers[i].hostName[0] = 0;
                cls.localServers[i].mapName[0] = 0;
                cls.localServers[i].maxClients = 0;
                cls.localServers[i].maxPing = 0;
                cls.localServers[i].minPing = 0;
                cls.localServers[i].ping = -1;
                cls.localServers[i].game[0] = 0;
                cls.localServers[i].gameType[0] = 0;
                cls.localServers[i].netType = from.type;
                cls.localServers[i].allowAnonymous = 0;
                cls.localServers[i].punkbuster = 0;
                String = MSG_ReadString(msg);
                I_strncpyz(info, String, 1024);
                if (&info[strlen(info) + 1] != &info[1])
                {
                    if (info[&info[strlen(info) + 1] - &info[1] - 1] != 10)
                        strncat(info, "\n", 0x400u);
                    v7 = NET_AdrToString(from);
                    Com_Printf(14, "%s: %s", v7, info);
                }
            }
        }
    }
    else
    {
        Com_DPrintf(14, "Different protocol info packet: %s\n", infoString);
    }
}

void __cdecl CL_Connect_f()
{
    const char *v0; // eax
    __int16 v1; // ax
    clientConnection_t *clc; // [esp+1Ch] [ebp-8h]
    char *server; // [esp+20h] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        if (clientUIActives[0].connectionState < CA_CONNECTING
            || Cmd_Argc() == 3 && (v0 = Cmd_Argv(2), !I_stricmp(v0, "reconnect")))
        {
            SND_StopSounds(SND_STOP_ALL);
            CL_GetLocalClientGlobals(0);
            clc = CL_GetLocalClientConnection(0);
            clc->serverMessage[0] = 0;
            server = (char *)Cmd_Argv(1);
            if (!strcmp(server, "localhost"))
                SV_KillLocalServer();
            cl_serverLoadingMap = 0;
            g_waitingForServer = 0;
            SV_Frame(0);
            CL_Disconnect(0);
            Con_Close(0);
            I_strncpyz(cls.servername, server, 256);
            if (NET_StringToAdr(cls.servername, &clc->serverAddress))
            {
                if (!clc->serverAddress.port)
                    clc->serverAddress.port = BigShort(28960);
                v1 = BigShort(clc->serverAddress.port);
                Com_Printf(
                    0,
                    "%s resolved to %i.%i.%i.%i:%i\n",
                    cls.servername,
                    clc->serverAddress.ip[0],
                    clc->serverAddress.ip[1],
                    clc->serverAddress.ip[2],
                    clc->serverAddress.ip[3],
                    v1);
                if (NET_IsLocalAddress(clc->serverAddress))//  || CL_CDKeyValidate(cl_cdkey, cl_cdkeychecksum))
                {
                    if (Com_HasPlayerProfile())
                    {
                        if (NET_IsLocalAddress(clc->serverAddress))
                        {
                            clientUIActives[0].connectionState = CA_CHALLENGING;
                            clc->lastPacketTime = Sys_Milliseconds();
                        }
                        else
                        {
                            clientUIActives[0].connectionState = CA_CONNECTING;
                        }
                        clientUIActives[0].keyCatchers = 0;
                        clientUIActives[0].displayHUDWithKeycatchUI = 0;
                        clc->connectTime = -99999;
                        clc->connectPacketCount = 0;
                        clc->qport = g_qport;
                        Cbuf_ExecuteBuffer(0, 0, "selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
                        UI_CloseAll(0);
                        SCR_UpdateLoadScreen();
                    }
                    else
                    {
                        Com_Error(ERR_DROP, "PLATFORM_NOTSIGNEDINTOPROFILE");
                    }
                }
                else
                {
                    Com_Error(ERR_DROP, "EXE_ERR_INVALID_CD_KEY");
                }
            }
            else
            {
                Com_Printf(0, "Bad server address\n");
                clientUIActives[0].connectionState = CA_DISCONNECTED;
            }
        }
        else
        {
            Com_Printf(0, "Already connected to a server. Disconnect first\n");
        }
    }
    else
    {
        Com_Printf(0, "usage: connect [server]\n");
    }
}

bool __cdecl CL_CDKeyValidate(const char *key, const char *checksum)
{
    char chs[8]; // [esp+4h] [ebp-1Ch] BYREF
    unsigned int crcAcc; // [esp+10h] [ebp-10h]
    int index; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    unsigned int crcAccInit; // [esp+1Ch] [ebp-4h]

    crcAccInit = 0;
    crcAcc = 0;
    for (index = 0; index < 16; ++index)
    {
        crcAcc ^= key[index];
        for (i = 8; i; --i)
        {
            if ((crcAcc & 1) != 0)
                crcAcc = (crcAcc >> 1) ^ 0xA001;
            else
                crcAcc >>= 1;
        }
    }
    sprintf(chs, "%04x", crcAcc);
    return checksum && !I_strnicmp(chs, checksum, 4) || checksum == 0;
}

void __cdecl CL_GlobalServers_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char *buffptr; // [esp+10h] [ebp-42Ch]
    netadr_t to; // [esp+14h] [ebp-428h] BYREF
    int i; // [esp+28h] [ebp-414h]
    char command[1028]; // [esp+2Ch] [ebp-410h] BYREF
    serverInfo_t *server; // [esp+434h] [ebp-8h]
    int count; // [esp+438h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        for (i = 0; i < cls.numglobalservers; ++i)
        {
            server = &cls.globalServers[i];
            if (!++server->requestCount)
                --server->requestCount;
        }
        Com_Printf(0, "Requesting servers from the master...\n");
        NET_StringToAdr((char *)com_masterServerName->current.integer, &to);
        cls.waitglobalserverresponse = 1;
        cls.pingUpdateSource = 1;
        to.type = NA_IP;
        to.port = BigShort(com_masterPort->current.integer);
        v0 = Cmd_Argv(2);
        sprintf(command, "getservers %s", v0);
        buffptr = &command[&command[strlen(command) + 1] - &command[1]];
        count = Cmd_Argc();
        for (i = 3; i < count; ++i)
        {
            v1 = Cmd_Argv(i);
            buffptr += sprintf(buffptr, " %s", v1);
        }
        if (Dvar_GetBool("fs_restrict"))
            sprintf(buffptr, " demo");
        NET_OutOfBandPrint(NS_SERVER, to, command);
    }
    else
    {
        Com_Printf(0, "usage: globalservers <master# 0-1> <protocol> [keywords]\n");
    }
}

void __cdecl CL_ServerStatusResponse(netadr_t from, msg_t *msg)
{
    char *v2; // eax
    char *v3; // eax
    char info[1024]; // [esp+30h] [ebp-420h] BYREF
    int l; // [esp+430h] [ebp-20h]
    int ping; // [esp+434h] [ebp-1Ch] BYREF
    int len; // [esp+438h] [ebp-18h]
    const char *s; // [esp+43Ch] [ebp-14h]
    serverStatus_s *serverStatus; // [esp+440h] [ebp-10h]
    int i; // [esp+444h] [ebp-Ch]
    int score; // [esp+448h] [ebp-8h] BYREF

    serverStatus = 0;
    for (i = 0; i < 16; ++i)
    {
        if (NET_CompareAdr(from, cl_serverStatusList[i].address))
        {
            serverStatus = &cl_serverStatusList[i];
            break;
        }
    }
    if (serverStatus)
    {
        s = MSG_ReadStringLine(msg);
        len = 0;
        Com_sprintf(serverStatus->string, 0x2000u, "%s", s);
        if (serverStatus->print)
        {
            Com_Printf(14, "Server settings:\n");
            while (*s)
            {
                for (i = 0; i < 2 && *s; ++i)
                {
                    if (*s == 92)
                        ++s;
                    l = 0;
                    do
                    {
                        if (!*s)
                            break;
                        info[l++] = *s;
                        if (l >= 1023)
                            break;
                        ++s;
                    } while (*s != 92);
                    info[l] = 0;
                    if (i)
                        Com_Printf(14, "%s\n", info);
                    else
                        Com_Printf(14, "%-24s", info);
                }
            }
        }
        len = strlen(serverStatus->string);
        Com_sprintf(&serverStatus->string[len], 0x2000 - len, "\\");
        if (serverStatus->print)
        {
            Com_Printf(14, "\nPlayers:\n");
            Com_Printf(14, "num: score: ping: name:\n");
        }
        i = 0;
        s = MSG_ReadStringLine(msg);
        while (*s)
        {
            len = strlen(serverStatus->string);
            Com_sprintf(&serverStatus->string[len], 0x2000 - len, "\\%s", s);
            if (serverStatus->print)
            {
                ping = 0;
                score = 0;
                sscanf(s, "%d %d", &score, &ping);
                v2 = strchr((char *)s, 0x20u);
                if (v2)
                {
                    s = strchr((char *)s + 1, 0x20u);
                }
                if (s)
                    ++s;
                else
                    s = "unknown";
                Com_Printf(14, "%-2d   %-3d    %-3d   %s\n", i, score, ping, s);
            }
            s = MSG_ReadStringLine(msg);
            ++i;
        }
        len = strlen(serverStatus->string);
        Com_sprintf(&serverStatus->string[len], 0x2000 - len, "\\");
        serverStatus->time = Sys_Milliseconds();
        serverStatus->address = from;
        serverStatus->pending = 0;
        if (serverStatus->print)
            serverStatus->retrieved = 1;
    }
}

void __cdecl CL_ResetPlayerMuting(unsigned int muteClientIndex)
{
    if (muteClientIndex >= 0x40)
        MyAssertHandler(
            ".\\client_mp\\cl_main_pc_mp.cpp",
            1378,
            0,
            "%s",
            "muteClientIndex >= 0 && muteClientIndex < MAX_CLIENTS");
    s_playerMute[muteClientIndex] = 0;
}

void __cdecl CL_MutePlayer(int localClientNum, unsigned int muteClientIndex)
{
    const char *v2; // eax

    if (muteClientIndex >= 0x40)
        MyAssertHandler(
            ".\\client_mp\\cl_main_pc_mp.cpp",
            1385,
            0,
            "muteClientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            muteClientIndex,
            64);
    s_playerMute[muteClientIndex] = !s_playerMute[muteClientIndex];
    if (s_playerMute[muteClientIndex])
        v2 = va("muteplayer %i", muteClientIndex);
    else
        v2 = va("unmuteplayer %i", muteClientIndex);
    Cbuf_AddText(0, v2);
}

bool __cdecl CL_IsPlayerMuted(int localClientNum, unsigned int muteClientIndex)
{
    if (muteClientIndex >= 0x40)
        MyAssertHandler(
            ".\\client_mp\\cl_main_pc_mp.cpp",
            1396,
            0,
            "%s",
            "muteClientIndex >= 0 && muteClientIndex < MAX_CLIENTS");
    return s_playerMute[muteClientIndex];
}

void __cdecl CL_ClearMutedList()
{
    memset((unsigned __int8 *)s_playerMute, 0, sizeof(s_playerMute));
}

