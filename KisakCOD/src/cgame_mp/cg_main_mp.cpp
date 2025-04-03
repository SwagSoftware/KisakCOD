#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <universal/surfaceflags.h>
#include <ui_mp/ui_mp.h>
#include <qcommon/mem_track.h>
#include <DynEntity/DynEntity_client.h>
#include <ragdoll/ragdoll.h>
#include <client/client.h>
#include <universal/com_sndalias.h>
#include <universal/com_memory.h>
#include <EffectsCore/fx_system.h>
#include <gfx_d3d/r_rendercmds.h>
#include <aim_assist/aim_assist.h>
#include <script/scr_const.h>
#include <universal/com_files.h>
#include <qcommon/threads.h>
#include <stringed/stringed_hooks.h>

float cg_entityOriginArray[1][1024][3];
weaponInfo_s cg_weaponsArray[1][128];
cgMedia_t cgMedia;
centity_s cg_entitiesArray[1][1024];
cg_s cgArray[1];
cgs_t cgsArray[1];
UiContext cgDC[1];

const dvar_t * cg_hudGrenadeIconEnabledFlash;
const dvar_t *cg_hudGrenadePointerPulseMax;
const dvar_t *cg_laserLight;
const dvar_t *cg_drawVersionY;
const dvar_t *cg_drawVersion;
const dvar_t *cg_gun_y  ;
const dvar_t *cg_hudSayPosition;
const dvar_t *cg_enemyNameFadeOut;
const dvar_t *cg_fovMin ;
const dvar_t *cg_drawBreathHint;
const dvar_t *cg_enemyNameFadeIn;
const dvar_t *cg_hudGrenadeIconMaxRangeFrag;
const dvar_t *cg_drawFPS;
const dvar_t *cg_drawVersionX;
const dvar_t *cg_thirdPerson;
const dvar_t *cg_overheadNamesNearDist;
const dvar_t *cg_drawHealth;
const dvar_t *cg_brass  ;
const dvar_t *cg_weaponHintsCoD1Style;
const dvar_t *debugOverlay;
const dvar_t *cg_viewZSmoothingMax;
const dvar_t *cg_marks_ents_player_only;
const dvar_t *cg_drawCrosshair;
const dvar_t *cg_friendlyNameFadeOut;
const dvar_t *cg_packetAnalysisEntTextScale;
const dvar_t *cg_hudSplitscreenCompassScale;
const dvar_t *cg_laserRadius;
const dvar_t *cg_invalidCmdHintDuration;
const dvar_t *cg_mapLocationSelectionCursorSpeed;
const dvar_t *cg_hudSplitscreenStanceScale;
const dvar_t *cg_hudGrenadeIconWidth;
const dvar_t *cg_gun_z  ;
const dvar_t *cg_hudDamageIconOffset;
const dvar_t *overrideNVGModelWithKnife;
const dvar_t *cg_voiceIconSize;
const dvar_t *cg_viewZSmoothingMin;
const dvar_t *cg_drawTurretCrosshair;
const dvar_t *cg_hudDamageIconHeight;
const dvar_t *cg_drawSnapshot;
const dvar_t *cg_hudChatPosition;
const dvar_t *cg_weaponrightbone;
const dvar_t *cg_drawShellshock;
const dvar_t *cg_overheadNamesMaxDist;
const dvar_t *cg_gun_move_minspeed;
const dvar_t *cg_gun_ofs_u;
const dvar_t *cg_subtitles;
const dvar_t *cg_packetAnalysisTextY;
const dvar_t *cg_hudStanceHintPrints;
const dvar_t *cg_youInKillCamSize;
const dvar_t *cg_hudDamageIconWidth;
const dvar_t *cg_overheadNamesFarScale;
const dvar_t *cg_hudProneY;
const dvar_t *cg_draw2D ;
const dvar_t *cg_gun_x  ;
const dvar_t *cg_crosshairAlpha;
const dvar_t *cg_friendlyNameFadeIn;
const dvar_t *cg_laserLightRadius;
const dvar_t *cg_overheadNamesFarDist;
const dvar_t *cg_packetAnalysisClient;
const dvar_t *cg_debugPosition;
const dvar_t *cg_drawThroughWalls;
const dvar_t *cg_drawGun;
const dvar_t *cg_hudGrenadeIconOffset;
const dvar_t *cg_hudSplitscreenScoreboardScale;
const dvar_t *cg_overheadNamesSize;
const dvar_t *cg_centertime;
const dvar_t *cg_chatHeight;
const dvar_t *cg_hudGrenadePointerPulseFreq;
const dvar_t *cg_minicon;
const dvar_t *cg_drawCrosshairNamesPosX;
const dvar_t *cg_overheadNamesFont;
const dvar_t *cg_gun_move_rate;
const dvar_t *cg_tracerSpeed;
const dvar_t *cg_laserFlarePct;
const dvar_t *cg_invalidCmdHintBlinkInterval;
const dvar_t *cg_overheadRankSize;
const dvar_t *cg_hudGrenadePointerPivot;
const dvar_t *cg_crosshairAlphaMin;
const dvar_t *cg_laserLightEndOffset;
const dvar_t *cg_tracerChance;
const dvar_t *cg_hudGrenadeIconInScope;
const dvar_t *cg_hudSplitscreenBannerScoreboardScale;
const dvar_t *cg_fovScale;
const dvar_t *cg_crosshairEnemyColor;
const dvar_t *cg_drawRumbleDebug;
const dvar_t *cg_laserRangePlayer;
const dvar_t *cg_tracerScale;
const dvar_t *cg_weaponCycleDelay;
const dvar_t *cg_laserRange;
const dvar_t *cg_laserForceOn;
const dvar_t *cg_gameMessageWidth;
const dvar_t *cg_thirdPersonRange;
const dvar_t *cg_gun_ofs_r;
const dvar_t *cg_hudGrenadeIconMaxHeight;
const dvar_t *cg_debugEvents;
const dvar_t *cg_hudGrenadeIconMaxRangeFlash;
const dvar_t *cg_deadHearAllLiving;
const dvar_t *cg_hudStanceFlash;
const dvar_t *cg_overheadNamesGlow;
const dvar_t *cg_drawScriptUsage;
const dvar_t *cg_crosshairDynamic;
const dvar_t *cg_hudDamageIconInScope;
const dvar_t *cg_drawFriendlyNames;
const dvar_t *cg_drawCrosshairNames;
const dvar_t *cg_gun_move_r;
const dvar_t *cg_constantSizeHeadIcons;
const dvar_t *cg_viewZSmoothingTime;
const dvar_t *cg_footsteps;
const dvar_t *cg_gun_move_f;
const dvar_t *cg_hudGrenadeIconHeight;
const dvar_t *cg_hudGrenadePointerWidth;
const dvar_t *cg_drawWVisDebug;
const dvar_t *cg_nopredict;
const dvar_t *cg_drawMaterial;
const dvar_t *cg_deadChatWithTeam;
const dvar_t *cg_firstPersonTracerChance;
const dvar_t *cg_synchronousClients;
const dvar_t *snd_drawInfo;
const dvar_t *cg_debug_overlay_viewport;
const dvar_t *cg_packetAnalysisEntTextY;
const dvar_t *cg_connectionIconSize;
const dvar_t *cg_dumpAnims;
const dvar_t *cg_errorDecay;
const dvar_t *cg_subtitleWidthStandard;
const dvar_t *cg_hudGrenadePointerPulseMin;
const dvar_t *cg_tracerScaleMinDist;
const dvar_t *cg_laserEndOffset;
const dvar_t *cg_drawMantleHint;
const dvar_t *cg_gameBoldMessageWidth;
const dvar_t *cg_tracerWidth;
const dvar_t *cg_drawSpectatorMessages;
const dvar_t *cg_everyoneHearsEveryone;
const dvar_t *cg_laserLightBodyTweak;
const dvar_t *cg_packetAnalysisTextScale;
const dvar_t *cg_drawCrosshairNamesPosY;
const dvar_t *cg_headIconMinScreenRadius;
const dvar_t *cg_chatTime;
const dvar_t *cg_drawFPSLabels;
const dvar_t *cg_drawpaused;
const dvar_t *cg_splitscreenSpectatorScaleIncrease;
const dvar_t *cg_hudDamageIconTime;
const dvar_t *cg_debugInfoCornerOffset;
const dvar_t *cg_blood  ;
const dvar_t *cg_teamChatsOnly;
const dvar_t *cg_weaponleftbone;
const dvar_t *cg_scriptIconSize;
const dvar_t *cg_showmiss;
const dvar_t *cg_predictItems;
const dvar_t *cg_deadChatWithDead;
const dvar_t *cg_descriptiveText;
const dvar_t *cg_fov    ;
const dvar_t *cg_hudSplitscreenCompassElementScale;
const dvar_t *cg_subtitleWidthWidescreen;
const dvar_t *cg_drawLagometer;
const dvar_t *cg_developer;
const dvar_t *cg_tracerScrewRadius;
const dvar_t *cg_hudGrenadePointerHeight;
const dvar_t *cg_gun_move_u;
const dvar_t *cg_hintFadeTime;
const dvar_t *cg_subtitleMinTime;
const dvar_t *cg_tracerScaleDistRange;
const dvar_t *cg_deadHearTeamLiving;
const dvar_t *cg_paused ;
const dvar_t *cg_cursorHints;
const dvar_t *cg_thirdPersonAngle;
const dvar_t *cg_gun_ofs_f;
const dvar_t *cg_laserLightBeginOffset;
const dvar_t *cg_overheadIconSize;
const dvar_t *cg_tracerScrewDist;
const dvar_t *cg_tracerLength;
const dvar_t *cg_hudChatIntermissionPosition;
const dvar_t *cg_hudVotePosition;
const dvar_t *cg_fs_debug;
const dvar_t *g_compassShowEnemies;

const char *debugOverlayNames[4] = { "Off", "ViewmodelInfo", "FontTest", NULL }; // idb

const char *cg_drawFpsNames[5] =
{
    "Off",
    "Simple",
    "SimpleRanges",
    "Verbose",
    NULL
};

const char *snd_drawInfoStrings[5] =
{
    "None",
    "3D",
    "Stream",
    "2D",
    NULL
};

const char *cg_drawMaterialNames[5] =
{
    "Off",
    "CONTENTS_SOLID",
    "MASK_SHOT",
    "MASK_PLAYERSOLID",
    NULL
};

bool __cdecl CG_IsRagdollTrajectory(const trajectory_t *trajectory)
{
    if (!trajectory)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 367, 0, "%s", "trajectory");
    if (!ragdoll_enable)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 368, 0, "%s", "ragdoll_enable");
    return ragdoll_enable->current.enabled
        && trajectory->trType >= TR_FIRST_RAGDOLL
        && trajectory->trType <= TR_RAGDOLL_INTERPOLATE;
}

