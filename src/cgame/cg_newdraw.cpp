#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_newdraw.h"
#include "cg_main.h"

#include <xanim/xanim.h>
#include <ui/ui.h>
#include <stringed/stringed_hooks.h>
#include "cg_public.h"
#include "cg_vehicle_hud.h"
#include "cg_compassfriendlies.h"
#include "cg_scoreboard.h"
#include "cg_draw.h"
#include "cg_ents.h"
#include <script/scr_const.h>
#include "cg_view.h"

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
const dvar_t *hud_healthOverlay_pulseStart;
const dvar_t *hud_healthOverlay_pulseStop;
const dvar_t *hud_healthOverlay_phaseOne_toAlphaAdd;
const dvar_t *hud_healthOverlay_phaseOne_pulseDuration;
const dvar_t *hud_healthOverlay_phaseTwo_toAlphaMultiplier;
const dvar_t *hud_healthOverlay_phaseTwo_pulseDuration;
const dvar_t *hud_healthOverlay_phaseThree_toAlphaMultiplier;
const dvar_t *hud_healthOverlay_phaseThree_pulseDuration;
const dvar_t *hud_healthOverlay_phaseEnd_fromAlpha;
const dvar_t *hud_healthOverlay_phaseEnd_toAlpha;
const dvar_t *hud_healthOverlay_phaseEnd_pulseDuration;
const dvar_t *cg_sprintMeterFullColor;
const dvar_t *cg_sprintMeterEmptyColor;
const dvar_t *cg_sprintMeterDisabledColor;
const dvar_t *hud_gasMaskOverlay;
const dvar_t *hud_healthOverlay_regenPauseTime;

const dvar_t *hud_enable;


void __cdecl CG_AntiBurnInHUD_RegisterDvars()
{
    hud_fadeout_speed = Dvar_RegisterFloat("hud_fadeout_speed", 0.1f, 0.0f, 1.0f, DVAR_ARCHIVE, "The speed that the HUD will fade at");
    hud_enable = Dvar_RegisterBool("hud_enable", 1, DVAR_ARCHIVE, "Enable hud elements");
    hud_fade_ammodisplay = Dvar_RegisterFloat("hud_fade_ammodisplay", 0.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the ammo display to fade in seconds");
    hud_fade_healthbar = Dvar_RegisterFloat("hud_fade_healthbar", 2.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the health bar to fade in seconds");
    hud_fade_compass = Dvar_RegisterFloat("hud_fade_compass", 0.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the compass to fade in seconds");
    hud_fade_stance = Dvar_RegisterFloat("hud_fade_stance", 1.7f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the stance to fade in seconds");
    hud_fade_offhand = Dvar_RegisterFloat("hud_fade_offhand", 0.0f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the offhand weapons to fade in seconds");
    hud_fade_sprint = Dvar_RegisterFloat("hud_fade_sprint", 1.7f, 0.0f, 30.0f, DVAR_ARCHIVE, "The time for the sprint meter to fade in seconds");
    hud_health_startpulse_injured = Dvar_RegisterFloat("hud_health_startpulse_injured", 1.0f, 0.0f, 1.1f, DVAR_ARCHIVE, "The health level at which to start the 'injured' pulse effect");
    hud_health_startpulse_critical = Dvar_RegisterFloat("hud_health_startpulse_critical", 0.33f, 0.0f, 1.1f, DVAR_ARCHIVE, "The health level at which to start the 'critical' pulse effect");
    hud_health_pulserate_injured = Dvar_RegisterFloat("hud_health_pulserate_injured", 1.0f, 0.1f, 3.0f, DVAR_ARCHIVE, "The pulse rate of the 'injured' pulse effect");
    hud_health_pulserate_critical = Dvar_RegisterFloat("hud_health_pulserate_critical", 0.5f, 0.1f, 3.0f, DVAR_ARCHIVE, "The pulse rate of the 'critical' pulse effect");
    hud_deathQuoteFadeTime = Dvar_RegisterInt("hud_deathQuoteFadeTime", 1000, (DvarLimits)0x186A000000000LL, DVAR_ARCHIVE, "The time for the death quote to fade");
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

    // new for SP
    hud_healthOverlay_phaseOne_toAlphaAdd = Dvar_RegisterFloat("hud_healthOverlay_phaseOne_toAlphaAdd", 0.30000001, 0.0, 1.0, 0, 0);
    hud_healthOverlay_phaseEnd_fromAlpha = Dvar_RegisterFloat("hud_healthOverlay_phaseEnd_fromAlpha", 0.2, 0.0, 1.0, 0, 0);
    hud_healthOverlay_regenPauseTime = Dvar_RegisterInt("hud_healthOverlay_regenPauseTime", 8000, (DvarLimits)0x271000000000LL, DVAR_CHEAT, "The time in milliseconds before the health regeneration kicks in");
    hud_gasMaskOverlay = Dvar_RegisterBool("hud_gasMaskOverlay", 0, 0x1000u, "Signals the \"FacemaskOverlay\" menu to draw, meant to be changed by script.");
}

unsigned int __cdecl CG_GetSelectedWeaponIndex(const cg_s *cgameGlob)
{
    const unsigned int *p_weaponSelect; // r29
    unsigned int weaponSelect; // r30

    p_weaponSelect = &cgameGlob->weaponSelect;
    weaponSelect = cgameGlob->weaponSelect;
    if (weaponSelect < BG_GetNumWeapons() && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, weaponSelect))
        return *p_weaponSelect;
    else
        return cgameGlob->predictedPlayerState.weapon;
}

int __cdecl CG_IsHudHidden()
{
    unsigned __int8 v0; // r11

    if (!cg_paused->current.integer)
        return 0;
    v0 = 1;
    if (cg_drawpaused->current.enabled)
        return 0;
    return v0;
}

int __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, unsigned int weapIndex)
{
    const playerState_s *p_predictedPlayerState; // r30
    int TotalAmmoReserve; // r29
    __int64 v5; // r10
    int AmmoPlayerMax; // r3
    int result; // r3
    double v8; // fp0

    p_predictedPlayerState = &cgameGlob->predictedPlayerState;
    if (weapIndex)
    {
        BG_AmmoForWeapon(weapIndex);
        TotalAmmoReserve = BG_GetTotalAmmoReserve(p_predictedPlayerState, weapIndex);
        if (TotalAmmoReserve > 999)
            TotalAmmoReserve = 999;
        BG_GetWeaponDef(weapIndex);
        AmmoPlayerMax = BG_GetAmmoPlayerMax(p_predictedPlayerState, weapIndex, 0);
        if (AmmoPlayerMax > 999)
        {
            AmmoPlayerMax = 999;
            goto LABEL_7;
        }
        if (AmmoPlayerMax >= 0)
        {
        LABEL_7:
            LODWORD(v5) = TotalAmmoReserve;
            v8 = (float)((float)__SPAIR64__(TotalAmmoReserve, AmmoPlayerMax) * (float)0.2);
            result = 1;
            if ((float)v5 <= v8)
                return result;
        }
    }
    return 0;
}

int __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob)
{
    const unsigned int *p_weaponSelect; // r29
    unsigned int weaponSelect; // r30

    p_weaponSelect = &cgameGlob->weaponSelect;
    weaponSelect = cgameGlob->weaponSelect;
    if (weaponSelect < BG_GetNumWeapons() && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, weaponSelect))
        return CG_CheckPlayerForLowAmmoSpecific(cgameGlob, *p_weaponSelect);
    else
        return CG_CheckPlayerForLowAmmoSpecific(cgameGlob, cgameGlob->predictedPlayerState.weapon);
}

// local variable allocation has failed, the output may be wrong!
int __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, unsigned int weapIndex)
{
    int v4; // r31
    WeaponDef *WeaponDef; // r3
    __int64 v7; // r11 OVERLAPPED
    int result; // r3
    double v9; // fp0

    if (weapIndex)
    {
        if (!BG_WeaponIsClipOnly(weapIndex))
        {
            v4 = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIndex)];
            if (v4 >= 0)
            {
                if (v4 > 999)
                    v4 = 999;
                WeaponDef = BG_GetWeaponDef(weapIndex);
                LODWORD(v7) = WeaponDef->iClipSize;
                if ((int)v7 > 999)
                {
                    LODWORD(v7) = 999;
                    goto LABEL_9;
                }
                if ((int)v7 > 0)
                {
                LABEL_9:
                    HIDWORD(v7) = v4;
                    v9 = (float)(WeaponDef->lowAmmoWarningThreshold * (float)v7);
                    result = 1;
                    if ((float)*(__int64 *)((char *)&v7 + 4) <= v9)
                        return result;
                }
            }
        }
    }
    return 0;
}

int __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob)
{
    const unsigned int *p_weaponSelect; // r29
    unsigned int weaponSelect; // r30

    p_weaponSelect = &cgameGlob->weaponSelect;
    weaponSelect = cgameGlob->weaponSelect;
    if (weaponSelect < BG_GetNumWeapons() && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, weaponSelect))
        return CG_CheckPlayerForLowClipSpecific(cgameGlob, *p_weaponSelect);
    else
        return CG_CheckPlayerForLowClipSpecific(cgameGlob, cgameGlob->predictedPlayerState.weapon);
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
            string = UI_ReplaceConversionString((char *)string, keyBinding);
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

