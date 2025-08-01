#pragma once
#include <client/client.h>
#include <bgame/bg_local.h>
#include "cg_local.h"

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif
#include <gfx_d3d/r_reflection_probe.h>

struct rectDef_s;
struct windowDef_t;
struct dvar_s;
struct playerState_s;
struct snd_alias_list_t;
struct PhysPreset;
struct SndEntHandle;

struct cgs_t
{
    int viewX;
    int viewY;
    int viewWidth;
    int viewHeight;
    float viewAspect;
    char mapname[64];
    bool started;
    const FxEffectDef *fxs[100];
    shellshock_parms_t holdBreathParams;
    float compassWidth;
    float compassHeight;
};

struct objectiveInfo_t
{
    objectiveState_t state;
    float origin[8][3];
    char string[1024];
    int ringTime;
    int ringToggle;
    int icon;
};

struct playerEntity_t
{
    float fLastWeaponPosFrac;
    int bPositionToADS;
    float fOOPositionBlendTime;
    float vPositionBlendOrg[3];
    float vPositionBlendAng[3];
    float vPositionLastOrg[3];
    float vPositionLastAng[3];
    float fLastIdleFactor;
    float vLastMoveOrg[3];
    float vLastMoveAng[3];
};

struct targetInfo_t
{
    int entNum;
    float offset[3];
    int materialIndex;
    int offscreenMaterialIndex;
    int flags;
};

struct cg_s_shock
{
    int time;
    int duration;
};

struct cg_s
{
    int clientNum;
    int localClientNum;
    DemoType demoType;
    CubemapShot cubemapShot;
    int cubemapSize;
    int serverCommandSequence;
    int latestSnapshotNum;
    int latestSnapshotTime;
    int loaded;
    snapshot_s *snap;
    snapshot_s *nextSnap;
    snapshot_s activeSnapshots[2];
    int createdNextSnap;
    float frameInterpolation;
    int frametime;
    int animFrametime;
    int time;
    int oldTime;
    int physicsTime;
    playerState_s predictedPlayerState;
    centity_s predictedPlayerEntity;
    playerEntity_t playerEntity;
    int validPPS;
    int predictedErrorTime;
    float predictedError[3];
    float landChange;
    int landTime;
    float heightToCeiling;
    refdef_s refdef;
    float refdefViewAngles[3];
    float lastVieworg[3];
    float swayViewAngles[3];
    float swayAngles[3];
    float swayOffset[3];
    float zoomSensitivity;
    int vehicleInitView;
    float prevVehicleInvAxis[3][3];
    bool vehicleViewLocked;
    float vehicleViewLockedAngles[3];
    int showScores;
    int scoreFadeTime;
    int timeScaleTimeStart;
    int timeScaleTimeEnd;
    float timeScaleStart;
    float timeScaleEnd;
    int deadquoteStartTime;
    int cursorHintIcon;
    int cursorHintTime;
    int cursorHintFade;
    int cursorHintString;
    int lastClipFlashTime;
    InvalidCmdHintType invalidCmdHintType;
    int invalidCmdHintTime;
    int lastHealthPulseTime;
    int lastHealthLerpDelay;
    int lastHealthClient;
    float lastHealth;
    float healthOverlayFromAlpha;
    float healthOverlayToAlpha;
    int healthOverlayPulseTime;
    int healthOverlayPulseDuration;
    int healthOverlayPulsePhase;
    bool healthOverlayHurt;
    int proneBlockedEndTime;
    int lastStance;
    int lastStanceChangeTime;
    int lastStanceFlashTime;
    int voiceTime;
    unsigned int weaponSelect;
    int weaponSelectTime;
    unsigned int weaponLatestPrimaryIdx;
    int prevViewmodelWeapon;
    int equippedOffHand;
    viewDamage_t viewDamage[8];
    int damageTime;
    float damageX;
    float damageY;
    float damageValue;
    int weapIdleTime;
    int nomarks;
    int v_dmg_time;
    float v_dmg_pitch;
    float v_dmg_roll;
    float fBobCycle;
    float xyspeed;
    float kickAVel[3];
    float kickAngles[3];
    float gunPitch;
    float gunYaw;
    float gunXOfs;
    float gunYOfs;
    float gunZOfs;
    float vGunOffset[3];
    float vGunSpeed[3];
    float vAngOfs[3];
    float viewModelAxis[4][3];
    bool hideViewModel;
    float rumbleScale;
    float selectedLocation[2];
    float compassNorthYaw;
    float compassNorth[2];
    Material *compassMapMaterial;
    float compassMapUpperLeft[2];
    float compassMapWorldSize[2];
    int compassFadeTime;
    int healthFadeTime;
    int ammoFadeTime;
    int stanceFadeTime;
    int sprintFadeTime;
    int offhandFadeTime;
    int offhandFlashTime;
    objectiveInfo_t objectives[16];
    targetInfo_t targets[32];
    shellshock_t shellshock;
    cg_s_shock testShock;
    int holdBreathTime;
    int holdBreathInTime;
    int holdBreathDelay;
    float holdBreathFrac;
    int bloodLastTime;
    float vehReticleOffset[2];
    float vehReticleVel[2];
    int vehReticleLockOnStartTime;
    int vehReticleLockOnDuration;
    int vehReticleLockOnEntNum;
    cpose_t viewModelPose;
    visionSetVars_t visionSetPreLoaded[4];
    char visionSetPreLoadedName[4][64];
    visionSetVars_t visionSetFrom[2];
    visionSetVars_t visionSetTo[2];
    visionSetVars_t visionSetCurrent[2];
    visionSetLerpData_t visionSetLerpData[2];
    char visionNameNaked[64];
    char visionNameNight[64];
    int extraButtons;
    int lastActionSlotTime;
    bool playerTeleported;
    int stepViewStart;
    float stepViewChange;
    float zNear;
    hudElemSoundInfo_t hudElemSound[32];
};

