#pragma once
#include "rb_backend.h"
#include "rb_backend.h"


void __cdecl R_ChangeIndices(GfxCmdBufPrimState *state, IDirect3DIndexBuffer9 *ib);
void __cdecl R_ChangeStreamSource(
    GfxCmdBufPrimState *state,
    unsigned int streamIndex,
    IDirect3DVertexBuffer9 *vb,
    unsigned int vertexOffset,
    unsigned int vertexStride);
void __cdecl R_SetTexFilter();
void __cdecl R_SetInitialContextState(IDirect3DDevice9 *device);
void __cdecl R_ChangeDepthHackNearClip(GfxCmdBufSourceState *source, unsigned int depthHackFlags);
void __cdecl R_DepthHackNearClipChanged(GfxCmdBufSourceState *source);
GfxCmdBufSourceState *__cdecl R_GetCodeMatrix(
    GfxCmdBufSourceState *source,
    unsigned int sourceIndex,
    unsigned int firstRow);
void __cdecl R_DeriveCodeMatrix(GfxCmdBufSourceState *source, GfxCodeMatrices *activeMatrices, unsigned int baseIndex);
void __cdecl R_DeriveViewMatrix(GfxCmdBufSourceState *source);
void  R_DeriveWorldViewMatrix(GfxCmdBufSourceState *source);
void __cdecl R_DeriveProjectionMatrix(GfxCmdBufSourceState *source);
void __cdecl R_DeriveViewProjectionMatrix(GfxCmdBufSourceState *source);
void  R_DeriveWorldViewProjectionMatrix(GfxCmdBufSourceState *source);
void __cdecl R_DeriveShadowLookupMatrix(GfxCmdBufSourceState *source);
void  R_GenerateWorldOutdoorLookupMatrix(
    GfxCmdBufSourceState *source,
    float (*outMatrix)[4]);
void __cdecl Vec4Add(const float *a, const float *b, float *sum);
const GfxImage *__cdecl R_GetTextureFromCode(
    GfxCmdBufSourceState *source,
    unsigned int codeTexture,
    unsigned __int8 *samplerState);
void __cdecl R_TextureFromCodeError(GfxCmdBufSourceState *source, unsigned int codeTexture);
const GfxImage *__cdecl R_OverrideGrayscaleImage(const dvar_s *dvar);
void __cdecl R_SetLightmap(GfxCmdBufContext context, unsigned int lmapIndex);
void __cdecl R_SetReflectionProbe(GfxCmdBufContext context, unsigned int reflectionProbeIndex);
void __cdecl R_ChangeDepthRange(GfxCmdBufState *state, GfxDepthRangeType depthRangeType);
void __cdecl R_HW_SetViewport(IDirect3DDevice9 *device, const GfxViewport *viewport, float nearValue, float farValue);
int __cdecl R_BeginMaterial(GfxCmdBufState *state, const Material *material, MaterialTechniqueType techType);
void __cdecl R_ClearAllStreamSources(GfxCmdBufPrimState *state);
void __cdecl R_DrawIndexedPrimitive(GfxCmdBufPrimState *state, const GfxDrawPrimArgs *args);
void __cdecl R_ChangeState_0(GfxCmdBufState *state, unsigned int stateBits0);
void __cdecl R_HW_SetAlphaTestEnable(IDirect3DDevice9 *device, __int16 stateBits0);
void __cdecl R_HW_SetColorMask(IDirect3DDevice9 *device, unsigned int stateBits0);
void __cdecl R_HW_SetCullFace(IDirect3DDevice9 *device, __int16 stateBits0);
void __cdecl R_HW_SetPolygonMode(IDirect3DDevice9 *device, signed int stateBits0);
void __cdecl R_HW_DisableBlend(IDirect3DDevice9 *device);
void __cdecl R_HW_SetBlend(
    IDirect3DDevice9 *device,
    bool blendWasEnabled,
    unsigned int changedBits,
    unsigned int stateBits0);
