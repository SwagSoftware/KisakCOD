#include "rb_state.h"
#include <qcommon/mem_track.h>

GfxCmdBufContext gfxCmdBufContext;
GfxCmdBufSourceState gfxCmdBufSourceState;
GfxCmdBufState gfxCmdBufState;
GfxCmdBufInput gfxCmdBufInput;

const unsigned int s_cullTable_21[4] = { 0u, 1u, 3u, 2u }; // idb
const unsigned int s_blendTable_21[11] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u }; // idb
const unsigned int s_stencilOpTable_21[8] = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u }; // idb
const unsigned int s_stencilFuncTable_21[8] = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u }; // idb

void __cdecl TRACK_rb_state()
{
    track_static_alloc_internal((void *)s_blendTable_21, 44, "s_blendTable", 18);
    track_static_alloc_internal((void *)s_cullTable_21, 16, "s_cullTable", 18);
    track_static_alloc_internal((void *)s_stencilOpTable_21, 32, "s_stencilOpTable", 18);
    track_static_alloc_internal((void *)s_stencilFuncTable_21, 32, "s_stencilFuncTable", 18);
}

void __cdecl RB_SetInitialState()
{
    memset((unsigned __int8 *)&gfxCmdBufInput, 0, sizeof(gfxCmdBufInput));
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
    gfxCmdBufState.prim.device = dx.device;
    R_SetInitialContextState(dx.device);
    R_SetTexFilter();
    R_InitCmdBufState(&gfxCmdBufState);
    RB_InitSceneViewport();
    if (!stru_EA74F04.surface.color)
        MyAssertHandler(".\\rb_state.cpp", 62, 0, "%s", "gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color");
    if (!g_allocateMinimalResources && !stru_EA74F04.surface.depthStencil)
        MyAssertHandler(
            ".\\rb_state.cpp",
            64,
            0,
            "%s",
            "gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.depthStencil");
    if (!stru_EA74F04.width)
        MyAssertHandler(".\\rb_state.cpp", 66, 0, "%s", "gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].width");
    if (!stru_EA74F04.height)
        MyAssertHandler(".\\rb_state.cpp", 67, 0, "%s", "gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].height");
    if (gfxCmdBufSourceState.renderTargetWidth)
        MyAssertHandler(".\\rb_state.cpp", 68, 1, "%s", "gfxCmdBufSourceState.renderTargetWidth == 0");
    if (gfxCmdBufSourceState.renderTargetHeight)
        MyAssertHandler(".\\rb_state.cpp", 69, 1, "%s", "gfxCmdBufSourceState.renderTargetHeight == 0");
    if (gfxCmdBufSourceState.viewportBehavior)
        MyAssertHandler(".\\rb_state.cpp", 70, 1, "%s", "gfxCmdBufSourceState.viewportBehavior == 0");
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    if (gfxCmdBufState.prim.indexBuffer)
        MyAssertHandler(".\\rb_state.cpp", 76, 1, "%s", "gfxCmdBufState.prim.indexBuffer == NULL");
    if (gfxCmdBufState.prim.streams[0].vb)
        MyAssertHandler(".\\rb_state.cpp", 77, 1, "%s", "gfxCmdBufState.prim.streams[0].vb == NULL");
    if (gfxCmdBufState.prim.streams[0].offset)
        MyAssertHandler(".\\rb_state.cpp", 78, 1, "%s", "gfxCmdBufState.prim.streams[0].offset == 0");
    if (gfxCmdBufState.prim.streams[0].stride)
        MyAssertHandler(".\\rb_state.cpp", 79, 1, "%s", "gfxCmdBufState.prim.streams[0].stride == 0");
    if (gfxCmdBufState.prim.streams[1].vb)
        MyAssertHandler(".\\rb_state.cpp", 80, 1, "%s", "gfxCmdBufState.prim.streams[1].vb == NULL");
    if (gfxCmdBufState.prim.streams[1].offset)
        MyAssertHandler(".\\rb_state.cpp", 81, 1, "%s", "gfxCmdBufState.prim.streams[1].offset == 0");
    if (gfxCmdBufState.prim.streams[1].stride)
        MyAssertHandler(".\\rb_state.cpp", 82, 1, "%s", "gfxCmdBufState.prim.streams[1].stride == 0");
    if (gfxCmdBufState.vertexShader)
        MyAssertHandler(".\\rb_state.cpp", 86, 1, "%s", "gfxCmdBufState.vertexShader == NULL");
    if (gfxCmdBufState.material)
        MyAssertHandler(".\\rb_state.cpp", 88, 1, "%s", "gfxCmdBufState.material == NULL");
    if (gfxCmdBufState.technique)
        MyAssertHandler(".\\rb_state.cpp", 89, 1, "%s", "gfxCmdBufState.technique == NULL");
    if (gfxCmdBufState.alphaRef)
        MyAssertHandler(".\\rb_state.cpp", 92, 1, "%s", "gfxCmdBufState.alphaRef == 0");
}

void __cdecl RB_InitSceneViewport()
{
    gfxCmdBufSourceState.sceneViewport.x = 0;
    gfxCmdBufSourceState.sceneViewport.y = 0;
    gfxCmdBufSourceState.sceneViewport.width = vidConfig.displayWidth;
    gfxCmdBufSourceState.sceneViewport.height = vidConfig.displayHeight;
}

void __cdecl RB_InitImages()
{
    RB_InitCodeImages();
    RB_BindDefaultImages();
}

