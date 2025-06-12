#include "r_sunshadow.h"
#include "r_dvars.h"
#include "r_init.h"
#include "r_dpvs.h"
#include "r_pretess.h"
#include <universal/com_convexhull.h>
#include <universal/profile.h>

struct ShadowGlobals // sizeof=0x4
{                                       // ...
    const MaterialTechnique *defaultShadowCasterTechnique; // ...
};

const float g_shadowFrustumBound[5][2] =
{
    { -1.0f, -1.0f },
    { -1.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.0f, -1.0f },
    {-1.0f, -1.0f }
};

const float g_clipSpacePoints[4][3] =
{
    { -1.0f, -1.0f, 0.0f },
    { 1.0f, -1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    {-1.0f, 1.0f, 0.0f }
};


ShadowGlobals shadowGlob;

void __cdecl R_GetSunAxes(float (*sunAxis)[3])
{
    float v1; // [esp+0h] [ebp-1Ch]
    float *dir; // [esp+18h] [ebp-4h]

    if (!frontEndDataOut)
        MyAssertHandler(".\\r_shadowcookie.cpp", 62, 0, "%s", "frontEndDataOut");
    if (frontEndDataOut->sunLight.type != 1)
        MyAssertHandler(".\\r_shadowcookie.cpp", 63, 0, "%s", "frontEndDataOut->sunLight.type == GFX_LIGHT_TYPE_DIR");
    dir = frontEndDataOut->sunLight.dir;
    (*sunAxis)[0] = -frontEndDataOut->sunLight.dir[0];
    (*sunAxis)[1] = -dir[1];
    (*sunAxis)[2] = -dir[2];
    v1 = (*sunAxis)[1] * (*sunAxis)[1] + (*sunAxis)[0] * (*sunAxis)[0];
    if (v1 >= 0.1000000014901161)
    {
        (*sunAxis)[6] = 0.0;
        (*sunAxis)[7] = 0.0;
        (*sunAxis)[8] = 1.0;
    }
    else
    {
        (*sunAxis)[6] = 1.0;
        (*sunAxis)[7] = 0.0;
        (*sunAxis)[8] = 0.0;
    }
    Vec3Cross(&(*sunAxis)[6], (const float *)sunAxis, &(*sunAxis)[3]);
    Vec3Normalize(&(*sunAxis)[3]);
    Vec3Cross((const float *)sunAxis, &(*sunAxis)[3], &(*sunAxis)[6]);
}

void __cdecl R_SunShadowMapBoundingPoly(
    const GfxSunShadowBoundingPoly *boundingPoly,
    float sampleSize,
    float (*polyInClipSpace)[2],
    int *pointIsNear)
{
    float v4; // [esp+Ch] [ebp-10h]
    float viewOrgOffset; // [esp+10h] [ebp-Ch]
    float viewOrgOffset_4; // [esp+14h] [ebp-8h]
    int pointIndex; // [esp+18h] [ebp-4h]

    v4 = 2.0 / (sampleSize * 1024.0);
    viewOrgOffset = v4 * boundingPoly->snapDelta[0];
    viewOrgOffset_4 = v4 * boundingPoly->snapDelta[1];
    for (pointIndex = 0; pointIndex < boundingPoly->pointCount; ++pointIndex)
    {
        (*polyInClipSpace)[2 * pointIndex] = boundingPoly->points[pointIndex][0] + viewOrgOffset;
        (*polyInClipSpace)[2 * pointIndex + 1] = boundingPoly->points[pointIndex][1] + viewOrgOffset_4;
        pointIsNear[pointIndex] = boundingPoly->pointIsNear[pointIndex];
    }
}

void __cdecl R_SetupSunShadowMaps(const GfxViewParms *viewParms, GfxSunShadow *sunShadow)
{
    float *origin; // [esp+18h] [ebp-138h]
    float sunOrigin[3]; // [esp+50h] [ebp-100h] BYREF
    unsigned int planeCount; // [esp+5Ch] [ebp-F4h]
    const GfxSunShadowProjection *sunProj; // [esp+60h] [ebp-F0h]
    float shadowSampleSize; // [esp+64h] [ebp-ECh]
    float snappedViewOrgInClipSpace[2][2]; // [esp+68h] [ebp-E8h] BYREF
    float partitionFraction[4]; // [esp+78h] [ebp-D8h] BYREF
    float nearClip; // [esp+88h] [ebp-C8h] BYREF
    int partitionIndex; // [esp+8Ch] [ebp-C4h]
    float boundingPolyClipSpacePlanes[9][4]; // [esp+90h] [ebp-C0h] BYREF
    float sunAxis[3][3]; // [esp+120h] [ebp-30h] BYREF
    float farClip; // [esp+144h] [ebp-Ch] BYREF
    GfxViewParms *shadowViewParms; // [esp+148h] [ebp-8h]
    GfxSunShadowPartition *partition; // [esp+14Ch] [ebp-4h]

    R_GetSunAxes(sunAxis);
    R_SetupSunShadowMapProjection(viewParms, sunAxis, sunShadow, snappedViewOrgInClipSpace, partitionFraction);
    sunOrigin[0] = -sunShadow->sunProj.viewMatrix[3][0];
    sunOrigin[1] = -sunShadow->sunProj.viewMatrix[3][1];
    sunOrigin[2] = -sunShadow->sunProj.viewMatrix[3][2];
    shadowSampleSize = sm_sunSampleSizeNear->current.value;
    Profile_Begin(67);
    R_GetSceneExtentsAlongDir(sunOrigin, sunAxis[0], &nearClip, &farClip);
    Profile_EndInternal(0);
    sunProj = &sunShadow->sunProj;
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        partition = &sunShadow->partition[partitionIndex];
        shadowViewParms = &partition->shadowViewParms;
        memset((unsigned __int8 *)partition, 0xB0u, 0x140u);
        origin = shadowViewParms->origin;
        shadowViewParms->origin[0] = -sunAxis[0][0];
        origin[1] = -sunAxis[0][1];
        origin[2] = -sunAxis[0][2];
        shadowViewParms->origin[3] = 0.0;
        AxisCopy(sunAxis, shadowViewParms->axis);
        memcpy(shadowViewParms, sunProj, 0x40u);
        R_SunShadowMapProjectionMatrix(
            snappedViewOrgInClipSpace[partitionIndex],
            shadowSampleSize,
            nearClip,
            farClip,
            shadowViewParms);
        planeCount = R_SunShadowMapClipSpaceClipPlanes(
            &partition->boundingPoly,
            partitionIndex,
            shadowSampleSize,
            boundingPolyClipSpacePlanes);
        R_SetupShadowSurfacesDpvs(shadowViewParms, boundingPolyClipSpacePlanes, planeCount, partitionIndex);
        shadowSampleSize = shadowSampleSize * rg.sunShadowPartitionRatio;
    }
    partition = sunShadow->partition;
    R_GetSunShadowLookupMatrix(
        &sunShadow->partition[0].shadowViewParms,
        sunProj,
        partitionFraction,
        &sunShadow->lookupMatrix);
}

