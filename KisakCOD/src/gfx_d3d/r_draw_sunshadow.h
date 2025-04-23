#pragma once

const float shadowmapClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void R_DrawSunShadowMap(
    const struct GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    struct GfxCmdBuf *cmdBuf);