float __cdecl CG_FadeLowHealthOverlay(const cg_s *cgameGlob)
{
    _BYTE v1[12]; // r11 OVERLAPPED
    double healthOverlayToAlpha; // fp31
    double v3; // fp1

    *(unsigned int *)&v1[4] = cgameGlob->time - cgameGlob->healthOverlayPulseTime;
    if (*(int *)&v1[4] < 0)
        *(unsigned int *)&v1[4] = 0;
    *(unsigned int *)&v1[8] = cgameGlob->healthOverlayPulseDuration;
    if (*(int *)&v1[8] <= 0 || *(int *)&v1[4] >= *(int *)&v1[8])
    {
        healthOverlayToAlpha = cgameGlob->healthOverlayToAlpha;
    }
    else
    {
        *(unsigned int *)v1 = 171060;
        healthOverlayToAlpha = (float)((float)((float)((float)*(__int64 *)v1 / (float)*(__int64 *)&v1[4])
            * (float)(cgameGlob->healthOverlayToAlpha - cgameGlob->healthOverlayFromAlpha))
            + cgameGlob->healthOverlayFromAlpha);
    }
    if (healthOverlayToAlpha < 0.0 || healthOverlayToAlpha > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
            1020,
            0,
            "%s\n\t(curAlpha) = %g",
            HIDWORD(healthOverlayToAlpha),
            LODWORD(healthOverlayToAlpha));
    v3 = healthOverlayToAlpha;
    return *((float *)&v3 + 1);
}

void __cdecl CG_PulseLowHealthOverlay(cg_s *cgameGlob, double healthRatio)
{
#if 1
    int *p_healthOverlayPulseDuration; // r29
    int *p_healthOverlayPulseTime; // r23
    int *p_time; // r22
    int time; // r9
    bool *p_healthOverlayHurt; // r24
    int v9; // r8
    float *p_healthOverlayToAlpha; // r10
    int *p_healthOverlayPulsePhase; // r11
    double healthOverlayToAlpha; // fp0
    unsigned int healthOverlayPulsePhase; // r4
    const char *v14; // r3
    double v15; // fp0
    int integer; // r10
    double v21; // fp0
    const dvar_s *v26; // r10
    int v31; // r11
    const dvar_s *v32; // r11
    double value; // fp0
    const dvar_s *v34; // r11
    int v35; // r11

    p_healthOverlayPulseDuration = &cgameGlob->healthOverlayPulseDuration;
    p_healthOverlayPulseTime = &cgameGlob->healthOverlayPulseTime;
    p_time = &cgameGlob->time;
    time = cgameGlob->time;
    if (cgameGlob->healthOverlayPulseTime + cgameGlob->healthOverlayPulseDuration > time)
        return;
    if (healthRatio < hud_healthOverlay_pulseStart->current.value
        || (p_healthOverlayHurt = &cgameGlob->healthOverlayHurt, cgameGlob->healthOverlayHurt))
    {
        p_healthOverlayHurt = &cgameGlob->healthOverlayHurt;
        v9 = 1;
        if (!cgameGlob->healthOverlayHurt)
            *p_healthOverlayHurt = 1;
        *p_healthOverlayPulseTime = time;
        p_healthOverlayToAlpha = &cgameGlob->healthOverlayToAlpha;
        p_healthOverlayPulsePhase = &cgameGlob->healthOverlayPulsePhase;
        healthOverlayToAlpha = cgameGlob->healthOverlayToAlpha;
        cgameGlob->healthOverlayFromAlpha = cgameGlob->healthOverlayToAlpha;
        healthOverlayPulsePhase = cgameGlob->healthOverlayPulsePhase;
        if (healthOverlayPulsePhase)
        {
            if (healthOverlayPulsePhase != 1)
            {
                if (healthOverlayPulsePhase >= 3)
                {
                    if (!alwaysfails)
                    {
                        v14 = va("Invalid health overlay pulse phase: %i", healthOverlayPulsePhase);
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 1062, 0, v14);
                    }
                    goto LABEL_16;
                }
                v15 = (float)(hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value * (float)healthOverlayToAlpha);
                //_FP12 = (float)((float)v15 - (float)1.0);
                //_FP11 = -v15;
                //__asm { fsel      f13, f12, f13, f0 }
                //__asm { fsel      f0, f11, f0, f13 }
                //*p_healthOverlayToAlpha = _FP0;

                *p_healthOverlayToAlpha = CLAMP(v15 - 1.0f, 0.0f, 1.0f);
                integer = hud_healthOverlay_phaseThree_pulseDuration->current.integer;
                *p_healthOverlayPulsePhase = 0;
                goto LABEL_15;
            }
            v9 = 2;
            v21 = (float)(hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value * (float)healthOverlayToAlpha);
            //_FP12 = (float)((float)v21 - (float)1.0);
            //_FP11 = -v21;
            //__asm { fsel      f13, f12, f13, f0 }
            //__asm { fsel      f0, f11, f0, f13 }
            //*p_healthOverlayToAlpha = _FP0;
            *p_healthOverlayToAlpha = CLAMP(v21 - 1.0f, 0.0f, 1.0f);
            v26 = hud_healthOverlay_phaseTwo_pulseDuration;
        }
        else
        {
            //_FP12 = (float)((float)((float)((float)1.0 - (float)((float)healthRatio * (float)0.3f))
            //    + hud_healthOverlay_phaseOne_toAlphaAdd->current.value)
            //    - (float)1.0);
            //_FP11 = -(float)((float)((float)1.0 - (float)((float)healthRatio * (float)0.3f))
            //    + hud_healthOverlay_phaseOne_toAlphaAdd->current.value);
            //__asm { fsel      f13, f12, f0, f13 }
            //__asm { fsel      f0, f11, f0, f13 }
            //*p_healthOverlayToAlpha = _FP0;
            *p_healthOverlayToAlpha = CLAMP((1.0f - healthRatio * 0.3f), 0.0f, 1.0f);
            v26 = hud_healthOverlay_phaseOne_pulseDuration;
        }
        integer = v26->current.integer;
        *p_healthOverlayPulsePhase = v9;
    LABEL_15:
        *p_healthOverlayPulseDuration = integer;
    }
LABEL_16:
    if (healthRatio > hud_healthOverlay_pulseStop->current.value && *p_healthOverlayHurt)
    {
        v31 = *p_time;
        *p_healthOverlayHurt = 0;
        *p_healthOverlayPulseTime = v31;
        v32 = hud_healthOverlay_phaseEnd_toAlpha;
        cgameGlob->healthOverlayFromAlpha = hud_healthOverlay_phaseEnd_fromAlpha->current.value;
        value = v32->current.value;
        v34 = hud_healthOverlay_phaseEnd_pulseDuration;
        cgameGlob->healthOverlayToAlpha = value;
        v35 = v34->current.integer;
        cgameGlob->healthOverlayPulsePhase = 0;
        *p_healthOverlayPulseDuration = v35;
    }
#else
    float v3; // [esp+0h] [ebp-58h]
    float v4; // [esp+4h] [ebp-54h]
    float v5; // [esp+8h] [ebp-50h]
    float v6; // [esp+Ch] [ebp-4Ch]
    float v7; // [esp+10h] [ebp-48h]
    float v8; // [esp+14h] [ebp-44h]
    float v9; // [esp+18h] [ebp-40h]
    float maxSprint; // [esp+1Ch] [ebp-3Ch]
    float v11; // [esp+20h] [ebp-38h]
    int32_t healthOverlayPulsePhase; // [esp+24h] [ebp-34h]
    float v13; // [esp+28h] [ebp-30h]
    float v14; // [esp+2Ch] [ebp-2Ch]
    float v15; // [esp+30h] [ebp-28h]
    float string; // [esp+34h] [ebp-24h]
    float v6; // [esp+38h] [ebp-20h]
    float displayString; // [esp+3Ch] [ebp-1Ch]
    float pulseMags[4] = { 1.0f, 0.8f, 0.6f, 0.3f };

    if (cgameGlob->healthOverlayOldHealth > (double)healthRatio && hud_healthOverlay_pulseStart->current.value > (double)healthRatio)
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
                        string = hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
                    else
                        string = 1.0f;
                    v7 = 0.0f - v15;
                    if (v7 < 0.0f)
                        v6 = string;
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
                        v14 = hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value
                        * pulseMags[cgameGlob->healthOverlayPulseIndex];
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
                v6 = pulseMags[cgameGlob->healthOverlayPulseIndex];
                v11 = v6 - 1.0f;
                if (v11 < 0.0f)
                    displayString = v6;
                else
                    displayString = 1.0f;
                maxSprint = 0.0f - v6;
                if (maxSprint < 0.0f)
                    v9 = displayString;
                else
                    v9 = 0.0f;
                cgameGlob->healthOverlayToAlpha = v9;
                cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseOne_pulseDuration->current.integer;
                ++cgameGlob->healthOverlayPulsePhase;
            }
        }
    }
