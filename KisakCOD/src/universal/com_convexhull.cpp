#include "com_convexhull.h"
#include "com_math.h"
#include "assertive.h"


unsigned int __cdecl Com_ConvexHull(float (*points)[2], unsigned int pointCount, float (*hull)[2])
{
    float *v4; // [esp+0h] [ebp-218h]
    float *v5; // [esp+4h] [ebp-214h]
    unsigned int hullOrder[64]; // [esp+8h] [ebp-210h] BYREF
    float offset[2]; // [esp+108h] [ebp-110h] BYREF
    unsigned int pointOrder[64]; // [esp+110h] [ebp-108h] BYREF
    unsigned int hullPointIter; // [esp+210h] [ebp-8h]
    unsigned int hullPointCount; // [esp+214h] [ebp-4h]

    if (pointCount < 3 || pointCount > 0x40)
        MyAssertHandler(
            ".\\universal\\com_convexhull.cpp",
            296,
            0,
            "pointCount not in [3, ARRAY_COUNT( pointOrder )]\n\t%i not in [%i, %i]",
            pointCount,
            3,
            64);
    if (hull == points)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 298, 0, "%s", "hull != points");
    if (hull < (float (*)[2]) & (*points)[2 * pointCount] && points < (float (*)[2]) & (*hull)[2 * pointCount])
        MyAssertHandler(
            ".\\universal\\com_convexhull.cpp",
            299,
            0,
            "%s",
            "hull >= points + pointCount || points >= hull + pointCount");
    offset[0] = -(*points)[0];
    offset[1] = -(*points)[1];
    Com_TranslatePoints(points, pointCount, offset);
    Com_InitialHull(points, pointOrder, pointCount, hullOrder);
    hullPointCount = Com_GrowInitialHull(points, pointOrder, pointCount - 2, hullOrder);
    for (hullPointIter = 0; hullPointIter < hullPointCount; ++hullPointIter)
    {
        v4 = &(*hull)[2 * hullPointIter];
        v5 = &(*points)[2 * hullOrder[hullPointIter]];
        *v4 = *v5 - offset[0];
        v4[1] = v5[1] - offset[1];
    }
    return hullPointCount;
}

void __cdecl Com_InitialHull(
    const float (*points)[2],
    unsigned int *pointOrder,
    unsigned int pointCount,
    unsigned int *hullOrder)
{
    unsigned int maxIndex; // [esp+0h] [ebp-Ch]
    unsigned int pointIndex; // [esp+4h] [ebp-8h]
    unsigned int minIndex; // [esp+8h] [ebp-4h]

    minIndex = 0;
    maxIndex = 0;
    *pointOrder = 0;
    for (pointIndex = 1; pointIndex < pointCount; ++pointIndex)
    {
        pointOrder[pointIndex] = pointIndex;
        if ((*points)[2 * pointIndex + 1] < (double)(*points)[2 * maxIndex + 1])
        {
            if ((*points)[2 * pointIndex + 1] < (double)(*points)[2 * minIndex + 1])
                minIndex = pointIndex;
        }
        else
        {
            maxIndex = pointIndex;
        }
    }
    if (minIndex == maxIndex)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 36, 1, "%s", "minIndex != maxIndex");
    *hullOrder = minIndex;
    hullOrder[1] = maxIndex;
    if (minIndex <= maxIndex)
    {
        Com_SwapHullPoints(pointOrder, maxIndex, pointCount - 1);
        Com_SwapHullPoints(pointOrder, minIndex, pointCount - 2);
    }
    else
    {
        Com_SwapHullPoints(pointOrder, minIndex, pointCount - 1);
        Com_SwapHullPoints(pointOrder, maxIndex, pointCount - 2);
    }
}

void __cdecl Com_SwapHullPoints(unsigned int *pointOrder, unsigned int pointIndex0, unsigned int pointIndex1)
{
    unsigned int swapCache; // [esp+4h] [ebp-4h]

    swapCache = pointOrder[pointIndex0];
    pointOrder[pointIndex0] = pointOrder[pointIndex1];
    pointOrder[pointIndex1] = swapCache;
}

