#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_main.h"

#include "cg_local.h"
#include <DynEntity/DynEntity_client.h>
#include "cg_vehicle_hud.h"
#include <universal/com_sndalias.h>
#include <universal/surfaceflags.h>
#include <database/database.h>
#include "cg_servercmds.h"
#include <gfx_d3d/r_model.h>
#include <EffectsCore/fx_system.h>
#include <ragdoll/ragdoll.h>
#include "cg_consolecmds.h"
#include "cg_newdraw.h"
#include <aim_assist/aim_assist.h>
#include "cg_modelpreviewer.h"
#include <game/g_local.h>

UiContext cgDC;

cgs_t cgsArray[1];
cg_s cgArray[1];
cgMedia_t cgMedia;

weaponInfo_s cg_weaponsArray[1][128];
centity_s cg_entitiesArray[1][2176];
float cg_entityOriginArray[1][2176][3];

const dvar_t *cg_hudGrenadeIconEnabledFlash;
const dvar_t *vehHelicopterHeadSwayOnRollVert;
const dvar_t *cg_hudGrenadePointerPulseMax;
const dvar_t *cg_laserLight;
const dvar_t *hostileNameFontColor;
const dvar_t *cg_drawVersionY;
const dvar_t *turretScopeZoomRate;
const dvar_t *turretScopeZoomMax;
const dvar_t *cg_drawVersion;
const dvar_t *cg_gun_y;
const dvar_t *cg_drawBreathHint;
const dvar_t *cg_drawFriendlyFireCrosshair;
const dvar_t *cg_bloodLimitMsec;
const dvar_t *cg_hudGrenadeIconMaxRangeFrag;
const dvar_t *cg_drawFPS;
const dvar_t *cg_drawVersionX;
const dvar_t *vehHelicopterHeadSwayOnRollHorz;
const dvar_t *cg_drawHealth;
const dvar_t *cg_brass;
const dvar_t *replay_time;
const dvar_t *cg_weaponHintsCoD1Style;
const dvar_t *debugOverlay;
const dvar_t *cg_viewZSmoothingMax;
const dvar_t *cg_marks_ents_player_only;
const dvar_t *cg_drawCrosshair;
const dvar_t *hostileNameFontGlowColor;
const dvar_t *cg_drawPerformanceWarnings;
const dvar_t *cg_laserRadius;
const dvar_t *cg_invalidCmdHintDuration;
const dvar_t *hud_drawHUD;
const dvar_t *cg_mapLocationSelectionCursorSpeed;
const dvar_t *turretScopeZoom;
const dvar_t *cg_gun_rot_rate;
const dvar_t *cg_objectiveListWrapCountStandard;
const dvar_t *cg_hudGrenadeIconWidth;
const dvar_t *cg_gun_z;
const dvar_t *cg_hudDamageIconOffset;
const dvar_t *overrideNVGModelWithKnife;
const dvar_t *cg_viewZSmoothingMin;
const dvar_t *cg_drawTurretCrosshair;
const dvar_t *cg_hudDamageIconHeight;
const dvar_t *cg_drawShellshock;
const dvar_t *cg_gun_move_minspeed;
const dvar_t *cg_gun_ofs_u;
const dvar_t *cg_subtitles;
const dvar_t *cg_hudStanceHintPrints;
const dvar_t *vehHelicopterHeadSwayOnPitch;
const dvar_t *cg_modPrvMruAnims;
const dvar_t *cg_hudDamageIconWidth;
const dvar_t *cg_viewVehicleInfluence;
const dvar_t *cg_draw2D;
const dvar_t *cg_gun_x;
const dvar_t *hud_showStance;
const dvar_t *cg_crosshairAlpha;
const dvar_t *cg_laserLightRadius;
const dvar_t *hud_missionFailed;
const dvar_t *cg_objectiveListWrapCountWidescreen;
const dvar_t *cg_gun_rot_r;
const dvar_t *friendlyNameFontSize;
const dvar_t *cg_drawGun;
const dvar_t *cg_hudGrenadeIconOffset;
const dvar_t *cg_modPrvMruModels;
const dvar_t *cg_centertime;
const dvar_t *cg_hudGrenadePointerPulseFreq;
const dvar_t *cg_minicon;
const dvar_t *cg_gun_move_rate;
const dvar_t *cg_tracerSpeed;
const dvar_t *cg_bobWeaponRollAmplitude;
const dvar_t *cg_viewKickMax;
const dvar_t *cg_laserFlarePct;
const dvar_t *cg_invalidCmdHintBlinkInterval;
const dvar_t *friendlyNameFontGlowColor;
const dvar_t *cg_hudGrenadePointerPivot;
const dvar_t *cg_crosshairAlphaMin;
const dvar_t *vehHelicopterFreeLookReleaseSpeed;
const dvar_t *cg_laserLightEndOffset;
const dvar_t *cg_tracerChance;
const dvar_t *cg_hudGrenadeIconInScope;
const dvar_t *cg_crosshairEnemyColor;
const dvar_t *cg_drawRumbleDebug;
const dvar_t *cg_laserRangePlayer;
const dvar_t *cg_tracerScale;
const dvar_t *cg_weaponCycleDelay;
const dvar_t *cg_laserRange;
const dvar_t *cg_laserForceOn;
const dvar_t *cg_gun_rot_y;
const dvar_t *cg_gameMessageWidth;
const dvar_t *cg_gun_ofs_r;
const dvar_t *cg_hudGrenadeIconMaxHeight;
const dvar_t *cg_modPrvDrawAxis;
const dvar_t *cg_debugEvents;
const dvar_t *cg_hudGrenadeIconMaxRangeFlash;
const dvar_t *cg_drawPlayerPosInFreeMove;
const dvar_t *cg_hudStanceFlash;
const dvar_t *cg_drawScriptUsage;
const dvar_t *cg_drawHUD;
const dvar_t *cg_crosshairDynamic;
const dvar_t *cg_hudDamageIconInScope;
const dvar_t *cg_gun_move_r;
const dvar_t *cg_viewZSmoothingTime;
const dvar_t *cg_bobWeaponAmplitude;
const dvar_t *cg_footsteps;
const dvar_t *cg_gun_move_f;
const dvar_t *cg_hudGrenadeIconHeight;
const dvar_t *friendlyNameFontObjective;
const dvar_t *cg_hudGrenadePointerWidth;
const dvar_t *cg_nopredict;
const dvar_t *cg_drawMaterial;
const dvar_t *cg_r_forceLod;
const dvar_t *snd_drawInfo;
const dvar_t *cg_debug_overlay_viewport;
const dvar_t *cg_small_dev_string_fontscale;
const dvar_t *cg_dumpAnims;
const dvar_t *cg_gun_rot_p;
const dvar_t *cg_viewKickScale;
const dvar_t *cg_errorDecay;
const dvar_t *cg_subtitleWidthStandard;
const dvar_t *friendlyNameFontColor;
const dvar_t *sv_znear;
const dvar_t *cg_hudGrenadePointerPulseMin;
const dvar_t *cg_bobWeaponMax;
const dvar_t *cg_tracerScaleMinDist;
const dvar_t *cg_laserEndOffset;
const dvar_t *cg_drawMantleHint;
const dvar_t *cg_gameBoldMessageWidth;
const dvar_t *cg_tracerWidth;
const dvar_t *cg_bobWeaponLag;
const dvar_t *cg_laserLightBodyTweak;
const dvar_t *cg_bloodLimit;
const dvar_t *cg_drawFPSLabels;
const dvar_t *cg_drawpaused;
const dvar_t *cg_hudDamageIconTime;
const dvar_t *cg_debugInfoCornerOffset;
const dvar_t *cg_gun_rot_minspeed;
const dvar_t *cg_blood;
const dvar_t *cg_showmiss;
const dvar_t *cg_fov;
const dvar_t *turretScopeZoomMin;
const dvar_t *cg_subtitleWidthWidescreen;
const dvar_t *cg_developer;
const dvar_t *cg_tracerScrewRadius;
const dvar_t *cg_hudGrenadePointerHeight;
const dvar_t *cg_gun_move_u;
const dvar_t *cg_hintFadeTime;
const dvar_t *cg_subtitleMinTime;
const dvar_t *cg_tracerScaleDistRange;
const dvar_t *cg_paused;
const dvar_t *cg_cursorHints;
const dvar_t *cg_gun_ofs_f;
const dvar_t *vehHelicopterHeadSwayOnYaw;
const dvar_t *cg_laserLightBeginOffset;
const dvar_t *cg_tracerScrewDist;
const dvar_t *cg_tracerLength;
const dvar_t *cg_cinematicFullscreen;



const rectDef_s *__cdecl Window_GetRect(const windowDef_t *w)
{
    if (!w)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\../ui/ui_utils_api.h", 36, 0, "%s", "w");
    return &w->rect;
}

connstate_t __cdecl CL_GetLocalClientConnectionState(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../client/client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return clientUIActives[0].connectionState;
}

const char *cg_drawFpsNames[5] =
{ "Off", "Simple", "Extra", "Verbose", NULL };

const char *snd_drawInfoStrings[5] =
{ "None", "3D", "Stream", "2D", NULL };

const char *cg_drawMaterialNames[5] =
{ "Off", "CONTENTS_SOLID", "MASK_SHOT", "MASK_PLAYERSOLID", NULL };

const char *debugOverlayNames[4] =
{ "Off", "ViewmodelInfo", "FontTest", NULL };



