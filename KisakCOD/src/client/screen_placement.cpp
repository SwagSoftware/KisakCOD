#include "client.h"
#include <client_mp/client_mp.h>

struct ScreenPlacement *scrPlaceView;
struct ScreenPlacement scrPlaceFull;
float cg_hudSplitscreenScale;
ScreenPlacement scrPlaceFullUnsafe;

void __cdecl ScrPlace_SetupFloatViewport(
    ScreenPlacement *scrPlace,
    float viewportX,
    float viewportY,
    float viewportWidth,
    float viewportHeight)
{
    float horzAspectScale; // [esp+70h] [ebp-Ch]
    float adjustedRealWidth; // [esp+74h] [ebp-8h]
    float horzAspectPixelDiff; // [esp+78h] [ebp-4h]

    memset((unsigned __int8 *)scrPlace, 0xB0u, sizeof(ScreenPlacement));
    scrPlace->realViewportSize[0] = viewportWidth;
    scrPlace->realViewportSize[1] = viewportHeight;
    adjustedRealWidth = viewportHeight * 1.333333373069763 / cls.vidConfig.aspectRatioDisplayPixel;
    if (adjustedRealWidth > (double)viewportWidth)
        adjustedRealWidth = viewportWidth;
    horzAspectPixelDiff = viewportWidth - adjustedRealWidth;
    horzAspectScale = viewportWidth / adjustedRealWidth;
    ScrPlace_CalcSafeAreaOffsets(
        viewportX,
        viewportY,
        viewportWidth,
        viewportHeight,
        horzAspectScale,
        scrPlace->realViewableMin,
        scrPlace->realViewableMax,
        scrPlace->virtualViewableMin,
        scrPlace->virtualViewableMax);
    scrPlace->scaleVirtualToReal[0] = adjustedRealWidth / 640.0;
    scrPlace->scaleVirtualToReal[1] = viewportHeight / 480.0;
    scrPlace->scaleVirtualToFull[0] = viewportWidth / 640.0;
    scrPlace->scaleVirtualToFull[1] = viewportHeight / 480.0;
    scrPlace->scaleRealToVirtual[0] = 640.0 / adjustedRealWidth;
    scrPlace->scaleRealToVirtual[1] = 480.0 / viewportHeight;
    if (horzAspectPixelDiff < 0.0)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            112,
            0,
            "%s\n\t(horzAspectPixelDiff) = %g",
            "(horzAspectPixelDiff >= 0)",
            horzAspectPixelDiff);
    scrPlace->subScreenLeft = horzAspectPixelDiff * 0.5;
}

