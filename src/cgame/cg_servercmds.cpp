#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_servercmds.h"
#include <client/client.h>
#include <qcommon/com_bsp.h>
#include <gfx_d3d/r_dpvs.h>

void __cdecl CG_ParseServerInfo(int localClientNum)
{
    const char *ConfigString; // r3
    const char *v3; // r30

    ConfigString = CL_GetConfigString(localClientNum, 0);
    v3 = Info_ValueForKey(ConfigString, "mapname");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Com_GetBspFilename(cgsArray[0].mapname, 64, v3);
}

void __cdecl CG_ParseCullDist(int localClientNum)
{
    const char *ConfigString; // r3
    long double v2; // fp2

    ConfigString = CL_GetConfigString(localClientNum, 6u);
    v2 = atof(ConfigString);
    R_SetCullDist((float)*(double *)&v2);
}

void __cdecl CG_ParseSunLight(int localClientNum)
{
    const char *ConfigString; // r3
    int argCount; // r8
    float sunColor[3]; // [sp+50h] [-20h] BYREF

    ConfigString = CL_GetConfigString(localClientNum, 7);
    if (*ConfigString)
    {
        argCount = sscanf(ConfigString, "%g %g %g", &sunColor[0], &sunColor[1], &sunColor[2]);
        iassert(argCount == 3);
        R_SetSunLightOverride(sunColor);
    }
    else
    {
        R_ResetSunLightOverride();
    }
}

void __cdecl CG_ParseSunDirection(int localClientNum)
{
    const char *ConfigString; // r3
    int v2; // r3
    int v3; // [sp+8h] [-88h]
    int v4; // [sp+Ch] [-84h]
    int v5; // [sp+60h] [-30h]
    int v6; // [sp+64h] [-2Ch]
    float v7; // [sp+68h] [-28h] BYREF
    char v8; // [sp+6Ch] [-24h] BYREF
    char v9; // [sp+70h] [-20h] BYREF
    float v10; // [sp+78h] [-18h] BYREF
    char v11; // [sp+7Ch] [-14h] BYREF
    char v12; // [sp+80h] [-10h] BYREF

    ConfigString = CL_GetConfigString(localClientNum, 8u);
    if (*ConfigString)
    {
        v2 = sscanf(ConfigString, "%g %g %g %g %g %g %d %d", &v7, &v8, &v9, &v10, &v11, &v12, v3, v4);
        if (v2 == 3)
        {
            R_SetSunDirectionOverride(&v7);
        }
        else
        {
            if (v2 != 8)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
                    112,
                    0,
                    "%s\n\t(argCount) = %i",
                    "(argCount == 3 || argCount == 8)",
                    v2);
            R_LerpSunDirectionOverride(&v7, &v10, v6, v5);
        }
    }
    else
    {
        R_ResetSunDirectionOverride();
    }
}

void __cdecl CG_ParseFog(int time)
{
    int nesting; // r7
    const char *v3; // r3
    long double v4; // fp2
    int v5; // r7
    double v6; // fp28
    char *v7; // r3
    long double v8; // fp2
    double v9; // fp29
    const char *v10; // r3
    long double v11; // fp2
    long double v12; // fp2
    unsigned __int8 v13; // r31
    const char *v14; // r3
    long double v15; // fp2
    long double v16; // fp2
    unsigned __int8 v17; // r30
    const char *v18; // r3
    long double v19; // fp2
    const char *v20; // r3
    int v21; // r3
    unsigned __int8 v22; // r4
    int v23; // r31
    int v24; // r5
    int v25; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    v4 = atof(v3);
    v5 = cmd_args.nesting;
    v6 = (float)*(double *)&v4;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        v5 = cmd_args.nesting;
    }
    if (cmd_args.argc[v5] <= 2)
    {
        v7 = byte_82003CDD;
    }
    else
    {
        v7 = (char *)*((unsigned int *)cmd_args.argv[v5] + 2);
        if (!v7)
        {
        LABEL_13:
            v25 = 0;
            v24 = (int)v6;
            goto LABEL_14;
        }
    }
    if (!*v7)
        goto LABEL_13;
    v8 = atof(v7);
    v9 = (float)*(double *)&v8;
    v10 = Cmd_Argv(3);
    v11 = atof(v10);
    *(double *)&v11 = (float)((float)((float)*(double *)&v11 * (float)255.0) + (float)0.5);
    v12 = floor(v11);
    v13 = (int)(float)*(double *)&v12;
    v14 = Cmd_Argv(4);
    v15 = atof(v14);
    *(double *)&v15 = (float)((float)((float)*(double *)&v15 * (float)255.0) + (float)0.5);
    v16 = floor(v15);
    v17 = (int)(float)*(double *)&v16;
    v18 = Cmd_Argv(5);
    v19 = atof(v18);
    *(double *)&v19 = (float)((float)((float)*(double *)&v19 * (float)255.0) + (float)0.5);
    floor(v19);
    v20 = Cmd_Argv(6);
    v21 = atol(v20);
    v22 = v13;
    v23 = v21;
    R_SetFogFromServer(v6, v21, v22, v17, v9);
    v24 = v23;
    v25 = 1;
LABEL_14:
    R_SwitchFog(v25, time, v24);
}

void __cdecl CG_PrecacheScriptMenu(int localClientNum, int iConfigNum)
{
    const char *ConfigString; // r3
    const char *v5; // r31

    if (iConfigNum < 2551 || iConfigNum >= 2583)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
            165,
            0,
            "%s",
            "(iConfigNum >= CS_SCRIPT_MENUS) && (iConfigNum < CS_SCRIPT_MENUS + MAX_SCRIPT_MENUS)");
    ConfigString = CL_GetConfigString(localClientNum, iConfigNum);
    v5 = ConfigString;
    if (*ConfigString)
    {
        if (!Load_ScriptMenu(ConfigString, 7))
            Com_Error(ERR_DROP, byte_8200EEDC, v5);
    }
}

void __cdecl CG_RegisterServerMaterial(int localClientNum, int num)
{
    const char *ConfigString; // r3

    if (num < 2583 || num >= 2711)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
            180,
            0,
            "%s\n\t(num) = %i",
            "(num >= CS_SERVER_MATERIALS && num < CS_SERVER_MATERIALS + 128)",
            num);
    ConfigString = CL_GetConfigString(localClientNum, num);
    if (*ConfigString)
        Material_RegisterHandle(ConfigString, 7);
}

void __cdecl CG_RegisterServerMaterials(int localClientNum)
{
    signed int i; // r31
    const char *ConfigString; // r3

    for (i = 2584; i < 2711; ++i)
    {
        if (i < 2583)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
                180,
                0,
                "%s\n\t(num) = %i",
                "(num >= CS_SERVER_MATERIALS && num < CS_SERVER_MATERIALS + 128)",
                i);
        ConfigString = CL_GetConfigString(localClientNum, i);
        if (*ConfigString)
            Material_RegisterHandle(ConfigString, 7);
    }
}

void __cdecl CG_ConfigStringModifiedInternal(int localClientNum, unsigned int stringIndex)
{
    const char *ConfigString; // r3
    const char *v5; // r29
    const char *v6; // r3
    long double v7; // fp2
    cgs_t *LocalClientStaticGlobals; // r30
    const FxEffectDef *v9; // r3
    shellshock_parms_t *ShellshockParms; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ConfigString = CL_GetConfigString(localClientNum, stringIndex);
    v5 = ConfigString;
    if (stringIndex == 2711)
    {
        CG_RegisterItems(localClientNum);
    }
    else if (stringIndex == 1114)
    {
        CG_StartAmbient(localClientNum);
    }
    else if (stringIndex)
    {
        switch (stringIndex)
        {
        case 6u:
            v6 = CL_GetConfigString(localClientNum, 6u);
            v7 = atof(v6);
            R_SetCullDist((float)*(double *)&v7);
            break;
        case 7u:
            CG_ParseSunLight(localClientNum);
            break;
        case 8u:
            CG_ParseSunDirection(localClientNum);
            break;
        default:
            if (stringIndex - 1155 > 0x1FF)
            {
                if (stringIndex - 2179 > 0x63)
                {
                    if (stringIndex - 2535 > 0xF)
                    {
                        if (stringIndex - 11 > 0xF)
                        {
                            if (stringIndex - 2583 > 0x7F)
                            {
                                if (stringIndex == 1147)
                                {
                                    CG_NorthDirectionChanged(localClientNum);
                                }
                                else if (stringIndex == 1148)
                                {
                                    CG_MiniMapChanged(localClientNum);
                                }
                                else if (stringIndex - 27 > 0x1F)
                                {
                                    if (stringIndex == 1149)
                                    {
                                        CG_VisionSetConfigString_Naked(localClientNum);
                                    }
                                    else if (stringIndex == 1150)
                                    {
                                        CG_VisionSetConfigString_Night(localClientNum);
                                    }
                                }
                                else
                                {
                                    CG_TargetsChanged(localClientNum, stringIndex);
                                }
                            }
                            else
                            {
                                CG_RegisterServerMaterial(localClientNum, stringIndex);
                            }
                        }
                        else
                        {
                            CG_ParseObjectiveChange(localClientNum, stringIndex);
                        }
                    }
                    else if (*ConfigString && BG_LoadShellShockDvars(ConfigString))
                    {
                        CG_GetLocalClientStaticGlobals(localClientNum);
                        ShellshockParms = BG_GetShellshockParms(stringIndex - 2535);
                        BG_SetShellShockParmsFromDvars(ShellshockParms);
                    }
                }
                else
                {
                    LocalClientStaticGlobals = CG_GetLocalClientStaticGlobals(localClientNum);
                    v9 = FX_Register(v5);
                    *((unsigned int *)&LocalClientStaticGlobals[-7] + stringIndex - 211) = v9;
                    if (!v9)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
                            241,
                            0,
                            "%s",
                            "cgs->fxs[stringIndex - CS_EFFECT_NAMES]");
                }
            }
            else
            {
                R_RegisterModel(ConfigString);
            }
            break;
        }
    }
    else
    {
        CG_ParseServerInfo(localClientNum);
    }
}