void __cdecl CG_RegisterDvars()
{
    DvarLimits min; // [esp+8h] [ebp-10h]
    DvarLimits mina; // [esp+8h] [ebp-10h]
    DvarLimits minb; // [esp+8h] [ebp-10h]
    DvarLimits minc; // [esp+8h] [ebp-10h]
    DvarLimits mind; // [esp+8h] [ebp-10h]
    DvarLimits mine; // [esp+8h] [ebp-10h]
    DvarLimits minf; // [esp+8h] [ebp-10h]
    DvarLimits ming; // [esp+8h] [ebp-10h]
    DvarLimits minh; // [esp+8h] [ebp-10h]
    DvarLimits mini; // [esp+8h] [ebp-10h]
    DvarLimits minj; // [esp+8h] [ebp-10h]
    DvarLimits mink; // [esp+8h] [ebp-10h]
    DvarLimits minl; // [esp+8h] [ebp-10h]
    DvarLimits minm; // [esp+8h] [ebp-10h]
    DvarLimits minn; // [esp+8h] [ebp-10h]
    DvarLimits mino; // [esp+8h] [ebp-10h]
    DvarLimits minp; // [esp+8h] [ebp-10h]
    DvarLimits minq; // [esp+8h] [ebp-10h]
    DvarLimits minr; // [esp+8h] [ebp-10h]
    DvarLimits mins; // [esp+8h] [ebp-10h]
    DvarLimits mint; // [esp+8h] [ebp-10h]
    DvarLimits minu; // [esp+8h] [ebp-10h]
    DvarLimits minv; // [esp+8h] [ebp-10h]
    DvarLimits minw; // [esp+8h] [ebp-10h]
    DvarLimits minx; // [esp+8h] [ebp-10h]
    DvarLimits miny; // [esp+8h] [ebp-10h]
    DvarLimits minz; // [esp+8h] [ebp-10h]
    DvarLimits minba; // [esp+8h] [ebp-10h]
    DvarLimits minbb; // [esp+8h] [ebp-10h]
    DvarLimits minbc; // [esp+8h] [ebp-10h]
    DvarLimits minbd; // [esp+8h] [ebp-10h]
    DvarLimits minbe; // [esp+8h] [ebp-10h]
    DvarLimits minbf; // [esp+8h] [ebp-10h]
    DvarLimits minbg; // [esp+8h] [ebp-10h]
    DvarLimits minbh; // [esp+8h] [ebp-10h]
    DvarLimits minbi; // [esp+8h] [ebp-10h]
    DvarLimits minbj; // [esp+8h] [ebp-10h]
    DvarLimits minbk; // [esp+8h] [ebp-10h]
    DvarLimits minbl; // [esp+8h] [ebp-10h]
    DvarLimits minbm; // [esp+8h] [ebp-10h]
    DvarLimits minbn; // [esp+8h] [ebp-10h]
    DvarLimits minbo; // [esp+8h] [ebp-10h]
    DvarLimits minbp; // [esp+8h] [ebp-10h]
    DvarLimits minbq; // [esp+8h] [ebp-10h]
    DvarLimits minbr; // [esp+8h] [ebp-10h]
    DvarLimits minbs; // [esp+8h] [ebp-10h]
    DvarLimits minbt; // [esp+8h] [ebp-10h]
    DvarLimits minbu; // [esp+8h] [ebp-10h]
    DvarLimits minbv; // [esp+8h] [ebp-10h]
    DvarLimits minbw; // [esp+8h] [ebp-10h]
    DvarLimits minbx; // [esp+8h] [ebp-10h]
    DvarLimits minby; // [esp+8h] [ebp-10h]
    DvarLimits minbz; // [esp+8h] [ebp-10h]
    DvarLimits minca; // [esp+8h] [ebp-10h]
    DvarLimits mincb; // [esp+8h] [ebp-10h]
    DvarLimits mincc; // [esp+8h] [ebp-10h]
    DvarLimits mincd; // [esp+8h] [ebp-10h]
    DvarLimits mince; // [esp+8h] [ebp-10h]
    DvarLimits mincf; // [esp+8h] [ebp-10h]
    DvarLimits mincg; // [esp+8h] [ebp-10h]
    DvarLimits minch; // [esp+8h] [ebp-10h]
    DvarLimits minci; // [esp+8h] [ebp-10h]
    DvarLimits mincj; // [esp+8h] [ebp-10h]
    DvarLimits minck; // [esp+8h] [ebp-10h]
    DvarLimits mincl; // [esp+8h] [ebp-10h]
    DvarLimits mincm; // [esp+8h] [ebp-10h]
    DvarLimits mincn; // [esp+8h] [ebp-10h]
    DvarLimits minco; // [esp+8h] [ebp-10h]
    DvarLimits mincp; // [esp+8h] [ebp-10h]
    DvarLimits mincq; // [esp+8h] [ebp-10h]
    DvarLimits mincr; // [esp+8h] [ebp-10h]
    DvarLimits mincs; // [esp+8h] [ebp-10h]
    DvarLimits minct; // [esp+8h] [ebp-10h]
    DvarLimits mincu; // [esp+8h] [ebp-10h]
    DvarLimits mincv; // [esp+8h] [ebp-10h]
    DvarLimits mincw; // [esp+8h] [ebp-10h]
    DvarLimits mincx; // [esp+8h] [ebp-10h]
    DvarLimits mincy; // [esp+8h] [ebp-10h]
    DvarLimits mincz; // [esp+8h] [ebp-10h]
    DvarLimits minda; // [esp+8h] [ebp-10h]
    DvarLimits mindb; // [esp+8h] [ebp-10h]

    cg_drawGun = Dvar_RegisterBool("cg_drawGun", 1, 0x80u, "Draw the view model");
    cg_cursorHints = Dvar_RegisterInt(
        "cg_cursorHints",
        4,
        (DvarLimits)0x400000000LL,
        1u,
        "Draw cursor hints where:\n"
        " 0: no hints\n"
        "\t1:\tsin size pulse\n"
        "\t2:\tone way size pulse\n"
        "\t3:\talpha pulse\n"
        "\t4:\tstatic image");
    cg_weaponHintsCoD1Style = Dvar_RegisterBool(
        "cg_weaponHintsCoD1Style",
        1,
        0x1000u,
        "Draw weapon hints in CoD1 style: with the weapon name, and with the icon below");
    cg_hintFadeTime = Dvar_RegisterInt(
        "cg_hintFadeTime",
        100,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Time in milliseconds for the cursor hint to fade");
    min.value.max = 80.0;
    min.value.min = 65.0;
    cg_fov = Dvar_RegisterFloat("cg_fov", 65.0, min, 1u, "The field of view angle in degrees");
    mina.value.max = 2.0;
    mina.value.min = 0.2;
    cg_fovScale = Dvar_RegisterFloat("cg_fovScale", 1.0, mina, 0x80u, "Scale applied to the field of view");
    minb.value.max = 160.0;
    minb.value.min = 1.0;
    cg_fovMin = Dvar_RegisterFloat("cg_fovMin", 10.0, minb, 0x80u, "The minimum possible field of view");
    cg_draw2D = Dvar_RegisterBool("cg_draw2D", 1, 0x80u, "Draw 2D screen elements");
    cg_drawHealth = Dvar_RegisterBool("cg_drawHealth", 0, 0x80u, "Draw health bar");
    cg_drawBreathHint = Dvar_RegisterBool("cg_drawBreathHint", 1, 1u, "Draw a 'hold breath to steady' hint");
    cg_drawMantleHint = Dvar_RegisterBool("cg_drawMantleHint", 1, 1u, "Draw a 'press key to mantle' hint");
    cg_drawFPS = Dvar_RegisterEnum("cg_drawFPS", cg_drawFpsNames, 1, 1u, "Draw frames per second");
    cg_drawFPSLabels = Dvar_RegisterBool("cg_drawFPSLabels", 1, 1u, "Draw FPS Info Labels");
    minc.value.max = 640.0;
    minc.value.min = -200.0;
    cg_debugInfoCornerOffset = Dvar_RegisterVec2(
        "cg_debugInfoCornerOffset",
        0.0,
        0.0,
        minc,
        1u,
        "Offset from top-right corner, for cg_drawFPS, etc");
    cg_drawVersion = Dvar_RegisterBool("cg_drawVersion", 1, 0, "Draw the game version");
    mind.value.max = 512.0;
    mind.value.min = 0.0;
    cg_drawVersionX = Dvar_RegisterFloat("cg_drawVersionX", 50.0, mind, 0, "X offset for the version string");
    mine.value.max = 512.0;
    mine.value.min = 0.0;
    cg_drawVersionY = Dvar_RegisterFloat("cg_drawVersionY", 18.0, mine, 0, "Y offset for the version string");
    snd_drawInfo = Dvar_RegisterEnum("snd_drawInfo", snd_drawInfoStrings, 0, 0, "Draw debugging information for sounds");
    cg_drawScriptUsage = Dvar_RegisterBool("cg_drawScriptUsage", 0, 0, "Draw debugging information for scripts");
    cg_drawMaterial = Dvar_RegisterEnum(
        "cg_drawMaterial",
        cg_drawMaterialNames,
        0,
        0x80u,
        "Draw debugging information for materials");
    cg_drawSnapshot = Dvar_RegisterBool("cg_drawSnapshot", 0, 1u, "Draw debugging information for snapshots");
    cg_drawCrosshair = Dvar_RegisterBool("cg_drawCrosshair", 1, 0x81u, "Turn on weapon crosshair");
    cg_drawTurretCrosshair = Dvar_RegisterBool("cg_drawTurretCrosshair", 1, 1u, "Draw a cross hair when using a turret");
    cg_drawCrosshairNames = Dvar_RegisterBool(
        "cg_drawCrosshairNames",
        1,
        0x81u,
        "Draw the name of an enemy under the crosshair");
    cg_drawCrosshairNamesPosX = Dvar_RegisterInt(
        "cg_drawCrosshairNamesPosX",
        300,
        (DvarLimits)0x28000000000LL,
        0,
        "Virtual screen space position of the crosshair name");
    cg_drawCrosshairNamesPosY = Dvar_RegisterInt(
        "cg_drawCrosshairNamesPosY",
        180,
        (DvarLimits)0x1E000000000LL,
        0,
        "Virtual screen space position of the crosshair name");
    cg_drawShellshock = Dvar_RegisterBool("cg_drawShellshock", 1, 0x80u, "Draw shellshock & flashbang screen effects.");
    cg_drawSpectatorMessages = Dvar_RegisterBool(
        "cg_drawSpectatorMessages",
        1,
        0,
        "Enables drawing of spectator HUD messages.");
    cg_hudStanceFlash = Dvar_RegisterColor(
        "cg_hudStanceFlash",
        1.0,
        1.0,
        1.0,
        1.0,
        0,
        "The background color of the flash when the stance changes");
    cg_hudStanceHintPrints = Dvar_RegisterBool(
        "cg_hudStanceHintPrints",
        0,
        1u,
        "Draw helpful text to say how to change stances");
    minf.value.max = 512.0;
    minf.value.min = 0.0;
    cg_hudDamageIconWidth = Dvar_RegisterFloat("cg_hudDamageIconWidth", 128.0, minf, 1u, "The width of the damage icon");
    ming.value.max = 512.0;
    ming.value.min = 0.0;
    cg_hudDamageIconHeight = Dvar_RegisterFloat("cg_hudDamageIconHeight", 64.0, ming, 1u, "The height of the damage icon");
    minh.value.max = 512.0;
    minh.value.min = 0.0;
    cg_hudDamageIconOffset = Dvar_RegisterFloat(
        "cg_hudDamageIconOffset",
        128.0,
        minh,
        1u,
        "The offset from the center of the damage icon");
    cg_hudDamageIconTime = Dvar_RegisterInt(
        "cg_hudDamageIconTime",
        2000,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "The amount of time for the damage icon to stay on screen after damage is taken");
    cg_hudDamageIconInScope = Dvar_RegisterBool(
        "cg_hudDamageIconInScope",
        0,
        1u,
        "Draw damage icons when aiming down the sight of a scoped weapon");
    mini.value.max = 1000.0;
    mini.value.min = 0.0;
    cg_hudGrenadeIconMaxRangeFrag = Dvar_RegisterFloat(
        "cg_hudGrenadeIconMaxRangeFrag",
        250.0,
        mini,
        0x1080u,
        "The minimum distance that a grenade has to be from a player in order to be shown on "
        "the grenade indicator");
    minj.value.max = 2000.0;
    minj.value.min = 0.0;
    cg_hudGrenadeIconMaxRangeFlash = Dvar_RegisterFloat(
        "cg_hudGrenadeIconMaxRangeFlash",
        500.0,
        minj,
        0x1080u,
        "The minimum distance that a flashbang has to be from a player in order to be shown "
        "on the grenade indicator");
    mink.value.max = 1000.0;
    mink.value.min = 0.0;
    cg_hudGrenadeIconMaxHeight = Dvar_RegisterFloat(
        "cg_hudGrenadeIconMaxHeight",
        104.0,
        mink,
        1u,
        "The minimum height difference between a player and a grenade for the grenade to be show"
        "n on the grenade indicator");
    cg_hudGrenadeIconInScope = Dvar_RegisterBool(
        "cg_hudGrenadeIconInScope",
        0,
        1u,
        "Show the grenade indicator when aiming down the sight of a scoped weapon");
    minl.value.max = 512.0;
    minl.value.min = 0.0;
    cg_hudGrenadeIconOffset = Dvar_RegisterFloat(
        "cg_hudGrenadeIconOffset",
        50.0,
        minl,
        1u,
        "The offset from the center of the screen for a grenade icon");
    minm.value.max = 512.0;
    minm.value.min = 0.0;
    cg_hudGrenadeIconHeight = Dvar_RegisterFloat(
        "cg_hudGrenadeIconHeight",
        25.0,
        minm,
        1u,
        "The height of the grenade indicator icon");
    minn.value.max = 512.0;
    minn.value.min = 0.0;
    cg_hudGrenadeIconWidth = Dvar_RegisterFloat(
        "cg_hudGrenadeIconWidth",
        25.0,
        minn,
        1u,
        "The width of the grenade indicator icon");
    cg_hudGrenadeIconEnabledFlash = Dvar_RegisterBool(
        "cg_hudGrenadeIconEnabledFlash",
        0,
        1u,
        "Show the grenade indicator for flash grenades");
    mino.value.max = 512.0;
    mino.value.min = 0.0;
    cg_hudGrenadePointerHeight = Dvar_RegisterFloat(
        "cg_hudGrenadePointerHeight",
        12.0,
        mino,
        1u,
        "The height of the grenade indicator pointer");
    minp.value.max = 512.0;
    minp.value.min = 0.0;
    cg_hudGrenadePointerWidth = Dvar_RegisterFloat(
        "cg_hudGrenadePointerWidth",
        25.0,
        minp,
        1u,
        "The width of the grenade indicator pointer");
    minq.value.max = 512.0;
    minq.value.min = 0.0;
    cg_hudGrenadePointerPivot = Dvar_RegisterVec2(
        "cg_hudGrenadePointerPivot",
        12.0,
        27.0,
        minq,
        1u,
        "The pivot point of th grenade indicator pointer");
    minr.value.max = 50.0;
    minr.value.min = 0.1;
    cg_hudGrenadePointerPulseFreq = Dvar_RegisterFloat(
        "cg_hudGrenadePointerPulseFreq",
        1.7,
        minr,
        0,
        "The number of times per second that the grenade indicator flashes in Hertz");
    mins.value.max = 3.0;
    mins.value.min = 0.0;
    cg_hudGrenadePointerPulseMax = Dvar_RegisterFloat(
        "cg_hudGrenadePointerPulseMax",
        1.85,
        mins,
        0,
        "The maximum alpha of the grenade indicator pulse. Values higher than 1 will cause the"
        " indicator to remain at full brightness for longer");
    mint.value.max = 1.0;
    mint.value.min = -3.0;
    cg_hudGrenadePointerPulseMin = Dvar_RegisterFloat(
        "cg_hudGrenadePointerPulseMin",
        0.30000001,
        mint,
        0,
        "The minimum alpha of the grenade indicator pulse. Values lower than 0 will cause the "
        "indicator to remain at full transparency for longer");
    minu.value.max = 640.0;
    minu.value.min = 0.0;
    cg_hudChatPosition = Dvar_RegisterVec2("cg_hudChatPosition", 5.0, 204.0, minu, 1u, "Position of the HUD chat box");
    minv.value.max = 640.0;
    minv.value.min = 0.0;
    cg_hudChatIntermissionPosition = Dvar_RegisterVec2(
        "cg_hudChatIntermissionPosition",
        5.0,
        110.0,
        minv,
        1u,
        "Position of the HUD chat box during intermission");
    minw.value.max = 640.0;
    minw.value.min = 0.0;
    cg_hudSayPosition = Dvar_RegisterVec2("cg_hudSayPosition", 5.0, 180.0, minw, 1u, "Position of the HUD say box");
    minx.value.max = 640.0;
    minx.value.min = 0.0;
    cg_hudVotePosition = Dvar_RegisterVec2("cg_hudVotePosition", 5.0, 220.0, minx, 1u, "Position of the HUD vote box");
    cg_drawLagometer = Dvar_RegisterBool("cg_drawLagometer", 0, 1u, "Enable the 'lagometer'");
    miny.value.max = 10000.0;
    miny.value.min = -10000.0;
    cg_hudProneY = Dvar_RegisterFloat(
        "cg_hudProneY",
        -160.0,
        miny,
        1u,
        "Virtual screen y coordinate of the prone blocked message");
    minz.value.max = 1.0;
    minz.value.min = 0.001;
    cg_mapLocationSelectionCursorSpeed = Dvar_RegisterFloat(
        "cg_mapLocationSelectionCursorSpeed",
        0.60000002,
        minz,
        1u,
        "Speed of the cursor when selecting a location on the map");
    cg_packetAnalysisClient = Dvar_RegisterInt(
        "cg_packetAnalysisClient",
        0,
        (DvarLimits)0x40FFFFFFFFLL,
        0,
        "The client num to get the packet analysis done");
    minba.value.max = 1.0;
    minba.value.min = 0.0;
    cg_packetAnalysisTextScale = Dvar_RegisterFloat(
        "cg_packetAnalysisTextScale",
        0.2,
        minba,
        0,
        "The text scale of the packet analysis debug prints");
    minbb.value.max = 1.0;
    minbb.value.min = 0.0;
    cg_packetAnalysisEntTextScale = Dvar_RegisterFloat(
        "cg_packetAnalysisEntTextScale",
        0.23,
        minbb,
        0,
        "The text scale of the packet analysis entity debug prints");
    cg_packetAnalysisTextY = Dvar_RegisterInt(
        "cg_packetAnalysisTextY",
        15,
        (DvarLimits)0x400FFFFFFF6LL,
        0,
        "The y coordinate of the packet analysis debug prints");
    cg_packetAnalysisEntTextY = Dvar_RegisterInt(
        "cg_packetAnalysisEntTextY",
        -5,
        (DvarLimits)0x400FFFFFFF6LL,
        0,
        "The y coordinate of the packet analysis entity debug prints");
    cg_weaponCycleDelay = Dvar_RegisterInt(
        "cg_weaponCycleDelay",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "The delay after cycling to a new weapon to prevent holding down the cycle weapon button from cycling too fast");
    minbc.value.max = 1.0;
    minbc.value.min = 0.0;
    cg_crosshairAlpha = Dvar_RegisterFloat("cg_crosshairAlpha", 1.0, minbc, 0x81u, "The alpha value of the crosshair");
    minbd.value.max = 1.0;
    minbd.value.min = 0.0;
    cg_crosshairAlphaMin = Dvar_RegisterFloat(
        "cg_crosshairAlphaMin",
        0.5,
        minbd,
        0x81u,
        "The minimum alpha value of the crosshair when it fades in");
    cg_crosshairDynamic = Dvar_RegisterBool("cg_crosshairDynamic", 0, 0x81u, "Crosshair is Dynamic");
    cg_crosshairEnemyColor = Dvar_RegisterBool(
        "cg_crosshairEnemyColor",
        1,
        0x81u,
        "The crosshair color when over an enemy");
    cg_brass = Dvar_RegisterBool("cg_brass", 1, 1u, "Weapons eject brass");
    minbe.value.max = 3.4028235e38;
    minbe.value.min = -3.4028235e38;
    cg_gun_x = Dvar_RegisterFloat("cg_gun_x", 0.0, minbe, 0x80u, "x position of the viewmodel");
    minbf.value.max = 3.4028235e38;
    minbf.value.min = -3.4028235e38;
    cg_gun_y = Dvar_RegisterFloat("cg_gun_y", 0.0, minbf, 0x80u, "y position of the viewmodel");
    minbg.value.max = 3.4028235e38;
    minbg.value.min = -3.4028235e38;
    cg_gun_z = Dvar_RegisterFloat("cg_gun_z", 0.0, minbg, 0x80u, "z position of the viewmodel");
    minbh.value.max = 3.4028235e38;
    minbh.value.min = -3.4028235e38;
    cg_gun_move_f = Dvar_RegisterFloat(
        "cg_gun_move_f",
        0.0,
        minbh,
        0x80u,
        "Weapon movement forward due to player movement");
    minbi.value.max = 3.4028235e38;
    minbi.value.min = -3.4028235e38;
    cg_gun_move_r = Dvar_RegisterFloat("cg_gun_move_r", 0.0, minbi, 0x80u, "Weapon movement right due to player movement");
    minbj.value.max = 3.4028235e38;
    minbj.value.min = -3.4028235e38;
    cg_gun_move_u = Dvar_RegisterFloat("cg_gun_move_u", 0.0, minbj, 0x80u, "Weapon movement up due to player movement");
    minbk.value.max = 3.4028235e38;
    minbk.value.min = -3.4028235e38;
    cg_gun_ofs_f = Dvar_RegisterFloat("cg_gun_ofs_f", 0.0, minbk, 0x80u, "Forward weapon offset when prone/ducked");
    minbl.value.max = 3.4028235e38;
    minbl.value.min = -3.4028235e38;
    cg_gun_ofs_r = Dvar_RegisterFloat("cg_gun_ofs_r", 0.0, minbl, 0x80u, "Right weapon offset when prone/ducked");
    minbm.value.max = 3.4028235e38;
    minbm.value.min = -3.4028235e38;
    cg_gun_ofs_u = Dvar_RegisterFloat("cg_gun_ofs_u", 0.0, minbm, 0x80u, "Up weapon offset when prone/ducked");
    minbn.value.max = 3.4028235e38;
    minbn.value.min = -3.4028235e38;
    cg_gun_move_rate = Dvar_RegisterFloat("cg_gun_move_rate", 0.0, minbn, 0x80u, "The base weapon movement rate");
    minbo.value.max = 3.4028235e38;
    minbo.value.min = -3.4028235e38;
    cg_gun_move_minspeed = Dvar_RegisterFloat(
        "cg_gun_move_minspeed",
        0.0,
        minbo,
        0x80u,
        "The minimum weapon movement rate");
    minbp.value.max = 3.4028235e38;
    minbp.value.min = 0.0;
    cg_centertime = Dvar_RegisterFloat(
        "cg_centertime",
        5.0,
        minbp,
        0x80u,
        "The time for a center printed message to fade");
    cg_debugPosition = Dvar_RegisterBool("cg_debugposition", 0, 0x80u, "Output position debugging information");
    cg_debugEvents = Dvar_RegisterBool("cg_debugevents", 0, 0x80u, "Output event debug information");
    minbq.value.max = 3.4028235e38;
    minbq.value.min = 0.0;
    cg_errorDecay = Dvar_RegisterFloat("cg_errordecay", 100.0, minbq, 0, "Decay for predicted error");
    cg_nopredict = Dvar_RegisterBool("cg_nopredict", 0, 0, "Don't do client side prediction");
    cg_showmiss = Dvar_RegisterInt("cg_showmiss", 0, (DvarLimits)0x200000000LL, 0, "Show prediction errors");
    cg_footsteps = Dvar_RegisterBool("cg_footsteps", 1, 0x80u, "Play footstep sounds");
    minbr.value.max = 1.0;
    minbr.value.min = 0.0;
    cg_firstPersonTracerChance = Dvar_RegisterFloat(
        "cg_firstPersonTracerChance",
        0.5,
        minbr,
        0x80u,
        "The probability that a bullet is a tracer round for your bullets");
    cg_laserForceOn = Dvar_RegisterBool(
        "cg_laserForceOn",
        0,
        0x80u,
        "Force laser sights on in all possible places (for debug purposes).");
    minbs.value.max = 3.4028235e38;
    minbs.value.min = 1.0;
    cg_laserRange = Dvar_RegisterFloat("cg_laserRange", 1500.0, minbs, 0x80u, "The maximum range of a laser beam");
    minbt.value.max = 3.4028235e38;
    minbt.value.min = 1.0;
    cg_laserRangePlayer = Dvar_RegisterFloat(
        "cg_laserRangePlayer",
        1500.0,
        minbt,
        0x80u,
        "The maximum range of the player's laser beam");
    minbu.value.max = 3.4028235e38;
    minbu.value.min = 0.001;
    cg_laserRadius = Dvar_RegisterFloat("cg_laserRadius", 0.80000001, minbu, 0x80u, "The size (radius) of a laser beam");
    cg_laserLight = Dvar_RegisterBool(
        "cg_laserLight",
        1,
        1u,
        "Whether to draw the light emitted from a laser (not the laser itself)");
    minbv.value.max = 3.4028235e38;
    minbv.value.min = -3.4028235e38;
    cg_laserLightBodyTweak = Dvar_RegisterFloat(
        "cg_laserLightBodyTweak",
        15.0,
        minbv,
        0x80u,
        "Amount to add to length of beam for light when laser hits a body (for hitboxes).");
    minbw.value.max = 3.4028235e38;
    minbw.value.min = 0.001;
    cg_laserLightRadius = Dvar_RegisterFloat(
        "cg_laserLightRadius",
        3.0,
        minbw,
        0x80u,
        "The radius of the light at the far end of a laser beam");
    minbx.value.max = 3.4028235e38;
    minbx.value.min = -3.4028235e38;
    cg_laserLightBeginOffset = Dvar_RegisterFloat(
        "cg_laserLightBeginOffset",
        13.0,
        minbx,
        0x80u,
        "How far from the true beginning of the beam the light at the beginning is.");
    minby.value.max = 3.4028235e38;
    minby.value.min = -3.4028235e38;
    cg_laserLightEndOffset = Dvar_RegisterFloat(
        "cg_laserLightEndOffset",
        -3.0,
        minby,
        0x80u,
        "How far from the true end of the beam the light at the end is.");
    minbz.value.max = 3.4028235e38;
    minbz.value.min = -3.4028235e38;
    cg_laserEndOffset = Dvar_RegisterFloat(
        "cg_laserEndOffset",
        0.5,
        minbz,
        0x80u,
        "How far from the point of collision the end of the beam is.");
    minca.value.max = 3.4028235e38;
    minca.value.min = 0.0;
    cg_laserFlarePct = Dvar_RegisterFloat(
        "cg_laserFlarePct",
        0.2,
        minca,
        0x80u,
        "Percentage laser widens over distance from viewer.");
    cg_marks_ents_player_only = Dvar_RegisterBool(
        "cg_marks_ents_player_only",
        0,
        1u,
        "Marks on entities from players' bullets only.");
    mincb.value.max = 1.0;
    mincb.value.min = 0.0;
    cg_tracerChance = Dvar_RegisterFloat(
        "cg_tracerchance",
        0.2,
        mincb,
        0x80u,
        "The probability that a bullet is a tracer round");
    mincc.value.max = 3.4028235e38;
    mincc.value.min = 0.0;
    cg_tracerWidth = Dvar_RegisterFloat("cg_tracerwidth", 4.0, mincc, 0x80u, "The width of the tracer round");
    mincd.value.max = 3.4028235e38;
    mincd.value.min = 0.0;
    cg_tracerSpeed = Dvar_RegisterFloat(
        "cg_tracerSpeed",
        7500.0,
        mincd,
        0x80u,
        "The speed of a tracer round in units per second");
    mince.value.max = 3.4028235e38;
    mince.value.min = 0.0;
    cg_tracerLength = Dvar_RegisterFloat("cg_tracerlength", 160.0, mince, 0x80u, "The length of a tracer round");
    mincf.value.max = 3.4028235e38;
    mincf.value.min = 1.0;
    cg_tracerScale = Dvar_RegisterFloat(
        "cg_tracerScale",
        1.0,
        mincf,
        0x80u,
        "Scale the tracer at a distance, so it's still visible");
    mincg.value.max = 3.4028235e38;
    mincg.value.min = 0.0;
    cg_tracerScaleMinDist = Dvar_RegisterFloat(
        "cg_tracerScaleMinDist",
        5000.0,
        mincg,
        0x80u,
        "The minimum distance to scale a tracer");
    minch.value.max = 3.4028235e38;
    minch.value.min = 0.0;
    cg_tracerScaleDistRange = Dvar_RegisterFloat(
        "cg_tracerScaleDistRange",
        25000.0,
        minch,
        0x80u,
        "The range at which a tracer is scaled to its maximum amount");
    minci.value.max = 3.4028235e38;
    minci.value.min = 0.0;
    cg_tracerScrewDist = Dvar_RegisterFloat(
        "cg_tracerScrewDist",
        100.0,
        minci,
        0x80u,
        "The length a tracer goes as it completes a full corkscrew revolution");
    mincj.value.max = 3.4028235e38;
    mincj.value.min = 0.0;
    cg_tracerScrewRadius = Dvar_RegisterFloat(
        "cg_tracerScrewRadius",
        0.5,
        mincj,
        0x80u,
        "The radius of a tracer's corkscrew motion");
    minck.value.max = 1024.0;
    minck.value.min = 0.0;
    cg_thirdPersonRange = Dvar_RegisterFloat(
        "cg_thirdPersonRange",
        120.0,
        minck,
        0x80u,
        "The range of the camera from the player in third person view");
    mincl.value.max = 360.0;
    mincl.value.min = -180.0;
    cg_thirdPersonAngle = Dvar_RegisterFloat(
        "cg_thirdPersonAngle",
        0.0,
        mincl,
        0x80u,
        "The angle of the camera from the player in third person view");
    cg_thirdPerson = Dvar_RegisterBool("cg_thirdPerson", 0, 0x80u, "Use third person view");
    cg_chatTime = Dvar_RegisterInt(
        "cg_chatTime",
        12000,
        (DvarLimits)0xEA6000000000LL,
        1u,
        "The amount of time that a chat message is visible");
    cg_chatHeight = Dvar_RegisterInt(
        "cg_chatHeight",
        8,
        (DvarLimits)0x800000000LL,
        1u,
        "The font height of a chat message");
    cg_predictItems = Dvar_RegisterBool("cg_predictItems", 1, 3u, "Turn on client side prediction for item pickup");
    cg_teamChatsOnly = Dvar_RegisterBool("cg_teamChatsOnly", 0, 1u, "Allow chatting only on the same team");
    cg_paused = Dvar_RegisterInt("cl_paused", 0, (DvarLimits)0x200000000LL, 0x40u, "Pause the game");
    cg_drawpaused = Dvar_RegisterBool("cg_drawpaused", 1, 0, "Draw paused screen");
    cg_synchronousClients = Dvar_RegisterBool(
        "g_synchronousClients",
        0,
        8u,
        "Client is synchronized to the server - allows smooth demos");
    cg_debug_overlay_viewport = Dvar_RegisterBool(
        "cg_debug_overlay_viewport",
        0,
        0x80u,
        "Remove the sniper overlay so you can check that the scissor window is correct.");
    cg_fs_debug = Dvar_RegisterInt(
        "fs_debug",
        0,
        (DvarLimits)0x200000000LL,
        0,
        "Output debugging information for the file system");
    cg_dumpAnims = Dvar_RegisterInt(
        "cg_dumpAnims",
        -1,
        (DvarLimits)0x3FFFFFFFFFFLL,
        0x80u,
        "Output animation info for the given entity id");
    cg_developer = Dvar_RegisterInt("developer", 0, (DvarLimits)0x200000000LL, 0, "Enable development options");
    cg_minicon = Dvar_RegisterBool("con_minicon", 0, 1u, "Display the mini console on screen");
    cg_subtitles = Dvar_RegisterBool("cg_subtitles", 1, 1u, "Show subtitles");
    mincm.value.max = 3.4028235e38;
    mincm.value.min = 0.0;
    cg_subtitleMinTime = Dvar_RegisterFloat(
        "cg_subtitleMinTime",
        3.0,
        mincm,
        1u,
        "The minimum time that the subtitles are displayed on screen in seconds");
    cg_subtitleWidthStandard = Dvar_RegisterInt(
        "cg_subtitleWidthStandard",
        520,
        (DvarLimits)0x68000000082LL,
        1u,
        "The width of the subtitles in non wide-screen");
    cg_subtitleWidthWidescreen = Dvar_RegisterInt(
        "cg_subtitleWidthWidescreen",
        520,
        (DvarLimits)0x68000000082LL,
        1u,
        "The width of the subtitles in wide-screen ");
    cg_gameMessageWidth = Dvar_RegisterInt(
        "cg_gameMessageWidth",
        455,
        (DvarLimits)0x68000000082LL,
        1u,
        "The maximum character width of the game messages");
    cg_gameBoldMessageWidth = Dvar_RegisterInt(
        "cg_gameBoldMessageWidth",
        390,
        (DvarLimits)0x68000000082LL,
        1u,
        "The maximum character width of the bold game messages");
    cg_descriptiveText = Dvar_RegisterBool("cg_descriptiveText", 1, 1u, "Draw descriptive spectator messages");
    mincn.value.max = 100.0;
    mincn.value.min = 0.0;
    cg_youInKillCamSize = Dvar_RegisterFloat(
        "cg_youInKillCamSize",
        6.0,
        mincn,
        1u,
        "Size of the 'you' Icon in the kill cam");
    minco.value.max = 100.0;
    minco.value.min = 0.0;
    cg_scriptIconSize = Dvar_RegisterFloat("cg_scriptIconSize", 0.0, minco, 1u, "Size of Icons defined by script");
    mincp.value.max = 100.0;
    mincp.value.min = 0.0;
    cg_connectionIconSize = Dvar_RegisterFloat("cg_connectionIconSize", 0.0, mincp, 1u, "Size of the connection icon");
    mincq.value.max = 100.0;
    mincq.value.min = 0.0;
    cg_voiceIconSize = Dvar_RegisterFloat("cg_voiceIconSize", 0.0, mincq, 1u, "Size of the 'voice' icon");
    cg_constantSizeHeadIcons = Dvar_RegisterBool(
        "cg_constantSizeHeadIcons",
        0,
        0x80u,
        "Head icons are the same size regardless of distance from the player");
    mincr.value.max = 1.0;
    mincr.value.min = 0.0;
    cg_headIconMinScreenRadius = Dvar_RegisterFloat(
        "cg_headIconMinScreenRadius",
        0.02,
        mincr,
        1u,
        "The minumum radius of a head icon on the screen");
    mincs.value.max = 3.4028235e38;
    mincs.value.min = 0.0;
    cg_overheadNamesMaxDist = Dvar_RegisterFloat(
        "cg_overheadNamesMaxDist",
        10000.0,
        mincs,
        0x80u,
        "The maximum distance for showing friendly player names");
    minct.value.max = 3.4028235e38;
    minct.value.min = 0.0;
    cg_overheadNamesNearDist = Dvar_RegisterFloat(
        "cg_overheadNamesNearDist",
        256.0,
        minct,
        0x80u,
        "The near distance at which names are full size");
    mincu.value.max = 3.4028235e38;
    mincu.value.min = 0.0;
    cg_overheadNamesFarDist = Dvar_RegisterFloat(
        "cg_overheadNamesFarDist",
        1024.0,
        mincu,
        0x80u,
        "The far distance at which name sizes are scaled by cg_overheadNamesFarScale");
    mincv.value.max = 3.4028235e38;
    mincv.value.min = 0.0;
    cg_overheadNamesFarScale = Dvar_RegisterFloat(
        "cg_overheadNamesFarScale",
        0.60000002,
        mincv,
        0x80u,
        "The amount to scale overhead name sizes at cg_overheadNamesFarDist");
    mincw.value.max = 100.0;
    mincw.value.min = 0.0;
    cg_overheadNamesSize = Dvar_RegisterFloat(
        "cg_overheadNamesSize",
        0.5,
        mincw,
        0x80u,
        "The maximum size to show overhead names");
    mincx.value.max = 100.0;
    mincx.value.min = 0.0;
    cg_overheadIconSize = Dvar_RegisterFloat(
        "cg_overheadIconSize",
        0.69999999,
        mincx,
        0x80u,
        "The maximum size to show overhead icons like 'rank'");
    mincy.value.max = 3.4028235e38;
    mincy.value.min = 0.0;
    cg_overheadRankSize = Dvar_RegisterFloat("cg_overheadRankSize", 0.5, mincy, 0x80u, "The size to show rank text");
    cg_overheadNamesGlow = Dvar_RegisterColor(
        "cg_overheadNamesGlow",
        0.0,
        0.0,
        0.0,
        1.0,
        0x80u,
        "Glow color for overhead names");
    cg_overheadNamesFont = Dvar_RegisterInt(
        "cg_overheadNamesFont",
        2,
        (DvarLimits)0x600000000LL,
        0x80u,
        "Font for overhead names ( see menudefinition.h )");
    cg_drawFriendlyNames = Dvar_RegisterBool("cg_drawFriendlyNames", 1, 0x80u, "Whether to show friendly names in game");
    cg_enemyNameFadeIn = Dvar_RegisterInt(
        "cg_enemyNameFadeIn",
        250,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x80u,
        "Time in milliseconds to fade in enemy names");
    cg_friendlyNameFadeIn = Dvar_RegisterInt(
        "cg_friendlyNameFadeIn",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x80u,
        "Time in milliseconds to fade in friendly names");
    cg_enemyNameFadeOut = Dvar_RegisterInt(
        "cg_enemyNameFadeOut",
        250,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x80u,
        "Time in milliseconds to fade out enemy names");
    cg_friendlyNameFadeOut = Dvar_RegisterInt(
        "cg_friendlyNameFadeOut",
        1500,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x80u,
        "Time in milliseconds to fade out friendly names");
    cg_drawThroughWalls = Dvar_RegisterBool(
        "cg_drawThroughWalls",
        0,
        0x80u,
        "Whether to draw friendly names through walls or not");
    cg_blood = Dvar_RegisterBool("cg_blood", 1, 1u, "Show Blood");
    cg_weaponleftbone = Dvar_RegisterString("cg_weaponleftbone", "tag_weapon_left", 0, "Left hand weapon bone name");
    cg_weaponrightbone = Dvar_RegisterString("cg_weaponrightbone", "tag_weapon_right", 0, "Right handed weapon bone name");
    cg_invalidCmdHintDuration = Dvar_RegisterInt(
        "cg_invalidCmdHintDuration",
        1800,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Duration of an invalid command hint");
    cg_invalidCmdHintBlinkInterval = Dvar_RegisterInt(
        "cg_invalidCmdHintBlinkInterval",
        600,
        (DvarLimits)0x7FFFFFFF00000001LL,
        1u,
        "Blink rate of an invalid command hint");
    mincz.value.max = 3.4028235e38;
    mincz.value.min = 0.0;
    cg_viewZSmoothingMin = Dvar_RegisterFloat(
        "cg_viewZSmoothingMin",
        1.0,
        mincz,
        1u,
        "Threshhold for the minimum smoothing distance it must move to smooth");
    minda.value.max = 3.4028235e38;
    minda.value.min = 0.0;
    cg_viewZSmoothingMax = Dvar_RegisterFloat(
        "cg_viewZSmoothingMax",
        16.0,
        minda,
        1u,
        "Threshhold for the maximum smoothing distance we'll do");
    mindb.value.max = 3.4028235e38;
    mindb.value.min = 0.0;
    cg_viewZSmoothingTime = Dvar_RegisterFloat(
        "cg_viewZSmoothingTime",
        0.1,
        mindb,
        1u,
        "Amount of time to spread the smoothing over");
    overrideNVGModelWithKnife = Dvar_RegisterBool(
        "overrideNVGModelWithKnife",
        0,
        0x1000u,
        "When true, nightvision animations will attach the weapDef's knife model instead of the n"
        "ight vision goggles.");
    CG_ViewRegisterDvars();
    DynEntCl_RegisterDvars();
    CG_OffhandRegisterDvars();
    CG_CompassRegisterDvars();
    CG_AmmoCounterRegisterDvars();
    CG_RegisterVisionSetsDvars();
    CG_RegisterScoreboardDvars();
    CG_HudElemRegisterDvars();
    CG_ClientSideEffectsRegisterDvars();
    CG_VehRegisterDvars();
    g_compassShowEnemies = Dvar_RegisterBool(
        "g_compassShowEnemies",
        0,
        0x84u,
        "Whether enemies are visible on the compass at all times");
    BG_RegisterDvars();
    cg_drawWVisDebug = Dvar_RegisterBool("cg_drawWVisDebug", 0, 0, "Display weapon visibility debug info");
    debugOverlay = Dvar_RegisterEnum(
        "debugOverlay",
        debugOverlayNames,
        0,
        0,
        "Toggles the display of various debug info.");
}

void __cdecl TRACK_cg_main()
{
    track_static_alloc_internal(cgDC, 5752, "cgDC", 34);
    track_static_alloc_internal(cgArray, 1045888, "cgArray", 9);
    track_static_alloc_internal(cgsArray, 14884, "cgsArray", 9);
    track_static_alloc_internal(cg_entitiesArray, 487424, "cg_entitiesArray", 9);
    track_static_alloc_internal(cg_weaponsArray, 8704, "cg_weaponsArray", 9);
    track_static_alloc_internal(cg_entityOriginArray, 12288, "cg_entityOriginArray", 9);
}

void __cdecl CG_GetDObjOrientation(int localClientNum, signed int dobjHandle, float (*axis)[3], float *origin)
{
    centity_s *cent; // [esp+Ch] [ebp-4h]

    if ((unsigned int)dobjHandle >= 0x480)
        MyAssertHandler(
            ".\\cgame_mp\\cg_main_mp.cpp",
            744,
            0,
            "%s\n\t(dobjHandle) = %i",
            "(dobjHandle >= 0 && dobjHandle < (((1<<10)) + 128))",
            dobjHandle);
    if (!axis)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 745, 0, "%s", "axis");
    if (!origin)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 746, 0, "%s", "origin");
    if (dobjHandle >= 1024)
    {
        if (dobjHandle - 1024 >= 128)
            MyAssertHandler(
                ".\\cgame_mp\\cg_main_mp.cpp",
                755,
                0,
                "%s\n\t(dobjHandle) = %i",
                "(dobjHandle >= ((1<<10)) && dobjHandle - ((1<<10)) < 128)",
                dobjHandle);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        AxisCopy(cgArray[0].viewModelAxis, axis);
        *origin = cgArray[0].viewModelAxis[3][0];
        origin[1] = cgArray[0].viewModelAxis[3][1];
        origin[2] = cgArray[0].viewModelAxis[3][2];
    }
    else
    {
        cent = CG_GetEntity(localClientNum, dobjHandle);
        AnglesToAxis(cent->pose.angles, axis);
        *origin = cent->pose.origin[0];
        origin[1] = cent->pose.origin[1];
        origin[2] = cent->pose.origin[2];
    }
}

