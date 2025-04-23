#pragma once
#include <d3d9.h>
#include "rb_backend.h"

struct GfxPointLitSurfsInfo // sizeof=0x1C
{                                       // ...
    const GfxViewInfo *viewInfo;        // ...
    GfxMeshData *clearQuadMesh;         // ...
    const GfxDrawSurfListInfo *drawSurfInfo; // ...
    unsigned int x;                     // ...
    unsigned int y;                     // ...
    unsigned int w;                     // ...
    unsigned int h;                     // ...
};

void __cdecl R_HW_InsertFence(IDirect3DQuery9 **fence);
void __cdecl R_ShowTris(GfxCmdBufContext context, const GfxDrawSurfListInfo *info);
void __cdecl RB_Draw3DInternal(const GfxViewInfo *viewInfo);
void __cdecl RB_FullbrightDrawCommands(const GfxViewInfo *viewInfo);
void __cdecl RB_EndSceneRendering(GfxCmdBufContext context, const GfxCmdBufInput *input, const GfxViewInfo *viewInfo);
void __cdecl R_SetAndClearSceneTarget(const GfxViewport *viewport);
void __cdecl R_ClearForFrameBuffer(IDirect3DDevice9 *device, const GfxViewport *viewport);
void __cdecl R_DrawFullbright(const GfxViewInfo *viewInfo, GfxCmdBufInput *input, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawDebugShaderLitCallback(const GfxViewInfo *data, GfxCmdBufContext context);
void __cdecl R_DrawFullbrightDecalCallback(const GfxViewInfo *data, GfxCmdBufContext context);
void __cdecl R_DrawDebugShaderEmissiveCallback(const GfxViewInfo *data, GfxCmdBufContext context);
void __cdecl RB_DebugShaderDrawCommands(const GfxViewInfo *viewInfo);
void __cdecl R_DrawDebugShader(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawDebugShaderDecalCallback(char *data, GfxCmdBufContext context);
void __cdecl RB_StandardDrawCommands(const GfxViewInfo *viewInfo);
void  R_DrawLights(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawPointLitSurfs(GfxCmdBufSourceState *source, const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawPointLitSurfsCallback(const GfxPointLitSurfsInfo *userData, GfxCmdBufContext *context, GfxCmdBufContext *prepassContext);
void  R_DrawEmissive(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawEmissiveCallback(const GfxViewInfo *userData, GfxCmdBufContext *context, GfxCmdBufContext *prepassContext);
void __cdecl RB_Draw3DCommon();
void __cdecl R_SetResolvedScene(GfxCmdBufContext context);
GfxCmdBufSourceState *RB_DebugShaderDrawCommandsCommon();
void RB_StandardDrawCommandsCommon();
void __cdecl RB_ApplyLatePostEffects(const GfxViewInfo *viewInfo);
void RB_DrawDebugPostEffects();
void RB_ShowFbColorDebug_Screen();
void RB_ShowFbColorDebug_Feedback();
void RB_ShowFloatZDebug();
void RB_ShowShadowsDebug();
