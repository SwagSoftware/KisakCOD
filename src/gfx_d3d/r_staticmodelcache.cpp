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
#include "rb_logfile.h"
#include "r_utils.h"
#include "r_xsurface.h"
#include "r_model_lighting.h"


static_model_cache_t s_cache;


void __cdecl TRACK_r_staticmodelcache()
{
    track_static_alloc_internal(&s_cache, sizeof(static_model_cache_t), "s_cache", 18);
}

void *R_AllocStaticModelCache()
{
    iassert(!gfxBuf.smodelCacheVb);
    return R_AllocDynamicVertexBuffer(&gfxBuf.smodelCacheVb, 0x800000);
}

void __cdecl R_InitStaticModelCache()
{
    R_AllocStaticModelCache();
    SMC_ClearCache();
}

static_model_leaf_t *SMC_GetLeaf(unsigned int cacheIndex)
{
    iassert(cacheIndex);
    static_model_leaf_t *retval = &s_cache.leafs[0][cacheIndex - 1];
    iassert((char *)retval == ((char *)&s_cache.leafs + (sizeof(static_model_leaf_t) * (cacheIndex - 1))));
    return retval;
}

void __cdecl R_ShutdownStaticModelCache()
{
    IDirect3DVertexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    R_FlushStaticModelCache();
    if (gfxBuf.smodelCacheVb)
    {
        do
        {
            if (r_logFile)
            {
                if (r_logFile->current.integer)
                    RB_LogPrint("gfxBuf.smodelCacheVb->Release()\n");
            }
            varCopy = gfxBuf.smodelCacheVb;
            gfxBuf.smodelCacheVb = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(
                (IDirect3DSurface9 *)varCopy,
                "gfxBuf.smodelCacheVb",
                ".\\r_staticmodelcache.cpp",
                753);
        } while (alwaysfails);
    }
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
        iassert( baseIndex < SMC_MAX_INDEX_IN_CACHE );
        if (baseIndex + 3 * xsurf->triCount > 0x100000)
            MyAssertHandler(
                ".\\r_staticmodelcache.cpp",
                478,
                0,
                "%s",
                "baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE");
        twoDstIndices = (unsigned int *)&gfxBuf.smodelCache.indices[baseIndex];
        iterationCount = xsurf->triCount / 2;
        iassert( iterationCount * 2 == xsurf->triCount );
        iassert( iterationCount );
        {
            PROF_SCOPED("R_memcpy");
            do
            {
                *twoDstIndices++ = *twoSrcIndices + twoBaseOffsets;
                twoSrcIndicesa = twoSrcIndices + 1;
                *twoDstIndices++ = *twoSrcIndicesa + twoBaseOffsets;
                *twoDstIndices++ = *++twoSrcIndicesa + twoBaseOffsets;
                twoSrcIndices = twoSrcIndicesa + 1;
                --iterationCount;
            } while (iterationCount);
        }
    }
}

char __cdecl SMC_ForceFreeBlock(unsigned int smcIndex)
{
    static_model_leaf_t *leafs; // [esp+8h] [ebp-8h]
    static_model_tree_t *treenode; // [esp+Ch] [ebp-4h]

    treenode = (static_model_tree_t *)s_cache.usedlist[smcIndex].prev;
    iassert( (uintptr_t)treenode != (uintptr_t)&s_cache.usedlist[smcIndex] );
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
    leafs = s_cache.leafs[((char *)treenode - (char *)&s_cache) / sizeof(static_model_tree_t)];
    SMC_FreeCachedSurface_r(treenode, leafs, 0, 5);
    treenode->usedlist.next->prev = treenode->usedlist.prev;
    treenode->usedlist.prev->next = treenode->usedlist.next;
    leafs->freenode.prev = s_cache.freelist[smcIndex];
    leafs->freenode.next = s_cache.freelist[smcIndex][0].next;
    leafs->freenode.prev->next = (static_model_node_list_t *)leafs;
    leafs->freenode.next->prev = (static_model_node_list_t *)leafs;
    return 1;
}

