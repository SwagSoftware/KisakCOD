#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <qcommon/qcommon.h>
#include <bgame/bg_local.h>
#include <script/scr_variable.h>
#include "game_public.h"

#define MAX_HUDELEMS_TOTAL 0x100

struct SpawnFuncEntry
{
    const char *classname;
    void(*callback)(gentity_s *);
};

struct ent_field_t
{
    const char *name;
    int ofs;
    fieldtype_t type;
    void(*callback)(gentity_s *, int);
};

struct animscripted_s
{
    float axis[4][3];
    float originError[3];
    float anglesError[3];
    unsigned __int16 anim;
    unsigned __int16 root;
    unsigned __int8 bStarted;
    unsigned __int8 mode;
    float fHeightOfs;
    float fEndPitch;
    float fEndRoll;
    float fOrientLerp;
};

struct entityHandler_t
{
    void(*think)(gentity_s *);
    void(*reached)(gentity_s *);
    void(*blocked)(gentity_s *, gentity_s *);
    void(*touch)(gentity_s *, gentity_s *, int);
    void(*use)(gentity_s *, gentity_s *, gentity_s *);
    void(*pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int);
    void(*die)(gentity_s *, gentity_s *, gentity_s *, const int, const int, const int, const float *, const hitLocation_t);
    void(*entinfo)(gentity_s *, float *);
    void(*controller)(const gentity_s *, int *);
    int methodOfDeath;
    int splashMethodOfDeath;
};

// g_active
void __cdecl P_DamageFeedback(gentity_s *player);
void __cdecl G_SetClientSound(gentity_s *ent);
void __cdecl G_TouchEnts(gentity_s *ent, int numtouch, int *touchents);
void __cdecl ClientImpacts(gentity_s *ent, pmove_t *pm);
void __cdecl G_DoTouchTriggers(gentity_s *ent);
void __cdecl NotifyGrenadePullback(gentity_s *ent, unsigned int weaponIndex);
bool __cdecl IsLiveGrenade(gentity_s *ent);
void __cdecl AttemptLiveGrenadePickup(gentity_s *clientEnt);
void __cdecl ClientEvents(gentity_s *ent, int oldEventSequence);
void __cdecl Client_ClaimNode(gentity_s *ent);
void __cdecl G_PlayerStateToEntityStateExtrapolate(playerState_s *ps, entityState_s *s, int time, int snap);
// local variable allocation has failed, the output may be wrong!
void __cdecl ClientThink_real(gentity_s *ent, long double a2);
void __cdecl ClientThink(int clientNum);
void __cdecl ClientEndFrame(gentity_s *ent);
gentity_s *__cdecl G_GetPlayer();
void __cdecl G_UpdatePlayer(gentity_s *ent);
void __cdecl G_UpdatePlayerTriggers(gentity_s *ent);



// g_animscripted
void __cdecl LocalToWorldOriginAndAngles(
    const float (*matrix)[3],
    const float *trans,
    const float *rot,
    float *origin,
    float *angles);
void __cdecl CalcDeltaOriginAndAngles(
    DObj_s *obj,
    unsigned int anim,
    const float (*matrix)[3],
    float *origin,
    float *angles);
void __cdecl GetDeltaOriginAndAngles(
    const XAnim_s *anims,
    unsigned int anim,
    const float (*matrix)[3],
    float *trans,
    float *origin,
    float *angles);
void __cdecl G_Animscripted_DeathPlant(
    gentity_s *ent,
    const XAnim_s *anims,
    unsigned int anim,
    float *origin,
    const float *angles);
void __cdecl G_AnimScripted_ClearAnimWeights(
    DObj_s *obj,
    XAnimTree_s *pAnimTree,
    unsigned int root,
    struct actor_s *pActor);
void __cdecl G_Animscripted(
    gentity_s *ent,
    float *origin,
    const float *angles,
    unsigned int anim,
    unsigned int root,
    unsigned int notifyName,
    unsigned __int8 animMode);
void __cdecl G_ReduceOriginError(float *origin, float *originError, double maxChange);
void __cdecl G_ReduceAnglesError(float *angles, float *anglesError, double maxChange);
void __cdecl G_AnimScripted_Think_DeathPlant(gentity_s *ent, XAnimTree_s *tree, float *origin, float *angles);
void __cdecl G_AnimScripted_UpdateEntityOriginAndAngles(gentity_s *ent, float *origin, const float *angles);
void __cdecl G_Animscripted_Think(gentity_s *ent);
void __cdecl GScr_GetStartOrigin();
void __cdecl GScr_GetStartAngles();
void __cdecl GScr_GetCycleOriginOffset();


