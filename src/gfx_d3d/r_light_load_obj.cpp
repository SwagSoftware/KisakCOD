#include "r_light.h"
#include <qcommon/com_bsp.h>
#include "r_bsp.h"

#include <algorithm>
#include <universal/com_files.h>
#include "r_image.h"

const float s_lightGridRotAxis[3][3] =
{
  { 0.47140452f, 0.0f, 0.33333334f },
  { -0.23570226f, 0.40824828f, 0.33333334f },
  { -0.23570226f, -0.40824828f, 0.33333334f }
}; // idb
const float standardFrustumSidePlanes[4][4] =
{
  { -1.0f, 0.0f, 0.0f, 1.0f },
  { 1.0f, 0.0f, 0.0f, 1.0f },
  { 0.0f, -1.0f, 0.0f, 1.0f },
  { 0.0f, 1.0f, 0.0f, 1.0f }
}; // idb

int s_lmapPixelsUsedForFalloff;

void __cdecl R_ModernizeLegacyLightGridColors(const unsigned __int8 *legacyColors, GfxLightGridColors *modernColors)
{
    float v2; // [esp+18h] [ebp-BCh]
    float scale; // [esp+1Ch] [ebp-B8h]
    float v4; // [esp+20h] [ebp-B4h]
    float v5; // [esp+24h] [ebp-B0h]
    float v6; // [esp+28h] [ebp-ACh]
    float v7; // [esp+2Ch] [ebp-A8h]
    float v8; // [esp+30h] [ebp-A4h]
    float v9; // [esp+34h] [ebp-A0h]
    float v10; // [esp+38h] [ebp-9Ch]
    float v11; // [esp+3Ch] [ebp-98h]
    float projected[3]; // [esp+58h] [ebp-7Ch] BYREF
    float cornerWeight[8]; // [esp+64h] [ebp-70h]
    float delta[3]; // [esp+84h] [ebp-50h] BYREF
    float colorSum; // [esp+90h] [ebp-44h]
    int channelIndex; // [esp+94h] [ebp-40h]
    int z; // [esp+98h] [ebp-3Ch]
    float axisWeight[2][3]; // [esp+9Ch] [ebp-38h]
    int cornerIndex; // [esp+B4h] [ebp-20h]
    float rotated[3]; // [esp+B8h] [ebp-1Ch] BYREF
    int x; // [esp+C4h] [ebp-10h]
    int y; // [esp+C8h] [ebp-Ch]
    float length; // [esp+CCh] [ebp-8h]
    int basisIndex; // [esp+D0h] [ebp-4h]

    basisIndex = 0;
    for (z = 0; z < 4; ++z)
    {
        delta[2] = z * 0.6666666865348816 - 1.0;
        for (y = 0; y < 4; ++y)
        {
            delta[1] = y * 0.6666666865348816 - 1.0;
            for (x = 0; x < 4; ++x)
            {
                if (x <= 0 || x >= 3 || y <= 0 || y >= 3 || z <= 0 || z >= 3)
                {
                    delta[0] = x * 0.6666666865348816 - 1.0;
                    rotated[0] = Vec3Dot(delta, s_lightGridRotAxis[0]);
                    rotated[1] = Vec3Dot(delta, s_lightGridRotAxis[1]);
                    rotated[2] = Vec3Dot(delta, s_lightGridRotAxis[2]);
                    v9 = I_fabs(rotated[2]);
                    v8 = I_fabs(rotated[1]);
                    v7 = v8 - v9;
                    if (v7 < 0.0)
                        v11 = v9;
                    else
                        v11 = v8;
                    v6 = I_fabs(rotated[0]);
                    v5 = v6 - v11;
                    if (v5 < 0.0)
                        v4 = v11;
                    else
                        v4 = v6;
                    length = v4;
                    if (v4 <= 0.0)
                        MyAssertHandler(".\\r_bsp_load_obj.cpp", 812, 1, "%s", "length > 0.0f");
                    scale = 1.0 / length;
                    Vec3Scale(rotated, scale, projected);
                    axisWeight[0][0] = projected[0] * 0.5 + 0.5;
                    axisWeight[1][0] = 1.0 - axisWeight[0][0];
                    axisWeight[0][1] = projected[1] * 0.5 + 0.5;
                    axisWeight[1][1] = 1.0 - axisWeight[0][1];
                    axisWeight[0][2] = projected[2] * 0.5 + 0.5;
                    axisWeight[1][2] = 1.0 - axisWeight[0][2];
                    cornerWeight[0] = axisWeight[0][0] * axisWeight[0][1] * axisWeight[0][2];
                    cornerWeight[1] = axisWeight[1][0] * axisWeight[0][1] * axisWeight[0][2];
                    cornerWeight[2] = axisWeight[0][0] * axisWeight[1][1] * axisWeight[0][2];
                    cornerWeight[3] = axisWeight[1][0] * axisWeight[1][1] * axisWeight[0][2];
                    cornerWeight[4] = axisWeight[0][0] * axisWeight[0][1] * axisWeight[1][2];
                    cornerWeight[5] = axisWeight[1][0] * axisWeight[0][1] * axisWeight[1][2];
                    cornerWeight[6] = axisWeight[0][0] * axisWeight[1][1] * axisWeight[1][2];
                    cornerWeight[7] = axisWeight[1][0] * axisWeight[1][1] * axisWeight[1][2];
                    v10 = cornerWeight[7]
                        + cornerWeight[6]
                        + cornerWeight[5]
                        + cornerWeight[4]
                        + cornerWeight[3]
                        + cornerWeight[2]
                        + cornerWeight[1]
                        + cornerWeight[0]
                        - 1.0;
                    v2 = I_fabs(v10);
                    if (v2 >= EQUAL_EPSILON)
                        MyAssertHandler(
                            ".\\r_bsp_load_obj.cpp",
                            830,
                            0,
                            "%s",
                            "I_I_fabs( cornerWeight[0] + cornerWeight[1] + cornerWeight[2] + cornerWeight[3] + cornerWeight[4] + cornerWe"
                            "ight[5] + cornerWeight[6] + cornerWeight[7] - 1.0f ) < 0.001f");
                    for (channelIndex = 0; channelIndex < 3; ++channelIndex)
                    {
                        colorSum = 0.0;
                        for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
                            colorSum = legacyColors[8 * channelIndex + cornerIndex] * cornerWeight[cornerIndex] + colorSum;
                        modernColors->rgb[basisIndex][channelIndex] = colorSum;
                    }
                    ++basisIndex;
                }
            }
        }
    }
    if (basisIndex != 56)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 844, 1, "%s", "basisIndex == GFX_LIGHTGRID_SAMPLE_COUNT");
}

