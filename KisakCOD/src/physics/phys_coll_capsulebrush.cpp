#include "phys_local.h"
#include "phys_coll_local.h"
#include <cgame/cg_local.h>

int numLocalContacts;
LocalContactData localContacts[32];

void __cdecl Phys_InfoToCapsule(const objInfo *info, Capsule *capsule)
{
    float scale; // [esp+0h] [ebp-20h]
    unsigned int dir; // [esp+1Ch] [ebp-4h]

    dir = info->cylDirection - 1;
    if (dir > 2)
        MyAssertHandler(
            ".\\physics\\phys_coll_capsulebrush.cpp",
            1182,
            0,
            "dir not in [0, 2]\n\t%i not in [%i, %i]",
            dir,
            0,
            2);
    capsule->radius = info->u.sideExtents[0];
    capsule->halfHeight = info->u.sideExtents[2];
    capsule->halfLength = capsule->halfHeight + capsule->radius;
    capsule->sqRadius = capsule->radius * capsule->radius;
    capsule->center[0] = info->pos[0];
    capsule->center[1] = info->pos[1];
    capsule->center[2] = info->pos[2];
    capsule->axis[0] = -info->R[dir][0];
    capsule->axis[1] = -info->R[dir][1];
    capsule->axis[2] = -info->R[dir][2];
    scale = -info->u.sideExtents[2];
    Vec3Mad(capsule->center, scale, capsule->axis, capsule->p0);
    Vec3Mad(capsule->center, info->u.sideExtents[2], capsule->axis, capsule->p1);
}

double __cdecl Phys_DistanceOfCapsuleFromPlane(const float *plane, const Capsule *capsule)
{
    double v3; // st7
    float v5; // [esp+4h] [ebp-10h]
    float p0Dist; // [esp+8h] [ebp-Ch]
    float p1Dist; // [esp+Ch] [ebp-8h]
    float centerDist; // [esp+10h] [ebp-4h]

    centerDist = Vec3Dot(capsule->center, plane) - plane[3];
    if (centerDist < 0.0)
        return -3.4028235e38;
    p0Dist = Vec3Dot(capsule->p0, plane) - plane[3] - capsule->radius;
    v3 = Vec3Dot(capsule->p1, plane);
    p1Dist = v3 - plane[3] - capsule->radius;
    v5 = p1Dist - p0Dist;
    if (v5 < 0.0)
        return (v3 - plane[3] - capsule->radius);
    else
        return p0Dist;
}

bool __cdecl Phys_SimilarContacts(const LocalContactData *ca, const LocalContactData *cb)
{
    int dirNear; // [esp+Ch] [ebp-8h]
    int posNear; // [esp+10h] [ebp-4h]

    posNear = VecNCompareCustomEpsilon(ca->pos, cb->pos, 0.001, 3);
    dirNear = VecNCompareCustomEpsilon(ca->normal, cb->normal, 0.001, 3);
    return posNear && dirNear;
}

bool __cdecl Phys_ContactBetter(const LocalContactData *ca, const LocalContactData *cb)
{
    return cb->depth < ca->depth;
}

int Phys_CancelSimilarContacts()
{
    int result; // eax
    int j; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int numContacts; // [esp+8h] [ebp-4h]

    result = numLocalContacts;
    numContacts = numLocalContacts;
    for (i = 0; i < numContacts - 1; ++i)
    {
        result = i + 1;
        for (j = i + 1; j < numContacts; ++j)
        {
            LOBYTE(result) = Phys_SimilarContacts(&localContacts[i], &localContacts[j]);
            if (result)
            {
                LOBYTE(result) = Phys_ContactBetter(&localContacts[i], &localContacts[j]);
                if (result)
                {
                    result = 36 * j;
                    localContacts[j].inUse = 0;
                }
                else
                {
                    localContacts[i].inUse = 0;
                }
            }
        }
    }
    return result;
}

void __cdecl Phys_CapsuleOptimizeLocalResults(Results *results)
{
    int i; // [esp+1Ch] [ebp-4h]

    if (numLocalContacts > 1)
        Phys_CancelSimilarContacts();
    for (i = 0; i < numLocalContacts; ++i)
    {
        if (localContacts[i].inUse)
        {
            iassert(!IS_NAN((localContacts[i].normal)[0]) && !IS_NAN((localContacts[i].normal)[1]) && !IS_NAN((localContacts[i].normal)[2]));
            Phys_AddContactData(
                results,
                localContacts[i].depth,
                localContacts[i].normal,
                localContacts[i].pos,
                localContacts[i].surfFlags);
        }
    }
}

