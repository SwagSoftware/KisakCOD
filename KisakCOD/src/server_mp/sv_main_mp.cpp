#include "server.h"

#include <qcommon/qcommon.h>
#include <qcommon/mem_track.h>
#include <qcommon/cmd.h>
#include <server/sv_game.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/files.h>
#include <universal/com_files.h>
#include <qcommon/threads.h>
#include <script/scr_variable.h>
#include <client/client.h>
#include <script/scr_vm.h>
#include <universal/timing.h>
#include <script/scr_debugger.h>



const dvar_t *sv_allowedClan2;
const dvar_t *sv_maxPing;
const dvar_t *sv_debugPacketContentsForClientThisFrame;
const dvar_t *sv_privateClients;
const dvar_t *sv_maxclients;
const dvar_t *sv_hostname;
const dvar_t *sv_allowedClan1;
const dvar_t *sv_smp    ;
const dvar_t *sv_debugReliableCmds;
const dvar_t *sv_clientSideBullets;
const dvar_t *sv_privateClientsForClients;
const dvar_t *sv_reconnectlimit;
const dvar_t *sv_kickBanTime;
const dvar_t *sv_floodProtect;
const dvar_t *sv_gametype;
const dvar_t *sv_mapname;
const dvar_t *sv_cheats ;
const dvar_t *sv_maxRate;
const dvar_t *sv_showCommands;
const dvar_t *sv_packet_info;
const dvar_t *sv_mapRotationCurrent;
const dvar_t *sv_connectTimeout;
const dvar_t *sv_disableClientConsole;
const dvar_t *sv_network_fps;
const dvar_t *sv_minPing;
const dvar_t *sv_mapcrc ;
const dvar_t *sv_debugPacketContents;
const dvar_t *sv_zombietime;
const dvar_t *sv_debugRate;
const dvar_t *sv_showAverageBPS;
const dvar_t *sv_timeout;
const dvar_t *sv_padPackets;
const dvar_t *sv_debugPlayerstate;
const dvar_t *sv_maxHappyPingTime;
const dvar_t *sv_endGameIfISuck;
const dvar_t *sv_debugMessageKey;
const dvar_t *sv_fps    ;
const dvar_t *sv_botsPressAttackBtn;
const dvar_t *sv_serverid;
const dvar_t *sv_mapRotation;

serverStatic_t svs;
server_t sv;

int com_inServerFrame;
int com_time;

void __cdecl TRACK_sv_main()
{
    track_static_alloc_internal(&svs, 0xB227480, "svs", 9);
    track_static_alloc_internal(&sv, 392288, "sv", 9);
}

char string_2[1024];
char *__cdecl SV_ExpandNewlines(char *in)
{
    unsigned int l; // [esp+0h] [ebp-4h]

    l = 0;
    while (*in && l < 1021)
    {
        if (*in == 10)
        {
            string_2[l] = 92;
            string_2[l + 1] = 110;
            l += 2;
            goto LABEL_10;
        }
        if (*in == '\x14' || *in == '\x15')
        {
            ++in;
        }
        else
        {
            string_2[l++] = *in;
        LABEL_10:
            ++in;
        }
    }
    string_2[l] = 0;
    return string_2;
}

void __cdecl SV_AddServerCommand(client_t *client, svscmd_type type, char *cmd)
{
    int from; // [esp+8h] [ebp-10h]
    int index; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    int to; // [esp+14h] [ebp-4h]

    if (!client->bIsTestClient)
    {
        if (client->reliableSequence - client->reliableAcknowledge < 64 && client->header.state == 4
            || (SV_CullIgnorableServerCommands(client), type))
        {
            to = SV_CanReplaceServerCommand(client, cmd);
            if (to < 0)
            {
                ++client->reliableSequence;
            }
            else
            {
                for (from = to + 1; from <= client->reliableSequence; ++from)
                    memcpy(
                        &client->reliableCommandInfo[to++ & 0x7F],
                        &client->reliableCommandInfo[from & 0x7F],
                        sizeof(client->reliableCommandInfo[to++ & 0x7F]));
            }
            if (client->reliableSequence - client->reliableAcknowledge == 129)
            {
                Com_Printf(15, "===== pending server commands =====\n");
                for (i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i)
                    Com_Printf(
                        15,
                        "cmd %5d: %8d: %s\n",
                        i,
                        client->reliableCommandInfo[i & 0x7F].time,
                        client->reliableCommandInfo[i & 0x7F].cmd);
                Com_Printf(15, "cmd %5d: %8d: %s\n", i, svs.time, cmd);
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "disconnect");
                SV_DelayDropClient(client, "EXE_SERVERCOMMANDOVERFLOW");
                type = SV_CMD_RELIABLE;
                cmd = va("%c \"EXE_SERVERCOMMANDOVERFLOW\"", 119);
            }
            index = client->reliableSequence & 0x7F;
            MSG_WriteReliableCommandToBuffer(cmd, client->reliableCommandInfo[index].cmd, 1024);
            client->reliableCommandInfo[index].time = svs.time;
            client->reliableCommandInfo[index].type = type;
        }
    }
}