void __cdecl R_LoadDefaultLightGridColors(GfxLightGridColors *colors)
{
    float scale; // [esp+2Ch] [ebp-70h]
    float v2; // [esp+30h] [ebp-6Ch]
    float v3; // [esp+34h] [ebp-68h]
    float v4; // [esp+38h] [ebp-64h]
    float v5; // [esp+3Ch] [ebp-60h]
    float v6; // [esp+40h] [ebp-5Ch]
    float v7; // [esp+44h] [ebp-58h]
    float v8; // [esp+48h] [ebp-54h]
    float projected[3]; // [esp+64h] [ebp-38h] BYREF
    float delta[3]; // [esp+70h] [ebp-2Ch] BYREF
    int z; // [esp+7Ch] [ebp-20h]
    float rotated[3]; // [esp+80h] [ebp-1Ch] BYREF
    int x; // [esp+8Ch] [ebp-10h]
    int y; // [esp+90h] [ebp-Ch]
    float length; // [esp+94h] [ebp-8h]
    int basisIndex; // [esp+98h] [ebp-4h]

    basisIndex = 0;
    for (z = 0; z < 4; ++z)
    {
        delta[2] = z * 0.6666666865348816 - 1.0;
        for (y = 0; y < 4; ++y)
        {
            delta[1] = y * 0.6666666865348816 - 1.0;
            for (x = 0; x < 4; ++x)
            {
                if (x <= 0 || x >= 3 || y <= 0 || y >= 3 || z <= 0 || z >= 3)
                {
                    delta[0] = x * 0.6666666865348816 - 1.0;
                    rotated[0] = Vec3Dot(delta, s_lightGridRotAxis[0]);
                    rotated[1] = Vec3Dot(delta, s_lightGridRotAxis[1]);
                    rotated[2] = Vec3Dot(delta, s_lightGridRotAxis[2]);
                    v7 = I_fabs(rotated[2]);
                    v6 = I_fabs(rotated[1]);
                    v5 = v6 - v7;
                    if (v5 < 0.0)
                        v8 = v7;
                    else
                        v8 = v6;
                    v4 = I_fabs(rotated[0]);
                    v3 = v4 - v8;
                    if (v3 < 0.0)
                        v2 = v8;
                    else
                        v2 = v4;
                    length = v2;
                    if (v2 <= 0.0)
                        MyAssertHandler(".\\r_bsp_load_obj.cpp", 877, 1, "%s", "length > 0.0f");
                    scale = 1.0 / length;
                    Vec3Scale(rotated, scale, projected);
                    colors->rgb[basisIndex][0] = ((projected[0] * 0.5 + 0.5) * 255.0);
                    colors->rgb[basisIndex][1] = ((projected[1] * 0.5 + 0.5) * 255.0);
                    colors->rgb[basisIndex++][2] = ((projected[2] * 0.5 + 0.5) * 255.0);
                }
            }
        }
    }
    if (basisIndex != 56)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 887, 1, "%s", "basisIndex == GFX_LIGHTGRID_SAMPLE_COUNT");
}

void __cdecl R_LoadLightGridColors(unsigned int bspVersion)
{
    char *rawColorData; // [esp+0h] [ebp-Ch]
    unsigned int colorIndex; // [esp+8h] [ebp-4h]

    rawColorData = Com_GetBspLump(LUMP_LIGHTGRIDCOLORS, bspVersion > 10 ? 168 : 24, &s_world.lightGrid.colorCount);
    s_world.lightGrid.colors = (GfxLightGridColors *)Hunk_Alloc(168 * (s_world.lightGrid.colorCount + 1), "R_LoadLightGridColors", 20);
    if (bspVersion > 10)
    {
        Com_Memcpy(s_world.lightGrid.colors, rawColorData, 168 * s_world.lightGrid.colorCount);
    }
    else
    {
        for (colorIndex = 0; colorIndex < s_world.lightGrid.colorCount; ++colorIndex)
            R_ModernizeLegacyLightGridColors((unsigned char *)&rawColorData[24 * colorIndex], &s_world.lightGrid.colors[colorIndex]);
    }
    R_LoadDefaultLightGridColors(&s_world.lightGrid.colors[s_world.lightGrid.colorCount]);
    ++s_world.lightGrid.colorCount;
}

