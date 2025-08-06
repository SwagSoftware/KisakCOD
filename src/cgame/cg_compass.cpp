#include "cg_local.h"
#include "cg_public.h"

#include <cgame_mp/cg_local_mp.h>
#include <client/client.h>
#include <client_mp/client_mp.h>
#include <database/database.h>

const dvar_t *compassObjectiveArrowWidth;
const dvar_t *compassObjectiveTextScale;
const dvar_t *compassMinRange;
const dvar_t *compassTickertapeStretch;
const dvar_t *compassRadarPingFadeTime;
const dvar_t *compassFriendlyHeight;
const dvar_t *compassObjectiveArrowRotateDist;
const dvar_t *cg_hudMapPlayerWidth;
const dvar_t *compassEnemyFootstepMaxRange;
const dvar_t *compassObjectiveNearbyDist;
const dvar_t *compassRadarUpdateTime;
const dvar_t *compassSize;
const dvar_t *compassObjectiveMaxHeight;
const dvar_t *compassObjectiveMinDistRange;
const dvar_t *compassObjectiveArrowHeight;
const dvar_t *cg_hudMapFriendlyHeight;
const dvar_t *compassObjectiveIconWidth;
const dvar_t *compassObjectiveRingTime;
const dvar_t *compassPlayerHeight;
const dvar_t *compassSoundPingFadeTime;
const dvar_t *cg_hudMapRadarLineThickness;
const dvar_t *compass   ;
const dvar_t *compassDebug;
const dvar_t *compassObjectiveMinAlpha;
const dvar_t *compassEnemyFootstepEnabled;
const dvar_t *compassClampIcons;
const dvar_t *compassObjectiveRingSize;
const dvar_t *cg_hudMapFriendlyWidth;
const dvar_t *compassRotation;
const dvar_t *compassMaxRange;
const dvar_t *compassObjectiveDrawLines;
const dvar_t *compassEnemyFootstepMaxZ;
const dvar_t *compassObjectiveArrowOffset;
const dvar_t *compassMinRadius;
const dvar_t *compassFriendlyWidth;
const dvar_t *compassObjectiveIconHeight;
const dvar_t *compassObjectiveTextHeight;
const dvar_t *compassObjectiveDetailDist;
const dvar_t *compassObjectiveMaxRange;
const dvar_t *compassPlayerWidth;
const dvar_t *compassObjectiveWidth;
const dvar_t *compassEnemyFootstepMinSpeed;
const dvar_t *compassObjectiveHeight;
const dvar_t *compassCoords;
const dvar_t *compassRadarLineThickness;
const dvar_t *compassObjectiveMinHeight;
const dvar_t *cg_hudMapPlayerHeight;
const dvar_t *compassECoordCutoff;
const dvar_t *cg_hudMapBorderWidth;
const dvar_t *compassObjectiveNumRings;

#ifdef KISAK_SP
const dvar_t *compassIconTankWidth;
const dvar_t *compassIconTankHeight;
const dvar_t *compassIconOtherVehWidth;
const dvar_t *compassIconOtherVehHeight;
#endif

