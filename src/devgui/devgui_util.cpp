#include "devgui.h"
#include <cgame/cg_local.h>
#include <gfx_d3d/r_rendercmds.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <client/client.h>
#endif


uint32_t __cdecl DevGui_GetScreenWidth()
{
    return cls.vidConfig.displayWidth;
}

uint32_t __cdecl DevGui_GetScreenHeight()
{
    return cls.vidConfig.displayHeight;
}

void __cdecl DevGui_DrawBox(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *color)
{
    float v5; // [esp+0h] [ebp-58h]
    float v6; // [esp+4h] [ebp-54h]
    float v7; // [esp+8h] [ebp-50h]
    float v8; // [esp+Ch] [ebp-4Ch]
    float unpackedColor[4]; // [esp+48h] [ebp-10h] BYREF

    if (!w)
        MyAssertHandler(".\\devgui\\devgui_util.cpp", 116, 0, "%s", "w");
    if (!h)
        MyAssertHandler(".\\devgui\\devgui_util.cpp", 117, 0, "%s", "h");
    Byte4UnpackRgba(color, unpackedColor);
    v8 = (float)h;
    v7 = (float)w;
    v6 = (float)y;
    v5 = (float)x;
    R_AddCmdDrawStretchPic(v5, v6, v7, v8, 0.0, 0.0, 0.0, 0.0, unpackedColor, cls.whiteMaterial);
}

void __cdecl DevGui_DrawBoxCentered(int32_t centerX, int32_t centerY, int32_t w, int32_t h, const uint8_t *color)
{
    DevGui_DrawBox(centerX - w / 2, centerY - h / 2, w, h, color);
}