const dvar_s *CG_RegisterDvars()
{
    const char *v0; // r5
    unsigned __int16 v1; // r4
    const char *v2; // r5
    unsigned __int16 v3; // r4
    const char *v4; // r5
    unsigned __int16 v5; // r4
    const char *v6; // r5
    unsigned __int16 v7; // r4
    const char *v8; // r5
    unsigned __int16 v9; // r4
    const char *v10; // r5
    unsigned __int16 v11; // r4
    const char *v12; // r5
    unsigned __int16 v13; // r4
    const char *v14; // r5
    unsigned __int16 v15; // r4
    const char *v16; // r5
    unsigned __int16 v17; // r4
    const char *v18; // r5
    unsigned __int16 v19; // r4
    const char *v20; // r5
    unsigned __int16 v21; // r4
    const char *v22; // r5
    unsigned __int16 v23; // r4
    const char *v24; // r5
    unsigned __int16 v25; // r4
    const char *v26; // r5
    unsigned __int16 v27; // r4
    const char *v28; // r5
    unsigned __int16 v29; // r4
    const char *v30; // r5
    unsigned __int16 v31; // r4
    const char *v32; // r5
    unsigned __int16 v33; // r4
    const char *v34; // r5
    unsigned __int16 v35; // r4
    const char *v36; // r5
    unsigned __int16 v37; // r4
    const char *v38; // r5
    unsigned __int16 v39; // r4
    const char *v40; // r5
    unsigned __int16 v41; // r4
    const char *v42; // r5
    unsigned __int16 v43; // r4
    const char *v44; // r5
    unsigned __int16 v45; // r4
    const char *v46; // r5
    unsigned __int16 v47; // r4
    const char *v48; // r5
    unsigned __int16 v49; // r4
    const char *v50; // r5
    unsigned __int16 v51; // r4
    const char *v52; // r5
    unsigned __int16 v53; // r4
    const char *v54; // r5
    unsigned __int16 v55; // r4
    const char *v56; // r5
    unsigned __int16 v57; // r4
    const char *v58; // r5
    unsigned __int16 v59; // r4
    const char *v60; // r5
    unsigned __int16 v61; // r4
    const char *v62; // r5
    unsigned __int16 v63; // r4
    const char *v64; // r5
    unsigned __int16 v65; // r4
    const char *v66; // r5
    unsigned __int16 v67; // r4
    const char *v68; // r5
    unsigned __int16 v69; // r4
    const char *v70; // r5
    unsigned __int16 v71; // r4
    const char *v72; // r5
    unsigned __int16 v73; // r4
    const char *v74; // r5
    unsigned __int16 v75; // r4
    const char *v76; // r5
    unsigned __int16 v77; // r4
    const char *v78; // r5
    unsigned __int16 v79; // r4
    const char *v80; // r5
    unsigned __int16 v81; // r4
    const char *v82; // r5
    unsigned __int16 v83; // r4
    const char *v84; // r5
    unsigned __int16 v85; // r4
    const char *v86; // r5
    unsigned __int16 v87; // r4
    const char *v88; // r5
    unsigned __int16 v89; // r4
    const char *v90; // r5
    unsigned __int16 v91; // r4
    const char *v92; // r5
    unsigned __int16 v93; // r4
    const char *v94; // r5
    unsigned __int16 v95; // r4
    const char *v96; // r5
    unsigned __int16 v97; // r4
    const char *v98; // r5
    unsigned __int16 v99; // r4
    const char *v100; // r5
    unsigned __int16 v101; // r4
    const char *v102; // r5
    unsigned __int16 v103; // r4
    const char *v104; // r5
    unsigned __int16 v105; // r4
    const char *v106; // r5
    unsigned __int16 v107; // r4
    const char *v108; // r5
    unsigned __int16 v109; // r4
    const char *v110; // r5
    unsigned __int16 v111; // r4
    const char *v112; // r5
    unsigned __int16 v113; // r4
    const char *v114; // r5
    unsigned __int16 v115; // r4
    const char *v116; // r5
    unsigned __int16 v117; // r4
    const char *v118; // r5
    unsigned __int16 v119; // r4
    const char *v120; // r5
    unsigned __int16 v121; // r4
    const char *v122; // r5
    unsigned __int16 v123; // r4
    const char *v124; // r5
    unsigned __int16 v125; // r4
    const char *v126; // r5
    unsigned __int16 v127; // r4
    const char *v128; // r5
    unsigned __int16 v129; // r4
    const char *v130; // r5
    unsigned __int16 v131; // r4
    const char *v132; // r5
    unsigned __int16 v133; // r4
    const char *v134; // r5
    unsigned __int16 v135; // r4
    const char *v136; // r5
    unsigned __int16 v137; // r4
    const char *v138; // r5
    unsigned __int16 v139; // r4
    const char *v140; // r5
    unsigned __int16 v141; // r4
    const char *v142; // r5
    unsigned __int16 v143; // r4
    const char *v144; // r5
    unsigned __int16 v145; // r4
    const char *v146; // r5
    unsigned __int16 v147; // r4
    const char *v148; // r5
    unsigned __int16 v149; // r4
    const char *v150; // r5
    unsigned __int16 v151; // r4
    const char *v152; // r5
    unsigned __int16 v153; // r4
    const char *v154; // r5
    unsigned __int16 v155; // r4
    const char *v156; // r5
    unsigned __int16 v157; // r4
    const char *v158; // r5
    unsigned __int16 v159; // r4
    const char *v160; // r5
    unsigned __int16 v161; // r4
    const char *v162; // r5
    unsigned __int16 v163; // r4
    const char *v164; // r5
    unsigned __int16 v165; // r4
    const char *v166; // r5
    unsigned __int16 v167; // r4
    const dvar_s *result; // r3

    cg_drawGun = Dvar_RegisterBool("cg_drawGun", 1, 0x80u, "Draw the view model");
    cg_cursorHints = Dvar_RegisterInt(
        "cg_cursorHints",
        3,
        0,
        4,
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
        0,
        0x7FFFFFFF,
        1u,
        "Time in milliseconds for the cursor hint to fade");
    cg_fov = Dvar_RegisterFloat("cg_fov", 65.0, 1.0, 160.0, v1, v0);
    cg_viewVehicleInfluence = Dvar_RegisterFloat("cg_viewVehicleInfluence", 1.0, 0.0, 1.0, v3, v2);
    cg_draw2D = Dvar_RegisterBool("cg_draw2D", 1, 0, "Draw 2D screen elements");
    cg_drawHUD = Dvar_RegisterBool("cg_drawHUD", 1, 0, "Draw HUD elements");
    cg_drawHealth = Dvar_RegisterBool("cg_drawHealth", 0, 0x80u, "Draw health bar");
    cg_drawBreathHint = Dvar_RegisterBool("cg_drawBreathHint", 1, 1u, "Draw a 'hold breath to steady' hint");
    cg_drawMantleHint = Dvar_RegisterBool("cg_drawMantleHint", 1, 1u, "Draw a 'press key to mantle' hint");
    replay_time = Dvar_RegisterBool("replay_time", 0, 1u, "Draw replay time");
    cg_drawFPS = Dvar_RegisterEnum("cg_drawFPS", cg_drawFpsNames, 1, 1u, "Draw frames per second");
    cg_drawFPSLabels = Dvar_RegisterBool("cg_drawFPSLabels", 1, 1u, "Draw FPS Info Labels");
    DvarLimits limits;
    limits.value.min = -200.0f;
    limits.value.max = 640.0f;
    //cg_debugInfoCornerOffset = Dvar_RegisterVec2("cg_debugInfoCornerOffset", 20.0, -20.0, -200.0, 640.0, v5, v4);
    cg_debugInfoCornerOffset = Dvar_RegisterVec2("cg_debugInfoCornerOffset", 20.0, -20.0, limits, v5, v4);
    cg_drawVersion = Dvar_RegisterBool("cg_drawVersion", 1, 0, "Draw the game version");
    cg_drawVersionX = Dvar_RegisterFloat("cg_drawVersionX", 50.0, 0.0, 512.0, v7, v6);
    cg_drawVersionY = Dvar_RegisterFloat("cg_drawVersionY", 18.0, 0.0, 512.0, v9, v8);
    snd_drawInfo = Dvar_RegisterEnum("snd_drawInfo", snd_drawInfoStrings, 0, 0, "Draw debugging information for sounds");
    cg_drawScriptUsage = Dvar_RegisterBool("cg_drawScriptUsage", 0, 0, "Draw debugging information for scripts");
    cg_drawMaterial = Dvar_RegisterEnum(
        "cg_drawMaterial",
        cg_drawMaterialNames,
        0,
        0x80u,
        "Draw debugging information for materials");
    cg_drawTurretCrosshair = Dvar_RegisterBool("cg_drawTurretCrosshair", 1, 1u, "Draw a cross hair when using a turret");
    cg_drawShellshock = Dvar_RegisterBool("cg_drawShellshock", 1, 0x80u, "Draw shellshock & flashbang screen effects.");
    cg_drawPlayerPosInFreeMove = Dvar_RegisterBool(
        "cg_drawPlayerPosInFreeMove",
        1,
        0x80u,
        "Draw a red box at the player's pos in noclip/ufo.");
    cg_drawPerformanceWarnings = Dvar_RegisterBool(
        "cg_drawPerformanceWarnings",
        1,
        0x80u,
        "Draw various debug overlays.  Only useful when cg_draw2D is off.");
    cg_hudStanceFlash = Dvar_RegisterColor("cg_hudStanceFlash", 1.0, 1.0, 1.0, 1.0, v11, v10);
    cg_hudStanceHintPrints = Dvar_RegisterBool(
        "cg_hudStanceHintPrints",
        0,
        1u,
        "Draw helpful text to say how to change stances");
    cg_hudDamageIconWidth = Dvar_RegisterFloat("cg_hudDamageIconWidth", 128.0, 0.0, 512.0, v13, v12);
    cg_hudDamageIconHeight = Dvar_RegisterFloat("cg_hudDamageIconHeight", 64.0, 0.0, 512.0, v15, v14);
    cg_hudDamageIconOffset = Dvar_RegisterFloat("cg_hudDamageIconOffset", 128.0, 0.0, 512.0, v17, v16);
    cg_hudDamageIconTime = Dvar_RegisterInt(
        "cg_hudDamageIconTime",
        2000,
        0,
        0x7FFFFFFF,
        1u,
        "The amount of time for the damage icon to stay on screen after damage is taken");
    cg_hudDamageIconInScope = Dvar_RegisterBool(
        "cg_hudDamageIconInScope",
        0,
        1u,
        "Draw damage icons when aiming down the sight of a scoped weapon");
    cg_hudGrenadeIconMaxRangeFrag = Dvar_RegisterFloat("cg_hudGrenadeIconMaxRangeFrag", 256.0, 0.0, 1000.0, v19, v18);
    cg_hudGrenadeIconMaxRangeFlash = Dvar_RegisterFloat("cg_hudGrenadeIconMaxRangeFlash", 500.0, 0.0, 2000.0, v21, v20);
    cg_hudGrenadeIconMaxHeight = Dvar_RegisterFloat("cg_hudGrenadeIconMaxHeight", 104.0, 0.0, 1000.0, v23, v22);
    cg_hudGrenadeIconInScope = Dvar_RegisterBool(
        "cg_hudGrenadeIconInScope",
        1,
        1u,
        "Show the grenade indicator when aiming down the sight of a scoped weapon");
    cg_hudGrenadeIconOffset = Dvar_RegisterFloat("cg_hudGrenadeIconOffset", 50.0, 0.0, 512.0, v25, v24);
    cg_hudGrenadeIconHeight = Dvar_RegisterFloat("cg_hudGrenadeIconHeight", 25.0, 0.0, 512.0, v27, v26);
    cg_hudGrenadeIconWidth = Dvar_RegisterFloat("cg_hudGrenadeIconWidth", 25.0, 0.0, 512.0, v29, v28);
    cg_hudGrenadeIconEnabledFlash = Dvar_RegisterBool(
        "cg_hudGrenadeIconEnabledFlash",
        0,
        1u,
        "Show the grenade indicator for flash grenades");
    cg_hudGrenadePointerHeight = Dvar_RegisterFloat("cg_hudGrenadePointerHeight", 12.0, 0.0, 512.0, v31, v30);
    cg_hudGrenadePointerWidth = Dvar_RegisterFloat("cg_hudGrenadePointerWidth", 25.0, 0.0, 512.0, v33, v32);
    limits.value.min = 0.0f;
    limits.value.max = 512.0f;
    //cg_hudGrenadePointerPivot = Dvar_RegisterVec2("cg_hudGrenadePointerPivot", 12.0, 27.0, 0.0, 512.0, v35, v34);
    cg_hudGrenadePointerPivot = Dvar_RegisterVec2("cg_hudGrenadePointerPivot", 12.0, 27.0, limits, v35, v34);
    cg_hudGrenadePointerPulseFreq = Dvar_RegisterFloat("cg_hudGrenadePointerPulseFreq", 1.7, 0.1, 50.0, v37, v36);
    cg_hudGrenadePointerPulseMax = Dvar_RegisterFloat("cg_hudGrenadePointerPulseMax", 1.85, 0.0, 3.0, v39, v38);
    cg_hudGrenadePointerPulseMin = Dvar_RegisterFloat("cg_hudGrenadePointerPulseMin", 0.30000001, -3.0, 1.0, v41, v40);
    cg_weaponCycleDelay = Dvar_RegisterInt(
        "cg_weaponCycleDelay",
        0,
        0,
        0x7FFFFFFF,
        1u,
        "The delay after cycling to a new weapon to prevent holding down the cycle weapon button from cycling too fast");
    cg_crosshairAlpha = Dvar_RegisterFloat("cg_crosshairAlpha", 1.0, 0.0, 1.0, v43, v42);
    cg_crosshairAlphaMin = Dvar_RegisterFloat("cg_crosshairAlphaMin", 0.5, 0.0, 1.0, v45, v44);
    cg_crosshairDynamic = Dvar_RegisterBool("cg_crosshairDynamic", 0, 0x81u, "Crosshair is Dynamic");
    cg_crosshairEnemyColor = Dvar_RegisterBool(
        "cg_crosshairEnemyColor",
        1,
        0x81u,
        "The crosshair color when over an enemy");
    cg_drawFriendlyFireCrosshair = Dvar_RegisterBool(
        "cg_drawFriendlyFireCrosshair",
        0,
        0x81u,
        "draw the friendly fire crosshair (friendly move)");
    cg_brass = Dvar_RegisterBool("cg_brass", 1, 1u, "Weapons eject brass");
    cg_gun_x = Dvar_RegisterFloat("cg_gun_x", 0.0, -FLT_MAX, FLT_MAX, v47, v46);
    cg_gun_y = Dvar_RegisterFloat("cg_gun_y", 0.0, -FLT_MAX, FLT_MAX, v49, v48);
    cg_gun_z = Dvar_RegisterFloat("cg_gun_z", 0.0, -FLT_MAX, FLT_MAX, v51, v50);
    cg_gun_move_f = Dvar_RegisterFloat("cg_gun_move_f", 0.0, -FLT_MAX, FLT_MAX, v53, v52);
    cg_gun_move_r = Dvar_RegisterFloat("cg_gun_move_r", 0.0, -FLT_MAX, FLT_MAX, v55, v54);
    cg_gun_move_u = Dvar_RegisterFloat("cg_gun_move_u", 0.0, -FLT_MAX, FLT_MAX, v57, v56);
    cg_gun_ofs_f = Dvar_RegisterFloat("cg_gun_ofs_f", 0.0, -FLT_MAX, FLT_MAX, v59, v58);
    cg_gun_ofs_r = Dvar_RegisterFloat("cg_gun_ofs_r", 0.0, -FLT_MAX, FLT_MAX, v61, v60);
    cg_gun_ofs_u = Dvar_RegisterFloat("cg_gun_ofs_u", 0.0, -FLT_MAX, FLT_MAX, v63, v62);
    cg_gun_rot_p = Dvar_RegisterFloat("cg_gun_rot_p", 0.0, -FLT_MAX, FLT_MAX, v65, v64);
    cg_gun_rot_y = Dvar_RegisterFloat("cg_gun_rot_y", 0.0, -FLT_MAX, FLT_MAX, v67, v66);
    cg_gun_rot_r = Dvar_RegisterFloat("cg_gun_rot_r", 0.0, -FLT_MAX, FLT_MAX, v69, v68);
    cg_gun_move_rate = Dvar_RegisterFloat("cg_gun_move_rate", 0.0, -FLT_MAX, FLT_MAX, v71, v70);
    cg_gun_move_minspeed = Dvar_RegisterFloat("cg_gun_move_minspeed", 0.0, -FLT_MAX, FLT_MAX, v73, v72);
    cg_gun_rot_rate = Dvar_RegisterFloat("cg_gun_rot_rate", 0.0, -FLT_MAX, FLT_MAX, v75, v74);
    cg_gun_rot_minspeed = Dvar_RegisterFloat("cg_gun_rot_minspeed", 0.0, -FLT_MAX, FLT_MAX, v77, v76);
    cg_centertime = Dvar_RegisterFloat("cg_centertime", 3.0, 0.0, FLT_MAX, v79, v78);
    cg_bobWeaponAmplitude = Dvar_RegisterFloat("cg_bobWeaponAmplitude", 0.16, 0.0, 1.0, v81, v80);
    cg_bobWeaponRollAmplitude = Dvar_RegisterFloat("cg_bobWeaponRollAmplitude", 1.5, 0.0, 90.0, v83, v82);
    cg_bobWeaponMax = Dvar_RegisterFloat("cg_bobWeaponMax", 10.0, 0.0, 36.0, v85, v84);
    cg_bobWeaponLag = Dvar_RegisterFloat("cg_bobWeaponLag", 0.25, -1.0, 1.0, v87, v86);
    cg_debugEvents = Dvar_RegisterBool("cg_debugevents", 0, 0x80u, "Output event debug information");
    cg_errorDecay = Dvar_RegisterFloat("cg_errordecay", 100.0, 0.0, FLT_MAX, v89, v88);
    cg_nopredict = Dvar_RegisterBool("cg_nopredict", 0, 1u, "Don't do client side prediction");
    cg_cinematicFullscreen = Dvar_RegisterBool("cg_cinematicFullscreen", 1, 0x1000u, "Draw ingame cinematics full screen");
    cg_showmiss = Dvar_RegisterInt("cg_showmiss", 0, 0, 2, 0, "Show prediction errors");
    cg_footsteps = Dvar_RegisterBool("cg_footsteps", 1, 0x1080u, "Play footstep sounds");
    cg_laserForceOn = Dvar_RegisterBool(
        "cg_laserForceOn",
        0,
        0x80u,
        "Force laser sights on in all possible places (for debug purposes).");
    cg_laserRange = Dvar_RegisterFloat("cg_laserRange", 1500.0, 1.0, FLT_MAX, v91, v90);
    cg_laserRangePlayer = Dvar_RegisterFloat("cg_laserRangePlayer", 1500.0, 1.0, FLT_MAX, v93, v92);
    cg_laserRadius = Dvar_RegisterFloat("cg_laserRadius", 0.80000001, 0.001, FLT_MAX, v95, v94);
    cg_laserLight = Dvar_RegisterBool(
        "cg_laserLight",
        1,
        0,
        "Whether to draw the light emitted from a laser (not the laser itself)");
    cg_laserLightBodyTweak = Dvar_RegisterFloat("cg_laserLightBodyTweak", 15.0, -FLT_MAX, FLT_MAX, v97, v96);
    cg_laserLightRadius = Dvar_RegisterFloat("cg_laserLightRadius", 3.0, 0.001, FLT_MAX, v99, v98);
    cg_laserLightBeginOffset = Dvar_RegisterFloat(
        "cg_laserLightBeginOffset",
        13.0,
        -FLT_MAX,
        FLT_MAX,
        v101,
        v100);
    cg_laserLightEndOffset = Dvar_RegisterFloat("cg_laserLightEndOffset", -3.0, -FLT_MAX, FLT_MAX, v103, v102);
    cg_laserEndOffset = Dvar_RegisterFloat("cg_laserEndOffset", 0.5, -FLT_MAX, FLT_MAX, v105, v104);
    cg_laserFlarePct = Dvar_RegisterFloat("cg_laserFlarePct", 0.2, 0.0, FLT_MAX, v107, v106);
    cg_marks_ents_player_only = Dvar_RegisterBool(
        "cg_marks_ents_player_only",
        0,
        1u,
        "Marks on entities from player's bullets only.");
    cg_tracerChance = Dvar_RegisterFloat("cg_tracerchance", 0.2, 0.0, 1.0, v109, v108);
    cg_tracerWidth = Dvar_RegisterFloat("cg_tracerwidth", 4.0, 0.0, FLT_MAX, v111, v110);
    cg_tracerSpeed = Dvar_RegisterFloat("cg_tracerSpeed", 7500.0, 0.0, FLT_MAX, v113, v112);
    cg_tracerLength = Dvar_RegisterFloat("cg_tracerlength", 160.0, 0.0, FLT_MAX, v115, v114);
    cg_tracerScale = Dvar_RegisterFloat("cg_tracerScale", 1.0, 1.0, FLT_MAX, v117, v116);
    cg_tracerScaleMinDist = Dvar_RegisterFloat("cg_tracerScaleMinDist", 5000.0, 0.0, FLT_MAX, v119, v118);
    cg_tracerScaleDistRange = Dvar_RegisterFloat("cg_tracerScaleDistRange", 25000.0, 0.0, FLT_MAX, v121, v120);
    cg_tracerScrewDist = Dvar_RegisterFloat("cg_tracerScrewDist", 100.0, 0.0, FLT_MAX, v123, v122);
    cg_tracerScrewRadius = Dvar_RegisterFloat("cg_tracerScrewRadius", 0.5, 0.0, FLT_MAX, v125, v124);
    cg_paused = Dvar_RegisterInt("cl_paused", 0, 0, 2, 0, "Pause the game");
    cg_drawpaused = Dvar_RegisterBool("cg_drawpaused", 1, 0, "Draw paused screen");
    cg_debug_overlay_viewport = Dvar_RegisterBool(
        "cg_debug_overlay_viewport",
        0,
        0x80u,
        "Remove the sniper overlay so you can check that the scissor window is correct.");
    cg_objectiveListWrapCountStandard = Dvar_RegisterInt(
        "cg_objectiveListWrapCountStandard",
        325,
        100,
        640,
        0,
        "The amount of on-screen length to wrap an objective in non wide-screen mode");
    cg_objectiveListWrapCountWidescreen = Dvar_RegisterInt(
        "cg_objectiveListWrapCountWidescreen",
        530,
        100,
        640,
        0,
        "The amount of on-screen length to wrap an objective in wide-screen mode");
    cg_dumpAnims = Dvar_RegisterInt("cg_dumpAnims", -1, -1, ENTITYNUM_NONE, 0x80u, "Output animation info for the given entity id");
    cg_developer = Dvar_RegisterInt("developer", 0, 0, 2, 0, "Turn on Development systems");
    cg_minicon = Dvar_RegisterBool("con_minicon", 0, 1u, "Display the mini console on screen");
    cg_subtitleMinTime = Dvar_RegisterFloat("cg_subtitleMinTime", 3.0, 0.0, FLT_MAX, v127, v126);
    cg_subtitleWidthStandard = Dvar_RegisterInt(
        "cg_subtitleWidthStandard",
        306,
        130,
        1664,
        1u,
        "The width of the subtitles on a non wide-screen");
    cg_subtitleWidthWidescreen = Dvar_RegisterInt(
        "cg_subtitleWidthWidescreen",
        468,
        130,
        1664,
        1u,
        "The width of the subtitle on a wide-screen");
    cg_gameMessageWidth = Dvar_RegisterInt(
        "cg_gameMessageWidth",
        500,
        130,
        1664,
        1u,
        "The maximum character width of the game messages");
    cg_gameBoldMessageWidth = Dvar_RegisterInt(
        "cg_gameBoldMessageWidth",
        390,
        130,
        1664,
        1u,
        "The maximum character width of the bold game messages");
    cg_blood = Dvar_RegisterBool("cg_blood", 1, 1u, "Show blood effects");
    cg_bloodLimit = Dvar_RegisterBool("cg_bloodLimit", 0, 1u, "Limit blood effects (to 'prevent excess blood stacking')");
    cg_bloodLimitMsec = Dvar_RegisterInt(
        "cg_bloodLimitMsec",
        330,
        1,
        2000,
        1u,
        "When limiting blood effects, number of milliseconds between effects.");
    cg_small_dev_string_fontscale = Dvar_RegisterFloat(
        "cg_small_dev_string_fontscale",
        1.5,
        0.0,
        FLT_MAX,
        v129,
        v128);
    cg_mapLocationSelectionCursorSpeed = Dvar_RegisterFloat(
        "cg_mapLocationSelectionCursorSpeed",
        0.60000002,
        0.001,
        1.0,
        v131,
        v130);
    turretScopeZoom = Dvar_RegisterFloat("turretScopeZoom", 20.0, 0.0099999998, 180.0, v133, v132);
    turretScopeZoomMin = Dvar_RegisterFloat("turretScopeZoomMin", 5.0, 0.0099999998, 180.0, v135, v134);
    turretScopeZoomMax = Dvar_RegisterFloat("turretScopeZoomMax", 20.0, 0.0099999998, 180.0, v137, v136);
    turretScopeZoomRate = Dvar_RegisterFloat("turretScopeZoomRate", 15.0, 0.0, FLT_MAX, v139, v138);
    cg_viewZSmoothingMin = Dvar_RegisterFloat("cg_viewZSmoothingMin", 1.0, 0.0, FLT_MAX, v141, v140);
    cg_viewZSmoothingMax = Dvar_RegisterFloat("cg_viewZSmoothingMax", 16.0, 0.0, FLT_MAX, v143, v142);
    cg_viewZSmoothingTime = Dvar_RegisterFloat("cg_viewZSmoothingTime", 0.1, 0.0, FLT_MAX, v145, v144);
    overrideNVGModelWithKnife = Dvar_RegisterBool(
        "overrideNVGModelWithKnife",
        0,
        0x1000u,
        "When true, nightvision animations will attach the weapDef's knife model instead of the n"
        "ight vision goggles.");
    cg_modPrvDrawAxis = Dvar_RegisterBool("cg_modPrvDrawAxis", 1, 1u, "Draw Axes in the model previewer");
    cg_modPrvMruModels = Dvar_RegisterString("cg_modPrvMruModels", "", 1u, "");
    cg_modPrvMruAnims = Dvar_RegisterString("cg_modPrvMruAnims", "", 1u, "");
    cg_r_forceLod = Dvar_FindVar("r_forceLod");
    if (!cg_r_forceLod)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 490, 0, "%s", "cg_r_forceLod");
    sv_znear = Dvar_RegisterFloat("sv_znear", 0.0, 0.0, 10000.0, v147, v146);
    DynEntCl_RegisterDvars();
    CG_OffhandRegisterDvars();
    CG_CompassRegisterDvars();
    CG_AmmoCounterRegisterDvars();
    CG_VehicleHudRegisterDvars();
    CG_RegisterVisionSetsDvars();
    CG_HudElemRegisterDvars();
    BG_RegisterDvars();
    cg_drawRumbleDebug = Dvar_RegisterBool("cg_drawrumbledebug", 0, 0, "Display rumble debug information");
    cg_invalidCmdHintDuration = Dvar_RegisterInt(
        "cg_invalidCmdHintDuration",
        1800,
        0,
        0x7FFFFFFF,
        1u,
        "Duration of an invalid command hint");
    cg_invalidCmdHintBlinkInterval = Dvar_RegisterInt(
        "cg_invalidCmdHintBlinkInterval",
        600,
        1,
        0x7FFFFFFF,
        1u,
        "Blink rate of an invalid command hint");
    vehHelicopterFreeLookReleaseSpeed = Dvar_RegisterFloat(
        "vehHelicopterFreeLookReleaseSpeed",
        8.0,
        0.0099999998,
        FLT_MAX,
        v149,
        v148);
    vehHelicopterHeadSwayOnYaw = Dvar_RegisterFloat(
        "vehHelicopterHeadSwayOnYaw",
        0.1,
        -FLT_MAX,
        FLT_MAX,
        v151,
        v150);
    vehHelicopterHeadSwayOnPitch = Dvar_RegisterFloat(
        "vehHelicopterHeadSwayOnPitch",
        0.2,
        -FLT_MAX,
        FLT_MAX,
        v153,
        v152);
    vehHelicopterHeadSwayOnRollHorz = Dvar_RegisterFloat(
        "vehHelicopterHeadSwayOnRollHorz",
        0.1,
        -FLT_MAX,
        FLT_MAX,
        v155,
        v154);
    vehHelicopterHeadSwayOnRollVert = Dvar_RegisterFloat(
        "vehHelicopterHeadSwayOnRollVert",
        2.0,
        -FLT_MAX,
        FLT_MAX,
        v157,
        v156);
    hud_showStance = Dvar_RegisterBool(
        "hud_showStance",
        1,
        0x1000u,
        "When true, allow player's stance indicator to draw.");
    hud_drawHUD = Dvar_RegisterBool("hud_drawHUD", 1, 0x1000u, "Draw HUD elements. Controlled from non-UI script");
    hud_missionFailed = Dvar_RegisterBool(
        "hud_missionFailed",
        0,
        0x1000u,
        "Intended to be set by script and referenced by hud.menu elements.");
    friendlyNameFontObjective = Dvar_RegisterBool(
        "friendlyNameFontObjective",
        1,
        0,
        "Use the objective font for friendly prints.");
    friendlyNameFontSize = Dvar_RegisterFloat("friendlyNameFontSize", 0.30000001, 0.0099999998, 100.0, v159, v158);
    limits.value.min = 0.0f;
    limits.value.max = 1.0f;
    friendlyNameFontColor = Dvar_RegisterVec4(
        "friendlyNameFontColor",
        0.89999998,
        1.0,
        0.89999998,
        0.69999999,
        limits,
        v161,
        v160);
    limits.value.min = 0.0f;
    limits.value.max = 1.0f;
    friendlyNameFontGlowColor = Dvar_RegisterVec4(
        "friendlyNameFontGlowColor",
        0.0,
        0.30000001,
        0.0,
        1.0,
        limits,
        v163,
        v162);
    limits.value.min = 0.0f;
    limits.value.max = 1.0f;
    hostileNameFontColor = Dvar_RegisterVec4(
        "hostileNameFontColor",
        1.0,
        0.89999998,
        0.89999998,
        0.69999999,
        limits,
        v165,
        v164);
    limits.value.min = 0.0f;
    limits.value.max = 1.0f;
    hostileNameFontGlowColor = Dvar_RegisterVec4(
        "hostileNameFontGlowColor",
        0.60000002,
        0.0,
        0.0,
        1.0,
        limits,
        v167,
        v166);
    result = Dvar_RegisterEnum("debugOverlay", debugOverlayNames, 0, 0, "Toggles the display of various debug info.");
    debugOverlay = result;
    return result;
}

