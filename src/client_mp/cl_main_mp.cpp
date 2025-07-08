#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "client_mp.h"

#include <xanim/xanim.h>
#include <universal/assertive.h>
#include <qcommon/threads.h>
#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>
#include <qcommon/com_fileaccess.h>

#include <universal/com_memory.h>
#include <universal/q_parse.h>
#include <win32/win_local.h>
#include <win32/win_storage.h>
#include <gfx_d3d/r_rendercmds.h>
#include <physics/phys_local.h>
#include <devgui/devgui.h>
#include <client/client.h>
#include <qcommon/sv_msg_write_mp.h>
#include <cgame_mp/cg_local_mp.h>
#include <universal/com_files.h>
#include <ragdoll/ragdoll.h>
#include <DynEntity/DynEntity_client.h>
#include <database/database.h>
#include <sound/snd_local.h>
#include <stringed/stringed_hooks.h>
#include <gfx_d3d/r_init.h>
#include <qcommon/dl_main.h>
#include <server_mp/server_mp.h>
#include <universal/profile.h>
#include <qcommon/com_bsp.h>

#include <ui_mp/ui_mp.h>

#ifdef WIN32
#include <win32/win_steam.h>
#include <universal/base64.h>
#else
#error Steam Auth for Arch
#endif

const dvar_t *cl_conXOffset;
const dvar_t *cl_hudDrawsBehindsUI;
const dvar_t *cl_showSend;
const dvar_t *input_invertPitch;
const dvar_t *cl_avidemo;
const dvar_t *cl_nodelta;
const dvar_t *cl_showServerCommands;
const dvar_t *motd;
const dvar_t *cl_connectTimeout;
const dvar_t *cl_sensitivity;
const dvar_t *cl_forceavidemo;
const dvar_t *cl_timeout;
const dvar_t *m_yaw;
const dvar_t *customclass[5];
const dvar_t *m_pitch;
const dvar_t *cl_activeAction;
const dvar_t *playlist;
const dvar_t *cl_debugMessageKey;
const dvar_t *systemlink;
const dvar_t *nextdemo;
const dvar_t *cl_connectionAttempts;
const dvar_t *onlinegame;
const dvar_t *cl_showMouseRate;
const dvar_t *m_forward;
const dvar_t *cl_packetdup;
const dvar_t *cl_mouseAccel;
const dvar_t *cl_maxpackets;
const dvar_t *cl_motdString;
const dvar_t *onlinegameandhost;
const dvar_t *cl_freezeDemo;
const dvar_t *cl_showTimeDelta;
const dvar_t *input_viewSensitivity;
const dvar_t *input_autoAim;
const dvar_t *cl_ingame;
const dvar_t *cl_inGameVideo;
const dvar_t *cl_noprint; 
const dvar_t *m_side;
const dvar_t *cl_profileTextY;
const dvar_t *cl_serverStatusResendTime;
const dvar_t *m_filter;
const dvar_t *cl_profileTextHeight;
const dvar_t *cl_shownuments;
const dvar_t *splitscreen;
const dvar_t *onlineunreankedgameandhost;
const dvar_t *cl_freelook;
const dvar_t *cl_shownet;

const dvar_t *cl_updateavailable;
const dvar_t *cl_updatefiles;
const dvar_t *cl_updateoldversion;
const dvar_t *cl_updateversion;
const dvar_t *cl_allowDownload;
const dvar_t *cl_wwwDownload;
const dvar_t *cl_talking;
const dvar_t *cl_hudDrawsBehindUI;
const dvar_t *cl_voice;
const dvar_t *name;

serverStatus_s cl_serverStatusList[16];
int32_t serverStatusCount;

const char *customClassDvars[6] =
{
  "customclass1",
  "customclass2",
  "customclass3",
  "customclass4",
  "customclass5",
  NULL
};

BOOL g_waitingForServer;
BOOL cl_serverLoadingMap;

//ping_t *cl_pinglist;
ping_t cl_pinglist[16];

bool cl_waitingOnServerToLoadMap[1];

int32_t cl_maxLocalClients;
int32_t old_com_frameTime;
uint32_t frame_msec;

clientConnection_t clientConnections[MAX_CLIENTS];
clientUIActive_t clientUIActives[MAX_CLIENTS];
clientActive_t clients[MAX_CLIENTS];

clientStatic_t cls;

int32_t lastUpdateKeyAuthTime;

char cl_cdkey[34] = {"                                "};

struct $03EB187DDD3425F4F7BCEA9E0EB47FBF // sizeof=0x2C
{                                       // ...
    char password[24];                  // ...
    netadr_t host;                      // ...
};
$03EB187DDD3425F4F7BCEA9E0EB47FBF rconGlob;

void __cdecl TRACK_cl_main()
{
    track_static_alloc_internal(clients, sizeof(clientActive_t) * MAX_CLIENTS /*1776604*/, "clients", 9);
    track_static_alloc_internal(clientConnections, sizeof(clientConnection_t) * MAX_CLIENTS /*398824*/, "clientConnections", 9);
    track_static_alloc_internal(clientUIActives, sizeof(clientUIActive_t) * MAX_CLIENTS, "clientUIActives", 9);
    track_static_alloc_internal(&cls, sizeof(clientStatic_t)/*3002480*/, "cls", 9);
}

int32_t autoupdateChecked;
void __cdecl CL_GetAutoUpdate()
{
    if (autoupdateChecked)
    {
        if (strlen(cl_updatefiles->current.string))
            Sys_OpenURL(cl_updatefiles->current.string, 1);
    }
}

char __cdecl CL_IsLocalClientActive(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            350,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum >= 0 && localClientNum < 1)",
            localClientNum);
    return 1;
}

int32_t __cdecl CL_LocalActiveIndexFromClientNum(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            498,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum >= 0 && localClientNum < 1)",
            localClientNum);
    return 0;
}

int32_t __cdecl CL_ControllerIndexFromClientNum(int32_t clientIndex)
{
    if (clientIndex)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            506,
            0,
            "%s\n\t(clientIndex) = %i",
            "((clientIndex >= 0) && (clientIndex < 1))",
            clientIndex);
    return clientIndex;
}

char __cdecl CL_AllLocalClientsDisconnected()
{
    int32_t client; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 551, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    if (!Sys_IsMainThread())
        return 1;
    if (UI_IsFullscreen(0))
        return 1;
    for (client = 0; client < 1; ++client)
    {
        if (clientUIActives[client].active && clientUIActives[client].connectionState >= CA_CONNECTING)
            return 0;
    }
    return 1;
}

char __cdecl CL_AnyLocalClientChallenging()
{
    int32_t clientIndex; // [esp+0h] [ebp-4h]

    for (clientIndex = 0; clientIndex < 1; ++clientIndex)
    {
        if (clientUIActives[clientIndex].active && clientUIActives[clientIndex].connectionState == CA_CHALLENGING)
            return 1;
    }
    return 0;
}

const char *__cdecl CL_GetUsernameForLocalClient()
{
    return name->current.string;
}

void __cdecl CL_AddReliableCommand(int32_t localClientNum, const char *cmd)
{
    clientConnection_t *clc; // [esp+0h] [ebp-8h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if (clc->reliableSequence - clc->reliableAcknowledge > 128)
        Com_Error(ERR_DROP, "EXE_ERR_CLIENT_CMD_OVERFLOW");
    MSG_WriteReliableCommandToBuffer(cmd, clc->reliableCommands[++clc->reliableSequence & 0x7F], 1024);
}

void __cdecl CL_ShutdownDevGui()
{
    CL_DestroyDevGui();
    DevGui_Shutdown();
    Cmd_RemoveCommand("devgui_dvar");
    Cmd_RemoveCommand("devgui_cmd");
    Cmd_RemoveCommand("devgui_open");
}

void __cdecl CL_ShutdownHunkUsers()
{
    int32_t client; // [esp+0h] [ebp-4h]

    Com_SyncThreads();
    if (cls.hunkUsersStarted)
    {
        for (client = 0; client < 1; ++client)
            CL_ShutdownCGame(client);
        Phys_Shutdown();
        if (cls.devGuiStarted)
        {
            CL_ShutdownDevGui();
            cls.devGuiStarted = 0;
        }
        CL_ShutdownUI();
        if (cls.uiStarted)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1330, 0, "%s", "!cls.uiStarted");
        cls.hunkUsersStarted = 0;
    }
}

void __cdecl CL_ShutdownAll()
{
    R_SyncRenderThread();
    CL_ShutdownHunkUsers();
    if (cls.rendererStarted)
    {
        CL_ShutdownRenderer(0);
        if (cls.rendererStarted)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1346, 0, "%s", "!cls.rendererStarted");
    }
    track_shutdown(3);
}

char __cdecl CL_AnyLocalClientsRunning()
{
    int32_t localClientNum; // [esp+0h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        if (clientUIActives[localClientNum].isRunning)
            return 1;
    }
    return 0;
}

void __cdecl CL_MapLoading(const char *mapname)
{
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-10h]
    int32_t localClientNum; // [esp+10h] [ebp-Ch]
    int32_t localClientNuma; // [esp+10h] [ebp-Ch]
    netsrc_t localClientNumb; // [esp+10h] [ebp-Ch]
    clientConnection_t *clc; // [esp+14h] [ebp-8h]
    clientConnection_t *clca; // [esp+14h] [ebp-8h]

    if (CL_AnyLocalClientsRunning())
    {
        g_waitingForServer = 0;
        for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        {
            Con_Close(localClientNum);
            clientUIActives[localClientNum].keyCatchers = 0;
            clientUIActives[localClientNum].displayHUDWithKeycatchUI = 0;
        }
        LiveStorage_UploadStats();
        UI_CloseAllMenus(0);
        cl_serverLoadingMap = 1;
        if (!com_sv_running->current.enabled)
            Cbuf_ExecuteBuffer(0, 0, "selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
        if (clientUIActives[0].connectionState >= 5 && !I_stricmp(cls.servername, "localhost"))
        {
            memset((uint8_t *)cls.updateInfoString, 0, sizeof(cls.updateInfoString));
            for (localClientNuma = 0; localClientNuma < 1; ++localClientNuma)
            {
                if (CL_IsLocalClientActive(localClientNuma))
                {
                    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNuma);
                    clc = CL_GetLocalClientConnection(localClientNuma);
                    clientUIActives[localClientNuma].connectionState = CA_CONNECTED;
                    memset((uint8_t *)clc->serverMessage, 0, sizeof(clc->serverMessage));
                    memset((uint8_t *)&LocalClientGlobals->gameState, 0, sizeof(LocalClientGlobals->gameState));
                    clc->lastPacketSentTime = -9999;
                    if (!*mapname)
                        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1463, 0, "%s", "mapname[0]");
                    cl_waitingOnServerToLoadMap[localClientNuma] = 0;
                }
            }
        }
        else
        {
            Dvar_SetString((dvar_s *)nextmap, (char *)"");
            I_strncpyz(cls.servername, "localhost", 256);
            for (localClientNumb = NS_CLIENT1; localClientNumb < NS_SERVER; ++localClientNumb)
            {
                if (CL_IsLocalClientActive(localClientNumb))
                {
                    CL_Disconnect(localClientNumb);
                    KISAK_NULLSUB();
                    UI_CloseAll(localClientNumb);
                    clientUIActives[localClientNumb].connectionState = CA_CHALLENGING;
                    clca = CL_GetLocalClientConnection(localClientNumb);
                    clca->connectTime = -3000;
                    clca->qport = localClientNumb + g_qport;
                    NET_StringToAdr(cls.servername, &clca->serverAddress);
                    CL_CheckForResend(localClientNumb);
                    if (!*mapname)
                        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1498, 0, "%s", "mapname[0]");
                    cl_waitingOnServerToLoadMap[localClientNumb] = 0;
                }
            }
        }
        SND_FadeAllSounds(0.0, 0);
    }
}

void __cdecl CL_ResetSkeletonCache(int32_t localClientNum)
{
    clientActive_t *v1; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1512, 0, "%s", "Sys_IsMainThread()");
    if (!clients)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1513, 0, "%s", "clients");
    if (localClientNum)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            1514,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    v1 = &clients[localClientNum];
    if (!++v1->skelTimeStamp)
        ++v1->skelTimeStamp;
    v1->skelMemoryStart = (char *)((uint32_t)&v1->skelMemory[15] & 0xFFFFFFF0);
    v1->skelMemPos = 0;
}

void __cdecl CL_ClearState(int32_t localClientNum)
{
    clientActive_t *dst; // [esp+0h] [ebp-4h]

    if (localClientNum < 1)
    {
        dst = CL_GetLocalClientGlobals(localClientNum);
        memset((uint8_t *)dst, 0, sizeof(clientActive_t));
    }
    Com_ClientDObjClearAllSkel();
}

void __cdecl CL_ClearStaticDownload()
{
    if (cls.wwwDlDisconnected)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1540, 0, "%s", "!cls.wwwDlDisconnected");
    cls.downloadRestart = 0;
    cls.downloadTempName[0] = 0;
    cls.downloadName[0] = 0;
    cls.originalDownloadName[0] = 0;
    cls.wwwDlInProgress = 0;
    cls.wwwDlDisconnected = 0;
    cls.downloadFlags = 0;
    DL_CancelDownload();
}

void __cdecl CL_Disconnect(int32_t localClientNum)
{
    int32_t v1; // eax
    connstate_t connstate; // [esp+4h] [ebp-Ch]
    clientConnection_t *clc; // [esp+8h] [ebp-8h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1596, 0, "%s", "Sys_IsMainThread()");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].isRunning)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        connstate = clientUIActives[0].connectionState;
        if (clientUIActives[0].connectionState < CA_CONNECTED)
            clc = 0;
        else
            clc = CL_GetLocalClientConnection(localClientNum);
        if (connstate >= CA_CONNECTED && clc->demorecording)
        {
            v1 = CL_ControllerIndexFromClientNum(localClientNum);
            Cmd_ExecuteSingleCommand(localClientNum, v1, (char*)"stoprecord");
        }
        if (!cls.wwwDlDisconnected)
        {
            if (cls.download)
            {
                FS_FCloseFile(cls.download);
                cls.download = 0;
            }
            cls.downloadName[0] = 0;
            cls.downloadTempName[0] = 0;
            legacyHacks.cl_downloadName[0] = 0;
        }
        legacyHacks.cl_downloadName[0] = 0;
        if (connstate >= CA_CONNECTED && clc->demofile)
        {
            FS_FCloseFile(clc->demofile);
            clc->demofile = 0;
            clc->demoplaying = 0;
            clc->demorecording = 0;
        }
        SCR_StopCinematic(localClientNum);
        if (connstate >= CA_CONNECTED && clc->reliableSequence - clc->reliableAcknowledge <= 128)
        {
            CL_AddReliableCommand(localClientNum, "disconnect");
            CL_WritePacket(localClientNum);
            CL_WritePacket(localClientNum);
            CL_WritePacket(localClientNum);
        }
        CL_ClearState(localClientNum);
        Ragdoll_Shutdown();
        CL_ClearMutedList();
        if (connstate >= CA_CONNECTED)
            memset((uint8_t *)clc, 0, sizeof(clientConnection_t));
        clientUIActives[localClientNum].connectionState = CA_DISCONNECTED;
        if (!cls.wwwDlDisconnected)
            CL_ClearStaticDownload();
        DynEntCl_Shutdown(localClientNum);
        SND_DisconnectListener(localClientNum);
        if (connstate >= CA_CONNECTING)
            clientUIActives[0].keyCatchers &= 1u;
        KISAK_NULLSUB();
        // LWSS ADD
        Steam_CancelClientTicket();
        // LWSS END
        if (CL_AllLocalClientsDisconnected())
        {
            autoupdateStarted = 0;
            autoupdateFilename[0] = 0;
            Dvar_SetBool(sv_disableClientConsole, 0);
            cl_connectedToPureServer = 0;
            fs_checksumFeed = 0;
            LiveStorage_UploadStats();
        }
    }
}

void __cdecl CL_ForwardCommandToServer(int32_t localClientNum, const char *string)
{
    const char *cmd; // [esp+8h] [ebp-4h]

    cmd = Cmd_Argv(0);
    if (*cmd != 45)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (clientUIActives[0].connectionState < 5 || *cmd == 43 || CL_GetLocalClientConnection(localClientNum)->demoplaying)
        {
            Com_Printf(14, "Unknown command \"%s\"\n", cmd);
        }
        else if (Cmd_Argc() <= 1)
        {
            CL_AddReliableCommand(localClientNum, cmd);
        }
        else
        {
            CL_AddReliableCommand(localClientNum, string);
        }
    }
}

void __cdecl CL_RequestAuthorization(netsrc_t localClientNum)
{
    //__int16 v1; // ax
    //const char *v2; // eax
    //int32_t j; // [esp+10h] [ebp-78h]
    //int32_t l; // [esp+14h] [ebp-74h]
    //char md5Str[36]{ 0 }; // [esp+18h] [ebp-70h] BYREF
    //const dvar_s *v6; // [esp+3Ch] [ebp-4Ch]
    //char nums[64]; // [esp+40h] [ebp-48h] BYREF
    //int32_t i; // [esp+84h] [ebp-4h]
    //
    //lastUpdateKeyAuthTime = cls.realtime;
    // KISAKKEY
    //if (!CL_CDKeyValidate(cl_cdkey, cl_cdkeychecksum))
    //{
    //    Com_Error(ERR_DROP, "EXE_ERR_INVALID_CD_KEY");
    //    return;
    //}
    //if (!cls.authorizeServer.port)
    //{
    //    Com_Printf(14, "Resolving %s\n", com_authServerName->current.string);
    //    if (!NET_StringToAdr((char *)com_authServerName->current.integer, &cls.authorizeServer))
    //    {
    //        Com_Printf(14, "Couldn't resolve address\n");
    //        return;
    //    }
    //    cls.authorizeServer.port = BigShort(com_authPort->current.integer);
    //    v1 = BigShort(cls.authorizeServer.port);
    //    Com_Printf(
    //        14,
    //        "%s resolved to %i.%i.%i.%i:%i\n",
    //        com_authServerName->current.string,
    //        cls.authorizeServer.ip[0],
    //        cls.authorizeServer.ip[1],
    //        cls.authorizeServer.ip[2],
    //        cls.authorizeServer.ip[3],
    //        v1);
    //}
    //if (cls.authorizeServer.type != NA_BAD)
    //{
    //    if (Dvar_GetBool("fs_restrict"))
    //    {
    //        I_strncpyz(nums, "demo", 64);
    //    }
    //    else
    //    {
    //        j = 0;
    //        l = strlen(cl_cdkey);
    //        if (l > 32)
    //            l = 32;
    //        for (i = 0; i < l; ++i)
    //        {
    //            if (cl_cdkey[i] >= 48 && cl_cdkey[i] <= 57
    //                || cl_cdkey[i] >= 97 && cl_cdkey[i] <= 122
    //                || cl_cdkey[i] >= 65 && cl_cdkey[i] <= 90)
    //            {
    //                nums[j++] = cl_cdkey[i];
    //            }
    //        }
    //        nums[j] = 0;
    //    }
    //    v6 = Dvar_RegisterBool("cl_anonymous", 0, 0x1Bu, "Allow anonymous log in");
    //    //CL_BuildMd5StrFromCDKey(md5Str);
    //    v2 = va("getKeyAuthorize %i %s PB %s", v6->current.color[0], nums, md5Str);
    //    NET_OutOfBandPrint(localClientNum, cls.authorizeServer, v2);
    //}
}