void __cdecl Phys_CollideCapsuleWithBrush(const cbrush_t *brush, const objInfo *info, Results *results)
{
    int v3; // [esp+2Ch] [ebp-D5Ch]
    cplane_s *v4; // [esp+30h] [ebp-D58h]
    float plane[4]; // [esp+3Ch] [ebp-D4Ch] BYREF
    float v6; // [esp+4Ch] [ebp-D3Ch]
    unsigned int i; // [esp+50h] [ebp-D38h]
    float v8; // [esp+54h] [ebp-D34h]
    float axialPlanes_1[6][4]; // [esp+58h] [ebp-D30h] BYREF
    Poly outWinding; // [esp+B8h] [ebp-CD0h] BYREF
    unsigned int tri; // [esp+C0h] [ebp-CC8h]
    int surfaceFlags; // [esp+C4h] [ebp-CC4h]
    float brushPlane[4];
    unsigned int ptCount; // [esp+D8h] [ebp-CB0h]
    int brushSideIndex; // [esp+DCh] [ebp-CACh]
    Capsule capsule; // [esp+E0h] [ebp-CA8h] BYREF
    float *pts; // [esp+120h] [ebp-C68h]
    float brushSeparation; // [esp+124h] [ebp-C64h]
    float brushVerts[256][3]; // [esp+128h] [ebp-C60h] BYREF
    float axialPlanes[6][4]; // [esp+D28h] [ebp-60h] BYREF

    vassert(results->contactCount < results->maxContacts, "results->contactCount = %d, results->maxContacts = %d", results->contactCount, results->maxContacts);
    numLocalContacts = 0;
    Phys_InfoToCapsule(info, &capsule);
    iassert(brush);
    brushPlane[0] = 0.0;
    brushPlane[1] = 0.0;
    brushPlane[2] = 0.0;
    brushPlane[3] = 0.0;
    v8 = FLT_MIN;
    brushSideIndex = -1;
    CM_BuildAxialPlanes(brush, &axialPlanes_1);
    for (i = 0; i < 6; ++i)
    {
        v6 = Phys_DistanceOfCapsuleFromPlane(axialPlanes_1[i], &capsule);
        if (v6 >= 0.0)
        {
            v3 = 0;
            goto LABEL_24;
        }
        if (brush->edgeCount[i & 1][i >> 1] && v8 < v6)
        {
            v8 = v6;
            brushSideIndex = i;
            brushPlane[0] = axialPlanes_1[i][0];
            brushPlane[1] = axialPlanes_1[i][1];
            brushPlane[2] = axialPlanes_1[i][2];
            brushPlane[3] = axialPlanes_1[i][3];
        }
    }
    for (i = 0; i < brush->numsides; ++i)
    {
        v4 = brush->sides[i].plane;
        plane[0] = v4->normal[0];
        plane[1] = v4->normal[1];
        plane[2] = v4->normal[2];
        plane[3] = brush->sides[i].plane->dist;
        v6 = Phys_DistanceOfCapsuleFromPlane(plane, &capsule);
        if (v6 >= 0.0)
        {
            v3 = 0;
            goto LABEL_24;
        }
        if (brush->sides[i].edgeCount && v8 < (double)v6)
        {
            v8 = v6;
            brushSideIndex = i + 6;
            brushPlane[0] = plane[0];
            brushPlane[1] = plane[1];
            brushPlane[2] = plane[2];
            brushPlane[3] = plane[3];
        }
    }
    brushSeparation = v8;
    v3 = 1;
LABEL_24:
    if (v3 && brushSideIndex >= 0)
    {
        bcassert(brushSideIndex, brush->numsides + 6);

        if (Phys_TestCapsulePlane(brushPlane, &capsule))
        {
            outWinding.pts = brushVerts;
            outWinding.ptCount = 0;
            CM_BuildAxialPlanes(brush, &axialPlanes);
            Phys_GetWindingForBrushFace2(brush, brushSideIndex, &outWinding, 256, axialPlanes);
            ptCount = outWinding.ptCount;
            if (outWinding.ptCount)
            {
                if (phys_drawCollisionWorld->current.enabled)
                    Phys_DrawPoly(&outWinding, colorCyan);
                vassert(ptCount > 3, "ptCount = %d", ptCount);

                pts = (float *)outWinding.pts;
                ptCount -= 3;
                surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, brushSideIndex);
                if (ptCount)
                {
                    Phys_CapsuleBuildContactsForTriEndEdges(results, brushPlane, &capsule, pts, pts + 6, pts + 3, surfaceFlags);
                    for (tri = 1; tri < ptCount; ++tri)
                        Phys_CapsuleBuildContactsForTriMiddleEdge(
                            results,
                            brushPlane,
                            &capsule,
                            pts,
                            &pts[3 * tri + 6],
                            &pts[3 * tri + 3],
                            surfaceFlags);
                    Phys_CapsuleBuildContactsForTriEndEdges(
                        results,
                        brushPlane,
                        &capsule,
                        &pts[3 * ptCount],
                        &pts[3 * ptCount + 6],
                        &pts[3 * ptCount + 3],
                        surfaceFlags);
                }
                else
                {
                    Phys_CapsuleBuildContactsForTri(results, brushPlane, &capsule, pts, pts + 6, pts + 3, surfaceFlags);
                }
                Phys_CapsuleOptimizeLocalResults(results);
            }
        }
    }
}

