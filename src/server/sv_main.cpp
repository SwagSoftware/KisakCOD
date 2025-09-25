#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "server.h"
#include "sv_public.h"
#include <game/savememory.h>
#include <cgame/cg_main.h>
#include <game/g_save.h>
#include <script/scr_vm.h>
#include <client/cl_demo.h>
#include <game/g_local.h>
#include "sv_game.h"
#include <gfx_d3d/r_workercmds.h>
#include <qcommon/threads.h>
#include <win32/win_local.h>
#include <universal/profile.h>
#include <client/cl_scrn.h>
#include <game/savedevice.h>

server_t sv;
serverStatic_t svs;

int com_time;
int com_inServerFrame;

const dvar_t *sv_lastSaveGame;
const dvar_t *sv_smp;
const dvar_t *sv_player_damageMultiplier;
const dvar_t *sv_player_maxhealth;
const dvar_t *sv_saveOnStartMap;
const dvar_t *sv_gameskill;
const dvar_t *sv_mapname;
const dvar_t *sv_saveDeviceAvailable;
const dvar_t *sv_cheats;
const dvar_t *player_healthEasy;
const dvar_t *player_healthHard;
const dvar_t *sv_player_deathInvulnerableTime;
const dvar_t *runForTime;
const dvar_t *sv_saveGameSuccess;
const dvar_t *sv_saveGameAvailable;
const dvar_t *sv_saveGameNotReadable;
const dvar_t *replay_autosave;
const dvar_t *player_healthMedium;
const dvar_t *player_healthFu;
const dvar_t *replay_asserts;

PendingSaveList pendingSaveGlob;

void __cdecl TRACK_sv_main()
{
    track_static_alloc_internal(&svs, 40, "svs", 9);
    track_static_alloc_internal(&sv, 72480, "sv", 9);
    track_static_alloc_internal(&pendingSaveGlob, 1208, "pendingSaveGlob", 10);
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

void __cdecl SV_DumpServerCommands(client_t *client)
{
    int i; // r31

    Com_Printf(15, "===== pending server commands =====\n");
    for (i = client->reliableCommands.header.sent + 1; i <= client->reliableCommands.header.sequence; ++i)
        Com_Printf(
            15,
            "cmd %5d: %s\n",
            i,
            &client->reliableCommands.buf[client->reliableCommands.commands[(unsigned __int8)i]]);
}

void __cdecl AppendCommandsForInternalSave(const char *filename)
{
    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 191, 0, "%s", "filename");
}

void __cdecl SV_InitiatePendingSave(
    const char *filename,
    const char *description,
    const char *screenshot,
    SaveType saveType,
    unsigned int commitLevel,
    PendingSave *pendingSave,
    bool suppressPlayerNotify)
{
    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 219, 0, "%s", "filename");
    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 220, 0, "%s", "pendingSave");
    if (!description)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 221, 0, "%s", "description");
    if (!screenshot)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 222, 0, "%s", "screenshot");
    I_strncpyz(pendingSave->filename, filename, 64);
    I_strncpyz(pendingSave->description, description, 256);
    I_strncpyz(pendingSave->screenShotName, screenshot, 64);
    pendingSave->saveType = saveType;
    pendingSave->commitLevel = commitLevel;
    if (saveType == SAVE_TYPE_AUTOSAVE)
    {
        pendingSave->saveId = SaveMemory_GenerateSaveId();
        pendingSave->suppressPlayerNotify = suppressPlayerNotify;
    }
    else
    {
        pendingSave->suppressPlayerNotify = suppressPlayerNotify;
        pendingSave->saveId = 0;
    }
}

