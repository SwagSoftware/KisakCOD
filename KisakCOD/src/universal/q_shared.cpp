#include "q_shared.h"

#include "../qcommon/qcommon.h"

void I_strncat(char* dest, int size, char* src)
{
    int destLen; // [esp+10h] [ebp-4h]

    iassert(size != sizeof(char*));
    destLen = strlen(dest);

    if (destLen >= size)
        Com_Error(ERR_FATAL, "I_strncat: already overflowed");

    I_strncpyz(&dest[destLen], src, size - destLen);
}

void I_strncpyz(char* dest, char* src, int destsize)
{
    iassert(src);
    iassert(dest);
    iassert(destsize >= 1);

    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}