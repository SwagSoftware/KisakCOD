#pragma once
#include "rb_backend.h"


void __cdecl TRACK_rb_state();
void __cdecl RB_SetInitialState();
void __cdecl RB_InitSceneViewport();
void __cdecl RB_InitImages();


extern GfxCmdBufContext gfxCmdBufContext;
extern GfxCmdBufSourceState gfxCmdBufSourceState;
extern GfxCmdBufState gfxCmdBufState;
extern GfxCmdBufInput gfxCmdBufInput;