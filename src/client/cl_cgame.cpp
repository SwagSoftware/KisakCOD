#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"
#include <qcommon/cmd.h>
#include <gfx_d3d/r_fog.h>
#include <gfx_d3d/r_cinematic.h>
#include <universal/com_files.h>
#include <stringed/stringed_hooks.h>
#include <cgame/cg_servercmds.h>
#include <gfx_d3d/r_init.h>
#include <cgame/cg_snapshot.h>
#include "cl_demo.h"
#include <cgame/cg_main.h>
#include <server/server.h>
#include <EffectsCore/fx_system.h>
#include <cgame/cg_newdraw.h>
#include <ui/ui.h>
#include <gfx_d3d/r_scene.h>
#include <universal/com_sndalias.h>
#include <xanim/dobj_utils.h>
#include "cl_pose.h"
#include <game/savememory.h>
#include <server/sv_game.h>
#include <qcommon/com_bsp.h>

char bigConfigString[8192];
const float g_color_table[8][4]
{
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.36f, 0.36f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.36f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f }
};


void __cdecl CL_SetLocalClientConnectionState(int localClientNum, connstate_t connstate)
{
    iassert(localClientNum == 0);
    clientUIActives[0].connectionState = connstate;
}

void __cdecl TRACK_cl_cgame()
{
    track_static_alloc_internal(bigConfigString, 0x2000, "bigConfigString", 9);
    track_static_alloc_internal((void *)g_color_table, 128, "g_color_table", 10);
}

void __cdecl CL_GetScreenDimensions(unsigned int *width, unsigned int *height, float *aspect)
{
    iassert(width);
    iassert(height);
    iassert(aspect);

    *width = cls.vidConfig.displayWidth;
    *height = cls.vidConfig.displayHeight;
    *aspect = cls.vidConfig.aspectRatioWindow;
}

float __cdecl CL_GetScreenAspectRatioDisplayPixel()
{
    double aspectRatioDisplayPixel; // fp1

    aspectRatioDisplayPixel = cls.vidConfig.aspectRatioDisplayPixel;
    return *((float *)&aspectRatioDisplayPixel + 1);
}

int __cdecl CL_GetUserCmd(int localClientNum, int cmdNumber, usercmd_s *ucmd)
{
    int v5; // r6

    iassert(localClientNum == 0);
    
    if (cmdNumber > clients[0].cmdNumber)
    {
        Com_Error(ERR_DROP, "CL_GetUserCmd:cmdNumber %i >= %i", cmdNumber, clients[0].cmdNumber);
    }

    if (cmdNumber <= v5 - 64 || cmdNumber <= 0)
        return 0;

    memcpy(ucmd, &clients[0].cmds[cmdNumber & 0x3F], sizeof(usercmd_s));

    return 1;
}

int __cdecl CL_GetCurrentCmdNumber(int localClientNum)
{
    iassert(localClientNum == 0);
    return clients[0].cmdNumber;
}

void __cdecl CL_GetCurrentSnapshotNumber(int localClientNum, int *snapshotNumber, int *serverTime)
{
    iassert(localClientNum == 0);

    *snapshotNumber = clients[0].snap.messageNum;
    *serverTime = clients[0].snap.serverTime;
}

