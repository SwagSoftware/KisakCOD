#include "r_screenshot.h"
#include <qcommon/mem_track.h>
#include "r_init.h"
#include "rb_logfile.h"
#include "r_utils.h"
#include <universal/profile.h>
#include <universal/com_files.h>
#include "r_state.h"
#include "rb_state.h"
#include <qcommon/cmd.h>
#include "r_reflection_probe.h"

#define ratio 4

struct $EF604BEDDA69129AF7FD28DC5064E1AD // sizeof=0x18
{                                       // ...
    unsigned __int8 *pixels[6];         // ...
};
$EF604BEDDA69129AF7FD28DC5064E1AD cubeShotGlob;

struct $8245A080F03119DF8EECD02BF0FDD113 // sizeof=0x48004
{                                       // ...
    float cubeDirs[6][64][64][3];
    bool cubeDirsInited;                // ...
    // padding byte
    // padding byte
    // padding byte
};
$8245A080F03119DF8EECD02BF0FDD113 cubeMapUtilsGlob;

const float cubemapShotAxis[7][3][3] =
{
  { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } },
  { { 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } },
  { { 0.0, 0.0, -1.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } },
  { { 0.0, 1.0, 0.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } },
  { { 0.0, -1.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } },
  { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } },
  { { -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.0, 0.0, 1.0 } }
}; // idb

void __cdecl TRACK_r_screenshot()
{
    track_static_alloc_internal((void *)cubemapShotAxis, 252, "cubemapShotAxis", 18);
    track_static_alloc_internal(&cubeShotGlob, 24, "cubeShotGlob", 18);
}

void __cdecl Image_Blend1x1Faces(unsigned __int8 *(*pixels)[15], int mipLevel)
{
    int i; // [esp+10h] [ebp-10h]
    int face; // [esp+14h] [ebp-Ch]
    int color; // [esp+18h] [ebp-8h]
    float value; // [esp+1Ch] [ebp-4h]
    float valuea; // [esp+1Ch] [ebp-4h]

    for (color = 0; color < 4; ++color)
    {
        value = 0.0;
        for (face = 0; face < 6; ++face)
            value = (&(*pixels)[15 * face])[mipLevel][color] + value;
        valuea = value / 6.0;
        for (i = 0; i < 6; ++i)
            (&(*pixels)[15 * i])[mipLevel][color] = valuea;
    }
}

unsigned __int8 *__cdecl Image_GetCubeCornerPixel(
    unsigned __int8 *facePixels,
    unsigned int coordx,
    int coordy,
    int edgeSize)
{
    const char *v4; // eax
    const char *v6; // eax
    int x; // [esp+8h] [ebp-8h]
    int y; // [esp+Ch] [ebp-4h]

    if (coordx > 1 || coordy != 2 && coordy != 3)
        MyAssertHandler(
            ".\\r_cubemap_utils.cpp",
            372,
            0,
            "%s",
            "( coordx == CUBE_X0 || coordx == CUBE_X1 ) && ( coordy == CUBE_Y0 || coordy == CUBE_Y1 )");
    if (coordx)
    {
        if (coordx != 1)
        {
            if (!alwaysfails)
            {
                v4 = va("Unhandled coordx %d", coordx);
                MyAssertHandler(".\\r_cubemap_utils.cpp", 385, 0, v4);
            }
            return 0;
        }
        x = edgeSize - 1;
    }
    else
    {
        x = 0;
    }
    if (coordy == 2)
    {
        y = 0;
        return &facePixels[4 * x + 4 * y * edgeSize];
    }
    if (coordy == 3)
    {
        y = edgeSize - 1;
        return &facePixels[4 * x + 4 * y * edgeSize];
    }
    if (!alwaysfails)
    {
        v6 = va("Unhandled coordy %d", coordy);
        MyAssertHandler(".\\r_cubemap_utils.cpp", 401, 0, v6);
    }
    return 0;
}

unsigned __int8 *__cdecl Image_GetCubeFaceEdgePixel(
    unsigned __int8 *facePixels,
    int pixel,
    int edgeSize,
    CubeCoord edge)
{
    unsigned __int8 *result; // eax
    const char *v5; // eax

    switch (edge)
    {
    case CUBE_X0:
        result = &facePixels[pixel * 4 * edgeSize];
        break;
    case CUBE_X1:
        result = &facePixels[4 * edgeSize - 4 + 4 * pixel * edgeSize];
        break;
    case CUBE_Y0:
        result = &facePixels[4 * pixel];
        break;
    case CUBE_Y1:
        result = &facePixels[4 * pixel + 4 * edgeSize * (edgeSize - 1)];
        break;
    default:
        if (!alwaysfails)
        {
            v5 = va("Unhandled edge %d", edge);
            MyAssertHandler(".\\r_cubemap_utils.cpp", 328, 0, v5);
        }
        result = 0;
        break;
    }
    return result;
}

void __cdecl Image_BlendCubeCorner(
    unsigned __int8 *facePixels0,
    unsigned __int8 *facePixels1,
    unsigned __int8 *facePixels2,
    int edgeSize,
    CubeCoord coord0x,
    CubeCoord coord0y,
    CubeCoord coord1x,
    CubeCoord coord1y,
    CubeCoord coord2x,
    CubeCoord coord2y)
{
    int color; // [esp+10h] [ebp-10h]
    unsigned __int8 *pixel2; // [esp+14h] [ebp-Ch]
    unsigned __int8 *pixel0; // [esp+18h] [ebp-8h]
    unsigned __int8 *pixel1; // [esp+1Ch] [ebp-4h]

    pixel0 = Image_GetCubeCornerPixel(facePixels0, coord0x, coord0y, edgeSize);
    pixel1 = Image_GetCubeCornerPixel(facePixels1, coord1x, coord1y, edgeSize);
    pixel2 = Image_GetCubeCornerPixel(facePixels2, coord2x, coord2y, edgeSize);
    for (color = 0; color < 4; ++color)
    {
        pixel0[color] = ((pixel2[color] + pixel1[color] + pixel0[color]) / 3.0);
        pixel1[color] = pixel0[color];
        pixel2[color] = pixel0[color];
    }
}

void __cdecl Image_BlendCubeFaceEdge(
    unsigned __int8 *thisFacePixels,
    unsigned __int8 *otherFacePixels,
    int edgeSize,
    CubeCoord thisEdge,
    CubeCoord otherEdge,
    FlipEdge flip)
{
    int color; // [esp+0h] [ebp-14h]
    unsigned __int8 *otherPixel; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int iother; // [esp+Ch] [ebp-8h]
    unsigned __int8 *thisPixel; // [esp+10h] [ebp-4h]

    for (i = 0; i < edgeSize; ++i)
    {
        if (flip == FLIP_EDGE)
            iother = edgeSize - i - 1;
        else
            iother = i;
        thisPixel = Image_GetCubeFaceEdgePixel(thisFacePixels, i, edgeSize, thisEdge);
        otherPixel = Image_GetCubeFaceEdgePixel(otherFacePixels, iother, edgeSize, otherEdge);
        for (color = 0; color < 4; ++color)
        {
            thisPixel[color] = (otherPixel[color] + thisPixel[color]) / 2;
            otherPixel[color] = thisPixel[color];
        }
    }
}