// g_client
void __cdecl G_FinishSetupSpawnPoint(gentity_s *ent);
void __cdecl G_SetupSpawnPoint(gentity_s *ent);
void __cdecl SP_info_player_start(gentity_s *ent);
int __cdecl SpotWouldTelefrag(gentity_s *spot);
gentity_s *__cdecl SelectNearestDeathmatchSpawnPoint(const float *from);
gentity_s *__cdecl SelectRandomDeathmatchSpawnPoint();
gentity_s *__cdecl SelectSpawnPoint(const float *avoidPoint, float *origin, float *angles);
gentity_s *__cdecl SelectInitialSpawnPoint(float *origin, float *angles);
void __cdecl SetClientOrigin(gentity_s *ent, float *origin);
void __cdecl InitClientDeltaAngles(gclient_s *client, long double a2);
void __cdecl SetClientViewAngle(gentity_s *ent, const float *angle, long double a3);
void __cdecl G_GetPlayerViewOrigin(const playerState_s *ps, float *origin);
void __cdecl G_GetPlayerViewDirection(const gentity_s *ent, float *forward, float *right, float *up);
int __cdecl Client_GetPushed(gentity_s *pSelf, gentity_s *pOther);
void __cdecl Client_Touch(gentity_s *pSelf, gentity_s *pOther, int bTouched);
void __cdecl respawn(gentity_s *ent);
char *__cdecl ClientConnect(int clientNum);
void __cdecl ClientSpawn(gentity_s *ent);
void __cdecl HeadHitEnt_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    const float *vDir,
    const hitLocation_t hitLoc,
    const int weaponIdx);
void __cdecl HeadHitEnt_Die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int meansOfDeath,
    int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc);
void __cdecl G_UpdateHeadHitEnt(gentity_s *pSelf);
void __cdecl G_RemoveHeadHitEnt(gentity_s *pSelf);
void __cdecl ClientBegin(int clientNum);


// g_main
const dvar_s *G_RegisterServerDemoDvars();
void G_RegisterDebugDvars();
void __cdecl TRACK_g_main();
int __cdecl G_GetServerSnapTime();
int __cdecl G_GetTime();
int __cdecl G_GetSpecialIndex(int entnum);
void G_RegisterDvars();
void G_InitDvars();
void __cdecl G_FreeEntities();
void *__cdecl Hunk_AllocActorXAnimServer(int size);
void *G_LoadAnimTreeInstances();
void G_FreeAnimTreeInstances();
void __cdecl G_ClearLowHunk();
bool __cdecl G_DemoPlaying();
void *__cdecl Hunk_AllocXAnimServer(int size);
int __cdecl G_ExitAfterConnectPaths();
void *GScr_LoadScriptsAndAnims();
void ScriptIOFilesInit();
void ScriptIOFilesShutdown();
void __cdecl G_PrintFastFileErrors(const char *fastfile, int a2, const char *a3);
void __cdecl G_PrintAllFastFileErrors(int a1, int a2, const char *a3);
void __cdecl G_InitGame(
    unsigned int randomSeed,
    int restart,
    int checksum,
    int loadScripts,
    int savegame,
    SaveGame **save,
    int a7,
    int a8,
    int a9,
    int a10,
    int a11,
    unsigned int a12);
void __cdecl G_ShutdownGame(int clearScripts);
void G_ChangeLevel();
bool __cdecl G_IsNextMapWaiting();
void __cdecl G_SetNextMap(const char *mapname);
void __cdecl G_LoadNextMap();
void __cdecl G_CheckReloadStatus();
void __cdecl G_ApplyEntityEq(gentity_s *ent);
void __cdecl G_RunThink(gentity_s *ent);
void __cdecl G_DrawEntityBBoxes(int a1, int a2, int a3, const float *a4);
void DebugDumpAnims();
void DebugDumpAIEventListeners();
void __cdecl G_CheckLoadGame(int checksum, SaveGame *save);
void __cdecl G_XAnimUpdateEnt(gentity_s *ent);
void __cdecl G_ClientDoPerFrameNotifies(gentity_s *ent);
void __cdecl G_RunFrameForEntityInternal(gentity_s *ent);
void __cdecl G_RunFrameForEntity(gentity_s *ent);
void __cdecl G_RunPreFrame();
int __cdecl G_GetFramePos();
int __cdecl NotifyTriggers();
void __cdecl G_SendClientMessages();
void __cdecl G_ArchiveSpecialEntityInfo(const entityState_s *es, MemoryFile *memFile);
void __cdecl G_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentmask);
int __cdecl G_TraceCapsuleComplete(
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentmask);
void __cdecl G_LocationalTrace(
    trace_t *results,
    const float *start,
    const float *end,
    int passEntityNum,
    int contentmask,
    unsigned __int8 *priorityMap);
void __cdecl G_LocationalTraceAllowChildren(
    trace_t *results,
    const float *start,
    const float *end,
    int passEntityNum,
    int contentmask,
    unsigned __int8 *priorityMap);
int __cdecl G_LocationalTracePassed(
    const float *start,
    const float *end,
    int passEntityNum,
    int passEntityNum1,
    int contentmask,
    unsigned __int8 *priorityMap);