int __cdecl CL_GetSnapshot(int localClientNum, snapshot_s *snapshot)
{
    int numEntities; // r28
    int v4; // r30
    int *entityNums; // r29
    unsigned int v6; // r7

    iassert(localClientNum == 0);

    if (!clients[0].snapshots[0].valid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 155, 0, "%s", "clSnap->valid");
    if (clients[0].parseEntitiesNum - clients[0].snapshots[0].parseEntitiesNum >= 2048)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            159,
            0,
            "%s",
            "cl->parseEntitiesNum - clSnap->parseEntitiesNum < MAX_PARSE_ENTITIES");
    snapshot->snapFlags = clients[0].snapshots[0].snapFlags;
    snapshot->serverCommandSequence = clients[0].snapshots[0].serverCommandNum;
    snapshot->serverTime = clients[0].snapshots[0].serverTime;
    memcpy(&snapshot->ps, &clients[0].snapshots[0].ps, sizeof(snapshot->ps));
    numEntities = clients[0].snapshots[0].numEntities;
    if (clients[0].snapshots[0].numEntities > 2048)
    {
        if (com_statmon->current.enabled)
            StatMon_Warning(4, 3000, "code_warning_snapshotents");
        else
            Com_DPrintf(14, "CL_GetSnapshot: truncated %i entities to %i\n", clients[0].snapshots[0].numEntities, 2048);
        numEntities = 2048;
    }
    v4 = 0;
    snapshot->numEntities = numEntities;
    if (numEntities > 0)
    {
        entityNums = snapshot->entityNums;
        do
        {
            v6 = *(int *)((char *)clients[0].parseEntityNums + ((4 * (clients[0].snapshots[0].parseEntitiesNum + v4)) & 0x1FFC));
            *entityNums = v6;
            if (v6 >= 0x880)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
                    181,
                    0,
                    "snapshot->entityNums[i] doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    v6,
                    2176);
            ++v4;
            ++entityNums;
        } while (v4 < numEntities);
    }
    return 1;
}

void __cdecl CL_SetUserCmdWeapons(int localClientNum, int weapon, int offHandIndex)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].cgameUserCmdWeapon = weapon;
    clients[0].cgameUserCmdOffHandIndex = offHandIndex;
}

void __cdecl CL_SetUserCmdAimValues(
    int localClientNum,
    double gunPitch,
    double gunYaw,
    double gunXOfs,
    double gunYOfs,
    double gunZOfs)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].cgameUserCmdGunPitch = gunPitch;
    clients[0].cgameUserCmdGunYaw = gunYaw;
    clients[0].cgameUserCmdGunXOfs = gunXOfs;
    clients[0].cgameUserCmdGunYOfs = gunYOfs;
    clients[0].cgameUserCmdGunZOfs = gunZOfs;
}

void __cdecl CL_SetFOVSensitivityScale(int localClientNum, double scale)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].cgameFOVSensitivityScale = scale;
}

void __cdecl CL_SetExtraButtons(int localClientNum, int buttons)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].cgameExtraButtons |= buttons;
}

void CL_ConfigstringModified()
{
    int nesting; // r7
    const char *v1; // r3
    unsigned int index; // r30
    int v3; // r7
    const char *v4; // r26
    const char *v6; // r29
    const char *v7; // r10
    const char *v8; // r11
    int v9; // r8

    v1 = Cmd_Argv(1);
    index = atol(v1);
    if (index > 2814)
        Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");

    v4 = Cmd_Argv(2);
    if (!clients[0].configstrings[index])
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 244, 0, "%s", "cl->configstrings[index]");

    v6 = SL_ConvertToString(clients[0].configstrings[index]);
    if (!v6)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 246, 0, "%s", "old");

    v7 = v4;
    v8 = v6;
    do
    {
        v9 = *(unsigned __int8 *)v8 - *(unsigned __int8 *)v7;
        if (!*v8)
            break;
        ++v8;
        ++v7;
    } while (!v9);
    if (v9)
    {
        SL_RemoveRefToString(clients[0].configstrings[index]);
        clients[0].configstrings[index] = SL_GetString_(v4, 0, 19);
    }
}

void __cdecl CL_Restart()
{
    unsigned __int16 *configstrings; // r31

    SND_ResetPauseSettingsToDefaults();
    //CG_StopAllRumbles(0); // KISAKTODO: cg_rumble
    R_Cinematic_StopPlayback();
    configstrings = clients[0].configstrings;
    do
    {
        if (*configstrings)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 279, 0, "%s", "!cl->configstrings[i]");
        *configstrings++ = SL_GetString_("", 0, 19);
    } while ((int)configstrings < (int)clients[0].mapname);
    Con_ClearNotify(0);
    Con_ClearErrors(0);
    Con_InitMessageBuffer();
    Con_InitGameMsgChannels();
    R_ClearFogs();
    CG_ShutdownPhysics(0);
    CG_MapInit(1);
    CG_ParseSunLight(0);
}