#endif
}

void __cdecl CG_DrawPlayerLowHealthOverlay(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color)
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

int __cdecl CG_ServerMaterialName(int localClientNum, int index, char *materialName, unsigned int maxLen)
{
    const char *ConfigString; // r3
    char *v7; // r11
    const char *v8; // r10
    int v10; // r10

    if ((unsigned int)(index - 1) > 0x7E)
        return 0;
    ConfigString = CL_GetConfigString(localClientNum, index + 2583);
    v7 = (char*)ConfigString;
    if (!*ConfigString)
        return 0;
    v8 = ConfigString;
    while (*(unsigned __int8 *)v8++)
        ;
    if (v8 - ConfigString - 1 >= maxLen)
        return 0;
    do
    {
        v10 = *(unsigned __int8 *)v7;
        (v7++)[materialName - ConfigString] = v10;
    } while (v10);
    return 1;
}

Material *__cdecl CG_ObjectiveIcon(int icon, unsigned int type)
{
    char v5[72]; // [sp+50h] [-60h] BYREF

    if (type >= 2)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
            1128,
            0,
            "type doesn't index ARRAY_COUNT( cgMedia.objectiveMaterials )\n\t%i not in [0, %i)",
            type,
            2);
    if (icon && CG_ServerMaterialName(0, icon, v5, 0x40u))
        return Material_RegisterHandle(v5, 7);
    else
        return cgMedia.objectiveMaterials[type];
}

void __cdecl CG_UpdateCursorHints(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].nextSnap->ps.cursorHint)
    {
        cgArray[0].cursorHintTime = cgArray[0].time;
        cgArray[0].cursorHintFade = cg_hintFadeTime->current.integer;
        cgArray[0].cursorHintIcon = cgArray[0].nextSnap->ps.cursorHint;
        cgArray[0].cursorHintString = cgArray[0].nextSnap->ps.cursorHintString;
    }
}

char *__cdecl CG_GetWeaponUseString(int localClientNum, const char **secondaryString)
{
    int cursorHintIcon; // r11
    int v5; // r30
    WeaponDef *WeaponDef; // r29
    char *v7; // r28
    char *v8; // r30
    char *v9; // r30
    char v11[336]; // [sp+50h] [-150h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cursorHintIcon = cgArray[0].cursorHintIcon;
    if (cgArray[0].cursorHintIcon < 5 || cgArray[0].cursorHintIcon > 132)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
            1176,
            0,
            "%s",
            "(cgameGlob->cursorHintIcon >= FIRST_WEAPON_HINT) && (cgameGlob->cursorHintIcon <= LAST_WEAPON_HINT)");
        cursorHintIcon = cgArray[0].cursorHintIcon;
    }
    v5 = cursorHintIcon - 4;
    WeaponDef = BG_GetWeaponDef(cursorHintIcon - 4);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            924,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v7 = (char *)cg_weaponsArray + 72 * v5;
    if (WeaponDef->inventoryType)
    {
        if (WeaponDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE)
        {
            v9 = UI_SafeTranslateString("PLATFORM_THROWBACKGRENADE");
            UI_GetKeyBindingLocalizedString(localClientNum, "+frag", v11);
            return UI_ReplaceConversionString(v9, v11);
        }
        v8 = UI_SafeTranslateString("PLATFORM_PICKUPNEWWEAPON");
        UI_GetKeyBindingLocalizedString(localClientNum, "+activate", v11);
    }
    else
    {
        UI_GetKeyBindingLocalizedString(localClientNum, "+activate", v11);
        if (BG_PlayerWeaponCountPrimaryTypes(&cgArray[0].predictedPlayerState) >= 2)
            v8 = UI_SafeTranslateString("PLATFORM_SWAPWEAPONS");
        else
            v8 = UI_SafeTranslateString("PLATFORM_PICKUPNEWWEAPON");
    }
    *secondaryString = (const char *)*((unsigned int *)v7 + 15);
    return UI_ReplaceConversionString(v8, v11);
}

char *__cdecl CG_GetUseString(int localClientNum)
{
    int cursorHintString; // r11
    const char *ConfigString; // r3
    const char *v4; // r31
    char *v5; // r3
    const char *v6; // r3
    char v8[264]; // [sp+50h] [-120h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cursorHintString = cgArray[0].cursorHintString;
    if (cgArray[0].cursorHintString < 0)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
            1227,
            0,
            "%s",
            "cgameGlob->cursorHintString >= 0");
        cursorHintString = cgArray[0].cursorHintString;
    }
    ConfigString = CL_GetConfigString(localClientNum, cursorHintString + 59);
    v4 = ConfigString;
    if (!ConfigString || !*ConfigString)
        return 0;
    if (!UI_GetKeyBindingLocalizedString(localClientNum, "+activate", v8))
    {
        v5 = UI_SafeTranslateString("KEY_USE");
        I_strncpyz(v8, v5, 256);
    }
    v6 = SEH_LocalizeTextMessage(v4, "Hint String", LOCMSG_SAFE);
    return UI_ReplaceConversionString(v6, v8);
}

