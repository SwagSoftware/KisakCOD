#include "r_draw_lit.h"

#include "r_rendercmds.h"
#include "rb_backend.h"
#include "r_utils.h"
#include "r_state.h"


void __cdecl R_DrawLitCallback(const GfxViewInfo *userData, GfxCmdBufContext *context, GfxCmdBufContext *prepassContext)
{
    int height; // [esp+4h] [ebp-28h]
    int width; // [esp+8h] [ebp-24h]
    int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    tagRECT v7; // [esp+18h] [ebp-14h] BYREF
    const GfxViewInfo *viewInfo; // [esp+28h] [ebp-4h]

    viewInfo = userData;
    R_SetRenderTarget(*context, R_RENDERTARGET_SCENE);
    if (prepassContext->state)
        R_SetRenderTarget(*prepassContext, R_RENDERTARGET_SCENE);
    height = viewInfo->scissorViewport.height;
    width = viewInfo->scissorViewport.width;
    y = viewInfo->scissorViewport.y;
    device = context->state->prim.device;
    v7.left = viewInfo->scissorViewport.x;
    v7.top = y;
    v7.right = width + v7.left;
    v7.bottom = height + y;
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1u);
    device->SetScissorRect(&v7);
    R_DrawSurfs(*context, prepassContext->state, &viewInfo->litInfo);
    context->state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
}

void R_DrawLit(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf, GfxCmdBuf *prepassCmdBuf)
{
    GfxCmdBufSourceState v6; // [sp+50h] [-F00h] BYREF

    R_InitCmdBufSourceState(&v6, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v6, R_RENDERTARGET_SCENE);
    R_SetViewportStruct(&v6, &viewInfo->sceneViewport);
    R_DrawCall(
        (void(*)(const void*, GfxCmdBufContext*, GfxCmdBufContext*))R_DrawLitCallback,
        viewInfo,
        &v6,
        viewInfo,
        &viewInfo->litInfo,
        &viewInfo->viewParms,
        cmdBuf,
        prepassCmdBuf);
}

void __cdecl R_DrawDecalCallback(const GfxViewInfo *viewInfo, GfxCmdBufContext *context, GfxCmdBufContext *prepassContext)
{
    int height; // [esp+4h] [ebp-28h]
    int width; // [esp+8h] [ebp-24h]
    int y; // [esp+Ch] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-18h]
    tagRECT v7; // [esp+18h] [ebp-14h] BYREF

    R_SetRenderTarget(*context, R_RENDERTARGET_SCENE);
    if (prepassContext->state)
        R_SetRenderTarget(*prepassContext, R_RENDERTARGET_SCENE);
    height = viewInfo->scissorViewport.height;
    width = viewInfo->scissorViewport.width;
    y = viewInfo->scissorViewport.y;
    device = context->state->prim.device;
    v7.left = viewInfo->scissorViewport.x;
    v7.top = y;
    v7.right = width + v7.left;
    v7.bottom = height + y;
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1u);
    device->SetScissorRect(&v7);
    R_DrawSurfs(*context, prepassContext->state, &viewInfo->decalInfo);
    context->state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
}

void R_DrawDecal(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf, GfxCmdBuf *prepassCmdBuf)
{
    GfxCmdBufSourceState v6; // [sp+50h] [-F00h] BYREF

    R_InitCmdBufSourceState(&v6, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v6, R_RENDERTARGET_SCENE);
    R_SetViewportStruct(&v6, &viewInfo->sceneViewport);
    R_DrawCall(
        (void(*)(const void*, GfxCmdBufContext*, GfxCmdBufContext*))R_DrawDecalCallback,
        viewInfo,
        &v6,
        viewInfo,
        &viewInfo->decalInfo,
        &viewInfo->viewParms,
        cmdBuf,
        prepassCmdBuf);
}