void __cdecl CubeMap_BlendFaceEdges(unsigned __int8 *(*pixels)[15], int mipLevel, int edgeSize)
{
    if (edgeSize == 1)
    {
        Image_Blend1x1Faces(pixels, mipLevel);
    }
    else
    {
        Image_BlendCubeCorner(
            (*pixels)[mipLevel],
            (*pixels)[mipLevel + 60],
            (*pixels)[mipLevel + 30],
            edgeSize,
            CUBE_X0,
            CUBE_Y0,
            CUBE_X1,
            CUBE_Y0,
            CUBE_X1,
            CUBE_Y1);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel],
            (*pixels)[mipLevel + 60],
            (*pixels)[mipLevel + 45],
            edgeSize,
            CUBE_X0,
            CUBE_Y1,
            CUBE_X1,
            CUBE_Y1,
            CUBE_X1,
            CUBE_Y0);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel],
            (*pixels)[mipLevel + 75],
            (*pixels)[mipLevel + 30],
            edgeSize,
            CUBE_X1,
            CUBE_Y0,
            CUBE_X0,
            CUBE_Y0,
            CUBE_X1,
            CUBE_Y0);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel],
            (*pixels)[mipLevel + 75],
            (*pixels)[mipLevel + 45],
            edgeSize,
            CUBE_X1,
            CUBE_Y1,
            CUBE_X0,
            CUBE_Y1,
            CUBE_X1,
            CUBE_Y1);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 60],
            (*pixels)[mipLevel + 30],
            edgeSize,
            CUBE_X1,
            CUBE_Y0,
            CUBE_X0,
            CUBE_Y0,
            CUBE_X0,
            CUBE_Y1);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 60],
            (*pixels)[mipLevel + 45],
            edgeSize,
            CUBE_X1,
            CUBE_Y1,
            CUBE_X0,
            CUBE_Y1,
            CUBE_X0,
            CUBE_Y0);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 75],
            (*pixels)[mipLevel + 30],
            edgeSize,
            CUBE_X0,
            CUBE_Y0,
            CUBE_X1,
            CUBE_Y0,
            CUBE_X0,
            CUBE_Y0);
        Image_BlendCubeCorner(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 75],
            (*pixels)[mipLevel + 45],
            edgeSize,
            CUBE_X0,
            CUBE_Y1,
            CUBE_X1,
            CUBE_Y1,
            CUBE_X0,
            CUBE_Y1);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel], (*pixels)[mipLevel + 60], edgeSize, CUBE_X0, CUBE_X1, DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel], (*pixels)[mipLevel + 75], edgeSize, CUBE_X1, CUBE_X0, DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel], (*pixels)[mipLevel + 30], edgeSize, CUBE_Y0, CUBE_X1, FLIP_EDGE);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel], (*pixels)[mipLevel + 45], edgeSize, CUBE_Y1, CUBE_X1, DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 30],
            edgeSize,
            CUBE_Y0,
            CUBE_X0,
            DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel + 15], (*pixels)[mipLevel + 45], edgeSize, CUBE_Y1, CUBE_X0, FLIP_EDGE);
        Image_BlendCubeFaceEdge(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 60],
            edgeSize,
            CUBE_X1,
            CUBE_X0,
            DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge(
            (*pixels)[mipLevel + 15],
            (*pixels)[mipLevel + 75],
            edgeSize,
            CUBE_X0,
            CUBE_X1,
            DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge(
            (*pixels)[mipLevel + 45],
            (*pixels)[mipLevel + 60],
            edgeSize,
            CUBE_Y0,
            CUBE_Y1,
            DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel + 45], (*pixels)[mipLevel + 75], edgeSize, CUBE_Y1, CUBE_Y1, FLIP_EDGE);
        Image_BlendCubeFaceEdge(
            (*pixels)[mipLevel + 30],
            (*pixels)[mipLevel + 60],
            edgeSize,
            CUBE_Y1,
            CUBE_Y0,
            DONT_FLIP_EDGE);
        Image_BlendCubeFaceEdge((*pixels)[mipLevel + 30], (*pixels)[mipLevel + 75], edgeSize, CUBE_Y0, CUBE_Y0, FLIP_EDGE);
    }
}

void __cdecl R_CubemapShotDownSample(unsigned __int8 *pixels, int baseRes, int downSampleRes)
{
    int i; // [esp+10h] [ebp-34h]
    int colIndex; // [esp+14h] [ebp-30h]
    int subx; // [esp+18h] [ebp-2Ch]
    int suby; // [esp+1Ch] [ebp-28h]
    int x; // [esp+20h] [ebp-24h]
    int y; // [esp+24h] [ebp-20h]
    float total[6]; // [esp+28h] [ebp-1Ch]
    unsigned __int8 *pixel; // [esp+40h] [ebp-4h]

    LODWORD(total[5]) = 4;
    LODWORD(total[4]) = 4;
    iassert( baseRes == ratio * downSampleRes );
    for (y = 0; y < downSampleRes; ++y)
    {
        for (x = 0; x < downSampleRes; ++x)
        {
            total[0] = 0.0;
            total[1] = 0.0;
            total[2] = 0.0;
            total[3] = 0.0;
            for (suby = 0; suby < 4; ++suby)
            {
                for (subx = 0; subx < 4; ++subx)
                {
                    pixel = &pixels[16 * x + 4 * subx + 4 * baseRes * (suby + 4 * y)];
                    for (colIndex = 0; colIndex < 4; ++colIndex)
                        total[colIndex] = pixel[colIndex] + total[colIndex];
                }
            }
            pixel = &pixels[4 * x + 4 * downSampleRes * y];
            for (i = 0; i < 4; ++i)
            {
                total[i] = total[i] / 16.0;
                iassert( total[colIndex] >= 0.0f && total[colIndex] <= 255.0f );
                pixel[i] = total[i];
            }
        }
    }
}

void __cdecl Image_FlipVertically(unsigned __int8 *pic, int size)
{
    int *v2; // ecx
    int t; // [esp+4h] [ebp-1Ch]
    int cache; // [esp+8h] [ebp-18h]
    int s; // [esp+14h] [ebp-Ch]
    int *p; // [esp+1Ch] [ebp-4h]

    for (s = 0; s < size; ++s)
    {
        p = (int*)&pic[4 * s];
        for (t = 0; t < size / 2; ++t)
        {
            v2 = &p[size * (size - 1) - size * t];
            cache = p[size * t];
            p[size * t] = *v2;
            *v2 = cache;
        }
    }
}

void __cdecl Image_FlipDiagonally(unsigned __int8 *pic, int size)
{
    int t; // [esp+0h] [ebp-14h]
    int cache; // [esp+4h] [ebp-10h]
    int s; // [esp+10h] [ebp-4h]

    for (s = 1; s < size; ++s)
    {
        for (t = 0; t < s; ++t)
        {
            cache = *(_DWORD *)&pic[4 * s + 4 * size * t];
            *(_DWORD *)&pic[4 * s + 4 * size * t] = *(_DWORD *)&pic[4 * t + 4 * size * s];
            *(_DWORD *)&pic[4 * t + 4 * size * s] = cache;
        }
    }
}

void __cdecl Image_FlipHorizontally(unsigned __int8 *pic, int size)
{
    int t; // [esp+0h] [ebp-1Ch]
    int cache; // [esp+4h] [ebp-18h]
    int *pb; // [esp+8h] [ebp-14h]
    int *pa; // [esp+10h] [ebp-Ch]
    int s; // [esp+18h] [ebp-4h]

    for (t = 0; t < size; ++t)
    {
        pa = (int*)&pic[4 * size * t];
        pb = &pa[size - 1];
        for (s = 0; s < size / 2; ++s)
        {
            cache = pa[s];
            pa[s] = pb[-s];
            pb[-s] = cache;
        }
    }
}

void __cdecl CubeMap_FlipSides(unsigned __int8 **pic, int size)
{
    Image_FlipDiagonally(*pic, size);
    Image_FlipDiagonally(pic[1], size);
    Image_FlipHorizontally(pic[1], size);
    Image_FlipVertically(pic[1], size);
    Image_FlipVertically(pic[2], size);
    Image_FlipHorizontally(pic[3], size);
    Image_FlipDiagonally(pic[4], size);
    Image_FlipDiagonally(pic[5], size);
}

void __cdecl Image_CubeMapDirRemap(float *dir, int size)
{
    double v2; // st7
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        v2 = dir[i] / (size - 1);
        dir[i] = v2 + v2 - 1.0;
    }
}

void __cdecl Image_CubeMapDir(int face, int u, int v, int size, float *dir)
{
    const char *v5; // eax

    switch (face)
    {
    case 0:
        *dir = (size - 1);
        dir[1] = (size - 1 - v);
        dir[2] = (size - 1 - u);
        break;
    case 1:
        *dir = 0.0;
        dir[1] = (size - 1 - v);
        dir[2] = u;
        break;
    case 2:
        *dir = u;
        dir[1] = (size - 1);
        dir[2] = v;
        break;
    case 3:
        *dir = u;
        dir[1] = 0.0;
        dir[2] = (size - 1 - v);
        break;
    case 4:
        *dir = u;
        dir[1] = (size - 1 - v);
        dir[2] = (size - 1);
        break;
    case 5:
        *dir = (size - 1 - u);
        dir[1] = (size - 1 - v);
        dir[2] = 0.0;
        break;
    default:
        if (!alwaysfails)
        {
            v5 = va("Image_CubeMapDir: unknown face %d", face);
            MyAssertHandler(".\\r_cubemap_utils.cpp", 206, 0, v5);
        }
        break;
    }
    Image_CubeMapDirRemap(dir, size);
    Vec3Normalize(dir);
}

void __cdecl CubeMap_CacheHighMipDirs()
{
    int u; // [esp+0h] [ebp-Ch]
    int v; // [esp+4h] [ebp-8h]
    int face; // [esp+8h] [ebp-4h]

    if (!cubeMapUtilsGlob.cubeDirsInited)
    {
        cubeMapUtilsGlob.cubeDirsInited = 1;
        for (face = 0; face < 6; ++face)
        {
            for (v = 0; v < 64; ++v)
            {
                for (u = 0; u < 64; ++u)
                    Image_CubeMapDir(face, u, v, 64, cubeMapUtilsGlob.cubeDirs[face][v][u]);
            }
        }
    }
}

double __cdecl Image_CubeMipMapBlurDot(int mipSize)
{
    double result; // st7
    const char *v2; // eax
    float v3; // [esp+0h] [ebp-20h]
    float v4; // [esp+4h] [ebp-1Ch]
    float v5; // [esp+8h] [ebp-18h]
    float v6; // [esp+Ch] [ebp-14h]
    float v7; // [esp+10h] [ebp-10h]
    float v8; // [esp+14h] [ebp-Ch]
    float v9; // [esp+18h] [ebp-8h]

    switch (mipSize)
    {
    case 1:
        v4 = cos(3.1415927);
        result = v4;
        break;
    case 2:
        v5 = cos(1.5707964);
        result = v5;
        break;
    case 4:
        v6 = cos(0.78539819);
        result = v6;
        break;
    case 8:
        v7 = cos(0.38397244);
        result = v7;
        break;
    case 16:
        v8 = cos(0.19198622);
        result = v8;
        break;
    case 32:
        v9 = cos(0.08726646);
        result = v9;
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("ERROR: mipSize %d unexpected for cube map", mipSize);
            MyAssertHandler(".\\r_cubemap_utils.cpp", 295, 0, v2);
        }
        v3 = cos(0.08726646);
        result = v3;
        break;
    }
    return result;
}

