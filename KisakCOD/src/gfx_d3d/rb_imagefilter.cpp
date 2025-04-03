#include "rb_imagefilter.h"


void __cdecl RB_GaussianFilterImage(
    float radius,
    GfxRenderTargetId srcRenderTargetId,
    GfxRenderTargetId dstRenderTargetId)
{
    float radiusY; // [esp+20h] [ebp-1328h] BYREF
    int srcWidth; // [esp+24h] [ebp-1324h]
    float radiusX; // [esp+28h] [ebp-1320h] BYREF
    int dstWidth; // [esp+2Ch] [ebp-131Ch]
    int srcHeight; // [esp+30h] [ebp-1318h]
    int dstHeight; // [esp+34h] [ebp-1314h]
    GfxImageFilter filter; // [esp+38h] [ebp-1310h] BYREF

    RB_VirtualToSceneRadius(radius, &radiusX, &radiusY);
    srcWidth = dword_EA74EFC[5 * srcRenderTargetId];
    srcHeight = dword_EA74F00[5 * srcRenderTargetId];
    dstWidth = dword_EA74EFC[5 * dstRenderTargetId];
    dstHeight = dword_EA74F00[5 * dstRenderTargetId];
    filter.sourceImage = gfxRenderTargets[srcRenderTargetId].image;
    filter.finalTarget = dstRenderTargetId;
    filter.passCount = RB_GenerateGaussianFilterChain(
        radiusX,
        radiusY,
        srcWidth,
        srcHeight,
        dstWidth,
        dstHeight,
        32,
        filter.passes);
    RB_FilterImage(&filter);
}

void __cdecl RB_VirtualToSceneRadius(float radius, float *radiusX, float *radiusY)
{
    if (!radiusX)
        MyAssertHandler(".\\rb_imagefilter.cpp", 57, 0, "%s", "radiusX");
    if (!radiusY)
        MyAssertHandler(".\\rb_imagefilter.cpp", 58, 0, "%s", "radiusY");
    *radiusY = (double)vidConfig.sceneHeight * radius / 480.0;
    *radiusX = *radiusY * vidConfig.aspectRatioScenePixel;
}

int __cdecl RB_GenerateGaussianFilterChain(
    float radiusX,
    float radiusY,
    unsigned int srcWidth,
    unsigned int srcHeight,
    int dstWidth,
    int dstHeight,
    int passLimit,
    GfxImageFilterPass *filterPass)
{
    float v9; // [esp+18h] [ebp-58h]
    float v10; // [esp+1Ch] [ebp-54h]
    float v11; // [esp+20h] [ebp-50h]
    float v12; // [esp+24h] [ebp-4Ch]
    float v13; // [esp+28h] [ebp-48h]
    float v14; // [esp+2Ch] [ebp-44h]
    float v15; // [esp+30h] [ebp-40h]
    float v16; // [esp+3Ch] [ebp-34h]
    float v17; // [esp+40h] [ebp-30h]
    float v18; // [esp+4Ch] [ebp-24h]
    float v19; // [esp+50h] [ebp-20h]
    float v20; // [esp+54h] [ebp-1Ch]
    int dstRes[2]; // [esp+58h] [ebp-18h] BYREF
    float passRadius; // [esp+60h] [ebp-10h]
    int passRes; // [esp+64h] [ebp-Ch]
    int passAxis; // [esp+68h] [ebp-8h]
    int passCount; // [esp+6Ch] [ebp-4h]

    dstRes[0] = dstWidth;
    dstRes[1] = dstHeight;
    passCount = 0;
    if (srcWidth != dstWidth || srcHeight != dstHeight)
    {
        v20 = radiusY - radiusX;
        if (v20 < 0.0)
            v19 = radiusY;
        else
            v19 = radiusX;
        passRadius = v19;
        if (v19 > 1.389560461044312)
            passRadius = 1.3895605;
        v18 = radiusX * radiusX - passRadius * passRadius;
        v17 = sqrt(v18);
        radiusX = v17 * (double)dstWidth / (double)(int)srcWidth;
        v16 = radiusY * radiusY - passRadius * passRadius;
        v15 = sqrt(v16);
        radiusY = v15 * (double)dstHeight / (double)(int)srcHeight;
        if (passCount >= passLimit)
            MyAssertHandler(".\\rb_imagefilter.cpp", 237, 0, "%s", "passCount < passLimit");
        RB_GenerateGaussianFilter2D(passRadius, srcWidth, srcHeight, dstWidth, dstHeight, &filterPass[passCount++]);
    }
    while (passCount < 32 && (radiusX >= 0.3295051157474518 || radiusY >= 0.3295051157474518))
    {
        v14 = radiusX - radiusY;
        v13 = fabs(v14);
        if (v13 < 0.3295051157474518)
        {
            passRadius = (radiusX + radiusY) * 0.5;
            if (passRadius <= 1.389560461044312)
            {
                if (passCount >= passLimit)
                    MyAssertHandler(".\\rb_imagefilter.cpp", 249, 0, "%s", "passCount < passLimit");
                RB_GenerateGaussianFilter2D(passRadius, dstWidth, dstHeight, dstWidth, dstHeight, &filterPass[passCount++]);
                break;
            }
        }
        if (radiusY >= (double)radiusX)
        {
            if (radiusY > 6.497750282287598)
            {
                passRadius = 6.4977503;
                v10 = radiusY * radiusY - 42.22076034545898;
                v9 = sqrt(v10);
                radiusY = v9;
            }
            else
            {
                passRadius = radiusY;
                radiusY = 0.0;
            }
            passAxis = 1;
            passRes = dstHeight;
        }
        else
        {
            if (radiusX >= 6.497750282287598)
            {
                passRadius = 6.4977503;
                v12 = radiusX * radiusX - 42.22076034545898;
                v11 = sqrt(v12);
                radiusX = v11;
            }
            else
            {
                passRadius = radiusX;
                radiusX = 0.0;
            }
            passAxis = 0;
            passRes = dstWidth;
        }
        if (passCount >= passLimit)
            MyAssertHandler(".\\rb_imagefilter.cpp", 287, 0, "%s", "passCount < passLimit");
        RB_GenerateGaussianFilter1D(passRadius, dstRes, passAxis, &filterPass[passCount++]);
    }
    if (passCount > passLimit)
        MyAssertHandler(".\\rb_imagefilter.cpp", 292, 1, "%s", "passCount <= passLimit");
    return passCount;
}