void __cdecl CG_GetSoundEntityOrientation(SndEntHandle sndEnt, float *origin_out, float (*axis_out)[3])
{
    if (sndEnt.handle < 1024)
        CG_CopyEntityOrientation(0, sndEnt.handle, origin_out, axis_out);
    else
        CG_CopyClientSideSoundEntityOrientation(sndEnt.field.entIndex - 1024, origin_out, axis_out);
}

void __cdecl CG_CopyEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3])
{
    centity_s *Entity; // eax

    if (!origin_out)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 768, 0, "%s", "origin_out");
    if (!axis_out)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 769, 0, "%s", "axis_out");
    Entity = CG_GetEntity(localClientNum, entIndex);
    *origin_out = Entity->pose.origin[0];
    origin_out[1] = Entity->pose.origin[1];
    origin_out[2] = Entity->pose.origin[2];
    AnglesToAxis(Entity->pose.angles, axis_out);
}

const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return &cgArray[0].predictedPlayerState;
}

void __cdecl CG_GameMessage(int localClientNum, const char *msg)
{
    CL_ConsolePrint(localClientNum, 2, msg, 0, cg_gameMessageWidth->current.integer, 0);
}

void __cdecl CG_BoldGameMessage(int localClientNum, const char *msg)
{
    CL_ConsolePrint(localClientNum, 3, msg, 0, cg_gameBoldMessageWidth->current.integer, 0);
}

