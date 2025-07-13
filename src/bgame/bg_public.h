#pragma once
#include <server/sv_world.h>

#define abs8(x) abs(x)
#define abs32(x) abs(x)

enum entity_event_t : __int32
{
    EV_NONE = 0x0,
    EV_FOLIAGE_SOUND = 0x1,
    EV_STOP_WEAPON_SOUND = 0x2,
    EV_SOUND_ALIAS = 0x3,
    EV_SOUND_ALIAS_AS_MASTER = 0x4,
    EV_STOPSOUNDS = 0x5,
    EV_STANCE_FORCE_STAND = 0x6,
    EV_STANCE_FORCE_CROUCH = 0x7,
    EV_STANCE_FORCE_PRONE = 0x8,
    EV_ITEM_PICKUP = 0x9,
    EV_AMMO_PICKUP = 0xA,
    EV_NOAMMO = 0xB,
    EV_EMPTYCLIP = 0xC,
    EV_EMPTY_OFFHAND = 0xD,
    EV_RESET_ADS = 0xE,
    EV_RELOAD = 0xF,
    EV_RELOAD_FROM_EMPTY = 0x10,
    EV_RELOAD_START = 0x11,
    EV_RELOAD_END = 0x12,
    EV_RELOAD_START_NOTIFY = 0x13,
    EV_RELOAD_ADDAMMO = 0x14,
    EV_RAISE_WEAPON = 0x15,
    EV_FIRST_RAISE_WEAPON = 0x16,
    EV_PUTAWAY_WEAPON = 0x17,
    EV_WEAPON_ALT = 0x18,
    EV_PULLBACK_WEAPON = 0x19,
    EV_FIRE_WEAPON = 0x1A,
    EV_FIRE_WEAPON_LASTSHOT = 0x1B,
    EV_RECHAMBER_WEAPON = 0x1C,
    EV_EJECT_BRASS = 0x1D,
    EV_MELEE_SWIPE = 0x1E,
    EV_FIRE_MELEE = 0x1F,
    EV_PREP_OFFHAND = 0x20,
    EV_USE_OFFHAND = 0x21,
    EV_SWITCH_OFFHAND = 0x22,
    EV_MELEE_HIT = 0x23,
    EV_MELEE_MISS = 0x24,
    EV_MELEE_BLOOD = 0x25,
    EV_FIRE_WEAPON_MG42 = 0x26,
    EV_FIRE_QUADBARREL_1 = 0x27,
    EV_FIRE_QUADBARREL_2 = 0x28,
    EV_BULLET_HIT = 0x29,
    EV_BULLET_HIT_CLIENT_SMALL = 0x2A,
    EV_BULLET_HIT_CLIENT_LARGE = 0x2B,
    EV_GRENADE_BOUNCE = 0x2C,
    EV_GRENADE_EXPLODE = 0x2D,
    EV_ROCKET_EXPLODE = 0x2E,
    EV_ROCKET_EXPLODE_NOMARKS = 0x2F,
    EV_FLASHBANG_EXPLODE = 0x30,
    EV_CUSTOM_EXPLODE = 0x31,
    EV_CUSTOM_EXPLODE_NOMARKS = 0x32,
    EV_CHANGE_TO_DUD = 0x33,
    EV_DUD_EXPLODE = 0x34,
    EV_DUD_IMPACT = 0x35,
    EV_BULLET = 0x36,
    EV_PLAY_FX = 0x37,
    EV_PLAY_FX_ON_TAG = 0x38,
    EV_PHYS_EXPLOSION_SPHERE = 0x39,
    EV_PHYS_EXPLOSION_CYLINDER = 0x3A,
    EV_PHYS_EXPLOSION_JOLT = 0x3B,
    EV_PHYS_JITTER = 0x3C,
    EV_EARTHQUAKE = 0x3D,
    EV_GRENADE_SUICIDE = 0x3E,
    EV_DETONATE = 0x3F,
    EV_NIGHTVISION_WEAR = 0x40,
    EV_NIGHTVISION_REMOVE = 0x41,
    EV_OBITUARY = 0x42,
    EV_NO_FRAG_GRENADE_HINT = 0x43,
    EV_NO_SPECIAL_GRENADE_HINT = 0x44,
    EV_TARGET_TOO_CLOSE_HINT = 0x45,
    EV_TARGET_NOT_ENOUGH_CLEARANCE = 0x46,
    EV_LOCKON_REQUIRED_HINT = 0x47,
    EV_FOOTSTEP_SPRINT = 0x48,
    EV_FOOTSTEP_RUN = 0x49,
    EV_FOOTSTEP_WALK = 0x4A,
    EV_FOOTSTEP_PRONE = 0x4B,
    EV_JUMP = 0x4C,
    EV_LANDING_FIRST = 0x4D,
    EV_LANDING_LAST = 0x69,
    EV_LANDING_PAIN_FIRST = 0x6A,
    EV_LANDING_PAIN_LAST = 0x86,
    EV_MAX_EVENTS = 0x87,
};

