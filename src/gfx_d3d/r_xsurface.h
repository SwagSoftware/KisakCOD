#pragma once
#include "r_material.h"

struct XSurface;

void __cdecl XSurfaceGetVerts(const XSurface *surf, float *pVert, float *pTexCoord, float *pNormal);
int __cdecl XSurfaceGetNumVerts(const XSurface *surface);
int __cdecl XSurfaceGetNumTris(const XSurface *surface);

#ifdef KISAK_RADIANT
struct XModel;
// Editor-only XModel CPU geometry extraction (vert-snap + model ray-pick). See r_xsurface.cpp.
uint16_t *__cdecl R_CheckTris(const XSurface *surface, void *dstIndices, uint16_t baseVert);
int __cdecl Editor_ExtractXModelGeo(XModel *model, float *verts, int vertLimit,
                                    uint16_t *indices, int indexLimit);
#endif