void __cdecl CL_ForwardToServer_f()
{
    char command[1028]; // [esp+Ch] [ebp-408h] BYREF

    if (CL_GetLocalClientConnection(0)->demoplaying || clientUIActives[0].connectionState != 9)
    {
        Com_Printf(0, "Not connected to a server.\n");
    }
    else if (Cmd_Argc() > 1)
    {
        Cmd_ArgsBuffer(1, command, 1024);
        CL_AddReliableCommand(0, command);
    }
}

void __cdecl CL_Setenv_f()
{
    char *v0; // eax
    char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *env; // [esp+0h] [ebp-414h]
    char buffer[1028]; // [esp+4h] [ebp-410h] BYREF
    int32_t i; // [esp+40Ch] [ebp-8h]
    int32_t argc; // [esp+410h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc <= 2)
    {
        if (argc == 2)
        {
            v2 = Cmd_Argv(1);
            env = getenv(v2);
            if (env)
            {
                v3 = Cmd_Argv(1);
                Com_Printf(0, "%s=%s\n", v3, env);
            }
            else
            {
                v4 = Cmd_Argv(1);
                Com_Printf(0, "%s undefined\n", v4);
            }
        }
    }
    else
    {
        v0 = (char *)Cmd_Argv(1);
        I_strncpyz(buffer, v0, 1024);
        I_strncat(buffer, 1024, "=");
        for (i = 2; i < argc; ++i)
        {
            v1 = (char *)Cmd_Argv(i);
            I_strncat(buffer, 1024, v1);
            I_strncat(buffer, 1024, " ");
        }
        _putenv(buffer);
    }
}

void __cdecl CL_DisconnectLocalClient(int32_t localClientNum)
{
    bool v1; // [esp+0h] [ebp-Ch]

    SCR_StopCinematic(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v1 = clientUIActives[0].connectionState > (uint32_t)CA_LOGO;
    CL_Disconnect(localClientNum);
    if (v1)
    {
        if (CL_AllLocalClientsDisconnected())
            Com_Error(ERR_DISCONNECT, "PLATFORM_DISCONNECTED_FROM_SERVER");
    }
}

void __cdecl CL_Reconnect_f()
{
    const char *v0; // eax

    if (strlen(cls.servername) && strcmp(cls.servername, "localhost"))
    {
        v0 = va("connect %s reconnect\n", cls.servername);
        Cbuf_AddText(0, v0);
    }
    else
    {
        Com_Printf(0, "Can't reconnect to localhost.\n");
    }
}

void __cdecl CL_ResetPureClientAtServer(int32_t localClientNum)
{
    const char *v1; // eax

    v1 = va("vdr");
    CL_AddReliableCommand(localClientNum, v1);
}

void __cdecl CL_SendPureChecksums(int32_t localClientNum)
{
    char *pChecksums; // [esp+0h] [ebp-40Ch]
    char cMsg[1028]; // [esp+4h] [ebp-408h] BYREF

    pChecksums = FS_ReferencedIwdPureChecksums();
    Com_sprintf(cMsg, 0x400u, "Va ");
    I_strncat(cMsg, 1024, pChecksums);
    cMsg[0] += 13;
    cMsg[1] += 15;
    CL_AddReliableCommand(localClientNum, cMsg);
}

void __cdecl CL_Vid_Restart_f()
{
    uint8_t *v0; // eax
    const char *v1; // eax
    BOOL v2; // [esp+0h] [ebp-D4h]
    char *info; // [esp+4h] [ebp-D0h]
    XZoneInfo zoneInfo[1]; // [esp+8h] [ebp-CCh] BYREF
    char zoneName[64]; // [esp+14h] [ebp-C0h] BYREF
    char mapname[64]; // [esp+54h] [ebp-80h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+98h] [ebp-3Ch]
    clientUIActive_t *clientUIActive; // [esp+9Ch] [ebp-38h]
    connstate_t connstate; // [esp+A0h] [ebp-34h]
    int32_t localClientNum; // [esp+A4h] [ebp-30h]
    clientConnection_t *clc; // [esp+A8h] [ebp-2Ch]
    int32_t clientStateBytes; // [esp+ACh] [ebp-28h]
    MemoryFile memFile; // [esp+B0h] [ebp-24h] BYREF
    uint8_t *clientStateBuf; // [esp+CCh] [ebp-8h]
    int32_t fileSystemRestarted; // [esp+D0h] [ebp-4h]

    if (com_sv_running->current.enabled)
    {
        Com_Printf(0, "Listen server cannot video restart.\n");
    }
    else
    {
        localClientNum = 0;
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        clientUIActive = clientUIActives;
        clc = CL_GetLocalClientConnection(0);
        connstate = clientUIActives[0].connectionState;
        clientStateBuf = 0;
        clientStateBytes = 0;
        if (clientUIActives[0].cgameInitialized)
        {
            v0 = (uint8_t *)Z_VirtualAlloc(0xA00000, "demo", 0);
            MemFile_InitForWriting(&memFile, 0xA00000, v0, 1, 0);
            CL_ArchiveClientState(localClientNum, &memFile);
            MemFile_StartSegment(&memFile, -1);
            clientStateBytes = memFile.bytesUsed;
            clientStateBuf = (uint8_t *)Z_VirtualAlloc(memFile.bytesUsed, "CL_Vid_Restart_f", 10);
            memcpy(clientStateBuf, memFile.buffer, clientStateBytes);
            Z_VirtualFree(memFile.buffer);
        }
        com_expectedHunkUsage = 0;
        g_waitingForServer = 0;
        SND_StopSounds(SND_KEEP_REVERB);
        CL_ShutdownHunkUsers();
        CL_ShutdownRef();
        cls.rendererStarted = 0;
        CL_ResetPureClientAtServer(localClientNum);
        Com_Restart();
        Dvar_RegisterInt("loc_language", 0, (DvarLimits)0xE00000000LL, DVAR_ARCHIVE | DVAR_LATCH, "The current language locale");
        Dvar_RegisterBool("loc_translate", true, DVAR_LATCH, "Turn on string translation");
        Dvar_RegisterBool("fs_ignoreLocalized", false, DVAR_LATCH | DVAR_CHEAT, "Ignore localized assets");
        v2 = FS_ConditionalRestart(localClientNum, clc->checksumFeed) || cls.gameDirChanged;
        fileSystemRestarted = v2;
        SEH_UpdateLanguageInfo();
        Dvar_SetInt(cl_paused, 0);
        CL_InitRef();
        CL_InitRenderer();
        CL_StartHunkUsers();
        if (connstate > CA_CONNECTED)
        {
            info = CL_GetConfigString(localClientNum, 0);
            v1 = Info_ValueForKey(info, "mapname");
            I_strncpyz(mapname, v1, 64);
            DB_ResetZoneSize(0);
            Com_sprintf(zoneName, 0x40u, "%s_load", mapname);
            zoneInfo[0].name = zoneName;
            zoneInfo[0].allocFlags = 32;
            zoneInfo[0].freeFlags = 96;
            DB_LoadXAssets(zoneInfo, 1u, 0);
            DB_SyncXAssets();
            DB_UpdateDebugZone();
            CL_InitCGame(localClientNum);
            CL_SendPureChecksums(localClientNum);
        }
        if (fileSystemRestarted && !com_dedicated->current.integer)
            LiveStorage_ReadStats();
        if (clientStateBuf)
        {
            if (clientUIActive->cgameInitialized)
            {
                MemFile_InitForReading(&memFile, clientStateBytes, clientStateBuf, 0);
                CL_ArchiveClientState(localClientNum, &memFile);
                MemFile_MoveToSegment(&memFile, -1);
            }
            Z_VirtualFree(clientStateBuf);
        }
    }
}

void __cdecl CL_Snd_Restart_f()
{
    uint8_t *v0; // eax
    uint8_t *soundStateBuf; // [esp+0h] [ebp-90h]
    MemoryFile memFile; // [esp+4h] [ebp-8Ch] BYREF
    snd_listener listeners[2]; // [esp+20h] [ebp-70h] BYREF

    if (com_sv_running->current.enabled)
    {
        Com_Printf(0, "Listen server cannot sound restart.\n");
    }
    else
    {
        Hunk_CheckTempMemoryClear();
        v0 = (uint8_t *)Z_VirtualAlloc(0xA00000, "demo", 0);
        MemFile_InitForWriting(&memFile, 0xA00000, v0, 1, 0);
        SND_Save(&memFile);
        MemFile_StartSegment(&memFile, -1);
        soundStateBuf = (uint8_t *)Z_VirtualAlloc(memFile.bytesUsed, "CL_Snd_Restart_f", 10);
        memcpy(soundStateBuf, memFile.buffer, memFile.bytesUsed);
        Z_VirtualFree(memFile.buffer);
        SND_SaveListeners(listeners);
        SND_Shutdown();
        SND_InitDriver();
        SND_Init();
        SND_RestoreListeners(listeners);
        CL_Vid_Restart_f();
        MemFile_InitForReading(&memFile, memFile.bytesUsed, soundStateBuf, 0);
        SND_Restore(&memFile);
        MemFile_MoveToSegment(&memFile, -1);
        Z_VirtualFree(soundStateBuf);
    }
}

void __cdecl CL_Configstrings_f()
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-10h]
    int32_t ofs; // [esp+4h] [ebp-Ch]
    int32_t i; // [esp+Ch] [ebp-4h]

    if (clientUIActives[0].connectionState == 9)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        for (i = 0; i < 2442; ++i)
        {
            ofs = LocalClientGlobals->gameState.stringOffsets[i];
            if (ofs)
                Com_Printf(0, "%4i: %s\n", i, &LocalClientGlobals->gameState.stringData[ofs]);
        }
    }
    else
    {
        Com_Printf(0, "Not connected to a server.\n");
    }
}

void __cdecl CL_Clientinfo_f()
{
    char *v0; // eax

    Com_Printf(0, "--------- Client Information ---------\n");
    Com_Printf(0, "state: %i\n", clientUIActives[0].connectionState);
    Com_Printf(0, "Server: %s\n", cls.servername);
    Com_Printf(0, "User info settings:\n");
    v0 = Dvar_InfoString(0, 2);
    Info_Print(v0);
    Com_Printf(0, "--------------------------------------\n");
}

bool __cdecl CL_WasMapAlreadyLoaded()
{
    return com_sv_running->current.enabled;
}

void __cdecl LoadMapLoadscreen(const char *mapname)
{
    XZoneInfo zoneInfo[1]; // [esp+0h] [ebp-54h] BYREF
    char zoneName[68]; // [esp+Ch] [ebp-48h] BYREF

    DB_ResetZoneSize(0);
    Com_sprintf(zoneName, 0x40u, "%s_load", mapname);
    zoneInfo[0].name = zoneName;
    zoneInfo[0].allocFlags = 32;
    zoneInfo[0].freeFlags = 96;
    DB_LoadXAssets(zoneInfo, 1u, 0);
    DB_SyncXAssets();
    DB_UpdateDebugZone();
}

void __cdecl CL_DownloadsComplete(int32_t localClientNum)
{
    char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    char *info; // [esp+10h] [ebp-98h]
    char *fn; // [esp+14h] [ebp-94h]
    char gametype[68]; // [esp+18h] [ebp-90h] BYREF
    clientConnection_t *clc; // [esp+5Ch] [ebp-4Ch]
    char mapname[68]; // [esp+60h] [ebp-48h] BYREF

    clc = CL_GetLocalClientConnection(localClientNum);
    if (autoupdateStarted)
    {
        if (autoupdateFilename)
        {
            if (strlen(autoupdateFilename) > 4)
            {
                v1 = FS_ShiftStr("ni]Zm^l", 7);
                fn = va("%s/%s", v1, autoupdateFilename);
                Sys_QuitAndStartProcess(fn, 0);
            }
        }
        autoupdateStarted = 0;
        CL_Disconnect(localClientNum);
        return;
    }
    if (cls.downloadRestart)
    {
        if (com_sv_running->current.enabled)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2571, 0, "%s", "!com_sv_running->current.enabled");
        cls.downloadRestart = 0;
        FS_Restart(localClientNum, clc->checksumFeed);
        CL_Vid_Restart_f();
        if (!cls.wwwDlDisconnected)
            CL_AddReliableCommand(localClientNum, "donedl");
        cls.wwwDlDisconnected = 0;
        CL_ClearStaticDownload();
        return;
    }
    Com_SyncThreads();
    if (cls.wwwDlDisconnected)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2604, 0, "%s", "!cls.wwwDlDisconnected");
    clientUIActives[localClientNum].connectionState = CA_LOADING;
    Com_Printf(14, "Setting state to CA_LOADING in CL_DownloadsComplete\n");
    if (!CL_WasMapAlreadyLoaded())
    {
        info = CL_GetConfigString(localClientNum, 0);
        v2 = Info_ValueForKey(info, "mapname");
        I_strncpyz(mapname, v2, 64);
        v3 = Info_ValueForKey(info, "g_gametype");
        I_strncpyz(gametype, v3, 64);
        if (cls.gameDirChanged)
            CL_Vid_Restart_f();
        if (!g_waitingForServer)
            LoadMapLoadscreen(mapname);
        UI_SetMap(mapname, gametype);
        SCR_UpdateScreen();
        CL_ShutdownAll();
        Com_Restart();
        if (cls.hunkUsersStarted)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2640, 0, "%s", "!cls.hunkUsersStarted");
        CL_InitRenderer();
        CL_StartHunkUsers();
        SCR_UpdateScreen();
    LABEL_25:
        Dvar_SetInt(cl_paused, 1);
        CL_InitCGame(localClientNum);
        CL_SendPureChecksums(localClientNum);
        CL_WritePacket(localClientNum);
        CL_WritePacket(localClientNum);
        CL_WritePacket(localClientNum);
        return;
    }
    if (!cls.hunkUsersStarted)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2649, 0, "%s", "cls.hunkUsersStarted");
    if (!CL_IsCgameInitialized(localClientNum))
        goto LABEL_25;
}

uint8_t msgBuffer[2048];
void __cdecl CL_CheckForResend(netsrc_t localClientNum)
{
    int32_t v1; // eax
    const char *v2; // eax
    char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int32_t v7; // [esp+0h] [ebp-1188h]
    char md5Str[36]; // [esp+2Ch] [ebp-115Ch] BYREF
    uint8_t dst[1244]; // [esp+50h] [ebp-1138h] BYREF
    connstate_t connectionState; // [esp+52Ch] [ebp-C5Ch]
    char dest[1028]; // [esp+530h] [ebp-C58h] BYREF
    int32_t pktlen; // [esp+934h] [ebp-854h] BYREF
    uint8_t src[1028]; // [esp+938h] [ebp-850h] BYREF
    uint32_t count; // [esp+D3Ch] [ebp-44Ch]
    msg_t buf; // [esp+D40h] [ebp-448h] BYREF
    int32_t length; // [esp+D68h] [ebp-420h]
    void *data; // [esp+D6Ch] [ebp-41Ch]
    clientConnection_t *clc; // [esp+D70h] [ebp-418h]
    int32_t c; // [esp+D74h] [ebp-414h]
    char pkt[1036]; // [esp+D78h] [ebp-410h] BYREF

    unsigned char *pSteamClientTicket = NULL;
    uint32 steamClientTicketSize = 0;
    char steamIDbuf[25];
    unsigned char steamTicketBase64[2048]{ 0 };

#ifdef _DEBUG
    unsigned char steamTicketDecodeBuf[1024]{ 0 };
#endif

    iassert(localClientNum == 0);
    connectionState = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState == CA_CONNECTING
        || connectionState == CA_CHALLENGING
        || connectionState == CA_SENDINGSTATS)
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if (connectionState == CA_SENDINGSTATS)
        {
            if (cls.realtime - clc->lastPacketSentTime < 100)
                return;
        }
        else if (cls.realtime - clc->connectTime < 3000)
        {
            return;
        }
        if (!clc->demoplaying)
        {
            clc->connectTime = cls.realtime;
            ++clc->connectPacketCount;
            switch (connectionState)
            {
            case CA_CONNECTING:
                if (net_lanauthorize->current.enabled || !Sys_IsLANAddress(clc->serverAddress))
                    CL_RequestAuthorization(localClientNum);
                strcpy(pkt, "getchallenge");
                pktlen = &pkt[strlen(pkt) + 1] - &pkt[1];
                //PbClientConnecting(1, pkt, &pktlen);
                //CL_BuildMd5StrFromCDKey(md5Str);
                iassert(Steam_GetRawClientTicket(&pSteamClientTicket, &steamClientTicketSize));

                b64_encode(pSteamClientTicket, steamClientTicketSize, steamTicketBase64);
#ifdef _DEBUG
                iassert(b64_decode(steamTicketBase64, strlen((char *)steamTicketBase64), steamTicketDecodeBuf) == steamClientTicketSize);
#endif
                //v2 = va("getchallenge 0 \"%s\"", md5Str);
                v2 = va("getchallenge 0 \"%s\" \"%llu\"", steamTicketBase64, Steam_GetClientSteamID64());
                NET_OutOfBandPrint(localClientNum, clc->serverAddress, v2);
                break;
            case CA_CHALLENGING:
                v3 = Dvar_InfoString(localClientNum, 2);
                I_strncpyz(dest, v3, 1024);
                v4 = va("%i", 1);
                Info_SetValueForKey(dest, "protocol", v4);
                v5 = va("%i", clc->challenge);
                Info_SetValueForKey(dest, "challenge", v5);
                if (!clc->qport)
                    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2980, 0, "%s", "clc->qport != 0");
                v6 = va("%i", clc->qport);
                Info_SetValueForKey(dest, "qport", v6);
                qmemcpy(src, "connect \"", 9);
                count = &dest[strlen(dest) + 1] - &dest[1];
                memcpy(&src[9], dest, count);
                src[count + 9] = 34;
                src[count + 10] = 0;
                pktlen = count + 10;
                memcpy(pkt, src, count + 10);
                //PbClientConnecting(2, pkt, &pktlen);
                NET_OutOfBandData(localClientNum, clc->serverAddress, src, count + 10);
                dvar_modifiedFlags &= ~2u;
                break;
            case CA_SENDINGSTATS:
                MSG_Init(&buf, msgBuffer, 2048);
                MSG_WriteString(&buf, "stats");
                c = CL_HighestPriorityStatPacket(clc);
                if (c > 6)
                    MyAssertHandler(
                        ".\\client_mp\\cl_main_mp.cpp",
                        3017,
                        0,
                        "%s\n\t(packetToSend) = %i",
                        "(packetToSend >= 0 && packetToSend < 7)",
                        c);
                if (LiveStorage_DoWeHaveStats())
                {
                    data = (char*)LiveStorage_GetStatBuffer() + 1240 * c;
                }
                else
                {
                    if (onlinegame->current.enabled)
                        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 3029, 0, "%s", "!onlinegame->current.enabled");
                    memset(dst, 0, 1240u);
                    data = dst;
                }
                MSG_WriteShort(&buf, clc->qport);
                MSG_WriteByte(&buf, c);
                if (0x2000 - 1240 * c > 1240)
                    v7 = 1240;
                else
                    v7 = 0x2000 - 1240 * c;
                length = v7;
                MSG_WriteData(&buf, (unsigned char*)data, v7);
                clc->statPacketSendTime[c] = cls.realtime;
                clc->lastPacketSentTime = cls.realtime;
                NET_OutOfBandData(localClientNum, clc->serverAddress, buf.data, buf.cursize);
                break;
            default:
                Com_Error(ERR_FATAL, "CL_CheckForResend: bad connstate");
                break;
            }
        }
    }
}

