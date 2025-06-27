#pragma once
#include "r_rendercmds.h"

struct GfxImageFilterPass // sizeof=0x98
{                                       // ...
    const Material *material;           // ...
    float srcWidth;                     // ...
    float srcHeight;                    // ...
    int dstWidth;                       // ...
    int dstHeight;                      // ...
    int tapHalfCount;                   // ...
    float tapOffsetsAndWeights[8][4];
};

struct GfxImageFilter // sizeof=0x130C
{                                       // ...
    int passCount;                      // ...
    GfxImageFilterPass passes[32];      // ...
    GfxImage *sourceImage;              // ...
    GfxRenderTargetId finalTarget;      // ...
};


void __cdecl RB_GaussianFilterImage(
    float radius,
    GfxRenderTargetId srcRenderTargetId,
    GfxRenderTargetId dstRenderTargetId);
void __cdecl RB_VirtualToSceneRadius(float radius, float *radiusX, float *radiusY);
int __cdecl RB_GenerateGaussianFilterChain(
    float radiusX,
    float radiusY,
    unsigned int srcWidth,
    unsigned int srcHeight,
    int dstWidth,
    int dstHeight,
    int passLimit,
    GfxImageFilterPass *filterPass);
void __cdecl RB_GenerateGaussianFilter1D(float radius, int *res, int axis, GfxImageFilterPass *filterPass);
int __cdecl RB_PickSymmetricFilterMaterial(int halfTapCount, const Material **material);
int __cdecl RB_GaussianFilterPoints1D(float pixels, int srcRes, int dstRes, int tapLimit, float *tapOffsets, float *tapWeights);
void __cdecl RB_GenerateGaussianFilter2D(
    float radius,
    unsigned int srcWidth,
    unsigned int srcHeight,
    int dstWidth,
    int dstHeight,
    GfxImageFilterPass *filterPass);
void __cdecl RB_FilterImage(GfxImageFilter *filter);
void __cdecl RB_SetupFilterPass(const GfxImageFilterPass *filterPass);
void __cdecl RB_FilterPingPong(const GfxImageFilter *filter, int passIndex);
void __cdecl RB_GlowFilterImage(float radius);
GfxRenderTargetId __cdecl RB_ApplyGlowFilter(
    float radius,
    GfxRenderTargetId srcRenderTarget,
    GfxRenderTargetId dstRenderTarget);
