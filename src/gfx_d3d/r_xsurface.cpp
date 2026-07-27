#include <universal/q_shared.h>
#include "r_xsurface.h"
#include <universal/assertive.h>
#include "r_model_skin.h"

#include <xanim/xanim.h>

void __cdecl XSurfaceGetVerts(const XSurface *surf, float *pVert, float *pTexCoord, float *pNormal)
{
    int render_count; // [esp+0h] [ebp-Ch]
    GfxPackedVertex *verts1; // [esp+4h] [ebp-8h]
    GfxPackedVertex *verts0; // [esp+8h] [ebp-4h]

    verts0 = surf->verts0;
    iassert( verts0 );
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
    iassert( surface );
    return surface->vertCount;
}

int __cdecl XSurfaceGetNumTris(const XSurface *surface)
{
    iassert( surface );
    return surface->triCount;
}

#ifdef KISAK_RADIANT
#include <xanim/xmodel.h>   // XModel, XModelGetSurfaces, XModelBad
#include <string.h>         // memcpy

// ─────────────────────────────────────────────────────────────────────────────
// R_CheckTris (IDA 0x51de30, r_xsurface.cpp:37) — copy a surface's triangle indices
// into dstIndices, adding baseVert to every index.  Editor-only (CoD3's r_xsurface.cpp
// didn't carry it; the editor uses it to extract XModel geometry for vert-snap + ray-pick).
// baseVert is broadcast into both 16-bit lanes of a dword so two indices are offset per add;
// the caller guarantees baseVert+index <= USHRT_MAX (vertLimit <= 0x10000) so no lane carries.
// ─────────────────────────────────────────────────────────────────────────────
uint16_t *__cdecl R_CheckTris( const XSurface *surface, void *dstIndices, uint16_t baseVert )
{
    iassert( (reinterpret_cast<size_t>( surface->triIndices ) & 3) == 0 );
    iassert( (reinterpret_cast<size_t>( dstIndices ) & 3) == 0 );
    iassert( (surface->triCount & 1) == 0 );

    uint16_t *src = surface->triIndices;
    if ( !baseVert )
        return (uint16_t *)memcpy( dstIndices, src, 6 * surface->triCount );

    uint32_t  add = baseVert | ( (uint32_t)baseVert << 16 );
    uint32_t *dst = (uint32_t *)dstIndices;
    uint32_t *s   = (uint32_t *)src;
    for ( int pair = surface->triCount >> 1; pair; --pair )   // 2 tris (6 indices = 3 dwords) per step
    {
        dst[0] = add + s[0];
        dst[1] = add + s[1];
        dst[2] = add + s[2];
        dst += 3;
        s   += 3;
    }
    return (uint16_t *)dst;
}

// ─────────────────────────────────────────────────────────────────────────────
// Editor_ExtractXModelGeo (IDA sub_4FEBB0 0x4febb0, r_ed_scene.cpp:748) — extract an
// XModel's LOD-0 surface geometry (vertex POSITIONS into verts[3*N], triangle INDICES into
// indices[]) into plain caller buffers.  Returns the total index count written (0 on a bad
// model or on buffer overflow).  Pure CPU extraction — the GPU render-into-VB path
// (sub_456850) is separate + still parked.  Consumed by the editor's misc_model vert-snap
// (VertSnap_CollectModel) and model ray-pick (select.cpp).
// ─────────────────────────────────────────────────────────────────────────────
int __cdecl Editor_ExtractXModelGeo( XModel *model, float *verts, int vertLimit,
                                     uint16_t *indices, int indexLimit )
{
    iassert( model );
    iassert( verts );
    iassert( indices );
    iassert( vertLimit > 0 );
    iassert( vertLimit <= 0x10000 );   // USHRT_MAX + 1
    iassert( indexLimit > 0 );

    if ( XModelBad( model ) )
        return 0;

    XSurface *surfaces;
    int surfaceCount = XModelGetSurfaces( model, &surfaces, 0 );   // LODForXmodel(model, &surfs, 0)
    int vertCount = 0;
    int indexCount = 0;

    for ( int i = 0; i < surfaceCount; ++i )
    {
        XSurface *surf = &surfaces[i];
        int nv = XSurfaceGetNumVerts( surf );
        if ( vertCount + nv > vertLimit )
            return 0;
        int ni = 3 * XSurfaceGetNumTris( surf );
        if ( indexCount + ni > indexLimit )
            return 0;
        XSurfaceGetVerts( surf, verts + 3 * vertCount, 0, 0 );        // positions only (sub_51DF10)
        R_CheckTris( surf, indices + indexCount, (uint16_t)vertCount );
        vertCount += nv;
        indexCount += ni;
    }
    return indexCount;
}
#endif // KISAK_RADIANT

