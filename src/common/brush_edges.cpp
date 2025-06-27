#include "brush.h"
#include <universal/assertive.h>

#include <cstring>
#include <universal/com_math.h>
#include <qcommon/qcommon.h>

adjacencyWinding_t *__cdecl BuildBrushdAdjacencyWindingForSide(
    float *sideNormal,
    int basePlaneIndex,
    const SimplePlaneIntersection *InPts,
    int InPtCount,
    adjacencyWinding_t *optionalOutWinding,
    int optionalOutWindingCount)
{
    const SimplePlaneIntersection *v7; // eax
    float *v8; // [esp+10h] [ebp-6054h]
    const SimplePlaneIntersection *v9; // [esp+14h] [ebp-6050h]
    adjacencyWinding_t *w; // [esp+1Ch] [ebp-6048h]
    int i2; // [esp+20h] [ebp-6044h] BYREF
    float perimiter1; // [esp+24h] [ebp-6040h]
    float plane[4]; // [esp+28h] [ebp-603Ch] BYREF
    int ptsCount; // [esp+38h] [ebp-602Ch]
    float v0[3073]; // [esp+3Ch] [ebp-6028h] BYREF
    const SimplePlaneIntersection *xyz[1024]; // [esp+3044h] [ebp-3020h] BYREF
    const SimplePlaneIntersection *point[1024]; // [esp+4044h] [ebp-2020h] BYREF
    const SimplePlaneIntersection *v19[1024]; // [esp+5044h] [ebp-1020h] BYREF
    int i0; // [esp+6044h] [ebp-20h] BYREF
    char v21; // [esp+6048h] [ebp-1Ch]
    char v22; // [esp+6049h] [ebp-1Bh]
    int v23; // [esp+604Ch] [ebp-18h]
    unsigned int v24[2]; // [esp+6050h] [ebp-14h]
    int i1; // [esp+6058h] [ebp-Ch] BYREF
    float perimiter2; // [esp+605Ch] [ebp-8h]
    int plane2; // [esp+6060h] [ebp-4h]

    ptsCount = GetPointListAllowDupes(basePlaneIndex, InPts, InPtCount, xyz, 1024);
    if (ptsCount < 3)
        return 0;
    if (NumberOfUniquePoints(xyz, ptsCount) < 3)
        return 0;
    ptsCount = ReduceToACycle(basePlaneIndex, xyz, ptsCount);
    if (ptsCount < 3)
        return 0;
    v23 = 0;
    while (ptsCount)
    {
        point[1024 * v23] = xyz[--ptsCount];
        v24[v23] = 1;
        plane2 = SecondPlane(point[1024 * v23], basePlaneIndex);
        while (ptsCount)
        {
            v7 = RemoveNextPointFormedByThisPlane(plane2, xyz, &xyz[ptsCount]);
            *(&point[1024 * v23] + v24[v23]) = v7;
            if (!*(&point[1024 * v23] + v24[v23]))
                break;
            plane2 = ThirdPlane(*(&point[1024 * v23] + v24[v23]), basePlaneIndex, plane2);
            --ptsCount;
            ++v24[v23];
        }
        if (!IsPtFormedByThisPlane(plane2, point[1024 * v23]))
            MyAssertHandler(
                "..\\common\\brush_edges.cpp",
                832,
                1,
                "%s",
                "IsPtFormedByThisPlane( planeIndex, cycle[cycleIndex][0] )");
        if (v23 <= 0)
        {
            v23 = 1;
        }
        else
        {
            perimiter1 = CyclePerimiter(point, v24[0]);
            perimiter2 = CyclePerimiter(v19, v24[1]);
            v21 = TestConvexWithoutNearPoints(point, v24[0]);
            v22 = TestConvexWithoutNearPoints(v19, v24[1]);
            if (CycleLess(v21, v22, perimiter1, perimiter2, v24[0], v24[1]))
            {
                memcpy(point, v19, 4 * v24[1]);
                v24[0] = v24[1];
            }
        }
    }
    if (v24[0] <= 2)
        MyAssertHandler("..\\common\\brush_edges.cpp", 850, 1, "%s", "cycleCount[0] > 2");
    w = 0;
    if (optionalOutWinding)
    {
        w = optionalOutWinding;
        if (optionalOutWindingCount < v24[0])
        {
            Com_PrintError(1, "Brush face has too many edges");
            return 0;
        }
    }
    else if (!alwaysfails)
    {
        MyAssertHandler("..\\common\\brush_edges.cpp", 869, 0, "Not supported");
    }
    if (!w)
        MyAssertHandler("..\\common\\brush_edges.cpp", 873, 0, "%s", "winding");
    if (!PlaneInCommonExcluding(point[0], xyz[v24[0] + 1023], basePlaneIndex, w->sides))
        MyAssertHandler("..\\common\\brush_edges.cpp", 875, 1, "%s", "rv");
    v0[0] = point[0]->xyz[0];
    v0[1] = point[0]->xyz[1];
    v0[2] = point[0]->xyz[2];
    for (w->numsides = 1; w->numsides < v24[0]; ++w->numsides)
    {
        w->sides[w->numsides] = ThirdPlane(xyz[w->numsides + 1023], basePlaneIndex, *(&w->numsides + w->numsides));
        v8 = &v0[3 * w->numsides];
        v9 = point[w->numsides];
        *v8 = v9->xyz[0];
        v8[1] = v9->xyz[1];
        v8[2] = v9->xyz[2];
    }
    if (w->sides[0] != ThirdPlane(xyz[v24[0] + 1023], basePlaneIndex, *(&w->numsides + w->numsides)))
        MyAssertHandler(
            "..\\common\\brush_edges.cpp",
            884,
            1,
            "%s",
            "winding->sides[0] == ThirdPlane( cycle[0][cycleCount[0]-1], basePlaneIndex, winding->sides[winding->numsides-1] )");
    if (RepresentativeTriangleFromWinding((const float(*)[3])v0, w->numsides, sideNormal, &i0, &i1, &i2) < EQUAL_EPSILON)
        return 0;
    PlaneFromPoints(plane, &v0[3 * i0], &v0[3 * i1], &v0[3 * i2]);
    if (Vec3Dot(plane, sideNormal) < 0.0)
        ReverseAdjacencyWinding(w);
    return w;
}