int __cdecl SV_CanReplaceServerCommand(client_t *client, const char *cmd)
{
    int result; // eax
    int index; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    for (i = client->reliableSent + 1; ; ++i)
    {
        if (i > client->reliableSequence)
            return -1;
        index = i & 0x7F;
        if (client->reliableCommandInfo[index].type)
        {
            if (client->reliableCommandInfo[index].type != 1)
                MyAssertHandler(
                    ".\\server_mp\\sv_main_mp.cpp",
                    284,
                    0,
                    "%s",
                    "client->reliableCommandInfo[index].type == SV_CMD_RELIABLE");
            if (*cmd == client->reliableCommandInfo[index].cmd[0] && (*cmd < 120 || *cmd > 122))
                break;
        }
    LABEL_2:
        ;
    }
    if (!strcmp(cmd + 1, &client->reliableCommandInfo[index].cmd[1]))
        return i;
    switch (*cmd)
    {
    case 'C':
    case 'D':
    case 'a':
    case 'b':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 't':
        result = i;
        break;
    case 'd':
    case 'v':
        if (cmd[1] != 32)
            MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 317, 0, "%s", "cmd[1] == ' '");
        if (!SV_IsFirstTokenEqual(cmd + 2, &client->reliableCommandInfo[index].cmd[2]))
            goto LABEL_2;
        result = i;
        break;
    default:
        goto LABEL_2;
    }
    return result;
}

bool __cdecl SV_IsFirstTokenEqual(const char *str1, const char *str2)
{
    while (*str1 && *str2 && *str1 != 32 && *str2 != 32)
    {
        if (*str1 != *str2)
            return 0;
        ++str1;
        ++str2;
    }
    return (!*str1 || *str1 == 32) && (!*str2 || *str2 == 32);
}

void __cdecl SV_CullIgnorableServerCommands(client_t *client)
{
    int from; // [esp+8h] [ebp-10h]
    int fromIndex; // [esp+Ch] [ebp-Ch]
    int to; // [esp+14h] [ebp-4h]

    to = client->reliableSent + 1;
    for (from = to; from <= client->reliableSequence; ++from)
    {
        fromIndex = from & 0x7F;
        if (client->reliableCommandInfo[fromIndex].type)
        {
            if ((to & 0x7F) != fromIndex)
                memcpy(
                    &client->reliableCommandInfo[to & 0x7F],
                    &client->reliableCommandInfo[fromIndex],
                    sizeof(client->reliableCommandInfo[to & 0x7F]));
            ++to;
        }
    }
    client->reliableSequence = to - 1;
}

unsigned __int8 tempServerCommandBuf[131072];
void SV_SendServerCommand(client_t *cl, svscmd_type type, const char *fmt, ...)
{
    const char *v3; // eax
    client_t *client; // [esp+0h] [ebp-Ch]
    int j; // [esp+4h] [ebp-8h]
    va_list va; // [esp+20h] [ebp+14h] BYREF

    va_start(va, fmt);
    _vsnprintf((char *)tempServerCommandBuf, 0x20000u, fmt, va);
    if (cl)
    {
        SV_AddServerCommand(cl, type, (char *)tempServerCommandBuf);
    }
    else
    {
        if (com_dedicated->current.integer && !strncmp((const char *)tempServerCommandBuf, "print", 5u))
        {
            v3 = SV_ExpandNewlines((char *)tempServerCommandBuf);
            Com_Printf(15, "broadcast: %s\n", v3);
        }
        j = 0;
        client = svs.clients;
        while (j < sv_maxclients->current.integer)
        {
            if (client->header.state >= 3)
                SV_AddServerCommand(client, type, (char *)tempServerCommandBuf);
            ++j;
            ++client;
        }
    }
}

client_t *__cdecl SV_FindClientByAddress(netadr_t from, int qport)
{
    client_t *j; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]

    i = 0;
    for (j = svs.clients; ; ++j)
    {
        if (i >= sv_maxclients->current.integer)
            return 0;
        if (j->header.state
            && NET_CompareBaseAdr(from, j->header.netchan.remoteAddress)
            && j->header.netchan.qport == qport)
        {
            break;
        }
        ++i;
    }
    if (j->header.netchan.remoteAddress.port != from.port)
    {
        Com_Printf(15, "SV_ReadPackets: fixing up a translated port\n");
        j->header.netchan.remoteAddress.port = from.port;
    }
    return j;
}

