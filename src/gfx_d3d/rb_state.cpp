#include "rb_state.h"
#include <qcommon/mem_track.h>
#include "r_utils.h"
#include "r_state.h"

GfxCmdBufSourceState gfxCmdBufSourceState;
GfxCmdBufState gfxCmdBufState;
GfxCmdBufInput gfxCmdBufInput;

const GfxCmdBufContext gfxCmdBufContext = {
    .source = &gfxCmdBufSourceState,
    .state = &gfxCmdBufState
};

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
    iassert( gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color );
    if (!g_allocateMinimalResources && !gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.depthStencil)
        MyAssertHandler(
            ".\\rb_state.cpp",
            64,
            0,
            "%s",
            "gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.depthStencil");
    iassert( gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].width );
    iassert( gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].height );
    iassert( gfxCmdBufSourceState.renderTargetWidth == 0 );
    iassert( gfxCmdBufSourceState.renderTargetHeight == 0 );
    iassert( gfxCmdBufSourceState.viewportBehavior == 0 );
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    iassert( gfxCmdBufState.prim.indexBuffer == NULL );
    iassert( gfxCmdBufState.prim.streams[0].vb == NULL );
    iassert( gfxCmdBufState.prim.streams[0].offset == 0 );
    iassert( gfxCmdBufState.prim.streams[0].stride == 0 );
    iassert( gfxCmdBufState.prim.streams[1].vb == NULL );
    iassert( gfxCmdBufState.prim.streams[1].offset == 0 );
    iassert( gfxCmdBufState.prim.streams[1].stride == 0 );
    iassert( gfxCmdBufState.vertexShader == NULL );
    iassert( gfxCmdBufState.material == NULL );
    iassert( gfxCmdBufState.technique == NULL );
    iassert( gfxCmdBufState.alphaRef == 0 );
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

