#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <qcommon/cmd.h>


void __cdecl CG_ScoresUp(int32_t localClientNum)
{
    cg_s *cgameGlob;

    if (CG_IsScoreboardDisplayed(localClientNum))
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cgameGlob->showScores = 0;
        cgameGlob->scoresTop = -1;
        cgameGlob->scoreFadeTime = cgameGlob->time;
    }
}

cmd_function_s CG_Viewpos_f_VAR;
cmd_function_s CG_ScoresDown_f_VAR;
cmd_function_s CG_ScoresUp_f_VAR;
cmd_function_s CG_NextWeapon_f_VAR;
cmd_function_s CG_PrevWeapon_f_VAR;
cmd_function_s CG_ActionSlotDown_f_VAR;
cmd_function_s CG_ActionSlotUp_f_VAR;
cmd_function_s CG_ShellShock_f_VAR;
cmd_function_s CG_ShellShock_Load_f_VAR;
cmd_function_s CG_ShellShock_Save_f_VAR;
cmd_function_s CG_QuickMessage_f_VAR;
cmd_function_s CG_VoiceChat_f_VAR;
cmd_function_s CG_TeamVoiceChat_f_VAR;
cmd_function_s CG_FxSetTestPosition_VAR;
cmd_function_s CG_FxTest_VAR;
cmd_function_s CG_RestartSmokeGrenades_f_VAR;
cmd_function_s UpdateGlowTweaks_f_VAR;
cmd_function_s UpdateFilmTweaks_f_VAR;
cmd_function_s mr_VAR;
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
cmd_function_s say_VAR;
cmd_function_s say_team_VAR;
cmd_function_s team_VAR;
cmd_function_s follow_VAR;
cmd_function_s callvote_VAR;
cmd_function_s vote_VAR;
cmd_function_s follownext_VAR;
cmd_function_s followprev_VAR;
cmd_function_s printentities_VAR;
cmd_function_s muteplayer_VAR;
cmd_function_s unmuteplayer_VAR;
cmd_function_s VisionSetNaked_VAR;
cmd_function_s VisionSetNight_VAR;


void __cdecl CG_InitConsoleCommands()
{
    Cmd_AddCommandInternal("viewpos", CG_Viewpos_f, &CG_Viewpos_f_VAR);
    Cmd_AddCommandInternal("+scores", CG_ScoresDown_f, &CG_ScoresDown_f_VAR);
    Cmd_AddCommandInternal("-scores", CG_ScoresUp_f, &CG_ScoresUp_f_VAR);
    Cmd_AddCommandInternal("weapnext", CG_NextWeapon_f, &CG_NextWeapon_f_VAR);
    Cmd_AddCommandInternal("weapprev", CG_PrevWeapon_f, &CG_PrevWeapon_f_VAR);
    Cmd_AddCommandInternal("+actionslot", CG_ActionSlotDown_f, &CG_ActionSlotDown_f_VAR);
    Cmd_AddCommandInternal("-actionslot", CG_ActionSlotUp_f, &CG_ActionSlotUp_f_VAR);
    Cmd_AddCommandInternal("cg_shellshock", CG_ShellShock_f, &CG_ShellShock_f_VAR);
    Cmd_AddCommandInternal("cg_shellshock_load", CG_ShellShock_Load_f, &CG_ShellShock_Load_f_VAR);
    Cmd_AddCommandInternal("cg_shellshock_save", CG_ShellShock_Save_f, &CG_ShellShock_Save_f_VAR);
    Cmd_AddCommandInternal("mp_QuickMessage", CG_QuickMessage_f, &CG_QuickMessage_f_VAR);
    Cmd_AddCommandInternal("VoiceChat", CG_VoiceChat_f, &CG_VoiceChat_f_VAR);
    Cmd_AddCommandInternal("VoiceTeamChat", CG_TeamVoiceChat_f, &CG_TeamVoiceChat_f_VAR);
    Cmd_AddCommandInternal("fxSetTestPosition", CG_FxSetTestPosition, &CG_FxSetTestPosition_VAR);
    Cmd_AddCommandInternal("fxTest", CG_FxTest, &CG_FxTest_VAR);
    Cmd_AddCommandInternal("restartsmokegrenades", CG_RestartSmokeGrenades_f, &CG_RestartSmokeGrenades_f_VAR);
    Cmd_AddCommandInternal("updateGlowTweaks", UpdateGlowTweaks_f, &UpdateGlowTweaks_f_VAR);
    Cmd_AddCommandInternal("updateFilmTweaks", UpdateFilmTweaks_f, &UpdateFilmTweaks_f_VAR);
    Cmd_AddCommandInternal("mr", 0, &mr_VAR);
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
    Cmd_AddCommandInternal("say", 0, &say_VAR);
    Cmd_AddCommandInternal("say_team", 0, &say_team_VAR);
    Cmd_AddCommandInternal("team", 0, &team_VAR);
    Cmd_AddCommandInternal("follow", 0, &follow_VAR);
    Cmd_AddCommandInternal("callvote", 0, &callvote_VAR);
    Cmd_AddCommandInternal("vote", 0, &vote_VAR);
    Cmd_AddCommandInternal("follownext", 0, &follownext_VAR);
    Cmd_AddCommandInternal("followprev", 0, &followprev_VAR);
    Cmd_AddCommandInternal("printentities", 0, &printentities_VAR);
    Cmd_AddCommandInternal("muteplayer", 0, &muteplayer_VAR);
    Cmd_AddCommandInternal("unmuteplayer", 0, &unmuteplayer_VAR);
    Cmd_AddCommandInternal("VisionSetNaked", 0, &VisionSetNaked_VAR);
    Cmd_AddCommandInternal("VisionSetNight", 0, &VisionSetNight_VAR);
}

