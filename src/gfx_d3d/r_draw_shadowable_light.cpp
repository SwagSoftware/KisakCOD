#include "r_draw_shadowable_light.h"
#include "r_dvars.h"
#include "r_utils.h"
#include "rb_postfx.h"
#include <devgui/devgui.h>
#include <qcommon/com_bsp.h>



void __cdecl R_SetLightProperties(
    GfxCmdBufSourceState *source,
    const GfxLight *light,
    const GfxLightDef *def,
    LightHasShadowMap hasShadowMap,
    float spotShadowFade)
{
    float v5; // [esp+14h] [ebp-70h]
    float v6; // [esp+18h] [ebp-6Ch]
    float v7; // [esp+24h] [ebp-60h]
    float v8; // [esp+28h] [ebp-5Ch]
    float v9; // [esp+34h] [ebp-50h]
    float v10; // [esp+38h] [ebp-4Ch]
    float v11; // [esp+44h] [ebp-40h]
    float v12; // [esp+48h] [ebp-3Ch]
    float v13; // [esp+4Ch] [ebp-38h]
    float spotDotScale; // [esp+54h] [ebp-30h]
    float lightOrigin[3]; // [esp+58h] [ebp-2Ch] BYREF
    float diffuseColor[3]; // [esp+64h] [ebp-20h] BYREF
    float spotExponent; // [esp+70h] [ebp-14h]
    float specularColor[3]; // [esp+74h] [ebp-10h] BYREF
    float spotDotBias; // [esp+80h] [ebp-4h]

    if (source->viewMode != VIEW_MODE_3D)
        MyAssertHandler(
            ".\\r_draw_shadowablelight.cpp",
            55,
            0,
            "%s\n\t(source->viewMode) = %i",
            "(source->viewMode == VIEW_MODE_3D)",
            source->viewMode);
    if (!light)
        MyAssertHandler(".\\r_draw_shadowablelight.cpp", 56, 0, "%s", "light");
    if (light->type != 3 && light->type != 2)
        MyAssertHandler(
            ".\\r_draw_shadowablelight.cpp",
            57,
            0,
            "%s",
            "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
    if (light->radius <= 0.0)
        MyAssertHandler(
            ".\\r_draw_shadowablelight.cpp",
            58,
            0,
            "%s\n\t(light->radius) = %g",
            "(light->radius > 0.0f)",
            light->radius);
    R_SetCodeImageTexture(source, 0xFu, def->attenuation.image);
    R_SetCodeImageSamplerState(source, 0xFu, def->attenuation.samplerState);
    Vec3Sub(light->origin, source->eyeOffset, lightOrigin);
    Vec3Scale(light->color, r_diffuseColorScale->current.value, diffuseColor);
    Vec3Scale(light->color, r_specularColorScale->current.value, specularColor);
    v11 = lightOrigin[1];
    v12 = lightOrigin[2];
    v13 = 1.0 / light->radius;
    source->input.consts[0][0] = lightOrigin[0];
    source->input.consts[0][1] = v11;
    source->input.consts[0][2] = v12;
    source->input.consts[0][3] = v13;
    R_DirtyCodeConstant(source, 0);
    v9 = diffuseColor[1];
    v10 = diffuseColor[2];
    source->input.consts[1][0] = diffuseColor[0];
    source->input.consts[1][1] = v9;
    source->input.consts[1][2] = v10;
    source->input.consts[1][3] = 1.0;
    R_DirtyCodeConstant(source, 1u);
    v7 = specularColor[1];
    v8 = specularColor[2];
    source->input.consts[2][0] = specularColor[0];
    source->input.consts[2][1] = v7;
    source->input.consts[2][2] = v8;
    source->input.consts[2][3] = 1.0;
    R_DirtyCodeConstant(source, 2u);
    v5 = light->dir[1];
    v6 = light->dir[2];
    source->input.consts[3][0] = light->dir[0];
    source->input.consts[3][1] = v5;
    source->input.consts[3][2] = v6;
    source->input.consts[3][3] = 0.0;
    R_DirtyCodeConstant(source, 3u);
    if (light->type == 2 || hasShadowMap == LIGHT_HAS_SHADOWMAP)
    {
        if (light->cosHalfFovOuter >= (double)light->cosHalfFovInner)
            MyAssertHandler(
                ".\\r_draw_shadowablelight.cpp",
                80,
                0,
                "light->cosHalfFovInner > light->cosHalfFovOuter\n\t%g, %g",
                light->cosHalfFovInner,
                light->cosHalfFovOuter);
        spotDotScale = 1.0 / (light->cosHalfFovInner - light->cosHalfFovOuter);
        spotDotBias = -spotDotScale * light->cosHalfFovOuter;
        spotExponent = (float)light->exponent;
        Vec4Set(source->input.consts[4], spotDotScale, spotDotBias, spotExponent, spotShadowFade);
        R_DirtyCodeConstant(source, 4u);
    }
}

void __cdecl R_SetCodeImageSamplerState(
    GfxCmdBufSourceState *source,
    unsigned int codeTexture,
    unsigned __int8 samplerState)
{
    const char *v3; // eax

    if (codeTexture >= 0x1B)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            564,
            0,
            "codeTexture doesn't index TEXTURE_SRC_CODE_COUNT\n\t%i not in [0, %i)",
            codeTexture,
            27);
    if ((samplerState & 7) == 0)
    {
        v3 = va("R_SetCodeImageSamplerState %d %d", codeTexture, samplerState);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            565,
            0,
            "%s\n\t%s",
            "samplerState & SAMPLER_FILTER_MASK",
            v3);
    }
    source->input.codeImageSamplerStates[codeTexture] = samplerState;
}

