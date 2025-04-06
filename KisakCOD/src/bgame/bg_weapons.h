#pragma once
#include <universal/com_memory.h>


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

enum OffhandSecondaryClass : __int32
{
    PLAYER_OFFHAND_SECONDARY_SMOKE = 0x0,
    PLAYER_OFFHAND_SECONDARY_FLASH = 0x1,
    PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
};

enum weaponstate_t : __int32
{                                       // ...
    WEAPON_READY = 0x0,
    WEAPON_RAISING = 0x1,
    WEAPON_RAISING_ALTSWITCH = 0x2,
    WEAPON_DROPPING = 0x3,
    WEAPON_DROPPING_QUICK = 0x4,
    WEAPON_FIRING = 0x5,
    WEAPON_RECHAMBERING = 0x6,
    WEAPON_RELOADING = 0x7,
    WEAPON_RELOADING_INTERUPT = 0x8,
    WEAPON_RELOAD_START = 0x9,
    WEAPON_RELOAD_START_INTERUPT = 0xA,
    WEAPON_RELOAD_END = 0xB,
    WEAPON_MELEE_INIT = 0xC,
    WEAPON_MELEE_FIRE = 0xD,
    WEAPON_MELEE_END = 0xE,
    WEAPON_OFFHAND_INIT = 0xF,
    WEAPON_OFFHAND_PREPARE = 0x10,
    WEAPON_OFFHAND_HOLD = 0x11,
    WEAPON_OFFHAND_START = 0x12,
    WEAPON_OFFHAND = 0x13,
    WEAPON_OFFHAND_END = 0x14,
    WEAPON_DETONATING = 0x15,
    WEAPON_SPRINT_RAISE = 0x16,
    WEAPON_SPRINT_LOOP = 0x17,
    WEAPON_SPRINT_DROP = 0x18,
    WEAPON_NIGHTVISION_WEAR = 0x19,
    WEAPON_NIGHTVISION_REMOVE = 0x1A,
    WEAPONSTATES_NUM = 0x1B,
};
enum OffhandClass : __int32
{                                       // ...
    OFFHAND_CLASS_NONE = 0x0,
    OFFHAND_CLASS_FRAG_GRENADE = 0x1,
    OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
    OFFHAND_CLASS_FLASH_GRENADE = 0x3,
    OFFHAND_CLASS_COUNT = 0x4,
};

// bg_weapons_load_obj
struct SurfaceTypeSoundList // sizeof=0x8
{
    char *surfaceSoundBase;
    snd_alias_list_t **soundAliasList;
};
WeaponDef *__cdecl BG_LoadWeaponDef_LoadObj(const char *name);

inline void __cdecl SetConfigString(char **ppszConfigString, const char *pszKeyValue)
{
    char v2; // [esp+3h] [ebp-21h]
    char *v3; // [esp+8h] [ebp-1Ch]
    const char *v4; // [esp+Ch] [ebp-18h]
    char *buf; // [esp+20h] [ebp-4h]

    if (*pszKeyValue)
    {
        buf = (char *)Hunk_AllocLowAlign(strlen(pszKeyValue) + 1, 1, "SetConfigString", 9);
        v4 = pszKeyValue;
        v3 = buf;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        } while (v2);
        *ppszConfigString = buf;
    }
    else
    {
        *ppszConfigString = (char *)"";
    }
}

inline void __cdecl SetConfigString2(unsigned __int8 *pMember, const char *pszKeyValue)
{
    SetConfigString((char **)pMember, pszKeyValue);
}