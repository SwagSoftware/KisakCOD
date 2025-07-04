#include "cg_local.h"
#include "cg_public.h"

#include <cgame_mp/cg_local_mp.h>

#include <client/client.h>
#include <client_mp/client_mp.h>

#include <stringed/stringed_hooks.h>

const dvar_t *lowAmmoWarningPulseMin;
const dvar_t *actionSlotsHide;
const dvar_t *lowAmmoWarningNoAmmoColor1;
const dvar_t *lowAmmoWarningPulseFreq;
const dvar_t *lowAmmoWarningNoReloadColor2;
const dvar_t *lowAmmoWarningNoAmmoColor2;
const dvar_t *lowAmmoWarningNoReloadColor1;
const dvar_t *lowAmmoWarningPulseMax;
const dvar_t *ammoCounterHide;
const dvar_t *lowAmmoWarningColor2;
const dvar_t *lowAmmoWarningColor1;

const float MYFLASHTERM = 40.0f;
const float TEST_bullet_step_1[2] = { 20.0f, 12.0f };
const float TEST_bullet_wh_1[2] = { 16.0f, 8.0f };
const float TEST_bullet_step_2[2] = { 72.0f, 12.0f };
const float TEST_bullet_wh_2[2] = {64.0f, 16.0f};
const int32_t TEST_bullet_rowCnt = 1;
const float TEST_bullet_wh_3[2] = { 8.0f, 4.0f };
const float TEST_bullet_step_3[2] = { 8.0f, -2.0f };

const float colorLowAmmo[4] = { 1.0f, 0.3f, 0.3f, 1.0f };
const float colorDpadArrow[4] = { 1.0f, 0.97f, 0.55f, 1.0f };
const float MY_ACTIVECOLOR[3] = { 1.2f, 1.2f, 1.2f };
const float MY_OFFSETS[4][2] =
{
    { 0.25f, 0.12f },
    { 0.25, 0.63f },
    { 0.12f, 0.25f },
    { 0.63f, 0.25f }
};
const float MY_ST[4][4] =
{
  { 0.0, 0.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 0.0 },
  { 0.0, 0.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 0.0 }
}; // idb
const float MY_DIMS[4][2] = { { 0.5, 0.25 }, { 0.5, 0.25 }, { 0.25, 0.5 }, { 0.25, 0.5 } }; // idb

void __cdecl CG_AmmoCounterRegisterDvars()
{
    DvarLimits b; // [esp+8h] [ebp-10h]
    DvarLimits ba; // [esp+8h] [ebp-10h]
    DvarLimits bb; // [esp+8h] [ebp-10h]

    ammoCounterHide = Dvar_RegisterBool("ammoCounterHide", 0, DVAR_SAVED, "Hide the Ammo Counter");
    actionSlotsHide = Dvar_RegisterBool("actionSlotsHide", 0, DVAR_SAVED, "Hide the actionslots.");
    lowAmmoWarningColor1 = Dvar_RegisterColor(
        "lowAmmoWarningColor1",
        0.89999998f,
        0.89999998f,
        0.89999998f,
        0.80000001f,
        DVAR_NOFLAG,
        "Color 1 of 2 to oscilate between");
    lowAmmoWarningColor2 = Dvar_RegisterColor(
        "lowAmmoWarningColor2",
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        DVAR_NOFLAG,
        "Color 2 of 2 to oscilate between");
    b.value.max = FLT_MAX;
    b.value.min = 0.0f;
    lowAmmoWarningPulseFreq = Dvar_RegisterFloat(
        "lowAmmoWarningPulseFreq",
        1.7f,
        b,
        DVAR_NOFLAG,
        "Frequency of the pulse (oscilation between the 2 colors)");
    ba.value.max = FLT_MAX;
    ba.value.min = 0.0f;
    lowAmmoWarningPulseMax = Dvar_RegisterFloat(
        "lowAmmoWarningPulseMax",
        1.5f,
        ba,
        DVAR_NOFLAG,
        "Min of oscilation range: 0 is color1 and 1.0 is color2.  Can be < 0, and the wave will clip at 0.");
    bb.value.max = 1.0f;
    bb.value.min = -FLT_MAX;
    lowAmmoWarningPulseMin = Dvar_RegisterFloat(
        "lowAmmoWarningPulseMin",
        0.0f,
        bb,
        DVAR_NOFLAG,
        "Max of oscilation range: 0 is color1 and 1.0 is color2.  Can be > 1.0, and the wave will clip at 1.0.");
    lowAmmoWarningNoReloadColor1 = Dvar_RegisterColor(
        "lowAmmoWarningNoReloadColor1",
        0.69999999f,
        0.69999999f,
        0.0f,
        0.80000001f,
        DVAR_NOFLAG,
        "Like lowAmmoWarningColor1, but when no ammo to reload with.");
    lowAmmoWarningNoReloadColor2 = Dvar_RegisterColor(
        "lowAmmoWarningNoReloadColor2",
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        DVAR_NOFLAG,
        "lowAmmoWarningColor2, but when no ammo to reload with.");
    lowAmmoWarningNoAmmoColor1 = Dvar_RegisterColor(
        "lowAmmoWarningNoAmmoColor1",
        0.80000001f,
        0.0f,
        0.0f,
        0.80000001f,
        DVAR_NOFLAG,
        "Like lowAmmoWarningColor1, but when no ammo.");
    lowAmmoWarningNoAmmoColor2 = Dvar_RegisterColor(
        "lowAmmoWarningNoAmmoColor2",
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        DVAR_NOFLAG,
        "lowAmmoWarningColor2, but when no ammo.");
}

