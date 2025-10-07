#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "server.h"
#include "sv_public.h"
#include <universal/q_shared.h>
#include <ui/ui.h>
#include <database/database.h>
#include <qcommon/cmd.h>
#include "sv_game.h"
#include <qcommon/com_bsp.h>
#include <client/cl_demo.h>
#include <game/savedevice.h>
#include <game/savememory.h>
#include <game/g_main.h>
#include <cgame/cg_main.h>
#include <client/cl_scrn.h>
#include <universal/profile.h>
#include <qcommon/threads.h>
#include <universal/com_files.h>
#include <universal/com_sndalias.h>

const dvar_t *sv_clientFrameRateFix;
const dvar_t *sv_loadMyChanges;

client_t g_sv_clients[1];

void __cdecl TRACK_sv_init()
{
    track_static_alloc_internal(g_sv_clients, 55080, "g_sv_clients", 9);
}

void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize)
{
    if (bufferSize < 1)
        Com_Error(ERR_DROP, "SV_GetConfigstring: bufferSize == %i", bufferSize);
    if (index >= ARRAY_COUNT(sv.configstrings))
        Com_Error(ERR_DROP, "SV_GetConfigstring: bad index %i", index);

    iassert(sv.configstrings[index]);
    I_strncpyz(buffer, SL_ConvertToString(sv.configstrings[index]), bufferSize);
}

unsigned int __cdecl SV_GetConfigstringConst(unsigned int index)
{
    iassert((unsigned)index < MAX_CONFIGSTRINGS);
    iassert(sv.configstrings[index]);

    return sv.configstrings[index];
}

void __cdecl SV_InitReliableCommandsForClient(client_t *cl)
{
    Com_Memset(&cl->reliableCommands.header, 0, sizeof(serverCommandsHeader_t));
}

void __cdecl SV_FreeReliableCommandsForClient(client_t *cl)
{
    Com_Memset(&cl->reliableCommands.header, 0, sizeof(serverCommandsHeader_t));
}

void __cdecl SV_AddReliableCommand(client_t *cl, int index, const char *cmd)
{
    const char *v5; // r11
    int v8; // r30
    int v9; // r10
    char *i; // r11
    int v11; // r9

    v5 = cmd;
    while (*(unsigned __int8 *)v5++)
        ;
    v8 = v5 - cmd - 1;
    if (v5 - cmd + cl->reliableCommands.header.rover > 0x2000)
    {
        SV_DumpServerCommands(cl);
        Com_Error(ERR_DROP, "Reliable command buffer overflow");
    }
    v9 = 0;
    cl->reliableCommands.commands[index] = cl->reliableCommands.header.rover;
    for (i = &cl->reliableCommands.buf[cl->reliableCommands.header.rover]; v9 < v8; ++i)
    {
        v11 = (unsigned __int8)cmd[v9];
        *i = v11;
        if (v11 == 37)
            *i = 46;
        ++v9;
    }
    *i = 0;
    cl->reliableCommands.header.rover += v8 + 1;
}

void __cdecl SV_Startup()
{
    if (svs.initialized)
        Com_Error(ERR_FATAL, "SV_Startup() - already initialized");

    svs.clients = g_sv_clients;
    svs.numSnapshotEntities = MAX_GENTITIES;
    svs.initialized = 1;
    Dvar_SetBool(com_sv_running, 1);
}

void __cdecl SV_ClearServer()
{
    unsigned __int16 *configstrings; // r31

    if (svs.clients)
        Com_Memset(&svs.clients->reliableCommands, 0, 12);
    configstrings = sv.configstrings;
    do
    {
        if (*configstrings)
            SL_RemoveRefToString(*configstrings);
        ++configstrings;
    } while ((int)configstrings < (int)&sv.svEntities[0].worldSector);
    if (sv.emptyConfigString)
        SL_RemoveRefToString(sv.emptyConfigString);
    Com_Memset(&sv, 0, sizeof(server_t));
    SV_ClearPendingSaves();
    com_inServerFrame = 0;
}