int __cdecl CL_PreprocessServerCommand(const char *s)
{
    int nesting; // r7
    char *v3; // r31
    char *v4; // r11
    const char *v5; // r10
    int v6; // r8
    char *v7; // r11
    const char *v8; // r10
    int v9; // r8
    char *v10; // r11
    const char *v11; // r10
    int v12; // r8
    char *v13; // r11
    const char *v14; // r10
    int v15; // r8
    const char *v16; // r3
    char *v17; // r10
    const char *v18; // r31
    const char *v20; // r11
    int v21; // r10
    const char *v23; // r9
    char *v24; // r11
    char *v26; // r11
    int v27; // r10
    const char *v28; // r31
    const char *v29; // r3
    const char *v31; // r3
    char *v32; // r10
    const char *v33; // r31
    const char *v35; // r11
    int v36; // r10
    const char *v38; // r9
    char *v39; // r11
    char *v41; // r11
    int v42; // r10
    const char *v43; // r10
    char *v44; // r11
    int v45; // r8

    while (1)
    {
        Cmd_TokenizeString((char *)s);

        v3 = (char*)Cmd_Argv(0);

        v4 = v3;
        v5 = "disconnect";
        do
        {
            v6 = (unsigned __int8)*v4 - *(unsigned __int8 *)v5;
            if (!*v4)
                break;
            ++v4;
            ++v5;
        } while (!v6);

        if (!v6)
            Com_Error(ERR_SERVERDISCONNECT, "EXE_SERVER_DISCONNECTED");

        v7 = v3;
        v8 = "bcs0";
        do
        {
            v9 = (unsigned __int8)*v7 - *(unsigned __int8 *)v8;
            if (!*v7)
                break;
            ++v7;
            ++v8;
        } while (!v9);

        if (!v9)
        {
            Cmd_EndTokenizedString();
            Cmd_TokenizeStringWithLimit((char *)s, 3);
            v28 = Cmd_Argv(2);
            v29 = Cmd_Argv(1);
            Com_sprintf(bigConfigString, 0x2000, "cs %s %s", v29, v28);
            Cmd_EndTokenizedString();
            return 0;
        }
        v10 = v3;
        v11 = "bcs1";
        do
        {
            v12 = (unsigned __int8)*v10 - *(unsigned __int8 *)v11;
            if (!*v10)
                break;
            ++v10;
            ++v11;
        } while (!v12);
        if (!v12)
            break;
        v13 = v3;
        v14 = "bcs2";
        do
        {
            v15 = (unsigned __int8)*v13 - *(unsigned __int8 *)v14;
            if (!*v13)
                break;
            ++v13;
            ++v14;
        } while (!v15);
        if (v15)
        {
            v43 = "cs";
            v44 = v3;
            do
            {
                v45 = (unsigned __int8)*v44 - *(unsigned __int8 *)v43;
                if (!*v44)
                    break;
                ++v44;
                ++v43;
            } while (!v45);
            if (!v45)
            {
                Cmd_EndTokenizedString();
                Cmd_TokenizeStringWithLimit((char*)s, 3);
                CL_ConfigstringModified();
            }
            return 1;
        }
        Cmd_EndTokenizedString();
        Cmd_TokenizeStringWithLimit((char *)s, 3);
        v16 = Cmd_Argv(2);
        v17 = bigConfigString;
        v18 = v16;
        while (*v17++)
            ;
        v20 = v16;
        v21 = v17 - bigConfigString - 1;
        while (*(unsigned __int8 *)v20++)
            ;
        if ((unsigned int)(v20 - v16 + v21) >= 0x2000)
            Com_Error(ERR_DROP, "bcs exceeded BIG_INFO_STRING");
        v23 = v18;
        v24 = bigConfigString;
        while (*v24++)
            ;
        v26 = v24 - 1;
        do
        {
            v27 = *(unsigned __int8 *)v23++;
            *v26++ = v27;
        } while (v27);
        Cmd_EndTokenizedString();
        s = bigConfigString;
    }
    Cmd_EndTokenizedString();
    Cmd_TokenizeStringWithLimit((char*)s, 3);
    v31 = Cmd_Argv(2);
    v32 = bigConfigString;
    v33 = v31;
    while (*v32++)
        ;
    v35 = v31;
    v36 = v32 - bigConfigString - 1;
    while (*(unsigned __int8 *)v35++)
        ;
    if ((unsigned int)(v35 - v31 - 1 + v36) >= 0x2000)
        Com_Error(ERR_DROP, "bcs exceeded BIG_INFO_STRING");
    v38 = v33;
    v39 = bigConfigString;
    while (*v39++)
        ;
    v41 = v39 - 1;
    do
    {
        v42 = *(unsigned __int8 *)v38++;
        *v41++ = v42;
    } while (v42);
    Cmd_EndTokenizedString();
    return 0;
}

