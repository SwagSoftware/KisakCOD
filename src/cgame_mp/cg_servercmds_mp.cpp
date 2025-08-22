#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <client_mp/client_mp.h>
#include <database/database.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <gfx_d3d/r_fog.h>
#include <EffectsCore/fx_system.h>
#include <DynEntity/DynEntity_client.h>
#include <gfx_d3d/r_bsp.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_storage.h>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_model.h>
#include <gfx_d3d/r_primarylights.h>
#include <qcommon/com_bsp.h>

struct $59835072FC2CD3936CE4A4C9F556010B // sizeof=0x48
{                                       // ...
    char name[64];                      // ...
    int32_t index;                          // ...
    bool useMouse;                      // ...
    // padding byte
    // padding byte
    // padding byte
};
$59835072FC2CD3936CE4A4C9F556010B cg_waitingScriptMenu[1];

void __cdecl CG_ParseServerInfo(int32_t localClientNum)
{
    const char *info; // [esp+0h] [ebp-Ch]
    const char *mapname; // [esp+8h] [ebp-4h]
    cgs_t *cgs;
    info = CL_GetConfigString(localClientNum, 0);

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    strncpy(cgs->szHostName, Info_ValueForKey(info, "sv_hostname"), 0x100u);
    strncpy(cgs->gametype, Info_ValueForKey(info, "g_gametype"), 0x20u);
    if (!cgs->localServer)
        Dvar_SetStringByName("g_gametype", cgs->gametype);
    cgs->maxclients = atoi(Info_ValueForKey(info, "sv_maxclients"));
    iassert((cgs->maxclients >= 1 && cgs->maxclients <= 64));
    mapname = Info_ValueForKey(info, "mapname");
    Com_GetBspFilename(cgs->mapname, 0x40u, mapname);
}

void __cdecl CG_ParseCodInfo(int32_t localClientNum)
{
    const char *key; // [esp+4h] [ebp-Ch]
    int32_t i; // [esp+8h] [ebp-8h]
    const char *value; // [esp+Ch] [ebp-4h]
    cgs_t *cgs;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    
    if (!cgs->localServer)
    {
        for (i = 0; i < 128; ++i)
        {
            key = CL_GetConfigString(localClientNum, i + 20);
            if (!*key)
                break;
            value = CL_GetConfigString(localClientNum, i + 148);
            Dvar_SetFromStringByName(key, value);
        }
    }
}

void __cdecl CG_ParseFog(int32_t localClientNum)
{
    parseInfo_t *v1; // eax
    parseInfo_t *v2; // eax
    parseInfo_t *v3; // eax
    parseInfo_t *v4; // eax
    float v5; // [esp+14h] [ebp-68h]
    float v6; // [esp+18h] [ebp-64h]
    float v7; // [esp+1Ch] [ebp-60h]
    float v8; // [esp+24h] [ebp-58h]
    float v9; // [esp+38h] [ebp-44h]
    float v10; // [esp+4Ch] [ebp-30h]
    const char *info; // [esp+5Ch] [ebp-20h] BYREF
    uint8_t r; // [esp+63h] [ebp-19h]
    int32_t transitionTime; // [esp+64h] [ebp-18h]
    float start; // [esp+68h] [ebp-14h]
    uint8_t g; // [esp+6Eh] [ebp-Eh]
    uint8_t b; // [esp+6Fh] [ebp-Dh]
    float density; // [esp+70h] [ebp-Ch]
    int32_t time; // [esp+74h] [ebp-8h]
    const char *token; // [esp+78h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    time = cgameGlob->time;
    info = CL_GetConfigString(localClientNum, 9u);
    token = (const char *)Com_Parse(&info);
    start = atof(token);
    token = (const char *)Com_Parse(&info);
    if (token && *token)
    {
        density = atof(token);
        v1 = Com_Parse(&info);
        v7 = atof(v1->token);
        r = (int)(v7 * 255.0f);
        v2 = Com_Parse(&info);
        v6 = atof(v2->token);
        g = (int)(v6 * 255.0f);
        v3 = Com_Parse(&info);
        v5 = atof(v3->token);
        b = (int)(v5 * 255.0f);
        v4 = Com_Parse(&info);
        transitionTime = atoi(v4->token);
        R_SetFogFromServer(start, r, g, b, density);
        R_SwitchFog(1u, time, transitionTime);
    }
    else
    {
        R_SwitchFog(0, time, (int)start);
    }
}

void __cdecl CG_SetConfigValues(int32_t localClientNum)
{
    const char *ConfigString; // eax
    int32_t i; // [esp+8h] [ebp-4h]
    int32_t ia; // [esp+8h] [ebp-4h]
    int32_t ib; // [esp+8h] [ebp-4h]
    int32_t ic; // [esp+8h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    CL_ParseMapCenter(localClientNum);
    ConfigString = CL_GetConfigString(localClientNum, 4u);
    cgameGlob->teamScores[1] = atoi(ConfigString);
    cgameGlob->teamScores[2] = atoi(CL_GetConfigString(localClientNum, 5));
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    R_SwitchFog(0, cgameGlob->time, 0);
    for (i = 1970; i < 2002; ++i)
        CG_PrecacheScriptMenu(localClientNum, i);
    for (ia = 2259; ia < 2267; ++ia)
    {
        Material_RegisterHandle(CL_GetConfigString(localClientNum, ia), 7);
    }
    for (ib = 2267; ib < 2282; ++ib)
    {
        Material_RegisterHandle(CL_GetConfigString(localClientNum, ib), 7);
    }
    for (ic = 2003; ic < 2258; ++ic)
        CG_RegisterServerMaterial(localClientNum, ic);
    CG_ParseGameEndTime(localClientNum);
    CG_VisionSetConfigString_Naked(localClientNum);
    CG_VisionSetConfigString_Night(localClientNum);
}

void __cdecl CG_ParseGameEndTime(int32_t localClientNum)
{
    const char *ConfigString; // eax

    ConfigString = CL_GetConfigString(localClientNum, 0xBu);
    CG_GetLocalClientStaticGlobals(localClientNum)->gameEndTime = atoi(ConfigString);
}

