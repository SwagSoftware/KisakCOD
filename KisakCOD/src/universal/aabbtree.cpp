#include "aabbtree.h"
#include "assertive.h"
#include "com_math.h"
#include "q_shared.h"
#include <bgame/bg_public.h>

int aabbTreeCount;
float *sortedMaxs;
float *sortedMins;
float *sortedCoplanar;

int __cdecl compare_floats(float *e0, float *e1)
{
    float diff; // [esp+4h] [ebp-4h]

    diff = *e0 - *e1;
    if (diff >= 0.0)
        return diff > 0.0;
    else
        return -1;
}

BOOL __cdecl PickAabbSplitPlane(
    float (*mins)[3],
    float (*maxs)[3],
    int *remap,
    int count,
    int *chosenAxis,
    float *chosenDist)
{
    float v7; // [esp+4h] [ebp-A4h]
    float v8; // [esp+8h] [ebp-A0h]
    float v9; // [esp+10h] [ebp-98h]
    float v10; // [esp+20h] [ebp-88h]
    float v11; // [esp+24h] [ebp-84h]
    float v12; // [esp+2Ch] [ebp-7Ch]
    int sideSplitCount; // [esp+3Ch] [ebp-6Ch]
    float nextDist; // [esp+40h] [ebp-68h]
    int prevMinCount; // [esp+44h] [ebp-64h]
    int prevOnCount; // [esp+48h] [ebp-60h]
    float dist; // [esp+4Ch] [ebp-5Ch]
    signed int minMaxCount; // [esp+50h] [ebp-58h]
    signed int coplanarCount; // [esp+54h] [ebp-54h]
    int axisIndex; // [esp+58h] [ebp-50h]
    signed int bestHeuristic; // [esp+5Ch] [ebp-4Ch]
    int smallestAxis; // [esp+60h] [ebp-48h]
    int maxIndex; // [esp+64h] [ebp-44h]
    float globalMaxs[3]; // [esp+68h] [ebp-40h] BYREF
    int sideFrontCount; // [esp+74h] [ebp-34h]
    int i; // [esp+78h] [ebp-30h]
    int sideOnCount; // [esp+7Ch] [ebp-2Ch]
    float globalMins[6]; // [esp+80h] [ebp-28h] BYREF
    int minIndex; // [esp+98h] [ebp-10h]
    int onIndex; // [esp+9Ch] [ebp-Ch]
    int heuristic; // [esp+A0h] [ebp-8h]
    int sideBackCount; // [esp+A4h] [ebp-4h]

    ClearBounds(globalMins, globalMaxs);
    for (i = 0; i < count; ++i)
        ExpandBounds(&(*mins)[3 * remap[i]], &(*maxs)[3 * remap[i]], globalMins, globalMaxs);
    smallestAxis = globalMaxs[0] - globalMins[0] > globalMaxs[1] - globalMins[1];
    if (globalMaxs[smallestAxis] - globalMins[smallestAxis] > globalMaxs[2] - globalMins[2])
        smallestAxis = 2;
    for (i = 0; i < 3; ++i)
    {
        v12 = (globalMaxs[i] - globalMins[i] + 1.0) * 10.0 / (globalMaxs[smallestAxis] - globalMins[smallestAxis] + 1.0);
        LODWORD(globalMins[i + 3]) = (v12 + 0.4999999990686774);
    }
    bestHeuristic = 0x80000000;
    for (axisIndex = 0; axisIndex < 3; ++axisIndex)
    {
        minMaxCount = 0;
        coplanarCount = 0;
        for (i = 0; i < count; ++i)
        {
            if ((*maxs)[3 * remap[i] + axisIndex] == (*mins)[3 * remap[i] + axisIndex])
            {
                sortedCoplanar[coplanarCount++] = (*mins)[3 * remap[i] + axisIndex];
            }
            else
            {
                sortedMins[minMaxCount] = (*mins)[3 * remap[i] + axisIndex];
                sortedMaxs[minMaxCount++] = (*maxs)[3 * remap[i] + axisIndex];
            }
        }
        qsort(sortedMins, minMaxCount, 4u, (int(*)(const void*, const void*))compare_floats);
        qsort(sortedMaxs, minMaxCount, 4u, (int(*)(const void *, const void *))compare_floats);
        qsort(sortedCoplanar, coplanarCount, 4u, (int(*)(const void *, const void *))compare_floats);
        sideFrontCount = 0;
        sideBackCount = count;
        sideSplitCount = 0;
        sideOnCount = 0;
        minIndex = 0;
        maxIndex = 0;
        onIndex = 0;
        prevMinCount = 0;
        prevOnCount = 0;
        v10 = *sortedMins;
        v11 = *sortedCoplanar;
        v8 = v11 - v10;
        if (v8 < 0.0)
            v7 = v11;
        else
            v7 = v10;
        nextDist = v7;
        while (nextDist < 3.402823466385289e38)
        {
            dist = nextDist;
            nextDist = FLT_MAX;
            sideSplitCount += prevMinCount;
            sideBackCount -= prevMinCount;
            prevMinCount = 0;
            while (minIndex < minMaxCount && dist == sortedMins[minIndex])
            {
                ++prevMinCount;
                ++minIndex;
            }
            if (minIndex < minMaxCount && nextDist > sortedMins[minIndex])
                nextDist = sortedMins[minIndex];
            while (maxIndex < minMaxCount && dist == sortedMaxs[maxIndex])
            {
                ++sideFrontCount;
                --sideSplitCount;
                ++maxIndex;
            }
            if (maxIndex < minMaxCount && nextDist > sortedMaxs[maxIndex])
                nextDist = sortedMaxs[maxIndex];
            sideFrontCount += prevOnCount;
            sideOnCount -= prevOnCount;
            prevOnCount = 0;
            while (onIndex < coplanarCount && dist == sortedCoplanar[onIndex])
            {
                ++prevOnCount;
                ++onIndex;
            }
            sideOnCount += prevOnCount;
            sideBackCount -= prevOnCount;
            if (onIndex < coplanarCount && nextDist > sortedCoplanar[onIndex])
                nextDist = sortedCoplanar[onIndex];
            if (sideOnCount + sideSplitCount + sideBackCount + sideFrontCount != count)
                MyAssertHandler(
                    ".\\universal\\aabbtree.cpp",
                    161,
                    0,
                    "%s",
                    "sideFrontCount + sideBackCount + sideSplitCount + sideOnCount == count");
            if (sideFrontCount < 0)
                MyAssertHandler(".\\universal\\aabbtree.cpp", 162, 0, "%s", "sideFrontCount >= 0");
            if (sideBackCount < 0)
                MyAssertHandler(".\\universal\\aabbtree.cpp", 163, 0, "%s", "sideBackCount >= 0");
            if (sideSplitCount < 0)
                MyAssertHandler(".\\universal\\aabbtree.cpp", 164, 0, "%s", "sideSplitCount >= 0");
            if (sideOnCount < 0)
                MyAssertHandler(".\\universal\\aabbtree.cpp", 165, 0, "%s", "sideOnCount >= 0");
            if (sideFrontCount > 1 && sideBackCount > 1)
            {
                heuristic = LODWORD(globalMins[axisIndex + 3])
                    + count
                    - abs32(sideFrontCount - sideBackCount)
                    - sideOnCount
                    - 4 * sideSplitCount;
                if (!sideOnCount && !sideSplitCount && !prevMinCount)
                {
                    v9 = nextDist - dist;
                    heuristic += (v9);
                }
                if (heuristic > bestHeuristic)
                {
                    bestHeuristic = heuristic;
                    *chosenAxis = axisIndex;
                    if (sideOnCount || sideSplitCount || prevMinCount)
                        *chosenDist = dist;
                    else
                        *chosenDist = (dist + nextDist) * 0.5;
                }
            }
        }
    }
    return bestHeuristic != 0x80000000;
}

