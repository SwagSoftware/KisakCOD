#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_draw.h"
#include <stringed/stringed_hooks.h>
#include "cg_main.h"
#include "cg_servercmds.h"
#include "cg_newdraw.h"
#include <gfx_d3d/r_cinematic.h>
#include <ui/ui.h>
#include "cg_view.h"
#include <gfx_d3d/r_reflection_probe.h>

CenterPrint s_centerPrint[1];
ScreenBlur s_screenBlur[1];
ScreenFade s_screenFade[1];

void __cdecl TRACK_cg_draw()
{
    track_static_alloc_internal(s_centerPrint, 1028, "s_centerPrint", 9);
    track_static_alloc_internal(s_screenBlur, 28, "s_screenBlur", 9);
    track_static_alloc_internal(s_screenFade, 16, "s_screenFade", 9);
}

void __cdecl CG_CenterPrint(int localClientNum, const char *str)
{
    CenterPrint *v3; // r30
    const char *v4; // r3

    v3 = &s_centerPrint[localClientNum];
    v4 = SEH_LocalizeTextMessage(str, "Center Print", LOCMSG_SAFE);
    I_strncpyz(s_centerPrint[localClientNum].text, v4, 1024);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v3->time = cgArray[0].time;
}

void __cdecl CG_DrawCenterString(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    float *textStyle)
{
    CenterPrint *v11; // r30
    int time; // r4
    int v13; // r5
    float *v14; // r3
    __int64 v15; // r11
    long double v16; // fp2
    long double v17; // fp2
    __int64 v18; // r11
    double v19; // fp8
    double v20; // fp7
    double v21; // fp6
    double v22; // fp5
    double v23; // fp4
    float v24; // [sp+8h] [-B8h]
    float v25; // [sp+10h] [-B0h]
    float v26; // [sp+18h] [-A8h]
    float v27; // [sp+20h] [-A0h]
    float v28; // [sp+28h] [-98h]
    float v29; // [sp+30h] [-90h]
    float v30; // [sp+38h] [-88h]
    float v31; // [sp+40h] [-80h]
    float v32; // [sp+48h] [-78h]
    float v33; // [sp+50h] [-70h]
    float v34; // [sp+58h] [-68h]
    float v35; // [sp+60h] [-60h]
    float v36; // [sp+68h] [-58h]
    __int64 v37; // [sp+70h] [-50h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v11 = &s_centerPrint[localClientNum];
    time = v11->time;
    if (v11->time && !cg_paused->current.integer)
    {
        v13 = (int)(float)(cg_centertime->current.value * (float)1000.0);
        HIDWORD(v37) = v13;
        v14 = CG_FadeColor(cgArray[0].time, time, v13, 100);
        if (v14)
        {
            *textStyle = *textStyle * *v14;
            textStyle[1] = v14[1] * textStyle[1];
            textStyle[2] = v14[2] * textStyle[2];
            textStyle[3] = v14[3] * textStyle[3];
            LODWORD(v15) = UI_TextWidth(s_centerPrint[localClientNum].text, 0, font, fontscale);
            v37 = v15;
            *(double *)&v16 = (float)((float)((float)v15 * (float)0.5) + (float)0.5);
            v17 = floor(v16);
            LODWORD(v18) = (int)(float)*(double *)&v17;
            HIDWORD(v18) = rect->vertAlign;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                s_centerPrint[localClientNum].text,
                0x7FFFFFFF,
                font,
                (float)(rect->x - (float)v18), // x
                rect->y, // y
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                colorLtCyan, //KISAKTODO: args not correct
                0 // style
            );
        }
        else
        {
            v11->time = 0;
        }
    }
}

int __cdecl CG_DrawFriendlyFire(const cg_s *cgameGlob)
{
    const playerState_s *p_predictedPlayerState; // r31
    double fWeaponPosFrac; // fp30
    int weapFlags; // r11
    unsigned int ViewmodelWeaponIndex; // r3
    double v7; // fp1
    double v8; // fp2
    Material *v9; // r4
    const float *v10; // r3
    float v11; // [sp+60h] [-40h] BYREF
    float v12; // [sp+64h] [-3Ch] BYREF
    float v13; // [sp+68h] [-38h] BYREF
    float v14[3]; // [sp+6Ch] [-34h] BYREF

    p_predictedPlayerState = &cgameGlob->predictedPlayerState;
    fWeaponPosFrac = cgameGlob->predictedPlayerState.fWeaponPosFrac;
    if ((cgameGlob->predictedPlayerState.eFlags & 0x20000) != 0)
        return 0;
    weapFlags = cgameGlob->predictedPlayerState.weapFlags;
    if ((weapFlags & 8) == 0)
        return 0;
    if ((weapFlags & 0x200) != 0)
        return 0;
    if (cg_paused->current.integer)
        return 0;
    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    if (BG_GetWeaponDef(ViewmodelWeaponIndex)->overlayMaterial)
    {
        if (p_predictedPlayerState->fWeaponPosFrac > 0.0)
            return 0;
    }
    CG_CalcCrosshairPosition(cgameGlob, &v11, &v12);
    v11 = v11 * (float)fWeaponPosFrac;
    v12 = v12 * (float)fWeaponPosFrac;
    v14[0] = 40.0;
    v13 = 40.0;
    ScrPlace_ApplyRect(&scrPlaceFull, &v11, &v12, &v13, v14, 2, 2);
    v7 = (float)-(float)((float)(v13 * (float)0.5) - v11);
    v8 = (float)-(float)((float)(v14[0] * (float)0.5) - v12);
    v11 = -(float)((float)(v13 * (float)0.5) - v11);
    v12 = -(float)((float)(v14[0] * (float)0.5) - v12);
    CL_DrawStretchPicPhysical(v7, v8, v13, v14[0], 0.0, 0.0, 1.0, 1.0, v10, v9);
    return 1;
}

