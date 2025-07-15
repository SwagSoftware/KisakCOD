#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "server.h"

int sv_loadScripts;
int sv_map_restart;

void __cdecl SV_SetValuesFromSkill()
{
    unsigned int unsignedInt; // r11
    __int64 v1; // r11
    int v2; // r4
    __int64 v3; // r11
    __int64 v4; // r11
    __int64 v5; // r11

    Dvar_SetInt(sv_player_maxhealth, 100);
    unsignedInt = sv_gameskill->current.unsignedInt;
    if (unsignedInt)
    {
        if (unsignedInt == 1)
        {
            HIDWORD(v4) = player_healthMedium->current.integer;
            LODWORD(v4) = HIDWORD(v4);
            Dvar_SetFloat(sv_player_damageMultiplier, (float)((float)100.0 / (float)v4));
            v2 = 1000;
        }
        else if (unsignedInt < 3)
        {
            HIDWORD(v3) = player_healthHard->current.integer;
            LODWORD(v3) = HIDWORD(v3);
            Dvar_SetFloat(sv_player_damageMultiplier, (float)((float)100.0 / (float)v3));
            v2 = 100;
        }
        else
        {
            if (unsignedInt != 3)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp",
                    73,
                    0,
                    "%s",
                    "sv_gameskill->current.integer == DIFFICULTY_FU");
            HIDWORD(v1) = player_healthFu->current.integer;
            LODWORD(v1) = HIDWORD(v1);
            Dvar_SetFloat(sv_player_damageMultiplier, (float)((float)100.0 / (float)v1));
            v2 = 100;
        }
    }
    else
    {
        HIDWORD(v5) = player_healthEasy->current.integer;
        LODWORD(v5) = HIDWORD(v5);
        Dvar_SetFloat(sv_player_damageMultiplier, (float)((float)100.0 / (float)v5));
        v2 = 4000;
    }
    Dvar_SetInt(sv_player_deathInvulnerableTime, v2);
}

void SV_DifficultyEasy()
{
    Dvar_SetInt(sv_gameskill, 0);
    SV_SetValuesFromSkill();
}

void SV_DifficultyMedium()
{
    Dvar_SetInt(sv_gameskill, 1);
    SV_SetValuesFromSkill();
}

void SV_DifficultyHard()
{
    Dvar_SetInt(sv_gameskill, 2);
    SV_SetValuesFromSkill();
}

void SV_DifficultyFu()
{
    Dvar_SetInt(sv_gameskill, 3);
    SV_SetValuesFromSkill();
}

int __cdecl ReadSaveHeader(const char *filename, SaveHeader *header)
{
    int v5; // r31
    void *v6; // [sp+50h] [-20h] BYREF

    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 143, 0, "%s", "filename");
    MemCard_SetUseDevDrive(g_useDevSaveArea);
    if (OpenDevice(filename, &v6) >= 0)
    {
        v5 = ReadFromDevice(header, 1108, v6);
        CloseDevice(v6);
        MemCard_SetUseDevDrive(0);
        if (v5 == 1108)
        {
            if (header->saveVersion == 287)
            {
                return 1;
            }
            else
            {
                Com_Printf(15, "Bad save version %d, expecting %d\n", header->saveVersion, 287);
                return 0;
            }
        }
        else
        {
            Com_Printf(15, "Bad save read.\n");
            return 0;
        }
    }
    else
    {
        MemCard_SetUseDevDrive(0);
        Com_Printf(15, "Can't find savegame %s\n", filename);
        return 0;
    }
}

int __cdecl ExtractMapStringFromSaveGame(const char *filename, char *mapname)
{
    char *v5; // r11
    int v6; // r10
    const dvar_s *v7; // r3
    SaveHeader v8; // [sp+50h] [-480h] BYREF

    if (!mapname)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 189, 0, "%s", "mapname");
    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 190, 0, "%s", "filename");
    if (!(unsigned __int8)ReadSaveHeader(filename, &v8))
        return 0;
    v5 = v8.mapName;
    do
    {
        v6 = (unsigned __int8)*v5;
        (v5++)[mapname - v8.mapName] = v6;
    } while (v6);
    v7 = Dvar_RegisterString("ui_campaign", "american", 0x1000u, "Campaign");
    Dvar_SetString(v7, v8.campaign);
    return 1;
}

