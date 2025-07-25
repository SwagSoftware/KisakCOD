#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_newdraw.h"
#include "cg_main.h"

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

void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, DvarValue *color)
{
    float frac; // [esp+8h] [ebp-18h]
    const DvarValue *p_current; // [esp+Ch] [ebp-14h]
    int32_t sprintLeft; // [esp+18h] [ebp-8h]
    int32_t maxSprint; // [esp+1Ch] [ebp-4h]

    maxSprint = BG_GetMaxSprintTime(ps);
    if (ps->pm_type == PM_DEAD || !maxSprint)
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

void __cdecl CG_DrawStanceIcon(
    int localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    double x,
    double y,
    double fadeAlpha,
    const float *a7)
{
    double w; // fp31
    double h; // fp30
    Material *v15; // r7
    const ScreenPlacement *v16; // r30
    Material *v17; // r7
    const float *v18; // r6
    int v19; // r5
    int v20; // r4
    __int64 v21; // r11

    if (!rect)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 539, 0, "%s", "rect");
    w = rect->w;
    h = rect->h;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v15 = (Material *)(68 * localClientNum);
    v16 = &scrPlaceView[localClientNum];
    UI_DrawHandlePic(v16, x, y, w, h, (int)rect, (int)drawColor, a7, v15);
    if (cgArray[0].lastStanceChangeTime + 1000 > cgArray[0].time)
    {
        Dvar_GetUnpackedColor(cg_hudStanceFlash, drawColor);
        HIDWORD(v21) = cgArray[0].lastStanceChangeTime;
        LODWORD(v21) = cgArray[0].lastStanceChangeTime - cgArray[0].time + 1000;
        drawColor[3] = (float)((float)v21 * (float)0.001) * (float)0.80000001;
        if (fadeAlpha < (float)((float)((float)v21 * (float)0.001) * (float)0.80000001))
            drawColor[3] = fadeAlpha;
        UI_DrawHandlePic(v16, x, y, w, h, v20, v19, v18, v17);
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawStanceHintPrints(
    int localClientNum,
    const rectDef_s *rect,
    double x,
    const float *color,
    double fadeAlpha,
    Font_s *font,
    double scale,
    int textStyle,
    Font_s *a9)
{
    int v9; // r30 OVERLAPPED
    int v10; // r31 OVERLAPPED
    double v14; // fp0
    __int64 v16; // r11
    double v17; // fp0
    __int64 v18; // r11
    int v19; // r23
    int v20; // r22
    const char **v21; // r27
    int v22; // r25
    char lastStance; // r11
    double v24; // fp29
    int v25; // r28
    int *v26; // r10
    const char *v27; // r30
    int v28; // r29
    int IsCommandBound; // r3
    int *v30; // r10
    double v31; // fp31
    const char *v32; // r4
    char *v33; // r3
    char *v34; // r3
    int v35; // r7
    double v36; // fp8
    double v37; // fp7
    double v38; // fp6
    double v39; // fp5
    double v40; // fp4
    float v41; // [sp+8h] [-348h]
    float v42; // [sp+10h] [-340h]
    float v43; // [sp+18h] [-338h]
    float v44; // [sp+20h] [-330h]
    float v45; // [sp+28h] [-328h]
    float v46; // [sp+30h] [-320h]
    float v47; // [sp+38h] [-318h]
    float v48; // [sp+40h] [-310h]
    float v49; // [sp+48h] [-308h]
    float v50; // [sp+50h] [-300h]
    float v51; // [sp+58h] [-2F8h]
    float v52; // [sp+60h] [-2F0h]
    float v53; // [sp+68h] [-2E8h]
    __int64 v54; // [sp+70h] [-2E0h] BYREF
    __int64 *v55; // [sp+7Ch] [-2D4h]
    const char *v56; // [sp+80h] [-2D0h]
    _BYTE *v57; // [sp+84h] [-2CCh]
    __int64 *v58; // [sp+88h] [-2C8h]
    _BYTE *v59; // [sp+8Ch] [-2C4h]
    _BYTE *v60; // [sp+90h] [-2C0h]
    float v61[3]; // [sp+A0h] [-2B0h] BYREF
    float v62; // [sp+ACh] [-2A4h]
    __int64 *v63; // [sp+B0h] [-2A0h]
    unsigned int v64[6]; // [sp+B8h] [-298h] BYREF
    unsigned int v65[2]; // [sp+D0h] [-280h] BYREF
    __int64 v66; // [sp+D8h] [-278h] BYREF
    __int64 v67; // [sp+E0h] [-270h]
    int v68; // [sp+E8h] [-268h]
    _BYTE v69[20]; // [sp+ECh] [-264h] BYREF
    const char *v70; // [sp+100h] [-250h]
    int v71; // [sp+104h] [-24Ch]
    __int64 v72; // [sp+108h] [-248h] BYREF
    __int64 v73; // [sp+110h] [-240h]
    int v74; // [sp+120h] [-230h] BYREF
    _BYTE v75[20]; // [sp+124h] [-22Ch] BYREF
    const char *v76; // [sp+138h] [-218h]
    int v77; // [sp+13Ch] [-214h]
    __int64 v78; // [sp+140h] [-210h]
    __int64 v79; // [sp+148h] [-208h]
    int v80; // [sp+150h] [-200h]
    int v81; // [sp+154h] [-1FCh]
    int v82; // [sp+158h] [-1F8h]
    int v83; // [sp+15Ch] [-1F4h]
    int v84; // [sp+160h] [-1F0h]
    int v85; // [sp+164h] [-1ECh]
    unsigned int v86[8]; // [sp+170h] [-1E0h] BYREF
    __int64 v87; // [sp+190h] [-1C0h] BYREF
    __int64 v88; // [sp+198h] [-1B8h]
    int v89; // [sp+1A0h] [-1B0h]
    _BYTE v90[28]; // [sp+1A4h] [-1ACh] BYREF
    char v91[304]; // [sp+1C0h] [-190h] BYREF

    v10 = 0;
    v58 = &v66;
    v65[0] = "raisestance";
    v59 = v69;
    HIDWORD(v54) = "raisestance";
    v63 = &v87;
    v56 = "lowerstance";
    v60 = v75;
    v57 = v90;
    v55 = &v72;
    memset(v86, 0, 24);
    v65[1] = 0;
    v74 = 0;
    v86[6] = "lowerstance";
    v86[7] = 0;
    v66 = *(_QWORD *)(&v9 - 1);
    v67 = *(_QWORD *)(&v9 - 1);
    v68 = 0;
    memset(v69, 0, sizeof(v69));
    v87 = *(_QWORD *)(&v9 - 1);
    v88 = *(_QWORD *)(&v9 - 1);
    v89 = 0;
    memset(v75, 0, sizeof(v75));
    v70 = "lowerstance";
    memset(v90, 0, 20);
    v71 = 0;
    v72 = *(_QWORD *)(&v9 - 1);
    v73 = *(_QWORD *)(&v9 - 1);
    v76 = "raisestance";
    v77 = 0;
    v61[0] = *(float *)&font->fontName;
    v78 = *(_QWORD *)(&v9 - 1);
    v79 = *(_QWORD *)(&v9 - 1);
    v61[1] = *(float *)&font->pixelHeight;
    v14 = *(float *)&font->glyphCount;
    v80 = 0;
    v81 = 0;
    v82 = 0;
    v61[2] = v14;
    v83 = 0;
    v84 = 0;
    v64[1] = "PLATFORM_STANCEHINT_CROUCH";
    v64[0] = "PLATFORM_STANCEHINT_STAND";
    v64[2] = "PLATFORM_STANCEHINT_PRONE";
    v85 = 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    HIDWORD(v16) = cgArray[0].lastStanceChangeTime;
    if (cgArray[0].lastStanceChangeTime - cgArray[0].time + 3000 <= 1000)
    {
        LODWORD(v16) = cgArray[0].lastStanceChangeTime - cgArray[0].time + 3000;
        v54 = v16;
        v17 = (float)((float)v16 * (float)0.001);
    }
    else
    {
        v17 = 1.0;
    }
    v62 = v17;
    LODWORD(v18) = UI_TextHeight(a9, scale);
    v19 = 0;
    v20 = 0;
    v21 = (const char **)&v54;
    v22 = 0;
    v54 = v18;
    v24 = (float)v18;
    lastStance = cgArray[0].lastStance;
    do
    {
        *v21 = 0;
        v25 = 0;
        if ((lastStance & 1) != 0)
        {
            v26 = &v74;
        }
        else
        {
            v26 = v65;
            if ((lastStance & 2) == 0)
                v26 = v86;
        }
        v27 = (const char *)v26[v22];
        v28 = v20;
        while (v27)
        {
            IsCommandBound = Key_IsCommandBound(localClientNum, v27);
            lastStance = cgArray[0].lastStance;
            if (IsCommandBound)
            {
                ++v19;
                *v21 = v27;
                break;
            }
            ++v25;
            v28 += 4;
            if ((cgArray[0].lastStance & 1) != 0)
            {
                v30 = &v74;
            }
            else
            {
                v30 = v65;
                if ((cgArray[0].lastStance & 2) == 0)
                    v30 = v86;
            }
            v27 = *(const char **)((char *)v30 + v28);
            if (v25 >= 6)
                break;
        }
        v22 += 6;
        v20 += 24;
        ++v21;
    } while (v22 < 18);
    v31 = (float)((float)((float)(rect->h * (float)0.5) + rect->y) - (float)1.5);
    if (v19 == 1)
    {
        v31 = (float)((float)((float)v24 * (float)0.5)
            + (float)((float)((float)(rect->h * (float)0.5) + rect->y) - (float)1.5));
    }
    else if (v19 == 3)
    {
        v31 = (float)((float)((float)((float)(rect->h * (float)0.5) + rect->y) - (float)1.5)
            - (float)((float)((float)v24 * (float)0.5) + (float)1.5));
    }
    if (fadeAlpha < v62)
        v62 = fadeAlpha;
    do
    {
        v32 = *(const char **)((char *)&v54 + v10 * 4);
        if (v32)
        {
            UI_GetKeyBindingLocalizedString(localClientNum, v32, v91);
            v33 = UI_SafeTranslateString((const char *)v64[v10]);
            v34 = UI_ReplaceConversionString(v33, v91);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                v34,
                0x7FFFFFFF,
                a9,
                (float)(rect->w + (float)x),
                v31,
                v35,
                (int)v61,
                scale,
                (const float *)rect->horzAlign,
                rect->vertAlign,
                v40,
                v39,
                v38,
                v37,
                v36,
                v41,
                v42,
                v43,
                v44,
                v45,
                v46,
                v47,
                v48,
                v49,
                v50,
                v51,
                v52,
                v53,
                *(float *)&v54);
            v31 = (float)((float)((float)v31 + (float)v24) + (float)1.5);
        }
        ++v10;
    } while (v10 < 3);
}

float __cdecl CG_CalcPlayerHealth(const playerState_s *ps)
{
    _BYTE v1[12]; // r11 OVERLAPPED

    *(unsigned int *)&v1[4] = ps->stats[0];
    if (*(unsigned int *)&v1[4] && (*(unsigned int *)&v1[8] = ps->stats[2]) != 0 && (*(unsigned int *)v1 = ps->pm_type, *(unsigned int *)v1 != 5))
    {
        _FP12 = (float)((float)((float)*(__int64 *)v1 / (float)*(__int64 *)&v1[4]) - (float)1.0);
        _FP11 = -(float)((float)*(__int64 *)v1 / (float)*(__int64 *)&v1[4]);
        __asm { fsel      f13, f12, f13, f0 }
        __asm { fsel      f1, f11, f0, f13 }
    }
    else
    {
        _FP1 = 0.0;
    }
    return *((float *)&_FP1 + 1);
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
    int v20; // r10
    double v21; // fp0
    int v26; // r10
    int v31; // r11
    int v32; // r11
    double v33; // fp0
    const dvar_s *v34; // r11
    int integer; // r11

    p_healthOverlayPulseDuration = &cgameGlob->healthOverlayPulseDuration;
    p_healthOverlayPulseTime = &cgameGlob->healthOverlayPulseTime;
    p_time = &cgameGlob->time;
    time = cgameGlob->time;
    if (cgameGlob->healthOverlayPulseTime + cgameGlob->healthOverlayPulseDuration > time)
        return;
    if (healthRatio < *(float *)(dword_827D8A48 + 12)
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
                v15 = (float)(*(float *)(dword_827D8AA4 + 12) * (float)healthOverlayToAlpha);
                _FP12 = (float)((float)v15 - (float)1.0);
                _FP11 = -v15;
                __asm { fsel      f13, f12, f13, f0 }
                __asm { fsel      f0, f11, f0, f13 }
                *p_healthOverlayToAlpha = _FP0;
                v20 = *(unsigned int *)(dword_827D8AA8 + 12);
                *p_healthOverlayPulsePhase = 0;
                goto LABEL_15;
            }
            v9 = 2;
            v21 = (float)(*(float *)(dword_827D8A40 + 12) * (float)healthOverlayToAlpha);
            _FP12 = (float)((float)v21 - (float)1.0);
            _FP11 = -v21;
            __asm { fsel      f13, f12, f13, f0 }
            __asm { fsel      f0, f11, f0, f13 }
            *p_healthOverlayToAlpha = _FP0;
            v26 = dword_827D8A88;
        }
        else
        {
            _FP12 = (float)((float)((float)((float)1.0 - (float)((float)healthRatio * (float)0.30000001))
                + *(float *)(dword_827D8A4C + 12))
                - (float)1.0);
            _FP11 = -(float)((float)((float)1.0 - (float)((float)healthRatio * (float)0.30000001))
                + *(float *)(dword_827D8A4C + 12));
            __asm { fsel      f13, f12, f0, f13 }
            __asm { fsel      f0, f11, f0, f13 }
            *p_healthOverlayToAlpha = _FP0;
            v26 = dword_827D8A90;
        }
        v20 = *(unsigned int *)(v26 + 12);
        *p_healthOverlayPulsePhase = v9;
    LABEL_15:
        *p_healthOverlayPulseDuration = v20;
    }
