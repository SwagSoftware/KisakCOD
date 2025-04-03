#include "rb_postfx.h"
#include "r_dvars.h"
#include "rb_state.h"
#include "r_image.h"
#include "r_state.h"
#include "rb_imagefilter.h"


bool __cdecl R_UsingGlow(const GfxViewInfo *viewInfo)
{
    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 14, 0, "%s", "viewInfo");
    return (r_glow_allowed->current.enabled || r_glow_allowed_script_forced->current.enabled)
        && viewInfo->glow.enabled
        && !r_fullbright->current.enabled
        && r_glow->current.enabled
        && viewInfo->glow.bloomIntensity != 0.0
        && viewInfo->glow.radius != 0.0;
}

bool __cdecl R_UsingDepthOfField(const GfxViewInfo *viewInfo)
{
    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 21, 0, "%s", "viewInfo");
    if (viewInfo->dof.viewModelEnd > viewInfo->dof.viewModelStart + 1.0)
        return 1;
    if (viewInfo->dof.nearEnd > viewInfo->dof.nearStart + 1.0)
        return 1;
    return viewInfo->dof.farEnd > viewInfo->dof.farStart + 1.0 && viewInfo->dof.farBlur > 0.0;
}

bool __cdecl RB_UsingColorManipulation(const GfxViewInfo *viewInfo)
{
    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 36, 0, "%s", "viewInfo");
    if (!viewInfo->film.enabled)
        return 0;
    if (viewInfo->film.contrast != 1.0)
        return 1;
    if (viewInfo->film.brightness != 0.0)
        return 1;
    if (viewInfo->film.desaturation != 0.0)
        return 1;
    if (viewInfo->film.invert)
        return 1;
    if (viewInfo->film.tintDark[0] != 1.0 || viewInfo->film.tintDark[1] != 1.0 || viewInfo->film.tintDark[2] != 1.0)
        return 1;
    return viewInfo->film.tintLight[0] != 1.0 || viewInfo->film.tintLight[1] != 1.0 || viewInfo->film.tintLight[2] != 1.0;
}

void __cdecl RB_ApplyColorManipulationFullscreen(const GfxViewInfo *viewInfo)
{
    if (!RB_UsingColorManipulation(viewInfo))
        MyAssertHandler(".\\rb_postfx.cpp", 235, 0, "%s", "RB_UsingColorManipulation( viewInfo )");
    if (!viewInfo->isRenderingFullScreen)
        MyAssertHandler(".\\rb_postfx.cpp", 236, 0, "%s", "viewInfo->isRenderingFullScreen");
    RB_FullScreenFilter(rgp.postFxColorMaterial);
}

void __cdecl RB_ApplyColorManipulationSplitscreen(const GfxViewInfo *viewInfo)
{
    if (!RB_UsingColorManipulation(viewInfo))
        MyAssertHandler(".\\rb_postfx.cpp", 244, 0, "%s", "RB_UsingColorManipulation( viewInfo )");
    if (viewInfo->isRenderingFullScreen)
        MyAssertHandler(".\\rb_postfx.cpp", 245, 0, "%s", "!viewInfo->isRenderingFullScreen");
    RB_GetResolvedScene();
    RB_SplitScreenFilter(rgp.postFxColorMaterial, viewInfo);
}

void RB_GetResolvedScene()
{
    if (!gfxCmdBufSourceState.input.codeImages[11])
    {
        if (gfxCmdBufState.renderTargetId != R_RENDERTARGET_SCENE)
            MyAssertHandler(
                ".\\rb_postfx.cpp",
                225,
                0,
                "gfxCmdBufState.renderTargetId == R_RENDERTARGET_SCENE\n\t%i, %i",
                gfxCmdBufState.renderTargetId,
                2);
        R_Resolve(gfxCmdBufContext, image.image);
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 0xBu, image.image);
    }
}

void __cdecl RB_GetDepthOfFieldInputImages(float radius)
{
    float v1; // [esp+Ch] [ebp-8h]

    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_POST_EFFECT_1);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_POST_EFFECT_1);
    RB_FullScreenFilter(rgp.dofDownsampleMaterial);
    v1 = radius * 0.25;
    RB_GaussianFilterImage(v1, R_RENDERTARGET_POST_EFFECT_1, R_RENDERTARGET_POST_EFFECT_0);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_PINGPONG_0);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_PINGPONG_0);
    RB_FullScreenFilter(rgp.dofNearCocMaterial);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_POST_EFFECT_1);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_POST_EFFECT_1);
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, stru_EA74F7C.image);
    RB_FullScreenFilter(rgp.smallBlurMaterial);
}

