#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"
#include <game/g_local.h>
#include "cl_ui.h"
#include <qcommon/cmd.h>
#include <devgui/devgui.h>
#include <win32/win_local.h>
#include <server/sv_public.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <database/database.h>
#include <ui/ui.h>
#include <qcommon/threads.h>
#include <cgame/cg_snapshot.h>
#include "cl_parse.h"
#include "cl_demo.h"
#include <gfx_d3d/r_init.h>
#include <gfx_d3d/r_cinematic.h>
#include <stringed/stringed_hooks.h>
#include "cl_input.h"
#include <cgame/cg_main.h>
#include <ragdoll/ragdoll.h>
#include "cl_scrn.h"
#include <qcommon/com_bsp.h>

enum MovieToPlayScriptOp : __int32
{
    MTPSOP_PLUS = 0x0,
    MTPSOP_MINUS = 0x1,
    MTPSOP_MUL = 0x2,
    MTPSOP_GT = 0x3,
    MTPSOP_LT = 0x4,
    MTPSOP_EQ = 0x5,
    MTPSOP_STRCMP = 0x6,
    MTPSOP_STRCAT = 0x7,
    MTPSOP_NOT = 0x8,
    MTPSOP_DUP = 0x9,
    MTPSOP_DROP = 0xA,
    MTPSOP_SWAP = 0xB,
    MTPSOP_GETDVAR = 0xC,
    MTPSOP_GETMAPNAME = 0xD,
    MTPSOP_IF = 0xE,
    MTPSOP_THEN = 0xF,
    MTPSOP_PLAY = 0x10,
    MTPSOP_LITERAL = 0x11,
    MTPSOP_COUNT = 0x12,
};

struct MovieToPlayScriptOpInfo
{
    MovieToPlayScriptOp op;
    const char *opName;
    unsigned int inValues;
    unsigned int outValues;
};

const MovieToPlayScriptOpInfo s_movieToPlayScriptOpInfo[18] =
{
  { MTPSOP_PLUS, "+", 2u, 1u },
  { MTPSOP_MINUS, "-", 2u, 1u },
  { MTPSOP_MUL, "*", 2u, 1u },
  { MTPSOP_GT, ">", 2u, 1u },
  { MTPSOP_LT, "<", 2u, 1u },
  { MTPSOP_EQ, "==", 2u, 1u },
  { MTPSOP_STRCMP, "strcmp", 2u, 1u },
  { MTPSOP_STRCAT, "strcat", 2u, 1u },
  { MTPSOP_NOT, "!", 1u, 1u },
  { MTPSOP_DUP, "dup", 1u, 2u },
  { MTPSOP_DROP, "drop", 1u, 0u },
  { MTPSOP_SWAP, "swap", 2u, 2u },
  { MTPSOP_GETDVAR, "getdvar", 1u, 1u },
  { MTPSOP_GETMAPNAME, "getmapname", 0u, 1u },
  { MTPSOP_IF, "if", 1u, 0u },
  { MTPSOP_THEN, "then", 0u, 0u },
  { MTPSOP_PLAY, "play", 1u, 0u },
  { MTPSOP_LITERAL, NULL, 0u, 1u }
};


clientConnection_t clientConnections[1];
clientUIActive_t clientUIActives[1];
clientActive_t clients[1];
clientStatic_t cls;

const dvar_t *input_invertPitch;
const dvar_t *cl_avidemo;
const dvar_t *cl_testAnimWeight;
const dvar_t *cl_freemoveScale;
#ifdef KISAK_MP
const dvar_t *motd;
#endif
const dvar_t *cl_sensitivity;
const dvar_t *cl_forceavidemo;
const dvar_t *m_yaw;
const dvar_t *m_pitch;
const dvar_t *nextdemo;
const dvar_t *cl_freemove;
const dvar_t *cl_showMouseRate;
const dvar_t *takeCoverWarnings;
const dvar_t *m_forward;
const dvar_t *cheat_items_set2;
const dvar_t *cl_mouseAccel;
const dvar_t *cheat_points;
const dvar_t *input_viewSensitivity;
const dvar_t *input_autoAim;
const dvar_t *cl_inGameVideo;
const dvar_t *cl_noprint;
const dvar_t *m_side;
const dvar_t *m_filter;
const dvar_t *cheat_items_set1;
const dvar_t *cl_freelook;
const dvar_t *cl_shownet;

const dvar_s *arcadeScore[19]{ 0 };

