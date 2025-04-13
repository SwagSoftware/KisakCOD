#include "rb_light.h"

#include "r_dvars.h"
#include "r_utils.h"

#include <qcommon/com_bsp.h>
#include <qcommon/threads.h>
#include <universal/com_files.h>

struct // sizeof=0x8
{                                       // ...
    unsigned __int16 (*history)[3];     // ...
    int count;                          // ...
} s_vc_log;

static int s_lightGridRowDelta;
static int s_lightGridSliceDelta;

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

    if (Sys_IsRenderThread())
        MyAssertHandler(".\\rb_light.cpp", 235, 0, "%s", "!Sys_IsRenderThread()");
    if (s_vc_log.history)
    {
        spread = r_vc_showlog->current.integer;
        if (spread > 0)
        {
            v6 = *viewOrigin - -131072.0;
            x = (int)(v6 + 9.313225746154785e-10) >> 5;
            v5 = viewOrigin[1] - -131072.0;
            y = (int)(v5 + 9.313225746154785e-10) >> 5;
            v4 = viewOrigin[2] - -131072.0;
            z = (int)(v4 - 0.4999999990686774) >> 6;
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
    unsigned __int8 *pixels)
{
    unsigned __int8 *pixelsa; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsb; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsc; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsd; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelse; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsf; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsg; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsh; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsi; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsj; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsk; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsl; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsm; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelsn; // [esp+210h] [ebp+10h]
    unsigned __int8 *pixelso; // [esp+210h] [ebp+10h]

    *(unsigned int *)pixels = (packedSunWeight << 24) | colors->rgb[0][2] | (colors->rgb[0][1] << 8) | (colors->rgb[0][0] << 16);
    *((unsigned int *)pixels + 1) = (packedSunWeight << 24)
        | colors->rgb[1][2]
        | (colors->rgb[1][1] << 8)
        | (colors->rgb[1][0] << 16);
    *((unsigned int *)pixels + 2) = (packedSunWeight << 24)
        | colors->rgb[2][2]
        | (colors->rgb[2][1] << 8)
        | (colors->rgb[2][0] << 16);
    *((unsigned int *)pixels + 3) = (packedSunWeight << 24)
        | colors->rgb[3][2]
        | (colors->rgb[3][1] << 8)
        | (colors->rgb[3][0] << 16);
    pixelsa = &pixels[s_lightGridRowDelta];
    *(unsigned int *)pixelsa = (packedSunWeight << 24)
        | colors->rgb[4][2]
        | (colors->rgb[4][1] << 8)
        | (colors->rgb[4][0] << 16);
    *((unsigned int *)pixelsa + 1) = (packedSunWeight << 24)
        | colors->rgb[5][2]
        | (colors->rgb[5][1] << 8)
        | (colors->rgb[5][0] << 16);
    *((unsigned int *)pixelsa + 2) = (packedSunWeight << 24)
        | colors->rgb[6][2]
        | (colors->rgb[6][1] << 8)
        | (colors->rgb[6][0] << 16);
    *((unsigned int *)pixelsa + 3) = (packedSunWeight << 24)
        | colors->rgb[7][2]
        | (colors->rgb[7][1] << 8)
        | (colors->rgb[7][0] << 16);
    pixelsb = &pixelsa[s_lightGridRowDelta];
    *(unsigned int *)pixelsb = (packedSunWeight << 24)
        | colors->rgb[8][2]
        | (colors->rgb[8][1] << 8)
        | (colors->rgb[8][0] << 16);
    *((unsigned int *)pixelsb + 1) = (packedSunWeight << 24)
        | colors->rgb[9][2]
        | (colors->rgb[9][1] << 8)
        | (colors->rgb[9][0] << 16);
    *((unsigned int *)pixelsb + 2) = (packedSunWeight << 24)
        | colors->rgb[10][2]
        | (colors->rgb[10][1] << 8)
        | (colors->rgb[10][0] << 16);
    *((unsigned int *)pixelsb + 3) = (packedSunWeight << 24)
        | colors->rgb[11][2]
        | (colors->rgb[11][1] << 8)
        | (colors->rgb[11][0] << 16);
    pixelsc = &pixelsb[s_lightGridRowDelta];
    *(unsigned int *)pixelsc = (packedSunWeight << 24)
        | colors->rgb[12][2]
        | (colors->rgb[12][1] << 8)
        | (colors->rgb[12][0] << 16);
    *((unsigned int *)pixelsc + 1) = (packedSunWeight << 24)
        | colors->rgb[13][2]
        | (colors->rgb[13][1] << 8)
        | (colors->rgb[13][0] << 16);
    *((unsigned int *)pixelsc + 2) = (packedSunWeight << 24)
        | colors->rgb[14][2]
        | (colors->rgb[14][1] << 8)
        | (colors->rgb[14][0] << 16);
    *((unsigned int *)pixelsc + 3) = (packedSunWeight << 24)
        | colors->rgb[15][2]
        | (colors->rgb[15][1] << 8)
        | (colors->rgb[15][0] << 16);
    pixelsd = &pixelsc[s_lightGridSliceDelta];
    *(unsigned int *)pixelsd = (packedSunWeight << 24)
        | colors->rgb[16][2]
        | (colors->rgb[16][1] << 8)
        | (colors->rgb[16][0] << 16);
    *((unsigned int *)pixelsd + 1) = (packedSunWeight << 24)
        | colors->rgb[17][2]
        | (colors->rgb[17][1] << 8)
        | (colors->rgb[17][0] << 16);
    *((unsigned int *)pixelsd + 2) = (packedSunWeight << 24)
        | colors->rgb[18][2]
        | (colors->rgb[18][1] << 8)
        | (colors->rgb[18][0] << 16);
    *((unsigned int *)pixelsd + 3) = (packedSunWeight << 24)
        | colors->rgb[19][2]
        | (colors->rgb[19][1] << 8)
        | (colors->rgb[19][0] << 16);
    pixelse = &pixelsd[s_lightGridRowDelta];
    *(unsigned int *)pixelse = (packedSunWeight << 24)
        | colors->rgb[20][2]
        | (colors->rgb[20][1] << 8)
        | (colors->rgb[20][0] << 16);
    *((unsigned int *)pixelse + 1) = (packedSunWeight << 24)
        | colors->rgb[0][2]
        | (colors->rgb[0][1] << 8)
        | (colors->rgb[0][0] << 16);
    *((unsigned int *)pixelse + 2) = (packedSunWeight << 24)
        | colors->rgb[3][2]
        | (colors->rgb[3][1] << 8)
        | (colors->rgb[3][0] << 16);
    *((unsigned int *)pixelse + 3) = (packedSunWeight << 24)
        | colors->rgb[21][2]
        | (colors->rgb[21][1] << 8)
        | (colors->rgb[21][0] << 16);
    pixelsf = &pixelse[s_lightGridRowDelta];
    *(unsigned int *)pixelsf = (packedSunWeight << 24)
        | colors->rgb[22][2]
        | (colors->rgb[22][1] << 8)
        | (colors->rgb[22][0] << 16);
    *((unsigned int *)pixelsf + 1) = (packedSunWeight << 24)
        | colors->rgb[12][2]
        | (colors->rgb[12][1] << 8)
        | (colors->rgb[12][0] << 16);
    *((unsigned int *)pixelsf + 2) = (packedSunWeight << 24)
        | colors->rgb[15][2]
        | (colors->rgb[15][1] << 8)
        | (colors->rgb[15][0] << 16);
    *((unsigned int *)pixelsf + 3) = (packedSunWeight << 24)
        | colors->rgb[23][2]
        | (colors->rgb[23][1] << 8)
        | (colors->rgb[23][0] << 16);
    pixelsg = &pixelsf[s_lightGridRowDelta];
    *(unsigned int *)pixelsg = (packedSunWeight << 24)
        | colors->rgb[24][2]
        | (colors->rgb[24][1] << 8)
        | (colors->rgb[24][0] << 16);
    *((unsigned int *)pixelsg + 1) = (packedSunWeight << 24)
        | colors->rgb[25][2]
        | (colors->rgb[25][1] << 8)
        | (colors->rgb[25][0] << 16);
    *((unsigned int *)pixelsg + 2) = (packedSunWeight << 24)
        | colors->rgb[26][2]
        | (colors->rgb[26][1] << 8)
        | (colors->rgb[26][0] << 16);
    *((unsigned int *)pixelsg + 3) = (packedSunWeight << 24)
        | colors->rgb[27][2]
        | (colors->rgb[27][1] << 8)
        | (colors->rgb[27][0] << 16);
    pixelsh = &pixelsg[s_lightGridSliceDelta];
    *(unsigned int *)pixelsh = (packedSunWeight << 24)
        | colors->rgb[28][2]
        | (colors->rgb[28][1] << 8)
        | (colors->rgb[28][0] << 16);
    *((unsigned int *)pixelsh + 1) = (packedSunWeight << 24)
        | colors->rgb[29][2]
        | (colors->rgb[29][1] << 8)
        | (colors->rgb[29][0] << 16);
    *((unsigned int *)pixelsh + 2) = (packedSunWeight << 24)
        | colors->rgb[30][2]
        | (colors->rgb[30][1] << 8)
        | (colors->rgb[30][0] << 16);
    *((unsigned int *)pixelsh + 3) = (packedSunWeight << 24)
        | colors->rgb[31][2]
        | (colors->rgb[31][1] << 8)
        | (colors->rgb[31][0] << 16);
    pixelsi = &pixelsh[s_lightGridRowDelta];
    *(unsigned int *)pixelsi = (packedSunWeight << 24)
        | colors->rgb[32][2]
        | (colors->rgb[32][1] << 8)
        | (colors->rgb[32][0] << 16);
    *((unsigned int *)pixelsi + 1) = (packedSunWeight << 24)
        | colors->rgb[40][2]
        | (colors->rgb[40][1] << 8)
        | (colors->rgb[40][0] << 16);
    *((unsigned int *)pixelsi + 2) = (packedSunWeight << 24)
        | colors->rgb[43][2]
        | (colors->rgb[43][1] << 8)
        | (colors->rgb[43][0] << 16);
    *((unsigned int *)pixelsi + 3) = (packedSunWeight << 24)
        | colors->rgb[33][2]
        | (colors->rgb[33][1] << 8)
        | (colors->rgb[33][0] << 16);
    pixelsj = &pixelsi[s_lightGridRowDelta];
    *(unsigned int *)pixelsj = (packedSunWeight << 24)
        | colors->rgb[34][2]
        | (colors->rgb[34][1] << 8)
        | (colors->rgb[34][0] << 16);
    *((unsigned int *)pixelsj + 1) = (packedSunWeight << 24)
        | colors->rgb[52][2]
        | (colors->rgb[52][1] << 8)
        | (colors->rgb[52][0] << 16);
    *((unsigned int *)pixelsj + 2) = (packedSunWeight << 24)
        | colors->rgb[55][2]
        | (colors->rgb[55][1] << 8)
        | (colors->rgb[55][0] << 16);
    *((unsigned int *)pixelsj + 3) = (packedSunWeight << 24)
        | colors->rgb[35][2]
        | (colors->rgb[35][1] << 8)
        | (colors->rgb[35][0] << 16);
    pixelsk = &pixelsj[s_lightGridRowDelta];
    *(unsigned int *)pixelsk = (packedSunWeight << 24)
        | colors->rgb[36][2]
        | (colors->rgb[36][1] << 8)
        | (colors->rgb[36][0] << 16);
    *((unsigned int *)pixelsk + 1) = (packedSunWeight << 24)
        | colors->rgb[37][2]
        | (colors->rgb[37][1] << 8)
        | (colors->rgb[37][0] << 16);
    *((unsigned int *)pixelsk + 2) = (packedSunWeight << 24)
        | colors->rgb[38][2]
        | (colors->rgb[38][1] << 8)
        | (colors->rgb[38][0] << 16);
    *((unsigned int *)pixelsk + 3) = (packedSunWeight << 24)
        | colors->rgb[39][2]
        | (colors->rgb[39][1] << 8)
        | (colors->rgb[39][0] << 16);
    pixelsl = &pixelsk[s_lightGridSliceDelta];
    *(unsigned int *)pixelsl = (packedSunWeight << 24)
        | colors->rgb[40][2]
        | (colors->rgb[40][1] << 8)
        | (colors->rgb[40][0] << 16);
    *((unsigned int *)pixelsl + 1) = (packedSunWeight << 24)
        | colors->rgb[41][2]
        | (colors->rgb[41][1] << 8)
        | (colors->rgb[41][0] << 16);
    *((unsigned int *)pixelsl + 2) = (packedSunWeight << 24)
        | colors->rgb[42][2]
        | (colors->rgb[42][1] << 8)
        | (colors->rgb[42][0] << 16);
    *((unsigned int *)pixelsl + 3) = (packedSunWeight << 24)
        | colors->rgb[43][2]
        | (colors->rgb[43][1] << 8)
        | (colors->rgb[43][0] << 16);
    pixelsm = &pixelsl[s_lightGridRowDelta];
    *(unsigned int *)pixelsm = (packedSunWeight << 24)
        | colors->rgb[44][2]
        | (colors->rgb[44][1] << 8)
        | (colors->rgb[44][0] << 16);
    *((unsigned int *)pixelsm + 1) = (packedSunWeight << 24)
        | colors->rgb[45][2]
        | (colors->rgb[45][1] << 8)
        | (colors->rgb[45][0] << 16);
    *((unsigned int *)pixelsm + 2) = (packedSunWeight << 24)
        | colors->rgb[46][2]
        | (colors->rgb[46][1] << 8)
        | (colors->rgb[46][0] << 16);
    *((unsigned int *)pixelsm + 3) = (packedSunWeight << 24)
        | colors->rgb[47][2]
        | (colors->rgb[47][1] << 8)
        | (colors->rgb[47][0] << 16);
    pixelsn = &pixelsm[s_lightGridRowDelta];
    *(unsigned int *)pixelsn = (packedSunWeight << 24)
        | colors->rgb[48][2]
        | (colors->rgb[48][1] << 8)
        | (colors->rgb[48][0] << 16);
    *((unsigned int *)pixelsn + 1) = (packedSunWeight << 24)
        | colors->rgb[49][2]
        | (colors->rgb[49][1] << 8)
        | (colors->rgb[49][0] << 16);
    *((unsigned int *)pixelsn + 2) = (packedSunWeight << 24)
        | colors->rgb[50][2]
        | (colors->rgb[50][1] << 8)
        | (colors->rgb[50][0] << 16);
    *((unsigned int *)pixelsn + 3) = (packedSunWeight << 24)
        | colors->rgb[51][2]
        | (colors->rgb[51][1] << 8)
        | (colors->rgb[51][0] << 16);
    pixelso = &pixelsn[s_lightGridRowDelta];
    *(unsigned int *)pixelso = (packedSunWeight << 24)
        | colors->rgb[52][2]
        | (colors->rgb[52][1] << 8)
        | (colors->rgb[52][0] << 16);
    *((unsigned int *)pixelso + 1) = (packedSunWeight << 24)
        | colors->rgb[53][2]
        | (colors->rgb[53][1] << 8)
        | (colors->rgb[53][0] << 16);
    *((unsigned int *)pixelso + 2) = (packedSunWeight << 24)
        | colors->rgb[54][2]
        | (colors->rgb[54][1] << 8)
        | (colors->rgb[54][0] << 16);
    *((unsigned int *)pixelso + 3) = (packedSunWeight << 24)
        | colors->rgb[55][2]
        | (colors->rgb[55][1] << 8)
        | (colors->rgb[55][0] << 16);
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
        R_WeightedAccumulateLightGridColors(
            &lightGrid->colors[colorsIndex[colorsIter]],
            fixedPointWeight[colorsIter],
            accumulated);
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
        scaled[sampleIter] = fixedPointWeight * colors->rgb[0][sampleIter];
        scaled[sampleIter + 1] = fixedPointWeight * colors->rgb[0][sampleIter + 1];
        scaled[sampleIter + 2] = fixedPointWeight * colors->rgb[0][sampleIter + 2];
        scaled[sampleIter + 3] = fixedPointWeight * colors->rgb[1][sampleIter];
        scaled[sampleIter + 4] = fixedPointWeight * colors->rgb[1][sampleIter + 1];
        scaled[sampleIter + 5] = fixedPointWeight * colors->rgb[1][sampleIter + 2];
        scaled[sampleIter + 6] = fixedPointWeight * colors->rgb[2][sampleIter];
        scaled[sampleIter + 7] = fixedPointWeight * colors->rgb[2][sampleIter + 1];
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
        accumulated[sampleIter] += fixedPointWeight * colors->rgb[0][sampleIter];
        accumulated[sampleIter + 1] += fixedPointWeight * colors->rgb[0][sampleIter + 1];
        accumulated[sampleIter + 2] += fixedPointWeight * colors->rgb[0][sampleIter + 2];
        accumulated[sampleIter + 3] += fixedPointWeight * colors->rgb[1][sampleIter];
        accumulated[sampleIter + 4] += fixedPointWeight * colors->rgb[1][sampleIter + 1];
        accumulated[sampleIter + 5] += fixedPointWeight * colors->rgb[1][sampleIter + 2];
        accumulated[sampleIter + 6] += fixedPointWeight * colors->rgb[2][sampleIter];
        accumulated[sampleIter + 7] += fixedPointWeight * colors->rgb[2][sampleIter + 1];
    }
}