static const char *entityTypeNames[17] =
{
    "ET_GENERAL",
    "ET_PLAYER",
    "ET_PLAYER_CORPSE",
    "ET_ITEM",
    "ET_MISSILE",
    "ET_INVISIBLE",
    "ET_SCRIPTMOVER",
    "ET_SOUND_BLEND",
    "ET_FX",
    "ET_LOOP_FX",
    "ET_PRIMARY_LIGHT",
    "ET_MG42",
    "ET_HELICOPTER",
    "ET_PLANE",
    "ET_VEHICLE",
    "ET_VEHICLE_COLLMAP",
    "ET_VEHICLE_CORPSE"
};

static const char *eventnames[135] =
{
  "EV_NONE",
  "EV_FOLIAGE_SOUND",
  "EV_STOP_WEAPON_SOUND",
  "EV_SOUND_ALIAS",
  "EV_SOUND_ALIAS_AS_MASTER",
  "EV_STOPSOUNDS",
  "EV_STANCE_FORCE_STAND",
  "EV_STANCE_FORCE_CROUCH",
  "EV_STANCE_FORCE_PRONE",
  "EV_ITEM_PICKUP",
  "EV_AMMO_PICKUP",
  "EV_NOAMMO",
  "EV_EMPTYCLIP",
  "EV_EMPTY_OFFHAND",
  "EV_RESET_ADS",
  "EV_RELOAD",
  "EV_RELOAD_FROM_EMPTY",
  "EV_RELOAD_START",
  "EV_RELOAD_END",
  "EV_RELOAD_START_NOTIFY",
  "EV_RELOAD_ADDAMMO",
  "EV_RAISE_WEAPON",
  "EV_FIRST_RAISE_WEAPON",
  "EV_PUTAWAY_WEAPON",
  "EV_WEAPON_ALT",
  "EV_PULLBACK_WEAPON",
  "EV_FIRE_WEAPON",
  "EV_FIRE_WEAPON_LASTSHOT",
  "EV_RECHAMBER_WEAPON",
  "EV_EJECT_BRASS",
  "EV_MELEE_SWIPE",
  "EV_FIRE_MELEE",
  "EV_PREP_OFFHAND",
  "EV_USE_OFFHAND",
  "EV_SWITCH_OFFHAND",
  "EV_MELEE_HIT",
  "EV_MELEE_MISS",
  "EV_MELEE_BLOOD",
  "EV_FIRE_WEAPON_MG42",
  "EV_FIRE_QUADBARREL_1",
  "EV_FIRE_QUADBARREL_2",
  "EV_BULLET_HIT",
  "EV_BULLET_HIT_CLIENT_SMALL",
  "EV_BULLET_HIT_CLIENT_LARGE",
  "EV_GRENADE_BOUNCE",
  "EV_GRENADE_EXPLODE",
  "EV_ROCKET_EXPLODE",
  "EV_ROCKET_EXPLODE_NOMARKS",
  "EV_FLASHBANG_EXPLODE",
  "EV_CUSTOM_EXPLODE",
  "EV_CUSTOM_EXPLODE_NOMARKS",
  "EV_CHANGE_TO_DUD",
  "EV_DUD_EXPLODE",
  "EV_DUD_IMPACT",
  "EV_BULLET",
  "EV_PLAY_FX",
  "EV_PLAY_FX_ON_TAG",
  "EV_PHYS_EXPLOSION_SPHERE",
  "EV_PHYS_EXPLOSION_CYLINDER",
  "EV_PHYS_EXPLOSION_JOLT",
  "EV_PHYS_JITTER",
  "EV_EARTHQUAKE",
  "EV_GRENADE_SUICIDE",
  "EV_DETONATE",
  "EV_NIGHTVISION_WEAR",
  "EV_NIGHTVISION_REMOVE",
  "EV_OBITUARY",
  "EV_NO_FRAG_GRENADE_HINT",
  "EV_NO_SPECIAL_GRENADE_HINT",
  "EV_TARGET_TOO_CLOSE_HINT",
  "EV_TARGET_NOT_ENOUGH_CLEARANCE",
  "EV_LOCKON_REQUIRED_HINT",
  "EV_FOOTSTEP_SPRINT",
  "EV_FOOTSTEP_RUN",
  "EV_FOOTSTEP_WALK",
  "EV_FOOTSTEP_PRONE",
  "EV_JUMP",
  "EV_LANDING_DEFAULT",
  "EV_LANDING_BARK",
  "EV_LANDING_BRICK",
  "EV_LANDING_CARPET",
  "EV_LANDING_CLOTH",
  "EV_LANDING_CONCRETE",
  "EV_LANDING_DIRT",
  "EV_LANDING_FLESH",
  "EV_LANDING_FOLIAGE",
  "EV_LANDING_GLASS",
  "EV_LANDING_GRASS",
  "EV_LANDING_GRAVEL",
  "EV_LANDING_ICE",
  "EV_LANDING_METAL",
  "EV_LANDING_MUD",
  "EV_LANDING_PAPER",
  "EV_LANDING_PLASTER",
  "EV_LANDING_ROCK",
  "EV_LANDING_SAND",
  "EV_LANDING_SNOW",
  "EV_LANDING_WATER",
  "EV_LANDING_WOOD",
  "EV_LANDING_ASPHALT",
  "EV_LANDING_CERAMIC",
  "EV_LANDING_PLASTIC",
  "EV_LANDING_RUBBER",
  "EV_LANDING_CUSHION",
  "EV_LANDING_FRUIT",
  "EV_LANDING_PAINTEDMETAL",
  "EV_LANDING_PAIN_DEFAULT",
  "EV_LANDING_PAIN_BARK",
  "EV_LANDING_PAIN_BRICK",
  "EV_LANDING_PAIN_CARPET",
  "EV_LANDING_PAIN_CLOTH",
  "EV_LANDING_PAIN_CONCRETE",
  "EV_LANDING_PAIN_DIRT",
  "EV_LANDING_PAIN_FLESH",
  "EV_LANDING_PAIN_FOLIAGE",
  "EV_LANDING_PAIN_GLASS",
  "EV_LANDING_PAIN_GRASS",
  "EV_LANDING_PAIN_GRAVEL",
  "EV_LANDING_PAIN_ICE",
  "EV_LANDING_PAIN_METAL",
  "EV_LANDING_PAIN_MUD",
  "EV_LANDING_PAIN_PAPER",
  "EV_LANDING_PAIN_PLASTER",
  "EV_LANDING_PAIN_ROCK",
  "EV_LANDING_PAIN_SAND",
  "EV_LANDING_PAIN_SNOW",
  "EV_LANDING_PAIN_WATER",
  "EV_LANDING_PAIN_WOOD",
  "EV_LANDING_PAIN_ASPHALT",
  "EV_LANDING_PAIN_CERAMIC",
  "EV_LANDING_PAIN_PLASTIC",
  "EV_LANDING_PAIN_RUBBER",
  "EV_LANDING_PAIN_CUSHION",
  "EV_LANDING_PAIN_FRUIT",
  "EV_LANDING_PAIN_PAINTEDMETAL"
};

