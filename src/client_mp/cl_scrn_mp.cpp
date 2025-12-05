#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "client_mp.h"

#include <client/client.h>
#include <gfx_d3d/r_rendercmds.h>
#include <win32/win_local.h>
#include <qcommon/threads.h>
#include <sound/snd_public.h>
#include <gfx_d3d/r_dvars.h>
#include <gfx_d3d/r_reflection_probe.h>
#include <cgame_mp/cg_local_mp.h>
#include <universal/profile.h>
#include <devgui/devgui.h>
#include <universal/com_files.h>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_screenshot.h>
#include <qcommon/qcommon.h>

int scr_initialized;
BOOL updateScreenCalled;

void __cdecl SCR_DrawSmallStringExt(int x, int y, char *string, const float *setColor)
{
    float yAdj; // [esp+28h] [ebp-8h]
    float xAdj; // [esp+2Ch] [ebp-4h]

    xAdj = (float)x;
    yAdj = (double)R_TextHeight(cls.consoleFont) + (double)y;
    R_AddCmdDrawText(string, 0x7FFFFFFF, cls.consoleFont, xAdj, yAdj, 1.0, 1.0, 0.0, setColor, 0);
}

void __cdecl SCR_Init()
{
    scr_initialized = 1;
}

int s_lastUpdateScreenTime;
static char __cdecl SCR_ShouldSkipUpdateScreen()
{
    connstate_t clcState; // [esp+0h] [ebp-Ch]
    DWORD timeNow; // [esp+4h] [ebp-8h]

    clcState = clientUIActives[0].connectionState;
    if (clientUIActives[0].connectionState >= CA_ACTIVE)
        return 0;
    timeNow = Sys_Milliseconds();
    if ((int)(timeNow - s_lastUpdateScreenTime) < (clcState < CA_LOADING ? 16 : 33))
        return 1;
    s_lastUpdateScreenTime = timeNow;
    return 0;
}

float __cdecl CL_GetMenuBlurRadius(int localClientNum)
{
    if (Key_IsCatcherActive(localClientNum, 16) && cls.uiStarted)
        return UI_GetBlurRadius(localClientNum);
    else
        return 0.0f;
}

void __cdecl SCR_UpdateScreen()
{
    if (!updateScreenCalled && !SCR_ShouldSkipUpdateScreen())
    {
        PROF_SCOPED("SCR_UpdateScreen");
        if (clientUIActives[0].connectionState == CA_LOADING)
            Sys_LoadingKeepAlive();
        if (scr_initialized && !com_errorEntered)
        {
            updateScreenCalled = 1;
            SCR_UpdateFrame();
            updateScreenCalled = 0;
        }
    }
}

void SCR_UpdateFrame()
{
    int refreshedUI; // [esp+8h] [ebp-4h]

    iassert(Sys_IsMainThread() || Sys_IsRenderThread());

    R_BeginFrame();
    SND_InitFXSounds();
    refreshedUI = CL_CGameRendering(0);
    if (Sys_IsMainThread() && !refreshedUI)
        CL_UpdateSound();
    SCR_DrawScreenField(0, refreshedUI);
    CL_DrawScreen(0);
    R_EndFrame();
    R_IssueRenderCommands(0xFFFFFFFF);
    if (r_reflectionProbeGenerate->current.enabled && refreshedUI && CL_GetLocalClientGlobals(0)->serverTime > 1000)
        R_BspGenerateReflections();
}

int __cdecl CL_CGameRendering(int localClientNum)
{
    BOOL demType; // eax
    BOOL v3; // [esp-4h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState != CA_ACTIVE)
        return 0;
    R_BeginClientCmdList2D();
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    v3 = UI_IsFullscreen(localClientNum) == 0;
    demType = CL_GetDemoType();
    if (CG_DrawActiveFrame(localClientNum, LocalClientGlobals->serverTime, (DemoType)demType, CUBEMAPSHOT_NONE, 0, v3))
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                1063,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if ((clientUIActives[0].keyCatchers & 0x10) != 0)
        {
            UI_UpdateTime(localClientNum, cls.realtime);
            UI_Refresh(localClientNum);
        }
        R_AddCmdEndOfList();
        return 1;
    }
    else
    {
        CL_SendCmd(localClientNum);
        return 0;
    }
}

