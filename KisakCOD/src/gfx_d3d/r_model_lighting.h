#pragma once
#include "r_gfx.h"
#include "r_rendercmds.h"
#include "rb_backend.h"

enum GfxModelLightExtrapolation : __int32
{                                       // ...
    GFX_MODELLIGHT_EXTRAPOLATE = 0x0,
    GFX_MODELLIGHT_SHOW_MISSING = 0x1,
};

struct GfxLightingInfo // sizeof=0x2
{                                       // ...
    unsigned __int8 primaryLightIndex;  // ...
    unsigned __int8 reflectionProbeIndex; // ...
};


void __cdecl R_SetModelLightingCoords(unsigned __int16 handle, float *out);
unsigned int __cdecl R_ModelLightingIndexFromHandle(unsigned __int16 handle);
void __cdecl R_GetPackedStaticModelLightingCoords(unsigned int smodelIndex, PackedLightingCoords *packedCoords);
char __cdecl R_AllocStaticModelLighting(GfxStaticModelDrawInst *smodelDrawInst, unsigned int smodelIndex);
unsigned int __cdecl R_AllocModelLighting_PrimaryLight(
    float *lightingOrigin,
    unsigned int dynEntId,
    unsigned __int16 *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_AllocModelLighting(
    float *lightingOrigin,
    unsigned __int16 *cachedLightingHandle,
    unsigned int(__cdecl *GetPrimaryLightCallback)(const void *),
    const void *userData,
    GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_DynEntPrimaryLightCallback(const void *userData);
unsigned int __cdecl R_AllocModelLighting_Box(
    const GfxViewInfo *viewInfo,
    float *lightingOrigin,
    const float *boxMins,
    const float *boxMaxs,
    unsigned __int16 *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_GetPrimaryLightForBoxCallback(const void *userData);
unsigned int __cdecl R_AllocModelLighting_Sphere(
    const GfxViewInfo *viewInfo,
    float *lightingOrigin,
    const float *origin,
    float radius,
    unsigned __int16 *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_GetPrimaryLightForSphereCallback(const void *userData);
void __cdecl R_ToggleModelLightingFrame();
unsigned int __cdecl R_CalcModelLighting(
    unsigned int entryIndex,
    const float *lightingOrigin,
    unsigned int nonSunPrimaryLightIndex,
    GfxModelLightExtrapolation extrapolateBehavior);
void __cdecl R_BeginAllStaticModelLighting();
void __cdecl R_SetAllStaticModelLighting();
void __cdecl R_SetStaticModelLighting(unsigned int smodelIndex);
void __cdecl R_SetModelGroundLighting(unsigned int entryIndex, const unsigned __int8 *groundLighting);
void __cdecl R_SetModelLightingCoordsForSource(unsigned __int16 handle, GfxCmdBufSourceState *source);
void __cdecl R_SetStaticModelLightingCoordsForSource(unsigned int smodelIndex, GfxCmdBufSourceState *source);
unsigned int R_SetModelLightingSampleDeltas();
void __cdecl R_SetModelLightingLookupScale(GfxCmdBufInput *input);
void __cdecl R_SetupDynamicModelLighting(GfxCmdBufInput *input);
void __cdecl R_InitModelLightingGlobals();
void __cdecl R_ShutdownModelLightingGlobals();
char *__cdecl R_AllocModelLightingGlobal(unsigned int bytes);
void __cdecl R_ResetModelLighting();
void __cdecl R_InitModelLightingImage();
void __cdecl R_ShutdownModelLightingImage();
void __cdecl R_InitStaticModelLighting();

void __cdecl RB_PatchModelLighting(const GfxModelLightingPatch *patchList, unsigned int patchCount);