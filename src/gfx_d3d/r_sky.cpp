#include "r_sky.h"
#include <universal/com_files.h>
#include "r_init.h"
#include <qcommon/cmd.h>
#include "r_dvars.h"


const dvar_t *r_sunflare_min_size;
const dvar_t *r_sunsprite_shader;
const dvar_t *r_sunblind_min_angle;
const dvar_t *r_sunblind_fadeout;
const dvar_t *r_sun_fx_position;
const dvar_t *r_sunglare_fadein;
const dvar_t *r_sunglare_fadeout;
const dvar_t *r_sunflare_fadein;
const dvar_t *r_sunblind_fadein;
const dvar_t *r_sunflare_fadeout;
const dvar_t *r_sunflare_max_alpha;
const dvar_t *r_sunblind_max_darken;
const dvar_t *r_sunflare_max_angle;
const dvar_t *r_sunblind_max_angle;
const dvar_t *r_sunglare_max_angle;
const dvar_t *r_sunglare_min_angle;
const dvar_t *r_sunflare_min_angle;
const dvar_t *r_sunflare_max_size;
const dvar_t *r_sunflare_shader;
const dvar_t *r_sunsprite_size;
const dvar_t *r_sunglare_max_lighten;

unsigned int __cdecl R_GetSunDvarCount()
{
    return 21;
}