void __cdecl G_SightTrace(int *hitNum, const float *start, const float *end, int passEntityNum, int contentmask);
void __cdecl G_AddDebugString(const float *xyz, const float *color, double scale, const char *pszText);
void __cdecl G_AddDebugStringWithDuration(
    const float *xyz,
    const float *color,
    double scale,
    const char *pszText,
    int duration);
void __cdecl ShowEntityInfo();
int __cdecl G_RunFrame(ServerFrameExtent extent, int timeCap);
void __cdecl G_LoadLevel();


// g_misc
float __cdecl G_GetEntInfoScale();
void __cdecl SP_info_notnull(gentity_s *self);
void __cdecl SP_light(gentity_s *self);
void __cdecl SP_info_volume(gentity_s *self);
void __cdecl TeleportPlayer(gentity_s *player, float *origin, const float *angles);
void __cdecl SP_sound_blend(gentity_s *self);
gentity_s *__cdecl G_SpawnSoundBlend();
void __cdecl G_SetSoundBlend(gentity_s *ent, unsigned __int16 alias0, unsigned __int16 alias1, double lerp);
void __cdecl G_SetSoundBlendVolumeScale(gentity_s *ent, double scale);
float __cdecl G_GetSoundBlendVolumeScale(gentity_s *ent);
void __cdecl EntinfoPosAndScale(gentity_s *self, float *source, float *pos, float *textScale, float *dist);
void __cdecl misc_EntInfo(gentity_s *self, float *source);
void __cdecl EntInfo_Item(gentity_s *self, float *source);


// g_client_script_cmd
int __cdecl G_GetNeededStartAmmo(gentity_s *pSelf, WeaponDef *weapDef);
void __cdecl InitializeAmmo(gentity_s *pSelf, int weaponIndex, unsigned __int8 weaponModel, int hadWeapon);
void __cdecl G_FlushCommandNotifies();
void __cdecl G_ProcessCommandNotifies();