void __cdecl ShowLoadErrorsSummary(const char *mapName, unsigned int count, int a3)
{
    const char *v3; // r6
    int v4; // r11
    char *v5; // r11
    char *v7; // r11
    char *v9; // r11
    char v11[1024]; // [sp+50h] [-410h] BYREF

    v3 = mapName;
    v4 = 0;
    if (com_errorPrintsCount)
    {
        if (count == 1)
            Com_sprintf(v11, 1024, ERRMSG_SINGLE, mapName);
        else
            Com_sprintf(v11, 1024, ERRMSG_PLURAL, mapName);
        v5 = v11;
        while (*v5++)
            ;
        v4 = v5 - v11 - 1;
    }
    if (com_consoleLogOpenFailed)
    {
        Com_sprintf(&v11[v4], 1024 - v4, "\nUNABLE TO OPEN %s", "console.log");
        v7 = v11;
        while (*v7++)
            ;
        v4 = v7 - v11 - 1;
    }
    if (com_missingAssetOpenFailed)
    {
        Com_sprintf(&v11[v4], 1024 - v4, "\nUNABLE TO OPEN %s", "missingasset.csv");
        v9 = v11;
        while (*v9++)
            ;
        v4 = v9 - v11 - 1;
    }
    if (v4)
        Com_Error(ERR_MAPLOADERRORSUMMARY, v11, a3, v3);
}

void __cdecl SV_ClearLoadGame()
{
    sv_save_filename[0] = 0;
    sv_map_restart = 0;
    sv_loadScripts = 0;
}

void __cdecl SV_MapRestart(int savegame, int loadScripts)
{
    int integer; // r28
    int v5; // r3
    SaveGame *v6; // [sp+50h] [-40h] BYREF

    Com_SyncThreads();
    track_hunk_ClearToStart();
    integer = cl_paused->current.integer;
    sv_save_filename[0] = 0;
    sv_map_restart = 0;
    sv_loadScripts = 0;
    if (com_sv_running->current.enabled)
    {
        if (loadScripts)
            Con_Close(0);
        v5 = Sys_Milliseconds();
        SV_RestartGameProgs(v5, savegame, &v6, loadScripts);
        CL_Restart();
        SV_CheckLoadLevel(v6);
        Dvar_SetInt(cl_paused, integer);
        Com_EventLoop();
        SV_InitSnapshot();
        if (!savegame)
        {
            if (loadScripts)
            {
                SV_ClearPendingSaves();
                SV_AddPendingSave("map_restart", "Start Level Save", "", SAVE_TYPE_AUTOSAVE, 2u, 1);
                SV_ProcessPendingSaves();
            }
        }
        CL_SetActive();
        Com_ResetFrametime();
    }
    else
    {
        Com_Printf(15, "Game is not running.\n");
    }
}

int __cdecl CheckForSaveGame(char *mapname, char *filename)
{
    char *v4; // r11
    int v5; // r10
    SaveGame *SaveHandle; // r31
    const SaveHeader *Header; // r31
    char *v8; // r11
    int v9; // r10
    char *v11; // r11
    int v12; // r10
    SaveHeader v13; // [sp+50h] [-490h] BYREF

    if (!mapname)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 396, 0, "%s", "mapname");
    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 397, 0, "%s", "filename");
    v4 = sv_save_filename;
    do
    {
        v5 = (unsigned __int8)*v4;
        (v4++)[filename - sv_save_filename] = v5;
    } while (v5);
    sv_save_filename[0] = 0;
    if ((unsigned __int8)SaveMemory_IsCommittedSaveAvailable(filename, sv.checksum))
    {
        SaveHandle = SaveMemory_GetSaveHandle(2u);
        if (!SaveHandle)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 406, 0, "%s", "lastCommittedSave");
        Header = SaveMemory_GetHeader(SaveHandle);
        if (!Header)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 408, 0, "%s", "lastCommittedHeader");
        v8 = Header->mapName;
        do
        {
            v9 = (unsigned __int8)*v8;
            (v8++)[mapname - Header->mapName] = v9;
        } while (v9);
        G_SetPendingLoadName(filename);
        return 1;
    }
    else if ((unsigned __int8)ReadSaveHeader(filename, &v13))
    {
        v11 = v13.mapName;
        do
        {
            v12 = (unsigned __int8)*v11;
            (v11++)[mapname - v13.mapName] = v12;
        } while (v12);
        G_SetPendingLoadName(filename);
        return 1;
    }
    else
    {
        return 0;
    }
}

