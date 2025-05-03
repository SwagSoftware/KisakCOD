#pragma once

#include "cg_public_mp.h"

#include <cgame/cg_local.h>

#include <gfx_d3d/r_gfx.h>
#include <ui_mp/ui_mp.h>
#include <client_mp/client_mp.h>

#include <sound/snd_public.h>
#include <script/scr_const.h>

const float up[3] = { 0.0f, 0.0f, 1.0f };


struct tagInfo_s // sizeof=0x70
{
    gentity_s* parent;
    gentity_s* next;
    unsigned __int16 name;
    // padding byte
    // padding byte
    int index;
    float axis[4][3];
    float parentInvAxis[4][3];
};


struct gentity_s // sizeof=0x274
{                                       // ...
    entityState_s s;                    // ...
    entityShared_t r;                   // ...
    gclient_s* client;                  // ...
    turretInfo_s* pTurretInfo;
    scr_vehicle_s* scr_vehicle;
    unsigned __int16 model;
    unsigned __int8 physicsObject;
    unsigned __int8 takedamage;
    unsigned __int8 active;
    unsigned __int8 nopickup;
    unsigned __int8 handler;
    unsigned __int8 team;
    unsigned __int16 classname;         // ...
    unsigned __int16 target;
    unsigned __int16 targetname;
    // padding byte
    // padding byte
    unsigned int attachIgnoreCollision;
    int spawnflags;                     // ...
    int flags;                          // ...
    int eventTime;
    int freeAfterEvent;
    int unlinkAfterEvent;
    int clipmask;
    int processedFrame;
    EntHandle parent;
    int nextthink;
    int health;                         // ...
    int maxHealth;
    int damage;
    int count;
    gentity_s* chain;
    //$4FD1F2C094A0DF020529999C4E24827D ___u30;
    union //$4FD1F2C094A0DF020529999C4E24827D // sizeof=0x60
    {                                       // ...
        item_ent_t item[2];
        trigger_ent_t trigger;
        mover_ent_t mover;
        corpse_ent_t corpse;
        missile_ent_t missile;
    };
    EntHandle missileTargetEnt;
    tagInfo_s* tagInfo;
    gentity_s* tagChildren;
    unsigned __int16 attachModelNames[19]; // ...
    unsigned __int16 attachTagNames[19];
    int useCount;
    gentity_s* nextFree;
};

struct snapshot_s // sizeof=0x2307C
{                                       // ...
    int snapFlags;
    int ping;
    int serverTime;
    playerState_s ps;
    int numEntities;
    int numClients;
    entityState_s entities[512];
    clientState_s clients[64];
    int serverCommandSequence;
};

struct playerEntity_t // sizeof=0x30
{                                       // ...
    float fLastWeaponPosFrac;
    int bPositionToADS;
    float vPositionLastOrg[3];
    float fLastIdleFactor;
    float vLastMoveOrg[3];
    float vLastMoveAng[3];
};

struct score_t // sizeof=0x28
{                                       // ...
    int client;
    int score;
    int ping;
    int deaths;
    int team;
    int kills;
    int rank;
    int assists;
    Material* hStatusIcon;
    Material* hRankIcon;
};

struct cg_s // sizeof=0xFF580
{
    int clientNum;
    int localClientNum;
    DemoType demoType;
    CubemapShot cubemapShot;
    int cubemapSize;
    int renderScreen;
    int latestSnapshotNum;
    int latestSnapshotTime;
    snapshot_s* snap;
    snapshot_s* nextSnap;
    snapshot_s activeSnapshots[2];
    float frameInterpolation;
    int frametime;
    int time;
    int oldTime;
    int physicsTime;
    int mapRestart;
    int renderingThirdPerson;
    playerState_s predictedPlayerState;
    centity_s predictedPlayerEntity;
    playerEntity_t playerEntity;
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
    int iEntityLastType[1024];
    XModel* pEntityLastXModel[1024];
    float zoomSensitivity;
    bool isLoading;
    char objectiveText[1024];
    char scriptMainMenu[256];
    // padding byte
    // padding byte
    // padding byte
    int scoresRequestTime;
    int numScores;
    int teamScores[4];
    int teamPings[4];
    int teamPlayers[4];
    score_t scores[64];
    int scoreLimit;
    int showScores;
    int scoreFadeTime;
    int scoresTop;
    int scoresOffBottom;
    int scoresBottom;
    int drawHud;
    int crosshairClientNum;
    int crosshairClientLastTime;
    int crosshairClientStartTime;
    int identifyClientNum;
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
    // padding byte
    // padding byte
    // padding byte
    int healthOverlayLastHitTime;
    float healthOverlayOldHealth;
    int healthOverlayPulseIndex;
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
    float viewFade;
    int weapIdleTime;
    int nomarks;
    int v_dmg_time;
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
    int compassFadeTime;
    int healthFadeTime;
    int ammoFadeTime;
    int stanceFadeTime;
    int sprintFadeTime;
    int offhandFadeTime;
    int offhandFlashTime;
    shellshock_t shellshock;
    //cg_s::<unnamed_type_testShock> testShock;
    struct
    {
        int time;
        int duration;
    }testShock;
    int holdBreathTime;
    int holdBreathInTime;
    int holdBreathDelay;
    float holdBreathFrac;
    float radarProgress;
    float selectedLocation[2];
    SprintState sprintStates;
    int packetAnalysisFrameCount;
    unsigned __int8 bitsSent[100][13];
    int entBitsUsed[10][18];
    int numEntsSent[10][18];
    int numEntFields[10][18];
    int numSnapshots;
    int adsViewErrorDone;
    int inKillCam;
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
    int extraButtons;
    int lastActionSlotTime;
    bool playerTeleported;
    // padding byte
    // padding byte
    // padding byte
    int stepViewStart;
    float stepViewChange;
    //cg_s::<unnamed_type_lastFrame> lastFrame;
    struct
    {
        float aimSpreadScale;
    } lastFrame;

