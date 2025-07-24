#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <bgame/bg_public.h>

#include <bgame/bg_actor_prone.h>
#include <script/scr_variable.h>

#include <bgame/bg_local.h>
#include "actor_animapi.h"

struct scr_data_t_tag
{
    scr_animtree_t tree;
};

struct BuiltinFunctionDef
{
    const char *actionString;
    void(__fastcall *actionFunc)();
    int type;
};

struct corpseInfo_t
{
    XAnimTree_s *tree;
    int entnum;
    actor_prone_info_s proneInfo;
};

struct actorBackup_s
{
    ai_animmode_t eAnimMode;
    ai_animmode_t eDesiredAnimMode;
    ai_animmode_t eScriptSetAnimMode;
    bool bUseGoalWeight;
    actor_physics_t Physics;
    ai_orient_t ScriptOrient;
    ai_orient_t CodeOrient;
    float fDesiredBodyYaw;
    float currentOrigin[3];
    float currentAngles[3];
    float vLookForward[3];
    float vLookRight[3];
    float vLookUp[3];
};

struct scr_data_t
{
    int levelscript;
    int scripted_init;
    scr_data_t_tag generic_human;
    AnimScriptList anim;
    AnimScriptList dogAnim;
    int delete_;
    int initstructs;
    int createstruct;
    XAnimTree_s *actorXAnimTrees[32];
    corpseInfo_t actorCorpseInfo[16];
    XAnimTree_s *actorBackupXAnimTree;
    actorBackup_s *actorBackup;
    XAnimTree_s *actorXAnimClientTrees[64];
    bool actorXAnimClientTreesInuse[64];
    int actorFreeClientTree;
};

struct ScriptFunctions
{
    int maxSize;
    int count;
    int *address;
};

struct cached_tag_mat_t
{
    int time;
    int entnum;
    unsigned __int16 name;
    float tagMat[4][3];
};

unsigned int __cdecl GScr_AllocString(const char *s);
void __cdecl TRACK_g_scr_main();
void __cdecl Scr_LoadLevel();
int __cdecl GScr_SetScriptAndLabel(
    ScriptFunctions *functions,
    const char *filename,
    const char *label,
    int bEnforceExists);
void __cdecl GScr_SetLevelScript(ScriptFunctions *functions);
void *__cdecl GScr_AnimscriptAlloc(int size);
void __cdecl GScr_SetScriptsForPathNode(pathnode_t *loadNode, ScriptFunctions *data);
void __cdecl GScr_SetScriptsForPathNodes(ScriptFunctions *functions);
scr_animtree_t __cdecl GScr_FindAnimTree(const char *filename, int bEnforceExists);
void __cdecl GScr_FindAnimTrees(int a1, const char *a2);
void __cdecl GScr_SetSingleAnimScript(ScriptFunctions *functions, scr_animscript_t *pAnim, const char *name);
void __cdecl GScr_SetAnimScripts(ScriptFunctions *functions);
void __cdecl GScr_SetDogAnimScripts(ScriptFunctions *functions);
void GScr_PostLoadScripts();
void *__cdecl Hunk_AllocXAnimCreate(int size);
void __cdecl GScr_FreeScripts();
gentity_s *__cdecl GetEntity(scr_entref_t *entref);
gentity_s *__cdecl GetPlayerEntity(scr_entref_t *entref);
void GScr_CreatePrintChannel();
void GScr_printChannelSet();
void print();
void println();
// attributes: thunk
void __cdecl Scr_LocalizationError(unsigned int iParm, const char *pszErrorMessage);
void __cdecl Scr_ValidateLocalizedStringRef(unsigned int parmIndex, const char *token, int tokenLen);
int __cdecl Scr_ValidateNonLocalizedStringRef(
    unsigned int parmIndex,
    const char *token,
    int tokenLen,
    const char *errorContext);
unsigned int __cdecl Scr_NonLocalizedStringErrorPrefix(
    unsigned int parmIndex,
    unsigned int tokenLen,
    const char *errorContext,
    unsigned int stringLen,
    unsigned int stringLimit,
    char *string);
int __cdecl Scr_IsValidMessageChar(char key);
void __cdecl Scr_ConstructMessageString(
    int firstParmIndex,
    int lastParmIndex,
    const char *errorContext,
    char *string,
    unsigned int stringLimit);
