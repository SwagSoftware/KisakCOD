#pragma once
#include <client_mp/client_mp.h>

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

static const pmoveHandler_t pmoveHandlers[2] = { { CG_TraceCapsule, NULL}, {&G_TraceCapsule, &G_PlayerEvent}}; // idb

// bg_jump
extern const dvar_t *jump_height;
extern const dvar_t *jump_stepSize;
extern const dvar_t *jump_slowdownEnable;
extern const dvar_t *jump_ladderPushVel;
extern const dvar_t *jump_spreadAdd;

// bg_weapons
extern unsigned int bg_lastParsedWeaponIndex;