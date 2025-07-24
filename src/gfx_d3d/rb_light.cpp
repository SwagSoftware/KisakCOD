#include "rb_light.h"

#include "r_dvars.h"
#include "r_utils.h"

#include <qcommon/com_bsp.h>
#include <qcommon/threads.h>
#include <universal/com_files.h>
#include <xanim/xmodel.h>
#include "r_xsurface.h"
#include "r_primarylights.h"
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif
#include <cgame/cg_local.h>
#endif

struct // sizeof=0x8
{                                       // ...
    unsigned __int16 (*history)[3];     // ...
    int count;                          // ...
} s_vc_log;

static int s_lightGridRowDelta;
static int s_lightGridSliceDelta;

void __cdecl R_SetLightGridSampleDeltas(int rowStride, int sliceStride)
{
    s_lightGridRowDelta = rowStride;
    s_lightGridSliceDelta = sliceStride - 3 * rowStride;
}

void __cdecl R_ShowLightVisCachePoints(const float *viewOrigin, const DpvsPlane *clipPlanes, int clipPlaneCount)
{
    float *color; // [esp+Ch] [ebp-A4h]
    float v4; // [esp+44h] [ebp-6Ch]
    float v5; // [esp+58h] [ebp-58h]
    float v6; // [esp+6Ch] [ebp-44h]
    int i; // [esp+7Ch] [ebp-34h]
    int spread; // [esp+80h] [ebp-30h]
    float origin[3]; // [esp+84h] [ebp-2Ch] BYREF
    unsigned int z; // [esp+90h] [ebp-20h]
    unsigned int iz; // [esp+94h] [ebp-1Ch]
    unsigned int iy; // [esp+98h] [ebp-18h]
    int dz; // [esp+9Ch] [ebp-14h]
    unsigned int ix; // [esp+A0h] [ebp-10h]
    unsigned int x; // [esp+A4h] [ebp-Ch]
    unsigned int y; // [esp+A8h] [ebp-8h]
    int dy; // [esp+ACh] [ebp-4h]

    iassert(!Sys_IsRenderThread());

    if (s_vc_log.history)
    {
        spread = r_vc_showlog->current.integer;
        if (spread > 0)
        {
            x = (int)(viewOrigin[0] - -131072.0f) >> 5;
            y = (int)(viewOrigin[1] - -131072.0f) >> 5;
            z = (int)(viewOrigin[2] - -131072.0f) >> 6;
            origin[0] = (double)x * 32.0 + -131072.0;
            origin[1] = (double)y * 32.0 + -131072.0;
            origin[2] = (double)z * 64.0 + -131072.0;
            for (dz = -1; dz <= 1; ++dz)
            {
                iz = dz + z;
                if (dz + z <= 0x1000)
                {
                    for (dy = -spread; dy <= spread; ++dy)
                    {
                        iy = dy + y;
                        if (dy + y <= 0x2000)
                        {
                            for (i = -spread; i <= spread; ++i)
                            {
                                ix = i + x;
                                if (i + x <= 0x2000)
                                {
                                    origin[0] = (double)ix * 32.0 + -131072.0;
                                    origin[1] = (double)iy * 32.0 + -131072.0;
                                    origin[2] = (double)iz * 64.0 + -131072.0;
                                    if (!R_CullPointAndRadius(origin, 0.0, clipPlanes, clipPlaneCount)
                                        && R_SortedHistoryEntry(ix, iy, iz, SH_ADD_NEVER) < 0)
                                    {
                                        if ((unsigned int)dz < 2)
                                            color = (float *)colorGreen;
                                        else
                                            color = (float *)colorYellow;
                                        R_AddDebugString(&frontEndDataOut->debugGlobals, origin, color, 1.0, ".");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int __cdecl R_SortedHistoryEntry(int x, int y, int z, GfxSortedHistoryAdd addMode)
{
    int v4; // edx
    int v5; // eax
    int v6; // ecx
    int compare; // [esp+0h] [ebp-10h]
    int top; // [esp+4h] [ebp-Ch]
    int bot; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    top = s_vc_log.count - 1;
    bot = 0;
    while (bot <= top)
    {
        mid = (bot + top) >> 1;
        v4 = s_vc_log.history[mid][0];
        compare = x - v4;
        if (x == v4)
        {
            v5 = s_vc_log.history[mid][1];
            compare = y - v5;
            if (y == v5)
            {
                v6 = s_vc_log.history[mid][2];
                compare = z - v6;
                if (z == v6)
                    return (bot + top) >> 1;
            }
        }
        if (compare >= 0)
            bot = mid + 1;
        else
            top = mid - 1;
    }
    if (bot != top + 1)
        MyAssertHandler(".\\rb_light.cpp", 210, 0, "%s", "bot == top + 1");
    if (addMode && R_AddSortedHistoryEntry(x, y, z, bot))
        return bot;
    else
        return -1;
}

char __cdecl R_AddSortedHistoryEntry(unsigned __int16 x, unsigned __int16 y, unsigned __int16 z, int pos)
{
    if (s_vc_log.count < 0x100000)
    {
        memmove(
            (unsigned __int8 *)s_vc_log.history[pos + 1],
            (unsigned __int8 *)s_vc_log.history[pos],
            6 * (s_vc_log.count - pos));
        s_vc_log.history[pos][0] = x;
        s_vc_log.history[pos][1] = y;
        s_vc_log.history[pos][2] = z;
        ++s_vc_log.count;
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_TOO_MANY_LIGHT_GRID_POINTS);
        return 0;
    }
}

void __cdecl R_SetLightGridColors(
    const GfxLightGridColors *colors,
    unsigned __int8 packedSunWeight,
    unsigned __int8 *pixels_arg)
{
    unsigned int *pixels = (unsigned int *)pixels_arg;

    pixels[0] = (packedSunWeight << 24) | colors->rgb[0][2] | (colors->rgb[0][1] << 8) | (colors->rgb[0][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[1][2] | (colors->rgb[1][1] << 8) | (colors->rgb[1][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[2][2] | (colors->rgb[2][1] << 8) | (colors->rgb[2][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[3][2] | (colors->rgb[3][1] << 8) | (colors->rgb[3][0] << 16);

    pixels = (unsigned int*)((char*)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[4][2] | (colors->rgb[4][1] << 8) | (colors->rgb[4][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[5][2] | (colors->rgb[5][1] << 8) | (colors->rgb[5][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[6][2] | (colors->rgb[6][1] << 8) | (colors->rgb[6][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[7][2] | (colors->rgb[7][1] << 8) | (colors->rgb[7][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[8][2] | (colors->rgb[8][1] << 8) | (colors->rgb[8][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[9][2] | (colors->rgb[9][1] << 8) | (colors->rgb[9][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[10][2] | (colors->rgb[10][1] << 8) | (colors->rgb[10][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[11][2] | (colors->rgb[11][1] << 8) | (colors->rgb[11][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[12][2] | (colors->rgb[12][1] << 8) | (colors->rgb[12][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[13][2] | (colors->rgb[13][1] << 8) | (colors->rgb[13][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[14][2] | (colors->rgb[14][1] << 8) | (colors->rgb[14][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[15][2] | (colors->rgb[15][1] << 8) | (colors->rgb[15][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridSliceDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[16][2] | (colors->rgb[16][1] << 8) | (colors->rgb[16][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[17][2] | (colors->rgb[17][1] << 8) | (colors->rgb[17][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[18][2] | (colors->rgb[18][1] << 8) | (colors->rgb[18][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[19][2] | (colors->rgb[19][1] << 8) | (colors->rgb[19][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[20][2] | (colors->rgb[20][1] << 8) | (colors->rgb[20][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[0][2] | (colors->rgb[0][1] << 8) | (colors->rgb[0][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[3][2] | (colors->rgb[3][1] << 8) | (colors->rgb[3][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[21][2] | (colors->rgb[21][1] << 8) | (colors->rgb[21][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[22][2] | (colors->rgb[22][1] << 8) | (colors->rgb[22][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[12][2] | (colors->rgb[12][1] << 8) | (colors->rgb[12][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[15][2] | (colors->rgb[15][1] << 8) | (colors->rgb[15][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[23][2] | (colors->rgb[23][1] << 8) | (colors->rgb[23][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[24][2] | (colors->rgb[24][1] << 8) | (colors->rgb[24][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[25][2] | (colors->rgb[25][1] << 8) | (colors->rgb[25][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[26][2] | (colors->rgb[26][1] << 8) | (colors->rgb[26][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[27][2] | (colors->rgb[27][1] << 8) | (colors->rgb[27][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridSliceDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[28][2] | (colors->rgb[28][1] << 8) | (colors->rgb[28][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[29][2] | (colors->rgb[29][1] << 8) | (colors->rgb[29][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[30][2] | (colors->rgb[30][1] << 8) | (colors->rgb[30][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[31][2] | (colors->rgb[31][1] << 8) | (colors->rgb[31][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[32][2] | (colors->rgb[32][1] << 8) | (colors->rgb[32][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[40][2] | (colors->rgb[40][1] << 8) | (colors->rgb[40][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[43][2] | (colors->rgb[43][1] << 8) | (colors->rgb[43][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[33][2] | (colors->rgb[33][1] << 8) | (colors->rgb[33][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[52][2] | (colors->rgb[52][1] << 8) | (colors->rgb[52][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[55][2] | (colors->rgb[55][1] << 8) | (colors->rgb[55][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[35][2] | (colors->rgb[35][1] << 8) | (colors->rgb[35][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[36][2] | (colors->rgb[36][1] << 8) | (colors->rgb[36][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[37][2] | (colors->rgb[37][1] << 8) | (colors->rgb[37][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[38][2] | (colors->rgb[38][1] << 8) | (colors->rgb[38][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[39][2] | (colors->rgb[39][1] << 8) | (colors->rgb[39][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridSliceDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[40][2] | (colors->rgb[40][1] << 8) | (colors->rgb[40][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[41][2] | (colors->rgb[41][1] << 8) | (colors->rgb[41][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[42][2] | (colors->rgb[42][1] << 8) | (colors->rgb[42][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[43][2] | (colors->rgb[43][1] << 8) | (colors->rgb[43][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[44][2] | (colors->rgb[44][1] << 8) | (colors->rgb[44][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[45][2] | (colors->rgb[45][1] << 8) | (colors->rgb[45][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[46][2] | (colors->rgb[46][1] << 8) | (colors->rgb[46][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[47][2] | (colors->rgb[47][1] << 8) | (colors->rgb[47][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[48][2] | (colors->rgb[48][1] << 8) | (colors->rgb[48][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[49][2] | (colors->rgb[49][1] << 8) | (colors->rgb[49][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[50][2] | (colors->rgb[50][1] << 8) | (colors->rgb[50][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[51][2] | (colors->rgb[51][1] << 8) | (colors->rgb[51][0] << 16);

    pixels = (unsigned int *)((char *)pixels + s_lightGridRowDelta);
    pixels[0] = (packedSunWeight << 24) | colors->rgb[52][2] | (colors->rgb[52][1] << 8) | (colors->rgb[52][0] << 16);
    pixels[1] = (packedSunWeight << 24) | colors->rgb[53][2] | (colors->rgb[53][1] << 8) | (colors->rgb[53][0] << 16);
    pixels[2] = (packedSunWeight << 24) | colors->rgb[54][2] | (colors->rgb[54][1] << 8) | (colors->rgb[54][0] << 16);
    pixels[3] = (packedSunWeight << 24) | colors->rgb[55][2] | (colors->rgb[55][1] << 8) | (colors->rgb[55][0] << 16);
}

void __cdecl R_FixedPointBlendLightGridColors(
    const GfxLightGrid *lightGrid,
    const unsigned __int16 *colorsIndex,
    unsigned __int16 *fixedPointWeight,
    unsigned int colorsCount,
    GfxLightGridColors *outPacked)
{
    unsigned __int16 accumulated[168]; // [esp+18h] [ebp-158h] BYREF
    unsigned int colorsIter; // [esp+16Ch] [ebp-4h]

    R_ScaleLightGridColors(&lightGrid->colors[*colorsIndex], *fixedPointWeight, accumulated);

    colorsIter = 1;
    do
    {
        R_WeightedAccumulateLightGridColors(&lightGrid->colors[colorsIndex[colorsIter]], fixedPointWeight[colorsIter], accumulated);
        ++colorsIter;
    } while (colorsIter < colorsCount);

    R_PackAccumulatedLightGridColors(accumulated, outPacked);
}

void __cdecl R_ScaleLightGridColors(
    const GfxLightGridColors *colors,
    unsigned __int16 fixedPointWeight,
    unsigned __int16 *scaled)
{
    unsigned int sampleIter; // [esp+4h] [ebp-4h]

    for (sampleIter = 0; sampleIter != 168; sampleIter += 8)
    {
        unsigned char *rgb = ((unsigned char *)&colors->rgb + sampleIter);

        scaled[sampleIter]     = (fixedPointWeight * rgb[0]);
        scaled[sampleIter + 1] = (fixedPointWeight * rgb[1]);
        scaled[sampleIter + 2] = (fixedPointWeight * rgb[2]);
        scaled[sampleIter + 3] = (fixedPointWeight * rgb[3]);
        scaled[sampleIter + 4] = (fixedPointWeight * rgb[4]);
        scaled[sampleIter + 5] = (fixedPointWeight * rgb[5]);
        scaled[sampleIter + 6] = (fixedPointWeight * rgb[6]);
        scaled[sampleIter + 7] = (fixedPointWeight * rgb[7]);
    }
}

void __cdecl R_WeightedAccumulateLightGridColors(
    const GfxLightGridColors *colors,
    unsigned __int16 fixedPointWeight,
    unsigned __int16 *accumulated)
{
    unsigned int sampleIter; // [esp+4h] [ebp-4h]

    for (sampleIter = 0; sampleIter != 168; sampleIter += 8)
    {
        unsigned char *rgb = ((unsigned char *)&colors->rgb + sampleIter);

        accumulated[sampleIter]     += fixedPointWeight * rgb[0];
        accumulated[sampleIter + 1] += fixedPointWeight * rgb[1];
        accumulated[sampleIter + 2] += fixedPointWeight * rgb[2];
        accumulated[sampleIter + 3] += fixedPointWeight * rgb[3];
        accumulated[sampleIter + 4] += fixedPointWeight * rgb[4];
        accumulated[sampleIter + 5] += fixedPointWeight * rgb[5];
        accumulated[sampleIter + 6] += fixedPointWeight * rgb[6];
        accumulated[sampleIter + 7] += fixedPointWeight * rgb[7];
    }
}

void __cdecl R_PackAccumulatedLightGridColors(const unsigned __int16 *accumulated, GfxLightGridColors *packed)
{
    unsigned int sampleIter; // [esp+4h] [ebp-4h]

    for (sampleIter = 0; sampleIter < 168; sampleIter += 8)
    {
        unsigned char *rgb = ((unsigned char *)&packed->rgb + sampleIter);

        rgb[0] = (unsigned __int16)(accumulated[sampleIter] + 127) >> 8;
        rgb[1] = (unsigned __int16)(accumulated[sampleIter + 1] + 127) >> 8;
        rgb[2] = (unsigned __int16)(accumulated[sampleIter + 2] + 127) >> 8;
        rgb[3] = (unsigned __int16)(accumulated[sampleIter + 3] + 127) >> 8;
        rgb[4] = (unsigned __int16)(accumulated[sampleIter + 4] + 127) >> 8;
        rgb[5] = (unsigned __int16)(accumulated[sampleIter + 5] + 127) >> 8;
        rgb[6] = (unsigned __int16)(accumulated[sampleIter + 6] + 127) >> 8;
        rgb[7] = (unsigned __int16)(accumulated[sampleIter + 7] + 127) >> 8;
    }
}

unsigned __int8 __cdecl R_GetPrimaryLightFromGrid(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    unsigned __int8 sunPrimaryLightIndex)
{
    float cornerWeight[8]; // [esp+0h] [ebp-50h] BYREF
    const GfxLightGridEntry *entry; // [esp+20h] [ebp-30h]
    unsigned int cornerIndex; // [esp+24h] [ebp-2Ch]
    unsigned __int8 primaryLightIndex; // [esp+2Bh] [ebp-25h]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+2Ch] [ebp-24h] BYREF
    unsigned int defaultGridEntry; // [esp+4Ch] [ebp-4h] BYREF

    iassert(lightGrid);

    primaryLightIndex = R_LightGridLookup(lightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);
    if (primaryLightIndex == 255)
        primaryLightIndex = sunPrimaryLightIndex;
    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
    {
        entry = cornerEntry[cornerIndex];
        if (entry
            && entry->primaryLightIndex != 255
            && (!entry->primaryLightIndex || entry->primaryLightIndex == primaryLightIndex))
        {
            return primaryLightIndex;
        }
    }
    return sunPrimaryLightIndex;
}

unsigned __int8 __cdecl R_LightGridLookup(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    float *cornerWeight,
    const GfxLightGridEntry **cornerEntry,
    unsigned int *defaultGridEntry)
{
    bool v6; // [esp+8h] [ebp-7Ch]
    float v7; // [esp+24h] [ebp-60h]
    float v8; // [esp+28h] [ebp-5Ch]
    float v9; // [esp+2Ch] [ebp-58h]
    bool v10; // [esp+32h] [ebp-52h]
    unsigned __int8 v11; // [esp+33h] [ebp-51h]
    unsigned int pos[3]; // [esp+40h] [ebp-44h] BYREF
    bool honorSuppression; // [esp+4Fh] [ebp-35h]
    const GfxLightGridEntry *entry; // [esp+50h] [ebp-34h]
    unsigned int cornerTraceBit; // [esp+54h] [ebp-30h]
    unsigned int cornerIndex; // [esp+58h] [ebp-2Ch]
    float axisLerp[3]; // [esp+5Ch] [ebp-28h]
    unsigned __int8 primaryLightIndex; // [esp+6Bh] [ebp-19h]
    float bestPrimaryLightWeight; // [esp+6Ch] [ebp-18h]
    bool suppressEntry; // [esp+73h] [ebp-11h]
    float quadWeight; // [esp+74h] [ebp-10h]
    bool suppressEntryLog[8]; // [esp+78h] [ebp-Ch] BYREF

    iassert(lightGrid);

    pos[0] = ((int)floor(samplePos[0]) + 0x20000) >> 5;
    pos[1] = ((int)floor(samplePos[1]) + 0x20000) >> 5;
    pos[2] = ((int)floor(samplePos[2]) + 0x20000) >> 6;

    iassert((lightGrid->rowAxis == 0 && lightGrid->colAxis == 1) || (lightGrid->rowAxis == 1 && lightGrid->colAxis == 0));

    axisLerp[0] = (samplePos[lightGrid->rowAxis] - -131072.0f) * 0.03125f - pos[lightGrid->rowAxis];
    axisLerp[1] = (samplePos[lightGrid->colAxis] - -131072.0f) * 0.03125f - pos[lightGrid->colAxis];
    axisLerp[2] = (samplePos[2] - -131072.0f) * 0.015625f - pos[2];

    quadWeight = (1.0f - axisLerp[1]) * (1.0f - axisLerp[2]);
    cornerWeight[0] = (1.0f - axisLerp[0]) * quadWeight;
    cornerWeight[4] = quadWeight * axisLerp[0];

    quadWeight = (1.0f - axisLerp[1]) * axisLerp[2];
    cornerWeight[1] = (1.0f - axisLerp[0]) * quadWeight;
    cornerWeight[5] = quadWeight * axisLerp[0];

    quadWeight = (1.0f - axisLerp[2]) * axisLerp[1];
    cornerWeight[2] = (1.0f - axisLerp[0]) * quadWeight;
    cornerWeight[6] = quadWeight * axisLerp[0];

    quadWeight = axisLerp[1] * axisLerp[2];
    cornerWeight[3] = (1.0f - axisLerp[0]) * quadWeight;
    cornerWeight[7] = quadWeight * axisLerp[0];

    *defaultGridEntry = 1;
    R_GetLightGridSampleEntryQuad(lightGrid, pos, cornerEntry, defaultGridEntry);
    ++pos[lightGrid->rowAxis];
    R_GetLightGridSampleEntryQuad(lightGrid, pos, cornerEntry + 4, defaultGridEntry);
    --pos[lightGrid->rowAxis];

    if (r_vc_makelog->current.integer)
        R_UpdateVisHistory(lightGrid, pos);

    primaryLightIndex = 0;
    bestPrimaryLightWeight = 0.0f;
    honorSuppression = 0;
    cornerIndex = 0;
    cornerTraceBit = 1;

    for (cornerIndex = 0; cornerIndex < 8; cornerIndex++, cornerTraceBit <<= 1)
    {
        entry = cornerEntry[cornerIndex];
        if (!entry)
            continue;

        if (cornerWeight[cornerIndex] < EQUAL_EPSILON)
        {
            cornerEntry[cornerIndex] = 0;
            continue;
        }

        v6 = ((unsigned __int8)cornerTraceBit & entry->needsTrace) != 0 && !R_IsValidLightGridSample(lightGrid, entry, cornerIndex, pos, samplePos);
        suppressEntry = v6;
        suppressEntryLog[cornerIndex] = v6;
        if (suppressEntry)
        {
            if (honorSuppression)
            {
                cornerEntry[cornerIndex] = 0;
                continue;
            }
        }
        else if (!honorSuppression)
        {
            honorSuppression = 1;
            bestPrimaryLightWeight = cornerWeight[cornerIndex];
            primaryLightIndex = entry->primaryLightIndex;
            memset((unsigned __int8 *)cornerEntry, 0, 4 * cornerIndex);
            continue;
        }
        v11 = entry->primaryLightIndex;
        if (primaryLightIndex)
        {
            if (v11)
                v10 = primaryLightIndex == 255 || v11 != 255 && cornerWeight[cornerIndex] > (double)bestPrimaryLightWeight;
            else
                v10 = 0;
        }
        else
        {
            v10 = 1;
        }
        if (v10)
        {
            bestPrimaryLightWeight = cornerWeight[cornerIndex];
            primaryLightIndex = entry->primaryLightIndex;
        }
    }
    if (r_showLightGrid->current.enabled)
        R_ShowLightGrid(lightGrid, pos, samplePos, cornerEntry, suppressEntryLog, honorSuppression);
    return primaryLightIndex;
}

void __cdecl R_ShowLightGrid(
    const GfxLightGrid *lightGrid,
    const unsigned int *pos,
    const float *samplePos,
    const GfxLightGridEntry **cornerEntry,
    bool *suppressEntry,
    bool honorSuppression)
{
    unsigned int yBit; // [esp+8h] [ebp-1Ch]
    unsigned int z; // [esp+Ch] [ebp-18h]
    unsigned int cornerIndex; // [esp+10h] [ebp-14h]
    unsigned int xBit; // [esp+18h] [ebp-Ch]
    unsigned int x; // [esp+1Ch] [ebp-8h]
    unsigned int y; // [esp+20h] [ebp-4h]

    R_ShowGridOrigin(samplePos);
    R_ShowGridBox(pos);

    iassert(rgp.world);

    if (lightGrid->rowAxis)
    {
        xBit = 2;
        yBit = 4;
    }
    else
    {
        xBit = 4;
        yBit = 2;
    }
    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
    {
        if (cornerEntry[cornerIndex])
        {
            x = pos[0] + ((xBit & cornerIndex) != 0);
            y = pos[1] + ((yBit & cornerIndex) != 0);
            z = pos[2] + ((cornerIndex & 1) != 0);
            if (((1 << cornerIndex) & cornerEntry[cornerIndex]->needsTrace) != 0)
            {
                if (suppressEntry[cornerIndex])
                {
                    if (honorSuppression)
                        R_ShowGridCorner(x, y, z, 1.1f, colorRed);
                    else
                        R_ShowGridCorner(x, y, z, 1.1f, colorOrange);
                }
                else
                {
                    R_ShowGridCorner(x, y, z, 1.0f, colorYellow);
                }
            }
            else
            {
                R_ShowGridCorner(x, y, z, 0.89999998f, colorGreen);
            }
        }
    }
}

void __cdecl R_ShowGridOrigin(const float *origin)
{
    float boxHalfSize[3]; // [esp+0h] [ebp-24h] BYREF
    float boxMaxs[3]; // [esp+Ch] [ebp-18h] BYREF
    float boxMins[3]; // [esp+18h] [ebp-Ch] BYREF

    boxHalfSize[0] = 0.5f;
    boxHalfSize[1] = 0.5f;
    boxHalfSize[2] = 0.5f;

    Vec3Sub(origin, boxHalfSize, boxMins);
    Vec3Add(origin, boxHalfSize, boxMaxs);

    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, colorBlue);
}

void __cdecl R_ShowGridBox(const unsigned int *pos)
{
    float origin[3]; // [esp+18h] [ebp-24h]
    float boxMaxs[3]; // [esp+24h] [ebp-18h] BYREF
    float boxMins[3]; // [esp+30h] [ebp-Ch] BYREF

    origin[0] = pos[0] * 32.0f + -131072.0f;
    origin[1] = pos[1] * 32.0f + -131072.0f;
    origin[2] = pos[2] * 64.0f + -131072.0f;

    boxMins[0] = origin[0];
    boxMins[1] = origin[1];
    boxMins[2] = origin[2];

    boxMaxs[0] = origin[0] + 32.0f;
    boxMaxs[1] = origin[1] + 32.0f;
    boxMaxs[2] = origin[2] + 64.0f;

    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, colorWhite);
}

void __cdecl R_ShowGridCorner(unsigned int x, unsigned int y, unsigned int z, float halfSize, const float *color)
{
    float origin[3]; // [esp+18h] [ebp-24h]
    float boxMaxs[3]; // [esp+24h] [ebp-18h] BYREF
    float boxMins[3]; // [esp+30h] [ebp-Ch] BYREF

    origin[0] = x * 32.0f + -131072.0f;
    origin[1] = y * 32.0f + -131072.0f;
    origin[2] = z * 64.0f + -131072.0f;

    boxMins[0] = origin[0] - halfSize;
    boxMins[1] = origin[1] - halfSize;
    boxMins[2] = origin[2] - halfSize;

    boxMaxs[0] = halfSize + origin[0];
    boxMaxs[1] = halfSize + origin[1];
    boxMaxs[2] = halfSize + origin[2];

    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, color);
}

void __cdecl R_UpdateVisHistory(const GfxLightGrid *lightGrid, const unsigned int *pos)
{
    unsigned int yBit; // [esp+0h] [ebp-1Ch]
    unsigned int cornerIndex; // [esp+8h] [ebp-14h]
    unsigned int xBit; // [esp+10h] [ebp-Ch]

    if (lightGrid->rowAxis)
    {
        xBit = 2;
        yBit = 4;
    }
    else
    {
        xBit = 4;
        yBit = 2;
    }
    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
        R_SortedHistoryEntry(
            *pos + ((xBit & cornerIndex) != 0),
            pos[1] + ((yBit & cornerIndex) != 0),
            pos[2] + ((cornerIndex & 1) != 0),
            SH_ADD_IF_NEW);
}

void __cdecl R_GetLightGridSampleEntryQuad(
    const GfxLightGrid *lightGrid,
    const unsigned int *pos,
    const GfxLightGridEntry **entries,
    unsigned int *defaultGridEntry)
{
    const GfxLightGridEntry *v4; // [esp+0h] [ebp-54h]
    const GfxLightGridEntry *v5; // [esp+4h] [ebp-50h]
    int v6; // [esp+8h] [ebp-4Ch]
    const GfxLightGridEntry *v7; // [esp+Ch] [ebp-48h]
    const GfxLightGridEntry *v8; // [esp+10h] [ebp-44h]
    const GfxLightGridEntry *v9; // [esp+14h] [ebp-40h]
    const GfxLightGridEntry *v10; // [esp+18h] [ebp-3Ch]
    int v11; // [esp+1Ch] [ebp-38h]
    const GfxLightGridEntry *v12; // [esp+20h] [ebp-34h]
    const GfxLightGridEntry *v13; // [esp+24h] [ebp-30h]
    unsigned int lookup; // [esp+28h] [ebp-2Ch]
    unsigned int lookupa; // [esp+28h] [ebp-2Ch]
    unsigned int lookupb; // [esp+28h] [ebp-2Ch]
    unsigned int lookupc; // [esp+28h] [ebp-2Ch]
    unsigned int firstBlockEntry; // [esp+2Ch] [ebp-28h]
    unsigned int firstBlockEntrya; // [esp+2Ch] [ebp-28h]
    unsigned int z; // [esp+30h] [ebp-24h]
    unsigned int localZ; // [esp+38h] [ebp-1Ch]
    const GfxLightGridRow *row; // [esp+3Ch] [ebp-18h]
    const unsigned __int8 *rleData; // [esp+40h] [ebp-14h]
    const unsigned __int8 *rleDataa; // [esp+40h] [ebp-14h]
    unsigned int rleSizeFull; // [esp+44h] [ebp-10h]
    unsigned int colIndex; // [esp+48h] [ebp-Ch]
    unsigned int rowIndex; // [esp+4Ch] [ebp-8h]
    unsigned int baseZ; // [esp+50h] [ebp-4h]
    unsigned int baseZa; // [esp+50h] [ebp-4h]
    unsigned int baseZb; // [esp+50h] [ebp-4h]
    unsigned int baseZc; // [esp+50h] [ebp-4h]

    rowIndex = pos[lightGrid->rowAxis] - lightGrid->mins[lightGrid->rowAxis];
    if (rowIndex >= lightGrid->maxs[lightGrid->rowAxis] + 1 - (unsigned int)lightGrid->mins[lightGrid->rowAxis]
        || lightGrid->rowDataStart[rowIndex] == 0xFFFF)
    {
        entries[0] = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        return;
    }
    bcassert(lightGrid->rowDataStart[rowIndex] * 4, lightGrid->rawRowDataSize);
    row = (const GfxLightGridRow *)&lightGrid->rawRowData[4 * lightGrid->rowDataStart[rowIndex]];
    bcassert(row->firstEntry, lightGrid->entryCount);

    colIndex = pos[lightGrid->colAxis] - row->colStart;
    z = pos[2] - row->zStart;
    if (colIndex + 1 > row->colCount)
    {
        entries[0] = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        return;
    }
    if (z + 1 > row->zCount)
    {
        entries[0] = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        if (pos[2] < row->zStart)
            *defaultGridEntry = 0;
        return;
    }

    firstBlockEntry = row->firstEntry;
    rleData = (const unsigned __int8 *)&row[1];
    rleSizeFull = (row->zCount > 255) + 3;

    if (colIndex == -1)
    {
        entries[0] = 0;
        entries[1] = 0;
        baseZ = LOBYTE(row[1].colCount);
        if (row->zCount > 255)
            baseZ += HIBYTE(row[1].colCount) << 8;
        lookup = z - baseZ + firstBlockEntry;
        if (z - baseZ < HIBYTE(row[1].colStart))
            v13 = &lightGrid->entries[lookup];
        else
            v13 = 0;
        entries[2] = v13;
        if (z - baseZ + 1 < HIBYTE(row[1].colStart))
            v12 = &lightGrid->entries[lookup + 1];
        else
            v12 = 0;
        entries[3] = v12;
        if (z < baseZ)
            *defaultGridEntry = 0;
    }
    else
    {
        while (colIndex >= *rleData)
        {
            colIndex -= *rleData;
            firstBlockEntry += rleData[1] * *rleData;
            if (rleData[1])
                v11 = (row->zCount > 255) + 3;
            else
                v11 = 2;
            rleData += v11;
        }
        if (rleData[1])
        {
            baseZb = rleData[2];
            if (row->zCount > 255)
                baseZb += rleData[3] << 8;
            if (z < baseZb)
                *defaultGridEntry = 0;
            localZ = z - baseZb;
            lookupa = z - baseZb + firstBlockEntry + colIndex * rleData[1];
            if (z - baseZb < rleData[1])
                v10 = &lightGrid->entries[lookupa];
            else
                v10 = 0;
            entries[0] = v10;
            if (localZ + 1 < rleData[1])
                v9 = &lightGrid->entries[lookupa + 1];
            else
                v9 = 0;
            entries[1] = v9;
            if (colIndex + 1 < *rleData)
            {
                lookupb = lookupa + rleData[1];
                if (localZ < rleData[1])
                    v8 = &lightGrid->entries[lookupb];
                else
                    v8 = 0;
                entries[2] = v8;
                if (localZ + 1 < rleData[1])
                    v7 = &lightGrid->entries[lookupb + 1];
                else
                    v7 = 0;
                entries[3] = v7;
                return;
            }
        }
        else
        {
            entries[0] = 0;
            entries[1] = 0;
            if (rleData[3])
            {
                baseZa = rleData[4];
                if (row->zCount > 0xFFu)
                    baseZa += rleData[5] << 8;
                if (z < baseZa + rleData[3])
                    *defaultGridEntry = 0;
            }
            if (colIndex + 1 < *rleData)
            {
                entries[2] = 0;
                entries[3] = 0;
                return;
            }
        }
        if (pos[lightGrid->colAxis] + 1 == row->colCount + row->colStart)
        {
            entries[2] = 0;
            entries[3] = 0;
        }
        else
        {
            firstBlockEntrya = firstBlockEntry + rleData[1] * *rleData;
            if (rleData[1])
                v6 = rleSizeFull;
            else
                v6 = 2;
            rleDataa = &rleData[v6];
            baseZc = rleDataa[2];
            if (row->zCount > 0xFFu)
                baseZc += rleDataa[3] << 8;
            lookupc = z - baseZc + firstBlockEntrya;
            if (z - baseZc < rleDataa[1])
                v5 = &lightGrid->entries[lookupc];
            else
                v5 = 0;
            entries[2] = v5;
            if (z - baseZc + 1 < rleDataa[1])
                v4 = &lightGrid->entries[lookupc + 1];
            else
                v4 = 0;
            entries[3] = v4;
        }
    }
}

bool __cdecl R_IsValidLightGridSample(
    const GfxLightGrid *lightGrid,
    const GfxLightGridEntry *entry,
    char cornerIndex,
    const unsigned int *pos,
    const float *samplePos)
{
    float traceDir[3]; // [esp+3Ch] [ebp-24h] BYREF
    float gridPos[3]; // [esp+48h] [ebp-18h] BYREF
    float nudgedGridPos[3]; // [esp+54h] [ebp-Ch] BYREF

    iassert(lightGrid);
    bcassert(lightGrid->rowAxis, 3);
    bcassert(lightGrid->colAxis, 3);

    gridPos[0] = pos[0] * 32.0f + -131072.0f;
    gridPos[1] = pos[1] * 32.0f + -131072.0f;
    gridPos[2] = pos[2] * 64.0f + -131072.0f;
    //gridPos[lightGrid->rowAxis] = (double)((cornerIndex & 4) != 0 ? 0x20 : 0) + gridPos[lightGrid->rowAxis];
    gridPos[lightGrid->rowAxis] += ((cornerIndex & 4) != 0 ? 32.0f : 0.0f);
    //gridPos[lightGrid->colAxis] = (double)((cornerIndex & 2) != 0 ? 0x20 : 0) + gridPos[lightGrid->colAxis];
    gridPos[lightGrid->colAxis] += ((cornerIndex & 2) != 0 ? 32.0f : 0.0f);
    //gridPos[2] = (double)((cornerIndex & 1) != 0 ? 0x40 : 0) + gridPos[2];
    gridPos[2] += ((cornerIndex & 1) != 0 ? 64.0f : 0.0f);
    Vec3Sub(samplePos, gridPos, traceDir);
    Vec3Normalize(traceDir);
    Vec3Mad(gridPos, 0.0099999998f, traceDir, nudgedGridPos);
    return CM_BoxSightTrace(0, samplePos, nudgedGridPos, vec3_origin, vec3_origin, 0, 8193) == 0;
}

unsigned int __cdecl R_GetLightingAtPoint(
    const GfxLightGrid *lightGrid,
    const float *samplePos,
    unsigned int nonSunPrimaryLightIndex,
    unsigned __int16 dest,
    GfxModelLightExtrapolation extrapolateBehavior)
{
    float v6; // [esp+Ch] [ebp-F4h]
    float cornerWeight[8]; // [esp+68h] [ebp-98h] BYREF
    const GfxLightGridEntry *entry; // [esp+88h] [ebp-78h]
    unsigned int cornerIndex; // [esp+8Ch] [ebp-74h]
    unsigned __int16 sampleColors[8]; // [esp+90h] [ebp-70h] BYREF
    unsigned int primaryLightIndex; // [esp+A4h] [ebp-5Ch]
    float primaryVisibleWeight; // [esp+A8h] [ebp-58h]
    const ComPrimaryLight *light; // [esp+ACh] [ebp-54h]
    float maxWeight; // [esp+B0h] [ebp-50h]
    unsigned int sampleCount; // [esp+B4h] [ebp-4Ch]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+B8h] [ebp-48h] BYREF
    unsigned int defaultGridEntry; // [esp+D8h] [ebp-28h] BYREF
    float primaryOccludedWeight; // [esp+DCh] [ebp-24h]
    float sampleWeight[8]; // [esp+E0h] [ebp-20h] BYREF

    iassert(lightGrid);

    PROF_SCOPED("R_GetStaticLights");

    primaryLightIndex = R_LightGridLookup(lightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);

    if (primaryLightIndex == 255)
    {
        primaryLightIndex = LOBYTE(lightGrid->sunPrimaryLightIndex);
    }
    else if (lightGrid->hasLightRegions && primaryLightIndex != lightGrid->sunPrimaryLightIndex)
    {
        primaryLightIndex = nonSunPrimaryLightIndex;
    }
    sampleCount = 0;
    maxWeight = 0.0;
    primaryVisibleWeight = 0.0;
    primaryOccludedWeight = 0.0;
    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
    {
        entry = cornerEntry[cornerIndex];
        if (entry)
        {
            if (entry->primaryLightIndex == primaryLightIndex)
            {
                primaryVisibleWeight = primaryVisibleWeight + cornerWeight[cornerIndex];
            }
            else if (!entry->primaryLightIndex || entry->primaryLightIndex == 255 && primaryLightIndex)
            {
                light = Com_GetPrimaryLight(primaryLightIndex);
                if (R_CanLightInfluenceLightGridCorner(lightGrid, light, samplePos, cornerIndex))
                    primaryOccludedWeight = primaryOccludedWeight + cornerWeight[cornerIndex];
            }
            maxWeight = maxWeight + cornerWeight[cornerIndex];
            sampleCount = R_AddLightGridSample(
                sampleColors,
                sampleWeight,
                sampleCount,
                entry->colorsIndex,
                cornerWeight[cornerIndex]);
        }
    }

    iassert(maxWeight >= 0.0f);
    iassert(primaryVisibleWeight >= 0.0f);

    if (sampleCount)
    {
        iassert(maxWeight > 0.0f);
        if (primaryLightIndex)
        {
            if (primaryOccludedWeight == 0.0)
            {
                if (primaryVisibleWeight != 0.0)
                    primaryVisibleWeight = 1.0;
            }
            else
            {
                primaryVisibleWeight = primaryVisibleWeight / (primaryVisibleWeight + primaryOccludedWeight);
            }
        }
        else
        {
            primaryVisibleWeight = 0.0;
        }
        if (sampleCount == 1)
        {
            R_SetLightGridColorsFromIndex(lightGrid, sampleColors[0], primaryVisibleWeight, dest);
        }
        else
        {
            v6 = 1.0 / maxWeight;
            R_BlendAndSetLightGridColors(
                lightGrid,
                (unsigned __int8 *)sampleColors,
                sampleWeight,
                sampleCount,
                primaryVisibleWeight,
                v6,
                dest);
        }
    }
    else
    {
        primaryLightIndex = R_ExtrapolateLightingAtPoint(lightGrid, dest, extrapolateBehavior, defaultGridEntry);
    }

    return primaryLightIndex;
}

GfxModelLightingPatch *__cdecl R_BackEndDataAllocAndClearModelLightingPatch(GfxBackEndData *frontEndDataOut)
{
    GfxModelLightingPatch *v1; // edx
    unsigned int patchIndex; // [esp+4h] [ebp-4h]

    patchIndex = InterlockedExchangeAdd(&frontEndDataOut->modelLightingPatchCount, 1);
    if (patchIndex >= 0x1000)
        Com_Error(ERR_FATAL, "modelLightingPatchList ran out of elements.");
    v1 = &frontEndDataOut->modelLightingPatchList[patchIndex];
    memset(v1, 0, sizeof(v1));
    return v1;
}

void __cdecl R_SetLightGridColorsFromIndex(
    const GfxLightGrid *lightGrid,
    unsigned int colorsIndex,
    float primaryLightWeight,
    unsigned __int16 dest)
{
    GfxModelLightingPatch *patch; // [esp+14h] [ebp-4h]

    patch = R_BackEndDataAllocAndClearModelLightingPatch(frontEndDataOut);
    patch->modelLightingIndex = dest;
    patch->primaryLightWeight = (int)(primaryLightWeight * 255.0f + 0.5f);
    patch->colorsCount = 1;
    iassert(colorsIndex == (unsigned short)colorsIndex);
    patch->colorsIndex[0] = colorsIndex;
}

void __cdecl R_BlendAndSetLightGridColors(
    const GfxLightGrid *lightGrid,
    unsigned __int8 *colorsIndex,
    const float *colorsWeight,
    unsigned int colorsCount,
    float primaryLightWeight,
    float weightNormalizeScale,
    unsigned __int16 dest)
{
    GfxModelLightingPatch *patch; // [esp+28h] [ebp-4h]

    patch = R_BackEndDataAllocAndClearModelLightingPatch(frontEndDataOut);
    patch->modelLightingIndex = dest;
    patch->primaryLightWeight = (int)(primaryLightWeight * 255.0f + 0.5f);
    iassert(colorsCount == (unsigned char)colorsCount);
    patch->colorsCount = colorsCount;
    R_GetLightGridColorsFixedPointBlendWeights(colorsWeight, colorsCount, weightNormalizeScale, patch->colorsWeight);
    memcpy((unsigned __int8 *)patch->colorsIndex, colorsIndex, 2 * colorsCount);
}

void __cdecl R_GetLightGridColorsFixedPointBlendWeights(
    const float *colorsWeight,
    unsigned int colorsCount,
    float weightNormalizeScale,
    unsigned __int16 *fixedPointWeight)
{
    unsigned int maxWeightIndex; // [esp+Ch] [ebp-Ch]
    unsigned __int16 fixedPointWeightSum; // [esp+10h] [ebp-8h]
    unsigned int colorsIter; // [esp+14h] [ebp-4h]

    fixedPointWeightSum = 0;
    maxWeightIndex = 0;
    colorsIter = 0;
    do
    {
        fixedPointWeight[colorsIter] = (int)(weightNormalizeScale * 256.0f * colorsWeight[colorsIter] + 0.5f);
        fixedPointWeightSum += fixedPointWeight[colorsIter];
        if (fixedPointWeight[maxWeightIndex] < (int)fixedPointWeight[colorsIter])
            maxWeightIndex = colorsIter;
        ++colorsIter;
    } while (colorsIter < colorsCount);
    fixedPointWeight[maxWeightIndex] += 256 - fixedPointWeightSum;
}

unsigned __int8 __cdecl R_ExtrapolateLightingAtPoint(
    const GfxLightGrid *lightGrid,
    unsigned __int16 dest,
    GfxModelLightExtrapolation extrapolateBehavior,
    unsigned int defaultGridEntry)
{
    if (extrapolateBehavior == GFX_MODELLIGHT_SHOW_MISSING
        && !defaultGridEntry
        && r_showMissingLightGrid->current.enabled
        || lightGrid->colorCount <= defaultGridEntry)
    {
        R_SetLightGridColorsFromIndex(lightGrid, lightGrid->colorCount - 1, 1.0, dest);
        return 0;
    }
    else
    {
        R_SetLightGridColorsFromIndex(lightGrid, defaultGridEntry, 1.0, dest);
        return lightGrid->sunPrimaryLightIndex;
    }
}

unsigned int __cdecl R_AddLightGridSample(
    unsigned __int16 *sampleColors,
    float *sampleWeight,
    unsigned int sampleCount,
    unsigned __int16 sampleColorsAdd,
    float sampleWeightAdd)
{
    unsigned int sampleIndex; // [esp+0h] [ebp-4h]

    for (sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        if (sampleColors[sampleIndex] == sampleColorsAdd)
        {
            sampleWeight[sampleIndex] = sampleWeight[sampleIndex] + sampleWeightAdd;
            return sampleCount;
        }
    }
    sampleColors[sampleCount] = sampleColorsAdd;
    sampleWeight[sampleCount] = sampleWeightAdd;
    return sampleCount + 1;
}

char __cdecl R_CanLightInfluenceLightGridCorner(
    const GfxLightGrid *lightGrid,
    const ComPrimaryLight *light,
    const float *samplePos,
    char cornerIndex)
{
    float gridPos[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if (light->type == 1)
        return 1;

    gridPos[0] = floor(samplePos[0] * 0.03125f) * 32.0f;
    gridPos[1] = floor(samplePos[1] * 0.03125f) * 32.0f;
    gridPos[2] = floor(samplePos[0] * 0.015625f) * 64.0f;
    //gridPos[lightGrid->rowAxis] = (float)((cornerIndex & 4) != 0 ? 0x20 : 0) + gridPos[lightGrid->rowAxis];
    gridPos[lightGrid->rowAxis] += ((cornerIndex & 4) != 0 ? 32.0f : 0.0f);
    //gridPos[lightGrid->colAxis] = (float)((cornerIndex & 2) != 0 ? 0x20 : 0) + gridPos[lightGrid->colAxis];
    gridPos[lightGrid->colAxis] += ((cornerIndex & 2) != 0 ? 32.0f : 0.0f);
    //gridPos[2] = (float)((cornerIndex & 1) != 0 ? 0x40 : 0) + gridPos[2];
    gridPos[2] += ((cornerIndex & 1) != 0 ? 64.0f : 0.0f);

    return Com_CanPrimaryLightAffectPoint(light, gridPos);
}

void __cdecl R_GetAverageLightingAtPoint(const float *samplePos, unsigned __int8 *outColor)
{
    float v2; // [esp+18h] [ebp-C0h]
    float v3; // [esp+1Ch] [ebp-BCh]
    float v4; // [esp+20h] [ebp-B8h]
    float v5; // [esp+2Ch] [ebp-ACh]
    float v6; // [esp+30h] [ebp-A8h]
    float cornerWeight[8]; // [esp+38h] [ebp-A0h] BYREF
    const GfxLightGridEntry *entry; // [esp+58h] [ebp-80h]
    unsigned int cornerIndex; // [esp+5Ch] [ebp-7Ch]
    unsigned __int16 sampleColors[8]; // [esp+60h] [ebp-78h] BYREF
    float weightNormalizeScale; // [esp+74h] [ebp-64h]
    unsigned __int8 primaryLightIndex; // [esp+7Bh] [ebp-5Dh]
    unsigned __int8 colorWithSunAlpha[4]; // [esp+7Ch] [ebp-5Ch] BYREF
    float maxWeight; // [esp+80h] [ebp-58h]
    unsigned int sampleCount; // [esp+84h] [ebp-54h]
    const GfxLightGridEntry *cornerEntry[8]; // [esp+88h] [ebp-50h] BYREF
    unsigned int defaultGridEntry; // [esp+A8h] [ebp-30h] BYREF
    int colorIndex; // [esp+ACh] [ebp-2Ch]
    float sampleWeight[8]; // [esp+B0h] [ebp-28h] BYREF
    float primaryWeight; // [esp+D0h] [ebp-8h]
    float *primaryLightColor; // [esp+D4h] [ebp-4h]

    sampleCount = 0;
    maxWeight = 0.0;
    primaryWeight = 0.0;
    primaryLightIndex = R_LightGridLookup(&rgp.world->lightGrid, samplePos, cornerWeight, cornerEntry, &defaultGridEntry);
    if (primaryLightIndex != 1)
    {
        colorWithSunAlpha[0] = 0;
        colorWithSunAlpha[1] = 0;
        colorWithSunAlpha[2] = 0;
        colorWithSunAlpha[3] = 0x80;
        goto LABEL_21;
    }
    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
    {
        entry = cornerEntry[cornerIndex];
        if (!entry)
            continue;
        if (entry->primaryLightIndex && entry->primaryLightIndex != 255)
        {
            if (entry->primaryLightIndex != primaryLightIndex)
                continue;
            primaryWeight = primaryWeight + cornerWeight[cornerIndex];
        }
        maxWeight = maxWeight + cornerWeight[cornerIndex];
        sampleCount = R_AddLightGridSample(
            sampleColors,
            sampleWeight,
            sampleCount,
            entry->colorsIndex,
            cornerWeight[cornerIndex]);
    }

    iassert(maxWeight >= 0.0f);
    iassert(primaryWeight >= 0.0f);

    if (sampleCount)
    {
        weightNormalizeScale = 1.0f / maxWeight;
        primaryWeight = primaryWeight * weightNormalizeScale;
        iassert(primaryWeight >= 0.0f);
        R_BlendAndAverageLightGridColors(
            &rgp.world->lightGrid,
            sampleColors,
            sampleWeight,
            sampleCount,
            primaryWeight,
            weightNormalizeScale,
            colorWithSunAlpha);
    }
    else
    {
        colorWithSunAlpha[0] = 0;
        colorWithSunAlpha[1] = 0;
        colorWithSunAlpha[2] = 0;
        colorWithSunAlpha[3] = -1;
    }
LABEL_21:
    primaryLightColor = rgp.world->sunLight->color;
    for (colorIndex = 0; colorIndex != 3; ++colorIndex)
    {
        v5 = primaryLightColor[colorIndex] * (colorWithSunAlpha[3] * 0.5f) + colorWithSunAlpha[colorIndex];
        v4 = v5 - 255.0f;
        if (v4 < 0.0f)
            v6 = primaryLightColor[colorIndex] * (colorWithSunAlpha[3] * 0.5f) + colorWithSunAlpha[colorIndex];
        else
            v6 = 255.0f;
        v3 = 0.0f - v5;
        if (v3 < 0.0f)
            v2 = v6;
        else
            v2 = 0.0f;
        outColor[colorIndex] = (int)v2;
    }
    outColor[3] = -1;
}

void __cdecl R_BlendAndAverageLightGridColors(
    const GfxLightGrid *lightGrid,
    const unsigned __int16 *colorsIndex,
    const float *colorsWeight,
    unsigned int colorsCount,
    float primaryLightWeight,
    float weightNormalizeScale,
    unsigned __int8 *outAverage)
{
    unsigned __int16 fixedPointWeight[8]; // [esp+184h] [ebp-BCh] BYREF
    GfxLightGridColors packed; // [esp+198h] [ebp-A8h] BYREF

    R_GetLightGridColorsFixedPointBlendWeights(colorsWeight, colorsCount, weightNormalizeScale, fixedPointWeight);
    R_FixedPointBlendLightGridColors(lightGrid, colorsIndex, fixedPointWeight, colorsCount, &packed);
    R_AverageLightGridColors(&packed, primaryLightWeight, outAverage);
}

void __cdecl R_AverageLightGridColors(const GfxLightGridColors *colors, float sunWeight, unsigned __int8 *outColor)
{
    int accumulator[3]; // [esp+Ch] [ebp-14h]
    int sampleIndex; // [esp+18h] [ebp-8h]
    int colorIndex; // [esp+1Ch] [ebp-4h]

    for (colorIndex = 0; colorIndex != 3; ++colorIndex)
        accumulator[colorIndex] = 0;
    for (sampleIndex = 0; sampleIndex != 56; ++sampleIndex)
    {
        for (colorIndex = 0; colorIndex != 3; ++colorIndex)
            accumulator[colorIndex] += colors->rgb[sampleIndex][colorIndex];
    }
    for (colorIndex = 0; colorIndex != 3; ++colorIndex)
        outColor[colorIndex] = accumulator[colorIndex] / 56;
    outColor[3] = (int)(sunWeight * 255.0 + 0.5);
}

void __cdecl R_InitLightVisHistory(char *bspName)
{
    char filename[68]; // [esp+30h] [ebp-50h] BYREF
    unsigned __int16 (*buffer)[3]; // [esp+78h] [ebp-8h] BYREF
    int count; // [esp+7Ch] [ebp-4h]

    s_vc_log.history = 0;
    s_vc_log.count = 0;
    if (r_vc_makelog->current.integer)
    {
        s_vc_log.history = (unsigned __int16 (*)[3])Z_VirtualAlloc(6291456, "R_InitLightVisHistory", 0);
        if (r_vc_makelog->current.integer == 2)
        {
            R_LightVisHistoryFilename(bspName, filename);
            count = FS_ReadFile(filename, (void **)&buffer);
            if (count >= 0)
            {
                if (!(count % 6u))
                {
                    if (count > 6291456)
                        count = 6291456;
                    {
                        PROF_SCOPED("R_memcpy");
                        memcpy((unsigned __int8 *)s_vc_log.history, (unsigned __int8 *)buffer, count);
                    }
                    s_vc_log.count = count / 6u;
                }
                FS_FreeFile((char *)buffer);
            }
        }
    }
}

void __cdecl R_LightVisHistoryFilename(char *bspName, char *filename)
{
    iassert(bspName);

    Com_StripExtension(bspName, filename);
    if (strlen(filename) + 5 >= 0x40)
        Com_Error(ERR_DROP, "light grid log filename %s.grid is too long", filename);
    strcat(filename, ".grid");
}

void __cdecl R_SaveLightVisHistory()
{
    char filename[68]; // [esp+0h] [ebp-48h] BYREF

    if (s_vc_log.history)
    {
        if (rgp.world)
        {
            R_LightVisHistoryFilename((char *)rgp.world->name, filename);
            FS_WriteFile(filename, (char *)s_vc_log.history, 6 * s_vc_log.count);
            Z_VirtualFree(s_vc_log.history);
            s_vc_log.history = 0;
            s_vc_log.count = 0;
        }
    }
}

char __cdecl R_IsPointInLightRegionHull(const float *relPoint, const GfxLightRegionHull *hull)
{
    float v3; // [esp+0h] [ebp-80h]
    float v4; // [esp+4h] [ebp-7Ch]
    float v5; // [esp+8h] [ebp-78h]
    float v6; // [esp+Ch] [ebp-74h]
    float v7; // [esp+10h] [ebp-70h]
    float v8; // [esp+14h] [ebp-6Ch]
    float v9; // [esp+18h] [ebp-68h]
    float v10; // [esp+1Ch] [ebp-64h]
    float v11; // [esp+20h] [ebp-60h]
    float v12; // [esp+24h] [ebp-5Ch]
    float v13; // [esp+2Ch] [ebp-54h]
    float v14; // [esp+34h] [ebp-4Ch]
    float v15; // [esp+3Ch] [ebp-44h]
    float v16; // [esp+44h] [ebp-3Ch]
    float v17; // [esp+4Ch] [ebp-34h]
    float v18; // [esp+54h] [ebp-2Ch]
    float v19; // [esp+5Ch] [ebp-24h]
    float v20; // [esp+64h] [ebp-1Ch]
    float v21; // [esp+6Ch] [ebp-14h]
    float v22; // [esp+74h] [ebp-Ch]
    unsigned int axisIter; // [esp+78h] [ebp-8h]
    float midPointAlongDir; // [esp+7Ch] [ebp-4h]

    v22 = *relPoint - hull->kdopMidPoint[0];
    v12 = I_fabs(v22);
    if (hull->kdopHalfSize[0] <= v12)
        return 0;
    v21 = relPoint[1] - hull->kdopMidPoint[1];
    v11 = I_fabs(v21);
    if (hull->kdopHalfSize[1] <= v11)
        return 0;
    v20 = relPoint[2] - hull->kdopMidPoint[2];
    v10 = I_fabs(v20);
    if (hull->kdopHalfSize[2] <= v10)
        return 0;
    v19 = relPoint[1] + *relPoint - hull->kdopMidPoint[3];
    v9 = I_fabs(v19);
    if (hull->kdopHalfSize[3] <= v9)
        return 0;
    v18 = *relPoint - relPoint[1] - hull->kdopMidPoint[4];
    v8 = I_fabs(v18);
    if (hull->kdopHalfSize[4] <= v8)
        return 0;
    v17 = relPoint[2] + *relPoint - hull->kdopMidPoint[5];
    v7 = I_fabs(v17);
    if (hull->kdopHalfSize[5] <= v7)
        return 0;
    v16 = *relPoint - relPoint[2] - hull->kdopMidPoint[6];
    v6 = I_fabs(v16);
    if (hull->kdopHalfSize[6] <= v6)
        return 0;
    v15 = relPoint[2] + relPoint[1] - hull->kdopMidPoint[7];
    v5 = I_fabs(v15);
    if (hull->kdopHalfSize[7] <= v5)
        return 0;
    v14 = relPoint[1] - relPoint[2] - hull->kdopMidPoint[8];
    v4 = I_fabs(v14);
    if (hull->kdopHalfSize[8] <= v4)
        return 0;
    for (axisIter = 0; axisIter < hull->axisCount; ++axisIter)
    {
        midPointAlongDir = Vec3Dot(hull->axis[axisIter].dir, relPoint);
        v13 = midPointAlongDir - hull->axis[axisIter].midPoint;
        v3 = I_fabs(v13);
        if (hull->axis[axisIter].halfSize <= v3)
            return 0;
    }
    return 1;
}

unsigned int __cdecl R_GetPrimaryLightForModelVertex(
    const float *point,
    unsigned int primaryLightCount,
    const bool *checkLight,
    const GfxLightRegion *lightRegions)
{
    float v5; // [esp+0h] [ebp-28h]
    unsigned int hullIter; // [esp+4h] [ebp-24h]
    const ComPrimaryLight *light; // [esp+8h] [ebp-20h]
    unsigned int primaryLightIter; // [esp+Ch] [ebp-1Ch]
    float cosHalfFov; // [esp+10h] [ebp-18h]
    float lenSq; // [esp+14h] [ebp-14h]
    float relPoint[3]; // [esp+18h] [ebp-10h] BYREF
    float dot; // [esp+24h] [ebp-4h]

    for (primaryLightIter = 0; primaryLightIter < primaryLightCount; ++primaryLightIter)
    {
        if (checkLight[primaryLightIter])
        {
            light = Com_GetPrimaryLight(primaryLightIter);
            if (light->type != 3 && light->type != 2)
                MyAssertHandler(
                    ".\\rb_light.cpp",
                    1273,
                    0,
                    "%s\n\t(light->type) = %i",
                    "(light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT)",
                    light->type);
            Vec3Sub(point, light->origin, relPoint);
            lenSq = Vec3LengthSq(relPoint);
            v5 = light->radius * light->radius;
            if (lenSq <= v5)
            {
                if (light->type == 2)
                {
                    cosHalfFov = light->cosHalfFovExpanded;
                    dot = Vec3Dot(relPoint, light->dir);
                    if (cosHalfFov >= 0.0)
                    {
                        if (dot > 0.0 || cosHalfFov * cosHalfFov * lenSq > dot * dot)
                            continue;
                    }
                    else if (dot > 0.0 && cosHalfFov * cosHalfFov * lenSq < dot * dot)
                    {
                        continue;
                    }
                }
                if (!lightRegions[primaryLightIter].hullCount)
                    return primaryLightIter;
                for (hullIter = 0; hullIter < lightRegions[primaryLightIter].hullCount; ++hullIter)
                {
                    if (R_IsPointInLightRegionHull(relPoint, &lightRegions[primaryLightIter].hulls[hullIter]))
                        return primaryLightIter;
                }
            }
        }
    }
    return 0;
}

unsigned __int8 __cdecl R_GetPrimaryLightForModel(
    const XModel *model,
    const float *origin,
    const float (*axis)[3],
    float scale,
    const float *mins,
    const float *maxs,
    const GfxLightRegion *lightRegions)
{
    unsigned int chosenLight; // [esp+Ch] [ebp-564h]
    unsigned int primaryLightCount; // [esp+10h] [ebp-560h]
    const ComPrimaryLight *light; // [esp+14h] [ebp-55Ch]
    unsigned int surfCount; // [esp+18h] [ebp-558h]
    unsigned int primaryLightIter; // [esp+1Ch] [ebp-554h]
    unsigned int lod; // [esp+20h] [ebp-550h]
    float *verts; // [esp+24h] [ebp-54Ch]
    unsigned int checkCount; // [esp+28h] [ebp-548h]
    float boxHalfSize[3]; // [esp+2Ch] [ebp-544h] BYREF
    unsigned int votes[255]; // [esp+38h] [ebp-538h] BYREF
    unsigned int mostVotes; // [esp+43Ch] [ebp-134h]
    unsigned int surfIter; // [esp+440h] [ebp-130h]
    unsigned int vertCount; // [esp+444h] [ebp-12Ch]
    bool checkLight[255]; // [esp+448h] [ebp-128h] BYREF
    unsigned int bestLight; // [esp+54Ch] [ebp-24h]
    XSurface *surfs; // [esp+550h] [ebp-20h] BYREF
    unsigned int vertIter; // [esp+554h] [ebp-1Ch]
    float boxMidPoint[3]; // [esp+558h] [ebp-18h] BYREF
    float point[3]; // [esp+564h] [ebp-Ch] BYREF

    Vec3Avg(mins, maxs, boxMidPoint);
    Vec3Sub(boxMidPoint, mins, boxHalfSize);

    iassert(comWorld.isInUse); // c:\\trees\\cod3\\src\\gfx_d3d\\../qcommon/com_bsp_api.h

    primaryLightCount = comWorld.primaryLightCount;
    Com_Memset(checkLight, 0, comWorld.primaryLightCount);
    checkCount = 0;
    for (primaryLightIter = 0; primaryLightIter < primaryLightCount; ++primaryLightIter)
    {
        light = Com_GetPrimaryLight(primaryLightIter);
        if (light->type && light->type != 1 && !Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize))
        {
            checkLight[primaryLightIter] = 1;
            ++checkCount;
        }
    }
    if (!checkCount)
        return 0;
    verts = (float*)Hunk_AllocateTempMemory(393216, "R_GetXModelBounds");
    //memset(votes, 0, 4 * primaryLightCount);
    memset(votes, 0, sizeof(votes));
    mostVotes = 0;
    bestLight = 0;
    lod = XModelGetNumLods(model) - 1;
    surfCount = XModelGetSurfaces(model, &surfs, lod);
    for (surfIter = 0; surfIter < surfCount; ++surfIter)
    {
        vertCount = XSurfaceGetNumVerts(&surfs[surfIter]);
        iassert(vertCount <= MODELSURF_MAX_VERTICES);
        XSurfaceGetVerts(&surfs[surfIter], verts, 0, 0);
        for (vertIter = 0; vertIter < vertCount; ++vertIter)
        {
            MatrixTransformVector(&verts[3 * vertIter], *(const mat3x3*)axis, point);
            Vec3Mad(origin, scale, point, point);
            chosenLight = R_GetPrimaryLightForModelVertex(point, primaryLightCount, checkLight, lightRegions);
            ++votes[chosenLight];
            if (chosenLight)
            {
                if (votes[chosenLight] > mostVotes)
                {
                    mostVotes = votes[chosenLight];
                    bestLight = chosenLight;
                    if (checkCount == 1)
                        break;
                }
            }
        }
    }
    Hunk_FreeTempMemory((char*)verts);
    //if (bestLight != bestLight)
    //    MyAssertHandler(
    //        "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
    //        281,
    //        0,
    //        "i == static_cast< Type >( i )\n\t%i, %i",
    //        bestLight,
    //        bestLight);
    return bestLight;
}