#include "r_staticmodelcache.h"
#include <qcommon/mem_track.h>
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include "r_init.h"
#include <xanim/xmodel.h>
#include "r_dvars.h"
#include <qcommon/threads.h>
#include "r_rendercmds.h"
#include "r_workercmds.h"
#include "r_buffers.h"
#include <universal/profile.h>


static_model_cache_t s_cache;


void __cdecl TRACK_r_staticmodelcache()
{
    track_static_alloc_internal(&s_cache, 266464, "s_cache", 18);
}

void __cdecl R_CacheStaticModelIndices(unsigned int smodelIndex, unsigned int lod, unsigned int cacheBaseVertIndex)
{
    unsigned int surfIndex; // [esp+30h] [ebp-2Ch]
    unsigned int baseIndex; // [esp+38h] [ebp-24h]
    XModel *model; // [esp+3Ch] [ebp-20h]
    unsigned int surfCount; // [esp+40h] [ebp-1Ch]
    unsigned int *twoSrcIndices; // [esp+44h] [ebp-18h]
    unsigned int *twoSrcIndicesa; // [esp+44h] [ebp-18h]
    int iterationCount; // [esp+48h] [ebp-14h]
    const XSurface *xsurf; // [esp+4Ch] [ebp-10h]
    XSurface *surfs; // [esp+50h] [ebp-Ch] BYREF
    unsigned int *twoDstIndices; // [esp+54h] [ebp-8h]
    unsigned int twoBaseOffsets; // [esp+58h] [ebp-4h]

    model = rgp.world->dpvs.smodelDrawInsts[smodelIndex].model;
    XModelGetSurfaces(model, &surfs, lod);
    surfCount = XModelGetSurfCount(model, lod);
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        xsurf = &surfs[surfIndex];
        twoBaseOffsets = (unsigned __int16)(cacheBaseVertIndex + xsurf->baseVertIndex)
            | ((unsigned __int16)(cacheBaseVertIndex + xsurf->baseVertIndex) << 16);
        twoSrcIndices = (unsigned int *)xsurf->triIndices;
        baseIndex = 3 * xsurf->baseTriIndex + 4 * cacheBaseVertIndex;
        if (baseIndex >= 0x100000)
            MyAssertHandler(".\\r_staticmodelcache.cpp", 477, 0, "%s", "baseIndex < SMC_MAX_INDEX_IN_CACHE");
        if (baseIndex + 3 * xsurf->triCount > 0x100000)
            MyAssertHandler(
                ".\\r_staticmodelcache.cpp",
                478,
                0,
                "%s",
                "baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE");
        twoDstIndices = (unsigned int *)&gfxBuf.smodelCache.indices[baseIndex];
        iterationCount = xsurf->triCount / 2;
        if (2 * iterationCount != xsurf->triCount)
            MyAssertHandler(".\\r_staticmodelcache.cpp", 482, 0, "%s", "iterationCount * 2 == xsurf->triCount");
        if (!iterationCount)
            MyAssertHandler(".\\r_staticmodelcache.cpp", 483, 0, "%s", "iterationCount");
        Profile_Begin(166);
        do
        {
            *twoDstIndices++ = *twoSrcIndices + twoBaseOffsets;
            twoSrcIndicesa = twoSrcIndices + 1;
            *twoDstIndices++ = *twoSrcIndicesa + twoBaseOffsets;
            *twoDstIndices++ = *++twoSrcIndicesa + twoBaseOffsets;
            twoSrcIndices = twoSrcIndicesa + 1;
            --iterationCount;
        } while (iterationCount);
        Profile_EndInternal(0);
    }
}

