#include "client_mp.h"

#include <cgame_mp/cg_local_mp.h>
#include <client/client.h>

#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>

#include <gfx_d3d/r_rendercmds.h>

#include <xanim/xanim.h>
#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/com_bsp.h>
#include <universal/com_sndalias.h>
#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_bsp.h>
#include <database/database.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <ui/ui.h>
#include <EffectsCore/fx_system.h>
#include <gfx_d3d/r_fog.h>

float color_allies[4];
float color_axis[4];

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

void __cdecl TRACK_cl_cgame()
{
    track_static_alloc_internal(bigConfigString, 0x2000, "bigConfigString", 9);
    track_static_alloc_internal((void *)g_color_table, 128, "g_color_table", 10);
}

void __cdecl CL_GetScreenDimensions(int *width, int *height, float *aspect)
{
    if (!width)
        MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 93, 0, "%s", "width");
    if (!height)
        MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 94, 0, "%s", "height");
    if (!aspect)
        MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 95, 0, "%s", "aspect");
    *width = cls.vidConfig.displayWidth;
    *height = cls.vidConfig.displayHeight;
    *aspect = cls.vidConfig.aspectRatioWindow;
}

double __cdecl CL_GetScreenAspectRatioDisplayPixel()
{
    return cls.vidConfig.aspectRatioDisplayPixel;
}

int __cdecl CL_GetUserCmd(int localClientNum, int cmdNumber, usercmd_s *ucmd)
{
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (cmdNumber > LocalClientGlobals->cmdNumber)
        Com_Error(ERR_DROP, "CL_GetUserCmd: %i >= %i", cmdNumber, LocalClientGlobals->cmdNumber);
    if (cmdNumber <= LocalClientGlobals->cmdNumber - 128)
        return 0;
    memcpy(ucmd, &LocalClientGlobals->cmds[cmdNumber & 0x7F], sizeof(usercmd_s));
    return 1;
}

int __cdecl CL_GetCurrentCmdNumber(int localClientNum)
{
    return CL_GetLocalClientGlobals(localClientNum)->cmdNumber;
}

void __cdecl CL_GetCurrentSnapshotNumber(int localClientNum, int *snapshotNumber, int *serverTime)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    *snapshotNumber = LocalClientGlobals->snap.messageNum;
    *serverTime = LocalClientGlobals->snap.serverTime;
}

int __cdecl CL_GetSnapshot(int localClientNum, int snapshotNumber, snapshot_s *snapshot)
{
    const char *v4; // eax
    unsigned int number; // [esp+8h] [ebp-418h]
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-414h]
    bool entityFound[1024]; // [esp+10h] [ebp-410h] BYREF
    const clSnapshot_t *clSnap; // [esp+414h] [ebp-Ch]
    int i; // [esp+418h] [ebp-8h]
    int count; // [esp+41Ch] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (snapshotNumber > LocalClientGlobals->snap.messageNum)
        Com_Error(ERR_DROP, "CL_GetSnapshot: snapshotNumber > cl->snapshot.messageNum");
    if (LocalClientGlobals->snap.messageNum - snapshotNumber >= 32)
        return 0;
    clSnap = &LocalClientGlobals->snapshots[snapshotNumber & 0x1F];
    if (!clSnap->valid)
        return 0;
    if (LocalClientGlobals->parseEntitiesNum - clSnap->parseEntitiesNum >= 2048)
        return 0;
    if (LocalClientGlobals->parseClientsNum - clSnap->parseClientsNum >= 2048)
        return 0;
    snapshot->snapFlags = clSnap->snapFlags;
    snapshot->serverCommandSequence = clSnap->serverCommandNum;
    snapshot->ping = clSnap->ping;
    snapshot->serverTime = clSnap->serverTime;
    memcpy((unsigned __int8 *)&snapshot->ps, (unsigned __int8 *)&clSnap->ps, sizeof(snapshot->ps));
    count = clSnap->numEntities;
    if (count > 512)
    {
        if (com_statmon->current.enabled)
            StatMon_Warning(4, 3000, "code_warning_snapshotents");
        else
            Com_DPrintf(14, "CL_GetSnapshot: truncated %i entities to %i\n", count, 512);
        count = 512;
    }
    snapshot->numEntities = count;
    memset((unsigned __int8 *)entityFound, 0, sizeof(entityFound));
    for (i = 0; i < count; ++i)
    {
        memcpy(
            &snapshot->entities[i],
            &LocalClientGlobals->parseEntities[((_WORD)i + (unsigned __int16)clSnap->parseEntitiesNum) & 0x7FF],
            sizeof(snapshot->entities[i]));
        number = snapshot->entities[i].number;
        if (number >= 0x400)
            MyAssertHandler(
                ".\\client_mp\\cl_cgame_mp.cpp",
                221,
                0,
                "%s\n\t(number) = %i",
                "(( number >= 0 && number < (1<<10) ))",
                number);
        if (entityFound[number])
        {
            v4 = va("EntityNum %i was found twice in this snapshot", number);
            MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 222, 0, "%s\n\t%s", "!entityFound[ number ]", v4);
        }
        entityFound[number] = 1;
    }
    count = clSnap->numClients;
    if (count > 64)
        count = 64;
    memset((unsigned __int8 *)snapshot->clients, 0xAAu, sizeof(snapshot->clients));
    snapshot->numClients = count;
    for (i = 0; i < count; ++i)
        memcpy(
            &snapshot->clients[i],
            &LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)clSnap->parseClientsNum) & 0x7FF],
            sizeof(snapshot->clients[i]));
    return 1;
}

