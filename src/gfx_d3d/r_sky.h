#pragma once

#include <qcommon/qcommon.h>
#include "r_material.h"

struct sunflare_t // sizeof=0x60
{                                       // ...
    bool hasValidData;
    // padding byte
    // padding byte
    // padding byte
    Material *spriteMaterial;
    Material *flareMaterial;
    float spriteSize;
    float flareMinSize;
    float flareMinDot;
    float flareMaxSize;
    float flareMaxDot;
    float flareMaxAlpha;
    int flareFadeInTime;
    int flareFadeOutTime;
    float blindMinDot;
    float blindMaxDot;
    float blindMaxDarken;
    int blindFadeInTime;
    int blindFadeOutTime;
    float glareMinDot;
    float glareMaxDot;
    float glareMaxLighten;
    int glareFadeInTime;
    int glareFadeOutTime;
    float sunFxPosition[3];
};

static const char* sunDvarNames[] =
{
    "r_sunsprite_shader",
    "r_sunsprite_size",
    "r_sunflare_shader",
    "r_sunflare_min_size",
    "r_sunflare_min_angle",
    "r_sunflare_max_size",
    "r_sunflare_max_angle",
    "r_sunflare_max_alpha",
    "r_sunflare_fadein",
    "r_sunflare_fadeout",
    "r_sunblind_min_angle",
    "r_sunblind_max_angle",
    "r_sunblind_max_darken",
    "r_sunblind_fadein",
    "r_sunblind_fadeout",
    "r_sunglare_min_angle",
    "r_sunglare_max_angle",
    "r_sunglare_max_lighten",
    "r_sunglare_fadein",
    "r_sunglare_fadeout",
    "r_sun_fx_position",
};

struct SunFlareDynamic // sizeof=0x28
{                                       // ...
    float flareIntensity;
    float currentBlind;
    float currentGlare;
    int lastTime;
    float lastVisibility;
    float lastDot;
    bool error;
    bool sunQueryIssued[2];             // ...
    // padding byte
    IDirect3DQuery9 *sunQuery[2];       // ...
    int hitNum;
};

unsigned int __cdecl R_GetSunDvarCount();
void __cdecl R_RegisterSunDvars();
void __cdecl R_SetSunFromDvars(sunflare_t *sun);
void __cdecl R_LoadSunThroughDvars(const char *sunName, sunflare_t *sun);
void __cdecl R_Cmd_LoadSun();
void __cdecl R_Cmd_SaveSun();
void __cdecl R_SaveSunFromDvars(const char *sunName);
void __cdecl R_FlushSun();

extern const dvar_t *r_sunflare_min_size;
extern const dvar_t *r_sunsprite_shader;
extern const dvar_t *r_sunblind_min_angle;
extern const dvar_t *r_sunblind_fadeout;
extern const dvar_t *r_sun_fx_position;
extern const dvar_t *r_sunglare_fadein;
extern const dvar_t *r_sunglare_fadeout;
extern const dvar_t *r_sunflare_fadein;
extern const dvar_t *r_sunblind_fadein;
extern const dvar_t *r_sunflare_fadeout;
extern const dvar_t *r_sunflare_max_alpha;
extern const dvar_t *r_sunblind_max_darken;
extern const dvar_t *r_sunflare_max_angle;
extern const dvar_t *r_sunblind_max_angle;
extern const dvar_t *r_sunglare_max_angle;
extern const dvar_t *r_sunglare_min_angle;
extern const dvar_t *r_sunflare_min_angle;
extern const dvar_t *r_sunflare_max_size;
extern const dvar_t *r_sunflare_shader;
extern const dvar_t *r_sunsprite_size;
extern const dvar_t *r_sunglare_max_lighten;

extern SunFlareDynamic sunFlareArray[4];