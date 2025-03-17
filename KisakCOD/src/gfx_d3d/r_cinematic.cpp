#include "r_cinematic.h"

#include <universal/q_shared.h>
#include <universal/assertive.h>

int __cdecl CinematicHunk_Alloc(CinematicHunk* hunk, int size)
{
    const char* v2; // eax
    char* alloced; // [esp+0h] [ebp-4h]

    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 366, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 367, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 368, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 369, 0, "%s", "hunk->end");
    if (size < 0)
        MyAssertHandler(".\\r_cinematic.cpp", 370, 0, "%s", "size >= 0");
    alloced = (char*)hunk->atFront;
    hunk->atFront = &alloced[size];
    if (hunk->atFront <= hunk->atBack)
        return (int)alloced;
    if (!alwaysfails)
    {
        v2 = va("CinematicHunk_Alloc failed: 0x%08x 0x%08x 0x%08x\n", hunk->atFront, size, hunk->atBack);
        MyAssertHandler(".\\r_cinematic.cpp", 376, 0, v2);
    }
    return -1;
}