void __cdecl R_PackAccumulatedLightGridColors(const unsigned __int16 *accumulated, GfxLightGridColors *packed)
{
    unsigned int sampleIter; // [esp+4h] [ebp-4h]

    for (sampleIter = 0; sampleIter < 0xA8; sampleIter += 8)
    {
        packed->rgb[0][sampleIter] = (unsigned __int16)(accumulated[sampleIter] + 127) >> 8;
        packed->rgb[0][sampleIter + 1] = (unsigned __int16)(accumulated[sampleIter + 1] + 127) >> 8;
        packed->rgb[0][sampleIter + 2] = (unsigned __int16)(accumulated[sampleIter + 2] + 127) >> 8;
        packed->rgb[1][sampleIter] = (unsigned __int16)(accumulated[sampleIter + 3] + 127) >> 8;
        packed->rgb[1][sampleIter + 1] = (unsigned __int16)(accumulated[sampleIter + 4] + 127) >> 8;
        packed->rgb[1][sampleIter + 2] = (unsigned __int16)(accumulated[sampleIter + 5] + 127) >> 8;
        packed->rgb[2][sampleIter] = (unsigned __int16)(accumulated[sampleIter + 6] + 127) >> 8;
        packed->rgb[2][sampleIter + 1] = (unsigned __int16)(accumulated[sampleIter + 7] + 127) >> 8;
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

    if (!lightGrid)
        MyAssertHandler(".\\rb_light.cpp", 1190, 0, "%s", "lightGrid");
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

    if (!lightGrid)
        MyAssertHandler(".\\rb_light.cpp", 1083, 0, "%s", "lightGrid");
    v9 = floor(*samplePos);
    pos[0] = ((int)v9 + 0x20000) >> 5;
    v8 = floor(samplePos[1]);
    pos[1] = ((int)v8 + 0x20000) >> 5;
    v7 = floor(samplePos[2]);
    pos[2] = ((int)v7 + 0x20000) >> 6;
    if ((lightGrid->rowAxis || lightGrid->colAxis != 1) && (lightGrid->rowAxis != 1 || lightGrid->colAxis))
        MyAssertHandler(
            ".\\rb_light.cpp",
            1092,
            1,
            "%s",
            "(lightGrid->rowAxis == 0 && lightGrid->colAxis == 1) || (lightGrid->rowAxis == 1 && lightGrid->colAxis == 0)");
    axisLerp[0] = (samplePos[lightGrid->rowAxis] - -131072.0) * 0.03125 - (double)pos[lightGrid->rowAxis];
    axisLerp[1] = (samplePos[lightGrid->colAxis] - -131072.0) * 0.03125 - (double)pos[lightGrid->colAxis];
    axisLerp[2] = (samplePos[2] - -131072.0) * 0.015625 - (double)pos[2];
    quadWeight = (1.0 - axisLerp[1]) * (1.0 - axisLerp[2]);
    *cornerWeight = (1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[4] = quadWeight * axisLerp[0];
    quadWeight = (1.0 - axisLerp[1]) * axisLerp[2];
    cornerWeight[1] = (1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[5] = quadWeight * axisLerp[0];
    quadWeight = (1.0 - axisLerp[2]) * axisLerp[1];
    cornerWeight[2] = (1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[6] = quadWeight * axisLerp[0];
    quadWeight = axisLerp[1] * axisLerp[2];
    cornerWeight[3] = (1.0 - axisLerp[0]) * quadWeight;
    cornerWeight[7] = quadWeight * axisLerp[0];
    *defaultGridEntry = 1;
    R_GetLightGridSampleEntryQuad(lightGrid, pos, cornerEntry, defaultGridEntry);
    ++pos[lightGrid->rowAxis];
    R_GetLightGridSampleEntryQuad(lightGrid, pos, cornerEntry + 4, defaultGridEntry);
    --pos[lightGrid->rowAxis];
    if (r_vc_makelog->current.integer)
        R_UpdateVisHistory(lightGrid, pos);
    primaryLightIndex = 0;
    bestPrimaryLightWeight = 0.0;
    honorSuppression = 0;
    cornerIndex = 0;
    cornerTraceBit = 1;
    while (cornerIndex < 8)
    {
        entry = cornerEntry[cornerIndex];
        if (!entry)
            goto LABEL_12;
        if (cornerWeight[cornerIndex] < 0.001000000047497451)
        {
            cornerEntry[cornerIndex] = 0;
            goto LABEL_12;
        }
        v6 = ((unsigned __int8)cornerTraceBit & entry->needsTrace) != 0
            && !R_IsValidLightGridSample(lightGrid, entry, cornerIndex, pos, samplePos);
        suppressEntry = v6;
        suppressEntryLog[cornerIndex] = v6;
        if (suppressEntry)
        {
            if (honorSuppression)
            {
                cornerEntry[cornerIndex] = 0;
                goto LABEL_12;
            }
        }
        else if (!honorSuppression)
        {
            honorSuppression = 1;
            bestPrimaryLightWeight = cornerWeight[cornerIndex];
            primaryLightIndex = entry->primaryLightIndex;
            memset((unsigned __int8 *)cornerEntry, 0, 4 * cornerIndex);
            goto LABEL_12;
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
    LABEL_12:
        ++cornerIndex;
        cornerTraceBit *= 2;
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
    if (!rgp.world)
        MyAssertHandler(".\\rb_light.cpp", 126, 0, "%s", "rgp.world");
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
            x = *pos + ((xBit & cornerIndex) != 0);
            y = pos[1] + ((yBit & cornerIndex) != 0);
            z = pos[2] + ((cornerIndex & 1) != 0);
            if (((1 << cornerIndex) & cornerEntry[cornerIndex]->needsTrace) != 0)
            {
                if (suppressEntry[cornerIndex])
                {
                    if (honorSuppression)
                        R_ShowGridCorner(x, y, z, 1.1, colorRed);
                    else
                        R_ShowGridCorner(x, y, z, 1.1, colorOrange);
                }
                else
                {
                    R_ShowGridCorner(x, y, z, 1.0, colorYellow);
                }
            }
            else
            {
                R_ShowGridCorner(x, y, z, 0.89999998, colorGreen);
            }
        }
    }
}

void __cdecl R_ShowGridOrigin(const float *origin)
{
    float boxHalfSize[3]; // [esp+0h] [ebp-24h] BYREF
    float boxMaxs[3]; // [esp+Ch] [ebp-18h] BYREF
    float boxMins[3]; // [esp+18h] [ebp-Ch] BYREF

    boxHalfSize[0] = 0.5;
    boxHalfSize[1] = 0.5;
    boxHalfSize[2] = 0.5;
    Vec3Sub(origin, boxHalfSize, boxMins);
    Vec3Add(origin, boxHalfSize, boxMaxs);
    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, colorBlue);
}

void __cdecl R_ShowGridBox(const unsigned int *pos)
{
    float origin; // [esp+18h] [ebp-24h]
    float origin_4; // [esp+1Ch] [ebp-20h]
    float origin_8; // [esp+20h] [ebp-1Ch]
    float boxMaxs[3]; // [esp+24h] [ebp-18h] BYREF
    float boxMins[3]; // [esp+30h] [ebp-Ch] BYREF

    origin = (double)*pos * 32.0 + -131072.0;
    origin_4 = (double)pos[1] * 32.0 + -131072.0;
    origin_8 = (double)pos[2] * 64.0 + -131072.0;
    boxMins[0] = origin;
    boxMins[1] = origin_4;
    boxMins[2] = origin_8;
    boxMaxs[0] = origin + 32.0;
    boxMaxs[1] = origin_4 + 32.0;
    boxMaxs[2] = origin_8 + 64.0;
    R_AddDebugBox(&frontEndDataOut->debugGlobals, boxMins, boxMaxs, colorWhite);
}

void __cdecl R_ShowGridCorner(unsigned int x, unsigned int y, unsigned int z, float halfSize, const float *color)
{
    float origin; // [esp+18h] [ebp-24h]
    float origin_4; // [esp+1Ch] [ebp-20h]
    float origin_8; // [esp+20h] [ebp-1Ch]
    float boxMaxs[3]; // [esp+24h] [ebp-18h] BYREF
    float boxMins[3]; // [esp+30h] [ebp-Ch] BYREF

    origin = (double)x * 32.0 + -131072.0;
    origin_4 = (double)y * 32.0 + -131072.0;
    origin_8 = (double)z * 64.0 + -131072.0;
    boxMins[0] = origin - halfSize;
    boxMins[1] = origin_4 - halfSize;
    boxMins[2] = origin_8 - halfSize;
    boxMaxs[0] = halfSize + origin;
    boxMaxs[1] = halfSize + origin_4;
    boxMaxs[2] = halfSize + origin_8;
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
        *entries = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        return;
    }
    if (4 * (unsigned int)lightGrid->rowDataStart[rowIndex] >= lightGrid->rawRowDataSize)
        MyAssertHandler(
            ".\\rb_light.cpp",
            904,
            0,
            "lightGrid->rowDataStart[rowIndex] * 4 doesn't index lightGrid->rawRowDataSize\n\t%i not in [0, %i)",
            4 * lightGrid->rowDataStart[rowIndex],
            lightGrid->rawRowDataSize);
    row = (const GfxLightGridRow *)&lightGrid->rawRowData[4 * lightGrid->rowDataStart[rowIndex]];
    if (row->firstEntry >= lightGrid->entryCount)
        MyAssertHandler(
            ".\\rb_light.cpp",
            906,
            0,
            "row->firstEntry doesn't index lightGrid->entryCount\n\t%i not in [0, %i)",
            row->firstEntry,
            lightGrid->entryCount);
    colIndex = pos[lightGrid->colAxis] - row->colStart;
    z = pos[2] - row->zStart;
    if (colIndex + 1 > row->colCount)
    {
        *entries = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        return;
    }
    if (z + 1 > row->zCount)
    {
        *entries = 0;
        entries[1] = 0;
        entries[2] = 0;
        entries[3] = 0;
        if (pos[2] < row->zStart)
            *defaultGridEntry = 0;
        return;
    }
    firstBlockEntry = row->firstEntry;
    rleData = (const unsigned __int8 *)&row[1];
    if (colIndex == -1)
    {
        *entries = 0;
        entries[1] = 0;
        baseZ = LOBYTE(row[1].colCount);
        if (row->zCount > 0xFFu)
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
        rleSizeFull = (row->zCount > 0xFFu) + 3;
        while (colIndex >= *rleData)
        {
            colIndex -= *rleData;
            firstBlockEntry += rleData[1] * *rleData;
            if (rleData[1])
                v11 = (row->zCount > 0xFFu) + 3;
            else
                v11 = 2;
            rleData += v11;
        }
        if (rleData[1])
        {
            baseZb = rleData[2];
            if (row->zCount > 0xFFu)
                baseZb += rleData[3] << 8;
            if (z < baseZb)
                *defaultGridEntry = 0;
            localZ = z - baseZb;
            lookupa = z - baseZb + firstBlockEntry + colIndex * rleData[1];
            if (z - baseZb < rleData[1])
                v10 = &lightGrid->entries[lookupa];
            else
                v10 = 0;
            *entries = v10;
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
            *entries = 0;
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

    if (!lightGrid)
        MyAssertHandler(".\\rb_light.cpp", 1046, 0, "%s", "lightGrid");
    if (lightGrid->rowAxis >= 3)
        MyAssertHandler(
            ".\\rb_light.cpp",
            1047,
            0,
            "lightGrid->rowAxis doesn't index 3\n\t%i not in [0, %i)",
            lightGrid->rowAxis,
            3);
    if (lightGrid->colAxis >= 3)
        MyAssertHandler(
            ".\\rb_light.cpp",
            1048,
            0,
            "lightGrid->colAxis doesn't index 3\n\t%i not in [0, %i)",
            lightGrid->colAxis,
            3);
    gridPos[0] = (double)*pos * 32.0 + -131072.0;
    gridPos[1] = (double)pos[1] * 32.0 + -131072.0;
    gridPos[2] = (double)pos[2] * 64.0 + -131072.0;
    gridPos[lightGrid->rowAxis] = (double)((cornerIndex & 4) != 0 ? 0x20 : 0) + gridPos[lightGrid->rowAxis];
    gridPos[lightGrid->colAxis] = (double)((cornerIndex & 2) != 0 ? 0x20 : 0) + gridPos[lightGrid->colAxis];
    gridPos[2] = (double)((cornerIndex & 1) != 0 ? 0x40 : 0) + gridPos[2];
    Vec3Sub(samplePos, gridPos, traceDir);
    Vec3Normalize(traceDir);
    Vec3Mad(gridPos, 0.0099999998, traceDir, nudgedGridPos);
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

    if (!lightGrid)
        MyAssertHandler(".\\rb_light.cpp", 1428, 0, "%s", "lightGrid");
    //Profile_Begin(85);
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
    if (maxWeight < 0.0)
        MyAssertHandler(".\\rb_light.cpp", 1463, 0, "%s\n\t(maxWeight) = %g", "(maxWeight >= 0.0f)", maxWeight);
    if (primaryVisibleWeight < 0.0)
        MyAssertHandler(
            ".\\rb_light.cpp",
            1464,
            0,
            "%s\n\t(primaryVisibleWeight) = %g",
            "(primaryVisibleWeight >= 0.0f)",
            primaryVisibleWeight);
    if (sampleCount)
    {
        if (maxWeight <= 0.0)
            MyAssertHandler(".\\rb_light.cpp", 1473, 1, "%s", "maxWeight > 0.0f");
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
    //Profile_EndInternal(0);
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
    patch->primaryLightWeight = (int)(primaryLightWeight * 255.0 + 0.5);
    patch->colorsCount = 1;
    if (colorsIndex != (unsigned __int16)colorsIndex)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            colorsIndex,
            (unsigned __int16)colorsIndex);
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
    patch->primaryLightWeight = (int)(primaryLightWeight * 255.0 + 0.5);
    if (colorsCount != (unsigned __int8)colorsCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            colorsCount,
            (unsigned __int8)colorsCount);
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
        fixedPointWeight[colorsIter] = (int)(weightNormalizeScale * 256.0 * colorsWeight[colorsIter] + 0.5);
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
    float v5; // [esp+14h] [ebp-24h]
    float v6; // [esp+18h] [ebp-20h]
    float v7; // [esp+1Ch] [ebp-1Ch]
    float v8; // [esp+20h] [ebp-18h]
    float v9; // [esp+24h] [ebp-14h]
    float v10; // [esp+28h] [ebp-10h]
    float gridPos[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if (light->type == 1)
        return 1;
    v10 = *samplePos * 0.03125;
    v7 = floor(v10);
    gridPos[0] = v7 * 32.0;
    v9 = samplePos[1] * 0.03125;
    v6 = floor(v9);
    gridPos[1] = v6 * 32.0;
    v8 = *samplePos * 0.015625;
    v5 = floor(v8);
    gridPos[2] = v5 * 64.0;
    gridPos[lightGrid->rowAxis] = (double)((cornerIndex & 4) != 0 ? 0x20 : 0) + gridPos[lightGrid->rowAxis];
    gridPos[lightGrid->colAxis] = (double)((cornerIndex & 2) != 0 ? 0x20 : 0) + gridPos[lightGrid->colAxis];
    gridPos[2] = (double)((cornerIndex & 1) != 0 ? 0x40 : 0) + gridPos[2];
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
    if (maxWeight < 0.0)
        MyAssertHandler(".\\rb_light.cpp", 1542, 0, "%s\n\t(maxWeight) = %g", "(maxWeight >= 0.0f)", maxWeight);
    if (primaryWeight < 0.0)
        MyAssertHandler(".\\rb_light.cpp", 1543, 0, "%s\n\t(primaryWeight) = %g", "(primaryWeight >= 0.0f)", primaryWeight);
    if (sampleCount)
    {
        weightNormalizeScale = 1.0 / maxWeight;
        primaryWeight = primaryWeight * weightNormalizeScale;
        if (primaryWeight < 0.0)
            MyAssertHandler(
                ".\\rb_light.cpp",
                1557,
                0,
                "%s\n\t(primaryWeight) = %g",
                "(primaryWeight >= 0.0f)",
                primaryWeight);
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
        v5 = primaryLightColor[colorIndex] * ((double)colorWithSunAlpha[3] * 0.5) + (double)colorWithSunAlpha[colorIndex];
        v4 = v5 - 255.0;
        if (v4 < 0.0)
            v6 = primaryLightColor[colorIndex] * ((double)colorWithSunAlpha[3] * 0.5) + (double)colorWithSunAlpha[colorIndex];
        else
            v6 = 255.0;
        v3 = 0.0 - v5;
        if (v3 < 0.0)
            v2 = v6;
        else
            v2 = 0.0;
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
                    //Profile_Begin(166);
                    memcpy((unsigned __int8 *)s_vc_log.history, (unsigned __int8 *)buffer, count);
                    //Profile_EndInternal(0);
                    s_vc_log.count = count / 6u;
                }
                FS_FreeFile((char *)buffer);
            }
        }
    }
}

void __cdecl R_LightVisHistoryFilename(char *bspName, char *filename)
{
    if (!bspName)
        MyAssertHandler(".\\rb_light.cpp", 1583, 0, "%s", "bspName");
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

