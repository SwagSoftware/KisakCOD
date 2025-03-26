#pragma once
#include "cg_public_mp.h"
#include <client_mp/client_mp.h>
#include <gfx_d3d/r_scene.h>


struct clientControllers_t // sizeof=0x60
{                                       // ...
    float angles[6][3];
    float tag_origin_angles[3];
    float tag_origin_offset[3];
};
struct __declspec(align(4)) CEntPlayerInfo // sizeof=0xC
{                                       // ...
    clientControllers_t *control;       // ...
    unsigned __int8 tag[6];             // ...
    // padding byte
    // padding byte
};
struct CEntTurretAngles // sizeof=0x8
{                                       // ...
    float pitch;
    float yaw;
};
union $06707BB8B07567CD0B9AB50B15101D6E // sizeof=0x8
{                                       // ...
    CEntTurretAngles angles;
    const float *viewAngles;
};
struct CEntTurretInfo // sizeof=0x10
{                                       // ...
    $06707BB8B07567CD0B9AB50B15101D6E ___u0;
    float barrelPitch;
    bool playerUsing;
    unsigned __int8 tag_aim;
    unsigned __int8 tag_aim_animated;
    unsigned __int8 tag_flash;
};
struct __declspec(align(2)) CEntVehicleInfo // sizeof=0x24
{                                       // ...
    __int16 pitch;
    __int16 yaw;
    __int16 roll;
    __int16 barrelPitch;
    float barrelRoll;
    __int16 steerYaw;
    // padding byte
    // padding byte
    float time;
    unsigned __int16 wheelFraction[4];
    unsigned __int8 wheelBoneIndex[4];
    unsigned __int8 tag_body;
    unsigned __int8 tag_turret;
    unsigned __int8 tag_barrel;
    // padding byte
};

struct CEntFx // sizeof=0x8
{                                       // ...
    int triggerTime;
    FxEffect *effect;
};

struct GfxSkinCacheEntry // sizeof=0xC
{                                       // ...
    unsigned int frameCount;
    int skinnedCachedOffset;
    unsigned __int16 numSkinnedVerts;
    unsigned __int16 ageCount;
};

struct cpose_t // sizeof=0x64
{                                       // ...
    unsigned __int16 lightingHandle;
    unsigned __int8 eType;
    unsigned __int8 eTypeUnion;
    unsigned __int8 localClientNum;
    // padding byte
    // padding byte
    // padding byte
    int cullIn;
    unsigned __int8 isRagdoll;
    // padding byte
    // padding byte
    // padding byte
    int ragdollHandle;
    int killcamRagdollHandle;
    int physObjId;
    float origin[3];
    float angles[3];
    GfxSkinCacheEntry skinCacheEntry;
    //$9D88A49AD898204B3D6E378457DD8419 ___u12;
    union //$9D88A49AD898204B3D6E378457DD8419 // sizeof=0x24
    {                                       // ...
        CEntPlayerInfo player;
        CEntTurretInfo turret;
        CEntVehicleInfo vehicle;
        CEntFx fx;
    };
};

struct centity_s // sizeof=0x1DC
{                                       // ...
    cpose_t pose;
    LerpEntityState currentState;
    entityState_s nextState;
    bool nextValid;
    bool bMuzzleFlash;
    bool bTrailMade;
    // padding byte
    int previousEventSequence;
    int miscTime;
    float lightingOrigin[3];
    XAnimTree_s *tree;
};

enum sessionState_t : __int32
{                                       // ...
    SESS_STATE_PLAYING = 0x0,
    SESS_STATE_DEAD = 0x1,
    SESS_STATE_SPECTATOR = 0x2,
    SESS_STATE_INTERMISSION = 0x3,
};

enum clientConnected_t : __int32
{                                       // ...
    CON_DISCONNECTED = 0x0,
    CON_CONNECTING = 0x1,
    CON_CONNECTED = 0x2,
};

