#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <bgame/bg_local.h>

#include <client/client.h>

#include <ui/ui.h>
#include <cgame/cg_public.h>
#include <stringed/stringed_hooks.h>

const dvar_t *hud_fade_sprint;
const dvar_t *hud_health_pulserate_injured;
const dvar_t *hud_health_startpulse_critical;
const dvar_t *hud_fade_offhand;
const dvar_t *hud_deathQuoteFadeTime;
const dvar_t *hud_fade_ammodisplay;
const dvar_t *hud_health_startpulse_injured;
const dvar_t *hud_fade_stance;
const dvar_t *hud_fade_compass;
const dvar_t *hud_health_pulserate_critical;
const dvar_t *hud_fade_healthbar;
const dvar_t *hud_fadeout_speed;
const dvar_t *hud_enable;
const dvar_t *hud_healthOverlay_regenPauseTime;
const dvar_t *hud_healthOverlay_pulseStart;
const dvar_t *hud_healthOverlay_phaseOne_pulseDuration;
const dvar_t *hud_healthOverlay_phaseTwo_toAlphaMultiplier;
const dvar_t *hud_healthOverlay_phaseTwo_pulseDuration;
const dvar_t *hud_healthOverlay_phaseThree_toAlphaMultiplier;
const dvar_t *hud_healthOverlay_phaseThree_pulseDuration;
const dvar_t *hud_healthOverlay_phaseEnd_toAlpha;
const dvar_t *hud_healthOverlay_phaseEnd_pulseDuration;
const dvar_t *cg_sprintMeterFullColor;
const dvar_t *cg_sprintMeterEmptyColor;
const dvar_t *cg_sprintMeterDisabledColor;
const dvar_t *cg_drawTalk;

const char *cg_drawTalkNames[5] = { "NONE", "ALL", "FRIENDLY", "ENEMY", NULL }; // idb

void __cdecl CG_AntiBurnInHUD_RegisterDvars()
{
    DvarLimits min; // [esp+10h] [ebp-10h]
    DvarLimits mina; // [esp+10h] [ebp-10h]
    DvarLimits minb; // [esp+10h] [ebp-10h]
    DvarLimits minc; // [esp+10h] [ebp-10h]
    DvarLimits mind; // [esp+10h] [ebp-10h]
    DvarLimits mine; // [esp+10h] [ebp-10h]
    DvarLimits minf; // [esp+10h] [ebp-10h]
    DvarLimits ming; // [esp+10h] [ebp-10h]
    DvarLimits minh; // [esp+10h] [ebp-10h]
    DvarLimits mini; // [esp+10h] [ebp-10h]
    DvarLimits minj; // [esp+10h] [ebp-10h]
    DvarLimits mink; // [esp+10h] [ebp-10h]
    DvarLimits minl; // [esp+10h] [ebp-10h]
    DvarLimits minm; // [esp+10h] [ebp-10h]
    DvarLimits minn; // [esp+10h] [ebp-10h]
    DvarLimits mino; // [esp+10h] [ebp-10h]
    DvarLimits minp; // [esp+10h] [ebp-10h]
    DvarLimits minq; // [esp+10h] [ebp-10h]

    min.value.max = 1.0;
    min.value.min = 0.0;
    hud_fadeout_speed = Dvar_RegisterFloat("hud_fadeout_speed", 0.1, min, 1u, "The speed that the HUD will fade at");
    hud_enable = Dvar_RegisterBool("hud_enable", 1, 1u, "Enable hud elements");
    mina.value.max = 30.0;
    mina.value.min = 0.0;
    hud_fade_ammodisplay = Dvar_RegisterFloat(
        "hud_fade_ammodisplay",
        0.0,
        mina,
        1u,
        "The time for the ammo display to fade in seconds");
    minb.value.max = 30.0;
    minb.value.min = 0.0;
    hud_fade_healthbar = Dvar_RegisterFloat(
        "hud_fade_healthbar",
        2.0,
        minb,
        1u,
        "The time for the health bar to fade in seconds");
    minc.value.max = 30.0;
    minc.value.min = 0.0;
    hud_fade_compass = Dvar_RegisterFloat(
        "hud_fade_compass",
        0.0,
        minc,
        1u,
        "The time for the compass to fade in seconds");
    mind.value.max = 30.0;
    mind.value.min = 0.0;
    hud_fade_stance = Dvar_RegisterFloat("hud_fade_stance", 1.7, mind, 1u, "The time for the stance to fade in seconds");
    mine.value.max = 30.0;
    mine.value.min = 0.0;
    hud_fade_offhand = Dvar_RegisterFloat(
        "hud_fade_offhand",
        0.0,
        mine,
        1u,
        "The time for the offhand weapons to fade in seconds");
    minf.value.max = 30.0;
    minf.value.min = 0.0;
    hud_fade_sprint = Dvar_RegisterFloat(
        "hud_fade_sprint",
        1.7,
        minf,
        1u,
        "The time for the sprint meter to fade in seconds");
    ming.value.max = 1.1;
    ming.value.min = 0.0;
    hud_health_startpulse_injured = Dvar_RegisterFloat(
        "hud_health_startpulse_injured",
        1.0,
        ming,
        1u,
        "The health level at which to start the 'injured' pulse effect");
    minh.value.max = 1.1;
    minh.value.min = 0.0;
    hud_health_startpulse_critical = Dvar_RegisterFloat(
        "hud_health_startpulse_critical",
        0.33000001,
        minh,
        1u,
        "The health level at which to start the 'critical' pulse effect");
    mini.value.max = 3.0;
    mini.value.min = 0.1;
    hud_health_pulserate_injured = Dvar_RegisterFloat(
        "hud_health_pulserate_injured",
        1.0,
        mini,
        1u,
        "The pulse rate of the 'injured' pulse effect");
    minj.value.max = 3.0;
    minj.value.min = 0.1;
    hud_health_pulserate_critical = Dvar_RegisterFloat(
        "hud_health_pulserate_critical",
        0.5,
        minj,
        1u,
        "The pulse rate of the 'critical' pulse effect");
    hud_deathQuoteFadeTime = Dvar_RegisterInt(
        "hud_deathQuoteFadeTime",
        1000,
        (DvarLimits)0x186A000000000LL,
        1u,
        "The time for the death quote to fade");
    hud_healthOverlay_regenPauseTime = Dvar_RegisterInt(
        "hud_healthOverlay_regenPauseTime",
        8000,
        (DvarLimits)0x271000000000LL,
        0x80u,
        "The time in milliseconds before the health regeneration kicks in");
    mink.value.max = 1.0;
    mink.value.min = 0.0;
    hud_healthOverlay_pulseStart = Dvar_RegisterFloat(
        "hud_healthOverlay_pulseStart",
        0.55000001,
        mink,
        0x80u,
        "The percentage of full health at which the low-health warning overlay begins flashing");
    hud_healthOverlay_phaseOne_pulseDuration = Dvar_RegisterInt(
        "hud_healthOverlay_phaseOne_pulseDuration",
        150,
        (DvarLimits)0x3E800000000LL,
        0x80u,
        "Time in milliseconds to ramp up to the first alpha value (the peak of the pulse)");
    minl.value.max = 1.0;
    minl.value.min = 0.0;
    hud_healthOverlay_phaseTwo_toAlphaMultiplier = Dvar_RegisterFloat(
        "hud_healthOverlay_phaseTwo_toAlphaMultiplier",
        0.69999999,
        minl,
        0x80u,
        "Alpha multiplier for the second health overlay phase (percentage of the pulse peak)");
    hud_healthOverlay_phaseTwo_pulseDuration = Dvar_RegisterInt(
        "hud_healthOverlay_phaseTwo_pulseDuration",
        320,
        (DvarLimits)0x3E800000000LL,
        0x80u,
        "Time in milliseconds to fade the alpha to hud_healthOverlay_phaseTwo_toAlphaMultiplier");
    minm.value.max = 1.0;
    minm.value.min = 0.0;
    hud_healthOverlay_phaseThree_toAlphaMultiplier = Dvar_RegisterFloat(
        "hud_healthOverlay_phaseThree_toAlphaMultiplier",
        0.60000002,
        minm,
        0x80u,
        "Alpha multiplier for the third health overlay phase (percentage of the pulse peak)");
    hud_healthOverlay_phaseThree_pulseDuration = Dvar_RegisterInt(
        "hud_healthOverlay_phaseThree_pulseDuration",
        400,
        (DvarLimits)0x3E800000000LL,
        0x80u,
        "Time in milliseconds to fade the alpha to hud_healthOverlay_phaseThree_"
        "toAlphaMultiplier");
    minn.value.max = 1.0;
    minn.value.min = 0.0;
    hud_healthOverlay_phaseEnd_toAlpha = Dvar_RegisterFloat(
        "hud_healthOverlay_phaseEnd_toAlpha",
        0.0,
        minn,
        0x80u,
        "Alpha multiplier to fade to before turning off the overlay (percentage of the pulse peak)");
    hud_healthOverlay_phaseEnd_pulseDuration = Dvar_RegisterInt(
        "hud_healthOverlay_phaseEnd_pulseDuration",
        700,
        (DvarLimits)0x3E800000000LL,
        0x80u,
        "Time in milliseconds to fade out the health overlay after it is done flashing");
    mino.value.max = 1.0;
    mino.value.min = 0.0;
    cg_sprintMeterFullColor = Dvar_RegisterVec4(
        "cg_sprintMeterFullColor",
        0.80000001,
        0.80000001,
        0.80000001,
        0.80000001,
        mino,
        1u,
        "The color of the sprint meter when the sprint meter is full");
    minp.value.max = 1.0;
    minp.value.min = 0.0;
    cg_sprintMeterEmptyColor = Dvar_RegisterVec4(
        "cg_sprintMeterEmptyColor",
        0.69999999,
        0.5,
        0.2,
        0.80000001,
        minp,
        1u,
        "The color of the sprint meter when the sprint meter is empty");
    minq.value.max = 1.0;
    minq.value.min = 0.0;
    cg_sprintMeterDisabledColor = Dvar_RegisterVec4(
        "cg_sprintMeterDisabledColor",
        0.80000001,
        0.1,
        0.1,
        0.2,
        minq,
        1u,
        "The color of the sprint meter when the sprint meter is disabled");
    cg_drawTalk = Dvar_RegisterEnum(
        "cg_drawTalk",
        cg_drawTalkNames,
        1,
        0,
        "Controls which icons CG_TALKER ownerdraw draws");
}