void __cdecl CG_CompassRegisterDvars()
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
    DvarLimits mins; // [esp+Ch] [ebp-10h]
    DvarLimits mint; // [esp+Ch] [ebp-10h]
    DvarLimits minu; // [esp+Ch] [ebp-10h]
    DvarLimits minv; // [esp+Ch] [ebp-10h]
    DvarLimits minw; // [esp+Ch] [ebp-10h]
    DvarLimits minx; // [esp+Ch] [ebp-10h]
    DvarLimits miny; // [esp+Ch] [ebp-10h]
    DvarLimits minz; // [esp+Ch] [ebp-10h]
    DvarLimits minba; // [esp+Ch] [ebp-10h]
    DvarLimits minbb; // [esp+Ch] [ebp-10h]
    DvarLimits minbc; // [esp+Ch] [ebp-10h]
    DvarLimits minbd; // [esp+Ch] [ebp-10h]
    DvarLimits minbe; // [esp+Ch] [ebp-10h]
    DvarLimits minbf; // [esp+Ch] [ebp-10h]
    DvarLimits minbg; // [esp+Ch] [ebp-10h]
    DvarLimits minbh; // [esp+Ch] [ebp-10h]
    DvarLimits minbi; // [esp+Ch] [ebp-10h]
    DvarLimits minbj; // [esp+Ch] [ebp-10h]
    DvarLimits minbk; // [esp+Ch] [ebp-10h]
    DvarLimits minbl; // [esp+Ch] [ebp-10h]
    DvarLimits minbm; // [esp+Ch] [ebp-10h]
    DvarLimits minbn; // [esp+Ch] [ebp-10h]
    DvarLimits minbo; // [esp+Ch] [ebp-10h]

    compass = Dvar_RegisterBool("compass", 1, DVAR_SAVED, "Display Compass");
    min.value.max = FLT_MAX;
    min.value.min = 0.0f;
    compassSize = Dvar_RegisterFloat("compassSize", 1.0f, min, DVAR_ARCHIVE, "Scale the compass");
    mina.value.max = FLT_MAX;
    mina.value.min = 0.000099999997f;
    compassMaxRange = Dvar_RegisterFloat(
        "compassMaxRange",
        2500.0f,
        mina,
        DVAR_ARCHIVE | DVAR_CHEAT | DVAR_SAVED,
        "The maximum range from the player in world space that objects will be shown on the compass");
    minb.value.max = FLT_MAX;
    minb.value.min = 0.000099999997f;
    compassMinRange = Dvar_RegisterFloat(
        "compassMinRange",
        0.000099999997f,
        minb,
        DVAR_ARCHIVE,
        "The minimum range from the player in world space that objects will appear on the compass");
    minc.value.max = FLT_MAX;
    minc.value.min = 0.000099999997f;
    compassMinRadius = Dvar_RegisterFloat(
        "compassMinRadius",
        0.000099999997f,
        minc,
        DVAR_ARCHIVE,
        "The minimum radius from the center of the compass that objects will appear.");
    mind.value.max = 10.0f;
    mind.value.min = 0.0f;
    compassSoundPingFadeTime = Dvar_RegisterFloat(
        "compassSoundPingFadeTime",
        2.0f,
        mind,
        DVAR_CHEAT | DVAR_ARCHIVE,
        "The time in seconds for the sound overlay on the compass to fade");
    compassClampIcons = Dvar_RegisterBool(
        "compassClampIcons",
        1,
        DVAR_CHEAT | DVAR_ARCHIVE,
        "If true, friendlies and enemy pings clamp to the edge of the radar.  If false, they disappear off the edge.");
    mine.value.max = FLT_MAX;
    mine.value.min = 0.0f;
    compassFriendlyWidth = Dvar_RegisterFloat(
        "compassFriendlyWidth",
        18.75f,
        mine,
        DVAR_ARCHIVE,
        "The size of the friendly icon on the compass");
    minf.value.max = FLT_MAX;
    minf.value.min = 0.0f;
    compassFriendlyHeight = Dvar_RegisterFloat(
        "compassFriendlyHeight",
        18.75f,
        minf,
        DVAR_ARCHIVE,
        "The size of the friendly icon on the compass");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0f;
    compassPlayerWidth = Dvar_RegisterFloat(
        "compassPlayerWidth",
        18.75f,
        ming,
        DVAR_ARCHIVE,
        "The size of the player's icon on the compass");
    minh.value.max = FLT_MAX;
    minh.value.min = 0.0f;
    compassPlayerHeight = Dvar_RegisterFloat(
        "compassPlayerHeight",
        18.75f,
        minh,
        DVAR_ARCHIVE,
        "The size of the player's icon on the compass");
    mini.value.max = FLT_MAX;
    mini.value.min = 0.0f;
    compassCoords = Dvar_RegisterVec3(
        "compassCoords",
        740.0f,
        3590.0f,
        400.0f,
        mini,
        DVAR_ARCHIVE,
        "x = North-South coord base value, \n"
        "y = East-West coord base value, \n"
        "z = scale (game units per coord unit)");
    minj.value.max = FLT_MAX;
    minj.value.min = 0.0f;
    compassECoordCutoff = Dvar_RegisterFloat(
        "compassECoordCutoff",
        37.0f,
        minj,
        DVAR_ARCHIVE,
        "Left cutoff for the scrolling east-west coords");
    compassRotation = Dvar_RegisterBool("compassRotation", 1, DVAR_ARCHIVE, "Style of compass");
    mink.value.max = 1.0f;
    mink.value.min = 0.0099999998f;
    compassTickertapeStretch = Dvar_RegisterFloat(
        "compassTickertapeStretch",
        0.5f,
        mink,
        DVAR_ARCHIVE,
        "How far the tickertape should stretch from its center.");
    minl.value.max = 60.0f;
    minl.value.min = 0.0099999998f;
    compassRadarPingFadeTime = Dvar_RegisterFloat(
        "compassRadarPingFadeTime",
        4.0f,
        minl,
        DVAR_CHEAT | DVAR_ARCHIVE,
        "How long an enemy is visible on the compass after it is detected by radar");
    minm.value.max = 60.0f;
    minm.value.min = 0.0099999998f;
    compassRadarUpdateTime = Dvar_RegisterFloat("compassRadarUpdateTime", 4.0f, minm, 0x81u, "Time between radar updates");
    minn.value.max = 10.0f;
    minn.value.min = 0.0099999998f;
    compassRadarLineThickness = Dvar_RegisterFloat(
        "compassRadarLineThickness",
        0.40000001f,
        minn,
        DVAR_CHEAT | DVAR_ARCHIVE,
        "Thickness, relative to the compass size, of the radar texture that sweeps across the map");
    mino.value.max = FLT_MAX;
    mino.value.min = 0.0f;
    compassObjectiveWidth = Dvar_RegisterFloat(
        "compassObjectiveWidth",
        20.0f,
        mino,
        DVAR_ARCHIVE,
        "The size of the objective on the compass");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.0f;
    compassObjectiveHeight = Dvar_RegisterFloat(
        "compassObjectiveHeight",
        20.0f,
        minp,
        DVAR_ARCHIVE,
        "The size of the objective on the compass");
    minq.value.max = FLT_MAX;
    minq.value.min = 0.0f;
    compassObjectiveArrowWidth = Dvar_RegisterFloat(
        "compassObjectiveArrowWidth",
        20.0f,
        minq,
        DVAR_ARCHIVE,
        "The size of the objective arrow on the compass");
    minr.value.max = FLT_MAX;
    minr.value.min = 0.0f;
    compassObjectiveArrowHeight = Dvar_RegisterFloat(
        "compassObjectiveArrowHeight",
        20.0f,
        minr,
        DVAR_ARCHIVE,
        "The size of the objective arrow on the compass");
    mins.value.max = FLT_MAX;
    mins.value.min = 0.0f;
    compassObjectiveArrowOffset = Dvar_RegisterFloat(
        "compassObjectiveArrowOffset",
        2.0f,
        mins,
        DVAR_ARCHIVE,
        "The offset of the objective arrow inward from the edge of the compass map");
    mint.value.max = FLT_MAX;
    mint.value.min = 0.0f;
    compassObjectiveArrowRotateDist = Dvar_RegisterFloat(
        "compassObjectiveArrowRotateDist",
        5.0,
        mint,
        DVAR_ARCHIVE,
        "Distance from the corner of the compass map at which the objective arrow rotates to 45 degrees");
    minu.value.max = FLT_MAX;
    minu.value.min = 0.0f;
    compassObjectiveMaxRange = Dvar_RegisterFloat(
        "compassObjectiveMaxRange",
        2048.0f,
        minu,
        DVAR_ARCHIVE,
        "The maximum range at which an objective is visible on the compass");
    minv.value.max = 1.0f;
    minv.value.min = 0.0f;
    compassObjectiveMinAlpha = Dvar_RegisterFloat(
        "compassObjectiveMinAlpha",
        1.0f,
        minv,
        DVAR_ARCHIVE,
        "The minimum alpha for an objective at the edge of the compass");
    compassObjectiveNumRings = Dvar_RegisterInt(
        "compassObjectiveNumRings",
        10,
        (DvarLimits)0x1400000000LL,
        DVAR_ARCHIVE,
        "The number of rings when a new objective appears");
    compassObjectiveRingTime = Dvar_RegisterInt(
        "compassObjectiveRingTime",
        10000,
        (DvarLimits)0x7FFFFFFF00000000LL,
        DVAR_ARCHIVE,
        "The amount of time between each ring when an objective appears");
    minw.value.max = FLT_MAX;
    minw.value.min = 0.0f;
    compassObjectiveRingSize = Dvar_RegisterFloat(
        "compassObjectiveRingSize",
        80.0f,
        minw,
        DVAR_ARCHIVE,
        "The maximum objective ring sige when a new objective appears on the compass");
    minx.value.max = FLT_MAX;
    minx.value.min = 0.0000099999997f;
    compassObjectiveTextScale = Dvar_RegisterFloat(
        "compassObjectiveTextScale",
        0.30000001f,
        minx,
        DVAR_ARCHIVE,
        "Scale to apply to hud objectives");
    miny.value.max = FLT_MAX;
    miny.value.min = 0.0000099999997f;
    compassObjectiveTextHeight = Dvar_RegisterFloat("compassObjectiveTextHeight", 18.0f, miny, 1u, "Objective text height");
    compassObjectiveDrawLines = Dvar_RegisterBool(
        "compassObjectiveDrawLines",
        1,
        DVAR_ARCHIVE,
        "Draw horizontal and vertical lines to the active target, if it is within the minimap boundries");
    minz.value.max = FLT_MAX;
    minz.value.min = 0.0f;
    compassObjectiveIconWidth = Dvar_RegisterFloat(
        "compassObjectiveIconWidth",
        16.0f,
        minz,
        DVAR_ARCHIVE,
        "The size of the objective on the full map");
    minba.value.max = FLT_MAX;
    minba.value.min = 0.0f;
    compassObjectiveIconHeight = Dvar_RegisterFloat(
        "compassObjectiveIconHeight",
        16.0f,
        minba,
        DVAR_ARCHIVE,
        "The size of the objective on the full map");
    minbb.value.max = FLT_MAX;
    minbb.value.min = 0.0099999998f;
    compassObjectiveNearbyDist = Dvar_RegisterFloat(
        "compassObjectiveNearbyDist",
        4.0f,
        minbb,
        DVAR_SAVED,
        "When an objective is closer than this distance (in meters), an \"Objective Nearby\" typ"
        "e of indicator is shown.");
    minbc.value.max = FLT_MAX;
    minbc.value.min = 0.0099999998f;
    compassObjectiveMinDistRange = Dvar_RegisterFloat(
        "compassObjectiveMinDistRange",
        1.0f,
        minbc,
        DVAR_SAVED,
        "The distance that objective transition effects play over, centered on compassObjectiveNearbyDist.");
    minbd.value.max = FLT_MAX;
    minbd.value.min = 0.0099999998f;
    compassObjectiveDetailDist = Dvar_RegisterFloat(
        "compassObjectiveDetailDist",
        10.0f,
        minbd,
        DVAR_SAVED,
        "When an objective is closer than this distance (in meters), the icon will not be drawn "
        "on the tickertape.");
    minbe.value.max = 0.0f;
    minbe.value.min = -FLT_MAX;
    compassObjectiveMinHeight = Dvar_RegisterFloat(
        "compassObjectiveMinHeight",
        -70.0f,
        minbe,
        DVAR_SAVED,
        "The minimum height that an objective is considered to be on this level");
    minbf.value.max = FLT_MAX;
    minbf.value.min = 0.0f;
    compassObjectiveMaxHeight = Dvar_RegisterFloat(
        "compassObjectiveMaxHeight",
        70.0f,
        minbf,
        DVAR_SAVED,
        "The maximum height that an objective is considered to be on this level");
    minbg.value.max = FLT_MAX;
    minbg.value.min = 0.0f;
    compassEnemyFootstepMaxRange = Dvar_RegisterFloat(
        "compassEnemyFootstepMaxRange",
        500.0f,
        minbg,
        DVAR_CHEAT,
        "The maximum distance at which an enemy may appear on the compass due to 'footsteps'");
    minbh.value.max = FLT_MAX;
    minbh.value.min = 0.0f;
    compassEnemyFootstepMaxZ = Dvar_RegisterFloat(
        "compassEnemyFootstepMaxZ",
        100.0f,
        minbh,
        DVAR_CHEAT,
        "The maximum vertical distance enemy may be from the player and appear on the compass due to 'footsteps'");
    minbi.value.max = FLT_MAX;
    minbi.value.min = 0.0f;
    compassEnemyFootstepMinSpeed = Dvar_RegisterFloat(
        "compassEnemyFootstepMinSpeed",
        140.0f,
        minbi,
        DVAR_CHEAT,
        "The minimum speed an enemy must be moving to appear on the compass due to 'footsteps'");
    compassEnemyFootstepEnabled = Dvar_RegisterBool(
        "compassEnemyFootstepEnabled",
        0,
        DVAR_CHEAT,
        "Enables enemies showing on the compass because of moving rapidly nearby.");
#ifdef KISAK_SP
    compassIconTankWidth = Dvar_RegisterFloat("compassIconTankWidth", 35.0, 0.0, FLT_MAX, 0, 0);
    compassIconTankHeight = Dvar_RegisterFloat("compassIconTankHeight", 35.0, 0.0, FLT_MAX, 0, 0);
    compassIconOtherVehWidth = Dvar_RegisterFloat("compassIconOtherVehWidth", 40.0, 0.0, FLT_MAX, 0, 0);
    compassIconOtherVehHeight = Dvar_RegisterFloat("compassIconOtherVehHeight", 40.0, 0.0, FLT_MAX, 0, 0);