void __cdecl CL_SetUserCmdWeapons(int localClientNum, int weapon, int offHandIndex)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameUserCmdWeapon = weapon;
    LocalClientGlobals->cgameUserCmdOffHandIndex = offHandIndex;
}

void __cdecl CL_SetUserCmdAimValues(int localClientNum, const float *kickAngles)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameKickAngles[0] = *kickAngles;
    LocalClientGlobals->cgameKickAngles[1] = kickAngles[1];
    LocalClientGlobals->cgameKickAngles[2] = kickAngles[2];
}

void __cdecl CL_SetUserCmdOrigin(
    int localClientNum,
    const float *origin,
    const float *velocity,
    const float *viewangles,
    int bobCycle,
    int movementDir)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgamePredictedDataServerTime = LocalClientGlobals->serverTime;
    LocalClientGlobals->cgameOrigin[0] = *origin;
    LocalClientGlobals->cgameOrigin[1] = origin[1];
    LocalClientGlobals->cgameOrigin[2] = origin[2];
    LocalClientGlobals->cgameVelocity[0] = *velocity;
    LocalClientGlobals->cgameVelocity[1] = velocity[1];
    LocalClientGlobals->cgameVelocity[2] = velocity[2];
    LocalClientGlobals->cgameBobCycle = bobCycle;
    LocalClientGlobals->cgameMovementDir = movementDir;
    LocalClientGlobals->cgameViewangles[0] = *viewangles;
    LocalClientGlobals->cgameViewangles[1] = viewangles[1];
    LocalClientGlobals->cgameViewangles[2] = viewangles[2];
}

void __cdecl CL_SetFOVSensitivityScale(int localClientNum, float scale)
{
    CL_GetLocalClientGlobals(localClientNum)->cgameFOVSensitivityScale = scale;
}

void __cdecl CL_SetExtraButtons(int localClientNum, int buttons)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameExtraButtons |= buttons;
}

void __cdecl CL_DumpReliableCommands(int localClientNum)
{
    clientConnection_t *clc; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    for (i = 0; i < 128; ++i)
        Com_PrintError(1, "cmd %5d: '%s'\n", i, clc->serverCommands[i]);
}

int __cdecl CL_CGameNeedsServerCommand(int localClientNum, int serverCommandNumber)
{
    int result; // eax
    const char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    char *v7; // eax
    const char *v8; // eax
    const char *v9; // [esp-4h] [ebp-9Ch]
    const char *v10; // [esp-4h] [ebp-9Ch]
    clientActive_t *LocalClientGlobals; // [esp+84h] [ebp-14h]
    clientConnection_t *clc; // [esp+88h] [ebp-10h]
    char *s; // [esp+8Ch] [ebp-Ch]
    const char *sa; // [esp+8Ch] [ebp-Ch]
    const char *sb; // [esp+8Ch] [ebp-Ch]
    const char *cmd; // [esp+90h] [ebp-8h]
    int argc; // [esp+94h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if (serverCommandNumber <= clc->serverCommandSequence - 128)
    {
        if (clc->demoplaying)
            return 0;
        Com_Printf(14, "===== CL_CGameNeedsServerCommand =====\n");
        Com_Printf(14, "serverCommandNumber: %d\n", serverCommandNumber & 0x7F);
        CL_DumpReliableCommands(localClientNum);
        Com_Error(ERR_DROP, "CL_CGameNeedsServerCommand: EXE_ERR_RELIABLE_CYCLED_OUT");
    }
    if (serverCommandNumber > clc->serverCommandSequence)
        Com_Error(ERR_DROP, "CL_CGameNeedsServerCommand: EXE_ERRO_NOT_RECEIVED");
    s = clc->serverCommands[serverCommandNumber & 0x7F];
    clc->lastExecutedServerCommand = serverCommandNumber;
    if (cl_showServerCommands->current.enabled)
        Com_DPrintf(14, "serverCommand: %i : %s\n", serverCommandNumber, s);
    while (2)
    {
        Cmd_TokenizeString(s);
        cmd = Cmd_Argv(0);
        argc = Cmd_Argc();
        switch (*cmd)
        {
        case 'B':
        case 'n':
            Con_ClearNotify(localClientNum);
            LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
            memset((unsigned __int8 *)LocalClientGlobals->cmds, 0, sizeof(LocalClientGlobals->cmds));
            return 1;
        case 'd':
            Cmd_EndTokenizedString();
            Cmd_TokenizeStringWithLimit(s, 3);
            CL_ConfigstringModified(localClientNum);
            return 1;
        case 'w':
            if (argc >= 3 && (v3 = Cmd_Argv(2), !_stricmp(v3, "PB")))
            {
                v9 = Cmd_Argv(1);
                v4 = SEH_SafeTranslateString((char*)"EXE_SERVERDISCONNECTREASON");
                v5 = UI_ReplaceConversionString(v4, v9);
                v6 = va("%s", v5);
                Com_Error(ERR_SERVERDISCONNECT, v6);
            }
            else if (argc >= 2)
            {   
                v7 = (char *)Cmd_Argv(1);
                CL_DisconnectError(v7);
            }
            else
            {
                Com_Error(ERR_SERVERDISCONNECT, "EXE_SERVER_DISCONNECTED");
            }
            goto $LN7_22;
        case 'x':
        $LN7_22:
            Cmd_EndTokenizedString();
            Cmd_TokenizeStringWithLimit(s, 3);
            v10 = Cmd_Argv(2);
            v8 = Cmd_Argv(1);
            Com_sprintf(bigConfigString, 0x2000u, "%c %s %s", 100, v8, v10);
            Cmd_EndTokenizedString();
            result = 0;
            break;
        case 'y':
            Cmd_EndTokenizedString();
            Cmd_TokenizeStringWithLimit(s, 3);
            sa = Cmd_Argv(2);
            if (strlen(sa) + strlen(bigConfigString) >= 0x2000)
                Com_Error(ERR_DROP, "bcs exceeded BIG_INFO_STRING");
            strcat(bigConfigString, sa);
            Cmd_EndTokenizedString();
            result = 0;
            break;
        case 'z':
            Cmd_EndTokenizedString();
            Cmd_TokenizeStringWithLimit(s, 3);
            sb = Cmd_Argv(2);
            if (strlen(bigConfigString) + strlen(sb) + 1 >= 0x2000)
                Com_Error(ERR_DROP, "bcs exceeded BIG_INFO_STRING");
            strcat(bigConfigString, sb);
            s = bigConfigString;
            Cmd_EndTokenizedString();
            continue;
        default:
            result = 1;
            break;
        }
        return result;
    }
}

