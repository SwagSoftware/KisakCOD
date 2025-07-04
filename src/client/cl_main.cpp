#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"

clientConnection_t clientConnections[1];
clientUIActive_t clientUIActives[1];
clientActive_t clients[1];
clientStatic_t cls;

// struct dvar_s const *const input_invertPitch 827f8df8     cl_main.obj
// struct dvar_s const *const cl_avidemo 827f8dfc     cl_main.obj
// struct dvar_s const *const cl_testAnimWeight 8283b220     cl_main.obj
// struct dvar_s const *const cl_freemoveScale 8283b224     cl_main.obj
// struct dvar_s const *const motd       8283b228     cl_main.obj
// struct dvar_s const *const cl_sensitivity 8283b22c     cl_main.obj
// struct dvar_s const *const cl_forceavidemo 8283b230     cl_main.obj
// struct dvar_s const *const m_yaw      8283b234     cl_main.obj
// struct dvar_s const *const m_pitch    8283b238     cl_main.obj
// struct dvar_s const *const nextdemo   8283b23c     cl_main.obj
// struct dvar_s const **arcadeScore 8283b240     cl_main.obj
// struct dvar_s const *const cl_freemove 8283b28c     cl_main.obj
// struct dvar_s const *const cl_showMouseRate 8283b4f4     cl_main.obj
// struct dvar_s const *const takeCoverWarnings 8283b4f8     cl_main.obj
// int marker_cl_main       8283b4fc     cl_main.obj
// struct dvar_s const *const m_forward  8283b500     cl_main.obj
// struct dvar_s const *const cheat_items_set2 8283b504     cl_main.obj
// struct dvar_s const *const cl_mouseAccel 8283b508     cl_main.obj
// struct dvar_s const *const cheat_points 8283b50c     cl_main.obj
// struct dvar_s const *const input_viewSensitivity 8283b510     cl_main.obj
// struct dvar_s const *const input_autoAim 8283b514     cl_main.obj
// struct dvar_s const *const cl_inGameVideo 8283b52c     cl_main.obj
// struct dvar_s const *const cl_noprint 8283b530     cl_main.obj
// struct dvar_s const *const m_side     8283b534     cl_main.obj
// struct dvar_s const *const m_filter   8283b538     cl_main.obj
// struct dvar_s const *const cheat_items_set1 8283b53c     cl_main.obj
// struct dvar_s const *const cl_freelook 828d5ffc     cl_main.obj
// struct dvar_s const *const cl_shownet 828d6000     cl_main.obj

void __fastcall TRACK_cl_main()
{
    track_static_alloc_internal(clientUIActives, 20, "clientUIActives", 9);
    track_static_alloc_internal(clients, 633532, "clients", 9);
    track_static_alloc_internal(clientConnections, 271392, "clientConnections", 9);
    track_static_alloc_internal(&cls, 612, "cls", 9);
}

int __fastcall CL_GetLocalClientActiveCount()
{
    return 1;
}

int __fastcall CL_GetFirstActiveLocalClient()
{
    return 0;
}

bool __fastcall CL_IsLocalClientActive(int clientNum)
{
    return clientNum == 0;
}

void __fastcall CL_SetLocalClientActive(int clientNum, bool active)
{
    ;
}

int __fastcall CL_LocalClientNumFromControllerIndex(unsigned int controllerIndex)
{
    if (controllerIndex >= 4)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            219,
            0,
            "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
            controllerIndex,
            4);
    if (!cl_multi_gamepads_enabled && controllerIndex != cl_controller_in_use)
        Com_PrintError(
            14,
            "Request for controller %i's clientNum, but that controller doesn't have a clientNum because only controller %i is playing\n",
            controllerIndex,
            cl_controller_in_use);
    return 0;
}

int __fastcall CL_ControllerIndexFromClientNum(int clientIndex)
{
    if (clientIndex)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            230,
            0,
            "clientIndex doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            clientIndex,
            1);
    return cl_controller_in_use;
}

int __fastcall CL_GetFirstActiveControllerIndex()
{
    return cl_controller_in_use;
}

int __fastcall CL_AllLocalClientsInactive()
{
    return 0;
}

// attributes: thunk
void __fastcall CL_RunOncePerClientFrame(int localClientNum, int msec)
{
    IN_Frame();
}

void __fastcall CL_DumpReliableCommand(int cmdIndex, const char *cmd)
{
    Com_Printf(0, "cmd[%d] '%s'\n", cmdIndex, cmd);
}

void __fastcall CL_DumpReliableCommands(clientConnection_t *clc)
{
    int reliableAcknowledge; // r11
    int reliableSequence; // r28
    int v4; // r31
    const char *v5; // r30
    const char *v6; // r29
    int v7; // r31
    char *v8; // r30

    reliableAcknowledge = clc->reliableAcknowledge;
    reliableSequence = (unsigned __int8)clc->reliableSequence;
    v4 = (unsigned __int8)(reliableAcknowledge + 1);
    Com_Printf(
        0,
        "command numbers %d - %d = %d %d -> %d\n",
        clc->reliableSequence,
        reliableAcknowledge + 1,
        clc->reliableSequence - reliableAcknowledge - 1,
        v4,
        reliableSequence);
    if (v4 >= reliableSequence)
    {
        v6 = clc->reliableCommands[v4];
        do
        {
            Com_Printf(0, "cmd[%d] '%s'\n", v4++, v6);
            v6 += 1024;
        } while (v4 < 256);
        v7 = 0;
        v8 = clc->reliableCommands[0];
        do
        {
            Com_Printf(0, "cmd[%d] '%s'\n", v7++, v8);
            v8 += 1024;
        } while (v7 <= reliableSequence);
    }
    else
    {
        v5 = clc->reliableCommands[v4];
        do
        {
            Com_Printf(0, "cmd[%d] '%s'\n", v4++, v5);
            v5 += 1024;
        } while (v4 <= reliableSequence);
    }
}

void __fastcall CL_AddReliableCommand(int localClientNum, const char *cmd)
{
    int reliableSequence; // r11

    if (!G_DemoPlaying() && SV_Loaded())
    {
        Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
        reliableSequence = clientConnections[0].reliableSequence;
        if (clientConnections[0].reliableSequence - clientConnections[0].reliableAcknowledge > 256)
        {
            CL_DumpReliableCommands(clientConnections);
            Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
            Com_Error(ERR_DROP, "EXE_ERR_CLIENT_CMD_OVERFLOW");
            reliableSequence = clientConnections[0].reliableSequence;
        }
        clientConnections[0].reliableSequence = reliableSequence + 1;
        I_strncpyz(&clientConnections[0].reliableCommands[0][((reliableSequence + 1) << 10) & 0x3FC00], cmd, 1024);
        Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
    }
}

void __fastcall CL_ShutdownDevGui()
{
    CL_DestroyDevGui();
    DevGui_Shutdown();
    Cmd_RemoveCommand("devgui_dvar");
    Cmd_RemoveCommand("devgui_cmd");
    Cmd_RemoveCommand("devgui_open");
}

void __fastcall CL_ShutdownHunkUsers()
{
    Com_SyncThreads();
    CL_ShutdownCGame();
    if (cls.devGuiStarted)
    {
        CL_DestroyDevGui();
        DevGui_Shutdown();
        Cmd_RemoveCommand("devgui_dvar");
        Cmd_RemoveCommand("devgui_cmd");
        Cmd_RemoveCommand("devgui_open");
        cls.devGuiStarted = 0;
    }
    CL_ShutdownUI();
    if (cls.uiStarted)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 383, 0, "%s", "!cls.uiStarted");
}