void __cdecl CG_DrawPlayerWeaponAmmoStock(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int32_t textStyle)
{
    uint32_t WeaponIndex; // eax
    char str[64]; // [esp+30h] [ebp-68h] BYREF
    cg_s *cgameGlob; // [esp+74h] [ebp-24h]
    int32_t ammoStock; // [esp+78h] [ebp-20h]
    int32_t weapIndex; // [esp+80h] [ebp-18h]
    const playerState_s *ps; // [esp+84h] [ebp-14h]
    float colorMod[4]; // [esp+88h] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->predictedPlayerState;

    colorMod[0] = color[0];
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];

    if (colorMod[3] != 0.0)
    {
        WeaponIndex = GetWeaponIndex(cgameGlob);
        weapIndex = ClipCounterWeapIdx(cgameGlob, WeaponIndex);
        if (weapIndex)
        {
            if (!BG_GetWeaponDef(weapIndex)->suppressAmmoReserveDisplay)
            {
                ammoStock = BG_GetTotalAmmoReserve(ps, weapIndex);
                if (CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapIndex))
                {
                    colorMod[0] = 1.0f;
                    colorMod[1] = 0.3f;
                    colorMod[2] = 0.3f;
                }
                Com_sprintf(str, 0x40u, "%3i", ammoStock);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    str,
                    0x7FFFFFFF,
                    font,
                    rect->x,
                    rect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    scale,
                    colorMod,
                    textStyle);
            }
        }
    }
}

uint32_t __cdecl ClipCounterWeapIdx(const cg_s *cgameGlob, uint32_t weapIndex)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (!weapIndex)
        return 0;
    weapDef = BG_GetWeaponDef(weapIndex);
    if (weapDef->ammoCounterClip == AMMO_COUNTER_CLIP_NONE)
        return 0;
    if (weapDef->ammoCounterClip == AMMO_COUNTER_CLIP_ALTWEAPON)
        return GetWeaponAltIndex(cgameGlob, weapDef);
    return weapIndex;
}

uint32_t __cdecl GetWeaponAltIndex(const cg_s *cgameGlob, const WeaponDef *weapDef)
{
    const WeaponDef *weapDefAlt; // [esp+0h] [ebp-4h]

    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 89, 0, "%s", "weapDef");
    if (weapDef->altWeaponIndex)
    {
        weapDefAlt = BG_GetWeaponDef(weapDef->altWeaponIndex);
        if (weapDefAlt->ammoCounterClip == AMMO_COUNTER_CLIP_ALTWEAPON)
        {
            Com_PrintWarning(
                17,
                "Weapon \"%s\" and it's altweapon \"%s\" both have their ammoCounterClip property set to \"AltWeapon\".\n",
                weapDef->szInternalName,
                weapDefAlt->szInternalName);
            return 0;
        }
        else
        {
            return weapDef->altWeaponIndex;
        }
    }
    else
    {
        Com_PrintWarning(
            17,
            "Weapon \"%s\" ammoCounterClip property is set to \"AltWeapon\", but it has no alternate weapon.\n",
            weapDef->szInternalName);
        return 0;
    }
}

