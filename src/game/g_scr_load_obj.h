#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct ScriptFunctions;
struct pathnode_t;

int __cdecl GScr_LoadScriptAndLabel(const char *filename, const char *label, ScriptFunctions *functions);
void __cdecl GScr_LoadSingleAnimScript(const char *name, ScriptFunctions *functions);
void __cdecl GScr_LoadAnimScripts(ScriptFunctions *functions);
void __cdecl GScr_LoadDogAnimScripts(ScriptFunctions *functions);
void __cdecl GScr_LoadLevelScript(const char *mapname, ScriptFunctions *functions);
void __cdecl GScr_LoadScriptsForPathNode(pathnode_t *loadNode, ScriptFunctions **data);
void __cdecl GScr_LoadScriptsForPathNodes(ScriptFunctions *functions);
void __cdecl GScr_LoadScriptsForEntities(ScriptFunctions *functions);
void __cdecl GScr_LoadEntities();
void __cdecl GScr_LoadScripts(const char *mapname, ScriptFunctions *functions);