void __cdecl Phys_CapsuleCenterEdgePts(
    const AxisTestResults *axisResults,
    const Capsule *capsule,
    float *edge0,
    float *edge1)
{
    float scale; // [esp+0h] [ebp-18h]
    float center[3]; // [esp+Ch] [ebp-Ch] BYREF

    Vec3Mad(capsule->center, capsule->radius, axisResults->normal, center);
    Vec3Mad(center, capsule->halfHeight, capsule->axis, edge0);
    scale = -capsule->halfHeight;
    Vec3Mad(center, scale, capsule->axis, edge1);
}

char __cdecl Phys_CapsuleClipEdgeToPlane(float *p0, float *p1, float *plane)
{
    float delta[3]; // [esp+0h] [ebp-24h] BYREF
    float dist1; // [esp+Ch] [ebp-18h]
    float planeFact; // [esp+10h] [ebp-14h]
    float isectPt[3]; // [esp+14h] [ebp-10h]
    float dist0; // [esp+20h] [ebp-4h]

    dist0 = Vec3Dot(p0, plane) + plane[3];
    dist1 = Vec3Dot(p1, plane) + plane[3];
    if (dist0 < 0.0 && dist1 < 0.0)
        return 0;
    if (dist0 > 0.0 && dist1 > 0.0)
        return 1;
    if ((dist0 <= 0.0 || dist1 >= 0.0) && (dist0 >= 0.0 || dist1 <= 0.0))
        return 1;
    Vec3Sub(p0, p1, delta);
    planeFact = dist0 / (dist0 - dist1);
    isectPt[0] = *p0 - delta[0] * planeFact;
    isectPt[1] = p0[1] - delta[1] * planeFact;
    isectPt[2] = p0[2] - delta[2] * planeFact;
    if (dist0 >= 0.0)
    {
        *p1 = isectPt[0];
        p1[1] = isectPt[1];
        p1[2] = isectPt[2];
    }
    else
    {
        *p0 = isectPt[0];
        p0[1] = isectPt[1];
        p0[2] = isectPt[2];
    }
    return 1;
}