double __cdecl AmmoCounterFadeAlpha(int32_t localClientNum, cg_s *cgameGlob)
{
    float v3; // [esp+4h] [ebp-10h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 152, 0, "%s", "cgameGlob");
    if ((cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0)
        return 0.0;

    return CG_FadeHudMenu(
        localClientNum,
        hud_fade_ammodisplay,
        cgameGlob->ammoFadeTime,
        (int)(hud_fade_ammodisplay->current.value * 1000.0f));
}

double __cdecl CG_GetHudAlphaDPad(int32_t localClientNum)
{
    cg_s *LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    return DpadFadeAlpha(localClientNum, LocalClientGlobals);
}

double __cdecl DpadFadeAlpha(int32_t localClientNum, cg_s *cgameGlob)
{
    uint32_t idx; // [esp+14h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 578, 0, "%s", "cgameGlob");
    if ((cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0)
        return 0.0;
    for (idx = 0; idx < 4; ++idx)
    {
        if (ActionSlotIsActive(localClientNum, idx))
            return 1.0;
    }
    return AmmoCounterFadeAlpha(localClientNum, cgameGlob);
}

bool __cdecl ActionSlotIsActive(int32_t localClientNum, uint32_t slotIdx)
{
    ActionSlotType v3; // [esp+0h] [ebp-10h]
    playerState_s *ps; // [esp+8h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    v3 = cgameGlob->predictedPlayerState.actionSlotType[slotIdx];

    switch (v3)
    {
    case ACTIONSLOTTYPE_SPECIFYWEAPON:
        if (cgameGlob->weaponSelect == ps->actionSlotParam[slotIdx].specifyWeapon.index)
            return 1;
        break;
    case ACTIONSLOTTYPE_ALTWEAPONTOGGLE:
        if (BG_GetWeaponDef(cgameGlob->weaponSelect)->inventoryType == WEAPINVENTORY_ALTMODE)
            return 1;
        break;
    case ACTIONSLOTTYPE_NIGHTVISION:
        if (CG_LookingThroughNightVision(localClientNum))
            return 1;
        break;
    default:
        iassert(ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_DONOTHING);
        break;
    }

    return 0;
}

double __cdecl CG_GetHudAlphaAmmoCounter(int32_t localClientNum)
{
    cg_s *LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    return AmmoCounterFadeAlpha(localClientNum, LocalClientGlobals);
}

bool __cdecl CG_ActionSlotIsUsable(int32_t localClientNum, uint32_t slotIdx)
{
    uint32_t weapIdx; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]
    cg_s *cgameGlob;

    bcassert2(slotIdx, ACTIONSLOTS_NUM);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;

    switch (cgameGlob->predictedPlayerState.actionSlotType[slotIdx])
    {
    case ACTIONSLOTTYPE_SPECIFYWEAPON:
        weapIdx = ps->actionSlotParam[slotIdx].specifyWeapon.index;
        if (weapIdx)
        {
            iassert(ps);
            if (Com_BitCheckAssert(cgameGlob->predictedPlayerState.weapons, weapIdx, 16))
                return 1;
        }
        break;

    case ACTIONSLOTTYPE_ALTWEAPONTOGGLE:
        if (CG_AltWeaponToggleIndex(localClientNum, cgameGlob))
            return 1;
        break;

    case ACTIONSLOTTYPE_NIGHTVISION:
        return 1;

    default:
        iassert(ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_DONOTHING);
        break;
    }
    return 0;
}

void __cdecl CG_DrawPlayerActionSlotDpad(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material)
{
    float x; // [esp+30h] [ebp-30h]
    float y; // [esp+34h] [ebp-2Ch]
    float h; // [esp+38h] [ebp-28h]
    float w; // [esp+3Ch] [ebp-24h]
    ScreenPlacement *scrPlace; // [esp+40h] [ebp-20h]
    int32_t idx; // [esp+48h] [ebp-18h]
    float colorMod[4]; // [esp+50h] [ebp-10h] BYREF
    cg_s *LocalClientGlobals;

    iassert(rect);

    if (CG_ActionSlotIsUsable(localClientNum, 0)
        || CG_ActionSlotIsUsable(localClientNum, 1u)
        || CG_ActionSlotIsUsable(localClientNum, 2u)
        || CG_ActionSlotIsUsable(localClientNum, 3u))
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);

        colorMod[0] = color[0];
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = DpadFadeAlpha(localClientNum, LocalClientGlobals) * colorMod[3];

        if (colorMod[3] != 0.0)
        {
            scrPlace = &scrPlaceView[localClientNum];
            CL_DrawStretchPic(
                scrPlace,
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
                colorMod,
                material);
            colorMod[0] = 1.0f;
            colorMod[1] = 0.97f;
            colorMod[2] = 0.55f;
            for (idx = 0; idx < 4; ++idx)
            {
                if (ActionSlotIsActive(localClientNum, idx))
                {
                    x = rect->w * (float)MY_DIMS[idx + 4][0] + rect->x;
                    y = rect->h * (float)MY_DIMS[idx + 4][1] + rect->y;
                    w = rect->w * (float)MY_DIMS[idx][0];
                    h = rect->h * (float)MY_DIMS[idx][1];
                    if ((uint32_t)idx > 1)
                        CL_DrawStretchPicFlipST(
                            scrPlace,
                            x,
                            y,
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            MY_ST[idx][0],
                            MY_ST[idx][1],
                            MY_ST[idx][2],
                            MY_ST[idx][3],
                            colorMod,
                            cgMedia.hudDpadArrow);
                    else
                        CL_DrawStretchPic(
                            scrPlace,
                            x,
                            y,
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            MY_ST[idx][0],
                            MY_ST[idx][1],
                            MY_ST[idx][2],
                            MY_ST[idx][3],
                            colorMod,
                            cgMedia.hudDpadArrow);
                }
            }
        }
    }
}
void __cdecl CG_DrawPlayerActionSlot(
    int32_t localClientNum,
    const rectDef_s* rect,
    uint32_t slotIdx,
    float* color,
    Font_s* textFont,
    float textScale,
    int32_t textStyle)
{
    int32_t v8; // eax
    ActionSlotType v9; // [esp+30h] [ebp-7Ch]
    char str[64]; // [esp+34h] [ebp-78h] BYREF
    int32_t ammo; // [esp+78h] [ebp-34h]
    cg_s* cgameGlob; // [esp+7Ch] [ebp-30h]
    uint32_t weapIdx; // [esp+80h] [ebp-2Ch]
    const playerState_s* ps; // [esp+84h] [ebp-28h]
    float colorMod[4]; // [esp+88h] [ebp-24h] BYREF
    float x; // [esp+98h] [ebp-14h] BYREF
    float y; // [esp+9Ch] [ebp-10h] BYREF
    WeaponDef* weapDef; // [esp+A0h] [ebp-Ch]
    float h; // [esp+A4h] [ebp-8h] BYREF
    float w; // [esp+A8h] [ebp-4h] BYREF

    iassert(rect);
    bcassert2(slotIdx, ACTIONSLOTS_NUM);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->predictedPlayerState;

    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];

    if (ActionSlotIsActive(localClientNum, slotIdx))
    {
        Vec3Mul(colorMod, MY_ACTIVECOLOR, colorMod);
        colorMod[3] = 1.0f;
    }

    colorMod[3] = DpadFadeAlpha(localClientNum, cgameGlob) * colorMod[3];

    if (colorMod[3] != 0.0f)
    {
        v9 = ps->actionSlotType[slotIdx];
        if (v9 != ACTIONSLOTTYPE_SPECIFYWEAPON)
        {
            if (v9 != ACTIONSLOTTYPE_ALTWEAPONTOGGLE)
            {
                if (v9 == ACTIONSLOTTYPE_NIGHTVISION)
                {
                    if (cgMedia.hudIconNVG)
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
                            colorMod,
                            cgMedia.hudIconNVG);
                    else
                        Com_PrintWarning(14, "CG_DrawNightVisionOverlay(): Nightvision Assets not Precached.\n");
                }
                else if (ps->actionSlotType[slotIdx])
                {
                    MyAssertHandler(
                        ".\\cgame\\cg_ammocounter.cpp",
                        794,
                        0,
                        "%s",
                        "ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_DONOTHING");
                }
                return;
            }
            weapIdx = cgameGlob->weaponSelect;
            weapDef = BG_GetWeaponDef(weapIdx);
            if (weapDef->inventoryType != WEAPINVENTORY_ALTMODE)
            {
                weapIdx = CG_AltWeaponToggleIndex(localClientNum, cgameGlob);
                if (!weapIdx)
                    return;
                weapDef = BG_GetWeaponDef(weapIdx);
            }
            DpadIconDims(rect, slotIdx, weapDef, &x, &y, &w, &h);
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
                colorMod,
                weapDef->dpadIcon);
            ammo = BG_WeaponAmmo(ps, weapIdx);
            if (!ammo)
            {
                colorMod[0] = 1.0f;
                colorMod[1] = 0.30000001f;
                colorMod[2] = 0.30000001f;
            }
        LABEL_21:
            DpadTextPos(rect, slotIdx, weapDef, &x, &y);
            v8 = BG_WeaponAmmo(ps, weapIdx);
            Com_sprintf(str, 0x40u, "%3i", v8);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                str,
                0x7FFFFFFF,
                textFont,
                x,
                y,
                rect->horzAlign,
                rect->vertAlign,
                textScale,
                colorMod,
                textStyle);
            return;
        }
        weapIdx = ps->actionSlotParam[slotIdx].specifyWeapon.index;
        if (weapIdx)
        {
            if (!ps)
                MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
            if (Com_BitCheckAssert(ps->weapons, weapIdx, 16))
            {
                weapDef = BG_GetWeaponDef(weapIdx);
                DpadIconDims(rect, slotIdx, weapDef, &x, &y, &w, &h);
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
                    colorMod,
                    weapDef->dpadIcon);
                ammo = BG_WeaponAmmo(ps, weapIdx);
                if (!ammo)
                {
                    colorMod[0] = 1.0f;
                    colorMod[1] = 0.30000001f;
                    colorMod[2] = 0.30000001f;
                }
                goto LABEL_21;
            }
        }
    }
}