void __cdecl PlayerCmd_giveWeapon(scr_entref_t *entref);
void __cdecl PlayerCmd_takeWeapon(scr_entref_t *entref);
void __cdecl PlayerCmd_takeAllWeapons(scr_entref_t *entref);
void __cdecl PlayerCmd_getCurrentWeapon(scr_entref_t *entref);
void __cdecl PlayerCmd_getCurrentWeaponClipAmmo(scr_entref_t *entref);
void __cdecl PlayerCmd_getCurrentOffhand(scr_entref_t *entref);
void __cdecl PlayerCmd_setOffhandSecondaryClass(scr_entref_t *entref);
void __cdecl PlayerCmd_getOffhandSecondaryClass(scr_entref_t *entref);
void __cdecl PlayerCmd_hasWeapon(scr_entref_t *entref);
void __cdecl PlayerCmd_switchToWeapon(scr_entref_t *entref);
void __cdecl PlayerCmd_switchToOffhand(scr_entref_t *entref);
void __cdecl PlayerCmd_giveStartAmmo(scr_entref_t *entref);
void __cdecl PlayerCmd_giveMaxAmmo(scr_entref_t *entref);
void __cdecl PlayerCmd_getFractionStartAmmo(scr_entref_t *entref);
void __cdecl PlayerCmd_getFractionMaxAmmo(scr_entref_t *entref);
void __cdecl PlayerCmd_setOrigin(scr_entref_t *entref);
void __cdecl PlayerCmd_SetVelocity(scr_entref_t *entref);
void __cdecl PlayerCmd_GetVelocity(scr_entref_t *entref);
void __cdecl PlayerCmd_setAngles(scr_entref_t *entref);
void __cdecl PlayerCmd_getAngles(scr_entref_t *entref);
void __cdecl PlayerCmd_getViewHeight(scr_entref_t *entref);
void __cdecl PlayerCmd_getNormalizedMovement(scr_entref_t *entref);
void __cdecl PlayerCmd_useButtonPressed(scr_entref_t *entref);
void __cdecl PlayerCmd_attackButtonPressed(scr_entref_t *entref);
void __cdecl PlayerCmd_adsButtonPressed(scr_entref_t *entref);
void __cdecl PlayerCmd_meleeButtonPressed(scr_entref_t *entref);
int __cdecl PlayerCmd_CheckButtonPressed();
void __cdecl PlayerCmd_buttonPressed(scr_entref_t *entref);
void __cdecl PlayerCmd_notifyOnCommand(scr_entref_t *entref);
void __cdecl PlayerCmd_playerADS(scr_entref_t *entref);
void __cdecl PlayerCmd_isOnGround(scr_entref_t *entref);
void __cdecl PlayerCmd_SetViewmodel(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowADS(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowJump(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowSprint(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowMelee(scr_entref_t *entref);
void __cdecl PlayerCmd_SetSpreadOverride(scr_entref_t *entref);
void __cdecl PlayerCmd_ResetSpreadOverride(scr_entref_t *entref);
void __cdecl PlayerCmd_ShowViewmodel(scr_entref_t *entref);
void __cdecl PlayerCmd_HideViewmodel(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowStand(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowCrouch(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowProne(scr_entref_t *entref);
void __cdecl PlayerCmd_AllowLean(scr_entref_t *entref);
void __cdecl PlayerCmd_OpenMenu(scr_entref_t *entref);
void __cdecl PlayerCmd_OpenMenuNoMouse(scr_entref_t *entref);
void __cdecl PlayerCmd_CloseMenu(scr_entref_t *entref);
void __cdecl PlayerCmd_FreezeControls(scr_entref_t *entref);
void __cdecl PlayerCmd_SetEQLerp(scr_entref_t *entref);
void __cdecl PlayerCmd_SetEQ(scr_entref_t *entref);
void __cdecl PlayerCmd_DeactivateEq(scr_entref_t *entref);
void __cdecl PlayerCmd_SetReverb(scr_entref_t *entref);
void __cdecl PlayerCmd_DeactivateReverb(scr_entref_t *entref);
void __cdecl PlayerCmd_SetChannelVolumes(scr_entref_t *entref);
void __cdecl PlayerCmd_DeactivateChannelVolumes(scr_entref_t *entref);
void __cdecl ScrCmd_IsLookingAt(scr_entref_t *entref);
void __cdecl PlayerCmd_IsFiring(scr_entref_t *entref);
void __cdecl PlayerCmd_IsThrowingGrenade(scr_entref_t *entref);
void __cdecl PlayerCmd_IsMeleeing(scr_entref_t *entref);
void __cdecl ScrCmd_PlayLocalSound(scr_entref_t *entref);
void __cdecl ScrCmd_StopLocalSound(scr_entref_t *entref);
void __cdecl ScrCmd_SetAutoPickup(scr_entref_t *entref);
void __cdecl PlayerCmd_SetWeaponAmmoClip(scr_entref_t *entref);
void __cdecl PlayerCmd_SetWeaponAmmoStock(scr_entref_t *entref);
void __cdecl PlayerCmd_GetWeaponAmmoClip(scr_entref_t *entref);
void __cdecl PlayerCmd_GetWeaponAmmoStock(scr_entref_t *entref);
void __cdecl PlayerCmd_AnyAmmoForWeaponModes(scr_entref_t *entref);
void __cdecl PlayerCmd_EnableHealthShield(scr_entref_t *entref);
void __cdecl PlayerCmd_SetClientDvar(scr_entref_t *entref);
void __cdecl PlayerCmd_SetClientDvars(scr_entref_t *entref);
void __cdecl PlayerCmd_BeginLocationSelection(scr_entref_t *entref);
void __cdecl PlayerCmd_EndLocationSelection(scr_entref_t *entref);
void __cdecl PlayerCmd_WeaponLockStart(scr_entref_t *entref);
void __cdecl PlayerCmd_WeaponLockFinalize(scr_entref_t *entref);
void __cdecl PlayerCmd_WeaponLockFree(scr_entref_t *entref);
void __cdecl PlayerCmd_WeaponLockTargetTooClose(scr_entref_t *entref);
void __cdecl PlayerCmd_WeaponLockNoClearance(scr_entref_t *entref);
void __cdecl PlayerCmd_SetActionSlot(scr_entref_t *entref);
void __cdecl PlayerCmd_DisableWeapons(scr_entref_t *entref);
void __cdecl PlayerCmd_EnableWeapons(scr_entref_t *entref);
void __cdecl PlayerCmd_NightVisionForceOff(scr_entref_t *entref);
void __cdecl PlayerCmd_GetWeaponsList(scr_entref_t *entref);
void __cdecl PlayerCmd_GetWeaponsListPrimaries(scr_entref_t *entref);
void __cdecl PlayerCmd_EnableInvulnerability(scr_entref_t *entref);
void __cdecl PlayerCmd_DisableInvulnerability(scr_entref_t *entref);
void __cdecl PlayerCmd_ForceViewmodelAnimation(scr_entref_t *entref);
void __cdecl PlayerCmd_DisableTurretDismount(scr_entref_t *entref);
void __cdecl PlayerCmd_EnableTurretDismount(scr_entref_t *entref);
void __cdecl PlayerCmd_UploadScore(scr_entref_t *entref);
void __cdecl PlayerCmd_UploadTime(scr_entref_t *entref);
void(__cdecl *__cdecl Player_GetMethod(const char **pName))(scr_entref_t);
void __cdecl G_AddCommandNotify(volatile unsigned __int16 notify);


// g_cmds
int __cdecl CheatsOkInternal(gentity_s *ent);
int __cdecl CheatsOk(gentity_s *ent);
char *__cdecl ConcatArgs(int start);
void __cdecl SanitizeString(char *in, char *out);
void __cdecl G_setfog(const char *fogstring);
void __cdecl Cmd_Fogswitch_f();
void __cdecl Cmd_SetSoundLength_f();
void __cdecl Cmd_RemoveCorpse_f();
void __cdecl Cmd_Give_f(gentity_s *ent);
void __cdecl Cmd_Take_f(gentity_s *ent);
void __cdecl Cmd_God_f(gentity_s *ent);
void __cdecl Cmd_DemiGod_f(gentity_s *ent);
void __cdecl Cmd_Notarget_f(gentity_s *ent);
void __cdecl Cmd_Noclip_f(gentity_s *ent);
void __cdecl Cmd_UFO_f(gentity_s *ent);
void __cdecl Cmd_Kill_f(gentity_s *ent);
void __cdecl Cmd_Where_f(gentity_s *ent);
void __cdecl Cmd_SetViewpos_f(gentity_s *ent);
void __cdecl Cmd_JumpToNode_f(gentity_s *ent);
void __cdecl Cmd_InterruptCamera_f(gentity_s *ent);
void __cdecl Cmd_DropWeapon_f(gentity_s *pSelf);
void __cdecl Cmd_MenuResponse_f(gentity_s *pEnt);
// attributes: thunk
void __cdecl Cmd_PrintEntities_f();
void Cmd_VisionSetNaked_f();
void Cmd_VisionSetNight_f();
void __cdecl ClientCommand(int clientNum, const char *s);


// g_combat
void __cdecl TRACK_g_combat();
void __cdecl G_HitLocStrcpy(unsigned __int8 *pMember, const char *pszKeyValue);
void __cdecl G_ParseHitLocDmgTable();
void __cdecl TossClientItems(gentity_s *self);
void __cdecl LookAtKiller(gentity_s *self, gentity_s *inflictor, gentity_s *attacker);
int __cdecl G_MeansOfDeathFromScriptParam(unsigned int scrParam);
void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int meansOfDeath,
    unsigned int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc);
float __cdecl G_GetWeaponHitLocationMultiplier(unsigned int hitLoc, unsigned int weapon);
void __cdecl handleDeathInvulnerability(gentity_s *targ, int prevHealth, int mod);
void __cdecl G_DamageNotify(
    unsigned __int16 notify,
    gentity_s *targ,
    gentity_s *attacker,
    const float *dir,
    const float *point,
    int damage,
    int mod,
    unsigned int modelIndex,
    unsigned int partName,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    unsigned int a28);
int __cdecl G_GetWeaponIndexForEntity(const gentity_s *ent);
void __cdecl G_DamageKnockback(
    gentity_s *targ,
    const gentity_s *attacker,
    const float *dir,
    float *scaledDir,
    __int64 dflags,
    int mod);
bool __cdecl G_ShouldTakeBulletDamage(gentity_s *targ, gentity_s *attacker);
void __cdecl G_Damage(
    gentity_s *targ,
    gentity_s *inflictor,
    gentity_s *attacker,
    const float *dir,
    const float *point,
    int damage,
    unsigned int dflags,
    int mod,
    int weapon,
    hitLocation_t hitLoc,
    unsigned int modelIndex,
    unsigned int partName,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    unsigned int a28,
    int a29,
    unsigned int a30,
    int a31,
    unsigned int a32,
    int a33,
    unsigned int a34);
int __cdecl G_CanRadiusDamageFromPos(
    gentity_s *targ,
    const float *targetPos,
    gentity_s *inflictor,
    const float *centerPos,
    double radius,
    double coneAngleCos,
    float *coneDirection,
    double maxHeight,
    bool useEyeOffset,
    float *contentMask);
float __cdecl EntDistToPoint(float *origin, gentity_s *ent);
void __cdecl GetEntListForRadius(
    const float *origin,
    double radius_max,
    double radius_min,
    int *entList,
    int *entListCount,
    int *a6,
    unsigned int *a7);
void __cdecl AddScrTeamName(team_t team);
float __cdecl G_GetRadiusDamageDistanceSquared(float *damageOrigin, gentity_s *ent);
bool __cdecl G_WithinDamageRadius(float *damageOrigin, double radiusSquared, gentity_s *ent, gentity_s *a4);
bool __cdecl G_ClientFlashbanged(gclient_s *client);
int __cdecl G_GetHitLocationString(unsigned int hitLoc);
int __cdecl G_CanRadiusDamage(
    gentity_s *targ,
    gentity_s *inflictor,
    const float *centerPos,
    double radius,
    double coneAngleCos,
    float *coneDirection,
    float *contentMask,
    float *a8,
    int a9);
void __cdecl FlashbangBlastEnt(
    gentity_s *ent,
    float *blastOrigin,
    double radius_max,
    double radius_min,
    gentity_s *attacker,
    team_t team,
    gentity_s *a7,
    team_t a8);
void __cdecl G_FlashbangBlast(
    float *origin,
    double radius_max,
    double radius_min,
    gentity_s *attacker,
    int *team,
    gentity_s *a6,
    team_t a7);
int __cdecl G_RadiusDamage(
    float *origin,
    gentity_s *inflictor,
    gentity_s *attacker,
    double fInnerDamage,
    double fOuterDamage,
    double radius,
    double coneAngleCos,
    float *coneDirection,
    gentity_s *ignore,
    int mod,
    int weapon,
    float *a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    int a31,
    gentity_s *a32,
    int a33,
    int a34,
    int a35,
    unsigned int a36);


// g_spawn
int __cdecl G_LevelSpawnString(const char *key, const char *defaultString, const char **out);
int __cdecl G_SpawnFloat(const char *key, const char *defaultString, float *out);
int __cdecl G_SpawnInt(const char *key, const char *defaultString, int *out);
int __cdecl G_SpawnVector(const char *key, const char *defaultString, float *out);
void __cdecl Scr_ReadOnlyField(gentity_s *ent, int offset);
unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, const char *value);
void G_SpawnStruct();
void __cdecl G_DuplicateEntityFields(gentity_s *dest, const gentity_s *source);
void __cdecl G_DuplicateScriptFields(gentity_s *dest, const gentity_s *source);
const gitem_s *__cdecl G_GetItemForClassname(const char *classname, unsigned __int8 model);
void(__cdecl *__cdecl G_FindSpawnFunc(
    const char *classname,
    const SpawnFuncEntry *spawnFuncArray,
    int spawnFuncCount))(gentity_s *);
void __cdecl G_PrintBadModelMessage(gentity_s *ent);
int __cdecl G_CallSpawnEntity(gentity_s *ent);
void __cdecl GScr_AddFieldsForEntity();
void __cdecl GScr_AddFieldsForRadiant();
void __cdecl Scr_FreeEntity(gentity_s *ent);
void __cdecl Scr_AddEntity(gentity_s *ent);
gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index);
gentity_s *__cdecl Scr_GetEntity(unsigned int index);
void __cdecl Scr_FreeHudElem(game_hudelem_s *hud);
void __cdecl Scr_AddHudElem(game_hudelem_s *hud);
game_hudelem_s *__cdecl Scr_GetHudElem(unsigned int index);
int __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount);
void __cdecl Scr_AddExecEntThread(gentity_s *ent, int handle, unsigned int paramcount);
void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount);
void __cdecl Scr_GetGenericEnt(unsigned int offset, unsigned int name);
void __cdecl Scr_GetEnt();
void __cdecl Scr_GetGenericEntArray(unsigned int offset, unsigned int name);
void __cdecl Scr_GetEntArray();
void __cdecl GScr_SetDynamicEntityField(gentity_s *ent, unsigned int index);
void __cdecl SP_worldspawn();
void __cdecl G_LoadStructs();
void __cdecl G_SetEntityScriptVariable(const char *key, const char *value, gentity_s *ent);
void __cdecl G_ParseEntityField(const char *key, const char *value, gentity_s *ent, int ignoreModel);
void __cdecl G_ParseEntityFields(gentity_s *ent, int ignoreModel);
void G_CallSpawn();
void __cdecl Scr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs);
void __cdecl Scr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs);
void __cdecl G_SpawnEntitiesFromString();
int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset);
int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, int offset);
void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_GetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset);