enum VehicleMoveState : __int32
{                                       // ...
    VEH_MOVESTATE_STOP = 0x0,
    VEH_MOVESTATE_MOVE = 0x1,
    VEH_MOVESTATE_HOVER = 0x2,
};

enum VehicleTurretState : __int32
{                                       // ...
    VEH_TURRET_STOPPED = 0x0,
    VEH_TURRET_STOPPING = 0x1,
    VEH_TURRET_MOVING = 0x2,
};

struct EntHandle // sizeof=0x4
{                                       // ...
    unsigned __int16 number;
    unsigned __int16 infoIndex;
};

struct entityShared_t // sizeof=0x68
{                                       // ...
    unsigned __int8 linked;
    unsigned __int8 bmodel;
    unsigned __int8 svFlags;
    // padding byte
    int clientMask[2];
    unsigned __int8 inuse;              // ...
    // padding byte
    // padding byte
    // padding byte
    int broadcastTime;
    float mins[3];                      // ...
    float maxs[3];
    int contents;                       // ...
    float absmin[3];                    // ...
    float absmax[3];
    float currentOrigin[3];             // ...
    float currentAngles[3];
    EntHandle ownerNum;
    int eventTime;
};

struct playerTeamState_t // sizeof=0x4
{                                       // ...
    int location;
};

struct clientSession_t // sizeof=0x110
{                                       // ...
    sessionState_t sessionState;
    int forceSpectatorClient;
    int killCamEntity;
    int status_icon;
    int archiveTime;
    int score;
    int deaths;
    int kills;
    int assists;
    unsigned __int16 scriptPersId;
    // padding byte
    // padding byte
    clientConnected_t connected;
    usercmd_s cmd;
    usercmd_s oldcmd;
    int localClient;
    int predictItemPickup;
    char newnetname[16];
    int maxHealth;
    int enterTime;
    playerTeamState_t teamState;
    int voteCount;
    int teamVoteCount;
    float moveSpeedScaleMultiplier;
    int viewmodelIndex;
    int noSpectate;
    int teamInfo;
    clientState_s cs;
    int psOffsetTime;
};

struct gclient_s // sizeof=0x3184
{                                       // ...
    playerState_s ps;
    clientSession_t sess;
    int spectatorClient;
    int noclip;
    int ufo;
    int bFrozen;
    int lastCmdTime;
    int buttons;
    int oldbuttons;
    int latched_buttons;
    int buttonsSinceLastFrame;
    float oldOrigin[3];
    float fGunPitch;
    float fGunYaw;
    int damage_blood;
    float damage_from[3];
    int damage_fromWorld;
    int accurateCount;
    int accuracy_shots;
    int accuracy_hits;
    int inactivityTime;
    int inactivityWarning;
    int lastVoiceTime;
    int switchTeamTime;
    float currentAimSpreadScale;
    gentity_s* persistantPowerup;
    int portalID;
    int dropWeaponTime;
    int sniperRifleFiredTime;
    float sniperRifleMuzzleYaw;
    int PCSpecialPickedUpCount;
    EntHandle useHoldEntity;
    int useHoldTime;
    int useButtonDone;
    int iLastCompassPlayerInfoEnt;
    int compassPingTime;
    int damageTime;
    float v_dmg_roll;
    float v_dmg_pitch;
    float swayViewAngles[3];
    float swayOffset[3];
    float swayAngles[3];
    float vLastMoveAng[3];
    float fLastIdleFactor;
    float vGunOffset[3];
    float vGunSpeed[3];
    int weapIdleTime;
    int lastServerTime;
    int lastSpawnTime;
    unsigned int lastWeapon;
    bool previouslyFiring;
    bool previouslyUsingNightVision;
    bool previouslySprinting;
    // padding byte
    int hasRadar;
    int lastStand;
    int lastStandTime;
};