int32_t __cdecl CL_HighestPriorityStatPacket(clientConnection_t *clc)
{
    int32_t packet; // [esp+0h] [ebp-Ch]
    int32_t oldestPacketTime; // [esp+4h] [ebp-8h]
    int32_t oldestPacket; // [esp+8h] [ebp-4h]

    oldestPacketTime = cls.realtime;
    oldestPacket = -1;
    for (packet = 0; packet < 7; ++packet)
    {
        if (((1 << packet) & clc->statPacketsToSend) != 0)
        {
            if (!clc->statPacketSendTime[packet])
                return packet;
            if (clc->statPacketSendTime[packet] < oldestPacketTime)
            {
                oldestPacketTime = clc->statPacketSendTime[packet];
                oldestPacket = packet;
            }
        }
    }
    if (oldestPacket < 0)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2863, 0, "%s", "oldestPacket >= 0");
    return oldestPacket;
}

void __cdecl CL_DisconnectError(char *message)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // [esp-4h] [ebp-4h]

    if (!message)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 3157, 0, "%s", "message");
    v3 = SEH_SafeTranslateString(message);
    v1 = SEH_SafeTranslateString((char*)"EXE_SERVERDISCONNECTREASON");
    v2 = UI_ReplaceConversionString(v1, v3);
    Com_Error(ERR_SERVERDISCONNECT, v2);
}

char __cdecl CL_ConnectionlessPacket(netsrc_t localClientNum, netadr_t from, msg_t *msg, int32_t time)
{
    const char *v5; // eax
    char success; // [esp+3h] [ebp-9h]
    char *s; // [esp+4h] [ebp-8h]

    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    CL_Netchan_AddOOBProfilePacket(localClientNum, msg->cursize);
    //if (!strnicmp((const char *)msg->data + 4, "PB_", 3u))
    if (!_strnicmp((const char *)msg->data + 4, "PB_", 3u))
    {
        // LWSS: Remove Punkbuster crap
        //if (msg->data[7] == 83 || msg->data[7] == 50 || msg->data[7] == 73)
        //    PbSvAddEvent(13, -1, msg->cursize - 4, (char *)msg->data + 4);
        //else
        //    PbClAddEvent(13, msg->cursize - 4, (char *)msg->data + 4);
        return 1;
    }
    else
    {
        s = MSG_ReadStringLine(msg);
        if (showpackets->current.integer)
        {
            v5 = NET_AdrToString(from);
            Com_Printf(16, "recv: %s->'%s'\n", v5, s);
        }
        Cmd_TokenizeString(s);
        success = CL_DispatchConnectionlessPacket(localClientNum, from, msg, time);
        Cmd_EndTokenizedString();
        return success;
    }
}

void __cdecl CL_UpdateInfoPacket(netadr_t from)
{
    __int16 v1; // ax
    __int16 v2; // ax
    const char *v3; // eax
    int32_t v4; // eax
    char *v5; // eax
    char *v6; // eax

    if (cls.autoupdateServer.type == NA_BAD)
    {
        Com_DPrintf(14, "CL_UpdateInfoPacket:  Auto-Updater has bad address\n");
    }
    else
    {
        v1 = BigShort(cls.autoupdateServer.port);
        Com_DPrintf(
            14,
            "Auto-Updater resolved to %i.%i.%i.%i:%i\n",
            cls.autoupdateServer.ip[0],
            cls.autoupdateServer.ip[1],
            cls.autoupdateServer.ip[2],
            cls.autoupdateServer.ip[3],
            v1);
        if (NET_CompareAdr(from, cls.autoupdateServer))
        {
            v3 = Cmd_Argv(1);
            v4 = atoi(v3);
            Dvar_SetBool(cl_updateavailable, v4 != 0);
            if (cl_updateavailable->current.enabled)
            {
                v5 = (char *)Cmd_Argv(2);
                Dvar_SetString((dvar_s *)cl_updatefiles, v5);
                v6 = (char *)Cmd_Argv(3);
                Dvar_SetString((dvar_s *)cl_updateversion, v6);
                Dvar_SetString((dvar_s *)cl_updateoldversion, (char*)"1.0");
            }
        }
        else
        {
            v2 = BigShort(from.port);
            Com_DPrintf(
                14,
                "CL_UpdateInfoPacket:  Received packet from %i.%i.%i.%i:%i\n",
                from.ip[0],
                from.ip[1],
                from.ip[2],
                from.ip[3],
                v2);
        }
    }
}

void __cdecl CL_InitServerInfo(serverInfo_t *server, netadr_t adr)
{
    server->adr = adr;
    server->clients = 0;
    server->hostName[0] = 0;
    server->mapName[0] = 0;
    server->maxClients = 0;
    server->maxPing = 0;
    server->minPing = 0;
    server->ping = -1;
    server->game[0] = 0;
    server->gameType[0] = 0;
    server->netType = 0;
    server->allowAnonymous = 0;
    server->dirty = 1;
    server->requestCount = 0;
}

int32_t __cdecl CL_FindServerInfo(netadr_t adr)
{
    int32_t cmp; // [esp+0h] [ebp-14h]
    int32_t low; // [esp+4h] [ebp-10h]
    int32_t i; // [esp+8h] [ebp-Ch]
    int32_t ia; // [esp+8h] [ebp-Ch]
    int32_t high; // [esp+Ch] [ebp-8h]

    low = 0;
    high = cls.numglobalservers;
    while (1)
    {
        while (1)
        {
            if (low >= high)
                return 0;
            i = (high + low) / 2;
            cmp = NET_CompareAdrSigned(&adr, &cls.globalServers[i].adr);
            if (cmp >= 0)
                break;
            high = (high + low) / 2;
        }
        if (cmp <= 0)
            break;
        low = i + 1;
    }
    do
        --i;
    while (i >= 0 && !NET_CompareAdrSigned(&adr, &cls.globalServers[i].adr));
    ia = i + 1;
    do
        CL_InitServerInfo(&cls.globalServers[ia++], adr);
    while (ia < cls.numglobalservers && !NET_CompareAdrSigned(&adr, &cls.globalServers[ia].adr));
    return 1;
}

int32_t __cdecl CL_CompareAdrSigned(netadr_t *a, netadr_t *b)
{
    return NET_CompareAdrSigned(a, b);
}

void __cdecl CL_SortGlobalServers()
{
    qsort(
        cls.globalServers,
        cls.numglobalservers,
        0x94u,
        (int(__cdecl *)(const void *, const void *))CL_CompareAdrSigned);
}

void __cdecl CL_ServersResponsePacket(netadr_t from, msg_t *msg)
{
    int32_t v2; // eax
    netadr_t v3; // [esp-14h] [ebp-64Ch]
    int32_t numservers; // [esp+8h] [ebp-630h]
    uint8_t *buffend; // [esp+Ch] [ebp-62Ch]
    serverAddress_t addresses[256]; // [esp+10h] [ebp-628h] BYREF
    uint8_t *buffptr; // [esp+610h] [ebp-28h]
    int32_t i; // [esp+614h] [ebp-24h]
    serverInfo_t *server; // [esp+618h] [ebp-20h]
    int32_t count; // [esp+61Ch] [ebp-1Ch]
    netadr_t adr; // [esp+620h] [ebp-18h]

    Com_Printf(14, "CL_ServersResponsePacket\n");
    cls.waitglobalserverresponse = 0;
    numservers = 0;
    buffptr = msg->data;
    buffend = &buffptr[msg->cursize];
    do
    {
        if (buffptr + 1 >= buffend)
            break;
        do
            v2 = *buffptr++;
        while (v2 != 92 && buffptr < buffend);
        if (buffptr >= buffend - 6)
            break;
        addresses[numservers].ip[0] = *buffptr++;
        addresses[numservers].ip[1] = *buffptr++;
        addresses[numservers].ip[2] = *buffptr++;
        addresses[numservers].ip[3] = *buffptr++;
        addresses[numservers].port = *buffptr++ << 8;
        addresses[numservers].port += *buffptr++;
        addresses[numservers].port = BigShort(addresses[numservers].port);
        if (*buffptr != 92)
            break;
        Com_DPrintf(
            14,
            "server: %d ip: %d.%d.%d.%d:%d\n",
            numservers,
            addresses[numservers].ip[0],
            addresses[numservers].ip[1],
            addresses[numservers].ip[2],
            addresses[numservers].ip[3],
            addresses[numservers].port);
        if (++numservers >= 256)
            break;
    } while (buffptr[1] != 69 || buffptr[2] != 79 || buffptr[3] != 84);
    count = cls.numglobalservers;
    for (i = 0; i < numservers && count < 20000; ++i)
    {
        adr.type = NA_IP;
        adr.ip[0] = addresses[i].ip[0];
        adr.ip[1] = addresses[i].ip[1];
        adr.ip[2] = addresses[i].ip[2];
        adr.ip[3] = addresses[i].ip[3];
        adr.port = addresses[i].port;
        *(_QWORD *)&v3.type = __PAIR64__(*(uint32_t *)adr.ip, 4);
        *(_DWORD *)&v3.port = *(_DWORD *)&adr.port;
        *(_QWORD *)&v3.ipx[2] = *(_QWORD *)&adr.ipx[2];
        if (!CL_FindServerInfo(v3))
        {
            server = &cls.globalServers[count++];
            CL_InitServerInfo(server, adr);
        }
    }
    cls.numglobalservers = count;
    CL_SortGlobalServers();
    Com_Printf(14, "%d servers parsed (total %d)\n", numservers, count);
}

char printBuf[2048];
char __cdecl CL_DispatchConnectionlessPacket(netsrc_t localClientNum, netadr_t from, msg_t *msg, int32_t time)
{
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    char *StringLine; // eax
    char *v12; // eax
    clientConnection_t *LocalClientConnection; // eax
    netadr_t v14; // [esp-14h] [ebp-80h]
    netadr_t serverAddress; // [esp-14h] [ebp-80h]
    const char *v16; // [esp-4h] [ebp-70h]
    bool v17; // [esp+0h] [ebp-6Ch]
    char *v18; // [esp+4h] [ebp-68h]
    connstate_t connstate; // [esp+Ch] [ebp-60h]
    const char *c; // [esp+10h] [ebp-5Ch]
    int32_t statPacketsNeeded; // [esp+18h] [ebp-54h]
    clientConnection_t *clcc; // [esp+1Ch] [ebp-50h]
    clientConnection_t *clc; // [esp+1Ch] [ebp-50h]
    clientConnection_t *clca; // [esp+1Ch] [ebp-50h]
    clientConnection_t *clcd; // [esp+1Ch] [ebp-50h]
    clientConnection_t *clce; // [esp+1Ch] [ebp-50h]
    clientConnection_t *clcb; // [esp+1Ch] [ebp-50h]
    char *s; // [esp+20h] [ebp-4Ch]
    char *sa; // [esp+20h] [ebp-4Ch]
    char mapname[68]; // [esp+24h] [ebp-48h] BYREF

    c = Cmd_Argv(0);
    if (!I_stricmp(c, "v"))
    {
        CL_VoicePacket(localClientNum, msg);
        return 1;
    }
    if (!I_stricmp(c, "vt"))
        return 1;

    iassert(localClientNum == 0);
    connstate = clientUIActives[0].connectionState;
    if (I_stricmp(c, "challengeResponse"))
    {
        if (I_stricmp(c, "connectResponse"))
        {
            if (I_stricmp(c, "statResponse"))
            {
                if (!I_stricmp(c, "infoResponse"))
                {
                    CL_ServerInfoPacket(from, msg, time);
                    return 1;
                }
                if (!I_stricmp(c, "statusResponse"))
                {
                    CL_ServerStatusResponse(from, msg);
                    return 1;
                }
                if (I_stricmp(c, "disconnect"))
                {
                    if (!I_stricmp(c, "echo"))
                    {
                        v9 = Cmd_Argv(1);
                        v10 = va("%s", v9);
                        NET_OutOfBandPrint(localClientNum, from, v10);
                        return 1;
                    }
                    if (!I_stricmp(c, "keyAuthorize"))
                        return 1;
                    if (!I_stricmp(c, "print"))
                    {
                        clcd = CL_GetLocalClientConnection(localClientNum);
                        s = MSG_ReadBigString(msg);
                        I_strncpyz(clcd->serverMessage, s, 256);
                        Com_sprintf(printBuf, 0x800u, "%s", s);
                        Com_PrintMessage(14, printBuf, 0);
                        return 1;
                    }
                    if (I_stricmp(c, "error"))
                    {
                        if (!I_stricmp(c, "updateResponse"))
                        {
                            CL_UpdateInfoPacket(from);
                            return 1;
                        }
                        if (!I_strncmp(c, "getserversResponse", 18))
                        {
                            CL_ServersResponsePacket(from, msg);
                            return 1;
                        }
                        if (!I_strncmp(c, "needcdkey", 9))
                        {
                            clce = CL_GetLocalClientConnection(localClientNum);
                            I_strncpyz(clce->serverMessage, "EXE_AWAITINGCDKEYAUTH", 256);
                            SEH_LocalizeTextMessage("EXE_AWAITINGCDKEYAUTH", "need cd key message", LOCMSG_SAFE);
                            Com_Printf(14, "%s\n", clce->serverMessage);
                            CL_RequestAuthorization(localClientNum);
                            return 1;
                        }
                        if (I_stricmp(c, "loadingnewmap"))
                        {
                            if (!I_stricmp(c, "requeststats"))
                            {
                                if (cls.downloadName[0])
                                    return 1;
                                if (localClientNum)
                                    MyAssertHandler(
                                        "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                                        1112,
                                        0,
                                        "%s\n\t(localClientNum) = %i",
                                        "(localClientNum == 0)",
                                        localClientNum);
                                if (clientUIActives[0].connectionState != CA_SENDINGSTATS)
                                {
                                    if (localClientNum)
                                        MyAssertHandler(
                                            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                                            1120,
                                            0,
                                            "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                            localClientNum,
                                            1);
                                    clientUIActives[localClientNum].connectionState = CA_SENDINGSTATS;
                                    LocalClientConnection = CL_GetLocalClientConnection(localClientNum);
                                    LocalClientConnection->statPacketSendTime[0] = 0;
                                    LocalClientConnection->statPacketSendTime[1] = 0;
                                    LocalClientConnection->statPacketSendTime[2] = 0;
                                    LocalClientConnection->statPacketSendTime[3] = 0;
                                    LocalClientConnection->statPacketSendTime[4] = 0;
                                    LocalClientConnection->statPacketSendTime[5] = 0;
                                    LocalClientConnection->statPacketSendTime[6] = 0;
                                    LocalClientConnection->statPacketsToSend = 127;
                                    LocalClientConnection->lastPacketTime = cls.realtime;
                                    LocalClientConnection->lastPacketSentTime = -9999;
                                }
                            }
                            if (I_stricmp(c, "fastrestart"))
                            {
                                return 0;
                            }
                            else
                            {
                                clcb = CL_GetLocalClientConnection(localClientNum);
                                if (localClientNum)
                                    MyAssertHandler(
                                        "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                                        1112,
                                        0,
                                        "%s\n\t(localClientNum) = %i",
                                        "(localClientNum == 0)",
                                        localClientNum);
                                v17 = clientUIActives[0].connectionState == CA_ACTIVE && NET_CompareBaseAdr(from, clcb->serverAddress);
                                clcb->isServerRestarting = v17;
                                return 1;
                            }
                        }
                        else
                        {
                            serverAddress = CL_GetLocalClientConnection(localClientNum)->serverAddress;
                            if (NET_CompareBaseAdr(from, serverAddress))
                            {
                                if (cls.downloadName[0])
                                    return 1;
                                UI_CloseAllMenus(localClientNum);
                                Cbuf_AddText(localClientNum, "uploadStats\n");
                                StringLine = MSG_ReadStringLine(msg);
                                I_strncpyz(mapname, StringLine, 64);
                                clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                                v12 = MSG_ReadStringLine(msg);
                                CL_SetupForNewServerMap(mapname, v12);
                            }
                            return 1;
                        }
                    }
                    else if (connstate
                        && (v14 = CL_GetLocalClientConnection(localClientNum)->serverAddress, NET_CompareBaseAdr(from, v14)))
                    {
                        sa = MSG_ReadBigString(msg);
                        Com_Error(ERR_DROP, "%s", sa);
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (Cmd_Argc() <= 1)
                    {
                        CL_DisconnectPacket(localClientNum, from, 0);
                    }
                    else
                    {
                        v8 = (char *)Cmd_Argv(1);
                        CL_DisconnectPacket(localClientNum, from, v8);
                    }
                    return 1;
                }
            }
            else if (connstate <= CA_SENDINGSTATS)
            {
                if (connstate == CA_SENDINGSTATS)
                {
                    clca = CL_GetLocalClientConnection(localClientNum);
                    v7 = Cmd_Argv(1);
                    statPacketsNeeded = atoi(v7);
                    if (statPacketsNeeded)
                    {
                        clca->statPacketsToSend = statPacketsNeeded & 0x7F;
                    }
                    else
                    {
                        if (localClientNum)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                                1120,
                                0,
                                "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                localClientNum,
                                1);
                        clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                        clca->statPacketsToSend = 0;
                    }
                    clca->lastPacketTime = cls.realtime;
                    clca->lastPacketSentTime = -9999;
                    return 1;
                }
                else
                {
                    Com_Printf(14, "statResponse packet while not syncing stats.  Ignored.\n");
                    return 0;
                }
            }
            else
            {
                Com_Printf(14, "Dup statResponse received.  Ignored.\n");
                return 0;
            }
        }
        else if (connstate < CA_CONNECTED)
        {
            if (connstate == CA_CHALLENGING)
            {
                clc = CL_GetLocalClientConnection(localClientNum);
                if (NET_CompareBaseAdr(from, clc->serverAddress))
                {
                    if (autoupdateChecked
                        && NET_CompareAdr(cls.autoupdateServer, clc->serverAddress)
                        && cl_updateavailable->current.enabled)
                    {
                        autoupdateStarted = 1;
                    }
                    Netchan_Setup(
                        localClientNum,
                        &clc->netchan,
                        from,
                        localClientNum + g_qport,
                        clc->netchanOutgoingBuffer,
                        2048,
                        clc->netchanIncomingBuffer,
                        0x20000);
                    if (Cmd_Argc() <= 1)
                        v18 = (char *)"";
                    else
                        v18 = (char *)Cmd_Argv(1);
                    if (I_stricmp(v18, fs_gameDirVar->current.string))
                        LiveStorage_ReadStatsFromDir(v18);
                    if (localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                            1120,
                            0,
                            "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1);
                    clientUIActives[localClientNum].connectionState = CA_SENDINGSTATS;
                    clc->statPacketSendTime[0] = 0;
                    clc->statPacketSendTime[1] = 0;
                    clc->statPacketSendTime[2] = 0;
                    clc->statPacketSendTime[3] = 0;
                    clc->statPacketSendTime[4] = 0;
                    clc->statPacketSendTime[5] = 0;
                    clc->statPacketSendTime[6] = 0;
                    clc->statPacketsToSend = 127;
                    clc->lastPacketTime = cls.realtime;
                    clc->lastPacketSentTime = -9999;
                    return 1;
                }
                else
                {
                    Com_Printf(14, "connectResponse from a different address.  Ignored.\n");
                    v16 = NET_AdrToString(clc->serverAddress);
                    v6 = NET_AdrToString(from);
                    Com_Printf(14, "%s should have been %s\n", v6, v16);
                    return 0;
                }
            }
            else
            {
                Com_Printf(14, "connectResponse packet while not connecting.  Ignored.\n");
                return 0;
            }
        }
        else
        {
            Com_Printf(14, "Dup connect received.  Ignored.\n");
            return 0;
        }
    }
    else if (connstate == CA_CONNECTING)
    {
        clcc = CL_GetLocalClientConnection(localClientNum);
        v5 = Cmd_Argv(1);
        clcc->challenge = atoi(v5);
        clientUIActives[localClientNum].connectionState = CA_CHALLENGING;
        clcc->connectPacketCount = 0;
        clcc->connectTime = -99999;
        clcc->serverAddress = from;
        Com_DPrintf(14, "challenge: %d\n", clcc->challenge);
        return 1;
    }
    else
    {
        Com_Printf(14, "Unwanted challenge response received.  Ignored.\n");
        return 0;
    }
}

void __cdecl CL_DisconnectPacket(int32_t localClientNum, netadr_t from, char *reason)
{
    clientConnection_t *clc; // [esp+4h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= 3)
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if (NET_CompareAdr(from, clc->netchan.remoteAddress))
        {
            if (cls.realtime - clc->lastPacketTime >= 3000)
            {
                if (cls.wwwDlDisconnected)
                {
                    CL_Disconnect(localClientNum);
                }
                else if (reason)
                {
                    CL_DisconnectError(reason);
                }
                else
                {
                    Com_Error(ERR_DROP, "EXE_SERVER_DISCONNECTED");
                }
            }
        }
    }
}

