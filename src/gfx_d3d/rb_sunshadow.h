#pragma once
#include "r_rendercmds.h"



void __cdecl TRACK_rb_sunshadow();
void __cdecl RB_SunShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo);
void __cdecl RB_GetShadowOverlayDepthBounds(float *nearDepth, float *farDepth);
void __cdecl RB_DrawSunShadowOverlay();
void __cdecl RB_SunShadowOverlayPoint(const float *xy, float x0, float y0, float w, float h, float *point);
void RB_SetSunShadowOverlayScaleAndBias();