void __cdecl R_GetSceneExtentsAlongDir(const float *origin, const float *forward, float *nearCap, float *farCap)
{
    float v4; // [esp+8h] [ebp-24h]
    float nearPlane; // [esp+Ch] [ebp-20h]
    float nearPlane_4; // [esp+10h] [ebp-1Ch]
    float nearPlane_12; // [esp+18h] [ebp-14h]
    unsigned __int8 nearPlane_16; // [esp+1Ch] [ebp-10h]
    unsigned __int8 nearPlane_17; // [esp+1Dh] [ebp-Fh]
    unsigned __int8 nearPlane_18; // [esp+1Eh] [ebp-Eh]
    float distMin; // [esp+24h] [ebp-8h]
    float distMax; // [esp+28h] [ebp-4h]

    nearPlane = *forward;
    nearPlane_4 = forward[1];
    nearPlane_12 = -origin[2];
    nearPlane_16 = SLODWORD(nearPlane) <= 0 ? 0 : 0xC;
    nearPlane_17 = SLODWORD(nearPlane_4) <= 0 ? 4 : 16;
    v4 = forward[2];
    nearPlane_18 = SLODWORD(v4) <= 0 ? 8 : 20;
    distMin = *(float *)((char *)&rgp.world->materialMemoryCount - nearPlane_18) * v4
        + *(float *)((char *)&rgp.world->maxs[2] - nearPlane_17) * nearPlane_4
        + *(float *)((char *)rgp.world->maxs - nearPlane_16) * nearPlane
        + nearPlane_12;
    distMax = *(float *)((char *)rgp.world->mins + nearPlane_18) * v4
        + *(float *)((char *)rgp.world->mins + nearPlane_17) * nearPlane_4
        + *(float *)((char *)rgp.world->mins + nearPlane_16) * nearPlane
        + nearPlane_12;
    if (distMax <= (double)distMin)
        MyAssertHandler(".\\r_sunshadow.cpp", 61, 0, "%s", "distMin < distMax");
    *nearCap = distMin;
    *farCap = distMax;
}

unsigned int __cdecl R_SunShadowMapClipSpaceClipPlanes(
    const GfxSunShadowBoundingPoly *boundingPoly,
    int partitionIndex,
    float sampleSize,
    float (*boundingPolyClipSpacePlanes)[4])
{
    float v5; // [esp+Ch] [ebp-A4h]
    float v6; // [esp+10h] [ebp-A0h]
    float v7; // [esp+14h] [ebp-9Ch]
    float v8; // [esp+18h] [ebp-98h]
    float *v9; // [esp+2Ch] [ebp-84h]
    unsigned int planeCount; // [esp+30h] [ebp-80h]
    int pointIsNear[10]; // [esp+34h] [ebp-7Ch] BYREF
    int pointIndex; // [esp+5Ch] [ebp-54h]
    float frustumBoundPolyInClipSpace[10][2]; // [esp+60h] [ebp-50h] BYREF

    R_SunShadowMapBoundingPoly(boundingPoly, sampleSize, frustumBoundPolyInClipSpace, pointIsNear);
    v9 = frustumBoundPolyInClipSpace[boundingPoly->pointCount];
    *v9 = frustumBoundPolyInClipSpace[0][0];
    v9[1] = frustumBoundPolyInClipSpace[0][1];
    pointIsNear[boundingPoly->pointCount] = pointIsNear[0];
    planeCount = 0;
    for (pointIndex = 0; pointIndex < boundingPoly->pointCount; ++pointIndex)
    {
        if (!partitionIndex || !rg.sunShadowFull || pointIsNear[pointIndex] || pointIsNear[pointIndex + 1])
        {
            (*boundingPolyClipSpacePlanes)[4 * planeCount] = frustumBoundPolyInClipSpace[pointIndex + 1][1]
                - frustumBoundPolyInClipSpace[pointIndex][1];
            (*boundingPolyClipSpacePlanes)[4 * planeCount + 1] = frustumBoundPolyInClipSpace[pointIndex][0]
                - frustumBoundPolyInClipSpace[pointIndex + 1][0];
            (*boundingPolyClipSpacePlanes)[4 * planeCount + 2] = 0.0;
            v8 = (*boundingPolyClipSpacePlanes)[4 * planeCount + 1] * frustumBoundPolyInClipSpace[pointIndex][1]
                + (*boundingPolyClipSpacePlanes)[4 * planeCount] * frustumBoundPolyInClipSpace[pointIndex][0];
            (*boundingPolyClipSpacePlanes)[4 * planeCount++ + 3] = -v8;
        }
    }
    if (partitionIndex && rg.sunShadowFull)
    {
        if (planeCount > 5)
            MyAssertHandler(".\\r_sunshadow.cpp", 186, 0, "%s\n\t(planeCount) = %i", "(planeCount <= 9 - 4)", planeCount);
        for (pointIndex = 0; pointIndex < 4; ++pointIndex)
        {
            v7 = ((float)g_shadowFrustumBound[pointIndex + 1][1] - (float)g_shadowFrustumBound[pointIndex][1])
                * rg.sunShadowmapScale;
            (*boundingPolyClipSpacePlanes)[4 * planeCount] = v7;
            v6 = ((float)g_shadowFrustumBound[pointIndex][0] - (float)g_shadowFrustumBound[pointIndex + 1][0])
                * rg.sunShadowmapScale;
            (*boundingPolyClipSpacePlanes)[4 * planeCount + 1] = v6;
            (*boundingPolyClipSpacePlanes)[4 * planeCount + 2] = 0.0;
            v5 = (*boundingPolyClipSpacePlanes)[4 * planeCount + 1] * (float)g_shadowFrustumBound[pointIndex][1]
                + (*boundingPolyClipSpacePlanes)[4 * planeCount] * (float)g_shadowFrustumBound[pointIndex][0];
            (*boundingPolyClipSpacePlanes)[4 * planeCount++ + 3] = -v5 * rg.sunShadowmapScale;
        }
    }
    return planeCount;
}