void __cdecl ReverseAdjacencyWinding(adjacencyWinding_t *w)
{
    int swapTemp; // [esp+0h] [ebp-Ch]
    int *start; // [esp+4h] [ebp-8h]
    int *end; // [esp+8h] [ebp-4h]

    start = w->sides;
    for (end = &w->numsides + w->numsides; start < end; --end)
    {
        swapTemp = *start;
        *start = *end;
        *end = swapTemp;
        ++start;
    }
}

double __cdecl RepresentativeTriangleFromWinding(
    const float (*xyz)[3],
    int pointCount,
    const float *normal,
    int *i0,
    int *i1,
    int *i2)
{
    float v7; // [esp+0h] [ebp-40h]
    float v8; // [esp+4h] [ebp-3Ch]
    int j; // [esp+8h] [ebp-38h]
    float areaBest; // [esp+Ch] [ebp-34h]
    float vb[3]; // [esp+14h] [ebp-2Ch] BYREF
    int k; // [esp+20h] [ebp-20h]
    float vc[3]; // [esp+24h] [ebp-1Ch] BYREF
    float va[3]; // [esp+30h] [ebp-10h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    *i0 = 0;
    *i1 = 1;
    *i2 = 2;
    areaBest = 0.0;
    for (k = 2; k < pointCount; ++k)
    {
        for (j = 1; j < k; ++j)
        {
            Vec3Sub(&(*xyz)[3 * k], &(*xyz)[3 * j], vb);
            for (i = 0; i < j; ++i)
            {
                Vec3Sub(&(*xyz)[3 * i], &(*xyz)[3 * j], va);
                Vec3Cross(va, vb, vc);
                v8 = Vec3Dot(vc, normal);
                v7 = fabs(v8);
                if (areaBest < v7)
                {
                    areaBest = v7;
                    *i0 = i;
                    *i1 = j;
                    *i2 = k;
                }
            }
        }
    }
    return areaBest;
}

int __cdecl GetPointListAllowDupes(
    int planeIndex,
    const SimplePlaneIntersection *pts,
    int ptCount,
    const SimplePlaneIntersection **xyz,
    int xyzLimit)
{
    int xyzCount; // [esp+0h] [ebp-8h]
    int ptIndex; // [esp+4h] [ebp-4h]

    xyzCount = 0;
    for (ptIndex = 0; ptIndex < ptCount; ++ptIndex)
    {
        if (planeIndex == pts[ptIndex].planeIndex[0]
            || planeIndex == pts[ptIndex].planeIndex[1]
            || planeIndex == pts[ptIndex].planeIndex[2])
        {
            if (xyzCount == xyzLimit)
                return 0;
            xyz[xyzCount++] = &pts[ptIndex];
        }
    }
    return xyzCount;
}

bool __cdecl IsPtFormedByThisPlane(int plane, const SimplePlaneIntersection *pt)
{
    if (pt->planeIndex[0] == plane)
        return 1;
    if (pt->planeIndex[1] == plane)
        return 1;
    return pt->planeIndex[2] == plane;
}