void __cdecl RB_ProcessPostEffects(const GfxViewInfo *viewInfo)
{
    float blurRadius; // [esp+38h] [ebp-4h]

    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 403, 0, "%s", "viewInfo");
    if (RB_UsingPostEffects(viewInfo))
    {
        //Profile_Begin(115);
        RB_GetResolvedScene();
        if (RB_UsingMergedPostEffects(viewInfo))
            RB_ApplyMergedPostEffects(viewInfo);
        if (R_UsingGlow(viewInfo))
        {
            RB_CalcGlowEffect(viewInfo);
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
            RB_ApplyGlowEffect(viewInfo);
        }
        if (RB_UsingBlur(viewInfo->blurRadius))
        {
            blurRadius = RB_GetBlurRadius(viewInfo->blurRadius);
            if (blurRadius <= 0.0)
                MyAssertHandler(".\\rb_postfx.cpp", 426, 1, "%s\n\t(blurRadius) = %g", "(blurRadius > 0.0f)", blurRadius);
            RB_BlurScreen(viewInfo, blurRadius);
        }
        //Profile_EndInternal(0);
    }
}

bool __cdecl RB_UsingMergedPostEffects(const GfxViewInfo *viewInfo)
{
    return R_UsingDepthOfField(viewInfo) || RB_UsingColorManipulation(viewInfo);
}

bool __cdecl RB_UsingBlur(float blurRadius)
{
    return r_blur->current.value > 0.0 || blurRadius > 0.0;
}

bool __cdecl RB_UsingPostEffects(const GfxViewInfo *viewInfo)
{
    if (R_UsingGlow(viewInfo))
        return 1;
    if (r_showFbColorDebug->current.integer == 2)
        return 1;
    if (RB_UsingBlur(viewInfo->blurRadius))
        return 1;
    return RB_UsingMergedPostEffects(viewInfo);
}

void __cdecl RB_CalcGlowEffect(const GfxViewInfo *viewInfo)
{
    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 122, 0, "%s", "viewInfo");
    if (viewInfo->glow.bloomIntensity == 0.0)
        MyAssertHandler(".\\rb_postfx.cpp", 123, 0, "%s", "viewInfo->glow.bloomIntensity");
    RB_GlowFilterImage(viewInfo->glow.radius);
}

void __cdecl RB_ApplyGlowEffect(const GfxViewInfo *viewInfo)
{
    if ((IDirect3DSurface9 *)dword_EA74EF4[5 * gfxCmdBufState.renderTargetId] != stru_EA74F04.surface.color)
        MyAssertHandler(
            ".\\rb_postfx.cpp",
            143,
            0,
            "%s",
            "gfxRenderTargets[gfxCmdBufState.renderTargetId].surface.color == gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color");
    if (backEnd.glowCount > 0)
    {
        if (backEnd.glowCount != 1)
            MyAssertHandler(".\\rb_postfx.cpp", 148, 0, "%s", "backEnd.glowCount == 1");
        RB_ApplyGlowEffectPass(viewInfo, backEnd.glowImage);
    }
    backEnd.glowCount = 0;
}

void __cdecl RB_ApplyGlowEffectPass(const GfxViewInfo *viewInfo, GfxImage *glowImage)
{
    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 132, 0, "%s", "viewInfo");
    if (!rgp.world)
        MyAssertHandler(".\\rb_postfx.cpp", 133, 0, "%s", "rgp.world");
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, glowImage);
    RB_FullScreenFilter(rgp.glowApplyBloomMaterial);
}