void __cdecl TRACK_cg_main()
{
    track_static_alloc_internal(&cgDC, 3448, "cgDC", 34);
    track_static_alloc_internal(cgArray, 192072, "cgArray", 9);
    track_static_alloc_internal(cgsArray, 1112, "cgsArray", 9);
    track_static_alloc_internal(cg_entitiesArray, 809472, "cg_entitiesArray", 9);
    track_static_alloc_internal(cg_weaponsArray, 9216, "cg_weaponsArray", 9);
    track_static_alloc_internal(cg_entityOriginArray, 26112, "cg_entityOriginArray", 9);
}

void __cdecl CG_GetDObjOrientation(int localClientNum, int dobjHandle, float (*axis)[3], float *origin)
{
    centity_s *Entity; // r31

    if ((unsigned int)dobjHandle >= 0x900)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp",
            571,
            0,
            "%s\n\t(dobjHandle) = %i",
            "(dobjHandle >= 0 && dobjHandle < (((2176)) + 128))",
            dobjHandle);
    if (!axis)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 572, 0, "%s", "axis");
    if (!origin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 573, 0, "%s", "origin");
    if (dobjHandle >= 2176)
    {
        if (dobjHandle - 2176 >= 128)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp",
                582,
                0,
                "%s\n\t(dobjHandle) = %i",
                "(dobjHandle >= ((2176)) && dobjHandle - ((2176)) < 128)",
                dobjHandle);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        AxisCopy((const mat3x3&)cgArray[0].viewModelAxis, (mat3x3&)axis);
        *origin = cgArray[0].viewModelAxis[3][0];
        origin[1] = cgArray[0].viewModelAxis[3][1];
        origin[2] = cgArray[0].viewModelAxis[3][2];
    }
    else
    {
        Entity = CG_GetEntity(localClientNum, dobjHandle);
        AnglesToAxis(Entity->pose.angles, axis);
        *origin = Entity->pose.origin[0];
        origin[1] = Entity->pose.origin[1];
        origin[2] = Entity->pose.origin[2];
    }
}

