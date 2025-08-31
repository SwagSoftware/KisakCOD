#pragma once
#include <bgame/bg_local.h>
#include "r_reflection_probe.h"

#define CUBE_MAP_HIGH_MIP_SIZE 64

enum GfxScreenshotType : __int32
{                                       // ...
    R_SCREENSHOT_JPG = 0x0,
    R_SCREENSHOT_TGA = 0x1,
};

enum CubeCoord : __int32
{                                       // ...
    CUBE_X0 = 0x0,
    CUBE_X1 = 0x1,
    CUBE_Y0 = 0x2,
    CUBE_Y1 = 0x3,
    CUBE_NUM_COORDS = 0x4,
};

enum FlipEdge : __int32
{                                       // ...
    DONT_FLIP_EDGE = 0x0,
    FLIP_EDGE = 0x1,
};

void __cdecl TRACK_r_screenshot();
void __cdecl R_LevelShot();
void __cdecl R_SaveGameShot(const char *saveName);
void __cdecl R_BeginCubemapShot(int pixelWidthHeight, int pixelBorder);
void R_CubemapShotSetInitialState();
void __cdecl R_EndCubemapShot(CubemapShot shotIndex);
void __cdecl R_CopyCubemapShot(CubemapShot shotIndex);
void __cdecl R_CubemapShotCopySurfaceToBuffer(unsigned __int8 *buffer, int bufferSizeInBytes);
void R_CubemapShotRestoreState();
void __cdecl R_SaveCubemapShot(char *filename, CubemapShot shotIndex, float n0, float n1);
void __cdecl R_CubemapShotWriteTargaFile(char *filename, CubemapShot shotIndex, float n0, float n1);
void __cdecl R_CubemapShotWriteTargaHeader(int res, unsigned __int8 *fileBuffer);
void __cdecl R_CubemapShotCopyBufferToTarga(const unsigned __int8 *srcBuffer, unsigned __int8 *targa);
void __cdecl R_CubemapShotApplyFresnelToTarga(CubemapShot shotIndex, float n0, float n1, unsigned __int8 *targa);
unsigned __int8 __cdecl R_CubemapShotCalcReflectionFactor(
    int shotIndex,
    int colIndex,
    int rowIndex,
    float n0,
    float n1);
void __cdecl R_LightingFromCubemapShots(const float *baseColor);
void __cdecl R_GetDirForCubemapPixel(int faceIndex, float x, float y, float *dir);
void __cdecl R_CubemapShotExtractLinearLight(
    unsigned __int8 **pixels,
    int width,
    int height,
    float (**linearColors)[3]);

void __cdecl R_ScreenshotCommand(GfxScreenshotType type);

void __cdecl R_CreateReflectionRawDataFromCubemapShot(struct DiskGfxReflectionProbe *probeRawData, int downSampleRes);