void __cdecl Phys_AddLocalContactData(float depth, float *normal, const float *pos, int surfFlags)
{
    LocalContactData *contact; // [esp+10h] [ebp-4h]

    if (numLocalContacts < 32)
    {
        if ((COERCE_UNSIGNED_INT(*normal) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(normal[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(normal[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\phys_coll_capsulebrush.cpp",
                78,
                0,
                "%s",
                "!IS_NAN((normal)[0]) && !IS_NAN((normal)[1]) && !IS_NAN((normal)[2])");
        }
        contact = &localContacts[numLocalContacts++];
        contact->pos[0] = *pos;
        contact->pos[1] = pos[1];
        contact->pos[2] = pos[2];
        contact->normal[0] = *normal;
        contact->normal[1] = normal[1];
        contact->normal[2] = normal[2];
        contact->depth = depth;
        contact->surfFlags = surfFlags;
        contact->inUse = 1;
    }
}

void __cdecl Phys_CapsuleBuildContactsForTri(
    Results *results,
    const float *plane,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    int surfaceFlags)
{
    double v7; // st7
    double v8; // st7
    float v9; // [esp+10h] [ebp-8Ch]
    float v10; // [esp+14h] [ebp-88h]
    float v11; // [esp+18h] [ebp-84h]
    float v12; // [esp+1Ch] [ebp-80h]
    float delta[3]; // [esp+20h] [ebp-7Ch] BYREF
    float e1[3]; // [esp+2Ch] [ebp-70h] BYREF
    float clipPlane[4]; // [esp+38h] [ebp-64h] BYREF
    float depth1; // [esp+48h] [ebp-54h]
    AxisTestResults axisResults; // [esp+4Ch] [ebp-50h] BYREF
    float capEdge1[3]; // [esp+68h] [ebp-34h] BYREF
    float e0[3]; // [esp+74h] [ebp-28h] BYREF
    float e2[3]; // [esp+80h] [ebp-1Ch] BYREF
    float depth0; // [esp+8Ch] [ebp-10h]
    float capEdge0[3]; // [esp+90h] [ebp-Ch] BYREF

    if (Phys_CapsuleSeparatingAxisTest(&axisResults, plane, capsule, p0, p1, p2))
    {
        Phys_CapsuleCenterEdgePts(&axisResults, capsule, capEdge0, capEdge1);
        Vec3Sub(capEdge0, p0, capEdge0);
        Vec3Sub(capEdge1, p0, capEdge1);
        clipPlane[0] = -*plane;
        clipPlane[1] = -plane[1];
        clipPlane[2] = -plane[2];
        clipPlane[3] = 0.0;
        if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
        {
            Vec3Sub(p1, p0, e0);
            Vec3Cross(plane, e0, clipPlane);
            clipPlane[3] = 0.000001;
            if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
            {
                Vec3Sub(p2, p1, e1);
                Vec3Cross(plane, e1, clipPlane);
                clipPlane[3] = -(Vec3Dot(e0, clipPlane) - 0.000001);
                if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
                {
                    Vec3Sub(p0, p2, e2);
                    Vec3Cross(plane, e2, clipPlane);
                    clipPlane[3] = 0.000001;
                    if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
                    {
                        Vec3Add(capEdge0, p0, capEdge0);
                        Vec3Add(capEdge1, p0, capEdge1);
                        Vec3Sub(capEdge0, capsule->center, delta);
                        v7 = Vec3Dot(delta, axisResults.normal);
                        depth0 = v7 - (axisResults.bestCenter - axisResults.bestRt);
                        Vec3Sub(capEdge1, capsule->center, delta);
                        v8 = Vec3Dot(delta, axisResults.normal);
                        depth1 = v8 - (axisResults.bestCenter - axisResults.bestRt);
                        v12 = depth0 - 0.0;
                        if (v12 < 0.0)
                            v11 = 0.0;
                        else
                            v11 = depth0;
                        depth0 = v11;
                        v10 = depth1 - 0.0;
                        if (v10 < 0.0)
                            v9 = 0.0;
                        else
                            v9 = depth1;
                        depth1 = v9;
                        Phys_AddLocalContactData(depth0, axisResults.normal, capEdge0, surfaceFlags);
                        Phys_AddLocalContactData(depth1, axisResults.normal, capEdge1, surfaceFlags);
                    }
                }
            }
        }
    }
}

void __cdecl Phys_CalcAxis(const float *v0, const float *v1, const float *v2, float *result)
{
    float t0[3]; // [esp+0h] [ebp-18h] BYREF
    float t1[3]; // [esp+Ch] [ebp-Ch] BYREF

    Vec3Sub(v0, v1, t0);
    Vec3Cross(t0, v2, t1);
    Vec3Cross(t1, v2, result);
}

bool __cdecl Phys_CapsuleSeparatingAxisTest(
    AxisTestResults *axisResults,
    const float *plane,
    const Capsule *capsule,
    const float *tri0,
    const float *tri1,
    const float *tri2)
{
    float scale; // [esp+0h] [ebp-1B8h]
    float v8[3]; // [esp+Ch] [ebp-1ACh] BYREF
    float v9[3]; // [esp+18h] [ebp-1A0h] BYREF
    float v10[3]; // [esp+24h] [ebp-194h] BYREF
    float v11[3]; // [esp+30h] [ebp-188h] BYREF
    float v12[3]; // [esp+3Ch] [ebp-17Ch] BYREF
    float v13[3]; // [esp+48h] [ebp-170h] BYREF
    float v0[3]; // [esp+54h] [ebp-164h] BYREF
    float v15[52]; // [esp+60h] [ebp-158h] BYREF
    float diff[3]; // [esp+130h] [ebp-88h] BYREF
    float cross[3]; // [esp+13Ch] [ebp-7Ch] BYREF
    float *normal; // [esp+148h] [ebp-70h]
    float cp0[3]; // [esp+14Ch] [ebp-6Ch] BYREF
    float e1[3]; // [esp+158h] [ebp-60h] BYREF
    float testAxis[3]; // [esp+164h] [ebp-54h] BYREF
    float p1[3]; // [esp+170h] [ebp-48h] BYREF
    float cp1[3]; // [esp+17Ch] [ebp-3Ch] BYREF
    float p2[3]; // [esp+188h] [ebp-30h] BYREF
    float p0[3]; // [esp+194h] [ebp-24h] BYREF
    float e0[3]; // [esp+1A0h] [ebp-18h] BYREF
    float e2[3]; // [esp+1ACh] [ebp-Ch] BYREF

    Vec3Mad(capsule->center, capsule->halfHeight, capsule->axis, cp0);
    scale = -capsule->halfHeight;
    Vec3Mad(capsule->center, scale, capsule->axis, cp1);
    axisResults->bestDepth = -3.4028235e38;
    axisResults->bestCenter = 0.0;
    axisResults->bestAxis = 0;
    axisResults->bestRt = 0.0;
    normal = axisResults->normal;
    axisResults->normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 0.0;
    Vec3Sub(tri0, capsule->center, p0);
    Vec3Sub(tri1, capsule->center, p1);
    Vec3Sub(tri2, capsule->center, p2);
    LODWORD(v15[51]) = (uint32)capsule->axis;
    Vec3Sub(tri0, cp0, diff);
    Vec3Cross(diff, capsule->axis, cross);
    Vec3Cross(cross, capsule->axis, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 11, 0))
        return 0;
    Vec3Sub(tri1, tri0, e0);
    Vec3Cross(capsule->axis, e0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 2, 0))
        return 0;
    Vec3Sub(tri2, tri1, e1);
    Vec3Cross(capsule->axis, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 3, 0))
        return 0;
    Vec3Sub(tri0, tri2, e2);
    Vec3Cross(capsule->axis, e2, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 4, 0))
        return 0;
    Vec3Sub(cp0, tri0, v0);
    Vec3Cross(v0, e0, v15);
    Vec3Cross(v15, e0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 5, 0))
        return 0;
    Vec3Sub(cp0, tri1, v12);
    Vec3Cross(v12, e1, v13);
    Vec3Cross(v13, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 6, 0))
        return 0;
    Vec3Sub(cp0, tri2, v10);
    Vec3Cross(v10, e2, v11);
    Vec3Cross(v11, e2, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 7, 0))
        return 0;
    Vec3Sub(cp1, tri2, v8);
    Vec3Cross(v8, e2, v9);
    Vec3Cross(v9, e2, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 10, 0))
        return 0;
    Phys_CalcAxis(cp1, tri1, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 9, 0))
        return 0;
    Phys_CalcAxis(cp1, tri0, e0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 8, 0))
        return 0;
    Phys_CalcAxis(tri1, cp0, capsule->axis, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 12, 0))
        return 0;
    Phys_CalcAxis(tri2, cp0, capsule->axis, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 13, 0))
        return 0;
    Vec3Sub(tri0, cp0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 14, 0))
        return 0;
    Vec3Sub(tri1, cp0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 15, 0))
        return 0;
    Vec3Sub(tri2, cp0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 16, 0))
        return 0;
    Vec3Sub(tri0, cp1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 17, 0))
        return 0;
    Vec3Sub(tri1, cp1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 18, 0))
        return 0;
    Vec3Sub(tri2, cp1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 19, 0))
        return 0;
    Vec3Negate(plane, testAxis);
    return Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 1, 1) != 0;
}