char __cdecl PlaneInCommonExcluding(
    const SimplePlaneIntersection *pt1,
    const SimplePlaneIntersection *pt2,
    int excludePlane,
    int *result)
{
    int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if (pt1->planeIndex[i] == pt2->planeIndex[j] && pt1->planeIndex[i] != excludePlane)
            {
                *result = pt1->planeIndex[i];
                return 1;
            }
        }
    }
    return 0;
}

int __cdecl SecondPlane(const SimplePlaneIntersection *point, int plane)
{
    int planeIndex; // [esp+0h] [ebp-4h]

    for (planeIndex = 0; planeIndex < 3; ++planeIndex)
    {
        if (point->planeIndex[planeIndex] != plane)
            return point->planeIndex[planeIndex];
    }
    if (!alwaysfails)
        MyAssertHandler("..\\common\\brush_edges.cpp", 198, 0, "all planes identical");
    return -1;
}

int __cdecl ThirdPlane(const SimplePlaneIntersection *point, int plane1, int plane2)
{
    int planeIndex; // [esp+0h] [ebp-4h]

    for (planeIndex = 0; planeIndex < 3; ++planeIndex)
    {
        if (point->planeIndex[planeIndex] != plane1 && point->planeIndex[planeIndex] != plane2)
            return point->planeIndex[planeIndex];
    }
    if (!alwaysfails)
        MyAssertHandler("..\\common\\brush_edges.cpp", 212, 0, "no third plane");
    return -1;
}

const SimplePlaneIntersection *__cdecl RemoveNextPointFormedByThisPlane(
    int planeIndex,
    const SimplePlaneIntersection **begin,
    const SimplePlaneIntersection **end)
{
    const SimplePlaneIntersection *returnVal; // [esp+0h] [ebp-4h]
    const SimplePlaneIntersection **begina; // [esp+10h] [ebp+Ch]

    begina = NextPointFormedByThisPlane(planeIndex, begin, end);
    if (begina == end)
        return 0;
    returnVal = *begina;
    memmove(begina, begina + 4, 4 * (end - (begina + 1)));
    return returnVal;
}

const SimplePlaneIntersection **__cdecl NextPointFormedByThisPlane(
    int planeIndex,
    const SimplePlaneIntersection **begin,
    const SimplePlaneIntersection **end)
{
    while (begin != end && !IsPtFormedByThisPlane(planeIndex, *begin))
        ++begin;
    return begin;
}

double __cdecl CyclePerimiter(const SimplePlaneIntersection **pts, int ptsCount)
{
    double v3; // [esp+0h] [ebp-40h]
    float v[3]; // [esp+14h] [ebp-2Ch] BYREF
    float *a; // [esp+20h] [ebp-20h]
    float *b; // [esp+24h] [ebp-1Ch]
    float diff[3]; // [esp+2Ch] [ebp-14h] BYREF
    float perimiter; // [esp+38h] [ebp-8h]
    int ptsIndex; // [esp+3Ch] [ebp-4h]

    if (ptsCount <= 2)
        MyAssertHandler("..\\common\\brush_edges.cpp", 403, 0, "%s", "ptsCount > 2");
    a = (float*)pts[ptsCount - 1]->xyz;
    b = (float*)*pts; // KISAKTODO: bad cast??
    Vec3Sub(a, b, diff);
    perimiter = Vec3Length(diff);
    for (ptsIndex = 1; ptsIndex < ptsCount; ++ptsIndex)
    {
        Vec3Sub(pts[ptsIndex - 1]->xyz, pts[ptsIndex]->xyz, v);
        v3 = perimiter;
        perimiter = Vec3Length(v) + v3;
    }
    return perimiter;
}

char __cdecl TestConvexWithoutNearPoints(const SimplePlaneIntersection **pts, unsigned int ptCount)
{
    float v[3]; // [esp+0h] [ebp-3028h] BYREF
    float diff[3]; // [esp+Ch] [ebp-301Ch] BYREF
    float *v5; // [esp+18h] [ebp-3010h]
    const SimplePlaneIntersection *v6; // [esp+1Ch] [ebp-300Ch]
    float a[3073]; // [esp+20h] [ebp-3008h] BYREF
    unsigned int i; // [esp+3024h] [ebp-4h]

    for (i = 0; i < ptCount; ++i)
    {
        v5 = &a[3 * i];
        v6 = pts[i];
        *v5 = v6->xyz[0];
        v5[1] = v6->xyz[1];
        v5[2] = v6->xyz[2];
    }
    i = 1;
    while (i < ptCount)
    {
        Vec3Sub(&a[3 * i - 3], &a[3 * i], diff);
        if (Vec3LengthSq(diff) >= 1.0)
        {
            ++i;
        }
        else
        {
            memmove(&a[3 * i], &a[3 * i + 3], 12 * (ptCount - i - 1));
            --ptCount;
        }
    }
    Vec3Sub(&a[3 * ptCount - 3], a, v);
    if (Vec3LengthSq(v) < 1.0)
        --ptCount;
    if (ptCount >= 3)
        return IsConvex((const float(*)[3])a, ptCount);
    else
        return 0;
}