LABEL_16:
    if (healthRatio > *(float *)(dword_827D8A80 + 12) && *p_healthOverlayHurt)
    {
        v31 = *p_time;
        *p_healthOverlayHurt = 0;
        *p_healthOverlayPulseTime = v31;
        v32 = dword_827D8A98;
        cgameGlob->healthOverlayFromAlpha = *(float *)(dword_827D8A7C + 12);
        v33 = *(float *)(v32 + 12);
        v34 = hud_healthOverlay_phaseEnd_pulseDuration;
        cgameGlob->healthOverlayToAlpha = v33;
        integer = v34->current.integer;
        cgameGlob->healthOverlayPulsePhase = 0;
        *p_healthOverlayPulseDuration = integer;
    }
}

void __cdecl CG_DrawPlayerLowHealthOverlay(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    double v7; // fp1
    const float *v8; // r6
    int v9; // r5
    int v10; // r4
    double v11; // fp1
    float v12; // [sp+8h] [-A8h]
    float v13; // [sp+10h] [-A0h]
    float v14; // [sp+18h] [-98h]
    float v15; // [sp+20h] [-90h]
    float v16; // [sp+28h] [-88h]
    float v17; // [sp+30h] [-80h]
    float v18; // [sp+38h] [-78h]
    float v19; // [sp+40h] [-70h]
    float v20; // [sp+48h] [-68h]
    float v21; // [sp+50h] [-60h]
    float v22; // [sp+58h] [-58h]
    float v23; // [sp+60h] [-50h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v7 = CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps);
    if (v7 != 0.0)
    {
        CG_PulseLowHealthOverlay(cgArray, v7);
        v11 = CG_FadeLowHealthOverlay(cgArray);
        color[3] = v11;
        if (v11 != 0.0)
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                v10,
                v9,
                0.0,
                0.0,
                1.0,
                1.0,
                v8,
                (Material *)0x82000000,
                rect->horzAlign,
                rect->vertAlign,
                v12,
                v13,
                v14,
                v15,
                v16,
                v17,
                v18,
                v19,
                v20,
                v21,
                v22,
                v23);
    }
}

