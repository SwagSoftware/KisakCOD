#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall GScr_LoadScriptAndLabel(const char *filename, const char *label, ScriptFunctions *functions);
void __fastcall GScr_LoadSingleAnimScript(const char *name, ScriptFunctions *functions);
void __fastcall GScr_LoadAnimScripts(ScriptFunctions *functions);
void __fastcall GScr_LoadDogAnimScripts(ScriptFunctions *functions);
void __fastcall GScr_LoadLevelScript(const char *mapname, ScriptFunctions *functions);
void __fastcall GScr_LoadScriptsForPathNode(pathnode_t *loadNode, ScriptFunctions **data);
void __fastcall GScr_LoadScriptsForPathNodes(ScriptFunctions *functions);
void __fastcall GScr_LoadScriptsForEntities(ScriptFunctions *functions);
void __fastcall GScr_LoadEntities();
void __fastcall GScr_LoadScripts(const char *mapname, ScriptFunctions *functions);
