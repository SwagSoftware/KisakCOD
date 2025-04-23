#include <qcommon/qcommon.h>

#include <Windows.h>
#include <win32/win_local.h>
#include <qcommon/cmd.h>
#include "com_files.h"
#include "com_memory.h"
#include <stringed/stringed_hooks.h>
#include "q_parse.h"
#include <client_mp/client_mp.h>
#include <gfx_d3d/r_dvars.h>
#include <win32/win_net.h>
#include <devgui/devgui.h>

#include <algorithm>

const dvar_s *dvar_cheats;
int dvar_modifiedFlags;

LONG isSortingDvars;

static FastCriticalSection g_dvarCritSect;

static dvar_s* dvarHashTable[0x100];
    
static dvar_s dvarPool[0x1000];
static const dvar_s* sortedDvars[0x1000];
static bool areDvarsSorted;
static LONG isSortedDvars;
static int dvarCount;

bool isDvarSystemActive;
bool isLoadingAutoExecGlobalFlag;

static int generateHashValue(const char* fname)
{
    if (!fname)
    {
        Com_Error(ERR_DROP, "null name in generateHashValue");
    }
    int hash = 0;
    for (int i = 0; fname[i]; ++i)
        hash += tolower(fname[i]) * (i + 119);
    return (unsigned __int8)hash;
}

int __cdecl Dvar_Command()
{
    const char *v0; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // [esp-4h] [ebp-100Ch]
    char combined[4096]; // [esp+0h] [ebp-1008h] BYREF
    dvar_s *dvar; // [esp+1004h] [ebp-4h]

    v0 = Cmd_Argv(0);
    dvar = (dvar_s *)Dvar_FindVar(v0);
    if (!dvar)
        return 0;
    if (Cmd_Argc() == 1)
    {
        v5 = Dvar_DisplayableResetValue(dvar);
        v2 = Dvar_DisplayableValue(dvar);
        Com_Printf(0, "\"%s\" is: \"%s^7\" default: \"%s^7\"\n", dvar->name, v2, v5);
        if (Dvar_HasLatchedValue(dvar))
        {
            v3 = Dvar_DisplayableLatchedValue(dvar);
            Com_Printf(0, "latched: \"%s\"\n", v3);
        }
        Dvar_PrintDomain(dvar->type, dvar->domain);
        return 1;
    }
    else
    {
        Dvar_GetCombinedString(combined, 1);
        v4 = Cmd_Argv(0);
        Dvar_SetCommand(v4, combined);
        return 1;
    }
}

void __cdecl Dvar_GetCombinedString(char *combined, int first)
{
    char *v2; // eax
    int c; // [esp+10h] [ebp-10h]
    int l; // [esp+14h] [ebp-Ch]
    int len; // [esp+18h] [ebp-8h]

    c = Cmd_Argc();
    *combined = 0;
    l = 0;
    while (first < c)
    {
        len = strlen(Cmd_Argv(first)) + 1;
        if (len + l >= 4094)
            break;
        v2 = (char *)Cmd_Argv(first);
        I_strncat(combined, 4096, v2);
        if (first != c - 1)
            I_strncat(combined, 4096, " ");
        l += len;
        ++first;
    }
}

void __cdecl Dvar_Toggle_f()
{
    Dvar_ToggleInternal();
}

bool __cdecl Dvar_ToggleInternal()
{
    const char *v0; // eax
    char *v2; // eax
    const char *string; // [esp+24h] [ebp-18h]
    int argIndex; // [esp+28h] [ebp-14h]
    const char *argString; // [esp+2Ch] [ebp-10h]
    char *argStringa; // [esp+2Ch] [ebp-10h]
    const char *dvarName; // [esp+30h] [ebp-Ch]
    dvar_s *dvar; // [esp+34h] [ebp-8h]
    const char *enumString; // [esp+38h] [ebp-4h]
    const char *enumStringa; // [esp+38h] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        dvarName = Cmd_Argv(1);
        if (!dvarName)
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 218, 0, "%s", "dvarName");
        dvar = (dvar_s *)Dvar_FindVar(dvarName);
        if (dvar)
        {
            if (Cmd_Argc() == 2)
            {
                return Dvar_ToggleSimple(dvar);
            }
            else
            {
                string = Dvar_DisplayableValue(dvar);
                for (argIndex = 2; argIndex + 1 < Cmd_Argc(); ++argIndex)
                {
                    argString = Cmd_Argv(argIndex);
                    if (dvar->type == 6)
                    {
                        enumString = Dvar_IndexStringToEnumString(dvar, argString);
                        if (strlen(enumString))
                            argString = enumString;
                    }
                    if (!I_stricmp(string, argString))
                    {
                        v2 = (char *)Cmd_Argv(argIndex + 1);
                        Dvar_SetCommand(dvarName, v2);
                        return 1;
                    }
                }
                argStringa = (char *)Cmd_Argv(2);
                if (dvar->type == 6)
                {
                    enumStringa = Dvar_IndexStringToEnumString(dvar, argStringa);
                    if (strlen(enumStringa))
                        argStringa = (char *)enumStringa;
                }
                Dvar_SetCommand(dvarName, argStringa);
                return 1;
            }
        }
        else
        {
            Com_Printf(0, "toggle failed: dvar '%s' not found.\n", dvarName);
            return 0;
        }
    }
    else
    {
        if (!Cmd_Argv(0))
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 212, 0, "%s", "Cmd_Argv( 0 )");
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <variable> <optional value sequence>\n", v0);
        return 0;
    }
}

bool __cdecl Dvar_ToggleSimple(dvar_s *dvar)
{
    bool result; // al
    const char *v2; // eax

    if (!dvar)
        MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 138, 0, "%s", "dvar");
    switch (dvar->type)
    {
    case 0u:
        Dvar_SetBoolFromSource(dvar, !dvar->current.enabled, DVAR_SOURCE_EXTERNAL);
        result = 1;
        break;
    case 1u:
        if (dvar->domain.value.min > 0.0 || dvar->domain.value.max < 1.0)
        {
            if (dvar->domain.value.min == dvar->current.value)
                Dvar_SetFloatFromSource(dvar, dvar->domain.value.max, DVAR_SOURCE_EXTERNAL);
            else
                Dvar_SetFloatFromSource(dvar, dvar->domain.value.min, DVAR_SOURCE_EXTERNAL);
        }
        else if (dvar->current.value == 0.0)
        {
            Dvar_SetFloatFromSource(dvar, 1.0, DVAR_SOURCE_EXTERNAL);
        }
        else
        {
            Dvar_SetFloatFromSource(dvar, 0.0, DVAR_SOURCE_EXTERNAL);
        }
        result = 1;
        break;
    case 2u:
    case 3u:
    case 4u:
    case 7u:
    case 8u:
        Com_Printf(0, "'toggle' with no arguments makes no sense for dvar '%s'\n", dvar->name);
        result = 0;
        break;
    case 5u:
        if (dvar->domain.enumeration.stringCount > 0 || dvar->domain.integer.max < 1)
        {
            if (dvar->current.integer == dvar->domain.enumeration.stringCount)
                Dvar_SetIntFromSource(dvar, dvar->domain.integer.max, DVAR_SOURCE_EXTERNAL);
            else
                Dvar_SetIntFromSource(dvar, dvar->domain.enumeration.stringCount, DVAR_SOURCE_EXTERNAL);
        }
        else if (dvar->current.integer)
        {
            Dvar_SetIntFromSource(dvar, 0, DVAR_SOURCE_EXTERNAL);
        }
        else
        {
            Dvar_SetIntFromSource(dvar, 1, DVAR_SOURCE_EXTERNAL);
        }
        result = 1;
        break;
    case 6u:
        if (dvar->domain.enumeration.stringCount)
            Dvar_SetIntFromSource(
                dvar,
                (dvar->current.integer + 1) % dvar->domain.enumeration.stringCount,
                DVAR_SOURCE_EXTERNAL);
        result = 1;
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("unhandled case %i", dvar->type);
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 195, 1, v2);
        }
        result = 0;
        break;
    }
    return result;
}

void __cdecl Dvar_TogglePrint_f()
{
    int v0; // eax
    const char *string; // [esp+0h] [ebp-Ch]
    const char *dvarName; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    if (Dvar_ToggleInternal())
    {
        if (Cmd_Argc() < 2)
        {
            v0 = Cmd_Argc();
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 292, 0, "%s\n\t(Cmd_Argc()) = %i", "(Cmd_Argc() >= 2)", v0);
        }
        dvarName = Cmd_Argv(1);
        if (!dvarName)
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 294, 1, "%s", "dvarName");
        dvar = Dvar_FindVar(dvarName);
        if (!dvar)
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 296, 1, "%s", "dvar");
        string = Dvar_DisplayableValue(dvar);
        if (!string)
            MyAssertHandler(".\\qcommon\\dvar_cmds.cpp", 298, 1, "%s", "string");
        Com_Printf(0, "%s toggled to %s\n", dvarName, string);
    }
}

void __cdecl Dvar_Set_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char combined[4096]; // [esp+4h] [ebp-1008h] BYREF
    char *dvarName; // [esp+1008h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        dvarName = (char *)Cmd_Argv(1);
        if (Dvar_IsValidName(dvarName))
        {
            Dvar_GetCombinedString(combined, 2);
            v1 = Cmd_Argv(1);
            Dvar_SetCommand(v1, combined);
        }
        else
        {
            v0 = Cmd_Argv(1);
            Com_Printf(0, "invalid variable name: %s\n", v0);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: set <variable> <value>\n");
    }
}

void __cdecl Dvar_SetU_f()
{
    const char *v0; // eax
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        Dvar_Set_f();
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if (dvar)
            Dvar_AddFlags(dvar, 2);
    }
    else
    {
        Com_Printf(0, "USAGE: setu <variable> <value>\n");
    }
}

void __cdecl Dvar_SetS_f()
{
    const char *v0; // eax
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        Dvar_Set_f();
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if (dvar)
            Dvar_AddFlags(dvar, 4);
    }
    else
    {
        Com_Printf(0, "USAGE: sets <variable> <value>\n");
    }
}

void __cdecl Dvar_SetA_f()
{
    const char *v0; // eax
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        Dvar_Set_f();
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if (dvar)
            Dvar_AddFlags(dvar, 1);
    }
    else
    {
        Com_Printf(0, "USAGE: seta <variable> <value>\n");
    }
}

void __cdecl Dvar_SetFromDvar_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *v3; // [esp-4h] [ebp-8h]
    const dvar_s *dvarSrc; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() == 3)
    {
        v0 = Cmd_Argv(2);
        dvarSrc = Dvar_FindVar(v0);
        if (dvarSrc)
        {
            v3 = (char *)Dvar_DisplayableValue(dvarSrc);
            v2 = Cmd_Argv(1);
            Dvar_SetCommand(v2, v3);
        }
        else
        {
            v1 = Cmd_Argv(2);
            Com_Printf(0, "dvar '%s' doesn't exist\n", v1);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: setfromdvar <dest_dvar> <source_dvar>\n");
    }
}

void __cdecl Dvar_Reset_f()
{
    const char *v0; // eax
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        dvar = (dvar_s *)Dvar_FindVar(v0);
        if (dvar)
            Dvar_Reset(dvar, DVAR_SOURCE_EXTERNAL);
    }
    else
    {
        Com_Printf(0, "USAGE: reset <variable>\n");
    }
}

void __cdecl Dvar_WriteVariables(int f)
{
    Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))Dvar_WriteSingleVariable, &f);
}

void __cdecl Dvar_WriteSingleVariable(const dvar_s *dvar, int *userData)
{
    const char *v2; // eax
    int f; // [esp+0h] [ebp-4h]

    if (I_stricmp(dvar->name, "cl_cdkey"))
    {
        if ((dvar->flags & 1) != 0)
        {
            f = *userData;
            v2 = Dvar_DisplayableLatchedValue(dvar);
            FS_Printf(f, "seta %s \"%s\"\n", dvar->name, v2);
        }
    }
}

void __cdecl Dvar_WriteDefaults(int f)
{
    Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))Dvar_WriteSingleDefault, &f);
}

void __cdecl Dvar_WriteSingleDefault(const dvar_s *dvar, int *userData)
{
    const char *v2; // eax
    int f; // [esp+0h] [ebp-4h]

    if (I_stricmp(dvar->name, "cl_cdkey"))
    {
        if ((dvar->flags & 0x40C0) == 0)
        {
            f = *userData;
            v2 = Dvar_DisplayableResetValue(dvar);
            FS_Printf(f, "set %s \"%s\"\n", dvar->name, v2);
        }
    }
}

void __cdecl Dvar_List_f()
{
    char *match; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() <= 1)
    {
        Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))Dvar_ListSingle, 0);
    }
    else
    {
        match = (char *)Cmd_Argv(1);
        Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))Dvar_ListSingle, match);
    }
    Com_Printf(0, "\n%i total dvars\n", dvarCount);
}

void __cdecl Dvar_ListSingle(const dvar_s *dvar, const char *userData)
{
    const char *v2; // eax

    if (!userData || Com_Filter(userData, (char *)dvar->name, 0))
    {
        if ((dvar->flags & 0x404) != 0)
            Com_Printf(0, "S");
        else
            Com_Printf(0, " ");
        if ((dvar->flags & 2) != 0)
            Com_Printf(0, "U");
        else
            Com_Printf(0, " ");
        if ((dvar->flags & 0x40) != 0)
            Com_Printf(0, "R");
        else
            Com_Printf(0, " ");
        if ((dvar->flags & 0x10) != 0)
            Com_Printf(0, "I");
        else
            Com_Printf(0, " ");
        if ((dvar->flags & 1) != 0)
            Com_Printf(0, "A");
        else
            Com_Printf(0, " ");
        if ((dvar->flags & 0x20) != 0)
            Com_Printf(0, "L");
        else
            Com_Printf(0, " ");
        if ((dvar->flags & 0x80) != 0)
            Com_Printf(0, "C");
        else
            Com_Printf(0, " ");
        v2 = Dvar_DisplayableValue(dvar);
        Com_Printf(0, " %s \"%s\"\n", dvar->name, v2);
    }
}