bool __cdecl CG_ShouldDrawHud(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return cgArray[0].drawHud && CL_ShouldDisplayHud(localClientNum);
}

double __cdecl CG_FadeHudMenu(int localClientNum, const dvar_s *fadeDvar, int displayStartTime, int duration)
{
    float *fadeColor; // [esp+4h] [ebp-4h]

    if (!hud_enable->current.enabled)
        return 0.0;
    if (!CG_ShouldDrawHud(localClientNum))
        return 0.0;
    if (fadeDvar->current.value == 0.0)
        return 1.0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    fadeColor = CG_FadeColor(cgArray[0].time, displayStartTime, duration, 700);
    if (fadeColor)
        return fadeColor[3];
    else
        return 0.0;
}

bool __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, unsigned int weapIndex)
{
    int maxAmmo; // [esp+4h] [ebp-10h]
    int curAmmo; // [esp+8h] [ebp-Ch]
    const playerState_s *ps; // [esp+Ch] [ebp-8h]

    ps = &cgameGlob->predictedPlayerState;
    if (!weapIndex)
        return 0;
    BG_AmmoForWeapon(weapIndex);
    curAmmo = BG_GetTotalAmmoReserve(ps, weapIndex);
    if (curAmmo > 999)
        curAmmo = 999;
    BG_GetWeaponDef(weapIndex);
    maxAmmo = BG_GetAmmoPlayerMax(ps, weapIndex, 0);
    if (maxAmmo > 999)
        maxAmmo = 999;
    return maxAmmo >= 0 && (double)maxAmmo * 0.2000000029802322 >= (double)curAmmo;
}

bool __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    weapIndex = GetWeaponIndex(cgameGlob);
    return CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapIndex);
}

bool __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, unsigned int weapIndex)
{
    int curClipVal; // [esp+0h] [ebp-10h]
    WeaponDef *weapDef; // [esp+8h] [ebp-8h]
    int fullClipVal; // [esp+Ch] [ebp-4h]

    if (!weapIndex)
        return 0;
    if (BG_WeaponIsClipOnly(weapIndex))
        return 0;
    curClipVal = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIndex)];
    if (curClipVal < 0)
        return 0;
    if (curClipVal > 999)
        curClipVal = 999;
    weapDef = BG_GetWeaponDef(weapIndex);
    fullClipVal = weapDef->iClipSize;
    if (fullClipVal > 999)
        fullClipVal = 999;
    return fullClipVal > 0 && (double)fullClipVal * weapDef->lowAmmoWarningThreshold >= (double)curClipVal;
}

bool __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    weapIndex = GetWeaponIndex(cgameGlob);
    return CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex);
}

double __cdecl CG_CalcPlayerHealth(const playerState_s *ps)
{
    float v3; // [esp+4h] [ebp-10h]
    float v4; // [esp+8h] [ebp-Ch]
    float v5; // [esp+Ch] [ebp-8h]
    float health; // [esp+10h] [ebp-4h]

    if (!ps->stats[0] || !ps->stats[2] || ps->pm_type == 7)
        return 0.0;
    health = (double)ps->stats[0] / (double)ps->stats[2];
    v4 = health - 1.0;
    if (v4 < 0.0)
        v5 = (double)ps->stats[0] / (double)ps->stats[2];
    else
        v5 = 1.0;
    v3 = 0.0 - health;
    if (v3 < 0.0)
        return v5;
    else
        return (float)0.0;
}

void __cdecl CG_ResetLowHealthOverlay(cg_s *cgameGlob)
{
    cgameGlob->healthOverlayHurt = 0;
    cgameGlob->healthOverlayToAlpha = hud_healthOverlay_phaseEnd_toAlpha->current.value;
    cgameGlob->healthOverlayPulseDuration = 0;
    cgameGlob->healthOverlayPulsePhase = 0;
    cgameGlob->healthOverlayPulseIndex = 0;
    cgameGlob->healthOverlayOldHealth = 1.0;
}

int __cdecl CG_ServerMaterialName(int localClientNum, int index, char *materialName, unsigned int maxLen)
{
    char v5; // cl
    char *v7; // [esp+Ch] [ebp-18h]
    char *string; // [esp+20h] [ebp-4h]

    if (index <= 0 || index >= 256)
        return 0;
    string = CL_GetConfigString(localClientNum, index + 2002);
    if (!*string)
        return 0;
    if (strlen(string) >= maxLen)
        return 0;
    v7 = string;
    do
    {
        v5 = *v7;
        *materialName++ = *v7++;
    } while (v5);
    return 1;
}

Material *__cdecl CG_ObjectiveIcon(int localClientNum, int icon, int type)
{
    char shaderName[68]; // [esp+0h] [ebp-48h] BYREF

    if (type)
        MyAssertHandler(
            ".\\cgame_mp\\cg_newDraw_mp.cpp",
            1208,
            0,
            "%s",
            "type >= 0 && static_cast<unsigned int>( type ) < ARRAY_COUNT( cgMedia.objectiveMaterials )");
    if (icon && CG_ServerMaterialName(localClientNum, icon, shaderName, 0x40u))
        return Material_RegisterHandle(shaderName, 7);
    else
        return cgMedia.objectiveMaterials[type];
}

const char *__cdecl CG_ScriptMainMenu(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return cgArray[0].scriptMainMenu;
}