void __cdecl SV_StartMap(int randomSeed)
{
    com_inServerFrame = 0;
    sv.state = SS_LOADING;
    com_time = 0;
    sv.skelTimeStamp = 0;
    Dvar_SetInt(cl_paused, 1);
}

void __cdecl SV_Settle()
{
    int v0; // r30

    v0 = 5;
    do
    {
        sv.demo.forwardMsec -= 50;
        if (sv.demo.forwardMsec < 0)
            sv.demo.forwardMsec = 0;
        SV_PreFrame();
        SV_RunFrame(SV_FRAME_DO_ALL, 0);
        --v0;
    } while (v0);
}

int __cdecl SV_SaveImmediately(const char *levelName)
{
    int v3; // r3

    if (!sv_saveOnStartMap->current.enabled)
        return 0;
    R_EndRemoteScreenUpdate();
    UI_SetActiveMenu(0, UIMENU_SAVE_LOADING);
    R_BeginRemoteScreenUpdate();
    R_EndRemoteScreenUpdate();
    R_BeginRemoteScreenUpdate();
    v3 = CL_ControllerIndexFromClientNum(0);
    //GamerProfile_UpdateProfileFromDvars(v3, PROFILE_WRITE_IF_CHANGED);
    SV_ClearPendingSaves();
    SV_AddPendingSave(levelName, "Start Level Save", "", SAVE_TYPE_AUTOSAVE, 6u, 1);
    return SV_ProcessPendingSaves();
}

void __cdecl SV_LoadLevelAssets(const char *mapname)
{
    XZoneInfo zoneInfo; // [sp+50h] [-20h] BYREF

    zoneInfo.name = mapname;
    //zoneInfo.allocFlags = 2;
    //zoneInfo.freeFlags = 2;
    // LWSS: I am changing the flags here 2 -> 8. This is accurate to SP on PC.
    // Unloading one of the other zones causes an error with mp/defaultstringtable not being found.
    // That file is the default stringtable file, but is only located in `code_post_gfx_mp` which is not loaded at all in SP
    zoneInfo.allocFlags = 8;
    zoneInfo.freeFlags = 8;
    DB_LoadXAssets(&zoneInfo, 1, 0);
    if (sv_loadMyChanges->current.enabled)
    {
        Cbuf_ExecuteBuffer(0, CL_ControllerIndexFromClientNum(0), "loadzone mychanges\n");
    }
}

bool __cdecl SV_Loaded()
{
    return sv.state == SS_GAME;
}