void __cdecl CG_RegisterSounds()
{
    cgMedia.landDmgSound = Com_FindSoundAlias("land_damage");
    CG_RegisterSurfaceTypeSounds("grenade_explode", cgMedia.grenadeExplodeSound);
    CG_RegisterSurfaceTypeSounds("rocket_explode", cgMedia.rocketExplodeSound);
    CG_RegisterSurfaceTypeSounds("bullet_small", cgMedia.bulletHitSmallSound);
    CG_RegisterSurfaceTypeSounds("bullet_large", cgMedia.bulletHitLargeSound);
    CG_RegisterSurfaceTypeSounds("bullet_ap", cgMedia.bulletHitAPSound);
    CG_RegisterSurfaceTypeSounds("bulletspray_small", cgMedia.shotgunHitSound);
    CG_RegisterSurfaceTypeSounds("bullet_small_exit", cgMedia.bulletExitSmallSound);
    CG_RegisterSurfaceTypeSounds("bullet_large_exit", cgMedia.bulletExitLargeSound);
    CG_RegisterSurfaceTypeSounds("bullet_ap_exit", cgMedia.bulletExitAPSound);
    CG_RegisterSurfaceTypeSounds("bulletspray_small_exit", cgMedia.shotgunExitSound);
    CG_RegisterSurfaceTypeSounds("step_sprint", cgMedia.stepSprintSound);
    CG_RegisterSurfaceTypeSounds("step_sprint_plr", cgMedia.stepSprintSoundPlayer);
    CG_RegisterSurfaceTypeSounds("step_run", cgMedia.stepRunSound);
    CG_RegisterSurfaceTypeSounds("step_run_plr", cgMedia.stepRunSoundPlayer);
    CG_RegisterSurfaceTypeSounds("step_walk", cgMedia.stepWalkSound);
    CG_RegisterSurfaceTypeSounds("step_walk_plr", cgMedia.stepWalkSoundPlayer);
    CG_RegisterSurfaceTypeSounds("step_prone", cgMedia.stepProneSound);
    CG_RegisterSurfaceTypeSounds("step_prone_plr", cgMedia.stepProneSoundPlayer);
    CG_RegisterSurfaceTypeSounds("land", cgMedia.landSound);
    CG_RegisterSurfaceTypeSounds("land_plr", cgMedia.landSoundPlayer);
    CG_RegisterPhysicsSounds();
    cgMedia.sprintingEquipmentSound = Com_FindSoundAlias("gear_rattle_sprint");
    cgMedia.sprintingEquipmentSoundPlayer = Com_FindSoundAlias("gear_rattle_plr_sprint");
    cgMedia.runningEquipmentSound = Com_FindSoundAlias("gear_rattle_run");
    cgMedia.runningEquipmentSoundPlayer = Com_FindSoundAlias("gear_rattle_plr_run");
    cgMedia.walkingEquipmentSound = Com_FindSoundAlias("gear_rattle_walk");
    cgMedia.walkingEquipmentSoundPlayer = Com_FindSoundAlias("gear_rattle_plr_walk");
    CG_RegisterSurfaceTypeSounds("qstep_sprint", &cgMedia.stepSprintSound[29]);
    CG_RegisterSurfaceTypeSounds("qstep_sprint_plr", &cgMedia.stepSprintSoundPlayer[29]);
    CG_RegisterSurfaceTypeSounds("qstep_run", &cgMedia.stepRunSound[29]);
    CG_RegisterSurfaceTypeSounds("qstep_run_plr", &cgMedia.stepRunSoundPlayer[29]);
    CG_RegisterSurfaceTypeSounds("qstep_walk", &cgMedia.stepWalkSound[29]);
    CG_RegisterSurfaceTypeSounds("qstep_walk_plr", &cgMedia.stepWalkSoundPlayer[29]);
    CG_RegisterSurfaceTypeSounds("qstep_prone", &cgMedia.stepProneSound[29]);
    CG_RegisterSurfaceTypeSounds("qstep_prone_plr", &cgMedia.stepProneSoundPlayer[29]);
    CG_RegisterSurfaceTypeSounds("qland", &cgMedia.landSound[29]);
    CG_RegisterSurfaceTypeSounds("qland_plr", &cgMedia.landSoundPlayer[29]);
    cgMedia.qsprintingEquipmentSound = Com_FindSoundAlias("qgear_rattle_sprint");
    cgMedia.qsprintingEquipmentSoundPlayer = Com_FindSoundAlias("qgear_rattle_plr_sprint");
    cgMedia.qrunningEquipmentSound = Com_FindSoundAlias("qgear_rattle_run");
    cgMedia.qrunningEquipmentSoundPlayer = Com_FindSoundAlias("qgear_rattle_plr_run");
    cgMedia.qwalkingEquipmentSound = Com_FindSoundAlias("qgear_rattle_walk");
    cgMedia.qwalkingEquipmentSoundPlayer = Com_FindSoundAlias("qgear_rattle_plr_walk");
    cgMedia.playerSprintGasp = Com_FindSoundAlias("sprint_gasp");
    cgMedia.foliageMovement = Com_FindSoundAlias("movement_foliage");
    cgMedia.bulletWhizby = Com_FindSoundAlias("whizby");
    cgMedia.meleeHit = Com_FindSoundAlias("melee_hit");
    cgMedia.meleeHitOther = Com_FindSoundAlias("melee_hit_other");
    cgMedia.meleeKnifeHit = Com_FindSoundAlias("melee_knife_hit_body");
    cgMedia.meleeKnifeHitOther = Com_FindSoundAlias("melee_knife_hit_other");
    cgMedia.nightVisionOn = Com_FindSoundAlias("item_nightvision_on");
    cgMedia.nightVisionOff = Com_FindSoundAlias("item_nightvision_off");
    cgMedia.playerHeartBeatSound = Com_FindSoundAlias("weap_sniper_heartbeat");
    cgMedia.playerBreathInSound = Com_FindSoundAlias("weap_sniper_breathin");
    cgMedia.playerBreathOutSound = Com_FindSoundAlias("weap_sniper_breathout");
    cgMedia.playerBreathGaspSound = Com_FindSoundAlias("weap_sniper_breathgasp");
    cgMedia.playerSwapOffhand = Com_FindSoundAlias("weap_offhand_select");
}