char __cdecl SMC_ForceFreeBlock(unsigned int smcIndex)
{
    static_model_leaf_t *leafs; // [esp+8h] [ebp-8h]
    static_model_tree_t *treenode; // [esp+Ch] [ebp-4h]

    treenode = (static_model_tree_t *)s_cache.usedlist[smcIndex].prev;
    if (treenode == (static_model_tree_t *)(8 * smcIndex + 245603296))
        MyAssertHandler(".\\r_staticmodelcache.cpp", 201, 0, "%s", "treenode != &s_cache.usedlist[smcIndex]");
    if (rg.frontEndFrameCount - treenode->frameCount < 4)
        return 0;
    if (treenode->nodes[0].usedVerts <= 0)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            208,
            0,
            "%s\n\t(tree->nodes[0].usedVerts) = %i",
            "(tree->nodes[0].usedVerts > 0)",
            treenode->nodes[0].usedVerts);
    leafs = s_cache.leafs[((char *)treenode - (char *)&s_cache) / 264];
    SMC_FreeCachedSurface_r(treenode, leafs, 0, 5);
    treenode->usedlist.next->prev = treenode->usedlist.prev;
    treenode->usedlist.prev->next = treenode->usedlist.next;
    leafs->cachedSurf.baseVertIndex = (unsigned int)s_cache.freelist[smcIndex];
    leafs->freenode.next = s_cache.freelist[smcIndex][0].next;
    *(_DWORD *)(leafs->cachedSurf.baseVertIndex + 4) = (unsigned int)leafs;
    leafs->freenode.next->prev = (static_model_node_list_t *)leafs;
    return 1;
}

char __cdecl SMC_GetFreeBlockOfSize(unsigned int smcIndex, unsigned int listIndex)
{
    static_model_node_list_t *block; // [esp+Ch] [ebp-1Ch]
    static_model_node_list_t *blocka; // [esp+Ch] [ebp-1Ch]
    static_model_tree_t *tree; // [esp+10h] [ebp-18h]
    static_model_leaf_t *leafs; // [esp+14h] [ebp-14h]
    static_model_node_list_t *freelist; // [esp+18h] [ebp-10h]
    unsigned int index; // [esp+1Ch] [ebp-Ch]
    unsigned int treeIndex; // [esp+24h] [ebp-4h]

    if (listIndex >= 6)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            233,
            0,
            "listIndex doesn't index ARRAY_COUNT( s_cache.freelist[smcIndex] )\n\t%i not in [0, %i)",
            listIndex,
            6);
    if (s_cache.freelist[smcIndex][listIndex].next != &s_cache.freelist[smcIndex][listIndex])
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            234,
            0,
            "%s",
            "s_cache.freelist[smcIndex][listIndex].next == &s_cache.freelist[smcIndex][listIndex]");
    if (s_cache.freelist[smcIndex][listIndex].prev != &s_cache.freelist[smcIndex][listIndex])
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            235,
            0,
            "%s",
            "s_cache.freelist[smcIndex][listIndex].prev == &s_cache.freelist[smcIndex][listIndex]");
    if (!listIndex)
        return SMC_ForceFreeBlock(smcIndex);
    freelist = (static_model_node_list_t *)(48 * smcIndex + 8 * listIndex + 245603096);
    if (s_cache.leafs[511][6 * smcIndex + 31 + listIndex].freenode.next == freelist
        && !SMC_GetFreeBlockOfSize(smcIndex, listIndex - 1))
    {
        return 0;
    }
    block = s_cache.leafs[511][6 * smcIndex + 31 + listIndex].freenode.next;
    if (block == freelist)
        MyAssertHandler(".\\r_staticmodelcache.cpp", 248, 0, "%s", "block != freelist");
    block->next->prev = block->prev;
    block->prev->next = block->next;
    treeIndex = ((char *)block - (char *)s_cache.leafs) / 256;
    if (treeIndex >= 0x200)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            254,
            0,
            "treeIndex doesn't index ARRAY_COUNT( s_cache.trees )\n\t%i not in [0, %i)",
            treeIndex,
            512);
    tree = &s_cache.trees[treeIndex];
    if (listIndex == 1)
    {
        tree->usedlist.prev = (static_model_tree_list_t *)(8 * smcIndex + 245603296);
        tree->usedlist.next = s_cache.usedlist[smcIndex].next;
        tree->usedlist.prev->next = &tree->usedlist;
        tree->usedlist.next->prev = &tree->usedlist;
    }
    leafs = s_cache.leafs[treeIndex];
    index = ((char *)block - (char *)leafs) / 8;
    if (index >= 0x20)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            264,
            0,
            "index doesn't index ARRAY_COUNT( s_cache.leafs[treeIndex] )\n\t%i not in [0, %i)",
            index,
            32);
    if (block != (static_model_node_list_t *)&leafs[index])
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            265,
            0,
            "%s\n\t(index) = %i",
            "(block == &leafs[index].freenode)",
            index);
    block->prev = &s_cache.freelist[smcIndex][listIndex];
    block->next = s_cache.freelist[smcIndex][listIndex].next;
    block->prev->next = block;
    block->next->prev = block;
    blocka = (static_model_node_list_t *)&leafs[index + (1 << (5 - listIndex))];
    blocka->prev = &s_cache.freelist[smcIndex][listIndex];
    blocka->next = s_cache.freelist[smcIndex][listIndex].next;
    blocka->prev->next = blocka;
    blocka->next->prev = blocka;
    return 1;
}