int __cdecl CL_CGameNeedsServerCommand(int localClientNum, int serverCommandNumber)
{
    int sequence; // r11
    const char *v4; // r3

    iassert(localClientNum == 0);

    sequence = clientConnections[0].serverCommands.header.sequence;

    if (serverCommandNumber <= clientConnections[0].serverCommands.header.sequence - 256)
    {
        Com_Error(ERR_DROP, "a reliable command was cycled out");
        sequence = clientConnections[0].serverCommands.header.sequence;
    }
    if (serverCommandNumber > sequence)
        Com_Error(ERR_DROP, "requested a command not received");
    v4 = &clientConnections[0].serverCommands.buf[*(int *)((char *)clientConnections[0].serverCommands.commands
        + ((4 * serverCommandNumber) & 0x3FC))];
    clientConnections[0].serverCommands.header.sent = serverCommandNumber;
    return CL_PreprocessServerCommand(v4);
}

void __cdecl CL_ArchiveServerCommands(MemoryFile *memFile)
{
    int i; // r30
    int rover; // r30

    MemFile_ArchiveData(memFile, 12, &clientConnections[0].serverCommands);
    for (i = clientConnections[0].serverCommands.header.sent + 1;
        i <= clientConnections[0].serverCommands.header.sequence;
        ++i)
    {
        if (!memFile)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\../universal/memfile.h", 188, 0, "%s", "memFile");
        if (!memFile->archiveProc)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\../universal/memfile.h",
                189,
                0,
                "%s",
                "memFile->archiveProc");
        memFile->archiveProc(memFile, 4, (unsigned char *)clientConnections[0].serverCommands.commands + ((4 * i) & 0x3FC));
    }
    rover = clientConnections[0].serverCommands.header.rover;
    if (!memFile)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\../universal/memfile.h", 188, 0, "%s", "memFile");
    if (!memFile->archiveProc)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../universal/memfile.h",
            189,
            0,
            "%s",
            "memFile->archiveProc");
    memFile->archiveProc(memFile, rover, (byte*)clientConnections[0].serverCommands.buf);
}

void __cdecl CL_LoadServerCommands(SaveGame *save)
{
    int i; // r31

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 409, 0, "%s", "save");
    SaveMemory_LoadRead(&clientConnections[0].serverCommands, 12, save);
    for (i = clientConnections[0].serverCommands.header.sent + 1;
        i <= clientConnections[0].serverCommands.header.sequence;
        ++i)
    {
        SaveMemory_LoadRead((char *)clientConnections[0].serverCommands.commands + ((4 * i) & 0x3FC), 4, save);
    }
    SaveMemory_LoadRead(clientConnections[0].serverCommands.buf, clientConnections[0].serverCommands.header.rover, save);
}

