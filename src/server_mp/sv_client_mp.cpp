#include "server.h"
#include <qcommon/cmd.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <game_mp/g_public_mp.h>
#include <server/sv_game.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_local.h>
#include <universal/com_constantconfigstrings.h>
#include <universal/profile.h>

#ifdef WIN32
#include <win32/win_steam.h>
#include <universal/base64.h>
#else
#error Steam auth for Arch (Server)
#endif

struct ucmd_t // sizeof=0xC
{                                       // ...
    const char *name;
    void(__cdecl *func)(client_t *);
    int allowFromOldServer;
};

ucmd_t ucmds[13] =
{
  { "userinfo", &SV_UpdateUserinfo_f, 0 },
  { "disconnect", &SV_Disconnect_f, 1 },
  { "cp", &SV_VerifyIwds_f, 0 },
  { "vdr", &SV_ResetPureClient_f, 0 },
  { "download", &SV_BeginDownload_f, 0 },
  { "nextdl", &SV_NextDownload_f, 0 },
  { "stopdl", &SV_StopDownload_f, 0 },
  { "donedl", &SV_DoneDownload_f, 0 },
  { "retransdl", &SV_RetransmitDownload_f, 0 },
  { "wwwdl", &SV_WWWDownload_f, 0 },
  { "muteplayer", &SV_MutePlayer_f, 0 },
  { "unmuteplayer", &SV_UnmutePlayer_f, 0 },
  { NULL, NULL, 0 }
}; // idb

unsigned __int16 botport;

BOOL __cdecl SV_ShouldAuthorizeAddress(netadr_t adr)
{
    if (net_lanauthorize->current.enabled)
        return 1;
    if (com_dedicated->current.integer == 2)
        return !Sys_IsLANAddress_IgnoreSubnet(adr);
    return Sys_IsLANAddress(adr) == 0;
}

void __cdecl SV_AuthorizeRequest(netadr_t from, int challenge, const char *cdkeyHash)
{
    const char *v3; // eax
    const char *v4; // eax
    char v5; // [esp+3h] [ebp-419h]
    char *v6; // [esp+8h] [ebp-414h]
    char *integer; // [esp+Ch] [ebp-410h]
    const dvar_s *v8; // [esp+10h] [ebp-40Ch]
    char game[1027]; // [esp+14h] [ebp-408h] BYREF
    bool allowAnonymous; // [esp+417h] [ebp-5h]

    if (!SV_ShouldAuthorizeAddress(from))
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 398, 0, "%s", "SV_ShouldAuthorizeAddress( from )");
    if (svs.authorizeAddress.type != NA_BAD)
    {
        game[0] = 0;
        v8 = Dvar_RegisterString("fs_game", "", 0x1Cu, "File sysytem base game name");
        if (v8 && v8->current.integer)
        {
            integer = (char*)v8->current.integer;
            v6 = game;
            do
            {
                v5 = *integer;
                *v6++ = *integer++;
            } while (v5);
        }
        v3 = NET_AdrToString(from);
        Com_DPrintf(15, "sending getIpAuthorize for %s\n", v3);
        allowAnonymous = Dvar_GetBool("sv_allowAnonymous");
        if (*cdkeyHash)
            v4 = va(
                "getIpAuthorize %i %i.%i.%i.%i \"%s\" %i PB \"%s\"",
                challenge,
                from.ip[0],
                from.ip[1],
                from.ip[2],
                from.ip[3],
                game,
                allowAnonymous,
                cdkeyHash);
        else
            v4 = va(
                "getIpAuthorize %i %i.%i.%i.%i \"%s\" %i",
                challenge,
                from.ip[0],
                from.ip[1],
                from.ip[2],
                from.ip[3],
                game,
                allowAnonymous);
        NET_OutOfBandPrint(NS_SERVER, svs.authorizeAddress, v4);
    }
}

int __cdecl SV_IsTempBannedGuid(const char *cdkeyHash)
{
    unsigned int banSlot; // [esp+Ch] [ebp-4h]

    if (!*cdkeyHash)
        return 0;
    for (banSlot = 0; banSlot < 0x10; ++banSlot)
    {
        if (!memcmp(&svs.tempBans[banSlot], cdkeyHash, 0x20u)
            && sv_kickBanTime->current.value * 1000.0 >= (svs.time - LODWORD(svs.mapCenter[9 * banSlot - 136])))
        {
            return 1;
        }
    }
    return 0;
}

void __cdecl SV_GetChallenge(netadr_t from)
{
    int v1; // esi
    const char *v2; // eax
    __int16 v3; // ax
    const char *v4; // eax
    netadr_t v5; // [esp-14h] [ebp-30h]
    //char *cdkeyHash; // [esp+4h] [ebp-18h]
    char *clientSteamTicketBase64;
    challenge_t *challenge; // [esp+8h] [ebp-14h]
    int oldest; // [esp+Ch] [ebp-10h]
    int i; // [esp+10h] [ebp-Ch]
    int oldestTime; // [esp+14h] [ebp-8h]

    oldest = 0;
    oldestTime = 0x7FFFFFFF;
    challenge = svs.challenges;
    for (i = 0; i < 1024 && (challenge->connected || !NET_CompareAdr(from, challenge->adr)); ++i)
    {
        if (challenge->time < oldestTime)
        {
            oldestTime = challenge->time;
            oldest = i;
        }
        ++challenge;
    }
    if (i == 1024)
    {
        challenge = &svs.challenges[oldest];
        v1 = rand() << 16;
        challenge->challenge = svs.time ^ rand() ^ v1;
        challenge->adr = from;
        challenge->firstTime = svs.time;
        challenge->firstPing = 0;
        challenge->time = svs.time;
        challenge->connected = 0;
        i = oldest;
    }
    //cdkeyHash = (char *)SV_Cmd_Argv(2);
    clientSteamTicketBase64 = (char *)SV_Cmd_Argv(2);
    char *clientSteamID64 = (char *)SV_Cmd_Argv(3);
    unsigned char decodedSteamTicket[1024 + 128]{ 0 };

    if (!clientSteamTicketBase64[0] || !clientSteamID64[0])
    {
        iassert(0); // guy didn't send enough information
        return;
    }

    if (strlen(clientSteamTicketBase64) > sizeof(decodedSteamTicket))
    {
        iassert(0);
        return;
    }

    uint32 decodedLen = b64_decode((unsigned char*)clientSteamTicketBase64, strlen(clientSteamTicketBase64), decodedSteamTicket);




    //if (SV_IsBannedGuid(cdkeyHash))
    if (SV_IsBannedGuid(clientSteamID64))
    {
        Com_Printf(15, "rejected connection from permanently banned GUID \"%s\"\n", clientSteamID64);
        NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\xA\x15You are permanently banned from this server");
        memset((unsigned __int8 *)&svs.challenges[i], 0, sizeof(svs.challenges[i]));
        return;
    }
    //if (SV_IsTempBannedGuid(cdkeyHash))
    if (SV_IsTempBannedGuid(clientSteamID64))
    {
        Com_Printf(15, "rejected connection from temporarily banned GUID \"%s\"\n", clientSteamID64);
        NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\xA\x15You are temporarily banned from this server");
        memset((unsigned __int8 *)&svs.challenges[i], 0, sizeof(svs.challenges[i]));
        return;
    }

    //I_strncpyz(svs.challenges[i].cdkeyHash, cdkeyHash, 33);
    I_strncpyz(svs.challenges[i].cdkeyHash, clientSteamID64, 33);
    
    char *endptr;
    uint64_t steamID64 = strtoull(clientSteamID64, &endptr, 10);

    //if (!SV_ShouldAuthorizeAddress(from))
    if (Steam_CheckClientTicket(decodedSteamTicket, decodedLen, steamID64))
    {
        challenge->pingTime = svs.time;
        v2 = va("challengeResponse %i", challenge->challenge);
        NET_OutOfBandPrint(NS_SERVER, from, v2);
        return;
    }

    //if (!svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD)
    //{
    //    Com_Printf(15, "Resolving %s\n", com_authServerName->current.string);
    //    if (!NET_StringToAdr((char *)com_authServerName->current.integer, &svs.authorizeAddress))
    //    {
    //        Com_Printf(15, "Couldn't resolve address\n");
    //        return;
    //    }
    //    svs.authorizeAddress.port = BigShort(com_authPort->current.integer);
    //    v3 = BigShort(svs.authorizeAddress.port);
    //    Com_Printf(
    //        15,
    //        "%s resolved to %i.%i.%i.%i:%i\n",
    //        com_authServerName->current.string,
    //        svs.authorizeAddress.ip[0],
    //        svs.authorizeAddress.ip[1],
    //        svs.authorizeAddress.ip[2],
    //        svs.authorizeAddress.ip[3],
    //        v3);
    //}

    //if (svs.time - svs.sv_lastTimeMasterServerCommunicated <= 1200000
    //    || svs.time - challenge->firstTime <= 7000
    //    || (v5 = *SV_MasterAddress(), NET_CompareAdr(from, v5)))
    //{
    //    SV_AuthorizeRequest(from, svs.challenges[i].challenge, cdkeyHash);
    //}
    //else
    //{
    //    Com_DPrintf(15, "authorize server timed out\n");
    //    challenge->pingTime = svs.time;
    //    v4 = va("challengeResponse %i", challenge->challenge);
    //    NET_OutOfBandPrint(NS_SERVER, challenge->adr, v4);
    //}
}