void __cdecl CG_DrawCursorhint(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle)
{
    const char *translatedDisplayName; // r26
    char v10; // r11
    cg_s *LocalClientGlobals; // r22
    int *p_cursorHintTime; // r29
    Material *v13; // r7
    const float *v14; // r6
    int v15; // r5
    int v16; // r4
    long double v17; // fp2
    char *displayString; // r28
    double heightScale; // fp29
    double widthScale; // fp23
    double v21; // fp28
    __int64 v22; // r11
    double halfscale; // fp25
    double scale; // fp31
    __int64 v25; // r11
    double v26; // fp13
    double v27; // fp0
    long double v28; // fp2
    int cursorHintIcon; // r11
    char *UseString; // r3
    const char *v31; // r30
    __int64 v32; // r11
    double v33; // fp29
    __int64 v34; // r7
    double v35; // fp8
    double v36; // fp7
    double v37; // fp6
    double v38; // fp5
    double v39; // fp4
    int v40; // r29
    WeaponDef *weapDef; // r3
    WeaponDef *v42; // r30
    weaponIconRatioType_t hudIconRatio; // r4
    const char *v44; // r3
    char *WeaponUseString; // r3
    char *v46; // r3
    __int64 v47; // r11
    double length; // fp28
    __int64 v49; // r11
    int v50; // r8
    int v51; // r7
    double v52; // fp8
    double v53; // fp7
    double v54; // fp6
    double v55; // fp5
    double v56; // fp4
    double height; // fp24
    int secondaryLength; // r3
    double x; // fp22
    double y; // fp30
    int v61; // r8
    int v62; // r7
    double v63; // fp8
    double v64; // fp7
    double v65; // fp6
    double v66; // fp5
    double v67; // fp4
    const char *secondaryString; // r4
    int vertAlign; // r28
    const float *horzAlign; // r26
    double v71; // fp25
    const char *v72; // r3
    int v73; // r8
    int v74; // r7
    double v75; // fp8
    double v76; // fp7
    double v77; // fp6
    double v78; // fp5
    double v79; // fp4
    Material *v80; // r7
    const float *v81; // r6
    int v82; // r5
    int v83; // r4
    double v84; // fp3
    double v85; // fp27
    double v86; // fp26
    const float *v88; // r6
    int v89; // r5
    int v90; // r4
    float v91; // [sp+8h] [-228h]
    float v92; // [sp+8h] [-228h]
    float v93; // [sp+10h] [-220h]
    float v94; // [sp+10h] [-220h]
    float v95; // [sp+18h] [-218h]
    float v96; // [sp+18h] [-218h]
    float v97; // [sp+20h] [-210h]
    float v98; // [sp+20h] [-210h]
    float v99; // [sp+28h] [-208h]
    float v100; // [sp+28h] [-208h]
    float v101; // [sp+30h] [-200h]
    float v102; // [sp+30h] [-200h]
    float v103; // [sp+38h] [-1F8h]
    float v104; // [sp+38h] [-1F8h]
    float v105; // [sp+40h] [-1F0h]
    float v106; // [sp+40h] [-1F0h]
    float v107; // [sp+48h] [-1E8h]
    float v108; // [sp+48h] [-1E8h]
    float v109; // [sp+50h] [-1E0h]
    float v110; // [sp+50h] [-1E0h]
    float v111; // [sp+58h] [-1D8h]
    float v112; // [sp+58h] [-1D8h]
    float v113; // [sp+60h] [-1D0h]
    float v114; // [sp+60h] [-1D0h]
    float v115; // [sp+68h] [-1C8h]
    float v116; // [sp+68h] [-1C8h]
    float v117; // [sp+70h] [-1C0h]
    const char *v118; // [sp+78h] [-1B8h] BYREF
    char v119[336]; // [sp+80h] [-1B0h] BYREF

    if (cg_cursorHints->current.integer)
    {
        translatedDisplayName = 0;
        if (!cg_paused->current.integer || (v10 = 1, cg_drawpaused->current.enabled))
            v10 = 0;
        if (!v10)
        {
            CG_UpdateCursorHints(localClientNum);
            LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
            p_cursorHintTime = &LocalClientGlobals->cursorHintTime;
            if (!CG_FadeColor(
                LocalClientGlobals->time,
                LocalClientGlobals->cursorHintTime,
                LocalClientGlobals->cursorHintFade,
                100))
            {
                LocalClientGlobals->cursorHintIcon = 0;
                return;
            }
            displayString = 0;
            v118 = 0;
            heightScale = 1.0;
            LODWORD(v22) = cg_cursorHints->current.integer;
            widthScale = 1.0;
            v21 = 0.0;
            HIDWORD(v22) = 0x82000000;
            if ((int)v22 < 3)
            {
                if ((unsigned int)v22 == 2)
                {
                    HIDWORD(v25) = 1000 * (*p_cursorHintTime / 1000);
                    LODWORD(v25) = *p_cursorHintTime % 1000;
                    v26 = (float)v25;
                    v27 = 0.0099999998;
                }
                else
                {
                    LODWORD(v22) = LocalClientGlobals->time;
                    *(double *)&v17 = (float)((float)v22 * (float)0.0066666668);
                    v28 = sin(v17);
                    v26 = (float)((float)((float)*(double *)&v28 + (float)1.0) * (float)0.5);
                    v27 = 10.0;
                }
                scale = (float)((float)v26 * (float)v27);
                halfscale = (float)((float)((float)v26 * (float)v27) * (float)0.5);
            }
            else
            {
                halfscale = 0.0;
                scale = 0.0;
            }
            cursorHintIcon = LocalClientGlobals->cursorHintIcon;
            if (cursorHintIcon == 1)
            {
                if (LocalClientGlobals->cursorHintString >= 0)
                {
                    UseString = CG_GetUseString(localClientNum);
                    v31 = UseString;
                    if (UseString)
                    {
                        if (*UseString)
                        {
                            //LODWORD(v32) = UI_TextWidth(UseString, 0, font, fontscale);
                            //v33 = (float)v32;
                            //LODWORD(v34) = UI_TextHeight(font, fontscale);
                            //UI_DrawText(
                            //    &scrPlaceView[localClientNum],
                            //    v31,
                            //    0x7FFFFFFF,
                            //    font,
                            //    (float)((float)((float)v33 + (float)scale) * (float)-0.5),
                            //    (float)((float)((float)v34 * (float)0.5) + rect->y),
                            //    0x82000000,
                            //    68 * localClientNum,
                            //    fontscale,
                            //    (const float *)rect->horzAlign,
                            //    rect->vertAlign,
                            //    v39,
                            //    v38,
                            //    v37,
                            //    v36,
                            //    v35,
                            //    v91,
                            //    v93,
                            //    v95,
                            //    v97,
                            //    v99,
                            //    v101,
                            //    v103,
                            //    v105,
                            //    v107,
                            //    v109,
                            //    v111,
                            //    v113,
                            //    v115,
                            //    *(float *)&v34);

                            float length = UI_TextWidth(UseString, 0, font, fontscale);
                            float heighta = UI_TextHeight(font, fontscale);
                            float x = (scale + length) * -0.5;
                            float y = rect->y - rect->h * 0.5;
                            scale = heighta * 0.5 + rect->y;
                            UI_DrawText(
                                &scrPlaceView[localClientNum],
                                UseString,
                                0x7FFFFFFF,
                                font,
                                x,
                                scale,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                        }
                    }
                }
                return;
            }
            if (!cgMedia.hintMaterials[cursorHintIcon])
                return;
            if (cursorHintIcon < 5 || cursorHintIcon > 132)
            {
                if (LocalClientGlobals->cursorHintString < 0)
                {
                    if (cursorHintIcon != 3)
                    {
                    LABEL_39:
                        if (displayString && *displayString)
                        {
                            length = UI_TextWidth(displayString, 0, font, fontscale);
                            height = UI_TextHeight(font, fontscale);
                            if (translatedDisplayName && cg_weaponHintsCoD1Style->current.enabled)
                            {
                                secondaryLength = UI_TextWidth(translatedDisplayName, 0, font, fontscale);
                                x = (length + secondaryLength) * -0.5;
                                y = rect->y - rect->h * 0.5 * heightScale;
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
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

                                secondaryString = translatedDisplayName;

                                UI_DrawText(&scrPlaceView[localClientNum],
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

                                UI_DrawHandlePic(
                                    &scrPlaceView[localClientNum],
                                    (float)((float)((float)(rect->w * (float)widthScale) + (float)scale) * (float)-0.5),
                                    (float)((float)((float)height * (float)1.5) + (float)x),
                                    (float)((float)(rect->w * (float)widthScale) + (float)scale),
                                    (float)((float)(rect->h * (float)heightScale) + (float)scale),
                                    v83,
                                    v82,
                                    v81,
                                    v80);
                            }
                            else
                            {
                                float x = (rect->w * widthScale + scale + length) * -0.5;
                                float y = rect->y - rect->h * 0.5 * heightScale;
                                v16 = height * 0.5 + rect->y;
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
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

                                UI_DrawHandlePic(&scrPlaceView[localClientNum],
                                    x + length,
                                    y, 
                                    rect->w * widthScale + scale, 
                                    rect->h * heightScale + scale, 
                                    rect->horzAlign,
                                    rect->vertAlign, 
                                    color, 
                                    cgMedia.hintMaterials[cursorHintIcon]);
                            }
                        }
                        else
                        {
                            UI_DrawHandlePic(
                                &scrPlaceView[localClientNum],
                                (float)-(float)((float)((float)((float)(rect->w + (float)halfscale) + (float)v21) * (float)0.5) - rect->x),
                                (float)-(float)((float)((float)halfscale * (float)heightScale) - rect->y),
                                (float)((float)(rect->w * (float)widthScale) + (float)scale),
                                (float)((float)(rect->h * (float)heightScale) + (float)scale),
                                v16,
                                v15,
                                v14,
                                v13);
                        }
                        return;
                    }
                    UI_GetKeyBindingLocalizedString(localClientNum, "+activate", v119);
                    v46 = UI_SafeTranslateString("PLATFORM_PICKUPHEALTH");
                    WeaponUseString = UI_ReplaceConversionString(v46, v119);
                }
                else
                {
                    WeaponUseString = CG_GetUseString(localClientNum);
                }
            }
            else
            {
                v40 = cursorHintIcon - 4;
                weapDef = BG_GetWeaponDef(cursorHintIcon - 4);
                v42 = weapDef;
                if (weapDef->hudIcon)
                {
                    hudIconRatio = weapDef->hudIconRatio;
                    if (hudIconRatio)
                    {
                        if (hudIconRatio != WEAPON_ICON_RATIO_2TO1)
                        {
                            if (hudIconRatio != WEAPON_ICON_RATIO_4TO1)
                            {
                                v44 = va("hudIconRatio %d, weapon %s", hudIconRatio, weapDef->szInternalName);
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
                                    1357,
                                    0,
                                    "%s\n\t%s",
                                    "weapDef->hudIconRatio == WEAPON_ICON_RATIO_4TO1",
                                    v44);
                            }
                            heightScale = 0.5;
                        }
                        v21 = (float)(rect->w * (float)-0.5);
                        widthScale = 2.0;
                    }
                }
                if (v42->weapClass == WEAPCLASS_TURRET)
                {
                    if (LocalClientGlobals->cursorHintString >= 0)
                        displayString = CG_GetUseString(localClientNum);
                    translatedDisplayName = CG_GetLocalClientWeaponInfo(localClientNum, v40)->translatedDisplayName;
                    goto LABEL_39;
                }
                WeaponUseString = CG_GetWeaponUseString(localClientNum, &v118);
                translatedDisplayName = v118;
            }
            displayString = WeaponUseString;
            goto LABEL_39;
        }
    }
}

void __cdecl CG_DrawHoldBreathHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    int textStyle)
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

void __cdecl CG_DrawSaving(int localClientNum, const rectDef_s *rect, float *color, Material *material)
{
    ;
}

int __cdecl CG_OwnerDrawVisible(int flags)
{
    return 0;
}

