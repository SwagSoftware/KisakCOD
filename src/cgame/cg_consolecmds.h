#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

int __fastcall CG_CheatsOK(const char *cmdName);
void CG_Viewpos_f();
void CG_ScoresUp_f();
void CG_ScoresDown_f();
void CG_Fade_f();
void CG_ShellShock_f();
void CG_ShellShock_Load_f();
void CG_ShellShock_Save_f();
void CG_ModelPreviewerStepAnim_f();
void CG_ModelPreviewerPauseAnim_f();
void CG_Noclip_f();
void CG_UFO_f();
void __fastcall CG_SetViewPos_f();
void __fastcall SphereCoordsToPos(
    double sphereDistance,
    double sphereYaw,
    double sphereAltitude,
    float *result,
    int a5,
    int a6,
    float *a7);
void __fastcall CG_SetViewOrbit_f();
void CG_PlayRumble_f();
void __fastcall UpdateGlowTweaks_f();
void __fastcall UpdateFilmTweaks_f();
void __fastcall CG_InitConsoleCommands();