void __cdecl Dvar_Dump_f()
{
    const char *match; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() <= 1)
    {
        Com_DvarDump(0, 0);
    }
    else
    {
        match = Cmd_Argv(1);
        Com_DvarDump(0, match);
    }
}

void __cdecl PBdvar_set(const char *var_name, char *value)
{
    if (Dvar_FindVar(var_name))
        Dvar_SetFromStringByName(var_name, value);
}

char *__cdecl Dvar_InfoString(int localClientNum, char bit)
{
    const char *UsernameForLocalClient; // eax

    info1[0] = 0;
    Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))Dvar_InfoStringSingle, &bit);
    if ((bit & 2) != 0)
    {
        UsernameForLocalClient = CL_GetUsernameForLocalClient();
        Info_SetValueForKey(info1, "name", UsernameForLocalClient);
    }
    return info1;
}

void __cdecl Dvar_InfoStringSingle(const dvar_s *dvar, unsigned int *userData)
{
    const char *v2; // eax

    if ((*userData & dvar->flags) != 0)
    {
        v2 = Dvar_DisplayableValue(dvar);
        Info_SetValueForKey(info1, (char *)dvar->name, v2);
    }
}

char *__cdecl Dvar_InfoString_Big(int bit)
{
    info2[0] = 0;
    Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))Dvar_InfoStringSingle_Big, &bit);
    return info2;
}

void __cdecl Dvar_InfoStringSingle_Big(const dvar_s *dvar, unsigned int *userData)
{
    const char *v2; // eax

    if ((*userData & dvar->flags) != 0)
    {
        v2 = Dvar_DisplayableValue(dvar);
        Info_SetValueForKey_Big(info2, (char *)dvar->name, v2);
    }
}

cmd_function_s Dvar_Toggle_f_VAR;
cmd_function_s Dvar_TogglePrint_f_VAR;
cmd_function_s Dvar_Set_f_VAR;
cmd_function_s Dvar_SetS_f_VAR;
cmd_function_s Dvar_SetA_f_VAR;
cmd_function_s Dvar_SetFromDvar_f_VAR;
cmd_function_s Dvar_SetFromLocalizedStr_f_VAR;
cmd_function_s Dvar_SetToTime_f_VAR;
cmd_function_s Dvar_Reset_f_VAR;
cmd_function_s Dvar_List_f_VAR;
cmd_function_s Dvar_Dump_f_VAR;
cmd_function_s Dvar_RegisterBool_f_VAR;
cmd_function_s Dvar_RegisterInt_f_VAR;
cmd_function_s Dvar_RegisterFloat_f_VAR;
cmd_function_s Dvar_SetU_f_VAR;

void __cdecl Dvar_AddCommands()
{
    Cmd_AddCommandInternal("toggle", Dvar_Toggle_f, &Dvar_Toggle_f_VAR);
    Cmd_AddCommandInternal("togglep", Dvar_TogglePrint_f, &Dvar_TogglePrint_f_VAR);
    Cmd_AddCommandInternal("set", Dvar_Set_f, &Dvar_Set_f_VAR);
    Cmd_AddCommandInternal("sets", Dvar_SetS_f, &Dvar_SetS_f_VAR);
    Cmd_AddCommandInternal("seta", Dvar_SetA_f, &Dvar_SetA_f_VAR);
    Cmd_AddCommandInternal("setfromdvar", Dvar_SetFromDvar_f, &Dvar_SetFromDvar_f_VAR);
    Cmd_AddCommandInternal("setfromlocString", Dvar_SetFromLocalizedStr_f, &Dvar_SetFromLocalizedStr_f_VAR);
    Cmd_AddCommandInternal("setdvartotime", Dvar_SetToTime_f, &Dvar_SetToTime_f_VAR);
    Cmd_AddCommandInternal("reset", Dvar_Reset_f, &Dvar_Reset_f_VAR);
    Cmd_AddCommandInternal("dvarlist", Dvar_List_f, &Dvar_List_f_VAR);
    Cmd_AddCommandInternal("dvardump", Dvar_Dump_f, &Dvar_Dump_f_VAR);
    Cmd_AddCommandInternal("dvar_bool", Dvar_RegisterBool_f, &Dvar_RegisterBool_f_VAR);
    Cmd_AddCommandInternal("dvar_int", Dvar_RegisterInt_f, &Dvar_RegisterInt_f_VAR);
    Cmd_AddCommandInternal("dvar_float", Dvar_RegisterFloat_f, &Dvar_RegisterFloat_f_VAR);
    Cmd_AddCommandInternal("setu", Dvar_SetU_f, &Dvar_SetU_f_VAR);
}

void __cdecl Dvar_RegisterBool_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *dvarName; // [esp+0h] [ebp-10h]
    const dvar_s *dvar; // [esp+4h] [ebp-Ch]
    bool value; // [esp+Fh] [ebp-1h]

    if (Cmd_Argc() == 3)
    {
        dvarName = Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        value = atoi(v1) != 0;
        dvar = Dvar_FindVar(dvarName);
        if (!dvar || dvar->type == 7 && (dvar->flags & 0x4000) != 0)
        {
            Dvar_RegisterBool(dvarName, value, 0x4000u, "External Dvar");
        }
        else if (dvar->type)
        {
            Com_Printf(0, "dvar '%s' is not a boolean dvar\n", dvar->name);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <default>\n", v0);
    }
}

void __cdecl Dvar_RegisterInt_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int max; // [esp+0h] [ebp-18h]
    const char *dvarName; // [esp+4h] [ebp-14h]
    int min; // [esp+8h] [ebp-10h]
    const dvar_s *dvar; // [esp+Ch] [ebp-Ch]
    int value; // [esp+14h] [ebp-4h]

    if (Cmd_Argc() == 5)
    {
        dvarName = Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        value = atoi(v1);
        v2 = Cmd_Argv(3);
        min = atoi(v2);
        v3 = Cmd_Argv(4);
        max = atoi(v3);
        if (min <= max)
        {
            dvar = Dvar_FindVar(dvarName);
            if (!dvar || dvar->type == 7 && (dvar->flags & 0x4000) != 0)
            {
                Dvar_RegisterInt(dvarName, value, DvarLimits(max, min), 0x4000u, "External Dvar");
            }
            else if (dvar->type != 5 && dvar->type != 6)
            {
                Com_Printf(0, "dvar '%s' is not an integer dvar\n", dvar->name);
            }
        }
        else
        {
            Com_Printf(0, "dvar %s: min %i should not be greater than max %i\n", dvarName, min, max);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <default> <min> <max>\n", v0);
    }
}

void __cdecl Dvar_RegisterFloat_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    DvarLimits v4; // [esp+4h] [ebp-28h]
    float max; // [esp+14h] [ebp-18h]
    const char *dvarName; // [esp+18h] [ebp-14h]
    float min; // [esp+1Ch] [ebp-10h]
    const dvar_s *dvar; // [esp+20h] [ebp-Ch]
    float value; // [esp+28h] [ebp-4h]

    if (Cmd_Argc() == 5)
    {
        dvarName = Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        value = atof(v1);
        v2 = Cmd_Argv(3);
        min = atof(v2);
        v3 = Cmd_Argv(4);
        max = atof(v3);
        if (max >= (double)min)
        {
            dvar = Dvar_FindVar(dvarName);
            if (!dvar || dvar->type == 7 && (dvar->flags & 0x4000) != 0)
            {
                v4.value.max = max;
                v4.value.min = min;
                Dvar_RegisterFloat(dvarName, value, v4, 0x4000u, "External Dvar");
            }
            else if (dvar->type != 1)
            {
                Com_Printf(0, "dvar '%s' is not an integer dvar\n", dvar->name);
            }
        }
        else
        {
            Com_Printf(0, "dvar %s: min %g should not be greater than max %g\n", dvarName, min, max);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <default> <min> <max>\n", v0);
    }
}

void __cdecl Dvar_SetFromLocalizedStr_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char combined; // [esp+4h] [ebp-1010h] BYREF
    char pszInputBuffer[4099]; // [esp+5h] [ebp-100Fh] BYREF
    char *dvarName; // [esp+100Ch] [ebp-8h]
    char *src; // [esp+1010h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        dvarName = (char *)Cmd_Argv(1);
        if (Dvar_IsValidName(dvarName))
        {
            Dvar_GetCombinedString(&combined, 2);
            if (combined == 64)
            {
                src = SEH_LocalizeTextMessage(pszInputBuffer, "dvar string", LOCMSG_NOERR);
                if (src)
                {
                    if (*src)
                        I_strncpyz(&combined, src, 4096);
                }
            }
            v1 = Cmd_Argv(1);
            Dvar_SetCommand(v1, &combined);
        }
        else
        {
            v0 = Cmd_Argv(1);
            Com_Printf(0, "invalid variable name: %s\n", v0);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: setFromLocalizedString <variable> <string>\n");
    }
}

void __cdecl Dvar_SetToTime_f()
{
    const char *v0; // eax
    unsigned int v1; // eax
    const char *v2; // eax
    char *v3; // [esp-4h] [ebp-Ch]
    const char *dvarName; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        dvarName = Cmd_Argv(1);
        if (Dvar_IsValidName(dvarName))
        {
            v1 = Sys_Milliseconds();
            v3 = va("%i", v1);
            v2 = Cmd_Argv(1);
            Dvar_SetCommand(v2, v3);
        }
        else
        {
            v0 = Cmd_Argv(1);
            Com_Printf(0, "invalid variable name: %s\n", v0);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: set <variable>\n");
    }
}

void __cdecl CL_SelectStringTableEntryInDvar_f()
{
    const char *v0; // eax
    unsigned int v1; // eax
    const char *v2; // eax
    int v3; // eax
    const char *v4; // eax
    char *ColumnValueForRow; // [esp-4h] [ebp-18h]
    double rowCount; // [esp+4h] [ebp-10h]
    const StringTable *table; // [esp+Ch] [ebp-8h] BYREF
    int row; // [esp+10h] [ebp-4h]

    if (!r_reflectionProbeGenerate->current.enabled)
    {
        if (Cmd_Argc() >= 4)
        {
            v0 = Cmd_Argv(1);
            StringTable_GetAsset(v0, (XAssetHeader *)&table);
            v1 = Sys_Milliseconds();
            srand(v1);
            rowCount = (double)table->rowCount;
            row = (int)((double)rand() * rowCount / 32767.0);
            v2 = Cmd_Argv(2);
            v3 = atoi(v2);
            ColumnValueForRow = (char *)StringTable_GetColumnValueForRow(table, row, v3);
            v4 = Cmd_Argv(3);
            Dvar_SetStringByName(v4, ColumnValueForRow);
        }
        else
        {
            Com_Printf(16, "usage: selectStringTableEntryInDvar <tableFileName> <columnNum> <dvarName>");
        }
    }
}

void __cdecl Dvar_ForEach(void(__cdecl *callback)(const dvar_s *, void *), void *userData)
{
    int dvarIter; // [esp+4h] [ebp-4h]

    InterlockedIncrement(&g_dvarCritSect.readCount);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    if (!areDvarsSorted)
        Dvar_Sort();
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
        callback(sortedDvars[dvarIter], userData);
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
}

void Dvar_Sort()
{
    if (InterlockedCompareExchange(&isSortingDvars, 1, 0))
    {
        while (isSortingDvars)
            NET_Sleep(1);
    }
    else
    {
        //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
        //    (const GfxStaticModelDrawInst **)sortedDvars,
        //    (const GfxStaticModelDrawInst **)(4 * dvarCount + 230766384),
        //    (4 * dvarCount + 230766384 - (int)sortedDvars) >> 2,
        //    (bool(__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))Material_CachedShaderTextLess);
        std::sort(&sortedDvars[0], &sortedDvars[0x1000 - 1], Material_CachedShaderTextLess);
        areDvarsSorted = 1;
        isSortingDvars = 0;
    }
}

void __cdecl Dvar_ForEachName(void(__cdecl *callback)(const char *))
{
    int dvarIter; // [esp+4h] [ebp-4h]

    InterlockedIncrement(&g_dvarCritSect.readCount);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    if (!areDvarsSorted)
        Dvar_Sort();
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
        callback(sortedDvars[dvarIter]->name);
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
}

const dvar_s *__cdecl Dvar_GetAtIndex(unsigned int index)
{
    if (index >= dvarCount)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            125,
            0,
            "index doesn't index dvarCount\n\t%i not in [0, %i)",
            index,
            dvarCount);
    return &dvarPool[index];
}

void __cdecl Dvar_SetInAutoExec(bool inAutoExec)
{
    isLoadingAutoExecGlobalFlag = inAutoExec;
}

bool __cdecl Dvar_IsSystemActive()
{
    return isDvarSystemActive;
}

char __cdecl Dvar_IsValidName(const char *dvarName)
{
    char nameChar; // [esp+3h] [ebp-5h]
    int index; // [esp+4h] [ebp-4h]

    if (!dvarName)
        return 0;
    for (index = 0; dvarName[index]; ++index)
    {
        nameChar = dvarName[index];
        if (!isalnum(nameChar) && nameChar != 95)
            return 0;
    }
    return 1;
}

