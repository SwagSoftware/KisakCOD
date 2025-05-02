#include "phys_local.h"
#include "phys_coll_local.h"
#include <cgame/cg_local.h>
#include <EffectsCore/fx_system.h>


void __cdecl Phys_CollideCylinderWithBrush(const cbrush_t *brush, const objInfo *info, Results *results)
{
    double v3; // st7
    cplane_s *v4; // [esp+0h] [ebp-D58h]
    int v5; // [esp+30h] [ebp-D28h]
    cplane_s *v6; // [esp+34h] [ebp-D24h]
    float plane[4]; // [esp+3Ch] [ebp-D1Ch] BYREF
    float v8; // [esp+4Ch] [ebp-D0Ch]
    unsigned int j; // [esp+50h] [ebp-D08h]
    float v10; // [esp+54h] [ebp-D04h]
    float v11[12]; // [esp+58h] [ebp-D00h] BYREF
    float tempBrushPlane[4]; // [esp+B8h] [ebp-CA0h] BYREF
    unsigned int newBrushSideIndex; // [esp+C8h] [ebp-C90h]
    int surfaceFlags; // [esp+CCh] [ebp-C8Ch]
    float brushPlane[4]; // [esp+D0h] [ebp-C88h] BYREF
    int brushSideIndex; // [esp+E0h] [ebp-C78h]
    Poly brushPoly; // [esp+E4h] [ebp-C74h] BYREF
    float brushSeparation; // [esp+ECh] [ebp-C6Ch]
    float brushVerts[256][3]; // [esp+F0h] [ebp-C68h] BYREF
    unsigned int i; // [esp+CF4h] [ebp-64h]
    float axialPlanes[6][4]; // [esp+CF8h] [ebp-60h] BYREF

    if (results->contactCount >= results->maxContacts)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            801,
            0,
            "%s",
            "results->contactCount < results->maxContacts");
    if (!brush)
        MyAssertHandler("c:\\trees\\cod3\\src\\physics\\phys_coll_local.h", 175, 0, "%s", "brush");
    brushPlane[0] = 0.0;
    brushPlane[1] = 0.0;
    brushPlane[2] = 0.0;
    brushPlane[3] = 0.0;
    v10 = -3.4028235e38;
    brushSideIndex = -1;
    CM_BuildAxialPlanes(brush, (float(*)[4])v11);
    for (j = 0; j < 6; ++j)
    {
        v8 = Phys_DistanceOfCylinderFromPlane(&v11[4 * j], info);
        if (v8 >= 0.0)
        {
            v5 = 0;
            goto LABEL_24;
        }
        if (brush->edgeCount[j & 1][j >> 1] && v10 < v8)
        {
            v10 = v8;
            brushSideIndex = j;
            brushPlane[0] = v11[4 * j];
            brushPlane[1] = v11[4 * j + 1];
            brushPlane[2] = v11[4 * j + 2];
            brushPlane[3] = v11[4 * j + 3];
        }
    }
    for (j = 0; j < brush->numsides; ++j)
    {
        v6 = brush->sides[j].plane;
        plane[0] = v6->normal[0];
        plane[1] = v6->normal[1];
        plane[2] = v6->normal[2];
        plane[3] = brush->sides[j].plane->dist;
        v8 = Phys_DistanceOfCylinderFromPlane(plane, info);
        if (v8 >= 0.0)
        {
            v5 = 0;
            goto LABEL_24;
        }
        if (brush->sides[j].edgeCount && v10 < v8)
        {
            v10 = v8;
            brushSideIndex = j + 6;
            Vec4Copy(plane, brushPlane);
        }
    }
    brushSeparation = v10;
    v5 = 1;