void __cdecl SV_Init()
{
    const char *v0; // r5
    unsigned __int16 v1; // r4

    //Memcard_InitializeSystem(); KISAKSAVE KISAKMEMCARD
    SaveDevice_Init();
    SV_AddOperatorCommands();
    sv_gameskill = Dvar_RegisterInt("g_gameskill", 1, 0, 3, 0x64u, "Game skill level");
    sv_player_maxhealth = Dvar_RegisterInt("g_player_maxhealth", 100, 10, 2000, 2u, "Maximum player health");
    sv_player_damageMultiplier = Dvar_RegisterFloat("player_damageMultiplier", 1.0, 0.0, 1000.0, 0, 0);
    player_healthEasy = Dvar_RegisterInt("player_healthEasy", 500, 10, 2000, 2u, "Player health on easy mode");
    player_healthMedium = Dvar_RegisterInt("player_healthMedium", 275, 10, 2000, 2u, "Player health in medium mode");
    player_healthHard = Dvar_RegisterInt("player_healthHard", 165, 10, 2000, 2u, "Player health in challenging mode");
    player_healthFu = Dvar_RegisterInt("player_healthFu", 115, 10, 2000, 2u, "Player health in veteran mode");
    sv_player_deathInvulnerableTime = Dvar_RegisterInt(
        "player_deathInvulnerableTime",
        1000,
        0,
        0x7FFFFFFF,
        0x1082u,
        "Time player is invlunerable just before death");
    sv_mapname = Dvar_RegisterString("mapname", "", 0x44u, "current map name");
    sv_lastSaveGame = Dvar_RegisterString("sv_lastSaveGame", "", 1u, "Last save game file name");
    sv_saveOnStartMap = Dvar_RegisterBool("sv_saveOnStartMap", 0, 0x1004u, "Save at the start of a level");
    sv_saveGameAvailable = Dvar_RegisterBool(
        "sv_saveGameAvailable",
        0,
        0x44u,
        "True if the save game is currently available");
    sv_saveGameNotReadable = Dvar_RegisterBool(
        "sv_saveGameNotReadable",
        0,
        0x44u,
        "True if the save game is not readable");
    sv_saveDeviceAvailable = Dvar_RegisterBool(
        "sv_saveDeviceAvailable",
        0,
        0x44u,
        "True if the save device is currently available");
    sv_cheats = Dvar_RegisterBool("sv_cheats", 1, 0x48u, "Enable server cheats");
    replay_autosave = Dvar_RegisterInt(
        "replay_autosave",
        30,
        0,
        0x7FFFFFFF,
        0,
        "Use autosaves as part of demos - will make demo access faster but will cause hitches");
    replay_asserts = Dvar_RegisterBool("replay_asserts", 1, 0, "Enable/Disable replay aborts due to inconsistency");
    SV_InitDemoSystem();
    nextmap = Dvar_RegisterString("nextmap", "", 0, "Next map to load");
    Dvar_RegisterInt("g_reloading", 0, 0, 4, 0x40u, "True if the game is currently reloading");
    sv_smp = Dvar_RegisterBool("sv_smp", 1, 0, "Enable server multithreading");
    sv_loadMyChanges = Dvar_RegisterBool("sv_loadMyChanges", 0, 0, "Load my changes fast file on devmap.");
    sv_clientFrameRateFix = Dvar_RegisterBool(
        "sv_clientFrameRateFix",
        1,
        0x1004u,
        "Slow down server frame time to allow good client frame rate with server bound.");
}

void __cdecl SV_Shutdown(const char *finalmsg)
{
    serverStatic_t *v1; // r11
    int v2; // ctr

    if (com_sv_running && com_sv_running->current.enabled)
    {
        //LSP_LogStringEvenIfControllerIsInactive("server shutdown");
        //LSP_ForceSendPacket();
        Com_Printf(15, "----- Server Shutdown -----\n");
        SV_RemoveOperatorCommands();
        SV_ShutdownGameProgs();
        SaveMemory_CleanupSaveMemory();
        SaveMemory_ShutdownSaveSystem();
        SV_ClearServer();
        v1 = &svs;
        v2 = 10;
        do
        {
            v1->initialized = 0;
            v1 = (serverStatic_t *)((char *)v1 + 4);
            --v2;
        } while (v2);
        Dvar_SetBool(com_sv_running, 0);
        Dvar_SetFloat(com_timescale, 1.0);
        Com_Printf(15, "---------------------------\n");
        CL_Disconnect(0);
    }
}