char __cdecl IsConvex(const float (*pts)[3], unsigned int ptCount)
{
    unsigned int ptIndex3; // [esp+18h] [ebp-40h]
    unsigned int ptIndex3a; // [esp+18h] [ebp-40h]
    float edge1[3]; // [esp+1Ch] [ebp-3Ch] BYREF
    float edge2[3]; // [esp+28h] [ebp-30h] BYREF
    float normal[3]; // [esp+34h] [ebp-24h] BYREF
    unsigned int ptIndex2; // [esp+40h] [ebp-18h]
    unsigned int ptIndex1; // [esp+44h] [ebp-14h]
    float normal2[3]; // [esp+48h] [ebp-10h] BYREF
    float normalMag; // [esp+54h] [ebp-4h]

    if (ptCount <= 2)
        MyAssertHandler("..\\common\\brush_edges.cpp", 424, 0, "%s", "ptCount > 2");
    ptIndex2 = ptCount - 1;
    ptIndex1 = ptCount - 2;
    for (ptIndex3 = 0; ptIndex3 < ptCount; ++ptIndex3)
    {
        Vec3Sub(&(*pts)[3 * ptIndex2], &(*pts)[3 * ptIndex1], edge1);
        Vec3Sub(&(*pts)[3 * ptIndex3], &(*pts)[3 * ptIndex2], edge2);
        if (Vec3LengthSq(edge1) <= 0.0)
            MyAssertHandler("..\\common\\brush_edges.cpp", 434, 0, "%s", "Vec3LengthSq( edge1 ) > 0");
        if (Vec3LengthSq(edge2) <= 0.0)
            MyAssertHandler("..\\common\\brush_edges.cpp", 435, 0, "%s", "Vec3LengthSq( edge2 ) > 0");
        Vec3Cross(edge1, edge2, normal);
        normalMag = Vec3Normalize(normal);
        if (normalMag >= 0.01)
            break;
        if (Vec3Dot(edge1, edge2) < 0.0)
            return 0;
        ptIndex1 = ptIndex2;
        ptIndex2 = ptIndex3;
    }
    ptIndex1 = ptIndex2;
    ptIndex2 = ptIndex3;
    for (ptIndex3a = ptIndex3 + 1; ptIndex3a < ptCount; ++ptIndex3a)
    {
        Vec3Sub(&(*pts)[3 * ptIndex2], &(*pts)[3 * ptIndex1], edge1);
        Vec3Sub(&(*pts)[3 * ptIndex3a], &(*pts)[3 * ptIndex2], edge2);
        if (Vec3LengthSq(edge1) <= 0.0)
            MyAssertHandler("..\\common\\brush_edges.cpp", 458, 0, "%s", "Vec3LengthSq( edge1 ) > 0");
        if (Vec3LengthSq(edge2) <= 0.0)
            MyAssertHandler("..\\common\\brush_edges.cpp", 459, 0, "%s", "Vec3LengthSq( edge2 ) > 0");
        Vec3Cross(edge1, edge2, normal2);
        normalMag = Vec3Normalize(normal2);
        if (normalMag >= 0.01)
        {
            if (Vec3Dot(normal, normal2) < 0.5)
                return 0;
        }
        else if (Vec3Dot(edge1, edge2) < 0.0)
        {
            return 0;
        }
        ptIndex1 = ptIndex2;
        ptIndex2 = ptIndex3a;
    }
    return 1;
}

bool __cdecl CycleLess(
    bool isConvex1,
    bool isConvex2,
    float perimiter1,
    float perimiter2,
    int nodeCount1,
    int nodeCount2)
{
    if (isConvex1)
    {
        if (!isConvex2)
            return 0;
    }
    else if (isConvex2)
    {
        return 1;
    }
    return perimiter1 < perimiter2 - 1.0 || perimiter2 >= perimiter1 - 1.0 && nodeCount1 > nodeCount2;
}