void R_LoadLightGridRowData()
{
    char *rawData; // [esp+0h] [ebp-4h]

    rawData = Com_GetBspLump(LUMP_LIGHTGRIDROWS, 1u, &s_world.lightGrid.rawRowDataSize);
    if (s_world.lightGrid.rawRowDataSize)
    {
        s_world.lightGrid.rawRowData = Hunk_Alloc(s_world.lightGrid.rawRowDataSize, "R_LoadLightGridRowData", 20);
        Com_Memcpy(s_world.lightGrid.rawRowData, rawData, s_world.lightGrid.rawRowDataSize);
    }
    else
    {
        s_world.lightGrid.rawRowData = 0;
    }
}

unsigned __int8 *R_LoadLightGridEntries()
{
    unsigned __int8 *result; // eax
    GfxLightGridEntry *out; // [esp+0h] [ebp-Ch]
    unsigned int entryIndex; // [esp+4h] [ebp-8h]
    char *in; // [esp+8h] [ebp-4h]

    in = Com_GetBspLump(LUMP_LIGHTGRIDENTRIES, 4u, &s_world.lightGrid.entryCount);
    result = (unsigned __int8 *)Hunk_Alloc(4 * s_world.lightGrid.entryCount, "R_LoadLightGridPoints", 20);
    s_world.lightGrid.entries = (GfxLightGridEntry *)result;
    out = (GfxLightGridEntry *)result;
    entryIndex = 0;
    while (entryIndex < s_world.lightGrid.entryCount)
    {
        out->colorsIndex = *in;
        out->primaryLightIndex = in[2];
        out->needsTrace = in[3];
        ++entryIndex;
        result = (unsigned char *)(in + 4);
        in += 4;
        ++out;
    }
    return result;
}

void __cdecl R_AssertLightGridValid(const GfxLightGrid *lightGrid)
{
    unsigned int rowCount; // [esp+0h] [ebp-Ch]
    const GfxLightGridRow *row; // [esp+4h] [ebp-8h]
    unsigned int rowIndex; // [esp+8h] [ebp-4h]

    if (lightGrid->mins[0] > lightGrid->maxs[0])
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1425,
            0,
            "lightGrid->mins[0] <= lightGrid->maxs[0]\n\t%i, %i",
            lightGrid->mins[0],
            lightGrid->maxs[0]);
    if (lightGrid->mins[1] > lightGrid->maxs[1])
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1426,
            0,
            "lightGrid->mins[1] <= lightGrid->maxs[1]\n\t%i, %i",
            lightGrid->mins[1],
            lightGrid->maxs[1]);
    if (lightGrid->mins[2] > lightGrid->maxs[2])
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1427,
            0,
            "lightGrid->mins[2] <= lightGrid->maxs[2]\n\t%i, %i",
            lightGrid->mins[2],
            lightGrid->maxs[2]);
    rowCount = lightGrid->maxs[lightGrid->rowAxis] + 1 - lightGrid->mins[lightGrid->rowAxis];
    if (rowCount > 0x2000)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1431,
            0,
            "rowCount <= WORLD_SIZE >> GFX_LIGHTGRID_BITS_XY\n\t%i, %i",
            rowCount,
            0x2000);
    for (rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        if (lightGrid->rowDataStart[rowIndex] != 0xFFFF)
        {
            if (4 * lightGrid->rowDataStart[rowIndex] >= lightGrid->rawRowDataSize)
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    1442,
                    0,
                    "lightGrid->rowDataStart[rowIndex] * 4 doesn't index lightGrid->rawRowDataSize\n\t%i not in [0, %i)",
                    4 * lightGrid->rowDataStart[rowIndex],
                    lightGrid->rawRowDataSize);
            row = (const GfxLightGridRow *)&lightGrid->rawRowData[4 * lightGrid->rowDataStart[rowIndex]];
            if (row->firstEntry >= lightGrid->entryCount)
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    1444,
                    0,
                    "row->firstEntry doesn't index lightGrid->entryCount\n\t%i not in [0, %i)",
                    row->firstEntry,
                    lightGrid->entryCount);
        }
    }
}

int R_InitEmptyLightGrid()
{
    int result; // eax

    s_world.lightGrid.mins[0] = 0;
    s_world.lightGrid.mins[1] = 0;
    s_world.lightGrid.mins[2] = 0;
    s_world.lightGrid.maxs[0] = 0;
    s_world.lightGrid.maxs[1] = 0;
    s_world.lightGrid.maxs[2] = 0;
    s_world.lightGrid.rowAxis = 0;
    s_world.lightGrid.colAxis = 1;
    s_world.lightGrid.rowDataStart = (unsigned short *)Hunk_Alloc(2u, "R_InitEmptyLightGrid", 20);
    result = 0xFFFF;
    *s_world.lightGrid.rowDataStart = -1;
    return result;
}

void R_LoadLightGridHeader()
{
    char *header; // [esp+0h] [ebp-Ch]
    unsigned int len; // [esp+4h] [ebp-8h] BYREF
    unsigned int rowCount; // [esp+8h] [ebp-4h]

    header = Com_GetBspLump(LUMP_LIGHTGRIDHEADER, 1u, &len);
    if (len < 0x14)
        Com_Error(ERR_DROP, "light grid header is truncated");
    
    const uint16* header_u16 = (uint16*)header;
    const uint32* header_u32 = (uint32*)header;

    const uint32 header3 = header_u32[3];
    const uint32 header_off = header3 * 2;

#if 0
    rowCount = *(uint16*)&header[header_off + 6]
        - *(uint16*)&header[header_off]
        + 1;

    const uint32 rowCount_test = header_u16[header3 + 3] - header_u16[header3] + 1;
    iassert(rowCount_test == rowCount);
#else
    rowCount = header_u16[header3 + 3] - header_u16[header3] + 1;
#endif

    if (len != 2 * rowCount + 20)
        Com_Error(ERR_DROP, "light grid header has unexpected size");
    s_world.lightGrid.sunPrimaryLightIndex = s_world.sunPrimaryLightIndex;
    
    s_world.lightGrid.mins[0] = header_u16[0];
    s_world.lightGrid.mins[1] = header_u16[1];
    s_world.lightGrid.mins[2] = header_u16[2];
    s_world.lightGrid.maxs[0] = header_u16[3];
    s_world.lightGrid.maxs[1] = header_u16[4];
    s_world.lightGrid.maxs[2] = header_u16[5];

    s_world.lightGrid.rowAxis = header_u32[3];
    s_world.lightGrid.colAxis = header_u32[4];
    
    s_world.lightGrid.rowDataStart = (unsigned short *)Hunk_Alloc(2 * rowCount, "R_LoadLightGridHeader", 20);
    
    Com_Memcpy(s_world.lightGrid.rowDataStart, header + 20, 2 * rowCount);
    if (s_world.lightGrid.entryCount)
        R_AssertLightGridValid(&s_world.lightGrid);
    else
        R_InitEmptyLightGrid();
}