void __cdecl R_RegisterSunDvars()
{
    DvarLimits min; // [esp+Ch] [ebp-10h]
    DvarLimits mina; // [esp+Ch] [ebp-10h]
    DvarLimits minb; // [esp+Ch] [ebp-10h]
    DvarLimits minc; // [esp+Ch] [ebp-10h]
    DvarLimits mind; // [esp+Ch] [ebp-10h]
    DvarLimits mine; // [esp+Ch] [ebp-10h]
    DvarLimits minf; // [esp+Ch] [ebp-10h]
    DvarLimits ming; // [esp+Ch] [ebp-10h]
    DvarLimits minh; // [esp+Ch] [ebp-10h]
    DvarLimits mini; // [esp+Ch] [ebp-10h]
    DvarLimits minj; // [esp+Ch] [ebp-10h]
    DvarLimits mink; // [esp+Ch] [ebp-10h]
    DvarLimits minl; // [esp+Ch] [ebp-10h]
    DvarLimits minm; // [esp+Ch] [ebp-10h]
    DvarLimits minn; // [esp+Ch] [ebp-10h]
    DvarLimits mino; // [esp+Ch] [ebp-10h]
    DvarLimits minp; // [esp+Ch] [ebp-10h]
    DvarLimits minq; // [esp+Ch] [ebp-10h]
    DvarLimits minr; // [esp+Ch] [ebp-10h]

    r_sunsprite_shader = Dvar_RegisterString(
        "r_sunsprite_shader",
        "sun",
        DVAR_NOFLAG,
        "name for static sprite; can be any material");
    min.value.max = 1000.0;
    min.value.min = 1.0;
    r_sunsprite_size = Dvar_RegisterFloat("r_sunsprite_size", 16.0, min, DVAR_NOFLAG, "diameter in pixels at 640x480 and 80 fov");
    r_sunflare_shader = Dvar_RegisterString(
        "r_sunflare_shader",
        "sun_flare",
        DVAR_NOFLAG,
        "name for flare effect; can be any material");
    mina.value.max = 10000.0;
    mina.value.min = 0.0;
    r_sunflare_min_size = Dvar_RegisterFloat(
        "r_sunflare_min_size",
        0.0,
        mina,
        DVAR_NOFLAG,
        "smallest size of flare effect in pixels at 640x480");
    minb.value.max = 90.0;
    minb.value.min = 0.0;
    r_sunflare_min_angle = Dvar_RegisterFloat(
        "r_sunflare_min_angle",
        45.0,
        minb,
        DVAR_NOFLAG,
        "angle from sun in degrees outside which effect is 0");
    minc.value.max = 10000.0;
    minc.value.min = 0.0;
    r_sunflare_max_size = Dvar_RegisterFloat(
        "r_sunflare_max_size",
        2500.0,
        minc,
        DVAR_NOFLAG,
        "largest size of flare effect in pixels at 640x480");
    mind.value.max = 90.0;
    mind.value.min = 0.0;
    r_sunflare_max_angle = Dvar_RegisterFloat(
        "r_sunflare_max_angle",
        2.0,
        mind,
        DVAR_NOFLAG,
        "angle from sun in degrees inside which effect is max");
    mine.value.max = 1.0;
    mine.value.min = 0.0;
    r_sunflare_max_alpha = Dvar_RegisterFloat(
        "r_sunflare_max_alpha",
        1.0,
        mine,
        DVAR_NOFLAG,
        "0-1 vertex color and alpha of sun at max effect");
    minf.value.max = 60.0;
    minf.value.min = 0.0;
    r_sunflare_fadein = Dvar_RegisterFloat(
        "r_sunflare_fadein",
        1.0,
        minf,
        DVAR_NOFLAG,
        "time in seconds to fade alpha from 0% to 100%");
    ming.value.max = 60.0;
    ming.value.min = 0.0;
    r_sunflare_fadeout = Dvar_RegisterFloat(
        "r_sunflare_fadeout",
        1.0,
        ming,
        DVAR_NOFLAG,
        "time in seconds to fade alpha from 100% to 0%");
    minh.value.max = 90.0;
    minh.value.min = 0.0;
    r_sunblind_min_angle = Dvar_RegisterFloat(
        "r_sunblind_min_angle",
        30.0,
        minh,
        DVAR_NOFLAG,
        "angle from sun in degrees outside which blinding is 0");
    mini.value.max = 90.0;
    mini.value.min = 0.0;
    r_sunblind_max_angle = Dvar_RegisterFloat(
        "r_sunblind_max_angle",
        5.0,
        mini,
        DVAR_NOFLAG,
        "angle from sun in degrees inside which blinding is max");
    minj.value.max = 1.0;
    minj.value.min = 0.0;
    r_sunblind_max_darken = Dvar_RegisterFloat(
        "r_sunblind_max_darken",
        0.75,
        minj,
        DVAR_NOFLAG,
        "0-1 fraction for how black the world is at max blind");
    mink.value.max = 60.0;
    mink.value.min = 0.0;
    r_sunblind_fadein = Dvar_RegisterFloat(
        "r_sunblind_fadein",
        0.5,
        mink,
        DVAR_NOFLAG,
        "time in seconds to fade blind from 0% to 100%");
    minl.value.max = 60.0;
    minl.value.min = 0.0;
    r_sunblind_fadeout = Dvar_RegisterFloat(
        "r_sunblind_fadeout",
        3.0,
        minl,
        DVAR_NOFLAG,
        "time in seconds to fade blind from 100% to 0%");
    minm.value.max = 90.0;
    minm.value.min = 0.0;
    r_sunglare_min_angle = Dvar_RegisterFloat(
        "r_sunglare_min_angle",
        30.0,
        minm,
        DVAR_NOFLAG,
        "angle from sun in degrees inside which glare is maximum");
    minn.value.max = 90.0;
    minn.value.min = 0.0;
    r_sunglare_max_angle = Dvar_RegisterFloat(
        "r_sunglare_max_angle",
        5.0,
        minn,
        DVAR_NOFLAG,
        "angle from sun in degrees inside which glare is minimum");
    mino.value.max = 1.0;
    mino.value.min = 0.0;
    r_sunglare_max_lighten = Dvar_RegisterFloat(
        "r_sunglare_max_lighten",
        0.75,
        mino,
        DVAR_NOFLAG,
        "0-1 fraction for how white the world is at max glare");
    minp.value.max = 60.0;
    minp.value.min = 0.0;
    r_sunglare_fadein = Dvar_RegisterFloat(
        "r_sunglare_fadein",
        0.5,
        minp,
        DVAR_NOFLAG,
        "time in seconds to fade glare from 0% to 100%");
    minq.value.max = 60.0;
    minq.value.min = 0.0;
    r_sunglare_fadeout = Dvar_RegisterFloat(
        "r_sunglare_fadeout",
        3.0,
        minq,
        DVAR_NOFLAG,
        "time in seconds to fade glare from 100% to 0%");
    minr.value.max = 360.0;
    minr.value.min = -360.0;
    r_sun_fx_position = Dvar_RegisterVec3(
        "r_sun_fx_position",
        0.0,
        0.0,
        0.0,
        minr,
        DVAR_NOFLAG,
        "Position in degrees of the sun effect");
}