void __cdecl CG_CopyEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3])
{
    centity_s *Entity; // r3

    if (!origin_out)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 595, 0, "%s", "origin_out");
    if (!axis_out)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 596, 0, "%s", "axis_out");
    Entity = CG_GetEntity(localClientNum, entIndex);
    *origin_out = Entity->pose.origin[0];
    origin_out[1] = Entity->pose.origin[1];
    origin_out[2] = Entity->pose.origin[2];
    AnglesToAxis(Entity->pose.angles, axis_out);
}

void __cdecl CG_GetSoundEntityOrientation(SndEntHandle sndEnt, float *origin_out, float (*axis_out)[3])
{
    CG_CopyEntityOrientation(0, sndEnt.field.entIndex, origin_out, axis_out);
}

unsigned int __cdecl CG_SoundEntityUseEq(SndEntHandle sndEnt)
{
    unsigned int entityIndex = sndEnt.field.entIndex;

    if (entityIndex == 0xFFFF)
        return 1;

    bcassert(entityIndex, MAX_GENTITIES);
    return ((unsigned int)~cg_entitiesArray[0][entityIndex].currentState.eFlags >> 21) & 1;
}

const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return &cgArray[0].predictedPlayerState;
}

int __cdecl CG_IsValidRemoteInputState(int localClientNum)
{
    int result; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (g_godModeRemoteInputValid)
        return 1;
    if (cgArray[0].predictedPlayerState.pm_type == 3)
        return 1;
    if (cgArray[0].predictedPlayerState.pm_type == 2)
        return 1;
    result = 0;
    if (cgArray[0].predictedPlayerState.pm_type == 4)
        return 1;
    return result;
}

