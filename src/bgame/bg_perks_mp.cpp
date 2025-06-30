#include "bg_public.h"
#include "bg_local.h"

const dvar_t *perk_parabolicIcon;
const dvar_t *perk_parabolicRadius;
const dvar_t *perk_parabolicAngle;
const dvar_t *perk_bulletPenetrationMultiplier;
const dvar_t *perk_weapSpreadMultiplier;
const dvar_t *perk_extraBreath;
const dvar_t *perk_grenadeDeath;
const dvar_t *perk_weapReloadMultiplier;
const dvar_t *perk_weapRateMultiplier;
const dvar_t *perk_sprintMultiplier;

const char *bg_perkNames[20] =
{
    "specialty_gpsjammer",
    "specialty_bulletaccuracy",
    "specialty_fastreload",
    "specialty_rof",
    "specialty_holdbreath",
    "specialty_bulletpenetration",
    "specialty_grenadepulldeath",
    "specialty_pistoldeath",
    "specialty_quieter",
    "specialty_parabolic",
    "specialty_longersprint",
    "specialty_detectexplosive",
    "specialty_explosivedamage",
    "specialty_exposeenemy",
    "specialty_bulletdamage",
    "specialty_extraammo",
    "specialty_twoprimaries",
    "specialty_armorvest",
    "specialty_fraggrenade",
    "specialty_specialgrenade",
};

unsigned int __cdecl BG_GetPerkIndexForName(const char *perkName)
{
    unsigned int idx; // [esp+0h] [ebp-4h]

    if (!perkName)
        return 20;
    for (idx = 0; idx < 0x14 && I_stricmp(perkName, bg_perkNames[idx]); ++idx)
        ;
    return idx;
}

void __cdecl Perks_RegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]
    DvarLimits mine; // [esp+4h] [ebp-10h]
    DvarLimits minf; // [esp+4h] [ebp-10h]
    DvarLimits ming; // [esp+4h] [ebp-10h]

    min.value.max = 1.0f;
    min.value.min = 0.0f;
    perk_weapSpreadMultiplier = Dvar_RegisterFloat(
        "perk_weapSpreadMultiplier",
        0.64999998f,
        min,
        DVAR_CHEAT,
        "Percentage of weapon spread to use");
    mina.value.max = 1.0f;
    mina.value.min = 0.0f;
    perk_weapReloadMultiplier = Dvar_RegisterFloat(
        "perk_weapReloadMultiplier",
        0.5f,
        mina,
        DVAR_CHEAT,
        "Percentage of weapon reload time to use");
    minb.value.max = 1.0f;
    minb.value.min = 0.0f;
    perk_weapRateMultiplier = Dvar_RegisterFloat(
        "perk_weapRateMultiplier",
        0.75f,
        minb,
        DVAR_CHEAT,
        "Percentage of weapon firing rate to use");
    minc.value.max = FLT_MAX;
    minc.value.min = 0.0f;
    perk_extraBreath = Dvar_RegisterFloat(
        "perk_extraBreath",
        5.0f,
        minc,
        DVAR_CHEAT,
        "Number of extra seconds a player can hold his breath");
    mind.value.max = 30.0f;
    mind.value.min = 0.0f;
    perk_bulletPenetrationMultiplier = Dvar_RegisterFloat(
        "perk_bulletPenetrationMultiplier",
        2.0f,
        mind,
        DVAR_CHEAT,
        "Multiplier for extra bullet penetration");
    perk_grenadeDeath = Dvar_RegisterString(
        "perk_grenadeDeath",
        "frag_grenade_short_mp",
        DVAR_CHEAT,
        "Name of the grenade weapon to drop");
    mine.value.max = FLT_MAX;
    mine.value.min = 0.0f;
    perk_parabolicRadius = Dvar_RegisterFloat(
        "perk_parabolicRadius",
        400.0f,
        mine,
        DVAR_CHEAT,
        "Eavesdrop perk's effective radius");
    minf.value.max = 180.0f;
    minf.value.min = 0.0f;
    perk_parabolicAngle = Dvar_RegisterFloat(
        "perk_parabolicAngle",
        180.0f,
        minf,
        DVAR_CHEAT,
        "Eavesdrop perk's effective FOV angle");
    perk_parabolicIcon = Dvar_RegisterString(
        "perk_parabolicIcon",
        "specialty_parabolic",
        DVAR_CHEAT,
        "Eavesdrop icon to use when displaying eavesdropped voice chats");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0f;
    perk_sprintMultiplier = Dvar_RegisterFloat(
        "perk_sprintMultiplier",
        2.0f,
        ming,
        DVAR_CHEAT,
        "Multiplier for player_sprinttime");
}