int __cdecl SV_CheckLoadGame()
{
    int result; // r3
    bool v1; // zf
    const char *v2; // r3
    char v3[64]; // [sp+50h] [-150h] BYREF
    char v4[264]; // [sp+90h] [-110h] BYREF

    if (sv_map_restart)
    {
        SV_MapRestart(0, sv_loadScripts);
        return 1;
    }
    else if (sv_save_filename[0])
    {
        v1 = (unsigned __int8)CheckForSaveGame(v4, v3) == 0;
        result = 0;
        if (!v1)
        {
            SND_StopSounds(SND_STOP_ALL);
            if (!com_sv_running->current.enabled || I_stricmp(v4, sv_mapname->current.string))
            {
                if (sv_cheats->current.enabled)
                    v2 = va("devmap %s\n", v3);
                else
                    v2 = va("map %s\n", v3);
                Cbuf_AddText(0, v2);
                return 1;
            }
            else
            {
                SV_MapRestart(1, 0);
                return 1;
            }
        }
    }
    else
    {
        return 0;
    }
    return result;
}

void __cdecl SV_RequestMapRestart(int loadScripts)
{
    CL_FlushDebugServerData();
    CL_UpdateDebugServerData();
    CL_ShutdownDebugData();
    sv_map_restart = 1;
    sv_loadScripts = loadScripts;
    if (!com_sv_running->current.enabled)
        SV_CheckLoadGame();
}

void __cdecl SV_FastRestart_f()
{
    CL_FlushDebugServerData();
    CL_UpdateDebugServerData();
    CL_ShutdownDebugData();
    sv_map_restart = 1;
    sv_loadScripts = 0;
    if (!com_sv_running->current.enabled)
        SV_CheckLoadGame();
}

void __cdecl SV_MapRestart_f()
{
    CL_FlushDebugServerData();
    CL_UpdateDebugServerData();
    CL_ShutdownDebugData();
    sv_map_restart = 1;
    sv_loadScripts = 1;
    if (!com_sv_running->current.enabled)
        SV_CheckLoadGame();
}

// attributes: thunk
void __cdecl SV_NextLevel_f()
{
    G_LoadNextMap();
}

void __cdecl SV_LoadGame_f()
{
    int nesting; // r7
    char *v1; // r31
    int v2; // r11
    char *v3; // r11
    unsigned int v5; // r3
    char *i; // r3
    char v7[64]; // [sp+50h] [-50h] BYREF

    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
            182,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] <= 1)
        v1 = "";
    else
        v1 = (char *)*((unsigned int *)sv_cmd_args.argv[nesting] + 1);
    if (!*v1)
    {
        Com_Printf(0, "You must specify a savegame to load\n");
        return;
    }
    if (!I_strncmp(v1, "save", 4) && ((v2 = v1[4], v2 == 47) || v2 == 92))
    {
        I_strncpyz(v7, v1, 64);
        v3 = v7;
        while (*v3++)
            ;
        v5 = v3 - v7 - 1;
    }
    else
    {
        v5 = Com_sprintf(v7, 64, "save/%s", v1);
        if (v5 >= 0x40)
        {
            Com_Printf(0, "filename 'save/%s' is too long", v1);
            return;
        }
    }
    if (v5 < 4 || I_stricmp(&v7[v5 - 4], ".svg"))
        I_strncat(v7, 64, ".svg");
    for (i = strchr(v7, 92); i; i = strchr(i, 92))
        *i = 47;
    g_useDevSaveArea = 1;
    if (Com_BuildPlayerProfilePath(sv_save_filename, 64, v7) < 0)
    {
        Com_Printf(0, "filename '%s' is too long", v7);
        sv_save_filename[0] = 0;
    }
    if (!com_sv_running->current.enabled)
        SV_CheckLoadGame();
}