double __cdecl AddedVolume(const float *addedmins, const float *addedmaxs, const float *mins, const float *maxs)
{
    float expandedMins[3]; // [esp+4h] [ebp-20h] BYREF
    float expandedVolume; // [esp+10h] [ebp-14h]
    float baseVolume; // [esp+14h] [ebp-10h]
    float expandedMaxs[3]; // [esp+18h] [ebp-Ch] BYREF

    expandedMins[0] = *mins;
    expandedMins[1] = mins[1];
    expandedMins[2] = mins[2];
    expandedMaxs[0] = *maxs;
    expandedMaxs[1] = maxs[1];
    expandedMaxs[2] = maxs[2];
    ExpandBounds(addedmins, addedmaxs, expandedMins, expandedMaxs);
    expandedVolume = (expandedMaxs[0] - expandedMins[0])
        * (expandedMaxs[1] - expandedMins[1])
        * (expandedMaxs[2] - expandedMins[2]);
    baseVolume = (*maxs - *mins) * (maxs[1] - mins[1]) * (maxs[2] - mins[2]);
    return (expandedVolume - baseVolume);
}

int __cdecl SplitAabbTree(int count, const GenericAabbTreeOptions *options, int *remap, int *midStart, int *lastStart)
{
    double v6; // [esp+4h] [ebp-60h]
    double v7; // [esp+Ch] [ebp-58h]
    int top; // [esp+14h] [ebp-50h]
    float (*mins)[3]; // [esp+18h] [ebp-4Ch]
    int splitAxis; // [esp+1Ch] [ebp-48h] BYREF
    int bot; // [esp+20h] [ebp-44h]
    float bounds[2][2][3]; // [esp+24h] [ebp-40h] BYREF
    float (*maxs)[3]; // [esp+54h] [ebp-10h]
    float splitDist; // [esp+58h] [ebp-Ch] BYREF
    int swapCache; // [esp+5Ch] [ebp-8h]
    int mid; // [esp+60h] [ebp-4h]

    mins = options->mins;
    maxs = options->maxs;
    if (!PickAabbSplitPlane(mins, maxs, remap, count, &splitAxis, &splitDist))
        return 0;
    ClearBounds(bounds[0][0], bounds[0][1]);
    ClearBounds(bounds[1][0], bounds[1][1]);
    bot = 0;
    top = count - 1;
    while (bot <= top)
    {
        while (bot <= top && splitDist >= maxs[remap[bot]][splitAxis] && splitDist > mins[remap[bot]][splitAxis])
        {
            ExpandBounds(mins[remap[bot]], maxs[remap[bot]], bounds[0][0], bounds[0][1]);
            ++bot;
        }
        while (bot <= top && splitDist <= mins[remap[top]][splitAxis] && splitDist < maxs[remap[top]][splitAxis])
        {
            ExpandBounds(mins[remap[top]], maxs[remap[top]], bounds[1][0], bounds[1][1]);
            --top;
        }
        if (bot > top)
            break;
        if ((splitDist > mins[remap[bot]][splitAxis] || splitDist >= maxs[remap[bot]][splitAxis])
            && (splitDist < maxs[remap[top]][splitAxis] || splitDist <= mins[remap[top]][splitAxis]))
        {
            for (mid = bot; mid < top; ++mid)
            {
                if (splitDist <= mins[remap[mid]][splitAxis] && splitDist < maxs[remap[mid]][splitAxis])
                {
                    swapCache = remap[mid];
                    remap[mid] = remap[top];
                    remap[top] = swapCache;
                    break;
                }
                if (splitDist >= maxs[remap[mid]][splitAxis] && splitDist > mins[remap[mid]][splitAxis])
                {
                    swapCache = remap[mid];
                    remap[mid] = remap[bot];
                    remap[bot] = swapCache;
                    break;
                }
            }
            if (mid == top)
                break;
        }
        else
        {
            swapCache = remap[bot];
            remap[bot] = remap[top];
            remap[top] = swapCache;
        }
    }
    if (bot <= top
        && (bot < options->minItemsPerLeaf
            || top - bot + 1 < options->minItemsPerLeaf
            || count - top - 1 < options->minItemsPerLeaf))
    {
        while (bot <= top)
        {
            while (bot <= top)
            {
                v7 = AddedVolume(mins[remap[bot]], maxs[remap[bot]], bounds[0][0], bounds[0][1]);
                if (AddedVolume(mins[remap[bot]], maxs[remap[bot]], bounds[1][0], bounds[1][1]) < v7)
                    break;
                ExpandBounds(mins[remap[bot]], maxs[remap[bot]], bounds[0][0], bounds[0][1]);
                ++bot;
            }
            while (bot <= top)
            {
                v6 = AddedVolume(mins[remap[top]], maxs[remap[top]], bounds[1][0], bounds[1][1]);
                if (AddedVolume(mins[remap[top]], maxs[remap[top]], bounds[0][0], bounds[0][1]) < v6)
                    break;
                ExpandBounds(mins[remap[top]], maxs[remap[top]], bounds[1][0], bounds[1][1]);
                --top;
            }
            if (bot >= top)
            {
                if (bot == top)
                {
                    if (2 * bot >= count)
                        --top;
                    else
                        ++bot;
                }
            }
            else
            {
                swapCache = remap[bot];
                remap[bot] = remap[top];
                remap[top] = swapCache;
                ++bot;
                --top;
            }
        }
    }
    if (!bot || bot == count)
        return 0;
    *midStart = bot;
    *lastStart = top + 1;
    return 1;
}

