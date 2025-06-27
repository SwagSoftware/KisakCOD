#pragma once
#include "fxprimitives.h"
#include "r_scene.h"


void __cdecl R_ProcessCmd_UpdateFxSpotLight(FxCmd *cmd);
void __cdecl R_ProcessCmd_UpdateFxNonDependent(FxCmd *cmd);
void __cdecl R_ProcessCmd_UpdateFxRemaining(FxCmd *cmd);
void __cdecl R_UpdateSpotLightEffect(FxCmd *cmd);
void __cdecl R_UpdateNonDependentEffects(FxCmd *cmd);
void __cdecl R_UpdateRemainingEffects(FxCmd *cmd);
void __cdecl R_UpdateXModelBoundsDelayed(GfxSceneEntity *sceneEnt);
void __cdecl R_SkinGfxEntityDelayed(GfxSceneEntity *sceneEnt);
