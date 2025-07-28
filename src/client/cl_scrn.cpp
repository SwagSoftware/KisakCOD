#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cl_scrn.h"
#include <gfx_d3d/r_font.h>
#include <gfx_d3d/r_rendercmds.h>
#include "client.h"
#include <ui/ui.h>
#include <game/g_local.h>
#include "cl_demo.h"
#include <cgame/cg_view.h>
#include <devgui/devgui.h>
#include <qcommon/threads.h>
#include <win32/win_local.h>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_screenshot.h>

const char *WeaponStateNames_51[27] =
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
};

const char *szShotName[6] =
{ "_rt", "_lf", "_bk", "_ft", "_up", "_dn" };


int scr_initialized;
bool updateScreenCalled;

void __cdecl TRACK_cl_srcn()
{
    ;
}

void __cdecl SCR_DrawSmallStringExt(unsigned int x, int y, const char *string, const float *setColor)
{
    R_AddCmdDrawText(string, 0x7FFFFFFF, cls.consoleFont, x, y, 1.0f, 1.0f, 0.0f, setColor, 0);
}

void __cdecl SCR_Init()
{
    scr_initialized = 1;
}

bool __cdecl CL_IsCGameRendering()
{
    return cls.uiStarted && !UI_IsFullscreen() && clientUIActives[0].connectionState == CA_ACTIVE;
}

DemoType CL_GetDemoType()
{
    if (G_DemoPlaying())
        return DEMO_TYPE_SERVER;
    else
        return (DemoType)CL_DemoPlaying();
}

int __cdecl CL_CGameRendering()
{
    int animFrametime; // r31
    DemoType DemoType; // r3

    if (clientUIActives[0].connectionState != CA_ACTIVE)
        return 0;
    if (UI_IsFullscreen())
        return 0;
    R_BeginClientCmdList2D();
    animFrametime = cls.animFrametime;
    DemoType = CL_GetDemoType();
    if (!CG_DrawActiveFrame(0, clients[0].serverTime, DemoType, CUBEMAPSHOT_NONE, 0, animFrametime))
        return 0;
    if ((clientUIActives[0].keyCatchers & 0x10) != 0)
        UI_Refresh();
    R_AddCmdEndOfList();
    return 1;
}

void CL_DrawScreen()
{
    if (clientUIActives[0].connectionState == CA_ACTIVE)
    {
        //Profile_Begin(349);
        CG_DrawFullScreenDebugOverlays(0);
        //Profile_EndInternal(0);
    }
    R_AddCmdDrawProfile();
    Con_DrawConsole(0);
    DevGui_Draw(0);
}

void __cdecl SCR_ClearScreen(int a1, int a2, unsigned __int8 a3)
{
    R_AddCmdClearScreen(15, colorBlack, 0.0, a3);
}

void __cdecl SCR_DrawScreenField(int refreshedUI)
{
    unsigned __int8 v2; // r5
    connstate_t connectionState; // r31
    Material *v4; // r4
    const float *v5; // r3

    R_BeginSharedCmdList();
    R_AddCmdProjectionSet2D();
    if (!cls.uiStarted
        || (connectionState = clientUIActives[0].connectionState, clientUIActives[0].connectionState == CA_MAP_RESTART))
    {
    LABEL_2:
        R_AddCmdClearScreen(15, colorBlack, 0.0, v2);
    }
    else
    {
        UI_UpdateTime(cls.realtime);
        if (!UI_IsFullscreen())
        {
            switch (connectionState)
            {
            case CA_DISCONNECTED:
                goto LABEL_2;
            case CA_CINEMATIC:
                R_AddCmdClearScreen(15, colorBlack, 0.0, v2);
                SCR_DrawCinematic(0);
                goto LABEL_12;
            case CA_LOGO:
                R_AddCmdClearScreen(15, colorBlack, 0.0, v2);
                CL_DrawLogo();
                goto LABEL_12;
            case CA_LOADING:
                R_AddCmdClearScreen(15, colorBlack, 0.0, v2);
                goto LABEL_14;
            case CA_ACTIVE:
                goto LABEL_12;
            default:
                goto LABEL_11;
            }
        }
        switch (connectionState)
        {
        case CA_DISCONNECTED:
        case CA_LOGO:
        case CA_LOADING:
            R_AddCmdClearScreen(15, colorBlack, 0.0, v2);
            break;
        case CA_CINEMATIC:
        case CA_ACTIVE:
            break;
        default:
        LABEL_11:
            Com_Error(ERR_FATAL, "SCR_DrawScreenField: bad clcState");
            break;
        }
    LABEL_12:
        if (!refreshedUI && Key_IsCatcherActive(0, 16))
            LABEL_14 :
            UI_Refresh();
    }
}

