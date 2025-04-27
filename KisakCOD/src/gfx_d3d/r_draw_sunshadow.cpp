#include "r_draw_sunshadow.h"
#include "rb_backend.h"
#include "r_state.h"
#include "r_utils.h"
#include "r_dvars.h"
#include "rb_postfx.h"


void __cdecl R_DrawSunShadowMapCallback(const GfxSunShadowPartition *userData, GfxCmdBufContext context)
{
    int height; // [esp+10h] [ebp-28h]
    int width; // [esp+14h] [ebp-24h]
    int v4; // [esp+18h] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+20h] [ebp-18h]
    RECT v6; // [esp+24h] [ebp-14h] BYREF
    const GfxSunShadowPartition *partition; // [esp+34h] [ebp-4h]

    partition = userData;
    R_SetRenderTarget(context, R_RENDERTARGET_SHADOWMAP_SUN);
    if (!userData->partitionIndex)
        R_ClearScreen(context.state->prim.device, 3u, shadowmapClearColor, 1.0, 0, 0);
    height = partition->viewport.height;
    width = partition->viewport.width;
    v4 = partition->viewport.y + (partition->partitionIndex << 10);
    device = context.state->prim.device;
    v6.left = partition->viewport.x;
    v6.top = v4;
    v6.right = width + v6.left;
    v6.bottom = height + v4;
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1u);
    device->SetScissorRect(&v6);
    R_DrawSurfs(context, 0, &partition->info);
    context.state->prim.device->SetRenderState(
        D3DRS_SCISSORTESTENABLE,
        0);
}

void R_DrawSunShadowMap(
    const GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    GfxCmdBuf *cmdBuf)
{
    float v4; // [esp+10h] [ebp-F2Ch]
    float x; // [esp+14h] [ebp-F28h]
    float v6; // [esp+18h] [ebp-F24h]
    int v7; // [esp+20h] [ebp-F1Ch] BYREF
    _DWORD v8[3]; // [esp+F30h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+F3Ch] [ebp+0h]

    //v8[0] = a1;
    //v8[1] = retaddr;
    R_InitCmdBufSourceState((GfxCmdBufSourceState *)&v7, &viewInfo->input, 0);
    R_SetRenderTargetSize((GfxCmdBufSourceState *)&v7, R_RENDERTARGET_SHADOWMAP_SUN);
    v6 = viewInfo->sunShadow.partition[partitionIndex].shadowViewParms.projectionMatrix.m[2][2];
    if (r_rendererInUse->current.integer || gfxMetrics.shadowmapBuildTechType != TECHNIQUE_BUILD_SHADOWMAP_COLOR)
    {
        v4 = sm_polygonOffsetBias->current.value * 0.25 * v6;
        R_UpdateCodeConstant((GfxCmdBufSourceState *)&v7, 9u, v4, sm_polygonOffsetScale->current.value, 0.0, 0.0);
    }
    else
    {
        x = sm_polygonOffsetBias->current.value * 4.0 * v6;
        R_UpdateCodeConstant((GfxCmdBufSourceState *)&v7, 9u, x, 0.0, 0.0, 0.0);
    }
    R_SetViewportValues((GfxCmdBufSourceState *)&v7, 0, partitionIndex << 10, 1024, 1024);
    R_DrawCall(
        (void(__cdecl *)(const void *, GfxCmdBufContext*, GfxCmdBufContext*))R_DrawSunShadowMapCallback,
        &viewInfo->sunShadow.partition[partitionIndex],
        (GfxCmdBufSourceState *)&v7,
        viewInfo,
        &viewInfo->sunShadow.partition[partitionIndex].info,
        &viewInfo->sunShadow.partition[partitionIndex].shadowViewParms,
        cmdBuf,
        0);
}