DemoType __cdecl CL_GetDemoType()
{
    return (DemoType)(CL_GetLocalClientConnection(0)->demoplaying != 0);
}

void __cdecl CL_DrawScreen(int localClientNum)
{
    if (cls.rendererStarted)
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
        {
            PROF_SCOPED("DebugOverlays");
            CG_DrawFullScreenDebugOverlays(localClientNum);
            CG_DrawUpperRightDebugInfo(localClientNum);
        }
        R_AddCmdDrawProfile();
        Con_DrawConsole(localClientNum);
        DevGui_Draw(localClientNum);
    }
}

void __cdecl SCR_DrawScreenField(int localClientNum, int refreshedUI)
{
    connstate_t clcState; // [esp+8h] [ebp-4h]

    R_BeginSharedCmdList();
    R_AddCmdProjectionSet2D();
    if (!cls.uiStarted)
    {
        SCR_ClearScreen();
        return;
    }
    UI_UpdateTime(localClientNum, cls.realtime);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clcState = clientUIActives[0].connectionState;
    if (!UI_IsFullscreen(localClientNum))
    {
        switch (clcState)
        {
        case CA_DISCONNECTED:
            SCR_ClearScreen();
            SND_StopSounds(SND_STOP_ALL);
            if (Sys_IsMainThread())
            {
                if (!CL_AllLocalClientsDisconnected())
                    MyAssertHandler(".\\client_mp\\cl_scrn_mp.cpp", 281, 0, "%s", "CL_AllLocalClientsDisconnected()");
                if (cls.wwwDlInProgress)
                {
                    UI_Refresh(localClientNum);
                    UI_DrawConnectScreen(localClientNum);
                }
            }
            else if (CL_AnyLocalClientChallenging())
            {
                MyAssertHandler(".\\client_mp\\cl_scrn_mp.cpp", 278, 0, "%s", "!CL_AnyLocalClientChallenging()");
            }
            break;
        case CA_CINEMATIC:
            SCR_ClearScreen();
            SCR_DrawCinematic(localClientNum);
            break;
        case CA_LOGO:
            SCR_ClearScreen();
            CL_DrawLogo(localClientNum);
            break;
        case CA_CONNECTING:
        case CA_CHALLENGING:
        case CA_CONNECTED:
        case CA_SENDINGSTATS:
        case CA_LOADING:
        case CA_PRIMED:
            SCR_ClearScreen();
            UI_Refresh(localClientNum);
            UI_DrawConnectScreen(localClientNum);
            refreshedUI = 1;
            break;
        case CA_ACTIVE:
            SCR_DrawDemoRecording();
            break;
        default:
            goto LABEL_23;
        }
        goto LABEL_26;
    }
    if (clcState < CA_DISCONNECTED)
        goto LABEL_23;
    if (clcState <= CA_PRIMED)
    {
        SCR_ClearScreen();
    }
    else if (clcState != CA_ACTIVE)
    {
    LABEL_23:
        Com_Error(ERR_FATAL, "SCR_DrawScreenField: bad clcState");
    }
LABEL_26:
    if (!refreshedUI && Key_IsCatcherActive(localClientNum, 16))
        UI_Refresh(localClientNum);
    if (net_showprofile->current.integer)
        Net_DisplayProfile(localClientNum);
}

