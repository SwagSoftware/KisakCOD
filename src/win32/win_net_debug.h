#pragma once

extern int g_debugClient;

int __cdecl Sys_IsRemoteDebugClient();

void __cdecl NET_ShutdownDebug();
void NET_InitDebug();
void NET_RestartDebug();

void        NET_RestartDebug();

void __cdecl Sys_Listen_f();

void Sys_DebugSocketError(const char *message);

int __cdecl Sys_ReadDebugSocketInt();
void __cdecl Sys_WriteDebugSocketInt(int value);
int __cdecl Sys_ReadDebugSocketMessageType(unsigned __int8 *type, int blocking);
int __cdecl Sys_UpdateDebugSocket();
int __cdecl Sys_ReadDebugSocketData(char *buffer, int len, int blocking);
void __cdecl Sys_ReadDebugSocketStringBuffer(char *buffer, int len);
void __cdecl Sys_FlushDebugSocketData();
void __cdecl Sys_AckDebugSocket();
char *__cdecl Sys_ReadDebugSocketString();

void __cdecl Sys_WriteDebugSocketData(unsigned __int8 *buffer, int len);
void __cdecl Sys_WriteDebugSocketMessageType(unsigned __int8 type);
void __cdecl Sys_EndWriteDebugSocket();

extern unsigned __int8 g_debugPacket[1][8192];