// local variable allocation has failed, the output may be wrong!
static int lastTime;
void __cdecl CG_DrawFlashFade(int localClientNum)
{
    ScreenFade *v2; // r31
    double alpha; // fp0
    int v5; // r3
    __int64 v6; // r11 OVERLAPPED
    double alphaCurrent; // fp13
    double v8; // fp13
    double v9; // fp13
    double v10; // fp0
    __int64 v11; // r10
    const float *v12; // r3
    unsigned int v13; // [sp+50h] [-50h] BYREF
    unsigned int v14; // [sp+54h] [-4Ch] BYREF
    float v15[2]; // [sp+58h] [-48h] BYREF
    __int64 v16; // [sp+60h] [-40h]
    unsigned __int64 v17; // [sp+68h] [-38h]
    float v18; // [sp+70h] [-30h]
    float v19; // [sp+74h] [-2Ch]
    float v20; // [sp+78h] [-28h]
    float v21; // [sp+7Ch] [-24h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v2 = &s_screenFade[localClientNum];
    if (v2->startTime + v2->duration >= cgArray[0].time)
    {
        if (v2->alphaCurrent == v2->alpha)
            goto LABEL_12;
        v5 = Sys_Milliseconds();
        HIDWORD(v6) = v5 - lastTime;
        LODWORD(v6) = v5 - lastTime - 1;
        lastTime = v5;
        if ((unsigned int)v6 > 0x1F2)
            goto LABEL_12;
        LODWORD(v6) = v2->duration;
        alphaCurrent = v2->alphaCurrent;
        alpha = v2->alpha;
        if (alphaCurrent <= alpha)
        {
            v17 = *(__int64 *)((char *)&v6 + 4);
            v9 = (float)((float)((float)*(__int64 *)((char *)&v6 + 4) / (float)v6) + v2->alphaCurrent);
            v16 = v6;
            v2->alphaCurrent = v9;
            if (v9 <= alpha)
                goto LABEL_12;
        }
        else
        {
            v16 = *(__int64 *)((char *)&v6 + 4);
            v17 = v6;
            v8 = (float)((float)alphaCurrent - (float)((float)*(__int64 *)((char *)&v6 + 4) / (float)v6));
            v2->alphaCurrent = v8;
            if (v8 >= alpha)
                goto LABEL_12;
        }
    }
    else
    {
        alpha = v2->alpha;
    }
    v2->alphaCurrent = alpha;
LABEL_12:
    v10 = v2->alphaCurrent;
    if (v10 > 0.0)
    {
        v18 = 0.0;
        v19 = 0.0;
        v20 = 0.0;
        v21 = v10;
        CL_GetScreenDimensions(&v14, &v13, v15);
        LODWORD(v11) = v14;
        v17 = __PAIR64__(v14, v13);
        v16 = v11;
        UI_FillRectPhysical(0.0, 0.0, (float)v11, (float)__SPAIR64__(v14, v13), v12);
    }
}

// local variable allocation has failed, the output may be wrong!
//int __cdecl CG_CheckPlayerMovement(
//    __int64 newCmd,
//    __int64 a2,
//    __int64 a3,
//    __int64 a4,
//    __int64 a5,
//    __int64 a6,
//    __int64 a7,
//    __int64 a8,
//    __int64 a9,
//    __int64 a10,
//    __int64 a11,
//    __int64 a12,
//    __int64 a13,
//    __int64 a14,
//    int a15,
//    int a16,
//    int a17,
//    __int16 a18)
//{
//    __int64 *v18; // r11
//    int v19; // r7
//    int v20; // r9
//    int result; // r3
//
//    v18 = &a7;
//    a11 = a3;
//    a14 = a4;
//    LODWORD(a4) = &a15;
//    LODWORD(a3) = (char *)&a8 + 4;
//    a7 = newCmd;
//    a8 = *(__int64 *)((char *)&a2 + 4);
//    a9 = a2;
//    a10 = *(__int64 *)((char *)&a3 + 4);
//    a12 = *(__int64 *)((char *)&a4 + 4);
//    do
//    {
//        v19 = *(unsigned __int8 *)a4;
//        v20 = *(unsigned __int8 *)v18 - v19;
//        if (*(unsigned __int8 *)v18 != v19)
//            break;
//        v18 = (__int64 *)((char *)v18 + 1);
//        LODWORD(a4) = a4 + 1;
//    } while (v18 != (__int64 *)((char *)&a8 + 4));
//    if (v20)
//        return 1;
//    if (HIBYTE(a18))
//        return 1;
//    result = 0;
//    if ((_BYTE)a18)
//        return 1;
//    return result;
//}

int __cdecl CG_CheckPlayerMovement(usercmd_s oldCmd, usercmd_s newCmd)
{
    if (memcmp(oldCmd.angles, newCmd.angles, 20))
        return 1;

    return newCmd.forwardmove || newCmd.rightmove;
}

int __cdecl CG_CheckPlayerStanceChange(int localClientNum, __int16 newButtons, __int16 changedButtons)
{
    if ((changedButtons & 0x1300) != 0)
    {
        CG_MenuShowNotify(localClientNum, 3);
        return 1;
    }
    else
    {
        if ((newButtons & 0x1300) != 0)
            CG_MenuShowNotify(localClientNum, 3);
        return 0;
    }
}

