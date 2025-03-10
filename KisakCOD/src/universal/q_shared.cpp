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

int I_stricmp(const char* s0, const char* s1)
{
    iassert(s0);
    iassert(s1);

    return I_strnicmp(s0, s1, 0x7FFFFFFF);
}

int I_strnicmp(const char* s0, const char* s1, int n)
{
    int c1; // [esp+0h] [ebp-8h]
    int c0; // [esp+4h] [ebp-4h]

    do
    {
        c0 = *(unsigned __int8*)s0;
        c1 = *(unsigned __int8*)s1;
        ++s0;
        ++s1;
        if (!n--)
            return 0;
        if (c0 != c1)
        {
            if (I_isupper(c0))
                c0 += 32;
            if (I_isupper(c1))
                c1 += 32;
            if (c0 != c1)
                return 2 * (c0 >= c1) - 1;
        }
    } while (c0);
    return 0;
}

bool I_islower(int c)
{
    return c >= 'a' && c <= 'z';
}
bool I_isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}
bool I_isalpha(int c)
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}
bool I_iscsym(int c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c < '0' || c > '9')
        return c == '_';
    return 1;
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char* QDECL va(const char* format, ...) {
    va_list		argptr;
    static char		string[2][32000];	// in case va is called by nested functions
    static int		index = 0;
    char* buf;

    buf = string[index & 1];
    index++;

    va_start(argptr, format);
    vsprintf(buf, format, argptr);
    va_end(argptr);

    return buf;
}