void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, snd_alias_list_t **sound)
{
    const char *v2; // eax
    snd_alias_list_t *defaultAliasList; // [esp+0h] [ebp-110h]
    snd_alias_list_t *defaultAliasLista; // [esp+0h] [ebp-110h]
    int i; // [esp+4h] [ebp-10Ch]
    int ia; // [esp+4h] [ebp-10Ch]
    char szAliasName[260]; // [esp+8h] [ebp-108h] BYREF

    if (!pszType)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 871, 0, "%s", "pszType");
    if (*pszType)
    {
        sprintf(szAliasName, "%s_default", pszType);
        defaultAliasLista = Com_FindSoundAlias(szAliasName);
        for (ia = 0; ia < 29; ++ia)
        {
            v2 = Com_SurfaceTypeToName(ia);
            sprintf(szAliasName, "%s_%s", pszType, v2);
            sound[ia] = Com_FindSoundAliasNoErrors(szAliasName);
            if (!sound[ia])
                sound[ia] = defaultAliasLista;
        }
    }
    else
    {
        Com_DPrintf(9, "WARNING: no alias prefix defined, using default\n");
        defaultAliasList = Com_FindSoundAliasNoErrors("collision_default");
        for (i = 0; i < 29; ++i)
            sound[i] = defaultAliasList;
    }
}

void CG_RegisterPhysicsSounds_LoadObj()
{
    char classes[50][64]; // [esp+0h] [ebp-C98h] BYREF
    PhysPreset *physPreset; // [esp+C84h] [ebp-14h]
    int nclasses; // [esp+C88h] [ebp-10h] BYREF
    const char **physicsFiles; // [esp+C8Ch] [ebp-Ch]
    int i; // [esp+C90h] [ebp-8h]
    int physPresetCount; // [esp+C94h] [ebp-4h] BYREF

    nclasses = 0;
    physicsFiles = FS_ListFilesInLocation("physic", "", FS_LIST_PURE_ONLY, &physPresetCount, 59);
    if (physPresetCount <= 50)
    {
        for (i = 0; i < physPresetCount; ++i)
        {
            if (physicsFiles[i])
            {
                physPreset = PhysPresetPrecache(physicsFiles[i], (void *(__cdecl *)(int))Hunk_AllocPhysPresetPrecache);
                CG_AddAudioPhysicsClass(physPreset, classes, &nclasses);
            }
        }
        FS_FreeFileList(physicsFiles);
    }
    else
    {
        Com_PrintError(20, "ERROR: exceeded 'audio class' max %d > %d\n", physPresetCount, 50);
    }
}

int CG_RegisterPhysicsSounds()
{
    if (useFastFile->current.enabled)
        return ((int(__cdecl *)(int (*)()))CG_RegisterPhysicsSounds_FastFile)(CG_RegisterPhysicsSounds_FastFile);
    else
        return ((int(__cdecl *)(void (*)()))CG_RegisterPhysicsSounds_LoadObj)(CG_RegisterPhysicsSounds_LoadObj);
}

