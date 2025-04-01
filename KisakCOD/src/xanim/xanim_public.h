#pragma once

enum nodeType : __int32
{                                       // ...
    NODE_BADNODE = 0x0,
    NODE_PATHNODE = 0x1,
    NODE_COVER_STAND = 0x2,
    NODE_COVER_CROUCH = 0x3,
    NODE_COVER_CROUCH_WINDOW = 0x4,
    NODE_COVER_PRONE = 0x5,
    NODE_COVER_RIGHT = 0x6,
    NODE_COVER_LEFT = 0x7,
    NODE_COVER_WIDE_RIGHT = 0x8,
    NODE_COVER_WIDE_LEFT = 0x9,
    NODE_CONCEALMENT_STAND = 0xA,
    NODE_CONCEALMENT_CROUCH = 0xB,
    NODE_CONCEALMENT_PRONE = 0xC,
    NODE_REACQUIRE = 0xD,
    NODE_BALCONY = 0xE,
    NODE_SCRIPTED = 0xF,
    NODE_NEGOTIATION_BEGIN = 0x10,
    NODE_NEGOTIATION_END = 0x11,
    NODE_TURRET = 0x12,
    NODE_GUARD = 0x13,
    NODE_NUMTYPES = 0x14,
    NODE_DONTLINK = 0x14,
};

enum expDataType : __int32
{                                       // ...
    VAL_INT = 0x0,
    VAL_FLOAT = 0x1,
    VAL_STRING = 0x2,
};

enum operationEnum : __int32
{                                       // ...
    OP_NOOP = 0x0,
    OP_RIGHTPAREN = 0x1,
    OP_MULTIPLY = 0x2,
    OP_DIVIDE = 0x3,
    OP_MODULUS = 0x4,
    OP_ADD = 0x5,
    OP_SUBTRACT = 0x6,
    OP_NOT = 0x7,
    OP_LESSTHAN = 0x8,
    OP_LESSTHANEQUALTO = 0x9,
    OP_GREATERTHAN = 0xA,
    OP_GREATERTHANEQUALTO = 0xB,
    OP_EQUALS = 0xC,
    OP_NOTEQUAL = 0xD,
    OP_AND = 0xE,
    OP_OR = 0xF,
    OP_LEFTPAREN = 0x10,
    OP_COMMA = 0x11,
    OP_BITWISEAND = 0x12,
    OP_BITWISEOR = 0x13,
    OP_BITWISENOT = 0x14,
    OP_BITSHIFTLEFT = 0x15,
    OP_BITSHIFTRIGHT = 0x16,
    OP_SIN = 0x17,
    OP_FIRSTFUNCTIONCALL = 0x17,
    OP_COS = 0x18,
    OP_MIN = 0x19,
    OP_MAX = 0x1A,
    OP_MILLISECONDS = 0x1B,
    OP_DVARINT = 0x1C,
    OP_DVARBOOL = 0x1D,
    OP_DVARFLOAT = 0x1E,
    OP_DVARSTRING = 0x1F,
    OP_STAT = 0x20,
    OP_UIACTIVE = 0x21,
    OP_FLASHBANGED = 0x22,
    OP_SCOPED = 0x23,
    OP_SCOREBOARDVISIBLE = 0x24,
    OP_INKILLCAM = 0x25,
    OP_PLAYERFIELD = 0x26,
    OP_SELECTINGLOCATION = 0x27,
    OP_TEAMFIELD = 0x28,
    OP_OTHERTEAMFIELD = 0x29,
    OP_MARINESFIELD = 0x2A,
    OP_OPFORFIELD = 0x2B,
    OP_MENUISOPEN = 0x2C,
    OP_WRITINGDATA = 0x2D,
    OP_INLOBBY = 0x2E,
    OP_INPRIVATEPARTY = 0x2F,
    OP_PRIVATEPARTYHOST = 0x30,
    OP_PRIVATEPARTYHOSTINLOBBY = 0x31,
    OP_ALONEINPARTY = 0x32,
    OP_ADSJAVELIN = 0x33,
    OP_WEAPLOCKBLINK = 0x34,
    OP_WEAPATTACKTOP = 0x35,
    OP_WEAPATTACKDIRECT = 0x36,
    OP_SECONDSASTIME = 0x37,
    OP_TABLELOOKUP = 0x38,
    OP_LOCALIZESTRING = 0x39,
    OP_LOCALVARINT = 0x3A,
    OP_LOCALVARBOOL = 0x3B,
    OP_LOCALVARFLOAT = 0x3C,
    OP_LOCALVARSTRING = 0x3D,
    OP_TIMELEFT = 0x3E,
    OP_SECONDSASCOUNTDOWN = 0x3F,
    OP_GAMEMSGWNDACTIVE = 0x40,
    OP_TOINT = 0x41,
    OP_TOSTRING = 0x42,
    OP_TOFLOAT = 0x43,
    OP_GAMETYPENAME = 0x44,
    OP_GAMETYPE = 0x45,
    OP_GAMETYPEDESCRIPTION = 0x46,
    OP_SCORE = 0x47,
    OP_FRIENDSONLINE = 0x48,
    OP_FOLLOWING = 0x49,
    OP_STATRANGEBITSSET = 0x4A,
    OP_KEYBINDING = 0x4B,
    OP_ACTIONSLOTUSABLE = 0x4C,
    OP_HUDFADE = 0x4D,
    OP_MAXPLAYERS = 0x4E,
    OP_ACCEPTINGINVITE = 0x4F,
    OP_ISINTERMISSION = 0x50,
    NUM_OPERATORS = 0x51,
};