void __cdecl TRACK_cl_main()
{
    track_static_alloc_internal(clientUIActives, 20, "clientUIActives", 9);
    track_static_alloc_internal(clients, 633532, "clients", 9);
    track_static_alloc_internal(clientConnections, 271392, "clientConnections", 9);
    track_static_alloc_internal(&cls, 612, "cls", 9);
}

int __cdecl CL_GetLocalClientActiveCount()
{
    return 1;
}

int __cdecl CL_GetFirstActiveLocalClient()
{
    return 0;
}

bool __cdecl CL_IsLocalClientActive(int clientNum)
{
    return clientNum == 0;
}

void __cdecl CL_SetLocalClientActive(int clientNum, bool active)
{
    ;
}

int __cdecl CL_LocalClientNumFromControllerIndex(unsigned int controllerIndex)
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

int __cdecl CL_ControllerIndexFromClientNum(int clientIndex)
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

int __cdecl CL_GetFirstActiveControllerIndex()
{
    return cl_controller_in_use;
}

int __cdecl CL_AllLocalClientsInactive()
{
    return 0;
}

// attributes: thunk
void __cdecl CL_RunOncePerClientFrame(int localClientNum, int msec)
{
    IN_Frame();
}

void __cdecl CL_DumpReliableCommand(int cmdIndex, const char *cmd)
{
    Com_Printf(0, "cmd[%d] '%s'\n", cmdIndex, cmd);
}

void __cdecl CL_DumpReliableCommands(clientConnection_t *clc)
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

void __cdecl CL_AddReliableCommand(int localClientNum, const char *cmd)
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

void __cdecl CL_SaveSettings(MemoryFile *memFile)
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

void __cdecl CL_RestoreSettings(MemoryFile *memFile)
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