char __cdecl Phys_TestAxis(
    AxisTestResults *axisResults,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    const float *inAxis,
    int axisNum,
    bool noFlip)
{
    float v9; // [esp+8h] [ebp-64h]
    float v10; // [esp+Ch] [ebp-60h]
    float v11; // [esp+10h] [ebp-5Ch]
    float v12; // [esp+14h] [ebp-58h]
    float scale; // [esp+18h] [ebp-54h]
    float v14; // [esp+2Ch] [ebp-40h]
    float depth; // [esp+34h] [ebp-38h]
    float frc; // [esp+38h] [ebp-34h]
    float afv[3]; // [esp+3Ch] [ebp-30h]
    float radius; // [esp+48h] [ebp-24h]
    float minCoord; // [esp+4Ch] [ebp-20h]
    int i; // [esp+50h] [ebp-1Ch]
    float axis[3]; // [esp+54h] [ebp-18h] BYREF
    float maxCoord; // [esp+60h] [ebp-Ch]
    float axisLen; // [esp+64h] [ebp-8h]
    float center; // [esp+68h] [ebp-4h]

    axisLen = Vec3Length(inAxis);
    if (axisLen < 0.000001)
        return 1;
    scale = 1.0 / axisLen;
    Vec3Scale(inAxis, scale, axis);
    v14 = Vec3Dot(capsule->axis, axis);
    v12 = fabs(v14);
    frc = v12 * capsule->halfHeight + capsule->radius;
    afv[0] = Vec3Dot(p0, axis);
    afv[1] = Vec3Dot(p1, axis);
    afv[2] = Vec3Dot(p2, axis);
    minCoord = afv[0];
    maxCoord = afv[0];
    for (i = 1; i < 3; ++i)
    {
        if (minCoord > afv[i])
            minCoord = afv[i];
        if (maxCoord < afv[i])
            maxCoord = afv[i];
    }
    center = (minCoord + maxCoord) * 0.5;
    radius = (maxCoord - minCoord) * 0.5;
    v11 = frc + radius;
    v10 = fabs(center);
    if (v10 > v11)
        return 0;
    v9 = fabs(center);
    depth = v9 - (frc + radius);
    if (axisResults->bestDepth < depth)
    {
        axisResults->bestDepth = depth;
        axisResults->bestCenter = center;
        axisResults->bestRt = radius;
        axisResults->normal[0] = axis[0];
        axisResults->normal[1] = axis[1];
        axisResults->normal[2] = axis[2];
        axisResults->bestAxis = axisNum;
        if (center < 0.0 && !noFlip)
        {
            axisResults->normal[0] = -axisResults->normal[0];
            axisResults->normal[1] = -axisResults->normal[1];
            axisResults->normal[2] = -axisResults->normal[2];
            axisResults->bestCenter = -center;
        }
    }
    return 1;
}

