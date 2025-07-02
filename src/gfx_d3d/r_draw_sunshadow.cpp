#include "r_draw_sunshadow.h"
#include "rb_backend.h"
#include "r_state.h"
#include "r_utils.h"
#include "r_dvars.h"
#include "rb_postfx.h"


void __cdecl R_DrawSunShadowMapCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    int height; // [esp+10h] [ebp-28h]
    int width; // [esp+14h] [ebp-24h]
    int verticalOffset; // [esp+18h] [ebp-20h]
    RECT rect; // [esp+24h] [ebp-14h] BYREF
    const GfxSunShadowPartition *partition; // [esp+34h] [ebp-4h]

    partition = (const GfxSunShadowPartition * )userData;
    R_SetRenderTarget(context, R_RENDERTARGET_SHADOWMAP_SUN);

    if (!partition->partitionIndex)
        R_ClearScreen(context.state->prim.device, 3, shadowmapClearColor, 1.0f, 0, 0);

    height = partition->viewport.height;
    width = partition->viewport.width;
    verticalOffset = partition->viewport.y + (partition->partitionIndex * 1024);

    rect.left = partition->viewport.x;
    rect.top = verticalOffset;
    rect.right = width + rect.left;
    rect.bottom = height + verticalOffset;

    context.state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1);
    context.state->prim.device->SetScissorRect(&rect);

    R_DrawSurfs(context, 0, &partition->info);

    context.state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
}

void R_DrawSunShadowMap(
    const GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    GfxCmdBuf *cmdBuf)
{
    float x; // [esp+14h] [ebp-F28h]
    GfxCmdBufSourceState state; // [esp+20h] [ebp-F1Ch] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 0);
    R_SetRenderTargetSize(&state, R_RENDERTARGET_SHADOWMAP_SUN);

    if (r_rendererInUse->current.integer || gfxMetrics.shadowmapBuildTechType != TECHNIQUE_BUILD_SHADOWMAP_COLOR)
    {
        x = sm_polygonOffsetBias->current.value * 0.25f * viewInfo->sunShadow.partition[partitionIndex].shadowViewParms.projectionMatrix.m[2][2];
        R_UpdateCodeConstant(&state, 9, x, sm_polygonOffsetScale->current.value, 0.0f, 0.0f);
    }
    else
    {
        x = sm_polygonOffsetBias->current.value * 4.0f * viewInfo->sunShadow.partition[partitionIndex].shadowViewParms.projectionMatrix.m[2][2];
        R_UpdateCodeConstant(&state, 9, x, 0.0f, 0.0f, 0.0f);
    }

    R_SetViewportValues(&state, 0, partitionIndex * 1024, 1024, 1024);
    R_DrawCall(
        R_DrawSunShadowMapCallback,
        &viewInfo->sunShadow.partition[partitionIndex],
        &state,
        viewInfo,
        &viewInfo->sunShadow.partition[partitionIndex].info,
        &viewInfo->sunShadow.partition[partitionIndex].shadowViewParms,
        cmdBuf,
        0);
}