void __cdecl CL_MapLoading_CalcMovieToPlay(const char *buffer, const char *inMapName, char *outMovieName)
{
    unsigned int v3; // r30
    char *v4; // r14
    int v5; // r28
    const char *v6; // r3
    const char *v7; // r31
    const char *v8; // r11
    const char *v9; // r10
    int v10; // r8
    const char *v11; // r11
    const char *v12; // r10
    int v13; // r8
    const MovieToPlayScriptOpInfo *i; // r28
    const char *opName; // r10
    const char *v16; // r11
    int v17; // r8
    const char *v18; // r6
    const char *v19; // r6
    int v20; // r14
    int v21; // r3
    int v22; // r14
    int v23; // r3
    int v24; // r14
    int v25; // r3
    int v26; // r14
    BOOL v27; // r6
    int v28; // r14
    BOOL v29; // r6
    int v30; // r14
    int v31; // r3
    char *v32; // r10
    char *v33; // r11
    int v34; // r6
    int v35; // r3
    const char *VariantString; // r3
    int v37; // r11
    int v38; // [sp+50h] [-2F0h]
    char v39[736]; // [sp+60h] [-2E0h] BYREF
    const char *v40; // [sp+354h] [+14h] BYREF
    const char *v41; // [sp+35Ch] [+1Ch]
    char *v42; // [sp+364h] [+24h]

    v3 = 0;
    v40 = buffer;
    v41 = inMapName;
    v4 = outMovieName;
    v5 = 0;
    v42 = outMovieName;
    v38 = 0;
    *outMovieName = 0;
    Com_BeginParseSession("video/cin_levels.txt");
    do
    {
        v6 = Com_Parse(&v40)->token;
        v7 = v6;
        if (!*v6)
            break;
        if (v5)
        {
            v8 = v6;
            v9 = "then";
            do
            {
                v10 = *(unsigned __int8 *)v8 - *(unsigned __int8 *)v9;
                if (!*v8)
                    break;
                ++v8;
                ++v9;
            } while (!v10);
            if (v10)
            {
                v11 = v6;
                v12 = "if";
                do
                {
                    v13 = *(unsigned __int8 *)v11 - *(unsigned __int8 *)v12;
                    if (!*v11)
                        break;
                    ++v11;
                    ++v12;
                } while (!v13);
                if (!v13)
                    v38 = ++v5;
            }
            else
            {
                v38 = --v5;
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
                v16 = v7;
                do
                {
                    v17 = *(unsigned __int8 *)v16 - *(unsigned __int8 *)opName;
                    if (!*v16)
                        break;
                    ++v16;
                    ++opName;
                } while (!v17);
                if (!v17)
                    break;
            }
            if (v3 < i->inValues)
            {
                v18 = "do special command";
                if (!i->opName)
                    v18 = "push literal";
                Com_Error(ERR_FATAL, "Stack underflow in %s, trying to %s '%s'.", "video/cin_levels.txt", v18, v7);
            }
            if (i->outValues - i->inValues + v3 >= 8)
            {
                v19 = "do special command";
                if (!i->opName)
                    v19 = "push literal";
                Com_Error(ERR_FATAL, "Stack overflow in %s, trying to %s '%s'.", "video/cin_levels.txt", v19, v7);
            }
            switch (i->op)
            {
            case MTPSOP_PLUS:
                v20 = atol(&v39[64 * v3]);
                v21 = atol(&v39[64 * v3 - 64]);
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v20 + v21);
                break;
            case MTPSOP_MINUS:
                v22 = atol(&v39[64 * v3]);
                v23 = atol(&v39[64 * v3 - 64]);
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v23 - v22);
                break;
            case MTPSOP_MUL:
                v24 = atol(&v39[64 * v3]);
                v25 = atol(&v39[64 * v3 - 64]);
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v24 * v25);
                break;
            case MTPSOP_GT:
                v26 = atol(&v39[64 * v3]);
                v27 = atol(&v39[64 * v3 - 64]) > v26;
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v27);
                break;
            case MTPSOP_LT:
                v28 = atol(&v39[64 * v3]);
                v29 = atol(&v39[64 * v3 - 64]) < v28;
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v29);
                break;
            case MTPSOP_EQ:
                v30 = atol(&v39[64 * v3]);
                v31 = atol(&v39[64 * v3 - 64]);
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v30 == v31);
                break;
            case MTPSOP_STRCMP:
                v32 = &v39[64 * v3];
                v33 = &v39[64 * v3 - 64];
                do
                {
                    v34 = (unsigned __int8)*v33 - (unsigned __int8)*v32;
                    if (!*v33)
                        break;
                    ++v33;
                    ++v32;
                } while (!v34);
                snprintf(&v39[64 * v3 - 64], 0x40u, "%i", v34);
                break;
            case MTPSOP_STRCAT:
                snprintf(v39, 0x40u, "%s%s", &v39[64 * v3 - 64], &v39[64 * v3]);
                I_strncpyz(&v39[64 * v3 - 64], v39, 64);
                break;
            case MTPSOP_NOT:
                v35 = atol(&v39[64 * v3]);
                snprintf(&v39[64 * v3], 0x40u, "%i", v35 == 0);
                break;
            case MTPSOP_DUP:
                I_strncpyz(&v39[64 * v3 + 64], &v39[64 * v3], 64);
                break;
            case MTPSOP_DROP:
            case MTPSOP_THEN:
                break;
            case MTPSOP_SWAP:
                I_strncpyz(v39, &v39[64 * v3 - 64], 64);
                I_strncpyz(&v39[64 * v3 - 64], &v39[64 * v3], 64);
                I_strncpyz(&v39[64 * v3], v39, 64);
                break;
            case MTPSOP_GETDVAR:
                VariantString = Dvar_GetVariantString(&v39[64 * v3]);
                I_strncpyz(&v39[64 * v3], VariantString, 64);
                break;
            case MTPSOP_GETMAPNAME:
                I_strncpyz(&v39[64 * v3 + 64], v41, 64);
                break;
            case MTPSOP_IF:
                if (!atol(&v39[64 * v3]))
                    v38 = 1;
                break;
            case MTPSOP_PLAY:
                I_strncpyz(v4, &v39[64 * v3], 256);
                break;
            case MTPSOP_LITERAL:
                I_strncpyz(&v39[64 * v3 + 64], v7, 64);
                break;
            default:
                if (!alwaysfails)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 606, 0, "Can't happen.");
                break;
            }
            v4 = v42;
            v37 = i->outValues - i->inValues;
            v5 = v38;
            v3 += v37;
        }
    } while (!*v4);
    Com_EndParseSession();
    if (v5)
        Com_Error(ERR_FATAL, "Unterminated if in %s", "video/cin_levels.txt");
    if (!*v4)
        Com_Error(ERR_FATAL, "No loading movie specified by %s", "video/cin_levels.txt");
    if (!*v4)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 622, 0, "%s", "outMovieName[0]");
}

void __cdecl CL_MapLoading_CalcMovieToPlay_FastFile(const char *inMapName, char *outMovieName)
{
    XAssetHeader asset; // r30

    asset = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, "video/cin_levels.txt");

    if (!asset.rawfile)
        Com_Error(ERR_FATAL, "Could not open %s", "video/cin_levels.txt");

    CL_MapLoading_CalcMovieToPlay((const char *)asset.xmodelPieces->pieces, inMapName, outMovieName);
}