unsigned __int16 __cdecl SMC_Allocate(unsigned int smcIndex, unsigned int bitCount)
{
    static_model_node_list_t *block; // [esp+8h] [ebp-2Ch]
    unsigned int listIndex; // [esp+Ch] [ebp-28h]
    static_model_tree_t *tree; // [esp+10h] [ebp-24h]
    unsigned int nodeIndex; // [esp+14h] [ebp-20h]
    static_model_leaf_t *leafs; // [esp+1Ch] [ebp-18h]
    static_model_node_list_t *freelist; // [esp+20h] [ebp-14h]
    unsigned int index; // [esp+24h] [ebp-10h]
    unsigned int treeIndex; // [esp+30h] [ebp-4h]

    if (bitCount < 4 || bitCount > 9)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            297,
            0,
            "bitCount not in [SMC_MIN_ALLOC_BITS, SMC_MAX_ALLOC_BITS]\n\t%i not in [%i, %i]",
            bitCount,
            4,
            9);
    listIndex = 9 - bitCount;
    freelist = &s_cache.freelist[smcIndex][9 - bitCount];
    if (freelist->next == freelist && !SMC_GetFreeBlockOfSize(smcIndex, listIndex))
        return 0;
    block = freelist->next;
    block->next->prev = block->prev;
    block->prev->next = block->next;
    treeIndex = ((char *)block - (char *)s_cache.leafs) / 256;
    if (treeIndex >= 0x200)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            314,
            0,
            "treeIndex doesn't index ARRAY_COUNT( s_cache.trees )\n\t%i not in [0, %i)",
            treeIndex,
            512);
    tree = &s_cache.trees[treeIndex];
    if (!listIndex)
    {
        tree->usedlist.prev = (static_model_tree_list_t *)(8 * smcIndex + 245603296);
        tree->usedlist.next = s_cache.usedlist[smcIndex].next;
        tree->usedlist.prev->next = &tree->usedlist;
        tree->usedlist.next->prev = &tree->usedlist;
    }
    leafs = s_cache.leafs[treeIndex];
    index = ((char *)block - (char *)leafs) / 8;
    if (index >= 0x20)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            321,
            0,
            "index doesn't index ARRAY_COUNT( s_cache.leafs[treeIndex] )\n\t%i not in [0, %i)",
            index,
            32);
    if (block != (static_model_node_list_t *)&leafs[index])
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            322,
            0,
            "%s\n\t(index) = %i",
            "(block == &leafs[index].freenode)",
            index);
    nodeIndex = ((index + 32) >> (5 - listIndex)) - 1;
    if (nodeIndex >= 0x3F)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            327,
            0,
            "nodeIndex doesn't index ARRAY_COUNT( tree->nodes )\n\t%i not in [0, %i)",
            nodeIndex,
            63);
    tree->nodes[nodeIndex].inuse = 1;
    while (1)
    {
        tree->nodes[nodeIndex].usedVerts += 1 << bitCount;
        if (!nodeIndex)
            break;
        nodeIndex = (nodeIndex - 1) >> 1;
    }
    if (32 * (_WORD)treeIndex + (_WORD)index == 0xFFFF)
        MyAssertHandler(".\\r_staticmodelcache.cpp", 278, 0, "%s", "cacheIndex");
    if (&leafs[index] != (static_model_leaf_t *)(8 * (unsigned __int16)(32 * treeIndex + index + 1) + 245472024))
        MyAssertHandler(".\\r_staticmodelcache.cpp", 339, 0, "%s", "&leafs[index] == SMC_GetLeaf( cacheIndex )");
    leafs[index].cachedSurf.baseVertIndex = 16 * index + (treeIndex << 9);
    return 32 * treeIndex + index + 1;
}

