#pragma once
#include "rb_backend.h"
#include "rb_backend.h"

#define CONST_SRC_MATRIX_INVERSE_BIT 1
#define CONST_SRC_MATRIX_TRANSPOSE_BIT 2

enum $03401F5063F140792915F3E4BD6A646F : __int32
{
    CONST_SRC_CODE_MAYBE_DIRTY_PS_BEGIN = 0x0,
    CONST_SRC_CODE_LIGHT_POSITION = 0x0,
    CONST_SRC_CODE_LIGHT_DIFFUSE = 0x1,
    CONST_SRC_CODE_LIGHT_SPECULAR = 0x2,
    CONST_SRC_CODE_LIGHT_SPOTDIR = 0x3,
    CONST_SRC_CODE_LIGHT_SPOTFACTORS = 0x4,
    CONST_SRC_CODE_NEARPLANE_ORG = 0x5,
    CONST_SRC_CODE_NEARPLANE_DX = 0x6,
    CONST_SRC_CODE_NEARPLANE_DY = 0x7,
    CONST_SRC_CODE_SHADOW_PARMS = 0x8,
    CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET = 0x9,
    CONST_SRC_CODE_RENDER_TARGET_SIZE = 0xA,
    CONST_SRC_CODE_LIGHT_FALLOFF_PLACEMENT = 0xB,
    CONST_SRC_CODE_DOF_EQUATION_VIEWMODEL_AND_FAR_BLUR = 0xC,
    CONST_SRC_CODE_DOF_EQUATION_SCENE = 0xD,
    CONST_SRC_CODE_DOF_LERP_SCALE = 0xE,
    CONST_SRC_CODE_DOF_LERP_BIAS = 0xF,
    CONST_SRC_CODE_DOF_ROW_DELTA = 0x10,
    CONST_SRC_CODE_PARTICLE_CLOUD_COLOR = 0x11,
    CONST_SRC_CODE_GAMETIME = 0x12,
    CONST_SRC_CODE_MAYBE_DIRTY_PS_END = 0x13,
    CONST_SRC_CODE_ALWAYS_DIRTY_PS_BEGIN = 0x13,
    CONST_SRC_CODE_PIXEL_COST_FRACS = 0x13,
    CONST_SRC_CODE_PIXEL_COST_DECODE = 0x14,
    CONST_SRC_CODE_FILTER_TAP_0 = 0x15,
    CONST_SRC_CODE_FILTER_TAP_1 = 0x16,
    CONST_SRC_CODE_FILTER_TAP_2 = 0x17,
    CONST_SRC_CODE_FILTER_TAP_3 = 0x18,
    CONST_SRC_CODE_FILTER_TAP_4 = 0x19,
    CONST_SRC_CODE_FILTER_TAP_5 = 0x1A,
    CONST_SRC_CODE_FILTER_TAP_6 = 0x1B,
    CONST_SRC_CODE_FILTER_TAP_7 = 0x1C,
    CONST_SRC_CODE_COLOR_MATRIX_R = 0x1D,
    CONST_SRC_CODE_COLOR_MATRIX_G = 0x1E,
    CONST_SRC_CODE_COLOR_MATRIX_B = 0x1F,
    CONST_SRC_CODE_ALWAYS_DIRTY_PS_END = 0x20,
    CONST_SRC_CODE_NEVER_DIRTY_PS_BEGIN = 0x20,
    CONST_SRC_CODE_SHADOWMAP_SWITCH_PARTITION = 0x20,
    CONST_SRC_CODE_SHADOWMAP_SCALE = 0x21,
    CONST_SRC_CODE_ZNEAR = 0x22,
    CONST_SRC_CODE_SUN_POSITION = 0x23,
    CONST_SRC_CODE_SUN_DIFFUSE = 0x24,
    CONST_SRC_CODE_SUN_SPECULAR = 0x25,
    CONST_SRC_CODE_LIGHTING_LOOKUP_SCALE = 0x26,
    CONST_SRC_CODE_DEBUG_BUMPMAP = 0x27,
    CONST_SRC_CODE_MATERIAL_COLOR = 0x28,
    CONST_SRC_CODE_FOG = 0x29,
    CONST_SRC_CODE_FOG_COLOR = 0x2A,
    CONST_SRC_CODE_GLOW_SETUP = 0x2B,
    CONST_SRC_CODE_GLOW_APPLY = 0x2C,
    CONST_SRC_CODE_COLOR_BIAS = 0x2D,
    CONST_SRC_CODE_COLOR_TINT_BASE = 0x2E,
    CONST_SRC_CODE_COLOR_TINT_DELTA = 0x2F,
    CONST_SRC_CODE_OUTDOOR_FEATHER_PARMS = 0x30,
    CONST_SRC_CODE_ENVMAP_PARMS = 0x31,
    CONST_SRC_CODE_SPOT_SHADOWMAP_PIXEL_ADJUST = 0x32,
    CONST_SRC_CODE_CLIP_SPACE_LOOKUP_SCALE = 0x33,
    CONST_SRC_CODE_CLIP_SPACE_LOOKUP_OFFSET = 0x34,
    CONST_SRC_CODE_PARTICLE_CLOUD_MATRIX = 0x35,
    CONST_SRC_CODE_DEPTH_FROM_CLIP = 0x36,
    CONST_SRC_CODE_CODE_MESH_ARG_0 = 0x37,
    CONST_SRC_CODE_CODE_MESH_ARG_1 = 0x38,
    CONST_SRC_CODE_CODE_MESH_ARG_LAST = 0x38,
    CONST_SRC_CODE_BASE_LIGHTING_COORDS = 0x39,
    CONST_SRC_CODE_NEVER_DIRTY_PS_END = 0x3A,
    CONST_SRC_CODE_COUNT_FLOAT4 = 0x3A,
    CONST_SRC_FIRST_CODE_MATRIX = 0x3A,
    CONST_SRC_CODE_WORLD_MATRIX = 0x3A,
    CONST_SRC_CODE_INVERSE_WORLD_MATRIX = 0x3B,
    CONST_SRC_CODE_TRANSPOSE_WORLD_MATRIX = 0x3C,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_MATRIX = 0x3D,
    CONST_SRC_CODE_VIEW_MATRIX = 0x3E,
    CONST_SRC_CODE_INVERSE_VIEW_MATRIX = 0x3F,
    CONST_SRC_CODE_TRANSPOSE_VIEW_MATRIX = 0x40,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_VIEW_MATRIX = 0x41,
    CONST_SRC_CODE_PROJECTION_MATRIX = 0x42,
    CONST_SRC_CODE_INVERSE_PROJECTION_MATRIX = 0x43,
    CONST_SRC_CODE_TRANSPOSE_PROJECTION_MATRIX = 0x44,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_PROJECTION_MATRIX = 0x45,
    CONST_SRC_CODE_WORLD_VIEW_MATRIX = 0x46,
    CONST_SRC_CODE_INVERSE_WORLD_VIEW_MATRIX = 0x47,
    CONST_SRC_CODE_TRANSPOSE_WORLD_VIEW_MATRIX = 0x48,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX = 0x49,
    CONST_SRC_CODE_VIEW_PROJECTION_MATRIX = 0x4A,
    CONST_SRC_CODE_INVERSE_VIEW_PROJECTION_MATRIX = 0x4B,
    CONST_SRC_CODE_TRANSPOSE_VIEW_PROJECTION_MATRIX = 0x4C,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_VIEW_PROJECTION_MATRIX = 0x4D,
    CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX = 0x4E,
    CONST_SRC_CODE_INVERSE_WORLD_VIEW_PROJECTION_MATRIX = 0x4F,
    CONST_SRC_CODE_TRANSPOSE_WORLD_VIEW_PROJECTION_MATRIX = 0x50,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_VIEW_PROJECTION_MATRIX = 0x51,
    CONST_SRC_CODE_SHADOW_LOOKUP_MATRIX = 0x52,
    CONST_SRC_CODE_INVERSE_SHADOW_LOOKUP_MATRIX = 0x53,
    CONST_SRC_CODE_TRANSPOSE_SHADOW_LOOKUP_MATRIX = 0x54,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_SHADOW_LOOKUP_MATRIX = 0x55,
    CONST_SRC_CODE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x56,
    CONST_SRC_CODE_INVERSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x57,
    CONST_SRC_CODE_TRANSPOSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x58,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x59,
    CONST_SRC_TOTAL_COUNT = 0x5A,
    CONST_SRC_NONE = 0x5B,
};

