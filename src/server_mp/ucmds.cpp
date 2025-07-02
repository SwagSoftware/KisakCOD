// KISAK original file
// These are the Server->Client commands for the `ucmds` array in sv_client_mp.cpp
#include "server_mp.h"
#include <qcommon/cmd.h>
#include <universal/com_files.h>


void __cdecl SV_UnmutePlayer_f(client_t *cl)
{
    const char *v1; // eax
    unsigned int otherClient; // [esp+0h] [ebp-4h]

    v1 = SV_Cmd_Argv(1);
    otherClient = atoi(v1);
    if (otherClient <= 0x40)
        cl->muteList[otherClient] = 0;
    else
        Com_Printf(0, "Invalid unmute client %i\n", otherClient);
}

void __cdecl SV_MutePlayer_f(client_t *cl)
{
    const char *v1; // eax
    unsigned int otherClient; // [esp+0h] [ebp-4h]

    v1 = SV_Cmd_Argv(1);
    otherClient = atoi(v1);
    if (otherClient <= 0x40)
        cl->muteList[otherClient] = 1;
    else
        Com_Printf(0, "Invalid mute client %i\n", otherClient);
}

void __cdecl SV_WWWDownLoad_Clear(client_t *cl)
{
    if (!cl)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1866, 0, "%s", "cl");
    SV_Download_Clear(cl);
    cl->clientDownloadingWWW = 0;
    cl->downloadingWWW = 0;
}

void __cdecl SV_WWWDownload_f(client_t *cl)
{
    const char *subcmd; // [esp+0h] [ebp-4h]

    subcmd = SV_Cmd_Argv(1);
    if (!cl->downloadingWWW)
        goto LABEL_2;
    if (I_stricmp(subcmd, "ack"))
    {
        if (I_stricmp(subcmd, "bbl8r"))
        {
            if (!cl->clientDownloadingWWW)
            {
            LABEL_2:
                Com_Printf(15, "SV_WWWDownload: unexpected wwwdl '%s' for client '%s'\n", subcmd, cl->name);
                SV_DropClient(cl, "PC_PATCH_1_1_UNEXPECTEDDOWLOADMESSAGE", 1);
                return;
            }
            if (I_stricmp(subcmd, "done"))
            {
                if (I_stricmp(subcmd, "fail"))
                {
                    if (I_stricmp(subcmd, "chkfail"))
                    {
                        Com_Printf(15, "SV_WWWDownload: unknown wwwdl subcommand '%s' for client '%s'\n", subcmd, cl->name);
                        SV_DropClient(cl, "PC_PATCH_1_1_UNEXPECTEDDOWLOADMESSAGE", 1);
                    }
                    else
                    {
                        Com_Printf(
                            15,
                            "WARNING: client '%s' reports that the redirect download for '%s' had wrong checksum.\n",
                            cl->name,
                            cl->downloadName);
                        Com_Printf(15, "         you should check your download redirect configuration.\n");
                        SV_WWWDownLoad_Clear(cl);
                        cl->wwwFallback = 1;
                        SV_SendClientGameState(cl);
                    }
                }
                else
                {
                    SV_WWWDownLoad_Clear(cl);
                    cl->wwwFallback = 1;
                    Com_Printf(
                        15,
                        "Client '%s' reported that the http download of '%s' failed, falling back to a server download\n",
                        cl->name,
                        cl->downloadName);
                    SV_SendClientGameState(cl);
                }
            }
            else
            {
                SV_WWWDownLoad_Clear(cl);
            }
        }
        else
        {
            SV_DropClient(cl, "PC_PATCH_1_1_DOWNLOADDISCONNECTED", 1);
        }
    }
    else
    {
        if (cl->clientDownloadingWWW)
            Com_Printf(15, "WARNING: dupe wwwdl ack from client '%s'\n", cl->name);
        cl->clientDownloadingWWW = 1;
    }
}

void __cdecl SV_RetransmitDownload_f(client_t *cl)
{
    const char *v1; // eax

    v1 = SV_Cmd_Argv(1);
    if (atoi(v1) == cl->downloadClientBlock)
        cl->downloadXmitBlock = cl->downloadClientBlock;
}

void __cdecl SV_DoneDownload_f(client_t *cl)
{
    Com_DPrintf(0, "clientDownload: %s Done\n", cl->name);
    SV_Download_Clear(cl);
    SV_SendClientGameState(cl);
}

