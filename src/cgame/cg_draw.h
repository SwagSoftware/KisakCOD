#pragma once
#include "cg_local.h"


#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

enum BlurTime : __int32
{
    BLUR_TIME_RELATIVE = 0x0,
    BLUR_TIME_ABSOLUTE = 0x1,
};

enum BlurPriority : __int32
{
    BLUR_PRIORITY_NONE = 0x0,
    BLUR_PRIORITY_SCRIPT = 0x1,
    BLUR_PRIORITY_CODE = 0x2,
};

weaponInfo_s *__fastcall CG_GetLocalClientWeaponInfo(int localClientNum, int weaponIndex);
void __fastcall TRACK_cg_draw();
void __fastcall CG_CenterPrint(int localClientNum, const char *str);
void __fastcall CG_DrawCenterString(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    float *textStyle);
int __fastcall CG_DrawFriendlyFire(const cg_s *cgameGlob);
void __fastcall CG_DrawFlashFade(int localClientNum);
int __fastcall CG_CheckPlayerMovement(
    __int64 newCmd,
    __int64 a2,
    __int64 a3,
    __int64 a4,
    __int64 a5,
    __int64 a6,
    __int64 a7,
    __int64 a8,
    __int64 a9,
    __int64 a10,
    __int64 a11,
    __int64 a12,
    __int64 a13,
    __int64 a14,
    int a15,
    int a16,
    int a17,
    __int16 a18);
int __fastcall CG_CheckPlayerStanceChange(int localClientNum, __int16 newButtons, __int16 changedButtons);
int __fastcall CG_CheckPlayerTryReload(int localClientNum, char buttons);
int __fastcall CG_CheckPlayerFireNonTurret(int localClientNum, char buttons);
int __fastcall CG_CheckPlayerWeaponUsage(int localClientNum, char buttons);
int __fastcall CG_CheckPlayerOffHandUsage(int localClientNum, __int16 buttons);
unsigned int __fastcall CG_CheckPlayerMiscInput(int buttons);
void __fastcall CG_CheckForPlayerInput(int localClientNum);
void __fastcall CG_CheckHudHealthDisplay(int localClientNum);
void __fastcall CG_CheckHudAmmoDisplay(int localClientNum);
void __fastcall CG_CheckHudCompassDisplay(int localClientNum);
void __fastcall CG_CheckHudStanceDisplay(int localClientNum);
void __fastcall CG_CheckHudSprintDisplay(int localClientNum);
void __fastcall CG_CheckHudOffHandDisplay(int localClientNum);
void __fastcall CG_CheckHudObjectiveDisplay(int localClientNum);
void __fastcall CG_CheckTimedMenus(int localClientNum);
void __fastcall CG_Blur(
    int localClientNum,
    int time,
    double endBlur,
    BlurTime timeType,
    BlurTime priority,
    BlurPriority a6);
void __fastcall CG_ClearBlur(int localClientNum);
float __fastcall CG_GetBlurRadius(int localClientNum);
void __fastcall CG_ScreenBlur(int localClientNum);
void __fastcall CG_Fade(int localClientNum, int r, __int64 b, int a, int startTime, int duration);
void CG_DrawMiniConsole();
void CG_DrawErrorMessages();
void __fastcall CG_DrawFadeInCinematic(int localClientNum);
void __fastcall CG_DrawFriendOverlay(int localClientNum);
void __fastcall CG_DrawPaused(int localClientNum);
void __fastcall CG_AlterTimescale(int localClientNum, int time, double startScale, double endScale);
void __fastcall CG_UpdateTimeScale(cg_s *cgameGlob);
void __fastcall DrawFontTest(int localClientNum);
void __fastcall DrawViewmodelInfo(int localClientNum);
void __fastcall CG_Draw2D(int localClientNum);
void __fastcall CG_DrawActive(int localClientNum);
void __fastcall CG_AddSceneTracerBeams(int localClientNum);
void __fastcall CG_GenerateSceneVerts(int localClientNum);