void __cdecl R_SetSunFromDvars(sunflare_t *sun)
{
    iassert(sun);

    sun->spriteMaterial = Material_RegisterHandle((char *)r_sunsprite_shader->current.integer, 6);
    sun->spriteSize = r_sunsprite_size->current.value;
    sun->flareMaterial = Material_RegisterHandle((char *)r_sunflare_shader->current.integer, 6);
    sun->flareMinSize = r_sunflare_min_size->current.value * 0.5f;
    sun->flareMinDot = cos(r_sunflare_min_angle->current.value * 0.01745329238474369);
    sun->flareMaxSize = r_sunflare_max_size->current.value * 0.5f;
    sun->flareMaxDot = cos(r_sunflare_max_angle->current.value * 0.01745329238474369);
    sun->flareMaxAlpha = r_sunflare_max_alpha->current.value;
    sun->flareFadeInTime = (int)(r_sunflare_fadein->current.value * 1000.0f);
    sun->flareFadeOutTime = (int)(r_sunflare_fadeout->current.value * 1000.0f);
    sun->blindMinDot = cos(r_sunblind_min_angle->current.value * 0.01745329238474369);
    sun->blindMaxDot = cos(r_sunblind_max_angle->current.value * 0.01745329238474369);
    sun->blindMaxDarken = r_sunblind_max_darken->current.value;
    sun->blindFadeInTime = (int)(r_sunblind_fadein->current.value * 1000.0f);
    sun->blindFadeOutTime = (int)(r_sunblind_fadeout->current.value * 1000.0f);
    sun->glareMinDot = cos(r_sunglare_min_angle->current.value * 0.01745329238474369);
    sun->glareMaxDot = cos(r_sunglare_max_angle->current.value * 0.01745329238474369);
    sun->glareMaxLighten = r_sunglare_max_lighten->current.value;
    sun->glareFadeInTime = (int)(r_sunglare_fadein->current.value * 1000.0f);
    sun->glareFadeOutTime = (int)(r_sunglare_fadeout->current.value * 1000.0f);
    AngleVectors(&r_sun_fx_position->current.value, sun->sunFxPosition, 0, 0);
    sun->hasValidData = 1;
}

void __cdecl R_LoadSunThroughDvars(const char *sunName, sunflare_t *sun)
{
    unsigned int SunDvarCount; // eax
    char *v3; // [esp-8h] [ebp-14h]
    char *fullpath; // [esp+0h] [ebp-Ch]
    char *sunFile; // [esp+8h] [ebp-4h] BYREF

    iassert( sunName );
    iassert( sun );
    fullpath = va("sun/%s.sun", sunName);
    if (FS_ReadFile(fullpath, (void **)&sunFile) >= 0)
    {
        v3 = sunFile;
        SunDvarCount = R_GetSunDvarCount();
        if (Com_LoadDvarsFromBuffer(sunDvarNames, SunDvarCount, v3, fullpath))
            R_SetSunFromDvars(sun);
        FS_FreeFile(sunFile);
    }
    else
    {
        Com_Printf(8, "WARNING: couldn't load sun file '%s'\n", fullpath);
    }
}

void __cdecl R_Cmd_LoadSun()
{
    const char *v0; // eax
    sunflare_t *p_sun; // [esp-4h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        if (sv_cheats->current.enabled)
        {
            if (rgp.world)
            {
                p_sun = &rgp.world->sun;
                v0 = Cmd_Argv(1);
                R_LoadSunThroughDvars(v0, p_sun);
            }
            else
            {
                Com_Printf(8, "You can't r_loadsun while a map isn't loaded\n");
            }
        }
        else
        {
            Com_Printf(8, "You must have cheats enabled to use r_loadsun\n");
        }
    }
    else
    {
        Com_Printf(8, "USAGE: r_loadsun <sunname>\n  sunname must not have an extension\n");
    }
}

void __cdecl R_Cmd_SaveSun()
{
    const char *v0; // eax

    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        R_SaveSunFromDvars(v0);
    }
    else
    {
        Com_Printf(8, "USAGE: r_savesun <sunname>\n  sunname must not have an extension\n");
    }
}

void __cdecl R_SaveSunFromDvars(const char *sunName)
{
    char *v1; // eax
    char buffer; // [esp+10h] [ebp-2008h] BYREF
    _BYTE v3[3]; // [esp+11h] [ebp-2007h] BYREF

    if (Com_SaveDvarsToBuffer(sunDvarNames, 0x15u, &buffer, 0x2000u))
    {
        v1 = va("sun/%s.sun", sunName);
        FS_WriteFile(v1, &buffer, &v3[strlen(&buffer)] - v3);
    }
}

void __cdecl R_FlushSun()
{
    SunFlareDynamic *sunFlare; // [esp+0h] [ebp-8h]
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    for (viewIndex = 0; viewIndex < 4; ++viewIndex)
    {
        sunFlare = &sunFlareArray[viewIndex];
        sunFlare->currentBlind = 0.0;
        sunFlare->currentGlare = 0.0;
        sunFlare->flareIntensity = 0.0;
        sunFlare->hitNum = 0;
        sunFlare->lastDot = 0.0;
        sunFlare->lastTime = 0;
        sunFlare->lastVisibility = 0.0;
    }
}