struct turretInfo_s // sizeof=0x48
{                                       // ...
    int inuse;                          // ...
    int flags;
    int fireTime;
    float arcmin[2];
    float arcmax[2];
    float dropPitch;
    int stance;
    int prevStance;
    int fireSndDelay;
    float userOrigin[3];
    float playerSpread;
    float pitchCap;
    int triggerDown;
    unsigned __int8 fireSnd;
    unsigned __int8 fireSndPlayer;
    unsigned __int8 stopSnd;
    unsigned __int8 stopSndPlayer;
};

struct VehicleRideSlot_t // sizeof=0xC
{                                       // ...
    unsigned int tagName;
    int boneIdx;
    int entNum;
};

struct vehicle_node_t // sizeof=0x44
{                                       // ...
    unsigned __int16 name;
    unsigned __int16 target;
    unsigned __int16 script_linkname;
    unsigned __int16 script_noteworthy;
    __int16 index;
    // padding byte
    // padding byte
    int rotated;
    float speed;
    float lookAhead;
    float origin[3];
    float dir[3];
    float angles[3];
    float length;
    __int16 nextIdx;
    __int16 prevIdx;
};

struct vehicle_pathpos_t // sizeof=0xC0
{                                       // ...
    __int16 nodeIdx;
    __int16 endOfPath;
    float frac;
    float speed;
    float lookAhead;
    float slide;
    float origin[3];
    float angles[3];
    float lookPos[3];
    vehicle_node_t switchNode[2];
};
struct vehicle_physic_t // sizeof=0xF8
{                                       // ...
    float origin[3];
    float prevOrigin[3];
    float angles[3];
    float prevAngles[3];
    float maxAngleVel[3];
    float yawAccel;
    float yawDecel;
    char inputAccelerationOLD;
    char inputTurning;
    // padding byte
    // padding byte
    float driverPedal;
    float driverSteer;
    int onGround;
    float colVelDelta[3];
    float mins[3];
    float maxs[3];
    float vel[3];
    float bodyVel[3];
    float rotVel[3];
    float accel[3];
    float maxPitchAngle;
    float maxRollAngle;
    float wheelZVel[4];
    float wheelZPos[4];
    int wheelSurfType[4];
    float worldTilt[3];
    float worldTiltVel[3];
};

struct VehicleTags // sizeof=0x60
{                                       // ...
    VehicleRideSlot_t riderSlots[3];
    int detach;
    int popout;
    int body;
    int turret;
    int turret_base;
    int barrel;
    int flash[5];
    int wheel[4];
};

struct VehicleTurret // sizeof=0x14
{                                       // ...
    int fireTime;
    int fireBarrel;
    float barrelOffset;
    int barrelBlocked;
    VehicleTurretState turretState;
};
struct VehicleJitter // sizeof=0x3C
{                                       // ...
    int jitterPeriodMin;
    int jitterPeriodMax;
    int jitterEndTime;
    float jitterOffsetRange[3];
    float jitterDeltaAccel[3];
    float jitterAccel[3];
    float jitterPos[3];
};
struct VehicleHover // sizeof=0x1C
{                                       // ...
    float hoverRadius;
    float hoverSpeed;
    float hoverAccel;
    float hoverGoalPos[3];
    int useHoverAccelForAngles;
};

struct item_ent_t // sizeof=0xC
{                                       // ...
    int ammoCount;
    int clipAmmoCount;
    int index;
};
struct __declspec(align(4)) trigger_ent_t // sizeof=0x14
{                                       // ...
    int threshold;
    int accumulate;
    int timestamp;
    int singleUserEntIndex;
    bool requireLookAt;
    // padding byte
    // padding byte
    // padding byte
};
struct mover_ent_t // sizeof=0x60
{                                       // ...
    float decelTime;
    float aDecelTime;
    float speed;
    float aSpeed;
    float midTime;
    float aMidTime;
    float pos1[3];
    float pos2[3];
    float pos3[3];
    float apos1[3];
    float apos2[3];
    float apos3[3];
};
struct corpse_ent_t // sizeof=0x4
{                                       // ...
    int deathAnimStartTime;
};

