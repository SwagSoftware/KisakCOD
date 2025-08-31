#include "r_debug.h"

#include <qcommon/qcommon.h>
#include <universal/com_memory.h>

void __cdecl R_DebugAlloc(void **memPtr, int size, const char *name)
{
    iassert( memPtr );
    iassert( !*memPtr );
    iassert( size > 0 );
    iassert( (size & 0x3) == 0 );
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

