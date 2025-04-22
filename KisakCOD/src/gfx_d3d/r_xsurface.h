#pragma once
#include "r_material.h"

void __cdecl XSurfaceGetVerts(const XSurface *surf, float *pVert, float *pTexCoord, float *pNormal);
int __cdecl XSurfaceGetNumVerts(const XSurface *surface);
int __cdecl XSurfaceGetNumTris(const XSurface *surface);