LABEL_24:
    if (v5 && brushSideIndex >= 0)
    {
        CM_BuildAxialPlanes(brush, axialPlanes);
        brushPoly.pts = brushVerts;
        Phys_GetWindingForBrushFace2(brush, brushSideIndex, &brushPoly, 256, axialPlanes);
        if (phys_drawCollisionWorld->current.enabled)
            Phys_DrawPoly(&brushPoly, colorCyan);
        if (brushSideIndex >= brush->numsides + 6)
            MyAssertHandler(
                ".\\physics\\phys_coll_cylinderbrush.cpp",
                818,
                0,
                "brushSideIndex doesn't index static_cast< int >( brush->numsides ) + 6\n\t%i not in [0, %i)",
                brushSideIndex,
                brush->numsides + 6);
        surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, brushSideIndex);
        Phys_CollideCylinderWithFace(brushPlane, &brushPoly, info, surfaceFlags, results);
        if (info->isNarrow)
        {
            for (i = 0; i < 3; ++i)
            {
                if (brush->mins[i] <= info->pos[i])
                {
                    if (brush->maxs[i] < info->pos[i])
                    {
                        newBrushSideIndex = 2 * i + 1;
                        if (newBrushSideIndex != brushSideIndex)
                        {
                            Phys_GetWindingForBrushFace2(brush, newBrushSideIndex, &brushPoly, 256, axialPlanes);
                            if (brushPoly.ptCount)
                            {
                                if (phys_drawCollisionWorld->current.enabled)
                                    Phys_DrawPoly(&brushPoly, colorCyan);
                                surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, newBrushSideIndex);
                                Phys_CollideCylinderWithFace(axialPlanes[newBrushSideIndex], &brushPoly, info, surfaceFlags, results);
                            }
                        }
                    }
                }
                else
                {
                    newBrushSideIndex = 2 * i;
                    if (2 * i != brushSideIndex)
                    {
                        Phys_GetWindingForBrushFace2(brush, newBrushSideIndex, &brushPoly, 256, axialPlanes);
                        if (brushPoly.ptCount)
                        {
                            if (phys_drawCollisionWorld->current.enabled)
                                Phys_DrawPoly(&brushPoly, colorCyan);
                            surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, newBrushSideIndex);
                            Phys_CollideCylinderWithFace(axialPlanes[2 * i], &brushPoly, info, surfaceFlags, results);
                        }
                    }
                }
            }
            for (i = 0; i < brush->numsides; ++i)
            {
                newBrushSideIndex = i + 6;
                if (brushSideIndex != i + 6)
                {
                    v3 = Vec3Dot(info->pos, brush->sides[i].plane->normal);
                    if (brush->sides[i].plane->dist < v3)
                    {
                        Phys_GetWindingForBrushFace2(brush, newBrushSideIndex, &brushPoly, 256, axialPlanes);
                        if (brushPoly.ptCount)
                        {
                            if (phys_drawCollisionWorld->current.enabled)
                                Phys_DrawPoly(&brushPoly, colorCyan);
                            v4 = brush->sides[i].plane;
                            tempBrushPlane[0] = v4->normal[0];
                            tempBrushPlane[1] = v4->normal[1];
                            tempBrushPlane[2] = v4->normal[2];
                            tempBrushPlane[3] = brush->sides[i].plane->dist;
                            surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, newBrushSideIndex);
                            Phys_CollideCylinderWithFace(tempBrushPlane, &brushPoly, info, surfaceFlags, results);
                        }
                    }
                }
            }
        }
    }
}
void __cdecl Phys_CollideCylinderWithFace(
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    float v5; // [esp+10h] [ebp-38h]
    unsigned int nextVertIndex; // [esp+14h] [ebp-34h]
    unsigned int nextVertIndexa; // [esp+14h] [ebp-34h]
    float edge[3]; // [esp+18h] [ebp-30h] BYREF
    float distanceBodyCenterToPlane; // [esp+24h] [ebp-24h]
    SeparatingAxisInfo axisInfo; // [esp+28h] [ebp-20h] BYREF
    unsigned int direction; // [esp+3Ch] [ebp-Ch]
    float dot; // [esp+40h] [ebp-8h]
    float distanceCylinderCenterToPlane; // [esp+44h] [ebp-4h]

    direction = info->cylDirection - 1;
    distanceCylinderCenterToPlane = Vec3Dot(info->pos, polyPlane) - polyPlane[3];
    distanceBodyCenterToPlane = Vec3Dot(info->bodyCenter, polyPlane) - polyPlane[3];
    if ((distanceCylinderCenterToPlane > 0.0 || distanceBodyCenterToPlane > 0.0)
        && Phys_CylinderFaceTestSeparatingAxes(polyPlane, poly, info, &axisInfo))
    {
        if (!axisInfo.bestAxis)
            MyAssertHandler(".\\physics\\phys_coll_cylinderbrush.cpp", 705, 0, "%s", "axisInfo.bestAxis");
        if (axisInfo.bestAxis == 1)
        {
            dot = Vec3Dot(axisInfo.bestContactNormal, info->R[direction]);
            v5 = fabs(dot);
            if (v5 >= 0.9)
                Phys_ClipCylinderEndcapToPoly(polyPlane, poly, info, surfaceFlags, results);
            else
                Phys_ClipCylinderEdgeToPoly(&axisInfo, polyPlane, poly, info, surfaceFlags, results);
        }
        else if (axisInfo.bestAxis == 2)
        {
            Phys_PushPolyOutOfCylinderEndcapPlane(results, poly, polyPlane, info, surfaceFlags);
        }
        else
        {
            axisInfo.bestAxis -= 3;
            if (axisInfo.bestAxis >= poly->ptCount)
            {
                axisInfo.bestAxis -= poly->ptCount;
                if (axisInfo.bestAxis >= poly->ptCount)
                {
                    axisInfo.bestAxis -= poly->ptCount;
                    if (axisInfo.bestAxis >= 2 * poly->ptCount)
                        MyAssertHandler(
                            ".\\physics\\phys_coll_cylinderbrush.cpp",
                            748,
                            0,
                            "%s",
                            "axisInfo.bestAxis < poly->ptCount * 2");
                    axisInfo.bestAxis >>= 1;
                    nextVertIndexa = axisInfo.bestAxis + 1;
                    if (axisInfo.bestAxis + 1 == poly->ptCount)
                        nextVertIndexa = 0;
                    Vec3Sub(poly->pts[nextVertIndexa], poly->pts[axisInfo.bestAxis], edge);
                    Phys_PushEdgeAwayFromCylinderCircle(
                        poly->pts[axisInfo.bestAxis],
                        poly->pts[nextVertIndexa],
                        edge,
                        axisInfo.bestContactNormal,
                        info,
                        surfaceFlags,
                        results);
                }
                else
                {
                    Phys_AddContactData(
                        results,
                        axisInfo.bestDepth,
                        axisInfo.bestContactNormal,
                        poly->pts[axisInfo.bestAxis],
                        surfaceFlags);
                }
            }
            else
            {
                nextVertIndex = axisInfo.bestAxis + 1;
                if (axisInfo.bestAxis + 1 == poly->ptCount)
                    nextVertIndex = 0;
                Vec3Sub(poly->pts[nextVertIndex], poly->pts[axisInfo.bestAxis], edge);
                Phys_PushLinesAway(
                    info->pos,
                    info->R[direction],
                    poly->pts[axisInfo.bestAxis],
                    edge,
                    axisInfo.bestContactNormal,
                    axisInfo.bestDepth,
                    results,
                    surfaceFlags);
            }
        }
    }
}

void __cdecl Phys_PushPolyOutOfCylinderEndcapPlane(
    Results *results,
    const Poly *poly,
    const float *polyNormal,
    const objInfo *info,
    int surfaceFlags)
{
    double v5; // st7
    float v6; // [esp+10h] [ebp-C30h]
    float depth; // [esp+14h] [ebp-C2Ch]
    unsigned int clippedPolyCount; // [esp+1Ch] [ebp-C24h]
    float clippedPoly[256][3]; // [esp+20h] [ebp-C20h] BYREF
    float dist; // [esp+C24h] [ebp-1Ch]
    unsigned int ptIndex; // [esp+C28h] [ebp-18h]
    float negR[3]; // [esp+C2Ch] [ebp-14h] BYREF
    int direction; // [esp+C38h] [ebp-8h]
    float centerDist; // [esp+C3Ch] [ebp-4h]

    direction = info->cylDirection - 1;
    centerDist = Vec3Dot(info->R[direction], info->pos);
    clippedPolyCount = Phys_ClipPolygonAgainstCylinderRadius(poly, info, clippedPoly, 0x100u);
    for (ptIndex = 0; ptIndex < clippedPolyCount; ++ptIndex)
    {
        v5 = Vec3Dot(info->R[direction], clippedPoly[ptIndex]);
        dist = v5 - centerDist;
        if (Vec3Dot(info->R[direction], polyNormal) >= 0.0)
        {
            if (dist >= -info->u.sideExtents[2])
            {
                negR[0] = -info->R[direction][0];
                negR[1] = -info->R[direction][1];
                negR[2] = -info->R[direction][2];
                v6 = info->u.sideExtents[2] + dist;
                Phys_AddContactData(results, v6, negR, clippedPoly[ptIndex], surfaceFlags);
            }
        }
        else if (info->u.sideExtents[2] >= dist)
        {
            depth = info->u.sideExtents[2] - dist;
            Phys_AddContactData(results, depth, (float*)info->R[direction], clippedPoly[ptIndex], surfaceFlags);
        }
    }
}

