#pragma once
#include "r_gfx.h"

struct DiskGfxReflectionProbe11 // sizeof=0x20004
{
    float origin[3];
    unsigned __int8 pixels[131064];
};

struct DiskGfxReflectionProbe // sizeof=0x20044
{                                       // ...
    float origin[3];                    // ...
    char colorCorrectionFilename[64];   // ...
    unsigned __int8 pixels[131064];     // ...
};

struct ColorCorrectionData // sizeof=0x54
{                                       // ...
    char name[64];
    float black_level;
    float white_level;
    float gamma;
    float saturation;
    float range;
};

void __cdecl R_GenerateReflections(char *mapname, GfxReflectionProbe *probes, unsigned int probeCount);
void __cdecl R_BspGenerateReflections();
void __cdecl R_GenerateReflectionImages(
    GfxReflectionProbe *probes,
    const DiskGfxReflectionProbe *probeRawData,
    int probeCount,
    int probeBaseIndex);


// r_reflection_probe_load_obj
void __cdecl R_LoadReflectionProbes(unsigned int bspVersion);
void R_CreateDefaultProbe();
void R_CreateDefaultProbes();