int __cdecl SV_AddPendingSave(
    const char *filename,
    const char *description,
    const char *screenshot,
    SaveType saveType,
    unsigned int commitLevel,
    bool suppressPlayerNotify)
{
    PendingSave *v13; // r30

    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 242, 0, "%s", "filename");
    if (!description)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 243, 0, "%s", "description");
    if (!screenshot)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 244, 0, "%s", "screenshot");
    if (Dvar_GetInt("g_reloading"))
    {
        Com_Printf(15, "savegame request ignored\n");
        return -1;
    }
    if (saveType == SAVE_TYPE_AUTOSAVE)
    {
        if (pendingSaveGlob.isAutoSaving)
        {
            Com_PrintWarning(15, "Warning: Multiple Autosaves attempted in same frame. Save %s ignored\n", filename);
            return -2;
        }
        pendingSaveGlob.isAutoSaving = 1;
    }
    if (pendingSaveGlob.count < 3)
    {
        if (pendingSaveGlob.count < 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp",
                275,
                0,
                "%s\n\t(pendingSaveGlob.count) = %i",
                "(pendingSaveGlob.count < PENDING_SAVES_LIMIT && pendingSaveGlob.count >= 0)",
                pendingSaveGlob.count);
        v13 = &pendingSaveGlob.pendingSaves[pendingSaveGlob.count++];
        SV_InitiatePendingSave(filename, description, screenshot, saveType, commitLevel, v13, suppressPlayerNotify);
        return v13->saveId;
    }
    else
    {
        Com_PrintWarning(15, "Warning: Pending Saves limit exceeded. Save %s ignored\n", filename);
        return -3;
    }
}

int __cdecl SV_ProcessPendingSave(PendingSave *pendingSave)
{
    int v2; // r3
    int v3; // r28

    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 292, 0, "%s", "pendingSave");
    v2 = SV_GetCheckSum();
    v3 = G_SaveGame(pendingSave, v2);
    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 191, 0, "%s", "filename");
    return v3;
}

int __cdecl SV_ProcessPendingSaves()
{
    int result; // r3
    int v1; // r28
    volatile int v2; // r30
    PendingSaveList *v3; // r29

    if (!pendingSaveGlob.count)
        return 0;
    if (Dvar_GetInt("g_reloading"))
    {
        Com_Printf(15, "savegame request ignored\n");
        return 0;
    }
    v1 = 0;
    v2 = 0;
    if (pendingSaveGlob.count > 0)
    {
        v3 = &pendingSaveGlob;
        do
        {
            if (SV_ProcessPendingSave(v3->pendingSaves))
                v1 = 1;
            ++v2;
            v3 = (PendingSaveList *)((char *)v3 + 400);
        } while (v2 < pendingSaveGlob.count);
    }
    result = v1;
    pendingSaveGlob.count = 0;
    pendingSaveGlob.isAutoSaving = 0;
    return result;
}

void __cdecl SV_ClearPendingSaves()
{
    pendingSaveGlob.count = 0;
    pendingSaveGlob.isAutoSaving = 0;
}

int __cdecl SV_IsInternalSave(const char *filename)
{
    const char *v1; // r31
    int v2; // r11
    char v3; // r11
    bool v4; // zf

    v1 = filename;
    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 351, 0, "%s", "filename");
LABEL_3:
    if (!I_strnicmp(v1, "internal", 8) && v1[8] == 45)
        return 1;
    while (1)
    {
        v2 = *v1;
        if (!*v1)
            return 0;
        if (v2 == 47 || (v4 = v2 != 92, v3 = 0, !v4))
            v3 = 1;
        ++v1;
        if (v3)
            goto LABEL_3;
    }
}

void __cdecl SV_SetLastSaveName(const char *filename)
{
    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 374, 0, "%s", "filename");
    if (!(unsigned __int8)SV_IsInternalSave(filename))
        Dvar_SetString(sv_lastSaveGame, filename);
}

void __cdecl SV_AddServerCommand(client_t *client, const char *cmd)
{
    int v4; // r4

    if (client->reliableCommands.header.sequence - client->reliableCommands.header.sent == 256)
    {
        SV_DumpServerCommands(client);
        Com_Error(ERR_DROP, "Server command overflow");
    }
    if (client->reliableCommands.header.sequence - client->reliableCommands.header.sent >= 256)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp",
            399,
            0,
            "%s",
            "client->reliableCommands.header.sequence - client->reliableCommands.header.sent < MAX_RELIABLE_COMMANDS");
    v4 = (unsigned __int8)(HIBYTE(client->reliableCommands.header.sequence) + 1);
    ++client->reliableCommands.header.sequence;
    SV_AddReliableCommand(client, v4, cmd);
}

unsigned __int8 tempServerCommandBuf[131072];
void SV_SendServerCommand(client_t *cl, const char *fmt, ...)
{
    client_t *clients;
    va_list va;

    va_start(va, fmt);

    _vsnprintf((char *)tempServerCommandBuf, 0x20000u, fmt, va);

    if (cl)
    {
        clients = cl;
    }
    else
    {
        clients = svs.clients;
        if (svs.clients->state != 1)
        {
            return;
        }
    }

    SV_AddServerCommand(clients, (const char*)tempServerCommandBuf);

}