unsigned int __cdecl Phys_ClipPolygonAgainstCylinderRadius(
    const Poly *poly,
    const objInfo *info,
    float (*result)[3],
    unsigned int maxVerts)
{
    double v4; // st7
    double v6; // st7
    float *v7; // [esp+8h] [ebp-78h]
    float *v8; // [esp+10h] [ebp-70h]
    float *v9; // [esp+14h] [ebp-6Ch]
    float *v10; // [esp+18h] [ebp-68h]
    float *v11; // [esp+1Ch] [ebp-64h]
    float *v12; // [esp+20h] [ebp-60h]
    float *v13; // [esp+28h] [ebp-58h]
    float *v14; // [esp+2Ch] [ebp-54h]
    float cross[3]; // [esp+34h] [ebp-4Ch] BYREF
    float radiusSq; // [esp+40h] [ebp-40h]
    bool isInsidePrev; // [esp+47h] [ebp-39h]
    unsigned int currIndex; // [esp+48h] [ebp-38h]
    bool isInsideCurrent; // [esp+4Fh] [ebp-31h]
    float intersection[2][3]; // [esp+50h] [ebp-30h] BYREF
    float toPt[3]; // [esp+68h] [ebp-18h] BYREF
    unsigned int outVertIndex; // [esp+74h] [ebp-Ch]
    int direction; // [esp+78h] [ebp-8h]
    unsigned int lastIndex; // [esp+7Ch] [ebp-4h]

    if (poly->ptCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_cylinderbrush.cpp", 96, 0, "%s", "poly->ptCount > 2");
    radiusSq = info->u.sideExtents[0] * info->u.sideExtents[0];
    outVertIndex = 0;
    lastIndex = poly->ptCount - 1;
    direction = info->cylDirection - 1;
    Vec3Sub(poly->pts[poly->ptCount - 1], info->pos, toPt);
    Vec3Cross(info->R[direction], toPt, cross);
    v4 = Vec3LengthSq(cross);
    isInsidePrev = radiusSq >= v4;
    for (currIndex = 0; currIndex < poly->ptCount; ++currIndex)
    {
        if (outVertIndex >= maxVerts)
            return outVertIndex;
        Vec3Sub(poly->pts[currIndex], info->pos, toPt);
        Vec3Cross(info->R[direction], toPt, cross);
        v6 = Vec3LengthSq(cross);
        isInsideCurrent = radiusSq >= v6;
        if (isInsidePrev)
        {
            if (isInsideCurrent)
            {
                v13 = &(*result)[3 * outVertIndex];
                v14 = poly->pts[currIndex];
                *v13 = *v14;
                v13[1] = v14[1];
                v13[2] = v14[2];
            }
            else
            {
                Phys_ClipLineSegmentAgainstCylinderRadius(poly->pts[lastIndex], poly->pts[currIndex], info, intersection);
                v12 = &(*result)[3 * outVertIndex];
                *v12 = intersection[1][0];
                v12[1] = intersection[1][1];
                v12[2] = intersection[1][2];
            }
            ++outVertIndex;
        }
        else if (isInsideCurrent)
        {
            Phys_ClipLineSegmentAgainstCylinderRadius(poly->pts[lastIndex], poly->pts[currIndex], info, intersection);
            v11 = &(*result)[3 * outVertIndex];
            *v11 = intersection[0][0];
            v11[1] = intersection[0][1];
            v11[2] = intersection[0][2];
            if (++outVertIndex >= maxVerts)
                return outVertIndex;
            v9 = &(*result)[3 * outVertIndex];
            v10 = poly->pts[currIndex];
            *v9 = *v10;
            v9[1] = v10[1];
            v9[2] = v10[2];
            ++outVertIndex;
        }
        else if (Phys_ClipLineSegmentAgainstCylinderRadius(poly->pts[lastIndex], poly->pts[currIndex], info, intersection))
        {
            v8 = &(*result)[3 * outVertIndex];
            *v8 = intersection[0][0];
            v8[1] = intersection[0][1];
            v8[2] = intersection[0][2];
            if (++outVertIndex >= maxVerts)
                return outVertIndex;
            v7 = &(*result)[3 * outVertIndex];
            *v7 = intersection[1][0];
            v7[1] = intersection[1][1];
            v7[2] = intersection[1][2];
            ++outVertIndex;
        }
        isInsidePrev = isInsideCurrent;
        lastIndex = currIndex;
    }
    return outVertIndex;
}

unsigned int __cdecl Phys_ClipLineSegmentAgainstCylinderRadius(
    const float *pt1,
    const float *pt2,
    const objInfo *info,
    float (*result)[3])
{
    float v5; // [esp+8h] [ebp-60h]
    float v6; // [esp+Ch] [ebp-5Ch]
    float pt1Rel[3]; // [esp+18h] [ebp-50h] BYREF
    float edge[3]; // [esp+24h] [ebp-44h] BYREF
    float A; // [esp+30h] [ebp-38h]
    float t1; // [esp+34h] [ebp-34h]
    float C; // [esp+38h] [ebp-30h]
    float discriminant; // [esp+3Ch] [ebp-2Ch]
    float RCrossE[3]; // [esp+40h] [ebp-28h] BYREF
    float RCrossPt1[3]; // [esp+4Ch] [ebp-1Ch] BYREF
    float B; // [esp+58h] [ebp-10h]
    float t2; // [esp+5Ch] [ebp-Ch]
    float oneOverTwoA; // [esp+60h] [ebp-8h]
    int direction; // [esp+64h] [ebp-4h]

    direction = info->cylDirection - 1;
    Vec3Sub(pt2, pt1, edge);
    Vec3Sub(pt1, info->pos, pt1Rel);
    Vec3Cross(info->R[direction], pt1Rel, RCrossPt1);
    Vec3Cross(info->R[direction], edge, RCrossE);
    A = Vec3LengthSq(RCrossE);
    if (A < 0.001000000047497451)
        return 0;
    B = Vec3Dot(RCrossPt1, RCrossE) * 2.0;
    v6 = info->u.sideExtents[0] * info->u.sideExtents[0];
    C = Vec3LengthSq(RCrossPt1) - v6;
    discriminant = B * B - A * 4.0 * C;
    if (discriminant < 0.0)
        return 0;
    v5 = sqrt(discriminant);
    discriminant = v5;
    oneOverTwoA = 0.5 / A;
    t1 = (-B - v5) * oneOverTwoA;
    t2 = (v5 - B) * oneOverTwoA;
    if (t1 > 1.0 || t2 < 0.0)
        return 0;
    if (t1 <= 0.0)
    {
        (*result)[0] = *pt1;
        (*result)[1] = pt1[1];
        (*result)[2] = pt1[2];
    }
    else
    {
        Vec3Lerp(pt1, pt2, t1, (float*)result);
    }
    if (t2 >= 1.0)
    {
        (*result)[3] = *pt2;
        (*result)[4] = pt2[1];
        (*result)[5] = pt2[2];
    }
    else
    {
        Vec3Lerp(pt1, pt2, t2, &(*result)[3]);
    }
    return 2;
}

char __cdecl Phys_CylinderFaceTestSeparatingAxes(
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    SeparatingAxisInfo *axisInfo)
{
    float scale; // [esp+4h] [ebp-C84h]
    float scalea; // [esp+4h] [ebp-C84h]
    const float *v9; // [esp+8h] [ebp-C80h]
    bool v10; // [esp+1Bh] [ebp-C6Dh]
    float v11; // [esp+1Ch] [ebp-C6Ch]
    float v12; // [esp+38h] [ebp-C50h]
    float clockwiseTestDir[3]; // [esp+3Ch] [ebp-C4Ch] BYREF
    float testAxis[3]; // [esp+48h] [ebp-C40h] BYREF
    float endcapCenter1[3]; // [esp+54h] [ebp-C34h] BYREF
    float toVert[3]; // [esp+60h] [ebp-C28h] BYREF
    float endcapCenter0[3]; // [esp+6Ch] [ebp-C1Ch] BYREF
    unsigned int vertIndexNext; // [esp+78h] [ebp-C10h]
    unsigned int direction; // [esp+7Ch] [ebp-C0Ch]
    float edges[256][3]; // [esp+80h] [ebp-C08h] BYREF
    unsigned int vertIndex; // [esp+C80h] [ebp-8h]
    unsigned int testAxisNumber; // [esp+C84h] [ebp-4h]

    if (poly->ptCount > 0x100)
        MyAssertHandler(".\\physics\\phys_coll_cylinderbrush.cpp", 373, 0, "%s", "poly->ptCount <= ARRAY_COUNT( edges )");
    direction = info->cylDirection - 1;
    if (direction >= 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            376,
            0,
            "direction doesn't index 3\n\t%i not in [0, %i)",
            direction,
            3);
    axisInfo->bestAxis = 0;
    axisInfo->bestDepth = 3.4028235e38;
    testAxis[0] = -*polyPlane;
    testAxis[1] = -polyPlane[1];
    testAxis[2] = -polyPlane[2];
    if (!Phys_CylinderFaceTestAxis(polyPlane, poly, info, testAxis, 0.0, axisInfo, 1u, 0))
        return 0;
    if (!Phys_CylinderFaceTestAxis(polyPlane, poly, info, info->R[direction], 0.0, axisInfo, 2u, 0))
        return 0;
    for (vertIndex = 0; vertIndex < poly->ptCount; ++vertIndex)
    {
        vertIndexNext = vertIndex + 1;
        if (vertIndex + 1 == poly->ptCount)
            vertIndexNext = 0;
        Vec3Sub(poly->pts[vertIndexNext], poly->pts[vertIndex], edges[vertIndex]);
    }
    v12 = Vec3Dot(polyPlane, info->R[direction]);
    v11 = fabs(v12);
    if (v11 <= 0.01999999955296516)
    {
        testAxisNumber = poly->ptCount + 3;
    }
    else
    {
        testAxisNumber = 3;
        vertIndex = 0;
        while (vertIndex < poly->ptCount)
        {
            Vec3Cross(info->R[direction], edges[vertIndex], testAxis);
            if (Vec3Dot(testAxis, polyPlane) > 0.0)
            {
                testAxis[0] = -testAxis[0];
                testAxis[1] = -testAxis[1];
                testAxis[2] = -testAxis[2];
            }
            Vec3Cross(edges[vertIndex], polyPlane, clockwiseTestDir);
            if (Vec3Normalize(testAxis) > 0.000009999999747378752)
            {
                v10 = Vec3Dot(testAxis, clockwiseTestDir) < 0.0;
                if (!Phys_CylinderFaceTestAxis(polyPlane, poly, info, testAxis, 0.0, axisInfo, testAxisNumber, v10))
                    return 0;
            }
            ++vertIndex;
            ++testAxisNumber;
        }
    }
    if (testAxisNumber != poly->ptCount + 3)
        MyAssertHandler(".\\physics\\phys_coll_cylinderbrush.cpp", 425, 0, "%s", "testAxisNumber == 3 + poly->ptCount");
    for (vertIndex = 0; vertIndex < poly->ptCount; ++vertIndex)
    {
        Vec3Sub(poly->pts[vertIndex], info->pos, toVert);
        v9 = info->R[direction];
        scale = -Vec3Dot(toVert, v9);
        Vec3Mad(toVert, scale, v9, testAxis);
        if (Vec3Normalize(testAxis) > 0.000009999999747378752
            && !Phys_CylinderFaceTestAxis(polyPlane, poly, info, testAxis, 0.0, axisInfo, testAxisNumber, 0))
        {
            return 0;
        }
        ++testAxisNumber;
    }
    if (testAxisNumber != 2 * poly->ptCount + 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            442,
            0,
            "%s",
            "testAxisNumber == 3 + ( 2 * poly->ptCount )");
    Vec3Mad(info->pos, info->u.sideExtents[2], info->R[direction], endcapCenter0);
    scalea = -info->u.sideExtents[2];
    Vec3Mad(info->pos, scalea, info->R[direction], endcapCenter1);
    for (vertIndex = 0; vertIndex < poly->ptCount; ++vertIndex)
    {
        if (!Phys_TestCircleToEdgeAxis(
            polyPlane,
            poly,
            poly->pts[vertIndex],
            edges[vertIndex],
            info,
            endcapCenter0,
            axisInfo,
            testAxisNumber++))
            return 0;
        if (!Phys_TestCircleToEdgeAxis(
            polyPlane,
            poly,
            poly->pts[vertIndex],
            edges[vertIndex],
            info,
            endcapCenter1,
            axisInfo,
            testAxisNumber++))
            return 0;
    }
    if (testAxisNumber != 4 * poly->ptCount + 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            456,
            0,
            "%s",
            "testAxisNumber == 3 + ( 4 * poly->ptCount )");
    return 1;
}

char __cdecl Phys_CylinderFaceTestAxis(
    const float *polyNormal,
    const Poly *poly,
    const objInfo *info,
    const float *axis,
    float depthEpsilon,
    SeparatingAxisInfo *axisInfo,
    unsigned int axisNumber,
    bool testForRejectionOnly)
{
    double v8; // st7
    float v10; // [esp+0h] [ebp-78h]
    float v11; // [esp+4h] [ebp-74h]
    float v12; // [esp+8h] [ebp-70h]
    float v13; // [esp+Ch] [ebp-6Ch]
    float v14; // [esp+10h] [ebp-68h]
    float v15; // [esp+14h] [ebp-64h]
    float v16; // [esp+18h] [ebp-60h]
    float v17; // [esp+1Ch] [ebp-5Ch]
    float v18; // [esp+20h] [ebp-58h]
    float v19; // [esp+24h] [ebp-54h]
    float v20; // [esp+28h] [ebp-50h]
    float v21; // [esp+2Ch] [ebp-4Ch]
    float v22; // [esp+30h] [ebp-48h]
    float v23; // [esp+3Ch] [ebp-3Ch]
    float depth; // [esp+44h] [ebp-34h]
    float polyMin; // [esp+48h] [ebp-30h]
    float polyMax; // [esp+50h] [ebp-28h]
    float cylHalfLengthAlongAxisa; // [esp+54h] [ebp-24h]
    float cylHalfLengthAlongAxis; // [esp+54h] [ebp-24h]
    float normalizedAxis[3]; // [esp+58h] [ebp-20h] BYREF
    float axisNormalDot; // [esp+64h] [ebp-14h]
    unsigned int direction; // [esp+68h] [ebp-10h]
    float vertDist; // [esp+6Ch] [ebp-Ch]
    unsigned int vertIndex; // [esp+70h] [ebp-8h]
    float cylCenterDist; // [esp+74h] [ebp-4h]

    if (!Vec3IsNormalized(axis))
        MyAssertHandler(".\\physics\\phys_coll_cylinderbrush.cpp", 269, 0, "%s", "Vec3IsNormalized( axis )");
    direction = info->cylDirection - 1;
    if (direction >= 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            272,
            0,
            "direction doesn't index 3\n\t%i not in [0, %i)",
            direction,
            3);
    axisNormalDot = -Vec3Dot(axis, polyNormal);
    if (axisNormalDot < 0.0)
        v22 = -*axis;
    else
        v22 = *axis;
    normalizedAxis[0] = v22;
    if (axisNormalDot < 0.0)
        v21 = -axis[1];
    else
        v21 = axis[1];
    normalizedAxis[1] = v21;
    if (axisNormalDot < 0.0)
        v20 = -axis[2];
    else
        v20 = axis[2];
    normalizedAxis[2] = v20;
    v23 = Vec3Dot(info->R[direction], normalizedAxis);
    v19 = fabs(v23);
    v18 = 1.0 - v19;
    if (v18 < 0.0)
        v17 = 1.0;
    else
        v17 = v19;
    cylHalfLengthAlongAxisa = info->u.sideExtents[2] * v17;
    v16 = v17 * v17;
    v15 = 1.0 - v16;
    v14 = sqrt(v15);
    cylHalfLengthAlongAxis = info->u.sideExtents[0] * v14 + cylHalfLengthAlongAxisa;
    polyMin = 3.4028235e38;
    polyMax = -3.4028235e38;
    cylCenterDist = Vec3Dot(info->pos, normalizedAxis);
    for (vertIndex = 0; vertIndex < poly->ptCount; ++vertIndex)
    {
        v8 = Vec3Dot(poly->pts[vertIndex], normalizedAxis);
        vertDist = v8 - cylCenterDist;
        v13 = vertDist - polyMin;
        if (v13 < 0.0)
            v12 = vertDist;
        else
            v12 = polyMin;
        polyMin = v12;
        v11 = polyMax - vertDist;
        if (v11 < 0.0)
            v10 = vertDist;
        else
            v10 = polyMax;
        polyMax = v10;
    }
    if (cylHalfLengthAlongAxis < polyMin)
        return 0;
    if (polyMax < -cylHalfLengthAlongAxis)
        return 0;
    if (testForRejectionOnly)
        return 1;
    depth = cylHalfLengthAlongAxis - polyMin;
    if (axisInfo->bestDepth > depth + depthEpsilon)
    {
        axisInfo->bestDepth = depth;
        axisInfo->bestContactNormal[0] = normalizedAxis[0];
        axisInfo->bestContactNormal[1] = normalizedAxis[1];
        axisInfo->bestContactNormal[2] = normalizedAxis[2];
        axisInfo->bestAxis = axisNumber;
    }
    return 1;
}

char __cdecl Phys_TestCircleToEdgeAxis(
    const float *polyNormal,
    const Poly *poly,
    const float *pt1,
    const float *edge,
    const objInfo *info,
    const float *circleCenter,
    SeparatingAxisInfo *axisInfo,
    unsigned int axisNumber)
{
    float scale; // [esp+10h] [ebp-70h]
    double v10; // [esp+14h] [ebp-6Ch]
    float v11; // [esp+1Ch] [ebp-64h]
    float linePlaneIntersection[3]; // [esp+2Ch] [ebp-54h] BYREF
    float testAxis[3]; // [esp+38h] [ebp-48h] BYREF
    float tangent[3]; // [esp+44h] [ebp-3Ch] BYREF
    float toCenter[3]; // [esp+50h] [ebp-30h] BYREF
    float normalizedEdge[3]; // [esp+5Ch] [ebp-24h] BYREF
    float axialLengthOfNormalizedEdge; // [esp+68h] [ebp-18h]
    float distToCirclePlane; // [esp+6Ch] [ebp-14h]
    unsigned int direction; // [esp+70h] [ebp-10h]
    float clockwiseTestDir[3]; // [esp+74h] [ebp-Ch] BYREF

    direction = info->cylDirection - 1;
    if (direction >= 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            331,
            0,
            "direction doesn't index 3\n\t%i not in [0, %i)",
            direction,
            3);
    if (Vec3NormalizeTo(edge, normalizedEdge) < 0.000009999999747378752)
        return 1;
    axialLengthOfNormalizedEdge = Vec3Dot(normalizedEdge, info->R[direction]);
    v11 = fabs(axialLengthOfNormalizedEdge);
    if (v11 < 0.000009999999747378752)
        return 1;
    Vec3Cross(normalizedEdge, polyNormal, clockwiseTestDir);
    v10 = Vec3Dot(info->pos, clockwiseTestDir);
    if (Vec3Dot(pt1, clockwiseTestDir) < v10 && Vec3Dot(info->R[direction], clockwiseTestDir) > 0.0)
        return 1;
    Vec3Sub(circleCenter, pt1, toCenter);
    distToCirclePlane = Vec3Dot(toCenter, info->R[direction]);
    scale = distToCirclePlane / axialLengthOfNormalizedEdge;
    Vec3Mad(pt1, scale, normalizedEdge, linePlaneIntersection);
    Vec3Sub(circleCenter, linePlaneIntersection, toCenter);
    Vec3Cross(toCenter, info->R[direction], tangent);
    Vec3Cross(tangent, normalizedEdge, testAxis);
    Vec3Normalize(testAxis);
    return Phys_CylinderFaceTestAxis(polyNormal, poly, info, testAxis, 1.0, axisInfo, axisNumber, 0);
}

void __cdecl Phys_ClipCylinderEdgeToPoly(
    SeparatingAxisInfo *axisInfo,
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    float scale; // [esp+0h] [ebp-50h]
    float scalea; // [esp+0h] [ebp-50h]
    float normalPerp[3]; // [esp+18h] [ebp-38h] BYREF
    float endcapCenter1[3]; // [esp+24h] [ebp-2Ch] BYREF
    float cylPosTrans[3]; // [esp+30h] [ebp-20h] BYREF
    float endcapCenter0[3]; // [esp+3Ch] [ebp-14h] BYREF
    unsigned int direction; // [esp+48h] [ebp-8h]
    float dot; // [esp+4Ch] [ebp-4h]

    direction = info->cylDirection - 1;
    dot = Vec3Dot(info->R[direction], axisInfo->bestContactNormal);
    scale = -dot;
    Vec3Mad(axisInfo->bestContactNormal, scale, info->R[direction], normalPerp);
    if (Vec3Normalize(normalPerp) >= 0.00001)
    {
        Vec3Mad(info->pos, info->u.sideExtents[0], normalPerp, cylPosTrans);
        Vec3Mad(cylPosTrans, info->u.sideExtents[2], info->R[direction], endcapCenter0);
        scalea = -info->u.sideExtents[2];
        Vec3Mad(cylPosTrans, scalea, info->R[direction], endcapCenter1);
        if (Phys_ClipLineSegmentAgainstPoly(polyPlane, poly->pts, poly->ptCount, endcapCenter0, endcapCenter1))
        {
            Phys_AddContactForPlane(results, endcapCenter0, polyPlane, axisInfo->bestContactNormal, surfaceFlags);
            Phys_AddContactForPlane(results, endcapCenter1, polyPlane, axisInfo->bestContactNormal, surfaceFlags);
        }
    }
}

void __cdecl Phys_AddContactForPlane(Results *results, float *pt, const float *plane, float *normal, int surfaceFlags)
{
    float depth; // [esp+10h] [ebp-4h]

    depth = plane[3] - Vec3Dot(pt, plane);
    if (depth >= 0.0)
        Phys_AddContactData(results, depth, normal, pt, surfaceFlags);
}

void __cdecl Phys_PushEdgeAwayFromCylinderCircle(
    const float *ptOnEdge,
    const float *pt2OnEdge,
    const float *edge,
    float *contactNormal,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    float scale; // [esp+4h] [ebp-74h]
    float contactPlaneDist; // [esp+1Ch] [ebp-5Ch]
    float depth; // [esp+20h] [ebp-58h]
    float endcapCenter[3]; // [esp+24h] [ebp-54h] BYREF
    unsigned int contactCount; // [esp+30h] [ebp-48h]
    float choppingPlane[4]; // [esp+34h] [ebp-44h] BYREF
    unsigned int contactIndex; // [esp+44h] [ebp-34h]
    float edgeNormalized[3]; // [esp+48h] [ebp-30h] BYREF
    unsigned int direction; // [esp+54h] [ebp-24h]
    float length; // [esp+58h] [ebp-20h]
    float dot; // [esp+5Ch] [ebp-1Ch]
    float contactPt[2][3]; // [esp+60h] [ebp-18h] BYREF

    direction = info->cylDirection - 1;
    if (direction >= 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            565,
            0,
            "direction doesn't index 3\n\t%i not in [0, %i)",
            direction,
            3);
    length = Vec3NormalizeTo(edge, edgeNormalized);
    if (length >= 0.000009999999747378752)
    {
        Vec3Cross(edgeNormalized, contactNormal, choppingPlane);
        length = Vec3Normalize(choppingPlane);
        if (length <= 0.001000000047497451)
            MyAssertHandler(".\\physics\\phys_coll_cylinderbrush.cpp", 573, 0, "%s", "length > 1e-3f");
        choppingPlane[3] = Vec3Dot(choppingPlane, ptOnEdge);
        dot = Vec3Dot(info->R[direction], contactNormal);
        if (dot >= 0.0)
        {
            Vec3Mad(info->pos, info->u.sideExtents[2], info->R[direction], endcapCenter);
        }
        else
        {
            scale = -info->u.sideExtents[2];
            Vec3Mad(info->pos, scale, info->R[direction], endcapCenter);
        }
        contactCount = Phys_IntersectionOfCircleWithPlane(
            choppingPlane,
            endcapCenter,
            info->R[direction],
            info->u.sideExtents[0],
            contactPt[0],
            contactPt[1]);
        if (contactCount)
        {
            contactPlaneDist = Vec3Dot(ptOnEdge, contactNormal);
            choppingPlane[0] = edgeNormalized[0];
            choppingPlane[1] = edgeNormalized[1];
            choppingPlane[2] = edgeNormalized[2];
            choppingPlane[3] = Vec3Dot(pt2OnEdge, choppingPlane);
            if (Phys_ClipLineSegmentAgainstPlane(contactPt[0], contactPt[1], choppingPlane))
            {
                choppingPlane[0] = -edgeNormalized[0];
                choppingPlane[1] = -edgeNormalized[1];
                choppingPlane[2] = -edgeNormalized[2];
                choppingPlane[3] = Vec3Dot(ptOnEdge, choppingPlane);
                if (Phys_ClipLineSegmentAgainstPlane(contactPt[0], contactPt[1], choppingPlane))
                {
                    for (contactIndex = 0; contactIndex < contactCount; ++contactIndex)
                    {
                        depth = Vec3Dot(contactPt[contactIndex], contactNormal) - contactPlaneDist;
                        if (depth >= 0.0)
                            Phys_AddContactData(results, depth, contactNormal, contactPt[contactIndex], surfaceFlags);
                    }
                }
            }
        }
    }
}

unsigned int __cdecl Phys_IntersectionOfCircleWithPlane(
    const float *plane,
    const float *circleCenter,
    const float *circleAxis,
    float circleRadius,
    float *pt1,
    float *pt2)
{
    float scale; // [esp+0h] [ebp-74h]
    float v8; // [esp+Ch] [ebp-68h]
    float v9; // [esp+10h] [ebp-64h]
    float v10; // [esp+14h] [ebp-60h]
    float v11; // [esp+18h] [ebp-5Ch]
    float v12; // [esp+1Ch] [ebp-58h]
    float axis1[3]; // [esp+20h] [ebp-54h] BYREF
    float frac; // [esp+2Ch] [ebp-48h]
    float pointOnPlane[3]; // [esp+30h] [ebp-44h] BYREF
    float distAlongPlane; // [esp+3Ch] [ebp-38h]
    float closestDist; // [esp+40h] [ebp-34h]
    float axis2[3]; // [esp+44h] [ebp-30h] BYREF
    float axis0[3]; // [esp+50h] [ebp-24h] BYREF
    float closestPoint[3]; // [esp+5Ch] [ebp-18h] BYREF
    float dot; // [esp+68h] [ebp-Ch]
    float lengthSq; // [esp+6Ch] [ebp-8h]
    float centerDist; // [esp+70h] [ebp-4h]

    dot = Vec3Dot(plane, circleAxis);
    if (dot <= 0.0)
    {
        axis2[0] = *circleAxis;
        axis2[1] = circleAxis[1];
        axis2[2] = circleAxis[2];
    }
    else
    {
        axis2[0] = -*circleAxis;
        axis2[1] = -circleAxis[1];
        axis2[2] = -circleAxis[2];
        dot = -dot;
    }
    axis1[0] = -*plane;
    axis1[1] = -plane[1];
    axis1[2] = -plane[2];
    Vec3Mad(axis1, dot, axis2, axis1);
    lengthSq = Vec3LengthSq(axis1);
    if (lengthSq < 1.0e-10)
        return 0;
    v12 = sqrt(lengthSq);
    v11 = 1.0 / v12;
    Vec3Scale(axis1, v11, axis1);
    Vec3Mad(circleCenter, circleRadius, axis1, closestPoint);
    centerDist = Vec3Dot(circleCenter, plane) - plane[3];
    closestDist = Vec3Dot(closestPoint, plane) - plane[3];
    Vec3Cross(axis1, axis2, axis0);
    if (closestDist >= centerDist)
        return 0;
    frac = circleRadius * centerDist / (centerDist - closestDist);
    v10 = circleRadius * circleRadius;
    v9 = frac * frac;
    lengthSq = v10 - v9;
    if (lengthSq <= 0.0)
        return 0;
    v8 = sqrt(lengthSq);
    distAlongPlane = v8;
    Vec3Mad(circleCenter, frac, axis1, pointOnPlane);
    Vec3Mad(pointOnPlane, distAlongPlane, axis0, pt1);
    scale = -distAlongPlane;
    Vec3Mad(pointOnPlane, scale, axis0, pt2);
    return 2;
}

void __cdecl Phys_ClipCylinderEndcapToPoly(
    const float *polyPlane,
    const Poly *poly,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    float scale0; // [esp+14h] [ebp-118h]
    float v6; // [esp+18h] [ebp-114h]
    float v7; // [esp+1Ch] [ebp-110h]
    float v8; // [esp+20h] [ebp-10Ch]
    float endcapCenter[3]; // [esp+30h] [ebp-FCh] BYREF
    Poly circlePoly; // [esp+3Ch] [ebp-F0h] BYREF
    float axis1[3]; // [esp+44h] [ebp-E8h] BYREF
    float contactNormal[3]; // [esp+50h] [ebp-DCh] BYREF
    float circlePoints[25]; // [esp+5Ch] [ebp-D0h] BYREF
    const float *radius; // [esp+C0h] [ebp-6Ch]
    unsigned int pointIndex; // [esp+C4h] [ebp-68h]
    float axis2[3]; // [esp+C8h] [ebp-64h] BYREF
    float axis0[3]; // [esp+D4h] [ebp-58h] BYREF
    float rDotN; // [esp+E0h] [ebp-4Ch]
    unsigned int direction; // [esp+E4h] [ebp-48h]
    float lengthSq; // [esp+E8h] [ebp-44h]
    float circleCoords[8][2]; // [esp+ECh] [ebp-40h]

    radius = info->u.sideExtents;
    circleCoords[0][0] = 1.0;
    circleCoords[0][1] = 0.0;
    circleCoords[1][0] = 0.70710677;
    circleCoords[1][1] = 0.70710677;
    circleCoords[2][0] = 0.0;
    circleCoords[2][1] = 1.0;
    circleCoords[3][0] = -0.70710677;
    circleCoords[3][1] = 0.70710677;
    circleCoords[4][0] = -1.0;
    circleCoords[4][1] = 0.0;
    circleCoords[5][0] = -0.70710677;
    circleCoords[5][1] = -0.70710677;
    circleCoords[6][0] = 0.0;
    circleCoords[6][1] = -1.0;
    circleCoords[7][0] = 0.70710677;
    circleCoords[7][1] = -0.70710677;
    direction = info->cylDirection - 1;
    if (direction >= 3)
        MyAssertHandler(
            ".\\physics\\phys_coll_cylinderbrush.cpp",
            637,
            0,
            "direction doesn't index 3\n\t%i not in [0, %i)",
            direction,
            3);
    rDotN = Vec3Dot(info->R[direction], polyPlane);
    if (rDotN <= 0.0)
    {
        axis2[0] = info->R[direction][0];
        axis2[1] = info->R[direction][1];
        axis2[2] = info->R[direction][2];
    }
    else
    {
        axis2[0] = -info->R[direction][0];
        axis2[1] = -info->R[direction][1];
        axis2[2] = -info->R[direction][2];
        rDotN = -rDotN;
    }
    contactNormal[0] = -*polyPlane;
    contactNormal[1] = -polyPlane[1];
    contactNormal[2] = -polyPlane[2];
    Vec3Mad(contactNormal, rDotN, axis2, axis1);
    lengthSq = Vec3LengthSq(axis1);
    if (lengthSq >= 1.0e-10)
    {
        v8 = sqrt(lengthSq);
        v7 = 1.0 / v8;
        Vec3Scale(axis1, v7, axis1);
    }
    else
    {
        axis1[0] = info->R[(direction + 1) % 3][0];
        axis1[1] = info->R[(direction + 1) % 3][1];
        axis1[2] = info->R[(direction + 1) % 3][2];
    }
    Vec3Cross(axis1, axis2, axis0);
    Vec3Mad(info->pos, info->u.sideExtents[2], axis2, endcapCenter);
    for (pointIndex = 0; pointIndex < 8; ++pointIndex)
    {
        v6 = *radius * circleCoords[pointIndex][1];
        scale0 = *radius * circleCoords[pointIndex][0];
        Vec3MadMad(endcapCenter, scale0, axis0, v6, axis1, &circlePoints[3 * pointIndex]);
    }
    circlePoly.ptCount = 8;
    circlePoly.pts = (float(*)[3])circlePoints;
    Phys_ProjectFaceOntoFaceAndClip(polyPlane, poly, &circlePoly, surfaceFlags, results, contactNormal);
}

void __cdecl Phys_PushLinesAway(
    const float *p1,
    const float *dir1,
    const float *p2,
    const float *dir2,
    float *contactNormal,
    float depth,
    Results *results,
    int surfaceFlags)
{
    float t; // [esp+10h] [ebp-14h] BYREF
    float s; // [esp+14h] [ebp-10h] BYREF
    float contactPt[3]; // [esp+18h] [ebp-Ch] BYREF

    ClosestApproachOfTwoLines(p1, dir1, p2, dir2, &s, &t);
    Vec3Mad(p2, t, dir2, contactPt);
    Phys_AddContactData(results, depth, contactNormal, contactPt, surfaceFlags);
}

double __cdecl Phys_DistanceOfCylinderFromPlane(const float *plane, const objInfo *info)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    float v6; // [esp+Ch] [ebp-20h]
    float v7; // [esp+10h] [ebp-1Ch]
    float v8; // [esp+14h] [ebp-18h]
    float dist; // [esp+1Ch] [ebp-10h]
    float dista; // [esp+1Ch] [ebp-10h]
    int direction; // [esp+20h] [ebp-Ch]
    float centerDist; // [esp+28h] [ebp-4h]

    direction = info->cylDirection - 1;
    dist = Vec3Dot(info->pos, plane) - plane[3];
    centerDist = Vec3Dot(info->bodyCenter, plane) - plane[3];
    if (dist < 0.0 && centerDist < 0.0)
        return -3.4028235e38;
    v8 = Vec3Dot(info->R[direction], plane);
    v7 = fabs(v8);
    v6 = 1.0 - v7;
    if (v6 < 0.0)
        v5 = 1.0;
    else
        v5 = v7;
    v4 = 1.0 - v5 * v5;
    v3 = sqrt(v4);
    dista = dist - v5 * info->u.sideExtents[2];
    return (dista - v3 * info->u.sideExtents[0]);
}

void __cdecl Phys_CollideCylinderWithTriangleList(
    const unsigned __int16 *a_indices,
    const float (*verts)[3],
    unsigned int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    const float *v6; // [esp+A4h] [ebp-68h]
    const float *v7; // [esp+ACh] [ebp-60h]
    const float *v8; // [esp+B0h] [ebp-5Ch]
    float pos[3]; // [esp+B8h] [ebp-54h] BYREF
    float triVerts[3][3]; // [esp+C4h] [ebp-48h] BYREF
    float triPlane[4]; // [esp+E8h] [ebp-24h] BYREF
    float radius; // [esp+F8h] [ebp-14h]
    const unsigned __int16 *indices; // [esp+FCh] [ebp-10h]
    unsigned int triIndex; // [esp+100h] [ebp-Ch]
    Poly triPoly; // [esp+104h] [ebp-8h] BYREF

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
            Phys_CollideCylinderWithFace(triPlane, &triPoly, info, surfaceFlags, results);
    }
}