void __cdecl CG_Viewpos_f()
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
        Com_Printf(
            0,
            "(%.0f %.0f %.0f) : %.0f %.0f\n",
            cgameGlob->refdef.vieworg[0],
            cgameGlob->refdef.vieworg[1],
            cgameGlob->refdef.vieworg[2],
            cgameGlob->refdefViewAngles[1],
            cgameGlob->refdefViewAngles[0]);
}

void __cdecl CG_ScoresUp_f()
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        CG_ScoresUp(0);
        if (UI_GetActiveMenu(0) == 10)
            UI_SetActiveMenu(0, UIMENU_NONE);
    }
}

void __cdecl CG_ScoresDown_f()
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        CG_ScoresDown(0);
        if (UI_GetActiveMenu(0) != 10)
            UI_SetActiveMenu(0, UIMENU_SCOREBOARD);
    }
}

void __cdecl CG_ScoresDown(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->scoresRequestTime + 2000 >= cgameGlob->time)
    {
        cgameGlob->showScores = 1;
    }
    else
    {
        cgameGlob->scoresRequestTime = cgameGlob->time;
        CL_AddReliableCommand(localClientNum, "score");
        if (!CG_IsScoreboardDisplayed(localClientNum))
        {
            cgameGlob->numScores = 0;
            cgameGlob->scoresTop = -1;
            cgameGlob->showScores = 1;
        }
    }
}

void __cdecl CG_ShellShock_f()
{
    const char *v0; // eax
    const char *v1; // eax
    shellshock_parms_t *ShellshockParms; // eax
    float v3; // [esp+0h] [ebp-28h]
    int32_t v4; // [esp+4h] [ebp-24h]
    float v5; // [esp+8h] [ebp-20h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        v4 = Cmd_Argc();
        if (v4 == 2)
        {
        LABEL_5:
            v1 = Cmd_Argv(1);
            v3 = atof(v1);
            ShellshockParms = BG_GetShellshockParms(0);
            BG_SetShellShockParmsFromDvars(ShellshockParms);
            cgameGlob->testShock.time = cgameGlob->time;
            cgameGlob->testShock.duration = (int)(v3 * 1000.0f);
            return;
        }
        if (v4 == 3)
        {
            v0 = Cmd_Argv(2);
            if (!BG_LoadShellShockDvars(v0))
                return;
            goto LABEL_5;
        }
        Com_Printf(0, "USAGE: cg_shellshock <duration> <filename?>\n");
    }
}

