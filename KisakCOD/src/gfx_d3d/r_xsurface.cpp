#include "r_xsurface.h"
#include <universal/assertive.h>



int __cdecl XSurfaceGetNumVerts(const XSurface *surface)
{
    if (!surface)
        MyAssertHandler(".\\r_xsurface.cpp", 21, 0, "%s", "surface");
    return surface->vertCount;
}

int __cdecl XSurfaceGetNumTris(const XSurface *surface)
{
    if (!surface)
        MyAssertHandler(".\\r_xsurface.cpp", 28, 0, "%s", "surface");
    return surface->triCount;
}