void __cdecl CG_PrecacheScriptMenu(int32_t localClientNum, int32_t configStringIndex)
{
    const char *configString; // [esp+0h] [ebp-4h]

    if (configStringIndex < 1970 || configStringIndex >= 2002)
        MyAssertHandler(
            ".\\cgame_mp\\cg_servercmds_mp.cpp",
            591,
            0,
            "%s",
            "(configStringIndex >= CS_SCRIPT_MENUS) && (configStringIndex < CS_SCRIPT_MENUS + MAX_SCRIPT_MENUS)");
    configString = CL_GetConfigString(localClientNum, configStringIndex);
    if (*configString)
    {
        if (!Load_ScriptMenu(localClientNum, configString, 7))
            Com_Error(ERR_DROP, "Could not load script menu file %s", configString);
    }
}

void __cdecl CG_RegisterServerMaterial(int32_t localClientNum, int32_t configStringIndex)
{
    const char *materialName; // [esp+0h] [ebp-4h]

    if (configStringIndex < 2002 || configStringIndex >= 2258)
        MyAssertHandler(
            ".\\cgame_mp\\cg_servercmds_mp.cpp",
            606,
            0,
            "%s\n\t(configStringIndex) = %i",
            "(configStringIndex >= CS_SERVER_MATERIALS && configStringIndex < CS_SERVER_MATERIALS + 256)",
            configStringIndex);
    materialName = CL_GetConfigString(localClientNum, configStringIndex);
    if (*materialName)
        Material_RegisterHandle(materialName, 7);
}

void __cdecl CG_MapRestart(int32_t localClientNum, int32_t savepersist)
{
    cg_s *cgameGlob;
    cgs_t *cgs;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    if (cg_showmiss->current.integer)
        Com_Printf(14, "CG_MapRestart\n");

    CG_ClearCenterPrint(localClientNum);
    cgameGlob->cursorHintFade = 0;
    cgameGlob->lastHealthLerpDelay = 1;
    CG_InitLocalEntities(localClientNum);
    FX_KillAllEffects(localClientNum);
    FX_ShutdownSystem(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    if (CG_ShouldPlaySoundOnLocalClient())
    {
        Phys_Shutdown();
        Phys_Init();
    }
    DynEntCl_InitEntities(localClientNum);
    R_InitPrimaryLights(cgameGlob->refdef.primaryLights);
    R_ClearShadowedPrimaryLightHistory(localClientNum);
    FX_InitSystem(localClientNum);
    CG_ClearEntityFxHandles(localClientNum);
    CG_VisionSetConfigString_Naked(localClientNum);
    CG_VisionSetConfigString_Night(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    cgs->voteTime = 0;
    cgameGlob->mapRestart = 1;
    SND_StopSounds(SND_STOP_ALL);
    CG_StartAmbient(localClientNum);
    cgameGlob->v_dmg_time = 0;
    memset((uint8_t *)cgameGlob->viewDamage, 0, sizeof(cgameGlob->viewDamage));
    Dvar_SetBool(cg_thirdPerson, 0);
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);
    if (!savepersist)
    {
        CG_CloseScriptMenu(localClientNum, 0);
        UI_CloseAllMenus(localClientNum);
        memset((uint8_t *)cgameGlob->scores, 0, sizeof(cgameGlob->scores));
        cgameGlob->teamScores[0] = 0;
        cgameGlob->teamScores[1] = 0;
        cgameGlob->teamScores[2] = 0;
        cgameGlob->teamScores[3] = 0;
    }
    CG_ScoresUp(localClientNum);
    cgameGlob->objectiveText[0] = 0;
    CL_SyncTimes(localClientNum);
    CG_StartClientSideEffects(localClientNum);
}

void __cdecl CG_ClearEntityFxHandles(int32_t localClientNum)
{
    centity_s *cent; // [esp+4h] [ebp-10h]
    int32_t num; // [esp+Ch] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    for (num = 0; num < cgameGlob->snap->numEntities; ++num)
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->snap->entities[num].number);
        if (cent->nextState.eType == ET_FX || cent->nextState.eType == ET_LOOP_FX)
        {
            cent->pose.fx.effect = 0;
            cent->pose.fx.triggerTime = 0;
        }
    }
}

void __cdecl CG_CheckOpenWaitingScriptMenu(int32_t localClientNum)
{
    if (cg_waitingScriptMenu[localClientNum].name[0])
    {
        if (UI_PopupScriptMenu(
            localClientNum,
            cg_waitingScriptMenu[localClientNum].name,
            cg_waitingScriptMenu[localClientNum].useMouse))
        {
            cg_waitingScriptMenu[localClientNum].name[0] = 0;
        }
    }
}

void __cdecl CG_CloseScriptMenu(int32_t localClientNum, bool allowResponse)
{
    UI_ClosePopupScriptMenu(localClientNum, allowResponse);
    cg_waitingScriptMenu[localClientNum].name[0] = 0;
}

void __cdecl CG_MenuShowNotify(int32_t localClientNum, int32_t menuToShow)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    
    switch (menuToShow)
    {
    case 0:
        if (cgameGlob->healthFadeTime < cgameGlob->time)
        {
            cgameGlob->healthFadeTime = cgameGlob->time;
            if (CL_GetLocalClientActiveCount() == 1)
                Menus_ShowByName(&cgDC[localClientNum], "Health");
            else
                Menus_ShowByName(&cgDC[localClientNum], "Health_mp");
        }
        break;
    case 1:
    case 4:
        if (cgameGlob->ammoFadeTime < cgameGlob->time)
        {
            cgameGlob->ammoFadeTime = cgameGlob->time;
            if (CL_GetLocalClientActiveCount() == 1)
            {
                Menus_ShowByName(&cgDC[localClientNum], "weaponinfo");
                Menus_ShowByName(&cgDC[localClientNum], "weaponinfo_lowdef");
            }
            else
            {
                Menus_ShowByName(&cgDC[localClientNum], "weaponinfo_mp");
            }
        }
        if (cgameGlob->offhandFadeTime < cgameGlob->time)
        {
            cgameGlob->offhandFadeTime = cgameGlob->time;
            if (CL_GetLocalClientActiveCount() == 1)
                Menus_ShowByName(&cgDC[localClientNum], "offhandinfo");
            else
                Menus_ShowByName(&cgDC[localClientNum], "offhandinfo_mp");
        }
        break;
    case 2:
        if (cgameGlob->compassFadeTime < cgameGlob->time)
        {
            cgameGlob->compassFadeTime = cgameGlob->time;
            if (CL_GetLocalClientActiveCount() == 1)
                Menus_ShowByName(&cgDC[localClientNum], "Compass");
            else
                Menus_ShowByName(&cgDC[localClientNum], "Compass_mp");
        }
        break;
    case 3:
        if (cgameGlob->stanceFadeTime < cgameGlob->time)
        {
            cgameGlob->stanceFadeTime = cgameGlob->time;
            if (CL_GetLocalClientActiveCount() == 1)
                Menus_ShowByName(&cgDC[localClientNum], "stance");
            else
                Menus_ShowByName(&cgDC[localClientNum], "stance_mp");
        }
        break;
    case 5:
        if (cgameGlob->scoreFadeTime < cgameGlob->time)
        {
            cgameGlob->scoreFadeTime = cgameGlob->time;
            Menus_ShowByName(&cgDC[localClientNum], "objectiveinfo");
        }
        break;
    case 6:
        if (cgameGlob->sprintFadeTime < cgameGlob->time)
        {
            cgameGlob->sprintFadeTime = cgameGlob->time;
            if (CL_GetLocalClientActiveCount() == 1)
                Menus_ShowByName(&cgDC[localClientNum], "sprintMeter");
            else
                Menus_ShowByName(&cgDC[localClientNum], "sprintMeter_mp");
        }
        break;
    default:
        return;
    }
}

