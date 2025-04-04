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