int __cdecl CG_ServerMaterialName(int localClientNum, int index, char *materialName, unsigned int maxLen)
{
    const char *ConfigString; // r3
    const char *v7; // r11
    const char *v8; // r10
    int v10; // r10

    if ((unsigned int)(index - 1) > 0x7E)
        return 0;
    ConfigString = CL_GetConfigString(localClientNum, index + 2583);
    v7 = ConfigString;
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
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    int textStyle)
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
    char *v18; // r28
    double v19; // fp29
    double v20; // fp23
    double v21; // fp28
    __int64 v22; // r11
    double v23; // fp25
    double v24; // fp31
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
    WeaponDef *WeaponDef; // r3
    WeaponDef *v42; // r30
    weaponIconRatioType_t hudIconRatio; // r4
    const char *v44; // r3
    char *WeaponUseString; // r3
    char *v46; // r3
    __int64 v47; // r11
    double v48; // fp28
    __int64 v49; // r11
    int v50; // r8
    int v51; // r7
    double v52; // fp8
    double v53; // fp7
    double v54; // fp6
    double v55; // fp5
    double v56; // fp4
    double v57; // fp24
    int v58; // r3
    double v59; // fp22
    double v60; // fp30
    int v61; // r8
    int v62; // r7
    double v63; // fp8
    double v64; // fp7
    double v65; // fp6
    double v66; // fp5
    double v67; // fp4
    const char *v68; // r4
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
    Material *v87; // r7
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
            v18 = 0;
            v118 = 0;
            v19 = 1.0;
            LODWORD(v22) = cg_cursorHints->current.integer;
            v20 = 1.0;
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
                v24 = (float)((float)v26 * (float)v27);
                v23 = (float)((float)((float)v26 * (float)v27) * (float)0.5);
            }
            else
            {
                v23 = 0.0;
                v24 = 0.0;
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
                            LODWORD(v32) = UI_TextWidth(UseString, 0, font, fontscale);
                            v33 = (float)v32;
                            LODWORD(v34) = UI_TextHeight(font, fontscale);
                            UI_DrawText(
                                &scrPlaceView[localClientNum],
                                v31,
                                0x7FFFFFFF,
                                font,
                                (float)((float)((float)v33 + (float)v24) * (float)-0.5),
                                (float)((float)((float)v34 * (float)0.5) + rect->y),
                                0x82000000,
                                68 * localClientNum,
                                fontscale,
                                (const float *)rect->horzAlign,
                                rect->vertAlign,
                                v39,
                                v38,
                                v37,
                                v36,
                                v35,
                                v91,
                                v93,
                                v95,
                                v97,
                                v99,
                                v101,
                                v103,
                                v105,
                                v107,
                                v109,
                                v111,
                                v113,
                                v115,
                                *(float *)&v34);
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
                        if (v18 && *v18)
                        {
                            LODWORD(v47) = UI_TextWidth(v18, 0, font, fontscale);
                            v48 = (float)v47;
                            LODWORD(v49) = UI_TextHeight(font, fontscale);
                            v57 = (float)v49;
                            if (translatedDisplayName && cg_weaponHintsCoD1Style->current.enabled)
                            {
                                v58 = UI_TextWidth(translatedDisplayName, 0, font, fontscale);
                                v59 = (float)-(float)((float)((float)(rect->h * (float)v19) * (float)0.5) - rect->y);
                                v60 = (float)((float)((float)(v58 | 0x7FFF000000000000uLL) + (float)v48) * (float)-0.5);
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    v18,
                                    0x7FFFFFFF,
                                    font,
                                    v60,
                                    (float)(rect->y + (float)((float)v57 * (float)0.5)),
                                    v62,
                                    v61,
                                    fontscale,
                                    (const float *)rect->horzAlign,
                                    rect->vertAlign,
                                    v67,
                                    v66,
                                    v65,
                                    v64,
                                    v63,
                                    v91,
                                    v93,
                                    v95,
                                    v97,
                                    v99,
                                    v101,
                                    v103,
                                    v105,
                                    v107,
                                    v109,
                                    v111,
                                    v113,
                                    v115,
                                    NAN);
                                v68 = translatedDisplayName;
                                vertAlign = rect->vertAlign;
                                horzAlign = (const float *)rect->horzAlign;
                                v71 = (float)((float)((float)v57 * (float)0.5) + rect->y);
                                v72 = va(" %s", v68);
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    v72,
                                    0x7FFFFFFF,
                                    font,
                                    (float)((float)v60 + (float)v48),
                                    v71,
                                    v74,
                                    v73,
                                    fontscale,
                                    horzAlign,
                                    vertAlign,
                                    v79,
                                    v78,
                                    v77,
                                    v76,
                                    v75,
                                    v92,
                                    v94,
                                    v96,
                                    v98,
                                    v100,
                                    v102,
                                    v104,
                                    v106,
                                    v108,
                                    v110,
                                    v112,
                                    v114,
                                    v116,
                                    v117);
                                UI_DrawHandlePic(
                                    &scrPlaceView[localClientNum],
                                    (float)((float)((float)(rect->w * (float)v20) + (float)v24) * (float)-0.5),
                                    (float)((float)((float)v57 * (float)1.5) + (float)v59),
                                    (float)((float)(rect->w * (float)v20) + (float)v24),
                                    (float)((float)(rect->h * (float)v19) + (float)v24),
                                    v83,
                                    v82,
                                    v81,
                                    v80);
                            }
                            else
                            {
                                v84 = fontscale;
                                v85 = (float)((float)((float)((float)((float)v20 * rect->w) + (float)v48) + (float)v24) * (float)-0.5);
                                v86 = (float)-(float)((float)((float)(rect->h * (float)v19) * (float)0.5) - rect->y);
                                UI_DrawText(
                                    &scrPlaceView[localClientNum],
                                    v18,
                                    0x7FFFFFFF,
                                    font,
                                    v85,
                                    (float)((float)((float)v49 * (float)0.5) + rect->y),
                                    v51,
                                    v50,
                                    v84,
                                    (const float *)rect->horzAlign,
                                    rect->vertAlign,
                                    v56,
                                    v55,
                                    v54,
                                    v53,
                                    v52,
                                    v91,
                                    v93,
                                    v95,
                                    v97,
                                    v99,
                                    v101,
                                    v103,
                                    v105,
                                    v107,
                                    v109,
                                    v111,
                                    v113,
                                    v115,
                                    *(float *)&v49);
                                UI_DrawHandlePic(
                                    &scrPlaceView[localClientNum],
                                    (float)((float)v85 + (float)v48),
                                    v86,
                                    (float)((float)((float)v20 * rect->w) + (float)v24),
                                    (float)((float)(rect->h * (float)v19) + (float)v24),
                                    v90,
                                    v89,
                                    v88,
                                    v87);
                            }
                        }
                        else
                        {
                            UI_DrawHandlePic(
                                &scrPlaceView[localClientNum],
                                (float)-(float)((float)((float)((float)(rect->w + (float)v23) + (float)v21) * (float)0.5) - rect->x),
                                (float)-(float)((float)((float)v23 * (float)v19) - rect->y),
                                (float)((float)(rect->w * (float)v20) + (float)v24),
                                (float)((float)(rect->h * (float)v19) + (float)v24),
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
                WeaponDef = BG_GetWeaponDef(cursorHintIcon - 4);
                v42 = WeaponDef;
                if (WeaponDef->hudIcon)
                {
                    hudIconRatio = WeaponDef->hudIconRatio;
                    if (hudIconRatio)
                    {
                        if (hudIconRatio != WEAPON_ICON_RATIO_2TO1)
                        {
                            if (hudIconRatio != WEAPON_ICON_RATIO_4TO1)
                            {
                                v44 = va("hudIconRatio %d, weapon %s", hudIconRatio, WeaponDef->szInternalName);
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
                                    1357,
                                    0,
                                    "%s\n\t%s",
                                    "weapDef->hudIconRatio == WEAPON_ICON_RATIO_4TO1",
                                    v44);
                            }
                            v19 = 0.5;
                        }
                        v21 = (float)(rect->w * (float)-0.5);
                        v20 = 2.0;
                    }
                }
                if (v42->weapClass == WEAPCLASS_TURRET)
                {
                    if (LocalClientGlobals->cursorHintString >= 0)
                        v18 = CG_GetUseString(localClientNum);
                    translatedDisplayName = CG_GetLocalClientWeaponInfo(localClientNum, v40)->translatedDisplayName;
                    goto LABEL_39;
                }
                WeaponUseString = CG_GetWeaponUseString(localClientNum, &v118);
                translatedDisplayName = v118;
            }
            v18 = WeaponUseString;
            goto LABEL_39;
        }
    }
}