#endif
    compassDebug = Dvar_RegisterBool("compassDebug", 0, 1u, "Compass Debugging Mode");
    minbj.value.max = 10.0f;
    minbj.value.min = 0.0099999998f;
    cg_hudMapRadarLineThickness = Dvar_RegisterFloat(
        "cg_hudMapRadarLineThickness",
        0.15000001f,
        minbj,
        DVAR_CHEAT | DVAR_ARCHIVE,
        "Thickness, relative to the map width, of the radar texture that sweeps across the full screen map");
    minbk.value.max = FLT_MAX;
    minbk.value.min = 0.0f;
    cg_hudMapFriendlyWidth = Dvar_RegisterFloat(
        "cg_hudMapFriendlyWidth",
        15.0f,
        minbk,
        DVAR_ARCHIVE,
        "The size of the friendly icon on the full map");
    minbl.value.max = FLT_MAX;
    minbl.value.min = 0.0f;
    cg_hudMapFriendlyHeight = Dvar_RegisterFloat(
        "cg_hudMapFriendlyHeight",
        15.0f,
        minbl,
        DVAR_ARCHIVE,
        "The size of the friendly icon on the full map");
    minbm.value.max = FLT_MAX;
    minbm.value.min = 0.0f;
    cg_hudMapPlayerWidth = Dvar_RegisterFloat(
        "cg_hudMapPlayerWidth",
        20.0f,
        minbm,
        DVAR_ARCHIVE,
        "The size of the player's icon on the full map");
    minbn.value.max = FLT_MAX;
    minbn.value.min = 0.0f;
    cg_hudMapPlayerHeight = Dvar_RegisterFloat(
        "cg_hudMapPlayerHeight",
        20.0f,
        minbn,
        DVAR_ARCHIVE,
        "The size of the player's icon on the full map");
    minbo.value.max = FLT_MAX;
    minbo.value.min = 0.0f;
    cg_hudMapBorderWidth = Dvar_RegisterFloat(
        "cg_hudMapBorderWidth",
        2.0f,
        minbo,
        DVAR_ARCHIVE,
        "The size of the full map's border, filled by the CG_PLAYER_FULLMAP_BORDER ownerdraw");
}

bool __cdecl CG_IsSelectingLocation(int32_t localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.locationSelectionInfo != 0;
}

bool __cdecl CG_WorldPosToCompass(
    CompassType compassType,
    const cg_s *cgameGlob,
    const rectDef_s *mapRect,
    const float *north,
    const float *playerWorldPos,
    const float *in,
    float *out,
    float *outClipped)
{
    float v9; // [esp+8h] [ebp-30h]
    float v10; // [esp+Ch] [ebp-2Ch]
    float v11; // [esp+10h] [ebp-28h]
    float v12; // [esp+14h] [ebp-24h]
    bool clipped; // [esp+1Fh] [ebp-19h]
    float posDelta; // [esp+20h] [ebp-18h]
    float posDeltaa; // [esp+20h] [ebp-18h]
    float posDeltab; // [esp+20h] [ebp-18h]
    float posDelta_4; // [esp+24h] [ebp-14h]
    float posDelta_4a; // [esp+24h] [ebp-14h]
    float posDelta_4b; // [esp+24h] [ebp-14h]
    float pixPerInch; // [esp+28h] [ebp-10h]
    float outTemp; // [esp+2Ch] [ebp-Ch]
    float outTempa; // [esp+2Ch] [ebp-Ch]
    float outTempb; // [esp+2Ch] [ebp-Ch]
    float outTemp_4; // [esp+30h] [ebp-8h]
    float outTemp_4a; // [esp+30h] [ebp-8h]
    float outTemp_4b; // [esp+30h] [ebp-8h]
    float scale; // [esp+34h] [ebp-4h]
    float scalea; // [esp+34h] [ebp-4h]
    float scaleb; // [esp+34h] [ebp-4h]
    float scalec; // [esp+34h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 228, 0, "%s", "cgameGlob");
    if (!mapRect)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 229, 0, "%s", "mapRect");
    if (!playerWorldPos)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 230, 0, "%s", "playerWorldPos");
    if (!in)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 231, 0, "%s", "in");
    if (compassType)
    {
        if (compassType != COMPASS_TYPE_FULL)
            MyAssertHandler(".\\cgame\\cg_compass.cpp", 249, 0, "%s", "compassType == COMPASS_TYPE_FULL");
        posDeltab = *in - cgameGlob->compassMapUpperLeft[0];
        posDelta_4b = in[1] - cgameGlob->compassMapUpperLeft[1];
        outTempa = cgameGlob->compassNorth[1] * posDeltab - cgameGlob->compassNorth[0] * posDelta_4b;
        outTemp_4a = -cgameGlob->compassNorth[1] * posDelta_4b - cgameGlob->compassNorth[0] * posDeltab;
        outTempb = outTempa / cgameGlob->compassMapWorldSize[0] - 0.5;
        outTemp_4b = outTemp_4a / cgameGlob->compassMapWorldSize[1] - 0.5;
        outTemp = outTempb * mapRect->w;
        outTemp_4 = outTemp_4b * mapRect->h;
    }
    else
    {
        if (!north)
            MyAssertHandler(".\\cgame\\cg_compass.cpp", 235, 0, "%s", "north");
        if (compassMaxRange->current.value < 0.0)
            MyAssertHandler(
                ".\\cgame\\cg_compass.cpp",
                237,
                0,
                "%s\n\t(compassMaxRange->current.value) = %g",
                "(compassMaxRange->current.value >= 0.0f)",
                compassMaxRange->current.value);
        pixPerInch = mapRect->h / compassMaxRange->current.value;
        posDelta = *in - *playerWorldPos;
        posDelta_4 = in[1] - playerWorldPos[1];
        posDeltaa = pixPerInch * posDelta;
        posDelta_4a = pixPerInch * posDelta_4;
        outTemp = north[1] * posDeltaa - *north * posDelta_4a;
        outTemp_4 = -north[1] * posDelta_4a - *north * posDeltaa;
    }
    clipped = 0;
    if (outClipped && mapRect->w >= 0.0 && mapRect->h >= 0.0)
    {
        *outClipped = outTemp;
        outClipped[1] = outTemp_4;
        v12 = mapRect->w * 0.5;
        if (*outClipped <= (double)v12)
        {
            v11 = -mapRect->w * 0.5;
            if (*outClipped < (double)v11)
            {
                scalea = -(mapRect->w * 0.5) / *outClipped;
                *outClipped = scalea * *outClipped;
                outClipped[1] = scalea * outClipped[1];
                clipped = 1;
            }
        }
        else
        {
            scale = mapRect->w * 0.5 / *outClipped;
            *outClipped = scale * *outClipped;
            outClipped[1] = scale * outClipped[1];
            clipped = 1;
        }
        v10 = mapRect->h * 0.5;
        if (outClipped[1] <= (double)v10)
        {
            v9 = -mapRect->h * 0.5;
            if (outClipped[1] < (double)v9)
            {
                scalec = -(mapRect->h * 0.5) / outClipped[1];
                *outClipped = scalec * *outClipped;
                outClipped[1] = scalec * outClipped[1];
                clipped = 1;
            }
        }
        else
        {
            scaleb = mapRect->h * 0.5 / outClipped[1];
            *outClipped = scaleb * *outClipped;
            outClipped[1] = scaleb * outClipped[1];
            clipped = 1;
        }
    }
    if (out)
    {
        *out = outTemp;
        out[1] = outTemp_4;
    }
    return clipped;
}