struct pmove_t // sizeof=0x110
{                                       // ...
    playerState_s *ps;                  // ...
    usercmd_s cmd;                      // ...
    usercmd_s oldcmd;                   // ...
    int tracemask;                      // ...
    int numtouch;
    int touchents[32];
    float mins[3];                      // ...
    float maxs[3];                      // ...
    float xyspeed;
    int proneChange;
    float maxSprintTimeMultiplier;
    bool mantleStarted;                 // ...
    // padding byte
    // padding byte
    // padding byte
    float mantleEndPos[3];              // ...
    int mantleDuration;                 // ...
    int viewChangeTime;                 // ...
    float viewChange;
    unsigned __int8 handler;            // ...
    // padding byte
    // padding byte
    // padding byte
};

struct pmoveHandler_t // sizeof=0x8
{
    void(__cdecl *trace)(trace_t *, const float *, const float *, const float *, const float *, int, int);
    void(__cdecl *playerEvent)(int, int);
};

struct tagInfo_s // sizeof=0x70  (SP/MP same)
{
    gentity_s *parent;
    gentity_s *next;
    uint16_t name;
    // padding byte
    // padding byte
    int32_t index;
    float axis[4][3];
    float parentInvAxis[4][3];
};

struct trigger_ent_t // sizeof=0x14
{                                       // ...
    int32_t threshold;
    int32_t accumulate;
    int32_t timestamp;
#ifdef KISAK_MP
    int32_t singleUserEntIndex;
#endif
    bool requireLookAt;
    // padding byte
    // padding byte
    // padding byte
};