void __cdecl CG_DrawTankBody(int localClientNum, rectDef_s *rect, Material *material, float *color)
{
    centity_s *Entity; // r3
    double v7; // fp31
    cgs_t *LocalClientStaticGlobals; // r3
    const float *v9; // r6
    int v10; // r5
    int v11; // r4

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) != 0
        && (cgArray[0].predictedPlayerState.eFlags & 0x80000) == 0)
    {
        Entity = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum);
        if (Entity->nextState.eType == 11 && (Entity->nextState.lerp.eFlags & 0x10000) != 0)
        {
            v7 = AngleSubtract(cgArray[0].refdefViewAngles[1], Entity->pose.angles[1]);
            LocalClientStaticGlobals = CG_GetLocalClientStaticGlobals(localClientNum);
            CG_DrawRotatedPic(
                &scrPlaceView[localClientNum],
                (float)((float)((float)((float)(compassSize->current.value - (float)1.0) * LocalClientStaticGlobals->compassWidth)
                    * (float)0.69999999)
                    + rect->x),
                rect->y,
                rect->w,
                rect->h,
                v11,
                v10,
                v7,
                v9,
                (Material *)(68 * localClientNum));
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawDeadQuote(
    const cg_s *cgameGlob,
    rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    int textStyle,
    double text_x,
    double text_y)
{
    const int *p_deadquoteStartTime; // r30
    int deadquoteStartTime; // r11
    const int *p_time; // r29
    const dvar_s *Var; // r3
    int v18; // r9
    float v20; // fp0
    const char *string; // r3
    const char *text; // r3
    rectDef_s textrect;

    p_deadquoteStartTime = &cgameGlob->deadquoteStartTime;
    deadquoteStartTime = cgameGlob->deadquoteStartTime;
    if (deadquoteStartTime)
    {
        p_time = &cgameGlob->time;
        if (deadquoteStartTime < cgameGlob->time)
        {
            Var = Dvar_FindVar("ui_deadquote");
            if (Var)
            {
                if ((*p_time - *p_deadquoteStartTime) > (hud_deathQuoteFadeTime->current.integer))
                {
                    v20 = 1.0f;
                }
                else
                {
                    v18 = *p_deadquoteStartTime;
                    v20 = (float)(*p_time - *p_deadquoteStartTime) / (float)(hud_deathQuoteFadeTime->current.integer);
                }
                string = Var->current.string;
                rect->h = v20;
                if (*string == 64)
                    ++string;
                text = SEH_LocalizeTextMessage(string, "game message", LOCMSG_SAFE);
                UI_DrawWrappedText(
                    &scrPlaceFull,
                    text,
                    rect,
                    font,
                    (float)(rect->x + (float)text_x),
                    (float)(rect->y + (float)text_y),
                    fontscale,
                    color,
                    textStyle,
                    true,
                    &textrect); // KISAKTODO: prob wrong args
            }
        }
    }
}

void __cdecl CG_DrawTankBarrel(int localClientNum, const rectDef_s *rect, Material *material, const float *color)
{
    centity_s *Entity; // r3
    const cpose_t *p_pose; // r28
    DObj_s *ClientDObj; // r4
    double v9; // fp31
    cgs_t *LocalClientStaticGlobals; // r3
    double x; // fp12
    int horzAlign; // r8
    int vertAlign; // r9
    const ScreenPlacement *v14; // r30
    double v15; // fp13
    const float *v16; // r5
    const float (*v17)[2]; // r4
    float w; // [sp+50h] [-B0h] BYREF
    float h; // [sp+54h] [-ACh] BYREF
    float y; // [sp+58h] [-A8h] BYREF
    float v21; // [sp+5Ch] [-A4h] BYREF
    float v22[4]; // [sp+60h] [-A0h] BYREF
    Material v23; // [sp+70h] [-90h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) != 0
        && (cgArray[0].predictedPlayerState.eFlags & 0x80000) == 0)
    {
        Entity = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum);
        p_pose = &Entity->pose;
        if (Entity->nextState.eType == 11 && (Entity->nextState.lerp.eFlags & 0x10000) != 0)
        {
            ClientDObj = Com_GetClientDObj(Entity->nextState.number, 0);
            if (ClientDObj)
            {
                if (CG_DObjGetWorldTagMatrix(
                    p_pose,
                    ClientDObj,
                    scr_const.tag_turret,
                    (float (*)[3]) & v23.stateBitsEntry[8],
                    (float *)&v23.stateBitsTable))
                {
                    AxisToAngles((const mat3x3&)v23.stateBitsEntry[8], v22);
                    v9 = AngleSubtract(cgArray[0].refdefViewAngles[1], v22[1]);
                    LocalClientStaticGlobals = CG_GetLocalClientStaticGlobals(localClientNum);
                    x = rect->x;
                    horzAlign = rect->horzAlign;
                    vertAlign = rect->vertAlign;
                    v14 = &scrPlaceView[localClientNum];
                    v15 = (float)((float)(compassSize->current.value - (float)1.0) * LocalClientStaticGlobals->compassWidth);
                    y = rect->y;
                    w = rect->w;
                    h = rect->h;
                    v21 = (float)((float)v15 * (float)0.69999999) + (float)x;
                    ScrPlace_ApplyRect(v14, &v21, &y, &w, &h, horzAlign, vertAlign);
                    *(float *)&v23.info.name = w * (float)0.5;
                    *(float *)v23.stateBitsEntry = *(float *)&v23.info.name;
                    *((float *)&v23.info.surfaceTypeBits + 1) = h * (float)0.25;
                    *(float *)&v23.stateBitsEntry[4] = *((float *)&v23.info.surfaceTypeBits + 1);
                    *(float *)&v23.info.gameFlags = -(float)(h * (float)0.75);
                    *((float *)&v23.info.drawSurf.packed + 1) = *(float *)&v23.info.gameFlags;
                    *(float *)&v23.info.drawSurf.fields = -*(float *)&v23.info.name;
                    v23.info.surfaceTypeBits = HIDWORD(v23.info.drawSurf.packed);
                    CG_DrawRotatedQuadPic(v14, v21, y, v17, v9, v16, &v23);
                }
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawInvalidCmdHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    int textStyle)
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

void __cdecl CG_ArchiveState(int localClientNum, MemoryFile *memFile)
{
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    CG_ArchiveWeaponInfo(memFile);
    CG_ArchiveViewInfo(cgArray, memFile);
    CG_ArchiveCameraShake(localClientNum, memFile);
    //CG_ArchiveActiveRumbles(memFile); // KISAKRUMBLE

    MemFile_ArchiveData(memFile, 4, &cgameGlob->healthFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->ammoFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->stanceFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->compassFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->offhandFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->sprintFadeTime);
    MemFile_ArchiveData(memFile, 8, cgameGlob->vehReticleOffset);
    MemFile_ArchiveData(memFile, 8, cgameGlob->vehReticleVel);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->vehReticleLockOnStartTime);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->vehReticleLockOnDuration);
    MemFile_ArchiveData(memFile, 4, &cgameGlob->vehReticleLockOnEntNum);
    MemFile_ArchiveData(memFile, 160, cgameGlob->visionSetFrom);
    MemFile_ArchiveData(memFile, 160, cgameGlob->visionSetTo);
    MemFile_ArchiveData(memFile, 160, cgameGlob->visionSetCurrent);
    MemFile_ArchiveData(memFile, 24, cgameGlob->visionSetLerpData);
    MemFile_ArchiveData(memFile, 64, cgameGlob->visionNameNaked);
    MemFile_ArchiveData(memFile, 64, cgameGlob->visionNameNight);
    MemFile_ArchiveData(memFile, 128, cgameGlob->hudElemSound);
}

float __cdecl CG_FadeHudMenu(int localClientNum, const dvar_s *fadeDvar, int displayStartTime, int duration)
{
    double v8; // fp1
    char v9; // r11
    int LocalClientTime; // r3
    float *v11; // r3

    if (CG_GetPredictedPlayerState(localClientNum)->pm_type == 4)
        goto LABEL_2;
    if (!cg_paused->current.integer || (v9 = 1, cg_drawpaused->current.enabled))
        v9 = 0;
    if (v9)
    {
    LABEL_2:
        v8 = 0.0;
    }
    else if (fadeDvar->current.value == 0.0)
    {
        v8 = 1.0;
    }
    else
    {
        LocalClientTime = CG_GetLocalClientTime(localClientNum);
        v11 = CG_FadeColor(LocalClientTime, displayStartTime, duration, 700);
        if (v11)
            v8 = v11[3];
        else
            v8 = 0.0;
    }
    return *((float *)&v8 + 1);
}

