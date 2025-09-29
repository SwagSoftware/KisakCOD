#pragma once

#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_public_mp.h"

#include <cgame/cg_local.h>
#include <bgame/bg_local.h>

#include <gfx_d3d/r_gfx.h>
#include <gfx_d3d/r_reflection_probe.h>
#include <ui_mp/ui_mp.h>
#include <client_mp/client_mp.h>

#include <sound/snd_public.h>
#include <script/scr_const.h>

#define MAX_CLIENT_CORPSES 8

#define STATIC_MAX_LOCAL_CLIENTS 1
#define MAX_LOCAL_CLIENTS 1

#define MAX_COMPASS_ACTORS 64

struct snapshot_s // sizeof=0x2307C
{                                       // ...
    int32_t snapFlags;
    int32_t ping;
    int32_t serverTime;
    playerState_s ps;
    int32_t numEntities;
    int32_t numClients;
    entityState_s entities[512];
    clientState_s clients[64];
    int32_t serverCommandSequence;
};

struct playerEntity_t // sizeof=0x30
{                                       // ...
    float fLastWeaponPosFrac;
    int32_t bPositionToADS;
    float vPositionLastOrg[3];
    float fLastIdleFactor;
    float vLastMoveOrg[3];
    float vLastMoveAng[3];
};

struct score_t // sizeof=0x28
{                                       // ...
    int32_t client;
    int32_t score;
    int32_t ping;
    int32_t deaths;
    int32_t team;
    int32_t kills;
    int32_t rank;
    int32_t assists;
    Material* hStatusIcon;
    Material* hRankIcon;
};

struct cg_s // sizeof=0xFF580
{
    int32_t clientNum;
    int32_t localClientNum;
    DemoType demoType;
    CubemapShot cubemapShot;
    int32_t cubemapSize;
    int32_t renderScreen;
    int32_t latestSnapshotNum;
    int32_t latestSnapshotTime;
    snapshot_s* snap;
    snapshot_s* nextSnap;
    snapshot_s activeSnapshots[2];
    float frameInterpolation;
    int32_t frametime;
    int32_t time;
    int32_t oldTime;
    int32_t physicsTime;
    int32_t mapRestart;
    int32_t renderingThirdPerson;
    playerState_s predictedPlayerState;
    centity_s predictedPlayerEntity;
    playerEntity_t playerEntity;
    int32_t predictedErrorTime;
    float predictedError[3];
    float landChange;
    int32_t landTime;
    float heightToCeiling;
    refdef_s refdef;
    float refdefViewAngles[3];
    float lastVieworg[3];
    float swayViewAngles[3];
    float swayAngles[3];
    float swayOffset[3];
    int32_t iEntityLastType[1024];
    XModel* pEntityLastXModel[1024];
    float zoomSensitivity;
    bool isLoading;
    char objectiveText[1024];
    char scriptMainMenu[256];
    // padding byte
    // padding byte
    // padding byte
    int32_t scoresRequestTime;
    int32_t numScores;
    int32_t teamScores[4];
    int32_t teamPings[4];
    int32_t teamPlayers[4];
    score_t scores[64];
    int32_t scoreLimit;
    int32_t showScores;
    int32_t scoreFadeTime;
    int32_t scoresTop;
    int32_t scoresOffBottom;
    int32_t scoresBottom;
    int32_t drawHud;
    int32_t crosshairClientNum;
    int32_t crosshairClientLastTime;
    int32_t crosshairClientStartTime;
    int32_t identifyClientNum;
    int32_t cursorHintIcon;
    int32_t cursorHintTime;
    int32_t cursorHintFade;
    int32_t cursorHintString;
    int32_t lastClipFlashTime;
    InvalidCmdHintType invalidCmdHintType;
    int32_t invalidCmdHintTime;
    int32_t lastHealthPulseTime;
    int32_t lastHealthLerpDelay;
    int32_t lastHealthClient;
    float lastHealth;
    float healthOverlayFromAlpha;
    float healthOverlayToAlpha;
    int32_t healthOverlayPulseTime;
    int32_t healthOverlayPulseDuration;
    int32_t healthOverlayPulsePhase;
    bool healthOverlayHurt;
    // padding byte
    // padding byte
    // padding byte
    int32_t healthOverlayLastHitTime;
    float healthOverlayOldHealth;
    int32_t healthOverlayPulseIndex;
    int32_t proneBlockedEndTime;
    int32_t lastStance;
    int32_t lastStanceChangeTime;
    int32_t lastStanceFlashTime;
    int32_t voiceTime;
    uint32_t weaponSelect;
    int32_t weaponSelectTime;
    uint32_t weaponLatestPrimaryIdx;
    int32_t prevViewmodelWeapon;
    int32_t equippedOffHand;
    viewDamage_t viewDamage[8];
    int32_t damageTime;
    float damageX;
    float damageY;
    float damageValue;
    float viewFade;
    int32_t weapIdleTime;
    int32_t nomarks;
    int32_t v_dmg_time;
    float v_dmg_pitch;
    float v_dmg_roll;
    float fBobCycle;
    float xyspeed;
    float kickAVel[3];
    float kickAngles[3];
    float offsetAngles[3];
    float gunPitch;
    float gunYaw;
    float gunXOfs;
    float gunYOfs;
    float gunZOfs;
    float vGunOffset[3];
    float vGunSpeed[3];
    float viewModelAxis[4][3];
    float rumbleScale;
    float compassNorthYaw;
    float compassNorth[2];
    Material* compassMapMaterial;
    float compassMapUpperLeft[2];
    float compassMapWorldSize[2];
    int32_t compassFadeTime;
    int32_t healthFadeTime;
    int32_t ammoFadeTime;
    int32_t stanceFadeTime;
    int32_t sprintFadeTime;
    int32_t offhandFadeTime;
    int32_t offhandFlashTime;
    shellshock_t shellshock;
    //cg_s::<unnamed_type_testShock> testShock;
    struct
    {
        int32_t time;
        int32_t duration;
    }testShock;
    int32_t holdBreathTime;
    int32_t holdBreathInTime;
    int32_t holdBreathDelay;
    float holdBreathFrac;
    float radarProgress;
    float selectedLocation[2];
    SprintState sprintStates;
    int32_t packetAnalysisFrameCount;
    uint8_t bitsSent[100][13];
    int32_t entBitsUsed[10][18];
    int32_t numEntsSent[10][18];
    int32_t numEntFields[10][18];
    int32_t numSnapshots;
    int32_t adsViewErrorDone;
    int32_t inKillCam;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    bgs_t bgs;
    cpose_t viewModelPose;
    visionSetVars_t visionSetPreLoaded[4];
    char visionSetPreLoadedName[4][64];
    visionSetVars_t visionSetFrom[2];
    visionSetVars_t visionSetTo[2];
    visionSetVars_t visionSetCurrent[2];
    visionSetLerpData_t visionSetLerpData[2];
    char visionNameNaked[64];
    char visionNameNight[64];
    int32_t extraButtons;
    int32_t lastActionSlotTime;
    bool playerTeleported;
    // padding byte
    // padding byte
    // padding byte
    int32_t stepViewStart;
    float stepViewChange;
    //cg_s::<unnamed_type_lastFrame> lastFrame;
    struct
    {
        float aimSpreadScale;
    } lastFrame;

    hudElemSoundInfo_t hudElemSound[32];
    int32_t vehicleFrame;
};

// cg_ents_mp
struct ComPrimaryLight;
struct GfxSceneEntity;

void __cdecl CG_Player_PreControllers(DObj_s *obj, centity_s *cent);
void __cdecl CG_mg42_PreControllers(DObj_s *obj, centity_s *cent);
void  CG_UpdateBModelWorldBounds(uint32_t localClientNum, centity_s *cent, int32_t forceFilter);
bool __cdecl CG_VecLessThan(float *a, float *b);
void __cdecl CG_AdjustPositionForMover(
    int32_t localClientNum,
    const float *in,
    int32_t moverNum,
    int32_t fromTime,
    int32_t toTime,
    float *out,
    float *outDeltaAngles);
void __cdecl CG_SetFrameInterpolation(int32_t localClientNum);
void __cdecl CG_ProcessClientNoteTracks(cg_s *cgameGlob, uint32_t clientNum);
void __cdecl CG_AddPacketEntity(int32_t localClientNum, int32_t entnum);
void __cdecl CG_UpdateClientDobjPartBits(centity_s *cent, int32_t entnum, int32_t localClientNum);
int32_t __cdecl CG_AddPacketEntities(int32_t localClientNum);
void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj_s *obj, bool notify);
int32_t __cdecl CG_DObjGetWorldBoneMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    int32_t boneIndex,
    float (*tagMat)[3],
    float *origin);
DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, int32_t boneIndex);
int32_t __cdecl CG_DObjGetWorldTagMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    uint32_t tagName,
    float (*tagMat)[3],
    float *origin);
DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj_s *obj, uint32_t tagName);
int32_t __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj_s *obj, uint32_t tagName, float *pos);
cpose_t *__cdecl CG_GetPose(int32_t localClientNum, uint32_t handle);
void __cdecl CG_CalcEntityLerpPositions(int32_t localClientNum, centity_s *cent);
void __cdecl CG_InterpolateEntityPosition(cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_CalcEntityPhysicsPositions(int32_t localClientNum, centity_s *cent);
void __cdecl CG_CreatePhysicsObject(int32_t localClientNum, centity_s *cent);
void __cdecl CG_UpdatePhysicsPose(centity_s *cent);
char __cdecl CG_ExpiredLaunch(int32_t localClientNum, centity_s *cent);
void __cdecl CG_CalcEntityRagdollPositions(int32_t localClientNum, centity_s *cent);
void __cdecl CG_CreateRagdollObject(int32_t localClientNum, centity_s *cent);
void __cdecl CG_UpdateRagdollPose(centity_s *cent);
DObj_s *__cdecl CG_PreProcess_GetDObj(int32_t localClientNum, int32_t entIndex, int32_t entType, XModel *model);
XAnim_s *__cdecl CG_GetAnimations(int32_t localClientNum, uint32_t entIndex, int32_t entType);
XAnim_s *__cdecl CG_GetMG42Anims(centity_s *cent);
XAnim_s *__cdecl CG_GetHelicopterAnims(centity_s *cent);
char *__cdecl CG_AllocAnimTree(int32_t size);
void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj_s *obj, int32_t boneIndex);
void __cdecl CG_ClearUnion(int32_t localClientNum, centity_s *cent);
void __cdecl CG_SetUnionType(int32_t localClientNum, centity_s *cent);
void __cdecl CG_UpdatePoseUnion(int32_t localClientNum, centity_s *cent);
void __cdecl CG_ProcessEntity(int32_t localClientNum, centity_s *cent);
void __cdecl CG_General(int32_t localClientNum, centity_s *cent);
void __cdecl CG_LockLightingOrigin(centity_s *cent, float *lightingOrigin);
void __cdecl CG_Item(int32_t localClientNum, centity_s *cent);
void __cdecl CG_EntityEffects(int32_t localClientNum, centity_s *cent);
void __cdecl CG_AddEntityLoopSound(int32_t localClientNum, const centity_s *cent);
void __cdecl CG_mg42(int32_t localClientNum, centity_s *cent);
void __cdecl CG_Missile(int32_t localClientNum, centity_s *cent);
void __cdecl CG_ScriptMover(int32_t localClientNum, centity_s *cent);
void __cdecl CG_SoundBlend(int32_t localClientNum, centity_s *cent);
void __cdecl CG_Fx(int32_t localClientNum, centity_s *cent);
FxEffect *__cdecl CG_StartFx(int32_t localClientNum, centity_s *cent, int32_t startAtTime);
void __cdecl CG_LoopFx(int32_t localClientNum, centity_s *cent);
void __cdecl CG_PrimaryLight(int32_t localClientNum, centity_s *cent);
const ComPrimaryLight *__cdecl Com_GetPrimaryLight(uint32_t primaryLightIndex);
void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight);
void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight);
void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin);
void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles);
float *__cdecl CG_GetEntityOrigin(int32_t localClientNum, uint32_t entnum);
void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt);


// cg_main_mp
struct cgMedia_t // sizeof=0x27A8
{                                       // ...
    Material *whiteMaterial;            // ...
    Material *teamStatusBar;            // ...
    Material *balloonMaterial;          // ...
    Material *connectionMaterial;       // ...
    Material *youInKillCamMaterial;     // ...
    Material *tracerMaterial;           // ...
    Material *laserMaterial;            // ...
    Material *laserLightMaterial;       // ...
    Material *lagometerMaterial;        // ...
    Material *hintMaterials[133];       // ...
    Material *stanceMaterials[4];       // ...
    Material *objectiveMaterials[1];    // ...
    Material *friendMaterials[2];       // ...
    Material *damageMaterial;           // ...
    Material *mantleHint;               // ...
    Font_s *smallDevFont;               // ...
    Font_s *bigDevFont;                 // ...
    snd_alias_list_t *landDmgSound;     // ...
    snd_alias_list_t *grenadeExplodeSound[29]; // ...
    snd_alias_list_t *rocketExplodeSound[29]; // ...
    snd_alias_list_t *bulletHitSmallSound[29]; // ...
    snd_alias_list_t *bulletHitLargeSound[29]; // ...
    snd_alias_list_t *bulletHitAPSound[29]; // ...
    snd_alias_list_t *shotgunHitSound[29]; // ...
    snd_alias_list_t *bulletExitSmallSound[29]; // ...
    snd_alias_list_t *bulletExitLargeSound[29]; // ...
    snd_alias_list_t *bulletExitAPSound[29]; // ...
    snd_alias_list_t *shotgunExitSound[29]; // ...
    snd_alias_list_t *stepSprintSound[58]; // ...
    snd_alias_list_t *stepSprintSoundPlayer[58]; // ...
    snd_alias_list_t *stepRunSound[58]; // ...
    snd_alias_list_t *stepRunSoundPlayer[58]; // ...
    snd_alias_list_t *stepWalkSound[58]; // ...
    snd_alias_list_t *stepWalkSoundPlayer[58]; // ...
    snd_alias_list_t *stepProneSound[58]; // ...
    snd_alias_list_t *stepProneSoundPlayer[58]; // ...
    snd_alias_list_t *landSound[58];    // ...
    snd_alias_list_t *landSoundPlayer[58]; // ...
    snd_alias_list_t *qsprintingEquipmentSound; // ...
    snd_alias_list_t *qsprintingEquipmentSoundPlayer; // ...
    snd_alias_list_t *qrunningEquipmentSound; // ...
    snd_alias_list_t *qrunningEquipmentSoundPlayer; // ...
    snd_alias_list_t *qwalkingEquipmentSound; // ...
    snd_alias_list_t *qwalkingEquipmentSoundPlayer; // ...
    snd_alias_list_t *sprintingEquipmentSound; // ...
    snd_alias_list_t *sprintingEquipmentSoundPlayer; // ...
    snd_alias_list_t *runningEquipmentSound; // ...
    snd_alias_list_t *runningEquipmentSoundPlayer; // ...
    snd_alias_list_t *walkingEquipmentSound; // ...
    snd_alias_list_t *walkingEquipmentSoundPlayer; // ...
    snd_alias_list_t *foliageMovement;  // ...
    snd_alias_list_t *bulletWhizby;     // ...
    snd_alias_list_t *meleeHit;         // ...
    snd_alias_list_t *meleeHitOther;    // ...
    snd_alias_list_t *meleeKnifeHit;    // ...
    snd_alias_list_t *meleeKnifeHitOther; // ...
    snd_alias_list_t *nightVisionOn;    // ...
    snd_alias_list_t *nightVisionOff;   // ...
    snd_alias_list_t *playerSprintGasp; // ...
    snd_alias_list_t *playerHeartBeatSound; // ...
    snd_alias_list_t *playerBreathInSound; // ...
    snd_alias_list_t *playerBreathOutSound; // ...
    snd_alias_list_t *playerBreathGaspSound; // ...
    snd_alias_list_t *playerSwapOffhand; // ...
    snd_alias_list_t *physCollisionSound[50][29]; // ...
    Material *compassping_friendlyfiring; // ...
    Material *compassping_friendlyyelling; // ...
    Material *compassping_enemy;        // ...
    Material *compassping_enemyfiring;  // ...
    Material *compassping_enemyyelling; // ...
    Material *compassping_grenade;      // ...
    Material *compassping_explosion;    // ...
    Material *compass_radarline;        // ...
    Material *compass_helicopter_friendly; // ...
    Material *compass_helicopter_enemy; // ...
    Material *compass_plane_friendly;   // ...
    Material *compass_plane_enemy;      // ...
    Material *grenadeIconFrag;          // ...
    Material *grenadeIconFlash;         // ...
    Material *grenadeIconThrowBack;     // ...
    Material *grenadePointer;           // ...
    Material *offscreenObjectivePointer; // ...
    FxImpactTable *fx;                  // ...
    const FxEffectDef *fxNoBloodFleshHit; // ...
    const FxEffectDef *fxKnifeBlood;    // ...
    const FxEffectDef *fxKnifeNoBlood;  // ...
    const FxEffectDef *heliDustEffect;  // ...
    const FxEffectDef *heliWaterEffect; // ...
    const FxEffectDef *helicopterLightSmoke; // ...
    const FxEffectDef *helicopterHeavySmoke; // ...
    const FxEffectDef *helicopterOnFire; // ...
    const FxEffectDef *jetAfterburner;  // ...
    const FxEffectDef *fxVehicleTireDust; // ...
    Material *nightVisionOverlay;       // ...
    Material *hudIconNVG;               // ...
    Material *hudDpadArrow;             // ...
    Material *ammoCounterBullet;        // ...
    Material *ammoCounterBeltBullet;    // ...
    Material *ammoCounterRifleBullet;   // ...
    Material *ammoCounterRocket;        // ...
    Material *ammoCounterShotgunShell;  // ...
    Material *textDecodeCharacters;     // ...
    Material *textDecodeCharactersGlow; // ...
};
struct cgs_t // sizeof=0x3A24
{                                       // ...
    int32_t viewX;
    int32_t viewY;
    int32_t viewWidth;
    int32_t viewHeight;
    float viewAspect;
    int32_t serverCommandSequence;
    int32_t processedSnapshotNum;
    int32_t localServer;
    char gametype[32];
    char szHostName[256];
    int32_t maxclients;
    char mapname[64];
    int32_t gameEndTime;
    int32_t voteTime;
    int32_t voteYes;
    int32_t voteNo;
    char voteString[256];
    XModel *gameModels[512];
    const FxEffectDef *fxs[100];
    const FxEffectDef *smokeGrenadeFx;
    shellshock_parms_t holdBreathParams;
    char teamChatMsgs[8][160];
    int32_t teamChatMsgTimes[8];
    int32_t teamChatPos;
    int32_t teamLastChatPos;
    float compassWidth;
    float compassHeight;
    float compassY;
    clientInfo_t corpseinfo[8];
};