void __fastcall CL_ShutdownDemo()
{
    void *demofile; // r3
    void *demobuf; // r3

    demofile = cls.demofile;
    if (cls.demofile)
    {
        if (cls.demoplaying)
        {
            G_ClearDemoEntities();
            demofile = cls.demofile;
        }
        FS_FCloseFile(demofile);
        cls.demofile = 0;
        cls.demoplaying = 0;
        if (cls.demorecording)
        {
            cls.demorecording = 0;
            demobuf = cls.demobuf;
            if (!cls.demobuf)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 417, 0, "%s", "cls.demobuf");
                demobuf = cls.demobuf;
            }
            Z_VirtualFree(demobuf, 0);
            cls.demobuf = 0;
        }
    }
}

void __fastcall CL_SaveSettings(MemoryFile *memFile)
{
    bool usingAds; // r31
    bool v3; // [sp+50h] [-30h] BYREF

    if (!MemFile_IsWriting(memFile))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 428, 0, "%s", "MemFile_IsWriting( memFile )");
    usingAds = clients[0].usingAds;
    if (clients[0].usingAds && !clients[0].usingAds)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            431,
            0,
            "%s\n\t(cl->usingAds) = %i",
            "(usingAds == 0 || usingAds == 1)",
            clients[0].usingAds);
    v3 = usingAds;
    MemFile_WriteData(memFile, 1, &v3);
}

void __fastcall CL_RestoreSettings(MemoryFile *memFile)
{
    int v2; // r31
    _BYTE v3[8]; // [sp+50h] [-20h] BYREF

    if (MemFile_IsWriting(memFile))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 439, 0, "%s", "!MemFile_IsWriting( memFile )");
    MemFile_ReadData(memFile, 1, v3);
    v2 = v3[0];
    if (v3[0] > 1u)
        Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
    if (v2 > 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            445,
            0,
            "%s\n\t(usingAds) = %i",
            "(usingAds >= 0 && usingAds <= 1)",
            v2);
    clients[0].usingAds = v2 > 0;
}

void __fastcall CL_MapLoading_CalcMovieToPlay(
    const char *buffer,
    const char *inMapName,
    char *outMovieName,
    int a4,
    int a5,
    int a6,
    int a7,
    int a8,
    int a9,
    int a10,
    int a11,
    const char *a12,
    int a13,
    const char *a14,
    int a15,
    char *a16)
{
    unsigned int v16; // r30
    char *v17; // r14
    int v18; // r28
    const char *v19; // r3
    const char *v20; // r31
    const char *v21; // r11
    const char *v22; // r10
    int v23; // r8
    const char *v24; // r11
    const char *v25; // r10
    int v26; // r8
    const MovieToPlayScriptOpInfo *i; // r28
    const char *opName; // r10
    const char *v29; // r11
    int v30; // r8
    const char *v31; // r6
    const char *v32; // r6
    int v33; // r14
    int v34; // r3
    int v35; // r14
    int v36; // r3
    int v37; // r14
    int v38; // r3
    int v39; // r14
    bool v40; // r6
    int v41; // r14
    bool v42; // r6
    int v43; // r14
    int v44; // r3
    char *v45; // r10
    char *v46; // r11
    int v47; // r6
    int v48; // r3
    const char *VariantString; // r3
    int v50; // r11
    int v51; // [sp+50h] [-2F0h]
    char v52[736]; // [sp+60h] [-2E0h] BYREF

    v16 = 0;
    a12 = buffer;
    a14 = inMapName;
    v17 = outMovieName;
    v18 = 0;
    a16 = outMovieName;
    v51 = 0;
    *outMovieName = 0;
    Com_BeginParseSession("video/cin_levels.txt");
    do
    {
        v19 = Com_Parse(&a12);
        v20 = v19;
        if (!*v19)
            break;
        if (v18)
        {
            v21 = v19;
            v22 = "then";
            do
            {
                v23 = *(unsigned __int8 *)v21 - *(unsigned __int8 *)v22;
                if (!*v21)
                    break;
                ++v21;
                ++v22;
            } while (!v23);
            if (v23)
            {
                v24 = v19;
                v25 = "if";
                do
                {
                    v26 = *(unsigned __int8 *)v24 - *(unsigned __int8 *)v25;
                    if (!*v24)
                        break;
                    ++v24;
                    ++v25;
                } while (!v26);
                if (!v26)
                    v51 = ++v18;
            }
            else
            {
                v51 = --v18;
            }
        }
        else
        {
            for (i = s_movieToPlayScriptOpInfo; ; ++i)
            {
                if (i == (const MovieToPlayScriptOpInfo *)"cls")
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
                        538,
                        0,
                        "%s",
                        "opInfo != &s_movieToPlayScriptOpInfo[ARRAY_COUNT( s_movieToPlayScriptOpInfo )]");
                opName = i->opName;
                if (!opName)
                    break;
                v29 = v20;
                do
                {
                    v30 = *(unsigned __int8 *)v29 - *(unsigned __int8 *)opName;
                    if (!*v29)
                        break;
                    ++v29;
                    ++opName;
                } while (!v30);
                if (!v30)
                    break;
            }
            if (v16 < i->inValues)
            {
                v31 = "do special command";
                if (!i->opName)
                    v31 = "push literal";
                Com_Error(ERR_FATAL, "Stack underflow in %s, trying to %s '%s'.", "video/cin_levels.txt", v31, v20);
            }
            if (i->outValues - i->inValues + v16 >= 8)
            {
                v32 = "do special command";
                if (!i->opName)
                    v32 = "push literal";
                Com_Error(ERR_FATAL, "Stack overflow in %s, trying to %s '%s'.", "video/cin_levels.txt", v32, v20);
            }
            switch (i->op)
            {
            case MTPSOP_PLUS:
                v33 = atol(&v52[64 * v16]);
                v34 = atol(&v52[64 * v16 - 64]);
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v33 + v34);
                break;
            case MTPSOP_MINUS:
                v35 = atol(&v52[64 * v16]);
                v36 = atol(&v52[64 * v16 - 64]);
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v36 - v35);
                break;
            case MTPSOP_MUL:
                v37 = atol(&v52[64 * v16]);
                v38 = atol(&v52[64 * v16 - 64]);
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v37 * v38);
                break;
            case MTPSOP_GT:
                v39 = atol(&v52[64 * v16]);
                v40 = atol(&v52[64 * v16 - 64]) > v39;
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v40);
                break;
            case MTPSOP_LT:
                v41 = atol(&v52[64 * v16]);
                v42 = atol(&v52[64 * v16 - 64]) < v41;
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v42);
                break;
            case MTPSOP_EQ:
                v43 = atol(&v52[64 * v16]);
                v44 = atol(&v52[64 * v16 - 64]);
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v43 == v44);
                break;
            case MTPSOP_STRCMP:
                v45 = &v52[64 * v16];
                v46 = &v52[64 * v16 - 64];
                do
                {
                    v47 = (unsigned __int8)*v46 - (unsigned __int8)*v45;
                    if (!*v46)
                        break;
                    ++v46;
                    ++v45;
                } while (!v47);
                snprintf(&v52[64 * v16 - 64], 0x40u, "%i", v47);
                break;
            case MTPSOP_STRCAT:
                snprintf(v52, 0x40u, "%s%s", &v52[64 * v16 - 64], &v52[64 * v16]);
                I_strncpyz(&v52[64 * v16 - 64], v52, 64);
                break;
            case MTPSOP_NOT:
                v48 = atol(&v52[64 * v16]);
                snprintf(&v52[64 * v16], 0x40u, "%i", v48 == 0);
                break;
            case MTPSOP_DUP:
                I_strncpyz(&v52[64 * v16 + 64], &v52[64 * v16], 64);
                break;
            case MTPSOP_DROP:
            case MTPSOP_THEN:
                break;
            case MTPSOP_SWAP:
                I_strncpyz(v52, &v52[64 * v16 - 64], 64);
                I_strncpyz(&v52[64 * v16 - 64], &v52[64 * v16], 64);
                I_strncpyz(&v52[64 * v16], v52, 64);
                break;
            case MTPSOP_GETDVAR:
                VariantString = Dvar_GetVariantString(&v52[64 * v16]);
                I_strncpyz(&v52[64 * v16], VariantString, 64);
                break;
            case MTPSOP_GETMAPNAME:
                I_strncpyz(&v52[64 * v16 + 64], a14, 64);
                break;
            case MTPSOP_IF:
                if (!atol(&v52[64 * v16]))
                    v51 = 1;
                break;
            case MTPSOP_PLAY:
                I_strncpyz(v17, &v52[64 * v16], 256);
                break;
            case MTPSOP_LITERAL:
                I_strncpyz(&v52[64 * v16 + 64], v20, 64);
                break;
            default:
                if (!alwaysfails)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 606, 0, "Can't happen.");
                break;
            }
            v17 = a16;
            v50 = i->outValues - i->inValues;
            v18 = v51;
            v16 += v50;
        }
    } while (!*v17);
    Com_EndParseSession();
    if (v18)
        Com_Error(ERR_FATAL, "Unterminated if in %s", "video/cin_levels.txt");
    if (!*v17)
        Com_Error(ERR_FATAL, "No loading movie specified by %s", "video/cin_levels.txt");
    if (!*v17)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 622, 0, "%s", "outMovieName[0]");
}

