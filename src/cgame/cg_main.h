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

const rectDef_s *__fastcall Window_GetRect(const windowDef_t *w);
connstate_t __fastcall CL_GetLocalClientConnectionState(int localClientNum);
const dvar_s *CG_RegisterDvars();
void __fastcall TRACK_cg_main();
void __fastcall CG_GetDObjOrientation(int localClientNum, int dobjHandle, float (*axis)[3], float *origin);
void __fastcall CG_CopyEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3]);
void __fastcall CG_GetSoundEntityOrientation(const SndEntHandle *sndEnt, float *origin_out, float (*axis_out)[3]);
unsigned int __fastcall CG_SoundEntityUseEq(const SndEntHandle *sndEnt);
const playerState_s *__fastcall CG_GetPredictedPlayerState(int localClientNum);
int __fastcall CG_IsValidRemoteInputState(int localClientNum);
void __fastcall CG_SetTime(int serverTime);
void __fastcall CG_SetServerCommandSequence(int reliableSent);
void __fastcall CG_GameMessage(const char *msg, int flags);
void __fastcall CG_BoldGameMessage(const char *msg, int flags);
void __fastcall CG_RegisterSurfaceTypeSounds(const char *pszType, snd_alias_list_t **sound);
void __fastcall CG_AddAudioPhysicsClass(PhysPreset *physPreset, char (*classes)[64], int *nclasses);
void CG_RegisterPhysicsSounds_FastFile();
void CG_RegisterPhysicsSounds();
void __fastcall CG_RegisterSounds();
void __fastcall CG_LoadWorld(int savegame);
void __fastcall RegisterNightVisionAssets(int localClientNum);
void __fastcall CG_RegisterGraphics(int localClientNum, const char *mapname);
void __fastcall CG_StartAmbient(int localClientNum);
void __fastcall CG_StopSoundAlias(const int localClientNum, SndEntHandle *entitynum, snd_alias_list_t *aliasList);
void __fastcall CG_StopSoundsOnEnt(const int localClientNum, SndEntHandle *entitynum);
void __fastcall CG_StopSoundAliasByName(int localClientNum, SndEntHandle *entityNum, const char *aliasName);
void __fastcall CG_StopClientSoundAliasByName(int localClientNum, const char *aliasName);
void __fastcall CG_SubtitlePrint(int msec, snd_alias_t *alias, long double a3);
void __fastcall CG_SubtitleSndLengthNotify(int msec, snd_alias_t *lengthNotifyData, long double a3);
void __fastcall CG_ScriptNotifySndLengthNotify(int msec, void *lengthNotifyData);
void __fastcall CG_AddFXSoundAlias(int localClientNum, const float *origin, snd_alias_list_t *aliasList);
int __fastcall CG_PlaySoundAlias(
    int localClientNum,
    SndEntHandle *entitynum,
    const float *origin,
    snd_alias_list_t *aliasList);
int __fastcall CG_PlaySoundAliasByName(
    int localClientNum,
    SndEntHandle *entitynum,
    const float *origin,
    const char *aliasname);
int __fastcall CG_PlaySoundAliasAsMasterByName(
    int localClientNum,
    SndEntHandle *entitynum,
    const float *origin,
    const char *aliasname);
void __fastcall CG_LoadHudMenu(int localClientNum);
void __fastcall CG_InitViewDimensions(int localClientNum);
char *__fastcall CG_GetTeamName(team_t team);
const char *__fastcall CG_GetPlayerTeamName(int localClientNum);
const char *__fastcall CG_GetPlayerOpposingTeamName(int localClientNum);
bool __fastcall CG_IsPlayerDead(int localClientNum);
int __fastcall CG_GetPlayerClipAmmoCount(int localClientNum);
void __fastcall CG_InitDof(GfxDepthOfField *dof);
void __fastcall CG_Init(int localClientNum, int savegame);
void __fastcall CG_FreeWeapons(int localClientNum);
void __fastcall CG_Shutdown(int localClientNum);
void *__fastcall Hunk_AllocXAnimClient(int size);
int __fastcall CG_PlayClientSoundAlias(int localClientNum, snd_alias_list_t *aliasList);
int __fastcall CG_PlayClientSoundAliasByName(int localClientNum, const char *aliasname);
int __fastcall CG_PlayEntitySoundAlias(int localClientNum, SndEntHandle *entitynum, snd_alias_list_t *aliasList);
int __fastcall CG_PlayEntitySoundAliasByName(int localClientNum, SndEntHandle *entitynum, const char *aliasname);