void __cdecl R_SunShadowMapProjectionMatrix(
    const float *snappedViewOrgInClipSpace,
    float shadowSampleSize,
    float nearClip,
    float farClip,
    GfxViewParms *shadowViewParms)
{
    float shadowSampleExtents; // [esp+0h] [ebp-4h]

    if (shadowSampleSize == 0.0)
        MyAssertHandler(".\\r_sunshadow.cpp", 230, 0, "%s", "shadowSampleSize");
    memset((unsigned __int8 *)&shadowViewParms->projectionMatrix, 0, sizeof(shadowViewParms->projectionMatrix));
    shadowSampleExtents = shadowSampleSize * 1024.0;
    shadowViewParms->projectionMatrix.m[0][0] = 2.0 / shadowSampleExtents;
    shadowViewParms->projectionMatrix.m[3][0] = *snappedViewOrgInClipSpace;
    shadowViewParms->projectionMatrix.m[1][1] = shadowViewParms->projectionMatrix.m[0][0];
    shadowViewParms->projectionMatrix.m[3][1] = snappedViewOrgInClipSpace[1];
    shadowViewParms->projectionMatrix.m[2][2] = 1.0 / (farClip - nearClip + 2.0);
    shadowViewParms->projectionMatrix.m[3][2] = -(nearClip - 1.0) * shadowViewParms->projectionMatrix.m[2][2];
    shadowViewParms->projectionMatrix.m[3][3] = 1.0;
    shadowViewParms->depthHackNearClip = shadowViewParms->projectionMatrix.m[3][2];
    R_SetupViewProjectionMatrices(shadowViewParms);
}