char __cdecl SMC_GetFreeBlockOfSize(unsigned int smcIndex, unsigned int listIndex)
{
    static_model_node_list_t *block; // [esp+Ch] [ebp-1Ch]
    static_model_node_list_t *blocka; // [esp+Ch] [ebp-1Ch]
    static_model_tree_t *tree; // [esp+10h] [ebp-18h]
    static_model_leaf_t *leafs; // [esp+14h] [ebp-14h]
    static_model_leaf_t *freelist; // [esp+18h] [ebp-10h]
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
    freelist = &s_cache.leafs[511][6 * smcIndex + 31 + listIndex];
    if (freelist->freenode.next == (static_model_node_list_t*)freelist && !SMC_GetFreeBlockOfSize(smcIndex, listIndex - 1))
        return 0;
    block = freelist->freenode.next;
    iassert( (uintptr_t)block != (uintptr_t)freelist );
    block->next->prev = block->prev;
    block->prev->next = block->next;
    treeIndex = ((char*)block - (char*)s_cache.leafs) / sizeof(s_cache.leafs[0]);
    bcassert(treeIndex, ARRAY_COUNT(s_cache.trees));
    tree = &s_cache.trees[treeIndex];
    if (listIndex == 1)
    {
        tree->usedlist.prev = &s_cache.usedlist[smcIndex];
        tree->usedlist.next = s_cache.usedlist[smcIndex].next;
        tree->usedlist.prev->next = &tree->usedlist;
        tree->usedlist.next->prev = &tree->usedlist;
    }
    leafs = s_cache.leafs[treeIndex];
    index = ((char*)block - (char*)leafs) / 8;
    bcassert(index, ARRAY_COUNT(s_cache.leafs[treeIndex]));
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

    iassert(bitCount >= 4 && bitCount <= 9);

    listIndex = 9 - bitCount;
    freelist = &s_cache.freelist[smcIndex][9 - bitCount];
    if (freelist->next == freelist && !SMC_GetFreeBlockOfSize(smcIndex, listIndex))
        return 0;
    block = freelist->next;
    iassert(block->next->prev == block);
    iassert(block->prev->next == block);
    block->next->prev = block->prev;
    block->prev->next = block->next;
    static_assert(sizeof(s_cache.leafs[0]) == 256);
    treeIndex = ((char *)block - (char *)s_cache.leafs) / sizeof(s_cache.leafs[0]);
    bcassert(treeIndex, ARRAY_COUNT(s_cache.trees));
    tree = &s_cache.trees[treeIndex];
    if (!listIndex)
    {
        tree->usedlist.prev = &s_cache.usedlist[smcIndex];
        tree->usedlist.next = s_cache.usedlist[smcIndex].next;
        tree->usedlist.prev->next = &tree->usedlist;
        tree->usedlist.next->prev = &tree->usedlist;
    }
    leafs = s_cache.leafs[treeIndex];
    index = ((char *)block - (char *)leafs) / 8;
    bcassert(index, ARRAY_COUNT(s_cache.leafs[treeIndex]));
    iassert(block == &leafs[index].freenode);
    nodeIndex = ((index + 32) >> (5 - listIndex)) - 1;
    bcassert(nodeIndex, ARRAY_COUNT(tree->nodes));
    tree->nodes[nodeIndex].inuse = 1;

    while (1)
    {
        tree->nodes[nodeIndex].usedVerts += 1 << bitCount;
        if (!nodeIndex)
            break;
        nodeIndex = (nodeIndex - 1) >> 1;
    }

    unsigned short cacheIndex = (32 * treeIndex + index + 1);
    iassert(cacheIndex);
    iassert(&leafs[index] == SMC_GetLeaf(cacheIndex));

    leafs[index].cachedSurf.baseVertIndex = 16 * index + (treeIndex << 9);
    return cacheIndex;
}