void __cdecl SV_ForceSelectSaveDevice_f()
{
    int v0; // r31

    v0 = CL_ControllerIndexFromClientNum(0);
    if (Gamer//Profile_IsProfileLoggedIn(v0) )
        {
          UI_ShowReadingSaveDevicePopup();
          Memcard_SelectSaveDevice(v0, 1, 1572864);
          UI_HideReadingSaveDevicePopup();
        }
}

void __cdecl SV_SelectSaveDevice_f()
{
    int v0; // r30
    bool v1; // r3

    v0 = CL_ControllerIndexFromClientNum(0);
    Dvar_SetBool(sv_saveGameAvailable, 0);
    if (Gamer//Profile_IsProfileLoggedIn(v0) )
        {
          if (CL_IsLocalClientInGame(0))
          {
            Com_Printf(
              0,
              "select_save_device is not available while a game is running - use force_select_save_device instead\n");
          }
          else
          {
            UI_ShowReadingSaveDevicePopup();
            if (Memcard_IsDeviceAvailable(v0))
            {
              v1 = SaveExists(CONSOLE_DEFAULT_SAVE_NAME);
              Dvar_SetBool(sv_saveGameAvailable, v1);
            }
            else
            {
              Memcard_SelectSaveDevice(v0, 0, 1572864);
              Memcard_WaitForSaveDeviceSelection();
            }
            UI_HideReadingSaveDevicePopup();
          }
        }
}

void __cdecl CheckSaveExists(const char *filename)
{
    __int64 v2; // r10
    __int64 v3; // r8
    __int64 v4; // r6
    int v5; // [sp+8h] [-58h]
    int v6; // [sp+Ch] [-54h]
    int v7; // [sp+10h] [-50h]
    int v8; // [sp+14h] [-4Ch]
    int v9; // [sp+18h] [-48h]
    int v10; // [sp+1Ch] [-44h]
    int v11; // [sp+20h] [-40h]
    int v12; // [sp+24h] [-3Ch]

    if (!(unsigned __int8)SaveMemory_IsCommittedSaveAvailable(filename, sv.checksum) && !SaveExists(filename))
    {
        HIDWORD(v4) = &unk_8207A214;
        sv_save_filename[0] = 0;
        sv_map_restart = 0;
        LODWORD(v2) = &g_msgBuf[632];
        sv_loadScripts = 0;
        G_SaveError(ERR_DROP, SAVE_ERROR_MISSING_DEVICE, v4, v3, v2, v5, v6, v7, v8, v9, v10, v11, v12);
    }
}

void __cdecl SV_LoadGameContinue_f()
{
    __int64 v0; // r10
    __int64 v1; // r8
    __int64 v2; // r6
    int v3; // [sp+8h] [-58h]
    int v4; // [sp+Ch] [-54h]
    int v5; // [sp+10h] [-50h]
    int v6; // [sp+14h] [-4Ch]
    int v7; // [sp+18h] [-48h]
    int v8; // [sp+1Ch] [-44h]
    int v9; // [sp+20h] [-40h]
    int v10; // [sp+24h] [-3Ch]

    if (SV_IsDemoPlaying() && SV_DemoHasMark())
    {
        SV_DemoRestart_f();
    }
    else
    {
        g_useDevSaveArea = 0;
        I_strncpyz(sv_save_filename, CONSOLE_DEFAULT_SAVE_NAME, 64);
        CheckSaveExists(sv_save_filename);
        if (!com_sv_running->current.enabled && !SV_CheckLoadGame())
        {
            HIDWORD(v2) = &unk_8207A214;
            G_SaveError(ERR_DROP, SAVE_ERROR_MISSING_DEVICE, v2, v1, v0, v3, v4, v5, v6, v7, v8, v9, v10);
        }
    }
}

// attributes: thunk
void __cdecl SV_ScriptUsage_f()
{
    Scr_DumpScriptThreads();
}

void SV_ScriptVarUsage_f_usage()
{
    Com_Printf(
        1,
        "usage: scriptVarUsage [-spreadsheet] [-summary] [-functionSummary] [-total] [-filename name]  [-function name] [-lin"
        "enumber number] [-minCount number] [-lineSort]\n");
    Com_Printf(1, "    -spreadsheet     - tab delimited with one entry per line\n");
    Com_Printf(1, "    -summary         - display totals per file\n");
    Com_Printf(1, "    -functionSummary - display totals per function\n");
    Com_Printf(1, "    -total           - display grand total only\n");
    Com_Printf(1, "    -filename        - filter files based on the specified string\n");
    Com_Printf(1, "    -function        - filter lines based on the function name\n");
    Com_Printf(1, "    -minCount        - only show variables used more often than this count\n");
    Com_Printf(1, "    -lineSort        - sort by line number instead of number of variables\n");
}

void SV_ScriptVarUsage_f()
{
    bool v0; // r27
    int v1; // r31
    bool v2; // r14
    bool v3; // r15
    int v4; // r26
    const char *v5; // r24
    const char *v6; // r25
    int nesting; // r7
    const char *v8; // r3
    const char *v9; // r3
    const char *v10; // r3
    const char *v11; // r3
    const char *v12; // r3
    const char *v13; // r3
    int v14; // r7
    int v15; // r31
    const char *v16; // r3
    const char *v17; // r3
    int v18; // r7
    int v19; // r31
    const char *v20; // r3
    const char *v21; // r3
    int v22; // r7
    int v23; // r31
    const char *v24; // r3
    bool v25; // [sp+50h] [-2C0h]
    bool v26; // [sp+51h] [-2BFh]
    char v27[272]; // [sp+60h] [-2B0h] BYREF
    char v28[416]; // [sp+170h] [-1A0h] BYREF

    v0 = 0;
    v1 = 1;
    v26 = 0;
    v2 = 0;
    v3 = 0;
    v4 = 0;
    v5 = 0;
    v6 = 0;
LABEL_2:
    v25 = v0;
    while (1)
    {
        nesting = sv_cmd_args.nesting;
        if (sv_cmd_args.nesting >= 8u)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
                167,
                0,
                "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                sv_cmd_args.nesting,
                8);
            nesting = sv_cmd_args.nesting;
        }
        if (v1 >= sv_cmd_args.argc[nesting])
            break;
        v8 = SV_Cmd_Argv(v1);
        if (I_stricmp("-spreadsheet", v8))
        {
            v9 = SV_Cmd_Argv(v1);
            if (!I_stricmp("-summary", v9))
            {
                v0 = 1;
                ++v1;
                goto LABEL_2;
            }
            v10 = SV_Cmd_Argv(v1);
            if (I_stricmp("-lineSort", v10))
            {
                v11 = SV_Cmd_Argv(v1);
                if (I_stricmp("-functionSummary", v11))
                {
                    v12 = SV_Cmd_Argv(v1);
                    if (I_stricmp("-total", v12))
                    {
                        v13 = SV_Cmd_Argv(v1);
                        if (I_stricmp("-filename", v13))
                        {
                            v17 = SV_Cmd_Argv(v1);
                            if (I_stricmp("-function", v17))
                            {
                                v21 = SV_Cmd_Argv(v1);
                                if (I_stricmp("-minCount", v21))
                                    goto LABEL_31;
                                v22 = sv_cmd_args.nesting;
                                v23 = v1 + 1;
                                if (sv_cmd_args.nesting >= 8u)
                                {
                                    MyAssertHandler(
                                        "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
                                        167,
                                        0,
                                        "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                                        sv_cmd_args.nesting,
                                        8);
                                    v22 = sv_cmd_args.nesting;
                                }
                                if (v23 >= sv_cmd_args.argc[v22])
                                {
                                LABEL_31:
                                    SV_ScriptVarUsage_f_usage();
                                    return;
                                }
                                v24 = SV_Cmd_Argv(v23);
                                v4 = atol(v24);
                                v1 = v23 + 1;
                            }
                            else
                            {
                                v18 = sv_cmd_args.nesting;
                                v19 = v1 + 1;
                                if (sv_cmd_args.nesting >= 8u)
                                {
                                    MyAssertHandler(
                                        "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
                                        167,
                                        0,
                                        "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                                        sv_cmd_args.nesting,
                                        8);
                                    v18 = sv_cmd_args.nesting;
                                }
                                if (v19 >= sv_cmd_args.argc[v18])
                                    goto LABEL_31;
                                v20 = SV_Cmd_Argv(v19);
                                I_strncpyz(v28, v20, 260);
                                v6 = v28;
                                v1 = v19 + 1;
                            }
                        }
                        else
                        {
                            v14 = sv_cmd_args.nesting;
                            v15 = v1 + 1;
                            if (sv_cmd_args.nesting >= 8u)
                            {
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
                                    167,
                                    0,
                                    "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                                    sv_cmd_args.nesting,
                                    8);
                                v14 = sv_cmd_args.nesting;
                            }
                            if (v15 >= sv_cmd_args.argc[v14])
                                goto LABEL_31;
                            v16 = SV_Cmd_Argv(v15);
                            I_strncpyz(v27, v16, 260);
                            v5 = v27;
                            v1 = v15 + 1;
                        }
                    }
                    else
                    {
                        ++v1;
                        v26 = 1;
                    }
                }
                else
                {
                    v3 = 1;
                    v0 = 1;
                    ++v1;
                }
            }
            else
            {
                v2 = 1;
                v0 = 1;
                ++v1;
            }
        }
        else
        {
            v0 = 1;
            ++v1;
        }
    }
    Scr_DumpScriptVariables(v0, v25, v26, v3, v2, v5, v6, v4);
}