#define CS_EFFECT_NAMES 244

extern weaponInfo_s cg_weaponsArray[1][128];
extern cg_s cgArray[1];
extern cgs_t cgsArray[1];

struct clientConnection_t;
struct snd_alias_t;
struct snd_alias_list_t;
struct PhysPreset;

bool __cdecl CG_IsRagdollTrajectory(const trajectory_t *trajectory);
void __cdecl CG_RegisterDvars();
void __cdecl TRACK_cg_main();
void __cdecl CG_GetDObjOrientation(int32_t localClientNum, int32_t dobjHandle, mat3x3 &axis, float *origin);
void __cdecl CG_GetSoundEntityOrientation(SndEntHandle sndEnt, float *origin_out, float (*axis_out)[3]);
void __cdecl CG_CopyEntityOrientation(int32_t localClientNum, int32_t entIndex, float *origin_out, float (*axis_out)[3]);
const playerState_s *__cdecl CG_GetPredictedPlayerState(int32_t localClientNum);
void __cdecl CG_GameMessage(int32_t localClientNum, const char *msg);
void __cdecl CG_BoldGameMessage(int32_t localClientNum, const char *msg);
void __cdecl CG_RegisterSounds();
void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, snd_alias_list_t **sound);
void CG_RegisterPhysicsSounds();
void CG_RegisterPhysicsSounds_FastFile();
void __cdecl CG_AddAudioPhysicsClass(PhysPreset *physPreset, char (*classes)[64], int32_t *nclasses);
void __cdecl CG_StartAmbient(int32_t localClientNum);
int32_t __cdecl CG_PlayClientSoundAlias(int32_t localClientNum, snd_alias_list_t *aliasList);
int32_t __cdecl CG_PlayClientSoundAliasByName(int32_t localClientNum, const char *aliasname);
int32_t __cdecl CG_PlayEntitySoundAlias(int32_t localClientNum, int32_t entitynum, snd_alias_list_t *aliasList);
void __cdecl CG_StopSoundAlias(int32_t localClientNum, int32_t entitynum, snd_alias_list_t *aliasList);
void __cdecl CG_StopSoundsOnEnt(int32_t localClientNum, int32_t entitynum);
void __cdecl CG_StopSoundAliasByName(int32_t localClientNum, int32_t entityNum, const char *aliasName);
void __cdecl CG_StopClientSoundAliasByName(int32_t localClientNum, const char *aliasName);
void __cdecl CG_SubtitleSndLengthNotify(int32_t msec, const snd_alias_t *lengthNotifyData);
void __cdecl CG_SubtitlePrint(int32_t msec, const snd_alias_t *alias);
void __cdecl CG_AddFXSoundAlias(int32_t localClientNum, const float *origin, snd_alias_list_t *aliasList);
int32_t __cdecl CG_PlaySoundAlias(int32_t localClientNum, int32_t entitynum, const float *origin, snd_alias_list_t *aliasList);
int32_t __cdecl CG_PlaySoundAliasByName(int32_t localClientNum, int32_t entitynum, const float *origin, const char *aliasname);
int32_t __cdecl CG_PlaySoundAliasAsMasterByName(
    int32_t localClientNum,
    int32_t entitynum,
    const float *origin,
    const char *aliasname);
void __cdecl CG_RestartSmokeGrenades(int32_t localClientNum);
void __cdecl CG_InitVote(int32_t localClientNum);
uint16_t __cdecl CG_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType);
int32_t __cdecl CG_GetClientNum(int32_t localClientNum);
void __cdecl CG_Init(int32_t localClientNum, int32_t serverMessageNum, int32_t serverCommandSequence, int32_t clientNum);
clientConnection_t *__cdecl CL_GetLocalClientConnection(int32_t localClientNum);
void __cdecl CG_RegisterGraphics(int32_t localClientNum, const char *mapname);
int32_t __cdecl CG_PlayAnimScriptSoundAlias(int32_t clientIndex, snd_alias_list_t *aliasList);
void __cdecl CG_LoadHudMenu(int32_t localClientNum);
uint16_t __cdecl CG_AttachWeapon(DObjModel_s *dobjModels, uint16_t numModels, clientInfo_t *ci);
void __cdecl CG_CreateDObj(
    DObjModel_s *dobjModels,
    uint16_t numModels,
    XAnimTree_s *tree,
    uint32_t handle,
    int32_t localClientNum,
    clientInfo_t *ci);
DObj_s *__cdecl CG_GetDObj(uint32_t handle, int32_t localClientNum);
void __cdecl CG_LoadAnimTreeInstances(int32_t localClientNum);
void __cdecl CG_InitEntities(int32_t localClientNum);
void __cdecl CG_InitViewDimensions(int32_t localClientNum);
void __cdecl CG_InitDof(GfxDepthOfField *dof);
void __cdecl CG_FreeWeapons(int32_t localClientNum);
void __cdecl CG_Shutdown(int32_t localClientNum);
void __cdecl CG_FreeAnimTreeInstances(int32_t localClientNum);
uint8_t *__cdecl Hunk_AllocXAnimClient(uint32_t size);
uint8_t __cdecl CG_ShouldPlaySoundOnLocalClient();

// cg_newDraw_mp
struct MemoryFile;
void __cdecl CG_AntiBurnInHUD_RegisterDvars();
bool __cdecl CG_ShouldDrawHud(int32_t localClientNum);
double __cdecl CG_FadeHudMenu(int32_t localClientNum, const dvar_s *fadeDvar, int32_t displayStartTime, int32_t duration);
bool __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, uint32_t weapIndex);
bool __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob);
bool __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, uint32_t weapIndex);
bool __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob);
float CG_CalcPlayerHealth(const playerState_s *ps);
void __cdecl CG_ResetLowHealthOverlay(cg_s *cgameGlob);
int32_t __cdecl CG_ServerMaterialName(int32_t localClientNum, int32_t index, char *materialName, uint32_t maxLen);
Material *__cdecl CG_ObjectiveIcon(int32_t localClientNum, int32_t icon, int32_t type);
const char *__cdecl CG_ScriptMainMenu(int32_t localClientNum);
void __cdecl CG_OwnerDraw(
    int32_t localClientNum,
    rectDef_s parentRect,
    float x,
    float y,
    float w,
    float h,
    int32_t horzAlign,
    int32_t vertAlign,
    float text_x,
    float text_y,
    int32_t ownerDraw,
    int32_t ownerDrawFlags,
    int32_t align,
    float special,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int32_t textStyle,
    char textAlignMode);