void __cdecl ScrPlace_CalcSafeAreaOffsets(
    float viewportX,
    float viewportY,
    float viewportWidth,
    float viewportHeight,
    float horzAspectScale,
    float *realViewableMin,
    float *realViewableMax,
    float *virtualViewableMin,
    float *virtualViewableMax)
{
    const char *v9; // eax
    const char *v10; // eax
    float v11; // [esp+18h] [ebp-90h]
    float v12; // [esp+1Ch] [ebp-8Ch]
    float v13; // [esp+20h] [ebp-88h]
    float v14; // [esp+24h] [ebp-84h]
    float v15; // [esp+28h] [ebp-80h]
    float v16; // [esp+2Ch] [ebp-7Ch]
    float v17; // [esp+30h] [ebp-78h]
    float v18; // [esp+34h] [ebp-74h]
    float v19; // [esp+68h] [ebp-40h]
    float v20; // [esp+6Ch] [ebp-3Ch]
    float v21; // [esp+70h] [ebp-38h]
    float v22; // [esp+74h] [ebp-34h]
    float safeAreaRatioHorz; // [esp+88h] [ebp-20h]
    float safeAreaRatioVert; // [esp+94h] [ebp-14h]
    float safeAreaSize; // [esp+98h] [ebp-10h]
    float safeAreaSize_4; // [esp+9Ch] [ebp-Ch]

    safeAreaRatioHorz = 1.0;
    safeAreaRatioVert = 1.0;
    if ((float)1.0 < 0.0 || safeAreaRatioHorz > 1.0)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            51,
            0,
            "safeAreaRatioHorz not in [0, 1]\n\t%g not in [%g, %g]",
            safeAreaRatioHorz,
            0.0,
            1.0);
    if (safeAreaRatioVert < 0.0 || safeAreaRatioVert > 1.0)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            52,
            0,
            "safeAreaRatioVert not in [0, 1]\n\t%g not in [%g, %g]",
            safeAreaRatioVert,
            0.0,
            1.0);
    if (!cls.vidConfig.displayWidth)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            53,
            0,
            "%s\n\t(cls.vidConfig.displayWidth) = %i",
            "(cls.vidConfig.displayWidth > 0)",
            0);
    if (!cls.vidConfig.displayHeight)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            54,
            0,
            "%s\n\t(cls.vidConfig.displayHeight) = %i",
            "(cls.vidConfig.displayHeight > 0)",
            0);
    if (viewportX < 0.0)
        MyAssertHandler(".\\client\\screen_placement.cpp", 55, 0, "%s\n\t(viewportX) = %g", "(viewportX >= 0)", viewportX);
    if (viewportY < 0.0)
        MyAssertHandler(".\\client\\screen_placement.cpp", 56, 0, "%s\n\t(viewportY) = %g", "(viewportY >= 0)", viewportY);
    if (viewportWidth <= 0.0)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            57,
            0,
            "%s\n\t(viewportWidth) = %g",
            "(viewportWidth > 0)",
            viewportWidth);
    if (viewportHeight <= 0.0)
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            58,
            0,
            "%s\n\t(viewportHeight) = %g",
            "(viewportHeight > 0)",
            viewportHeight);
    if ((double)cls.vidConfig.displayWidth < viewportX + viewportWidth)
    {
        v9 = va("%g + %g > %i", viewportX, viewportWidth, cls.vidConfig.displayWidth);
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            59,
            0,
            "%s\n\t%s",
            "viewportX + viewportWidth <= cls.vidConfig.displayWidth",
            v9);
    }
    if ((double)cls.vidConfig.displayHeight < viewportY + viewportHeight)
    {
        v10 = va("%g + %g > %i", viewportY, viewportHeight, cls.vidConfig.displayHeight);
        MyAssertHandler(
            ".\\client\\screen_placement.cpp",
            60,
            0,
            "%s\n\t%s",
            "viewportY + viewportHeight <= cls.vidConfig.displayHeight",
            v10);
    }
    safeAreaSize = (1.0 - safeAreaRatioHorz) * 0.5 * (double)cls.vidConfig.displayWidth;
    safeAreaSize_4 = (1.0 - safeAreaRatioVert) * 0.5 * (double)cls.vidConfig.displayHeight;
    v18 = viewportX - safeAreaSize;
    if (v18 < 0.0)
        v17 = (1.0 - safeAreaRatioHorz) * 0.5 * (double)cls.vidConfig.displayWidth;
    else
        v17 = viewportX;
    v16 = viewportY - safeAreaSize_4;
    if (v16 < 0.0)
        v15 = (1.0 - safeAreaRatioVert) * 0.5 * (double)cls.vidConfig.displayHeight;
    else
        v15 = viewportY;
    v20 = viewportWidth + viewportX;
    v21 = (double)cls.vidConfig.displayWidth - safeAreaSize;
    v14 = v21 - v20;
    if (v14 < 0.0)
        v13 = (double)cls.vidConfig.displayWidth - safeAreaSize;
    else
        v13 = viewportWidth + viewportX;
    v19 = viewportHeight + viewportY;
    v22 = (double)cls.vidConfig.displayHeight - safeAreaSize_4;
    v12 = v22 - v19;
    if (v12 < 0.0)
        v11 = (double)cls.vidConfig.displayHeight - safeAreaSize_4;
    else
        v11 = viewportHeight + viewportY;
    *realViewableMin = v17 - viewportX;
    realViewableMin[1] = v15 - viewportY;
    *realViewableMax = v13 - viewportX;
    realViewableMax[1] = v11 - viewportY;
    *virtualViewableMin = *realViewableMin * horzAspectScale * (640.0 / viewportWidth);
    virtualViewableMin[1] = 480.0 / viewportHeight * realViewableMin[1];
    *virtualViewableMax = *realViewableMax * horzAspectScale * (640.0 / viewportWidth);
    virtualViewableMax[1] = 480.0 / viewportHeight * realViewableMax[1];
}

