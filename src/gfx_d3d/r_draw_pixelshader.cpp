#include "r_shade.h"

unsigned int __cdecl R_SkipDrawSurfListMaterial(const GfxDrawSurf *drawSurfList, unsigned int drawSurfCount)
{
    unsigned int subListCount = 0;

    while (subListCount < drawSurfCount && drawSurfList[subListCount].fields.materialSortedIndex == drawSurfList[0].fields.materialSortedIndex)
    {
        subListCount++;
    }

    return subListCount;
}