void __cdecl CL_ShutdownCGame()
{
    if (clientUIActives[0].cgameInitCalled)
    {
        CG_Shutdown(0);
        clientUIActives[0].cgameInitCalled = 0;
        clientUIActives[0].cgameInitialized = 0;
        clientUIActives[0].isLoadComplete = 0;
        track_shutdown(1);
    }
    else if (clientUIActives[0].cgameInitialized)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 431, 0, "%s", "!clUI->cgameInitialized");
    }
}

static int warnCount;
int __cdecl CL_DObjCreateSkelForBone(DObj_s *obj, int boneIndex)
{
    int SkelTimeStamp; // r31
    unsigned int AllocSkelSize; // r3
    char *v7; // r4

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 461, 0, "%s", "obj");
    SkelTimeStamp = CL_GetSkelTimeStamp();
    if (DObjSkelExists(obj, SkelTimeStamp))
        return DObjSkelIsBoneUpToDate(obj, boneIndex);
    AllocSkelSize = DObjGetAllocSkelSize(obj);
    v7 = CL_AllocSkelMemory(AllocSkelSize);
    if (v7)
    {
        DObjCreateSkel(obj, v7, SkelTimeStamp);
        return 0;
    }
    else
    {
        if (warnCount != SkelTimeStamp)
        {
            warnCount = SkelTimeStamp;
            Com_PrintWarning(14, "WARNING: CL_SKEL_MEMORY_SIZE exceeded - not calculating skeleton\n");
        }
        return 1;
    }
}

void __cdecl LoadWorld(const char *name, int savegame)
{
    int v2; // [sp+50h] [-10h] BYREF

    R_LoadWorld((char*)name, &v2, savegame);
    SV_SetCheckSum(v2);
}

void __cdecl CL_SubtitlePrint(int localClientNum, const char *text, int duration, int pixelWidth)
{
    const char *String; // r3

    if (cg_subtitles->current.enabled)
    {
        String = SEH_StringEd_GetString(text);
        if (!String)
        {
            if (loc_warnings->current.enabled)
            {
                if (loc_warningsAsErrors->current.enabled)
                    Com_Error(ERR_LOCALIZATION, "Could not translate subtitle text: \"%s\"", text);
                else
                    Com_PrintWarning(14, "WARNING: Could not translate subtitle text: \"%s\"\n", text);
                String = va("^1UNLOCALIZED(^7%s^1)^7", text);
            }
            else
            {
                String = text;
            }
        }
        CL_ConsolePrint(localClientNum, 4, String, duration, pixelWidth, 32);
    }
}

const char *__cdecl CL_GetConfigString(int localClientNum, unsigned int configStringIndex)
{
    unsigned int v3; // r30

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            526,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (configStringIndex >= 0xAFF)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            527,
            0,
            "configStringIndex doesn't index MAX_CONFIGSTRINGS\n\t%i not in [0, %i)",
            configStringIndex,
            2815);
    v3 = configStringIndex;
    if (!clients[0].configstrings[v3])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            531,
            0,
            "%s",
            "cl->configstrings[configStringIndex]");
    return SL_ConvertToString(clients[0].configstrings[v3]);
}

// attributes: thunk
snd_alias_t *__cdecl CL_PickSoundAlias(const char *aliasname)
{
    return Com_PickSoundAlias(aliasname);
}

void __cdecl CL_FinishLoadingModels()
{
    ;
}

void __cdecl CL_GetViewForward(float *forward)
{
    *forward = cls.renderForward[0];
    forward[1] = cls.renderForward[1];
    forward[2] = cls.renderForward[2];
}

void __cdecl CL_GetViewPos(float *pos)
{
    *pos = cls.renderPos[0];
    pos[1] = cls.renderPos[1];
    pos[2] = cls.renderPos[2];
}

void __cdecl CL_RenderScene(const refdef_s *fd)
{
    cls.renderPos[0] = fd->vieworg[0];
    cls.renderPos[1] = fd->vieworg[1];
    cls.renderPos[2] = fd->vieworg[2];
    cls.renderForward[0] = fd->viewaxis[0][0];
    cls.renderForward[1] = fd->viewaxis[0][1];
    cls.renderForward[2] = fd->viewaxis[0][2];
    R_RenderScene(fd);
}