unsigned __int16 __cdecl R_CacheStaticModelSurface(
    unsigned int smcIndex,
    unsigned int smodelIndex,
    const XModelLodInfo *lodInfo)
{
    unsigned int smcPatchVertsUsed; // [esp+4h] [ebp-2Ch]
    static_model_tree_t *tree; // [esp+18h] [ebp-18h]
    static_model_tree_t *treea; // [esp+18h] [ebp-18h]
    unsigned __int16 cacheIndex; // [esp+24h] [ebp-Ch]
    unsigned __int16 cacheIndexa; // [esp+24h] [ebp-Ch]
    SkinCachedStaticModelCmd skinSmodelCmd; // [esp+28h] [ebp-8h] BYREF
    unsigned int cachedVertsNeeded; // [esp+2Ch] [ebp-4h]

    if (!lodInfo)
        MyAssertHandler(".\\r_staticmodelcache.cpp", 564, 0, "%s", "lodInfo");
    if (lodInfo->smcAllocBits < 4u || lodInfo->smcAllocBits > 9u)
        MyAssertHandler(
            ".\\r_staticmodelcache.cpp",
            565,
            0,
            "lodInfo->smcAllocBits not in [SMC_MIN_ALLOC_BITS, SMC_MAX_ALLOC_BITS]\n\t%i not in [%i, %i]",
            lodInfo->smcAllocBits,
            4,
            9);
    if (!r_smc_enable->current.enabled)
        MyAssertHandler(".\\r_staticmodelcache.cpp", 566, 0, "%s", "r_smc_enable->current.enabled");
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_staticmodelcache.cpp", 567, 0, "%s", "Sys_IsMainThread()");
    if (dx.deviceLost)
        return 0;
    cacheIndex = rgp.world->dpvs.smodelDrawInsts[smodelIndex].smodelCacheIndex[lodInfo->lod];
    if (cacheIndex)
    {
        tree = &s_cache.trees[(8 * cacheIndex + 245472024 - (int)s_cache.leafs) / 256];
        if (tree->frameCount != rg.frontEndFrameCount)
        {
            tree->frameCount = rg.frontEndFrameCount;
            tree->usedlist.next->prev = tree->usedlist.prev;
            tree->usedlist.prev->next = tree->usedlist.next;
            tree->usedlist.prev = (static_model_tree_list_t *)(8 * smcIndex + 245603296);
            tree->usedlist.next = s_cache.usedlist[smcIndex].next;
            tree->usedlist.prev->next = &tree->usedlist;
            tree->usedlist.next->prev = &tree->usedlist;
        }
        if (*(unsigned __int16 *)&s_cache.trees[511].nodes[2 * cacheIndex + 62].inuse == 0xFFFF)
            MyAssertHandler(".\\r_staticmodelcache.cpp", 587, 0, "%s", "cachedSurf->smodelIndex != SMODEL_INDEX_NONE");
        return cacheIndex;
    }
    else
    {
        cachedVertsNeeded = 1 << lodInfo->smcAllocBits;
        if (frontEndDataOut->smcPatchCount == 256 || cachedVertsNeeded + frontEndDataOut->smcPatchVertsUsed > 0x2000)
        {
            return 0;
        }
        else
        {
            cacheIndexa = SMC_Allocate(smcIndex, lodInfo->smcAllocBits);
            if (cacheIndexa)
            {
                frontEndDataOut->smcPatchList[frontEndDataOut->smcPatchCount++] = cacheIndexa;
                rgp.world->dpvs.smodelDrawInsts[smodelIndex].smodelCacheIndex[lodInfo->lod] = cacheIndexa;
                if (smodelIndex != (unsigned __int16)smodelIndex)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                        281,
                        0,
                        "i == static_cast< Type >( i )\n\t%i, %i",
                        smodelIndex,
                        (unsigned __int16)smodelIndex);
                *(_WORD *)&s_cache.trees[511].nodes[2 * cacheIndexa + 62].inuse = smodelIndex;
                s_cache.trees[511].nodes[2 * cacheIndexa + 62].usedVerts = lodInfo->lod;
                skinSmodelCmd.cacheIndex = cacheIndexa;
                smcPatchVertsUsed = frontEndDataOut->smcPatchVertsUsed;
                if (smcPatchVertsUsed != (unsigned __int16)smcPatchVertsUsed)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                        281,
                        0,
                        "i == static_cast< Type >( i )\n\t%i, %i",
                        smcPatchVertsUsed,
                        (unsigned __int16)smcPatchVertsUsed);
                skinSmodelCmd.firstPatchVert = smcPatchVertsUsed;
                frontEndDataOut->smcPatchVertsUsed += cachedVertsNeeded;
                R_AddWorkerCmd(15, (unsigned __int8 *)&skinSmodelCmd);
                R_CacheStaticModelIndices(
                    *(unsigned __int16 *)&s_cache.trees[511].nodes[2 * cacheIndexa + 62].inuse,
                    (unsigned __int16)s_cache.trees[511].nodes[2 * cacheIndexa + 62].usedVerts,
                    *(_DWORD *)&s_cache.trees[511].nodes[2 * cacheIndexa + 61]);
                treea = &s_cache.trees[(8 * cacheIndexa + 245472024 - (int)s_cache.leafs) / 256];
                treea->frameCount = rg.frontEndFrameCount;
                treea->usedlist.next->prev = treea->usedlist.prev;
                treea->usedlist.prev->next = treea->usedlist.next;
                treea->usedlist.prev = (static_model_tree_list_t *)(8 * smcIndex + 245603296);
                treea->usedlist.next = s_cache.usedlist[smcIndex].next;
                treea->usedlist.prev->next = &treea->usedlist;
                treea->usedlist.next->prev = &treea->usedlist;
                return cacheIndexa;
            }
            else
            {
                return 0;
            }
        }
    }
}