void __cdecl R_SetupSunShadowMapProjection(
    const GfxViewParms *viewParms,
    const float (*sunAxis)[3],
    GfxSunShadow *sunShadow,
    float (*snappedViewOrgInClipSpace)[2],
    float *partitionFraction)
{
    double v5; // st7
    double v6; // st7
    unsigned int v7; // [esp+18h] [ebp-29Ch]
    float v8; // [esp+1Ch] [ebp-298h]
    unsigned int v9; // [esp+38h] [ebp-27Ch]
    float v10; // [esp+3Ch] [ebp-278h]
    unsigned int v11; // [esp+58h] [ebp-25Ch]
    float v12; // [esp+5Ch] [ebp-258h]
    unsigned int sunShadowSize; // [esp+78h] [ebp-23Ch]
    float v14; // [esp+7Ch] [ebp-238h]
    float v15; // [esp+A4h] [ebp-210h]
    float v16; // [esp+B4h] [ebp-200h]
    bool v17; // [esp+B8h] [ebp-1FCh]
    float v18; // [esp+C4h] [ebp-1F0h]
    float v19; // [esp+C8h] [ebp-1ECh]
    float v20; // [esp+CCh] [ebp-1E8h]
    float v21; // [esp+D0h] [ebp-1E4h]
    float *shadowmapScale; // [esp+D4h] [ebp-1E0h]
    float v23; // [esp+D8h] [ebp-1DCh]
    float v24; // [esp+DCh] [ebp-1D8h]
    float v25; // [esp+E8h] [ebp-1CCh]
    float v26; // [esp+F4h] [ebp-1C0h]
    float v27; // [esp+100h] [ebp-1B4h]
    float v28; // [esp+10Ch] [ebp-1A8h]
    float v29; // [esp+110h] [ebp-1A4h]
    float v30; // [esp+114h] [ebp-1A0h]
    float v31; // [esp+11Ch] [ebp-198h]
    float v32; // [esp+120h] [ebp-194h]
    float *v33; // [esp+124h] [ebp-190h]
    float v34; // [esp+128h] [ebp-18Ch]
    float minsInSunProj[2][2]; // [esp+140h] [ebp-174h] BYREF
    float sizeInSunProj[2]; // [esp+150h] [ebp-164h]
    float frustumPoints[4][3]; // [esp+158h] [ebp-15Ch] BYREF
    float maxsInSunProj[2][2]; // [esp+188h] [ebp-12Ch] BYREF
    GfxSunShadowProjection *sunProj; // [esp+198h] [ebp-11Ch]
    float frustumPointsInSunProj[2][8][2]; // [esp+19Ch] [ebp-118h] BYREF
    int snappedViewOrgInTicks[2]; // [esp+220h] [ebp-94h]
    float viewOrgInTexSpace[2][2]; // [esp+228h] [ebp-8Ch] BYREF
    float shadowOrg[3]; // [esp+238h] [ebp-7Ch] BYREF
    float viewOrgInSunProj[2]; // [esp+244h] [ebp-70h] BYREF
    float sampleSizeFar; // [esp+24Ch] [ebp-68h]
    unsigned int farShadowBegin; // [esp+250h] [ebp-64h]
    GfxSunShadowPartition *partitionNear; // [esp+254h] [ebp-60h]
    float offset[2]; // [esp+258h] [ebp-5Ch]
    unsigned int pointIndex; // [esp+260h] [ebp-54h]
    unsigned int farShadowEnd; // [esp+264h] [ebp-50h]
    float frustumPoint[3]; // [esp+268h] [ebp-4Ch]
    int useShadowOffset; // [esp+274h] [ebp-40h]
    unsigned int farShadowMiddle; // [esp+278h] [ebp-3Ch]
    GfxSunShadowPartition *partitionFar; // [esp+27Ch] [ebp-38h]
    float sampleSizeNear; // [esp+280h] [ebp-34h]
    float scale; // [esp+284h] [ebp-30h]
    float maxSizeInSunProj; // [esp+288h] [ebp-2Ch]
    float shadowOrgInSunProj[2]; // [esp+28Ch] [ebp-28h]
    float farShadowSizeRatio; // [esp+294h] [ebp-20h]
    float scaleToFitUsable; // [esp+298h] [ebp-1Ch]
    float viewOrgInPixels[2][2]; // [esp+29Ch] [ebp-18h] BYREF
    float snappedViewOrgInSunProj[2]; // [esp+2ACh] [ebp-8h] BYREF

    viewOrgInSunProj[0] = -Vec3Dot(viewParms->origin, &(*sunAxis)[3]);
    viewOrgInSunProj[1] = Vec3Dot(viewParms->origin, &(*sunAxis)[6]);
    minsInSunProj[0][0] = FLT_MAX;
    minsInSunProj[0][1] = FLT_MAX;
    maxsInSunProj[0][0] = -FLT_MAX;
    maxsInSunProj[0][1] = -FLT_MAX;
    R_GetFrustumNearClipPoints(&viewParms->inverseViewProjectionMatrix, frustumPoints);
    for (pointIndex = 0; pointIndex < 4; ++pointIndex)
    {
        frustumPoint[0] = frustumPoints[pointIndex][0];
        frustumPoint[1] = frustumPoints[pointIndex][1];
        frustumPoint[2] = frustumPoints[pointIndex][2];
        v5 = Vec3Dot(frustumPoints[pointIndex], &(*sunAxis)[3]);
        frustumPointsInSunProj[0][pointIndex][0] = -v5;
        v6 = Vec3Dot(frustumPoints[pointIndex], &(*sunAxis)[6]);
        frustumPointsInSunProj[0][pointIndex][1] = v6;
        AddPointToBounds2D(frustumPointsInSunProj[0][pointIndex], minsInSunProj[0], maxsInSunProj[0]);
    }
    frustumPointsInSunProj[0][4][0] = viewOrgInSunProj[0];
    frustumPointsInSunProj[0][4][1] = viewOrgInSunProj[1];
    minsInSunProj[1][0] = minsInSunProj[0][0];
    minsInSunProj[1][1] = minsInSunProj[0][1];
    maxsInSunProj[1][0] = maxsInSunProj[0][0];
    maxsInSunProj[1][1] = maxsInSunProj[0][1];
    memcpy(frustumPointsInSunProj[1], frustumPointsInSunProj, 0x20u);
    for (pointIndex = 0; pointIndex < 4; ++pointIndex)
    {
        offset[0] = frustumPointsInSunProj[1][pointIndex][0] - viewOrgInSunProj[0];
        offset[1] = frustumPointsInSunProj[1][pointIndex][1] - viewOrgInSunProj[1];
        v33 = frustumPointsInSunProj[1][pointIndex + 4];
        v34 = 0.75 / rg.sunShadowPartitionRatio;
        *v33 = v34 * offset[0] + viewOrgInSunProj[0];
        v33[1] = v34 * offset[1] + viewOrgInSunProj[1];
        AddPointToBounds2D(frustumPointsInSunProj[1][pointIndex + 4], minsInSunProj[1], maxsInSunProj[1]);
    }
    AddPointToBounds2D(viewOrgInSunProj, minsInSunProj[0], maxsInSunProj[0]);
    sizeInSunProj[0] = maxsInSunProj[0][0] - minsInSunProj[0][0];
    sizeInSunProj[1] = maxsInSunProj[0][1] - minsInSunProj[0][1];
    v21 = sizeInSunProj[0] - sizeInSunProj[1];
    if (v21 < 0.0)
        v20 = sizeInSunProj[1];
    else
        v20 = sizeInSunProj[0];
    maxSizeInSunProj = v20;
    sampleSizeNear = sm_sunSampleSizeNear->current.value;
    sampleSizeFar = sampleSizeNear * rg.sunShadowPartitionRatio;
    scale = sampleSizeFar * rg.sunShadowmapScaleNum;
    v32 = viewOrgInSunProj[0] / scale;
    v19 = floor(v32);
    snappedViewOrgInTicks[0] = (int)v19;
    v31 = viewOrgInSunProj[1] / scale;
    v18 = floor(v31);
    snappedViewOrgInTicks[1] = (int)v18;
    snappedViewOrgInSunProj[0] = (double)snappedViewOrgInTicks[0] * scale;
    snappedViewOrgInSunProj[1] = (double)snappedViewOrgInTicks[1] * scale;
    scaleToFitUsable = 1023.0 / maxSizeInSunProj;
    farShadowBegin = (1024 - rg.sunShadowSize) >> 1;
    farShadowEnd = 1024 - farShadowBegin;
    farShadowMiddle = 512;
    farShadowSizeRatio = (double)rg.sunShadowSize / 1024.0;
    partitionNear = sunShadow->partition;
    partitionFar = &sunShadow->partition[1];
    shadowOrg[0] = sm_sunShadowCenter->current.value;
    shadowOrg[1] = sm_sunShadowCenter->current.vector[1];
    shadowOrg[2] = sm_sunShadowCenter->current.vector[2];
    v17 = 0.0 == shadowOrg[0] && 0.0 == shadowOrg[1] && 0.0 == shadowOrg[2];
    useShadowOffset = !v17;
    shadowOrgInSunProj[0] = -Vec3Dot(shadowOrg, &(*sunAxis)[3]);
    shadowOrgInSunProj[1] = Vec3Dot(shadowOrg, &(*sunAxis)[6]);
    v30 = 1024.0 / maxSizeInSunProj * sizeInSunProj[0] - EQUAL_EPSILON;
    v16 = ceil(v30);
    partitionNear->viewport.width = (unsigned __int16)(int)v16;
    v29 = 1024.0 / maxSizeInSunProj * sizeInSunProj[1] - EQUAL_EPSILON;
    v15 = ceil(v29);
    partitionNear->viewport.height = (unsigned __int16)(int)v15;
    if (viewOrgInSunProj[0] < (minsInSunProj[0][0] + maxsInSunProj[0][0]) * 0.5)
    {
        viewOrgInPixels[0][0] = (viewOrgInSunProj[0] - minsInSunProj[0][0]) * scaleToFitUsable + 1.0;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][0] = (double)farShadowMiddle + (viewOrgInSunProj[0] - shadowOrgInSunProj[0]) / sampleSizeFar;
            partitionFar->viewport.x = farShadowBegin;
        }
        else
        {
            viewOrgInPixels[1][0] = (double)(farShadowBegin + 1)
                + (viewOrgInSunProj[0] - minsInSunProj[0][0]) * scaleToFitUsable * farShadowSizeRatio;
            v27 = scaleToFitUsable * (minsInSunProj[1][0] - minsInSunProj[0][0]) + (double)farShadowBegin;
            v12 = floor(v27);
            if ((int)farShadowBegin < (int)v12)
                v11 = (int)v12;
            else
                v11 = farShadowBegin;
            partitionFar->viewport.x = v11;
        }
        partitionNear->viewport.x = 0;
        partitionFar->viewport.width = farShadowEnd - partitionFar->viewport.x;
    }
    else
    {
        viewOrgInPixels[0][0] = 1023.0 - (maxsInSunProj[0][0] - viewOrgInSunProj[0]) * scaleToFitUsable;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][0] = (double)farShadowMiddle - (viewOrgInSunProj[0] - shadowOrgInSunProj[0]) / sampleSizeFar;
            partitionFar->viewport.width = rg.sunShadowSize;
        }
        else
        {
            viewOrgInPixels[1][0] = (double)(farShadowEnd - 1)
                - (maxsInSunProj[0][0] - viewOrgInSunProj[0]) * scaleToFitUsable * farShadowSizeRatio;
            v28 = (double)farShadowEnd - scaleToFitUsable * (maxsInSunProj[0][0] - maxsInSunProj[1][0]);
            v14 = ceil(v28);
            if ((int)((int)v14 - farShadowBegin) < (int)rg.sunShadowSize)
                sunShadowSize = (int)v14 - farShadowBegin;
            else
                sunShadowSize = rg.sunShadowSize;
            partitionFar->viewport.width = sunShadowSize;
        }
        partitionNear->viewport.x = 1024 - partitionNear->viewport.width;
        partitionFar->viewport.x = farShadowBegin;
    }
    if (viewOrgInSunProj[1] > (minsInSunProj[0][1] + maxsInSunProj[0][1]) * 0.5)
    {
        viewOrgInPixels[0][1] = (maxsInSunProj[0][1] - viewOrgInSunProj[1]) * scaleToFitUsable + 1.0;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][1] = (double)farShadowMiddle + (viewOrgInSunProj[1] - shadowOrgInSunProj[1]) / sampleSizeFar;
            partitionFar->viewport.y = farShadowBegin;
        }
        else
        {
            viewOrgInPixels[1][1] = (double)(farShadowBegin + 1)
                + (maxsInSunProj[0][1] - viewOrgInSunProj[1]) * scaleToFitUsable * farShadowSizeRatio;
            v25 = scaleToFitUsable * (maxsInSunProj[0][1] - maxsInSunProj[1][1]) + (double)farShadowBegin;
            v8 = floor(v25);
            if ((int)farShadowBegin < (int)v8)
                v7 = (int)v8;
            else
                v7 = farShadowBegin;
            partitionFar->viewport.y = v7;
        }
        partitionNear->viewport.y = 0;
        partitionFar->viewport.height = farShadowEnd - partitionFar->viewport.y;
    }
    else
    {
        viewOrgInPixels[0][1] = 1023.0 - (viewOrgInSunProj[1] - minsInSunProj[0][1]) * scaleToFitUsable;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][1] = (double)farShadowMiddle - (viewOrgInSunProj[1] - shadowOrgInSunProj[1]) / sampleSizeFar;
            partitionFar->viewport.height = rg.sunShadowSize;
        }
        else
        {
            viewOrgInPixels[1][1] = (double)(farShadowEnd - 1)
                - (viewOrgInSunProj[1] - minsInSunProj[0][1]) * scaleToFitUsable * farShadowSizeRatio;
            v26 = (double)farShadowEnd - scaleToFitUsable * (minsInSunProj[1][1] - minsInSunProj[0][1]);
            v10 = ceil(v26);
            if ((int)((int)v10 - farShadowBegin) < (int)rg.sunShadowSize)
                v9 = (int)v10 - farShadowBegin;
            else
                v9 = rg.sunShadowSize;
            partitionFar->viewport.height = v9;
        }
        partitionNear->viewport.y = 1024 - partitionNear->viewport.height;
        partitionFar->viewport.y = farShadowBegin;
    }
    if (!rg.sunShadowFull)
        partitionFar->viewport = partitionNear->viewport;
    R_GetSunShadowMapPartitionViewOrgInTextureSpace(
        viewOrgInPixels[0],
        viewOrgInSunProj,
        snappedViewOrgInSunProj,
        sampleSizeNear,
        viewOrgInTexSpace[0]);
    R_GetSunShadowMapPartitionViewOrgInTextureSpace(
        viewOrgInPixels[1],
        viewOrgInSunProj,
        snappedViewOrgInSunProj,
        sampleSizeFar,
        viewOrgInTexSpace[1]);
    sunProj = &sunShadow->sunProj;
    sunShadow->sunProj.switchPartition[3] = 1.0 / rg.sunShadowPartitionRatio;
    sunProj->switchPartition[0] = viewOrgInTexSpace[1][0] - viewOrgInTexSpace[0][0] * sunProj->switchPartition[3];
    sunProj->switchPartition[1] = (viewOrgInTexSpace[1][1] - viewOrgInTexSpace[0][1] * sunProj->switchPartition[3] + 1.0)
        * 0.5;
    sunProj->switchPartition[2] = 0.0;
    shadowmapScale = sunProj->shadowmapScale;
    v23 = 16.0 / rg.sunShadowmapScale;
    v24 = 32.0 / rg.sunShadowmapScale;
    sunProj->shadowmapScale[0] = v23;
    shadowmapScale[1] = v24;
    shadowmapScale[2] = 0.0;
    shadowmapScale[3] = 0.0;
    (*snappedViewOrgInClipSpace)[0] = viewOrgInTexSpace[0][0] + viewOrgInTexSpace[0][0] - 1.0;
    (*snappedViewOrgInClipSpace)[1] = 1.0 - (viewOrgInTexSpace[0][1] + viewOrgInTexSpace[0][1]);
    (*snappedViewOrgInClipSpace)[2] = viewOrgInTexSpace[1][0] + viewOrgInTexSpace[1][0] - 1.0;
    (*snappedViewOrgInClipSpace)[3] = 1.0 - (viewOrgInTexSpace[1][1] + viewOrgInTexSpace[1][1]);
    R_SetupSunShadowMapViewMatrix(snappedViewOrgInSunProj, sunAxis, sunProj);
    R_SetupSunShadowBoundingPoly(
        frustumPointsInSunProj[0],
        viewOrgInSunProj,
        snappedViewOrgInSunProj,
        maxSizeInSunProj,
        (const float *)snappedViewOrgInClipSpace,
        &partitionNear->boundingPoly,
        5u);
    R_SetupSunShadowBoundingPoly(
        frustumPointsInSunProj[1],
        viewOrgInSunProj,
        snappedViewOrgInSunProj,
        maxSizeInSunProj,
        &(*snappedViewOrgInClipSpace)[2],
        &partitionFar->boundingPoly,
        8u);
    R_SetupSunShadowMapPartitionFraction(viewParms, scaleToFitUsable, sunProj, partitionFraction);
    R_SetupNearRegionPlane(partitionFraction);
}