void __cdecl DevGui_DrawBevelBox(int32_t x, int32_t y, int32_t w, int32_t h, float shade, const uint8_t *color) 
{
    float v6; // [esp+8h] [ebp-F0h]
    float v7; // [esp+Ch] [ebp-ECh]
    float v8; // [esp+10h] [ebp-E8h]
    float v9; // [esp+14h] [ebp-E4h]
    float v10; // [esp+18h] [ebp-E0h]
    float v11; // [esp+1Ch] [ebp-DCh]
    float v12; // [esp+20h] [ebp-D8h]
    float v13; // [esp+24h] [ebp-D4h]
    float v14; // [esp+28h] [ebp-D0h]
    float v15; // [esp+2Ch] [ebp-CCh]
    float v16; // [esp+30h] [ebp-C8h]
    float v17; // [esp+34h] [ebp-C4h]
    float v18; // [esp+38h] [ebp-C0h]
    float v19; // [esp+3Ch] [ebp-BCh]
    float v20; // [esp+40h] [ebp-B8h]
    float v21; // [esp+44h] [ebp-B4h]
    float v22; // [esp+48h] [ebp-B0h]
    float v23; // [esp+4Ch] [ebp-ACh]
    float v24; // [esp+50h] [ebp-A8h]
    float v25; // [esp+54h] [ebp-A4h]
    float v26; // [esp+58h] [ebp-A0h]
    float v27; // [esp+5Ch] [ebp-9Ch]
    float v28; // [esp+60h] [ebp-98h]
    float v29; // [esp+64h] [ebp-94h]
    float v30; // [esp+68h] [ebp-90h]
    float v31; // [esp+6Ch] [ebp-8Ch]
    float v32; // [esp+70h] [ebp-88h]
    float v33; // [esp+74h] [ebp-84h]
    float v34; // [esp+78h] [ebp-80h]
    float v35; // [esp+7Ch] [ebp-7Ch]
    float v36; // [esp+80h] [ebp-78h]
    float v37; // [esp+84h] [ebp-74h]
    float v38; // [esp+88h] [ebp-70h]
    float v39; // [esp+8Ch] [ebp-6Ch]
    float v40; // [esp+90h] [ebp-68h]
    float v41; // [esp+94h] [ebp-64h]
    float v42; // [esp+98h] [ebp-60h]
    float v43; // [esp+9Ch] [ebp-5Ch]
    float v44; // [esp+A0h] [ebp-58h]
    float v45; // [esp+A4h] [ebp-54h]
    float v46; // [esp+A8h] [ebp-50h]
    float v47; // [esp+ACh] [ebp-4Ch]
    float v48; // [esp+B0h] [ebp-48h]
    float v49; // [esp+B4h] [ebp-44h]
    float v50; // [esp+B8h] [ebp-40h]
    float v51; // [esp+BCh] [ebp-3Ch]
    float v52; // [esp+C0h] [ebp-38h]
    float v53; // [esp+C4h] [ebp-34h]
    int32_t vtxs[4][2]; // [esp+C8h] [ebp-30h] BYREF
    float unpackedColor[4]; // [esp+E8h] [ebp-10h] BYREF

    if ((double)w < 8.0)
        MyAssertHandler((char *)".\\devgui\\devgui_util.cpp", 152, 0, "%s", "w >= (DEVGUI_BEVEL_SIZE * 2.0f)");
    if ((double)h < 8.0)
        MyAssertHandler((char *)".\\devgui\\devgui_util.cpp", 153, 0, "%s", "h >= (DEVGUI_BEVEL_SIZE * 2.0f)");
    Byte4UnpackRgba(color, unpackedColor);
    DevGui_DrawBox(x, y, w, h, color);
    Vec3Scale(unpackedColor, shade, unpackedColor);
    v41 = unpackedColor[0] - 1.0;
    if (v41 < 0.0)
        v53 = unpackedColor[0];
    else
        v53 = 1.0;
    v40 = 0.0 - unpackedColor[0];
    if (v40 < 0.0)
        v39 = v53;
    else
        v39 = 0.0;
    unpackedColor[0] = v39;
    v38 = unpackedColor[1] - 1.0;
    if (v38 < 0.0)
        v52 = unpackedColor[1];
    else
        v52 = 1.0;
    v37 = 0.0 - unpackedColor[1];
    if (v37 < 0.0)
        v36 = v52;
    else
        v36 = 0.0;
    unpackedColor[1] = v36;
    v35 = unpackedColor[2] - 1.0;
    if (v35 < 0.0)
        v51 = unpackedColor[2];
    else
        v51 = 1.0;
    v34 = 0.0 - unpackedColor[2];
    if (v34 < 0.0)
        v33 = v51;
    else
        v33 = 0.0;
    unpackedColor[2] = v33;
    *(_QWORD *)&vtxs[0][0] = __PAIR64__(y, x);
    vtxs[1][0] = x + 4;
    vtxs[1][1] = y + 4;
    vtxs[2][0] = x + 4;
    vtxs[2][1] = y + h - 4;
    vtxs[3][0] = x;
    vtxs[3][1] = h + y;
    DevGui_DrawQuad(vtxs, unpackedColor);
    Vec3Scale(unpackedColor, shade, unpackedColor);
    v32 = unpackedColor[0] - 1.0;
    if (v32 < 0.0)
        v50 = unpackedColor[0];
    else
        v50 = 1.0;
    v31 = 0.0 - unpackedColor[0];
    if (v31 < 0.0)
        v30 = v50;
    else
        v30 = 0.0;
    unpackedColor[0] = v30;
    v29 = unpackedColor[1] - 1.0;
    if (v29 < 0.0)
        v49 = unpackedColor[1];
    else
        v49 = 1.0;
    v28 = 0.0 - unpackedColor[1];
    if (v28 < 0.0)
        v27 = v49;
    else
        v27 = 0.0;
    unpackedColor[1] = v27;
    v26 = unpackedColor[2] - 1.0;
    if (v26 < 0.0)
        v48 = unpackedColor[2];
    else
        v48 = 1.0;
    v25 = 0.0 - unpackedColor[2];
    if (v25 < 0.0)
        v24 = v48;
    else
        v24 = 0.0;
    unpackedColor[2] = v24;
    *(_QWORD *)&vtxs[0][0] = __PAIR64__(y, x);
    vtxs[1][0] = w + x;
    vtxs[1][1] = y;
    vtxs[2][0] = x + w - 4;
    vtxs[2][1] = y + 4;
    vtxs[3][0] = x + 4;
    vtxs[3][1] = y + 4;
    DevGui_DrawQuad(vtxs, unpackedColor);
    Vec3Scale(unpackedColor, shade, unpackedColor);
    v23 = unpackedColor[0] - 1.0;
    if (v23 < 0.0)
        v47 = unpackedColor[0];
    else
        v47 = 1.0;
    v22 = 0.0 - unpackedColor[0];
    if (v22 < 0.0)
        v21 = v47;
    else
        v21 = 0.0;
    unpackedColor[0] = v21;
    v20 = unpackedColor[1] - 1.0;
    if (v20 < 0.0)
        v46 = unpackedColor[1];
    else
        v46 = 1.0;
    v19 = 0.0 - unpackedColor[1];
    if (v19 < 0.0)
        v18 = v46;
    else
        v18 = 0.0;
    unpackedColor[1] = v18;
    v17 = unpackedColor[2] - 1.0;
    if (v17 < 0.0)
        v45 = unpackedColor[2];
    else
        v45 = 1.0;
    v16 = 0.0 - unpackedColor[2];
    if (v16 < 0.0)
        v15 = v45;
    else
        v15 = 0.0;
    unpackedColor[2] = v15;
    vtxs[0][0] = x;
    vtxs[0][1] = h + y;
    vtxs[1][0] = x + 4;
    vtxs[1][1] = y + h - 4;
    vtxs[2][0] = x + w - 4;
    vtxs[2][1] = vtxs[1][1];
    vtxs[3][0] = w + x;
    vtxs[3][1] = h + y;
    DevGui_DrawQuad(vtxs, unpackedColor);
    Vec3Scale(unpackedColor, shade, unpackedColor);
    v14 = unpackedColor[0] - 1.0;
    if (v14 < 0.0)
        v44 = unpackedColor[0];
    else
        v44 = 1.0;
    v13 = 0.0 - unpackedColor[0];
    if (v13 < 0.0)
        v12 = v44;
    else
        v12 = 0.0;
    unpackedColor[0] = v12;
    v11 = unpackedColor[1] - 1.0;
    if (v11 < 0.0)
        v43 = unpackedColor[1];
    else
        v43 = 1.0;
    v10 = 0.0 - unpackedColor[1];
    if (v10 < 0.0)
        v9 = v43;
    else
        v9 = 0.0;
    unpackedColor[1] = v9;
    v8 = unpackedColor[2] - 1.0;
    if (v8 < 0.0)
        v42 = unpackedColor[2];
    else
        v42 = 1.0;
    v7 = 0.0 - unpackedColor[2];
    if (v7 < 0.0)
        v6 = v42;
    else
        v6 = 0.0;
    unpackedColor[2] = v6;
    vtxs[0][0] = w + x;
    vtxs[0][1] = y;
    vtxs[1][0] = w + x;
    vtxs[1][1] = h + y;
    vtxs[2][0] = x + w - 4;
    vtxs[2][1] = y + h - 4;
    vtxs[3][0] = vtxs[2][0];
    vtxs[3][1] = y + 4;
    DevGui_DrawQuad(vtxs, unpackedColor);
}