void __cdecl CG_DrawPlayerAmmoBackdrop(int localClientNum, const rectDef_s *rect, float *color, Material *material)
{
    long double v7; // fp2
    long double v8; // fp2
    const float *v9; // r6
    int v10; // r5
    int v11; // r4

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0)
    {
        if (CG_GetSelectedWeaponIndex(cgArray))
        {
            *(double *)&v7 = (float)((float)(hud_fade_ammodisplay->current.value * (float)1000.0) + (float)0.5);
            v8 = floor(v7);
            if (CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgArray[0].ammoFadeTime, (int)(float)*(double *)&v8) != 0.0)
            {
                if (!material)
                    Material_RegisterHandle("$default", 3);
                CG_CheckPlayerForLowAmmo(cgArray);
                UI_DrawHandlePic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    v11,
                    v10,
                    v9,
                    (Material *)(68 * localClientNum));
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerAmmoValue(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    Material *material,
    int textStyle)
{
    long double v12; // fp2
    long double v13; // fp2
    double v14; // fp1
    unsigned int SelectedWeaponIndex; // r3
    int v16; // r30
    char v17; // r25
    char v18; // r28
    char v19; // r21
    char v20; // r26
    int TotalAmmoReserve; // r27
    int v22; // r8
    int v23; // r7
    double v24; // fp8
    double v25; // fp7
    double v26; // fp6
    double v27; // fp5
    double v28; // fp4
    int v29; // r5
    __int64 v30; // r11
    double v31; // fp0
    int v32; // r8
    int v33; // r7
    double v34; // fp8
    double v35; // fp7
    double v36; // fp6
    double v37; // fp5
    double v38; // fp4
    double v39; // fp30
    __int64 v40; // r8
    double v41; // fp8
    double v42; // fp7
    double v43; // fp6
    double v44; // fp5
    double v45; // fp4
    __int64 v46; // r11
    int v47; // r7
    double v48; // fp8
    double v49; // fp7
    double v50; // fp6
    double v51; // fp5
    double v52; // fp4
    __int64 v53; // r11
    double v54; // fp30
    int v55; // r8
    int v56; // r7
    double v57; // fp8
    double v58; // fp7
    double v59; // fp6
    double v60; // fp5
    double v61; // fp4
    int v62; // r8
    int v63; // r7
    double v64; // fp8
    double v65; // fp7
    double v66; // fp6
    double v67; // fp5
    double v68; // fp4
    __int64 v69; // r11
    int v70; // r7
    double v71; // fp8
    double v72; // fp7
    double v73; // fp6
    double v74; // fp5
    double v75; // fp4
    float v76; // [sp+8h] [-328h]
    float v77; // [sp+8h] [-328h]
    float v78; // [sp+8h] [-328h]
    float v79; // [sp+8h] [-328h]
    float v80; // [sp+10h] [-320h]
    float v81; // [sp+10h] [-320h]
    float v82; // [sp+10h] [-320h]
    float v83; // [sp+10h] [-320h]
    float v84; // [sp+18h] [-318h]
    float v85; // [sp+18h] [-318h]
    float v86; // [sp+18h] [-318h]
    float v87; // [sp+18h] [-318h]
    float v88; // [sp+20h] [-310h]
    float v89; // [sp+20h] [-310h]
    float v90; // [sp+20h] [-310h]
    float v91; // [sp+20h] [-310h]
    float v92; // [sp+28h] [-308h]
    float v93; // [sp+28h] [-308h]
    float v94; // [sp+28h] [-308h]
    float v95; // [sp+28h] [-308h]
    float v96; // [sp+30h] [-300h]
    float v97; // [sp+30h] [-300h]
    float v98; // [sp+30h] [-300h]
    float v99; // [sp+30h] [-300h]
    float v100; // [sp+38h] [-2F8h]
    float v101; // [sp+38h] [-2F8h]
    float v102; // [sp+38h] [-2F8h]
    float v103; // [sp+38h] [-2F8h]
    float v104; // [sp+40h] [-2F0h]
    float v105; // [sp+40h] [-2F0h]
    float v106; // [sp+40h] [-2F0h]
    float v107; // [sp+40h] [-2F0h]
    float v108; // [sp+48h] [-2E8h]
    float v109; // [sp+48h] [-2E8h]
    float v110; // [sp+48h] [-2E8h]
    float v111; // [sp+48h] [-2E8h]
    float v112; // [sp+50h] [-2E0h]
    float v113; // [sp+50h] [-2E0h]
    float v114; // [sp+50h] [-2E0h]
    float v115; // [sp+50h] [-2E0h]
    float v116; // [sp+58h] [-2D8h]
    float v117; // [sp+58h] [-2D8h]
    float v118; // [sp+58h] [-2D8h]
    float v119; // [sp+58h] [-2D8h]
    float v120; // [sp+60h] [-2D0h]
    float v121; // [sp+60h] [-2D0h]
    float v122; // [sp+60h] [-2D0h]
    float v123; // [sp+60h] [-2D0h]
    float v124; // [sp+68h] [-2C8h]
    float v125; // [sp+68h] [-2C8h]
    float v126; // [sp+68h] [-2C8h]
    float v127; // [sp+68h] [-2C8h]
    int v128; // [sp+70h] [-2C0h]
    float v129; // [sp+70h] [-2C0h]
    float v130; // [sp+70h] [-2C0h]
    float v131; // [sp+80h] [-2B0h] BYREF
    float v132; // [sp+84h] [-2ACh]
    float v133; // [sp+88h] [-2A8h]
    float v134; // [sp+8Ch] [-2A4h]
    float v135; // [sp+90h] [-2A0h]
    float v136; // [sp+94h] [-29Ch]
    float v137; // [sp+98h] [-298h]
    float v138; // [sp+9Ch] [-294h]
    char v139[256]; // [sp+A0h] [-290h] BYREF
    char v140[264]; // [sp+1A0h] [-190h] BYREF

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->predictedPlayerState.eFlags & 0x20000)
    {
        return;
    }

#if 0
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0)
    {
        *(double *)&v12 = (float)((float)(hud_fade_ammodisplay->current.value * (float)1000.0) + (float)0.5);
        v13 = floor(v12);
        v128 = (int)(float)*(double *)&v13;
        v14 = CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgArray[0].ammoFadeTime, v128);
        *((float *)&material->info.drawSurf.packed + 1) = v14;
        if (v14 != 0.0)
        {
            SelectedWeaponIndex = CG_GetSelectedWeaponIndex(cgArray);
            v16 = SelectedWeaponIndex;
            if (SelectedWeaponIndex)
            {
                v17 = 1;
                v18 = 1;
                v19 = 0;
                v20 = 0;
                TotalAmmoReserve = BG_GetTotalAmmoReserve(&cgArray[0].predictedPlayerState, SelectedWeaponIndex);
                if (BG_WeaponIsClipOnly(v16))
                {
                    v29 = -1;
                }
                else
                {
                    v29 = cgArray[0].predictedPlayerState.ammoclip[BG_ClipForWeapon(v16)];
                    if (v29 >= 0)
                    {
                    LABEL_10:
                        if (v29 > 999)
                            v29 = 999;
                        if (TotalAmmoReserve < 0)
                            v17 = 0;
                        if (TotalAmmoReserve > 999)
                            TotalAmmoReserve = 999;
                        if (v18)
                        {
                            sprintf(v139, "%2i", v29);
                            v20 = CG_CheckPlayerForLowClip(cgArray);
                        }
                        if (v17)
                        {
                            sprintf(v140, "%3i", TotalAmmoReserve);
                            v19 = CG_CheckPlayerForLowAmmo(cgArray);
                        }
                        if (v20)
                        {
                            v30 = __PAIR64__(cgArray[0].time, cgArray[0].lastClipFlashTime);
                            if (cgArray[0].lastClipFlashTime > cgArray[0].time
                                || cgArray[0].lastClipFlashTime + 800 < cgArray[0].time)
                            {
                                LODWORD(v30) = cgArray[0].time;
                                cgArray[0].lastClipFlashTime = cgArray[0].time;
                            }
                            v31 = *((float *)&material->info.drawSurf.packed + 1);
                            v135 = 0.88999999;
                            LODWORD(v30) = v30 - cgArray[0].time + 800;
                            v136 = 0.18000001;
                            v137 = 0.0099999998;
                            v128 = HIDWORD(v30);
                            v138 = (float)v30 * (float)0.00125;
                            if (v31 < v138)
                                v138 = v31;
                        }
                        if (v19)
                        {
                            v131 = 0.88999999;
                            v132 = 0.18000001;
                            v133 = 0.0099999998;
                        }
                        else
                        {
                            v131 = *(float *)&material->info.name;
                            v132 = *(float *)&material->info.gameFlags;
                            v133 = *(float *)&material->info.drawSurf.fields;
                        }
                        v134 = *((float *)&material->info.drawSurf.packed + 1);
                        if (v18)
                        {
                            if (v17)
                            {
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    v139,
                                    0x7FFFFFFF,
                                    font,
                                    rect->x,
                                    rect->y,
                                    v23,
                                    v22,
                                    scale,
                                    (const float *)rect->horzAlign,
                                    rect->vertAlign,
                                    v28,
                                    v27,
                                    v26,
                                    v25,
                                    v24,
                                    v76,
                                    v80,
                                    v84,
                                    v88,
                                    v92,
                                    v96,
                                    v100,
                                    v104,
                                    v108,
                                    v112,
                                    v116,
                                    v120,
                                    v124,
                                    *(float *)&v128);
                                if (v20)
                                    UI_DrawText(
                                        &scrPlaceView[localClientNum],
                                        v139,
                                        0x7FFFFFFF,
                                        font,
                                        rect->x,
                                        rect->y,
                                        v33,
                                        v32,
                                        scale,
                                        (const float *)rect->horzAlign,
                                        rect->vertAlign,
                                        v38,
                                        v37,
                                        v36,
                                        v35,
                                        v34,
                                        v77,
                                        v81,
                                        v85,
                                        v89,
                                        v93,
                                        v97,
                                        v101,
                                        v105,
                                        v109,
                                        v113,
                                        v117,
                                        v121,
                                        v125,
                                        v129);
                                v39 = (float)(rect->x + rect->w);
                                LODWORD(v40) = UI_TextWidth(v140, 0, font, scale);
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    v140,
                                    0x7FFFFFFF,
                                    font,
                                    (float)((float)v39 - (float)v40),
                                    rect->y,
                                    SHIDWORD(v40),
                                    v40,
                                    scale,
                                    (const float *)rect->horzAlign,
                                    rect->vertAlign,
                                    v45,
                                    v44,
                                    v43,
                                    v42,
                                    v41,
                                    v77,
                                    v81,
                                    v85,
                                    v89,
                                    v93,
                                    v97,
                                    v101,
                                    v105,
                                    v109,
                                    v113,
                                    v117,
                                    v121,
                                    v125,
                                    *(float *)&v40);
                                LODWORD(v46) = UI_TextWidth("|", 0, font, scale);
                                HIDWORD(v46) = rect->vertAlign;
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    "|",
                                    0x7FFFFFFF,
                                    font,
                                    (float)((float)((float)((float)(rect->w - (float)v46) * (float)0.5) + rect->x) - (float)5.0),
                                    rect->y,
                                    v47,
                                    (int)&v131,
                                    scale,
                                    (const float *)rect->horzAlign,
                                    SHIDWORD(v46),
                                    v52,
                                    v51,
                                    v50,
                                    v49,
                                    v48,
                                    v78,
                                    v82,
                                    v86,
                                    v90,
                                    v94,
                                    v98,
                                    v102,
                                    v106,
                                    v110,
                                    v114,
                                    v118,
                                    v122,
                                    v126,
                                    *(float *)&v46);
                            }
                            else
                            {
                                LODWORD(v53) = UI_TextWidth(v139, 0, font, scale);
                                HIDWORD(v53) = rect->vertAlign;
                                v54 = (float)((float)((float)(rect->w - (float)v53) * (float)0.5) + rect->x);
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    v139,
                                    0x7FFFFFFF,
                                    font,
                                    v54,
                                    rect->y,
                                    v56,
                                    v55,
                                    scale,
                                    (const float *)rect->horzAlign,
                                    SHIDWORD(v53),
                                    v61,
                                    v60,
                                    v59,
                                    v58,
                                    v57,
                                    v76,
                                    v80,
                                    v84,
                                    v88,
                                    v92,
                                    v96,
                                    v100,
                                    v104,
                                    v108,
                                    v112,
                                    v116,
                                    v120,
                                    v124,
                                    *(float *)&v53);
                                if (v20)
                                    UI_DrawText(
                                        &scrPlaceView[localClientNum],
                                        v139,
                                        0x7FFFFFFF,
                                        font,
                                        v54,
                                        rect->y,
                                        v63,
                                        v62,
                                        scale,
                                        (const float *)rect->horzAlign,
                                        rect->vertAlign,
                                        v68,
                                        v67,
                                        v66,
                                        v65,
                                        v64,
                                        v79,
                                        v83,
                                        v87,
                                        v91,
                                        v95,
                                        v99,
                                        v103,
                                        v107,
                                        v111,
                                        v115,
                                        v119,
                                        v123,
                                        v127,
                                        v130);
                            }
                        }
                        else if (v17)
                        {
                            LODWORD(v69) = UI_TextWidth(v140, 0, font, scale);
                            HIDWORD(v69) = rect->vertAlign;
                            UI_DrawText(
                                &scrPlaceView[localClientNum],
                                v140,
                                0x7FFFFFFF,
                                font,
                                (float)((float)((float)(rect->w - (float)v69) * (float)0.5) + rect->x),
                                rect->y,
                                v70,
                                (int)&v131,
                                scale,
                                (const float *)rect->horzAlign,
                                SHIDWORD(v69),
                                v75,
                                v74,
                                v73,
                                v72,
                                v71,
                                v76,
                                v80,
                                v84,
                                v88,
                                v92,
                                v96,
                                v100,
                                v104,
                                v108,
                                v112,
                                v116,
                                v120,
                                v124,
                                *(float *)&v69);
                        }
                        return;
                    }
                }
                v18 = 0;
                goto LABEL_10;
            }
        }
    }
