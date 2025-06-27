#pragma once

#include "rb_backend.h"
#include "r_init.h"
#include "r_rendercmds.h"

void __cdecl R_InitContext(const GfxBackEndData *data, GfxCmdBuf *cmdBuf);
void __cdecl R_CmdBufSet3D(GfxCmdBufSourceState *source);