void __cdecl ScrPlace_SetupViewport(
    ScreenPlacement *scrPlace,
    int viewportX,
    int viewportY,
    int viewportWidth,
    int viewportHeight)
{
    float v5; // [esp+0h] [ebp-10h]
    float v6; // [esp+4h] [ebp-Ch]
    float v7; // [esp+8h] [ebp-8h]
    float v8; // [esp+Ch] [ebp-4h]

    v8 = (float)viewportHeight;
    v7 = (float)viewportWidth;
    v6 = (float)viewportY;
    v5 = (float)viewportX;
    ScrPlace_SetupFloatViewport(scrPlace, v5, v6, v7, v8);
}

void __cdecl ScrPlace_SetupUnsafeViewport(
    ScreenPlacement *scrPlace,
    int viewportX,
    int viewportY,
    int viewportWidth,
    int viewportHeight)
{
    float v5; // [esp+0h] [ebp-18h]
    float v6; // [esp+4h] [ebp-14h]
    float v7; // [esp+8h] [ebp-10h]
    float v8; // [esp+Ch] [ebp-Ch]

    v8 = (float)viewportHeight;
    v7 = (float)viewportWidth;
    v6 = (float)viewportY;
    v5 = (float)viewportX;
    ScrPlace_SetupFloatViewport(scrPlace, v5, v6, v7, v8);
}

double __cdecl ScrPlace_ApplyX(const ScreenPlacement *scrPlace, float x, int horzAlign)
{
    double result; // st7
    float v4; // [esp+0h] [ebp-20h]
    float v5; // [esp+4h] [ebp-1Ch]
    float v6; // [esp+8h] [ebp-18h]
    float v7; // [esp+Ch] [ebp-14h]
    float v8; // [esp+10h] [ebp-10h]
    float v9; // [esp+14h] [ebp-Ch]
    float v10; // [esp+18h] [ebp-8h]

    switch (horzAlign)
    {
    case 1:
        v9 = x * scrPlace->scaleVirtualToReal[0] + scrPlace->realViewableMin[0];
        result = v9;
        break;
    case 2:
        v8 = x * scrPlace->scaleVirtualToReal[0] + scrPlace->realViewportSize[0] * 0.5;
        result = v8;
        break;
    case 3:
        v6 = x * scrPlace->scaleVirtualToReal[0] + scrPlace->realViewableMax[0];
        result = v6;
        break;
    case 4:
        v5 = x * scrPlace->scaleVirtualToFull[0];
        result = v5;
        break;
    case 5:
        result = x;
        break;
    case 6:
        v4 = x * scrPlace->scaleRealToVirtual[0];
        result = v4;
        break;
    case 7:
        v7 = x * scrPlace->scaleVirtualToReal[0] + (scrPlace->realViewableMin[0] + scrPlace->realViewableMax[0]) * 0.5;
        result = v7;
        break;
    default:
        if (horzAlign)
            MyAssertHandler(
                ".\\client\\screen_placement.cpp",
                158,
                0,
                "%s\n\t(horzAlign) = %i",
                "(horzAlign == 0)",
                horzAlign);
        v10 = x * scrPlace->scaleVirtualToReal[0] + scrPlace->subScreenLeft;
        result = v10;
        break;
    }
    return result;
}

double __cdecl ScrPlace_ApplyY(const ScreenPlacement *scrPlace, float y, int vertAlign)
{
    double result; // st7
    float v4; // [esp+0h] [ebp-20h]
    float v5; // [esp+4h] [ebp-1Ch]
    float v6; // [esp+8h] [ebp-18h]
    float v7; // [esp+Ch] [ebp-14h]
    float v8; // [esp+10h] [ebp-10h]
    float v9; // [esp+14h] [ebp-Ch]
    float v10; // [esp+18h] [ebp-8h]

    switch (vertAlign)
    {
    case 1:
        v9 = y * scrPlace->scaleVirtualToReal[1] + scrPlace->realViewableMin[1];
        result = v9;
        break;
    case 2:
        v8 = y * scrPlace->scaleVirtualToReal[1] + scrPlace->realViewportSize[1] * 0.5;
        result = v8;
        break;
    case 3:
        v6 = y * scrPlace->scaleVirtualToReal[1] + scrPlace->realViewableMax[1];
        result = v6;
        break;
    case 4:
        v5 = y * scrPlace->scaleVirtualToFull[1];
        result = v5;
        break;
    case 5:
        result = y;
        break;
    case 6:
        v4 = y * scrPlace->scaleRealToVirtual[1];
        result = v4;
        break;
    case 7:
        v7 = y * scrPlace->scaleVirtualToReal[1] + (scrPlace->realViewableMin[1] + scrPlace->realViewableMax[1]) * 0.5;
        result = v7;
        break;
    default:
        if (vertAlign)
            MyAssertHandler(
                ".\\client\\screen_placement.cpp",
                195,
                0,
                "%s\n\t(vertAlign) = %i",
                "(vertAlign == 0)",
                vertAlign);
        v10 = y * scrPlace->scaleVirtualToReal[1];
        result = v10;
        break;
    }
    return result;
}