void __cdecl SV_SaveServerCommands(SaveGame *save)
{
    client_t *clients; // r30
    int i; // r31

    clients = svs.clients;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 455, 0, "%s", "save");
    SaveMemory_SaveWrite(&clients->reliableCommands, 12, save);
    for (i = clients->reliableCommands.header.sent + 1; i <= clients->reliableCommands.header.sequence; ++i)
        SaveMemory_SaveWrite(&clients->reliableCommands.commands[(unsigned __int8)i], 4, save);
    SaveMemory_SaveWrite(clients->reliableCommands.buf, clients->reliableCommands.header.rover, save);
}

void __cdecl SV_LoadServerCommands(SaveGame *save)
{
    client_t *clients; // r30
    int i; // r31

    clients = svs.clients;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 471, 0, "%s", "save");
    SaveMemory_LoadRead(&clients->reliableCommands, 12, save);
    for (i = clients->reliableCommands.header.sent + 1; i <= clients->reliableCommands.header.sequence; ++i)
        SaveMemory_LoadRead(&clients->reliableCommands.commands[(unsigned __int8)i], 4, save);
    SaveMemory_LoadRead(clients->reliableCommands.buf, clients->reliableCommands.header.rover, save);
    CG_SetServerCommandSequence(clients->reliableCommands.header.sent);
}

void __cdecl SV_PreFrame()
{
    char v0; // r11
    const char *v1; // r3
    const char *v2; // r3

    //Profile_Begin(259);
    v0 = dvar_modifiedFlags;
    if ((dvar_modifiedFlags & 4) != 0)
    {
        v1 = Dvar_InfoString(0, 4);
        SV_SetConfigstring(0, v1);
        v0 = dvar_modifiedFlags & 0xFB;
        dvar_modifiedFlags &= ~4u;
    }
    if ((v0 & 8) != 0)
    {
        v2 = Dvar_InfoString_Big(8);
        SV_SetConfigstring(1u, v2);
        dvar_modifiedFlags &= ~8u;
    }
    CL_RecordServerDebugData();
    G_RunPreFrame();
    SV_ResetSkeletonCache();
    //Profile_EndInternal(0);
}

int __cdecl SV_RunFrame(ServerFrameExtent extent, int timeCap)
{
    int v4; // r30

    //Profile_Begin(259);
    CL_FlushDebugServerData();
    v4 = 1;
    if (!CL_DemoPlaying())
    {
        //Profile_Begin(234);
        v4 = G_RunFrame(extent, timeCap);
        //Profile_EndInternal(0);
    }
    if (extent == SV_FRAME_DO_ALL)
    {
        Scr_ProfileUpdate();
        Scr_ProfileBuiltinUpdate();
        //Profile_ResetCounters(1);
        //Profile_ResetScriptCounters();
    }
    CL_UpdateDebugServerData();
    //Profile_EndInternal(0);
    return v4;
}

void SV_ProcessPostFrame()
{
    if (SV_ProcessPendingSaves())
        SV_DisplaySaveErrorUI(); // savedevice_xenon

    Scr_UpdateDebugger();
    SV_UpdateDemo();
}

static int svPerfCounter = 0;
int serverPreviousFrameTimes[200];
void __cdecl SV_UpdatePerformanceFrame(int time)
{
    volatile int max; // r7
    volatile int min; // r10
    int v3; // r9
    bool v4; // cr56

    max = INT_MIN;
    min = INT_MAX;
    v3 = svPerfCounter % 200;
    v4 = ++svPerfCounter < 200;
    serverPreviousFrameTimes[v3] = time;
    if (!v4)
    {
        int sum = 0;
        for (int i = 0; i < 200; i++)
        {
            sum += serverPreviousFrameTimes[i];

            if (serverPreviousFrameTimes[i] < min)
            {
                min = serverPreviousFrameTimes[i];
            }
            if (serverPreviousFrameTimes[i] > max)
            {
                max = serverPreviousFrameTimes[i];
            }
        }
        sv.serverFrameTime = (int)((float)sum / 200.0f);

        if (min <= 0)
            min = 1;
        sv.serverFrameTimeMin = min;
        sv.serverFrameTimeMax = max;
    }
}