GenericAabbTree *__cdecl AllocAabbTreeNode(const GenericAabbTreeOptions *options)
{
    if (aabbTreeCount == options->treeNodeLimit)
        Com_Error(ERR_DROP, "More than %i AABB nodes needed\n", options->treeNodeLimit);
    return &options->treeNodePool[aabbTreeCount++];
}

void __cdecl CreateAabbSubTrees(
    GenericAabbTree *tree,
    const GenericAabbTreeOptions *options,
    int *remap,
    int firstIndex,
    int count)
{
    int midStart; // [esp+0h] [ebp-Ch] BYREF
    int lastStart; // [esp+4h] [ebp-8h] BYREF
    GenericAabbTree *subtree; // [esp+8h] [ebp-4h]

    if (count > options->maxItemsPerLeaf && SplitAabbTree(count, options, &remap[firstIndex], &midStart, &lastStart))
    {
        subtree = AllocAabbTreeNode(options);
        subtree->firstItem = firstIndex + tree->firstItem;
        subtree->itemCount = midStart;
        if (midStart < lastStart)
        {
            subtree = AllocAabbTreeNode(options);
            subtree->firstItem = midStart + firstIndex + tree->firstItem;
            subtree->itemCount = lastStart - midStart;
        }
        subtree = AllocAabbTreeNode(options);
        subtree->firstItem = lastStart + firstIndex + tree->firstItem;
        subtree->itemCount = count - lastStart;
    }
    else
    {
        subtree = AllocAabbTreeNode(options);
        subtree->firstItem = firstIndex + tree->firstItem;
        subtree->itemCount = count;
    }
}

