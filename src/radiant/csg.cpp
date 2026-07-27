#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\csg.cpp
// Brush CSG: hollow, merge, and the AutoCaulk visibility test.
//   CSG_MakeHollow 0x47D3C0, Brush_MergeList 0x47D600, CSG_Merge 0x47DA40,
//   Brush_GetFaceFlags 0x47DC30, Brush_IsFixedOrPatch 0x47DD20,
//   Brush_FaceFlags_Check80 0x47DDB0, Brush_IsFiltered 0x47DDE0,
//   Brush_IsCulled 0x47DE10, CSG_FaceVisible 0x47DE60,
//   Brush_AutoCaulkFace 0x47E080, Brush_AutoCaulk 0x47E0F0.
//
// List model (CoD4Radiant, NOT GtkRadiant): selected_brushes / active_brushes /
// filtered_brushes are EMBEDDED 56-byte selbrush_t sentinel NODES, iterated
// `for (b = selected_brushes.next; b != &selected_brushes; b = b->next)`.
// selbrush_t.def (+0x14) is the 88-byte brush_t DEFINITION holding the geometry;
// many instances may share one def (def->refCount).  Geometry comes from
// def->faceCount (+0x40) / def->faces (+0x44), a FLAT array of stride 232.

#include "stdafx.h"
#include "winding.h"
#include <universal/assertive.h>   // iassert (USE_ASSERTS always on; same handler as Assert)

// ─── externs from qe3.cpp / engine_stubs.cpp ─────────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );

// ─── externs from brush.cpp ──────────────────────────────────────────────────
// Splits brush b with face f; front/back are new brushes (NULL if no piece).
extern void     Brush_SplitBrushByFace( brush_t *b, face_t *f,
                                         brush_t **front, brush_t **back );    // 0x471960
// Allocates a new face slot in dest's face array and copies src into it.
// Returns a pointer to the newly allocated face entry.
// IDA signature: char* __cdecl Face_Alloc(int brush, _DWORD *face)
extern face_t  *Face_Alloc( brush_t *dest, face_t *src );                    // 0x471500
// Frees a DEF (88-byte brush_t) without list/entity cleanup (raw free).
// IDA: __usercall(def@<edi>) — arg in edi; cdecl wrapper needed when brush.cpp is ported.
extern void     Brush_Free_R( brush_t *def );                                  // 0x475af0
// Allocates a selbrush_t INSTANCE for def, bumps def->refCount, links it into
// owner's entity brush list, and returns the new instance node. (Misnamed in IDB;
// really "instance-for-def" / Entity_LinkBrush.) IDA: __usercall@<eax>(a1@<eax>, a2).
extern selbrush_t *Brush_AddToList( brush_t *def, entity_s *owner );           // 0x475980
// Links an existing instance node into selected_brushes and notifies the UI.
// IDA: __usercall(b@<eax>) — arg in eax; cdecl wrapper needed when brush.cpp is ported.
extern void     Brush_AddToList2( selbrush_t *b );                             // 0x4765a0
// Removes an instance from its lists and frees it (full cleanup; --def->refCount).
extern void     Brush_Free( selbrush_t *b );                                   // 0x475ba0
// Unlinks an instance node from the display list (clears next/prev).
// IDA: __usercall(a1@<eax>) — arg in eax; cdecl wrapper needed when select.cpp is ported.
extern void     Brush_RemoveFromList( selbrush_t *b );                         // 0x476680
// Rebuilds windings for all faces of brush b.
// IDA: __usercall(def@<ecx>, bsnap) — first arg in ecx; cdecl wrapper needed.
extern void     Brush_BuildWindings( brush_t *b, int bFull );                  // 0x477ac0
// Updates brush colour based on entity class.
extern unsigned int Entity_ColorSth( brush_t *b );                             // 0x475110 (entity.cpp)
// Applies a face material to the face in brush; used by AutoCaulk.
extern void     sub_476740( int texdef, face_t *face, brush_t *brush );        // 0x476740

// ─── externs from materialdef.cpp ────────────────────────────────────────────
extern qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *def );         // 0x4314a0
// Sets *out to the material descriptor for name.
extern void     SetMaterial( const char *name, patchMesh_material *out );      // 0x4315c0

// CRT free thunk (IDA j__free_0) — matches the eclass.cpp pattern.
static inline void j__free_0( void *p ) { free( p ); }

