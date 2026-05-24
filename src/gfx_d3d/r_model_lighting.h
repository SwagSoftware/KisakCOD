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
    uint8_t primaryLightIndex;  // ...
    uint8_t reflectionProbeIndex; // ...
};


void __cdecl R_SetModelLightingCoords(uint16_t handle, float *out);
uint32_t __cdecl R_ModelLightingIndexFromHandle(uint16_t handle);
void __cdecl R_GetPackedStaticModelLightingCoords(uint32_t smodelIndex, PackedLightingCoords *packedCoords);
char __cdecl R_AllocStaticModelLighting(GfxStaticModelDrawInst *smodelDrawInst, uint32_t smodelIndex);
uint32_t __cdecl R_AllocModelLighting_PrimaryLight(
    float *lightingOrigin,
    uint32_t dynEntId,
    uint16_t *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut);
uint32_t __cdecl R_AllocModelLighting(
    float *lightingOrigin,
    uint16_t *cachedLightingHandle,
    uint32_t(__cdecl *GetPrimaryLightCallback)(const void *),
    const void *userData,
    GfxLightingInfo *lightingInfoOut);
uint32_t __cdecl R_DynEntPrimaryLightCallback(const void *userData);
uint32_t __cdecl R_AllocModelLighting_Box(
    const GfxViewInfo *viewInfo,
    float *lightingOrigin,
    const float *boxMins,
    const float *boxMaxs,
    uint16_t *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut);
uint32_t __cdecl R_GetPrimaryLightForBoxCallback(const void *userData);
uint32_t __cdecl R_AllocModelLighting_Sphere(
    const GfxViewInfo *viewInfo,
    float *lightingOrigin,
    const float *origin,
    float radius,
    uint16_t *cachedLightingHandle,
    GfxLightingInfo *lightingInfoOut);
uint32_t __cdecl R_GetPrimaryLightForSphereCallback(const void *userData);
void __cdecl R_ToggleModelLightingFrame();
uint32_t __cdecl R_CalcModelLighting(
    uint32_t entryIndex,
    const float *lightingOrigin,
    uint32_t nonSunPrimaryLightIndex,
    GfxModelLightExtrapolation extrapolateBehavior);
void __cdecl R_BeginAllStaticModelLighting();
void __cdecl R_SetAllStaticModelLighting();
void __cdecl R_SetStaticModelLighting(uint32_t smodelIndex);
void __cdecl R_SetModelGroundLighting(uint32_t entryIndex, const uint8_t *groundLighting);
void __cdecl R_SetModelLightingCoordsForSource(uint16_t handle, GfxCmdBufSourceState *source);
void __cdecl R_SetStaticModelLightingCoordsForSource(uint32_t smodelIndex, GfxCmdBufSourceState *source);
uint32_t R_SetModelLightingSampleDeltas();
void __cdecl R_SetModelLightingLookupScale(GfxCmdBufInput *input);
void __cdecl R_SetupDynamicModelLighting(GfxCmdBufInput *input);
void __cdecl R_InitModelLightingGlobals();
void __cdecl R_ShutdownModelLightingGlobals();
char *__cdecl R_AllocModelLightingGlobal(uint32_t bytes);
void __cdecl R_ResetModelLighting();
void __cdecl R_InitModelLightingImage();
void __cdecl R_ShutdownModelLightingImage();
void __cdecl R_InitStaticModelLighting();

void __cdecl RB_PatchModelLighting(const GfxModelLightingPatch *patchList, uint32_t patchCount);