void __cdecl DpadIconDims(
    const rectDef_s *rect,
    uint32_t slotIdx,
    WeaponDef *weapDef,
    float *x,
    float *y,
    float *w,
    float *h)
{
    if (!rect)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 490, 0, "%s", "rect");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 491, 0, "%s", "weapDef");
    if (!x)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 492, 0, "%s", "x");
    if (!y)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 493, 0, "%s", "y");
    if (!w)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 494, 0, "%s", "w");
    if (!h)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 495, 0, "%s", "h");
    if (weapDef->dpadIconRatio == WEAPON_ICON_RATIO_4TO1)
    {
        *x = rect->x;
        *w = rect->w + rect->w;
        *y = rect->h * 0.25f + rect->y;
        *h = rect->h * 0.5f;
    }
    else
    {
        if (weapDef->dpadIconRatio == WEAPON_ICON_RATIO_2TO1)
        {
            *x = rect->x;
            *w = rect->w + rect->w;
            *y = rect->y;
        }
        else
        {
            *x = rect->x;
            *y = rect->y;
            *w = rect->w;
        }
        *h = rect->h;
    }
}

void __cdecl DpadTextPos(const rectDef_s *rect, uint32_t slotIdx, WeaponDef *weapDef, float *x, float *y)
{
    if (!rect)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 526, 0, "%s", "rect");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 527, 0, "%s", "weapDef");
    if (!x)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 528, 0, "%s", "x");
    if (!y)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 529, 0, "%s", "y");
    *x = rect->x + rect->w + -13.0f;
    *y = rect->y + rect->h + 3.0f;
    if (weapDef->dpadIconRatio == WEAPON_ICON_RATIO_2TO1 || weapDef->dpadIconRatio == WEAPON_ICON_RATIO_4TO1)
        *x = *x + rect->w;
}

