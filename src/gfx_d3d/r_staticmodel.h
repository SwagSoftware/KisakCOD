#pragma once
#include "r_gfx.h"

struct GfxStaticModelCombinedInst // sizeof=0x68
{                                       // ...
    GfxStaticModelDrawInst smodelDrawInst;
    GfxStaticModelInst smodelInst;
};

void __cdecl R_StaticModelWriteInfoHeader(int fileHandle);
void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, const float dist);
bool __cdecl R_StaticModelHasLighting(unsigned int smodelIndex);
int __cdecl R_StaticModelGetMemoryUsageInst();
int __cdecl R_StaticModelGetMemoryUsage(XModel *model, int *modelCount);
bool __cdecl R_StaticModelCompare(
    const GfxStaticModelCombinedInst &smodelInst0,
    const GfxStaticModelCombinedInst &smodelInst1);