void __cdecl CL_MapLoading_StartCinematic(const char *mapname, float volume)
{
    XAssetHeader asset; // r30
    char v7[264]; // [sp+50h] [-130h] BYREF

    asset = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, "video/cin_levels.txt");

    if (!asset.rawfile)
        Com_Error(ERR_FATAL, "Could not open %s", "video/cin_levels.txt");

    CL_MapLoading_CalcMovieToPlay((const char *)asset.xmodelPieces->pieces, mapname, v7);
    R_Cinematic_StartPlayback(v7, 5, volume);
}

void __cdecl CL_MapLoading(const char *mapname)
{
    if (!clientUIActives[0].isRunning)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 677, 0, "%s", "clUI->isRunning");
    Con_Close(0);
    clientUIActives[0].keyCatchers = 0;
    clientUIActives[0].displayHUDWithKeycatchUI = 0;
    I_strncpyz(cls.servername, "localhost", 256);
    
    iassert(CL_GetLocalClientConnectionState(0) == CA_MAP_RESTART);

    clientUIActives[0].connectionState = CA_LOADING;
#ifndef KISAK_NO_FASTFILES
    CL_MapLoading_StartCinematic(mapname, (float)(snd_cinematicVolumeScale->current.value * snd_volume->current.value)),
#endif
    UI_DrawConnectScreen();
    //Live_SetCurrentMapname(mapname);
    if (cl_multi_gamepads_enabled)
        Cmd_ExecuteSingleCommand(0, cl_controller_in_use, (char*)"nosplitscreen\n");
    SND_FadeAllSounds(0.0, 0);
}

void __cdecl CL_ResetSkeletonCache()
{
    iassert(Sys_IsMainThread());
    //PIXSetMarker(0xFFFFFFFF, "CL_ResetSkeletonCache");
    if (!++clients[0].skelTimeStamp)
        clients[0].skelTimeStamp = 1;
    clients[0].skelMemoryStart = (char *)((unsigned int)&clients[0].skelMemory[15] & 0xFFFFFFF0);
    clients[0].skelMemPos = 0;
}

void __cdecl CL_ClearState()
{
    unsigned __int16 *configstrings; // r31

    CG_CreateNextSnap(0, 0.0, 0);
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
    Com_ClientDObjClearAllSkel();
    memset(clientConnections, 0, sizeof(clientConnections));
}

void __cdecl CL_Disconnect(int localClientNum)
{
    if (clientUIActives[0].isRunning)
    {
        SCR_StopCinematic(localClientNum);
        CL_SetLocalClientConnectionState(localClientNum, CA_DISCONNECTED);
        SND_DisconnectListener(localClientNum);
        //CL_ResetLastGamePadEventTime(); // KISAKTODO
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
                230,
                0,
                "clientIndex doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        //Live_Disconnected(cl_controller_in_use);
    }
}