void __cdecl CG_DrawPlayerWeaponBackground(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material)
{
    float colorMod[4]; // [esp+4Ch] [ebp-10h] BYREF
    cg_s *cgameGlob;

    iassert(rect);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    colorMod[0] = color[0];
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];

    if (colorMod[3] != 0.0f)
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
            colorMod,
            material);
}

void __cdecl CG_DrawPlayerWeaponAmmoClipGraphic(int32_t localClientNum, const rectDef_s *rect, const float *color)
{
    int32_t weapIdx; // [esp+18h] [ebp-20h]
    float base[2]; // [esp+1Ch] [ebp-1Ch] BYREF
    float colorMod[4]; // [esp+24h] [ebp-14h] BYREF
    WeaponDef *weapDef; // [esp+34h] [ebp-4h]
    cg_s *cgameGlob;

    iassert(rect);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    colorMod[0] = color[0];
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];

    if (colorMod[3] != 0.0f)
    {
        weapIdx = GetWeaponIndex(cgameGlob);
        if (weapIdx)
        {
            weapDef = BG_GetWeaponDef(weapIdx);
            GetBaseRectPos(localClientNum, rect, base);
            DrawClipAmmo(cgameGlob, base, weapIdx, weapDef, colorMod);
        }
    }
}

void __cdecl GetBaseRectPos(int32_t localClientNum, const rectDef_s *rect, float *base)
{
    float dummyW; // [esp+0h] [ebp-8h] BYREF
    float dummyH; // [esp+4h] [ebp-4h] BYREF

    if (!rect)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 69, 0, "%s", "rect");
    *base = rect->x;
    base[1] = rect->y;
    ScrPlace_ApplyRect(&scrPlaceView[localClientNum], base, base + 1, &dummyW, &dummyH, rect->horzAlign, rect->vertAlign);
}

void __cdecl DrawClipAmmo(cg_s *cgameGlob, float *base, uint32_t weapIdx, const WeaponDef *weapDef, float *color)
{
    WeaponDef *weapDefAlt; // [esp+4h] [ebp-8h]
    int32_t weapIdxAlt; // [esp+8h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 324, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 325, 0, "%s", "weapDef");
    switch (weapDef->ammoCounterClip)
    {
    case AMMO_COUNTER_CLIP_MAGAZINE:
        DrawClipAmmoMagazine(cgameGlob, base, weapIdx, weapDef, color);
        break;
    case AMMO_COUNTER_CLIP_SHORTMAGAZINE:
        DrawClipAmmoShortMagazine(cgameGlob, base, weapIdx, weapDef, color);
        break;
    case AMMO_COUNTER_CLIP_SHOTGUN:
        DrawClipAmmoShotgunShells(cgameGlob, base, weapIdx, weapDef, color);
        break;
    case AMMO_COUNTER_CLIP_ROCKET:
        DrawClipAmmoRockets(cgameGlob, base, weapIdx, weapDef, color);
        break;
    case AMMO_COUNTER_CLIP_BELTFED:
        DrawClipAmmoBeltfed(cgameGlob, base, weapIdx, weapDef, color);
        break;
    case AMMO_COUNTER_CLIP_ALTWEAPON:
        weapIdxAlt = GetWeaponAltIndex(cgameGlob, weapDef);
        if (weapIdxAlt)
        {
            weapDefAlt = BG_GetWeaponDef(weapIdxAlt);
            DrawClipAmmo(cgameGlob, base, weapIdxAlt, weapDefAlt, color);
        }
        break;
    default:
        if (weapDef->ammoCounterClip)
            MyAssertHandler(
                ".\\cgame\\cg_ammocounter.cpp",
                354,
                0,
                "%s",
                "weapDef->ammoCounterClip == AMMO_COUNTER_CLIP_NONE");
        break;
    }
}