void __cdecl CG_DrawHoldBreathHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    int textStyle)
{
    int integer; // r10
    char v10; // r11
    unsigned int ViewmodelWeaponIndex; // r3
    WeaponDef *WeaponDef; // r3
    char *v13; // r3
    char *v14; // r31
    __int64 v15; // r11
    long double v16; // fp2
    long double v17; // fp2
    __int64 v18; // r11
    double v19; // fp8
    double v20; // fp7
    double v21; // fp6
    double v22; // fp5
    double v23; // fp4
    float v24; // [sp+8h] [-1B8h]
    float v25; // [sp+10h] [-1B0h]
    float v26; // [sp+18h] [-1A8h]
    float v27; // [sp+20h] [-1A0h]
    float v28; // [sp+28h] [-198h]
    float v29; // [sp+30h] [-190h]
    float v30; // [sp+38h] [-188h]
    float v31; // [sp+40h] [-180h]
    float v32; // [sp+48h] [-178h]
    float v33; // [sp+50h] [-170h]
    float v34; // [sp+58h] [-168h]
    float v35; // [sp+60h] [-160h]
    float v36; // [sp+68h] [-158h]
    char v37[256]; // [sp+80h] [-140h] BYREF

    if (cg_drawBreathHint->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        integer = cg_paused->current.integer;
        if (!integer || (v10 = 1, cg_drawpaused->current.enabled))
            v10 = 0;
        if (!v10 && !cgArray[0].showScores && !integer && (cgArray[0].predictedPlayerState.weapFlags & 4) == 0)
        {
            ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
            WeaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
            if (WeaponDef->overlayReticle)
            {
                if (WeaponDef->weapClass != WEAPCLASS_ITEM && cgArray[0].predictedPlayerState.fWeaponPosFrac == 1.0)
                {
                    if (!UI_GetKeyBindingLocalizedString(localClientNum, "+holdbreath", v37)
                        && !UI_GetKeyBindingLocalizedString(localClientNum, "+melee_breath", v37))
                    {
                        UI_GetKeyBindingLocalizedString(localClientNum, "+breath_sprint", v37);
                    }
                    v13 = UI_SafeTranslateString("PLATFORM_HOLD_BREATH");
                    v14 = UI_ReplaceConversionString(v13, v37);
                    LODWORD(v15) = UI_TextWidth(v14, 0, font, fontscale);
                    *(double *)&v16 = (float)((float)((float)v15 * (float)0.5) + (float)0.5);
                    v17 = floor(v16);
                    LODWORD(v18) = (int)(float)*(double *)&v17;
                    HIDWORD(v18) = rect->vertAlign;
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        v14,
                        0x7FFFFFFF,
                        font,
                        (float)(rect->x - (float)v18),
                        rect->y,
                        68 * localClientNum,
                        (int)scrPlaceView,
                        fontscale,
                        (const float *)rect->horzAlign,
                        SHIDWORD(v18),
                        v23,
                        v22,
                        v21,
                        v20,
                        v19,
                        v24,
                        v25,
                        v26,
                        v27,
                        v28,
                        v29,
                        v30,
                        v31,
                        v32,
                        v33,
                        v34,
                        v35,
                        v36,
                        *(float *)&v18);
                }
            }
        }
    }
}

