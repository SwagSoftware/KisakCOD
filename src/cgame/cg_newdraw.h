#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

void __cdecl CG_AntiBurnInHUD_RegisterDvars();
unsigned int __cdecl CG_GetSelectedWeaponIndex(const cg_s *cgameGlob);
int __cdecl CG_IsHudHidden();
int __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, unsigned int weapIndex);
int __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob);
// local variable allocation has failed, the output may be wrong!
int __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, unsigned int weapIndex);
int __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob);
void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, float *color);
void __cdecl CG_DrawStanceIcon(
    int32_t localClientNum,
    const rectDef_s *rect,
    float *drawColor,
    float x,
    float y,
    float fadeAlpha);
void __cdecl CG_DrawStanceHintPrints(
    int32_t localClientNum,
    const rectDef_s *rect,
    float x,
    const float *color,
    float fadeAlpha,
    Font_s *font,
    float scale,
    int32_t textStyle);
float CG_CalcPlayerHealth(const playerState_s *ps); 
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
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
    float *color,
    int32_t textStyle);
void __cdecl CG_DrawHoldBreathHint(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float fontscale,
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
    Material *material);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerStance(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Font_s *font,
    float scale,
    int32_t textStyle);
void __cdecl CG_DrawPlayerSprintBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_DrawPlayerSprintMeter(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_DrawPlayerBarHealth(int localClientNum, const rectDef_s *rect, Material *material, float *color);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_DrawPlayerBarHealthBack(
    int localClientNum,
    const rectDef_s *rect,
    Material *material,
    float *color);
void CG_OwnerDraw(
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
    char textAlignMode);


extern const dvar_t *hud_fade_sprint;
extern const dvar_t *hud_health_pulserate_injured;
extern const dvar_t *hud_health_startpulse_critical;
extern const dvar_t *hud_fade_offhand;
extern const dvar_t *hud_deathQuoteFadeTime;
extern const dvar_t *hud_fade_ammodisplay;
extern const dvar_t *hud_health_startpulse_injured;
extern const dvar_t *hud_fade_stance;
extern const dvar_t *hud_fade_compass;
extern const dvar_t *hud_health_pulserate_critical;
extern const dvar_t *hud_fade_healthbar;