void __cdecl CG_ConfigStringModified(int localClientNum)
{
    int nesting; // r7
    const char *v3; // r3
    unsigned int v4; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    v4 = atol(v3);
    CG_ConfigStringModifiedInternal(localClientNum, v4);
}

void __cdecl CG_ShutdownPhysics(int localClientNum)
{
    DynEntCl_Shutdown(localClientNum);
    FX_KillAllEffects(localClientNum);
    Phys_Shutdown();
}

void __cdecl CG_OpenScriptMenu(int localClientNum)
{
    int nesting; // r7
    const char *v3; // r3
    unsigned int v4; // r3
    int v5; // r29
    const char *ConfigString; // r30
    bool v7; // r27
    const char *v8; // r3
    const char *v9; // r11
    const char *v11; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    v4 = atol(v3);
    v5 = v4;
    if (v4 > 0x1F)
    {
        Com_Printf(14, "Server tried to open a bad script menu index: %i\n", v4);
        goto LABEL_23;
    }
    ConfigString = CL_GetConfigString(localClientNum, v4 + 2551);
    if (!*ConfigString)
    {
        Com_Printf(14, "Server tried to open a non-loaded script menu index: %i\n", v5);
    LABEL_23:
        v11 = va("cmd mr %i bad\n", v5);
        Cbuf_AddText(localClientNum, v11);
        return;
    }
    if (Cmd_Argc() <= 2 || !Cmd_Argv(2) || (v7 = 0, !*Cmd_Argv(2)))
        v7 = 1;
    if (!UI_PopupScriptMenu(ConfigString, v7))
    {
        if (cg_waitingScriptMenu.name[0])
        {
            if (!I_stricmp(ConfigString, cg_waitingScriptMenu.name))
                return;
            v8 = va("cmd mr %i noop\n", cg_waitingScriptMenu.index);
            Cbuf_AddText(localClientNum, v8);
        }
        v9 = ConfigString;
        while (*(unsigned __int8 *)v9++)
            ;
        if ((unsigned int)(v9 - ConfigString - 1) >= 0x40)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
                421,
                0,
                "%s\n\t(menuName) = %s",
                "(I_strlen( menuName ) < (sizeof( cg_waitingScriptMenu.name ) / (sizeof( cg_waitingScriptMenu.name[0] ) * (sizeof"
                "( cg_waitingScriptMenu.name ) != 4 || sizeof( cg_waitingScriptMenu.name[0] ) <= 4))))",
                ConfigString);
        I_strncpyz(cg_waitingScriptMenu.name, ConfigString, 64);
        cg_waitingScriptMenu.index = v5;
        cg_waitingScriptMenu.useMouse = v7;
    }
}

void __cdecl CG_CheckOpenWaitingScriptMenu()
{
    if (cg_waitingScriptMenu.name[0])
    {
        if (UI_PopupScriptMenu(cg_waitingScriptMenu.name, cg_waitingScriptMenu.useMouse))
            cg_waitingScriptMenu.name[0] = 0;
    }
}

void __cdecl CG_CloseScriptMenu(bool allowResponse)
{
    UI_ClosePopupScriptMenu(0, allowResponse);
    cg_waitingScriptMenu.name[0] = 0;
}

void __cdecl CG_MenuShowNotify(int localClientNum, int menuToShow)
{
    int time; // r10
    const char *v4; // r4
    int v5; // r9
    int v6; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    switch (menuToShow)
    {
    case 0:
        time = cgArray[0].time;
        if (cgArray[0].healthFadeTime < cgArray[0].time)
        {
            v4 = "Health";
            v5 = 171444;
            goto LABEL_18;
        }
        break;
    case 1:
    case 4:
        v6 = cgArray[0].time;
        if (cgArray[0].ammoFadeTime < cgArray[0].time)
        {
            cgArray[0].ammoFadeTime = cgArray[0].time;
            Menus_ShowByName(&cgDC, "weaponinfo");
            Menus_ShowByName(&cgDC, "weaponinfo_lowdef");
            v6 = cgArray[0].time;
        }
        if (cgArray[0].offhandFadeTime < v6)
        {
            v4 = "offhandinfo";
            cgArray[0].offhandFadeTime = v6;
            goto LABEL_19;
        }
        break;
    case 2:
        time = cgArray[0].time;
        if (cgArray[0].compassFadeTime < cgArray[0].time)
        {
            v4 = "Compass";
            v5 = 171440;
            goto LABEL_18;
        }
        break;
    case 3:
        time = cgArray[0].time;
        if (cgArray[0].stanceFadeTime < cgArray[0].time)
        {
            v4 = "stance";
            v5 = 171452;
            goto LABEL_18;
        }
        break;
    case 5:
        time = cgArray[0].time;
        if (cgArray[0].scoreFadeTime < cgArray[0].time)
        {
            v4 = "objectiveinfo";
            v5 = 170992;
            goto LABEL_18;
        }
        break;
    case 6:
        time = cgArray[0].time;
        if (cgArray[0].sprintFadeTime < cgArray[0].time)
        {
            v4 = "sprintMeter";
            v5 = 171456;
        LABEL_18:
            *(int *)((char *)&cgArray[0].clientNum + v5) = time;
        LABEL_19:
            Menus_ShowByName(&cgDC, v4);
        }
        break;
    default:
        return;
    }
}

void __cdecl CG_HudMenuShowAllTimed(int localClientNum)
{
    int time; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    time = cgArray[0].time;
    if (cgArray[0].healthFadeTime < cgArray[0].time)
    {
        cgArray[0].healthFadeTime = cgArray[0].time;
        Menus_ShowByName(&cgDC, "Health");
        time = cgArray[0].time;
    }
    if (cgArray[0].ammoFadeTime < time)
    {
        cgArray[0].ammoFadeTime = time;
        Menus_ShowByName(&cgDC, "weaponinfo");
        Menus_ShowByName(&cgDC, "weaponinfo_lowdef");
        time = cgArray[0].time;
    }
    if (cgArray[0].compassFadeTime < time)
    {
        cgArray[0].compassFadeTime = time;
        Menus_ShowByName(&cgDC, "Compass");
        time = cgArray[0].time;
    }
    if (cgArray[0].stanceFadeTime < time)
    {
        cgArray[0].stanceFadeTime = time;
        Menus_ShowByName(&cgDC, "stance");
        time = cgArray[0].time;
    }
    if (cgArray[0].sprintFadeTime < time)
    {
        cgArray[0].sprintFadeTime = time;
        Menus_ShowByName(&cgDC, "sprintMeter");
        time = cgArray[0].time;
    }
    if (cgArray[0].offhandFadeTime < time)
    {
        cgArray[0].offhandFadeTime = time;
        Menus_ShowByName(&cgDC, "offhandinfo");
        time = cgArray[0].time;
    }
    if (cgArray[0].scoreFadeTime < time)
    {
        cgArray[0].scoreFadeTime = time;
        Menus_ShowByName(&cgDC, "objectiveinfo");
    }
}

void CG_EqCommand()
{
    int nesting; // r7
    int v1; // r5
    const char *v2; // r31
    const char *v3; // r3
    int v4; // r30
    const char *v5; // r3
    int v6; // r29
    const char *v7; // r3
    SND_EQTYPE v8; // r28
    const char *v9; // r3
    long double v10; // fp2
    double v11; // fp31
    const char *v12; // r3
    long double v13; // fp2
    double v14; // fp30
    const char *v15; // r3
    long double v16; // fp2

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
    v1 = cmd_args.argc[nesting];
    if (v1 == 8)
    {
        v2 = Cmd_Argv(1);
        v3 = Cmd_Argv(2);
        v4 = atol(v3);
        v5 = Cmd_Argv(3);
        v6 = atol(v5);
        v7 = Cmd_Argv(4);
        v8 = atol(v7);
        v9 = Cmd_Argv(5);
        v10 = atof(v9);
        v11 = (float)*(double *)&v10;
        v12 = Cmd_Argv(6);
        v13 = atof(v12);
        v14 = (float)*(double *)&v13;
        v15 = Cmd_Argv(7);
        v16 = atof(v15);
        SND_SetEq(v2, v4, v6, v8, v11, v14, (float)*(double *)&v16);
    }
    else
    {
        Com_PrintError(14, "ERROR: CG_EqCommand called with %i args (should be 8)\n", v1);
    }
}