    hudElemSoundInfo_t hudElemSound[32];
    int vehicleFrame;
};

// cg_ents_mp
struct ComPrimaryLight;
struct GfxSceneEntity;

void __cdecl CG_Player_PreControllers(DObj_s *obj, centity_s *cent);
void __cdecl CG_mg42_PreControllers(DObj_s *obj, centity_s *cent);
void  CG_UpdateBModelWorldBounds(unsigned int localClientNum, centity_s *cent, int forceFilter);
bool __cdecl CG_VecLessThan(float *a, float *b);
void __cdecl CG_AdjustPositionForMover(
    int localClientNum,
    const float *in,
    int moverNum,
    int fromTime,
    int toTime,
    float *out,
    float *outDeltaAngles);
void __cdecl CG_SetFrameInterpolation(int localClientNum);
void __cdecl CG_ProcessClientNoteTracks(cg_s *cgameGlob, unsigned int clientNum);
void __cdecl CG_AddPacketEntity(int localClientNum, int entnum);
void __cdecl CG_UpdateClientDobjPartBits(centity_s *cent, int entnum, int localClientNum);
int __cdecl CG_AddPacketEntities(int localClientNum);
void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj_s *obj, bool notify);
int __cdecl CG_DObjGetWorldBoneMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    int boneIndex,
    float (*tagMat)[3],
    float *origin);
DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, int boneIndex);
int __cdecl CG_DObjGetWorldTagMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    unsigned int tagName,
    float (*tagMat)[3],
    float *origin);
DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj_s *obj, unsigned int tagName);
int __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj_s *obj, unsigned int tagName, float *pos);
cpose_t *__cdecl CG_GetPose(int localClientNum, unsigned int handle);
void __cdecl CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent);
void __cdecl CG_InterpolateEntityPosition(cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_CalcEntityPhysicsPositions(int localClientNum, centity_s *cent);
void __cdecl CG_CreatePhysicsObject(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePhysicsPose(centity_s *cent);
char __cdecl CG_ExpiredLaunch(int localClientNum, centity_s *cent);
void __cdecl CG_CalcEntityRagdollPositions(int localClientNum, centity_s *cent);
void __cdecl CG_CreateRagdollObject(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateRagdollPose(centity_s *cent);
DObj_s *__cdecl CG_PreProcess_GetDObj(int localClientNum, int entIndex, int entType, XModel *model);
XAnim_s *__cdecl CG_GetAnimations(int localClientNum, unsigned int entIndex, int entType);
XAnim_s *__cdecl CG_GetMG42Anims(centity_s *cent);
XAnim_s *__cdecl CG_GetHelicopterAnims(centity_s *cent);
char *__cdecl CG_AllocAnimTree(int size);
void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj_s *obj, int boneIndex);
void __cdecl CG_ClearUnion(int localClientNum, centity_s *cent);
void __cdecl CG_SetUnionType(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePoseUnion(int localClientNum, centity_s *cent);
void __cdecl CG_ProcessEntity(int localClientNum, centity_s *cent);
void __cdecl CG_General(int localClientNum, centity_s *cent);
void __cdecl CG_LockLightingOrigin(centity_s *cent, float *lightingOrigin);
void __cdecl CG_Item(int localClientNum, centity_s *cent);
void __cdecl CG_EntityEffects(int localClientNum, centity_s *cent);
void __cdecl CG_AddEntityLoopSound(int localClientNum, const centity_s *cent);
void __cdecl CG_mg42(int localClientNum, centity_s *cent);
void __cdecl CG_Missile(int localClientNum, centity_s *cent);
void __cdecl CG_ScriptMover(int localClientNum, centity_s *cent);
void __cdecl CG_SoundBlend(int localClientNum, centity_s *cent);
void __cdecl CG_Fx(int localClientNum, centity_s *cent);
FxEffect *__cdecl CG_StartFx(int localClientNum, centity_s *cent, int startAtTime);
void __cdecl CG_LoopFx(int localClientNum, centity_s *cent);
void __cdecl CG_PrimaryLight(int localClientNum, centity_s *cent);
const ComPrimaryLight *__cdecl Com_GetPrimaryLight(unsigned int primaryLightIndex);
void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight);
void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight);
void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin);
void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles);
float *__cdecl CG_GetEntityOrigin(int localClientNum, unsigned int entnum);
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
    int viewX;
    int viewY;
    int viewWidth;
    int viewHeight;
    float viewAspect;
    int serverCommandSequence;
    int processedSnapshotNum;
    int localServer;
    char gametype[32];
    char szHostName[256];
    int maxclients;
    char mapname[64];
    int gameEndTime;
    int voteTime;
    int voteYes;
    int voteNo;
    char voteString[256];
    XModel *gameModels[512];
    const FxEffectDef *fxs[100];
    const FxEffectDef *smokeGrenadeFx;
    shellshock_parms_t holdBreathParams;
    char teamChatMsgs[8][160];
    int teamChatMsgTimes[8];
    int teamChatPos;
    int teamLastChatPos;
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
void __cdecl CG_GetDObjOrientation(int localClientNum, unsigned int dobjHandle, float (*axis)[3], float *origin);
void __cdecl CG_GetSoundEntityOrientation(SndEntHandle sndEnt, float *origin_out, float (*axis_out)[3]);
void __cdecl CG_CopyEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3]);
const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum);
void __cdecl CG_GameMessage(int localClientNum, const char *msg);
void __cdecl CG_BoldGameMessage(int localClientNum, const char *msg);
void __cdecl CG_RegisterSounds();
void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, snd_alias_list_t **sound);
int CG_RegisterPhysicsSounds();
int CG_RegisterPhysicsSounds_FastFile();
void __cdecl CG_AddAudioPhysicsClass(PhysPreset *physPreset, char (*classes)[64], int *nclasses);
void __cdecl CG_StartAmbient(int localClientNum);
int __cdecl CG_PlayClientSoundAlias(int localClientNum, snd_alias_list_t *aliasList);
int __cdecl CG_PlayClientSoundAliasByName(int localClientNum, const char *aliasname);
int __cdecl CG_PlayEntitySoundAlias(int localClientNum, int entitynum, snd_alias_list_t *aliasList);
void __cdecl CG_StopSoundAlias(int localClientNum, int entitynum, snd_alias_list_t *aliasList);
void __cdecl CG_StopSoundsOnEnt(int localClientNum, int entitynum);
void __cdecl CG_StopSoundAliasByName(int localClientNum, int entityNum, const char *aliasName);
void __cdecl CG_StopClientSoundAliasByName(int localClientNum, const char *aliasName);
void __cdecl CG_SubtitleSndLengthNotify(int msec, const snd_alias_t *lengthNotifyData);
void __cdecl CG_SubtitlePrint(int msec, const snd_alias_t *alias);
void __cdecl CG_AddFXSoundAlias(int localClientNum, const float *origin, snd_alias_list_t *aliasList);
int __cdecl CG_PlaySoundAlias(int localClientNum, int entitynum, const float *origin, snd_alias_list_t *aliasList);
int __cdecl CG_PlaySoundAliasByName(int localClientNum, int entitynum, const float *origin, const char *aliasname);
int __cdecl CG_PlaySoundAliasAsMasterByName(
    int localClientNum,
    int entitynum,
    const float *origin,
    const char *aliasname);
