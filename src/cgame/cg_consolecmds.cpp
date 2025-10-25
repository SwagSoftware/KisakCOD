#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_consolecmds.h"

#include <server/server.h>
#include <qcommon/cmd.h>
#include "cg_draw.h"
#include "cg_actors.h"
#include "cg_main.h"
#include "cg_modelpreviewer.h"
#include <stringed/stringed_hooks.h>
#include "cg_view.h"

int __cdecl CG_CheatsOK(const char *cmdName)
{
    if (sv_cheats->current.enabled)
        return 1;
    Com_Printf(16, "%s is cheat protected.\n", cmdName);
    return 0;
}

void CG_Viewpos_f()
{
    if (cgArray[0].nextSnap)
    {
        if (cmd_args.nesting >= 8u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
                191,
                0,
                "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                cmd_args.nesting,
                8);
        Com_Printf(
            0,
            "(%.0f %.0f %.0f) : %.0f %.0f\n",
            cgArray[0].refdef.vieworg[0],
            cgArray[0].refdef.vieworg[1],
            cgArray[0].refdef.vieworg[2],
            cgArray[0].refdefViewAngles[0],
            cgArray[0].refdefViewAngles[1]
        );
    }
}

void CG_ScoresUp_f()
{
    if (cgArray[0].nextSnap)
    {
        if (cgArray[0].showScores)
        {
            cgArray[0].showScores = 0;
            cgArray[0].scoreFadeTime = cgArray[0].time;
        }
    }
}

void CG_ScoresDown_f()
{
    if (cgArray[0].nextSnap)
    {
        if (!cgArray[0].showScores)
        {
            cgArray[0].showScores = 1;
            cgArray[0].scoreFadeTime = cgArray[0].time;
        }
    }
}

void CG_Fade_f()
{
    int r; // r30
    int g; // r29
    int b; // r28
    int a; // r27
    int duration; // r26
    int LocalClientTime; // r3

    if (cgArray[0].nextSnap)
    {
        if (Cmd_Argc() >= 6)
        {
            r = atol(Cmd_Argv(1));
            g = atol(Cmd_Argv(2));
            b = atol(Cmd_Argv(3));
            a = atol(Cmd_Argv(4));
            duration = 1000 * atol(Cmd_Argv(5));
            LocalClientTime = CG_GetLocalClientTime(Cmd_LocalClientNum());
            CG_Fade(Cmd_LocalClientNum(), r, g, b, a, LocalClientTime, duration);
        }
    }
}

void CG_ShellShock_f()
{
    int v0; // r3
    const char *v1; // r3
    const char *v2; // r3
    long double v3; // fp2
    long double v4; // fp2
    shellshock_parms_t *ShellshockParms; // r3
    int v6; // [sp+50h] [-20h]

    if (cgArray[0].nextSnap)
    {
        v0 = Cmd_Argc();
        if (v0 == 2)
            goto LABEL_6;
        if (v0 != 3)
        {
            Com_Printf(0, "USAGE: cg_shellshock <duration> <filename?>\n");
            return;
        }
        v1 = Cmd_Argv(2);
        if (BG_LoadShellShockDvars(v1))
        {
        LABEL_6:
            v2 = Cmd_Argv(1);
            v3 = atof(v2);
            *(double *)&v3 = (float)((float)((float)*(double *)&v3 * (float)1000.0) + (float)0.5);
            v4 = floor(v3);
            v6 = (int)(float)*(double *)&v4;
            if (cmd_args.nesting >= 8u)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
                    191,
                    0,
                    "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                    cmd_args.nesting,
                    8);
            ShellshockParms = BG_GetShellshockParms(0);
            BG_SetShellShockParmsFromDvars(ShellshockParms);
            cgArray[0].testShock.time = cgArray[0].time;
            cgArray[0].testShock.duration = v6;
        }
    }
}

void CG_ShellShock_Load_f()
{
    const char *v0; // r3

    if (cgArray[0].nextSnap)
    {
        if (Cmd_Argc() == 2)
        {
            v0 = Cmd_Argv(1);
            BG_LoadShellShockDvars(v0);
        }
        else
        {
            Com_Printf(0, "USAGE: cg_shellshock_load <name>\n");
        }
    }
}