bool __cdecl R_AnnotatedLightGridPointSortsBefore(const AnnotatedLightGridPoint& p0, const AnnotatedLightGridPoint& p1)
{
    if (p0.pos[s_world.lightGrid.rowAxis] < p1.pos[s_world.lightGrid.rowAxis])
        return 1;
    if (p0.pos[s_world.lightGrid.rowAxis] > p1.pos[s_world.lightGrid.rowAxis])
        return 0;
    if (p0.pos[s_world.lightGrid.colAxis] < p1.pos[s_world.lightGrid.colAxis])
        return 1;
    if (p0.pos[s_world.lightGrid.colAxis] <= p1.pos[s_world.lightGrid.colAxis])
        return p0.pos[2] < p1.pos[2];
    return 0;
}

void __cdecl R_EmitLightGridEntry_Version15(const AnnotatedLightGridPoint *point)
{
    s_world.lightGrid.entries[s_world.lightGrid.entryCount++] = point->entry;
}

unsigned int R_EmitDefaultLightGridEntry_Version15()
{
    unsigned int result; // eax

    s_world.lightGrid.entries[s_world.lightGrid.entryCount].colorsIndex = 0;
    s_world.lightGrid.entries[s_world.lightGrid.entryCount].primaryLightIndex = s_world.lightGrid.sunPrimaryLightIndex;
    result = s_world.lightGrid.entryCount;
    s_world.lightGrid.entries[s_world.lightGrid.entryCount++].needsTrace = 0;
    return result;
}

char __cdecl R_EmitLightGridBlock_Version15(
    const AnnotatedLightGridPoint *pointsArray,
    signed int runCount,
    const unsigned __int16 *zSubRange,
    const unsigned __int16 *zRangeGlobal,
    unsigned int beginBlock,
    unsigned int endBlock)
{
    signed int v7; // [esp+0h] [ebp-20h]
    __int16 zBase; // [esp+8h] [ebp-18h]
    unsigned int zOffset; // [esp+Ch] [ebp-14h]
    unsigned int height; // [esp+10h] [ebp-10h]
    unsigned int pointIndex; // [esp+14h] [ebp-Ch]
    unsigned int colOffset; // [esp+18h] [ebp-8h]
    bool zBaseUsesShort; // [esp+1Fh] [ebp-1h]

    height = zSubRange[1] - *zSubRange + 1;
    if (height <= 0xFF)
    {
        if (height * runCount < endBlock - beginBlock)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                1058,
                0,
                "runCount * height >= endBlock - beginBlock\n\t%i, %i",
                height * runCount,
                endBlock - beginBlock);
        pointIndex = beginBlock;
        for (colOffset = 0; colOffset < runCount; ++colOffset)
        {
            for (zOffset = 0; zOffset < height; ++zOffset)
            {
                if (pointsArray[pointIndex].pos[2] == zOffset + pointsArray[beginBlock].pos[2])
                {
                    if (pointsArray[pointIndex].pos[s_world.lightGrid.colAxis] != colOffset
                        + pointsArray[beginBlock].pos[s_world.lightGrid.colAxis])
                        MyAssertHandler(
                            ".\\r_bsp_load_obj.cpp",
                            1067,
                            0,
                            "%s",
                            "pointsArray[pointIndex].pos[s_world.lightGrid.colAxis] == pointsArray[beginBlock].pos[s_world.lightGrid.co"
                            "lAxis] + colOffset");
                    R_EmitLightGridEntry_Version15(&pointsArray[pointIndex++]);
                }
                else
                {
                    R_EmitDefaultLightGridEntry_Version15();
                }
            }
        }
        if (pointIndex != endBlock)
            MyAssertHandler(".\\r_bsp_load_obj.cpp", 1077, 0, "%s", "pointIndex == endBlock");
        zBase = *zSubRange - *zRangeGlobal;
        zBaseUsesShort = zRangeGlobal[1] - *zRangeGlobal + 1 > 255;
        do
        {
            if (runCount > 255)
                v7 = 255;
            else
                v7 = runCount;
            if (s_world.lightGrid.rawRowDataSize + 4 >= 0x40000)
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    1085,
                    0,
                    "%s",
                    "s_world.lightGrid.rawRowDataSize + 4 < MAX_MAP_LIGHTGRID_ROWDATA");
            s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = v7;
            s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 1] = height;
            s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 2] = zBase;
            s_world.lightGrid.rawRowDataSize += 3;
            if (zBaseUsesShort)
                s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize++] = HIBYTE(zBase);
            runCount -= v7;
        } while (runCount);
        return 1;
    }
    else
    {
        Com_PrintWarning(8, "light grid vertical variation is too extreme -- ignoring light grid");
        return 0;
    }
}

