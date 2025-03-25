#include "r_debug.h"
#include <universal/com_memory.h>


void __cdecl R_DebugAlloc(void **memPtr, int size, const char *name)
{
    if (!memPtr)
        MyAssertHandler(".\\r_debug_alloc.cpp", 20, 0, "%s", "memPtr");
    if (*memPtr)
        MyAssertHandler(".\\r_debug_alloc.cpp", 21, 0, "%s", "!*memPtr");
    if (size <= 0)
        MyAssertHandler(".\\r_debug_alloc.cpp", 22, 0, "%s", "size > 0");
    if ((size & 3) != 0)
        MyAssertHandler(".\\r_debug_alloc.cpp", 23, 0, "%s", "(size & 0x3) == 0");
    *memPtr = Z_TryVirtualAlloc(size, name, 0);
    if (!*memPtr)
        R_WarnOncePerFrame(R_WARN_DEBUG_ALLOC, name);
}

void __cdecl R_DebugFree(void **dataPtr)
{
    if (*dataPtr)
    {
        Z_VirtualFree(*dataPtr);
        *dataPtr = 0;
    }
}

