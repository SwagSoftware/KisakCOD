#include "rb_pixelcost.h"
#include "rb_sky.h"
#include <universal/timing.h>

struct GfxPixelCostKey_s // sizeof=0x8
{                                       // ...
    const Material *material;
    MaterialTechniqueType techType;
};
union GfxPixelCostKey // sizeof=0x8
{                                       // ...
    GfxPixelCostKey_s mtl;
    unsigned __int64 packed;
};
struct GfxPixelCostRecord // sizeof=0x20
{                                       // ...
    unsigned __int16 costHistory[12];   // ...
    GfxPixelCostKey key;                // ...
};

struct $F77C05005AAF2867FE3D26D91A48F99E // sizeof=0x10030
{                                       // ...
    GfxPixelCostMode savedMode;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    long double msecOverhead;           // ...
    unsigned __int64 timeBegin;         // ...
    long double msecElapsed;            // ...
    int frameIndex;                     // ...
    int expectedCount;                  // ...
    int recordCount;                    // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxPixelCostRecord records[2048];   // ...
};

$F77C05005AAF2867FE3D26D91A48F99E pixelCostGlob;

GfxPixelCostMode pixelCostMode;


const Material *__cdecl R_PixelCost_GetAccumulationMaterial(const Material *material)
{
    const Material *result; // eax
    const char *v2; // eax
    int v3; // [esp+4h] [ebp-8h]

    if (pixelCostMode == GFX_PIXEL_COST_MODE_ADD_COST_IGNORE_DEPTH)
        return rgp.pixelCostAddDepthDisableMaterial;
    if (pixelCostMode == GFX_PIXEL_COST_MODE_ADD_PASSES_IGNORE_DEPTH)
        return rgp.pixelCostAddDepthDisableMaterial;
    if (material->techniqueSet->techniques[4])
        v3 = material->stateBitsEntry[4];
    else
        v3 = 0;
    switch (material->stateBitsTable[v3].loadBits[1] & 0xF)
    {
    case 0u:
    case 1u:
        result = rgp.pixelCostAddDepthAlwaysMaterial;
        break;
    case 2u:
    case 3u:
        result = rgp.pixelCostAddDepthDisableMaterial;
        break;
    case 4u:
    case 5u:
        result = rgp.pixelCostAddDepthLessMaterial;
        break;
    case 6u:
    case 7u:
        result = rgp.pixelCostAddDepthDisableMaterial;
        break;
    case 8u:
    case 9u:
        result = rgp.pixelCostAddDepthEqualMaterial;
        break;
    case 0xAu:
    case 0xBu:
        result = rgp.pixelCostAddDepthDisableMaterial;
        break;
    case 0xCu:
        result = rgp.pixelCostAddNoDepthWriteMaterial;
        break;
    case 0xDu:
        result = rgp.pixelCostAddDepthWriteMaterial;
        break;
    case 0xEu:
    case 0xFu:
        result = rgp.pixelCostAddDepthDisableMaterial;
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("unhandled case %i", material->stateBitsTable[v3].loadBits[1] & 0xF);
            MyAssertHandler(".\\rb_pixelcost.cpp", 186, 1, v2);
        }
        result = rgp.pixelCostAddNoDepthWriteMaterial;
        break;
    }
    return result;
}

void __cdecl R_PixelCost_BeginSurface(GfxCmdBufContext context)
{
    int cost; // [esp+4h] [ebp-Ch]
    unsigned __int64 packedKey; // [esp+8h] [ebp-8h]
    unsigned __int64 packedKeya; // [esp+8h] [ebp-8h]

    if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST)
    {
        packedKey = R_PixelCost_PackedKeyForMaterial(*(_QWORD *)&context.state->material);
        if (!RB_PixelCost_DoesPrimMatch(packedKey))
            RB_PixelCost_ResetPrim(packedKey);
        ++pixelCostGlob.expectedCount;
        RB_PixelCost_BeginTiming();
        RB_HW_BeginOcclusionQuery(gfxAssets.pixelCountQuery);
    }
    else if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        packedKeya = R_PixelCost_PackedKeyForMaterial(*(_QWORD *)&context.state->material);
        if (!RB_PixelCost_DoesPrimMatch(packedKeya))
            RB_PixelCost_ResetPrim(packedKeya);
        ++pixelCostGlob.expectedCount;
        RB_PixelCost_BeginTiming();
    }
    else
    {
        if (pixelCostMode <= GFX_PIXEL_COST_MODE_MEASURE_MSEC)
            MyAssertHandler(".\\rb_pixelcost.cpp", 362, 0, "%s", "RB_PixelCost_IsAccumulating()");
        cost = RB_PixelCost_GetCostForRecordIndex(pixelCostGlob.recordCount);
        R_PixelCost_SetConstant(context.source, cost);
    }
}