bool __cdecl SV_CheckSkipTimeout()
{
    return SV_CheckAutoSaveHistory(0) != 0;
}

int __cdecl SV_CheckStartServer()
{
    int result; // r3

    result = Sys_WaitStartServer(0);
    if (result || sv.restartServerThread)
        return 1;
    return result;
}

int __cdecl SV_WaitStartServer()
{
    int result; // r3

    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "wait start server");
    R_ProcessWorkerCmdsWithTimeout(SV_CheckStartServer, 1);
    //PIXEndNamedEvent();
    if (!sv.restartServerThread)
        return 1;
    result = 0;
    sv.serverExecTime = 0;
    sv.restartServerThread = 0;
    sv.clientMessageTimeout = 0;
    return result;
}

void __cdecl  SV_ServerThread(unsigned int threadContext)
{
    void *Value; // r3
    void *v2; // r3
    int v3; // r28
    bool v4; // r29
    int v5; // r28
    int v6; // r29

    iassert(threadContext == THREAD_CONTEXT_SERVER);
    Value = Sys_GetValue(2);
    if (setjmp((int*)Value))
    {
        do
        {
            Profile_Recover(1);
            v2 = Sys_GetValue(2);
        } while (setjmp((int *)v2));
    }
    Profile_Guard(1);
    Sys_InitServerEvents();
    while (1)
    {
        while (1)
        {
            Sys_ServerCompleted();
            {
                PROF_SCOPED("wait start server");
                R_ProcessWorkerCmdsWithTimeout(SV_CheckStartServer, 1);
            }
            if (!sv.restartServerThread)
                break;
            sv.serverExecTime = 0;
            sv.restartServerThread = 0;
            sv.clientMessageTimeout = 0;
        }
        v3 = Sys_Milliseconds();
        {
            PROF_SCOPED("run frame");
            SV_PreFrame();
        }
        CL_FlushDebugServerData();
        if (!CL_DemoPlaying())
        {
            PROF_SCOPED("G_RunFrame");
            G_RunFrame(SV_FRAME_DO_ALL, 0);
        }
        Scr_ProfileUpdate();
        Scr_ProfileBuiltinUpdate();
        CL_UpdateDebugServerData();
        Sys_ServerCompleted();
        v4 = SV_CheckAutoSaveHistory(0) != 0;
        v5 = Sys_Milliseconds() - v3;
        Sys_WaitClientMessageReceived();
        Sys_EnterCriticalSection(CRITSECT_CLIENT_MESSAGE);
        if (!v4 && !Sys_ServerTimeout())
        {
            Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
            {
                PROF_SCOPED("server timeout")
                R_ProcessWorkerCmdsWithTimeout(Sys_ServerTimeout, 1);
            }
            Sys_EnterCriticalSection(CRITSECT_CLIENT_MESSAGE);
        }
        if (sv.clientMessageTimeout)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 784, 0, "%s", "!sv.clientMessageTimeout");
        sv.clientMessageTimeout = 1;
        Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
        {
            PROF_SCOPED("wait send msg");
            R_ProcessWorkerCmdsWithTimeout(Sys_CanSendClientMessages, 1);
        }
        {
            PROF_SCOPED("wait start server");
            R_ProcessWorkerCmdsWithTimeout(SV_CheckStartServer, 1);
        }
        if (sv.restartServerThread)
        {
            sv.serverExecTime = 0;
            sv.restartServerThread = 0;
            sv.clientMessageTimeout = 0;
        }
        else
        {
            v6 = Sys_Milliseconds();
            {
                PROF_SCOPED("post frame");
                Sys_ClearClientMessage();
                SV_SendClientMessages();
                CL_CreateNextSnap();
                Sys_ServerSnapshotCompleted();
                if (SV_ProcessPendingSaves())
                    SV_DisplaySaveErrorUI(); // savedevice_xenon
                Scr_UpdateDebugger();
                SV_UpdateDemo();
            }
            sv.serverExecTime = Sys_Milliseconds() + v5 - v6;
            SV_UpdatePerformanceFrame(sv.serverExecTime);
        }
    }
}

void __cdecl SV_InitServerThread()
{
    if (!Sys_SpawnServerThread(SV_ServerThread))
        Sys_Error("Failed to create server thread");
}

