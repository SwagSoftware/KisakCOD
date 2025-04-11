#pragma once
#include "r_rendercmds.h"

struct GfxCandidateShadowedLight // sizeof=0x8
{                                       // ...
    unsigned int shadowableLightIndex;  // ...
    float score;
};

struct GfxShadowedLightEntry // sizeof=0x8
{                                       // ...
    unsigned __int8 shadowableLightIndex;
    bool isFadingOut;
    // padding byte
    // padding byte
    float fade;
};
struct GfxShadowedLightHistory // sizeof=0x48
{                                       // ...
    unsigned int shadowableLightWasUsed[8];
    GfxShadowedLightEntry entries[4];
    unsigned int entryCount;
    unsigned int lastUpdateTime;
};

void __cdecl R_ClearShadowedPrimaryLightHistory(int localClientNum);
void __cdecl R_AddDynamicShadowableLight(GfxViewInfo *viewInfo, const GfxLight *visibleLight);
bool __cdecl R_IsDynamicShadowedLight(unsigned int shadowableLightIndex);
bool __cdecl R_IsPrimaryLight(unsigned int shadowableLightIndex);
void __cdecl R_ChooseShadowedLights(GfxViewInfo *viewInfo);
unsigned int __cdecl R_AddPotentiallyShadowedLight(
    const GfxViewInfo *viewInfo,
    unsigned int shadowableLightIndex,
    GfxCandidateShadowedLight *candidateLights,
    unsigned int candidateLightCount);
double __cdecl R_ShadowedSpotLightScore(const GfxViewParms *viewParms, const GfxLight *light);
void __cdecl R_AddShadowsForLight(GfxViewInfo *viewInfo, unsigned int shadowableLightIndex, float spotShadowFade);
void __cdecl R_AddShadowedLightToShadowHistory(
    GfxShadowedLightHistory *shadowHistory,
    unsigned int shadowableLightIndex,
    float fadeDelta);
void __cdecl R_FadeOutShadowHistoryEntries(GfxShadowedLightHistory *shadowHistory, float fadeDelta);
void __cdecl R_LinkSphereEntityToPrimaryLights(
    unsigned int localClientNum,
    unsigned int entityNum,
    const float *origin,
    float radius);
unsigned int __cdecl R_GetPrimaryLightEntityShadowBit(
    unsigned int localClientNum,
    unsigned int entnum,
    unsigned int primaryLightIndex);
void __cdecl R_LinkBoxEntityToPrimaryLights(
    unsigned int localClientNum,
    unsigned int entityNum,
    const float *mins,
    const float *maxs);
char __cdecl R_CullBoxFromLightRegionHull(
    const GfxLightRegionHull *hull,
    const float *boxMidPoint,
    const float *boxHalfSize);
void __cdecl R_LinkDynEntToPrimaryLights(
    unsigned int dynEntId,
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs);
bool __cdecl Com_CullBoxFromPrimaryLight(
    const ComPrimaryLight *light,
    const float *boxMidPoint,
    const float *boxHalfSize);
unsigned int __cdecl R_GetPrimaryLightDynEntShadowBit(unsigned int entnum, unsigned int primaryLightIndex);
void __cdecl R_UnlinkEntityFromPrimaryLights(unsigned int localClientNum, unsigned int entityNum);
void __cdecl R_UnlinkDynEntFromPrimaryLights(unsigned int dynEntId, DynEntityDrawType drawType);
bool __cdecl R_IsEntityVisibleToPrimaryLight(
    unsigned int localClientNum,
    unsigned int entityNum,
    unsigned int primaryLightIndex);
bool __cdecl R_IsDynEntVisibleToPrimaryLight(
    unsigned int dynEntId,
    DynEntityDrawType drawType,
    unsigned int primaryLightIndex);
int __cdecl R_IsEntityVisibleToAnyShadowedPrimaryLight(const GfxViewInfo *viewInfo, unsigned int entityNum);
bool __cdecl R_IsEntityVisibleToShadowedPrimaryLight(unsigned int baseBitIndex, unsigned int shadowableLightIndex);
int __cdecl R_IsDynEntVisibleToAnyShadowedPrimaryLight(
    const GfxViewInfo *viewInfo,
    unsigned int dynEntId,
    DynEntityDrawType drawType);
bool __cdecl R_IsDynEntVisibleToShadowedPrimaryLight(
    unsigned int baseBitIndex,
    DynEntityDrawType drawType,
    unsigned int shadowableLightIndex);
unsigned int __cdecl R_GetNonSunPrimaryLightForBox(
    const GfxViewInfo *viewInfo,
    const float *boxMidPoint,
    const float *boxHalfSize);
unsigned int __cdecl R_GetNonSunPrimaryLightForSphere(const GfxViewInfo *viewInfo, const float *origin, float radius);
char __cdecl R_CullSphereFromLightRegionHull(const GfxLightRegionHull *hull, const float *origin, float radius);
bool __cdecl Com_CullSphereFromPrimaryLight(const ComPrimaryLight *light, const float *origin, float radius);
