#include "rb_depthprepass.h"
#include "r_rendercmds.h"
#include "rb_backend.h"
#include "r_utils.h"
#include "r_state.h"
#include "r_meshdata.h"


void R_DepthPrepassCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    int height; // [esp+10h] [ebp-54h]
    int width; // [esp+14h] [ebp-50h]
    int y; // [esp+18h] [ebp-4Ch]
    IDirect3DDevice9 *device; // [esp+20h] [ebp-44h]
    tagRECT v6; // [esp+24h] [ebp-40h] BYREF
    GfxDrawSurfListInfo info; // [esp+34h] [ebp-30h] BYREF
    MaterialTechniqueType baseTechType; // [esp+5Ch] [ebp-8h]
    const GfxViewInfo *viewInfo; // [esp+60h] [ebp-4h]

    viewInfo = (const GfxViewInfo * )userData;
    height = viewInfo->scissorViewport.height;
    width = viewInfo->scissorViewport.width;
    y = viewInfo->scissorViewport.y;
    device = context.state->prim.device;
    v6.left = viewInfo->scissorViewport.x;
    v6.top = y;
    v6.right = width + v6.left;
    v6.bottom = height + y;
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1u);
    device->SetScissorRect(&v6);
    if (viewInfo->needsFloatZ)
    {
        if (!gfxRenderTargets[5].surface.color)
            MyAssertHandler(".\\rb_depthprepass.cpp", 31, 0, "%s", "R_HaveFloatZ()");
        R_SetRenderTarget(context, R_RENDERTARGET_FLOAT_Z);
        baseTechType = TECHNIQUE_BUILD_FLOAT_Z;
        R_DrawQuadMesh(context, rgp.shadowClearMaterial, &viewInfo->fullSceneViewMesh->meshData);
        context.source->input.consts[54][0] = 0.0;
        context.source->input.consts[54][1] = 0.0;
        context.source->input.consts[54][2] = 0.0;
        context.source->input.consts[54][3] = 1.0;
        R_DirtyCodeConstant(context.source, 0x36u);
    }
    else
    {
        R_SetRenderTarget(context, R_RENDERTARGET_DYNAMICSHADOWS);
        baseTechType = TECHNIQUE_DEPTH_PREPASS;
    }
    qmemcpy(&info, &viewInfo->litInfo, sizeof(info));
    info.baseTechType = baseTechType;
    R_DrawSurfs(context, 0, &info);
    qmemcpy(&info, &viewInfo->decalInfo, sizeof(info));
    info.baseTechType = baseTechType;
    R_DrawSurfs(context, 0, &info);
    //(context.state->prim.device->SetRenderState)(context.state->prim.device, context.state->prim.device, 174, 0);
    context.state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
}

void R_DepthPrepass(
    GfxRenderTargetId renderTargetId,
    const GfxViewInfo *viewInfo,
    GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState v6; // [sp+50h] [-F00h] BYREF

    R_InitCmdBufSourceState(&v6, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v6, renderTargetId);
    R_SetViewportStruct(&v6, &viewInfo->sceneViewport);
    R_DrawCall(R_DepthPrepassCallback, viewInfo, &v6, viewInfo, 0, &viewInfo->viewParms, cmdBuf, 0);
}