void __cdecl CG_SetTime(int serverTime)
{
    cgArray[0].time = serverTime;
    cgArray[0].oldTime = serverTime;
}

void __cdecl CG_SetServerCommandSequence(int reliableSent)
{
    cgArray[0].serverCommandSequence = reliableSent;
}

void __cdecl CG_GameMessage(const char *msg, int flags)
{
    CL_ConsolePrint(0, 2, msg, 0, cg_gameMessageWidth->current.integer, flags);
}

void __cdecl CG_BoldGameMessage(const char *msg, int flags)
{
    CL_ConsolePrint(0, 3, msg, 0, cg_gameBoldMessageWidth->current.integer, flags);
}

void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, snd_alias_list_t **sound)
{
    snd_alias_list_t *v4; // r3
    snd_alias_list_t **v5; // r11
    int v6; // ctr
    snd_alias_list_t *SoundAlias; // r27
    int i; // r30
    const char *v9; // r3
    snd_alias_list_t *SoundAliasNoErrors; // r3
    char v11[304]; // [sp+50h] [-130h] BYREF

    if (!pszType)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 700, 0, "%s", "pszType");
    if (*pszType)
    {
        sprintf(v11, "%s_default", pszType);
        SoundAlias = Com_FindSoundAlias(v11);
        for (i = 0; i < 29; ++i)
        {
            v9 = Com_SurfaceTypeToName(i);
            sprintf(v11, "%s_%s", pszType, v9);
            SoundAliasNoErrors = Com_FindSoundAliasNoErrors(v11);
            *sound = SoundAliasNoErrors;
            if (!SoundAliasNoErrors)
                *sound = SoundAlias;
            ++sound;
        }
    }
    else
    {
        Com_Printf(9, "WARNING: no alias prefix defined, using default\n");
        v4 = Com_FindSoundAliasNoErrors("collision_default");
        v5 = sound;
        v6 = 29;
        do
        {
            *v5++ = v4;
            --v6;
        } while (v6);
    }
}

void __cdecl CG_AddAudioPhysicsClass(PhysPreset *physPreset, char (*classes)[64], int *nclasses)
{
    int v6; // r28
    const char *v7; // r29
    const char *sndAliasPrefix; // r3
    int v9; // r10

    if (!physPreset)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 729, 0, "%s", "physPreset");
    if (!physPreset->sndAliasPrefix)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 730, 0, "%s", "physPreset->sndAliasPrefix");
    if (!nclasses)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 731, 0, "%s", "nclasses");
    if (!classes)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 732, 0, "%s", "classes");
    v6 = 0;
    if (*nclasses <= 0)
    {
    LABEL_13:
        sndAliasPrefix = physPreset->sndAliasPrefix;
        v9 = *nclasses;
        physPreset->type = *nclasses;
        CG_RegisterSurfaceTypeSounds(sndAliasPrefix, cgMedia.physCollisionSound[v9]);
        I_strncpyz(&(*classes)[64 * *nclasses], physPreset->sndAliasPrefix, 64);
        ++*nclasses;
    }
    else
    {
        v7 = (const char *)classes;
        while (I_stricmp(physPreset->sndAliasPrefix, v7))
        {
            ++v6;
            v7 += 64;
            if (v6 >= *nclasses)
                goto LABEL_13;
        }
        physPreset->type = v6;
    }
}