void __cdecl CG_OwnerDraw(
    int localClientNum,
    rectDef_s parentRect,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float text_x,
    float text_y,
    int ownerDraw,
    int ownerDrawFlags,
    int align,
    float special,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle,
    char textAlignMode)
{
    OffhandClass offSecond; // [esp+20h] [ebp-24h]
    rectDef_s rect; // [esp+28h] [ebp-1Ch] BYREF
    const playerState_s *ps; // [esp+40h] [ebp-4h]

    ps = CG_GetPredictedPlayerState(localClientNum);
    if (!ps)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1748, 0, "%s", "ps");
    if (ps->offhandSecondary)
    {
        if (ps->offhandSecondary != PLAYER_OFFHAND_SECONDARY_FLASH)
            MyAssertHandler(
                ".\\cgame_mp\\cg_newDraw_mp.cpp",
                1756,
                0,
                "%s",
                "ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH");
        offSecond = OFFHAND_CLASS_FLASH_GRENADE;
    }
    else
    {
        offSecond = OFFHAND_CLASS_SMOKE_GRENADE;
    }
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    switch (ownerDraw)
    {
    case 5:
        CG_DrawPlayerAmmoValue(localClientNum, &rect, font, scale, color, material, textStyle);
        break;
    case 6:
        CG_DrawPlayerAmmoBackdrop(localClientNum, &rect, color, material);
        break;
    case 20:
        CG_DrawPlayerStance(localClientNum, &rect, color, font, scale, textStyle);
        break;
    case 71:
        CG_DrawHoldBreathHint(localClientNum, &rect, font, scale, textStyle);
        break;
    case 72:
        CG_DrawCursorhint(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 79:
        CG_DrawPlayerBarHealth(localClientNum, &rect, material, color);
        break;
    case 80:
        CG_DrawMantleHint(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 81:
        CG_DrawPlayerWeaponName(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 82:
        CG_DrawPlayerWeaponNameBack(localClientNum, &rect, font, scale, color, material);
        break;
    case 90:
        CG_DrawCenterString(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 98:
        CG_DrawPlayerBarHealthBack(localClientNum, &rect, material, color);
        break;
    case 103:
        CG_DrawOffHandIcon(localClientNum, &rect, scale, color, material, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 104:
        CG_DrawOffHandIcon(localClientNum, &rect, scale, color, material, offSecond);
        break;
    case 105:
        CG_DrawOffHandAmmo(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 106:
        CG_DrawOffHandAmmo(localClientNum, &rect, font, scale, color, textStyle, offSecond);
        break;
    case 107:
        CG_DrawOffHandName(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 108:
        CG_DrawOffHandName(localClientNum, &rect, font, scale, color, textStyle, offSecond);
        break;
    case 109:
        CG_DrawOffHandHighlight(localClientNum, &rect, scale, color, material, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 110:
        CG_DrawOffHandHighlight(localClientNum, &rect, scale, color, material, offSecond);
        break;
    case 112:
        CG_DrawPlayerLowHealthOverlay(localClientNum, &rect, material, color);
        break;
    case 113:
        CG_DrawInvalidCmdHint(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 114:
        CG_DrawPlayerSprintMeter(localClientNum, &rect, material, color);
        break;
    case 115:
        CG_DrawPlayerSprintBack(localClientNum, &rect, material, color);
        break;
    case 116:
        CG_DrawPlayerWeaponBackground(localClientNum, &rect, color, material);
        break;
    case 117:
        CG_DrawPlayerWeaponAmmoClipGraphic(localClientNum, &rect, color);
        break;
    case 118:
        CG_DrawPlayerWeaponIcon(localClientNum, &rect, color);
        break;
    case 119:
        CG_DrawPlayerWeaponAmmoStock(localClientNum, &rect, font, scale, color, material, textStyle);
        break;
    case 120:
        CG_DrawPlayerWeaponLowAmmoWarning(
            localClientNum,
            &rect,
            font,
            scale,
            textStyle,
            text_x,
            text_y,
            textAlignMode,
            material);
        break;
    case 145:
        CG_CompassDrawTickertape(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            &parentRect,
            &rect,
            material,
            color,
            font,
            scale,
            textStyle,
            1);
        break;
    case 146:
        CG_CompassDrawTickertape(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            &parentRect,
            &rect,
            material,
            color,
            font,
            scale,
            textStyle,
            0);
        break;
    case 150:
        CG_CompassDrawPlayer(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 151:
        CG_CompassDrawPlayerBack(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 152:
        CG_CompassDrawPlayerPointers_MP(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 155:
        CG_CompassDrawVehicles(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            12,
            &parentRect,
            &rect,
            cgMedia.compass_helicopter_enemy,
            cgMedia.compass_helicopter_friendly,
            color);
        break;
    case 156:
        CG_CompassDrawVehicles(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            13,
            &parentRect,
            &rect,
            cgMedia.compass_plane_enemy,
            cgMedia.compass_plane_friendly,
            color);
        break;
    case 158:
        CG_CompassDrawFriendlies(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
        break;
    case 159:
        CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 160:
        CG_CompassDrawPlayerNorthCoord(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            &parentRect,
            &rect,
            font,
            material,
            color,
            textStyle
        );
        break;
    case 161:
        CG_CompassDrawPlayerEastCoord(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            &parentRect,
            &rect,
            font,
            material,
            color,
            textStyle
        );
        break;
    case 162:
        CG_CompassDrawPlayerNCoordScroll(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            &parentRect,
            &rect,
            font,
            material,
            color,
            textStyle);
        break;
    case 163:
        CG_CompassDrawPlayerECoordScroll(
            localClientNum,
            COMPASS_TYPE_PARTIAL,
            &parentRect,
            &rect,
            font,
            material,
            color,
            textStyle);
        break;
    case 165:
        CG_DrawPlayerActionSlotDpad(localClientNum, &rect, color, material);
        break;
    case 166:
    case 167:
    case 168:
    case 169:
        CG_DrawPlayerActionSlot(localClientNum, &rect, ownerDraw - 166, color, font, scale, textStyle);
        break;
    case 170:
        CG_CompassDrawEnemies(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
        break;
    case 193:
    case 194:
    case 195:
    case 196:
        CG_DrawTalkerNum(localClientNum, ownerDraw - 193, &rect, font, color, scale, textStyle);
        break;
    default:
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (!cgArray[0].inKillCam)
        {
            switch (ownerDraw)
            {
            case 180:
                CG_CompassDrawPlayerBack(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                break;
            case 181:
                CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                break;
            case 182:
                CG_CompassDrawPlayerPointers_MP(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                break;
            case 183:
                CG_CompassDrawPlayer(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                break;
            case 185:
                CG_CompassDrawFriendlies(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                break;
            case 186:
                CG_CompassDrawPlayerMapLocationSelector(
                    localClientNum,
                    COMPASS_TYPE_FULL,
                    &parentRect,
                    &rect,
                    material,
                    color);
                break;
            case 187:
                CG_CompassDrawBorder(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                break;
            case 188:
                CG_CompassDrawEnemies(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                break;
            default:
                return;
            }
        }
        break;
    }
}

void __cdecl CG_DrawPlayerAmmoBackdrop(
    int localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material)
{
    float v4; // [esp+24h] [ebp-24h]
    float drawColor[4]; // [esp+38h] [ebp-10h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.weapon)
    {
        v4 = hud_fade_ammodisplay->current.value * 1000.0;
        drawColor[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_ammodisplay,
            cgArray[0].ammoFadeTime,
            (v4 + 9.313225746154785e-10));
        if (drawColor[3] != 0.0)
        {
            if (CG_CheckPlayerForLowAmmo(cgArray))
            {
                drawColor[0] = 0.88999999;
                drawColor[1] = 0.18000001;
                drawColor[2] = 0.0099999998;
            }
            else
            {
                drawColor[0] = *color;
                drawColor[1] = color[1];
                drawColor[2] = color[2];
            }
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                drawColor,
                material);
        }
    }
}

void __cdecl CG_DrawPlayerAmmoValue(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle)
{
    double v7; // [esp+2Ch] [ebp-274h]
    float v8; // [esp+3Ch] [ebp-264h]
    float ammoColor[5]; // [esp+4Ch] [ebp-254h] BYREF
    const ScreenPlacement *scrPlace; // [esp+60h] [ebp-240h]
    int ammoVal; // [esp+64h] [ebp-23Ch]
    cg_s *cgameGlob; // [esp+68h] [ebp-238h]
    bool drawAmmo; // [esp+6Fh] [ebp-231h]
    char clipString[260]; // [esp+70h] [ebp-230h] BYREF
    const centity_s *cent; // [esp+174h] [ebp-12Ch]
    char ammoString[256]; // [esp+178h] [ebp-128h] BYREF
    bool lowAmmo; // [esp+27Dh] [ebp-23h]
    bool lowClip; // [esp+27Eh] [ebp-22h]
    bool drawClip; // [esp+27Fh] [ebp-21h]
    float flashColor[4]; // [esp+280h] [ebp-20h] BYREF
    int weapIndex; // [esp+290h] [ebp-10h]
    const playerState_s *ps; // [esp+294h] [ebp-Ch]
    float x; // [esp+298h] [ebp-8h]
    int clipVal; // [esp+29Ch] [ebp-4h]

    LODWORD(ammoColor[4]) = 800;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    if (cgArray[0].predictedPlayerState.weapon)
    {
        v8 = hud_fade_ammodisplay->current.value * 1000.0;
        color[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_ammodisplay,
            cgameGlob->ammoFadeTime,
            (v8 + 9.313225746154785e-10));
        if (color[3] != 0.0)
        {
            if (!cgameGlob->nextSnap)
                MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 339, 0, "%s", "cgameGlob->nextSnap");
            cent = CG_GetEntity(localClientNum, cgameGlob->nextSnap->ps.clientNum);
            if ((cgameGlob->nextSnap->ps.otherFlags & 4) != 0 && cgameGlob->weaponSelect < BG_GetNumWeapons())
                weapIndex = cgameGlob->weaponSelect;
            else
                weapIndex = cent->nextState.weapon;
            if (weapIndex)
            {
                drawAmmo = 1;
                drawClip = 1;
                lowAmmo = 0;
                lowClip = 0;
                ps = &cgameGlob->predictedPlayerState;
                ammoVal = BG_GetTotalAmmoReserve(&cgameGlob->predictedPlayerState, weapIndex);
                if (BG_WeaponIsClipOnly(weapIndex))
                    clipVal = -1;
                else
                    clipVal = ps->ammoclip[BG_ClipForWeapon(weapIndex)];
                if (clipVal < 0)
                    drawClip = 0;
                if (clipVal > 999)
                    clipVal = 999;
                if (ammoVal < 0)
                    drawAmmo = 0;
                if (ammoVal > 999)
                    ammoVal = 999;
                if (drawClip)
                {
                    sprintf(clipString, "%2i", clipVal);
                    lowClip = CG_CheckPlayerForLowClip(cgameGlob);
                }
                if (drawAmmo)
                {
                    sprintf(ammoString, "%3i", ammoVal);
                    lowAmmo = CG_CheckPlayerForLowAmmo(cgameGlob);
                }
                if (lowClip)
                {
                    if (cgameGlob->lastClipFlashTime > cgameGlob->time || cgameGlob->lastClipFlashTime + 800 < cgameGlob->time)
                        cgameGlob->lastClipFlashTime = cgameGlob->time;
                    flashColor[0] = 0.88999999;
                    flashColor[1] = 0.18000001;
                    flashColor[2] = 0.0099999998;
                    flashColor[3] = (cgameGlob->lastClipFlashTime + 800 - cgameGlob->time) / 800.0;
                    if (flashColor[3] > color[3])
                        flashColor[3] = color[3];
                }
                if (lowAmmo)
                {
                    ammoColor[0] = 0.88999999;
                    ammoColor[1] = 0.18000001;
                    ammoColor[2] = 0.0099999998;
                }
                else
                {
                    ammoColor[0] = *color;
                    ammoColor[1] = color[1];
                    ammoColor[2] = color[2];
                }
                ammoColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                if (drawClip && drawAmmo)
                {
                    UI_DrawText(
                        scrPlace,
                        clipString,
                        0x7FFFFFFF,
                        font,
                        rect->x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        color,
                        textStyle);
                    if (lowClip)
                        UI_DrawText(
                            scrPlace,
                            clipString,
                            0x7FFFFFFF,
                            font,
                            rect->x,
                            rect->y,
                            rect->horzAlign,
                            rect->vertAlign,
                            scale,
                            flashColor,
                            textStyle);
                    v7 = rect->x + rect->w;
                    x = v7 - UI_TextWidth(ammoString, 0, font, scale);
                    UI_DrawText(
                        scrPlace,
                        ammoString,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        ammoColor,
                        textStyle);
                    x = (rect->w - UI_TextWidth("|", 0, font, scale)) * 0.5 + rect->x - 5.0;
                    UI_DrawText(
                        scrPlace,
                        "|",
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        ammoColor,
                        textStyle);
                }
                else if (drawClip)
                {
                    x = (rect->w - UI_TextWidth(clipString, 0, font, scale)) * 0.5 + rect->x;
                    UI_DrawText(
                        scrPlace,
                        clipString,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        color,
                        textStyle);
                    if (lowClip)
                        UI_DrawText(
                            scrPlace,
                            clipString,
                            0x7FFFFFFF,
                            font,
                            x,
                            rect->y,
                            rect->horzAlign,
                            rect->vertAlign,
                            scale,
                            flashColor,
                            textStyle);
                }
                else if (drawAmmo)
                {
                    x = (rect->w - UI_TextWidth(ammoString, 0, font, scale)) * 0.5 + rect->x;
                    UI_DrawText(
                        scrPlace,
                        ammoString,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        ammoColor,
                        textStyle);
                }
            }
        }
    }
}

void __cdecl CG_DrawPlayerWeaponName(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle)
{
    double v6; // [esp+20h] [ebp-34h]
    weaponInfo_s *weapInfo; // [esp+28h] [ebp-2Ch]
    float *fadeColor; // [esp+2Ch] [ebp-28h]
    char *string; // [esp+34h] [ebp-20h]
    float drawColor[4]; // [esp+38h] [ebp-1Ch] BYREF
    int weapIndex; // [esp+48h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4Ch] [ebp-8h]
    float x; // [esp+50h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    fadeColor = CG_FadeColor(cgArray[0].time, cgArray[0].weaponSelectTime, 1800, 700);
    if (fadeColor)
    {
        if ((cgArray[0].predictedPlayerState.weapFlags & 0x80) == 0)
        {
            drawColor[3] = fadeColor[3];
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            weapIndex = GetWeaponIndex(cgArray);
            if (weapIndex)
            {
                if (localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                        1095,
                        0,
                        "%s\n\t(localClientNum) = %i",
                        "(localClientNum == 0)",
                        localClientNum);
                weapInfo = &cg_weaponsArray[0][weapIndex];
                weapDef = BG_GetWeaponDef(weapIndex);
                if (*weapDef->szModeName)
                    string = va("%s / %s", weapInfo->translatedDisplayName, weapInfo->translatedModename);
                else
                    string = va("%s", weapInfo->translatedDisplayName);
                v6 = rect->x + rect->w;
                x = v6 - UI_TextWidth(string, 0, font, scale) - 28.0;
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    string,
                    0x7FFFFFFF,
                    font,
                    x,
                    rect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    scale,
                    drawColor,
                    textStyle);
            }
        }
    }
}

void __cdecl CG_DrawPlayerWeaponNameBack(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    Material *material)
{
    const weaponInfo_s *weapInfo; // [esp+24h] [ebp-2Ch]
    char *string; // [esp+2Ch] [ebp-24h]
    float drawColor[4]; // [esp+30h] [ebp-20h] BYREF
    int weapIndex; // [esp+40h] [ebp-10h]
    WeaponDef *weapDef; // [esp+44h] [ebp-Ch]
    float x; // [esp+48h] [ebp-8h]
    float w; // [esp+4Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    drawColor[3] = CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgArray[0].weaponSelectTime, 1800);
    if (drawColor[3] != 0.0)
    {
        drawColor[0] = *color;
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        weapIndex = GetWeaponIndex(cgArray);
        if (weapIndex)
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                    1095,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            weapInfo = &cg_weaponsArray[0][weapIndex];
            weapDef = BG_GetWeaponDef(weapIndex);
            if (*weapDef->szModeName)
                string = va("%s / %s", weapInfo->translatedDisplayName, weapInfo->translatedModename);
            else
                string = va("%s", weapInfo->translatedDisplayName);
            w = UI_TextWidth(string, 0, font, scale) + 28.0 + 8.0;
            x = rect->x + rect->w - w;
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                x,
                rect->y,
                w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                drawColor,
                material);
        }
    }
}

void __cdecl CG_DrawPlayerStance(
    int localClientNum,
    const rectDef_s *rect,
    const float *color,
    Font_s *font,
    float scale,
    int textStyle)
{
    float v6; // [esp+0h] [ebp-78h]
    float v7; // [esp+1Ch] [ebp-5Ch]
    float v8; // [esp+20h] [ebp-58h]
    float v9; // [esp+24h] [ebp-54h]
    float v10; // [esp+38h] [ebp-40h]
    float halfWidth; // [esp+4Ch] [ebp-2Ch]
    float drawColor[5]; // [esp+50h] [ebp-28h] BYREF
    float x; // [esp+64h] [ebp-14h]
    float y; // [esp+68h] [ebp-10h]
    const char *proneStr; // [esp+6Ch] [ebp-Ch]
    float fadeAlpha; // [esp+70h] [ebp-8h]
    float deltaTime; // [esp+74h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v10 = hud_fade_stance->current.value * 1000.0;
    fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_stance, cgArray[0].stanceFadeTime, (v10 + 9.313225746154785e-10));
    if (fadeAlpha != 0.0)
    {
        if (cg_hudStanceHintPrints->current.enabled)
        {
            if (cgArray[0].lastStance != (cgArray[0].predictedPlayerState.pm_flags & 3))
                cgArray[0].lastStanceChangeTime = cgArray[0].time;
        }
        else
        {
            cgArray[0].lastStanceChangeTime = 0;
        }
        cgArray[0].lastStance = cgArray[0].predictedPlayerState.pm_flags & 3;
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        drawColor[4] = 1.4025731e-38;
        x = (compassSize->current.value - 1.0) * cgsArray[0].compassWidth * 0.699999988079071 + rect->x;
        y = rect->y;
        CL_ResetStats_f();
        drawColor[0] = *color;
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        if ((cgArray[0].predictedPlayerState.pm_flags & 0x1000) != 0 && cgArray[0].proneBlockedEndTime < cgArray[0].time)
            cgArray[0].proneBlockedEndTime = cgArray[0].time + 1500;
        if (cgArray[0].proneBlockedEndTime > cgArray[0].time)
        {
            if (BG_WeaponBlocksProne(cgArray[0].predictedPlayerState.weapon))
                proneStr = UI_SafeTranslateString("CGAME_PRONE_BLOCKED_WEAPON");
            else
                proneStr = UI_SafeTranslateString("CGAME_PRONE_BLOCKED");
            halfWidth = UI_TextWidth(proneStr, 0, font, scale) * 0.5;
            deltaTime = (cgArray[0].proneBlockedEndTime - cgArray[0].time);
            v9 = deltaTime / 1500.0 * 540.0 * 0.01745329238474369;
            v8 = sin(v9);
            v7 = fabs(v8);
            drawColor[3] = v7;
            v6 = -halfWidth;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                proneStr,
                0x7FFFFFFF,
                font,
                v6,
                cg_hudProneY->current.value,
                7,
                3,
                scale,
                drawColor,
                textStyle);
        }
        if (cg_hudStanceHintPrints->current.enabled && cgArray[0].lastStanceChangeTime + 3000 > cgArray[0].time)
            CG_DrawStanceHintPrints(localClientNum, rect, x, color, fadeAlpha, font, scale, textStyle);
        drawColor[3] = color[3] * fadeAlpha;
        CG_DrawStanceIcon(localClientNum, rect, drawColor, x, y, fadeAlpha);
    }
}

void __cdecl CG_DrawStanceIcon(
    int localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    float x,
    float y,
    float fadeAlpha)
{
    Material *icon; // [esp+24h] [ebp-10h]
    float width; // [esp+2Ch] [ebp-8h]
    float height; // [esp+30h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 601, 0, "%s", "rect");
    width = rect->w;
    height = rect->h;
    CL_ResetStats_f();
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].lastStance & 1) != 0)
    {
        icon = cgMedia.stanceMaterials[2];
    }
    else if ((cgArray[0].lastStance & 2) != 0)
    {
        icon = cgMedia.stanceMaterials[1];
    }
    else
    {
        icon = cgMedia.stanceMaterials[0];
    }
    UI_DrawHandlePic(
        &scrPlaceView[localClientNum],
        x,
        y,
        width,
        height,
        rect->horzAlign,
        rect->vertAlign,
        drawColor,
        icon);
    if (cgArray[0].lastStanceChangeTime + 1000 > cgArray[0].time)
    {
        Dvar_GetUnpackedColor(cg_hudStanceFlash, drawColor);
        drawColor[3] = (cgArray[0].lastStanceChangeTime + 1000 - cgArray[0].time) / 1000.0 * 0.800000011920929;
        if (drawColor[3] > fadeAlpha)
            drawColor[3] = fadeAlpha;
        UI_DrawHandlePic(
            &scrPlaceView[localClientNum],
            x,
            y,
            width,
            height,
            rect->horzAlign,
            rect->vertAlign,
            drawColor,
            cgMedia.stanceMaterials[3]);
    }
}

void __cdecl CG_DrawStanceHintPrints(
    int localClientNum,
    const rectDef_s *rect,
    float x,
    const float *color,
    float fadeAlpha,
    Font_s *font,
    float scale,
    int textStyle)
{
    float v8; // [esp+1Ch] [ebp-240h]
    char keyBinding[256]; // [esp+2Ch] [ebp-230h] BYREF
    int j; // [esp+130h] [ebp-12Ch]
    const cg_s *cgameGlob; // [esp+134h] [ebp-128h]
    const char *string; // [esp+138h] [ebp-124h]
    const char *proneCmds[3][6]; // [esp+13Ch] [ebp-120h] BYREF
    float height; // [esp+188h] [ebp-D4h]
    float drawColor[4]; // [esp+18Ch] [ebp-D0h] BYREF
    const char *duckCmds[3][6]; // [esp+19Ch] [ebp-C0h] BYREF
    const char *hintLineCmds[3]; // [esp+1E8h] [ebp-74h]
    const char *standCmds[3][6]; // [esp+1F4h] [ebp-68h] BYREF
    const char *hintTypeStrings[3]; // [esp+240h] [ebp-1Ch]
    int numHintLines; // [esp+24Ch] [ebp-10h]
    int i; // [esp+250h] [ebp-Ch]
    const char *binding; // [esp+254h] [ebp-8h]
    float y; // [esp+258h] [ebp-4h]

    memset(standCmds, 0, 24);
    standCmds[1][0] = "gocrouch";
    standCmds[1][1] = "togglecrouch";
    standCmds[1][2] = "lowerstance";
    standCmds[1][3] = "+movedown";
    *&standCmds[1][4] = 0;
    standCmds[2][0] = "goprone";
    standCmds[2][1] = "+prone";
    *&standCmds[2][2] = 0;
    *&standCmds[2][4] = 0;
    duckCmds[0][0] = "+gostand";
    duckCmds[0][1] = "raisestance";
    duckCmds[0][2] = "+moveup";
    *&duckCmds[0][3] = 0;
    duckCmds[0][5] = 0;
    memset(duckCmds[1], 0, sizeof(const char *[6]));
    duckCmds[2][0] = "goprone";
    duckCmds[2][1] = "lowerstance";
    duckCmds[2][2] = "toggleprone";
    duckCmds[2][3] = "+prone";
    *&duckCmds[2][4] = 0;
    proneCmds[0][0] = "+gostand";
    proneCmds[0][1] = "toggleprone";
    *&proneCmds[0][2] = 0;
    *&proneCmds[0][4] = 0;
    proneCmds[1][0] = "gocrouch";
    proneCmds[1][1] = "togglecrouch";
    proneCmds[1][2] = "raisestance";
    proneCmds[1][3] = "+movedown";
    proneCmds[1][4] = "+moveup";
    proneCmds[1][5] = 0;
    memset(proneCmds[2], 0, sizeof(const char *[6]));
    hintTypeStrings[0] = "PLATFORM_STANCEHINT_STAND";
    hintTypeStrings[1] = "PLATFORM_STANCEHINT_CROUCH";
    hintTypeStrings[2] = "PLATFORM_STANCEHINT_PRONE";
    drawColor[0] = *color;
    drawColor[1] = color[1];
    drawColor[2] = color[2];
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    if (cgArray[0].lastStanceChangeTime + 3000 - cgArray[0].time <= 1000)
        drawColor[3] = (cgameGlob->lastStanceChangeTime + 3000 - cgameGlob->time) * 0.001000000047497451;
    else
        drawColor[3] = 1.0;
    height = UI_TextHeight(font, scale);
    numHintLines = 0;
    for (i = 0; i < 3; ++i)
    {
        hintLineCmds[i] = 0;
        j = 0;
        if ((cgameGlob->lastStance & 1) != 0)
        {
            binding = proneCmds[i][j];
        }
        else if ((cgameGlob->lastStance & 2) != 0)
        {
            binding = duckCmds[i][j];
        }
        else
        {
            binding = standCmds[i][j];
        }
        while (j < 6 && binding)
        {
            if (Key_IsCommandBound(localClientNum, binding))
            {
                hintLineCmds[i] = binding;
                ++numHintLines;
                break;
            }
            ++j;
            if ((cgameGlob->lastStance & 1) != 0)
            {
                binding = proneCmds[i][j];
            }
            else if ((cgameGlob->lastStance & 2) != 0)
            {
                binding = duckCmds[i][j];
            }
            else
            {
                binding = standCmds[i][j];
            }
        }
    }
    y = rect->h * 0.5 + rect->y - 1.5;
    if (numHintLines == 1)
    {
        y = height * 0.5 + y;
    }
    else if (numHintLines == 3)
    {
        y = y - (height * 0.5 + 1.5);
    }
    if (drawColor[3] > fadeAlpha)
        drawColor[3] = fadeAlpha;
    for (i = 0; i < 3; ++i)
    {
        if (hintLineCmds[i])
        {
            UI_GetKeyBindingLocalizedString(localClientNum, hintLineCmds[i], keyBinding);
            string = UI_SafeTranslateString(hintTypeStrings[i]);
            string = UI_ReplaceConversionString((char*)string, keyBinding);
            v8 = x + rect->w;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                string,
                0x7FFFFFFF,
                font,
                v8,
                y,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                drawColor,
                textStyle);
            y = height + 1.5 + y;
        }
    }
}

void __cdecl CG_DrawPlayerSprintBack(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+34h] [ebp-28h]
    float drawColor[4]; // [esp+48h] [ebp-14h] BYREF
    float fadeAlpha; // [esp+58h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v4 = hud_fade_sprint->current.value * 1000.0;
    fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgArray[0].sprintFadeTime, (v4 + 9.313225746154785e-10));
    if (fadeAlpha != 0.0)
    {
        drawColor[0] = *color;
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        drawColor[3] = color[3] * fadeAlpha;
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            drawColor,
            material);
    }
}

void __cdecl CG_DrawPlayerSprintMeter(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+34h] [ebp-48h]
    float sprint; // [esp+44h] [ebp-38h]
    float drawColor[4]; // [esp+4Ch] [ebp-30h] BYREF
    int sprintLeft; // [esp+5Ch] [ebp-20h]
    float x; // [esp+60h] [ebp-1Ch]
    float y; // [esp+64h] [ebp-18h]
    playerState_s *ps; // [esp+68h] [ebp-14h]
    int maxSprint; // [esp+6Ch] [ebp-10h]
    float h; // [esp+70h] [ebp-Ch]
    float fadeAlpha; // [esp+74h] [ebp-8h]
    float w; // [esp+78h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ps = &cgArray[0].predictedPlayerState;
    v4 = hud_fade_sprint->current.value * 1000.0;
    fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgArray[0].sprintFadeTime, (v4 + 9.313225746154785e-10));
    if (fadeAlpha != 0.0)
    {
        sprintLeft = PM_GetSprintLeft(ps, cgArray[0].time);
        maxSprint = BG_GetMaxSprintTime(ps);
        sprint = sprintLeft / maxSprint;
        if (sprint > 0.0)
        {
            x = rect->x;
            y = rect->y;
            w = rect->w * sprint;
            h = rect->h;
            if (!material)
                material = cgMedia.whiteMaterial;
            CG_CalcPlayerSprintColor(cgArray, ps, color);
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            drawColor[3] = color[3] * fadeAlpha;
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
                sprint,
                1.0,
                drawColor,
                material);
        }
    }
}

void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, float *color)
{
    float frac; // [esp+8h] [ebp-18h]
    const DvarValue *p_current; // [esp+Ch] [ebp-14h]
    int sprintLeft; // [esp+18h] [ebp-8h]
    int maxSprint; // [esp+1Ch] [ebp-4h]

    maxSprint = BG_GetMaxSprintTime(ps);
    if (ps->pm_type == 7 || !maxSprint)
    {
        p_current = &cg_sprintMeterFullColor->current;
        *color = cg_sprintMeterFullColor->current.value;
        color[1] = p_current->vector[1];
        color[2] = p_current->vector[2];
    }
    else
    {
        if (PM_IsSprinting(ps))
            sprintLeft = PM_GetSprintLeft(ps, cgameGlob->time);
        else
            sprintLeft = PM_GetSprintLeftLastTime(ps);
        if (sprintLeft)
        {
            frac = (double)sprintLeft / (double)maxSprint;
            Vec4Lerp(&cg_sprintMeterEmptyColor->current.value, &cg_sprintMeterFullColor->current.value, frac, color);
        }
        else
        {
            *(DvarValue *)color = cg_sprintMeterDisabledColor->current;
        }
    }
}

void __cdecl CG_DrawPlayerBarHealth(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+30h] [ebp-4Ch]
    float v5; // [esp+34h] [ebp-48h]
    float v6; // [esp+38h] [ebp-44h]
    float v7; // [esp+3Ch] [ebp-40h]
    float v8; // [esp+44h] [ebp-38h]
    float health; // [esp+58h] [ebp-24h]
    playerState_s *ps; // [esp+64h] [ebp-18h]
    float x; // [esp+68h] [ebp-14h]
    float xa; // [esp+68h] [ebp-14h]
    float y; // [esp+6Ch] [ebp-10h]
    float ya; // [esp+6Ch] [ebp-10h]
    float h; // [esp+70h] [ebp-Ch]
    float ha; // [esp+70h] [ebp-Ch]
    float w; // [esp+78h] [ebp-4h]
    float wa; // [esp+78h] [ebp-4h]

    if (cg_drawHealth->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        health = CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps);
        v8 = hud_fade_healthbar->current.value * 1000.0;
        color[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_healthbar,
            cgArray[0].healthFadeTime,
            (v8 + 9.313225746154785e-10));
        if (color[3] != 0.0)
        {
            ps = &cgArray[0].nextSnap->ps;
            v6 = health - 1.0;
            if (v6 < 0.0)
                v7 = health;
            else
                v7 = 1.0;
            v5 = 0.0 - health;
            if (v5 < 0.0)
                v4 = v7;
            else
                v4 = 0.0;
            if (health > 0.0)
            {
                x = rect->x;
                y = rect->y;
                w = rect->w * health;
                h = rect->h;
                if (v4 <= 0.5)
                {
                    color[1] = (v4 + 0.2000000029802322) * color[1];
                    color[1] = color[1] + 0.300000011920929;
                }
                else
                {
                    *color = (1.0 - v4 + 1.0 - v4) * *color;
                    color[2] = (1.0 - v4 + 1.0 - v4) * color[2];
                }
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
                    health,
                    1.0,
                    color,
                    material);
            }
            if (cgArray[0].lastHealthClient == ps->clientNum)
            {
                if (cgArray[0].lastHealth <= health)
                {
                    cgArray[0].lastHealth = health;
                    cgArray[0].lastHealthLerpDelay = 1;
                }
                else if (cgArray[0].lastHealthLerpDelay)
                {
                    cgArray[0].lastHealthLerpDelay -= cgArray[0].frametime;
                    if (cgArray[0].lastHealthLerpDelay < 0)
                        cgArray[0].lastHealthLerpDelay = 0;
                }
                else
                {
                    cgArray[0].lastHealth = cgArray[0].lastHealth - cgArray[0].frametime * 0.0012000001;
                    if (health >= cgArray[0].lastHealth)
                    {
                        cgArray[0].lastHealth = health;
                        cgArray[0].lastHealthLerpDelay = 1;
                    }
                }
            }
            else
            {
                cgArray[0].lastHealthClient = ps->clientNum;
                cgArray[0].lastHealth = health;
                cgArray[0].lastHealthLerpDelay = 1;
            }
            if (health < cgArray[0].lastHealth)
            {
                xa = rect->w * health + rect->x;
                ya = rect->y;
                wa = (cgArray[0].lastHealth - health) * rect->w;
                ha = rect->h;
                *color = 1.0;
                color[1] = 0.0;
                color[2] = 0.0;
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    xa,
                    ya,
                    wa,
                    ha,
                    rect->horzAlign,
                    rect->vertAlign,
                    health,
                    0.0,
                    cgArray[0].lastHealth,
                    1.0,
                    color,
                    material);
            }
        }
    }
}

