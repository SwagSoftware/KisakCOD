#pragma once
#include "cg_local.h"


#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

struct CenterPrint
{
    int time;
    char text[1024];
};

struct ScreenBlur
{
    BlurPriority priority;
    BlurTime time;
    int timeStart;
    int timeEnd;
    float start;
    float end;
    float radius;
};

struct ScreenFade
{
    float alpha;
    float alphaCurrent;
    int startTime;
    int duration;
};

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

weaponInfo_s *__cdecl CG_GetLocalClientWeaponInfo(int localClientNum, int weaponIndex);
void __cdecl TRACK_cg_draw();
void __cdecl CG_CenterPrint(int localClientNum, const char *str);
void __cdecl CG_DrawCenterString(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    float *textStyle);
int __cdecl CG_DrawFriendlyFire(const cg_s *cgameGlob);
void __cdecl CG_DrawFlashFade(int localClientNum);
int __cdecl CG_CheckPlayerMovement(
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
int __cdecl CG_CheckPlayerStanceChange(int localClientNum, __int16 newButtons, __int16 changedButtons);
int __cdecl CG_CheckPlayerTryReload(int localClientNum, char buttons);
int __cdecl CG_CheckPlayerFireNonTurret(int localClientNum, char buttons);
int __cdecl CG_CheckPlayerWeaponUsage(int localClientNum, char buttons);
int __cdecl CG_CheckPlayerOffHandUsage(int localClientNum, __int16 buttons);
unsigned int __cdecl CG_CheckPlayerMiscInput(int buttons);
void __cdecl CG_CheckForPlayerInput(int localClientNum);
void __cdecl CG_CheckHudHealthDisplay(int localClientNum);
void __cdecl CG_CheckHudAmmoDisplay(int localClientNum);
void __cdecl CG_CheckHudCompassDisplay(int localClientNum);
void __cdecl CG_CheckHudStanceDisplay(int localClientNum);
void __cdecl CG_CheckHudSprintDisplay(int localClientNum);
void __cdecl CG_CheckHudOffHandDisplay(int localClientNum);
void __cdecl CG_CheckHudObjectiveDisplay(int localClientNum);
void __cdecl CG_CheckTimedMenus(int localClientNum);
void __cdecl CG_Blur(
    int localClientNum,
    int time,
    double endBlur,
    BlurTime timeType,
    BlurTime priority,
    BlurPriority a6);
void __cdecl CG_ClearBlur(int localClientNum);
float __cdecl CG_GetBlurRadius(int localClientNum);
void __cdecl CG_ScreenBlur(int localClientNum);
void __cdecl CG_Fade(int localClientNum, int r, int g, int b, int a, int startTime, int duration);
void CG_DrawMiniConsole();
void CG_DrawErrorMessages();
void __cdecl CG_DrawFadeInCinematic(int localClientNum);
void __cdecl CG_DrawFriendOverlay(int localClientNum);
void __cdecl CG_DrawPaused(int localClientNum);
void __cdecl CG_AlterTimescale(int localClientNum, int time, double startScale, double endScale);
void __cdecl CG_UpdateTimeScale(cg_s *cgameGlob);
void __cdecl DrawFontTest(int localClientNum);
void __cdecl DrawViewmodelInfo(int localClientNum);
void __cdecl CG_Draw2D(int localClientNum);
void __cdecl CG_DrawActive(int localClientNum);
void __cdecl CG_AddSceneTracerBeams(int localClientNum);
void __cdecl CG_GenerateSceneVerts(int localClientNum);
