#pragma once
#include "r_rendercmds.h"
#include "r_drawsurf.h"

#define MAX_DRAWSURFS 0x8000

enum DrawSurfType : __int32;

struct GfxBspPreTessDrawSurf // sizeof=0x4
{                                       // ...
    unsigned __int16 baseSurfIndex;     // ...
    unsigned __int16 totalTriCount;     // ...
};

void __cdecl R_InitDrawSurfListInfo(GfxDrawSurfListInfo *info);
void __cdecl R_EmitDrawSurfList(GfxDrawSurf *drawSurfs, unsigned int drawSurfCount);
void __cdecl R_MergeAndEmitDrawSurfLists(DrawSurfType firstStage, int stageCount);
unsigned int __cdecl R_EmitDrawSurfListForKey(
    const GfxDrawSurf *drawSurfs,
    unsigned int drawSurfCount,
    unsigned int primarySortKey);


unsigned __int16 *__cdecl R_AllocPreTessIndices(int count);

void __cdecl R_EndPreTess();
void __cdecl R_BeginPreTess();

int __cdecl R_ReadBspPreTessDrawSurfs(
    struct GfxReadCmdBuf *cmdBuf,
    const struct GfxBspPreTessDrawSurf **list,
    unsigned int *count,
    unsigned int *baseIndex);