void __cdecl R_EmitEmptyLightGridBlock_Version15(unsigned int emptyCount)
{
    while (emptyCount > 0xFF)
    {
        if (s_world.lightGrid.rawRowDataSize + 2 >= 0x300000)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                1007,
                0,
                "%s",
                "s_world.lightGrid.rawRowDataSize + 2 < MAX_MAP_LIGHTGRID_POINTS * 3");
        s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = -1;
        s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 1] = 0;
        s_world.lightGrid.rawRowDataSize += 2;
        emptyCount -= 255;
    }
    if (s_world.lightGrid.rawRowDataSize + 2 >= 0x300000)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1014,
            0,
            "%s",
            "s_world.lightGrid.rawRowDataSize + 2 < MAX_MAP_LIGHTGRID_POINTS * 3");
    s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = emptyCount;
    s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize + 1] = 0;
    s_world.lightGrid.rawRowDataSize += 2;
}

char __cdecl R_CompressLightGridRow_Version15(
    const AnnotatedLightGridPoint *pointsArray,
    unsigned int beginRow,
    unsigned int endRow,
    unsigned __int16 *zRangeGlobal)
{
    unsigned __int16 run; // [esp+0h] [ebp-2Ch]
    unsigned __int16 zSubRangeRun[2]; // [esp+4h] [ebp-28h] BYREF
    unsigned int beginBlock; // [esp+8h] [ebp-24h]
    GfxLightGridRow rowHeader; // [esp+Ch] [ebp-20h]
    unsigned __int16 zSubRange[2]; // [esp+18h] [ebp-14h]
    unsigned int beginCol; // [esp+1Ch] [ebp-10h]
    unsigned int endCol; // [esp+20h] [ebp-Ch]
    unsigned __int16 colRun; // [esp+24h] [ebp-8h]
    unsigned __int16 col; // [esp+28h] [ebp-4h]

    rowHeader.firstEntry = s_world.lightGrid.entryCount;
    rowHeader.colStart = pointsArray[beginRow].pos[s_world.lightGrid.colAxis];
    rowHeader.colCount = pointsArray[endRow - 1].pos[s_world.lightGrid.colAxis] - rowHeader.colStart + 1;
    rowHeader.zStart = *zRangeGlobal;
    rowHeader.zCount = zRangeGlobal[1] - rowHeader.zStart + 1;
    //*&s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize] = rowHeader;
    memcpy(&s_world.lightGrid.rawRowData[s_world.lightGrid.rawRowDataSize], &rowHeader, sizeof(GfxLightGridRow));
    s_world.lightGrid.rawRowDataSize += 12;
    beginBlock = 0;
    zSubRangeRun[0] = -1;
    zSubRangeRun[1] = 0;
    colRun = -1;
    run = 0;
    for (beginCol = beginRow; beginCol != endRow; beginCol = endCol)
    {
        col = pointsArray[beginCol].pos[s_world.lightGrid.colAxis];
        for (endCol = beginCol + 1; endCol != endRow && pointsArray[endCol].pos[s_world.lightGrid.colAxis] == col; ++endCol)
            ;
        zSubRange[0] = pointsArray[beginCol].pos[2];
        zSubRange[1] = pointsArray[endCol - 1].pos[2];
        if (col == run + colRun && zSubRange[0] == zSubRangeRun[0] && zSubRange[1] == zSubRangeRun[1] && run < 0xFFu)
        {
            ++run;
        }
        else
        {
            if (run)
            {
                if (!R_EmitLightGridBlock_Version15(pointsArray, run, zSubRangeRun, zRangeGlobal, beginBlock, beginCol))
                    return 0;
                if (col != run + colRun)
                    R_EmitEmptyLightGridBlock_Version15(col - (run + colRun));
            }
            beginBlock = beginCol;
            zSubRangeRun[0] = zSubRange[0];
            zSubRangeRun[1] = zSubRange[1];
            colRun = col;
            run = 1;
        }
    }
    if (!R_EmitLightGridBlock_Version15(pointsArray, run, zSubRangeRun, zRangeGlobal, beginBlock, endRow))
        return 0;
    s_world.lightGrid.rawRowDataSize = (s_world.lightGrid.rawRowDataSize + 3) & 0xFFFFFFFC;
    return 1;
}

