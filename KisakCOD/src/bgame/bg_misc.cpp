#include "bg_public.h"
#include "bg_local.h"

#include <qcommon/threads.h>
#include <sound/snd_public.h>
#include <game/game_public.h>
#include <universal/com_files.h>

const char *bgShockDvarNames[27] =
{
  "bg_shock_screenType",
  "bg_shock_screenBlurBlendTime",
  "bg_shock_screenBlurBlendFadeTime",
  "bg_shock_screenFlashWhiteFadeTime",
  "bg_shock_screenFlashShotFadeTime",
  "bg_shock_viewKickPeriod",
  "bg_shock_viewKickRadius",
  "bg_shock_viewKickFadeTime",
  "bg_shock_soundLoop",
  "bg_shock_soundLoopSilent",
  "bg_shock_soundEnd",
  "bg_shock_soundEndAbort",
  "bg_shock_sound",
  "bg_shock_soundFadeInTime",
  "bg_shock_soundFadeOutTime",
  "bg_shock_soundLoopFadeTime",
  "bg_shock_soundLoopEndDelay",
  "bg_shock_soundRoomType",
  "bg_shock_soundDryLevel",
  "bg_shock_soundWetLevel",
  "bg_shock_soundModEndDelay",
  "bg_shock_lookControl",
  "bg_shock_lookControl_maxpitchspeed",
  "bg_shock_lookControl_maxyawspeed",
  "bg_shock_lookControl_mousesensitivityscale",
  "bg_shock_lookControl_fadeTime",
  "bg_shock_movement"
}; // idb

char filebuf[65536];

const int serverOnlyEvents[4] = { 31, 20, 19, -1 }; // idb
const int singleClientEvents[13] = { 6, 7, 8, 34, 13, 14, 32, 33, 34, 37, 42, 43, -1 }; // idb


//char **eventnames       827b08a8     bg_misc.obj
//char (*)[80] bgShockChannelNames 827ea9d8     bg_misc.obj
//char const **bgShockDvarNames 827b0ae0     bg_misc.obj
//struct shellshock_parms_t *bg_shellshockParms 827ebf30     bg_misc.obj
shellshock_parms_t bg_shellshockParms[16];

const dvar_t *player_footstepsThreshhold;
const dvar_t *player_debugHealth;
const dvar_t *bg_shock_lookControl_mousesensitivityscale;
const dvar_t *bg_shock_soundEnd;
const dvar_t *player_view_pitch_down;
const dvar_t *player_runbkThreshhold;
const dvar_t *player_sprintForwardMinimum;
const dvar_t *player_turnAnims;
const dvar_t *stopspeed ;
const dvar_t *bg_shock_soundEndAbort;
const dvar_t *bullet_penetrationMinFxDist;
const dvar_t *player_view_pitch_up;
const dvar_t *bg_maxGrenadeIndicatorSpeed;
const dvar_t *player_meleeHeight;
const dvar_t *bg_foliagesnd_minspeed;
const dvar_t *bg_shock_screenBlurBlendTime;
const dvar_t *player_dmgtimer_flinchTime;
const dvar_t *player_move_factor_on_torso;
const dvar_t *bg_shock_screenBlurBlendFadeTime;
const dvar_t *animscript_debug;
const dvar_t *player_adsExitDelay;
const dvar_t *bg_shock_soundDryLevel;
const dvar_t *bg_swingSpeed;
const dvar_t *bg_shock_movement;
const dvar_s *bg_shock_volume[64];
const dvar_t *bg_aimSpreadMoveSpeedThreshold;
const dvar_t *bg_shock_lookControl;
const dvar_t *player_breath_snd_lerp;
const dvar_t *player_breath_gasp_scale;
const dvar_t *bg_shock_soundLoopFadeTime;
const dvar_t *player_sprintThreshhold;
const dvar_t *bg_shock_screenType;
const dvar_t *player_meleeRange;
const dvar_t *bg_shock_viewKickFadeTime;
const dvar_t *bg_shock_lookControl_fadeTime;
const dvar_t *player_strafeAnimCosAngle;
const dvar_t *player_moveThreshhold;
const dvar_t *player_dmgtimer_minScale;
const dvar_t *player_sprintMinTime;
const dvar_t *bg_viewKickMin;
const dvar_t *bg_foliagesnd_fastinterval;
const dvar_t *bg_shock_soundLoopSilent;
const dvar_t *player_breath_snd_delay;
const dvar_t *inertiaDebug;
const dvar_t *bg_fallDamageMaxHeight;
const dvar_t *player_runThreshhold;
const dvar_t *bg_shock_soundFadeInTime;
const dvar_t *player_spectateSpeedScale;
const dvar_t *bg_shock_soundLoopEndDelay;
const dvar_t *player_dmgtimer_timePerPoint;
const dvar_t *bg_prone_yawcap;
const dvar_t *friction  ;
const dvar_t *bg_bobAmplitudeSprinting;
const dvar_t *inertiaMax;
const dvar_t *bg_shock_soundFadeOutTime;
const dvar_t *player_scopeExitOnDamage;
const dvar_t *player_dmgtimer_stumbleTime;
const dvar_t *bg_foliagesnd_resetinterval;
const dvar_t *bg_shock_lookControl_maxyawspeed;
const dvar_t *player_backSpeedScale;
const dvar_t *player_breath_fire_delay;
const dvar_t *bg_foliagesnd_slowinterval;
const dvar_t *bg_viewKickScale;
const dvar_t *bg_shock_soundWetLevel;
const dvar_t *player_breath_hold_lerp;
const dvar_t *inertiaAngle;
const dvar_t *player_dmgtimer_maxTime;
const dvar_t *bg_bobMax ;
const dvar_t *player_burstFireCooldown;
const dvar_t *bg_shock_screenFlashWhiteFadeTime;
const dvar_t *player_breath_gasp_time;
const dvar_t *bg_shock_soundLoop;
const dvar_t *bg_shock_viewKickPeriod;
const dvar_t *bg_bobAmplitudeProne;
const dvar_t *player_meleeChargeFriction;
const dvar_t *player_sprintSpeedScale;
const dvar_t *xanim_debug;
const dvar_t *bg_shock_sound;
const dvar_t *player_meleeWidth;
const dvar_t *player_sprintRechargePause;
const dvar_t *bg_legYawTolerance;
const dvar_t *bg_shock_lookControl_maxpitchspeed;
const dvar_t *bg_shock_viewKickRadius;
const dvar_t *player_breath_gasp_lerp;
const dvar_t *player_sprintStrafeSpeedScale;
const dvar_t *player_sprintTime;
const dvar_t *bg_fallDamageMinHeight;
const dvar_t *bg_bobAmplitudeDucked;
const dvar_t *player_strafeSpeedScale;
const dvar_t *bg_shock_soundRoomType;
const dvar_t *player_breath_hold_time;
const dvar_t *anim_debugSpeeds;
const dvar_t *bg_ladder_yawcap;
const dvar_t *bg_shock_screenFlashShotFadeTime;
const dvar_t *bg_shock_soundModEndDelay;
const dvar_t *bg_viewKickRandom;
const dvar_t *bg_bobAmplitudeStanding;
const dvar_t *bg_viewKickMax;
const dvar_t *bg_foliagesnd_maxspeed;
const dvar_t *player_sprintCameraBob;
const dvar_t *player_sustainAmmo;

const dvar_t *player_lean_shift_left;
const dvar_t *player_lean_shift_right;
const dvar_t *player_lean_shift_crouch_left;
const dvar_t *player_lean_shift_crouch_right;
const dvar_t *player_lean_rotate_left;
const dvar_t *player_lean_rotate_right;
const dvar_t *player_lean_rotate_crouch_left;
const dvar_t *player_lean_rotate_crouch_right;

const char *bg_ShockScreenTypeNames[4] =
{
    "blurred",
    "flashed",
    "none",
    NULL
};

const char *bg_soundRoomTypes[27] =
{
    "generic",
    "paddedcell",
    "room",
    "bathroom",
    "livingroom",
    "stoneroom",
    "auditorium",
    "concerthall",
    "cave"
    "arena",
    "hangar",
    "carpetedhallway",
    "stonecorridor",
    "alley",
    "forest",
    "city",
    "mountains",
    "quarry",
    "plain",
    "parkinglot",
    "sewerpipe",
    "underwater",
    "drugged",
    "dizzy",
    "psychotic",
    NULL
};

char bgShockChannelNames[64][80];

void __cdecl BG_RegisterShockVolumeDvars()
{
    DvarLimits min; // [esp+4h] [ebp-28h]
    snd_entchannel_info_t *channelName; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    for (i = 0; i < SND_GetEntChannelCount(); ++i)
    {
        channelName = SND_GetEntChannelName(i);
        if (!channelName)
            MyAssertHandler(".\\bgame\\bg_misc.cpp", 498, 0, "%s", "channelName");
        if (strlen(channelName->name) > 0x40)
            MyAssertHandler(".\\bgame\\bg_misc.cpp", 499, 0, "%s", "strlen(channelName) <= SND_MAX_ENTCHANNEL_NAMELENGTH");
        sprintf(bgShockChannelNames[i], "bg_shock_volume_%s", channelName->name);
        min.value.max = 1.0;
        min.value.min = 0.0;
        bg_shock_volume[i] = Dvar_RegisterFloat(bgShockChannelNames[i], 0.5, min, 0x80u, "");
    }
}