void __cdecl CG_ServerCommand(int32_t localClientNum)
{
    CG_DeployServerCommand(localClientNum);
    Cmd_EndTokenizedString();
}

void __cdecl CG_DeployServerCommand(int32_t localClientNum)
{
    const char *v1; // eax
    int32_t v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    const snd_alias_t *v18; // eax
    const char *v19; // eax
    int32_t v20; // eax
    const char *v21; // eax
    const char *v22; // eax
    char *v23; // eax
    char *v24; // eax
    const char *v25; // eax
    const char *v26; // eax
    uint32_t v27; // eax
    const char *v28; // eax
    int32_t v29; // eax
    const char *v30; // eax
    int32_t v31; // eax
    const char *v32; // eax
    const char *v33; // eax
    int32_t v34; // eax
    const char *v35; // eax
    int32_t v36; // eax
    const char *v37; // eax
    const char *v38; // eax
    int32_t v39; // eax
    const char *v40; // eax
    const char *v41; // eax
    bool volume; // [esp+0h] [ebp-200h]
    float volumea; // [esp+0h] [ebp-200h]
    int32_t volumeb; // [esp+0h] [ebp-200h]
    int32_t v45; // [esp+4h] [ebp-1FCh]
    int32_t v46; // [esp+4h] [ebp-1FCh]
    uint32_t v47; // [esp+4h] [ebp-1FCh]
    float pos[3]; // [esp+28h] [ebp-1D8h] BYREF
    float dir[3]; // [esp+34h] [ebp-1CCh] BYREF
    uint32_t drawType; // [esp+40h] [ebp-1C0h]
    uint16_t id; // [esp+44h] [ebp-1BCh]
    char hudElemString[260]; // [esp+48h] [ebp-1B8h] BYREF
    cg_s *cgameGlob; // [esp+14Ch] [ebp-B4h]
    const char *s; // [esp+150h] [ebp-B0h]
    int32_t weapIndex; // [esp+154h] [ebp-ACh]
    int32_t i; // [esp+158h] [ebp-A8h]
    const char *cmd; // [esp+15Ch] [ebp-A4h]
    char text[152]; // [esp+160h] [ebp-A0h] BYREF
    int32_t argc; // [esp+1FCh] [ebp-4h]

    cmd = Cmd_Argv(0);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    switch (*cmd)
    {
    case 0:
        return;
    case 0x42:
        CG_MapRestart(localClientNum, 0);
        break;
    case 0x43:
        v25 = Cmd_Argv(1);
        weapIndex = atoi(v25);
        if (!weapIndex || BG_GetWeaponDef(weapIndex)->offhandClass)
            CG_SetEquippedOffHand(localClientNum, weapIndex);
        break;
    case 0x44:
        CG_DeactivateReverbCmd();
        break;
    case 0x45:
        CG_SetChannelVolCmd(localClientNum);
        break;
    case 0x46:
        CG_DeactivateChannelVolCmd();
        break;
    case 0x47:
        v28 = Cmd_Argv(1);
        v29 = atoi(v28);
        CG_SetTeamScore(localClientNum, 1u, v29);
        break;
    case 0x48:
        v30 = Cmd_Argv(1);
        v31 = atoi(v30);
        CG_SetTeamScore(localClientNum, 2u, v31);
        break;
    case 0x49:
        v32 = Cmd_Argv(2);
        v46 = atoi(v32);
        v33 = Cmd_Argv(1);
        v34 = atoi(v33);
        CG_SetSingleClientScore(localClientNum, v34, v46);
        break;
    case 0x4A:
        v35 = Cmd_Argv(1);
        v36 = atoi(v35);
        CG_MenuShowNotify(localClientNum, v36);
        break;
    case 0x4B:
        v26 = Cmd_Argv(1);
        v27 = atoi(v26);
        CL_ResetPlayerMuting(v27);
        break;
    case 0x4C:
        UI_CloseInGameMenu(localClientNum);
        break;
    case 0x4E:
        v37 = Cmd_Argv(2);
        v47 = atoi(v37);
        v38 = Cmd_Argv(1);
        volumeb = atoi(v38);
        v39 = CL_ControllerIndexFromClientNum(localClientNum);
        LiveStorage_SetStat(v39, volumeb, v47);
        break;
    case 0x61:
        v1 = Cmd_Argv(1);
        v2 = atoi(v1);
        CG_SelectWeaponIndex(localClientNum, v2);
        break;
    case 0x62:
        CG_ParseScores(localClientNum);
        break;
    case 0x63:
        v3 = Cmd_Argv(1);
        CG_TranslateHudElemMessage(localClientNum, v3, "announcement message", hudElemString);
        CG_BoldGameMessage(localClientNum, hudElemString);
        break;
    case 0x64:
        CG_ConfigStringModified(localClientNum);
        break;
    case 0x65:
    case 0x66:
        v4 = Cmd_Argv(1);
        CG_TranslateHudElemMessage(localClientNum, v4, "game message", hudElemString);
        CG_GameMessage(localClientNum, hudElemString);
        break;
    case 0x67:
        v5 = Cmd_Argv(1);
        CG_TranslateHudElemMessage(localClientNum, v5, "bold game message", hudElemString);
        CG_BoldGameMessage(localClientNum, hudElemString);
        break;
    case 0x68:
        if (!cg_teamChatsOnly->current.enabled)
        {
            v6 = Cmd_Argv(1);
            s = SEH_LocalizeTextMessage(v6, "chat message", LOCMSG_SAFE);
            I_strncpyz(text, (char *)s, 150);
            CG_RemoveChatEscapeChar(text);
            CG_AddToTeamChat(localClientNum, text);
            Com_Printf(14, "%s\n", text);
        }
        break;
    case 0x69:
        v7 = Cmd_Argv(1);
        s = SEH_LocalizeTextMessage(v7, "team chat message", LOCMSG_SAFE);
        I_strncpyz(text, (char *)s, 150);
        CG_RemoveChatEscapeChar(text);
        CG_AddToTeamChat(localClientNum, text);
        Com_Printf(14, "%s\n", text);
        break;
    case 0x6A:
        v8 = Cmd_Argv(1);
        id = atoi(v8);
        v9 = Cmd_Argv(2);
        drawType = atoi(v9);
        v10 = Cmd_Argv(3);
        pos[2] = atof(v10);
        //__asm { fstp[ebp + var_1E4] }
        v11 = Cmd_Argv(4);
        pos[1] = atof(v11);
        //__asm { fstp[ebp + var_1E0] }
        v12 = Cmd_Argv(5);
        pos[0] = atof(v12);
        //__asm
        //{
        //    fstp[ebp + var_1DC]
        //    fld[ebp + var_1E4]
        //    fstp[ebp + pos]
        //    fld[ebp + var_1E0]
        //    fstp[ebp + pos + 4]
        //    fld[ebp + var_1DC]
        //    fstp[ebp + pos + 8]
        //}
        v13 = Cmd_Argv(6);
        dir[2] = atof(v13);
        //__asm { fstp[ebp + var_1F0] }
        v14 = Cmd_Argv(7);
        dir[1] = atof(v14);
        //__asm { fstp[ebp + var_1EC] }
        v15 = Cmd_Argv(8);
        dir[0] = atof(v15);
        //__asm
        //{
        //    fstp[ebp + var_1E8]
        //    fld[ebp + var_1F0]
        //    fstp[ebp + dir]
        //    fld[ebp + var_1EC]
        //    fstp[ebp + dir + 4]
        //    fld[ebp + var_1E8]
        //    fstp[ebp + dir + 8]
        //}
        DynEntCl_DestroyEvent(localClientNum, id, (DynEntityCollType)drawType, pos, dir);
        break;
    case 0x6B:
        LocalSoundStop(localClientNum);
        break;
    case 0x6E:
        CG_MapRestart(localClientNum, 1);
        break;
    case 0x6F:
        if (CG_ShouldPlaySoundOnLocalClient())
        {
            v16 = Cmd_Argv(2);
            volume = atoi(v16) != 0;
            v17 = Cmd_Argv(1);
            v18 = CL_PickSoundAlias(v17);
            SND_PlayMusicAlias(localClientNum, v18, volume, SASYS_CGAME);
        }
        break;
    case 0x70:
        if (CG_ShouldPlaySoundOnLocalClient())
        {
            v19 = Cmd_Argv(1);
            v20 = atoi(v19);
            SND_StopMusic(v20);
        }
        break;
    case 0x71:
        if (CG_ShouldPlaySoundOnLocalClient())
        {
            v21 = Cmd_Argv(2);
            v45 = atoi(v21);
            v22 = Cmd_Argv(1);
            volumea = atof(v22);
            //__asm
            //{
            //    fstp[ebp + var_1F8]
            //    fld[ebp + var_1F8]
            //}
            //__asm { fstp[esp + 200h + volume] }
            SND_FadeAllSounds(volumea, v45);
        }
        break;
    case 0x72:
        CG_ReverbCmd();
        break;
    case 0x73:
        if (!LocalSound(localClientNum))
            CL_DumpReliableCommands(localClientNum);
        break;
    case 0x74:
        CG_OpenScriptMenu(localClientNum);
        break;
    case 0x75:
        CG_CloseScriptMenu(localClientNum, 1);
        break;
    case 0x76:
        for (i = 1; i < Cmd_Argc(); i += 2)
        {
            v23 = (char *)Cmd_Argv(i);
            I_strncpyz(text, v23, 150);
            v24 = (char *)Cmd_Argv(i + 1);
            CG_SetClientDvarFromServer(cgameGlob, text, v24);
        }
        break;
    default:
        v40 = Cmd_Argv(0);
        Com_Printf(14, "Unknown client game command: %s\n", v40);
        argc = Cmd_Argc();
        if (argc > 1)
        {
            Com_Printf(14, "Arguments(%i):", argc - 1);
            for (i = 1; i < argc; ++i)
            {
                v41 = Cmd_Argv(i);
                Com_Printf(14, " %s", v41);
            }
            Com_Printf(14, "\n");
        }
        break;
    }
}