void __cdecl SVC_Status(netadr_t from)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char v5; // [esp+3h] [ebp-4861h]
    unsigned __int8 *v6; // [esp+8h] [ebp-485Ch]
    char *v7; // [esp+Ch] [ebp-4858h]
    int ClientScore; // [esp+20h] [ebp-4844h]
    char v9; // [esp+27h] [ebp-483Dh]
    char *v10; // [esp+2Ch] [ebp-4838h]
    char *v11; // [esp+30h] [ebp-4834h]
    char dest[1024]; // [esp+34h] [ebp-4830h] BYREF
    client_t *v13; // [esp+434h] [ebp-4430h]
    int v14; // [esp+438h] [ebp-442Ch]
    char s[8196]; // [esp+43Ch] [ebp-4428h] BYREF
    char *text_in; // [esp+2440h] [ebp-2424h]
    char data[8192]; // [esp+2444h] [ebp-2420h] BYREF
    const char *v18; // [esp+4444h] [ebp-420h]
    const char *String; // [esp+4448h] [ebp-41Ch]
    int num; // [esp+444Ch] [ebp-418h]
    int v21; // [esp+4450h] [ebp-414h]
    int v22; // [esp+4454h] [ebp-410h]
    playerState_s *v23; // [esp+4458h] [ebp-40Ch]
    char v24; // [esp+445Ch] [ebp-408h] BYREF
    _BYTE v25[3]; // [esp+445Dh] [ebp-407h] BYREF
    int v26; // [esp+485Ch] [ebp-8h]

    v22 = 0;
    v11 = Dvar_InfoString(0, 4);
    v10 = s;
    do
    {
        v9 = *v11;
        *v10++ = *v11++;
    } while (v9);
    v1 = SV_Cmd_Argv(1);
    Info_SetValueForKey(s, "challenge", v1);
    if (Dvar_GetBool("fs_restrict"))
    {
        v2 = Info_ValueForKey(s, "sv_keywords");
        Com_sprintf(dest, 0x400u, "demo %s", v2);
        Info_SetValueForKey(s, "sv_keywords", dest);
    }
    tempServerMsgBuf[0] = 0;
    v21 = 0;
    for (num = 0; num < sv_maxclients->current.integer; ++num)
    {
        v13 = &svs.clients[num];
        if (v13->header.state >= 2)
        {
            v23 = SV_GameClientNum(num);
            if (gameInitialized)
            {
                ClientScore = G_GetClientScore(v13 - svs.clients);
                Com_sprintf(&v24, 0x400u, "%i %i \"%s\"\n", ClientScore, v13->ping, v13->name);
            }
            else
            {
                Com_sprintf(&v24, 0x400u, "%i %i \"%s\"\n", 0, v13->ping, v13->name);
            }
            v14 = &v25[strlen(&v24)] - v25;
            if (v14 + v21 >= 0x20000)
                break;
            v7 = &v24;
            v6 = &tempServerMsgBuf[v21];
            do
            {
                v5 = *v7;
                *v6++ = *v7++;
            } while (v5);
            v21 += v14;
        }
    }
    String = Dvar_GetString("g_password");
    if (String && *String)
        Info_SetValueForKey(s, "pswrd", "1");
    else
        Info_SetValueForKey(s, "pswrd", "0");
    v18 = Dvar_GetString("fs_game");
    if (!sv_pure->current.enabled || v18 && *v18)
    {
        v22 = 1;
    }
    else
    {
        text_in = (char *)Dvar_GetString("sv_referencedIwdNames");
        if (*text_in)
        {
            SV_Cmd_TokenizeString(text_in);
            v26 = SV_Cmd_Argc();
            for (num = 0; num < v26; ++num)
            {
                v3 = (char *)SV_Cmd_Argv(num);
                if (!FS_iwIwd((char*)v3, (char*)"main"))
                {
                    v22 = 1;
                    break;
                }
            }
            SV_Cmd_EndTokenizedString();
        }
    }
    v4 = va("%i", v22);
    Info_SetValueForKey(s, "mod", v4);
    Com_sprintf(data, 0x2000u, "statusResponse\n%s\n%s", s, (const char *)tempServerMsgBuf);
    NET_OutOfBandPrint(NS_SERVER, from, data);
}

void __cdecl SVC_GameCompleteStatus(netadr_t from)
{
    const char *v1; // eax
    const char *v2; // eax
    int ClientScore; // eax
    const char *v4; // eax
    int ping; // [esp-8h] [ebp-C58h]
    const char *name; // [esp-4h] [ebp-C54h]
    char v7; // [esp+3h] [ebp-C4Dh]
    unsigned __int8 *v8; // [esp+8h] [ebp-C48h]
    char *v9; // [esp+Ch] [ebp-C44h]
    char v10; // [esp+23h] [ebp-C2Dh]
    char *v11; // [esp+28h] [ebp-C28h]
    char *v12; // [esp+2Ch] [ebp-C24h]
    char keywords[1024]; // [esp+30h] [ebp-C20h] BYREF
    client_t *v14; // [esp+430h] [ebp-820h]
    int playerLength; // [esp+434h] [ebp-81Ch]
    char infostring[1028]; // [esp+438h] [ebp-818h] BYREF
    int i; // [esp+83Ch] [ebp-414h]
    int statusLength; // [esp+840h] [ebp-410h]
    playerState_s *ps; // [esp+844h] [ebp-40Ch]
    char player[1028]; // [esp+848h] [ebp-408h] BYREF

    v12 = Dvar_InfoString(0, 4);
    v11 = infostring;
    do
    {
        v10 = *v12;
        *v11++ = *v12++;
    } while (v10);
    v1 = SV_Cmd_Argv(1);
    Info_SetValueForKey(infostring, "challenge", v1);
    if (Dvar_GetBool("fs_restrict"))
    {
        v2 = Info_ValueForKey(infostring, "sv_keywords");
        Com_sprintf(keywords, 0x400u, "demo %s", v2);
        Info_SetValueForKey(infostring, "sv_keywords", keywords);
    }
    tempServerMsgBuf[0] = 0;
    statusLength = 0;
    for (i = 0; i < sv_maxclients->current.integer; ++i)
    {
        v14 = &svs.clients[i];
        if (v14->header.state >= 2)
        {
            ps = SV_GameClientNum(i);
            name = v14->name;
            ping = v14->ping;
            ClientScore = G_GetClientScore(v14 - svs.clients);
            Com_sprintf(player, 0x400u, "%i %i \"%s\"\n", ClientScore, ping, name);
            playerLength = &player[strlen(player) + 1] - &player[1];
            if (playerLength + statusLength >= 0x20000)
                break;
            v9 = player;
            v8 = &tempServerMsgBuf[statusLength];
            do
            {
                v7 = *v9;
                *v8++ = *v9++;
            } while (v7);
            statusLength += playerLength;
        }
    }
    v4 = va("gameCompleteStatus\n%s\n%s", infostring, (const char *)tempServerMsgBuf);
    NET_OutOfBandPrint(NS_SERVER, from, v4);
}