void CG_RegisterPhysicsSounds_FastFile()
{
    int AllXAssetOfType; // r3
    XAssetHeader *v1; // r30
    int v2; // r31
    int v3[4]; // [sp+50h] [-D80h] BYREF
    XAssetHeader v4[854]; // [sp+60h] [-D70h] BYREF

    v3[0] = 0;
    AllXAssetOfType = DB_GetAllXAssetOfType(ASSET_TYPE_PHYSPRESET, v4, 50);
    if (AllXAssetOfType > 0)
    {
        v1 = v4;
        v2 = AllXAssetOfType;
        do
        {
            CG_AddAudioPhysicsClass(v1->physPreset, (char (*)[64]) & v4[52], v3);
            --v2;
            ++v1;
        } while (v2);
    }
}

// attributes: thunk
void CG_RegisterPhysicsSounds()
{
    CG_RegisterPhysicsSounds_FastFile();
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
    CG_RegisterPhysicsSounds_FastFile();
    cgMedia.sprintingEquipmentSound = Com_FindSoundAlias("gear_rattle_sprint");
    cgMedia.sprintingEquipmentSoundPlayer = Com_FindSoundAlias("gear_rattle_plr_sprint");
    cgMedia.runningEquipmentSound = Com_FindSoundAlias("gear_rattle_run");
    cgMedia.runningEquipmentSoundPlayer = Com_FindSoundAlias("gear_rattle_plr_run");
    cgMedia.walkingEquipmentSound = Com_FindSoundAlias("gear_rattle_walk");
    cgMedia.walkingEquipmentSoundPlayer = Com_FindSoundAlias("gear_rattle_plr_walk");
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

void __cdecl CG_LoadWorld(int savegame)
{
    ProfLoad_Begin("Load world");
    LoadWorld(cgsArray[0].mapname, savegame);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_ParseSunLight(0);
    SCR_UpdateLoadScreen();
}

void __cdecl RegisterNightVisionAssets(int localClientNum)
{
    if (*CL_GetConfigString(localClientNum, 0x47Fu) == 49)
    {
        ProfLoad_Begin("Register night vision assets");
        cgMedia.nightVisionOverlay = Material_RegisterHandle("nightvision_overlay_goggles", 7);
        cgMedia.hudIconNVG = Material_RegisterHandle("hud_icon_nvg", 7);
        cgMedia.nightVisionGoggles = R_RegisterModel("viewmodel_NVG");
        ProfLoad_End();
    }
}

void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname)
{
    int i; // r29
    const char *ConfigString; // r28
    int v6; // r28
    const FxEffectDef **v7; // r29
    const char *v8; // r3
    const FxEffectDef *v9; // r3
    int j; // r28
    const char *v11; // r3
    const char *v12; // r29
    shellshock_parms_t *ShellshockParms; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_DrawInformation(localClientNum);
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Register code assets");
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
    cgMedia.objectiveMaterials[1] = Material_RegisterHandle("objective_ring", 7);
    cgMedia.damageMaterial = Material_RegisterHandle("hit_direction", 7);
    cgMedia.mantleHint = Material_RegisterHandle("hint_mantle", 7);
    cgMedia.checkbox_active = Material_RegisterHandle("hud_checkbox_active", 7);
    cgMedia.checkbox_current = Material_RegisterHandle("hud_checkbox_current", 7);
    cgMedia.checkbox_done = Material_RegisterHandle("hud_checkbox_done", 7);
    cgMedia.checkbox_fail = Material_RegisterHandle("hud_checkbox_fail", 7);
    cgMedia.compassping_friendlyfiring = Material_RegisterHandle("compassping_friendlyfiring", 7);
    cgMedia.compassping_friendlyyelling = Material_RegisterHandle("compassping_friendlyyelling", 7);
    cgMedia.compassping_enemyfiring = Material_RegisterHandle("compassping_enemyfiring", 7);
    cgMedia.compassping_enemyyelling = Material_RegisterHandle("compassping_enemyyelling", 7);
    cgMedia.compassping_grenade = Material_RegisterHandle("compassping_grenade", 7);
    cgMedia.compassping_explosion = Material_RegisterHandle("compassping_explosion", 7);
    cgMedia.grenadeIconFrag = Material_RegisterHandle("hud_grenadeicon", 7);
    cgMedia.grenadeIconFlash = Material_RegisterHandle("hud_flashbangicon", 7);
    cgMedia.grenadeIconThrowBack = Material_RegisterHandle("hud_grenadethrowback", 7);
    cgMedia.grenadePointer = Material_RegisterHandle("hud_grenadepointer", 7);
    cgMedia.offscreenObjectivePointer = Material_RegisterHandle("hud_offscreenobjectivepointer", 7);
    cgMedia.vehCenterCircle = Material_RegisterHandle("veh_centercircle", 7);
    cgMedia.vehMovingCircle = Material_RegisterHandle("veh_movingcircle", 7);
    cgMedia.vehHudLine = Material_RegisterHandle("veh_hud_line", 7);
    cgMedia.vehBouncingDiamondReticle = Material_RegisterHandle("veh_hud_diamond", 7);
    cgMedia.textDecodeCharacters = Material_RegisterHandle("decode_characters", 7);
    cgMedia.textDecodeCharactersGlow = Material_RegisterHandle("decode_characters_glow", 7);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Register items");
    CG_RegisterItems(localClientNum);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Register server models");
    for (i = 1; i < 512; ++i)
    {
        ConfigString = CL_GetConfigString(localClientNum, i + 1155);
        if (!*ConfigString)
            break;
        CG_DrawInformation(localClientNum);
        SCR_UpdateLoadScreen();
        R_RegisterModel(ConfigString);
    }
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Register known effects");
    v6 = 1;
    v7 = &cgsArray[0].fxs[1];
    do
    {
        v8 = CL_GetConfigString(localClientNum, v6 + 2179);
        if (!*v8)
            break;
        v9 = FX_Register(v8);
        *v7 = v9;
        if (!v9)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 1026, 0, "%s", "cgs->fxs[i]");
        ++v7;
        ++v6;
    } while ((int)v7 < (int)&cgsArray[0].holdBreathParams);
    ProfLoad_End();
    ProfLoad_Begin("Register shellshocks");
    for (j = 1; j < 16; ++j)
    {
        v11 = CL_GetConfigString(localClientNum, j + 2535);
        v12 = v11;
        if (!*v11)
            break;
        if (!BG_LoadShellShockDvars(v11))
            Com_Error(ERR_DROP, "couldn't register shellchock '%s' -- see console", v12);
        ShellshockParms = BG_GetShellshockParms(j);
        BG_SetShellShockParmsFromDvars(ShellshockParms);
    }
    if (!BG_LoadShellShockDvars("hold_breath"))
        Com_Error(ERR_DROP, "Couldn't find shock file [hold_breath.shock]\n");
    BG_SetShellShockParmsFromDvars(&cgsArray[0].holdBreathParams);
    ProfLoad_End();
    ProfLoad_Begin("Register rumbles");
    //CG_RegisterRumbles(localClientNum); // KISAKTODO
    ProfLoad_End();
    ProfLoad_Begin("Register impact effects");
    cgMedia.fx = CG_RegisterImpactEffects(mapname);
    if (!cgMedia.fx)
        Com_Error(ERR_DROP, "Error reading CSV files in the fx directory to identify impact effects");
    cgMedia.fxNoBloodFleshHit = FX_Register("impacts/flesh_hit_noblood");
    cgMedia.fxKnifeBlood = FX_Register("impacts/flesh_hit_knife");
    cgMedia.fxKnifeNoBlood = FX_Register("impacts/flesh_hit_knife_noblood");
    ProfLoad_End();
    if (*CL_GetConfigString(localClientNum, 0x47Fu) == 49)
    {
        ProfLoad_Begin("Register night vision assets");
        cgMedia.nightVisionOverlay = Material_RegisterHandle("nightvision_overlay_goggles", 7);
        cgMedia.hudIconNVG = Material_RegisterHandle("hud_icon_nvg", 7);
        cgMedia.nightVisionGoggles = R_RegisterModel("viewmodel_NVG");
        ProfLoad_End();
    }
    SCR_UpdateLoadScreen();
}

void __cdecl CG_StartAmbient(int localClientNum)
{
    const char *ConfigString; // r31
    const char *v3; // r29
    const char *v4; // r31
    const char *v5; // r3
    int time; // r31
    int v7; // r31
    const char *v8; // r11
    const snd_alias_t *v10; // r3

    ConfigString = CL_GetConfigString(localClientNum, 0x45Au);
    v3 = Info_ValueForKey(ConfigString, "n");
    v4 = Info_ValueForKey(ConfigString, "t");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v5 = v4;
    time = cgArray[0].time;
    v7 = atol(v5) - time;
    if (v7 < 0)
        v7 = 0;
    v8 = v3;
    while (*(unsigned __int8 *)v8++)
        ;
    if (v8 - v3 == 1)
    {
        SND_StopAmbient(localClientNum, v7);
    }
    else
    {
        v10 = CL_PickSoundAlias(v3);
        SND_PlayAmbientAlias(localClientNum, v10, v7, SASYS_CGAME);
    }
}