enum MissileStage : __int32
{                                       // ...
    MISSILESTAGE_SOFTLAUNCH = 0x0,
    MISSILESTAGE_ASCENT = 0x1,
    MISSILESTAGE_DESCENT = 0x2,
};
enum MissileFlightMode : __int32
{                                       // ...
    MISSILEFLIGHTMODE_TOP = 0x0,
    MISSILEFLIGHTMODE_DIRECT = 0x1,
};
struct missile_ent_t // sizeof=0x3C
{                                       // ...
    float time;
    int timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    float curvature[3];
    float targetOffset[3];
    MissileStage stage;
    MissileFlightMode flightMode;
};

struct scr_vehicle_s // sizeof=0x354
{                                       // ...
    vehicle_pathpos_t pathPos;
    vehicle_physic_t phys;
    int entNum;                         // ...
    __int16 infoIdx;
    // padding byte
    // padding byte
    int flags;
    int team;
    VehicleMoveState moveState;
    __int16 waitNode;
    // padding byte
    // padding byte
    float waitSpeed;
    VehicleTurret turret;
    VehicleJitter jitter;
    VehicleHover hover;
    int drawOnCompass;
    unsigned __int16 lookAtText0;
    unsigned __int16 lookAtText1;
    int manualMode;
    float manualSpeed;
    float manualAccel;
    float manualDecel;
    float manualTime;
    float speed;
    float maxDragSpeed;
    float turningAbility;
    int hasTarget;
    int hasTargetYaw;
    int hasGoalYaw;
    int stopAtGoal;
    int stopping;
    int targetEnt;
    EntHandle lookAtEnt;
    float targetOrigin[3];
    float targetOffset[3];
    float targetYaw;
    float goalPosition[3];
    float goalYaw;
    float prevGoalYaw;
    float yawOverShoot;
    int yawSlowDown;
    float nearGoalNotifyDist;
    float joltDir[2];
    float joltTime;
    float joltWave;
    float joltSpeed;
    float joltDecel;
    int playEngineSound;
    EntHandle idleSndEnt;
    EntHandle engineSndEnt;
    float idleSndLerp;
    float engineSndLerp;
    VehicleTags boneIndex;
    int turretHitNum;
    float forcedMaterialSpeed;
};

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

