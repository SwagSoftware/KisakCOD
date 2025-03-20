#include "client_mp.h"

#include <xanim/xanim.h>
#include <universal/assertive.h>
#include <qcommon/threads.h>
#include <qcommon/mem_track.h>

#define MAX_CLIENTS 1 // LWSS Add

//char const **customClassDvars 827b3184     cl_main_mp.obj
//struct dvar_s const *const cl_conXOffset 8287aa48     cl_main_mp.obj
//struct dvar_s const *const cl_hudDrawsBehindUI 8287aa4c     cl_main_mp.obj
//struct dvar_s const *const cl_showSend 8287aa50     cl_main_mp.obj
//struct dvar_s const *const input_invertPitch 8287aa54     cl_main_mp.obj
//struct clientConnection_t *clientConnections 8287aa58     cl_main_mp.obj
clientConnection_t clientConnections[MAX_CLIENTS];
//BOOL g_waitingForServer  8287aa5c     cl_main_mp.obj
//BOOL cl_serverLoadingMap 8287aa5d     cl_main_mp.obj
//struct voiceCommunication_t cl_voiceCommunication 8287aa60     cl_main_mp.obj
//struct dvar_s const *const cl_avidemo 8287aacc     cl_main_mp.obj
//struct dvar_s const *const cl_nodelta 8287aad0     cl_main_mp.obj
//struct dvar_s const *const cl_showServerCommands 8287aad4     cl_main_mp.obj
//struct dvar_s const *const motd       8287aad8     cl_main_mp.obj
//struct dvar_s const *const cl_connectTimeout 8287aadc     cl_main_mp.obj
//struct dvar_s const *const cl_sensitivity 8287aae0     cl_main_mp.obj
//struct dvar_s const *const cl_forceavidemo 8287aae4     cl_main_mp.obj
//struct dvar_s const *const cl_timeout 8287aae8     cl_main_mp.obj
//struct dvar_s const *const m_yaw      8287aaec     cl_main_mp.obj
//struct dvar_s const **customclass 8287aaf0     cl_main_mp.obj
//struct dvar_s const *const m_pitch    8287ab04     cl_main_mp.obj
//struct dvar_s const *const cl_activeAction 8287ab08     cl_main_mp.obj
//struct dvar_s const *const playlist   8287ab0c     cl_main_mp.obj
//struct dvar_s const *const cl_debugMessageKey 8287ab10     cl_main_mp.obj
//struct dvar_s const *const systemlink 8287ab14     cl_main_mp.obj
//struct dvar_s const *const nextdemo   8287ab18     cl_main_mp.obj
//int cl_maxLocalClients   8287ab1c     cl_main_mp.obj
//struct clientStatic_t cls  8287ab20     cl_main_mp.obj
static clientStatic_t cls;
//struct ping_t *cl_pinglist 8287bdc8     cl_main_mp.obj
//struct dvar_s const *const cl_connectionAttempts 82880148     cl_main_mp.obj
//int old_com_frameTime    8288014c     cl_main_mp.obj
//struct dvar_s const *const onlinegame 82880150     cl_main_mp.obj
//struct dvar_s const *const cl_showMouseRate 82880154     cl_main_mp.obj
//unsigned int frame_msec           82880158     cl_main_mp.obj
//struct dvar_s const *const m_forward  8288015c     cl_main_mp.obj
//struct dvar_s const *const cl_packetdup 82880160     cl_main_mp.obj
//struct dvar_s const *const cl_mouseAccel 82880164     cl_main_mp.obj
//struct dvar_s const *const cl_maxpackets 82880168     cl_main_mp.obj
//struct dvar_s const *const cl_motdString 8288016c     cl_main_mp.obj
//struct dvar_s const *const onlinegameandhost 82880170     cl_main_mp.obj
//struct dvar_s const *const cl_freezeDemo 82880174     cl_main_mp.obj
//struct dvar_s const *const cl_showTimeDelta 82880178     cl_main_mp.obj
//BOOL * cl_waitingOnServerToLoadMap 8288017c     cl_main_mp.obj
//struct dvar_s const *const input_viewSensitivity 82880180     cl_main_mp.obj
//struct dvar_s const *const input_autoAim 82880184     cl_main_mp.obj
//struct dvar_s const *const cl_ingame  82880188     cl_main_mp.obj
//struct dvar_s const *const cl_inGameVideo 8288018c     cl_main_mp.obj
//struct clientUIActive_t *clientUIActives 82880190     cl_main_mp.obj
clientUIActive_t clientUIActives[MAX_CLIENTS];
//struct dvar_s const *const name       82880220     cl_main_mp.obj
dvar_t *name;
//struct dvar_s const *const cl_noprint 82880224     cl_main_mp.obj
//struct dvar_s const *const m_side     82880228     cl_main_mp.obj
//struct dvar_s const *const cl_profileTextY 8288022c     cl_main_mp.obj
//struct dvar_s const *const cl_serverStatusResendTime 82880230     cl_main_mp.obj
//struct dvar_s const *const m_filter   82880234     cl_main_mp.obj
//struct dvar_s const *const cl_profileTextHeight 82880238     cl_main_mp.obj
//struct clientActive_t *clients 8288023c     cl_main_mp.obj
clientActive_t clients[MAX_CLIENTS];
//struct dvar_s const *const cl_shownuments 82880240     cl_main_mp.obj
//struct dvar_s const *const splitscreen 82880244     cl_main_mp.obj
//struct dvar_s const *const onlineunrankedgameandhost 8288024c     cl_main_mp.obj
//struct dvar_s const *const cl_freelook 82880250     cl_main_mp.obj
//struct dvar_s const *const cl_shownet 82880254     cl_main_mp.obj


//void __cdecl TRACK_cl_main()
//{
//    track_static_alloc_internal(clients, 1776604, "clients", 9);
//    track_static_alloc_internal(clientConnections, 398824, "clientConnections", 9);
//    track_static_alloc_internal(clientUIActives, 16, "clientUIActives", 9);
//    track_static_alloc_internal(&cls, 3002480, "cls", 9);
//}