void __cdecl CG_StopSoundAlias(const int localClientNum, SndEntHandle entitynum, snd_alias_list_t *aliasList)
{
    if (aliasList)
    {
        if (aliasList->aliasName)
            SND_StopSoundAliasOnEnt(entitynum, aliasList->aliasName);
    }
}

void __cdecl CG_StopSoundsOnEnt(const int localClientNum, SndEntHandle entitynum)
{
    SND_StopSoundsOnEnt(entitynum);
}

void __cdecl CG_StopSoundAliasByName(int localClientNum, SndEntHandle entityNum, const char *aliasName)
{
    SND_StopSoundAliasOnEnt(entityNum, aliasName);
}

void __cdecl CG_StopClientSoundAliasByName(int localClientNum, const char *aliasName)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    SND_StopSoundAliasOnEnt((SndEntHandle)cgArray[0].nextSnap->ps.clientNum, aliasName);
}

static void __cdecl CG_SubtitlePrint(int msec, const snd_alias_t *alias)
{
    const dvar_s *v5; // r10
    const dvar_s *v6; // r11
    int integer; // r31
    long double v8; // fp2
    int v9; // r5

    if (!alias)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 1194, 0, "%s", "alias");
    if (!cg_subtitleWidthStandard)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 1195, 0, "%s", "cg_subtitleWidthStandard");
    if (!cg_subtitleWidthWidescreen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 1196, 0, "%s", "cg_subtitleWidthWidescreen");
    v5 = cg_subtitleMinTime;
    if (!cg_subtitleMinTime)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 1197, 0, "%s", "cg_subtitleMinTime");
        v5 = cg_subtitleMinTime;
    }
    if (msec && alias->subtitle)
    {
        if (cgsArray[0].viewAspect <= 1.3333334)
            v6 = cg_subtitleWidthStandard;
        else
            v6 = cg_subtitleWidthWidescreen;
        integer = v6->current.integer;
        v8 = floor((float)((float)(v5->current.value * (float)1000.0) + (float)0.5));
        v9 = (int)(float)*(double *)&v8;
        if (v9 < msec)
            v9 = msec;
        CL_SubtitlePrint(0, alias->subtitle, v9, integer);
    }
}

// attributes: thunk
void __cdecl CG_SubtitleSndLengthNotify(int msec, const snd_alias_t *lengthNotifyData)
{
    CG_SubtitlePrint(msec, lengthNotifyData);
}

void __cdecl CG_ScriptNotifySndLengthNotify(int msec, void *lengthNotifyData)
{
    const char *v2; // r3

    v2 = va("sl %i %i", lengthNotifyData, msec);
    CL_AddReliableCommand(0, v2);
}

void __cdecl CG_AddFXSoundAlias(int localClientNum, const float *origin, snd_alias_list_t *aliasList)
{
    snd_alias_t *v4; // r3
    snd_alias_t *v5; // r31

    v4 = Com_PickSoundAliasFromList(aliasList);
    v5 = v4;
    if (v4)
    {
        Snd_AssertAliasValid(v4);
        SND_AddPlayFXSoundAlias(v5, (SndEntHandle)ENTITYNUM_WORLD, origin);
    }
}

int __cdecl CG_PlaySoundAlias(int localClientNum, SndEntHandle entitynum, const float *origin, snd_alias_list_t *aliasList)
{
    const snd_alias_t *v6; // r3
    snd_alias_t *v7; // r31
    int v9; // r30

    v6 = Com_PickSoundAliasFromList(aliasList);
    v7 = (snd_alias_t *)v6;
    if (!v6)
        return -1;
    v9 = SND_PlaySoundAlias(v6, entitynum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(v9, v7, SndLengthNotify_Subtitle);
    return v9;
}

int __cdecl CG_PlaySoundAliasByName(int localClientNum, SndEntHandle entitynum, const float *origin, const char *aliasname)
{
    const snd_alias_t *v6; // r3
    snd_alias_t *v7; // r31
    int v9; // r30

    v6 = CL_PickSoundAlias(aliasname);
    v7 = (snd_alias_t *)v6;
    if (!v6)
        return -1;
    v9 = SND_PlaySoundAlias(v6, entitynum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(v9, v7, SndLengthNotify_Subtitle);
    return v9;
}

int __cdecl CG_PlaySoundAliasAsMasterByName(int localClientNum, SndEntHandle entitynum, const float *origin, const char *aliasname)
{
    const snd_alias_t *v6; // r3
    snd_alias_t *v7; // r31
    int v9; // r30

    v6 = CL_PickSoundAlias(aliasname);
    v7 = (snd_alias_t *)v6;
    if (!v6)
        return -1;
    v9 = SND_PlaySoundAliasAsMaster(v6, entitynum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(v9, v7, SndLengthNotify_Subtitle);
    return v9;
}

void __cdecl CG_LoadHudMenu(int localClientNum)
{
    MenuList *Menus; // r3
    menuDef_t *v3; // r31

    Menus = UI_LoadMenus((char*)"ui/hud.txt", 7);
    UI_AddMenuList(&cgDC, Menus);
    v3 = Menus_FindByName(&cgDC, "Compass");
    if (v3)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                917,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgsArray[0].compassWidth = v3->window.rect.w;
        cgsArray[0].compassHeight = v3->window.rect.h;
    }
}

void __cdecl CG_InitViewDimensions(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgsArray[0].viewX = 0;
    CL_GetScreenDimensions(&cgsArray[0].viewWidth,&cgsArray[0].viewHeight, &cgsArray[0].viewAspect);
    if (cgsArray[0].viewWidth <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp",
            1326,
            1,
            "%s\n\t(cgs->viewWidth) = %i",
            "(cgs->viewWidth > 0)",
            cgsArray[0].viewWidth);
    if (cgsArray[0].viewHeight <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp",
            1327,
            1,
            "%s\n\t(cgs->viewHeight) = %i",
            "(cgs->viewHeight > 0)",
            cgsArray[0].viewHeight);
    if (cgsArray[0].viewAspect <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp",
            1328,
            1,
            "%s\n\t(cgs->viewAspect) = %g",
            "(cgs->viewAspect > 0)",
            cgsArray[0].viewAspect);
}

const char *__cdecl CG_GetTeamName(team_t team)
{
    const char *result; // r3
    const char *v2; // r3

    switch (team)
    {
    case TEAM_FREE:
        result = "TEAM_FREE";
        break;
    case TEAM_AXIS:
        result = "TEAM_AXIS";
        break;
    case TEAM_ALLIES:
        result = "TEAM_ALLIES";
        break;
    case TEAM_NEUTRAL:
        result = "TEAM_NEUTRAL";
        break;
    case TEAM_DEAD:
        result = "TEAM_DEAD";
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("Unhandled team index %i!", team);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp", 1347, 0, v2);
        }
        result = "";
        break;
    }
    return result;
}

const char *__cdecl CG_GetPlayerTeamName(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../client/client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_ACTIVE)
        return "TEAM_ALLIES";
    else
        return "TEAM_FREE";
}

const char *__cdecl CG_GetPlayerOpposingTeamName(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../client/client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_ACTIVE)
        return "TEAM_AXIS";
    else
        return "TEAM_FREE";
}

bool __cdecl CG_IsPlayerDead(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../client/client.h",
            569,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState < CA_ACTIVE)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return cgArray[0].nextSnap->ps.stats[0] == 0;
}

int __cdecl CG_GetPlayerClipAmmoCount(int localClientNum)
{
    playerState_s *p_ps; // r31

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    p_ps = &cgArray[0].nextSnap->ps;
    return p_ps->ammoclip[BG_ClipForWeapon(cgArray[0].nextSnap->ps.weapon)];
}

void __cdecl CG_InitDof(GfxDepthOfField *dof)
{
    dof->nearStart = 0.0;
    dof->nearEnd = 0.0;
    dof->farBlur = 0.0;
    dof->farStart = 5000.0;
    dof->farEnd = 5000.0;
    dof->nearBlur = 6.0;
}