void __fastcall CL_MapLoading_CalcMovieToPlay_FastFile(const char *inMapName, char *outMovieName, const char *a3)
{
    XAssetHeader *XAssetHeader; // r30
    int v6; // r10
    int v7; // r9
    int v8; // r8
    int v9; // r7
    int v10; // r6
    int v11; // [sp+8h] [-78h]
    int v12; // [sp+Ch] [-74h]
    int v13; // [sp+10h] [-70h]
    const char *v14; // [sp+14h] [-6Ch]
    int v15; // [sp+18h] [-68h]
    const char *v16; // [sp+1Ch] [-64h]
    int v17; // [sp+20h] [-60h]
    char *v18; // [sp+24h] [-5Ch]

    XAssetHeader = DB_FindXAssetHeader((XAssetHeader *)0x20, (XAssetType)"video/cin_levels.txt", a3);
    if (!XAssetHeader)
        Com_Error(ERR_FATAL, "Could not open %s", "video/cin_levels.txt");
    CL_MapLoading_CalcMovieToPlay(
        (const char *)XAssetHeader[2].xmodelPieces,
        inMapName,
        outMovieName,
        v10,
        v9,
        v8,
        v7,
        v6,
        v11,
        v12,
        v13,
        v14,
        v15,
        v16,
        v17,
        v18);
}

void __fastcall CL_MapLoading_StartCinematic(const char *mapname, double volume, int a3, const char *a4)
{
    XAssetHeader *XAssetHeader; // r30
    int v7; // r10
    int v8; // r9
    int v9; // r8
    int v10; // r7
    int v11; // r6
    int v12; // [sp+8h] [-178h]
    int v13; // [sp+Ch] [-174h]
    int v14; // [sp+10h] [-170h]
    const char *v15; // [sp+14h] [-16Ch]
    int v16; // [sp+18h] [-168h]
    const char *v17; // [sp+1Ch] [-164h]
    int v18; // [sp+20h] [-160h]
    char *v19; // [sp+24h] [-15Ch]
    char v20[264]; // [sp+50h] [-130h] BYREF

    XAssetHeader = DB_FindXAssetHeader((XAssetHeader *)0x20, (XAssetType)"video/cin_levels.txt", a4);
    if (!XAssetHeader)
        Com_Error(ERR_FATAL, "Could not open %s", "video/cin_levels.txt");
    CL_MapLoading_CalcMovieToPlay(
        (const char *)XAssetHeader[2].xmodelPieces,
        mapname,
        v20,
        v11,
        v10,
        v9,
        v8,
        v7,
        v12,
        v13,
        v14,
        v15,
        v16,
        v17,
        v18,
        v19);
    R_Cinematic_StartPlayback(v20, 5u, volume);
}

void __fastcall CL_MapLoading(const char *mapname)
{
    const char *v2; // r5
    int v3; // r4
    int v4; // r3

    if (!clientUIActives[0].isRunning)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 677, 0, "%s", "clUI->isRunning");
    Con_Close(0);
    clientUIActives[0].keyCatchers = 0;
    clientUIActives[0].displayHUDWithKeycatchUI = 0;
    I_strncpyz(cls.servername, "localhost", 256);
    if (clientUIActives[0].connectionState != CA_MAP_RESTART)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            684,
            0,
            "%s\n\t(CL_GetLocalClientConnectionState( 0 )) = %i",
            "(CL_GetLocalClientConnectionState( 0 ) == CA_MAP_RESTART)",
            clientUIActives[0].connectionState);
    clientUIActives[0].connectionState = CA_LOADING;
    CL_MapLoading_StartCinematic(
        mapname,
        (float)(snd_cinematicVolumeScale->current.value * snd_volume->current.value),
        v3,
        v2);
    UI_DrawConnectScreen();
    Live_SetCurrentMapname(mapname);
    if (cl_multi_gamepads_enabled)
        Cmd_ExecuteSingleCommand(0, cl_controller_in_use, "nosplitscreen\n");
    SND_FadeAllSounds(0.0, v4);
}

void __fastcall CL_ResetSkeletonCache()
{
    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 714, 0, "%s", "Sys_IsMainThread()");
    PIXSetMarker(0xFFFFFFFF, "CL_ResetSkeletonCache");
    if (!++clients[0].skelTimeStamp)
        clients[0].skelTimeStamp = 1;
    clients[0].skelMemoryStart = (char *)((unsigned int)&clients[0].skelMemory[15] & 0xFFFFFFF0);
    clients[0].skelMemPos = 0;
}

void __fastcall CL_ClearState(int a1, int a2)
{
    unsigned __int16 *configstrings; // r31

    CG_CreateNextSnap(0, 0.0, a2);
    CG_SetNextSnap(0);
    SND_StopSounds(SND_STOP_ALL);
    configstrings = clients[0].configstrings;
    do
    {
        if (*configstrings)
            SL_RemoveRefToString(*configstrings);
        ++configstrings;
    } while ((int)configstrings < (int)clients[0].mapname);
    memset(clients, 0, sizeof(clients));
    Com_ClientDObjClearAllSkel(0);
    memset(clientConnections, 0, sizeof(clientConnections));
}

void __fastcall CL_Disconnect(int localClientNum)
{
    if (clientUIActives[0].isRunning)
    {
        SCR_StopCinematic(localClientNum);
        CL_SetLocalClientConnectionState(localClientNum, CA_DISCONNECTED);
        SND_DisconnectListener(localClientNum);
        CL_ResetLastGamePadEventTime();
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
                230,
                0,
                "clientIndex doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        Live_Disconnected(cl_controller_in_use);
    }
}