void __cdecl CG_DrawPlayerBarHealthBack(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+34h] [ebp-54h]
    float v5; // [esp+44h] [ebp-44h]
    float v6; // [esp+58h] [ebp-30h]
    int flashTime; // [esp+68h] [ebp-20h]
    float health; // [esp+70h] [ebp-18h]
    float x; // [esp+74h] [ebp-14h]
    float y; // [esp+78h] [ebp-10h]
    float h; // [esp+7Ch] [ebp-Ch]
    float fadeAlpha; // [esp+80h] [ebp-8h]
    float w; // [esp+84h] [ebp-4h]

    if (cg_drawHealth->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        v6 = hud_fade_healthbar->current.value * 1000.0;
        fadeAlpha = CG_FadeHudMenu(
            localClientNum,
            hud_fade_healthbar,
            cgArray[0].healthFadeTime,
            (v6 + 9.313225746154785e-10));
        if (fadeAlpha != 0.0)
        {
            color[3] = fadeAlpha;
            x = rect->x;
            y = rect->y;
            w = rect->w;
            h = rect->h;
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
            health = CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps);
            if (health != 0.0)
            {
                if (hud_health_startpulse_critical->current.value <= health)
                {
                    if (hud_health_startpulse_injured->current.value <= health)
                    {
                        flashTime = 0;
                    }
                    else
                    {
                        v4 = hud_health_pulserate_injured->current.value * 1000.0;
                        flashTime = (v4 + 9.313225746154785e-10);
                    }
                }
                else
                {
                    v5 = hud_health_pulserate_critical->current.value * 1000.0;
                    flashTime = (v5 + 9.313225746154785e-10);
                }
                if (flashTime)
                {
                    if (cgArray[0].lastHealthPulseTime > cgArray[0].time
                        || flashTime + cgArray[0].lastHealthPulseTime < cgArray[0].time)
                    {
                        cgArray[0].lastHealthPulseTime = cgArray[0].time;
                    }
                    *color = 0.88999999;
                    color[1] = 0.18000001;
                    color[2] = 0.0099999998;
                    color[3] = (flashTime + cgArray[0].lastHealthPulseTime - cgArray[0].time) / flashTime;
                    if (color[3] > fadeAlpha)
                        color[3] = fadeAlpha;
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
        }
    }
}

