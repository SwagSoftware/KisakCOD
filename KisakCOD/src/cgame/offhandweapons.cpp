#include "cg_local.h"
#include "cg_public.h"

#include <cgame_mp/cg_local_mp.h>

void __cdecl CG_OffhandRegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]

    min.value.max = 30.0;
    min.value.min = 0.0;
    hud_flash_time_offhand = Dvar_RegisterFloat(
        "hud_flash_time_offhand",
        2.0,
        min,
        1u,
        "Offhand weapons flash duration on changing weapon");
    mina.value.max = 30.0;
    mina.value.min = 0.0;
    hud_flash_period_offhand = Dvar_RegisterFloat(
        "hud_flash_period_offhand",
        0.5,
        mina,
        1u,
        "Offhand weapons flash period on changing weapon");
}

void __cdecl CG_DrawOffHandIcon(
    int localClientNum,
    const rectDef_s *rect,
    float scale,
    const float *color,
    Material *material,
    OffhandClass weaponType)
{
    float v6; // [esp+24h] [ebp-30h]
    float drawColor[4]; // [esp+38h] [ebp-1Ch] BYREF
    int weapIndex; // [esp+48h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4Ch] [ebp-8h]
    const WeaponDef *equippedWeapDef; // [esp+50h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 100, 0, "%s", "rect");
    if (!color)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 101, 0, "%s", "color");
    if (weaponType <= OFFHAND_CLASS_NONE)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 102, 0, "%s", "weaponType > OFFHAND_CLASS_NONE");
    if (weaponType >= OFFHAND_CLASS_COUNT)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 103, 0, "%s", "weaponType < OFFHAND_CLASS_COUNT");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (IsOffHandDisplayVisible(cgArray))
    {
        if (GetBestOffhand((const playerState_s *)&MEMORY[0x9D5574], weaponType))
        {
            v6 = hud_fade_offhand->current.value * 1000.0;
            drawColor[3] = CG_FadeHudMenu(
                localClientNum,
                hud_fade_offhand,
                MEMORY[0x9DF8F0][4],
                (int)(v6 + 9.313225746154785e-10))
                * color[3];
            if (drawColor[3] != 0.0)
            {
                drawColor[0] = *color;
                drawColor[1] = color[1];
                drawColor[2] = color[2];
                weapIndex = 0;
                if (MEMORY[0x9DF71C][38])
                {
                    weapDef = BG_GetWeaponDef(MEMORY[0x9DF71C][38]);
                    if (weapDef->offhandClass == weaponType)
                        weapIndex = MEMORY[0x9DF71C][38];
                }
                if (!weapIndex)
                    weapIndex = GetBestOffhand((const playerState_s *)&MEMORY[0x9D5574], weaponType);
                if (weapIndex)
                {
                    equippedWeapDef = BG_GetWeaponDef(weapIndex);
                    UI_DrawHandlePic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        drawColor,
                        equippedWeapDef->hudIcon);
                }
            }
        }
    }
}

int __cdecl GetBestOffhand(const playerState_s *predictedPlayerState, int offhandClass)
{
    int newOffhand; // [esp+0h] [ebp-4h]

    newOffhand = BG_GetFirstAvailableOffhand(predictedPlayerState, offhandClass);
    if (!newOffhand)
        return BG_GetFirstEquippedOffhand(predictedPlayerState, offhandClass);
    return newOffhand;
}

bool __cdecl IsOffHandDisplayVisible(const cg_s *cgameGlob)
{
    return cgameGlob->predictedPlayerState.pm_type < 7 && (cgameGlob->predictedPlayerState.weapFlags & 0x80) == 0;
}

