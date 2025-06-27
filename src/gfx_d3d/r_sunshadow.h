#pragma once
#include "r_rendercmds.h"


void __cdecl R_GetSunAxes(float (*sunAxis)[3]);
void __cdecl R_SunShadowMapBoundingPoly(
    const GfxSunShadowBoundingPoly *boundingPoly,
    float sampleSize,
    float (*polyInClipSpace)[2],
    int *pointIsNear);
void __cdecl R_SetupSunShadowMaps(const GfxViewParms *viewParms, GfxSunShadow *sunShadow);
void __cdecl R_GetSceneExtentsAlongDir(const float *origin, const float *forward, float *nearCap, float *farCap);
unsigned int __cdecl R_SunShadowMapClipSpaceClipPlanes(
    const GfxSunShadowBoundingPoly *boundingPoly,
    int partitionIndex,
    float sampleSize,
    float (*boundingPolyClipSpacePlanes)[4]);
void __cdecl R_SunShadowMapProjectionMatrix(
    const float *snappedViewOrgInClipSpace,
    float shadowSampleSize,
    float nearClip,
    float farClip,
    GfxViewParms *shadowViewParms);
void __cdecl R_SetupSunShadowMapProjection(
    const GfxViewParms *viewParms,
    const float (*sunAxis)[3],
    GfxSunShadow *sunShadow,
    float (*snappedViewOrgInClipSpace)[2],
    float *partitionFraction);
void __cdecl R_GetFrustumNearClipPoints(const GfxMatrix *invViewProjMtx, float (*frustumPoints)[3]);
void __cdecl R_ClipSpaceToWorldSpace(
    const GfxMatrix *invViewProjMtx,
    const float (*clipSpacePoints)[3],
    int pointCount,
    float (*worldSpacePoints)[3]);
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
    const float (*sunAxis)[3],
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