const char *__cdecl Dvar_EnumToString(const dvar_s *dvar)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 278, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 279, 0, "%s", "dvar->name");
    if (dvar->type != 6)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            280,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_ENUM)",
            dvar->name);
    if (!dvar->domain.integer.max)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            281,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->domain.enumeration.strings)",
            dvar->name);
    if ((dvar->current.integer < 0 || dvar->current.integer >= dvar->domain.enumeration.stringCount)
        && dvar->current.integer)
    {
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            282,
            0,
            "%s\n\t(dvar->current.integer) = %i",
            "(dvar->current.integer >= 0 && dvar->current.integer < dvar->domain.enumeration.stringCount || dvar->current.integer == 0)",
            dvar->current.integer);
    }
    if (dvar->domain.enumeration.stringCount)
        return *(const char **)(dvar->domain.integer.max + 4 * dvar->current.integer);
    else
        return "";
}

const char *__cdecl Dvar_IndexStringToEnumString(const dvar_s *dvar, const char *indexString)
{
    signed int v3; // [esp+0h] [ebp-1Ch]
    int enumIndex; // [esp+14h] [ebp-8h]
    int indexStringIndex; // [esp+18h] [ebp-4h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 296, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 297, 0, "%s", "dvar->name");
    if (dvar->type != 6)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            298,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_ENUM)",
            dvar->name);
    if (!dvar->domain.integer.max)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            299,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->domain.enumeration.strings)",
            dvar->name);
    if (!indexString)
        MyAssertHandler(".\\universal\\dvar.cpp", 300, 0, "%s\n\t(dvar->name) = %s", "(indexString)", dvar->name);
    if (!dvar->domain.enumeration.stringCount)
        return "";
    v3 = strlen(indexString);
    for (indexStringIndex = 0; indexStringIndex < v3; ++indexStringIndex)
    {
        if (!isdigit(indexString[indexStringIndex]))
            return "";
    }
    enumIndex = atoi(indexString);
    if (enumIndex >= 0 && enumIndex < dvar->domain.enumeration.stringCount)
        return *(const char **)(dvar->domain.integer.max + 4 * enumIndex);
    else
        return "";
}

const char *__cdecl Dvar_DisplayableValue(const dvar_s *dvar)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 519, 0, "%s", "dvar");
    return Dvar_ValueToString(dvar, dvar->current);
}

const char *__cdecl Dvar_ValueToString(const dvar_s *dvar, DvarValue value)
{
    const char *result; // eax
    const char *v3; // eax
    const char *v4; // [esp+30h] [ebp-Ch]

    switch (dvar->type)
    {
    case 0u:
        if (value.enabled)
            v4 = "1";
        else
            v4 = "0";
        result = v4;
        break;
    case 1u:
        result = va("%g", value.value);
        break;
    case 2u:
        result = va("%g %g", value.value, value.vector[1]);
        break;
    case 3u:
        result = va("%g %g %g", value.value, value.vector[1], value.vector[2]);
        break;
    case 4u:
        result = va("%g %g %g %g", value.value, value.vector[1], value.vector[2], value.vector[3]);
        break;
    case 5u:
        result = va("%i", value.integer);
        break;
    case 6u:
        if ((value.integer < 0 || value.integer >= dvar->domain.enumeration.stringCount) && value.integer)
            MyAssertHandler(
                ".\\universal\\dvar.cpp",
                346,
                0,
                "%s\n\t(value.integer) = %i",
                "(value.integer >= 0 && value.integer < dvar->domain.enumeration.stringCount || value.integer == 0)",
                value.integer);
        if (dvar->domain.enumeration.stringCount)
            result = *(const char **)(dvar->domain.integer.max + 4 * value.integer);
        else
            result = "";
        break;
    case 7u:
        if (!value.integer)
            MyAssertHandler(".\\universal\\dvar.cpp", 352, 0, "%s\n\t(dvar->name) = %s", "(value.string)", dvar->name);
        result = va("%s", value.string);
        break;
    case 8u:
        result = va(
            "%g %g %g %g",
            (double)value.color[0] * 0.003921568859368563,
            (double)value.color[1] * 0.003921568859368563,
            (double)value.color[2] * 0.003921568859368563,
            (double)value.color[3] * 0.003921568859368563);
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("unhandled dvar type '%i'", dvar->type);
            MyAssertHandler(".\\universal\\dvar.cpp", 357, 1, v3);
        }
        result = "";
        break;
    }
    return result;
}

const char *__cdecl Dvar_DisplayableResetValue(const dvar_s *dvar)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 527, 0, "%s", "dvar");
    return Dvar_ValueToString(dvar, dvar->reset);
}

const char *__cdecl Dvar_DisplayableLatchedValue(const dvar_s *dvar)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 535, 0, "%s", "dvar");
    return Dvar_ValueToString(dvar, dvar->latched);
}

char __cdecl Dvar_ValueInDomain(unsigned __int8 type, DvarValue value, DvarLimits domain)
{
    char result; // al
    const char *v4; // eax
    bool v5; // [esp+8h] [ebp-Ch]

    switch (type)
    {
    case 0u:
        if (value.color[0] != 1 && value.enabled)
            MyAssertHandler(".\\universal\\dvar.cpp", 636, 0, "%s", "value.enabled == true || value.enabled == false");
        result = 1;
        break;
    case 1u:
        result = domain.value.min <= (double)value.value && domain.value.max >= (double)value.value;
        break;
    case 2u:
        result = Dvar_VectorInDomain(&value.value, 2, domain.value.min, domain.value.max);
        break;
    case 3u:
        result = Dvar_VectorInDomain(&value.value, 3, domain.value.min, domain.value.max);
        break;
    case 4u:
        result = Dvar_VectorInDomain(&value.value, 4, domain.value.min, domain.value.max);
        break;
    case 5u:
        if (domain.enumeration.stringCount > domain.integer.max)
            MyAssertHandler(".\\universal\\dvar.cpp", 640, 0, "%s", "domain.integer.min <= domain.integer.max");
        result = value.integer >= domain.enumeration.stringCount && value.integer <= domain.integer.max;
        break;
    case 6u:
        v5 = value.integer >= 0 && value.integer < domain.enumeration.stringCount || !value.integer;
        result = v5;
        break;
    case 7u:
        result = 1;
        break;
    case 8u:
        result = 1;
        break;
    default:
        if (!alwaysfails)
        {
            v4 = va("unhandled dvar type '%i'", type);
            MyAssertHandler(".\\universal\\dvar.cpp", 676, 1, v4);
        }
        result = 0;
        break;
    }
    return result;
}

char __cdecl Dvar_VectorInDomain(const float *vector, int components, float min, float max)
{
    int channel; // [esp+0h] [ebp-4h]

    for (channel = 0; channel < components; ++channel)
    {
        if (min > (double)vector[channel])
            return 0;
        if (max < (double)vector[channel])
            return 0;
    }
    return 1;
}

const char *__fastcall Dvar_DomainToString(
    unsigned __int8 type,
    DvarLimits *domain,
    char *outBuffer,
    unsigned int outBufferLen)
{
    return Dvar_DomainToString_Internal(type, *domain, outBuffer, outBufferLen, 0);
}

const char *__cdecl Dvar_DomainToString_Internal(
    unsigned __int8 type,
    DvarLimits domain,
    char *outBuffer,
    unsigned int outBufferLen,
    int *outLineCount)
{
    const char *v4; // eax
    char *outBufferEnd; // [esp+14h] [ebp-10h]
    char *outBufferWalk; // [esp+18h] [ebp-Ch]
    int charsWritten; // [esp+1Ch] [ebp-8h]
    int charsWrittena; // [esp+1Ch] [ebp-8h]
    int stringIndex; // [esp+20h] [ebp-4h]

    if ((int)outBuffer <= 0)
        MyAssertHandler(".\\universal\\dvar.cpp", 708, 0, "%s", "outBufferLen > 0");

    outBufferEnd = (char *)(HIDWORD(outBuffer) + outBuffer);
    if (outLineCount)
        *outLineCount = 0;
    switch (type)
    {
    case 0u:
        _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is 0 or 1");
        break;
    case 1u:
        if (domain.value.min == -3.402823466385289e38)
        {
            if (domain.value.max == 3.402823466385289e38)
                _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any number");
            else
                _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any number %g or smaller", domain.value.max);
        }
        else if (domain.value.max == 3.402823466385289e38)
        {
            _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any number %g or bigger", domain.value.min);
        }
        else
        {
            _snprintf(
                (char *)outBuffer,
                HIDWORD(outBuffer),
                "Domain is any number from %g to %g",
                domain.value.min,
                domain.value.max);
        }
        break;
    case 2u:
        Dvar_VectorDomainToString(2, domain, outBuffer, outBufferLen);
        break;
    case 3u:
        Dvar_VectorDomainToString(3, domain, outBuffer, outBufferLen);
        break;
    case 4u:
        Dvar_VectorDomainToString(4, domain, outBuffer, outBufferLen);
        break;
    case 5u:
        if (domain.enumeration.stringCount == 0x80000000)
        {
            if (domain.integer.max == 0x7FFFFFFF)
                _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any integer");
            else
                _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any integer %i or smaller", domain.integer.max);
        }
        else if (domain.integer.max == 0x7FFFFFFF)
        {
            _snprintf(
                (char *)outBuffer,
                HIDWORD(outBuffer),
                "Domain is any integer %i or bigger",
                domain.enumeration.stringCount);
        }
        else
        {
            _snprintf(
                (char *)outBuffer,
                HIDWORD(outBuffer),
                "Domain is any integer from %i to %i",
                domain.enumeration.stringCount,
                domain.integer.max);
        }
        break;
    case 6u:
        charsWritten = _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is one of the following:");
        if (charsWritten >= 0)
        {
            outBufferWalk = (char *)(charsWritten + outBuffer);
            for (stringIndex = 0; stringIndex < domain.enumeration.stringCount; ++stringIndex)
            {
                charsWrittena = _snprintf(
                    outBufferWalk,
                    outBufferEnd - outBufferWalk,
                    "\n  %2i: %s",
                    stringIndex,
                    *(const char **)(domain.integer.max + 4 * stringIndex));
                if (charsWrittena < 0)
                    break;
                if (outLineCount)
                    ++*outLineCount;
                outBufferWalk += charsWrittena;
            }
        }
        break;
    case 7u:
        _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any text");
        break;
    case 8u:
        _snprintf((char *)outBuffer, HIDWORD(outBuffer), "Domain is any 4-component color, in RGBA format");
        break;
    default:
        if (!alwaysfails)
        {
            v4 = va("unhandled dvar type '%i'", type);
            MyAssertHandler(".\\universal\\dvar.cpp", 794, 1, v4);
        }
        *(_BYTE *)outBuffer = 0;
        break;
    }
    *(outBufferEnd - 1) = 0;
    return (const char *)outBuffer;
}

void __cdecl Dvar_VectorDomainToString(int components, DvarLimits domain, char *outBuffer, unsigned int outBufferLen)
{
    if (domain.value.min == -3.402823466385289e38)
    {
        if (domain.value.max == 3.402823466385289e38)
            _snprintf(outBuffer, outBufferLen, "Domain is any %iD vector", components);
        else
            _snprintf(
                outBuffer,
                outBufferLen,
                "Domain is any %iD vector with components %g or smaller",
                components,
                domain.value.max);
    }
    else if (domain.value.max == 3.402823466385289e38)
    {
        _snprintf(
            outBuffer,
            outBufferLen,
            "Domain is any %iD vector with components %g or bigger",
            components,
            domain.value.min);
    }
    else
    {
        _snprintf(
            outBuffer,
            outBufferLen,
            "Domain is any %iD vector with components from %g to %g",
            components,
            domain.value.min,
            domain.value.max);
    }
}

const char *Dvar_DomainToString_GetLines(
    unsigned __int8 type,
    DvarLimits *domain,
    char *outBuffer,
    unsigned int outBufferLen,
    int *outLineCount)
{
    if (!outLineCount)
        MyAssertHandler(".\\universal\\dvar.cpp", 812, 0, "%s", "outLineCount");
    return Dvar_DomainToString_Internal(type, *domain, outBuffer, outBufferLen, outLineCount);
}

void __cdecl Dvar_PrintDomain(unsigned __int8 type, DvarLimits domain)
{
    //const char *v2; // eax
    //__int64 v3; // [esp-8h] [ebp-410h]
    //char domainBuffer[1024]; // [esp+0h] [ebp-408h] BYREF
    //
    //HIDWORD(v3) = 1024;
    //LODWORD(v3) = domainBuffer;
    //v2 = Dvar_DomainToString(type, domain, v3);
    //Com_Printf(16, "  %s\n", v2);
}

bool __cdecl Dvar_HasLatchedValue(const dvar_s *dvar)
{
    return Dvar_ValuesEqual(dvar->type, dvar->current, dvar->latched) == 0;
}