void CG_ShellShock_Save_f()
{
    const char *v0; // r3

    if (cgArray[0].nextSnap)
    {
        if (Cmd_Argc() == 2)
        {
            v0 = Cmd_Argv(1);
            BG_SaveShellShockDvars(v0);
        }
        else
        {
            Com_Printf(0, "USAGE: cg_shellshock_save <name>\n");
        }
    }
}

void CG_ModelPreviewerStepAnim_f()
{
    int v0; // r3
    const char *v1; // r3
    long double v2; // fp2

    if (cgArray[0].nextSnap)
    {
        v0 = Cmd_Argc();
        if (v0 == 1)
        {
            CG_ModelPreviewerStepAnim(-1.0);
        }
        else if (v0 == 2)
        {
            v1 = Cmd_Argv(1);
            v2 = atof(v1);
            CG_ModelPreviewerStepAnim((float)*(double *)&v2);
        }
        else
        {
            Com_Printf(0, "USAGE: cg_mpstepanim <deltaTime> : default = 1/30.0 second \n");
        }
    }
}

void CG_ModelPreviewerPauseAnim_f()
{
    if (cgArray[0].nextSnap)
        CG_ModelPreviewerPauseAnim();
}

void CG_Noclip_f()
{
    char v0; // r11
    const dvar_s *v1; // r3
    int v2; // r31
    const char *v3; // r30
    const char *v4; // r3

    if (cgArray[0].nextSnap)
    {
        if (sv_cheats->current.enabled)
        {
            v0 = 1;
        }
        else
        {
            Com_Printf(16, "%s is cheat protected.\n", "cg_noclip");
            v0 = 0;
        }
        if (v0)
        {
            v1 = cl_freemove;
            if (cl_freemove->current.integer == 1)
            {
                v2 = 0;
                v3 = "GAME_NOCLIPOFF";
                if (cg_paused->current.integer == 2)
                {
                    Dvar_SetInt(cg_paused, 1);
                    v1 = cl_freemove;
                }
            }
            else
            {
                v2 = 1;
                v3 = "GAME_NOCLIPON";
            }
            Dvar_SetInt(v1, v2);
            v4 = SEH_LocalizeTextMessage(v3, "noclip print", LOCMSG_SAFE);
            Com_Printf(0, "%s\n", v4);
        }
    }
}

void CG_UFO_f()
{
    char v0; // r11
    const dvar_s *v1; // r3
    int v2; // r31
    const char *v3; // r30
    const char *v4; // r3

    if (cgArray[0].nextSnap)
    {
        if (sv_cheats->current.enabled)
        {
            v0 = 1;
        }
        else
        {
            Com_Printf(16, "%s is cheat protected.\n", "cg_ufo");
            v0 = 0;
        }
        if (v0)
        {
            v1 = cl_freemove;
            if (cl_freemove->current.integer == 2)
            {
                v2 = 0;
                v3 = "GAME_UFOOFF";
                if (cg_paused->current.integer == 2)
                {
                    Dvar_SetInt(cg_paused, 1);
                    v1 = cl_freemove;
                }
            }
            else
            {
                v2 = 2;
                v3 = "GAME_UFOON";
            }
            Dvar_SetInt(v1, v2);
            v4 = SEH_LocalizeTextMessage(v3, "ufo print", LOCMSG_SAFE);
            Com_Printf(0, "%s\n", v4);
        }
    }
}