int CG_RegisterPhysicsSounds_FastFile()
{
    int result; // eax
    char classes[50][64]; // [esp+0h] [ebp-D60h] BYREF
    PhysPreset *physPreset; // [esp+C88h] [ebp-D8h]
    int nclasses; // [esp+C8Ch] [ebp-D4h] BYREF
    int i; // [esp+C90h] [ebp-D0h]
    int physPresetCount; // [esp+C94h] [ebp-CCh]
    XAssetHeader assets[50]; // [esp+C98h] [ebp-C8h] BYREF

    nclasses = 0;
    result = DB_GetAllXAssetOfType(ASSET_TYPE_PHYSPRESET, assets, 50);
    physPresetCount = result;
    for (i = 0; i < physPresetCount; ++i)
    {
        physPreset = assets[i].physPreset;
        CG_AddAudioPhysicsClass(physPreset, classes, &nclasses);
    }
    return result;
}

void __cdecl CG_AddAudioPhysicsClass(PhysPreset *physPreset, char (*classes)[64], int *nclasses)
{
    int i; // [esp+0h] [ebp-4h]

    if (!physPreset)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 900, 0, "%s", "physPreset");
    if (!physPreset->sndAliasPrefix)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 901, 0, "%s", "physPreset->sndAliasPrefix");
    if (!nclasses)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 902, 0, "%s", "nclasses");
    if (!classes)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 903, 0, "%s", "classes");
    for (i = 0; i < *nclasses; ++i)
    {
        if (!I_stricmp(physPreset->sndAliasPrefix, &(*classes)[64 * i]))
        {
            physPreset->type = i;
            return;
        }
    }
    physPreset->type = *nclasses;
    CG_RegisterSurfaceTypeSounds(physPreset->sndAliasPrefix, cgMedia.physCollisionSound[physPreset->type]);
    I_strncpyz(&(*classes)[64 * *nclasses], (char *)physPreset->sndAliasPrefix, 64);
    ++*nclasses;
}

void __cdecl CG_StartAmbient(int localClientNum)
{
    int fadetime; // [esp+10h] [ebp-18h]
    char *pszInfoString; // [esp+14h] [ebp-14h]
    const char *pszFadeTime; // [esp+18h] [ebp-10h]
    const char *aliasName; // [esp+1Ch] [ebp-Ch]
    int time; // [esp+20h] [ebp-8h]
    snd_alias_t *alias; // [esp+24h] [ebp-4h]

    pszInfoString = CL_GetConfigString(localClientNum, 0x335u);
    aliasName = Info_ValueForKey(pszInfoString, "n");
    pszFadeTime = Info_ValueForKey(pszInfoString, "t");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    time = cgArray[0].time;
    fadetime = atoi(pszFadeTime) - time;
    if (fadetime < 0 || !time)
        fadetime = 0;
    if (strlen(aliasName))
    {
        alias = CL_PickSoundAlias(aliasName);
        SND_PlayAmbientAlias(localClientNum, alias, fadetime, SASYS_CGAME);
    }
    else
    {
        SND_StopAmbient(localClientNum, fadetime);
    }
}

int __cdecl CG_PlayClientSoundAlias(int localClientNum, snd_alias_list_t *aliasList)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return CG_PlaySoundAlias(localClientNum, cgArray[0].nextSnap->ps.clientNum, cgArray[0].nextSnap->ps.origin, aliasList);
}

int __cdecl CG_PlayClientSoundAliasByName(int localClientNum, const char *aliasname)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return CG_PlaySoundAliasByName(
        localClientNum,
        cgArray[0].nextSnap->ps.clientNum,
        cgArray[0].nextSnap->ps.origin,
        aliasname);
}

int __cdecl CG_PlayEntitySoundAlias(int localClientNum, int entitynum, snd_alias_list_t *aliasList)
{
    centity_s *Entity; // eax

    Entity = CG_GetEntity(localClientNum, entitynum);
    return CG_PlaySoundAlias(localClientNum, entitynum, Entity->nextState.lerp.pos.trBase, aliasList);
}

void __cdecl CG_StopSoundAlias(int localClientNum, int entitynum, snd_alias_list_t *aliasList)
{
    if (aliasList)
    {
        if (aliasList->aliasName)
            SND_StopSoundAliasOnEnt((SndEntHandle)entitynum, aliasList->aliasName);
    }
}

void __cdecl CG_StopSoundsOnEnt(int localClientNum, int entitynum)
{
    SND_StopSoundsOnEnt((SndEntHandle)entitynum);
}

void __cdecl CG_StopSoundAliasByName(int localClientNum, int entityNum, const char *aliasName)
{
    SND_StopSoundAliasOnEnt((SndEntHandle)entityNum, aliasName);
}

void __cdecl CG_StopClientSoundAliasByName(int localClientNum, const char *aliasName)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_StopSoundAliasByName(localClientNum, cgArray[0].nextSnap->ps.clientNum, aliasName);
}

void __cdecl CG_SubtitleSndLengthNotify(int msec, const snd_alias_t *lengthNotifyData)
{
    CG_SubtitlePrint(msec, lengthNotifyData);
}

void __cdecl CG_SubtitlePrint(int msec, const snd_alias_t *alias)
{
    int integer; // [esp+4h] [ebp-20h]
    int v3; // [esp+8h] [ebp-1Ch]
    float v4; // [esp+Ch] [ebp-18h]

    if (!alias)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1436, 0, "%s", "alias");
    if (!cg_subtitleWidthStandard)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1437, 0, "%s", "cg_subtitleWidthStandard");
    if (!cg_subtitleWidthWidescreen)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1438, 0, "%s", "cg_subtitleWidthWidescreen");
    if (!cg_subtitleMinTime)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1439, 0, "%s", "cg_subtitleMinTime");
    if (msec && alias->subtitle)
    {
        if (cgsArray[0].viewAspect <= 1.333333373069763)
            integer = cg_subtitleWidthStandard->current.integer;
        else
            integer = cg_subtitleWidthWidescreen->current.integer;
        v4 = cg_subtitleMinTime->current.value * 1000.0;
        v3 = (int)(v4 + 9.313225746154785e-10);
        if (v3 < msec)
            CL_SubtitlePrint(0, alias->subtitle, msec, integer);
        else
            CL_SubtitlePrint(0, alias->subtitle, v3, integer);
    }
}

void __cdecl CG_AddFXSoundAlias(int localClientNum, const float *origin, snd_alias_list_t *aliasList)
{
    snd_alias_t *alias; // [esp+Ch] [ebp-4h]

    alias = Com_PickSoundAliasFromList(aliasList);
    if (alias)
    {
        Snd_AssertAliasValid(alias);
        SND_AddPlayFXSoundAlias(alias, (SndEntHandle)1022, origin);
    }
}