void __fastcall CL_ForwardCommandToServer(int localClientNum, const char *string)
{
    int nesting; // r7
    const char *v5; // r31
    int v6; // r10

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 0)
        v5 = byte_82003CDD;
    else
        v5 = *cmd_args.argv[nesting];
    v6 = *v5;
    if (v6 != 45)
    {
        if (cls.demoplaying || clientUIActives[0].connectionState == CA_DISCONNECTED || v6 == 43)
        {
            Com_Printf(14, "Unknown command \"%s\"\n", v5);
        }
        else if (Cmd_Argc() <= 1)
        {
            CL_AddReliableCommand(localClientNum, v5);
        }
        else
        {
            CL_AddReliableCommand(localClientNum, string);
        }
    }
}

void __fastcall CL_ForwardToServer_f()
{
    char v0[1032]; // [sp+50h] [-410h] BYREF

    if (clientUIActives[0].connectionState != CA_ACTIVE || cls.demoplaying)
    {
        Com_Printf(0, "Not connected to a server.\n");
    }
    else if (Cmd_Argc() > 1)
    {
        Cmd_ArgsBuffer(1, v0, 1024);
        CL_AddReliableCommand(0, v0);
    }
}

void __fastcall CL_ConnectResponse()
{
    if (!clientUIActives[0].isRunning)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 998, 0, "%s", "clUI->isRunning");
    clientUIActives[0].connectionState = !clientUIActives[0].cgameInitialized ? CA_LOADING : CA_MAP_RESTART;
}

void __fastcall CL_InitLoad(const char *mapname)
{
    com_expectedHunkUsage = 0;
    Dvar_SetString(nextmap, byte_82003CDD);
    if (clientUIActives[0].isRunning)
    {
        SCR_StopCinematic(0);
        clientUIActives[0].connectionState = CA_DISCONNECTED;
        SND_DisconnectListener(0);
        CL_ResetLastGamePadEventTime();
        Live_Disconnected(cl_controller_in_use);
    }
    UI_SetMap(mapname);
    clientUIActives[0].connectionState = CA_MAP_RESTART;
}

void __fastcall CL_PacketEvent(msg_t *msg, int serverMessageSequence)
{
    int readcount; // r27
    int messageNum; // r26

    clientConnections[0].lastPacketTime = cls.realtime;
    if (msg->cursize < 4 || *(unsigned int *)msg->data == -1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            1043,
            0,
            "%s",
            "msg->cursize >= 4 && *(int *)msg->data != -1");
    MSG_BeginReading(msg);
    readcount = msg->readcount;
    messageNum = clients[0].snap.messageNum;
    clientConnections[0].serverMessageSequence = serverMessageSequence;
    clientConnections[0].lastPacketTime = cls.realtime;
    CL_ParseServerMessage(msg);
    if (cls.demorecording)
    {
        CL_WriteDemoMessage(msg, readcount);
        if (messageNum != clients[0].snap.messageNum)
            CL_WriteDemoSnapshotData();
    }
}

void __fastcall CL_SetFrametime(int frametime, int animFrametime)
{
    if (frametime - animFrametime < 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            1078,
            0,
            "%s",
            "frametime - animFrametime >= 0");
    cls.frametime = frametime;
    cls.animFrametime = animFrametime;
}

void __fastcall CheckForConsoleGuidePause(int localClientNum)
{
    if (Live_IsSystemUiActive()
        && !cl_paused->current.integer
        && com_sv_running->current.enabled
        && (!R_Cinematic_IsStarted() && !R_Cinematic_IsPending() || !cg_cinematicFullscreen->current.enabled)
        && !Key_IsCatcherActive(localClientNum, 16))
    {
        UI_SetActiveMenu(localClientNum, UIMENU_INGAME);
    }
}

void __fastcall CL_Frame(
    int localClientNum,
    int msec,
    int a3,
    int a4,
    int a5,
    int a6,
    int a7,
    int a8,
    int a9,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14)
{
    int v14; // r29
    int v16; // r3
    char v21; // cr34
    const dvar_s *v22; // r11
    __int64 v23; // r11
    int v24; // r3

    v14 = msec;
    a14 = msec;
    v16 = Hunk_CheckTempMemoryClear(localClientNum);
    Hunk_CheckTempMemoryHighClear(v16);
    PIXSetMarker(0xFFFFFFFF, "CL_Frame");
    if (clientUIActives[0].isRunning)
    {
        _R11 = 0;
        _R8 = &cls.scriptError;
        do
        {
            __asm
            {
                mfmsr     r9
                mtmsree   r13
                lwarx     r10, (_cls__3UclientStatic_t__A.scriptError - 0x8283B3A0), r8# clientStatic_t cls ...
                stwcx.r11, (_cls__3UclientStatic_t__A.scriptError - 0x8283B3A0), r8# clientStatic_t cls ...
                mtmsree   r9
            }
        } while (!v21);
        if (_R10)
            UI_SetActiveMenu(0, UIMENU_MAIN);
        if (clientUIActives[0].connectionState == CA_DISCONNECTED
            && (clientUIActives[0].keyCatchers & 0x10) == 0
            && !com_sv_running->current.enabled)
        {
            SND_StopSounds(SND_STOP_ALL);
            UI_SetActiveMenu(0, UIMENU_MAIN);
        }
        CL_DevGuiFrame(0);
        //Profile_Begin(366);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
                230,
                0,
                "clientIndex doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        Live_Frame(cl_controller_in_use, v14);
        //Profile_EndInternal(0);
        CheckForConsoleGuidePause(localClientNum);
        v22 = cl_avidemo;
        if (cl_avidemo->current.integer && v14)
        {
            if (clientUIActives[0].connectionState == CA_ACTIVE || cl_forceavidemo->current.enabled)
            {
                Cmd_ExecuteSingleCommand(0, cl_controller_in_use, "screenshot silent\n");
                v22 = cl_avidemo;
            }
            LODWORD(v23) = v22->current.integer;
            HIDWORD(v23) = &a14;
            a14 = (int)(float)((float)((float)1000.0 / (float)v23) * com_timescaleValue);
            v14 = a14;
            if (!a14)
                v14 = 1;
        }
        cls.realFrametime = v14;
        cls.realtime += cls.frametime;
        if ((clientUIActives[0].keyCatchers & 0x10) != 0)
        {
            v24 = CL_ControllerIndexFromClientNum(localClientNum);
            CL_GamepadRepeatScrollingButtons(localClientNum, v24);
        }
        CL_SetCGameTime(localClientNum);
    }
}

bool __fastcall CL_IsLocalClientInGame(int localClientNum)
{
    return clientUIActives[0].connectionState == CA_ACTIVE;
}

bool __fastcall CL_IsUIActive(const int localClientNum)
{
    return (clientUIActives[0].keyCatchers & 0x10) != 0;
}