void __cdecl R_GetFrustumNearClipPoints(const GfxMatrix *invViewProjMtx, float (*frustumPoints)[3])
{
    R_ClipSpaceToWorldSpace(invViewProjMtx, g_clipSpacePoints, 4, frustumPoints);
}

void __cdecl R_ClipSpaceToWorldSpace(
    const GfxMatrix *invViewProjMtx,
    const float (*clipSpacePoints)[3],
    int pointCount,
    float (*worldSpacePoints)[3])
{
    float xyz; // [esp+Ch] [ebp-28h]
    float xyz_4; // [esp+10h] [ebp-24h]
    float xyz_8; // [esp+14h] [ebp-20h]
    float homogenousPoint[4]; // [esp+18h] [ebp-1Ch] BYREF
    int pointIndex; // [esp+28h] [ebp-Ch]
    int term; // [esp+2Ch] [ebp-8h]
    float scale; // [esp+30h] [ebp-4h]

    for (pointIndex = 0; pointIndex < pointCount; ++pointIndex)
    {
        xyz = (*clipSpacePoints)[3 * pointIndex];
        xyz_4 = (*clipSpacePoints)[3 * pointIndex + 1];
        xyz_8 = (*clipSpacePoints)[3 * pointIndex + 2];
        for (term = 0; term < 4; ++term)
            homogenousPoint[term] = xyz * invViewProjMtx->m[0][term]
            + xyz_4 * invViewProjMtx->m[1][term]
                + xyz_8 * invViewProjMtx->m[2][term]
                    + invViewProjMtx->m[3][term];
                    if (homogenousPoint[3] <= 0.0)
                        MyAssertHandler(".\\r_sunshadow.cpp", 97, 0, "%s", "homogenousPoint[3] > 0");
                    scale = 1.0 / homogenousPoint[3];
                    Vec3Scale(homogenousPoint, scale, &(*worldSpacePoints)[3 * pointIndex]);
    }
}