void __cdecl CG_DrawPlayerLowHealthOverlay(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float healthRatio; // [esp+34h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    healthRatio = CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps);
    if (healthRatio != 0.0)
    {
        CG_PulseLowHealthOverlay(cgArray, healthRatio);
        color[3] = CG_FadeLowHealthOverlay(cgArray);
        if (color[3] != 0.0)
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
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

double __cdecl CG_FadeLowHealthOverlay(const cg_s *cgameGlob)
{
    float curAlpha; // [esp+8h] [ebp-Ch]
    float lerp; // [esp+Ch] [ebp-8h]
    int timeSinceFadeStarted; // [esp+10h] [ebp-4h]

    timeSinceFadeStarted = cgameGlob->time - cgameGlob->healthOverlayPulseTime;
    if (timeSinceFadeStarted < 0)
        timeSinceFadeStarted = 0;
    if (cgameGlob->healthOverlayPulseDuration <= 0 || timeSinceFadeStarted >= cgameGlob->healthOverlayPulseDuration)
    {
        curAlpha = cgameGlob->healthOverlayToAlpha;
    }
    else
    {
        if (!cgameGlob->healthOverlayPulseDuration)
            MyAssertHandler(
                ".\\cgame_mp\\cg_newDraw_mp.cpp",
                1067,
                0,
                "%s\n\t(cgameGlob->healthOverlayPulseDuration) = %i",
                "(cgameGlob->healthOverlayPulseDuration)",
                cgameGlob->healthOverlayPulseDuration);
        lerp = (double)timeSinceFadeStarted / (double)cgameGlob->healthOverlayPulseDuration;
        curAlpha = (cgameGlob->healthOverlayToAlpha - cgameGlob->healthOverlayFromAlpha) * lerp
            + cgameGlob->healthOverlayFromAlpha;
    }
    if (curAlpha < 0.0 || curAlpha > 1.0)
        MyAssertHandler(
            ".\\cgame_mp\\cg_newDraw_mp.cpp",
            1076,
            0,
            "%s\n\t(curAlpha) = %g",
            "(curAlpha >= 0.0f && curAlpha <= 1.0f)",
            curAlpha);
    return curAlpha;
}

void __cdecl CG_PulseLowHealthOverlay(cg_s *cgameGlob, float healthRatio)
{
    const char *v2; // eax
    float v3; // [esp+0h] [ebp-58h]
    float v4; // [esp+4h] [ebp-54h]
    float v5; // [esp+8h] [ebp-50h]
    float v6; // [esp+Ch] [ebp-4Ch]
    float v7; // [esp+10h] [ebp-48h]
    float v8; // [esp+14h] [ebp-44h]
    float v9; // [esp+18h] [ebp-40h]
    float v10; // [esp+1Ch] [ebp-3Ch]
    float v11; // [esp+20h] [ebp-38h]
    int healthOverlayPulsePhase; // [esp+24h] [ebp-34h]
    float v13; // [esp+28h] [ebp-30h]
    float v14; // [esp+2Ch] [ebp-2Ch]
    float v15; // [esp+30h] [ebp-28h]
    float v16; // [esp+34h] [ebp-24h]
    float v17; // [esp+38h] [ebp-20h]
    float v18; // [esp+3Ch] [ebp-1Ch]
    float pulseMags[4]; // [esp+48h] [ebp-10h]

    pulseMags[0] = 1.0;
    pulseMags[1] = 0.80000001;
    pulseMags[2] = 0.60000002;
    pulseMags[3] = 0.30000001;
    if (cgameGlob->healthOverlayOldHealth > (double)healthRatio
        && hud_healthOverlay_pulseStart->current.value > (double)healthRatio)
    {
        cgameGlob->healthOverlayLastHitTime = cgameGlob->time;
        cgameGlob->healthOverlayPulseIndex = 0;
    }
    cgameGlob->healthOverlayOldHealth = healthRatio;
    if (cgameGlob->healthOverlayPulseDuration + cgameGlob->healthOverlayPulseTime <= cgameGlob->time
        && (hud_healthOverlay_pulseStart->current.value > (double)healthRatio || cgameGlob->healthOverlayHurt))
    {
        if (!cgameGlob->healthOverlayHurt)
            cgameGlob->healthOverlayHurt = 1;
        cgameGlob->healthOverlayPulseTime = cgameGlob->time;
        cgameGlob->healthOverlayFromAlpha = cgameGlob->healthOverlayToAlpha;
        if (cgameGlob->healthOverlayPulseIndex >= 4u)
        {
            cgameGlob->healthOverlayHurt = 0;
            cgameGlob->healthOverlayToAlpha = hud_healthOverlay_phaseEnd_toAlpha->current.value;
            cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseEnd_pulseDuration->current.integer;
            cgameGlob->healthOverlayPulsePhase = 0;
        }
        else
        {
            healthOverlayPulsePhase = cgameGlob->healthOverlayPulsePhase;
            if (healthOverlayPulsePhase)
            {
                if (healthOverlayPulsePhase == 1)
                {
                    v15 = hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    v8 = v15 - 1.0;
                    if (v8 < 0.0)
                        v16 = hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    else
                        v16 = 1.0;
                    v7 = 0.0 - v15;
                    if (v7 < 0.0)
                        v6 = v16;
                    else
                        v6 = 0.0;
                    cgameGlob->healthOverlayToAlpha = v6;
                    cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseTwo_pulseDuration->current.integer;
                    ++cgameGlob->healthOverlayPulsePhase;
                }
                else if (healthOverlayPulsePhase == 2)
                {
                    v13 = hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    v5 = v13 - 1.0;
                    if (v5 < 0.0)
                        v14 = hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    else
                        v14 = 1.0;
                    v4 = 0.0 - v13;
                    if (v4 < 0.0)
                        v3 = v14;
                    else
                        v3 = 0.0;
                    cgameGlob->healthOverlayToAlpha = v3;
                    cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseThree_pulseDuration->current.integer;
                    cgameGlob->healthOverlayPulsePhase = 0;
                    if (cgameGlob->time >= hud_healthOverlay_regenPauseTime->current.integer
                        + cgameGlob->healthOverlayLastHitTime
                        - 3
                        * (hud_healthOverlay_phaseThree_pulseDuration->current.integer
                            + hud_healthOverlay_phaseTwo_pulseDuration->current.integer
                            + hud_healthOverlay_phaseOne_pulseDuration->current.integer))
                        ++cgameGlob->healthOverlayPulseIndex;
                }
                else if (!alwaysfails)
                {
                    v2 = va("Invalid health overlay pulse phase: %i", cgameGlob->healthOverlayPulsePhase);
                    MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1134, 0, v2);
                }
            }
            else
            {
                v17 = pulseMags[cgameGlob->healthOverlayPulseIndex];
                v11 = v17 - 1.0;
                if (v11 < 0.0)
                    v18 = v17;
                else
                    v18 = 1.0;
                v10 = 0.0 - v17;
                if (v10 < 0.0)
                    v9 = v18;
                else
                    v9 = 0.0;
                cgameGlob->healthOverlayToAlpha = v9;
                cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseOne_pulseDuration->current.integer;
                ++cgameGlob->healthOverlayPulsePhase;
            }
        }
    }
}

void __cdecl CG_DrawCursorhint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int textStyle)
{
    double v6; // st7
    char *v7; // eax
    char *v8; // eax
    int h; // [esp+Ch] [ebp-1C4h]
    int vertAlign; // [esp+10h] [ebp-1C0h]
    float v11; // [esp+20h] [ebp-1B0h]
    float v12; // [esp+24h] [ebp-1ACh]
    float v13; // [esp+28h] [ebp-1A8h]
    float v14; // [esp+2Ch] [ebp-1A4h]
    float v15; // [esp+30h] [ebp-1A0h]
    float v16; // [esp+34h] [ebp-19Ch]
    float v17; // [esp+38h] [ebp-198h]
    float w; // [esp+3Ch] [ebp-194h]
    float v19; // [esp+40h] [ebp-190h]
    float v20; // [esp+44h] [ebp-18Ch]
    float v21; // [esp+48h] [ebp-188h]
    float v22; // [esp+4Ch] [ebp-184h]
    weaponIconRatioType_t hudIconRatio; // [esp+5Ch] [ebp-174h]
    float v24; // [esp+60h] [ebp-170h]
    float v25; // [esp+6Ch] [ebp-164h]
    float v26; // [esp+74h] [ebp-15Ch]
    float v27; // [esp+78h] [ebp-158h]
    float v28; // [esp+80h] [ebp-150h]
    float secondaryLength; // [esp+84h] [ebp-14Ch]
    Material *hintIcon; // [esp+8Ch] [ebp-144h]
    ScreenPlacement *scrPlace; // [esp+90h] [ebp-140h]
    float heightScale; // [esp+98h] [ebp-138h]
    float heighta; // [esp+9Ch] [ebp-134h]
    float height; // [esp+9Ch] [ebp-134h]
    float halfscale; // [esp+A0h] [ebp-130h]
    unsigned int weaponIndex; // [esp+A4h] [ebp-12Ch]
    char *displayString; // [esp+A8h] [ebp-128h]
    char *displayStringa; // [esp+A8h] [ebp-128h]
    float scale; // [esp+ACh] [ebp-124h]
    char binding[256]; // [esp+B0h] [ebp-120h] BYREF
    float widthOfs; // [esp+1B4h] [ebp-1Ch]
    float length; // [esp+1B8h] [ebp-18h]
    float x; // [esp+1BCh] [ebp-14h]
    float y; // [esp+1C0h] [ebp-10h]
    WeaponDef *weapDef; // [esp+1C4h] [ebp-Ch]
    const char *secondaryString; // [esp+1C8h] [ebp-8h] BYREF
    float widthScale; // [esp+1CCh] [ebp-4h]

    if (cg_cursorHints->current.integer)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        CG_UpdateCursorHints(cgArray);
        color[3] = CG_FadeAlpha(cgArray[0].time, cgArray[0].cursorHintTime, cgArray[0].cursorHintFade, 100) * color[3];
        if (color[3] == 0.0)
        {
            cgArray[0].cursorHintIcon = 0;
        }
        else
        {
            widthScale = 1.0;
            widthOfs = 0.0;
            heightScale = 1.0;
            displayString = 0;
            secondaryString = 0;
            if (cg_cursorHints->current.integer == 3)
            {
                v28 = cgArray[0].time / 150.0;
                v26 = sin(v28);
                color[3] = (v26 * 0.5 + 0.5) * color[3];
            }
            if (cg_cursorHints->current.integer < 3)
            {
                if (cg_cursorHints->current.integer == 2)
                {
                    v6 = (cgArray[0].cursorHintTime % 1000) / 100.0;
                }
                else
                {
                    v27 = cgArray[0].time / 150.0;
                    v25 = sin(v27);
                    v6 = (v25 * 0.5 + 0.5) * 10.0;
                }
                scale = v6;
                halfscale = scale * 0.5;
            }
            else
            {
                halfscale = 0.0;
                scale = 0.0;
            }
            if (cgArray[0].cursorHintIcon == 1)
            {
                if (cgArray[0].cursorHintString >= 0)
                {
                    displayStringa = CG_GetUseString(localClientNum);
                    if (displayStringa)
                    {
                        if (*displayStringa)
                        {
                            length = UI_TextWidth(displayStringa, 0, font, fontscale);
                            heighta = UI_TextHeight(font, fontscale);
                            x = (scale + length) * -0.5;
                            y = rect->y - rect->h * 0.5;
                            v24 = heighta * 0.5 + rect->y;
                            UI_DrawText(
                                &scrPlaceView[localClientNum],
                                displayStringa,
                                0x7FFFFFFF,
                                font,
                                x,
                                v24,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                        }
                    }
                }
            }
            else
            {
                hintIcon = cgMedia.hintMaterials[cgArray[0].cursorHintIcon];
                if (hintIcon)
                {
                    if (cgArray[0].cursorHintIcon < 5 || cgArray[0].cursorHintIcon > 132)
                    {
                        if (cgArray[0].cursorHintString < 0)
                        {
                            if (cgArray[0].cursorHintIcon == 3)
                            {
                                UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding);
                                v7 = UI_SafeTranslateString("PLATFORM_PICKUPHEALTH");
                                displayString = UI_ReplaceConversionString(v7, binding);
                            }
                        }
                        else
                        {
                            displayString = CG_GetUseString(localClientNum);
                        }
                    }
                    else
                    {
                        weaponIndex = cgArray[0].cursorHintIcon - 4;
                        weapDef = BG_GetWeaponDef(weaponIndex);
                        if (weapDef->hudIcon)
                        {
                            hudIconRatio = weapDef->hudIconRatio;
                            if (hudIconRatio)
                            {
                                if (hudIconRatio == WEAPON_ICON_RATIO_2TO1)
                                {
                                    widthScale = 2.0;
                                    widthOfs = rect->w * -0.5;
                                    heightScale = 1.0;
                                }
                                else
                                {
                                    if (weapDef->hudIconRatio != WEAPON_ICON_RATIO_4TO1)
                                        MyAssertHandler(
                                            ".\\cgame_mp\\cg_newDraw_mp.cpp",
                                            1440,
                                            0,
                                            "%s",
                                            "weapDef->hudIconRatio == WEAPON_ICON_RATIO_4TO1");
                                    widthScale = 2.0;
                                    widthOfs = rect->w * -0.5;
                                    heightScale = 0.5;
                                }
                            }
                        }
                        if (weapDef->weapClass == WEAPCLASS_TURRET)
                        {
                            if (cgArray[0].cursorHintString >= 0)
                                displayString = CG_GetUseString(localClientNum);
                            if (localClientNum)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                                    1095,
                                    0,
                                    "%s\n\t(localClientNum) = %i",
                                    "(localClientNum == 0)",
                                    localClientNum);
                            secondaryString = cg_weaponsArray[0][weaponIndex].translatedDisplayName;
                        }
                        else
                        {
                            displayString = CG_GetWeaponUseString(localClientNum, &secondaryString);
                        }
                    }
                    scrPlace = &scrPlaceView[localClientNum];
                    if (displayString && *displayString)
                    {
                        length = UI_TextWidth(displayString, 0, font, fontscale);
                        height = UI_TextHeight(font, fontscale);
                        if (secondaryString && cg_weaponHintsCoD1Style->current.enabled)
                        {
                            secondaryLength = UI_TextWidth(secondaryString, 0, font, fontscale);
                            x = (length + secondaryLength) * -0.5;
                            y = rect->y - rect->h * 0.5 * heightScale;
                            v22 = height * 0.5 + rect->y;
                            UI_DrawText(
                                scrPlace,
                                displayString,
                                0x7FFFFFFF,
                                font,
                                x,
                                v22,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                            vertAlign = rect->vertAlign;
                            h = rect->horzAlign;
                            v21 = height * 0.5 + rect->y;
                            v20 = x + length;
                            v8 = va(" %s", secondaryString);
                            UI_DrawText(scrPlace, v8, 0x7FFFFFFF, font, v20, v21, h, vertAlign, fontscale, color, textStyle);
                            x = (rect->w * widthScale + scale) * -0.5;
                            v19 = rect->h * heightScale + scale;
                            w = rect->w * widthScale + scale;
                            v17 = height * 1.5 + y;
                            UI_DrawHandlePic(scrPlace, x, v17, w, v19, rect->horzAlign, rect->vertAlign, color, hintIcon);
                        }
                        else
                        {
                            x = (rect->w * widthScale + scale + length) * -0.5;
                            y = rect->y - rect->h * 0.5 * heightScale;
                            v16 = height * 0.5 + rect->y;
                            UI_DrawText(
                                scrPlace,
                                displayString,
                                0x7FFFFFFF,
                                font,
                                x,
                                v16,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                            v15 = rect->h * heightScale + scale;
                            v14 = rect->w * widthScale + scale;
                            v13 = x + length;
                            UI_DrawHandlePic(scrPlace, v13, y, v14, v15, rect->horzAlign, rect->vertAlign, color, hintIcon);
                        }
                    }
                    else
                    {
                        x = rect->x - (rect->w + halfscale + widthOfs) * 0.5;
                        y = rect->y - halfscale * heightScale;
                        v12 = rect->h * heightScale + scale;
                        v11 = rect->w * widthScale + scale;
                        UI_DrawHandlePic(scrPlace, x, y, v11, v12, rect->horzAlign, rect->vertAlign, color, hintIcon);
                    }
                }
            }
        }
    }
}