void __cdecl SVC_Info(netadr_t from)
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
    const char *v11; // eax
    const char *v12; // eax
    char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    int clientCount; // [esp+0h] [ebp-838h]
    int friendlyfire; // [esp+4h] [ebp-834h]
    char infostring[1028]; // [esp+8h] [ebp-830h] BYREF
    const char *refIwdNames; // [esp+40Ch] [ebp-42Ch]
    const char *gamedir; // [esp+410h] [ebp-428h]
    int killcam; // [esp+414h] [ebp-424h]
    int maxclients; // [esp+418h] [ebp-420h]
    const char *password; // [esp+41Ch] [ebp-41Ch]
    int privateClientCount; // [esp+420h] [ebp-418h]
    int i; // [esp+424h] [ebp-414h]
    int serverModded; // [esp+428h] [ebp-410h]
    int count; // [esp+42Ch] [ebp-40Ch]
    char response[1028]; // [esp+430h] [ebp-408h] BYREF

    serverModded = 0;
    privateClientCount = 0;
    for (i = 0; i < sv_privateClients->current.integer; ++i)
    {
        if (svs.clients[i].header.state >= 2)
            ++privateClientCount;
    }
    clientCount = privateClientCount;
    for (i = sv_privateClients->current.integer; i < sv_maxclients->current.integer; ++i)
    {
        if (svs.clients[i].header.state >= 2)
            ++clientCount;
    }
    infostring[0] = 0;
    v1 = SV_Cmd_Argv(1);
    Info_SetValueForKey(infostring, "challenge", v1);
    v2 = va("%i", 1);
    Info_SetValueForKey(infostring, "protocol", v2);
    Info_SetValueForKey(infostring, "hostname", sv_hostname->current.string);
    Info_SetValueForKey(infostring, "mapname", sv_mapname->current.string);
    if (clientCount)
    {
        v3 = va("%i", clientCount);
        Info_SetValueForKey(infostring, "clients", v3);
    }
    maxclients = sv_maxclients->current.integer - (sv_privateClients->current.integer - privateClientCount);
    if (maxclients > 0)
    {
        v4 = va("%i", maxclients);
        Info_SetValueForKey(infostring, "sv_maxclients", v4);
    }
    Info_SetValueForKey(infostring, "gametype", sv_gametype->current.string);
    if (sv_pure->current.enabled || fs_numServerIwds)
        Info_SetValueForKey(infostring, "pure", "1");
    if (sv_minPing->current.integer)
    {
        v5 = va("%i", sv_minPing->current.integer);
        Info_SetValueForKey(infostring, "minPing", v5);
    }
    if (sv_maxPing->current.integer)
    {
        v6 = va("%i", sv_maxPing->current.integer);
        Info_SetValueForKey(infostring, "maxPing", v6);
    }
    gamedir = Dvar_GetString("fs_game");
    if (*gamedir)
        Info_SetValueForKey(infostring, "game", gamedir);
    if (sv_allowAnonymous->current.enabled)
    {
        v7 = va("%i", sv_allowAnonymous->current.color[0]);
        Info_SetValueForKey(infostring, "sv_allowAnonymous", v7);
    }
    if (sv_disableClientConsole->current.enabled)
    {
        v8 = va("%i", sv_disableClientConsole->current.color[0]);
        Info_SetValueForKey(infostring, "con_disabled", v8);
    }
    password = Dvar_GetString("g_password");
    if (password && *password)
        Info_SetValueForKey(infostring, "pswrd", "1");
    friendlyfire = Dvar_GetInt("scr_team_fftype");
    if (friendlyfire)
    {
        v9 = va("%i", friendlyfire);
        Info_SetValueForKey(infostring, "ff", v9);
    }
    killcam = Dvar_GetInt("scr_game_allowkillcam");
    if (killcam)
    {
        v10 = va("%i", killcam);
        Info_SetValueForKey(infostring, "kc", v10);
    }
    if (com_dedicated && com_dedicated->current.integer)
    {
        v12 = va("%i", 2);
        Info_SetValueForKey(infostring, "hw", v12);
    }
    else
    {
        v11 = va("%i", 6);
        Info_SetValueForKey(infostring, "hw", v11);
    }
    if (!sv_pure->current.enabled || gamedir && *gamedir)
    {
        serverModded = 1;
    }
    else
    {
        refIwdNames = Dvar_GetString("sv_referencedIwdNames");
        if (*refIwdNames)
        {
            SV_Cmd_TokenizeString((char *)refIwdNames);
            count = SV_Cmd_Argc();
            for (i = 0; i < count; ++i)
            {
                v13 = (char *)SV_Cmd_Argv(i);
                if (!FS_iwIwd(v13, (char*)"main"))
                {
                    serverModded = 1;
                    break;
                }
            }
            SV_Cmd_EndTokenizedString();
        }
    }
    v14 = va("%i", serverModded);
    Info_SetValueForKey(infostring, "mod", v14);
    v15 = va("%i", sv_voice->current.color[0]);
    Info_SetValueForKey(infostring, "voice", v15);
    v16 = va("%i", sv_punkbuster->current.color[0]);
    Info_SetValueForKey(infostring, "pb", v16);
    I_strncpyz(response, "infoResponse\n", 1024);
    I_strncat(response, 1024, infostring);
    NET_OutOfBandPrint(NS_SERVER, from, response);
}