void __cdecl CL_InitLoad(const char *mapname, const char *gametype)
{
    if (CL_AnyLocalClientsRunning())
    {
        com_expectedHunkUsage = 0;
        UI_SetMap((char*)mapname, (char*)gametype);
        clientUIActives[0].connectionState = (connstate_t)(clientUIActives[0].connectionState < 5 ? 0 : 5);
        SCR_UpdateScreen();
    }
}

void __cdecl CL_WriteDemoClientArchive(
    const clientConnection_t *clc,
    const clientActive_t *cl,
    int32_t localClientNum,
    int32_t index)
{
    char *archive; // [esp+0h] [ebp-8h]
    uint8_t msgType; // [esp+7h] [ebp-1h] BYREF

    archive = (char *)&cl->clientArchive[index];
    msgType = 1;
    FS_Write((char *)&msgType, 1u, clc->demofile);
    FS_Write((char *)&index, 4u, clc->demofile);
    FS_Write(archive + 4, 0xCu, clc->demofile);
    FS_Write(archive + 16, 0xCu, clc->demofile);
    FS_Write(archive + 32, 4u, clc->demofile);
    FS_Write(archive + 28, 4u, clc->demofile);
    FS_Write(archive, 4u, clc->demofile);
    FS_Write(archive + 36, 0xCu, clc->demofile);
}

void __cdecl CL_WriteNewDemoClientArchive(int32_t localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    while (clc->lastClientArchiveIndex != LocalClientGlobals->clientArchiveIndex)
    {
        CL_WriteDemoClientArchive(clc, LocalClientGlobals, localClientNum, clc->lastClientArchiveIndex);
        clc->lastClientArchiveIndex = (clc->lastClientArchiveIndex + 1) % 256;
    }
}

void __cdecl CL_WriteDemoMessage(int32_t localClientNum, msg_t *msg, int32_t headerBytes)
{
    clientConnection_t *LocalClientConnection; // eax
    uint32_t len; // [esp+0h] [ebp-10h]
    clientConnection_t *clc; // [esp+4h] [ebp-Ch]
    int32_t swlen; // [esp+8h] [ebp-8h] BYREF
    uint8_t networkPacketMarker; // [esp+Fh] [ebp-1h] BYREF

    LocalClientConnection = CL_GetLocalClientConnection(0);
    networkPacketMarker = 0;
    FS_Write((char *)&networkPacketMarker, 1u, LocalClientConnection->demofile);
    clc = CL_GetLocalClientConnection(localClientNum);
    swlen = clc->serverMessageSequence;
    FS_Write((char *)&swlen, 4u, clc->demofile);
    swlen = msg->cursize - headerBytes;
    len = swlen;
    FS_Write((char *)&swlen, 4u, clc->demofile);
    FS_Write((char *)&msg->data[headerBytes], len, clc->demofile);
}

char __cdecl CL_PacketEvent(netsrc_t localClientNum, netadr_t from, msg_t *msg, int32_t time)
{
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    int32_t v8; // [esp-8h] [ebp-24h]
    connstate_t connstate; // [esp+4h] [ebp-18h]
    int32_t savedServerMessageSequence; // [esp+8h] [ebp-14h]
    clientConnection_t *clc; // [esp+Ch] [ebp-10h]
    int32_t headerBytes; // [esp+10h] [ebp-Ch]
    int32_t savedReliableAcknowledge; // [esp+14h] [ebp-8h]

    if (msg->cursize >= 4 && *(uint32_t *)msg->data == -1)
        return CL_ConnectionlessPacket(localClientNum, from, msg, time);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connstate = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if (msg->cursize >= 4)
        {
            if (NET_CompareAdr(from, clc->netchan.remoteAddress))
            {
                clc->lastPacketTime = cls.realtime;
                if (Netchan_Process(&clc->netchan, msg))
                {
                    headerBytes = msg->readcount;
                    savedServerMessageSequence = clc->serverMessageSequence;
                    savedReliableAcknowledge = clc->reliableAcknowledge;
                    clc->serverMessageSequence = *(uint32_t *)msg->data;
                    clc->reliableAcknowledge = MSG_ReadLong(msg);
                    if (clc->reliableAcknowledge >= clc->reliableSequence - 128)
                    {
                        CL_Netchan_Decode(&msg->data[msg->readcount], msg->cursize - msg->readcount);
                        CL_ParseServerMessage(localClientNum, msg);
                        if (msg->overflowed)
                        {
                            Com_DPrintf(14, "ignoring illegible message");
                            clc->serverMessageSequence = savedServerMessageSequence;
                            clc->reliableAcknowledge = savedReliableAcknowledge;
                            return 0;
                        }
                        else
                        {
                            if (clc->demorecording)
                            {
                                if (!clc->demowaiting)
                                {
                                    CL_WriteNewDemoClientArchive(localClientNum);
                                    CL_WriteDemoMessage(localClientNum, msg, headerBytes);
                                }
                            }
                            return 1;
                        }
                    }
                    else
                    {
                        clc->reliableAcknowledge = clc->reliableSequence;
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                v7 = NET_AdrToString(from);
                Com_DPrintf(14, "%s:sequenced packet without connection\n", v7);
                return 0;
            }
        }
        else
        {
            v6 = NET_AdrToString(from);
            Com_Printf(14, "%s: Runt packet\n", v6);
            return 1;
        }
    }
    else
    {
        v8 = *(uint32_t *)msg->data;
        v5 = NET_AdrToString(from);
        Com_DPrintf(14, "%s: Got msg sequence %i but connstate (%i) is < CA_CONNECTED\n", v5, v8, connstate);
        return 0;
    }
}

void __cdecl CL_VoiceTransmit(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1072,
            0,
            "localClientNum doesn't index 1\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if ((int)(Sys_Milliseconds() - cl_voiceCommunication.voicePacketLastTransmit) >= 200
        || cl_voiceCommunication.voicePacketCount >= 10)
    {
        if (cl_voiceCommunication.voicePacketCount > 0)
        {
            CL_WriteVoicePacket(localClientNum);
            cl_voiceCommunication.voicePacketCount = 0;
        }
        cl_voiceCommunication.voicePacketLastTransmit = Sys_Milliseconds();
    }
}

void __cdecl CL_RunOncePerClientFrame(int32_t localClientNum, int32_t msec)
{
    int32_t v2; // eax

    FakeLag_Frame();
    if (UI_IsFullscreen(localClientNum))
        CL_SyncGpu(0);
    IN_Frame();
    if (cl_avidemo->current.integer && msec)
    {
        if (clientUIActives[0].connectionState == 9 || cl_forceavidemo->current.enabled)
        {
            v2 = CL_ControllerIndexFromClientNum(localClientNum);
            Cmd_ExecuteSingleCommand(0, v2, (char*)"screenshot silent\n");
        }
        msec = (int)(1000.0 / (double)cl_avidemo->current.integer * com_timescaleValue);
        if (!msec)
            msec = 1;
    }
    cls.realFrametime = msec;
    cls.frametime = msec;
    cls.realtime += msec;
    frame_msec = com_frameTime - old_com_frameTime;
    if (com_frameTime == old_com_frameTime)
        frame_msec = 1;
    if (frame_msec > 0xC8)
        frame_msec = 200;
    old_com_frameTime = com_frameTime;
}

void __cdecl CL_FinishMotdDownload()
{
    void *buf; // [esp+0h] [ebp-8h] BYREF
    int32_t fileSize; // [esp+4h] [ebp-4h]

    fileSize = FS_ReadFile("motd.txt", &buf);
    if (fileSize >= 0)
    {
        Dvar_SetStringByName("motd", (const char*)buf);
        FS_FreeFile((char*)buf);
    }
}

void __cdecl CL_BeginDownload(char *localName, char *remoteName);

void __cdecl CL_NextDownload(int32_t localClientNum);

void __cdecl CL_WWWDownload()
{
    char *error; // [esp+10h] [ebp-110h]
    dlStatus_t ret; // [esp+14h] [ebp-10Ch]
    char to_ospath[260]; // [esp+18h] [ebp-108h] BYREF

    ret = (dlStatus_t)DL_DownloadLoop();
    if (ret)
    {
        if (DL_DLIsMotd())
        {
            if (ret == DL_DONE)
                CL_FinishMotdDownload();
            cls.wwwDlInProgress = 0;
        }
        else if (ret == DL_DONE)
        {
            cls.download = 0;
            FS_BuildOSPath((char*)fs_homepath->current.integer, cls.originalDownloadName, (char*)"", to_ospath);
            to_ospath[&to_ospath[strlen(to_ospath) + 1] - &to_ospath[1] - 1] = 0;
            if (rename(cls.downloadTempName, to_ospath))
            {
                FS_CopyFile(cls.downloadTempName, to_ospath);
                remove(cls.downloadTempName);
            }
            cls.downloadName[0] = 0;
            cls.downloadTempName[0] = 0;
            I_strncpyz(legacyHacks.cl_downloadName, "", 64);
            if (cls.wwwDlDisconnected)
            {
                if (!autoupdateStarted)
                    Cbuf_AddText(0, "reconnect\n");
            }
            else
            {
                CL_AddReliableCommand(0, "wwwdl done");
            }
            cls.wwwDlInProgress = 0;
            CL_NextDownload(0);
        }
        else if (cls.wwwDlDisconnected)
        {
            error = va("Download failure while getting %s", cls.downloadName);
            cls.wwwDlDisconnected = 0;
            CL_ClearStaticDownload();
            Com_Error(ERR_DROP, error);
        }
        else
        {
            Com_Printf(14, "Download failure while getting %s", cls.downloadName);
            CL_AddReliableCommand(0, "wwwdl fail");
            cls.wwwDlInProgress = 0;
        }
    }
}

void __cdecl CL_CheckForUpdateKeyAuth(netsrc_t localClientNum)
{
    int32_t v1; // eax
    clientConnection_t *clc; // [esp+0h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_ACTIVE)
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if (cls.realtime - lastUpdateKeyAuthTime > 300000)
        {
            if (net_lanauthorize->current.enabled || !Sys_IsLANAddress(clc->serverAddress))
                CL_RequestAuthorization(localClientNum);
        }
    }
}

void __cdecl CL_Frame(netsrc_t localClientNum)
{
    connstate_t connstate; // [esp+34h] [ebp-4h]

    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    connstate = clientUIActives[0].connectionState;
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if (clientUIActives[0].isRunning)
    {
        CL_DevGuiFrame(localClientNum);
        {
            PROF_SCOPED("CL_VoiceFrame");
            CL_VoiceFrame(localClientNum);
        }
        CL_UpdateColor(localClientNum);
        CL_CheckUserinfo(localClientNum);
        CL_CheckForResend(localClientNum);
        CL_CheckTimeout(localClientNum);

        if (DL_InProgress())
            CL_WWWDownload();

        CL_UpdateInGameState(localClientNum);
        CL_SetCGameTime(localClientNum);
        CL_CreateCmdsDuringConnection(localClientNum);
        if (connstate < CA_ACTIVE && connstate >= CA_CONNECTED)
            CL_SendCmd(localClientNum);
        CL_CheckForUpdateKeyAuth(localClientNum);
    }
}

void __cdecl CL_CheckTimeout(int32_t localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-Ch]
    connstate_t connstate; // [esp+Ch] [ebp-8h]
    clientConnection_t *clc; // [esp+10h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connstate = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState >= 3)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        clc = CL_GetLocalClientConnection(localClientNum);
        if (cl_paused->current.integer && sv_paused->current.integer
            || connstate < CA_PRIMED
            || clc->lastPacketTime <= 0
            || cl_timeout->current.value * 1000.0 >= (double)(cls.realtime - clc->lastPacketTime))
        {
            if (connstate <= CA_CONNECTING
                || connstate >= CA_PRIMED
                || clc->lastPacketTime <= 0
                || cl_connectTimeout->current.value * 1000.0 >= (double)(cls.realtime - clc->lastPacketTime))
            {
                if (connstate != CA_CONNECTING
                    || clc->lastPacketTime
                    || clc->connectPacketCount <= cl_connectionAttempts->current.integer)
                {
                    LocalClientGlobals->timeoutcount = 0;
                }
                else if (++LocalClientGlobals->timeoutcount > 5)
                {
                    CL_ServerTimedOut();
                }
            }
            else if (++LocalClientGlobals->timeoutcount > 5)
            {
                CL_ServerTimedOut();
            }
        }
        else if (++LocalClientGlobals->timeoutcount > 5)
        {
            CL_ServerTimedOut();
        }
    }
    else if (localClientNum < 1)
    {
        CL_GetLocalClientGlobals(localClientNum)->timeoutcount = 0;
    }
}

void __cdecl CL_ServerTimedOut()
{
    Com_Error(ERR_DROP, "EXE_ERR_SERVER_TIMEOUT");
}

void __cdecl CL_CheckUserinfo(int32_t localClientNum)
{
    char *v1; // eax
    const char *v2; // eax

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= 4 && !cl_paused->current.integer && (dvar_modifiedFlags & 2) != 0)
    {
        v1 = Dvar_InfoString(localClientNum, 2);
        v2 = va("userinfo \"%s\"", v1);
        CL_AddReliableCommand(localClientNum, v2);
    }
}

void __cdecl CL_UpdateInGameState(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == 9)
    {
        if (!cl_ingame->current.enabled)
            Dvar_SetBool((dvar_s *)cl_ingame, 1);
    }
    else if (cl_ingame->current.enabled)
    {
        Dvar_SetBool((dvar_s *)cl_ingame, 0);
    }
}

void __cdecl CL_VoiceFrame(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1072,
            0,
            "localClientNum doesn't index 1\n\t%i not in [0, %i)",
            localClientNum,
            1);
    Voice_GetLocalVoiceData();
    Voice_Playback();
}

bool __cdecl CL_IsLocalClientInGame(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return clientUIActives[0].connectionState == 9;
}

char __cdecl CL_IsClientLocal(int32_t clientNum)
{
    int32_t client; // [esp+0h] [ebp-4h]

    for (client = 0; client < 1; ++client)
    {
        if (client)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                client);
        if (clientUIActives[0].connectionState > 7 && CG_GetClientNum(client) == clientNum)
            return 1;
    }
    return 0;
}

void __cdecl CL_ParseBadPacket_f()
{
    msg_t msg; // [esp+0h] [ebp-30h] BYREF
    int32_t fileSize; // [esp+28h] [ebp-8h]
    char *file; // [esp+2Ch] [ebp-4h] BYREF

    fileSize = FS_ReadFile("badpacket.dat", (void **)&file);
    if (fileSize >= 0)
    {
        msg.overflowed = 0;
        msg.readOnly = 0;
        msg.splitData = 0;
        msg.maxsize = 0;
        memset(&msg.splitSize, 0, 16);
        msg.cursize = fileSize;
        msg.data = (uint8_t *)file;
        MSG_ReadLong(&msg);
        MSG_ReadLong(&msg);
        if (!alwaysfails)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4257, 1, "Time to debug this packet, baby!");
        CL_ParseServerMessage(NS_CLIENT1, &msg);
        FS_FreeFile(file);
    }
}

void __cdecl CL_ShutdownRef()
{
    R_SyncRenderThread();
    CL_ShutdownRenderer(1);
    track_shutdown(3);
    StatMon_Reset();
}