// g_targets
void __cdecl G_InitTargets();
void __cdecl G_LoadTargets();
void __cdecl Scr_Target_SetShader();
void __cdecl Scr_Target_SetOffscreenShader();
void __cdecl Scr_Target_GetArray();
int __cdecl TargetIndex(gentity_s *ent);
void __cdecl Scr_Target_IsTarget();
void __cdecl Scr_Target_Set();
int __cdecl Targ_Remove(gentity_s *ent);
void __cdecl Targ_RemoveAll();
void __cdecl Scr_Target_Remove();
int __cdecl G_WorldDirToScreenPos(
    const gentity_s *player,
    double fov_x,
    const float *worldDir,
    const float *outScreenPos,
    float *a5);
int __cdecl ScrGetTargetScreenPos(float *screenPos);
void __cdecl Scr_Target_IsInCircle();
void __cdecl Scr_Target_IsInRect();
void __cdecl Scr_Target_StartLockOn(int a1, unsigned int a2);
void __cdecl Scr_Target_ClearLockOn();
int __cdecl GetTargetIdx(const gentity_s *ent);
int __cdecl G_TargetGetOffset(const gentity_s *targ, float *result);
int __cdecl G_TargetAttackProfileTop(const gentity_s *ent);
void __cdecl Scr_Target_SetAttackMode();
void __cdecl Scr_Target_SetJavelinOnly();


