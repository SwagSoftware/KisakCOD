#pragma once
#include "r_rendercmds.h"

enum $A1218AF7D1C12B1D50BD9B9B70D78FD4 : __int32
{
    SCENE_VIEW_CAMERA = 0x0,
    SCENE_VIEW_SUNSHADOW_0 = 0x1,
    SCENE_VIEW_SUNSHADOW_1 = 0x2,
    SCENE_VIEW_COUNT = 0x3,

    SCENE_VIEW_SPOTSHADOW_0 = 0x3,
    SCENE_VIEW_SPOTSHADOW_1 = 0x4,
    SCENE_VIEW_SPOTSHADOW_2 = 0x5,
    SCENE_VIEW_SPOTSHADOW_3 = 0x6,
    SCENE_VIEW_COUNT_ENTVIS = 0x7,
};

void __cdecl R_SunShadowMapBoundingPoly(
    const GfxSunShadowBoundingPoly *boundingPoly,
    float sampleSize,
    float (*polyInClipSpace)[9][2],
    int *pointIsNear);
void __cdecl R_GetSunAxes(float (*sunAxis)[3][3]);
void __cdecl R_SetupSunShadowMaps(const GfxViewParms *viewParms, GfxSunShadow *sunShadow);
void __cdecl R_GetSceneExtentsAlongDir(const float *origin, const float *forward, float *nearCap, float *farCap);
void __cdecl R_SetupSunShadowMapProjection(
    const GfxViewParms *viewParms,
    const float (*sunAxis)[3][3],
    GfxSunShadow *sunShadow,
    float (*snappedViewOrgInClipSpace)[2][2],
    float *partitionFraction);
void __cdecl R_SunShadowMapProjectionMatrix(
    const float *snappedViewOrgInClipSpace,
    float shadowSampleSize,
    float nearClip,
    float farClip,
    GfxViewParms *shadowViewParms);
void __cdecl R_ClipSpaceToWorldSpace(
    const GfxMatrix *invViewProjMtx,
    const float (*clipSpacePoints)[4][3],
    int pointCount,
    float (*worldSpacePoints)[4][3]);
void __cdecl R_SetupSunShadowBoundingPoly(
    float (*frustumPointsInSunProj)[2],
    const float *viewOrgInSunProj,
    const float *snappedViewOrgInSunProj,
    float maxSizeInSunProj,
    const float *snappedViewOrgInClipSpace,
    GfxSunShadowBoundingPoly *boundingPoly,
    unsigned int pointCount);
void __cdecl R_SetupSunShadowMapViewMatrix(
    const float *snappedViewOrgInSunProj,
    const float (*sunAxis)[3][3],
    GfxSunShadowProjection *sunProj);
void __cdecl R_SetupSunShadowMapPartitionFraction(
    const GfxViewParms *viewParms,
    float scaleToFitUsable,
    GfxSunShadowProjection *sunProj,
    float *partitionFraction);
void __cdecl R_GetSunShadowMapPartitionViewOrgInTextureSpace(
    const float *viewOrgInPixels,
    const float *viewOrgInSunProj,
    const float *snappedViewOrgInSunProj,
    float sampleSize,
    float *viewOrgInTexSpace);
void __cdecl R_SetupNearRegionPlane(const float *partitionFraction);
void __cdecl R_GetSunShadowLookupMatrix(
    const GfxViewParms *shadowViewParms,
    const GfxSunShadowProjection *sunProj,
    const float *partitionFraction,
    GfxMatrix *lookupMatrix);
void __cdecl R_SunShadowMaps();
void __cdecl R_MergeAndEmitSunShadowMapsSurfs(GfxViewInfo *viewInfo);