int __cdecl SV_IsBannedGuid(const char *cdkeyHash)
{
    char *file; // [esp+18h] [ebp-10h] BYREF
    int banned; // [esp+1Ch] [ebp-Ch]
    const char *token; // [esp+20h] [ebp-8h]
    const char *text; // [esp+24h] [ebp-4h] BYREF

    if (!*cdkeyHash)
        return 0;
    if (FS_ReadFile("ban.txt", (void **)&file) < 0)
        return 0;
    text = file;
    banned = 0;
    while (1)
    {
        token = (const char *)Com_Parse(&text);
        if (!*token)
            break;
        if (!strcmp(token, cdkeyHash))
        {
            banned = 1;
            break;
        }
        Com_SkipRestOfLine(&text);
    }
    FS_FreeFile(file);
    return banned;
}

void __cdecl SV_ReceiveStats(netadr_t from, msg_t *msg)
{
    const char *v2; // eax
    const char *v3; // eax
    int v4; // [esp+0h] [ebp-20h]
    client_t *ClientByAddress; // [esp+8h] [ebp-18h]
    int start; // [esp+Ch] [ebp-14h]
    unsigned int packetNum; // [esp+14h] [ebp-Ch]
    int qport; // [esp+18h] [ebp-8h]

    qport = MSG_ReadShort(msg);
    ClientByAddress = SV_FindClientByAddress(from, qport);
    if (ClientByAddress)
    {
        packetNum = MSG_ReadByte(msg);
        if (packetNum < 8)
        {
            Com_Printf(15, "Received packet %i of stats data\n", packetNum);
            start = 1240 * packetNum;
            if ((int)(0x2000 - 1240 * packetNum) > 1240)
                v4 = 1240;
            else
                v4 = 0x2000 - 1240 * packetNum;
            if (v4 <= 0)
                MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 320, 0, "%s\n\t(size) = %i", "(size > 0)", v4);
            if (v4 + start > 0x2000)
            {
                v2 = va("start is %i, size is %i", start, v4);
                MyAssertHandler(
                    ".\\server_mp\\sv_client_mp.cpp",
                    321,
                    0,
                    "%s\n\t%s",
                    "start + size <= PERSISTENT_STATS_SIZE",
                    v2);
            }
            MSG_ReadData(msg, &ClientByAddress->stats[start], v4);
            ClientByAddress->statPacketsReceived |= 1 << packetNum;
            ClientByAddress->lastPacketTime = svs.time;
            v3 = va("statResponse %i", ~ClientByAddress->statPacketsReceived & 0x7F);
            NET_OutOfBandPrint(NS_SERVER, from, v3);
        }
        else
        {
            Com_PrintWarning(15, "Invalid stat packet %i of stats data\n", packetNum);
        }
    }
    else
    {
        Com_PrintWarning(
            15,
            "Received stats packet from unknown remote client %u.%u.%u.%u\n",
            from.ip[0],
            from.ip[1],
            from.ip[2],
            from.ip[3]);
    }
}

void __cdecl SV_SetClientStat(int clientNum, int index, unsigned int value)
{
    const char *v3; // eax
    client_t *v4; // [esp+0h] [ebp-8h]

    if (svs.clients[clientNum].statPacketsReceived != 127)
        MyAssertHandler(
            ".\\server_mp\\sv_client_mp.cpp",
            335,
            0,
            "%s",
            "svs.clients[clientNum].statPacketsReceived == ( 1 << MAX_STATPACKETS ) - 1");
    if (svs.clients[clientNum].header.state < 2)
        MyAssertHandler(
            ".\\server_mp\\sv_client_mp.cpp",
            336,
            0,
            "%s",
            "svs.clients[clientNum].header.state >= CS_CONNECTED");
    v4 = &svs.clients[clientNum];
    if (index >= 2000)
    {
        if (index < 3498)
        {
            if (*(unsigned int *)&svs.clients[clientNum].voicePackets[17].data[4 * index + 75] == value) // KISAKTODO
                return;
            *(unsigned int *)&svs.clients[clientNum].voicePackets[17].data[4 * index + 75] = value;
            goto LABEL_16;
        }
        if (!alwaysfails)
        {
            v3 = va("Unhandled stat index %i", index);
            MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 359, 0, v3);
        }
    }
    else
    {
        if (value >= 0x100)
            MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 342, 0, "%s", "value >= 0 && value <= 255");
        if (v4->stats[index + 4] != (unsigned __int8)value)
        {
            v4->stats[index + 4] = value;
        LABEL_16:
            SV_SendServerCommand(v4, SV_CMD_RELIABLE, "%c %i %i", 78, index, value);
        }
    }
}

int __cdecl SV_GetClientStat(int clientNum, int index)
{
    const char *v3; // eax

    if (svs.clients[clientNum].statPacketsReceived != 127)
        MyAssertHandler(
            ".\\server_mp\\sv_client_mp.cpp",
            369,
            0,
            "%s",
            "svs.clients[clientNum].statPacketsReceived == ( 1 << MAX_STATPACKETS ) - 1");
    if (svs.clients[clientNum].header.state < 2)
        MyAssertHandler(
            ".\\server_mp\\sv_client_mp.cpp",
            370,
            0,
            "%s",
            "svs.clients[clientNum].header.state >= CS_CONNECTED");
    if (index < 2000)
        return svs.clients[clientNum].stats[index + 4];
    if (index < 3498)
        return *(unsigned int *)&svs.clients[clientNum].voicePackets[17].data[4 * index + 75]; // KISAKTODO
    if (!alwaysfails)
    {
        v3 = va("Unhandled stat index %i", index);
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 383, 0, v3);
    }
    return 0;
}

void __cdecl SV_BanGuidBriefly(const char *cdkeyHash)
{
    unsigned int banSlot; // [esp+8h] [ebp-4h]

    banSlot = SV_FindFreeTempBanSlot();
    memcpy(&svs.tempBans[banSlot], cdkeyHash, 0x20u);
    LODWORD(svs.mapCenter[9 * banSlot - 136]) = svs.time;
}

unsigned int __cdecl SV_FindFreeTempBanSlot()
{
    unsigned int oldestSlot; // [esp+0h] [ebp-8h]
    unsigned int banSlot; // [esp+4h] [ebp-4h]

    oldestSlot = 0;
    for (banSlot = 0; banSlot < 0x10; ++banSlot)
    {
        if (!svs.tempBans[banSlot].cdkeyHash[0])
            return banSlot;
        if (SLODWORD(svs.mapCenter[9 * banSlot - 136]) < SLODWORD(svs.mapCenter[9 * oldestSlot - 136]))
            oldestSlot = banSlot;
    }
    return oldestSlot;
}