void __cdecl R_SetAlphaTestFunction(GfxCmdBufState *state, __int16 stateBits0);
void __cdecl R_ChangeState_1(GfxCmdBufState *state, unsigned int stateBits1);
void __cdecl R_HW_SetDepthWriteEnable(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_SetDepthTestEnable(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_SetDepthTestFunction(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_EnableStencil(IDirect3DDevice9 *device);
void __cdecl R_HW_DisableStencil(IDirect3DDevice9 *device);
void __cdecl R_HW_SetFrontStencilOp(
    IDirect3DDevice9 *device,
    unsigned int stencilOpPass,
    unsigned int stencilOpFail,
    unsigned int stencilOpZFail);
void __cdecl R_HW_SetBackStencilOp(
    IDirect3DDevice9 *device,
    unsigned int stencilOpPass,
    unsigned int stencilOpFail,
    unsigned int stencilOpZFail);
void __cdecl R_HW_SetFrontStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc);
void __cdecl R_HW_SetBackStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc);
void __cdecl R_SetSampler(
    GfxCmdBufContext context,
    unsigned int samplerIndex,
    unsigned __int8 samplerState,
    const GfxImage *image);
unsigned int __cdecl R_HW_SetSamplerState(
    IDirect3DDevice9 *device,
    unsigned int samplerIndex,
    unsigned int samplerState,
    unsigned int oldSamplerState);
unsigned int __cdecl R_DecodeSamplerState(unsigned __int8 samplerState);
void __cdecl R_SetSamplerState(GfxCmdBufState *state, unsigned int samplerIndex, unsigned __int8 samplerState);
void __cdecl R_ForceSetBlendState(IDirect3DDevice9 *device, unsigned int stateBits0);
void __cdecl R_ForceSetStencilState(IDirect3DDevice9 *device, unsigned int stateBits1);
void __cdecl R_GetViewport(GfxCmdBufSourceState *source, GfxViewport *outViewport);
void __cdecl R_SetViewport(GfxCmdBufState *state, const GfxViewport *viewport);
void __cdecl R_SetViewportStruct(GfxCmdBufSourceState *source, const GfxViewport *viewport);
void __cdecl R_SetViewportValues(GfxCmdBufSourceState *source, int x, int y, int width, int height);
void __cdecl R_UpdateViewport(GfxCmdBufSourceState *source, GfxViewport *viewport);
void __cdecl R_DisableSampler(GfxCmdBufState *state, unsigned int samplerIndex);
void __cdecl R_HW_DisableSampler(IDirect3DDevice9 *device, unsigned int samplerIndex);
void __cdecl R_UnbindImage(GfxCmdBufState *state, const GfxImage *image);
void __cdecl R_SetRenderTargetSize(GfxCmdBufSourceState *source, GfxRenderTargetId newTargetId);
GfxViewportBehavior __cdecl R_ViewportBehaviorForRenderTarget(GfxRenderTargetId renderTargetId);
void __cdecl R_SetRenderTarget(GfxCmdBufContext context, GfxRenderTargetId newTargetId);
void __cdecl R_HW_SetRenderTarget(GfxCmdBufState *state, GfxRenderTargetId newTargetId);
void __cdecl R_UpdateStatsTarget(int newTargetId);
void __cdecl R_ClearScreenInternal(
    IDirect3DDevice9 *device,
    unsigned __int8 whichToClear,
    const float *color,
    float depth,
    unsigned __int8 stencil,
    const GfxViewport *viewport);
void __cdecl R_ClearScreen(
    IDirect3DDevice9 *device,
    unsigned __int8 whichToClear,
    const float *color,
    float depth,
    unsigned __int8 stencil,
    const GfxViewport *viewport);
void __cdecl R_ForceSetPolygonOffset(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_SetPolygonOffset(IDirect3DDevice9 *device, float scale, float bias);
void __cdecl R_SetMeshStream(GfxCmdBufState *state, GfxMeshData *mesh);
void __cdecl R_SetCompleteState(IDirect3DDevice9 *device, unsigned int *stateBits);
// bad sp value at call has been detected, the output may be wrong!
void  R_DrawCall(
    void(__cdecl *callback)(const void *, GfxCmdBufContext*, GfxCmdBufContext*),
    const void *userData,
    GfxCmdBufSourceState *source,
    const GfxViewInfo *viewInfo,
    const GfxDrawSurfListInfo *info,
    const GfxViewParms *viewParms,
    GfxCmdBuf *cmdBuf,
    GfxCmdBuf *prepassCmdBuf);
void __cdecl R_SetCodeConstant(GfxCmdBufSourceState *source, unsigned int constant, float x, float y, float z, float w);
void __cdecl R_SetAlphaAntiAliasingState(IDirect3DDevice9 *device, __int16 stateBits0);
