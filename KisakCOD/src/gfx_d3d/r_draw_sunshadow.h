#pragma once


void __cdecl R_DrawSunShadowMapCallback(const GfxSunShadowPartition *userData, GfxCmdBufContext context);
void __usercall R_DrawSunShadowMap(
    int a1@<ebp>,
    const GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    GfxCmdBuf *cmdBuf);