void __cdecl CL_ConfigstringModified(int localClientNum)
{
    const char *v1; // eax
    unsigned int v2; // [esp+0h] [ebp-4Ch]
    clientActive_t *LocalClientGlobals; // [esp+24h] [ebp-28h]
    unsigned __int8 *oldGs; // [esp+28h] [ebp-24h]
    char *dup; // [esp+2Ch] [ebp-20h]
    int index; // [esp+3Ch] [ebp-10h]
    const char *s; // [esp+40h] [ebp-Ch]
    int i; // [esp+44h] [ebp-8h]
    const char *old; // [esp+48h] [ebp-4h]

    LargeLocal oldGs_large_local(0x2262C);
    //LargeLocal::LargeLocal(&oldGs_large_local, 140844);
    //oldGs = LargeLocal::GetBuf(&oldGs_large_local);
    oldGs = oldGs_large_local.GetBuf();
    v1 = Cmd_Argv(1);
    index = atoi(v1);
    if ((unsigned int)index >= 2442)
        Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
    s = Cmd_Argv(2);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    old = &LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.stringOffsets[index]];
    if (strcmp(old, s))
    {
        memcpy(oldGs, (unsigned __int8 *)&LocalClientGlobals->gameState, 0x2262Cu);
        memset((unsigned __int8 *)&LocalClientGlobals->gameState, 0, sizeof(LocalClientGlobals->gameState));
        LocalClientGlobals->gameState.dataCount = 1;
        for (i = 0; i < 2442; ++i)
        {
            if (i == index)
                dup = (char *)s;
            else
                dup = (char *)&oldGs[*(unsigned int *)&oldGs[4 * i] + 9768];
            if (*dup)
            {
                v2 = strlen(dup);
                if ((int)(v2 + LocalClientGlobals->gameState.dataCount + 1) > 0x20000)
                    Com_Error(ERR_DROP, "MAX_GAMESTATE_CHARS exceeded");
                LocalClientGlobals->gameState.stringOffsets[i] = LocalClientGlobals->gameState.dataCount;
                memcpy(
                    (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                    (unsigned __int8 *)dup,
                    v2 + 1);
                LocalClientGlobals->gameState.dataCount += v2 + 1;
            }
        }
        if (index == 1)
            CL_SystemInfoChanged(localClientNum);
    }
    //LargeLocal::~LargeLocal(&oldGs_large_local);
}

void __cdecl CL_CM_LoadMap(char *mapname)
{
    int checksum; // [esp+0h] [ebp-4h] BYREF

    if (!useFastFile->current.enabled)
        Com_LoadBsp(mapname);
    CM_LoadMap(mapname, &checksum);
    if (!com_sv_running->current.enabled)
    {
        CM_LinkWorld();
        Com_LoadWorld(mapname);
    }
}

void __cdecl CL_ShutdownCGame(int localClientNum)
{
    Com_UnloadSoundAliases(SASYS_CGAME);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].cgameInitCalled)
    {
        CG_Shutdown(localClientNum);
        clientUIActives[0].cgameInitCalled = 0;
        clientUIActives[0].cgameInitialized = 0;
        track_shutdown(1);
    }
    else if (clientUIActives[0].cgameInitialized)
    {
        MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 599, 0, "%s", "!cl->cgameInitialized");
    }
}

int warnCount;
bool __cdecl CL_DObjCreateSkelForBone(DObj_s *obj, int boneIndex)
{
    char *buf; // [esp+0h] [ebp-Ch]
    unsigned int len; // [esp+4h] [ebp-8h]
    int timeStamp; // [esp+8h] [ebp-4h]

    if (!obj)
        MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 628, 0, "%s", "obj");
    timeStamp = CL_GetSkelTimeStamp();
    if (DObjSkelExists(obj, timeStamp))
        return DObjSkelIsBoneUpToDate(obj, boneIndex);
    len = DObjGetAllocSkelSize(obj);
    buf = CL_AllocSkelMemory(len);
    if (buf)
    {
        DObjCreateSkel(obj, buf, timeStamp);
        return 0;
    }
    else
    {
        if (warnCount != timeStamp)
        {
            warnCount = timeStamp;
            Com_PrintWarning(14, "WARNING: CL_SKEL_MEMORY_SIZE exceeded - not calculating skeleton\n");
        }
        return 1;
    }
}