int __cdecl Dvar_ValuesEqual(unsigned __int8 type, DvarValue val0, DvarValue val1)
{
    int result; // eax
    const char *v4; // eax
    bool v5; // [esp+14h] [ebp-14h]
    bool v6; // [esp+18h] [ebp-10h]

    switch (type)
    {
    case 0u:
        result = val0.color[0] == val1.color[0];
        break;
    case 1u:
        result = val1.value == val0.value;
        break;
    case 2u:
        v6 = val1.value == val0.value && val1.vector[1] == val0.vector[1];
        result = v6;
        break;
    case 3u:
        v5 = val1.value == val0.value && val1.vector[1] == val0.vector[1] && val1.vector[2] == val0.vector[2];
        result = v5;
        break;
    case 4u:
        result = Vec4Compare(&val0.value, &val1.value);
        break;
    case 5u:
        result = val0.integer == val1.integer;
        break;
    case 6u:
        result = val0.integer == val1.integer;
        break;
    case 7u:
        if (!val0.integer)
            MyAssertHandler(".\\universal\\dvar.cpp", 853, 0, "%s", "val0.string");
        if (!val1.integer)
            MyAssertHandler(".\\universal\\dvar.cpp", 854, 0, "%s", "val1.string");
        result = strcmp(val0.string, val1.string) == 0;
        break;
    case 8u:
        result = val0.integer == val1.integer;
        break;
    default:
        if (!alwaysfails)
        {
            v4 = va("unhandled dvar type '%i'", type);
            MyAssertHandler(".\\universal\\dvar.cpp", 859, 1, v4);
        }
        result = 0;
        break;
    }
    return result;
}

static dvar_s *__cdecl Dvar_FindMalleableVar(const char *dvarName)
{
    dvar_s *var; // [esp+8h] [ebp-8h]

    InterlockedIncrement(&g_dvarCritSect.readCount);

    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);

    for (var = dvarHashTable[generateHashValue(dvarName)]; var; var = var->hashNext)
    {
        if (!I_stricmp(dvarName, var->name))
        {
            if (g_dvarCritSect.readCount <= 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
                    76,
                    0,
                    "%s",
                    "critSect->readCount > 0");
            InterlockedDecrement(&g_dvarCritSect.readCount);
            return var;
        }
    }
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
    return 0;
}

const dvar_s *__cdecl Dvar_FindVar(const char *dvarName)
{
    return Dvar_FindMalleableVar(dvarName);
}

void __cdecl Dvar_ClearModified(dvar_s *dvar)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1100, 0, "%s", "dvar");
    dvar->modified = 0;
}

void __cdecl Dvar_SetModified(dvar_s *dvar)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1107, 0, "%s", "dvar");
    dvar->modified = 1;
}

void __cdecl Dvar_UpdateEnumDomain(dvar_s *dvar, const char **stringTable)
{
    const char *v2; // eax
    const char *v3; // eax
    DvarValue result; // [esp+0h] [ebp-28h] BYREF
    DvarValue v5; // [esp+10h] [ebp-18h]
    int stringCount; // [esp+20h] [ebp-8h]
    dvar_s *malleableDvar; // [esp+24h] [ebp-4h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1117, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1118, 0, "%s", "dvar->name");
    if (!stringTable)
        MyAssertHandler(".\\universal\\dvar.cpp", 1119, 0, "%s\n\t(dvar->name) = %s", "(stringTable)", dvar->name);
    if (dvar->type != 6)
    {
        v2 = va("dvar %s type %i", dvar->name, dvar->type);
        MyAssertHandler(".\\universal\\dvar.cpp", 1120, 0, "%s\n\t%s", "dvar->type == DVAR_TYPE_ENUM", v2);
    }
    for (stringCount = 0; stringTable[stringCount]; ++stringCount)
        ;
    if (dvar->reset.integer < 0 || dvar->reset.integer >= stringCount && dvar->reset.integer)
    {
        v3 = va("name %i reset %i count %i", dvar->name, dvar->reset.integer, stringCount);
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1125,
            0,
            "%s\n\t%s",
            "dvar->reset.integer >= 0 && (dvar->reset.integer < stringCount || dvar->reset.integer == 0)",
            v3);
    }
    malleableDvar = dvar;
    dvar->domain.enumeration.stringCount = stringCount;
    malleableDvar->domain.integer.max = (int)stringTable;
    v5 = *Dvar_ClampValueToDomain(&result, dvar->type, dvar->current, dvar->reset, dvar->domain);
    malleableDvar->current = v5;
    malleableDvar->latched = dvar->current;
}

DvarValue *__cdecl Dvar_ClampValueToDomain(
    DvarValue *result,
    unsigned __int8 type,
    DvarValue value,
    DvarValue resetValue,
    DvarLimits domain)
{
    const char *v5; // eax

    switch (type)
    {
    case 0u:
        value.enabled = value.color[0] != 0;
        break;
    case 1u:
        if (domain.value.min <= (double)value.value)
        {
            if (domain.value.max < (double)value.value)
                value.value = domain.value.max;
        }
        else
        {
            value.value = domain.value.min;
        }
        break;
    case 2u:
        Dvar_ClampVectorToDomain(&value.value, 2, domain.value.min, domain.value.max);
        break;
    case 3u:
        Dvar_ClampVectorToDomain(&value.value, 3, domain.value.min, domain.value.max);
        break;
    case 4u:
        Dvar_ClampVectorToDomain(&value.value, 4, domain.value.min, domain.value.max);
        break;
    case 5u:
        if (domain.enumeration.stringCount > domain.integer.max)
            MyAssertHandler(".\\universal\\dvar.cpp", 579, 0, "%s", "domain.integer.min <= domain.integer.max");
        if (value.integer >= domain.enumeration.stringCount)
        {
            if (value.integer > domain.integer.max)
                value.integer = domain.integer.max;
        }
        else
        {
            value.integer = domain.enumeration.stringCount;
        }
        break;
    case 6u:
        if (value.integer < 0 || value.integer >= domain.enumeration.stringCount)
        {
            value.integer = resetValue.integer;
            if (resetValue.integer < 0 || value.integer >= domain.enumeration.stringCount)
            {
                if (value.integer)
                    MyAssertHandler(
                        ".\\universal\\dvar.cpp",
                        613,
                        0,
                        "%s\n\t(value.integer) = %i",
                        "(value.integer >= 0 && value.integer < domain.enumeration.stringCount || value.integer == 0)",
                        value.integer);
            }
        }
        break;
    case 7u:
    case 8u:
        break;
    default:
        if (!alwaysfails)
        {
            v5 = va("unhandled dvar type '%i'", type);
            MyAssertHandler(".\\universal\\dvar.cpp", 623, 1, v5);
        }
        break;
    }
    *result = value;
    return result;
}

void __cdecl Dvar_ClampVectorToDomain(float *vector, int components, float min, float max)
{
    int channel; // [esp+0h] [ebp-4h]

    for (channel = 0; channel < components; ++channel)
    {
        if (min <= (double)vector[channel])
        {
            if (max < (double)vector[channel])
                vector[channel] = max;
        }
        else
        {
            vector[channel] = min;
        }
    }
}

bool __cdecl Dvar_GetBool(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (!dvar)
        return 0;
    if (dvar->type && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1143,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_bool || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->type);
    if (dvar->type)
        return Dvar_StringToBool(dvar->current.string);
    else
        return dvar->current.enabled;
}

bool __cdecl Dvar_StringToBool(const char *string)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 365, 0, "%s", "string");
    return atoi(string) != 0;
}

int __cdecl Dvar_GetInt(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (!dvar)
        return 0;
    if (dvar->type != 5 && dvar->type != 6 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1159,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_ENUM || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->type);
    if (dvar->type == 5 || dvar->type == 6)
        return dvar->current.integer;
    else
        return Dvar_StringToInt(dvar->current.string);
}

int __cdecl Dvar_StringToInt(const char *string)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 372, 0, "%s", "string");
    return atoi(string);
}

double __cdecl Dvar_GetFloat(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (!dvar)
        return 0.0;
    if (dvar->type != 1 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1175,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_FLOAT || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->type);
    if (dvar->type == 1)
        return dvar->current.value;
    else
        return Dvar_StringToFloat(dvar->current.string);
}

double __cdecl Dvar_StringToFloat(const char *string)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 379, 0, "%s", "string");
    return (float)atof(string);
}

const char *__cdecl Dvar_GetString(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (!dvar)
        return "";
    if (dvar->type != 7 && dvar->type != 6)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1207,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_STRING || dvar->type == DVAR_TYPE_ENUM)",
            dvar->type);
    if (dvar->type == 6)
        return Dvar_EnumToString(dvar);
    else
        return dvar->current.string;
}

const char *__cdecl Dvar_GetVariantString(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (dvar)
        return Dvar_ValueToString(dvar, dvar->current);
    else
        return "";
}

void __cdecl Dvar_GetUnpackedColor(const dvar_s *dvar, float *expandedColor)
{
    unsigned __int8 color[4]; // [esp+10h] [ebp-4h] BYREF

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1230, 0, "%s", "dvar");
    if (dvar->type != 8 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1231,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_COLOR || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->type);
    if (dvar->type == 8)
        *(unsigned int *)color = dvar->current.integer;
    else
        Dvar_StringToColor(dvar->current.string, color);
    *expandedColor = (double)color[0] * 0.003921568859368563;
    expandedColor[1] = (double)color[1] * 0.003921568859368563;
    expandedColor[2] = (double)color[2] * 0.003921568859368563;
    expandedColor[3] = (double)color[3] * 0.003921568859368563;
}

void __cdecl Dvar_StringToColor(const char *string, unsigned __int8 *color)
{
    float v2; // [esp+0h] [ebp-A0h]
    float v3; // [esp+4h] [ebp-9Ch]
    float v4; // [esp+8h] [ebp-98h]
    float v5; // [esp+Ch] [ebp-94h]
    float v6; // [esp+10h] [ebp-90h]
    float v7; // [esp+14h] [ebp-8Ch]
    float v8; // [esp+18h] [ebp-88h]
    float v9; // [esp+1Ch] [ebp-84h]
    float v10; // [esp+20h] [ebp-80h]
    float v11; // [esp+24h] [ebp-7Ch]
    float v12; // [esp+28h] [ebp-78h]
    float v13; // [esp+2Ch] [ebp-74h]
    float v14; // [esp+34h] [ebp-6Ch]
    float v15; // [esp+44h] [ebp-5Ch]
    float v16; // [esp+4Ch] [ebp-54h]
    float v17; // [esp+5Ch] [ebp-44h]
    float v18; // [esp+64h] [ebp-3Ch]
    float v19; // [esp+74h] [ebp-2Ch]
    float v20; // [esp+7Ch] [ebp-24h]
    float v21; // [esp+8Ch] [ebp-14h]
    float colorVec[4]; // [esp+90h] [ebp-10h] BYREF

    colorVec[0] = 0.0;
    colorVec[1] = 0.0;
    colorVec[2] = 0.0;
    colorVec[3] = 0.0;
    sscanf(string, "%g %g %g %g", colorVec, &colorVec[1], &colorVec[2], &colorVec[3]);
    v13 = colorVec[0] - 1.0;
    if (v13 < 0.0)
        v21 = colorVec[0];
    else
        v21 = 1.0;
    v12 = 0.0 - v21;
    if (v12 < 0.0)
        v11 = v21;
    else
        v11 = 0.0;
    v20 = v11 * 255.0;
    *color = (int)(v20 + 9.313225746154785e-10);
    v10 = colorVec[1] - 1.0;
    if (v10 < 0.0)
        v19 = colorVec[1];
    else
        v19 = 1.0;
    v9 = 0.0 - v19;
    if (v9 < 0.0)
        v8 = v19;
    else
        v8 = 0.0;
    v18 = v8 * 255.0;
    color[1] = (int)(v18 + 9.313225746154785e-10);
    v7 = colorVec[2] - 1.0;
    if (v7 < 0.0)
        v17 = colorVec[2];
    else
        v17 = 1.0;
    v6 = 0.0 - v17;
    if (v6 < 0.0)
        v5 = v17;
    else
        v5 = 0.0;
    v16 = v5 * 255.0;
    color[2] = (int)(v16 + 9.313225746154785e-10);
    v4 = colorVec[3] - 1.0;
    if (v4 < 0.0)
        v15 = colorVec[3];
    else
        v15 = 1.0;
    v3 = 0.0 - v15;
    if (v3 < 0.0)
        v2 = v15;
    else
        v2 = 0.0;
    v14 = v2 * 255.0;
    color[3] = (int)(v14 + 9.313225746154785e-10);
}

void __cdecl Dvar_GetUnpackedColorByName(const char *dvarName, float *expandedColor)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (dvar)
    {
        Dvar_GetUnpackedColor(dvar, expandedColor);
    }
    else
    {
        *expandedColor = 1.0;
        expandedColor[1] = 1.0;
        expandedColor[2] = 1.0;
        expandedColor[3] = 1.0;
    }
}

void __cdecl Dvar_Shutdown()
{
    int dvarIter; // [esp+0h] [ebp-8h]
    dvar_s *dvar; // [esp+4h] [ebp-4h]

    Sys_LockWrite(&g_dvarCritSect);
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
    {
        dvar = &dvarPool[dvarIter];
        if (dvar->type == 7)
        {
            if (Dvar_ShouldFreeCurrentString(dvar))
                Dvar_FreeString(&dvar->current);
            dvar->current.integer = 0;
            if (Dvar_ShouldFreeResetString(dvar))
                Dvar_FreeString(&dvar->reset);
            dvar->reset.integer = 0;
            if (Dvar_ShouldFreeLatchedString(dvar))
                Dvar_FreeString(&dvar->latched);
            dvar->latched.integer = 0;
        }
        if ((dvar->flags & 0x4000) != 0)
            Dvar_FreeNameString(dvar->name);
    }
    dvarCount = 0;
    dvar_cheats = 0;
    dvar_modifiedFlags = 0;
    isDvarSystemActive = 0;
    memset((unsigned __int8 *)dvarHashTable, 0, sizeof(dvarHashTable));
    Sys_UnlockWrite(&g_dvarCritSect);
}

void __cdecl Dvar_FreeNameString(const char *name)
{
    FreeString(name);
}