void SCR_DrawDemoRecording()
{
    signed int pos; // [esp+1Ch] [ebp-430h]
    float xScale; // [esp+20h] [ebp-42Ch] BYREF
    char string[1028]; // [esp+24h] [ebp-428h] BYREF
    const clientConnection_t *clc; // [esp+42Ch] [ebp-20h]
    float color[4]; // [esp+430h] [ebp-1Ch] BYREF
    float x; // [esp+440h] [ebp-Ch] BYREF
    float y; // [esp+444h] [ebp-8h] BYREF
    float yScale; // [esp+448h] [ebp-4h] BYREF

    clc = CL_GetLocalClientConnection(0);
    if (clc->demorecording)
    {
        pos = FS_FTell(clc->demofile);
        snprintf(string, ARRAYSIZE(string), "RECORDING %s: %ik", clc->demoName, pos / 1024);
        CL_LookupColor(0, 0x37u, color);
        x = 5.0f;
        y = 479.0f;
        xScale = R_NormalizedTextScale(cls.consoleFont, 0.33333334f);
        yScale = xScale;
        ScrPlace_ApplyRect(scrPlaceView, &x, &y, &xScale, &yScale, 1, 1);
        R_AddCmdDrawText(string, 0x7FFFFFFF, cls.consoleFont, x, y, xScale, yScale, 0.0, color, 0);
    }
}

void SCR_ClearScreen()
{
    R_AddCmdClearScreen(1, colorBlack, 1.0, 0);
}

void __cdecl SCR_UpdateLoadScreen()
{
    if (!IsFastFileLoad())
        SCR_UpdateScreen();
}

const char *WeaponStateNames_65[27] =
{
  "WEAPON_READY",
  "WEAPON_RAISING",
  "WEAPON_RAISING_ALTSWITCH",
  "WEAPON_DROPPING",
  "WEAPON_DROPPING_QUICK",
  "WEAPON_FIRING",
  "WEAPON_RECHAMBERING",
  "WEAPON_RELOADING",
  "WEAPON_RELOADING_INTERUPT",
  "WEAPON_RELOAD_START",
  "WEAPON_RELOAD_START_INTERUPT",
  "WEAPON_RELOAD_END",
  "WEAPON_MELEE_INIT",
  "WEAPON_MELEE_FIRE",
  "WEAPON_MELEE_END",
  "WEAPON_OFFHAND_INIT",
  "WEAPON_OFFHAND_PREPARE",
  "WEAPON_OFFHAND_HOLD",
  "WEAPON_OFFHAND_START",
  "WEAPON_OFFHAND",
  "WEAPON_OFFHAND_END",
  "WEAPON_DETONATING",
  "WEAPON_SPRINT_RAISE",
  "WEAPON_SPRINT_LOOP",
  "WEAPON_SPRINT_DROP",
  "WEAPON_NIGHTVISION_WEAR",
  "WEAPON_NIGHTVISION_REMOVE"
}; // idb