void __cdecl BuildAabbTree_r(GenericAabbTree *tree, const GenericAabbTreeOptions *options, int *remap)
{
    int midStart; // [esp+0h] [ebp-10h] BYREF
    int childIndex; // [esp+4h] [ebp-Ch]
    int lastStart; // [esp+8h] [ebp-8h] BYREF
    GenericAabbTree *subtree; // [esp+Ch] [ebp-4h]

    if (!tree->itemCount)
        MyAssertHandler(".\\universal\\aabbtree.cpp", 391, 0, "%s", "tree->itemCount");
    tree->firstChild = aabbTreeCount;
    tree->childCount = 0;
    if (tree->itemCount > options->maxItemsPerLeaf
        && SplitAabbTree(tree->itemCount, options, remap, &midStart, &lastStart))
    {
        subtree = &options->treeNodePool[aabbTreeCount];
        if (tree->firstChild != aabbTreeCount)
            MyAssertHandler(".\\universal\\aabbtree.cpp", 403, 1, "%s", "tree->firstChild == aabbTreeCount");
        CreateAabbSubTrees(tree, options, remap, 0, midStart);
        if (midStart < lastStart)
            CreateAabbSubTrees(tree, options, remap, midStart, lastStart - midStart);
        CreateAabbSubTrees(tree, options, remap, lastStart, tree->itemCount - lastStart);
        tree->childCount = aabbTreeCount - tree->firstChild;
        for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
            BuildAabbTree_r(&subtree[childIndex], options, &remap[subtree[childIndex].firstItem - tree->firstItem]);
    }
}

