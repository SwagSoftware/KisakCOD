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
#include <universal/profile.h>

GfxPointVertex g_overlayPoints[36];

void __cdecl TRACK_rb_sunshadow()
{
    track_static_alloc_internal(g_overlayPoints, 576, "g_overlayPoints", 18);
}

void __cdecl RB_SunShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo)
{
    GfxCmdBuf cmdBuf; // [esp+0h] [ebp-8h] BYREF
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    if (pixelCostMode == GFX_PIXEL_COST_MODE_OFF)
    {
        iassert(data);

        R_InitContext(data, &cmdBuf);

        PROF_SCOPED("Sun Shadow Maps");

        for (int partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
        {
            ZoneTextF("Sun Shadow Map %d", partitionIndex);
            R_DrawSunShadowMap(viewInfo, partitionIndex, &cmdBuf);
        }
    }
}

void __cdecl RB_GetShadowOverlayDepthBounds(float *nearDepth, float *farDepth)
{
    *nearDepth = sm_showOverlayDepthBounds->current.vector[0];
    *farDepth = sm_showOverlayDepthBounds->current.vector[1];

    if (I_fabs(*farDepth - *nearDepth) < 0.01f)
    {
        if (*farDepth <= *nearDepth)
        {
            *farDepth = (*nearDepth + *farDepth) * 0.5 - 0.005f;
            *nearDepth = *farDepth + 0.01f;
        }
        else
        {
            *nearDepth = (*nearDepth + *farDepth) * 0.5 - 0.005f;
            *farDepth = *nearDepth + 0.01f;
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

    iassert(backEndData->viewInfoCount > 0);
    viewInfo = backEndData->viewInfo;
    sunShadow = &viewInfo->sunShadow;
    x0 = 4.0f;
    y0 = 4.0f;
    h = (float)vidConfig.displayHeight * 0.5f;
    w = h;
    RB_SetSunShadowOverlayScaleAndBias();
    gfxCmdBufSourceState.input.codeImageSamplerStates[9] = 97;
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9, gfxRenderTargets[13].image);
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        t0 = (float)partitionIndex * 0.5f;
        t1 = t0 + 0.5f;
        v0 = (float)partitionIndex * w + x0;
        RB_DrawStretchPic(rgp.shadowOverlayMaterial, v0, y0, w, h, 0.0f, t0, 1.0f, t1, 0xFFFFFFFF, GFX_PRIM_STATS_HUD);
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
            &clipSpacePoints,
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
    point[0] = ((xy[0] * 0.5f) + 0.5f) * w + x0;
    point[1] = (0.5f - (xy[1] * 0.5f)) * h + y0;
    point[2] = 0.0f;
}

void RB_SetSunShadowOverlayScaleAndBias()
{
    float nearDepth; // [esp+18h] [ebp-10h] BYREF
    float bias; // [esp+1Ch] [ebp-Ch]
    float scale; // [esp+20h] [ebp-8h]
    float farDepth; // [esp+24h] [ebp-4h] BYREF

    RB_GetShadowOverlayDepthBounds(&nearDepth, &farDepth);
    scale = 1.0f / (farDepth - nearDepth);
    bias = -scale * nearDepth;
    R_UpdateCodeConstant(&gfxCmdBufSourceState, 0x15u, scale, bias, 1.0f, 1.0f);
}