void __cdecl SV_ExitAfterTime()
{
    const dvar_s *v0; // r3
    int integer; // r11

    v0 = runForTime;
    if (!runForTime)
    {
        v0 = Dvar_RegisterInt("RunForTime", 0, 0, 0x7FFFFFFF, 0, "Time for the server to run");
        runForTime = v0;
    }
    integer = v0->current.integer;
    if (integer)
    {
        if (1000 * integer < sv.levelTime)
        {
            Com_Printf(15, "ALLGOOD - quit after good run of %d time.\n", sv.levelTime);
            Com_Quit_f();
        }
    }
}

void SV_WakeServer()
{
    if (!com_inServerFrame)
    {
        com_inServerFrame = 1;
        if (sv.smp)
        {
            Sys_WakeServer();
        }
        else if (!sv.inFrame)
        {
            sv.inFrame = 1;
            SV_PreFrame();
        }
    }
}

void __cdecl SV_WaitServer()
{
    const dvar_s *v0; // r11

    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 887, 0, "%s", "Sys_IsMainThread()");
    if (com_inServerFrame)
    {
        com_inServerFrame = 0;
        v0 = com_sv_running;
        if (!com_sv_running)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 893, 0, "%s", "com_sv_running");
            v0 = com_sv_running;
        }
        if (!v0->current.enabled)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp",
                894,
                0,
                "%s",
                "com_sv_running->current.enabled");
        if (sv.smp)
        {
            Sys_SleepServer();
            if (!sv.restartServerThread)
                Sys_DisallowSendClientMessages();
            if (!sv.pendingSnapshot)
            {
                Sys_EnterCriticalSection(CRITSECT_CLIENT_MESSAGE);
                if (sv.clientMessageTimeout)
                {
                    com_time = sv.levelTime;
                    sv.clientMessageTimeout = 0;
                    sv.levelTime += 50;
                    sv.timeResidual = 0;
                    sv.pendingSnapshot = 1;
                }
                Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
            }
            while (!Sys_WaitServer())
                Com_CheckSyncFrame();
        }
        else if (!sv.clientMessageTimeout)
        {
            sv.clientMessageTimeout = 1;
            SV_RunFrame(SV_FRAME_DO_ALL, 0);
        }
    }
}

void __cdecl SV_InitSnapshot()
{
    if (com_inServerFrame)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 942, 0, "%s", "!com_inServerFrame");
    sv.restartServerThread = 1;
    Sys_InitServerEvents();
    sv.smp = sv_smp->current.enabled;
    sv.inFrame = 0;
    sv.clientMessageTimeout = 0;
    sv.pendingSnapshot = 0;
    SV_SendClientGameState(svs.clients);
    sv.state = SS_GAME;
    CL_FirstSnapshot();
    sv.levelTime = G_GetTime();
}

void __cdecl SV_WaitSaveGame()
{
    if (sv.requestSaveGame)
    {
        sv.requestSaveGame = 0;
        //__lwsync();
        sv.savingGame = 1;
        do
            Sys_Sleep(1);
        while (sv.savingGame);
    }
}

void __cdecl SV_BeginSaveGame()
{
    if (!Sys_IsMainThread())
    {
        sv.requestSaveGame = 1;
        while (!sv.savingGame)
            Sys_Sleep(1);
    }
}

void __cdecl SV_EndSaveGame()
{
    if (!Sys_IsMainThread())
    {
        //__lwsync();
        sv.savingGame = 0;
    }
}