void __cdecl Image_CubeMapFaceDir(int face, float *dir)
{
    const char *v2; // eax

    switch (face)
    {
    case 0:
        *dir = 1.0;
        dir[1] = 0.0;
        dir[2] = 0.0;
        break;
    case 1:
        *dir = -1.0;
        dir[1] = 0.0;
        dir[2] = 0.0;
        break;
    case 2:
        *dir = 0.0;
        dir[1] = 1.0;
        dir[2] = 0.0;
        break;
    case 3:
        *dir = 0.0;
        dir[1] = -1.0;
        dir[2] = 0.0;
        break;
    case 4:
        *dir = 0.0;
        dir[1] = 0.0;
        dir[2] = 1.0;
        break;
    case 5:
        *dir = 0.0;
        dir[1] = 0.0;
        dir[2] = -1.0;
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("Image_CubeMapDir: unknown face %d", face);
            MyAssertHandler(".\\r_cubemap_utils.cpp", 166, 0, v2);
        }
        break;
    }
}

void __cdecl Image_CalcCubeMipMapTexel32SubSample(
    int face,
    int u,
    int v,
    float *dir,
    unsigned __int8 *(*sourceFacePixels)[15],
    int sourceSize,
    float minDot,
    float *color,
    float *weight)
{
    int i; // [esp+10h] [ebp-1Ch]
    float sourceColor[4]; // [esp+14h] [ebp-18h] BYREF
    float adjustedDot; // [esp+24h] [ebp-8h]
    float dot; // [esp+28h] [ebp-4h]

    dot = Vec3Dot(cubeMapUtilsGlob.cubeDirs[face][v][u], dir);
    if (minDot < dot)
    {
        adjustedDot = dot - minDot;
        for (i = 0; i < 4; ++i)
            sourceColor[i] = (*sourceFacePixels)[15 * face][4 * u + 4 * sourceSize * v + i] / 255.0;
        Vec4Mad(color, adjustedDot, sourceColor, color);
        *weight = *weight + adjustedDot;
    }
}

void __cdecl Image_CalcCubeMipMapTexel32(
    unsigned __int8 *pixel,
    float *dir,
    unsigned __int8 *(*sourceFacePixels)[15],
    int sourceSize,
    float blurDotMin)
{
    int j; // [esp+18h] [ebp-28h]
    int u; // [esp+1Ch] [ebp-24h]
    int v; // [esp+20h] [ebp-20h]
    int face; // [esp+24h] [ebp-1Ch]
    int i; // [esp+28h] [ebp-18h]
    float weight; // [esp+2Ch] [ebp-14h] BYREF
    float color[4]; // [esp+30h] [ebp-10h] BYREF

    weight = 0.0;
    for (i = 0; i < 4; ++i)
        color[i] = 0.0;
    for (face = 0; face < 6; ++face)
    {
        for (v = 0; v < sourceSize; ++v)
        {
            for (u = 0; u < sourceSize; ++u)
                Image_CalcCubeMipMapTexel32SubSample(face, u, v, dir, sourceFacePixels, sourceSize, blurDotMin, color, &weight);
        }
    }
    iassert( weight > 0.0f );
    for (j = 0; j < 4; ++j)
    {
        color[j] = color[j] / weight;
        if (color[j] > 1.0)
            color[j] = 1.0;
        pixel[j] = (color[j] * 255.0);
    }
}

void __cdecl CubeMap_GenerateMipMap32(
    unsigned __int8 *image,
    int size,
    unsigned __int8 *(*sourceFacePixels)[15],
    int sourceSize,
    int face)
{
    float dir[3]; // [esp+4h] [ebp-18h] BYREF
    float blurDotMin; // [esp+10h] [ebp-Ch]
    int u; // [esp+14h] [ebp-8h]
    int v; // [esp+18h] [ebp-4h]
    int sizea; // [esp+28h] [ebp+Ch]

    iassert( size > 1 );
    iassert( sourceSize == CUBE_MAP_HIGH_MIP_SIZE );
    sizea = size >> 1;
    blurDotMin = Image_CubeMipMapBlurDot(sizea);
    if (sizea == 1)
    {
        Image_CubeMapFaceDir(face, dir);
        Image_CalcCubeMipMapTexel32(image, dir, sourceFacePixels, sourceSize, blurDotMin);
    }
    else
    {
        for (v = 0; v < sizea; ++v)
        {
            for (u = 0; u < sizea; ++u)
            {
                Image_CubeMapDir(face, u, v, sizea, dir);
                Image_CalcCubeMipMapTexel32(&image[4 * u + 4 * sizea * v], dir, sourceFacePixels, sourceSize, blurDotMin);
            }
        }
    }
}

void __cdecl R_CreateReflectionRawDataFromCubemapShot(DiskGfxReflectionProbe *probeRawData, int downSampleRes)
{
    int v2; // [esp+0h] [ebp-190h]
    int mip; // [esp+4h] [ebp-18Ch]
    unsigned __int8 *data; // [esp+Ch] [ebp-184h]
    unsigned __int8 *pixels[6][15]; // [esp+10h] [ebp-180h] BYREF
    unsigned __int8 *rawPixels; // [esp+17Ch] [ebp-14h]
    int mipmapLevelSize; // [esp+180h] [ebp-10h]
    int imgIndex; // [esp+184h] [ebp-Ch]
    int scaledSize; // [esp+188h] [ebp-8h]
    int mipLevel; // [esp+18Ch] [ebp-4h]

    iassert( downSampleRes <= gfxMetrics.cubemapShotRes );
    for (imgIndex = 0; imgIndex < 6; ++imgIndex)
    {
        R_CubemapShotDownSample(cubeShotGlob.pixels[imgIndex], gfxMetrics.cubemapShotRes, downSampleRes);
        Image_FlipVertically(cubeShotGlob.pixels[imgIndex], downSampleRes);
    }
    CubeMap_FlipSides(cubeShotGlob.pixels, downSampleRes);
    mipLevel = 0;
    mipmapLevelSize = downSampleRes * 4 * downSampleRes;
    rawPixels = probeRawData->pixels;
    for (imgIndex = 0; imgIndex < 6; ++imgIndex)
    {
        pixels[imgIndex][0] = rawPixels;
        rawPixels += mipmapLevelSize;
        memcpy(pixels[imgIndex][0], cubeShotGlob.pixels[imgIndex], mipmapLevelSize);
    }
    CubeMap_CacheHighMipDirs();
    iassert( downSampleRes == CUBE_MAP_HIGH_MIP_SIZE );
    for (imgIndex = 0; imgIndex < 6; ++imgIndex)
    {
        scaledSize = downSampleRes;
        data = cubeShotGlob.pixels[imgIndex];
        mipLevel = 1;
        do
        {
            CubeMap_GenerateMipMap32(data, scaledSize, pixels, downSampleRes, imgIndex);
            if (scaledSize >> 1 >= 1)
                v2 = scaledSize >> 1;
            else
                v2 = 1;
            scaledSize = v2;
            mipmapLevelSize = v2 * 4 * v2;
            pixels[imgIndex][mipLevel] = rawPixels;
            rawPixels += mipmapLevelSize;
            memcpy(pixels[imgIndex][mipLevel++], data, mipmapLevelSize);
        } while (scaledSize != 1);
    }
    for (mip = 0; mip < mipLevel; ++mip)
        CubeMap_BlendFaceEdges(pixels, mip, downSampleRes >> mip);
    iassert( mipLevel >= 1 && mipLevel < ARRAY_COUNT( pixels[0] ) );
    for (imgIndex = 0; imgIndex < 6; ++imgIndex)
        Z_VirtualFree(cubeShotGlob.pixels[imgIndex]);
    if (rawPixels - probeRawData->pixels != 131064)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            1051,
            1,
            "%s",
            "rawPixels - probeRawData->pixels == sizeof( probeRawData->pixels )");
}