void CG_DeactivateEqCmd()
{
    int nesting; // r7
    int v1; // r31
    const char *v2; // r3
    int v3; // r3
    int v4; // r30
    const char *v5; // r3
    int v6; // r31
    const char *v7; // r3
    const char *v8; // r3

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
    v1 = cmd_args.argc[nesting];
    if ((unsigned int)(v1 - 2) > 2)
    {
        Com_PrintError(14, "ERROR: CG_DeactivateEqCmd called with %i args (should >= 2 and <= 4)\n", v1);
    }
    else
    {
        v2 = Cmd_Argv(1);
        v3 = atol(v2);
        v4 = v3;
        switch (v1)
        {
        case 2:
            SND_DeactivateAllEq(v3);
            break;
        case 3:
            v8 = Cmd_Argv(2);
            SND_DeactivateChannelEq(v8, v4);
            break;
        case 4:
            v5 = Cmd_Argv(3);
            v6 = atol(v5);
            v7 = Cmd_Argv(2);
            SND_DeactivateEq(v7, v4, v6);
            break;
        }
    }
}

void CG_ReverbCmd()
{
    int nesting; // r7
    int v1; // r5
    const char *v2; // r3
    int v3; // r31
    const char *v4; // r3
    long double v5; // fp2
    double v6; // fp30
    const char *v7; // r3
    long double v8; // fp2
    double v9; // fp29
    const char *v10; // r3
    long double v11; // fp2
    double v12; // fp31
    const char *v13; // r30
    long double v14; // fp2
    int v15; // r5

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
    v1 = cmd_args.argc[nesting];
    if (v1 == 6)
    {
        v2 = Cmd_Argv(1);
        v3 = atol(v2);
        v4 = Cmd_Argv(3);
        v5 = atof(v4);
        v6 = (float)*(double *)&v5;
        v7 = Cmd_Argv(4);
        v8 = atof(v7);
        v9 = (float)*(double *)&v8;
        v10 = Cmd_Argv(5);
        v11 = atof(v10);
        v12 = (float)*(double *)&v11;
        v13 = Cmd_Argv(2);
        *(double *)&v14 = (float)((float)((float)v12 * (float)1000.0) + (float)0.5);
        floor(v14);
        SND_SetEnvironmentEffects(v3, v13, v6, v9, v15);
    }
    else
    {
        Com_PrintError(14, "ERROR: CG_ReverbCmd called with %i args (should be 6)\n", v1);
    }
}

void CG_DeactivateReverbCmd()
{
    int nesting; // r7
    int v1; // r5
    const char *v2; // r3
    int v3; // r31
    const char *v4; // r3
    long double v5; // fp2
    long double v6; // fp2
    int v7; // r11

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
    v1 = cmd_args.argc[nesting];
    if (v1 == 3)
    {
        v2 = Cmd_Argv(1);
        v3 = atol(v2);
        v4 = Cmd_Argv(2);
        v5 = atof(v4);
        *(double *)&v5 = (float)((float)((float)*(double *)&v5 * (float)1000.0) + (float)0.5);
        v6 = floor(v5);
        v7 = (int)(float)*(double *)&v6;
        if (v7 <= 0)
            v7 = 0;
        SND_DeactivateEnvironmentEffects(v3, v7);
    }
    else
    {
        Com_PrintError(14, "ERROR: CG_DeactivateReverbCmd called with %i args (should be 3)\n", v1);
    }
}

void __cdecl CG_SetChannelVolCmd(int localClientNum)
{
    int nesting; // r7
    int v3; // r5
    const char *v4; // r3
    int v5; // r29
    const char *v6; // r3
    int v7; // r28
    const char *v8; // r3
    long double v9; // fp2
    double v10; // fp31
    long double v11; // fp2
    int v12; // r31
    shellshock_parms_t *ShellshockParms; // r3

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
    v3 = cmd_args.argc[nesting];
    if (v3 == 4)
    {
        v4 = Cmd_Argv(1);
        v5 = atol(v4);
        v6 = Cmd_Argv(2);
        v7 = atol(v6);
        v8 = Cmd_Argv(3);
        v9 = atof(v8);
        v10 = (float)*(double *)&v9;
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                917,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        *(double *)&v9 = (float)((float)((float)v10 * (float)1000.0) + (float)0.5);
        v11 = floor(v9);
        v12 = (int)(float)*(double *)&v11;
        if (v12 <= 0)
            v12 = 0;
        ShellshockParms = BG_GetShellshockParms(v7);
        SND_SetChannelVolumes(v5, (const float *)&ShellshockParms->sound[292], v12);
    }
    else
    {
        Com_PrintError(14, "ERROR: CG_SetChannelVolCmd called with %i args (should be 4)\n", v3);
    }
}

void CG_DeactivateChannelVolCmd()
{
    int nesting; // r7
    int v1; // r5
    const char *v2; // r3
    int v3; // r31
    const char *v4; // r3
    long double v5; // fp2
    long double v6; // fp2
    int v7; // r11

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
    v1 = cmd_args.argc[nesting];
    if (v1 == 3)
    {
        v2 = Cmd_Argv(1);
        v3 = atol(v2);
        v4 = Cmd_Argv(2);
        v5 = atof(v4);
        *(double *)&v5 = (float)((float)((float)*(double *)&v5 * (float)1000.0) + (float)0.5);
        v6 = floor(v5);
        v7 = (int)(float)*(double *)&v6;
        if (v7 <= 0)
            v7 = 0;
        SND_DeactivateChannelVolumes(v3, v7);
    }
    else
    {
        Com_PrintError(9, "ERROR: CG_DeactivateChannelVolCmd called with %i args (should be 3)\n", v1);
    }
}

void __cdecl LocalSound(int localClientNum)
{
    int nesting; // r7
    int v3; // r31
    const char *v4; // r3
    int v5; // r3
    const char *ConfigString; // r3
    int v7; // r30
    const char *v8; // r3
    void *v9; // r3

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
    v3 = cmd_args.argc[nesting];
    if (v3 >= 2)
    {
        v4 = Cmd_Argv(1);
        v5 = atol(v4);
        if ((unsigned int)(v5 - 1) > 0x1FF)
        {
            Com_PrintError(9, "ERROR: LocalSound() called with index %i (should be in range[1,%i])\n", v5, 512);
        }
        else
        {
            ConfigString = CL_GetConfigString(localClientNum, v5 + 1667);
            v7 = CG_PlayClientSoundAliasByName(localClientNum, ConfigString);
            if (v3 > 2)
            {
                v8 = Cmd_Argv(2);
                v9 = (void *)atol(v8);
                SND_AddLengthNotify(v7, v9, SndLengthNotify_Script);
            }
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: LocalSound() called with %i args (should be >= 2)\n", v3);
    }
}

void __cdecl LocalSoundStop(int localClientNum)
{
    int nesting; // r7
    const char *v3; // r3
    int v4; // r3
    const char *ConfigString; // r3

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
    if (cmd_args.argc[nesting] == 2)
    {
        v3 = Cmd_Argv(1);
        v4 = atol(v3);
        if ((unsigned int)(v4 - 1) > 0x1FF)
        {
            Com_PrintError(9, "ERROR: LocalSoundStop() called with index %i (should be in range[1,%i])\n", v4, 512);
        }
        else
        {
            ConfigString = CL_GetConfigString(localClientNum, v4 + 1667);
            CG_StopClientSoundAliasByName(localClientNum, ConfigString);
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: LocalSoundStop(), should be called with 2 arguments.\n");
    }
}

void CG_ReachedCheckpoint()
{
    const char *v0; // r3
    const char *v1; // r3

    v0 = SEH_LocalizeTextMessage("EXE_CHECKPOINT_REACHED", "game message", LOCMSG_SAFE);
    v1 = va("^7%s", v0);
    CG_GameMessage(v1, 17);
}

void __cdecl CG_GameSaveFailed(cg_s *cgameGlob)
{
    UI_SetActiveMenu(0, UIMENU_SAVEERROR);
}

void __cdecl CG_BlurServerCommand(int localClientNum)
{
    int nesting; // r7
    const char *v3; // r3
    int v4; // r3
    int v5; // r7
    int v6; // r26
    const char *v7; // r3
    long double v8; // fp2
    int v9; // r7
    double v10; // fp31
    const char *v11; // r3
    int v12; // r3
    int v13; // r7
    BlurTime v14; // r28
    const char *v15; // r3
    BlurPriority v16; // r29
    BlurTime v17; // r5

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    v4 = atol(v3);
    v5 = cmd_args.nesting;
    v6 = v4;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        v5 = cmd_args.nesting;
    }
    if (cmd_args.argc[v5] <= 2)
        v7 = byte_82003CDD;
    else
        v7 = (const char *)*((unsigned int *)cmd_args.argv[v5] + 2);
    v8 = atof(v7);
    v9 = cmd_args.nesting;
    v10 = (float)*(double *)&v8;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        v9 = cmd_args.nesting;
    }
    if (cmd_args.argc[v9] <= 3)
        v11 = byte_82003CDD;
    else
        v11 = (const char *)*((unsigned int *)cmd_args.argv[v9] + 3);
    v12 = atol(v11);
    v13 = cmd_args.nesting;
    v14 = v12;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        v13 = cmd_args.nesting;
    }
    if (cmd_args.argc[v13] <= 4)
        v15 = byte_82003CDD;
    else
        v15 = (const char *)*((unsigned int *)cmd_args.argv[v13] + 4);
    v16 = atol(v15);
    if (v6 < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp", 859, 0, "%s", "time >= 0");
    if (v10 < 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp", 860, 0, "%s", "blurEndValue >= 0");
    CG_Blur(localClientNum, v6, v10, v17, v14, v16);
}

void __cdecl CG_SlowServerCommand(int localClientNum)
{
    int nesting; // r7
    const char *v3; // r3
    int v4; // r3
    int v5; // r7
    int v6; // r27
    const char *v7; // r3
    long double v8; // fp2
    int v9; // r7
    double v10; // fp31
    const char *v11; // r3
    long double v12; // fp2

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    v4 = atol(v3);
    v5 = cmd_args.nesting;
    v6 = v4;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        v5 = cmd_args.nesting;
    }
    if (cmd_args.argc[v5] <= 2)
        v7 = byte_82003CDD;
    else
        v7 = (const char *)*((unsigned int *)cmd_args.argv[v5] + 2);
    v8 = atof(v7);
    v9 = cmd_args.nesting;
    v10 = (float)*(double *)&v8;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        v9 = cmd_args.nesting;
    }
    if (cmd_args.argc[v9] <= 3)
        v11 = byte_82003CDD;
    else
        v11 = (const char *)*((unsigned int *)cmd_args.argv[v9] + 3);
    v12 = atof(v11);
    CG_AlterTimescale(localClientNum, v6, v10, (float)*(double *)&v12);
}

