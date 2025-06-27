#pragma once
#include "rb_backend.h"

#define GFX_MAX_EMISSIVE_SPOT_LIGHTS 1

enum LightHasShadowMap : __int32
{                                       // ...
    LIGHT_HAS_SHADOWMAP = 0x0,
    LIGHT_HAS_NO_SHADOWMAP = 0x1,
};

void __cdecl R_SetLightProperties(
    GfxCmdBufSourceState *source,
    const GfxLight *light,
    const GfxLightDef *def,
    LightHasShadowMap hasShadowMap,
    float spotShadowFade);
void __cdecl R_SetCodeImageSamplerState(
    GfxCmdBufSourceState *source,
    unsigned int codeTexture,
    unsigned __int8 samplerState);
void __cdecl R_SetShadowableLight(
    GfxCmdBufSourceState *source,
    unsigned int shadowableLightIndex,
    const GfxViewInfo *viewInfo);
void __cdecl R_SetDrawSurfsShadowableLight(GfxCmdBufSourceState *source, const GfxDrawSurfListInfo *info);
unsigned int __cdecl R_GetShadowableLightIndex(
    const GfxBackEndData *data,
    const GfxViewInfo *viewInfo,
    const GfxLight *light);
