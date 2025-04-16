#include "files.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>

char basename[64];
char *__cdecl FS_GetMapBaseName(char *mapname)
{
    unsigned int v2; // [esp+0h] [ebp-18h]
    int c; // [esp+10h] [ebp-8h]
    signed int len; // [esp+14h] [ebp-4h]

    if (!mapname)
        MyAssertHandler(".\\qcommon\\files.cpp", 1666, 0, "%s", "mapname");
    if (!I_strnicmp(mapname, "maps/mp/", 8))
        mapname += 8;
    v2 = strlen(mapname);
    len = v2;
    if (!I_stricmp(&mapname[v2 - 3], "bsp"))
        len = v2 - 7;
    memcpy((unsigned __int8 *)basename, (unsigned __int8 *)mapname, len);
    basename[len] = 0;
    for (c = 0; c < len; ++c)
    {
        if (basename[c] == 37)
            basename[c] = 95;
    }
    return basename;
}