void __cdecl CG_RestartSmokeGrenades(int localClientNum);
void __cdecl CG_InitVote(int localClientNum);
unsigned __int16 __cdecl CG_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType);
int __cdecl CG_GetClientNum(int localClientNum);
void __cdecl CG_Init(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum);
clientConnection_t *__cdecl CL_GetLocalClientConnection(int localClientNum);
void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname);
int __cdecl CG_PlayAnimScriptSoundAlias(int clientIndex, snd_alias_list_t *aliasList);
void __cdecl CG_LoadHudMenu(int localClientNum);
unsigned __int16 __cdecl CG_AttachWeapon(DObjModel_s *dobjModels, unsigned __int16 numModels, clientInfo_t *ci);
void __cdecl CG_CreateDObj(
    DObjModel_s *dobjModels,
    unsigned __int16 numModels,
    XAnimTree_s *tree,
    unsigned int handle,
    int localClientNum,
    clientInfo_t *ci);
DObj_s *__cdecl CG_GetDObj(unsigned int handle, int localClientNum);
void __cdecl CG_LoadAnimTreeInstances(int localClientNum);
void __cdecl CG_InitEntities(int localClientNum);
void __cdecl CG_InitViewDimensions(int localClientNum);
void __cdecl CG_InitDof(GfxDepthOfField *dof);
void __cdecl CG_FreeWeapons(int localClientNum);
void __cdecl CG_Shutdown(int localClientNum);
void __cdecl CG_FreeAnimTreeInstances(int localClientNum);
unsigned __int8 *__cdecl Hunk_AllocXAnimClient(unsigned int size);
unsigned __int8 __cdecl CG_ShouldPlaySoundOnLocalClient();

// cg_newDraw_mp
struct MemoryFile;
void __cdecl CG_AntiBurnInHUD_RegisterDvars();
bool __cdecl CG_ShouldDrawHud(int localClientNum);
double __cdecl CG_FadeHudMenu(int localClientNum, const dvar_s *fadeDvar, int displayStartTime, int duration);
bool __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, unsigned int weapIndex);
bool __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob);
bool __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, unsigned int weapIndex);
bool __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob);
double __cdecl CG_CalcPlayerHealth(const playerState_s *ps);
void __cdecl CG_ResetLowHealthOverlay(cg_s *cgameGlob);
int __cdecl CG_ServerMaterialName(int localClientNum, int index, char *materialName, unsigned int maxLen);
Material *__cdecl CG_ObjectiveIcon(int localClientNum, int icon, int type);
const char *__cdecl CG_ScriptMainMenu(int localClientNum);
void __cdecl CG_OwnerDraw(
    int localClientNum,
    rectDef_s parentRect,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float text_x,
    float text_y,
    int ownerDraw,
    int ownerDrawFlags,
    int align,
    float special,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle,
    char textAlignMode);