void __cdecl SV_BanClient(client_t *cl)
{
    int file; // [esp+0h] [ebp-4Ch] BYREF
    char cleanName[68]; // [esp+4h] [ebp-48h] BYREF

    if (!cl)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 461, 0, "%s", "cl");
    if (cl->header.netchan.remoteAddress.type == NA_LOOPBACK)
    {
        SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c \"EXE_CANNOTKICKHOSTPLAYER\"", 101);
    }
    else if (cl->cdkeyHash[0])
    {
        if (SV_IsBannedGuid(cl->cdkeyHash))
        {
            Com_Printf(15, "This GUID (%s) is already banned\n", cl->cdkeyHash);
        }
        else if ((FS_FOpenFileByMode((char*)"ban.txt", &file, FS_APPEND) & 0x80000000) == 0)
        {
            I_strncpyz(cleanName, cl->name, 64);
            I_CleanStr(cleanName);
            FS_Printf(file, "%s %s\r\n", cl->cdkeyHash, cleanName);
            FS_FCloseFile(file);
            SV_DropClient(cl, "EXE_PLAYERKICKED", 1);
            cl->lastPacketTime = svs.time;
        }
    }
    else
    {
        Com_Printf(15, "Can't ban user, GUID is unknown\n");
    }
}

void __cdecl SV_UnbanClient(char *name)
{
    bool v1; // [esp+0h] [ebp-7Ch]
    int nameLen; // [esp+14h] [ebp-68h]
    char *file; // [esp+18h] [ebp-64h] BYREF
    int fileSize; // [esp+1Ch] [ebp-60h]
    int unban; // [esp+20h] [ebp-5Ch]
    char cleanName[68]; // [esp+24h] [ebp-58h] BYREF
    char *line; // [esp+6Ch] [ebp-10h]
    const char *token; // [esp+70h] [ebp-Ch]
    char *text; // [esp+74h] [ebp-8h] BYREF
    int found; // [esp+78h] [ebp-4h]

    fileSize = FS_ReadFile("ban.txt", (void **)&file);
    if (fileSize >= 0)
    {
        I_strncpyz(cleanName, name, 64);
        I_CleanStr(cleanName);
        nameLen = &cleanName[strlen(cleanName) + 1] - &cleanName[1];
        found = 0;
        text = file;
        while (1)
        {
            line = text;
            token = (const char *)Com_Parse((const char **)&text);
            if (!*token)
                break;
            while (*text && *text <= 32)
                ++text;
            v1 = !I_strnicmp(text, cleanName, nameLen) && (text[nameLen] == 13 || text[nameLen] == 10);
            unban = v1;
            Com_SkipRestOfLine((const char **)&text);
            if (unban)
            {
                ++found;
                memmove((unsigned __int8 *)line, (unsigned __int8 *)text, fileSize - (text - file) + 1);
                fileSize -= text - line;
                text = line;
            }
        }
        FS_WriteFile((char*)"ban.txt", file, fileSize);
        FS_FreeFile(file);
        if (found)
            Com_Printf(15, "unbanned %i user(s) named %s\n", found, cleanName);
        else
            Com_Printf(15, "no banned user has name %s\n", cleanName);
    }
}

void __cdecl SV_CloseDownload(client_t *cl)
{
    int i; // [esp+0h] [ebp-4h]

    if (cl->download)
        FS_FCloseFile(cl->download);
    cl->download = 0;
    cl->downloadName[0] = 0;
    for (i = 0; i < 8; ++i)
    {
        if (cl->downloadBlocks[i])
        {
            Z_Free(cl->downloadBlocks[i], 9);
            cl->downloadBlocks[i] = 0;
        }
    }
}

void __cdecl SV_FreeClient(client_t *cl)
{
    if (cl->header.state < 2)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 686, 0, "%s", "cl->header.state >= CS_CONNECTED");
    SV_CloseDownload(cl);
    if (SV_Loaded())
        ClientDisconnect(cl - svs.clients);
    SV_SetUserinfo(cl - svs.clients, (char *)"");
    SV_FreeClientScriptId(cl);
}

void __cdecl SV_FreeClients()
{
    client_t *clients; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    clients = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (clients->header.state >= 2)
            SV_FreeClient(clients);
        ++i;
        ++clients;
    }
}