void __cdecl RB_GenerateGaussianFilter1D(float radius, int *res, int axis, GfxImageFilterPass *filterPass)
{
    __int64 v4; // [esp+4h] [ebp-5Ch]
    float tapWeights[8]; // [esp+18h] [ebp-48h] BYREF
    int tapHalfCount; // [esp+38h] [ebp-28h]
    int tapIndex; // [esp+3Ch] [ebp-24h]
    float tapOffsets[8]; // [esp+40h] [ebp-20h] BYREF

    HIDWORD(v4) = res[axis];
    LODWORD(v4) = HIDWORD(v4);
    tapHalfCount = RB_GaussianFilterPoints1D(radius, v4, 8, tapOffsets, tapWeights);
    filterPass->tapHalfCount = RB_PickSymmetricFilterMaterial(tapHalfCount, &filterPass->material);
    for (tapIndex = 0; tapIndex < 8; ++tapIndex)
    {
        filterPass->tapOffsetsAndWeights[tapIndex][axis] = tapOffsets[tapIndex];
        filterPass->tapOffsetsAndWeights[tapIndex][1 - axis] = 0.0;
        filterPass->tapOffsetsAndWeights[tapIndex][2] = 0.0;
        filterPass->tapOffsetsAndWeights[tapIndex][3] = tapWeights[tapIndex];
    }
    filterPass->srcWidth = 1.0;
    filterPass->srcHeight = 1.0;
    filterPass->dstWidth = *res;
    filterPass->dstHeight = res[1];
}

int __cdecl RB_PickSymmetricFilterMaterial(int halfTapCount, const Material **material)
{
    if (halfTapCount <= 0 || halfTapCount > 8)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            76,
            0,
            "%s\n\t(halfTapCount) = %i",
            "(halfTapCount > 0 && halfTapCount <= 8)",
            halfTapCount);
    *material = (const Material *)*((unsigned int *)&rgp.postFxMaterial + halfTapCount);
    return halfTapCount;
}