unsigned int __cdecl Com_GrowInitialHull(
    const float (*points)[2],
    unsigned int *pointOrder,
    unsigned int pointCount,
    unsigned int *hullOrder)
{
    float *v4; // ecx
    float *v5; // ecx
    float v7; // [esp+0h] [ebp-40h]
    float v8; // [esp+4h] [ebp-3Ch]
    float edgeEq[3]; // [esp+14h] [ebp-2Ch] BYREF
    float dist; // [esp+20h] [ebp-20h]
    int botIndex; // [esp+24h] [ebp-1Ch]
    int topIndex; // [esp+28h] [ebp-18h]
    int frontIndex; // [esp+2Ch] [ebp-14h]
    float backDist; // [esp+30h] [ebp-10h]
    int backIndex; // [esp+34h] [ebp-Ch]
    float frontDist; // [esp+38h] [ebp-8h]
    unsigned int hullPointCount; // [esp+3Ch] [ebp-4h]

    if (!pointCount)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 168, 0, "%s", "pointCount >= 1");
    edgeEq[0] = (float)(*points)[2 * hullOrder[1] + 1] - (float)(*points)[2 * *hullOrder + 1];
    edgeEq[1] = (float)(*points)[2 * *hullOrder] - (float)(*points)[2 * hullOrder[1]];
    Vec2Normalize(edgeEq);
    edgeEq[2] = (float)(*points)[2 * *hullOrder + 1] * edgeEq[1] + (float)(*points)[2 * *hullOrder] * edgeEq[0];
    botIndex = 0;
    topIndex = pointCount - 1;
    frontDist = 0.001;
    frontIndex = -1;
    backDist = -0.001;
    backIndex = -1;
    while (botIndex <= topIndex)
    {
        while (1)
        {
            v4 = (float *)&(*points)[2 * pointOrder[botIndex]];
            v8 = v4[1] * edgeEq[1] + *v4 * edgeEq[0];
            dist = v8 - edgeEq[2];
            if (dist < 0.0)
                break;
            if (dist > (double)frontDist)
            {
                frontDist = dist;
                frontIndex = botIndex;
            }
            if (++botIndex > topIndex)
                goto done_splitting;
        }
        if (dist < (double)backDist)
        {
            backDist = dist;
            backIndex = botIndex;
        }
        if (botIndex > topIndex)
            MyAssertHandler(".\\universal\\com_convexhull.cpp", 204, 1, "%s", "botIndex <= topIndex");
        while (1)
        {
            v5 = (float *)&(*points)[2 * pointOrder[topIndex]];
            v7 = v5[1] * edgeEq[1] + *v5 * edgeEq[0];
            dist = v7 - edgeEq[2];
            if (dist > 0.0)
                break;
            if (dist < (double)backDist)
            {
                backDist = dist;
                backIndex = topIndex;
            }
            if (botIndex > --topIndex)
                goto done_splitting;
        }
        if (dist > (double)frontDist)
        {
            frontDist = dist;
            frontIndex = botIndex;
        }
        if (botIndex >= topIndex)
            MyAssertHandler(".\\universal\\com_convexhull.cpp", 227, 1, "%s", "botIndex < topIndex");
        Com_SwapHullPoints(pointOrder, botIndex, topIndex);
        if (backIndex == botIndex)
            backIndex = topIndex;
        ++botIndex;
        --topIndex;
    }
done_splitting:
    if (topIndex != botIndex - 1)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 237, 1, "%s", "topIndex == botIndex - 1");
    if (frontIndex < 0 && backIndex < 0)
        return 0;
    hullPointCount = 2;
    if (frontIndex >= 0)
    {
        if (frontIndex > topIndex)
            MyAssertHandler(".\\universal\\com_convexhull.cpp", 245, 1, "%s", "frontIndex <= topIndex");
        Com_SwapHullPoints(pointOrder, frontIndex, topIndex);
        hullPointCount = Com_AddPointToHull(pointOrder[topIndex], 2u, hullOrder, hullPointCount);
        if (topIndex > 0)
        {
            hullPointCount = Com_RecursivelyGrowHull(points, pointOrder, topIndex, 2u, 0, hullOrder, hullPointCount);
            hullPointCount = Com_RecursivelyGrowHull(points, pointOrder, topIndex, 1u, 2u, hullOrder, hullPointCount);
        }
    }
    if (backIndex >= 0)
    {
        if (backIndex < botIndex)
            MyAssertHandler(".\\universal\\com_convexhull.cpp", 259, 1, "%s", "backIndex >= botIndex");
        Com_SwapHullPoints(pointOrder, backIndex, botIndex);
        hullPointCount = Com_AddPointToHull(pointOrder[botIndex], 1u, hullOrder, hullPointCount);
        if (pointCount != botIndex + 1)
        {
            hullPointCount = Com_RecursivelyGrowHull(
                points,
                &pointOrder[botIndex + 1],
                pointCount - (botIndex + 1),
                1u,
                2u,
                hullOrder,
                hullPointCount);
            return Com_RecursivelyGrowHull(
                points,
                &pointOrder[botIndex + 1],
                pointCount - (botIndex + 1),
                0,
                1u,
                hullOrder,
                hullPointCount);
        }
    }
    return hullPointCount;
}

unsigned int __cdecl Com_AddPointToHull(
    unsigned int pointIndex,
    unsigned int newIndex,
    unsigned int *hullOrder,
    unsigned int hullPointCount)
{
    if (newIndex > hullPointCount)
        MyAssertHandler(
            ".\\universal\\com_convexhull.cpp",
            59,
            0,
            "newIndex not in [0, hullPointCount]\n\t%i not in [%i, %i]",
            newIndex,
            0,
            hullPointCount);
    memmove(
        (unsigned __int8 *)&hullOrder[newIndex + 1],
        (unsigned __int8 *)&hullOrder[newIndex],
        4 * (hullPointCount - newIndex));
    hullOrder[newIndex] = pointIndex;
    return hullPointCount + 1;
}