void __cdecl R_SetupSunShadowBoundingPoly(
    float (*frustumPointsInSunProj)[2],
    const float *viewOrgInSunProj,
    const float *snappedViewOrgInSunProj,
    float maxSizeInSunProj,
    const float *snappedViewOrgInClipSpace,
    GfxSunShadowBoundingPoly *boundingPoly,
    unsigned int pointCount)
{
    unsigned int nearPointIndex; // [esp+Ch] [ebp-94h]
    float scaleToClipSpace; // [esp+10h] [ebp-90h]
    int pointIndex; // [esp+14h] [ebp-8Ch]
    float tempFrustumPointsInSunProj[8][2]; // [esp+18h] [ebp-88h] BYREF
    float frustumBoundingPolyInSunProj[9][2]; // [esp+58h] [ebp-48h] BYREF

    memcpy((unsigned __int8 *)tempFrustumPointsInSunProj, (unsigned __int8 *)frustumPointsInSunProj, 8 * pointCount);
    boundingPoly->pointCount = Com_ConvexHull(tempFrustumPointsInSunProj, pointCount, frustumBoundingPolyInSunProj);
    if (boundingPoly->pointCount < 3 || boundingPoly->pointCount > 9)
        MyAssertHandler(
            ".\\r_sunshadow.cpp",
            121,
            1,
            "boundingPoly->pointCount not in [3, R_SUNSHADOW_SIDE_PLANES_MAX]\n\t%i not in [%i, %i]",
            boundingPoly->pointCount,
            3,
            9);
    scaleToClipSpace = 2.0 / maxSizeInSunProj;
    for (pointIndex = 0; pointIndex < boundingPoly->pointCount; ++pointIndex)
    {
        boundingPoly->points[pointIndex][0] = (frustumBoundingPolyInSunProj[pointIndex][0] - *viewOrgInSunProj)
            * scaleToClipSpace
            + *snappedViewOrgInClipSpace;
        boundingPoly->points[pointIndex][1] = (frustumBoundingPolyInSunProj[pointIndex][1] - viewOrgInSunProj[1])
            * scaleToClipSpace
            + snappedViewOrgInClipSpace[1];
        boundingPoly->pointIsNear[pointIndex] = 0;
        for (nearPointIndex = 4; nearPointIndex < pointCount; ++nearPointIndex)
        {
            if ((*frustumPointsInSunProj)[2 * nearPointIndex] == frustumBoundingPolyInSunProj[pointIndex][0]
                && (*frustumPointsInSunProj)[2 * nearPointIndex + 1] == frustumBoundingPolyInSunProj[pointIndex][1])
            {
                boundingPoly->pointIsNear[pointIndex] = 1;
                break;
            }
        }
    }
    boundingPoly->snapDelta[0] = *viewOrgInSunProj - *snappedViewOrgInSunProj;
    boundingPoly->snapDelta[1] = viewOrgInSunProj[1] - snappedViewOrgInSunProj[1];
}

void __cdecl R_SetupSunShadowMapViewMatrix(
    const float *snappedViewOrgInSunProj,
    const float (*sunAxis)[3],
    GfxSunShadowProjection *sunProj)
{
    sunProj->viewMatrix[0][0] = -(*sunAxis)[3];
    sunProj->viewMatrix[1][0] = -(*sunAxis)[4];
    sunProj->viewMatrix[2][0] = -(*sunAxis)[5];
    sunProj->viewMatrix[3][0] = -*snappedViewOrgInSunProj;
    sunProj->viewMatrix[0][1] = (*sunAxis)[6];
    sunProj->viewMatrix[1][1] = (*sunAxis)[7];
    sunProj->viewMatrix[2][1] = (*sunAxis)[8];
    sunProj->viewMatrix[3][1] = -snappedViewOrgInSunProj[1];
    sunProj->viewMatrix[0][2] = (*sunAxis)[0];
    sunProj->viewMatrix[1][2] = (*sunAxis)[1];
    sunProj->viewMatrix[2][2] = (*sunAxis)[2];
    sunProj->viewMatrix[3][2] = 0.0;
    sunProj->viewMatrix[0][3] = 0.0;
    sunProj->viewMatrix[1][3] = 0.0;
    sunProj->viewMatrix[2][3] = 0.0;
    sunProj->viewMatrix[3][3] = 1.0;
}