void __cdecl CL_SubtitlePrint(int localClientNum, const char *text, int duration, int lineWidth)
{
    const char *translation; // [esp+0h] [ebp-4h]
    char *translationa; // [esp+0h] [ebp-4h]

    translation = SEH_StringEd_GetString(text);
    if (translation)
        goto LABEL_8;
    if (!loc_warnings->current.enabled)
    {
        translation = text;
    LABEL_8:
        CL_ConsolePrint(localClientNum, 4, translation, duration, lineWidth, 0);
        return;
    }
    if (loc_warningsAsErrors->current.enabled)
        Com_Error(ERR_LOCALIZATION, "Could not translate subtitle text: \"%s\"", text);
    else
        Com_PrintWarning(14, "WARNING: Could not translate subtitle text: \"%s\"\n", text);
    translationa = va("^1UNLOCALIZED(^7%s^1)^7", text);
    CL_ConsolePrint(localClientNum, 4, translationa, duration, lineWidth, 0);
}

char *__cdecl CL_GetConfigString(int localClientNum, unsigned int configStringIndex)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]

    if (configStringIndex >= 0x98A)
        MyAssertHandler(
            ".\\client_mp\\cl_cgame_mp.cpp",
            726,
            0,
            "configStringIndex doesn't index MAX_CONFIGSTRINGS\n\t%i not in [0, %i)",
            configStringIndex,
            2442);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (LocalClientGlobals->gameState.stringData[0])
        MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 729, 0, "%s", "!cl->gameState.stringData[0]");
    return &LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.stringOffsets[configStringIndex]];
}

snd_alias_t *__cdecl CL_PickSoundAlias(const char *aliasname)
{
    return Com_PickSoundAlias(aliasname);
}

void __cdecl CL_RenderScene(const refdef_s *fd)
{
    cls.debugRenderPos[0] = fd->vieworg[0];
    cls.debugRenderPos[1] = fd->vieworg[1];
    cls.debugRenderPos[2] = fd->vieworg[2];
    R_RenderScene(fd);
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

void __cdecl CL_DrawStretchPicPhysicalRotateXY(
    float x,
    float y,
    float w,
    float h,
    float s1,
    float t1,
    float s2,
    float t2,
    float angle,
    const float *color,
    Material *material)
{
    R_AddCmdDrawStretchPicRotateXY(x, y, w, h, s1, t1, s2, t2, angle, color, material);
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
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float s1,
    float t1,
    float s2,
    float t2,
    const float *color,
    Material *material)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    CL_DrawStretchPicPhysical(x, y, w, h, s1, t1, s2, t2, color, material);
}

void __cdecl CL_DrawStretchPicFlipST(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float s1,
    float t1,
    float s2,
    float t2,
    const float *color,
    Material *material)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    CL_DrawStretchPicPhysicalFlipST(x, y, w, h, s1, t1, s2, t2, color, material);
}

void __cdecl CL_DrawStretchPicRotatedST(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float centerS,
    float centerT,
    float radiusST,
    float scaleFinalS,
    float scaleFinalT,
    float angle,
    const float *color,
    Material *material)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    R_AddCmdDrawStretchPicRotateST(
        x,
        y,
        w,
        h,
        centerS,
        centerT,
        radiusST,
        scaleFinalS,
        scaleFinalT,
        angle,
        color,
        material);
}

void __cdecl CL_CapTurnRate(int localClientNum, float maxPitchSpeed, float maxYawSpeed)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameMaxPitchSpeed = maxPitchSpeed;
    LocalClientGlobals->cgameMaxYawSpeed = maxYawSpeed;
}

void __cdecl CL_SyncTimes(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_ACTIVE)
        CL_FirstSnapshot(localClientNum);
}

int __cdecl LoadWorld(char *mapname)
{
    int checksum; // [esp+0h] [ebp-4h] BYREF

    R_LoadWorld(mapname, &checksum, 0);
    if (!useFastFile->current.enabled)
        Com_UnloadBsp();
    return checksum;
}

void __cdecl CL_StartLoading()
{
    if (CL_AnyLocalClientsRunning())
    {
        CL_InitRenderer();
        CL_StartHunkUsers();
        SCR_UpdateScreen();
    }
}

void __cdecl CL_SetExpectedHunkUsage(const char *mapname)
{
    int handle[2]; // [esp+0h] [ebp-18h] BYREF
    char *buf; // [esp+8h] [ebp-10h]
    int len; // [esp+Ch] [ebp-Ch]
    const char *token; // [esp+10h] [ebp-8h]
    const char *buftrav; // [esp+14h] [ebp-4h] BYREF

    handle[1] = (int)"hunkusage.dat";
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", handle, FS_READ);
    if (len >= 0)
    {
        buf = (char *)Z_Malloc(len + 1, "CL_SetExpectedHunkUsage", 10);
        memset((unsigned __int8 *)buf, 0, len + 1);
        FS_Read((unsigned __int8 *)buf, len, handle[0]);
        FS_FCloseFile(handle[0]);
        buftrav = buf;
        while (1)
        {
            token = (const char *)Com_Parse(&buftrav);
            if (!token)
                MyAssertHandler(".\\client_mp\\cl_cgame_mp.cpp", 536, 0, "%s", "token");
            if (!*token)
                break;
            if (!I_stricmp(token, mapname))
            {
                token = (const char *)Com_Parse(&buftrav);
                if (token)
                {
                    if (*token)
                    {
                        com_expectedHunkUsage = atoi(token);
                        Z_Free(buf, 10);
                        return;
                    }
                }
            }
        }
        Z_Free(buf, 10);
    }
    com_expectedHunkUsage = 0;
}

