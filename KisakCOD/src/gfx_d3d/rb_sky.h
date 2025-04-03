#pragma once
#include "r_sky.h"


void __cdecl TRACK_rb_sky();
unsigned int __cdecl RB_CalcSunSpriteSamples();
void __cdecl RB_DrawSun(unsigned int localClientNum);
void __cdecl RB_DrawSunQuerySprite(SunFlareDynamic *sunFlare);
void __cdecl RB_HW_BeginOcclusionQuery(IDirect3DQuery9 *query);
unsigned int __cdecl RB_HW_ReadOcclusionQuery(IDirect3DQuery9 *query);
void __cdecl RB_TessSunBillboard(float widthInClipSpace, float heightInClipSpace, GfxColor color);
GfxVertex *__cdecl RB_SetTessQuad(GfxColor color);
void __cdecl RB_UpdateSunVisibilityWithoutQuery(SunFlareDynamic *sunFlare);
double __cdecl RB_GetSunSampleRectRelativeArea(int widthInPixels, int heightInPixels);
void __cdecl RB_AddSunEffects(SunFlareDynamic *sunFlare);
void RB_DrawSunSprite();
void __cdecl RB_DrawSunPostEffects(unsigned int localClientNum);
void __cdecl RB_DrawSunFlare(SunFlareDynamic *sunFlare, int frameTime);
double __cdecl R_UpdateOverTime(float fCurrent, float fGoal, int iFadeInTime, int iFadeOutTime, int frametime);
void __cdecl RB_DrawSunFlareCore(float alpha, float sizeIn640x480);
void __cdecl RB_DrawBlindAndGlare(SunFlareDynamic *sunFlare, int frameTime);
void __cdecl RB_CalcSunBlind(SunFlareDynamic *sunFlare, int frameTime, float *blind, float *glare);
void __cdecl RB_AllocSunSpriteQueries();