void __cdecl CG_CompassCalcDimensions(
    CompassType compassType,
    const cg_s *cgameGlob,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *x,
    float *y,
    float *w,
    float *h)
{
    float mapAspectRatio; // [esp+0h] [ebp-1Ch]
    float basex; // [esp+4h] [ebp-18h]
    float fraction; // [esp+8h] [ebp-14h]
    float fractiona; // [esp+8h] [ebp-14h]
    float border; // [esp+Ch] [ebp-10h]
    float rectAspectRatio; // [esp+10h] [ebp-Ch]
    float basey; // [esp+14h] [ebp-8h]
    float center; // [esp+18h] [ebp-4h]
    float centera; // [esp+18h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 303, 0, "%s", "cgameGlob");
    if (!parentRect)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 304, 0, "%s", "parentRect");
    if (!rect)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 305, 0, "%s", "rect");
    if (!x)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 306, 0, "%s", "x");
    if (!y)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 307, 0, "%s", "y");
    if (!w)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 308, 0, "%s", "w");
    if (!h)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 309, 0, "%s", "h");
    if (compassType)
    {
        if (rect->w <= 0.0 || rect->h <= 0.0)
            Com_Error(ERR_FATAL, "Compass ownerdraw had width or height of 0");
        if (rect->w == 0.0)
            MyAssertHandler(".\\cgame\\cg_compass.cpp", 340, 0, "%s", "rect->w");
        if (rect->h == 0.0)
            MyAssertHandler(".\\cgame\\cg_compass.cpp", 341, 0, "%s", "rect->h");
        if (cgameGlob->compassMapWorldSize[0] == 0.0)
            MyAssertHandler(".\\cgame\\cg_compass.cpp", 342, 0, "%s", "cgameGlob->compassMapWorldSize[0]");
        if (cgameGlob->compassMapWorldSize[1] == 0.0)
            MyAssertHandler(".\\cgame\\cg_compass.cpp", 343, 0, "%s", "cgameGlob->compassMapWorldSize[1]");
        rectAspectRatio = rect->w / rect->h;
        mapAspectRatio = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
        basex = parentRect->x + rect->x;
        basey = parentRect->y + rect->y;
        if (rectAspectRatio >= (double)mapAspectRatio)
        {
            centera = rect->w * 0.5 + basex;
            fractiona = mapAspectRatio / rectAspectRatio * rect->w;
            *x = centera - fractiona * 0.5;
            *y = basey;
            *w = fractiona;
            *h = rect->h;
        }
        else
        {
            center = rect->h * 0.5 + basey;
            fraction = rectAspectRatio / mapAspectRatio * rect->h;
            *x = basex;
            *y = center - fraction * 0.5;
            *w = rect->w;
            *h = fraction;
        }
        border = cg_hudMapBorderWidth->current.value;
        if (border > *w * 0.25)
            border = *w * 0.25;
        if (border > *h * 0.25)
            border = *h * 0.25;
        *x = *x + border;
        *y = *y + border;
        *w = *w - border * 2.0;
        *h = *h - border * 2.0;
    }
    else
    {
        *x = rect->x;
        *y = rect->y;
        *w = rect->w * compassSize->current.value;
        *h = rect->h * compassSize->current.value;
        KISAK_NULLSUB();
        if (*w <= 0.0 || *h <= 0.0)
            Com_Error(ERR_FATAL, "Compass ownerdraw had width or height of 0");
    }
}

double __cdecl CG_FadeCompass(int32_t localClientNum, int32_t displayStartTime, CompassType compassType)
{
    float v4; // [esp+4h] [ebp-10h]

    if (compassType)
        return 1.0;

    return CG_FadeHudMenu(localClientNum, hud_fade_compass, displayStartTime, (int)(hud_fade_compass->current.value * 1000.0f));
}

void __cdecl CG_CompassDrawPlayerBack(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    float x; // [esp+34h] [ebp-10h] BYREF
    float y; // [esp+38h] [ebp-Ch] BYREF
    float h; // [esp+3Ch] [ebp-8h] BYREF
    float w; // [esp+40h] [ebp-4h] BYREF
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (color[3] != 0.0)
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            material);
    }
}
void __cdecl CG_CompassDrawPlayerNorthCoord(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *const color,
    int32_t style)
{
    float v8; // [esp+24h] [ebp-5Ch]
    float v9; // [esp+28h] [ebp-58h]
    float v10; // [esp+2Ch] [ebp-54h]
    float v11; // [esp+30h] [ebp-50h]
    float v12; // [esp+38h] [ebp-48h]
    float v13; // [esp+3Ch] [ebp-44h]
    int32_t v14; // [esp+40h] [ebp-40h]
    int32_t v15; // [esp+44h] [ebp-3Ch]
    float textW; // [esp+50h] [ebp-30h]
    float textWa; // [esp+50h] [ebp-30h]
    int32_t integerPortion; // [esp+54h] [ebp-2Ch]
    float coord; // [esp+5Ch] [ebp-24h]
    float coorda; // [esp+5Ch] [ebp-24h]
    float SMALL_FRAC; // [esp+60h] [ebp-20h]
    float scale; // [esp+64h] [ebp-1Ch]
    float x; // [esp+68h] [ebp-18h] BYREF
    float y; // [esp+6Ch] [ebp-14h] BYREF
    float h; // [esp+70h] [ebp-10h] BYREF
    float smallscale; // [esp+74h] [ebp-Ch]
    char *text; // [esp+78h] [ebp-8h]
    float w; // [esp+7Ch] [ebp-4h] BYREF

    const cg_s *cgameGlob;

    SMALL_FRAC = 0.8f;

    iassert(compassType == COMPASS_TYPE_PARTIAL);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);

    if (color[3] != 0.0f)
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        coord = cgameGlob->refdef.vieworg[1] * cgameGlob->compassNorth[1]
            + cgameGlob->refdef.vieworg[0] * cgameGlob->compassNorth[0];
        coorda = 1.0 / compassCoords->current.vector[2] * coord + compassCoords->current.vector[1];
        v15 = R_TextHeight(font);
        scale = w / (double)v15;
        smallscale = scale * SMALL_FRAC;
        integerPortion = (int)coorda;
        text = va("%i ", integerPortion / 100);
        v14 = R_TextWidth(text, 10, font);
        textW = (double)v14 * smallscale;
        v13 = y + h;
        v12 = w * SMALL_FRAC + x;
        CL_DrawTextRotate(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            v12,
            v13,
            -90.0,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            style);
        y = y - textW;
        text = va("%i ", integerPortion % 100);
        textWa = (double)R_TextWidth(text, 10, font) * scale;
        v11 = y + h;
        v10 = x + w;
        CL_DrawTextRotate(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            v10,
            v11,
            -90.0,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            scale,
            color,
            style);
        y = y - textWa;
        text = va("%.4f", coorda - (double)integerPortion);
        v9 = y + h;
        v8 = x + w;
        CL_DrawTextRotate(
            &scrPlaceView[localClientNum],
            text + 2,
            40,
            font,
            v8,
            v9,
            -90.0,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            style);
    }
}

void __cdecl CG_CompassDrawPlayerEastCoord(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *const color,
    int32_t style)
{
    float v8; // [esp+20h] [ebp-54h]
    float v9; // [esp+24h] [ebp-50h]
    float v10; // [esp+2Ch] [ebp-48h]
    int32_t v11; // [esp+30h] [ebp-44h]
    int32_t v12; // [esp+34h] [ebp-40h]
    float textW; // [esp+3Ch] [ebp-38h]
    float textWa; // [esp+3Ch] [ebp-38h]
    int32_t integerPortion; // [esp+40h] [ebp-34h]
    float coord; // [esp+48h] [ebp-2Ch]
    float coorda; // [esp+48h] [ebp-2Ch]
    float SMALL_FRAC; // [esp+4Ch] [ebp-28h]
    float scale; // [esp+50h] [ebp-24h]
    float x; // [esp+54h] [ebp-20h] BYREF
    float y; // [esp+58h] [ebp-1Ch] BYREF
    float east[2]; // [esp+5Ch] [ebp-18h]
    float h; // [esp+64h] [ebp-10h] BYREF
    float smallscale; // [esp+68h] [ebp-Ch]
    char *text; // [esp+6Ch] [ebp-8h]
    float w; // [esp+70h] [ebp-4h] BYREF
    const cg_s *cgameGlob;

    SMALL_FRAC = 0.85f;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(compassType == COMPASS_TYPE_PARTIAL);

    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (color[3] != 0.0)
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        east[0] = cgameGlob->compassNorth[1];
        east[1] = -cgameGlob->compassNorth[0];
        coord = cgameGlob->refdef.vieworg[1] * east[1] + cgameGlob->refdef.vieworg[0] * east[0];
        coorda = 1.0 / compassCoords->current.vector[2] * coord + compassCoords->current.value;
        v12 = R_TextHeight(font);
        scale = h / (double)v12;
        smallscale = scale * SMALL_FRAC;
        integerPortion = (int)coorda;
        text = va("%i ", integerPortion / 100);
        v11 = R_TextWidth(text, 10, font);
        textW = (double)v11 * smallscale;
        v10 = h * SMALL_FRAC + y;
        CL_DrawText(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            x,
            v10,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            style);
        x = x + textW;
        text = va("%i ", integerPortion % 100);
        textWa = (double)R_TextWidth(text, 10, font) * scale;
        v9 = y + h;
        CL_DrawText(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            x,
            v9,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            scale,
            color,
            style);
        x = x + textWa;
        text = va("%.4f", coorda - (double)integerPortion);
        v8 = y + h;
        CL_DrawText(
            &scrPlaceView[localClientNum],
            text + 2,
            40,
            font,
            x,
            v8,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            style);
    }
}

