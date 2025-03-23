#pragma once
#include <client_mp/client_mp.h>

struct gentity_s;

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