void __cdecl CL_SetFullScreenViewport()
{
    R_AddCmdSetViewportValues(0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
}

// attributes: thunk
void __cdecl CL_SetViewport(int x, int y, int width, int height)
{
    R_AddCmdSetViewportValues(x, y, width, height);
}

void __cdecl CL_DrawStretchPicPhysical(
    float x,
    float y,
    float w,
    float h,
    float s1,
    float t1,
    float s2,
    float t2,
    const float *color,
    Material *material)
{
    R_AddCmdDrawStretchPic(x, y, w, h, s1, t1, s2, t2, color, material);
}

void __cdecl CL_DrawStretchPicPhysicalFlipST(
    float x,
    float y,
    float w,
    float h,
    float s1,
    float t1,
    float s2,
    float t2,
    const float *color,
    Material *material)
{
    R_AddCmdDrawStretchPicFlipST(x, y, w, h, s1, t1, s2, t2, color, material);
}

void __cdecl CL_DrawStretchPic(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material,
    int a14,
    int a15,
    float a16,
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
    float a27)
{
    Material *v31; // r4
    const float *v32; // r3

    a21 = x;
    a23 = y;
    a25 = w;
    a27 = h;
    ScrPlace_ApplyRect(scrPlace, &a21, &a23, &a25, &a27, a14, a15);
    R_AddCmdDrawStretchPic(a21, a23, a25, a27, s1, t1, s2, t2, v32, v31);
}

void __cdecl CL_DrawStretchPicPhysicalRotateXY(
    double x,
    double y,
    double w,
    double h,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material,
    float angle)
{
    float v11; // [sp+8h] [-68h]

    R_AddCmdDrawStretchPicRotateXY(x, y, w, h, s1, t1, s2, t2, v11, color, material);
}

void __cdecl CL_DrawStretchPicFlipST(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material,
    int a14,
    int a15,
    float a16,
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
    float a27)
{
    Material *v31; // r4
    const float *v32; // r3

    a21 = x;
    a23 = y;
    a25 = w;
    a27 = h;
    ScrPlace_ApplyRect(scrPlace, &a21, &a23, &a25, &a27, a14, a15);
    R_AddCmdDrawStretchPicFlipST(a21, a23, a25, a27, s1, t1, s2, t2, v32, v31);
}

void __cdecl CL_DrawStretchPicRotatedST(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double centerS,
    double centerT,
    double radiusST,
    double scaleFinalS,
    const float *color,
    Material *material,
    int a14,
    int a15,
    float scaleFinalT,
    float a17,
    float angle,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27)
{
    Material *v31; // r4
    const float *v32; // r3
    float v33; // [sp+8h] [-A8h]
    float v34; // [sp+10h] [-A0h]

    a21 = x;
    a23 = y;
    a25 = w;
    a27 = h;
    ScrPlace_ApplyRect(scrPlace, &a21, &a23, &a25, &a27, a14, a15);
    R_AddCmdDrawStretchPicRotateST(a21, a23, a25, a27, centerS, centerT, radiusST, scaleFinalS, v33, v34, v32, v31);
}

// attributes: thunk
void __cdecl CL_ProjectionSet2D()
{
    R_AddCmdProjectionSet2D();
}

// attributes: thunk
void __cdecl CL_ProjectionSet3D()
{
    R_AddCmdProjectionSet3D();
}

void __cdecl CL_CapTurnRate(int localClientNum, double maxPitchSpeed, double maxYawSpeed)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].cgameMaxPitchSpeed = maxPitchSpeed;
    clients[0].cgameMaxYawSpeed = maxYawSpeed;
}

void __cdecl CL_SetViewAngles(int localClientNum, float *angles)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].viewangles[0] = *angles;
    clients[0].viewangles[1] = angles[1];
    clients[0].viewangles[2] = angles[2];
}