void __cdecl CL_InitRenderer()
{
    int32_t localClientNum; // [esp+0h] [ebp-4h]

    if (cls.rendererStarted)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4284, 0, "%s", "!cls.rendererStarted");
    cls.rendererStarted = 1;
    R_BeginRegistration(&cls.vidConfig);
    ScrPlace_SetupUnsafeViewport(&scrPlaceFullUnsafe, 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
    ScrPlace_SetupViewport(&scrPlaceFull, 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        ScrPlace_SetupViewport(&scrPlaceView[localClientNum], 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
    cls.whiteMaterial = Material_RegisterHandle("white", 3);
    cls.consoleMaterial = Material_RegisterHandle("console", 3);
    cls.consoleFont = R_RegisterFont("fonts/consoleFont", 3);
    g_console_field_width = cls.vidConfig.displayWidth - 48;
    g_consoleField.widthInPixels = cls.vidConfig.displayWidth - 48;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.fixedSize = 1;
    StatMon_Reset();
    Con_InitClientAssets();
}

void __cdecl CL_ShutdownRenderer(int32_t destroyWindow)
{
    if (!cls.rendererStarted && !destroyWindow)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4313, 0, "%s", "cls.rendererStarted || destroyWindow");
    cls.rendererStarted = 0;
    Com_ShutdownWorld();
    if (useFastFile->current.enabled && destroyWindow)
        CM_Shutdown();
    R_Shutdown(destroyWindow);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    KISAK_NULLSUB();
}

void __cdecl CL_StartHunkUsers()
{
    if (cls.hunkUsersStarted)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4400, 0, "%s", "!cls.hunkUsersStarted");
    if (CL_AnyLocalClientsRunning())
    {
        if (!cls.soundStarted)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4405, 0, "%s", "cls.soundStarted");
        if (!cls.rendererStarted)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4406, 0, "%s", "cls.rendererStarted");
        if (!cls.uiStarted)
        {
            CL_InitUI();
            if (!cls.uiStarted)
                MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4411, 0, "%s", "cls.uiStarted");
        }
        if (!cls.devGuiStarted)
        {
            cls.devGuiStarted = 1;
            CL_InitDevGui();
        }
        cls.hunkUsersStarted = 1;
    }
}

cmd_function_s CL_DevGuiDvar_f_VAR;
cmd_function_s CL_DevGuiCmd_f_VAR;
cmd_function_s CL_DevGuiOpen_f_VAR;

void CL_InitDevGui()
{
    DevGui_Init();
    Cmd_AddCommandInternal("devgui_dvar", CL_DevGuiDvar_f, &CL_DevGuiDvar_f_VAR);
    Cmd_AddCommandInternal("devgui_cmd", CL_DevGuiCmd_f, &CL_DevGuiCmd_f_VAR);
    Cmd_AddCommandInternal("devgui_open", CL_DevGuiOpen_f, &CL_DevGuiOpen_f_VAR);
    CL_CreateDevGui();
}

void __cdecl CL_DevGuiDvar_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        v1 = Cmd_Argv(2);
        dvar = Dvar_FindVar(v1);
        if (dvar)
        {
            v3 = Cmd_Argv(1);
            DevGui_AddDvar(v3, dvar);
        }
        else
        {
            v2 = Cmd_Argv(2);
            Com_Printf(11, "dvar '%s' doesn't exist\n", v2);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\" dvarName\n", v0);
    }
}

void __cdecl CL_DevGuiCmd_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char *v2; // [esp-4h] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        v2 = (char *)Cmd_Argv(2);
        v1 = Cmd_Argv(1);
        DevGui_AddCommand(v1, v2);
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\" \"command text\"\n", v0);
    }
}

void __cdecl CL_DevGuiOpen_f()
{
    const char *v0; // eax
    const char *v1; // eax

    if (Cmd_Argc() == 2)
    {
        v1 = Cmd_Argv(1);
        DevGui_OpenMenu(v1);
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\"\n", v0);
    }
}

int32_t __cdecl CL_ScaledMilliseconds()
{
    return cls.realtime;
}

void __cdecl CL_SetFastFileNames(GfxConfiguration *config, bool dedicatedServer)
{
    iassert(config);

    config->codeFastFileName = "code_post_gfx_mp";
    config->uiFastFileName = !dedicatedServer ? "ui_mp" : 0;
    config->commonFastFileName = "common_mp";
    config->localizedCodeFastFileName = "localized_code_post_gfx_mp";
    config->localizedCommonFastFileName = "localized_common_mp";
    config->modFastFileName = DB_ModFileExists() != 0 ? "mod" : 0;
}

void __cdecl CL_InitRef()
{
    GfxConfiguration config; // [esp+0h] [ebp-30h] BYREF

    Com_Printf(14, "----- Initializing Renderer ----\n");
    SetupGfxConfig(&config);
    CL_SetFastFileNames(&config, 0);
    R_ConfigureRenderer(&config);
    Dvar_SetInt((dvar_s *)cl_paused, 0);
}

void __cdecl SetupGfxConfig(GfxConfiguration *config)
{
    if (!config)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4453, 0, "%s", "config");
    config->maxClientViews = 1;
    config->entCount = 1024;
    config->entnumNone = 1023;
    config->entnumOrdinaryEnd = 1022;
    config->threadContextCount = 7;
    config->critSectCount = 22;
}

void __cdecl CL_startSingleplayer_f()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4558, 0, "%s", "Sys_IsMainThread()");
    Sys_QuitAndStartProcess("cod3sp.exe", 0);
}

void __cdecl CL_DrawLogo(int32_t localClientNum)
{
    float fade; // [esp+44h] [ebp-24h]
    int32_t time; // [esp+48h] [ebp-20h]
    float h1; // [esp+4Ch] [ebp-1Ch]
    float h0; // [esp+50h] [ebp-18h]
    float color[4]; // [esp+54h] [ebp-14h] BYREF
    float w; // [esp+64h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState != 2)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            4644,
            0,
            "%s",
            "CL_GetLocalClientConnectionState( localClientNum ) == CA_LOGO");
    time = cls.realtime - cls.logo.startTime;
    if (cls.realtime - cls.logo.startTime >= cls.logo.fadein)
    {
        if (time <= cls.logo.duration - cls.logo.fadeout)
            fade = 1.0;
        else
            fade = (double)(cls.logo.duration - time) / (double)cls.logo.fadeout;
    }
    else
    {
        fade = (double)time / (double)cls.logo.fadein;
    }
    if (fade >= 0.0)
    {
        if (fade > 1.0)
            fade = 1.0;
    }
    else
    {
        fade = 0.0;
    }
    color[0] = fade;
    color[1] = fade;
    color[2] = fade;
    color[3] = 1.0;
    w = (float)cls.vidConfig.displayWidth;
    h0 = ((double)cls.vidConfig.displayHeight + (double)cls.vidConfig.displayHeight) / 3.0;
    h1 = (double)cls.vidConfig.displayHeight - h0;
    R_AddCmdDrawStretchPic(0.0, 0.0, w, h0, 0.0, 0.0, 1.0, 1.0, color, cls.logo.material[0]);
    R_AddCmdDrawStretchPic(0.0, h0, w, h1, 0.0, 0.0, 1.0, 1.0, color, cls.logo.material[1]);
    if (time > cls.logo.duration)
        CL_StopLogo(localClientNum);
}

void __cdecl CL_StopLogo(int32_t localClientNum)
{
    clientUIActives[localClientNum].connectionState = CA_DISCONNECTED;
}

void __cdecl CL_PlayLogo_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    float v5; // [esp+8h] [ebp-48h]
    float v6; // [esp+Ch] [ebp-44h]
    float v7; // [esp+10h] [ebp-40h]
    float v8; // [esp+14h] [ebp-3Ch]
    float v9; // [esp+24h] [ebp-2Ch]
    float v10; // [esp+34h] [ebp-1Ch]
    const char *name; // [esp+4Ch] [ebp-4h]

    if (Cmd_Argc() != 5)
    {
        Com_Printf(0, "USAGE: logo <image name> <fadein seconds> <full duration seconds> <fadeout seconds>\n");
        return;
    }
    Com_DPrintf(0, "CL_PlayLogo_f\n");
    if (clientUIActives[0].connectionState == CA_CINEMATIC)
    {
        SCR_StopCinematic(0);
    }
    else if (clientUIActives[0].connectionState == CA_LOGO)
    {
        CL_StopLogo(0);
    }
    else if (clientUIActives[0].connectionState)
    {
        return;
    }
    clientUIActives[0].connectionState = CA_LOGO;
    if (cls.uiStarted)
        UI_SetActiveMenu(0, (uiMenuCommand_t)0);
    SND_StopSounds(SND_STOP_ALL);
    SND_FadeAllSounds(1.0, 0);
    name = Cmd_Argv(1);
    v0 = Cmd_Argv(2);
    v7 = atof(v0);
    cls.logo.fadein = (int)(v7 * 1000.0f);
    v1 = Cmd_Argv(3);
    v6 = atof(v1);
    cls.logo.duration = (int)(v6 * 1000.0f);
    v2 = Cmd_Argv(4);
    v5 = atof(v2);
    cls.logo.fadeout = (int)(v5 * 1000.0f);
    cls.logo.duration += cls.logo.fadeout + cls.logo.fadein;
    v3 = va("%s1", name);
    cls.logo.material[0] = Material_RegisterHandle(v3, 3);
    v4 = va("%s2", name);
    cls.logo.material[1] = Material_RegisterHandle(v4, 3);
    cls.logo.startTime = cls.realtime + 100;
}

void __cdecl CL_StopLogoOrCinematic(int32_t localClientNum)
{
    connstate_t clcState; // [esp+0h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clcState = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState == 1)
    {
        SCR_StopCinematic(localClientNum);
    }
    else
    {
        if (clientUIActives[0].connectionState != 2)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4744, 0, "%s", "clcState == CA_LOGO");
        CL_StopLogo(localClientNum);
    }
    SND_StopSounds(SND_STOP_ALL);
    if (clcState)
        UI_SetActiveMenu(localClientNum, (uiMenuCommand_t)0);
    else
        UI_SetActiveMenu(localClientNum, (uiMenuCommand_t)1);
}

void __cdecl CL_ToggleMenu_f()
{
    uiMenuCommand_t ActiveMenu; // [esp+0h] [ebp-18h]
    connstate_t connstate; // [esp+8h] [ebp-10h]
    clientConnection_t *clc; // [esp+10h] [ebp-8h]

    clc = CL_GetLocalClientConnection(0);
    connstate = clientUIActives[0].connectionState;
    if ((clientUIActives[0].keyCatchers & 0x10) != 0)
        ActiveMenu = UI_GetActiveMenu(0);
    else
        ActiveMenu = UIMENU_NONE;
    if (clc->demoplaying)
    {
    LABEL_13:
        UI_SetActiveMenu(0, UIMENU_MAIN);
        return;
    }
    if (connstate != CA_ACTIVE)
    {
        if (!cl_waitingOnServerToLoadMap[0])
            return;
        goto LABEL_13;
    }
    if ((clientUIActives[0].keyCatchers & 0x10) != 0 && ActiveMenu == UIMENU_INGAME)
    {
        UI_SetActiveMenu(0, UIMENU_NONE);
    }
    else if (!clientUIActives[0].keyCatchers)
    {
        UI_SetActiveMenu(0, UIMENU_INGAME);
    }
}

void __cdecl CL_WriteAllDemoClientArchive(int32_t localClientNum)
{
    const clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    clientConnection_t *clc; // [esp+4h] [ebp-8h]
    int32_t index; // [esp+8h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    for (index = 0; index < 256; ++index)
        CL_WriteDemoClientArchive(clc, LocalClientGlobals, localClientNum, index);
}

cmd_function_s CL_ForwardToServer_f_VAR;
cmd_function_s CL_Configstrings_f_VAR;
cmd_function_s CL_Clientinfo_f_VAR;
cmd_function_s CL_Vid_Restart_f_VAR;
cmd_function_s CL_Vid_Restart_f_VAR_SERVER;
cmd_function_s CL_Snd_Restart_f_VAR;
cmd_function_s CL_Snd_Restart_f_VAR_SERVER;
cmd_function_s CL_Disconnect_f_VAR;
cmd_function_s CL_Disconnect_f_VAR_SERVER;
cmd_function_s CL_Record_f_VAR;
cmd_function_s CL_StopRecord_f_VAR;
cmd_function_s CL_PlayDemo_f_VAR_0;
cmd_function_s CL_PlayDemo_f_VAR_SERVER_0;
cmd_function_s CL_PlayDemo_f_VAR;
cmd_function_s CL_PlayDemo_f_VAR_SERVER;
cmd_function_s CL_PlayCinematic_f_VAR;
cmd_function_s CL_PlayUnskippableCinematic_f_VAR;
cmd_function_s CL_PlayLogo_f_VAR;
cmd_function_s CL_Connect_f_VAR;
cmd_function_s CL_Connect_f_VAR_SERVER;
cmd_function_s CL_Reconnect_f_VAR;
cmd_function_s CL_Reconnect_f_VAR_SERVER;
cmd_function_s CL_LocalServers_f_VAR;
cmd_function_s CL_GlobalServers_f_VAR;
cmd_function_s CL_Rcon_f_VAR;
cmd_function_s CL_Ping_f_VAR;
cmd_function_s CL_Ping_f_VAR_SERVER;
cmd_function_s CL_ServerStatus_f_VAR;
cmd_function_s CL_ServerStatus_f_VAR_SERVER;
cmd_function_s CL_Setenv_f_VAR;
cmd_function_s CL_ShowIP_f_VAR;
cmd_function_s CL_ToggleMenu_f_VAR;
cmd_function_s CL_OpenedIWDList_f_VAR;
cmd_function_s CL_ReferencedIWDList_f_VAR;
cmd_function_s CL_UpdateLevelHunkUsage_VAR;
cmd_function_s CL_startSingleplayer_f_VAR;
cmd_function_s CL_ParseBadPacket_f_VAR;
cmd_function_s CL_CubemapShot_f_VAR;
cmd_function_s CL_OpenScriptMenu_f_VAR;
cmd_function_s UI_OpenMenu_f_VAR;
cmd_function_s UI_CloseMenu_f_VAR;
cmd_function_s UI_ListMenus_f_VAR;
cmd_function_s Com_WriteLocalizedSoundAliasFiles_VAR;
cmd_function_s CL_SelectStringTableEntryInDvar_f_VAR;
cmd_function_s KISAK_NULLSUB_VAR;

void __cdecl CL_Record_f()
{
    int32_t v0; // eax
    int32_t number; // [esp+4h] [ebp-2C8h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-2C4h]
    __int16 configStringCount; // [esp+Ch] [ebp-2C0h]
    int32_t compressedSize; // [esp+18h] [ebp-2B4h]
    connstate_t connstate; // [esp+1Ch] [ebp-2B0h]
    uint8_t (*bufData)[131072]; // [esp+20h] [ebp-2ACh]
    char demoName[64]; // [esp+24h] [ebp-2A8h] BYREF
    entityState_s nullstate; // [esp+64h] [ebp-268h] BYREF
    uint8_t (*compressedBuf)[131072]; // [esp+15Ch] [ebp-170h]
    int32_t localClientNum; // [esp+160h] [ebp-16Ch]
    msg_t buf; // [esp+164h] [ebp-168h] BYREF
    SnapshotInfo_s snapInfo; // [esp+18Ch] [ebp-140h] BYREF
    char name[260]; // [esp+1A4h] [ebp-128h] BYREF
    int32_t len; // [esp+2ACh] [ebp-20h] BYREF
    clientConnection_t *clc; // [esp+2B0h] [ebp-1Ch]
    uint8_t type; // [esp+2BFh] [ebp-Dh] BYREF
    const entityState_s *ent; // [esp+2C0h] [ebp-Ch]
    const char *s; // [esp+2C4h] [ebp-8h]
    int32_t i; // [esp+2C8h] [ebp-4h]

    LargeLocal bufData_large_local(0x20000); // [esp+2B4h] [ebp-18h] BYREF
    LargeLocal compressedBuf_large_local(0x20000); // [esp+10h] [ebp-2BCh] BYREF

    //LargeLocal::LargeLocal(&bufData_large_local, 0x20000);
    //bufData = (uint8_t (*)[131072])LargeLocal::GetBuf(&bufData_large_local);
    bufData = (uint8_t (*)[131072])bufData_large_local.GetBuf();
    //LargeLocal::LargeLocal(&compressedBuf_large_local, 0x20000);
    //compressedBuf = (uint8_t (*)[131072])LargeLocal::GetBuf(&compressedBuf_large_local);
    compressedBuf = (uint8_t (*)[131072])compressedBuf_large_local.GetBuf();
    if (Cmd_Argc() <= 2)
    {
        localClientNum = 0;
        clc = CL_GetLocalClientConnection(0);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        connstate = clientUIActives[0].connectionState;
        if (clc->demorecording)
        {
            Com_Printf(0, "Already recording.\n");
        }
        else if (connstate == CA_ACTIVE)
        {
            if (Cmd_Argc() == 2)
            {
                s = Cmd_Argv(1);
                I_strncpyz(demoName, (char *)s, 64);
                Com_sprintf(name, 0x100u, "demos/%s.dm_%d", demoName, 1);
            }
            else
            {
                for (number = 0; number <= 9999; ++number)
                {
                    Com_sprintf(demoName, 0x40u, "demo%04i", number);
                    Com_sprintf(name, 0x100u, "demos/%s.dm_%d", demoName, 1);
                    if (!FS_FileExists(name))
                        break;
                }
            }
            Com_Printf(0, "recording to %s.\n", name);
            v0 = FS_FOpenFileWrite(name);
            clc->demofile = v0;
            if (clc->demofile)
            {
                clc->demorecording = 1;
                I_strncpyz(clc->demoName, demoName, 64);
                clc->demowaiting = 1;
                MSG_Init(&buf, (uint8_t *)bufData, 0x20000);
                MSG_WriteLong(&buf, clc->reliableSequence);
                MSG_WriteByte(&buf, 1u);
                MSG_WriteLong(&buf, clc->serverCommandSequence);
                LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
                MSG_WriteByte(&buf, 2u);
                configStringCount = 0;
                for (i = 0; i < 2442; ++i)
                {
                    if (LocalClientGlobals->gameState.stringOffsets[i])
                        ++configStringCount;
                }
                MSG_WriteShort(&buf, configStringCount);
                for (i = 0; i < 2442; ++i)
                {
                    if (LocalClientGlobals->gameState.stringOffsets[i])
                    {
                        s = &LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.stringOffsets[i]];
                        MSG_WriteBit0(&buf);
                        MSG_WriteBits(&buf, i, 0xCu);
                        MSG_WriteBigString(&buf, (char *)s);
                    }
                }
                svsHeaderValid = 1;
                svsHeader.mapCenter[0] = cls.mapCenter[0];
                svsHeader.mapCenter[1] = cls.mapCenter[1];
                svsHeader.mapCenter[2] = cls.mapCenter[2];
                memset(&snapInfo, 0, sizeof(snapInfo));
                memset((uint8_t *)&nullstate, 0, sizeof(nullstate));
                for (i = 0; i < 1024; ++i)
                {
                    ent = &LocalClientGlobals->entityBaselines[i];
                    if (ent->number)
                    {
                        MSG_WriteByte(&buf, 3u);
                        MSG_WriteEntity(&snapInfo, &buf, -90000, &nullstate, ent, 1);
                    }
                }
                MSG_WriteByte(&buf, 7u);
                MSG_WriteLong(&buf, clc->clientNum);
                MSG_WriteLong(&buf, clc->checksumFeed);
                MSG_WriteByte(&buf, 7u);
                if (buf.cursize < 4)
                    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 987, 0, "%s", "buf.cursize >= CL_DECODE_START");
                *(_DWORD *)compressedBuf = *(_DWORD *)buf.data;
                compressedSize = MSG_WriteBitsCompress(
                    0,
                    (const uint8_t *)buf.data + 4,
                    &(*compressedBuf)[4],
                    buf.cursize - 4)
                    + 4;
                type = 0;
                FS_Write((char *)&type, 1u, clc->demofile);
                len = clc->serverMessageSequence;
                FS_Write((char *)&len, 4u, clc->demofile);
                len = compressedSize;
                FS_Write((char *)&len, 4u, clc->demofile);
                FS_Write((char *)compressedBuf, compressedSize, clc->demofile);
                CL_WriteAllDemoClientArchive(localClientNum);
                clc->lastClientArchiveIndex = LocalClientGlobals->clientArchiveIndex;
            }
            else
            {
                Com_PrintError(0, "ERROR: couldn't open.\n");
            }
        }
        else
        {
            Com_Printf(0, "You must be in a level to record.\n");
        }
    }
    else
    {
        Com_Printf(0, "record <demoname>\n");
    }
}