void __cdecl Phys_CapsuleBuildContactsForTriEndEdges(
    Results *results,
    const float *plane,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    int surfaceFlags)
{
    double v7; // st7
    double v8; // st7
    float v9; // [esp+10h] [ebp-8Ch]
    float v10; // [esp+14h] [ebp-88h]
    float v11; // [esp+18h] [ebp-84h]
    float v12; // [esp+1Ch] [ebp-80h]
    float delta[3]; // [esp+20h] [ebp-7Ch] BYREF
    float e1[3]; // [esp+2Ch] [ebp-70h] BYREF
    float clipPlane[4]; // [esp+38h] [ebp-64h] BYREF
    float depth1; // [esp+48h] [ebp-54h]
    AxisTestResults axisResults; // [esp+4Ch] [ebp-50h] BYREF
    float capEdge1[3]; // [esp+68h] [ebp-34h] BYREF
    float e0[3]; // [esp+74h] [ebp-28h] BYREF
    float e2[3]; // [esp+80h] [ebp-1Ch] BYREF
    float depth0; // [esp+8Ch] [ebp-10h]
    float capEdge0[3]; // [esp+90h] [ebp-Ch] BYREF

    if (Phys_CapsuleSeparatingAxisTestEndEdges(&axisResults, plane, capsule, p0, p1, p2))
    {
        Phys_CapsuleCenterEdgePts(&axisResults, capsule, capEdge0, capEdge1);
        Vec3Sub(capEdge0, p0, capEdge0);
        Vec3Sub(capEdge1, p0, capEdge1);
        clipPlane[0] = -*plane;
        clipPlane[1] = -plane[1];
        clipPlane[2] = -plane[2];
        clipPlane[3] = 0.0;
        if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
        {
            Vec3Sub(p1, p0, e0);
            Vec3Sub(p2, p1, e1);
            Vec3Cross(plane, e1, clipPlane);
            clipPlane[3] = -(Vec3Dot(e0, clipPlane) - 0.000001);
            if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
            {
                Vec3Sub(p0, p2, e2);
                Vec3Cross(plane, e2, clipPlane);
                clipPlane[3] = 0.000001;
                if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
                {
                    Vec3Add(capEdge0, p0, capEdge0);
                    Vec3Add(capEdge1, p0, capEdge1);
                    Vec3Sub(capEdge0, capsule->center, delta);
                    v7 = Vec3Dot(delta, axisResults.normal);
                    depth0 = v7 - (axisResults.bestCenter - axisResults.bestRt);
                    Vec3Sub(capEdge1, capsule->center, delta);
                    v8 = Vec3Dot(delta, axisResults.normal);
                    depth1 = v8 - (axisResults.bestCenter - axisResults.bestRt);
                    v12 = depth0 - 0.0;
                    if (v12 < 0.0)
                        v11 = 0.0;
                    else
                        v11 = depth0;
                    depth0 = v11;
                    v10 = depth1 - 0.0;
                    if (v10 < 0.0)
                        v9 = 0.0;
                    else
                        v9 = depth1;
                    depth1 = v9;
                    Phys_AddLocalContactData(depth0, axisResults.normal, capEdge0, surfaceFlags);
                    Phys_AddLocalContactData(depth1, axisResults.normal, capEdge1, surfaceFlags);
                }
            }
        }
    }
}

bool __cdecl Phys_CapsuleSeparatingAxisTestEndEdges(
    AxisTestResults *axisResults,
    const float *plane,
    const Capsule *capsule,
    const float *tri0,
    const float *tri1,
    const float *tri2)
{
    float scale; // [esp+0h] [ebp-138h]
    float v8[3]; // [esp+Ch] [ebp-12Ch] BYREF
    float v9[3]; // [esp+18h] [ebp-120h] BYREF
    float v0[3]; // [esp+24h] [ebp-114h] BYREF
    float v11[3]; // [esp+30h] [ebp-108h] BYREF
    float diff[3]; // [esp+3Ch] [ebp-FCh] BYREF
    float cross[35]; // [esp+48h] [ebp-F0h] BYREF
    float *normal; // [esp+D4h] [ebp-64h]
    float cp0[3]; // [esp+D8h] [ebp-60h] BYREF
    float e1[3]; // [esp+E4h] [ebp-54h] BYREF
    float testAxis[3]; // [esp+F0h] [ebp-48h] BYREF
    float p1[3]; // [esp+FCh] [ebp-3Ch] BYREF
    float cp1[3]; // [esp+108h] [ebp-30h] BYREF
    float p2[3]; // [esp+114h] [ebp-24h] BYREF
    float p0[3]; // [esp+120h] [ebp-18h] BYREF
    float e2[3]; // [esp+12Ch] [ebp-Ch] BYREF

    Vec3Mad(capsule->center, capsule->halfHeight, capsule->axis, cp0);
    scale = -capsule->halfHeight;
    Vec3Mad(capsule->center, scale, capsule->axis, cp1);
    axisResults->bestDepth = -3.4028235e38;
    axisResults->bestCenter = 0.0;
    axisResults->bestAxis = 0;
    axisResults->bestRt = 0.0;
    normal = axisResults->normal;
    axisResults->normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 0.0;
    Vec3Sub(tri0, capsule->center, p0);
    Vec3Sub(tri1, capsule->center, p1);
    Vec3Sub(tri2, capsule->center, p2);
    testAxis[0] = -*plane;
    testAxis[1] = -plane[1];
    testAxis[2] = -plane[2];
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 1, 1))
        return 0;
    Vec3Sub(tri2, tri1, e1);
    Vec3Cross(capsule->axis, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 3, 0))
        return 0;
    Vec3Sub(tri0, tri2, e2);
    Vec3Cross(capsule->axis, e2, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 4, 0))
        return 0;
    Vec3Sub(cp0, tri1, diff);
    Vec3Cross(diff, e1, cross);
    Vec3Cross(cross, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 6, 0))
        return 0;
    Vec3Sub(cp0, tri2, v0);
    Vec3Cross(v0, e2, v11);
    Vec3Cross(v11, e2, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 7, 0))
        return 0;
    Vec3Sub(cp1, tri1, v8);
    Vec3Cross(v8, e1, v9);
    Vec3Cross(v9, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 9, 0))
        return 0;
    Phys_CalcAxis(cp1, tri2, e2, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 10, 0))
        return 0;
    Phys_CalcAxis(tri1, cp0, capsule->axis, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 12, 0))
        return 0;
    Phys_CalcAxis(tri2, cp0, capsule->axis, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 13, 0))
        return 0;
    Vec3Sub(tri1, cp0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 15, 0))
        return 0;
    Vec3Sub(tri2, cp0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 16, 0))
        return 0;
    Vec3Sub(tri1, cp1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 18, 0))
        return 0;
    Vec3Sub(tri2, cp1, testAxis);
    return Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 19, 0) != 0;
}

