#include "r_sunshadow.h"
#include "r_dvars.h"
#include "r_init.h"
#include "r_dpvs.h"
#include "r_pretess.h"
#include <universal/com_convexhull.h>
#include <universal/profile.h>
#include "r_drawsurf.h"

struct ShadowGlobals // sizeof=0x4
{                                       // ...
    const MaterialTechnique *defaultShadowCasterTechnique; // ...
};

static ShadowGlobals shadowGlob;

static const float g_shadowFrustumBound[5][2] =
{
    { -1.0f, -1.0f },
    { -1.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.0f, -1.0f },
    {-1.0f, -1.0f }
};

static const float g_clipSpacePoints[4][3] =
{
    { -1.0f, -1.0f, 0.0f },
    { 1.0f, -1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    {-1.0f, 1.0f, 0.0f }
};

// LWSS: separate from the one in r_shadowcookie
static void __cdecl R_GetSunAxes2(float (*sunAxis)[3][3])
{
    float v1; // [esp+0h] [ebp-1Ch]

    iassert(frontEndDataOut);
    iassert(frontEndDataOut->sunLight.type == GFX_LIGHT_TYPE_DIR);

    (*sunAxis)[0][0] = -frontEndDataOut->sunLight.dir[0];
    (*sunAxis)[0][1] = -frontEndDataOut->sunLight.dir[1];
    (*sunAxis)[0][2] = -frontEndDataOut->sunLight.dir[2];

    v1 = (*sunAxis)[0][1] * (*sunAxis)[0][1] + (*sunAxis)[0][0] * (*sunAxis)[0][0];

    if (v1 >= 0.1f)
    {
        (*sunAxis)[2][0] = 0.0f;
        (*sunAxis)[2][1] = 0.0f;
        (*sunAxis)[2][2] = 1.0f;
    }
    else
    {
        (*sunAxis)[2][0] = 1.0f;
        (*sunAxis)[2][1] = 0.0f;
        (*sunAxis)[2][2] = 0.0f;
    }
    Vec3Cross((*sunAxis)[2], (const float *)sunAxis, (*sunAxis)[1]);
    Vec3Normalize((*sunAxis)[1]);
    Vec3Cross((const float *)sunAxis, (*sunAxis)[1], (*sunAxis)[2]);
}

void __cdecl R_SunShadowMapBoundingPoly(
    const GfxSunShadowBoundingPoly *boundingPoly,
    float sampleSize,
    float (*polyInClipSpace)[9][2],
    int *pointIsNear)
{
    float viewOrgOffset[2]; // [esp+10h] [ebp-Ch]
    int pointIndex; // [esp+18h] [ebp-4h]

    // (2 / (sampleSize * 1024)) converts from texel offset to clip space [-1..1]
    // boundingPoly->snapDelta stores how much the sun projection was shifted
    viewOrgOffset[0] = 2.0f / (sampleSize * 1024.0f) * boundingPoly->snapDelta[0];
    viewOrgOffset[1] = 2.0f / (sampleSize * 1024.0f) * boundingPoly->snapDelta[1];

    // Convert all boundingpoly points into clipspace
    for (pointIndex = 0; pointIndex < boundingPoly->pointCount; ++pointIndex)
    {
        (*polyInClipSpace)[pointIndex][0] = boundingPoly->points[pointIndex][0] + viewOrgOffset[0];
        (*polyInClipSpace)[pointIndex][1] = boundingPoly->points[pointIndex][1] + viewOrgOffset[1];
        pointIsNear[pointIndex] = boundingPoly->pointIsNear[pointIndex];
    }
}

static unsigned int __cdecl R_SunShadowMapClipSpaceClipPlanes(
    const GfxSunShadowBoundingPoly *boundingPoly,
    int partitionIndex,
    float sampleSize,
    float (*boundingPolyClipSpacePlanes)[9][4])
{
    float v5; // [esp+Ch] [ebp-A4h]
    float v8; // [esp+18h] [ebp-98h]
    unsigned int planeCount; // [esp+30h] [ebp-80h]
    int pointIsNear[10]; // [esp+34h] [ebp-7Ch] BYREF
    float frustumBoundPolyInClipSpace[10][2]; // [esp+60h] [ebp-50h] BYREF
    //float frustumBoundPolyInClipSpace[9][2]; // [esp+60h] [ebp-50h] BYREF

    // Convert the sunspace boundingPoly to shadowmap clip space and get an array of points that are in the near plane
    R_SunShadowMapBoundingPoly(boundingPoly, sampleSize, (float (*)[9][2])&frustumBoundPolyInClipSpace, pointIsNear);

    // Final iteration for the below loop
    frustumBoundPolyInClipSpace[boundingPoly->pointCount][0] = frustumBoundPolyInClipSpace[0][0];
    frustumBoundPolyInClipSpace[boundingPoly->pointCount][1] = frustumBoundPolyInClipSpace[0][1];
    pointIsNear[boundingPoly->pointCount] = pointIsNear[0];

    planeCount = 0;

    // Build the near planes with the poly edges 
    for (int i = 0; i < boundingPoly->pointCount; ++i)
    {
        if (!partitionIndex || !rg.sunShadowFull || pointIsNear[i] || pointIsNear[i + 1])
        {
            (*boundingPolyClipSpacePlanes)[planeCount][0] = frustumBoundPolyInClipSpace[i + 1][1] - frustumBoundPolyInClipSpace[i][1];
            (*boundingPolyClipSpacePlanes)[planeCount][1] = frustumBoundPolyInClipSpace[i][0] - frustumBoundPolyInClipSpace[i + 1][0];
            (*boundingPolyClipSpacePlanes)[planeCount][2] = 0.0f; // Z not used

            v8 = (*boundingPolyClipSpacePlanes)[planeCount][1] * frustumBoundPolyInClipSpace[i][1] + (*boundingPolyClipSpacePlanes)[planeCount][0] * frustumBoundPolyInClipSpace[i][0];
            (*boundingPolyClipSpacePlanes)[planeCount][3] = -v8;
            planeCount++;
        }
    }

    if (partitionIndex && rg.sunShadowFull)
    {
        iassert(planeCount <= 9 - 4);

        for (int i = 0; i < 4; ++i)
        {
            (*boundingPolyClipSpacePlanes)[planeCount][0] = (g_shadowFrustumBound[i + 1][1] - g_shadowFrustumBound[i][1]) * rg.sunShadowmapScale;
            (*boundingPolyClipSpacePlanes)[planeCount][1] = (g_shadowFrustumBound[i][0] - g_shadowFrustumBound[i + 1][0]) * rg.sunShadowmapScale;;
            (*boundingPolyClipSpacePlanes)[planeCount][2] = 0.0; // Z not used
            v5 = (*boundingPolyClipSpacePlanes)[planeCount][1] * g_shadowFrustumBound[i][1] + (*boundingPolyClipSpacePlanes)[planeCount][0] * g_shadowFrustumBound[i][0];
            (*boundingPolyClipSpacePlanes)[planeCount][3] = -v5 * rg.sunShadowmapScale;
            planeCount++;
        }
    }

    return planeCount;
}

void __cdecl R_SetupSunShadowMaps(const GfxViewParms *viewParms, GfxSunShadow *sunShadow)
{
    float sunOrigin[3]; // [esp+50h] [ebp-100h] BYREF
    unsigned int planeCount; // [esp+5Ch] [ebp-F4h]
    float shadowSampleSize; // [esp+64h] [ebp-ECh]
    float snappedViewOrgInClipSpace[2][2]; // [esp+68h] [ebp-E8h] BYREF
    float partitionFraction[4]; // [esp+78h] [ebp-D8h] BYREF
    float nearClip; // [esp+88h] [ebp-C8h] BYREF
    float boundingPolyClipSpacePlanes[9][4]; // [esp+90h] [ebp-C0h] BYREF
    float sunAxis[3][3]; // [esp+120h] [ebp-30h] BYREF
    float farClip; // [esp+144h] [ebp-Ch] BYREF
    GfxViewParms *shadowViewParms; // [esp+148h] [ebp-8h]
    GfxSunShadowPartition *partition; // [esp+14Ch] [ebp-4h]

    // Get the 3 Sun axes [forward, right, up]
    R_GetSunAxes2(&sunAxis);

    // Build the sun shadow projection
    // Snaps shadow projection to texel grid and returns partition Fractions
    R_SetupSunShadowMapProjection(viewParms, &sunAxis, sunShadow, &snappedViewOrgInClipSpace, partitionFraction);

    // Get sun origin from viewmatrix
    sunOrigin[0] = -sunShadow->sunProj.viewMatrix[3][0];
    sunOrigin[1] = -sunShadow->sunProj.viewMatrix[3][1];
    sunOrigin[2] = -sunShadow->sunProj.viewMatrix[3][2];

    // Find the near and far clip limits of the cascade range
    {
        PROF_SCOPED("R_BoundScene");
        R_GetSceneExtentsAlongDir(sunOrigin, sunAxis[0], &nearClip, &farClip);
    }

    shadowSampleSize = sm_sunSampleSizeNear->current.value;

    // Iterate over each shadow parition (0 = near, 1 = far)
    for (int partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        partition = &sunShadow->partition[partitionIndex];

        shadowViewParms = &partition->shadowViewParms;

        // Init with 0xB0's (AKA Bobo's, this is a common cod4 default memory assignment)
        memset(shadowViewParms, 0xB0u, sizeof(GfxViewParms));

        // Abuse shadowViewParms->origin, setting it to the "backwards" vector of the sun (looking into the sun)
        shadowViewParms->origin[0] = -sunAxis[0][0];
        shadowViewParms->origin[1] = -sunAxis[0][1];
        shadowViewParms->origin[2] = -sunAxis[0][2];
        shadowViewParms->origin[3] = 0.0f;

        // Copy sunAxis to shadowViewParms->axis
        AxisCopy(sunAxis, shadowViewParms->axis);

        // Copy the global sun projection view matrix
        memcpy(&shadowViewParms->viewMatrix, sunShadow->sunProj.viewMatrix, sizeof(GfxMatrix));

        // Compute Partition's projection matrix (shadowViewParms->projectionMatrix)
        R_SunShadowMapProjectionMatrix(
            snappedViewOrgInClipSpace[partitionIndex],
            shadowSampleSize,
            nearClip,
            farClip,
            shadowViewParms);

        // Build clipSpace bounding planes
        planeCount = R_SunShadowMapClipSpaceClipPlanes(
            &partition->boundingPoly,
            partitionIndex,
            shadowSampleSize,
            &boundingPolyClipSpacePlanes);

        // Feed it all into the DPVS system for rendering
        R_SetupShadowSurfacesDpvs(shadowViewParms, boundingPolyClipSpacePlanes, planeCount, partitionIndex);

        // Increase the sampleSize by the PartitionRatio for the next Partition (Far)
        shadowSampleSize = shadowSampleSize * rg.sunShadowPartitionRatio;
    }

    // Generate lookup matrix used by lightning pass (R_SetShadowableLight)
    R_GetSunShadowLookupMatrix(
        &sunShadow->partition[0].shadowViewParms,
        &sunShadow->sunProj,
        partitionFraction,
        &sunShadow->lookupMatrix);
}

void __cdecl R_GetSceneExtentsAlongDir(const float *origin, const float *forward, float *nearCap, float *farCap)
{
    DpvsPlane nearPlane;
    float distMin;
    float distMax;

    nearPlane.coeffs[0] = forward[0];
    nearPlane.coeffs[1] = forward[1];
    nearPlane.coeffs[2] = forward[2];

    nearPlane.coeffs[3] = -origin[2];

    R_SetDpvsPlaneSides(&nearPlane);

    // LWSS: (Semi Adapted from BLOPS - same exact results)
    float bounds[2][3];
    bounds[0][0] = rgp.world->mins[0];
    bounds[0][1] = rgp.world->mins[1];
    bounds[0][2] = rgp.world->mins[2];

    bounds[1][0] = rgp.world->maxs[0];
    bounds[1][1] = rgp.world->maxs[1];
    bounds[1][2] = rgp.world->maxs[2];

    distMin = R_DpvsPlaneMinSignedDistToBox(&nearPlane, bounds[0]);
    distMax = R_DpvsPlaneMaxSignedDistToBox(&nearPlane, bounds[0]);

    iassert(distMin < distMax);
    
    *nearCap = distMin;
    *farCap = distMax;
}

void __cdecl R_SunShadowMapProjectionMatrix(
    const float *snappedViewOrgInClipSpace,
    float shadowSampleSize,
    float nearClip,
    float farClip,
    GfxViewParms *shadowViewParms)
{
    float shadowSampleExtents; // [esp+0h] [ebp-4h]

    iassert(shadowSampleSize);

    memset(&shadowViewParms->projectionMatrix, 0, sizeof(shadowViewParms->projectionMatrix));
    shadowSampleExtents = shadowSampleSize * 1024.0f;
    shadowViewParms->projectionMatrix.m[0][0] = 2.0f / shadowSampleExtents;
    shadowViewParms->projectionMatrix.m[3][0] = *snappedViewOrgInClipSpace;
    shadowViewParms->projectionMatrix.m[1][1] = shadowViewParms->projectionMatrix.m[0][0];
    shadowViewParms->projectionMatrix.m[3][1] = snappedViewOrgInClipSpace[1];
    shadowViewParms->projectionMatrix.m[2][2] = 1.0f / (farClip - nearClip + 2.0f);
    shadowViewParms->projectionMatrix.m[3][2] = -(nearClip - 1.0) * shadowViewParms->projectionMatrix.m[2][2];
    shadowViewParms->projectionMatrix.m[3][3] = 1.0f;
    shadowViewParms->depthHackNearClip = shadowViewParms->projectionMatrix.m[3][2];
    R_SetupViewProjectionMatrices(shadowViewParms);
}


static void __cdecl R_GetFrustumNearClipPoints(const GfxMatrix *invViewProjMtx, float (*frustumPoints)[4][3])
{
    R_ClipSpaceToWorldSpace(invViewProjMtx, &g_clipSpacePoints, 4, frustumPoints);
}

// This entire function checked for accuracy
// While some of the floats may be off by 0.01, it's sound.
// The only thing not checked were the called bounding poly functions at the bottom (bounding seems OK)
void __cdecl R_SetupSunShadowMapProjection(
    const GfxViewParms *viewParms,
    const float (*sunAxis)[3][3],
    GfxSunShadow *sunShadow,
    float (*snappedViewOrgInClipSpace)[2][2],
    float *partitionFraction)
{
    unsigned int sunShadowSize; // [esp+78h] [ebp-23Ch]
    float minsInSunProj[2][2]; // [esp+140h] [ebp-174h] BYREF
    float sizeInSunProj[2]; // [esp+150h] [ebp-164h]
    float nearFrustumPoints[4][3]; // [esp+158h] [ebp-15Ch] BYREF
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

    // Convert Origin to X/Y coordinates in the Sun Projection (2D)
    // sunAxis[0] = forward
    // sunAxis[1] = right 
    // sunAxis[2] = up
    viewOrgInSunProj[0] = -Vec3Dot(viewParms->origin, (*sunAxis)[1]);
    viewOrgInSunProj[1] = Vec3Dot(viewParms->origin, (*sunAxis)[2]);

    // Prep these values for a typical Smallest/Largest value search
    minsInSunProj[0][0] = FLT_MAX;
    minsInSunProj[0][1] = FLT_MAX;
    maxsInSunProj[0][0] = -FLT_MAX;
    maxsInSunProj[0][1] = -FLT_MAX;

    // Get the 4 Corners of the ZNear Frustum in 3D Space
    R_GetFrustumNearClipPoints(&viewParms->inverseViewProjectionMatrix, &nearFrustumPoints);

    // Iterate the 4 points of the zNear
    for (pointIndex = 0; pointIndex < 4; ++pointIndex)
    {
        float frustumPoint[3];
        frustumPoint[0] = nearFrustumPoints[pointIndex][0];
        frustumPoint[1] = nearFrustumPoints[pointIndex][1];
        frustumPoint[2] = nearFrustumPoints[pointIndex][2];

        float *pt = frustumPointsInSunProj[0][pointIndex];

        // Convert each 3D Point to Sun Projection 2D 
        pt[0] = -Vec3Dot(frustumPoint, (*sunAxis)[1]);
        pt[1] = Vec3Dot(frustumPoint, (*sunAxis)[2]);

        // Check if 2D pt beats the current Min/Max, set if so
        AddPointToBounds2D(pt, minsInSunProj[0], maxsInSunProj[0]);
    }

    // set frustumPointsInSunProj[0][4] to the Player Origin (The zNear Frustum starts about 4 units away)
    frustumPointsInSunProj[0][4][0] = viewOrgInSunProj[0];
    frustumPointsInSunProj[0][4][1] = viewOrgInSunProj[1];

    // Copy the first 5 points from frustumPointsInSunProj[0] into frustumPointsInSunProj[1]
    // (The Near Bounding poly has only 5 points, while the Far one has 8)
    frustumPointsInSunProj[1][0][0] = frustumPointsInSunProj[0][0][0];
    frustumPointsInSunProj[1][0][1] = frustumPointsInSunProj[0][0][1];

    frustumPointsInSunProj[1][1][0] = frustumPointsInSunProj[0][1][0];
    frustumPointsInSunProj[1][1][1] = frustumPointsInSunProj[0][1][1];

    frustumPointsInSunProj[1][2][0] = frustumPointsInSunProj[0][2][0];
    frustumPointsInSunProj[1][2][1] = frustumPointsInSunProj[0][2][1];

    frustumPointsInSunProj[1][3][0] = frustumPointsInSunProj[0][3][0];
    frustumPointsInSunProj[1][3][1] = frustumPointsInSunProj[0][3][1];

    //memcpy(frustumPointsInSunProj[1], frustumPointsInSunProj[0], 32);

    // Copy the Mins/Maxs from [0] to [1] (Used as starter Min/Max Values)
    minsInSunProj[1][0] = minsInSunProj[0][0];
    minsInSunProj[1][1] = minsInSunProj[0][1];
    maxsInSunProj[1][0] = maxsInSunProj[0][0];
    maxsInSunProj[1][1] = maxsInSunProj[0][1];

    // Extrapolate 4 Points for the Far Frustum partition
    for (pointIndex = 0; pointIndex < 4; ++pointIndex)
    {
        // Get offset from Origin to each point
        offset[0] = frustumPointsInSunProj[1][pointIndex][0] - viewOrgInSunProj[0];
        offset[1] = frustumPointsInSunProj[1][pointIndex][1] - viewOrgInSunProj[1];

        // Start writing at [4]
        float *pt = frustumPointsInSunProj[1][4 + pointIndex];

        // Multiply the PartitionRatio with the Offset to Extrapolate the far cascade
        pt[0] = ((0.75f / rg.sunShadowPartitionRatio) * offset[0]) + viewOrgInSunProj[0];
        pt[1] = ((0.75f / rg.sunShadowPartitionRatio) * offset[1]) + viewOrgInSunProj[1];

        // Update the minsInSunProj[1] / maxsInSunProj[1] if needed 
        AddPointToBounds2D(pt, minsInSunProj[1], maxsInSunProj[1]);
    }

    // Check if the Origin(In Sun Proj) is smaller or bigger than the mins/maxs in [0] (near partition). Update min/max in [0] if so.
    AddPointToBounds2D(viewOrgInSunProj, minsInSunProj[0], maxsInSunProj[0]);

    // Calculate sizeInSunProj = (Max - Min) -- (For the Near Partition)
    sizeInSunProj[0] = maxsInSunProj[0][0] - minsInSunProj[0][0];
    sizeInSunProj[1] = maxsInSunProj[0][1] - minsInSunProj[0][1];

    maxSizeInSunProj = max(sizeInSunProj[0], sizeInSunProj[1]);

    sampleSizeNear = sm_sunSampleSizeNear->current.value; // Default: 0.25f
    sampleSizeFar = sampleSizeNear * rg.sunShadowPartitionRatio; // rg.sunShadowPartitionRatio Typically 4.0f

    scale = sampleSizeFar * rg.sunShadowmapScaleNum;
    snappedViewOrgInTicks[0] = SnapFloatToInt(floor(viewOrgInSunProj[0] / scale));
    snappedViewOrgInTicks[1] = SnapFloatToInt(floor(viewOrgInSunProj[1] / scale));
    snappedViewOrgInSunProj[0] = (float)snappedViewOrgInTicks[0] * scale;
    snappedViewOrgInSunProj[1] = (float)snappedViewOrgInTicks[1] * scale;
    scaleToFitUsable = 1023.0f / maxSizeInSunProj;
    farShadowBegin = (1024 - rg.sunShadowSize) / 2;
    farShadowEnd = 1024 - farShadowBegin;
    farShadowMiddle = 512;
    farShadowSizeRatio = (float)rg.sunShadowSize / 1024.0f;


    // use shadow Offset if `sm_sunShadowCenter` is set (Not usually!)
    {
        shadowOrg[0] = sm_sunShadowCenter->current.vector[0];
        shadowOrg[1] = sm_sunShadowCenter->current.vector[1];
        shadowOrg[2] = sm_sunShadowCenter->current.vector[2];
        useShadowOffset = !(0.0 == shadowOrg[0] && 0.0 == shadowOrg[1] && 0.0 == shadowOrg[2]);

        // these values are only used if (useShadowOffset)
        if (useShadowOffset)
        {
            shadowOrgInSunProj[0] = -Vec3Dot(shadowOrg, (*sunAxis)[1]);
            shadowOrgInSunProj[1] = Vec3Dot(shadowOrg, (*sunAxis)[2]);
        }
    }

    partitionNear = &sunShadow->partition[0];
    partitionFar = &sunShadow->partition[1];
    
    // Setup the Near Viewport, this is used for D3D ScissorRect
    partitionNear->viewport.width =  SnapFloatToInt(ceilf(1024.0f / maxSizeInSunProj * sizeInSunProj[0] - EQUAL_EPSILON));
    partitionNear->viewport.height = SnapFloatToInt(ceilf(1024.0f / maxSizeInSunProj * sizeInSunProj[1] - EQUAL_EPSILON));

    // Align the viewports depending on whether the viewOrgInSunProj(origin in Sun 2D space) is on the left/right and top/bottom
    if (viewOrgInSunProj[0] < (minsInSunProj[0][0] + maxsInSunProj[0][0]) * 0.5f)
    {
        viewOrgInPixels[0][0] = (viewOrgInSunProj[0] - minsInSunProj[0][0]) * scaleToFitUsable + 1.0f;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][0] = (double)farShadowMiddle + (viewOrgInSunProj[0] - shadowOrgInSunProj[0]) / sampleSizeFar;
            partitionFar->viewport.x = farShadowBegin;
        }
        else
        {
            viewOrgInPixels[1][0] = (float)(farShadowBegin + 1) + (viewOrgInSunProj[0] - minsInSunProj[0][0]) * scaleToFitUsable * farShadowSizeRatio;
            float v12 = floor(scaleToFitUsable * (minsInSunProj[1][0] - minsInSunProj[0][0]) + (float)farShadowBegin);

            if ((int)farShadowBegin < SnapFloatToInt(v12))
                partitionFar->viewport.x = SnapFloatToInt(v12);
            else
                partitionFar->viewport.x = farShadowBegin;
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

            float v14 = ceil((float)farShadowEnd - scaleToFitUsable * (maxsInSunProj[0][0] - maxsInSunProj[1][0]));

            if ((int)(SnapFloatToInt(v14) - farShadowBegin) < (int)rg.sunShadowSize)
                sunShadowSize = SnapFloatToInt(v14) - farShadowBegin;
            else
                sunShadowSize = rg.sunShadowSize;
            partitionFar->viewport.width = sunShadowSize;
        }
        partitionNear->viewport.x = 1024 - partitionNear->viewport.width;
        partitionFar->viewport.x = farShadowBegin;
    }

    // (Do it for the far cascade too)
    if (viewOrgInSunProj[1] > (minsInSunProj[0][1] + maxsInSunProj[0][1]) * 0.5)
    {
        viewOrgInPixels[0][1] = (maxsInSunProj[0][1] - viewOrgInSunProj[1]) * scaleToFitUsable + 1.0;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][1] = (float)farShadowMiddle + (viewOrgInSunProj[1] - shadowOrgInSunProj[1]) / sampleSizeFar;
            partitionFar->viewport.y = farShadowBegin;
        }
        else
        {
            viewOrgInPixels[1][1] = (float)(farShadowBegin + 1)
                + (maxsInSunProj[0][1] - viewOrgInSunProj[1]) * scaleToFitUsable * farShadowSizeRatio;
            float v8 = floor(scaleToFitUsable * (maxsInSunProj[0][1] - maxsInSunProj[1][1]) + (float)farShadowBegin);
            if ((int)farShadowBegin < SnapFloatToInt(v8))
                partitionFar->viewport.y = SnapFloatToInt(v8);
            else
                partitionFar->viewport.y = farShadowBegin;
        }
        partitionNear->viewport.y = 0;
        partitionFar->viewport.height = farShadowEnd - partitionFar->viewport.y;
    }
    else
    {
        viewOrgInPixels[0][1] = 1023.0f - (viewOrgInSunProj[1] - minsInSunProj[0][1]) * scaleToFitUsable;
        if (useShadowOffset)
        {
            viewOrgInPixels[1][1] = (double)farShadowMiddle - (viewOrgInSunProj[1] - shadowOrgInSunProj[1]) / sampleSizeFar;
            partitionFar->viewport.height = rg.sunShadowSize;
        }
        else
        {
            viewOrgInPixels[1][1] = (double)(farShadowEnd - 1)
                - (viewOrgInSunProj[1] - minsInSunProj[0][1]) * scaleToFitUsable * farShadowSizeRatio;
            float v10 = ceil((float)farShadowEnd - scaleToFitUsable * (minsInSunProj[1][1] - minsInSunProj[0][1]));
            if ((int)(SnapFloatToInt(v10) - farShadowBegin) < (int)rg.sunShadowSize)
                partitionFar->viewport.height = SnapFloatToInt(v10) - farShadowBegin;
            else
                partitionFar->viewport.height = rg.sunShadowSize;
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

    sunProj->switchPartition[3] = 1.0f / rg.sunShadowPartitionRatio;

    sunProj->switchPartition[0] = viewOrgInTexSpace[1][0] - viewOrgInTexSpace[0][0] * sunProj->switchPartition[3];
    sunProj->switchPartition[1] = (viewOrgInTexSpace[1][1] - viewOrgInTexSpace[0][1] * sunProj->switchPartition[3] + 1.0f) * 0.5f;
    sunProj->switchPartition[2] = 0.0f;

    sunProj->shadowmapScale[0] = 16.0f / rg.sunShadowmapScale;
    sunProj->shadowmapScale[1] = 32.0f / rg.sunShadowmapScale;
    sunProj->shadowmapScale[2] = 0.0f;
    sunProj->shadowmapScale[3] = 0.0f;

    (*snappedViewOrgInClipSpace)[0][0] = (viewOrgInTexSpace[0][0] * 2.0f) - 1.0f;
    (*snappedViewOrgInClipSpace)[0][1] = 1.0f - (viewOrgInTexSpace[0][1] * 2.0f);
    (*snappedViewOrgInClipSpace)[1][0] = (viewOrgInTexSpace[1][0] * 2.0f) - 1.0f;
    (*snappedViewOrgInClipSpace)[1][1] = 1.0f - (viewOrgInTexSpace[1][1] * 2.0f);

    R_SetupSunShadowMapViewMatrix(snappedViewOrgInSunProj, sunAxis, sunProj);

    // Near cascade uses 5 point poly 
    // 
    // (FPS View)
    //   
    //    X-------------------------------------X         
    //    |                                     |
    //    |                                     |
    //    |                                     |
    //    |                                     |
    //    |                 X                   |
    //    |                                     |
    //    |                                     |
    //    |                                     |
    //    |                                     |
    //    X-------------------------------------X
    // 
    //  Origin, (4x) zNear Frustum Corners
    R_SetupSunShadowBoundingPoly(
        frustumPointsInSunProj[0],
        viewOrgInSunProj,
        snappedViewOrgInSunProj,
        maxSizeInSunProj,
        (*snappedViewOrgInClipSpace)[0],
        &partitionNear->boundingPoly,
        5);

    // Far cascade uses 8 point poly
    //             
    //             X--------X
    //            /|     /  |
    //          /  |   /    |
    //        /    X/-------X
    //      X----/X      /
    //      |   / |   /
    //      | /   | /
    //      X-----X
    // 
    // 
    // (4x) zNear Frustum Corners, (4x) zFar Frustum Corners
    R_SetupSunShadowBoundingPoly(
        frustumPointsInSunProj[1],
        viewOrgInSunProj,
        snappedViewOrgInSunProj,
        maxSizeInSunProj,
        (*snappedViewOrgInClipSpace)[1],
        &partitionFar->boundingPoly,
        8);

    R_SetupSunShadowMapPartitionFraction(viewParms, scaleToFitUsable, sunProj, partitionFraction);
    R_SetupNearRegionPlane(partitionFraction);
}

void __cdecl R_ClipSpaceToWorldSpace(
    const GfxMatrix *invViewProjMtx,
    const float (*clipSpacePoints)[4][3],
    int pointCount,
    float (*worldSpacePoints)[4][3])
{
    float xyz[3]; // [esp+Ch] [ebp-28h]
    float homogenousPoint[4]; // [esp+18h] [ebp-1Ch] BYREF

    for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
    {
        xyz[0] = (*clipSpacePoints)[pointIndex][0];
        xyz[1] = (*clipSpacePoints)[pointIndex][1];
        xyz[2] = (*clipSpacePoints)[pointIndex][2];

        for (int term = 0; term < 4; ++term)
        {
            homogenousPoint[term] = xyz[0] * invViewProjMtx->m[0][term] 
                                  + xyz[1] * invViewProjMtx->m[1][term] 
                                  + xyz[2] * invViewProjMtx->m[2][term] 
                                  + invViewProjMtx->m[3][term];
        }

        iassert(homogenousPoint[3] > 0.0f);
        float scale = 1.0f / homogenousPoint[3];
        Vec3Scale(homogenousPoint, scale, (*worldSpacePoints)[pointIndex]);
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

    memcpy(tempFrustumPointsInSunProj, frustumPointsInSunProj, 8 * pointCount);
    boundingPoly->pointCount = Com_ConvexHull(tempFrustumPointsInSunProj, pointCount, frustumBoundingPolyInSunProj);
    iassert(boundingPoly->pointCount >= 3 && boundingPoly->pointCount <= 9);
    scaleToClipSpace = 2.0 / maxSizeInSunProj;
    for (pointIndex = 0; pointIndex < boundingPoly->pointCount; ++pointIndex)
    {
        boundingPoly->points[pointIndex][0] = (frustumBoundingPolyInSunProj[pointIndex][0] - *viewOrgInSunProj) * scaleToClipSpace + *snappedViewOrgInClipSpace;
        boundingPoly->points[pointIndex][1] = (frustumBoundingPolyInSunProj[pointIndex][1] - viewOrgInSunProj[1]) * scaleToClipSpace + snappedViewOrgInClipSpace[1];

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
    const float (*sunAxis)[3][3],
    GfxSunShadowProjection *sunProj)
{
    sunProj->viewMatrix[0][0] = -(*sunAxis)[1][0];
    sunProj->viewMatrix[1][0] = -(*sunAxis)[1][1];
    sunProj->viewMatrix[2][0] = -(*sunAxis)[1][2];
    sunProj->viewMatrix[3][0] = -snappedViewOrgInSunProj[0];
    sunProj->viewMatrix[0][1] = (*sunAxis)[2][0];
    sunProj->viewMatrix[1][1] = (*sunAxis)[2][1];
    sunProj->viewMatrix[2][1] = (*sunAxis)[2][2];
    sunProj->viewMatrix[3][1] = -snappedViewOrgInSunProj[1];
    sunProj->viewMatrix[0][2] = (*sunAxis)[0][0];
    sunProj->viewMatrix[1][2] = (*sunAxis)[0][1];
    sunProj->viewMatrix[2][2] = (*sunAxis)[0][2];
    sunProj->viewMatrix[3][2] = 0.0f;
    sunProj->viewMatrix[0][3] = 0.0f;
    sunProj->viewMatrix[1][3] = 0.0f;
    sunProj->viewMatrix[2][3] = 0.0f;
    sunProj->viewMatrix[3][3] = 1.0f;
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

    iassert( zNear );

    endOfNearFrustum = zNear * sm_sunSampleSizeNear->current.value * scaleToFitUsable;
    scale = 1.0 / endOfNearFrustum;
    Vec3Scale(viewParms->axis[0], scale, partitionFraction);
    partitionFraction[3] = -Vec3Dot(viewParms->origin, partitionFraction);
}

void __cdecl R_GetSunShadowMapPartitionViewOrgInTextureSpace(
    const float *viewOrgInPixels, // base (in texels) of the cascade within the shadowmap
    const float *viewOrgInSunProj, // view Origin in the Sun space (2D world units)
    const float *snappedViewOrgInSunProj, // view origin snapped to the shadowmap texel grid
    float sampleSize, // how many world units per texel
    float *viewOrgInTexSpace) // [OUTPUT] normalized texture space coordinates [0-1]
{
    float snappedViewOrgInPixels[2]; // [esp+10h] [ebp-10h]

    snappedViewOrgInPixels[0] = (snappedViewOrgInSunProj[0] - viewOrgInSunProj[0]) / sampleSize + viewOrgInPixels[0];
    snappedViewOrgInPixels[1] = viewOrgInPixels[1] - (snappedViewOrgInSunProj[1] - viewOrgInSunProj[1]) / sampleSize;

    viewOrgInTexSpace[0] = floor(snappedViewOrgInPixels[0]) * (1.0f / 1024.0f);
    viewOrgInTexSpace[1] = floor(snappedViewOrgInPixels[1]) * (1.0f / 1024.0f);
}

void __cdecl R_SetupNearRegionPlane(const float *partitionFraction)
{
    float size; // [esp+24h] [ebp-14h]
    DpvsPlane *shadowFarPlane; // [esp+28h] [ebp-10h]
    unsigned int partitionIndex; // [esp+2Ch] [ebp-Ch]
    float scale; // [esp+30h] [ebp-8h]
    float length; // [esp+34h] [ebp-4h]

    scene.shadowNearPlane[0].coeffs[0] = partitionFraction[0];
    scene.shadowNearPlane[0].coeffs[1] = partitionFraction[1];
    scene.shadowNearPlane[0].coeffs[2] = partitionFraction[2];
    scene.shadowNearPlane[0].coeffs[3] = partitionFraction[3];
    length = Vec3Length(scene.shadowNearPlane[0].coeffs);
    iassert(length > 0);
    scale = 1.0f / length;
    Vec4Scale(scene.shadowNearPlane[0].coeffs, scale, scene.shadowNearPlane[0].coeffs);
    R_SetDpvsPlaneSides(&scene.shadowNearPlane[0]);

    scene.shadowNearPlane[1].coeffs[0] = partitionFraction[0];
    scene.shadowNearPlane[1].coeffs[1] = partitionFraction[1];
    scene.shadowNearPlane[1].coeffs[2] = partitionFraction[2];
    scene.shadowNearPlane[1].coeffs[3] = partitionFraction[3];
    scene.shadowNearPlane[1].coeffs[3] = scene.shadowNearPlane[1].coeffs[3] - 0.75f;
    length = Vec3Length(scene.shadowNearPlane[1].coeffs);
    iassert(length > 0);
    scale = 1.0f / length;
    Vec4Scale(scene.shadowNearPlane[1].coeffs, scale, scene.shadowNearPlane[1].coeffs);
    R_SetDpvsPlaneSides(&scene.shadowNearPlane[1]);

    size = 1.0f;
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        shadowFarPlane = &scene.shadowFarPlane[partitionIndex];
        shadowFarPlane->coeffs[0] = partitionFraction[0];
        shadowFarPlane->coeffs[1] = partitionFraction[1];
        shadowFarPlane->coeffs[2] = partitionFraction[2];
        shadowFarPlane->coeffs[3] = partitionFraction[3];
        shadowFarPlane->coeffs[3] = shadowFarPlane->coeffs[3] - size;
        length = Vec3Length(shadowFarPlane->coeffs);
        iassert(length > 0);
        scale = -1.0f / length;
        Vec4Scale(shadowFarPlane->coeffs, scale, shadowFarPlane->coeffs);
        R_SetDpvsPlaneSides(shadowFarPlane);
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
    unsigned int oldViewIndex;
    int partitionIndex;
    unsigned int viewIndex;

    iassert(rgp.world);

    PROF_SCOPED("R_SunShadowMaps");

    oldViewIndex = R_SetVisData(0);
    shadowGlob.defaultShadowCasterTechnique = Material_GetTechnique(rgp.depthPrepassMaterial, gfxMetrics.shadowmapBuildTechType);

    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        viewIndex = SCENE_VIEW_SUNSHADOW_0 + partitionIndex;
        iassert(((viewIndex >= SCENE_VIEW_SUNSHADOW_0) && (viewIndex <= SCENE_VIEW_SUNSHADOW_1)));
        R_SetVisData(viewIndex);
        {
            PROF_SCOPED("R_AddShadowSurfacesDpvs");
            R_AddWorldSurfacesFrustumOnly();
        }
    }
    R_SetVisData(oldViewIndex);
}

void __cdecl R_MergeAndEmitSunShadowMapsSurfs(GfxViewInfo *viewInfo)
{
    int firstDrawSurf; // [esp+38h] [ebp-14h]
    GfxDrawSurfListInfo *info; // [esp+3Ch] [ebp-10h]
    unsigned int partitionIndex; // [esp+40h] [ebp-Ch]
    GfxSunShadow *sunShadow; // [esp+44h] [ebp-8h]

    sunShadow = &viewInfo->sunShadow;

    PROF_SCOPED("EmitSunShadow");

    iassert(frontEndDataOut->sunLight.type == GFX_LIGHT_TYPE_DIR);

    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        info = &sunShadow->partition[partitionIndex].info;
        R_InitDrawSurfListInfo(info);
        info->baseTechType = gfxMetrics.shadowmapBuildTechType;
        info->viewInfo = viewInfo;
        info->viewOrigin[0] = frontEndDataOut->sunLight.dir[0];
        info->viewOrigin[1] = frontEndDataOut->sunLight.dir[1];
        info->viewOrigin[2] = frontEndDataOut->sunLight.dir[2];
        info->viewOrigin[3] = 0.0f;
        iassert(!info->cameraView);
        firstDrawSurf = frontEndDataOut->drawSurfCount;
        DrawSurfType bspSunShadowDrawType = (DrawSurfType)((int)DRAW_SURF_BSP_SUNSHADOW_0 + (3 * partitionIndex));
        DrawSurfType smodelSunShadowDrawType = (DrawSurfType)((int)DRAW_SURF_SMODEL_SUNSHADOW_0 + (3 * partitionIndex));
        R_MergeAndEmitDrawSurfLists(bspSunShadowDrawType, 1);
        R_MergeAndEmitDrawSurfLists(smodelSunShadowDrawType, 2);
        sunShadow->partition[partitionIndex].info.drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
        sunShadow->partition[partitionIndex].info.drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
        sunShadow->partition[partitionIndex].partitionIndex = partitionIndex;
    }
}