void __cdecl CL_StopRecord_f()
{
    int32_t len; // [esp+0h] [ebp-Ch] BYREF
    clientConnection_t *clc; // [esp+4h] [ebp-8h]
    char type; // [esp+Bh] [ebp-1h] BYREF

    clc = CL_GetLocalClientConnection(0);
    if (clc->demorecording)
    {
        type = 0;
        FS_Write(&type, 1u, clc->demofile);
        len = -1;
        FS_Write((char*)&len, 4u, clc->demofile);
        FS_Write((char *)&len, 4u, clc->demofile);
        FS_FCloseFile(clc->demofile);
        clc->demofile = 0;
        clc->demorecording = 0;
        Com_Printf(0, "Stopped demo.\n");
    }
    else
    {
        Com_Printf(0, "Not recording a demo.\n");
    }
}

void __cdecl CL_PlayDemo_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int32_t v4; // eax
    const char *v5; // eax
    char extension[32]; // [esp+24h] [ebp-134h] BYREF
    int32_t localClientNum; // [esp+44h] [ebp-114h]
    char name[260]; // [esp+48h] [ebp-110h] BYREF
    clientConnection_t *clc; // [esp+150h] [ebp-8h]
    const char *arg; // [esp+154h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        if (com_sv_running->current.enabled)
        {
            Com_Printf(14, "listen server cannot play a demo.\n");
        }
        else
        {
            localClientNum = 0;
            CL_Disconnect(0);
            arg = Cmd_Argv(1);
            Com_sprintf(extension, 0x20u, ".dm_%d", 1);
            if (I_stricmp(&arg[strlen(arg) - (&extension[strlen(extension) + 1] - &extension[1])], extension))
                Com_sprintf(name, 0x100u, "demos/%s.dm_%d", arg, 1);
            else
                Com_sprintf(name, 0x100u, "demos/%s", arg);
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                    1112,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            clc = CL_GetLocalClientConnection(localClientNum);
            FS_FOpenFileRead(name, &clc->demofile);
            if (!clc->demofile)
            {
                v1 = va("EXE_ERR_NOT_FOUND %s", name);
                Com_Error(ERR_DROP, v1);
            }
            v2 = Cmd_Argv(1);
            I_strncpyz(clc->demoName, v2, 64);
            Con_Close(localClientNum);
            KISAK_NULLSUB();
            clientUIActives[localClientNum].connectionState = CA_CONNECTED;
            clc->demoplaying = 1;
            v3 = Cmd_Argv(0);
            v4 = I_stricmp(v3, "timedemo");
            clc->isTimeDemo = v4 == 0;
            clc->lastClientArchiveIndex = 0;
            v5 = Cmd_Argv(1);
            I_strncpyz(cls.servername, v5, 256);
            while (clientUIActives[localClientNum].connectionState >= CA_CONNECTED
                && clientUIActives[localClientNum].connectionState < CA_PRIMED)
                CL_ReadDemoMessage(localClientNum);
            clc->firstDemoFrameSkipped = 0;
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(14, "%s <demoname>\n", v0);
    }
}

void __cdecl CL_RconInit()
{
    rconGlob.password[0] = 0;
    rconGlob.host.type = NA_BAD;
}

void CL_RconLogin()
{
    uint32_t v0; // [esp+Ch] [ebp-Ch]
    const char *password; // [esp+14h] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        password = Cmd_Argv(2);
        if (strlen(password) < 24)
            memcpy(&rconGlob, password, strlen(password));
        else
            Com_Printf(14, "rcon password must be %i characters or less\n", 24);
    }
    else
    {
        Com_Printf(14, "USAGE: rcon login <password>\n");
    }
}

void CL_RconLogout()
{
    if (rconGlob.password[0])
        rconGlob.password[0] = 0;
    else
        Com_Printf(14, "Not logged in\n");
}

ping_t *__cdecl CL_GetFreePing()
{
    ping_t *best; // [esp+0h] [ebp-18h]
    DWORD currentTime; // [esp+8h] [ebp-10h]
    int32_t oldest; // [esp+Ch] [ebp-Ch]
    int32_t i; // [esp+10h] [ebp-8h]
    int32_t ia; // [esp+10h] [ebp-8h]
    ping_t *pingptr; // [esp+14h] [ebp-4h]
    ping_t *pingptra; // [esp+14h] [ebp-4h]

    currentTime = Sys_Milliseconds();
    pingptr = cl_pinglist;
    for (i = 0; i < 16; ++i)
    {
        if (!pingptr->adr.port)
            goto LABEL_9;
        if (pingptr->time)
        {
            if (pingptr->time >= 500)
            {
            LABEL_9:
                pingptr->adr.port = 0;
                return pingptr;
            }
        }
        else if ((currentTime - pingptr->start) >= 500)
        {
            goto LABEL_9;
        }
        ++pingptr;
    }
    pingptra = cl_pinglist;
    best = cl_pinglist;
    oldest = 0x80000000;
    for (ia = 0; ia < 16; ++ia)
    {
        if ((currentTime - pingptra->start) > oldest)
        {
            oldest = currentTime - pingptra->start;
            best = pingptra;
        }
        ++pingptra;
    }
    return best;
}

void __cdecl CL_Ping_f()
{
    netadr_t to; // [esp+0h] [ebp-20h] BYREF
    ping_t *pingptr; // [esp+18h] [ebp-8h]
    const char *server; // [esp+1Ch] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        memset(&to, 0, sizeof(to));
        server = Cmd_Argv(1);
        if (NET_StringToAdr((char*)server, &to))
        {
            pingptr = CL_GetFreePing();
            pingptr->adr = to;
            pingptr->start = Sys_Milliseconds();
            pingptr->time = 0;
            CL_SetServerInfoByAddress(pingptr->adr, 0, 0);
            NET_OutOfBandPrint(NS_CLIENT1, to, "getinfo xxx");
        }
    }
    else
    {
        Com_Printf(0, "usage: ping [server]\n");
    }
}

void CL_RconHost()
{
    const char *hostName; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        hostName = Cmd_Argv(2);
        if (NET_StringToAdr((char*)hostName, &rconGlob.host))
        {
            if (rconGlob.host.type == NA_BAD)
                MyAssertHandler(".\\client_mp\\cl_main_pc_mp.cpp", 782, 1, "%s", "rconGlob.host.type != NA_BAD");
            if (!rconGlob.host.port)
                rconGlob.host.port = BigShort(28960);
        }
        else
        {
            Com_Printf(14, "bad host address\n");
            if (rconGlob.host.type != NA_BAD)
                MyAssertHandler(
                    ".\\client_mp\\cl_main_pc_mp.cpp",
                    779,
                    1,
                    "%s\n\t(rconGlob.host.type) = %i",
                    "(rconGlob.host.type == NA_BAD)",
                    rconGlob.host.type);
        }
    }
    else
    {
        Com_Printf(14, "USAGE: rcon host <address>\n");
    }
}

void __cdecl CL_Rcon_f()
{
    const char *v0; // eax
    int32_t v1; // [esp-Ch] [ebp-450h]
    int32_t v2; // [esp-8h] [ebp-44Ch]
    connstate_t connstate; // [esp+10h] [ebp-434h]
    char message[1028]; // [esp+14h] [ebp-430h] BYREF
    int32_t maxlen; // [esp+418h] [ebp-2Ch]
    int32_t len; // [esp+41Ch] [ebp-28h]
    const clientConnection_t *clc; // [esp+420h] [ebp-24h]
    int32_t i; // [esp+424h] [ebp-20h]
    netadr_t to; // [esp+428h] [ebp-1Ch]
    const char *cmd; // [esp+440h] [ebp-4h]

    if (Cmd_Argc() < 2)
    {
        Com_Printf(0, "USAGE: rcon <command> <options...>\n");
        return;
    }
    cmd = Cmd_Argv(1);
    if (!I_stricmp(cmd, "login"))
    {
        CL_RconLogin();
        return;
    }
    if (!I_stricmp(cmd, "logout"))
    {
        CL_RconLogout();
        return;
    }
    if (!I_stricmp(cmd, "host"))
    {
        CL_RconHost();
        return;
    }
    if (!rconGlob.password[0])
    {
        Com_Printf(0, "You need to log in with 'rcon login <password>' before using rcon.\n");
        return;
    }
    maxlen = 1024;
    len = Com_AddToString("rcon ", message, 0, 1024, 0);
    len = Com_AddToString(rconGlob.password, message, len, maxlen, 0);
    for (i = 1; i < Cmd_Argc(); ++i)
    {
        len = Com_AddToString(" ", message, len, maxlen, 0);
        v2 = maxlen;
        v1 = len;
        v0 = Cmd_Argv(i);
        len = Com_AddToString(v0, message, v1, v2, 1);
    }
    if (len == maxlen)
    {
        Com_Printf(0, "rcon commands are limited to %i characters\n", maxlen - 1);
        return;
    }
    message[len] = 0;
    connstate = clientUIActives[0].connectionState;
    clc = CL_GetLocalClientConnection(0);
    if (connstate < CA_CONNECTED)
    {
        if (rconGlob.host.type == NA_BAD)
        {
            Com_Printf(0, "Can't determine rcon target.  You can fix this by either:\n");
            Com_Printf(0, "1) Joining the server as a player.\n");
            Com_Printf(0, "2) Setting the host server with 'rcon host <address>'.\n");
            return;
        }
        to = rconGlob.host;
    }
    else
    {
        to = clc->netchan.remoteAddress;
    }
    NET_OutOfBandData(NS_CLIENT1, to, (const unsigned char*)message, &message[strlen(message) + 1] - &message[1] + 1);
}

void __cdecl CL_OpenedIWDList_f()
{
    char *v0; // eax

    v0 = FS_LoadedIwdNames();
    Com_Printf(0, "Opened IWD Names: %s\n", v0);
}

void __cdecl CL_ReferencedIWDList_f()
{
    char *v0; // eax

    v0 = FS_ReferencedIwdNames();
    Com_Printf(0, "Referenced IWD Names: %s\n", v0);
}

void __cdecl CL_UpdateLevelHunkUsage()
{
    int32_t v0; // eax
    const char *v1; // eax
    uint32_t v2; // eax
    int32_t handle; // [esp+20h] [ebp-130h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+24h] [ebp-12Ch]
    const char *memlistfile; // [esp+28h] [ebp-128h]
    char *buf; // [esp+2Ch] [ebp-124h]
    int32_t localClientNum; // [esp+30h] [ebp-120h]
    int32_t len; // [esp+34h] [ebp-11Ch]
    char *outbuftrav; // [esp+38h] [ebp-118h]
    int32_t memusage; // [esp+3Ch] [ebp-114h]
    char outstr[256]; // [esp+40h] [ebp-110h] BYREF
    const char *token; // [esp+144h] [ebp-Ch]
    char *outbuf; // [esp+148h] [ebp-8h]
    const char *buftrav; // [esp+14Ch] [ebp-4h] BYREF

    memlistfile = "hunkusage.dat";
    memusage = Hunk_Used();
    localClientNum = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", &handle, FS_READ);
    if (len >= 0)
    {
        buf = (char*)Z_Malloc(len + 1, "CL_UpdateLevelHunkUsage", 10);
        memset(buf, 0, len + 1);
        outbuf = (char*)Z_Malloc(len + 1, "CL_UpdateLevelHunkUsage", 10);
        memset(outbuf, 0, len + 1);
        FS_Read((unsigned char*)buf, len, handle);
        FS_FCloseFile(handle);
        buftrav = buf;
        outbuftrav = outbuf;
        *outbuf = 0;
        while (1)
        {
            token = Com_Parse(&buftrav)->token;
            if (!token || !*token)
                break;
            if (I_stricmp(token, LocalClientGlobals->mapname))
            {
                I_strncat(outbuftrav, len + 1, token);
                I_strncat(outbuftrav, len + 1, " ");
                token = Com_Parse(&buftrav)->token;
                if (token && *token)
                {
                    I_strncat(outbuftrav, len + 1, token);
                    I_strncat(outbuftrav, len + 1, "\n");
                }
                else
                {
                    Com_Error(ERR_DROP, "EXE_ERR_HUNGUSAGE_CORRUPT");
                }
            }
            else
            {
                token = Com_Parse(&buftrav)->token;
                if (token)
                {
                    if (*token)
                    {
                        v0 = atoi(token);
                        if (v0 == memusage)
                        {
                            Z_Free(buf, 10);
                            Z_Free(outbuf, 10);
                            return;
                        }
                    }
                }
            }
        }
        handle = FS_FOpenFileWrite((char*)memlistfile);
        if (!handle)
        {
            v1 = va("EXE_ERR_CANT_CREATE %s", memlistfile);
            Com_Error(ERR_DROP, v1);
        }
        len = strlen(outbuf);
        v2 = FS_Write(outbuf, len, handle);
        if (v2 != len)
            Com_Error(ERR_DROP, "EXE_ERR_CANT_WRITE %s", memlistfile);
        FS_FCloseFile(handle);
        Z_Free(buf, 10);
        Z_Free(outbuf, 10);
    }
    FS_FOpenFileByMode((char*)memlistfile, &handle, FS_APPEND);
    if (!handle)
        Com_Error(ERR_DROP, "EXE_ERR_HUNKUSAGE_CANT_WRITE");
    Com_sprintf(outstr, 0x100u, "%s %i\n", LocalClientGlobals->mapname, memusage);
    FS_Write(outstr, &outstr[strlen(outstr) + 1] - &outstr[1], handle);
    FS_FCloseFile(handle);
    len = FS_FOpenFileByMode((char *)memlistfile, &handle, FS_READ);
    if (len >= 0)
        FS_FCloseFile(handle);
}

void __cdecl CL_OpenScriptMenu_f()
{
    int32_t Int; // eax
    const char *v1; // eax
    char *menuName; // [esp+0h] [ebp-10h]
    const char *menuResponse; // [esp+4h] [ebp-Ch]
    int32_t menuIndex; // [esp+8h] [ebp-8h]
    const char *parentMenuName; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        if (UI_AllowScriptMenuResponse(0))
        {
            if (cls.uiStarted)
            {
                parentMenuName = Cmd_Argv(1);
                menuResponse = Cmd_Argv(2);
                if (parentMenuName)
                {
                    if (menuResponse)
                    {
                        for (menuIndex = 0; menuIndex < 32; ++menuIndex)
                        {
                            menuName = CL_GetConfigString(0, menuIndex + 1970);
                            if (*menuName)
                            {
                                if (!I_stricmp(parentMenuName, menuName))
                                    break;
                            }
                        }
                        if (menuIndex == 32)
                            menuIndex = -1;
                        Int = Dvar_GetInt("sv_serverId");
                        v1 = va("cmd mr %i %i %s\n", Int, menuIndex, menuResponse);
                        Cbuf_AddText(0, v1);
                    }
                }
            }
        }
    }
    else
    {
        Com_Printf(0, "USAGE: openscriptmenu <parent menu name> <script menu response>\n");
        Com_Printf(0, "EXAMPLE: openscriptmenu ingame changeweapon\n");
    }
}

void __cdecl COM_WriteFinalStringEdFile(char *fromOSPath, char *toOSPath)
{
    uint8_t *buf; // [esp+0h] [ebp-Ch]
    int32_t len; // [esp+4h] [ebp-8h]
    FILE *f; // [esp+8h] [ebp-4h]
    FILE *fa; // [esp+8h] [ebp-4h]

    f = FS_FileOpenReadBinary(fromOSPath);
    if (f)
    {
        len = FS_FileGetFileSize(f);
        buf = (unsigned char*)malloc(len);
        if (FS_FileRead(buf, len, f) != len)
            Com_Error(ERR_FATAL, "Short read in COM_WriteFinalStringEdFile()");
        FS_FileClose(f);
        fa = FS_FileOpenWriteBinary(toOSPath);
        if (fa)
        {
            if (FS_FileWrite(buf, len, fa) != len)
                Com_Error(ERR_FATAL, "Short write in COM_WriteFinalStringEdFile()");
            FS_FileClose(fa);
            free(buf);
        }
        else
        {
            free(buf);
        }
    }
}

void __cdecl Com_WriteLocalizedSoundAliasFiles()
{
    char stringEdFileName[256]; // [esp+10h] [ebp-218h] BYREF
    int32_t mark; // [esp+110h] [ebp-118h]
    const char **fileNames; // [esp+114h] [ebp-114h]
    char stringEdExternalFileName[256]; // [esp+118h] [ebp-110h] BYREF
    FILE *f; // [esp+21Ch] [ebp-Ch]
    int32_t i; // [esp+220h] [ebp-8h]
    int32_t fileCount; // [esp+224h] [ebp-4h] BYREF

    FS_BuildOSPath(
        (char*)fs_homepath->current.integer,
        (char*)"../source_data/string_resources/subtitle.st",
        (char*)"",
        stringEdExternalFileName);
    stringEdExternalFileName[&stringEdExternalFileName[strlen(stringEdExternalFileName) + 1]
        - &stringEdExternalFileName[1]
        - 1] = 0;
    f = fopen(stringEdExternalFileName, "r+");
    if (f)
    {
        fclose(f);
        FS_BuildOSPath((char*)fs_basepath->current.integer, fs_gamedir, (char*)"soundaliases/subtitle.st", stringEdFileName);
        FS_CopyFile(stringEdExternalFileName, stringEdFileName);
        if (FS_FileExists((char*)"soundaliases/subtitle.st"))
        {
            Com_Printf(9, "Localizing sound alias subtitle text...\n");
            Com_Printf(9, "Writing to StringEd file %s\n", stringEdExternalFileName);
            fileNames = FS_ListFiles("soundaliases", "csv", FS_LIST_PURE_ONLY, &fileCount);
            if (fileCount)
            {
                mark = Hunk_HideTempMemory();
                for (i = 0; i < fileCount; ++i)
                {
                    Com_ProcessSoundAliasFileLocalization((char*)fileNames[i], (char*)"all_mp");
                    Hunk_ClearTempMemory();
                }
                Hunk_ShowTempMemory(mark);
                FS_FreeFileList(fileNames);
                COM_WriteFinalStringEdFile(stringEdFileName, stringEdExternalFileName);
                FS_Remove(stringEdFileName);
                Com_Printf(9, "done\n");
            }
            else
            {
                Com_PrintWarning(9, "WARNING: can't find any sound alias files (soundaliases/*.csv)\n");
            }
        }
        else
        {
            Com_PrintWarning(9, "WARNING: Could not make local copy of StringEd file %s\n", "soundaliases/subtitle.st");
        }
    }
    else
    {
        Com_PrintWarning(9, "WARNING: Can not write to StringEd file %s\n", stringEdExternalFileName);
    }
}

