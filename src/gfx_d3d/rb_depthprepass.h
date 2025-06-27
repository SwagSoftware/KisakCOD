#pragma once

#include "r_rendercmds.h"
#include "rb_backend.h"

void R_DepthPrepass(
    GfxRenderTargetId renderTargetId,
    const struct GfxViewInfo *viewInfo,
    struct GfxCmdBuf *cmdBuf);