int __cdecl RB_GaussianFilterPoints1D(float pixels, __int64 srcRes, int tapLimit, float *tapOffsets, float *tapWeights)
{
    const char *v5; // eax
    double v6; // st7
    float v8; // [esp+8h] [ebp-58h]
    float v9; // [esp+Ch] [ebp-54h]
    float v10; // [esp+10h] [ebp-50h]
    float v11; // [esp+14h] [ebp-4Ch]
    float v12; // [esp+20h] [ebp-40h]
    float v13; // [esp+24h] [ebp-3Ch]
    int tapHalfCount; // [esp+34h] [ebp-2Ch]
    int tapIndex; // [esp+38h] [ebp-28h]
    int tapIndexa; // [esp+38h] [ebp-28h]
    int resolutionRatio; // [esp+3Ch] [ebp-24h]
    float totalWeight; // [esp+44h] [ebp-1Ch]
    float gaussianExponent; // [esp+48h] [ebp-18h]
    float weight; // [esp+4Ch] [ebp-14h]
    float weightScale; // [esp+54h] [ebp-Ch]
    float sample; // [esp+58h] [ebp-8h]
    float sample_4; // [esp+5Ch] [ebp-4h]

    if (pixels <= 0.0)
        MyAssertHandler(".\\rb_imagefilter.cpp", 94, 0, "%s\n\t(pixels) = %g", "(pixels > 0)", pixels);
    if (SHIDWORD(srcRes) <= 0)
        MyAssertHandler(".\\rb_imagefilter.cpp", 95, 0, "%s\n\t(dstRes) = %i", "(dstRes > 0)", HIDWORD(srcRes));
    if ((int)srcRes < SHIDWORD(srcRes))
        MyAssertHandler(".\\rb_imagefilter.cpp", 96, 0, "srcRes >= dstRes\n\t%i, %i", (unsigned int)srcRes, HIDWORD(srcRes));
    v13 = (double)(int)srcRes / (double)SHIDWORD(srcRes);
    resolutionRatio = (int)(v13 + 9.313225746154785e-10);
    if ((int)abs(srcRes - HIDWORD(srcRes) * resolutionRatio) >= resolutionRatio)
    {
        v5 = va("%i %i", (unsigned int)srcRes, HIDWORD(srcRes));
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            99,
            0,
            "%s\n\t%s",
            "abs( srcRes - resolutionRatio * dstRes ) < resolutionRatio",
            v5);
    }
    if ((resolutionRatio & 1) != 0)
        v12 = 0.0;
    else
        v12 = 0.5;
    gaussianExponent = -0.5 / (pixels * pixels);
    totalWeight = 0.0;
    for (tapIndex = 0; tapIndex < tapLimit; ++tapIndex)
    {
        sample = (double)(2 * tapIndex) + v12;
        sample_4 = (double)(2 * tapIndex + 1) + v12;
        v11 = sample * (sample * gaussianExponent);
        v10 = exp(v11);
        weight = v10;
        v9 = sample_4 * (sample_4 * gaussianExponent);
        v8 = exp(v9);
        if (!tapIndex && v12 == 0.0)
            weight = v10 * 0.5;
        tapWeights[tapIndex] = weight + v8;
        if (tapWeights[tapIndex] == 0.0)
            v6 = (sample + sample_4) * 0.5 / (double)(int)srcRes;
        else
            v6 = (sample * weight + sample_4 * v8) / ((double)(int)srcRes * tapWeights[tapIndex]);
        tapOffsets[tapIndex] = v6;
        totalWeight = totalWeight + tapWeights[tapIndex];
    }
    if (totalWeight > 0.001000000047497451)
    {
        if (totalWeight <= 0.0)
            MyAssertHandler(".\\rb_imagefilter.cpp", 131, 0, "%s\n\t(totalWeight) = %g", "(totalWeight > 0)", totalWeight);
        tapHalfCount = tapLimit;
        for (tapIndexa = tapLimit - 1; tapIndexa >= 0; --tapIndexa)
        {
            weightScale = 0.5 / totalWeight;
            tapWeights[tapIndexa] = tapWeights[tapIndexa] * weightScale;
            if (tapWeights[tapIndexa] < 0.009999999776482582)
                tapHalfCount = tapIndexa + 1;
        }
    }
    else
    {
        *tapWeights = 0.5;
        return 1;
    }
    return tapHalfCount;
}