enum weapType_t : __int32
{                                       // ...
    WEAPTYPE_BULLET = 0x0,
    WEAPTYPE_GRENADE = 0x1,
    WEAPTYPE_PROJECTILE = 0x2,
    WEAPTYPE_BINOCULARS = 0x3,
    WEAPTYPE_NUM = 0x4,
};

enum weapClass_t : __int32
{                                       // ...
    WEAPCLASS_RIFLE = 0x0,
    WEAPCLASS_MG = 0x1,
    WEAPCLASS_SMG = 0x2,
    WEAPCLASS_SPREAD = 0x3,
    WEAPCLASS_PISTOL = 0x4,
    WEAPCLASS_GRENADE = 0x5,
    WEAPCLASS_ROCKETLAUNCHER = 0x6,
    WEAPCLASS_TURRET = 0x7,
    WEAPCLASS_NON_PLAYER = 0x8,
    WEAPCLASS_ITEM = 0x9,
    WEAPCLASS_NUM = 0xA,
};
enum PenetrateType : __int32
{                                       // ...
    PENETRATE_TYPE_NONE = 0x0,
    PENETRATE_TYPE_SMALL = 0x1,
    PENETRATE_TYPE_MEDIUM = 0x2,
    PENETRATE_TYPE_LARGE = 0x3,
    PENETRATE_TYPE_COUNT = 0x4,
};
enum ImpactType : __int32
{                                       // ...
    IMPACT_TYPE_NONE = 0x0,
    IMPACT_TYPE_BULLET_SMALL = 0x1,
    IMPACT_TYPE_BULLET_LARGE = 0x2,
    IMPACT_TYPE_BULLET_AP = 0x3,
    IMPACT_TYPE_SHOTGUN = 0x4,
    IMPACT_TYPE_GRENADE_BOUNCE = 0x5,
    IMPACT_TYPE_GRENADE_EXPLODE = 0x6,
    IMPACT_TYPE_ROCKET_EXPLODE = 0x7,
    IMPACT_TYPE_PROJECTILE_DUD = 0x8,
    IMPACT_TYPE_COUNT = 0x9,
};

enum weapInventoryType_t : __int32
{                                       // ...
    WEAPINVENTORY_PRIMARY = 0x0,
    WEAPINVENTORY_OFFHAND = 0x1,
    WEAPINVENTORY_ITEM = 0x2,
    WEAPINVENTORY_ALTMODE = 0x3,
    WEAPINVENTORYCOUNT = 0x4,
};

