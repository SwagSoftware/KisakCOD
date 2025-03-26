#include "r_sky.h"
#include <universal/com_files.h>
#include "r_init.h"
#include <qcommon/cmd.h>


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

SunFlareDynamic sunFlareArray[4];

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
        0,
        "name for static sprite; can be any material");
    min.value.max = 1000.0;
    min.value.min = 1.0;
    r_sunsprite_size = Dvar_RegisterFloat("r_sunsprite_size", 16.0, min, 0, "diameter in pixels at 640x480 and 80 fov");
    r_sunflare_shader = Dvar_RegisterString(
        "r_sunflare_shader",
        "sun_flare",
        0,
        "name for flare effect; can be any material");
    mina.value.max = 10000.0;
    mina.value.min = 0.0;
    r_sunflare_min_size = Dvar_RegisterFloat(
        "r_sunflare_min_size",
        0.0,
        mina,
        0,
        "smallest size of flare effect in pixels at 640x480");
    minb.value.max = 90.0;
    minb.value.min = 0.0;
    r_sunflare_min_angle = Dvar_RegisterFloat(
        "r_sunflare_min_angle",
        45.0,
        minb,
        0,
        "angle from sun in degrees outside which effect is 0");
    minc.value.max = 10000.0;
    minc.value.min = 0.0;
    r_sunflare_max_size = Dvar_RegisterFloat(
        "r_sunflare_max_size",
        2500.0,
        minc,
        0,
        "largest size of flare effect in pixels at 640x480");
    mind.value.max = 90.0;
    mind.value.min = 0.0;
    r_sunflare_max_angle = Dvar_RegisterFloat(
        "r_sunflare_max_angle",
        2.0,
        mind,
        0,
        "angle from sun in degrees inside which effect is max");
    mine.value.max = 1.0;
    mine.value.min = 0.0;
    r_sunflare_max_alpha = Dvar_RegisterFloat(
        "r_sunflare_max_alpha",
        1.0,
        mine,
        0,
        "0-1 vertex color and alpha of sun at max effect");
    minf.value.max = 60.0;
    minf.value.min = 0.0;
    r_sunflare_fadein = Dvar_RegisterFloat(
        "r_sunflare_fadein",
        1.0,
        minf,
        0,
        "time in seconds to fade alpha from 0% to 100%");
    ming.value.max = 60.0;
    ming.value.min = 0.0;
    r_sunflare_fadeout = Dvar_RegisterFloat(
        "r_sunflare_fadeout",
        1.0,
        ming,
        0,
        "time in seconds to fade alpha from 100% to 0%");
    minh.value.max = 90.0;
    minh.value.min = 0.0;
    r_sunblind_min_angle = Dvar_RegisterFloat(
        "r_sunblind_min_angle",
        30.0,
        minh,
        0,
        "angle from sun in degrees outside which blinding is 0");
    mini.value.max = 90.0;
    mini.value.min = 0.0;
    r_sunblind_max_angle = Dvar_RegisterFloat(
        "r_sunblind_max_angle",
        5.0,
        mini,
        0,
        "angle from sun in degrees inside which blinding is max");
    minj.value.max = 1.0;
    minj.value.min = 0.0;
    r_sunblind_max_darken = Dvar_RegisterFloat(
        "r_sunblind_max_darken",
        0.75,
        minj,
        0,
        "0-1 fraction for how black the world is at max blind");
    mink.value.max = 60.0;
    mink.value.min = 0.0;
    r_sunblind_fadein = Dvar_RegisterFloat(
        "r_sunblind_fadein",
        0.5,
        mink,
        0,
        "time in seconds to fade blind from 0% to 100%");
    minl.value.max = 60.0;
    minl.value.min = 0.0;
    r_sunblind_fadeout = Dvar_RegisterFloat(
        "r_sunblind_fadeout",
        3.0,
        minl,
        0,
        "time in seconds to fade blind from 100% to 0%");
    minm.value.max = 90.0;
    minm.value.min = 0.0;
    r_sunglare_min_angle = Dvar_RegisterFloat(
        "r_sunglare_min_angle",
        30.0,
        minm,
        0,
        "angle from sun in degrees inside which glare is maximum");
    minn.value.max = 90.0;
    minn.value.min = 0.0;
    r_sunglare_max_angle = Dvar_RegisterFloat(
        "r_sunglare_max_angle",
        5.0,
        minn,
        0,
        "angle from sun in degrees inside which glare is minimum");
    mino.value.max = 1.0;
    mino.value.min = 0.0;
    r_sunglare_max_lighten = Dvar_RegisterFloat(
        "r_sunglare_max_lighten",
        0.75,
        mino,
        0,
        "0-1 fraction for how white the world is at max glare");
    minp.value.max = 60.0;
    minp.value.min = 0.0;
    r_sunglare_fadein = Dvar_RegisterFloat(
        "r_sunglare_fadein",
        0.5,
        minp,
        0,
        "time in seconds to fade glare from 0% to 100%");
    minq.value.max = 60.0;
    minq.value.min = 0.0;
    r_sunglare_fadeout = Dvar_RegisterFloat(
        "r_sunglare_fadeout",
        3.0,
        minq,
        0,
        "time in seconds to fade glare from 100% to 0%");
    minr.value.max = 360.0;
    minr.value.min = -360.0;
    r_sun_fx_position = Dvar_RegisterVec3(
        "r_sun_fx_position",
        0.0,
        0.0,
        0.0,
        minr,
        0,
        "Position in degrees of the sun effect");
}