int __cdecl CG_CheckPlayerTryReload(int localClientNum, char buttons)
{
    int result; // r3

    if ((buttons & 0x30) == 0)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.pm_flags & 4) != 0)
        return 0;
    result = 1;
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) != 0)
        return 0;
    return result;
}

int __cdecl CG_CheckPlayerFireNonTurret(int localClientNum, char buttons)
{
    int result; // r3

    if ((buttons & 1) == 0)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    result = 1;
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) != 0)
        return 0;
    return result;
}

int __cdecl CG_CheckPlayerWeaponUsage(int localClientNum, char buttons)
{
    int result; // r3

    if (CG_CheckPlayerFireNonTurret(localClientNum, buttons)
        || (result = CG_CheckPlayerTryReload(localClientNum, buttons)) != 0)
    {
        CG_MenuShowNotify(localClientNum, 1);
        return 1;
    }
    return result;
}

int __cdecl CG_CheckPlayerOffHandUsage(int localClientNum, __int16 buttons)
{
    if ((buttons & 0xC000) == 0)
        return 0;
    CG_MenuShowNotify(localClientNum, 4);
    return 1;
}

unsigned int __cdecl CG_CheckPlayerMiscInput(int buttons)
{
    return buttons & 0xFFFFECFF;
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_CheckForPlayerInput(int localClientNum)
{
    int CurrentCmdNumber; // r3
    int v3; // r30
    int buttons; // r30
    int v5; // r28
    int v6; // r29
    _BYTE v7[48]; // [sp+58h] [-D8h] BYREF
    usercmd_s v8; // [sp+90h] [-A0h] BYREF
    usercmd_s v9; // [sp+D0h] [-60h] BYREF

    CurrentCmdNumber = CL_GetCurrentCmdNumber(localClientNum);
    v3 = CurrentCmdNumber;
    if (CurrentCmdNumber > 1)
    {
        CL_GetUserCmd(localClientNum, CurrentCmdNumber - 1, &v8);
        CL_GetUserCmd(localClientNum, v3, &v9);
        buttons = v9.buttons;
        v5 = v9.buttons ^ v8.buttons;
        memcpy(v7, v9.angles, sizeof(v7));
        v6 = CG_CheckPlayerMovement(*(usercmd_s *)v8.angles[0], *(usercmd_s *)v8.angles[2]);
        if (CG_CheckPlayerWeaponUsage(localClientNum, buttons))
            v6 = 1;
        if ((v9.buttons & 0xC000) != 0)
        {
            CG_MenuShowNotify(localClientNum, 4);
            v6 = 1;
        }
        if ((v5 & 0x1300) != 0)
        {
            CG_MenuShowNotify(localClientNum, 3);
            CG_MenuShowNotify(localClientNum, 2);
        }
        else
        {
            if ((v9.buttons & 0x1300) != 0)
                CG_MenuShowNotify(localClientNum, 3);
            if (v6 || (v5 & 0xFFFFECFF) != 0)
                CG_MenuShowNotify(localClientNum, 2);
        }
    }
}

void __cdecl CG_CheckHudHealthDisplay(int localClientNum)
{
    __int64 v2; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (CG_CalcPlayerHealth(&cgArray[0].nextSnap->ps) >= (double)hud_health_startpulse_injured->current.value)
    {
        if (hud_fade_healthbar->current.value != 0.0 && cgArray[0].healthFadeTime)
        {
            HIDWORD(v2) = cgArray[0].time - cgArray[0].healthFadeTime;
            LODWORD(v2) = cgArray[0].time - cgArray[0].healthFadeTime;
            if ((float)v2 > (double)(float)(hud_fade_healthbar->current.value * (float)1000.0))
            {
                Menus_HideByName(&cgDC, "Health");
                cgArray[0].healthFadeTime = 0;
            }
        }
    }
    else
    {
        CG_MenuShowNotify(localClientNum, 0);
    }
}

void __cdecl CG_CheckHudAmmoDisplay(int localClientNum)
{
    __int64 v2; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (CG_CheckPlayerForLowAmmo(cgArray) || CG_CheckPlayerForLowClip(cgArray))
        CG_MenuShowNotify(localClientNum, 1);
    if (hud_fade_ammodisplay->current.value != 0.0 && cgArray[0].ammoFadeTime)
    {
        HIDWORD(v2) = cgArray[0].time - cgArray[0].ammoFadeTime;
        LODWORD(v2) = cgArray[0].time - cgArray[0].ammoFadeTime;
        if ((float)v2 > (double)(float)(hud_fade_ammodisplay->current.value * (float)1000.0))
        {
            Menus_HideByName(&cgDC, "weaponinfo");
            Menus_HideByName(&cgDC, "weaponinfo_lowdef");
            cgArray[0].ammoFadeTime = 0;
        }
    }
}

void __cdecl CG_CheckHudCompassDisplay(int localClientNum)
{
    const dvar_s *v1; // r11
    __int64 v2; // r9

    v1 = hud_fade_compass;
    if (hud_fade_compass->current.value != 0.0)
    {
        if (localClientNum)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
            v1 = hud_fade_compass;
        }
        if (cgArray[0].compassFadeTime)
        {
            HIDWORD(v2) = cgArray[0].time;
            LODWORD(v2) = cgArray[0].time - cgArray[0].compassFadeTime;
            if ((float)v2 > (double)(float)(v1->current.value * (float)1000.0))
            {
                cgArray[0].compassFadeTime = 0;
                Menus_HideByName(&cgDC, "Compass");
            }
        }
    }
}