void __fastcall CL_InitRenderer()
{
    if (cls.rendererStarted)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1199, 0, "%s", "!cls.rendererStarted");
    cls.rendererStarted = 1;
    R_BeginRegistration(&cls.vidConfig);
    ScrPlace_SetupUnsafeViewport(&scrPlaceFullUnsafe, 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
    ScrPlace_SetupViewport(&scrPlaceFull, 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
    ScrPlace_SetupViewport(scrPlaceView, 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
    cls.whiteMaterial = Material_RegisterHandle("white", 3);
    cls.consoleMaterial = Material_RegisterHandle("console", 3);
    cls.consoleFont = R_RegisterFont("fonts/consoleFont", 3);
    g_console_field_width = cls.vidConfig.displayWidth - 40;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.widthInPixels = cls.vidConfig.displayWidth - 40;
    g_consoleField.fixedSize = 1;
    StatMon_Reset();
    Con_InitClientAssets();
}

void __fastcall CL_ShutdownRenderer(int destroyWindow)
{
    if (!cls.rendererStarted && !destroyWindow)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            1233,
            0,
            "%s",
            "cls.rendererStarted || destroyWindow");
    cls.rendererStarted = 0;
    R_Shutdown(destroyWindow);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    Con_ShutdownClientAssets();
}

void CL_DevGuiDvar_f()
{
    int nesting; // r7
    const char *v1; // r3
    const char *v2; // r3
    const dvar_s *Var; // r31
    const char *v4; // r3
    const char *v5; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] == 3)
    {
        v2 = Cmd_Argv(2);
        Var = Dvar_FindVar(v2);
        if (Var)
        {
            v5 = Cmd_Argv(1);
            DevGui_AddDvar(v5, Var);
        }
        else
        {
            v4 = Cmd_Argv(2);
            Com_Printf(11, "dvar '%s' doesn't exist\n", v4);
        }
    }
    else
    {
        v1 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\" dvarName\n", v1);
    }
}

void CL_DevGuiCmd_f()
{
    int nesting; // r7
    const char *v1; // r3
    const char *v2; // r31
    const char *v3; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] == 3)
    {
        v2 = Cmd_Argv(2);
        v3 = Cmd_Argv(1);
        DevGui_AddCommand(v3, v2);
    }
    else
    {
        v1 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\" \"command text\"\n", v1);
    }
}

void CL_DevGuiOpen_f()
{
    int nesting; // r7
    const char *v1; // r3
    const char *v2; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] == 2)
    {
        v2 = Cmd_Argv(1);
        DevGui_OpenMenu(v2);
    }
    else
    {
        v1 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\"\n", v1);
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

void __fastcall CL_StartHunkUsers()
{
    if (clientUIActives[0].isRunning)
    {
        if (!cls.rendererStarted)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1339, 0, "%s", "cls.rendererStarted");
        if (!cls.soundStarted)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1341, 0, "%s", "cls.soundStarted");
        if (!cls.uiStarted)
        {
            CL_InitUI();
            Sys_LoadingKeepAlive();
        }
        if (!cls.devGuiStarted)
        {
            cls.devGuiStarted = 1;
            CL_InitDevGui();
            Sys_LoadingKeepAlive();
        }
    }
}

int __fastcall CL_ScaledMilliseconds()
{
    return cls.realtime;
}

void __fastcall CL_InitRef()
{
    GfxConfiguration v0; // [sp+50h] [-20h] BYREF

    Com_Printf(14, "----- Initializing Renderer ----\n");
    v0.maxClientViews = 1;
    v0.entCount = 2208;
    v0.entnumNone = 2175;
    v0.entnumOrdinaryEnd = 2174;
    v0.threadContextCount = 12;
    v0.critSectCount = 32;
    R_ConfigureRenderer(&v0);
    Dvar_SetInt(cl_paused, 0);
}

void __fastcall CL_VoidCommand()
{
    ;
}

void __fastcall CL_startMultiplayer_f()
{
    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1453, 0, "%s", "Sys_IsMainThread()");
    LSP_SendLogRequest(cl_controller_in_use);
    LB_EndOngoingTasks();
    Com_SyncThreads();
    Sys_SuspendOtherThreads();
    R_ShutdownDirect3D();
    XLaunchNewImage("cod3mp.exe", 0);
    JUMPOUT(0x82179E04);
}

void __fastcall CL_ShellExecute_URL_f()
{
    int nesting; // r7
    const char *v1; // r3
    int v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Com_DPrintf(0, "CL_ShellExecute_URL_f\n");
    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v1 = byte_82003CDD;
    else
        v1 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    if (I_stricmp(v1, "open"))
    {
        Com_DPrintf(0, "invalid CL_ShellExecute_URL_f syntax (shellExecute \"open\" <url> <doExit>)\n");
    }
    else
    {
        if (Cmd_Argc() >= 4)
        {
            v3 = Cmd_Argv(3);
            v2 = atol(v3);
        }
        else
        {
            v2 = 1;
        }
        v4 = Cmd_Argv(2);
        Sys_OpenURL(v4, v2);
    }
}

void __fastcall CL_IncAnimWeight_f(int a1, int a2, int a3, int a4, int a5, int a6, __int64 a7)
{
    const dvar_s *v7; // r3
    double v8; // fp0
    double value; // fp31
    double v10; // fp31

    v7 = cl_testAnimWeight;
    v8 = com_timescaleValue;
    value = cl_testAnimWeight->current.value;
    if (com_timescaleValue == 0.0)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1575, 0, "%s", "com_timescaleValue");
        v7 = cl_testAnimWeight;
        v8 = com_timescaleValue;
    }
    LODWORD(a7) = cls.frametime;
    v10 = (float)((float)((float)((float)a7 / (float)v8) * (float)0.0020000001) + (float)value);
    if (v10 > 1.0)
        v10 = 1.0;
    Dvar_SetFloat(v7, v10);
    Com_Printf(0, (const char *)HIDWORD(v10), LODWORD(v10));
}

void __fastcall CL_DecAnimWeight_f(int a1, int a2, int a3, int a4, int a5, int a6, __int64 a7)
{
    const dvar_s *v7; // r3
    double v8; // fp0
    double value; // fp31
    double v10; // fp31

    v7 = cl_testAnimWeight;
    v8 = com_timescaleValue;
    value = cl_testAnimWeight->current.value;
    if (com_timescaleValue == 0.0)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1589, 0, "%s", "com_timescaleValue");
        v7 = cl_testAnimWeight;
        v8 = com_timescaleValue;
    }
    LODWORD(a7) = cls.frametime;
    v10 = (float)-(float)((float)((float)((float)a7 / (float)v8) * (float)0.0020000001) - (float)value);
    if (v10 < 0.0)
        v10 = 0.0;
    Dvar_SetFloat(v7, v10);
    Com_Printf(0, (const char *)HIDWORD(v10), LODWORD(v10));
}

void __fastcall CL_StopLogo(int localClientNum)
{
    const char *string; // r4
    const char *v3; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_LOGO)
    {
        CL_SetLocalClientConnectionState(localClientNum, CA_DISCONNECTED);
        CL_ResetLastGamePadEventTime();
        string = nextmap->current.string;
        if (*string)
        {
            v3 = va("%s\n", string);
            Cbuf_AddText(0, v3);
            Dvar_SetString(nextmap, byte_82003CDD);
        }
    }
}

void __fastcall CL_PlayLogo_f()
{
    int nesting; // r7
    int v1; // r3
    const char *v2; // r30
    const char *v3; // r3
    long double v4; // fp2
    long double v5; // fp2
    const char *v6; // r3
    long double v7; // fp2
    long double v8; // fp2
    const char *v9; // r3
    long double v10; // fp2
    long double v11; // fp2
    const char *v12; // r3
    const char *v13; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] != 5)
    {
        Com_Printf(0, "USAGE: logo <image name> <fadein seconds> <full duration seconds> <fadeout seconds>\n");
        return;
    }
    Com_DPrintf(0, "CL_PlayLogo_f\n");
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            191,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
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
    if (cls.uiStarted)
        UI_SetActiveMenu(0, UIMENU_NONE);
    SND_StopSounds(SND_STOP_ALL);
    SND_FadeAllSounds(1.0, v1);
    v2 = Cmd_Argv(1);
    v3 = Cmd_Argv(2);
    v4 = atof(v3);
    *(double *)&v4 = (float)((float)((float)*(double *)&v4 * (float)1000.0) + (float)0.5);
    v5 = floor(v4);
    cls.logo.fadein = (int)(float)*(double *)&v5;
    v6 = Cmd_Argv(3);
    v7 = atof(v6);
    *(double *)&v7 = (float)((float)((float)*(double *)&v7 * (float)1000.0) + (float)0.5);
    v8 = floor(v7);
    cls.logo.duration = (int)(float)*(double *)&v8;
    v9 = Cmd_Argv(4);
    v10 = atof(v9);
    *(double *)&v10 = (float)((float)((float)*(double *)&v10 * (float)1000.0) + (float)0.5);
    v11 = floor(v10);
    cls.logo.fadeout = (int)(float)*(double *)&v11;
    cls.logo.duration += cls.logo.fadeout + cls.logo.fadein;
    v12 = va("%s1", v2);
    cls.logo.material[0] = Material_RegisterHandle(v12, 3);
    v13 = va("%s2", v2);
    cls.logo.material[1] = Material_RegisterHandle(v13, 3);
    cls.logo.startTime = cls.realtime + 100;
}

