#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <bgame/bg_local.h>

#include <client/client.h>

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
    hud_fadeout_speed = Dvar_RegisterFloat("hud_fadeout_speed", 0.1f, 0.0f, 1.0f, DVAR_ARCHIVE, "The speed that the HUD will fade at");
    hud_enable = Dvar_RegisterBool("hud_enable", 1, DVAR_ARCHIVE, "Enable hud elements");
    hud_fade_ammodisplay = Dvar_RegisterFloat("hud_fade_ammodisplay", 0.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the ammo display to fade in seconds");
    hud_fade_healthbar = Dvar_RegisterFloat("hud_fade_healthbar", 2.0f, 0.0f, 30.0f, DVAR_ARCHIVE,"The time for the health bar to fade in seconds");
    hud_fade_compass = Dvar_RegisterFloat("hud_fade_compass", 0.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the compass to fade in seconds");
    hud_fade_stance = Dvar_RegisterFloat("hud_fade_stance", 1.7f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the stance to fade in seconds");
    hud_fade_offhand = Dvar_RegisterFloat("hud_fade_offhand", 0.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the offhand weapons to fade in seconds");
    hud_fade_sprint = Dvar_RegisterFloat("hud_fade_sprint", 1.7f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the sprint meter to fade in seconds");
    hud_health_startpulse_injured = Dvar_RegisterFloat("hud_health_startpulse_injured", 1.0f, 0.0f, 1.1f, DVAR_ARCHIVE, "The health level at which to start the 'injured' pulse effect");
    hud_health_startpulse_critical = Dvar_RegisterFloat("hud_health_startpulse_critical", 0.33f, 0.0f, 1.1f, DVAR_ARCHIVE, "The health level at which to start the 'critical' pulse effect");
    hud_health_pulserate_injured = Dvar_RegisterFloat("hud_health_pulserate_injured", 1.0f, 0.1f, 3.0f, DVAR_ARCHIVE, "The pulse rate of the 'injured' pulse effect");
    hud_health_pulserate_critical = Dvar_RegisterFloat("hud_health_pulserate_critical", 0.5f, 0.1f, 3.0f, DVAR_ARCHIVE, "The pulse rate of the 'critical' pulse effect");
    hud_deathQuoteFadeTime = Dvar_RegisterInt("hud_deathQuoteFadeTime", 1000, (DvarLimits)0x186A000000000LL, DVAR_ARCHIVE,"The time for the death quote to fade");
    hud_healthOverlay_regenPauseTime = Dvar_RegisterInt("hud_healthOverlay_regenPauseTime", 8000, (DvarLimits)0x271000000000LL, DVAR_CHEAT, "The time in milliseconds before the health regeneration kicks in");
    hud_healthOverlay_pulseStart = Dvar_RegisterFloat("hud_healthOverlay_pulseStart", 0.55f, 0.0f, 1.0f, DVAR_CHEAT, "The percentage of full health at which the low-health warning overlay begins flashing");
    hud_healthOverlay_phaseOne_pulseDuration = Dvar_RegisterInt("hud_healthOverlay_phaseOne_pulseDuration", 150, (DvarLimits)0x3E800000000LL, DVAR_CHEAT, "Time in milliseconds to ramp up to the first alpha value (the peak of the pulse)");
    hud_healthOverlay_phaseTwo_toAlphaMultiplier = Dvar_RegisterFloat("hud_healthOverlay_phaseTwo_toAlphaMultiplier", 0.7f, 0.0f, 1.0f, DVAR_CHEAT, "Alpha multiplier for the second health overlay phase (percentage of the pulse peak)");
    hud_healthOverlay_phaseTwo_pulseDuration = Dvar_RegisterInt("hud_healthOverlay_phaseTwo_pulseDuration", 320, (DvarLimits)0x3E800000000LL, DVAR_CHEAT, "Time in milliseconds to fade the alpha to hud_healthOverlay_phaseTwo_toAlphaMultiplier");
    hud_healthOverlay_phaseThree_toAlphaMultiplier = Dvar_RegisterFloat("hud_healthOverlay_phaseThree_toAlphaMultiplier", 0.6f, 0.0f, 1.0f, DVAR_CHEAT, "Alpha multiplier for the third health overlay phase (percentage of the pulse peak)");
    hud_healthOverlay_phaseThree_pulseDuration = Dvar_RegisterInt("hud_healthOverlay_phaseThree_pulseDuration", 400, (DvarLimits)0x3E800000000LL, DVAR_CHEAT, "Time in milliseconds to fade the alpha to hud_healthOverlay_phaseThree_" "toAlphaMultiplier");
    hud_healthOverlay_phaseEnd_toAlpha = Dvar_RegisterFloat("hud_healthOverlay_phaseEnd_toAlpha", 0.0f, 0.0f, 1.0f, DVAR_CHEAT, "Alpha multiplier to fade to before turning off the overlay (percentage of the pulse peak)");
    hud_healthOverlay_phaseEnd_pulseDuration = Dvar_RegisterInt("hud_healthOverlay_phaseEnd_pulseDuration", 700, (DvarLimits)0x3E800000000LL, DVAR_CHEAT, "Time in milliseconds to fade out the health overlay after it is done flashing");
    cg_sprintMeterFullColor = Dvar_RegisterVec4("cg_sprintMeterFullColor", 0.8f, 0.8f, 0.8f, 0.8f, 0.0f, 1.0f, DVAR_ARCHIVE, "The color of the sprint meter when the sprint meter is full");
    cg_sprintMeterEmptyColor = Dvar_RegisterVec4("cg_sprintMeterEmptyColor", 0.7f, 0.5f, 0.2f, 0.8f, 0.0f, 1.0f, DVAR_ARCHIVE, "The color of the sprint meter when the sprint meter is empty");
    cg_sprintMeterDisabledColor = Dvar_RegisterVec4("cg_sprintMeterDisabledColor", 0.8f, 0.1f, 0.1f, 0.2f, 0.0f, 1.0f, DVAR_ARCHIVE, "The color of the sprint meter when the sprint meter is disabled");
    cg_drawTalk = Dvar_RegisterEnum( "cg_drawTalk", cg_drawTalkNames, 1, DVAR_NOFLAG, "Controls which icons CG_TALKER ownerdraw draws");
}

bool __cdecl CG_ShouldDrawHud(int32_t localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cgameGlob && cgameGlob->drawHud && CL_ShouldDisplayHud(localClientNum);
}

double __cdecl CG_FadeHudMenu(int32_t localClientNum, const dvar_s *fadeDvar, int32_t displayStartTime, int32_t duration)
{
    float *fadeColor; // [esp+4h] [ebp-4h]

    if (!hud_enable->current.enabled)
        return 0.0;

    if (!CG_ShouldDrawHud(localClientNum))
        return 0.0;

    if (fadeDvar->current.value == 0.0)
        return 1.0;

    fadeColor = CG_FadeColor(CG_GetLocalClientGlobals(localClientNum)->time, displayStartTime, duration, 700);
    if (fadeColor)
        return fadeColor[3];
    else
        return 0.0;
}

bool __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, uint32_t weapIndex)
{
    int32_t maxAmmo; // [esp+4h] [ebp-10h]
    int32_t curAmmo; // [esp+8h] [ebp-Ch]
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
    int32_t weapIndex; // [esp+0h] [ebp-4h]

    weapIndex = GetWeaponIndex(cgameGlob);
    return CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapIndex);
}

bool __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, uint32_t weapIndex)
{
    int32_t curClipVal; // [esp+0h] [ebp-10h]
    WeaponDef *weapDef; // [esp+8h] [ebp-8h]
    int32_t fullClipVal; // [esp+Ch] [ebp-4h]

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
    int32_t weapIndex; // [esp+0h] [ebp-4h]

    weapIndex = GetWeaponIndex(cgameGlob);
    return CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex);
}