void __cdecl CG_CheckHudStanceDisplay(int localClientNum)
{
    int eFlags; // r11
    __int64 v3; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    eFlags = cgArray[0].nextSnap->ps.eFlags;
    if ((eFlags & 8) != 0 && (eFlags & 0x100) != 0 || (eFlags & 4) != 0 && (eFlags & 0x200) != 0)
        CG_MenuShowNotify(localClientNum, 3);
    if (hud_fade_stance->current.value != 0.0 && cgArray[0].stanceFadeTime)
    {
        HIDWORD(v3) = cgArray[0].time - cgArray[0].stanceFadeTime;
        LODWORD(v3) = cgArray[0].time - cgArray[0].stanceFadeTime;
        if ((float)v3 > (double)(float)(hud_fade_stance->current.value * (float)1000.0))
        {
            Menus_HideByName(&cgDC, "stance");
            cgArray[0].stanceFadeTime = 0;
        }
    }
}

void __cdecl CG_CheckHudSprintDisplay(int localClientNum)
{
    playerState_s *p_ps; // r30
    __int64 v3; // r11
    int v4; // [sp+50h] [-30h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    p_ps = &cgArray[0].nextSnap->ps;
    if (cgArray[0].nextSnap->ps.pm_type != 5)
    {
        v4 = (int)(float)(BG_GetWeaponDef(cgArray[0].nextSnap->ps.weapon)->sprintDurationScale
            * (float)(player_sprintTime->current.value * (float)1000.0));
        if (PM_GetSprintLeft(p_ps, cgArray[0].time) < v4)
            CG_MenuShowNotify(localClientNum, 6);
    }
    if (p_ps->pm_type != 5
        && cgArray[0].predictedPlayerState.sprintState.lastSprintStart > cgArray[0].predictedPlayerState.sprintState.lastSprintEnd)
    {
        CG_MenuShowNotify(localClientNum, 6);
    }
    if (hud_fade_sprint->current.value != 0.0 && cgArray[0].sprintFadeTime)
    {
        HIDWORD(v3) = cgArray[0].time - cgArray[0].sprintFadeTime;
        LODWORD(v3) = cgArray[0].time - cgArray[0].sprintFadeTime;
        if ((float)v3 > (double)(float)(hud_fade_stance->current.value * (float)1000.0))
        {
            Menus_HideByName(&cgDC, "sprintMeter");
            cgArray[0].sprintFadeTime = 0;
        }
    }
}

void __cdecl CG_CheckHudOffHandDisplay(int localClientNum)
{
    const dvar_s *v1; // r11
    __int64 v2; // r9

    v1 = hud_fade_offhand;
    if (hud_fade_offhand->current.value != 0.0)
    {
        if (localClientNum)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                910,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
            v1 = hud_fade_offhand;
        }
        if (cgArray[0].offhandFadeTime)
        {
            HIDWORD(v2) = cgArray[0].time;
            LODWORD(v2) = cgArray[0].time - cgArray[0].offhandFadeTime;
            if ((float)v2 > (double)(float)(v1->current.value * (float)1000.0))
            {
                cgArray[0].offhandFadeTime = 0;
                Menus_HideByName(&cgDC, "offhandinfo");
            }
        }
    }
}

void __cdecl CG_CheckHudObjectiveDisplay(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].showScores)
    {
        Menus_ShowByName(&cgDC, "objectiveinfo");
    }
    else if (cgArray[0].time - cgArray[0].scoreFadeTime > 300)
    {
        Menus_HideByName(&cgDC, "objectiveinfo");
    }
}

void __cdecl CG_CheckTimedMenus(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].voiceTime && cgArray[0].time - cgArray[0].voiceTime > 2500)
    {
        Menus_CloseByName(&cgDC, "voiceMenu");
        cgArray[0].voiceTime = 0;
    }
    CG_CheckForPlayerInput(localClientNum);
    CG_CheckHudHealthDisplay(localClientNum);
    CG_CheckHudAmmoDisplay(localClientNum);
    CG_CheckHudCompassDisplay(localClientNum);
    CG_CheckHudStanceDisplay(localClientNum);
    CG_CheckHudOffHandDisplay(localClientNum);
    CG_CheckHudObjectiveDisplay(localClientNum);
    CG_CheckHudSprintDisplay(localClientNum);
}

void __cdecl CG_Blur(
    int localClientNum,
    int time,
    double endBlur,
    BlurTime timeType,
    BlurTime priority,
    BlurPriority a6)
{
    double blurRadius; // fp1
    ScreenBlur *v12; // r31
    int v13; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (time < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp", 592, 0, "%s", "time >= 0");
    if (endBlur < 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
            593,
            0,
            "%s\n\t(endBlur) = %g",
            HIDWORD(endBlur),
            LODWORD(endBlur));
    blurRadius = cgArray[0].refdef.blurRadius;
    if (cgArray[0].refdef.blurRadius < 0.0)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
            594,
            0,
            "%s\n\t(cgameGlob->refdef.blurRadius) = %g",
            "(cgameGlob->refdef.blurRadius >= 0)",
            blurRadius);
        blurRadius = cgArray[0].refdef.blurRadius;
    }
    v12 = &s_screenBlur[localClientNum];
    if (a6 >= v12->priority)
    {
        if (priority == BLUR_TIME_ABSOLUTE)
        {
            v13 = Sys_Milliseconds();
            blurRadius = cgArray[0].refdef.blurRadius;
        }
        else
        {
            v13 = cgArray[0].time;
        }
        v12->start = blurRadius;
        v12->end = endBlur;
        v12->timeStart = v13;
        v12->time = priority;
        v12->priority = a6;
        v12->timeEnd = v13 + time;
    }
}