void __cdecl CG_ParseScores(int32_t localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    team_t team; // [esp+8h] [ebp-1Ch]
    int32_t statusIconIndex; // [esp+10h] [ebp-14h]
    int32_t i; // [esp+18h] [ebp-Ch]
    int32_t ia; // [esp+18h] [ebp-Ch]
    int32_t clientNum; // [esp+1Ch] [ebp-8h]
    const char *pszIcon; // [esp+20h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->numScores <= 0)
        cgameGlob->scoresTop = -1;
    v1 = Cmd_Argv(1);
    cgameGlob->numScores = atoi(v1);
    if (cgameGlob->numScores > 64)
        cgameGlob->numScores = 64;
    cgameGlob->teamScores[0] = 0;
    cgameGlob->teamScores[1] = 0;
    cgameGlob->teamScores[2] = 0;
    cgameGlob->teamScores[3] = 0;
    v2 = Cmd_Argv(2);
    cgameGlob->teamScores[1] = atoi(v2);
    v3 = Cmd_Argv(3);
    cgameGlob->teamScores[2] = atoi(v3);
    v4 = Cmd_Argv(4);
    cgameGlob->scoreLimit = atoi(v4);
    memset((uint8_t *)cgameGlob->scores, 0, sizeof(cgameGlob->scores));
    cgameGlob->teamPings[0] = 0;
    cgameGlob->teamPings[1] = 0;
    cgameGlob->teamPings[2] = 0;
    cgameGlob->teamPings[3] = 0;
    cgameGlob->teamPlayers[0] = 0;
    cgameGlob->teamPlayers[1] = 0;
    cgameGlob->teamPlayers[2] = 0;
    cgameGlob->teamPlayers[3] = 0;
    for (i = 0; i < cgameGlob->numScores; ++i)
    {
        v5 = Cmd_Argv(7 * i + 5);
        cgameGlob->scores[i].client = atoi(v5);
        v6 = Cmd_Argv(7 * i + 6);
        cgameGlob->scores[i].score = atoi(v6);
        v7 = Cmd_Argv(7 * i + 7);
        cgameGlob->scores[i].ping = atoi(v7);
        v8 = Cmd_Argv(7 * i + 8);
        cgameGlob->scores[i].deaths = atoi(v8);
        v9 = Cmd_Argv(7 * i + 9);
        statusIconIndex = atoi(v9);
        v10 = Cmd_Argv(7 * i + 10);
        cgameGlob->scores[i].kills = atoi(v10);
        v11 = Cmd_Argv(7 * i + 11);
        cgameGlob->scores[i].assists = atoi(v11);
        clientNum = cgameGlob->scores[i].client;
        if (!cgameGlob->bgs.clientinfo[clientNum].infoValid)
            Com_PrintError(14, "Invalid score client %i, bad scoreboard message\n", cgameGlob->scores[i].client);
        if (statusIconIndex > 0 && statusIconIndex <= 8)
        {
            pszIcon = CL_GetConfigString(localClientNum, statusIconIndex + 2258);
            cgameGlob->scores[i].hStatusIcon = Material_RegisterHandle(pszIcon, 7);
        }
        cgameGlob->scores[i].rank = cgameGlob->bgs.clientinfo[clientNum].rank;
        CL_GetRankIcon(
            cgameGlob->bgs.clientinfo[clientNum].rank,
            cgameGlob->bgs.clientinfo[clientNum].prestige,
            &cgameGlob->scores[i].hRankIcon);
        if (cgameGlob->scores[i].client >= 0x40u)
            cgameGlob->scores[i].client = 0;
        bcassert(cgameGlob->scores[0].client, MAX_CLIENTS);
        cgameGlob->bgs.clientinfo[cgameGlob->scores[i].client].score = cgameGlob->scores[i].score;
        bcassert(cgameGlob->scores[i].client, MAX_CLIENTS);
        if (cgameGlob->bgs.clientinfo[cgameGlob->scores[i].client].infoValid)
            team = cgameGlob->bgs.clientinfo[cgameGlob->scores[i].client].team;
        else
            team = TEAM_FREE;
        cgameGlob->scores[i].team = team;
        ++cgameGlob->teamPlayers[cgameGlob->scores[i].team];
        cgameGlob->teamPings[cgameGlob->scores[i].team] += cgameGlob->scores[i].ping;
    }
    for (ia = 0; ia < 4; ++ia)
    {
        if (cgameGlob->teamPlayers[ia] >= 1 && cgameGlob->teamPings[ia] >= 1)
            cgameGlob->teamPings[ia] /= cgameGlob->teamPlayers[ia];
        else
            cgameGlob->teamPings[ia] = 0;
    }
}

