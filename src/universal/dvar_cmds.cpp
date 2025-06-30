#include <qcommon/qcommon.h>
#include <qcommon/mem_track.h>
#include <qcommon/cmd.h>
#include "com_files.h"

#include <server_mp/server.h>
#include <client_mp/client_mp.h>

#include <stringed/stringed_hooks.h>

struct DvarDumpInfo // sizeof=0xC
{                                       // ...
    int count;                          // ...
    int channel;                        // ...
    const char *match;                  // ...
};
struct DvarSetConfigInfo // sizeof=0xC
{                                       // ...
    int start;                          // ...
    int max;                            // ...
    int bit;                            // ...
};

int dvarCount;

char info1[1024];
char info2[8192];

void __cdecl TRACK_dvar_cmds()
{
    track_static_alloc_internal(info1, sizeof(info1), "info1", 10);
    track_static_alloc_internal(info2, sizeof(info2), "info2", 10);
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

void __cdecl Com_DvarDump(int channel, const char *match)
{
    DvarDumpInfo dumpInfo; // [esp+0h] [ebp-94h] BYREF
    char summary[132]; // [esp+Ch] [ebp-88h] BYREF

    if (channel != 6 || com_logfile && com_logfile->current.integer)
    {
        Com_PrintMessage(channel, "=============================== DVAR DUMP ========================================\n", 0);
        dumpInfo.count = 0;
        dumpInfo.channel = channel;
        dumpInfo.match = match;
        Dvar_ForEach(Com_DvarDumpSingle, &dumpInfo);
        Com_sprintf(summary, 0x80u, "\n%i total dvars\n%i dvar indexes\n", dumpInfo.count, dvarCount);
        Com_PrintMessage(channel, summary, 0);
        Com_PrintMessage(
            channel,
            "=============================== END DVAR DUMP =====================================\n",
            0);
    }
}

void __cdecl Com_DvarDumpSingle(const dvar_s *dvar, void *userData)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // [esp-4h] [ebp-810h]
    char message[2052]; // [esp+4h] [ebp-808h] BYREF

    ++*(unsigned int *)userData;
    if (!*((unsigned int *)userData + 2) || Com_Filter(*((const char **)userData + 2), (char *)dvar->name, 0))
    {
        if (Dvar_HasLatchedValue(dvar))
        {
            v4 = Dvar_DisplayableLatchedValue(dvar);
            v2 = Dvar_DisplayableValue(dvar);
            Com_sprintf(message, 0x800u, "      %s \"%s\" -- latched \"%s\"\n", dvar->name, v2, v4);
        }
        else
        {
            v3 = Dvar_DisplayableValue(dvar);
            Com_sprintf(message, 0x800u, "      %s \"%s\"\n", dvar->name, v3);
        }
        Com_PrintMessage(*((unsigned int *)userData + 1), message, 0);
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

void __cdecl SV_SetConfig(int start, int max, int bit)
{
    DvarSetConfigInfo info; // [esp+0h] [ebp-Ch] BYREF

    info.start = start;
    info.max = max;
    info.bit = bit;
    Dvar_ForEach((void(__cdecl *)(const dvar_s *, void *))SV_SetConfigDvar, &info);
}

void __cdecl SV_SetConfigDvar(const dvar_s *dvar, int *userData)
{
    char *v2; // eax

    if ((userData[2] & dvar->flags) != 0)
    {
        v2 = (char *)Dvar_DisplayableValue(dvar);
        SV_SetConfigValueForKey(*userData, userData[1], (char *)dvar->name, v2);
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
            Dvar_RegisterBool(dvarName, value, DVAR_EXTERNAL, "External Dvar");
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
                DvarLimits dLimits;
                dLimits.integer.max = max;
                dLimits.integer.min = min;
                Dvar_RegisterInt(dvarName, value, dLimits, DVAR_EXTERNAL, "External Dvar");
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
                Dvar_RegisterFloat(dvarName, value, v4, DVAR_EXTERNAL, "External Dvar");
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