bool __cdecl Dvar_ShouldFreeCurrentString(dvar_s *dvar)
{
    return dvar->current.integer
        && dvar->current.integer != dvar->latched.integer
        && dvar->current.integer != dvar->reset.integer;
}

bool __cdecl Dvar_ShouldFreeLatchedString(dvar_s *dvar)
{
    return dvar->latched.integer
        && dvar->latched.integer != dvar->current.integer
        && dvar->latched.integer != dvar->reset.integer;
}

bool __cdecl Dvar_ShouldFreeResetString(dvar_s *dvar)
{
    return dvar->reset.integer
        && dvar->reset.integer != dvar->current.integer
        && dvar->reset.integer != dvar->latched.integer;
}

void __cdecl Dvar_FreeString(DvarValue *value)
{
    FreeString(value->string);
    value->integer = 0;
}

void __cdecl Dvar_ChangeResetValue(dvar_s *dvar, DvarValue value)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1379, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1380, 0, "%s", "dvar->name");
    if ((dvar->flags & 0x200) == 0)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1381,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->flags & (1 << 9))",
            dvar->name);
    Dvar_UpdateResetValue(dvar, value);
}

void __cdecl Dvar_UpdateResetValue(dvar_s *dvar, DvarValue value)
{
    DvarValue oldString; // [esp+10h] [ebp-28h] BYREF
    bool shouldFree; // [esp+23h] [ebp-15h]
    DvarValue resetString; // [esp+24h] [ebp-14h] BYREF

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1340, 0, "%s", "dvar");
    switch (dvar->type)
    {
    case 2u:
        dvar->reset.value = value.value;
        dvar->reset.vector[1] = value.vector[1];
        break;
    case 3u:
        dvar->reset.value = value.value;
        dvar->reset.vector[1] = value.vector[1];
        dvar->reset.vector[2] = value.vector[2];
        break;
    case 4u:
        dvar->reset = value;
        break;
    case 7u:
        if (dvar->reset.integer != value.integer)
        {
            shouldFree = Dvar_ShouldFreeResetString(dvar);
            if (shouldFree)
                oldString.integer = dvar->reset.integer;
            Dvar_AssignResetStringValue(dvar, &resetString, (char *)value.integer);
            dvar->reset.integer = resetString.integer;
            if (shouldFree)
                Dvar_FreeString(&oldString);
        }
        break;
    default:
        dvar->reset = value;
        break;
    }
}

void __cdecl Dvar_AssignResetStringValue(dvar_s *dvar, DvarValue *dest, const char *string)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 266, 0, "%s", "string");
    if (dvar->current.integer && (string == (char *)dvar->current.integer || !strcmp(string, dvar->current.string)))
    {
        Dvar_WeakCopyString(dvar->current.string, dest);
    }
    else if (dvar->latched.integer && (string == (char *)dvar->latched.integer || !strcmp(string, dvar->latched.string)))
    {
        Dvar_WeakCopyString(dvar->latched.string, dest);
    }
    else
    {
        Dvar_CopyString(string, dest);
    }
}

void __cdecl Dvar_CopyString(const char *string, DvarValue *value)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 203, 0, "%s", "string");
    value->integer = (int)CopyString(string);
}

void __cdecl Dvar_WeakCopyString(const char *string, DvarValue *value)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 210, 0, "%s", "string");
    value->integer = (int)string;
}

void __cdecl Dvar_MakeLatchedValueCurrent(dvar_s *dvar)
{
    Dvar_SetVariant(dvar, dvar->latched, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetVariant(dvar_s *dvar, DvarValue value, DvarSetSource source)
{
    const char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *name; // [esp-4h] [ebp-48h]
    const char *v8; // [esp-4h] [ebp-48h]
    bool shouldFreeString; // [esp+1Fh] [ebp-25h]
    DvarValue oldString; // [esp+20h] [ebp-24h] BYREF
    DvarValue currentString; // [esp+30h] [ebp-14h] BYREF

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 925, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 926, 0, "%s", "dvar->name");
    if (Com_LogFileOpen())
    {
        v3 = Dvar_ValueToString(dvar, value);
        v4 = va("      dvar set %s %s\n", dvar->name, v3);
        Com_PrintMessage(6, v4, 0);
    }
    if (!Dvar_ValueInDomain(dvar->type, value, dvar->domain))
    {
        name = dvar->name;
        v5 = Dvar_ValueToString(dvar, value);
        Com_Printf(16, "'%s' is not a valid value for dvar '%s'\n", v5, name);
        Dvar_PrintDomain(dvar->type, dvar->domain);
        if (dvar->type == 6)
        {
            if (!Dvar_ValueInDomain(dvar->type, dvar->reset, dvar->domain))
                MyAssertHandler(
                    ".\\universal\\dvar.cpp",
                    955,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(Dvar_ValueInDomain( dvar->type, dvar->reset, dvar->domain ))",
                    dvar->name);
            Dvar_SetVariant(dvar, dvar->reset, source);
        }
        return;
    }
    if (dvar->domainFunc
        && !((unsigned __int8(__cdecl *)(dvar_s *, int, unsigned int, unsigned int, unsigned int))dvar->domainFunc)(
            dvar,
            value.integer,
            LODWORD(value.vector[1]),
            LODWORD(value.vector[2]),
            LODWORD(value.vector[3])))
    {
        v8 = dvar->name;
        v6 = Dvar_ValueToString(dvar, value);
        Com_Printf(16, "'%s' is not a valid value for dvar '%s'\n\n", v6, v8);
        return;
    }
    if (source == DVAR_SOURCE_EXTERNAL || source == DVAR_SOURCE_SCRIPT)
    {
        if ((dvar->flags & 0x40) != 0)
        {
            Com_Printf(16, "%s is read only.\n", dvar->name);
            return;
        }
        if ((dvar->flags & 0x10) != 0)
        {
            Com_Printf(16, "%s is write protected.\n", dvar->name);
            return;
        }
        if (source == DVAR_SOURCE_EXTERNAL && (dvar->flags & 0x80) != 0 && !dvar_cheats->current.enabled)
        {
            Com_Printf(16, "%s is cheat protected.\n", dvar->name);
            return;
        }
        if ((dvar->flags & 0x20) != 0)
        {
            Dvar_SetLatchedValue(dvar, value);
            if (!Dvar_ValuesEqual(dvar->type, dvar->latched, dvar->current))
                Com_Printf(16, "%s will be changed upon restarting.\n", dvar->name);
            return;
        }
    }
    else if (source == DVAR_SOURCE_DEVGUI && (dvar->flags & 0x800) != 0)
    {
        Dvar_SetLatchedValue(dvar, value);
        return;
    }
    if (Dvar_ValuesEqual(dvar->type, dvar->current, value))
    {
        Dvar_SetLatchedValue(dvar, dvar->current);
    }
    else
    {
        dvar_modifiedFlags |= dvar->flags;
        switch (dvar->type)
        {
        case 2u:
            dvar->current.value = value.value;
            dvar->current.vector[1] = value.vector[1];
            dvar->latched.value = value.value;
            dvar->latched.vector[1] = value.vector[1];
            break;
        case 3u:
            dvar->current.value = value.value;
            dvar->current.vector[1] = value.vector[1];
            dvar->current.vector[2] = value.vector[2];
            dvar->latched.value = value.value;
            dvar->latched.vector[1] = value.vector[1];
            dvar->latched.vector[2] = value.vector[2];
            break;
        case 4u:
            dvar->current = value;
            dvar->latched = value;
            break;
        case 7u:
            if (!dvar->name)
                MyAssertHandler(".\\universal\\dvar.cpp", 1020, 0, "%s", "dvar->name");
            if (value.integer == dvar->current.integer
                && value.integer != dvar->latched.integer
                && value.integer != dvar->reset.integer)
            {
                MyAssertHandler(
                    ".\\universal\\dvar.cpp",
                    1021,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(value.string != dvar->current.string || value.string == dvar->latched.string || value.string == dvar->reset.string)",
                    dvar->name);
            }
            shouldFreeString = Dvar_ShouldFreeCurrentString(dvar);
            if (shouldFreeString)
                oldString.integer = dvar->current.integer;
            Dvar_AssignCurrentStringValue(dvar, &currentString, (char*)value.string);
            dvar->current.integer = currentString.integer;
            if (Dvar_ShouldFreeLatchedString(dvar))
                Dvar_FreeString(&dvar->latched);
            dvar->latched.integer = 0;
            Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
            if (shouldFreeString)
                Dvar_FreeString(&oldString);
            break;
        default:
            dvar->current = value;
            dvar->latched = value;
            break;
        }
        dvar->modified = 1;
    }
}

void __cdecl Dvar_AssignCurrentStringValue(dvar_s *dvar, DvarValue *dest, char *string)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 242, 0, "%s", "string");
    if (dvar->latched.integer && (string == (char *)dvar->latched.integer || !strcmp(string, dvar->latched.string)))
    {
        Dvar_WeakCopyString(dvar->latched.string, dest);
    }
    else if (dvar->reset.integer && (string == (char *)dvar->reset.integer || !strcmp(string, dvar->reset.string)))
    {
        Dvar_WeakCopyString(dvar->reset.string, dest);
    }
    else
    {
        Dvar_CopyString(string, dest);
    }
}

void __cdecl Dvar_SetLatchedValue(dvar_s *dvar, DvarValue value)
{
    DvarValue latchedString; // [esp+10h] [ebp-28h] BYREF
    DvarValue oldString; // [esp+20h] [ebp-18h] BYREF
    bool shouldFree; // [esp+33h] [ebp-5h]

    switch (dvar->type)
    {
    case 2u:
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        break;
    case 3u:
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        dvar->latched.vector[2] = value.vector[2];
        break;
    case 4u:
        dvar->latched = value;
        break;
    case 7u:
        if (dvar->latched.integer != value.integer)
        {
            shouldFree = Dvar_ShouldFreeLatchedString(dvar);
            if (shouldFree)
                oldString.integer = dvar->latched.integer;
            Dvar_AssignLatchedStringValue(dvar, &latchedString, (char*)value.string);
            dvar->latched.integer = latchedString.integer;
            if (shouldFree)
                Dvar_FreeString(&oldString);
        }
        break;
    default:
        dvar->latched = value;
        break;
    }
}

void __cdecl Dvar_AssignLatchedStringValue(dvar_s *dvar, DvarValue *dest, char *string)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 254, 0, "%s", "string");
    if (dvar->current.integer && (string == (char *)dvar->current.integer || !strcmp(string, dvar->current.string)))
    {
        Dvar_WeakCopyString(dvar->current.string, dest);
    }
    else if (dvar->reset.integer && (string == (char *)dvar->reset.integer || !strcmp(string, dvar->reset.string)))
    {
        Dvar_WeakCopyString(dvar->reset.string, dest);
    }
    else
    {
        Dvar_CopyString(string, dest);
    }
}

void __cdecl Dvar_ClearLatchedValue(dvar_s *dvar)
{
    if (Dvar_HasLatchedValue(dvar))
        Dvar_SetLatchedValue(dvar, dvar->current);
}

const dvar_s *__cdecl Dvar_RegisterBool(
    const char *dvarName,
    bool value,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue dvarValue; // [esp+8h] [ebp-14h]

    dvarValue.enabled = value;
    return Dvar_RegisterVariant(dvarName, 0, flags, dvarValue, 0LL, description);
}

const dvar_s *__cdecl Dvar_RegisterNew(
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue value,
    DvarLimits domain,
    const char *description)
{
    int HashValue; // eax
    dvar_s *dvar; // [esp+2Ch] [ebp-8h]

    Sys_LockWrite(&g_dvarCritSect);
    if (dvarCount >= 4096)
    {
        Sys_UnlockWrite(&g_dvarCritSect);
        Com_Error(ERR_FATAL, "Can't create dvar '%s': %i dvars already exist", dvarName, 4096);
    }
    dvar = &dvarPool[dvarCount];
    sortedDvars[dvarCount] = dvar;
    areDvarsSorted = 0;
    ++dvarCount;
    dvar->type = type;
    if ((flags & 0x4000) != 0)
        dvar->name = Dvar_AllocNameString(dvarName);
    else
        dvar->name = dvarName;
    switch (type)
    {
    case 2u:
        dvar->current.value = value.value;
        dvar->current.vector[1] = value.vector[1];
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        dvar->reset.value = value.value;
        dvar->reset.vector[1] = value.vector[1];
        break;
    case 3u:
        dvar->current.value = value.value;
        dvar->current.vector[1] = value.vector[1];
        dvar->current.vector[2] = value.vector[2];
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        dvar->latched.vector[2] = value.vector[2];
        dvar->reset.value = value.value;
        dvar->reset.vector[1] = value.vector[1];
        dvar->reset.vector[2] = value.vector[2];
        break;
    case 4u:
        dvar->current = value;
        dvar->latched = value;
        dvar->reset = value;
        break;
    case 7u:
        Dvar_CopyString(value.string, &dvar->current);
        Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
        Dvar_WeakCopyString(dvar->current.string, &dvar->reset);
        break;
    default:
        dvar->current = value;
        dvar->latched = value;
        dvar->reset = value;
        break;
    }
    dvar->domain = domain;
    dvar->modified = 0;
    dvar->domainFunc = 0;
    dvar->flags = flags;
    dvar->description = description;
    HashValue = generateHashValue(dvarName);
    dvar->hashNext = dvarHashTable[HashValue];
    dvarHashTable[HashValue] = dvar;
    Sys_UnlockWrite(&g_dvarCritSect);
    return dvar;
}

DvarValue *__cdecl Dvar_GetReinterpretedResetValue(DvarValue *result, dvar_s *__formal, DvarValue value)
{
    *result = value;
    return result;
}