void __cdecl SV_StopDownload_f(client_t *cl)
{
    if (cl->downloadName[0])
        Com_DPrintf(0, "clientDownload: %d : file \"%s\" aborted\n", cl - svs.clients, cl->downloadName);
    SV_CloseDownload(cl);
    SV_Download_Clear(cl);
}

void __cdecl SV_ResetPureClient_f(client_t *cl)
{
    cl->pureAuthentic = 0;
}

void __cdecl SV_NextDownload_f(client_t *cl)
{
    const char *v1; // eax
    int block; // [esp+0h] [ebp-4h]

    v1 = SV_Cmd_Argv(1);
    block = atoi(v1);
    if (block == cl->downloadClientBlock)
    {
        Com_DPrintf(0, "clientDownload: %d : client acknowledge of block %d\n", cl - svs.clients, block);
        if (cl->downloadBlockSize[cl->downloadClientBlock % 8])
        {
            cl->downloadSendTime = svs.time;
            ++cl->downloadClientBlock;
        }
        else
        {
            Com_Printf(0, "clientDownload: %d : file \"%s\" completed\n", cl - svs.clients, cl->downloadName);
            SV_CloseDownload(cl);
        }
    }
    else
    {
        SV_DropClient(cl, "broken download", 1);
    }
}

void __cdecl SV_BeginDownload_f(client_t *cl)
{
	const char *v1; // eax

	SV_CloseDownload(cl);
	cl->downloading = 1;
	v1 = SV_Cmd_Argv(1);
	I_strncpyz(cl->downloadName, v1, 64);
}

void __cdecl SV_VerifyIwds_f(client_t *cl)
{
    int v1; // eax
    const char *v2; // eax
    int v3; // eax
    const char *nptr; // [esp+0h] [ebp-2038h]
    int j; // [esp+Ch] [ebp-202Ch]
    int k; // [esp+Ch] [ebp-202Ch]
    const char *v7; // [esp+10h] [ebp-2028h]
    int v8; // [esp+14h] [ebp-2024h]
    int v9; // [esp+14h] [ebp-2024h]
    char *text_in; // [esp+18h] [ebp-2020h]
    int v11; // [esp+20h] [ebp-2018h]
    int checksumFeed; // [esp+24h] [ebp-2014h]
    _DWORD v13[1024]; // [esp+28h] [ebp-2010h]
    int v14; // [esp+1028h] [ebp-1010h]
    int i; // [esp+102Ch] [ebp-100Ch]
    _DWORD v16[1025]; // [esp+1030h] [ebp-1008h]
    int v17; // [esp+2034h] [ebp-4h]

    v11 = 1;
    v8 = SV_Cmd_Argc();
    v17 = 1;
    if (v8 >= 2)
    {
        v7 = SV_Cmd_Argv(v17++);
        if (*v7 == 64)
        {
            i = 0;
            while (v17 < v8)
            {
                nptr = SV_Cmd_Argv(v17);
                v1 = atoi(nptr);
                v13[i] = v1;
                ++v17;
                ++i;
            }
            v9 = i - 1;
            for (i = 0; i < v9; ++i)
            {
                for (j = 0; j < v9; ++j)
                {
                    if (i != j && v13[i] == v13[j])
                    {
                        v11 = 0;
                        break;
                    }
                }
                if (!v11)
                    break;
            }
            if (v11)
            {
                text_in = FS_LoadedIwdPureChecksums();
                SV_Cmd_TokenizeString(text_in);
                v14 = SV_Cmd_Argc();
                if (v14 > 1024)
                    v14 = 1024;
                for (i = 0; i < v14; ++i)
                {
                    v2 = SV_Cmd_Argv(i);
                    v3 = atoi(v2);
                    v16[i] = v3;
                }
                SV_Cmd_EndTokenizedString();
                for (i = 0; i < v9; ++i)
                {
                    for (k = 0; k < v14 && v13[i] != v16[k]; ++k)
                        ;
                    if (k >= v14)
                    {
                        v11 = 0;
                        break;
                    }
                }
                if (v11)
                {
                    checksumFeed = sv.checksumFeed;
                    for (i = 0; i < v9; ++i)
                        checksumFeed ^= v13[i];
                    if ((v9 ^ checksumFeed) != v13[v9])
                        v11 = 0;
                }
            }
        }
        else
        {
            v11 = 0;
        }
    }
    else
    {
        v11 = 0;
    }
    if (v11)
        cl->pureAuthentic = 1;
    else
        cl->pureAuthentic = 2;
}