void SV_Script//Profile_f()
{
  int nesting; // r7
  const char *v1; // r3
  long double v2; // fp2

  nesting = sv_cmd_args.nesting;
  if (sv_cmd_args.nesting >= 8u)
  {
    MyAssertHandler(
      "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
      182,
      0,
      "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
      sv_cmd_args.nesting,
      8);
    nesting = sv_cmd_args.nesting;
  }
  if (sv_cmd_args.argc[nesting] <= 1)
    v1 = "";
  else
    v1 = (const char *)*((unsigned int *)sv_cmd_args.argv[nesting] + 1);
  v2 = atof(v1);
  Scr_DoProfile((float)*(double *)&v2);
}

void SV_ScriptBuiltin_f()
{
    int nesting; // r7
    const char *v1; // r3
    long double v2; // fp2

    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
            182,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] <= 1)
        v1 = "";
    else
        v1 = (const char *)*((unsigned int *)sv_cmd_args.argv[nesting] + 1);
    v2 = atof(v1);
    Scr_DoProfileBuiltin((float)*(double *)&v2);
}

void SV_ScriptProfileReset_f()
{
    //Profile_ClearScriptCounters();
    Scr_ScriptProfileTimesReset();
}

// attributes: thunk
void __cdecl SV_ScriptProfileFile_f()
{
    Scr_PrintScriptProfileTimesForScriptFile();
}