void __cdecl Dvar_PerformUnregistration(dvar_s *dvar)
{
    const char *v1; // eax
    const char *v2; // eax
    DvarValue resetString; // [esp+0h] [ebp-14h] BYREF

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1305, 0, "%s", "dvar");
    if ((dvar->flags & 0x4000) == 0)
    {
        dvar->flags |= 0x4000u;
        dvar->name = Dvar_AllocNameString(dvar->name);
    }
    if (dvar->type != 7)
    {
        v1 = Dvar_DisplayableLatchedValue(dvar);
        Dvar_CopyString(v1, &dvar->current);
        if (Dvar_ShouldFreeLatchedString(dvar))
            Dvar_FreeString(&dvar->latched);
        dvar->latched.integer = 0;
        Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
        if (Dvar_ShouldFreeResetString(dvar))
            Dvar_FreeString(&dvar->reset);
        dvar->reset.integer = 0;
        v2 = Dvar_DisplayableResetValue(dvar);
        Dvar_AssignResetStringValue(dvar, &resetString, v2);
        dvar->reset.integer = resetString.integer;
        dvar->type = 7;
    }
}

void __cdecl Dvar_ReinterpretDvar(
    dvar_s *dvar,
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue value,
    DvarLimits domain)
{
    DvarValue result; // [esp+0h] [ebp-38h] BYREF
    DvarValue v7; // [esp+14h] [ebp-24h]
    DvarValue resetValue; // [esp+24h] [ebp-14h]

    if ((dvar->flags & 0x4000) != 0 && (flags & 0x4000) == 0)
    {
        v7 = *Dvar_GetReinterpretedResetValue(&result, dvar, value);
        resetValue = v7;
        Dvar_PerformUnregistration(dvar);
        Dvar_FreeNameString(dvar->name);
        dvar->name = dvarName;
        dvar->flags &= ~0x4000u;
        Dvar_MakeExplicitType(dvar, dvarName, type, flags, resetValue, domain);
    }
}

void __cdecl Dvar_Reregister(
    dvar_s *dvar,
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue resetValue,
    DvarLimits domain,
    const char *description)
{
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // [esp-4h] [ebp-8h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1547, 0, "%s", "dvar");
    if (!dvarName)
        MyAssertHandler(".\\universal\\dvar.cpp", 1548, 0, "%s", "dvarName");
    if (dvar->type != type && (dvar->flags & 0x4000) == 0)
    {
        v7 = va("%s: %i != %i", dvarName, dvar->type, type);
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1549,
            0,
            "%s\n\t%s",
            "dvar->type == type || (dvar->flags & DVAR_EXTERNAL)",
            v7);
    }
    if (((flags ^ dvar->flags) & 0x4000) != 0)
        Dvar_ReinterpretDvar(dvar, dvarName, type, flags, resetValue, domain);
    if ((dvar->flags & 0x4000) != 0 && dvar->type != type)
    {
        if (dvar->type != 7)
        {
            v8 = va("dvar %s, type %i", dvar->name, dvar->type);
            MyAssertHandler(".\\universal\\dvar.cpp", 1556, 0, "%s\n\t%s", "dvar->type == DVAR_TYPE_STRING", v8);
        }
        Dvar_MakeExplicitType(dvar, dvarName, type, flags, resetValue, domain);
    }
    if (dvar->type != type)
        MyAssertHandler(".\\universal\\dvar.cpp", 1560, 0, "%s\n\t(dvarName) = %s", "(dvar->type == type)", dvarName);
    if ((dvar->flags & 0x9200) == 0 && !Dvar_ValuesEqual(type, dvar->reset, resetValue))
    {
        v11 = Dvar_ValueToString(dvar, resetValue);
        v9 = Dvar_DisplayableResetValue(dvar);
        v10 = va("dvar %s, %s != %s", dvarName, v9, v11);
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1561,
            0,
            "%s\n\t%s",
            "(dvar->flags & (DVAR_CHANGEABLE_RESET|DVAR_SAVED|DVAR_AUTOEXEC)) || Dvar_ValuesEqual( type, dvar->reset, resetValue )",
            v10);
    }
    dvar->flags |= flags;
    if (description)
        dvar->description = description;
    if ((dvar->flags & 0x80) != 0 && dvar_cheats && !dvar_cheats->current.enabled)
    {
        Dvar_SetVariant(dvar, dvar->reset, DVAR_SOURCE_INTERNAL);
        Dvar_SetLatchedValue(dvar, dvar->reset);
    }
    if ((dvar->flags & 0x20) != 0)
        Dvar_MakeLatchedValueCurrent(dvar);
}

const dvar_s *__cdecl Dvar_RegisterVariant(
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue value,
    DvarLimits domain,
    const char *description)
{
    dvar_s *dvar; // [esp+0h] [ebp-8h]

    if ((flags & 0x4000) == 0 && !CanKeepStringPointer(dvarName))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1662,
            0,
            "%s\n\t(dvarName) = %s",
            "((flags & (1 << 14)) || CanKeepStringPointer( dvarName ))",
            dvarName);
    dvar = Dvar_FindMalleableVar(dvarName);
    if (!dvar)
        return Dvar_RegisterNew(dvarName, type, flags, value, domain, description);
    Dvar_Reregister(dvar, dvarName, type, flags, value, domain, description);
    return dvar;
}

void __cdecl Dvar_MakeExplicitType(
    dvar_s *dvar,
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue resetValue,
    DvarLimits domain)
{
    bool v6; // [esp+0h] [ebp-5Ch]
    DvarValue v7; // [esp+4h] [ebp-58h] BYREF
    DvarValue v8; // [esp+14h] [ebp-48h]
    DvarValue result; // [esp+24h] [ebp-38h] BYREF
    DvarValue v10; // [esp+34h] [ebp-28h]
    bool wasString; // [esp+47h] [ebp-15h]
    DvarValue castValue; // [esp+48h] [ebp-14h]

    if (dvar->type != 7)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1445,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_STRING)",
            dvar->type);
    dvar->type = type;
    dvar->domain = domain;
    if ((flags & 0x40) != 0 || (flags & 0x80) != 0 && dvar_cheats && !dvar_cheats->current.enabled)
    {
        castValue = resetValue;
    }
    else
    {
        v10 = *Dvar_StringToValue(&result, dvar->type, dvar->domain, dvar->current.string);
        castValue = v10;
        v8 = *Dvar_ClampValueToDomain(&v7, type, v10, resetValue, domain);
        castValue = v8;
    }
    v6 = dvar->type == 7 && castValue.integer;
    wasString = v6;
    if (v6)
        castValue.integer = (int)CopyString((char *)castValue.integer);
    if (dvar->type != 7 && Dvar_ShouldFreeCurrentString(dvar))
        Dvar_FreeString(&dvar->current);
    dvar->current.integer = 0;
    if (Dvar_ShouldFreeLatchedString(dvar))
        Dvar_FreeString(&dvar->latched);
    dvar->latched.integer = 0;
    if (Dvar_ShouldFreeResetString(dvar))
        Dvar_FreeString(&dvar->reset);
    dvar->reset.integer = 0;
    Dvar_UpdateResetValue(dvar, resetValue);
    Dvar_UpdateValue(dvar, castValue);
    dvar_modifiedFlags |= flags;
    if (wasString)
        FreeString(castValue.string);
}

DvarValue *__cdecl Dvar_StringToValue(DvarValue *result, unsigned __int8 type, DvarLimits domain, const char *string)
{
    const char *v4; // eax
    DvarValue value; // [esp+4h] [ebp-14h] BYREF

    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 466, 0, "%s", "string");
    switch (type)
    {
    case 0u:
        value.enabled = Dvar_StringToBool(string);
        break;
    case 1u:
        value.value = Dvar_StringToFloat(string);
        break;
    case 2u:
        Dvar_StringToVec2(string, &value.value);
        break;
    case 3u:
        Dvar_StringToVec3(string, &value.value);
        break;
    case 4u:
        Dvar_StringToVec4(string, &value.value);
        break;
    case 5u:
        value.integer = Dvar_StringToInt(string);
        break;
    case 6u:
        value.integer = Dvar_StringToEnum(&domain, string);
        break;
    case 7u:
        value.integer = (int)string;
        break;
    case 8u:
        Dvar_StringToColor(string, (unsigned __int8 *)&value);
        break;
    default:
        if (!alwaysfails)
        {
            v4 = va("unhandled dvar type '%i'", type);
            MyAssertHandler(".\\universal\\dvar.cpp", 508, 1, v4);
        }
        value.integer = 0;
        break;
    }
    *result = value;
    return result;
}

void __cdecl Dvar_StringToVec2(const char *string, float *vector)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 386, 0, "%s", "string");
    *vector = 0.0;
    vector[1] = 0.0;
    sscanf(string, "%g %g", vector, vector + 1);
}

void __cdecl Dvar_StringToVec3(const char *string, float *vector)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 395, 0, "%s", "string");
    *vector = 0.0;
    vector[1] = 0.0;
    vector[2] = 0.0;
    if (*string == 40)
        sscanf(string, "( %g, %g, %g )", vector, vector + 1, vector + 2);
    else
        sscanf(string, "%g %g %g", vector, vector + 1, vector + 2);
}

void __cdecl Dvar_StringToVec4(const char *string, float *vector)
{
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 407, 0, "%s", "string");
    *vector = 0.0;
    vector[1] = 0.0;
    vector[2] = 0.0;
    vector[3] = 0.0;
    sscanf(string, "%g %g %g %g", vector, vector + 1, vector + 2, vector + 3);
}

int __cdecl Dvar_StringToEnum(const DvarLimits *domain, const char *string)
{
    int v3; // [esp+0h] [ebp-1Ch]
    int stringIndex; // [esp+14h] [ebp-8h]
    int stringIndexa; // [esp+14h] [ebp-8h]
    int stringIndexb; // [esp+14h] [ebp-8h]
    const char *digit; // [esp+18h] [ebp-4h]

    if (!domain)
        MyAssertHandler(".\\universal\\dvar.cpp", 420, 0, "%s", "domain");
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 421, 0, "%s", "string");
    for (stringIndex = 0; stringIndex < domain->enumeration.stringCount; ++stringIndex)
    {
        if (!I_stricmp(string, *(const char **)(domain->integer.max + 4 * stringIndex)))
            return stringIndex;
    }
    stringIndexa = 0;
    for (digit = string; *digit; ++digit)
    {
        if (*digit < 48 || *digit > 57)
            return -1337;
        stringIndexa = 10 * stringIndexa + *digit - 48;
    }
    if (stringIndexa >= 0 && stringIndexa < domain->enumeration.stringCount)
        return stringIndexa;
    v3 = strlen(string);
    for (stringIndexb = 0; stringIndexb < domain->enumeration.stringCount; ++stringIndexb)
    {
        if (!I_strnicmp(string, *(const char **)(domain->integer.max + 4 * stringIndexb), v3))
            return stringIndexb;
    }
    return -1337;
}

void __cdecl Dvar_UpdateValue(dvar_s *dvar, DvarValue value)
{
    DvarValue oldString; // [esp+1Ch] [ebp-28h] BYREF
    bool shouldFree; // [esp+2Fh] [ebp-15h]
    DvarValue currentString; // [esp+30h] [ebp-14h] BYREF

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1393, 0, "%s", "dvar");
    switch (dvar->type)
    {
    case 2u:
        dvar->current.value = value.value;
        dvar->current.vector[1] = value.vector[1];
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        break;
    case 3u:
        dvar->current.value = value.value;
        dvar->current.vector[1] = value.vector[1];
        dvar->current.vector[2] = value.vector[2];
        dvar->latched.value = value.value;
        dvar->latched.vector[1] = value.vector[1];
        dvar->latched.vector[2] = value.vector[2];
        break;
    case 4u:
        dvar->current = value;
        dvar->latched = value;
        break;
    case 7u:
        if (value.integer != dvar->current.integer)
        {
            shouldFree = Dvar_ShouldFreeCurrentString(dvar);
            if (shouldFree)
                oldString.integer = dvar->current.integer;
            Dvar_AssignCurrentStringValue(dvar, &currentString, (char *)value.integer);
            dvar->current.integer = currentString.integer;
            if (Dvar_ShouldFreeLatchedString(dvar))
                Dvar_FreeString(&dvar->latched);
            dvar->latched.integer = 0;
            Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
            if (shouldFree)
                Dvar_FreeString(&oldString);
        }
        break;
    default:
        dvar->current = value;
        dvar->latched = value;
        break;
    }
}

char *__cdecl Dvar_AllocNameString(const char *name)
{
    return (char *)CopyString(name);
}

const dvar_s *__cdecl Dvar_RegisterInt(
    const char *dvarName,
    int value,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue v6; // [esp-1Ch] [ebp-38h]
    __int64 dvarValue_4; // [esp+Ch] [ebp-10h]
    float dvarValue_12; // [esp+14h] [ebp-8h]

    v6.integer = value;
    *(_QWORD *)(&v6.value + 1) = dvarValue_4;
    v6.vector[3] = dvarValue_12;
    return Dvar_RegisterVariant(dvarName, 5u, flags, v6, min, description);
}