enum $091E6234341635363F651E71B7EC01B7 : __int32
{
    STREAM_SRC_POSITION = 0x0,
    STREAM_SRC_COLOR = 0x1,
    STREAM_SRC_TEXCOORD_0 = 0x2,
    STREAM_SRC_NORMAL = 0x3,
    STREAM_SRC_TANGENT = 0x4,
    STREAM_SRC_OPTIONAL_BEGIN = 0x5,
    STREAM_SRC_PRE_OPTIONAL_BEGIN = 0x4,
    STREAM_SRC_TEXCOORD_1 = 0x5,
    STREAM_SRC_TEXCOORD_2 = 0x6,
    STREAM_SRC_NORMAL_TRANSFORM_0 = 0x7,
    STREAM_SRC_NORMAL_TRANSFORM_1 = 0x8,
    STREAM_SRC_COUNT = 0x9,
};

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
#if 0
// bad sp value at call has been detected, the output may be wrong!
void  R_DrawCall(
    void(__cdecl* callback)(const void*, GfxCmdBufSourceState*, GfxCmdBufState*, GfxCmdBufSourceState*, GfxCmdBufState*),
    const void *userData,
    GfxCmdBufSourceState *source,
    const GfxViewInfo *viewInfo,
    const GfxDrawSurfListInfo *info,
    const GfxViewParms *viewParms,
    GfxCmdBuf *cmdBuf,
    GfxCmdBuf *prepassCmdBuf);
#else
typedef void (__cdecl*DrawCallCallback)(const void*, GfxCmdBufContext, GfxCmdBufContext);
void R_DrawCall(
    DrawCallCallback callback,
    const void* userData,
    GfxCmdBufSourceState* source,
    const GfxViewInfo* viewInfo,
    const GfxDrawSurfListInfo* info,
    const GfxViewParms* viewParms,
    GfxCmdBuf* cmdBufEA,
    GfxCmdBuf* prepassCmdBufEA);
#endif
void __cdecl R_SetCodeConstant(GfxCmdBufSourceState *source, unsigned int constant, float x, float y, float z, float w);
void __cdecl R_SetAlphaAntiAliasingState(IDirect3DDevice9 *device, __int16 stateBits0);