void __cdecl DrawClipAmmoMagazine(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color)
{
    float bulletX; // [esp+34h] [ebp-14h]
    float bulletY; // [esp+38h] [ebp-10h]
    int32_t clipIdx; // [esp+3Ch] [ebp-Ch]
    int32_t clipCnt; // [esp+44h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 170, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 171, 0, "%s", "weapDef");
    bulletX = *base - 4.0f;
    bulletY = base[1] - 8.0f * 0.5f;
    clipCnt = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIdx)];
    AmmoColor(cgameGlob, color, weapIdx);
    for (clipIdx = 0; clipIdx < weapDef->iClipSize; ++clipIdx)
    {
        if (clipIdx == clipCnt)
        {
            *color = 0.30000001f;
            color[1] = 0.30000001f;
            color[2] = 0.30000001f;
        }
        CL_DrawStretchPicPhysical(bulletX, bulletY, 4.0, 8.0, 0.0, 0.0, 1.0, 1.0, color, cgMedia.ammoCounterBullet);
        bulletX = bulletX - 4.0f;
    }
}

void __cdecl AmmoColor(cg_s *cgameGlob, float *color, uint32_t weapIndex)
{
    float v3; // [esp+0h] [ebp-10h]
    float delta; // [esp+8h] [ebp-8h]
    float deltaa; // [esp+8h] [ebp-8h]
    float deltab; // [esp+8h] [ebp-8h]
    int32_t idx; // [esp+Ch] [ebp-4h]

    if (CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex))
    {
        delta = (float)(cgameGlob->time - cgameGlob->lastClipFlashTime);
        deltaa = delta / (MYFLASHTERM * 3.141592741012573f);
        v3 = sin(deltaa);
        for (idx = 0; idx < 3; ++idx)
        {
            deltab = v3 * 0.5f + 0.5f;
            color[idx] = (colorLowAmmo[idx] - color[idx]) * deltab + color[idx];
        }
    }
    else
    {
        cgameGlob->lastClipFlashTime = cgameGlob->time;
    }
}

void __cdecl DrawClipAmmoShortMagazine(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color)
{
    float bulletX; // [esp+34h] [ebp-14h]
    float bulletY; // [esp+38h] [ebp-10h]
    int32_t clipIdx; // [esp+3Ch] [ebp-Ch]
    int32_t clipCnt; // [esp+44h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 200, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 201, 0, "%s", "weapDef");
    bulletX = *base - 32.0f;
    bulletY = base[1] - 8.0f * 0.5f;
    clipCnt = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIdx)];
    AmmoColor(cgameGlob, color, weapIdx);
    for (clipIdx = 0; clipIdx < weapDef->iClipSize; ++clipIdx)
    {
        if (clipIdx == clipCnt)
        {
            *color = 0.30000001f;
            color[1] = 0.30000001f;
            color[2] = 0.30000001f;
        }
        CL_DrawStretchPicPhysical(bulletX, bulletY, 32.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, color, cgMedia.ammoCounterRifleBullet);
        bulletX = bulletX - 40.0f;
    }
}

void __cdecl DrawClipAmmoShotgunShells(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color)
{
    int32_t magCnt; // [esp+34h] [ebp-14h]
    float bulletX; // [esp+38h] [ebp-10h]
    float bulletY; // [esp+3Ch] [ebp-Ch]
    int32_t magIdx; // [esp+44h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 230, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 231, 0, "%s", "weapDef");
    bulletX = *base - TEST_bullet_wh_1[0];
    bulletY = base[1] - TEST_bullet_wh_1[1] * 0.5f;
    magCnt = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIdx)];
    AmmoColor(cgameGlob, color, weapIdx);
    for (magIdx = 0; magIdx < weapDef->iClipSize; ++magIdx)
    {
        if (magIdx == magCnt)
        {
            *color = 0.30000001f;
            color[1] = 0.30000001f;
            color[2] = 0.30000001f;
        }
        CL_DrawStretchPicPhysical(
            bulletX,
            bulletY,
            TEST_bullet_wh_1[0],
            TEST_bullet_wh_1[1],
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            color,
            cgMedia.ammoCounterShotgunShell);
        bulletX = bulletX - TEST_bullet_step_1[0];
    }
}