// g_trigger
void __cdecl G_Trigger(gentity_s *self, gentity_s *other);
int __cdecl InitTrigger(gentity_s *self);
void __cdecl InitTriggerWait(gentity_s *ent, int spawnflag);
void __cdecl InitSentientTrigger(gentity_s *self);
void __cdecl multi_trigger(gentity_s *ent, gentity_s *activator);
void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int bTouched);
void __cdecl SP_trigger_multiple(gentity_s *ent);
void __cdecl SP_trigger_radius(gentity_s *ent);
void __cdecl SP_trigger_disk(gentity_s *ent);
void __cdecl Touch_FriendlyChain(gentity_s *self, gentity_s *other, int bTouched);
void __cdecl SP_trigger_friendlychain(gentity_s *ent);
void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int bTouched);
void __cdecl hurt_use(gentity_s *self, gentity_s *other, gentity_s *activator);
void __cdecl SP_trigger_hurt(gentity_s *self);
void __cdecl SP_trigger_once(gentity_s *ent);
bool __cdecl Respond_trigger_damage(gentity_s *trigger, int damageType);
void __cdecl Activate_trigger_damage(gentity_s *pEnt, gentity_s *pOther, int iDamage, int iMOD);
void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *pActivator);
void __cdecl Pain_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    const float *vDir,
    const hitLocation_t hitLoc,
    const int weaponIdx);