void __cdecl Phys_CapsuleBuildContactsForTriMiddleEdge(
    Results *results,
    const float *plane,
    const Capsule *capsule,
    const float *p0,
    const float *p1,
    const float *p2,
    int surfaceFlags)
{
    double v7; // st7
    double v8; // st7
    float v9; // [esp+10h] [ebp-80h]
    float v10; // [esp+14h] [ebp-7Ch]
    float v11; // [esp+18h] [ebp-78h]
    float v12; // [esp+1Ch] [ebp-74h]
    float delta[3]; // [esp+20h] [ebp-70h] BYREF
    float e1[3]; // [esp+2Ch] [ebp-64h] BYREF
    float clipPlane[4]; // [esp+38h] [ebp-58h] BYREF
    float depth1; // [esp+48h] [ebp-48h]
    AxisTestResults axisResults; // [esp+4Ch] [ebp-44h] BYREF
    float capEdge1[3]; // [esp+68h] [ebp-28h] BYREF
    float e0[3]; // [esp+74h] [ebp-1Ch] BYREF
    float depth0; // [esp+80h] [ebp-10h]
    float capEdge0[3]; // [esp+84h] [ebp-Ch] BYREF

    if (Phys_CapsuleSeparatingAxisTestMiddleEdge(&axisResults, plane, capsule, p0, p1, p2))
    {
        Phys_CapsuleCenterEdgePts(&axisResults, capsule, capEdge0, capEdge1);
        Vec3Sub(capEdge0, p0, capEdge0);
        Vec3Sub(capEdge1, p0, capEdge1);
        clipPlane[0] = -*plane;
        clipPlane[1] = -plane[1];
        clipPlane[2] = -plane[2];
        clipPlane[3] = 0.0;
        if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
        {
            Vec3Sub(p1, p0, e0);
            Vec3Sub(p2, p1, e1);
            Vec3Cross(plane, e1, clipPlane);
            clipPlane[3] = -(Vec3Dot(e0, clipPlane) - 0.000001);
            if (Phys_CapsuleClipEdgeToPlane(capEdge0, capEdge1, clipPlane))
            {
                Vec3Add(capEdge0, p0, capEdge0);
                Vec3Add(capEdge1, p0, capEdge1);
                Vec3Sub(capEdge0, capsule->center, delta);
                v7 = Vec3Dot(delta, axisResults.normal);
                depth0 = v7 - (axisResults.bestCenter - axisResults.bestRt);
                Vec3Sub(capEdge1, capsule->center, delta);
                v8 = Vec3Dot(delta, axisResults.normal);
                depth1 = v8 - (axisResults.bestCenter - axisResults.bestRt);
                v12 = depth0 - 0.0;
                if (v12 < 0.0)
                    v11 = 0.0;
                else
                    v11 = depth0;
                depth0 = v11;
                v10 = depth1 - 0.0;
                if (v10 < 0.0)
                    v9 = 0.0;
                else
                    v9 = depth1;
                depth1 = v9;
                Phys_AddLocalContactData(depth0, axisResults.normal, capEdge0, surfaceFlags);
                Phys_AddLocalContactData(depth1, axisResults.normal, capEdge1, surfaceFlags);
            }
        }
    }
}