void __cdecl R_PixelCost_SetConstant(GfxCmdBufSourceState *source, int cost)
{
    float weights[4]; // [esp+14h] [ebp-10h] BYREF

    if (gfxAssets.pixelCountQuery)
    {
        weights[0] = ((float)(cost >> 6) + 0.009999999776482582f) * 0.003921568859368563f;
        weights[1] = ((float)((cost >> 4) & 3) + 0.009999999776482582f) * 0.003921568859368563f;
        weights[2] = ((float)((cost >> 2) & 3) + 0.009999999776482582f) * 0.003921568859368563f;
        weights[3] = ((float)(cost & 3) + 0.009999999776482582f) * 0.003921568859368563f;
    }
    else
    {
        weights[0] = 0.094156861f;
        weights[1] = 0.031411767f;
        weights[2] = 0.0f;
        weights[3] = 0.0039607841f;
    }
    R_SetCodeConstantFromVec4(source, 0x13u, weights);
}

int __cdecl RB_PixelCost_GetCostForRecordIndex(int recordIndex)
{
    __int64 v2; // rax
    int v3; // [esp+4h] [ebp-68h]
    int v4; // [esp+Ch] [ebp-60h]
    int v5; // [esp+10h] [ebp-5Ch]
    int avgCost; // [esp+18h] [ebp-54h]
    int cost; // [esp+1Ch] [ebp-50h]
    int frameIndex; // [esp+20h] [ebp-4Ch]
    int frameIndexa; // [esp+20h] [ebp-4Ch]
    int frameIndexb; // [esp+20h] [ebp-4Ch]
    int totalCost; // [esp+24h] [ebp-48h]
    int validCount; // [esp+28h] [ebp-44h]
    double costDelta; // [esp+2Ch] [ebp-40h]
    double standardDeviationSum; // [esp+34h] [ebp-38h]
    int costHistory[12]; // [esp+3Ch] [ebp-30h]

    totalCost = 0;
    validCount = 0;
    for (frameIndex = 0; frameIndex < 12; ++frameIndex)
    {
        cost = pixelCostGlob.records[recordIndex].costHistory[frameIndex];
        if (pixelCostGlob.records[recordIndex].costHistory[frameIndex])
        {
            totalCost += cost;
            costHistory[validCount++] = cost;
        }
    }
    if (validCount == 1)
        return totalCost;
    if (validCount == 2)
    {
        if (costHistory[0] < costHistory[1])
            v4 = costHistory[1];
        else
            v4 = costHistory[0];
        return totalCost - v4;
    }
    else
    {
        if (validCount <= 0)
            MyAssertHandler(".\\rb_pixelcost.cpp", 222, 0, "%s\n\t(validCount) = %i", "(validCount > 0)", validCount);
        standardDeviationSum = 0.0f;
        avgCost = totalCost / validCount;
        for (frameIndexa = 0; frameIndexa < validCount; ++frameIndexa)
        {
            costDelta = (float)(costHistory[frameIndexa] - avgCost);
            standardDeviationSum = costDelta * costDelta + standardDeviationSum;
        }
        v5 = (int)(sqrt(standardDeviationSum / (float)validCount) * 1.5f);
        if (v5 > 10)
            v3 = v5;
        else
            v3 = 10;
        for (frameIndexb = validCount - 1; frameIndexb >= 0; --frameIndexb)
        {
            v2 = costHistory[frameIndexb] - avgCost;
            if ((float)v3 < (float)(int)((HIDWORD(v2) ^ v2) - HIDWORD(v2)))
            {
                totalCost -= costHistory[frameIndexb];
                --validCount;
            }
        }
        if (validCount < 2)
            MyAssertHandler(".\\rb_pixelcost.cpp", 242, 0, "%s\n\t(validCount) = %i", "(validCount >= 2)", validCount);
        return totalCost / validCount;
    }
}

unsigned __int64 __cdecl R_PixelCost_PackedKeyForMaterial(__int64 material)
{
    if (!(unsigned int)material)
        MyAssertHandler(".\\rb_pixelcost.cpp", 257, 0, "%s", "material");
    return material;
}

bool __cdecl RB_PixelCost_DoesPrimMatch(unsigned __int64 packedKey)
{
    return __PAIR64__(
        pixelCostGlob.records[pixelCostGlob.recordCount].key.mtl.techType,
        pixelCostGlob.records[pixelCostGlob.recordCount].key.mtl.material) == packedKey;
}