struct item_ent_t // sizeof=0xC (SP/MP same)
{                                       // ...
    int32_t ammoCount;
    int32_t clipAmmoCount;
    int32_t index;
};
static_assert(sizeof(item_ent_t) == 0xC);

struct mover_ent_t // sizeof=0x60 (SP/MP same)
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
static_assert(sizeof(mover_ent_t) == 0x60);

#ifdef KISAK_MP
struct corpse_ent_t // sizeof=0x4
{                                       // ...
    int32_t deathAnimStartTime;
};
static_assert(sizeof(corpse_ent_t) == 0x4);

struct gentity_s // sizeof=0x274
{                                       // ...
    entityState_s s;                    // ...
    entityShared_t r;                   // ...
    struct gclient_s *client;                  // ...
    turretInfo_s *pTurretInfo;
    scr_vehicle_s *scr_vehicle;
    uint16_t model;
    uint8_t physicsObject;
    uint8_t takedamage;
    uint8_t active;
    uint8_t nopickup;
    uint8_t handler;
    uint8_t team;
    uint16_t classname;         // ...
    uint16_t target;
    uint16_t targetname;
    // padding byte
    // padding byte
    uint32_t attachIgnoreCollision;
    int32_t spawnflags;                     // ...
    int32_t flags;                          // ...
    int32_t eventTime;
    int32_t freeAfterEvent;
    int32_t unlinkAfterEvent;
    int32_t clipmask;
    int32_t processedFrame;
    EntHandle parent;
    int32_t nextthink;
    int32_t health;                         // ...
    int32_t maxHealth;
    int32_t damage;
    int32_t count;
    gentity_s *chain;
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
    tagInfo_s *tagInfo;
    gentity_s *tagChildren;
    uint16_t attachModelNames[19]; // ...
    uint16_t attachTagNames[19];
    int32_t useCount;
    gentity_s *nextFree;
};
#elif KISAK_SP

struct potential_threat_t
{
    bool isEnabled;
    float direction[2];
};

struct sentient_s
{
    gentity_s *ent;
    team_t eTeam;
    int iThreatBias;
    int iThreatBiasGroupIndex;
    bool bIgnoreMe;
    bool bIgnoreAll;
    bool originChanged;
    float oldOrigin[3];
    float maxVisibleDist;
    int iEnemyNotifyTime;
    int attackerCount;
    gentity_s *lastAttacker;
    EntHandle syncedMeleeEnt;
    EntHandle targetEnt;
    EntHandle scriptTargetEnt;
    float entityTargetThreat;
    int meleeAttackerSpot[4];
    float attackerAccuracy;
    bool ignoreRandomBulletDamage;
    bool turretInvulnerability;
    pathnode_t *pClaimedNode;
    pathnode_t *pPrevClaimedNode;
    pathnode_t *pActualChainPos;
    int iActualChainPosTime;
    pathnode_t *pNearestNode;
    unsigned __int8 bNearestNodeValid;
    unsigned __int8 bNearestNodeBad;
    bool inuse;
    int banNodeTime;
};

enum AISpecies : __int32
{
    AI_SPECIES_HUMAN = 0x0,
    AI_SPECIES_DOG = 0x1,
    MAX_AI_SPECIES = 0x2,
    AI_SPECIES_ALL = 0x2,
};

enum ai_state_t : __int32
{
    AIS_INVALID = 0x0,
    AIS_KEEPCURRENT = 0x0,
    AIS_EXPOSED = 0x1,
    AIS_TURRET = 0x2,
    AIS_GRENADE_RESPONSE = 0x3,
    AIS_BADPLACE_FLEE = 0x4,
    AIS_COVERARRIVAL = 0x5,
    AIS_DEATH = 0x6,
    AIS_DEFAULT = 0x1,
    AIS_SETABLE_FIRST = 0x1,
    AIS_SETABLE_LAST = 0x6,
    AIS_PAIN = 0x7,
    AIS_SCRIPTEDANIM = 0x8,
    AIS_CUSTOMANIM = 0x9,
    AIS_NEGOTIATION = 0xA,
    AIS_PUSHABLE_FIRST = 0x7,
    AIS_PUSHABLE_LAST = 0xA,
    AIS_COUNT = 0xB,
};