// attributes: thunk
void __cdecl SV_StringUsage_f()
{
    MT_DumpTree();
}

void SV_SaveGame_f()
{
    const dvar_s *v0; // r11
    char *cmd; // r11

    v0 = com_sv_running;
    if (!com_sv_running)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 874, 0, "%s", "com_sv_running");
        v0 = com_sv_running;
    }
    if (v0->current.enabled && sv.state == SS_GAME)
    {
        if (sv_cheats->current.enabled)
        {
            if (Dvar_GetInt("g_reloading"))
            {
                Com_Printf(0, "savegame request ignored\n");
            }
            else
            {
                SV_Cmd_ArgvBuffer(1, sv.cmd, 1024);
                cmd = sv.cmd;
                while (*cmd++)
                    ;
                if (cmd - sv.cmd == 1)
                {
                    Com_Printf(0, "syntax: devsave <name>\n");
                }
                else
                {
                    if (strrchr(sv.cmd, 46))
                        *strrchr(sv.cmd, 46) = 0;
                    SV_Cmd_ArgvBuffer(2, sv.cmd2, 1024);
                    SV_Cmd_ArgvBuffer(3, sv.cmd3, 1024);
                    SV_Cmd_ArgvBuffer(4, sv.cmd4, 1024);
                    if (com_inServerFrame)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 910, 0, "%s", "!com_inServerFrame");
                    SV_AddPendingSave(sv.cmd, sv.cmd2, sv.cmd3, SAVE_TYPE_CONSOLE, 6u, 0);
                }
            }
        }
        else
        {
            Com_Printf(0, "Saving games from the console is cheat protected.\n");
        }
    }
    else
    {
        Com_Printf(0, "Game is not running.\n");
    }
}