void __cdecl DrawClipAmmoRockets(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color)
{
    int32_t magCnt; // [esp+34h] [ebp-14h]
    float bulletX; // [esp+38h] [ebp-10h]
    float bulletY; // [esp+3Ch] [ebp-Ch]
    int32_t magIdx; // [esp+44h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 260, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 261, 0, "%s", "weapDef");
    bulletX = *base - TEST_bullet_wh_2[0];
    bulletY = base[1] - TEST_bullet_wh_2[1] * 0.5;
    magCnt = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIdx)];
    AmmoColor(cgameGlob, color, weapIdx);
    for (magIdx = 0; magIdx < weapDef->iClipSize; ++magIdx)
    {
        if (magIdx == magCnt)
        {
            *color = 0.30000001f;
            color[1] = 0.30000001f;
            color[2] = 0.30000001f;
        }
        CL_DrawStretchPicPhysical(
            bulletX,
            bulletY,
            TEST_bullet_wh_2[0],
            TEST_bullet_wh_2[1],
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            color,
            cgMedia.ammoCounterRocket);
        bulletX = bulletX - TEST_bullet_step_2[0];
    }
}

void __cdecl DrawClipAmmoBeltfed(
    cg_s *cgameGlob,
    float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color)
{
    float stepX; // [esp+38h] [ebp-18h]
    float bulletX; // [esp+3Ch] [ebp-14h]
    float bulletY; // [esp+40h] [ebp-10h]
    int32_t clipIdx; // [esp+44h] [ebp-Ch]
    int32_t clipCnt; // [esp+4Ch] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 292, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 293, 0, "%s", "weapDef");
    stepX = TEST_bullet_step_3[0];
    bulletX = *base;
    bulletY = TEST_bullet_wh_3[1] * 0.25 * (double)(weapDef->iClipSize / TEST_bullet_rowCnt) + base[1];
    clipCnt = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIdx)];
    AmmoColor(cgameGlob, color, weapIdx);
    for (clipIdx = 0; clipIdx < weapDef->iClipSize; ++clipIdx)
    {
        if (clipIdx == clipCnt)
        {
            *color = 0.30000001f;
            color[1] = 0.30000001f;
            color[2] = 0.30000001f;
        }
        if (!(clipIdx % TEST_bullet_rowCnt))
        {
            stepX = stepX * -1.0f;
            bulletY = bulletY + TEST_bullet_step_3[1];
            bulletX = bulletX + stepX;
        }
        CL_DrawStretchPicPhysical(
            bulletX,
            bulletY,
            TEST_bullet_wh_3[0],
            TEST_bullet_wh_3[1],
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            color,
            cgMedia.ammoCounterBeltBullet);
        bulletX = bulletX + stepX;
    }
}

void __cdecl CG_DrawPlayerWeaponIcon(int32_t localClientNum, const rectDef_s *rect, const float *color)
{
    int32_t weapIdx; // [esp+18h] [ebp-18h]
    float colorMod[4]; // [esp+1Ch] [ebp-14h] BYREF
    WeaponDef *weapDef; // [esp+2Ch] [ebp-4h]
    cg_s *cgameGlob;

    iassert(rect);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    colorMod[0] = color[0];
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];

    if (colorMod[3] != 0.0f)
    {
        weapIdx = GetWeaponIndex(cgameGlob);
        if (weapIdx)
        {
            weapDef = BG_GetWeaponDef(weapIdx);
            DrawStretchPicGun(
                &scrPlaceView[localClientNum],
                rect,
                colorMod,
                weapDef->ammoCounterIcon,
                weapDef->ammoCounterIconRatio);
        }
    }
}

void __cdecl DrawStretchPicGun(
    const ScreenPlacement *scrPlace,
    const rectDef_s *rect,
    const float *color,
    Material *material,
    weaponIconRatioType_t ratio)
{
    float x; // [esp+2Ch] [ebp-10h] BYREF
    float y; // [esp+30h] [ebp-Ch] BYREF
    float h; // [esp+34h] [ebp-8h] BYREF
    float w; // [esp+38h] [ebp-4h] BYREF

    if (!rect)
        MyAssertHandler(".\\cgame\\cg_ammocounter.cpp", 850, 0, "%s", "rect");
    x = rect->x;
    y = rect->y;
    w = rect->w;
    h = rect->h;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
    if (ratio)
    {
        if (ratio == WEAPON_ICON_RATIO_2TO1)
        {
            x = x - w;
            w = w + w;
        }
        else
        {
            if (ratio != WEAPON_ICON_RATIO_4TO1)
                MyAssertHandler(
                    ".\\cgame\\cg_ammocounter.cpp",
                    866,
                    0,
                    "%s\n\t(ratio) = %i",
                    "(ratio == WEAPON_ICON_RATIO_4TO1)",
                    ratio);
            x = x - w * 3.0f;
            w = w * 4.0f;
        }
    }
    CL_DrawStretchPicPhysical(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, color, material);
}