void __cdecl ScrPlace_ApplyRect(
    const ScreenPlacement *scrPlace,
    float *x,
    float *y,
    float *w,
    float *h,
    int horzAlign,
    int vertAlign)
{
    if (!x)
        MyAssertHandler(".\\client\\screen_placement.cpp", 244, 0, "%s", "x");
    if (!y)
        MyAssertHandler(".\\client\\screen_placement.cpp", 245, 0, "%s", "y");
    if (!w)
        MyAssertHandler(".\\client\\screen_placement.cpp", 246, 0, "%s", "w");
    if (!h)
        MyAssertHandler(".\\client\\screen_placement.cpp", 247, 0, "%s", "h");
    switch (horzAlign)
    {
    case 0:
        goto $LN22_10;
    case 1:
        *x = *x * scrPlace->scaleVirtualToReal[0] + scrPlace->realViewableMin[0];
        *w = *w * scrPlace->scaleVirtualToReal[0];
        break;
    case 2:
        *x = *x * scrPlace->scaleVirtualToReal[0] + scrPlace->realViewportSize[0] * 0.5;
        *w = *w * scrPlace->scaleVirtualToReal[0];
        break;
    case 3:
        *x = *x * scrPlace->scaleVirtualToReal[0] + scrPlace->realViewableMax[0];
        *w = *w * scrPlace->scaleVirtualToReal[0];
        break;
    case 4:
        *x = *x * scrPlace->scaleVirtualToFull[0];
        *w = *w * scrPlace->scaleVirtualToFull[0];
        break;
    case 5:
        break;
    case 6:
        *x = *x * scrPlace->scaleRealToVirtual[0];
        *w = *w * scrPlace->scaleRealToVirtual[0];
        break;
    case 7:
        *x = *x * scrPlace->scaleVirtualToReal[0] + (scrPlace->realViewableMin[0] + scrPlace->realViewableMax[0]) * 0.5;
        *w = *w * scrPlace->scaleVirtualToReal[0];
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\client\\screen_placement.cpp", 256, 0, "invalid horizontal alignment case");
    $LN22_10:
        *x = *x * scrPlace->scaleVirtualToReal[0] + scrPlace->subScreenLeft;
        *w = *w * scrPlace->scaleVirtualToReal[0];
        break;
    }
    switch (vertAlign)
    {
    case 0:
        goto $LN9_20;
    case 1:
        *y = *y * scrPlace->scaleVirtualToReal[1] + scrPlace->realViewableMin[1];
        *h = *h * scrPlace->scaleVirtualToReal[1];
        break;
    case 2:
        *y = *y * scrPlace->scaleVirtualToReal[1] + scrPlace->realViewportSize[1] * 0.5;
        *h = *h * scrPlace->scaleVirtualToReal[1];
        break;
    case 3:
        *y = *y * scrPlace->scaleVirtualToReal[1] + scrPlace->realViewableMax[1];
        *h = *h * scrPlace->scaleVirtualToReal[1];
        break;
    case 4:
        *y = *y * scrPlace->scaleVirtualToFull[1];
        *h = *h * scrPlace->scaleVirtualToFull[1];
        break;
    case 5:
        return;
    case 6:
        *y = *y * scrPlace->scaleRealToVirtual[1];
        *h = *h * scrPlace->scaleRealToVirtual[1];
        break;
    case 7:
        *y = *y * scrPlace->scaleVirtualToReal[1] + (scrPlace->realViewableMin[1] + scrPlace->realViewableMax[1]) * 0.5;
        *h = *h * scrPlace->scaleVirtualToReal[1];
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\client\\screen_placement.cpp", 292, 0, "invalid vertical alignment case");
    $LN9_20:
        *y = *y * scrPlace->scaleVirtualToReal[1];
        *h = *h * scrPlace->scaleVirtualToReal[1];
        break;
    }
}