void __cdecl CG_SetViewPos_f()
{
    int v0; // r30
    float *origin; // r31
    const char *v2; // r3
    long double v3; // fp2
    unsigned int nesting; // r7
    const char *v5; // r3
    long double v6; // fp2
    int v7; // r7
    const char *v8; // r3
    long double v9; // fp2
    float v10[20]; // [sp+50h] [-50h] BYREF

    if (cgArray[0].nextSnap)
    {
        if (cmd_args.nesting >= 8u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
                191,
                0,
                "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                cmd_args.nesting,
                8);
        if (!cgArray[0].predictedPlayerState.pm_type)
            Com_Printf(
                0,
                "\"cg_setviewpos\" isn't very useful when server controlled.  Use cg_ufo/cg_noclip or use \"setviewpos\"\n");
        if (Cmd_Argc() == 4 || Cmd_Argc() == 6)
        {
            v0 = 0;
            origin = cgArray[0].predictedPlayerState.origin;
            do
            {
                v2 = Cmd_Argv(++v0);
                v3 = atof(v2);
                *origin++ = *(double *)&v3;
            } while ((int)origin < (int)cgArray[0].predictedPlayerState.velocity);
            nesting = cmd_args.nesting;
            if (cmd_args.nesting >= 8u)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
                    160,
                    0,
                    "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                    cmd_args.nesting,
                    8);
                nesting = cmd_args.nesting;
            }
            if (cmd_args.argc[nesting] == 6)
            {
                if (nesting >= 8)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
                        174,
                        0,
                        "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                        nesting,
                        8);
                    nesting = cmd_args.nesting;
                }
                if (cmd_args.argc[nesting] <= 4)
                    v5 = "";
                else
                    v5 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 4);
                v6 = atof(v5);
                v7 = cmd_args.nesting;
                v10[1] = *(double *)&v6;
                if (cmd_args.nesting >= 8u)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
                        174,
                        0,
                        "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                        cmd_args.nesting,
                        8);
                    v7 = cmd_args.nesting;
                }
                if (cmd_args.argc[v7] <= 5)
                    v8 = "";
                else
                    v8 = (const char *)*((unsigned int *)cmd_args.argv[v7] + 5);
                v9 = atof(v8);
                v10[2] = 0.0;
                v10[0] = *(double *)&v9;
                CG_SetDebugAngles(v10);
            }
        }
        else
        {
            Com_Printf(0, "USAGE: cg_setviewpos x y z [yaw pitch]\n");
        }
    }
}

void __cdecl SphereCoordsToPos(
    float sphereDistance,
    float sphereYaw,
    float sphereAltitude,
    float *result)
{
    float v15; // fp28
    float v17; // fp29
    float v19; // fp30
    float v20; // fp2

    v15 = sinf(sphereYaw);
    v17 = cosf(((90.0f - sphereAltitude) * 0.017453292f));
    v19 = sinf(((sphereYaw - 90.0f) * 0.017453292f));
    v20 = cosf(((sphereYaw - 90.0f) * 0.017453292f));

    result[0] = (v20 * v15) * sphereDistance;
    result[1] = (v19 * v15) * sphereDistance;
    result[2] = v17 * sphereDistance;
}

void __cdecl CG_SetViewOrbit_f()
{
    playerState_s *p_predictedPlayerState; // r31
    float focusX; // fp31
    float focusY; // fp30
    float focusZ; // fp29
    float dist; // fp28
    float degUp; // fp27
    long double degAround; // fp2
    float len; // fp11
    float vec[3]; // [sp+50h] [-70h] BYREF // v30
    float pos[4]; // [sp+60h] [-60h] BYREF
    float angles[3]; // [sp+70h] [-50h] BYREF

    if (cgArray[0].nextSnap)
    {
        if (Cmd_Argc() == 7)
        {
            p_predictedPlayerState = &CG_GetLocalClientGlobals(Cmd_LocalClientNum())->predictedPlayerState;
            Dvar_SetInt(cl_freemove, 2);

            focusX = atof(Cmd_Argv(1));
            focusY = atof(Cmd_Argv(2));
            focusZ = atof(Cmd_Argv(3));
            dist = atof(Cmd_Argv(4));
            degUp = atof(Cmd_Argv(5));
            degAround = atof(Cmd_Argv(6));

            focusZ = (float)((float)focusZ - p_predictedPlayerState->viewHeightCurrent);

            SphereCoordsToPos(dist, degAround, degUp, pos);

            p_predictedPlayerState->origin[0] = pos[0] + (float)focusX;
            p_predictedPlayerState->origin[1] = pos[1] + (float)focusY;
            p_predictedPlayerState->origin[2] = pos[2] + (float)focusZ;

            vec[0] = (float)focusX - p_predictedPlayerState->origin[0];
            vec[1] = (float)focusY - p_predictedPlayerState->origin[1];
            vec[2] = (float)focusZ - p_predictedPlayerState->origin[2];

            float temp = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
            len = 1.0f / sqrtf(temp);

            vec[0] *= len;
            vec[1] *= len;
            vec[2] *= len;
            vectoangles(vec, angles);
            CG_SetDebugAngles(angles);
        }
        else
        {
            Com_Printf(0, "USAGE: cg_setViewOrbit focusX focusY focusZ dist degUp degAround\n");
        }
    }
}