bool __cdecl Phys_CapsuleSeparatingAxisTestMiddleEdge(
    AxisTestResults *axisResults,
    const float *plane,
    const Capsule *capsule,
    const float *tri0,
    const float *tri1,
    const float *tri2)
{
    float scale; // [esp+0h] [ebp-BCh]
    float diff[3]; // [esp+Ch] [ebp-B0h] BYREF
    float cross[19]; // [esp+18h] [ebp-A4h] BYREF
    float *normal; // [esp+64h] [ebp-58h]
    float cp0[3]; // [esp+68h] [ebp-54h] BYREF
    float e1[3]; // [esp+74h] [ebp-48h] BYREF
    float testAxis[3]; // [esp+80h] [ebp-3Ch] BYREF
    float p1[3]; // [esp+8Ch] [ebp-30h] BYREF
    float cp1[3]; // [esp+98h] [ebp-24h] BYREF
    float p2[3]; // [esp+A4h] [ebp-18h] BYREF
    float p0[3]; // [esp+B0h] [ebp-Ch] BYREF

    Vec3Mad(capsule->center, capsule->halfHeight, capsule->axis, cp0);
    scale = -capsule->halfHeight;
    Vec3Mad(capsule->center, scale, capsule->axis, cp1);
    axisResults->bestDepth = -3.4028235e38;
    axisResults->bestCenter = 0.0;
    axisResults->bestAxis = 0;
    axisResults->bestRt = 0.0;
    normal = axisResults->normal;
    axisResults->normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 0.0;
    Vec3Sub(tri0, capsule->center, p0);
    Vec3Sub(tri1, capsule->center, p1);
    Vec3Sub(tri2, capsule->center, p2);
    testAxis[0] = -*plane;
    testAxis[1] = -plane[1];
    testAxis[2] = -plane[2];
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 1, 1))
        return 0;
    Vec3Sub(tri2, tri1, e1);
    Vec3Cross(capsule->axis, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 3, 0))
        return 0;
    Vec3Sub(cp0, tri1, diff);
    Vec3Cross(diff, e1, cross);
    Vec3Cross(cross, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 6, 0))
        return 0;
    Phys_CalcAxis(cp1, tri1, e1, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 9, 0))
        return 0;
    Phys_CalcAxis(tri1, cp0, capsule->axis, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 12, 0))
        return 0;
    Vec3Sub(tri1, cp0, testAxis);
    if (!Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 15, 0))
        return 0;
    Vec3Sub(tri1, cp1, testAxis);
    return Phys_TestAxis(axisResults, capsule, p0, p1, p2, testAxis, 18, 0) != 0;
}

bool __cdecl Phys_TestCapsulePlane(const float *plane, const Capsule *capsule)
{
    float v3; // [esp+4h] [ebp-20h]
    float v4; // [esp+8h] [ebp-1Ch]
    float v5; // [esp+Ch] [ebp-18h]
    float v6; // [esp+10h] [ebp-14h]
    float p0Dist; // [esp+1Ch] [ebp-8h]
    float p1Dist; // [esp+20h] [ebp-4h]

    p0Dist = Vec3Dot(capsule->p0, plane) - plane[3];
    p1Dist = Vec3Dot(capsule->p1, plane) - plane[3];
    if (p0Dist * p1Dist < 0.0)
        return 1;
    v6 = fabs(p1Dist);
    v5 = fabs(p0Dist);
    v4 = v6 - v5;
    if (v4 < 0.0)
        v3 = v6;
    else
        v3 = v5;
    return capsule->radius > v3;
}

void __cdecl Phys_CollideCapsuleWithTriangleList(
    const unsigned __int16 *a_indices,
    const float (*verts)[3],
    unsigned int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    const float *v6; // [esp+A4h] [ebp-ACh]
    const float *v7; // [esp+ACh] [ebp-A4h]
    const float *v8; // [esp+B0h] [ebp-A0h]
    float pos[3]; // [esp+B8h] [ebp-98h] BYREF
    float triVerts[3][3]; // [esp+C4h] [ebp-8Ch] BYREF
    float triPlane[4]; // [esp+E8h] [ebp-68h] BYREF
    Capsule capsule; // [esp+F8h] [ebp-58h] BYREF
    float radius; // [esp+13Ch] [ebp-14h]
    const unsigned __int16 *indices; // [esp+140h] [ebp-10h]
    unsigned int triIndex; // [esp+144h] [ebp-Ch]
    Poly triPoly; // [esp+148h] [ebp-8h]

    if (results->contactCount < results->maxContacts)
    {
        Phys_InfoToCapsule(info, &capsule);
        numLocalContacts = 0;
        triPoly.ptCount = 3;
        triPoly.pts = triVerts;
        indices = a_indices;
        pos[0] = info->pos[0];
        pos[1] = info->pos[1];
        pos[2] = info->pos[2];
        radius = info->radius;
        for (triIndex = 0; triIndex < triCount; ++triIndex)
        {
            v8 = &(*verts)[3 * *indices];
            triVerts[0][0] = *v8;
            triVerts[0][1] = v8[1];
            triVerts[0][2] = v8[2];
            v7 = &(*verts)[3 * *++indices];
            triVerts[1][0] = *v7;
            triVerts[1][1] = v7[1];
            triVerts[1][2] = v7[2];
            v6 = &(*verts)[3 * *++indices];
            triVerts[2][0] = *v6;
            triVerts[2][1] = v6[1];
            triVerts[2][2] = v6[2];
            ++indices;
            if (Phys_GetPlaneForTriangle2(triVerts, pos, radius, triPlane))
            {
                if (Phys_TestCapsulePlane(triPlane, &capsule))
                    Phys_CapsuleBuildContactsForTri(
                        results,
                        triPlane,
                        &capsule,
                        triVerts[0],
                        triVerts[2],
                        triVerts[1],
                        surfaceFlags);
            }
        }
        Phys_CapsuleOptimizeLocalResults(results);
    }
}