#else
double v7; // [esp+2Ch] [ebp-274h]
float v8; // [esp+3Ch] [ebp-264h]
float ammoColor[5]; // [esp+4Ch] [ebp-254h] BYREF
const ScreenPlacement *scrPlace; // [esp+60h] [ebp-240h]
int32_t ammoVal; // [esp+64h] [ebp-23Ch]
//cg_s *cgameGlob; // [esp+68h] [ebp-238h]
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
#endif
}

void __cdecl CG_DrawPlayerWeaponName(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle)
{
    unsigned int SelectedWeaponIndex; // r3
    unsigned int v11; // r28
    weaponInfo_s *LocalClientWeaponInfo; // r31
    WeaponDef *WeaponDef; // r3
    const char *translatedDisplayName; // r4
    const char *v15; // r3
    const char *string; // r31

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0
        && (cgArray[0].predictedPlayerState.weapFlags & 0x80) == 0
        && CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgArray[0].weaponSelectTime, 1800) != 0.0)
    {
        SelectedWeaponIndex = CG_GetSelectedWeaponIndex(cgArray);
        v11 = SelectedWeaponIndex;
        if (SelectedWeaponIndex)
        {
            LocalClientWeaponInfo = CG_GetLocalClientWeaponInfo(localClientNum, SelectedWeaponIndex);
            WeaponDef = BG_GetWeaponDef(v11);
            translatedDisplayName = LocalClientWeaponInfo->translatedDisplayName;
            if (*WeaponDef->szModeName)
                v15 = va("%s / %s", translatedDisplayName, LocalClientWeaponInfo->translatedModename);
            else
                v15 = va("%s", translatedDisplayName);
            string = v15;
            float x = (rect->w + rect->x) - UI_TextWidth(string, 0, font, scale) - 28.0f;

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
                color,
                textStyle);
        }
    }
}

void __cdecl CG_DrawPlayerWeaponNameBack(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    Material *material)
{
    unsigned int weapIndex; // r3
    weaponInfo_s *weapInfo; // r30
    WeaponDef *WeaponDef; // r3
    const char *string; // r3

    float drawColor[4]; // [esp+30h] [ebp-20h] BYREF

    if (cg_paused->current.integer && !cg_drawpaused->current.enabled)
    {
        return;
    }

    if (!material)
        material = Material_RegisterHandle("$default", 3);

    const cg_s *cgameGlob;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000))
    {
        return;
    }

    drawColor[3] = CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgameGlob->weaponSelectTime, 1800);
    if (drawColor[3] != 0.0f)
    {
        weapIndex = CG_GetSelectedWeaponIndex(cgArray);
        if (weapIndex)
        {
            weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weapIndex);
            WeaponDef = BG_GetWeaponDef(weapIndex);

            if (*WeaponDef->szModeName)
                string = va("%s / %s", weapInfo->translatedDisplayName, weapInfo->translatedModename);
            else
                string = va("%s", weapInfo->translatedDisplayName);

            float w = UI_TextWidth(string, 0, font, scale) + 28.0 + 8.0;
            float x = rect->x + rect->w - w;

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

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerStance(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Font_s *font,
    float scale,
    int32_t textStyle)
{
#if 0
    long double v11; // fp2
    long double v12; // fp2
    double v13; // fp31
    int v14; // r7
    const float *v15; // r6
    const float *v16; // r5
    cgs_t *LocalClientStaticGlobals; // r3
    double y; // fp27
    double x; // fp12
    double v20; // fp13
    double v21; // fp30
    int proneBlockedEndTime; // r11
    bool v23; // cr57
    const char *v24; // r3
    char *v25; // r30
    int v26; // r3
    __int128 v27; // r11
    double v28; // fp31
    long double v29; // fp2
    long double v30; // fp2
    int v31; // r7
    double v32; // fp8
    double v33; // fp7
    double v34; // fp6
    double v35; // fp5
    double v36; // fp4
    float v37; // [sp+8h] [-F8h]
    float v38; // [sp+10h] [-F0h]
    float v39; // [sp+18h] [-E8h]
    float v40; // [sp+20h] [-E0h]
    float v41; // [sp+28h] [-D8h]
    float v42; // [sp+30h] [-D0h]
    float v43; // [sp+38h] [-C8h]
    float v44; // [sp+40h] [-C0h]
    float v45; // [sp+48h] [-B8h]
    float v46; // [sp+50h] [-B0h]
    float v47; // [sp+58h] [-A8h]
    float v48; // [sp+60h] [-A0h]
    float v49; // [sp+68h] [-98h]
    float v50; // [sp+70h] [-90h]
    float v51[3]; // [sp+80h] [-80h] BYREF
    float v52; // [sp+8Ch] [-74h]

    if (hud_showStance->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        *(double *)&v11 = (float)((float)(hud_fade_stance->current.value * (float)1000.0) + (float)0.5);
        v12 = floor(v11);
        v13 = CG_FadeHudMenu(localClientNum, hud_fade_stance, cgArray[0].stanceFadeTime, (int)(float)*(double *)&v12);
        if (v13 != 0.0
            && ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0
                || (cgArray[0].predictedPlayerState.eFlags & 0x80000) != 0))
        {
            if (cgArray[0].lastStanceChangeTime > cgArray[0].time
                || cgArray[0].lastStance != (cgArray[0].predictedPlayerState.pm_flags & 3))
            {
                cgArray[0].lastStanceChangeTime = cgArray[0].time;
            }
            cgArray[0].lastStance = cgArray[0].predictedPlayerState.pm_flags & 3;
            LocalClientStaticGlobals = CG_GetLocalClientStaticGlobals(localClientNum);
            y = rect->y;
            x = rect->x;
            v20 = (float)((float)(compassSize->current.value - (float)1.0) * LocalClientStaticGlobals->compassWidth);
            v51[0] = *(float *)&color->fontName;
            v51[1] = *(float *)&color->pixelHeight;
            v51[2] = *(float *)&color->glyphCount;
            v21 = (float)((float)((float)v20 * (float)0.69999999) + (float)x);
            if (cg_hudStanceHintPrints->current.enabled && cgArray[0].lastStanceChangeTime + 3000 > cgArray[0].time)
                CG_DrawStanceHintPrints(
                    localClientNum,
                    rect,
                    (float)((float)((float)v20 * (float)0.69999999) + (float)x),
                    v16,
                    v13,
                    color,
                    scale,
                    v14,
                    font);
            v52 = *(float *)&color->material * (float)v13;
            CG_DrawStanceIcon(localClientNum, rect, v51, v21, y, v13, v15);
            proneBlockedEndTime = cgArray[0].proneBlockedEndTime;
            if ((cgArray[0].predictedPlayerState.pm_flags & 0x1000) != 0)
            {
                v23 = cgArray[0].proneBlockedEndTime > cgArray[0].time;
                if (cgArray[0].proneBlockedEndTime >= cgArray[0].time)
                    goto LABEL_17;
                proneBlockedEndTime = cgArray[0].time + 1500;
                cgArray[0].proneBlockedEndTime = cgArray[0].time + 1500;
            }
            v23 = proneBlockedEndTime > cgArray[0].time;
        LABEL_17:
            if (v23)
            {
                if (BG_WeaponBlocksProne(cgArray[0].predictedPlayerState.weapon))
                    v24 = "CGAME_PRONE_BLOCKED_WEAPON";
                else
                    v24 = "CGAME_PRONE_BLOCKED";
                v25 = UI_SafeTranslateString(v24);
                v26 = UI_TextWidth(v25, 0, font, scale);
                LODWORD(v27) = cgArray[0].proneBlockedEndTime - cgArray[0].time;
                DWORD2(v27) = v26;
                v50 = *(float *)&v26;
                v28 = (float)((float)*(__int64 *)((char *)&v27 + 4) * (float)0.5);
                *(double *)&v29 = (float)((float)((float)((float)(__int64)v27 * (float)0.00066666666) * (float)540.0)
                    * (float)0.017453292);
                v30 = sin(v29);
                v52 = I_fabs((float)*(double *)&v30);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    v25,
                    0x7FFFFFFF,
                    font,
                    -v28,
                    -260.0,
                    v31,
                    68 * localClientNum,
                    scale,
                    (const float *)7,
                    3,
                    v36,
                    v35,
                    v34,
                    v33,
                    v32,
                    v37,
                    v38,
                    v39,
                    v40,
                    v41,
                    v42,
                    v43,
                    v44,
                    v45,
                    v46,
                    v47,
                    v48,
                    v49,
                    v50);
            }
        }
    }