void __cdecl SV_DirectConnect(netadr_t from)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    char *v11; // edi
    const char *v12; // eax
    char *v13; // edi
    const char *v14; // eax
    bool v15; // [esp-8h] [ebp-490h]
    bool v16; // [esp-4h] [ebp-48Ch]
    char *fromAddr; // [esp+1Ch] [ebp-46Ch]
    int ping; // [esp+20h] [ebp-468h]
    char cdkeyHash[36]; // [esp+24h] [ebp-464h] BYREF
    client_t *clients; // [esp+48h] [ebp-440h]
    int cl_pb; // [esp+4Ch] [ebp-43Ch]
    char *denied; // [esp+50h] [ebp-438h]
    int version; // [esp+54h] [ebp-434h]
    client_t *newcl; // [esp+58h] [ebp-430h]
    int challenge; // [esp+5Ch] [ebp-42Ch]
    int startIndex; // [esp+60h] [ebp-428h]
    const char *password; // [esp+64h] [ebp-424h]
    char userinfo[1024]; // [esp+68h] [ebp-420h] BYREF
    gentity_s *ent; // [esp+468h] [ebp-20h]
    unsigned int scriptId; // [esp+46Ch] [ebp-1Ch]
    char *pb_authmsg; // [esp+470h] [ebp-18h]
    int i; // [esp+474h] [ebp-14h]
    int clientNum; // [esp+478h] [ebp-10h]
    int count; // [esp+47Ch] [ebp-Ch]
    int qport; // [esp+480h] [ebp-8h]

    Com_DPrintf(15, "SV_DirectConnect()\n");
    v1 = (char *)SV_Cmd_Argv(1);
    I_strncpyz(userinfo, v1, 1024);
    v2 = Info_ValueForKey(userinfo, "protocol");
    version = atoi(v2);
    if (version != 1)
    {
        v3 = va("EXE_SERVER_IS_DIFFERENT_VER %s", "1.0");
        NET_OutOfBandPrint(NS_SERVER, from, v3);
        Com_DPrintf(15, "    rejected connect from protocol version %i (should be %i)\n", version, 1);
        return;
    }
    v4 = Info_ValueForKey(userinfo, "challenge");
    challenge = atoi(v4);
    v5 = Info_ValueForKey(userinfo, "qport");
    qport = atoi(v5);
    i = 0;
    clients = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (NET_CompareBaseAdr(from, clients->header.netchan.remoteAddress)
            && (clients->header.netchan.qport == qport || from.port == clients->header.netchan.remoteAddress.port))
        {
            if (svs.time - clients->lastConnectTime < 1000 * sv_reconnectlimit->current.integer)
            {
                v6 = NET_AdrToString(from);
                Com_DPrintf(15, "%s:reconnect rejected : too soon\n", v6);
                return;
            }
            break;
        }
        ++i;
        ++clients;
    }
    cdkeyHash[0] = 0;
    if (NET_IsLocalAddress(from))
        goto LABEL_31;
    for (i = 0; i < 1024; ++i)
    {
        if (NET_CompareAdr(from, svs.challenges[i].adr) && challenge == svs.challenges[i].challenge)
        {
            memcpy(cdkeyHash, svs.challenges[i].cdkeyHash, 0x21u);
            break;
        }
    }
    if (i == 1024)
    {
        NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_BAD_CHALLENGE");
        return;
    }
    if (svs.challenges[i].firstPing)
    {
        ping = svs.challenges[i].firstPing;
    }
    else
    {
        ping = svs.time - svs.challenges[i].pingTime;
        svs.challenges[i].firstPing = ping;
    }
    v7 = NET_AdrToString(from);
    Com_Printf(15, "Client %i connecting with %i challenge ping from %s\n", i, ping, v7);
    svs.challenges[i].connected = 1;
    if (Sys_IsLANAddress(from))
        goto LABEL_31;
    if (sv_minPing->current.integer && ping < sv_minPing->current.integer)
    {
        NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_ERR_HIGH_PING_ONLY");
        Com_DPrintf(15, "Client %i rejected on a too low ping\n", i);
        return;
    }
    if (!sv_maxPing->current.integer || ping <= sv_maxPing->current.integer)
    {
    LABEL_31:
        // LWSS: Remove punkbuster crap
        //v8 = Info_ValueForKey(userinfo, "cl_punkbuster");
        //cl_pb = atoi(v8);
        //if (NET_IsLocalAddress(from))
        //{
        //    v9 = PbAuthClient("localhost", cl_pb, cdkeyHash);
        //}
        //else
        //{
        //    fromAddr = NET_AdrToString(from);
        //    v9 = PbAuthClient(fromAddr, cl_pb, cdkeyHash);
        //}
        //pb_authmsg = v9;
        //if (v9)
        //{
        //    if (!I_strnicmp(pb_authmsg, "error\n", 6))
        //        NET_OutOfBandPrint(NS_SERVER, from, pb_authmsg);
        //}
        //else
        {
            i = 0;
            clients = svs.clients;
            while (i < sv_maxclients->current.integer)
            {
                if (clients->header.state
                    && NET_CompareBaseAdr(from, clients->header.netchan.remoteAddress)
                    && (clients->header.netchan.qport == qport || from.port == clients->header.netchan.remoteAddress.port))
                {
                    v16 = from.port == clients->header.netchan.remoteAddress.port;
                    v15 = clients->header.netchan.qport == qport;
                    v10 = NET_AdrToString(from);
                    Com_Printf(15, "%s:reconnect. same qport: %i, same port: %i\n", v10, v15, v16);
                    if (clients->header.state >= 2)
                    {
                        if (!cdkeyHash[0] && clients->cdkeyHash[0] && !alwaysfails)
                            MyAssertHandler(
                                ".\\server_mp\\sv_client_mp.cpp",
                                954,
                                0,
                                "Going from a known GUID to an unknown GUID due to reconnect\n");
                        SV_FreeClient(clients);
                    }
                    newcl = clients;
                    goto gotnewcl;
                }
                ++i;
                ++clients;
            }
            password = Info_ValueForKey(userinfo, "password");
            if (!strcmp(password, sv_privatePassword->current.string))
                startIndex = 0;
            else
                startIndex = sv_privateClients->current.integer;
            newcl = 0;
            for (i = startIndex; i < sv_maxclients->current.integer; ++i)
            {
                clients = &svs.clients[i];
                if (!clients->header.state)
                {
                    newcl = clients;
                    break;
                }
            }
            if (!newcl)
            {
                NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_SERVERISFULL");
                Com_DPrintf(15, "Rejected a connection.\n");
                return;
            }
            clients->reliableAcknowledge = 0;
            clients->reliableSequence = 0;
        gotnewcl:
            memset((unsigned __int8 *)newcl, 0, sizeof(client_t));
            clientNum = newcl - svs.clients;
            ent = SV_GentityNum(clientNum);
            newcl->gentity = ent;
            if (newcl->scriptId)
                MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1074, 0, "%s", "!newcl->scriptId");
            scriptId = Scr_AllocArray();
            if (scriptId != (unsigned __int16)scriptId)
                MyAssertHandler(
                    ".\\server_mp\\sv_client_mp.cpp",
                    1077,
                    0,
                    "%s",
                    "scriptId == static_cast<unsigned short>( scriptId )");
            newcl->scriptId = scriptId;
            Com_Printf(15, "SV_DirectConnect: %d, 0 -> %d\n", newcl - svs.clients, newcl->scriptId);
            newcl->challenge = challenge;
            if (!cdkeyHash[0])
                Com_Printf(15, "Connecting player #%i has an unknown GUID\n", clientNum);
            v11 = newcl->cdkeyHash;
            memcpy(newcl->cdkeyHash, cdkeyHash, 0x20u);
            v11[32] = cdkeyHash[32];
            Netchan_Setup(
                NS_SERVER,
                &newcl->header.netchan,
                from,
                qport,
                newcl->netchanOutgoingBuffer,
                0x20000,
                newcl->netchanIncomingBuffer,
                2048);
            newcl->voicePacketCount = 0;
            newcl->sendVoice = 1;
            I_strncpyz(newcl->userinfo, userinfo, 1024);
            denied = ClientConnect(clientNum, newcl->scriptId);
            if (denied)
            {
                v12 = va("error\n%s", denied);
                NET_OutOfBandPrint(NS_SERVER, from, v12);
                Com_DPrintf(15, "Game rejected a connection: %s.\n", denied);
                SV_FreeClientScriptId(newcl);
            }
            else
            {
                Com_Printf(
                    15,
                    "Going from CS_FREE to CS_CONNECTED for %s (num %i guid \"%s\")\n",
                    newcl->name,
                    clientNum,
                    newcl->cdkeyHash);
                newcl->header.state = 2;
                newcl->nextSnapshotTime = svs.time;
                newcl->lastPacketTime = svs.time;
                newcl->lastConnectTime = svs.time;
                v13 = newcl->cdkeyHash;
                memcpy(newcl->cdkeyHash, cdkeyHash, 0x20u);
                v13[32] = cdkeyHash[32];
                SV_UserinfoChanged(newcl);
                svs.challenges[i].firstPing = 0;
                v14 = va("connectResponse %s", fs_gameDirVar->current.string);
                NET_OutOfBandPrint(NS_SERVER, from, v14);
                newcl->gamestateMessageNum = -1;
                count = 0;
                i = 0;
                clients = svs.clients;
                while (i < sv_maxclients->current.integer)
                {
                    if (svs.clients[i].header.state >= 2)
                        ++count;
                    ++i;
                    ++clients;
                }
                if (count == 1 || count == sv_maxclients->current.integer)
                    SV_Heartbeat_f();
            }
        }
    }
    else
    {
        NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_ERR_LOW_PING_ONLY");
        Com_DPrintf(15, "Client %i rejected on a too high ping: %i\n", i, ping);
    }
}

void __cdecl SV_FreeClientScriptPers()
{
    client_t *clients; // [esp+0h] [ebp-Ch]
    unsigned int scriptId; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    i = 0;
    clients = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (clients->header.state >= 2)
        {
            SV_FreeClientScriptId(clients);
            scriptId = Scr_AllocArray();
            if (scriptId != (unsigned __int16)scriptId)
                MyAssertHandler(
                    ".\\server_mp\\sv_client_mp.cpp",
                    1201,
                    0,
                    "%s",
                    "scriptId == static_cast<unsigned short>( scriptId )");
            clients->scriptId = scriptId;
            Com_Printf(15, "SV_FreeClientScriptPers: %d, 0 -> %d\n", clients - svs.clients, clients->scriptId);
        }
        ++i;
        ++clients;
    }
}

void __cdecl SV_SendDisconnect(
    client_t *client,
    int state,
    const char *reason,
    bool translationForReason,
    const char *clientName)
{
    const char *v5; // eax

    if (!client)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1221, 0, "%s", "client");
    if (state < 2)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1222, 0, "%s\n\t(state) = %i", "(state >= CS_CONNECTED)", state);
    if (!reason)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1223, 0, "%s", "reason");
    if (state == 4)
    {
        if (translationForReason)
            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c \"%s\"", 119, reason);
        else
            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c \"%s^7 %s\" PB", 119, clientName, reason);
    }
    else
    {
        if (state != 2 && state != 3)
            MyAssertHandler(
                ".\\server_mp\\sv_client_mp.cpp",
                1242,
                0,
                "%s\n\t(state) = %i",
                "(state == CS_CONNECTED || state == CS_CLIENTLOADING)",
                state);
        v5 = va("disconnect %s", reason);
        NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, v5);
    }
}

