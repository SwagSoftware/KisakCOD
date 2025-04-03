#pragma once
#include "rb_backend.h"

struct GfxShaderConstantState_s // sizeof=0x8
{                                       // ...
    MaterialArgumentCodeConst codeConst;
    unsigned int version;
};
union GfxShaderConstantState // sizeof=0x8
{                                       // ...
    GfxShaderConstantState_s fields;
    unsigned __int64 packed;
};

void __cdecl R_SetupPassPerPrimArgs(GfxCmdBufContext context);
void __cdecl R_SetVertexShaderConstantFromCode(GfxCmdBufContext context, const MaterialShaderArgument *routingData);
void __cdecl R_HW_SetVertexShaderConstant(__int64 device, __int64 data);
GfxCmdBufInput *__cdecl R_GetCodeConstant(GfxCmdBufContext context, unsigned int constant);
char __cdecl R_IsVertexShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData);
char __cdecl R_IsShaderMatrixUpToDate(
    const GfxCmdBufSourceState *source,
    GfxShaderConstantState *constant,
    const MaterialShaderArgument *routingData);
char __cdecl R_IsShaderConstantUpToDate(
    const GfxCmdBufSourceState *source,
    GfxShaderConstantState *constant,
    const MaterialShaderArgument *routingData);
void __cdecl R_SetupPassPerObjectArgs(GfxCmdBufContext context);
void __cdecl R_SetPixelShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal);
void __cdecl R_HW_SetPixelShaderConstant(
    IDirect3DDevice9 *device,
    unsigned int dest,
    const float *data,
    unsigned int rowCount);
int __cdecl R_IsPixelShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData);
void __cdecl R_SetPixelShaderConstantFromCode(GfxCmdBufContext context, const MaterialShaderArgument *routingData);
void __cdecl R_SetupPassCriticalPixelShaderArgs(GfxCmdBufContext context);
void __cdecl R_SetupPassVertexShaderArgs(GfxCmdBufContext context);
void __cdecl R_OverrideImage(GfxImage **image, const MaterialTextureDef *texdef);
void __cdecl R_SetPixelShader(GfxCmdBufState *state, const MaterialPixelShader *pixelShader);
void __cdecl R_SetVertexShader(GfxCmdBufState *state, const MaterialVertexShader *vertexShader);
void __cdecl R_UpdateVertexDecl(GfxCmdBufState *state);
void __cdecl R_SetupPass(GfxCmdBufContext context, unsigned int passIndex);
void __cdecl R_SetState(GfxCmdBufState *state, unsigned int *stateBits);
void __cdecl R_SetVertexShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal);
const MaterialTextureDef *__cdecl R_SetPixelSamplerFromMaterial(
    GfxCmdBufContext context,
    const MaterialShaderArgument *arg,
    const MaterialTextureDef *texDef);

void __cdecl R_SetPassShaderPrimArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
    const MaterialShaderArgument *arg);
void __cdecl R_SetPassShaderObjectArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
    const MaterialShaderArgument *arg);
void __cdecl R_SetPassPixelShaderStableArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
    const MaterialShaderArgument *arg);
void __cdecl R_SetPassVertexShaderStableArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
    const MaterialShaderArgument *arg);
void __cdecl R_SetPassShaderStableArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
    const MaterialShaderArgument *arg);


// r_draw_pixelshader
unsigned int __cdecl R_SkipDrawSurfListMaterial(const GfxDrawSurf *drawSurfList, unsigned int drawSurfCount);