void __cdecl CG_DrawPlayerAmmoBackdrop(
    int localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerAmmoValue(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle);
void __cdecl CG_DrawPlayerWeaponName(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle);
void __cdecl CG_DrawPlayerWeaponNameBack(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerStance(
    int localClientNum,
    const rectDef_s *rect,
    const float *color,
    Font_s *font,
    float scale,
    int textStyle);
void __cdecl CG_DrawStanceIcon(
    int localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    float x,
    float y,
    float fadeAlpha);
void __cdecl CG_DrawStanceHintPrints(
    int localClientNum,
    const rectDef_s *rect,
    float x,
    const float *color,
    float fadeAlpha,
    Font_s *font,
    float scale,
    int textStyle);
void __cdecl CG_DrawPlayerSprintBack(int localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_DrawPlayerSprintMeter(int localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, float *color);
void __cdecl CG_DrawPlayerBarHealth(int localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_DrawPlayerBarHealthBack(int localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_DrawPlayerLowHealthOverlay(int localClientNum, const rectDef_s *rect, Material *material, float *color);
double __cdecl CG_FadeLowHealthOverlay(const cg_s *cgameGlob);
void __cdecl CG_PulseLowHealthOverlay(cg_s *cgameGlob, float healthRatio);
void __cdecl CG_DrawCursorhint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int textStyle);
void __cdecl CG_UpdateCursorHints(cg_s *cgameGlob);
char *__cdecl CG_GetWeaponUseString(int localClientNum, const char **secondaryString);
char *__cdecl CG_GetUseString(int localClientNum);
void __cdecl CG_DrawHoldBreathHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    int textStyle);
void __cdecl CG_DrawMantleHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    const float *color,
    int textStyle);
void __cdecl CG_DrawInvalidCmdHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int textStyle);
void __cdecl CG_DrawTalkerNum(
    int localClientNum,
    int num,
    rectDef_s *rect,
    Font_s *font,
    float *color,
    float textScale,
    int style);
void __cdecl CG_ArchiveState(int localClientNum, MemoryFile *memFile);



// cg_draw_mp
struct OverheadFade // sizeof=0xC
{                                       // ...
    int lastTime;                       // ...
    int startTime;                      // ...
    bool visible;                       // ...
    // padding byte
    // padding byte
    // padding byte
};
void __cdecl TRACK_cg_draw();
void __cdecl CG_PriorityCenterPrint(int localClientNum, const char *str, int priority);
void __cdecl CG_ClearCenterPrint(int localClientNum);
void __cdecl CG_DrawCenterString(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int textStyle);
void __cdecl CG_ClearOverheadFade();
void __cdecl CG_Draw2D(int localClientNum);
void __cdecl CG_DrawChatMessages(int localClientNum);
void __cdecl CG_ScanForCrosshairEntity(int localClientNum);
void __cdecl CG_CheckTimedMenus(int localClientNum);
void __cdecl CG_CheckForPlayerInput(int localClientNum);
bool __cdecl CG_CheckPlayerMovement(usercmd_s oldCmd, usercmd_s newCmd);
int __cdecl CG_CheckPlayerStanceChange(int localClientNum, __int16 newButtons, __int16 changedButtons);
int __cdecl CG_CheckPlayerWeaponUsage(int localClientNum, char buttons);
bool __cdecl CG_CheckPlayerTryReload(int localClientNum, char buttons);
bool __cdecl CG_CheckPlayerFireNonTurret(int localClientNum, char buttons);
int __cdecl CG_CheckPlayerOffHandUsage(int localClientNum, __int16 buttons);
unsigned int __cdecl CG_CheckPlayerMiscInput(int buttons);
void __cdecl CG_CheckHudHealthDisplay(int localClientNum);
void __cdecl CG_CheckHudAmmoDisplay(int localClientNum);
void __cdecl CG_CheckHudCompassDisplay(int localClientNum);
void __cdecl CG_CheckHudStanceDisplay(int localClientNum);
void __cdecl CG_CheckHudSprintDisplay(int localClientNum);
void __cdecl CG_CheckHudOffHandDisplay(int localClientNum);
void __cdecl CG_CheckHudObjectiveDisplay(int localClientNum);
void __cdecl CG_DrawMiniConsole(int localClientNum);
void __cdecl CG_DrawErrorMessages(int localClientNum);
void __cdecl CG_DrawSay(int localClientNum);
void __cdecl CG_DrawVote(int localClientNum);
void __cdecl DrawIntermission(int localClientNum);
void __cdecl CG_DrawSpectatorMessage(int localClientNum);
int __cdecl CG_DrawFollow(int localClientNum);
void __cdecl CG_UpdatePlayerNames(int localClientNum);
void __cdecl CG_DrawFriendlyNames(int localClientNum);
void __cdecl CG_DrawOverheadNames(int localClientNum, const centity_s *cent, float alpha);
char __cdecl CG_CalcNamePosition(int localClientNum, float *origin, float *xOut, float *yOut);
double __cdecl CG_FadeCrosshairNameAlpha(int time, int startMsec, int lastMsec, int fadeInMsec, int fadeOutMsec);
bool __cdecl CG_CanSeeFriendlyHead(int localClientNum, const centity_s *cent);
void __cdecl CG_DrawCrosshairNames(int localClientNum);
void __cdecl DrawViewmodelInfo(int localClientNum);
void __cdecl CG_DrawActive(int localClientNum);
void __cdecl CG_AddSceneTracerBeams(int localClientNum);
void __cdecl CG_GenerateSceneVerts(int localClientNum);
void __cdecl CG_GetViewAxisProjections(const refdef_s *refdef, const float *worldPoint, float *projections);


// cg_players_mp
void __cdecl CG_AddAllPlayerSpriteDrawSurfs(int localClientNum);
void __cdecl CG_AddPlayerSpriteDrawSurfs(int localClientNum, const centity_s *cent);
void  CG_AddPlayerSpriteDrawSurf(
    int localClientNum,
    const centity_s *cent,
    Material *material,
    float additionalRadiusSize,
    int height,
    bool fixedScreenSize);
void __cdecl CG_Player(int localClientNum, centity_s *cent);
void __cdecl CG_PlayerTurretPositionAndBlend(int localClientNum, centity_s *cent);
void __cdecl CG_Corpse(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePlayerDObj(int localClientNum, centity_s *cent);
void __cdecl CG_ResetPlayerEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent, int resetAnimation);
const char *__cdecl CG_GetTeamName(team_t team);
const char *__cdecl CG_GetOpposingTeamName(team_t team);
const char *__cdecl CG_GetPlayerTeamName(int localClientNum);
const char *__cdecl CG_GetPlayerOpposingTeamName(int localClientNum);
bool __cdecl CG_IsPlayerDead(int localClientNum);
int __cdecl CG_GetPlayerClipAmmoCount(int localClientNum);
void __cdecl CG_UpdateWeaponVisibility(int localClientNum, centity_s *cent);
bool __cdecl CG_IsWeaponVisible(int localClientNum, centity_s *cent, XModel *weapModel, float *origin, float *forward);
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
    int iAlignment;
};
void __cdecl UpdateScores(int localClientNum);
const score_t *__cdecl UI_GetOurClientScore(int localClientNum);
const score_t *__cdecl GetClientScore(int localClientNum, int clientNum);
const score_t *__cdecl UI_GetScoreAtRank(int localClientNum, int rank);
char *__cdecl CG_GetGametypeDescription(int localClientNum);
char __cdecl CG_DrawScoreboard_GetTeamColorIndex(int team, int localClientNum);
int __cdecl CG_DrawScoreboard(int localClientNum);
void __cdecl CG_DrawScoreboard_Backdrop(int localClientNum, float alpha);
double __cdecl CG_BackdropLeft(int localClientNum);
double __cdecl CG_BackdropTop();
double __cdecl CG_BannerScoreboardScaleMultiplier();
void __cdecl CG_DrawScoreboard_ScoresList(int localClientNum, float alpha);
double __cdecl CG_DrawScoreboard_ListColumnHeaders(
    int localClientNum,
    const float *color,
    float y,
    float h,
    float listWidth);