void __cdecl R_SetupSunShadowMapPartitionFraction(
    const GfxViewParms *viewParms,
    float scaleToFitUsable,
    GfxSunShadowProjection *sunProj,
    float *partitionFraction)
{
    float scale; // [esp+8h] [ebp-Ch]
    float zNear; // [esp+Ch] [ebp-8h]
    float endOfNearFrustum; // [esp+10h] [ebp-4h]

    zNear = viewParms->zNear;
    if (zNear == 0.0)
        MyAssertHandler(".\\r_sunshadow.cpp", 268, 0, "%s", "zNear");
    endOfNearFrustum = zNear * sm_sunSampleSizeNear->current.value * scaleToFitUsable;
    scale = 1.0 / endOfNearFrustum;
    Vec3Scale(viewParms->axis[0], scale, partitionFraction);
    partitionFraction[3] = -Vec3Dot(viewParms->origin, partitionFraction);
}

void __cdecl R_GetSunShadowMapPartitionViewOrgInTextureSpace(
    const float *viewOrgInPixels,
    const float *viewOrgInSunProj,
    const float *snappedViewOrgInSunProj,
    float sampleSize,
    float *viewOrgInTexSpace)
{
    float v5; // [esp+8h] [ebp-18h]
    float v6; // [esp+Ch] [ebp-14h]
    float snappedViewOrgInPixels; // [esp+10h] [ebp-10h]
    float snappedViewOrgInPixels_4; // [esp+14h] [ebp-Ch]

    snappedViewOrgInPixels = (*snappedViewOrgInSunProj - *viewOrgInSunProj) / sampleSize + *viewOrgInPixels;
    snappedViewOrgInPixels_4 = viewOrgInPixels[1] - (snappedViewOrgInSunProj[1] - viewOrgInSunProj[1]) / sampleSize;
    v6 = floor(snappedViewOrgInPixels);
    v5 = floor(snappedViewOrgInPixels_4);
    *viewOrgInTexSpace = v6 * 0.0009765625;
    viewOrgInTexSpace[1] = v5 * 0.0009765625;
}

void __cdecl R_SetupNearRegionPlane(const float *partitionFraction)
{
    float size; // [esp+24h] [ebp-14h]
    DpvsPlane *shadowFarPlane; // [esp+28h] [ebp-10h]
    unsigned int partitionIndex; // [esp+2Ch] [ebp-Ch]
    float scale; // [esp+30h] [ebp-8h]
    float scalea; // [esp+30h] [ebp-8h]
    float scaleb; // [esp+30h] [ebp-8h]
    float length; // [esp+34h] [ebp-4h]
    float lengtha; // [esp+34h] [ebp-4h]
    float lengthb; // [esp+34h] [ebp-4h]

    scene.shadowNearPlane[0].coeffs[0] = *partitionFraction;
    scene.shadowNearPlane[0].coeffs[1] = partitionFraction[1];
    scene.shadowNearPlane[0].coeffs[2] = partitionFraction[2];
    scene.shadowNearPlane[0].coeffs[3] = partitionFraction[3];
    length = Vec3Length(scene.shadowNearPlane[0].coeffs);
    if (length <= 0.0)
        MyAssertHandler(".\\r_sunshadow.cpp", 306, 0, "%s", "length > 0");
    scale = 1.0 / length;
    Vec4Scale(scene.shadowNearPlane[0].coeffs, scale, scene.shadowNearPlane[0].coeffs);
    scene.shadowNearPlane[0].side[0] = SLODWORD(scene.shadowNearPlane[0].coeffs[0]) <= 0 ? 0 : 0xC;
    scene.shadowNearPlane[0].side[1] = COERCE_INT(scene.shadowNearPlane[0].coeffs[1]) <= 0 ? 4 : 16;
    scene.shadowNearPlane[0].side[2] = COERCE_INT(scene.shadowNearPlane[0].coeffs[2]) <= 0 ? 8 : 20;
    scene.shadowNearPlane[1].coeffs[0] = *partitionFraction;
    scene.shadowNearPlane[1].coeffs[1] = partitionFraction[1];
    scene.shadowNearPlane[1].coeffs[2] = partitionFraction[2];
    scene.shadowNearPlane[1].coeffs[3] = partitionFraction[3];
    scene.shadowNearPlane[1].coeffs[3] = scene.shadowNearPlane[1].coeffs[3] - 0.75;
    lengtha = Vec3Length(scene.shadowNearPlane[1].coeffs);
    if (lengtha <= 0.0)
        MyAssertHandler(".\\r_sunshadow.cpp", 316, 0, "%s", "length > 0");
    scalea = 1.0 / lengtha;
    Vec4Scale(scene.shadowNearPlane[1].coeffs, scalea, scene.shadowNearPlane[1].coeffs);
    scene.shadowNearPlane[1].side[0] = SLODWORD(scene.shadowNearPlane[1].coeffs[0]) <= 0 ? 0 : 0xC;
    scene.shadowNearPlane[1].side[1] = COERCE_INT(scene.shadowNearPlane[1].coeffs[1]) <= 0 ? 4 : 16;
    scene.shadowNearPlane[1].side[2] = COERCE_INT(scene.shadowNearPlane[1].coeffs[2]) <= 0 ? 8 : 20;
    size = 1.0;
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        shadowFarPlane = &scene.shadowFarPlane[partitionIndex];
        shadowFarPlane->coeffs[0] = *partitionFraction;
        shadowFarPlane->coeffs[1] = partitionFraction[1];
        shadowFarPlane->coeffs[2] = partitionFraction[2];
        shadowFarPlane->coeffs[3] = partitionFraction[3];
        shadowFarPlane->coeffs[3] = shadowFarPlane->coeffs[3] - size;
        lengthb = Vec3Length(shadowFarPlane->coeffs);
        if (lengthb <= 0.0)
            MyAssertHandler(".\\r_sunshadow.cpp", 329, 0, "%s", "length > 0");
        scaleb = -1.0 / lengthb;
        Vec4Scale(shadowFarPlane->coeffs, scaleb, shadowFarPlane->coeffs);
        shadowFarPlane->side[0] = COERCE_INT(shadowFarPlane->coeffs[0]) <= 0 ? 0 : 0xC;
        shadowFarPlane->side[1] = COERCE_INT(shadowFarPlane->coeffs[1]) <= 0 ? 4 : 16;
        shadowFarPlane->side[2] = COERCE_INT(shadowFarPlane->coeffs[2]) <= 0 ? 8 : 20;
        size = size * rg.sunShadowPartitionRatio;
    }
}

