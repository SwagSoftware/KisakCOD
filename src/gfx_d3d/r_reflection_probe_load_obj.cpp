#include "r_reflection_probe.h"
#include "r_bsp.h"
#include <qcommon/com_bsp.h>

void R_CreateDefaultProbes()
{
    s_world.reflectionProbeCount = 1;
    s_world.reflectionProbes = (GfxReflectionProbe *)Hunk_Alloc(0x10u, "R_CreateDefaultProbe", 20);
    s_world.reflectionProbeTextures = (GfxTexture *)Hunk_Alloc(4u, "R_CreateDefaultProbe", 20);
    R_CreateDefaultProbe();
    rgl.reflectionProbesLoaded = 1;
}

void R_CreateDefaultProbe()
{
    DiskGfxReflectionProbe probeRawData; // [esp+4h] [ebp-20058h] BYREF
    unsigned __int8 *pixels; // [esp+20054h] [ebp-8h]
    int v2; // [esp+20058h] [ebp-4h]

    probeRawData.origin[0] = 0.0;
    probeRawData.origin[1] = 0.0;
    probeRawData.origin[2] = 0.0;
    memset(probeRawData.colorCorrectionFilename, 0, sizeof(probeRawData.colorCorrectionFilename));
    pixels = probeRawData.pixels;
    v2 = 0;
    while (v2 != 131064)
    {
        *pixels = -65536;
        v2 += 4;
        pixels += 4;
    }
    R_GenerateReflectionImages(s_world.reflectionProbes, &probeRawData, 1, 0);
}

void __cdecl R_LoadReflectionProbes(unsigned int bspVersion)
{
    char *v1; // eax
    DiskGfxReflectionProbe *v2; // ecx
    const DiskGfxReflectionProbe *reflectionProbeRawData; // [esp+0h] [ebp-10h]
    unsigned int i; // [esp+4h] [ebp-Ch]
    DiskGfxReflectionProbe *reflectionProbeRawData12; // [esp+8h] [ebp-8h]
    char *reflectionProbeRawData11; // [esp+Ch] [ebp-4h]

    if (bspVersion > 0xB)
    {
        reflectionProbeRawData = (const DiskGfxReflectionProbe * )Com_GetBspLump(LUMP_REFLECTION_PROBES, 0x20044u, &s_world.reflectionProbeCount);
        ++s_world.reflectionProbeCount;
        s_world.reflectionProbes = (GfxReflectionProbe*)Hunk_Alloc(16 * s_world.reflectionProbeCount, "R_LoadReflectionProbes", 20);
        s_world.reflectionProbeTextures = (GfxTexture*)Hunk_Alloc(4 * s_world.reflectionProbeCount, "R_LoadReflectionProbes", 20);
        R_CreateDefaultProbe();
        R_GenerateReflectionImages(
            s_world.reflectionProbes + 1,
            reflectionProbeRawData,
            s_world.reflectionProbeCount - 1,
            1);
    }
    else
    {
        reflectionProbeRawData11 = Com_GetBspLump(LUMP_REFLECTION_PROBES, 0x20004u, &s_world.reflectionProbeCount);
        ++s_world.reflectionProbeCount;
        s_world.reflectionProbes = (GfxReflectionProbe *)Hunk_Alloc(16 * s_world.reflectionProbeCount, "R_LoadReflectionProbes", 20);
        s_world.reflectionProbeTextures = (GfxTexture *)Hunk_Alloc(4 * s_world.reflectionProbeCount, "R_LoadReflectionProbes", 20);
        reflectionProbeRawData12 = (DiskGfxReflectionProbe*)Hunk_AllocateTempMemory(131140 * s_world.reflectionProbeCount, "R_LoadReflectionProbes");
        for (i = 0; i < s_world.reflectionProbeCount - 1; ++i)
        {
            memset(
                reflectionProbeRawData12[i].colorCorrectionFilename,
                0,
                sizeof(reflectionProbeRawData12[i].colorCorrectionFilename));
            v1 = &reflectionProbeRawData11[131076 * i];
            v2 = &reflectionProbeRawData12[i];
            v2->origin[0] = *v1;
            v2->origin[1] = *(v1 + 1);
            v2->origin[2] = *(v1 + 2);
            memcpy(v2->pixels, v1 + 12, sizeof(v2->pixels));
        }
        R_CreateDefaultProbe();
        R_GenerateReflectionImages(
            s_world.reflectionProbes + 1,
            reflectionProbeRawData12,
            s_world.reflectionProbeCount - 1,
            1);
        Hunk_FreeTempMemory((char*)reflectionProbeRawData12);
    }
    rgl.reflectionProbesLoaded = 1;
}