void __cdecl CG_DrawPlayerAmmoBackdrop(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerAmmoValue(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int32_t textStyle);
void __cdecl CG_DrawPlayerWeaponName(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int32_t textStyle);
void __cdecl CG_DrawPlayerWeaponNameBack(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerStance(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Font_s *font,
    float scale,
    int32_t textStyle);
void __cdecl CG_DrawStanceIcon(
    int32_t localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    float x,
    float y,
    float fadeAlpha);
void __cdecl CG_DrawStanceHintPrints(
    int32_t localClientNum,
    const rectDef_s *rect,
    float x,
    const float *color,
    float fadeAlpha,
    Font_s *font,
    float scale,
    int32_t textStyle);
void __cdecl CG_DrawPlayerSprintBack(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_DrawPlayerSprintMeter(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, float *color);
void __cdecl CG_DrawPlayerBarHealth(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_DrawPlayerBarHealthBack(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_DrawPlayerLowHealthOverlay(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color);
double __cdecl CG_FadeLowHealthOverlay(const cg_s *cgameGlob);
void __cdecl CG_PulseLowHealthOverlay(cg_s *cgameGlob, float healthRatio);
void __cdecl CG_DrawCursorhint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle);
void __cdecl CG_UpdateCursorHints(cg_s *cgameGlob);
char *__cdecl CG_GetWeaponUseString(int32_t localClientNum, const char **secondaryString);
char *__cdecl CG_GetUseString(int32_t localClientNum);
void __cdecl CG_DrawHoldBreathHint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    int32_t textStyle);
void __cdecl CG_DrawMantleHint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    const float *color,
    int32_t textStyle);
void __cdecl CG_DrawInvalidCmdHint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle);
void __cdecl CG_DrawTalkerNum(
    int32_t localClientNum,
    int32_t num,
    rectDef_s *rect,
    Font_s *font,
    float *color,
    float textScale,
    int32_t style);
void __cdecl CG_ArchiveState(int32_t localClientNum, MemoryFile *memFile);



// cg_draw_mp
struct OverheadFade // sizeof=0xC
{                                       // ...
    int32_t lastTime;                       // ...
    int32_t startTime;                      // ...
    bool visible;                       // ...
    // padding byte
    // padding byte
    // padding byte
};
void __cdecl TRACK_cg_draw();
void __cdecl CG_PriorityCenterPrint(int32_t localClientNum, const char *str, int32_t priority);
void __cdecl CG_ClearCenterPrint(int32_t localClientNum);
void __cdecl CG_DrawCenterString(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle);
void __cdecl CG_ClearOverheadFade();
void __cdecl CG_Draw2D(int32_t localClientNum);
void __cdecl CG_DrawChatMessages(int32_t localClientNum);
void __cdecl CG_ScanForCrosshairEntity(int32_t localClientNum);
void __cdecl CG_CheckTimedMenus(int32_t localClientNum);
void __cdecl CG_CheckForPlayerInput(int32_t localClientNum);
bool __cdecl CG_CheckPlayerMovement(usercmd_s oldCmd, usercmd_s newCmd);
int32_t __cdecl CG_CheckPlayerStanceChange(int32_t localClientNum, __int16 newButtons, __int16 changedButtons);
int32_t __cdecl CG_CheckPlayerWeaponUsage(int32_t localClientNum, char buttons);
bool __cdecl CG_CheckPlayerTryReload(int32_t localClientNum, char buttons);
bool __cdecl CG_CheckPlayerFireNonTurret(int32_t localClientNum, char buttons);
int32_t __cdecl CG_CheckPlayerOffHandUsage(int32_t localClientNum, __int16 buttons);
uint32_t __cdecl CG_CheckPlayerMiscInput(int32_t buttons);
void __cdecl CG_CheckHudHealthDisplay(int32_t localClientNum);
void __cdecl CG_CheckHudAmmoDisplay(int32_t localClientNum);
void __cdecl CG_CheckHudCompassDisplay(int32_t localClientNum);
void __cdecl CG_CheckHudStanceDisplay(int32_t localClientNum);
void __cdecl CG_CheckHudSprintDisplay(int32_t localClientNum);
void __cdecl CG_CheckHudOffHandDisplay(int32_t localClientNum);
void __cdecl CG_CheckHudObjectiveDisplay(int32_t localClientNum);
void __cdecl CG_DrawMiniConsole(int32_t localClientNum);
void __cdecl CG_DrawErrorMessages(int32_t localClientNum);
void __cdecl CG_DrawSay(int32_t localClientNum);
void __cdecl CG_DrawVote(int32_t localClientNum);
void __cdecl DrawIntermission(int32_t localClientNum);
void __cdecl CG_DrawSpectatorMessage(int32_t localClientNum);
int32_t __cdecl CG_DrawFollow(int32_t localClientNum);
void __cdecl CG_UpdatePlayerNames(int32_t localClientNum);
void __cdecl CG_DrawFriendlyNames(int32_t localClientNum);
void __cdecl CG_DrawOverheadNames(int32_t localClientNum, const centity_s *cent, float alpha);
char __cdecl CG_CalcNamePosition(int32_t localClientNum, float *origin, float *xOut, float *yOut);
double __cdecl CG_FadeCrosshairNameAlpha(int32_t time, int32_t startMsec, int32_t lastMsec, int32_t fadeInMsec, int32_t fadeOutMsec);
bool __cdecl CG_CanSeeFriendlyHead(int32_t localClientNum, const centity_s *cent);
void __cdecl CG_DrawCrosshairNames(int32_t localClientNum);
void __cdecl DrawViewmodelInfo(int32_t localClientNum);
void __cdecl CG_DrawActive(int32_t localClientNum);
void __cdecl CG_AddSceneTracerBeams(int32_t localClientNum);
void __cdecl CG_GenerateSceneVerts(int32_t localClientNum);
void __cdecl CG_GetViewAxisProjections(const refdef_s *refdef, const float *worldPoint, float *projections);


// cg_players_mp
void __cdecl CG_AddAllPlayerSpriteDrawSurfs(int32_t localClientNum);
void __cdecl CG_AddPlayerSpriteDrawSurfs(int32_t localClientNum, const centity_s *cent);
void  CG_AddPlayerSpriteDrawSurf(
    int32_t localClientNum,
    const centity_s *cent,
    Material *material,
    float additionalRadiusSize,
    int32_t height,
    bool fixedScreenSize);
void __cdecl CG_Player(int32_t localClientNum, centity_s *cent);
void __cdecl CG_PlayerTurretPositionAndBlend(int32_t localClientNum, centity_s *cent);
void __cdecl CG_Corpse(int32_t localClientNum, centity_s *cent);
void __cdecl CG_UpdatePlayerDObj(int32_t localClientNum, centity_s *cent);
void __cdecl CG_ResetPlayerEntity(int32_t localClientNum, cg_s *cgameGlob, centity_s *cent, int32_t resetAnimation);
const char *__cdecl CG_GetTeamName(team_t team);
const char *__cdecl CG_GetOpposingTeamName(team_t team);
const char *__cdecl CG_GetPlayerTeamName(int32_t localClientNum);
const char *__cdecl CG_GetPlayerOpposingTeamName(int32_t localClientNum);
bool __cdecl CG_IsPlayerDead(int32_t localClientNum);
int32_t __cdecl CG_GetPlayerClipAmmoCount(int32_t localClientNum);
void __cdecl CG_UpdateWeaponVisibility(int32_t localClientNum, centity_s *cent);
bool __cdecl CG_IsWeaponVisible(int32_t localClientNum, centity_s *cent, XModel *weapModel, float *origin, float *forward);
void __cdecl CG_CalcWeaponVisTrace(
    XModel *weapModel,
    float *origin,
    float *forward,
    float *start,
    float *end,
    float *modelLen);



// cg_scoreboard_mp
enum listColumnTypes_t : __int32
{                                       // ...
    LCT_NAME = 0x0,             // ...
    LCT_CLAN = 0x1,
    LCT_SCORE = 0x2,             // ...
    LCT_DEATHS = 0x3,             // ...
    LCT_PING = 0x4,             // ...
    LCT_STATUS_ICON = 0x5,             // ...
    LCT_TALKING_ICON = 0x6,             // ...
    LCT_KILLS = 0x7,             // ...
    LCT_RANK_ICON = 0x8,             // ...
    LCT_ASSISTS = 0x9,             // ...
    LCT_NUM = 0xA,
};
struct listColumnInfo_t // sizeof=0x10
{                                       // ...
    listColumnTypes_t type;
    float fWidth;
    const char *pszName;
    int32_t iAlignment;
};
void __cdecl UpdateScores(int32_t localClientNum);
const score_t *__cdecl UI_GetOurClientScore(int32_t localClientNum);
const score_t *__cdecl GetClientScore(int32_t localClientNum, int32_t clientNum);
const score_t *__cdecl UI_GetScoreAtRank(int32_t localClientNum, int32_t rank);
char *__cdecl CG_GetGametypeDescription(int32_t localClientNum);
char __cdecl CG_DrawScoreboard_GetTeamColorIndex(int32_t team, int32_t localClientNum);
int32_t __cdecl CG_DrawScoreboard(int32_t localClientNum);
void __cdecl CG_DrawScoreboard_Backdrop(int32_t localClientNum, float alpha);
double __cdecl CG_BackdropLeft(int32_t localClientNum);
double __cdecl CG_BackdropTop();
float __cdecl CG_BannerScoreboardScaleMultiplier();
void __cdecl CG_DrawScoreboard_ScoresList(int32_t localClientNum, float alpha);
double __cdecl CG_DrawScoreboard_ListColumnHeaders(
    int32_t localClientNum,
    const float *color,
    float y,
    float h,
    float listWidth);
void __cdecl CG_GetScoreboardInfo(const listColumnInfo_t **colInfo, int32_t *numFields);
int32_t __cdecl CG_ScoreboardTotalLines(int32_t localClientNum);
double __cdecl CG_DrawTeamOfClientScore(
    int32_t localClientNum,
    const float *color,
    float y,
    int32_t team,
    float listWidth,
    int32_t *drawLine);
int32_t __cdecl CG_CheckDrawScoreboardLine(int32_t localClientNum, int32_t *drawLine, float y, float lineHeight);
double __cdecl CG_DrawScoreboard_ListBanner(
    int32_t localClientNum,
    const float *color,
    float y,
    float w,
    float h,
    int32_t team,
    int32_t *piDrawLine);
double __cdecl CG_DrawClientScore(
    int32_t localClientNum,
    const float *color,
    float y,
    const score_t *score,
    float listWidth);
double __cdecl CalcXAdj(int32_t align, float maxw, float w);
void __cdecl DrawListString(
    int32_t localClientNum,
    char *string,
    float x,
    float y,
    float width,
    int32_t alignment,
    Font_s *font,
    float scale,
    int32_t style,
    const float *color);
void __cdecl CG_DrawClientPing(int32_t localClientNum, int32_t ping, float x, float y, float maxWidth, float maxHeight);
void __cdecl CG_DrawScrollbar(int32_t localClientNum, const float *color, float top);
void __cdecl CenterViewOnClient(int32_t localClientNum);
int32_t __cdecl CG_IsScoreboardDisplayed(int32_t localClientNum);
void __cdecl CG_ScrollScoreboardUp(cg_s *cgameGlob);
void __cdecl CG_ScrollScoreboardDown(cg_s *cgameGlob);
void __cdecl CG_RegisterScoreboardDvars();
void __cdecl CG_RegisterScoreboardGraphics();
bool __cdecl Scoreboard_HandleInput(int32_t localClientNum, int32_t key);


// cg_vehicles_mp
struct vehicleEffects // sizeof=0x28
{
    bool active;
    // padding byte
    // padding byte
    // padding byte
    int32_t lastAccessed;
    int32_t entityNum;
    int32_t nextDustFx;
    int32_t nextSmokeFx;
    bool soundPlaying;
    // padding byte
    // padding byte
    // padding byte
    float barrelVelocity;
    float barrelPos;
    int32_t lastBarrelUpdateTime;
    uint8_t tag_engine_left;
    uint8_t tag_engine_right;
    // padding byte
    // padding byte
};
struct vehfx_t // sizeof=0x48
{                                       // ...
    bool tireActive[4];
    float tireGroundPoint[4][3];
    uint8_t tireGroundSurfType[4];
    bool soundEnabled;
    // padding byte
    // padding byte
    // padding byte
    float soundEngineOrigin[3];
};
void __cdecl CG_VehRegisterDvars();
DObj_s *__cdecl GetVehicleEntDObj(int32_t localClientNum, centity_s *centVeh);
void __cdecl CG_VehGunnerPOV(int32_t localClientNum, float *resultOrigin, float *resultAngles);
clientInfo_t *__cdecl ClientInfoForLocalClient(int32_t localClientNum);
void __cdecl GetTagMatrix(
    int32_t localClientNum,
    uint32_t vehEntNum,
    uint16_t tagName,
    //float (*resultTagMat)[3],
    mat3x3 &resultTagMat,
    float *resultOrigin);
bool __cdecl CG_VehLocalClientUsingVehicle(int32_t localClientNum);
bool __cdecl CG_VehLocalClientDriving(int32_t localClientNum);
bool __cdecl CG_VehEntityUsingVehicle(int32_t localClientNum, uint32_t entNum);
clientInfo_t *__cdecl ClientInfoForEntity(int32_t localClientNum, uint32_t entNum);
int32_t __cdecl CG_VehLocalClientVehicleSlot(int32_t localClientNum);
int32_t __cdecl CG_VehPlayerVehicleSlot(int32_t localClientNum, uint32_t entNum);
void __cdecl CG_VehSeatTransformForPlayer(
    int32_t localClientNum,
    uint32_t entNum,
    float *resultOrigin,
    float *resultAngles);
void __cdecl SeatTransformForClientInfo(int32_t localClientNum, clientInfo_t *ci, float *resultOrigin, float *resultAngles);
void __cdecl SeatTransformForSlot(
    int32_t localClientNum,
    uint32_t vehEntNum,
    uint32_t vehSlotIdx,
    float *resultOrigin,
    float *resultAngles);
void __cdecl CG_VehSeatOriginForLocalClient(int32_t localClientNum, float *result);
double __cdecl Veh_GetTurretBarrelRoll(int32_t localClientNum, centity_s *cent);
int32_t __cdecl CG_GetEntityIndex(int32_t localClientNum, const centity_s *cent);
void __cdecl Veh_IncTurretBarrelRoll(int32_t localClientNum, int32_t entityNum, float rotation);
void __cdecl CG_VehProcessEntity(int32_t localClientNum, centity_s *cent);
void __cdecl SetupPoseControllers(int32_t localClientNum, DObj_s *obj, centity_s *cent, vehfx_t *fxInfo);
void __cdecl VehicleFXTest(int32_t localClientNum, const DObj_s *obj, centity_s *cent, vehfx_t *fxInfo);
double __cdecl GetSpeed(int32_t localClientNum, centity_s *cent);
void __cdecl CG_VehSphereCoordsToPos(float sphereDistance, float sphereYaw, float sphereAltitude, float *result);
void __cdecl CG_Veh_Init();


// cg_snapshot_mp
void __cdecl CG_ShutdownEntity(int32_t localClientNum, centity_s *cent);
void __cdecl CG_SetInitialSnapshot(int32_t localClientNum, snapshot_s *snap);
void __cdecl CG_SetNextSnap(int32_t localClientNum, snapshot_s *snap);
void __cdecl CG_ResetEntity(int32_t localClientNum, centity_s *cent, int32_t newEntity);
void __cdecl CG_CopyCorpseInfo(clientInfo_t *corpseInfo, const clientInfo_t *ci);
void __cdecl CG_TransitionKillcam(int32_t localClientNum);
void __cdecl CG_ProcessSnapshots(int32_t localClientNum);
void __cdecl CG_TransitionSnapshot(int32_t localClientNum);
snapshot_s *__cdecl CG_ReadNextSnapshot(int32_t localClientNum);
void __cdecl CG_ExtractTransPlayerState(const playerState_s *ps, transPlayerState_t *transPs);


// cg_servercmds_mp
void __cdecl CG_ParseServerInfo(int32_t localClientNum);
void __cdecl CG_ParseCodInfo(int32_t localClientNum);
void __cdecl CG_ParseFog(int32_t localClientNum);
void __cdecl CG_SetConfigValues(int32_t localClientNum);
void __cdecl CG_ParseGameEndTime(int32_t localClientNum);
void __cdecl CG_PrecacheScriptMenu(int32_t localClientNum, int32_t configStringIndex);
void __cdecl CG_RegisterServerMaterial(int32_t localClientNum, int32_t configStringIndex);
void __cdecl CG_MapRestart(int32_t localClientNum, int32_t savepersist);
void __cdecl CG_ClearEntityFxHandles(int32_t localClientNum);
void __cdecl CG_CheckOpenWaitingScriptMenu(int32_t localClientNum);
void __cdecl CG_CloseScriptMenu(int32_t localClientNum, bool allowResponse);
void __cdecl CG_MenuShowNotify(int32_t localClientNum, int32_t menuToShow);
void __cdecl CG_ServerCommand(int32_t localClientNum);
void __cdecl CG_DeployServerCommand(int32_t localClientNum);
void __cdecl CG_ParseScores(int32_t localClientNum);
void __cdecl CG_SetSingleClientScore(int32_t localClientNum, int32_t clientIndex, int32_t newScore);
void __cdecl CG_SortSingleClientScore(cg_s *cgameGlob, int32_t scoreIndex);
bool __cdecl CG_ClientScoreIsBetter(score_t *scoreA, score_t *scoreB);
void __cdecl CG_ConfigStringModified(int32_t localClientNum);
void __cdecl CG_UpdateVoteString(int32_t localClientNum, const char *rawVoteString);
void __cdecl CG_AddToTeamChat(int32_t localClientNum, const char *str);
void __cdecl CG_OpenScriptMenu(int32_t localClientNum);
void __cdecl CG_RemoveChatEscapeChar(char *text);
void __cdecl CG_SetTeamScore(int32_t localClientNum, uint32_t team, int32_t score);
void CG_ReverbCmd();
void CG_DeactivateReverbCmd();
void __cdecl CG_SetChannelVolCmd(int32_t localClientNum);
void CG_DeactivateChannelVolCmd();
char __cdecl LocalSound(int32_t localClientNum);
void __cdecl LocalSoundStop(int32_t localClientNum);
void __cdecl CG_SetClientDvarFromServer(cg_s *cgameGlob, const char *dvarname, char *value);
void __cdecl CG_SetObjectiveText(cg_s *cgameGlob, char *text);
void __cdecl CG_SetDrawHud(cg_s *cgameGlob, uint32_t value);
void __cdecl CG_SetScriptMainMenu(cg_s *cgameGlob, char *text);
void __cdecl CG_ExecuteNewServerCommands(int32_t localClientNum, int32_t latestSequence);



// cg_predict_mp
void __cdecl TRACK_cg_predict();
int32_t __cdecl CG_ItemListLocalClientNum();
void __cdecl CG_ClearItemList();
void __cdecl CG_BuildItemList(int32_t localClientNum, const snapshot_s *nextSnap);
void __cdecl CG_PredictPlayerState(int32_t localClientNum);
void __cdecl CG_PredictPlayerState_Internal(int32_t localClientNum);
void __cdecl CG_TouchItemPrediction(int32_t localClientNum);
void __cdecl CG_TouchItem(cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_InterpolatePlayerState(int32_t localClientNum, int32_t grabAngles);



// cg_pose_mp
void __cdecl BG_Player_DoControllers(const CEntPlayerInfo *player, const DObj_s *obj, int32_t *partBits);
void  CG_VehPoseControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits);
void __cdecl CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits);
void __cdecl CG_Player_DoControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits);
void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int32_t *partBits);
void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int32_t *setPartBits);
DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int32_t *partBits);