void __cdecl CG_GetScoreboardInfo(const listColumnInfo_t **colInfo, int *numFields);
int __cdecl CG_ScoreboardTotalLines(int localClientNum);
double __cdecl CG_DrawTeamOfClientScore(
    int localClientNum,
    const float *color,
    float y,
    int team,
    float listWidth,
    int *drawLine);
int __cdecl CG_CheckDrawScoreboardLine(int localClientNum, int *drawLine, float y, float lineHeight);
double __cdecl CG_DrawScoreboard_ListBanner(
    int localClientNum,
    const float *color,
    float y,
    float w,
    float h,
    int team,
    int *piDrawLine);
double __cdecl CG_DrawClientScore(
    int localClientNum,
    const float *color,
    float y,
    const score_t *score,
    float listWidth);
double __cdecl CalcXAdj(int align, float maxw, float w);
void __cdecl DrawListString(
    int localClientNum,
    char *string,
    float x,
    float y,
    float width,
    int alignment,
    Font_s *font,
    float scale,
    int style,
    const float *color);
void __cdecl CG_DrawClientPing(int localClientNum, int ping, float x, float y, float maxWidth, float maxHeight);
void __cdecl CG_DrawScrollbar(int localClientNum, const float *color, float top);
void __cdecl CenterViewOnClient(int localClientNum);
int __cdecl CG_IsScoreboardDisplayed(int localClientNum);
void __cdecl CG_ScrollScoreboardUp(cg_s *cgameGlob);
void __cdecl CG_ScrollScoreboardDown(cg_s *cgameGlob);
void __cdecl CG_RegisterScoreboardDvars();
void __cdecl CG_RegisterScoreboardGraphics();
bool __cdecl Scoreboard_HandleInput(int localClientNum, int key);


// cg_vehicles_mp
struct vehicleEffects // sizeof=0x28
{
    bool active;
    // padding byte
    // padding byte
    // padding byte
    int lastAccessed;
    int entityNum;
    int nextDustFx;
    int nextSmokeFx;
    bool soundPlaying;
    // padding byte
    // padding byte
    // padding byte
    float barrelVelocity;
    float barrelPos;
    int lastBarrelUpdateTime;
    unsigned __int8 tag_engine_left;
    unsigned __int8 tag_engine_right;
    // padding byte
    // padding byte
};
struct vehfx_t // sizeof=0x48
{                                       // ...
    bool tireActive[4];
    float tireGroundPoint[4][3];
    unsigned __int8 tireGroundSurfType[4];
    bool soundEnabled;
    // padding byte
    // padding byte
    // padding byte
    float soundEngineOrigin[3];
};
void __cdecl CG_VehRegisterDvars();
DObj_s *__cdecl GetVehicleEntDObj(int localClientNum, centity_s *centVeh);
void __cdecl CG_VehGunnerPOV(int localClientNum, float *resultOrigin, float *resultAngles);
clientInfo_t *__cdecl ClientInfoForLocalClient(int localClientNum);
void __cdecl GetTagMatrix(
    int localClientNum,
    unsigned int vehEntNum,
    unsigned __int16 tagName,
    //float (*resultTagMat)[3],
    mat3x3 &resultTagMat,
    float *resultOrigin);