void __cdecl BG_RegisterDvars()
{
    DvarLimits min; // [esp+8h] [ebp-18h]
    DvarLimits mina; // [esp+8h] [ebp-18h]
    DvarLimits minb; // [esp+8h] [ebp-18h]
    DvarLimits minc; // [esp+8h] [ebp-18h]
    DvarLimits mind; // [esp+8h] [ebp-18h]
    DvarLimits mine; // [esp+8h] [ebp-18h]
    DvarLimits minf; // [esp+8h] [ebp-18h]
    DvarLimits ming; // [esp+8h] [ebp-18h]
    DvarLimits minh; // [esp+8h] [ebp-18h]
    DvarLimits mini; // [esp+8h] [ebp-18h]
    DvarLimits minj; // [esp+8h] [ebp-18h]
    DvarLimits mink; // [esp+8h] [ebp-18h]
    DvarLimits minl; // [esp+8h] [ebp-18h]
    DvarLimits minm; // [esp+8h] [ebp-18h]
    DvarLimits minn; // [esp+8h] [ebp-18h]
    DvarLimits mino; // [esp+8h] [ebp-18h]
    DvarLimits minp; // [esp+8h] [ebp-18h]
    DvarLimits minq; // [esp+8h] [ebp-18h]
    DvarLimits minr; // [esp+8h] [ebp-18h]
    DvarLimits mins; // [esp+8h] [ebp-18h]
    DvarLimits mint; // [esp+8h] [ebp-18h]
    DvarLimits minu; // [esp+8h] [ebp-18h]
    DvarLimits minv; // [esp+8h] [ebp-18h]
    DvarLimits minw; // [esp+8h] [ebp-18h]
    DvarLimits minx; // [esp+8h] [ebp-18h]
    DvarLimits miny; // [esp+8h] [ebp-18h]
    DvarLimits minz; // [esp+8h] [ebp-18h]
    DvarLimits minba; // [esp+8h] [ebp-18h]
    DvarLimits minbb; // [esp+8h] [ebp-18h]
    DvarLimits minbc; // [esp+8h] [ebp-18h]
    DvarLimits minbd; // [esp+8h] [ebp-18h]
    DvarLimits minbe; // [esp+8h] [ebp-18h]
    DvarLimits minbf; // [esp+8h] [ebp-18h]
    DvarLimits minbg; // [esp+8h] [ebp-18h]
    DvarLimits minbh; // [esp+8h] [ebp-18h]
    DvarLimits minbi; // [esp+8h] [ebp-18h]
    DvarLimits minbj; // [esp+8h] [ebp-18h]
    DvarLimits minbk; // [esp+8h] [ebp-18h]
    DvarLimits minbl; // [esp+8h] [ebp-18h]
    DvarLimits minbm; // [esp+8h] [ebp-18h]
    DvarLimits minbn; // [esp+8h] [ebp-18h]
    DvarLimits minbo; // [esp+8h] [ebp-18h]
    DvarLimits minbp; // [esp+8h] [ebp-18h]
    DvarLimits minbq; // [esp+8h] [ebp-18h]
    DvarLimits minbr; // [esp+8h] [ebp-18h]
    DvarLimits minbs; // [esp+8h] [ebp-18h]
    DvarLimits minbt; // [esp+8h] [ebp-18h]
    DvarLimits minbu; // [esp+8h] [ebp-18h]
    DvarLimits minbv; // [esp+8h] [ebp-18h]
    DvarLimits minbw; // [esp+8h] [ebp-18h]
    DvarLimits minbx; // [esp+8h] [ebp-18h]
    DvarLimits minby; // [esp+8h] [ebp-18h]
    DvarLimits minbz; // [esp+8h] [ebp-18h]
    DvarLimits minca; // [esp+8h] [ebp-18h]
    DvarLimits mincb; // [esp+8h] [ebp-18h]
    DvarLimits mincc; // [esp+8h] [ebp-18h]
    DvarLimits mincd; // [esp+8h] [ebp-18h]
    DvarLimits mince; // [esp+8h] [ebp-18h]
    DvarLimits mincf; // [esp+8h] [ebp-18h]
    DvarLimits mincg; // [esp+8h] [ebp-18h]
    DvarLimits minch; // [esp+8h] [ebp-18h]
    DvarLimits minci; // [esp+8h] [ebp-18h]
    DvarLimits mincj; // [esp+8h] [ebp-18h]
    DvarLimits minck; // [esp+8h] [ebp-18h]
    DvarLimits mincl; // [esp+8h] [ebp-18h]
    DvarLimits mincm; // [esp+8h] [ebp-18h]
    DvarLimits mincn; // [esp+8h] [ebp-18h]
    DvarLimits minco; // [esp+8h] [ebp-18h]
    DvarLimits mincp; // [esp+8h] [ebp-18h]
    DvarLimits mincq; // [esp+8h] [ebp-18h]
    DvarLimits mincr; // [esp+8h] [ebp-18h]
    DvarLimits mincs; // [esp+8h] [ebp-18h]
    DvarLimits minct; // [esp+8h] [ebp-18h]
    DvarLimits mincu; // [esp+8h] [ebp-18h]
    DvarLimits mincv; // [esp+8h] [ebp-18h]
    DvarLimits mincw; // [esp+8h] [ebp-18h]
    DvarLimits mincx; // [esp+8h] [ebp-18h]
    DvarLimits mincy; // [esp+8h] [ebp-18h]
    DvarLimits mincz; // [esp+8h] [ebp-18h]
    DvarLimits minda; // [esp+8h] [ebp-18h]
    DvarLimits mindb; // [esp+8h] [ebp-18h]

    min.value.max = 10.0;
    min.value.min = 0.0;
    bg_viewKickScale = Dvar_RegisterFloat(
        "bg_viewKickScale",
        0.2,
        min,
        0x80u,
        "The scale to apply to the damage done to caluclate damage view kick");
    mina.value.max = 90.0;
    mina.value.min = 0.0;
    bg_viewKickMax = Dvar_RegisterFloat("bg_viewKickMax", 90.0, mina, 0x80u, "The maximum view kick");
    minb.value.max = 90.0;
    minb.value.min = 0.0;
    bg_viewKickMin = Dvar_RegisterFloat("bg_viewKickMin", 5.0, minb, 0x80u, "The minimum view kick");
    minc.value.max = 1.0;
    minc.value.min = 0.0;
    bg_viewKickRandom = Dvar_RegisterFloat(
        "bg_viewKickRandom",
        0.40000001,
        minc,
        0x80u,
        "The random direction scale view kick");
    mind.value.max = 90.0;
    mind.value.min = 0.0;
    player_view_pitch_up = Dvar_RegisterFloat(
        "player_view_pitch_up",
        85.0,
        mind,
        0x180u,
        "Maximum angle that the player can look up");
    mine.value.max = 90.0;
    mine.value.min = 0.0;
    player_view_pitch_down = Dvar_RegisterFloat(
        "player_view_pitch_down",
        85.0,
        mine,
        0x180u,
        "Maximum angle that the player can look down");
    minf.value.max = 20.0;
    minf.value.min = 0.0;
    player_lean_shift_left = Dvar_RegisterFloat(
        "player_lean_shift_left",
        5.0,
        minf,
        0x180u,
        "Amount to shift the player 3rd person model when leaning left");
    ming.value.max = 20.0;
    ming.value.min = 0.0;
    player_lean_shift_right = Dvar_RegisterFloat(
        "player_lean_shift_right",
        2.5,
        ming,
        0x180u,
        "Amount to shift the player 3rd person model when leaning right");
    minh.value.max = 20.0;
    minh.value.min = 0.0;
    player_lean_shift_crouch_left = Dvar_RegisterFloat(
        "player_lean_shift_crouch_left",
        12.5,
        minh,
        0x180u,
        "Amount to shift the player 3rd person model when crouch leaning left");
    mini.value.max = 20.0;
    mini.value.min = 0.0;
    player_lean_shift_crouch_right = Dvar_RegisterFloat(
        "player_lean_shift_crouch_right",
        13.0,
        mini,
        0x180u,
        "Amount to shift the player 3rd person model when crouch leaning right");
    minj.value.max = 3.0;
    minj.value.min = 0.0;
    player_lean_rotate_left = Dvar_RegisterFloat(
        "player_lean_rotate_left",
        1.25,
        minj,
        0x180u,
        "Amount to rotate the player 3rd person model when leaning left");
    mink.value.max = 3.0;
    mink.value.min = 0.0;
    player_lean_rotate_right = Dvar_RegisterFloat(
        "player_lean_rotate_right",
        1.25,
        mink,
        0x180u,
        "Amount to rotate the player 3rd person model when leaning right");
    minl.value.max = 3.0;
    minl.value.min = 0.0;
    player_lean_rotate_crouch_left = Dvar_RegisterFloat(
        "player_lean_rotate_crouch_left",
        1.25,
        minl,
        0x180u,
        "Amount to rotate the player 3rd person model when crouch leaning left");
    minm.value.max = 3.0;
    minm.value.min = 0.0;
    player_lean_rotate_crouch_right = Dvar_RegisterFloat(
        "player_lean_rotate_crouch_right",
        1.0,
        minm,
        0x180u,
        "Amount to rotate the player 3rd person model when crouch leaning right");
    minn.value.max = 360.0;
    minn.value.min = 0.0;
    bg_ladder_yawcap = Dvar_RegisterFloat(
        "bg_ladder_yawcap",
        100.0,
        minn,
        0x180u,
        "The maximum angle that a player can look around while on a ladder");
    mino.value.max = 360.0;
    mino.value.min = 0.0;
    bg_prone_yawcap = Dvar_RegisterFloat(
        "bg_prone_yawcap",
        85.0,
        mino,
        0x180u,
        "The maximum angle that a player can look around quickly while prone");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.0;
    bg_foliagesnd_minspeed = Dvar_RegisterFloat(
        "bg_foliagesnd_minspeed",
        40.0,
        minp,
        0x180u,
        "The speed that a player must be going to make minimum noise while moving through foliage");
    minq.value.max = FLT_MAX;
    minq.value.min = 0.0;
    bg_foliagesnd_maxspeed = Dvar_RegisterFloat(
        "bg_foliagesnd_maxspeed",
        180.0,
        minq,
        0x180u,
        "The speed that a player must be going to make maximum noise while moving through foliage");
    bg_foliagesnd_slowinterval = Dvar_RegisterInt(
        "bg_foliagesnd_slowinterval",
        1500,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x180u,
        "The time between each foliage sound when moving slowly");
    bg_foliagesnd_fastinterval = Dvar_RegisterInt(
        "bg_foliagesnd_fastinterval",
        500,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x180u,
        "The time between each foliage sound when moving quickly");
    bg_foliagesnd_resetinterval = Dvar_RegisterInt(
        "bg_foliagesnd_resetinterval",
        500,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x180u,
        "The time interval before foliage sounds are reset after the player has stopped moving");
    minr.value.max = FLT_MAX;
    minr.value.min = 1.0;
    bg_fallDamageMinHeight = Dvar_RegisterFloat(
        "bg_fallDamageMinHeight",
        128.0,
        minr,
        0x188u,
        "The height that a player will start to take minimum damage if they fall");
    mins.value.max = FLT_MAX;
    mins.value.min = 1.0;
    bg_fallDamageMaxHeight = Dvar_RegisterFloat(
        "bg_fallDamageMaxHeight",
        300.0,
        mins,
        0x188u,
        "The height that a player will take maximum damage when falling");
    mint.value.max = 1000.0;
    mint.value.min = 0.0;
    inertiaMax = Dvar_RegisterFloat("inertiaMax", 50.0, mint, 0x180u, "Maximum player inertia");
    inertiaDebug = Dvar_RegisterBool("inertiaDebug", 0, 0x180u, "Show inertia debug information");
    minu.value.max = 1.0;
    minu.value.min = -1.0;
    inertiaAngle = Dvar_RegisterFloat(
        "inertiaAngle",
        0.0,
        minu,
        0x180u,
        "The cosine of the angle at which inertia occurs");
    minv.value.max = 100.0;
    minv.value.min = 0.0;
    friction = Dvar_RegisterFloat("friction", 5.5, minv, 0x180u, "Player friction");
    minw.value.max = 1000.0;
    minw.value.min = 0.0;
    stopspeed = Dvar_RegisterFloat("stopspeed", 100.0, minw, 0x180u, "The player deceleration");
    minx.value.max = 1.0;
    minx.value.min = 0.0;
    bg_swingSpeed = Dvar_RegisterFloat(
        "bg_swingSpeed",
        0.2,
        minx,
        0x80u,
        "The rate at which the player's legs swing around when strafing(multi-player only)");
    miny.value.max = 180.0;
    miny.value.min = 0.0;
    bg_legYawTolerance = Dvar_RegisterFloat(
        "bg_legYawTolerance",
        20.0,
        miny,
        0x80u,
        "The amount the player's leg yaw can differ from his torso before moving ta match");
    minz.value.max = 1.0;
    minz.value.min = 0.0;
    bg_bobAmplitudeSprinting = Dvar_RegisterVec2(
        "bg_bobAmplitudeSprinting",
        0.02,
        0.014,
        minz,
        0x180u,
        "The multiplier to apply to the player's speed to get the bob amplitude while sprinting");
    minba.value.max = 1.0;
    minba.value.min = 0.0;
    bg_bobAmplitudeStanding = Dvar_RegisterVec2(
        "bg_bobAmplitudeStanding",
        0.0070000002,
        0.0070000002,
        minba,
        0x1180u,
        "The multiplier to apply to the player's speed to get the bob amplitude while standing");
    minbb.value.max = 1.0;
    minbb.value.min = 0.0;
    bg_bobAmplitudeDucked = Dvar_RegisterVec2(
        "bg_bobAmplitudeDucked",
        0.0074999998,
        0.0074999998,
        minbb,
        0x180u,
        "The multiplier to apply to the player's speed to get the bob amplitude while ducking");
    minbc.value.max = 1.0;
    minbc.value.min = 0.0;
    bg_bobAmplitudeProne = Dvar_RegisterVec2(
        "bg_bobAmplitudeProne",
        0.02,
        0.0049999999,
        minbc,
        0x180u,
        "The multiplier to apply to the player's speed to get the bob amplitude while prone");
    minbd.value.max = 36.0;
    minbd.value.min = 0.0;
    bg_bobMax = Dvar_RegisterFloat("bg_bobMax", 8.0, minbd, 0x180u, "The maximum allowed bob amplitude");
    minbe.value.max = 300.0;
    minbe.value.min = 0.0;
    bg_aimSpreadMoveSpeedThreshold = Dvar_RegisterFloat(
        "bg_aimSpreadMoveSpeedThreshold",
        11.0,
        minbe,
        0x180u,
        "When player is moving faster than this speed, the aim spread will increase");
    minbf.value.max = 1000.0;
    minbf.value.min = 0.0;
    bg_maxGrenadeIndicatorSpeed = Dvar_RegisterFloat(
        "bg_maxGrenadeIndicatorSpeed",
        20.0,
        minbf,
        0x180u,
        "Maximum speed of grenade that will show up in indicator and can be thrown back.");
    minbg.value.max = 30.0;
    minbg.value.min = 0.0;
    player_breath_hold_time = Dvar_RegisterFloat(
        "player_breath_hold_time",
        4.5,
        minbg,
        0x180u,
        "The maximum time a player can hold his breath");
    minbh.value.max = 30.0;
    minbh.value.min = 0.0;
    player_breath_gasp_time = Dvar_RegisterFloat(
        "player_breath_gasp_time",
        1.0,
        minbh,
        0x180u,
        "The amount of time a player will gasp once they can breath again");
    minbi.value.max = 30.0;
    minbi.value.min = 0.0;
    player_breath_fire_delay = Dvar_RegisterFloat(
        "player_breath_fire_delay",
        0.0,
        minbi,
        0x180u,
        "The amount of time subtracted from the player remaining breath time when a weapon is fired");
    minbj.value.max = 50.0;
    minbj.value.min = 0.0;
    player_breath_gasp_scale = Dvar_RegisterFloat(
        "player_breath_gasp_scale",
        4.5,
        minbj,
        0x180u,
        "Scale value to apply to the target waver during a gasp");
    minbk.value.max = 50.0;
    minbk.value.min = 0.0;
    player_breath_hold_lerp = Dvar_RegisterFloat(
        "player_breath_hold_lerp",
        1.0,
        minbk,
        0x180u,
        "The interpolation rate for the target waver amplitude when holding breath");
    minbl.value.max = 50.0;
    minbl.value.min = 0.0;
    player_breath_gasp_lerp = Dvar_RegisterFloat(
        "player_breath_gasp_lerp",
        6.0,
        minbl,
        0x180u,
        "The interpolation rate for the target waver amplitude when gasping");
    minbm.value.max = 100.0;
    minbm.value.min = 0.0;
    player_breath_snd_lerp = Dvar_RegisterFloat(
        "player_breath_snd_lerp",
        2.0,
        minbm,
        0x180u,
        "The interpolation rate for the player hold breath sound");
    minbn.value.max = 2.0;
    minbn.value.min = 0.0;
    player_breath_snd_delay = Dvar_RegisterFloat(
        "player_breath_snd_delay",
        1.0,
        minbn,
        0x180u,
        "The delay before playing the breathe in sound");
    player_scopeExitOnDamage = Dvar_RegisterBool(
        "player_scopeExitOnDamage",
        0,
        0x180u,
        "Exit the scope if the player takes damage");
    player_adsExitDelay = Dvar_RegisterInt(
        "player_adsExitDelay",
        0,
        (DvarLimits)0x3E800000000LL,
        0x180u,
        "Delay before exiting aim down sight");
    minbo.value.max = 1.0;
    minbo.value.min = 0.0;
    player_move_factor_on_torso = Dvar_RegisterFloat(
        "player_move_factor_on_torso",
        0.0,
        minbo,
        0x80u,
        "The contribution movement direction has on player torso direction(multi-player only)");
    player_debugHealth = Dvar_RegisterBool("player_debugHealth", 0, 0x180u, "Turn on debugging info for player health");
    player_sustainAmmo = Dvar_RegisterBool("player_sustainAmmo", 0, 0x80u, "Firing weapon will not decrease clip ammo.");
    minbp.value.max = 20.0;
    minbp.value.min = 0.0000000099999999;
    player_moveThreshhold = Dvar_RegisterFloat(
        "player_moveThreshhold",
        10.0,
        minbp,
        0x1C0u,
        "The speed at which the player is considered to be moving for the purposes of \n"
        "view model bob and multiplayer model movement");
    minbq.value.max = 50000.0;
    minbq.value.min = 0.0;
    player_footstepsThreshhold = Dvar_RegisterFloat(
        "player_footstepsThreshhold",
        0.0,
        minbq,
        0x1C0u,
        "The minimum speed at which the player makes loud footstep noises");
    minbr.value.max = 20.0;
    minbr.value.min = 0.0;
    player_strafeSpeedScale = Dvar_RegisterFloat(
        "player_strafeSpeedScale",
        0.80000001,
        minbr,
        0x180u,
        "The scale applied to the player speed when strafing");
    minbs.value.max = 20.0;
    minbs.value.min = 0.0;
    player_backSpeedScale = Dvar_RegisterFloat(
        "player_backSpeedScale",
        0.69999999,
        minbs,
        0x180u,
        "The scale applied to the player speed when moving backwards");
    minbt.value.max = 1.0;
    minbt.value.min = 0.0;
    player_strafeAnimCosAngle = Dvar_RegisterFloat(
        "player_strafeAnimCosAngle",
        0.5,
        minbt,
        0x180u,
        "Cosine of the angle which player starts using strafe animations");
    minbu.value.max = 20.0;
    minbu.value.min = 0.0;
    player_spectateSpeedScale = Dvar_RegisterFloat(
        "player_spectateSpeedScale",
        1.0,
        minbu,
        0x180u,
        "The scale applied to the player speed when spectating");
    player_sprintForwardMinimum = Dvar_RegisterInt(
        "player_sprintForwardMinimum",
        105,
        (DvarLimits)0xFF00000000LL,
        0x100u,
        "The minimum forward deflection required to maintain a sprint");
    minbv.value.max = 5.0;
    minbv.value.min = 0.0;
    player_sprintSpeedScale = Dvar_RegisterFloat(
        "player_sprintSpeedScale",
        1.5,
        minbv,
        0x180u,
        "The scale applied to the player speed when sprinting");
    minbw.value.max = 12.8;
    minbw.value.min = 0.0;
    player_sprintTime = Dvar_RegisterFloat(
        "player_sprintTime",
        4.0,
        minbw,
        0x180u,
        "The base length of time a player can sprint");
    minbx.value.max = 12.8;
    minbx.value.min = 0.0;
    player_sprintMinTime = Dvar_RegisterFloat(
        "player_sprintMinTime",
        1.0,
        minbx,
        0x180u,
        "The minimum sprint time needed in order to start sprinting");
    minby.value.max = 9000.0;
    minby.value.min = 0.0;
    player_sprintRechargePause = Dvar_RegisterFloat(
        "player_sprintRechargePause",
        0.0,
        minby,
        0x180u,
        "The length of time the meter will pause before starting to recharge after a player sprints");
    minbz.value.max = 5000.0;
    minbz.value.min = 0.0;
    player_sprintStrafeSpeedScale = Dvar_RegisterFloat(
        "player_sprintStrafeSpeedScale",
        0.667,
        minbz,
        0x180u,
        "The speed at which you can strafe while sprinting");
    minca.value.max = 2.0;
    minca.value.min = 0.0;
    player_sprintCameraBob = Dvar_RegisterFloat(
        "player_sprintCameraBob",
        0.5,
        minca,
        0x180u,
        "The speed the camera bobs while you sprint");
    player_turnAnims = Dvar_RegisterBool(
        "player_turnAnims",
        0,
        0x180u,
        "Use animations to turn a player's model in multiplayer");
    xanim_debug = Dvar_RegisterBool("xanim_debug", 0, 0, "Turn on Xanim Debugging information");
    animscript_debug = Dvar_RegisterBool("animscript_debug", 0, 0, "Turn on animscript debugging information");
    anim_debugSpeeds = Dvar_RegisterBool("anim_debugSpeeds", 0, 0, "Print out animation speed information");
    mincb.value.max = FLT_MAX;
    mincb.value.min = 0.0;
    player_dmgtimer_timePerPoint = Dvar_RegisterFloat(
        "player_dmgtimer_timePerPoint",
        100.0,
        mincb,
        0x180u,
        "The time in milliseconds that the player is slowed down per point of damage");
    mincc.value.max = FLT_MAX;
    mincc.value.min = 0.0;
    player_dmgtimer_maxTime = Dvar_RegisterFloat(
        "player_dmgtimer_maxTime",
        750.0,
        mincc,
        0x180u,
        "The maximum time that the player is slowed due to damage");
    mincd.value.max = 1.0;
    mincd.value.min = 0.0;
    player_dmgtimer_minScale = Dvar_RegisterFloat(
        "player_dmgtimer_minScale",
        0.0,
        mincd,
        0x180u,
        "The minimum scale value to slow the player by when damaged");
    player_dmgtimer_stumbleTime = Dvar_RegisterInt(
        "player_dmgtimer_stumbleTime",
        500,
        (DvarLimits)0x7D000000000LL,
        0x180u,
        "Maximum time to play stumble animations");
    player_dmgtimer_flinchTime = Dvar_RegisterInt(
        "player_dmgtimer_flinchTime",
        500,
        (DvarLimits)0x7D000000000LL,
        0x180u,
        "Maximum time to play flinch animations");
    bg_shock_soundLoop = Dvar_RegisterString("bg_shock_soundLoop", "shellshock_loop", 0x80u, "Shellshock loop alias");
    bg_shock_soundLoopSilent = Dvar_RegisterString(
        "bg_shock_soundLoopSilent",
        "shellshock_loop_silent",
        0x80u,
        "The sound that gets blended with the shellshock loop alias");
    bg_shock_soundEnd = Dvar_RegisterString("bg_shock_soundEnd", "shellshock_end", 0x80u, "Shellshock end sound alias");
    bg_shock_soundEndAbort = Dvar_RegisterString(
        "bg_shock_soundEndAbort",
        "shellshock_end_abort",
        0x80u,
        "Shellshock aborted end sound alias");
    bg_shock_screenType = Dvar_RegisterEnum(
        "bg_shock_screenType",
        bg_ShockScreenTypeNames,
        0,
        0x80u,
        "Shell shock screen effect type");
    mince.value.max = 10.0;
    mince.value.min = 0.001;
    bg_shock_screenBlurBlendTime = Dvar_RegisterFloat(
        "bg_shock_screenBlurBlendTime",
        0.40000001,
        mince,
        0x80u,
        "The amount of time in seconds for the shellshock effect to blend");
    mincf.value.max = 1000.0;
    mincf.value.min = 0.001;
    bg_shock_screenBlurBlendFadeTime = Dvar_RegisterFloat(
        "bg_shock_screenBlurBlendFadeTime",
        1.0,
        mincf,
        0x80u,
        "The amount of time in seconds for the shellshock effect to fade");
    mincg.value.max = 1000.0;
    mincg.value.min = 0.0;
    bg_shock_screenFlashWhiteFadeTime = Dvar_RegisterFloat(
        "bg_shock_screenFlashWhiteFadeTime",
        1.0,
        mincg,
        0x80u,
        "In seconds, how soon from the end of the effect to start blending out the whiteout layer.");
    minch.value.max = 1000.0;
    minch.value.min = 0.0;
    bg_shock_screenFlashShotFadeTime = Dvar_RegisterFloat(
        "bg_shock_screenFlashShotFadeTime",
        1.0,
        minch,
        0x80u,
        "In seconds, how soon from the end of the effect to start blending out the screengrab layer.");
    minci.value.max = 1000.0;
    minci.value.min = 0.001;
    bg_shock_viewKickPeriod = Dvar_RegisterFloat(
        "bg_shock_viewKickPeriod",
        0.75,
        minci,
        0x80u,
        "The period of the shellshock view kick effect");
    mincj.value.max = 1.0;
    mincj.value.min = 0.0;
    bg_shock_viewKickRadius = Dvar_RegisterFloat(
        "bg_shock_viewKickRadius",
        0.050000001,
        mincj,
        0x80u,
        "Shell shock kick radius");
    minck.value.max = 1000.0;
    minck.value.min = 0.001;
    bg_shock_viewKickFadeTime = Dvar_RegisterFloat(
        "bg_shock_viewKickFadeTime",
        3.0,
        minck,
        0x80u,
        "The time for the shellshock kick effect to fade");
    bg_shock_sound = Dvar_RegisterBool("bg_shock_sound", 1, 0x80u, "Play shell shock sound");
    mincl.value.max = 1000.0;
    mincl.value.min = 0.001;
    bg_shock_soundFadeInTime = Dvar_RegisterFloat(
        "bg_shock_soundFadeInTime",
        0.25,
        mincl,
        0x80u,
        "Shell shock sound fade in time in seconds");
    mincm.value.max = 1000.0;
    mincm.value.min = 0.001;
    bg_shock_soundFadeOutTime = Dvar_RegisterFloat(
        "bg_shock_soundFadeOutTime",
        2.5,
        mincm,
        0x80u,
        "Shell shock sound fade out time in seconds");
    mincn.value.max = 1000.0;
    mincn.value.min = 0.001;
    bg_shock_soundLoopFadeTime = Dvar_RegisterFloat(
        "bg_shock_soundLoopFadeTime",
        1.5,
        mincn,
        0x80u,
        "Shell shock sound loop fade time in seconds");
    minco.value.max = 1000.0;
    minco.value.min = -10.0;
    bg_shock_soundLoopEndDelay = Dvar_RegisterFloat(
        "bg_shock_soundLoopEndDelay",
        -3.0,
        minco,
        0x80u,
        "Sound loop end offset time from the end of the shellshock in seconds");
    bg_shock_soundRoomType = Dvar_RegisterEnum(
        "bg_shock_soundRoomType",
        bg_soundRoomTypes,
        0,
        0x80u,
        "Shell shock sound room type");
    mincp.value.max = 1.0;
    mincp.value.min = 0.0;
    bg_shock_soundDryLevel = Dvar_RegisterFloat(
        "bg_shock_soundDryLevel",
        1.0,
        mincp,
        0x80u,
        "Shell shock sound dry level");
    mincq.value.max = 1.0;
    mincq.value.min = 0.0;
    bg_shock_soundWetLevel = Dvar_RegisterFloat(
        "bg_shock_soundWetLevel",
        0.5,
        mincq,
        0x80u,
        "Shell shock sound wet level");
    mincr.value.max = 1000.0;
    mincr.value.min = -1000.0;
    bg_shock_soundModEndDelay = Dvar_RegisterFloat(
        "bg_shock_soundModEndDelay",
        2.0,
        mincr,
        0x80u,
        "The delay from the end of the shell shock to the end of the sound modification");
    BG_RegisterShockVolumeDvars();
    bg_shock_lookControl = Dvar_RegisterBool("bg_shock_lookControl", 1, 0x80u, "Alter player control during shellshock");
    mincs.value.max = FLT_MAX;
    mincs.value.min = 0.0;
    bg_shock_lookControl_maxpitchspeed = Dvar_RegisterFloat(
        "bg_shock_lookControl_maxpitchspeed",
        90.0,
        mincs,
        0x80u,
        "Maximum pitch movement rate while shellshocked in degrees per second");
    minct.value.max = FLT_MAX;
    minct.value.min = 0.0;
    bg_shock_lookControl_maxyawspeed = Dvar_RegisterFloat(
        "bg_shock_lookControl_maxyawspeed",
        90.0,
        minct,
        0x80u,
        "Maximum yaw movement rate while shell shocked in degrees per second");
    mincu.value.max = 2.0;
    mincu.value.min = 0.0;
    bg_shock_lookControl_mousesensitivityscale = Dvar_RegisterFloat(
        "bg_shock_lookControl_mousesensitivityscale",
        0.5,
        mincu,
        0x80u,
        "Sensitivity scale to apply to a shellshocked player");
    mincv.value.max = 1000.0;
    mincv.value.min = 0.001;
    bg_shock_lookControl_fadeTime = Dvar_RegisterFloat(
        "bg_shock_lookControl_fadeTime",
        2.0,
        mincv,
        0x80u,
        "The time for the shellshock player control to fade in seconds");
    bg_shock_movement = Dvar_RegisterBool(
        "bg_shock_movement",
        1,
        0x80u,
        "Affect player's movement speed duringi shellshock");
    mincw.value.max = 1000.0;
    mincw.value.min = 0.0;
    player_meleeRange = Dvar_RegisterFloat(
        "player_meleeRange",
        64.0,
        mincw,
        0x80u,
        "The maximum range of the player's mellee attack");
    mincx.value.max = 1000.0;
    mincx.value.min = 0.0;
    player_meleeWidth = Dvar_RegisterFloat(
        "player_meleeWidth",
        10.0,
        mincx,
        0x80u,
        "The width of the player's melee attack");
    mincy.value.max = 1000.0;
    mincy.value.min = 0.0;
    player_meleeHeight = Dvar_RegisterFloat(
        "player_meleeHeight",
        10.0,
        mincy,
        0x80u,
        "The height of the player's melee attack");
    mincz.value.max = 5000.0;
    mincz.value.min = 1.0;
    player_meleeChargeFriction = Dvar_RegisterFloat(
        "player_meleeChargeFriction",
        1200.0,
        mincz,
        0x80u,
        "Friction used during melee charge");
    minda.value.max = 60.0;
    minda.value.min = 0.0;
    player_burstFireCooldown = Dvar_RegisterFloat(
        "player_burstFireCooldown",
        0.2,
        minda,
        0x80u,
        "Seconds after a burst fire before weapons can be fired again.");
    mindb.value.max = 1024.0;
    mindb.value.min = 0.0;
    bullet_penetrationMinFxDist = Dvar_RegisterFloat(
        "bullet_penetrationMinFxDist",
        30.0,
        mindb,
        0x80u,
        "Min distance a penetrated bullet must travel before it'll trigger the effects");
    Jump_RegisterDvars();
    Mantle_RegisterDvars();
    Perks_RegisterDvars();
}