const dvar_t *__cdecl Dvar_RegisterInt(
    const char *dvarName,
    int value,
    unsigned int min,
    unsigned int max,
    unsigned int flags,
    const char *description)
{
    DvarValue dvarValue; // [esp+4h] [ebp-20h]
    DvarLimits dvarDomain; // [esp+14h] [ebp-10h]

    dvarValue.integer = value;
    dvarDomain.integer.min = min;
    dvarDomain.integer.max = max;

    return Dvar_RegisterVariant(dvarName, DVAR_TYPE_INT, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl Dvar_RegisterFloat(
    const char *dvarName,
    float value,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue v6; // [esp-1Ch] [ebp-38h]
    __int64 dvarValue_4; // [esp+Ch] [ebp-10h]
    float dvarValue_12; // [esp+14h] [ebp-8h]

    v6.value = value;
    *(_QWORD *)(&v6.value + 1) = dvarValue_4;
    v6.vector[3] = dvarValue_12;
    return Dvar_RegisterVariant(dvarName, 1u, flags, v6, min, description);
}

const dvar_s *__cdecl Dvar_RegisterFloat(
    const char *dvarName,
    float value,
    float min,
    float max,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue dvarValue; // [esp+4h] [ebp-20h]
    DvarLimits dvarDomain; // [esp+14h] [ebp-10h]

    dvarValue.value = value;
    dvarDomain.value.min = min;
    dvarDomain.value.max = max;

    return Dvar_RegisterVariant(dvarName, DVAR_TYPE_FLOAT, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl Dvar_RegisterVec2(
    const char *dvarName,
    float x,
    float y,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue v7; // [esp-1Ch] [ebp-38h]
    __int64 dvarValue_4; // [esp+Ch] [ebp-10h]
    float dvarValue_12; // [esp+14h] [ebp-8h]

    *(float *)&dvarValue_4 = y;
    v7.value = x;
    *(_QWORD *)(&v7.value + 1) = dvarValue_4;
    v7.vector[3] = dvarValue_12;
    return Dvar_RegisterVariant(dvarName, 2u, flags, v7, min, description);
}

const dvar_s *__cdecl Dvar_RegisterVec3(
    const char *dvarName,
    float x,
    float y,
    float z,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue v8; // [esp-1Ch] [ebp-38h]
    __int64 dvarValue_4; // [esp+Ch] [ebp-10h]
    float dvarValue_12; // [esp+14h] [ebp-8h]

    *(float *)&dvarValue_4 = y;
    *((float *)&dvarValue_4 + 1) = z;
    v8.value = x;
    *(_QWORD *)(&v8.value + 1) = dvarValue_4;
    v8.vector[3] = dvarValue_12;
    return Dvar_RegisterVariant(dvarName, 3u, flags, v8, min, description);
}

const dvar_s *__cdecl Dvar_RegisterVec4(
    const char *dvarName,
    float x,
    float y,
    float z,
    float w,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue v9; // [esp-1Ch] [ebp-38h]
    __int64 dvarValue_4; // [esp+Ch] [ebp-10h]

    *(float *)&dvarValue_4 = y;
    *((float *)&dvarValue_4 + 1) = z;
    v9.value = x;
    *(_QWORD *)(&v9.value + 1) = dvarValue_4;
    v9.vector[3] = w;
    return Dvar_RegisterVariant(dvarName, 4u, flags, v9, min, description);
}

const dvar_s *__cdecl Dvar_RegisterString(
    const char *dvarName,
    const char *value,
    unsigned __int16 flags,
    const char *description)
{
    DvarValue v5; // [esp-1Ch] [ebp-38h]
    __int64 dvarValue_4; // [esp+Ch] [ebp-10h]
    float dvarValue_12; // [esp+14h] [ebp-8h]

    if (!dvarName)
        MyAssertHandler(".\\universal\\dvar.cpp", 1751, 0, "%s", "dvarName");
    if (!value)
        MyAssertHandler(".\\universal\\dvar.cpp", 1752, 0, "%s", "value");
    if ((flags & 0x4000) == 0 && !CanKeepStringPointer(value))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1753,
            0,
            "%s\n\t(dvarName) = %s",
            "((flags & (1 << 14)) || CanKeepStringPointer( value ))",
            dvarName);
    v5.integer = (int)value;
    *(_QWORD *)(&v5.value + 1) = dvarValue_4;
    v5.vector[3] = dvarValue_12;
    return Dvar_RegisterVariant(dvarName, 7u, flags, v5, 0, description);
}

const dvar_s *__cdecl Dvar_RegisterEnum(
    const char *dvarName,
    const char **valueList,
    int defaultIndex,
    unsigned __int16 flags,
    const char *description)
{
    DvarLimits dvarDomain; // [esp+0h] [ebp-1Ch]
    DvarValue dvarValue; // [esp+8h] [ebp-14h]

    if (!dvarName)
        MyAssertHandler(".\\universal\\dvar.cpp", 1766, 0, "%s", "dvarName");
    if (!valueList)
        MyAssertHandler(".\\universal\\dvar.cpp", 1767, 0, "%s", "valueList");
    dvarValue.integer = defaultIndex;
    dvarDomain.integer.max = (int)valueList;
    for (dvarDomain.enumeration.stringCount = 0;
        valueList[dvarDomain.enumeration.stringCount];
        ++dvarDomain.enumeration.stringCount)
    {
        ;
    }
    if (defaultIndex < 0 || defaultIndex >= dvarDomain.enumeration.stringCount)
    {
        if (defaultIndex)
            MyAssertHandler(
                ".\\universal\\dvar.cpp",
                1773,
                0,
                "%s\n\t(dvarName) = %s",
                "(defaultIndex >= 0 && defaultIndex < dvarDomain.enumeration.stringCount || defaultIndex == 0)",
                dvarName);
    }
    return Dvar_RegisterVariant(dvarName, 6u, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl Dvar_RegisterColor(
    const char *dvarName,
    float r,
    float g,
    float b,
    float a,
    unsigned __int16 flags,
    const char *description)
{
    float v8; // [esp+0h] [ebp-ACh]
    float v9; // [esp+4h] [ebp-A8h]
    float v10; // [esp+8h] [ebp-A4h]
    float v11; // [esp+Ch] [ebp-A0h]
    float v12; // [esp+10h] [ebp-9Ch]
    float v13; // [esp+14h] [ebp-98h]
    float v14; // [esp+18h] [ebp-94h]
    float v15; // [esp+1Ch] [ebp-90h]
    float v16; // [esp+20h] [ebp-8Ch]
    float v17; // [esp+24h] [ebp-88h]
    float v18; // [esp+28h] [ebp-84h]
    float v19; // [esp+2Ch] [ebp-80h]
    float v20; // [esp+34h] [ebp-78h]
    float v21; // [esp+44h] [ebp-68h]
    float v22; // [esp+4Ch] [ebp-60h]
    float v23; // [esp+5Ch] [ebp-50h]
    float v24; // [esp+64h] [ebp-48h]
    float v25; // [esp+74h] [ebp-38h]
    float v26; // [esp+7Ch] [ebp-30h]
    float v27; // [esp+8Ch] [ebp-20h]
    DvarValue dvarValue; // [esp+98h] [ebp-14h]

    v19 = r - 1.0;
    if (v19 < 0.0)
        v27 = r;
    else
        v27 = 1.0;
    v18 = 0.0 - v27;
    if (v18 < 0.0)
        v17 = v27;
    else
        v17 = 0.0;
    v16 = g - 1.0;
    if (v16 < 0.0)
        v25 = g;
    else
        v25 = 1.0;
    v15 = 0.0 - v25;
    if (v15 < 0.0)
        v14 = v25;
    else
        v14 = 0.0;
    v13 = b - 1.0;
    if (v13 < 0.0)
        v23 = b;
    else
        v23 = 1.0;
    v12 = 0.0 - v23;
    if (v12 < 0.0)
        v11 = v23;
    else
        v11 = 0.0;
    v10 = a - 1.0;
    if (v10 < 0.0)
        v21 = a;
    else
        v21 = 1.0;
    v9 = 0.0 - v21;
    if (v9 < 0.0)
        v8 = v21;
    else
        v8 = 0.0;
    v20 = v8 * 255.0 + 0.001000000047497451;
    dvarValue.color[3] = (int)(v20 + 9.313225746154785e-10);
    v26 = v17 * 255.0 + 0.001000000047497451;
    dvarValue.enabled = (int)(v26 + 9.313225746154785e-10);
    v24 = v14 * 255.0 + 0.001000000047497451;
    dvarValue.color[1] = (int)(v24 + 9.313225746154785e-10);
    v22 = v11 * 255.0 + 0.001000000047497451;
    dvarValue.color[2] = (int)(v22 + 9.313225746154785e-10);
    return Dvar_RegisterVariant(dvarName, 8u, flags, dvarValue, 0, description);
}

void __cdecl Dvar_SetBoolFromSource(dvar_s *dvar, bool value, DvarSetSource source)
{
    const char *v3; // [esp+0h] [ebp-18h]
    DvarValue newValue; // [esp+4h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1798, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1799, 0, "%s", "dvar->name");
    if (dvar->type && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1800,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_bool || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type)
    {
        if (value)
            v3 = "1";
        else
            v3 = "0";
        newValue.integer = (int)v3;
    }
    else
    {
        newValue.enabled = value;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetIntFromSource(dvar_s *dvar, int value, DvarSetSource source)
{
    char string[32]; // [esp+0h] [ebp-34h] BYREF
    DvarValue newValue; // [esp+20h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1816, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1817, 0, "%s", "dvar->name");
    if (dvar->type != 5 && dvar->type != 6 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1818,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_ENUM || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type == 5 || dvar->type == 6)
    {
        newValue.integer = value;
    }
    else
    {
        Com_sprintf(string, 0x20u, "%i", value);
        newValue.integer = (int)string;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetFloatFromSource(dvar_s *dvar, float value, DvarSetSource source)
{
    char string[32]; // [esp+8h] [ebp-34h] BYREF
    DvarValue newValue; // [esp+28h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1839, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1840, 0, "%s", "dvar->name");
    if (dvar->type != 1 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1841,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_FLOAT || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type == 1)
    {
        newValue.value = value;
    }
    else
    {
        Com_sprintf(string, 0x20u, "%g", value);
        newValue.integer = (int)string;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetVec2FromSource(dvar_s *dvar, float x, float y, DvarSetSource source)
{
    char string[68]; // [esp+10h] [ebp-58h] BYREF
    DvarValue newValue; // [esp+54h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1862, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1863, 0, "%s", "dvar->name");
    if (dvar->type != 4 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1864,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_FLOAT_4 || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type == 4)
    {
        newValue.value = x;
        newValue.vector[1] = y;
    }
    else
    {
        Com_sprintf(string, 0x40u, "%g %g", x, y);
        newValue.integer = (int)string;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetVec3FromSource(dvar_s *dvar, float x, float y, float z, DvarSetSource source)
{
    char string[100]; // [esp+18h] [ebp-78h] BYREF
    DvarValue newValue; // [esp+7Ch] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1885, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1886, 0, "%s", "dvar->name");
    if (dvar->type != 3 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1887,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_FLOAT_3 || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type == 3)
    {
        newValue.value = x;
        newValue.vector[1] = y;
        newValue.vector[2] = z;
    }
    else
    {
        Com_sprintf(string, 0x60u, "%g %g %g", x, y, z);
        newValue.integer = (int)string;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetVec4FromSource(dvar_s *dvar, float x, float y, float z, float w, DvarSetSource source)
{
    char string[132]; // [esp+20h] [ebp-98h] BYREF
    DvarValue newValue; // [esp+A4h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1908, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1909, 0, "%s", "dvar->name");
    if (dvar->type != 4 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1910,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_FLOAT_4 || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type == 4)
    {
        newValue.value = x;
        newValue.vector[1] = y;
        newValue.vector[2] = z;
        newValue.vector[3] = w;
    }
    else
    {
        Com_sprintf(string, 0x80u, "%g %g %g %g", x, y, z, w);
        newValue.integer = (int)string;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetColorFromSource(dvar_s *dvar, float r, float g, float b, float a, DvarSetSource source)
{
    float v6; // [esp+20h] [ebp-128h]
    float v7; // [esp+24h] [ebp-124h]
    float v8; // [esp+28h] [ebp-120h]
    float v9; // [esp+2Ch] [ebp-11Ch]
    float v10; // [esp+30h] [ebp-118h]
    float v11; // [esp+34h] [ebp-114h]
    float v12; // [esp+38h] [ebp-110h]
    float v13; // [esp+3Ch] [ebp-10Ch]
    float v14; // [esp+40h] [ebp-108h]
    float v15; // [esp+44h] [ebp-104h]
    float v16; // [esp+48h] [ebp-100h]
    float v17; // [esp+4Ch] [ebp-FCh]
    float v18; // [esp+54h] [ebp-F4h]
    float v19; // [esp+64h] [ebp-E4h]
    float v20; // [esp+6Ch] [ebp-DCh]
    float v21; // [esp+7Ch] [ebp-CCh]
    float v22; // [esp+84h] [ebp-C4h]
    float v23; // [esp+94h] [ebp-B4h]
    float v24; // [esp+9Ch] [ebp-ACh]
    float v25; // [esp+ACh] [ebp-9Ch]
    char string[132]; // [esp+B0h] [ebp-98h] BYREF
    DvarValue newValue; // [esp+134h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1956, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1957, 0, "%s", "dvar->name");
    if (dvar->type != 8 && (dvar->type != 7 || (dvar->flags & 0x4000) == 0))
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1958,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_COLOR || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name);
    if (dvar->type == 8)
    {
        v17 = r - 1.0;
        if (v17 < 0.0)
            v25 = r;
        else
            v25 = 1.0;
        v16 = 0.0 - v25;
        if (v16 < 0.0)
            v15 = v25;
        else
            v15 = 0.0;
        v24 = v15 * 255.0;
        newValue.enabled = (int)(v24 + 9.313225746154785e-10);
        v14 = g - 1.0;
        if (v14 < 0.0)
            v23 = g;
        else
            v23 = 1.0;
        v13 = 0.0 - v23;
        if (v13 < 0.0)
            v12 = v23;
        else
            v12 = 0.0;
        v22 = v12 * 255.0;
        newValue.color[1] = (int)(v22 + 9.313225746154785e-10);
        v11 = b - 1.0;
        if (v11 < 0.0)
            v21 = b;
        else
            v21 = 1.0;
        v10 = 0.0 - v21;
        if (v10 < 0.0)
            v9 = v21;
        else
            v9 = 0.0;
        v20 = v9 * 255.0;
        newValue.color[2] = (int)(v20 + 9.313225746154785e-10);
        v8 = a - 1.0;
        if (v8 < 0.0)
            v19 = a;
        else
            v19 = 1.0;
        v7 = 0.0 - v19;
        if (v7 < 0.0)
            v6 = v19;
        else
            v6 = 0.0;
        v18 = v6 * 255.0;
        newValue.color[3] = (int)(v18 + 9.313225746154785e-10);
    }
    else
    {
        Com_sprintf(string, 0x80u, "%g %g %g %g", r, g, b, a);
        newValue.integer = (int)string;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetBool(dvar_s *dvar, bool value)
{
    Dvar_SetBoolFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetInt(dvar_s *dvar, int value)
{
    Dvar_SetIntFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetFloat(dvar_s *dvar, float value)
{
    Dvar_SetFloatFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetVec3(dvar_s *dvar, float x, float y, float z)
{
    Dvar_SetVec3FromSource(dvar, x, y, z, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetString(dvar_s *dvar, char *value)
{
    Dvar_SetStringFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetStringFromSource(dvar_s *dvar, char *string, DvarSetSource source)
{
    const char *v3; // eax
    char stringCopy[1028]; // [esp+0h] [ebp-418h] BYREF
    DvarValue newValue; // [esp+404h] [ebp-14h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 1931, 0, "%s", "dvar");
    if (!dvar->name)
        MyAssertHandler(".\\universal\\dvar.cpp", 1932, 0, "%s", "dvar->name");
    if (dvar->type != 7 && dvar->type != 6)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            1933,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_STRING || dvar->type == DVAR_TYPE_ENUM)",
            dvar->name);
    if (!string)
        MyAssertHandler(".\\universal\\dvar.cpp", 1934, 0, "%s", "string");
    if (dvar->type == 7)
    {
        I_strncpyz(stringCopy, string, 1024);
        newValue.integer = (int)stringCopy;
    }
    else
    {
        newValue.integer = Dvar_StringToEnum(&dvar->domain, string);
        if (newValue.integer == -1337)
        {
            v3 = va("%s doesn't include %s", dvar->name, string);
            MyAssertHandler(".\\universal\\dvar.cpp", 1944, 0, "%s\n\t%s", "newValue.integer != DVAR_INVALID_ENUM_INDEX", v3);
        }
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetColor(dvar_s *dvar, float r, float g, float b, float a)
{
    Dvar_SetColorFromSource(dvar, r, g, b, a, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetFromString(dvar_s *dvar, char *string)
{
    Dvar_SetFromStringFromSource(dvar, string, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetFromStringFromSource(dvar_s *dvar, char *string, DvarSetSource source)
{
    DvarValue result; // [esp+0h] [ebp-438h] BYREF
    DvarValue v4; // [esp+10h] [ebp-428h]
    char buf[1028]; // [esp+20h] [ebp-418h] BYREF
    DvarValue newValue; // [esp+424h] [ebp-14h]

    I_strncpyz(buf, string, 1024);
    v4 = *Dvar_StringToValue(&result, dvar->type, dvar->domain, buf);
    newValue = v4;
    if (dvar->type == 6 && newValue.integer == -1337)
    {
        Com_Printf(16, "'%s' is not a valid value for dvar '%s'\n", buf, dvar->name);
        Dvar_PrintDomain(dvar->type, dvar->domain);
        newValue = dvar->reset;
    }
    Dvar_SetVariant(dvar, newValue, source);
}

void __cdecl Dvar_SetBoolByName(const char *dvarName, bool value)
{
    dvar_s *dvar; // [esp+4h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if (dvar)
    {
        Dvar_SetBool(dvar, value);
    }
    else if (value)
    {
        Dvar_RegisterString(dvarName, "1", 0x4000u, "External Dvar");
    }
    else
    {
        Dvar_RegisterString(dvarName, "0", 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetIntByName(const char *dvarName, int value)
{
    char string[32]; // [esp+0h] [ebp-28h] BYREF
    const dvar_s *dvar; // [esp+24h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if (dvar)
    {
        Dvar_SetInt((dvar_s *)dvar, value);
    }
    else
    {
        Com_sprintf(string, 0x20u, "%i", value);
        Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetFloatByName(const char *dvarName, float value)
{
    char *v2; // eax
    dvar_s *dvar; // [esp+10h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if (dvar)
    {
        Dvar_SetFloat(dvar, value);
    }
    else
    {
        v2 = va("%g", value);
        Dvar_RegisterString(dvarName, v2, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetVec3ByName(const char *dvarName, float x, float y, float z)
{
    char *v4; // eax
    dvar_s *dvar; // [esp+20h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if (dvar)
    {
        Dvar_SetVec3(dvar, x, y, z);
    }
    else
    {
        v4 = va("%g %g %g", x, y, z);
        Dvar_RegisterString(dvarName, v4, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetStringByName(const char *dvarName, char *value)
{
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if (dvar)
        Dvar_SetString(dvar, value);
    else
        Dvar_RegisterString(dvarName, value, 0x4000u, "External Dvar");
}

const dvar_s *__cdecl Dvar_SetFromStringByNameFromSource(const char *dvarName, const char *string, DvarSetSource source)
{
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if (!dvar)
        return Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    Dvar_SetFromStringFromSource(dvar, (char*)string, source);
    return dvar;
}

void __cdecl Dvar_SetFromStringByName(const char *dvarName, char *string)
{
    Dvar_SetFromStringByNameFromSource(dvarName, string, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetCommand(const char *dvarName, char *string)
{
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = (dvar_s *)Dvar_SetFromStringByNameFromSource(dvarName, string, DVAR_SOURCE_EXTERNAL);
    if (dvar)
    {
        if (isLoadingAutoExecGlobalFlag)
        {
            Dvar_AddFlags(dvar, 0x8000);
            Dvar_UpdateResetValue(dvar, dvar->current);
        }
    }
}

void __cdecl Dvar_SetDomainFunc(dvar_s *dvar, bool(__cdecl *customFunc)(dvar_s *, DvarValue))
{
    const char *v2; // eax
    const char *name; // [esp-4h] [ebp-4h]

    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 2198, 0, "%s", "dvar");
    dvar->domainFunc = customFunc;
    if (customFunc)
    {
        if (!((unsigned __int8(__cdecl *)(dvar_s *, int, unsigned int, unsigned int, unsigned int))dvar->domainFunc)(
            dvar,
            dvar->current.integer,
            LODWORD(dvar->current.vector[1]),
            LODWORD(dvar->current.vector[2]),
            LODWORD(dvar->current.vector[3])))
        {
            name = dvar->name;
            v2 = Dvar_ValueToString(dvar, dvar->current);
            Com_Printf(16, "'%s' is not a valid value for dvar '%s'\n\n", v2, name);
            Dvar_Reset(dvar, DVAR_SOURCE_INTERNAL);
        }
    }
}

void __cdecl Dvar_AddFlags(dvar_s *dvar, int flags)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 2212, 0, "%s", "dvar");
    if ((flags & 0x40F0) != 0)
        MyAssertHandler(
            ".\\universal\\dvar.cpp",
            2213,
            0,
            "%s\n\t(flags) = %i",
            "((flags & ((1 << 7) | (1 << 4) | (1 << 6) | (1 << 14) | (1 << 5))) == 0)",
            flags);
    dvar->flags |= flags;
}

void __cdecl Dvar_Reset(dvar_s *dvar, DvarSetSource setSource)
{
    if (!dvar)
        MyAssertHandler(".\\universal\\dvar.cpp", 2220, 0, "%s", "dvar");
    Dvar_SetVariant(dvar, dvar->reset, setSource);
}

void __cdecl Dvar_SetCheatState()
{
    int dvarIter; // [esp+4h] [ebp-8h]
    dvar_s *dvar; // [esp+8h] [ebp-4h]

    InterlockedIncrement(&g_dvarCritSect.readCount);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
    {
        dvar = &dvarPool[dvarIter];
        if ((dvar->flags & 0x80) != 0)
            Dvar_SetVariant(dvar, dvar->reset, DVAR_SOURCE_INTERNAL);
    }
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
}

void __cdecl Dvar_Init()
{
    isDvarSystemActive = 1;
    dvar_cheats = Dvar_RegisterBool("sv_cheats", 1, 0x18u, "External Dvar");
    Dvar_AddCommands();
}

void __cdecl Dvar_ResetScriptInfo()
{
    int dvarIter; // [esp+4h] [ebp-4h]

    InterlockedIncrement(&g_dvarCritSect.readCount);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
        dvarPool[dvarIter].flags &= ~0x400u;
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
}

char __cdecl Dvar_AnyLatchedValues()
{
    int dvarIter; // [esp+8h] [ebp-4h]

    InterlockedIncrement(&g_dvarCritSect.readCount);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
    {
        if (Dvar_HasLatchedValue(&dvarPool[dvarIter]))
        {
            if (g_dvarCritSect.readCount <= 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
                    76,
                    0,
                    "%s",
                    "critSect->readCount > 0");
            InterlockedDecrement(&g_dvarCritSect.readCount);
            return 1;
        }
    }
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
    return 0;
}

void __cdecl Dvar_ResetDvars(unsigned __int16 filter, DvarSetSource setSource)
{
    int dvarIter; // [esp+4h] [ebp-8h]

    InterlockedIncrement(&g_dvarCritSect.readCount);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    for (dvarIter = 0; dvarIter < dvarCount; ++dvarIter)
    {
        if ((filter & dvarPool[dvarIter].flags) != 0)
            Dvar_Reset(&dvarPool[dvarIter], setSource);
    }
    if (g_dvarCritSect.readCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../qcommon/threads_interlock.h",
            76,
            0,
            "%s",
            "critSect->readCount > 0");
    InterlockedDecrement(&g_dvarCritSect.readCount);
}

int __cdecl Com_SaveDvarsToBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, unsigned int bufsize)
{
    const char *string; // [esp+0h] [ebp-10h]
    int written; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    const dvar_s *dvar; // [esp+Ch] [ebp-4h]

    for (i = 0; i < numDvars; ++i)
    {
        dvar = Dvar_FindVar(dvarnames[i]);
        if (!dvar)
            MyAssertHandler(".\\universal\\dvar.cpp", 2454, 0, "%s", "dvar");
        string = Dvar_DisplayableValue(dvar);
        written = _snprintf(buffer, bufsize, "%s \"%s\"\n", dvar->name, string);
        if (written < 0)
            return 0;
        buffer += written;
        bufsize -= written;
    }
    return 1;
}

int __cdecl Com_LoadDvarsFromBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, char *filename)
{
    const char *v4; // eax
    unsigned __int8 dst[16388]; // [esp+0h] [ebp-4018h] BYREF
    unsigned int i; // [esp+4008h] [ebp-10h]
    char *s0; // [esp+400Ch] [ebp-Ch]
    dvar_s *dvar; // [esp+4010h] [ebp-8h]
    int v10; // [esp+4014h] [ebp-4h]

    if (numDvars >= 0x4000)
        MyAssertHandler(".\\universal\\dvar.cpp", 2486, 0, "%s", "numDvars < ARRAY_COUNT( wasRead )");
    memset(dst, 0, numDvars);
    v10 = 0;
    for (i = 0; i < numDvars; ++i)
    {
        dvar = (dvar_s *)Dvar_FindVar(dvarnames[i]);
        if (!dvar)
        {
            v4 = va("Unable to find dvar '%s'", dvarnames[i]);
            MyAssertHandler(".\\universal\\dvar.cpp", 2493, 0, "%s\n\t%s", "dvar", v4);
        }
        Dvar_Reset(dvar, DVAR_SOURCE_INTERNAL);
    }
    Com_BeginParseSession(filename);
    while (1)
    {
        s0 = (char *)Com_Parse((const char **)&buffer);
        if (!*s0)
            break;
        for (i = 0; ; ++i)
        {
            if (i >= numDvars)
            {
                Com_PrintWarning(16, "WARNING: unknown dvar '%s' in file '%s'\n", s0, filename);
                goto next_dvar;
            }
            if (!I_stricmp(s0, dvarnames[i]))
                break;
        }
        dvar = (dvar_s *)Dvar_FindVar(dvarnames[i]);
        if (!dvar)
            MyAssertHandler(".\\universal\\dvar.cpp", 2509, 0, "%s", "dvar");
        s0 = (char *)Com_ParseOnLine((const char **)&buffer);
        Dvar_SetFromString(dvar, s0);
        if (!dst[i])
        {
            dst[i] = 1;
            ++v10;
        }
    next_dvar:
        Com_SkipRestOfLine((const char **)&buffer);
    }
    Com_EndParseSession();
    if (v10 == numDvars)
        return 1;
    Com_PrintError(16, "ERROR: the following dvars were not specified in file '%s'\n", filename);
    for (i = 0; i < numDvars; ++i)
    {
        if (!dst[i])
            Com_PrintError(16, "  %s\n", dvarnames[i]);
    }
    return 0;
}