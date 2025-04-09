#include "ui.h"
#include <client_mp/client_mp.h>

#include <ui_mp/ui_mp.h>
#include <database/database.h>

int com_expectedHunkUsage;

double __cdecl UI_LoadBarProgress_LoadObj()
{
    float v2; // [esp+4h] [ebp-14h]
    double v3; // [esp+Ch] [ebp-Ch]
    float v4; // [esp+14h] [ebp-4h]

    if (com_expectedHunkUsage <= 0)
        return 0.0;
    v3 = (double)com_expectedHunkUsage;
    v4 = (double)Hunk_Used() / v3;
    v2 = v4 - 1.0;
    if (v2 < 0.0)
        return v4;
    else
        return (float)1.0;
}

void __cdecl UI_DrawHandlePic(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    const float *color,
    Material *material)
{
    float t0; // [esp+30h] [ebp-10h]
    float t1; // [esp+34h] [ebp-Ch]
    float s1; // [esp+38h] [ebp-8h]
    float s0; // [esp+3Ch] [ebp-4h]

    if (w >= 0.0)
    {
        s0 = 0.0;
        s1 = 1.0;
    }
    else
    {
        w = -w;
        s0 = 1.0;
        s1 = 0.0;
    }
    if (h >= 0.0)
    {
        t0 = 0.0;
        t1 = 1.0;
    }
    else
    {
        h = -h;
        t0 = 1.0;
        t1 = 0.0;
    }
    CL_DrawStretchPic(scrPlace, x, y, w, h, horzAlign, vertAlign, s0, t0, s1, t1, color, material);
}

void __cdecl UI_DrawLoadBar(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    const float *color,
    Material *material)
{
    float v9; // [esp+30h] [ebp-20h]
    double (*v10)(void); // [esp+34h] [ebp-1Ch]
    float percentDone; // [esp+40h] [ebp-10h]

    if (useFastFile->current.enabled)
        v10 = UI_LoadBarProgress_FastFile;
    else
        v10 = UI_LoadBarProgress_LoadObj;
    percentDone = v10();
    v9 = w * percentDone;
    CL_DrawStretchPic(scrPlace, x, y, v9, h, horzAlign, vertAlign, 0.0, 0.0, percentDone, 1.0, color, material);
}

double __cdecl UI_LoadBarProgress_FastFile()
{
    return DB_GetLoadedFraction();
}

void __cdecl UI_FillRectPhysical(float x, float y, float width, float height, const float *color)
{
    if (sharedUiInfo.assets.whiteMaterial)
        CL_DrawStretchPicPhysical(x, y, width, height, 0.0, 0.0, 0.0, 0.0, color, sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_FillRect(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    int horzAlign,
    int vertAlign,
    const float *color)
{
    if (sharedUiInfo.assets.whiteMaterial)
        CL_DrawStretchPic(
            scrPlace,
            x,
            y,
            width,
            height,
            horzAlign,
            vertAlign,
            0.0,
            0.0,
            0.0,
            0.0,
            color,
            sharedUiInfo.assets.whiteMaterial);
}