char *__cdecl BG_GetEntityTypeName(int eType)
{
    if (eType < 17)
        return (char*)entityTypeNames[eType];

    iassert((eType - ET_EVENTS >= 0 && eType - ET_EVENTS < EV_MAX_EVENTS));

    return va("Event %s (%i)", eventnames[eType - ET_EVENTS], eType - ET_EVENTS);
}

const gitem_s *__cdecl BG_FindItemForWeapon(unsigned int weapon, int model)
{
    unsigned int NumWeapons; // eax

    bcassert(weapon, BG_GetNumWeapons());
    return &bg_itemlist[(weapon + (model * 128))];
}

const gitem_s *__cdecl G_FindItem(const char *pickupName, int model)
{
    unsigned int iIndex; // [esp+0h] [ebp-4h]

    iIndex = G_GetWeaponIndexForName(pickupName);
    if (iIndex)
        return &bg_itemlist[(iIndex + (model * 128))];
    else
        return 0;
}

bool __cdecl BG_PlayerTouchesItem(const playerState_s *ps, const entityState_s *item, int atTime)
{
    float origin[3]; // [esp+0h] [ebp-Ch] BYREF

    BG_EvaluateTrajectory(&item->lerp.pos, atTime, origin);
    return ps->origin[0] - origin[0] <= 36.0
        && ps->origin[0] - origin[0] >= -36.0
        && ps->origin[1] - origin[1] <= 36.0
        && ps->origin[1] - origin[1] >= -36.0
        && ps->origin[2] - origin[2] <= 18.0
        && ps->origin[2] - origin[2] >= -88.0;
}