enum weapFireType_t : __int32
{                                       // ...
    WEAPON_FIRETYPE_FULLAUTO = 0x0,
    WEAPON_FIRETYPE_SINGLESHOT = 0x1,
    WEAPON_FIRETYPE_BURSTFIRE2 = 0x2,
    WEAPON_FIRETYPE_BURSTFIRE3 = 0x3,
    WEAPON_FIRETYPE_BURSTFIRE4 = 0x4,
    WEAPON_FIRETYPECOUNT = 0x5,
};
enum OffhandClass : __int32
{                                       // ...
    OFFHAND_CLASS_NONE = 0x0,
    OFFHAND_CLASS_FRAG_GRENADE = 0x1,
    OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
    OFFHAND_CLASS_FLASH_GRENADE = 0x3,
    OFFHAND_CLASS_COUNT = 0x4,
};
enum weapStance_t : __int32
{                                       // ...
    WEAPSTANCE_STAND = 0x0,
    WEAPSTANCE_DUCK = 0x1,
    WEAPSTANCE_PRONE = 0x2,
    WEAPSTANCE_NUM = 0x3,
};
enum activeReticleType_t : __int32
{                                       // ...
    VEH_ACTIVE_RETICLE_NONE = 0x0,
    VEH_ACTIVE_RETICLE_PIP_ON_A_STICK = 0x1,
    VEH_ACTIVE_RETICLE_BOUNCING_DIAMOND = 0x2,
    VEH_ACTIVE_RETICLE_COUNT = 0x3,
};
enum weaponIconRatioType_t : __int32
{                                       // ...
    WEAPON_ICON_RATIO_1TO1 = 0x0,
    WEAPON_ICON_RATIO_2TO1 = 0x1,
    WEAPON_ICON_RATIO_4TO1 = 0x2,
    WEAPON_ICON_RATIO_COUNT = 0x3,
};
enum ammoCounterClipType_t : __int32
{                                       // ...
    AMMO_COUNTER_CLIP_NONE = 0x0,
    AMMO_COUNTER_CLIP_MAGAZINE = 0x1,
    AMMO_COUNTER_CLIP_SHORTMAGAZINE = 0x2,
    AMMO_COUNTER_CLIP_SHOTGUN = 0x3,
    AMMO_COUNTER_CLIP_ROCKET = 0x4,
    AMMO_COUNTER_CLIP_BELTFED = 0x5,
    AMMO_COUNTER_CLIP_ALTWEAPON = 0x6,
    AMMO_COUNTER_CLIP_COUNT = 0x7,
};
enum weapOverlayReticle_t : __int32
{                                       // ...
    WEAPOVERLAYRETICLE_NONE = 0x0,
    WEAPOVERLAYRETICLE_CROSSHAIR = 0x1,
    WEAPOVERLAYRETICLE_NUM = 0x2,
};
enum WeapOverlayInteface_t : __int32
{                                       // ...
    WEAPOVERLAYINTERFACE_NONE = 0x0,
    WEAPOVERLAYINTERFACE_JAVELIN = 0x1,
    WEAPOVERLAYINTERFACE_TURRETSCOPE = 0x2,
    WEAPOVERLAYINTERFACECOUNT = 0x3,
};
enum weapProjExposion_t : __int32
{                                       // ...
    WEAPPROJEXP_GRENADE = 0x0,
    WEAPPROJEXP_ROCKET = 0x1,
    WEAPPROJEXP_FLASHBANG = 0x2,
    WEAPPROJEXP_NONE = 0x3,
    WEAPPROJEXP_DUD = 0x4,
    WEAPPROJEXP_SMOKE = 0x5,
    WEAPPROJEXP_HEAVY = 0x6,
    WEAPPROJEXP_NUM = 0x7,
};
enum WeapStickinessType : __int32
{                                       // ...
    WEAPSTICKINESS_NONE = 0x0,
    WEAPSTICKINESS_ALL = 0x1,
    WEAPSTICKINESS_GROUND = 0x2,
    WEAPSTICKINESS_GROUND_WITH_YAW = 0x3,
    WEAPSTICKINESS_COUNT = 0x4,
};
enum guidedMissileType_t : __int32
{                                       // ...
    MISSILE_GUIDANCE_NONE = 0x0,
    MISSILE_GUIDANCE_SIDEWINDER = 0x1,
    MISSILE_GUIDANCE_HELLFIRE = 0x2,
    MISSILE_GUIDANCE_JAVELIN = 0x3,
    MISSILE_GUIDANCE_COUNT = 0x4,
};