// ─── globals ──────────────────────────────────────────────────────────────────
// Indexed grid size table.  d_gridsize indexes into this.
extern float    grid_sizes[];                                                   // 0x6dde5c
// Active layer name string (default "000_Global").
extern char     g_activeLayer_string[];                                         // 0x73bf78
// Bitmask of pending window update bits.  Set to -1 (W_ALL) to refresh all.
extern int      g_nUpdateBits;                                                  // 0x25d5a74

// g_windingAlloc (winding.cpp, IDB dword_24CE4FC) — live winding malloc/free count.

// ─────────────────────────────────────────────────────────────────────────────
//  Face by flat-array index; face_t stride is 232 (static_assert in qe3.h).
// ─────────────────────────────────────────────────────────────────────────────
static inline face_t *BrushFaceAt( brush_t *def, unsigned int idx )
{
    return (face_t *)( (char *)def->faces + idx * 232u );
}

// ─────────────────────────────────────────────────────────────────────────────
//  Same, from a raw face pointer.
// ─────────────────────────────────────────────────────────────────────────────
static inline face_t *FaceAt( face_t *base, unsigned int idx )
{
    return (face_t *)( (char *)base + idx * 232u );
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_GetFaceFlags (0x47DC30, def@<esi>).  Accumulates the current-edit-layer
// material flags over every face of the brush geometry.  Any output pointer may be
// NULL.  *anyInUse = OR of qtexture_s::in_use, *allInUse = AND (init -1),
// *allFilter = AND of color_or_surfacetype_filter (init -1).  The binary also
// accumulates the OR of the filter word but never returns it.
// Asserts csg.cpp:302 "def" and csg.cpp:303 "!def->patch".
// ─────────────────────────────────────────────────────────────────────────────
void Brush_GetFaceFlags( brush_t *def,
                         int *out_anyInUse,
                         int *out_allInUse,
                         int *out_allFilter )
{
    iassert( def );
    iassert( !def->patch );

    int anyInUse  = 0;
    int allInUse  = -1;
    int anyFilter = 0;   // IDA: v11 — not returned but accumulated for internal use
    int allFilter = -1;  // IDA: i  — returned in *out_allFilter

    int layer = g_qeglobals.current_edit_layer;

    for ( unsigned int fi = 0; fi < (unsigned int)def->faceCount; fi++ )
    {
        face_t *f = BrushFaceAt( def, fi );
        // IDA `a1[17] + fi*232 + 4*(9*layer+9)` == faces + fi*232 + 36*(layer+1)
        // == &face->mtldef[layer] (mtldef starts at face_t+0x24, MaterialDef is 36 B).
        MaterialDef *md = &f->mtldef[layer];

        qtexture_s *tex = MaterialDef_GetLayeredMaterial( md );
        // qtexture_s::in_use is at +0x20, color_or_surfacetype_filter at +0x1c.
        int inUse = tex ? tex->in_use : 0;
        anyInUse |= inUse;
        allInUse &= inUse;

        tex = MaterialDef_GetLayeredMaterial( md );
        int filt = tex ? tex->color_or_surfacetype_filter : 0;
        anyFilter |= filt;
        allFilter &= filt;
    }

    if ( out_anyInUse )  *out_anyInUse  = anyInUse;
    if ( out_allInUse )  *out_allInUse  = allInUse;
    if ( out_allFilter ) *out_allFilter = allFilter;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_IsFixedOrPatch (0x47DD20, a1@<esi>).  True when the brush must be skipped in
// CSG ops: def->owner->eclass->fixedsize or def->patch != NULL.
// ─────────────────────────────────────────────────────────────────────────────
bool Brush_IsFixedOrPatch( brush_t *def )
{
    iassert( def );
    iassert( def->owner );
    iassert( def->owner->eclass );
    return def->owner->eclass->fixedsize || ( def->patch != nullptr );
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_FaceFlags_Check80 (0x47DDB0).  True (not filtered) when no face flag carries
// bit 0x80; wraps Brush_GetFaceFlags and tests allFilter & 0x80.
// ─────────────────────────────────────────────────────────────────────────────
bool Brush_FaceFlags_Check80( brush_t *def )
{
    int allFilter = 0;
    Brush_GetFaceFlags( def, nullptr, nullptr, &allFilter );
    return ( (unsigned int)allFilter & 0x80u ) == 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_IsFiltered (0x47DDE0).  The layer-visible gate:
//   !(anyInUse & 0x20000000) && (allInUse & 1).
// ─────────────────────────────────────────────────────────────────────────────
bool Brush_IsFiltered( brush_t *def )
{
    int anyInUse = 0, allInUse = 0, allFilter = 0;
    Brush_GetFaceFlags( def, &anyInUse, &allInUse, &allFilter );
    if ( anyInUse & 0x20000000 )
        return false;
    return ( allInUse & 1 ) != 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_IsCulled (0x47DE10, a1@<eax>).  Brush_IsFixedOrPatch, or the negation of the
// Brush_IsFiltered gate.
// ─────────────────────────────────────────────────────────────────────────────
bool Brush_IsCulled( brush_t *def )
{
    if ( Brush_IsFixedOrPatch( def ) )
        return true;
    // The binary passes a real &v3 for the 4th (allFilter) arg, not NULL (dead output).
    int anyInUse = 0, allInUse = 0, allFilter = 0;
    Brush_GetFaceFlags( def, &anyInUse, &allInUse, &allFilter );
    if ( anyInUse & 0x20000000 )
        return true;
    return ( allInUse & 1 ) == 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// CSG_FaceVisible (0x47DE60).  Recursive winding-clip visibility test used by
// AutoCaulk.  Walks the selected-brush display list from `brushNode`; for each
// eligible brush it clips `w` face by face, keeping the BACK piece (the part inside
// that convex brush) and recursing on the FRONT piece into the rest of the list.
//   returns 1 = the winding survived  -> the face is VISIBLE
//   returns 0 = the winding was fully accounted for -> the face is HIDDEN (caulk it)
// `w` is CONSUMED (freed) on every path.  Call with faceIdx = 0 and
// brushNode = selected_brushes.next; `srcDef` is skipped (it owns the face).
// The per-brush face-count LIMIT comes from the selbrush_t NODE (+0x18), while the
// face DATA comes from node->def; assert csg.cpp:407.
// plane.dist is read from a DOUBLE slot at plane+0x10, but Plane_FromPoints computes
// it as a float and merely widens it for storage, so the port's float dist is exactly
// the binary's double.  (plane_t = {vec3 normal; int type; float dist; float unk} = 24 B,
// so face.plane.dist is at face+0xC0+0x10 = 0xD0 — where the binary reads it.)
// ─────────────────────────────────────────────────────────────────────────────
bool CSG_FaceVisible( winding_t *w, float *faceNormal, brush_t *srcDef,
                      selbrush_t *brushNode, unsigned int faceIdx )
{
    // The binary zeroes a5 in the prologue regardless of what the caller passed.
    faceIdx = 0;

    if ( !w )
        return false;

    while ( true )
    {
        // Next eligible node: not srcDef, not fixed/patch, and passing the filter
        // gate !(anyInUse & 0x20000000) && (allInUse & 1).
        while ( brushNode != &selected_brushes )
        {
            brush_t *def = brushNode->def;

            if ( def != srcDef && !Brush_IsFixedOrPatch( def ) )
            {
                // The binary passes a real &v12 for the 4th arg, not NULL (dead output).
                int anyInUse = 0, allInUse = 0, allFilter = 0;
                Brush_GetFaceFlags( def, &anyInUse, &allInUse, &allFilter );
                if ( ( anyInUse & 0x20000000 ) == 0 && ( allInUse & 1 ) != 0 )
                    break;  // found eligible
            }
            brushNode = brushNode->next;
        }

        if ( brushNode == &selected_brushes )
        {
            // Sentinel: nothing left to clip against, the winding survived → VISIBLE.
            --g_windingAlloc;
            free( w );
            return true;
        }

        brush_t *def = brushNode->def;

        // The bound is the NODE's faceCount (+0x18), not def->faceCount.
        unsigned int nodeFaceCount = (unsigned int)brushNode->faceCount;
        // KEEP_VERBOSE: the port's locals don't byte-match the binary's #expr, so the
        // verbose Assert form preserves the exact string.  faceIdx is unsigned, so the
        // binary's `jb` reduces to the upper-bound test.
        if ( faceIdx >= nodeFaceCount )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\csg.cpp",
                    407, 0, "%s", "faceIndex >= 0 && faceIndex < b->faceCount" );

        face_t *clipFace = BrushFaceAt( def, faceIdx );
        plane_t *clipPlane = &clipFace->plane;

        winding_t *front = nullptr, *back = nullptr;
        int sideCode = Winding_Clip( w, clipPlane->normal, (double)clipPlane->dist,
                                     0.1f, &front, &back );

        --g_windingAlloc;
        free( w );
        w = back;  // back = piece on the clip plane's back side

        selbrush_t *nextNode = brushNode->next;
        if ( CSG_FaceVisible( front, faceNormal, srcDef, nextNode, 0 ) )
        {
            // The front piece is visible through the rest of the list → so is the face.
            if ( w )
            {
                --g_windingAlloc;
                free( w );
            }
            return true;
        }

        // 0x47df85 `cmp var_18, 2; jnz`: the dot test runs ONLY when Winding_Clip
        // returned SIDE_ON (2 = winding fully coplanar with the clip plane).  For
        // 0/1/3 the binary advances to the next FACE of the same brush; only a
        // coplanar SAME-FACING clip skips to the next brush.
        if ( sideCode == 2 )
        {
            float dot = clipPlane->normal[0] * faceNormal[0]
                      + clipPlane->normal[1] * faceNormal[1]
                      + clipPlane->normal[2] * faceNormal[2];
            if ( dot > 0.0f )
            {
                // Coplanar same-facing → next brush, face index reset.
                brushNode = brushNode->next;
                faceIdx = 0;
                if ( !w )
                    return false;
                continue;
            }
        }

        ++faceIdx;

        if ( !w )
            return false;

        if ( faceIdx >= nodeFaceCount )
        {
            // Every face of this brush clipped it and it is still inside → HIDDEN.
            --g_windingAlloc;
            free( w );
            return false;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CSG_MakeHollow (0x47D3C0).  For each selected brush, inset every face by one grid
// unit along its normal, split the brush on the inset plane, DISCARD the back piece
// (the new inner-shell fragment) and keep the front piece (the wall slab); then free
// the original.  gridScale = grid_sizes[g_qeglobals.d_gridsize].
// The iterator saves p_prev = &node->next->prev BEFORE any free; in a doubly-linked
// list with prev at offset 0 that address IS node->next.
// ─────────────────────────────────────────────────────────────────────────────
void CSG_MakeHollow()
{
    selbrush_t *node = selected_brushes.next;

    while ( node != &selected_brushes )
    {
        brush_t  *def   = node->def;     // selbrush_t.def → 88-byte geometry
        entity_s *owner = node->owner;   // selbrush_t.owner

        // Saved before any free — see the header note.
        selbrush_t *nextNode = node->next;

        // KEEP_VERBOSE: the port's pre-extracted locals would not byte-match the
        // binary's #expr string, so the verbose Assert form is used.
        if ( owner->def != def->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\csg.cpp",
                    34, 0, "%s", "b->owner->def == b->def->owner" );

        // Skip fixed-size entities, patches and xx5-flagged brushes.
        patch_t *nodePatch = node->patch;   // selbrush_t.patch (instance)
        int      nodeXx5   = node->xx5;     // selbrush_t.xx5

        if ( !def->owner->eclass->fixedsize && !nodePatch && !nodeXx5 )
        {
            float gridScale = grid_sizes[g_qeglobals.d_gridsize];
            unsigned int faceCount = (unsigned int)def->faceCount;

            for ( unsigned int fi = 0; fi < faceCount; fi++ )
            {
                face_t *srcFace = BrushFaceAt( def, fi );

                face_t face;
                memcpy( &face, srcFace, sizeof(face_t) );

                // move = plane.normal * gridScale (planepts0[48..50] == plane.normal).
                float moveX = srcFace->plane.normal[0] * gridScale;
                float moveY = srcFace->plane.normal[1] * gridScale;
                float moveZ = srcFace->plane.normal[2] * gridScale;

                // Subtract it from all 3 planepts of the COPIED face.
                for ( int pi = 0; pi < 3; pi++ )
                {
                    face.planepts[pi][0] -= moveX;
                    face.planepts[pi][1] -= moveY;
                    face.planepts[pi][2] -= moveZ;
                }

                brush_t *front = nullptr, *back = nullptr;
                Brush_SplitBrushByFace( def, &face, &front, &back );

                if ( back )
                {
                    // Inlined Entity_UnlinkBrush (assert entity.cpp:1208).
                    if ( !back->onext || !back->oprev )
                    {
                        Com_Error( ERR_FATAL, "Entity_UnlinkBrush: Not currently linked" );
                    }
                    if ( back->refCount <= 0 )
                    {
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp",
                                1208, 0, "%s\n\t(b->refCount) = %i",
                                "(b->refCount > 0)", back->refCount );
                    }
                    brush_t *bN = back->onext;
                    brush_t *bP = back->oprev;
                    --back->refCount;
                    bN->oprev = bP;
                    back->oprev->onext = back->onext;
                    back->oprev = nullptr;
                    back->onext = nullptr;
                    back->owner = nullptr;
                    Brush_Free_R( back );
                }

                if ( front )
                {
                    // Brush_AddToList allocates the selbrush_t INSTANCE for the new def
                    // and links it to the entity; the DISPLAY-list links (next/prev,
                    // not the def's onext/oprev) must still be clear.
                    selbrush_t *added = Brush_AddToList( front, owner );
                    if ( added->next || added->prev )
                    {
                        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
                    }
                    Brush_AddToList2( added );
                }
            }  // for each face

            Brush_Free( node );
        }

        node = nextNode;
    }

    // The binary sets this at the END of the whole list loop, unconditionally.
    g_nUpdateBits = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_MergeList (0x47D600).  Try to merge a singly-linked list of selbrush_t nodes
// (threaded via .next, NULL-terminated — CSG_Merge builds it by reusing each instance's
// own link, NOT the circular display list) into one new convex brush; NULL if the
// result would be non-convex or carry duplicate planes.  The input list is not freed.
//   Phase 1 — for every pair of OUTER faces (a face is INNER when another brush in the
//     list has a flipped-equal plane), reject if Winding_PlanesConcave.
//   Phase 2 — Face_Alloc every outer face whose plane is not already in the new brush.
//   Then link the new def to the entity and build its windings.
// Plane_Equal(p2, p1, 1) negates its FIRST arg, so it means "p1 flipped == p2"
// (touching); Plane_Equal(p1, p2, 0) is the same-side test.
// The binary reads the phase-2 loop bound from the selbrush_t NODE's faceCount (+0x18)
// but the face DATA from node->def; the two mirror each other in the editor, so this
// port uses def->faceCount throughout.
// ─────────────────────────────────────────────────────────────────────────────
selbrush_t *Brush_MergeList( selbrush_t *brushList )
{
    if ( !brushList )
        return nullptr;


    // ── Phase 1: convexity check ──────────────────────────────────────────────
    for ( selbrush_t *node1 = brushList; node1; node1 = node1->next )
    {
        brush_t *def1 = node1->def;
        unsigned int fc1 = (unsigned int)def1->faceCount;

        for ( unsigned int fi1 = 0; fi1 < fc1; fi1++ )
        {
            face_t *face1   = BrushFaceAt( def1, fi1 );
            plane_t *plane1 = &face1->plane;

            // Does face1 touch another brush (flipped-equal plane somewhere else)?
            selbrush_t *node2;
            for ( node2 = brushList; node2; node2 = node2->next )
            {
                if ( node2 == node1 )
                    continue;
                brush_t *def2 = node2->def;
                unsigned int fc2 = (unsigned int)def2->faceCount;
                unsigned int fi2;
                for ( fi2 = 0; fi2 < fc2; fi2++ )
                {
                    face_t *face2 = BrushFaceAt( def2, fi2 );
                    if ( Plane_Equal( &face2->plane, plane1, 1 ) )
                        break;
                }
                if ( fi2 < fc2 )
                    break;  // face1 touches node2 → it's an interior face
            }
            if ( node2 )
                continue;  // interior face, skip convexity check

            // face1 is outer — compare it against the outer faces of later brushes.
            for ( node2 = node1->next; node2; node2 = node2->next )
            {
                brush_t *def2 = node2->def;
                unsigned int fc2 = (unsigned int)def2->faceCount;

                for ( unsigned int fi2 = 0; fi2 < fc2; fi2++ )
                {
                    face_t *face2   = BrushFaceAt( def2, fi2 );
                    plane_t *plane2 = &face2->plane;

                    selbrush_t *node3;
                    for ( node3 = brushList; node3; node3 = node3->next )
                    {
                        if ( node3 == node2 )
                            continue;
                        brush_t *def3 = node3->def;
                        unsigned int fc3 = (unsigned int)def3->faceCount;
                        unsigned int fi3;
                        for ( fi3 = 0; fi3 < fc3; fi3++ )
                        {
                            face_t *face3 = BrushFaceAt( def3, fi3 );
                            if ( Plane_Equal( &face3->plane, plane2, 1 ) )
                                break;
                        }
                        if ( fi3 < fc3 )
                            break;  // face2 touches node3
                    }
                    if ( node3 )
                        continue;  // interior face, skip

                    // Both outer — concavity test.  0x47d76c passes face1 in params
                    // (1,4,5) and face2 in (2,3,6); +0x20 from a plane pointer is that
                    // face's winding (plane@face+0xC0, +0x20 -> face+0xE0 == face.w).
                    // Winding_PlanesConcave is symmetric, but match the binary's order.
                    if ( Winding_PlanesConcave( face1->w, face2->w,
                                                plane2->normal, plane1->normal,
                                                (float)plane1->dist, (float)plane2->dist ) )
                    {
                        return nullptr;
                    }
                }
            }
        }
    }

    // Phase 2: allocate the new brush DEF and collect the outer faces.
    brush_t *newBrush = (brush_t *)operator new( 0x58u );
    memset( newBrush, 0, 0x58 );  // sizeof(brush_t) = 0x58 = 88 (the DEF, not the instance)

    for ( selbrush_t *node1 = brushList; node1; node1 = node1->next )
    {
        brush_t *def1    = node1->def;
        unsigned int fc1 = (unsigned int)def1->faceCount;

        for ( unsigned int fi1 = 0; fi1 < fc1; fi1++ )
        {
            face_t *face1   = BrushFaceAt( def1, fi1 );
            plane_t *plane1 = &face1->plane;

            selbrush_t *node2;
            for ( node2 = brushList; node2; node2 = node2->next )
            {
                if ( node2 == node1 )
                    continue;
                brush_t *def2 = node2->def;
                unsigned int fc2 = (unsigned int)def2->faceCount;
                unsigned int fi2;
                for ( fi2 = 0; fi2 < fc2; fi2++ )
                {
                    face_t *face2 = BrushFaceAt( def2, fi2 );
                    if ( Plane_Equal( &face2->plane, plane1, 1 ) )
                        break;
                }
                if ( fi2 < fc2 )
                    break;
            }
            if ( node2 )
                continue;  // interior face

            // Already in newBrush? (the binary tests both Plane_Equal polarities)
            unsigned int existCount = (unsigned int)newBrush->faceCount;
            unsigned int ei;
            for ( ei = 0; ei < existCount; ei++ )
            {
                face_t *ef = BrushFaceAt( newBrush, ei );
                if ( Plane_Equal( &ef->plane, plane1, 0 ) )
                    break;
                if ( Plane_Equal( &ef->plane, plane1, 1 ) )
                    break;
            }
            if ( ei < existCount )
                continue;  // duplicate

            Face_Alloc( newBrush, face1 );
        }
    }

    // Link the new brush to the entity and build its windings.
    // KEEP_VERBOSE: the binary's string is brush.cpp:2354 "b" (cross-file + name mismatch).
    if ( !newBrush )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                2354, 0, "%s", "b" );

    if ( newBrush->parent_layer_string )
        j__free_0( newBrush->parent_layer_string );
    unsigned int layerLen = (unsigned int)strlen( g_activeLayer_string );
    char *layerCopy = (char *)operator new( layerLen + 1 );
    memcpy( layerCopy, g_activeLayer_string, layerLen + 1 );
    newBrush->parent_layer_string = layerCopy;

    // Inlined Entity_LinkBrush (asserts entity.cpp:1190 / 1208).  The new DEF goes into
    // the ENTITY DEF's brush list, headed by entity_s_def.def (+0x8) and threaded via
    // brush_t onext (+0x4) / oprev (+0x0).  A brush DEF's .owner holds the
    // entity_s_def*, not the entity_s* — that is the "b->owner == owner->def" invariant.
    entity_s *listOwner = brushList->owner;                  // selbrush_t.owner (entity_s*)
    void     *entityDef = listOwner->def;   // entity_s_def*

    if ( newBrush->oprev || newBrush->onext )
        Com_Error( ERR_FATAL, "Entity_LinkBrush: Allready linked" );

    int rc = newBrush->refCount;
    newBrush->owner = (entity_s *)entityDef;
    if ( rc < 0 )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp",
                1190, 0, "%s\n\t(b->refCount) = %i",
                "(b->refCount >= 0)", rc );
    }
    ++newBrush->refCount;

    // Insert newBrush at the head of the entity DEF's brush list (entity_s_def.def @ +0x8).
    brush_t **entityBrushListHead = (brush_t **)&( (entity_s_def *)entityDef )->def;
    newBrush->onext = (brush_t *)entityBrushListHead;  // onext = &entity_def.def (sentinel)
    newBrush->oprev = *entityBrushListHead;            // oprev = current head def
    ( *entityBrushListHead )->onext = newBrush;        // old head's onext → newBrush
    *entityBrushListHead = newBrush;                   // entity def-list head → newBrush

    Entity_ColorSth( newBrush );

    // KEEP_VERBOSE: the binary's csg.cpp:190 string doesn't byte-match the port's locals.
    if ( newBrush->owner != (entity_s *)entityDef )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\csg.cpp",
                190, 0, "%s", "newBrushDef->owner == brushList->owner->def" );

    selbrush_t *result = Brush_AddToList( newBrush, listOwner );
    Brush_BuildWindings( newBrush, 0 );
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// CSG_Merge (0x47DA40).  Merge every selected brush into one: validate (>=2 selected,
// none fixed-size or patch, all owned by the same entity), unlink them all from
// selected_brushes into a NULL-terminated merge list built from their own .next links,
// call Brush_MergeList, then either free the originals and select the result, or
// re-add the originals on failure.
// ─────────────────────────────────────────────────────────────────────────────
int CSG_Merge()
{
    Sys_Printf( "Merging...\n" );

    selbrush_t *sel = selected_brushes.next;

    if ( sel == &selected_brushes )
        return Sys_Printf( "No brushes selected.\n" );

    if ( sel->next == &selected_brushes )
        return Sys_Printf( "At least two brushes have to be selected.\n" );

    // Ownership invariant: node.owner->def == node.def->owner (both entity_s_def*).
    if ( (void *)sel->owner->def != (void *)sel->def->owner )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\csg.cpp",
                222, 0, "%s",
                "selected_brushes.next->owner->def == selected_brushes.next->def->owner" );
        sel = selected_brushes.next;
    }

    void *ownerDef0 = sel->owner->def;

    for ( selbrush_t *b = sel; b != &selected_brushes; b = b->next )
    {
        iassert( b->owner->def == b->def->owner );   // casts dropped: entity_s.def is void*, so == entity_s* compiles; #expr now byte-matches IDB "b->owner->def == b->def->owner"

        entity_s_def *bOwnerDef = (entity_s_def *)b->owner->def;
        eclass_t *ec = bOwnerDef->eclass;
        if ( *( int * )&ec->fixedsize )
            return Sys_Printf( "Cannot add fixed size entities.\n" );

        if ( b->patch )   // selbrush_t.patch @0x20 — binary 0x47db07 reads [esi+20h] (the INSTANCE patch), NOT def->patch@0x50
            return Sys_Printf( "Cannot add patches.\n" );

        if ( bOwnerDef != ownerDef0 )
            return Sys_Printf( "Cannot add brushes from different entities.\n" );
    }

    // Remove each selected node from the display list and push it onto a NULL-terminated
    // merge list, reusing the instance's own next/prev links (captured before removal).
    selbrush_t *mergeList = nullptr;
    for ( selbrush_t *b = sel; b != &selected_brushes; )
    {
        selbrush_t *nextB = b->next;   // capture before RemoveFromList clears the links
        Brush_RemoveFromList( b );
        b->next = mergeList;           // IDA: v0->next = v5; v0->prev = 0
        b->prev = nullptr;
        mergeList = b;
        b = nextB;
    }

    selbrush_t *merged = Brush_MergeList( mergeList );

    if ( merged )
    {
        for ( selbrush_t *cur = mergeList; cur; )
        {
            selbrush_t *next = cur->next;
            cur->next = nullptr;
            cur->prev = nullptr;
            Brush_Free( cur );
            cur = next;
        }
        if ( merged->next || merged->prev )   // IDA: v7->next || v7->prev (not onext/oprev)
            Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
        Brush_AddToList2( merged );
        int r = Sys_Printf( "done.\n" );
        g_nUpdateBits = -1;
        return r;
    }
    else
    {
        for ( selbrush_t *cur = mergeList; cur; )
        {
            selbrush_t *next = cur->next;
            cur->next = nullptr;
            cur->prev = nullptr;
            Brush_AddToList2( cur );
            cur = next;
        }
        return Sys_Printf( "Cannot add a set of brushes with a concave hull.\n" );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_AutoCaulkFace (0x47E080).  Caulk face `face` of `brush` if CSG_FaceVisible
// says it is hidden by the surrounding brushes.  Returns true on the two caulking
// paths (no winding, or hidden) and false for a visible face — Brush_AutoCaulk counts
// the true returns, so an unconditional `return true` would inflate the count.
// The binary reaches the caulk arm through `&face->w->numpoints`, which is NULL when
// face->w is NULL, i.e. a face with no winding is treated as hidden.
// ─────────────────────────────────────────────────────────────────────────────
bool Brush_AutoCaulkFace( brush_t *brush, face_t *face )
{
    winding_t *w = face->w;
    if ( !w )
    {
        patchMesh_material mat{};
        SetMaterial( "caulk", &mat );
        sub_476740( (int)&mat, face, brush );   // pass &mat — sub_476740 derefs a1[0]/a1[1] = {lyrMtl,radMtl}; *(int*)&mat passed mat.lyrMtl (garbage/NULL-deref)
        return true;
    }

    winding_t *wClone = Winding_Clone( w );

    // CSG_FaceVisible returns non-zero when the face is VISIBLE (0x47e0ba tests == 0).
    selbrush_t *firstSelected = selected_brushes.next;
    bool hidden = !CSG_FaceVisible( wClone, face->plane.normal, brush, firstSelected, 0 );

    if ( hidden )
    {
        patchMesh_material mat{};
        SetMaterial( "caulk", &mat );
        sub_476740( (int)&mat, face, brush );
        return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_AutoCaulk (0x47E0F0).  For every non-fixed/non-patch selected brush whose
// face flags lack 0x80, caulk each face whose current-layer material isn't already
// "caulk" and is hidden by its neighbours; report the count.  The per-face name
// compare reads radMtl->name (qtexture_s+4) per 0x47e1d8/db.  UI command only
// (ON_COMMAND 33220 / OnSelectionAutoCaulk), off the map-load path.
// ─────────────────────────────────────────────────────────────────────────────
extern int  dword_181F51C;                          // materialdef.cpp realize-state
extern void MaterialDef_02( MaterialDef *def, int ( *cb )( qtexture_s * ) );   // 0x431520
extern int  MaterialDef_08( qtexture_s *mtl );       // 0x431990 (passed as the cb)

void Brush_AutoCaulk()
{
    int count = 0;
    selbrush_t *inst = selected_brushes.next;
    if ( inst != &selected_brushes )
    {
        for ( ;; )
        {
            brush_t *def = inst->def;
            if ( !Brush_IsFixedOrPatch( def ) )
            {
                int flags = 0;
                Brush_GetFaceFlags( def, nullptr, nullptr, &flags );
                if ( ( flags & 0x80 ) == 0 )
                {
                    for ( unsigned int fi = 0; fi < (unsigned int)def->faceCount; ++fi )
                    {
                        face_t      *fp = &def->faces[fi];
                        MaterialDef *md = &fp->mtldef[g_qeglobals.current_edit_layer];
                        dword_181F51C = 128;
                        MaterialDef_02( md, MaterialDef_08 );
                        if ( !dword_181F51C )
                        {
                            if ( !md || (int)( md->lyrMtl != nullptr ) + (int)( md->radMtl != nullptr ) != 1 )
                                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                                        85, 0, "%s", "MtlDef_IsValid( mtlDef )" );
                            const char *name = (const char *)md->lyrMtl;   // IDA: mov eax,[esi]  (lyrMtl* used as char*)
                            if ( !md->lyrMtl )
                                name = md->radMtl->name;          // IDA 0x47e1d8/db: mov ecx,[esi+4]; mov eax,[ecx+4] = radMtl->name (qtexture_s+4)
                            if ( _stricmp( name, "caulk" ) )
                            {
                                if ( Brush_AutoCaulkFace( def, fp ) )
                                    ++count;
                            }
                        }
                    }
                }
            }
            if ( inst->next == &selected_brushes )
                break;
            inst = inst->next;
        }
    }

    if ( count == 1 )
        Sys_Printf( "1 face caulked\n" );
    else if ( count )
        Sys_Printf( "%i faces caulked\n", count );
    else
        Sys_Printf( "no faces caulked\n" );
    g_nUpdateBits |= 1u;
}