bool __cdecl BG_PlayerCanPickUpWeaponType(const WeaponDef *weapDef, const playerState_s *ps)
{
    if (!weapDef)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 812, 0, "%s", "weapDef");
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 813, 0, "%s", "ps");
    if (weapDef->offhandClass == OFFHAND_CLASS_FLASH_GRENADE && ps->offhandSecondary != PLAYER_OFFHAND_SECONDARY_FLASH)
        return 0;
    return weapDef->offhandClass != OFFHAND_CLASS_SMOKE_GRENADE || ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_SMOKE;
}

bool __cdecl BG_CanItemBeGrabbed(const entityState_s *ent, const playerState_s *ps, int touched)
{
    const char *v4; // eax
    int weapIdx; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 855, 0, "%s", "ent");
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 856, 0, "%s", "ps");
    if ((ps->weapFlags & 0x80) != 0)
        return 0;
    if (ent->index.brushmodel < 1 || ent->index.brushmodel >= 2048)
    {
        v4 = va("BG_CanItemBeGrabbed: index out of range (index is %i, eType is %i)", ent->index.brushmodel, ent->eType);
        Com_Error(ERR_DROP, v4);
    }
    if (ent->clientNum == ps->clientNum)
        return 0;
    if ((ps->pm_flags & 0x100000) != 0)
        return 0;
    weapIdx = ent->index.brushmodel % 128;
    weapDef = BG_GetWeaponDef(weapIdx);
    if (bg_itemlist[128 * (ent->index.brushmodel / 128) + weapIdx].giType != IT_WEAPON)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            877,
            0,
            "%s",
            "bg_itemlist[ ITEM_WEAPMODEL( ent->index.item ) * MAX_WEAPONS + weapIdx].giType == IT_WEAPON");
    if (WeaponEntCanBeGrabbed(ent, ps, touched, weapIdx))
        return 1;
    return weapDef->altWeaponIndex && WeaponEntCanBeGrabbed(ent, ps, touched, weapDef->altWeaponIndex);
}

char __cdecl WeaponEntCanBeGrabbed(
    const entityState_s *weaponEntState,
    const playerState_s *ps,
    int touched,
    unsigned int weapIdx)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIdx);
    if (!BG_PlayerCanPickUpWeaponType(weapDef, ps))
        return 0;
    if (weaponEntState->eType == 4 && weapDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
        return 1;
    if (touched)
    {
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if ((Com_BitCheckAssert(ps->weapons, weapIdx, 16) || BG_PlayerHasCompatibleWeapon(ps, weapIdx))
            && HaveRoomForAmmo(ps, weapIdx))
        {
            return 1;
        }
    }
    else
    {
        if (!ps)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (!Com_BitCheckAssert(ps->weapons, weapIdx, 16))
            return 1;
    }
    return 0;
}

