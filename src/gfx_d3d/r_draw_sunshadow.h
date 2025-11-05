#pragma once

const float shadowmapClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void R_DrawSunShadowMap(
    const struct GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    struct GfxCmdBuf *cmdBuf);

// LWSS ADD from blops
void __cdecl DrawSunDirectionDebug(const float *viewOrg, const float *viewForward);