void __cdecl CL_CubemapShot_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    bool demType; // eax
    char *v9; // eax
    CubemapShot v10; // [esp-4h] [ebp-98h]
    float v11; // [esp+0h] [ebp-94h]
    float v12; // [esp+4h] [ebp-90h]
    char v13; // [esp+Bh] [ebp-89h]
    char *v14; // [esp+10h] [ebp-84h]
    const char *v15; // [esp+14h] [ebp-80h]
    clientActive_t *LocalClientGlobals; // [esp+28h] [ebp-6Ch]
    float rgb[3]; // [esp+2Ch] [ebp-68h] BYREF
    int shot; // [esp+38h] [ebp-5Ch]
    int localClientNum; // [esp+3Ch] [ebp-58h]
    int size; // [esp+40h] [ebp-54h]
    char szBaseName[64]; // [esp+44h] [ebp-50h] BYREF
    float n0; // [esp+88h] [ebp-Ch]
    bool isLightingShot; // [esp+8Fh] [ebp-5h]
    float n1; // [esp+90h] [ebp-4h]

    if (!CL_IsCgameInitialized(0))
    {
        Com_Printf(0, "must be in a map to use this command\n");
        return;
    }
    if (Cmd_Argc() < 3 || strlen(Cmd_Argv(2)) > 0x28)
    {
        CL_CubemapShotUsage();
        return;
    }
    v15 = Cmd_Argv(2);
    v14 = szBaseName;
    do
    {
        v13 = *v15;
        *v14++ = *v15++;
    } while (v13);
    v0 = Cmd_Argv(1);
    size = atoi(v0);
    if (size < 4 || size > 1024 || (size & (size - 1)) != 0)
        goto LABEL_23;
    rgb[0] = 0.0f;
    rgb[1] = 0.0f;
    rgb[2] = 0.0f;
    isLightingShot = 0;
    n0 = 1.0f;
    n1 = 1.3329999f;
    if (Cmd_Argc() == 7)
    {
        v1 = Cmd_Argv(3);
        if (!I_stricmp(v1, "lighting"))
        {
            v2 = Cmd_Argv(4);
            rgb[0] = atof(v2);
            v3 = Cmd_Argv(5);
            rgb[1] = atof(v3);
            v4 = Cmd_Argv(6);
            rgb[2] = atof(v4);
            isLightingShot = 1;
            goto LABEL_24;
        }
    LABEL_23:
        CL_CubemapShotUsage();
        return;
    }
    if (Cmd_Argc() == 6)
    {
        v5 = Cmd_Argv(3);
        if (!I_stricmp(v5, "fresnel"))
        {
            v6 = Cmd_Argv(4);
            n0 = atof(v6);
            v7 = Cmd_Argv(5);
            n1 = atof(v7);
            if (n0 >= 1.0 && n1 >= 1.0)
                goto LABEL_24;
        }
        goto LABEL_23;
    }
    if (Cmd_Argc() != 3)
        goto LABEL_23;
LABEL_24:
    R_SyncRenderThread();
    localClientNum = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    for (shot = 1; shot < 7; ++shot)
    {
        R_BeginCubemapShot(size, 1);
        R_BeginFrame();
        R_BeginSharedCmdList();
        R_ClearClientCmdList2D();
        demType = CL_GetDemoType();
        CG_DrawActiveFrame(localClientNum, LocalClientGlobals->serverTime, (DemoType)demType, (CubemapShot)shot, size, 1);
        R_EndFrame();
        R_IssueRenderCommands(0xFFFFFFFF);
        R_EndCubemapShot((CubemapShot)shot);
    }
    if (isLightingShot)
        R_LightingFromCubemapShots(rgb);
    for (shot = 1; shot < 7; ++shot)
    {
        v12 = n1;
        v11 = n0;
        v10 = (CubemapShot)shot;
        v9 = va("env/%s%s.tga", szBaseName, WeaponStateNames_65[shot + 22]);
        R_SaveCubemapShot(v9, v10, v11, v12);
    }
}

void CL_CubemapShotUsage()
{
    Com_Printf(0, "Syntax: cubemapShot size basefilename [lighting r g b | fresnel n0 n1]\n");
    Com_Printf(0, "* size must be a power of 2 that is at least 4 and not more than 1024.\n");
    Com_Printf(0, "* screenshots will be written to 'env/basefilename_*.tga'\n");
    Com_Printf(0, "* basefilename must not exceed %i chars\n", 40);
    Com_Printf(0, "* If 'lighting' is specified, a diffuse environment-based lighting cubemap is generated.\n");
    Com_Printf(0, "  This takes exponentially longer to make larger image sizes.\n");
    Com_Printf(0, "  16 is a good iteration size.  32 is a good final image size.\n");
    Com_Printf(0, "* If 'fresnel' is specified, the alpha channel of the cubemap contains the reflection factor.\n");
    Com_Printf(0, "  n0 and n1 are the index of refraction of the 'air' and 'water' surfaces, respectively.\n");
    Com_Printf(0, "  The index of refraction must always be 1 or greater.\n");
    Com_Printf(0, "  This is always calculated, and defaults to air-water interface (n0 = 1, n1 = 1.333).\n");
}

