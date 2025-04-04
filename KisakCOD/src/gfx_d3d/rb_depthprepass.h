#pragma once


void __cdecl R_DepthPrepassCallback(const GfxViewInfo *userData, GfxCmdBufContext context);
void __usercall R_DepthPrepass(
    int a1@<ebp>,
    GfxRenderTargetId renderTargetId,
    const GfxViewInfo *viewInfo,
    GfxCmdBuf *cmdBuf);