void __cdecl R_SetSunFromDvars(sunflare_t *sun)
{
    float v1; // [esp+0h] [ebp-A8h]
    float v2; // [esp+4h] [ebp-A4h]
    float v3; // [esp+8h] [ebp-A0h]
    float v4; // [esp+Ch] [ebp-9Ch]
    float v5; // [esp+10h] [ebp-98h]
    float v6; // [esp+14h] [ebp-94h]
    float v7; // [esp+1Ch] [ebp-8Ch]
    float v8; // [esp+30h] [ebp-78h]
    float v9; // [esp+40h] [ebp-68h]
    float v10; // [esp+44h] [ebp-64h]
    float v11; // [esp+4Ch] [ebp-5Ch]
    float v12; // [esp+60h] [ebp-48h]
    float v13; // [esp+70h] [ebp-38h]
    float v14; // [esp+74h] [ebp-34h]
    float v15; // [esp+7Ch] [ebp-2Ch]
    float v16; // [esp+90h] [ebp-18h]
    float v17; // [esp+A0h] [ebp-8h]
    float v18; // [esp+A4h] [ebp-4h]

    if (!sun)
        MyAssertHandler(".\\r_sky.cpp", 108, 0, "%s", "sun");
    sun->spriteMaterial = Material_RegisterHandle((char *)r_sunsprite_shader->current.integer, 6);
    sun->spriteSize = r_sunsprite_size->current.value;
    sun->flareMaterial = Material_RegisterHandle((char *)r_sunflare_shader->current.integer, 6);
    sun->flareMinSize = r_sunflare_min_size->current.value * 0.5;
    v18 = r_sunflare_min_angle->current.value * 0.01745329238474369;
    v6 = cos(v18);
    sun->flareMinDot = v6;
    sun->flareMaxSize = r_sunflare_max_size->current.value * 0.5;
    v17 = r_sunflare_max_angle->current.value * 0.01745329238474369;
    v5 = cos(v17);
    sun->flareMaxDot = v5;
    sun->flareMaxAlpha = r_sunflare_max_alpha->current.value;
    v16 = r_sunflare_fadein->current.value * 1000.0;
    sun->flareFadeInTime = (int)(v16 + 9.313225746154785e-10);
    v15 = r_sunflare_fadeout->current.value * 1000.0;
    sun->flareFadeOutTime = (int)(v15 + 9.313225746154785e-10);
    v14 = r_sunblind_min_angle->current.value * 0.01745329238474369;
    v4 = cos(v14);
    sun->blindMinDot = v4;
    v13 = r_sunblind_max_angle->current.value * 0.01745329238474369;
    v3 = cos(v13);
    sun->blindMaxDot = v3;
    sun->blindMaxDarken = r_sunblind_max_darken->current.value;
    v12 = r_sunblind_fadein->current.value * 1000.0;
    sun->blindFadeInTime = (int)(v12 + 9.313225746154785e-10);
    v11 = r_sunblind_fadeout->current.value * 1000.0;
    sun->blindFadeOutTime = (int)(v11 + 9.313225746154785e-10);
    v10 = r_sunglare_min_angle->current.value * 0.01745329238474369;
    v2 = cos(v10);
    sun->glareMinDot = v2;
    v9 = r_sunglare_max_angle->current.value * 0.01745329238474369;
    v1 = cos(v9);
    sun->glareMaxDot = v1;
    sun->glareMaxLighten = r_sunglare_max_lighten->current.value;
    v8 = r_sunglare_fadein->current.value * 1000.0;
    sun->glareFadeInTime = (int)(v8 + 9.313225746154785e-10);
    v7 = r_sunglare_fadeout->current.value * 1000.0;
    sun->glareFadeOutTime = (int)(v7 + 9.313225746154785e-10);
    AngleVectors(&r_sun_fx_position->current.value, sun->sunFxPosition, 0, 0);
    sun->hasValidData = 1;
}

void __cdecl R_LoadSunThroughDvars(const char *sunName, sunflare_t *sun)
{
    unsigned int SunDvarCount; // eax
    char *v3; // [esp-8h] [ebp-14h]
    char *fullpath; // [esp+0h] [ebp-Ch]
    char *sunFile; // [esp+8h] [ebp-4h] BYREF

    if (!sunName)
        MyAssertHandler(".\\r_sky.cpp", 148, 0, "%s", "sunName");
    if (!sun)
        MyAssertHandler(".\\r_sky.cpp", 149, 0, "%s", "sun");
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