char __cdecl HaveRoomForAmmo(const playerState_s *ps, unsigned int weaponIndex)
{
    int ammoIndex; // [esp+0h] [ebp-14h]
    int weapCount; // [esp+4h] [ebp-10h]
    WeaponDef *weaponDef; // [esp+8h] [ebp-Ch]
    signed int weapIndex; // [esp+Ch] [ebp-8h]

    if (!ps)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 789, 0, "%s", "ps");
    weapCount = BG_GetNumWeapons();
    weaponDef = BG_GetWeaponDef(weaponIndex);
    ammoIndex = weaponDef->iAmmoIndex;
    if (!*weaponDef->szAmmoName)
        return 1;
    for (weapIndex = 1; weapIndex < weapCount; ++weapIndex)
    {
        if (BG_GetWeaponDef(weapIndex)->iAmmoIndex == ammoIndex && BG_GetMaxPickupableAmmo(ps, weapIndex) > 0)
            return 1;
    }
    return 0;
}

bool __cdecl BG_PlayerHasRoomForEntAllAmmoTypes(const entityState_s *ent, const playerState_s *ps)
{
    const char *v2; // eax
    int v3; // ecx
    unsigned int weapIdx; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!ent)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 893, 0, "%s", "ent");
    if (!ps)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 894, 0, "%s", "ps");
    if (ent->index.brushmodel < 1 || ent->index.brushmodel >= 2048)
    {
        v2 = va("BG_PlayerHasRoomForAllAmmoTypesOfEnt: index out of range (index is %i, eType is %i)", ent->index.brushmodel, ent->eType);
        Com_Error(ERR_DROP, v2);
    }
    v3 = ent->index.brushmodel % 128;
    weapIdx = v3;
    if (!v3)
        return 0;
    if (bg_itemlist[128 * (ent->index.brushmodel / 128) + v3].giType != IT_WEAPON)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            902,
            0,
            "%s",
            "bg_itemlist[ ITEM_WEAPMODEL( ent->index.item ) * MAX_WEAPONS + weapIdx].giType == IT_WEAPON");
    if (!BG_GetMaxPickupableAmmo(ps, weapIdx))
        return 0;
    weapDef = BG_GetWeaponDef(weapIdx);
    return !weapDef->altWeaponIndex || BG_GetMaxPickupableAmmo(ps, weapDef->altWeaponIndex);
}

