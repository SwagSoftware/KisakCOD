#pragma once

extern int g_debugClient;

void __cdecl NET_ShutdownDebug();
void NET_InitDebug();
void NET_RestartDebug();

void        NET_RestartDebug();

void __cdecl Sys_Listen_f();