void __cdecl CG_SetSingleClientScore(int32_t localClientNum, int32_t clientIndex, int32_t newScore)
{
    bool foundScoreIndex; // [esp+3h] [ebp-9h]
    int32_t scoreIndex; // [esp+8h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    foundScoreIndex = 0;
    for (scoreIndex = 0; scoreIndex < cgameGlob->numScores; ++scoreIndex)
    {
        if (cgameGlob->scores[scoreIndex].client == clientIndex)
        {
            cgameGlob->scores[scoreIndex].score = newScore;
            foundScoreIndex = 1;
            break;
        }
    }
    if (foundScoreIndex)
    {
        CG_SortSingleClientScore(cgameGlob, scoreIndex);
    }
    else if (!cgameGlob->scoresRequestTime || cgameGlob->scoresRequestTime + 10000 < cgameGlob->time)
    {
        UpdateScores(localClientNum);
    }
}

void __cdecl CG_SortSingleClientScore(cg_s *cgameGlob, int32_t scoreIndex)
{
    score_t temp; // [esp+8h] [ebp-28h] BYREF

    while (scoreIndex > 0
        && CG_ClientScoreIsBetter(&cgameGlob->scores[scoreIndex], (score_t *)&cgameGlob->teamScores[10 * scoreIndex + 2]))
    {
        memcpy(&temp, &cgameGlob->teamScores[10 * scoreIndex + 2], sizeof(temp));
        memcpy(&cgameGlob->teamScores[10 * scoreIndex + 2], &cgameGlob->scores[scoreIndex], 0x28u);
        memcpy(&cgameGlob->scores[scoreIndex--], &temp, sizeof(cgameGlob->scores[scoreIndex--]));
    }
    while (scoreIndex < cgameGlob->numScores - 1
        && CG_ClientScoreIsBetter(&cgameGlob->scores[scoreIndex + 1], &cgameGlob->scores[scoreIndex]))
    {
        memcpy(&temp, &cgameGlob->scores[scoreIndex + 1], sizeof(temp));
        memcpy(
            &cgameGlob->scores[scoreIndex + 1],
            &cgameGlob->scores[scoreIndex],
            sizeof(cgameGlob->scores[scoreIndex + 1]));
        memcpy(&cgameGlob->scores[scoreIndex++], &temp, sizeof(cgameGlob->scores[scoreIndex++]));
    }
}