char __cdecl R_GetFrontBufferData(int x, int y, int width, int height, int bytesPerPixel, unsigned __int8 *buffer)
{
    const char *v7; // eax
    const char *v8; // eax
    HRESULT v9; // [esp-4h] [ebp-DCh]
    HRESULT v10; // [esp-4h] [ebp-DCh]
    IDirect3DSurface9 *v11; // [esp+60h] [ebp-78h]
    IDirect3DSurface9 *var; // [esp+64h] [ebp-74h]
    IDirect3DSurface9 *varCopy; // [esp+68h] [ebp-70h]
    tagMONITORINFO monitorInfo; // [esp+6Ch] [ebp-6Ch] BYREF
    HRESULT hr; // [esp+94h] [ebp-44h]
    _D3DLOCKED_RECT lockedRect; // [esp+98h] [ebp-40h] BYREF
    unsigned __int8 *dstPixel; // [esp+A0h] [ebp-38h]
    const unsigned __int8 *srcPixel; // [esp+A4h] [ebp-34h]
    HMONITOR__ *monitor; // [esp+A8h] [ebp-30h]
    int surfHeight; // [esp+ACh] [ebp-2Ch]
    tagRECT sourceRect; // [esp+B0h] [ebp-28h] BYREF
    int surfWidth; // [esp+C0h] [ebp-18h]
    int row; // [esp+C4h] [ebp-14h]
    tagPOINT pt; // [esp+C8h] [ebp-10h] BYREF
    IDirect3DSurface9 *surface; // [esp+D0h] [ebp-8h] BYREF
    int col; // [esp+D4h] [ebp-4h]

    surface = 0;
    pt.x = x;
    pt.y = y;
    if (vidConfig.isFullscreen)
    {
        surfWidth = vidConfig.displayWidth;
        surfHeight = vidConfig.displayHeight;
    }
    else
    {
        monitor = MonitorFromWindow(dx.windows[0].hwnd, 2u);
        monitorInfo.cbSize = 40;
        if (!GetMonitorInfoA(monitor, &monitorInfo))
        {
            Com_PrintError(8, "ERROR: cannot take screenshot: couldn't get screen dimensions\n");
            return 0;
        }
        ClientToScreen(dx.windows[0].hwnd, &pt);
        if (pt.x < monitorInfo.rcMonitor.left
            || pt.y < monitorInfo.rcMonitor.top
            || width + pt.x > monitorInfo.rcMonitor.right
            || height + pt.y > monitorInfo.rcMonitor.bottom)
        {
            Com_PrintError(8, "ERROR: cannot take screenshot: game window is partially off-screen\n");
            return 0;
        }
        surfWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
        surfHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
        pt.x -= monitorInfo.rcMonitor.left;
        pt.y -= monitorInfo.rcMonitor.top;
    }
    //hr = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int, int, int, IDirect3DSurface9 **, _DWORD))dx.device->CreateOffscreenPlainSurface)(
    //    dx.device,
    //    dx.device,
    //    surfWidth,
    //    surfHeight,
    //    21,
    //    3,
    //    &surface,
    //    0);
    hr = dx.device->CreateOffscreenPlainSurface(
        surfWidth,
        surfHeight,
        D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, 0);

    if (hr < 0)
        goto LABEL_12;
    hr = dx.windows[0].swapChain->GetFrontBufferData(surface);
    if (hr < 0)
    {
        do
        {
            if (r_logFile)
            {
                if (r_logFile->current.integer)
                    RB_LogPrint("surface->Release()\n");
            }
            varCopy = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(varCopy, "surface", ".\\r_screenshot.cpp", 327);
        } while (alwaysfails);
    LABEL_12:
        v9 = hr;
        v7 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: couldn't create the off-screen surface: %s (0x%08x)\n", v7, v9);
        return 0;
    }
    sourceRect.left = pt.x;
    sourceRect.right = width + pt.x;
    sourceRect.top = pt.y;
    sourceRect.bottom = height + pt.y;
    hr = surface->LockRect(&lockedRect, &sourceRect, 16u);
    if (hr >= 0)
    {
        srcPixel = (const unsigned __int8 *)lockedRect.pBits;
        dstPixel = buffer;
        iassert( bytesPerPixel == 3 || bytesPerPixel == 4 );
        if (bytesPerPixel == 3)
        {
            for (row = 0; row < height; ++row)
            {
                for (col = 0; col < width; ++col)
                {
                    *dstPixel = *srcPixel;
                    dstPixel[1] = srcPixel[1];
                    dstPixel[2] = srcPixel[2];
                    dstPixel += 3;
                    srcPixel += 4;
                }
                srcPixel += lockedRect.Pitch - 4 * width;
            }
        }
        else if (lockedRect.Pitch == 4 * width)
        {
            PROF_SCOPED("R_memcpy");
            Com_Memcpy((char *)dstPixel, (char *)srcPixel, 4 * height * width);
        }
        else
        {
            for (row = 0; row < height; ++row)
            {
                PROF_SCOPED("R_memcpy");
                Com_Memcpy((char *)dstPixel, (char *)srcPixel, 4 * width);
                dstPixel += 4 * width;
                srcPixel += lockedRect.Pitch;
            }
        }
        surface->UnlockRect();
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("surface->Release()\n");
            v11 = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(v11, "surface", ".\\r_screenshot.cpp", 388);
        } while (alwaysfails);
        return 1;
    }
    else
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("surface->Release()\n");
            var = surface;
            surface = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(var, "surface", ".\\r_screenshot.cpp", 339);
        } while (alwaysfails);
        v10 = hr;
        v8 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: LockRect failed: %s (0x%08x)\n", v8, v10);
        return 0;
    }
}

void __cdecl R_UpsamplePixelData(
    int oldSize,
    int newSize,
    int stride,
    int bytesPerPixel,
    unsigned __int8 *src,
    unsigned __int8 *dst)
{
    int backwardWeight; // [esp+90h] [ebp-18h]
    int nextSample; // [esp+94h] [ebp-14h]
    float colorScale; // [esp+98h] [ebp-10h]
    unsigned __int8 *currSrc; // [esp+9Ch] [ebp-Ch]
    int column; // [esp+A0h] [ebp-8h]
    int forwardWeight; // [esp+A4h] [ebp-4h]
    unsigned __int8 *dsta; // [esp+C4h] [ebp+1Ch]

    iassert( newSize > oldSize );
    nextSample = bytesPerPixel * stride;
    currSrc = &src[bytesPerPixel * stride * (oldSize - 1)];
    dsta = &dst[bytesPerPixel * stride * (newSize - 1)];
    currSrc[bytesPerPixel * stride] = *currSrc;
    currSrc[nextSample + 1] = currSrc[1];
    currSrc[nextSample + 2] = currSrc[2];
    forwardWeight = newSize - oldSize;
    backwardWeight = oldSize + newSize;
    colorScale = 0.5 / (double)newSize;
    for (column = newSize - 1; column >= 0; --column)
    {
        if (currSrc < src)
        {
            dsta[0] = (int)(colorScale * (float)((forwardWeight + backwardWeight) * src[0]));
            dsta[1] = (int)colorScale * (float)((forwardWeight + backwardWeight) * src[1]);
            dsta[2] = (int)(colorScale * (float)((forwardWeight + backwardWeight) * src[2]));
        }
        else
        {
            dsta[0] = (int)(colorScale * (float)(forwardWeight * currSrc[nextSample] + backwardWeight * currSrc[0]));
            dsta[1] = (int)(colorScale * (float)(forwardWeight * currSrc[nextSample + 1] + backwardWeight * currSrc[1]));
            dsta[2] = (int)(colorScale * (float)(forwardWeight * currSrc[nextSample + 2] + backwardWeight * currSrc[2]));
        }
        dsta -= nextSample;
        backwardWeight += 2 * oldSize;
        forwardWeight -= 2 * oldSize;
        if (forwardWeight < 0)
        {
            backwardWeight -= 2 * newSize;
            forwardWeight += 2 * newSize;
            currSrc -= nextSample;
        }
    }
}

void __cdecl R_DownsamplePixelData(
    int oldSize,
    int newSize,
    int stride,
    int bytesPerPixel,
    unsigned __int8 *src,
    unsigned __int8 *dst)
{
    float v6; // [esp+4h] [ebp-54h]
    float v7; // [esp+18h] [ebp-40h]
    float v8; // [esp+2Ch] [ebp-2Ch]
    int nextSample; // [esp+3Ch] [ebp-1Ch]
    float colorScale; // [esp+40h] [ebp-18h]
    int residual; // [esp+44h] [ebp-14h]
    int column; // [esp+48h] [ebp-10h]
    int color; // [esp+4Ch] [ebp-Ch]
    int color_4; // [esp+50h] [ebp-8h]
    int color_4a; // [esp+50h] [ebp-8h]
    int color_8; // [esp+54h] [ebp-4h]
    int color_8a; // [esp+54h] [ebp-4h]

    iassert( newSize < oldSize );
    colorScale = 1.0 / (double)oldSize;
    nextSample = bytesPerPixel * stride;
    residual = newSize;
    for (column = 0; column < newSize; ++column)
    {
        iassert( residual > 0 );
        color = residual * *src;
        color_4 = residual * src[1];
        color_8 = residual * src[2];
        src += nextSample;
        while (newSize + residual - oldSize <= 0)
        {
            color += newSize * *src;
            color_4 += newSize * src[1];
            color_8 += newSize * src[2];
            residual += newSize;
            src += nextSample;
        }
        residual = newSize + residual - oldSize;
        color_4a = color_4 + src[1] * (newSize - residual);
        color_8a = color_8 + src[2] * (newSize - residual);
        dst[0] = (int)(colorScale * (float)(color + *src * (newSize - residual)));
        dst[1] = (int)(colorScale * (float)color_4a);
        dst[2] = (int)(colorScale * (float)color_8a);
        dst += nextSample;
    }
}

