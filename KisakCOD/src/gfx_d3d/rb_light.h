#pragma once
#include "r_gfx.h"
#include "r_model_lighting.h"

enum GfxSortedHistoryAdd : __int32
{                                       // ...
    SH_ADD_NEVER = 0x0,
    SH_ADD_IF_NEW = 0x1,
};

void __cdecl R_SetLightGridSampleDeltas(int rowStride, int sliceStride);
void __cdecl R_ShowLightVisCachePoints(const float *viewOrigin, const DpvsPlane *clipPlanes, int clipPlaneCount);
int __cdecl R_SortedHistoryEntry(int x, int y, int z, GfxSortedHistoryAdd addMode);
char __cdecl R_AddSortedHistoryEntry(unsigned __int16 x, unsigned __int16 y, unsigned __int16 z, int pos);
void __cdecl R_SetLightGridColors(
    const GfxLightGridColors *colors,
    unsigned __int8 packedSunWeight,
    unsigned __int8 *pixels);
void __cdecl R_FixedPointBlendLightGridColors(
    const GfxLightGrid *lightGrid,
    const unsigned __int16 *colorsIndex,
    unsigned __int16 *fixedPointWeight,
    unsigned int colorsCount,
    GfxLightGridColors *outPacked);
void __cdecl R_ScaleLightGridColors(
    const GfxLightGridColors *colors,
    unsigned __int16 fixedPointWeight,
    unsigned __int16 *scaled);
void __cdecl R_WeightedAccumulateLightGridColors(
    const GfxLightGridColors *colors,
    unsigned __int16 fixedPointWeight,
    unsigned __int16 *accumulated);
void __cdecl R_PackAccumulatedLightGridColors(const unsigned __int16 *accumulated, GfxLightGridColors *packed);
unsigned __int8 __cdecl R_GetPrimaryLightFromGrid(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    unsigned __int8 sunPrimaryLightIndex);
unsigned __int8 __cdecl R_LightGridLookup(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    float *cornerWeight,
    const GfxLightGridEntry **cornerEntry,
    unsigned int *defaultGridEntry);
void __cdecl R_ShowLightGrid(
    const GfxLightGrid *lightGrid,
    const unsigned int *pos,
    const float *samplePos,
    const GfxLightGridEntry **cornerEntry,
    bool *suppressEntry,
    bool honorSuppression);
void __cdecl R_ShowGridOrigin(const float *origin);
void __cdecl R_ShowGridBox(const unsigned int *pos);
void __cdecl R_ShowGridCorner(unsigned int x, unsigned int y, unsigned int z, float halfSize, const float *color);
void __cdecl R_UpdateVisHistory(const GfxLightGrid *lightGrid, const unsigned int *pos);
void __cdecl R_GetLightGridSampleEntryQuad(
    const GfxLightGrid *lightGrid,
    const unsigned int *pos,
    const GfxLightGridEntry **entries,
    unsigned int *defaultGridEntry);
bool __cdecl R_IsValidLightGridSample(
    const GfxLightGrid *lightGrid,
    const GfxLightGridEntry *entry,
    char cornerIndex,
    const unsigned int *pos,
    const float *samplePos);
unsigned int __cdecl R_GetLightingAtPoint(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    unsigned int nonSunPrimaryLightIndex,
    unsigned __int16 dest,
    GfxModelLightExtrapolation extrapolateBehavior);
void __cdecl R_SetLightGridColorsFromIndex(
    const GfxLightGrid *lightGrid,
    unsigned int colorsIndex,
    float primaryLightWeight,
    unsigned __int16 dest);
void __cdecl R_BlendAndSetLightGridColors(
    const GfxLightGrid *lightGrid,
    unsigned __int8 *colorsIndex,
    const float *colorsWeight,
    unsigned int colorsCount,
    float primaryLightWeight,
    float weightNormalizeScale,
    unsigned __int16 dest);
void __cdecl R_GetLightGridColorsFixedPointBlendWeights(
    const float *colorsWeight,
    unsigned int colorsCount,
    float weightNormalizeScale,
    unsigned __int16 *fixedPointWeight);
unsigned __int8 __cdecl R_ExtrapolateLightingAtPoint(
    const GfxLightGrid *lightGrid,
    unsigned __int16 dest,
    GfxModelLightExtrapolation extrapolateBehavior,
    unsigned int defaultGridEntry);
unsigned int __cdecl R_AddLightGridSample(
    unsigned __int16 *sampleColors,
    float *sampleWeight,
    unsigned int sampleCount,
    unsigned __int16 sampleColorsAdd,
    float sampleWeightAdd);
char __cdecl R_CanLightInfluenceLightGridCorner(
    const GfxLightGrid *lightGrid,
    const ComPrimaryLight *light,
    const float *samplePos,
    char cornerIndex);
void __cdecl R_GetAverageLightingAtPoint(const float *samplePos, unsigned __int8 *outColor);
void __cdecl R_BlendAndAverageLightGridColors(
    const GfxLightGrid *lightGrid,
    const unsigned __int16 *colorsIndex,
    const float *colorsWeight,
    unsigned int colorsCount,
    float primaryLightWeight,
    float weightNormalizeScale,
    unsigned __int8 *outAverage);
void __cdecl R_AverageLightGridColors(const GfxLightGridColors *colors, float sunWeight, unsigned __int8 *outColor);
void __cdecl R_InitLightVisHistory(char *bspName);
void __cdecl R_LightVisHistoryFilename(char *bspName, char *filename);
void __cdecl R_SaveLightVisHistory();
unsigned __int8 __cdecl R_GetPrimaryLightForModel(
    const XModel *model,
    const float *origin,
    const float (*axis)[3],
    float scale,
    const float *mins,
    const float *maxs,
    const GfxLightRegion *lightRegions);

GfxModelLightingPatch *__cdecl R_BackEndDataAllocAndClearModelLightingPatch(GfxBackEndData *frontEndDataOut);