void __cdecl CL_InitCGame(int localClientNum)
{
    const char *v1; // eax
    int v2; // eax
    XZoneInfo zoneInfo; // [esp+10h] [ebp-70h] BYREF
    clientUIActive_t *clientUIActive; // [esp+20h] [ebp-60h]
    clientActive_t *LocalClientGlobals; // [esp+24h] [ebp-5Ch]
    const char *info; // [esp+28h] [ebp-58h]
    int t1; // [esp+2Ch] [ebp-54h]
    clientConnection_t *clc; // [esp+30h] [ebp-50h]
    int t2; // [esp+34h] [ebp-4Ch]
    char mapname[68]; // [esp+38h] [ebp-48h] BYREF

    t1 = Sys_Milliseconds();
    SND_ErrorCleanup();
    Con_Close(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActive = clientUIActives;
    info = &LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.stringOffsets[0]];
    v1 = Info_ValueForKey((char *)info, "mapname");
    I_strncpyz(mapname, v1, 64);
    Dvar_SetStringByName("mapname", mapname);
    Com_GetBspFilename(LocalClientGlobals->mapname, 0x40u, mapname);
    if (!CL_WasMapAlreadyLoaded())
    {
        Com_InitDObj();
        if (useFastFile->current.enabled)
        {
            zoneInfo.name = mapname;
            zoneInfo.allocFlags = 8;
            zoneInfo.freeFlags = 8;
            DB_LoadXAssets(&zoneInfo, 1u, 0);
        }
        else
        {
            CL_SetExpectedHunkUsage(LocalClientGlobals->mapname);
        }
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1120,
            0,
            "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    clientUIActives[localClientNum].connectionState = CA_LOADING;
    Com_Printf(14, "Setting state to CA_LOADING in CL_InitCGame\n");
    clientUIActive->cgameInitCalled = 1;
    cl_serverLoadingMap = 0;
    clc = CL_GetLocalClientConnection(localClientNum);
    CG_Init(localClientNum, clc->serverMessageSequence, clc->lastExecutedServerCommand, clc->clientNum);
    clientUIActive->cgameInitialized = 1;
    R_BeginRemoteScreenUpdate();
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1120,
            0,
            "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    clientUIActives[localClientNum].connectionState = CA_PRIMED;
    t2 = Sys_Milliseconds();
    Com_Printf(14, "CL_InitCGame: %5.2f seconds\n", (double)(t2 - t1) / 1000.0);
    R_EndRegistration();
    Com_TouchMemory();
    Con_ClearNotify(localClientNum);
    Con_InitMessageBuffer();
    Con_InitGameMsgChannels();
    if (!useFastFile->current.enabled)
    {
        v2 = CL_ControllerIndexFromClientNum(localClientNum);
        Cmd_ExecuteSingleCommand(localClientNum, v2, (char*)"updatehunkusage");
    }
    R_EndRemoteScreenUpdate();
    if (useFastFile->current.enabled)
        DB_SyncXAssets();
}

void __cdecl CL_FirstSnapshot(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ((LocalClientGlobals->snap.snapFlags & 2) == 0)
    {
        CG_RegisterSounds();
        clc = CL_GetLocalClientConnection(localClientNum);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                1120,
                0,
                "client doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        clientUIActives[localClientNum].connectionState = CA_ACTIVE;
        clc->isServerRestarting = 0;
        UI_CloseAll(localClientNum);
        LocalClientGlobals->serverTimeDelta = LocalClientGlobals->snap.serverTime - cls.realtime;
        LocalClientGlobals->oldServerTime = LocalClientGlobals->snap.serverTime;
        LocalClientGlobals->serverTime = LocalClientGlobals->snap.serverTime;
        clc->timeDemoBaseTime = LocalClientGlobals->snap.serverTime;
        Con_TimeJumped(localClientNum, LocalClientGlobals->serverTime);
        if (*(_BYTE *)cl_activeAction->current.integer)
        {
            Cbuf_AddText(localClientNum, cl_activeAction->current.string);
            Cbuf_AddText(localClientNum, "\n");
            Dvar_SetString((dvar_s *)cl_activeAction, (char *)"");
        }
    }
}

char *__cdecl CL_TimeDemoLogBaseName(const char *mapname)
{
    char *result; // eax
    const char *pos; // [esp+0h] [ebp-10h]
    const char *start; // [esp+4h] [ebp-Ch]
    const char *end; // [esp+8h] [ebp-8h]

    end = 0;
    start = mapname;
    for (pos = mapname; *pos; ++pos)
    {
        if (*pos == 47 || *pos == 92)
        {
            start = pos + 1;
            end = 0;
        }
        else if (*pos == 46)
        {
            end = pos;
        }
    }
    if (!end)
        return (char *)start;
    result = va("%s", start);
    result[end - start] = 0;
    return result;
}

void __cdecl CL_UpdateTimeDemo(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    int Int; // [esp-4h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    clientConnection_t *clc; // [esp+4h] [ebp-8h]
    DWORD currentTime; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    currentTime = Sys_Milliseconds();
    if (!clc->timeDemoLog)
    {
        Int = Dvar_GetInt("r_mode");
        v1 = CL_TimeDemoLogBaseName(LocalClientGlobals->mapname);
        v2 = va("demos/timedemo_%s_mode_%i.csv", v1, Int);
        clc->timeDemoLog = FS_FOpenFileWrite(v2);
    }
    if (clc->timeDemoStart)
    {
        if (clc->timeDemoLog)
            FS_Printf(clc->timeDemoLog, "%i,%i\n", clc->timeDemoFrames, currentTime - clc->timeDemoPrev);
    }
    else
    {
        clc->timeDemoStart = currentTime;
    }
    clc->timeDemoPrev = currentTime;
    ++clc->timeDemoFrames;
    LocalClientGlobals->serverTime = clc->timeDemoBaseTime + 50 * clc->timeDemoFrames;
}

void __cdecl CL_NextDemo(int localClientNum)
{
    char v[1028]; // [esp+0h] [ebp-408h] BYREF

    I_strncpyz(v, (char *)nextdemo->current.integer, 1024);
    Com_DPrintf(14, "CL_NextDemo: %s\n", v);
    if (v[0])
    {
        Dvar_SetString((dvar_s *)nextdemo, (char *)"");
        Cbuf_AddText(localClientNum, v);
        Cbuf_AddText(localClientNum, "\n");
        Cbuf_Execute(localClientNum, 0);
    }
    else
    {
        Com_Error(ERR_DISCONNECT, "Demo is over");
    }
}

void __cdecl CL_DemoCompleted(int localClientNum)
{
    int time; // [esp+10h] [ebp-8h]
    clientConnection_t *clc; // [esp+14h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if (clc->isTimeDemo)
    {
        time = Sys_Milliseconds() - clc->timeDemoStart;
        if (time > 0)
            Com_Printf(
                14,
                "%i frames, %3.1f seconds: %3.1f fps\n",
                clc->timeDemoFrames,
                (double)time / 1000.0,
                (double)clc->timeDemoFrames * 1000.0 / (double)time);
    }
    if (clc->timeDemoLog)
    {
        FS_FCloseFile(clc->timeDemoLog);
        clc->timeDemoLog = 0;
    }
    CL_Disconnect(0);
    CL_NextDemo(0);
}

void __cdecl CL_ReadDemoClientArchive(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-14h]
    unsigned __int8 *archive; // [esp+8h] [ebp-Ch]
    clientConnection_t *clc; // [esp+Ch] [ebp-8h]
    int index; // [esp+10h] [ebp-4h] BYREF

    clc = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (FS_Read((unsigned __int8 *)&index, 4u, clc->demofile) == 4)
    {
        if ((unsigned int)index < 0x100)
        {
            archive = (unsigned __int8 *)&LocalClientGlobals->clientArchive[index];
            FS_Read((unsigned __int8 *)LocalClientGlobals->clientArchive[index].origin, 0xCu, clc->demofile);
            FS_Read(archive + 16, 0xCu, clc->demofile);
            FS_Read(archive + 32, 4u, clc->demofile);
            FS_Read(archive + 28, 4u, clc->demofile);
            FS_Read(archive, 4u, clc->demofile);
            FS_Read(archive + 36, 0xCu, clc->demofile);
            LocalClientGlobals->clientArchiveIndex = index + 1;
        }
        else
        {
            Com_Printf(14, "Demo file was corrupt.\n");
            CL_DemoCompleted(localClientNum);
        }
    }
    else
    {
        CL_DemoCompleted(localClientNum);
    }
}