unsigned int __cdecl Com_RecursivelyGrowHull(
    const float (*points)[2],
    unsigned int *pointOrder,
    unsigned int pointCount,
    unsigned int firstIndex,
    unsigned int secondIndex,
    unsigned int *hullOrder,
    unsigned int hullPointCount)
{
    float *v7; // edx
    float *v8; // ecx
    unsigned int v10; // eax
    float v11; // [esp+0h] [ebp-34h]
    float v12; // [esp+4h] [ebp-30h]
    float *v13; // [esp+10h] [ebp-24h]
    float edgeEq[3]; // [esp+14h] [ebp-20h] BYREF
    float dist; // [esp+20h] [ebp-14h]
    int topIndex; // [esp+24h] [ebp-10h]
    int botIndex; // [esp+28h] [ebp-Ch]
    int frontIndex; // [esp+2Ch] [ebp-8h]
    float frontDist; // [esp+30h] [ebp-4h]
    unsigned int hullPointCounta; // [esp+54h] [ebp+20h]

    if (!pointCount)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 77, 0, "%s", "pointCount > 0");
    if (secondIndex != firstIndex + 1 && (firstIndex != hullPointCount - 1 || secondIndex))
        MyAssertHandler(
            ".\\universal\\com_convexhull.cpp",
            78,
            0,
            "%s",
            "secondIndex == firstIndex + 1 || (firstIndex == hullPointCount - 1 && secondIndex == 0)");
    edgeEq[0] = (float)(*points)[2 * hullOrder[firstIndex] + 1] - (float)(*points)[2 * hullOrder[secondIndex] + 1];
    edgeEq[1] = (float)(*points)[2 * hullOrder[secondIndex]] - (float)(*points)[2 * hullOrder[firstIndex]];
    Vec2Normalize(edgeEq);
    v13 = (float *)&(*points)[2 * hullOrder[firstIndex]];
    edgeEq[2] = v13[1] * edgeEq[1] + *v13 * edgeEq[0];
    botIndex = 0;
    topIndex = pointCount - 1;
    frontDist = 0.001;
    frontIndex = -1;
    while (botIndex <= topIndex)
    {
        while (1)
        {
            v7 = (float *)&(*points)[2 * pointOrder[botIndex]];
            v12 = v7[1] * edgeEq[1] + *v7 * edgeEq[0];
            dist = v12 - edgeEq[2];
            if (dist <= 0.0)
                break;
            if (dist > (double)frontDist)
            {
                frontDist = dist;
                frontIndex = botIndex;
            }
            if (++botIndex > topIndex)
                goto done_splitting_0;
        }
        if (botIndex > topIndex)
            MyAssertHandler(".\\universal\\com_convexhull.cpp", 104, 1, "%s", "botIndex <= topIndex");
        while (1)
        {
            v8 = (float *)&(*points)[2 * pointOrder[topIndex]];
            v11 = v8[1] * edgeEq[1] + *v8 * edgeEq[0];
            dist = v11 - edgeEq[2];
            if (dist > 0.0)
                break;
            if (botIndex > --topIndex)
                goto done_splitting_0;
        }
        if (dist > (double)frontDist)
        {
            frontDist = dist;
            frontIndex = botIndex;
        }
        if (botIndex >= topIndex)
            MyAssertHandler(".\\universal\\com_convexhull.cpp", 122, 1, "%s", "botIndex < topIndex");
        Com_SwapHullPoints(pointOrder, botIndex++, topIndex--);
    }
done_splitting_0:
    if (topIndex != botIndex - 1)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 130, 1, "%s", "topIndex == botIndex - 1");
    if (frontIndex > topIndex)
        MyAssertHandler(".\\universal\\com_convexhull.cpp", 131, 1, "%s", "frontIndex <= topIndex");
    if (frontIndex < 0)
        return hullPointCount;
    Com_SwapHullPoints(pointOrder, frontIndex, topIndex);
    hullPointCounta = Com_AddPointToHull(pointOrder[topIndex], firstIndex + 1, hullOrder, hullPointCount);
    if (!topIndex)
        return hullPointCounta;
    if (secondIndex)
        secondIndex = firstIndex + 2;
    v10 = Com_RecursivelyGrowHull(points, pointOrder, topIndex, firstIndex + 1, secondIndex, hullOrder, hullPointCounta);
    return Com_RecursivelyGrowHull(points, pointOrder, topIndex, firstIndex, firstIndex + 1, hullOrder, v10);
}

void __cdecl Com_TranslatePoints(float (*points)[2], unsigned int pointCount, float *offset)
{
    unsigned int pointIdx; // [esp+0h] [ebp-4h]

    for (pointIdx = 0; pointIdx < pointCount; ++pointIdx)
    {
        (*points)[2 * pointIdx] = (*points)[2 * pointIdx] + *offset;
        (*points)[2 * pointIdx + 1] = (*points)[2 * pointIdx + 1] + offset[1];
    }
}