void __cdecl SV_DropClient(client_t *drop, const char *reason, bool tellThem)
{
    char v3; // [esp+Bh] [ebp-31h]
    char *v4; // [esp+10h] [ebp-2Ch]
    char *name; // [esp+14h] [ebp-28h]
    int dropState; // [esp+18h] [ebp-24h]
    char droppedClientName[16]; // [esp+1Ch] [ebp-20h] BYREF
    bool translationForReason; // [esp+33h] [ebp-9h]
    challenge_t *challenge; // [esp+34h] [ebp-8h]
    int i; // [esp+38h] [ebp-4h]

    // LWSS ADD
    if (com_dedicated->current.integer)
    {
        char *endptr;
        uint64_t steamID64 = strtoull(drop->cdkeyHash, &endptr, 10);
        Steam_OnClientDropped(steamID64);
    }
    // LWSS END

    if (!drop->header.state)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1267, 0, "%s", "drop->header.state != CS_FREE");
    dropState = drop->header.state;
    if (drop->header.state == 1)
    {
        if (drop->dropReason)
            MyAssertHandler(
                ".\\server_mp\\sv_client_mp.cpp",
                1271,
                0,
                "%s\n\t(drop->dropReason) = %s",
                "(drop->dropReason == 0)",
                drop->dropReason);
    }
    else
    {
        drop->dropReason = 0;
        if (dropState < 2)
            MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1276, 0, "%s", "dropState >= CS_CONNECTED");
        name = drop->name;
        v4 = droppedClientName;
        do
        {
            v3 = *name;
            *v4++ = *name++;
        } while (v3);
        Com_DPrintf(15, "Going to CS_ZOMBIE from %i for %s\n", dropState, droppedClientName);
        SV_FreeClient(drop);
        drop->header.state = 1;
        if (!drop->gentity)
        {
            challenge = svs.challenges;
            i = 0;
            while (i < 1024)
            {
                if (NET_CompareAdr(drop->header.netchan.remoteAddress, challenge->adr))
                {
                    challenge->connected = 0;
                    break;
                }
                ++i;
                ++challenge;
            }
        }
        translationForReason = SEH_StringEd_GetString(reason) != 0;
        if (I_stricmp(reason, "EXE_DISCONNECTED"))
        {
            if (translationForReason)
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c %s^7 %s%s", 101, droppedClientName, "", reason);
            else
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c %s^7 %s%s", 101, droppedClientName, "", reason);
        }
        else if (translationForReason)
        {
            SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c %s^7 %s%s", 101, droppedClientName, "", "EXE_LEFTGAME");
        }
        else
        {
            SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c %s^7 %s%s", 101, droppedClientName, "", "EXE_LEFTGAME");
        }
        Com_Printf(15, "%i:%s %s\n", drop - svs.clients, droppedClientName, reason);
        SV_SendServerCommand(0, SV_CMD_RELIABLE, "%c %d", 75, drop - svs.clients);
        if (tellThem)
            SV_SendDisconnect(drop, dropState, reason, translationForReason, droppedClientName);
        for (i = 0; i < sv_maxclients->current.integer && svs.clients[i].header.state < 2; ++i)
            ;
        if (i == sv_maxclients->current.integer)
            SV_Heartbeat_f();
    }
}

void __cdecl SV_DelayDropClient(client_t *drop, const char *reason)
{
    if (!drop)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1356, 0, "%s", "drop");
    if (!reason)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1357, 0, "%s", "reason");
    if (!drop->header.state)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1359, 0, "%s", "drop->header.state != CS_FREE");
    if (drop->header.state == 1)
    {
        if (drop->dropReason)
            MyAssertHandler(
                ".\\server_mp\\sv_client_mp.cpp",
                1362,
                0,
                "%s\n\t(drop->dropReason) = %s",
                "(drop->dropReason == 0)",
                drop->dropReason);
    }
    else if (!drop->dropReason)
    {
        drop->dropReason = reason;
    }
}

unsigned __int8 msgBuffer_0[131072];
void __cdecl SV_SendClientGameState(client_t *client)
{
    int configStringCount; // [esp+38h] [ebp-16Ch]
    entityState_s nullstate; // [esp+3Ch] [ebp-168h] BYREF
    int numWritten; // [esp+138h] [ebp-6Ch]
    int start; // [esp+13Ch] [ebp-68h]
    SnapshotInfo_s snapInfo; // [esp+140h] [ebp-64h] BYREF
    int size; // [esp+158h] [ebp-4Ch]
    msg_t msg; // [esp+15Ch] [ebp-48h] BYREF
    int lastStringIndex; // [esp+184h] [ebp-20h]
    int largestString; // [esp+188h] [ebp-1Ch]
    int nextConstConfigString; // [esp+18Ch] [ebp-18h]
    const char *configString; // [esp+190h] [ebp-14h]
    int dataStart; // [esp+194h] [ebp-10h]
    entityState_s *base; // [esp+198h] [ebp-Ch]
    int clientNum; // [esp+19Ch] [ebp-8h]
    int totalStringSize; // [esp+1A0h] [ebp-4h]

    while (client->header.state && client->header.netchan.unsentFragments)
        SV_Netchan_TransmitNextFragment(client, &client->header.netchan);
    if (client->bIsTestClient)
    {
        memset(client->stats, 0, sizeof(client->stats));
        client->statPacketsReceived = 127;
    }
    if (client->statPacketsReceived != 127)
    {
        if (!client->statPacketsReceived)
            NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "requeststats\n");
        return;
    }
    SV_SetServerStaticHeader();
    Com_DPrintf(15, "SV_SendClientGameState() for %s\n", client->name);
    Com_DPrintf(15, "Going from CS_CONNECTED to CS_CLIENTLOADING for %s\n", client->name);
    client->header.state = 3;
    client->pureAuthentic = 0;
    client->gamestateMessageNum = client->header.netchan.outgoingSequence;
    MSG_Init(&msg, msgBuffer_0, 0x20000);
    SV_PacketDataIsHeader(client - svs.clients, &msg);
    MSG_ClearLastReferencedEntity(&msg);
    MSG_WriteLong(&msg, client->lastClientCommand);
    dataStart = msg.cursize;
    SV_UpdateServerCommandsToClient(client, &msg);
    Com_Printf(15, "Gamestate has %i bytes of server commands\n", msg.cursize - dataStart);
    SV_PacketDataIsHeader(client - svs.clients, &msg);
    MSG_WriteByte(&msg, 1u);
    MSG_WriteLong(&msg, client->reliableSequence);
    numWritten = 0;
    largestString = 0;
    totalStringSize = 0;
    dataStart = msg.cursize;
    configStringCount = 0;
    MSG_WriteByte(&msg, 2u);
    nextConstConfigString = 0;
    for (start = 0; start < 2442; ++start)
    {
        if (constantConfigStrings[nextConstConfigString].configStringNum == start)
        {
            configString = SL_ConvertToString(sv.configstrings[start]);
            if (constantConfigStrings[nextConstConfigString].configStringNum >= 821
                && I_stricmp(constantConfigStrings[nextConstConfigString].configString, configString))
            {
                ++configStringCount;
            }
            else if (constantConfigStrings[nextConstConfigString].configStringNum < 821
                && strcmp(constantConfigStrings[nextConstConfigString].configString, configString))
            {
                ++configStringCount;
            }
            ++nextConstConfigString;
        }
        else if (sv.configstrings[start] != sv.emptyConfigString)
        {
            ++configStringCount;
        }
    }
    MSG_WriteShort(&msg, configStringCount);
    nextConstConfigString = 0;
    lastStringIndex = -1;
    for (start = 0; start < 2442; ++start)
    {
        if (constantConfigStrings[nextConstConfigString].configStringNum == start)
        {
            ++nextConstConfigString;
            configString = SL_ConvertToString(sv.configstrings[start]);
            if (start >= 821 && !I_stricmp(constantConfigStrings[nextConstConfigString - 1].configString, configString)
                || start < 821 && !strcmp(constantConfigStrings[nextConstConfigString - 1].configString, configString))
            {
                continue;
            }
            if (sv.configstrings[start] == sv.emptyConfigString)
            {
                if (configStringCount < 0)
                    MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1556, 0, "%s", "configStringCount >= 0");
                MSG_WriteBit0(&msg);
                MSG_WriteBits(&msg, start, 0xCu);
                lastStringIndex = start;
                MSG_WriteBigString(&msg, (char *)"");
                ++numWritten;
                --configStringCount;
            }
        }
        if (sv.configstrings[start] != sv.emptyConfigString)
        {
            configString = SL_ConvertToString(sv.configstrings[start]);
            if (configStringCount < 0)
                MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1573, 0, "%s", "configStringCount >= 0");
            if (start == lastStringIndex + 1)
            {
                MSG_WriteBit1(&msg);
            }
            else
            {
                MSG_WriteBit0(&msg);
                MSG_WriteBits(&msg, start, 0xCu);
            }
            lastStringIndex = start;
            MSG_WriteBigString(&msg, (char *)configString);
            --configStringCount;
            size = strlen(configString);
            if (size > largestString)
                largestString = size;
            totalStringSize += size;
            ++numWritten;
        }
    }
    if (configStringCount)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1599, 0, "%s", "configStringCount == 0");
    Com_Printf(
        15,
        "Gamestate has %i bytes of config strings (%i total config strings)\n",
        msg.cursize - dataStart,
        numWritten);
    Com_Printf(15, "   Largest config string was %i bytes\n", largestString);
    Com_Printf(15, "   Average config string was %i bytes\n", totalStringSize / numWritten);
    dataStart = msg.cursize;
    numWritten = 0;
    clientNum = client - svs.clients;
    if ((unsigned int)clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_client_mp.cpp",
            1610,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    memset((unsigned __int8 *)&nullstate, 0, sizeof(nullstate));
    for (start = 0; start < 1024; ++start)
    {
        base = &sv.svEntities[start].baseline.s;
        if (base->number)
        {
            SV_PacketDataIsHeader(client - svs.clients, &msg);
            MSG_WriteByte(&msg, 3u);
            snapInfo.clientNum = client - svs.clients;
            snapInfo.snapshotDeltaTime = -1;
            snapInfo.fromBaseline = 1;
            snapInfo.packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_BASELINE;
            MSG_WriteEntity(&snapInfo, &msg, 0, &nullstate, base, 1);
            ++numWritten;
            snapInfo.fromBaseline = 0;
        }
    }
    Com_Printf(15, "Gamestate has %i bytes of svc_baselines\n", numWritten);
    Com_Printf(15, "Gamestate has %i bytes of gentity numbers\n", 10 * numWritten / 8);
    Com_Printf(
        15,
        "Gamestate has %i bytes of entity deltas\n",
        msg.cursize - dataStart - numWritten - 10 * numWritten / 8);
    SV_PacketDataIsHeader(client - svs.clients, &msg);
    MSG_WriteByte(&msg, 7u);
    MSG_WriteLong(&msg, client - svs.clients);
    MSG_WriteLong(&msg, sv.checksumFeed);
    MSG_WriteByte(&msg, 7u);
    Com_DPrintf(15, "Sending %i bytes in gamestate to client: %i\n", msg.cursize, client - svs.clients);
    SV_SendMessageToClient(&msg, client);
    SV_GetServerStaticHeader();
}