void __cdecl CG_CompassDrawPlayerNCoordScroll(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *color,
    int32_t textStyle)
{
    int32_t v8; // [esp+24h] [ebp-48h]
    int32_t textW; // [esp+38h] [ebp-34h]
    float coorda; // [esp+40h] [ebp-2Ch]
    float coordb; // [esp+40h] [ebp-2Ch]
    float coordc; // [esp+40h] [ebp-2Ch]
    float coord; // [esp+40h] [ebp-2Ch]
    float fracPortion; // [esp+44h] [ebp-28h]
    float pixelY; // [esp+48h] [ebp-24h]
    float pixelPerCoord; // [esp+4Ch] [ebp-20h]
    float textHeight; // [esp+50h] [ebp-1Ch]
    float scale; // [esp+54h] [ebp-18h]
    float scalea; // [esp+54h] [ebp-18h]
    float x; // [esp+58h] [ebp-14h] BYREF
    float y; // [esp+5Ch] [ebp-10h] BYREF
    float h; // [esp+60h] [ebp-Ch] BYREF
    char *text; // [esp+64h] [ebp-8h]
    float w; // [esp+68h] [ebp-4h] BYREF

    const cg_s *cgameGlob;

    iassert(compassType == COMPASS_TYPE_PARTIAL);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);

    if (color[3] != 0.0f)
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        coorda = cgameGlob->refdef.vieworg[1] * cgameGlob->compassNorth[1]
            + cgameGlob->refdef.vieworg[0] * cgameGlob->compassNorth[0];
        coordb = coorda / compassCoords->current.vector[2] + compassCoords->current.vector[1];
        coordc = compassMaxRange->current.value * 0.5 / compassCoords->current.vector[2] + coordb;
        pixelPerCoord = compassCoords->current.vector[2] * h / compassMaxRange->current.value;
        fracPortion = 1.0 - (coordc - (double)(int)coordc);
        pixelY = y - fracPortion * pixelPerCoord;
        coord = coordc + fracPortion;
        v8 = R_TextWidth("99", 2, font);
        scale = w / (double)v8;
        textHeight = (double)R_TextHeight(font) * scale;
        while (pixelY < y + textHeight)
        {
            pixelY = pixelY + pixelPerCoord;
            coord = coord - 1.0;
        }
        while (pixelY < y + h)
        {
            text = va("%2i", (int)coord % 100);
            textW = R_TextWidth(text, 10, font);
            scalea = w / (double)textW;
            CL_DrawText(
                &scrPlaceView[localClientNum],
                text,
                40,
                font,
                x,
                pixelY,
                rect->horzAlign,
                rect->vertAlign,
                scalea,
                scalea,
                color,
                textStyle);
            pixelY = pixelY + pixelPerCoord;
            coord = coord - 1.0;
        }
    }
}

void __cdecl CG_CompassDrawPlayerECoordScroll(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *color,
    int32_t textStyle)
{
    float v8; // [esp+20h] [ebp-54h]
    int32_t v9; // [esp+28h] [ebp-4Ch]
    int32_t integerPortion; // [esp+34h] [ebp-40h]
    float leftCutoff; // [esp+38h] [ebp-3Ch]
    float coord; // [esp+40h] [ebp-34h]
    float coorda; // [esp+40h] [ebp-34h]
    float coordb; // [esp+40h] [ebp-34h]
    float textWidth; // [esp+44h] [ebp-30h]
    float fracPortion; // [esp+48h] [ebp-2Ch]
    float pixelPerCoord; // [esp+4Ch] [ebp-28h]
    float pixelX; // [esp+50h] [ebp-24h]
    float scale; // [esp+54h] [ebp-20h]
    float x; // [esp+58h] [ebp-1Ch] BYREF
    float y; // [esp+5Ch] [ebp-18h] BYREF
    float east[2]; // [esp+60h] [ebp-14h]
    float h; // [esp+68h] [ebp-Ch] BYREF
    char *text; // [esp+6Ch] [ebp-8h]
    float w; // [esp+70h] [ebp-4h] BYREF
    const cg_s *cgameGlob;

    iassert(compassType == COMPASS_TYPE_PARTIAL);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (color[3] != 0.0)
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        east[0] = cgameGlob->compassNorth[1];
        east[1] = -cgameGlob->compassNorth[0];
        coord = cgameGlob->refdef.vieworg[1] * east[1] + cgameGlob->refdef.vieworg[0] * east[0];
        coorda = coord / compassCoords->current.vector[2] + compassCoords->current.value;
        coordb = coorda - compassMaxRange->current.value * 0.5 / compassCoords->current.vector[2];
        pixelPerCoord = compassCoords->current.vector[2] * w / compassMaxRange->current.value;
        fracPortion = coordb - (double)(int)coordb;
        pixelX = x - fracPortion * pixelPerCoord;
        integerPortion = (int)coordb;
        v9 = R_TextHeight(font);
        scale = h / (double)v9;
        textWidth = (double)R_TextWidth("99", 2, font) * scale;
        while (1)
        {
            leftCutoff = compassECoordCutoff->current.value * compassSize->current.value + x;
            if (leftCutoff <= (double)pixelX)
                break;
            pixelX = pixelX + pixelPerCoord;
            ++integerPortion;
        }
        while (pixelX < x + w - textWidth)
        {
            text = va("%2i", integerPortion % 100);
            v8 = y + h;
            CL_DrawText(
                &scrPlaceView[localClientNum],
                text,
                40,
                font,
                pixelX,
                v8,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                scale,
                color,
                textStyle);
            pixelX = pixelX + pixelPerCoord;
            ++integerPortion;
        }
    }
}

void __cdecl CG_CompassDrawPlayerMap(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    float texCenter; // [esp+40h] [ebp-4Ch]
    float texCenter_4; // [esp+44h] [ebp-48h]
    float deltaEast; // [esp+48h] [ebp-44h]
    float delta; // [esp+4Ch] [ebp-40h]
    float delta_4; // [esp+50h] [ebp-3Ch]
    float texRadius; // [esp+58h] [ebp-34h]
    float rotation; // [esp+5Ch] [ebp-30h]
    float deltaSouth; // [esp+60h] [ebp-2Ch]
    float scaleFinalT; // [esp+64h] [ebp-28h]
    float x; // [esp+68h] [ebp-24h] BYREF
    float y; // [esp+6Ch] [ebp-20h] BYREF
    float south[2]; // [esp+70h] [ebp-1Ch]
    float east[2]; // [esp+78h] [ebp-14h]
    float h; // [esp+80h] [ebp-Ch] BYREF
    float scaleFinalS; // [esp+84h] [ebp-8h]
    float w; // [esp+88h] [ebp-4h] BYREF
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);

    if (color[3] != 0.0)
    {
        iassert(cgameGlob->compassMapWorldSize[0] != 0);
        iassert(cgameGlob->compassMapWorldSize[1] != 0);

        if (compassType)
        {
            iassert(compassType == COMPASS_TYPE_FULL);
            CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                cgameGlob->compassMapMaterial);
        }
        else
        {
            east[0] = cgameGlob->compassNorth[1];
            east[1] = -cgameGlob->compassNorth[0];
            south[0] = -cgameGlob->compassNorth[0];
            south[1] = -cgameGlob->compassNorth[1];
            delta = cgameGlob->refdef.vieworg[0] - cgameGlob->compassMapUpperLeft[0];
            delta_4 = cgameGlob->refdef.vieworg[1] - cgameGlob->compassMapUpperLeft[1];
            deltaEast = east[1] * delta_4 + east[0] * delta;
            deltaSouth = south[1] * delta_4 + south[0] * delta;
            texCenter = deltaEast / cgameGlob->compassMapWorldSize[0];
            texCenter_4 = deltaSouth / cgameGlob->compassMapWorldSize[1];
            if (cgameGlob->compassMapWorldSize[1] >= (double)cgameGlob->compassMapWorldSize[0])
            {
                texRadius = compassMaxRange->current.value * 0.5 / cgameGlob->compassMapWorldSize[1];
                scaleFinalS = cgameGlob->compassMapWorldSize[1] / cgameGlob->compassMapWorldSize[0];
                scaleFinalT = 1.0;
            }
            else
            {
                texRadius = compassMaxRange->current.value * 0.5 / cgameGlob->compassMapWorldSize[0];
                scaleFinalS = 1.0;
                scaleFinalT = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
            }
            if (compassRotation->current.enabled)
                rotation = -(cgameGlob->refdefViewAngles[1] - cgameGlob->compassNorthYaw);
            else
                rotation = 0.0;
            CG_CompassCalcDimensions(COMPASS_TYPE_PARTIAL, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            CL_DrawStretchPicRotatedST(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                texCenter,
                texCenter_4,
                texRadius,
                scaleFinalS,
                scaleFinalT,
                rotation,
                color,
                cgameGlob->compassMapMaterial);
        }
        CG_CompassDrawRadarEffects(localClientNum, compassType, parentRect, rect, color);
    }
}