void __cdecl SV_SetConfigstring(unsigned int index, const char *val)
{
    unsigned int v4; // r31
    const char *v5; // r3
    const char *v6; // r11
    int v7; // r9
    unsigned __int16 v8; // r3
    client_t *clients; // r27
    const char *v10; // r11
    int v12; // r30
    int v13; // r29
    const char *v14; // r31
    char v15[1120]; // [sp+50h] [-460h] BYREF

    if (index > MAX_CONFIGSTRINGS)
        Com_Error(ERR_DROP, "SV_SetConfigstring: bad index %i", index);
    v4 = index;
    if (sv.configstrings[index])
    {
        if (!val)
            val = "";
        v5 = SL_ConvertToString(sv.configstrings[index]);
        v6 = val;
        do
        {
            v7 = *(unsigned __int8 *)v6 - *(unsigned __int8 *)v5;
            if (!*v6)
                break;
            ++v6;
            ++v5;
        } while (!v7);
        if (v7)
        {
            SL_RemoveRefToString(sv.configstrings[v4]);
            v8 = (int)index < 1114 ? SL_GetString_(val, 0, 19) : SL_GetLowercaseString_(val, 0, 19);
            sv.configstrings[v4] = v8;
            if (sv.state == SS_GAME)
            {
                clients = svs.clients;
                if (svs.clients->state == 1)
                {
                    v10 = val;
                    while (*(unsigned __int8 *)v10++)
                        ;
                    v12 = v10 - val - 1;
                    if (v12 < 1000)
                    {
                        SV_SendServerCommand(svs.clients, "cs %i %s", index, val);
                    }
                    else
                    {
                        v13 = 0;
                        do
                        {
                            if (v13)
                            {
                                v14 = "bcs2";
                                if (v12 >= 1000)
                                    v14 = "bcs1";
                            }
                            else
                            {
                                v14 = "bcs0";
                            }
                            I_strncpyz(v15, &val[v13], 1000);
                            SV_SendServerCommand(clients, "%s %i %s", v14, index, v15);
                            v12 -= 999;
                            v13 += 999;
                        } while (v12 > 0);
                    }
                }
            }
        }
    }
    else if (val)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_init.cpp", 91, 0, "%s", "!val");
    }
}

void SV_SaveSystemInfo()
{
    const char *v0; // r3
    const char *v1; // r3
    char v2[0x2000]; // [sp+50h] [-2010h] BYREF

    v0 = Dvar_InfoString_Big(8);
    I_strncpyz(v2, v0, 0x2000);
    dvar_modifiedFlags &= ~8u;
    SV_SetConfigstring(1u, v2);
    v1 = Dvar_InfoString(0, 4);
    SV_SetConfigstring(0, v1);
    dvar_modifiedFlags &= ~4u;
}