void __cdecl DevGui_DrawQuad(const int32_t (*vtxs)[2], const float *color)
{
    float xy[4][2]; // [esp+0h] [ebp-20h] BYREF

    xy[0][0] = (float)(*vtxs)[0];
    xy[0][1] = (float)(*vtxs)[1];
    xy[1][0] = (float)(*vtxs)[2];
    xy[1][1] = (float)(*vtxs)[3];
    xy[2][0] = (float)(*vtxs)[4];
    xy[2][1] = (float)(*vtxs)[5];
    xy[3][0] = (float)(*vtxs)[6];
    xy[3][1] = (float)(*vtxs)[7];
    R_AddCmdDrawQuadPic(xy, color, cls.whiteMaterial);
}

void __cdecl DevGui_DrawLine(float *start, float *end, int32_t width, const uint8_t *color)
{
    float h; // [esp+Ch] [ebp-C0h]
    float x; // [esp+2Ch] [ebp-A0h]
    float y; // [esp+30h] [ebp-9Ch]
    float v7; // [esp+38h] [ebp-94h]
    float v8; // [esp+3Ch] [ebp-90h]
    float v9; // [esp+40h] [ebp-8Ch]
    float v10; // [esp+44h] [ebp-88h]
    float v11; // [esp+48h] [ebp-84h]
    float v12; // [esp+4Ch] [ebp-80h]
    float v13; // [esp+50h] [ebp-7Ch]
    float v14; // [esp+54h] [ebp-78h]
    float v15; // [esp+58h] [ebp-74h]
    float v16; // [esp+70h] [ebp-5Ch]
    float v17; // [esp+74h] [ebp-58h]
    float v18; // [esp+78h] [ebp-54h]
    float v19; // [esp+7Ch] [ebp-50h]
    float v20; // [esp+80h] [ebp-4Ch]
    float v21; // [esp+84h] [ebp-48h]
    float v22; // [esp+88h] [ebp-44h]
    float v23; // [esp+8Ch] [ebp-40h]
    float pos; // [esp+90h] [ebp-3Ch]
    float pos_4; // [esp+94h] [ebp-38h]
    float diff[3]; // [esp+98h] [ebp-34h] BYREF
    float tl[2]; // [esp+A4h] [ebp-28h]
    float angle; // [esp+ACh] [ebp-20h]
    float len; // [esp+B0h] [ebp-1Ch]
    float br[2]; // [esp+B4h] [ebp-18h]
    float unpackedColor[4]; // [esp+BCh] [ebp-10h] BYREF

    v22 = *start;
    v23 = *end;
    v15 = v23 - v22;
    if (v15 < 0.0)
        v14 = v23;
    else
        v14 = v22;
    tl[0] = v14;
    v20 = start[1];
    v21 = end[1];
    v13 = v21 - v20;
    if (v13 < 0.0)
        v12 = v21;
    else
        v12 = v20;
    tl[1] = v12;
    v18 = *start;
    v19 = *end;
    v11 = v18 - v19;
    if (v11 < 0.0)
        v10 = v19;
    else
        v10 = v18;
    br[0] = v10;
    v16 = start[1];
    v17 = end[1];
    v9 = v16 - v17;
    if (v9 < 0.0)
        v8 = v17;
    else
        v8 = v16;
    br[1] = v8;
    diff[0] = *end - *start;
    diff[1] = end[1] - start[1];
    diff[2] = 0.0;
    pos = (br[0] - tl[0]) / 2.0 + tl[0];
    pos_4 = (v8 - tl[1]) / 2.0 + tl[1];
    len = Vec2Length(diff);
    Vec3Normalize(diff);
    v7 = acos(diff[0]);
    angle = v7 * 57.2957763671875;
    if (start[1] > (double)end[1])
        angle = -angle;
    while (angle < 0.0)
        angle = angle + 360.0;
    Byte4UnpackRgba(color, unpackedColor);
    h = (float)width;
    y = pos_4 - (double)(width / 2);
    x = pos - len / 2.0;
    R_AddCmdDrawStretchPicRotateXY(x, y, len, h, 0.0, 0.0, 1.0, 1.0, angle, unpackedColor, cls.whiteMaterial);
}

void __cdecl DevGui_DrawFont(int32_t x, int32_t y, const uint8_t *color, char *text)
{
    float v4; // [esp+0h] [ebp-2Ch]
    float v5; // [esp+4h] [ebp-28h]
    float unpackedColor[4]; // [esp+1Ch] [ebp-10h] BYREF

    if (!text)
        MyAssertHandler(".\\devgui\\devgui_util.cpp", 290, 0, "%s", "text");
    if (*text)
    {
        Byte4UnpackRgba(color, unpackedColor);
        v5 = (float)(y + DevGui_GetFontHeight());
        v4 = (float)x;
        R_AddCmdDrawText(text, 0x7FFFFFFF, cls.consoleFont, v4, v5, 1.0, 1.0, 0.0, unpackedColor, 0);
    }
}

int32_t __cdecl DevGui_GetFontWidth(const char *text)
{
    return R_TextWidth(text, 0, cls.consoleFont);
}

int32_t __cdecl DevGui_GetFontHeight()
{
    return R_TextHeight(cls.consoleFont);
}