void __cdecl R_ResampleImage(
    int oldWidth,
    int oldHeight,
    int newWidth,
    int newHeight,
    int bytesPerPixel,
    unsigned __int8 *data)
{
    unsigned __int8 *src; // [esp+0h] [ebp-10h]
    unsigned __int8 *srca; // [esp+0h] [ebp-10h]
    unsigned __int8 *srcb; // [esp+0h] [ebp-10h]
    unsigned __int8 *srcc; // [esp+0h] [ebp-10h]
    int row; // [esp+4h] [ebp-Ch]
    int rowa; // [esp+4h] [ebp-Ch]
    unsigned __int8 *dst; // [esp+8h] [ebp-8h]
    unsigned __int8 *dsta; // [esp+8h] [ebp-8h]
    unsigned __int8 *dstb; // [esp+8h] [ebp-8h]
    unsigned __int8 *dstc; // [esp+8h] [ebp-8h]
    int col; // [esp+Ch] [ebp-4h]
    int cola; // [esp+Ch] [ebp-4h]

    if (oldWidth <= newWidth)
    {
        if (oldWidth < newWidth)
        {
            srca = &data[bytesPerPixel * oldWidth * (oldHeight - 1)];
            dsta = &data[bytesPerPixel * newWidth * (oldHeight - 1)];
            for (rowa = oldHeight - 1; rowa >= 0; --rowa)
            {
                R_UpsamplePixelData(oldWidth, newWidth, 1, bytesPerPixel, srca, dsta);
                srca -= bytesPerPixel * oldWidth;
                dsta -= bytesPerPixel * newWidth;
            }
        }
    }
    else
    {
        src = data;
        dst = data;
        for (row = 0; row < oldHeight; ++row)
        {
            R_DownsamplePixelData(oldWidth, newWidth, 1, bytesPerPixel, src, dst);
            src += bytesPerPixel * oldWidth;
            dst += bytesPerPixel * newWidth;
        }
    }
    if (oldHeight <= newHeight)
    {
        if (oldHeight < newHeight)
        {
            srcc = &data[bytesPerPixel * (newWidth - 1)];
            dstc = srcc;
            for (cola = newWidth - 1; cola >= 0; --cola)
            {
                R_UpsamplePixelData(oldHeight, newHeight, newWidth, bytesPerPixel, srcc, dstc);
                srcc -= bytesPerPixel;
                dstc -= bytesPerPixel;
            }
        }
    }
    else
    {
        srcb = data;
        dstb = data;
        for (col = 0; col < newWidth; ++col)
        {
            R_DownsamplePixelData(oldHeight, newHeight, newWidth, bytesPerPixel, srcb, dstb);
            srcb += bytesPerPixel;
            dstb += bytesPerPixel;
        }
    }
}

unsigned __int8 *__cdecl R_TakeResampledScreenshot(int width, int height, int bytesPerPixel, int headerSize)
{
    unsigned int displayHeight; // [esp+0h] [ebp-14h]
    unsigned int displayWidth; // [esp+4h] [ebp-10h]
    unsigned __int8 *buffer; // [esp+8h] [ebp-Ch]

    if (width < (int)vidConfig.displayWidth)
        displayWidth = vidConfig.displayWidth;
    else
        displayWidth = width;
    if (height < (int)vidConfig.displayHeight)
        displayHeight = vidConfig.displayHeight;
    else
        displayHeight = height;
    buffer = (unsigned __int8 *)Z_Malloc(headerSize + bytesPerPixel * displayHeight * displayWidth, "R_ScreenShot", 22);
    if (R_GetFrontBufferData(0, 0, vidConfig.displayWidth, vidConfig.displayHeight, bytesPerPixel, buffer))
    {
        R_ResampleImage(vidConfig.displayWidth, vidConfig.displayHeight, width, height, bytesPerPixel, &buffer[headerSize]);
        return buffer;
    }
    else
    {
        Z_Free((char *)buffer, 22);
        return 0;
    }
}

void __cdecl R_LevelShot()
{
    unsigned __int8 *buffer; // [esp+0h] [ebp-10Ch]
    char checkname[260]; // [esp+4h] [ebp-108h] BYREF

    sprintf(checkname, "levelshots/%s.tga", rgp.world->baseName);
    buffer = R_TakeResampledScreenshot(128, 128, 3, 18);
    if (buffer)
    {
        *(unsigned int *)buffer = 0;
        *((unsigned int *)buffer + 1) = 0;
        *((unsigned int *)buffer + 2) = 0;
        *((unsigned int *)buffer + 3) = 0;
        *((_WORD *)buffer + 8) = 0;
        buffer[2] = 2;
        buffer[12] = 0x80;
        buffer[14] = 0x80;
        buffer[16] = 24;
        FS_WriteFile(checkname, (char *)buffer, 0xC012u);
        Z_Free((char *)buffer, 22);
        Com_Printf(8, "Wrote %s\n", checkname);
    }
}

void __cdecl R_SaveJpg(
    char *filename,
    int quality,
    unsigned int image_width,
    unsigned int image_height,
    unsigned __int8 *image_buffer)
{
    // KISAKTODO: Add JPEG
#if 0
    unsigned __int8 *out; // [esp+0h] [ebp-214h]
    unsigned __int8 *row_pointer[1]; // [esp+8h] [ebp-20Ch] BYREF
    jpeg_compress_struct cinfo; // [esp+Ch] [ebp-208h] BYREF
    jpeg_error_mgr jerr; // [esp+17Ch] [ebp-98h] BYREF

    cinfo.err = jpeg_std_error(&jerr, (void (*)(...))ExitJpeg, PrintfJpeg);
    cinfo.alloc.malloc = (void *(__cdecl *)(unsigned int))Z_MallocJpeg;
    cinfo.alloc.free = (void(__cdecl *)(void *, unsigned int))Com_FreeEvent;
    jpeg_CreateCompress((jpeg_common_struct *)&cinfo, 62, 0x170u);
    out = (unsigned __int8 *)Hunk_AllocateTempMemory(3 * image_height * image_width, "SaveJPG");
    jpegDest((jpeg_common_struct *)&cinfo, out, 3 * image_height * image_width);
    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults((jpeg_common_struct *)&cinfo);
    jpeg_set_quality((jpeg_common_struct *)&cinfo, quality, 1u);
    jpeg_start_compress((jpeg_common_struct *)&cinfo, 1u);
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &image_buffer[3 * image_width * cinfo.next_scanline];
        jpeg_write_scanlines((jpeg_common_struct *)&cinfo, row_pointer, 1u);
    }
    jpeg_finish_compress((jpeg_common_struct *)&cinfo);
    FS_WriteFile(filename, (char *)out, hackSize);
    Hunk_FreeTempMemory((char *)out);
    jpeg_destroy_compress((jpeg_common_struct *)&cinfo);
#endif
}

void __cdecl R_SaveGameShot(const char *saveName)
{
    char filename[256]; // [esp+0h] [ebp-110h] BYREF
    unsigned __int8 *pixels; // [esp+104h] [ebp-Ch]
    int width; // [esp+108h] [ebp-8h]
    int height; // [esp+10Ch] [ebp-4h]

    width = 512;
    height = 512;
    sprintf(filename, "%s.jpg", saveName);
    pixels = R_TakeResampledScreenshot(width, height, 3, 0);
    if (pixels)
    {
        R_SaveJpg(filename, 90, width, height, pixels);
        Z_Free((char *)pixels, 22);
    }
}

void __cdecl R_BeginCubemapShot(int pixelWidthHeight, int pixelBorder)
{
    if (pixelWidthHeight <= 0)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            909,
            0,
            "%s\n\t(pixelWidthHeight) = %i",
            "(pixelWidthHeight > 0)",
            pixelWidthHeight);
    if (pixelWidthHeight >= 0x10000)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            910,
            0,
            "%s\n\t(pixelWidthHeight) = %i",
            "(pixelWidthHeight < 65536)",
            pixelWidthHeight);
    iassert( (pixelBorder >= 0) );
    if (pixelBorder >= pixelWidthHeight)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            912,
            0,
            "%s\n\t(pixelBorder) = %i",
            "(pixelBorder < pixelWidthHeight)",
            pixelBorder);
    gfxMetrics.cubemapShotRes = pixelWidthHeight;
    gfxMetrics.cubemapShotPixelBorder = pixelBorder;
    R_CubemapShotSetInitialState();
}