void __cdecl CG_DrawMantleHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    int textStyle)
{
    int integer; // r9
    char v10; // r11
    char *v11; // r3
    char *v12; // r30
    __int64 v13; // r11
    double v14; // fp29
    __int64 v15; // r7
    double v16; // fp3
    double v17; // fp30
    double v18; // fp8
    double v19; // fp7
    double v20; // fp6
    double v21; // fp5
    double v22; // fp4
    Material *v23; // r7
    const float *v24; // r6
    int v25; // r5
    int v26; // r4
    float v27; // [sp+8h] [-1C8h]
    float v28; // [sp+10h] [-1C0h]
    float v29; // [sp+18h] [-1B8h]
    float v30; // [sp+20h] [-1B0h]
    float v31; // [sp+28h] [-1A8h]
    float v32; // [sp+30h] [-1A0h]
    float v33; // [sp+38h] [-198h]
    float v34; // [sp+40h] [-190h]
    float v35; // [sp+48h] [-188h]
    float v36; // [sp+50h] [-180h]
    float v37; // [sp+58h] [-178h]
    float v38; // [sp+60h] [-170h]
    float v39; // [sp+68h] [-168h]
    char v40[256]; // [sp+80h] [-150h] BYREF

    if (!cgMedia.mantleHint)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 1484, 0, "%s", "cgMedia.mantleHint");
    if (cg_drawMantleHint->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        integer = cg_paused->current.integer;
        if (!integer || (v10 = 1, cg_drawpaused->current.enabled))
            v10 = 0;
        if (!v10 && !cgArray[0].showScores && !integer && (cgArray[0].predictedPlayerState.mantleState.flags & 8) != 0)
        {
            if (!UI_GetKeyBindingLocalizedString(localClientNum, "+gostand", v40))
                UI_GetKeyBindingLocalizedString(localClientNum, "+moveup", v40);
            v11 = UI_SafeTranslateString("PLATFORM_MANTLE");
            v12 = UI_ReplaceConversionString(v11, v40);
            LODWORD(v13) = UI_TextWidth(v12, 0, font, fontscale);
            v14 = (float)v13;
            LODWORD(v15) = UI_TextHeight(font, fontscale);
            v16 = fontscale;
            v17 = (float)-(float)((float)((float)((float)v14 + rect->w) * (float)0.5) - rect->x);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                v12,
                0x7FFFFFFF,
                font,
                v17,
                (float)((float)((float)v15 * (float)0.5) + rect->y),
                (int)virtualKeyConvert[68],
                68 * localClientNum,
                v16,
                (const float *)rect->horzAlign,
                rect->vertAlign,
                v22,
                v21,
                v20,
                v19,
                v18,
                v27,
                v28,
                v29,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35,
                v36,
                v37,
                v38,
                v39,
                *(float *)&v15);
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                (float)((float)v17 + (float)v14),
                (float)-(float)((float)(rect->h * (float)0.5) - rect->y),
                rect->w,
                rect->h,
                v26,
                v25,
                v24,
                v23);
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
    rectDef_s *textStyle,
    double text_x,
    double text_y)
{
    int *p_deadquoteStartTime; // r30
    int deadquoteStartTime; // r11
    int *p_time; // r29
    const dvar_s *Var; // r3
    int v18; // r9
    __int64 v19; // r11 OVERLAPPED
    double v20; // fp0
    const char *string; // r3
    const char *v22; // r3
    const float *v23; // r7
    _BYTE v24[32]; // [sp+80h] [-70h] BYREF

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
                HIDWORD(v19) = *p_time - *p_deadquoteStartTime;
                LODWORD(v19) = hud_deathQuoteFadeTime->current.integer;
                if (SHIDWORD(v19) > (int)v19)
                {
                    v20 = 1.0;
                }
                else
                {
                    v18 = *p_deadquoteStartTime;
                    v20 = (float)((float)*(__int64 *)((char *)&v19 + 4) / (float)v19);
                }
                string = Var->current.string;
                textStyle->h = v20;
                if (*string == 64)
                    ++string;
                v22 = SEH_LocalizeTextMessage(string, "game message", LOCMSG_SAFE);
                UI_DrawWrappedText(
                    &scrPlaceFull,
                    v22,
                    rect,
                    font,
                    (float)(rect->x + (float)text_x),
                    (float)(rect->y + (float)text_y),
                    fontscale,
                    v23,
                    1,
                    (int)v24,
                    textStyle);
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
                    AxisToAngles((const float (*)[3]) & v23.stateBitsEntry[8], v22);
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
    InvalidCmdHintType invalidCmdHintType; // r11
    const char *v12; // r3
    char *v13; // r30
    __int64 v14; // r9 OVERLAPPED
    signed int integer; // r31
    int invalidCmdHintTime; // r11
    int v17; // r10
    int v18; // r10
    int v19; // r11
    int v20; // r10
    __int64 v21; // r11
    long double v22; // fp2
    long double v23; // fp2
    double x; // fp0
    __int64 v25; // r11
    const float *horzAlign; // r9
    double v27; // fp8
    double v28; // fp7
    double v29; // fp6
    double v30; // fp5
    double v31; // fp4
    float v32; // [sp+8h] [-D8h]
    float v33; // [sp+10h] [-D0h]
    float v34; // [sp+18h] [-C8h]
    float v35; // [sp+20h] [-C0h]
    float v36; // [sp+28h] [-B8h]
    float v37; // [sp+30h] [-B0h]
    float v38; // [sp+38h] [-A8h]
    float v39; // [sp+40h] [-A0h]
    float v40; // [sp+48h] [-98h]
    float v41; // [sp+50h] [-90h]
    float v42; // [sp+58h] [-88h]
    float v43; // [sp+60h] [-80h]
    float v44; // [sp+68h] [-78h]
    __int64 v45; // [sp+70h] [-70h] BYREF
    __int64 v46; // [sp+78h] [-68h]

    if (!rect)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 1679, 0, "%s", "rect");
    if (!textStyle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 1680, 0, "%s", "color");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cg_invalidCmdHintDuration->current.integer + cgArray[0].invalidCmdHintTime >= cgArray[0].time)
    {
        invalidCmdHintType = cgArray[0].invalidCmdHintType;
    }
    else
    {
        invalidCmdHintType = INVALID_CMD_NONE;
        cgArray[0].invalidCmdHintType = INVALID_CMD_NONE;
    }
    if (cgArray[0].predictedPlayerState.pm_type < 5)
    {
        switch (invalidCmdHintType)
        {
        case INVALID_CMD_NO_AMMO_BULLETS:
            v12 = "WEAPON_NO_AMMO";
            goto LABEL_20;
        case INVALID_CMD_NO_AMMO_FRAG_GRENADE:
            v12 = "WEAPON_NO_FRAG_GRENADE";
            goto LABEL_20;
        case INVALID_CMD_NO_AMMO_SPECIAL_GRENADE:
            v12 = "WEAPON_NO_SPECIAL_GRENADE";
            goto LABEL_20;
        case INVALID_CMD_STAND_BLOCKED:
            v12 = "GAME_STAND_BLOCKED";
            goto LABEL_20;
        case INVALID_CMD_CROUCH_BLOCKED:
            v12 = "GAME_CROUCH_BLOCKED";
            goto LABEL_20;
        case INVALID_CMD_TARGET_TOO_CLOSE:
            v12 = "WEAPON_TARGET_TOO_CLOSE";
            goto LABEL_20;
        case INVALID_CMD_LOCKON_REQUIRED:
            v12 = "WEAPON_LOCKON_REQUIRED";
            goto LABEL_20;
        case INVALID_CMD_NOT_ENOUGH_CLEARANCE:
            v12 = "WEAPON_TARGET_NOT_ENOUGH_CLEARANCE";
        LABEL_20:
            v13 = UI_SafeTranslateString(v12);
            integer = cg_invalidCmdHintBlinkInterval->current.integer;
            if (integer <= 0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 1731, 0, "%s", "blinkInterval > 0");
            invalidCmdHintTime = cgArray[0].invalidCmdHintTime;
            *(__int64 *)((char *)&v14 - 4) = __PAIR64__(integer, cgArray[0].time);
            __twllei(integer, 0);
            v45 = v14;
            v18 = v17 - invalidCmdHintTime;
            v19 = __ROL4__(v18, 1);
            v20 = v18 % integer;
            __twlgei(integer & ~(v19 - 1), 0xFFFFFFFF);
            *(float *)(textStyle + 12) = (float)*(__int64 *)((char *)&v14 - 4) / (float)v14;
            LODWORD(v21) = UI_TextWidth(v13, 0, font, fontscale);
            v46 = v21;
            *(double *)&v22 = (float)((float)((float)v21 * (float)0.5) + (float)0.5);
            v23 = floor(v22);
            x = rect->x;
            HIDWORD(v25) = rect->vertAlign;
            horzAlign = (const float *)rect->horzAlign;
            *((double *)&v23 + 1) = rect->y;
            LODWORD(v25) = (int)(float)*(double *)&v23;
            v46 = v25;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                v13,
                0x7FFFFFFF,
                font,
                (float)((float)x - (float)v25),
                *((double *)&v23 + 1),
                (int)&v45,
                68 * localClientNum,
                fontscale,
                horzAlign,
                SHIDWORD(v25),
                v31,
                v30,
                v29,
                v28,
                v27,
                v32,
                v33,
                v34,
                v35,
                v36,
                v37,
                v38,
                v39,
                v40,
                v41,
                v42,
                v43,
                v44,
                *((float *)&v25 + 1));
            break;
        default:
            if (invalidCmdHintType)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
                    1693,
                    0,
                    "%s",
                    "cgameGlob->invalidCmdHintType == INVALID_CMD_NONE");
            break;
        }
    }
}

void __cdecl CG_ArchiveState(int localClientNum, MemoryFile *memFile)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_ArchiveWeaponInfo(memFile);
    CG_ArchiveViewInfo(cgArray, memFile);
    CG_ArchiveCameraShake(localClientNum, memFile);
    CG_ArchiveActiveRumbles(memFile);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].healthFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].ammoFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].stanceFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].compassFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].offhandFadeTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].sprintFadeTime);
    MemFile_ArchiveData(memFile, 8, cgArray[0].vehReticleOffset);
    MemFile_ArchiveData(memFile, 8, cgArray[0].vehReticleVel);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].vehReticleLockOnStartTime);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].vehReticleLockOnDuration);
    MemFile_ArchiveData(memFile, 4, &cgArray[0].vehReticleLockOnEntNum);
    MemFile_ArchiveData(memFile, 160, cgArray[0].visionSetFrom);
    MemFile_ArchiveData(memFile, 160, cgArray[0].visionSetTo);
    MemFile_ArchiveData(memFile, 160, cgArray[0].visionSetCurrent);
    MemFile_ArchiveData(memFile, 24, cgArray[0].visionSetLerpData);
    MemFile_ArchiveData(memFile, 64, cgArray[0].visionNameNaked);
    MemFile_ArchiveData(memFile, 64, cgArray[0].visionNameNight);
    MemFile_ArchiveData(memFile, 128, cgArray[0].hudElemSound);
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
    const char *v16; // r31
    double v17; // fp31
    __int64 v18; // r7
    double v19; // fp8
    double v20; // fp7
    double v21; // fp6
    double v22; // fp5
    double v23; // fp4
    float v24; // [sp+8h] [-D8h]
    float v25; // [sp+10h] [-D0h]
    float v26; // [sp+18h] [-C8h]
    float v27; // [sp+20h] [-C0h]
    float v28; // [sp+28h] [-B8h]
    float v29; // [sp+30h] [-B0h]
    float v30; // [sp+38h] [-A8h]
    float v31; // [sp+40h] [-A0h]
    float v32; // [sp+48h] [-98h]
    float v33; // [sp+50h] [-90h]
    float v34; // [sp+58h] [-88h]
    float v35; // [sp+60h] [-80h]
    float v36; // [sp+68h] [-78h]

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
            v16 = v15;
            v17 = (float)(rect->w + rect->x);
            LODWORD(v18) = UI_TextWidth(v15, 0, font, scale);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                v16,
                0x7FFFFFFF,
                font,
                (float)((float)((float)v17 - (float)v18) - (float)28.0),
                rect->y,
                v18,
                68 * localClientNum,
                scale,
                (const float *)rect->horzAlign,
                rect->vertAlign,
                v23,
                v22,
                v21,
                v20,
                v19,
                v24,
                v25,
                v26,
                v27,
                v28,
                v29,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35,
                v36,
                *(float *)&v18);
        }
    }
}