enum DemoType : __int32
{                                       // ...
    DEMO_TYPE_NONE = 0x0,
    DEMO_TYPE_CLIENT = 0x1,
    DEMO_TYPE_SERVER = 0x2,
};
enum CubemapShot : __int32
{                                       // ...
    CUBEMAPSHOT_NONE = 0x0,
    CUBEMAPSHOT_RIGHT = 0x1,
    CUBEMAPSHOT_LEFT = 0x2,
    CUBEMAPSHOT_BACK = 0x3,
    CUBEMAPSHOT_FRONT = 0x4,
    CUBEMAPSHOT_UP = 0x5,
    CUBEMAPSHOT_DOWN = 0x6,
    CUBEMAPSHOT_COUNT = 0x7,
};
enum InvalidCmdHintType : __int32
{                                       // ...
    INVALID_CMD_NONE = 0x0,
    INVALID_CMD_NO_AMMO_BULLETS = 0x1,
    INVALID_CMD_NO_AMMO_FRAG_GRENADE = 0x2,
    INVALID_CMD_NO_AMMO_SPECIAL_GRENADE = 0x3,
    INVALID_CMD_NO_AMMO_FLASH_GRENADE = 0x4,
    INVALID_CMD_STAND_BLOCKED = 0x5,
    INVALID_CMD_CROUCH_BLOCKED = 0x6,
    INVALID_CMD_TARGET_TOO_CLOSE = 0x7,
    INVALID_CMD_LOCKON_REQUIRED = 0x8,
    INVALID_CMD_NOT_ENOUGH_CLEARANCE = 0x9,
};
enum ShockViewTypes : __int32
{                                       // ...
    SHELLSHOCK_VIEWTYPE_BLURRED = 0x0,
    SHELLSHOCK_VIEWTYPE_FLASHED = 0x1,
    SHELLSHOCK_VIEWTYPE_NONE = 0x2,
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
struct refdef_s // sizeof=0x4098
{                                       // ...
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    float tanHalfFovX;
    float tanHalfFovY;
    float vieworg[3];
    float viewaxis[3][3];
    float viewOffset[3];
    int time;
    float zNear;
    float blurRadius;
    GfxDepthOfField dof;
    GfxFilm film;
    GfxGlow glow;
    GfxLight primaryLights[255];
    GfxViewport scissorViewport;
    bool useScissorViewport;
    // padding byte
    // padding byte
    // padding byte
    int localClientNum;
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
    Material *hStatusIcon;
    Material *hRankIcon;
};
struct viewDamage_t // sizeof=0xC
{                                       // ...
    int time;
    int duration;
    float yaw;
};
struct shellshock_parms_t_screenblend // sizeof=0x14
{                                       // ...
    int blurredFadeTime;
    int blurredEffectTime;
    int flashWhiteFadeTime;
    int flashShotFadeTime;
    ShockViewTypes type;
};
struct shellshock_parms_t_view // sizeof=0xC
{                                       // ...
    int fadeTime;
    float kickRate;
    float kickRadius;
};
struct shellshock_parms_t_sound // sizeof=0x230
{                                       // ...
    bool affect;
    char loop[64];
    char loopSilent[64];
    char end[64];
    char endAbort[64];
    // padding byte
    // padding byte
    // padding byte
    int fadeInTime;
    int fadeOutTime;
    float drylevel;
    float wetlevel;
    char roomtype[16];
    float channelvolume[64];
    int modEndDelay;
    int loopFadeTime;
    int loopEndDelay;
};
struct shellshock_parms_t_lookcontrol // sizeof=0x14
{                                       // ...
    bool affect;
    // padding byte
    // padding byte
    // padding byte
    int fadeTime;
    float mouseSensitivity;
    float maxPitchSpeed;
    float maxYawSpeed;
};
struct shellshock_parms_t_movement // sizeof=0x1
{                                       // ...
    bool affect;
};
const struct __declspec(align(4)) shellshock_parms_t // sizeof=0x268
{                                       // ...
    shellshock_parms_t_screenblend screenBlend;
    shellshock_parms_t_view view;
    shellshock_parms_t_sound sound;
    shellshock_parms_t_lookcontrol lookControl;
    shellshock_parms_t_movement movement;
    // padding byte
    // padding byte
    // padding byte
};
struct shellshock_t // sizeof=0x20
{                                       // ...
    const shellshock_parms_t *parms;
    int startTime;
    int duration;
    int loopEndTime;
    float sensitivity;
    float viewDelta[2];
    int hasSavedScreen;
};
struct lerpFrame_t // sizeof=0x30
{                                       // ...
    float yawAngle;
    int yawing;
    float pitchAngle;
    int pitching;
    int animationNumber;
    animation_s *animation;
    int animationTime;
    float oldFramePos[3];
    float animSpeedScale;
    int oldFrameSnapshotTime;
};
struct __declspec(align(4)) clientInfo_t // sizeof=0x4CC
{                                       // ...
    int infoValid;                      // ...
    int nextValid;
    int clientNum;
    char name[16];
    team_t team;
    team_t oldteam;
    int rank;
    int prestige;
    int perks;
    int score;
    int location;
    int health;
    char model[64];
    char attachModelNames[6][64];
    char attachTagNames[6][64];
    lerpFrame_t legs;
    lerpFrame_t torso;
    float lerpMoveDir;
    float lerpLean;
    float playerAngles[3];              // ...
    int leftHandGun;
    int dobjDirty;
    clientControllers_t control;
    unsigned int clientConditions[10][2];
    XAnimTree_s *pXAnimTree;            // ...
    int iDObjWeapon;
    unsigned __int8 weaponModel;
    // padding byte
    // padding byte
    // padding byte
    int stanceTransitionTime;
    int turnAnimEndTime;
    char turnAnimType;
    // padding byte
    // padding byte
    // padding byte
    int attachedVehEntNum;
    int attachedVehSlotIndex;
    bool hideWeapon;
    bool usingKnife;
    // padding byte
    // padding byte
};
struct bgs_t_human // sizeof=0x10
{                                       // ...
    scr_animtree_t tree;                // ...
    scr_anim_s torso;
    scr_anim_s legs;
    scr_anim_s turning;
};
struct bgs_t // sizeof=0xADD08
{                                       // ...
    animScriptData_t animScriptData;    // ...
    bgs_t_human generic_human; // ...
    int time;                           // ...
    int latestSnapshotTime;             // ...
    int frametime;                      // ...
    int anim_user;                      // ...
    XModel *(__cdecl *GetXModel)(const char *); // ...
    void(__cdecl *CreateDObj)(DObjModel_s *, unsigned __int16, XAnimTree_s *, int, int, clientInfo_t *); // ...
    unsigned __int16(__cdecl *AttachWeapon)(DObjModel_s *, unsigned __int16, clientInfo_t *); // ...
    DObj_s *(__cdecl *GetDObj)(int, int); // ...
    void(__cdecl *SafeDObjFree)(int, int); // ...
    void *(__cdecl *AllocXAnim)(int);   // ...
    clientInfo_t clientinfo[64];        // ...
};
struct visionSetVars_t // sizeof=0x50
{                                       // ...
    bool glowEnable;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float glowBloomCutoff;              // ...
    float glowBloomDesaturation;        // ...
    float glowBloomIntensity0;          // ...
    float glowBloomIntensity1;          // ...
    float glowRadius0;                  // ...
    float glowRadius1;                  // ...
    float glowSkyBleedIntensity0;       // ...
    float glowSkyBleedIntensity1;       // ...
    bool filmEnable;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float filmBrightness;               // ...
    float filmContrast;                 // ...
    float filmDesaturation;             // ...
    bool filmInvert;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float filmLightTint[3];             // ...
    float filmDarkTint[3];              // ...
};
enum visionSetLerpStyle_t : __int32
{                                       // ...
    VISIONSETLERP_UNDEFINED = 0x0,
    VISIONSETLERP_NONE = 0x1,
    VISIONSETLERP_TO_LINEAR = 0x2,
    VISIONSETLERP_TO_SMOOTH = 0x3,
    VISIONSETLERP_BACKFORTH_LINEAR = 0x4,
    VISIONSETLERP_BACKFORTH_SMOOTH = 0x5,
};
struct visionSetLerpData_t // sizeof=0xC
{                                       // ...
    int timeStart;
    int timeDuration;
    visionSetLerpStyle_t style;
};
struct hudElemSoundInfo_t // sizeof=0x4
{                                       // ...
    int lastPlayedTime;
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
    snapshot_s *snap;
    snapshot_s *nextSnap;
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
    XModel *pEntityLastXModel[1024];
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
void __cdecl CG_Player_PreControllers(DObj_s *obj, centity_s *cent);
void __cdecl CG_mg42_PreControllers(DObj_s *obj, centity_s *cent);
void  CG_UpdateBModelWorldBounds(int a1@<ebp>, unsigned int localClientNum, centity_s *cent, int forceFilter);
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
centity_s *__cdecl CG_GetPose(int localClientNum, unsigned int handle);
void __cdecl CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent);
void __cdecl CG_InterpolateEntityPosition(cg_s *cgameGlob, centity_s *cent);
void __cdecl Vec3Lerp(const float *start, const float *end, float fraction, float *endpos);
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
void __cdecl Vec3ScaleMad(float scale0, const float *dir0, float scale1, const float *dir1, float *result);
bool __cdecl Vec3IsNormalized(const float *v);
void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin);
void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles);
float *__cdecl CG_GetEntityOrigin(int localClientNum, unsigned int entnum);
void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt);