void __cdecl CL_StartLoading(const char *mapname)
{
    if (!clientUIActives[0].isRunning)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            791,
            0,
            "%s",
            "CL_GetLocalClientUIGlobals( ONLY_LOCAL_CLIENT_NUM )->isRunning");
    SND_ResetPauseSettingsToDefaults();
    CL_InitRenderer();
    CL_StartHunkUsers();
    UI_DrawConnectScreen();
    Dvar_SetInt(cl_paused, 1);
}

void __cdecl CL_InitCGame(int localClientNum, int savegame)
{
    int startTime; // r22
    unsigned __int16 v5; // r11
    const char *info; // r31
    const char *v7; // r31
    int v8; // r3
    __int64 v9; // r11

    startTime = Sys_Milliseconds();
    SND_ErrorCleanup();
    if (!com_sv_running->current.enabled)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            821,
            0,
            "%s",
            "com_sv_running->current.enabled");
    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            548,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    v5 = clients[0].configstrings[0];
    if (!clients[0].configstrings[0])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            826,
            0,
            "%s",
            "cl->configstrings[CS_SERVERINFO]");
        v5 = clients[0].configstrings[0];
    }
    info = SL_ConvertToString(v5);
    if (!info)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 828, 0, "%s", "info");
    v7 = Info_ValueForKey(info, "mapname");
    Com_GetBspFilename(clients[0].mapname, 64, v7);
    //Live_SetCurrentMapname(v7);
    clientUIActives[0].isLoadComplete = 1;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState != CA_LOADING)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            836,
            0,
            "%s",
            "CL_GetLocalClientConnectionState( localClientNum ) == CA_LOADING");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            562,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientConnections[0].serverMessageSequence)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            838,
            0,
            "%s",
            "!CL_GetLocalClientConnection( localClientNum )->serverMessageSequence");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            562,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientConnections[0].serverCommands.header.sent && !cls.demoplaying)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp",
            839,
            0,
            "%s",
            "!CL_GetLocalClientConnection( localClientNum )->serverCommands.header.sent || cls.demoplaying");
    clientUIActives[0].cgameInitCalled = 1;
    CG_Init(localClientNum, savegame);
    //v8 = Sys_Milliseconds();
    //HIDWORD(v9) = "erverTime - cgameGlob->time > 0";
    //LODWORD(v9) = v8 - v4;
    Com_Printf(
        14,
        "CL_InitCGame: %5.2f seconds\n",
        Sys_Milliseconds() - startTime
    );
    R_EndRegistration();
    Con_ClearNotify(0);
    Con_InitMessageBuffer();
    Con_InitGameMsgChannels();
}

void __cdecl CL_FirstSnapshot()
{
    if (!clientUIActives[0].isRunning)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_cgame.cpp", 886, 0, "%s", "clUI->isRunning");
    clients[0].serverTime = com_time;
    Con_TimeJumped(0, com_time);
    CL_ResetSkeletonCache();
    if (cls.demoplaying)
        CL_StartPlayingDemo();
    CG_FirstSnapshot(0);
    //__lwsync();
    clientUIActives[0].cgameInitialized = 1;
}

void __cdecl CL_SetActive()
{
    clientUIActives[0].connectionState = CA_ACTIVE;
}

void __cdecl CL_CreateNextSnap()
{
    __int64 v0; // r11
    int v1; // r4

    LODWORD(v0) = SV_GetPartialFrametime();
    CG_CreateNextSnap(0, (float)((float)v0 * (float)0.001), v1);
}

char *__cdecl CL_TimeDemoLogBaseName(const char *mapname)
{
    char *v1; // r31
    const char *v2; // r30
    int v3; // r11
    char *result; // r3

    v1 = 0;
    v2 = mapname;
    v3 = *mapname;
    if (!*mapname)
        return (char *)v2;
    do
    {
        if (v3 == 47 || v3 == 92)
        {
            v2 = mapname + 1;
            v1 = 0;
        }
        else if (v3 == 46)
        {
            v1 = (char*)mapname;
        }
        v3 = *++mapname;
    } while (*mapname);
    if (!v1)
        return (char *)v2;
    result = va("%s", v2);
    v1[result - v2] = 0;
    return result;
}