enum ai_substate_t : __int32
{
    STATE_EXPOSED_COMBAT = 0x64,
    STATE_EXPOSED_NONCOMBAT = 0x65,
    STATE_EXPOSED_REACQUIRE_MOVE = 0x66,
    STATE_EXPOSED_FLASHBANGED = 0x67,
    STATE_DEATH_PRECLEANUP = 0xC8,
    STATE_DEATH_POSTCLEANUP = 0xC9,
    STATE_GRENADE_FLEE = 0x12C,
    STATE_GRENADE_TAKECOVER = 0x12D,
    STATE_GRENADE_COWER = 0x12E,
    STATE_GRENADE_COMBAT = 0x12F,
    STATE_GRENADE_COVERATTACK = 0x130,
    STATE_GRENADE_ACQUIRE = 0x131,
    STATE_GRENADE_THROWBACK = 0x132,
};

enum ai_state_transition_t : __int32
{
    AIS_TRANSITION_CANONICAL = 0xFFFFFFFF,
    AIS_TRANSITION_NONE = 0x0,
    AIS_TRANSITION_SET = 0x1,
    AIS_TRANSITION_PUSH = 0x2,
    AIS_TRANSITION_POP = 0x3,
};

struct ai_transition_cmd_t
{
    ai_state_transition_t eTransition;
    ai_state_t eState;
};

enum ai_orient_mode_t : __int32
{
    AI_ORIENT_INVALID = 0x0,
    AI_ORIENT_DONT_CHANGE = 0x1,
    AI_ORIENT_TO_MOTION = 0x2,
    AI_ORIENT_TO_ENEMY = 0x3,
    AI_ORIENT_TO_ENEMY_OR_MOTION = 0x4,
    AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP = 0x5,
    AI_ORIENT_TO_GOAL = 0x6,
    AI_ORIENT_COUNT = 0x7,
};


struct ai_orient_t
{
    ai_orient_mode_t eMode;
    float fDesiredLookPitch;
    float fDesiredLookYaw;
    float fDesiredBodyYaw;
};

struct ActorAnimSets
{
    unsigned __int16 aimLow;
    unsigned __int16 aimLevel;
    unsigned __int16 aimHigh;
    unsigned __int16 shootLow;
    unsigned __int16 shootLevel;
    unsigned __int16 shootHigh;
    unsigned __int16 animProneLow;
    unsigned __int16 animProneLevel;
    unsigned __int16 animProneHigh;
};

struct ActorCachedInfo
{
    int time;
    float pos[3];
    float dir[3];
};

struct ActorLookAtInfo
{
    float vLookAtPos[3];
    float fLookAtTurnAngle;
    float fLookAtTurnSpeed;
    float fLookAtTurnAccel;
    float fLookAtAnimYawLimit;
    float fLookAtYawLimit;
    unsigned __int16 animLookAtStraight;
    unsigned __int16 animLookAtLeft;
    unsigned __int16 animLookAtRight;
    bool bDoLookAt;
    bool bLookAtSetup;
    int iLookAtBlendEndTime;
    float fLookAtAnimBlendRate;
    float fLookAtLimitBlendRate;
};

enum ai_stance_e : __int32
{
    STANCE_BAD = 0x0,
    STANCE_STAND = 0x1,
    STANCE_CROUCH = 0x2,
    STANCE_PRONE = 0x4,
    STANCE_ANY = 0x7,
};

struct __declspec(align(4)) scr_animscript_t
{
    int func;
    unsigned __int16 name;
};

enum ai_traverse_mode_t : __int32
{
    AI_TRAVERSE_INVALID = 0x0,
    AI_TRAVERSE_GRAVITY = 0x1,
    AI_TRAVERSE_NOGRAVITY = 0x2,
    AI_TRAVERSE_NOCLIP = 0x3,
    AI_TRAVERSE_COUNT = 0x4,
};

enum ai_animmode_t : __int32
{
    AI_ANIM_UNKNOWN = 0x0,
    AI_ANIM_MOVE_CODE = 0x1,
    AI_ANIM_USE_POS_DELTAS = 0x2,
    AI_ANIM_USE_ANGLE_DELTAS = 0x3,
    AI_ANIM_USE_BOTH_DELTAS = 0x4,
    AI_ANIM_USE_BOTH_DELTAS_NOCLIP = 0x5,
    AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY = 0x6,
    AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS = 0x7,
    AI_ANIM_NOPHYSICS = 0x8,
    AI_ANIM_POINT_RELATIVE = 0x9,
};