void __fastcall CL_StopLogoOrCinematic(int localClientNum)
{
    connstate_t connectionState; // r30

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connectionState = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState == CA_CINEMATIC)
    {
        SCR_StopCinematic(localClientNum);
    }
    else
    {
        if (clientUIActives[0].connectionState != CA_LOGO)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
                1718,
                0,
                "%s",
                "clcState == CA_CINEMATIC || clcState == CA_LOGO");
        CL_StopLogo(localClientNum);
    }
    SND_StopSounds(SND_STOP_ALL);
    UI_SetActiveMenu(localClientNum, (uiMenuCommand_t)(connectionState == CA_DISCONNECTED));
}

// attributes: thunk
void __fastcall CL_InitOnceForAllClients()
{
    Ragdoll_Register();
}

void __fastcall CL_StopControllerRumbles()
{
    CG_StopAllRumbles(0);
}

void CL_Pause_f()
{
    const dvar_s *v0; // r3
    uiMenuCommand_t ActiveMenu; // r3
    char v2; // r11
    uiMenuCommand_t v3; // r4

    v0 = cl_paused;
    if (!cl_paused->current.integer)
    {
        if ((R_Cinematic_IsStarted() || R_Cinematic_IsPending()) && cg_cinematicFullscreen->current.enabled)
            return;
        v0 = cl_paused;
    }
    Dvar_SetInt(v0, v0->current.integer == 0);
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            191,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    if ((clientUIActives[0].keyCatchers & 0x10) == 0
        || (ActiveMenu = UI_GetActiveMenu(0), v2 = 1, ActiveMenu != UIMENU_INGAME))
    {
        v2 = 0;
    }
    if (v2)
    {
        if (cl_paused->current.integer)
            return;
        v3 = UIMENU_NONE;
        goto LABEL_18;
    }
    if (!cl_paused_simple->current.enabled && !Key_IsDown(0, 16) && cl_paused->current.integer)
    {
        v3 = UIMENU_INGAME;
    LABEL_18:
        UI_SetActiveMenu(0, v3);
    }
}

void __fastcall CL_Shutdown(int localClientNum)
{
    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1982, 0, "%s", "Sys_IsMainThread()");
    Com_SyncThreads();
    Com_Printf(14, "----- CL_Shutdown -----\n");
    if (recursive)
    {
        printf("recursive shutdown\n");
    }
    else
    {
        recursive = 1;
        Live_Shutdown();
        CL_ShutdownDebugData();
        if (clientUIActives[0].isRunning)
        {
            SCR_StopCinematic(0);
            clientUIActives[0].connectionState = CA_DISCONNECTED;
            SND_DisconnectListener(0);
            CL_ResetLastGamePadEventTime();
            Live_Disconnected(cl_controller_in_use);
        }
        CL_ShutdownHunkUsers();
        SND_Shutdown();
        CL_ShutdownInput();
        Cmd_RemoveCommand("cmd");
        Cmd_RemoveCommand("disconnect");
        Cmd_RemoveCommand("record");
        Cmd_RemoveCommand("demo");
        Cmd_RemoveCommand("cinematic");
        Cmd_RemoveCommand("logo");
        Cmd_RemoveCommand("stoprecord");
        Cmd_RemoveCommand("setenv");
        Cmd_RemoveCommand("fs_openedList");
        Cmd_RemoveCommand("fs_referencedList");
        Cmd_RemoveCommand("updatehunkusage");
        Cmd_RemoveCommand("sl");
        Cmd_RemoveCommand("startMultiplayer");
        Cmd_RemoveCommand("shellExecute");
        Cmd_RemoveCommand("+incAnimWeight");
        Cmd_RemoveCommand("+decAnimWeight");
        Cmd_RemoveCommand("cubemapShot");
        clientUIActives[0].isRunning = 0;
        recursive = 0;
        memset(&cls, 0, sizeof(cls));
        Com_Printf(14, "-----------------------\n");
    }
}

void __fastcall CL_DrawTextPhysical(
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    double xScale,
    double yScale,
    const float *color,
    int style)
{
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style);
}

void __fastcall CL_DrawTextPhysicalWithEffects(
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    double xScale,
    double yScale,
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
    int v16; // [sp+8h] [-98h]
    int v17; // [sp+Ch] [-94h]
    int v18; // [sp+10h] [-90h]
    int v19; // [sp+14h] [-8Ch]

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
        v16,
        v17,
        v18,
        v19);
}

void __fastcall CL_DrawText(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    int horzAlign,
    int vertAlign,
    double xScale,
    double yScale,
    const float *color,
    int style,
    double a13,
    double a14,
    double a15,
    double a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30,
    float a31,
    float a32,
    float a33,
    float a34,
    float a35,
    float a36,
    float a37,
    float a38)
{
    int v41; // r7
    const float *v42; // r6

    a28 = x;
    a30 = y;
    a36 = xScale;
    a38 = yScale;
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, &a36, &a38, (int)color, style);
    R_AddCmdDrawText(text, maxChars, font, a28, a30, a36, a38, 0.0, v42, v41);
}

void __fastcall CL_DrawTextRotate(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    double rotation,
    int horzAlign,
    int vertAlign,
    double xScale,
    double yScale,
    const float *color,
    int style,
    double a14,
    double a15,
    double a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30,
    float a31,
    float a32,
    float a33,
    float a34,
    float a35,
    int a36,
    float a37,
    float a38,
    float a39,
    float a40)
{
    int v44; // r7
    const float *v45; // r6

    a28 = x;
    a30 = y;
    a38 = xScale;
    a40 = yScale;
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, &a38, &a40, style, a36);
    R_AddCmdDrawText(text, maxChars, font, a28, a30, a38, a40, rotation, v45, v44);
}

void __fastcall CL_DrawTextPhysicalWithCursor(
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    double xScale,
    double yScale,
    const float *color,
    int style,
    int cursorPos,
    char cursor)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

void __fastcall CL_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    int horzAlign,
    int vertAlign,
    double xScale,
    double yScale,
    const float *color,
    int style,
    double a13,
    double a14,
    double a15,
    double a16,
    int cursorPos,
    char cursor,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    float a28,
    int a29,
    float a30,
    int a31,
    int a32,
    int a33,
    int a34,
    int a35,
    float a36,
    int a37,
    float a38)
{
    char v44; // r9
    int v45; // r8
    int v46; // r7
    const float *v47; // r6

    a28 = x;
    a30 = y;
    a36 = xScale;
    a38 = yScale;
    if (maxChars <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 2102, 0, "%s", "maxChars > 0");
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, &a36, &a38, (int)color, style);
    R_AddCmdDrawTextWithCursor(text, maxChars, font, a28, a30, a36, a38, 0.0, v47, v46, v45, v44);
}