void CL_UpdateTimeDemo()
{
    const char *String; // r30
    char *v1; // r3
    const char *v2; // r3
    int v3; // r3
    int v4; // r30
    int v5; // r10

    if (!cls.timeDemoLog)
    {
        String = Dvar_GetString("r_mode");
        v1 = CL_TimeDemoLogBaseName(clients[0].mapname);
        v2 = va("demos/timedemo_%s_mode_%s.csv", v1, String);
        cls.timeDemoLog = FS_FOpenFileWrite(v2);
    }
    v3 = Sys_Milliseconds();
    v4 = v3;
    if (cls.timeDemoStart)
    {
        if (cls.timeDemoLog)
            FS_Printf(cls.timeDemoLog, "%i,%i\n", cls.timeDemoFrames, v3 - cls.timeDemoPrev);
    }
    else
    {
        cls.timeDemoStart = v3;
    }
    v5 = 50 * ++cls.timeDemoFrames;
    cls.timeDemoPrev = v4;
    clients[0].serverTime = v5 + cls.timeDemoBaseTime;
    Com_Printf(14, "time %i (UpdateTimeDemo)\n", v5 + cls.timeDemoBaseTime);
}

void __cdecl CL_SetCGameTime(int localClientNum)
{
    int serverTime; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_ACTIVE)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
                555,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        serverTime = com_time;
        clients[0].serverTime = com_time;
        if (cls.demoplaying)
        {
            if (cls.isTimeDemo)
            {
                CL_UpdateTimeDemo();
                serverTime = clients[0].serverTime;
            }
            if (serverTime >= clients[0].snap.serverTime)
            {
                do
                    CL_ReadDemoMessage();
                while (cls.demoplaying && clients[0].serverTime >= clients[0].snap.serverTime);
            }
        }
    }
}

void __cdecl CL_SetADS(int localClientNum, bool ads)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            555,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clients[0].usingAds = ads;
}

void __cdecl CL_ArchiveClientState(MemoryFile *memFile, int segmentIndex)
{
    unsigned int UsedSize; // r3
    unsigned int v5; // r3
    unsigned int v6; // r3
    unsigned int v7; // r3

    if (MemFile_IsWriting(memFile))
    {
        if (segmentIndex)
            MemFile_StartSegment(memFile, segmentIndex);
        UsedSize = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("sound", UsedSize);
        SND_Save(memFile);
        v5 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v5);
        MemFile_StartSegment(memFile, segmentIndex + 1);
        v6 = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("FX", v6);
        FX_Save(0, memFile);
        v7 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v7);
        CL_SaveSettings(memFile);
        Con_SaveChannels(memFile);
    }
    else
    {
        MemFile_MoveToSegment(memFile, segmentIndex);
        SND_RestoreEventually(memFile);
        MemFile_MoveToSegment(memFile, segmentIndex + 1);
        FX_Restore(0, memFile);
        CL_RestoreSettings(memFile);
        Con_RestoreChannels(memFile);
    }
    CL_ArchiveMessages(memFile);
    MemFile_ArchiveData(memFile, 4, &clients[0].stance);
    CG_ArchiveState(0, memFile);
    R_ArchiveFogState(memFile);
}

void __cdecl CL_LookupColor(unsigned __int8 c, float *color)
{
    unsigned __int8 v3; // r3
    float *v4; // r11
    double v5; // fp0

    v3 = ColorIndex(c);
    if (v3 >= 8u)
    {
        v5 = 1.0;
        *color = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
    }
    else
    {
        v4 = (float *)g_color_table[v3];
        *color = *v4;
        color[1] = v4[1];
        color[2] = v4[2];
        v5 = v4[3];
    }
    color[3] = v5;
}

bool __cdecl CL_IsCgameInitialized(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
            548,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return clientUIActives[0].cgameInitialized;
}