enum aiphys_t : __int32
{
    AIPHYS_BAD = 0x0,
    AIPHYS_NORMAL_ABSOLUTE = 0x1,
    AIPHYS_NORMAL_RELATIVE = 0x2,
    AIPHYS_NOCLIP = 0x3,
    AIPHYS_NOGRAVITY = 0x4,
    AIPHYS_ZONLY_PHYSICS_RELATIVE = 0x5,
};

struct __declspec(align(4)) actor_physics_t
{
    float vOrigin[3];
    float vVelocity[3];
    unsigned __int16 groundEntNum;
    int iFootstepTimer;
    int bHasGroundPlane;
    float groundplaneSlope;
    int iSurfaceType;
    float vWishDelta[3];
    int bIsAlive;
    int iEntNum;
    aiphys_t ePhysicsType;
    float fGravity;
    int iMsec;
    float vMins[3];
    float vMaxs[3];
    bool prone;
    int iTraceMask;
    int foliageSoundTime;
    int iNumTouch;
    int iTouchEnts[32];
    int iHitEntnum;
    float vHitOrigin[2];
    float vHitNormal[2];
    unsigned __int8 bStuck;
    unsigned __int8 bDeflected;
};

struct pathpoint_t
{
    float vOrigPoint[3];
    float fDir2D[2];
    float fOrigLength;
    int iNodeNum;
};

struct path_t
{
    pathpoint_t pts[32];
    __int16 wPathLen;
    __int16 wOrigPathLen;
    __int16 wDodgeCount;
    __int16 wNegotiationStartNode;
    __int16 lookaheadNextNode;
    __int16 wDodgeEntity;
    float vFinalGoal[3];
    float lookaheadDir[3];
    float forwardLookaheadDir2D[2];
    float fLookaheadDist;
    float fLookaheadAmount;
    float fLookaheadDistToNextNode;
    int minLookAheadNodes;
    int flags;
    int iPathTime;
    team_t eTeam;
    float fCurrLength;
    float vCurrPoint[3];
    int iPathEndTime;
    float pathEndAnimDistSq;
    int pathEndAnimNotified;
};

struct path_trim_t
{
    int iIndex;
    int iDelta;
};

struct actor_goal_s
{
    float pos[3];
    float radius;
    float height;
    pathnode_t *node;
    gentity_s *volume;
};

enum aiGoalSources : __int32
{
    AI_GOAL_SRC_SCRIPT_GOAL = 0x0,
    AI_GOAL_SRC_SCRIPT_ENTITY_GOAL = 0x1,
    AI_GOAL_SRC_FRIENDLY_CHAIN = 0x2,
    AI_GOAL_SRC_ENEMY = 0x3,
};

struct ActorCoverArrivalInfo
{
    int arrivalNotifyRequested;
    int animscriptOverrideRunTo;
    float animscriptOverrideRunToPos[3];
    float animscriptOverrideOriginError[3];
};

struct vis_cache_t
{
    bool bVisible;
    int iLastUpdateTime;
    int iLastVisTime;
};

struct sentient_info_t
{
    vis_cache_t VisCache;
    int iLastAttackMeTime;
    int lastKnownPosTime;
    int attackTime;
    float vLastKnownPos[3];
    pathnode_t *pLastKnownNode;
};

struct SentientHandle
{
    unsigned __int16 number;
    unsigned __int16 infoIndex;
};

struct ai_suppression_t
{
    int iTime;
    sentient_s *pSuppressor;
    float clipPlane[3];
    int movementOnly;
};