char __cdecl R_EncodeLightGrid_Version15(const AnnotatedLightGridPoint *pointsArray, unsigned int pointsArrayCount)
{
    unsigned int pointIndex; // [esp+0h] [ebp-18h]
    unsigned __int16 zRange[2]; // [esp+4h] [ebp-14h] BYREF
    unsigned int pointCount; // [esp+8h] [ebp-10h]
    unsigned __int16 row; // [esp+Ch] [ebp-Ch]
    const AnnotatedLightGridPoint *point; // [esp+10h] [ebp-8h]
    unsigned __int16 rowIndex; // [esp+14h] [ebp-4h]

    if (s_world.lightGrid.entryCount)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 1182, 0, "%s", "s_world.lightGrid.entryCount == 0");
    for (pointIndex = 0; pointIndex < pointsArrayCount; pointIndex += pointCount)
    {
        point = &pointsArray[pointIndex];
        row = point->pos[s_world.lightGrid.rowAxis];
        zRange[0] = point->pos[2];
        zRange[1] = point->pos[2];
        for (pointCount = 1; pointCount + pointIndex < pointsArrayCount; ++pointCount)
        {
            point = &pointsArray[pointCount + pointIndex];
            if (point->pos[s_world.lightGrid.rowAxis] != row)
                break;
            if (zRange[0] > point->pos[2])
                zRange[0] = point->pos[2];
            if (zRange[1] < point->pos[2])
                zRange[1] = point->pos[2];
        }
        rowIndex = row - s_world.lightGrid.mins[s_world.lightGrid.rowAxis];
        s_world.lightGrid.rowDataStart[rowIndex] = s_world.lightGrid.rawRowDataSize >> 2;
        if (4 * s_world.lightGrid.rowDataStart[rowIndex] != s_world.lightGrid.rawRowDataSize)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                1203,
                1,
                "static_cast< uint >( s_world.lightGrid.rowDataStart[rowIndex] * 4 ) == s_world.lightGrid.rawRowDataSize\n"
                "\t%i, %i",
                4 * s_world.lightGrid.rowDataStart[rowIndex],
                s_world.lightGrid.rawRowDataSize);
        if (!R_CompressLightGridRow_Version15(pointsArray, pointIndex, pointCount + pointIndex, zRange))
            return 0;
        if (s_world.lightGrid.rowDataStart[rowIndex] == s_world.lightGrid.rawRowDataSize)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                1206,
                0,
                "%s",
                "s_world.lightGrid.rowDataStart[rowIndex] != s_world.lightGrid.rawRowDataSize");
    }
    if (s_world.lightGrid.entryCount < pointsArrayCount)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1209,
            0,
            "s_world.lightGrid.entryCount >= pointsArrayCount\n\t%i, %i",
            s_world.lightGrid.entryCount,
            pointsArrayCount);
    return 1;
}