int __cdecl SV_WaitServerSnapshot()
{
    int v1; // r29
    int timeResidual; // r11

    iassert(Sys_IsMainThread());

    if (!sv.pendingSnapshot)
        return 0;

    sv.pendingSnapshot = 0;

    if (sv.smp)
    {
        v1 = Sys_Milliseconds();
        while (!Sys_WaitServerSnapshot())
        {
            Com_CheckError();
            if (!com_inServerFrame)
            {
                com_inServerFrame = 1;
                if (sv.smp)
                {
                    Sys_WakeServer();
                }
                else if (!sv.inFrame)
                {
                    sv.inFrame = 1;
                    SV_PreFrame();
                }
            }
            Sys_AllowSendClientMessages();
            Sys_EnterCriticalSection(CRITSECT_CLIENT_MESSAGE);
            if (!sv.clientMessageTimeout)
                Sys_SetServerTimeout(0);
            Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
            Com_CheckSyncFrame();

            if (g_kisakScriptDebuggerHack)
            {
                Scr_DisplayDebugger();
            }

            g_kisakScriptDebuggerHack = false;
        }
        sv.waitSnapshotTime = Sys_Milliseconds() - v1;
        Sys_DisallowSendClientMessages();
        Sys_EnterCriticalSection(CRITSECT_CLIENT_MESSAGE);
        sv.clientMessageTimeout = 0;
        timeResidual = sv.timeResidual;
        if (sv.timeResidual >= 50)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp",
                1054,
                0,
                "%s",
                "sv.timeResidual < FRAME_MSEC");
            timeResidual = sv.timeResidual;
        }
        Sys_SetServerTimeout(50 - timeResidual);
        Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
        Sys_ClientMessageReceived();
        return 1;
    }
    else
    {
        SV_WaitServer();
        if (SV_ProcessPendingSaves())
            SV_DisplaySaveErrorUI(); // savedevice_xenon
        Scr_UpdateDebugger();
        SV_UpdateDemo();
        SV_SendClientMessages();
        CL_CreateNextSnap();
        sv.inFrame = 0;
        sv.clientMessageTimeout = 0;
        if (!cl_paused->current.integer)
            SV_WakeServer();
        return 1;
    }
}

bool __cdecl SV_ReachedServerCommandThreshold()
{
    client_t *clients; // r11

    clients = svs.clients;
    if (!svs.clients)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1088, 0, "%s", "svs.clients");
        clients = svs.clients;
    }
    return clients->reliableCommands.header.sequence - clients->reliableCommands.header.sent >= 32;
}

void __cdecl SV_FrameInternal(int msec)
{
    int timeResidual; // r11
    int v3; // r11
    int v4; // r30
    int v5; // r29
    int v6; // r11
    int v7; // r4
    int v8; // r4
    int v9; // r3

    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1104, 0, "%s", "Sys_IsMainThread()");
    if (msec < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1105, 0, "%s", "msec >= 0");
    if (!com_inServerFrame)
    {
        com_inServerFrame = 1;
        if (sv.smp)
        {
            Sys_WakeServer();
        }
        else if (!sv.inFrame)
        {
            sv.inFrame = 1;
            SV_PreFrame();
        }
    }
    if (sv.pendingSnapshot)
    {
        CL_SetFrametime(msec, 0);
        return;
    }
    timeResidual = sv.timeResidual;
    if (sv.timeResidual >= 50)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1118, 0, "%s", "sv.timeResidual < FRAME_MSEC");
        timeResidual = sv.timeResidual;
    }
    v3 = timeResidual + msec;
    sv.timeResidual = v3;
    if (sv.levelTime > 2130706432)
    {
        Com_Shutdown("EXE_SERVERRESTARTTIMEWRAP");
        return;
    }
    if (svs.nextSnapshotEntities >= 2130706432)
    {
        Com_Shutdown("EXE_SERVERRESTARTSNAPSHOTWRAP");
        return;
    }
    v4 = 50 - v3;
    if (sv.smp)
    {
        Sys_EnterCriticalSection(CRITSECT_CLIENT_MESSAGE);
        if (v4 > 0)
        {
            if (!sv.clientMessageTimeout)
            {
                v5 = sv.levelTime - v4;
                if (sv.levelTime - v4 - com_time >= 0)
                {
                    Sys_DisallowSendClientMessages();
                    Sys_SetServerTimeout(v4);
                    Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
                    com_time = v5;
                    CL_SetFrametime(msec, msec);
                    return;
                }
            }
        LABEL_26:
            sv.timeResidual = 50;
        }
    }
    else if (v4 > 0)
    {
        if (sv.levelTime - v4 - com_time >= 0)
        {
            com_time = sv.levelTime - v4;
            CL_SetFrametime(msec, msec);
            return;
        }
        goto LABEL_26;
    }
    if (!CL_DemoPlaying())
    {
        if (sv.pendingSnapshot)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1181, 0, "%s", "!sv.pendingSnapshot");
        sv.pendingSnapshot = 1;
    }
    v6 = sv.timeResidual - 50;
    sv.levelTime += 50;
    sv.timeResidual = v6;
    if (v6 < 0)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1187, 0, "%s", "sv.timeResidual >= 0");
        v6 = sv.timeResidual;
    }
    while (1)
    {
        if (sv.clearTimeResidual || cl_paused->current.integer)
            goto LABEL_54;
        if (SV_ReachedServerCommandThreshold())
        {
            v6 = sv.timeResidual;
        LABEL_54:
            msec -= v6;
            sv.clearTimeResidual = 0;
            if (msec >= 0)
                goto LABEL_59;
            v8 = 1195;
            goto LABEL_58;
        }
        v7 = sv.timeResidual;
        if (sv.timeResidual < 50)
            goto LABEL_60;
        if (sv.smp)
            break;
        SV_WaitServer();
        if (SV_ProcessPendingSaves())
            SV_DisplaySaveErrorUI(); // savedevice_xenon
        Scr_UpdateDebugger();
        SV_UpdateDemo();
        sv.inFrame = 0;
        sv.clientMessageTimeout = 0;
        if (!com_inServerFrame)
        {
            com_inServerFrame = 1;
            if (sv.smp)
            {
                Sys_WakeServer();
            }
            else
            {
                sv.inFrame = 1;
                SV_PreFrame();
            }
        }
    LABEL_50:
        v6 = sv.timeResidual - 50;
        sv.levelTime += 50;
        sv.timeResidual = v6;
        if (v6 < 0)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1242, 0, "%s", "sv.timeResidual >= 0");
            v6 = sv.timeResidual;
        }
        sv.partialFrametime += 50;
    }
    if (sv.demo.forwardMsec)
    {
        SV_WaitServer();
        if (SV_ProcessPendingSaves())
            SV_DisplaySaveErrorUI(); // savedevice_xenon
        Scr_UpdateDebugger();
        SV_UpdateDemo();
        sv.smp = 0;
        sv.inFrame = 0;
        sv.clientMessageTimeout = 0;
        if (!com_inServerFrame)
        {
            com_inServerFrame = 1;
            sv.inFrame = 1;
            SV_PreFrame();
        }
        SV_WaitServer();
        sv.smp = 1;
        sv.inFrame = 0;
        sv.clientMessageTimeout = 0;
        goto LABEL_50;
    }
    msec -= sv.timeResidual;
    if (msec >= 0)
        goto LABEL_59;
    v8 = 1210;