void __cdecl CG_SetClientDvarFromServer(const char *dvarname, const char *value)
{
    if (I_stricmp(dvarname, "hud_drawHUD"))
        Com_Error(ERR_DROP, byte_8200F418, dvarname);
    else
        Dvar_SetFromStringByName("hud_drawHUD", value);
}

void CG_ParseAmp()
{
    int nesting; // r7
    int v1; // r5
    const char *v2; // r3
    long double v3; // fp2
    const char *v4; // r3
    long double v5; // fp2
    const char *v6; // r3
    long double v7; // fp2
    const char *v8; // r3
    int v9; // r31
    const char *v10; // r3
    int v11; // r30
    const char *v12; // r3
    long double v13; // fp2
    double v14; // fp31
    const char *v15; // r3
    long double v16; // fp2
    double v17; // fp30
    const char *v18; // r3
    long double v19; // fp2
    double v20; // fp3
    float v21[6]; // [sp+50h] [-40h] BYREF

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
    v1 = cmd_args.argc[nesting];
    if (v1 == 9)
    {
        v2 = Cmd_Argv(1);
        v3 = atof(v2);
        v21[0] = *(double *)&v3;
        v4 = Cmd_Argv(2);
        v5 = atof(v4);
        v21[1] = *(double *)&v5;
        v6 = Cmd_Argv(3);
        v7 = atof(v6);
        v21[2] = *(double *)&v7;
        v8 = Cmd_Argv(4);
        v9 = atol(v8);
        v10 = Cmd_Argv(5);
        v11 = atol(v10);
        v12 = Cmd_Argv(6);
        v13 = atof(v12);
        v14 = (float)*(double *)&v13;
        v15 = Cmd_Argv(7);
        v16 = atof(v15);
        v17 = (float)*(double *)&v16;
        v18 = Cmd_Argv(8);
        v19 = atof(v18);
        v20 = (float)*(double *)&v19;
        if (v9 >= 0)
        {
            if (v11 >= v9)
            {
                if (v14 >= 0.0)
                {
                    if (v17 >= v14)
                    {
                        if (v20 >= 0.0)
                            SND_Amplify(v21, v9, v11, v14, v17, v20);
                        else
                            Com_PrintError(14, (const char *)HIDWORD(v20), LODWORD(v20));
                    }
                    else
                    {
                        Com_PrintError(14, (const char *)HIDWORD(v17), LODWORD(v17), LODWORD(v14));
                    }
                }
                else
                {
                    Com_PrintError(14, (const char *)HIDWORD(v14), LODWORD(v14));
                }
            }
            else
            {
                Com_PrintError(14, "amplify(): max_range (%i) must be >= min_range %i)", v11, v9);
            }
        }
        else
        {
            Com_PrintError(14, "amplify(): min_range (%i) must be >= 0\n", v9);
        }
    }
    else
    {
        Com_PrintError(14, "amplify called with %i arguments, should be 8\n", v1);
    }
}

void __cdecl CG_ParsePhysGravityDir(int localClientNum)
{
    unsigned int nesting; // r7
    int v3; // r28
    const char *v4; // r3
    const char *v5; // r3
    const char *v6; // r3
    long double v7; // fp2
    const char *v8; // r3
    long double v9; // fp2
    const char *v10; // r3
    long double v11; // fp2
    float v12; // [sp+50h] [-40h] BYREF
    float v13; // [sp+54h] [-3Ch]
    float v14; // [sp+58h] [-38h]

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
    v3 = cmd_args.argc[nesting];
    if (nesting >= 8)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] == 4)
    {
        v4 = Cmd_Argv(0);
        if (I_strcmp(v4, "phys_grav"))
        {
            v5 = Cmd_Argv(0);
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
                980,
                0,
                "%s\n\t(Cmd_Argv( 0 )) = %s",
                "(!I_strcmp( Cmd_Argv( 0 ), \"phys_grav\" ))",
                v5);
        }
        v6 = Cmd_Argv(1);
        v7 = atof(v6);
        v12 = *(double *)&v7;
        v8 = Cmd_Argv(2);
        v9 = atof(v8);
        v13 = *(double *)&v9;
        v10 = Cmd_Argv(3);
        v11 = atof(v10);
        v14 = *(double *)&v11;
        if (v12 == 0.0 && v13 == 0.0 && (float)*(double *)&v11 == 0.0)
        {
            v12 = 0.0;
            v13 = 0.0;
            v14 = -1.0;
        }
        Phys_SetGravityDir(&v12);
        DynEntCl_WakeUpAroundPlayer(localClientNum);
    }
    else
    {
        Com_PrintError(14, "phys_grav called with %i arguments, should be 3\n", v3);
    }
}

