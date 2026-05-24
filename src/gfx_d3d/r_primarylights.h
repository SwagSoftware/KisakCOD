#pragma once
#include "r_rendercmds.h"

enum GfxLightType : __int32
{
    GFX_LIGHT_TYPE_NONE = 0x0,
    GFX_LIGHT_TYPE_DIR = 0x1,
    GFX_LIGHT_TYPE_SPOT = 0x2,
    GFX_LIGHT_TYPE_OMNI = 0x3,
    GFX_LIGHT_TYPE_COUNT = 0x4,
    GFX_LIGHT_TYPE_DIR_SHADOWMAP = 0x4,
    GFX_LIGHT_TYPE_SPOT_SHADOWMAP = 0x5,
    GFX_LIGHT_TYPE_OMNI_SHADOWMAP = 0x6,
    GFX_LIGHT_TYPE_COUNT_WITH_SHADOWMAP_VERSIONS = 0x7,
};

struct GfxCandidateShadowedLight // sizeof=0x8
{                                       // ...
    uint32_t shadowableLightIndex;  // ...
    float score;
};

struct GfxShadowedLightEntry // sizeof=0x8
{                                       // ...
    uint8_t shadowableLightIndex;
    bool isFadingOut;
    // padding byte
    // padding byte
    float fade;
};
struct GfxShadowedLightHistory // sizeof=0x48
{                                       // ...
    uint32_t shadowableLightWasUsed[8];
    GfxShadowedLightEntry entries[4];
    uint32_t entryCount;
    uint32_t lastUpdateTime;
};

struct GfxShadowGeometry // sizeof=0xC
{
    uint16_t surfaceCount;
    uint16_t smodelCount;
    uint16_t *sortedSurfIndex;
    uint16_t *smodelIndex;
};
struct GfxLightRegionAxis // sizeof=0x14
{
    float dir[3];
    float midPoint;
    float halfSize;
};
struct GfxLightRegionHull // sizeof=0x50
{
    float kdopMidPoint[9];
    float kdopHalfSize[9];
    uint32_t axisCount;
    GfxLightRegionAxis *axis;
};
struct GfxLightRegion // sizeof=0x8
{
    uint32_t hullCount;
    GfxLightRegionHull *hulls;
};

void __cdecl R_ClearShadowedPrimaryLightHistory(int localClientNum);
void __cdecl R_AddDynamicShadowableLight(GfxViewInfo *viewInfo, const GfxLight *visibleLight);
bool __cdecl R_IsDynamicShadowedLight(uint32_t shadowableLightIndex);
bool __cdecl R_IsPrimaryLight(uint32_t shadowableLightIndex);
void __cdecl R_ChooseShadowedLights(GfxViewInfo *viewInfo);
uint32_t __cdecl R_AddPotentiallyShadowedLight(
    const GfxViewInfo *viewInfo,
    uint32_t shadowableLightIndex,
    GfxCandidateShadowedLight *candidateLights,
    uint32_t candidateLightCount);
double __cdecl R_ShadowedSpotLightScore(const GfxViewParms *viewParms, const GfxLight *light);
void __cdecl R_AddShadowsForLight(GfxViewInfo *viewInfo, uint32_t shadowableLightIndex, float spotShadowFade);
void __cdecl R_AddShadowedLightToShadowHistory(
    GfxShadowedLightHistory *shadowHistory,
    uint32_t shadowableLightIndex,
    float fadeDelta);
void __cdecl R_FadeOutShadowHistoryEntries(GfxShadowedLightHistory *shadowHistory, float fadeDelta);
void __cdecl R_LinkSphereEntityToPrimaryLights(
    uint32_t localClientNum,
    uint32_t entityNum,
    const float *origin,
    float radius);
uint32_t __cdecl R_GetPrimaryLightEntityShadowBit(
    uint32_t localClientNum,
    uint32_t entnum,
    uint32_t primaryLightIndex);
void __cdecl R_LinkBoxEntityToPrimaryLights(
    uint32_t localClientNum,
    uint32_t entityNum,
    const float *mins,
    const float *maxs);
char __cdecl R_CullBoxFromLightRegionHull(
    const GfxLightRegionHull *hull,
    const float *boxMidPoint,
    const float *boxHalfSize);
void __cdecl R_LinkDynEntToPrimaryLights(
    uint32_t dynEntId,
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs);
bool __cdecl Com_CullBoxFromPrimaryLight(
    const struct ComPrimaryLight *light,
    const float *boxMidPoint,
    const float *boxHalfSize);
uint32_t __cdecl R_GetPrimaryLightDynEntShadowBit(uint32_t entnum, uint32_t primaryLightIndex);
void __cdecl R_UnlinkEntityFromPrimaryLights(uint32_t localClientNum, uint32_t entityNum);
void __cdecl R_UnlinkDynEntFromPrimaryLights(uint32_t dynEntId, DynEntityDrawType drawType);
bool __cdecl R_IsEntityVisibleToPrimaryLight(
    uint32_t localClientNum,
    uint32_t entityNum,
    uint32_t primaryLightIndex);
bool __cdecl R_IsDynEntVisibleToPrimaryLight(
    uint32_t dynEntId,
    DynEntityDrawType drawType,
    uint32_t primaryLightIndex);
int __cdecl R_IsEntityVisibleToAnyShadowedPrimaryLight(const GfxViewInfo *viewInfo, uint32_t entityNum);
bool __cdecl R_IsEntityVisibleToShadowedPrimaryLight(uint32_t baseBitIndex, uint32_t shadowableLightIndex);
int __cdecl R_IsDynEntVisibleToAnyShadowedPrimaryLight(
    const GfxViewInfo *viewInfo,
    uint32_t dynEntId,
    DynEntityDrawType drawType);
bool __cdecl R_IsDynEntVisibleToShadowedPrimaryLight(
    uint32_t baseBitIndex,
    DynEntityDrawType drawType,
    uint32_t shadowableLightIndex);
uint32_t __cdecl R_GetNonSunPrimaryLightForBox(
    const GfxViewInfo *viewInfo,
    const float *boxMidPoint,
    const float *boxHalfSize);
uint32_t __cdecl R_GetNonSunPrimaryLightForSphere(const GfxViewInfo *viewInfo, const float *origin, float radius);
char __cdecl R_CullSphereFromLightRegionHull(const GfxLightRegionHull *hull, const float *origin, float radius);
bool __cdecl Com_CullSphereFromPrimaryLight(const struct ComPrimaryLight *light, const float *origin, float radius);