void __cdecl CL_ForwardCommandToServer(int localClientNum, const char *string)
{
    const char *cmd; // r31

    cmd = Cmd_Argv(0);

    if (*cmd != '-')
    {
        if (cls.demoplaying || clientUIActives[0].connectionState == CA_DISCONNECTED || *cmd == '+')
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

void __cdecl CL_ForwardToServer_f()
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

void __cdecl CL_ConnectResponse()
{
    if (!clientUIActives[0].isRunning)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 998, 0, "%s", "clUI->isRunning");
    clientUIActives[0].connectionState = !clientUIActives[0].cgameInitialized ? CA_LOADING : CA_MAP_RESTART;
}

void __cdecl CL_InitLoad(const char *mapname)
{
    com_expectedHunkUsage = 0;
    Dvar_SetString(nextmap, "");
    if (clientUIActives[0].isRunning)
    {
        SCR_StopCinematic(0);
        clientUIActives[0].connectionState = CA_DISCONNECTED;
        SND_DisconnectListener(0);
        //CL_ResetLastGamePadEventTime(); // KISAKTODO
        //Live_Disconnected(cl_controller_in_use);
    }
    UI_SetMap(mapname);
    clientUIActives[0].connectionState = CA_MAP_RESTART;
}

void __cdecl CL_PacketEvent(msg_t *msg, int serverMessageSequence)
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

void __cdecl CL_SetFrametime(int frametime, int animFrametime)
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

void __cdecl CheckForConsoleGuidePause(int localClientNum)
{
#ifdef KISAK_XBOX
    if (Live_IsSystemUiActive()
        && !cl_paused->current.integer
        && com_sv_running->current.enabled
        && (!R_Cinematic_IsStarted() && !R_Cinematic_IsPending() || !cg_cinematicFullscreen->current.enabled)
        && !Key_IsCatcherActive(localClientNum, 16))
    {
        UI_SetActiveMenu(localClientNum, UIMENU_INGAME);
    }
#endif
}

void __cdecl CL_Frame(int localClientNum, int msec)
{
    int v14; // r29
    char v21; // cr34
    const dvar_s *v22; // r11
    __int64 v23; // r11
    int v24; // r3

    v14 = msec;
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    //PIXSetMarker(0xFFFFFFFF, "CL_Frame");
    if (clientUIActives[0].isRunning)
    {
        //_R11 = 0;
        //_R8 = &cls.scriptError;
        //do
        //{
        //    __asm
        //    {
        //        mfmsr     r9
        //        mtmsree   r13
        //        lwarx     r10, (_cls__3UclientStatic_t__A.scriptError - 0x8283B3A0), r8# clientStatic_t cls ...
        //        stwcx.r11, (_cls__3UclientStatic_t__A.scriptError - 0x8283B3A0), r8# clientStatic_t cls ...
        //        mtmsree   r9
        //    }
        //} while (!v21);
        //if (_R10)
        //    UI_SetActiveMenu(0, UIMENU_MAIN);

        if (cls.scriptError) {
            cls.scriptError = 0;
            UI_SetActiveMenu(0, UIMENU_MAIN);
        }

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
        //Live_Frame(cl_controller_in_use, v14);
        //Profile_EndInternal(0);
        CheckForConsoleGuidePause(localClientNum);
        v22 = cl_avidemo;
        if (cl_avidemo->current.integer && v14)
        {
            if (clientUIActives[0].connectionState == CA_ACTIVE || cl_forceavidemo->current.enabled)
            {
                Cmd_ExecuteSingleCommand(0, cl_controller_in_use, (char*)"screenshot silent\n");
                v22 = cl_avidemo;
            }
            v14 = (int)(float)((float)((float)1000.0 / (float)v22->current.integer) * com_timescaleValue);
            if (!v14)
                v14 = 1;
        }
        cls.realFrametime = v14;
        cls.realtime += cls.frametime;
        if ((clientUIActives[0].keyCatchers & 0x10) != 0)
        {
            v24 = CL_ControllerIndexFromClientNum(localClientNum);
            //CL_GamepadRepeatScrollingButtons(localClientNum, v24); // KISAKTODO
        }
        CL_SetCGameTime(localClientNum);
    }
}

bool __cdecl CL_IsLocalClientInGame(int localClientNum)
{
    return clientUIActives[0].connectionState == CA_ACTIVE;
}

bool __cdecl CL_IsUIActive(const int localClientNum)
{
    return (clientUIActives[0].keyCatchers & 0x10) != 0;
}

void __cdecl CL_InitRenderer()
{
    iassert(!cls.rendererStarted);
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
        DevGui_AddCommand(v3, (char*)v2);
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

void __cdecl CL_StartHunkUsers()
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

int __cdecl CL_ScaledMilliseconds()
{
    return cls.realtime;
}

static void SetupGfxConfig(GfxConfiguration *config)
{
    config->maxClientViews = 1;
    config->entCount = 2208;
    config->entnumNone = ENTITYNUM_NONE;
    config->entnumOrdinaryEnd = ENTITYNUM_WORLD;
    config->threadContextCount = THREAD_CONTEXT_COUNT;
    config->critSectCount = CRITSECT_COUNT;
}


static void CL_SetFastFileNames(GfxConfiguration *config, bool dedicatedServer)
{
    iassert(config);

    config->codeFastFileName = "code_post_gfx";
    config->uiFastFileName = "ui";
    config->commonFastFileName = "common";
    config->localizedCodeFastFileName = NULL;
    config->localizedCommonFastFileName = NULL;
    config->modFastFileName = DB_ModFileExists() != 0 ? "mod" : NULL;
}

void __cdecl CL_InitRef()
{
    GfxConfiguration config; // [sp+50h] [-20h] BYREF

    Com_Printf(14, "----- Initializing Renderer ----\n");
    SetupGfxConfig(&config);
    CL_SetFastFileNames(&config, 0);
    R_ConfigureRenderer(&config);
    Dvar_SetInt(cl_paused, 0);
}

void __cdecl CL_VoidCommand()
{
    ;
}

void __cdecl CL_startMultiplayer_f()
{
    iassert(0); // KISAKTODO
    if (!Sys_IsMainThread())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp", 1453, 0, "%s", "Sys_IsMainThread()");
    //LSP_SendLogRequest(cl_controller_in_use);
    //LB_EndOngoingTasks();
    Com_SyncThreads();
    Sys_SuspendOtherThreads();
    R_ShutdownDirect3D();
    //XLaunchNewImage("cod3mp.exe", 0);
    //JUMPOUT(0x82179E04);
}

void __cdecl CL_ShellExecute_URL_f()
{
    int nesting; // r7
    const char *v1; // r3
    int v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    Com_DPrintf(0, "CL_ShellExecute_URL_f\n");

    v1 = Cmd_Argv(1);

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

void __cdecl CL_IncAnimWeight_f()
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
    v10 = (float)((float)((float)((float)cls.frametime / (float)v8) * (float)0.0020000001) + (float)value);
    if (v10 > 1.0)
        v10 = 1.0;
    Dvar_SetFloat(v7, v10);
    Com_Printf(0, (const char *)HIDWORD(v10), LODWORD(v10));
}

void __cdecl CL_DecAnimWeight_f()
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
    v10 = (float)-(float)((float)((float)((float)cls.frametime / (float)v8) * (float)0.0020000001) - (float)value);
    if (v10 < 0.0)
        v10 = 0.0;
    Dvar_SetFloat(v7, v10);
    Com_Printf(0, (const char *)HIDWORD(v10), LODWORD(v10));
}

void __cdecl CL_StopLogo(int localClientNum)
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
        //CL_ResetLastGamePadEventTime(); // KISAKTODO
        string = nextmap->current.string;
        if (*string)
        {
            v3 = va("%s\n", string);
            Cbuf_AddText(0, v3);
            Dvar_SetString(nextmap, "");
        }
    }
}

