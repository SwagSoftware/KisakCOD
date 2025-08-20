#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_scr_load_obj.h"
#include "g_scr_main.h"
#include "g_local.h"
#include "g_main.h"
#include <script/scr_main.h>
#include <xanim/xanim.h>

int __cdecl GScr_LoadScriptAndLabel(const char *filename, const char *label, ScriptFunctions *functions)
{
    int FunctionHandle; // r3

    if ((unsigned __int8)G_ExitAfterConnectPaths())
        return 1;
    if (functions->count >= functions->maxSize)
    {
        Com_PrintError(15, "CODE ERROR: GScr_LoadScriptAndLabel: functions->maxSize exceeded\n");
        return 0;
    }
    if (!Scr_LoadScript(filename))
    {
        functions->address[functions->count++] = 0;
        Com_Printf(15, "Could not find script '%s'\n", filename);
        return 0;
    }
    FunctionHandle = Scr_GetFunctionHandle(filename, label);
    functions->address[functions->count++] = FunctionHandle;
    if (FunctionHandle)
        return 1;
    Com_Printf(15, "Could not find label '%s' in script '%s'\n", label, filename);
    return 0;
}

void __cdecl GScr_LoadSingleAnimScript(const char *name, ScriptFunctions *functions)
{
    char v4[72]; // [sp+50h] [-60h] BYREF

    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_load_obj.cpp", 60, 0, "%s", "name");
    Com_sprintf(v4, 64, "animscripts/%s", name);
    GScr_LoadScriptAndLabel(v4, "main", functions);
}

void __cdecl GScr_LoadAnimScripts(ScriptFunctions *functions)
{
    GScr_LoadSingleAnimScript("combat", functions);
    GScr_LoadSingleAnimScript("concealment_crouch", functions);
    GScr_LoadSingleAnimScript("concealment_prone", functions);
    GScr_LoadSingleAnimScript("concealment_stand", functions);
    GScr_LoadSingleAnimScript("cover_arrival", functions);
    GScr_LoadSingleAnimScript("cover_crouch", functions);
    GScr_LoadSingleAnimScript("cover_left", functions);
    GScr_LoadSingleAnimScript("cover_prone", functions);
    GScr_LoadSingleAnimScript("cover_right", functions);
    GScr_LoadSingleAnimScript("cover_stand", functions);
    GScr_LoadSingleAnimScript("cover_wide_left", functions);
    GScr_LoadSingleAnimScript("cover_wide_right", functions);
    GScr_LoadSingleAnimScript("death", functions);
    GScr_LoadSingleAnimScript("grenade_return_throw", functions);
    GScr_LoadSingleAnimScript("init", functions);
    GScr_LoadSingleAnimScript("pain", functions);
    GScr_LoadSingleAnimScript("move", functions);
    GScr_LoadSingleAnimScript("scripted", functions);
    GScr_LoadSingleAnimScript("stop", functions);
    GScr_LoadSingleAnimScript("grenade_cower", functions);
    GScr_LoadSingleAnimScript("flashed", functions);
    GScr_LoadScriptAndLabel("animscripts/scripted", "init", functions);
}

void __cdecl GScr_LoadDogAnimScripts(ScriptFunctions *functions)
{
    GScr_LoadSingleAnimScript("dog_combat", functions);
    GScr_LoadSingleAnimScript("dog_death", functions);
    GScr_LoadSingleAnimScript("dog_init", functions);
    GScr_LoadSingleAnimScript("dog_pain", functions);
    GScr_LoadSingleAnimScript("dog_move", functions);
    GScr_LoadSingleAnimScript("dog_scripted", functions);
    GScr_LoadSingleAnimScript("dog_stop", functions);
    GScr_LoadSingleAnimScript("dog_flashed", functions);
}

void __cdecl GScr_LoadLevelScript(const char *mapname, ScriptFunctions *functions)
{
    char v3[64]; // [sp+50h] [-50h] BYREF

    Com_sprintf(v3, 64, "maps/%s", mapname);
    GScr_LoadScriptAndLabel(v3, "main", functions);
}

void __cdecl GScr_LoadScriptsForPathNode(pathnode_t *loadNode, void *data)
{
    ScriptFunctions *functions; // r28
    unsigned int count; // r27
    const char *animscript; // r31
    char v6[112]; // [sp+50h] [-70h] BYREF

    functions = (ScriptFunctions *)data;
    count = functions->count;

    iassert(functions);

    if (loadNode->constant.type)
    {
        if (loadNode->constant.type == NODE_NEGOTIATION_BEGIN)
        {
            if (loadNode->constant.animscript)
            {
                animscript = SL_ConvertToString(loadNode->constant.animscript);

                iassert(animscript);

                if (Scr_AddStringSet(count, animscript))
                {
                    Com_sprintf(v6, 64, "animscripts/traverse/%s", animscript);
                    GScr_LoadScriptAndLabel(v6, "main", functions);
                }
            }
            else
            {
                Com_PrintError(
                    1,
                    "ERROR: Pathnode (Begin) at (%g %g %g) has no animscript specified\n",
                    loadNode->constant.vOrigin[0],
                    loadNode->constant.vOrigin[1],
                    loadNode->constant.vOrigin[2]
                );
                loadNode->constant.type = NODE_BADNODE;
            }
        }
        else if (loadNode->constant.animscript)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_scr_load_obj.cpp",
                137,
                0,
                "%s",
                "!loadNode->constant.animscript");
        }
    }
}