void __cdecl CG_CompassDrawPlayerMapLocationSelector(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    float w; // [esp+30h] [ebp-64h]
    float h; // [esp+34h] [ebp-60h]
    float texMax; // [esp+3Ch] [ebp-58h]
    float texMax_4; // [esp+40h] [ebp-54h]
    const char *mtlName; // [esp+44h] [ebp-50h]
    int32_t mtlIndex; // [esp+48h] [ebp-4Ch]
    float radius; // [esp+50h] [ebp-44h]
    Material *selectorMaterial; // [esp+54h] [ebp-40h]
    float posScreen; // [esp+58h] [ebp-3Ch]
    float posScreen_4; // [esp+5Ch] [ebp-38h]
    float quadRad; // [esp+60h] [ebp-34h]
    float quadMax; // [esp+64h] [ebp-30h]
    float quadMax_4; // [esp+68h] [ebp-2Ch]
    float quadMin; // [esp+6Ch] [ebp-28h]
    float quadMin_4; // [esp+70h] [ebp-24h]
    rectDef_s scaledRect; // [esp+74h] [ebp-20h] BYREF
    float texMin[2]; // [esp+8Ch] [ebp-8h]
    const cg_s *cgameGlob;

    iassert(parentRect);
    iassert(rect);
    iassert(compassType == COMPASS_TYPE_FULL);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(cgameGlob->compassMapWorldSize[0]);
    iassert(cgameGlob->compassMapWorldSize[1]);

    if (cgameGlob->predictedPlayerState.locationSelectionInfo)
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        mtlIndex = cgameGlob->predictedPlayerState.locationSelectionInfo & 2;
        mtlName = CL_GetConfigString(localClientNum, mtlIndex + 827);
        selectorMaterial = Material_RegisterHandle(mtlName, 7);
        radius = (float)(cgameGlob->predictedPlayerState.locationSelectionInfo >> 2) / 63.0f;
        if (radius > 0.0)
        {
            quadRad = radius * scaledRect.h;
            posScreen = scaledRect.w * cgameGlob->selectedLocation[0] + scaledRect.x;
            posScreen_4 = scaledRect.h * cgameGlob->selectedLocation[1] + scaledRect.y;
            texMin[0] = (scaledRect.x - posScreen) / quadRad + 0.5;
            texMin[1] = (scaledRect.y - posScreen_4) / quadRad + 0.5;
            texMax = (scaledRect.x + scaledRect.w - posScreen) / quadRad + 0.5;
            texMax_4 = (scaledRect.y + scaledRect.h - posScreen_4) / quadRad + 0.5;
            quadMin = scaledRect.x;
            quadMin_4 = scaledRect.y;
            quadMax = scaledRect.x + scaledRect.w;
            quadMax_4 = scaledRect.y + scaledRect.h;
            if (texMin[0] < 0.0)
            {
                quadMin = (0.0 - texMin[0]) / (texMax - texMin[0]) * (quadMax - scaledRect.x) + scaledRect.x;
                texMin[0] = 0.0;
            }
            if (texMin[1] < 0.0)
            {
                quadMin_4 = (0.0 - texMin[1]) / (texMax_4 - texMin[1]) * (quadMax_4 - scaledRect.y) + scaledRect.y;
                texMin[1] = 0.0;
            }
            if (texMax > 1.0)
            {
                quadMax = (1.0 - texMin[0]) / (texMax - texMin[0]) * (quadMax - quadMin) + quadMin;
                texMax = 1.0;
            }
            if (texMax_4 > 1.0)
            {
                quadMax_4 = (1.0 - texMin[1]) / (texMax_4 - texMin[1]) * (quadMax_4 - quadMin_4) + quadMin_4;
                texMax_4 = 1.0;
            }
            h = quadMax_4 - quadMin_4;
            w = quadMax - quadMin;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                quadMin,
                quadMin_4,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                texMin[0],
                texMin[1],
                texMax,
                texMax_4,
                color,
                selectorMaterial);
        }
    }
}

void __cdecl CG_CompassDrawPlayer(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    rectDef_s *rect,
    Material *material,
    float *color)
{
    float xy[2]; // [esp+28h] [ebp-40h] BYREF
    const cg_s *cgameGlob; // [esp+30h] [ebp-38h]
    float angle; // [esp+34h] [ebp-34h]
    float centerY; // [esp+38h] [ebp-30h]
    rectDef_s scaledRect; // [esp+3Ch] [ebp-2Ch] BYREF
    float x; // [esp+54h] [ebp-14h]
    float y; // [esp+58h] [ebp-10h]
    float centerX; // [esp+5Ch] [ebp-Ch]
    float h; // [esp+60h] [ebp-8h]
    float w; // [esp+64h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType) * color[3];
    if (color[3] != 0.0)
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = scaledRect.w * 0.5 + scaledRect.x;
        centerY = scaledRect.h * 0.5 + scaledRect.y;
        if (compassType == COMPASS_TYPE_FULL)
        {
            w = cg_hudMapPlayerWidth->current.value;
            h = cg_hudMapPlayerHeight->current.value;
            xy[0] = 0.0;
            xy[1] = 0.0;
            CG_WorldPosToCompass(
                COMPASS_TYPE_FULL,
                cgameGlob,
                &scaledRect,
                0,
                cgameGlob->refdef.vieworg,
                cgameGlob->refdef.vieworg,
                0,
                xy);
            x = xy[0];
            y = xy[1];
        }
        else
        {
            if (compassType)
                MyAssertHandler(".\\cgame\\cg_compass.cpp", 880, 0, "%s", "compassType == COMPASS_TYPE_PARTIAL");
            w = compassPlayerWidth->current.value * compassSize->current.value;
            h = compassPlayerHeight->current.value * compassSize->current.value;
            KISAK_NULLSUB();
            x = 0.0;
            y = 0.0;
        }
        x = centerX - w * 0.5 + x;
        y = centerY - h * 0.5 + y;
        if (compassType || !compassRotation->current.enabled)
            angle = AngleDelta(cgameGlob->compassNorthYaw, cgameGlob->refdefViewAngles[1]);
        else
            angle = 0.0;
        CG_DrawRotatedPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            angle,
            color,
            material);
    }
}

void __cdecl CG_CompassDrawBorder(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    rectDef_s *rect,
    Material *material,
    float *color)
{
    float v6; // [esp+30h] [ebp-74h]
    float v7; // [esp+34h] [ebp-70h]
    float v8; // [esp+38h] [ebp-6Ch]
    float v9; // [esp+3Ch] [ebp-68h]
    float v10; // [esp+40h] [ebp-64h]
    float w; // [esp+44h] [ebp-60h]
    float v12; // [esp+48h] [ebp-5Ch]
    float v13; // [esp+4Ch] [ebp-58h]
    float v14; // [esp+50h] [ebp-54h]
    float v15; // [esp+54h] [ebp-50h]
    float v16; // [esp+58h] [ebp-4Ch]
    float h; // [esp+5Ch] [ebp-48h]
    float v18; // [esp+60h] [ebp-44h]
    float v19; // [esp+64h] [ebp-40h]
    float v20; // [esp+68h] [ebp-3Ch]
    float v21; // [esp+6Ch] [ebp-38h]
    float v22; // [esp+70h] [ebp-34h]
    float v23; // [esp+74h] [ebp-30h]
    float x; // [esp+78h] [ebp-2Ch]
    float y; // [esp+7Ch] [ebp-28h]
    float borderWidth; // [esp+84h] [ebp-20h]
    float border; // [esp+88h] [ebp-1Ch]
    rectDef_s scaledRect; // [esp+8Ch] [ebp-18h] BYREF
    const cg_s *cgameGlob;

    iassert(compassType == COMPASS_TYPE_FULL);
    border = cg_hudMapBorderWidth->current.value;
    if (border > 0.0f)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
        if (color[3] != 0.0f)
        {
            CG_CompassCalcDimensions(
                compassType,
                cgameGlob,
                parentRect,
                rect,
                &scaledRect.x,
                &scaledRect.y,
                &scaledRect.w,
                &scaledRect.h);
            if (border > scaledRect.w * 0.5f)
                border = scaledRect.w * 0.5f;
            if (border > scaledRect.h * 0.5f)
                border = scaledRect.h * 0.5f;
            borderWidth = border + border;
            y = scaledRect.y + scaledRect.h - border;
            x = scaledRect.x - border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                x,
                y,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.5,
                0.5,
                1.0,
                color,
                material);
            v23 = scaledRect.y - border;
            v22 = scaledRect.x - border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v22,
                v23,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                0.5,
                0.5,
                color,
                material);
            v21 = scaledRect.y + scaledRect.h - border;
            v20 = scaledRect.x + scaledRect.w - border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v20,
                v21,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.5,
                1.0,
                1.0,
                color,
                material);
            v19 = scaledRect.y - border;
            v18 = scaledRect.x + scaledRect.w - border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v18,
                v19,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.0,
                1.0,
                0.5,
                color,
                material);
            h = scaledRect.h - border * 2.0;
            v16 = scaledRect.y + border;
            v15 = scaledRect.x - border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v15,
                v16,
                borderWidth,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.5,
                0.5,
                0.5,
                color,
                material);
            v14 = scaledRect.h - border * 2.0;
            v13 = scaledRect.y + border;
            v12 = scaledRect.x + scaledRect.w - border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v12,
                v13,
                borderWidth,
                v14,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.5,
                1.0,
                0.5,
                color,
                material);
            w = scaledRect.w - border * 2.0;
            v10 = scaledRect.y - border;
            v9 = scaledRect.x + border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v9,
                v10,
                w,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.0,
                0.5,
                0.5,
                color,
                material);
            v8 = scaledRect.w - border * 2.0;
            v7 = scaledRect.y + scaledRect.h - border;
            v6 = scaledRect.x + border;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                v6,
                v7,
                v8,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.5,
                0.5,
                1.0,
                color,
                material);
        }
    }
}