int __cdecl ReduceToACycle(int basePlane, const SimplePlaneIntersection **pts, int ptsCount)
{
    int v4; // eax
    int v5; // eax
    int partition[1024]; // [esp+10h] [ebp-5040h] BYREF
    int v7; // [esp+1010h] [ebp-4040h]
    int listCount; // [esp+1014h] [ebp-403Ch]
    char CycleBFS; // [esp+1018h] [ebp-4038h]
    char v10; // [esp+1019h] [ebp-4037h]
    int v11; // [esp+101Ch] [ebp-4034h]
    const SimplePlaneIntersection *resultCycle[1024]; // [esp+1020h] [ebp-4030h] BYREF
    const SimplePlaneIntersection *v13[1024]; // [esp+2020h] [ebp-3030h] BYREF
    int list[1024]; // [esp+3020h] [ebp-2030h] BYREF
    int v15; // [esp+4020h] [ebp-1030h]
    char v16; // [esp+4024h] [ebp-102Ch]
    char v17; // [esp+4025h] [ebp-102Bh]
    int i; // [esp+4028h] [ebp-1028h]
    int v19; // [esp+402Ch] [ebp-1024h]
    int edgeCount; // [esp+4030h] [ebp-1020h]
    int k; // [esp+4034h] [ebp-101Ch]
    int resultCycleCount; // [esp+4038h] [ebp-1018h] BYREF
    int ptCount; // [esp+403Ch] [ebp-1014h] BYREF
    float perimiter1; // [esp+4040h] [ebp-1010h]
    float perimiter2; // [esp+4044h] [ebp-100Ch]
    const SimplePlaneIntersection *points[4]; // [esp+4048h] [ebp-1008h] BYREF
    int j; // [esp+504Ch] [ebp-4h]
    int ptsCounta; // [esp+5060h] [ebp+10h]

    ptsCounta = RemovePtsWithPlanesThatOccurLessThanTwice(pts, ptsCount);
    if (ptsCounta < 3)
        return ptsCounta;
    listCount = 0;
    for (i = 0; i < ptsCounta; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if (basePlane != pts[i]->planeIndex[j] && !IntAlreadyInList(list, listCount, pts[i]->planeIndex[j]))
            {
                if (listCount >= 0x400)
                    MyAssertHandler(
                        "..\\common\\brush_edges.cpp",
                        678,
                        0,
                        "planeCount doesn't index ARRAY_COUNT( planeList )\n\t%i not in [0, %i)",
                        listCount,
                        1024);
                list[listCount++] = pts[i]->planeIndex[j];
            }
        }
    }
    if (listCount <= 2)
        MyAssertHandler("..\\common\\brush_edges.cpp", 685, 1, "%s", "planeCount > 2");
    for (j = 0; j < listCount; ++j)
    {
        while (1)
        {
            edgeCount = GetPtsFormedByPlane(list[j], pts, ptsCounta, points, 1024);
            if (edgeCount <= 2)
                break;
            v15 = PartitionEdges(basePlane, list[j], pts, ptsCounta, points, edgeCount, partition);
            v7 = 0;
            for (k = 0; k < v15 && k < 2; ++k)
            {
                v11 = partition[k] - v7;
                if (v11 <= 0)
                    MyAssertHandler("..\\common\\brush_edges.cpp", 699, 0, "%s", "partitionSize > 0");
                if (v11 == 1)
                {
                    ptsCounta = Remove(pts, ptsCounta, points[v7]);
                    break;
                }
                if (v11 > 2)
                {
                    v19 = ChooseEdgeToRemove(basePlane, list[j], pts, ptsCounta, &points[v7]);
                    ptsCounta = Remove(pts, ptsCounta, *(&points[v19] + v7));
                    break;
                }
                v7 = partition[k];
            }
            if (k == 2)
            {
                if (partition[0] != 2)
                    MyAssertHandler("..\\common\\brush_edges.cpp", 720, 0, "%s", "partitions[0] == 2");
                if (partition[1] != 4)
                    MyAssertHandler("..\\common\\brush_edges.cpp", 721, 0, "%s", "partitions[1] == 4");
                CycleBFS = FindCycleBFS(
                    basePlane,
                    pts,
                    ptsCounta,
                    points[0],
                    points[1],
                    list[j],
                    resultCycle,
                    &resultCycleCount);
                v10 = FindCycleBFS(basePlane, pts, ptsCounta, points[2], points[3], list[j], v13, &ptCount);
                if (!CycleBFS || !v10)
                    MyAssertHandler("..\\common\\brush_edges.cpp", 725, 0, "%s", "isCycle[0] && isCycle[1]");
                perimiter1 = CyclePerimiter(resultCycle, resultCycleCount);
                perimiter2 = CyclePerimiter(v13, ptCount);
                v16 = TestConvexWithoutNearPoints(resultCycle, resultCycleCount);
                v17 = TestConvexWithoutNearPoints(v13, ptCount);
                if (CycleLess(v16, v17, perimiter1, perimiter2, resultCycleCount, ptCount))
                {
                    v4 = Remove(pts, ptsCounta, points[0]);
                    ptsCounta = Remove(pts, v4, points[1]);
                }
                else
                {
                    v5 = Remove(pts, ptsCounta, points[2]);
                    ptsCounta = Remove(pts, v5, points[3]);
                }
            }
            if (ptsCounta < 3)
                return ptsCounta;
        }
    }
    return ptsCounta;
}