void __cdecl CL_ReadDemoNetworkPacket(int localClientNum)
{
    unsigned int v1; // edx
    int v2; // eax
    unsigned __int8 *bufData; // [esp+4h] [ebp-3Ch]
    msg_t buf; // [esp+8h] [ebp-38h] BYREF
    clientConnection_t *clc; // [esp+30h] [ebp-10h]
    int s; // [esp+3Ch] [ebp-4h] BYREF

    LargeLocal bufData_large_local(0x20000);
    //LargeLocal::LargeLocal(&bufData_large_local, 0x20000);
    //bufData = LargeLocal::GetBuf(&bufData_large_local);
    bufData = bufData_large_local.GetBuf();
    clc = CL_GetLocalClientConnection(localClientNum);
    if (FS_Read((unsigned __int8 *)&s, 4u, clc->demofile) == 4)
    {
        clc->serverMessageSequence = s;
        MSG_Init(&buf, bufData, 0x20000);
        if (FS_Read((unsigned __int8 *)&buf.cursize, 4u, clc->demofile) != 4 || buf.cursize == -1)
        {
            CL_DemoCompleted(localClientNum);
        }
        else
        {
            if (buf.cursize > buf.maxsize)
                Com_Error(ERR_DROP, "CL_ReadDemoMessage: demoMsglen > MAX_MSGLEN");
            v1 = FS_Read(buf.data, buf.cursize, clc->demofile);
            if (v1 == buf.cursize)
            {
                clc->lastPacketTime = cls.realtime;
                buf.readcount = 0;
                v2 = MSG_ReadLong(&buf);
                clc->reliableAcknowledge = v2;
                if (clc->reliableAcknowledge >= clc->reliableSequence - 128)
                    CL_ParseServerMessage((netsrc_t)localClientNum, &buf);
                else
                    clc->reliableAcknowledge = clc->reliableSequence;
            }
            else
            {
                Com_Printf(14, "Demo file was truncated.\n");
                CL_DemoCompleted(localClientNum);
            }
        }
    }
    else
    {
        CL_DemoCompleted(localClientNum);
    }
}