void __cdecl RB_ApplyMergedPostEffects(const GfxViewInfo *viewInfo)
{
    const char *v1; // eax
    float v2; // [esp+1Ch] [ebp-84h]
    float v3; // [esp+20h] [ebp-80h]
    float v4; // [esp+24h] [ebp-7Ch]
    float v5; // [esp+28h] [ebp-78h]
    float v6; // [esp+2Ch] [ebp-74h]
    float v7; // [esp+30h] [ebp-70h]
    float v8; // [esp+34h] [ebp-6Ch]
    float v9; // [esp+38h] [ebp-68h]
    float v10; // [esp+44h] [ebp-5Ch]
    float v11; // [esp+7Ch] [ebp-24h]
    float dofEquation[4]; // [esp+88h] [ebp-18h] BYREF
    float smallFrac; // [esp+98h] [ebp-8h]
    float mediumFrac; // [esp+9Ch] [ebp-4h]

    if (R_UsingDepthOfField(viewInfo))
    {
        if (!viewInfo->needsFloatZ)
            MyAssertHandler(".\\rb_postfx.cpp", 321, 0, "%s", "viewInfo->needsFloatZ");
        RB_GetSceneDepthOfFieldEquation(
            viewInfo->dof.nearStart,
            viewInfo->dof.nearEnd,
            viewInfo->dof.farStart,
            viewInfo->dof.farEnd,
            dofEquation,
            viewInfo->viewParms.zNear);
        if (!Vec4Compare(gfxCmdBufSourceState.input.consts[13], dofEquation))
            R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, 0xDu, dofEquation);
        RB_GetViewModelDepthOfFieldEquation(viewInfo->dof.viewModelStart, viewInfo->dof.viewModelEnd, dofEquation);
        v11 = viewInfo->dof.farBlur / viewInfo->dof.nearBlur;
        v10 = pow(v11, r_dof_bias->current.value);
        dofEquation[3] = v10;
        if (!Vec4Compare(gfxCmdBufSourceState.input.consts[12], dofEquation))
            R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, 0xCu, dofEquation);
        v9 = 1.0 / (double)vidConfig.sceneHeight;
        R_UpdateCodeConstant(&gfxCmdBufSourceState, 0x10u, 0.0, v9, 0.0, 0.0);
        smallFrac = RB_GetDepthOfFieldBlurFraction(viewInfo, 1.4);
        mediumFrac = RB_GetDepthOfFieldBlurFraction(viewInfo, 3.5999999);
        if (smallFrac <= 0.0 || mediumFrac <= (double)smallFrac || mediumFrac >= 1.0)
        {
            v1 = va("%g, %g, %g, %i", smallFrac, mediumFrac, viewInfo->dof.nearBlur, vidConfig.sceneHeight);
            MyAssertHandler(
                ".\\rb_postfx.cpp",
                335,
                0,
                "%s\n\t%s",
                "0.0f < smallFrac && smallFrac < mediumFrac && mediumFrac < 1.0f",
                v1);
        }
        v8 = 1.0 / (1.0 - mediumFrac);
        v7 = -1.0 / (1.0 - mediumFrac);
        v6 = -1.0 / (mediumFrac - smallFrac);
        v5 = -1.0 / smallFrac;
        R_UpdateCodeConstant(&gfxCmdBufSourceState, 0xEu, v5, v6, v7, v8);
        v4 = -mediumFrac / (1.0 - mediumFrac);
        v3 = 1.0 / (1.0 - mediumFrac);
        v2 = mediumFrac / (mediumFrac - smallFrac);
        R_UpdateCodeConstant(&gfxCmdBufSourceState, 0xFu, 1.0, v2, v3, v4);
        RB_GetDepthOfFieldInputImages(viewInfo->dof.nearBlur);
        R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
        R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
        if (RB_UsingColorManipulation(viewInfo))
            RB_FullScreenFilter(rgp.postFxDofColorMaterial);
        else
            RB_FullScreenFilter(rgp.postFxDofMaterial);
    }
    else
    {
        RB_ApplyColorManipulationFullscreen(viewInfo);
    }
}

void __cdecl R_UpdateCodeConstant(
    GfxCmdBufSourceState *source,
    unsigned int constant,
    float x,
    float y,
    float z,
    float w)
{
    float *v6; // [esp+0h] [ebp-8h]

    if (x != source->input.consts[constant][0]
        || y != source->input.consts[constant][1]
        || z != source->input.consts[constant][2]
        || w != source->input.consts[constant][3])
    {
        if (constant >= 0x3A)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
                495,
                0,
                "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
                constant,
                58);
        v6 = source->input.consts[constant];
        *v6 = x;
        v6[1] = y;
        v6[2] = z;
        v6[3] = w;
        R_DirtyCodeConstant(source, constant);
    }
}

void __cdecl RB_GetSceneDepthOfFieldEquation(
    float nearOutOfFocus,
    float nearInFocus,
    float farInFocus,
    float farOutOfFocus,
    float *dofEquation,
    float zNear)
{
    float v6; // [esp+14h] [ebp-Ch]
    float v7; // [esp+18h] [ebp-8h]

    if (zNear == 0.0)
        MyAssertHandler(".\\rb_postfx.cpp", 175, 0, "%s", "zNear");
    RB_GetNearDepthOfFieldEquation(nearOutOfFocus, nearInFocus, zNear, 1.0, dofEquation);
    v7 = zNear - farInFocus;
    if (v7 < 0.0)
        v6 = farInFocus;
    else
        v6 = zNear;
    if (v6 < (double)farOutOfFocus)
    {
        dofEquation[1] = (float)1.0 / (farOutOfFocus - farInFocus);
        dofEquation[3] = farInFocus / (farInFocus - farOutOfFocus);
    }
    else
    {
        dofEquation[1] = 0.0;
        dofEquation[3] = 0.0;
    }
}