void __cdecl SV_SpawnServer(const char *mapname, int savegame)
{
    int startTime; // r21
    int v7; // r22
    int v9; // r30
    SaveGame *v11; // [sp+50h] [-B0h] BYREF
    char filename[160]; // [sp+60h] [-A0h] BYREF

    Com_SyncThreads();
    startTime = Sys_Milliseconds();
    Sys_BeginLoadThreadPriorities(); // (FROM MP)

    // MP ADD
    if (IsFastFileLoad())
    {
        char zoneName[64];
        XZoneInfo zoneInfo;

        DB_ResetZoneSize(0);
        Com_sprintf(zoneName, 0x40u, "%s_load", mapname);
        zoneInfo.name = zoneName;
        zoneInfo.allocFlags = 32;
        zoneInfo.freeFlags = 96;
        DB_LoadXAssets(&zoneInfo, 1, 0);
    }
    // MP END


    CL_InitLoad(mapname);
    // MP ADD
    if (IsFastFileLoad())
    {
        DB_SyncXAssets();
        DB_UpdateDebugZone();
    }
    // MP END

    //LSP_LogString(cl_controller_in_use, va("loading map: %s", server));
    CL_MapLoading(mapname);
    R_BeginRemoteScreenUpdate();
    if (fs_debug->current.integer == 2)
        Dvar_SetInt((dvar_s *)fs_debug, 0);
    //Live_Frame(CL_ControllerIndexFromClientNum(0), 0);
    
    ProfLoad_Activate();

    {
        PROF_SCOPED("Clear load game");
        SV_ClearLoadGame();
        iassert(sv_gameskill);
    }
    
    {
        PROF_SCOPED("Shutdown systems");
        R_EndRemoteScreenUpdate();
        R_BeginRemoteScreenUpdate();
        R_EndRemoteScreenUpdate();
        //SCR_UpdateLoadScreen(); // NULLSUB
        CL_ShutdownAll(false);
        SV_ShutdownGameProgs();
        SaveMemory_CleanupSaveMemory();
        SaveMemory_ShutdownSaveSystem();
        Com_Printf(15, "------ Server Initialization ------\n");
        Com_Printf(15, "Server: %s\n", mapname);
        SV_ClearServer();
    }


    {
        PROF_SCOPED("Shutdown file system");
        v7 = Sys_MillisecondsRaw();
        Com_Restart();
        if (!com_sv_running->current.enabled)
            SV_Startup();
    }
    
    {
        PROF_SCOPED("After file system restart");
        {
            PROF_SCOPED("Start loading");
            CL_StartLoading(mapname);
        }

#ifndef KISAK_NO_FASTFILES
        if (*mapname)
        {
            PROF_SCOPED("Load fast file");
            SV_LoadLevelAssets(mapname);
        }
#endif

        R_BeginRemoteScreenUpdate();
        UI_LoadIngameMenus();
        svs.nextSnapshotEntities = 0;
        Dvar_SetString(nextmap, "map_restart");

        iassert(!strstr(mapname, "\\"));
        Dvar_SetString(sv_mapname, mapname);
    }

    {
        PROF_SCOPED("allocate empty config strings");
        sv.emptyConfigString = SL_GetString_((char *)"", 0, 19);
        for (int i = 0; i < MAX_CONFIGSTRINGS; ++i)
        {
            iassert(!sv.configstrings[i]);
            sv.configstrings[i] = SL_GetString_((char *)"", 0, 19);
        }

        SCR_UpdateLoadScreen();
    }

    Com_GetBspFilename(filename, 64, mapname);
    {
#ifdef KISAK_NO_FASTFILES
        if (!IsFastFileLoad())
            Com_LoadBsp(filename);
#endif
        PROF_SCOPED("Load collision (server)");
        CM_LoadMap(filename, &sv.checksum);
    }

    Com_LoadWorld(filename);
    //Live_SetCurrentMapname(server);
    SCR_UpdateLoadScreen();
    SCR_UpdateLoadScreen();
    SaveMemory_InitializeSaveSystem();
    SaveMemory_ClearDemoSave();

    // MP ADD
    if (!IsFastFileLoad())
    {
        Com_GetBspFilename(filename, 0x40u, mapname);
        Com_LoadSoundAliases(filename, "all_mp", SASYS_GAME);
    }
    // MP END

    SV_InitGameProgs(v7, savegame, &v11);
    CL_SetSkipRendering(1);
    if (CL_DemoPlaying())
        CL_FinishLoadingDemo();
    SCR_UpdateLoadScreen();
    {
        PROF_SCOPED("Save system info");
        SV_SaveSystemInfo();
    }

    Com_Printf(15, "-----------------------------------\n");

    SCR_UpdateLoadScreen();
    {
        PROF_SCOPED("Send client game state");
        SV_SendClientGameState(svs.clients);
    }
    
    SCR_UpdateLoadScreen();
    {
        PROF_SCOPED("Init client");
        CL_InitCGame(0, v11 != 0);
    }

    SCR_UpdateLoadScreen();
    {
        PROF_SCOPED("Check load level");
        SV_CheckLoadLevel(v11);
    }

    SCR_UpdateLoadScreen();
    {
        PROF_SCOPED("Event loop");
        Com_EventLoop();
    }

    SCR_UpdateLoadScreen();

    Dvar_SetInt(cl_paused, 1);
    SV_InitSnapshot();
    v9 = 0;
    if (!savegame)
    {
        PROF_SCOPED("Save game");
        v9 = SV_SaveImmediately(mapname);
    }

    {
        PROF_SCOPED("Register sounds");
        CG_RegisterSounds();
    }

    R_EndRemoteScreenUpdate();
    DB_SyncXAssets();
    //ProfLoad_Deactivate();
    UI_SetActiveMenu(0, UIMENU_PREGAME);
    if (v9)
        SV_DisplaySaveErrorUI();
    CL_SetActive();
    Com_Printf(15, "Load time: %d msec\n", Sys_Milliseconds() - startTime);
    Com_ResetFrametime();
}