void __cdecl CG_CompassUpYawVector(const cg_s *cgameGlob, float *result)
{
    if (compassRotation->current.enabled)
        YawVectors2D(cgameGlob->refdefViewAngles[1], result, 0);
    else
        *(double *)result = *(double *)cgameGlob->compassNorth;
}

void __cdecl CG_CompassDrawTickertape(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color,
    Font_s *textFont,
    float textScale,
    int32_t textStyle,
    bool drawObjectives)
{
    float angle; // [esp+2Ch] [ebp-B0h]
    float v11; // [esp+30h] [ebp-ACh]
    float v12; // [esp+34h] [ebp-A8h]
    float textAlpha; // [esp+48h] [ebp-94h]
    float dista; // [esp+4Ch] [ebp-90h]
    float dist; // [esp+4Ch] [ebp-90h]
    float percent; // [esp+50h] [ebp-8Ch]
    float goalAngleDelta; // [esp+54h] [ebp-88h]
    float posDelta[2]; // [esp+5Ch] [ebp-80h] BYREF
    Material *iconMaterial; // [esp+64h] [ebp-78h]
    float iconX; // [esp+68h] [ebp-74h]
    float iconDrawX; // [esp+6Ch] [ebp-70h]
    float goalYaw; // [esp+70h] [ebp-6Ch]
    const float *goalOrig; // [esp+74h] [ebp-68h]
    float tapeRight; // [esp+78h] [ebp-64h]
    float iconY; // [esp+7Ch] [ebp-60h]
    float defAlpha; // [esp+80h] [ebp-5Ch]
    float iconDrawY; // [esp+84h] [ebp-58h]
    const cg_s *cgameGlob; // [esp+88h] [ebp-54h]
    float tapeLeft; // [esp+8Ch] [ebp-50h]
    centity_s *cent; // [esp+90h] [ebp-4Ch]
    float tapeAngleStretch; // [esp+94h] [ebp-48h]
    float nearestDistHeightDelta; // [esp+98h] [ebp-44h]
    float iconW; // [esp+9Ch] [ebp-40h] BYREF
    float iconH; // [esp+A0h] [ebp-3Ch] BYREF
    int32_t objIdx; // [esp+A4h] [ebp-38h]
    const objective_t *objective; // [esp+A8h] [ebp-34h]
    float x; // [esp+ACh] [ebp-30h] BYREF
    float y; // [esp+B0h] [ebp-2Ch] BYREF
    float tapeRotation; // [esp+B4h] [ebp-28h]
    float tapeCenter; // [esp+B8h] [ebp-24h]
    float colorMod[4]; // [esp+BCh] [ebp-20h] BYREF
    float nearestDist; // [esp+CCh] [ebp-10h]
    float h; // [esp+D0h] [ebp-Ch] BYREF
    float w; // [esp+D4h] [ebp-8h] BYREF
    float tapeAngleCenter; // [esp+D8h] [ebp-4h]

    iassert(compassType == COMPASS_TYPE_PARTIAL);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    defAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (defAlpha != 0.0)
    {
        if (color[3] < (double)defAlpha)
            defAlpha = color[3];
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = defAlpha;
        tapeRotation = -(cgameGlob->refdefViewAngles[1] - cgameGlob->compassNorthYaw);
        tapeCenter = tapeRotation / 360.0;
        tapeLeft = tapeCenter - compassTickertapeStretch->current.value * 0.5;
        tapeRight = compassTickertapeStretch->current.value * 0.5 + tapeCenter;
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            tapeLeft,
            0.0,
            tapeRight,
            1.0,
            colorMod,
            material);
        if (drawObjectives)
        {
            CalcCompassPointerSize(compassType, &iconW, &iconH);
            iconY = h * 0.5 + y;
            iconDrawY = iconY - iconH * 0.5;
            tapeAngleStretch = compassTickertapeStretch->current.value * 360.0 * 0.5;
            angle = -tapeRotation;
            tapeAngleCenter = AngleNormalize360(angle);
            nearestDist = FLT_MAX;
            nearestDistHeightDelta = 0.0;
            for (objIdx = 0; objIdx < 16; ++objIdx)
            {
                objective = &cgameGlob->nextSnap->ps.objective[objIdx];
                if (objective->state == OBJST_CURRENT || objective->state == OBJST_ACTIVE)
                {
                    if (objective->entNum == 1023)
                    {
                        goalOrig = objective->origin;
                    }
                    else
                    {
                        cent = CG_GetEntity(localClientNum, objective->entNum);
                        goalOrig = cent->pose.origin;
                    }
                    colorMod[3] = defAlpha;
                    if (*goalOrig != 0.0 || goalOrig[1] != 0.0 || goalOrig[2] != 0.0)
                    {
                        posDelta[0] = *goalOrig - cgameGlob->refdef.vieworg[0];
                        posDelta[1] = goalOrig[1] - cgameGlob->refdef.vieworg[1];
                        dista = Vec2Length(posDelta);
                        dist = 0.0254 * dista;
                        if (dist < (double)nearestDist)
                        {
                            nearestDist = dist;
                            nearestDistHeightDelta = cgameGlob->refdef.vieworg[2] - goalOrig[2];
                        }
                        v12 = atan2(posDelta[1], posDelta[0]);
                        goalYaw = v12 * 180.0 / 3.141592741012573;
                        v11 = goalYaw - cgameGlob->compassNorthYaw;
                        goalYaw = AngleNormalize360(v11);
                        goalAngleDelta = -AngleDelta(goalYaw, tapeAngleCenter);
                        if (goalAngleDelta >= -tapeAngleStretch)
                        {
                            if (tapeAngleStretch >= (double)goalAngleDelta)
                                percent = (goalAngleDelta + tapeAngleStretch) / (tapeAngleStretch + tapeAngleStretch);
                            else
                                percent = 1.0;
                        }
                        else
                        {
                            percent = 0.0;
                        }
                        textAlpha = defAlpha;
                        iconMaterial = CG_ObjectiveIcon(localClientNum, objective->icon, 0);
                        iconX = w * percent + x;
                        iconDrawX = iconX - iconW * 0.5;
                        CL_DrawStretchPic(
                            &scrPlaceView[localClientNum],
                            iconDrawX,
                            iconDrawY,
                            iconW,
                            iconH,
                            rect->horzAlign,
                            rect->vertAlign,
                            0.0,
                            0.0,
                            1.0,
                            1.0,
                            colorMod,
                            iconMaterial);
                        colorMod[3] = textAlpha;
                        DrawIconDistanceText(
                            localClientNum,
                            dist,
                            iconX,
                            iconY,
                            iconH,
                            rect,
                            colorMod,
                            textFont,
                            textScale,
                            textStyle);
                    }
                }
            }
            if (compassObjectiveNearbyDist->current.value >= (double)nearestDist)
                KISAK_NULLSUB();
        }
    }
}

void __cdecl CalcCompassPointerSize(CompassType compassType, float *w, float *h)
{
    if (!w)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 975, 0, "%s", "w");
    if (!h)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 976, 0, "%s", "h");
    if (compassType)
    {
        *w = compassObjectiveIconWidth->current.value;
        *h = compassObjectiveIconHeight->current.value;
    }
    else
    {
        *w = compassObjectiveWidth->current.value * compassSize->current.value;
        *h = compassObjectiveHeight->current.value * compassSize->current.value;
    }
}

void __cdecl DrawIconDistanceText(
    int32_t localClientNum,
    float distance,
    float iconX,
    float iconY,
    float iconH,
    const rectDef_s *rect,
    const float *color,
    Font_s *textFont,
    float textScale,
    int32_t textStyle)
{
    double v10; // st7
    char str[68]; // [esp+24h] [ebp-68h] BYREF
    float textY; // [esp+6Ch] [ebp-20h]
    float textX; // [esp+70h] [ebp-1Ch]
    float textWidth; // [esp+74h] [ebp-18h]
    float textHeight; // [esp+78h] [ebp-14h]
    float colorMod[4]; // [esp+7Ch] [ebp-10h] BYREF

    if (!rect)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 1111, 0, "%s", "rect");
    colorMod[0] = 0.89999998f;
    colorMod[1] = 1.0f;
    colorMod[2] = 0.1f;
    colorMod[3] = color[3];
    if (compassObjectiveDetailDist->current.value <= (double)distance)
    {
        Com_sprintf(str, 0x40u, "%im", (int)distance);
    }
    else
    {
        v10 = CutFloat(distance);
        Com_sprintf(str, 0x40u, "%.1fm", v10);
    }
    textWidth = (float)UI_TextWidth(str, 20, textFont, textScale);
    textHeight = (float)UI_TextHeight(textFont, textScale);
    textX = iconX - textWidth * 0.5;
    textY = iconY + iconH + 2.0;
    UI_DrawTextNoSnap(
        &scrPlaceView[localClientNum],
        str,
        0x7FFFFFFF,
        textFont,
        textX,
        textY,
        rect->horzAlign,
        rect->vertAlign,
        textScale,
        colorMod,
        textStyle);
}

