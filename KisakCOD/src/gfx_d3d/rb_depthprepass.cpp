#include "rb_depthprepass.h"



void __usercall R_DepthPrepass(
    int a1@<ebp>,
    GfxRenderTargetId renderTargetId,
    const GfxViewInfo *viewInfo,
    GfxCmdBuf *cmdBuf)
{
    _BYTE v4[12]; // [esp-Ch] [ebp-F1Ch] BYREF
    _DWORD v5[3]; // [esp+F04h] [ebp-Ch] BYREF
    _UNKNOWN *retaddr; // [esp+F10h] [ebp+0h]

    v5[0] = a1;
    v5[1] = retaddr;
    R_InitCmdBufSourceState((GfxCmdBufSourceState *)v4, &viewInfo->input, 1);
    R_SetRenderTargetSize((GfxCmdBufSourceState *)v4, renderTargetId);
    R_SetViewportStruct((GfxCmdBufSourceState *)v4, &viewInfo->sceneViewport);
    R_DrawCall(
        (int)v5,
        (void(__cdecl *)(const void *, GfxCmdBufContext, GfxCmdBufContext))R_DepthPrepassCallback,
        viewInfo,
        (GfxCmdBufSourceState *)v4,
        viewInfo,
        0,
        &viewInfo->viewParms,
        cmdBuf,
        0);
}

void __cdecl R_DepthPrepassCallback(const GfxViewInfo *userData, GfxCmdBufContext context)
{
    int height; // [esp+10h] [ebp-54h]
    int width; // [esp+14h] [ebp-50h]
    int y; // [esp+18h] [ebp-4Ch]
    IDirect3DDevice9 *device; // [esp+20h] [ebp-44h]
    _DWORD v6[4]; // [esp+24h] [ebp-40h] BYREF
    GfxDrawSurfListInfo info; // [esp+34h] [ebp-30h] BYREF
    MaterialTechniqueType baseTechType; // [esp+5Ch] [ebp-8h]
    const GfxViewInfo *viewInfo; // [esp+60h] [ebp-4h]

    viewInfo = userData;
    height = userData->scissorViewport.height;
    width = userData->scissorViewport.width;
    y = userData->scissorViewport.y;
    device = context.state->prim.device;
    v6[0] = userData->scissorViewport.x;
    v6[1] = y;
    v6[2] = width + v6[0];
    v6[3] = height + y;
    device->SetRenderState(device, D3DRS_SCISSORTESTENABLE, 1u);
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, _DWORD *))device->SetScissorRect)(device, device, v6);
    if (viewInfo->needsFloatZ)
    {
        if (!renderTarget.surface.color)
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
    ((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, _DWORD))context.state->prim.device->SetRenderState)(
        context.state->prim.device,
        context.state->prim.device,
        174,
        0);
}