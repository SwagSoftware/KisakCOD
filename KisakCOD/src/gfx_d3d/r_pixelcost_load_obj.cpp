#include "r_pixelcost_load_obj.h"
#include <qcommon/qcommon.h>

struct GfxPixelCostColorCode // sizeof=0x8
{                                       // ...
    unsigned __int8 count;              // ...
    unsigned __int8 r;                  // ...
    unsigned __int8 g;                  // ...
    unsigned __int8 b;                  // ...
    const char *colorName;              // ...
};

const GfxPixelCostColorCode s_pixelCostColorCodeTable[6] =
{
  { 0u, 0u, 0u, 0u, "black" },
  { 40u, 0u, 255u, 0u, "green" },
  { 56u, 255u, 255u, 0u, "yellow" },
  { 72u, 255u, 128u, 0u, "orange" },
  { 128u, 255u, 0u, 0u, "red" },
  { 255u, 255u, 255u, 255u, "white" }
}; // idb

void __cdecl R_PixelCost_PrintColorCodeKey()
{
    float percentage; // [esp+10h] [ebp-8h]
    unsigned int codeIndex; // [esp+14h] [ebp-4h]

    Com_Printf(8, "\n---- PIXEL COST KEY ----\n");
    for (codeIndex = 0; codeIndex < 6; ++codeIndex)
    {
        percentage = (double)s_pixelCostColorCodeTable[codeIndex].count * 1.5625;
        Com_Printf(8, "  %3.0f%%: %s\n", percentage, s_pixelCostColorCodeTable[codeIndex].colorName);
    }
    Com_Printf(8, "----------------------\n\n");
}

void __cdecl RB_PixelCost_BuildColorCodeMap(unsigned __int8 (*pixels)[4], int pixelCount)
{
    float v2; // [esp+1Ch] [ebp-5Ch]
    float v3; // [esp+30h] [ebp-48h]
    float v4; // [esp+44h] [ebp-34h]
    float lerp; // [esp+60h] [ebp-18h]
    int codeIndex; // [esp+70h] [ebp-8h]
    int pixelIndex; // [esp+74h] [ebp-4h]

    if (pixelCount != 256)
        MyAssertHandler(".\\r_pixelcost_load_obj.cpp", 41, 0, "%s\n\t(pixelCount) = %i", "(pixelCount == 256)", pixelCount);
    codeIndex = -1;
    for (pixelIndex = 0; pixelIndex < 256; ++pixelIndex)
    {
        if (s_pixelCostColorCodeTable[codeIndex + 1].count == pixelIndex)
        {
            ++codeIndex;
            *(_DWORD *)&(*pixels)[4 * pixelIndex] = s_pixelCostColorCodeTable[codeIndex].b
                | (s_pixelCostColorCodeTable[codeIndex].g << 8)
                | (s_pixelCostColorCodeTable[codeIndex].r << 16)
                | 0xFF000000;
        }
        else
        {
            lerp = (double)(pixelIndex - s_pixelCostColorCodeTable[codeIndex].count)
                / (double)(s_pixelCostColorCodeTable[codeIndex + 1].count - s_pixelCostColorCodeTable[codeIndex].count);
            v4 = (double)(s_pixelCostColorCodeTable[codeIndex + 1].b - s_pixelCostColorCodeTable[codeIndex].b) * lerp
                + (double)s_pixelCostColorCodeTable[codeIndex].b;
            (*pixels)[4 * pixelIndex] = (int)(v4);
            v3 = (double)(s_pixelCostColorCodeTable[codeIndex + 1].g - s_pixelCostColorCodeTable[codeIndex].g) * lerp
                + (double)s_pixelCostColorCodeTable[codeIndex].g;
            (*pixels)[4 * pixelIndex + 1] = (int)(v3);
            v2 = (double)(s_pixelCostColorCodeTable[codeIndex + 1].r - s_pixelCostColorCodeTable[codeIndex].r) * lerp
                + (double)s_pixelCostColorCodeTable[codeIndex].r;
            (*pixels)[4 * pixelIndex + 2] = (int)(v2);
            (*pixels)[4 * pixelIndex + 3] = -1;
        }
    }
}