bool __cdecl CG_ClientScoreIsBetter(score_t *scoreA, score_t *scoreB)
{
    if (scoreA->team != scoreB->team && (scoreA->team == 3 || scoreB->team == 3))
        return 0;
    if (scoreA->score > scoreB->score)
        return 1;
    if (scoreA->score >= scoreB->score)
        return scoreA->deaths < scoreB->deaths;
    return 0;
}

void __cdecl CG_ConfigStringModified(int32_t localClientNum)
{
    const char *v1; // eax
    shellshock_parms_t *ShellshockParms; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-1Ch]
    int32_t time; // [esp+4h] [ebp-18h] BYREF
    int32_t serverId; // [esp+8h] [ebp-14h] BYREF
    const char *str; // [esp+Ch] [ebp-10h]
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    cgs_t *cgs; // [esp+14h] [ebp-8h]
    int32_t num; // [esp+18h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    v1 = Cmd_Argv(1);
    num = atoi(v1);
    str = CL_GetConfigString(localClientNum, num);
    switch (num)
    {
    case 2258:
        CG_SetupWeaponDef(localClientNum);
        break;
    case 2314:
        CG_RegisterItems(localClientNum);
        break;
    case 821:
        CG_StartAmbient(localClientNum);
        break;
    default:
        if (num)
        {
            if (num < 20 || num >= 276)
            {
                switch (num)
                {
                case 4:
                    cgameGlob->teamScores[1] = atoi(str);
                    break;
                case 5:
                    cgameGlob->teamScores[2] = atoi(str);
                    break;
                case 13:
                    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
                    cgs->voteTime = 0;
                    if (sscanf(str, "%d %d", &time, &serverId) == 2 && serverId == LocalClientGlobals->serverId)
                        cgs->voteTime = time;
                    break;
                case 15:
                    cgs->voteYes = atoi(str);
                    break;
                case 16:
                    cgs->voteNo = atoi(str);
                    break;
                case 14:
                    CG_UpdateVoteString(localClientNum, str);
                    break;
                case 12:
                    CL_ParseMapCenter(localClientNum);
                    break;
                case 11:
                    CG_ParseGameEndTime(localClientNum);
                    break;
                case 9:
                    CG_ParseFog(localClientNum);
                    break;
                default:
                    if (num < 830 || num >= 1342)
                    {
                        if (num < 1598 || num >= 1698)
                        {
                            if (num < 1954 || num >= 1970)
                            {
                                if (num >= 2259 && num < 2267 || num >= 2267 && num < 2282)
                                {
                                    Material_RegisterHandle(CL_GetConfigString(localClientNum, num), 7);
                                }
                                else if (num < 2002 || num >= 2258)
                                {
                                    switch (num)
                                    {
                                    case 822:
                                        CG_NorthDirectionChanged(localClientNum);
                                        break;
                                    case 823:
                                        CG_MiniMapChanged(localClientNum);
                                        break;
                                    case 824:
                                        CG_VisionSetConfigString_Naked(localClientNum);
                                        break;
                                    case 825:
                                        CG_VisionSetConfigString_Night(localClientNum);
                                        break;
                                    }
                                }
                                else
                                {
                                    CG_RegisterServerMaterial(localClientNum, num);
                                }
                            }
                            else if (*str && BG_LoadShellShockDvars(str))
                            {
                                ShellshockParms = BG_GetShellshockParms(num - 1954);
                                BG_SetShellShockParmsFromDvars(ShellshockParms);
                            }
                        }
                        else
                        {
                            cgs->fxs[num - CS_EFFECT_NAMES] = FX_Register(str);
                            iassert(cgs->fxs[num - CS_EFFECT_NAMES]);
                        }
                    }
                    else
                    {
                        *((uint32_t *)cgs + num - 665) = (uint32_t)R_RegisterModel(str); // KISAKTODO: unhack typing
                    }
                    break;
                }
            }
            else
            {
                CG_ParseCodInfo(localClientNum);
            }
        }
        else
        {
            CG_ParseServerInfo(localClientNum);
        }
        break;
    }
}

void __cdecl CG_UpdateVoteString(int32_t localClientNum, const char *rawVoteString)
{
    char *v2; // eax
    int32_t mapNameIndex; // [esp+0h] [ebp-120h]
    int32_t loadMapNameLength; // [esp+4h] [ebp-11Ch] BYREF
    int32_t srcIndex; // [esp+8h] [ebp-118h]
    cgs_t *cgs; // [esp+Ch] [ebp-114h]
    char voteStringWithLongMapNames[260]; // [esp+10h] [ebp-110h] BYREF
    const char *mapName; // [esp+118h] [ebp-8h]
    int32_t dstIndex; // [esp+11Ch] [ebp-4h]

    dstIndex = 0;
    for (srcIndex = 0; srcIndex < 256; ++srcIndex)
    {
        mapName = UI_GetMapDisplayNameFromPartialLoadNameMatch(&rawVoteString[srcIndex], &loadMapNameLength);
        if (mapName)
        {
            for (mapNameIndex = 0; srcIndex < 256 && mapName[mapNameIndex]; ++mapNameIndex)
                voteStringWithLongMapNames[dstIndex++] = mapName[mapNameIndex];
            srcIndex += loadMapNameLength;
        }
        voteStringWithLongMapNames[dstIndex++] = rawVoteString[srcIndex];
        if (!rawVoteString[srcIndex])
            break;
    }
    voteStringWithLongMapNames[255] = 0;
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    v2 = SEH_LocalizeTextMessage(voteStringWithLongMapNames, "vote string", LOCMSG_SAFE);
    I_strncpyz(cgs->voteString, v2, 256);
}

