#pragma once
#include "r_rendercmds.h"


void R_DrawSunShadowMap(
    const GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    GfxCmdBuf *cmdBuf);