void __cdecl Scr_MakeGameMessage(const char *cmd);
void __cdecl Scr_VerifyWeaponIndex(int weaponIndex, const char *weaponName);
void iprintln();
void iprintlnbold();
void GScr_print3d();
void GScr_line();
void assertCmd();
void assertexCmd();
void assertmsgCmd();
void GScr_IsDefined();
void GScr_IsString();
void GScr_IsArray();
void GScr_IsAlive();
void GScr_IsPlayer();
void GScr_IsAI();
void GScr_IsSentient();
void GScr_IsGodMode();
void GScr_GetDvar();
void GScr_GetDvarInt();
void GScr_GetDvarFloat();
void GScr_GetDebugDvar();
void GScr_GetDebugDvarInt();
void GScr_GetDebugDvarFloat();
void GScr_SetDvar();
void GScr_SetSavedDvar();
void GScr_GetTime();
void GScr_GetDifficulty();
void Scr_GetEntByNum();
int __cdecl Scr_GetTeamFlag(const char *pszTeamName, const char *pszCaller);
int __cdecl Scr_GetTeamFlags(unsigned int i, const char *pszCaller);
int __cdecl Scr_GetSpecies(unsigned __int16 speciesString);
actor_s *Scr_GetAIArray();
actor_s *Scr_GetAISpeciesArray();
void Scr_GetSpawnerArray();
void Scr_GetSpawnerTeamArray();
void Scr_GetWeaponModel();
void Scr_GetWeaponClipModel();
void __cdecl GScr_GetAnimLength(int a1, int a2, XAnimTree_s *a3);
void __cdecl GScr_AnimHasNotetrack(int a1, int a2, XAnimTree_s *a3);
void __cdecl GScr_GetNotetrackTimes(int a1, int a2, XAnimTree_s *a3);
void GScr_GetBrushModelCenter();
void GScr_GetKeyBinding();
void GScr_GetCommandFromKey();
void GScr_Spawn();
void GScr_SpawnVehicle();
void GScr_SpawnTurret();
void GScr_CanSpawnTurret();
int GScr_PrecacheTurret();
void __cdecl ScrCmd_startIgnoringSpotLight(scr_entref_t *entref);
void __cdecl ScrCmd_stopIgnoringSpotLight(scr_entref_t *entref);
void __cdecl ScrCmd_attach(scr_entref_t *entref);
void __cdecl ScrCmd_detach(scr_entref_t *entref);
void __cdecl ScrCmd_detachAll(scr_entref_t *entref);
void __cdecl ScrCmd_GetAttachSize(scr_entref_t *entref);
void __cdecl ScrCmd_GetAttachModelName(scr_entref_t *entref);
void __cdecl ScrCmd_GetAttachTagName(scr_entref_t *entref);
void __cdecl ScrCmd_GetAttachIgnoreCollision(scr_entref_t *entref);
void __cdecl ScrCmd_hidepart(scr_entref_t *entref);
void __cdecl ScrCmd_showpart(scr_entref_t *entref);
void __cdecl ScrCmd_showallparts(scr_entref_t *entref);
void __cdecl ScrCmd_LinkTo(scr_entref_t *entref);
void __cdecl ScrCmd_SetMoveSpeedScale(scr_entref_t *entref);
void ScrCmd_GetTimeScale();
void ScrCmd_SetTimeScale();
void ScrCmd_SetPhysicsGravityDir();
void __cdecl ScrCmd_PlayerSetGroundReferenceEnt(scr_entref_t *entref);
void __cdecl ScrCmd_PlayerLinkTo(scr_entref_t *entref);
void __cdecl ScrCmd_PlayerLinkToDelta(scr_entref_t *entref);
void __cdecl ScrCmd_PlayerLinkToAbsolute(scr_entref_t *entref);
void __cdecl ScrCmd_Unlink(scr_entref_t *entref);
void __cdecl ScrCmd_EnableLinkTo(scr_entref_t *entref);
void __cdecl ScrCmd_DontInterpolate(scr_entref_t *entref);
void __cdecl ScrCmd_dospawn(scr_entref_t *entref);
void __cdecl ScrCmd_StalingradSpawn(scr_entref_t *entref);
void __cdecl ScrCmd_GetOrigin(scr_entref_t *entref);
void __cdecl ScrCmd_GetCentroid(scr_entref_t *entref);
void __cdecl ScrCmd_GetStance(scr_entref_t *entref);
void __cdecl ScrCmd_SetStance(scr_entref_t *entref);
void __cdecl ScrCmd_ItemWeaponSetAmmo(scr_entref_t *entref);
void __cdecl ScrCmd_MagicGrenade(scr_entref_t *entref);
void __cdecl ScrCmd_MagicGrenadeManual(scr_entref_t *entref);
void __cdecl Scr_BulletSpread();
void __cdecl Scr_BulletTracer();
void __cdecl Scr_MagicBullet();
void __cdecl GScr_IsFiringTurret(scr_entref_t *entref);
void __cdecl GScr_SetFriendlyChain(scr_entref_t *entref);
int __cdecl GScr_UpdateTagInternal(
    gentity_s *ent,
    unsigned int tagName,
    cached_tag_mat_t *cachedTag,
    int showScriptError);