void __cdecl GScr_LoadScriptsForPathNodes(ScriptFunctions *functions)
{
    ScriptFunctions *v1; // [sp+50h] [-20h] BYREF
    unsigned int inited; // [sp+54h] [-1Ch]

    v1 = functions;
    inited = Scr_InitStringSet();
    Path_CallFunctionForNodes((void(__cdecl *)(pathnode_t *, void *))GScr_LoadScriptsForPathNode, &v1);
    Scr_ShutdownStringSet(inited);
}

void __cdecl GScr_LoadScriptsForEntities(ScriptFunctions *functions)
{
    int v2; // r17
    unsigned int inited; // r16
    unsigned int WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r3
    const char *szScript; // r3
    const char *v7; // [sp+50h] [-AF0h] BYREF
    const char *v8; // [sp+54h] [-AECh] BYREF
    char v9[64]; // [sp+60h] [-AE0h] BYREF
    SpawnVar v10; // [sp+A0h] [-AA0h] BYREF

    v2 = 0;
    if (!G_ParseSpawnVars(&v10))
        Com_Error(ERR_DROP, "GScr_LoadScriptsForEntities: no entities");
    inited = Scr_InitStringSet();
    while (G_ParseSpawnVars(&v10))
    {
        G_SpawnString(&v10, "classname", "", &v7);
        if (I_strnicmp(v7, "actor_", 6))
        {
            if (!I_stricmp(v7, "misc_mg42") || !I_stricmp(v7, "misc_turret"))
            {
                G_SpawnString(&v10, "weaponinfo", "", &v8);
                WeaponIndexForName = G_GetWeaponIndexForName(v8);
                if (WeaponIndexForName)
                {
                    WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
                    if (WeaponDef->weapClass == WEAPCLASS_TURRET)
                    {
                        szScript = WeaponDef->szScript;
                        if (*szScript)
                            GScr_LoadSingleAnimScript(szScript, functions);
                    }
                    else
                    {
                        Com_PrintError(23, "ERROR: weapClass in weapon info '%s' for %s must be 'turret'\n", v8, v7);
                    }
                }
                else
                {
                    Com_PrintError(23, "ERROR: could not find weapon info '%s' for %s\n", v8, v7);
                }
            }
        }
        else
        {
            v7 += 6;
            if (Scr_AddStringSet(inited, v7))
            {
                if (!v2 && I_stristr(v7, "dog"))
                {
                    v2 = 1;
                    GScr_LoadDogAnimScripts(functions);
                }
                Com_sprintf(v9, 64, "aitype/%s", v7);
                GScr_LoadScriptAndLabel(v9, "main", functions);
                GScr_LoadScriptAndLabel(v9, "precache", functions);
                GScr_LoadScriptAndLabel(v9, "spawner", functions);
            }
        }
    }
    Scr_ShutdownStringSet(inited);
    G_ResetEntityParsePoint();
}

void __cdecl GScr_LoadEntities()
{
    const char *v0; // [sp+50h] [-A50h] BYREF
    const char *v1; // [sp+54h] [-A4Ch] BYREF
    SpawnVar v2; // [sp+60h] [-A40h] BYREF

    if (!G_ParseSpawnVars(&v2))
        Com_Error(ERR_DROP, "GScr_LoadEntities: no entities");
    while (G_ParseSpawnVars(&v2))
    {
        G_SpawnString(&v2, "classname", "", &v0);
        if (!I_stricmp(v0, "misc_mg42") || !I_stricmp(v0, "misc_turret"))
        {
            G_SpawnString(&v2, "weaponinfo", "", &v1);
            G_GetWeaponIndexForName(v1);
        }
    }
    G_ResetEntityParsePoint();
}

void __cdecl GScr_LoadScripts(const char *mapname, ScriptFunctions *functions)
{
    unsigned int inited; // [sp+54h] [-7Ch]
    char filename[112]; // [sp+60h] [-70h] BYREF

    Scr_BeginLoadScripts();
    ProfLoad_Begin("load misc scripts");
    GScr_LoadScriptAndLabel("codescripts/delete", "main", functions);
    GScr_LoadScriptAndLabel("codescripts/struct", "initstructs", functions);
    GScr_LoadScriptAndLabel("codescripts/struct", "createstruct", functions);
    ProfLoad_End();
    ProfLoad_Begin("load anim scripts");
    GScr_LoadAnimScripts(functions);
    ProfLoad_End();
    ProfLoad_Begin("load level script");
    Com_sprintf(filename, 64, "maps/%s", mapname);
    GScr_LoadScriptAndLabel(filename, "main", functions);
    ProfLoad_End();
    ProfLoad_Begin("load entity scripts");
    GScr_LoadScriptsForEntities(functions);
    ProfLoad_End();
    inited = Scr_InitStringSet();
    Path_CallFunctionForNodes(GScr_LoadScriptsForPathNode, functions);
    Scr_ShutdownStringSet(inited);
    Scr_PostCompileScripts();
}