void __cdecl SMC_FreeCachedSurface_r(
    static_model_tree_t *tree,
    static_model_leaf_t *leafs,
    int nodeIndex,
    int levelsToLeaf)
{
    static_model_leaf_t *cachedSurf; // [esp+0h] [ebp-Ch]
    static_model_leaf_t *freenode; // [esp+8h] [ebp-4h]

    if (tree->nodes[nodeIndex].usedVerts)
    {
        tree->nodes[nodeIndex].usedVerts = 0;
        if (tree->nodes[nodeIndex].inuse)
        {
            cachedSurf = &leafs[((nodeIndex + 1) << levelsToLeaf) - 32];
            if (cachedSurf->cachedSurf.smodelIndex != 0xFFFF)
            {
                rgp.world->dpvs.smodelDrawInsts[cachedSurf->cachedSurf.smodelIndex].smodelCacheIndex[cachedSurf->cachedSurf.lodIndex] = 0;
                cachedSurf->cachedSurf.smodelIndex = -1;
            }
            tree->nodes[nodeIndex].inuse = 0;
        }
        else
        {
            SMC_FreeCachedSurface_r(tree, leafs, 2 * nodeIndex + 1, levelsToLeaf - 1);
            SMC_FreeCachedSurface_r(tree, leafs, 2 * nodeIndex + 2, levelsToLeaf - 1);
        }
    }
    else
    {
        freenode = &leafs[((nodeIndex + 1) << levelsToLeaf) - 32];
        freenode->freenode.next->prev = (static_model_node_list_t *)freenode->cachedSurf.baseVertIndex;
        *(_DWORD *)(freenode->cachedSurf.baseVertIndex + 4) = (unsigned int)freenode->freenode.next;
    }
}

unsigned int SMC_ClearCache()
{
    unsigned int result; // eax
    static_model_leaf_t *v1; // [esp+4h] [ebp-14h]
    unsigned int treeIter; // [esp+8h] [ebp-10h]
    unsigned int treeItera; // [esp+8h] [ebp-10h]
    unsigned int leafIter; // [esp+Ch] [ebp-Ch]
    unsigned int listIter; // [esp+10h] [ebp-8h]
    unsigned int smcIter; // [esp+14h] [ebp-4h]

    for (treeIter = 0; treeIter < 0x200; ++treeIter)
    {
        for (leafIter = 0; leafIter < 0x20; ++leafIter)
            s_cache.leafs[treeIter][leafIter].cachedSurf.smodelIndex = -1;
        result = treeIter + 1;
    }
    for (smcIter = 0; smcIter < 4; ++smcIter)
    {
        s_cache.usedlist[smcIter].prev = (static_model_tree_list_t *)(8 * smcIter + 245603296);
        result = smcIter;
        s_cache.usedlist[smcIter].next = (static_model_tree_list_t *)(8 * smcIter + 245603296);
        for (listIter = 0; listIter < 6; ++listIter)
        {
            s_cache.freelist[smcIter][listIter].prev = &s_cache.freelist[smcIter][listIter];
            s_cache.freelist[smcIter][listIter].next = &s_cache.freelist[smcIter][listIter];
            result = listIter + 1;
        }
        for (treeItera = 0; treeItera < 0x80; ++treeItera)
        {
            v1 = s_cache.leafs[treeItera + (smcIter << 7)];
            v1->cachedSurf.baseVertIndex = (unsigned int)s_cache.freelist[smcIter];
            v1->freenode.next = s_cache.freelist[smcIter][0].next;
            *(_DWORD *)(v1->cachedSurf.baseVertIndex + 4) = (unsigned int)v1;
            result = (unsigned int)v1;
            v1->freenode.next->prev = (static_model_node_list_t *)v1;
        }
    }
    return result;
}