struct actor_s
{
    gentity_s *ent;
    sentient_s *sentient;
    AISpecies species;
    ai_state_t eState[5];
    ai_substate_t eSubState[5];
    unsigned int stateLevel;
    int iStateTime;
    int preThinkTime;
    ai_transition_cmd_t StateTransitions[11];
    unsigned int transitionCount;
    ai_state_t eSimulatedState[5];
    unsigned int simulatedStateLevel;
    int iPainTime;
    bool allowPain;
    bool allowDeath;
    bool delayedDeath;
    bool provideCoveringFire;
    float accuracy;
    float playerSightAccuracy;
    unsigned int missCount;
    unsigned int hitCount;
    float debugLastAccuracy;
    int lastShotTime;
    unsigned __int16 properName;
    unsigned __int16 weaponName;
    int iTraceCount;
    float fLookPitch;
    float fLookYaw;
    float vLookForward[3];
    float vLookRight[3];
    float vLookUp[3];
    ai_orient_t CodeOrient;
    ai_orient_t ScriptOrient;
    float fDesiredBodyYaw;
    ActorAnimSets animSets;
    unsigned __int16 anim_pose;
    float fInvProneAnimLowPitch;
    float fInvProneAnimHighPitch;
    float fProneLastDiff;
    int bProneOK;
    actor_prone_info_s ProneInfo;
    ActorCachedInfo eyeInfo;
    ActorCachedInfo muzzleInfo;
    ActorLookAtInfo lookAtInfo;
    int iDamageTaken;
    int iDamageYaw;
    float damageDir[3];
    unsigned __int16 damageHitLoc;
    unsigned __int16 damageWeapon;
    ai_stance_e eAllowedStances;
    unsigned __int16 AnimScriptHandle;
    scr_animscript_t *pAnimScriptFunc;
    scr_animscript_t AnimScriptSpecific;
    ai_traverse_mode_t eTraverseMode;
    unsigned __int8 moveMode;
    bool safeToChangeScript;
    bool bUseGoalWeight;
    ai_animmode_t eAnimMode;
    ai_animmode_t eScriptSetAnimMode;
    actor_physics_t Physics;
    path_t Path;
    float fWalkDist;
    path_trim_t TrimInfo;
    int iFollowMin;
    int iFollowMax;
    float fInterval;
    int pathWaitTime;
    int iTeamMoveWaitTime;
    int iTeamMoveDodgeTime;
    actor_s *pPileUpActor;
    gentity_s *pPileUpEnt;
    int bDontAvoidPlayer;
    __int16 chainFallback;
    float sideMove;
    unsigned __int8 keepClaimedNode;
    unsigned __int8 keepClaimedNodeInGoal;
    unsigned __int8 keepNodeDuringScriptedAnim;
    bool noDodgeMove;
    int mayMoveTime;
    float prevMoveDir[2];
    float leanAmount;
    int exposedStartTime;
    int exposedDuration;
    actor_goal_s codeGoal;
    aiGoalSources codeGoalSrc;
    actor_goal_s scriptGoal;
    EntHandle scriptGoalEnt;
    float pathEnemyLookahead;
    float pathEnemyFightDist;
    float meleeAttackDist;
    bool useEnemyGoal;
    bool useMeleeAttackSpot;
    bool goalPosChanged;
    bool commitToFixedNode;
    bool ignoreForFixedNodeSafeCheck;
    bool fixedNode;
    float fixedNodeSafeRadius;
    float fixedNodeSafeVolumeRadiusSq;
    EntHandle fixedNodeSafeVolume;
    pathnode_t *pDesiredChainPos;
    ActorCoverArrivalInfo arrivalInfo;
    int bPacifist;
    int iPacifistWait;
    int numCoverNodesInGoal;
    int iPotentialCoverNodeCount;
    pathnode_t *pPotentialReacquireNode[10];
    int iPotentialReacquireNodeCount;
    float engageMinDist;
    float engageMinFalloffDist;
    float engageMaxDist;
    float engageMaxFalloffDist;
    scr_animscript_t *pAttackScriptFunc;
    float fovDot;
    float fMaxSightDistSqrd;
    int ignoreCloseFoliage;
    sentient_info_t sentientInfo[33];
    SentientHandle pFavoriteEnemy;
    int talkToSpecies;
    float lastEnemySightPos[3];
    bool lastEnemySightPosValid;
    float anglesToLikelyEnemyPath[3];
    int faceLikelyEnemyPathNeedCheckTime;
    int faceLikelyEnemyPathNeedRecalculateTime;
    const pathnode_t *faceLikelyEnemyPathNode;
    ai_suppression_t Suppressant[4];
    int ignoreSuppression;
    int suppressionWait;
    int suppressionDuration;
    int suppressionStartTime;
    float suppressionMeter;
    potential_threat_t potentialThreat;
    int threatUpdateTime;
    int hasThreateningEnemy;
    float grenadeAwareness;
    EntHandle pGrenade;
    int iGrenadeWeaponIndex;
    unsigned __int16 GrenadeTossMethod;
    int bGrenadeTossValid;
    int bGrenadeTargetValid;
    int iGrenadeAmmo;
    float vGrenadeTossPos[3];
    float vGrenadeTargetPos[3];
    float vGrenadeTossVel[3];
    int bDropWeapon;
    int bDrawOnCompass;
    int iUseHintString;
    gentity_s *pTurret;
    unsigned __int16 turretAnim;
    unsigned __int8 turretAnimSet;
    unsigned __int8 useable;
    bool ignoreTriggers;
    bool pushable;
    bool inuse;
    bool isInBadPlace;
    float badPlaceAwareness;
    float goodShootPos[3];
    int goodShootPosValid;
    unsigned __int16 scriptState;
    unsigned __int16 lastScriptState;
    unsigned __int16 stateChangeReason;
    EntHandle pCloseEnt;
    int moveHistoryIndex;
    float moveHistory[10][2];
    int flashBanged;
    float flashBangedStrength;
    int flashBangImmunity;
    const char *pszDebugInfo;
    pathnode_t *pPotentialCoverNode[1000];
};
struct gentity_s_tag
{
    unsigned __int16 notifyString;
    unsigned __int16 index;
    unsigned __int8 stoppable;
    int basetime;
    int duration;
};

