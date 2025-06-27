#pragma once
#include "r_rendercmds.h"
#include "rb_backend.h"


bool __cdecl R_UsingGlow(const GfxViewInfo *viewInfo);
bool __cdecl R_UsingDepthOfField(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingColorManipulation(const GfxViewInfo *viewInfo);
void __cdecl RB_ApplyColorManipulationFullscreen(const GfxViewInfo *viewInfo);
void __cdecl RB_ApplyColorManipulationSplitscreen(const GfxViewInfo *viewInfo);
void RB_GetResolvedScene();
void __cdecl RB_GetDepthOfFieldInputImages(float radius);
void __cdecl RB_ProcessPostEffects(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingMergedPostEffects(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingBlur(float blurRadius);
bool __cdecl RB_UsingPostEffects(const GfxViewInfo *viewInfo);
void __cdecl RB_CalcGlowEffect(const GfxViewInfo *viewInfo);
void __cdecl RB_ApplyGlowEffect(const GfxViewInfo *viewInfo);
void __cdecl RB_ApplyGlowEffectPass(const GfxViewInfo *viewInfo, GfxImage *glowImage);
void __cdecl RB_ApplyMergedPostEffects(const GfxViewInfo *viewInfo);
void __cdecl R_UpdateCodeConstant(
    GfxCmdBufSourceState *source,
    unsigned int constant,
    float x,
    float y,
    float z,
    float w);
void __cdecl RB_GetSceneDepthOfFieldEquation(
    float nearOutOfFocus,
    float nearInFocus,
    float farInFocus,
    float farOutOfFocus,
    float *dofEquation,
    float zNear);
void __cdecl RB_GetNearDepthOfFieldEquation(
    float outOfFocus,
    float inFocus,
    float nearClip,
    float depthScale,
    float *dofEquation);
void __cdecl RB_GetViewModelDepthOfFieldEquation(float outOfFocus, float inFocus, float *dofEquation);
double __cdecl RB_GetDepthOfFieldBlurFraction(const GfxViewInfo *viewInfo, float pixelRadiusAtSceneRes);
double __cdecl RB_GetBlurRadius(float blurRadiusFromCode);
void __cdecl RB_BlurScreen(const GfxViewInfo *viewInfo, float blurRadius);
