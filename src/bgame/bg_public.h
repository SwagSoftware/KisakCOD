#pragma once
#include <server/sv_world.h>

#ifdef KISAK_MP
#include <qcommon/msg_mp.h>
#elif KISAK_SP
#include <qcommon/msg.h>
#endif

#include <game/enthandle.h>

#define abs8(x) abs(x)
#define abs32(x) abs(x)

// Player Movement Flags (PMF)
#define PMF_DUCKED 1
#define PMF_PRONE 2
#define PMF_MANTLE 4
#define PMF_LADDER 8
#define PMF_JUMPING 0x4000
#define PMF_VEHICLE_ATTACHED 0x100000

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

#ifdef KISAK_MP
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
#elif KISAK_SP
static const char *entityTypeNames[17] =
{
  "ET_GENERAL",
  "ET_PLAYER",
  "ET_ITEM",
  "ET_MISSILE",
  "ET_INVISIBLE",
  "ET_SCRIPTMOVER",
  "ET_SOUND_BLEND",
  "ET_FX",
  "ET_LOOP_FX",
  "ET_PRIMARY_LIGHT",
  "ET_MG42",
  "ET_VEHICLE",
  "ET_VEHICLE_CORPSE",
  "ET_VEHICLE_COLLMAP",
  "ET_ACTOR",
  "ET_ACTOR_SPAWNER",
  "ET_ACTOR_CORPSE"
};
#endif

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

#ifdef KISAK_SP
struct __declspec(align(4)) pmove_t
{
    playerState_s *ps;
    usercmd_s cmd;
    usercmd_s oldcmd;
    int tracemask;
    int numtouch;
    int touchents[32];
    float mins[3];
    float maxs[3];
    float xyspeed;
    int viewChangeTime;
    float viewChange;
    unsigned __int8 handler;
};
#elif KISAK_MP
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
#endif

#ifdef KISAK_MP
struct pmoveHandler_t // sizeof=0x8
{
    void(__cdecl *trace)(trace_t *, const float *, const float *, const float *, const float *, int, int);
    void(__cdecl *playerEvent)(int, int);
};
#elif KISAK_SP
struct pmoveHandler_t
{
    void(*trace)(trace_t *, const float *, const float *, const float *, const float *, int, int);
};
#endif

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
struct entityShared_t // sizeof=0x68
{                                       // ...
    uint8_t linked;
    uint8_t bmodel;
    uint8_t svFlags;
    int32_t clientMask[2];
    uint8_t inuse;              // ...
    int32_t broadcastTime;
    float mins[3];                      // ...
    float maxs[3];
    int32_t contents;                       // ...
    float absmin[3];                    // ...
    float absmax[3];
    float currentOrigin[3];             // ...
    float currentAngles[3];
    EntHandle ownerNum;
    int32_t eventTime;
};
static_assert(sizeof(entityShared_t) == 0x68);
#elif KISAK_SP
struct entityShared_t
{
    unsigned __int8 linked;
    unsigned __int8 bmodel;
    unsigned __int8 svFlags;
    unsigned __int8 eventType;
    unsigned __int8 inuse;
    float mins[3];
    float maxs[3];
    int contents;
    float absmin[3];
    float absmax[3];
    float currentOrigin[3];
    float currentAngles[3];
    EntHandle ownerNum;
    int eventTime;
};
#endif

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

enum team_t;
#ifdef KISAK_MP
struct corpse_ent_t // sizeof=0x4
{                                       // ...
    int32_t deathAnimStartTime;
};
static_assert(sizeof(corpse_ent_t) == 0x4);



struct missile_ent_t // sizeof=0x3C
{                                       // ...
    float time;
    int32_t timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    float curvature[3];
    float targetOffset[3];
    MissileStage stage;
    MissileFlightMode flightMode;
};
static_assert(sizeof(missile_ent_t) == 0x3C);

