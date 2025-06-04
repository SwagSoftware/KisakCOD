#include "r_shade.h"

unsigned int __cdecl R_SkipDrawSurfListMaterial(const GfxDrawSurf *drawSurfList, unsigned int drawSurfCount)
{
    unsigned int subListCount; // [esp+14h] [ebp-Ch]

    for (subListCount = 0;
        subListCount < drawSurfCount
        && (drawSurfList[subListCount].packed & 0xE0000000) == (drawSurfList->packed & 0xE0000000)
        && HIDWORD(drawSurfList[subListCount].packed) == HIDWORD(drawSurfList->packed);
        ++subListCount)
    {
        ;
    }
    return subListCount;
}