// cg_draw_net_mp
void __cdecl CG_AddLagometerFrameInfo(const cg_s *cgameGlob);
void __cdecl CG_AddLagometerSnapshotInfo(snapshot_s *snap);
void __cdecl CG_DrawSnapshotAnalysis(int32_t localClientNum);
int32_t __cdecl CG_ComparePacketAnalysisSamples(int32_t *a, int32_t *b);
void __cdecl CG_DrawSnapshotEntityAnalysis(int32_t localClientNum);
int32_t __cdecl CG_CompareEntityAnalysisSamples(uint32_t *a, uint32_t *b);
void __cdecl CG_DrawPingAnalysis(int32_t localClientNum);
void __cdecl CG_DrawLagometer(int32_t localClientNum);
void __cdecl CG_DrawDisconnect(int32_t localClientNum);



struct CompassVehicle // sizeof=0x1C
{                                       // ...
    int32_t entityNum;                      // ...
    int32_t lastUpdate;                     // ...
    float lastPos[2];
    float lastYaw;
    team_t team;
    int32_t ownerIndex;
};
struct CompassActor // sizeof=0x30
{                                       // ...
    int32_t lastUpdate;
    float lastPos[2];
    float lastEnemyPos[2];
    float lastYaw;
    int32_t pingTime;
    int32_t beginFadeTime;
    int32_t beginRadarFadeTime;
    int32_t beginVoiceFadeTime;
    bool enemy;
    // padding byte
    // padding byte
    // padding byte
    int32_t perks;
};
void __cdecl TRACK_cg_compassfriendlies();
void __cdecl CG_ClearCompassPingData();
void __cdecl CG_CompassUpdateVehicleInfo(int32_t localClientNum, int32_t entityIndex);
CompassVehicle *__cdecl GetVehicle(int32_t localClientNum, int32_t entityNum);
void __cdecl CG_CompassRadarPingEnemyPlayers(int32_t localClientNum, float oldRadarProgress, float newRadarProgress);
void __cdecl GetRadarLine(cg_s *cgameGlob, float radarProgress, float *line);
double __cdecl GetRadarLineMargin(cg_s *cgameGlob);
bool __cdecl DoLinesSurroundPoint(cg_s *cgameGlob, float *radarLine1, float *radarLine2, float *pos);
void __cdecl RadarPingEnemyPlayer(CompassActor *actor, int32_t time);
void __cdecl CG_CompassIncreaseRadarTime(int32_t localClientNum);
void __cdecl CG_CompassAddWeaponPingInfo(int32_t localClientNum, const centity_s *cent, const float *origin, int32_t msec);
void __cdecl ActorUpdatePos(int32_t localClientNum, CompassActor *actor, const float *newPos, int32_t actorClientIndex);
bool __cdecl DoesMovementCrossRadar(cg_s *cgameGlob, float radarProgress, const float *p1, const float *p2);
bool __cdecl CanLocalPlayerHearActorFootsteps(int32_t localClientNum, const float *actorPos, uint32_t actorClientIndex);
void __cdecl CG_CompassUpdateActors(int32_t localClientNum);
void __cdecl CG_CompassDrawFriendlies(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color);
void __cdecl CG_CompassDrawEnemies(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color);
void __cdecl CG_CompassDrawRadarEffects(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color);
double __cdecl GetRadarLineEastWestPercentage(cg_s *cgameGlob, float radarProgress);
void __cdecl CG_CompassDrawVehicles(
    int32_t localClientNum,
    CompassType compassType,
    int32_t eType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *enemyMaterial,
    Material *friendlyMaterial,
    const float *color);



