#pragma once
#include "r_material.h"

#define HCOUNT 4096

struct WaterGlob // sizeof=0x9000
{                                       // ...
    complex_s H[4096];
    unsigned __int8 pixels[4096];       // ...
};

struct WaterGlobStatic // sizeof=0x1C00
{                                       // ...
    float sinTable[1024];               // ...
    complex_s fftTrigTable[256];        // ...
    int fftBitswap[256];                // ...
};

void __cdecl TRACK_r_water();
void __cdecl R_UploadWaterTextureInternal(water_t **data);
void __cdecl WaterFrequenciesAtTime(complex_s *H, const water_t *water, float t);
void __cdecl WaterAmplitudesFromFrequencies(complex_s *H, const water_t *water);
void __cdecl TransposeArray(complex_s *H, unsigned int M);
void __cdecl WaterPixelsFromAmplitudes(GfxColor *pixels, complex_s *H, const water_t *water);
void __cdecl GenerateMipMaps(_D3DFORMAT format, unsigned __int8 *pixels, water_t *water);
void __cdecl R_UploadWaterTexture(water_t *water, float floatTime);
void __cdecl R_InitWater();
void __cdecl Load_PicmipWater(water_t **waterRef);

// r_water_load_obj
void __cdecl R_InitLoadWater();
water_t *__cdecl R_LoadWaterSetup(const water_t *water);
void __cdecl R_ShutdownLoadWater();