void __cdecl CG_AddToTeamChat(int32_t localClientNum, const char *str)
{
    char *ls; // [esp+8h] [ebp-18h]
    int32_t len; // [esp+Ch] [ebp-14h]
    char lastcolor; // [esp+14h] [ebp-Ch]
    char *p; // [esp+18h] [ebp-8h]
    char *pa; // [esp+18h] [ebp-8h]
    char *pb; // [esp+18h] [ebp-8h]
    int32_t chatHeight; // [esp+1Ch] [ebp-4h]
    const char *stra; // [esp+2Ch] [ebp+Ch]
    cgs_t *cgs;
    cg_s *cgameGlob;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    chatHeight = cg_chatHeight->current.integer;
    if (chatHeight && cg_chatTime->current.integer > 0)
    {
        len = 0;
        p = cgs->teamChatMsgs[cgs->teamChatPos % chatHeight];
        *p = 0;
        lastcolor = 55;
        ls = 0;
        while (*str)
        {
            if (len > 52)
            {
                if (ls)
                {
                    str = &str[-(p - ls) + 1];
                    p = ls;
                }
                *p = 0;
                cgs->teamChatMsgTimes[cgs->teamChatPos++ % chatHeight] = cgameGlob->time;
                pa = cgs->teamChatMsgs[cgs->teamChatPos % chatHeight];
                *pa = 0;
                *pa++ = 94;
                *pa = lastcolor;
                p = pa + 1;
                len = 0;
                ls = 0;
            }
            if (str && *str == 94 && str[1] && str[1] != 94 && str[1] >= 48 && str[1] <= 57)
            {
                *p = *str;
                pb = p + 1;
                stra = str + 1;
                lastcolor = *stra;
                *pb = *stra;
                p = pb + 1;
                str = stra + 1;
            }
            else
            {
                if (*str == 32)
                    ls = p;
                *p++ = *str++;
                ++len;
            }
        }
        *p = 0;
        cgs->teamChatMsgTimes[cgs->teamChatPos++ % chatHeight] = cgameGlob->time;
        if (cgs->teamChatPos - cgs->teamLastChatPos > chatHeight)
            cgs->teamLastChatPos = cgs->teamChatPos - chatHeight;
    }
    else
    {
        cgs->teamLastChatPos = 0;
        cgs->teamChatPos = 0;
    }
}

void __cdecl CG_OpenScriptMenu(int32_t localClientNum)
{
    int32_t v4; // eax
    const char *v5; // eax
    const char *menuName; // [esp+10h] [ebp-10h]
    uint32_t menuIndex; // [esp+14h] [ebp-Ch]
    bool useMouse; // [esp+1Bh] [ebp-5h]

    menuIndex = atoi(Cmd_Argv(1));
    if (menuIndex >= 0x20)
    {
        Com_Printf(14, "Server tried to open a bad script menu index: %i\n", menuIndex);
        Cbuf_AddText(localClientNum, va("cmd mr %i bad\n", menuIndex));
        return;
    }
    menuName = CL_GetConfigString(localClientNum, menuIndex + 1970);
    if (!*menuName)
    {
        Com_Printf(14, "Server tried to open a non-loaded script menu index: %i\n", menuIndex);
        Cbuf_AddText(localClientNum, va("cmd mr %i bad\n", menuIndex));
        return;
    }
    if (Cmd_Argc() > 2 && Cmd_Argv(2) && *Cmd_Argv(2))
    {
        useMouse = 0;
        v4 = UI_PopupScriptMenu(localClientNum, menuName, 0);
    }
    else
    {
        useMouse = 1;
        v4 = UI_PopupScriptMenu(localClientNum, menuName, 1);
    }
    if (!v4)
    {
        if (cg_waitingScriptMenu[localClientNum].name[0])
        {
            if (!I_stricmp(menuName, cg_waitingScriptMenu[localClientNum].name))
                return;
            v5 = va("cmd mr %i noop\n", cg_waitingScriptMenu[localClientNum].index);
            Cbuf_AddText(localClientNum, v5);
        }
        if (strlen(menuName) >= 0x40)
            MyAssertHandler(
                ".\\cgame_mp\\cg_servercmds_mp.cpp",
                1108,
                0,
                "%s\n\t(menuName) = %s",
                "(I_strlen( menuName ) < (sizeof( cg_waitingScriptMenu[localClientNum].name ) / (sizeof( cg_waitingScriptMenu[loc"
                "alClientNum].name[0] ) * (sizeof( cg_waitingScriptMenu[localClientNum].name ) != 4 || sizeof( cg_waitingScriptMe"
                "nu[localClientNum].name[0] ) <= 4))))",
                menuName);
        I_strncpyz(cg_waitingScriptMenu[localClientNum].name, menuName, 64);
        cg_waitingScriptMenu[localClientNum].index = menuIndex;
        cg_waitingScriptMenu[localClientNum].useMouse = useMouse;
    }
}

void __cdecl CG_RemoveChatEscapeChar(char *text)
{
    int32_t l; // [esp+0h] [ebp-8h]
    int32_t i; // [esp+4h] [ebp-4h]

    l = 0;
    for (i = 0; text[i]; ++i)
    {
        if (text[i] != 25)
            text[l++] = text[i];
    }
    text[l] = 0;
}

void __cdecl CG_SetTeamScore(int32_t localClientNum, uint32_t team, int32_t score)
{
    iassert(team >= 0 && team < TEAM_NUM_TEAMS);
    CG_GetLocalClientGlobals(localClientNum)->teamScores[team] = score;
}

void CG_ReverbCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int32_t fademsec; // [esp+Ch] [ebp-30h]
    float v5; // [esp+14h] [ebp-28h]
    const char *roomstring; // [esp+24h] [ebp-18h]
    float drylevel; // [esp+28h] [ebp-14h]
    float fadetime; // [esp+2Ch] [ebp-10h]
    float wetlevel; // [esp+30h] [ebp-Ch]
    int32_t prio; // [esp+34h] [ebp-8h]
    int32_t argc; // [esp+38h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc == 6)
    {
        v0 = Cmd_Argv(1);
        prio = atoi(v0);
        v1 = Cmd_Argv(3);
        drylevel = atof(v1);
        v2 = Cmd_Argv(4);
        wetlevel = atof(v2);
        v3 = Cmd_Argv(5);
        fadetime = atof(v3);
        roomstring = Cmd_Argv(2);
        if ((int)(fadetime * 1000.0f) > 0)
            fademsec = (int)(fadetime * 1000.0f);
        else
            fademsec = 0;
        SND_SetEnvironmentEffects(prio, roomstring, drylevel, wetlevel, fademsec);
    }
    else
    {
        Com_PrintError(14, "ERROR: CG_ReverbCmd called with %i args (should be 6)\n", argc);
    }
}