void __cdecl SV_ClientEnterWorld(client_t *client, usercmd_s *cmd)
{
    gentity_s *v2; // eax

    Com_DPrintf(15, "Going from CS_CLIENTLOADING to CS_ACTIVE for %s\n", client->name);
    client->header.state = 4;
    v2 = SV_GentityNum(client - svs.clients);
    v2->s.number = client - svs.clients;
    client->gentity = v2;
    client->header.deltaMessage = -1;
    client->nextSnapshotTime = svs.time;
    memcpy(&client->lastUsercmd, cmd, sizeof(client->lastUsercmd));
    ClientBegin(client - svs.clients);
}

void __cdecl SV_Disconnect_f(client_t *cl)
{
    SV_DropClient(cl, "EXE_DISCONNECTED", 1);
}

void __cdecl SV_UserinfoChanged(client_t *cl)
{
    const char *v1; // eax
    const char *val; // [esp+30h] [ebp-8h]
    const char *vala; // [esp+30h] [ebp-8h]
    const char *valc; // [esp+30h] [ebp-8h]
    const char *valb; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    v1 = Info_ValueForKey(cl->userinfo, "name");
    I_strncpyz(cl->name, v1, 16);
    if (!Sys_IsLANAddress(cl->header.netchan.remoteAddress) || com_dedicated->current.integer == 2)
    {
        val = Info_ValueForKey(cl->userinfo, "rate");
        if (strlen(val))
        {
            cl->rate = atoi(val);
            if (cl->rate >= 1000)
            {
                if (cl->rate > 90000)
                    cl->rate = 90000;
            }
            else
            {
                cl->rate = 1000;
            }
        }
        else
        {
            cl->rate = 5000;
        }
    }
    else
    {
        cl->rate = 99999;
    }
    vala = Info_ValueForKey(cl->userinfo, "snaps");
    if (strlen(vala))
    {
        i = atoi(vala);
        if (i >= 1)
        {
            if (i > 30)
                i = 30;
        }
        else
        {
            i = 1;
        }
        cl->snapshotMsec = 1000 / i;
    }
    else
    {
        cl->snapshotMsec = 50;
    }
    valc = Info_ValueForKey(cl->userinfo, "cl_voice");
    cl->sendVoice = atoi(valc) > 0;
    if (cl->rate < 5000)
        cl->sendVoice = 0;
    valb = Info_ValueForKey(cl->userinfo, "cl_wwwDownload");
    cl->wwwOk = 0;
    if (strlen(valb))
    {
        if (atoi(valb))
            cl->wwwOk = 1;
    }
}

void __cdecl SV_UpdateUserinfo_f(client_t *cl)
{
    char *v1; // eax

    v1 = (char *)SV_Cmd_Argv(1);
    I_strncpyz(cl->userinfo, v1, 1024);
    SV_UserinfoChanged(cl);
    ClientUserinfoChanged(cl - svs.clients);
}

void __cdecl SV_ClientThink(client_t *cl, usercmd_s *cmd)
{
    char *v2; // eax

    if (cmd->serverTime - svs.time <= 20000)
    {
        memcpy(&cl->lastUsercmd, cmd, sizeof(cl->lastUsercmd));
        if (cl->header.state == 4)
        {
            if ((unsigned int)(cl - svs.clients) >= 0x40)
                MyAssertHandler(
                    ".\\server_mp\\sv_client_mp.cpp",
                    2934,
                    0,
                    "%s\n\t(cl - svs.clients) = %i",
                    "(cl - svs.clients >= 0 && cl - svs.clients < 64)",
                    cl - svs.clients);
            G_SetLastServerTime(cl - svs.clients, cmd->serverTime);
            ClientThink(cl - svs.clients);
        }
    }
    else
    {
        v2 = va("Invalid command time %i from client %s, current server time is %i", cmd->serverTime, cl->name, svs.time);
        Com_PrintError(15, v2);
    }
}