float CG_CalcPlayerHealth(const playerState_s *ps)
{
    float v5; // [esp+Ch] [ebp-8h]
    float health; // [esp+10h] [ebp-4h]

    if (!ps->stats[0] || !ps->stats[2] || ps->pm_type == PM_DEAD)
        return 0.0;
    health = (double)ps->stats[0] / (double)ps->stats[2];

    if ((health - 1.0f) < 0.0)
        v5 = (double)ps->stats[0] / (double)ps->stats[2];
    else
        v5 = 1.0;

    if (0.0f - health < 0.0)
        return v5;
    else
        return 0.0f;
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

int32_t __cdecl CG_ServerMaterialName(int32_t localClientNum, int32_t index, char *materialName, uint32_t maxLen)
{
    char v5; // cl
    const char *v7; // [esp+Ch] [ebp-18h]
    const char *string; // [esp+20h] [ebp-4h]

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

Material *__cdecl CG_ObjectiveIcon(int32_t localClientNum, int32_t icon, int32_t type)
{
    char shaderName[68]; // [esp+0h] [ebp-48h] BYREF

    if (type)
        MyAssertHandler(
            ".\\cgame_mp\\cg_newDraw_mp.cpp",
            1208,
            0,
            "%s",
            "type >= 0 && static_cast<uint32_t>( type ) < ARRAY_COUNT( cgMedia.objectiveMaterials )");
    if (icon && CG_ServerMaterialName(localClientNum, icon, shaderName, 0x40u))
        return Material_RegisterHandle(shaderName, 7);
    else
        return cgMedia.objectiveMaterials[type];
}

const char *__cdecl CG_ScriptMainMenu(int32_t localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->scriptMainMenu;
}

void __cdecl CG_OwnerDraw(
    int32_t localClientNum,
    rectDef_s parentRect,
    float x,
    float y,
    float w,
    float h,
    int32_t horzAlign,
    int32_t vertAlign,
    float text_x,
    float text_y,
    int32_t ownerDraw,
    int32_t ownerDrawFlags,
    int32_t align,
    float special,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int32_t textStyle,
    char textAlignMode)
{
    OffhandClass offSecond; // [esp+20h] [ebp-24h]
    rectDef_s rect; // [esp+28h] [ebp-1Ch] BYREF
    const playerState_s *ps; // [esp+40h] [ebp-4h]

    ps = CG_GetPredictedPlayerState(localClientNum);
    iassert(ps);
    if (ps->offhandSecondary)
    {
        iassert(ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH);
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
        if (!CG_GetLocalClientGlobals(localClientNum)->inKillCam)
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
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material)
{
    float v4; // [esp+24h] [ebp-24h]
    float drawColor[4]; // [esp+38h] [ebp-10h] BYREF
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->predictedPlayerState.weapon)
    {
        drawColor[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_ammodisplay,
            cgameGlob->ammoFadeTime,
            (int)(hud_fade_ammodisplay->current.value * 1000.0f));
        if (drawColor[3] != 0.0)
        {
            if (CG_CheckPlayerForLowAmmo(cgameGlob))
            {
                drawColor[0] = 0.89f;
                drawColor[1] = 0.18f;
                drawColor[2] = 0.01f;
            }
            else
            {
                drawColor[0] = color[0];
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
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int32_t textStyle)
{
    double v7; // [esp+2Ch] [ebp-274h]
    float v8; // [esp+3Ch] [ebp-264h]
    float ammoColor[5]; // [esp+4Ch] [ebp-254h] BYREF
    const ScreenPlacement *scrPlace; // [esp+60h] [ebp-240h]
    int32_t ammoVal; // [esp+64h] [ebp-23Ch]
    cg_s *cgameGlob; // [esp+68h] [ebp-238h]
    bool drawAmmo; // [esp+6Fh] [ebp-231h]
    char clipString[260]; // [esp+70h] [ebp-230h] BYREF
    const centity_s *cent; // [esp+174h] [ebp-12Ch]
    char ammoString[256]; // [esp+178h] [ebp-128h] BYREF
    bool lowAmmo; // [esp+27Dh] [ebp-23h]
    bool lowClip; // [esp+27Eh] [ebp-22h]
    bool drawClip; // [esp+27Fh] [ebp-21h]
    float flashColor[4]; // [esp+280h] [ebp-20h] BYREF
    int32_t weapIndex; // [esp+290h] [ebp-10h]
    const playerState_s *ps; // [esp+294h] [ebp-Ch]
    float x; // [esp+298h] [ebp-8h]
    int32_t clipVal; // [esp+29Ch] [ebp-4h]
    int32_t flashTime;

    flashTime = 800;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (cgameGlob->predictedPlayerState.weapon)
    {
        color[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_ammodisplay,
            cgameGlob->ammoFadeTime,
            (int)(hud_fade_ammodisplay->current.value * 1000.0f));
        if (color[3] != 0.0)
        {
            iassert(cgameGlob->nextSnap);
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
                    snprintf(clipString, ARRAYSIZE(clipString), "%2i", clipVal);
                    lowClip = CG_CheckPlayerForLowClip(cgameGlob);
                }
                if (drawAmmo)
                {
                    snprintf(ammoString, ARRAYSIZE(ammoString), "%3i", ammoVal);
                    lowAmmo = CG_CheckPlayerForLowAmmo(cgameGlob);
                }
                if (lowClip)
                {
                    if (cgameGlob->lastClipFlashTime > cgameGlob->time || cgameGlob->lastClipFlashTime + 800 < cgameGlob->time)
                        cgameGlob->lastClipFlashTime = cgameGlob->time;
                    flashColor[0] = 0.88999999f;
                    flashColor[1] = 0.18000001f;
                    flashColor[2] = 0.0099999998f;
                    flashColor[3] = (cgameGlob->lastClipFlashTime + 800 - cgameGlob->time) / 800.0f;
                    if (flashColor[3] > color[3])
                        flashColor[3] = color[3];
                }
                if (lowAmmo)
                {
                    ammoColor[0] = 0.89f;
                    ammoColor[1] = 0.18f;
                    ammoColor[2] = 0.01f;
                }
                else
                {
                    ammoColor[0] = color[0];
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
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int32_t textStyle)
{
    double v6; // [esp+20h] [ebp-34h]
    weaponInfo_s *weapInfo; // [esp+28h] [ebp-2Ch]
    float *fadeColor; // [esp+2Ch] [ebp-28h]
    char *string; // [esp+34h] [ebp-20h]
    float drawColor[4]; // [esp+38h] [ebp-1Ch] BYREF
    int32_t weapIndex; // [esp+48h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+4Ch] [ebp-8h]
    float x; // [esp+50h] [ebp-4h]
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fadeColor = CG_FadeColor(cgameGlob->time, cgameGlob->weaponSelectTime, 1800, 700);
    if (fadeColor)
    {
        if ((cgameGlob->predictedPlayerState.weapFlags & 0x80) == 0)
        {
            drawColor[3] = fadeColor[3];
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            weapIndex = GetWeaponIndex(cgameGlob);
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
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    Material *material)
{
    const weaponInfo_s *weapInfo; // [esp+24h] [ebp-2Ch]
    char *string; // [esp+2Ch] [ebp-24h]
    float drawColor[4]; // [esp+30h] [ebp-20h] BYREF
    int32_t weapIndex; // [esp+40h] [ebp-10h]
    WeaponDef *weapDef; // [esp+44h] [ebp-Ch]
    float x; // [esp+48h] [ebp-8h]
    float w; // [esp+4Ch] [ebp-4h]
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    drawColor[3] = CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgameGlob->weaponSelectTime, 1800);
    if (drawColor[3] != 0.0)
    {
        drawColor[0] = color[0];
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        weapIndex = GetWeaponIndex(cgameGlob);
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
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Font_s *font,
    float scale,
    int32_t textStyle)
{
    float v9; // [esp+24h] [ebp-54h]
    float v10; // [esp+38h] [ebp-40h]
    float halfWidth; // [esp+4Ch] [ebp-2Ch]
    float drawColor[5]; // [esp+50h] [ebp-28h] BYREF
    float x; // [esp+64h] [ebp-14h]
    float y; // [esp+68h] [ebp-10h]
    const char *proneStr; // [esp+6Ch] [ebp-Ch]
    float fadeAlpha; // [esp+70h] [ebp-8h]
    float deltaTime; // [esp+74h] [ebp-4h]
    cg_s *cgameGlob;
    const cgs_t *cgs;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_stance, cgameGlob->stanceFadeTime, (int)(hud_fade_stance->current.value * 1000.0f));
    if (fadeAlpha != 0.0)
    {
        if (cg_hudStanceHintPrints->current.enabled)
        {
            if (cgameGlob->lastStance != (cgameGlob->predictedPlayerState.pm_flags & (PMF_PRONE | PMF_DUCKED)))
                cgameGlob->lastStanceChangeTime = cgameGlob->time;
        }
        else
        {
            cgameGlob->lastStanceChangeTime = 0;
        }
        cgameGlob->lastStance = cgameGlob->predictedPlayerState.pm_flags & (PMF_PRONE | PMF_DUCKED);
        drawColor[4] = 1.4025731e-38f;
        x = (compassSize->current.value - 1.0f) * cgs->compassWidth * 0.699999988079071f + rect->x;
        y = rect->y;
        KISAK_NULLSUB();
        drawColor[0] = color[0];
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        if ((cgameGlob->predictedPlayerState.pm_flags & PMF_NO_PRONE) != 0 && cgameGlob->proneBlockedEndTime < cgameGlob->time)
            cgameGlob->proneBlockedEndTime = cgameGlob->time + 1500;
        if (cgameGlob->proneBlockedEndTime > cgameGlob->time)
        {
            if (BG_WeaponBlocksProne(cgameGlob->predictedPlayerState.weapon))
                proneStr = UI_SafeTranslateString("CGAME_PRONE_BLOCKED_WEAPON");
            else
                proneStr = UI_SafeTranslateString("CGAME_PRONE_BLOCKED");
            halfWidth = UI_TextWidth(proneStr, 0, font, scale) * 0.5;
            deltaTime = (cgameGlob->proneBlockedEndTime - cgameGlob->time);
            v9 = deltaTime / 1500.0f * 540.0f * (M_PI / 180.0f);
            drawColor[3] = I_fabs(sin(v9));
            UI_DrawText(
                &scrPlaceView[localClientNum],
                proneStr,
                0x7FFFFFFF,
                font,
                -halfWidth,
                cg_hudProneY->current.value,
                7,
                3,
                scale,
                drawColor,
                textStyle);
        }
        if (cg_hudStanceHintPrints->current.enabled && cgameGlob->lastStanceChangeTime + 3000 > cgameGlob->time)
            CG_DrawStanceHintPrints(localClientNum, rect, x, color, fadeAlpha, font, scale, textStyle);
        drawColor[3] = color[3] * fadeAlpha;
        CG_DrawStanceIcon(localClientNum, rect, drawColor, x, y, fadeAlpha);
    }
}

void __cdecl CG_DrawStanceIcon(
    int32_t localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    float x,
    float y,
    float fadeAlpha)
{
    Material *icon; // [esp+24h] [ebp-10h]
    float width; // [esp+2Ch] [ebp-8h]
    float height; // [esp+30h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(rect);

    width = rect->w;
    height = rect->h;
    KISAK_NULLSUB();

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((cgameGlob->lastStance & 1) != 0)
    {
        icon = cgMedia.stanceMaterials[2];
    }
    else if ((cgameGlob->lastStance & 2) != 0)
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
    if (cgameGlob->lastStanceChangeTime + 1000 > cgameGlob->time)
    {
        Dvar_GetUnpackedColor(cg_hudStanceFlash, drawColor);
        drawColor[3] = (cgameGlob->lastStanceChangeTime + 1000 - cgameGlob->time) / 1000.0 * 0.800000011920929;
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
    int32_t localClientNum,
    const rectDef_s *rect,
    float x,
    const float *color,
    float fadeAlpha,
    Font_s *font,
    float scale,
    int32_t textStyle)
{
    float v8; // [esp+1Ch] [ebp-240h]
    char keyBinding[256]; // [esp+2Ch] [ebp-230h] BYREF
    int32_t j; // [esp+130h] [ebp-12Ch]
    const cg_s *cgameGlob; // [esp+134h] [ebp-128h]
    const char *string; // [esp+138h] [ebp-124h]
    const char *proneCmds[3][6]; // [esp+13Ch] [ebp-120h] BYREF
    float height; // [esp+188h] [ebp-D4h]
    float drawColor[4]; // [esp+18Ch] [ebp-D0h] BYREF
    const char *duckCmds[3][6]; // [esp+19Ch] [ebp-C0h] BYREF
    const char *hintLineCmds[3]; // [esp+1E8h] [ebp-74h]
    const char *standCmds[3][6]; // [esp+1F4h] [ebp-68h] BYREF
    const char *hintTypeStrings[3]; // [esp+240h] [ebp-1Ch]
    int32_t numHintLines; // [esp+24Ch] [ebp-10h]
    int32_t i; // [esp+250h] [ebp-Ch]
    const char *binding; // [esp+254h] [ebp-8h]
    float y; // [esp+258h] [ebp-4h]

    memset(standCmds, 0, 24);
    standCmds[1][0] = "gocrouch";
    standCmds[1][1] = "togglecrouch";
    standCmds[1][2] = "lowerstance";
    standCmds[1][3] = "+movedown";
    standCmds[1][4] = NULL;
    standCmds[1][5] = NULL;

    standCmds[2][0] = "goprone";
    standCmds[2][1] = "+prone";
    standCmds[2][2] = NULL;
    standCmds[2][3] = NULL;
    standCmds[2][4] = NULL;
    standCmds[2][5] = NULL;

    duckCmds[0][0] = "+gostand";
    duckCmds[0][1] = "raisestance";
    duckCmds[0][2] = "+moveup";
    duckCmds[0][3] = NULL;
    duckCmds[0][4] = NULL;
    duckCmds[0][5] = NULL;

    duckCmds[1][0] = NULL;
    duckCmds[1][1] = NULL;
    duckCmds[1][2] = NULL;
    duckCmds[1][3] = NULL;
    duckCmds[1][4] = NULL;
    duckCmds[1][5] = NULL;

    duckCmds[2][0] = "goprone";
    duckCmds[2][1] = "lowerstance";
    duckCmds[2][2] = "toggleprone";
    duckCmds[2][3] = "+prone";
    duckCmds[2][4] = NULL;
    duckCmds[2][5] = NULL;

    proneCmds[0][0] = "+gostand";
    proneCmds[0][1] = "toggleprone";
    proneCmds[0][2] = NULL;
    proneCmds[0][3] = NULL;
    proneCmds[0][4] = NULL;
    proneCmds[0][5] = NULL;

    proneCmds[1][0] = "gocrouch";
    proneCmds[1][1] = "togglecrouch";
    proneCmds[1][2] = "raisestance";
    proneCmds[1][3] = "+movedown";
    proneCmds[1][4] = "+moveup";
    proneCmds[1][5] = 0;

    proneCmds[2][0] = NULL;
    proneCmds[2][1] = NULL;
    proneCmds[2][2] = NULL;
    proneCmds[2][3] = NULL;
    proneCmds[2][4] = NULL;
    proneCmds[2][5] = NULL;

    hintTypeStrings[0] = "PLATFORM_STANCEHINT_STAND";
    hintTypeStrings[1] = "PLATFORM_STANCEHINT_CROUCH";
    hintTypeStrings[2] = "PLATFORM_STANCEHINT_PRONE";

    drawColor[0] = color[0];
    drawColor[1] = color[1];
    drawColor[2] = color[2];
    
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->lastStanceChangeTime + 3000 - cgameGlob->time <= 1000)
        drawColor[3] = (cgameGlob->lastStanceChangeTime + 3000 - cgameGlob->time) * EQUAL_EPSILON;
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

void __cdecl CG_DrawPlayerSprintBack(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+34h] [ebp-28h]
    float drawColor[4]; // [esp+48h] [ebp-14h] BYREF
    float fadeAlpha; // [esp+58h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgameGlob->sprintFadeTime, (int)(hud_fade_sprint->current.value * 1000.0f));
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

void __cdecl CG_DrawPlayerSprintMeter(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+34h] [ebp-48h]
    float sprint; // [esp+44h] [ebp-38h]
    float drawColor[4]; // [esp+4Ch] [ebp-30h] BYREF
    int32_t sprintLeft; // [esp+5Ch] [ebp-20h]
    float x; // [esp+60h] [ebp-1Ch]
    float y; // [esp+64h] [ebp-18h]
    playerState_s *ps; // [esp+68h] [ebp-14h]
    int32_t maxSprint; // [esp+6Ch] [ebp-10h]
    float h; // [esp+70h] [ebp-Ch]
    float fadeAlpha; // [esp+74h] [ebp-8h]
    float w; // [esp+78h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgameGlob->sprintFadeTime, (int)(hud_fade_sprint->current.value * 1000.0f));
    if (fadeAlpha != 0.0)
    {
        sprintLeft = PM_GetSprintLeft(ps, cgameGlob->time);
        maxSprint = BG_GetMaxSprintTime(ps);
        sprint = sprintLeft / maxSprint;
        if (sprint > 0.0f)
        {
            x = rect->x;
            y = rect->y;
            w = rect->w * sprint;
            h = rect->h;
            if (!material)
                material = cgMedia.whiteMaterial;
            CG_CalcPlayerSprintColor(cgameGlob, ps, color);
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
    int32_t sprintLeft; // [esp+18h] [ebp-8h]
    int32_t maxSprint; // [esp+1Ch] [ebp-4h]

    maxSprint = BG_GetMaxSprintTime(ps);
    if (ps->pm_type == PM_DEAD || !maxSprint)
    {
        p_current = &cg_sprintMeterFullColor->current;
        color[0] = p_current->vector[0];
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
            frac = (float)sprintLeft / (float)maxSprint;
            Vec4Lerp(&cg_sprintMeterEmptyColor->current.value, &cg_sprintMeterFullColor->current.value, frac, color);
        }
        else
        {
            //*(DvarValue *)color = cg_sprintMeterDisabledColor->current;
            color[0] = cg_sprintMeterDisabledColor->current.vector[0];
            color[1] = cg_sprintMeterDisabledColor->current.vector[1];
            color[2] = cg_sprintMeterDisabledColor->current.vector[2];
        }
    }
}

void __cdecl CG_DrawPlayerBarHealth(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color)
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
    cg_s *cgameGlob;

    if (cg_drawHealth->current.enabled)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        health = CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps);
        color[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_healthbar,
            cgameGlob->healthFadeTime,
            (int)(hud_fade_healthbar->current.value * 1000.0f));
        if (color[3] != 0.0)
        {
            ps = &cgameGlob->nextSnap->ps;
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

            if (health > 0.0f)
            {
                x = rect->x;
                y = rect->y;
                w = rect->w * health;
                h = rect->h;
                if (v4 <= 0.5)
                {
                    color[1] = (v4 + 0.2f) * color[1];
                    color[1] = color[1] + 0.3f;
                }
                else
                {
                    color[0] = (1.0f - v4 + 1.0f - v4) * color[0];
                    color[2] = (1.0f - v4 + 1.0f - v4) * color[2];
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
            if (cgameGlob->lastHealthClient == ps->clientNum)
            {
                if (cgameGlob->lastHealth <= health)
                {
                    cgameGlob->lastHealth = health;
                    cgameGlob->lastHealthLerpDelay = 1;
                }
                else if (cgameGlob->lastHealthLerpDelay)
                {
                    cgameGlob->lastHealthLerpDelay -= cgameGlob->frametime;
                    if (cgameGlob->lastHealthLerpDelay < 0)
                        cgameGlob->lastHealthLerpDelay = 0;
                }
                else
                {
                    cgameGlob->lastHealth = cgameGlob->lastHealth - cgameGlob->frametime * 0.0012f;
                    if (health >= cgameGlob->lastHealth)
                    {
                        cgameGlob->lastHealth = health;
                        cgameGlob->lastHealthLerpDelay = 1;
                    }
                }
            }
            else
            {
                cgameGlob->lastHealthClient = ps->clientNum;
                cgameGlob->lastHealth = health;
                cgameGlob->lastHealthLerpDelay = 1;
            }
            if (health < cgameGlob->lastHealth)
            {
                xa = rect->w * health + rect->x;
                ya = rect->y;
                wa = (cgameGlob->lastHealth - health) * rect->w;
                ha = rect->h;
                color[0] = 1.0f;
                color[1] = 0.0f;
                color[2] = 0.0f;
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    xa,
                    ya,
                    wa,
                    ha,
                    rect->horzAlign,
                    rect->vertAlign,
                    health,
                    0.0f,
                    cgameGlob->lastHealth,
                    1.0f,
                    color,
                    material);
            }
        }
    }
}

void __cdecl CG_DrawPlayerBarHealthBack(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    int32_t flashTime; // [esp+68h] [ebp-20h]
    float health; // [esp+70h] [ebp-18h]
    float x; // [esp+74h] [ebp-14h]
    float y; // [esp+78h] [ebp-10h]
    float h; // [esp+7Ch] [ebp-Ch]
    float fadeAlpha; // [esp+80h] [ebp-8h]
    float w; // [esp+84h] [ebp-4h]
    cg_s *cgameGlob;

    if (cg_drawHealth->current.enabled)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        fadeAlpha = CG_FadeHudMenu(
            localClientNum,
            hud_fade_healthbar,
            cgameGlob->healthFadeTime,
            (hud_fade_healthbar->current.value * 1000.0f));
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
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                color,
                material);
            health = CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps);
            if (health != 0.0f)
            {
                if (hud_health_startpulse_critical->current.value <= health)
                {
                    if (hud_health_startpulse_injured->current.value <= health)
                    {
                        flashTime = 0;
                    }
                    else
                    {
                        flashTime = (hud_health_pulserate_injured->current.value * 1000.0f);
                    }
                }
                else
                {
                    flashTime = (hud_health_pulserate_critical->current.value * 1000.0f);
                }
                if (flashTime)
                {
                    if (cgameGlob->lastHealthPulseTime > cgameGlob->time
                        || flashTime + cgameGlob->lastHealthPulseTime < cgameGlob->time)
                    {
                        cgameGlob->lastHealthPulseTime = cgameGlob->time;
                    }
                    color[0] = 0.89f;
                    color[1] = 0.18f;
                    color[2] = 0.01f;
                    color[3] = (flashTime + cgameGlob->lastHealthPulseTime - cgameGlob->time) / flashTime;
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
                        0.0f,
                        0.0f,
                        1.0f,
                        1.0f,
                        color,
                        material);
                }
            }
        }
    }
}

void __cdecl CG_DrawPlayerLowHealthOverlay(int32_t localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float healthRatio; // [esp+34h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    healthRatio = CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps);
    if (healthRatio != 0.0)
    {
        CG_PulseLowHealthOverlay(cgameGlob, healthRatio);
        color[3] = CG_FadeLowHealthOverlay(cgameGlob);
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
    int32_t timeSinceFadeStarted; // [esp+10h] [ebp-4h]

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
    float v3; // [esp+0h] [ebp-58h]
    float v4; // [esp+4h] [ebp-54h]
    float v5; // [esp+8h] [ebp-50h]
    float v6; // [esp+Ch] [ebp-4Ch]
    float v7; // [esp+10h] [ebp-48h]
    float v8; // [esp+14h] [ebp-44h]
    float v9; // [esp+18h] [ebp-40h]
    float v10; // [esp+1Ch] [ebp-3Ch]
    float v11; // [esp+20h] [ebp-38h]
    int32_t healthOverlayPulsePhase; // [esp+24h] [ebp-34h]
    float v13; // [esp+28h] [ebp-30h]
    float v14; // [esp+2Ch] [ebp-2Ch]
    float v15; // [esp+30h] [ebp-28h]
    float v16; // [esp+34h] [ebp-24h]
    float v17; // [esp+38h] [ebp-20h]
    float v18; // [esp+3Ch] [ebp-1Ch]
    float pulseMags[4] = { 1.0f, 0.8f, 0.6f, 0.3f };

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
                    v8 = v15 - 1.0f;
                    if (v8 < 0.0f)
                        v16 = hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    else
                        v16 = 1.0f;
                    v7 = 0.0f - v15;
                    if (v7 < 0.0f)
                        v6 = v16;
                    else
                        v6 = 0.0f;
                    cgameGlob->healthOverlayToAlpha = v6;
                    cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseTwo_pulseDuration->current.integer;
                    ++cgameGlob->healthOverlayPulsePhase;
                }
                else if (healthOverlayPulsePhase == 2)
                {
                    v13 = hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    v5 = v13 - 1.0f;
                    if (v5 < 0.0f)
                        v14 = hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    else
                        v14 = 1.0f;
                    v4 = 0.0f - v13;
                    if (v4 < 0.0f)
                        v3 = v14;
                    else
                        v3 = 0.0f;
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
                    MyAssertHandler(".\\cgame_mp\\cg_newDraw_mp.cpp", 1134, 0, va("Invalid health overlay pulse phase: %i", cgameGlob->healthOverlayPulsePhase));
                }
            }
            else
            {
                v17 = pulseMags[cgameGlob->healthOverlayPulseIndex];
                v11 = v17 - 1.0f;
                if (v11 < 0.0f)
                    v18 = v17;
                else
                    v18 = 1.0f;
                v10 = 0.0f - v17;
                if (v10 < 0.0f)
                    v9 = v18;
                else
                    v9 = 0.0f;
                cgameGlob->healthOverlayToAlpha = v9;
                cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseOne_pulseDuration->current.integer;
                ++cgameGlob->healthOverlayPulsePhase;
            }
        }
    }
}