// cg_client_side_effects_mp
struct ClientEntSound // sizeof=0x10
{                                       // ...
    float origin[3];
    snd_alias_list_t *aliasList;        // ...
};
void __cdecl CG_StartClientSideEffects(int32_t localClientNum);
void __cdecl CG_LoadClientEffects(int32_t localClientNum, const char *filename);
void __cdecl CG_ParseClientEffects(int32_t localClientNum, char *buffer);
const char *__cdecl CG_SkipLine(char *line, const char *skipLine);
const char *__cdecl CG_SkipWhiteSpace(const char *line);
char *__cdecl CG_SkipText(char *line, const char *skipText);
const char *__cdecl CG_SkipLineStartingWith(char *line, const char *skipLine);
const char *__cdecl CG_SkipRestOfLine(const char *line);
bool __cdecl CG_MatchLineStartingWith(const char *line, const char *startLine);
const char *__cdecl CG_ParseSound(int32_t localClientNum, char *line);
void __cdecl CG_AddClientEntSound(const float *origin, const char *soundalias);
const char *__cdecl CG_ParseVec3Finish(char *line, float *origin);
const char *__cdecl CG_ParseStringFinish(char *line, char *text, uint32_t bufferSize);
char *__cdecl CG_ParseString(char *line, char *text, uint32_t bufferSize);
char *__cdecl CG_ParseEffect(int32_t localClientNum, char *line);
const char *__cdecl CG_ParseFloatFinish(char *line, float *value);
char __cdecl CG_FindFileName(const char *name, char *filename, int32_t size);
void __cdecl CG_LoadClientEffects_FastFile(int32_t localClientNum, const char *filename);
void __cdecl CG_LoadClientEffectMapping(const char *filename);
void __cdecl CG_ParseClientEffectMapping(const char *buffer);
void __cdecl CG_AddPairToMap(char *name, char *filename);
void __cdecl CG_LoadClientEffectMapping_FastFile(const char *filename);
void __cdecl CG_ClientSideEffectsRegisterDvars();
void __cdecl CG_AddClientSideSounds(int32_t localClientNum);
void __cdecl CG_AddClientSideSound(int32_t localClientNum, int32_t index, const ClientEntSound *sound);
void __cdecl CG_CopyClientSideSoundEntityOrientation(
    uint32_t clientSoundEntIndex,
    float *origin_out,
    float (*axis_out)[3]);


