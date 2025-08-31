#include "rb_spotshadow.h"
#include "rb_shade.h"
#include "rb_state.h"
#include "rb_backend.h"
#include "r_rendercmds.h"
#include "rb_postfx.h"
#include "rb_sunshadow.h"

void __cdecl RB_SetSpotShadowOverlayScaleAndBias(const GfxSpotShadow *spotShadow)
{
    float nearDepth; // [esp+20h] [ebp-1Ch] BYREF
    MaterialTechniqueType shadowmapBuildTechType; // [esp+24h] [ebp-18h]
    float zNear; // [esp+28h] [ebp-14h]
    float bias; // [esp+2Ch] [ebp-10h]
    float zFar; // [esp+30h] [ebp-Ch]
    float scale; // [esp+34h] [ebp-8h]
    float farDepth; // [esp+38h] [ebp-4h] BYREF

    RB_GetShadowOverlayDepthBounds(&nearDepth, &farDepth);
    zNear = -spotShadow->shadowViewParms.projectionMatrix.m[3][2] / spotShadow->shadowViewParms.projectionMatrix.m[2][2];
    zFar = spotShadow->light->radius;
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    if (gfxMetrics.shadowmapBuildTechType == TECHNIQUE_BUILD_SHADOWMAP_COLOR)
    {
        scale = 1.0f / ((zFar - zNear) * (farDepth - nearDepth));
        bias = -scale * ((zFar - zNear) * nearDepth + zNear);
        R_UpdateCodeConstant(&gfxCmdBufSourceState, 0x15u, scale, bias, 1.0f, 1.0f);
    }
    else
    {
        scale = 1.0f / (farDepth - nearDepth);
        bias = -scale * nearDepth;
        R_UpdateCodeConstant(&gfxCmdBufSourceState, 0x15u, scale, bias, zNear, zFar);
    }
}

void __cdecl RB_DrawSpotShadowOverlay()
{
    float x; // [esp+28h] [ebp-40h]
    float t0; // [esp+44h] [ebp-24h]
    float t1; // [esp+50h] [ebp-18h]
    GfxViewInfo *viewInfo; // [esp+54h] [ebp-14h]
    unsigned int spotShadowIndex; // [esp+5Ch] [ebp-Ch]
    float h; // [esp+60h] [ebp-8h]

    iassert( backEndData->viewInfoCount > 0 );
    viewInfo = backEndData->viewInfo;
    if (viewInfo->spotShadowCount)
    {
        h = vidConfig.displayHeight * 0.25;
        gfxCmdBufSourceState.input.codeImageSamplerStates[9] = 97;
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, gfxRenderTargets[14].image);
        for (spotShadowIndex = 0; spotShadowIndex < viewInfo->spotShadowCount; ++spotShadowIndex)
        {
            t0 = spotShadowIndex * 0.25;
            t1 = t0 + 0.25;
            RB_SetSpotShadowOverlayScaleAndBias(&viewInfo->spotShadows[spotShadowIndex]);
            x = spotShadowIndex * (h + 2.0) + 4.0;
            RB_DrawStretchPic(rgp.shadowOverlayMaterial, x, 4.0, h, h, 0.0, t0, 1.0, t1, 0xFFFFFFFF, GFX_PRIM_STATS_HUD);
            RB_EndTessSurface();
        }
        gfxCmdBufSourceState.input.codeImageSamplerStates[9] = 98;
    }
}