bool __cdecl CG_VehLocalClientUsingVehicle(int localClientNum);
bool __cdecl CG_VehLocalClientDriving(int localClientNum);
bool __cdecl CG_VehEntityUsingVehicle(int localClientNum, unsigned int entNum);
clientInfo_t *__cdecl ClientInfoForEntity(int localClientNum, unsigned int entNum);
int __cdecl CG_VehLocalClientVehicleSlot(int localClientNum);
int __cdecl CG_VehPlayerVehicleSlot(int localClientNum, unsigned int entNum);
void __cdecl CG_VehSeatTransformForPlayer(
    int localClientNum,
    unsigned int entNum,
    float *resultOrigin,
    float *resultAngles);
void __cdecl SeatTransformForClientInfo(int localClientNum, clientInfo_t *ci, float *resultOrigin, float *resultAngles);
void __cdecl SeatTransformForSlot(
    int localClientNum,
    unsigned int vehEntNum,
    unsigned int vehSlotIdx,
    float *resultOrigin,
    float *resultAngles);
void __cdecl CG_VehSeatOriginForLocalClient(int localClientNum, float *result);
double __cdecl Veh_GetTurretBarrelRoll(int localClientNum, centity_s *cent);
int __cdecl CG_GetEntityIndex(int localClientNum, const centity_s *cent);
vehicleEffects *__cdecl VehicleGetFxInfo(int localClientNum, int entityNum);
void __cdecl Veh_IncTurretBarrelRoll(int localClientNum, int entityNum, float rotation);
void __cdecl CG_VehProcessEntity(int localClientNum, centity_s *cent);
void __cdecl SetupPoseControllers(int localClientNum, DObj_s *obj, centity_s *cent, vehfx_t *fxInfo);
unsigned __int16 __cdecl CompressUnit(float unit);
void __cdecl VehicleFXTest(int localClientNum, const DObj_s *obj, centity_s *cent, vehfx_t *fxInfo);
double __cdecl GetSpeed(int localClientNum, centity_s *cent);
void __cdecl CG_VehSphereCoordsToPos(float sphereDistance, float sphereYaw, float sphereAltitude, float *result);
void __cdecl CG_Veh_Init();


// cg_snapshot_mp
void __cdecl CG_ShutdownEntity(int localClientNum, centity_s *cent);
void __cdecl CG_SetInitialSnapshot(int localClientNum, snapshot_s *snap);
void __cdecl CG_SetNextSnap(int localClientNum, snapshot_s *snap);
void __cdecl CG_ResetEntity(int localClientNum, centity_s *cent, int newEntity);
void __cdecl CG_CopyCorpseInfo(clientInfo_t *corpseInfo, const clientInfo_t *ci);
void __cdecl CG_TransitionKillcam(int localClientNum);
void __cdecl CG_ProcessSnapshots(int localClientNum);
void __cdecl CG_TransitionSnapshot(int localClientNum);
snapshot_s *__cdecl CG_ReadNextSnapshot(int localClientNum);
void __cdecl CG_ExtractTransPlayerState(const playerState_s *ps, transPlayerState_t *transPs);


// cg_servercmds_mp
void __cdecl CG_ParseServerInfo(int localClientNum);
void __cdecl CG_ParseCodInfo(int localClientNum);
void __cdecl CG_ParseFog(int localClientNum);
void __cdecl CG_SetConfigValues(int localClientNum);
void __cdecl CG_ParseGameEndTime(int localClientNum);
void __cdecl CG_PrecacheScriptMenu(int localClientNum, int configStringIndex);
void __cdecl CG_RegisterServerMaterial(int localClientNum, int configStringIndex);
void __cdecl CG_MapRestart(int localClientNum, int savepersist);
void __cdecl CG_ClearEntityFxHandles(int localClientNum);
void __cdecl CG_CheckOpenWaitingScriptMenu(int localClientNum);
void __cdecl CG_CloseScriptMenu(int localClientNum, bool allowResponse);
void __cdecl CG_MenuShowNotify(int localClientNum, int menuToShow);
void __cdecl CG_ServerCommand(int localClientNum);
void __cdecl CG_DeployServerCommand(int localClientNum);
void __cdecl CG_ParseScores(int localClientNum);
void __cdecl CG_SetSingleClientScore(int localClientNum, int clientIndex, int newScore);
void __cdecl CG_SortSingleClientScore(cg_s *cgameGlob, int scoreIndex);
bool __cdecl CG_ClientScoreIsBetter(score_t *scoreA, score_t *scoreB);
void __cdecl CG_ConfigStringModified(int localClientNum);
void __cdecl CG_UpdateVoteString(int localClientNum, const char *rawVoteString);
void __cdecl CG_AddToTeamChat(int localClientNum, const char *str);
void __cdecl CG_OpenScriptMenu(int localClientNum);
void __cdecl CG_RemoveChatEscapeChar(char *text);
void __cdecl CG_SetTeamScore(int localClientNum, unsigned int team, int score);
void CG_ReverbCmd();
void CG_DeactivateReverbCmd();
void __cdecl CG_SetChannelVolCmd(int localClientNum);
void CG_DeactivateChannelVolCmd();
char __cdecl LocalSound(int localClientNum);
void __cdecl LocalSoundStop(int localClientNum);
void __cdecl CG_SetClientDvarFromServer(cg_s *cgameGlob, const char *dvarname, char *value);
void __cdecl CG_SetObjectiveText(cg_s *cgameGlob, char *text);
void __cdecl CG_SetDrawHud(cg_s *cgameGlob, unsigned int value);
void __cdecl CG_SetScriptMainMenu(cg_s *cgameGlob, char *text);
void __cdecl CG_ExecuteNewServerCommands(int localClientNum, int latestSequence);