int __cdecl CG_PlaySoundAlias(int localClientNum, int entitynum, const float *origin, snd_alias_list_t *aliasList)
{
    int playbackId; // [esp+8h] [ebp-Ch]
    snd_alias_t *alias; // [esp+10h] [ebp-4h]

    if (!aliasList || !aliasList->aliasName || !CG_ShouldPlaySoundOnLocalClient())
        return -1;
    alias = Com_PickSoundAliasFromList(aliasList);
    if (!alias)
        return -1;
    playbackId = SND_PlaySoundAlias(alias, (SndEntHandle)entitynum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(playbackId, alias, SndLengthNotify_Subtitle);
    return playbackId;
}

int __cdecl CG_PlaySoundAliasByName(int localClientNum, int entitynum, const float *origin, const char *aliasname)
{
    int playbackId; // [esp+8h] [ebp-Ch]
    snd_alias_t *alias; // [esp+10h] [ebp-4h]

    if (!aliasname || !CG_ShouldPlaySoundOnLocalClient())
        return -1;
    alias = CL_PickSoundAlias(aliasname);
    if (!alias)
        return -1;
    playbackId = SND_PlaySoundAlias(alias, (SndEntHandle)entitynum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(playbackId, alias, SndLengthNotify_Subtitle);
    return playbackId;
}

int __cdecl CG_PlaySoundAliasAsMasterByName(
    int localClientNum,
    int entitynum,
    const float *origin,
    const char *aliasname)
{
    int playbackId; // [esp+8h] [ebp-Ch]
    snd_alias_t *alias; // [esp+10h] [ebp-4h]

    if (!aliasname || !CG_ShouldPlaySoundOnLocalClient())
        return -1;
    alias = CL_PickSoundAlias(aliasname);
    if (!alias)
        return -1;
    playbackId = SND_PlaySoundAliasAsMaster(alias, (SndEntHandle)entitynum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(playbackId, alias, SndLengthNotify_Subtitle);
    return playbackId;
}

void __cdecl CG_RestartSmokeGrenades(int localClientNum)
{
    int eventIndex; // [esp+18h] [ebp-3Ch]
    snapshot_s *nextSnap; // [esp+20h] [ebp-34h]
    int v3; // [esp+24h] [ebp-30h]
    int i; // [esp+2Ch] [ebp-28h]
    float axis[3][3]; // [esp+30h] [ebp-24h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgsArray[0].smokeGrenadeFx)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        Com_Printf(14, "Playing smoke grenades at time %i\n", cgArray[0].time);
        FX_KillEffectDef(localClientNum, cgsArray[0].smokeGrenadeFx);
        FX_RewindTo(localClientNum, cgArray[0].time);
        nextSnap = cgArray[0].nextSnap;
        for (i = 0; i < nextSnap->numEntities; ++i)
        {
            v3 = (int)&nextSnap->entities[i];
            if ((nextSnap->entities[i].lerp.eFlags & 0x10000) != 0
                && nextSnap->entities[i].time2 >= cgArray[0].time
                && nextSnap->entities[i].lerp.u.missile.launchTime <= cgArray[0].time)
            {
                if (nextSnap->entities[i].eType)
                    MyAssertHandler(
                        ".\\cgame_mp\\cg_main_mp.cpp",
                        1584,
                        0,
                        "%s\n\t(es->eType) = %i",
                        "(es->eType == ET_GENERAL)",
                        nextSnap->entities[i].eType);
                eventIndex = ((unsigned __int8)nextSnap->entities[i].eventSequence - 1) & 3;
                if (*(int *)(v3 + 4 * eventIndex + 164) < 45 || *(int *)(v3 + 4 * eventIndex + 164) > 50)
                    MyAssertHandler(
                        ".\\cgame_mp\\cg_main_mp.cpp",
                        1586,
                        0,
                        "es->events[eventIndex] not in [EV_GRENADE_EXPLODE, EV_CUSTOM_EXPLODE_NOMARKS]\n\t%i not in [%i, %i]",
                        *(_DWORD *)(v3 + 4 * eventIndex + 164),
                        45,
                        50);
                ByteToDir(*(_DWORD *)(v3 + 4 * eventIndex + 180), axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                Com_Printf(
                    14,
                    "Restarting smoke grenade at time %i at ( %f, %f, %f )\n",
                    nextSnap->entities[i].lerp.u.missile.launchTime,
                    nextSnap->entities[i].lerp.pos.trBase[0],
                    nextSnap->entities[i].lerp.pos.trBase[1],
                    nextSnap->entities[i].lerp.pos.trBase[2]);
                FX_PlayOrientedEffect(
                    localClientNum,
                    cgsArray[0].smokeGrenadeFx,
                    nextSnap->entities[i].lerp.u.missile.launchTime,
                    nextSnap->entities[i].lerp.pos.trBase,
                    axis);
            }
        }
    }
}

void __cdecl CG_InitVote(int localClientNum)
{
    char *ConfigString; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-10h]
    int time; // [esp+8h] [ebp-8h] BYREF
    int serverId; // [esp+Ch] [ebp-4h] BYREF

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgsArray[0].voteTime = 0;
    ConfigString = CL_GetConfigString(localClientNum, 0xDu);
    if (sscanf(ConfigString, "%d %d", &time, &serverId) == 2 && serverId == LocalClientGlobals->serverId)
        cgsArray[0].voteTime = time;
    v2 = CL_GetConfigString(localClientNum, 0xFu);
    cgsArray[0].voteYes = atoi(v2);
    v3 = CL_GetConfigString(localClientNum, 0x10u);
    cgsArray[0].voteNo = atoi(v3);
    v4 = CL_GetConfigString(localClientNum, 0xEu);
    v5 = SEH_LocalizeTextMessage(v4, "vote string", LOCMSG_SAFE);
    I_strncpyz(cgsArray[0].voteString, v5, 256);
}

unsigned __int16 __cdecl CG_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType)
{
    if (weapType == WEAPTYPE_GRENADE)
        return scr_const.tag_inhand;
    if (ci->leftHandGun)
        return SL_FindString(cg_weaponleftbone->current.string);
    return SL_FindString(cg_weaponrightbone->current.string);
}

int __cdecl CG_GetClientNum(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return cgArray[0].clientNum;
}

void __cdecl CG_Init(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    char *s; // [esp+24h] [ebp-4Ch]
    char mapname[68]; // [esp+28h] [ebp-48h] BYREF

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1883, 0, "%s", "Sys_IsMainThread()");
    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    CL_GetLocalClientConnection(localClientNum);
    memset((unsigned __int8 *)cgsArray, 0, sizeof(cgsArray));
    memset((unsigned __int8 *)cgArray, 0, 0xFF580u);
    memset((unsigned __int8 *)&cgDC[localClientNum], 0, sizeof(UiContext));
    memset((unsigned __int8 *)cg_entitiesArray[localClientNum], 0, sizeof(centity_s[1024]));
    memset((unsigned __int8 *)cg_weaponsArray[localClientNum], 0, sizeof(weaponInfo_s[128]));
    cgDC[localClientNum].localClientNum = localClientNum;
    CG_ClearCompassPingData();
    CG_Veh_Init();
    CG_ClearOverheadFade();
    CG_InitDof(&cgArray[0].refdef.dof);
    Ragdoll_Init();
    Phys_Init();
    cgArray[0].localClientNum = localClientNum;
    cgArray[0].viewModelPose.eType = 17;
    cgArray[0].viewModelPose.localClientNum = localClientNum;
    if (cgArray[0].viewModelPose.localClientNum != localClientNum)
        MyAssertHandler(
            ".\\cgame_mp\\cg_main_mp.cpp",
            1912,
            0,
            "%s",
            "cgameGlob->viewModelPose.localClientNum == localClientNum");
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);
    cgArray[0].objectiveText[0] = 0;
    cgArray[0].bgs.animScriptData.soundAlias = Com_FindSoundAlias;
    cgArray[0].bgs.animScriptData.playSoundAlias = CG_PlayAnimScriptSoundAlias;
    cgArray[0].bgs.GetXModel = FX_RegisterModel;
    cgArray[0].bgs.CreateDObj = (void(__cdecl *)(DObjModel_s *, unsigned __int16, XAnimTree_s *, int, int, clientInfo_t *))CG_CreateDObj;
    cgArray[0].bgs.AttachWeapon = CG_AttachWeapon;
    cgArray[0].bgs.GetDObj = (DObj_s * (__cdecl *)(int, int))CG_GetDObj;
    cgArray[0].bgs.SafeDObjFree = (void(__cdecl *)(int, int))Com_SafeClientDObjFree;
    cgArray[0].bgs.AllocXAnim = (void *(__cdecl *)(int))Hunk_AllocXAnimClient;
    cgArray[0].bgs.anim_user = 0;
    cgArray[0].clientNum = clientNum;
    cgArray[0].drawHud = 1;
    cgArray[0].lastHealthLerpDelay = 1;
    cgsArray[0].processedSnapshotNum = serverMessageNum;
    cgsArray[0].serverCommandSequence = serverCommandSequence;
    CG_ParseServerInfo(localClientNum);
    CG_ParseCodInfo(localClientNum);
    R_BeginRemoteScreenUpdate();
    UI_LoadIngameMenus(localClientNum);
    SCR_UpdateLoadScreen();
    cgMedia.whiteMaterial = Material_RegisterHandle("white", 7);
    cgMedia.smallDevFont = CL_RegisterFont("fonts/smallDevFont", 1);
    cgMedia.bigDevFont = CL_RegisterFont("fonts/bigDevFont", 1);
    Material_RegisterHandle("net_disconnect", 7);
    cgMedia.nightVisionOverlay = Material_RegisterHandle("nightvision_overlay_goggles", 7);
    cgMedia.hudIconNVG = Material_RegisterHandle("hud_icon_nvg", 7);
    cgMedia.hudDpadArrow = Material_RegisterHandle("hud_dpad_arrow", 7);
    cgMedia.ammoCounterBullet = Material_RegisterHandle("ammo_counter_bullet", 7);
    cgMedia.ammoCounterBeltBullet = Material_RegisterHandle("ammo_counter_beltbullet", 7);
    cgMedia.ammoCounterRifleBullet = Material_RegisterHandle("ammo_counter_riflebullet", 7);
    cgMedia.ammoCounterRocket = Material_RegisterHandle("ammo_counter_rocket", 7);
    cgMedia.ammoCounterShotgunShell = Material_RegisterHandle("ammo_counter_shotgunshell", 7);
    cgMedia.textDecodeCharacters = Material_RegisterHandle("decode_characters", 7);
    cgMedia.textDecodeCharactersGlow = Material_RegisterHandle("decode_characters_glow", 7);
    Material_RegisterHandle("code_warning_soundcpu", 7);
    Material_RegisterHandle("code_warning_snapshotents", 7);
    Material_RegisterHandle("code_warning_maxeffects", 7);
    Material_RegisterHandle("code_warning_models", 7);
    Material_RegisterHandle("code_warning_file", 7);
    Material_RegisterHandle("code_warning_fps", 7);
    Material_RegisterHandle("code_warning_serverfps", 7);
    Material_RegisterHandle("killicondied", 7);
    Material_RegisterHandle("killiconcrush", 7);
    Material_RegisterHandle("killiconfalling", 7);
    Material_RegisterHandle("killiconsuicide", 7);
    Material_RegisterHandle("killiconheadshot", 7);
    Material_RegisterHandle("killiconmelee", 7);

    if (cg_fs_debug->current.integer == 2)
        Dvar_SetInt(cg_fs_debug, 0);

    CG_AntiBurnInHUD_RegisterDvars();
    CG_InitConsoleCommands();
    CG_InitViewDimensions(localClientNum);
    s = CL_GetConfigString(localClientNum, 2u);
    if (strcmp(s, "cod"))
        Com_Error(ERR_DROP, "Client/Server game mismatch: %s/%s", "cod", s);
    SCR_UpdateLoadScreen();
    if (!com_sv_running)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 2011, 0, "%s", "com_sv_running");
    if (!com_sv_running->current.enabled)
        Mantle_CreateAnims((void *(__cdecl *)(int))Hunk_AllocXAnimClient);
    cgsArray[0].localServer = com_sv_running->current.color[0];
    if (!bg_lastParsedWeaponIndex)
    {
        Com_SetWeaponInfoMemory(2);
        BG_ClearWeaponDef();
    }
    if (!g_mapLoaded && !useFastFile->current.enabled)
    {
        CG_LoadingString(localClientNum, "sound aliases");
        CL_LoadSoundAliases(cgsArray[0].mapname);
    }
    CG_SetupWeaponDef(localClientNum);
    CGScr_LoadAnimTrees();
    if (bgs)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 2041, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
    bgs = &cgArray[0].bgs;
    BG_LoadAnim();
    CG_LoadAnimTreeInstances(localClientNum);
    if (!cgsArray[0].localServer)
    {
        GScr_LoadConsts();
        BG_LoadPenetrationDepthTable();
    }
    CG_LoadingString(localClientNum, "collision map");
    CL_CM_LoadMap(cgsArray[0].mapname);
    Menu_Setup(&cgDC[localClientNum]);
    CG_LoadingString(localClientNum, "graphics");
    if (!g_mapLoaded)
    {
        CG_LoadingString(localClientNum, cgsArray[0].mapname);
        LoadWorld(cgsArray[0].mapname);
        g_mapLoaded = 1;
    }
    CG_LoadingString(localClientNum, "game media");
    if (I_strnicmp(cgsArray[0].mapname, "maps/", 5))
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 2111, 0, "%s", "!I_strnicmp( cgs->mapname, \"maps/\", 5 )");
    Com_StripExtension(&cgsArray[0].mapname[5], mapname);
    ProfLoad_Begin("Init effects system");
    FX_InitSystem(localClientNum);
    FX_RegisterDefaultEffect();
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_RegisterGraphics(localClientNum, mapname);
    CG_LoadingString(localClientNum, "clients");
    CG_LoadHudMenu(localClientNum);
    CG_InitEntities(localClientNum);
    CG_InitLocalEntities(localClientNum);
    DynEntCl_InitEntities(localClientNum);
    cgArray[0].isLoading = 0;
    CG_SetConfigValues(localClientNum);
    CG_LoadingString(localClientNum, "");
    CG_NorthDirectionChanged(localClientNum);
    if (!g_mapLoaded)
        SND_StopSounds(SND_STOP_ALL);
    CG_ParseFog(cgArray[0].time);
    R_InitPrimaryLights(cgArray[0].refdef.primaryLights);
    R_ClearShadowedPrimaryLightHistory(localClientNum);
    if (!g_ambientStarted)
    {
        CG_StartAmbient(localClientNum);
        g_ambientStarted = 1;
    }
    CL_SetADS(localClientNum, 0);
    AimTarget_Init(localClientNum);
    AimAssist_Init(localClientNum);
    CG_InitVote(localClientNum);
    CG_StartClientSideEffects(localClientNum);
    if (bgs != &cgArray[0].bgs)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 2184, 0, "%s\n\t(bgs) = %p", "(bgs == &cgameGlob->bgs)", bgs);
    bgs = 0;
    R_EndRemoteScreenUpdate();
}

clientConnection_t *__cdecl CL_GetLocalClientConnection(int localClientNum)
{
    if (!clientConnections)
        MyAssertHandler("c:\\trees\\cod3\\src\\cgame_mp\\../client_mp/client_mp.h", 1095, 0, "%s", "clientConnections");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../client_mp/client_mp.h",
            1100,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return clientConnections;
}

void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname)
{
    shellshock_parms_t *ShellshockParms; // eax
    char *shellshock; // [esp+0h] [ebp-14h]
    char *effectname; // [esp+4h] [ebp-10h]
    char *modelName; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]
    int ia; // [esp+10h] [ebp-4h]
    signed int ib; // [esp+10h] [ebp-4h]

    SCR_UpdateLoadScreen();
    CG_LoadingString(localClientNum, " - textures");
    cgMedia.lagometerMaterial = Material_RegisterHandle("lagometer", 7);
    cgMedia.connectionMaterial = Material_RegisterHandle("headicondisconnected", 7);
    cgMedia.youInKillCamMaterial = Material_RegisterHandle("headiconyouinkillcam", 7);
    Material_RegisterHandle("killiconmelee", 7);
    Material_RegisterHandle("killiconsuicide", 7);
    Material_RegisterHandle("killiconfalling", 7);
    Material_RegisterHandle("killiconcrush", 7);
    Material_RegisterHandle("killicondied", 7);
    cgMedia.tracerMaterial = Material_RegisterHandle("gfx_tracer", 6);
    cgMedia.laserMaterial = Material_RegisterHandle("gfx_laser", 6);
    cgMedia.laserLightMaterial = Material_RegisterHandle("gfx_laser_light", 6);
    cgMedia.hintMaterials[2] = Material_RegisterHandle("hint_usable", 7);
    cgMedia.hintMaterials[3] = Material_RegisterHandle("hint_health", 7);
    cgMedia.hintMaterials[4] = Material_RegisterHandle("hint_friendly", 7);
    cgMedia.stanceMaterials[0] = Material_RegisterHandle("stance_stand", 7);
    cgMedia.stanceMaterials[1] = Material_RegisterHandle("stance_crouch", 7);
    cgMedia.stanceMaterials[2] = Material_RegisterHandle("stance_prone", 7);
    cgMedia.stanceMaterials[3] = Material_RegisterHandle("stance_flash", 7);
    cgMedia.objectiveMaterials[0] = Material_RegisterHandle("objective", 7);
    cgMedia.friendMaterials[0] = Material_RegisterHandle("compassping_friendly_mp", 7);
    cgMedia.friendMaterials[1] = Material_RegisterHandle("objective_friendly_chat", 7);
    cgMedia.damageMaterial = Material_RegisterHandle("hit_direction", 7);
    cgMedia.mantleHint = Material_RegisterHandle("hint_mantle", 7);
    cgMedia.compassping_friendlyfiring = Material_RegisterHandle("compassping_friendlyfiring_mp", 7);
    cgMedia.compassping_friendlyyelling = Material_RegisterHandle("compassping_friendlyyelling_mp", 7);
    cgMedia.compassping_enemy = Material_RegisterHandle("compassping_enemy", 7);
    cgMedia.compassping_enemyfiring = Material_RegisterHandle("compassping_enemyfiring", 7);
    cgMedia.compassping_enemyyelling = Material_RegisterHandle("compassping_enemyyelling", 7);
    cgMedia.compassping_grenade = Material_RegisterHandle("compassping_grenade", 7);
    cgMedia.compassping_explosion = Material_RegisterHandle("compassping_explosion", 7);
    cgMedia.compass_radarline = Material_RegisterHandle("compass_radarline", 7);
    cgMedia.compass_helicopter_enemy = Material_RegisterHandle("compass_objpoint_helicopter_busy", 7);
    cgMedia.compass_helicopter_friendly = Material_RegisterHandle("compass_objpoint_helicopter_friendly", 7);
    cgMedia.compass_plane_enemy = Material_RegisterHandle("compass_objpoint_airstrike_busy", 7);
    cgMedia.compass_plane_friendly = Material_RegisterHandle("compass_objpoint_airstrike_friendly", 7);
    cgMedia.grenadeIconFrag = Material_RegisterHandle("hud_grenadeicon", 7);
    cgMedia.grenadeIconFlash = Material_RegisterHandle("hud_flashbangicon", 7);
    cgMedia.grenadeIconThrowBack = Material_RegisterHandle("hud_grenadethrowback", 7);
    cgMedia.grenadePointer = Material_RegisterHandle("hud_grenadepointer", 7);
    cgMedia.offscreenObjectivePointer = Material_RegisterHandle("hud_offscreenobjectivepointer", 7);
    cgMedia.teamStatusBar = Material_RegisterHandle("hudcolorbar", 7);
    CG_LoadingString(localClientNum, " - models");
    cgMedia.balloonMaterial = Material_RegisterHandle("headicontalkballoon", 7);
    CG_RegisterScoreboardGraphics();
    CG_LoadingString(localClientNum, " - items");
    CG_RegisterItems(localClientNum);
    CG_LoadingString(localClientNum, " - inline models");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_LoadingString(localClientNum, " - server models");
    for (i = 1; i < 512; ++i)
    {
        modelName = CL_GetConfigString(localClientNum, i + 830);
        if (*modelName)
        {
            SCR_UpdateLoadScreen();
            cgsArray[0].gameModels[i] = R_RegisterModel(modelName);
        }
    }
    for (ia = 1; ia < 100; ++ia)
    {
        effectname = CL_GetConfigString(localClientNum, ia + 1598);
        if (*effectname)
        {
            cgsArray[0].fxs[ia] = FX_Register(effectname);
            if (!cgsArray[0].fxs[ia])
                MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1196, 0, "%s", "cgs->fxs[i]");
        }
    }
    cgsArray[0].smokeGrenadeFx = FX_Register("props/american_smoke_grenade_mp");
    if (!cgsArray[0].smokeGrenadeFx)
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1200, 0, "%s", "cgs->smokeGrenadeFx");
    for (ib = 1; ib < 16; ++ib)
    {
        shellshock = CL_GetConfigString(localClientNum, ib + 1954);
        if (*shellshock)
        {
            if (!BG_LoadShellShockDvars(shellshock))
                Com_Error(ERR_DROP, &byte_86B96C, shellshock);
            ShellshockParms = BG_GetShellshockParms(ib);
            BG_SetShellShockParmsFromDvars(ShellshockParms);
        }
    }
    if (!BG_LoadShellShockDvars("hold_breath"))
        Com_Error(ERR_DROP, "Couldn't find shock file [hold_breath.shock]\n");
    BG_SetShellShockParmsFromDvars(&cgsArray[0].holdBreathParams);
    cgMedia.fx = CG_RegisterImpactEffects(mapname);
    if (!cgMedia.fx)
        Com_Error(ERR_DROP, &byte_86B8E8);
    cgMedia.fxNoBloodFleshHit = FX_Register("impacts/flesh_hit_noblood");
    cgMedia.fxKnifeBlood = FX_Register("impacts/flesh_hit_knife");
    cgMedia.fxKnifeNoBlood = FX_Register("impacts/flesh_hit_knife_noblood");
    cgMedia.fxVehicleTireDust = FX_Register("dust/dust_vehicle_tires");
    cgMedia.heliDustEffect = FX_Register("treadfx/heli_dust_default");
    cgMedia.heliWaterEffect = FX_Register("treadfx/heli_water");
    cgMedia.helicopterLightSmoke = FX_Register("smoke/smoke_trail_white_heli");
    cgMedia.helicopterHeavySmoke = FX_Register("smoke/smoke_trail_black_heli");
    cgMedia.helicopterOnFire = FX_Register("fire/fire_smoke_trail_L");
    cgMedia.jetAfterburner = FX_Register("fire/jet_afterburner");
    CG_LoadingString(localClientNum, " - game media done");
}