void R_CubemapShotSetInitialState()
{
    const char *v0; // eax
    int hr; // [esp+8h] [ebp-4h]

    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "dx.device->Clear( 0, 0, 0x00000001l | 0x00000002l | 0x00000004l, ((D3DCOLOR)((((255)&0xff)<<24)|(((255)&0xff)<<1"
                "6)|(((0)&0xff)<<8)|((255)&0xff))), ((123987 / (((((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123987 / (123"
                "987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0 || 0 || (12"
                "3987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 / (123987))) ? (123987) : (-123987)) * (((123987 / ("
                "(((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123"
                "987)) == 123987 / (123987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 /"
                " (123987))) == 0 || ((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) ? (123987) : (-123987)"
                ") * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 123987 "
                "/ (123987))) == 1))) == 123987 / (123987))) == 1))) == 123987 / (123987)) ? 0.0f : 1.0f), 0 )\n");
        //hr = ((int(__stdcall *)(IDirect3DDevice9 *, unsigned int, unsigned int, int, int, unsigned int, unsigned int))dx.device->Clear)(
        //    dx.device,
        //    0,
        //    0,
        //    7,
        //    -65281,
        //    1.0,
        //    0);
        hr = dx.device->Clear(0, 0, 7, -65281, 1.0f, 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v0 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_screenshot.cpp (%i) dx.device->Clear( 0, 0, 0x00000001l | 0x00000002l | 0x00000004l, ((D3DCOLOR)((((255)&"
                    "0xff)<<24)|(((255)&0xff)<<16)|(((0)&0xff)<<8)|((255)&0xff))), ((123987 / (((((123987 / ((((0 || 0 || (123987 /"
                    " ((-123987)) == 123987 / (123987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (1"
                    "23987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 / (123987))) ? (123"
                    "987) : (-123987)) * (((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) ? (123987) : (-1239"
                    "87)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0 || 0 || (123987 / ((-123987)) == 12"
                    "3987 / (123987))) == 1))) == 123987 / (123987))) == 0 || ((123987 / ((((0 || 0 || (123987 / ((-123987)) == 123"
                    "987 / (123987))) ? (123987) : (-123987)) * ((0 || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 0 || (0"
                    " || 0 || (123987 / ((-123987)) == 123987 / (123987))) == 1))) == 123987 / (123987))) == 1))) == 123987 / (1239"
                    "87)) ? 0.0f : 1.0f), 0 ) failed: %s\n",
                    895,
                    v0);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_EndCubemapShot(CubemapShot shotIndex)
{
    R_CopyCubemapShot(shotIndex);
    R_CubemapShotRestoreState();
}

void __cdecl R_CopyCubemapShot(CubemapShot imgIndex)
{
    int sizeInBytes; // [esp+4h] [ebp-4h]

    if (imgIndex <= CUBEMAPSHOT_NONE || imgIndex >= CUBEMAPSHOT_COUNT)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            854,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
            imgIndex);
    sizeInBytes = 4 * gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes;
    cubeShotGlob.pixels[imgIndex - 1] = (unsigned __int8 *)Z_VirtualAlloc(sizeInBytes, "R_CopyCubemapShot", 22);
    iassert( cubeShotGlob.pixels[imgIndex] );
    R_CubemapShotCopySurfaceToBuffer(cubeShotGlob.pixels[imgIndex - 1], sizeInBytes);
}

char __cdecl R_GetBackBufferData(int x, int y, int width, int height, int bytesPerPixel, unsigned __int8 *buffer)
{
    const char *v6; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    IDirect3DSurface9 *var; // [esp+60h] [ebp-5Ch]
    IDirect3DSurface9 *varCopy; // [esp+64h] [ebp-58h]
    int hr; // [esp+68h] [ebp-54h]
    int hra; // [esp+68h] [ebp-54h]
    int hrb; // [esp+68h] [ebp-54h]
    int hrc; // [esp+68h] [ebp-54h]
    int hrd; // [esp+68h] [ebp-54h]
    _D3DLOCKED_RECT lockedRect; // [esp+6Ch] [ebp-50h] BYREF
    unsigned __int8 *dstPixel; // [esp+74h] [ebp-48h]
    IDirect3DSurface9 *surfaceBackBuffer; // [esp+78h] [ebp-44h] BYREF
    const unsigned __int8 *srcPixel; // [esp+7Ch] [ebp-40h]
    tagRECT sourceRect; // [esp+80h] [ebp-3Ch] BYREF
    int row; // [esp+90h] [ebp-2Ch]
    IDirect3DSurface9 *surface; // [esp+94h] [ebp-28h] BYREF
    _D3DSURFACE_DESC desc; // [esp+98h] [ebp-24h] BYREF
    int col; // [esp+B8h] [ebp-4h]

    surfaceBackBuffer = 0;
    surface = 0;
    hr = dx.device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surfaceBackBuffer);
    if (hr >= 0)
    {
        hra = surfaceBackBuffer->GetDesc(&desc);
        if (hra >= 0)
        {
            //hrb = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, unsigned int, _D3DFORMAT, int, IDirect3DSurface9 **, _DWORD))dx.device->CreateOffscreenPlainSurface)(
            //    dx.device,
            //    dx.device,
            //    desc.Width,
            //    desc.Height,
            //    desc.Format,
            //    2,
            //    &surface,
            //    0);
            hrb = dx.device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &surface, 0);
            if (hrb >= 0)
            {
                //hrc = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, IDirect3DSurface9 *, IDirect3DSurface9 *))dx.device->GetRenderTargetData)(
                //    dx.device,
                //    dx.device,
                //    surfaceBackBuffer,
                //    surface);
                hrc = dx.device->GetRenderTargetData(surfaceBackBuffer, surface);
                if (hrc >= 0)
                {
                    sourceRect.left = x;
                    sourceRect.right = width + x;
                    sourceRect.top = y;
                    sourceRect.bottom = height + y;
                    hrd = surface->LockRect(&lockedRect, &sourceRect, 16u);
                    if (hrd >= 0)
                    {
                        srcPixel = (const unsigned __int8 *)lockedRect.pBits;
                        dstPixel = buffer;
                        iassert( bytesPerPixel == 3 || bytesPerPixel == 4 );
                        if (bytesPerPixel == 3)
                        {
                            for (row = 0; row < height; ++row)
                            {
                                for (col = 0; col < width; ++col)
                                {
                                    *dstPixel = *srcPixel;
                                    dstPixel[1] = srcPixel[1];
                                    dstPixel[2] = srcPixel[2];
                                    dstPixel += 3;
                                    srcPixel += 4;
                                }
                                srcPixel += lockedRect.Pitch - 4 * width;
                            }
                        }
                        else if (lockedRect.Pitch == 4 * width)
                        {
                            PROF_SCOPED("R_memcpy");
                            Com_Memcpy((char *)dstPixel, (char *)srcPixel, 4 * height * width);
                        }
                        else
                        {
                            for (row = 0; row < height; ++row)
                            {
                                PROF_SCOPED("R_memcpy");
                                Com_Memcpy((char *)dstPixel, (char *)srcPixel, 4 * width);
                                dstPixel += 4 * width;
                                srcPixel += lockedRect.Pitch;
                            }
                        }
                        surface->UnlockRect();
                        surfaceBackBuffer->Release();
                        do
                        {
                            if (r_logFile && r_logFile->current.integer)
                                RB_LogPrint("surface->Release()\n");
                            var = surface;
                            surface = 0;
                            R_ReleaseAndSetNULL<IDirect3DDevice9>(var, "surface", ".\\r_screenshot.cpp", 494);
                        } while (alwaysfails);
                        return 1;
                    }
                    else
                    {
                        v11 = R_ErrorDescription(hrd);
                        Com_PrintError(8, "ERROR: cannot take screenshot: LockRect failed: %s (0x%08x)\n", v11, hrd);
                        do
                        {
                            if (r_logFile)
                            {
                                if (r_logFile->current.integer)
                                    RB_LogPrint("surface->Release()\n");
                            }
                            varCopy = surface;
                            surface = 0;
                            R_ReleaseAndSetNULL<IDirect3DDevice9>(varCopy, "surface", ".\\r_screenshot.cpp", 447);
                        } while (alwaysfails);
                        return 0;
                    }
                }
                else
                {
                    v10 = R_ErrorDescription(hrc);
                    Com_PrintError(8, "ERROR: cannot take screenshot: GetRenderTargetData failed: %s (0x%08x)\n", v10, hrc);
                    surfaceBackBuffer->Release();
                    return 0;
                }
            }
            else
            {
                v9 = R_ErrorDescription(hrb);
                Com_PrintError(
                    8,
                    "ERROR: cannot take screenshot: couldn't create the off-screen surface: %s (0x%08x)\n",
                    v9,
                    hrb);
                surfaceBackBuffer->Release();
                return 0;
            }
        }
        else
        {
            v8 = R_ErrorDescription(hra);
            Com_PrintError(8, "ERROR: cannot take screenshot: couldn't get desc: %s (0x%08x)\n", v8, hra);
            surfaceBackBuffer->Release();
            return 0;
        }
    }
    else
    {
        v6 = R_ErrorDescription(hr);
        Com_PrintError(8, "ERROR: cannot take screenshot: couldn't get back buffer surface: %s (0x%08x)\n", v6, hr);
        surfaceBackBuffer->Release();
        return 0;
    }
}

void __cdecl R_CubemapShotFlipVerticalBuffer(unsigned __int8 *buffer)
{
    int srcIndex; // [esp+0h] [ebp-14h]
    unsigned __int8 swapBuffer[4]; // [esp+4h] [ebp-10h]
    int colIndex; // [esp+8h] [ebp-Ch]
    int rowIndex; // [esp+Ch] [ebp-8h]
    int dstIndex; // [esp+10h] [ebp-4h]

    iassert( buffer );
    for (rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes / 2; ++rowIndex)
    {
        for (colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex)
        {
            srcIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes);
            dstIndex = 4 * (colIndex + gfxMetrics.cubemapShotRes * (gfxMetrics.cubemapShotRes - 1 - rowIndex));
            *(_DWORD *)swapBuffer = *(_DWORD *)&buffer[srcIndex];
            *(_DWORD *)&buffer[srcIndex] = *(_DWORD *)&buffer[dstIndex];
            *(_DWORD *)&buffer[dstIndex] = *(_DWORD *)swapBuffer;
        }
    }
}

void __cdecl R_CubemapShotCopySurfaceToBuffer(unsigned __int8 *buffer, int bufferSizeInBytes)
{
    iassert( buffer );
    if (bufferSizeInBytes <= 0)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            770,
            0,
            "%s\n\t(bufferSizeInBytes) = %i",
            "(bufferSizeInBytes > 0)",
            bufferSizeInBytes);
    R_GetBackBufferData(
        gfxMetrics.cubemapShotPixelBorder,
        gfxMetrics.cubemapShotPixelBorder,
        gfxMetrics.cubemapShotRes,
        gfxMetrics.cubemapShotRes,
        4,
        buffer);
    R_CubemapShotFlipVerticalBuffer(buffer);
    if (vidConfig.deviceSupportsGamma)
        R_GammaCorrect(buffer, bufferSizeInBytes);
}