// attributes: thunk
Font_s *__fastcall CL_RegisterFont(const char *fontName, int imageTrack)
{
    return R_RegisterFont(fontName, imageTrack);
}

void __fastcall CL_SetSkipRendering(bool skip)
{
    cl_skipRendering = skip;
}

bool __fastcall CL_SkipRendering()
{
    return cl_skipRendering;
}

void __fastcall CL_UpdateSound()
{
    PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "update sound");
    SND_PlayFXSounds();
    SND_UpdateLoopingSounds();
    SND_Update();
    PIXEndNamedEvent();
}

void __fastcall CL_ShutdownAll()
{
    R_SyncRenderThread();
    CL_ShutdownHunkUsers();
    if (!cls.rendererStarted)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            1233,
            0,
            "%s",
            "cls.rendererStarted || destroyWindow");
    cls.rendererStarted = 0;
    R_Shutdown(0);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    Con_ShutdownClientAssets();
    if (cls.rendererStarted)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 394, 0, "%s", "!cls.rendererStarted");
    track_shutdown(3);
}

void __fastcall CL_DisconnectLocalClient()
{
    connstate_t connectionState; // r30
    int v1; // r31

    SND_ResetPauseSettingsToDefaults();
    connectionState = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState == CA_CINEMATIC)
    {
        SCR_StopCinematic(0);
    }
    else if (clientUIActives[0].connectionState == CA_LOGO)
    {
        CL_StopLogo(0);
    }
    Dvar_SetIntByName("g_reloading", 0);
    v1 = cl_controller_in_use;
    Gamer//Profile_UpdateProfileFromDvars(cl_controller_in_use, PROFILE_WRITE_IF_CHANGED);
        Live_Disconnected(v1);
    if ((unsigned int)connectionState > CA_LOGO)
        Com_Error(ERR_DISCONNECT, "EXE_DISCONNECTED");
}

// attributes: thunk
void __fastcall CL_Disconnect_f()
{
    CL_DisconnectLocalClient();
}

void __fastcall CL_ShutdownRef()
{
    R_SyncRenderThread();
    cls.rendererStarted = 0;
    R_Shutdown(1);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    Con_ShutdownClientAssets();
    track_shutdown(3);
    StatMon_Reset();
}

// local variable allocation has failed, the output may be wrong!
void __fastcall CL_DrawLogo(const float *a1, Material *a2)
{
    unsigned int v2; // r9 OVERLAPPED
    int v3; // r30
    __int64 v4; // r11 OVERLAPPED
    double v5; // fp0
    __int64 v6; // r11 OVERLAPPED
    int v7; // r9
    __int128 v8; // r10
    double v9; // fp28
    double v10; // fp29
    double v11; // fp27
    Material *v12; // r4
    const float *v13; // r3
    float v14[14]; // [sp+70h] [-50h] BYREF

    if (clientUIActives[0].connectionState != CA_LOGO)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            1613,
            0,
            "%s",
            "CL_GetLocalClientConnectionState( ONLY_LOCAL_CLIENT_NUM ) == CA_LOGO");
    v2 = 0x82000000;
    v3 = cls.realtime - cls.logo.startTime;
    LODWORD(v4) = cls.logo.fadein;
    if (cls.realtime - cls.logo.startTime >= cls.logo.fadein)
    {
        LODWORD(v6) = cls.logo.fadeout;
        if (v3 <= cls.logo.duration - cls.logo.fadeout)
        {
        LABEL_10:
            v5 = 1.0;
            goto LABEL_11;
        }
        HIDWORD(v6) = cls.logo.duration - v3;
        v7 = cls.logo.duration - cls.logo.fadeout;
        v5 = (float)((float)*(__int64 *)((char *)&v6 + 4) / (float)v6);
    }
    else
    {
        HIDWORD(v4) = cls.realtime - cls.logo.startTime;
        v5 = (float)((float)*(__int64 *)((char *)&v4 + 4) / (float)v4);
    }
    if (v5 < 0.0)
    {
        v5 = 0.0;
        goto LABEL_11;
    }
    if (v5 > 1.0)
        goto LABEL_10;
LABEL_11:
    *((_QWORD *)&v8 + 1) = *(_QWORD *)&cls.vidConfig.displayWidth;
    v14[0] = v5;
    v14[1] = v5;
    v14[2] = v5;
    DWORD1(v8) = v14;
    v14[3] = 1.0;
    v9 = (float)*(__int64 *)((char *)&v8 + 4);
    v10 = (float)((float)((float)*(__int64 *)&cls.vidConfig.displayWidth * (float)2.0) * (float)0.33333334);
    v11 = (float)((float)(__int64)v8 - (float)((float)((float)(__int64)v8 * (float)2.0) * (float)0.33333334));
    R_AddCmdDrawStretchPic(0.0, 0.0, v9, v10, 0.0, 0.0, 1.0, 1.0, a1, a2);
    R_AddCmdDrawStretchPic(0.0, v10, v9, v11, 0.0, 0.0, 1.0, 1.0, v13, v12);
    if (v3 > cls.logo.duration)
        CL_StopLogo(0);
}

