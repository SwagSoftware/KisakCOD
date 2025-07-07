#pragma once
#include <client/client.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

struct rectDef_s;
struct windowDef_t;
struct dvar_s;
struct playerState_s;
struct snd_alias_list_t;
struct PhysPreset;

const rectDef_s *__cdecl Window_GetRect(const windowDef_t *w);
connstate_t __cdecl CL_GetLocalClientConnectionState(int localClientNum);
const dvar_s *CG_RegisterDvars();
void __cdecl TRACK_cg_main();
void __cdecl CG_GetDObjOrientation(int localClientNum, int dobjHandle, float (*axis)[3], float *origin);
void __cdecl CG_CopyEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3]);
void __cdecl CG_GetSoundEntityOrientation(const SndEntHandle *sndEnt, float *origin_out, float (*axis_out)[3]);
unsigned int __cdecl CG_SoundEntityUseEq(const SndEntHandle *sndEnt);
const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum);
int __cdecl CG_IsValidRemoteInputState(int localClientNum);
void __cdecl CG_SetTime(int serverTime);
void __cdecl CG_SetServerCommandSequence(int reliableSent);
void __cdecl CG_GameMessage(const char *msg, int flags);
void __cdecl CG_BoldGameMessage(const char *msg, int flags);
void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, snd_alias_list_t **sound);
void __cdecl CG_AddAudioPhysicsClass(PhysPreset *physPreset, char (*classes)[64], int *nclasses);
void CG_RegisterPhysicsSounds_FastFile();
void CG_RegisterPhysicsSounds();
void __cdecl CG_RegisterSounds();
void __cdecl CG_LoadWorld(int savegame);
void __cdecl RegisterNightVisionAssets(int localClientNum);
void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname);
void __cdecl CG_StartAmbient(int localClientNum);
void __cdecl CG_StopSoundAlias(const int localClientNum, SndEntHandle *entitynum, snd_alias_list_t *aliasList);
void __cdecl CG_StopSoundsOnEnt(const int localClientNum, SndEntHandle *entitynum);
void __cdecl CG_StopSoundAliasByName(int localClientNum, SndEntHandle *entityNum, const char *aliasName);
void __cdecl CG_StopClientSoundAliasByName(int localClientNum, const char *aliasName);
void __cdecl CG_SubtitlePrint(int msec, snd_alias_t *alias, long double a3);
void __cdecl CG_SubtitleSndLengthNotify(int msec, snd_alias_t *lengthNotifyData, long double a3);
void __cdecl CG_ScriptNotifySndLengthNotify(int msec, void *lengthNotifyData);
void __cdecl CG_AddFXSoundAlias(int localClientNum, const float *origin, snd_alias_list_t *aliasList);
int __cdecl CG_PlaySoundAlias(
    int localClientNum,
    SndEntHandle *entitynum,
    const float *origin,
    snd_alias_list_t *aliasList);
int __cdecl CG_PlaySoundAliasByName(
    int localClientNum,
    SndEntHandle *entitynum,
    const float *origin,
    const char *aliasname);
int __cdecl CG_PlaySoundAliasAsMasterByName(
    int localClientNum,
    SndEntHandle *entitynum,
    const float *origin,
    const char *aliasname);
void __cdecl CG_LoadHudMenu(int localClientNum);
void __cdecl CG_InitViewDimensions(int localClientNum);
char *__cdecl CG_GetTeamName(team_t team);
const char *__cdecl CG_GetPlayerTeamName(int localClientNum);
const char *__cdecl CG_GetPlayerOpposingTeamName(int localClientNum);
bool __cdecl CG_IsPlayerDead(int localClientNum);
int __cdecl CG_GetPlayerClipAmmoCount(int localClientNum);
void __cdecl CG_InitDof(GfxDepthOfField *dof);
void __cdecl CG_Init(int localClientNum, int savegame);
void __cdecl CG_FreeWeapons(int localClientNum);
void __cdecl CG_Shutdown(int localClientNum);
void *__cdecl Hunk_AllocXAnimClient(int size);
int __cdecl CG_PlayClientSoundAlias(int localClientNum, snd_alias_list_t *aliasList);
int __cdecl CG_PlayClientSoundAliasByName(int localClientNum, const char *aliasname);
int __cdecl CG_PlayEntitySoundAlias(int localClientNum, SndEntHandle *entitynum, snd_alias_list_t *aliasList);
int __cdecl CG_PlayEntitySoundAliasByName(int localClientNum, SndEntHandle *entitynum, const char *aliasname);