float __cdecl CL_GetMenuBlurRadius(int localClientNum)
{
    double BlurRadius; // fp1

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_scrn.cpp",
            271,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (Key_IsCatcherActive(0, 16) && cls.uiStarted && clientUIActives[0].connectionState != CA_CINEMATIC)
        BlurRadius = UI_GetBlurRadius();
    else
        BlurRadius = 0.0;
    return *((float *)&BlurRadius + 1);
}

void __cdecl SCR_UpdateRumble()
{
    // KISAKTODO
    //int v0; // r3
    //
    //if (!cl_paused)
    //    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_scrn.cpp", 284, 0, "%s", "cl_paused");
    //v0 = CL_ControllerIndexFromClientNum(0);
    //if (clientUIActives[0].connectionState != CA_ACTIVE || cl_paused->current.integer)
    //    GPad_StopRumbles(v0);
    //else
    //    GPad_UpdateRumbles(v0);
}

void SCR_UpdateFrame()
{
    int v0; // r31

    iassert(Sys_IsMainThread() || Sys_IsRenderThread());
    //Profile_Begin(18);
    //Profile_Begin(19);
    R_BeginFrame();
    //Profile_EndInternal(0);
    SND_InitFXSounds();
    //Profile_Begin(20);
    v0 = 0;
    if (Sys_IsMainThread())
    {
        v0 = CL_CGameRendering();
        if (!v0)
            CL_UpdateSound();
    }
    SCR_DrawScreenField(v0);
    if (clientUIActives[0].connectionState == CA_ACTIVE)
    {
        //Profile_Begin(349);
        CG_DrawFullScreenDebugOverlays(0);
        //Profile_EndInternal(0);
    }
    R_AddCmdDrawProfile();
    Con_DrawConsole(0);
    DevGui_Draw(0);
    //Profile_EndInternal(0);
    //Profile_Begin(21);
    R_EndFrame();
    R_IssueRenderCommands(0xFFFFFFFF);
    //Profile_EndInternal(0);
#ifdef KISAK_XBOX
    if (R_SkinCacheReachedThreshold() && g_allowRemoveCorpse)
    {
        CL_AddReliableCommand(0, "removecorpse");
        g_allowRemoveCorpse = 0;
    }
#endif
    //Profile_EndInternal(0);
}

void __cdecl SCR_UpdateScreen()
{
    if (!updateScreenCalled)
    {
        //Profile_Begin(34);
        if (clientUIActives[0].connectionState == CA_LOADING)
            Sys_LoadingKeepAlive();
        if (scr_initialized)
        {
            if (!com_errorEntered)
            {
                updateScreenCalled = 1;
                SCR_UpdateFrame();
                updateScreenCalled = 0;
            }
        }
        //Profile_EndInternal(0);
    }
}