char __cdecl IntAlreadyInList(const int *list, int listCount, int value)
{
    int listIndex; // [esp+0h] [ebp-4h]

    for (listIndex = 0; listIndex < listCount; ++listIndex)
    {
        if (list[listIndex] == value)
            return 1;
    }
    return 0;
}

char __cdecl FindCycleBFS(
    int basePlane,
    const SimplePlaneIntersection **pts,
    int ptsCount,
    const SimplePlaneIntersection *start,
    const SimplePlaneIntersection *end,
    int connectingPlane,
    const SimplePlaneIntersection **resultCycle,
    int *resultCycleCount)
{
    const SimplePlaneIntersection **v9; // [esp+0h] [ebp-4028h]
    const SimplePlaneIntersection **enda; // [esp+4h] [ebp-4024h]
    const SimplePlaneIntersection *v11; // [esp+8h] [ebp-4020h] BYREF
    int planeIndex; // [esp+Ch] [ebp-401Ch]
    int v13; // [esp+10h] [ebp-4018h]
    unsigned int v14[4094]; // [esp+14h] [ebp-4014h]
    int v15; // [esp+400Ch] [ebp-1Ch]
    const SimplePlaneIntersection *v16; // [esp+4010h] [ebp-18h]
    const SimplePlaneIntersection **i; // [esp+4014h] [ebp-14h]
    signed int j; // [esp+4018h] [ebp-10h]
    int v19; // [esp+401Ch] [ebp-Ch]
    signed int v20; // [esp+4020h] [ebp-8h]
    signed int v21; // [esp+4024h] [ebp-4h]

    if (!IsPtFormedByThisPlane(connectingPlane, start))
        MyAssertHandler("..\\common\\brush_edges.cpp", 266, 0, "%s", "IsPtFormedByThisPlane( connectingPlane, start )");
    if (!IsPtFormedByThisPlane(connectingPlane, end))
        MyAssertHandler("..\\common\\brush_edges.cpp", 267, 0, "%s", "IsPtFormedByThisPlane( connectingPlane, end )");
    v11 = start;
    planeIndex = ThirdPlane(start, basePlane, connectingPlane);
    v13 = 1;
    v14[0] = 0;
    v20 = 0;
    v21 = 1;
    v19 = ThirdPlane(end, basePlane, connectingPlane);
LABEL_6:
    if (v21 <= v20)
    {
        *resultCycleCount = 0;
        return 0;
    }
    else
    {
        enda = &pts[ptsCount];
        for (i = NextPointFormedByThisPlane(*(&planeIndex + 4 * v20), pts, enda);
            ;
            i = NextPointFormedByThisPlane(*(&planeIndex + 4 * v20), i + 1, enda))
        {
            if (i == enda)
            {
                ++v20;
                goto LABEL_6;
            }
            v15 = ThirdPlane(*i, basePlane, *(&planeIndex + 4 * v20));
            if (v15 != connectingPlane)
            {
                for (j = 0; j < v21 && *(&planeIndex + 4 * j) != v15; ++j)
                    ;
                if (j >= v21)
                {
                    if (v21 >= 0x400)
                        MyAssertHandler(
                            "..\\common\\brush_edges.cpp",
                            299,
                            0,
                            "queueHead doesn't index ARRAY_COUNT( queue )\n\t%i not in [0, %i)",
                            v21,
                            1024);
                    *(&v11 + 4 * v21) = *i;
                    *(&planeIndex + 4 * v21) = v15;
                    v14[4 * v21 - 1] = v14[4 * v20 - 1] + 1;
                    v14[4 * v21++] = (unsigned int)&v11 + 4 * v20; // KISAKTODO: sus cast
                    if (v15 == v19)
                        break;
                }
            }
        }
        v9 = &v11 + 4 * v21 - 4;
        if ((int)v9[1] != v19) // KISAKTODO: sus cast
            MyAssertHandler("..\\common\\brush_edges.cpp", 318, 1, "%s", "node->plane == goalPlane");
        *resultCycleCount = (int)(v9[2]->xyz + 1);
        v16 = v9[2];
        while (v9)
        {
            resultCycle[(unsigned int)v16] = *v9;
            v16 = (v16 - 1);
            v9 = &v9[3];
        }
        if (v16)
            MyAssertHandler("..\\common\\brush_edges.cpp", 326, 1, "%s", "cycleIndex == 0");
        *resultCycle = end;
        return 1;
    }
}