void __cdecl RB_GenerateGaussianFilter2D(
    float radius,
    unsigned int srcWidth,
    unsigned int srcHeight,
    int dstWidth,
    int dstHeight,
    GfxImageFilterPass *filterPass)
{
    float tapOffsetsY[2]; // [esp+18h] [ebp-2Ch] BYREF
    float tapWeightsX[2]; // [esp+20h] [ebp-24h] BYREF
    int tapIndex; // [esp+28h] [ebp-1Ch]
    float tapOffsetsX[2]; // [esp+2Ch] [ebp-18h] BYREF
    int x; // [esp+34h] [ebp-10h]
    int y; // [esp+38h] [ebp-Ch]
    float tapWeightsY[2]; // [esp+3Ch] [ebp-8h] BYREF

    RB_GaussianFilterPoints1D(radius, __SPAIR64__(dstWidth, srcWidth), 2, tapOffsetsX, tapWeightsX);
    RB_GaussianFilterPoints1D(radius, __SPAIR64__(dstHeight, srcHeight), 2, tapOffsetsY, tapWeightsY);
    tapIndex = 0;
    for (y = 0; y < 2; ++y)
    {
        for (x = 0; x < 2; ++x)
        {
            filterPass->tapOffsetsAndWeights[2 * tapIndex][0] = -tapOffsetsX[x];
            filterPass->tapOffsetsAndWeights[2 * tapIndex][1] = tapOffsetsY[y];
            filterPass->tapOffsetsAndWeights[2 * tapIndex][2] = 0.0;
            filterPass->tapOffsetsAndWeights[2 * tapIndex][3] = tapWeightsX[x] * tapWeightsY[y];
            filterPass->tapOffsetsAndWeights[2 * tapIndex + 1][0] = tapOffsetsX[x];
            filterPass->tapOffsetsAndWeights[2 * tapIndex + 1][1] = tapOffsetsY[y];
            filterPass->tapOffsetsAndWeights[2 * tapIndex + 1][2] = 0.0;
            filterPass->tapOffsetsAndWeights[2 * tapIndex++ + 1][3] = tapWeightsX[x] * tapWeightsY[y];
        }
    }
    if (2 * tapIndex != 8)
        MyAssertHandler(".\\rb_imagefilter.cpp", 203, 1, "%s\n\t(tapIndex) = %i", "(tapIndex * 2 == 8)", tapIndex);
    filterPass->tapHalfCount = RB_PickSymmetricFilterMaterial(2 * tapIndex, &filterPass->material);
    filterPass->srcWidth = 1.0;
    filterPass->srcHeight = 1.0;
    filterPass->dstWidth = dstWidth;
    filterPass->dstHeight = dstHeight;
}

void __cdecl RB_FilterImage(GfxImageFilter *filter)
{
    int passIndex; // [esp+2Ch] [ebp-Ch]
    float h; // [esp+30h] [ebp-8h]
    float w; // [esp+34h] [ebp-4h]

    if (!filter)
        MyAssertHandler(".\\rb_imagefilter.cpp", 359, 0, "%s", "filter");
    if (filter->passCount <= 0)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            360,
            0,
            "%s\n\t(filter->passCount) = %i",
            "(filter->passCount > 0)",
            filter->passCount);
    if (!filter->sourceImage)
        MyAssertHandler(".\\rb_imagefilter.cpp", 361, 0, "%s", "filter->sourceImage");
    if (tess.indexCount)
        RB_EndTessSurface();
    for (passIndex = 0; passIndex < filter->passCount; ++passIndex)
    {
        RB_FilterPingPong(filter, passIndex);
        RB_SetupFilterPass(&filter->passes[passIndex]);
        w = (float)filter->passes[passIndex].dstWidth;
        h = (float)filter->passes[passIndex].dstHeight;
        R_Set2D(&gfxCmdBufSourceState);
        RB_DrawStretchPic(
            filter->passes[passIndex].material,
            0.0,
            0.0,
            w,
            h,
            0.0,
            0.0,
            filter->passes[passIndex].srcWidth,
            filter->passes[passIndex].srcHeight,
            0xFFFFFFFF,
            GFX_PRIM_STATS_CODE);
        RB_EndTessSurface();
    }
    CL_ResetStats_f();
}

void __cdecl RB_SetupFilterPass(const GfxImageFilterPass *filterPass)
{
    int constIndex; // [esp+4h] [ebp-4h]

    if (filterPass->tapHalfCount >= 9u)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            302,
            0,
            "filterPass->tapHalfCount doesn't index ARRAY_COUNT( filterPass->tapOffsetsAndWeights ) + 1\n\t%i not in [0, %i)",
            filterPass->tapHalfCount,
            9);
    for (constIndex = 0; constIndex < filterPass->tapHalfCount; ++constIndex)
        R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, constIndex + 21, filterPass->tapOffsetsAndWeights[constIndex]);
}

void __cdecl RB_FilterPingPong(const GfxImageFilter *filter, int passIndex)
{
    GfxRenderTargetId finalTarget; // [esp+0h] [ebp-14h]
    GfxImage *image; // [esp+4h] [ebp-10h]
    unsigned int pingpong; // [esp+8h] [ebp-Ch]

    pingpong = passIndex & 1;
    if (passIndex)
        image = gfxRenderTargets[8 - pingpong].image;
    else
        image = filter->sourceImage;
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 9u, image);
    if (passIndex == filter->passCount - 1)
        finalTarget = filter->finalTarget;
    else
        finalTarget = pingpong + 7;
    R_SetRenderTargetSize(&gfxCmdBufSourceState, finalTarget);
    R_SetRenderTarget(gfxCmdBufContext, finalTarget);
}

