#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

void __fastcall TRACK_cl_srcn();
void __fastcall SCR_DrawSmallStringExt(unsigned int x, int y, const char *string, const float *setColor);
void __fastcall SCR_Init();
bool __fastcall CL_IsCGameRendering();
int __fastcall CL_GetDemoType();
int __fastcall CL_CGameRendering();
void CL_DrawScreen();
void __fastcall SCR_ClearScreen(int a1, int a2, unsigned __int8 a3);
void __fastcall SCR_DrawScreenField(int refreshedUI);
float __fastcall CL_GetMenuBlurRadius(int localClientNum);
void __fastcall SCR_UpdateRumble();
int SCR_UpdateFrame();
void __fastcall SCR_UpdateScreen();
void __fastcall SCR_UpdateLoadScreen();
void CL_CubemapShotUsage();
void __fastcall CL_CubemapShot_f();
