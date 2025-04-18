#include "server.h"
#include <qcommon/cmd.h>

#include <cgame_mp/cg_local_mp.h>
#include <game_mp/g_main_mp.h>

netadr_t adr;

const netadr_t *__cdecl SV_MasterAddress()
{
    __int16 v1; // ax

    if (adr.type == NA_BOT)
    {
        Com_Printf(15, "Resolving %s\n", com_masterServerName->current.string);
        if (NET_StringToAdr((char *)com_masterServerName->current.integer, &adr))
        {
            const char* result = strstr(":", (char*)com_masterServerName->current.integer);
            if (!result)
                adr.port = BigShort(com_masterPort->current.integer);
            v1 = BigShort(adr.port);
            Com_Printf(
                15,
                "%s resolved to %i.%i.%i.%i:%i\n",
                com_masterServerName->current.string,
                adr.ip[0],
                adr.ip[1],
                adr.ip[2],
                adr.ip[3],
                v1);
            if (adr.type == NA_BOT)
                MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 50, 0, "%s", "adr.type != 0");
        }
        else
        {
            if (adr.type != NA_BAD)
                MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 42, 0, "%s", "adr.type == NA_BAD");
            Com_Printf(15, "Couldn't resolve address: %s\n", com_masterServerName->current.string);
        }
    }
    return &adr;
}

void __cdecl SV_UpdateLastTimeMasterServerCommunicated(netadr_t from)
{
    netadr_t v1; // [esp-14h] [ebp-18h]

    v1 = *SV_MasterAddress();
    if (NET_CompareBaseAdr(from, v1))
        svs.sv_lastTimeMasterServerCommunicated = svs.time;
}


void __cdecl SV_MasterGameCompleteStatus()
{
    netadr_t *adr; // [esp+0h] [ebp-4h]

    if (com_dedicated && com_dedicated->current.integer == 2)
    {
        adr = (netadr_t *)SV_MasterAddress();
        if (!adr)
            MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 125, 0, "%s", "adr");
        if (adr->type != NA_BAD)
        {
            Com_Printf(15, "Sending gameCompleteStatus to %s\n", com_masterServerName->current.string);
            SVC_GameCompleteStatus(*adr);
        }
    }
}

void __cdecl SV_MasterHeartbeat(const char *hbname)
{
    const char *v1; // eax
    netadr_t *adr; // [esp+0h] [ebp-4h]

    if (com_dedicated && com_dedicated->current.integer == 2 && svs.time >= svs.nextHeartbeatTime)
    {
        svs.nextHeartbeatTime = svs.time + 180000;
        adr = (netadr_t *)SV_MasterAddress();
        if (!adr)
            MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 94, 0, "%s", "adr");
        if (adr->type != NA_BAD)
        {
            Com_Printf(15, "Sending heartbeat to %s\n", com_masterServerName->current.string);
            v1 = va("heartbeat %s\n", hbname);
            NET_OutOfBandPrint(NS_SERVER, *adr, v1);
        }
    }
}

void __cdecl SV_MasterShutdown()
{
    svs.nextHeartbeatTime = 0x80000000;
    SV_MasterHeartbeat("flatline");
}

void __cdecl SV_AuthorizeIpPacket(netadr_t from)
{
    const char *v1; // eax
    challenge_t *v2; // edx
    challenge_t *v3; // eax
    const char *v4; // eax
    netadr_t v5; // [esp-18h] [ebp-444h]
    const char *v6; // [esp-4h] [ebp-430h]
    const char *cdkeyHashFromAuth; // [esp+14h] [ebp-418h]
    const char *r; // [esp+18h] [ebp-414h]
    char ret[1024]; // [esp+1Ch] [ebp-410h] BYREF
    int challenge; // [esp+41Ch] [ebp-10h]
    const char *s; // [esp+420h] [ebp-Ch]
    int i; // [esp+424h] [ebp-8h]

    if (!NET_CompareBaseAdr(from, svs.authorizeAddress))
    {
        Com_Printf(15, "SV_AuthorizeIpPacket: not from authorize server\n");
        return;
    }
    v1 = SV_Cmd_Argv(1);
    challenge = atoi(v1);
    for (i = 0; i < 1024 && svs.challenges[i].challenge != challenge; ++i)
        ;
    if (i == 1024)
    {
        Com_Printf(15, "SV_AuthorizeIpPacket: challenge not found\n");
        return;
    }
    svs.challenges[i].pingTime = svs.time;
    s = SV_Cmd_Argv(2);
    r = SV_Cmd_Argv(3);
    cdkeyHashFromAuth = SV_Cmd_Argv(5);
    if (!I_stricmp(s, "deny"))
    {
        if (r && *r)
        {
            if (!I_stricmp(r, "CLIENT_UNKNOWN_TO_AUTH") || !I_stricmp(r, "BAD_CDKEY"))
            {
                NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "needcdkey");
            LABEL_22:
                memset(&svs.challenges[i], 0, sizeof(svs.challenges[i]));
                return;
            }
            if (I_stricmp(r, "INVALID_CDKEY"))
            {
                if (I_stricmp(r, "BANNED_CDKEY"))
                {
                    v3 = &svs.challenges[i];
                    *&v5.type = *&v3->adr.type;
                    *&v5.port = *&v3->adr.port;
                    *&v5.ipx[2] = *&v3->adr.ipx[2];
                }
                else
                {
                    v2 = &svs.challenges[i];
                    *&v5.type = *&v2->adr.type;
                    *&v5.port = *&v2->adr.port;
                    *&v5.ipx[2] = *&v2->adr.ipx[2];
                }
                NET_OutOfBandPrint(NS_SERVER, v5, "error\nEXE_ERR_BAD_CDKEY");
                goto LABEL_22;
            }
        }
        NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\nEXE_ERR_CDKEY_IN_USE");
        goto LABEL_22;
    }
    if (!strcmp(svs.challenges[i].cdkeyHash, cdkeyHashFromAuth))
    {
        if (I_stricmp(s, "demo"))
        {
            if (I_stricmp(s, "accept"))
            {
                if (r && *r)
                {
                    sprintf(ret, "error\n%s", r);
                    NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, ret);
                }
                else
                {
                    NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\nEXE_ERR_BAD_CDKEY");
                }
                memset(&svs.challenges[i], 0, sizeof(svs.challenges[i]));
            }
            else if (!svs.challenges[i].connected)
            {
                v6 = va("challengeResponse %i", svs.challenges[i].challenge);
                NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, v6);
            }
        }
        else if (Dvar_GetBool("fs_restrict"))
        {
            v4 = va("challengeResponse %i", svs.challenges[i].challenge);
            NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, v4);
        }
        else
        {
            NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\nEXE_ERR_NOT_A_DEMO_SERVER");
            memset(&svs.challenges[i], 0, sizeof(svs.challenges[i]));
        }
    }
    else
    {
        Com_Printf(
            15,
            "rejecting connection due to mismatched GUID: expected \"%s\", got \"%s\"\n",
            svs.challenges[i].cdkeyHash,
            cdkeyHashFromAuth);
        NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\nEXE_ERR_BAD_CDKEY");
        memset(&svs.challenges[i], 0, sizeof(svs.challenges[i]));
    }
}