void __cdecl CG_ClearBlur(int localClientNum)
{
    ScreenBlur *v1; // r11

    v1 = &s_screenBlur[localClientNum];
    v1->start = 0.0;
    v1->timeStart = 0;
    v1->end = 0.0;
    v1->timeEnd = 0;
    v1->radius = 0.0;
    v1->priority = BLUR_PRIORITY_NONE;
}

float __cdecl CG_GetBlurRadius(int localClientNum)
{
    double radius; // fp1

    radius = s_screenBlur[localClientNum].radius;
    return *((float *)&radius + 1);
}

void __cdecl CG_ScreenBlur(int localClientNum)
{
    ScreenBlur *v2; // r31
    int time; // r28
    int timeStart; // r11
    int timeEnd; // r11
    __int64 v6; // r11
    int v7; // r11
    int v8[4]; // r11
    double v9; // fp31
    double end; // fp0
    double v11; // fp31
    double _FP12; // fp12
    double _FP0; // fp0

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v2 = &s_screenBlur[localClientNum];
    if (v2->time == BLUR_TIME_ABSOLUTE)
        time = Sys_Milliseconds();
    else
        time = cgArray[0].time;
    timeStart = v2->timeStart;
    if (timeStart)
    {
        if (timeStart > time)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp", 649, 0, "%s", "scrBlur->timeStart <= time");
        if (v2->timeStart > v2->timeEnd)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
                650,
                0,
                "%s",
                "scrBlur->timeStart <= scrBlur->timeEnd");
        timeEnd = v2->timeEnd;
        if (time >= timeEnd)
        {
            end = v2->end;
            v11 = end;
            v2->timeStart = 0;
            v2->timeEnd = 0;
            v2->priority = BLUR_PRIORITY_NONE;
        }
        else
        {
            HIDWORD(v6) = v2->timeStart;
            LODWORD(v6) = timeEnd - HIDWORD(v6);
            if ((float)v6 <= 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
                    654,
                    0,
                    "%s",
                    "scrBlur->timeEnd - scrBlur->timeStart > 0.0f");
            v7 = v2->timeStart;
            v8[1] = time - v7;
            v8[2] = time - v7;
            v8[3] = v2->timeEnd - v7;
            v9 = (float)((float)*(__int64 *)&v8[1] / (float)*(__int64 *)&v8[2]);
            if (v9 < 0.0 || v9 >= 1.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
                    656,
                    0,
                    "%s\n\t(t) = %g",
                    HIDWORD(v9),
                    LODWORD(v9));
            end = v2->end;
            v11 = (float)((float)((float)((float)1.0 - (float)v9) * v2->start) + (float)(v2->end * (float)v9));
        }

        // fsel f0, f1, f2, f3  --------  f0 = ( f1 >= 0 ? f2 : f3 )

        //_FP12 = (float)((float)end - v2->start);
        //__asm { fsel      f12, f12, f13, f0 }
        //
        //if (v11 < _FP12)
        //    goto LABEL_21;
        //
        //_FP12 = (float)(v2->start - (float)end);
        //__asm { fsel      f0, f12, f13, f0 }
        //
        //if (v11 > _FP0)
        //    LABEL_21:

        float _FP12 = ((float)end - v2->start);
        _FP12 = (_FP12 >= 0.0f) ? v2->start : end;

        //if (v11 < _FP12)
        //    goto LABEL_21;

        float _FP0 = (v2->start - (float)end);
        _FP0 = (_FP0 >= 0.0f) ? v2->start : end;

        //if (v11 > _FP0)
        //    LABEL_21:


        //MyAssertHandler(
        //    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
        //    668,
        //    0,
        //    "%s\n\t(blur) = %g",
        //    HIDWORD(v11),
        //    LODWORD(v11));
        v2->radius = v11;
    }
}

void __cdecl CG_Fade(int localClientNum, int r, int g, int b, int a, int startTime, int duration)
{
    ScreenFade *v6; // r31

    v6 = &s_screenFade[localClientNum];
    v6->startTime = startTime;
    v6->duration = duration;
    v6->alpha = (float)a * 0.0039215689f;

    iassert(localClientNum == 0);
    //if (localClientNum)
    //    MyAssertHandler(
    //        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
    //        910,
    //        0,
    //        "%s\n\t(localClientNum) = %i",
    //        "(localClientNum == 0)",
    //        localClientNum);
    if (v6->duration + v6->startTime <= cgArray[0].time)
        v6->alphaCurrent = v6->alpha;
}

void CG_DrawMiniConsole()
{
    if (cg_minicon->current.enabled)
        Con_DrawMiniConsole(0, 2, 4, 1.0);
}

void CG_DrawErrorMessages()
{
    Con_DrawErrors(0, 2, 300, 1.0);
}

void __cdecl CG_DrawFadeInCinematic(int localClientNum)
{
    char v2; // r31

    v2 = 0;
    if (R_Cinematic_IsNextReady())
    {
        R_Cinematic_StartNextPlayback();
        v2 = 1;
    }
    if (R_Cinematic_IsFinished())
        R_Cinematic_StopPlayback();
    if (R_Cinematic_IsStarted() || R_Cinematic_IsPending())
        v2 = 1;
    if (v2)
    {
        if (cg_cinematicFullscreen->current.enabled)
            R_Cinematic_DrawStretchPic_Letterboxed();
    }
}