struct TurretInfo
{
    bool inuse;
    int state;
    int flags;
    int fireTime;
    EntHandle manualTarget;
    EntHandle target;
    float targetPos[3];
    int targetTime;
    float missOffsetNormalized[3];
    float arcmin[2];
    float arcmax[2];
    float initialYawmin;
    float initialYawmax;
    float forwardAngleDot;
    float dropPitch;
    int convergenceTime[2];
    int suppressTime;
    float maxRangeSquared;
    SentientHandle detachSentient;
    int stance;
    int prevStance;
    int fireSndDelay;
    float accuracy;
    float userOrigin[3];
    int prevSentTarget;
    float aiSpread;
    float playerSpread;
    team_t eTeam;
    float originError[3];
    float anglesError[3];
    float pitchCap;
    int triggerDown;
    unsigned __int16 fireSnd;
    unsigned __int16 fireSndPlayer;
    unsigned __int16 stopSnd;
    unsigned __int16 stopSndPlayer;
};

struct spawner_ent_t
{
    int team;
    int timestamp;
};

union $B62A4B71B7088F8B102AB9DD52F45DCF
{
    item_ent_t item[2];
    spawner_ent_t spawner;
    trigger_ent_t trigger;
    mover_ent_t mover;
    missile_ent_t missile;
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

struct gentity_s
{
    entityState_s s;
    entityShared_t r;
    gclient_s *client;
    actor_s *actor;
    sentient_s *sentient;
    scr_vehicle_s *scr_vehicle;
    TurretInfo *pTurretInfo;
    unsigned __int8 physicsObject;
    unsigned __int8 takedamage;
    unsigned __int8 active;
    unsigned __int8 nopickup;
    unsigned __int16 model;
    unsigned __int8 handler;
    unsigned __int16 classname;
    unsigned __int16 script_linkName;
    unsigned __int16 script_noteworthy;
    unsigned __int16 target;
    unsigned __int16 targetname;
    unsigned int attachIgnoreCollision;
    int spawnflags;
    int flags;
    int clipmask;
    int processedFrame;
    EntHandle parent;
    int nextthink;
    int health;
    int maxHealth;
    int nexteq;
    int damage;
    int count;
    gentity_s *chain;
    gentity_s *activator;
    $B62A4B71B7088F8B102AB9DD52F45DCF ___u32;
    EntHandle missileTargetEnt;
    unsigned __int16 lookAtText0;
    unsigned __int16 lookAtText1;
    gentity_s_tag snd_wait;
    tagInfo_s *tagInfo;
    gentity_s *tagChildren;
    animscripted_s *scripted;
    unsigned __int16 attachModelNames[31];
    unsigned __int16 attachTagNames[31];
    unsigned __int16 disconnectedLinks;
    int iDisconnectTime;
    float angleLerpRate;
    XAnimTree_s *pAnimTree;
    gentity_s *nextFree;
};
#endif

void __cdecl G_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentmask);

void __cdecl G_PlayerEvent(int clientNum, int event);
void __cdecl CG_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentMask);

//#ifdef DEDICATED
//static const pmoveHandler_t pmoveHandlers[2] = { { G_TraceCapsule, NULL}, {&G_TraceCapsule, &G_PlayerEvent} }; // idb
//#else
static const pmoveHandler_t pmoveHandlers[2] = { { CG_TraceCapsule, NULL}, {&G_TraceCapsule, &G_PlayerEvent} }; // idb
//#endif

// bg_jump
extern const dvar_t *jump_height;
extern const dvar_t *jump_stepSize;
extern const dvar_t *jump_slowdownEnable;
extern const dvar_t *jump_ladderPushVel;
extern const dvar_t *jump_spreadAdd;

// bg_weapons
extern unsigned int bg_lastParsedWeaponIndex;