void __cdecl SV_SaveGameLastCommit_f()
{
    SaveGame *LastCommittedSave; // r31

    LastCommittedSave = SaveMemory_GetLastCommittedSave();
    if (!LastCommittedSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_ccmds.cpp", 922, 0, "%s", "save");
    if (SaveMemory_WriteSaveToDevice(LastCommittedSave))
        SV_DisplaySaveErrorUI();
}

void __cdecl SV_RemoveOperatorCommands()
{
    ;
}

void SV_Map_f()
{
    char *v0; // r3
    bool v1; // r31
    __int64 v2; // r10
    __int64 v3; // r8
    __int64 v4; // r6
    int v5; // r5
    int v6; // [sp+8h] [-E8h]
    int v7; // [sp+Ch] [-E4h]
    int v8; // [sp+10h] [-E0h]
    int v9; // [sp+14h] [-DCh]
    int v10; // [sp+18h] [-D8h]
    int v11; // [sp+1Ch] [-D4h]
    int v12; // [sp+20h] [-D0h]
    int v13; // [sp+24h] [-CCh]
    char v14[64]; // [sp+50h] [-A0h] BYREF
    char v15[72]; // [sp+90h] [-60h] BYREF

    com_errorPrintsCount = 0;
    SV_Cmd_ArgvBuffer(1, v14, 64);
    if (!ui_skipMainLockout->current.enabled)
        SV_SelectSaveDevice_f();
    I_strlwr(v14);
    v0 = strstr(v14, ".svg");
    v1 = v0 != 0;
    if (v0)
    {
        I_strncpyz(v15, v14, 64);
        if (!(unsigned __int8)ExtractMapStringFromSaveGame(v15, v14))
        {
            HIDWORD(v4) = &unk_8207A5B4;
            G_SaveError(ERR_DROP, SAVE_ERROR_MISSING_DEVICE, v4, v3, v2, v6, v7, v8, v9, v10, v11, v12, v13);
        }
    }
    Dvar_SetBool(sv_cheats, 1);
    CL_ShutdownDemo();
    FS_ConvertPath(v14);
    SV_SpawnServer(v14, v1);
    ShowLoadErrorsSummary(v14, com_errorPrintsCount, v5);
}

void __cdecl SV_AddOperatorCommands()
{
    if (!initialized_0)
    {
        initialized_0 = 1;
        Cmd_AddCommandInternal("fast_restart", Cbuf_AddServerText_f, &SV_FastRestart_f_VAR);
        Cmd_AddServerCommandInternal("fast_restart", SV_FastRestart_f, &SV_FastRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("map_restart", Cbuf_AddServerText_f, &SV_MapRestart_f_VAR);
        Cmd_AddServerCommandInternal("map_restart", SV_MapRestart_f, &SV_MapRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("spmap", Cbuf_AddServerText_f, &SV_Map_f_VAR_2);
        Cmd_AddServerCommandInternal("spmap", SV_Map_f, &SV_Map_f_VAR_SERVER_2);
        Cmd_AddCommandInternal("map", Cbuf_AddServerText_f, &SV_Map_f_VAR_1);
        Cmd_AddServerCommandInternal("map", SV_Map_f, &SV_Map_f_VAR_SERVER_1);
        Cmd_AddCommandInternal("devmap", Cbuf_AddServerText_f, &SV_Map_f_VAR_0);
        Cmd_AddServerCommandInternal("devmap", SV_Map_f, &SV_Map_f_VAR_SERVER_0);
        Cmd_AddCommandInternal("spdevmap", Cbuf_AddServerText_f, &SV_Map_f_VAR);
        Cmd_AddServerCommandInternal("spdevmap", SV_Map_f, &SV_Map_f_VAR_SERVER);
        Cmd_SetAutoComplete("map", "maps", "d3dbsp");
        Cmd_SetAutoComplete("spmap", "maps", "d3dbsp");
        Cmd_SetAutoComplete("devmap", "maps", "d3dbsp");
        Cmd_SetAutoComplete("spdevmap", "maps", "d3dbsp");
        Cmd_AddCommandInternal("loadgame", Cbuf_AddServerText_f, &SV_LoadGame_f_VAR);
        Cmd_AddServerCommandInternal("loadgame", SV_LoadGame_f, &SV_LoadGame_f_VAR_SERVER);
        Cmd_SetAutoComplete("loadgame", "save", "svg");
        Cmd_AddCommandInternal("nextlevel", Cbuf_AddServerText_f, &SV_NextLevel_f_VAR);
        Cmd_AddServerCommandInternal("nextlevel", SV_NextLevel_f, &SV_NextLevel_f_VAR_SERVER);
        Cmd_AddCommandInternal("select_save_device", Cbuf_AddServerText_f, &SV_SelectSaveDevice_f_VAR);
        Cmd_AddServerCommandInternal("select_save_device", SV_SelectSaveDevice_f, &SV_SelectSaveDevice_f_VAR_SERVER);
        Cmd_AddCommandInternal("force_select_save_device", Cbuf_AddServerText_f, &SV_ForceSelectSaveDevice_f_VAR);
        Cmd_AddServerCommandInternal(
            "force_select_save_device",
            SV_ForceSelectSaveDevice_f,
            &SV_ForceSelectSaveDevice_f_VAR_SERVER);
        Cmd_AddCommandInternal("loadgame_continue", Cbuf_AddServerText_f, &SV_LoadGameContinue_f_VAR);
        Cmd_AddServerCommandInternal("loadgame_continue", SV_LoadGameContinue_f, &SV_LoadGameContinue_f_VAR_SERVER);
        Cmd_AddCommandInternal("scriptUsage", Cbuf_AddServerText_f, &SV_ScriptUsage_f_VAR);
        Cmd_AddServerCommandInternal("scriptUsage", SV_ScriptUsage_f, &SV_ScriptUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("scriptVarUsage", Cbuf_AddServerText_f, &SV_ScriptVarUsage_f_VAR);
        Cmd_AddServerCommandInternal("scriptVarUsage", SV_ScriptVarUsage_f, &SV_ScriptVarUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("difficultyEasy", Cbuf_AddServerText_f, &SV_DifficultyEasy_VAR);
        Cmd_AddServerCommandInternal("difficultyEasy", SV_DifficultyEasy, &SV_DifficultyEasy_VAR_SERVER);
        Cmd_AddCommandInternal("difficultyMedium", Cbuf_AddServerText_f, &SV_DifficultyMedium_VAR);
        Cmd_AddServerCommandInternal("difficultyMedium", SV_DifficultyMedium, &SV_DifficultyMedium_VAR_SERVER);
        Cmd_AddCommandInternal("difficultyHard", Cbuf_AddServerText_f, &SV_DifficultyHard_VAR);
        Cmd_AddServerCommandInternal("difficultyHard", SV_DifficultyHard, &SV_DifficultyHard_VAR_SERVER);
        Cmd_AddCommandInternal("difficultyFu", Cbuf_AddServerText_f, &SV_DifficultyFu_VAR);
        Cmd_AddServerCommandInternal("difficultyFu", SV_DifficultyFu, &SV_DifficultyFu_VAR_SERVER);
        Cmd_AddCommandInternal("scriptProfile", Cbuf_AddServerText_f, &SV_Script//Profile_f_VAR);
            Cmd_AddServerCommandInternal("scriptProfile", SV_Script//Profile_f, &SV_Script//Profile_f_VAR_SERVER);
                Cmd_AddCommandInternal("scriptBuiltin", Cbuf_AddServerText_f, &SV_ScriptBuiltin_f_VAR);
        Cmd_AddServerCommandInternal("scriptBuiltin", SV_ScriptBuiltin_f, &SV_ScriptBuiltin_f_VAR_SERVER);
        Cmd_AddCommandInternal("profile_script_reset", Cbuf_AddServerText_f, &SV_ScriptProfileReset_f_VAR);
        Cmd_AddServerCommandInternal("profile_script_reset", SV_ScriptProfileReset_f, &SV_ScriptProfileReset_f_VAR_SERVER);
        Cmd_AddCommandInternal("profile_script_by_file_dump", Cbuf_AddServerText_f, &SV_ScriptProfileFile_f_VAR);
        Cmd_AddServerCommandInternal(
            "profile_script_by_file_dump",
            SV_ScriptProfileFile_f,
            &SV_ScriptProfileFile_f_VAR_SERVER);
        Cmd_AddCommandInternal("stringUsage", Cbuf_AddServerText_f, &SV_StringUsage_f_VAR);
        Cmd_AddServerCommandInternal("stringUsage", SV_StringUsage_f, &SV_StringUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("devsave", Cbuf_AddServerText_f, &SV_SaveGame_f_VAR);
        Cmd_AddServerCommandInternal("devsave", SV_SaveGame_f, &SV_SaveGame_f_VAR_SERVER);
        Cmd_AddCommandInternal("savegame_lastcommit", Cbuf_AddServerText_f, &SV_SaveGameLastCommit_f_VAR);
        Cmd_AddServerCommandInternal("savegame_lastcommit", SV_SaveGameLastCommit_f, &SV_SaveGameLastCommit_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_save", Cbuf_AddServerText_f, &SV_SaveDemo_f_VAR);
        Cmd_AddServerCommandInternal("replay_save", SV_SaveDemo_f, &SV_SaveDemo_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_restart", Cbuf_AddServerText_f, &SV_DemoRestart_f_VAR);
        Cmd_AddServerCommandInternal("replay_restart", SV_DemoRestart_f, &SV_DemoRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_mark", Cbuf_AddServerText_f, &SV_DemoMark_f_VAR);
        Cmd_AddServerCommandInternal("replay_mark", SV_DemoMark_f, &SV_DemoMark_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_goto", Cbuf_AddServerText_f, &SV_DemoGoto_f_VAR);
        Cmd_AddServerCommandInternal("replay_goto", SV_DemoGoto_f, &SV_DemoGoto_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_back", Cbuf_AddServerText_f, &SV_DemoBack_f_VAR);
        Cmd_AddServerCommandInternal("replay_back", SV_DemoBack_f, &SV_DemoBack_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_forward", Cbuf_AddServerText_f, &SV_DemoForward_f_VAR);
        Cmd_AddServerCommandInternal("replay_forward", SV_DemoForward_f, &SV_DemoForward_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_full_forward", Cbuf_AddServerText_f, &SV_DemoFullForward_f_VAR);
        Cmd_AddServerCommandInternal("replay_full_forward", SV_DemoFullForward_f, &SV_DemoFullForward_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_live", Cbuf_AddServerText_f, &SV_DemoLive_f_VAR);
        Cmd_AddServerCommandInternal("replay_live", SV_DemoLive_f, &SV_DemoLive_f_VAR_SERVER);
        Cmd_AddCommandInternal("replay_info", Cbuf_AddServerText_f, &SV_DemoInfo_f_VAR);
        Cmd_AddServerCommandInternal("replay_info", SV_DemoInfo_f, &SV_DemoInfo_f_VAR_SERVER);
    }
}