void __cdecl CG_UpdateCursorHints(cg_s *cgameGlob)
{
    if (!cgameGlob->renderingThirdPerson)
    {
        if (cgameGlob->predictedPlayerState.cursorHint)
        {
            cgameGlob->cursorHintTime = cgameGlob->time;
            cgameGlob->cursorHintFade = cg_hintFadeTime->current.integer;
            cgameGlob->cursorHintIcon = cgameGlob->predictedPlayerState.cursorHint;
            cgameGlob->cursorHintString = cgameGlob->predictedPlayerState.cursorHintString;
        }
    }
}

char *__cdecl CG_GetWeaponUseString(int localClientNum, const char **secondaryString)
{
    const weaponInfo_s *weapInfo; // [esp+0h] [ebp-120h]
    int weaponIndex; // [esp+8h] [ebp-118h]
    char *displayString; // [esp+Ch] [ebp-114h]
    char binding[260]; // [esp+10h] [ebp-110h] BYREF
    WeaponDef *weapDef; // [esp+118h] [ebp-8h]
    const playerState_s *ps; // [esp+11Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].cursorHintIcon < 5 || cgArray[0].cursorHintIcon > 132)
        MyAssertHandler(
            ".\\cgame_mp\\cg_newDraw_mp.cpp",
            1259,
            0,
            "%s",
            "(cgameGlob->cursorHintIcon >= FIRST_WEAPON_HINT) && (cgameGlob->cursorHintIcon <= LAST_WEAPON_HINT)");
    weaponIndex = cgArray[0].cursorHintIcon - 4;
    ps = &cgArray[0].predictedPlayerState;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1095,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    weapInfo = &cg_weaponsArray[0][weaponIndex];
    if (weapDef->inventoryType)
    {
        if (weapDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
        {
            displayString = UI_SafeTranslateString("PLATFORM_THROWBACKGRENADE");
            UI_GetKeyBindingLocalizedString(localClientNum, "+frag", binding);
        }
        else
        {
            displayString = UI_SafeTranslateString("PLATFORM_PICKUPNEWWEAPON");
            UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding);
            *secondaryString = weapInfo->translatedDisplayName;
        }
    }
    else
    {
        UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding);
        if (BG_PlayerWeaponCountPrimaryTypes(ps) >= 2)
            displayString = UI_SafeTranslateString("PLATFORM_SWAPWEAPONS");
        else
            displayString = UI_SafeTranslateString("PLATFORM_PICKUPNEWWEAPON");
        *secondaryString = weapInfo->translatedDisplayName;
    }
    return UI_ReplaceConversionString(displayString, binding);
}