void __cdecl SCR_UpdateLoadScreen()
{
    ;
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

void __cdecl CL_CubemapShot_f()
{
    const char *v0; // r3
    const char *v1; // r11
    const char *v3; // r3
    char *v4; // r11
    int v5; // r10
    const char *v6; // r3
    int v7; // r3
    int v8; // r28
    char v9; // r27
    double v10; // fp29
    double v11; // fp31
    const char *v12; // r3
    const char *v13; // r3
    long double v14; // fp2
    const char *v15; // r3
    long double v16; // fp2
    const char *v17; // r3
    long double v18; // fp2
    const char *v19; // r3
    const char *v20; // r3
    long double v21; // fp2
    const char *v22; // r3
    long double v23; // fp2
    unsigned int displayWidth; // r11
    CubemapShot i; // r31
    DemoType DemoType; // r3
    CubemapShot v27; // r30
    const char **v28; // r31
    const char *v29; // r3
    float v30; // [sp+50h] [-A0h] BYREF
    float v31; // [sp+54h] [-9Ch]
    float v32; // [sp+58h] [-98h]
    char v33[72]; // [sp+60h] [-90h] BYREF

    if (!CL_IsCgameInitialized(0))
    {
        Com_Printf(0, "must be in a map to use this command\n");
        return;
    }
    if (Cmd_Argc() < 3)
        goto LABEL_18;
    v0 = Cmd_Argv(2);
    v1 = v0;
    while (*(unsigned __int8 *)v0++)
        ;
    if ((unsigned int)(v0 - v1 - 1) > 0x28)
        goto LABEL_18;
    v3 = Cmd_Argv(2);
    v4 = v33;
    do
    {
        v5 = *(unsigned __int8 *)v3++;
        *v4++ = v5;
    } while (v5);
    v6 = Cmd_Argv(1);
    v7 = atol(v6);
    v8 = v7;
    if ((unsigned int)(v7 - 4) > 0x3FC || ((v7 - 1) & v7) != 0)
        goto LABEL_18;
    v9 = 0;
    v30 = 0.0;
    v31 = 0.0;
    v32 = 0.0;
    v10 = 1.0;
    v11 = 1.3329999;
    if (Cmd_Argc() != 7)
    {
        if (Cmd_Argc() == 6)
        {
            v19 = Cmd_Argv(3);
            if (!I_stricmp(v19, "fresnel"))
            {
                v20 = Cmd_Argv(4);
                v21 = atof(v20);
                v10 = (float)*(double *)&v21;
                v22 = Cmd_Argv(5);
                v23 = atof(v22);
                v11 = (float)*(double *)&v23;
                if (v10 >= 1.0 && v11 >= 1.0)
                    goto LABEL_20;
            }
        }
        else if (Cmd_Argc() == 3)
        {
            goto LABEL_20;
        }
    LABEL_18:
        CL_CubemapShotUsage();
        return;
    }
    v12 = Cmd_Argv(3);
    if (I_stricmp(v12, "lighting"))
        goto LABEL_18;
    v13 = Cmd_Argv(4);
    v14 = atof(v13);
    v30 = *(double *)&v14;
    v15 = Cmd_Argv(5);
    v16 = atof(v15);
    v31 = *(double *)&v16;
    v17 = Cmd_Argv(6);
    v18 = atof(v17);
    v32 = *(double *)&v18;
    v9 = 1;
LABEL_20:
    displayWidth = cls.vidConfig.displayWidth;
    if ((int)cls.vidConfig.displayHeight < (int)cls.vidConfig.displayWidth)
        displayWidth = cls.vidConfig.displayHeight;
    if (v8 <= (int)(displayWidth - 2))
    {
        R_SyncRenderThread();
        for (i = CUBEMAPSHOT_RIGHT; i < CUBEMAPSHOT_COUNT; ++i)
        {
            R_BeginCubemapShot(v8, 1);
            R_BeginFrame();
            R_BeginSharedCmdList();
            R_ClearClientCmdList2D();
            DemoType = CL_GetDemoType();
            CG_DrawActiveFrame(0, clients[0].serverTime, DemoType, i, v8, 0);
            R_EndFrame();
            R_IssueRenderCommands(0xFFFFFFFF);
            R_EndCubemapShot(i);
        }
        if (v9)
            R_LightingFromCubemapShots(&v30);
        v27 = CUBEMAPSHOT_RIGHT;
        v28 = (const char **)&WeaponStateNames_51[23];
        do
        {
            v29 = va("env/%s%s.tga", v33, *v28);
            R_SaveCubemapShot((char*)v29, v27, v10, v11);
            ++v28;
            ++v27;
        } while ((int)v28 < (int)&szShotName[2]);
    }
    else
    {
        Com_Printf(
            0,
            "The cubemapshot size may not exceed %i for this resolution.  Try reducing the cubemapshot size or increasing your "
            "screen resolution.\n",
            displayWidth - 2);
    }
}