void __cdecl Die_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    int iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc);
void __cdecl SP_trigger_damage(gentity_s *pSelf);
void __cdecl G_CheckHitTriggerDamage(
    gentity_s *pActivator,
    const float *vStart,
    const float *vEnd,
    int iDamage,
    unsigned int iMOD);
void __cdecl G_GrenadeTouchTriggerDamage(
    gentity_s *pActivator,
    const float *vStart,
    const float *vEnd,
    int iDamage,
    int iMOD);
void __cdecl SP_trigger_lookat(gentity_s *self);


// g_utils
void __cdecl TRACK_g_utils();
void __cdecl G_DumpConfigStrings(int start, int max);
int __cdecl G_FindConfigstringIndex(const char *name, int start, int max, int create, const char *errormsg);
int __cdecl G_LocalizedStringIndex(const char *string);
int __cdecl G_MaterialIndex(const char *name);
void __cdecl G_SetModelIndex(int modelIndex, const char *name);
int __cdecl G_ModelIndex(const char *name);
XModel *__cdecl G_GetModel(int index);
bool __cdecl G_GetModelBounds(int index, float *outMins, float *outMaxs);
int __cdecl G_XModelBad(int index);
unsigned int __cdecl G_ModelName(unsigned int index);
void __cdecl G_EntityCentroidWithBounds(const gentity_s *ent, const float *mins, const float *maxs, float *centroid);
void __cdecl G_EntityCentroid(const gentity_s *ent, float *centroid);
int __cdecl G_EffectIndex(const char *name);
int __cdecl G_ShellShockIndex(const char *name);
unsigned int __cdecl G_SoundAliasIndexTransientAdvance(unsigned __int16 aliasIndex, int offset);
unsigned int __cdecl G_SoundAliasIndexTransient(const char *name);
int __cdecl G_SoundAliasIndexPermanent(const char *name);
int __cdecl G_RumbleIndex(const char *name);
void __cdecl G_SetClientDemoTime(int time);
void __cdecl G_SetClientDemoServerSnapTime(int time);
void __cdecl G_ClearDemoEntities();
void __cdecl G_UpdateDemoEntity(entityState_s *es);
unsigned int __cdecl G_GetEntAnimTreeId(int entnum);
XAnimTree_s *__cdecl G_GetEntAnimTreeForId(int entnum, unsigned int id);
void __cdecl G_ShutdownClientDemo();
XAnimTree_s *__cdecl G_GetEntAnimTree(gentity_s *ent);
void __cdecl G_CheckDObjUpdate(gentity_s *ent);
void __cdecl G_SetModel(gentity_s *ent, const char *modelName);
// attributes: thunk
void __cdecl G_ReplaceModel_FastFile(const char *originalName, const char *replacementName);
void __cdecl G_OverrideModel(unsigned int modelindex, const char *defaultModelName);
void __cdecl G_PrecacheDefaultModels();
int __cdecl G_EntIsLinkedTo(gentity_s *ent, gentity_s *parent);
void __cdecl G_UpdateViewAngleClamp(gclient_s *client, const float *worldAnglesCenter);
void __cdecl G_UpdateGroundTilt(gclient_s *client);
bool __cdecl G_SlideMove(
    double deltaT,
    float *origin,
    float *velocity,
    float *mins,
    const float *maxs,
    const float *gravity,
    unsigned __int8 passEntityNum,
    int clipMask,
    int a9);
void __cdecl G_StepSlideMove(
    double deltaT,
    float *origin,
    float *velocity,
    float *mins,
    const float *maxs,
    const float *gravity,
    unsigned __int8 passEntityNum,
    int clipMask,
    int a9);