void __cdecl CL_CheckAutoUpdate()
{
    __int16 v0; // ax
    const char *v1; // eax
    int32_t rnd; // [esp+0h] [ebp-3Ch]
    netadr_t temp; // [esp+4h] [ebp-38h] BYREF
    const char *servername; // [esp+1Ch] [ebp-20h]
    int32_t i; // [esp+20h] [ebp-1Ch]
    int32_t validServerNum; // [esp+24h] [ebp-18h]
    const char *pszGoodServers[5]; // [esp+28h] [ebp-14h]

    validServerNum = 0;
    i = 0;
    if (!autoupdateChecked)
    {
        for (i = 0; i < 5; ++i)
        {
            if (NET_StringToAdr(cls.autoupdateServerNames[i], &temp))
                pszGoodServers[validServerNum++] = cls.autoupdateServerNames[i];
        }
        if (validServerNum)
        {
            rnd = rand() % validServerNum;
            servername = pszGoodServers[rnd];
            Com_DPrintf(14, "Resolving AutoUpdate Server... ");
            if (NET_StringToAdr((char*)servername, &cls.autoupdateServer))
                goto LABEL_17;
            Com_DPrintf(14, "\nCouldn't resolve first address, trying others... ");
            for (i = 1; i < validServerNum; ++i)
            {
                servername = pszGoodServers[(i + rnd) % validServerNum];
                if (NET_StringToAdr((char *)servername, &cls.autoupdateServer))
                {
                    Com_DPrintf(14, "\nAlternate server address resolved... ");
                    break;
                }
            }
            if (i != validServerNum)
            {
            LABEL_17:
                cls.autoupdateServer.port = BigShort(28960);
                v0 = BigShort(cls.autoupdateServer.port);
                Com_DPrintf(
                    14,
                    "%i.%i.%i.%i:%i\n",
                    cls.autoupdateServer.ip[0],
                    cls.autoupdateServer.ip[1],
                    cls.autoupdateServer.ip[2],
                    cls.autoupdateServer.ip[3],
                    v0);
                v1 = va("getUpdateInfo2 \"%s\" \"%s\" \"%s\"\n", "CoD4 MP", "1.0", "win-x86");
                NET_OutOfBandPrint(NS_CLIENT1, cls.autoupdateServer, v1);
                autoupdateChecked = 1;
            }
            else
            {
                Com_DPrintf(14, "\nFailed to resolve any Auto-update servers.\n");
                autoupdateChecked = 1;
            }
        }
        else
        {
            Com_DPrintf(14, "Couldn't resolve an AutoUpdate Server address.\n");
            autoupdateChecked = 1;
        }
    }
}

serverStatus_s *__cdecl CL_GetServerStatus(netadr_t from)
{
    int32_t oldest; // [esp+0h] [ebp-10h]
    int32_t i; // [esp+4h] [ebp-Ch]
    int32_t ia; // [esp+4h] [ebp-Ch]
    int32_t ib; // [esp+4h] [ebp-Ch]
    int32_t oldestTime; // [esp+8h] [ebp-8h]

    for (i = 0; i < 16; ++i)
    {
        if (NET_CompareAdr(from, cl_serverStatusList[i].address))
            return &cl_serverStatusList[i];
    }
    for (ia = 0; ia < 16; ++ia)
    {
        if (cl_serverStatusList[ia].retrieved)
            return &cl_serverStatusList[ia];
    }
    oldest = -1;
    oldestTime = 0;
    for (ib = 0; ib < 16; ++ib)
    {
        if (oldest == -1 || cl_serverStatusList[ib].startTime < oldestTime)
        {
            oldest = ib;
            oldestTime = cl_serverStatusList[ib].startTime;
        }
    }
    if (oldest == -1)
        return &cl_serverStatusList[++serverStatusCount & 0xF];
    else
        return &cl_serverStatusList[oldest];
}

void __cdecl CL_ServerStatus_f()
{
    connstate_t connstate; // [esp+0h] [ebp-28h]
    clientConnection_t *clc; // [esp+4h] [ebp-24h]
    serverStatus_s *serverStatus; // [esp+8h] [ebp-20h]
    netadr_t to; // [esp+Ch] [ebp-1Ch] BYREF
    const char *server; // [esp+24h] [ebp-4h]

    Com_Memset(&to, 0, 20);
    if (Cmd_Argc() == 2)
    {
        server = Cmd_Argv(1);
    }
    else
    {
        connstate = clientUIActives[0].connectionState;
        clc = CL_GetLocalClientConnection(0);
        if (connstate != CA_ACTIVE || clc->demoplaying)
        {
            Com_Printf(0, "Not connected to a server.\n");
            Com_Printf(0, "Usage: serverstatus [server]\n");
            return;
        }
        server = cls.servername;
    }
    if (NET_StringToAdr((char*)server, &to))
    {
        NET_OutOfBandPrint(NS_CLIENT1, to, "getstatus");
        serverStatus = CL_GetServerStatus(to);
        serverStatus->address = to;
        serverStatus->print = 1;
        serverStatus->pending = 1;
    }
}

void __cdecl CL_InitOnceForAllClients()
{
    DWORD v0; // eax
    DvarLimits min; // [esp+4h] [ebp-18h]
    DvarLimits mina; // [esp+4h] [ebp-18h]
    DvarLimits minb; // [esp+4h] [ebp-18h]
    DvarLimits minc; // [esp+4h] [ebp-18h]
    DvarLimits mind; // [esp+4h] [ebp-18h]
    DvarLimits mine; // [esp+4h] [ebp-18h]
    DvarLimits minf; // [esp+4h] [ebp-18h]
    DvarLimits ming; // [esp+4h] [ebp-18h]
    DvarLimits minh; // [esp+4h] [ebp-18h]
    DvarLimits mini; // [esp+4h] [ebp-18h]
    DvarLimits minj; // [esp+4h] [ebp-18h]
    DvarLimits mink; // [esp+4h] [ebp-18h]
    DvarLimits minl; // [esp+4h] [ebp-18h]
    int32_t i; // [esp+18h] [ebp-4h]

    v0 = Sys_MillisecondsRaw();
    srand(v0);
    Con_Init();
    CL_InitInput();
    cl_noprint = Dvar_RegisterBool("cl_noprint", false, DVAR_NOFLAG, "Print nothing to the console");
    for (i = 0; i < 5; ++i)
        customclass[i] = Dvar_RegisterString(customClassDvars[i], "", 1u, "Custom class name");
    onlinegame = Dvar_RegisterBool(
        "onlinegame",
        true,
        DVAR_CHEAT,
        "Current game is an online game with stats, custom classes, unlocks");
    Dvar_SetBool(onlinegame, 1);
    cl_hudDrawsBehindUI = Dvar_RegisterBool("cl_hudDrawsBehindUI", true, DVAR_NOFLAG, "Should the HUD draw when the UI is up?");
    cl_voice = Dvar_RegisterBool("cl_voice", true, DVAR_ARCHIVE | DVAR_USERINFO, "Use voice communications");
    min.value.max = 3600.0;
    min.value.min = 0.0;
    cl_timeout = Dvar_RegisterFloat("cl_timeout", 40.0, min, DVAR_NOFLAG, "Seconds with no received packets until a timeout occurs");
    mina.value.max = 3600.0;
    mina.value.min = 0.0;
    cl_connectTimeout = Dvar_RegisterFloat(
        "cl_connectTimeout",
        200.0,
        mina,
        DVAR_NOFLAG,
        "Timeout time in seconds while connecting to a server");
    cl_connectionAttempts = Dvar_RegisterInt(
        "cl_connectionAttempts",
        10,
        (DvarLimits)0x7FFFFFFF00000000LL,
        DVAR_NOFLAG,
        "Maximum number of connection attempts before aborting");
    cl_shownet = Dvar_RegisterInt("cl_shownet", 0, (DvarLimits)0x4FFFFFFFELL, DVAR_NOFLAG, "Display network debugging information");
    cl_shownuments = Dvar_RegisterBool("cl_shownuments", false, DVAR_NOFLAG, "Show the number of entities");
    cl_showServerCommands = Dvar_RegisterBool(
        "cl_showServerCommands",
        0,
        DVAR_NOFLAG,
        "Enable debugging information for server commands");
    cl_showSend = Dvar_RegisterBool("cl_showSend", false, DVAR_NOFLAG, "Enable debugging information for sent commands");
    cl_showTimeDelta = Dvar_RegisterBool("cl_showTimeDelta", false, DVAR_NOFLAG, "Enable debugging information for time delta");
    cl_freezeDemo = Dvar_RegisterBool(
        "cl_freezeDemo",
        false, 
        DVAR_NOFLAG,
        "cl_freezeDemo is used to lock a demo in place for single frame advances");
    cl_activeAction = Dvar_RegisterString("activeAction", "", DVAR_NOFLAG, "Action to execute in first frame");
    cl_avidemo = Dvar_RegisterInt("cl_avidemo", 0, (DvarLimits)0x7FFFFFFF00000000LL, DVAR_NOFLAG, "AVI demo frames per second");
    cl_forceavidemo = Dvar_RegisterBool("cl_forceavidemo", false, DVAR_NOFLAG, "Record AVI demo even if client is not active");
    minb.value.max = FLT_MAX;
    minb.value.min = -FLT_MAX;
    cl_yawspeed = Dvar_RegisterFloat("cl_yawspeed", 140.0, minb, DVAR_ARCHIVE, "Max yaw speed in degrees for game pad and keyboard");
    minc.value.max = FLT_MAX;
    minc.value.min = -FLT_MAX;
    cl_pitchspeed = Dvar_RegisterFloat("cl_pitchspeed", 140.0, minc, DVAR_ARCHIVE, "Max pitch speed in degrees for game pad");
    mind.value.max = FLT_MAX;
    mind.value.min = 0.0;
    cl_anglespeedkey = Dvar_RegisterFloat(
        "cl_anglespeedkey",
        1.5,
        mind,
        DVAR_NOFLAG,
        "Multiplier for max angle speed for game pad and keyboard");
    cl_maxpackets = Dvar_RegisterInt(
        "cl_maxpackets",
        30,
        (DvarLimits)0x640000000FLL,
        DVAR_ARCHIVE,
        "Maximum number of packets sent per frame");
    cl_packetdup = Dvar_RegisterInt("cl_packetdup", 1, (DvarLimits)0x500000000LL, DVAR_ARCHIVE, "Enable packet duplication");
    mine.value.max = 100.0f;
    mine.value.min = 0.0099999998f;
    cl_sensitivity = Dvar_RegisterFloat("sensitivity", 5.0f, mine, DVAR_ARCHIVE, "Mouse sensitivity");
    minf.value.max = 100.0f;
    minf.value.min = 0.0f;
    cl_mouseAccel = Dvar_RegisterFloat("cl_mouseAccel", 0.0f, minf, DVAR_ARCHIVE, "Mouse acceleration");
    cl_freelook = Dvar_RegisterBool("cl_freelook", true, DVAR_ARCHIVE, "Enable looking with mouse");
    cl_showMouseRate = Dvar_RegisterBool(
        "cl_showmouserate",
        false,
        DVAR_NOFLAG,
        "Print mouse rate debugging information to the console");
    cl_allowDownload = Dvar_RegisterBool("cl_allowDownload", true, DVAR_ARCHIVE, "Allow client downloads from the server");
    cl_wwwDownload = Dvar_RegisterBool("cl_wwwDownload", true, DVAR_ARCHIVE | DVAR_USERINFO, "Download files via HTTP");
    cl_talking = Dvar_RegisterBool("cl_talking", false, DVAR_NOFLAG, "Client is talking");
    cl_inGameVideo = Dvar_RegisterBool("r_inGameVideo", true, DVAR_ARCHIVE, "Allow in game cinematics");
    cl_serverStatusResendTime = Dvar_RegisterInt(
        "cl_serverStatusResendTime",
        750,
        (DvarLimits)0xE1000000000LL,
        DVAR_NOFLAG,
        "Time in milliseconds to resend a server status message");
    cl_bypassMouseInput = Dvar_RegisterBool(
        "cl_bypassMouseInput",
        0,
        DVAR_NOFLAG,
        "Bypass UI mouse input and send directly to the game");
    ming.value.max = 1.0f;
    ming.value.min = -1.0f;
    m_pitch = Dvar_RegisterFloat("m_pitch", 0.022f, ming, DVAR_ARCHIVE, "Default pitch");
    minh.value.max = 1.0f;
    minh.value.min = -1.0f;
    m_yaw = Dvar_RegisterFloat("m_yaw", 0.022f, minh, DVAR_ARCHIVE, "Default yaw");
    mini.value.max = 1.0f;
    mini.value.min = -1.0f;
    m_forward = Dvar_RegisterFloat("m_forward", 0.25f, mini, DVAR_ARCHIVE, "Forward speed in units per second");
    minj.value.max = 1.0f;
    minj.value.min = -1.0f;
    m_side = Dvar_RegisterFloat("m_side", 0.25f, minj, DVAR_ARCHIVE, "Sideways motion in units per second");
    m_filter = Dvar_RegisterBool("m_filter", false, DVAR_ARCHIVE, "Allow mouse movement smoothing");
    cl_motdString = Dvar_RegisterString("cl_motdString", "", DVAR_ROM, "Message of the day");
    cl_ingame = Dvar_RegisterBool("cl_ingame", 0, DVAR_ROM, "True if the game is active");
    Dvar_RegisterInt("cl_maxPing", 800, (DvarLimits)0x7D000000014LL, DVAR_ARCHIVE, "Maximum ping for the client");
    cl_profileTextHeight = Dvar_RegisterInt(
        "cl_profileTextHeight",
        19,
        (DvarLimits)0x6400000001LL,
        DVAR_NOFLAG,
        "Text size to draw the network profile data");
    cl_profileTextY = Dvar_RegisterInt(
        "cl_profileTextY",
        110,
        (DvarLimits)0x32000000000LL,
        DVAR_NOFLAG,
        "Y position to draw the profile data");
    name = Dvar_RegisterString("name", "", 3u, "Player name");
    Dvar_RegisterInt("rate", 25000, (DvarLimits)0x61A8000003E8LL, DVAR_ARCHIVE | DVAR_USERINFO, "Player's preferred baud rate");
    Dvar_RegisterInt("snaps", 20, (DvarLimits)0x1E00000001LL, DVAR_ARCHIVE | DVAR_USERINFO, "Snapshot rate");
    Dvar_RegisterBool("cl_punkbuster", 1, DVAR_ARCHIVE | DVAR_USERINFO | DVAR_INIT, "Determines whether PunkBuster is enabled");
    Dvar_RegisterString("password", "", DVAR_USERINFO, "password");
    nextdemo = Dvar_RegisterString("nextdemo", "", DVAR_NOFLAG, "The next demo to play");
    Dvar_RegisterBool("hud_enable", 1, DVAR_ARCHIVE, "Enable the HUD display");
    Dvar_RegisterBool("cg_blood", 1, DVAR_ARCHIVE, "Show blood");
    cl_updateavailable = Dvar_RegisterBool("cl_updateavailable", false, DVAR_ROM, "True if there is an available update");
    cl_updatefiles = Dvar_RegisterString("cl_updatefiles", "", DVAR_ROM, "The file that is being updated");
    cl_updateoldversion = Dvar_RegisterString("cl_updateoldversion", "", DVAR_ROM, "The version before update");
    cl_updateversion = Dvar_RegisterString("cl_updateversion", "", DVAR_ROM, "The updated version");
    cl_debugMessageKey = Dvar_RegisterBool("cl_debugMessageKey", false, DVAR_NOFLAG, "Enable message key debugging information");
    I_strncpyz(cls.autoupdateServerNames[0], "cod2update.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[1], "cod2update2.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[2], "cod2update3.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[3], "cod2update4.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[4], "cod2update5.activision.com", 64);
    motd = Dvar_RegisterString("motd", "", DVAR_NOFLAG, "Message of the day");
    mink.value.max = 80.0f;
    mink.value.min = -80.0f;
    vehDriverViewHeightMin = Dvar_RegisterFloat(
        "vehDriverViewHeightMin",
        -15.0f,
        mink,
        DVAR_ARCHIVE,
        "Min orbit altitude for driver's view");
    minl.value.max = 80.0f;
    minl.value.min = -80.0f;
    vehDriverViewHeightMax = Dvar_RegisterFloat(
        "vehDriverViewHeightMax",
        50.0f,
        minl,
        DVAR_ARCHIVE,
        "Max orbit altitude for driver's view");
    Cmd_AddCommandInternal("cmd", CL_ForwardToServer_f, &CL_ForwardToServer_f_VAR);
    Cmd_AddCommandInternal("configstrings", CL_Configstrings_f, &CL_Configstrings_f_VAR);
    Cmd_AddCommandInternal("clientinfo", CL_Clientinfo_f, &CL_Clientinfo_f_VAR);
    Cmd_AddCommandInternal("vid_restart", Cbuf_AddServerText_f, &CL_Vid_Restart_f_VAR);
    Cmd_AddServerCommandInternal("vid_restart", CL_Vid_Restart_f, &CL_Vid_Restart_f_VAR_SERVER);
    Cmd_AddCommandInternal("snd_restart", Cbuf_AddServerText_f, &CL_Snd_Restart_f_VAR);
    Cmd_AddServerCommandInternal("snd_restart", CL_Snd_Restart_f, &CL_Snd_Restart_f_VAR_SERVER);
    Cmd_AddCommandInternal("disconnect", Cbuf_AddServerText_f, &CL_Disconnect_f_VAR);
    Cmd_AddServerCommandInternal("disconnect", CL_Disconnect_f, &CL_Disconnect_f_VAR_SERVER);
    Cmd_AddCommandInternal("record", CL_Record_f, &CL_Record_f_VAR);
    Cmd_AddCommandInternal("stoprecord", CL_StopRecord_f, &CL_StopRecord_f_VAR);
    Cmd_AddCommandInternal("demo", Cbuf_AddServerText_f, &CL_PlayDemo_f_VAR_0);
    Cmd_AddServerCommandInternal("demo", CL_PlayDemo_f, &CL_PlayDemo_f_VAR_SERVER_0);
    Cmd_AddCommandInternal("timedemo", Cbuf_AddServerText_f, &CL_PlayDemo_f_VAR);
    Cmd_AddServerCommandInternal("timedemo", CL_PlayDemo_f, &CL_PlayDemo_f_VAR_SERVER);
    Cmd_SetAutoComplete("demo", "demos", "dm_1");
    Cmd_SetAutoComplete("timedemo", "demos", "dm_1");
    Cmd_AddCommandInternal("cinematic", CL_PlayCinematic_f, &CL_PlayCinematic_f_VAR);
    Cmd_AddCommandInternal("unskippablecinematic", CL_PlayUnskippableCinematic_f, &CL_PlayUnskippableCinematic_f_VAR);
    Cmd_SetAutoComplete("cinematic", "video", "roq");
    Cmd_AddCommandInternal("logo", CL_PlayLogo_f, &CL_PlayLogo_f_VAR);
    Cmd_AddCommandInternal("connect", Cbuf_AddServerText_f, &CL_Connect_f_VAR);
    Cmd_AddServerCommandInternal("connect", CL_Connect_f, &CL_Connect_f_VAR_SERVER);
    Cmd_AddCommandInternal("reconnect", Cbuf_AddServerText_f, &CL_Reconnect_f_VAR);
    Cmd_AddServerCommandInternal("reconnect", CL_Reconnect_f, &CL_Reconnect_f_VAR_SERVER);
    Cmd_AddCommandInternal("localservers", CL_LocalServers_f, &CL_LocalServers_f_VAR);
    Cmd_AddCommandInternal("globalservers", CL_GlobalServers_f, &CL_GlobalServers_f_VAR);
    CL_RconInit();
    Cmd_AddCommandInternal("rcon", CL_Rcon_f, &CL_Rcon_f_VAR);
    Cmd_AddCommandInternal("ping", Cbuf_AddServerText_f, &CL_Ping_f_VAR);
    Cmd_AddServerCommandInternal("ping", CL_Ping_f, &CL_Ping_f_VAR_SERVER);
    Cmd_AddCommandInternal("serverstatus", Cbuf_AddServerText_f, &CL_ServerStatus_f_VAR);
    Cmd_AddServerCommandInternal("serverstatus", CL_ServerStatus_f, &CL_ServerStatus_f_VAR_SERVER);
    Cmd_AddCommandInternal("setenv", CL_Setenv_f, &CL_Setenv_f_VAR);
    Cmd_AddCommandInternal("showip", CL_ShowIP_f, &CL_ShowIP_f_VAR);
    Cmd_AddCommandInternal("toggleMenu", CL_ToggleMenu_f, &CL_ToggleMenu_f_VAR);
    Cmd_AddCommandInternal("fs_openedList", CL_OpenedIWDList_f, &CL_OpenedIWDList_f_VAR);
    Cmd_AddCommandInternal("fs_referencedList", CL_ReferencedIWDList_f, &CL_ReferencedIWDList_f_VAR);
    Cmd_AddCommandInternal("updatehunkusage", CL_UpdateLevelHunkUsage, &CL_UpdateLevelHunkUsage_VAR);
    Cmd_AddCommandInternal("startSingleplayer", CL_startSingleplayer_f, &CL_startSingleplayer_f_VAR);
    Cmd_AddCommandInternal("parseBadPacket", CL_ParseBadPacket_f, &CL_ParseBadPacket_f_VAR);
    Cmd_AddCommandInternal("cubemapShot", CL_CubemapShot_f, &CL_CubemapShot_f_VAR);
    Cmd_AddCommandInternal("openScriptMenu", CL_OpenScriptMenu_f, &CL_OpenScriptMenu_f_VAR);
    Cmd_AddCommandInternal(
        "localizeSoundAliasFiles",
        Com_WriteLocalizedSoundAliasFiles,
        &Com_WriteLocalizedSoundAliasFiles_VAR);
    Cmd_AddCommandInternal("openmenu", UI_OpenMenu_f, &UI_OpenMenu_f_VAR);
    Cmd_AddCommandInternal("closemenu", UI_CloseMenu_f, &UI_CloseMenu_f_VAR);
    Cmd_AddCommandInternal("listmenus", UI_ListMenus_f, &UI_ListMenus_f_VAR);
    Cmd_AddCommandInternal(
        "selectStringTableEntryInDvar",
        CL_SelectStringTableEntryInDvar_f,
        &CL_SelectStringTableEntryInDvar_f_VAR);
    Cmd_AddCommandInternal("resetStats", KISAK_NULLSUB, &KISAK_NULLSUB_VAR);
    autoupdateChecked = 0;
    autoupdateStarted = 0;
    CL_CheckAutoUpdate();
    CL_InitRef();
    SCR_Init();
    CG_RegisterDvars();
    Ragdoll_Register();
    cl_voiceCommunication.voicePacketCount = 0;
}