double __cdecl CutFloat(float original)
{
    return (float)((double)(int)(original * 10.0) * 0.1000000014901161);
}

double __cdecl CG_GetHudAlphaCompass(int32_t localClientNum)
{
    cg_s *LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    return CG_FadeCompass(localClientNum, LocalClientGlobals->compassFadeTime, COMPASS_TYPE_PARTIAL);
}

void __cdecl CalcCompassFriendlySize(CompassType compassType, float *w, float *h)
{
    if (!w)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 1470, 0, "%s", "w");
    if (!h)
        MyAssertHandler(".\\cgame\\cg_compass.cpp", 1471, 0, "%s", "h");
    if (compassType)
    {
        *w = cg_hudMapFriendlyWidth->current.value;
        *h = cg_hudMapFriendlyHeight->current.value;
    }
    else
    {
        *w = compassFriendlyWidth->current.value * compassSize->current.value;
        *h = compassFriendlyHeight->current.value * compassSize->current.value;
    }
}

void __cdecl CG_CompassDrawPlayerPointers_MP(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color)
{
    float v6; // [esp+20h] [ebp-B4h]
    float tempcolor; // [esp+30h] [ebp-A4h]
    float lerp; // [esp+34h] [ebp-A0h]
    float LINE_WIDTH; // [esp+38h] [ebp-9Ch]
    Material *icon; // [esp+3Ch] [ebp-98h]
    float yawVector[2]; // [esp+40h] [ebp-94h] BYREF
    bool clipped; // [esp+4Bh] [ebp-89h]
    const objective_t *obj; // [esp+4Ch] [ebp-88h]
    float delta[3]; // [esp+50h] [ebp-84h] BYREF
    float xy[2]; // [esp+5Ch] [ebp-78h] BYREF
    float origin[3]; // [esp+64h] [ebp-70h] BYREF
    cg_s *cgameGlob; // [esp+70h] [ebp-64h]
    centity_s *cent; // [esp+74h] [ebp-60h]
    float objDist; // [esp+78h] [ebp-5Ch]
    float xyClipped[2]; // [esp+7Ch] [ebp-58h] BYREF
    float centerY; // [esp+84h] [ebp-50h]
    rectDef_s scaledRect; // [esp+88h] [ebp-4Ch] BYREF
    float x; // [esp+A0h] [ebp-34h]
    float y; // [esp+A4h] [ebp-30h]
    const playerState_s *ps; // [esp+A8h] [ebp-2Ch]
    int32_t objNum; // [esp+ACh] [ebp-28h]
    float clipfade; // [esp+B0h] [ebp-24h]
    float centerX; // [esp+B4h] [ebp-20h]
    float h; // [esp+B8h] [ebp-1Ch] BYREF
    float fadeAlpha; // [esp+BCh] [ebp-18h]
    float w; // [esp+C0h] [ebp-14h] BYREF
    float newColor[4]; // [esp+C4h] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fadeAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (fadeAlpha != 0.0)
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = scaledRect.w * 0.5 + scaledRect.x;
        centerY = scaledRect.h * 0.5 + scaledRect.y;
        ps = &cgameGlob->nextSnap->ps;
        if (ps->clientNum >= 0x40u)
            MyAssertHandler(
                ".\\cgame\\cg_compass.cpp",
                1542,
                0,
                "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                ps->clientNum,
                64);
        if (cgameGlob->bgs.clientinfo[ps->clientNum].infoValid)
        {
            CG_CompassUpYawVector(cgameGlob, yawVector);
            for (objNum = 0; objNum < 16; ++objNum)
            {
                obj = &ps->objective[objNum];
                if (obj->state == OBJST_CURRENT || obj->state == OBJST_ACTIVE)
                {
                    if (obj->entNum == 1023)
                    {
                        origin[0] = obj->origin[0];
                        origin[1] = obj->origin[1];
                        origin[2] = obj->origin[2];
                    }
                    else
                    {
                        cent = CG_GetEntity(localClientNum, obj->entNum);
                        origin[0] = cent->pose.origin[0];
                        origin[1] = cent->pose.origin[1];
                        origin[2] = cent->pose.origin[2];
                    }
                    Vec3Sub(origin, cgameGlob->refdef.vieworg, delta);
                    objDist = Vec2Length(delta);
                    newColor[0] = *color;
                    newColor[1] = color[1];
                    newColor[2] = color[2];
                    newColor[3] = color[3];
                    if (compassMaxRange->current.value < (double)objDist)
                    {
                        if (compassObjectiveMaxRange->current.value > (double)objDist)
                        {
                            lerp = 0.0;
                            if (compassObjectiveMaxRange->current.value - compassMaxRange->current.value != 0.0)
                                lerp = (objDist - compassMaxRange->current.value)
                                / (compassObjectiveMaxRange->current.value - compassMaxRange->current.value);
                            newColor[3] = 1.0 - (1.0 - compassObjectiveMinAlpha->current.value) * lerp;
                        }
                        else
                        {
                            newColor[3] = compassObjectiveMinAlpha->current.value;
                        }
                    }
                    else
                    {
                        newColor[3] = 1.0;
                    }
                    xyClipped[0] = 0.0;
                    xyClipped[1] = 0.0;
                    clipped = CG_WorldPosToCompass(
                        compassType,
                        cgameGlob,
                        &scaledRect,
                        yawVector,
                        cgameGlob->refdef.vieworg,
                        origin,
                        xy,
                        xyClipped);
                    xy[0] = xy[0] + centerX;
                    xy[1] = xy[1] + centerY;
                    xyClipped[0] = xyClipped[0] + centerX;
                    xyClipped[1] = xyClipped[1] + centerY;
                    CalcCompassPointerSize(compassType, &w, &h);
                    icon = CG_ObjectiveIcon(localClientNum, obj->icon, 0);
                    if (newColor[3] > (double)fadeAlpha)
                        newColor[3] = fadeAlpha;
                    LINE_WIDTH = 2.0;
                    x = xy[0] - w * 0.5;
                    y = xy[1] - h * 0.5;
                    clipfade = GetObjectiveFade(&scaledRect, x, y, w, h);
                    if (clipfade > 0.0 && obj->state == OBJST_CURRENT)
                    {
                        tempcolor = newColor[3];
                        newColor[3] = newColor[3] * clipfade;
                        if (scaledRect.y <= (double)xy[1] && xy[1] <= scaledRect.y + scaledRect.h)
                        {
                            v6 = xy[1] - LINE_WIDTH * 0.5;
                            UI_DrawHandlePic(
                                &scrPlaceView[localClientNum],
                                scaledRect.x,
                                v6,
                                scaledRect.w,
                                LINE_WIDTH,
                                rect->horzAlign,
                                rect->vertAlign,
                                newColor,
                                material);
                        }
                        if (scaledRect.x <= (double)xy[0] && xy[0] <= scaledRect.x + scaledRect.w)
                            CG_DrawVLine(
                                &scrPlaceView[localClientNum],
                                xy[0],
                                scaledRect.y,
                                LINE_WIDTH,
                                scaledRect.h,
                                rect->horzAlign,
                                rect->vertAlign,
                                newColor,
                                material);
                        newColor[3] = tempcolor;
                    }
                    x = xyClipped[0] - w * 0.5;
                    y = xyClipped[1] - h * 0.5;
                    UI_DrawHandlePic(&scrPlaceView[localClientNum], x, y, w, h, rect->horzAlign, rect->vertAlign, newColor, icon);
                }
            }
        }
    }
}

double __cdecl GetObjectiveFade(const rectDef_s *clipRect, float x, float y, float width, float height)
{
    float clip; // [esp+4h] [ebp-8h]
    float clipa; // [esp+4h] [ebp-8h]
    float clipb; // [esp+4h] [ebp-8h]
    float clipc; // [esp+4h] [ebp-8h]
    float maxclip; // [esp+8h] [ebp-4h]

    maxclip = 0.0;
    clip = (clipRect->x - x) / width;
    if ((float)0.0 < (double)clip)
        maxclip = (clipRect->x - x) / width;
    clipa = (clipRect->y - y) / height;
    if (maxclip < (double)clipa)
        maxclip = (clipRect->y - y) / height;
    clipb = (x + width - (clipRect->x + clipRect->w)) / width;
    if (maxclip < (double)clipb)
        maxclip = (x + width - (clipRect->x + clipRect->w)) / width;
    clipc = (y + height - (clipRect->y + clipRect->h)) / height;
    if (maxclip < (double)clipc)
        maxclip = (y + height - (clipRect->y + clipRect->h)) / height;
    if (maxclip > 1.0)
        maxclip = 1.0;
    return (float)(1.0 - maxclip);
}