void __cdecl SV_ConnectionlessPacket(netadr_t from, msg_t *msg)
{
    const char *v2; // eax
    const char *v3; // eax
    char *fromAddr; // [esp+0h] [ebp-1Ch]
    client_t *clients; // [esp+4h] [ebp-18h]
    const char *c; // [esp+8h] [ebp-14h]
    int clientIndex; // [esp+Ch] [ebp-10h]
    char *s; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]

    clientIndex = -1;
    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    SV_Netchan_AddOOBProfilePacket(msg->cursize);
    if (I_strnicmp((const char *)msg->data + 4, "pb_", 3))
    {
        s = MSG_ReadStringLine(msg);
        SV_Cmd_TokenizeString(s);
        c = SV_Cmd_Argv(0);
        if (sv_packet_info->current.enabled)
        {
            v2 = NET_AdrToString(from);
            Com_Printf(15, "SV packet %s : %s\n", v2, c);
        }
        if (I_stricmp(c, "getstatus"))
        {
            if (I_stricmp(c, "v"))
            {
                if (I_stricmp(c, "getinfo"))
                {
                    if (I_stricmp(c, "getchallenge"))
                    {
                        if (I_stricmp(c, "connect"))
                        {
                            if (I_stricmp(c, "stats"))
                            {
                                if (I_stricmp(c, "ipAuthorize"))
                                {
                                    if (I_stricmp(c, "rcon"))
                                    {
                                        if (I_stricmp(c, "disconnect"))
                                        {
                                            v3 = NET_AdrToString(from);
                                            Com_DPrintf(15, "bad connectionless packet from %s\n", v3);
                                        }
                                    }
                                    else
                                    {
                                        SVC_RemoteCommand(from);
                                    }
                                }
                                else
                                {
                                    SV_UpdateLastTimeMasterServerCommunicated(from);
                                    SV_AuthorizeIpPacket(from);
                                }
                            }
                            else
                            {
                                SV_ReceiveStats(from, msg);
                            }
                        }
                        else
                        {
                            // LWSS: Remove punkbuster junk
                            //if (NET_IsLocalAddress(from))
                            //{
                            //    PbPassConnectString("localhost", (char *)msg->data);
                            //}
                            //else
                            //{
                            //    fromAddr = NET_AdrToString(from);
                            //    PbPassConnectString(fromAddr, (char *)msg->data);
                            //}
                            SV_DirectConnect(from);
                        }
                    }
                    else
                    {
                        SV_UpdateLastTimeMasterServerCommunicated(from);
                        SV_GetChallenge(from);
                    }
                }
                else
                {
                    SVC_Info(from);
                    SV_UpdateLastTimeMasterServerCommunicated(from);
                }
            }
            else
            {
                SV_VoicePacket(from, msg);
            }
        }
        else
        {
            SV_UpdateLastTimeMasterServerCommunicated(from);
            SVC_Status(from);
        }
        SV_Cmd_EndTokenizedString();
    }
    else
    {
        i = 0;
        clients = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (clients->header.state
                && NET_CompareBaseAdr(from, clients->header.netchan.remoteAddress)
                && clients->header.netchan.remoteAddress.port == from.port)
            {
                clientIndex = i;
                break;
            }
            ++i;
            ++clients;
        }
        // LWSS: Remove punkbuster crap
        //if (msg->data[7] != 67 && msg->data[7] != 49 && msg->data[7] != 74)
        //    PbSvAddEvent(13, clientIndex, msg->cursize - 4, (char *)msg->data + 4);
        //if (msg->data[7] != 83
        //    && msg->data[7] != 50
        //    && msg->data[7] != 71
        //    && msg->data[7] != 73
        //    && msg->data[7] != 89
        //    && msg->data[7] != 66
        //    && msg->data[7] != 76
        //    && (!com_dedicated || !com_dedicated->current.integer))
        //{
        //    PbClAddEvent(13, msg->cursize - 4, (char *)msg->data + 4);
        //}
    }
}

void __cdecl SV_PacketEvent(netadr_t from, msg_t *msg)
{
    client_t *client; // [esp+0h] [ebp-Ch]
    int qport; // [esp+4h] [ebp-8h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 1336, 0, "%s", "Sys_IsMainThread()");
    if (msg->cursize >= 4 && *(unsigned int *)msg->data == -1)
    {
        SV_ConnectionlessPacket(from, msg);
    }
    else
    {
        SV_ResetSkeletonCache();
        MSG_BeginReading(msg);
        MSG_ReadLong(msg);
        qport = MSG_ReadShort(msg);
        client = SV_FindClientByAddress(from, qport);
        if (client)
        {
            if (Netchan_Process(&client->header.netchan, msg))
            {
                client->serverId = MSG_ReadByte(msg);
                client->messageAcknowledge = MSG_ReadLong(msg);
                if (client->messageAcknowledge >= 0)
                {
                    client->reliableAcknowledge = MSG_ReadLong(msg);
                    if (client->reliableSequence - client->reliableAcknowledge < 128)
                    {
                        SV_Netchan_Decode(client, &msg->data[msg->readcount], msg->cursize - msg->readcount);
                        if (client->header.state != 1)
                        {
                            if (bgs)
                                MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 1406, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
                            client->lastPacketTime = svs.time;
                            SV_ExecuteClientMessage(client, msg);
                        }
                    }
                    else
                    {
                        Com_Printf(
                            15,
                            "Out of range reliableAcknowledge message from %s - cl->reliableSequence is %i, reliableAcknowledge is %i\n",
                            client->name,
                            client->reliableSequence,
                            client->reliableAcknowledge);
                        client->reliableAcknowledge = client->reliableSequence;
                    }
                }
                else
                {
                    Com_Printf(
                        15,
                        "Invalid reliableAcknowledge message from %s - reliableAcknowledge is %i\n",
                        client->name,
                        client->reliableAcknowledge);
                }
            }
        }
        else
        {
            NET_OutOfBandPrint(NS_SERVER, from, "disconnect");
        }
    }
}