void R_CubemapShotRestoreState()
{
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
}

void __cdecl R_SaveCubemapShot(char *filename, CubemapShot shotIndex, float n0, float n1)
{
    R_CubemapShotWriteTargaFile(filename, shotIndex, n0, n1);
}

void __cdecl R_CubemapShotWriteTargaFile(char *filename, CubemapShot shotIndex, float n0, float n1)
{
    int imgIndex; // [esp+Ch] [ebp-Ch]
    int fileSize; // [esp+10h] [ebp-8h]
    unsigned __int8 *targa; // [esp+14h] [ebp-4h]

    iassert( filename );
    if (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            872,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    imgIndex = shotIndex - 1;
    fileSize = 4 * gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes + 18;
    targa = (unsigned __int8 *)Z_VirtualAlloc(fileSize, "R_CubemapShotWriteTargaFile", 22);
    iassert( targa );
    R_CubemapShotWriteTargaHeader(gfxMetrics.cubemapShotRes, targa);
    R_CubemapShotCopyBufferToTarga(cubeShotGlob.pixels[imgIndex], targa);
    R_CubemapShotApplyFresnelToTarga(shotIndex, n0, n1, targa);
    FS_WriteFile(filename, (char *)targa, fileSize);
    Z_VirtualFree(targa);
    Z_VirtualFree(cubeShotGlob.pixels[imgIndex]);
}

void __cdecl R_CubemapShotWriteTargaHeader(int res, unsigned __int8 *fileBuffer)
{
    iassert( fileBuffer );
    iassert( (res > 0) );
    *(unsigned int *)fileBuffer = 0;
    *((unsigned int *)fileBuffer + 1) = 0;
    *((unsigned int *)fileBuffer + 2) = 0;
    *((unsigned int *)fileBuffer + 3) = 0;
    *((_WORD *)fileBuffer + 8) = 0;
    fileBuffer[2] = 2;
    *((_WORD *)fileBuffer + 6) = res;
    fileBuffer[14] = fileBuffer[12];
    fileBuffer[15] = fileBuffer[13];
    fileBuffer[16] = 32;
}

void __cdecl R_CubemapShotCopyBufferToTarga(const unsigned __int8 *srcBuffer, unsigned __int8 *targa)
{
    int srcIndex; // [esp+0h] [ebp-10h]
    int colIndex; // [esp+4h] [ebp-Ch]
    int rowIndex; // [esp+8h] [ebp-8h]
    int dstIndex; // [esp+Ch] [ebp-4h]

    iassert( srcBuffer );
    iassert( targa );
    for (rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes; ++rowIndex)
    {
        for (colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex)
        {
            srcIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes);
            dstIndex = srcIndex + 18;
            targa[dstIndex + 2] = srcBuffer[srcIndex + 2];
            targa[dstIndex + 1] = srcBuffer[srcIndex + 1];
            targa[dstIndex] = srcBuffer[srcIndex];
            targa[dstIndex + 3] = srcBuffer[srcIndex + 3];
        }
    }
}

void __cdecl R_CubemapShotApplyFresnelToTarga(CubemapShot shotIndex, float n0, float n1, unsigned __int8 *targa)
{
    int colIndex; // [esp+8h] [ebp-10h]
    int rowIndex; // [esp+Ch] [ebp-Ch]
    int dstIndex; // [esp+10h] [ebp-8h]

    iassert( targa );
    if (shotIndex <= CUBEMAPSHOT_NONE || shotIndex >= CUBEMAPSHOT_COUNT)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            835,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE && shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    for (rowIndex = 0; rowIndex < gfxMetrics.cubemapShotRes; ++rowIndex)
    {
        for (colIndex = 0; colIndex < gfxMetrics.cubemapShotRes; ++colIndex)
        {
            dstIndex = 4 * (colIndex + rowIndex * gfxMetrics.cubemapShotRes) + 18;
            targa[dstIndex + 3] = R_CubemapShotCalcReflectionFactor(shotIndex, colIndex, rowIndex, n0, n1);
        }
    }
}

unsigned __int8 __cdecl R_CubemapShotCalcReflectionFactor(
    int shotIndex,
    int colIndex,
    int rowIndex,
    float n0,
    float n1)
{
    float v6; // [esp+Ch] [ebp-48h]
    float v7; // [esp+14h] [ebp-40h]
    float scale; // [esp+1Ch] [ebp-38h]
    float v9; // [esp+28h] [ebp-2Ch]
    float dir[3]; // [esp+44h] [ebp-10h] BYREF
    float refraction; // [esp+50h] [ebp-4h]

    if (shotIndex <= 0)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            719,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex > CUBEMAPSHOT_NONE)",
            shotIndex);
    if (shotIndex >= 7)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            720,
            0,
            "%s\n\t(shotIndex) = %i",
            "(shotIndex < CUBEMAPSHOT_COUNT)",
            shotIndex);
    iassert( (colIndex >= 0) );
    if (colIndex >= gfxMetrics.cubemapShotRes)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            722,
            0,
            "%s\n\t(colIndex) = %i",
            "(colIndex < gfxMetrics.cubemapShotRes)",
            colIndex);
    iassert( (rowIndex >= 0) );
    if (rowIndex >= gfxMetrics.cubemapShotRes)
        MyAssertHandler(
            ".\\r_screenshot.cpp",
            724,
            0,
            "%s\n\t(rowIndex) = %i",
            "(rowIndex < gfxMetrics.cubemapShotRes)",
            rowIndex);
    iassert( (n0 != 0) );
    iassert( (n1 != 0) );
    scale = (double)gfxMetrics.cubemapShotRes * 0.5;
    Vec3Scale(cubemapShotAxis[shotIndex][0], scale, dir);
    v7 = (double)colIndex - (double)gfxMetrics.cubemapShotRes * 0.5 + 0.5;
    Vec3Mad(dir, v7, cubemapShotAxis[shotIndex][1], dir);
    v6 = rowIndex - gfxMetrics.cubemapShotRes * 0.5 + 0.5;
    Vec3Mad(dir, v6, cubemapShotAxis[shotIndex][2], dir);
    Vec3Normalize(dir);
    refraction = FresnelTerm(n0, n1, dir[2]);
    return (int)(refraction * 255.0f);
}

void __cdecl R_CubemapLightingForDir(
    float (**linearColors)[3],
    int width,
    int height,
    const float *dir,
    const float *baseColor,
    unsigned __int8 *pixel)
{
    float v6; // [esp+Ch] [ebp-A4h]
    float v7; // [esp+10h] [ebp-A0h]
    float v8; // [esp+14h] [ebp-9Ch]
    float v9; // [esp+18h] [ebp-98h]
    float v10; // [esp+1Ch] [ebp-94h]
    float v11; // [esp+20h] [ebp-90h]
    float v12; // [esp+24h] [ebp-8Ch]
    float v13; // [esp+28h] [ebp-88h]
    float v14; // [esp+2Ch] [ebp-84h]
    float v15; // [esp+30h] [ebp-80h]
    float v16; // [esp+34h] [ebp-7Ch]
    float v17; // [esp+38h] [ebp-78h]
    float v18; // [esp+40h] [ebp-70h]
    float v19; // [esp+54h] [ebp-5Ch]
    float v20; // [esp+68h] [ebp-48h]
    int faceIndex; // [esp+84h] [ebp-2Ch]
    int sampleCount; // [esp+88h] [ebp-28h]
    int x; // [esp+8Ch] [ebp-24h]
    int y; // [esp+90h] [ebp-20h]
    float color[3]; // [esp+94h] [ebp-1Ch] BYREF
    float sourceDir[3]; // [esp+A0h] [ebp-10h] BYREF
    float facing; // [esp+ACh] [ebp-4h]

    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    sampleCount = 0;
    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                v17 = ((double)y + 0.5 + (double)y + 0.5) / (double)height - 1.0;
                v16 = ((double)x + 0.5 + (double)x + 0.5) / (double)width - 1.0;
                R_GetDirForCubemapPixel(faceIndex, v16, v17, sourceDir);
                facing = Vec3Dot(dir, sourceDir);
                if (facing > 0.0)
                {
                    Vec3Mad(color, facing, &linearColors[faceIndex][x][3 * width * y], color);
                    ++sampleCount;
                }
            }
        }
    }
    v15 = 1.0 / (double)sampleCount;
    Vec3Scale(color, v15, color);
    v14 = pow(color[0], 0.45454544);
    color[0] = v14;
    v13 = pow(color[1], 0.45454544);
    color[1] = v13;
    v12 = pow(color[2], 0.45454544);
    color[2] = v12;

    Vec3Mul(color, baseColor, color);

    pixel[2] = CLAMP(color[0] * 255.0f, 0.0f, 1.0f);
    pixel[1] = CLAMP(color[1] * 255.0f, 0.0f, 1.0f);
    pixel[0] = CLAMP(color[2] * 255.0f, 0.0f, 1.0f);
    pixel[3] = -1;
}