void __cdecl CL_ReadDemoMessage(netsrc_t localClientNum)
{
    clientConnection_t *clc; // [esp+8h] [ebp-8h]
    unsigned __int8 s; // [esp+Fh] [ebp-1h] BYREF

    clc = CL_GetLocalClientConnection(localClientNum);
    if (clc->demofile)
    {
        if (FS_Read(&s, 1u, clc->demofile) == 1)
        {
            if (s)
            {
                if (s == 1)
                    CL_ReadDemoClientArchive(localClientNum);
            }
            else
            {
                CL_ReadDemoNetworkPacket(localClientNum);
            }
        }
        else
        {
            CL_DemoCompleted(localClientNum);
        }
    }
    else
    {
        CL_DemoCompleted(localClientNum);
    }
}

void __cdecl CL_SetCGameTime(netsrc_t localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    clientConnection_t *clc; // [esp+8h] [ebp-4h]

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
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        clc = CL_GetLocalClientConnection(localClientNum);
    LABEL_16:
        if (!LocalClientGlobals->snap.valid)
            Com_Error(ERR_DROP, "CL_SetCGameTime: !cl->snap.valid");
        if (!sv_paused->current.integer || !cl_paused->current.integer || !com_sv_running->current.enabled)
        {
            if (LocalClientGlobals->snap.serverTime < LocalClientGlobals->oldFrameServerTime)
            {
                if (I_stricmp(cls.servername, "localhost"))
                    Com_Error(ERR_DROP, "cl->snap.serverTime < cl->oldFrameServerTime");
                else
                    CL_FirstSnapshot(localClientNum);
            }
            LocalClientGlobals->oldFrameServerTime = LocalClientGlobals->snap.serverTime;
            if (!clc->demoplaying || !cl_freezeDemo->current.enabled)
            {
                LocalClientGlobals->serverTime = LocalClientGlobals->serverTimeDelta + cls.realtime;
                if (LocalClientGlobals->serverTime < LocalClientGlobals->oldServerTime)
                    LocalClientGlobals->serverTime = LocalClientGlobals->oldServerTime;
                LocalClientGlobals->oldServerTime = LocalClientGlobals->serverTime;
                if (LocalClientGlobals->serverTimeDelta + cls.realtime >= LocalClientGlobals->snap.serverTime - 5)
                {
                    LocalClientGlobals->extrapolatedSnapshot = 1;
                    if (cl_showTimeDelta->current.enabled)
                        Com_Printf(14, "Extrapolating snapshot!\n");
                }
            }
            if (LocalClientGlobals->newSnapshots)
                CL_AdjustTimeDelta(localClientNum);
            if (clc->demoplaying)
            {
                if (clc->isTimeDemo)
                    CL_UpdateTimeDemo(localClientNum);
                do
                {
                    if (LocalClientGlobals->serverTime < LocalClientGlobals->snap.serverTime)
                        break;
                    CL_ReadDemoMessage(localClientNum);
                    if (localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                            1112,
                            0,
                            "%s\n\t(localClientNum) = %i",
                            "(localClientNum == 0)",
                            localClientNum);
                } while (clientUIActives[0].connectionState == CA_ACTIVE);
            }
        }
        return;
    }
    if (clientUIActives[0].connectionState != CA_PRIMED)
        return;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    if (clc->demoplaying)
    {
        if (!clc->firstDemoFrameSkipped)
        {
            clc->firstDemoFrameSkipped = 1;
            return;
        }
        CL_ReadDemoMessage(localClientNum);
    }
    if (LocalClientGlobals->newSnapshots)
    {
        LocalClientGlobals->newSnapshots = 0;
        CL_FirstSnapshot(localClientNum);
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_ACTIVE)
        goto LABEL_16;
}

void __cdecl CL_AdjustTimeDelta(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-14h]
    int idealDelta; // [esp+8h] [ebp-10h]
    unsigned int snapInterval; // [esp+Ch] [ebp-Ch]
    int deltaCorrectionMagnitude; // [esp+10h] [ebp-8h]
    int oldDelta; // [esp+14h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->newSnapshots = 0;
    if (!CL_GetLocalClientConnection(localClientNum)->demoplaying)
    {
        snapInterval = LocalClientGlobals->snap.serverTime - LocalClientGlobals->oldSnapServerTime;
        oldDelta = LocalClientGlobals->serverTimeDelta;
        idealDelta = LocalClientGlobals->snap.serverTime - cls.realtime - snapInterval - 5;
        deltaCorrectionMagnitude = idealDelta - oldDelta;
        if (idealDelta - oldDelta <= 0)
        {
            deltaCorrectionMagnitude = oldDelta - idealDelta;
        }
        else if (snapInterval <= 0x1F4)
        {
            deltaCorrectionMagnitude -= snapInterval;
            if (deltaCorrectionMagnitude < 0)
                deltaCorrectionMagnitude = 0;
        }
        if (deltaCorrectionMagnitude <= 500)
        {
            if (deltaCorrectionMagnitude <= 100)
            {
                if (com_timescaleValue == 1.0)
                {
                    if (LocalClientGlobals->extrapolatedSnapshot)
                    {
                        LocalClientGlobals->extrapolatedSnapshot = 0;
                        LocalClientGlobals->serverTimeDelta -= 2;
                    }
                    else if (idealDelta <= LocalClientGlobals->serverTimeDelta)
                    {
                        if (idealDelta < LocalClientGlobals->serverTimeDelta)
                            --LocalClientGlobals->serverTimeDelta;
                    }
                    else
                    {
                        ++LocalClientGlobals->serverTimeDelta;
                    }
                }
            }
            else
            {
                if (cl_showTimeDelta->current.enabled)
                    Com_Printf(14, "<FAST> ");
                LocalClientGlobals->serverTimeDelta = (idealDelta + LocalClientGlobals->serverTimeDelta) >> 1;
            }
        }
        else
        {
            Com_PrintWarning(
                14,
                "Cl_AdjustTimeDelta RESET: snap is %i, last snap was %i, walltime is %i, current delta is %i, old server time was"
                " %i, server time is %i\n",
                LocalClientGlobals->snap.serverTime,
                LocalClientGlobals->oldSnapServerTime,
                cls.realtime,
                LocalClientGlobals->serverTimeDelta,
                LocalClientGlobals->oldServerTime,
                LocalClientGlobals->serverTime);
            LocalClientGlobals->serverTimeDelta = LocalClientGlobals->snap.serverTime - cls.realtime - 5;
            LocalClientGlobals->oldServerTime = LocalClientGlobals->snap.serverTime;
            LocalClientGlobals->serverTime = LocalClientGlobals->snap.serverTime;
            if (cl_showTimeDelta->current.enabled)
                Com_Printf(14, "<RESET> ");
        }
        if (LocalClientGlobals->serverTimeDelta != oldDelta)
            Con_TimeNudged(localClientNum, LocalClientGlobals->serverTimeDelta - oldDelta);
        if (cl_showTimeDelta->current.enabled)
        {
            Com_Printf(
                14,
                "client time: %i, server time: %i\n",
                LocalClientGlobals->serverTimeDelta + cls.realtime,
                LocalClientGlobals->snap.serverTime);
            Com_Printf(14, "ideal delta: %i, current delta: %i\n", idealDelta, LocalClientGlobals->serverTimeDelta);
        }
    }
}