void __cdecl CG_DrawOffHandHighlight(
    int localClientNum,
    const rectDef_s *rect,
    float scale,
    const float *color,
    Material *material,
    OffhandClass weaponType)
{
    float v6; // [esp+24h] [ebp-38h]
    float drawColor[4]; // [esp+38h] [ebp-24h] BYREF
    float flashColor[4]; // [esp+48h] [ebp-14h] BYREF
    WeaponDef *weapDef; // [esp+58h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 206, 0, "%s", "rect");
    if (!color)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 207, 0, "%s", "color");
    if (weaponType <= OFFHAND_CLASS_NONE)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 208, 0, "%s", "weaponType > OFFHAND_CLASS_NONE");
    if (weaponType >= OFFHAND_CLASS_COUNT)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 209, 0, "%s", "weaponType < OFFHAND_CLASS_COUNT");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (IsOffHandDisplayVisible(cgArray))
    {
        if (GetBestOffhand((const playerState_s *)&MEMORY[0x9D5574], weaponType))
        {
            if (MEMORY[0x9DF71C][38])
            {
                v6 = hud_fade_offhand->current.value * 1000.0;
                drawColor[3] = CG_FadeHudMenu(
                    localClientNum,
                    hud_fade_offhand,
                    MEMORY[0x9DF8F0][4],
                    (int)(v6 + 9.313225746154785e-10))
                    * color[3];
                if (drawColor[3] != 0.0)
                {
                    weapDef = BG_GetWeaponDef(MEMORY[0x9DF71C][38]);
                    if (weaponType == weapDef->offhandClass)
                    {
                        if (CalcOffHandAmmo((const playerState_s *)&MEMORY[0x9D5574], weaponType))
                        {
                            drawColor[0] = *color;
                            drawColor[1] = color[1];
                            drawColor[2] = color[2];
                        }
                        else
                        {
                            drawColor[0] = 0.88999999;
                            drawColor[1] = 0.18000001;
                            drawColor[2] = 0.0099999998;
                        }
                        OffHandFlash(cgArray, drawColor, flashColor);
                        UI_DrawHandlePic(
                            &scrPlaceView[localClientNum],
                            rect->x,
                            rect->y,
                            rect->w,
                            rect->h,
                            rect->horzAlign,
                            rect->vertAlign,
                            flashColor,
                            material);
                    }
                }
            }
        }
    }
}

void __cdecl OffHandFlash(const cg_s *cgameGlob, const float *base_color, float *out_color)
{
    float v3; // [esp+0h] [ebp-14h]
    float phi; // [esp+8h] [ebp-Ch]
    float fade; // [esp+Ch] [ebp-8h]
    float flashTime; // [esp+10h] [ebp-4h]

    if (!base_color)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 147, 0, "%s", "base_color");
    if (!out_color)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 148, 0, "%s", "out_color");
    if (!hud_flash_time_offhand)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 149, 0, "%s", "hud_flash_time_offhand");
    if (!hud_flash_period_offhand)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 150, 0, "%s", "hud_flash_period_offhand");
    *out_color = *base_color;
    out_color[1] = base_color[1];
    out_color[2] = base_color[2];
    out_color[3] = base_color[3];
    flashTime = (double)(cgameGlob->time - cgameGlob->offhandFlashTime) / 1000.0;
    if (hud_flash_time_offhand->current.value > (double)flashTime)
    {
        if (hud_flash_period_offhand->current.value <= 0.0)
            MyAssertHandler(".\\cgame\\offhandweapons.cpp", 157, 0, "%s", "hud_flash_period_offhand->current.value > 0.0f");
        phi = flashTime * 6.283185482025146 / hud_flash_period_offhand->current.value;
        v3 = cos(phi);
        fade = v3 * 0.5 + 0.5;
        out_color[3] = fade * base_color[3];
    }
}