void __cdecl GScr_GetTagOrigin(scr_entref_t *entref);
void __cdecl GScr_GetTagAngles(scr_entref_t *entref);
void __cdecl ScrCmd_GetEye(scr_entref_t *entref);
void __cdecl ScrCmd_GetDebugEye(scr_entref_t *entref);
void __cdecl ScrCmd_UseBy(scr_entref_t *entref);
void __cdecl ScrCmd_IsTouching(scr_entref_t *entref);
void __cdecl ParsePlaySoundCmd(scr_entref_t *entref, int event, int notifyevent);
void ScrCmd_SoundExists();
void __cdecl ScrCmd_PlaySound(scr_entref_t *entref);
void __cdecl ScrCmd_PlaySoundAsMaster(scr_entref_t *entref);
void __cdecl ScrCmd_PlayLoopSound(scr_entref_t *entref);
void __cdecl ScrCmd_StopLoopSound(scr_entref_t *entref);
void __cdecl ScrCmd_StopSounds(scr_entref_t *entref);
void __cdecl ScrCmd_EqOn(scr_entref_t *entref);
void __cdecl ScrCmd_EqOff(scr_entref_t *entref);
void __cdecl ScrCmd_HasEq(scr_entref_t *entref);
void __cdecl ScrCmd_IsWaitingOnSound(scr_entref_t *entref);
void __cdecl ScrCmd_Delete(scr_entref_t *entref);
void __cdecl ScrCmd_SetModel(scr_entref_t *entref);
void __cdecl ScrCmd_SetShadowHint(scr_entref_t *entref);
// local variable allocation has failed, the output may be wrong!
void __cdecl ScrCmd_GetNormalHealth(scr_entref_t *entref);
void __cdecl ScrCmd_SetNormalHealth(scr_entref_t *entref);
void __cdecl ScrCmd_DoDamage(scr_entref_t *entref);
void __cdecl ScrCmd_Show(scr_entref_t *entref);
void __cdecl ScrCmd_Hide(scr_entref_t *entref);
void __cdecl ScrCmd_LaserOn(scr_entref_t *entref);
void __cdecl ScrCmd_LaserOff(scr_entref_t *entref);
void __cdecl ScrCmd_SetContents(scr_entref_t *entref);
void __cdecl Scr_SetStableMissile(scr_entref_t *entref);
void __cdecl GScr_DisconnectPaths(scr_entref_t *entref);
void __cdecl GScr_ConnectPaths(scr_entref_t *entref);
void __cdecl GScr_StartFiring(scr_entref_t *entref);
void __cdecl GScr_StopFiring(scr_entref_t *entref);
void __cdecl GScr_ShootTurret(scr_entref_t *entref);
void __cdecl GScr_SetMode(scr_entref_t *entref);
void __cdecl GScr_GetTurretOwner(scr_entref_t *entref);
void __cdecl GScr_SetTargetEntity(scr_entref_t *entref);
void __cdecl GScr_SetAiSpread(scr_entref_t *entref);
void __cdecl GScr_SetPlayerSpread(scr_entref_t *entref);
void __cdecl GScr_SetConvergenceTime(scr_entref_t *entref);
void __cdecl GScr_SetSuppressionTime(scr_entref_t *entref);
void __cdecl GScr_ClearTargetEntity(scr_entref_t *entref);
void __cdecl GScr_SetTurretTeam(scr_entref_t *entref);
void __cdecl GScr_SetTurretIgnoreGoals(scr_entref_t *entref);
void __cdecl GScr_MakeTurretUsable(scr_entref_t *entref);
void __cdecl GScr_MakeTurretUnusable(scr_entref_t *entref);
void __cdecl GScr_SetTurretAccuracy(scr_entref_t *entref);
void __cdecl GScr_GetTurretTarget(scr_entref_t *entref);
void __cdecl GScr_SetCursorHint(scr_entref_t *entref);
int __cdecl G_GetHintStringIndex(int *piIndex, const char *pszString);
void __cdecl GScr_SetHintString(scr_entref_t *entref);
void __cdecl GScr_UseTriggerRequireLookAt(scr_entref_t *entref);
void __cdecl G_InitObjectives();
int __cdecl PrintObjectiveUpdate(unsigned int state, const char *objectiveDesc);
int __cdecl ObjectiveStateIndexFromString(int *piStateIndex, unsigned int stateString);
void __cdecl SetObjectiveIconIntoConfigString(char *objConfigString, unsigned int paramNum);
int Scr_Objective_Add();
void Scr_Objective_Delete();
int Scr_Objective_State();
void __cdecl Scr_Objective_String_Internal(int makeUpdateMessage);
void Scr_Objective_String();
void Scr_Objective_String_NoMessage();
void Scr_Objective_Icon();
void Scr_Objective_Position();
void Scr_Objective_AdditionalPosition();
int Scr_Objective_Current();
int Scr_Objective_AdditionalCurrent();
void Scr_Objective_Ring();
void Scr_BulletTrace();
void Scr_BulletTracePassed();
void __cdecl Scr_SightTracePassed();
void Scr_PhysicsTrace();
void Scr_PlayerPhysicsTrace();
void Scr_RandomInt();
void Scr_RandomFloat();
void Scr_RandomIntRange();
void Scr_RandomFloatRange();
void GScr_sin();
void GScr_cos();
void GScr_tan();
void GScr_asin();
void GScr_acos();
void GScr_atan();
void GScr_CastInt();
void GScr_abs();
void GScr_min();
void GScr_max();
void GScr_floor();
void GScr_ceil();
void GScr_sqrt();
void GScr_VectorFromLineToPoint();
void GScr_PointOnSegmentNearestToPoint();
void Scr_Distance();
void Scr_Distance2D();
void Scr_DistanceSquared();
void Scr_Length();
void Scr_LengthSquared();
void Scr_Closer();
void Scr_VectorDot();
void Scr_VectorNormalize();
void Scr_VectorToAngles();
void Scr_VectorLerp();
void Scr_AnglesToUp();
void Scr_AnglesToRight();
void Scr_AnglesToForward();
void Scr_CombineAngles();
void Scr_IsSubStr();
void Scr_GetSubStr();
void Scr_ToLower();
void Scr_StrTok();
void Scr_SetBlur();
void Scr_MusicPlay();
void Scr_MusicStop();
void Scr_SoundFade();
void Scr_Amplify();
void Scr_AmplifyStop();
void __cdecl Scr_ErrorOnDefaultAsset(XAssetHeader *type, const char *assetName, const char *a3);
int Scr_PrecacheModel();
void Scr_PrecacheShellShock();
void Scr_PrecacheItem();
int Scr_PrecacheMaterial();
void Scr_PrecacheString();
int Scr_PrecacheRumble();
void GScr_PrecacheLocationSelector();
void Scr_PrecacheNightvisionCodeAssets();
int __cdecl GScr_GetLocSelIndex(const char *mtlName);
void Scr_AmbientPlay();
void Scr_AmbientStop();
void __cdecl Scr_CheckForSaveErrors(int saveId);
void Scr_SaveGame();
void Scr_SaveGameNoCommit();
void Scr_IsSaveSuccessful();
void Scr_IsRecentlyLoaded();
void Scr_CommitSave();
void __cdecl GScr_RadiusDamageInternal(gentity_s *inflictor);
void GScr_RadiusDamage();
void __cdecl GScr_EntityRadiusDamage(scr_entref_t *entref);
void __cdecl GScr_Detonate(scr_entref_t *entref);
int GScr_SetPlayerIgnoreRadiusDamage();
void __cdecl GScr_DamageConeTraceInternal(scr_entref_t *entref, int contentMask);
void __cdecl GScr_DamageConeTrace(scr_entref_t *entref);
void __cdecl GScr_SightConeTrace(scr_entref_t *entref);
void GScr_ChangeLevel();
void GScr_MissionSuccess();
void GScr_MissionFailed();
void __cdecl GScr_SetMissionDvar();
void GScr_Cinematic();
void GScr_CinematicInGame();
void GScr_CinematicInGameSync();
void GScr_CinematicInGameLoop();
void GScr_CinematicInGameLoopResident();
void GScr_CinematicInGameLoopFromFastfile();
void GScr_StopCinematicInGame();
void GScr_Earthquake();
int GScr_DrawCompassFriendlies();
void GScr_WeaponFireTime();
void GScr_WeaponClipSize();
void __cdecl GScr_GetAmmoCount(scr_entref_t *entref);
void GScr_WeaponIsSemiAuto();
void GScr_WeaponIsBoltAction();
void GScr_WeaponType();
void GScr_WeaponClass();
void GScr_WeaponInventoryType();
void GScr_WeaponStartAmmo();
void GScr_WeaponMaxAmmo();
void GScr_WeaponAltWeaponName();
WeaponDef *__cdecl GScr_GetWeaponDef();
void GScr_WeaponFightDist();
void GScr_WeaponMaxDist();
void GScr_IsTurretActive();
void GScr_IsWeaponClipOnly();
void GScr_IsWeaponDetonationTimed();
void GScr_GetMoveDelta();
void GScr_GetAngleDelta();
void GScr_GetNorthYaw();
void __cdecl Scr_SetFxAngles(unsigned int givenAxisCount, float (*axis)[3], float *angles);
void Scr_LoadFX();
void __cdecl Scr_FxParamError(unsigned int paramIndex, const char *errorString, int fxId);
void Scr_PlayFX();
void Scr_PlayFXOnTag();
void Scr_PlayLoopedFX();
void Scr_SpawnFX();
void Scr_TriggerFX();
void Scr_GetFXVis();
void Scr_PhysicsExplosionSphere();
void Scr_PhysicsRadiusJolt();
void Scr_PhysicsRadiusJitter();
void Scr_PhysicsExplosionCylinder();
void __cdecl Scr_SetFog(const char *cmd, double start, double density, double r, double g, double b, double time);
void Scr_SetExponentialFog();
void Scr_VisionSetNaked();
void Scr_VisionSetNight();
void Scr_SetCullDist();
void Scr_GetMapSunLight();
void Scr_SetSunLight();
void Scr_ResetSunLight();
void Scr_GetMapSunDirection();
void Scr_SetSunDirection();
void Scr_LerpSunDirection();
void Scr_ResetSunDirection();
void Scr_BadPlace_Delete();
void Scr_BadPlace_Cylinder();
void Scr_BadPlace_Arc();
void Scr_BadPlace_Brush();
void Scr_ClearAllCorpses();
void __cdecl GScr_GetNumParts();
void __cdecl GScr_GetPartName();
XAnimTree_s *__cdecl GScr_GetEntAnimTree(gentity_s *ent);
void __cdecl G_FlagAnimForUpdate(gentity_s *ent);
void __cdecl Scr_AnimRelative(scr_entref_t *entref);
void __cdecl ScrCmd_animrelative(scr_entref_t *entref);
void __cdecl DumpAnimCommand(
    const char *funcName,
    XAnimTree_s *tree,
    unsigned int anim,
    int root,
    double weight,
    double time,
    double rate);