// cg_animtree_mp
void __cdecl CGScr_LoadAnimTrees();
void __cdecl CG_FreeClientDObjInfo(int32_t localClientNum);
void __cdecl CG_SetDObjInfo(int32_t localClientNum, int32_t iEntNum, int32_t iEntType, XModel *pXModel);
bool __cdecl CG_CheckDObjInfoMatches(int32_t localClientNum, int32_t iEntNum, int32_t iEntType, XModel *pXModel);
void __cdecl CG_SafeDObjFree(int32_t localClientNum, int32_t entIndex);
void __cdecl CG_FreeEntityDObjInfo(int32_t localClientNum);


// cg_view_mp
struct ClientViewParams // sizeof=0x10
{                                       // ...
    float x;
    float y;
    float width;
    float height;
};
struct TestEffect // sizeof=0x54
{                                       // ...
    char name[64];
    float pos[3];
    int32_t time;
    int32_t respawnTime;
};

void __cdecl CL_SyncGpu(int(__cdecl *WorkCallback)(uint64_t));

void __cdecl TRACK_cg_view();
void __cdecl CG_FxSetTestPosition();
void __cdecl CG_FxTest();
void __cdecl CG_PlayTestFx(int32_t localClientNum);
double __cdecl CG_GetViewFov(int32_t localClientNum);
void __cdecl CG_ViewRegisterDvars();
void __cdecl CG_UpdateHelicopterKillCam(int32_t localClientNum);
void __cdecl CG_UpdateFov(int32_t localClientNum, float fov_x);
void __cdecl CG_UpdateHelicopterKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_UpdateAirstrikeKillCam(int32_t localClientNum);
void __cdecl CG_UpdateAirstrikeKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_InitView(int32_t localClientNum);
void __cdecl CG_CalcViewValues(int32_t localClientNum);
void __cdecl CG_OffsetThirdPersonView(cg_s *cgameGlob);
void __cdecl ThirdPersonViewTrace(cg_s *cgameGlob, float *start, float *end, int32_t contentMask, float *result);
void __cdecl CG_CalcVrect(int32_t localClientNum);
void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob);
void __cdecl CG_OffsetFirstPersonView(cg_s *cgameGlob);
void __cdecl CG_CalcFov(int32_t localClientNum);
void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob);
void __cdecl CG_CalcTurretViewValues(int32_t localClientNum);
void __cdecl CG_ApplyViewAnimation(int32_t localClientNum);
void __cdecl CalcViewValuesVehicle(int32_t localClientNum);
void CalcViewValuesVehicleDriver(int32_t localClientNum);
void CalcViewValuesVehiclePassenger(int32_t localClientNum);
void __cdecl CalcViewValuesVehicleGunner(int32_t localClientNum);
bool __cdecl CG_HelicopterKillCamEnabled(int32_t localClientNum);
bool __cdecl CG_AirstrikeKillCamEnabled(int32_t localClientNum);
void __cdecl CG_UpdateThirdPerson(int32_t localClientNum);
bool __cdecl CG_KillCamEntityEnabled(int32_t localClientNum);
const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int32_t localClientNum);
void __cdecl CG_UpdateViewOffset(int32_t localClientNum);
void __cdecl CG_UpdateKillCamEntityViewOffset(int32_t localClientNum);
int32_t __cdecl CG_DrawActiveFrame(
    int32_t localClientNum,
    int32_t serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int32_t cubemapSize,
    int32_t renderScreen);
void __cdecl CG_UpdateTestFX(int32_t localClientNum);
void __cdecl CG_KickAngles(cg_s *cgameGlob);
void __cdecl CG_UpdateEntInfo(int32_t localClientNum);
void __cdecl GetCeilingHeight(cg_s *cgameGlob);
void __cdecl DumpAnims(int32_t localClientNum);
void __cdecl DrawShellshockBlend(int32_t localClientNum);
void __cdecl CG_UpdateSceneDepthOfField(int32_t localClientNum);
void __cdecl CG_UpdateAdsDof(int32_t localClientNum, GfxDepthOfField *dof);
double __cdecl CG_UpdateAdsDofValue(float currentValue, float targetValue, float maxChange, float dt);


// cg_consolecmds_mp
void __cdecl CG_ScoresUp(int32_t localClientNum);
void __cdecl CG_InitConsoleCommands();
void __cdecl CG_Viewpos_f();
void __cdecl CG_ScoresUp_f();
void __cdecl CG_ScoresDown_f();
void __cdecl CG_ScoresDown(int32_t localClientNum);
void __cdecl CG_ShellShock_f();
void __cdecl CG_ShellShock_Load_f();
void __cdecl CG_ShellShock_Save_f();
void __cdecl CG_QuickMessage_f();
void __cdecl CG_VoiceChat_f();
void __cdecl CG_TeamVoiceChat_f();
void __cdecl CG_RestartSmokeGrenades_f();
void __cdecl UpdateGlowTweaks_f();
void __cdecl UpdateFilmTweaks_f();
void __cdecl CG_ShutdownConsoleCommands();


