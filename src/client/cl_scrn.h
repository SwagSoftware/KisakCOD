#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

void __cdecl TRACK_cl_srcn();
void __cdecl SCR_DrawSmallStringExt(unsigned int x, int y, const char *string, const float *setColor);
void __cdecl SCR_Init();
bool __cdecl CL_IsCGameRendering();
int __cdecl CL_CGameRendering();
void CL_DrawScreen();
void __cdecl SCR_ClearScreen(int a1, int a2, unsigned __int8 a3);
void __cdecl SCR_DrawScreenField(int refreshedUI);
void __cdecl SCR_UpdateRumble();
void SCR_UpdateFrame();
void __cdecl SCR_UpdateScreen();
void __cdecl SCR_UpdateLoadScreen();
void CL_CubemapShotUsage();
void __cdecl CL_CubemapShot_f();