void __cdecl CG_ShellShock_Load_f()
{
    const char *name; // [esp+0h] [ebp-4h]

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        if (Cmd_Argc() == 2)
        {
            name = Cmd_Argv(1);
            BG_LoadShellShockDvars(name);
        }
        else
        {
            Com_Printf(0, "USAGE: cg_shellshock_load <name>\n");
        }
    }
}

void __cdecl CG_ShellShock_Save_f()
{
    const char *name; // [esp+0h] [ebp-4h]

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        if (Cmd_Argc() == 2)
        {
            name = Cmd_Argv(1);
            BG_SaveShellShockDvars(name);
        }
        else
        {
            Com_Printf(0, "USAGE: cg_shellshock_save <name>\n");
        }
    }
}

void __cdecl CG_QuickMessage_f()
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        if ((cgameGlob->nextSnap->ps.otherFlags & 4) != 0)
            UI_Popup(0, "UIMENU_WM_QUICKMESSAGE");
    }
}

void __cdecl CG_VoiceChat_f()
{
    char *v0; // eax
    const char *v1; // eax
    const char *chatCmd; // [esp+8h] [ebp-4h]

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap && Cmd_Argc() == 2)
    {
        if (cgameGlob->nextSnap->ps.pm_type == PM_INTERMISSION || (cgameGlob->nextSnap->ps.otherFlags & 4) != 0)
        {
            chatCmd = Cmd_Argv(1);
            v1 = va("cmd vsay %s\n", chatCmd);
            Cbuf_AddText(0, v1);
        }
        else
        {
            v0 = UI_SafeTranslateString((char*)"CGAME_NOSPECTATORVOICECHAT");
            Com_Printf(0, "%s\n", v0);
        }
    }
}

void __cdecl CG_TeamVoiceChat_f()
{
    char *v0; // eax
    const char *v1; // eax
    const char *chatCmd; // [esp+8h] [ebp-4h]

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap && Cmd_Argc() == 2)
    {
        if (cgameGlob->nextSnap->ps.pm_type == PM_INTERMISSION || (cgameGlob->nextSnap->ps.otherFlags & 4) != 0)
        {
            chatCmd = Cmd_Argv(1);
            v1 = va("cmd vsay_team %s\n", chatCmd);
            Cbuf_AddText(0, v1);
        }
        else
        {
            v0 = UI_SafeTranslateString((char*)"CGAME_NOSPECTATORVOICECHAT");
            Com_Printf(0, "%s\n", v0);
        }
    }
}

void __cdecl CG_RestartSmokeGrenades_f()
{
    CG_RestartSmokeGrenades(0);
}

void __cdecl UpdateGlowTweaks_f()
{
    CG_VisionSetUpdateTweaksFromFile_Glow();
}

void __cdecl UpdateFilmTweaks_f()
{
    CG_VisionSetUpdateTweaksFromFile_Film();
}

void __cdecl CG_ShutdownConsoleCommands()
{
    Cmd_RemoveCommand("viewpos");
    Cmd_RemoveCommand("+scores");
    Cmd_RemoveCommand("-scores");
    Cmd_RemoveCommand("weapnext");
    Cmd_RemoveCommand("weapprev");
    Cmd_RemoveCommand("+actionslot");
    Cmd_RemoveCommand("-actionslot");
    Cmd_RemoveCommand("cg_shellshock");
    Cmd_RemoveCommand("cg_shellshock_load");
    Cmd_RemoveCommand("cg_shellshock_save");
    Cmd_RemoveCommand("mp_QuickMessage");
    Cmd_RemoveCommand("VoiceChat");
    Cmd_RemoveCommand("VoiceTeamChat");
    Cmd_RemoveCommand("fxSetTestPosition");
    Cmd_RemoveCommand("fxTest");
    Cmd_RemoveCommand("restartsmokegrenades");
    Cmd_RemoveCommand("updateGlowTweaks");
    Cmd_RemoveCommand("updateFilmTweaks");
}