void __cdecl SV_UserMove(client_t *cl, msg_t *msg, int delta)
{
    const char *v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // esi
    int v7; // eax
    int v8; // [esp-4h] [ebp-46Ch]
    char *v9; // [esp+18h] [ebp-450h]
    svscmd_info_t *v10; // [esp+1Ch] [ebp-44Ch]
    char *pszString; // [esp+20h] [ebp-448h]
    usercmd_s nullcmd; // [esp+24h] [ebp-444h] BYREF
    usercmd_s *oldcmd; // [esp+44h] [ebp-424h]
    int cmdCount; // [esp+48h] [ebp-420h]
    int key; // [esp+4Ch] [ebp-41Ch]
    usercmd_s cmds[32]; // [esp+50h] [ebp-418h] BYREF
    int i; // [esp+458h] [ebp-10h]
    playerState_s *ps; // [esp+45Ch] [ebp-Ch]
    int value; // [esp+460h] [ebp-8h]
    usercmd_s *cmd; // [esp+464h] [ebp-4h]

    if (delta)
        cl->header.deltaMessage = cl->messageAcknowledge;
    else
        cl->header.deltaMessage = -1;
    if (cl->reliableSequence - cl->reliableAcknowledge < 128)
    {
        cmdCount = MSG_ReadByte(msg);
        if (cmdCount >= 1)
        {
            if (cmdCount <= 32)
            {
                key = sv.checksumFeed;
                key = cl->messageAcknowledge ^ sv.checksumFeed;
                key ^= Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd, 32);
                if (sv_debugMessageKey->current.enabled)
                {
                    v9 = &cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd[1];
                    v10 = &cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F];
                    v8 = cl->reliableAcknowledge & 0x7F;
                    v4 = Com_HashKey(v10->cmd, 32);
                    Com_Printf(
                        15,
                        "key:%i, checksumFeed:%i, messageAcknowledge:%i, Com_HashKey:%i, servercommand(%i):'%s', len:%i\n",
                        key,
                        sv.checksumFeed,
                        cl->messageAcknowledge,
                        v4,
                        v8,
                        v10->cmd,
                        &v10->cmd[strlen(v10->cmd) + 1] - v9);
                }
                ps = SV_GameClientNum(cl - svs.clients);
                if (!ps)
                    MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 3012, 0, "%s", "ps");
                if (!BG_ValidateWeaponNumber(ps->weapon))
                    MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 3013, 0, "%s", "BG_ValidateWeaponNumber( ps->weapon )");
                MSG_SetDefaultUserCmd(ps, &nullcmd);
                cmd = &nullcmd;
                oldcmd = &nullcmd;
                for (i = 0; i < cmdCount; ++i)
                {
                    cmd = &cmds[i];
                    MSG_ReadDeltaUsercmdKey(msg, key, oldcmd, cmd);
                    if (ps->weapon != LOBYTE(ps->weapon))
                        MyAssertHandler(
                            ".\\server_mp\\sv_client_mp.cpp",
                            3024,
                            0,
                            "%s",
                            "ps->weapon == static_cast<byte>( ps->weapon )");
                    if (!BG_IsWeaponValid(ps, cmd->weapon))
                        cmd->weapon = ps->weapon;
                    if (ps->offHandIndex != LOBYTE(ps->offHandIndex))
                        MyAssertHandler(
                            ".\\server_mp\\sv_client_mp.cpp",
                            3032,
                            0,
                            "%s",
                            "ps->offHandIndex == static_cast<byte>( ps->offHandIndex )");
                    if (!BG_IsWeaponValid(ps, cmd->offHandIndex))
                        cmd->offHandIndex = ps->offHandIndex;
                    if (!BG_ValidateWeaponNumber(cmd->weapon) || !BG_ValidateWeaponNumber(cmd->offHandIndex))
                    {
                        Com_Printf(15, "###!#!#!#!#!#!#!#!#!#!#!#!#!#!#!#!###\n");
                        Com_Printf(
                            15,
                            "Encountered corrupt user command. This means the client's write key and the server read key for the net me"
                            "ssage were different\n");
                        Com_Printf(15, "Problem occured with client #%i:'%s'\n", cl - svs.clients, cl->name);
                        Com_Printf(15, "Server game time: %i\n", svs.time);
                        Com_Printf(15, "---- Corrupt user command data:\n");
                        Com_Printf(15, "Command was %i of %i in the packet.\n", i, cmdCount);
                        Com_Printf(15, "serverTime = %i\n", cmd->serverTime);
                        Com_Printf(15, "angles[0] = %i(%f)\n", cmd->angles[0], (double)cmd->angles[0] * 0.0054931640625);
                        Com_Printf(15, "angles[1] = %i(%f)\n", cmd->angles[1], (double)cmd->angles[1] * 0.0054931640625);
                        Com_Printf(15, "angles[2] = %i(%f)\n", cmd->angles[2], (double)cmd->angles[2] * 0.0054931640625);
                        Com_Printf(15, "forwardmove = %i\n", cmd->forwardmove);
                        Com_Printf(15, "rightmove = %i\n", cmd->rightmove);
                        Com_Printf(15, "buttons = %i\n", cmd->buttons);
                        Com_Printf(15, "weapon = %i\n", cmd->weapon);
                        Com_Printf(15, "---- %i Client Info\n", cl - svs.clients);
                        switch (cl->header.state)
                        {
                        case 0:
                            Com_Printf(15, "state: %s\n", "free");
                            break;
                        case 1:
                            Com_Printf(15, "state: %s\n", "zombie");
                            break;
                        case 2:
                            Com_Printf(15, "state: %s\n", "connected");
                            break;
                        case 3:
                            Com_Printf(15, "state: %s\n", "clientloading");
                            break;
                        case 4:
                            Com_Printf(15, "state: %s\n", "active");
                            break;
                        default:
                            pszString = va("unknown(%i)", cl->header.state);
                            Com_Printf(15, "state: %s\n", pszString);
                            break;
                        }
                        Com_Printf(15, "userinfo: '%s'\n", cl->userinfo);
                        Com_Printf(15, "reliableSequence: %i\n", cl->reliableSequence);
                        Com_Printf(15, "reliableAcknowledge: %i\n", cl->reliableAcknowledge);
                        Com_Printf(15, "reliableSent: %i\n", cl->reliableSent);
                        Com_Printf(15, "messageAcknowledge: %i\n", cl->messageAcknowledge);
                        Com_Printf(15, "gamestateMessageNum: %i\n", cl->gamestateMessageNum);
                        Com_Printf(15, "challenge: %i\n", cl->challenge);
                        Com_Printf(15, "lastClientCommand: %i\n", cl->lastClientCommand);
                        Com_Printf(15, "deltaMessage: %i\n", cl->header.deltaMessage);
                        Com_Printf(15, "nextReliableTime: %i\n", cl->nextReliableTime);
                        Com_Printf(15, "lastPacketTime: %i\n", cl->lastPacketTime);
                        Com_Printf(15, "lastConnectTime: %i\n", cl->lastConnectTime);
                        Com_Printf(15, "nextSnapshotTime: %i\n", cl->nextSnapshotTime);
                        Com_Printf(15, "rateDelayed: %i\n", cl->header.rateDelayed);
                        Com_Printf(15, "timeoutCount: %i\n", cl->timeoutCount);
                        Com_Printf(15, "ping: %i\n", cl->ping);
                        Com_Printf(15, "rate: %i\n", cl->rate);
                        Com_Printf(15, "snapshotMsec: %i\n", cl->snapshotMsec);
                        Com_Printf(15, "pureAuthentic: %i\n", cl->pureAuthentic);
                        Com_Printf(15, "---- Misc Messaging Info\n");
                        Com_Printf(15, "sv.checksumFeed: %i\n", sv.checksumFeed);
                        Com_Printf(15, "cl->messageAcknowledge: %i\n", cl->messageAcknowledge);
                        Com_Printf(15, "cl->reliableAcknowledge: %i\n", cl->reliableAcknowledge);
                        Com_Printf(15, "cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1): %i\n", cl->reliableAcknowledge & 0x7F);
                        Com_Printf(
                            15,
                            "cl->reliableCommandInfo[cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1)].cmd: '%s'\n",
                            cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd);
                        v5 = Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd, 32);
                        Com_Printf(
                            15,
                            "Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1)].cmd,32): %i\n",
                            v5);
                        Com_Printf(15, "key = sv.checksumFeed: %i\n", sv.checksumFeed);
                        Com_Printf(15, "key ^= cl->messageAcknowledge: %i\n", cl->messageAcknowledge ^ sv.checksumFeed);
                        v6 = cl->messageAcknowledge ^ sv.checksumFeed;
                        v7 = Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd, 32);
                        Com_Printf(
                            15,
                            "key ^= Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1)].cmd,32): %i\n",
                            v7 ^ v6);
                        Com_Printf(15, "key: %i\n", key);
                        Com_Printf(15, "key ^= cmd->serverTime: %i\n", cmd->serverTime ^ key);
                        Com_Printf(15, "########################################\n");
                        SV_DropClient(cl, "Corrupted network messaging detected", 1);
                    }
                    oldcmd = cmd;
                }
                if (cmdCount > 0)
                {
                    *(float *)&value = COERCE_FLOAT(MSG_ReadLong(msg));
                    cl->header.predictedOrigin[0] = *(float *)&value;
                    *(float *)&value = COERCE_FLOAT(MSG_ReadLong(msg));
                    cl->header.predictedOrigin[1] = *(float *)&value;
                    *(float *)&value = COERCE_FLOAT(MSG_ReadLong(msg));
                    cl->header.predictedOrigin[2] = *(float *)&value;
                    cl->header.predictedOriginServerTime = MSG_ReadLong(msg);
                }
                if (cl->frames[cl->messageAcknowledge & 0x1F].messageAcked <= 0)
                    cl->frames[cl->messageAcknowledge & 0x1F].messageAcked = Sys_Milliseconds();
                if (cl->header.state == 3)
                    SV_ClientEnterWorld(cl, cmds);
                if (!sv_pure->current.enabled || cl->pureAuthentic)
                {
                    if (cl->header.state == 4)
                    {
                        for (i = 0; i < cmdCount; ++i)
                        {
                            if (cmds[i].serverTime <= cmds[cmdCount - 1].serverTime
                                && cmds[i].serverTime > cl->lastUsercmd.serverTime)
                            {
                                SV_ClientThink(cl, &cmds[i]);
                            }
                        }
                    }
                    else
                    {
                        cl->header.deltaMessage = -1;
                    }
                }
                else
                {
                    SV_DropClient(cl, "EXE_CANNOTVALIDATEPURECLIENT", 1);
                }
            }
            else
            {
                if (!alwaysfails)
                    MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 2991, 0, "cmdCount > MAX_PACKET_USERCMDS");
                Com_Printf(15, "cmdCount > MAX_PACKET_USERCMDS\n");
            }
        }
        else
        {
            if (!alwaysfails)
                MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 2984, 0, "cmdCount < 1");
            Com_Printf(15, "cmdCount < 1\n");
        }
    }
    else if (!alwaysfails)
    {
        v3 = va(
            "cl->reliableSequence is %i, cl->reliableAcknowledge is %i for %s",
            cl->reliableSequence,
            cl->reliableAcknowledge,
            cl->name);
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 2976, 0, v3);
    }
}

