#pragma once

//sv_init_mp
void __cdecl SV_SetConfigstring(int index, char *val);
void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize);
unsigned int __cdecl SV_GetConfigstringConst(unsigned int index);
void __cdecl SV_SetConfigValueForKey(int start, int max, char *key, char *value);
void __cdecl SV_SetUserinfo(int index, char *val);
void __cdecl SV_GetUserinfo(int index, char *buffer, int bufferSize);
void __cdecl SV_CreateBaseline();
void __cdecl SV_BoundMaxClients(int minimum);
void __cdecl SV_Startup();
void __cdecl SV_ClearServer();
void __cdecl SV_InitArchivedSnapshot();
void __cdecl SV_InitDvar();
void __cdecl SV_SpawnServer(char *server);
void SV_SaveSystemInfo();
bool __cdecl SV_Loaded();
void __cdecl SV_Init();
void __cdecl SV_DropAllClients();
void __cdecl SV_Shutdown(const char *finalmsg);
void __cdecl SV_FinalMessage(const char *message);
void __cdecl SV_CheckThread();
