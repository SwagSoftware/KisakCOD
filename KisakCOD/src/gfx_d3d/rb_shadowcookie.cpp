#include "rb_shadowcookie.h"
#include "r_rendercmds.h"
#include <universal/profile.h>
#include "r_utils.h"
#include "rb_state.h"
#include "r_state.h"
#include "r_dvars.h"
#include "rb_shade.h"
#include <cgame/cg_local.h>

void __cdecl RB_ClearShadowCookieBufferIfNeeded(int firstTime, const float *cookieClearColor)
{
    R_ClearScreen(gfxCmdBufState.prim.device, 7u, cookieClearColor, 1.0, 0, 0);
}

void __cdecl RB_SetShadowCookie(const GfxMatrix *lookupMatrix, float fade)
{
    R_SetShadowLookupMatrix(&gfxCmdBufSourceState, lookupMatrix);
    gfxCmdBufSourceState.input.consts[8][0] = 0.0;
    gfxCmdBufSourceState.input.consts[8][1] = 0.0;
    gfxCmdBufSourceState.input.consts[8][2] = 0.0;
    gfxCmdBufSourceState.input.consts[8][3] = fade;
    R_DirtyCodeConstant(&gfxCmdBufSourceState, 8u);
}

void __cdecl RB_BlurShadowPass(float st0, float st1)
{
    float w; // [esp+28h] [ebp-14h]
    float h; // [esp+2Ch] [ebp-10h]
    float screenWidth; // [esp+30h] [ebp-Ch]
    const GfxImage *workingImage; // [esp+34h] [ebp-8h]
    float screenHeight; // [esp+38h] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    screenWidth = gfxCmdBufSourceState.renderTargetWidth;
    screenHeight = gfxCmdBufSourceState.renderTargetHeight;
    workingImage = gfxRenderTargets[10].image;
    R_Resolve(gfxCmdBufContext, gfxRenderTargets[10].image);
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, workingImage);
    R_Set2D(&gfxCmdBufSourceState);
    h = screenHeight - 2.0;
    w = screenWidth - 2.0;
    RB_DrawStretchPic(rgp.shadowCookieBlurMaterial, 1.0, 1.0, w, h, st0, st0, st1, st1, 0xFFFFFFFF, GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
}

int RB_BlurShadowCookie()
{
    int result; // eax
    float v1; // [esp+8h] [ebp-1Ch]
    float v2; // [esp+Ch] [ebp-18h]
    float v3; // [esp+10h] [ebp-14h]
    float v4; // [esp+14h] [ebp-10h]
    float shadowHalfTexel; // [esp+18h] [ebp-Ch]
    float shadowBorderTexel; // [esp+1Ch] [ebp-8h]
    int blurIter; // [esp+20h] [ebp-4h]

    shadowHalfTexel = 0.00390625;
    shadowBorderTexel = 0.0078125;
    for (blurIter = 0; blurIter < sc_blur->current.integer; ++blurIter)
    {
        v4 = 1.0 - shadowBorderTexel + shadowHalfTexel;
        v3 = shadowBorderTexel + shadowHalfTexel;
        RB_BlurShadowPass(v3, v4);
        v2 = 1.0 - shadowBorderTexel - shadowHalfTexel;
        v1 = shadowBorderTexel - shadowHalfTexel;
        RB_BlurShadowPass(v1, v2);
        result = blurIter + 1;
    }
    return result;
}

void __cdecl RB_DrawShadowCookies(const GfxViewInfo *viewInfo)
{
    float cookieClearColor[4]; // [esp+44h] [ebp-28h] BYREF
    int needToClearDynamicShadows; // [esp+54h] [ebp-18h]
    const ShadowCookie *cookie; // [esp+58h] [ebp-14h]
    const ShadowCookieList *cookieList; // [esp+60h] [ebp-Ch]
    unsigned int cookieIter; // [esp+64h] [ebp-8h]
    int receiverSurfCount; // [esp+68h] [ebp-4h]

    Profile_Begin(154);
    CL_ResetStats_f();
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
    qmemcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    memset(gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
    memset(gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
    needToClearDynamicShadows = 1;
    cookieList = &viewInfo->shadowCookieList;
    cookieClearColor[0] = 0.5;
    cookieClearColor[1] = 0.5;
    cookieClearColor[2] = 0.5;
    cookieClearColor[3] = 0.0;
    for (cookieIter = 0; cookieIter != cookieList->cookieCount; ++cookieIter)
    {
        cookie = &cookieList->cookies[cookieIter];
        receiverSurfCount = cookie->receiverInfo.drawSurfCount;
        if (cookie->casterInfo.drawSurfCount)
        {
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SHADOWCOOKIE);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SHADOWCOOKIE);
            RB_ClearShadowCookieBufferIfNeeded(needToClearDynamicShadows, cookieClearColor);
            R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, cookie->shadowViewParms);
            R_SetViewportValues(&gfxCmdBufSourceState, 1, 1, 126, 126);
            RB_SetShadowCookie(&cookie->shadowLookupMatrix, cookie->fade);
            R_DrawSurfs(gfxCmdBufContext, 0, &cookie->casterInfo);
            R_SetViewportValues(&gfxCmdBufSourceState, 0, 0, 128, 128);
            if (sc_blur->current.integer)
                RB_BlurShadowCookie();
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_DYNAMICSHADOWS);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_DYNAMICSHADOWS);
            if (needToClearDynamicShadows)
            {
                needToClearDynamicShadows = 0;
                if (!viewInfo->needsFloatZ)
                    R_ClearScreen(gfxCmdBufState.prim.device, 1u, colorWhite, 0.0, 0, 0);
            }
            R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->viewParms);
            R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->sceneViewport);
            if (receiverSurfCount)
                R_DrawSurfs(gfxCmdBufContext, 0, &cookie->receiverInfo);
        }
    }
    if (needToClearDynamicShadows)
    {
        needToClearDynamicShadows = 0;
        R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_DYNAMICSHADOWS);
        R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_DYNAMICSHADOWS);
        if (!viewInfo->needsFloatZ)
            R_ClearScreen(gfxCmdBufState.prim.device, 1u, colorWhite, 0.0, 0, 0);
    }
    qmemcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
    Profile_EndInternal(0);
}

void RB_ShadowCookieOverlay()
{
    unsigned int v1; // [esp+2Ch] [ebp-68h] BYREF
    float v2; // [esp+30h] [ebp-64h]
    float v3; // [esp+34h] [ebp-60h]
    __int64 color; // [esp+38h] [ebp-5Ch]
    float w; // [esp+40h] [ebp-54h]
    float x; // [esp+44h] [ebp-50h]
    GfxMatrix v7; // [esp+48h] [ebp-4Ch] BYREF
    int identityMatrix_52; // [esp+88h] [ebp-Ch]
    void *identityMatrix_56; // [esp+8Ch] [ebp-8h]
    void *retaddr; // [esp+94h] [ebp+0h]

    MatrixIdentity44(v7.m);
    x = 4.0;
    w = 4.0;
    color = vidConfig.displayWidth;
    v3 = vidConfig.displayWidth * 0.25;
    v2 = v3;
    RB_SetShadowCookie(&v7, 1.0);
    R_ConvertColorToBytes(colorWhite, &v1);
    RB_DrawStretchPic(rgp.shadowCookieOverlayMaterial, x, w, v3, v2, 0.0, 0.0, 1.0, 1.0, v1, GFX_PRIM_STATS_HUD);
    RB_EndTessSurface();
}