int __cdecl SV_ProcessClientCommands(client_t *cl, msg_t *msg, int fromOldServer, int *lastCommand)
{
    do
    {
        *lastCommand = MSG_ReadBits(msg, 3u);
        if (*lastCommand == 3)
            return 1;
        if (*lastCommand != 2)
            return 1;
        if (!SV_ClientCommand(cl, msg, fromOldServer))
            return 0;
    } while (cl->header.state != 1);
    return 0;
}

unsigned __int8 msgCompressed_buf_0[2048];
void __cdecl SV_ExecuteClientMessage(client_t *cl, msg_t *msg)
{
    msg_t v2; // [esp+60h] [ebp-54h] BYREF
    int c; // [esp+88h] [ebp-2Ch] BYREF
    msg_t msgCompressed; // [esp+8Ch] [ebp-28h] BYREF

    MSG_Init(&msgCompressed, msgCompressed_buf_0, 2048);
    msgCompressed.cursize = MSG_ReadBitsCompress(
        &msg->data[msg->readcount],
        msgCompressed_buf_0,
        msg->cursize - msg->readcount);
    if (cl->serverId == sv_serverId_value || cl->downloading || cl->downloadingWWW || cl->clientDownloadingWWW)
    {
        if (SV_ProcessClientCommands(cl, &msgCompressed, 0, &c))
        {
            if (sv_pure->current.enabled && cl->pureAuthentic == 2 && cl->header.state >= 4)
            {
                cl->nextSnapshotTime = -1;
                SV_DropClient(cl, "EXE_UNPURECLIENTDETECTED", 1);
                cl->header.state = 4;
                if (cl->header.state == 4 || cl->header.state == 1)
                {
                    PROF_SCOPED("SV_BuildClientSnapshot");
                    SV_BuildClientSnapshot(cl);
                }
                SV_SetServerStaticHeader();
                SV_BeginClientSnapshot(cl, &v2);
                if (cl->header.state == 4 || cl->header.state == 1)
                    SV_WriteSnapshotToClient(cl, &v2);
                SV_EndClientSnapshot(cl, &v2);
                SV_GetServerStaticHeader();
                cl->header.state = 1;
            }
            iassert(bgs == 0);
            {
                PROF_SCOPED("SV_UserMove");
                if (c)
                {
                    if (c == 1)
                    {
                        SV_UserMove(cl, &msgCompressed, 0);
                    }
                    else if (c != 3)
                    {
                        Com_PrintWarning(15, "WARNING: bad command byte %i for client %i\n", c, cl - svs.clients);
                    }
                }
                else
                {
                    SV_UserMove(cl, &msgCompressed, 1);
                }
            }
            iassert(bgs == 0);
        }
    }
    else if ((cl->serverId & 0xF0) == (sv_serverId_value & 0xF0))
    {
        if (cl->header.state == 3)
            SV_ClientEnterWorld(cl, &cl->lastUsercmd);
    }
    else if (SV_ProcessClientCommands(cl, &msgCompressed, 1, &c))
    {
        if (cl->messageAcknowledge > cl->gamestateMessageNum)
        {
            Com_DPrintf(15, "%s : dropped gamestate, resending\n", cl->name);
            SV_SendClientGameState(cl);
            if (SV_ShouldAuthorizeAddress(cl->header.netchan.remoteAddress))
                SV_AuthorizeRequest(cl->header.netchan.remoteAddress, cl->challenge, cl->cdkeyHash);
        }
    }
}

int __cdecl SV_ClientCommand(client_t *cl, msg_t *msg, int fromOldServer)
{
    int seq; // [esp+0h] [ebp-10h]
    int floodprotect; // [esp+4h] [ebp-Ch]
    char *s; // [esp+8h] [ebp-8h]
    int clientOk; // [esp+Ch] [ebp-4h]

    clientOk = 1;
    floodprotect = 1;
    seq = MSG_ReadLong(msg);
    s = MSG_ReadString(msg);
    if (cl->lastClientCommand >= seq)
        return 1;
    if (sv_showCommands->current.enabled)
        Com_Printf(15, "clientCommand: %i : %s\n", seq, s);
    if (seq <= cl->lastClientCommand + 1)
    {
        if (!I_strncmp("team ", s, 5) || !I_strncmp("score ", s, 6) || !I_strncmp("mr ", s, 3))
            floodprotect = 0;
        if (fromOldServer
            || cl->header.state >= 4
            && cl->header.netchan.remoteAddress.type != NA_LOOPBACK
            && sv_floodProtect->current.enabled
            && svs.time < cl->nextReliableTime
            && floodprotect)
        {
            clientOk = 0;
            Com_DPrintf(15, "client text ignored for %s: %s\n", cl->name, s);
        }
        if (floodprotect)
            cl->nextReliableTime = svs.time + 800;
        SV_ExecuteClientCommand(cl, s, clientOk, fromOldServer);
        cl->lastClientCommand = seq;
        Com_sprintf(cl->lastClientCommandString, 0x400u, "%s", s);
        return 1;
    }
    else
    {
        Com_Printf(15, "Client %s lost %i clientCommands\n", cl->name, seq - cl->lastClientCommand + 1);
        SV_DropClient(cl, "EXE_LOSTRELIABLECOMMANDS", 1);
        return 0;
    }
}

void __cdecl SV_ExecuteClientCommand(client_t *cl, char *s, int clientOK, int fromOldServer)
{
    const ucmd_t *u; // [esp+14h] [ebp-4h]

    SV_Cmd_TokenizeString(s);
    for (u = ucmds; u->name; ++u)
    {
        if (!strcmp(SV_Cmd_Argv(0), u->name))
        {
            if (!fromOldServer || u->allowFromOldServer)
                u->func(cl);
            break;
        }
    }
    if (clientOK && !u->name && SV_Loaded())
        ClientCommand(cl - svs.clients);
    SV_Cmd_EndTokenizedString();
}

gentity_s *__cdecl SV_AddTestClient()
{
    netadr_t v1; // [esp-14h] [ebp-458h]
    client_t *client; // [esp+0h] [ebp-444h]
    client_t *clienta; // [esp+0h] [ebp-444h]
    usercmd_s nullcmd; // [esp+4h] [ebp-440h] BYREF
    char file[1028]; // [esp+24h] [ebp-420h] BYREF
    int i; // [esp+428h] [ebp-1Ch]
    netadr_t a; // [esp+42Ch] [ebp-18h] BYREF

    if (!com_sv_running->current.enabled)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 3344, 0, "%s", "com_sv_running->current.enabled");
    i = 0;
    for (client = svs.clients; i < sv_maxclients->current.integer && client->header.state; ++client)
        ++i;
    if (i == sv_maxclients->current.integer)
        return 0;
    sprintf(
        file,
        "connect \"\\cg_predictItems\\1\\cl_punkbuster\\0\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\"
        "rate\\5000\\name\\bot%d\\protocol\\%d\\qport\\%d\"",
        botport,
        1,
        botport + 1);
    SV_Cmd_TokenizeString(file);
    *(unsigned int *)a.ip = 0;
    memset(a.ipx, 0, sizeof(a.ipx));
    a.type = NA_BOT;
    a.port = botport++;
    *(_QWORD *)&v1.type = 0;
    *(unsigned int *)&v1.port = a.port;
    *(_QWORD *)&v1.ipx[2] = 0;
    SV_DirectConnect(v1);
    SV_Cmd_EndTokenizedString();
    i = 0;
    for (clienta = svs.clients;
        i < sv_maxclients->current.integer
        && (!clienta->header.state || !NET_CompareBaseAdr(a, clienta->header.netchan.remoteAddress));
        ++clienta)
    {
        ++i;
    }
    if (i == sv_maxclients->current.integer)
        return 0;
    clienta->bIsTestClient = 1;
    SV_SendClientGameState(clienta);
    memset(&nullcmd, 0, sizeof(nullcmd));
    SV_ClientEnterWorld(clienta, &nullcmd);
    return SV_GentityNum(i);
}

