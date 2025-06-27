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

struct GfxShadowGeometry // sizeof=0xC
{
    unsigned __int16 surfaceCount;
    unsigned __int16 smodelCount;
    unsigned __int16 *sortedSurfIndex;
    unsigned __int16 *smodelIndex;
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
    unsigned int axisCount;
    GfxLightRegionAxis *axis;
};
struct GfxLightRegion // sizeof=0x8
{
    unsigned int hullCount;
    GfxLightRegionHull *hulls;
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
    const struct ComPrimaryLight *light,
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
bool __cdecl Com_CullSphereFromPrimaryLight(const struct ComPrimaryLight *light, const float *origin, float radius);