void __cdecl CG_DrawPlayerWeaponNameBack(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    Material *material,
    int a7)
{
    char v11; // r11
    unsigned int SelectedWeaponIndex; // r3
    unsigned int v13; // r29
    weaponInfo_s *LocalClientWeaponInfo; // r30
    WeaponDef *WeaponDef; // r3
    const char *translatedDisplayName; // r4
    const char *v17; // r3
    __int64 v18; // r10
    const float *v19; // r6
    int v20; // r5
    int v21; // r4

    if (!cg_paused->current.integer || (v11 = 1, cg_drawpaused->current.enabled))
        v11 = 0;
    if (!v11)
    {
        if (!a7)
            Material_RegisterHandle("$default", 3);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0
            && CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgArray[0].weaponSelectTime, 1800) != 0.0)
        {
            SelectedWeaponIndex = CG_GetSelectedWeaponIndex(cgArray);
            v13 = SelectedWeaponIndex;
            if (SelectedWeaponIndex)
            {
                LocalClientWeaponInfo = CG_GetLocalClientWeaponInfo(localClientNum, SelectedWeaponIndex);
                WeaponDef = BG_GetWeaponDef(v13);
                translatedDisplayName = LocalClientWeaponInfo->translatedDisplayName;
                if (*WeaponDef->szModeName)
                    v17 = va("%s / %s", translatedDisplayName, LocalClientWeaponInfo->translatedModename);
                else
                    v17 = va("%s", translatedDisplayName);
                LODWORD(v18) = UI_TextWidth(v17, 0, font, scale);
                HIDWORD(v18) = rect->vertAlign;
                UI_DrawHandlePic(
                    &scrPlaceView[localClientNum],
                    (float)((float)(rect->w + rect->x) - (float)((float)v18 + (float)36.0)),
                    rect->y,
                    (float)((float)v18 + (float)36.0),
                    rect->h,
                    v21,
                    v20,
                    v19,
                    (Material *)(68 * localClientNum));
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerStance(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *color,
    Font_s *font,
    double scale,
    int textStyle)
{
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
}

void __cdecl CG_DrawPlayerSprintBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color,
    long double a5)
{
    long double v7; // fp2
    const float *v8; // r6
    int v9; // r5
    int v10; // r4
    float v11; // [sp+8h] [-D8h]
    float v12; // [sp+10h] [-D0h]
    float v13; // [sp+18h] [-C8h]
    float v14; // [sp+20h] [-C0h]
    float v15; // [sp+28h] [-B8h]
    float v16; // [sp+30h] [-B0h]
    float v17; // [sp+38h] [-A8h]
    float v18; // [sp+40h] [-A0h]
    float v19; // [sp+48h] [-98h]
    float v20; // [sp+50h] [-90h]
    float v21; // [sp+58h] [-88h]
    float v22; // [sp+60h] [-80h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0
        || (cgArray[0].predictedPlayerState.eFlags & 0x80000) != 0)
    {
        *(double *)&a5 = (float)((float)(hud_fade_sprint->current.value * (float)1000.0) + (float)0.5);
        v7 = floor(a5);
        if (CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgArray[0].sprintFadeTime, (int)(float)*(double *)&v7) != 0.0)
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                v10,
                v9,
                0.0,
                0.0,
                1.0,
                1.0,
                v8,
                (Material *)0x82000000,
                rect->horzAlign,
                rect->vertAlign,
                v11,
                v12,
                v13,
                v14,
                v15,
                v16,
                v17,
                v18,
                v19,
                v20,
                v21,
                v22);
    }
}

void __cdecl CG_DrawPlayerSprintMeter(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    DvarValue *color,
    long double a5)
{
    long double v8; // fp2
    double v9; // fp29
    __int64 v10; // r11
    double v11; // fp30
    double x; // fp29
    double v13; // fp28
    double y; // fp27
    double h; // fp26
    const float *v16; // r6
    int v17; // r5
    int v18; // r4
    float v19; // [sp+8h] [-108h]
    float v20; // [sp+10h] [-100h]
    float v21; // [sp+18h] [-F8h]
    float v22; // [sp+20h] [-F0h]
    float v23; // [sp+28h] [-E8h]
    float v24; // [sp+30h] [-E0h]
    float v25; // [sp+38h] [-D8h]
    float v26; // [sp+40h] [-D0h]
    float v27; // [sp+48h] [-C8h]
    float v28; // [sp+50h] [-C0h]
    float v29; // [sp+58h] [-B8h]
    float v30; // [sp+60h] [-B0h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) == 0
        || (cgArray[0].predictedPlayerState.eFlags & 0x80000) != 0)
    {
        *(double *)&a5 = (float)((float)(hud_fade_sprint->current.value * (float)1000.0) + (float)0.5);
        v8 = floor(a5);
        if (CG_FadeHudMenu(localClientNum, hud_fade_sprint, cgArray[0].sprintFadeTime, (int)(float)*(double *)&v8) != 0.0)
        {
            v9 = (float)(PM_GetSprintLeft(&cgArray[0].predictedPlayerState, cgArray[0].time) | 0x1A6DC00000000uLL);
            LODWORD(v10) = (int)(float)((float)(BG_GetWeaponDef(cgArray[0].predictedPlayerState.weapon)->sprintDurationScale
                * player_sprintTime->current.value)
                * (float)1000.0);
            v11 = (float)((float)v9 / (float)v10);
            if (v11 > 0.0)
            {
                x = rect->x;
                v13 = (float)(rect->w * (float)v11);
                y = rect->y;
                h = rect->h;
                CG_CalcPlayerSprintColor(cgArray, &cgArray[0].predictedPlayerState, color);
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    v13,
                    h,
                    v18,
                    v17,
                    0.0,
                    0.0,
                    v11,
                    1.0,
                    v16,
                    (Material *)0x82000000,
                    rect->horzAlign,
                    rect->vertAlign,
                    v19,
                    v20,
                    v21,
                    v22,
                    v23,
                    v24,
                    v25,
                    v26,
                    v27,
                    v28,
                    v29,
                    v30);
            }
        }
    }
}