struct gentity_s // sizeof=0x274
{                                       // ...
    entityState_s s;                    // ...
    entityShared_t r;                   // ...
    struct gclient_s *client;                  // ...
    struct turretInfo_s *pTurretInfo;
    struct scr_vehicle_s *scr_vehicle;
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

struct missile_ent_t
{
    float predictLandPos[3];
    int predictLandTime;
    int timestamp;
    float time;
    int timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    int thrownBack;
    float curvature[3];
    float targetOffset[3];
    MissileStage stage;
    MissileFlightMode flightMode;
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

enum EntHandler_t : unsigned __int8 // (not a real enum name)
{
    ENT_HANDLER_NULL = 0x0,
    ENT_HANDLER_ACTOR_INIT = 0x1,
    ENT_HANDLER_ACTOR = 0x2,
    ENT_HANDLER_ACTOR_CORPSE = 0x3,
    ENT_HANDLER_TRIGGER_MULTIPLE = 0x4,
    ENT_HANDLER_FRIENDLYCHAIN = 0x5,
    ENT_HANDLER_TRIGGER_HURT = 0x6,
    ENT_HANDLER_TRIGGER_HURT_TOUCH = 0x7,
    ENT_HANDLER_TRIGGER_DAMAGE = 0x8,
    ENT_HANDLER_VEHICLE_INIT = 0x9,
    ENT_HANDLER_VEHICLE = 0xA,
    ENT_HANDLER_VEHICLE_FREE = 0xB,
    ENT_HANDLER_SCRIPT_MOVER = 0xC,
    ENT_HANDLER_SCRIPT_MODEL = 0xD,
    ENT_HANDLER_GRENADE = 0xE,
    ENT_HANDLER_ROCKET = 0xF,
    ENT_HANDLER_SPAWN_POINT = 0x10,
    ENT_HANDLER_CLIENT = 0x11,
    ENT_HANDLER_CLIENT_DEAD = 0x12,
    ENT_HANDLER_HEAD_HIT = 0x13,
    ENT_HANDLER_TURRET_INIT = 0x14,
    ENT_HANDLER_TURRET = 0x15,
    ENT_HANDLER_DROPPED_ITEM = 0x16,
    ENT_HANDLER_ITEM_INIT = 0x17,
    ENT_HANDLER_ITEM = 0x18,
    ENT_HANDLER_TRIGGER_USE = 0x19,
    ENT_HANDLER_PRIMARY_LIGHT = 0x1A,
    ENT_HANDLER_COUNT = 0x1B,
};

struct gentity_s
{
    gentity_s()
    {
        memset(this, 0, sizeof(gentity_s));
    }
    entityState_s s;
    entityShared_t r;
    struct gclient_s *client;
    struct actor_s *actor;
    sentient_s *sentient;
    struct scr_vehicle_s *scr_vehicle;
    TurretInfo *pTurretInfo;
    unsigned __int8 physicsObject;
    unsigned __int8 takedamage;
    unsigned __int8 active;
    unsigned __int8 nopickup;
    unsigned __int16 model;
    unsigned __int8 handler; // ENT_HANDLER_xxxxx
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
    //$B62A4B71B7088F8B102AB9DD52F45DCF ___u32;
    union //$B62A4B71B7088F8B102AB9DD52F45DCF
    {
        item_ent_t item[2];
        spawner_ent_t spawner;
        trigger_ent_t trigger;
        mover_ent_t mover;
        missile_ent_t missile;
    };
    EntHandle missileTargetEnt;
    unsigned __int16 lookAtText0;
    unsigned __int16 lookAtText1;
    gentity_s_tag snd_wait;
    tagInfo_s *tagInfo;
    gentity_s *tagChildren;
    struct animscripted_s *scripted;
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

#ifdef KISAK_MP
void __cdecl G_PlayerEvent(int clientNum, int event);
#endif
void __cdecl CG_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentMask);

#ifdef KISAK_MP
static const pmoveHandler_t pmoveHandlers[2] = { { CG_TraceCapsule, NULL}, {G_TraceCapsule, G_PlayerEvent} }; // idb
#elif KISAK_SP
static const pmoveHandler_t pmoveHandlers[2] = { { CG_TraceCapsule }, { G_TraceCapsule } };
#endif

// bg_jump
extern const dvar_t *jump_height;
extern const dvar_t *jump_stepSize;
extern const dvar_t *jump_slowdownEnable;
extern const dvar_t *jump_ladderPushVel;
extern const dvar_t *jump_spreadAdd;

// bg_weapons
extern unsigned int bg_lastParsedWeaponIndex;
extern struct WeaponDef *bg_weaponDefs[128];