#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

// sv_init

void __fastcall TRACK_sv_init();
void __fastcall SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize);
unsigned int __fastcall SV_GetConfigstringConst(unsigned int index);
void __fastcall SV_InitReliableCommandsForClient(client_t *cl);
void __fastcall SV_FreeReliableCommandsForClient(client_t *cl);
void __fastcall SV_AddReliableCommand(client_t *cl, int index, const char *cmd);
void __fastcall SV_Startup();
void __fastcall SV_ClearServer();
void __fastcall SV_StartMap(int randomSeed);
void __fastcall SV_Settle();
int __fastcall SV_SaveImmediately(const char *levelName);
void __fastcall SV_LoadLevelAssets(const char *mapname);
bool __fastcall SV_Loaded();
void __fastcall SV_Init();
void __fastcall SV_Shutdown(const char *finalmsg);
void __fastcall SV_SetConfigstring(unsigned int index, const char *val);
void SV_SaveSystemInfo();
void __fastcall SV_SpawnServer(const char *server, int savegame);