void __cdecl R_SetShadowableLight(
    GfxCmdBufSourceState *source,
    unsigned int shadowableLightIndex,
    const GfxViewInfo *viewInfo)
{
    float falloffScale; // [esp+28h] [ebp-28h]
    float spotShadowFade; // [esp+30h] [ebp-20h]
    GfxLightDef *def; // [esp+34h] [ebp-1Ch]
    float falloffShift; // [esp+3Ch] [ebp-14h]
    const GfxSpotShadow *spotShadow; // [esp+44h] [ebp-Ch]
    const GfxMatrix *lookupMatrix; // [esp+48h] [ebp-8h]
    LightHasShadowMap hasShadowMap; // [esp+4Ch] [ebp-4h]

    if (shadowableLightIndex >= 0xFF)
        MyAssertHandler(
            ".\\r_draw_shadowablelight.cpp",
            102,
            0,
            "shadowableLightIndex doesn't index GFX_MAX_PRIMARY_LIGHTS\n\t%i not in [0, %i)",
            shadowableLightIndex,
            255);
    if (source->shadowableLightIndex != shadowableLightIndex)
    {
        source->shadowableLightIndex = shadowableLightIndex;
        if (shadowableLightIndex)
        {
            if (shadowableLightIndex >= viewInfo->shadowableLightCount)
                MyAssertHandler(
                    ".\\r_draw_shadowablelight.cpp",
                    112,
                    0,
                    "shadowableLightIndex doesn't index viewInfo->shadowableLightCount\n\t%i not in [0, %i)",
                    shadowableLightIndex,
                    viewInfo->shadowableLightCount);
            if (viewInfo->shadowableLights[shadowableLightIndex].type == 1)
            {
                if (source->shadowableLightForShadowLookupMatrix != shadowableLightIndex)
                {
                    source->shadowableLightForShadowLookupMatrix = shadowableLightIndex;
                    R_SetShadowLookupMatrix(source, &viewInfo->sunShadow.lookupMatrix);
                }
            }
            else
            {
                def = viewInfo->shadowableLights[shadowableLightIndex].def;
                falloffShift = (double)def->lmapLookupStart * 0.001953125;
                falloffScale = (double)def->attenuation.image->width * 0.001953125;
                R_UpdateCodeConstant(source, 0xBu, falloffScale, 0.0, falloffShift, 0.0);
                hasShadowMap = LIGHT_HAS_NO_SHADOWMAP;
                spotShadowFade = 0.0;
                if (Com_BitCheckAssert(source->input.data->shadowableLightHasShadowMap, shadowableLightIndex, 32))
                {
                    if (viewInfo->shadowableLights[shadowableLightIndex].spotShadowIndex >= 4)
                        MyAssertHandler(
                            ".\\r_draw_shadowablelight.cpp",
                            137,
                            0,
                            "light->spotShadowIndex doesn't index R_SPOTSHADOW_TILE_COUNT\n\t%i not in [0, %i)",
                            viewInfo->shadowableLights[shadowableLightIndex].spotShadowIndex,
                            4);
                    spotShadow = &viewInfo->spotShadows[viewInfo->shadowableLights[shadowableLightIndex].spotShadowIndex];
                    lookupMatrix = &viewInfo->spotShadows[viewInfo->shadowableLights[shadowableLightIndex].spotShadowIndex].lookupMatrix;
                    if (source->shadowableLightForShadowLookupMatrix != shadowableLightIndex)
                    {
                        source->shadowableLightForShadowLookupMatrix = shadowableLightIndex;
                        R_SetShadowLookupMatrix(source, lookupMatrix);
                    }
                    hasShadowMap = LIGHT_HAS_SHADOWMAP;
                    spotShadowFade = spotShadow->fade;
                    R_SetCodeImageTexture(source, 8u, spotShadow->image);
                    if (!Vec4Compare(source->input.consts[50], spotShadow->pixelAdjust))
                        R_SetCodeConstantFromVec4(source, 0x32u, (float*)spotShadow->pixelAdjust);
                }
                R_SetLightProperties(
                    source,
                    &viewInfo->shadowableLights[shadowableLightIndex],
                    def,
                    hasShadowMap,
                    spotShadowFade);
            }
        }
    }
}

void __cdecl R_SetDrawSurfsShadowableLight(GfxCmdBufSourceState *source, const GfxDrawSurfListInfo *info)
{
    unsigned int shadowableLightIndex; // [esp+4h] [ebp-14h]
    const GfxLight *light; // [esp+14h] [ebp-4h]

    if (info->light)
    {
        light = info->light;
        shadowableLightIndex = R_GetShadowableLightIndex(source->input.data, info->viewInfo, light);
        if (shadowableLightIndex)
            R_SetShadowableLight(source, shadowableLightIndex, info->viewInfo);
        else
            R_SetLightProperties(source, light, light->def, LIGHT_HAS_NO_SHADOWMAP, 0.0);
    }
}

unsigned int __cdecl R_GetShadowableLightIndex(
    const GfxBackEndData *data,
    const GfxViewInfo *viewInfo,
    const GfxLight *light)
{
    iassert(light);

    if (!light->canUseShadowMap)
        return 0;

    iassert(comWorld.isInUse);

    if (!Com_BitCheckAssert(data->shadowableLightHasShadowMap, comWorld.primaryLightCount, 32))
        return 0;

    iassert(comWorld.isInUse);
    iassert(viewInfo->shadowableLightCount == Com_GetPrimaryLightCount() + GFX_MAX_EMISSIVE_SPOT_LIGHTS);
    iassert(comWorld.isInUse);

    return comWorld.primaryLightCount;
}