void __cdecl CG_DrawFriendOverlay(int localClientNum)
{
    cg_s *LocalClientGlobals; // r30
    const char *ConfigString; // r3
    const char *v4; // r31
    const char *friendName; // r25
    int v6; // r4
    const dvar_s *v7; // r11
    const dvar_s *v8; // r10
    const ScreenPlacement *place; // r31
    Font_s *FontHandle; // r27
    int v11; // r7
    double v12; // fp8
    double v13; // fp7
    double v14; // fp6
    double v15; // fp5
    double v16; // fp4
    const char *v17; // r3
    const char *v18; // r28
    int v70; // [sp+7Ch] [-74h]
    int integer; // [sp+80h] [-70h] BYREF
    float v72; // [sp+84h] [-6Ch]
    float v73; // [sp+88h] [-68h]
    float v74; // [sp+8Ch] [-64h]
    Material v75; // [sp+90h] [-60h] BYREF

    if (!cg_paused->current.integer && !CG_Flashbanged(localClientNum))
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        ConfigString = CL_GetConfigString(localClientNum, 9u);
        v4 = ConfigString;
        if (ConfigString)
        {
            if (*ConfigString && I_stricmp(ConfigString, "none") && !hud_missionFailed->current.enabled)
            {
                friendName = SEH_LocalizeTextMessage(v4, "Friend Name", LOCMSG_SAFE);
                v6 = !friendlyNameFontObjective->current.enabled ? 0 : 6;
                if ((LocalClientGlobals->predictedPlayerState.weapFlags & 0x10) != 0)
                {
                    v7 = hostileNameFontColor;
                    v8 = hostileNameFontGlowColor;
                }
                else
                {
                    v7 = friendlyNameFontColor;
                    v8 = friendlyNameFontGlowColor;
                }
                integer = v7->current.integer;
                v72 = v7->current.vector[1];
                v73 = v7->current.vector[2];
                v74 = v7->current.vector[3];
                *(float *)&v75.info.name = v8->current.value;
                *(float *)&v75.info.gameFlags = v8->current.vector[1];
                *(float *)&v75.info.drawSurf.fields = v8->current.vector[2];
                *((float *)&v75.info.drawSurf.packed + 1) = v8->current.vector[3];
                place = &scrPlaceView[localClientNum];
                FontHandle = UI_GetFontHandle(place, v6, friendlyNameFontSize->current.value);
                UI_DrawTextWithGlow(
                    place,
                    friendName,
                    0x7FFFFFFF,
                    FontHandle,
                    25.0, // x
                    -2.0, // y
                    2, // KISAKTODO: args bad
                    2,
                    friendlyNameFontSize->current.value,
                    (const float *)2,
                    2,
                    colorBlue,
                    false,
                    false);
                v17 = CL_GetConfigString(localClientNum, 0xAu);
                v18 = v17;
                if (v17 && *v17)
                {
                    if (I_stricmp(v17, "none"))
                    {
                        //LOBYTE(v70) = 0;
                        //LOBYTE(v66) = 0;
                        //v72 = 1.0;
                        //v73 = 1.0;
                        //v74 = 0.69999999;
                        UI_DrawTextWithGlow(
                            place,
                            UI_SafeTranslateString(v18),
                            0x7FFFFFFF,
                            FontHandle,
                            25.0, // x
                            20.0, // y
                            2,
                            2,
                            friendlyNameFontSize->current.value,
                            colorLtCyan,
                            0,
                            colorBlue,
                            false,
                            false); // KISAKTODO: args bad
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawPaused(int localClientNum)
{
    if (cg_paused->current.integer)
    {
        if (cg_drawpaused->current.enabled)
            CG_HudMenuShowAllTimed(localClientNum);
    }
}

void __cdecl CG_AlterTimescale(int localClientNum, int time, double startScale, double endScale)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].timeScaleTimeStart = Sys_Milliseconds();
    cgArray[0].timeScaleTimeEnd = cgArray[0].timeScaleTimeStart + time;
    cgArray[0].timeScaleStart = startScale;
    cgArray[0].timeScaleEnd = endScale;
}

void __cdecl CG_UpdateTimeScale(cg_s *cgameGlob)
{
    int v2; // r3
    int v3; // r29
    int *p_timeScaleTimeStart; // r28
    __int64 v5; // r11
    int *p_timeScaleTimeEnd; // r30
    int v7[4]; // r11
    double timeScaleEnd; // fp1

    v2 = Sys_Milliseconds();
    v3 = v2;
    p_timeScaleTimeStart = &cgameGlob->timeScaleTimeStart;
    HIDWORD(v5) = cgameGlob->timeScaleTimeStart;
    if (HIDWORD(v5) && SHIDWORD(v5) < v2)
    {
        p_timeScaleTimeEnd = &cgameGlob->timeScaleTimeEnd;
        LODWORD(v5) = cgameGlob->timeScaleTimeEnd;
        if (v2 >= (int)v5)
        {
            *p_timeScaleTimeStart = 0;
            timeScaleEnd = cgameGlob->timeScaleEnd;
            *p_timeScaleTimeEnd = 0;
            Com_SetTimeScale(timeScaleEnd);
        }
        else
        {
            LODWORD(v5) = v5 - HIDWORD(v5);
            if ((float)v5 <= 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_draw.cpp",
                    828,
                    0,
                    "%s",
                    "cgameGlob->timeScaleTimeEnd - cgameGlob->timeScaleTimeStart > 0.0f");
            v7[1] = *p_timeScaleTimeEnd;
            v7[2] = v3 - *p_timeScaleTimeStart;
            v7[3] = *p_timeScaleTimeEnd - *p_timeScaleTimeStart;
            Com_SetTimeScale((float)((float)((float)((float)1.0
                - (float)((float)*(__int64 *)&v7[1] / (float)*(__int64 *)&v7[2]))
                * cgameGlob->timeScaleStart)
                + (float)(cgameGlob->timeScaleEnd
                    * (float)((float)*(__int64 *)&v7[1] / (float)*(__int64 *)&v7[2]))));
        }
    }
}

const char strButtons[17] =
{
  '\x01',
  '\x02',
  '\x03',
  '\x04',
  '\x05',
  '\x06',
  '\x0E',
  '\x0F',
  '\x10',
  '\x11',
  '\x12',
  '\x13',
  '\x14',
  '\x15',
  '\x16',
  '\x17',
  '\0'
};

void DrawFontTest(int localClientNum)
{
    const ScreenPlacement *v1; // r31
    Font_s *FontHandle; // r25

    char txt[512]; // [sp+70h] [-250h] BYREF

    const float MY_X = -25.0f;

    v1 = &scrPlaceView[localClientNum];
    FontHandle = UI_GetFontHandle(v1, 1, 0.40000001);
    Com_sprintf(
        txt,
        512,
        "%s: %s All those moments will be lost in time, like tears in rain.",
        FontHandle->fontName,
        strButtons);

    int horzAlign = 0;
    int vertAlign = 0;

    UI_FilterStringForButtonAnimation(txt, 0x200u);
    UI_DrawText(v1, txt, 0x7FFFFFFF, FontHandle, MY_X, 10.0f, horzAlign, vertAlign, 0.4f, colorWhite, 1);
    Font_s *v5 = UI_GetFontHandle(v1, 2, 0.4f);
    Com_sprintf(txt, 512, "%s: %s All those moments will be lost in time, like tears in rain.", v5->fontName, strButtons);
    UI_FilterStringForButtonAnimation(txt, 0x200u);
    UI_DrawText(v1, txt, 0x7FFFFFFF, v5, MY_X, 35.0f, horzAlign, vertAlign, 0.4f, colorWhite, 1);
    Font_s *v8 = UI_GetFontHandle(v1, 3, 0.4f);
    Com_sprintf(txt, 512, "%s: %s All those moments will be lost in time, like tears in rain.", v8->fontName, strButtons);
    UI_FilterStringForButtonAnimation(txt, 0x200u);
    UI_DrawText(v1, txt, 0x7FFFFFFF, v8, MY_X, 60.0f, horzAlign, vertAlign, 0.4f, colorWhite, 1);
    Font_s *v11 = UI_GetFontHandle(v1, 5, 0.40000001);
    Com_sprintf(txt, 512, "%s: %s All those moments will be lost in time, like tears in rain.", v11->fontName, strButtons);
    UI_FilterStringForButtonAnimation(txt, 0x200u);
    UI_DrawText(v1, txt, 0x7FFFFFFF, v11, MY_X, 85.0f, horzAlign, vertAlign, 0.4f, colorWhite, 1);
    Font_s *v14 = UI_GetFontHandle(v1, 6, 0.4f);
    Com_sprintf(txt, 512, "%s: %s All those moments will be lost in time, like tears in rain.", v14->fontName, strButtons);
    UI_FilterStringForButtonAnimation(txt, 0x200u);
    UI_DrawText(v1, txt, 0x7FFFFFFF, v14, MY_X, 110.0f, horzAlign, vertAlign, 0.4f, colorWhite, 1);
}

static const char *WeaponStateNames_8[27] =
{
  "WEAPON_READY",
  "WEAPON_RAISING",
  "WEAPON_RAISING_ALTSWITCH",
  "WEAPON_DROPPING",
  "WEAPON_DROPPING_QUICK",
  "WEAPON_FIRING",
  "WEAPON_RECHAMBERING",
  "WEAPON_RELOADING",
  "WEAPON_RELOADING_INTERUPT",
  "WEAPON_RELOAD_START",
  "WEAPON_RELOAD_START_INTERUPT",
  "WEAPON_RELOAD_END",
  "WEAPON_MELEE_INIT",
  "WEAPON_MELEE_FIRE",
  "WEAPON_MELEE_END",
  "WEAPON_OFFHAND_INIT",
  "WEAPON_OFFHAND_PREPARE",
  "WEAPON_OFFHAND_HOLD",
  "WEAPON_OFFHAND_START",
  "WEAPON_OFFHAND",
  "WEAPON_OFFHAND_END",
  "WEAPON_DETONATING",
  "WEAPON_SPRINT_RAISE",
  "WEAPON_SPRINT_LOOP",
  "WEAPON_SPRINT_DROP",
  "WEAPON_NIGHTVISION_WEAR",
  "WEAPON_NIGHTVISION_REMOVE"
};


void DrawViewmodelInfo(int localClientNum)
{
    int ViewmodelWeaponIndex; // r31
    Font_s *FontHandle; // r24
    weaponInfo_s *LocalClientWeaponInfo; // r27
    WeaponDef *WeaponDef; // r28
    XModel *v6; // r11
    const char *name; // r31
    double ViewFov; // fp1
    const char *v9; // r8
    const char **p_name; // r11
    const char *v11; // r10
    const char **v12; // r11
    const char *v13; // r9
    char *v14; // r11
    int v16; // r7
    const char *v17; // [sp+8h] [-8B8h]
    const char *v18; // [sp+Ch] [-8B4h]
    char v19[2128]; // [sp+70h] [-850h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgArray[0].predictedPlayerState);
    FontHandle = UI_GetFontHandle(&scrPlaceView[localClientNum], 6, 0.25);
    if (ViewmodelWeaponIndex > 0)
    {
        LocalClientWeaponInfo = CG_GetLocalClientWeaponInfo(localClientNum, ViewmodelWeaponIndex);
        WeaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
        v6 = WeaponDef->gunXModel[cgArray[0].predictedPlayerState.weaponmodels[ViewmodelWeaponIndex]];
        if (v6)
            name = v6->name;
        else
            name = 0;
        ViewFov = CG_GetViewFov(localClientNum);
        v9 = "none";
        p_name = &LocalClientWeaponInfo->gogglesModel->name;
        if (p_name)
            v11 = *p_name;
        else
            v11 = "none";
        v12 = &LocalClientWeaponInfo->handModel->name;
        if (v12)
            v13 = *v12;
        else
            v13 = "none";
        if (name)
            v9 = name;
        Com_sprintf(
            v19,
            2048,
            "^6%s\n"
            "^7Weapon: ^2%s^7 - ^5%s\n"
            "^7Hands: ^5%s\n"
            "^7Goggles: ^5%s\n"
            "^7Rocket: ^5%s\n"
            "^7Knife: ^5%s\n"
            "^7ADS: ^5%.2f ^7-^5 %.0f^7fov\n"
            "^7---Anims---\n"
            "^3",
            WeaponStateNames_8[cgArray[0].predictedPlayerState.weaponstate],
            WeaponDef->szInternalName,
            v9,
            v13,
            v11,
            v17,
            v18,
            cgArray[0].predictedPlayerState.fWeaponPosFrac,
            ViewFov);
        v14 = v19;
        while (*v14++)
            ;
        DObjDisplayAnimToBuffer(
            LocalClientWeaponInfo->viewModelDObj,
            "",
            &v19[v14 - v19 - 1],
            2048 - (v14 - v19 - 1));
        UI_DrawText(&scrPlaceView[localClientNum], v19, 2048, FontHandle, 0.0, 20.0, v16, 3, 0.25, (const float *)1, 1);
    }
}

void __cdecl CG_Draw2D(int localClientNum)
{
    snapshot_s *nextSnap; // r29
    int integer; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    CG_UpdateTimeScale(cgArray);
    if (cgArray[0].predictedPlayerState.pm_type != 4 && cgArray[0].cubemapShot == CUBEMAPSHOT_NONE)
    {
        nextSnap = cgArray[0].nextSnap;
        if (cg_draw2D->current.enabled)
        {
            integer = debugOverlay->current.integer;
            if (integer == 1)
            {
                DrawViewmodelInfo(localClientNum);
            }
            else if (integer == 2)
            {
                DrawFontTest(localClientNum);
                return;
            }
            CG_DrawNightVisionOverlay(localClientNum);
            CG_DrawFadeInCinematic(localClientNum);
            CG_DrawFriendOverlay(localClientNum);
            CG_ScreenBlur(localClientNum);
            CG_DrawFlashDamage(cgArray);
            if (cg_drawHUD->current.enabled && hud_drawHUD->current.enabled)
            {
                CG_DrawDamageDirectionIndicators(localClientNum);
                if (nextSnap->ps.pm_type < 5)
                {
                    if (!cg_drawFriendlyFireCrosshair->current.enabled || !(unsigned __int8)CG_DrawFriendlyFire(cgArray))
                        CG_DrawCrosshair(localClientNum);
                    CG_DrawGrenadeIndicators(localClientNum);
                    CG_CheckTimedMenus(localClientNum);
                }
                CG_Draw2dHudElems(localClientNum, 0);
            }
            Menu_PaintAll(&cgDC);
            CG_Draw2dHudElems(localClientNum, 1);
            CG_DrawPerformanceWarnings();
            //Profile_Begin(349);
            CG_DrawDebugOverlays(localClientNum);
            //Profile_EndInternal(0);
            CG_DrawUpperRightDebugInfo(localClientNum);
            if (!cgArray[0].showScores && cg_minicon->current.enabled)
                Con_DrawMiniConsole(0, 2, 4, 1.0);
            Con_DrawErrors(0, 2, 300, 1.0);
            CG_DrawFlashFade(localClientNum);
            if (cg_paused->current.integer && cg_drawpaused->current.enabled)
                CG_HudMenuShowAllTimed(localClientNum);
        }
        else
        {
            if (cg_drawHUD->current.enabled && hud_drawHUD->current.enabled)
                CG_Draw2dHudElems(localClientNum, 0);
            CG_DrawFlashFade(localClientNum);
        }
    }
}

void __cdecl CG_DrawActive(int localClientNum)
{
    double zoomSensitivity; // fp1

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    zoomSensitivity = cgArray[0].zoomSensitivity;
    if (cgArray[0].shellshock.sensitivity != 0.0)
        zoomSensitivity = (float)(cgArray[0].shellshock.sensitivity * cgArray[0].zoomSensitivity);
    CL_SetFOVSensitivityScale(localClientNum, zoomSensitivity);
    CL_SetUserCmdWeapons(localClientNum, cgArray[0].weaponSelect, cgArray[0].equippedOffHand);
    CL_SetUserCmdAimValues(
        localClientNum,
        cgArray[0].gunPitch,
        cgArray[0].gunYaw,
        cgArray[0].gunXOfs,
        cgArray[0].gunYOfs,
        cgArray[0].gunZOfs);
    CL_SetExtraButtons(localClientNum, cgArray[0].extraButtons);
    cgArray[0].extraButtons = 0;
    CL_RenderScene(&cgArray[0].refdef);
}

// attributes: thunk
void __cdecl CG_AddSceneTracerBeams(int localClientNum)
{
    CG_AddLocalEntityTracerBeams(localClientNum);
}

// attributes: thunk
void __cdecl CG_GenerateSceneVerts(int localClientNum)
{
    CG_AddDrawSurfsFor3dHudElems(localClientNum);
}