char *__cdecl CG_GetUseString(int localClientNum)
{
    char *v2; // eax
    char *v3; // eax
    char *displayString; // [esp+4h] [ebp-10Ch]
    char binding[260]; // [esp+8h] [ebp-108h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].cursorHintString < 0)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1310, 0, "%s", "cgameGlob->cursorHintString >= 0");
    displayString = CL_GetConfigString(localClientNum, cgArray[0].cursorHintString + 277);
    if (!displayString || !*displayString)
        return 0;
    if (!UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding))
    {
        v2 = UI_SafeTranslateString("KEY_USE");
        I_strncpyz(binding, v2, 256);
    }
    v3 = SEH_LocalizeTextMessage(displayString, "Hint String", LOCMSG_SAFE);
    return UI_ReplaceConversionString(v3, binding);
}

void __cdecl CG_DrawHoldBreathHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    int textStyle)
{
    unsigned int ViewmodelWeaponIndex; // eax
    char *v6; // eax
    float v7; // [esp+24h] [ebp-124h]
    char *string; // [esp+34h] [ebp-114h]
    char binding[256]; // [esp+38h] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+13Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+140h] [ebp-8h]
    float x; // [esp+144h] [ebp-4h]

    if (cg_drawBreathHint->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        ps = &cgArray[0].predictedPlayerState;
        if ((cgArray[0].predictedPlayerState.weapFlags & 4) == 0)
        {
            ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
            weapDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
            if (weapDef->overlayReticle)
            {
                if (weapDef->weapClass != WEAPCLASS_ITEM && ps->fWeaponPosFrac == 1.0)
                {
                    if (!UI_GetKeyBindingLocalizedString(localClientNum, "+holdbreath", binding)
                        && !UI_GetKeyBindingLocalizedString(localClientNum, "+melee_breath", binding))
                    {
                        UI_GetKeyBindingLocalizedString(localClientNum, "+breath_sprint", binding);
                    }
                    v6 = UI_SafeTranslateString("PLATFORM_HOLD_BREATH");
                    string = UI_ReplaceConversionString(v6, binding);
                    v7 = UI_TextWidth(string, 0, font, fontscale) * 0.5;
                    x = rect->x - (v7 + 9.313225746154785e-10);
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        string,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        fontscale,
                        colorWhite,
                        textStyle);
                }
            }
        }
    }
}