// cg_predict_mp
void __cdecl TRACK_cg_predict();
int __cdecl CG_ItemListLocalClientNum();
void __cdecl CG_ClearItemList();
void __cdecl CG_BuildItemList(int localClientNum, const snapshot_s *nextSnap);
void __cdecl CG_PredictPlayerState(int localClientNum);
void __cdecl CG_PredictPlayerState_Internal(int localClientNum);
void __cdecl CG_TouchItemPrediction(int localClientNum);
void __cdecl CG_TouchItem(cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles);



// cg_pose_mp
void __cdecl BG_Player_DoControllers(const CEntPlayerInfo *player, const DObj_s *obj, int *partBits);
void  CG_VehPoseControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
void __cdecl CG_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
void __cdecl CG_Player_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj_s *obj, int *partBits);
void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj_s *obj, int *setPartBits);
DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *pose, const DObj_s *obj, int *partBits);



// cg_draw_net_mp
void __cdecl CG_AddLagometerFrameInfo(const cg_s *cgameGlob);
void __cdecl CG_AddLagometerSnapshotInfo(snapshot_s *snap);
void __cdecl CG_DrawSnapshotAnalysis(int localClientNum);
int __cdecl CG_ComparePacketAnalysisSamples(int *a, int *b);
void __cdecl CG_DrawSnapshotEntityAnalysis(int localClientNum);
int __cdecl CG_CompareEntityAnalysisSamples(unsigned int *a, unsigned int *b);
void __cdecl CG_DrawPingAnalysis(int localClientNum);
void __cdecl CG_DrawLagometer(int localClientNum);
void __cdecl CG_DrawDisconnect(int localClientNum);



struct CompassVehicle // sizeof=0x1C
{                                       // ...
    int entityNum;                      // ...
    int lastUpdate;                     // ...
    float lastPos[2];
    float lastYaw;
    team_t team;
    int ownerIndex;
};
struct CompassActor // sizeof=0x30
{                                       // ...
    int lastUpdate;
    float lastPos[2];
    float lastEnemyPos[2];
    float lastYaw;
    int pingTime;
    int beginFadeTime;
    int beginRadarFadeTime;
    int beginVoiceFadeTime;
    bool enemy;
    // padding byte
    // padding byte
    // padding byte
    int perks;
};
void __cdecl TRACK_cg_compassfriendlies();
void __cdecl CG_ClearCompassPingData();
void __cdecl CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex);
CompassVehicle *__cdecl GetVehicle(int localClientNum, int entityNum);
void __cdecl CG_CompassRadarPingEnemyPlayers(int localClientNum, float oldRadarProgress, float newRadarProgress);
void __cdecl GetRadarLine(cg_s *cgameGlob, float radarProgress, float *line);
double __cdecl GetRadarLineMargin(cg_s *cgameGlob);
bool __cdecl DoLinesSurroundPoint(cg_s *cgameGlob, float *radarLine1, float *radarLine2, float *pos);
void __cdecl RadarPingEnemyPlayer(CompassActor *actor, int time);
void __cdecl CG_CompassIncreaseRadarTime(int localClientNum);
void __cdecl CG_CompassAddWeaponPingInfo(int localClientNum, const centity_s *cent, const float *origin, int msec);
void __cdecl ActorUpdatePos(int localClientNum, CompassActor *actor, const float *newPos, int actorClientIndex);
bool __cdecl DoesMovementCrossRadar(cg_s *cgameGlob, float radarProgress, const float *p1, const float *p2);
bool __cdecl CanLocalPlayerHearActorFootsteps(int localClientNum, const float *actorPos, unsigned int actorClientIndex);
void __cdecl CG_CompassUpdateActors(int localClientNum);
void __cdecl CG_CompassDrawFriendlies(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color);
void __cdecl CG_CompassDrawEnemies(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color);
void __cdecl CG_CompassDrawRadarEffects(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color);
double __cdecl GetRadarLineEastWestPercentage(cg_s *cgameGlob, float radarProgress);
void __cdecl CG_CompassDrawVehicles(
    int localClientNum,
    CompassType compassType,
    int eType,
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
void __cdecl CG_StartClientSideEffects(int localClientNum);
void __cdecl CG_LoadClientEffects(int localClientNum, const char *filename);
void __cdecl CG_ParseClientEffects(int localClientNum, char *buffer);
const char *__cdecl CG_SkipLine(char *line, const char *skipLine);
const char *__cdecl CG_SkipWhiteSpace(const char *line);
char *__cdecl CG_SkipText(char *line, const char *skipText);
const char *__cdecl CG_SkipLineStartingWith(char *line, const char *skipLine);
const char *__cdecl CG_SkipRestOfLine(const char *line);
bool __cdecl CG_MatchLineStartingWith(const char *line, const char *startLine);
const char *__cdecl CG_ParseSound(int localClientNum, char *line);
void __cdecl CG_AddClientEntSound(const float *origin, const char *soundalias);
const char *__cdecl CG_ParseVec3Finish(char *line, float *origin);
const char *__cdecl CG_ParseStringFinish(char *line, char *text, unsigned int bufferSize);
char *__cdecl CG_ParseString(char *line, char *text, unsigned int bufferSize);
char *__cdecl CG_ParseEffect(int localClientNum, char *line);
const char *__cdecl CG_ParseFloatFinish(char *line, float *value);
char __cdecl CG_FindFileName(const char *name, char *filename, int size);
void __cdecl CG_LoadClientEffects_FastFile(int localClientNum, const char *filename);
void __cdecl CG_LoadClientEffectMapping(const char *filename);
void __cdecl CG_ParseClientEffectMapping(const char *buffer);
void __cdecl CG_AddPairToMap(char *name, char *filename);
void __cdecl CG_LoadClientEffectMapping_FastFile(const char *filename);
void __cdecl CG_ClientSideEffectsRegisterDvars();
void __cdecl CG_AddClientSideSounds(int localClientNum);
void __cdecl CG_AddClientSideSound(int localClientNum, int index, const ClientEntSound *sound);
void __cdecl CG_CopyClientSideSoundEntityOrientation(
    unsigned int clientSoundEntIndex,
    float *origin_out,
    float (*axis_out)[3]);


// cg_animtree_mp
void __cdecl CGScr_LoadAnimTrees();
void __cdecl CG_FreeClientDObjInfo(int localClientNum);
void __cdecl CG_SetDObjInfo(int localClientNum, int iEntNum, int iEntType, XModel *pXModel);
bool __cdecl CG_CheckDObjInfoMatches(int localClientNum, int iEntNum, int iEntType, XModel *pXModel);
void __cdecl CG_SafeDObjFree(int localClientNum, int entIndex);
void __cdecl CG_FreeEntityDObjInfo(int localClientNum);


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
    int time;
    int respawnTime;
};