void __cdecl CL_PlayLogo_f()
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

void __cdecl CL_StopLogoOrCinematic(int localClientNum)
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
void __cdecl CL_InitOnceForAllClients()
{
    Ragdoll_Register();
}

void __cdecl CL_StopControllerRumbles()
{
    //CG_StopAllRumbles(0); // KISAKTODO
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

static int recursive;
void __cdecl CL_Shutdown(int localClientNum)
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
        //Live_Shutdown();
        CL_ShutdownDebugData();
        if (clientUIActives[0].isRunning)
        {
            SCR_StopCinematic(0);
            clientUIActives[0].connectionState = CA_DISCONNECTED;
            SND_DisconnectListener(0);
            //CL_ResetLastGamePadEventTime(); // KISAKTODO
            //Live_Disconnected(cl_controller_in_use);
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
    iassert(maxChars > 0);
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

// attributes: thunk
Font_s *__cdecl CL_RegisterFont(const char *fontName, int imageTrack)
{
    return R_RegisterFont(fontName, imageTrack);
}

static bool cl_skipRendering;
void __cdecl CL_SetSkipRendering(bool skip)
{
    cl_skipRendering = skip;
}

bool __cdecl CL_SkipRendering()
{
    return cl_skipRendering;
}

void __cdecl CL_UpdateSound()
{
    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "update sound");
    SND_PlayFXSounds();
    SND_UpdateLoopingSounds();
    SND_Update();
    //PIXEndNamedEvent();
}


void __cdecl CL_ShutdownRenderer(int destroyWindow)
{
    iassert(cls.rendererStarted || destroyWindow);

    cls.rendererStarted = 0;
    // MP ADD
    Com_ShutdownWorld();
    if (IsFastFileLoad() && destroyWindow)
        CM_Shutdown();
    // MP END
    R_Shutdown(destroyWindow);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    //Con_ShutdownClientAssets(); // nullsub
}

void __cdecl CL_ShutdownAll(bool destroyWindow)
{
    R_SyncRenderThread();
    CL_ShutdownHunkUsers();

    if (cls.rendererStarted)
    {
        CL_ShutdownRenderer(destroyWindow);
        iassert(!cls.rendererStarted);
    }

    track_shutdown(3);
}

void __cdecl CL_DisconnectLocalClient()
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
    //GamerProfile_UpdateProfileFromDvars(cl_controller_in_use, PROFILE_WRITE_IF_CHANGED);
    //Live_Disconnected(v1);
    if ((unsigned int)connectionState > CA_LOGO)
        Com_Error(ERR_DISCONNECT, "EXE_DISCONNECTED");
}

// attributes: thunk
void __cdecl CL_Disconnect_f()
{
    CL_DisconnectLocalClient();
}

void __cdecl CL_ShutdownRef()
{
    R_SyncRenderThread();
    cls.rendererStarted = 0;
    R_Shutdown(1);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    //Con_ShutdownClientAssets(); // nullsub
    track_shutdown(3);
    StatMon_Reset();
}

