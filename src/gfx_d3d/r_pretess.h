#pragma once
#include "r_rendercmds.h"
#include "r_drawsurf.h"

#define MAX_DRAWSURFS 0x8000

enum DrawSurfType : __int32;

struct GfxBspPreTessDrawSurf // sizeof=0x4
{                                       // ...
    uint16_t baseSurfIndex;     // ...
    uint16_t totalTriCount;     // ...
};

void __cdecl R_InitDrawSurfListInfo(GfxDrawSurfListInfo *info);
void __cdecl R_EmitDrawSurfList(GfxDrawSurf *drawSurfs, uint32_t drawSurfCount);
void __cdecl R_MergeAndEmitDrawSurfLists(DrawSurfType firstStage, int stageCount);
uint32_t __cdecl R_EmitDrawSurfListForKey(
    const GfxDrawSurf *drawSurfs,
    uint32_t drawSurfCount,
    uint32_t primarySortKey);


uint16_t *__cdecl R_AllocPreTessIndices(int count);

void __cdecl R_EndPreTess();
void __cdecl R_BeginPreTess();

int __cdecl R_ReadBspPreTessDrawSurfs(
    struct GfxReadCmdBuf *cmdBuf,
    const struct GfxBspPreTessDrawSurf **list,
    uint32_t *count,
    uint32_t *baseIndex);