void __cdecl SV_CalcPings()
{
    int j; // [esp+4h] [ebp-18h]
    client_t *v1; // [esp+8h] [ebp-14h]
    int total; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    int count; // [esp+18h] [ebp-4h]

    for (i = 0; i < sv_maxclients->current.integer; ++i)
    {
        v1 = &svs.clients[i];
        if (v1->header.state == 4)
        {
            if (v1->gentity)
            {
                total = 0;
                count = 0;
                for (j = 0; j < 32; ++j)
                {
                    if (v1->frames[j].messageAcked > 0)
                    {
                        ++count;
                        total += v1->frames[j].messageAcked - v1->frames[j].messageSent;
                    }
                }
                if (count)
                {
                    v1->ping = total / count;
                    if (v1->ping > 999)
                    {
                        Com_DPrintf(15, "Giving %s a 999 ping - >999 calculated ping:\n", v1->name);
                        v1->ping = 999;
                    }
                }
                else
                {
                    if (v1->header.netchan.remoteAddress.type)
                        Com_DPrintf(15, "Giving %s a 999 ping - !count:\n", v1->name);
                    v1->ping = 999;
                }
            }
            else
            {
                Com_DPrintf(15, "Giving %s a 999 ping - not a gentity\n", v1->name);
                v1->ping = 999;
            }
        }
        else
        {
            v1->ping = 999;
        }
    }
}

void __cdecl SV_FreeClientScriptId(client_t *cl)
{
    Com_Printf(15, "SV_FreeClientScriptId: %d, %d -> 0\n", cl - svs.clients, cl->scriptId);
    if (!cl->scriptId)
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 1555, 0, "%s", "cl->scriptId");
    Scr_FreeValue(cl->scriptId);
    cl->scriptId = 0;
}

void __cdecl SV_CheckTimeouts()
{
    client_t *drop; // [esp+0h] [ebp-14h]
    int connectdroppoint; // [esp+4h] [ebp-10h]
    int zombiepoint; // [esp+8h] [ebp-Ch]
    int droppoint; // [esp+Ch] [ebp-8h]
    int clientNum; // [esp+10h] [ebp-4h]

    droppoint = svs.time - 1000 * sv_timeout->current.integer;
    connectdroppoint = svs.time - 1000 * sv_connectTimeout->current.integer;
    zombiepoint = svs.time - 1000 * sv_zombietime->current.integer;
    clientNum = 0;
    drop = svs.clients;
    while (clientNum < sv_maxclients->current.integer)
    {
        if (drop->lastPacketTime > svs.time)
            drop->lastPacketTime = svs.time;
        if (!drop->bIsTestClient)
        {
            if (drop->header.state == 1 && drop->lastPacketTime < zombiepoint)
            {
                Com_DPrintf(15, "Going from CS_ZOMBIE to CS_FREE for client #%i\n", clientNum);
                drop->header.state = 0;
                drop->lastPacketTime = 0;
            }
            else if (drop->header.state == 4 && drop->lastPacketTime < droppoint)
            {
                if (++drop->timeoutCount > 5)
                    SV_DropClient(drop, "EXE_TIMEDOUT", 1);
            }
            else if (drop->header.state < 2 || drop->lastPacketTime >= connectdroppoint)
            {
                drop->timeoutCount = 0;
            }
            else if (++drop->timeoutCount > 5)
            {
                SV_DropClient(drop, "EXE_TIMEDOUT", 1);
            }
        }
        ++clientNum;
        ++drop;
    }
}

int __cdecl SV_CheckPaused()
{
    client_t *clients; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    if (!cl_paused->current.integer)
        return 0;
    count = 0;
    i = 0;
    clients = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (clients->header.state >= 2)
            ++count;
        ++i;
        ++clients;
    }
    if (count <= 1)
    {
        Dvar_SetInt((dvar_s *)sv_paused, 1);
        return 1;
    }
    else
    {
        Dvar_SetInt((dvar_s *)sv_paused, 0);
        return 0;
    }
}

void __cdecl SV_RunFrame()
{
    unsigned __int64 v0; // [esp+40h] [ebp-44h]
    unsigned __int64 start; // [esp+6Ch] [ebp-18h]
    float time; // [esp+78h] [ebp-Ch]
    unsigned __int64 ticks; // [esp+7Ch] [ebp-8h]

    //Profile_Begin(256);
    if (Win_GetThreadLock() == THREAD_LOCK_ALL)
        start = __rdtsc();
    else
        start = (int)Sys_MillisecondsRaw();
    SV_ResetSkeletonCache();
    CL_FlushDebugServerData();
    G_RunFrame(svs.time);
    Scr_ProfileUpdate();
    Scr_ProfileBuiltinUpdate();
    //Profile_ResetCounters(1);
    //Profile_ResetScriptCounters();
    CL_UpdateDebugServerData();
    if (Win_GetThreadLock() == THREAD_LOCK_ALL)
        v0 = __rdtsc();
    else
        v0 = (int)Sys_MillisecondsRaw();
    ticks = v0 - start;
    if (Win_GetThreadLock() == THREAD_LOCK_ALL)
        time = (double)ticks * msecPerRawTimerTick;
    else
        time = (float)ticks;
    SV_UpdatePerformanceFrame(time);
    //Profile_EndInternal(0);
}