// aislop
void __cdecl CL_DrawLogo()
{
    if (clientUIActives[0].connectionState != CA_LOGO)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_main.cpp",
            1613,
            0,
            "%s",
            "CL_GetLocalClientConnectionState(ONLY_LOCAL_CLIENT_NUM) == CA_LOGO"
        );
    }

    int elapsed = cls.realtime - cls.logo.startTime;
    float alpha = 1.0f;

    if (elapsed < cls.logo.fadein)
    {
        // Fade in phase
        alpha = (float)elapsed / (float)cls.logo.fadein;
    }
    else if (elapsed > (cls.logo.duration - cls.logo.fadeout))
    {
        // Fade out phase
        int fadeOutElapsed = elapsed - (cls.logo.duration - cls.logo.fadeout);
        alpha = 1.0f - ((float)fadeOutElapsed / (float)cls.logo.fadeout);
    }

    // Clamp alpha
    if (alpha < 0.0f)
        alpha = 0.0f;
    else if (alpha > 1.0f)
        alpha = 1.0f;

    // Set color with alpha for drawing
    float color[4] = { alpha, alpha, alpha, 1.0f };

    float screenWidth = (float)cls.vidConfig.displayWidth;
    float screenHeight = (float)cls.vidConfig.displayHeight;

    float topHeight = screenHeight * (2.0f / 3.0f);
    float bottomHeight = screenHeight - topHeight;

    // Draw top part of logo
    R_AddCmdDrawStretchPic(
        0.0f, 0.0f,
        screenWidth, topHeight,
        0.0f, 0.0f, 1.0f, 1.0f,
        color,
        cls.logo.material[0]
    );

    // Draw bottom part of logo
    R_AddCmdDrawStretchPic(
        0.0f, topHeight,
        screenWidth, bottomHeight,
        0.0f, 0.0f, 1.0f, 1.0f,
        color,
        cls.logo.material[1]
    );

    if (elapsed > cls.logo.duration)
    {
        CL_StopLogo(0);
    }
}


cmd_function_s CL_ForwardToServer_f_VAR;
cmd_function_s CL_Disconnect_f_VAR;
cmd_function_s CL_Disconnect_f_VAR_SERVER;
cmd_function_s CL_PlayDemo_f_VAR_0;
cmd_function_s CL_PlayDemo_f_VAR_SERVER_0;
cmd_function_s CL_PlayDemo_f_VAR;
cmd_function_s CL_PlayDemo_f_VAR_SERVER;
cmd_function_s CL_Record_f_VAR;
cmd_function_s CL_StopRecord_f_VAR;
cmd_function_s CL_PlayLogo_f_VAR;
cmd_function_s CL_PlayCinematic_f_VAR;
cmd_function_s CL_PlayUnskippableCinematic_f_VAR;
cmd_function_s CL_Pause_f_VAR;
cmd_function_s CL_VoidCommand_VAR;
cmd_function_s CL_startMultiplayer_f_VAR;
cmd_function_s CL_ShellExecute_URL_f_VAR;
cmd_function_s CL_IncAnimWeight_f_VAR;
cmd_function_s CL_DecAnimWeight_f_VAR;
cmd_function_s XModelDumpInfo_VAR;
cmd_function_s CL_StopControllerRumbles_VAR;