void __cdecl GScr_ClearAnim(scr_entref_t *entref);
void __cdecl GScr_HandleAnimError(int error);
void __cdecl GScr_SetAnimKnobInternal(scr_entref_t *entref, unsigned int flags);
void __cdecl GScr_SetAnimKnob(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobLimited(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobRestart(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobLimitedRestart(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobAllInternal(scr_entref_t *entref, unsigned int flags);
void __cdecl GScr_SetAnimKnobAll(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobAllLimited(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobAllRestart(scr_entref_t *entref);
void __cdecl GScr_SetAnimKnobAllLimitedRestart(scr_entref_t *entref);
void __cdecl GScr_SetAnimInternal(scr_entref_t *entref, unsigned int flags);
void __cdecl GScr_SetAnim(scr_entref_t *entref);
void __cdecl GScr_SetAnimLimited(scr_entref_t *entref);
void __cdecl GScr_SetAnimRestart(scr_entref_t *entref);
void __cdecl GScr_SetAnimLimitedRestart(scr_entref_t *entref);
void __cdecl GScr_GetAnimTime(scr_entref_t *entref);
void __cdecl GScr_GetAnimAssetType(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimKnobInternal(scr_entref_t *entref, unsigned int flags);
void __cdecl GScr_SetFlaggedAnimKnob(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimKnobLimited(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimKnobRestart(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimKnobLimitedRestart(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimKnobAllInternal(scr_entref_t *entref, unsigned int flags, const char *usage);
void __cdecl GScr_SetFlaggedAnimKnobAll(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimKnobAllRestart(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimInternal(scr_entref_t *entref, unsigned int flags);
void __cdecl GScr_SetFlaggedAnim(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimLimited(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimRestart(scr_entref_t *entref);
void __cdecl GScr_SetFlaggedAnimLimitedRestart(scr_entref_t *entref);
void __cdecl GScr_SetAnimTime(scr_entref_t *entref);
void __cdecl GScr_DumpAnims(scr_entref_t *entref);
void __cdecl GScr_ShellShock(scr_entref_t *entref);
void __cdecl GScr_StopShellShock(scr_entref_t *entref);
void __cdecl GScr_SetDepthOfField(scr_entref_t *entref);
void __cdecl GScr_SetViewModelDepthOfField(scr_entref_t *entref);
void __cdecl GScr_ViewKick(scr_entref_t *entref);
void __cdecl GScr_GetEntnum(scr_entref_t *entref);
void __cdecl GScr_ValidateLightVis(int eType);
void __cdecl GScr_LockLightVis(scr_entref_t *entref);
void __cdecl GScr_UnlockLightVis(scr_entref_t *entref);
void __cdecl GScr_Launch(scr_entref_t *entref);
void __cdecl GScr_SetSoundBlend(scr_entref_t *entref);
void __cdecl GScr_LocalToWorldCoords(scr_entref_t *entref);
void __cdecl GScr_GetEntityNumber(scr_entref_t *entref);
void __cdecl GScr_EnableGrenadeTouchDamage(scr_entref_t *entref);
void __cdecl GScr_DisableGrenadeTouchDamage(scr_entref_t *entref);
void __cdecl GScr_MissileSetTarget(scr_entref_t *entref);
void __cdecl GScr_EnableAimAssist(scr_entref_t *entref);
void __cdecl GScr_DisableAimAssist(scr_entref_t *entref);
void __cdecl GScr_MakeFakeAI(scr_entref_t *entref);
void __cdecl GScr_SetLookatText(scr_entref_t *entref);
void __cdecl GScr_SetRightArc(scr_entref_t *entref);
void __cdecl GScr_SetLeftArc(scr_entref_t *entref);
void __cdecl GScr_SetTopArc(scr_entref_t *entref);
void __cdecl GScr_SetBottomArc(scr_entref_t *entref);
void __cdecl GScr_SetDefaultDropPitch(scr_entref_t *entref);
void __cdecl GScr_RestoreDefaultDropPitch(scr_entref_t *entref);
void __cdecl GScr_TurretFireDisable(scr_entref_t *entref);
void __cdecl GScr_TurretFireEnable(scr_entref_t *entref);
void __cdecl GScr_SetSpawnerTeam(scr_entref_t *entref);
void GScr_PrecacheMenu();
int __cdecl GScr_GetScriptMenuIndex(const char *pszMenu);
void GScr_SetDebugOrigin();
void GScr_SetDebugAngles();
void GScr_OpenFile();
void GScr_CloseFile();
void __cdecl Scr_FPrint_internal(bool commaBetweenFields);
void GScr_FPrintln();
void GScr_FPrintFields();
void GScr_FReadLn();
void GScr_FGetArg();
void __cdecl Scr_PlayRumbleOnPosition_Internal(int event);
gentity_s *Scr_PlayRumbleOnPosition();
gentity_s *Scr_PlayRumbleLoopOnPosition();
gentity_s *Scr_StopAllRumbles();
void ScrCmd_GiveAchievement();
void ScrCmd_UpdateGamerProfile();
void GScr_SetMiniMap();
void GScr_GetArrayKeys();
void GScr_ClearLocalizedStrings();
void Scr_TableLookup();
void Scr_TableLookupIString();
void __cdecl Scr_GetReflectionLocs();
void Scr_RefreshHudCompass();
void Scr_RefreshHudAmmoCounter();
void Scr_LogString();
void __cdecl ScrCmd_LogString(scr_entref_t *entref);
void(__cdecl *__cdecl BuiltIn_GetFunction(const char **pName, int *type))();
void(__cdecl *__cdecl Scr_GetFunction(const char **pName, int *type))();
void __cdecl ScrCmd_PlayRumbleOnEntity_Internal(scr_entref_t *entref, int event);
void __cdecl ScrCmd_PlayRumbleOnEntity(scr_entref_t *entref);
void __cdecl ScrCmd_PlayRumbleLoopOnEntity(scr_entref_t *entref);
void __cdecl ScrCmd_StopRumble(scr_entref_t *entref);
void __cdecl ScrCmd_AddAIEventListener(scr_entref_t *entref);
void __cdecl ScrCmd_RemoveAIEventListener(scr_entref_t *entref);
gentity_s *__cdecl GScr_SetupLightEntity(scr_entref_t *entref);
void __cdecl GScr_GetLightColor(scr_entref_t *entref);
void __cdecl GScr_SetLightColor(scr_entref_t *entref);
void __cdecl GScr_GetLightIntensity(scr_entref_t *entref);
void __cdecl GScr_SetLightIntensity(scr_entref_t *entref);
void __cdecl GScr_GetLightRadius(scr_entref_t *entref);
void __cdecl GScr_SetLightRadius(scr_entref_t *entref);
void __cdecl GScr_GetLightFovInner(scr_entref_t *entref);
void __cdecl GScr_GetLightFovOuter(scr_entref_t *entref);
void __cdecl GScr_SetLightFovRange(scr_entref_t *entref);
void __cdecl GScr_GetLightExponent(scr_entref_t *entref);
void __cdecl GScr_SetLightExponent(scr_entref_t *entref);
void __cdecl GScr_StartRagdoll(scr_entref_t *entref);
void __cdecl GScr_IsRagdoll(scr_entref_t *entref);
void(__cdecl *__cdecl BuiltIn_GetMethod(const char **pName, int *type))(scr_entref_t);
void(__cdecl *__cdecl Scr_GetMethod(const char **pName, int *type))(scr_entref_t *__struct_ptr);
void __cdecl Scr_SetOrigin(gentity_s *ent, int offset);
void __cdecl Scr_SetAngles(gentity_s *ent, int offset);
void __cdecl Scr_SetHealth(gentity_s *ent, int offset);
void __cdecl GScr_Shutdown();
void __cdecl GScr_SetScriptsAndAnimsForEntities(ScriptFunctions *functions);
void __cdecl GScr_SetScripts(ScriptFunctions *functions);
void __cdecl ScrCmd_GetShootAtPosition(scr_entref_t *entref);
void __cdecl ScrCmd_animscriptedInternal(scr_entref_t *entref, int bDelayForActor);
void __cdecl G_StopAnimScripted(gentity_s *ent);
void __cdecl ScrCmd_stopanimscripted(scr_entref_t *entref);
void __cdecl ScrCmd_animscripted(scr_entref_t *entref);
void __cdecl G_SetAnimTree(gentity_s *ent, scr_animtree_t *animtree);
void __cdecl GScr_UseAnimTree(scr_entref_t *entref);
void __cdecl GScr_StopUseAnimTree(scr_entref_t *entref);


static const char *nodeStringTable[20] =
{
  "BAD NODE",
  "Path",
  "Cover Stand",
  "Cover Crouch",
  "Cover Crouch Window",
  "Cover Prone",
  "Cover Right",
  "Cover Left",
  "Cover Wide Right",
  "Cover Wide Left",
  "Conceal Stand",
  "Conceal Crouch",
  "Conceal Prone",
  "Reacquire",
  "Balcony",
  "Scripted",
  "Begin",
  "End",
  "Turret",
  "Guard"
};


extern scr_data_t g_scr_data;