int s_lastWallClockEndTime;
int s_serverDebugFrame;
ServerProfileTimes s_serverProfileTimes[10];
void __cdecl SV_UpdatePerformanceFrame(float time)
{
    float v1; // [esp+0h] [ebp-28h]
    float v2; // [esp+4h] [ebp-24h]
    float v3; // [esp+8h] [ebp-20h]
    float v4; // [esp+Ch] [ebp-1Ch]
    DWORD wallClockEndTime; // [esp+14h] [ebp-14h]
    ServerProfileTimes *profile; // [esp+18h] [ebp-10h]
    float maxTime; // [esp+1Ch] [ebp-Ch]
    float minTime; // [esp+20h] [ebp-8h]
    int i; // [esp+24h] [ebp-4h]

    minTime = 3.4028235e38;
    maxTime = -3.4028235e38;
    wallClockEndTime = Sys_Milliseconds();
    //profile = (8 * (s_serverDebugFrame % 10) + 229164768);
    profile = &s_serverProfileTimes[s_serverDebugFrame % 10];
    ++s_serverDebugFrame;
    profile->frameTime = time;
    profile->wallClockTime = (wallClockEndTime - s_lastWallClockEndTime);
    s_lastWallClockEndTime = wallClockEndTime;
    if (s_serverDebugFrame >= 10)
    {
        sv.profile.frameTime = 0.0;
        sv.profile.wallClockTime = 0.0;
        for (i = 0; i < 10; ++i)
        {
            sv.profile.frameTime = sv.profile.frameTime + s_serverProfileTimes[i].frameTime;
            sv.profile.wallClockTime = sv.profile.wallClockTime + s_serverProfileTimes[i].wallClockTime;
            if (s_serverProfileTimes[i].frameTime < minTime)
                minTime = s_serverProfileTimes[i].frameTime;
            if (s_serverProfileTimes[i].frameTime > maxTime)
                maxTime = s_serverProfileTimes[i].frameTime;
        }
        sv.profile.frameTime = sv.profile.frameTime * 0.1000000014901161;
        sv.profile.wallClockTime = sv.profile.wallClockTime * 0.1000000014901161;
        v4 = minTime - 1.0;
        if (v4 < 0.0)
            v3 = 1.0;
        else
            v3 = minTime;
        sv.serverFrameTimeMin = v3;
        v2 = maxTime - 1.0;
        if (v2 < 0.0)
            v1 = 1.0;
        else
            v1 = maxTime;
        sv.serverFrameTimeMax = v1;
    }
}

void __cdecl SV_BotUserMove(client_t *cl)
{
    playerState_s *v1; // esi
    usercmd_s nullcmd; // [esp+4h] [ebp-24h] BYREF

    if (cl->gentity)
    {
        memset(&nullcmd, 0, sizeof(nullcmd));
        v1 = SV_GameClientNum(cl - svs.clients);
        if (v1->weapon != LOBYTE(SV_GameClientNum(cl - svs.clients)->weapon))
            MyAssertHandler(
                ".\\server_mp\\sv_main_mp.cpp",
                1876,
                0,
                "%s",
                "SV_GameClientNum( cl - svs.clients )->weapon == static_cast<byte>( SV_GameClientNum( cl - svs.clients )->weapon )");
        nullcmd.weapon = SV_GameClientNum(cl - svs.clients)->weapon;
        if (!G_GetClientArchiveTime(cl - svs.clients))
        {
            if (random() < 0.5 && sv_botsPressAttackBtn->current.enabled)
                nullcmd.buttons |= 1u;
            if (random() < 0.5)
                nullcmd.buttons |= 0x28u;
            if (random() >= 0.3300000131130219)
            {
                if (random() < 0.5)
                    nullcmd.forwardmove = -127;
            }
            else
            {
                nullcmd.forwardmove = 127;
            }
            if (random() >= 0.3300000131130219)
            {
                if (random() < 0.5)
                    nullcmd.rightmove = -127;
            }
            else
            {
                nullcmd.rightmove = 127;
            }
            if (random() < 0.3300000131130219)
                nullcmd.angles[0] = (int)(crandom() * 360.0);
            if (random() < 0.3300000131130219)
                nullcmd.angles[1] = (int)(crandom() * 360.0);
            if (random() < 0.3300000131130219)
                nullcmd.angles[2] = (int)(crandom() * 360.0);
        }
        cl->header.deltaMessage = cl->header.netchan.outgoingSequence - 1;
        SV_ClientThink(cl, &nullcmd);
    }
}

void __cdecl SV_UpdateBots()
{
    client_t *clients; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    //Profile_Begin(303);
    SV_ResetSkeletonCache();
    i = 0;
    clients = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (clients->header.state)
        {
            if (clients->header.netchan.remoteAddress.type == NA_BOT)
                SV_BotUserMove(clients);
        }
        ++i;
        ++clients;
    }
    //Profile_EndInternal(0);
}

void __cdecl SV_WaitServer()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2156, 0, "%s", "Sys_IsMainThread()");
    if (com_inServerFrame)
    {
        com_inServerFrame = 0;
        if (!com_sv_running)
            MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2163, 0, "%s", "com_sv_running");
        if (!com_sv_running->current.enabled)
            MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2164, 0, "%s", "com_sv_running->current.enabled");
        SV_RunFrame();
    }
}

void __cdecl SV_InitSnapshot()
{
    if (com_inServerFrame)
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2186, 0, "%s", "!com_inServerFrame");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2188, 0, "%s", "Sys_IsMainThread()");
    sv.inFrame = 0;
}

void __cdecl SV_KillLocalServer()
{
    if (com_sv_running->current.enabled)
        sv.killServer = 1;
}

void __cdecl SV_SetSystemInfoConfig()
{
    char *v0; // eax
    char dest[0x2000]; // [esp+24h] [ebp-2008h] BYREF

    v0 = Dvar_InfoString_Big(8);
    I_strncpyz(dest, v0, 0x2000);
    if (!fs_gameDirVar->current.integer)
    {
        if (strlen(dest) + strlen("\\fs_game\\\\") <= 0x400)
            I_strncat(dest, 1024, "\\fs_game\\\\");
        else
            Com_Printf(16, "Info string length exceeded key: fs_game Info string: %s", dest);
    }
    SV_SetConfigstring(1, dest);
    dvar_modifiedFlags &= ~8u;
}

void __cdecl SV_PreFrame()
{
    char *v0; // eax

    //Profile_Begin(255);
    CL_ResetStats_f();
    SV_UpdateBots();
    if ((dvar_modifiedFlags & 0x404) != 0)
    {
        v0 = Dvar_InfoString(0, 4);
        SV_SetConfigstring(0, v0);
        dvar_modifiedFlags &= 0xFFFFFBFB;
    }
    if ((dvar_modifiedFlags & 8) != 0)
        SV_SetSystemInfoConfig();
    if ((dvar_modifiedFlags & 0x100) != 0)
    {
        SV_SetConfig(20, 128, 256);
        dvar_modifiedFlags &= ~0x100u;
    }
    //Profile_EndInternal(0);
}