void __cdecl CG_DrawPlayerWeaponLowAmmoWarning(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float textScale,
    int32_t textStyle,
    float text_x,
    float text_y,
    char textAlignMode,
    Material *material)
{
    float v9; // [esp+30h] [ebp-9Ch]
    float v10; // [esp+34h] [ebp-98h]
    float v11; // [esp+38h] [ebp-94h]
    float v12; // [esp+44h] [ebp-88h]
    float fade; // [esp+5Ch] [ebp-70h]
    float frac; // [esp+60h] [ebp-6Ch]
    float amplitude; // [esp+68h] [ebp-64h]
    bool canReload; // [esp+6Eh] [ebp-5Eh]
    bool empty; // [esp+6Fh] [ebp-5Dh]
    char *localizedString; // [esp+70h] [ebp-5Ch]
    float color1[4]; // [esp+74h] [ebp-58h] BYREF
    int32_t weapIndex; // [esp+84h] [ebp-48h]
    float bias; // [esp+88h] [ebp-44h]
    float colorMod[4]; // [esp+8Ch] [ebp-40h] BYREF
    WeaponDef *weapDef; // [esp+9Ch] [ebp-30h]
    const char *text; // [esp+A0h] [ebp-2Ch]
    rectDef_s textRect; // [esp+A4h] [ebp-28h] BYREF
    float color2[4]; // [esp+BCh] [ebp-10h] BYREF
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ( cgameGlob->predictedPlayerState.pm_type < 7
        && cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0
        && cgameGlob->predictedPlayerState.weaponstate != 7
        && cgameGlob->predictedPlayerState.weaponstate != 9
        && cgameGlob->predictedPlayerState.weaponstate != 11
        && cgameGlob->predictedPlayerState.weaponstate != 10
        && cgameGlob->predictedPlayerState.weaponstate != 8)
    {
        if (cgameGlob->predictedPlayerState.weapon)
        {
            fade = AmmoCounterFadeAlpha(localClientNum, cgameGlob);
            if (fade != 0.0)
            {
                weapIndex = GetWeaponIndex(cgameGlob);
                if (CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex))
                {
                    canReload = cgameGlob->predictedPlayerState.ammo[BG_AmmoForWeapon(weapIndex)] > 0;
                    empty = cgameGlob->predictedPlayerState.ammoclip[BG_ClipForWeapon(weapIndex)] == 0;
                    weapDef = BG_GetWeaponDef(weapIndex);
                    if (weapDef->ammoCounterClip)
                    {
                        if (canReload)
                        {
                            if (weapDef->iClipSize == 1)
                                return;
                            text = "PLATFORM_RELOAD";
                            Byte4UnpackRgba((const uint8_t *)&lowAmmoWarningColor1->current, color1);
                            Byte4UnpackRgba((const uint8_t *)&lowAmmoWarningColor2->current, color2);
                        }
                        else if (empty)
                        {
                            text = "WEAPON_NO_AMMO";
                            Byte4UnpackRgba((const uint8_t *)&lowAmmoWarningNoAmmoColor1->current, color1);
                            Byte4UnpackRgba((const uint8_t *)&lowAmmoWarningNoAmmoColor2->current, color2);
                        }
                        else
                        {
                            text = "PLATFORM_LOW_AMMO_NO_RELOAD";
                            Byte4UnpackRgba((const uint8_t *)&lowAmmoWarningNoReloadColor1->current, color1);
                            Byte4UnpackRgba((const uint8_t *)&lowAmmoWarningNoReloadColor2->current, color2);
                        }
                        amplitude = (lowAmmoWarningPulseMax->current.value - lowAmmoWarningPulseMin->current.value) * 0.5f;
                        bias = lowAmmoWarningPulseMin->current.value + amplitude;
                        v12 = lowAmmoWarningPulseFreq->current.value * ((float)cgameGlob->time * 0.006283185444772243f);
                        v11 = sin(v12);
                        frac = v11 * amplitude + bias;
                        Vec4Lerp(color1, color2, frac, colorMod);
                        colorMod[3] = colorMod[3] * fade;
                        if (material)
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
                                colorMod,
                                material);
                        localizedString = SEH_LocalizeTextMessage(text, "low ammo warning", LOCMSG_SAFE);
                        v10 = rect->y + text_y;
                        v9 = rect->x + text_x;
                        UI_DrawWrappedText(
                            &scrPlaceView[localClientNum],
                            localizedString,
                            rect,
                            font,
                            v9,
                            v10,
                            textScale,
                            colorMod,
                            textStyle,
                            textAlignMode,
                            &textRect);
                    }
                }
            }
        }
    }
}

uint32_t __cdecl GetWeaponIndex(const cg_s *cgameGlob)
{
    uint32_t bitNum; // [esp+0h] [ebp-8h]

    if (cgameGlob->weaponSelect >= BG_GetNumWeapons())
        return cgameGlob->predictedPlayerState.weapon;
    bitNum = cgameGlob->weaponSelect;
    if (cgameGlob == (const cg_s *)-287036)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
    if (Com_BitCheckAssert(cgameGlob->predictedPlayerState.weapons, bitNum, 16))
        return cgameGlob->weaponSelect;
    else
        return cgameGlob->predictedPlayerState.weapon;
}

