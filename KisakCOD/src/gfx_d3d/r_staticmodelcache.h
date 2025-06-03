#pragma once
#include <xanim/xmodel.h>
#include "r_rendercmds.h"

#define SMODEL_INDEX_NONE 0xFFFF

struct SkinCachedStaticModelCmd // sizeof=0x4
{                                       // ...
    unsigned __int16 cacheIndex;        // ...
    unsigned __int16 firstPatchVert;    // ...
};

struct GfxCachedSModelSurf // sizeof=0x8
{                                       // ...
    unsigned int baseVertIndex;
    unsigned __int16 lodIndex;
    unsigned __int16 smodelIndex;
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
    unsigned __int8 reserved;
};
struct static_model_tree_t // sizeof=0x108
{                                       // ...
    static_model_tree_list_t usedlist;
    unsigned int frameCount;
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
    unsigned __int16 surfType;          // ...
    unsigned __int16 objectId;          // ...
};

void __cdecl R_InitStaticModelCache();
void __cdecl R_ShutdownStaticModelCache();

static_model_leaf_t *SMC_GetLeaf(unsigned int cacheIndex);

GfxStaticModelId __cdecl R_GetStaticModelId(unsigned int smodelIndex, int lod);

void __cdecl R_CacheStaticModelIndices(unsigned int smodelIndex, unsigned int lod, unsigned int cacheBaseVertIndex);
char __cdecl SMC_ForceFreeBlock(unsigned int smcIndex);
char __cdecl SMC_GetFreeBlockOfSize(unsigned int smcIndex, unsigned int listIndex);
unsigned __int16 __cdecl SMC_Allocate(unsigned int smcIndex, unsigned int bitCount);

const GfxBackEndData *RB_PatchStaticModelCache();

unsigned __int16 __cdecl R_CacheStaticModelSurface(
    unsigned int smcIndex,
    unsigned int smodelIndex,
    const XModelLodInfo *lodInfo);

void __cdecl SMC_FreeCachedSurface_r(
    static_model_tree_t *tree,
    static_model_leaf_t *leafs,
    int nodeIndex,
    int levelsToLeaf);

unsigned int SMC_ClearCache();
void __cdecl R_FlushStaticModelCache();


void __cdecl R_StaticModelCacheStats_f();
void __cdecl R_StaticModelCacheFlush_f();
void __cdecl R_ClearAllStaticModelCacheRefs();

void __cdecl R_UncacheStaticModel(unsigned int smodelIndex);

GfxCachedSModelSurf *__cdecl R_GetCachedSModelSurf(unsigned int cacheIndex);

void __cdecl R_SkinCachedStaticModelCmd(SkinCachedStaticModelCmd *skinCmd);