void __cdecl SV_FlushRedirect(char *outputbuf)
{
    char c; // [esp+13h] [ebp-525h]
    char buf[1304]; // [esp+18h] [ebp-520h] BYREF
    int len; // [esp+534h] [ebp-4h]

    len = strlen(outputbuf);
    while (len > 1294)
    {
        c = outputbuf[1294];
        outputbuf[1294] = 0;
        Com_sprintf(buf, 0x514u, "print\n%s", outputbuf);
        NET_OutOfBandPrint(NS_SERVER, svs.redirectAddress, buf);
        len -= 1294;
        outputbuf += 1294;
        *outputbuf = c;
    }
    Com_sprintf(buf, 0x514u, "print\n%s", outputbuf);
    NET_OutOfBandPrint(NS_SERVER, svs.redirectAddress, buf);
}

int lasttime;
void __cdecl SVC_RemoteCommand(netadr_t from)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // [esp-Ch] [ebp-C2Ch]
    const char *v5; // [esp-4h] [ebp-C24h]
    const char *v6; // [esp-4h] [ebp-C24h]
    char remaining[1024]; // [esp+18h] [ebp-C08h] BYREF
    char sv_outputbuf[2032]; // [esp+418h] [ebp-808h] BYREF
    int valid; // [esp+C08h] [ebp-18h]
    int len; // [esp+C0Ch] [ebp-14h]
    const char *password; // [esp+C10h] [ebp-10h]
    int time; // [esp+C14h] [ebp-Ch]
    int i; // [esp+C18h] [ebp-8h]

    time = Sys_Milliseconds();
    if (!lasttime || time - lasttime >= 500)
    {
        lasttime = time;
        password = SV_Cmd_Argv(1);
        if (rcon_password->current.integer && !strcmp(password, rcon_password->current.string))
        {
            valid = 1;
            v6 = SV_Cmd_Argv(2);
            v2 = NET_AdrToString(from);
            Com_Printf(15, "Rcon from %s:\n%s\n", v2, v6);
        }
        else
        {
            valid = 0;
            v5 = SV_Cmd_Argv(2);
            v1 = NET_AdrToString(from);
            Com_Printf(15, "Bad rcon from %s:\n%s\n", v1, v5);
        }
        svs.redirectAddress = from;
        Com_BeginRedirect(sv_outputbuf, 0x7F0u, SV_FlushRedirect);
        if (rcon_password->current.integer)
        {
            if (valid)
            {
                len = 0;
                for (i = 2; i < SV_Cmd_Argc(); ++i)
                {
                    v4 = len;
                    v3 = SV_Cmd_Argv(i);
                    len = Com_AddToString(v3, remaining, v4, 1024, 1);
                    len = Com_AddToString(" ", remaining, len, 1024, 0);
                }
                if (len < 1024)
                {
                    remaining[len] = 0;
                    SV_Cmd_ExecuteString(0, 0, remaining);
                    //if (!I_strnicmp(remaining, "pb_sv_", 6))
                    //    PbServerForceProcess();
                }
            }
            else if (*password)
            {
                Com_Printf(15, "Invalid password.\n");
            }
            else
            {
                Com_Printf(15, "You must log in with 'rcon login <password>' before using 'rcon'.\n");
            }
        }
        else
        {
            Com_Printf(15, "The server must set 'rcon_password' for clients to use 'rcon'.\n");
        }
        Com_EndRedirect();
    }
}


void __cdecl SV_VoicePacket(netadr_t from, msg_t *msg)
{
    client_t *ClientByAddress; // [esp+0h] [ebp-Ch]
    int qport; // [esp+4h] [ebp-8h]

    qport = MSG_ReadShort(msg);
    ClientByAddress = SV_FindClientByAddress(from, qport);
    if (ClientByAddress && ClientByAddress->header.state != 1)
    {
        ClientByAddress->lastPacketTime = svs.time;
        if (ClientByAddress->header.state >= 4)
        {
            if (!ClientByAddress->gentity)
                MyAssertHandler(".\\server_mp\\sv_main_mp.cpp", 534, 0, "%s", "cl->gentity");
            SV_UserVoice(ClientByAddress, msg);
        }
        else
        {
            SV_PreGameUserVoice(ClientByAddress, msg);
        }
    }
}