void __cdecl CL_SetADS(int localClientNum, bool ads)
{
    CL_GetLocalClientGlobals(localClientNum)->usingAds = ads;
}

void __cdecl CL_DrawString(int x, int y, char *pszString, int bShadow, int iCharHeight)
{
    float v5; // [esp+0h] [ebp-20h]
    float v6; // [esp+4h] [ebp-1Ch]
    float charHeight; // [esp+18h] [ebp-8h]

    charHeight = (float)iCharHeight;
    v6 = (float)y;
    v5 = (float)x;
    CG_DrawStringExt(&scrPlaceFull, v5, v6, pszString, 0, 0, bShadow, charHeight);
}

void __cdecl CL_DrawRect(int x, int y, int width, int height, const float *color)
{
    float v5; // [esp+0h] [ebp-30h]
    float v6; // [esp+4h] [ebp-2Ch]
    float w; // [esp+8h] [ebp-28h]
    float h; // [esp+Ch] [ebp-24h]

    h = (float)height;
    w = (float)width;
    v6 = (float)y;
    v5 = (float)x;
    CL_DrawStretchPic(&scrPlaceFull, v5, v6, w, h, 1, 1, 0.0, 0.0, 0.0, 0.0, color, cls.whiteMaterial);
}

void __cdecl CL_ArchiveClientState(int localClientNum, MemoryFile *memFile)
{
    CG_ArchiveState(localClientNum, memFile);
    FX_Archive(localClientNum, memFile);
    R_ArchiveFogState(memFile);
}

void __cdecl CL_LookupColor(int localClientNum, unsigned __int8 c, float *color)
{
    float *v3; // [esp+4h] [ebp-18h]
    float *v4; // [esp+8h] [ebp-14h]
    float *v5; // [esp+Ch] [ebp-10h]
    team_t team; // [esp+10h] [ebp-Ch]
    unsigned int index; // [esp+18h] [ebp-4h]

    index = ColorIndex(c);
    if (index >= 8)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        team = cgArray[0].bgs.clientinfo[cgArray[0].clientNum].team;
        if (team != TEAM_AXIS && team != TEAM_ALLIES)
            team = TEAM_ALLIES;
        if (c == 56)
        {
            if (team == TEAM_ALLIES)
                v4 = color_allies;
            else
                v4 = color_axis;
            *color = *v4;
            color[1] = v4[1];
            color[2] = v4[2];
            color[3] = v4[3];
        }
        else if (c == 57)
        {
            if (team == TEAM_ALLIES)
                v3 = color_axis;
            else
                v3 = color_allies;
            *color = *v3;
            color[1] = v3[1];
            color[2] = v3[2];
            color[3] = v3[3];
        }
        else
        {
            *color = 1.0;
            color[1] = 1.0;
            color[2] = 1.0;
            color[3] = 1.0;
        }
    }
    else
    {
        v5 = (float *)g_color_table[index];
        *color = *v5;
        color[1] = v5[1];
        color[2] = v5[2];
        color[3] = v5[3];
    }
}

void __cdecl CL_UpdateColor(int localClientNum)
{
    team_t team; // [esp+0h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (cgArray)
        {
            team = cgArray[0].bgs.clientinfo[cgArray[0].clientNum].team;
            if (team != TEAM_AXIS && team != TEAM_ALLIES)
                team = TEAM_ALLIES;
            CL_UpdateColorInternal("g_TeamColor_Allies", color_allies);
            CL_UpdateColorInternal("g_TeamColor_Axis", color_axis);
            R_UpdateTeamColors(team, color_allies, color_axis);
        }
    }
}

void __cdecl CL_UpdateColorInternal(const char *var_name, float *color)
{
    Dvar_GetUnpackedColorByName(var_name, color);
    color[3] = 1.0;
}

BOOL __cdecl CL_IsCgameInitialized(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return clientUIActives[0].cgameInitialized;
}