void CG_DeactivateReverbCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    int32_t v2; // [esp+4h] [ebp-20h]
    float v3; // [esp+8h] [ebp-1Ch]
    float fadetime; // [esp+18h] [ebp-Ch]
    int32_t prio; // [esp+1Ch] [ebp-8h]
    int32_t argc; // [esp+20h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc == 3)
    {
        v0 = Cmd_Argv(1);
        prio = atoi(v0);
        v1 = Cmd_Argv(2);
        fadetime = atof(v1);
        v2 = (int)(fadetime * 1000.0f);
        if (v2 > 0)
            SND_DeactivateEnvironmentEffects(prio, v2);
        else
            SND_DeactivateEnvironmentEffects(prio, 0);
    }
    else
    {
        Com_PrintError(14, "ERROR: CG_DeactivateReverbCmd called with %i args (should be 3)\n", argc);
    }
}

void __cdecl CG_SetChannelVolCmd(int32_t localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    shellshock_parms_t *ShellshockParms; // eax
    int32_t fademsec; // [esp+0h] [ebp-2Ch]
    float v6; // [esp+8h] [ebp-24h]
    float fadetime; // [esp+18h] [ebp-14h]
    uint32_t shockIndex; // [esp+20h] [ebp-Ch]
    int32_t prio; // [esp+24h] [ebp-8h]
    int32_t argc; // [esp+28h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc == 4)
    {
        v1 = Cmd_Argv(1);
        prio = atoi(v1);
        v2 = Cmd_Argv(2);
        shockIndex = atoi(v2);
        v3 = Cmd_Argv(3);
        fadetime = atof(v3);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if ((int)(fadetime * 1000.0f) > 0)
            fademsec = (int)(fadetime * 1000.0f);
        else
            fademsec = 0;
        ShellshockParms = BG_GetShellshockParms(shockIndex);
        SND_SetChannelVolumes(prio, ShellshockParms->sound.channelvolume, fademsec);
    }
    else
    {
        Com_PrintError(9, "ERROR: CG_SetChannelVolCmd called with %i args (should be 4)\n", argc);
    }
}

void CG_DeactivateChannelVolCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    int32_t v2; // [esp+4h] [ebp-20h]
    float v3; // [esp+8h] [ebp-1Ch]
    float fadetime; // [esp+18h] [ebp-Ch]
    int32_t prio; // [esp+1Ch] [ebp-8h]
    int32_t argc; // [esp+20h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc == 3)
    {
        v0 = Cmd_Argv(1);
        prio = atoi(v0);
        v1 = Cmd_Argv(2);
        fadetime = atof(v1);
        v2 = (int)(fadetime * 1000.0f);
        if (v2 > 0)
            SND_DeactivateChannelVolumes(prio, v2);
        else
            SND_DeactivateChannelVolumes(prio, 0);
    }
    else
    {
        Com_PrintError(9, "ERROR: CG_DeactivateChannelVolCmd called with %i args (should be 3)\n", argc);
    }
}

char __cdecl LocalSound(int32_t localClientNum)
{
    const char *aliasName; // [esp+0h] [ebp-Ch]
    int32_t index; // [esp+4h] [ebp-8h]
    int32_t argc; // [esp+8h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc == 2)
    {
        index = atoi(Cmd_Argv(1));
        if (index > 0 && index <= 256)
        {
            aliasName = CL_GetConfigString(localClientNum, index + 1342);
            CG_PlayClientSoundAliasByName(localClientNum, aliasName);
            return 1;
        }
        else
        {
            Com_PrintError(9, "ERROR: LocalSound() called with index %i (should be in range[1,%i])\n", index, 256);
            return 0;
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: LocalSound() called with %i args (should be 2)\n", argc);
        return 0;
    }
}

void __cdecl LocalSoundStop(int32_t localClientNum)
{
    const char *aliasName; // [esp+0h] [ebp-8h]
    int32_t index; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        index = atoi(Cmd_Argv(1));
        if (index > 0 && index <= 256)
        {
            aliasName = CL_GetConfigString(localClientNum, index + 1342);
            CG_StopClientSoundAliasByName(localClientNum, aliasName);
        }
        else
        {
            Com_PrintError(9, "ERROR: LocalSoundStop() called with index %i (should be in range[1,%i])\n", index, 256);
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: LocalSoundStop(), should be called with 2 arguments.\n");
    }
}

void __cdecl CG_SetClientDvarFromServer(cg_s *cgameGlob, const char *dvarname, char *value)
{
    uint32_t v3; // eax

    if (I_stricmp(dvarname, "cg_objectiveText"))
    {
        if (I_stricmp(dvarname, "hud_drawHud"))
        {
            if (I_stricmp(dvarname, "g_scriptMainMenu"))
                Dvar_SetFromStringByName(dvarname, value);
            else
                CG_SetScriptMainMenu(cgameGlob, value);
        }
        else
        {
            v3 = atoi(value);
            CG_SetDrawHud(cgameGlob, v3);
        }
    }
    else
    {
        CG_SetObjectiveText(cgameGlob, value);
    }
}

void __cdecl CG_SetObjectiveText(cg_s *cgameGlob, char *text)
{
    I_strncpyz(cgameGlob->objectiveText, text, 1024);
}

void __cdecl CG_SetDrawHud(cg_s *cgameGlob, uint32_t value)
{
    if (value > 1)
        MyAssertHandler(
            ".\\cgame_mp\\cg_servercmds_mp.cpp",
            1464,
            0,
            "%s\n\t(value) = %i",
            "((value == 0 || value == 1))",
            value);
    cgameGlob->drawHud = value;
}

void __cdecl CG_SetScriptMainMenu(cg_s *cgameGlob, char *text)
{
    if (!text)
        MyAssertHandler(".\\cgame_mp\\cg_servercmds_mp.cpp", 1471, 0, "%s", "text");
    I_strncpyz(cgameGlob->scriptMainMenu, text, 256);
}

void __cdecl CG_ExecuteNewServerCommands(int32_t localClientNum, int32_t latestSequence)
{
    int32_t nesting; // [esp+4h] [ebp-4h]
    cgs_t *cgs;

    nesting = cmd_args.nesting;
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    while (cgs->serverCommandSequence < latestSequence)
    {
        iassert(CG_GetLocalClientGlobals(localClientNum)->nextSnap);

        if (CL_CGameNeedsServerCommand(localClientNum, ++cgs->serverCommandSequence))
            CG_ServerCommand(localClientNum);

        iassert(nesting == cmd_args.nesting);
    }
}