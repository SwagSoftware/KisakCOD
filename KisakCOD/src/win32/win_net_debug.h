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