void __cdecl CG_DrawPlayerBarHealth(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    double v7; // fp31
    long double v8; // fp2
    long double v9; // fp2
    __int64 v10; // r10
    Material *v11; // r7
    const float *v12; // r6
    int v13; // r5
    int v14; // r4
    double v15; // fp1
    double x; // fp1
    double v21; // fp3
    double y; // fp2
    double h; // fp4
    double v24; // fp0
    double v25; // fp12
    double lastHealth; // fp0
    int lastHealthLerpDelay; // r10
    int v28; // r11
    double w; // fp13
    double v30; // fp12
    double v31; // fp2
    double v32; // fp4
    float v33; // [sp+8h] [-D8h]
    float v34; // [sp+10h] [-D0h]
    float v35; // [sp+18h] [-C8h]
    float v36; // [sp+20h] [-C0h]
    float v37; // [sp+28h] [-B8h]
    float v38; // [sp+30h] [-B0h]
    float v39; // [sp+38h] [-A8h]
    float v40; // [sp+40h] [-A0h]
    float v41; // [sp+48h] [-98h]
    float v42; // [sp+50h] [-90h]
    float v43; // [sp+58h] [-88h]
    float v44; // [sp+60h] [-80h]

    if (cg_drawHealth->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        v7 = CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps);
        *(double *)&v8 = (float)((float)(hud_fade_healthbar->current.value * (float)1000.0) + (float)0.5);
        v9 = floor(v8);
        v15 = CG_FadeHudMenu(localClientNum, hud_fade_healthbar, cgArray[0].healthFadeTime, (int)(float)*(double *)&v9);
        if (v15 != 0.0)
        {
            _FP13 = -v7;
            color[3] = v15;
            _FP0 = (float)((float)v7 - (float)1.0);
            __asm
            {
                fsel      f0, f0, f29, f31
                fsel      f0, f13, f30, f0
            }
            if (v7 > 0.0)
            {
                x = rect->x;
                v21 = (float)(rect->w * (float)v7);
                y = rect->y;
                h = rect->h;
                if (_FP0 <= 0.5)
                {
                    color[1] = (float)((float)((float)_FP0 + (float)0.2) * color[1]) + (float)0.30000001;
                }
                else
                {
                    v24 = (float)((float)1.0 - (float)_FP0);
                    v25 = (float)((float)v24 * color[2]);
                    *color = (float)((float)v24 * *color) * (float)2.0;
                    color[2] = (float)v25 * (float)2.0;
                }
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    v21,
                    h,
                    v14,
                    v13,
                    0.0,
                    0.0,
                    v7,
                    1.0,
                    v12,
                    v11,
                    rect->horzAlign,
                    rect->vertAlign,
                    v33,
                    v34,
                    v35,
                    v36,
                    v37,
                    v38,
                    v39,
                    v40,
                    v41,
                    v42,
                    v43,
                    v44);
            }
            lastHealth = cgArray[0].lastHealth;
            if (v7 < cgArray[0].lastHealth)
            {
                lastHealthLerpDelay = cgArray[0].lastHealthLerpDelay;
                if (cgArray[0].lastHealthLerpDelay)
                {
                    cgArray[0].lastHealthLerpDelay -= cgArray[0].frametime;
                    if (lastHealthLerpDelay - cgArray[0].frametime >= 0)
                        goto LABEL_17;
                    v28 = 0;
                    goto LABEL_16;
                }
                LODWORD(v10) = cgArray[0].frametime;
                lastHealth = (float)-(float)((float)((float)v10 * (float)0.0012000001) - cgArray[0].lastHealth);
                cgArray[0].lastHealth = -(float)((float)((float)v10 * (float)0.0012000001) - cgArray[0].lastHealth);
                if (lastHealth > v7)
                    goto LABEL_17;
            }
            lastHealth = v7;
            cgArray[0].lastHealth = v7;
            v28 = 1;
        LABEL_16:
            cgArray[0].lastHealthLerpDelay = v28;
        LABEL_17:
            if (lastHealth > v7)
            {
                w = rect->w;
                v30 = rect->x;
                v31 = rect->y;
                v32 = rect->h;
                *color = 1.0;
                color[1] = 0.0;
                color[2] = 0.0;
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)((float)w * (float)v7) + (float)v30),
                    v31,
                    (float)((float)((float)lastHealth - (float)v7) * (float)w),
                    v32,
                    v14,
                    v13,
                    v7,
                    0.0,
                    cgArray[0].lastHealth,
                    1.0,
                    v12,
                    v11,
                    rect->horzAlign,
                    rect->vertAlign,
                    v33,
                    v34,
                    v35,
                    v36,
                    v37,
                    v38,
                    v39,
                    v40,
                    v41,
                    v42,
                    v43,
                    v44);
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerBarHealthBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color,
    long double a5)
{
    long double v8; // fp2
    const float *v9; // r6
    int v10; // r5
    int v11; // r4
    double v12; // fp1
    double v13; // fp29
    double x; // fp28
    double y; // fp27
    double w; // fp26
    double h; // fp25
    const ScreenPlacement *v18; // r30
    long double v19; // fp2
    const dvar_s *v20; // r11
    Material *v21; // r7
    const float *v22; // r6
    int v23; // r5
    int v24; // r4
    long double v25; // fp2
    __int64 v26; // r10 OVERLAPPED
    int v27; // r11
    float v28; // [sp+8h] [-108h]
    float v29; // [sp+8h] [-108h]
    float v30; // [sp+10h] [-100h]
    float v31; // [sp+10h] [-100h]
    float v32; // [sp+18h] [-F8h]
    float v33; // [sp+18h] [-F8h]
    float v34; // [sp+20h] [-F0h]
    float v35; // [sp+20h] [-F0h]
    float v36; // [sp+28h] [-E8h]
    float v37; // [sp+28h] [-E8h]
    float v38; // [sp+30h] [-E0h]
    float v39; // [sp+30h] [-E0h]
    float v40; // [sp+38h] [-D8h]
    float v41; // [sp+38h] [-D8h]
    float v42; // [sp+40h] [-D0h]
    float v43; // [sp+40h] [-D0h]
    float v44; // [sp+48h] [-C8h]
    float v45; // [sp+48h] [-C8h]
    float v46; // [sp+50h] [-C0h]
    float v47; // [sp+50h] [-C0h]
    float v48; // [sp+58h] [-B8h]
    float v49; // [sp+58h] [-B8h]
    float v50; // [sp+60h] [-B0h]
    float v51; // [sp+60h] [-B0h]

    if (cg_drawHealth->current.enabled)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        *(double *)&a5 = (float)((float)(hud_fade_healthbar->current.value * (float)1000.0) + (float)0.5);
        v8 = floor(a5);
        v12 = CG_FadeHudMenu(localClientNum, hud_fade_healthbar, cgArray[0].healthFadeTime, (int)(float)*(double *)&v8);
        v13 = v12;
        if (v12 != 0.0)
        {
            color[3] = v12;
            x = rect->x;
            y = rect->y;
            w = rect->w;
            h = rect->h;
            v18 = &scrPlaceView[localClientNum];
            CL_DrawStretchPic(
                v18,
                x,
                y,
                w,
                h,
                v11,
                v10,
                0.0,
                0.0,
                1.0,
                1.0,
                v9,
                (Material *)0x82000000,
                rect->horzAlign,
                rect->vertAlign,
                v28,
                v30,
                v32,
                v34,
                v36,
                v38,
                v40,
                v42,
                v44,
                v46,
                v48,
                v50);
            *(double *)&v19 = CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps);
            if (*(double *)&v19 != 0.0)
            {
                if (*(double *)&v19 >= hud_health_startpulse_critical->current.value)
                {
                    if (*(double *)&v19 >= hud_health_startpulse_injured->current.value)
                        return;
                    v20 = hud_health_pulserate_injured;
                }
                else
                {
                    v20 = hud_health_pulserate_critical;
                }
                *(double *)&v19 = (float)((float)(v20->current.value * (float)1000.0) + (float)0.5);
                v25 = floor(v19);
                HIDWORD(v26) = (int)(float)*(double *)&v25;
                if (HIDWORD(v26))
                {
                    if (cgArray[0].lastHealthPulseTime > cgArray[0].time
                        || cgArray[0].lastHealthPulseTime + HIDWORD(v26) < cgArray[0].time)
                    {
                        cgArray[0].lastHealthPulseTime = cgArray[0].time;
                    }
                    *color = 0.88999999;
                    color[1] = 0.18000001;
                    color[2] = 0.0099999998;
                    LODWORD(v26) = (int)(float)*(double *)&v25;
                    v27 = cgArray[0].lastHealthPulseTime - cgArray[0].time + HIDWORD(v26);
                    color[3] = (float)*(__int64 *)((char *)&v26 - 4) / (float)v26;
                    if (v13 < (float)((float)*(__int64 *)((char *)&v26 - 4) / (float)v26))
                        color[3] = v13;
                    CL_DrawStretchPic(
                        v18,
                        x,
                        y,
                        w,
                        h,
                        v24,
                        v23,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        v22,
                        v21,
                        rect->horzAlign,
                        rect->vertAlign,
                        v29,
                        v31,
                        v33,
                        v35,
                        v37,
                        v39,
                        v41,
                        v43,
                        v45,
                        v47,
                        v49,
                        v51);
                }
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_OwnerDraw(
    rectDef_s *parentRect,
    double x,
    double y,
    double w,
    double h,
    __int64 vertAlign,
    double text_x,
    double text_y,
    int ownerDraw,
    int ownerDrawFlags,
    int align,
    double special,
    Font_s *font,
    double scale,
    float *color,
    Material *material,
    __int64 textAlignMode,
    __int64 a18,
    __int64 a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    Material *a31,
    int a32,
    int a33,
    int a34,
    int a35,
    int a36,
    int a37,
    int a38,
    int vertAlign_0,
    int a40,
    int a41,
    int a42,
    int a43,
    int a44,
    int ownerDraw_0,
    int a46,
    int ownerDrawFlags_0,
    int a48,
    int align_0,
    int a50,
    Material *a51,
    int a52,
    Font_s *font_0)
{
    const cg_s *LocalClientGlobals; // r28
    const playerState_s *PredictedPlayerState; // r29
    int v65; // r7
    float *v66; // r6
    const float *v67; // r5
    long double v68; // fp2
    bool v69; // [sp+Bh] [-D5h]
    rectDef_s v70[3]; // [sp+60h] [-80h] BYREF

    textAlignMode = *(_QWORD *)&parentRect;
    a18 = *(__int64 *)((char *)&vertAlign + 4);
    a19 = vertAlign;
    if (cg_drawHUD->current.enabled && hud_drawHUD->current.enabled)
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(0);
        PredictedPlayerState = CG_GetPredictedPlayerState(0);
        if (!PredictedPlayerState)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp", 1762, 0, "%s", "ps");
        if (PredictedPlayerState->offhandSecondary >= (unsigned int)PLAYER_OFFHAND_SECONDARIES_TOTAL)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_newDraw.cpp",
                1770,
                0,
                "%s",
                "ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH");
        v70[0].x = x;
        v70[0].y = y;
        v70[0].w = w;
        v70[0].h = h;
        v70[0].horzAlign = (int)color;
        v70[0].vertAlign = (int)a31;
        switch (a37)
        {
        case 5:
            CG_DrawPlayerAmmoValue(0, v70, (Font_s *)ownerDraw_0, scale, v66, (Material *)align_0, (int)a51);
            break;
        case 6:
            CG_DrawPlayerAmmoBackdrop(0, v70, (float *)align_0, a51);
            break;
        case 20:
            CG_DrawPlayerStance(0, v70, (Font_s *)align_0, (Font_s *)ownerDraw_0, scale, v65);
            break;
        case 71:
            CG_DrawHoldBreathHint(0, v70, (Font_s *)ownerDraw_0, scale, (int)v66);
            break;
        case 72:
            CG_DrawCursorhint(0, v70, (Font_s *)ownerDraw_0, scale, (int)v66);
            break;
        case 79:
            CG_DrawPlayerBarHealth(0, v70, a51, (float *)align_0);
            break;
        case 80:
            CG_DrawMantleHint(0, v70, (Font_s *)ownerDraw_0, scale, (int)v66);
            break;
        case 81:
            CG_DrawPlayerWeaponName(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0);
            break;
        case 82:
            CG_DrawPlayerWeaponNameBack(0, v70, (Font_s *)ownerDraw_0, scale, v66, (Material *)align_0, (int)a51);
            break;
        case 90:
            CG_DrawCenterString(0, v70, (Font_s *)ownerDraw_0, scale, v66, (float *)align_0);
            break;
        case 95:
            CG_DrawTankBody(0, v70, a51, (float *)align_0);
            break;
        case 96:
            CG_DrawTankBarrel(0, v70, a51, (const float *)align_0);
            break;
        case 97:
            CG_DrawDeadQuote(
                LocalClientGlobals,
                v70,
                (Font_s *)ownerDraw_0,
                scale,
                v66,
                (rectDef_s *)align_0,
                text_x,
                text_y);
            break;
        case 98:
            CG_DrawPlayerBarHealthBack(0, v70, a51, (float *)align_0, v68);
            break;
        case 99:
            CG_DrawObjectiveHeader(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0);
            break;
        case 100:
            CG_DrawObjectiveList(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0);
            break;
        case 101:
            CG_DrawObjectiveBackdrop(LocalClientGlobals, (const float *)align_0);
            break;
        case 102:
            CG_DrawPausedMenuLine(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0);
            break;
        case 103:
        case 104:
            CG_DrawOffHandIcon(0, v70, scale, v67, (Material *)align_0, (OffhandClass)a51);
            break;
        case 105:
        case 106:
            CG_DrawOffHandAmmo(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0, (OffhandClass)font_0);
            break;
        case 107:
        case 108:
            CG_DrawOffHandName(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0, (OffhandClass)font_0);
            break;
        case 109:
        case 110:
            CG_DrawOffHandHighlight(0, v70, scale, v67, (Material *)align_0, (OffhandClass)a51);
            break;
        case 112:
            CG_DrawPlayerLowHealthOverlay(0, v70, a51, (float *)align_0);
            break;
        case 113:
            CG_DrawInvalidCmdHint(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0);
            break;
        case 114:
            CG_DrawPlayerSprintMeter(0, v70, a51, (DvarValue *)align_0, v68);
            break;
        case 115:
            CG_DrawPlayerSprintBack(0, v70, a51, (float *)align_0, v68);
            break;
        case 116:
            CG_DrawPlayerWeaponBackground(0, v70, (const float *)align_0, a51);
            break;
        case 117:
            CG_DrawPlayerWeaponAmmoClipGraphic(0, v70, (const float *)align_0);
            break;
        case 118:
            CG_DrawPlayerWeaponIcon(0, v70, (const float *)align_0);
            break;
        case 119:
            CG_DrawPlayerWeaponAmmoStock(0, v70, (Font_s *)ownerDraw_0, scale, v66, (Material *)align_0, (int)a51);
            break;
        case 120:
            CG_DrawPlayerWeaponLowAmmoWarning(
                0,
                v70,
                (Font_s *)ownerDraw_0,
                scale,
                (int)v66,
                text_x,
                text_y,
                (int)font_0,
                a31);
            break;
        case 145:
        case 146:
            CG_CompassDrawTickertape(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (const float *)align_0,
                (Font_s *)ownerDraw_0,
                scale,
                (int)font_0,
                v69);
            break;
        case 150:
            CG_CompassDrawPlayer(0, COMPASS_TYPE_PARTIAL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 151:
            CG_CompassDrawPlayerBack(0, COMPASS_TYPE_PARTIAL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 152:
            CG_CompassDrawPlayerPointers_SP(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (const float *)align_0);
            break;
        case 153:
            CG_CompassDrawActors(0, COMPASS_TYPE_PARTIAL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 154:
            CG_CompassDrawVehicles(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (float *)align_0,
                1u);
            break;
        case 155:
            CG_CompassDrawVehicles(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (float *)align_0,
                2u);
            break;
        case 156:
            CG_CompassDrawVehicles(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (float *)align_0,
                3u);
            break;
        case 157:
            CG_CompassDrawVehicles(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (float *)align_0,
                4u);
            break;
        case 159:
            CG_CompassDrawPlayerMap(0, COMPASS_TYPE_PARTIAL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 160:
            CG_CompassDrawPlayerNorthCoord(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                (Font_s *)ownerDraw_0,
                a51,
                (float *)align_0,
                (int)font_0);
            break;
        case 161:
            CG_CompassDrawPlayerEastCoord(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                (Font_s *)ownerDraw_0,
                a51,
                (float *)align_0,
                (int)font_0);
            break;
        case 162:
            CG_CompassDrawPlayerNCoordScroll(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                (Font_s *)ownerDraw_0,
                a51,
                (float *)align_0,
                (int)font_0);
            break;
        case 163:
            CG_CompassDrawPlayerECoordScroll(
                0,
                COMPASS_TYPE_PARTIAL,
                (const rectDef_s *)&textAlignMode,
                v70,
                (Font_s *)ownerDraw_0,
                a51,
                (float *)align_0,
                (int)font_0);
            break;
        case 164:
            CG_CompassDrawGoalDistance(0, v70, (Font_s *)ownerDraw_0, scale, v66, align_0);
            break;
        case 165:
            CG_DrawPlayerActionSlotDpad(0, v70, (const float *)align_0, a51);
            break;
        case 166:
        case 167:
        case 168:
        case 169:
            CG_DrawPlayerActionSlot(0, v70, a37 - 166, (float *)align_0, (Font_s *)ownerDraw_0, scale, (int)a31);
            break;
        case 180:
            CG_CompassDrawPlayerBack(0, COMPASS_TYPE_FULL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 181:
            CG_CompassDrawPlayerMap(0, COMPASS_TYPE_FULL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 182:
            CG_CompassDrawPlayerPointers_SP(
                0,
                COMPASS_TYPE_FULL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (const float *)align_0);
            break;
        case 183:
            CG_CompassDrawPlayer(0, COMPASS_TYPE_FULL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 184:
            CG_CompassDrawActors(0, COMPASS_TYPE_FULL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 186:
            CG_CompassDrawPlayerMapLocationSelector(
                0,
                COMPASS_TYPE_FULL,
                (const rectDef_s *)&textAlignMode,
                v70,
                a51,
                (float *)align_0);
            break;
        case 187:
            CG_CompassDrawBorder(0, COMPASS_TYPE_FULL, (const rectDef_s *)&textAlignMode, v70, a51, (float *)align_0);
            break;
        case 190:
            CG_DrawVehicleReticle(0, v70, (float *)align_0);
            break;
        case 191:
            CG_DrawVehicleTargets(0, v70, (float *)align_0, a51);
            break;
        case 192:
            CG_DrawJavelinTargets(0, v70, (float *)align_0, a51);
            break;
        default:
            return;
        }
    }
}