void __cdecl CG_Init(int localClientNum, int savegame)
{
    const char *ConfigString; // r11
    const char *v5; // r10
    int v6; // r8
    int i; // r31
    const char *String; // r3
    char v9[64]; // [sp+50h] [-470h] BYREF
    char v10[1072]; // [sp+90h] [-430h] BYREF

    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    memset(&cgsArray[localClientNum], 0, sizeof(cgs_t));
    memset(&cgDC, 0, sizeof(cgDC));
    memset(cg_weaponsArray[localClientNum], 0, sizeof(weaponInfo_s[128]));
    memset(&cgArray[0].viewModelPose, 0, sizeof(cgArray[0].viewModelPose));
    cgArray[0].viewModelPose.eType = 17;
    cgArray[0].localClientNum = localClientNum;
    cgArray[0].refdef.dof.nearStart = 0.0;
    cgArray[0].refdef.dof.nearEnd = 0.0;
    cgArray[0].refdef.dof.farStart = 5000.0;
    cgArray[0].refdef.dof.farEnd = 5000.0;
    cgArray[0].refdef.dof.nearBlur = 6.0;
    cgArray[0].refdef.dof.farBlur = 0.0;
    CG_RegisterDvars();
    Ragdoll_Init();
    Phys_Init();
    CG_DrawInformation(localClientNum);
    SCR_UpdateLoadScreen();
    cgMedia.whiteMaterial = Material_RegisterHandle("white", 7);
    cgMedia.friendlyFireMaterial = Material_RegisterHandle("hudfriendlyfire", 7);
    cgMedia.smallDevFont = CL_RegisterFont("fonts/smalldevfont", 1);
    cgMedia.bigDevFont = CL_RegisterFont("fonts/bigdevfont", 1);
    cgMedia.compassFovMaterial = Material_RegisterHandle("compass_fov", 7);
    cgMedia.hudDpadArrow = Material_RegisterHandle("hud_dpad_arrow", 7);
    cgMedia.ammoCounterBullet = Material_RegisterHandle("ammo_counter_bullet", 7);
    cgMedia.ammoCounterBeltBullet = Material_RegisterHandle("ammo_counter_beltbullet", 7);
    cgMedia.ammoCounterRifleBullet = Material_RegisterHandle("ammo_counter_riflebullet", 7);
    cgMedia.ammoCounterRocket = Material_RegisterHandle("ammo_counter_rocket", 7);
    cgMedia.ammoCounterShotgunShell = Material_RegisterHandle("ammo_counter_shotgunshell", 7);
    cgMedia.nightVisionOverlay = 0;
    cgMedia.hudIconNVG = 0;
    cgMedia.nightVisionGoggles = 0;
    Material_RegisterHandle("code_warning_soundcpu", 7);
    Material_RegisterHandle("code_warning_snapshotents", 7);
    Material_RegisterHandle("code_warning_maxeffects", 7);
    Material_RegisterHandle("code_warning_models", 7);
    Material_RegisterHandle("code_warning_file", 7);
    Material_RegisterHandle("code_warning_fps", 7);
    Material_RegisterHandle("code_warning_serverfps", 7);
    CG_InitConsoleCommands();
    CG_InitViewDimensions(localClientNum);
    ConfigString = CL_GetConfigString(localClientNum, 2u);
    v5 = "cod-sp";
    do
    {
        v6 = *(unsigned __int8 *)ConfigString - *(unsigned __int8 *)v5;
        if (!*ConfigString)
            break;
        ++ConfigString;
        ++v5;
    } while (!v6);
    if (v6)
        Com_Error(ERR_DROP, "Client/Server game mismatch");
    ProfLoad_Begin("Parse server info");
    CG_ParseServerInfo(localClientNum);
    CG_SetupWeaponDef(localClientNum);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    Menu_Setup(&cgDC);
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Load world");
    LoadWorld(cgsArray[0].mapname, savegame);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_ParseSunLight(0);
    SCR_UpdateLoadScreen();
    SCR_UpdateLoadScreen();
    SCR_UpdateLoadScreen();
    if (I_strnicmp(cgsArray[0].mapname, "maps/", 5))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_main.cpp",
            1528,
            0,
            "%s",
            "!I_strnicmp( cgs->mapname, \"maps/\", 5 )");
    Com_StripExtension(&cgsArray[0].mapname[5], v9);
    ProfLoad_Begin("Init effects system");
    FX_InitSystem(localClientNum);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_RegisterGraphics(localClientNum, v9);
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Load hud menus");
    CG_LoadHudMenu(localClientNum);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Precache script menus");
    for (i = 2551; i < 2583; ++i)
        CG_PrecacheScriptMenu(localClientNum, i);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Register server materials");
    CG_RegisterServerMaterials(localClientNum);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Optimize all xmodels");
    CL_FinishLoadingModels();
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    ProfLoad_Begin("Map init");
    CG_MapInit(0);
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_AntiBurnInHUD_RegisterDvars();
    CL_SetADS(localClientNum, 0);
    AimAssist_Init(localClientNum);
    CG_ModelPreviewerCreateDevGui(localClientNum);
    //CG_InitDevguiRumbleGraph(localClientNum); // KISAKTODO
    I_strncpyz(v10, Dvar_GetString("profile"), 1024);
    Dvar_SetFromStringByName("profile", (char*)"");
    Dvar_SetFromStringByName("profile", v10);
    CG_InitVehicleReticle(localClientNum);
}

void __cdecl CG_FreeWeapons(int localClientNum)
{
    unsigned int v2; // r26
    XAnimTree_s **p_tree; // r30
    int v4; // r3

    v2 = 1;
    if (BG_GetNumWeapons() > 1)
    {
        p_tree = &cg_weaponsArray[0][1].tree;
        do
        {
            v4 = CG_WeaponDObjHandle(v2);
            Com_SafeClientDObjFree(v4, localClientNum);
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                    924,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            if (*p_tree)
            {
                XAnimFreeTree(*p_tree, 0);
                *p_tree = 0;
            }
            ++v2;
            p_tree += 18;
        } while (v2 < BG_GetNumWeapons());
    }
    memset(cg_weaponsArray[localClientNum], 0, sizeof(weaponInfo_s[128]));
}

void __cdecl CG_Shutdown(int localClientNum)
{
    int v2; // r3
    int i; // r30
    centity_s *Entity; // r31
    int ragdollHandle; // r3
    dxBody *physObjId; // r4

    R_TrackStatistics(0);
    SND_FadeAllSounds(1.0, v2);
    //CG_StopAllRumbles(localClientNum); // KISAKTODO
    for (i = 0; i < 2176; ++i)
    {
        Entity = CG_GetEntity(localClientNum, i);
        ragdollHandle = Entity->pose.ragdollHandle;
        if (ragdollHandle && ragdollHandle != -1)
        {
            Ragdoll_Remove(ragdollHandle);
            Entity->pose.ragdollHandle = 0;
        }
        physObjId = (dxBody *)Entity->pose.physObjId;
        if (physObjId && physObjId != (dxBody *)-1)
        {
            Phys_ObjDestroy(PHYS_WORLD_FX, physObjId);
            Entity->pose.physObjId = 0;
        }
    }
    Ragdoll_Shutdown();
    CG_FreeWeapons(localClientNum);
    Com_FreeWeaponInfoMemory(2);
    CG_ModelPreviewerDestroyDevGui();
    FX_KillAllEffects(localClientNum);
    FX_ShutdownSystem(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    Phys_Shutdown();
}

void *__cdecl Hunk_AllocXAnimClient(int size)
{
    return Hunk_AllocLowAlign(size, 4, "Hunk_AllocXAnimClient", 11);
}

int __cdecl CG_PlayClientSoundAlias(int localClientNum, snd_alias_list_t *aliasList)
{
    float *origin; // r29
    int clientNum; // r30
    const snd_alias_t *v5; // r3
    snd_alias_t *v6; // r31
    int v8; // r30

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    origin = cgArray[0].nextSnap->ps.origin;
    clientNum = cgArray[0].nextSnap->ps.clientNum;
    v5 = Com_PickSoundAliasFromList(aliasList);
    v6 = (snd_alias_t *)v5;
    if (!v5)
        return -1;
    v8 = SND_PlaySoundAlias(v5, (SndEntHandle)clientNum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(v8, v6, SndLengthNotify_Subtitle);
    return v8;
}

int __cdecl CG_PlayClientSoundAliasByName(int localClientNum, const char *aliasname)
{
    float *origin; // r29
    int clientNum; // r30
    const snd_alias_t *v5; // r3
    snd_alias_t *v6; // r31
    int v8; // r30

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    origin = cgArray[0].nextSnap->ps.origin;
    clientNum = cgArray[0].nextSnap->ps.clientNum;
    v5 = CL_PickSoundAlias(aliasname);
    v6 = (snd_alias_t *)v5;
    if (!v5)
        return -1;
    v8 = SND_PlaySoundAlias(v5, (SndEntHandle)clientNum, origin, 0, SASYS_CGAME);
    SND_AddLengthNotify(v8, v6, SndLengthNotify_Subtitle);
    return v8;
}

int __cdecl CG_PlayEntitySoundAlias(int localClientNum, SndEntHandle entitynum, snd_alias_list_t *aliasList)
{
    entityState_s *p_nextState; // r29
    const snd_alias_t *v6; // r3
    snd_alias_t *v7; // r31
    int v9; // r30

    p_nextState = &CG_GetEntity(localClientNum, entitynum.handle)->nextState;
    v6 = Com_PickSoundAliasFromList(aliasList);
    v7 = (snd_alias_t *)v6;
    if (!v6)
        return -1;
    v9 = SND_PlaySoundAlias(v6, entitynum, p_nextState->lerp.pos.trBase, 0, SASYS_CGAME);
    SND_AddLengthNotify(v9, v7, SndLengthNotify_Subtitle);
    return v9;
}

int __cdecl CG_PlayEntitySoundAliasByName(int localClientNum, SndEntHandle entitynum, const char *aliasname)
{
    entityState_s *p_nextState; // r29
    const snd_alias_t *v6; // r3
    snd_alias_t *v7; // r31
    int v9; // r30

    p_nextState = &CG_GetEntity(localClientNum, entitynum.handle)->nextState;
    v6 = CL_PickSoundAlias(aliasname);
    v7 = (snd_alias_t *)v6;
    if (!v6)
        return -1;
    v9 = SND_PlaySoundAlias(v6, entitynum, p_nextState->lerp.pos.trBase, 0, SASYS_CGAME);
    SND_AddLengthNotify(v9, v7, SndLengthNotify_Subtitle);
    return v9;
}