void __cdecl RB_PixelCost_ResetPrim(unsigned __int64 packedKey)
{
    GfxPixelCostRecord *record; // [esp+0h] [ebp-4h]

    record = &pixelCostGlob.records[pixelCostGlob.expectedCount];
    record->key.packed = packedKey;
    *(unsigned int *)record->costHistory = 0;
    *(unsigned int *)&record->costHistory[2] = 0;
    *(unsigned int *)&record->costHistory[4] = 0;
    *(unsigned int *)&record->costHistory[6] = 0;
    *(unsigned int *)&record->costHistory[8] = 0;
    *(unsigned int *)&record->costHistory[10] = 0;
}

unsigned __int64 RB_PixelCost_BeginTiming()
{
    unsigned __int64 result; // rax

    R_HW_FinishGpu();
    result = __rdtsc();
    pixelCostGlob.timeBegin = result;
    return result;
}

void __cdecl R_HW_FinishGpu()
{
    R_AcquireGpuFenceLock();
    R_FinishGpuFence();
    R_InsertGpuFence();
    R_FinishGpuFence();
    R_ReleaseGpuFenceLock();
}

void __cdecl R_PixelCost_EndSurface(GfxCmdBufContext context)
{
    unsigned __int16 v1; // [esp+10h] [ebp-38h]
    int v2; // [esp+14h] [ebp-34h]
    int v3; // [esp+28h] [ebp-20h]
    int cost; // [esp+40h] [ebp-8h]
    unsigned int pixelCount; // [esp+44h] [ebp-4h]

    if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST)
    {
        gfxAssets.pixelCountQuery->Issue(1);
        //((void(__thiscall *)(IDirect3DQuery9 *, IDirect3DQuery9 *, int))gfxAssets.pixelCountQuery->Issue)(
        //    gfxAssets.pixelCountQuery,
        //    gfxAssets.pixelCountQuery,
        //    1);
        RB_PixelCost_EndTiming();
        pixelCount = RB_HW_ReadOcclusionQuery(gfxAssets.pixelCountQuery);
        if (pixelCount)
        {
            v3 = (int)ceil(
                (float)(context.source->renderTargetHeight * context.source->renderTargetWidth)
                * pixelCostGlob.msecElapsed
                / (float)pixelCount
                * 30.72f);
            if (v3 > 1)
                v2 = v3;
            else
                v2 = 1;
            cost = v2;
        }
        else
        {
            cost = 1;
        }
        if (cost > 0xFFFF)
            v1 = -1;
        else
            v1 = cost;
        pixelCostGlob.records[pixelCostGlob.recordCount++].costHistory[pixelCostGlob.frameIndex] = v1;
    }
    else if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        RB_PixelCost_EndTiming();
        RB_PixelCost_AccumulateMsec();
    }
    else
    {
        if (pixelCostMode <= GFX_PIXEL_COST_MODE_MEASURE_MSEC)
            MyAssertHandler(".\\rb_pixelcost.cpp", 403, 0, "%s", "RB_PixelCost_IsAccumulating()");
        ++pixelCostGlob.recordCount;
    }
}

int RB_PixelCost_AccumulateMsec()
{
    int result; // eax
    unsigned __int16 v1; // [esp+8h] [ebp-Ch]
    int v2; // [esp+Ch] [ebp-8h]
    int timeQuantized; // [esp+10h] [ebp-4h]

    timeQuantized = (int)ceil(pixelCostGlob.msecElapsed * 3932.1f);
    if (timeQuantized < 0xFFFF)
        v2 = timeQuantized;
    else
        v2 = 0xFFFF;
    if (v2 > 1)
        v1 = v2;
    else
        v1 = 1;
    result = pixelCostGlob.frameIndex;
    pixelCostGlob.records[pixelCostGlob.recordCount++].costHistory[pixelCostGlob.frameIndex] = v1;
    return result;
}

void RB_PixelCost_EndTiming()
{
    R_HW_FinishGpu();
    pixelCostGlob.msecElapsed = (float)(__rdtsc() - pixelCostGlob.timeBegin) * msecPerRawTimerTick
        - pixelCostGlob.msecOverhead;
    if (pixelCostGlob.msecElapsed < 0.0f)
        pixelCostGlob.msecElapsed = 0.0f;
}

GfxRenderTargetId __cdecl RB_PixelCost_OverrideRenderTarget(GfxRenderTargetId targetId)
{
    if (targetId < R_RENDERTARGET_SCENE)
        return R_RENDERTARGET_FRAME_BUFFER;
    if (targetId >= R_RENDERTARGET_SHADOWCOOKIE)
        return targetId;
    return R_RENDERTARGET_SCENE;
}