#else
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
        if (cgameGlob->lastStance != (cgameGlob->predictedPlayerState.pm_flags & 3))
            cgameGlob->lastStanceChangeTime = cgameGlob->time;
    }
    else
    {
        cgameGlob->lastStanceChangeTime = 0;
    }
    cgameGlob->lastStance = cgameGlob->predictedPlayerState.pm_flags & 3;
    drawColor[4] = 1.4025731e-38f;
    x = (compassSize->current.value - 1.0f) * cgs->compassWidth * 0.699999988079071f + rect->x;
    y = rect->y;
    KISAK_NULLSUB();
    drawColor[0] = color[0];
    drawColor[1] = color[1];
    drawColor[2] = color[2];
    if ((cgameGlob->predictedPlayerState.pm_flags & 0x1000) != 0 && cgameGlob->proneBlockedEndTime < cgameGlob->time)
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
            //cg_hudProneY->current.value,
            -260.0f, // KISAKTODO: accurate??
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
#endif
}

void __cdecl CG_DrawPlayerSprintBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    float drawColor[4]; // [esp+48h] [ebp-14h] BYREF
    float fadeAlpha; // [esp+58h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((cgameGlob->predictedPlayerState.eFlags & 0x20000) == 0 || (cgameGlob->predictedPlayerState.eFlags & 0x80000) != 0)
    {
        fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgameGlob->sprintFadeTime, (int)(hud_fade_sprint->current.value * 1000.0f));
        if (fadeAlpha != 0.0)
        {
            drawColor[0] = color[0];
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
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                drawColor,
                material);
        }
    }
}

void __cdecl CG_DrawPlayerSprintMeter(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    float x; // fp29
    float y; // fp27
    float w;
    float h; // fp26

    cg_s *cgameGlob;
    float fadeAlpha; // [esp+74h] [ebp-8h]
    float drawColor[4]; // [esp+4Ch] [ebp-30h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((cgameGlob->predictedPlayerState.eFlags & 0x20000) == 0 || (cgameGlob->predictedPlayerState.eFlags & 0x80000) != 0)
    {
        fadeAlpha = CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgameGlob->sprintFadeTime, (int)(hud_fade_sprint->current.value * 1000.0f));
        if (fadeAlpha != 0.0f)
        {
            int32_t sprintLeft = PM_GetSprintLeft(&cgameGlob->predictedPlayerState, cgameGlob->time);
            int32_t maxSprint = BG_GetMaxSprintTime(&cgameGlob->predictedPlayerState);
            float sprint = sprintLeft / maxSprint;

            if (sprint > 0.0f)
            {
                x = rect->x;
                y = rect->y;
                w = rect->w * sprint;
                h = rect->h;
                if (!material)
                    material = cgMedia.whiteMaterial;
                CG_CalcPlayerSprintColor(cgameGlob, &cgameGlob->predictedPlayerState, color);
                drawColor[0] = color[0];
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

void __cdecl CG_DrawPlayerBarHealthBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color)
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

// local variable allocation has failed, the output may be wrong!
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
    const cg_s *LocalClientGlobals; // r28
    const playerState_s *ps; // r29
    int v65; // r7
    float *v66; // r6
    const float *v67; // r5
    long double v68; // fp2
    bool v69; // [sp+Bh] [-D5h]
    rectDef_s rect; // [sp+60h] [-80h] BYREF

    //textAlignMode = *(_QWORD *)&parentRect;
    //a18 = *(__int64 *)((char *)&vertAlign + 4);
    //a19 = vertAlign;

    if (!cg_drawHUD->current.enabled || !hud_drawHUD->current.enabled)
    {
        return;
    }

    LocalClientGlobals = CG_GetLocalClientGlobals(0);
    ps = CG_GetPredictedPlayerState(0);

    iassert(ps);
    //iassert(ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH);

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
    case 95:
        CG_DrawTankBody(localClientNum, &rect, material, color);
        break;
    case 96:
        CG_DrawTankBarrel(localClientNum, &rect, material, color);
        break;
    case 97:
        CG_DrawDeadQuote(LocalClientGlobals, &rect, font, scale, color, textStyle, text_x, text_y);
        break;
    case 98:
        CG_DrawPlayerBarHealthBack(localClientNum, &rect, material, color);
        break;
    case 99:
        CG_DrawObjectiveHeader(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 100:
        CG_DrawObjectiveList(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 101:
        CG_DrawObjectiveBackdrop(LocalClientGlobals, color);
        break;
    case 102:
        CG_DrawPausedMenuLine(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 103:
    case 104:
        CG_DrawOffHandIcon(localClientNum, &rect, scale, color, material, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 105:
    case 106:
        CG_DrawOffHandAmmo(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 107:
    case 108:
        CG_DrawOffHandName(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_CLASS_FRAG_GRENADE);
        break;
    case 109:
    case 110:
        CG_DrawOffHandHighlight(localClientNum, &rect, scale, color, material, OFFHAND_CLASS_FRAG_GRENADE);
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
            1);
        break;
    case 150:
        CG_CompassDrawPlayer(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 151:
        CG_CompassDrawPlayerBack(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 152:
        CG_CompassDrawPlayerPointers_SP(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 153:
        CG_CompassDrawActors(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
        break;
    case 154:
        CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color, 1);
        break;
    case 155:
        CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color, 2);
        break;
    case 156:
        CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color, 3);
        break;
    case 157:
        CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color, 4);
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
    case 164:
        CG_CompassDrawGoalDistance(localClientNum, &rect, font, scale, color, textStyle);
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
    case 180:
        CG_CompassDrawPlayerBack(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 181:
        CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 182:
        CG_CompassDrawPlayerPointers_SP(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 183:
        CG_CompassDrawPlayer(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 184:
        CG_CompassDrawActors(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 186:
        CG_CompassDrawPlayerMapLocationSelector(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 187:
        CG_CompassDrawBorder(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
        break;
    case 190:
        CG_DrawVehicleReticle(localClientNum, &rect, color);
        break;
    case 191:
        CG_DrawVehicleTargets(localClientNum, &rect, color, material);
        break;
    case 192:
        CG_DrawJavelinTargets(localClientNum, &rect, color, material);
        break;
    default:
        return;
    }
}