void __cdecl CG_DrawMantleHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    const float *color,
    int textStyle)
{
    char *v6; // eax
    char *string; // [esp+28h] [ebp-120h]
    float height; // [esp+2Ch] [ebp-11Ch]
    char binding[260]; // [esp+30h] [ebp-118h] BYREF
    const playerState_s *ps; // [esp+138h] [ebp-10h]
    float length; // [esp+13Ch] [ebp-Ch]
    float x; // [esp+140h] [ebp-8h]
    float y; // [esp+144h] [ebp-4h]

    if (!cgMedia.mantleHint)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1563, 0, "%s", "cgMedia.mantleHint");
    if (cg_drawMantleHint->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        ps = &cgArray[0].predictedPlayerState;
        if ((cgArray[0].predictedPlayerState.mantleState.flags & 8) != 0)
        {
            if (!UI_GetKeyBindingLocalizedString(localClientNum, "+gostand", binding))
                UI_GetKeyBindingLocalizedString(localClientNum, "+moveup", binding);
            v6 = UI_SafeTranslateString("PLATFORM_MANTLE");
            string = UI_ReplaceConversionString(v6, binding);
            length = UI_TextWidth(string, 0, font, fontscale);
            height = UI_TextHeight(font, fontscale);
            x = rect->x - (rect->w + length) * 0.5;
            y = height * 0.5 + rect->y;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                string,
                0x7FFFFFFF,
                font,
                x,
                y,
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                color,
                textStyle);
            x = x + length;
            y = rect->y - rect->h * 0.5;
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                x,
                y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                color,
                cgMedia.mantleHint);
        }
    }
}

void __cdecl CG_DrawInvalidCmdHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int textStyle)
{
    float v6; // [esp+2Ch] [ebp-20h]
    char *string; // [esp+40h] [ebp-Ch]
    float x; // [esp+44h] [ebp-8h]
    int blinkInterval; // [esp+48h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1619, 0, "%s", "rect");
    if (!color)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1620, 0, "%s", "color");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cg_invalidCmdHintDuration->current.integer + cgArray[0].invalidCmdHintTime < cgArray[0].time)
        cgArray[0].invalidCmdHintType = INVALID_CMD_NONE;
    switch (cgArray[0].invalidCmdHintType)
    {
    case INVALID_CMD_NO_AMMO_BULLETS:
        string = UI_SafeTranslateString("WEAPON_NO_AMMO");
        goto LABEL_21;
    case INVALID_CMD_NO_AMMO_FRAG_GRENADE:
        string = UI_SafeTranslateString("WEAPON_NO_FRAG_GRENADE");
        goto LABEL_21;
    case INVALID_CMD_NO_AMMO_SPECIAL_GRENADE:
        string = UI_SafeTranslateString("WEAPON_NO_SPECIAL_GRENADE");
        goto LABEL_21;
    case INVALID_CMD_STAND_BLOCKED:
        string = UI_SafeTranslateString("GAME_STAND_BLOCKED");
        goto LABEL_21;
    case INVALID_CMD_CROUCH_BLOCKED:
        string = UI_SafeTranslateString("GAME_CROUCH_BLOCKED");
        goto LABEL_21;
    case INVALID_CMD_TARGET_TOO_CLOSE:
        string = UI_SafeTranslateString("WEAPON_TARGET_TOO_CLOSE");
        goto LABEL_21;
    case INVALID_CMD_LOCKON_REQUIRED:
        string = UI_SafeTranslateString("WEAPON_LOCKON_REQUIRED");
        goto LABEL_21;
    case INVALID_CMD_NOT_ENOUGH_CLEARANCE:
        string = UI_SafeTranslateString("WEAPON_TARGET_NOT_ENOUGH_CLEARANCE");
    LABEL_21:
        blinkInterval = cg_invalidCmdHintBlinkInterval->current.integer;
        if (blinkInterval <= 0)
            MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1667, 0, "%s", "blinkInterval > 0");
        color[3] = ((cgArray[0].time - cgArray[0].invalidCmdHintTime) % blinkInterval) / blinkInterval;
        v6 = UI_TextWidth(string, 0, font, fontscale) * 0.5;
        x = rect->x - (v6 + 9.313225746154785e-10);
        UI_DrawText(
            &scrPlaceView[localClientNum],
            string,
            0x7FFFFFFF,
            font,
            x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            fontscale,
            color,
            textStyle);
        break;
    default:
        if (cgArray[0].invalidCmdHintType)
            MyAssertHandler(
                ".\\cgame_mp\\cg_newDraw_mp.cpp",
                1629,
                0,
                "%s",
                "cgameGlob->invalidCmdHintType == INVALID_CMD_NONE");
        break;
    }
}

void __cdecl CG_DrawTalkerNum(
    int localClientNum,
    int num,
    rectDef_s *rect,
    Font_s *font,
    float *color,
    float textScale,
    int style)
{
    float v7; // [esp+20h] [ebp-44h]
    float v8; // [esp+24h] [ebp-40h]
    bool v9; // [esp+28h] [ebp-3Ch]
    team_t team; // [esp+34h] [ebp-30h]
    int client; // [esp+38h] [ebp-2Ch]
    Material *material; // [esp+3Ch] [ebp-28h]
    float textColor[4]; // [esp+44h] [ebp-20h] BYREF
    char *name; // [esp+54h] [ebp-10h]
    int drawTalk; // [esp+58h] [ebp-Ch]
    int textHeight; // [esp+5Ch] [ebp-8h]
    bool isEnemy; // [esp+63h] [ebp-1h]

    if (!color)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1692, 0, "%s", "color");
    if (!rect)
        MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1693, 0, "%s", "rect");
    drawTalk = cg_drawTalk->current.integer;
    if (drawTalk)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        client = UI_GetTalkerClientNum(localClientNum, num);
        if (client >= 0)
        {
            name = cgArray[0].bgs.clientinfo[client].name;
            if (cgArray[0].bgs.clientinfo[client].infoValid)
            {
                v9 = 0;
                if (cgArray[0].bgs.clientinfo[cgArray[0].clientNum].team)
                {
                    team = cgArray[0].bgs.clientinfo[cgArray[0].clientNum].team;
                    if (team == TEAM_FREE || team != cgArray[0].bgs.clientinfo[client].team)
                        v9 = 1;
                }
                isEnemy = v9;
                if ((drawTalk != 2 || !isEnemy) && (drawTalk != 3 || isEnemy))
                {
                    textColor[0] = *color;
                    textColor[1] = color[1];
                    textColor[2] = color[2];
                    textColor[3] = color[3];
                    if (cgArray[0].nextSnap->ps.pm_type != 5 && isEnemy && (cgArray[0].nextSnap->ps.perks & 0x200) != 0)
                    {
                        CG_RelativeTeamColor(client, "g_TeamColor", textColor, localClientNum);
                        material = Material_RegisterHandle((const char*)perk_parabolicIcon->current.integer, 7);
                    }
                    else
                    {
                        material = Material_RegisterHandle("voice_on", 7);
                    }
                    textHeight = UI_TextHeight(font, textScale);
                    UI_DrawHandlePic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        color,
                        material);
                    v8 = textHeight + rect->y + (rect->h - textHeight) / 2.0;
                    v7 = rect->x + rect->w + 2.0;
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        name,
                        32,
                        font,
                        v7,
                        v8,
                        rect->horzAlign,
                        rect->vertAlign,
                        textScale,
                        textColor,
                        style);
                }
            }
            else
            {
                Com_PrintWarning(13, "client %i has invalid info but they are talking\n", client);
            }
        }
    }
}

void __cdecl CG_ArchiveState(int localClientNum, MemoryFile *memFile)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].healthFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].ammoFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].stanceFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].compassFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].offhandFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].sprintFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].drawHud);
    MemFile_ArchiveData(memFile, 1024, cgArray[0].objectiveText);
    MemFile_ArchiveData(memFile, 256, cgArray[0].scriptMainMenu);
    MemFile_ArchiveData(memFile, 160, cgArray[0].visionSetFrom);
    MemFile_ArchiveData(memFile, 160, cgArray[0].visionSetTo);
    MemFile_ArchiveData(memFile, 160, cgArray[0].visionSetCurrent);
    MemFile_ArchiveData(memFile, 24, cgArray[0].visionSetLerpData);
    MemFile_ArchiveData(memFile, 64, cgArray[0].visionNameNaked);
    MemFile_ArchiveData(memFile, 64, cgArray[0].visionNameNight);
    MemFile_ArchiveData(memFile, 128, cgArray[0].hudElemSound);
}