unsigned __int16 __cdecl R_CacheStaticModelSurface(
    unsigned int smcIndex,
    unsigned int smodelIndex,
    const XModelLodInfo *lodInfo)
{
    unsigned int smcPatchVertsUsed; // [esp+4h] [ebp-2Ch]
    static_model_tree_t *tree; // [esp+18h] [ebp-18h]
    static_model_tree_t *treea; // [esp+18h] [ebp-18h]
    GfxCachedSModelSurf *cachedSurf; // [esp+1Ch] [ebp-14h]
    GfxCachedSModelSurf *cachedSurfa; // [esp+1Ch] [ebp-14h]
    unsigned __int16 cacheIndex; // [esp+24h] [ebp-Ch]
    unsigned __int16 cacheIndexa; // [esp+24h] [ebp-Ch]
    SkinCachedStaticModelCmd skinSmodelCmd; // [esp+28h] [ebp-8h] BYREF
    unsigned int cachedVertsNeeded; // [esp+2Ch] [ebp-4h]

    iassert(lodInfo);
    iassert(lodInfo->smcAllocBits >= 4 && lodInfo->smcAllocBits <= 9);
    iassert(r_smc_enable->current.enabled);
    iassert(Sys_IsMainThread());

    if (dx.deviceLost)
        return 0;

    cacheIndex = rgp.world->dpvs.smodelDrawInsts[smodelIndex].smodelCacheIndex[lodInfo->lod];
    if (cacheIndex)
    {
        cachedSurf = &SMC_GetLeaf(cacheIndex)->cachedSurf;
        tree = &s_cache.trees[((char*)cachedSurf - (char*)s_cache.leafs) / 256];
        if (tree->frameCount != rg.frontEndFrameCount)
        {
            tree->frameCount = rg.frontEndFrameCount;
            tree->usedlist.next->prev = tree->usedlist.prev;
            tree->usedlist.prev->next = tree->usedlist.next;
            tree->usedlist.prev = &s_cache.usedlist[smcIndex];
            tree->usedlist.next = s_cache.usedlist[smcIndex].next;
            tree->usedlist.prev->next = &tree->usedlist;
            tree->usedlist.next->prev = &tree->usedlist;
        }
        iassert(cachedSurf->smodelIndex != SMODEL_INDEX_NONE);
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
                cachedSurfa = &SMC_GetLeaf(cacheIndexa)->cachedSurf;
                rgp.world->dpvs.smodelDrawInsts[smodelIndex].smodelCacheIndex[lodInfo->lod] = cacheIndexa;
                if (smodelIndex != smodelIndex)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                        281,
                        0,
                        "i == static_cast< Type >( i )\n\t%i, %i",
                        smodelIndex,
                        smodelIndex);
                cachedSurfa->smodelIndex = smodelIndex;
                cachedSurfa->lodIndex = lodInfo->lod;
                skinSmodelCmd.cacheIndex = cacheIndexa;
                smcPatchVertsUsed = frontEndDataOut->smcPatchVertsUsed;

                iassert(smcPatchVertsUsed == (unsigned short)smcPatchVertsUsed);

                skinSmodelCmd.firstPatchVert = smcPatchVertsUsed;
                frontEndDataOut->smcPatchVertsUsed += cachedVertsNeeded;
                R_AddWorkerCmd(15, (unsigned char*)&skinSmodelCmd);
                R_CacheStaticModelIndices(cachedSurfa->smodelIndex, cachedSurfa->lodIndex, cachedSurfa->baseVertIndex);
                treea = &s_cache.trees[((char*)cachedSurfa - (char*)s_cache.leafs) / 256];
                treea->frameCount = rg.frontEndFrameCount;
                
                iassert(treea->usedlist.prev->prev->next == treea->usedlist.prev);
                iassert(treea->usedlist.prev->next->prev == treea->usedlist.prev);
                iassert(treea->usedlist.next->prev->next == treea->usedlist.next);
                iassert(treea->usedlist.next->next->prev == treea->usedlist.next);

                treea->usedlist.next->prev = treea->usedlist.prev;
                treea->usedlist.prev->next = treea->usedlist.next;
                treea->usedlist.prev = &s_cache.usedlist[smcIndex];
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
    GfxCachedSModelSurf *cachedSurf; // [esp+0h] [ebp-Ch]
    static_model_leaf_t *freenode; // [esp+8h] [ebp-4h]

    if (tree->nodes[nodeIndex].usedVerts)
    {
        tree->nodes[nodeIndex].usedVerts = 0;
        if (tree->nodes[nodeIndex].inuse)
        {
            cachedSurf = &leafs[((nodeIndex + 1) << levelsToLeaf) - 32].cachedSurf;
            if (cachedSurf->smodelIndex != 0xFFFF)
            {
                rgp.world->dpvs.smodelDrawInsts[cachedSurf->smodelIndex].smodelCacheIndex[cachedSurf->lodIndex] = 0;
                cachedSurf->smodelIndex = -1;
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
        //freenode->freenode.next->prev = (static_model_node_list_t *)freenode->cachedSurf.baseVertIndex;
        freenode->freenode.next->prev = freenode->freenode.prev;
        freenode->freenode.prev->next = freenode->freenode.next;
        //*(_DWORD *)(freenode->cachedSurf.baseVertIndex + 4) = (unsigned int)freenode->freenode.next;
    }
}

void SMC_ClearCache()
{
    static_model_leaf_t *v0; // [esp+4h] [ebp-14h]
    unsigned int treeIter; // [esp+8h] [ebp-10h]
    unsigned int treeItera; // [esp+8h] [ebp-10h]
    unsigned int leafIter; // [esp+Ch] [ebp-Ch]
    unsigned int listIter; // [esp+10h] [ebp-8h]
    unsigned int smcIter; // [esp+14h] [ebp-4h]

    for (treeIter = 0; treeIter < 0x200; ++treeIter)
    {
        for (leafIter = 0; leafIter < 0x20; ++leafIter)
            s_cache.leafs[treeIter][leafIter].cachedSurf.smodelIndex = -1;
    }
    for (smcIter = 0; smcIter < 4; ++smcIter)
    {
        s_cache.usedlist[smcIter].prev = &s_cache.usedlist[smcIter];
        s_cache.usedlist[smcIter].next = &s_cache.usedlist[smcIter];
        for (listIter = 0; listIter < 6; ++listIter)
        {
            s_cache.freelist[smcIter][listIter].prev = &s_cache.freelist[smcIter][listIter];
            s_cache.freelist[smcIter][listIter].next = &s_cache.freelist[smcIter][listIter];
        }
        for (treeItera = 0; treeItera < 0x80; ++treeItera)
        {
            v0 = s_cache.leafs[treeItera + (smcIter << 7)];
            v0->freenode.prev = s_cache.freelist[smcIter];
            v0->freenode.next = s_cache.freelist[smcIter][0].next;
            v0->freenode.prev->next = (static_model_node_list_t *)v0;
            v0->freenode.next->prev = (static_model_node_list_t *)v0;
        }
    }
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
            while (s_cache.usedlist[smcIter].next != &s_cache.usedlist[smcIter]) 
            {
                iassert(s_cache.usedlist[smcIter].next);
                tree = (static_model_tree_t *)s_cache.usedlist[smcIter].next;
                leafs = s_cache.leafs[tree - s_cache.trees];
                SMC_FreeCachedSurface_r(tree, leafs, 0, 5);
                next = s_cache.usedlist[smcIter].next;
                iassert(next->next->prev == next);
                iassert(next->prev->next == next);
                next->next->prev = next->prev;
                next->prev->next = next->next;
                leafs->freenode.prev = s_cache.freelist[smcIter];
                leafs->freenode.next = s_cache.freelist[smcIter][0].next;
                leafs->freenode.prev->next = &leafs->freenode;
                leafs->freenode.next->prev = &leafs->freenode;
            }
        }
        SMC_ClearCache();
    }
}