void __cdecl RB_GlowFilterImage(float radius)
{
    float radiusScale; // [esp+1Ch] [ebp-8h]
    float radiusa; // [esp+2Ch] [ebp+8h]

    if (backEnd.glowCount)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            472,
            0,
            "%s\n\t(backEnd.glowCount) = %i",
            "(backEnd.glowCount == 0)",
            backEnd.glowCount);
    if (radius != 0.0)
    {
        radiusScale = (double)stru_EA74FCC.width / (double)image.width;
        radiusa = radius * radiusScale;
        backEnd.glowImage = gfxRenderTargets[RB_ApplyGlowFilter(
            radiusa,
            R_RENDERTARGET_RESOLVED_SCENE,
            R_RENDERTARGET_POST_EFFECT_0)].image;
        backEnd.glowCount = 1;
    }
}

GfxRenderTargetId __cdecl RB_ApplyGlowFilter(
    float radius,
    GfxRenderTargetId srcRenderTarget,
    GfxRenderTargetId dstRenderTarget)
{
    int GaussianFilterChain; // eax
    float v5; // [esp+20h] [ebp-1324h]
    float radiusY; // [esp+24h] [ebp-1320h] BYREF
    int srcWidth; // [esp+28h] [ebp-131Ch]
    int srcHeight; // [esp+2Ch] [ebp-1318h]
    float radiusX; // [esp+30h] [ebp-1314h] BYREF
    GfxImageFilter filter; // [esp+34h] [ebp-1310h] BYREF

    if ((unsigned int)srcRenderTarget > R_RENDERTARGET_SHADOWMAP_SPOT)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            427,
            0,
            "srcRenderTarget not in [0, R_RENDERTARGET_COUNT - 1]\n\t%i not in [%i, %i]",
            srcRenderTarget,
            0,
            14);
    if ((unsigned int)dstRenderTarget > R_RENDERTARGET_SHADOWMAP_SPOT)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            428,
            0,
            "dstRenderTarget not in [0, R_RENDERTARGET_COUNT - 1]\n\t%i not in [%i, %i]",
            dstRenderTarget,
            0,
            14);
    if (radius < 0.0)
        MyAssertHandler(".\\rb_imagefilter.cpp", 429, 0, "%s\n\t(radius) = %g", "(radius >= 0)", radius);
    if (radius == 0.0)
        return srcRenderTarget;
    if (dword_EA74EFC[5 * dstRenderTarget] != (unsigned int)dword_EA74EFC[5 * srcRenderTarget] >> 2)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            433,
            0,
            "gfxRenderTargets[dstRenderTarget].width == gfxRenderTargets[srcRenderTarget].width / 4\n\t%i, %i",
            dword_EA74EFC[5 * dstRenderTarget],
            (unsigned int)dword_EA74EFC[5 * srcRenderTarget] >> 2);
    if (dword_EA74F00[5 * dstRenderTarget] != (unsigned int)dword_EA74F00[5 * srcRenderTarget] >> 2)
        MyAssertHandler(
            ".\\rb_imagefilter.cpp",
            434,
            0,
            "gfxRenderTargets[dstRenderTarget].height == gfxRenderTargets[srcRenderTarget].height / 4\n\t%i, %i",
            dword_EA74F00[5 * dstRenderTarget],
            (unsigned int)dword_EA74F00[5 * srcRenderTarget] >> 2);
    v5 = radius * 0.25;
    RB_VirtualToSceneRadius(v5, &radiusX, &radiusY);
    srcWidth = dword_EA74EFC[5 * dstRenderTarget];
    srcHeight = dword_EA74F00[5 * dstRenderTarget];
    filter.sourceImage = gfxRenderTargets[srcRenderTarget].image;
    filter.finalTarget = dstRenderTarget;
    filter.passes[0].srcWidth = 1.0;
    filter.passes[0].srcHeight = 1.0;
    filter.passes[0].dstWidth = srcWidth;
    filter.passes[0].dstHeight = srcHeight;
    filter.passes[0].tapHalfCount = 0;
    filter.passes[0].material = rgp.glowConsistentSetupMaterial;
    filter.passCount = 1;
    GaussianFilterChain = RB_GenerateGaussianFilterChain(
        radiusX,
        radiusY,
        srcWidth,
        srcHeight,
        srcWidth,
        srcHeight,
        31,
        &filter.passes[1]);
    filter.passCount += GaussianFilterChain;
    if (!filter.passCount)
        return srcRenderTarget;
    RB_FilterImage(&filter);
    return dstRenderTarget;
}