void __cdecl CG_DispatchServerCommand(int localClientNum)
{
    int nesting; // r7
    const char *v3; // r31
    const char *v4; // r10
    const char *v5; // r11
    int v6; // r8
    const char *v7; // r3
    unsigned int v8; // r3
    const char *v9; // r10
    const char *v10; // r11
    int v11; // r8
    const char *v12; // r3
    const char *v13; // r10
    const char *v14; // r11
    int v15; // r8
    const char *v16; // r10
    const char *v17; // r11
    int v18; // r8
    const char *v19; // r3
    const char *v20; // r3
    const char *v21; // r10
    const char *v22; // r11
    int v23; // r8
    const char *v24; // r3
    const char *v25; // r3
    const char *v26; // r10
    const char *v27; // r11
    int v28; // r8
    const char *v29; // r3
    const char *v30; // r3
    const char *v31; // r10
    const char *v32; // r11
    int v33; // r8
    const char *v34; // r3
    unsigned __int16 v35; // r31
    const char *v36; // r3
    DynEntityDrawType v37; // r30
    const char *v38; // r3
    long double v39; // fp2
    double v40; // fp31
    const char *v41; // r3
    long double v42; // fp2
    double v43; // fp30
    const char *v44; // r3
    long double v45; // fp2
    const char *v46; // r3
    long double v47; // fp2
    double v48; // fp31
    const char *v49; // r3
    long double v50; // fp2
    double v51; // fp30
    const char *v52; // r3
    long double v53; // fp2
    const char *v54; // r10
    const char *v55; // r11
    int v56; // r8
    const char *v57; // r3
    const char *v58; // r3
    const char *v59; // r10
    const char *v60; // r11
    int v61; // r8
    const char *v62; // r3
    const char *v63; // r3
    const char *v64; // r10
    const char *v65; // r11
    int v66; // r8
    const char *v67; // r3
    const char *v68; // r3
    const char *v69; // r10
    const char *v70; // r11
    int v71; // r8
    const char *v72; // r10
    const char *v73; // r11
    int v74; // r8
    const char *v75; // r10
    const char *v76; // r11
    int v77; // r8
    int i; // r31
    const char *v79; // r10
    const char *v80; // r11
    int v81; // r8
    const char *v82; // r10
    const char *v83; // r11
    int v84; // r8
    const char *v85; // r10
    const char *v86; // r11
    int v87; // r8
    const char *v88; // r3
    bool v89; // r31
    const char *v90; // r3
    const snd_alias_t *v91; // r3
    const char *v92; // r10
    const char *v93; // r11
    int v94; // r8
    const char *v95; // r3
    int v96; // r3
    const char *v97; // r10
    const char *v98; // r11
    int v99; // r8
    const char *v100; // r3
    const char *v101; // r3
    long double v102; // fp2
    int v103; // r3
    const char *v104; // r10
    const char *v105; // r11
    int v106; // r8
    const char *v107; // r10
    const char *v108; // r11
    int v109; // r8
    const char *v110; // r10
    const char *v111; // r11
    int v112; // r8
    const char *v113; // r3
    int v114; // r31
    const char *v115; // r3
    int v116; // r30
    const char *v117; // r3
    long double v118; // fp2
    const char *v119; // r10
    const char *v120; // r11
    int v121; // r8
    const char *v122; // r10
    const char *v123; // r11
    int v124; // r8
    const char *v125; // r10
    const char *v126; // r11
    int v127; // r8
    const char *v128; // r10
    const char *v129; // r11
    int v130; // r8
    const char *v131; // r10
    const char *v132; // r11
    int v133; // r8
    const char *v134; // r3
    long double v135; // fp2
    const char *v136; // r10
    const char *v137; // r11
    int v138; // r8
    const char *v139; // r10
    const char *v140; // r11
    int v141; // r8
    const char *v142; // r10
    const char *v143; // r11
    int v144; // r8
    const char *v145; // r10
    const char *v146; // r11
    int v147; // r8
    const char *v148; // r10
    const char *v149; // r11
    int v150; // r8
    const char *v151; // r10
    const char *v152; // r11
    int v153; // r8
    const char *v154; // r10
    const char *v155; // r11
    int v156; // r8
    const char *v157; // r10
    const char *v158; // r11
    int v159; // r8
    const char *v160; // r10
    const char *v161; // r11
    int v162; // r8
    const char *v163; // r3
    int v164; // r3
    const char *v165; // r10
    const char *v166; // r11
    int v167; // r8
    const char *v168; // r3
    int v169; // r3
    const char *v170; // r10
    const char *v171; // r11
    int v172; // r8
    const char *v173; // r31
    const char *v174; // r3
    const char *v175; // r10
    const char *v176; // r11
    int v177; // r8
    const char *v178; // r10
    const char *v179; // r11
    int v180; // r8
    const char *v181; // r10
    const char *v182; // r11
    int v183; // r8
    const char *v184; // r3
    int v185; // r31
    const char *v186; // r3
    int v187; // r3
    const char *v188; // r10
    const char *v189; // r11
    int v190; // r8
    const char *v191; // r10
    const char *v192; // r11
    int v193; // r8
    const char *v194; // r10
    const char *v195; // r11
    int v196; // r8
    const char *v197; // r10
    const char *v198; // r11
    int v199; // r8
    const char *v200; // r3
    int v201; // r31
    const char *v202; // r3
    int v203; // r30
    int v204; // r3
    float v205[4]; // [sp+58h] [-98h] BYREF
    float v206[6]; // [sp+68h] [-88h] BYREF
    bool v207[64]; // [sp+80h] [-70h] BYREF

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 0)
        v3 = byte_82003CDD;
    else
        v3 = *cmd_args.argv[nesting];
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (*v3)
    {
        v4 = "sw";
        v5 = v3;
        do
        {
            v6 = *(unsigned __int8 *)v5 - *(unsigned __int8 *)v4;
            if (!*v5)
                break;
            ++v5;
            ++v4;
        } while (!v6);
        if (v6)
        {
            v9 = "cp";
            v10 = v3;
            do
            {
                v11 = *(unsigned __int8 *)v10 - *(unsigned __int8 *)v9;
                if (!*v10)
                    break;
                ++v10;
                ++v9;
            } while (!v11);
            if (v11)
            {
                v13 = "cs";
                v14 = v3;
                do
                {
                    v15 = *(unsigned __int8 *)v14 - *(unsigned __int8 *)v13;
                    if (!*v14)
                        break;
                    ++v14;
                    ++v13;
                } while (!v15);
                if (v15)
                {
                    v16 = "print";
                    v17 = v3;
                    do
                    {
                        v18 = *(unsigned __int8 *)v17 - *(unsigned __int8 *)v16;
                        if (!*v17)
                            break;
                        ++v17;
                        ++v16;
                    } while (!v18);
                    if (v18)
                    {
                        v21 = "gm";
                        v22 = v3;
                        do
                        {
                            v23 = *(unsigned __int8 *)v22 - *(unsigned __int8 *)v21;
                            if (!*v22)
                                break;
                            ++v22;
                            ++v21;
                        } while (!v23);
                        if (v23)
                        {
                            v26 = "gmb";
                            v27 = v3;
                            do
                            {
                                v28 = *(unsigned __int8 *)v27 - *(unsigned __int8 *)v26;
                                if (!*v27)
                                    break;
                                ++v27;
                                ++v26;
                            } while (!v28);
                            if (v28)
                            {
                                v31 = "dynent";
                                v32 = v3;
                                do
                                {
                                    v33 = *(unsigned __int8 *)v32 - *(unsigned __int8 *)v31;
                                    if (!*v32)
                                        break;
                                    ++v32;
                                    ++v31;
                                } while (!v33);
                                if (v33)
                                {
                                    v54 = "obj_update";
                                    v55 = v3;
                                    do
                                    {
                                        v56 = *(unsigned __int8 *)v55 - *(unsigned __int8 *)v54;
                                        if (!*v55)
                                            break;
                                        ++v55;
                                        ++v54;
                                    } while (!v56);
                                    if (v56)
                                    {
                                        v59 = "obj_complete";
                                        v60 = v3;
                                        do
                                        {
                                            v61 = *(unsigned __int8 *)v60 - *(unsigned __int8 *)v59;
                                            if (!*v60)
                                                break;
                                            ++v60;
                                            ++v59;
                                        } while (!v61);
                                        if (v61)
                                        {
                                            v64 = "obj_failed";
                                            v65 = v3;
                                            do
                                            {
                                                v66 = *(unsigned __int8 *)v65 - *(unsigned __int8 *)v64;
                                                if (!*v65)
                                                    break;
                                                ++v65;
                                                ++v64;
                                            } while (!v66);
                                            if (v66)
                                            {
                                                v69 = "opendeadscreen";
                                                v70 = v3;
                                                do
                                                {
                                                    v71 = *(unsigned __int8 *)v70 - *(unsigned __int8 *)v69;
                                                    if (!*v70)
                                                        break;
                                                    ++v70;
                                                    ++v69;
                                                } while (!v71);
                                                if (v71)
                                                {
                                                    v72 = "closedeadscreen";
                                                    v73 = v3;
                                                    do
                                                    {
                                                        v74 = *(unsigned __int8 *)v73 - *(unsigned __int8 *)v72;
                                                        if (!*v73)
                                                            break;
                                                        ++v73;
                                                        ++v72;
                                                    } while (!v74);
                                                    if (v74)
                                                    {
                                                        v75 = "openvictoryscreen";
                                                        v76 = v3;
                                                        do
                                                        {
                                                            v77 = *(unsigned __int8 *)v76 - *(unsigned __int8 *)v75;
                                                            if (!*v76)
                                                                break;
                                                            ++v76;
                                                            ++v75;
                                                        } while (!v77);
                                                        if (v77)
                                                        {
                                                            v79 = "gamesavefailed";
                                                            v80 = v3;
                                                            do
                                                            {
                                                                v81 = *(unsigned __int8 *)v80 - *(unsigned __int8 *)v79;
                                                                if (!*v80)
                                                                    break;
                                                                ++v80;
                                                                ++v79;
                                                            } while (!v81);
                                                            if (v81)
                                                            {
                                                                v82 = "reachedcheckpoint";
                                                                v83 = v3;
                                                                do
                                                                {
                                                                    v84 = *(unsigned __int8 *)v83 - *(unsigned __int8 *)v82;
                                                                    if (!*v83)
                                                                        break;
                                                                    ++v83;
                                                                    ++v82;
                                                                } while (!v84);
                                                                if (v84)
                                                                {
                                                                    v85 = "mu_play";
                                                                    v86 = v3;
                                                                    do
                                                                    {
                                                                        v87 = *(unsigned __int8 *)v86 - *(unsigned __int8 *)v85;
                                                                        if (!*v86)
                                                                            break;
                                                                        ++v86;
                                                                        ++v85;
                                                                    } while (!v87);
                                                                    if (v87)
                                                                    {
                                                                        v92 = "mu_stop";
                                                                        v93 = v3;
                                                                        do
                                                                        {
                                                                            v94 = *(unsigned __int8 *)v93 - *(unsigned __int8 *)v92;
                                                                            if (!*v93)
                                                                                break;
                                                                            ++v93;
                                                                            ++v92;
                                                                        } while (!v94);
                                                                        if (v94)
                                                                        {
                                                                            v97 = "snd_fade";
                                                                            v98 = v3;
                                                                            do
                                                                            {
                                                                                v99 = *(unsigned __int8 *)v98 - *(unsigned __int8 *)v97;
                                                                                if (!*v98)
                                                                                    break;
                                                                                ++v98;
                                                                                ++v97;
                                                                            } while (!v99);
                                                                            if (v99)
                                                                            {
                                                                                v104 = "scr_blur";
                                                                                v105 = v3;
                                                                                do
                                                                                {
                                                                                    v106 = *(unsigned __int8 *)v105 - *(unsigned __int8 *)v104;
                                                                                    if (!*v105)
                                                                                        break;
                                                                                    ++v105;
                                                                                    ++v104;
                                                                                } while (!v106);
                                                                                if (v106)
                                                                                {
                                                                                    v107 = "clear_blur";
                                                                                    v108 = v3;
                                                                                    do
                                                                                    {
                                                                                        v109 = *(unsigned __int8 *)v108 - *(unsigned __int8 *)v107;
                                                                                        if (!*v108)
                                                                                            break;
                                                                                        ++v108;
                                                                                        ++v107;
                                                                                    } while (!v109);
                                                                                    if (v109)
                                                                                    {
                                                                                        v110 = "scr_fade";
                                                                                        v111 = v3;
                                                                                        do
                                                                                        {
                                                                                            v112 = *(unsigned __int8 *)v111 - *(unsigned __int8 *)v110;
                                                                                            if (!*v111)
                                                                                                break;
                                                                                            ++v111;
                                                                                            ++v110;
                                                                                        } while (!v112);
                                                                                        if (v112)
                                                                                        {
                                                                                            v119 = "time_slow";
                                                                                            v120 = v3;
                                                                                            do
                                                                                            {
                                                                                                v121 = *(unsigned __int8 *)v120 - *(unsigned __int8 *)v119;
                                                                                                if (!*v120)
                                                                                                    break;
                                                                                                ++v120;
                                                                                                ++v119;
                                                                                            } while (!v121);
                                                                                            if (v121)
                                                                                            {
                                                                                                v122 = "reverb";
                                                                                                v123 = v3;
                                                                                                do
                                                                                                {
                                                                                                    v124 = *(unsigned __int8 *)v123 - *(unsigned __int8 *)v122;
                                                                                                    if (!*v123)
                                                                                                        break;
                                                                                                    ++v123;
                                                                                                    ++v122;
                                                                                                } while (!v124);
                                                                                                if (v124)
                                                                                                {
                                                                                                    v125 = "eq";
                                                                                                    v126 = v3;
                                                                                                    do
                                                                                                    {
                                                                                                        v127 = *(unsigned __int8 *)v126 - *(unsigned __int8 *)v125;
                                                                                                        if (!*v126)
                                                                                                            break;
                                                                                                        ++v126;
                                                                                                        ++v125;
                                                                                                    } while (!v127);
                                                                                                    if (v127)
                                                                                                    {
                                                                                                        v128 = "deactivateeq";
                                                                                                        v129 = v3;
                                                                                                        do
                                                                                                        {
                                                                                                            v130 = *(unsigned __int8 *)v129 - *(unsigned __int8 *)v128;
                                                                                                            if (!*v129)
                                                                                                                break;
                                                                                                            ++v129;
                                                                                                            ++v128;
                                                                                                        } while (!v130);
                                                                                                        if (v130)
                                                                                                        {
                                                                                                            v131 = "eqLerp";
                                                                                                            v132 = v3;
                                                                                                            do
                                                                                                            {
                                                                                                                v133 = *(unsigned __int8 *)v132 - *(unsigned __int8 *)v131;
                                                                                                                if (!*v132)
                                                                                                                    break;
                                                                                                                ++v132;
                                                                                                                ++v131;
                                                                                                            } while (!v133);
                                                                                                            if (v133)
                                                                                                            {
                                                                                                                v136 = "deactivatereverb";
                                                                                                                v137 = v3;
                                                                                                                do
                                                                                                                {
                                                                                                                    v138 = *(unsigned __int8 *)v137 - *(unsigned __int8 *)v136;
                                                                                                                    if (!*v137)
                                                                                                                        break;
                                                                                                                    ++v137;
                                                                                                                    ++v136;
                                                                                                                } while (!v138);
                                                                                                                if (v138)
                                                                                                                {
                                                                                                                    v139 = "setchannelvol";
                                                                                                                    v140 = v3;
                                                                                                                    do
                                                                                                                    {
                                                                                                                        v141 = *(unsigned __int8 *)v140 - *(unsigned __int8 *)v139;
                                                                                                                        if (!*v140)
                                                                                                                            break;
                                                                                                                        ++v140;
                                                                                                                        ++v139;
                                                                                                                    } while (!v141);
                                                                                                                    if (v141)
                                                                                                                    {
                                                                                                                        v142 = "deactivatechannelvol";
                                                                                                                        v143 = v3;
                                                                                                                        do
                                                                                                                        {
                                                                                                                            v144 = *(unsigned __int8 *)v143 - *(unsigned __int8 *)v142;
                                                                                                                            if (!*v143)
                                                                                                                                break;
                                                                                                                            ++v143;
                                                                                                                            ++v142;
                                                                                                                        } while (!v144);
                                                                                                                        if (v144)
                                                                                                                        {
                                                                                                                            v145 = "fog";
                                                                                                                            v146 = v3;
                                                                                                                            do
                                                                                                                            {
                                                                                                                                v147 = *(unsigned __int8 *)v146
                                                                                                                                    - *(unsigned __int8 *)v145;
                                                                                                                                if (!*v146)
                                                                                                                                    break;
                                                                                                                                ++v146;
                                                                                                                                ++v145;
                                                                                                                            } while (!v147);
                                                                                                                            if (v147)
                                                                                                                            {
                                                                                                                                v148 = "ls";
                                                                                                                                v149 = v3;
                                                                                                                                do
                                                                                                                                {
                                                                                                                                    v150 = *(unsigned __int8 *)v149
                                                                                                                                        - *(unsigned __int8 *)v148;
                                                                                                                                    if (!*v149)
                                                                                                                                        break;
                                                                                                                                    ++v149;
                                                                                                                                    ++v148;
                                                                                                                                } while (!v150);
                                                                                                                                if (v150)
                                                                                                                                {
                                                                                                                                    v151 = "ls_stop";
                                                                                                                                    v152 = v3;
                                                                                                                                    do
                                                                                                                                    {
                                                                                                                                        v153 = *(unsigned __int8 *)v152
                                                                                                                                            - *(unsigned __int8 *)v151;
                                                                                                                                        if (!*v152)
                                                                                                                                            break;
                                                                                                                                        ++v152;
                                                                                                                                        ++v151;
                                                                                                                                    } while (!v153);
                                                                                                                                    if (v153)
                                                                                                                                    {
                                                                                                                                        v154 = "popupopen";
                                                                                                                                        v155 = v3;
                                                                                                                                        do
                                                                                                                                        {
                                                                                                                                            v156 = *(unsigned __int8 *)v155
                                                                                                                                                - *(unsigned __int8 *)v154;
                                                                                                                                            if (!*v155)
                                                                                                                                                break;
                                                                                                                                            ++v155;
                                                                                                                                            ++v154;
                                                                                                                                        } while (!v156);
                                                                                                                                        if (v156)
                                                                                                                                        {
                                                                                                                                            v157 = "popupclose";
                                                                                                                                            v158 = v3;
                                                                                                                                            do
                                                                                                                                            {
                                                                                                                                                v159 = *(unsigned __int8 *)v158
                                                                                                                                                    - *(unsigned __int8 *)v157;
                                                                                                                                                if (!*v158)
                                                                                                                                                    break;
                                                                                                                                                ++v158;
                                                                                                                                                ++v157;
                                                                                                                                            } while (!v159);
                                                                                                                                            if (v159)
                                                                                                                                            {
                                                                                                                                                v160 = "menu_show_notify";
                                                                                                                                                v161 = v3;
                                                                                                                                                do
                                                                                                                                                {
                                                                                                                                                    v162 = *(unsigned __int8 *)v161
                                                                                                                                                        - *(unsigned __int8 *)v160;
                                                                                                                                                    if (!*v161)
                                                                                                                                                        break;
                                                                                                                                                    ++v161;
                                                                                                                                                    ++v160;
                                                                                                                                                } while (!v162);
                                                                                                                                                if (v162)
                                                                                                                                                {
                                                                                                                                                    v165 = "offhand";
                                                                                                                                                    v166 = v3;
                                                                                                                                                    do
                                                                                                                                                    {
                                                                                                                                                        v167 = *(unsigned __int8 *)v166
                                                                                                                                                            - *(unsigned __int8 *)v165;
                                                                                                                                                        if (!*v166)
                                                                                                                                                            break;
                                                                                                                                                        ++v166;
                                                                                                                                                        ++v165;
                                                                                                                                                    } while (!v167);
                                                                                                                                                    if (v167)
                                                                                                                                                    {
                                                                                                                                                        v170 = "setclientdvar";
                                                                                                                                                        v171 = v3;
                                                                                                                                                        do
                                                                                                                                                        {
                                                                                                                                                            v172 = *(unsigned __int8 *)v171
                                                                                                                                                                - *(unsigned __int8 *)v170;
                                                                                                                                                            if (!*v171)
                                                                                                                                                                break;
                                                                                                                                                            ++v171;
                                                                                                                                                            ++v170;
                                                                                                                                                        } while (!v172);
                                                                                                                                                        if (v172)
                                                                                                                                                        {
                                                                                                                                                            v175 = "showViewModel";
                                                                                                                                                            v176 = v3;
                                                                                                                                                            do
                                                                                                                                                            {
                                                                                                                                                                v177 = *(unsigned __int8 *)v176
                                                                                                                                                                    - *(unsigned __int8 *)v175;
                                                                                                                                                                if (!*v176)
                                                                                                                                                                    break;
                                                                                                                                                                ++v176;
                                                                                                                                                                ++v175;
                                                                                                                                                            } while (!v177);
                                                                                                                                                            if (v177)
                                                                                                                                                            {
                                                                                                                                                                v178 = "hideViewModel";
                                                                                                                                                                v179 = v3;
                                                                                                                                                                do
                                                                                                                                                                {
                                                                                                                                                                    v180 = *(unsigned __int8 *)v179
                                                                                                                                                                        - *(unsigned __int8 *)v178;
                                                                                                                                                                    if (!*v179)
                                                                                                                                                                        break;
                                                                                                                                                                    ++v179;
                                                                                                                                                                    ++v178;
                                                                                                                                                                } while (!v180);
                                                                                                                                                                if (v180)
                                                                                                                                                                {
                                                                                                                                                                    v181 = "ret_lock_on";
                                                                                                                                                                    v182 = v3;
                                                                                                                                                                    do
                                                                                                                                                                    {
                                                                                                                                                                        v183 = *(unsigned __int8 *)v182
                                                                                                                                                                            - *(unsigned __int8 *)v181;
                                                                                                                                                                        if (!*v182)
                                                                                                                                                                            break;
                                                                                                                                                                        ++v182;
                                                                                                                                                                        ++v181;
                                                                                                                                                                    } while (!v183);
                                                                                                                                                                    if (v183)
                                                                                                                                                                    {
                                                                                                                                                                        v188 = "amp";
                                                                                                                                                                        v189 = v3;
                                                                                                                                                                        do
                                                                                                                                                                        {
                                                                                                                                                                            v190 = *(unsigned __int8 *)v189
                                                                                                                                                                                - *(unsigned __int8 *)v188;
                                                                                                                                                                            if (!*v189)
                                                                                                                                                                                break;
                                                                                                                                                                            ++v189;
                                                                                                                                                                            ++v188;
                                                                                                                                                                        } while (!v190);
                                                                                                                                                                        if (v190)
                                                                                                                                                                        {
                                                                                                                                                                            v191 = "amp_stop";
                                                                                                                                                                            v192 = v3;
                                                                                                                                                                            do
                                                                                                                                                                            {
                                                                                                                                                                                v193 = *(unsigned __int8 *)v192
                                                                                                                                                                                    - *(unsigned __int8 *)v191;
                                                                                                                                                                                if (!*v192)
                                                                                                                                                                                    break;
                                                                                                                                                                                ++v192;
                                                                                                                                                                                ++v191;
                                                                                                                                                                            } while (!v193);
                                                                                                                                                                            if (v193)
                                                                                                                                                                            {
                                                                                                                                                                                v194 = "phys_grav";
                                                                                                                                                                                v195 = v3;
                                                                                                                                                                                do
                                                                                                                                                                                {
                                                                                                                                                                                    v196 = *(unsigned __int8 *)v195
                                                                                                                                                                                        - *(unsigned __int8 *)v194;
                                                                                                                                                                                    if (!*v195)
                                                                                                                                                                                        break;
                                                                                                                                                                                    ++v195;
                                                                                                                                                                                    ++v194;
                                                                                                                                                                                } while (!v196);
                                                                                                                                                                                if (v196)
                                                                                                                                                                                {
                                                                                                                                                                                    v197 = "upscore";
                                                                                                                                                                                    v198 = v3;
                                                                                                                                                                                    do
                                                                                                                                                                                    {
                                                                                                                                                                                        v199 = *(unsigned __int8 *)v198
                                                                                                                                                                                            - *(unsigned __int8 *)v197;
                                                                                                                                                                                        if (!*v198)
                                                                                                                                                                                            break;
                                                                                                                                                                                        ++v198;
                                                                                                                                                                                        ++v197;
                                                                                                                                                                                    } while (!v199);
                                                                                                                                                                                    if (v199)
                                                                                                                                                                                    {
                                                                                                                                                                                        Com_Printf(
                                                                                                                                                                                            14,
                                                                                                                                                                                            "Unknown client game command: %s\n",
                                                                                                                                                                                            v3);
                                                                                                                                                                                    }
                                                                                                                                                                                    else
                                                                                                                                                                                    {
                                                                                                                                                                                        v200 = Cmd_Argv(2);
                                                                                                                                                                                        v201 = atol(v200);
                                                                                                                                                                                        v202 = Cmd_Argv(1);
                                                                                                                                                                                        v203 = atol(v202);
                                                                                                                                                                                        v204 = CL_ControllerIndexFromClientNum(localClientNum);
                                                                                                                                                                                        LB_UploadPlayerScore(
                                                                                                                                                                                            v204,
                                                                                                                                                                                            v203,
                                                                                                                                                                                            v201);
                                                                                                                                                                                    }
                                                                                                                                                                                }
                                                                                                                                                                                else
                                                                                                                                                                                {
                                                                                                                                                                                    CG_ParsePhysGravityDir(localClientNum);
                                                                                                                                                                                }
                                                                                                                                                                            }
                                                                                                                                                                            else
                                                                                                                                                                            {
                                                                                                                                                                                SND_StopAmplify();
                                                                                                                                                                            }
                                                                                                                                                                        }
                                                                                                                                                                        else
                                                                                                                                                                        {
                                                                                                                                                                            CG_ParseAmp();
                                                                                                                                                                        }
                                                                                                                                                                    }
                                                                                                                                                                    else
                                                                                                                                                                    {
                                                                                                                                                                        v184 = Cmd_Argv(2);
                                                                                                                                                                        v185 = atol(v184);
                                                                                                                                                                        v186 = Cmd_Argv(1);
                                                                                                                                                                        v187 = atol(v186);
                                                                                                                                                                        CG_ReticleStartLockOn(
                                                                                                                                                                            localClientNum,
                                                                                                                                                                            v187,
                                                                                                                                                                            v185);
                                                                                                                                                                    }
                                                                                                                                                                }
                                                                                                                                                                else
                                                                                                                                                                {
                                                                                                                                                                    cgArray[0].hideViewModel = 1;
                                                                                                                                                                    PM_ResetWeaponState(&cgArray[0].predictedPlayerState);
                                                                                                                                                                }
                                                                                                                                                            }
                                                                                                                                                            else
                                                                                                                                                            {
                                                                                                                                                                cgArray[0].hideViewModel = 0;
                                                                                                                                                            }
                                                                                                                                                        }
                                                                                                                                                        else
                                                                                                                                                        {
                                                                                                                                                            v173 = Cmd_Argv(2);
                                                                                                                                                            v174 = Cmd_Argv(1);
                                                                                                                                                            CG_SetClientDvarFromServer(v174, v173);
                                                                                                                                                        }
                                                                                                                                                    }
                                                                                                                                                    else
                                                                                                                                                    {
                                                                                                                                                        v168 = Cmd_Argv(1);
                                                                                                                                                        v169 = atol(v168);
                                                                                                                                                        CG_SetEquippedOffHand(localClientNum, v169);
                                                                                                                                                    }
                                                                                                                                                }
                                                                                                                                                else
                                                                                                                                                {
                                                                                                                                                    v163 = Cmd_Argv(1);
                                                                                                                                                    v164 = atol(v163);
                                                                                                                                                    CG_MenuShowNotify(localClientNum, v164);
                                                                                                                                                }
                                                                                                                                            }
                                                                                                                                            else
                                                                                                                                            {
                                                                                                                                                UI_ClosePopupScriptMenu(0, 1);
                                                                                                                                                cg_waitingScriptMenu.name[0] = 0;
                                                                                                                                            }
                                                                                                                                        }
                                                                                                                                        else
                                                                                                                                        {
                                                                                                                                            CG_OpenScriptMenu(localClientNum);
                                                                                                                                        }
                                                                                                                                    }
                                                                                                                                    else
                                                                                                                                    {
                                                                                                                                        LocalSoundStop(localClientNum);
                                                                                                                                    }
                                                                                                                                }
                                                                                                                                else
                                                                                                                                {
                                                                                                                                    LocalSound(localClientNum);
                                                                                                                                }
                                                                                                                            }
                                                                                                                            else
                                                                                                                            {
                                                                                                                                CG_ParseFog(cgArray[0].time);
                                                                                                                            }
                                                                                                                        }
                                                                                                                        else
                                                                                                                        {
                                                                                                                            CG_DeactivateChannelVolCmd();
                                                                                                                        }
                                                                                                                    }
                                                                                                                    else
                                                                                                                    {
                                                                                                                        CG_SetChannelVolCmd(localClientNum);
                                                                                                                    }
                                                                                                                }
                                                                                                                else
                                                                                                                {
                                                                                                                    CG_DeactivateReverbCmd();
                                                                                                                }
                                                                                                            }
                                                                                                            else
                                                                                                            {
                                                                                                                v134 = Cmd_Argv(1);
                                                                                                                v135 = atof(v134);
                                                                                                                SND_SetEqLerp((float)*(double *)&v135);
                                                                                                            }
                                                                                                        }
                                                                                                        else
                                                                                                        {
                                                                                                            CG_DeactivateEqCmd();
                                                                                                        }
                                                                                                    }
                                                                                                    else
                                                                                                    {
                                                                                                        CG_EqCommand();
                                                                                                    }
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    CG_ReverbCmd();
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                CG_SlowServerCommand(localClientNum);
                                                                                            }
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            v113 = Cmd_Argv(3);
                                                                                            v114 = atol(v113);
                                                                                            v115 = Cmd_Argv(2);
                                                                                            v116 = atol(v115);
                                                                                            v117 = Cmd_Argv(1);
                                                                                            v118 = atof(v117);
                                                                                            CG_Fade(
                                                                                                localClientNum,
                                                                                                0,
                                                                                                0,
                                                                                                (int)(float)((float)*(double *)&v118 * (float)255.0),
                                                                                                v116,
                                                                                                v114);
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        CG_ClearBlur(localClientNum);
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    CG_BlurServerCommand(localClientNum);
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                v100 = Cmd_Argv(2);
                                                                                atol(v100);
                                                                                v101 = Cmd_Argv(1);
                                                                                v102 = atof(v101);
                                                                                SND_FadeAllSounds((float)*(double *)&v102, v103);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            v95 = Cmd_Argv(1);
                                                                            v96 = atol(v95);
                                                                            SND_StopMusic(v96);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        v88 = Cmd_Argv(2);
                                                                        v89 = (_cntlzw(atol(v88)) & 0x20) == 0;
                                                                        v90 = Cmd_Argv(1);
                                                                        v91 = CL_PickSoundAlias(v90);
                                                                        SND_PlayMusicAlias(localClientNum, v91, v89, SASYS_CGAME);
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    CG_ReachedCheckpoint();
                                                                }
                                                            }
                                                            else
                                                            {
                                                                UI_SetActiveMenu(0, UIMENU_SAVEERROR);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            for (i = 0; i < SND_GetEntChannelCount(); ++i)
                                                                v207[i] = SND_IsPausable(i);
                                                            SND_SetPauseSettings(v207);
                                                            SND_DeactivateChannelVolumes(1, 0);
                                                            UI_Popup(localClientNum, "victoryscreen");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        cgArray[0].deadquoteStartTime = 0;
                                                    }
                                                }
                                                else
                                                {
                                                    cgArray[0].deadquoteStartTime = cgArray[0].time;
                                                }
                                            }
                                            else
                                            {
                                                v67 = Cmd_Argv(1);
                                                v68 = SEH_LocalizeTextMessage(v67, "game message", LOCMSG_SAFE);
                                                CG_GameMessage(v68, 9);
                                            }
                                        }
                                        else
                                        {
                                            v62 = Cmd_Argv(1);
                                            v63 = SEH_LocalizeTextMessage(v62, "game message", LOCMSG_SAFE);
                                            CG_GameMessage(v63, 5);
                                        }
                                    }
                                    else
                                    {
                                        v57 = Cmd_Argv(1);
                                        v58 = SEH_LocalizeTextMessage(v57, "game message", LOCMSG_SAFE);
                                        CG_GameMessage(v58, 3);
                                    }
                                }
                                else
                                {
                                    v34 = Cmd_Argv(1);
                                    v35 = atol(v34);
                                    v36 = Cmd_Argv(2);
                                    v37 = atol(v36);
                                    v38 = Cmd_Argv(5);
                                    v39 = atof(v38);
                                    v40 = (float)*(double *)&v39;
                                    v41 = Cmd_Argv(4);
                                    v42 = atof(v41);
                                    v43 = (float)*(double *)&v42;
                                    v44 = Cmd_Argv(3);
                                    v45 = atof(v44);
                                    v205[1] = v43;
                                    v205[2] = v40;
                                    v205[0] = *(double *)&v45;
                                    v46 = Cmd_Argv(8);
                                    v47 = atof(v46);
                                    v48 = (float)*(double *)&v47;
                                    v49 = Cmd_Argv(7);
                                    v50 = atof(v49);
                                    v51 = (float)*(double *)&v50;
                                    v52 = Cmd_Argv(6);
                                    v53 = atof(v52);
                                    v206[1] = v51;
                                    v206[2] = v48;
                                    v206[0] = *(double *)&v53;
                                    DynEntCl_DestroyEvent(localClientNum, v35, v37, v205, v206);
                                }
                            }
                            else
                            {
                                v29 = Cmd_Argv(1);
                                v30 = SEH_LocalizeTextMessage(v29, "bold game message", LOCMSG_SAFE);
                                CG_BoldGameMessage(v30, 0);
                            }
                        }
                        else
                        {
                            v24 = Cmd_Argv(1);
                            v25 = SEH_LocalizeTextMessage(v24, "game message", LOCMSG_SAFE);
                            CG_GameMessage(v25, 0);
                        }
                    }
                    else
                    {
                        v19 = Cmd_Argv(1);
                        v20 = SEH_LocalizeTextMessage(v19, "server print", LOCMSG_SAFE);
                        Com_Printf(0, "%s\n", v20);
                    }
                }
                else
                {
                    CG_ConfigStringModified(localClientNum);
                }
            }
            else
            {
                v12 = Cmd_Argv(1);
                CG_CenterPrint(localClientNum, v12);
            }
        }
        else
        {
            v7 = Cmd_Argv(1);
            v8 = atol(v7);
            CG_SelectWeaponIndex(localClientNum, v8);
        }
    }
}

void __cdecl CG_ServerCommand(int localClientNum)
{
    CG_DispatchServerCommand(localClientNum);
    Cmd_EndTokenizedString();
}

void __cdecl CG_ExecuteNewServerCommands(int localClientNum, int latestSequence)
{
    int nesting; // r23
    int i; // r11

    nesting = cmd_args.nesting;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (i = cgArray[0].serverCommandSequence;
        cgArray[0].serverCommandSequence < latestSequence;
        i = cgArray[0].serverCommandSequence)
    {
        if (!cgArray[0].nextSnap)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp", 1354, 0, "%s", "cgameGlob->nextSnap");
            i = cgArray[0].serverCommandSequence;
        }
        cgArray[0].serverCommandSequence = i + 1;
        if (CL_CGameNeedsServerCommand(localClientNum, i + 1))
        {
            CG_DispatchServerCommand(localClientNum);
            Cmd_EndTokenizedString();
        }
        if (nesting != cmd_args.nesting)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_servercmds.cpp",
                1360,
                0,
                "%s",
                "nesting == cmd_args.nesting");
    }
}

void __cdecl CG_MapInit(int restart)
{
    signed int i; // r31
    int j; // r31
    const char *ConfigString; // r3
    long double v5; // fp2

    memset(cgArray, 0, sizeof(cgArray));
    memset(cg_entitiesArray, 0, sizeof(cg_entitiesArray));
    CG_ClearCompassPingData();
    cgArray[0].loaded = restart;
    cgArray[0].lastHealthLerpDelay = 1;
    CG_InitLocalEntities(0);
    DynEntCl_InitEntities(0);
    R_SwitchFog(0, cgArray[0].time, 0);
    R_InitPrimaryLights(cgArray[0].refdef.primaryLights);
    R_ClearShadowedPrimaryLightHistory(0);
    for (i = 11; i < 27; ++i)
        CG_ParseObjectiveChange(0, i);
    for (j = 27; j < 59; ++j)
        CG_TargetsChanged(0, j);
    ConfigString = CL_GetConfigString(0, 6u);
    v5 = atof(ConfigString);
    R_SetCullDist((float)*(double *)&v5);
    CG_NorthDirectionChanged(0);
    SND_MapInit();
    CG_StartAmbient(0);
    if (restart)
    {
        Ragdoll_Shutdown();
        Ragdoll_Init();
        Phys_Init();
    }
    FX_InitSystem(0);
    CG_VisionSetConfigString_Naked(0);
    CG_VisionSetConfigString_Night(0);
    UI_ClosePopupScriptMenu(0, 0);
    cg_waitingScriptMenu.name[0] = 0;
    CG_Respawn(0);
}