void __cdecl R_FlushStaticModelCache()
{
    static_model_tree_list_t *next; // [esp+4h] [ebp-10h]
    static_model_tree_t *tree; // [esp+8h] [ebp-Ch]
    unsigned int smcIter; // [esp+Ch] [ebp-8h]
    static_model_leaf_t *leafs; // [esp+10h] [ebp-4h]

    if (s_cache.usedlist[0].next)
    {
        for (smcIter = 0; smcIter < 4; ++smcIter)
        {
            while (s_cache.usedlist[smcIter].next != (static_model_tree_list_t *)(8 * smcIter + 245603296))
            {
                if (!s_cache.usedlist[smcIter].next)
                    MyAssertHandler(".\\r_staticmodelcache.cpp", 696, 0, "%s", "s_cache.usedlist[smcIter].next");
                tree = (static_model_tree_t *)s_cache.usedlist[smcIter].next;
                leafs = s_cache.leafs[((char *)tree - (char *)&s_cache) / 264];
                SMC_FreeCachedSurface_r(tree, leafs, 0, 5);
                next = s_cache.usedlist[smcIter].next;
                next->next->prev = next->prev;
                next->prev->next = next->next;
                leafs->cachedSurf.baseVertIndex = (unsigned int)s_cache.freelist[smcIter];
                leafs->freenode.next = s_cache.freelist[smcIter][0].next;
                *(_DWORD *)(leafs->cachedSurf.baseVertIndex + 4) = (unsigned int)leafs;
                leafs->freenode.next->prev = (static_model_node_list_t *)leafs;
            }
        }
        SMC_ClearCache();
    }
}

const GfxBackEndData *RB_PatchStaticModelCache()
{
    const GfxBackEndData *result; // eax
    IDirect3DVertexBuffer9 *handle; // [esp+58h] [ebp-24h]
    const GfxCachedSModelSurf *cachedSurf; // [esp+60h] [ebp-1Ch]
    unsigned int offset; // [esp+64h] [ebp-18h]
    unsigned int firstPatchVert; // [esp+6Ch] [ebp-10h]
    unsigned int patchIter; // [esp+70h] [ebp-Ch]
    char *bufferData; // [esp+74h] [ebp-8h]
    unsigned int vertCount; // [esp+78h] [ebp-4h]

    result = backEndData;
    if (backEndData->smcPatchCount)
    {
        handle = gfxBuf.smodelCacheVb;
        firstPatchVert = 0;
        for (patchIter = 0; ; ++patchIter)
        {
            result = backEndData;
            if (patchIter >= backEndData->smcPatchCount)
                break;
            cachedSurf = R_GetCachedSModelSurf(backEndData->smcPatchList[patchIter]);
            vertCount = XModelGetStaticModelCacheVertCount(
                rgp.world->dpvs.smodelDrawInsts[cachedSurf->smodelIndex].model,
                cachedSurf->lodIndex);
            offset = 32 * cachedSurf->baseVertIndex;
            Profile_Begin(164);
            bufferData = (char *)R_LockVertexBuffer(handle, offset, 32 * vertCount, 4096);
            Profile_EndInternal(0);
            Profile_Begin(166);
            Com_Memcpy(bufferData, (char *)&backEndData->smcPatchVerts[firstPatchVert], 32 * vertCount);
            Profile_EndInternal(0);
            Profile_Begin(164);
            R_UnlockVertexBuffer(handle);
            Profile_EndInternal(0);
            firstPatchVert += vertCount;
        }
    }
    return result;
}