int __cdecl RemovePtsWithPlanesThatOccurLessThanTwice(const SimplePlaneIntersection **pts, int ptsCount)
{
    int ptsIndex; // [esp+0h] [ebp-4h]

    ptsIndex = 0;
    while (ptsIndex < ptsCount)
    {
        if (NumberOfOccurancesOfPlane(pts[ptsIndex]->planeIndex[0], pts, ptsCount) >= 2
            && NumberOfOccurancesOfPlane(pts[ptsIndex]->planeIndex[1], pts, ptsCount) >= 2
            && NumberOfOccurancesOfPlane(pts[ptsIndex]->planeIndex[2], pts, ptsCount) >= 2)
        {
            ++ptsIndex;
        }
        else
        {
            memmove(&pts[ptsIndex], &pts[ptsIndex + 1], 4 * (ptsCount - ptsIndex) - 4);
            --ptsCount;
            ptsIndex = 0;
        }
    }
    return ptsIndex;
}

int __cdecl NumberOfOccurancesOfPlane(int planeIndex, const SimplePlaneIntersection **pts, int ptCount)
{
    const SimplePlaneIntersection **end; // [esp+0h] [ebp-8h]
    int occurances; // [esp+4h] [ebp-4h]
    const SimplePlaneIntersection **ptsa; // [esp+14h] [ebp+Ch]

    if (!pts)
        MyAssertHandler("..\\common\\brush_edges.cpp", 232, 0, "%s", "pts");
    if (!ptCount)
        return 0;
    end = &pts[ptCount];
    occurances = 0;
    for (ptsa = NextPointFormedByThisPlane(planeIndex, pts, end);
        ptsa != end;
        ptsa = NextPointFormedByThisPlane(planeIndex, ptsa + 1, end))
    {
        ++occurances;
    }
    return occurances;
}

int __cdecl GetPtsFormedByPlane(
    int planeIndex,
    const SimplePlaneIntersection **pts,
    int ptCount,
    const SimplePlaneIntersection **result,
    int maxResults)
{
    const SimplePlaneIntersection **end; // [esp+0h] [ebp-8h]
    int occurances; // [esp+4h] [ebp-4h]
    const SimplePlaneIntersection **ptsa; // [esp+14h] [ebp+Ch]

    if (!pts)
        MyAssertHandler("..\\common\\brush_edges.cpp", 375, 0, "%s", "pts");
    if (!result)
        MyAssertHandler("..\\common\\brush_edges.cpp", 376, 0, "%s", "result");
    if (!ptCount)
        return 0;
    end = &pts[ptCount];
    occurances = 0;
    for (ptsa = NextPointFormedByThisPlane(planeIndex, pts, end);
        ptsa != end;
        ptsa = NextPointFormedByThisPlane(planeIndex, ptsa + 1, end))
    {
        if (occurances >= maxResults)
            MyAssertHandler("..\\common\\brush_edges.cpp", 387, 0, "%s", "occurances < maxResults");
        result[occurances++] = *ptsa;
    }
    return occurances;
}

int __cdecl ChooseEdgeToRemove(
    int basePlane,
    int connectingPlane,
    const SimplePlaneIntersection **pts,
    int ptsCount,
    const SimplePlaneIntersection **edges)
{
    char CycleBFS; // [esp+10h] [ebp-302Ch]
    char v7; // [esp+11h] [ebp-302Bh]
    char v8; // [esp+12h] [ebp-302Ah]
    int v9; // [esp+14h] [ebp-3028h]
    const SimplePlaneIntersection *resultCycle[1024]; // [esp+1Ch] [ebp-3020h] BYREF
    const SimplePlaneIntersection *v11[1024]; // [esp+101Ch] [ebp-2020h] BYREF
    const SimplePlaneIntersection *v12[1025]; // [esp+201Ch] [ebp-1020h] BYREF
    char v13; // [esp+3020h] [ebp-1Ch]
    char v14; // [esp+3021h] [ebp-1Bh]
    char v15; // [esp+3022h] [ebp-1Ah]
    int resultCycleCount; // [esp+3024h] [ebp-18h] BYREF
    int ptCount; // [esp+3028h] [ebp-14h] BYREF
    int v18; // [esp+302Ch] [ebp-10h] BYREF
    float perimiter1; // [esp+3030h] [ebp-Ch]
    float perimiter2; // [esp+3034h] [ebp-8h]
    float v21; // [esp+3038h] [ebp-4h]

    CycleBFS = FindCycleBFS(basePlane, pts, ptsCount, *edges, edges[1], connectingPlane, resultCycle, &resultCycleCount);
    v7 = FindCycleBFS(basePlane, pts, ptsCount, *edges, edges[2], connectingPlane, v11, &ptCount);
    v8 = FindCycleBFS(basePlane, pts, ptsCount, edges[1], edges[2], connectingPlane, v12, &v18);
    if (!CycleBFS || !v7 || !v8)
        MyAssertHandler("..\\common\\brush_edges.cpp", 549, 0, "%s", "isCycle[0] && isCycle[1] && isCycle[2]");
    perimiter1 = CyclePerimiter(resultCycle, resultCycleCount);
    perimiter2 = CyclePerimiter(v11, ptCount);
    v21 = CyclePerimiter(v12, v18);
    v13 = TestConvexWithoutNearPoints(resultCycle, resultCycleCount);
    v14 = TestConvexWithoutNearPoints(v11, ptCount);
    v15 = TestConvexWithoutNearPoints(v12, v18);
    v9 = CycleLess(v13, v14, perimiter1, perimiter2, resultCycleCount, ptCount);
    if (CycleLess(*(&v13 + v9), v15, *(&perimiter1 + v9), v21, *(&resultCycleCount + v9), v18))
        v9 = 2;
    return 2 - v9;
}

