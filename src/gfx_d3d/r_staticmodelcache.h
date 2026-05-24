#pragma once
#include <xanim/xmodel.h>
#include "r_rendercmds.h"

#define SMODEL_INDEX_NONE 0xFFFF
#define SMC_MAX_INDEX_IN_CACHE 0x100000

struct SkinCachedStaticModelCmd // sizeof=0x4
{                                       // ...
    uint16_t cacheIndex;        // ...
    uint16_t firstPatchVert;    // ...
};

struct GfxCachedSModelSurf // sizeof=0x8
{                                       // ...
    uint32_t baseVertIndex;
    uint16_t lodIndex;
    uint16_t smodelIndex;
};

struct static_model_tree_list_t // sizeof=0x8
{                                       // ...
    static_model_tree_list_t *prev;     // ...
    static_model_tree_list_t *next;     // ...
};
struct static_model_node_t // sizeof=0x4
{                                       // ...
    __int16 usedVerts;
    bool inuse;
    uint8_t reserved;
};
struct static_model_tree_t // sizeof=0x108
{                                       // ...
    static_model_tree_list_t usedlist;
    uint32_t frameCount;
    static_model_node_t nodes[63];
};
struct static_model_node_list_t // sizeof=0x8
{                                       // ...
    static_model_node_list_t *prev;     // ...
    static_model_node_list_t *next;     // ...
};
union static_model_leaf_t // sizeof=0x8
{                                       // ...
    GfxCachedSModelSurf cachedSurf;
    static_model_node_list_t freenode;
};
struct static_model_cache_t // sizeof=0x410E0
{                                       // ...
    static_model_tree_t trees[512];
    static_model_leaf_t leafs[512][32]; // ...
    static_model_node_list_t freelist[4][6]; // ...
    static_model_tree_list_t usedlist[4]; // ...
};

struct GfxStaticModelId // sizeof=0x4
{                                       // ...
    uint16_t surfType;          // ...
    uint16_t objectId;          // ...
};

void __cdecl R_InitStaticModelCache();
void __cdecl R_ShutdownStaticModelCache();

static_model_leaf_t *SMC_GetLeaf(uint32_t cacheIndex);

GfxStaticModelId __cdecl R_GetStaticModelId(uint32_t smodelIndex, int lod);

void __cdecl R_CacheStaticModelIndices(uint32_t smodelIndex, uint32_t lod, uint32_t cacheBaseVertIndex);
char __cdecl SMC_ForceFreeBlock(uint32_t smcIndex);
char __cdecl SMC_GetFreeBlockOfSize(uint32_t smcIndex, uint32_t listIndex);
uint16_t __cdecl SMC_Allocate(uint32_t smcIndex, uint32_t bitCount);

const GfxBackEndData *RB_PatchStaticModelCache();

uint16_t __cdecl R_CacheStaticModelSurface(
    uint32_t smcIndex,
    uint32_t smodelIndex,
    const XModelLodInfo *lodInfo);

void __cdecl SMC_FreeCachedSurface_r(
    static_model_tree_t *tree,
    static_model_leaf_t *leafs,
    int nodeIndex,
    int levelsToLeaf);

void SMC_ClearCache();
void __cdecl R_FlushStaticModelCache();


void __cdecl R_StaticModelCacheStats_f();
void __cdecl R_StaticModelCacheFlush_f();
void __cdecl R_ClearAllStaticModelCacheRefs();

void __cdecl R_UncacheStaticModel(uint32_t smodelIndex);

GfxCachedSModelSurf *__cdecl R_GetCachedSModelSurf(uint32_t cacheIndex);

void __cdecl R_SkinCachedStaticModelCmd(SkinCachedStaticModelCmd *skinCmd);