void __cdecl CG_DrawCursorhint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle)
{
    double v6; // st7
    char *v7; // eax
    char *v8; // eax
    int32_t h; // [esp+Ch] [ebp-1C4h]
    int32_t vertAlign; // [esp+10h] [ebp-1C0h]
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
    uint32_t weaponIndex; // [esp+A4h] [ebp-12Ch]
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
    cg_s *cgameGlob;

    if (cg_cursorHints->current.integer)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        CG_UpdateCursorHints(cgameGlob);
        color[3] = CG_FadeAlpha(cgameGlob->time, cgameGlob->cursorHintTime, cgameGlob->cursorHintFade, 100) * color[3];
        if (color[3] == 0.0)
        {
            cgameGlob->cursorHintIcon = 0;
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
                v28 = cgameGlob->time / 150.0;
                v26 = sin(v28);
                color[3] = (v26 * 0.5 + 0.5) * color[3];
            }
            if (cg_cursorHints->current.integer < 3)
            {
                if (cg_cursorHints->current.integer == 2)
                {
                    v6 = (cgameGlob->cursorHintTime % 1000) / 100.0;
                }
                else
                {
                    v27 = cgameGlob->time / 150.0;
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
            if (cgameGlob->cursorHintIcon == 1)
            {
                if (cgameGlob->cursorHintString >= 0)
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
                hintIcon = cgMedia.hintMaterials[cgameGlob->cursorHintIcon];
                if (hintIcon)
                {
                    if (cgameGlob->cursorHintIcon < 5 || cgameGlob->cursorHintIcon > 132)
                    {
                        if (cgameGlob->cursorHintString < 0)
                        {
                            if (cgameGlob->cursorHintIcon == 3)
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
                        weaponIndex = cgameGlob->cursorHintIcon - 4;
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
                            if (cgameGlob->cursorHintString >= 0)
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
                            UI_DrawText(
                                scrPlace,
                                displayString,
                                0x7FFFFFFF,
                                font,
                                x,
                                height * 0.5f + rect->y,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                            UI_DrawText(scrPlace, 
                                va(" %s", secondaryString),
                                0x7FFFFFFF,
                                font,
                                x + length,
                                height * 0.5f + rect->y,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale, 
                                color, 
                                textStyle);

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

char *__cdecl CG_GetWeaponUseString(int32_t localClientNum, const char **secondaryString)
{
    const weaponInfo_s *weapInfo; // [esp+0h] [ebp-120h]
    int32_t weaponIndex; // [esp+8h] [ebp-118h]
    char *displayString; // [esp+Ch] [ebp-114h]
    char binding[260]; // [esp+10h] [ebp-110h] BYREF
    WeaponDef *weapDef; // [esp+118h] [ebp-8h]
    const playerState_s *ps; // [esp+11Ch] [ebp-4h]
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert((cgameGlob->cursorHintIcon >= FIRST_WEAPON_HINT) && (cgameGlob->cursorHintIcon <= LAST_WEAPON_HINT));

    weaponIndex = cgameGlob->cursorHintIcon - 4;
    ps = &cgameGlob->predictedPlayerState;
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

char *__cdecl CG_GetUseString(int32_t localClientNum)
{
    const char *displayString; // [esp+4h] [ebp-10Ch]
    char binding[260]; // [esp+8h] [ebp-108h] BYREF
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(cgameGlob->cursorHintString >= 0);

    displayString = CL_GetConfigString(localClientNum, cgameGlob->cursorHintString + 277);
    if (!displayString || !*displayString)
        return 0;

    if (!UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding))
    {
        I_strncpyz(binding, UI_SafeTranslateString("KEY_USE"), 256);
    }

    return UI_ReplaceConversionString(SEH_LocalizeTextMessage(displayString, "Hint String", LOCMSG_SAFE), binding);
}

void __cdecl CG_DrawHoldBreathHint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    int32_t textStyle)
{
    uint32_t ViewmodelWeaponIndex; // eax
    char *v6; // eax
    float v7; // [esp+24h] [ebp-124h]
    char *string; // [esp+34h] [ebp-114h]
    char binding[256]; // [esp+38h] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+13Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+140h] [ebp-8h]
    float x; // [esp+144h] [ebp-4h]
    cg_s *cgameGlob;

    if (cg_drawBreathHint->current.enabled)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        ps = &cgameGlob->predictedPlayerState;
        if ((cgameGlob->predictedPlayerState.weapFlags & 4) == 0)
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
                    x = rect->x - (UI_TextWidth(string, 0, font, fontscale) * 0.5f);
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
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    const float *color,
    int32_t textStyle)
{
    char *v6; // eax
    char *string; // [esp+28h] [ebp-120h]
    float height; // [esp+2Ch] [ebp-11Ch]
    char binding[260]; // [esp+30h] [ebp-118h] BYREF
    const playerState_s *ps; // [esp+138h] [ebp-10h]
    float length; // [esp+13Ch] [ebp-Ch]
    float x; // [esp+140h] [ebp-8h]
    float y; // [esp+144h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(cgMedia.mantleHint);

    if (cg_drawMantleHint->current.enabled)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        ps = &cgameGlob->predictedPlayerState;
        if ((cgameGlob->predictedPlayerState.mantleState.flags & 8) != 0)
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
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle)
{
    char *string; // [esp+40h] [ebp-Ch]
    float x; // [esp+44h] [ebp-8h]
    int32_t blinkInterval; // [esp+48h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(rect);
    iassert(color);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cg_invalidCmdHintDuration->current.integer + cgameGlob->invalidCmdHintTime < cgameGlob->time)
        cgameGlob->invalidCmdHintType = INVALID_CMD_NONE;

    switch (cgameGlob->invalidCmdHintType)
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
        color[3] = ((cgameGlob->time - cgameGlob->invalidCmdHintTime) % blinkInterval) / blinkInterval;
        x = rect->x - (UI_TextWidth(string, 0, font, fontscale) * 0.5f);
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
        iassert(cgameGlob->invalidCmdHintType == INVALID_CMD_NONE);
        break;
    }
}

void __cdecl CG_DrawTalkerNum(
    int32_t localClientNum,
    int32_t num,
    rectDef_s *rect,
    Font_s *font,
    float *color,
    float textScale,
    int32_t style)
{
    float v7; // [esp+20h] [ebp-44h]
    float v8; // [esp+24h] [ebp-40h]
    bool v9; // [esp+28h] [ebp-3Ch]
    team_t team; // [esp+34h] [ebp-30h]
    int32_t client; // [esp+38h] [ebp-2Ch]
    Material *material; // [esp+3Ch] [ebp-28h]
    float textColor[4]; // [esp+44h] [ebp-20h] BYREF
    char *name; // [esp+54h] [ebp-10h]
    int32_t drawTalk; // [esp+58h] [ebp-Ch]
    int32_t textHeight; // [esp+5Ch] [ebp-8h]
    bool isEnemy; // [esp+63h] [ebp-1h]
    cg_s *cgameGlob;

    iassert(color);
    iassert(rect);

    drawTalk = cg_drawTalk->current.integer;
    if (drawTalk)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        client = UI_GetTalkerClientNum(localClientNum, num);
        if (client >= 0)
        {
            name = cgameGlob->bgs.clientinfo[client].name;
            if (cgameGlob->bgs.clientinfo[client].infoValid)
            {
                v9 = 0;
                if (cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team)
                {
                    team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
                    if (team == TEAM_FREE || team != cgameGlob->bgs.clientinfo[client].team)
                        v9 = 1;
                }
                isEnemy = v9;
                if ((drawTalk != 2 || !isEnemy) && (drawTalk != 3 || isEnemy))
                {
                    textColor[0] = *color;
                    textColor[1] = color[1];
                    textColor[2] = color[2];
                    textColor[3] = color[3];
                    if (cgameGlob->nextSnap->ps.pm_type != PM_INTERMISSION && isEnemy && (cgameGlob->nextSnap->ps.perks & 0x200) != 0)
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

void __cdecl CG_ArchiveState(int32_t localClientNum, MemoryFile *memFile)
{
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    MemFile_ArchiveData(memFile, 4, &cgameGlob->healthFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->ammoFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->stanceFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->compassFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->offhandFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->sprintFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->drawHud);
    MemFile_ArchiveData(memFile, 1024, cgameGlob->objectiveText);
    MemFile_ArchiveData(memFile, 256, cgameGlob->scriptMainMenu);
    MemFile_ArchiveData(memFile, 160, cgameGlob->visionSetFrom);
    MemFile_ArchiveData(memFile, 160, cgameGlob->visionSetTo);
    MemFile_ArchiveData(memFile, 160, cgameGlob->visionSetCurrent);
    MemFile_ArchiveData(memFile, 24, cgameGlob->visionSetLerpData);
    MemFile_ArchiveData(memFile, 64, cgameGlob->visionNameNaked);
    MemFile_ArchiveData(memFile, 64, cgameGlob->visionNameNight);
    MemFile_ArchiveData(memFile, 128, cgameGlob->hudElemSound);
}