GfxCachedSModelSurf *__cdecl R_GetCachedSModelSurf(unsigned int cacheIndex)
{
    static_model_leaf_t *leaf;

    iassert(cacheIndex);
    leaf = SMC_GetLeaf(cacheIndex);
    iassert(leaf->cachedSurf.smodelIndex < rgp.world->dpvs.smodelCount);
    return &leaf->cachedSurf;
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
            {
                PROF_SCOPED("LockSModelBuffer");
                bufferData = (char *)R_LockVertexBuffer(handle, offset, 32 * vertCount, 4096);
            }
            {
                PROF_SCOPED("R_memcpy");
                Com_Memcpy(bufferData, (char *)&backEndData->smcPatchVerts[firstPatchVert], 32 * vertCount);
            }
            {
                PROF_SCOPED("LockSModelBuffer");
                R_UnlockVertexBuffer(handle);
            }
            firstPatchVert += vertCount;
        }
    }
    return result;
}

void __cdecl R_StaticModelCacheStats_f()
{
    unsigned int usedCount; // [esp+24h] [ebp-24h]
    unsigned int lodIter; // [esp+28h] [ebp-20h]
    unsigned int allocCount; // [esp+2Ch] [ebp-1Ch]
    unsigned int surfCount; // [esp+30h] [ebp-18h]
    unsigned int smodelIter; // [esp+34h] [ebp-14h]
    unsigned int lodCount; // [esp+38h] [ebp-10h]
    const GfxStaticModelDrawInst *drawInst; // [esp+3Ch] [ebp-Ch]
    unsigned int surfIter; // [esp+40h] [ebp-8h]
    XSurface *surfs; // [esp+44h] [ebp-4h] BYREF

    if (rgp.world)
    {
        allocCount = 0;
        usedCount = 0;
        for (smodelIter = 0; smodelIter < rgp.world->dpvs.smodelCount; ++smodelIter)
        {
            drawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIter];
            lodCount = XModelGetNumLods(drawInst->model);
            for (lodIter = 0; lodIter < lodCount; ++lodIter)
            {
                if (drawInst->smodelCacheIndex[lodIter])
                {
                    allocCount += 1 << XModelGetLodInfo(drawInst->model, lodIter)->smcAllocBits;
                    surfCount = XModelGetSurfCount(drawInst->model, lodIter);
                    XModelGetSurfaces(drawInst->model, &surfs, lodIter);
                    for (surfIter = 0; surfIter < surfCount; ++surfIter)
                        usedCount += XSurfaceGetNumVerts(&surfs[surfIter]);
                }
            }
        }
        Com_Printf(8, "%.2f%% of cache is currently allocated.\n", (double)allocCount * 100.0 / 262144.0);
        if (allocCount)
            Com_Printf(8, "%.2f%% allocated cache vertices are used.\n", (double)usedCount * 100.0 / (double)allocCount);
    }
}