struct cgMedia_t
{
    Material *whiteMaterial;
    Material *friendlyFireMaterial;
    Material *tracerMaterial;
    Material *laserMaterial;
    Material *laserLightMaterial;
    Material *hintMaterials[133];
    Material *stanceMaterials[4];
    Material *objectiveMaterials[2];
    Material *damageMaterial;
    Material *mantleHint;
    Font_s *smallDevFont;
    Font_s *bigDevFont;
    snd_alias_list_t *landDmgSound;
    snd_alias_list_t *grenadeExplodeSound[29];
    snd_alias_list_t *rocketExplodeSound[29];
    snd_alias_list_t *bulletHitSmallSound[29];
    snd_alias_list_t *bulletHitLargeSound[29];
    snd_alias_list_t *bulletHitAPSound[29];
    snd_alias_list_t *shotgunHitSound[29];
    snd_alias_list_t *bulletExitSmallSound[29];
    snd_alias_list_t *bulletExitLargeSound[29];
    snd_alias_list_t *bulletExitAPSound[29];
    snd_alias_list_t *shotgunExitSound[29];
    snd_alias_list_t *stepSprintSound[29];
    snd_alias_list_t *stepSprintSoundPlayer[29];
    snd_alias_list_t *stepRunSound[29];
    snd_alias_list_t *stepRunSoundPlayer[29];
    snd_alias_list_t *stepWalkSound[29];
    snd_alias_list_t *stepWalkSoundPlayer[29];
    snd_alias_list_t *stepProneSound[29];
    snd_alias_list_t *stepProneSoundPlayer[29];
    snd_alias_list_t *landSound[29];
    snd_alias_list_t *landSoundPlayer[29];
    snd_alias_list_t *sprintingEquipmentSound;
    snd_alias_list_t *sprintingEquipmentSoundPlayer;
    snd_alias_list_t *runningEquipmentSound;
    snd_alias_list_t *runningEquipmentSoundPlayer;
    snd_alias_list_t *walkingEquipmentSound;
    snd_alias_list_t *walkingEquipmentSoundPlayer;
    snd_alias_list_t *foliageMovement;
    snd_alias_list_t *bulletWhizby;
    snd_alias_list_t *meleeHit;
    snd_alias_list_t *meleeHitOther;
    snd_alias_list_t *meleeKnifeHit;
    snd_alias_list_t *meleeKnifeHitOther;
    snd_alias_list_t *nightVisionOn;
    snd_alias_list_t *nightVisionOff;
    snd_alias_list_t *playerHeartBeatSound;
    snd_alias_list_t *playerBreathInSound;
    snd_alias_list_t *playerBreathOutSound;
    snd_alias_list_t *playerBreathGaspSound;
    snd_alias_list_t *playerSwapOffhand;
    snd_alias_list_t *physCollisionSound[50][29];
    Material *checkbox_active;
    Material *checkbox_current;
    Material *checkbox_done;
    Material *checkbox_fail;
    Material *compassping_friendlyfiring;
    Material *compassping_friendlyyelling;
    Material *compassping_enemyfiring;
    Material *compassping_enemyyelling;
    Material *compassping_grenade;
    Material *compassping_explosion;
    Material *grenadeIconFrag;
    Material *grenadeIconFlash;
    Material *grenadeIconThrowBack;
    Material *grenadePointer;
    Material *offscreenObjectivePointer;
    FxImpactTable *fx;
    const FxEffectDef *fxNoBloodFleshHit;
    const FxEffectDef *fxKnifeBlood;
    const FxEffectDef *fxKnifeNoBlood;
    Material *vehCenterCircle;
    Material *vehMovingCircle;
    Material *vehHudLine;
    Material *vehBouncingDiamondReticle;
    Material *compassFovMaterial;
    XModel *nightVisionGoggles;
    Material *nightVisionOverlay;
    Material *hudIconNVG;
    Material *hudDpadArrow;
    Material *ammoCounterBullet;
    Material *ammoCounterBeltBullet;
    Material *ammoCounterRifleBullet;
    Material *ammoCounterRocket;
    Material *ammoCounterShotgunShell;
    Material *textDecodeCharacters;
    Material *textDecodeCharactersGlow;
};

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
void __cdecl CG_StopSoundAlias(const int localClientNum, SndEntHandle entitynum, snd_alias_list_t *aliasList);
void __cdecl CG_StopSoundsOnEnt(const int localClientNum, SndEntHandle entitynum);
void __cdecl CG_StopSoundAliasByName(int localClientNum, SndEntHandle entityNum, const char *aliasName);
void __cdecl CG_StopClientSoundAliasByName(int localClientNum, const char *aliasName);
//void __cdecl CG_SubtitlePrint(int msec, snd_alias_t *alias);
void __cdecl CG_SubtitleSndLengthNotify(int msec, snd_alias_t *lengthNotifyData);
void __cdecl CG_ScriptNotifySndLengthNotify(int msec, void *lengthNotifyData);
void __cdecl CG_AddFXSoundAlias(int localClientNum, const float *origin, snd_alias_list_t *aliasList);
int __cdecl CG_PlaySoundAlias(int localClientNum, SndEntHandle entitynum, const float *origin, snd_alias_list_t *aliasList);
int __cdecl CG_PlaySoundAliasByName(int localClientNum, SndEntHandle entitynum, const float *origin, const char *aliasname);
int __cdecl CG_PlaySoundAliasAsMasterByName(int localClientNum, SndEntHandle entitynum, const float *origin, const char *aliasname);
void __cdecl CG_LoadHudMenu(int localClientNum);
void __cdecl CG_InitViewDimensions(int localClientNum);
const char *__cdecl CG_GetTeamName(team_t team);
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
int __cdecl CG_PlayEntitySoundAlias(int localClientNum, SndEntHandle entitynum, snd_alias_list_t *aliasList);
int __cdecl CG_PlayEntitySoundAliasByName(int localClientNum, SndEntHandle entitynum, const char *aliasname);