void CG_PlayRumble_f()
{
    const char *v0; // r31
    int v1; // r3

    if (cgArray[0].nextSnap)
    {
        if (Cmd_Argc() == 2)
        {
            v0 = Cmd_Argv(1);
            v1 = Cmd_LocalClientNum();
            //CG_PlayRumbleOnClient(v1, v0); // KISAKTODO
        }
        else
        {
            Com_Printf(0, "USAGE: playrumble <rumblename>\n");
        }
    }
}

// attributes: thunk
void __cdecl UpdateGlowTweaks_f()
{
    CG_VisionSetUpdateTweaksFromFile_Glow();
}

// attributes: thunk
void __cdecl UpdateFilmTweaks_f()
{
    CG_VisionSetUpdateTweaksFromFile_Film();
}

cmd_function_s CG_Viewpos_f_VAR;
cmd_function_s CG_ScoresDown_f_VAR;
cmd_function_s CG_ScoresUp_f_VAR;
cmd_function_s CG_NextWeapon_f_VAR;
cmd_function_s CG_PrevWeapon_f_VAR;
cmd_function_s CG_Fade_f_VAR;
cmd_function_s CG_ActionSlotDown_f_VAR;
cmd_function_s CG_ActionSlotUp_f_VAR;
cmd_function_s CG_ShellShock_f_VAR;
cmd_function_s CG_ShellShock_Load_f_VAR;
cmd_function_s CG_ShellShock_Save_f_VAR;
cmd_function_s CG_ModelPreviewerPauseAnim_f_VAR;
cmd_function_s CG_ModelPreviewerStepAnim_f_VAR;
cmd_function_s CG_Noclip_f_VAR;
cmd_function_s CG_UFO_f_VAR;
cmd_function_s CG_FxSetTestPosition_VAR;
cmd_function_s CG_FxTest_VAR;
cmd_function_s CG_SetViewPos_f_VAR;
cmd_function_s CG_SetViewOrbit_f_VAR;
cmd_function_s UpdateGlowTweaks_f_VAR;
cmd_function_s UpdateFilmTweaks_f_VAR;
cmd_function_s CG_PlayRumble_f_VAR;
cmd_function_s ai_history_VAR;
cmd_function_s kill_VAR;
cmd_function_s give_VAR;
cmd_function_s take_VAR;
cmd_function_s god_VAR;
cmd_function_s demigod_VAR;
cmd_function_s notarget_VAR;
cmd_function_s noclip_VAR;
cmd_function_s ufo_VAR;
cmd_function_s levelshot_VAR;
cmd_function_s setviewpos_VAR;
cmd_function_s jumptonode_VAR;
cmd_function_s stats_VAR;
cmd_function_s echo_VAR;
cmd_function_s printentities_VAR;
cmd_function_s VisionSetNaked_VAR;
cmd_function_s VisionSetNight_VAR;