void __cdecl BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, float *result)
{
    float v3; // [esp+Ch] [ebp-7Ch]
    float v4; // [esp+14h] [ebp-74h]
    float scale; // [esp+24h] [ebp-64h]
    float v6; // [esp+28h] [ebp-60h]
    float phase; // [esp+74h] [ebp-14h]
    float phasea; // [esp+74h] [ebp-14h]
    float deltaTimea; // [esp+78h] [ebp-10h]
    float deltaTimeb; // [esp+78h] [ebp-10h]
    float deltaTime; // [esp+78h] [ebp-10h]
    float deltaTimec; // [esp+78h] [ebp-10h]
    float deltaTimed; // [esp+78h] [ebp-10h]
    float deltaTimee; // [esp+78h] [ebp-10h]
    float v[3]; // [esp+7Ch] [ebp-Ch] BYREF

    if (!tr)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 921, 0, "%s", "tr");
    if (!(!IS_NAN((tr->trBase)[0]) && !IS_NAN((tr->trBase)[1]) && !IS_NAN((tr->trBase)[2])))
    {
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            922,
            0,
            "%s",
            "!IS_NAN((tr->trBase)[0]) && !IS_NAN((tr->trBase)[1]) && !IS_NAN((tr->trBase)[2])");
    }
    if (!(!IS_NAN((tr->trDelta)[0]) && !IS_NAN((tr->trDelta)[1]) && !IS_NAN((tr->trDelta)[2])))
    {
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            923,
            0,
            "%s",
            "!IS_NAN((tr->trDelta)[0]) && !IS_NAN((tr->trDelta)[1]) && !IS_NAN((tr->trDelta)[2])");
    }
    switch (tr->trType)
    {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
    case TR_PHYSICS:
    case TR_RAGDOLL_INTERPOLATE:
        *result = tr->trBase[0];
        result[1] = tr->trBase[1];
        result[2] = tr->trBase[2];
        break;
    case TR_LINEAR:
    case TR_FIRST_RAGDOLL:
        deltaTimea = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        Vec3Mad(tr->trBase, deltaTimea, tr->trDelta, result);
        break;
    case TR_LINEAR_STOP:
        if (atTime > tr->trDuration + tr->trTime)
            atTime = tr->trDuration + tr->trTime;
        deltaTime = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        if (deltaTime < 0.0)
            deltaTime = 0.0;
        Vec3Mad(tr->trBase, deltaTime, tr->trDelta, result);
        break;
    case TR_SINE:
        deltaTimeb = (double)(atTime - tr->trTime) / (double)tr->trDuration;
        v6 = deltaTimeb * 3.141592741012573 + deltaTimeb * 3.141592741012573;
        scale = sin(v6);
        Vec3Mad(tr->trBase, scale, tr->trDelta, result);
        break;
    case TR_GRAVITY:
    case TR_RAGDOLL_GRAVITY:
        deltaTimec = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        Vec3Mad(tr->trBase, deltaTimec, tr->trDelta, result);
        result[2] = result[2] - deltaTimec * 400.0 * deltaTimec;
        break;
    case TR_ACCELERATE:
        if (atTime > tr->trDuration + tr->trTime)
            atTime = tr->trDuration + tr->trTime;
        deltaTimed = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        phase = Vec3Length(tr->trDelta) / ((double)tr->trDuration * EQUAL_EPSILON);
        Vec3NormalizeTo(tr->trDelta, result);
        v4 = phase * 0.5 * deltaTimed * deltaTimed;
        Vec3Mad(tr->trBase, v4, result, result);
        break;
    case TR_DECELERATE:
        if (atTime > tr->trDuration + tr->trTime)
            atTime = tr->trDuration + tr->trTime;
        deltaTimee = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        phasea = Vec3Length(tr->trDelta) / ((double)tr->trDuration * EQUAL_EPSILON);
        Vec3NormalizeTo(tr->trDelta, result);
        Vec3Mad(tr->trBase, deltaTimee, tr->trDelta, v);
        v3 = -phasea * 0.5 * deltaTimee * deltaTimee;
        Vec3Mad(v, v3, result, result);
        break;
    default:
        Com_Error(ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trType);
        break;
    }
    if ((COERCE_UNSIGNED_INT(tr->trBase[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trBase[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trBase[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            998,
            0,
            "%s",
            "!IS_NAN((tr->trBase)[0]) && !IS_NAN((tr->trBase)[1]) && !IS_NAN((tr->trBase)[2])");
    }
    if ((COERCE_UNSIGNED_INT(tr->trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            999,
            0,
            "%s",
            "!IS_NAN((tr->trDelta)[0]) && !IS_NAN((tr->trDelta)[1]) && !IS_NAN((tr->trDelta)[2])");
    }
}

void __cdecl BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, float *result)
{
    float scale; // [esp+Ch] [ebp-60h]
    float v4; // [esp+18h] [ebp-54h]
    float v5; // [esp+1Ch] [ebp-50h]
    float phase; // [esp+64h] [ebp-8h]
    float deltaTime; // [esp+68h] [ebp-4h]
    float deltaTimea; // [esp+68h] [ebp-4h]
    float deltaTimeb; // [esp+68h] [ebp-4h]
    float deltaTimec; // [esp+68h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(tr->trBase[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trBase[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trBase[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1015,
            0,
            "%s",
            "!IS_NAN((tr->trBase)[0]) && !IS_NAN((tr->trBase)[1]) && !IS_NAN((tr->trBase)[2])");
    }
    if ((COERCE_UNSIGNED_INT(tr->trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tr->trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1016,
            0,
            "%s",
            "!IS_NAN((tr->trDelta)[0]) && !IS_NAN((tr->trDelta)[1]) && !IS_NAN((tr->trDelta)[2])");
    }
    switch (tr->trType)
    {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
        *result = 0.0;
        result[1] = 0.0;
        result[2] = 0.0;
        goto LABEL_22;
    case TR_LINEAR:
        *result = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2];
        goto LABEL_22;
    case TR_LINEAR_STOP:
        if (atTime > tr->trDuration + tr->trTime)
            goto LABEL_14;
        *result = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2];
        goto LABEL_22;
    case TR_SINE:
        deltaTime = (double)(atTime - tr->trTime) / (double)tr->trDuration;
        v5 = deltaTime * 3.141592741012573 + deltaTime * 3.141592741012573;
        v4 = cos(v5);
        phase = v4 * 0.5;
        Vec3Scale(tr->trDelta, phase, result);
        goto LABEL_22;
    case TR_GRAVITY:
        deltaTimea = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        *result = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2];
        result[2] = result[2] - deltaTimea * 800.0;
        goto LABEL_22;
    case TR_ACCELERATE:
        if (atTime > tr->trDuration + tr->trTime)
        {
        LABEL_14:
            *result = 0.0;
            result[1] = 0.0;
            result[2] = 0.0;
            return;
        }
        deltaTimeb = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
        scale = deltaTimeb * deltaTimeb;
        Vec3Scale(tr->trDelta, scale, result);
        goto LABEL_22;
    case TR_DECELERATE:
        if (atTime <= tr->trDuration + tr->trTime)
        {
            deltaTimec = (double)(atTime - tr->trTime) * EQUAL_EPSILON;
            Vec3Scale(tr->trDelta, deltaTimec, result);
        LABEL_22:
            if ((COERCE_UNSIGNED_INT(tr->trBase[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(tr->trBase[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(tr->trBase[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\bgame\\bg_misc.cpp",
                    1069,
                    0,
                    "%s",
                    "!IS_NAN((tr->trBase)[0]) && !IS_NAN((tr->trBase)[1]) && !IS_NAN((tr->trBase)[2])");
            }
            if ((COERCE_UNSIGNED_INT(tr->trDelta[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(tr->trDelta[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(tr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\bgame\\bg_misc.cpp",
                    1070,
                    0,
                    "%s",
                    "!IS_NAN((tr->trDelta)[0]) && !IS_NAN((tr->trDelta)[1]) && !IS_NAN((tr->trDelta)[2])");
            }
        }
        else
        {
            *result = 0.0;
            result[1] = 0.0;
            result[2] = 0.0;
        }
        return;
    default:
        Com_Error(ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trType);
        goto LABEL_22;
    }
}

void __cdecl BG_AddPredictableEventToPlayerstate(unsigned int newEvent, unsigned int eventParm, playerState_s *ps)
{
    if (newEvent)
    {
        if (newEvent >= 0x100)
            MyAssertHandler(
                ".\\bgame\\bg_misc.cpp",
                1125,
                0,
                "newEvent doesn't index 256\n\t%i not in [0, %i)",
                newEvent,
                256);
        if (eventParm > 0xFF)
            MyAssertHandler(".\\bgame\\bg_misc.cpp", 1126, 0, "%s", "eventParm <= EVENT_PARM_MAX");
        if (Dvar_GetBool("showevents"))
            Com_Printf(
                17,
                "event svt %5d -> %5d: num = %20s parm %d\n",
                ps->commandTime,
                ps->eventSequence,
                eventnames[newEvent],
                eventParm);
        ps->events[ps->eventSequence & 3] = (unsigned __int8)newEvent;
        ps->eventParms[ps->eventSequence & 3] = (unsigned __int8)eventParm;
        ps->eventSequence = (unsigned __int8)(ps->eventSequence + 1);
    }
}

void __cdecl BG_PlayerStateToEntityState(playerState_s *ps, entityState_s *s, int snap, unsigned __int8 handler)
{
    BG_PlayerToEntitySetTrajectory(ps, s, snap);
    BG_PlayerToEntitySetFlags(ps, s);
    BG_PlayerToEntitySetMisc(ps, s);
    BG_PlayerToEntitySetPitchAngles(ps, s);
    BG_PlayerToEntityEventParm(ps, s);
    BG_PlayerToEntityProcessEvents(ps, s, handler);
}

void __cdecl BG_PlayerToEntityEventParm(playerState_s *ps, entityState_s *s)
{
    int v2; // [esp+4h] [ebp-Ch]
    int entityEventSequence; // [esp+8h] [ebp-8h]
    int seq; // [esp+Ch] [ebp-4h]

    entityEventSequence = ps->entityEventSequence;
    if (entityEventSequence <= ps->eventSequence + 64)
        v2 = ps->entityEventSequence;
    else
        v2 = entityEventSequence - 256;
    ps->entityEventSequence = v2;
    if (ps->entityEventSequence - ps->eventSequence >= 0)
    {
        s->eventParm = 0;
    }
    else
    {
        if (ps->eventSequence - ps->entityEventSequence > 4)
            ps->entityEventSequence = ps->eventSequence - 4;
        seq = ps->entityEventSequence & 3;
        if (ps->eventParms[seq] > 0xFF)
            MyAssertHandler(
                ".\\bgame\\bg_misc.cpp",
                1155,
                0,
                "%s",
                "(ps->eventParms[seq] >= 0) && (ps->eventParms[seq] <= EVENT_PARM_MAX)");
        s->eventParm = LOBYTE(ps->eventParms[seq]);
        ps->entityEventSequence = (unsigned __int8)(ps->entityEventSequence + 1);
    }
    if (s->eventParm > 0xFF)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1167,
            0,
            "%s\n\t(s->eventParm) = %i",
            "(s->eventParm <= 0xff)",
            s->eventParm);
}

void __cdecl BG_PlayerToEntityProcessEvents(playerState_s *ps, entityState_s *s, unsigned __int8 handler)
{
    int j; // [esp+4h] [ebp-10h]
    int ja; // [esp+4h] [ebp-10h]
    unsigned __int8 event; // [esp+Bh] [ebp-9h]
    void(__cdecl * playerEvent)(int, int); // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (ps->eventSequence - ps->oldEventSequence > 4)
        ps->oldEventSequence = ps->eventSequence - 4;
    if (ps->oldEventSequence - ps->eventSequence > 0)
        ps->oldEventSequence = ps->eventSequence;
    for (i = ps->oldEventSequence; i != ps->eventSequence; ++i)
    {
        event = ps->events[i & 3];
        playerEvent = pmoveHandlers[handler].playerEvent;
        if (playerEvent)
            playerEvent(s->number, event);
        for (j = 0; serverOnlyEvents[j] > 0 && serverOnlyEvents[j] != event; ++j)
            ;
        if (serverOnlyEvents[j] < 0)
        {
            for (ja = 0; singleClientEvents[ja] > 0 && singleClientEvents[ja] != event; ++ja)
                ;
            if (singleClientEvents[ja] < 0)
            {
                s->events[s->eventSequence & 3] = event;
                s->eventParms[s->eventSequence & 3] = LOBYTE(ps->eventParms[i & 3]);
                s->eventSequence = (unsigned __int8)(s->eventSequence + 1);
            }
        }
    }
    ps->oldEventSequence = ps->eventSequence;
}

void __cdecl BG_PlayerToEntitySetFlags(playerState_s *ps, entityState_s *s)
{
    int v2; // edx
    int v3; // ecx

    s->lerp.eFlags = ps->eFlags;
    if (ps->pm_type < 7)
        v2 = s->lerp.eFlags & 0xFFFDFFFF;
    else
        v2 = s->lerp.eFlags | 0x20000;
    s->lerp.eFlags = v2;
    if ((ps->pm_flags & 0x10) != 0)
        v3 = s->lerp.eFlags | 0x40000;
    else
        v3 = s->lerp.eFlags & 0xFFFBFFFF;
    s->lerp.eFlags = v3;
}

void __cdecl BG_PlayerToEntitySetPitchAngles(playerState_s *ps, entityState_s *s)
{
    float v2; // [esp+8h] [ebp-34h]
    float v3; // [esp+Ch] [ebp-30h]
    float v4; // [esp+10h] [ebp-2Ch]
    float v5; // [esp+14h] [ebp-28h]
    float v6; // [esp+18h] [ebp-24h]
    float v7; // [esp+1Ch] [ebp-20h]
    float v8; // [esp+28h] [ebp-14h]
    float v9; // [esp+30h] [ebp-Ch]
    float fLerpFrac; // [esp+38h] [ebp-4h]

    if (PM_GetEffectiveStance(ps) == 1)
    {
        if (ps->viewHeightLerpTime)
        {
            fLerpFrac = (double)(ps->commandTime - ps->viewHeightLerpTime)
                / (double)PM_GetViewHeightLerpTime(ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
            if (fLerpFrac >= 0.0)
            {
                if (fLerpFrac > 1.0)
                    fLerpFrac = 1.0;
            }
            else
            {
                fLerpFrac = 0.0;
            }
            if (!ps->viewHeightLerpDown)
                fLerpFrac = 1.0 - fLerpFrac;
        }
        else
        {
            fLerpFrac = 1.0;
        }
        v9 = ps->fTorsoPitch * 0.002777777845039964;
        v7 = v9 + 0.5;
        v6 = floor(v7);
        v5 = (v9 - v6) * 360.0;
        s->fTorsoPitch = v5 * fLerpFrac;
        v8 = ps->fWaistPitch * 0.002777777845039964;
        v4 = v8 + 0.5;
        v3 = floor(v4);
        v2 = (v8 - v3) * 360.0;
        s->fWaistPitch = v2 * fLerpFrac;
    }
    else
    {
        s->fTorsoPitch = 0.0;
        s->fWaistPitch = 0.0;
    }
}

void __cdecl BG_PlayerToEntitySetMisc(playerState_s *ps, entityState_s *s)
{
    s->legsAnim = ps->legsAnim;
    s->torsoAnim = ps->torsoAnim;
    s->lerp.u.turret.gunAngles[0] = ps->leanf;
    s->clientNum = ps->clientNum;
    if ((ps->eFlags & 0x300) != 0)
        s->otherEntityNum = ps->viewlocked_entNum;
    if ((ps->otherFlags & 6) != 0)
        s->eType = 1;
    else
        s->eType = 5;
    s->weapon = LOBYTE(ps->weapon);
    s->weaponModel = ps->weaponmodels[ps->weapon];
    s->groundEntityNum = LOWORD(ps->groundEntityNum);
}

void __cdecl BG_PlayerToEntitySetTrajectory(playerState_s *ps, entityState_s *s, int snap)
{
    s->lerp.pos.trType = TR_INTERPOLATE;
    s->lerp.pos.trDuration = 0;
    s->lerp.pos.trTime = 0;
    s->lerp.pos.trDelta[0] = 0.0;
    s->lerp.pos.trDelta[1] = 0.0;
    s->lerp.pos.trDelta[2] = 0.0;
    s->lerp.pos.trBase[0] = ps->origin[0];
    s->lerp.pos.trBase[1] = ps->origin[1];
    s->lerp.pos.trBase[2] = ps->origin[2];
    s->lerp.apos.trType = TR_INTERPOLATE;
    s->lerp.apos.trDuration = 0;
    s->lerp.apos.trTime = 0;
    s->lerp.apos.trDelta[0] = 0.0;
    s->lerp.apos.trDelta[1] = 0.0;
    s->lerp.apos.trDelta[2] = 0.0;
    s->lerp.apos.trBase[0] = ps->viewangles[0];
    s->lerp.apos.trBase[1] = ps->viewangles[1];
    s->lerp.apos.trBase[2] = ps->viewangles[2];
    if (ps->movementDir >= 128 || ps->movementDir < -128)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1352,
            0,
            "%s\n\t(ps->movementDir) = %i",
            "(ps->movementDir < 128 && ps->movementDir >= -128)",
            ps->movementDir);
    s->lerp.u.loopFx.period = ps->movementDir;
    if (snap)
    {
        s->lerp.pos.trBase[0] = (float)(int)s->lerp.pos.trBase[0];
        s->lerp.pos.trBase[1] = (float)(int)s->lerp.pos.trBase[1];
        s->lerp.pos.trBase[2] = (float)(int)s->lerp.pos.trBase[2];
        s->lerp.apos.trBase[0] = (float)(int)s->lerp.apos.trBase[0];
        s->lerp.apos.trBase[1] = (float)(int)s->lerp.apos.trBase[1];
        s->lerp.apos.trBase[2] = (float)(int)s->lerp.apos.trBase[2];
    }
}

char __cdecl BG_CheckProneValid(
    int passEntityNum,
    const float *vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float *pfTorsoPitch,
    float *pfWaistPitch,
    bool isAlreadyProne,
    bool isOnGround,
    bool groundIsWalkable,
    unsigned __int8 handler,
    proneCheckType_t proneCheckType,
    float prone_feet_dist)
{
    float v14; // [esp+Ch] [ebp-100h]
    float v15; // [esp+10h] [ebp-FCh]
    float v16; // [esp+14h] [ebp-F8h]
    float v17; // [esp+18h] [ebp-F4h]
    float v18; // [esp+1Ch] [ebp-F0h]
    float v19; // [esp+20h] [ebp-ECh]
    float scale; // [esp+24h] [ebp-E8h]
    float v21; // [esp+2Ch] [ebp-E0h]
    float v22; // [esp+30h] [ebp-DCh]
    float v23; // [esp+34h] [ebp-D8h]
    float v24; // [esp+38h] [ebp-D4h]
    float vFeetPos[3]; // [esp+48h] [ebp-C4h] BYREF
    int bFirstTraceHit; // [esp+54h] [ebp-B8h]
    float fWaistTraceDist; // [esp+58h] [ebp-B4h]
    void(__cdecl * traceFunc)(trace_t *, const float *, const float *, const float *, const float *, int, int); // [esp+5Ch] [ebp-B0h]
    float vEnd[3]; // [esp+60h] [ebp-ACh] BYREF
    float fTraceHeight; // [esp+6Ch] [ebp-A0h]
    float vTorsoPos[3]; // [esp+70h] [ebp-9Ch] BYREF
    bool success; // [esp+7Fh] [ebp-8Dh]
    trace_t trace; // [esp+80h] [ebp-8Ch] BYREF
    float vMins[3]; // [esp+ACh] [ebp-60h] BYREF
    float fTorsoPitch; // [esp+B8h] [ebp-54h]
    int iTraceMask; // [esp+BCh] [ebp-50h]
    float fWaistPitch; // [esp+C0h] [ebp-4Ch]
    float vForward[3]; // [esp+C4h] [ebp-48h] BYREF
    float fPitchDiff; // [esp+D0h] [ebp-3Ch]
    float fTraceRealHeight; // [esp+D4h] [ebp-38h]
    float fFirstTraceDist; // [esp+D8h] [ebp-34h]
    float vStart[3]; // [esp+DCh] [ebp-30h] BYREF
    float vWaistPos[3]; // [esp+E8h] [ebp-24h] BYREF
    float vMaxs[3]; // [esp+F4h] [ebp-18h] BYREF
    float vDelta[3]; // [esp+100h] [ebp-Ch] BYREF

    bFirstTraceHit = 0;
    success = 1;
    traceFunc = pmoveHandlers[handler].trace;
    if (!traceFunc)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 1435, 0, "%s", "traceFunc");
    if (proneCheckType)
        iTraceMask = 0x820011;
    else
        iTraceMask = 0x810011;
    if (!isAlreadyProne)
    {
        vMins[0] = -fSize;
        vMins[1] = vMins[0];
        vMins[2] = 0.0;
        vMaxs[0] = fSize;
        vMaxs[1] = fSize;
        vMaxs[2] = fHeight;
        vStart[0] = *vPos;
        vStart[1] = vPos[1];
        vStart[2] = vPos[2];
        vEnd[0] = *vPos;
        vEnd[1] = vPos[1];
        vEnd[2] = vPos[2];
        vEnd[2] = vEnd[2] + 10.0;
        traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
        if (trace.allsolid)
            return 0;
    }
    if (isOnGround && !groundIsWalkable)
        return 0;
    vMins[0] = -6.0;
    vMins[1] = -6.0;
    vMins[2] = -6.0;
    vMaxs[0] = 6.0;
    vMaxs[1] = 6.0;
    vMaxs[2] = 6.0;
    vEnd[0] = 0.0;
    vEnd[1] = fYaw - 180.0;
    vEnd[2] = 0.0;
    AngleVectors(vEnd, vForward, 0, 0);
    fTraceHeight = fHeight - 6.0;
    fTraceRealHeight = fTraceHeight - 6.0;
    vStart[0] = *vPos;
    vStart[1] = vPos[1];
    vStart[2] = vPos[2];
    vStart[2] = vStart[2] + fTraceHeight;
    scale = prone_feet_dist - 6.0;
    Vec3Mad(vStart, scale, vForward, vEnd);
    traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
    if (trace.fraction >= 1.0)
    {
        fFirstTraceDist = prone_feet_dist;
    }
    else
    {
        if (!isOnGround)
            return 0;
        bFirstTraceHit = 1;
        fFirstTraceDist = (prone_feet_dist - 6.0) * trace.fraction + 6.0;
        v19 = fSize + 2.0;
        if (fFirstTraceDist < (double)v19)
            return 0;
        if (fFirstTraceDist < fTraceHeight * 0.699999988079071 + 18.0)
        {
            bFirstTraceHit = 0;
            vEnd[2] = vEnd[2] + 22.0;
            Vec3Sub(vEnd, vStart, vDelta);
            fPitchDiff = Vec3NormalizeTo(vDelta, vForward);
            traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
            if (trace.fraction >= 1.0)
            {
                fFirstTraceDist = prone_feet_dist;
            }
            else
            {
                bFirstTraceHit = 1;
                fFirstTraceDist = trace.fraction * fPitchDiff + 6.0;
                if (fFirstTraceDist < fTraceHeight * 0.699999988079071 + 18.0)
                    return 0;
            }
        }
    }
    Vec3Lerp(vStart, vEnd, trace.fraction, vFeetPos);
    Vec3Mad(vPos, 18.0, vForward, vStart);
    vStart[2] = vStart[2] + fTraceHeight;
    vEnd[0] = vStart[0];
    vEnd[1] = vStart[1];
    vEnd[2] = vStart[2] - (fSize * 2.5 + fTraceHeight - 6.0);
    traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
    if (trace.fraction == 1.0)
        goto fail;
    if (!trace.walkable)
        return 0;
    fWaistTraceDist = (fSize * 2.5 + fTraceHeight - 6.0) * trace.fraction + 6.0;
    Vec3Lerp(vStart, vEnd, trace.fraction, vWaistPos);
    vWaistPos[2] = vWaistPos[2] - 6.0;
    if (bFirstTraceHit)
    {
        if (fWaistTraceDist * -0.75 > fFirstTraceDist - fWaistTraceDist)
            goto fail;
        Vec3Sub(vFeetPos, vWaistPos, vDelta);
        Vec3Mad(vDelta, 6.0, vForward, vDelta);
        vDelta[2] = vDelta[2] + 6.0;
        Vec3Normalize(vDelta);
        v18 = prone_feet_dist - 6.0 - 18.0;
        Vec3Mad(vStart, v18, vDelta, vEnd);
        vEnd[0] = ((prone_feet_dist - 6.0) * vForward[0] + *vPos + vEnd[0]) * 0.5;
        vEnd[1] = ((prone_feet_dist - 6.0) * vForward[1] + vPos[1] + vEnd[1]) * 0.5;
        traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
        if (trace.fraction < 1.0)
        {
            Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
            vStart[2] = vStart[2] + 18.0;
            vEnd[2] = vEnd[2] + 18.0;
            traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
            if (trace.fraction < 1.0)
                goto fail;
        }
        Vec3Lerp(vStart, vEnd, trace.fraction, vFeetPos);
    }
    vStart[0] = vFeetPos[0];
    vStart[1] = vFeetPos[1];
    vStart[2] = vFeetPos[2];
    vEnd[0] = vFeetPos[0];
    vEnd[1] = vFeetPos[1];
    vEnd[2] = vFeetPos[2] - (vFeetPos[2] - vWaistPos[2] + vFeetPos[2] - vWaistPos[2] + fSize * 1.0);
    traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
    if (trace.fraction != 1.0)
    {
        if (!trace.walkable)
            return 0;
        Vec3Lerp(vStart, vEnd, trace.fraction, vFeetPos);
        vFeetPos[2] = vFeetPos[2] - 6.0;
        vTorsoPos[0] = *vPos;
        vTorsoPos[1] = vPos[1];
        vTorsoPos[2] = vPos[2];
        Vec3Sub(vTorsoPos, vWaistPos, vDelta);
        v23 = vectopitch(vDelta);
        v24 = v23 * 0.002777777845039964;
        v17 = v24 + 0.5;
        v16 = floor(v17);
        fTorsoPitch = (v24 - v16) * 360.0;
        Vec3Sub(vWaistPos, vFeetPos, vDelta);
        v21 = vectopitch(vDelta);
        v22 = v21 * 0.002777777845039964;
        v15 = v22 + 0.5;
        v14 = floor(v15);
        fWaistPitch = (v22 - v14) * 360.0;
        fPitchDiff = AngleDelta(fTorsoPitch, fWaistPitch);
        if (fPitchDiff < -50.0 || fPitchDiff > 70.0)
            success = 0;
        vMins[0] = -0.0;
        vMins[1] = -0.0;
        vMins[2] = -0.0;
        vMaxs[0] = 0.0;
        vMaxs[1] = 0.0;
        vMaxs[2] = 0.0;
        vStart[0] = vTorsoPos[0];
        vStart[1] = vTorsoPos[1];
        vStart[2] = vTorsoPos[2] + 5.0;
        vEnd[0] = vWaistPos[0];
        vEnd[1] = vWaistPos[1];
        vEnd[2] = vWaistPos[2] + 5.0;
        traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
        if (trace.fraction < 1.0)
            success = 0;
        vStart[0] = vEnd[0];
        vStart[1] = vEnd[1];
        vStart[2] = vEnd[2];
        vEnd[0] = vFeetPos[0];
        vEnd[1] = vFeetPos[1];
        vEnd[2] = vFeetPos[2] + 5.0;
        traceFunc(&trace, vStart, vMins, vMaxs, vEnd, passEntityNum, iTraceMask);
        if (trace.fraction < 1.0)
            success = 0;
        if (pfTorsoPitch)
            *pfTorsoPitch = fTorsoPitch;
        if (pfWaistPitch)
            *pfWaistPitch = fWaistPitch;
        if (success)
            return 1;
    }
fail:
    if (isOnGround)
        return 0;
    if (pfTorsoPitch)
        *pfTorsoPitch = 0.0;
    if (pfWaistPitch)
        *pfWaistPitch = 0.0;
    return 1;
}

void __cdecl BG_GetPlayerViewOrigin(const playerState_s *ps, float *origin, int time)
{
    float v3; // [esp+10h] [ebp-24h]
    float delta; // [esp+18h] [ebp-1Ch]
    float fBobCycle; // [esp+1Ch] [ebp-18h]
    float vRight[3]; // [esp+20h] [ebp-14h] BYREF
    float xyspeed; // [esp+2Ch] [ebp-8h]
    float deltaB; // [esp+30h] [ebp-4h]

    if ((ps->eFlags & 0x300) != 0)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 1685, 0, "%s", "!( ps->eFlags & EF_TURRET_ACTIVE )");
    *origin = ps->origin[0];
    origin[1] = ps->origin[1];
    origin[2] = ps->origin[2];
    origin[2] = origin[2] + ps->viewHeightCurrent;
    fBobCycle = BG_GetBobCycle(ps);
    xyspeed = BG_GetSpeed(ps, time);
    delta = BG_GetVerticalBobFactor(ps, fBobCycle, xyspeed, bg_bobMax->current.value);
    origin[2] = origin[2] + delta;
    deltaB = BG_GetHorizontalBobFactor(ps, fBobCycle, xyspeed, bg_bobMax->current.value);
    BG_GetPlayerViewDirection(ps, 0, vRight, 0);
    Vec3Mad(origin, deltaB, vRight, origin);
    AddLeanToPosition(origin, ps->viewangles[1], ps->leanf, 16.0, 20.0);
    v3 = ps->origin[2] + 8.0;
    if (origin[2] < (double)v3)
        origin[2] = ps->origin[2] + 8.0;
}

void __cdecl BG_GetPlayerViewDirection(const playerState_s *ps, float *forward, float *right, float *up)
{
    AngleVectors(ps->viewangles, forward, right, up);
}

char __cdecl BG_CheckProne(
    int passEntityNum,
    const float *vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float *pfTorsoPitch,
    float *pfWaistPitch,
    bool isAlreadyProne,
    bool isOnGround,
    bool groundIsWalkable,
    unsigned __int8 handler,
    proneCheckType_t proneCheckType,
    float prone_feet_dist)
{
    return BG_CheckProneValid(
        passEntityNum,
        vPos,
        fSize,
        fHeight,
        fYaw,
        pfTorsoPitch,
        pfWaistPitch,
        isAlreadyProne,
        isOnGround,
        groundIsWalkable,
        handler,
        proneCheckType,
        prone_feet_dist);
}

void __cdecl BG_LerpHudColors(const hudelem_s *elem, int time, hudelem_color_t *toColor)
{
    float lerp; // [esp+58h] [ebp-8h]
    int timeSinceFadeStarted; // [esp+5Ch] [ebp-4h]

    timeSinceFadeStarted = time - elem->fadeStartTime;
    if (elem->fadeTime <= 0 || timeSinceFadeStarted >= elem->fadeTime)
    {
        *toColor = elem->color;
    }
    else
    {
        if (timeSinceFadeStarted < 0)
            timeSinceFadeStarted = 0;
        lerp = (double)timeSinceFadeStarted / (double)elem->fadeTime;
        if (lerp < 0.0 || lerp > 1.0)
            MyAssertHandler(".\\bgame\\bg_misc.cpp", 1890, 0, "%s\n\t(lerp) = %g", "(lerp >= 0.0f && lerp <= 1.0f)", lerp);
        toColor->r = (int)((double)elem->fromColor.r + (double)(elem->color.r - elem->fromColor.r) * lerp);
        toColor->g = (int)((double)elem->fromColor.g + (double)(elem->color.g - elem->fromColor.g) * lerp);
        toColor->b = (int)((double)elem->fromColor.b + (double)(elem->color.b - elem->fromColor.b) * lerp);
        toColor->a = (int)((double)elem->fromColor.a + (double)(elem->color.a - elem->fromColor.a) * lerp);
    }
}

int __cdecl BG_LoadShellShockDvars(const char *name)
{
    int EntChannelCount; // eax
    char *filebuf; // [esp+0h] [ebp-1C4h]
    const char *bg_shock_dvar_names[91]; // [esp+4h] [ebp-1C0h] BYREF
    char fullpath[68]; // [esp+174h] [ebp-50h] BYREF
    int success; // [esp+1BCh] [ebp-8h]
    int i; // [esp+1C0h] [ebp-4h]

    Com_sprintf(fullpath, 0x40u, "shock/%s.shock", name);
    filebuf = Com_LoadRawTextFile(fullpath);
    if (filebuf
        || (Com_PrintError(17, "couldn't open '%s'.\n", fullpath),
            (filebuf = Com_LoadRawTextFile("shock/default.shock")) != 0))
    {
        for (i = 0; i < 27; ++i)
            bg_shock_dvar_names[i] = bgShockDvarNames[i];
        for (i = 0; i < SND_GetEntChannelCount(); ++i)
            bg_shock_dvar_names[i + 27] = bgShockChannelNames[i];
        EntChannelCount = SND_GetEntChannelCount();
        success = Com_LoadDvarsFromBuffer(bg_shock_dvar_names, EntChannelCount + 27, filebuf, fullpath);
        Com_UnloadRawTextFile(filebuf);
        return success;
    }
    else
    {
        Com_PrintError(17, "couldn't open 'shock/default.shock'. This is a default shock file that you should have.\n");
        return 0;
    }
}

void __cdecl BG_SetShellShockParmsFromDvars(shellshock_parms_t *parms)
{
    unsigned __int8 *v1; // eax
    float v2; // [esp+8h] [ebp-ECh]
    float v3; // [esp+Ch] [ebp-E8h]
    float v4; // [esp+10h] [ebp-E4h]
    float v5; // [esp+14h] [ebp-E0h]
    float v6; // [esp+18h] [ebp-DCh]
    float v7; // [esp+20h] [ebp-D4h]
    float v8; // [esp+30h] [ebp-C4h]
    float v9; // [esp+34h] [ebp-C0h]
    float v10; // [esp+3Ch] [ebp-B8h]
    float v11; // [esp+50h] [ebp-A4h]
    float v12; // [esp+64h] [ebp-90h]
    float v13; // [esp+78h] [ebp-7Ch]
    float v14; // [esp+8Ch] [ebp-68h]
    float value; // [esp+9Ch] [ebp-58h]
    float v16; // [esp+A4h] [ebp-50h]
    float v17; // [esp+B8h] [ebp-3Ch]
    float v18; // [esp+CCh] [ebp-28h]
    float v19; // [esp+E0h] [ebp-14h]
    int i; // [esp+F0h] [ebp-4h]

    if (!parms)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 1943, 0, "%s", "parms");
    parms->screenBlend.blurredEffectTime = (int)(bg_shock_screenBlurBlendTime->current.value * 1000.0f);
    parms->screenBlend.blurredFadeTime = (int)(bg_shock_screenBlurBlendFadeTime->current.value * 1000.0f);
    parms->screenBlend.flashShotFadeTime = (int)(bg_shock_screenFlashShotFadeTime->current.value * 1000.0f);
    parms->screenBlend.flashWhiteFadeTime = (int)(bg_shock_screenFlashWhiteFadeTime->current.value * 1000.0f);
    if (parms->screenBlend.blurredFadeTime <= 0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1949,
            0,
            "%s\n\t(parms->screenBlend.blurredFadeTime) = %i",
            "(parms->screenBlend.blurredFadeTime > 0)",
            parms->screenBlend.blurredFadeTime);
    if (parms->screenBlend.blurredEffectTime <= 0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1950,
            0,
            "%s\n\t(parms->screenBlend.blurredEffectTime) = %i",
            "(parms->screenBlend.blurredEffectTime > 0)",
            parms->screenBlend.blurredEffectTime);
    parms->screenBlend.type = (ShockViewTypes)bg_shock_screenType->current.integer;
    parms->view.fadeTime = 3000;
    value = bg_shock_viewKickPeriod->current.value;
    v6 = 0.001 - value;
    if (v6 < 0.0)
        v5 = value;
    else
        v5 = 0.001;
    parms->view.kickRate = EQUAL_EPSILON / v5;
    parms->view.kickRadius = bg_shock_viewKickRadius->current.value;
    parms->sound.affect = bg_shock_sound->current.enabled;
    strncpy((unsigned __int8 *)parms->sound.loop, (unsigned __int8 *)bg_shock_soundLoop->current.integer, 0x40u);
    strncpy(
        (unsigned __int8 *)parms->sound.loopSilent,
        (unsigned __int8 *)bg_shock_soundLoopSilent->current.integer,
        0x40u);
    strncpy((unsigned __int8 *)parms->sound.end, (unsigned __int8 *)bg_shock_soundEnd->current.integer, 0x40u);
    strncpy((unsigned __int8 *)parms->sound.endAbort, (unsigned __int8 *)bg_shock_soundEndAbort->current.integer, 0x40u);
    parms->sound.fadeInTime = (int)(bg_shock_soundFadeInTime->current.value * 1000.0f);
    parms->sound.fadeOutTime = (int)(bg_shock_soundFadeOutTime->current.value * 1000.0f);
    parms->sound.loopFadeTime = (int)(bg_shock_soundLoopFadeTime->current.value * 1000.0f);
    parms->sound.loopEndDelay = (int)(bg_shock_soundLoopEndDelay->current.value * 1000.0f);
    v1 = (unsigned __int8 *)Dvar_EnumToString(bg_shock_soundRoomType);
    strncpy((unsigned __int8 *)parms->sound.roomtype, v1, 0xFu);
    parms->sound.roomtype[15] = 0;
    parms->sound.drylevel = bg_shock_soundDryLevel->current.value;
    parms->sound.wetlevel = bg_shock_soundWetLevel->current.value;
    parms->sound.modEndDelay = (int)(bg_shock_soundModEndDelay->current.value * 1000.0f);
    for (i = 0; i < SND_GetEntChannelCount(); ++i)
    {
        v9 = bg_shock_volume[i]->current.value;
        v4 = 0.0 - v9;
        if (v4 < 0.0)
            v8 = v9;
        else
            v8 = 0.0;
        v3 = v8 - 1.0;
        if (v3 < 0.0)
            v2 = v8;
        else
            v2 = 1.0;
        parms->sound.channelvolume[i] = v2;
        if (parms->sound.channelvolume[i] < 0.0 || parms->sound.channelvolume[i] > 1.0)
            MyAssertHandler(
                ".\\bgame\\bg_misc.cpp",
                1983,
                0,
                "%s\n\t(parms->sound.channelvolume[i]) = %g",
                "(parms->sound.channelvolume[i] >= 0 && parms->sound.channelvolume[i] <= 1)",
                parms->sound.channelvolume[i]);
    }
    if (parms->sound.fadeInTime <= 0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1986,
            0,
            "%s\n\t(parms->sound.fadeInTime) = %i",
            "(parms->sound.fadeInTime > 0)",
            parms->sound.fadeInTime);
    if (parms->sound.fadeOutTime <= 0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1987,
            0,
            "%s\n\t(parms->sound.fadeOutTime) = %i",
            "(parms->sound.fadeOutTime > 0)",
            parms->sound.fadeOutTime);
    if (parms->sound.loopFadeTime <= 0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1988,
            0,
            "%s\n\t(parms->sound.loopFadeTime) = %i",
            "(parms->sound.loopFadeTime > 0)",
            parms->sound.loopFadeTime);
    if (parms->sound.drylevel < 0.0 || parms->sound.drylevel > 1.0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1989,
            0,
            "%s\n\t(parms->sound.drylevel) = %g",
            "(parms->sound.drylevel >= 0 && parms->sound.drylevel <= 1)",
            parms->sound.drylevel);
    if (parms->sound.wetlevel < 0.0 || parms->sound.wetlevel > 1.0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1990,
            0,
            "%s\n\t(parms->sound.wetlevel) = %g",
            "(parms->sound.wetlevel >= 0 && parms->sound.wetlevel <= 1)",
            parms->sound.wetlevel);
    parms->lookControl.affect = bg_shock_lookControl->current.enabled;
    parms->lookControl.fadeTime = (int)(bg_shock_lookControl_fadeTime->current.value * 1000.0f);
    if (parms->lookControl.fadeTime <= 0)
        MyAssertHandler(
            ".\\bgame\\bg_misc.cpp",
            1994,
            0,
            "%s\n\t(parms->lookControl.fadeTime) = %i",
            "(parms->lookControl.fadeTime > 0)",
            parms->lookControl.fadeTime);
    parms->lookControl.maxPitchSpeed = bg_shock_lookControl_maxpitchspeed->current.value;
    parms->lookControl.maxYawSpeed = bg_shock_lookControl_maxyawspeed->current.value;
    parms->lookControl.mouseSensitivity = bg_shock_lookControl_mousesensitivityscale->current.value;
    parms->movement.affect = bg_shock_movement->current.enabled;
}

int __cdecl BG_SaveShellShockDvars(const char *name)
{
    int EntChannelCount; // eax
    const char *bg_shock_dvar_names[91]; // [esp+10h] [ebp-180h] BYREF
    const char *fullpath; // [esp+184h] [ebp-Ch]
    int fh; // [esp+188h] [ebp-8h] BYREF
    int i; // [esp+18Ch] [ebp-4h]

    for (i = 0; i < 27; ++i)
        bg_shock_dvar_names[i] = bgShockDvarNames[i];
    for (i = 0; i < SND_GetEntChannelCount(); ++i)
        bg_shock_dvar_names[i + 27] = bgShockChannelNames[i];
    EntChannelCount = SND_GetEntChannelCount();
    if (!Com_SaveDvarsToBuffer(bg_shock_dvar_names, EntChannelCount + 27, filebuf, 0x10000u))
        return 0;
    fullpath = va("shock/%s.shock", name);
    if ((FS_FOpenFileByMode((char *)fullpath, &fh, FS_WRITE) & 0x80000000) != 0)
        return 0;
    FS_Write(filebuf, strlen(filebuf), fh);
    FS_FCloseFile(fh);
    return 1;
}

shellshock_parms_t *__cdecl BG_GetShellshockParms(unsigned int index)
{
    if (index >= 0x10)
        MyAssertHandler(".\\bgame\\bg_misc.cpp", 2034, 0, "%s\n\t(index) = %i", "(index >= 0 && index < 16)", index);
    return &bg_shellshockParms[index];
}

void __cdecl BG_CreateXAnim(XAnim_s *anims, unsigned int animIndex, const char *name)
{
    if (!useFastFile->current.enabled)
        XAnimPrecache(name, (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
    XAnimCreate(anims, animIndex, name);
}

void __cdecl BG_CheckThread()
{
    iassert(Sys_IsMainThread());
}

int __cdecl BG_GetMaxSprintTime(const playerState_s *ps)
{
    double v3; // [esp+4h] [ebp-14h]
    float maxSprintTime; // [esp+14h] [ebp-4h]

    v3 = player_sprintTime->current.value * 1000.0;
    maxSprintTime = BG_GetWeaponDef(ps->weapon)->sprintDurationScale * v3;
    if ((ps->perks & 0x400) != 0)
        maxSprintTime = maxSprintTime * perk_sprintMultiplier->current.value;
    if ((int)maxSprintTime > 0x3FFF)
        return 0x3FFF;
    else
        return (int)maxSprintTime;
}