void __cdecl R_StaticModelCacheFlush_f()
{
    R_SyncRenderThread();
    R_ClearAllStaticModelCacheRefs();
}

void __cdecl R_UncacheStaticModel(unsigned int smodelIndex)
{
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-14h]
    unsigned int lod; // [esp+Ch] [ebp-8h]
    unsigned int cacheIndex; // [esp+10h] [ebp-4h]

    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
    for (lod = 0; lod < 4; ++lod)
    {
        cacheIndex = smodelDrawInst->smodelCacheIndex[lod];
        if (smodelDrawInst->smodelCacheIndex[lod])
        {
            GfxCachedSModelSurf *cachedSurf = &SMC_GetLeaf(cacheIndex)->cachedSurf;

            iassert(cachedSurf->smodelIndex == smodelIndex);
            cachedSurf->smodelIndex = -1;
            smodelDrawInst->smodelCacheIndex[lod] = 0;
        }
    }
}

void __cdecl R_ClearAllStaticModelCacheRefs()
{
    unsigned int smodelCount; // [esp+0h] [ebp-8h]
    unsigned int smodelIndex; // [esp+4h] [ebp-4h]

    if (rgp.world)
    {
        smodelCount = rgp.world->dpvs.smodelCount;
        for (smodelIndex = 0; smodelIndex < smodelCount; ++smodelIndex)
            R_UncacheStaticModel(smodelIndex);
    }
}