LABEL_58:
    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", v8, 0, "%s", "msec >= 0");
LABEL_59:
    v7 = 0;
    sv.timeResidual = 0;
LABEL_60:
    if (sv.smp)
    {
        if (sv.clientMessageTimeout)
        {
            v9 = 50 - v7;
            sv.clientMessageTimeout = 0;
        }
        else
        {
            v9 = 0;
        }
        Sys_SetServerTimeout(v9);
        Sys_LeaveCriticalSection(CRITSECT_CLIENT_MESSAGE);
        v7 = sv.timeResidual;
    }
    com_time = sv.levelTime + v7 - 50;
    CL_SetFrametime(msec, v7);
    SV_ExitAfterTime();
}

int __cdecl SV_GetPartialFrametime()
{
    return sv.partialFrametime;
}

int __cdecl SV_ForwardFrame()
{
    __int64 v1; // r10
    int integer; // r27
    int forwardMsec; // r3
    int levelTime; // r11
    int v5; // r30
    int v6; // r30

    if (!sv.demo.forwardMsec)
        return 0;
    SV_WaitServer();
    integer = cl_paused->current.integer;
    if (integer)
        Dvar_SetInt(cl_paused, 0);
    sv.timeResidual = 0;
    forwardMsec = sv.demo.forwardMsec;
    if (sv.demo.forwardMsec != 50)
    {
        Scr_EnableBreakpoints(0);
        forwardMsec = sv.demo.forwardMsec;
    }
    if (sv.smp)
    {
        SV_WakeServer();
        SV_WaitServer();
        if (sv.pendingSnapshot)
        {
            forwardMsec = sv.demo.forwardMsec;
        }
        else
        {
            forwardMsec = sv.demo.forwardMsec - 50;
            sv.demo.forwardMsec = forwardMsec;
            if (forwardMsec < 0)
            {
                forwardMsec = 0;
                sv.demo.forwardMsec = 0;
            }
        }
    }
    levelTime = sv.levelTime;
    v5 = sv.levelTime;
    if (forwardMsec)
    {
        SV_FrameInternal(forwardMsec);
        levelTime = sv.levelTime;
    }
    LODWORD(v1) = v5;
    v6 = levelTime - v5;
    sv.partialFrametime = 0;
    Com_Printf(
        15,
        "\n=== Replay moved forward %d msec from time %g. ===\n\n",
        v6,
        v1 * 0.001f);
        //(unsigned int)HIDWORD(COERCE_UNSIGNED_INT64((float)((float)v1 * (float)0.001))),
        //(float)((float)v1 * (float)0.001));
    sv.demo.forwardMsec -= v6;
    if (sv.demo.forwardMsec < 0)
        sv.demo.forwardMsec = 0;
    Scr_EnableBreakpoints(1);
    if (!cl_paused->current.integer)
    {
        if (integer)
            Dvar_SetInt(cl_paused, integer);
    }
    CL_SetSkipRendering(0);
    return 1;
}