void __cdecl RB_GetNearDepthOfFieldEquation(
    float outOfFocus,
    float inFocus,
    float nearClip,
    float depthScale,
    float *dofEquation)
{
    float v5; // [esp+0h] [ebp-8h]
    float v6; // [esp+4h] [ebp-4h]

    v6 = outOfFocus - nearClip;
    if (v6 < 0.0)
        v5 = nearClip;
    else
        v5 = outOfFocus;
    if (v5 >= (double)inFocus)
    {
        inFocus = nearClip * 0.5;
        outOfFocus = 0.0;
    }
    *dofEquation = depthScale / (outOfFocus - inFocus);
    dofEquation[2] = inFocus / (inFocus - outOfFocus);
}

void __cdecl RB_GetViewModelDepthOfFieldEquation(float outOfFocus, float inFocus, float *dofEquation)
{
    float nearClip; // [esp+14h] [ebp-8h]

    nearClip = r_znear_depthhack->current.value;
    dofEquation[1] = 0.0;
    RB_GetNearDepthOfFieldEquation(outOfFocus, inFocus, nearClip, 1.0, dofEquation);
}

double __cdecl RB_GetDepthOfFieldBlurFraction(const GfxViewInfo *viewInfo, float pixelRadiusAtSceneRes)
{
    float fraction; // [esp+18h] [ebp-8h]
    float normalizedRadius; // [esp+1Ch] [ebp-4h]

    if (viewInfo->dof.nearBlur < 4.0)
        MyAssertHandler(
            ".\\rb_postfx.cpp",
            302,
            0,
            "%s\n\t(viewInfo->dof.nearBlur) = %g",
            "(viewInfo->dof.nearBlur >= 4.0f)",
            viewInfo->dof.nearBlur);
    normalizedRadius = pixelRadiusAtSceneRes * 480.0 / (double)vidConfig.sceneHeight;
    fraction = normalizedRadius / viewInfo->dof.nearBlur;
    return (float)pow(fraction, r_dof_bias->current.value);
}

double __cdecl RB_GetBlurRadius(float blurRadiusFromCode)
{
    float v2; // [esp+8h] [ebp-14h]
    float v3; // [esp+Ch] [ebp-10h]
    float blurRadiusSqFromDvar; // [esp+10h] [ebp-Ch]
    float blurRadiusSqFromCode; // [esp+18h] [ebp-4h]

    blurRadiusSqFromDvar = r_blur->current.value * r_blur->current.value;
    blurRadiusSqFromCode = blurRadiusFromCode * blurRadiusFromCode;
    v3 = blurRadiusSqFromCode + blurRadiusSqFromDvar;
    v2 = sqrt(v3);
    if (v2 < 0.0)
        MyAssertHandler(".\\rb_postfx.cpp", 370, 1, "%s\n\t(blurRadiusFinal) = %g", "(blurRadiusFinal >= 0.0f)", v2);
    return v2;
}

void __cdecl RB_BlurScreen(const GfxViewInfo *viewInfo, float blurRadius)
{
    float v2; // [esp+18h] [ebp-18h]
    float blurRadiusMin; // [esp+28h] [ebp-8h]
    unsigned int color; // [esp+2Ch] [ebp-4h]

    if (!viewInfo)
        MyAssertHandler(".\\rb_postfx.cpp", 381, 0, "%s", "viewInfo");
    blurRadiusMin = 1440.0 / (double)gfxCmdBufSourceState.sceneViewport.height;
    color = -1;
    if (blurRadiusMin > (double)blurRadius)
    {
        v2 = blurRadius / blurRadiusMin * 255.0;
        HIBYTE(color) = (int)(v2 + 9.313225746154785e-10);
        blurRadius = 1440.0 / (double)gfxCmdBufSourceState.sceneViewport.height;
    }
    RB_GaussianFilterImage(blurRadius, R_RENDERTARGET_RESOLVED_SCENE, R_RENDERTARGET_POST_EFFECT_0);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, stru_EA74FCC.image);
    if (viewInfo->film.enabled)
        RB_FullScreenColoredFilter(rgp.feedbackFilmBlendMaterial, color);
    else
        RB_FullScreenColoredFilter(rgp.feedbackBlendMaterial, color);
}