void __cdecl SetupTransformUnitVec(const float4 *mtx, int (*fixedMtx)[3])
{
    (*fixedMtx)[0] = (int)(mtx->v[0] * (float)32768.0);
    (*fixedMtx)[1] = (int)(mtx->v[1] * (float)32768.0);
    (*fixedMtx)[2] = (int)(mtx->v[2] * (float)32768.0);
    (*fixedMtx)[3] = (int)(mtx[1].v[0] * (float)32768.0);
    (*fixedMtx)[4] = (int)(mtx[1].v[1] * (float)32768.0);
    (*fixedMtx)[5] = (int)(mtx[1].v[2] * (float)32768.0);
    (*fixedMtx)[6] = (int)(mtx[2].v[0] * (float)32768.0);
    (*fixedMtx)[7] = (int)(mtx[2].v[1] * (float)32768.0);
    (*fixedMtx)[8] = (int)(mtx[2].v[2] * (float)32768.0);
}

PackedUnitVec __cdecl LocalTransformUnitVec(PackedUnitVec in, const int (*fixedMtx)[3])
{
    PackedUnitVec out; // [esp+0h] [ebp-10h]

    out.array[0] = ((*fixedMtx)[3] * (in.array[3] + 192) * (in.array[1] - 127)
        + (*fixedMtx)[0] * (in.array[3] + 192) * (in.array[0] - 127)
        + (*fixedMtx)[6] * (in.array[3] + 192) * (in.array[2] - 127)
        + 1069547520) >> 23;
    out.array[1] = ((*fixedMtx)[4] * (in.array[3] + 192) * (in.array[1] - 127)
        + (*fixedMtx)[1] * (in.array[3] + 192) * (in.array[0] - 127)
        + (*fixedMtx)[7] * (in.array[3] + 192) * (in.array[2] - 127)
        + 1069547520) >> 23;
    out.array[2] = ((*fixedMtx)[5] * (in.array[3] + 192) * (in.array[1] - 127)
        + (*fixedMtx)[2] * (in.array[3] + 192) * (in.array[0] - 127)
        + (*fixedMtx)[8] * (in.array[3] + 192) * (in.array[2] - 127)
        + 1069547520) >> 23;
    out.array[3] = 64;
    return out;
}

void __cdecl R_SkinXSurfaceStaticVerts(
    const float4 *useAxis,
    const int (*normAxis)[3],
    unsigned int baseVertIndex,
    unsigned int vertCount,
    const GfxPackedVertex *srcVertArray,
    unsigned int smodelIndex,
    GfxSModelCachedVertex *verts)
{
    PackedUnitVec v7; // [esp+24h] [ebp-14h]
    PackedUnitVec v8; // [esp+28h] [ebp-10h]
    const GfxPackedVertex *srcVert; // [esp+2Ch] [ebp-Ch]
    PackedLightingCoords packedBaseLighting; // [esp+30h] [ebp-8h] BYREF
    unsigned int vertIndex; // [esp+34h] [ebp-4h]

    R_GetPackedStaticModelLightingCoords(smodelIndex, &packedBaseLighting);
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        srcVert = &srcVertArray[vertIndex];
        R_TransformSkelMat(srcVert->xyz, (const DObjSkelMat *)useAxis, verts[vertIndex].xyz);
        v8.packed = LocalTransformUnitVec(srcVert->normal, normAxis).packed;
        verts[vertIndex].normal = v8;
        verts[vertIndex].color.packed = srcVert->color.packed;
        verts[vertIndex].texCoord.packed = srcVert->texCoord.packed;
        v7.packed = LocalTransformUnitVec(srcVert->tangent, normAxis).packed;
        verts[vertIndex].tangent = v7;
        packedBaseLighting.array[3] = ((SLODWORD(srcVert->binormalSign) >> 30) & 0xFE) + 2;
        verts[vertIndex].baseLighting = packedBaseLighting;
    }
}

