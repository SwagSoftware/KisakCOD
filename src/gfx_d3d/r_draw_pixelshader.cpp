#include "r_shade.h"

uint32_t __cdecl R_SkipDrawSurfListMaterial(const GfxDrawSurf *drawSurfList, uint32_t drawSurfCount)
{
    uint32_t subListCount = 0;

    while (subListCount < drawSurfCount && drawSurfList[subListCount].fields.materialSortedIndex == drawSurfList[0].fields.materialSortedIndex)
    {
        subListCount++;
    }

    return subListCount;
}