int __cdecl SV_Frame(int msec)
{
    //Profile_Begin(255);
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if (sv.killServer)
    {
        if (sv.killReason && *sv.killReason)
            Com_Shutdown(sv.killReason);
        else
            Com_Shutdown("EXE_SERVERKILLED");
        sv.killServer = 0;
        sv.killReason = 0;
        //Profile_EndInternal(0);
        return msec;
    }
    else
    {
        if (com_sv_running->current.enabled)
        {
            if (!sv_pure->current.enabled && fs_numServerIwds)
                FS_ShutdownServerIwdNames();
            if (SV_CheckPaused())
                SV_WaitServer();
            else
                SV_FrameInternal(msec);
        }
        //Profile_EndInternal(0);
        return msec;
    }
}

void __cdecl SV_FrameInternal(int msec)
{
    int frameMsec; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2343, 0, "%s", "Sys_IsMainThread()");
    if (msec < 0)
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2344, 0, "%s", "msec >= 0");
    frameMsec = 1000 / sv_fps->current.integer;
    if (sv.timeResidual >= frameMsec)
        MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2349, 0, "%s", "sv.timeResidual < frameMsec");
    sv.timeResidual += msec;
    if (sv.timeResidual >= frameMsec && !SV_CheckOverflow())
    {
        SV_CalcPings();
        SV_PreFrame();
        while (1)
        {
            sv.timeResidual -= frameMsec;
            svs.time += frameMsec;
            SV_RunFrame();
            Scr_SetLoading(0);
            if (sv.timeResidual < frameMsec)
                break;
            SV_PostFrame();
        }
        SV_PostFrame();
        if (sv.timeResidual >= frameMsec)
            MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 2422, 0, "%s", "sv.timeResidual < frameMsec");
    }
}

void SV_PostFrame()
{
    Scr_UpdateDebugger();
    //Profile_Begin(305);
    SV_CheckTimeouts();
    //Profile_EndInternal(0);
    SV_SendClientMessages();
    SV_MasterHeartbeat("COD-4");
    //Profile_Begin(306);
    FakeLag_Frame();
    //return Profile_EndInternal(0);
}

char __cdecl SV_CheckOverflow()
{
    const char *v0; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    char mapname[68]; // [esp+0h] [ebp-48h] BYREF

    if (svs.time <= 1879048192)
    {
        if (svs.nextSnapshotEntities < 2147483646 - svs.numSnapshotEntities)
        {
            if (svs.nextCachedSnapshotEntities < 2147467262)
            {
                if (svs.nextCachedSnapshotClients < 2147479550)
                {
                    if (svs.nextArchivedSnapshotFrames < 2147482446)
                    {
                        if (svs.nextArchivedSnapshotBuffer < 2113929214)
                        {
                            if (svs.nextCachedSnapshotFrames < 2147483134)
                            {
                                if (svs.nextSnapshotClients < 2147483646 - svs.numSnapshotClients)
                                {
                                    return 0;
                                }
                                else
                                {
                                    I_strncpyz(mapname, sv_mapname->current.string, 64);
                                    Com_Shutdown("EXE_SERVERRESTARTMISC numSnapshotClients");
                                    v8 = va("map %s\n", mapname);
                                    Cbuf_AddText(0, v8);
                                    return 1;
                                }
                            }
                            else
                            {
                                I_strncpyz(mapname, sv_mapname->current.string, 64);
                                Com_Shutdown("EXE_SERVERRESTARTMISC #KISAKTODO");
                                v7 = va("map %s\n", mapname);
                                Cbuf_AddText(0, v7);
                                return 1;
                            }
                        }
                        else
                        {
                            I_strncpyz(mapname, sv_mapname->current.string, 64);
                            Com_Shutdown("EXE_SERVERRESTARTMISC #KISAKTODO");
                            v6 = va("map %s\n", mapname);
                            Cbuf_AddText(0, v6);
                            return 1;
                        }
                    }
                    else
                    {
                        I_strncpyz(mapname, sv_mapname->current.string, 64);
                        Com_Shutdown("EXE_SERVERRESTARTMISC #KISAKTODO");
                        v5 = va("map %s\n", mapname);
                        Cbuf_AddText(0, v5);
                        return 1;
                    }
                }
                else
                {
                    I_strncpyz(mapname, sv_mapname->current.string, 64);
                    Com_Shutdown("EXE_SERVERRESTARTMISC #KISAKTODO");
                    v4 = va("map %s\n", mapname);
                    Cbuf_AddText(0, v4);
                    return 1;
                }
            }
            else
            {
                I_strncpyz(mapname, sv_mapname->current.string, 64);
                Com_Shutdown("EXE_SERVERRESTARTMISC #KISAKTODO");
                v3 = va("map %s\n", mapname);
                Cbuf_AddText(0, v3);
                return 1;
            }
        }
        else
        {
            I_strncpyz(mapname, sv_mapname->current.string, 64);
            Com_Shutdown("EXE_SERVERRESTARTMISC #KISAKTODO");
            v2 = va("map %s\n", mapname);
            Cbuf_AddText(0, v2);
            return 1;
        }
    }
    else
    {
        I_strncpyz(mapname, sv_mapname->current.string, 64);
        Com_Shutdown("EXE_SERVERRESTARTTIMEWRAP");
        v0 = va("map %s\n", mapname);
        Cbuf_AddText(0, v0);
        return 1;
    }
}