void __cdecl R_SkinCachedStaticModelCmd(SkinCachedStaticModelCmd *skinCmd)
{
    float v1; // [esp+94h] [ebp-100h]
    float v2; // [esp+A0h] [ebp-F4h]
    float scale; // [esp+A8h] [ebp-ECh]
    int cacheIndex; // [esp+C0h] [ebp-D4h]
    float4 useAxis[4]; // [esp+C4h] [ebp-D0h] BYREF
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+108h] [ebp-8Ch]
    unsigned int surfIndex; // [esp+10Ch] [ebp-88h]
    int baseVertIndex; // [esp+110h] [ebp-84h]
    float4 normAxis[4]; // [esp+114h] [ebp-80h] BYREF
    const GfxCachedSModelSurf *cachedSurf; // [esp+158h] [ebp-3Ch]
    unsigned int surfCount; // [esp+15Ch] [ebp-38h]
    const static_model_leaf_t *leaf; // [esp+160h] [ebp-34h]
    GfxSModelCachedVertex *verts; // [esp+164h] [ebp-30h]
    const XSurface *xsurf; // [esp+168h] [ebp-2Ch]
    int fixedNormAxis[3][3]; // [esp+16Ch] [ebp-28h] BYREF
    XSurface *surfs; // [esp+190h] [ebp-4h] BYREF

    iassert(skinCmd);
    iassert(rgp.world);

    verts = &frontEndDataOut->smcPatchVerts[skinCmd->firstPatchVert];
    cacheIndex = skinCmd->cacheIndex;

    iassert(cacheIndex);

    leaf = SMC_GetLeaf(cacheIndex);

    cachedSurf = &leaf->cachedSurf;

    iassert(leaf->cachedSurf.smodelIndex < rgp.world->dpvs.smodelCount);

    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[cachedSurf->smodelIndex];
    normAxis[0].v[0] = smodelDrawInst->placement.axis[0][0];
    normAxis[0].v[1] = smodelDrawInst->placement.axis[0][1];
    normAxis[0].v[2] = smodelDrawInst->placement.axis[0][2];
    normAxis[0].v[3] = 0.0;

    normAxis[1].v[0] = smodelDrawInst->placement.axis[1][0];
    normAxis[1].v[1] = smodelDrawInst->placement.axis[1][1];
    normAxis[1].v[2] = smodelDrawInst->placement.axis[1][2];
    normAxis[1].v[3] = 0.0;

    normAxis[2].v[0] = smodelDrawInst->placement.axis[2][0];
    normAxis[2].v[1] = smodelDrawInst->placement.axis[2][1];
    normAxis[2].v[2] = smodelDrawInst->placement.axis[2][2];
    normAxis[2].v[3] = 0.0;

    scale = smodelDrawInst->placement.scale;

    useAxis[0].v[0] = scale * normAxis[0].v[0];
    useAxis[0].v[1] = scale * normAxis[0].v[1];
    useAxis[0].v[2] = scale * normAxis[0].v[2];
    useAxis[0].v[3] = scale * (float)0.0;

    v2 = smodelDrawInst->placement.scale;
    useAxis[1].v[0] = v2 * normAxis[1].v[0];
    useAxis[1].v[1] = v2 * normAxis[1].v[1];
    useAxis[1].v[2] = v2 * normAxis[1].v[2];
    useAxis[1].v[3] = v2 * (float)0.0;

    v1 = smodelDrawInst->placement.scale;
    useAxis[2].v[0] = v1 * normAxis[2].v[0];
    useAxis[2].v[1] = v1 * normAxis[2].v[1];
    useAxis[2].v[2] = v1 * normAxis[2].v[2];
    useAxis[2].v[3] = v1 * (float)0.0;

    useAxis[3].v[0] = smodelDrawInst->placement.origin[0];
    useAxis[3].v[1] = smodelDrawInst->placement.origin[1];
    useAxis[3].v[2] = smodelDrawInst->placement.origin[2];
    useAxis[3].v[3] = 0.0;

    SetupTransformUnitVec(normAxis, fixedNormAxis);
    surfCount = XModelGetSurfCount(smodelDrawInst->model, cachedSurf->lodIndex);
    XModelGetSurfaces(smodelDrawInst->model, &surfs, cachedSurf->lodIndex);
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        xsurf = &surfs[surfIndex];
        baseVertIndex = cachedSurf->baseVertIndex + xsurf->baseVertIndex;
        R_SkinXSurfaceStaticVerts(
            useAxis,
            fixedNormAxis,
            baseVertIndex,
            xsurf->vertCount,
            xsurf->verts0,
            cachedSurf->smodelIndex,
            &verts[xsurf->baseVertIndex]);
}
    }