void __cdecl CG_InitConsoleCommands()
{
    Cmd_AddCommandInternal("viewpos", CG_Viewpos_f, &CG_Viewpos_f_VAR);
    Cmd_AddCommandInternal("+scores", CG_ScoresDown_f, &CG_ScoresDown_f_VAR);
    Cmd_AddCommandInternal("-scores", CG_ScoresUp_f, &CG_ScoresUp_f_VAR);
    Cmd_AddCommandInternal("weapnext", CG_NextWeapon_f, &CG_NextWeapon_f_VAR);
    Cmd_AddCommandInternal("weapprev", CG_PrevWeapon_f, &CG_PrevWeapon_f_VAR);
    Cmd_AddCommandInternal("fade", CG_Fade_f, &CG_Fade_f_VAR);
    Cmd_AddCommandInternal("+actionslot", CG_ActionSlotDown_f, &CG_ActionSlotDown_f_VAR);
    Cmd_AddCommandInternal("-actionslot", CG_ActionSlotUp_f, &CG_ActionSlotUp_f_VAR);
    Cmd_AddCommandInternal("cg_shellshock", CG_ShellShock_f, &CG_ShellShock_f_VAR);
    Cmd_AddCommandInternal("cg_shellshock_load", CG_ShellShock_Load_f, &CG_ShellShock_Load_f_VAR);
    Cmd_AddCommandInternal("cg_shellshock_save", CG_ShellShock_Save_f, &CG_ShellShock_Save_f_VAR);
    Cmd_AddCommandInternal("cg_mppauseanim", CG_ModelPreviewerPauseAnim_f, &CG_ModelPreviewerPauseAnim_f_VAR);
    Cmd_AddCommandInternal("cg_mpstepanim", CG_ModelPreviewerStepAnim_f, &CG_ModelPreviewerStepAnim_f_VAR);
    Cmd_AddCommandInternal("cg_noclip", CG_Noclip_f, &CG_Noclip_f_VAR);
    Cmd_AddCommandInternal("cg_ufo", CG_UFO_f, &CG_UFO_f_VAR);
    Cmd_AddCommandInternal("fxSetTestPosition", CG_FxSetTestPosition, &CG_FxSetTestPosition_VAR);
    Cmd_AddCommandInternal("fxTest", CG_FxTest, &CG_FxTest_VAR);
    Cmd_AddCommandInternal("cg_setviewpos", CG_SetViewPos_f, &CG_SetViewPos_f_VAR);
    Cmd_AddCommandInternal("cg_setvieworbit", CG_SetViewOrbit_f, &CG_SetViewOrbit_f_VAR);
    Cmd_AddCommandInternal("updateGlowTweaks", UpdateGlowTweaks_f, &UpdateGlowTweaks_f_VAR);
    Cmd_AddCommandInternal("updateFilmTweaks", UpdateFilmTweaks_f, &UpdateFilmTweaks_f_VAR);
    Cmd_AddCommandInternal("playrumble", CG_PlayRumble_f, &CG_PlayRumble_f_VAR);
    Cmd_AddCommandInternal("ai_history", 0, &ai_history_VAR);
    Cmd_AddCommandInternal("kill", 0, &kill_VAR);
    Cmd_AddCommandInternal("give", 0, &give_VAR);
    Cmd_AddCommandInternal("take", 0, &take_VAR);
    Cmd_AddCommandInternal("god", 0, &god_VAR);
    Cmd_AddCommandInternal("demigod", 0, &demigod_VAR);
    Cmd_AddCommandInternal("notarget", 0, &notarget_VAR);
    Cmd_AddCommandInternal("noclip", 0, &noclip_VAR);
    Cmd_AddCommandInternal("ufo", 0, &ufo_VAR);
    Cmd_AddCommandInternal("levelshot", 0, &levelshot_VAR);
    Cmd_AddCommandInternal("setviewpos", 0, &setviewpos_VAR);
    Cmd_AddCommandInternal("jumptonode", 0, &jumptonode_VAR);
    Cmd_AddCommandInternal("stats", 0, &stats_VAR);
    Cmd_AddCommandInternal("echo", 0, &echo_VAR);
    Cmd_AddCommandInternal("printentities", 0, &printentities_VAR);
    Cmd_AddCommandInternal("VisionSetNaked", 0, &VisionSetNaked_VAR);
    Cmd_AddCommandInternal("VisionSetNight", 0, &VisionSetNight_VAR);
}

