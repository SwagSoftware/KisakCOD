#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

int __cdecl CG_CheatsOK(const char *cmdName);
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
void __cdecl CG_SetViewPos_f();
void __cdecl SphereCoordsToPos(
    float sphereDistance,
    float sphereYaw,
    float sphereAltitude,
    float *result);
void __cdecl CG_SetViewOrbit_f();
void CG_PlayRumble_f();
void __cdecl UpdateGlowTweaks_f();
void __cdecl UpdateFilmTweaks_f();
void __cdecl CG_InitConsoleCommands();