void __fastcall CL_Init(int localClientNum)
{
    unsigned int v2; // r3
    const char *v3; // r5
    unsigned __int16 v4; // r4
    const char *v5; // r5
    unsigned __int16 v6; // r4
    const char *v7; // r5
    unsigned __int16 v8; // r4
    const char *v9; // r5
    unsigned __int16 v10; // r4
    const char *v11; // r5
    unsigned __int16 v12; // r4
    const char *v13; // r5
    unsigned __int16 v14; // r4
    const char *v15; // r5
    unsigned __int16 v16; // r4
    const char *v17; // r5
    unsigned __int16 v18; // r4
    const char *v19; // r5
    unsigned __int16 v20; // r4
    int v21; // r28
    const dvar_s **v22; // r29
    const char *v23; // r5
    unsigned __int16 v24; // r4
    const char *v25; // r3
    const char *v26; // r5
    unsigned __int16 v27; // r4
    GfxConfiguration v28; // [sp+50h] [-B0h] BYREF
    char v29[80]; // [sp+70h] [-90h] BYREF

    Com_Printf(14, "----- Client Initialization -----\n");
    v2 = Sys_MillisecondsRaw();
    srand(v2);
    Con_Init();
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            576,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActives[0].connectionState = CA_DISCONNECTED;
    CL_ResetLastGamePadEventTime();
    cls.realtime = 0;
    CL_InitInput();
    cl_noprint = Dvar_RegisterBool("cl_noprint", 0, 0, "Print nothing to the console");
    cl_shownet = Dvar_RegisterInt("cl_shownet", 0, -2, 4, 0, "Display network debugging information");
    cl_avidemo = Dvar_RegisterInt("cl_avidemo", 0, 0, 0x7FFFFFFF, 0, "AVI demo frames per second");
    cl_forceavidemo = Dvar_RegisterBool("cl_forceavidemo", 0, 0, "Record AVI demo even if client is not active");
    cl_yawspeed = Dvar_RegisterFloat("cl_yawspeed", 140.0, -3.4028235e38, 3.4028235e38, v4, v3);
    cl_pitchspeed = Dvar_RegisterFloat("cl_pitchspeed", 140.0, -3.4028235e38, 3.4028235e38, v6, v5);
    cl_anglespeedkey = Dvar_RegisterFloat("cl_anglespeedkey", 1.5, 0.0, 3.4028235e38, v8, v7);
    cl_sensitivity = Dvar_RegisterFloat("sensitivity", 5.0, 0.0099999998, 100.0, v10, v9);
    cl_mouseAccel = Dvar_RegisterFloat("cl_mouseAccel", 0.0, 0.0, 100.0, v12, v11);
    cl_freelook = Dvar_RegisterBool("cl_freelook", 1, 1u, "Enable looking with mouse");
    cl_showMouseRate = Dvar_RegisterBool(
        "cl_showmouserate",
        0,
        0,
        "Print mouse rate debugging information to the console");
    cl_inGameVideo = Dvar_RegisterBool("r_inGameVideo", 1, 1u, "Allow in game cinematics");
    m_pitch = Dvar_RegisterFloat("m_pitch", 0.022, -1.0, 1.0, v14, v13);
    m_yaw = Dvar_RegisterFloat("m_yaw", 0.022, -1.0, 1.0, v16, v15);
    m_forward = Dvar_RegisterFloat("m_forward", 0.25, -1.0, 1.0, v18, v17);
    m_side = Dvar_RegisterFloat("m_side", 0.25, -1.0, 1.0, v20, v19);
    m_filter = Dvar_RegisterBool("m_filter", 0, 1u, "Allow mouse movement smoothing");
    cg_drawCrosshair = Dvar_RegisterBool("cg_drawCrosshair", 1, 1u, "Turn on weapon crosshair");
    cg_subtitles = Dvar_RegisterBool("cg_subtitles", 1, 1u, "Turn on subtitles");
    takeCoverWarnings = Dvar_RegisterInt(
        "takeCoverWarnings",
        -1,
        -1,
        50,
        0x4001u,
        "Number of times remaining to show the take cover warning (negative value indicates it has yet to"
        " be initialized)");
    cheat_points = Dvar_RegisterInt(
        "cheat_points",
        0,
        0,
        0x7FFFFFFF,
        0x4001u,
        "Used by script for keeping track of cheats");
    cheat_items_set1 = Dvar_RegisterInt(
        "cheat_items_set1",
        0,
        0,
        0x7FFFFFFF,
        0x4001u,
        "Used by script for keeping track of cheats");
    v21 = 0;
    v22 = arcadeScore;
    cheat_items_set2 = Dvar_RegisterInt(
        "cheat_items_set2",
        0,
        0,
        0x7FFFFFFF,
        0x4001u,
        "Used by script for keeping track of cheats");
    do
    {
        Com_sprintf(v29, 32, "s%d", v21);
        *v22++ = Dvar_RegisterInt(v29, 0, 0, 0x7FFFFFFF, 0x4001u, "Used by script for keeping track of arcade scores");
        ++v21;
    } while ((int)v22 < (int)&cl_freemove);
    input_invertPitch = Dvar_RegisterBool("input_invertPitch", 0, 0x400u, "Invert gamepad pitch");
    input_viewSensitivity = Dvar_RegisterFloat("input_viewSensitivity", 1.0, 0.000099999997, 5.0, v24, v23);
    input_autoAim = Dvar_RegisterBool("input_autoAim", 1, 0x400u, "Turn on auto aim for consoles");
    v25 = SEH_SafeTranslateString("PLATFORM_NOMOTD");
    motd = Dvar_RegisterString("motd", v25, 0, "Message of the day");
    nextmap = Dvar_RegisterString("nextmap", byte_82003CDD, 0, "The next map name");
    nextdemo = Dvar_RegisterString("nextdemo", byte_82003CDD, 0, "The next demo to play");
    Dvar_RegisterBool("cg_blood", 1, 1u, "Show blood");
    Campaign_RegisterDvars();
    if (!loc_language)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1881, 0, "%s", "loc_language");
    if (!loc_translate)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1882, 0, "%s", "loc_translate");
    if (!loc_warnings)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1883, 0, "%s", "loc_warnings");
    if (!loc_warningsAsErrors)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1884, 0, "%s", "loc_warningsAsErrors");
    Cmd_AddCommandInternal("cmd", CL_ForwardToServer_f, &CL_ForwardToServer_f_VAR);
    Cmd_AddCommandInternal("disconnect", Cbuf_AddServerText_f, &CL_Disconnect_f_VAR);
    Cmd_AddServerCommandInternal("disconnect", CL_Disconnect_f, &CL_Disconnect_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo", Cbuf_AddServerText_f, &CL_PlayDemo_f_VAR_0);
    Cmd_AddServerCommandInternal("demo", CL_PlayDemo_f, &CL_PlayDemo_f_VAR_SERVER_0);
    Cmd_AddCommandInternal("timedemo", Cbuf_AddServerText_f, &CL_PlayDemo_f_VAR);
    Cmd_AddServerCommandInternal("timedemo", CL_PlayDemo_f, &CL_PlayDemo_f_VAR_SERVER);
    Cmd_SetAutoComplete("demo", "demos", "spd");
    Cmd_SetAutoComplete("timedemo", "demos", "spd");
    Cmd_AddCommandInternal("record", CL_Record_f, &CL_Record_f_VAR);
    Cmd_AddCommandInternal("stoprecord", CL_StopRecord_f, &CL_StopRecord_f_VAR);
    Cmd_AddCommandInternal("logo", CL_PlayLogo_f, &CL_PlayLogo_f_VAR);
    Cmd_AddCommandInternal("cinematic", CL_PlayCinematic_f, &CL_PlayCinematic_f_VAR);
    Cmd_AddCommandInternal("unskippablecinematic", CL_PlayUnskippableCinematic_f, &CL_PlayUnskippableCinematic_f_VAR);
    Cmd_SetAutoComplete("cinematic", "video", "wmv");
    Cmd_AddCommandInternal("pause", CL_Pause_f, &CL_Pause_f_VAR);
    Cmd_AddCommandInternal("sl", CL_VoidCommand, &CL_VoidCommand_VAR);
    Cmd_AddCommandInternal("startMultiplayer", CL_startMultiplayer_f, &CL_startMultiplayer_f_VAR);
    Cmd_AddCommandInternal("shellExecute", CL_ShellExecute_URL_f, &CL_ShellExecute_URL_f_VAR);
    Cmd_AddCommandInternal("+incAnimWeight", (void(__fastcall *)())CL_IncAnimWeight_f, &CL_IncAnimWeight_f_VAR);
    Cmd_AddCommandInternal("+decAnimWeight", (void(__fastcall *)())CL_DecAnimWeight_f, &CL_DecAnimWeight_f_VAR);
    cl_testAnimWeight = Dvar_RegisterFloat("cl_testAnimWeight", 0.0, 0.0, 1.0, v27, v26);
    Cmd_AddCommandInternal("modelDumpInfo", XModelDumpInfo, &XModelDumpInfo_VAR);
    CL_Xenon_RegisterDvars();
    CL_Xenon_RegisterCommands();
    Cmd_AddCommandInternal("stopControllerRumble", CL_StopControllerRumbles, &CL_StopControllerRumbles_VAR);
    Com_Printf(14, "----- Initializing Renderer ----\n");
    v28.maxClientViews = 1;
    v28.entCount = 2208;
    v28.entnumNone = 2175;
    v28.entnumOrdinaryEnd = 2174;
    v28.threadContextCount = 12;
    v28.critSectCount = 32;
    R_ConfigureRenderer(&v28);
    Dvar_SetInt(cl_paused, 0);
    SCR_Init();
    Cbuf_Execute(0, cl_controller_in_use);
    clientUIActives[0].isRunning = 1;
    clients[0].usingAds = 0;
    Com_Printf(14, "----- Client Initialization Complete -----\n");
}

