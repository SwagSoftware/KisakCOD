#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

void __cdecl CG_AntiBurnInHUD_RegisterDvars(int a1, unsigned __int16 a2, const char *a3);
unsigned int __cdecl CG_GetSelectedWeaponIndex(const cg_s *cgameGlob);
int __cdecl CG_IsHudHidden();
int __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, unsigned int weapIndex);
int __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob);
// local variable allocation has failed, the output may be wrong!
int __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, unsigned int weapIndex);
int __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob);
void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, DvarValue *color);
void __cdecl CG_DrawStanceIcon(
    int localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    double x,
    double y,
    double fadeAlpha,
    const float *a7);
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
    Font_s *a9);
float __cdecl CG_CalcPlayerHealth(const playerState_s *ps);
float __cdecl CG_FadeLowHealthOverlay(const cg_s *cgameGlob);
void __cdecl CG_PulseLowHealthOverlay(cg_s *cgameGlob, double healthRatio);
void __cdecl CG_DrawPlayerLowHealthOverlay(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color);
int __cdecl CG_ServerMaterialName(int localClientNum, int index, char *materialName, unsigned int maxLen);
Material *__cdecl CG_ObjectiveIcon(int icon, unsigned int type);
void __cdecl CG_UpdateCursorHints(int localClientNum);
char *__cdecl CG_GetWeaponUseString(int localClientNum, const char **secondaryString);
char *__cdecl CG_GetUseString(int localClientNum);
void __cdecl CG_DrawCursorhint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    int textStyle);
void __cdecl CG_DrawHoldBreathHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    int textStyle);
void __cdecl CG_DrawMantleHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    int textStyle);
void __cdecl CG_DrawSaving(int localClientNum, const rectDef_s *rect, float *color, Material *material);
int __cdecl CG_OwnerDrawVisible(int flags);
void __cdecl CG_DrawTankBody(int localClientNum, rectDef_s *rect, Material *material, float *color);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawDeadQuote(
    const cg_s *cgameGlob,
    rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    rectDef_s *textStyle,
    double text_x,
    double text_y);
void __cdecl CG_DrawTankBarrel(int localClientNum, const rectDef_s *rect, Material *material, const float *color);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawInvalidCmdHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double fontscale,
    float *color,
    int textStyle);
void __cdecl CG_ArchiveState(int localClientNum, MemoryFile *memFile);
float __cdecl CG_FadeHudMenu(int localClientNum, const dvar_s *fadeDvar, int displayStartTime, int duration);
void __cdecl CG_DrawPlayerAmmoBackdrop(int localClientNum, const rectDef_s *rect, float *color, Material *material);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerAmmoValue(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    Material *material,
    int textStyle);
void __cdecl CG_DrawPlayerWeaponName(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle);
void __cdecl CG_DrawPlayerWeaponNameBack(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    Material *material,
    int a7);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerStance(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *color,
    Font_s *font,
    double scale,
    int textStyle);
void __cdecl CG_DrawPlayerSprintBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color,
    long double a5);
void __cdecl CG_DrawPlayerSprintMeter(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    DvarValue *color,
    long double a5);
void __cdecl CG_DrawPlayerBarHealth(int localClientNum, const rectDef_s *rect, Material *material, float *color);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerBarHealthBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color,
    long double a5);
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
    Font_s *font_0);