void __cdecl CL_SyncGpu(int(__cdecl *WorkCallback)(unsigned __int64));

void __cdecl TRACK_cg_view();
void __cdecl CG_FxSetTestPosition();
void __cdecl CG_FxTest();
void __cdecl CG_PlayTestFx(int localClientNum);
double __cdecl CG_GetViewFov(int localClientNum);
void __cdecl CG_ViewRegisterDvars();
void __cdecl CG_UpdateHelicopterKillCam(int localClientNum);
void __cdecl CG_UpdateFov(int localClientNum, float fov_x);
void __cdecl CG_UpdateHelicopterKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_UpdateAirstrikeKillCam(int localClientNum);
void __cdecl CG_UpdateAirstrikeKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_InitView(int localClientNum);
void __cdecl CG_CalcViewValues(int localClientNum);
void __cdecl CG_OffsetThirdPersonView(cg_s *cgameGlob);
void __cdecl ThirdPersonViewTrace(cg_s *cgameGlob, float *start, float *end, int contentMask, float *result);
void __cdecl CG_CalcVrect(int localClientNum);
void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob);
void __cdecl CG_OffsetFirstPersonView(cg_s *cgameGlob);
void __cdecl CG_CalcFov(int localClientNum);
void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob);
void __cdecl CG_CalcTurretViewValues(int localClientNum);
void __cdecl CG_ApplyViewAnimation(int localClientNum);
void __cdecl CalcViewValuesVehicle(int localClientNum);
void CalcViewValuesVehicleDriver(int localClientNum);
void CalcViewValuesVehiclePassenger(int localClientNum);
void __cdecl CalcViewValuesVehicleGunner(int localClientNum);
bool __cdecl CG_HelicopterKillCamEnabled(int localClientNum);
bool __cdecl CG_AirstrikeKillCamEnabled(int localClientNum);
void __cdecl CG_UpdateThirdPerson(int localClientNum);
bool __cdecl CG_KillCamEntityEnabled(int localClientNum);
const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int localClientNum);
void __cdecl CG_UpdateViewOffset(int localClientNum);
void __cdecl CG_UpdateKillCamEntityViewOffset(int localClientNum);
int __cdecl CG_DrawActiveFrame(
    int localClientNum,
    int serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int cubemapSize,
    int renderScreen);
void __cdecl CG_UpdateTestFX(int localClientNum);
void __cdecl CG_KickAngles(cg_s *cgameGlob);
void __cdecl CG_UpdateEntInfo(int localClientNum);
void __cdecl GetCeilingHeight(cg_s *cgameGlob);
void __cdecl DumpAnims(int localClientNum);
void __cdecl DrawShellshockBlend(int localClientNum);
void __cdecl CG_UpdateSceneDepthOfField(int localClientNum);
void __cdecl CG_UpdateAdsDof(int localClientNum, GfxDepthOfField *dof);
double __cdecl CG_UpdateAdsDofValue(float currentValue, float targetValue, float maxChange, float dt);


// cg_consolecmds_mp
void __cdecl CG_ScoresUp(int localClientNum);
void __cdecl CG_InitConsoleCommands();
void __cdecl CG_Viewpos_f();
void __cdecl CG_ScoresUp_f();
void __cdecl CG_ScoresDown_f();
void __cdecl CG_ScoresDown(int localClientNum);
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

extern const unsigned __int16 *wheelTags[4];
extern unsigned __int16 *s_flashTags[];

inline centity_s *__cdecl CG_GetEntity(int localClientNum, unsigned int entityIndex)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\aim_assist\\../cgame_mp/cg_local_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (entityIndex >= 0x400)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\aim_assist\\../cgame_mp/cg_local_mp.h",
            1113,
            0,
            "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entityIndex,
            1024);
    return &cg_entitiesArray[0][entityIndex];
}