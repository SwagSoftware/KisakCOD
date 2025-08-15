#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

void __cdecl TRACK_cl_srcn();
void __cdecl SCR_DrawSmallStringExt(unsigned int x, int y, const char *string, const float *setColor);
void __cdecl SCR_Init();
float __cdecl CL_GetMenuBlurRadius(int localClientNum);
void __cdecl SCR_UpdateScreen();
void SCR_UpdateFrame();
int __cdecl CL_CGameRendering();


bool __cdecl CL_IsCGameRendering();
void CL_DrawScreen();
void __cdecl SCR_DrawScreenField(int refreshedUI);
void __cdecl SCR_UpdateRumble();
void __cdecl SCR_UpdateLoadScreen();
void CL_CubemapShotUsage();
void __cdecl CL_CubemapShot_f();