void __cdecl R_GetSunShadowLookupMatrix(
    const GfxViewParms *shadowViewParms,
    const GfxSunShadowProjection *sunProj,
    const float *partitionFraction,
    GfxMatrix *lookupMatrix)
{
    float x0; // [esp+0h] [ebp-20h]
    float xScale; // [esp+4h] [ebp-1Ch]
    float x1; // [esp+8h] [ebp-18h]
    float xShift; // [esp+Ch] [ebp-14h]
    float yShift; // [esp+10h] [ebp-10h]
    float y1; // [esp+14h] [ebp-Ch]
    float y0; // [esp+18h] [ebp-8h]
    float yScale; // [esp+1Ch] [ebp-4h]

    x0 = (float)0.0 + 0.00048828125;
    x1 = (float)1.0 + 0.00048828125;
    y0 = (float)0.5 + 0.000244140625;
    y1 = (float)0.0 + 0.000244140625;
    xScale = (x1 - x0) * 0.5;
    xShift = (x1 + x0) * 0.5;
    yScale = (y1 - y0) * 0.5;
    yShift = (y1 + y0) * 0.5;
    lookupMatrix->m[0][0] = shadowViewParms->viewProjectionMatrix.m[0][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[0][3] * xShift;
    lookupMatrix->m[1][0] = shadowViewParms->viewProjectionMatrix.m[1][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[1][3] * xShift;
    lookupMatrix->m[2][0] = shadowViewParms->viewProjectionMatrix.m[2][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[2][3] * xShift;
    lookupMatrix->m[3][0] = shadowViewParms->viewProjectionMatrix.m[3][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[3][3] * xShift;
    lookupMatrix->m[0][1] = shadowViewParms->viewProjectionMatrix.m[0][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[0][3] * yShift;
    lookupMatrix->m[1][1] = shadowViewParms->viewProjectionMatrix.m[1][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[1][3] * yShift;
    lookupMatrix->m[2][1] = shadowViewParms->viewProjectionMatrix.m[2][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[2][3] * yShift;
    lookupMatrix->m[3][1] = shadowViewParms->viewProjectionMatrix.m[3][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[3][3] * yShift;
    lookupMatrix->m[0][2] = shadowViewParms->viewProjectionMatrix.m[0][2];
    lookupMatrix->m[1][2] = shadowViewParms->viewProjectionMatrix.m[1][2];
    lookupMatrix->m[2][2] = shadowViewParms->viewProjectionMatrix.m[2][2];
    lookupMatrix->m[3][2] = shadowViewParms->viewProjectionMatrix.m[3][2];
    lookupMatrix->m[0][3] = *partitionFraction;
    lookupMatrix->m[1][3] = partitionFraction[1];
    lookupMatrix->m[2][3] = partitionFraction[2];
    lookupMatrix->m[3][3] = partitionFraction[3];
}

void __cdecl R_SunShadowMaps()
{
    unsigned int oldViewIndex; // [esp+30h] [ebp-Ch]
    int partitionIndex; // [esp+34h] [ebp-8h]
    unsigned int viewIndex; // [esp+38h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_sunshadow.cpp", 656, 0, "%s", "rgp.world");
    CL_ResetStats_f();
    Profile_Begin(76);
    oldViewIndex = R_SetVisData(0);
    shadowGlob.defaultShadowCasterTechnique = Material_GetTechnique(
        rgp.depthPrepassMaterial,
        gfxMetrics.shadowmapBuildTechType);
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        viewIndex = partitionIndex + 1;
        if (partitionIndex == -1 || viewIndex > 2)
            MyAssertHandler(
                ".\\r_sunshadow.cpp",
                674,
                0,
                "%s\n\t(viewIndex) = %i",
                "((viewIndex >= SCENE_VIEW_SUNSHADOW_0) && (viewIndex <= SCENE_VIEW_SUNSHADOW_1))",
                viewIndex);
        R_SetVisData(viewIndex);
        Profile_Begin(72);
        R_AddWorldSurfacesFrustumOnly();
        Profile_EndInternal(0);
    }
    R_SetVisData(oldViewIndex);
    Profile_EndInternal(0);
}

void __cdecl R_MergeAndEmitSunShadowMapsSurfs(GfxViewInfo *viewInfo)
{
    float *dir; // [esp+20h] [ebp-2Ch]
    int firstDrawSurf; // [esp+38h] [ebp-14h]
    GfxDrawSurfListInfo *info; // [esp+3Ch] [ebp-10h]
    unsigned int partitionIndex; // [esp+40h] [ebp-Ch]
    GfxSunShadow *sunShadow; // [esp+44h] [ebp-8h]

    sunShadow = &viewInfo->sunShadow;
    Profile_Begin(75);
    CL_ResetStats_f();
    if (frontEndDataOut->sunLight.type != 1)
        MyAssertHandler(".\\r_sunshadow.cpp", 703, 0, "%s", "frontEndDataOut->sunLight.type == GFX_LIGHT_TYPE_DIR");
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        info = &sunShadow->partition[partitionIndex].info;
        R_InitDrawSurfListInfo(info);
        info->baseTechType = gfxMetrics.shadowmapBuildTechType;
        info->viewInfo = viewInfo;
        dir = frontEndDataOut->sunLight.dir;
        info->viewOrigin[0] = frontEndDataOut->sunLight.dir[0];
        info->viewOrigin[1] = dir[1];
        info->viewOrigin[2] = dir[2];
        info->viewOrigin[3] = 0.0;
        if (sunShadow->partition[partitionIndex].info.cameraView)
            MyAssertHandler(".\\r_sunshadow.cpp", 723, 0, "%s", "!info->cameraView");
        firstDrawSurf = frontEndDataOut->drawSurfCount;
        R_MergeAndEmitDrawSurfLists(3 * partitionIndex + 15, 1u);
        R_MergeAndEmitDrawSurfLists(3 * partitionIndex + 16, 2u);
        sunShadow->partition[partitionIndex].info.drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
        sunShadow->partition[partitionIndex].info.drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
        sunShadow->partition[partitionIndex].partitionIndex = partitionIndex;
    }
    Profile_EndInternal(0);
}