int __cdecl CalcOffHandAmmo(const playerState_s *predictedPlayerState, int weaponType)
{
    int ammoCount; // [esp+0h] [ebp-10h]
    unsigned int weapCount; // [esp+4h] [ebp-Ch]
    unsigned int weapIndex; // [esp+8h] [ebp-8h]
    WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    ammoCount = 0;
    weapCount = BG_GetNumWeapons();
    for (weapIndex = 1; weapIndex < weapCount; ++weapIndex)
    {
        if (!predictedPlayerState)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../bgame/bg_weapons.h", 229, 0, "%s", "ps");
        if (Com_BitCheckAssert(predictedPlayerState->weapons, weapIndex, 16))
        {
            weapDef = BG_GetWeaponDef(weapIndex);
            if (!weapDef)
                MyAssertHandler(".\\cgame\\offhandweapons.cpp", 186, 0, "%s", "weapDef");
            if (weapDef->offhandClass == weaponType)
                ammoCount += predictedPlayerState->ammoclip[BG_ClipForWeapon(weapIndex)];
        }
    }
    return ammoCount;
}

void __cdecl CG_DrawOffHandAmmo(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle,
    OffhandClass weaponType)
{
    float v7; // [esp+20h] [ebp-2Ch]
    int ammoCount; // [esp+30h] [ebp-1Ch]
    float drawColor[4]; // [esp+38h] [ebp-14h] BYREF
    const char *ammoCountString; // [esp+48h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 250, 0, "%s", "rect");
    if (!color)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 251, 0, "%s", "color");
    if (weaponType <= OFFHAND_CLASS_NONE)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 252, 0, "%s", "weaponType > OFFHAND_CLASS_NONE");
    if (weaponType >= OFFHAND_CLASS_COUNT)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 253, 0, "%s", "weaponType < OFFHAND_CLASS_COUNT");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (IsOffHandDisplayVisible(cgArray))
    {
        if (GetBestOffhand((const playerState_s *)&MEMORY[0x9D5574], weaponType))
        {
            v7 = hud_fade_offhand->current.value * 1000.0;
            drawColor[3] = CG_FadeHudMenu(
                localClientNum,
                hud_fade_offhand,
                MEMORY[0x9DF8F0][4],
                (int)(v7 + 9.313225746154785e-10))
                * color[3];
            if (drawColor[3] != 0.0)
            {
                ammoCount = CalcOffHandAmmo((const playerState_s *)&MEMORY[0x9D5574], weaponType);
                ammoCountString = va("%i", ammoCount);
                if (ammoCount)
                {
                    drawColor[0] = *color;
                    drawColor[1] = color[1];
                    drawColor[2] = color[2];
                }
                else
                {
                    drawColor[0] = 0.88999999;
                    drawColor[1] = 0.18000001;
                    drawColor[2] = 0.0099999998;
                }
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    (char *)ammoCountString,
                    0x7FFFFFFF,
                    font,
                    rect->x,
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

void __cdecl CG_DrawOffHandName(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle,
    OffhandClass weaponType)
{
    float v7; // [esp+20h] [ebp-28h]
    float drawColor[4]; // [esp+34h] [ebp-14h] BYREF
    const char *ammoNameString; // [esp+44h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 287, 0, "%s", "rect");
    if (!color)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 288, 0, "%s", "color");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (IsOffHandDisplayVisible(cgArray) && GetBestOffhand((const playerState_s *)&MEMORY[0x9D5574], weaponType))
    {
        v7 = hud_fade_offhand->current.value * 1000.0;
        drawColor[3] = CG_FadeHudMenu(
            localClientNum,
            hud_fade_offhand,
            MEMORY[0x9DF8F0][4],
            (int)(v7 + 9.313225746154785e-10))
            * color[3];
        if (drawColor[3] != 0.0)
        {
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            ammoNameString = UI_SafeTranslateString((char *)offhandStrings[weaponType]);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                (char *)ammoNameString,
                0x7FFFFFFF,
                font,
                rect->x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                drawColor,
                textStyle);
        }
    }
}

void __cdecl CG_SwitchOffHandCmd(int localClientNum)
{
    unsigned int newOffhand; // [esp+4h] [ebp-8h]
    WeaponDef* weapDef; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].equippedOffHand)
    {
        weapDef = BG_GetWeaponDef(cgArray[0].equippedOffHand);
        if (weapDef->offhandClass == OFFHAND_CLASS_NONE)
            MyAssertHandler(".\\cgame\\offhandweapons.cpp", 323, 0, "%s", "weapDef->offhandClass != OFFHAND_CLASS_NONE");
        newOffhand = BG_GetFirstAvailableOffhand(&cgArray[0].predictedPlayerState, weapDef->offhandClass);
        if (newOffhand)
            CG_SetEquippedOffHand(localClientNum, newOffhand);
    }
}