void __cdecl CL_Disconnect_f()
{
    CL_DisconnectLocalClient(0);
}

void __cdecl CL_Init(int32_t localClientNum)
{
    int32_t v1; // eax

    Com_Printf(14, "----- Client Initialization -----\n");
    CL_ClearState(localClientNum);
    // LWSS: functionality unknown! 
    //if (CountBitsEnabled(0xFFFFFFFF) != 32)
    //    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5206, 0, "%s", "CountBitsEnabled( 0xffffffff ) == 32");
    //if (CountBitsEnabled(0))
    //    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5207, 0, "%s", "CountBitsEnabled( 0x00000000 ) == 0");
    //if (CountBitsEnabled(0x11111111u) != 8)
    //    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5208, 0, "%s", "CountBitsEnabled( 0x11111111 ) == 8");
    //if (CountBitsEnabled(0x77777777u) != 24)
    //    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5209, 0, "%s", "CountBitsEnabled( 0x77777777 ) == 24");
    CL_ClearMutedList();
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActives[0].connectionState = CA_DISCONNECTED;
    cls.realtime = 0;
    clientUIActives[0].active = 1;
    cl_serverLoadingMap = 0;
    cl_waitingOnServerToLoadMap[localClientNum] = 0;
    g_waitingForServer = 0;
    v1 = CL_ControllerIndexFromClientNum(localClientNum);
    Cbuf_Execute(localClientNum, v1);
    clientUIActives[0].isRunning = 1;
    Com_Printf(14, "----- Client Initialization Complete -----\n");
}

// LWSS: I dont see the point of this function?
//int32_t __cdecl CountBitsEnabled(uint32_t num)
//{
//    uint32_t numa; // [esp+1Ch] [ebp+8h]
//    uint32_t numb; // [esp+1Ch] [ebp+8h]
//
//    numa = (((((num >> 1) & 0x55555555) + (num & 0x55555555)) >> 2) & 0x33333333)
//        + ((((num >> 1) & 0x55555555) + (num & 0x55555555)) & 0x33333333);
//    numb = (((((numa >> 4) & 0xF0F0F0F) + (numa & 0xF0F0F0F)) >> 8) & 0xFF00FF)
//        + ((((numa >> 4) & 0xF0F0F0F) + (numa & 0xF0F0F0F)) & 0xFF00FF);
//    return HIWORD(numb) + numb;
//}

int32_t recursive;
void __cdecl CL_Shutdown(int32_t localClientNum)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5246, 0, "%s", "Sys_IsMainThread()");
    Com_SyncThreads();
    Com_Printf(14, "----- CL_Shutdown -----\n");
    if (recursive)
    {
        printf("recursive shutdown\n");
    }
    else
    {
        recursive = 1;
        CL_Disconnect(localClientNum);
        if (CL_AllLocalClientsDisconnected())
        {
            CL_ShutdownDebugData();
            CL_ShutdownHunkUsers();
            SND_Shutdown();
            CL_ShutdownInput();
            Cmd_RemoveCommand("cmd");
            Cmd_RemoveCommand("configstrings");
            Cmd_RemoveCommand("clientinfo");
            Cmd_RemoveCommand("vid_restart");
            Cmd_RemoveCommand("snd_restart");
            Cmd_RemoveCommand("disconnect");
            Cmd_RemoveCommand("record");
            Cmd_RemoveCommand("demo");
            Cmd_RemoveCommand("cinematic");
            Cmd_RemoveCommand("logo");
            Cmd_RemoveCommand("stoprecord");
            Cmd_RemoveCommand("connect");
            Cmd_RemoveCommand("reconnect");
            Cmd_RemoveCommand("localservers");
            Cmd_RemoveCommand("globalservers");
            Cmd_RemoveCommand("rcon");
            Cmd_RemoveCommand("setenv");
            Cmd_RemoveCommand("ping");
            Cmd_RemoveCommand("serverstatus");
            Cmd_RemoveCommand("showip");
            Cmd_RemoveCommand("fs_openedList");
            Cmd_RemoveCommand("fs_referencedList");
            Cmd_RemoveCommand("updatehunkusage");
            Cmd_RemoveCommand("SaveTranslations");
            Cmd_RemoveCommand("SaveNewTranslations");
            Cmd_RemoveCommand("LoadTranslations");
            Cmd_RemoveCommand("startSingleplayer");
            Cmd_RemoveCommand("buyNow");
            Cmd_RemoveCommand("singlePlayLink");
            Cmd_RemoveCommand("setRecommended");
            Cmd_RemoveCommand("cubemapShot");
            Cmd_RemoveCommand("openScriptMenu");
            Cmd_RemoveCommand("openmenu");
            Cmd_RemoveCommand("closemenu");
            memset((uint8_t *)&cls, 0, sizeof(cls));
        }
        clientUIActives[localClientNum].isRunning = 0;
        recursive = 0;
        Com_Printf(14, "-----------------------\n");
    }
}

void __cdecl CL_LocalServers_f()
{
    uint8_t b; // [esp+10h] [ebp-2Ch]
    int32_t j; // [esp+14h] [ebp-28h]
    int32_t i; // [esp+20h] [ebp-1Ch]
    int32_t ia; // [esp+20h] [ebp-1Ch]
    netadr_t to; // [esp+24h] [ebp-18h] BYREF

    Com_Printf(0, "Scanning for servers on the local network...\n");
    cls.numlocalservers = 0;
    cls.pingUpdateSource = 0;
    for (i = 0; i < 128; ++i)
    {
        b = cls.localServers[i].dirty;
        Com_Memset((uint32_t *)&cls.localServers[i], 0, 148);
        cls.localServers[i].dirty = b;
    }
    Com_Memset((uint32_t *)&to, 0, 20);
    for (ia = 0; ia < 2; ++ia)
    {
        for (j = 0; j < 4; ++j)
        {
            to.port = BigShort(j + 28960);
            to.type = NA_BROADCAST;
            NET_OutOfBandData(NS_CLIENT1, to, (const unsigned char*)"getinfo xxx", strlen("getinfo xxx"));
        }
    }
}

void __cdecl CL_GetPing(int32_t n, char *buf, int32_t buflen, int32_t *pingtime)
{
    const char *str; // [esp+0h] [ebp-Ch]
    int32_t time; // [esp+4h] [ebp-8h]
    int32_t maxPing; // [esp+8h] [ebp-4h]

    if (cl_pinglist[n].adr.port)
    {
        str = NET_AdrToString(cl_pinglist[n].adr);
        I_strncpyz(buf, str, buflen);
        time = cl_pinglist[n].time;
        if (!time)
        {
            time = Sys_Milliseconds() - cl_pinglist[n].start;
            maxPing = Dvar_GetInt("cl_maxPing");
            if (maxPing < 100)
                maxPing = 100;
            if (time < maxPing)
                time = 0;
        }
        CL_SetServerInfoByAddress(cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time);
        *pingtime = time;
    }
    else
    {
        *buf = 0;
        *pingtime = 0;
    }
}

void __cdecl CL_ClearPing(uint32_t n)
{
    if (n < 0x10)
        cl_pinglist[n].adr.port = 0;
}

int32_t __cdecl CL_GetPingQueueCount()
{
    int32_t i; // [esp+0h] [ebp-Ch]
    ping_t *pingptr; // [esp+4h] [ebp-8h]
    int32_t count; // [esp+8h] [ebp-4h]

    count = 0;
    pingptr = cl_pinglist;
    for (i = 0; i < 16; ++i)
    {
        if (pingptr->adr.port)
            ++count;
        ++pingptr;
    }
    return count;
}

int32_t __cdecl CL_UpdateDirtyPings(netsrc_t localClientNum, uint32_t source)
{
    serverInfo_t *v3; // edx
    ping_t *v4; // eax
    int32_t j; // [esp+4h] [ebp-420h]
    int32_t ja; // [esp+4h] [ebp-420h]
    serverInfo_t *server; // [esp+8h] [ebp-41Ch]
    int32_t max; // [esp+Ch] [ebp-418h]
    int32_t status; // [esp+10h] [ebp-414h]
    int32_t slots; // [esp+14h] [ebp-410h]
    int32_t i; // [esp+18h] [ebp-40Ch]
    int32_t ia; // [esp+18h] [ebp-40Ch]
    char buff[1024]; // [esp+1Ch] [ebp-408h] BYREF
    int32_t pingTime; // [esp+420h] [ebp-4h] BYREF

    status = 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState)
        return 0;
    if (source > 2)
        return 0;
    cls.pingUpdateSource = source;
    slots = CL_GetPingQueueCount();
    if (slots < 16)
    {
        if (source)
        {
            if (source == 1)
            {
                server = cls.globalServers;
                max = cls.numglobalservers;
            }
            else
            {
                server = cls.favoriteServers;
                max = cls.numfavoriteservers;
            }
        }
        else
        {
            server = cls.localServers;
            max = cls.numlocalservers;
        }
        if (!server)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5592, 0, "%s", "server != NULL");
        for (i = 0; i < max; ++i)
        {
            if (server[i].dirty && server[i].ping == -1)
            {
                if (slots >= 16)
                    break;
                for (j = 0; j < 16 && (!cl_pinglist[j].adr.port || !NET_CompareAdr(cl_pinglist[j].adr, server[i].adr)); ++j)
                    ;
                if (j >= 16)
                {
                    status = 1;
                    for (ja = 0; ja < 16 && cl_pinglist[ja].adr.port; ++ja)
                        ;
                    v3 = &server[i];
                    v4 = &cl_pinglist[ja];
                    v4->adr.type = v3->adr.type;
                    *(uint32_t *)v4->adr.ip = *(uint32_t *)v3->adr.ip;
                    *(uint32_t *)&v4->adr.port = *(uint32_t *)&v3->adr.port;
                    *(uint32_t *)&v4->adr.ipx[2] = *(uint32_t *)&v3->adr.ipx[2];
                    *(uint32_t *)&v4->adr.ipx[6] = *(uint32_t *)&v3->adr.ipx[6];
                    cl_pinglist[ja].start = Sys_Milliseconds();
                    cl_pinglist[ja].time = 0;
                    cl_pinglist[ja].info[0] = 0;
                    NET_OutOfBandPrint(localClientNum, cl_pinglist[ja].adr, "getinfo xxx");
                    ++slots;
                }
            }
        }
    }
    if (slots)
        status = 1;
    for (ia = 0; ia < 16; ++ia)
    {
        if (cl_pinglist[ia].adr.port)
        {
            CL_GetPing(ia, buff, 1024, &pingTime);
            if (pingTime)
            {
                CL_ClearPing(ia);
                status = 1;
            }
        }
    }
    return status;
}


void __cdecl CL_ShowIP_f()
{
    Sys_ShowIP();
}

void __cdecl CL_SetupForNewServerMap(char *pszMapName, char *pszGametype)
{
    int32_t localClientNum; // [esp+0h] [ebp-4h]

    Com_Printf(14, "Server changing map %s, gametype %s\n", pszMapName, pszGametype);
    if (!com_sv_running->current.enabled)
        Cbuf_ExecuteBuffer(0, 0, "selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
    if (!*pszMapName || !*pszGametype)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5786, 0, "%s", "pszMapName[0] && pszGametype[0]");
    cl_serverLoadingMap = 1;
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        cl_waitingOnServerToLoadMap[localClientNum] = 0;
    if (!com_sv_running->current.enabled)
    {
        com_expectedHunkUsage = 0;
        g_waitingForServer = 1;
        UI_SetMap((char *)"", (char *)"");
        LoadMapLoadscreen(pszMapName);
        UI_SetMap(pszMapName, pszGametype);
    }
    SCR_UpdateScreen();
}

bool __cdecl CL_IsServerLoadingMap()
{
    return cl_serverLoadingMap;
}

bool __cdecl CL_IsWaitingOnServerToLoadMap(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            5838,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    return cl_waitingOnServerToLoadMap[localClientNum];
}

void __cdecl CL_SetWaitingOnServerToLoadMap(int32_t localClientNum, bool waiting)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\client_mp\\cl_main_mp.cpp",
            5845,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    cl_waitingOnServerToLoadMap[localClientNum] = waiting;
}

void __cdecl CL_DrawTextPhysical(
    const char *text,
    int32_t maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int32_t style)
{
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style);
}

void __cdecl CL_DrawTextPhysicalWithEffects(
    const char *text,
    int32_t maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int32_t style,
    const float *glowColor,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int32_t fxBirthTime,
    int32_t fxLetterTime,
    int32_t fxDecayStartTime,
    int32_t fxDecayDuration)
{
    R_AddCmdDrawTextWithEffects(
        text,
        maxChars,
        font,
        x,
        y,
        xScale,
        yScale,
        0.0,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration);
}

void __cdecl CL_DrawText(
    const ScreenPlacement *scrPlace,
    const char *text,
    int32_t maxChars,
    Font_s *font,
    float x,
    float y,
    int32_t horzAlign,
    int32_t vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int32_t style)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style);
}

void __cdecl CL_DrawTextRotate(
    const ScreenPlacement *scrPlace,
    const char *text,
    int32_t maxChars,
    Font_s *font,
    float x,
    float y,
    float rotation,
    int32_t horzAlign,
    int32_t vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int32_t style)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, rotation, color, style);
}

void __cdecl CL_DrawTextPhysicalWithCursor(
    char *text,
    int32_t maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int32_t style,
    int32_t cursorPos,
    char cursor)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

void __cdecl CL_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    const char *text,
    int32_t maxChars,
    Font_s *font,
    float x,
    float y,
    int32_t horzAlign,
    int32_t vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int32_t style,
    int32_t cursorPos,
    char cursor)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

bool __cdecl CL_ShouldDisplayHud(int32_t localClientNum)
{
    if (cl_hudDrawsBehindUI->current.enabled)
        return 1;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return (clientUIActives[0].keyCatchers & 0x10) == 0 || clientUIActives[0].displayHUDWithKeycatchUI;
}

bool __cdecl CL_IsUIActive(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return (clientUIActives[0].keyCatchers & 0x10) != 0;
}

Font_s *__cdecl CL_RegisterFont(const char *fontName, int32_t imageTrack)
{
    return R_RegisterFont(fontName, imageTrack);
}

void __cdecl CL_UpdateSound()
{
    KISAK_NULLSUB();
    SND_PlayFXSounds();
    SND_UpdateLoopingSounds();
    SND_Update();
}

float (*__cdecl CL_GetMapCenter())[3]
{
    return (float (*)[3])cls.mapCenter;
}

int32_t __cdecl CL_GetLocalClientActiveCount()
{
    return 1;
}

void __cdecl CL_InitDedicated()
{
    GfxConfiguration config; // [esp+0h] [ebp-30h] BYREF

    if (!onlinegame)
        onlinegame = Dvar_RegisterBool(
            "onlinegame",
            1,
            DVAR_CHEAT,
            "Current game is an online game with stats, custom classes, unlocks");
    SetupGfxConfig(&config);
    CL_SetFastFileNames(&config, 1);
    R_MakeDedicated(&config);
    Sys_HideSplashWindow();
    Sys_ShowConsole();
    Sys_NormalExit();
}