void __cdecl R_LoadLightGridPoints_Version15(unsigned int bspVersion)
{
    unsigned __int8 v1; // [esp+14h] [ebp-2E4h]
    unsigned int dstEntryIndex; // [esp+1D8h] [ebp-120h]
    unsigned int bestDefaultScore; // [esp+1DCh] [ebp-11Ch]
    GfxLightGridColors swapColors; // [esp+1E0h] [ebp-118h] BYREF
    float worldMaxs[3]; // [esp+288h] [ebp-70h] BYREF
    AnnotatedLightGridPoint *points; // [esp+294h] [ebp-64h]
    unsigned int entryIndex; // [esp+298h] [ebp-60h]
    unsigned int *defaultScore; // [esp+29Ch] [ebp-5Ch]
    unsigned __int8 needsTrace; // [esp+2A3h] [ebp-55h]
    unsigned int cornerIndex; // [esp+2A4h] [ebp-54h]
    const GfxLightGridEntry_Version15 *diskEntries; // [esp+2A8h] [ebp-50h]
    unsigned int colorsIndex; // [esp+2ACh] [ebp-4Ch]
    unsigned int rowCount; // [esp+2B0h] [ebp-48h]
    const DiskGfxCell *diskCells; // [esp+2B4h] [ebp-44h]
    unsigned int diskCellCount; // [esp+2B8h] [ebp-40h] BYREF
    float worldPos[3]; // [esp+2BCh] [ebp-3Ch] BYREF
    unsigned int entryCount; // [esp+2C8h] [ebp-30h] BYREF
    unsigned int defaultColorsIndex; // [esp+2CCh] [ebp-2Ch]
    unsigned int diskCellIndex; // [esp+2D0h] [ebp-28h]
    const DiskGfxCell_Version14 *diskCellsV14; // [esp+2D4h] [ebp-24h]
    float worldMins[3]; // [esp+2D8h] [ebp-20h] BYREF
    unsigned __int8 needsTraceSwizzle[2][8]; // [esp+2E4h] [ebp-14h] BYREF

    *(uint64*)&needsTraceSwizzle[0][0] = 0x703050106020400LL;
    *(uint64*)&needsTraceSwizzle[1][0] = 0x705030106040200LL;
    diskEntries = (const GfxLightGridEntry_Version15*)Com_GetBspLump(LUMP_LIGHTGRIDENTRIES, 8u, &entryCount);
    if (entryCount)
    {
        if (bspVersion > 0xE)
        {
            diskCells = (const DiskGfxCell *)Com_GetBspLump(LUMP_CELLS, 0x70u, &diskCellCount);
            ClearBounds(worldMins, worldMaxs);
            for (diskCellIndex = 0; diskCellIndex < diskCellCount; ++diskCellIndex)
                ExpandBounds(diskCells[diskCellIndex].mins, diskCells[diskCellIndex].maxs, worldMins, worldMaxs);
        }
        else
        {
            diskCellsV14 = (const DiskGfxCell_Version14 *)Com_GetBspLump(LUMP_CELLS, 0x34u, &diskCellCount);
            ClearBounds(worldMins, worldMaxs);
            for (diskCellIndex = 0; diskCellIndex < diskCellCount; ++diskCellIndex)
                ExpandBounds(diskCellsV14[diskCellIndex].mins, diskCellsV14[diskCellIndex].maxs, worldMins, worldMaxs);
        }
        if (!diskCellCount)
        {
            worldMins[0] = -131072.0;
            worldMins[1] = -131072.0;
            worldMins[2] = -131072.0;
            worldMaxs[0] = 131072.0;
            worldMaxs[1] = 131072.0;
            worldMaxs[2] = 131072.0;
        }
        s_world.lightGrid.sunPrimaryLightIndex = s_world.sunPrimaryLightIndex;
        s_world.lightGrid.mins[0] = -1;
        s_world.lightGrid.mins[1] = -1;
        s_world.lightGrid.mins[2] = -1;
        s_world.lightGrid.maxs[0] = 0;
        s_world.lightGrid.maxs[1] = 0;
        s_world.lightGrid.maxs[2] = 0;
        points = (AnnotatedLightGridPoint*)Hunk_AllocateTempMemory(10 * entryCount, "R_LoadLightGridPoints_Version15");
        defaultScore = Hunk_AllocateTempMemory(4 * s_world.lightGrid.colorCount, "R_LoadLightGridPoints_Version15");
        memset(defaultScore, 0, 4 * s_world.lightGrid.colorCount);
        dstEntryIndex = 0;
        for (entryIndex = 0; entryIndex < entryCount; ++entryIndex)
        {
            if (!entryIndex
                || diskEntries[entryIndex].xyzHighBits != diskEntries[entryIndex - 1].xyzHighBits
                || ((diskEntries[entryIndex - 1].xyzLowBitsAndPrimaryVis
                    ^ diskEntries[entryIndex].xyzLowBitsAndPrimaryVis)
                    & 0xFFFFFFFE) != 0)
            {
                points[dstEntryIndex].entry.colorsIndex = diskEntries[entryIndex].colorsIndex;
                v1 = (diskEntries[entryIndex].xyzLowBitsAndPrimaryVis & 1) != 0
                    ? LOBYTE(s_world.lightGrid.sunPrimaryLightIndex)
                    : 0;
                points[dstEntryIndex].entry.primaryLightIndex = v1;
                points[dstEntryIndex].entry.needsTrace = diskEntries[entryIndex].needsTrace;
                points[dstEntryIndex].pos[0] = (diskEntries[entryIndex].xyzLowBitsAndPrimaryVis >> 6) & 3
                    | (diskEntries[entryIndex].xyzHighBits >> 19) & 0x1FFC;
                points[dstEntryIndex].pos[1] = (diskEntries[entryIndex].xyzLowBitsAndPrimaryVis >> 4) & 3
                    | (diskEntries[entryIndex].xyzHighBits >> 8) & 0x1FFC;
                points[dstEntryIndex].pos[2] = (diskEntries[entryIndex].xyzLowBitsAndPrimaryVis >> 2) & 3
                    | (4 * diskEntries[entryIndex].xyzHighBits) & 0xFFC;
                worldPos[0] = (32 * points[dstEntryIndex].pos[0] - 0x20000);
                worldPos[1] = (32 * points[dstEntryIndex].pos[1] - 0x20000);
                worldPos[2] = ((points[dstEntryIndex].pos[2] << 6) - 0x20000);
                if (PointInBounds(worldPos, worldMins, worldMaxs))
                {
                    if (s_world.lightGrid.mins[0] > points[dstEntryIndex].pos[0])
                        s_world.lightGrid.mins[0] = points[dstEntryIndex].pos[0];
                    if (s_world.lightGrid.maxs[0] < points[dstEntryIndex].pos[0])
                        s_world.lightGrid.maxs[0] = points[dstEntryIndex].pos[0];
                    if (s_world.lightGrid.mins[1] > points[dstEntryIndex].pos[1])
                        s_world.lightGrid.mins[1] = points[dstEntryIndex].pos[1];
                    if (s_world.lightGrid.maxs[1] < points[dstEntryIndex].pos[1])
                        s_world.lightGrid.maxs[1] = points[dstEntryIndex].pos[1];
                    if (s_world.lightGrid.mins[2] > points[dstEntryIndex].pos[2])
                        s_world.lightGrid.mins[2] = points[dstEntryIndex].pos[2];
                    if (s_world.lightGrid.maxs[2] < points[dstEntryIndex].pos[2])
                        s_world.lightGrid.maxs[2] = points[dstEntryIndex].pos[2];
                    if (points[dstEntryIndex].entry.primaryLightIndex == s_world.lightGrid.sunPrimaryLightIndex)
                        ++defaultScore[points[dstEntryIndex].entry.colorsIndex];
                    ++dstEntryIndex;
                }
            }
        }
        entryCount = dstEntryIndex;
        if (dstEntryIndex)
        {
            bestDefaultScore = 0;
            defaultColorsIndex = 0;
            for (colorsIndex = 0; colorsIndex < s_world.lightGrid.colorCount; ++colorsIndex)
            {
                if (bestDefaultScore < defaultScore[colorsIndex])
                {
                    bestDefaultScore = defaultScore[colorsIndex];
                    defaultColorsIndex = colorsIndex;
                }
            }
            qmemcpy(&swapColors, &s_world.lightGrid.colors[defaultColorsIndex], sizeof(swapColors));
            qmemcpy(
                &s_world.lightGrid.colors[defaultColorsIndex],
                s_world.lightGrid.colors,
                sizeof(s_world.lightGrid.colors[defaultColorsIndex]));
            qmemcpy(s_world.lightGrid.colors, &swapColors, sizeof(GfxLightGridColors));
            for (entryIndex = 0; entryIndex < entryCount; ++entryIndex)
            {
                if (points[entryIndex].entry.colorsIndex)
                {
                    if (points[entryIndex].entry.colorsIndex == defaultColorsIndex)
                        points[entryIndex].entry.colorsIndex = 0;
                }
                else
                {
                    points[entryIndex].entry.colorsIndex = defaultColorsIndex;
                }
            }
            Hunk_FreeTempMemory((char*)defaultScore);
            defaultScore = 0;
            if (s_world.lightGrid.maxs[1] - s_world.lightGrid.mins[1] >= s_world.lightGrid.maxs[0]
                - s_world.lightGrid.mins[0])
            {
                s_world.lightGrid.rowAxis = 0;
                s_world.lightGrid.colAxis = 1;
            }
            else
            {
                s_world.lightGrid.rowAxis = 1;
                s_world.lightGrid.colAxis = 0;
            }
            //std::_Sort<AnnotatedLightGridPoint *, int, bool(__cdecl *)(AnnotatedLightGridPoint const &, AnnotatedLightGridPoint const &)>(
            //    points,
            //    &points[entryCount],
            //    (10 * entryCount) / 10,
            //    R_AnnotatedLightGridPointSortsBefore);
            std::sort(&points[0], &points[entryCount], R_AnnotatedLightGridPointSortsBefore);
            for (entryIndex = 0; entryIndex < entryCount; ++entryIndex)
            {
                if (points[entryIndex].entry.needsTrace)
                {
                    needsTrace = 0;
                    for (cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
                    {
                        if (((1 << cornerIndex) & points[entryIndex].entry.needsTrace) != 0)
                            needsTrace |= 1 << needsTraceSwizzle[s_world.lightGrid.rowAxis][cornerIndex];
                    }
                    points[entryIndex].entry.needsTrace = needsTrace;
                }
            }
            rowCount = s_world.lightGrid.maxs[s_world.lightGrid.rowAxis]
                - s_world.lightGrid.mins[s_world.lightGrid.rowAxis]
                + 1;
            s_world.lightGrid.rowDataStart = (unsigned short*)Hunk_Alloc(2 * rowCount, "R_LoadLightGridHeader", 20);
            memset(s_world.lightGrid.rowDataStart, 0xFFu, 2 * rowCount);
            s_world.lightGrid.rawRowData = Hunk_Alloc(0x40000u, "R_LoadLightGridRowData", 20);
            s_world.lightGrid.rawRowDataSize = 0;
            s_world.lightGrid.entries = (GfxLightGridEntry*)Hunk_Alloc(8 * entryCount, "R_LoadLightGridPoints", 20);
            s_world.lightGrid.entryCount = 0;
            if (!R_EncodeLightGrid_Version15(points, entryCount))
            {
                s_world.lightGrid.entryCount = 0;
                s_world.lightGrid.mins[0] = 0;
                s_world.lightGrid.mins[1] = 0;
                s_world.lightGrid.mins[2] = 0;
                s_world.lightGrid.maxs[0] = 0;
                s_world.lightGrid.maxs[1] = 0;
                s_world.lightGrid.maxs[2] = 0;
            }
            Hunk_FreeTempMemory((char*)points);
        }
        else
        {
            Hunk_FreeTempMemory((char *)defaultScore);
            Hunk_FreeTempMemory((char *)points);
            R_InitEmptyLightGrid();
        }
    }
    else
    {
        R_InitEmptyLightGrid();
    }
}

void __cdecl R_AllocateFalloffSpaceInLightmaps(GfxLightDef *def)
{
    int pixelsNeeded; // [esp+0h] [ebp-4h]

    if (!def)
        MyAssertHandler(".\\r_light_load_obj.cpp", 66, 0, "%s", "def");
    if (!def->attenuation.image)
        MyAssertHandler(".\\r_light_load_obj.cpp", 67, 0, "%s", "def->attenuation.image");
    pixelsNeeded = def->attenuation.image->width + 2;
    if (pixelsNeeded + s_lmapPixelsUsedForFalloff > 512)
        Com_Error(
            ERR_DROP,
            "Total pixel width of all attenuation textures plus 2 border pixels is %i > %i",
            pixelsNeeded + s_lmapPixelsUsedForFalloff,
            512);
    def->lmapLookupStart = s_lmapPixelsUsedForFalloff + 1;
    s_lmapPixelsUsedForFalloff += pixelsNeeded;
}

unsigned __int8 *__cdecl R_LoadLightImage(unsigned __int8 *readPos, GfxLightImage *lightImage)
{
    unsigned int v3; // [esp+0h] [ebp-18h]
    unsigned __int8 *readPosa; // [esp+20h] [ebp+8h]

    lightImage->samplerState = *readPos;
    readPosa = readPos + 1;
    v3 = strlen((const char *)readPosa);
    if (v3)
        lightImage->image = Image_Register((char *)readPosa, 1u, 5);
    else
        lightImage->image = 0;
    return &readPosa[v3 + 1];
}

GfxLightDef *__cdecl R_LoadLightDef(const char *name)
{
    char v2; // [esp+3h] [ebp-31h]
    char *v3; // [esp+8h] [ebp-2Ch]
    const char *v4; // [esp+Ch] [ebp-28h]
    char *filename; // [esp+20h] [ebp-14h]
    GfxLightDef *def; // [esp+24h] [ebp-10h]
    unsigned __int8 *file; // [esp+28h] [ebp-Ch] BYREF
    int fileSize; // [esp+2Ch] [ebp-8h]
    const unsigned __int8 *readPos; // [esp+30h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\r_light_load_obj.cpp", 88, 0, "%s", "name");
    filename = va("lights/%s", name);
    fileSize = FS_ReadFile(filename, (void **)&file);
    if (fileSize < 0)
        return 0;
    if (fileSize)
    {
        def = (GfxLightDef *)Hunk_Alloc(0x10u, "R_RegisterLightDef", 20);
        def->name = (const char *)Hunk_Alloc(strlen(name) + 1, "R_RegisterLightDef", 20);
        if (!def)
            MyAssertHandler(".\\r_light_load_obj.cpp", 103, 0, "%s", "def");
        readPos = file;
        readPos = R_LoadLightImage(file, &def->attenuation);
        v4 = name;
        v3 = (char *)def->name;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        } while (v2);
        I_strlwr((char *)def->name);
        FS_FreeFile((char *)file);
        R_AllocateFalloffSpaceInLightmaps(def);
        return def;
    }
    else
    {
        FS_FreeFile((char *)file);
        return 0;
    }
}