int __cdecl SV_ClientFrameRateFix(int msec)
{
    const dvar_t *v2; // r11
    volatile int serverExecTime; // r11
    int v4; // r10
    int v5; // r10
    int v6; // r9
    int result; // r3

    v2 = sv_clientFrameRateFix;
    if (!sv_clientFrameRateFix)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp", 1372, 0, "%s", "sv_clientFrameRateFix");
        v2 = sv_clientFrameRateFix;
    }
    if (!v2->current.enabled)
        return msec;
    serverExecTime = sv.serverExecTime;
    if (!sv.serverExecTime)
        return msec;
    if (!msec)
        return msec;
    v4 = msec - sv.waitSnapshotTime;
    if (msec - sv.waitSnapshotTime <= 0)
        return msec;
    if (sv.serverExecTime > 200)
        serverExecTime = 200;
    v6 = __ROL4__(50 * v4, 1);
    v5 = 50 * v4 / serverExecTime;
    //__twllei(serverExecTime, 0); // LWSS: I think these instructions are just traps for bad values?
    //__twlgei(serverExecTime & ~(v6 - 1), 0xFFFFFFFF);
    if (msec >= v5)
        msec = v5;
    result = 1;
    if (msec >= 1)
        return msec;
    return result;
}

int __cdecl SV_Frame(int msec)
{
    int v1; // r27
    int v2; // r3
    int v3; // r3
    int v4; // r3
    int timeResidual; // r11
    int v7; // r11

    v1 = msec;
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    //PIXSetMarker(0xFFFFFFFF, "SV_Frame");
    if (!com_sv_running->current.enabled)
    {
    LABEL_6:
        CL_SetFrametime(v1, 0);
        return v1;
    }
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if (!CL_IsCGameRendering())
    {
        SV_WaitServer();
        Com_CheckError();
        if (SV_ProcessPendingSaves())
            SV_DisplaySaveErrorUI(); // savedevice_xenon
        Scr_UpdateDebugger();
        SV_UpdateDemo();
        goto LABEL_6;
    }
    if (!CL_DemoPlaying() && SV_CheckLoadGame())
    {
        CL_SetFrametime(0, 0);
        return v1;
    }
    if ((unsigned __int8)SV_ForwardFrame())
        return v1;
    if (!cl_paused->current.integer)
    {
        timeResidual = sv.timeResidual;
        if (sv.timeResidual >= 50)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp",
                1458,
                0,
                "%s",
                "sv.timeResidual < FRAME_MSEC");
            timeResidual = sv.timeResidual;
        }
        sv.partialFrametime = 50 - timeResidual;
        v1 = SV_ClientFrameRateFix(v1);
        sv.waitSnapshotTime = 0;
        SV_FrameInternal(v1);
        v7 = sv.timeResidual;
        if (sv.timeResidual >= 50)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_main.cpp",
                1469,
                0,
                "%s",
                "sv.timeResidual < FRAME_MSEC");
            v7 = sv.timeResidual;
        }
        if (sv.smp && v7)
            sv.partialFrametime = 50 - v7;
        return v1;
    }
    SV_WaitServer();

    if (SV_ProcessPendingSaves())
        SV_DisplaySaveErrorUI(); // savedevice_xenon

    Scr_UpdateDebugger();
    SV_UpdateDemo();
    CL_SetFrametime(v1, 0);
    return v1;
}

bool __cdecl SV_SaveMemory_IsRecentlyLoaded()
{
    bool IsRecentlyLoaded; // r31

    if (sv.demo.playing)
        return SV_DemoIsRecentlyLoaded();
    IsRecentlyLoaded = SaveMemory_IsRecentlyLoaded();
    SV_RecordIsRecentlyLoaded(IsRecentlyLoaded);
    return IsRecentlyLoaded;
}