extern const dvar_t *cg_hudGrenadeIconEnabledFlash;
extern const dvar_t *cg_hudGrenadePointerPulseMax;
extern const dvar_t *cg_laserLight;
extern const dvar_t *cg_drawVersionY;
extern const dvar_t *cg_drawVersion;
extern const dvar_t *cg_gun_y;
extern const dvar_t *cg_hudSayPosition;
extern const dvar_t *cg_enemyNameFadeOut;
extern const dvar_t *cg_fovMin;
extern const dvar_t *cg_drawBreathHint;
extern const dvar_t *cg_enemyNameFadeIn;
extern const dvar_t *cg_hudGrenadeIconMaxRangeFrag;
extern const dvar_t *cg_drawFPS;
extern const dvar_t *cg_drawVersionX;
extern const dvar_t *cg_thirdPerson;
extern const dvar_t *cg_overheadNamesNearDist;
extern const dvar_t *cg_drawHealth;
extern const dvar_t *cg_brass;
extern const dvar_t *cg_weaponHintsCoD1Style;
extern const dvar_t *debugOverlay;
extern const dvar_t *cg_viewZSmoothingMax;
extern const dvar_t *cg_marks_ents_player_only;
extern const dvar_t *cg_drawCrosshair;
extern const dvar_t *cg_friendlyNameFadeOut;
extern const dvar_t *cg_packetAnalysisEntTextScale;
extern const dvar_t *cg_hudSplitscreenCompassScale;
extern const dvar_t *cg_laserRadius;
extern const dvar_t *cg_invalidCmdHintDuration;
extern const dvar_t *cg_mapLocationSelectionCursorSpeed;
extern const dvar_t *cg_hudSplitscreenStanceScale;
extern const dvar_t *cg_hudGrenadeIconWidth;
extern const dvar_t *cg_gun_z;
extern const dvar_t *cg_hudDamageIconOffset;
extern const dvar_t *overrideNVGModelWithKnife;
extern const dvar_t *cg_voiceIconSize;
extern const dvar_t *cg_viewZSmoothingMin;
extern const dvar_t *cg_drawTurretCrosshair;
extern const dvar_t *cg_hudDamageIconHeight;
extern const dvar_t *cg_drawSnapshot;
extern const dvar_t *cg_hudChatPosition;
extern const dvar_t *cg_weaponrightbone;
extern const dvar_t *cg_drawShellshock;
extern const dvar_t *cg_overheadNamesMaxDist;
extern const dvar_t *cg_gun_move_minspeed;
extern const dvar_t *cg_gun_ofs_u;
extern const dvar_t *cg_subtitles;
extern const dvar_t *cg_packetAnalysisTextY;
extern const dvar_t *cg_hudStanceHintPrints;
extern const dvar_t *cg_youInKillCamSize;
extern const dvar_t *cg_hudDamageIconWidth;
extern const dvar_t *cg_overheadNamesFarScale;
extern const dvar_t *cg_hudProneY;
extern const dvar_t *cg_draw2D;
extern const dvar_t *cg_gun_x;
extern const dvar_t *cg_crosshairAlpha;
extern const dvar_t *cg_friendlyNameFadeIn;
extern const dvar_t *cg_laserLightRadius;
extern const dvar_t *cg_overheadNamesFarDist;
extern const dvar_t *cg_packetAnalysisClient;
extern const dvar_t *cg_debugPosition;
extern const dvar_t *cg_drawThroughWalls;
extern const dvar_t *cg_drawGun;
extern const dvar_t *cg_hudGrenadeIconOffset;
extern const dvar_t *cg_hudSplitscreenScoreboardScale;
extern const dvar_t *cg_overheadNamesSize;
extern const dvar_t *cg_centertime;
extern const dvar_t *cg_chatHeight;
extern const dvar_t *cg_hudGrenadePointerPulseFreq;
extern const dvar_t *cg_minicon;
extern const dvar_t *cg_drawCrosshairNamesPosX;
extern const dvar_t *cg_overheadNamesFont;
extern const dvar_t *cg_gun_move_rate;
extern const dvar_t *cg_tracerSpeed;
extern const dvar_t *cg_laserFlarePct;
extern const dvar_t *cg_invalidCmdHintBlinkInterval;
extern const dvar_t *cg_overheadRankSize;
extern const dvar_t *cg_hudGrenadePointerPivot;
extern const dvar_t *cg_crosshairAlphaMin;
extern const dvar_t *cg_laserLightEndOffset;
extern const dvar_t *cg_tracerChance;
extern const dvar_t *cg_hudGrenadeIconInScope;
extern const dvar_t *cg_hudSplitscreenBannerScoreboardScale;
extern const dvar_t *cg_fovScale;
extern const dvar_t *cg_crosshairEnemyColor;
extern const dvar_t *cg_drawRumbleDebug;
extern const dvar_t *cg_laserRangePlayer;
extern const dvar_t *cg_tracerScale;
extern const dvar_t *cg_weaponCycleDelay;
extern const dvar_t *cg_laserRange;
extern const dvar_t *cg_laserForceOn;
extern const dvar_t *cg_gameMessageWidth;
extern const dvar_t *cg_thirdPersonRange;
extern const dvar_t *cg_gun_ofs_r;
extern const dvar_t *cg_hudGrenadeIconMaxHeight;
extern const dvar_t *cg_debugEvents;
extern const dvar_t *cg_hudGrenadeIconMaxRangeFlash;
extern const dvar_t *cg_deadHearAllLiving;
extern const dvar_t *cg_hudStanceFlash;
extern const dvar_t *cg_overheadNamesGlow;
extern const dvar_t *cg_drawScriptUsage;
extern const dvar_t *cg_crosshairDynamic;
extern const dvar_t *cg_hudDamageIconInScope;
extern const dvar_t *cg_drawFriendlyNames;
extern const dvar_t *cg_drawCrosshairNames;
extern const dvar_t *cg_gun_move_r;
extern const dvar_t *cg_constantSizeHeadIcons;
extern const dvar_t *cg_viewZSmoothingTime;
extern const dvar_t *cg_footsteps;
extern const dvar_t *cg_gun_move_f;
extern const dvar_t *cg_hudGrenadeIconHeight;
extern const dvar_t *cg_hudGrenadePointerWidth;
extern const dvar_t *cg_drawWVisDebug;
extern const dvar_t *cg_nopredict;
extern const dvar_t *cg_drawMaterial;
extern const dvar_t *cg_deadChatWithTeam;
extern const dvar_t *cg_firstPersonTracerChance;
extern const dvar_t *cg_synchronousClients;
extern const dvar_t *snd_drawInfo;
extern const dvar_t *cg_debug_overlay_viewport;
extern const dvar_t *cg_packetAnalysisEntTextY;
extern const dvar_t *cg_connectionIconSize;
extern const dvar_t *cg_dumpAnims;
extern const dvar_t *cg_errorDecay;
extern const dvar_t *cg_subtitleWidthStandard;
extern const dvar_t *cg_hudGrenadePointerPulseMin;
extern const dvar_t *cg_tracerScaleMinDist;
extern const dvar_t *cg_laserEndOffset;
extern const dvar_t *cg_drawMantleHint;
extern const dvar_t *cg_gameBoldMessageWidth;
extern const dvar_t *cg_tracerWidth;
extern const dvar_t *cg_drawSpectatorMessages;
extern const dvar_t *cg_everyoneHearsEveryone;
extern const dvar_t *cg_laserLightBodyTweak;
extern const dvar_t *cg_packetAnalysisTextScale;
extern const dvar_t *cg_drawCrosshairNamesPosY;
extern const dvar_t *cg_headIconMinScreenRadius;
extern const dvar_t *cg_chatTime;
extern const dvar_t *cg_drawFPSLabels;
extern const dvar_t *cg_drawpaused;
extern const dvar_t *cg_splitscreenSpectatorScaleIncrease;
extern const dvar_t *cg_hudDamageIconTime;
extern const dvar_t *cg_debugInfoCornerOffset;
extern const dvar_t *cg_blood;
extern const dvar_t *cg_teamChatsOnly;
extern const dvar_t *cg_weaponleftbone;
extern const dvar_t *cg_scriptIconSize;
extern const dvar_t *cg_showmiss;
extern const dvar_t *cg_predictItems;
extern const dvar_t *cg_deadChatWithDead;
extern const dvar_t *cg_descriptiveText;
extern const dvar_t *cg_fov;
extern const dvar_t *cg_hudSplitscreenCompassElementScale;
extern const dvar_t *cg_subtitleWidthWidescreen;
extern const dvar_t *cg_drawLagometer;
extern const dvar_t *cg_developer;
extern const dvar_t *cg_tracerScrewRadius;
extern const dvar_t *cg_hudGrenadePointerHeight;
extern const dvar_t *cg_gun_move_u;
extern const dvar_t *cg_hintFadeTime;
extern const dvar_t *cg_subtitleMinTime;
extern const dvar_t *cg_tracerScaleDistRange;
extern const dvar_t *cg_deadHearTeamLiving;
extern const dvar_t *cg_paused;
extern const dvar_t *cg_cursorHints;
extern const dvar_t *cg_thirdPersonAngle;
extern const dvar_t *cg_gun_ofs_f;
extern const dvar_t *cg_laserLightBeginOffset;
extern const dvar_t *cg_overheadIconSize;
extern const dvar_t *cg_tracerScrewDist;
extern const dvar_t *cg_tracerLength;
extern const dvar_t *cg_hudChatIntermissionPosition;
extern const dvar_t *cg_hudVotePosition;
extern const dvar_t *cg_fs_debug;

extern float cg_entityOriginArray[1][1024][3];
extern weaponInfo_s cg_weaponsArray[1][128];
extern cgMedia_t cgMedia;
extern centity_s cg_entitiesArray[1][1024];
extern cg_s cgArray[1];
extern cgs_t cgsArray[1];
extern UiContext cgDC[1];

extern vehicleEffects vehEffects[1][8];

extern uint16_t *wheelTags[4];
extern uint16_t *s_flashTags[];

inline centity_s *__cdecl CG_GetEntity(int32_t localClientNum, uint32_t entityIndex)
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

inline cgs_t *CG_GetLocalClientStaticGlobals(int32_t localClientNum)
{
    iassert(localClientNum == 0);

    return &cgsArray[localClientNum];
}

inline weaponInfo_s *__cdecl CG_GetLocalClientWeaponInfo(int localClientNum, int weaponIndex)
{
    iassert(localClientNum == 0);

    return &cg_weaponsArray[localClientNum][weaponIndex];
}

inline int CG_GetLocalClientConnectionState(int localClientNum)
{
    iassert(localClientNum == 0);

    return clientUIActives[0].connectionState;
}