int __cdecl CG_PlayAnimScriptSoundAlias(int clientIndex, snd_alias_list_t *aliasList)
{
    return CG_PlayClientSoundAlias(clientIndex, aliasList);
}

void __cdecl CG_LoadHudMenu(int localClientNum)
{
    menuDef_t *menu; // [esp+4h] [ebp-Ch]
    MenuList *menuList; // [esp+8h] [ebp-8h]

    menuList = UI_LoadMenus("ui_mp/hud.txt", 7);
    UI_AddMenuList(&cgDC[localClientNum], menuList);
    if (CL_GetLocalClientActiveCount() == 1)
        menu = Menus_FindByName(&cgDC[localClientNum], "Compass");
    else
        menu = Menus_FindByName(&cgDC[localClientNum], "Compass_mp");
    if (menu)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgsArray[0].compassWidth = menu->window.rect.w;
        cgsArray[0].compassHeight = menu->window.rect.h;
        cgsArray[0].compassY = menu->window.rect.y;
    }
}

unsigned __int16 __cdecl CG_AttachWeapon(DObjModel_s *dobjModels, unsigned __int16 numModels, clientInfo_t *ci)
{
    unsigned __int8 weaponModel; // [esp+7h] [ebp-5h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if (ci->iDObjWeapon)
    {
        weapDef = BG_GetWeaponDef(ci->iDObjWeapon);
        weaponModel = ci->weaponModel;
        if (weapDef->worldModel[weaponModel] && !ci->hideWeapon)
        {
            if (numModels >= 0x20u)
                MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1698, 0, "%s", "numModels < DOBJ_MAX_SUBMODELS");
            dobjModels[numModels].model = weapDef->worldModel[weaponModel];
            dobjModels[numModels].boneName = CG_GetWeaponAttachBone(ci, weapDef->weapType);
            dobjModels[numModels++].ignoreCollision = 0;
        }
        if (weapDef->worldKnifeModel && ci->usingKnife)
        {
            if (numModels >= 0x20u)
                MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1709, 0, "%s", "numModels < DOBJ_MAX_SUBMODELS");
            dobjModels[numModels].model = weapDef->worldKnifeModel;
            dobjModels[numModels].boneName = scr_const.tag_inhand;
            dobjModels[numModels++].ignoreCollision = 0;
        }
    }
    return numModels;
}

void __cdecl CG_CreateDObj(
    DObjModel_s *dobjModels,
    unsigned __int16 numModels,
    XAnimTree_s *tree,
    unsigned int handle,
    int localClientNum,
    clientInfo_t *ci)
{
    float *v6; // eax

    Com_ClientDObjCreate(dobjModels, numModels, tree, handle, localClientNum);
    v6 = cg_entityOriginArray[localClientNum][ci->clientNum];
    *v6 = 131072.0;
    v6[1] = 131072.0;
    v6[2] = 131072.0;
}

DObj_s *__cdecl CG_GetDObj(unsigned int handle, int localClientNum)
{
    return Com_GetClientDObj(handle, localClientNum);
}

void __cdecl CG_LoadAnimTreeInstances(int localClientNum)
{
    XAnim_s *generic_human; // [esp+0h] [ebp-10h]
    int i; // [esp+Ch] [ebp-4h]
    int ia; // [esp+Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    generic_human = (XAnim_s *)MEMORY[0xA7B0C8];
    for (i = 0; i < 64; ++i)
        cgArray[0].bgs.clientinfo[i].pXAnimTree = XAnimCreateTree(
            generic_human,
            (void *(__cdecl *)(int))Hunk_AllocXAnimClient);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (ia = 0; ia < 8; ++ia)
        cgsArray[0].corpseinfo[ia].pXAnimTree = XAnimCreateTree(
            generic_human,
            (void *(__cdecl *)(int))Hunk_AllocXAnimClient);
}

void __cdecl CG_InitEntities(int localClientNum)
{
    signed int entityIndex; // [esp+0h] [ebp-Ch]
    centity_s *cent; // [esp+8h] [ebp-4h]

    for (entityIndex = 0; entityIndex < 1024; ++entityIndex)
    {
        cent = CG_GetEntity(localClientNum, entityIndex);
        if (!cent)
            MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 1819, 0, "%s", "cent");
        cent->pose.localClientNum = localClientNum;
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    MEMORY[0x9D84DC] = localClientNum;
}

void __cdecl CG_InitViewDimensions(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgsArray[0].viewX = 0;
    CL_GetScreenDimensions(&cgsArray[0].viewWidth, &cgsArray[0].viewHeight, &cgsArray[0].viewAspect);
    if (cgsArray[0].viewWidth <= 0)
        MyAssertHandler(
            ".\\cgame_mp\\cg_main_mp.cpp",
            1838,
            1,
            "%s\n\t(cgs->viewWidth) = %i",
            "(cgs->viewWidth > 0)",
            cgsArray[0].viewWidth);
    if (cgsArray[0].viewHeight <= 0)
        MyAssertHandler(
            ".\\cgame_mp\\cg_main_mp.cpp",
            1839,
            1,
            "%s\n\t(cgs->viewHeight) = %i",
            "(cgs->viewHeight > 0)",
            cgsArray[0].viewHeight);
    if (cgsArray[0].viewAspect <= 0.0)
        MyAssertHandler(
            ".\\cgame_mp\\cg_main_mp.cpp",
            1840,
            1,
            "%s\n\t(cgs->viewAspect) = %g",
            "(cgs->viewAspect > 0)",
            cgsArray[0].viewAspect);
}

void __cdecl CG_InitDof(GfxDepthOfField *dof)
{
    dof->nearStart = 0.0;
    dof->nearEnd = 0.0;
    dof->farStart = 5000.0;
    dof->farEnd = 5000.0;
    dof->nearBlur = 6.0;
    dof->farBlur = 0.0;
}

void __cdecl CG_FreeWeapons(int localClientNum)
{
    unsigned int v1; // eax
    weaponInfo_s *weapInfo; // [esp+0h] [ebp-8h]
    unsigned int weapIndex; // [esp+4h] [ebp-4h]

    for (weapIndex = 1; weapIndex < BG_GetNumWeapons(); ++weapIndex)
    {
        v1 = CG_WeaponDObjHandle(weapIndex);
        Com_SafeClientDObjFree(v1, localClientNum);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        weapInfo = &cg_weaponsArray[0][weapIndex];
        if (weapInfo->tree)
        {
            XAnimFreeTree(weapInfo->tree, 0);
            weapInfo->tree = 0;
        }
    }
    memset((unsigned __int8 *)cg_weaponsArray[localClientNum], 0, sizeof(weaponInfo_s[128]));
}

void __cdecl CG_Shutdown(int localClientNum)
{
    centity_s *cent; // [esp+Ch] [ebp-Ch]
    signed int entnum; // [esp+14h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    R_TrackStatistics(0);
    SND_FadeAllSounds(1.0, 0);
    for (entnum = 0; entnum < 1024; ++entnum)
    {
        cent = CG_GetEntity(localClientNum, entnum);
        if (cent->pose.ragdollHandle && cent->pose.ragdollHandle != -1)
        {
            Ragdoll_Remove(cent->pose.ragdollHandle);
            cent->pose.ragdollHandle = 0;
        }
        if (cent->pose.physObjId)
        {
            if (cent->pose.physObjId != -1)
            {
                Phys_ObjDestroy(PHYS_WORLD_FX, (dxBody *)cent->pose.physObjId);
                cent->pose.physObjId = 0;
            }
        }
    }
    Ragdoll_Shutdown();
    g_ambientStarted = 0;
    g_mapLoaded = 0;
    Mantle_ShutdownAnims();
    if (!useFastFile->current.enabled)
        Menus_FreeAllMemory(&cgDC[localClientNum]);
    CG_FreeWeapons(localClientNum);
    CG_FreeClientDObjInfo(localClientNum);
    CG_FreeEntityDObjInfo(localClientNum);
    Com_FreeWeaponInfoMemory(2);
    FX_KillAllEffects(localClientNum);
    FX_ShutdownSystem(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    CG_FreeAnimTreeInstances(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgsArray[0].localServer)
        Scr_ShutdownGameStrings();
    memset((unsigned __int8 *)cgArray, 0, sizeof(cgArray));
    if (MEMORY[0x98F45C])
        MyAssertHandler(".\\cgame_mp\\cg_main_mp.cpp", 2298, 0, "%s", "!cgameGlob->nextSnap");
    CG_ClearCompassPingData();
    CG_ShutdownConsoleCommands();
}

void __cdecl CG_FreeAnimTreeInstances(int localClientNum)
{
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (i = 0; i < 64; ++i)
    {
        if (cgArray[0].bgs.clientinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgArray[0].bgs.clientinfo[i].pXAnimTree, 0);
            cgArray[0].bgs.clientinfo[i].pXAnimTree = 0;
        }
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    for (ia = 0; ia < 8; ++ia)
    {
        if (cgsArray[0].corpseinfo[ia].pXAnimTree)
        {
            XAnimFreeTree(cgsArray[0].corpseinfo[ia].pXAnimTree, 0);
            cgsArray[0].corpseinfo[ia].pXAnimTree = 0;
        }
    }
}

unsigned __int8 *__cdecl Hunk_AllocXAnimClient(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXAnimClient", 11);
}

unsigned __int8 __cdecl CG_ShouldPlaySoundOnLocalClient()
{
    return 1;
}