int __cdecl PartitionEdges(
    int basePlane,
    int connectingPlane,
    const SimplePlaneIntersection **pts,
    int ptsCount,
    const SimplePlaneIntersection **edges,
    int edgeCount,
    int *partition)
{
    const SimplePlaneIntersection *v8; // [esp+0h] [ebp-1020h]
    const SimplePlaneIntersection *resultCycle; // [esp+8h] [ebp-1018h] BYREF
    int v10; // [esp+100Ch] [ebp-14h]
    int i; // [esp+1010h] [ebp-10h]
    int resultCycleCount; // [esp+1014h] [ebp-Ch] BYREF
    int j; // [esp+1018h] [ebp-8h]
    int v14; // [esp+101Ch] [ebp-4h]

    v10 = 1;
    *partition = 1;
    while (partition[v10 - 1] < edgeCount)
    {
        v14 = partition[v10 - 1];
        for (i = 0; i < v10; ++i)
        {
            if (FindCycleBFS(
                basePlane,
                pts,
                ptsCount,
                edges[partition[i] - 1],
                edges[v14],
                connectingPlane,
                &resultCycle,
                &resultCycleCount))
            {
                if (i < v10 - 1)
                {
                    v8 = edges[v14];
                    memmove(&edges[partition[i] + 1], &edges[partition[i]], 4 * (v14 - partition[i]));
                    edges[partition[i]] = v8;
                }
                for (j = i; j < v10; ++j)
                    ++partition[j];
                break;
            }
        }
        if (i == v10)
            partition[v10++] = v14 + 1;
    }
    return v10;
}

int __cdecl Remove(const SimplePlaneIntersection **pts, int ptsCount, const SimplePlaneIntersection *removePoint)
{
    int ptsIndex; // [esp+0h] [ebp-4h]
    int ptsCounta; // [esp+10h] [ebp+Ch]

    for (ptsIndex = 0; ptsIndex < ptsCount && pts[ptsIndex] != removePoint; ++ptsIndex)
        ;
    if (ptsIndex == ptsCount)
        return ptsCount;
    memmove(&pts[ptsIndex], &pts[ptsIndex + 1], 4 * (ptsCount - ptsIndex) - 4);
    ptsCounta = ptsCount - 1;
    if (ptsCounta >= 3)
        return RemovePtsWithPlanesThatOccurLessThanTwice(pts, ptsCounta);
    else
        return ptsCounta;
}

int __cdecl NumberOfUniquePoints(const SimplePlaneIntersection **pts, int ptsCount)
{
    unsigned int v3[1025]; // [esp+10h] [ebp-1010h]
    int v4; // [esp+1014h] [ebp-Ch]
    int j; // [esp+1018h] [ebp-8h]
    int i; // [esp+101Ch] [ebp-4h]

    if (!pts)
        MyAssertHandler("..\\common\\brush_edges.cpp", 763, 0, "%s", "pts");
    if (ptsCount <= 2)
        MyAssertHandler("..\\common\\brush_edges.cpp", 764, 0, "%s", "ptsCount > 2");
    v4 = 0;
    for (i = 0; i < ptsCount; ++i)
    {
        for (j = 0; j < v4 && !VecNCompareCustomEpsilon(pts[i]->xyz, (const float*)v3[j], 0.0099999998f, 3); ++j) // KISAKTODO: more sus casts
            ;
        if (j == v4)
            v3[v4++] = (unsigned int)pts[i];
    }
    return v4;
}

