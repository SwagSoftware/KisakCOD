#pragma once

#include <universal/q_shared.h>

int Com_BuildPlayerProfilePath(char *path, int pathSize, const char *format, ...);
void __cdecl Com_InitPlayerProfiles(int localClientNum);
bool __cdecl Com_HasPlayerProfile();
char __cdecl Com_DeletePlayerProfile(const char *profileName);
char __cdecl Com_NewPlayerProfile(const char *profileName);
bool __cdecl Com_IsValidPlayerProfileDir(const char *profileName);
void __cdecl Com_ChangePlayerProfile(int localClientNum, char *profileName);

extern const dvar_t *ui_playerProfileAlreadyChosen;
extern const dvar_t *com_playerProfile;