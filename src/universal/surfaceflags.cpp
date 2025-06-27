#include "surfaceflags.h"
#include "q_shared.h"


int __cdecl Com_SurfaceTypeFromName(const char *name)
{
    int i; // [esp+0h] [ebp-4h]

    if (!I_stricmp(name, "default"))
        return 0;
    for (i = 0; i < 28; ++i)
    {
        if (!I_stricmp(name, infoParms[i].name))
            return (infoParms[i].surfaceFlags & 0x1F00000) >> 20;
    }
    return -1;
}


const char *__cdecl Com_SurfaceTypeToName(int iTypeIndex)
{
    if (iTypeIndex <= 0 || iTypeIndex >= 29)
        return "default";
    if ((infoParms[iTypeIndex - 1].surfaceFlags & 0x1F00000) >> 20 != iTypeIndex)
        MyAssertHandler(
            ".\\universal\\surfaceflags.cpp",
            139,
            0,
            "%s",
            "SURF_TYPEINDEX( infoParms[iTypeIndex - 1].surfaceFlags ) == iTypeIndex");
    return infoParms[iTypeIndex - 1].name;
}