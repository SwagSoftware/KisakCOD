#include "rb_sunshadow.h"
#include <qcommon/mem_track.h>
#include "r_dvars.h"
#include "rb_backend.h"
#include "r_draw_sunshadow.h"
#include "r_cmdbuf.h"
#include "rb_pixelcost.h"
#include "rb_state.h"
#include "rb_shade.h"
#include "r_sunshadow.h"
#include "rb_postfx.h"

GfxPointVertex g_overlayPoints[36];

void __cdecl TRACK_rb_sunshadow()
{
    track_static_alloc_internal(g_overlayPoints, 576, "g_overlayPoints", 18);
}

void __cdecl RB_SunShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo)
{
    GfxCmdBuf cmdBuf; // [esp+0h] [ebp-8h] BYREF
    int partitionIndex; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    if (pixelCostMode == GFX_PIXEL_COST_MODE_OFF)
    {
        if (!data)
            MyAssertHandler(".\\rb_sunshadow.cpp", 44, 0, "%s", "data");
        R_InitContext(data, &cmdBuf);
        CL_ResetStats_f();
        for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
            R_DrawSunShadowMap(viewInfo, partitionIndex, &cmdBuf);
    }
}

void __cdecl RB_GetShadowOverlayDepthBounds(float *nearDepth, float *farDepth)
{
    float v2; // [esp+0h] [ebp-8h]
    float v3; // [esp+4h] [ebp-4h]

    *nearDepth = sm_showOverlayDepthBounds->current.value;
    *farDepth = sm_showOverlayDepthBounds->current.vector[1];
    v3 = *farDepth - *nearDepth;
    v2 = fabs(v3);
    if (v2 < 0.009999999776482582)
    {
        if (*farDepth <= (double)*nearDepth)
        {
            *farDepth = (*nearDepth + *farDepth) * 0.5 - 0.004999999888241291;
            *nearDepth = *farDepth + 0.009999999776482582;
        }
        else
        {
            *nearDepth = (*nearDepth + *farDepth) * 0.5 - 0.004999999888241291;
            *farDepth = *nearDepth + 0.009999999776482582;
        }
    }
}

void __cdecl RB_DrawSunShadowOverlay()
{
    float v0; // [esp+28h] [ebp-C4h]
    float t0; // [esp+3Ch] [ebp-B0h]
    float x0; // [esp+44h] [ebp-A8h]
    float t1; // [esp+48h] [ebp-A4h]
    float clipSpacePoints[9][2]; // [esp+4Ch] [ebp-A0h] BYREF
    int pointIsNear[9]; // [esp+98h] [ebp-54h] BYREF
    float shadowSampleSize; // [esp+BCh] [ebp-30h]
    const GfxViewInfo *viewInfo; // [esp+C0h] [ebp-2Ch]
    int pointIndexDst; // [esp+C4h] [ebp-28h]
    int pointIndexSrc; // [esp+C8h] [ebp-24h]
    int partitionIndex; // [esp+CCh] [ebp-20h]
    float y0; // [esp+D0h] [ebp-1Ch]
    float x; // [esp+D4h] [ebp-18h]
    float y; // [esp+D8h] [ebp-14h]
    float h; // [esp+DCh] [ebp-10h]
    const GfxSunShadow *sunShadow; // [esp+E0h] [ebp-Ch]
    const GfxSunShadowPartition *partition; // [esp+E4h] [ebp-8h]
    float w; // [esp+E8h] [ebp-4h]

    if (!backEndData->viewInfoCount)
        MyAssertHandler(".\\rb_sunshadow.cpp", 163, 0, "%s", "backEndData->viewInfoCount > 0");
    viewInfo = backEndData->viewInfo;
    sunShadow = &viewInfo->sunShadow;
    x0 = 4.0;
    y0 = 4.0;
    h = (double)vidConfig.displayHeight * 0.5;
    w = h;
    RB_SetSunShadowOverlayScaleAndBias();
    gfxCmdBufSourceState.input.codeImageSamplerStates[9] = 97;
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, gfxRenderTargets[13].image);
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        t0 = (double)partitionIndex * 0.5;
        t1 = t0 + 0.5;
        v0 = (double)partitionIndex * w + x0;
        RB_DrawStretchPic(rgp.shadowOverlayMaterial, v0, y0, w, h, 0.0, t0, 1.0, t1, 0xFFFFFFFF, GFX_PRIM_STATS_HUD);
    }
    RB_EndTessSurface();
    gfxCmdBufSourceState.input.codeImageSamplerStates[9] = 98;
    shadowSampleSize = sm_sunSampleSizeNear->current.value;
    pointIndexDst = 0;
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        x = (double)partitionIndex * w + x0;
        y = y0;
        partition = &sunShadow->partition[partitionIndex];
        R_SunShadowMapBoundingPoly(
            &sunShadow->partition[partitionIndex].boundingPoly,
            shadowSampleSize,
            clipSpacePoints,
            pointIsNear);
        for (pointIndexSrc = 0; pointIndexSrc < partition->boundingPoly.pointCount; ++pointIndexSrc)
        {
            if (!partitionIndex
                || !rg.sunShadowFull
                || pointIsNear[pointIndexSrc]
                || pointIsNear[(pointIndexSrc + 1) % partition->boundingPoly.pointCount])
            {
                RB_SunShadowOverlayPoint(clipSpacePoints[pointIndexSrc], x, y, w, h, g_overlayPoints[pointIndexDst].xyz);
                RB_SunShadowOverlayPoint(
                    clipSpacePoints[(pointIndexSrc + 1) % partition->boundingPoly.pointCount],
                    x,
                    y,
                    w,
                    h,
                    g_overlayPoints[pointIndexDst + 1].xyz);
                *(unsigned int *)g_overlayPoints[pointIndexDst].color = -16711936;
                *(unsigned int *)g_overlayPoints[pointIndexDst + 1].color = -16711936;
                pointIndexDst += 2;
            }
        }
        shadowSampleSize = shadowSampleSize * rg.sunShadowPartitionRatio;
    }
    if (pointIndexDst)
    {
        RB_DrawLines2D(pointIndexDst / 2, 1, g_overlayPoints);
        RB_EndTessSurface();
    }
}

void __cdecl RB_SunShadowOverlayPoint(const float *xy, float x0, float y0, float w, float h, float *point)
{
    *point = (*xy * 0.5 + 0.5) * w + x0;
    point[1] = (0.5 - xy[1] * 0.5) * h + y0;
    point[2] = 0.0;
}

void RB_SetSunShadowOverlayScaleAndBias()
{
    float nearDepth; // [esp+18h] [ebp-10h] BYREF
    float bias; // [esp+1Ch] [ebp-Ch]
    float scale; // [esp+20h] [ebp-8h]
    float farDepth; // [esp+24h] [ebp-4h] BYREF

    RB_GetShadowOverlayDepthBounds(&nearDepth, &farDepth);
    scale = 1.0 / (farDepth - nearDepth);
    bias = -scale * nearDepth;
    R_UpdateCodeConstant(&gfxCmdBufSourceState, 0x15u, scale, bias, 1.0, 1.0);
}