extern UiContext cgDC;
extern cgs_t cgsArray[1];
extern cg_s cgArray[1];
extern cgMedia_t cgMedia;

extern weaponInfo_s cg_weaponsArray[1][128];
extern centity_s cg_entitiesArray[1][2176];
extern float cg_entityOriginArray[1][2176][3];

extern const dvar_t *cg_hudGrenadeIconEnabledFlash;
extern const dvar_t *vehHelicopterHeadSwayOnRollVert;
extern const dvar_t *cg_hudGrenadePointerPulseMax;
extern const dvar_t *cg_laserLight;
extern const dvar_t *hostileNameFontColor;
extern const dvar_t *cg_drawVersionY;
extern const dvar_t *turretScopeZoomRate;
extern const dvar_t *turretScopeZoomMax;
extern const dvar_t *cg_drawVersion;
extern const dvar_t *cg_gun_y;
extern const dvar_t *cg_drawBreathHint;
extern const dvar_t *cg_drawFriendlyFireCrosshair;
extern const dvar_t *cg_bloodLimitMsec;
extern const dvar_t *cg_hudGrenadeIconMaxRangeFrag;
extern const dvar_t *cg_drawFPS;
extern const dvar_t *cg_drawVersionX;
extern const dvar_t *vehHelicopterHeadSwayOnRollHorz;
extern const dvar_t *cg_drawHealth;
extern const dvar_t *cg_brass;
extern const dvar_t *replay_time;
extern const dvar_t *cg_weaponHintsCoD1Style;
extern const dvar_t *debugOverlay;
extern const dvar_t *cg_viewZSmoothingMax;
extern const dvar_t *cg_marks_ents_player_only;
extern const dvar_t *cg_drawCrosshair;
extern const dvar_t *hostileNameFontGlowColor;
extern const dvar_t *cg_drawPerformanceWarnings;
extern const dvar_t *cg_laserRadius;
extern const dvar_t *cg_invalidCmdHintDuration;
extern const dvar_t *hud_drawHUD;
extern const dvar_t *cg_mapLocationSelectionCursorSpeed;
extern const dvar_t *turretScopeZoom;
extern const dvar_t *cg_gun_rot_rate;
extern const dvar_t *cg_objectiveListWrapCountStandard;
extern const dvar_t *cg_hudGrenadeIconWidth;
extern const dvar_t *cg_gun_z;
extern const dvar_t *cg_hudDamageIconOffset;
extern const dvar_t *overrideNVGModelWithKnife;
extern const dvar_t *cg_viewZSmoothingMin;
extern const dvar_t *cg_drawTurretCrosshair;
extern const dvar_t *cg_hudDamageIconHeight;
extern const dvar_t *cg_drawShellshock;
extern const dvar_t *cg_gun_move_minspeed;
extern const dvar_t *cg_gun_ofs_u;
extern const dvar_t *cg_subtitles;
extern const dvar_t *cg_hudStanceHintPrints;
extern const dvar_t *vehHelicopterHeadSwayOnPitch;
extern const dvar_t *cg_modPrvMruAnims;
extern const dvar_t *cg_hudDamageIconWidth;
extern const dvar_t *cg_viewVehicleInfluence;
extern const dvar_t *cg_draw2D;
extern const dvar_t *cg_gun_x;
extern const dvar_t *hud_showStance;
extern const dvar_t *cg_crosshairAlpha;
extern const dvar_t *cg_laserLightRadius;
extern const dvar_t *hud_missionFailed;
extern const dvar_t *cg_objectiveListWrapCountWidescreen;
extern const dvar_t *cg_gun_rot_r;
extern const dvar_t *friendlyNameFontSize;
extern const dvar_t *cg_drawGun;
extern const dvar_t *cg_hudGrenadeIconOffset;
extern const dvar_t *cg_modPrvMruModels;
extern const dvar_t *cg_centertime;
extern const dvar_t *cg_hudGrenadePointerPulseFreq;
extern const dvar_t *cg_minicon;
extern const dvar_t *cg_gun_move_rate;
extern const dvar_t *cg_tracerSpeed;
extern const dvar_t *cg_bobWeaponRollAmplitude;
extern const dvar_t *cg_viewKickMax;
extern const dvar_t *cg_laserFlarePct;
extern const dvar_t *cg_invalidCmdHintBlinkInterval;
extern const dvar_t *friendlyNameFontGlowColor;
extern const dvar_t *cg_hudGrenadePointerPivot;
extern const dvar_t *cg_crosshairAlphaMin;
extern const dvar_t *vehHelicopterFreeLookReleaseSpeed;
extern const dvar_t *cg_laserLightEndOffset;
extern const dvar_t *cg_tracerChance;
extern const dvar_t *cg_hudGrenadeIconInScope;
extern const dvar_t *cg_crosshairEnemyColor;
extern const dvar_t *cg_drawRumbleDebug;
extern const dvar_t *cg_laserRangePlayer;
extern const dvar_t *cg_tracerScale;
extern const dvar_t *cg_weaponCycleDelay;
extern const dvar_t *cg_laserRange;
extern const dvar_t *cg_laserForceOn;
extern const dvar_t *cg_gun_rot_y;
extern const dvar_t *cg_gameMessageWidth;
extern const dvar_t *cg_gun_ofs_r;
extern const dvar_t *cg_hudGrenadeIconMaxHeight;
extern const dvar_t *cg_modPrvDrawAxis;
extern const dvar_t *cg_debugEvents;
extern const dvar_t *cg_hudGrenadeIconMaxRangeFlash;
extern const dvar_t *cg_drawPlayerPosInFreeMove;
extern const dvar_t *cg_hudStanceFlash;
extern const dvar_t *cg_drawScriptUsage;
extern const dvar_t *cg_drawHUD;
extern const dvar_t *cg_crosshairDynamic;
extern const dvar_t *cg_hudDamageIconInScope;
extern const dvar_t *cg_gun_move_r;
extern const dvar_t *cg_viewZSmoothingTime;
extern const dvar_t *cg_bobWeaponAmplitude;
extern const dvar_t *cg_footsteps;
extern const dvar_t *cg_gun_move_f;
extern const dvar_t *cg_hudGrenadeIconHeight;
extern const dvar_t *friendlyNameFontObjective;
extern const dvar_t *cg_hudGrenadePointerWidth;
extern const dvar_t *cg_nopredict;
extern const dvar_t *cg_drawMaterial;
extern const dvar_t *cg_r_forceLod;
extern const dvar_t *snd_drawInfo;
extern const dvar_t *cg_debug_overlay_viewport;
extern const dvar_t *cg_small_dev_string_fontscale;
extern const dvar_t *cg_dumpAnims;
extern const dvar_t *cg_gun_rot_p;
extern const dvar_t *cg_viewKickScale;
extern const dvar_t *cg_errorDecay;
extern const dvar_t *cg_subtitleWidthStandard;
extern const dvar_t *friendlyNameFontColor;
extern const dvar_t *sv_znear;
extern const dvar_t *cg_hudGrenadePointerPulseMin;
extern const dvar_t *cg_bobWeaponMax;
extern const dvar_t *cg_tracerScaleMinDist;
extern const dvar_t *cg_laserEndOffset;
extern const dvar_t *cg_drawMantleHint;
extern const dvar_t *cg_gameBoldMessageWidth;
extern const dvar_t *cg_tracerWidth;
extern const dvar_t *cg_bobWeaponLag;
extern const dvar_t *cg_laserLightBodyTweak;
extern const dvar_t *cg_bloodLimit;
extern const dvar_t *cg_drawFPSLabels;
extern const dvar_t *cg_drawpaused;
extern const dvar_t *cg_hudDamageIconTime;
extern const dvar_t *cg_debugInfoCornerOffset;
extern const dvar_t *cg_gun_rot_minspeed;
extern const dvar_t *cg_blood;
extern const dvar_t *cg_showmiss;
extern const dvar_t *cg_fov;
extern const dvar_t *turretScopeZoomMin;
extern const dvar_t *cg_subtitleWidthWidescreen;
extern const dvar_t *cg_developer;
extern const dvar_t *cg_tracerScrewRadius;
extern const dvar_t *cg_hudGrenadePointerHeight;
extern const dvar_t *cg_gun_move_u;
extern const dvar_t *cg_hintFadeTime;
extern const dvar_t *cg_subtitleMinTime;
extern const dvar_t *cg_tracerScaleDistRange;
extern const dvar_t *cg_paused;
extern const dvar_t *cg_cursorHints;
extern const dvar_t *cg_gun_ofs_f;
extern const dvar_t *vehHelicopterHeadSwayOnYaw;
extern const dvar_t *cg_laserLightBeginOffset;
extern const dvar_t *cg_tracerScrewDist;
extern const dvar_t *cg_tracerLength;
extern const dvar_t *cg_cinematicFullscreen;

#define CS_EFFECT_NAMES 211


inline cgs_t *CG_GetLocalClientStaticGlobals(int localClientNum)
{
    iassert(localClientNum == 0);

    return &cgsArray[localClientNum];
}

inline centity_s *CG_GetEntity(int32_t localClientNum, uint32_t entityIndex)
{
    iassert(localClientNum == 0);
    bcassert(entityIndex, MAX_GENTITIES);

    return &cg_entitiesArray[localClientNum][entityIndex];
}

inline cg_s *CG_GetLocalClientGlobals(int32_t localClientNum)
{
    iassert(localClientNum == 0);

    return &cgArray[localClientNum];
}

inline weaponInfo_s *__cdecl CG_GetLocalClientWeaponInfo(int localClientNum, int weaponIndex)
{
    iassert(localClientNum == 0);

    return &cg_weaponsArray[localClientNum][weaponIndex];
}