void __cdecl CG_PrepOffHand(int localClientNum, const entityState_s *ent, unsigned int weaponIndex)
{
    WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if (ent->eType != 1)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 339, 0, "%s", "ent->eType == ET_PLAYER");
    if (!weaponIndex || weaponIndex >= BG_GetNumWeapons())
        MyAssertHandler(
            ".\\cgame\\offhandweapons.cpp",
            340,
            0,
            "%s\n\t(weaponIndex) = %i",
            "(weaponIndex > 0 && weaponIndex < BG_GetNumWeapons())",
            weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (weapDef->pullbackSound)
        CG_PlayEntitySoundAlias(localClientNum, ent->number, weapDef->pullbackSound);
}

void __cdecl CG_UseOffHand(int localClientNum, const centity_s *cent, unsigned int weaponIndex)
{
    const weaponInfo_s *weapInfo; // [esp+0h] [ebp-1Ch]
    DObj_s *obj; // [esp+4h] [ebp-18h]
    DObj_s *obja; // [esp+4h] [ebp-18h]
    float origin[3]; // [esp+8h] [ebp-14h] BYREF
    cg_s *cgameGlob; // [esp+14h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    if (cent->nextState.eType != 1)
        MyAssertHandler(".\\cgame\\offhandweapons.cpp", 360, 0, "%s", "cent->nextState.eType == ET_PLAYER");
    if (!weaponIndex || weaponIndex >= BG_GetNumWeapons())
        MyAssertHandler(
            ".\\cgame\\offhandweapons.cpp",
            361,
            0,
            "%s\n\t(weaponIndex) = %i",
            "(weaponIndex > 0 && weaponIndex < BG_GetNumWeapons())",
            weaponIndex);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1095,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    weapInfo = &cg_weaponsArray[0][weaponIndex];
    weapDef = BG_GetWeaponDef(weaponIndex);
    if (weapDef->fireSound)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgameGlob = cgArray;    
        if (cent->nextState.number == cgArray[0].nextSnap->ps.clientNum)
        {
            obj = weapInfo->viewModelDObj;
            CG_UpdateViewModelPose(weapInfo->viewModelDObj, localClientNum);
            if (!obj || !CG_DObjGetWorldTagPos(&cgameGlob->viewModelPose, obj, scr_const.tag_flash, origin))
                BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, origin);
        }
        else
        {
            obja = Com_GetClientDObj(cent->nextState.number, localClientNum);
            if (!obja || !CG_DObjGetWorldTagPos(&cent->pose, obja, scr_const.tag_flash, origin))
                BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, origin);
        }
        CG_PlaySoundAlias(localClientNum, cent->nextState.number, origin, weapDef->fireSound);
    }
}

void __cdecl CG_SetEquippedOffHand(int localClientNum, unsigned int offHandIndex)
{
    WeaponDef *WeaponDef; // eax
    const char *v3; // eax

    if (offHandIndex && BG_GetWeaponDef(offHandIndex)->offhandClass == OFFHAND_CLASS_NONE)
    {
        WeaponDef = BG_GetWeaponDef(offHandIndex);
        v3 = va("offHandIndex = %d (%s)\n", offHandIndex, WeaponDef->szInternalName);
        MyAssertHandler(
            ".\\cgame\\offhandweapons.cpp",
            402,
            0,
            "%s\n\t%s",
            "offHandIndex == WP_NONE || BG_GetWeaponDef( offHandIndex )->offhandClass != OFFHAND_CLASS_NONE",
            v3);
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    MEMORY[0x9DF71C][38] = offHandIndex;
    CG_MenuShowNotify(localClientNum, 4);
}

