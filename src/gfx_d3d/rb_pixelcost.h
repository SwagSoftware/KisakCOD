#pragma once
#include "r_material.h"
#include "rb_backend.h"

enum GfxPixelCostMode : __int32
{                                       // ...
    GFX_PIXEL_COST_MODE_OFF = 0x0,
    GFX_PIXEL_COST_MODE_MEASURE_COST = 0x1,
    GFX_PIXEL_COST_MODE_MEASURE_MSEC = 0x2,
    GFX_PIXEL_COST_MODE_ADD_COST_USE_DEPTH = 0x3,
    GFX_PIXEL_COST_MODE_ADD_COST_IGNORE_DEPTH = 0x4,
    GFX_PIXEL_COST_MODE_ADD_PASSES_USE_DEPTH = 0x5,
    GFX_PIXEL_COST_MODE_ADD_PASSES_IGNORE_DEPTH = 0x6,
};


extern GfxPixelCostMode pixelCostMode;

const Material *__cdecl R_PixelCost_GetAccumulationMaterial(const Material *material);
void __cdecl R_PixelCost_BeginSurface(GfxCmdBufContext context);
void __cdecl R_PixelCost_SetConstant(GfxCmdBufSourceState *source, int cost);
int __cdecl RB_PixelCost_GetCostForRecordIndex(int recordIndex);
unsigned __int64 __cdecl R_PixelCost_PackedKeyForMaterial(__int64 material);
bool __cdecl RB_PixelCost_DoesPrimMatch(unsigned __int64 packedKey);
void __cdecl RB_PixelCost_ResetPrim(unsigned __int64 packedKey);
unsigned __int64 RB_PixelCost_BeginTiming();
void __cdecl R_HW_FinishGpu();
void __cdecl R_PixelCost_EndSurface(GfxCmdBufContext context);
int RB_PixelCost_AccumulateMsec();
void RB_PixelCost_EndTiming();
GfxRenderTargetId __cdecl RB_PixelCost_OverrideRenderTarget(GfxRenderTargetId targetId);
