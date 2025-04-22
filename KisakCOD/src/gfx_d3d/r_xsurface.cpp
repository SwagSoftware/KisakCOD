#include "r_xsurface.h"
#include <universal/assertive.h>
#include "r_model_skin.h"


void __cdecl XSurfaceGetVerts(const XSurface *surf, float *pVert, float *pTexCoord, float *pNormal)
{
    int render_count; // [esp+0h] [ebp-Ch]
    GfxPackedVertex *verts1; // [esp+4h] [ebp-8h]
    GfxPackedVertex *verts0; // [esp+8h] [ebp-4h]

    verts0 = surf->verts0;
    if (!verts0)
        MyAssertHandler(".\\r_xsurface.cpp", 79, 0, "%s", "verts0");
    verts1 = verts0;
    for (render_count = surf->vertCount; render_count; --render_count)
    {
        if (pNormal)
        {
            Vec3UnpackUnitVec(verts1->normal, pNormal);
            pNormal += 3;
        }
        if (pTexCoord)
        {
            Vec2UnpackTexCoords(verts1->texCoord, pTexCoord);
            pTexCoord += 2;
        }
        *pVert = verts0->xyz[0];
        pVert[1] = verts0->xyz[1];
        pVert[2] = verts0->xyz[2];
        ++verts0;
        ++verts1;
        pVert += 3;
    }
}

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