void __cdecl G_SafeDObjFree(gentity_s *ent);
int __cdecl G_DObjUpdateServerTime(gentity_s *ent, int bNotify);
void __cdecl G_DObjCalcPose(gentity_s *ent, int *partBits);
void __cdecl G_DObjCalcBone(const gentity_s *ent, int boneIndex);
struct DObjAnimMat *__cdecl G_DObjGetLocalBoneIndexMatrix(const gentity_s *ent, int boneIndex);
void __cdecl G_DObjGetWorldBoneIndexMatrix(const gentity_s *ent, int boneIndex, float (*tagMat)[3]);
void __cdecl G_DObjGetWorldBoneIndexPos(const gentity_s *ent, int boneIndex, float *pos);
struct DObjAnimMat *__cdecl G_DObjGetLocalTagMatrix(const gentity_s *ent, unsigned int tagName);
int __cdecl G_DObjGetWorldTagMatrix(const gentity_s *ent, unsigned int tagName, float (*tagMat)[3]);
int __cdecl G_DObjGetWorldTagPos(const gentity_s *ent, unsigned int tagName, float *pos);
void __cdecl G_DObjGetWorldTagPos_CheckTagExists(const gentity_s *ent, unsigned int tagName, float *pos);
gentity_s *__cdecl G_Find(gentity_s *from, int fieldofs, unsigned __int16 match);
void __cdecl G_InitGentity(gentity_s *e);
void __cdecl G_PrintEntities();
gentity_s *__cdecl G_Spawn();
void __cdecl G_FreeEntityRefs(gentity_s *ed);
void __cdecl G_FreeAllEntityRefs();
void __cdecl G_FreeEntityDelay(gentity_s *ed);
void __cdecl G_BroadcastEntity(gentity_s *ent);
void __cdecl G_FreeEntityAfterEvent(gentity_s *ent);
int __cdecl G_SaveFreeEntities(unsigned __int8 *buf);
void __cdecl G_LoadFreeEntities(unsigned __int8 *buf);
void __cdecl G_AddPredictableEvent(gentity_s *ent, int event, unsigned int eventParm);
void __cdecl G_AddEvent(gentity_s *ent, unsigned int event, unsigned int eventParm);
void __cdecl G_RegisterSoundWait(gentity_s *ent, unsigned __int16 index, unsigned int notifyString, int stoppable);
void __cdecl G_PlaySoundAliasWithNotify(
    gentity_s *ent,
    unsigned __int16 index,
    unsigned int notifyString,
    int stoppable,
    unsigned int event,
    unsigned int notifyevent);
void __cdecl G_PlaySoundAlias(gentity_s *ent, unsigned __int16 index);
void __cdecl G_SetOrigin(gentity_s *ent, float *origin);
void __cdecl G_SetAngle(gentity_s *ent, float *angle);
void __cdecl G_SetConstString(unsigned __int16 *to, const char *from);
const char *__cdecl G_GetEntityTypeName(const gentity_s *ent);
void __cdecl G_SetPM_MPViewer(bool setting);
void __cdecl G_srand(unsigned int seed);
unsigned int __cdecl G_GetRandomSeed();
unsigned int __cdecl G_rand();
float __cdecl G_flrand(double min, double max);
int __cdecl G_irand(int min, int max);
float __cdecl G_random();
float __cdecl G_crandom();
void __cdecl G_CalcTagParentAxis(gentity_s *ent, float (*parentAxis)[3]);
void __cdecl G_CalcTagParentRelAxis(gentity_s *ent, float (*parentRelAxis)[3]);
void __cdecl G_CalcTagAxis(gentity_s *ent, int bAnglesOnly);
void __cdecl G_SetFixedLink(gentity_s *ent, unsigned int eAngles);
void __cdecl G_SetPlayerFixedLink(gentity_s *ent);
void __cdecl G_GeneralLink(gentity_s *ent);
gentity_s *__cdecl G_TempEntity(float *origin, int event);
void __cdecl G_PlaySoundAliasAtPoint(float *origin, unsigned __int16 index);
void __cdecl G_EntUnlink(gentity_s *ent);
void __cdecl G_UpdateTagInfo(gentity_s *ent, int bParentHasDObj);
void __cdecl G_UpdateTagInfoOfChildren(gentity_s *parent, int bHasDObj);
void __cdecl G_EntUnlinkFree(gentity_s *ent);
void __cdecl G_FreeEntity(gentity_s *ed);
void __cdecl G_UpdateTags(gentity_s *ent, int bHasDObj);
void __cdecl G_DObjUpdate(gentity_s *ent);
int __cdecl G_EntDetach(gentity_s *ent, const char *modelName, unsigned int tagName);
void __cdecl G_EntDetachAll(gentity_s *ent);
int __cdecl G_EntLinkToInternal(gentity_s *ent, gentity_s *parent, unsigned int tagName);
int __cdecl G_EntLinkTo(gentity_s *ent, gentity_s *parent, unsigned int tagName);
int __cdecl G_EntLinkToWithOffset(
    gentity_s *ent,
    gentity_s *parent,
    unsigned int tagName,
    float *originOffset,
    const float *anglesOffset);
int __cdecl G_EntAttach(gentity_s *ent, const char *modelName, unsigned int tagName, int ignoreCollision);


extern bool g_godModeRemoteInputValid;