void __cdecl R_CubemapLighting(
    float (**linearColors)[3],
    int width,
    int height,
    const float *baseColor,
    unsigned __int8 **pixels)
{
    float v5; // [esp+Ch] [ebp-20h]
    float v6; // [esp+10h] [ebp-1Ch]
    float dir[3]; // [esp+14h] [ebp-18h] BYREF
    int faceIndex; // [esp+20h] [ebp-Ch]
    int x; // [esp+24h] [ebp-8h]
    int y; // [esp+28h] [ebp-4h]

    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                v6 = ((double)y + 0.5 + (double)y + 0.5) / (double)height - 1.0;
                v5 = ((double)x + 0.5 + (double)x + 0.5) / (double)width - 1.0;
                R_GetDirForCubemapPixel(faceIndex, v5, v6, dir);
                R_CubemapLightingForDir(linearColors, width, height, dir, baseColor, &pixels[faceIndex][4 * x + 4 * y * width]);
            }
        }
    }
}

void __cdecl R_LightingFromCubemapShots(const float *baseColor)
{
    int pixelsPerFace; // [esp+0h] [ebp-1Ch]
    float (*linearColors[6])[3]; // [esp+4h] [ebp-18h] BYREF

    pixelsPerFace = gfxMetrics.cubemapShotRes * gfxMetrics.cubemapShotRes;
    linearColors[0] = (float (*)[3])Z_VirtualAlloc(72 * pixelsPerFace, "R_LightingFromCubemapShot", 22);
    linearColors[1] = (float (*)[3])linearColors[0][pixelsPerFace];
    linearColors[2] = (float (*)[3])linearColors[1][pixelsPerFace];
    linearColors[3] = (float (*)[3])linearColors[2][pixelsPerFace];
    linearColors[4] = (float (*)[3])linearColors[3][pixelsPerFace];
    linearColors[5] = (float (*)[3])linearColors[4][pixelsPerFace];
    R_CubemapShotExtractLinearLight(
        cubeShotGlob.pixels,
        gfxMetrics.cubemapShotRes,
        gfxMetrics.cubemapShotRes,
        linearColors);
    R_CubemapLighting(linearColors, gfxMetrics.cubemapShotRes, gfxMetrics.cubemapShotRes, baseColor, cubeShotGlob.pixels);
    Z_VirtualFree(linearColors[0]);
}

void __cdecl R_GetDirForCubemapPixel(int faceIndex, float x, float y, float *dir)
{
    Vec3Mad(cubemapShotAxis[faceIndex + 1][0], x, (const float *)(36 * (faceIndex + 1) + 9391748), dir);
    Vec3Mad(dir, y, (const float *)(36 * (faceIndex + 1) + 9391760), dir);
    Vec3Normalize(dir);
}

void __cdecl R_CubemapShotExtractLinearLight(
    unsigned __int8 **pixels,
    int width,
    int height,
    float (**linearColors)[3])
{
    float v4; // [esp+0h] [ebp-40h]
    float v5; // [esp+8h] [ebp-38h]
    float v6; // [esp+10h] [ebp-30h]
    float v7; // [esp+1Ch] [ebp-24h]
    float v8; // [esp+24h] [ebp-1Ch]
    float v9; // [esp+2Ch] [ebp-14h]
    int faceIndex; // [esp+30h] [ebp-10h]
    int x; // [esp+34h] [ebp-Ch]
    int y; // [esp+38h] [ebp-8h]
    int pixelIndex; // [esp+3Ch] [ebp-4h]

    for (faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        pixelIndex = 0;
        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                v9 = (double)pixels[faceIndex][4 * pixelIndex + 2] * 0.003921568859368563;
                v6 = pow(v9, 2.2);
                linearColors[faceIndex][pixelIndex][0] = v6;
                v8 = (double)pixels[faceIndex][4 * pixelIndex + 1] * 0.003921568859368563;
                v5 = pow(v8, 2.2);
                linearColors[faceIndex][pixelIndex][1] = v5;
                v7 = (double)pixels[faceIndex][4 * pixelIndex] * 0.003921568859368563;
                v4 = pow(v7, 2.2);
                linearColors[faceIndex][pixelIndex++][2] = v4;
            }
        }
    }
}

void __cdecl R_ScreenshotFilename(unsigned int lastNumber, const char *extension, char *fileName)
{
    if (lastNumber < 0x2710)
        Com_sprintf(fileName, 0x100u, "screenshots/shot%04i.%s", lastNumber, extension);
    else
        Com_sprintf(fileName, 0x100u, "screenshots/shot9999.%s", extension);
}

void __cdecl R_TakeScreenshotJpg(int x, int y, int width, int height, const char *filename)
{
    unsigned __int8 *buffer; // [esp+0h] [ebp-4h]

    buffer = (unsigned __int8 *)Z_Malloc(3 * height * width, "R_TakeScreenshotJpg", 22);
    if (R_GetFrontBufferData(x, y, width, height, 3, buffer))
        R_SaveJpg((char*)filename, 90, width, height, buffer);
    Z_Free((char *)buffer, 22);
}

void __cdecl R_TakeScreenshotTga(int x, int y, int width, int height, char *filename)
{
    unsigned __int8 *buffer; // [esp+0h] [ebp-8h]

    buffer = (unsigned __int8 *)Z_Malloc(3 * height * width + 18, "R_TakeScreenshotTga", 22);
    *(_DWORD *)buffer = 0;
    *((_DWORD *)buffer + 1) = 0;
    *((_DWORD *)buffer + 2) = 0;
    *((_DWORD *)buffer + 3) = 0;
    *((_WORD *)buffer + 8) = 0;
    buffer[2] = 2;
    *((_WORD *)buffer + 6) = width;
    *((_WORD *)buffer + 7) = height;
    buffer[16] = 24;
    buffer[17] = 32;
    if (R_GetFrontBufferData(x, y, width, height, 3, buffer + 18))
        FS_WriteFile(filename, (char *)buffer, 3 * height * width + 18);
    Z_Free((char *)buffer, 22);
}

int lastNumber;
void __cdecl R_ScreenshotCommand(GfxScreenshotType type)
{
    const char *v1; // eax
    const char *v2; // eax
    char filename[260]; // [esp+44h] [ebp-110h] BYREF
    int silent; // [esp+14Ch] [ebp-8h]
    const char *extension; // [esp+150h] [ebp-4h]

    if (type)
    {
        if (type != R_SCREENSHOT_TGA)
        {
            if (!alwaysfails)
                MyAssertHandler(".\\r_screenshot.cpp", 651, 1, "unhandled case");
            return;
        }
        extension = "tga";
    }
    else
    {
        extension = "jpg";
    }
    if (!strcmp(Cmd_Argv(1), "levelshot"))
    {
        R_LevelShot();
        return;
    }
    if (!strcmp(Cmd_Argv(1), "savegame") && Cmd_Argc() == 3 && *Cmd_Argv(2))
    {
        v1 = Cmd_Argv(2);
        R_SaveGameShot(v1);
        return;
    }
    silent = strcmp(Cmd_Argv(1), "silent") == 0;
    if (Cmd_Argc() != 2 || silent)
    {
        while (lastNumber <= 9999)
        {
            R_ScreenshotFilename(lastNumber, extension, filename);
            if (!FS_FileExists(filename))
                break;
            ++lastNumber;
        }
        if (lastNumber >= 9999)
        {
            Com_Printf(8, "ScreenShot: Couldn't create a file\n");
            return;
        }
        ++lastNumber;
    }
    else
    {
        v2 = Cmd_Argv(1);
        Com_sprintf(filename, 0x100u, "screenshots/%s.%s", v2, extension);
    }
    if (type)
        R_TakeScreenshotTga(0, 0, vidConfig.displayWidth, vidConfig.displayHeight, filename);
    else
        R_TakeScreenshotJpg(0, 0, vidConfig.displayWidth, vidConfig.displayHeight, filename);
    if (!silent)
        Com_Printf(8, "Wrote %s\n", filename);
}