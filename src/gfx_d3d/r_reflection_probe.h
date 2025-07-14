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

enum CubemapShot : __int32
{                                       // ...
    CUBEMAPSHOT_NONE = 0x0,
    CUBEMAPSHOT_RIGHT = 0x1,
    CUBEMAPSHOT_LEFT = 0x2,
    CUBEMAPSHOT_BACK = 0x3,
    CUBEMAPSHOT_FRONT = 0x4,
    CUBEMAPSHOT_UP = 0x5,
    CUBEMAPSHOT_DOWN = 0x6,
    CUBEMAPSHOT_COUNT = 0x7,
};
inline CubemapShot &operator++(CubemapShot &e) {
    e = static_cast<CubemapShot>(static_cast<int>(e) + 1);
    return e;
}
inline CubemapShot &operator++(CubemapShot &e, int i)
{
    ++e;
    return e;
}

void __cdecl R_GenerateReflections(char *mapname, GfxReflectionProbe *probes, unsigned int probeCount);
void __cdecl R_BspGenerateReflections();
void __cdecl R_GenerateReflectionImages(
    GfxReflectionProbe *probes,
    const DiskGfxReflectionProbe *probeRawData,
    int probeCount,
    int probeBaseIndex);
void __cdecl R_CalcCubeMapViewValues(refdef_s *refdef, CubemapShot cubemapShot, int cubemapSize);


// r_reflection_probe_load_obj
void __cdecl R_LoadReflectionProbes(unsigned int bspVersion);
void R_CreateDefaultProbe();
void R_CreateDefaultProbes();