void __cdecl CL_Init(int localClientNum)
{
    int v21; // r28
    const dvar_s **v22; // r29
    const char *v23; // r5
    unsigned __int16 v24; // r4
    const char *v26; // r5
    unsigned __int16 v27; // r4
    char v29[80]; // [sp+70h] [-90h] BYREF

    Com_Printf(14, "----- Client Initialization -----\n");
    srand(Sys_MillisecondsRaw());
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
    //CL_ResetLastGamePadEventTime(); // KISAKTODO
    cls.realtime = 0;
    CL_InitInput();
    cl_noprint = Dvar_RegisterBool("cl_noprint", 0, 0, "Print nothing to the console");
    cl_shownet = Dvar_RegisterInt("cl_shownet", 0, -2, 4, 0, "Display network debugging information");
    cl_avidemo = Dvar_RegisterInt("cl_avidemo", 0, 0, 0x7FFFFFFF, 0, "AVI demo frames per second");
    cl_forceavidemo = Dvar_RegisterBool("cl_forceavidemo", 0, 0, "Record AVI demo even if client is not active");
    cl_yawspeed = Dvar_RegisterFloat("cl_yawspeed", 140.0, -FLT_MAX, FLT_MAX, 0, "Max yaw speed in degrees for game pad and keyboard");
    cl_pitchspeed = Dvar_RegisterFloat("cl_pitchspeed", 140.0, -FLT_MAX, FLT_MAX, 0, "Max pitch speed in degrees for game pad and keyboard");
    cl_anglespeedkey = Dvar_RegisterFloat("cl_anglespeedkey", 1.5, 0.0, FLT_MAX, 0, "Multiplier for max angle speed for gamepad and keyboard");
    cl_sensitivity = Dvar_RegisterFloat("sensitivity", 5.0, 0.01f, 100.0, 0, "Mouse sensitivity");
    cl_mouseAccel = Dvar_RegisterFloat("cl_mouseAccel", 0.0, 0.0, 100.0, 0, "Mouse acceleration");
    cl_freelook = Dvar_RegisterBool("cl_freelook", 1, 1u, "Enable looking with mouse");
    cl_showMouseRate = Dvar_RegisterBool(
        "cl_showmouserate",
        0,
        0,
        "Print mouse rate debugging information to the console");
    cl_inGameVideo = Dvar_RegisterBool("r_inGameVideo", 1, 1u, "Allow in game cinematics");
    m_pitch = Dvar_RegisterFloat("m_pitch", 0.022, -1.0, 1.0, 0, "Default pitch");
    m_yaw = Dvar_RegisterFloat("m_yaw", 0.022, -1.0, 1.0, 0, "Default yaw");
    m_forward = Dvar_RegisterFloat("m_forward", 0.25, -1.0, 1.0, 0, "Forward speed in units per second");
    m_side = Dvar_RegisterFloat("m_side", 0.25, -1.0, 1.0, 0, "Sideways motion in units per second");
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
    cheat_items_set2 = Dvar_RegisterInt(
        "cheat_items_set2",
        0,
        0,
        0x7FFFFFFF,
        0x4001u,
        "Used by script for keeping track of cheats");

    v21 = 0;
    v22 = arcadeScore;
    do
    {
        Com_sprintf(v29, 32, "s%d", v21);
        *v22++ = Dvar_RegisterInt(v29, 0, 0, 0x7FFFFFFF, 0x4001u, "Used by script for keeping track of arcade scores");
        ++v21;
    } while ((int)v22 < (int)&arcadeScore[19]);

    input_invertPitch = Dvar_RegisterBool("input_invertPitch", 0, 0x400u, "Invert gamepad pitch");
    input_viewSensitivity = Dvar_RegisterFloat("input_viewSensitivity", 1.0, 0.000099999997, 5.0, 0, 0);
    input_autoAim = Dvar_RegisterBool("input_autoAim", 1, 0x400u, "Turn on auto aim for consoles");
#ifdef KISAK_MP
    motd = Dvar_RegisterString("motd", SEH_SafeTranslateString((char*)"PLATFORM_NOMOTD"), 0, "Message of the day");
#endif
    nextmap = Dvar_RegisterString("nextmap", "", 0, "The next map name");
    nextdemo = Dvar_RegisterString("nextdemo", "", 0, "The next demo to play");
    Dvar_RegisterBool("cg_blood", 1, 1u, "Show blood");
    Campaign_RegisterDvars();
    iassert(loc_language);
    iassert(loc_translate);
    iassert(loc_warnings);
    iassert(loc_warningsAsErrors);
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
    Cmd_AddCommandInternal("+incAnimWeight", (void(__cdecl *)())CL_IncAnimWeight_f, &CL_IncAnimWeight_f_VAR);
    Cmd_AddCommandInternal("+decAnimWeight", (void(__cdecl *)())CL_DecAnimWeight_f, &CL_DecAnimWeight_f_VAR);
    cl_testAnimWeight = Dvar_RegisterFloat("cl_testAnimWeight", 0.0, 0.0, 1.0, 0, "test animation weighting");
    Cmd_AddCommandInternal("modelDumpInfo", XModelDumpInfo, &XModelDumpInfo_VAR);
    //CL_Xenon_RegisterDvars();
    //CL_Xenon_RegisterCommands();
    Cmd_AddCommandInternal("stopControllerRumble", CL_StopControllerRumbles, &CL_StopControllerRumbles_VAR);
    Com_Printf(14, "----- Initializing Renderer ----\n");

    CL_InitRef();

    SCR_Init();
    Cbuf_Execute(0, cl_controller_in_use);
    clientUIActives[0].isRunning = 1;
    clients[0].usingAds = 0;
    Com_Printf(14, "----- Client Initialization Complete -----\n");
}