void __cdecl TRACK_cl_main()
{
    track_static_alloc_internal(clients, sizeof(clientActive_t) * MAX_CLIENTS /*1776604*/, "clients", 9);
    track_static_alloc_internal(clientConnections, sizeof(clientConnection_t) * MAX_CLIENTS /*398824*/, "clientConnections", 9);
    track_static_alloc_internal(clientUIActives, sizeof(clientUIActive_t) * MAX_CLIENTS, "clientUIActives", 9);
    track_static_alloc_internal(&cls, sizeof(clientStatic_t)/*3002480*/, "cls", 9);
}

char __cdecl CL_IsLocalClientActive(int localClientNum)
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

int __cdecl CL_LocalActiveIndexFromClientNum(int localClientNum)
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

int __cdecl CL_ControllerIndexFromClientNum(int clientIndex)
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
    int client; // [esp+0h] [ebp-4h]

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
    int clientIndex; // [esp+0h] [ebp-4h]

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

void __cdecl CL_AddReliableCommand(int localClientNum, const char *cmd)
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
    int client; // [esp+0h] [ebp-4h]

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
    int localClientNum; // [esp+0h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        if (byte_E7A7C1[16 * localClientNum])
            return 1;
    }
    return 0;
}

void __cdecl CL_MapLoading(const char *mapname)
{
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-10h]
    int localClientNum; // [esp+10h] [ebp-Ch]
    int localClientNuma; // [esp+10h] [ebp-Ch]
    netsrc_t localClientNumb; // [esp+10h] [ebp-Ch]
    clientConnection_t *clc; // [esp+14h] [ebp-8h]
    clientConnection_t *clca; // [esp+14h] [ebp-8h]

    if (CL_AnyLocalClientsRunning())
    {
        g_waitingForServer = 0;
        for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        {
            Con_Close(localClientNum);
            dword_E7A7C4[4 * localClientNum] = 0;
            byte_E7A7C8[16 * localClientNum] = 0;
        }
        LiveStorage_UploadStats();
        UI_CloseAllMenus(0);
        cl_serverLoadingMap = 1;
        if (!com_sv_running->current.enabled)
            Cbuf_ExecuteBuffer(0, 0, "selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
        if (client_state[0] >= 5 && !I_stricmp(cls.servername, "localhost"))
        {
            memset((unsigned __int8 *)cls.updateInfoString, 0, sizeof(cls.updateInfoString));
            for (localClientNuma = 0; localClientNuma < 1; ++localClientNuma)
            {
                if (CL_IsLocalClientActive(localClientNuma))
                {
                    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNuma);
                    clc = CL_GetLocalClientConnection(localClientNuma);
                    client_state[4 * localClientNuma] = 5;
                    memset((unsigned __int8 *)clc->serverMessage, 0, sizeof(clc->serverMessage));
                    memset((unsigned __int8 *)&LocalClientGlobals->gameState, 0, sizeof(LocalClientGlobals->gameState));
                    clc->lastPacketSentTime = -9999;
                    if (!*mapname)
                        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 1463, 0, "%s", "mapname[0]");
                    cl_waitingOnServerToLoadMap[localClientNuma] = 0;
                }
            }
        }
        else
        {
            Dvar_SetString((dvar_s *)nextmap, (char *)&String);
            I_strncpyz(cls.servername, "localhost", 256);
            for (localClientNumb = NS_CLIENT1; localClientNumb < NS_SERVER; ++localClientNumb)
            {
                if (CL_IsLocalClientActive(localClientNumb))
                {
                    CL_Disconnect(localClientNumb);
                    CL_ResetStats_f();
                    UI_CloseAll(localClientNumb);
                    client_state[4 * localClientNumb] = 4;
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

void __cdecl CL_ResetSkeletonCache(int localClientNum)
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
    v1->skelMemoryStart = (char *)((unsigned int)&v1->skelMemory[15] & 0xFFFFFFF0);
    v1->skelMemPos = 0;
}

void __cdecl CL_ClearState(int localClientNum)
{
    clientActive_t *dst; // [esp+0h] [ebp-4h]

    if (localClientNum < 1)
    {
        dst = CL_GetLocalClientGlobals(localClientNum);
        memset((unsigned __int8 *)dst, 0, sizeof(clientActive_t));
    }
    Com_ClientDObjClearAllSkel(localClientNum);
}

void __cdecl CL_Disconnect(int localClientNum)
{
    int v1; // eax
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
        connstate = client_state[0];
        if (client_state[0] < 5)
            clc = 0;
        else
            clc = CL_GetLocalClientConnection(localClientNum);
        if (connstate >= CA_CONNECTED && clc->demorecording)
        {
            v1 = CL_ControllerIndexFromClientNum(localClientNum);
            Cmd_ExecuteSingleCommand(localClientNum, v1, "stoprecord");
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
            memset((unsigned __int8 *)clc, 0, sizeof(clientConnection_t));
        client_state[4 * localClientNum] = 0;
        if (!cls.wwwDlDisconnected)
            CL_ClearStaticDownload();
        DynEntCl_Shutdown(localClientNum);
        SND_DisconnectListener(localClientNum);
        if (connstate >= CA_CONNECTING)
            clientUIActives[0].keyCatchers &= 1u;
        CL_ResetStats_f();
        if (CL_AllLocalClientsDisconnected())
        {
            autoupdateStarted = 0;
            autoupdateFilename[0] = 0;
            Dvar_SetBool(sv_disableClientConsole, 0);
            cl_connectedToPureServer = 0;
            fs_checksumFeed = 0;
            LiveStorage_UploadStats(0);
        }
    }
}

void __cdecl CL_ForwardCommandToServer(int localClientNum, const char *string)
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
        if (client_state[0] < 5 || *cmd == 43 || CL_GetLocalClientConnection(localClientNum)->demoplaying)
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
    __int16 v1; // ax
    const char *v2; // eax
    int j; // [esp+10h] [ebp-78h]
    int l; // [esp+14h] [ebp-74h]
    char md5Str[36]; // [esp+18h] [ebp-70h] BYREF
    const dvar_s *v6; // [esp+3Ch] [ebp-4Ch]
    char nums[64]; // [esp+40h] [ebp-48h] BYREF
    int i; // [esp+84h] [ebp-4h]

    lastUpdateKeyAuthTime = cls.realtime;
    if (!CL_CDKeyValidate(cl_cdkey, cl_cdkeychecksum))
    {
        Com_Error(ERR_DROP, "EXE_ERR_INVALID_CD_KEY");
        return;
    }
    if (!cls.authorizeServer.port)
    {
        Com_Printf(14, "Resolving %s\n", com_authServerName->current.string);
        if (!NET_StringToAdr((char *)com_authServerName->current.integer, &cls.authorizeServer))
        {
            Com_Printf(14, "Couldn't resolve address\n");
            return;
        }
        cls.authorizeServer.port = BigShort(com_authPort->current.integer);
        v1 = BigShort(cls.authorizeServer.port);
        Com_Printf(
            14,
            "%s resolved to %i.%i.%i.%i:%i\n",
            com_authServerName->current.string,
            cls.authorizeServer.ip[0],
            cls.authorizeServer.ip[1],
            cls.authorizeServer.ip[2],
            cls.authorizeServer.ip[3],
            v1);
    }
    if (cls.authorizeServer.type != NA_BAD)
    {
        if (Dvar_GetBool("fs_restrict"))
        {
            I_strncpyz(nums, "demo", 64);
        }
        else
        {
            j = 0;
            l = strlen(cl_cdkey);
            if (l > 32)
                l = 32;
            for (i = 0; i < l; ++i)
            {
                if (cl_cdkey[i] >= 48 && cl_cdkey[i] <= 57
                    || cl_cdkey[i] >= 97 && cl_cdkey[i] <= 122
                    || cl_cdkey[i] >= 65 && cl_cdkey[i] <= 90)
                {
                    nums[j++] = cl_cdkey[i];
                }
            }
            nums[j] = 0;
        }
        v6 = Dvar_RegisterBool("cl_anonymous", 0, 0x1Bu, "Allow anonymous log in");
        CL_BuildMd5StrFromCDKey(md5Str);
        v2 = va("getKeyAuthorize %i %s PB %s", v6->current.color[0], nums, md5Str);
        NET_OutOfBandPrint(localClientNum, cls.authorizeServer, v2);
    }
}

void __cdecl CL_ForwardToServer_f()
{
    char command[1028]; // [esp+Ch] [ebp-408h] BYREF

    if (CL_GetLocalClientConnection(0)->demoplaying || client_state[0] != 9)
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
    int i; // [esp+40Ch] [ebp-8h]
    int argc; // [esp+410h] [ebp-4h]

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

void __cdecl CL_DisconnectLocalClient(int localClientNum)
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
    v1 = client_state[0] > 2u;
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

void __cdecl CL_Vid_Restart_f()
{
    unsigned __int8 *v0; // eax
    char *v1; // eax
    bool v2; // [esp+0h] [ebp-D4h]
    char *info; // [esp+4h] [ebp-D0h]
    XZoneInfo zoneInfo[1]; // [esp+8h] [ebp-CCh] BYREF
    char zoneName[64]; // [esp+14h] [ebp-C0h] BYREF
    char mapname[64]; // [esp+54h] [ebp-80h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+98h] [ebp-3Ch]
    clientUIActive_t *clientUIActive; // [esp+9Ch] [ebp-38h]
    connstate_t connstate; // [esp+A0h] [ebp-34h]
    int localClientNum; // [esp+A4h] [ebp-30h]
    clientConnection_t *clc; // [esp+A8h] [ebp-2Ch]
    int clientStateBytes; // [esp+ACh] [ebp-28h]
    MemoryFile memFile; // [esp+B0h] [ebp-24h] BYREF
    unsigned __int8 *clientStateBuf; // [esp+CCh] [ebp-8h]
    int fileSystemRestarted; // [esp+D0h] [ebp-4h]

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
        connstate = client_state[0];
        clientStateBuf = 0;
        clientStateBytes = 0;
        if (byte_E7A7C2)
        {
            v0 = (unsigned __int8 *)Z_VirtualAlloc((int)&unk_A00000, "demo", 0);
            MemFile_InitForWriting(&memFile, (int)&unk_A00000, v0, 1, 0);
            CL_ArchiveClientState(localClientNum, &memFile);
            MemFile_StartSegment(&memFile, -1);
            clientStateBytes = memFile.bytesUsed;
            clientStateBuf = (unsigned __int8 *)Z_VirtualAlloc(memFile.bytesUsed, "CL_Vid_Restart_f", 10);
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
        Dvar_RegisterInt("loc_language", 0, (DvarLimits)0xE00000000LL, 0x21u, "The current language locale");
        Dvar_RegisterBool("loc_translate", 1, 0x20u, "Turn on string translation");
        Dvar_RegisterBool("fs_ignoreLocalized", 0, 0xA0u, "Ignore localized assets");
        v2 = FS_ConditionalRestart(localClientNum, clc->checksumFeed) || cls.gameDirChanged;
        fileSystemRestarted = v2;
        SEH_UpdateLanguageInfo();
        Dvar_SetInt((dvar_s *)cl_paused, 0);
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
    unsigned __int8 *v0; // eax
    unsigned __int8 *soundStateBuf; // [esp+0h] [ebp-90h]
    MemoryFile memFile; // [esp+4h] [ebp-8Ch] BYREF
    snd_listener listeners[2]; // [esp+20h] [ebp-70h] BYREF

    if (com_sv_running->current.enabled)
    {
        Com_Printf(0, "Listen server cannot sound restart.\n");
    }
    else
    {
        Hunk_CheckTempMemoryClear();
        v0 = (unsigned __int8 *)Z_VirtualAlloc((int)&unk_A00000, "demo", 0);
        MemFile_InitForWriting(&memFile, (int)&unk_A00000, v0, 1, 0);
        SND_Save(&memFile);
        MemFile_StartSegment(&memFile, -1);
        soundStateBuf = (unsigned __int8 *)Z_VirtualAlloc(memFile.bytesUsed, "CL_Snd_Restart_f", 10);
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
    int ofs; // [esp+4h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-4h]

    if (client_state[0] == 9)
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
    Com_Printf(0, "state: %i\n", client_state[0]);
    Com_Printf(0, "Server: %s\n", cls.servername);
    Com_Printf(0, "User info settings:\n");
    v0 = Dvar_InfoString(0, 2);
    Info_Print(v0);
    Com_Printf(0, "--------------------------------------\n");
}

void __cdecl CL_DownloadsComplete(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
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
            if ((unsigned int)(&autoupdateFilename[strlen(autoupdateFilename) + 1] - (char *)&unk_B99511) > 4)
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
    client_state[4 * localClientNum] = 7;
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
        Dvar_SetInt((dvar_s *)cl_paused, 1);
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

void __cdecl CL_CheckForResend(netsrc_t localClientNum)
{
    const char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int v6; // [esp+0h] [ebp-1188h]
    char md5Str[36]; // [esp+2Ch] [ebp-115Ch] BYREF
    unsigned __int8 v8[1244]; // [esp+50h] [ebp-1138h] BYREF
    int v9; // [esp+52Ch] [ebp-C5Ch]
    char dest; // [esp+530h] [ebp-C58h] BYREF
    _BYTE v11[3]; // [esp+531h] [ebp-C57h] BYREF
    int pktlen; // [esp+934h] [ebp-854h] BYREF
    unsigned __int8 src[9]; // [esp+938h] [ebp-850h] BYREF
    unsigned __int8 dst[1019]; // [esp+941h] [ebp-847h] BYREF
    unsigned int count; // [esp+D3Ch] [ebp-44Ch]
    msg_t buf; // [esp+D40h] [ebp-448h] BYREF
    int length; // [esp+D68h] [ebp-420h]
    void *data; // [esp+D6Ch] [ebp-41Ch]
    clientConnection_t *clc; // [esp+D70h] [ebp-418h]
    int c; // [esp+D74h] [ebp-414h]
    char pkt[1036]; // [esp+D78h] [ebp-410h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v9 = client_state[0];
    if (client_state[0] == 3 || v9 == 4 || v9 == 6)
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if (v9 == 6)
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
            switch (v9)
            {
            case 3:
                if (net_lanauthorize->current.enabled || !Sys_IsLANAddress(clc->serverAddress))
                    CL_RequestAuthorization(localClientNum);
                strcpy(pkt, "getchallenge");
                pktlen = &pkt[strlen(pkt) + 1] - &pkt[1];
                PbClientConnecting(1, pkt, &pktlen);
                CL_BuildMd5StrFromCDKey(md5Str);
                v1 = va("getchallenge 0 \"%s\"", md5Str);
                NET_OutOfBandPrint(localClientNum, clc->serverAddress, v1);
                break;
            case 4:
                v2 = Dvar_InfoString(localClientNum, 2);
                I_strncpyz(&dest, v2, 1024);
                v3 = va("%i", 1);
                Info_SetValueForKey(&dest, "protocol", v3);
                v4 = va("%i", clc->challenge);
                Info_SetValueForKey(&dest, "challenge", v4);
                if (!clc->qport)
                    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2980, 0, "%s", "clc->qport != 0");
                v5 = va("%i", clc->qport);
                Info_SetValueForKey(&dest, "qport", v5);
                qmemcpy(src, "connect \"", sizeof(src));
                count = &v11[strlen(&dest)] - v11;
                memcpy(dst, (unsigned __int8 *)&dest, count);
                dst[count] = 34;
                dst[count + 1] = 0;
                pktlen = count + 10;
                memcpy((unsigned __int8 *)pkt, src, count + 10);
                PbClientConnecting(2, pkt, &pktlen);
                NET_OutOfBandData(localClientNum, clc->serverAddress, src, count + 10);
                dvar_modifiedFlags &= ~2u;
                break;
            case 6:
                MSG_Init(&buf, msgBuffer, 2048);
                MSG_WriteString(&buf, "stats");
                c = CL_HighestPriorityStatPacket(clc);
                if ((unsigned int)c > 6)
                    MyAssertHandler(
                        ".\\client_mp\\cl_main_mp.cpp",
                        3017,
                        0,
                        "%s\n\t(packetToSend) = %i",
                        "(packetToSend >= 0 && packetToSend < 7)",
                        c);
                CL_ControllerIndexFromClientNum(localClientNum);
                if (LiveStorage_DoWeHaveStats())
                {
                    CL_ControllerIndexFromClientNum(localClientNum);
                    data = (char *)LiveStorage_GetStatBuffer() + 1240 * c;
                }
                else
                {
                    if (onlinegame->current.enabled)
                        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 3029, 0, "%s", "!onlinegame->current.enabled");
                    memset(v8, 0, 0x4D8u);
                    data = v8;
                }
                MSG_WriteShort(&buf, clc->qport);
                MSG_WriteByte(&buf, c);
                if (0x2000 - 1240 * c > 1240)
                    v6 = 1240;
                else
                    v6 = 0x2000 - 1240 * c;
                length = v6;
                MSG_WriteData(&buf, (unsigned __int8 *)data, v6);
                clc->statPacketSendTime[c] = cls.realtime;
                clc->lastPacketSentTime = cls.realtime;
                NET_OutOfBandData(localClientNum, clc->serverAddress, buf.data, buf.cursize);
                break;
            default:
                Com_Error(ERR_FATAL, &byte_874D98);
                break;
            }
        }
    }
}

int __cdecl CL_HighestPriorityStatPacket(clientConnection_t *clc)
{
    int packet; // [esp+0h] [ebp-Ch]
    int oldestPacketTime; // [esp+4h] [ebp-8h]
    int oldestPacket; // [esp+8h] [ebp-4h]

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
    v1 = SEH_SafeTranslateString("EXE_SERVERDISCONNECTREASON");
    v2 = UI_ReplaceConversionString(v1, v3);
    Com_Error(ERR_SERVERDISCONNECT, v2);
}

char __cdecl CL_ConnectionlessPacket(netsrc_t localClientNum, netadr_t from, msg_t *msg, int time)
{
    char *v5; // eax
    char success; // [esp+3h] [ebp-9h]
    char *s; // [esp+4h] [ebp-8h]

    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    CL_Netchan_AddOOBProfilePacket(localClientNum, msg->cursize);
    if (!strnicmp((const char *)msg->data + 4, "PB_", 3u))
    {
        if (msg->data[7] == 83 || msg->data[7] == 50 || msg->data[7] == 73)
            PbSvAddEvent(13, -1, msg->cursize - 4, (char *)msg->data + 4);
        else
            PbClAddEvent(13, msg->cursize - 4, (char *)msg->data + 4);
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

char __cdecl CL_DispatchConnectionlessPacket(netsrc_t localClientNum, netadr_t from, msg_t *msg, int time)
{
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    char *StringLine; // eax
    char *v12; // eax
    clientConnection_t *LocalClientConnection; // eax
    netadr_t v14; // [esp-14h] [ebp-80h]
    netadr_t serverAddress; // [esp-14h] [ebp-80h]
    char *v16; // [esp-4h] [ebp-70h]
    bool v17; // [esp+0h] [ebp-6Ch]
    char *v18; // [esp+4h] [ebp-68h]
    connstate_t connstate; // [esp+Ch] [ebp-60h]
    const char *c; // [esp+10h] [ebp-5Ch]
    int statPacketsNeeded; // [esp+18h] [ebp-54h]
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
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connstate = client_state[0];
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
                                if (client_state[0] != 6)
                                {
                                    if (localClientNum)
                                        MyAssertHandler(
                                            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                                            1120,
                                            0,
                                            "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                            localClientNum,
                                            1);
                                    client_state[4 * localClientNum] = 6;
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
                                v17 = client_state[0] == 9 && NET_CompareBaseAdr(from, clcb->serverAddress);
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
                                client_state[4 * localClientNum] = 5;
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
                        client_state[4 * localClientNum] = 5;
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
                    client_state[4 * localClientNum] = 6;
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
        client_state[4 * localClientNum] = 4;
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

void __cdecl CL_DisconnectPacket(int localClientNum, netadr_t from, char *reason)
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
    if (client_state[0] >= 3)
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
        UI_SetMap(mapname, gametype);
        client_state[0] = client_state[0] < 5 ? 0 : 5;
        SCR_UpdateScreen();
    }
}

char __cdecl CL_PacketEvent(netsrc_t localClientNum, netadr_t from, msg_t *msg, int time)
{
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    int v8; // [esp-8h] [ebp-24h]
    connstate_t connstate; // [esp+4h] [ebp-18h]
    int savedServerMessageSequence; // [esp+8h] [ebp-14h]
    clientConnection_t *clc; // [esp+Ch] [ebp-10h]
    int headerBytes; // [esp+10h] [ebp-Ch]
    int savedReliableAcknowledge; // [esp+14h] [ebp-8h]

    if (msg->cursize >= 4 && *(unsigned int *)msg->data == -1)
        return CL_ConnectionlessPacket(localClientNum, from, msg, time);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connstate = client_state[0];
    if (client_state[0] >= 5)
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
                    clc->serverMessageSequence = *(unsigned int *)msg->data;
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
        v8 = *(unsigned int *)msg->data;
        v5 = NET_AdrToString(from);
        Com_DPrintf(14, "%s: Got msg sequence %i but connstate (%i) is < CA_CONNECTED\n", v5, v8, connstate);
        return 0;
    }
}

void __cdecl CL_VoiceTransmit(int localClientNum)
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

void __cdecl CL_RunOncePerClientFrame(int localClientNum, int msec)
{
    int v2; // eax

    FakeLag_Frame();
    if (UI_IsFullscreen(localClientNum))
        CL_SyncGpu(0);
    IN_Frame();
    if (cl_avidemo->current.integer && msec)
    {
        if (client_state[0] == 9 || cl_forceavidemo->current.enabled)
        {
            v2 = CL_ControllerIndexFromClientNum(localClientNum);
            Cmd_ExecuteSingleCommand(0, v2, "screenshot silent\n");
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
    connstate = client_state[0];
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if (byte_E7A7C1[0])
    {
        CL_DevGuiFrame(localClientNum);
        //Profile_Begin(357);
        CL_VoiceFrame(localClientNum);
        //Profile_EndInternal(0);
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

void __cdecl CL_CheckTimeout(int localClientNum)
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
    connstate = client_state[0];
    if (client_state[0] >= 3)
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

void __cdecl CL_CheckUserinfo(int localClientNum)
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
    if (client_state[0] >= 4 && !cl_paused->current.integer && (dvar_modifiedFlags & 2) != 0)
    {
        v1 = Dvar_InfoString(localClientNum, 2);
        v2 = va("userinfo \"%s\"", v1);
        CL_AddReliableCommand(localClientNum, v2);
    }
}

void __cdecl CL_UpdateInGameState(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (client_state[0] == 9)
    {
        if (!cl_ingame->current.enabled)
            Dvar_SetBool((dvar_s *)cl_ingame, 1);
    }
    else if (cl_ingame->current.enabled)
    {
        Dvar_SetBool((dvar_s *)cl_ingame, 0);
    }
}

void __cdecl CL_VoiceFrame(int localClientNum)
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

bool __cdecl CL_IsLocalClientInGame(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return client_state[0] == 9;
}

char __cdecl CL_IsClientLocal(int clientNum)
{
    int client; // [esp+0h] [ebp-4h]

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
        if (client_state[0] > 7 && CG_GetClientNum(client) == clientNum)
            return 1;
    }
    return 0;
}

void __cdecl CL_ParseBadPacket_f()
{
    msg_t msg; // [esp+0h] [ebp-30h] BYREF
    int fileSize; // [esp+28h] [ebp-8h]
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
        msg.data = (unsigned __int8 *)file;
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
    int localClientNum; // [esp+0h] [ebp-4h]

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

void __cdecl CL_ShutdownRenderer(int destroyWindow)
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
    CL_ResetStats_f();
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
        dvar = _Dvar_FindVar(v1);
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

int __cdecl CL_ScaledMilliseconds()
{
    return cls.realtime;
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

void __cdecl CL_DrawLogo(int localClientNum)
{
    float fade; // [esp+44h] [ebp-24h]
    int time; // [esp+48h] [ebp-20h]
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
    if (client_state[0] != 2)
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

void __cdecl CL_StopLogo(int localClientNum)
{
    client_state[4 * localClientNum] = 0;
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
    if (client_state[0] == 1)
    {
        SCR_StopCinematic(0);
    }
    else if (client_state[0] == 2)
    {
        CL_StopLogo(0);
    }
    else if (client_state[0])
    {
        return;
    }
    client_state[0] = 2;
    if (cls.uiStarted)
        UI_SetActiveMenu(0, 0);
    SND_StopSounds(SND_STOP_ALL);
    SND_FadeAllSounds(1.0, 0);
    name = Cmd_Argv(1);
    v0 = Cmd_Argv(2);
    v7 = atof(v0);
    v10 = v7 * 1000.0;
    cls.logo.fadein = (int)(v10 + 9.313225746154785e-10);
    v1 = Cmd_Argv(3);
    v6 = atof(v1);
    v9 = v6 * 1000.0;
    cls.logo.duration = (int)(v9 + 9.313225746154785e-10);
    v2 = Cmd_Argv(4);
    v5 = atof(v2);
    v8 = v5 * 1000.0;
    cls.logo.fadeout = (int)(v8 + 9.313225746154785e-10);
    cls.logo.duration += cls.logo.fadeout + cls.logo.fadein;
    v3 = va("%s1", name);
    cls.logo.material[0] = Material_RegisterHandle(v3, 3);
    v4 = va("%s2", name);
    cls.logo.material[1] = Material_RegisterHandle(v4, 3);
    cls.logo.startTime = cls.realtime + 100;
}

void __cdecl CL_StopLogoOrCinematic(int localClientNum)
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
    clcState = client_state[0];
    if (client_state[0] == 1)
    {
        SCR_StopCinematic(localClientNum);
    }
    else
    {
        if (client_state[0] != 2)
            MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 4744, 0, "%s", "clcState == CA_LOGO");
        CL_StopLogo(localClientNum);
    }
    SND_StopSounds(SND_STOP_ALL);
    if (clcState)
        UI_SetActiveMenu(localClientNum, 0);
    else
        UI_SetActiveMenu(localClientNum, 1);
}

void __cdecl CL_ToggleMenu_f()
{
    int ActiveMenu; // [esp+0h] [ebp-18h]
    connstate_t connstate; // [esp+8h] [ebp-10h]
    clientConnection_t *clc; // [esp+10h] [ebp-8h]

    clc = CL_GetLocalClientConnection(0);
    connstate = client_state[0];
    if ((dword_E7A7C4[0] & 0x10) != 0)
        ActiveMenu = UI_GetActiveMenu(0);
    else
        ActiveMenu = 0;
    if (clc->demoplaying)
    {
    LABEL_13:
        UI_SetActiveMenu(0, 1);
        return;
    }
    if (connstate != CA_ACTIVE)
    {
        if (!cl_waitingOnServerToLoadMap[0])
            return;
        goto LABEL_13;
    }
    if ((dword_E7A7C4[0] & 0x10) != 0 && ActiveMenu == 2)
    {
        UI_SetActiveMenu(0, 0);
    }
    else if (!dword_E7A7C4[0])
    {
        UI_SetActiveMenu(0, 2);
    }
}

void __cdecl CL_InitOnceForAllClients()
{
    unsigned int v0; // eax
    int i; // [esp+18h] [ebp-4h]

    v0 = Sys_MillisecondsRaw();
    srand(v0);
    Con_Init();
    CL_InitInput();
    cl_noprint = Dvar_RegisterBool("cl_noprint", 0, 0, "Print nothing to the console");
    for (i = 0; i < 5; ++i)
        customclass[i] = Dvar_RegisterString(customClassDvars[i], "", 1u, "Custom class name");
    onlinegame = Dvar_RegisterBool(
        "onlinegame",
        1,
        0x80u,
        "Current game is an online game with stats, custom classes, unlocks");
    Dvar_SetBool(onlinegame, 1);
    cl_hudDrawsBehindUI = Dvar_RegisterBool("cl_hudDrawsBehindUI", 1, 0, "Should the HUD draw when the UI is up?");
    cl_voice = Dvar_RegisterBool("cl_voice", 1, 3u, "Use voice communications");
    cl_timeout = Dvar_RegisterFloat(
        "cl_timeout",
        40.0,
        0.0,
        3600.0,
        0,
        "Seconds with no received packets until a timeout occurs");
    cl_connectTimeout = Dvar_RegisterFloat(
        "cl_connectTimeout",
        200.0,
        0.0,
        3600.0,
        0,
        "Timeout time in seconds while connecting to a server");
    cl_connectionAttempts = Dvar_RegisterInt(
        "cl_connectionAttempts",
        10,
        0,
        0x7FFFFFFF,
        0,
        "Maximum number of connection attempts before aborting");
    cl_shownet = Dvar_RegisterInt("cl_shownet", 0, -2, 4, 0, "Display network debugging information");
    cl_shownuments = Dvar_RegisterBool("cl_shownuments", 0, 0, "Show the number of entities");
    cl_showServerCommands = Dvar_RegisterBool(
        "cl_showServerCommands",
        0,
        0,
        "Enable debugging information for server commands");
    cl_showSend = Dvar_RegisterBool("cl_showSend", 0, 0, "Enable debugging information for sent commands");
    cl_showTimeDelta = Dvar_RegisterBool("cl_showTimeDelta", 0, 0, "Enable debugging information for time delta");
    cl_freezeDemo = Dvar_RegisterBool(
        "cl_freezeDemo",
        0,
        0,
        "cl_freezeDemo is used to lock a demo in place for single frame advances");
    cl_activeAction = Dvar_RegisterString("activeAction", "", 0, "Action to execute in first frame");
    cl_avidemo = Dvar_RegisterInt("cl_avidemo", 0, 0, 0x7FFFFFFF, 0, "AVI demo frames per second");
    cl_forceavidemo = Dvar_RegisterBool("cl_forceavidemo", 0, 0, "Record AVI demo even if client is not active");
    cl_yawspeed = Dvar_RegisterFloat(
        "cl_yawspeed",
        140.0,
        -3.4028235e38,
        3.4028235e38,
        1u,
        "Max yaw speed in degrees for game pad and keyboard");
    cl_pitchspeed = Dvar_RegisterFloat(
        "cl_pitchspeed",
        140.0,
        -3.4028235e38,
        3.4028235e38,
        1u,
        "Max pitch speed in degrees for game pad");
    cl_anglespeedkey = Dvar_RegisterFloat(
        "cl_anglespeedkey",
        1.5,
        0.0,
        3.4028235e38,
        0,
        "Multiplier for max angle speed for game pad and keyboard");
    cl_maxpackets = Dvar_RegisterInt("cl_maxpackets", 30, 15, 100, 1u, "Maximum number of packets sent per frame");
    cl_packetdup = Dvar_RegisterInt("cl_packetdup", 1, 0, 5, 1u, "Enable packet duplication");
    cl_sensitivity = Dvar_RegisterFloat("sensitivity", 5.0, 0.0099999998, 100.0, 1u, "Mouse sensitivity");
    cl_mouseAccel = Dvar_RegisterFloat("cl_mouseAccel", 0.0, 0.0, 100.0, 1u, "Mouse acceleration");
    cl_freelook = Dvar_RegisterBool("cl_freelook", 1, 1u, "Enable looking with mouse");
    cl_showMouseRate = Dvar_RegisterBool(
        "cl_showmouserate",
        0,
        0,
        "Print mouse rate debugging information to the console");
    cl_allowDownload = Dvar_RegisterBool("cl_allowDownload", 1, 1u, "Allow client downloads from the server");
    cl_wwwDownload = Dvar_RegisterBool("cl_wwwDownload", 1, 3u, "Download files via HTTP");
    cl_talking = Dvar_RegisterBool("cl_talking", 0, 0, "Client is talking");
    cl_inGameVideo = Dvar_RegisterBool("r_inGameVideo", 1, 1u, "Allow in game cinematics");
    cl_serverStatusResendTime = Dvar_RegisterInt(
        "cl_serverStatusResendTime",
        750,
        0,
        3600,
        0,
        "Time in milliseconds to resend a server status message");
    cl_bypassMouseInput = Dvar_RegisterBool(
        "cl_bypassMouseInput",
        0,
        0,
        "Bypass UI mouse input and send directly to the game");
    m_pitch = Dvar_RegisterFloat("m_pitch", 0.022, -1.0, 1.0, 1u, "Default pitch");
    m_yaw = Dvar_RegisterFloat("m_yaw", 0.022, -1.0, 1.0, 1u, "Default yaw");
    m_forward = Dvar_RegisterFloat("m_forward", 0.25, -1.0, 1.0, 1u, "Forward speed in units per second");
    m_side = Dvar_RegisterFloat("m_side", 0.25, -1.0, 1.0, 1u, "Sideways motion in units per second");
    m_filter = Dvar_RegisterBool("m_filter", 0, 1u, "Allow mouse movement smoothing");
    cl_motdString = Dvar_RegisterString("cl_motdString", "", 0x40u, "Message of the day");
    cl_ingame = Dvar_RegisterBool("cl_ingame", 0, 0x40u, "True if the game is active");
    Dvar_RegisterInt("cl_maxPing", 800, 20, 2000, 1u, "Maximum ping for the client");
    cl_profileTextHeight = Dvar_RegisterInt(
        "cl_profileTextHeight",
        19,
        1,
        100,
        0,
        "Text size to draw the network profile data");
    cl_profileTextY = Dvar_RegisterInt("cl_profileTextY", 110, 0, 800, 0, "Y position to draw the profile data");
    name = Dvar_RegisterString("name", "", 3u, "Player name");
    Dvar_RegisterInt("rate", 25000, 1000, 25000, 3u, "Player's preferred baud rate");
    Dvar_RegisterInt("snaps", 20, 1, 30, 3u, "Snapshot rate");
    Dvar_RegisterBool("cl_punkbuster", 1, 0x13u, "Determines whether PunkBuster is enabled");
    Dvar_RegisterString("password", "", 2u, "password");
    nextdemo = Dvar_RegisterString("nextdemo", "", 0, "The next demo to play");
    Dvar_RegisterBool("hud_enable", 1, 1u, "Enable the HUD display");
    Dvar_RegisterBool("cg_blood", 1, 1u, "Show blood");
    cl_updateavailable = Dvar_RegisterBool("cl_updateavailable", 0, 0x40u, "True if there is an available update");
    cl_updatefiles = Dvar_RegisterString("cl_updatefiles", "", 0x40u, "The file that is being updated");
    cl_updateoldversion = Dvar_RegisterString("cl_updateoldversion", "", 0x40u, "The version before update");
    cl_updateversion = Dvar_RegisterString("cl_updateversion", "", 0x40u, "The updated version");
    cl_debugMessageKey = Dvar_RegisterBool("cl_debugMessageKey", 0, 0, "Enable message key debugging information");
    I_strncpyz(cls.autoupdateServerNames[0], "cod2update.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[1], "cod2update2.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[2], "cod2update3.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[3], "cod2update4.activision.com", 64);
    I_strncpyz(cls.autoupdateServerNames[4], "cod2update5.activision.com", 64);
    motd = Dvar_RegisterString("motd", "", 0, "Message of the day");
    vehDriverViewHeightMin = Dvar_RegisterFloat(
        "vehDriverViewHeightMin",
        -15.0,
        -80.0,
        80.0,
        1u,
        "Min orbit altitude for driver's view");
    vehDriverViewHeightMax = Dvar_RegisterFloat(
        "vehDriverViewHeightMax",
        50.0,
        -80.0,
        80.0,
        1u,
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
    Cmd_AddCommandInternal("resetStats", CL_ResetStats_f, &CL_ResetStats_f_VAR);
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

void __cdecl CL_Init(int localClientNum)
{
    int v1; // eax

    Com_Printf(14, "----- Client Initialization -----\n");
    CL_ClearState(localClientNum);
    if (CountBitsEnabled(0xFFFFFFFF) != 32)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5206, 0, "%s", "CountBitsEnabled( 0xffffffff ) == 32");
    if (CountBitsEnabled(0))
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5207, 0, "%s", "CountBitsEnabled( 0x00000000 ) == 0");
    if (CountBitsEnabled(0x11111111u) != 8)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5208, 0, "%s", "CountBitsEnabled( 0x11111111 ) == 8");
    if (CountBitsEnabled(0x77777777u) != 24)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 5209, 0, "%s", "CountBitsEnabled( 0x77777777 ) == 24");
    CL_ClearMutedList();
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    client_state[0] = 0;
    cls.realtime = 0;
    clientUIActives[0].active = 1;
    cl_serverLoadingMap = 0;
    cl_waitingOnServerToLoadMap[localClientNum] = 0;
    g_waitingForServer = 0;
    v1 = CL_ControllerIndexFromClientNum(localClientNum);
    Cbuf_Execute(localClientNum, v1);
    byte_E7A7C1[0] = 1;
    Com_Printf(14, "----- Client Initialization Complete -----\n");
}

int __cdecl CountBitsEnabled(unsigned int num)
{
    unsigned int numa; // [esp+1Ch] [ebp+8h]
    unsigned int numb; // [esp+1Ch] [ebp+8h]

    numa = (((((num >> 1) & 0x55555555) + (num & 0x55555555)) >> 2) & 0x33333333)
        + ((((num >> 1) & 0x55555555) + (num & 0x55555555)) & 0x33333333);
    numb = (((unsigned int)&clients[0].parseEntities[1878].lerp.apos.trTime + 3)
        & ((((numa >> 4) & 0xF0F0F0F) + (numa & 0xF0F0F0F)) >> 8))
        + (((unsigned int)&clients[0].parseEntities[1878].lerp.apos.trTime + 3)
            & (((numa >> 4) & 0xF0F0F0F) + (numa & 0xF0F0F0F)));
    return HIWORD(numb) + (unsigned __int16)numb;
}

void __cdecl CL_Shutdown(int localClientNum)
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
            memset((unsigned __int8 *)&cls, 0, sizeof(cls));
        }
        byte_E7A7C1[16 * localClientNum] = 0;
        recursive = 0;
        Com_Printf(14, "-----------------------\n");
    }
}

void __cdecl CL_LocalServers_f()
{
    unsigned __int8 b; // [esp+10h] [ebp-2Ch]
    int j; // [esp+14h] [ebp-28h]
    int i; // [esp+20h] [ebp-1Ch]
    int ia; // [esp+20h] [ebp-1Ch]
    netadr_t to; // [esp+24h] [ebp-18h] BYREF

    Com_Printf(0, "Scanning for servers on the local network...\n");
    cls.numlocalservers = 0;
    cls.pingUpdateSource = 0;
    for (i = 0; i < 128; ++i)
    {
        b = cls.localServers[i].dirty;
        Com_Memset((unsigned int *)&cls.localServers[i], 0, 148);
        cls.localServers[i].dirty = b;
    }
    Com_Memset((unsigned int *)&to, 0, 20);
    for (ia = 0; ia < 2; ++ia)
    {
        for (j = 0; j < 4; ++j)
        {
            to.port = BigShort(j + 28960);
            to.type = NA_BROADCAST;
            NET_OutOfBandData(NS_CLIENT1, to, "getinfo xxx", strlen("getinfo xxx"));
        }
    }
}

void __cdecl CL_GetPing(int n, char *buf, int buflen, int *pingtime)
{
    char *str; // [esp+0h] [ebp-Ch]
    signed int time; // [esp+4h] [ebp-8h]
    int maxPing; // [esp+8h] [ebp-4h]

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

void __cdecl CL_ClearPing(unsigned int n)
{
    if (n < 0x10)
        cl_pinglist[n].adr.port = 0;
}

int __cdecl CL_GetPingQueueCount()
{
    int i; // [esp+0h] [ebp-Ch]
    ping_t *pingptr; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

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

int __cdecl CL_UpdateDirtyPings(netsrc_t localClientNum, unsigned int source)
{
    serverInfo_t *v3; // edx
    ping_t *v4; // eax
    int j; // [esp+4h] [ebp-420h]
    int ja; // [esp+4h] [ebp-420h]
    serverInfo_t *server; // [esp+8h] [ebp-41Ch]
    int max; // [esp+Ch] [ebp-418h]
    int status; // [esp+10h] [ebp-414h]
    int slots; // [esp+14h] [ebp-410h]
    int i; // [esp+18h] [ebp-40Ch]
    int ia; // [esp+18h] [ebp-40Ch]
    char buff[1024]; // [esp+1Ch] [ebp-408h] BYREF
    int pingTime; // [esp+420h] [ebp-4h] BYREF

    status = 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (client_state[0])
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
                    *(unsigned int *)v4->adr.ip = *(unsigned int *)v3->adr.ip;
                    *(unsigned int *)&v4->adr.port = *(unsigned int *)&v3->adr.port;
                    *(unsigned int *)&v4->adr.ipx[2] = *(unsigned int *)&v3->adr.ipx[2];
                    *(unsigned int *)&v4->adr.ipx[6] = *(unsigned int *)&v3->adr.ipx[6];
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
    int localClientNum; // [esp+0h] [ebp-4h]

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
        UI_SetMap((char *)"", (char *)&String);
        LoadMapLoadscreen(pszMapName);
        UI_SetMap(pszMapName, pszGametype);
    }
    SCR_UpdateScreen();
}

bool __cdecl CL_IsServerLoadingMap()
{
    return cl_serverLoadingMap;
}

bool __cdecl CL_IsWaitingOnServerToLoadMap(int localClientNum)
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

void __cdecl CL_SetWaitingOnServerToLoadMap(int localClientNum, bool waiting)
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
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style)
{
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style);
}

void __cdecl CL_DrawTextPhysicalWithEffects(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    const float *glowColor,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration)
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
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int style)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style);
}

void __cdecl CL_DrawTextRotate(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float rotation,
    int horzAlign,
    int vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int style)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, rotation, color, style);
}

void __cdecl CL_DrawTextPhysicalWithCursor(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    int cursorPos,
    char cursor)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

void __cdecl CL_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int style,
    int cursorPos,
    char cursor)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

bool __cdecl CL_ShouldDisplayHud(int localClientNum)
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

bool __cdecl CL_IsUIActive(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return (dword_E7A7C4[0] & 0x10) != 0;
}

Font_s *__cdecl CL_RegisterFont(const char *fontName, int imageTrack)
{
    return R_RegisterFont(fontName, imageTrack);
}

void __cdecl CL_UpdateSound()
{
    CL_ResetStats_f();
    SND_PlayFXSounds();
    SND_UpdateLoopingSounds();
    SND_Update();
}

float (*__cdecl CL_GetMapCenter())[3]
{
    return (float (*)[3])cls.mapCenter;
}

void __cdecl CL_ResetStats_f()
{
    ;
}

int __cdecl CL_GetLocalClientActiveCount()
{
    return 1;
}