int __cdecl BuildAabbTree(const GenericAabbTreeOptions *options)
{
    float *v2; // [esp+4h] [ebp-454h]
    float *v3; // [esp+8h] [ebp-450h]
    float *v4; // [esp+Ch] [ebp-44Ch]
    float *v5; // [esp+10h] [ebp-448h]
    unsigned __int8 *boundCopies; // [esp+44h] [ebp-414h]
    int *remap; // [esp+48h] [ebp-410h]
    int itemIndex; // [esp+4Ch] [ebp-40Ch]
    int itemIndexa; // [esp+4Ch] [ebp-40Ch]
    int itemIndexb; // [esp+4Ch] [ebp-40Ch]
    int itemIndexc; // [esp+4Ch] [ebp-40Ch]
    int remapBuffer[64]; // [esp+50h] [ebp-408h] BYREF
    float sortedBounds[3][64]; // [esp+150h] [ebp-308h] BYREF
    unsigned __int8 *itemCopies; // [esp+454h] [ebp-4h]

    if (options->itemCount > 0x40u)
    {
        remap = (int*)operator new(4 * options->itemCount);
        sortedMins = (float*)operator new(4 * options->itemCount);
        sortedMaxs = (float*)operator new(4 * options->itemCount);
        sortedCoplanar = (float*)operator new(4 * options->itemCount);
    }
    else
    {
        remap = remapBuffer;
        sortedMins = sortedBounds[0];
        sortedMaxs = sortedBounds[1];
        sortedCoplanar = sortedBounds[2];
    }
    for (itemIndex = 0; itemIndex < options->itemCount; ++itemIndex)
        remap[itemIndex] = itemIndex;
    options->treeNodePool->firstItem = 0;
    options->treeNodePool->itemCount = options->itemCount;
    aabbTreeCount = 1;
    BuildAabbTree_r(options->treeNodePool, options, remap);
    itemCopies = (unsigned char*)operator new(options->itemSize * options->itemCount);
    memcpy(itemCopies, options->items, options->itemSize * options->itemCount);
    for (itemIndexa = 0; itemIndexa < options->itemCount; ++itemIndexa)
        memcpy(
            (char*)options->items + options->itemSize * itemIndexa,
            &itemCopies[options->itemSize * remap[itemIndexa]],
            options->itemSize);
    operator delete(itemCopies);
    if (options->maintainValidBounds)
    {
        boundCopies = (unsigned char*)operator new(4 * ((3 * (unsigned __int64)options->itemCount) >> 32 != 0 ? -1 : 3 * options->itemCount));
        memcpy(boundCopies, options->mins, 12 * options->itemCount);
        for (itemIndexb = 0; itemIndexb < options->itemCount; ++itemIndexb)
        {
            v4 = options->mins[itemIndexb];
            v5 = (float*)&boundCopies[12 * remap[itemIndexb]];
            *v4 = *v5;
            v4[1] = v5[1];
            v4[2] = v5[2];
        }
        memcpy(boundCopies, options->maxs, 12 * options->itemCount);
        for (itemIndexc = 0; itemIndexc < options->itemCount; ++itemIndexc)
        {
            v2 = options->maxs[itemIndexc];
            v3 = (float*)&boundCopies[12 * remap[itemIndexc]];
            *v2 = *v3;
            v2[1] = v3[1];
            v2[2] = v3[2];
        }
        operator delete(boundCopies);
    }
    if (remap != remapBuffer)
    {
        operator delete(remap);
        operator delete(sortedMins);
        operator delete(sortedMaxs);
        operator delete(sortedCoplanar);
    }
    return aabbTreeCount;
}