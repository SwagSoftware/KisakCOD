#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\brush.cpp
// Ported from CoD4Radiant.exe (IW3xRadiant.i64).  GtkRadiant 1.6 radiant/brush.cpp was
// used only for algorithm names.

#include "stdafx.h"
#include "winding.h"
#include "prefs.h"                  // g_PrefsDlg (prefData_t* — texture/lightmap lock)
#include "linearmapping.h"          // LinearMapping_Setup/Apply (texture-lock reproject)
#include <gfx_d3d/r_rendercmds.h>   // R_AddCmd_Line3D, GfxPointVertex, GfxColor
#include <universal/assertive.h>    // iassert (USE_ASSERTS always on; same handler as Assert)

// faceVisuals_s / faceVis_s are now in qe3.h (moved so select.cpp can use them).
// The static_asserts remain in qe3.h.

// orientation_t is defined in fxprimitives.h (included via r_image.h → xanim.h → snd_public.h).
// Layout: float origin[3](0x00) + float axis[3][3](0x0C) = 48 bytes. Matches IDB.
static_assert(sizeof(orientation_t) == 48, "orientation_t (fxprimitives.h != IDB)");

// ─── helpers from qe3.cpp / engine_stubs.cpp ────────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );
extern char *AllocMaterialString( const char *src );

// ─── CRT import thunk (IDA: j__free_0 = __imp__free wrapper) ────────────────
// IDA 0x583a8d: jmp _free (thin forwarder).
static inline void j__free_0( void *p ) { free( p ); }

// ─── globals (defined in engine_stubs.cpp until home files ported) ───────────
extern float grid_sizes[];
extern char  g_activeLayer_string[];
extern int   g_nUpdateBits;
extern int   g_windingAlloc;  // winding.cpp dword_24CE4FC

// ─── world entity and UI notification stubs ─────────────────────────────────
extern entity_s *world_entity;
int UpdateSelection( int wParam, eclass_t *cls );   // win_ent.cpp (0x497180)
namespace SurfaceInspector { void UpdateSurfaceDialog(); }

// ─── MFC parent window (CMainFrame) ─────────────────────────────────────────
#include "mainfrm.h"
extern CMainFrame *g_pParentWnd;
extern int CCamWnd_RemoveLightPreview( selbrush_t *removed, CCamWnd *cam );   // camwnd.cpp (sub_4062D0 0x4062d0)

// ─── patch dialog (patchdialog.cpp, not yet ported) ─────────────────────────
// IDB: g_PatchDialog (0x436ba0 = CPatchDialog::GetPatchInfo).
// Stubs return a null HWND so the `if (m_hWnd)` check is always false at runtime.
extern CWnd *g_PatchDialog_GetHwnd();
extern void  g_PatchDialog_GetPatchInfo();

// ─── Entity_RebuildBounds (0x485390, entity.cpp) — was sub_485390 ───────────
extern void Entity_RebuildBounds( entity_s *e );

// ─── materialdef.cpp ────────────────────────────────────────────────────────
extern qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *def );
extern void        SetMaterial( const char *name, patchMesh_material *out );

// ─── Init_MaterialLayer (0x472c00) ──────────────────────────────────────────
// __usercall(a1@<eax>, a2): first arg in eax, second on stack
extern int Init_MaterialLayer( MaterialDef *channel, MaterialDef *src );

// ─── Byte4PackPixelColor (0x402ac0) ─────────────────────────────────────────
extern char Byte4PackPixelColor( float *from, GfxColor *out );
// colorWhite[4] is static const in q_shared.h (included via stdafx.h → qe3.h → qedefs.h)

// ─── brush geometry helpers ──────────────────────────────────────────────────
// The CM_* brush-winding pipeline + Brush_MakeFacePlanes / VectorMaxValues are now
// implemented in this file (see the "brush winding generation" block below).
extern unsigned int Brush_SnapPlanepts( brush_t *b );   // engine_stubs (bFull-only edit path)
extern void         SetupVertexSelection();             // engine_stubs (vertex/edge select mode)

// kisak engine math (IntersectPlanes / SnapPointToIntersectingPlanes / Vec3Cross).
#include <universal/com_math.h>
extern int Face_MakePlane( face_t *face );              // brush.cpp (Chunk C)

// ─── patch helpers (patchsys.cpp / patch.cpp) ───────────────────────────────
extern void        PMESH_32_Symbiot( int patch );
extern patch_t    *PMESH_55( patchMesh_t *def );
extern curvePatchDef_t *Patch_GenericMesh2( patchMesh_t *p, int layer, int colMap, int rowMap ); // pmesh.cpp 0x438b60
extern void        PMESH_33( patch_t *patch );
extern void        PMESH_22_Indices( patch_t *patch );
extern patchMesh_t *Patch_Duplicate( patchMesh_t *from );

// ─── entity link helper (0x485020) ──────────────────────────────────────────
extern void Entity_UnlinkBrush( brush_t *b );  // entity.cpp

// ─── vis helpers (Chunk B) ───────────────────────────────────────────────────
extern void Visuals_VisArray( faceVis_s *vis );
extern int  Visuals_InitFaceVis( faceVis_s *vis, face_t *faceDef,
                                  const orientation_t *orient );
// Vis_Free (0x4702b0) + PlanePts_Alloc (0x470250) — defined further below; the faithful
// Brush_MakeFaceVisuals (~:155) uses them before their definitions, so forward-declare.
void  Vis_Free( int count, faceVis_s *f, int brushPtr );
void *PlanePts_Alloc( int count );
// sub_482B50 = Entity_GetOrientationInverse (entity.cpp, 0x482B50)
extern void Entity_GetOrientationInverse( entity_s_def *entityDef, orientation_t *orient,
                                           orientation_t *outOrient );
// Entity_GetOrientation (entity.cpp, 0x482A70) — FORWARD orientation (used to place a
// placed prefab's instanced brushes in DrawBrush_PrefabContents).
extern void Entity_GetOrientation( entity_s_def *ent, orientation_t *orParent,
                                    orientation_t *orOut );

// prefab_s — local mirror of entity.cpp's prefab_s (0x54).  Only the instanced-brush
// list head/tail sentinels are accessed here (the prefab-render path).  Layout MUST
// match entity.cpp (static_assert below).
struct prefab_s
{
    entity_s    *prev_entity;           // 0x00
    entity_s    *next_entity;           // 0x04
    void        *unk;                   // 0x08
    selbrush_t  *active_brushlist;      // 0x0C   tail sentinel (prev side)
    selbrush_t  *active_brushlist_next; // 0x10   head sentinel (next side)
    char         _pad[0x54 - 0x14];    // 0x14 .. 0x53
};
static_assert(sizeof(prefab_s) == 0x54, "prefab_s (brush.cpp mirror != entity.cpp)");

// ═════════════════════════════════════════════════════════════════════════════
//  Brush_MakeFaceVisuals (0x477c50) + Brush_CheckBuildFaceVis (0x477d70)
//  — the per-instance faceVis builder.  ONE lifecycle, as in the binary:
//  Brush_CheckBuildFaceVis calls Brush_MakeFaceVisuals on a stale version, which
//  (re)allocates faceVis_s[faceCount] and per face runs Visuals_VisArray (0x46f590,
//  free the old visArray) then Visuals_InitFaceVis (0x46f7a0, build the per-layer
//  GfxWorldVertex run, upload it to the editor VB pool, record {Material*,vbHandle}).
//  This is the persistent surf-cache the filled/textured camera draw consumes.
//  KISAK: the GPU half is skipped when no D3D device is up (Radiant_FaceVisGpuReady
//  == false — the headless gates).  The faceVis then stays the identity array
//  (visCount=0, vertcount=winding count), which is all any SELECTION path reads.
// ─────────────────────────────────────────────────────────────────────────────

// Renderer-ready predicate for the GPU faceVis build.  True once the D3D9 device the
// editor surf-cache VB pool (Editor_VB_Upload, r_ed_vertbuf.cpp) writes through is up.
// Defined in camwnd.cpp (it owns the gfx_d3d includes); declared here.
extern bool Radiant_FaceVisGpuReady();

static void Brush_MakeFaceVisuals( selbrush_t *b, const float *orientArg )
{
    iassert( b );                                  // brush.cpp:3441
    iassert( b->version != b->def->version );      // brush.cpp:3442

    // Reallocate b->faces only when the face count changes (0x477cb4).  Vis_Free
    // (0x4702b0) releases each old face's visArray (Visuals_VisArray) before freeing;
    // PlanePts_Alloc (0x470250) = operator new(12*count) + memset 0 over faceVis_s[].
    if ( b->faceCount != b->def->faceCount )
    {
        if ( b->faces )
            Vis_Free( b->faceCount, b->faces, (int)(intptr_t)b );
        b->faceCount = b->def->faceCount;
        b->faces = (faceVis_s *)PlanePts_Alloc( b->faceCount > 0 ? b->faceCount : 1 );
    }

    // For a fixedsize entity (model/light) the binary inverts the entity orientation
    // so the per-face geometry is built in the entity's local frame (0x477ce7).
    // eclass lives on the entity DEF (via owner->def @ +0x08), NOT the 0x54-byte
    // instance — read it through owner->def (§11 instance-vs-def).  Ed_BrushFloorRay
    // passes a transient instance with owner==NULL (the floor-march wrapper); guard it.
    const orientation_t *orient = (const orientation_t *)orientArg;
    char localOrientBuf[48];                        // orientation_t is fwd-declared here
    orientation_t *localOrient = (orientation_t *)localOrientBuf;
    entity_s_def *eDef = b->owner ? (entity_s_def *)b->owner->def : nullptr;
    if ( eDef && eDef->eclass && eDef->eclass->fixedsize )
    {
        Entity_GetOrientationInverse( eDef, (orientation_t *)orientArg, localOrient );
        orient = localOrient;
    }

    // Per-face: free any stale visArray, then build the surf-cache visuals.  The GPU
    // build is the binary's default; skip it (identity array only) when no device.
    // Also skip it when owner==NULL: that is ONLY Ed_BrushFloorRay's TRANSIENT stack
    // instance (select.cpp), which forces a rebuild just to clip a ray against
    // def->faces[i].plane and never reads the GPU visArray.  Building it there would
    // alloc+free editor VB-pool slots on every floor ray, recycling slots out from under
    // in-flight geometry surfs.  The binary never GPU-builds an owner-less brush (its
    // floor-march passes real, already-built instances).
    const bool gpu = Radiant_FaceVisGpuReady() && ( b->owner != nullptr );
    for ( int i = 0; i < b->faceCount; ++i )
    {
        faceVis_s *fv = (faceVis_s *)((char *)b->faces + 12 * i);
        Visuals_VisArray( fv );                     // 0x46f590 (free old visArray)
        if ( gpu )
            Visuals_InitFaceVis( fv, &b->def->faces[i], orient );   // 0x46f7a0 (build + upload)
        else
            fv->vertcount = b->def->faces[i].w ? b->def->faces[i].w->numpoints : 0;
    }

    b->version = b->def->version;
}

// Brush_CheckBuildFaceVis (0x477d70) — rebuild the instance faceVis when the def
// geometry version has advanced past the cached one. (The faithful name in IDB
// callers is sub_477D70.)
void sub_477D70( selbrush_t *b, const float *orient )
{
    iassert( b );        // brush.cpp:3484
    iassert( b->def );   // brush.cpp:3485
    if ( b->version != b->def->version )
    {
        Brush_MakeFaceVisuals( b, orient );
        iassert( b->version == b->def->version );   // brush.cpp:3491 (level 0, post-rebuild invariant)
    }
}

// ─── editor line-draw bridge (draw.cpp) — used by the XY brush draw ────────
extern int  R_Add3DLine( GfxPointVertex *verts, const orientation_t *orient,
                         const float *p1, const float *p2, const unsigned int *color,
                         char width, int vertCount, int maxVertCount );           // 0x40c110
extern void OrientationDirToWorldDir( float *out, const orientation_t *orient,
                                      const float *dir );                          // 0x4ba4b0

// Entity_LinkBrush / Entity_UnlinkBrush ─ combined (0x475730)
// Declared here as Entity_LinkBrush; the IDB alias Entity_LinkBrush_0 is the same fn.
static void Entity_LinkBrush( entity_s *entity, selbrush_t *b );

// Entity_UnlinkBrush wrapper (0x485020): unlinks the brush def from its entity's def-list.
// Brush_Free uses this when refCount drops to 1 (still referenced by its owner).
// IDA 0x485020: thin forwarder; the body lives in entity.cpp.
static void Entity_UnlinkBrush_def( brush_t *def )
{
    Entity_UnlinkBrush( def );
}

// ─── Vis_Free forward (needed by Brush_Free, Brush_InvalidateVis) ────────────
extern void Vis_Free( int count, faceVis_s *f, int brushPtr );

// ─── PlanePts_Alloc forward ──────────────────────────────────────────────────
extern void *PlanePts_Alloc( int count );

// ────────────────────────────────────────────────────────────────────────────
// Face_Alloc_R  (0x470140) — allocates count * 232-byte face entries
// ────────────────────────────────────────────────────────────────────────────
static face_t *Face_Alloc_R( int count )
{
    iassert( count > 0 );
    face_t *v = (face_t *)operator new( (size_t)(232 * count) );
    memset( v, 0, (size_t)(232 * count) );
    return v;
}

// ────────────────────────────────────────────────────────────────────────────
// Face_Free  (0x4701a0) — frees windings in face array then frees the array
// ────────────────────────────────────────────────────────────────────────────
static void Face_Free( int count, face_t *f )
{
    iassert( f );
    iassert( count >= 0 );
    for ( int i = 0; i < count; ++i )
    {
        winding_t *w = f[i].w;
        if ( w )
        {
            --g_windingAlloc;
            free( w );
            f[i].w = nullptr;
        }
    }
    j__free_0( f );
}

// ────────────────────────────────────────────────────────────────────────────
// PlanePts_Alloc  (0x470250) — allocates count*12 zero bytes (faceVis planePts)
// ────────────────────────────────────────────────────────────────────────────
void *PlanePts_Alloc( int count )
{
    iassert( count > 0 );
    void *p = operator new( (size_t)(12 * count) );
    memset( p, 0, (size_t)(12 * count) );
    return p;
}

// ────────────────────────────────────────────────────────────────────────────
// Vis_Free  (0x4702b0) — calls Visuals_VisArray on each slot then frees array
// ────────────────────────────────────────────────────────────────────────────
void Vis_Free( int count, faceVis_s *f, int b )
{
    iassert( b );            // brush.cpp:337
    iassert( f );            // brush.cpp:338
    iassert( count >= 0 );   // brush.cpp:339
    for ( int i = 0; i < count; ++i )
        Visuals_VisArray( (faceVis_s *)((char *)f + 12 * i) );
    j__free_0( f );
}

// ────────────────────────────────────────────────────────────────────────────
// Face_Alloc  (0x471500) — appends a copy of *src to b's face array
// Returns pointer to the newly appended face entry.
// ────────────────────────────────────────────────────────────────────────────
face_t *Face_Alloc( brush_t *b, face_t *f )
{
    iassert( b );             // brush.cpp:829
    iassert( f );             // brush.cpp:830

    face_t *faceList = Face_Alloc_R( b->faceCount + 1 );
    iassert( faceList );   // brush.cpp:833

    // copy existing faces (INCLUDING their winding pointers) to faceList
    if ( b->faceCount )
        memcpy( faceList, b->faces, (size_t)(232 * b->faceCount) );

    // append new face (clone its winding)
    face_t *dst = faceList + b->faceCount;
    memcpy( dst, f, sizeof(face_t) );
    dst->w = f->w ? Winding_Clone( f->w ) : nullptr;

    // The existing faces' windings are now OWNED by newList (the pointers were
    // memcpy'd over). NULL them in the OLD array so the Face_Free below does NOT
    // free them out from under newList — the binary (Face_Alloc 0x471500) zeroes
    // faces[i].w in the OLD array, then frees it. (The previous port nulled
    // newList's windings instead — destroying every face winding and leaving the
    // new array's faces with NULL windings; latent where the caller rebuilt
    // windings, FATAL in Brush_MoveVertex which reads the windings after Face_Alloc.)
    if ( b->faces )
    {
        for ( int i = 0; i < b->faceCount; ++i )
            b->faces[i].w = nullptr;
        Face_Free( b->faceCount, b->faces );
    }

    ++b->faceCount;
    b->faces = faceList;
    return faceList + b->faceCount - 1;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_RemoveFace  (0x471640)
// Removes face at faceIndex from b's face array; shifts remaining faces down.
// Returns byte offset of removed face (mostly ignored by callers).
// ────────────────────────────────────────────────────────────────────────────
unsigned int Brush_RemoveFace( brush_t *b, unsigned int faceIndex )
{
    iassert( b );   // brush.cpp:852
    iassert( faceIndex >= 0 && faceIndex < b->faceCount );   // brush.cpp:853

    face_t *f = b->faces + faceIndex;
    iassert( f );   // brush.cpp:83 (inlined accessor null-check; never fires — faithful to binary)

    winding_t *w = f->w;
    if ( w )
    {
        --g_windingAlloc;
        free( w );
        f->w = nullptr;
    }

    --b->faceCount;
    if ( faceIndex < (unsigned int)b->faceCount )
    {
        unsigned int dstOff = 232 * faceIndex;
        for ( unsigned int i = faceIndex; i < (unsigned int)b->faceCount; ++i )
        {
            memcpy( (char *)b->faces + 232 * i,
                    (char *)b->faces + 232 * (i + 1), 232 );
        }
        return 232 * faceIndex;
    }
    return faceIndex;
}

// ────────────────────────────────────────────────────────────────────────────
// sub_471760  (0x471760) — face-index comparator for sorting
// Compares plane.dist of face[fi1] vs face[fi2]; returns -1/0/1
// ────────────────────────────────────────────────────────────────────────────
int Brush_FaceIndexCmp( unsigned int faceIndex2, brush_t *b, unsigned int faceIndex )
{
    iassert( faceIndex >= 0 && faceIndex < b->faceCount );     // brush.cpp:883
    iassert( faceIndex2 >= 0 && faceIndex2 < b->faceCount );   // brush.cpp:884

    double dist1 = (double)b->faces[faceIndex2].plane.dist;
    double dist2 = (double)b->faces[faceIndex].plane.dist;

    if ( dist1 < dist2 )
        return 1;
    if ( dist1 > dist2 )
        return -1;

    // equal dist: compare normal components. IDB 0x471806 fcompp of
    // (fi1.normal[c] vs fi2.normal[c]): fi1>fi2 -> -1 (0x471812 jz 4717E1),
    // fi1<fi2 -> +1 (0x471820 jnp 4717F5). (Was inverted.)
    float *n1 = b->faces[faceIndex2].plane.normal;
    float *n2 = b->faces[faceIndex].plane.normal;
    for ( int c = 0; c < 3; ++c )
    {
        if ( n1[c] > (double)n2[c] ) return -1;
        if ( n1[c] < (double)n2[c] ) return  1;
    }

    // equal normals: compare unk03 (axis alignment). IDB 0x47182d-471841 cmp
    // (fi2.unk03 vs fi1.unk03): fi2<fi1 -> -1 (jl 4717E1), else setnle(fi2>fi1)
    // i.e. fi1>fi2 -> -1, fi1<fi2 -> +1. (Was inverted.)
    int a1 = b->faces[faceIndex2].unk03;
    int a2 = b->faces[faceIndex].unk03;
    if ( a1 > a2 ) return -1;
    if ( a1 < a2 ) return  1;
    return 0;
}

// ────────────────────────────────────────────────────────────────────────────
// Face_InitMaterialChannel  (0x472C90) — initialises one material channel
// channel: 0=$default, 1=lightmap_gray, 2=smoothing_hard
// ────────────────────────────────────────────────────────────────────────────
static int Face_InitMaterialChannel( unsigned int textureChannel, face_t *faceDef,
                                      MaterialDef *src )
{
    static const char *tex_names[3] = { "$default", "lightmap_gray", "smoothing_hard" };
    iassert( faceDef );   // brush.cpp:1383
    vassert( (textureChannel >= 0 && textureChannel < QER_TEX_CHAN_COUNT),
             "(textureChannel) = %i", (int)textureChannel );   // brush.cpp:1384
    patchMesh_material *slot = (patchMesh_material *)&faceDef->mtldef[textureChannel];
    SetMaterial( tex_names[textureChannel], slot );
    return Init_MaterialLayer( (MaterialDef *)slot, src );
}

// ────────────────────────────────────────────────────────────────────────────
// Face_SetDefaultMaterials  (0x472D30)
// Sets material channels 1 and 2 from the d_savedinfo texture table.
// Channel 0 ($default) is skipped.
// ────────────────────────────────────────────────────────────────────────────
static void Face_SetDefaultMaterials( face_t *faceDef )
{
    iassert( faceDef );   // brush.cpp:1397
    // The d_savedinfo material channel table: 3 MaterialDef* entries at stride
    // 0x834 bytes starting at g_qeglobals + 0x700A8 (random_texture_stuff[0x24]).
    // Channel 0 is skipped (only lightmap and smoothing are defaulted here).
    MaterialDef **chan_ptr =
        (MaterialDef **)((char *)&g_qeglobals + 0x700A8);
    for ( unsigned int ch = 0; ch < 3; ++ch )
    {
        if ( ch )
            Face_InitMaterialChannel( ch, faceDef, *chan_ptr );
        chan_ptr = (MaterialDef **)((char *)chan_ptr + 0x834);
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_SetDefaultMaterials  (0x472D90)
// ────────────────────────────────────────────────────────────────────────────
static void Brush_SetDefaultMaterials( brush_t *b )
{
    iassert( b );   // brush.cpp:1413
    for ( int i = 0; i < b->faceCount; ++i )
        Face_SetDefaultMaterials( &b->faces[i] );
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Alloc  (0x4751e0) — allocates and zero-initialises an 88-byte brush_t
// a1 = planepts block (MaterialDef* block copied to each face at +0x24)
// a2 = eclass_t* (or nullptr)
// ────────────────────────────────────────────────────────────────────────────
brush_t *Brush_Alloc( const void *planeptsSrc, eclass_t *ecls )
{
    // Pack colorWhite into GfxColor
    GfxColor col;
    if ( ecls )
        Byte4PackPixelColor( (float *)((char *)ecls + 36), &col );
    else
        Byte4PackPixelColor( const_cast<float*>(colorWhite), &col );

    brush_t *b = (brush_t *)operator new( 0x58u );
    memset( b, 0, 0x58u );
    iassert( b );   // brush.cpp:2354

    if ( b->parent_layer_string )
        j__free_0( b->parent_layer_string );

    size_t layerLen = strlen( g_activeLayer_string );
    void *layerCopy = operator new( layerLen + 1 );
    memcpy( layerCopy, g_activeLayer_string, layerLen + 1 );
    b->parent_layer_string = (char *)layerCopy;

    b->faceCount = 6;
    if ( ecls && (ecls->classtype & 1) != 0 )
        b->faceCount = 8;

    b->faces = Face_Alloc_R( b->faceCount );

    for ( int i = 0; i < b->faceCount; ++i )
    {
        face_t *f = &b->faces[i];
        // copy planepts block (36 bytes) to f->mtldef[0] area
        memcpy( &f->mtldef[0], planeptsSrc, 0x24u );
        // clear certain flags then set packed color
        *(int *)&f->contents &= 0xF7FFDF7B;
        f->packedColor = col.packed;
    }
    Brush_SetDefaultMaterials( b );
    return b;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Create  (0x475300) — fills brush_t face planepts from mins/maxs
// Standard 6-face box; 8-face cylinder if eclass has classtype bit 0.
// NOTE: The function returns face_t* in IDA but modifies b->faces in place.
// ────────────────────────────────────────────────────────────────────────────
void Brush_Create( float *mins, float *maxs, brush_t *b, eclass_t *ecls )
{
    for ( int i = 0; i < 3; ++i )
    {
        if ( mins[i] > maxs[i] )
            Com_Error( ERR_FATAL, "Brush_InitSolid: backwards" );
    }

    face_t *faces = b->faces;

    if ( ecls && (ecls->classtype & 1) != 0 )
    {
        // 8-sided cylinder: 4 top + 4 bottom faces
        float cx = (mins[0] + maxs[0]) * 0.5f;
        float cy = (mins[1] + maxs[1]) * 0.5f;
        float cz = (mins[2] + maxs[2]) * 0.5f;

        // Corner vertices on the top plane at cz
        float vx[4] = { mins[0], mins[0], maxs[0], maxs[0] };
        float vy[4] = { mins[1], maxs[1], maxs[1], mins[1] };

        for ( int i = 0; i < 4; ++i )
        {
            face_t *f = &faces[i];
            f->planepts[0][0] = cx; f->planepts[0][1] = cy; f->planepts[0][2] = maxs[2];
            f->planepts[1][0] = vx[i];   f->planepts[1][1] = vy[i];   f->planepts[1][2] = cz;
            int ni = (i + 1) & 3;
            f->planepts[2][0] = vx[ni];  f->planepts[2][1] = vy[ni];  f->planepts[2][2] = cz;
        }
        for ( int i = 0; i < 4; ++i )
        {
            face_t *f = &faces[4 + i];
            f->planepts[0][0] = cx; f->planepts[0][1] = cy; f->planepts[0][2] = mins[2];
            int ni = (i + 1) & 3;
            f->planepts[1][0] = vx[ni];  f->planepts[1][1] = vy[ni];  f->planepts[1][2] = cz;
            f->planepts[2][0] = vx[i];   f->planepts[2][1] = vy[i];   f->planepts[2][2] = cz;
        }
    }
    else
    {
        // Standard 6-face box (from IDA verbatim order):
        // face 0: +X (right)
        faces[0].planepts[0][0] = maxs[0]; faces[0].planepts[0][1] = maxs[1]; faces[0].planepts[0][2] = mins[2];
        faces[0].planepts[1][0] = mins[0]; faces[0].planepts[1][1] = maxs[1]; faces[0].planepts[1][2] = mins[2];
        faces[0].planepts[2][0] = mins[0]; faces[0].planepts[2][1] = mins[1]; faces[0].planepts[2][2] = mins[2];
        // face 1: -X (left) — reading from IDA (result, result+1, etc.)
        faces[1].planepts[0][0] = mins[0]; faces[1].planepts[0][1] = mins[1]; faces[1].planepts[0][2] = maxs[2];
        faces[1].planepts[1][0] = mins[0]; faces[1].planepts[1][1] = maxs[1]; faces[1].planepts[1][2] = maxs[2];
        faces[1].planepts[2][0] = maxs[0]; faces[1].planepts[2][1] = maxs[1]; faces[1].planepts[2][2] = maxs[2];
        // face 2
        faces[2].planepts[0][0] = mins[0]; faces[2].planepts[0][1] = mins[1]; faces[2].planepts[0][2] = maxs[2];
        faces[2].planepts[1][0] = maxs[0]; faces[2].planepts[1][1] = mins[1]; faces[2].planepts[1][2] = maxs[2];
        faces[2].planepts[2][0] = maxs[0]; faces[2].planepts[2][1] = mins[1]; faces[2].planepts[2][2] = mins[2];
        // face 3: +X (x = maxs.x).  (Earlier port made this a DIAGONAL — pts 1,2 used
        // mins.x — collapsing every fixed-size entity bbox to a degenerate sliver.
        // Corrected verbatim from IDA Brush_Create 0x475300: all three pts at x=maxs.x.)
        faces[3].planepts[0][0] = maxs[0]; faces[3].planepts[0][1] = mins[1]; faces[3].planepts[0][2] = maxs[2];
        faces[3].planepts[1][0] = maxs[0]; faces[3].planepts[1][1] = maxs[1]; faces[3].planepts[1][2] = maxs[2];
        faces[3].planepts[2][0] = maxs[0]; faces[3].planepts[2][1] = maxs[1]; faces[3].planepts[2][2] = mins[2];
        // face 4: +Y (y = maxs.y).  (Earlier port had this as the -X face — swapped with 5.)
        faces[4].planepts[0][0] = maxs[0]; faces[4].planepts[0][1] = maxs[1]; faces[4].planepts[0][2] = maxs[2];
        faces[4].planepts[1][0] = mins[0]; faces[4].planepts[1][1] = maxs[1]; faces[4].planepts[1][2] = maxs[2];
        faces[4].planepts[2][0] = mins[0]; faces[4].planepts[2][1] = maxs[1]; faces[4].planepts[2][2] = mins[2];
        // face 5: -X (x = mins.x).
        faces[5].planepts[0][0] = mins[0]; faces[5].planepts[0][1] = maxs[1]; faces[5].planepts[0][2] = maxs[2];
        faces[5].planepts[1][0] = mins[0]; faces[5].planepts[1][1] = mins[1]; faces[5].planepts[1][2] = maxs[2];
        faces[5].planepts[2][0] = mins[0]; faces[5].planepts[2][1] = mins[1]; faces[5].planepts[2][2] = mins[2];
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Entity_LinkBrush  (0x475730) — combined link/unlink of selbrush_t into entity's
// brush list (ownerNext/ownerPrev chain). Pass entity=nullptr to unlink only.
// ────────────────────────────────────────────────────────────────────────────
static void Entity_LinkBrush( entity_s *entity, selbrush_t *b )
{
    iassert( b );   // brush.cpp:2319

    if ( b->owner )
    {
        iassert( b->ownerNext );   // brush.cpp:2323
        iassert( b->ownerPrev );   // brush.cpp:2324
        // IDA: *(_DWORD *)b->ownerNext->pad = b->ownerPrev
        // "pad" is hex-rays misname for ownerPrev on the next node (see ruling).
        b->ownerNext->ownerPrev = b->ownerPrev;
        b->ownerPrev->ownerNext = b->ownerNext;
        b->ownerNext = nullptr;
        b->ownerPrev = nullptr;
        b->owner     = nullptr;
    }
    else
    {
        iassert( !b->ownerNext );   // brush.cpp:2334
        iassert( !b->ownerPrev );   // brush.cpp:2335
    }

    // brush.cpp:2338/2339/2340 are LEVEL 1 in the binary (post-unlink invariants); iassert is
    // hardcoded level 0 (assertive.h), so converting downgrades the level — KEEP_VERBOSE.
    iassert( b->owner == NULL );   // brush.cpp:2338
    iassert( b->ownerNext == NULL );   // brush.cpp:2339
    iassert( b->ownerPrev == NULL );   // brush.cpp:2340

    if ( entity )
    {
        // IDA: b->owner = entity; b->ownerNext = entity->brushes.ownerNext;
        // *(_DWORD *)result->brushes.ownerNext->pad = b;
        // result->brushes.ownerNext = b; b->ownerPrev = &result->brushes;
        b->owner     = entity;
        b->ownerNext = entity->brushes.ownerNext;
        entity->brushes.ownerNext->ownerPrev = b;
        entity->brushes.ownerNext = b;
        b->ownerPrev = (selbrush_t *)&entity->brushes;
    }
}

// Entity_LinkBrush_0_extern — REAL external bridge to the file-static Entity_LinkBrush
// (0x475730) above, so entity.cpp can relink a selbrush_t INSTANCE's owner-chain into a
// new owner instance. (Was a no-op stub in engine_stubs.cpp, which silently broke
// Select_Ungroup's instance-relink → dangling owner + Entity_Free of a still-linked
// entity; and blocked Entity_Create's brush-entity / merge reparent paths.) The IDB
// fn is __usercall returning the entity in eax; the value is unused at every call site,
// so we return the instance for the declared signature.
// bridge to Entity_LinkBrush.
selbrush_t *Entity_LinkBrush_0_extern( entity_s *e, entity_brush_s *b )
{
    Entity_LinkBrush( e, (selbrush_t *)b );
    return (selbrush_t *)b;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_SetLayerString  (0x4758A0) — replaces b->parent_layer_string
// ────────────────────────────────────────────────────────────────────────────
void Brush_SetLayerString( brush_t *b, const char *str )
{
    iassert( b );   // brush.cpp:2354
    if ( b->parent_layer_string )
        j__free_0( b->parent_layer_string );
    b->parent_layer_string = AllocMaterialString( str );
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_SetInstanceLayerString  (0x4758E0) — sets the def's layer string and
// clears the instance's xx7 flag.
// ────────────────────────────────────────────────────────────────────────────
void Brush_SetInstanceLayerString( selbrush_t *b, const char *str )
{
    iassert( b );   // brush.cpp:2365
    // The binary (0x4758E0) inlines Brush_SetLayerString(b->def, str) here — its
    // 2354 "b" assert is that inlined def-null check; represent the inline as the call.
    Brush_SetLayerString( b->def, str );
    b->xx7 = 0;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_AddToList  (0x475980) — allocates a new selbrush_t INSTANCE for the
// given brush_t DEF, increments def->refCount, sets version/patch, and links
// the instance into owner's entity brush list.
// Returns the new 56-byte selbrush_t instance.
// ────────────────────────────────────────────────────────────────────────────
selbrush_t *Brush_AddToList( brush_t *def, entity_s *owner )
{
    iassert( def );     // brush.cpp:2384
    iassert( owner );   // brush.cpp:2385
    iassert( def->owner == owner->def );   // brush.cpp:2386

    selbrush_t *b = (selbrush_t *)operator new( 0x38u );
    memset( b, 0, 0x38u );
    b->def = (brush_t *)def;
    b->version = (short)(def->version - 1);  // LOWORD set to def->version-1
    b->cullFlag = true;                        // BYTE2 of version+2 = 1
    ++def->refCount;

    if ( def->patch )
    {
        patch_t *patchInst = PMESH_55( def->patch );
        b->patch = patchInst;
        // brush.cpp:2397 is LEVEL 1 in the binary (push 1) — iassert is hardcoded level 0
        // (assertive.h), so converting downgrades the level.  KEEP_VERBOSE.
        iassert( b->patch->def == b->def->patch );   // brush.cpp:2397
    }

    Entity_LinkBrush( owner, b );

    iassert( b->owner == owner );   // brush.cpp:2401
    iassert( b->owner->def == b->def->owner );   // brush.cpp:2402

    return b;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Free_R  (0x475af0) — frees a brush_t DEF (must have refCount == 0)
// ────────────────────────────────────────────────────────────────────────────
void Brush_Free_R( brush_t *def )
{
    iassert( def );   // brush.cpp:2422
    vassert( (def->refCount == 0), "(def->refCount) = %i", def->refCount );   // brush.cpp:2423

    if ( def->patch )
        PMESH_32_Symbiot( (int)def->patch );
    if ( def->faces )
        Face_Free( def->faceCount, def->faces );
    if ( def->parent_layer_string )
        j__free_0( def->parent_layer_string );
    iassert( def->onext == NULL );   // brush.cpp:2434
    operator delete( def );
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Clone  (0x475d20) — deep-copies a brush_t DEF into a new brush_t DEF.
// If the source has a patch, duplicates it (returns the patch's symbiont instance).
// The IDB takes the brush_t DEF directly: its `entity_brush_s *a1` + `a1[1].field`
// indexing is just how it reaches def fields, which sit at byte 56+ when the 56-byte
// entity_brush_s is indexed [1].  Reads are def's OWN patch(0x50) / owner(0x08) /
// numberId(0x54, from a1[1].refCount) / faceCount(0x40).  It copies numberId, NOT version.
// ────────────────────────────────────────────────────────────────────────────
brush_t *Brush_Clone( brush_t *def )
{
    if ( def->patch )
    {
        patchMesh_t *dup = Patch_Duplicate( (patchMesh_t *)(intptr_t)def->patch );
        Entity_UnlinkBrush( (brush_t *)dup->pSymbiot );  // IDA: Entity_UnlinkBrush(&pSymbiot->oprev) = pSymbiot@0
        return (brush_t *)dup->pSymbiot;
    }

    brush_t *b = (brush_t *)operator new( 0x58u );
    memset( b, 0, 0x58u );
    iassert( b );   // brush.cpp:2354

    if ( b->parent_layer_string )
        j__free_0( b->parent_layer_string );

    size_t layerLen = strlen( g_activeLayer_string );
    void *layerCopy = operator new( layerLen + 1 );
    memcpy( layerCopy, g_activeLayer_string, layerLen + 1 );
    b->parent_layer_string = (char *)layerCopy;

    b->owner    = def->owner;
    b->numberId = def->numberId;       // IDB: v5->total_size_0x58 = a1[1].refCount (= def numberId)
    b->faceCount = def->faceCount;
    b->faces = Face_Alloc_R( b->faceCount );

    for ( int i = 0; i < b->faceCount; ++i )
    {
        face_t *dst  = &b->faces[i];
        face_t *srcc = &def->faces[i];
        // copy mtldef block (0x6C bytes at offset +0x24=36)
        memcpy( &dst->mtldef[0], &srcc->mtldef[0], 0x6Cu );
        // copy extra fields at stride 144 from planepts
        memcpy( &dst->mtldef[3], &srcc->mtldef[3], 0x24u );
        // copy packed color and flags verbatim
        dst->packedColor = srcc->packedColor;
        dst->contents    = srcc->contents;
        dst->toolflags   = srcc->toolflags;
        // copy planepts
        memcpy( dst->planepts, srcc->planepts, 0x24u );
    }
    return b;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Deselect_Helper  (0x476330) — clears BRUSHFLAG_SELECTED and updates
// selection counters. Called from Brush_RemoveFromList.
// ────────────────────────────────────────────────────────────────────────────
// the version bump is def+0x78, 16-bit (see inline).
void Brush_Deselect_Helper( selbrush_t *b )
{
    iassert( b->brushFlags & BRUSHFLAG_SELECTED );   // brush.cpp:2676

    b->brushFlags &= ~0x80u;

    // eclass / modelClass / version live on the entity DEF, NOT the instance: the
    // instance (Prefab_Init operator new(0x54)=84B) has no eclass field (it's at +0x60).
    // The binary reads them all via owner->def (IDB sub_476330). Reading
    // them off the instance walks past its allocation and faults when the adjacent heap
    // block is freed (0xDDDDDDDD) — the delete-path UAF. (entity_s_def == entity_s typedef.)
    entity_s *eDef = (entity_s *)b->owner->def;

    if ( b->brushFlags & 0x100 )
    {
        b->brushFlags &= ~0x100u;
        eDef->modelClass = nullptr;
        ++*(unsigned __int16 *)&eDef->version_prob_wrong;   // IDA 0x476383: add word ptr [def+0x78],1 (def model version @0x78, 16-bit — NOT version@0x4C)
        b->def->unk01 = 0;
        Entity_RebuildBounds( eDef );
    }

    --g_qeglobals.d_select_count;

    if ( b->owner == world_entity )
    {
        --g_qeglobals.d_select_info.numBrushesAndPatches;
        if ( b->patch )
            --g_qeglobals.d_select_info.numBrushes;
        else
            --g_qeglobals.d_select_info.numPatches;
    }
    else
    {
        entity_s *ent = b->owner;
        if ( eDef->eclass->fixedsize )
        {
            --g_qeglobals.d_select_info.numFixedSize;
        }
        else if ( b->patch )
        {
            --g_qeglobals.d_select_info.numBrushes;
        }
        else
        {
            --g_qeglobals.d_select_info.numPatches;
        }

        // check if any other selected brush in same entity still has SELECTED flag
        int found = 0;
        for ( selbrush_t *v = selected_brushes.next;
              v != &selected_brushes; v = v->next )
        {
            if ( v->owner == ent && (v->brushFlags & 0x80) != 0 )
            {
                found = 1;
                break;
            }
        }
        if ( !found )
        {
            ent->mapLayer = (char *)((intptr_t)ent->mapLayer & ~0x80);
            --g_qeglobals.d_select_info.numEntWithFlag;
        }
    }

    iassert( g_qeglobals.d_select_count == (g_qeglobals.d_select_info.numBrushes
             + g_qeglobals.d_select_info.numPatches
             + g_qeglobals.d_select_info.numFixedSize) );   // brush.cpp:2720
}

// IDA name aliases — entity.cpp uses the original sub_NNN names.
// Keep these thin wrappers so we don't have to touch entity.cpp.
// 0x476330 — thin alias of Brush_Deselect_Helper.
void sub_476330( selbrush_t *b ) { Brush_Deselect_Helper( b ); }

// ────────────────────────────────────────────────────────────────────────────
// Brush_Select_Helper  (0x476470) — sets BRUSHFLAG_SELECTED and updates counters
// Called from Brush_AddToList2 BEFORE linking into selected_brushes.
// ────────────────────────────────────────────────────────────────────────────
// the version bump is def+0x78, 16-bit (see inline).
void Brush_Select_Helper( selbrush_t *b )
{
    iassert( !(b->brushFlags & BRUSHFLAG_SELECTED) );   // brush.cpp:2726

    b->brushFlags |= 0x80u;

    // eclass / modelClass / version live on the entity DEF (via owner->def),
    // NOT the 84-byte instance — see the Brush_Deselect_Helper note above. (IDB sub_476470.)
    entity_s *eDef = (entity_s *)b->owner->def;

    if ( g_qeglobals.w_cyclePreviewMode )
    {
        eclass_t *cls = eDef->eclass;
        if ( cls && cls->cycleModelName[(unsigned short)g_qeglobals.w_cyclePreviewMode] )
        {
            b->brushFlags |= 0x100u;
            eDef->modelClass = nullptr;
            ++*(unsigned __int16 *)&eDef->version_prob_wrong;   // IDA 0x4764e7: add [def+0x78],cx (def model version @0x78, 16-bit — NOT version@0x4C)
            b->def->unk01 = 0;
        }
    }

    ++g_qeglobals.d_select_count;

    if ( b->owner == world_entity )
    {
        ++g_qeglobals.d_select_info.numBrushesAndPatches;
        if ( b->patch )
            ++g_qeglobals.d_select_info.numBrushes;
        else
            ++g_qeglobals.d_select_info.numPatches;
    }
    else
    {
        entity_s *ent = b->owner;
        if ( eDef->eclass->fixedsize )
        {
            ++g_qeglobals.d_select_info.numFixedSize;
        }
        else if ( b->patch )
        {
            ++g_qeglobals.d_select_info.numBrushes;
        }
        else
        {
            ++g_qeglobals.d_select_info.numPatches;
        }

        if ( ((intptr_t)ent->mapLayer & 0x80) == 0 )
        {
            ent->mapLayer = (char *)((intptr_t)ent->mapLayer | 0x80);
            ++g_qeglobals.d_select_info.numEntWithFlag;
        }
    }

    iassert( g_qeglobals.d_select_count == (g_qeglobals.d_select_info.numBrushes
             + g_qeglobals.d_select_info.numPatches
             + g_qeglobals.d_select_info.numFixedSize) );   // brush.cpp:2758
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_AddToList2  (0x4765A0) — links an existing selbrush_t into the
// selected_brushes display list (tail-insertion, circular doubly-linked).
// ────────────────────────────────────────────────────────────────────────────
void Brush_AddToList2( selbrush_t *b )
{
    if ( b->next || b->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: allready linked" );

    if ( g_qeglobals.d_select_count < 2 )
        g_qeglobals.d_select_order[g_qeglobals.d_select_count] =
            (entity_brush_s *)b;

    Brush_Select_Helper( b );

    // tail-insertion into selected_brushes circular list
    // selected_brushes.prev = current tail; list iterates via .next
    selbrush_t *tail = selected_brushes.prev;
    b->prev = tail;
    tail->next = b;
    selected_brushes.prev = b;
    b->next = &selected_brushes;

    b->brushFlags &= ~0x1Fu;
    b->xx6 = 0;
    b->xx7 = 0;

    if ( b->patch )
        b->patch->selected = 1;

    UpdateSelection( -1, 0 );
    SurfaceInspector::UpdateSurfaceDialog();
    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
}

// IDA name alias for Brush_Select_Helper.
// 0x476470 — thin alias of Brush_Select_Helper (ICF fold).
void sub_476470( selbrush_t *b ) { Brush_Select_Helper( b ); }

// ────────────────────────────────────────────────────────────────────────────
// Brush_RemoveFromList  (0x476680) — unlinks a selbrush_t from the display list
// ────────────────────────────────────────────────────────────────────────────
void Brush_RemoveFromList( selbrush_t *b )
{
    selbrush_t *next = b->next;
    selbrush_t *prev = b->prev;
    if ( !next || !prev )
        Com_Error( ERR_FATAL, "Brush_RemoveFromList: not linked" );

    int wasSelected = (b->brushFlags >> 7) & 1;
    next->prev = prev;
    prev->next = next;
    b->prev = nullptr;
    b->next = nullptr;

    if ( wasSelected )
    {
        Brush_Deselect_Helper( b );
        if ( b->patch )
            b->patch->selected = 0;
        UpdateSelection( -1, 0 );
        SurfaceInspector::UpdateSurfaceDialog();
        if ( g_PatchDialog_GetHwnd() )
            g_PatchDialog_GetPatchInfo();
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Free  (0x475ba0) — full cleanup of a selbrush_t instance
// ────────────────────────────────────────────────────────────────────────────
// the head removes the CamWnd light-preview record (sub_4062D0).
void Brush_Free( selbrush_t *b )
{
    // IDA 0x475ba3: drop this brush's cached light-preview record from the camera window
    // (a2@<ebx> = g_pParentWnd->m_pCamWnd, offset 0x7C0; only when the cam window exists).
    CCamWnd *camWnd = g_pParentWnd ? g_pParentWnd->m_pCamWnd : nullptr;
    if ( camWnd )
        CCamWnd_RemoveLightPreview( b, camWnd );

    if ( b->next )
        Brush_RemoveFromList( b );

    if ( b->patch )
        PMESH_33( b->patch );

    if ( b->faces )
        Vis_Free( b->faceCount, b->faces, (int)(intptr_t)b );

    Entity_LinkBrush( nullptr, b );

    iassert( b->def );   // brush.cpp:2459

    brush_t *def = b->def;
    if ( def->owner && def->onext && def->refCount == 2 )
        Entity_UnlinkBrush_def( def );

    vassert( (b->def->refCount > 0), "(b->def->refCount) = %i", b->def->refCount );   // brush.cpp:2464

    if ( --b->def->refCount == 0 )
        Brush_Free_R( b->def );

    operator delete( b );
}

// ═════════════════════════════════════════════════════════════════════════════
//  Brush winding generation — the CoD "CM" half-space pipeline.
//
//  CoD4Radiant builds a brush's face windings the collision-model way, NOT Q3R's
//  plane-chop way: enumerate every triple of face planes, keep the intersection points
//  that lie inside all other half-spaces (CM_AddSimpleBrushPoint), then per face
//  gift-wrap the points lying on that plane into a convex polygon
//  (CM_BuildBrushWindingForSide).  Cluster 0x470470-0x471420 + CM_ReverseWinding 0x4d7dc0.
//
//  winding_t = { int numpoints; float p[][3]; } allocated 12*N+4 bytes; g_windingAlloc
//  (IDB dword_24CE4FC) tracks the live count. Plane format for the kisak math is
//  {nx,ny,nz,dist}.
// ═════════════════════════════════════════════════════════════════════════════

// These editor CM_* names collide with kisak's collision-model CM_* in qcommon.h
// (homonyms with different signatures — the editor brush.cpp set vs the engine cm_*.cpp
// set). Wrap the editor pipeline in a namespace so both coexist; the IDB names + EAs
// stay readable. Brush_BuildWindings qualifies its calls with `edwind::`.
namespace edwind {

// 0x470a90  CM_PointInList — is `pt` already in xyzList (within 0.1²·3 per-axis)?
static int CM_PointInList( const float *xyzList, int count, const float *pt )
{
    for ( int i = 0; i < count; ++i )
    {
        const float *p = &xyzList[3 * i];
        float dx = p[0] - pt[0], dy = p[1] - pt[1], dz = p[2] - pt[2];
        if ( dx * dx <= 0.01f && dy * dy <= 0.01f && dz * dz <= 0.01f )
            return 1;
    }
    return 0;
}

// 0x470af0  CM_GetXyzList — collect (deduped) intersection points that lie on face
// `faceIdx` into xyzList; returns the count.
// KISAK: 1024-point cap; the IDB warns then overruns, the port breaks.
static int CM_GetXyzList( int ptCount, int faceIdx, const BrushPt_t *brushPts, float *xyzList )
{
    int count = 0;
    for ( int i = 0; i < ptCount; ++i )
    {
        const BrushPt_t *bp = &brushPts[i];
        if ( ( faceIdx == bp->sideIndex[0] || faceIdx == bp->sideIndex[1] || faceIdx == bp->sideIndex[2] )
             && !CM_PointInList( xyzList, count, bp->xyz ) )
        {
            if ( count >= 1024 )
            {
                Com_PrintMessage( "Winding point limit (%i) exceeded on brush face", 1024 );
                break;                       // (IDB warns then overruns; we cap)
            }
            xyzList[3 * count + 0] = bp->xyz[0];
            xyzList[3 * count + 1] = bp->xyz[1];
            xyzList[3 * count + 2] = bp->xyz[2];
            ++count;
        }
    }
    return count;
}

// 0x470b70  CM_PickProjectionAxes — the two non-dominant axes of `normal`.
static void CM_PickProjectionAxes( const float *normal, int *axis1, int *axis2 )
{
    int dom = ( fabsf( normal[1] ) > fabsf( normal[0] ) );   // 0 or 1
    if ( fabsf( normal[2] ) > fabsf( normal[dom] ) )
        dom = 2;
    *axis1 = ( ( dom & 1 ) == 0 );      // dom0->1 dom1->0 dom2->1
    *axis2 = ( ~dom ) & 2;              // dom0->2 dom1->2 dom2->0
}

// 0x470c20  CM_AddColinearExteriorPointToWindingProjected — when the new point is
// (nearly) colinear with the best edge, extend that edge by replacing its further
// endpoint rather than inserting a new vertex.
// 572/597/589 KEEP_VERBOSE (shared-winding w->pts form).
static void CM_AddColinearExteriorPointToWindingProjected(
        winding_t *w, const float *pt, int i, int j, int index0, int index1 )
{
    float *pts = &w->p[0][0];

    iassert( w->pts[index0][i] != w->pts[index1][i] || w->pts[index0][j] != w->pts[index1][j] );   // brush.cpp:572

    float di = pts[3 * index1 + i] - pts[3 * index0 + i];
    float dj = pts[3 * index1 + j] - pts[3 * index0 + j];
    int   axis;
    float delta;
    if ( fabsf( dj ) > fabsf( di ) ) { delta = dj; axis = j; }
    else                             { delta = di; axis = i; }

    float p0 = pts[3 * index0 + axis];
    float p1 = pts[3 * index1 + axis];
    int   replaceIdx;
    if ( delta <= 0.0f )
    {
        iassert( w->pts[index0][axis] > w->pts[index1][axis] );   // brush.cpp:597
        if ( p0 < pt[axis] )            replaceIdx = index0;
        else if ( p1 <= pt[axis] )      return;
        else                            replaceIdx = index1;
    }
    else
    {
        iassert( w->pts[index0][axis] < w->pts[index1][axis] );   // brush.cpp:589
        if ( p0 <= pt[axis] )
        {
            if ( p1 >= pt[axis] )       return;
            replaceIdx = index1;
        }
        else                            replaceIdx = index0;
    }
    pts[3 * replaceIdx + 0] = pt[0];
    pts[3 * replaceIdx + 1] = pt[1];
    pts[3 * replaceIdx + 2] = pt[2];
}

// 0x470df0  CM_AddExteriorPointToWindingProjected — incremental convex-hull insert
// of `pt` into the (projected) winding: find the edge `pt` is most exterior to and
// insert/extend there.
// the insert memcpy is a backward shift; overlap-safe equivalent.
static void CM_AddExteriorPointToWindingProjected( const float *pt, winding_t *w, int a3, int a4 )
{
    int n = w->numpoints;
    if ( n == 0 )
        return;

    float *pts = &w->p[0][0];
    int   bestIndex = -1;
    float minArea   = 3.4028235e38f;
    int   prev      = n - 1;
    for ( int index = 0; index < n; ++index )
    {
        const float *cur = &pts[3 * index];
        const float *prv = &pts[3 * prev];
        float area = ( cur[a4] - pt[a4] ) * prv[a3]
                   + ( prv[a4] - cur[a4] ) * pt[a3]
                   + ( pt[a4]  - prv[a4] ) * cur[a3];
        if ( area < minArea ) { minArea = area; bestIndex = index; }
        prev = index;
    }

    if ( minArea < -0.001f )
    {
        for ( int k = n; k > bestIndex; --k )
        {
            pts[3 * k + 0] = pts[3 * ( k - 1 ) + 0];
            pts[3 * k + 1] = pts[3 * ( k - 1 ) + 1];
            pts[3 * k + 2] = pts[3 * ( k - 1 ) + 2];
        }
        pts[3 * bestIndex + 0] = pt[0];
        pts[3 * bestIndex + 1] = pt[1];
        pts[3 * bestIndex + 2] = pt[2];
        ++w->numpoints;
    }
    else if ( minArea <= 0.001f )
    {
        CM_AddColinearExteriorPointToWindingProjected(
            w, pt, a3, a4, ( n + bestIndex - 1 ) % n, bestIndex );
    }
}

// 0x470f20  CM_RepresentativeTriangleFromWinding — the winding triangle of largest
// area projected along `normal`; outputs its 3 vertex indices, returns the area.
static float CM_RepresentativeTriangleFromWinding(
        const winding_t *w, const float *normal, int *outI, int *outJ, int *outK )
{
    float best = 0.0f;
    *outI = 0; *outJ = 1; *outK = 2;
    const float *pts = &w->p[0][0];
    int n = w->numpoints;
    for ( int k = 2; k < n; ++k )
        for ( int j = 1; j < k; ++j )
            for ( int i = 0; i < j; ++i )
            {
                float e_kj[3] = { pts[3*k+0]-pts[3*j+0], pts[3*k+1]-pts[3*j+1], pts[3*k+2]-pts[3*j+2] };
                float e_ij[3] = { pts[3*i+0]-pts[3*j+0], pts[3*i+1]-pts[3*j+1], pts[3*i+2]-pts[3*j+2] };
                float cr[3];
                Vec3Cross( e_ij, e_kj, cr );
                float a = fabsf( normal[0]*cr[0] + normal[1]*cr[1] + normal[2]*cr[2] );
                if ( best < a ) { best = a; *outI = i; *outJ = j; *outK = k; }
            }
    return best;
}

// 0x4d7dc0  CM_ReverseWinding — a freshly-allocated winding with reversed point order.
static winding_t *CM_ReverseWinding( const winding_t *w )
{
    int n = w->numpoints;
    ++g_windingAlloc;
    winding_t *out = (winding_t *)malloc( 12 * n + 4 );
    if ( !out )
        Com_PrintMessage( "out of memory: winding_t\n" );
    const float *src = &w->p[0][0];
    float       *dst = &out->p[0][0];
    for ( int i = 0; i < n; ++i )
    {
        const float *s = &src[3 * ( n - 1 - i )];
        dst[3*i+0] = s[0]; dst[3*i+1] = s[1]; dst[3*i+2] = s[2];
    }
    out->numpoints = n;
    return out;
}

// 0x471090  CM_BuildBrushWindingForSide — convex winding for one face from the brush's
// intersection points, oriented to agree with the face plane normal.
// orientation chain wn = (p[kk]-p[ii]) x (p[jj]-p[ii]).
winding_t *CM_BuildBrushWindingForSide( int faceIdx, BrushPt_t *brushPts, brush_t *def, int ptCount )
{
    static float xyzList[1024 * 3];      // (IDB on-stack; static keeps the frame small)
    int xyzCount = CM_GetXyzList( ptCount, faceIdx, brushPts, xyzList );
    if ( xyzCount < 3 )
        return nullptr;

    face_t *face = &def->faces[faceIdx];
    int projAxis1, projAxis2;
    CM_PickProjectionAxes( face->plane.normal, &projAxis1, &projAxis2 );

    ++g_windingAlloc;
    winding_t *w = (winding_t *)malloc( 12 * xyzCount + 4 );
    if ( !w )
        Com_PrintMessage( "out of memory: winding_t\n" );
    float *pts = &w->p[0][0];
    pts[0]=xyzList[0]; pts[1]=xyzList[1]; pts[2]=xyzList[2];
    pts[3]=xyzList[3]; pts[4]=xyzList[4]; pts[5]=xyzList[5];
    w->numpoints = 2;

    for ( int m = 0; m < xyzCount - 2; ++m )
        CM_AddExteriorPointToWindingProjected( &xyzList[3 * ( m + 2 )], w, projAxis1, projAxis2 );

    int ii, jj, kk;
    // edwind:: qualified to suppress ADL — winding_t is global, so an unqualified call
    // would also find kisak's collision-model homonym (qcommon.h) and be ambiguous.
    float area = edwind::CM_RepresentativeTriangleFromWinding( w, face->plane.normal, &ii, &jj, &kk );
    if ( area < 0.001f )
    {
        --g_windingAlloc;
        free( w );
        return nullptr;
    }

    // Orientation: winding normal = (p[kk]-p[ii]) × (p[jj]-p[ii]) (PlaneFromPoints_Real
    // convention). Reverse the winding if it opposes the face plane normal. The cross
    // need not be normalised — only the sign of the dot matters.
    float *P  = &w->p[0][0];
    float ca[3] = { P[3*kk+0]-P[3*ii+0], P[3*kk+1]-P[3*ii+1], P[3*kk+2]-P[3*ii+2] };
    float ba[3] = { P[3*jj+0]-P[3*ii+0], P[3*jj+1]-P[3*ii+1], P[3*jj+2]-P[3*ii+2] };
    float wn[3];
    Vec3Cross( ca, ba, wn );
    float dot = face->plane.normal[0]*wn[0] + face->plane.normal[1]*wn[1] + face->plane.normal[2]*wn[2];
    if ( dot >= 0.0f )
        return w;

    winding_t *rev = edwind::CM_ReverseWinding( w );   // edwind:: — see note above
    --g_windingAlloc;
    free( w );
    return rev;
}

// 0x4779e0  CM_AddSimpleBrushPoint — record `xyz` (the intersection of the 3 planes in
// `sideIndices`) iff it lies inside every other face half-space.
static int CM_AddSimpleBrushPoint( brush_t *def, const int *sideIndices, const float *xyz,
                                   int ptCount, int numsides, BrushPt_t *brushPts )
{
    int faceCount = def->faceCount;
    for ( int i = 0; i < faceCount; ++i )
    {
        if ( i != sideIndices[0] && i != sideIndices[1] && i != sideIndices[2] )
        {
            const plane_t *pl = &def->faces[i].plane;
            float d = pl->normal[0]*xyz[0] + pl->normal[1]*xyz[1] + pl->normal[2]*xyz[2] - pl->dist;
            if ( d > 0.1f )
                return ptCount;             // outside this half-space — reject
        }
    }
    if ( ptCount == numsides )
    {
        Com_PrintMessage( "More than %i pts from plane intersections on %i-sided brush\n",
                          ptCount, faceCount );
        return ptCount;
    }
    brushPts[ptCount].xyz[0] = xyz[0];
    brushPts[ptCount].xyz[1] = xyz[1];
    brushPts[ptCount].xyz[2] = xyz[2];
    brushPts[ptCount].sideIndex[0] = sideIndices[0];
    brushPts[ptCount].sideIndex[1] = sideIndices[1];
    brushPts[ptCount].sideIndex[2] = sideIndices[2];
    return ptCount + 1;
}

// 0x470880  CM_ForEachBrushPlaneIntersection — fill brushPts[] with the brush's
// half-space vertices (every inside triple-plane intersection); returns the count.
// SnapPointToIntersectingPlanes uses the kisak com_math reimpl
// (its 0.25f/0.01f match flt_6F42F0/flt_6F4278).
int CM_ForEachBrushPlaneIntersection( brush_t *def, BrushPt_t *brushPts )
{
    int faceCount = def->faceCount;
    if ( faceCount == 2 )
        return 0;

    int ptCount = 0;
    float pl0[4], pl1[4], pl2[4];
    const float *planes[3] = { pl0, pl1, pl2 };
    for ( int i = 0; i + 2 < faceCount; ++i )
    {
        const plane_t *pi = &def->faces[i].plane;
        pl0[0]=pi->normal[0]; pl0[1]=pi->normal[1]; pl0[2]=pi->normal[2]; pl0[3]=pi->dist;
        for ( int j = i + 1; j + 1 < faceCount; ++j )
        {
            const plane_t *pj = &def->faces[j].plane;
            pl1[0]=pj->normal[0]; pl1[1]=pj->normal[1]; pl1[2]=pj->normal[2]; pl1[3]=pj->dist;
            for ( int k = j + 1; k < faceCount; ++k )
            {
                const plane_t *pk = &def->faces[k].plane;
                pl2[0]=pk->normal[0]; pl2[1]=pk->normal[1]; pl2[2]=pk->normal[2]; pl2[3]=pk->dist;
                float xyz[3];
                if ( IntersectPlanes( planes, xyz ) )
                {
                    SnapPointToIntersectingPlanes( planes, xyz, 0.25f, 0.01f );
                    int sideIdx[3] = { i, j, k };
                    ptCount = CM_AddSimpleBrushPoint( def, sideIdx, xyz, ptCount, 1024, brushPts );
                }
            }
        }
    }
    return ptCount;
}

// 0x470a50  Brush_MakeFacePlanes — recompute every face plane from its planepts.
void Brush_MakeFacePlanes( brush_t *b )
{
    for ( int i = 0; i < b->faceCount; ++i )
        Face_MakePlane( &b->faces[i] );
}

// 0x431690  CM_CheckBrushContents — true iff the two 8-byte blocks at face+0x24
// (the base mtldef's lyrMtl/radMtl handle pair) are identical (base-material
// homogeneity test). Affects def->xx1 only, not geometry.
bool CM_CheckBrushContents( const int *a, const int *b )
{
    return memcmp( a, b, 8 ) == 0;
}

// 0x4a8240  VectorMaxValues — expand [mins,maxs] to include point `pt`.
void VectorMaxValues( const float *pt, float *mins, float *maxs )
{
    for ( int i = 0; i < 3; ++i )
    {
        if ( mins[i] > pt[i] ) mins[i] = pt[i];
        if ( maxs[i] < pt[i] ) maxs[i] = pt[i];
    }
}

} // namespace edwind

// ─────────────────────────────────────────────────────────────────────────────
// sub_47ABE0  (0x47ABE0, brush.cpp:4618)  — Brush_AccumulateWorldBounds
// Expand [mins,maxs] to enclose every winding point of brush DEF `b`, transformed
// to world space by orientation `orient`.  Used by Entity_InitPrefabInst to compute
// the placed prefab's overall bbox from each instanced brush's def windings.
//
// IDA reads b->faceCount (b+0x40), b->faces (b+0x44, a face_t[]), and per face
// the winding b->faces[i].w (face_t+0xE0); per winding point: OrientationPos-
// ToWorldPos then VectorMaxValues.  (face_t stride 232, w at +224 — see qe3.h asserts.)
// ─────────────────────────────────────────────────────────────────────────────
extern void OrientationPosToWorldPos( float *out, const float *localPos,
                                      const orientation_t *orient );          // draw.cpp 0x4BA430
// 0x47ABE0 — 4619 KEEP_VERBOSE ("or" is a reserved word).
void Brush_AccumulateWorldBounds( int orientPtr, int brushPtr, int minsPtr, int maxsPtr )
{
    const orientation_t *orient = (const orientation_t *)orientPtr;
    brush_t             *b      = (brush_t *)brushPtr;
    float               *mins   = (float *)minsPtr;
    float               *maxs   = (float *)maxsPtr;

    iassert( b );      // brush.cpp:4618
    if ( !orient ) Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 4619, 0, "%s", "or" );   // 4619: binary param is "or" (C++ reserved word) — kept verbose
    iassert( mins );   // brush.cpp:4620
    iassert( maxs );   // brush.cpp:4621

    for ( int f = 0; f < b->faceCount; ++f )
    {
        winding_t *w = b->faces[f].w;
        if ( !w )
            continue;
        for ( int i = 0; i < w->numpoints; ++i )
        {
            float worldPt[3];
            OrientationPosToWorldPos( worldPt, w->p[i], orient );
            edwind::VectorMaxValues( worldPt, mins, maxs );
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_BuildWindings  (0x477AC0) — rebuilds all face windings for brush def
// bFull != 0: snap planepts first
// ────────────────────────────────────────────────────────────────────────────
void Brush_BuildWindings( brush_t *def, int bFull )
{
    if ( bFull )
        Brush_SnapPlanepts( def );

    edwind::Brush_MakeFacePlanes( def );

    // The brush's half-space vertices, computed on the stack (IDB used a ~24 KB stack
    // buffer here; capacity 1024 matches MAX_POINTS_ON_WINDING / the CM_* caps).
    static BrushPt_t brushPts[1024];
    int ptCount = edwind::CM_ForEachBrushPlaneIntersection( def, brushPts );

    if ( ptCount < 4 )
        Com_PrintMessage( "Brush_BuildWindings: Encountered degenerate brush.\n" );

    def->contents = -1;
    def->xx1      = (int)(intptr_t)def->faces->mtldef;

    for ( int fi = 0; fi < def->faceCount; ++fi )
    {
        face_t *face = &def->faces[fi];   // 83 "f": dead inlined-accessor null-check on &faces[fi] — elided

        winding_t *w = face->w;
        if ( w )
        {
            --g_windingAlloc;
            free( w );
            face->w = nullptr;
        }
        iassert( face->w == NULL );   // brush.cpp:3422

        face->w = edwind::CM_BuildBrushWindingForSide( fi, brushPts, def, ptCount );

        qtexture_s *lyrMtl = MaterialDef_GetLayeredMaterial( face->mtldef );
        // IDB Brush_BuildWindings 0x477ac0 reads lyrMtl->in_use (offset 0x20), NOT
        // offset 0x04 (= name). The old `((int*)lyrMtl)[1]` was masked by the
        // degenerate-shim null; corrected with the real qtexture_s layout (P5.4).
        int in_use = lyrMtl ? lyrMtl->in_use : 0;

        def->contents &= in_use;

        // IDB 0x477bd3: CM_CheckBrushContents compares the 8 bytes at
        // face+0x24 (`contents___startof_MaterialDef` = &mtldef[0], the base
        // layer's lyrMtl/radMtl handle pair), NOT kisak face+0xB4 (`contents`).
        // xx1 itself was set to &faces[0].mtldef above, so both operands
        // must address the mtldef block. (Was &f->contents — wrong field.)
        int *xx1 = (int *)(intptr_t)def->xx1;
        if ( xx1 && !edwind::CM_CheckBrushContents( (const int *)face->mtldef, xx1 ) )
            def->xx1 = 0;
    }

    // compute AABB from intersection points
    def->mins[0] = def->mins[1] = def->mins[2] =  131072.0f;
    def->maxs[0] = def->maxs[1] = def->maxs[2] = -131072.0f;

    for ( int i = 0; i < ptCount; ++i )
        edwind::VectorMaxValues( brushPts[i].xyz, def->mins, def->maxs );
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_BuildFaceVis (0x477C50) / Brush_EnsureFaceVis (0x477D70) — UNIFIED to the
// binary's single faceVis lifecycle: Brush_MakeFaceVisuals (~:133) IS the faithful
// 0x477C50 body and sub_477D70 (~:198) IS the faithful 0x477D70.  The earlier
// dormant duplicates that lived here were removed during the #26 keystone
// unification (every caller already calls sub_477D70 → Brush_MakeFaceVisuals).
// ────────────────────────────────────────────────────────────────────────────

// ────────────────────────────────────────────────────────────────────────────
// Brush_InvalidateVis  (0x478340) — frees faceVis cache and forces rebuild
// Returns b->def.
// ────────────────────────────────────────────────────────────────────────────
brush_t *Brush_InvalidateVis( selbrush_t *b )
{
    iassert( b );        // brush.cpp:3596
    iassert( b->def );   // brush.cpp:3597

    if ( b->faces )
    {
        j__free_0( b->faces );
        b->faces = nullptr;
        b->faceCount = 0;
    }

    if ( b->patch )
        PMESH_22_Indices( b->patch );

    b->version = (short)(b->def->version - 1);
    return b->def;
}

// ────────────────────────────────────────────────────────────────────────────
// Brush_Build  (0x4786D0) — full rebuild: create face planepts + windings
// ────────────────────────────────────────────────────────────────────────────
void Brush_Build( brush_t *b, float *mins, float *maxs )
{
    iassert( b );                  // brush.cpp:3698
    iassert( b->owner );           // brush.cpp:3699
    iassert( b->owner->eclass );   // brush.cpp:3700

    // Ground truth from Brush_Build's disasm (0x478745 `mov ecx,[ebp+mins]`): ecx (=
    // Brush_Create's arg1, the LOWER bound) comes from this function's `mins`, i.e.
    // Brush_Create(mins, maxs, ...).  Hex-rays renders the call with the first two
    // __fastcall args display-SWAPPED (same artifact at the NewBrushDrag callsite);
    // transcribing that swap trips Brush_InitSolid's "backwards" FATAL.
    Brush_Create( mins, maxs, b, b->owner->eclass );
    Brush_BuildWindings( b, 1 );

    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();

    ++b->version;
}

// ════════════════════════════════════════════════════════════════════════════
//  Brush_RebuildBrush  (0x438760) — rebuilds a PATCH SYMBIONT's 6-face bounding box
//  brush from new mins/maxs after the control grid changed (insert/remove row/col).
//  Called by Patch_Rebuild (pmesh.cpp).
//   1. Round each axis' mins/maxs (floor(2x+0.5)*0.5 = round to nearest 0.5; the
//      operands are already grid-snapped, so this lands on integers).  An axis that
//      collapses to a point is expanded by ±4 — Patch_CalcBounds' degenerate guard.
//   2. PRESERVE the old face[0]'s material block (3 MaterialDef layers = 0x6C bytes)
//      and apply it to every new face; contents/toolflags come from the patch, or from
//      the old face[0] when there is no patch.
//   3. Free the old faces, alloc 6 fresh ones, write the box, Brush_BuildWindings.
//  The IDB inlines the box winding from 4 vertical edges; this port reuses Brush_Create's
//  canonical box layout (0x475300 IS the binary's box builder) — the resulting
//  axis-aligned box is identical, and the symbiont bbox is never serialized (Brush_Write
//  dispatches patches to Patch_Write), so only its bounds/windings matter.
extern void Brush_BuildWindings( brush_t *def, int bFull );  // declared above (0x477AC0)
extern void MarkMapModified();                               // win_qe3.cpp (0x499BB0)
void Brush_RebuildBrush( brush_t *b, float *mins, float *maxs )
{
    // 1. round + expand-degenerate (IDA: floor(0.5 + x + x)*0.5 per axis)
    for ( int i = 0; i < 3; ++i )
    {
        float lo = (float)( floorf( 0.5f + mins[i] + mins[i] ) * 0.5f );
        float hi = (float)( floorf( 0.5f + maxs[i] + maxs[i] ) * 0.5f );
        mins[i] = lo;
        maxs[i] = hi;
        if ( (int)lo == (int)hi )
        {
            mins[i] = lo - 4.0f;
            maxs[i] = hi + 4.0f;
        }
    }

    face_t *oldFaces = b->faces;
    if ( !oldFaces )
        return;                              // no faces → nothing to rebuild

    // 2. snapshot the old material block (3 layers = 0x6C bytes) + contents/flags
    char savedMtl[0x6C];
    memcpy( savedMtl, oldFaces->mtldef, sizeof( savedMtl ) );

    int contents, toolflags;
    if ( b->patch )
    {
        contents  = b->patch->contents;
        toolflags = b->patch->flags;
    }
    else
    {
        contents  = oldFaces->contents;
        toolflags = oldFaces->toolflags;
    }

    Face_Free( b->faceCount, oldFaces );
    b->faces = nullptr;

    // backwards guard (IDA: Com_Error "Brush_RebuildBrush: backwards")
    for ( int i = 0; i < 3; ++i )
    {
        if ( mins[i] > (double)maxs[i] )
            Com_Error( ERR_FATAL, "Brush_RebuildBrush: backwards" );
    }

    // 3. alloc 6 fresh faces, restore material + contents/flags, build the box
    b->faceCount   = 6;
    b->faces = Face_Alloc_R( 6 );
    for ( int i = 0; i < 6; ++i )
    {
        memcpy( b->faces[i].mtldef, savedMtl, sizeof( savedMtl ) );
        b->faces[i].contents  = contents;
        b->faces[i].toolflags = toolflags;
    }
    Brush_Create( mins, maxs, b, nullptr );  // writes the 6 box-face planepts (eclass=null → box)

    Brush_BuildWindings( b, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++b->version;
}

// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────
// CXYWnd::OnSelectionAddToActiveLayer  (0x466930) — context-menu command "Add
// selection to active layer".  AFX msgmap nID = 0x88B9 (35001), an ON_COMMAND on the
// CXYWnd map (pfn 0x466930, entry at 0x6e6e3c; nID dword at pfn-0xC).  Walks
// selected_brushes; per instance replaces def->parent_layer_string with
// g_activeLayer_string and zeroes the instance's xx7 faceVis flag — transcribed inline
// from the 0x466930 disasm (operator new + memcpy of strlen+1).
// KISAK: the binary gates the loop on this->contextmenu_sub_hwnd (CXYWnd+0x110); the
// port routes the command through CMainFrame and has no such field, so that (no-op)
// gate is dropped.  Asserts preserved at the binary's line nums
// (brush.cpp:2365 "b" for the null instance, 2354 "b" for the null def).
// ─────────────────────────────────────────────────────────────────────────────
void CXYWnd_OnSelectionAddToActiveLayer()
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        // (the binary inlines Brush_SetInstanceLayerString here; the helper carries the
        // brush.cpp:2365/2354 checks itself)
        Brush_SetInstanceLayerString( i, g_activeLayer_string );   // free old + dup new + xx7=0
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// sub_48EFF0  (0x48EFF0) — "keep only selection in layer `a1`".  Called by
// CXYWnd::ContextMenuLayerSth (0x466840) for the right-click "select brushes in
// <layer>" item: walks selected_brushes and DESELECTS (moves back to active_brushes)
// every instance whose def->parent_layer_string != a1.
// The loop captures `next` (v1->next at +4) BEFORE Brush_RemoveFromList unlinks v1
// (0x48f00d), then relinks the removed instance at the HEAD of active_brushes by hand
// (0x48f085..0x48f09c) — NOT via Brush_AddToList2 (the binary only takes that branch in
// the degenerate active_brushes==selected_brushes case, which never holds).  next@+4,
// prev at offset 0.  Assert preserved at brush.cpp:2374 ("b", null def).
// ─────────────────────────────────────────────────────────────────────────────
void Layers_KeepOnlySelectionInLayer( const char *a1 )
{
    for ( selbrush_t *v1 = selected_brushes.next; v1 != &selected_brushes; )
    {
        brush_t   *b      = v1->def;            // [edi+14h] — the binary's local `b`
        selbrush_t *next  = v1->next;           // [edi+4]  (captured before relink)
        iassert( b );   // brush.cpp:2374 (0x48f027)

        if ( strcmp( a1, b->parent_layer_string ) )   // not in layer a1 → deselect
        {
            Brush_RemoveFromList( v1 );          // 0x48f060
            if ( v1->next || v1->prev )
                Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );   // 0x48f0bf

            if ( &active_brushes == &selected_brushes )  // never true; matches binary
            {
                Brush_AddToList2( v1 );          // 0x48f07e
            }
            else
            {
                v1->next = active_brushes.next;  // mov [edi+4], ecx
                active_brushes.next->prev = v1;  // mov [edx], edi   (prev @ +0)
                active_brushes.next = v1;        // mov active_brushes.next, edi
                v1->prev = &active_brushes;      // mov [edi], offset active_brushes
            }
        }
        v1 = next;                               // 0x48f0a8
    }
    g_nUpdateBits = -1;                          // 0x48f0b2
}

// ════════════════════════════════════════════════════════════════════════════
//  CHUNK B
// ════════════════════════════════════════════════════════════════════════════

// qtexture_s is now the canonical 40-byte IDB layout in qe3.h (was a wrong local
// char[64]@0 copy).

// ─── sub_51CB70 (r_ed_vertbuf.cpp): releases a vertex buffer / material handle
extern char sub_51CB70( Material *material, int vertCount, int vertHandle );

// ─── Face_FullClone (0x4703a0): deep copies one 232-byte face entry ────────────
// Undo_AddBrush clones the dragged brush def, and the clone copies every face through
// here.  Faithful
// to IDA sub_4703A0 (int* chunk copies): copy the face fields, then DEEP-CLONE the
// winding (a1->w) rather than alias it. Matches the binary's selective copy exactly
// (note unk02 @0xBC is intentionally NOT copied — left zeroed by Face_Alloc_R).
extern winding_t *Winding_Clone( winding_t *w );   // winding.cpp (0x4d7d60)
void Face_FullClone( int src, int dst )
{
    int *a1 = (int *)(intptr_t)src;
    int *a2 = (int *)(intptr_t)dst;
    memcpy( a2 + 9,  a1 + 9,  0x6Cu );   // mtldef bytes 36..143
    memcpy( a2 + 36, a1 + 36, 0x24u );   // mtldef bytes 144..179
    a2[57] = a1[57];                     // field_0xE4 (228)
    a2[45] = a1[45];                     // contents   (180)
    a2[46] = a1[46];                     // toolflags  (184)
    memcpy( a2, a1, 0x24u );             // planepts   (0..35)
    memcpy( a2 + 48, a1 + 48, 0x20u );   // plane + unk03/04 (192..223)
    winding_t *w = (winding_t *)(intptr_t)a1[56];   // face->w (224)
    a2[56] = w ? (int)(intptr_t)Winding_Clone( w ) : 0;
}

// ─── MarkMapModified (0x499bb0) — marks the map as modified ────────────────────
extern void MarkMapModified();

// ─── Orientation helpers (orientation.cpp) ─────────────────────────────────────
extern void OrientationDirToWorldDir( float *out, const orientation_t *orient,
                                       const float *dir );
extern void OrientationPosToWorldPos( float *out, const float *localPos,
                                       const orientation_t *orient );

// ─── forward declarations for Chunk B parse sub-functions ──────────────────────
static brush_t *Brush_ParsePhysicsBox( const char **text );
static brush_t *Brush_ParsePhysCylinder( const char **text );
// Write sub-functions (WriteWriter_t defined near Brush_Write further below)
// — forward declared there as static, prototypes repeated after typedef.

// materialdef.cpp helpers
// MaterialDef_11/13/14 externs removed (p4-final): dead — declared but never
// called here; the real defs live in materialdef.cpp with their true signatures.
extern int  TexWnd_06_LayerCount( int mtlDef, int layerHandle );
namespace LayerMat {
    extern int GetCurrentLayer( MaterialDef *def );
}

// surface/texture helpers
extern void Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                               int uvBase, float sizeX, float sizeY );
extern void sub_4A47D0( int outS, int inVecs );
extern void sub_46F6C0( int mtlDef, int faceDef, int visIdx, int *outData );
extern int  sub_51D1D0( int material, int vertCount, int positions, int sVec,
                         int tVec, float dummy, int normalInfo, int uvData );

// math helpers
extern void Vec3Cross( const float *a, const float *b, float *out );
extern float Vec3Normalize_R( float *v );   // returns pre-normalize length (callers here ignore it)
extern void MatrixInverse44( const void *src, void *dst );
extern void texturevecs_02( int surfDef, int uvVecs, float v5, int normal,
                             float dist, int arg6, int arg7, int arg8 );
extern void sub_4769A0( int a1, int a2, int a3 );

// ─── parsing (real engine parser — universal/q_parse.cpp) ───────────────────
// The CoD4Radiant binary parses brushes through the engine's q_parse.cpp, NOT a
// Radiant-local parser. parseInfo_t / ParseThreadInfo / Com_Parse / Com_ParseExt
// / Com_ParseOnLine / Com_MatchToken / Com_ParseFloat / Com_UngetToken /
// Com_SetSpaceDelimited all live there. The parse global g_parse @ IDB 0x734D50
// is q_parse.cpp's `ParseThreadInfo g_parse[4]`; we touch it only via the API.
// IDB "_GetToken" (0x4B83E0) is byte-identical to Com_ParseOnLine (verified).
// IDB "Com_Parse"-prologue (ungetToken handling) + Com_ParseExt(text,1) == Com_Parse.
#include <universal/q_parse.h>

extern void   Error( const char *fmt, ... );
extern long   j__atol( const char *str );

// Patch_Write (pmesh.cpp, 0x4458f0) declared near Brush_Write where WriteWriter_t exists.
extern void MapLoad_ParseBrush_Layer( int (***writer)(...), int layerStr );
extern void MapLoad_ParseBrush_Content( int contentName, void (***writer)(...),
                                         int val, void *table );
extern int  *contents;   // content table for MapLoad_ParseBrush_Content
extern int  *toolflags;  // toolflags table

// sub_42F8A0 - reads a `<key> "<value>"` line with a default (layers.cpp port).
extern void Map_ParseEntityLayerKey( const char **text, const char *defVal, char *outBuf, const char *key );
extern int  sub_42FB80( const char **text );
extern int  sub_42FBA0( const char **text );
extern char *sub_45AD50( char *name );   // texture name normaliser
extern float sub_4AACC0( float f, int threshold );  // 0x4AACC0 FP-noise cleaner (snap low 12 mantissa bits)

// brush geometry helpers
// Brush_MakeSided (0x4731E0) is defined further down in this file (void, char snap).
// sub_478630 (Brush_RotateFacePlanepts, 0x478630) is defined further down in this file.
static void     Brush_RotateFacePlanepts( const float *pivot, const float *anglesDeg,
                                          brush_t *b );
extern void     AxisToAngles( float *angles, float (*axis)[3] );
extern void     vectoangles( float *angles, int vec );
// sub_4AA220 (0x4AA220) — rotate a vec3 by sequential X/Y/Z Euler angles (engine_stubs.cpp).
extern void     sub_4AA220( const float *in, const float *anglesDeg, float *out );
// 0x4A59C0 (com_math.cpp:1399) is the engine's ClosestApproachOfTwoLines — the
// radiant binary carried its own copy; the port calls the engine fn (com_math.cpp).
extern void __cdecl ClosestApproachOfTwoLines( const float *p1, const float *dir1,
                                               const float *p2, const float *dir2,
                                               float *s, float *t );

// Patch_ParseMesh (0x444ac0, was the mis-named PMESH_TexLayer) — parse a
// mesh/curve block into a patchMesh_t + symbiont brush (pmesh.cpp). The text
// pointer is the IDA usercall ecx arg, normalised to the first cdecl param.
extern brush_t *Patch_ParseMesh( const char **text, int version, int isMesh );

// ════════════════════════════════════════════════════════════════════════════
//  4. Visuals_VisArray  (0x46f590)
//  Releases all render handles in the faceVis array and frees the array.
// ════════════════════════════════════════════════════════════════════════════
// line-71 assert is iassert(visuals->mtlHandle) after a struct-local rename.
void Visuals_VisArray( faceVis_s *f )
{
    if ( f->visCount )
    {
        iassert( f->visArray != NULL );   // brush.cpp:67
        for ( int i = 0; i < f->visCount; ++i )
        {
            faceVisuals_s *visuals = &f->visArray[i];
            iassert( visuals->mtlHandle );    // brush.cpp:71 (struct-local rename faceVisuals_s.material->mtlHandle: 4 sites, no overload collision)
            iassert( visuals->vertHandle );   // brush.cpp:72
            sub_51CB70( visuals->mtlHandle, f->vertcount, visuals->vertHandle );
        }
        j__free_0( f->visArray );
        f->visArray  = nullptr;
        f->visCount  = 0;
    }
    else if ( f->visArray )
    {
        iassert( f->visArray == NULL );   // brush.cpp:63
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  The editor surface-cache faceVis builder.
//
//  Visuals_InitFaceVis (0x46F7A0) builds, per face per material layer, a
//  GfxWorldVertex run in the per-material D3D9 VB pool (Editor_VB_Upload) and records
//  {Material*, vbHandle} in faceVis_s.visArray[layer].  The per-vertex math (world
//  position/normal, tangent/binormal, texdef texcoords, per-face colour) is factored
//  into Face_BuildLayerGeom so the camera surf-cache draw (Cam_DrawFaceCached) and the
//  immediate draw (Cam_DrawFace) emit bit-identical geometry.
//
//  Chain:
//    Ed_Normal_Calc (0x459A60) + Ed_baseaxis — TextureAxisFromPlane.
//    Ed_texturevecs (0x459BA0)               — base S/T axis vectors + s/t indices.
//    Face_MoveTexture (0x45A1C0)             — scale/shift/rotate → 2x4 tex matrix.
//    sub_4A47D0 (0x4A47D0)                   — Vec3Normalize (out-of-place).
//    Ed_SinCos (0x4AABC0)                    — degrees → sin/cos, exact 0/90/180/270.
//    TexWnd_06_LayerCount (0x45D360)         — find the layer's texdef block by key.
//    sub_46F6C0 (0x46F6C0)                   — per-layer face colour (colorTint·entity, else face colour).
//    Ed_PackColor (0x46F410)                 — float[4] → packed RGBA bytes.
//    Ed_Material_GetColorTint (0x51AD50)     — Material "colorTint" constant lookup.
// ════════════════════════════════════════════════════════════════════════════

// materialdef.cpp accessors (real; radMtl ⇒ single layer in the GUI).
extern int   MaterialDef_04( MaterialDef *m );
extern int   MaterialDef_11( MaterialDef *m );
extern int   MaterialDef_13( int visIndex, MaterialDef *m );
extern Material *MaterialDef_14( unsigned int visIndex, MaterialDef *m );

// r_ed_vertbuf.cpp — upload one layer's verts; returns the packed VB handle.
extern unsigned int Editor_VB_Upload( Material *material, int vertCount,
    const float *xyz, const float *tangent, const float *binormal,
    const float *normal, const float *texCoord, const float *color );

// OrientationPosToWorldPos declared further below (line ~3063 cluster); re-declare here.
extern void OrientationPosToWorldPos( float *out, const float *localPos, const orientation_t *orient );

// renderer string hash (r_utils.cpp) — for the material constant lookup.
extern unsigned int R_HashString( const char *string );

// ── texturevecs cluster — MOVED to texturevecs.cpp (the binary's common/texturevecs.cpp
//    TU; its asserts name that file).  Externs for the brush.cpp users: ──
extern void Ed_Normal_Calc( const float *normal, float *xv, float *yv );   // 0x459A60

// ════════════════════════════════════════════════════════════════════════════
//  Patch texture-PICK cluster (Drag_Begin middle-button, when the hit brush is a
//  patch).  Five inline-FPU functions transcribed from DISASM, x87 stack traced
//  op-by-op (hex-rays renders these as phantom/_ETn stack slots):
//
//    Ed_Normal_Calc        (0x459A60)  — TextureAxisFromPlane (already above; reused).
//    Ed_Patch_AffineInverse(0x43CEE0)  — 3x3 inverse of [[u_i, v_i, 1]] (Cramer).
//    Ed_Patch_PlanarTest   (0x4382D0)  — average per-quad normals over the curveDef
//                                        tessellated mesh; verify every CONTROL point
//                                        lies within ±1 of that plane.  out = {N[3],dist}.
//    Ed_Patch_PickSpan     (0x4380B0)  — pick the first of 8 candidate (col,row) index
//                                        triples whose 3 control points are non-collinear.
//    Ed_Patch_GetTexdef    (0x44B620)  — the whole pick: build the plane basis, project
//                                        3 control pts to (u,v)+(s,t), solve the affine
//                                        (u,v)->(s,t), recover size/shift/rotate, then a
//                                        built-in verification loop over ALL control pts.
//
//  BINARY FACT: sub_44B620 has NO success path — every exit is `xor al,al` (return 0);
//  there is no `mov al,1` in the function.  The texdef is the SIDE EFFECT: size[2]/
//  shift[2]/rotate are written iff the affine is valid AND all control points reproduce
//  within tol AND the recovered rotate rounds to an integer degree within 0.001.  The
//  consumer (Drag_Begin, drag.cpp) discards the return value.
//
//  FP constants (read from the IDB):
//    flt_6F43E0 = 0.001                (sub_4382D0 normalize-length guard;
//                                       sub_44B620 rotate roundtrip-check tolerance)
//    dbl_6F40A0 = 0.001                (verify-loop per-texel tolerance, as a double)
//    dbl_6F4578 = 57.2957763671875     (radians -> degrees, 180/pi)
//    dbl_6F4160 = 0.5                  (floorf(x+0.5) rotate-rounding offset)
//    flt_6F40C4 = -1.0                 (sub_4382D0 dot-vs-(-1) planar tolerance, ±1 band)
// ════════════════════════════════════════════════════════════════════════════
extern qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *def );   // materialdef.cpp 0x4314A0

// 0x43CEE0  Ed_Patch_AffineInverse — inverse of M = [[a1[i], a2[i], 1]] (i=0..2),
// Cramer's rule.  a1 = the 3 u's, a2 = the 3 v's (in the tex-plane basis).  Returns 0
// (degenerate) if |det| < 0.001, else fills a3[9] with the inverse and returns 1.
// det band ±0.001 (flt_6F43E0).
static char Ed_Patch_AffineInverse( const float *a1, const float *a2, double *a3 )
{
    double v3 = a2[1] - a2[2];
    double v4 = a1[2] - a1[1];
    double v5 = a1[1] * a2[2];
    double v6 = a1[2] * a2[1];
    double v7 = a1[0] * v3 + a2[0] * v4 + v5 - v6;        // determinant
    if ( v7 > -0.001000000047497451 && v7 < 0.001000000047497451 )
        return 0;
    double v9 = 1.0 / v7;
    a3[0] = v3 * v9;
    a3[1] = ( a2[2] - a2[0] ) * v9;
    a3[2] = ( a2[0] - a2[1] ) * v9;
    a3[3] = v4 * v9;
    a3[4] = ( a1[0] - a1[2] ) * v9;
    a3[5] = ( a1[1] - a1[0] ) * v9;
    a3[6] = ( v5 - v6 ) * v9;
    a3[7] = ( a1[2] * a2[0] - a1[0] * a2[2] ) * v9;
    a3[8] = v9 * ( a1[0] * a2[1] - a1[1] * a2[0] );
    return 1;
}

// 0x4382D0  Ed_Patch_PlanarTest — average the per-quad face normals over the patch's
// tessellated render mesh (curveDef), normalize that to the patch plane normal, then
// verify the (width-1)×(height-1) CONTROL-point sub-grid sits within ±1 of that plane.
// out = {normal[3], dist}.  Returns 1 if planar, else 0.  curveVert_t stride 44 for the
// quad normals (hex-rays `xyz[11..13]` are flat-index artifacts into the NEXT vert),
// drawVert_t stride 80 for the control-point check.
// two cross products per quad, len>0.001 (flt_6F43E0) gate before
// accumulate; plane dist = dot(centroidSum,N)/sampleCount; ±1 plane band.  The binary's
// second outer index v57 is updated `++v5; v57=v5` at the loop bottom, so v57 == v5.
char Ed_Patch_PlanarTest( patchMesh_t *patch, void *outBlk )  // out: float N[3]@0, double dist@16 (non-static: PMESH_34 in pmesh.cpp calls it)
{
    float *out = (float *)outBlk;
    curvePatchDef_t *cd = patch->curveDef;
    curveVert_t     *verts = cd->verts;

    float normal = 0.0f, v53 = 0.0f, v54 = 0.0f;     // running plane-normal sum
    float v49 = 0.0f, v50 = 0.0f, v51 = 0.0f;        // running centroid (source-vertex) sum
    int   v56 = 0;                                   // sample count

    // The binary keeps a second outer-index `v57` updated as `++v5; v57 = v5;` at the loop
    // bottom — so v57 == v5 on every iteration (incl. the first, both seeded to 0).  We use
    // v5 directly for the centroid indices; documented here so the transcription is exact.
    for ( int v5 = 0; v5 < cd->height - 1; ++v5 )
    {
        int v57 = v5;
        int width = cd->width;
        for ( int v7 = 0; v7 < cd->width - 1; ++v7 )
        {
            verts = cd->verts;
            // ── cross 1: edges from vert[v9] toward +column and +row ──
            int   v9  = v7 + v5 * width;
            float vec_a[3], a2a[3], cr[3];
            vec_a[0] = verts[v9 + 1].xyz[0] - verts[v9].xyz[0];
            vec_a[1] = verts[v9 + 1].xyz[1] - verts[v9].xyz[1];
            vec_a[2] = verts[v9 + 1].xyz[2] - verts[v9].xyz[2];
            int v12  = v7 + width * ( v5 + 1 );
            a2a[0]   = verts[v12].xyz[0] - verts[v9].xyz[0];
            a2a[1]   = verts[v12].xyz[1] - verts[v9].xyz[1];
            a2a[2]   = verts[v12].xyz[2] - verts[v9].xyz[2];
            Vec3Cross( vec_a, a2a, cr );
            if ( Vec3Normalize_R( cr ) > 0.001f )                 // flt_6F43E0
            {
                normal += cr[0]; v53 += cr[1]; v54 += cr[2];
                float *c = cd->verts[v7 + v57 * cd->width].xyz;   // centroid uses v57 (== v5)
                ++v56;
                v49 += c[0]; v50 += c[1]; v51 += c[2];
            }
            // ── cross 2: edges from vert[v20] (next row) toward -column and the diagonal ──
            verts    = cd->verts;
            int v20  = v7 + cd->width * ( v5 + 1 );
            vec_a[0] = verts[v20].xyz[0] - verts[v20 + 1].xyz[0];
            vec_a[1] = verts[v20].xyz[1] - verts[v20 + 1].xyz[1];
            vec_a[2] = verts[v20].xyz[2] - verts[v20 + 1].xyz[2];
            int v23  = v57 * cd->width + v7 + 1;
            a2a[0]   = verts[v23].xyz[0] - verts[v20 + 1].xyz[0];
            a2a[1]   = verts[v23].xyz[1] - verts[v20 + 1].xyz[1];
            a2a[2]   = verts[v23].xyz[2] - verts[v20 + 1].xyz[2];
            Vec3Cross( vec_a, a2a, cr );
            if ( Vec3Normalize_R( cr ) > 0.001f )
            {
                normal += cr[0]; v53 += cr[1]; v54 += cr[2];
                float *c = cd->verts[( v5 + 1 ) * cd->width + 1 + v7].xyz;
                ++v56;
                v49 += c[0]; v50 += c[1]; v51 += c[2];
            }
        }
    }

    float nrm[3] = { normal, v53, v54 };
    if ( Vec3Normalize_R( nrm ) < 0.001f )                        // degenerate average normal
        return 0;
    normal = nrm[0]; v53 = nrm[1]; v54 = nrm[2];

    // plane dist = average of dot(centroid, N) over the samples.
    float v34 = (float)( ( v51 * v54 + v49 * normal + v50 * v53 ) / (double)v56 );

    int width  = patch->width;
    int height = patch->height;
    if ( width - 1 >= 0 && width != 1 )
    {
        // verify the (width-1)×(height-1) control-point sub-grid lies within ±1 of the plane.
        for ( int col = 0; col < width - 1; ++col )
        {
            for ( int row = 0; row < height - 1; ++row )
            {
                const float *p = patch->ctrl[col][row].xyz;
                float d = p[0] * normal + p[1] * v53 + p[2] * v54 - v34;
                if ( d > 1.0f || d < -1.0f )
                    return 0;
            }
        }
    }

    out[0] = normal; out[1] = v53; out[2] = v54;
    *(double *)( (char *)outBlk + 16 ) = (double)v34;   // a2[+0x10] is a QWORD store (double)
    return 1;
}

// 0x4380B0  Ed_Patch_PickSpan — pick the first of 8 candidate control-point index
// triples whose 3 points are non-collinear (cross-product length² > 0.001).  Each
// candidate is 3 (col,row) pairs spanning the patch corners/edges; the candidates are
// seeded from the grid extents W1=width-1, H1=height-1, W2=width-2, H2=height-2.
// outCols[3] / outRows[3] receive the winning triple (control index = col*16 + row).
// Returns 1 on success, 0 if all 8 candidates are degenerate.
// 8-candidate table transcribed from the 48 ebp-local seeds; cross len²
// band ±0.001 (flt_6F43E0).
static char Ed_Patch_PickSpan( patchMesh_t *patch, int *outCols, int *outRows )
{
    const int W1 = patch->width  - 1;
    const int H1 = patch->height - 1;
    const int W2 = patch->width  - 2;
    const int H2 = patch->height - 2;

    // 8 candidates, each { col[3], row[3] }.  Transcribed verbatim from the IDB seed block
    // (out1=cols come from arg1/edi, out2=rows from arg2/ebx; index = col*16 + row).
    const int colTbl[8][3] = {
        { W1, 0,  0  },   // 0
        { 0,  0,  W1 },   // 1
        { 0,  W1, W1 },   // 2
        { W1, W1, 0  },   // 3
        { 1,  0,  0  },   // 4
        { 0,  0,  1  },   // 5
        { W2, W1, W1 },   // 6
        { W1, W1, W2 },   // 7
    };
    const int rowTbl[8][3] = {
        { 0,  0,  H1 },   // 0
        { 0,  H1, H1 },   // 1
        { H1, H1, 0  },   // 2
        { H1, 0,  0  },   // 3
        { 0,  0,  1  },   // 4
        { H2, H1, H1 },   // 5
        { H1, H1, H2 },   // 6
        { 1,  0,  0  },   // 7
    };

    for ( int i = 0; i < 8; ++i )
    {
        outCols[0] = colTbl[i][0]; outRows[0] = rowTbl[i][0];
        outCols[1] = colTbl[i][1]; outRows[1] = rowTbl[i][1];
        outCols[2] = colTbl[i][2]; outRows[2] = rowTbl[i][2];

        // three control points (index = col*16 + row), edges e1 = p1-p0, e2 = p2-p0.
        const float *p0 = patch->ctrl[ outCols[0] ][ outRows[0] ].xyz;
        const float *p1 = patch->ctrl[ outCols[1] ][ outRows[1] ].xyz;
        const float *p2 = patch->ctrl[ outCols[2] ][ outRows[2] ].xyz;
        float e1[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
        float e2[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };
        float cr[3];
        Vec3Cross( e1, e2, cr );
        float lenSq = cr[0] * cr[0] + cr[1] * cr[1] + cr[2] * cr[2];
        if ( lenSq > 0.001f )            // flt_6F43E0 — non-collinear: this candidate wins
            return 1;
    }
    return 0;
}

// 0x44B620  Ed_Patch_GetTexdef — extract a planar texdef (size/shift/rotate) from a
// patch's control-grid texCoords.  The patch analog of texturevecs_02; the middle-button
// patch texture-PICK (grab a patch's texture into the current texdef).  Side-effects
// `texdef` (size[2]@0, shift[2]@8, rotate@0x10) when ALL the patch's control points
// reproduce within the per-texel tolerance.
//
// IMPORTANT (faithful binary fact): this function ALWAYS returns 0 — there is no
// `mov al,1` anywhere in 0x44B620; every exit (including the success tail) is `xor al,al`.
// The texdef WRITE is the observable; the consumer Drag_Begin discards the return value.
//
// Chain: Ed_Patch_PlanarTest -> Ed_Patch_PickSpan -> Ed_Normal_Calc (basis) -> project 3
// control pts to (u,v)+(s,t) -> Ed_Patch_AffineInverse -> recover the 2x3 affine (u,v)->(s,t)
// -> degenerate guard -> texel size from MaterialDef_GetLayeredMaterial (512 fallback) ->
// per-control-point verification loop -> size=1/|coeff|, shift=const term, rotate=atan2*180/pi
// with a floorf(x+0.5) integer-degree roundtrip check.  FP constants read from the IDB.
static char Ed_Patch_GetTexdef( patchMesh_t *patch, texdef_sub_t *texdef )
{
    // ── Phase A: planar test (writes plane normal N[3] @ planeBlk[0..2]) ──
    // planeBlk doubles as the 2x3 affine-matrix store after the multiply below (the binary
    // reuses the same stack slot for N and for the recovered rows).
    double planeBlk[8];                         // [0..2]=N (as floats via the cast), then reused
    if ( !Ed_Patch_PlanarTest( patch, planeBlk ) )
        return 0;

    // ── Phase B: pick 3 non-collinear control points spanning the patch ──
    int cols[3], rows[3];
    if ( !Ed_Patch_PickSpan( patch, cols, rows ) )
        return 0;

    // ── Phase C: build the tex-plane basis (xv,yv) from N (TextureAxisFromPlane) ──
    float xv[3], yv[3];
    Ed_Normal_Calc( (const float *)planeBlk, xv, yv );   // N is the first 3 floats of planeBlk

    const int layer = g_qeglobals.current_edit_layer;

    // ── Phase D: project the 3 chosen control points to (u,v) tex-plane coords + (s,t). ──
    int   idx0 = cols[0] * 16 + rows[0];
    int   idx1 = cols[1] * 16 + rows[1];
    int   idx2 = cols[2] * 16 + rows[2];
    const float *xyz0 = patch->ctrl[0][idx0].xyz;
    const float *xyz1 = patch->ctrl[0][idx1].xyz;
    const float *xyz2 = patch->ctrl[0][idx2].xyz;

    float u[3], v[3];                 // u = xyz·xv, v = xyz·yv  (var_6C/68/64, var_78/74/70)
    u[0] = xyz0[0]*xv[0] + xyz0[1]*xv[1] + xyz0[2]*xv[2];
    v[0] = xyz0[0]*yv[0] + xyz0[1]*yv[1] + xyz0[2]*yv[2];
    u[1] = xyz1[0]*xv[0] + xyz1[1]*xv[1] + xyz1[2]*xv[2];
    v[1] = xyz1[0]*yv[0] + xyz1[1]*yv[1] + xyz1[2]*yv[2];
    u[2] = xyz2[0]*xv[0] + xyz2[1]*xv[1] + xyz2[2]*xv[2];
    v[2] = xyz2[0]*yv[0] + xyz2[1]*yv[1] + xyz2[2]*yv[2];

    // st coords: ctrl[idx].texCoord.st[2*layer + {0,1}].  The IDB reads these at byte
    // offset 0x44/0x48 + (layer + 10*idx)*8 from the patch base — i.e. st[2*layer] within
    // the idx-th control element (texCoord @ +0x0C, st @ +0).
    auto ctrlST = [&]( int idx, int comp ) -> float {
        return patch->ctrl[0][idx].texCoord.st[2 * layer + comp];
    };
    float s0 = ctrlST( idx0, 0 ), t0 = ctrlST( idx0, 1 );
    float s1 = ctrlST( idx1, 0 ), t1 = ctrlST( idx1, 1 );
    float s2 = ctrlST( idx2, 0 ), t2 = ctrlST( idx2, 1 );

    // ── Phase E: invert M = [[u_i, v_i, 1]] (Cramer) ──
    double inv[9];
    if ( !Ed_Patch_AffineInverse( u, v, inv ) )
        return 0;

    // ── Phase F: recover the 2x3 affine (u,v)->(s,t): rowS·(u,v,1)=s, rowT·(u,v,1)=t.
    //    rowS = {a_su, a_sv, a_sc}, rowT = {a_tu, a_tv, a_tc}.  (Regular double arithmetic —
    //    transcribed from the 44b7fe..44b8e5 block; hex-rays decoded it cleanly.) ──
    double rowS_u = (double)s0*inv[0] + (double)s1*inv[1] + (double)s2*inv[2];   // normal@0x54
    double rowS_v = (double)s2*inv[5] + (double)s1*inv[4] + (double)s0*inv[3];   // var_A0@0x5c
    double rowS_c = (double)s0*inv[6] + (double)s1*inv[7] + (double)s2*inv[8];   // var_98@0x64
    double rowT_u = inv[0]*(double)t0 + inv[1]*(double)t1 + inv[2]*(double)t2;   // var_90@0x6c
    double rowT_v = inv[5]*(double)t2 + inv[4]*(double)t1 + (double)t0*inv[3];   // var_88@0x74
    double rowT_c = inv[6]*(double)t0 + inv[7]*(double)t1 + (double)t2*inv[8];   // var_80@0x7c

    // ── Phase G: degenerate guard — each row's (u,v) coefficients must not both be 0. ──
    if ( 0.0 == rowS_u && 0.0 == rowS_v ) return 0;
    if ( 0.0 == rowT_u && 0.0 == rowT_v ) return 0;

    // ── Phase H: texel size from the layered material (512 fallback). ──
    int texelW, texelH;
    qtexture_s *lm = MaterialDef_GetLayeredMaterial(
        (MaterialDef *)( (char *)&patch->texture + (size_t)layer * sizeof( patchMesh_material ) ) );
    if ( lm ) { texelW = lm->width; texelH = lm->height; }
    else      { texelW = 512;       texelH = 512; }

    // ── Phase I: verification loop — every control point must reproduce its st within tol. ──
    //    s_rec = rowS_u*u_pt + rowS_v*v_pt + rowS_c ;  |s_rec - s|*texelW must be <= 0.001.
    //    t_rec = rowT_u*u_pt + rowT_v*v_pt + rowT_c ;  |t_rec - t|*texelH must be <= 0.001.
    for ( int col = 0; col < patch->width; ++col )
    {
        for ( int row = 0; row < patch->height; ++row )
        {
            const float *xyz = patch->ctrl[col][row].xyz;
            double u_pt = (double)xyz[0]*xv[0] + (double)xyz[1]*xv[1] + (double)xyz[2]*xv[2];
            double v_pt = (double)xyz[0]*yv[0] + (double)xyz[1]*yv[1] + (double)xyz[2]*yv[2];
            float  s_act = patch->ctrl[col][row].texCoord.st[2 * layer + 0];
            float  t_act = patch->ctrl[col][row].texCoord.st[2 * layer + 1];
            double errS = rowS_u*u_pt + rowS_v*v_pt + rowS_c - (double)s_act;
            if ( errS < 0.0 ) errS = -errS;
            if ( errS * (double)texelW > 0.001000000047497451 )    // dbl_6F40A0
                return 0;
            double errT = rowT_u*u_pt + rowT_v*v_pt + rowT_c - (double)t_act;
            if ( errT < 0.0 ) errT = -errT;
            if ( errT * (double)texelH > 0.001000000047497451 )
                return 0;
        }
    }

    // ── Phase J: size[i] = 1/|(coeff_u,coeff_v)|, shift[i] = const term, for rows S(0) and T(1). ──
    {
        double rows2[2][3] = { { rowS_u, rowS_v, rowS_c }, { rowT_u, rowT_v, rowT_c } };
        for ( int i = 0; i < 2; ++i )
        {
            texdef->shift[i] = (float)rows2[i][2];                              // const term -> shift
            double len = sqrt( rows2[i][0]*rows2[i][0] + rows2[i][1]*rows2[i][1] );
            if ( 0.0 == len )
                return 0;
            texdef->size[i] = (float)( 1.0 / len );
            // (the binary also normalizes the in-place (u,v) coeffs to (cos,sin); only the
            //  T row's normalized direction is used below, so re-normalize there directly.)
        }
    }

    // ── Phase K: rotate = atan2(rowT_v_norm, rowT_u_norm) * 180/pi, with a floorf(x+0.5)
    //    integer-degree roundtrip check (|round - raw| < 0.001 else fail). ──
    {
        // The size phase normalized rowT's (u,v) by lenT; the binary then calls
        // atan2(st1=rowT_u_norm, st0=rowT_v_norm) — i.e. CRT atan2(y=rowT_u, x=rowT_v).
        // Since lenT>0 the normalization cancels: atan2(rowT_u/lenT, rowT_v/lenT) == atan2(rowT_u, rowT_v).
        float  rotRaw = (float)( RAD2DEG( atan2( rowT_u, rowT_v ) ) );  // dbl_6F4578
        texdef->rotate = rotRaw;                                                // raw store (fst)
        float  rounded = floorf( rotRaw + 0.5f );                               // dbl_6F4160 = 0.5
        float  diff = rounded - rotRaw;
        if ( diff < 0.0f ) diff = -diff;
        if ( diff >= 0.001f )                                                   // flt_6F43E0
            return 0;
        texdef->rotate = rounded;
    }
    return 0;   // faithful: 0x44B620 has no success path (every exit is `xor al,al`)
}

// Drag_Begin (drag.cpp) middle-button PATCH texture-pick entry point — the real
// 0x44B620.  (Replaces the former FATAL stub.)  Return value is intentionally
// discarded by the caller; the texdef write is the observable.
char Radiant_PatchGetTexdef( patchMesh_t *patch, texdef_sub_t *texdef )
{
    return Ed_Patch_GetTexdef( patch, texdef );
}

// 0x45D360  TexWnd_06_LayerCount — find the texdef block whose vis key matches
// `layerHandle` within the MaterialDef. Returns the texdef base ptr (as int), or 0.
int TexWnd_06_LayerCount( int mtlDef, int layerHandle )
{
    int *a1 = (int *)mtlDef;                  // &MaterialDef (dword array)
    int  layerCount = MaterialDef_04( (MaterialDef *)a1 );
    iassert( layerCount );   // TexWnd.cpp:1646
    if ( layerCount <= 0 )
        return 0;
    int  v4 = 0;
    int *i  = a1 + 8;                          // first key at dword index 8 (= mat_texDef+24)
    while ( *i != layerHandle )
    {
        if ( ++v4 >= layerCount )
            return 0;
        i += 7;                                // 28-byte stride
    }
    return (int)&a1[7 * v4 + 2];               // &mat_texDef of the matched block
}

// 0x51AD50  Material "colorTint" constant lookup — fill out[4], return 1 if found.
// constantCount@+0x5B is a BYTE; stride 0x20; literal@+0x10.
static char Ed_Material_GetColorTint( Material *mtl, const char *name, float *out )
{
    unsigned int hash = R_HashString( name );
    if ( !mtl->constantCount )
        return 0;
    int v4 = 0;
    MaterialConstantDef *c = mtl->constantTable;
    while ( c->nameHash != hash )
    {
        if ( (unsigned)++v4 >= (unsigned)(unsigned char)mtl->constantCount )
            return 0;
        ++c;
    }
    const float *lit = mtl->constantTable[v4].literal;
    out[0] = lit[0]; out[1] = lit[1]; out[2] = lit[2]; out[3] = lit[3];
    return 1;
}

// 0x46F410  pack float[4] colour → 4 RGBA bytes (a2[2]=R, a2[1]=G, a2[0]=B, a2[3]=A).
// ROUNDS (binary inline fistp), does not truncate.  Covers Ed_PackColor
// (BGRA store order).
static int Ed_ClampColorByte( float f )
{
    int v = SnapFloatToInt( f * 255.0f + 9.313225746154785e-10f );   // IDA 0x46f430: inline fistp ROUNDS (NOT _ftol2 truncate); 2^-30 bias
    if ( v >= 255 ) return 255;
    if ( v <= 0 )   return 0;
    return v;
}
static void Ed_PackColor( const float *a1, unsigned char *a2 )
{
    a2[2] = (unsigned char)Ed_ClampColorByte( a1[0] );
    a2[1] = (unsigned char)Ed_ClampColorByte( a1[1] );
    a2[0] = (unsigned char)Ed_ClampColorByte( a1[2] );
    a2[3] = (unsigned char)Ed_ClampColorByte( a1[3] );
}

// 0x46F6C0  per-layer face colour: colorTint·entityColor if the material defines a
// "colorTint" constant, else the per-face packed colour (face_t+228, default white).
void sub_46F6C0( int mtlDef, int faceDef, int visIdx, int *outData )
{
    MaterialDef   *a1 = (MaterialDef *)mtlDef;
    unsigned char *a2 = (unsigned char *)faceDef;
    Material      *mtl = MaterialDef_14( visIdx, a1 );
    iassert( mtl );   // MaterialDef.cpp:336
    float tint[4];
    if ( Ed_Material_GetColorTint( mtl, "colorTint", tint ) )
    {
        const double k = 0.003921568859368563;   // 1/255
        float v9[4];
        v9[0] = tint[0] * (float)( (double)a2[230] * k );
        v9[1] = tint[1] * (float)( (double)a2[229] * k );
        v9[2] = tint[2] * (float)( (double)a2[228] * k );
        v9[3] = tint[3] * (float)( k * (double)a2[231] );
        Ed_PackColor( v9, (unsigned char *)outData );
    }
    else
    {
        *outData = ( (face_t *)a2 )->packedColor;
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  TEXTURE FIT  (Surface Inspector "Fit" button → Brush_FitTexture).
//
//  Texture_Fit (0x47C590) — fit one face's texture so the texture spans the face
//  `a3` times horizontally and `a2` times vertically (both clamped >= 1).  The
//  math is GtkRadiant's Face_FitTexture lineage, transcribed from the IDB including its
//  float ordering:
//    1. Bounding box of the face winding points (a4==0; the a4!=0 whole-selection
//       branch via sub_48FB70 is a Surface-Inspector mode OnFit does not use —
//       FATAL tripwire below).
//    2. Ed_Normal_Calc picks the face's base S/T axes (xv,yv) from its plane normal.
//    3. Project the 8 box corners onto (xv,yv) — only the 4 (min/max u)×(min/max v)
//       combinations matter — rotate each by the texdef's stored `rotate`, and take
//       the rotated u/v extents.
//    4. size0 = -uExtent/a3, size1 = -vExtent/a2 (texels-per-repeat, the editor's
//       inverted-scale convention); shift snapped into [0,extent) via floorf.
//  The texdef written is &md->mat_texDef + GetCurrentLayer(md) where md is the
//  layer's MaterialDef (face+36+36*current_edit_layer) — the SAME slot the Surface
//  Inspector edits (Surf_ApplyTexdefRaw) and the camera reads (Face_MoveTexture),
//  so a Fit re-projects live + round-trips through the .map writer.
// ════════════════════════════════════════════════════════════════════════════

// 0x48FB70 — fill [mins,maxs] from the whole current selection (Texture_Fit a4!=0
// "fit across selection" mode).  OnFit / Brush_FitTexture always pass a4==0, so this
// is a tripwire, not a silent no-op.  (Distinct from Select_GetBounds 0x48FB10.)
extern void sub_48FB70( int minsOut, int maxsOut );

// 0x47C590
void Texture_Fit( int a1, float a2, float a3, int a4 )
{
    face_t *f = (face_t *)(intptr_t)a1;

    if ( a2 < 1.0f ) a2 = 1.0f;
    if ( a3 < 1.0f ) a3 = 1.0f;

    // Bounding box of the face's winding points (IDB ±131072 = 2^17 init sentinel).
    float mins[3] = {  131072.0f,  131072.0f,  131072.0f };
    float maxs[3] = { -131072.0f, -131072.0f, -131072.0f };
    if ( a4 )
    {
        sub_48FB70( (int)(intptr_t)mins, (int)(intptr_t)maxs );
    }
    else
    {
        winding_t *w = f->w;
        if ( !w )
            return;
        for ( int i = 0; i < w->numpoints; ++i )
            edwind::VectorMaxValues( w->p[i], mins, maxs );   // defined in namespace edwind
    }

    // The layer MaterialDef + its current sub-layer texdef block (the edited slot).
    MaterialDef  *md = &f->mtldef[g_qeglobals.current_edit_layer];
    int           curLayer = LayerMat::GetCurrentLayer( md );
    float        *v10 = &md->mat_texDef.size[7 * curLayer];   // [0]=size0 [1]=size1 [2]=shift0 [3]=shift1
    float         rotate = md->mat_texDef.size[7 * curLayer + 4];  // block CL's rotate

    double rad  = DEG2RAD( (double)rotate );       // dbl_6F4298 (π/180)
    float  cosv = (float)cos( rad );
    float  sinv = (float)sin( rad );

    // Base S/T axes for the face plane (xv,yv vec3).
    float xv[3], yv[3];
    Ed_Normal_Calc( f->plane.normal, xv, yv );

    // The four candidate (u,v) corners.  Dot products transcribed in the IDB's
    // component order (xv·mins=v37, yv·mins=v38, xv·maxs=anon0, yv·maxs=anon1).
    float dotXvMins = xv[0]*mins[0] + xv[1]*mins[1] + xv[2]*mins[2];   // v37
    float dotYvMins = mins[0]*yv[0] + mins[1]*yv[1] + yv[2]*mins[2];   // v38
    float dotXvMaxs = xv[2]*maxs[2] + xv[0]*maxs[0] + xv[1]*maxs[1];   // anon0
    float dotYvMaxs = maxs[0]*yv[0] + yv[1]*maxs[1] + maxs[2]*yv[2];   // anon1

    float c0u = dotXvMins, c0v = dotYvMins;
    float c1u = dotXvMaxs, c1v = dotYvMins;
    float c2u = dotXvMins, c2v = dotYvMaxs;
    float c3u = dotXvMaxs, c3v = dotYvMaxs;

    // Rotate each corner and track the u/v extents.  Init exactly as the IDB:
    //   minU,minV start at +131072 ; maxU,maxV start at -131072.
    float minU =  131072.0f, minV =  131072.0f;
    float maxU = -131072.0f, maxV = -131072.0f;

    // C0
    float r0u = c0u*cosv - c0v*sinv;
    float r0v = c0u*sinv + c0v*cosv;
    if ( r0u < 131072.0f ) minU = r0u;
    if ( r0v < 131072.0f ) minV = r0v;
    // C1
    float r1u = c1u*cosv - c1v*sinv;
    float r1v = c1u*sinv + c1v*cosv;
    if ( r1u > -131072.0f ) maxU = r1u;
    if ( minV > r1v ) minV = r1v;
    // C2
    float r2u = c2u*cosv - c2v*sinv;
    float r2v = c2u*sinv + c2v*cosv;
    if ( minU > r2u ) minU = r2u;
    if ( r2v > -131072.0f ) maxV = r2v;
    // C3
    float r3u = c3u*cosv - c3v*sinv;
    float r3v = c3u*sinv + c3v*cosv;
    if ( maxU < r3u ) maxU = r3u;
    if ( maxV < r3v ) maxV = r3v;

    float uExtent = maxU - minU;
    float vExtent = maxV - minV;

    if ( uExtent == 0.0f || vExtent == 0.0f )
    {
        Sys_Printf( "Cannot fit texture - scale would be 0\n" );
        return;
    }

    v10[0] = -uExtent / a3;                  // size0
    v10[1] = -vExtent / a2;                  // size1
    float scaleW = v10[0] * a3;              // = -uExtent
    v10[2] = minU - floorf( minU / scaleW ) * scaleW;   // shift0
    float scaleH = v10[1] * a2;              // = -vExtent
    v10[3] = minV - floorf( minV / scaleH ) * scaleH;   // shift1
}

// 0x447600 — Patch_FitTexturing: fit one texture tile across a PATCH (st = normalized
// grid position).  Reached from sub_47C950 when def->patch != 0 (the Surface Inspector
// "Fit" path on a selected patch).  REAL port in pmesh.cpp.
extern void Patch_FitTexturing( patchMesh_t *patch );

// 0x47C950 — fit every face of a brush def (Brush_FitTexture's brush-pass).  Patch
// brushes route to Patch_FitTexturing; solid brushes Texture_Fit each face (a4==0).
// patch -> Patch_FitTexturing, else the per-face Texture_Fit loop.
void sub_47C950( int a1, float a2, float a3 )
{
    brush_t *def = (brush_t *)(intptr_t)a1;
    if ( def->patch )
    {
        Patch_FitTexturing( def->patch );
        return;
    }
    for ( int i = 0; i < def->faceCount; ++i )
        Texture_Fit( (int)(intptr_t)&def->faces[i], a2, a3, 0 );
}

// ════════════════════════════════════════════════════════════════════════════
//  SUN-LIGHT PREVIEW (directional sunlight in the camera).
//
//  The faithful chain is R_SunPrev_Main (0x4069C0): R_SunPrev_SetSunConstants sets the
//  SUN_POSITION/DIFFUSE/SPECULAR shader constants from the worldspawn sun keys, two
//  full-screen quads black the world + clear alpha/stencil, SunLightPreview_BrushShadow
//  (0x47B310 -> 0x47B2A0 -> sub_479610/479BF0/47B190 + MaterialDef_16) extrudes stencil
//  shadow volumes, then the world is re-drawn SUN-LIT via R_AddEditorSurfsCmd.
//
//  KISAK (not in the binary): g_edSun is an APPROXIMATION that bakes the Lambert term
//  (ambient + diffuse*N.L)*suncolor into the per-vertex colour in Face_BuildLayerGeom,
//  from the worldspawn sun keys read by SunPrev_Setup.  No stencil shadows.  It is only
//  active when Cam_Draw selects it (g_edSun.active), never on the faithful path.
// ════════════════════════════════════════════════════════════════════════════
extern int   Entity_GetVec3ForKey( entity_s_def *e, float *out, const char *key );
extern float Entity_GetFloatValueForKey( int e, const char *key );

struct EdSunState { int active; float dir[3]; float color[3]; float ambient; float diffuse; };
EdSunState g_edSun = { 0, { 0, 0, -1 }, { 1, 1, 1 }, 0.15f, 0.85f };

// Set once per camera frame by Cam_Draw (camwnd.cpp): 1 only when the kisak APPROXIMATION
// sun preview is the active path, 0 on the faithful R_SunPrev_Main path (and 0 with the
// preview off).  Gates the per-vertex sun bake in Face_BuildLayerGeom — see the comment
// there; on the faithful path the sun must come ONLY from the tech-26 additive re-add.
int g_edSunBakeVertColor = 0;

void SunPrev_Setup()
{
    g_edSun.active = 0;
    if ( !world_entity )
        return;
    entity_s_def *wd = (entity_s_def *)world_entity->def;
    if ( !wd )
        return;
    float ang[3] = { 0, 0, 0 };
    if ( !Entity_GetVec3ForKey( wd, ang, "sundirection" ) )
        return;                                         // no sun key → preview off
    float fwd[3], right[3], up[3];
    AngleVectors( ang, fwd, right, up );                // sun travel direction
    g_edSun.dir[0] = fwd[0]; g_edSun.dir[1] = fwd[1]; g_edSun.dir[2] = fwd[2];
    float col[3] = { 1, 1, 1 };
    Entity_GetVec3ForKey( wd, col, "suncolor" );
    g_edSun.color[0] = col[0]; g_edSun.color[1] = col[1]; g_edSun.color[2] = col[2];
    float sunlight = Entity_GetFloatValueForKey( (int)(intptr_t)wd, "sunlight" );
    if ( sunlight <= 0.0f ) sunlight = 1.0f;
    float diffuseFrac = Entity_GetFloatValueForKey( (int)(intptr_t)wd, "diffusefraction" );
    if ( diffuseFrac <= 0.0f ) diffuseFrac = 0.5f;
    float ambient = Entity_GetFloatValueForKey( (int)(intptr_t)wd, "ambient" );
    if ( ambient < 0.05f ) ambient = 0.15f;
    g_edSun.ambient = ambient;
    g_edSun.diffuse = sunlight * diffuseFrac;
    g_edSun.active  = 1;
}

// Modulate a packed face colour (Ed_PackColor: byte0=B, byte1=G, byte2=R, byte3=A)
// by the directional-sun Lambert term against the world normal `wn`.
static unsigned int SunPrev_ShadeColor( unsigned int packed, const float *wn )
{
    float ndl = -( wn[0]*g_edSun.dir[0] + wn[1]*g_edSun.dir[1] + wn[2]*g_edSun.dir[2] );
    if ( ndl < 0.0f ) ndl = 0.0f;
    float lit = g_edSun.ambient + g_edSun.diffuse * ndl;
    if ( lit > 1.5f ) lit = 1.5f;
    unsigned char *c = (unsigned char *)&packed;
    float b = (float)c[0] * ( g_edSun.color[2] * lit );   // B
    float gg = (float)c[1] * ( g_edSun.color[1] * lit );  // G
    float r = (float)c[2] * ( g_edSun.color[0] * lit );   // R
    int bi = (int)b; if ( bi > 255 ) bi = 255; if ( bi < 0 ) bi = 0;
    int gi = (int)gg; if ( gi > 255 ) gi = 255; if ( gi < 0 ) gi = 0;
    int ri = (int)r; if ( ri > 255 ) ri = 255; if ( ri < 0 ) ri = 0;
    c[0] = (unsigned char)bi; c[1] = (unsigned char)gi; c[2] = (unsigned char)ri;
    return packed;
}

int SunPrev_Active() { return g_edSun.active; }

// The editor brush technique (UNLIT) tints from CONST_SRC_CODE_MATERIAL_COLOR (a
// per-draw uniform), NOT per-vertex colour — so the visible directional-sun term is
// applied per face via R_AddCmdSetMaterialColor in the camera draw. Returns the
// face's sun colour {r,g,b,1} = suncolor·(ambient + diffuse·N·L).
void SunPrev_FaceShade( const float *wn, float *out4 )
{
    float ndl = -( wn[0]*g_edSun.dir[0] + wn[1]*g_edSun.dir[1] + wn[2]*g_edSun.dir[2] );
    if ( ndl < 0.0f ) ndl = 0.0f;
    float lit = g_edSun.ambient + g_edSun.diffuse * ndl;
    out4[0] = g_edSun.color[0] * lit;
    out4[1] = g_edSun.color[1] * lit;
    out4[2] = g_edSun.color[2] * lit;
    out4[3] = 1.0f;
}

// Face_BuildLayerGeom — the faithful Visuals_InitFaceVis (0x46F7A0) inner body for
// ONE material layer. Shared by the surf-cache draw and the faithful immediate
// draw so they emit bit-identical geometry. Returns false when the face has no
// winding or the layer texdef is missing (defensive; emit nothing).
// 0x46F7A0 (inner body) — Editor_VB_Upload arg order is (pos,tangent,binormal,normal,
// st,color).  KISAK: EdLayerGeom caps the winding at 64 verts vs the IDB's 1024 (safe:
// brush faces are convex).
bool Face_BuildLayerGeom( face_t *faceDef, const orientation_t *orient,
                          int layer, EdLayerGeom *g )
{
    MaterialDef *mtldef = &faceDef->mtldef[g_qeglobals.current_edit_layer];
    winding_t   *w      = faceDef->w;
    if ( !w )
        return false;
    int nv = w->numpoints;
    if ( nv < 1 || nv > 64 )
        return false;
    g->vertcount = nv;

    // world-space face normal (computed once per face; same for every layer).
    float wn[3];
    OrientationDirToWorldDir( wn, orient, faceDef->plane.normal );

    int   visVal = MaterialDef_13( layer, mtldef );
    int   texdef = TexWnd_06_LayerCount( (int)mtldef, visVal );
    if ( !texdef )
        return false;
    float texMat[8];
    texdef_sub_t *td = (texdef_sub_t *)(intptr_t)texdef;
    Face_MoveTexture( texdef, faceDef->plane.normal, (int)texMat, texdef + 8,
                      td->rotate, td->crossterm );

    float tang[3];   sub_4A47D0( (int)tang, (int)texMat );  // tangent = normalize(row0)
    float binorm[3]; Vec3Cross( wn, tang, binorm );          // binormal = N × T

    unsigned int col = 0;
    sub_46F6C0( (int)mtldef, (int)faceDef, layer, (int *)&col );
    // Stage-2 APPROXIMATION ONLY: bake the directional sun into the per-vertex colour.
    // The binary's Visuals_InitFaceVis (0x46F7A0) has NO such term — it is kisak's
    // fallback for the sun preview when the faithful R_SunPrev_Main path is unavailable.
    // [FIX 2026-07-25] The gate used to be `g_edSun.active` alone, and g_edSun.active is
    // set by SunPrev_Setup() for ANY map with a "sundirection" key as soon as the sun
    // preview pref is on (it used to require the RADIANT_SUNPREV env, which is why the
    // stale comment above calls it a dead no-op).  So on the FAITHFUL path every cached
    // face was ALSO being multiplied by suncolour·(ambient + diffuse·N·L) at build time —
    // a face pointing away from the sun baked down to `ambient` (0.15 on mp_backlot)
    // BEFORE the multiply quad and the additive re-add ran.  Additive blending cannot
    // undo that, so those facades stayed near-black no matter how well the re-add covered
    // them (this was the operator's "many whole building facades render very dark").
    // g_edSunBakeVertColor is set per frame by Cam_Draw: 1 only on the approximation path.
    if ( g_edSun.active && g_edSunBakeVertColor )
        col = SunPrev_ShadeColor( col, wn );

    for ( int i = 0; i < nv; ++i )
    {
        const float *p = w->p[i];
        OrientationPosToWorldPos( g->xyz[i], p, orient );
        g->normal[i][0]   = wn[0];     g->normal[i][1]   = wn[1];     g->normal[i][2]   = wn[2];
        g->tangent[i][0]  = tang[0];   g->tangent[i][1]  = tang[1];   g->tangent[i][2]  = tang[2];
        g->binormal[i][0] = binorm[0]; g->binormal[i][1] = binorm[1]; g->binormal[i][2] = binorm[2];
        g->st[i][0] = texMat[0]*p[0] + texMat[1]*p[1] + texMat[2]*p[2] + texMat[3];
        g->st[i][1] = texMat[4]*p[0] + texMat[5]*p[1] + texMat[6]*p[2] + texMat[7];
        g->color[i] = col;
    }
    g->material = MaterialDef_14( layer, mtldef );
    return true;
}

// ════════════════════════════════════════════════════════════════════════════
//  Brush_RealizeFaceMaterials — KISAK, no IDA counterpart (the stock editor never makes
//  brush-face materials degenerate).
//
//  Prefab-content brush faces parse with the DEGENERATE SetMaterial shim (lyrMtl =
//  name@0, radMtl = NULL, layerCount 0) because the port forces that path while
//  g_radiantLoadingPrefab != 0 (materialdef.cpp), to dodge the CoD4-water material-asset
//  AV in the CoD3-sized kisak engine.  A degenerate mtldef makes Visuals_InitFaceVis'
//  MaterialDef_11 return 0, so the face builds NO visuals and the wall renders as a
//  white void.
//
//  MUST run at LOAD time (Eclass_RealizeModel's post-parse brush walk), never inside a
//  draw: realizing 100+ wall materials mid-frame stalls the first frame hard enough that
//  the MFC window never pumps a paint.  Requires the renderer to be up
//  (g_radiantFirstLightRendererReady, set in CMainFrame::OnCreate before the map load).
//  Materialdef_Realize is idempotent, and only mtldef[current_edit_layer] — exactly the
//  layer Visuals_InitFaceVis reads — is realized.
// ════════════════════════════════════════════════════════════════════════════
void Brush_RealizeFaceMaterials( brush_t *def )
{
    if ( !def )
        return;
    extern bool Materialdef_Realize( MaterialDef *md );   // materialdef.cpp
    for ( int i = 0; i < def->faceCount; ++i )
        Materialdef_Realize( &def->faces[i].mtldef[g_qeglobals.current_edit_layer] );
}

// ════════════════════════════════════════════════════════════════════════════
//  0x46F7A0  Visuals_InitFaceVis — build the per-face, per-layer surface cache.
//  Allocates face->visArray[layerCount] = {Material*, vbHandle} and uploads each
//  layer's GfxWorldVertex run to the editor VB pool. Faithful to the IDB; uses the
//  shared Face_BuildLayerGeom for the per-vertex math. Reached via the brush
//  faceVis lifecycle (Brush_BuildFaceVis 0x477C50). Returns the layer count.
// ════════════════════════════════════════════════════════════════════════════
// 0x46F7A0 (outer body) — line-161 verbose Assert KEEP_VERBOSE (shared-winding ptCount;
// hoisted out of the loop, but loop-invariant, so it fires identically).
int Visuals_InitFaceVis( faceVis_s *face, face_t *def, const orientation_t *orient )
{
    iassert( face );                     // brush.cpp:130
    iassert( face->visCount == 0 );      // brush.cpp:131
    iassert( face->visArray == NULL );   // brush.cpp:132
    iassert( def );                      // brush.cpp:133

    winding_t *w = def->w;
    if ( !w )
        return 0;

    MaterialDef *mtldef = &def->mtldef[g_qeglobals.current_edit_layer];
    int layerCount = MaterialDef_11( mtldef );
    if ( !layerCount )
        return 0;

    face->visCount = layerCount;
    face->visArray = (faceVisuals_s *)operator new( (size_t)( 8 * layerCount ) );
    if ( !face->visArray )
        Error( "Out of memory allocating face visuals" );
    face->vertcount = w->numpoints;

    iassert( w->ptCount <= MAX_POINTS_ON_WINDING );   // brush.cpp:161

    for ( int L = 0; L < layerCount; ++L )
    {
        EdLayerGeom g;
        if ( !Face_BuildLayerGeom( def, orient, L, &g ) || !g.material )
        {
            face->visArray[L].mtlHandle  = nullptr;
            face->visArray[L].vertHandle = 0;
            continue;
        }
        face->visArray[L].mtlHandle  = g.material;
        face->visArray[L].vertHandle = (int)Editor_VB_Upload( g.material, g.vertcount,
            (const float *)g.xyz, (const float *)g.tangent, (const float *)g.binormal,
            (const float *)g.normal, (const float *)g.st, (const float *)g.color );
    }
    return layerCount;
}

// ════════════════════════════════════════════════════════════════════════════
//  6. Brush_SetFaceTexdefSize  (0x476740)  —  really Face_SetMaterial.
//  CORRECTED (find/replace session, 2026-06-13): the IDB writes the {lyrMtl, radMtl}
//  POINTER PAIR into face->mtldef[layer], NOT size[0..1].  Disasm 0x4767bb-c7:
//      mov edx,[ebx]            ; arg0[0]  (lyrMtl)
//      lea eax,[eax+eax*8+9]    ; float idx = 9*layer + 9  (= byte 36+36*layer = mtldef[layer])
//      mov [edi+eax*4], edx     ; face[idx]   = mtldef[layer].lyrMtl
//      mov ecx,[ebx+4]          ; arg0[1]  (radMtl)
//      mov [edi+eax*4+4], ecx   ; face[idx+1] = mtldef[layer].radMtl
//      add word ptr [esi+4Eh],1 ; ++b->version
//  The previous port wrote mat_texDef.size[0]/[1] (bytes 44/48), a latent bug: only
//  caller was csg.cpp Brush_AutoCaulkFace (`SetMaterial("caulk",&mat); sub_476740(&mat,…)`),
//  which passes a patchMesh_material {lyrMtl,radMtl} pair — never run in any gate, so the
//  mis-store stayed dormant.  arg `size2` is that {lyrMtl,radMtl} pair reinterpreted as
//  floats (the bit pattern is copied verbatim — pointers are 4 bytes on x86).
// ════════════════════════════════════════════════════════════════════════════
void Brush_SetFaceTexdefSize( const float *size2, face_t *f, brush_t *b )
{
    iassert( b );   // brush.cpp:2872
    iassert( f );   // brush.cpp:2873
    iassert( f >= &b->faces[0] && f < &b->faces[b->faceCount] );   // brush.cpp:2874

    MaterialDef *md = &f->mtldef[ g_qeglobals.current_edit_layer ];
    md->lyrMtl = *(LayerMaterialDef * const *)&size2[0];   // arg0[0]
    md->radMtl = *(qtexture_s * const *)&size2[1];         // arg0[1]
    ++b->version;
}

// ════════════════════════════════════════════════════════════════════════════
//  6b. sub_4766F0 (0x4766f0) — Brush_SetAllFaceTexdef: copy the current-layer
//  MaterialDef from the picked SOURCE face into EVERY face of the brush def, then
//  bump the def's version.  Reached from Drag_Begin's middle/right "apply face
//  texture but leave info" branch when the hit brush has a patch (def->patch != 0):
//  the caller already wrote random_texture_stuff[layer] into the picked face's
//  mtldef[layer], then passes that same face address here so the new MaterialDef
//  is propagated to ALL faces (keeping the patch + faces consistent).
//  IDA (0x4766f0): a1=def@edx, a2=srcFace.  for (v2=0; v2 < def->faceCount; ++v2)
//      qmemcpy(&def->faces[v2].mtldef[layer], &srcFace->mtldef[layer], 0x24);
//    ++def->version;
//  def->faceCount @ +0x40 (IDB "a1+64"), def->faces @ +0x44 ("a1+68"),
//  def->version @ +0x4E ("a1+78"); srcFace base + 36 + 36*layer == srcFace->mtldef[layer].
// ════════════════════════════════════════════════════════════════════════════
void sub_4766F0( brush_t *def, const face_t *srcFace )
{
    const int layer = g_qeglobals.current_edit_layer;
    const MaterialDef *s = &srcFace->mtldef[ layer ];
    for ( int i = 0; i < def->faceCount; ++i )
        memcpy( &def->faces[i].mtldef[ layer ], s, sizeof(MaterialDef) );   // 0x24 bytes
    ++def->version;
}

// forward declare TexMatToFakeTexCoords (0x472df0, __usercall eax/ebx)
extern void TexMatToFakeTexCoords( MaterialDef *mtlDef, texdef_sub_t *texDef );

// ════════════════════════════════════════════════════════════════════════════
//  7. Brush_SetFaceTexdef  (0x4767e0)
//  Copies a texdef_sub_t into the current-layer MaterialDef of a face.
// ════════════════════════════════════════════════════════════════════════════
brush_t *Brush_SetFaceTexdef( const texdef_sub_t *texDef, face_t *f, brush_t *b )
{
    iassert( b );        // brush.cpp:2885
    iassert( f );        // brush.cpp:2886
    iassert( f >= &b->faces[0] && f < &b->faces[b->faceCount] );   // brush.cpp:2887
    iassert( texDef );   // brush.cpp:2888

    MaterialDef *md = &f->mtldef[ g_qeglobals.current_edit_layer ];
    memcpy( &md->mat_texDef, texDef, sizeof(md->mat_texDef) );
    TexMatToFakeTexCoords( md, &md->mat_texDef );
    ++b->version;
    return b;
}

// ════════════════════════════════════════════════════════════════════════════
//  8. sub_4768B0 (0x4768b0, Brush_SetFaceSampleSize)
//  Sets the texture sample size for a face (or all faces if brush has a patch).
//  NAME-MATCH FIX: this body used to be named Brush_SetFaceSampleSize, but every caller
//  (Brush_SetSampleSize, and sub_477080's delegation) refers to it as `sub_4768B0` — which
//  resolved to the engine_stubs FATAL stub, so the real function was dead and "set sample
//  size" crashed.  Renamed to the IDB symbol so the callers' externs reach it.
// ════════════════════════════════════════════════════════════════════════════
// sub_477080 (0x477080): whole-brush sample size setter (defined just below).
extern void sub_477080( brush_t *b, int sampleSize );

void sub_4768B0( face_t *f, brush_t *b, int sampleSize )
{
    iassert( b );   // brush.cpp:2902
    iassert( f );   // brush.cpp:2903
    iassert( f >= &b->faces[0] && f < &b->faces[b->faceCount] );   // brush.cpp:2904

    if ( b->patch )
    {
        sub_477080( b, sampleSize );
    }
    else
    {
        // IDA: v6 = (MaterialDef *)&f->pad_0x0040[8];
        // face_t has mtldef[4] starting at offset 36 (0x24).
        // pad_0x0040 = IDA's name for the flat-array view starting at offset 0x40=64.
        // offset 64 + 8 = 72, but mtldef[0] starts at 36.
        // 72 - 36 = 36 = sizeof(MaterialDef) = mtldef[1].
        // So v6 = &face->mtldef[1] — the lightmap material def.
        MaterialDef *md = &f->mtldef[1];
        qtexture_s *lyrMtl = MaterialDef_GetLayeredMaterial( md );
        int width  = lyrMtl ? lyrMtl->width  : 512;
        int height = lyrMtl ? lyrMtl->height : 512;
        float *v10 = &md->mat_texDef.size[ 7 * LayerMat::GetCurrentLayer( md ) ];
        v10[0] = (float)(sampleSize * width);
        v10[1] = (float)(sampleSize * height);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  sub_476ED0 (0x476ED0, Brush_SetAllFaceTextures) — the per-BRUSH "set all face
//  textures" apply.  Reached from Brush_SetTexture's brush pass (select.cpp) when
//  whole BRUSHES (not faces) are selected and a material is clicked, and from the
//  drag.cpp camera apply-material paths.  Was a benign NO-OP stub in engine_stubs.cpp
//  — so clicking a texture with a brush selected did nothing.  Faithful port:
//   1) Copy the clicked MaterialDef (a2) into EVERY face's mtldef[current_edit_layer].
//      For a patch brush the IDB then fans face i's mtldef[layer] out to ALL faces'
//      mtldef[layer] (redundant inner copy, face stride 232 = sizeof(face_t)); bumps
//      ->version inside the per-face loop.
//   2) Brush_BuildWindings(a1,1); SetupVertexSelection() if vertex/edge mode;
//      MarkMapModified(); ++version ("tex wont update visually without this").
//   3) Patch brush: if a5 → stamp the patch's per-layer lyrMtl/radMtl from a2 + bump
//      patch version + refresh patch inspector (if open) + return; else (a5==0) →
//      PMESH_34 (drag-texture-onto-a-patch projection).
//   4) Brush_UpdateSpecialMaterialFlag(a1).
//  The COMMON case (the user's reported bug — a regular brush) has patch==null: only
//  the face-copy loop + BuildWindings + UpdateSpecialMaterialFlag run.
// ════════════════════════════════════════════════════════════════════════════
extern char PMESH_34( MaterialDef *a1, patchMesh_t *a2, char a3, float a4 );   // pmesh.cpp (0x4428f0)
static void Brush_UpdateSpecialMaterialFlag( brush_t *def );                   // fwd (static, defined below 0x47b940)

void sub_476ED0( brush_t *a1, MaterialDef *a2, char a3, float a4, char a5 )
{
    const int layer = g_qeglobals.current_edit_layer;

    for ( unsigned int i = 0; i < (unsigned int)a1->faceCount; ++i )            // 0x476edb
    {
        face_t *v5 = &a1->faces[i];                                             // 0x476ef6
        qmemcpy( &v5->mtldef[layer], a2, sizeof( MaterialDef ) );               // 0x476f0b

        if ( a1->patch )                                                        // 0x476f0d  fan-out (patch only)
        {
            for ( unsigned int j = 0; j < (unsigned int)a1->faceCount; ++j )    // 0x476f12
                qmemcpy( (char *)&a1->faces->mtldef[layer] + j * sizeof( face_t ), // 0x476f50  stride 232
                         &v5->mtldef[layer], sizeof( MaterialDef ) );
        }
        ++a1->version;                                                          // 0x476f5f
    }

    Brush_BuildWindings( a1, 1 );                                              // 0x476f84
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();                                               // 0x476f9a
    MarkMapModified();                                                        // 0x476f9f

    patchMesh_t *patch = a1->patch;                                           // 0x476fa4
    ++a1->version;                                                            // 0x476fa7  tex wont update visually without this
    if ( patch )                                                             // 0x476fad
    {
        if ( a5 )                                                            // 0x476fb3
        {
            patchMesh_material *pmDst = &patch->texture + layer;
            pmDst->lyrMtl = ( (patchMesh_material *)a2 )->lyrMtl;   // 0x476fc0
            pmDst->radMtl = ( (patchMesh_material *)a2 )->radMtl;   // 0x476fcd
            ++patch->version;                                                // 0x476fd1
            if ( g_PatchDialog_GetHwnd() )                                   // 0x476fd8 CWnd_PatchDialog.m_hWnd
            {
                g_PatchDialog_GetPatchInfo();                               // 0x476fe6
                Brush_UpdateSpecialMaterialFlag( a1 );                      // 0x476fec
                return;                                                     // 0x476ff9
            }
        }
        else
        {
            PMESH_34( a2, patch, a3, a4 );                                  // 0x477005
        }
    }
    Brush_UpdateSpecialMaterialFlag( a1 );                                  // 0x47700e
}

// ════════════════════════════════════════════════════════════════════════════
//  8b. sub_477080 (0x477080) — whole-brush sample size setter (was a FATAL stub).
//  Sets the lightmap-channel texdef size of every face to (sampleSize * material w/h),
//  rebuilds the windings, bumps the version, and for a patch brush delegates the patch
//  refinement to sub_442B00.  Reached from Brush_SetSampleSize's brush pass and from
//  sub_4768B0's patch branch.  The real sub_4768B0 body above was dead (name mismatch) and
//  this was a FATAL stub, so "set sample size" crashed on both passes.
// ════════════════════════════════════════════════════════════════════════════
extern void sub_442B00( patchMesh_t *patch, int sampleSize );   // pmesh.cpp 0x442B00 (patch sample-size core)

void sub_477080( brush_t *b, int sampleSize )
{
    for ( int i = 0; i < b->faceCount; ++i )
    {
        MaterialDef *md = &b->faces[i].mtldef[1];   // lightmap channel
        qtexture_s *lyrMtl = MaterialDef_GetLayeredMaterial( md );
        int width  = lyrMtl ? lyrMtl->width  : 512;
        int height = lyrMtl ? lyrMtl->height : 512;
        float *v7 = &md->mat_texDef.size[ 7 * LayerMat::GetCurrentLayer( md ) ];
        v7[0] = (float)(sampleSize * width);
        v7[1] = (float)(sampleSize * height);
    }
    Brush_BuildWindings( b, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++b->version;
    if ( b->patch )
        sub_442B00( b->patch, sampleSize );   // patch sample-size core (texCoord resample deferred)
}

// ════════════════════════════════════════════════════════════════════════════
//  8c. sub_477020 (0x477020, Brush_SetAllTextureMapping) — was a FATAL stub.
//  Apply one texdef to EVERY face of a brush (via the already-real sub_4767E0 =
//  Brush_SetFaceTexdef), then rebuild windings + bump version.  Reached from
//  Brush_SetTextureMapping's brush pass ("set brush texture mapping"); the FATAL stub
//  crashed that pass (the face pass already worked via sub_4767E0).
// ════════════════════════════════════════════════════════════════════════════
extern void sub_4767E0( const texdef_sub_t *texDef, int facePtr, int brushDef );   // engine_stubs trampoline -> Brush_SetFaceTexdef

void sub_477020( brush_t *b, const texdef_sub_t *texdef )
{
    for ( int i = 0; i < b->faceCount; ++i )
        sub_4767E0( texdef, (int)(intptr_t)&b->faces[i], (int)(intptr_t)b );
    Brush_BuildWindings( b, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++b->version;
}

// ════════════════════════════════════════════════════════════════════════════
//  9. Brush_ApplyTextureProjection  (0x476a30)
//  Copies texture projection from srcFace to dstFace.
//  a1=srcFace(int), a2=brush(int), a3=dstFace(int) — raw addresses used as
//  byte-offset bases throughout.
// ════════════════════════════════════════════════════════════════════════════
// IDA a1=srcFace, a2=dstBrushDef (brush_t*), a3=dstFace.
// Callers pass (srcFacePtr, brushPtr, dstFacePtr) — note a2 is the brush, not the face.
brush_t *Brush_ApplyTextureProjection( int srcFace, brush_t *b, int dstFace )
{
    // IDA's 36*layer+face+44 = &mtldef[layer].mat_texDef (MaterialDef @36+36*layer,
    // texDef @+8); +60 rotate, +64 crossterm; plane.normal @+192, plane.dist @+208.
    face_t *src = (face_t *)(intptr_t)srcFace;
    face_t *dst = (face_t *)(intptr_t)dstFace;

    int layer = g_qeglobals.current_edit_layer;

    texdef_sub_t *srcTd = &src->mtldef[layer].mat_texDef;
    texdef_sub_t *dstTd = &dst->mtldef[layer].mat_texDef;

    float src_v19 = srcTd->crossterm;
    float src_v17 = srcTd->rotate;
    float *src_normal = src->plane.normal;
    int dst_base = (int)(intptr_t)dstTd;

    {
        // IDA v39..v57 are adjacent stack slots filled through BYREF pointers
        // (&v39, &v32, &v35, &v55, &v47) — they must be real arrays here, not
        // separate scalars (separate locals are not guaranteed contiguous).
        float texVec[8];    // v39..v46 — Face_MoveTexture output, later matrix-transform output
        float inVec[8];     // v47..v54 — master/slave plane pair, then the two stride-2 input vectors
        float tvGrad[3];    // v32..v34 — Face_SolveTexGradient out, later Vec3Cross(dst) out
        float tvAxis[3];    // v35..v37 — shared rotation axis
        float crossSrc[3];  // v55..v57 — Vec3Cross(src) out

        Face_MoveTexture( (int)(intptr_t)srcTd, src_normal,
                          (int)texVec, (int)(intptr_t)srcTd->shift,
                          src_v17, src_v19 );

        float *dst_normal = dst->plane.normal;

        // dot product of normals
        float dot = src->plane.normal[2] * dst->plane.normal[2]
                  + src->plane.normal[1] * dst->plane.normal[1]
                  + src->plane.normal[0] * dst->plane.normal[0];

        if ( dot > 0.9990000128746033 || dot < -0.9989999999525025 )
        {
            // Parallel planes: direct copy of texdef values
            float dist_val = dst->plane.dist;
            int dst20 = dst_base + 20;
            int dst16 = dst_base + 16;
            texturevecs_02( dst_base, (int)texVec, (float)dst_normal[2],
                            (int)dst_normal, dist_val,
                            dst_base + 8, dst16, dst20 );
        }
        else
        {
            // Non-parallel: build rotation matrix. inVec = v47..v54:
            // [0..3] master plane (normal, -dist), [4..7] slave plane.
            inVec[0] = *src_normal;
            inVec[1] = src->plane.normal[1];
            inVec[2] = src->plane.normal[2];
            inVec[4] = *dst_normal;
            inVec[5] = dst->plane.normal[1];
            inVec[6] = dst->plane.normal[2];
            inVec[3] = -src->plane.dist;
            inVec[7] = -dst->plane.dist;

            sub_4769A0( (int)tvAxis, (int)inVec, (int)tvGrad );

            float v29 = src->plane.normal[2] * tvGrad[2]
                      + src->plane.normal[1] * tvGrad[1]
                      + tvGrad[0] * src->plane.normal[0];
            float distFromMasterPlane = (float)(v29 - (double)src->plane.dist);

            float v30 = tvGrad[2] * dst->plane.normal[2]
                      + tvGrad[1] * dst->plane.normal[1]
                      + tvGrad[0] * dst->plane.normal[0];
            float distFromSlavePlane = (float)(v30 - (double)dst->plane.dist);

            vassert( (fabsf( distFromMasterPlane ) < 0.01f), "(distFromMasterPlane) = %g", (double)distFromMasterPlane );   // brush.cpp:2992
            vassert( (fabsf( distFromSlavePlane ) < 0.01f),  "(distFromSlavePlane) = %g",  (double)distFromSlavePlane );    // brush.cpp:2993

            // Build 4x4 matrix v58 (64-byte GfxMatrix)
            float v58[16], v59[16];
            memset( v58, 0, sizeof(v58) );
            v58[0]  = tvGrad[0];
            v58[1]  = tvGrad[1];
            v58[2]  = tvGrad[2];
            v58[3]  = 1.0f;

            // texVec = Face_MoveTexture output (8 floats at IDA &v39):
            //   [0..2] S-vector, [3] S-shift, [4..6] T-vector, [7] T-shift
            // IDA verbatim (v24/v25 temporaries):
            float tv24 = tvGrad[2] * texVec[2] + texVec[1] * tvGrad[1] + texVec[0] * tvGrad[0];
            inVec[0] = tv24 + texVec[3];
            float tv25 = tvGrad[0] * texVec[4] + tvGrad[1] * texVec[5] + texVec[6] * tvGrad[2];
            inVec[1] = tv25 + texVec[7];

            v58[4]  = tvAxis[0];
            v58[5]  = tvAxis[1];
            v58[6]  = tvAxis[2];

            inVec[2] = tvAxis[0] * texVec[0] + tvAxis[1] * texVec[1] + tvAxis[2] * texVec[2];
            inVec[3] = texVec[5] * tvAxis[1] + texVec[4] * tvAxis[0] + texVec[6] * tvAxis[2];

            v58[8]  = *dst_normal;
            v58[9]  = dst->plane.normal[1];
            v58[10] = dst->plane.normal[2];

            inVec[4] = texVec[0] * src->plane.normal[0] + texVec[1] * src->plane.normal[1] + texVec[2] * src->plane.normal[2];
            inVec[5] = texVec[5] * src->plane.normal[1] + texVec[4] * src->plane.normal[0] + src->plane.normal[2] * texVec[6];

            Vec3Cross( dst_normal, tvAxis, tvGrad );
            Vec3Normalize_R( tvGrad );
            Vec3Cross( src_normal, tvAxis, crossSrc );
            Vec3Normalize_R( crossSrc );

            v58[12] = tvGrad[0];
            v58[13] = tvGrad[1];
            v58[14] = tvGrad[2];

            inVec[6] = crossSrc[1] * texVec[1] + crossSrc[0] * texVec[0] + crossSrc[2] * texVec[2];
            inVec[7] = crossSrc[1] * texVec[5] + crossSrc[0] * texVec[4] + crossSrc[2] * texVec[6];

            MatrixInverse44( v58, v59 );

            // Apply the inverse matrix v59 (row-major 4x4) to two input 4-vectors,
            // writing the 8 transformed tex-vec floats texVec[0..7]. IDA 0x476a30 nests
            // 2 (outer, in = &v47 then &v48) x 4 (row): each
            //   out[row] = v59[row][0]*in[0] + v59[row][1]*in[2]
            //            + v59[row][2]*in[4] + v59[row][3]*in[6]
            // (in indexed stride-2: in[0]/in[2]/in[4]/in[6] = v47/v49/v51/v53 then
            //  v48/v50/v52/v54; out[0..3]=texVec[0..3] then out[4..7]=texVec[4..7]).
            // FIX (rule-0 trace, 2026-06-20): the prior port collapsed this to a single
            // 2-iter loop with FIXED rows 0/3, writing only 2 outputs — a botched
            // transcription that yields wrong tex vectors on Drag_FaceAlign (no headless
            // gate exercises this path, so it was latent).
            for ( int o = 0; o < 2; ++o )
            {
                const float *in = &inVec[o];
                for ( int r = 0; r < 4; ++r )
                    texVec[4 * o + r] = v59[4 * r + 0] * in[0]
                                      + v59[4 * r + 1] * in[2]
                                      + v59[4 * r + 2] * in[4]
                                      + v59[4 * r + 3] * in[6];
            }

            float dist_dst = dst->plane.dist;
            texturevecs_02( dst_base, (int)texVec, (float)dst_normal[2],
                            (int)dst_normal, dist_dst,
                            dst_base + 8, dst_base + 16, dst_base + 20 );
        }
    }

    ++b->version;
    return b;
}

// ════════════════════════════════════════════════════════════════════════════
//  1. Face_InitTextureChannel  (0x472c90) — replaces Chunk A's Face_InitMaterialChannel
//  (The Chunk A version was named Face_InitMaterialChannel; this is the real IDA name.)
//  Kept as static; same body as in Chunk A but renamed to match the IDA name.
// ════════════════════════════════════════════════════════════════════════════
// Already ported in Chunk A as Face_InitMaterialChannel — no separate entry needed.

// ════════════════════════════════════════════════════════════════════════════
//  2+3. Face_InitMaterialChannels / Brush_InitMaterialChannels  (0x472d30/0x472d90)
//  The Chunk A ports were named Face_SetDefaultMaterials / Brush_SetDefaultMaterials.
//  These are the real implementations — same code already present above.
// ════════════════════════════════════════════════════════════════════════════
// Already ported in Chunk A — no separate entry needed.

// ════════════════════════════════════════════════════════════════════════════
//  10. Face_ParseSurfDef  (0x472ec0)
//  Parses one surface definition from the text stream for a face.
// ════════════════════════════════════════════════════════════════════════════
// the channel-2 unk3 clear is ((int*)v11_base)[6]=0 (mov [edi+18h],0);
// 1478 KEEP_VERBOSE (UNNAMED_FIELD, flattened MaterialDef).
static int Face_ParseSurfDef( const char **text, MaterialDef *surfDef, int version, int channel )
{
    iassert( surfDef );   // brush.cpp:1444

    patchMesh_material *v4 = (patchMesh_material *)surfDef;
    const char *v6;
    char v31[1028];

    if ( channel == 2 )
    {
        if ( version >= 4 )
        {
            Map_ParseEntityLayerKey( text, "smoothing_hard", v31, "smoothing" );
            v4 = (patchMesh_material *)surfDef;
            v6 = v31;
        }
        else
        {
            v6 = "smoothing_hard";
        }
    }
    else
    {
        parseInfo_t *tok;
        if ( version >= 4 )
        {
            Com_SetSpaceDelimited( 1 );
            tok = Com_ParseOnLine( text );
            Com_SetSpaceDelimited( 0 );
        }
        else
        {
            tok = Com_ParseOnLine( text );
        }
        char *v8 = tok->token;
        if ( strchr( tok->token, 32 ) )
        {
            Error( "Material name with a space in it! '%s'\n", v8 );
            return 0;
        }
        if ( version )
            v6 = v8;
        else
            v6 = sub_45AD50( v8 );
    }

    SetMaterial( v6, v4 );

    // 1478 KEEP_VERBOSE: this fn's surfDef param IS the MaterialDef (the binary's
    // surfDef was the enclosing texdef); no member-path matches the string here.
    if ( !v4->radMtl && !v4->lyrMtl )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                1478, 0, "%s", "surfDef->mtlDef.radMtl || surfDef->mtlDef.lyrMtl" );

    int curLayer = LayerMat::GetCurrentLayer( surfDef );
    // v11 = base of texdef for this layer (28 bytes each beyond the MaterialDef)
    // IDA: v11 = (int)&v4[1] + 28*curLayer;
    // v4 is patchMesh_material* (8 bytes), v4[1] starts 8 bytes past surfDef = surfDef->mat_texDef (+8)
    // Actually: &v4[1] = surfDef+8, which is &surfDef->mat_texDef... but patchMesh_material is 8 bytes.
    // So &v4[1] = (char*)surfDef + 8 = &surfDef->mat_texDef.size[0].
    // Then +28*curLayer advances through 28-byte texdef_sub_t blocks.
    // Since texdef_sub_t is 28 bytes and mat_texDef is a texdef_sub_t (the first block),
    // this is: &surfDef->mat_texDef + curLayer (where each block is 28 bytes).
    float *v11_base = (float *)((char *)surfDef + 8 + 28 * curLayer);

    double v12 = 0.0;

    if ( channel == 2 )
    {
        // smoothing channel: fixed defaults
        v11_base[0] = 0.25f;
        // IDA: *((_DWORD *)&v4[4].lyrMtl + 7 * CurrentLayer) = 0;
        // &v4[4].lyrMtl = surfDef+32; +7*curLayer DWORDs = surfDef+32+28*curLayer = v11+24.
        // FIX (rule-0 trace 2026-06-20): the prior port wrote [5] (v11+20) — which is ALSO
        // zeroed as a float just below (v11_base[5]=0) — and so NEVER zeroed unk3 at v11+24,
        // leaving the smoothing-channel texdef's unk3 uninitialized. The else-branch already
        // uses [6] (v11+24); match it here.
        ((int *)v11_base)[6] = 0;  // unk3 at +24 (= IDA &v4[4].lyrMtl + 7*curLayer)
        v11_base[1] = 0.25f;
        v11_base[2] = 0.0f;
        v11_base[3] = 0.0f;
        v11_base[4] = 0.0f;
        v11_base[5] = 0.0f;
    }
    else
    {
        parseInfo_t *v25 = Com_ParseOnLine( text );
        if ( version >= 4 )
        {
            v11_base[0] = (float)atof( v25->token );
            parseInfo_t *v17 = Com_ParseOnLine( text );
            v11_base[1] = (float)atof( v17->token );
            parseInfo_t *v18 = Com_ParseOnLine( text );
            v11_base[2] = (float)atof( v18->token );
            parseInfo_t *v19 = Com_ParseOnLine( text );
            v11_base[3] = (float)atof( v19->token );
            parseInfo_t *v20 = Com_ParseOnLine( text );
            v11_base[4] = (float)atof( v20->token );
            parseInfo_t *v21 = Com_ParseOnLine( text );
            v11_base[5] = (float)atof( v21->token );
        }
        else
        {
            v11_base[2] = (float)j__atol( v25->token );
            parseInfo_t *v13 = Com_ParseOnLine( text );
            v11_base[3] = (float)j__atol( v13->token );
            parseInfo_t *v14 = Com_ParseOnLine( text );
            v11_base[4] = (float)j__atol( v14->token );
            parseInfo_t *v15 = Com_ParseOnLine( text );
            v11_base[0] = (float)atof( v15->token );
            parseInfo_t *v16 = Com_ParseOnLine( text );
            float v28 = (float)atof( v16->token );
            v11_base[5] = 0.0f;
            v11_base[0] = sub_4AACC0( v11_base[0], 4 );   // IDA: mov ecx,4 (FP-noise scrub)
            v11_base[1] = sub_4AACC0( v28, 4 );
        }

        ((int *)v11_base)[6] = 0;   // unk3 at +24

        if ( v11_base[0] == 0.0f )
        {
            float def = channel ? 16.0f : 0.25f;
            v11_base[0] = def;
        }
    }

    if ( v11_base[1] == 0.0f )
    {
        float def = channel ? 16.0f : 0.25f;
        v11_base[1] = def;
    }

    TexMatToFakeTexCoords( surfDef, (texdef_sub_t *)v11_base );
    return 1;
}

// ════════════════════════════════════════════════════════════════════════════
//  Brush_RotateFacePlanepts (sub_478630, 0x478630) — rotate every face's three
//  planepts about a pivot by sequential X/Y/Z Euler angles (degrees), via
//  sub_4AA220.  Reached only on the physics box/cylinder map-parse path
//  (Brush_ParsePhysicsBox/ParsePhysCylinder), which orient the freshly-created
//  brush to its stored axis.  __usercall(pivot@<ebx>, anglesDeg@<eax>,
//  brush_t *b@<stack>) returning the face count → clean cdecl.  Offsets verified:
//  brush_t.faceCount @ +0x40, brush_t.faces @ +0x44 (qe3.h static_asserts);
//  face_t stride 232 (0xE8), planepts @ +0 (face_t static_assert).
// ════════════════════════════════════════════════════════════════════════════
static void Brush_RotateFacePlanepts( const float *pivot, const float *anglesDeg,
                                      brush_t *b )
{
    int faceCount = b->faceCount;
    face_t *faces = b->faces;
    for ( int fi = 0; fi < faceCount; ++fi )
    {
        face_t *f = &faces[fi];
        for ( int p = 0; p < 3; ++p )
        {
            float rel[3], rotated[3];
            rel[0] = f->planepts[p][0] - pivot[0];
            rel[1] = f->planepts[p][1] - pivot[1];
            rel[2] = f->planepts[p][2] - pivot[2];
            sub_4AA220( rel, anglesDeg, rotated );
            f->planepts[p][0] = pivot[0] + rotated[0];
            f->planepts[p][1] = pivot[1] + rotated[1];
            f->planepts[p][2] = pivot[2] + rotated[2];
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  12. Brush_ParsePhysicsBox  (0x473660)
// ════════════════════════════════════════════════════════════════════════════
// size = (double)wh * 0.25 (int fimul); AxisToAngles reorder
// {ang[2],ang[0],ang[1]}.
static brush_t *Brush_ParsePhysicsBox( const char **text )
{
    // IDA: { ungetToken-prologue + Com_ParseExt(text,1) } == Com_Parse(text)
    if ( Com_Parse( text )->token[0] != '{' )
        return nullptr;

    float axis[9];
    axis[0] = (float)Com_ParseFloat( text );
    parseInfo_t *t = Com_ParseOnLine( text );
    axis[1] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); axis[2] = (float)atof( t->token );
    axis[3] = (float)Com_ParseFloat( text );
    t = Com_ParseOnLine( text ); axis[4] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); axis[5] = (float)atof( t->token );
    axis[6] = (float)Com_ParseFloat( text );
    t = Com_ParseOnLine( text ); axis[7] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); axis[8] = (float)atof( t->token );

    // center[3] = box center; half[3] = box half-extents (§11: the IDA's
    // v25/v26/v27 and angles/v29/v30 are consecutive stack floats used as float[3]).
    float center[3], half[3];
    t = Com_ParseOnLine( text ); center[0] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); center[1] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); center[2] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); half[0]   = (float)atof( t->token );
    t = Com_ParseOnLine( text ); half[1]   = (float)atof( t->token );
    t = Com_ParseOnLine( text ); half[2]   = (float)atof( t->token );
    Com_MatchToken( text, "}", 0 );

    MaterialDef v23;
    memset( &v23.mat_texDef, 0, sizeof(v23.mat_texDef) );

    float mins[3], maxs[3];
    mins[0] = center[0] - half[0];
    mins[1] = center[1] - half[1];
    mins[2] = center[2] - half[2];
    maxs[0] = center[0] + half[0];
    maxs[1] = center[1] + half[1];
    maxs[2] = center[2] + half[2];

    v23.lyrMtl = g_qeglobals.random_texture_stuff[0].mtl.lyrMtl;
    v23.radMtl = g_qeglobals.random_texture_stuff[0].mtl.radMtl;

    qtexture_s *lm = MaterialDef_GetLayeredMaterial( &v23 );
    int w = lm ? lm->width  : 512;
    int h = lm ? lm->height : 512;

    texdef_sub_t *texSlot = &v23.mat_texDef + LayerMat::GetCurrentLayer( &v23 );
    texSlot->size[0] = (double)w * 0.25;
    texSlot->size[1] = (double)h * 0.25;

    brush_t *v21 = Brush_Alloc( &v23, nullptr );
    Brush_Create( mins, maxs, (brush_t *)v21, nullptr );
    v21->numberId = 1;
    Brush_BuildWindings( v21, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++v21->version;

    // Recover the orientation angles from the 3x3 axis matrix, then reorder
    // (pitch,yaw,roll) -> (roll,pitch,yaw) exactly as the binary does (0x4738df..),
    // and rotate the brush's face planepts about the box center.
    float ang[3];
    AxisToAngles( ang, (float (*)[3])axis );
    float rot[3] = { ang[2], ang[0], ang[1] };
    Brush_RotateFacePlanepts( center, rot, v21 );
    Brush_BuildWindings( v21, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++v21->version;
    return v21;
}

// ════════════════════════════════════════════════════════════════════════════
//  Brush primitives — reshape the SELECTED brush into an N-sided cylinder
//  (Brush_MakeSided), a cone (Brush_MakeSidedCone) or a sphere
//  (Brush_MakeSidedSphere).  All three operate IN-PLACE on the existing brush
//  DEF: they free the old face array and re-allocate it with the new face count,
//  then rebuild the windings.  This is the CoD divergence from GtkRadiant (which
//  Brush_Free's the brush and Brush_Alloc's a fresh one + prepends faces) — the
//  CoD binary keeps the same brush_t and writes the faces into an array in the
//  order [top, bottom, side0, side1, …].  Math is verified against GtkRadiant
//  1.6 for the planept rotation, but the constants (min-width, conditional snap,
//  in-place layout) are taken from the IDA decompile (0x4731E0 / 0x47BC10 /
//  0x47BE90), which is ground truth.
// ════════════════════════════════════════════════════════════════════════════
extern signed int QE_SingleBrush();                 // qe3.cpp (0x48c8b0)
// IDB 'selected_brushes_next' (0x23f1868) is just selected_brushes.next — the
// first selected INSTANCE node — not a distinct symbol (see drag.cpp note).

// Sphere_Point (sub_4AA390, 0x4AA390) — fill out[3] for one sphere planept.  The
// binary computes cos/sin of `ang` and writes radius·cos², radius·cos·sin,
// radius·sin (NOT a textbook spherical point — transcribed verbatim from the
// fsincos/fmul sequence; the `cos²` term is real, st-stack folding makes it look
// like `v4*a2*v4` in the decompile and that is exactly what it is).
// fsincos ring point, single precision.
static void Sphere_Point( float *out, float radius, float ang )
{
    float c = cosf( ang );
    float s = sinf( ang );
    out[0] = c * radius * c;
    out[1] = c * ( s * radius );
    out[2] = radius * s;
}

// ─── Brush_MakeSided (0x4731E0) ───────────────────────────────────────────────
// a1 = the brush DEF (selected_brushes_next->def), a2 = side count,
// a3 = the cylinder axis (0=X,1=Y,2=Z) from the active XY-view type,
// a4 = snap-to-integer flag (floor(x+0.5) on each side-face point).
void Brush_MakeSided( int a1, unsigned int sides, int axis, char snap )
{
    if ( sides < 3 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Bad sides number" );
        return;
    }
    if ( sides >= 1020 )                            // MAX_POINTS_ON_WINDING - 4
    {
        Sys_Printf( "too many sides.\n" );
        return;
    }

    brush_t *def = (brush_t *)a1;
    float mins[3], maxs[3], mid[3];
    Vec3Copy( def->mins, mins );
    Vec3Copy( def->maxs, maxs );

    // Centre of the brush; widest non-axis half-extent (min 1.0 — the CoD
    // divergence: GtkRadiant + the cone/sphere clamp to 8.0, makeSided to 1.0).
    float width = 1.0f;
    for ( int i = 0; i < 3; ++i )
    {
        mid[i] = ( maxs[i] + mins[i] ) * 0.5f;
        if ( i == axis )
            continue;
        float half = ( maxs[i] - mins[i] ) * 0.5f;
        if ( width < half )
            width = half;
    }

    // Re-allocate the face array in place: sides + a top and a bottom cap.
    Face_Free( def->faceCount, def->faces );
    def->faceCount  = (int)sides + 2;
    def->faces = Face_Alloc_R( def->faceCount );

    GfxColor white;
    Byte4PackPixelColor( const_cast<float *>( colorWhite ), &white );

    const int a1ax = ( axis + 1 ) % 3;
    const int a2ax = ( axis + 2 ) % 3;

    // face[0] — top cap (axis = maxs), winding wound to face +axis.
    face_t *top = &def->faces[0];
    memcpy( top->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );
    top->field_0xE4 = white.packed;
    top->planepts[0][a1ax] = maxs[a1ax]; top->planepts[0][a2ax] = maxs[a2ax]; top->planepts[0][axis] = maxs[axis];
    top->planepts[1][a1ax] = maxs[a1ax]; top->planepts[1][a2ax] = mins[a2ax]; top->planepts[1][axis] = maxs[axis];
    top->planepts[2][a1ax] = mins[a1ax]; top->planepts[2][a2ax] = mins[a2ax]; top->planepts[2][axis] = maxs[axis];

    // face[1] — bottom cap (axis = mins).
    face_t *bot = &def->faces[1];
    memcpy( bot->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );
    bot->field_0xE4 = white.packed;
    bot->planepts[0][a1ax] = mins[a1ax]; bot->planepts[0][a2ax] = mins[a2ax]; bot->planepts[0][axis] = mins[axis];
    bot->planepts[1][a1ax] = maxs[a1ax]; bot->planepts[1][a2ax] = mins[a2ax]; bot->planepts[1][axis] = mins[axis];
    bot->planepts[2][a1ax] = maxs[a1ax]; bot->planepts[2][a2ax] = maxs[a2ax]; bot->planepts[2][axis] = mins[axis];

    const double dtheta = 6.283185482025146 / (double)sides;
    for ( unsigned int i = 0; i < sides; ++i )
    {
        face_t *f = &def->faces[2 + i];
        memcpy( f->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );
        f->field_0xE4 = white.packed;

        float ang = (float)( (double)i * dtheta );
        float cv  = cosf( ang );
        float sv  = sinf( ang );

        f->planepts[0][a1ax] = cv * width + mid[a1ax];
        f->planepts[0][a2ax] = sv * width + mid[a2ax];
        if ( snap )
        {
            f->planepts[0][a1ax] = floorf( f->planepts[0][a1ax] + 0.5f );
            f->planepts[0][a2ax] = floorf( f->planepts[0][a2ax] + 0.5f );
        }
        f->planepts[0][axis] = mins[axis];

        f->planepts[1][a1ax] = f->planepts[0][a1ax];
        f->planepts[1][a2ax] = f->planepts[0][a2ax];
        f->planepts[1][axis] = maxs[axis];

        f->planepts[2][a1ax] = f->planepts[0][a1ax] - sv * width;
        f->planepts[2][a2ax] = cv * width + f->planepts[0][a2ax];
        if ( snap )
        {
            f->planepts[2][a1ax] = floorf( f->planepts[2][a1ax] + 0.5f );
            f->planepts[2][a2ax] = floorf( f->planepts[2][a2ax] + 0.5f );
        }
        f->planepts[2][axis] = maxs[axis];
    }

    Brush_SetDefaultMaterials( def );
    Brush_BuildWindings( def, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++def->version;
    g_nUpdateBits = -1;
}

// ─── Brush_MakePhysBox (0x47C180) / Brush_MakePhysCylinder (0x47C310) ─────────
//  Physics→Box (36120) / Physics→Cylinder (36113).  Both REPLACE the single selected
//  brush with a freshly-allocated brush whose numberId (IDB "total_size_0x58") tags it as
//  a physics primitive — 1 = box, 2 = cylinder — using the current MaterialDef from
//  g_qeglobals.random_texture_stuff[0] with the layer texture size seeded at width/4,
//  height/4 (the same seed the physics map-PARSE path uses, Brush_ParsePhysicsBox /
//  Brush_ParsePhysCylinder above).  The box keeps the source brush's AABB; the cylinder
//  squares the XY footprint to the LARGER of the two extents about the AABB centre and
//  then runs Brush_MakeSided_Prolog(16, 0) to cut the 16-sided barrel.
//  Ported verbatim, including the binary's dead `&active_brushes == &selected_brushes`
//  branch (same idiom as Layers_KeepOnlySelectionInLayer above).
// ─────────────────────────────────────────────────────────────────────────────
void Brush_MakeSided_Prolog( unsigned int sides, char snap );  // below (0x4735E0)
extern void Select_Delete();                                   // select.cpp (0x48E760)
extern void Select_Brush( selbrush_t *b, char some_overwrite,
                          char bStatus, char center_grid_on_selection );  // select.cpp (0x48DCC0)

// The shared tail of both handlers: seed a MaterialDef from the current texture, allocate
// + create the brush over [mins,maxs], tag it, link it into the source brush's entity,
// splice it into active_brushes, delete the old selection and select the new brush.
static selbrush_t *Brush_MakePhysPrimitive( float *mins, float *maxs,
                                            entity_s *owner, int physKind )
{
    MaterialDef mtl;
    // NOTE: the cylinder path (0x47C349) memsets 32 bytes from &radMtl, the box path
    // (0x47C1A1) memsets sizeof(mat_texDef) from &mat_texDef — both clear the same
    // trailing texdef block; the lyrMtl/radMtl pair is written immediately after.
    memset( &mtl.mat_texDef, 0, sizeof( mtl.mat_texDef ) );
    mtl.lyrMtl = g_qeglobals.random_texture_stuff[0].mtl.lyrMtl;
    mtl.radMtl = g_qeglobals.random_texture_stuff[0].mtl.radMtl;

    qtexture_s *lm = MaterialDef_GetLayeredMaterial( &mtl );
    int w = lm ? lm->width  : 512;
    int h = lm ? lm->height : 512;
    texdef_sub_t *texSlot = &mtl.mat_texDef + LayerMat::GetCurrentLayer( &mtl );
    texSlot->size[0] = (double)w * 0.25;
    texSlot->size[1] = 0.25 * (double)h;

    brush_t *def = Brush_Alloc( &mtl, nullptr );
    Brush_Create( mins, maxs, def, nullptr );
    def->numberId = physKind;                       // IDB total_size_0x58: 1 = box, 2 = cylinder

    entity_s *ownerDef = (entity_s *)owner->def;
    if ( ownerDef )
    {
        Entity_LinkBrush( ownerDef, (selbrush_t *)def );
        Brush_BuildWindings( def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex ||
             g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++def->version;
    }

    selbrush_t *inst = Brush_AddToList( def, owner );
    if ( inst->next || inst->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    if ( &active_brushes == &selected_brushes )     // never true; matches the binary
    {
        Brush_AddToList2( inst );
    }
    else
    {
        inst->next = active_brushes.next;
        active_brushes.next->prev = inst;
        active_brushes.next = inst;
        inst->prev = &active_brushes;
    }

    Select_Delete();
    Select_Brush( inst, 1, 1, 0 );
    return inst;
}

void Brush_MakePhysBox()          // 0x47C180
{
    if ( !QE_SingleBrush() )
        return;

    brush_t  *def   = selected_brushes.next->def;
    entity_s *owner = selected_brushes.next->owner;
    Brush_MakePhysPrimitive( def->mins, def->maxs, owner, 1 );
    g_nUpdateBits = -1;
}

void Brush_MakePhysCylinder()     // 0x47C310
{
    if ( !QE_SingleBrush() )
        return;

    brush_t  *def   = selected_brushes.next->def;
    entity_s *owner = selected_brushes.next->owner;

    // Square the XY footprint about the AABB centre using the LARGER extent.
    float cx = ( def->mins[0] + def->maxs[0] ) * 0.5f;
    float cy = ( def->mins[1] + def->maxs[1] ) * 0.5f;
    float ex = def->maxs[0] - def->mins[0];
    float ey = def->maxs[1] - def->mins[1];
    float side = ( ex - ey < 0.0f ) ? ey : ex;      // IDB 0x47C337: v12 < 0 -> take ey
    float half = 0.5f * side;

    float mins[3] = { cx - half, cy - half, def->mins[2] };
    float maxs[3] = { cx + half, cy + half, def->maxs[2] };

    Brush_MakePhysPrimitive( mins, maxs, owner, 2 );
    Brush_MakeSided_Prolog( 0x10u, 0 );
    g_nUpdateBits = -1;
}

// ─── Brush_MakeSided_Prolog (0x4735E0) ────────────────────────────────────────
// Picks the cylinder axis from the active XY-view orientation, then calls
// Brush_MakeSided on the single selected brush.  m_nViewType: 0=YZ→axis 0,
// 1=XZ→axis 1, 2=XY→axis 2 (no active view → axis 2).
// the IDA re-inits axis=0 when an active XY view exists, so an invalid
// viewType>=3 gives axis 0 (dead path).  The g_pParentWnd null-check is a port guard.
void Brush_MakeSided_Prolog( unsigned int sides, char snap )
{
    if ( !QE_SingleBrush() )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have a single brush selected" );
        return;
    }

    int axis = 2;
    CXYWnd *xy = g_pParentWnd ? g_pParentWnd->m_pActiveXY : nullptr;
    if ( xy )
    {
        switch ( xy->m_nViewType )
        {
        case 0: axis = 0; break;
        case 1: axis = 1; break;
        case 2: axis = 2; break;
        default: axis = 0; break;   // IDA 0x4735E0: baseline re-inits to 0 when an XY view exists
        }
    }
    Brush_MakeSided( (int)selected_brushes.next->def, sides, axis, snap );
}

// ─── Brush_MakeSidedCone (0x47BC10) ───────────────────────────────────────────
// Reshape the single selected brush into an N-sided cone (apex at +Z / maxs[2],
// base ring at mins[2]).  Axis is fixed to Z.  Always snaps to integer.
// cosf/sinf differ sub-ULP from the binary's (float)cos/sin, washed out by
// the floorf to integer planepts.
void Brush_MakeSidedCone( int sides )
{
    if ( sides < 3 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Bad sideCount number" );
        return;
    }
    if ( !QE_SingleBrush() )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have a single brush selected" );
        return;
    }

    brush_t *def = selected_brushes.next->def;
    float mins[3], maxs[3], mid[2];
    Vec3Copy( def->mins, mins );
    Vec3Copy( def->maxs, maxs );

    float width = 8.0f;
    for ( int i = 0; i < 2; ++i )
    {
        mid[i] = ( maxs[i] + mins[i] ) * 0.5f;
        float ext = maxs[i] - mins[i];
        if ( width < ext )
            width = ext;
    }
    width = 0.5f * width;

    Face_Free( def->faceCount, def->faces );
    def->faceCount  = sides + 1;
    def->faces = Face_Alloc_R( def->faceCount );

    // face[0] — the base triangle (the bottom is closed by the side faces meeting
    // the base ring; the cap is a single plane through three base corners).
    face_t *base = &def->faces[0];
    memcpy( base->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );
    base->planepts[0][0] = mins[0]; base->planepts[0][1] = mins[1]; base->planepts[0][2] = mins[2];
    base->planepts[1][0] = maxs[0]; base->planepts[1][1] = mins[1]; base->planepts[1][2] = mins[2];
    base->planepts[2][0] = maxs[0]; base->planepts[2][1] = maxs[1]; base->planepts[2][2] = mins[2];

    const double dtheta = 6.283185482025146 / (double)sides;
    for ( int i = 0; i < sides; ++i )
    {
        face_t *f = &def->faces[1 + i];
        memcpy( f->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );

        float ang = (float)( (double)i * dtheta );
        float cv  = cosf( ang );
        float sv  = sinf( ang );

        f->planepts[0][0] = floorf( cv * width + mid[0] + 0.5f );
        f->planepts[0][1] = floorf( sv * width + mid[1] + 0.5f );
        f->planepts[0][2] = mins[2];

        f->planepts[1][0] = mid[0];      // apex
        f->planepts[1][1] = mid[1];
        f->planepts[1][2] = maxs[2];

        f->planepts[2][0] = floorf( f->planepts[0][0] - sv * width + 0.5f );
        f->planepts[2][1] = floorf( cv * width + f->planepts[0][1] + 0.5f );
        f->planepts[2][2] = maxs[2];
    }

    Brush_SetDefaultMaterials( def );
    Brush_BuildWindings( def, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++def->version;
    g_nUpdateBits = -1;
}

// ─── Brush_MakeSidedSphere (0x47BE90) ─────────────────────────────────────────
// Reshape the single selected brush into a sphere built from sides×sides faces
// (sides bands × sides segments).  Each face's three planepts are spherical
// points (Sphere_Point) offset by the brush centre.  Needs sides >= 4.
void Brush_MakeSidedSphere( int sides )
{
    if ( sides < 4 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Bad sideCount number" );
        return;
    }
    if ( !QE_SingleBrush() )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have a single brush selected" );
        return;
    }

    brush_t *def = selected_brushes.next->def;
    float mins[3], maxs[3], mid[3];
    Vec3Copy( def->mins, mins );
    Vec3Copy( def->maxs, maxs );

    float radius = 8.0f;
    for ( int i = 0; i < 2; ++i )
    {
        mid[i] = ( maxs[i] + mins[i] ) * 0.5f;
        float ext = maxs[i] - mins[i];
        if ( radius < ext )
            radius = ext;
    }
    radius = 0.5f * radius;
    mid[2] = ( maxs[2] + mins[2] ) * 0.5f;   // IDB: v28..v30 are the brush centre

    Face_Free( def->faceCount, def->faces );
    def->faceCount  = sides * sides;
    def->faces = Face_Alloc_R( def->faceCount );

    const double dphi = 6.283185482025146 / (double)sides;
    face_t *f = def->faces;

    // Body bands: outer i in [0, sides), inner j runs (sides-1) times.  IMPORTANT
    // (a faithful CoD quirk, NOT a port bug): the IDA inner loop does NOT vary the
    // angle — it recomputes the same face (a0 = i·dphi for planepts[0]/[1], a1 =
    // a0+dphi for planepts[2]) (sides-1) times.  var_8/var_10 are hoisted out of
    // the inner loop in the disasm (loc_47BFC0 reuses them).  Transcribed verbatim.
    for ( int i = 0; i < sides; ++i )
    {
        float a0 = (float)( (double)i * dphi );
        float a1 = (float)( (double)dphi + (double)a0 );
        for ( int j = 0; j < sides - 1; ++j )
        {
            memcpy( f->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );

            Sphere_Point( f->planepts[0], radius, a0 );
            Sphere_Point( f->planepts[1], radius, a0 );
            Sphere_Point( f->planepts[2], radius, a1 );
            for ( int k = 0; k < 3; ++k )
            {
                f->planepts[k][0] += mid[0];
                f->planepts[k][1] += mid[1];
                f->planepts[k][2] += mid[2];
            }
            ++f;
        }
    }
    // Cap band: one face per segment (the IDA tail loop, sides faces).
    for ( int i = 0; i < sides; ++i )
    {
        memcpy( f->mtldef, &g_qeglobals.random_texture_stuff[0].mtl, sizeof( MaterialDef ) );
        float a0 = (float)( (double)i * dphi );
        float a1 = (float)( a0 + (double)dphi );
        Sphere_Point( f->planepts[0], radius, a0 );
        Sphere_Point( f->planepts[1], radius, a1 );
        Sphere_Point( f->planepts[2], radius, a1 );
        for ( int k = 0; k < 3; ++k )
        {
            f->planepts[k][0] += mid[0];
            f->planepts[k][1] += mid[1];
            f->planepts[k][2] += mid[2];
        }
        ++f;
    }

    Brush_SetDefaultMaterials( def );
    Brush_BuildWindings( def, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++def->version;
    g_nUpdateBits = -1;
}

// ════════════════════════════════════════════════════════════════════════════
//  13. Brush_ParsePhysCylinder  (0x473950)
// ════════════════════════════════════════════════════════════════════════════
// MakeSided(16); vectoangles reorder {roll,pitch,yaw}.
static brush_t *Brush_ParsePhysCylinder( const char **text )
{
    // IDA: { ungetToken-prologue + Com_ParseExt(text,1) } == Com_Parse(text)
    if ( Com_Parse( text )->token[0] != '{' )
        return nullptr;

    float vec[3];
    vec[0] = (float)Com_ParseFloat( text );
    parseInfo_t *t = Com_ParseOnLine( text );
    vec[1] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); vec[2] = (float)atof( t->token );

    // center[3] = cylinder center; length along X axis, radius on Y/Z (§11: the IDA's
    // v22/v23/v24 are consecutive stack floats used as the pivot float[3]).
    float center[3];
    t = Com_ParseOnLine( text ); center[0] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); center[1] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); center[2] = (float)atof( t->token );
    t = Com_ParseOnLine( text ); float length = (float)atof( t->token );
    t = Com_ParseOnLine( text ); float radius = (float)atof( t->token );
    Com_MatchToken( text, "}", 0 );

    double halfLen = length * 0.5;
    MaterialDef v19;
    memset( &v19.mat_texDef, 0, sizeof(v19.mat_texDef) );

    float mins[3], maxs[3];
    mins[0] = center[0] - (float)halfLen;
    mins[1] = center[1] - radius;
    mins[2] = center[2] - radius;
    maxs[0] = (float)halfLen + center[0];
    maxs[1] = center[1] + radius;
    maxs[2] = radius + center[2];

    v19.lyrMtl = g_qeglobals.random_texture_stuff[0].mtl.lyrMtl;
    v19.radMtl = g_qeglobals.random_texture_stuff[0].mtl.radMtl;

    qtexture_s *lm = MaterialDef_GetLayeredMaterial( &v19 );
    int w = lm ? lm->width  : 512;
    int h = lm ? lm->height : 512;

    texdef_sub_t *texSlot = &v19.mat_texDef + LayerMat::GetCurrentLayer( &v19 );
    texSlot->size[0] = (double)w * 0.25;
    texSlot->size[1] = (double)h * 0.25;

    brush_t *v17 = Brush_Alloc( &v19, nullptr );
    Brush_Create( mins, maxs, (brush_t *)v17, nullptr );
    v17->numberId = 2;
    Brush_BuildWindings( v17, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++v17->version;

    Brush_MakeSided( (int)v17, 16u, 0, 0 );

    // Orient the 16-sided cylinder along the parsed axis: vectoangles -> (pitch,yaw,0),
    // reorder (pitch,yaw,roll) -> (roll,pitch,yaw), rotate planepts about the center.
    float ang[3];
    vectoangles( ang, (int)vec );
    float rot[3] = { ang[2], ang[0], ang[1] };
    Brush_RotateFacePlanepts( center, rot, v17 );

    brush_t *v18 = v17;
    Brush_BuildWindings( v17, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex ||
         g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++v18->version;
    return v18;
}

// ════════════════════════════════════════════════════════════════════════════
//  11. Brush_Parse  (0x473bd0)
//  Main brush text parser. Parses one brush block from the map text.
// ════════════════════════════════════════════════════════════════════════════
// Real signature (IDA __usercall): the text pointer is the ecx register arg and
// `version` is the stack arg. ParseEntity (entity.cpp, 0x483e70) calls this as
// Brush_Parse(text, version). All the binary's inlined ungetToken/backup_text
// prologues are the engine parser functions folded in by LTCG:
//   { ungetToken-prologue ; Com_ParseExt(text,1) }          == Com_Parse(text)
//   { ungetToken+spaceDelim prologue ; Com_ParseExt(text,0) } == Com_ParseOnLine(text)
//   { ungetToken=1 ; tokenPos=prevTokenPos }                 == Com_UngetToken()
// version<4 reads radMtl->size[0]/[1] as INT (IDA fild), not float;
// 1925/1926/1927 KEEP_VERBOSE (UNNAMED_FIELD).
brush_t *Brush_Parse( const char **text, int version )
{
    // Original allocates a 118784-byte (512 * 232) face buffer on the stack; we use
    // file-static to avoid a 116 KB frame (non-reentrant — map parse is serial).
    static face_t s_faceStack[512];

    ++g_qeglobals.d_parsed_brushes;

    // First token: "patchDef5"/"curve"+"mesh"/"patchTerrainDef3" | physics_* | brush faces.
    parseInfo_t *tok5 = Com_Parse( text );
    const char *v6 = tok5->token;
    int v7;
    const char *v42;
    if ( version )
    {
        v7  = _stricmp( tok5->token, "curve" );
        v42 = "mesh";
    }
    else
    {
        v7  = _stricmp( tok5->token, "patchDef5" );
        v42 = "patchTerrainDef3";
    }
    bool v8b = (v7 == 0);
    bool v9b = (_stricmp( v6, v42 ) == 0);

    if ( v8b || v9b )
    {
        // PATCH / MESH brush — parse the real mesh/curve block (pmesh.cpp).
        // isMesh (v9b) = the token matched the terrain variant ("mesh" for v>=1 /
        // "patchTerrainDef3" for v0); Patch_ParseMesh sets PATCH_TERRAIN for it.
        // Patch_ParseMesh consumes through the mesh-block close '}'; Brush_Parse then
        // reads the brush's outer '}'.  (IDA Brush_Parse 0x473bd0 patch branch.)
        brush_t *patchBrush = Patch_ParseMesh( text, version, v9b ? 1 : 0 );
        parseInfo_t *closer = Com_Parse( text );   // the brush's outer '}'
        if ( !patchBrush || !closer->token[0] || strcmp( closer->token, "}" ) )
        {
            Error( "parsing patch/brush" );
            return nullptr;
        }
        return patchBrush;
    }

    if ( !_stricmp( v6, "physics_cylinder" ) )
    {
        brush_t *v10 = Brush_ParsePhysCylinder( text );
        parseInfo_t *closer = Com_Parse( text );
        if ( !v10 || !closer->token[0] || strcmp( closer->token, "}" ) )
        {
            Error( "parsing physics cylinder" );
            return nullptr;
        }
        return v10;
    }

    if ( !_stricmp( v6, "physics_box" ) )
    {
        brush_t *v13 = Brush_ParsePhysicsBox( text );
        parseInfo_t *closer = Com_Parse( text );
        if ( !v13 || !closer->token[0] || strcmp( closer->token, "}" ) )
        {
            Error( "parsing physics box" );
            return nullptr;
        }
        return v13;
    }

    Com_UngetToken();

    char layerStr[1028];
    int  v46 = 0, v48 = 0;
    if ( version >= 4 )
    {
        Map_ParseEntityLayerKey( text, "000_Global", layerStr, "layer" );
        v46 = sub_42FB80( text );
        v48 = sub_42FBA0( text );
    }
    else
    {
        strcpy( layerStr, "000_Global" );
    }

    int v16 = 0;  // face count
    face_t *v47 = s_faceStack;

    while ( true )
    {
        if ( Com_Parse( text )->token[0] == '}' )
        {
            // Done — build the brush
            brush_t *nb = (brush_t *)operator new( 0x58u );
            memset( nb, 0, 0x58u );
            nb->faceCount = v16;
            nb->faces     = Face_Alloc_R( v16 );
            if ( nb->parent_layer_string )                 // always null after the memset
                j__free_0( nb->parent_layer_string );      // (binary's inlined SetLayerString head)
            size_t ll = strlen( layerStr );
            void *lc = operator new( ll + 1 );
            memcpy( lc, layerStr, ll + 1 );
            nb->parent_layer_string = (char *)lc;
            memcpy( nb->faces, s_faceStack, 232 * v16 );
            return nb;
        }

        Com_UngetToken();   // unget the token we just tested for '}'

        if ( v16 == 512 )
        {
            Error( "too many faces in brush" );
            return nullptr;
        }

        face_t *facePtr = v47;
        v47 = (face_t *)((char *)v47 + 232);
        int nextCount = v16 + 1;
        ((int *)facePtr)[56] = 0;  // w = nullptr

        // Read 3 sets of planepts: "( x y z )"
        for ( int i = 0; i < 3; ++i )
        {
            parseInfo_t *ptok = i ? Com_ParseOnLine( text ) : Com_Parse( text );
            if ( ptok->token[0] != '(' )
            {
                Error( "missing '(' parsing brush" );
                return nullptr;
            }
            float *coords = facePtr->planepts[i];
            for ( int j = 0; j < 3; ++j )
            {
                parseInfo_t *num = Com_ParseOnLine( text );
                coords[j] = (float)atof( num->token );
            }
            parseInfo_t *ptok2 = Com_ParseOnLine( text );
            if ( ptok2->token[0] != ')' )
            {
                Error( "missing ')' parsing brush" );
                return nullptr;
            }
        }

        int v27 = version;
        if ( version < 4 )
        {
            Byte4PackPixelColor( const_cast<float*>(colorWhite),
                                 (GfxColor *)((char *)facePtr + 228) );
            // IDA: sub_472EC0(text, (MaterialDef*)v49+1, version, 0); &mtldef[0] = face+36.
            if ( Face_ParseSurfDef( text, &facePtr->mtldef[0], version, 0 ) )
            {
                face_t *f = facePtr;                    // the binary's local
                iassert( f->surfDef[0].mtlDef.radMtl );            // brush.cpp:1925
                iassert( f->surfDef[0].mtlDef.radMtl->size[0] );   // brush.cpp:1926
                iassert( f->surfDef[0].mtlDef.radMtl->size[1] );   // brush.cpp:1927

                int curLayer2 = LayerMat::GetCurrentLayer( &facePtr->mtldef[0] );
                // IDA: v32 = &mtldef[0] + 28*curLayer + 8 — NOTE the 28-byte (texdef-only)
                // stride over the 36-byte MaterialDef array; kept bit-faithful via the cast.
                texdef_sub_t *tdl =
                    (texdef_sub_t *)( (char *)&facePtr->mtldef[0] + 28 * curLayer2 + 8 );
                float v31 = tdl->size[0];
                tdl->shift[0] = -v31 * tdl->shift[0];
                tdl->shift[1] = -tdl->size[1] * tdl->shift[1];
                // radMtl->size[0]/[1] loaded with FILD (0x474065/0x47406f) = int width/height,
                // NOT a float reinterpret.
                qtexture_s *rm = facePtr->mtldef[0].radMtl;
                tdl->size[0] = (float)( (double)rm->width  * tdl->size[0] );
                tdl->size[1] = (float)( (double)rm->height * tdl->size[1] );
                tdl->sample_size = 0.0f;   // int-0 store in the binary; same bits
                v27 = version;
            }
            else
            {
                return nullptr;
            }
        }

        // face contents/flags words (face[45]=offset 180, face[46]=offset 184)
        if ( v27 > 3 )
        {
            ((int *)facePtr)[45] = v46;
        }
        else
        {
            parseInfo_t *v34 = Com_ParseOnLine( text );
            ((int *)facePtr)[45] = (int)j__atol( v34->token );
        }

        bool doSampleSize;   // = IDA LABEL_65 (sample size + lightmap + Init_MaterialLayer)
        if ( v27 == 3 )
        {
            parseInfo_t *v35 = Com_ParseOnLine( text );
            ((int *)facePtr)[46] = (int)j__atol( v35->token );
            doSampleSize = true;
        }
        else
        {
            ((int *)facePtr)[46] = v48;
            if ( v27 <= 1 )
                Com_ParseOnLine( text );
            if ( !v27 )
            {
                Com_ParseOnLine( text );
                doSampleSize = true;
            }
            else
            {
                doSampleSize = ( v27 < 4 );   // true for v27 == 1, 2
            }
        }

        if ( doSampleSize )
        {
            parseInfo_t *v36 = Com_ParseOnLine( text );
            int v53 = (int)j__atol( v36->token );
            if ( v53 <= 0 ) v53 = 16;

            Face_SetDefaultMaterials( facePtr );   // sub_472D30
            SetMaterial( "lightmap_gray", (patchMesh_material *)((char *)facePtr + 9 * 8) );
            // IDA: v44 = (float)v53; Init_MaterialLayer(..., (MaterialDef*)LODWORD(v44)).
            // §11 COERCE: the 2nd arg is the BIT pattern of the float sample size,
            // NOT a value-cast — Init_MaterialLayer reinterprets the pointer bits as float.
            float fv53 = (float)v53;
            Init_MaterialLayer( (MaterialDef *)facePtr + 2,
                                (MaterialDef *)(uintptr_t)*(unsigned int *)&fv53 );
        }
        else
        {
            // version >= 4: read up to 3 surface defs (radiosity / lightmap / smoothing)
            int chanIdx = 0;
            MaterialDef *mdPtr = (MaterialDef *)((char *)facePtr + 36);
            while ( Face_ParseSurfDef( text, mdPtr, version, chanIdx ) )
            {
                ++chanIdx;
                ++mdPtr;
                if ( chanIdx >= 3 )
                    break;
            }
            if ( chanIdx < 3 )   // a surf def failed to parse before reaching 3 → error
                return nullptr;
        }

        v16 = nextCount;
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  14. Brush_Write  (0x474e90)
//  Writes a brush to the map output stream.
// ════════════════════════════════════════════════════════════════════════════
// IDA: the writer is a void(***)(int ctx, const char* fmt, ...) where the ctx
// is the pointer itself cast to int.  WriteWriter_t = the triple-pointer handle.
typedef int WriteFunc_t( int ctx, const char *fmt, ... );
typedef WriteFunc_t **WriteWriter_t;
// Convenience: WRITE(w, fmt, ...) = (**w)((int)(intptr_t)w, fmt, ...)
#define WRITE(w, ...) ((**w)((int)(intptr_t)(w), __VA_ARGS__))

// Write sub-function forward declarations (defined as static below)
static int Brush_WritePhysicsBox( brush_t *b, WriteWriter_t writer );
static int Brush_WritePhysCylinder( brush_t *b, WriteWriter_t writer );

// MapLoad helpers needed by Brush_Write
extern int  Patch_Write( WriteWriter_t writer, patchMesh_t *patch );   // pmesh.cpp 0x4458f0
extern void MapLoad_ParseBrush_Layer( WriteWriter_t writer, int layerStr );
extern void MapLoad_ParseBrush_Content( int name, WriteWriter_t writer,
                                         int val, void *table );
extern int *contents_table;
extern int *toolflags_table;

// VC7.1 / pre-UCRT printf vs modern UCRT differ in TWO ways that break byte-exact
// round-trips of stock .map files (written by the VC7.1 CoD4Radiant):
//   (1) EXPONENT WIDTH — VC7.1 emits a MINIMUM 3-digit exponent ("7.1e-005"); UCRT
//       emits 2 ("7.1e-05") per C99.
//   (2) HALF ROUNDING — VC7.1's "%.8g" rounds ties AWAY from zero; UCRT rounds ties
//       to EVEN. A float that lands exactly on an 8th-significant-figure half breaks:
//       e.g. the float 2845.15625 ⇒ "-2845.1563" (VC7.1) vs "-2845.1562" (UCRT).
// Fmt8g reproduces VC7.1: detect the exact tie, round the magnitude up, then pad the
// exponent. Non-tie values are byte-identical to plain "%.8g", so only the handful of
// exact-half coordinates change. (Non-static — pmesh.cpp's patch writer shares it.)
const char *Fmt8g( char *buf, double v )
{
    sprintf( buf, "%.8g", v );

    if ( v != 0.0 )
    {
        double av = ( v < 0.0 ) ? -v : v;
        char   ebuf[32];
        sprintf( ebuf, "%.7e", av );                 // reliable base-10 exponent ('e' present
        char  *ec = strchr( ebuf, 'e' );             //  only for finite values — guards inf/nan)
        if ( ec )
        {
            int e        = atoi( ec + 1 );
            int decimals = 7 - e;                     // %g fixed-notation decimals for 8 sig figs
            if ( decimals >= 0 && decimals <= 18 )
            {
                double scale  = pow( 10.0, (double)decimals );
                double scaled = av * scale;
                double fl     = floor( scaled );
                if ( scaled - fl == 0.5 )             // exact tie at the 8th significant figure
                {
                    double awayMag = ( fl + 1.0 ) / scale;
                    sprintf( buf, "%.8g", ( v < 0.0 ) ? -awayMag : awayMag );
                }
            }
        }
    }

    char *e = strchr( buf, 'e' );
    if ( e && ( e[1] == '+' || e[1] == '-' ) )
    {
        int  expval = atoi( e + 1 );                 // signed exponent
        char sign   = ( expval < 0 ) ? '-' : '+';
        int  mag    = ( expval < 0 ) ? -expval : expval;
        sprintf( e + 1, "%c%03d", sign, mag );       // e-05 -> e-005 ; e-100 stays e-100
    }
    return buf;
}

// 0x474e90 — Fmt8g(%.8g) over 9 planepts + 6 texdef floats; MaterialDef.cpp:85
// MtlDef_IsValid KEEP_VERBOSE (CROSS_FILE inlined helper).
int Brush_Write( WriteWriter_t writer, brush_t *brush )
{
    if ( brush->patch )
        return Patch_Write( writer, brush->patch );

    int tot = brush->numberId;
    if ( tot == 2 )
        return Brush_WritePhysCylinder( brush, writer );
    if ( tot == 1 )
        return Brush_WritePhysicsBox( brush, writer );

    // Normal brush
    WRITE( writer,"{\n" );
    // NOTE (lighting2): faithful to the binary (0x474e90), which strcmp's parent_layer_string
    // with no guard — it relies on the invariant that every active brush has a non-NULL layer.
    // A kisak save-path UAF can violate that (a freed+reallocated brush def reaches here
    // during a save), but the right fix is the upstream UAF, not a guard here.
    if ( strcmp( brush->parent_layer_string, "000_Global" ) )
        MapLoad_ParseBrush_Layer( writer, (int)brush->parent_layer_string );

    MapLoad_ParseBrush_Content( (int)"contents",  writer,
                                 brush->faces->contents,  contents_table );
    MapLoad_ParseBrush_Content( (int)"toolFlags",  writer,
                                 brush->faces->toolflags, toolflags_table );

    for ( int fi = 0; fi < brush->faceCount; ++fi )
    {
        face_t *f = &brush->faces[fi];
        float *pp = f->planepts[0];
        for ( int pt = 0; pt < 3; ++pt )
        {
            WRITE( writer," (" );
            for ( int c = 0; c < 3; ++c )
            {
                char nb[40];
                WRITE( writer," %s", Fmt8g( nb, *pp++ ) );
            }
            WRITE( writer," )" );
        }
        for ( int ch = 0; ch < 3; ++ch )
        {
            // IDA: v9 = (MaterialDef*)(&v12[9*ch+9]) — raw float-array offset
            // &v12[0] = face->planepts[0][0]. 9*ch+9 floats forward = face+4*(9*ch+9).
            // For ch=0: 4*9=36 from planepts[0][0] = mtldef[0] start (+36 from face). Correct.
            MaterialDef *md = (MaterialDef *)((float *)f + (9 * ch + 9));
            // the binary inlines Materialdef_GetName here (MaterialDef.cpp:85 lives in it)
            extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );   // materialdef.cpp 0x431640
            const char *name    = (const char *)Materialdef_GetName( md );
            char       *mtlName = (char *)name;

            float *v11 = &md->mat_texDef.size[ 7 * LayerMat::GetCurrentLayer( md ) ];
            iassert( mtlName[0] );

            if ( ch == 2 )
            {
                if ( strcmp( name, "smoothing_hard" ) )
                    WRITE( writer," smoothing %s", name );
            }
            else
            {
                char nb[6][40];
                WRITE( writer," %s %s %s %s %s %s %s",
                            mtlName,
                            Fmt8g( nb[0], v11[0] ), Fmt8g( nb[1], v11[1] ), Fmt8g( nb[2], v11[2] ),
                            Fmt8g( nb[3], v11[3] ), Fmt8g( nb[4], v11[4] ), Fmt8g( nb[5], v11[5] ) );
            }
        }
        WRITE( writer,"\n" );
    }
    return WRITE( writer,"}\n" );
}

// ════════════════════════════════════════════════════════════════════════════
//  15. Brush_WritePhysicsBox  (0x4742c0)
// ════════════════════════════════════════════════════════════════════════════
// 1995/2001/2007/2008/2009 converted: the binary's axis[3][3] box frame is assembled
// alongside the port's scalar math so the Vec3Dot/I_fabs condition strings compile 1:1.
static int Brush_WritePhysicsBox( brush_t *b, WriteWriter_t file )
{
    iassert( b );      // brush.cpp:1988
    iassert( file );   // brush.cpp:1989
    iassert( b->faceCount == 6 );   // brush.cpp:1990

    face_t *faces = b->faces;
    float cx = (b->mins[0] + b->maxs[0]) * 0.5f;
    float cy = (b->mins[1] + b->maxs[1]) * 0.5f;
    float cz = (b->mins[2] + b->maxs[2]) * 0.5f;

    // axis[2] = faces[1].plane.normal
    float axis[3][3];                     // the binary's box frame (assert strings)
    float a2x = faces[1].plane.normal[0];
    float a2y = faces[1].plane.normal[1];
    float a2z = faces[1].plane.normal[2];
    axis[2][0] = a2x; axis[2][1] = a2y; axis[2][2] = a2z;

    iassert( Vec3Dot( axis[2], b->faces[0].plane.normal ) < -0.999f );   // brush.cpp:1995

    // Compute half-extents along axis[2] direction
    float d0 = faces[1].planepts[0][0]*a2x + faces[1].planepts[0][1]*a2y + faces[1].planepts[0][2]*a2z;
    float d1 = faces[0].planepts[0][0]*a2x + faces[0].planepts[0][1]*a2y + faces[0].planepts[0][2]*a2z;
    float halfZ = fabsf( d0 - d1 ) * 0.5f;

    // axis[1] = faces[4].plane.normal
    float a1x = faces[4].plane.normal[0];
    float a1y = faces[4].plane.normal[1];
    float a1z = faces[4].plane.normal[2];
    axis[1][0] = a1x; axis[1][1] = a1y; axis[1][2] = a1z;

    iassert( Vec3Dot( axis[1], b->faces[2].plane.normal ) < -0.999f );   // brush.cpp:2001

    float e0 = faces[4].planepts[0][0]*a1x + faces[4].planepts[0][1]*a1y + faces[4].planepts[0][2]*a1z;
    float e1 = faces[2].planepts[0][0]*a1x + faces[2].planepts[0][1]*a1y + faces[2].planepts[0][2]*a1z;
    float halfY = 0.5f * fabsf( e0 - e1 );

    // axis[0] = cross(axis[1], axis[2])
    float out[3];
    Vec3Cross( &a1x, &a2x, out );
    axis[0][0] = out[0]; axis[0][1] = out[1]; axis[0][2] = out[2];

    iassert( I_fabs( Vec3Dot( axis[0], b->faces[3].plane.normal ) ) > 0.999f );   // brush.cpp:2007
    iassert( I_fabs( Vec3Dot( axis[0], b->faces[5].plane.normal ) ) > 0.999f );   // brush.cpp:2008
    iassert( Vec3Dot( b->faces[3].plane.normal, b->faces[5].plane.normal ) < -0.999f );   // brush.cpp:2009

    float f3d = faces[3].planepts[0][0]*out[0] + faces[3].planepts[0][1]*out[1] + faces[3].planepts[0][2]*out[2];
    float f5d = faces[5].planepts[0][0]*out[0] + faces[5].planepts[0][1]*out[1] + faces[5].planepts[0][2]*out[2];
    float halfX = fabsf( f3d - f5d ) * 0.5f;

    WRITE( file,"  {\n    physics_box\n    {\n" );
    WRITE( file,"      %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                out[0], out[1], out[2],
                a1x, a1y, a1z,
                a2x, a2y, a2z,
                cx, cy, cz,
                halfX, halfY, halfZ );
    return WRITE( file,"    }\n  }\n" );
}

// ════════════════════════════════════════════════════════════════════════════
//  16. Brush_WritePhysCylinder  (0x474800)
//  Writes a physics_cylinder brush.  Recovers the cylinder axis/center/length/radius
//  from the N-sided prism's faces: faces[0]/faces[1] are the two end caps, faces[2..]
//  the side faces.  Transcribed from the disasm (0x474800); the binary's float-index
//  addressing (ownerNext[48..50] = faces[0].plane.normal, etc.) is decoded against
//  face_t (planepts @ +0, plane.normal @ +0xC0 = float 48..50, stride 232).  The
//  per-iteration unrolled radius loop in the binary is pure optimisation — written
//  here as one clean per-side-face loop (identical math).
//  KEY FIX over the prior placeholder port: the radius divisor is (faceCount - 2)
//  (the side-face count), not faceCount — the binary divides v38 by
//  ((uint)&v17[-1].xx10 + 2) = faceCount - 2 (0x474df7 `fild var_1C`).
// ════════════════════════════════════════════════════════════════════════════
static int Brush_WritePhysCylinder( brush_t *b, WriteWriter_t file )
{
    iassert( b );      // brush.cpp:2038
    iassert( file );   // brush.cpp:2039
    iassert( b->faceCount > 3 );   // brush.cpp:2040

    face_t *faces     = b->faces;
    int     faceCount = b->faceCount;

    // axis = faces[0].plane.normal (the cap normal = the cylinder axis direction).
    const float *axis = faces[0].plane.normal;

    // capDist0 (IDA v45) = faces[1].planepts[0] · axis ; capDist1 (IDA v46) =
    // faces[0].planepts[0] · axis.  length = |capDist1 - capDist0|.
    float capDist0 = faces[1].planepts[0][0]*axis[0] + faces[1].planepts[0][1]*axis[1]
                   + faces[1].planepts[0][2]*axis[2];
    float capDist1 = faces[0].planepts[0][0]*axis[0] + faces[0].planepts[0][1]*axis[1]
                   + faces[0].planepts[0][2]*axis[2];
    float length = fabsf( capDist1 - capDist0 );

    // Accumulate the (projected-into-faces[0]'s-plane) midpoints of every distinct
    // pair of side faces (a = faces[i-1], b = faces[j], j = i..end) to find the
    // cylinder's center in the cap plane.
    float cx = 0.0f, cy = 0.0f, cz = 0.0f;
    unsigned int edgeCount = 0;

    for ( int i = 3; i - 1 < faceCount; ++i )
    {
        face_t *fa = &faces[i - 1];                 // base face
        for ( int j = i; j < faceCount; ++j )
        {
            face_t *fb = &faces[j];                 // partner face
            float dot = fa->plane.normal[1]*fb->plane.normal[1]
                      + fa->plane.normal[0]*fb->plane.normal[0]
                      + fa->plane.normal[2]*fb->plane.normal[2];

            float mid[3];
            if ( dot <= 0.9998999834060669f && dot >= -0.9998999834060669f )
            {
                // Non-parallel: solve for the intersection parameter t along fa's
                // normal, then mid = fa.planepts[1] + fa.normal * t.
                float t = 0.0f, tUnused = 0.0f;
                ClosestApproachOfTwoLines( fa->planepts[0], fa->plane.normal,
                                           fb->planepts[0], fb->plane.normal,
                                           &t, &tUnused );
                mid[0] = fa->plane.normal[0]*t + fa->planepts[1][0];
                mid[1] = fa->plane.normal[1]*t + fa->planepts[1][1];
                mid[2] = t*fa->plane.normal[2] + fa->planepts[1][2];
            }
            else
            {
                // Parallel: midpoint of the two faces' planepts[0].
                mid[0] = (fa->planepts[0][0] + fb->planepts[0][0]) * 0.5f;
                mid[1] = (fa->planepts[0][1] + fb->planepts[0][1]) * 0.5f;
                mid[2] = (fa->planepts[0][2] + fb->planepts[0][2]) * 0.5f;
            }
            ++edgeCount;

            // Project the midpoint into faces[0]'s plane (remove the axis component).
            float proj = axis[0]*mid[0] + axis[1]*mid[1] + axis[2]*mid[2];
            float neg  = -proj;
            cx += mid[0] + axis[0]*neg;
            cy += mid[1] + axis[1]*neg;
            cz += mid[2] + axis[2]*neg;
        }
    }

    float inv = 1.0f / (float)edgeCount;
    float centerX = inv * cx;
    float centerY = cy * inv;
    float centerZ = inv * cz;

    // Lift the center back along the axis to the cylinder mid-plane.
    float axisOff = 0.5f * length + capDist0;
    centerX += axis[0] * axisOff;
    centerY += axis[1] * axisOff;
    centerZ += axis[2] * axisOff;

    // Radius = mean signed distance from the center to each side-face plane.
    float radiusSum = 0.0f;
    for ( int f = 2; f < faceCount; ++f )
    {
        const float *n  = faces[f].plane.normal;
        const float *p0 = faces[f].planepts[0];
        float planeDist  = p0[0]*n[0] + p0[1]*n[1] + p0[2]*n[2];
        float centerDist = n[0]*centerX + n[1]*centerY + n[2]*centerZ;
        radiusSum += planeDist - centerDist;
    }
    float radius = radiusSum / (float)( faceCount - 2 );

    WRITE( file,"  {\n    physics_cylinder\n    {\n" );
    WRITE( file,"      %f %f %f %f %f %f %f %f\n",
                axis[0], axis[1], axis[2],
                centerX, centerY, centerZ,
                length, radius );
    return WRITE( file,"    }\n  }\n" );
}

// ════════════════════════════════════════════════════════════════════════════
//  Chunk C — geometry (vertex/face topology): Brush_Convex / Brush_SplitBrushByFace /
//  Brush_MoveVertex, plus the brush.cpp deps Face_MakePlane /
//  Brush_RemoveEmptyFaces01 / Brush_RemoveEmptyFaces02.
// ════════════════════════════════════════════════════════════════════════════

extern unsigned int Entity_ColorSth( brush_t *b );   // 0x475110 (defined below, brush.cpp:2167)
extern float Vec3Normalize_R( float *v );    // 0x40A5E0 (real impl in engine_stubs.cpp)

// ─── Face_MakePlane  (0x470470) ─────────────────────────────────────────────
// Recomputes face->plane from its 3 planepts, normalises, sets face->unk03 (the
// major-axis index 0..3) and returns it.
// plane.unk@0xD4 has no xrefs; its readers reconstitute (double)dist.
// KISAK: the no-normal warning uses a VectorCompare epsilon, not an exact ==0.0f
// (diagnostic only).
int Face_MakePlane( face_t *face )
{
    float e1[3], e2[3];
    e1[0] = face->planepts[0][0] - face->planepts[1][0];
    e1[1] = face->planepts[0][1] - face->planepts[1][1];
    e1[2] = face->planepts[0][2] - face->planepts[1][2];
    e2[0] = face->planepts[2][0] - face->planepts[1][0];
    e2[1] = face->planepts[2][1] - face->planepts[1][1];
    e2[2] = face->planepts[2][2] - face->planepts[1][2];
    Vec3Cross( e1, e2, face->plane.normal );
    // IDA 0x4704c9 calls VectorCompare(face->plane.normal, vec3_origin) — an EPSILON test
    // (component squared > ~1e-6 => not degenerate), NOT an exact ==0 check. A tiny-but-nonzero
    // cross product warns in the binary but not under exact ==0.0f. (diagnostic-only)
    bool degenerate = true;
    for ( int i = 0; i < 3; ++i )
        if ( (double)face->plane.normal[i] * (double)face->plane.normal[i] > 0.00000100000011116208 )
        { degenerate = false; break; }
    if ( degenerate )
        printf( "WARNING: brush plane with no normal\n" );
    Vec3Normalize_R( face->plane.normal );
    // §11: IDA's *(double*)&face->plane.dist is an FPU widen of a float store — keep float.
    face->plane.dist = face->planepts[1][1] * face->plane.normal[1]
                     + face->plane.normal[0] * face->planepts[1][0]
                     + face->planepts[1][2] * face->plane.normal[2];
    int axis;
    if ( face->plane.normal[0] == 1.0f )
        axis = 0;
    else if ( face->plane.normal[1] == 1.0f )
        axis = 1;
    else
    {
        axis = 2;
        if ( face->plane.normal[2] != 1.0f )
            axis = 3;
    }
    face->unk03 = axis;
    return axis;
}

// ─── Brush_RemoveEmptyFaces01  (0x471720) ───────────────────────────────────
// Removes faces with a null winding. Brush_RemoveFace shifts the array down, so
// the index is not advanced on removal; faceCount is re-read each iteration.
void Brush_RemoveEmptyFaces01( brush_t *b )
{
    unsigned int i = 0;
    while ( i < (unsigned int)b->faceCount )
    {
        if ( b->faces[i].w )
            ++i;
        else
            Brush_RemoveFace( b, i );
    }
}

// ─── Brush_RemoveEmptyFaces02  (0x471850) ───────────────────────────────────
// Removes coincident faces: Brush_FaceIndexCmp(j,b,i)==0 means face[j] has the
// same plane as face[i] (a duplicate) → drop it.  The outer i advances every iteration
// (separate j local); faceCount@0x40 is unsigned.
bool Brush_RemoveEmptyFaces02( brush_t *b )
{
    bool changed = false;
    for ( unsigned int i = 0; i + 1 < (unsigned int)b->faceCount; ++i )
    {
        unsigned int j = i + 1;
        while ( j < (unsigned int)b->faceCount )
        {
            if ( Brush_FaceIndexCmp( j, b, i ) )   // nonzero = differ → keep
            {
                ++j;
            }
            else
            {
                Brush_RemoveFace( b, j );          // identical plane → drop dup
                changed = true;
            }
        }
    }
    return changed;
}

// ─── Brush_Convex  (0x471B60) ───────────────────────────────────────────────
// True if no ordered pair of face windings is concave w.r.t. the other's plane.
// IDA passes Winding_PlanesConcave args positionally as
// (w_i, w_j, normal_j, normal_i, dist_i, dist_j) — transcribed verbatim.
bool Brush_Convex( brush_t *b )
{
    unsigned int faceCount = (unsigned int)b->faceCount;
    if ( !faceCount )
        return true;
    for ( unsigned int i = 0; i < faceCount; ++i )
    {
        winding_t *wi = b->faces[i].w;
        if ( !wi )
            continue;
        for ( unsigned int j = 0; j < faceCount; ++j )
        {
            if ( i == j )
                continue;
            winding_t *wj = b->faces[j].w;
            if ( !wj )
                continue;
            if ( Winding_PlanesConcave( wi, wj,
                                        b->faces[j].plane.normal,
                                        b->faces[i].plane.normal,
                                        b->faces[i].plane.dist,
                                        b->faces[j].plane.dist ) )
                return false;
        }
    }
    return true;
}

// ─── Brush_SplitBrushByFace  (0x471960) ─────────────────────
// Clones `in` twice and clips each by `face`: clone 1 keeps the face as-is → *back;
// clone 2 gets the face reversed (planepts[0]<->[1], flipping the plane) → *front.
// A clone survives only if it still has >= 4 faces after dropping empty/coincident
// ones, else it is freed and the out-pointer nulled.
//
// The binary inlines entity.cpp's def-side Entity_LinkBrush (0x484FC0) here — a
// DIFFERENT function from this file's static instance-side Entity_LinkBrush
// (0x475730), hence the block-scope extern overload. (clone->owner = the ENTITY,
// not its def — the prior inline set it to def, a §11 owner-vs-def confusion that
// crashed once the clipper actually ran this path.)
static void Split_LinkCloneToEntity( brush_t *clone, entity_s *ownerEntity )
{
    extern void Entity_LinkBrush( brush_t *b, entity_s *world_ent );   // entity.cpp 0x484fc0
    Entity_LinkBrush( clone, ownerEntity );
}

void Brush_SplitBrushByFace( brush_t *in, face_t *face, brush_t **front, brush_t **back )
{
    // ── back half: clone clipped by face as-is ──────────────────────────────
    brush_t *b = Brush_Clone( in );          // Brush_Clone takes the DEF directly
    iassert( b );   // brush.cpp:2354
    if ( b->parent_layer_string )
        j__free_0( b->parent_layer_string );
    b->parent_layer_string = AllocMaterialString( in->parent_layer_string );
    face_t *nf = Face_Alloc( b, face );
    Byte4PackPixelColor( const_cast<float*>(colorWhite), (GfxColor *)((char *)nf + 228) );
    Brush_BuildWindings( b, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++b->version;
    Brush_RemoveEmptyFaces01( b );
    Brush_RemoveEmptyFaces02( b );
    if ( (unsigned int)b->faceCount >= 4u )
    {
        Split_LinkCloneToEntity( b, in->owner );
        *back = b;
    }
    else
    {
        Brush_Free_R( b );
        *back = nullptr;
    }

    // ── front half: clone clipped by the reversed face ──────────────────────
    b = Brush_Clone( in );          // reuse `b` (the binary uses a single var; matches the 2354 "b" assert)
    iassert( b );   // brush.cpp:2354
    if ( b->parent_layer_string )
        j__free_0( b->parent_layer_string );
    b->parent_layer_string = AllocMaterialString( in->parent_layer_string );
    face_t *nf2 = Face_Alloc( b, face );
    // Reverse the new face: swap planepts[0] <-> planepts[1] (flips the plane normal).
    float t0 = nf2->planepts[0][0], t1 = nf2->planepts[0][1], t2 = nf2->planepts[0][2];
    nf2->planepts[0][0] = nf2->planepts[1][0];
    nf2->planepts[0][1] = nf2->planepts[1][1];
    nf2->planepts[0][2] = nf2->planepts[1][2];
    nf2->planepts[1][0] = t0;
    nf2->planepts[1][1] = t1;
    nf2->planepts[1][2] = t2;
    Byte4PackPixelColor( const_cast<float*>(colorWhite), (GfxColor *)((char *)nf2 + 228) );
    Brush_BuildWindings( b, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++b->version;
    Brush_RemoveEmptyFaces01( b );
    Brush_RemoveEmptyFaces02( b );
    if ( (unsigned int)b->faceCount >= 4u )
    {
        Split_LinkCloneToEntity( b, in->owner );
        *front = b;
    }
    else
    {
        Brush_Free_R( b );
        *front = nullptr;
    }
}

// ─── Brush_MakeFaceWinding  (0x471260)  ──────────────────────────────────────
// Build the polygon for face `f` of brush `def` by starting from the face plane's
// base winding (clipped to the brush AABB ±1) and clipping it behind every OTHER
// face plane of the brush. Returns the winding (caller owns it) or NULL if the
// face is a duplicate plane / degenerate ("unused plane").
//
// This is the per-face winding builder SetupVertexSelection uses (via MakeFace) to
// recover the brush's corner vertices. The map-load/edit path's Brush_BuildWindings
// computes the same convex face polygon through CM_BuildBrushWindingForSide; this
// is the polylib path the vertex-selection code uses directly.
//
// IDA __usercall arg order recovered from the disasm of the MakeFace call site
// (edi=face, esi=def): Brush_MakeFaceWinding(a1@<edi>=face, a2@<esi>=def). The
// face plane.dist is read FPU-widened (*(double*)&plane.dist) — a §11 float widen,
// transcribed as float. The in-place clip is the polylib ClipWindingEpsilon
// (0x4d83b0); reproduced here via the ported split Winding_Clip keeping the BACK
// (inside-the-brush) half and freeing the input each pass.
extern winding_t *Winding_BaseForPlane( vec3_t maxs, vec3_t mins, plane_t *plane );  // winding.cpp 0x4d7bc0
extern int        Winding_Clip( winding_t *in, vec3_t normal, double dist, float epsilon,
                                winding_t **front, winding_t **back );               // winding.cpp 0x462860 (returns side code; ignored here)
extern void       Winding_Free( winding_t *w );                                      // winding.cpp

// 0x471260 — a1=face, a2=def.  Split Winding_Clip + 2 Winding_Free is equivalent to the
// in-place negated-plane Winding_Clip_real_ (identical g_windingAlloc delta); plane.dist
// narrows to float; constants 0.1/0.999/0.01.
winding_t *Brush_MakeFaceWinding( face_t *f, brush_t *def )
{
    // Base winding from the face plane, clipped to the brush AABB expanded ±1.
    vec3_t boundsMin = { def->mins[0] - 1.0f, def->mins[1] - 1.0f, def->mins[2] - 1.0f };
    vec3_t boundsMax = { def->maxs[0] + 1.0f, def->maxs[1] + 1.0f, def->maxs[2] + 1.0f };
    plane_t fplane;
    fplane.normal[0] = f->plane.normal[0];
    fplane.normal[1] = f->plane.normal[1];
    fplane.normal[2] = f->plane.normal[2];
    fplane.type      = f->plane.type;
    fplane.dist      = f->plane.dist;
    fplane.unk       = f->plane.unk;

    winding_t *w = Winding_BaseForPlane( boundsMax, boundsMin, &fplane );

    bool samePlaneSeen = false;
    for ( int i = 0; i < def->faceCount; ++i )
    {
        if ( !w )
            break;
        face_t *of = &def->faces[i];
        if ( of == f )
        {
            samePlaneSeen = true;
            continue;
        }
        // coplanar with the face? (dot of normals near 1 and dist near-equal)
        float ndot = f->plane.normal[0] * of->plane.normal[0]
                   + f->plane.normal[1] * of->plane.normal[1]
                   + f->plane.normal[2] * of->plane.normal[2];
        if ( ndot <= 0.99900001f || fabs( (double)f->plane.dist - (double)of->plane.dist ) >= 0.0099999998f )
        {
            // not coplanar — clip the running winding behind this face plane (keep
            // the BACK half = inside the brush), discard the front half.
            vec3_t n = { of->plane.normal[0], of->plane.normal[1], of->plane.normal[2] };
            winding_t *front = nullptr, *back = nullptr;
            Winding_Clip( w, n, of->plane.dist, 0.1f, &front, &back );
            if ( front ) Winding_Free( front );
            Winding_Free( w );
            w = back;
            if ( !w )
                return nullptr;
        }
        else if ( samePlaneSeen )
        {
            // a second face on the SAME plane as `f` — this face is the duplicate.
            Winding_Free( w );
            return nullptr;
        }
    }

    if ( !w )
    {
        Com_PrintMessage( "unused plane\n" );
        return nullptr;
    }
    if ( (unsigned int)w->numpoints < 3 )
    {
        Winding_Free( w );
        Com_PrintMessage( "unused plane\n" );
        return nullptr;
    }
    return w;
}

// ─── Brush_MoveVertex  (0x471C30) ────────────────────────
// Drags the brush vertex at `move_points` by `delta`, grid-snaps it to `end`,
// re-triangulates the faces sharing the vertex (collapse/split as needed), clamps
// the segment start->end against neighbour planes, and accepts the move only if
// the brush stays convex (else reverts and returns 0). Returns 1 if applied.
//
// Real signature (IDA __usercall, delta@eax): (delta, brush, move_points, end).
// There is no bSnap arg — the binary always grid-snaps (its do/while is always-true).
//
// §11 traps honoured: face stride 232; plane.dist read/written as float (the IDA
// *(double*)&dist are FPU float widens); the winding template is 0x28 =
// {int numpoints; float p[3][3]}; the IDA `movefacepoints[faceCount+64] = *v117`
// aliasing write is the adjacent originalFaceIndex[faceCount-1] = originalFaceIndex[fi]
// (per the roadmap); Plane_FromPoints/Winding_PlanesConcave arg orders verified
// against the binary (the ported helpers reorder vs IDA — see their notes).
//
// NOT runtime-verified: vertex dragging is a UI/drag.cpp path with no
// Gate-P4 harness. Transcribed from the 0x471C30 decompile; verify when a drag
// harness or the editor UI (Gate P5) exists.
static winding_t *MV_NewWindingFromTemplate( face_t *f, const void *tmpl28 )
{
    iassert( f );   // IDA brush.cpp:83 "f" — inlined winding-alloc face null-check
    if ( f->w ) { --g_windingAlloc; free( f->w ); f->w = nullptr; }
    ++g_windingAlloc;
    winding_t *w = (winding_t *)malloc( 0x28u );
    if ( !w )
        Com_PrintMessage( "out of memory: winding_t\n" );
    w->numpoints = 0;
    memcpy( w, tmpl28, 0x28u );
    f->w = w;
    return w;
}

int Brush_MoveVertex( vec3_t delta, brush_t *b, vec3_t move_points, vec3_t end )
{
    struct WTmpl { int numpoints; float p[3][3]; };   // 0x28 winding template (IDA &v123)
    WTmpl tmpl;
    tmpl.numpoints = 3;

    float start[3];
    int   result = 1;                  // v119
    int   movefaces[64];               // v134
    int   movefacepoints[66];          // movefacepoints (paired: movefaces[k] <-> movefacepoints[k+1])
    int   originalFaceIndex[64];
    unsigned int nummovefaces;
    int   settled;                     // v104

    start[0] = move_points[0];
    start[1] = move_points[1];
    start[2] = move_points[2];

    end[0] = move_points[0] + delta[0];
    end[1] = delta[1] + move_points[1];
    end[2] = delta[2] + move_points[2];

    for ( unsigned int axis = 0; axis < 3; ++axis )   // grid-snap end
    {
        int gs = g_qeglobals.d_gridsize;
        double snapped = floor( end[axis] / grid_sizes[gs] + 0.5 );
        end[axis] = (float)( snapped * grid_sizes[gs] );
    }

    if ( Point_Equal( start, end, 0.30000001f ) )
        return 0;

    // Phase 1: originalFaceIndex[i]=i; abort if `end` lands on an existing vertex.
    if ( b->faceCount )
    {
        for ( unsigned int fi = 0; fi < (unsigned int)b->faceCount; ++fi )
        {
            originalFaceIndex[fi] = fi;
            winding_t *w = b->faces[fi].w;
            if ( w )
            {
                for ( unsigned int k = 0; k < (unsigned int)w->numpoints; ++k )
                {
                    if ( Point_Equal( w->p[k], end, 0.30000001f ) )
                    {
                        end[0] = move_points[0];
                        end[1] = move_points[1];
                        end[2] = move_points[2];
                        return 0;
                    }
                }
            }
        }
    }

    do  // main loop until `settled`
    {
        nummovefaces = 0;

        // Phase 2a: find/build the faces that contain `start`, splitting/collapsing as needed.
        for ( unsigned int fi = 0; fi < (unsigned int)b->faceCount; ++fi )
        {
            face_t   *fp = &b->faces[fi];
            winding_t *w = fp->w;
            if ( !w || !w->numpoints )
                continue;

            unsigned int np  = (unsigned int)w->numpoints;
            unsigned int pti = 0;
            for ( ; pti < np; ++pti )
                if ( Point_Equal( w->p[pti], start, 0.2f ) )
                    break;
            if ( pti >= np )
                continue;   // this face doesn't touch `start`

            if ( np > 3 )
            {
                float ndote = fp->plane.normal[1] * end[1]
                            + end[0] * fp->plane.normal[0]
                            + fp->plane.normal[2] * end[2];
                float behind = ndote - fp->plane.dist;   // §11 float dist
                if ( behind <= 0.1f )
                {
                    // COLLAPSE: drop the vertex from this winding, splice in a triangle face.
                    tmpl.p[0][0] = w->p[(np + pti - 1) % np][0];
                    tmpl.p[0][1] = w->p[(np + pti - 1) % np][1];
                    tmpl.p[0][2] = w->p[(np + pti - 1) % np][2];
                    tmpl.p[1][0] = w->p[pti][0];
                    tmpl.p[1][1] = w->p[pti][1];
                    tmpl.p[1][2] = w->p[pti][2];
                    tmpl.p[2][0] = w->p[(pti + 1) % np][0];
                    tmpl.p[2][1] = w->p[(pti + 1) % np][1];
                    tmpl.p[2][2] = w->p[(pti + 1) % np][2];
                    bcassert( pti, np );   // inlined Winding.cpp:72 RemovePoint check
                    unsigned int newCount = (unsigned int)--w->numpoints;
                    if ( pti < newCount )
                        memcpy( w->p[pti], w->p[pti + 1], 12 * ( newCount - pti ) );
                    face_t *newface = Face_Alloc( b, fp );
                    iassert( newface == &b->faces[b->faceCount - 1] );   // brush.cpp:1146
                    originalFaceIndex[b->faceCount - 1] = originalFaceIndex[fi];
                    MV_NewWindingFromTemplate( newface, &tmpl );
                    iassert( newface == &b->faces[b->faceCount - 1] );   // brush.cpp:1152
                    movefaces[nummovefaces] = b->faceCount - 1;
                    movefacepoints[nummovefaces + 1] = 1;   // dragged vertex is tmpl.p[1]
                    ++nummovefaces;
                    if ( !Point_Equal( start,
                             b->faces[movefaces[nummovefaces - 1]].w->p[movefacepoints[nummovefaces]], 0.2f ) )
                        // KEEP_VERBOSE (also 1155/1166/1267): the binary string indexes
                        // movefacepoints[nummovefaces - 1]; the port reads that slot as
                        // [nummovefaces] before the -1 rebase — documented ±1 convention.
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                                1189, 1, "%s",
                                "Point_Equal( start, b->faces[movefaces[nummovefaces - 1]].w->pts[movefacepoints[nummovefaces - 1]], 0.2f )" );
                }
                else
                {
                    // SPLIT: fan out one triangle face per spanned edge, then re-triangulate
                    // the original face. `pti` is the dragged vertex; np unchanged here.
                    if ( pti < np + pti - 3 )
                    {
                        for ( unsigned int e = pti + 2; e - 2 < (unsigned int)w->numpoints + pti - 3; ++e )
                        {
                            unsigned int n2 = (unsigned int)w->numpoints;
                            tmpl.p[0][0] = w->p[pti][0];
                            tmpl.p[0][1] = w->p[pti][1];
                            tmpl.p[0][2] = w->p[pti][2];
                            tmpl.p[1][0] = w->p[(e - 1) % n2][0];
                            tmpl.p[1][1] = w->p[(e - 1) % n2][1];
                            tmpl.p[1][2] = w->p[(e - 1) % n2][2];
                            tmpl.p[2][0] = w->p[e % n2][0];
                            tmpl.p[2][1] = w->p[e % n2][1];
                            tmpl.p[2][2] = w->p[e % n2][2];
                            face_t *newface = Face_Alloc( b, fp );
                            fp = &b->faces[fi];   // re-fetch (Face_Alloc may realloc)
                            iassert( newface == &b->faces[b->faceCount - 1] );   // brush.cpp:1180
                            originalFaceIndex[b->faceCount - 1] = originalFaceIndex[fi];
                            MV_NewWindingFromTemplate( newface, &tmpl );
                            iassert( newface == &b->faces[b->faceCount - 1] );   // brush.cpp:1186
                            movefaces[nummovefaces] = b->faceCount - 1;
                            movefacepoints[nummovefaces + 1] = 0;   // dragged vertex is tmpl.p[0]
                            ++nummovefaces;
                            if ( !Point_Equal( start,
                                     b->faces[movefaces[nummovefaces - 1]].w->p[movefacepoints[nummovefaces]], 0.2f ) )
                                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                                        1155, 1, "%s",
                                        "Point_Equal( start, b->faces[movefaces[nummovefaces - 1]].w->pts[movefacepoints[nummovefaces - 1]], 0.2f )" );
                        }
                    }
                    // re-triangulate the original face (its winding becomes the last triangle)
                    unsigned int n3 = (unsigned int)w->numpoints;
                    tmpl.p[0][0] = w->p[(n3 + pti - 2) % n3][0];
                    tmpl.p[0][1] = w->p[(n3 + pti - 2) % n3][1];
                    tmpl.p[0][2] = w->p[(n3 + pti - 2) % n3][2];
                    tmpl.p[1][0] = w->p[(n3 + pti - 1) % n3][0];
                    tmpl.p[1][1] = w->p[(n3 + pti - 1) % n3][1];
                    tmpl.p[1][2] = w->p[(n3 + pti - 1) % n3][2];
                    tmpl.p[2][0] = w->p[pti][0];
                    tmpl.p[2][1] = w->p[pti][1];
                    tmpl.p[2][2] = w->p[pti][2];
                    MV_NewWindingFromTemplate( fp, &tmpl );
                    movefaces[nummovefaces] = fi;
                    movefacepoints[nummovefaces + 1] = 2;   // dragged vertex is tmpl.p[2]
                    ++nummovefaces;
                    if ( !Point_Equal( start,
                             b->faces[movefaces[nummovefaces - 1]].w->p[movefacepoints[nummovefaces]], 0.2f ) )
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                                1166, 1, "%s",
                                "Point_Equal( start, b->faces[movefaces[nummovefaces - 1]].w->pts[movefacepoints[nummovefaces - 1]], 0.2f )" );
                }
            }
            else
            {
                // numpoints <= 3: record this face as a move-face directly.
                movefacepoints[nummovefaces + 1] = pti;
                movefaces[nummovefaces] = fi;
                ++nummovefaces;
            }
        }

        // Phase 2b: clamp start->end against every non-move face plane; track the
        // smallest crossing fraction into (clampX,clampY,clampZ).
        float clampX = end[0], clampY = end[1], clampZ = end[2];
        float smallestFrac = 1.0f;
        settled = 1;
        for ( unsigned int f = 0; f < (unsigned int)b->faceCount; ++f )
        {
            if ( !nummovefaces )
                continue;
            // find the move-face slot whose originalFaceIndex maps to f
            unsigned int slot = 0;
            while ( (unsigned int)originalFaceIndex[movefaces[slot]] != f )
                if ( ++slot >= nummovefaces )
                    goto next_clamp_face;

            {
                char planeBuf[32];                       // IDA v133 (double[4]); plane via offset
                plane_t *pl = (plane_t *)planeBuf;
                // find a move-face whose movefaces[j]==f (a direct neighbour edge)
                unsigned int j = 0;
                while ( f != (unsigned int)movefaces[j] )
                {
                    if ( ++j >= nummovefaces )
                    {
                        // no direct neighbour: use the original face plane verbatim
                        memcpy( planeBuf, &b->faces[originalFaceIndex[f]].plane, sizeof(planeBuf) );
                        goto do_clamp;
                    }
                }
                {
                    int pj      = movefacepoints[j + 1];
                    winding_t *wj = b->faces[movefaces[j]].w;
                    unsigned int nj = (unsigned int)wj->numpoints;
                    float *a = wj->p[(unsigned int)(pj + 1) % nj];
                    tmpl.p[0][0] = a[0]; tmpl.p[0][1] = a[1]; tmpl.p[0][2] = a[2];
                    tmpl.p[1][0] = wj->p[(pj + 2) % nj][0];
                    tmpl.p[1][1] = wj->p[(pj + 2) % nj][1];
                    tmpl.p[1][2] = wj->p[(pj + 2) % nj][2];
                    int ps      = movefacepoints[slot + 1];
                    winding_t *ws = b->faces[movefaces[slot]].w;
                    float *c = ws->p[(unsigned int)(ps + 1) % (unsigned int)ws->numpoints];
                    tmpl.p[2][0] = c[0]; tmpl.p[2][1] = c[1]; tmpl.p[2][2] = c[2];
                    // Plane_FromPoints(p1,p2,p3,plane): IDA order maps to (tmpl.p[2],tmpl.p[1],tmpl.p[0],plane)
                    if ( !Plane_FromPoints( tmpl.p[2], tmpl.p[1], tmpl.p[0], pl ) )
                    {
                        unsigned int alt = (unsigned int)(ps + 2) % (unsigned int)ws->numpoints;
                        tmpl.p[2][0] = ws->p[alt][0];
                        tmpl.p[2][1] = ws->p[alt][1];
                        tmpl.p[2][2] = ws->p[alt][2];
                        if ( !Plane_FromPoints( tmpl.p[2], tmpl.p[1], tmpl.p[0], pl ) )
                            goto next_clamp_face;
                    }
                }
do_clamp:
                {
                    float startDist = pl->normal[0] * start[0] + pl->normal[1] * start[1]
                                    + pl->normal[2] * start[2] - pl->dist;
                    float endDist   = pl->normal[0] * end[0] + pl->normal[1] * end[1]
                                    + pl->normal[2] * end[2] - pl->dist;
                    if ( startDist < 0.0099999998f && endDist < 0.0099999998f )
                        goto next_clamp_face;   // both behind → no clamp from this plane
                    if ( startDist <= -0.0099999998f || endDist <= -0.0099999998f )
                    {
                        float denom = startDist - endDist;
                        if ( fabs( denom ) >= 0.001f )
                        {
                            float frac = startDist / denom;
                            settled = 0;
                            if ( smallestFrac > (double)frac )
                            {
                                clampX = ( end[0] - start[0] ) * frac + start[0];
                                clampY = start[1] + ( end[1] - start[1] ) * frac;
                                clampZ = start[2] + ( end[2] - start[2] ) * frac;
                                smallestFrac = startDist / denom;
                            }
                        }
                    }
                }
            }
next_clamp_face:;
        }

        // Phase 2c: write the clamped position into each move-face's dragged vertex,
        // rebuild planepts + plane; flag degenerate normals.
        for ( unsigned int i = 0; i < nummovefaces; ++i )
        {
            face_t   *fp = &b->faces[movefaces[i]];
            winding_t *w = fp->w;
            float    *vp = w->p[movefacepoints[i + 1]];
            if ( !Point_Equal( vp, start, 0.2f ) )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                        1267, 0, "%s", "Point_Equal( w->pts[movefacepoints[i]], start, 0.2f )" );
            vp[0] = clampX;
            vp[1] = clampY;
            vp[2] = clampZ;
            fp->planepts[0][0] = w->p[0][0]; fp->planepts[0][1] = w->p[0][1]; fp->planepts[0][2] = w->p[0][2];
            fp->planepts[1][0] = w->p[1][0]; fp->planepts[1][1] = w->p[1][1]; fp->planepts[1][2] = w->p[1][2];
            fp->planepts[2][0] = w->p[2][0]; fp->planepts[2][1] = w->p[2][1]; fp->planepts[2][2] = w->p[2][2];
            Face_MakePlane( fp );
            float nlen = (float)sqrt( fp->plane.normal[0] * fp->plane.normal[0]
                                    + fp->plane.normal[1] * fp->plane.normal[1]
                                    + fp->plane.normal[2] * fp->plane.normal[2] );
            if ( nlen < 0.1f )
                result = 0;
        }

        // Phase 2d: accept if still convex, else revert every move-face to `start`.
        if ( result && Brush_Convex( b ) )
        {
            start[0] = clampX;
            start[1] = clampY;
            start[2] = clampZ;
        }
        else
        {
            for ( unsigned int j = 0; j < nummovefaces; ++j )
            {
                face_t   *fp = &b->faces[movefaces[j]];
                winding_t *w = fp->w;
                float    *vp = w->p[movefacepoints[j + 1]];
                vp[0] = start[0];
                vp[1] = start[1];
                vp[2] = start[2];
                fp->planepts[0][0] = w->p[0][0]; fp->planepts[0][1] = w->p[0][1]; fp->planepts[0][2] = w->p[0][2];
                fp->planepts[1][0] = w->p[1][0]; fp->planepts[1][1] = w->p[1][1]; fp->planepts[1][2] = w->p[1][2];
                fp->planepts[2][0] = w->p[2][0]; fp->planepts[2][1] = w->p[2][1]; fp->planepts[2][2] = w->p[2][2];
                Face_MakePlane( fp );
            }
            end[0] = start[0];
            end[1] = start[1];
            end[2] = start[2];
            result  = 0;
            settled = 1;
        }

        // Phase 2e: merge coplanar adjacent faces; compact originalFaceIndex.
        for ( unsigned int faceIndex = 0; faceIndex < (unsigned int)b->faceCount; ++faceIndex )
            iassert( originalFaceIndex[faceIndex] <= faceIndex );

        {
            unsigned int i = 0, k = 0;
            while ( i < (unsigned int)b->faceCount )
            {
                unsigned int orig = (unsigned int)originalFaceIndex[i];
                if ( orig == i )
                {
                    ++i;
                    ++k;
                    continue;
                }
                face_t *fa = &b->faces[orig];
                if ( Plane_Equal( &fa->plane, &b->faces[k].plane, 0 ) )
                {
                    winding_t *merged = Winding_TryMerge( b->faces[k].w, fa->w,
                                                          fa->plane.normal, 0 );
                    if ( merged )
                    {
                        if ( fa->w ) { --g_windingAlloc; free( fa->w ); fa->w = nullptr; }
                        fa->w = merged;
                        Brush_RemoveFace( b, i );
                        if ( i >= (unsigned int)b->faceCount )
                            break;
                        unsigned int fc = (unsigned int)b->faceCount;
                        for ( unsigned int s = i; s < fc; ++s )
                        {
                            unsigned int nv = (unsigned int)originalFaceIndex[s + 1];
                            originalFaceIndex[s] = nv;
                            if ( nv >= i )
                                originalFaceIndex[s] = nv - 1;
                        }
                    }
                    else
                    {
                        ++i;
                        ++k;
                    }
                }
                else
                {
                    ++i;
                    ++k;
                }
            }
        }
    }
    while ( !settled );

    ++b->version;
    return result;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Model_SetModel  (0x478780, brush.cpp:3713) — PREFAB-LOAD-ON-MAP-OPEN.
//
//  Called by the GUI map load for every misc_prefab entity (Map_LoadFromFile ->
//  Map_LoadEntities post-process loop, gated on eclass->classtype & 0x10 =
//  CLASS_PREFAB).  Resolves the entity's "model" key into a loaded prefab .map and
//  INSTANCES that prefab's brushes into the live editor scene.
//
//  CONTROL FLOW (from the 0x478780 disasm; offsets verified against the qe3.h
//  static_asserts — entity_s.version@0x4C / modelInst@0x44 / prefab@0x48 / def@0x8 /
//  eclass@0x60 / modelClass@0x64; brush_t.modelFailed is the BYTE at brush_t+0x4C, the
//  low byte of brush_t.unk01, so it is accessed through a byte pointer):
//    1. Copy the caller's 12-float orientation matrix into a local (so the cyclePreview
//       ground-drop branch can perturb it without touching the caller's matrix).
//    2. VERSION-and-already-built gate: instance version == def version AND the instance
//       already has a modelInst or prefab -> return (asserting modelFailed is clear).
//    3. Otherwise rebuild: bail if modelFailed is already set; set modelFailed=1;
//       Entity_FreePrefab + RemoveModelInstFromBuf to tear down any stale instance.
//    4. Resolve modelClass: entityDef->modelClass if set; else Eclass_hasModel(eclass)
//       lazily loads an eclass-level model; else resolve the model NAME (the misc_prefab
//       "model" epair via ValueForKey2) and load it via Eclass_01 (-> Eclass_LoadModel
//       -> Prefab_Load -> Map_LoadEntities).
//    5. On success: Entity_RebuildBounds(entityDef) + SetupModelInst(matrix, instance).
//       SetupModelInst routes classtype&0x10 prefabs to Entity_InitPrefabInst and
//       everything else to Entity_UpdateModelInst; clears modelFailed if an instance now
//       exists.
//
//  The cyclePreview ground-drop branch (instance brushFlags & 0x100 ->
//  Trace_AllDirectionsIfFailed) is FATAL-stubbed: that flag is the model-cycling UI
//  toggle (w_cyclePreviewMode), never set during a map load.
// ─────────────────────────────────────────────────────────────────────────────
extern "C++" {
    extern char *ValueForKey2( int e, const char *key );          // entity.cpp 0x4825C0
    extern bool  Eclass_hasModel( eclass_t *e );                  // eclass.cpp 0x481740
    extern models_t *Eclass_01( const char *name, int ownerDef ); // eclass.cpp 0x482300
    extern void  Entity_FreePrefab( entity_s *e );                // entity.cpp 0x4825F0
    extern void  RemoveModelInstFromBuf( int inst );              // engine_stubs 0x4FDCE0
    extern void  SetupModelInst( float *ident_mtx, entity_s *e ); // entity.cpp 0x485890
}

// The byte at def+0x4C is modelFailed (brush_t.unk01 low byte); the set/clear/test widths
// come from Model_SetModel 0x478807/478846/4789e9/4789f2.
static inline unsigned char *Brush_ModelFailedByte( brush_t *def )
{
    return (unsigned char *)def + 0x4C;   // brush_t.unk01 low byte = modelFailed
}

// VecMultiplyAdd (0x40A5E0... 0x40A5A0) — vec3 multiply-add: out = a + scale*b.  (VectorMA is a
// deliberate compile-error in this tree, so the IDB-named helper is defined here for its sole
// radiant caller, the Model_SetModel cyclePreview ground-drop.)
// out[i] = a[i] + scale*b[i]; returns b (the Model_SetModel caller ignores it).
static void VecMultiplyAdd( const float *a, float scale, const float *b, float *out )
{
    out[0] = a[0] + scale * b[0];
    out[1] = a[1] + scale * b[1];
    out[2] = a[2] + scale * b[2];
}

// drop-to-floor ray (select.cpp 0x48DAA0; edTrace_t is defined in qe3.h).
extern edTrace_t *Trace_AllDirectionsIfFailed( float *cam_origin, edTrace_t *trace_result,
                                               float *dir, int contents );

bool Model_SetModel( entity_brush_s *b, int orientMatrix )
{
    const float *a2 = (const float *)(intptr_t)orientMatrix;   // 12-float orient matrix

    // 1. Local copy of the caller's orientation matrix (3x4 = 12 floats).
    float mtx[12];
    for ( int i = 0; i < 12; ++i )
        mtx[i] = a2[i];

    entity_s *inst = b->owner;   // entity instance (selbrush_t.owner @ +0x8)

    // 2. Version-and-already-built gate.  inst->version (short @0x4C) vs
    //    inst->def->version (short @0x78 of the def); plus modelInst(@0x44) /
    //    prefab(@0x48).  Disasm: cmp dx,[ecx+78h] / [eax+44h] / [eax+48h].
    {
        entity_s_def *instDef = (entity_s_def *)inst->def;
        if ( (short)inst->version == (short)instDef->version_prob_wrong
             && ( inst->modelInst != 0 || inst->prefab != nullptr ) )
        {
            // Up to date.
            iassert( !b->def->modelFailed );   // brush.cpp:3723
            return true;
        }
    }

    // 3. Needs rebuild.  Bail if a previous attempt already failed.
    if ( *Brush_ModelFailedByte( b->def ) )
        return false;
    *Brush_ModelFailedByte( b->def ) = 1;

    Entity_FreePrefab( inst );
    if ( inst->modelInst )
    {
        RemoveModelInstFromBuf( inst->modelInst );
        inst->modelInst = 0;
    }

    // ownerDef = b->def->owner (the def's owning entity DEF).
    iassert( b->owner->def == b->def->owner );
    entity_s_def *ownerDef = (entity_s_def *)b->def->owner;

    // 4. Resolve the model eclass node `e`.  Mirrors the IDB exactly (0x478780):
    //    e = ownerDef->modelClass; if (e) goto instance;
    //    v9 = !Eclass_hasModel(eclass);  e = eclass;   // ← eclass IS the modelClass node
    //    if (v9) { ... resolve "model" key -> e = Eclass_01(...) }
    // CRITICAL (model-render epic fix): when Eclass_hasModel(eclass) is TRUE, e is
    // the ECLASS itself — the prior port dropped this assignment, leaving e NULL
    // for a misc_model whose eclass resolved its model, so it never instanced.
    entitymodel_t *e = (entitymodel_t *) (models_t *)ownerDef->modelClass;   // entity+0x64
    if ( !e )
    {
        eclass_t *eclass = ownerDef->eclass;                  // entity+0x60
        bool eclassHasModel = Eclass_hasModel( eclass );
        e = (entitymodel_t *)eclass;                          // IDB: modelClass = ownerDef->eclass
        if ( !eclassHasModel )
        {
            // eclass has no built-in model node → resolve a per-entity model name.
            // The dword at &fixedsize (+0x8, bool + 3 pad bytes) gates non-modelled
            // classes out (misc_prefab is fixedsize → non-zero → proceeds).
            if ( !*(int *)&eclass->fixedsize )
                return *Brush_ModelFailedByte( b->def ) == 0;

            const char *modelName;
            if ( ( b->brushFlags & 0x100 ) != 0 )
            {
                // ── XMODEL cyclePreview ground-drop (model-render epic, now real).  The
                //    model-cycle UI (w_cyclePreviewMode) sets brushFlags&0x100: pick the cycle
                //    model name from the eclass's default_model_name[] char* array, trace
                //    straight down from the entity origin, and if the floor is hit, drop the
                //    orientation matrix's origin onto it (VecMultiplyAdd by the hit distance).
                modelName = eclass->cycleModelName[(unsigned short)g_qeglobals.w_cyclePreviewMode];
                float dropDir[3] = { 0.0f, 0.0f, -1.0f };
                edTrace_t tr;
                Trace_AllDirectionsIfFailed( ownerDef->origin, &tr, dropDir, 4608 );
                if ( tr.hit.brush )
                    VecMultiplyAdd( a2, tr.dist, dropDir, mtx );   // mtx.origin += dist*dir (to floor)
            }
            else
            {
                modelName = ValueForKey2( (int)(intptr_t)ownerDef, "model" );
            }

            // brush.cpp:3763 — LEVEL 1 (binary asserts modelName != NULL post-resolve).
            // Binary does NOT guard the return on !modelName; after the assert it falls
            // straight through to the !*modelName deref (latent NULL-deref in release).
            iassert( modelName );   // brush.cpp:3763
            if ( !*modelName )
                return *Brush_ModelFailedByte( b->def ) == 0;

            e = (entitymodel_t *)Eclass_01( modelName, (int)(intptr_t)ownerDef );
        }
    }

    // 5. Instance the resolved model.
    if ( e )
    {
        iassert( ownerDef->modelClass == e );

        Entity_RebuildBounds( ownerDef );
        SetupModelInst( mtx, b->owner );

        // If an instance now exists, clear modelFailed.
        if ( inst->modelInst != 0 || inst->prefab != nullptr )
            *Brush_ModelFailedByte( b->def ) = 0;

        // One-shot load trace (fires once per misc_prefab — the version gate at the
        // top short-circuits every subsequent call).  Counts the brush INSTANCES
        // linked into this prefab's active list, isolating prefab-loaded geometry
        // from any inline world brushes.  Consistent with the First-Light load trace.
        if ( inst->prefab )
        {
            extern void Radiant_FL_Log( const char *fmt, ... );
            prefab_s *pf = (prefab_s *)inst->prefab;
            int nBrush = 0;
            for ( selbrush_t *pb = pf->active_brushlist_next;
                  pb && pb != (selbrush_t *)&pf->active_brushlist;
                  pb = pb->next )
                ++nBrush;
            Radiant_FL_Log( "Model_SetModel: prefab '%s' instanced %d brush(es)",
                            ValueForKey2( (int)(intptr_t)ownerDef, "model" ), nBrush );
        }
    }

    return *Brush_ModelFailedByte( b->def ) == 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Brush_IsInSelectedList (0x478a20) — is brush instance `b` in the selected_brushes
// display list?  __usercall (b@<ecx>); a plain circular-list membership walk.  (The
// hex-rays for this is garbled — the disasm `cmp eax, ecx` loop is the real logic.)
// ─────────────────────────────────────────────────────────────────────────────
static char Brush_IsInSelectedList( selbrush_t *b )
{
    for ( selbrush_t *v = selected_brushes.next; v != &selected_brushes; v = v->next )
        if ( v == b )
            return 1;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Entity_HasRenderableModel (0x479610) — the model-presence gate DrawBrush + the
// sun-preview model shadow (SunLightPreview_DrawBrushShadow) use before instancing a
// model.  For a non-prefab (classtype & 0x10 == 0) entity, honours the View->Entities-As
// show-state (m_nEntityShowState 4096 = hide-all, 0x100 = selected-only via
// Brush_IsInSelectedList); then Model_SetModel and returns 1 iff a model/prefab instanced
// (else asserts unless modelFailed).  Faithful to 0x479610.  (Note: a1 is a brush instance;
// its def->modelFailed byte is at +0x4C = brush_t.unk01 low byte = Brush_ModelFailedByte.)
// ─────────────────────────────────────────────────────────────────────────────
char Entity_HasRenderableModel( brush_t_with_custom_def *a1, int orient )
{
    selbrush_t *b = (selbrush_t *)a1;
    entity_s_def *eDef = (entity_s_def *)b->owner->def;
    if ( ( eDef->eclass->classtype & 0x10 ) == 0 )         // not a prefab class
    {
        int show = g_PrefsDlg->m_nEntityShowState;
        if ( show == 4096 || ( ( show & 0x100 ) != 0 && !Brush_IsInSelectedList( b ) ) )
            return 0;                                      // hidden by the show-state filter
    }
    if ( !Model_SetModel( (entity_brush_s *)a1, orient ) )
    {
        iassert( b->def->modelFailed );   // brush.cpp:4037
        return 0;
    }
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Inlining artifacts (appeared in worklist only due to LTCG inlining):
// DONE: 0x41c850  CLayersDlg::AssignSelectionToLayer → Layers_AssignSelectionToLayer (layersdlg.cpp)
// DONE: 0x45e680  Undo_AddBrush → REAL in undo.cpp (~:512)
// DONE (SurfaceInspector multi-layer texmod transaction, commit 9b020fa):
//   0x4742c0  Brush_WritePhysicsBox  (sub_4742C0)
//   0x474800  Brush_WritePhysCylinder  (sub_474800)
//   0x474e90  Brush_Write

// ─────────────────────────────────────────────────────────────────────────────
// 0x47a1b0  DisassociateEntities  (brush.cpp:4329) — ported faithful.
// For each selected brush whose owner entity is not worldspawn and has the
// ScriptGroupKey epair, delete that key and re-check the entity's model/color.
// ─────────────────────────────────────────────────────────────────────────────

// Helpers defined in entity.cpp / filters.cpp / scriptgroup.cpp / select.cpp.
extern bool HasKeyValuePair( entity_s_def *e, const char *key );                   // entity.cpp 0x4838b0
extern void SetKeyValue( entity_s_def *e, const char *key, const char *value );    // entity.cpp 0x483690
extern void DeleteKey( epair_t **head, const char *key );                          // entity.cpp 0x483720
extern void Checkkey_Model( entity_s_def *e, const char *key );                    // entity.cpp 0x482f70 (Checkkey_Model_0)
extern void Checkkey_Color( entity_s_def *e, const char *key );                    // entity.cpp 0x483210
extern bool Entity_HasEpairMatch( entity_s *e, const char *key, const char *val ); // entity.cpp 0x483930
extern char FilterBrush( selbrush_t *b, int updateFilters );                       // filters.cpp 0x46a1f0
extern void ScriptGroup_Type();                                                    // scriptgroup.cpp 0x451200
extern void sub_43ECB0();                                                          // pmesh.cpp Patch_FinishCurveDrag 0x43ecb0
extern void CMainFrame_UpdatePatchToolbarButtons();                                // select.cpp 0x42aa70

void DisassociateEntities()
{
    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes;
          b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;

        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)b->owner->def;
        const char *ScriptGroupKey = g_PrefsDlg->ScriptGroupKey;
        if ( HasKeyValuePair( def, ScriptGroupKey ) )
        {
            DeleteKey( &def->epairs, ScriptGroupKey );
            Checkkey_Model( def, ScriptGroupKey );
            Checkkey_Color( def, ScriptGroupKey );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47a330  SelectedAssociated  (brush.cpp:4393)
// Grow the selection to every active brush whose owner entity shares the same
// ScriptGroupKey value as a currently-selected entity; repeat until a pass adds
// nothing.  First resets the edit mode to brush-select (closing a patch point-edit
// drag).  If ScriptGroupKey == ScriptColorTeamKey, defers to ScriptGroup_Type.
// The binary builds an MFC CString for the compared value (str_set); reproduced 1:1
// with a real CString, exactly as Prefab_Load / Select_Move do.
// ─────────────────────────────────────────────────────────────────────────────
void SelectedAssociated()
{
    CString matchValue;   // IDA: the str_set-built CString holding the compared value

    if ( selected_brushes.next == &selected_brushes )
        return;

    select_t prevMode = g_qeglobals.d_select_mode;
    g_qeglobals.d_select_mode = sel_brush;
    if ( prevMode == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prevMode == sel_addpoint )
        sub_43ECB0();                                    // Patch_FinishCurveDrag

    if ( !strcmp( g_PrefsDlg->ScriptGroupKey, g_PrefsDlg->ScriptColorTeamKey ) )
    {
        ScriptGroup_Type();
        return;
    }

    bool added;
    do
    {
        added = false;
        for ( selbrush_t *brush = selected_brushes.next; brush != &selected_brushes; brush = brush->next )
        {
            iassert( brush->def->owner == brush->owner->def );

            entity_s *owner = brush->def->owner;
            const char *ScriptGroupKey = g_PrefsDlg->ScriptGroupKey;
            if ( !HasKeyValuePair( owner, ScriptGroupKey ) )
                continue;

            // The entity's ScriptGroupKey value (empty string — the IDB `zero` global —
            // when the key is absent or its value is null).
            const char *value = "";
            for ( epair_t *ep = owner->epairs; ep; ep = ep->next )
            {
                if ( !_stricmp( ep->key, ScriptGroupKey ) )
                {
                    if ( ep->value )
                        value = ep->value;
                    break;
                }
            }
            matchValue = value;

            // Pull every active brush whose owner shares this value into the selection.
            // matchNext is captured BEFORE Brush_RemoveFromList/Brush_AddToList2 relink
            // `match` out of active_brushes (the binary saves &match->next->prev first).
            for ( selbrush_t *match = active_brushes.next; match != &active_brushes; )
            {
                selbrush_t *matchNext = match->next;
                entity_s *matchOwner = match->owner;
                if ( matchOwner && matchOwner != world_entity
                     && !FilterBrush( match, 0 )
                     && ( match->brushFlags & 0x20 ) == 0 )
                {
                    iassert( match->def->owner == match->owner->def );

                    if ( Entity_HasEpairMatch( (entity_s *)match->owner->def,
                                               g_PrefsDlg->ScriptGroupKey, matchValue ) )
                    {
                        Brush_RemoveFromList( match );
                        Brush_AddToList2( match );
                        added = true;
                    }
                }
                match = matchNext;
            }
        }
    } while ( added );

    g_nUpdateBits = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47a600  OverbrightShift  (brush.cpp:4444)
// Per selected fixed-size entity: a "light" nudges "overbrightShift" by a1 (clamped
// 0..1); anything else scales "fixedNodeSafeRadius" down by a1*320 (clamped >= 0).
// ─────────────────────────────────────────────────────────────────────────────
void OverbrightShift( float a1 )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)b->owner->def;
        eclass_t *eclass = def->eclass;
        if ( !*(int *)&eclass->fixedsize )
            continue;

        char buf[32];
        if ( !_stricmp( eclass->name, "light" ) )
        {
            float v = Entity_GetFloatValueForKey( (int)(intptr_t)def, "overbrightShift" ) + a1;
            if ( v > 1.0f )        v = 1.0f;
            else if ( v < 0.0f )   v = 0.0f;
            sprintf( buf, "%f", v );
            SetKeyValue( def, "overbrightShift", buf );
        }
        else
        {
            float v = Entity_GetFloatValueForKey( (int)(intptr_t)def, "fixedNodeSafeRadius" );
            if ( v == 0.0f )   v = 256.0f;
            v = v - a1 * 320.0f;
            if ( v < 0.0f )   v = 0.0f;
            sprintf( buf, "%f", v );
            SetKeyValue( def, "fixedNodeSafeRadius", buf );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47a790  Light_01  (brush.cpp:4484)
// Per selected fixed-size entity: a "light" scales its "light" value by a1 (default
// 300 if 0); anything else scales "radius" by a1 (default 256 if 0).  Both clamped >= 0.
// ─────────────────────────────────────────────────────────────────────────────
void Light_01( float a1 )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)b->owner->def;
        eclass_t *eclass = def->eclass;
        if ( !*(int *)&eclass->fixedsize )
            continue;

        char buf[32];
        if ( !_stricmp( eclass->name, "light" ) )
        {
            float v = Entity_GetFloatValueForKey( (int)(intptr_t)def, "light" );
            if ( v == 0.0f )   v = 300.0f;
            v = v * a1;
            if ( v < 0.0f )   v = 0.0f;
            sprintf( buf, "%f", v );
            SetKeyValue( def, "light", buf );
        }
        else
        {
            float v = Entity_GetFloatValueForKey( (int)(intptr_t)def, "radius" );
            if ( v == 0.0f )   v = 256.0f;
            v = v * a1;
            if ( v < 0.0f )   v = 0.0f;
            sprintf( buf, "%f", v );
            SetKeyValue( def, "radius", buf );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47a910  Light_Height  (brush.cpp:4523)
// Per selected fixed-size entity whose eclass classtype bit 0x40 is set, scale the
// "height" key by a1 (default 128 if 0), clamped >= 0.
// ─────────────────────────────────────────────────────────────────────────────
void Light_Height( float a1 )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)b->owner->def;
        eclass_t *eclass = def->eclass;
        if ( !*(int *)&eclass->fixedsize )
            continue;
        if ( ( eclass->classtype & 0x40 ) == 0 )
            continue;

        float v = Entity_GetFloatValueForKey( (int)(intptr_t)def, "height" );
        if ( v == 0.0f )   v = 128.0f;
        v = v * a1;
        if ( v < 0.0f )   v = 0.0f;
        char buf[32];
        sprintf( buf, "%f", v );
        SetKeyValue( def, "height", buf );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x47B380  Brush_DrawSubmitFaceWindings — light-region face-winding submit.
//  Faithful to disasm 0x47B380.  `inst` is a selbrush_t INSTANCE (the IDA
//  entity_brush_s type with hex-rays mins/maxs misnamed — disasm proves +0x14=def,
//  +0x18=faceCount, +0x20=patch).  `a3` = the light descriptor (float[8]+class via
//  &v19 in sub_406CE0): a3[0]=class, a3+8=cone center, a3+16=cone dir, a3[28]=cosFov,
//  a3[32]=radius.  `outList` = the region face-list head.
// ═════════════════════════════════════════════════════════════════════════════
#include "primarylights_region.h"
#include <universal/com_math.h>           // CullBoxFromConicSectionOfSphere
extern void  VectorRotateByAxis( float *out, const float *axisMatrix, const float *dir ); // draw.cpp 0x4ba6b0
extern void  MaterialDef_02( MaterialDef *m, int (*cb)( qtexture_s * ) );  // materialdef.cpp 0x431520
extern int   MaterialDef_06( qtexture_s *radMtl );                         // materialdef.cpp 0x4318A0
extern int   dword_181F51C;                                               // engine_stubs.cpp (realize-state)
extern bool  MaterialDef_10_LayeredMatHandle( MaterialDef *mtlDef );       // materialdef.cpp 0x431A60
extern char  MtlDef_IsFaceFiltered( MaterialDef *mtlDef );                 // mayaexport.cpp 0x46FBE0
extern void  PMESH_29_Winding( int patchInst, const orientation_t *orient, float *desc, rface_t **outList ); // pmesh.cpp 0x441AD0

// sub_409F20 (0x409F20) — midpoint of two vec3.
static void Region_BoundsMidpoint( const float *a, const float *b, float *out )
{
    out[0] = ( a[0] + b[0] ) * 0.5f;
    out[1] = ( a[1] + b[1] ) * 0.5f;
    out[2] = 0.5f * ( a[2] + b[2] );
}

// sub_4BA610 (0x4BA610) — world→local: out[i] = orient.axis[i] · (pos - origin).
// (Distinct from OrientationPosToWorldPos 0x4BA430 which is local→world; the kisak
// port conflated the two — see select.cpp/map.cpp.  This is the faithful inverse.)
static void Region_WorldPosToLocal( const float *pos, float *out, const orientation_t *orient )
{
    float d[3];
    d[0] = pos[0] - orient->origin[0];
    d[1] = pos[1] - orient->origin[1];
    d[2] = pos[2] - orient->origin[2];
    out[0] = orient->axis[0][0] * d[0] + orient->axis[0][1] * d[1] + orient->axis[0][2] * d[2];
    out[1] = orient->axis[1][0] * d[0] + orient->axis[1][1] * d[1] + orient->axis[1][2] * d[2];
    out[2] = orient->axis[2][0] * d[0] + orient->axis[2][1] * d[1] + orient->axis[2][2] * d[2];
}

void Brush_DrawSubmitFaceWindings( selbrush_t *inst, const orientation_t *orient,
                                   float *a3, rface_t **outList )
{
    if ( inst->owner->prefab )
        iassert( !inst->owner->prefab );          // brush.cpp:4831 "!brush->owner->prefab"

    // patch brush → route to the pmesh winding submitter
    if ( inst->patch )
    {
        PMESH_29_Winding( (int)(intptr_t)inst->patch, orient, a3, outList );
        return;
    }

    // fixed-size (model/light) entities have no submittable faces
    entity_s *owner = inst->owner;
    eclass_t *eclass = ((entity_s_def *)owner->def)->eclass;
    if ( *(int *)&eclass->fixedsize )
        return;

    // cone cull: if the light is a spot (a3[0]==2 && radius a3[32] > 0), test the
    // brush's def AABB against the light's conic section and skip when culled.
    if ( *(int *)a3 == 2 && a3[8] > 0.0f )         // a3+0=class, a3+32=radius (=a3[8])
    {
        brush_t *def = inst->def;
        float boxMid[3], boxHalf[3], coneCtr[3], coneDir[3];
        Region_BoundsMidpoint( def->mins, def->maxs, boxMid );
        boxHalf[0] = boxMid[0] - def->mins[0];
        boxHalf[1] = boxMid[1] - def->mins[1];
        boxHalf[2] = boxMid[2] - def->mins[2];
        Region_WorldPosToLocal( &a3[1], coneCtr, orient );     // a3+4 = cone center
        VectorRotateByAxis( coneDir, (const float *)orient, &a3[4] );  // a3+16 = cone dir
        if ( CullBoxFromConicSectionOfSphere( coneCtr, coneDir, a3[8], a3[7], boxMid, boxHalf ) )
            return;                                 // a3[7]=radius(@28), a3[8]=cosFov? — see note
    }

    winding_t *worldW = Winding_Alloc( 1024 );
    const int layer = g_qeglobals.current_edit_layer;
    for ( int fi = 0; fi < inst->faceCount; fi++ )
    {
        face_t *face = &inst->def->faces[fi];
        winding_t *fw = face->w;
        if ( !fw || (unsigned int)fw->numpoints > 0x400u )
            continue;
        if ( MtlDef_IsFaceFiltered( &face->mtldef[layer] ) )
            continue;

        MaterialDef *md = &face->mtldef[layer];
        int exterior;
        if ( MaterialDef_10_LayeredMatHandle( md ) )
        {
            exterior = 1;
        }
        else
        {
            dword_181F51C = 0x2000;
            MaterialDef_02( md, MaterialDef_06 );
            if ( dword_181F51C )
                continue;                            // not a drawn (shadow) material
            exterior = 0;
        }

        // transform the face winding into the light's orientation, then submit.
        worldW->numpoints = fw->numpoints;
        for ( int i = 0; i < fw->numpoints; i++ )
            OrientationPosToWorldPos( worldW->p[i], fw->p[i], orient );
        sub_4DAA70( worldW, outList, a3, exterior );
    }

    --g_windingAlloc;
    free( worldW );
}

// ═════════════════════════════════════════════════════════════════════════════
//  XY brush draw — the 2D wireframe line path.
//
//  Pipeline: CXYWnd::XY_Draw -> DrawBrush (per brush) -> DrawGeo -> DrawShadedWireframe
//  (per visible face) -> R_Add3DLine batches winding edges -> R_AddCmd_Line3D.
//
//  LINE-ONLY ADAPTATION: the faithful editor draws faces filled/textured through the
//  per-face faceVis cache (Visuals_InitFaceVis) + the editor vertex-buffer pipeline,
//  gated on faceVis::visCount = the material's layer count.  This path renders only the
//  TECHNIQUE_WIREFRAME_SHADED branch: it gates on face->w (built by Brush_BuildWindings
//  at load) instead of visCount and skips the materialdef draw-flag gates.
// ═════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// 0x47b940 / 0x47b900  Brush_UpdateSpecialMaterialFlag (+ _Recurse) — set the brush DEF's
//   "special material" 2D back-face-cull flag (def->unk01 HIBYTE, byte at def+0x4D, read by
//   DrawGeo's cullMode).  For a MODEL entity (eclass.fixedsize) it recurses through the
//   model's content brush tree; for a brush entity it clears the flag, then (when prefs
//   draw_toggle is on) sets it iff ANY face's current-layer material carries the
//   MaterialDef_06 flag (the realize callback ANDs each layer's qtexture unk_flags2 low word
//   into dword_181F51C, seeded to 2 — a non-zero survivor means "special").  Patch brushes
//   (def->patch) are skipped entirely.  Faithful to the disasm (0x47b940); the previous
//   engine_stubs no-op left the flag at 0 after clone/paste.
// ─────────────────────────────────────────────────────────────────────────────
extern void MaterialDef_02( MaterialDef *m, int (*cb)( qtexture_s * ) ); // materialdef.cpp (0x431520)
extern int  MaterialDef_06( qtexture_s *radMtl );                        // materialdef.cpp (0x4318A0)
extern int  dword_181F51C;                                               // engine_stubs.cpp (realize-state)

static void Brush_UpdateSpecialMaterialFlag( brush_t *def );             // fwd (mutually recursive)

// 0x47b900 — walk a model's content brush tree, refreshing each brush's special-material
// flag.  `node` is &model->x2 (model+8); the entity list head is node[1] (entities.next
// @+0xC) and each entity's brush list uses the def(+0x08)/brushes(+0x0C) head overlay.
static void Brush_UpdateSpecialMaterialFlag_Recurse( int *node )
{
    for ( entity_s *ent = (entity_s *)node[1]; ent != (entity_s *)node; ent = ent->next )
    {
        for ( brush_t *b = (brush_t *)ent->brushes.prev; b != (brush_t *)&ent->def; b = b->onext )
            Brush_UpdateSpecialMaterialFlag( b );
    }
}

static void Brush_UpdateSpecialMaterialFlag( brush_t *def )
{
    if ( def->patch )
        return;

    entity_s *owner = def->owner;
    if ( *(int *)&owner->eclass->fixedsize )                 // model/light fixedsize entity
    {
        entitymodel_t *mc = (entitymodel_t *)owner->modelClass;
        if ( !mc )                                           // IDA: modelClass==0 → return
            return;
        models_t *model = mc->model;                         // modelClass->model @0x160
        iassert( model );                                    // "b->owner->modelClass->model" (L0)
        if ( model->entities.next )                          // model+0xC non-empty
            Brush_UpdateSpecialMaterialFlag_Recurse( &model->x2 );
        return;
    }

    // brush entity: clear the flag, then set it if any face's material is "special".
    *( (unsigned char *)def + 0x4D ) = 0;                    // HIBYTE(def->unk01) = 0
    if ( !g_PrefsDlg->draw_toggle || def->faceCount <= 0 )
        return;

    const int layer = g_qeglobals.current_edit_layer;
    for ( int i = 0; i < def->faceCount; ++i )
    {
        MaterialDef *md = &def->faces[i].mtldef[layer];
        dword_181F51C = 2;
        MaterialDef_02( md, MaterialDef_06 );
        if ( dword_181F51C )                                 // a "special" flag survived
        {
            *( (unsigned char *)def + 0x4D ) = 1;            // HIBYTE(def->unk01) = 1
            break;
        }
    }
}

// brush.cpp callers (OnSelectionClone / OnEditPastebrush) reach this through the engine_stubs
// extern; expose the real symbol under the historical name.
void sub_47B940( brush_t *def ) { Brush_UpdateSpecialMaterialFlag( def ); }

// ─────────────────────────────────────────────────────────────────────────────
// 0x47b5c0  DrawShadedWireframe  (brush.cpp:4888) — faithful.
// Append one face's closed-winding edges (each point transformed to world space
// through `orient`) into a caller-owned GfxPointVertex batch, with 2D back-face
// culling: cullMode 0=YZ/1=XZ/2=XY drops faces whose world normal points away along
// that view axis; cullMode -1 (special-material brushes) skips the cull. Returns the
// updated vertex count.
// ─────────────────────────────────────────────────────────────────────────────
int DrawShadedWireframe( int cullMode, face_t *face, const orientation_t *orient,
                         GfxColor *lineColor, char width, int vertCount,
                         int vertLimit, GfxPointVertex *verts )
{
    iassert( face );
    iassert( face->w );
    iassert( lineColor );
    iassert( verts );
    vassert( (vertCount >= 0 && vertCount <= vertLimit), "(vertCount) = %i", vertCount );

    if ( cullMode >= 0 && cullMode <= 2 )
    {
        float wdir[3];
        OrientationDirToWorldDir( wdir, orient, face->plane.normal );
        if ( wdir[cullMode] <= 0.0f )
            return vertCount;             // back-facing in this 2D view — skip
    }

    winding_t *w = face->w;
    if ( w->numpoints == 0 )
        return vertCount;

    // Closed outline: first edge wraps last->first, then point[i-1]->point[i].
    const float *cur = &w->p[0][0];
    int prevIdx = w->numpoints - 1;
    for ( int i = 0; i < w->numpoints; ++i )
    {
        vertCount = R_Add3DLine( verts, orient, &w->p[prevIdx][0], cur,
                                 (const unsigned int *)lineColor, width, vertCount, vertLimit );
        prevIdx = i;
        cur += 3;
    }
    return vertCount;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47b780  Face_AddWindingToTriBatch (sub_47B780) — append ONE face's winding to a
// caller-owned white-UNLIT triangle batch as a fan, auto-flushing the batch through
// R_AddRenderCmdDrawTris(d_white, TECHNIQUE_UNLIT, ...) when the next winding would
// overrun the caller's 0x552-vertex / 0x7FB-index caps.
//   • per vertex: xyzw = winding point + w=1, normal = the FACE PLANE normal (face+192),
//     colour = the caller's packed colour word (bit-cast into the float colour array),
//     st = {0,0}.
//   • indices: a one-sided fan (base, base+i-1, base+i) for i = 2 .. numpoints-1.
// The ONLY caller is CCamWnd::Cam_Draw's selected-FACE fill pass (0x4081eb).
// `packedColor` is a pointer to a GfxColor read as a float — the binary stores the raw
// dword into the float colour array (a bit-cast, not a value cast).
// ─────────────────────────────────────────────────────────────────────────────
void Face_AddWindingToTriBatch( face_t *face, const float *packedColor,
                                int *indexCount, unsigned short *indices,
                                int *vertCount, float ( *xyzw )[4],
                                float ( *normal )[3], float *colorArr,
                                float ( *st )[2] )
{
    extern void __cdecl R_AddRenderCmdDrawTris(
        Material *material, MaterialTechniqueType techType, short indexCount,
        const uint16_t *indices, short vertexCount,
        const float (*xyzw)[4], const float (*normal)[3], float *color,
        const float (*st)[2] );

    winding_t *w = face->w;                                     // 0x47b78f (face+224)
    if ( !w )                                                   // 0x47b79a
        return;

    // 0x47b7bd — flush when the append would overflow either cap.
    if ( w->numpoints + *vertCount > 0x552
      || *indexCount + 3 * w->numpoints - 6 > 0x7FB )
    {
        R_AddRenderCmdDrawTris( g_qeglobals.d_white, TECHNIQUE_UNLIT,
                                (short)*indexCount, indices, (short)*vertCount,
                                xyzw, normal, colorArr, st );   // 0x47b7e6
        *indexCount = 0;                                        // 0x47b7f1
        *vertCount  = 0;                                        // 0x47b7f7
    }

    // 0x47b800 — vertex append (positions + face-plane normal + flat colour + zero UVs).
    for ( int i = 0; i < w->numpoints; ++i )
    {
        const int v = *vertCount + i;
        xyzw[v][0] = w->p[i][0];                                // 0x47b844
        xyzw[v][1] = w->p[i][1];                                // 0x47b850
        xyzw[v][2] = w->p[i][2];                                // 0x47b85c
        xyzw[v][3] = 1.0f;                                      // 0x47b861
        normal[v][0] = face->plane.normal[0];                   // 0x47b86a
        normal[v][1] = face->plane.normal[1];                   // 0x47b873
        normal[v][2] = face->plane.normal[2];                   // 0x47b87c
        colorArr[v]  = *packedColor;                            // 0x47b886
        st[v][0] = 0.0f;                                        // 0x47b888
        st[v][1] = 0.0f;                                        // 0x47b88e
    }

    // 0x47b8ab — one-sided triangle fan off the winding's first point.
    if ( w->numpoints > 2 )
    {
        int ic = *indexCount;
        for ( int i = 2; i < w->numpoints; ++i )
        {
            indices[ic]     = (unsigned short)( *vertCount );        // 0x47b8ca
            indices[ic + 1] = (unsigned short)( *vertCount + i - 1 );// 0x47b8d6
            indices[ic + 2] = (unsigned short)( *vertCount + i );    // 0x47b8da
            ic += 3;                                                 // 0x47b8c6
        }
        *indexCount = ic;                                            // 0x47b8ed
    }
    *vertCount += w->numpoints;                                      // 0x47b8f1
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47acf0  DrawGeo  (brush.cpp:4652) — FULL port (wireframe + FILLED branches).
// Per winding-bearing face (with per-face visuals built by Brush_CheckBuildFaceVis):
//   • technique 29 (WIREFRAME_SHADED): batch the face outline (DrawShadedWireframe).
//   • mtlOverride set: emit ONE surf with the override material (Editor_AddGeoFace,
//     + Editor_AddGeoBackFace for special-material brushes).  This is the arm the
//     binary's tint/sun passes use (ecx at the 0x47b17f callsite = DrawBrush's 4th
//     param; hex-rays displays the fastcall pair swapped — disasm-verified).
//   • else: emit each material LAYER's visual {mtlHandle, vertHandle} at sortKey
//     sub_4FDBB0(firstLayerMaterial)+layerIndex — the filled/textured surf-cache draw
//     the camera consumes via R_AddEditorSurfsCmd → RC_DRAW_EDITOR_SKINNEDCACHED.
// Gates (binary order): face->w → vis->visCount → !MtlDef_IsFaceFiltered →
// MaterialDef_15_Drawflag_Multiply.  ONE documented divergence: the WIREFRAME branch
// draws even when visCount==0 — the port's headless/no-device faceVis is the identity
// array (visCount=0, see Brush_MakeFaceVisuals' Radiant_FaceVisGpuReady gate), and
// gating the lines on it would blank every 2D view in the device-less selftests.
// ─────────────────────────────────────────────────────────────────────────────
extern int  Editor_MaterialSortKey( Material *handle );                       // r_ed_scene.cpp 0x4FDBB0
extern void Editor_AddGeoFace( Material *handle, int techType, int sortKey,
                               int vertCount, int vbIndexAndOffs );           // r_ed_scene.cpp 0x4FEEF0
extern void Editor_AddGeoBackFace( Material *handle, int techType, int sortKey,
                                   int vertCount, int vbIndexAndOffs );       // r_ed_scene.cpp 0x4FEF50
extern bool MaterialDef_15_Drawflag_Multiply( int drawFlags, MaterialDef *m );// materialdef.cpp 0x431E90

// True if `mtl` is the CoD4 "$opaque" clip/tool editor material (basename "$opaque",
// after stripping any "wc/" path).  See the CoD3↔CoD4 techset note at the FILLED-branch
// call site: this material has no lit technique in the CoD4 binary but does in kisak's
// CoD3-loaded form, so it must not draw as an opaque lit surface in the camera.
static bool Material_IsToolOpaque( Material *mtl )
{
    if ( !mtl )
        return false;
    const Material *m = Material_FromHandle( mtl );   // resolve handle → struct (as Editor_AddMeshCmd)
    if ( !m || !m->info.name )
        return false;
    const char *slash = strrchr( m->info.name, '/' );
    const char *base  = slash ? slash + 1 : m->info.name;
    return strcmp( base, "$opaque" ) == 0;
}

void DrawGeo( GfxColor *col, Material *mtlOverride, selbrush_t *b,
              const orientation_t *orient, int viewType, int technique,
              char width, int drawFlags )
{
    iassert( b->owner->def == b->def->owner );

    brush_t *def = b->def;
    if ( def->faceCount <= 0 )
        return;

    // The IDB keeps this batch on the stack (GfxPointVertex[5450] ≈ 85 KB); the editor
    // is single-threaded and DrawGeo is non-reentrant, so a function-local static is
    // equivalent and keeps the frame small.
    static GfxPointVertex s_lineVerts[5450];
    int vertCount = 0;

    // HIBYTE(def->unk01) marks "special material" brushes: suppress 2D back-face
    // culling (cullMode -1) on the wireframe, and ALSO emit the back-facing fan on
    // the filled branches (0x47ae81 / 0x47af13).
    const bool specialMtl = ( ( (unsigned __int16)def->unk01 >> 8 ) & 0xFF ) != 0;
    const int  cullMode   = specialMtl ? -1 : viewType;

    for ( int facenum = 0; facenum < def->faceCount; ++facenum )
    {
        face_t *face = &def->faces[facenum];
        if ( !face->w )
            continue;
        faceVis_s *vis = b->faces ? &( (faceVis_s *)b->faces )[facenum] : nullptr;

        // 0x47adb9: per-face material filter gates BOTH branches.
        MaterialDef *mtldef = &face->mtldef[g_qeglobals.current_edit_layer];
        if ( MtlDef_IsFaceFiltered( mtldef ) || !MaterialDef_15_Drawflag_Multiply( drawFlags, mtldef ) )
            continue;

        if ( technique == 29 /*TECHNIQUE_WIREFRAME_SHADED*/ )
        {
            // (binary also gates the wireframe on vis->visCount @0x47ada6 — see the
            // header note on the headless identity-faceVis divergence.)
            vertCount = DrawShadedWireframe( cullMode, face, orient, col, width,
                                             vertCount, 5450, s_lineVerts );
            continue;
        }

        // FILLED branches need the GPU visuals.
        if ( !vis || !vis->visCount || !vis->visArray )
            continue;

        if ( mtlOverride )                              // 0x47ae3f — single override surf
        {
            int sortKey = Editor_MaterialSortKey( mtlOverride );
            Editor_AddGeoFace( mtlOverride, technique, sortKey, vis->vertcount,
                               vis->visArray->vertHandle );
            if ( specialMtl )
                Editor_AddGeoBackFace( mtlOverride, technique, sortKey, vis->vertcount,
                                       vis->visArray->vertHandle );
        }
        else                                            // 0x47aeae — per-layer visuals
        {
            // Binary: base = sub_4FDBB0(visArray[0].handle), then base+i per layer.
            // Kisak defensive: a layer whose GPU build failed stores mtlHandle=NULL
            // (Visuals_InitFaceVis) — the binary has no such case; skip those entries
            // (and seed the base key from the first non-null layer).
            int sortKey = -1;
            for ( int i = 0; i < vis->visCount; ++i )
            {
                Material *mtl = vis->visArray[i].mtlHandle;
                if ( !mtl )
                    continue;
                // CoD3↔CoD4 TECHSET DIVERGENCE (§11): the CoD4 clip/tool editor material
                // "$opaque" has NO lit (FAKELIGHT_NORMAL/VIEW, tech 24/25) technique — the
                // binary's Editor_AddMeshCmd technique gate (techniques[techType+1]) skips it,
                // so a prefab's clip-hull faces never draw as opaque lit surfaces.  kisak's
                // CoD3-loaded "$opaque" DOES carry a fakelight technique + a 1×1 white colorMap,
                // so the SAME face flushed white-opaque over the model (BRUSHPROBE: wc/$opaque,
                // opaque loadBits, fakelight_normal(24) = the white shell around the ch46e).
                // Reproduce the binary's technique-gate RESULT: don't emit the $opaque tool
                // material at a LIT camera technique.  Tech 29 (wireframe, 2D) is unaffected;
                // non-tool world/model materials (real colorMap) are unaffected.
                if ( ( technique == 24 || technique == 25 ) && Material_IsToolOpaque( mtl ) )
                    continue;
                if ( sortKey < 0 )
                    sortKey = Editor_MaterialSortKey( mtl );
                Editor_AddGeoFace( mtl, technique, sortKey + i, vis->vertcount,
                                   vis->visArray[i].vertHandle );
                if ( specialMtl )
                    Editor_AddGeoBackFace( mtl, technique, sortKey + i, vis->vertcount,
                                           vis->visArray[i].vertHandle );
            }
        }
    }

    if ( vertCount )
        R_AddCmd_Line3D( (short)( vertCount / 2 ), width, s_lineVerts );
}

// ─── patch control-point overlay (defined just below DrawPatchesWireframeGrid) ──
// DrawPatchesWireframeGrid tails with Patch_DrawControlPoints, so it's forward-declared.
void  Patch_DrawControlPoints( patch_t *patchInstance, const orientation_t *orient );
extern float world_orient_matrix[4][3];   // entity.cpp (identity orient) — used by the overlay

// ═════════════════════════════════════════════════════════════════════════════
// 0x441620  DrawPatchesWireframeGrid  (PMESH.CPP:4137) — PATCH wireframe (STAGE 1).
//
// Draws a curve patch's tessellated render mesh (patchMesh_t.curveDef, built by
// Patch_GenericMesh2) as a wireframe grid of 3D lines — the patch analogue of
// DrawGeo, and the path DrawBrush takes for a patch brush in a 2D (XY/XZ/YZ) view
// with technique 29 (TECHNIQUE_WIREFRAME_SHADED).  Each tessellated quad cell emits
// its top/left edges (shared edges drawn once via the col==0 / row==0 guards) plus
// the two far edges and one diagonal; the lines batch through R_Add3DLine →
// R_AddCmd_Line3D, exactly like the brush wireframe.
//
// KISAK adaptations:
//  * The binary first calls PMESH_25_PatchVersion (sync the patch INSTANCE's visuals
//    to the def version via sub_440240).  That instance rebuild builds the FILLED
//    draw's per-layer visuals (MaterialDef_11 layer counts = the deferred material
//    epic) which the WIREFRAME does not read — so we skip it and instead ensure the
//    def's curveDef is built (Patch_ParseMesh already builds it at load; this is the
//    safety net for a never-tessellated patch).
//  * MaterialDef_15_Drawflag_Multiply (the draw-flag gate at the top) returns 1 for
//    the line path (drawFlags has no DRAWFLAG_ONLY/SKIP_MULTIPLY bits), so it is a
//    passthrough here — matching the degenerate-materialdef shim already used by the
//    brush wireframe.
//  * sub_440750 (the control-POINT vertex markers overlay) only draws in patch-vertex
//    edit modes (sel_curvepoint / sel_area / bend); in a normal view it early-returns,
//    so it is a documented no-op for Stage 1 (patch vertex editing is deferred with
//    the patch dialog).
//  The diagonal direction matches the binary: terrain patches honour the control
//  vertex's `turned_edge` flag (terrain render mesh == control grid, so the index is
//  in range); bezier patches always take the v10→v14 diagonal (type != 64).
// ═════════════════════════════════════════════════════════════════════════════
void DrawPatchesWireframeGrid( patch_t *patchInstance, GfxColor *col,
                               const orientation_t *orient, char width, int drawFlags )
{
    if ( !patchInstance )
        return;
    patchMesh_t *def = patchInstance->def;                 // instance.def == patch DEF
    if ( !def )
        return;

    // Ensure the render mesh exists (built at load; rebuild defensively if absent).
    if ( !def->curveDef )
    {
        def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
        ++def->version;
        if ( !def->curveDef )
            return;
    }

    curvePatchDef_t *mesh = def->curveDef;
    int   mw   = mesh->width;
    int   mh   = mesh->height;
    if ( mw <= 0 || mh <= 0 )
        return;
    curveVert_t *verts = mesh->verts;

    const bool terrain = ( *(int *)&def->type == 64 );      // PATCH_TERRAIN

    // The IDB keeps the line batch on the stack (GfxPointVertex[1362]); reuse the same
    // function-local-static pattern as DrawGeo (single-threaded, non-reentrant).
    static GfxPointVertex s_patchVerts[1362];
    const int kLimit = 1362;
    const unsigned int *lineCol = (const unsigned int *)col;
    int vc = 0;

    for ( int row = 0; row + 1 < mh; ++row )
    {
        for ( int col2 = 0; col2 + 1 < mw; ++col2 )
        {
            const float *v00 = verts[col2     +   row       * mw].xyz;   // [col][row]
            const float *v10 = verts[col2 + 1 +   row       * mw].xyz;   // [col+1][row]
            const float *v01 = verts[col2     + ( row + 1 ) * mw].xyz;   // [col][row+1]
            const float *v11 = verts[col2 + 1 + ( row + 1 ) * mw].xyz;   // [col+1][row+1]

            // shared edges drawn once: left edge only for the first column, top edge
            // only for the first row.
            if ( col2 == 0 )
                vc = R_Add3DLine( s_patchVerts, orient, v00, v01, lineCol, width, vc, kLimit );
            if ( row == 0 )
                vc = R_Add3DLine( s_patchVerts, orient, v00, v10, lineCol, width, vc, kLimit );
            // far edges of this cell.
            vc = R_Add3DLine( s_patchVerts, orient, v11, v10, lineCol, width, vc, kLimit );
            vc = R_Add3DLine( s_patchVerts, orient, v11, v01, lineCol, width, vc, kLimit );
            // diagonal: terrain honours the control vertex's turned_edge flag
            // (terrain render mesh == control grid, so [col][row] is in range);
            // bezier always takes the v01→v10 diagonal.
            bool turned = terrain && ( def->ctrl[col2][row].turned_edge & 1 );
            if ( turned )
                vc = R_Add3DLine( s_patchVerts, orient, v00, v11, lineCol, width, vc, kLimit );
            else
                vc = R_Add3DLine( s_patchVerts, orient, v01, v10, lineCol, width, vc, kLimit );
        }
    }

    if ( vc )
    {
        R_AddCmd_Line3D( (short)( vc / 2 ), width, s_patchVerts );
    }

    // Control-POINT marker overlay: the binary's DrawPatchesWireframeGrid tails with
    // sub_440750((int)orient) (0x441890) to draw this patch's control-point grid as
    // point markers when a patch-point edit mode is active.  Mirror it (display subset).
    Patch_DrawControlPoints( patchInstance, orient );
}

// ═════════════════════════════════════════════════════════════════════════════
// 0x440750  Patch_DrawControlPoints (sub_440750)  (PMESH.CPP) — per-patch
// CONTROL-POINT MARKER overlay.  Called at the tail of DrawPatchesWireframeGrid /
// DrawPatches / sub_4415D0 (all the per-patch draws).  In a patch point-edit mode
// (d_select_mode == sel_curvepoint|sel_area, or bend/redisperse modes) it walks the
// patch's 16×16 control grid and emits each control point as a coloured 3D point
// marker (R_AddPointCmd_W, size 6) so the user can see and pick the grid handles.
//
// PORT SCOPE: this ports the STEADY-STATE display branch (the binary's
// `!(g_bPatchBendMode || g_qeglobals_redispersePatchVerts)` tail at 0x44080d) — the
// plain sel_curvepoint/sel_area marker draw.  The BEND-MODE branches and the
// interactive control-point DRAG are not ported; bend mode is off by default.
//
// COLOURS (binary flt_6DE1xx, decoded from the IDB .data):
//   flt_6DE130 = red     (1,0,0)  — a control vertex flagged turned_edge&2 (a "marked" CP)
//   flt_6DE140 = blue    (0,0,1)  — an even-parity (row|col even) interior control point
//   flt_6DE1B0 = magenta (1,0,1)  — an odd-parity control point (the checkerboard)
//   flt_6DE160 = blue    (0,0,1)  — a SELECTED control point (in d_move_points), drawn
//                                   on top in a 2nd reverse-order pass.
// SELECTION TEST: sub_43C2C0(ctrlPtr) → index into g_qeglobals.d_move_points[] (the
// currently-selected control points, matched by exact xyz) or -1.  >=0 ⇒ deferred to
// the on-top "selected" pass; <0 ⇒ drawn now with the checkerboard colour.
//
// __usercall: the patch INSTANCE comes in ECX (`mov esi,ecx`); the orientation is the
// stack arg.  Instance[0] == the patchMesh_t* def; instance+6 is the "draw this patch"
// byte (set by Brush_Build, brush.cpp:991).  Faithfully gates on instance+6 != 0.
// ═════════════════════════════════════════════════════════════════════════════
// sub_43C2C0 (0x43c2c0): index of `ctrl` in d_move_points[] (selected set), else -1.
// Faithful: each d_move_points[i] is matched by xyz within a tiny epsilon (1e-6 per axis).
static int Patch_FindSelectedMovePoint( const drawVert_t *ctrl )
{
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        const drawVert_t *mp = g_qeglobals.d_move_points[i];
        int matched = 0;
        for ( ; matched < 3; ++matched )
        {
            float d = ctrl->xyz[matched] - mp->xyz[matched];
            if ( d * d > 1.00000011e-06f )   // binary's 0x358637C0 == 9.5367e-07
                break;
        }
        if ( matched >= 3 )
            return i;
    }
    return -1;
}

void Patch_DrawControlPoints( patch_t *patchInstance, const orientation_t *orient )
{
    if ( !patchInstance )
        return;

    // instance.selected (the per-patch "draw" flag) must be set, and a patch point-edit
    // mode must be active (the binary's top guard at 0x44076d/0x440777).
    if ( patchInstance->selected == 0 )
        return;

    extern int g_bPatchBendMode;                 // 0x25d5b04
    extern int g_qeglobals_redispersePatchVerts; // 0x25d5a6b (engine_stubs)
    if ( g_qeglobals.d_select_mode != sel_curvepoint &&
         g_qeglobals.d_select_mode != sel_area &&
         !g_bPatchBendMode && !g_qeglobals_redispersePatchVerts )
        return;

    patchMesh_t *def = patchInstance->def;               // instance.def == patch DEF
    if ( !def )
        return;

    // BEND-MODE branches deferred (documented above): render nothing in bend/redisperse
    // until the bend/drag port lands (matches the pre-unit behaviour for those modes).
    if ( g_bPatchBendMode || g_qeglobals_redispersePatchVerts )
        return;

    // Colours (binary flt_6DE130/140/1B0/160).  Packed once, reused per point.
    static const float s_red[4]     = { 1.0f, 0.0f, 0.0f, 1.0f };   // flt_6DE130 turned_edge&2
    static const float s_blue[4]    = { 0.0f, 0.0f, 1.0f, 1.0f };   // flt_6DE140 even-parity
    static const float s_selBlue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };   // flt_6DE160 selected
    static const float s_magenta[4] = { 1.0f, 0.0f, 1.0f, 1.0f };   // flt_6DE1B0 odd-parity
    GfxColor cRed, cBlue, cSelBlue, cMagenta;
    Byte4PackPixelColor( const_cast<float *>( s_red ),     &cRed );
    Byte4PackPixelColor( const_cast<float *>( s_blue ),    &cBlue );
    Byte4PackPixelColor( const_cast<float *>( s_magenta ), &cMagenta );
    Byte4PackPixelColor( const_cast<float *>( s_selBlue ), &cSelBlue );

    // Batch points (binary flushes at 1361 → s_pts[1362]); reuse DrawGeo's static-buffer
    // pattern (single-threaded, non-reentrant — the editor draws one window at a time).
    static GfxPointVertex s_pts[1362];
    int n = 0;

    // Selected control points are deferred to a 2nd reverse-order pass (binary v32[]).
    const drawVert_t *selStack[258];
    int selCount = 0;

    const int rows = def->width;     // [edi]    (control grid width)
    const int cols = def->height;    // [edi+4]  (control grid height)

    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < cols; ++j )
        {
            const drawVert_t *cp = &def->ctrl[i][j];   // &v3[14 + 20*i*... ] in the disasm

            if ( Patch_FindSelectedMovePoint( cp ) >= 0 )
            {
                // selected → on-top pass (binary pushes the ctrl ptr onto v32[++var_554C])
                if ( selCount < 258 )
                    selStack[selCount++] = cp;
                continue;
            }

            // not selected → checkerboard colour now.
            GfxColor *col;
            if ( ( cp->turned_edge & 2 ) != 0 )         // test byte [esi+4Ch],2
                col = &cRed;
            else if ( ( ( i | j ) & 1 ) != 0 )          // (var_5540 | var_5534) & 1
                col = &cMagenta;
            else
                col = &cBlue;

            OrientationPosToWorldPos( s_pts[n].xyz, cp->xyz, orient );
            *(GfxColor *)s_pts[n].color = *col;
            if ( ++n == 1361 ) { R_AddPointCmd_W( (short)n, 6, s_pts ); n = 0; }
        }
    }

    // 2nd pass: the selected control points, in REVERSE order, in the selected colour
    // (binary's `for (; v38; ) { v10 = v32[v38--]; ... col = flt_6DE160 }` at 0x440964).
    for ( int s = selCount - 1; s >= 0; --s )
    {
        OrientationPosToWorldPos( s_pts[n].xyz, selStack[s]->xyz, orient );
        *(GfxColor *)s_pts[n].color = cSelBlue;
        if ( ++n == 1361 ) { R_AddPointCmd_W( (short)n, 6, s_pts ); n = 0; }
    }

    if ( n )
        R_AddPointCmd_W( (short)n, 6, s_pts );
}

// ═════════════════════════════════════════════════════════════════════════════
// 0x47D060  sub_47D060 — brush-list DISPLAY REBUILD.  For every instance on the given
// display list: bump its DEF's version word (forces the faceVis/winding rebuild on the
// next draw), and for a PATCH brush re-tessellate the patch DEF at the CURRENT EDIT
// LAYER (free old curveDef → Patch_GenericMesh2 → ++def->version → assign).  If the
// instance's owner entity is a PREFAB, recurse into the prefab's own brush list
// (prefab_s+0xC is its embedded selbrush sentinel), then bump the owner DEF's model
// version word (+0x78, 16-bit — the established version_prob_wrong idiom).  31 call
// sites: layer switch / relayer / script-group / layered-material / prefab enter-leave.
// [audit U5, 2026-07-25 — was a no-op stub in engine_stubs.cpp whose comment called it
// a "benign display rebuild"; stale patch tessellation on layer ops was the symptom.]
void sub_47D060( int listHead )
{
    selbrush_t *sentinel = (selbrush_t *)(intptr_t)listHead;
    for ( selbrush_t *i = sentinel->next; i != sentinel; i = i->next )   // 0x47d072
    {
        ++i->def->version;                                    // 0x47d083  add word [def+0x4E],1
        if ( i->patch )                                       // 0x47d08c
        {
            patchMesh_t *pm = i->patch->def;                  // 0x47d08e
            if ( pm->curveDef )                               // 0x47d098
                free( pm->curveDef );                         // 0x47d09b
            curvePatchDef_t *cd =
                Patch_GenericMesh2( pm, g_qeglobals.current_edit_layer, 0, 0 );  // 0x47d0af
            ++pm->version;                                    // 0x47d0b7
            pm->curveDef = cd;                                // 0x47d0be
        }
        entity_s *owner = i->owner;                           // 0x47d0c4
        if ( owner )                                          // 0x47d0c9
        {
            if ( owner->prefab )                              // 0x47d0d0
                sub_47D060( (int)(intptr_t)( (char *)owner->prefab + 0xC ) );    // 0x47d0d6
            entity_s *eDef = (entity_s *)owner->def;          // 0x47d0e1
            ++*(unsigned __int16 *)&eDef->version_prob_wrong; // 0x47d0e4  add word [def+0x78],1
        }
    }
}

// 0x40c250  Draw_PatchSelectPoints  (CamWnd.cpp) — the curve-point CANDIDATE overlay,
// called once per view from BOTH XY_Draw (0x46d11b) and DrawGeneralWorld_ (0x407bb4).
// When a patch point-edit mode is active (sel_curvepoint|sel_area) it walks
// g_qeglobals.patch_verts_array01 (the displayed candidate-point list, vec3 world
// coords, count patch_verts_array01_count) and draws each point that is NOT in the
// selected set (sub_43C1C0 returns -1) as a GREEN marker (flt_6DE150 = {0,0.7,0,1}).
// Binary gate 0x40c2b7: `test eax,eax / jge → skip` — the >=0 (selected) points are
// SKIPPED here; they are drawn LIGHT BLUE by the inverse twin sub_40C360
// (Draw_PatchSelectPointsSelected below, called from the DrawConnectionLinks prefix).
// [2026-07-25 U1] The port previously drew the >=0 set here (the twins were conflated
// into one function with 0x40c250's colour and 0x40c360's gate) — the overlay showed
// exactly the COMPLEMENT of the correct green set, and the selected set never drew.
//
// FIDELITY: faithful 1:1.  The arrays live in qeglobals_t (patch_verts_array01 starts
// at the `unkown_pmesh_float1` field; each entry is a vec3 / 12 bytes).  The points
// are already in WORLD space, so they're transformed through the IDENTITY orientation
// (world_orient_matrix) exactly like the binary (OrientationPosToWorldPos(..,&world_orient_matrix)).
// ═════════════════════════════════════════════════════════════════════════════
// sub_43C1C0 (0x43c1c0): index of world point `pt` in patch_verts_array02 (the SELECTED
// curve points, vec3 stride), within 0.1u (dist² < 0.01), else -1.
// NON-STATIC: also called by Patch_SelectAreaPoints_sub (pmesh.cpp, 0x4484c0) — the
// sel_area rect-select path; see the extern decl there.
int Patch_FindSelectedArrayPoint( const float *pt )
{
    if ( g_qeglobals.patch_verts_array02_count <= 0 )
        return -1;
    // The binary's array02 base is &unkown_pmesh_float2 (the field 8 bytes BEFORE the
    // qe3.h `patch_verts_array02` symbol): sub_43c1c0 loads `offset patch_verts_array02`
    // then reads [-8]/[-4]/[+0] for the first point's xyz, advancing +12 per element.  So
    // element 0's x = unkown_pmesh_float2, mirroring array01's base (= unkown_pmesh_float1).
    const float *p = &g_qeglobals.unkown_pmesh_float2;
    for ( int i = 0; i < g_qeglobals.patch_verts_array02_count; ++i, p += 3 )
    {
        float dx = p[0] - pt[0];
        float dy = p[1] - pt[1];
        float dz = p[2] - pt[2];
        if ( dx * dx + dy * dy + dz * dz < 0.01f )
            return i;
    }
    return -1;
}

void Draw_PatchSelectPoints()
{
    if ( g_qeglobals.d_select_mode != sel_curvepoint &&
         g_qeglobals.d_select_mode != sel_area )
        return;
    if ( g_qeglobals.patch_verts_array01_count <= 0 )
        return;

    static const float s_green[4] = { 0.0f, 0.7f, 0.0f, 1.0f };   // flt_6DE150
    GfxColor col;
    Byte4PackPixelColor( const_cast<float *>( s_green ), &col );

    static GfxPointVertex s_pts[1362];
    int n = 0;

    // patch_verts_array01 begins at the `unkown_pmesh_float1` field (the binary reads
    // &g_qeglobals.unkown_pmesh_float1 with a 3-float stride).
    const float *pt = &g_qeglobals.unkown_pmesh_float1;
    for ( int i = 0; i < g_qeglobals.patch_verts_array01_count; ++i, pt += 3 )
    {
        if ( Patch_FindSelectedArrayPoint( pt ) >= 0 )  // 0x40c2b7 jge → skip: selected
            continue;                                   // points draw via the 0x40c360 twin
        OrientationPosToWorldPos( s_pts[n].xyz, pt,
                                  (const orientation_t *)world_orient_matrix );
        *(GfxColor *)s_pts[n].color = col;
        if ( ++n == 1362 ) { R_AddPointCmd_W( (short)n, 6, s_pts ); n = 0; }   // 0x40c2ea cmp eax,552h
    }

    if ( n )
        R_AddPointCmd_W( (short)n, 6, s_pts );
}

// 0x40c360  Draw_PatchSelectPointsSelected (sub_40C360) — the inverse twin of
// Draw_PatchSelectPoints (byte-identical body except colour + gate): draws each
// patch_verts_array01 point that IS in the selected set (sub_43C1C0 >= 0, gate
// 0x40c3c7 `jl → skip`) as a LIGHT-BLUE marker (flt_6DE170 = {0.5,0.5,1,1}).
// Same self-gate (sel_curvepoint|sel_area), same batching (flush at 1362,
// R_AddPointCmd_W technique 6, identity orientation).  The binary calls it from the
// DrawConnectionLinks (0x40c9f0) prefix at 0x40ca0f — i.e. from BOTH XY_Draw and
// Cam_Draw tails — right before the sel_vertex/sel_edge handle draw.
// [2026-07-25 U1] Newly ported: this half of the conflated pair was missing entirely.
void Draw_PatchSelectPointsSelected()
{
    if ( g_qeglobals.d_select_mode != sel_curvepoint &&
         g_qeglobals.d_select_mode != sel_area )
        return;
    if ( g_qeglobals.patch_verts_array01_count <= 0 )
        return;

    static const float s_selBlue[4] = { 0.5f, 0.5f, 1.0f, 1.0f };   // flt_6DE170
    GfxColor col;
    Byte4PackPixelColor( const_cast<float *>( s_selBlue ), &col );

    static GfxPointVertex s_pts[1362];
    int n = 0;

    const float *pt = &g_qeglobals.unkown_pmesh_float1;
    for ( int i = 0; i < g_qeglobals.patch_verts_array01_count; ++i, pt += 3 )
    {
        if ( Patch_FindSelectedArrayPoint( pt ) < 0 )   // 0x40c3c7 jl → skip: unselected
            continue;
        OrientationPosToWorldPos( s_pts[n].xyz, pt,
                                  (const orientation_t *)world_orient_matrix );
        *(GfxColor *)s_pts[n].color = col;
        if ( ++n == 1362 ) { R_AddPointCmd_W( (short)n, 6, s_pts ); n = 0; }   // 0x40c3fa cmp eax,552h
    }

    if ( n )
        R_AddPointCmd_W( (short)n, 6, s_pts );
}


// ─────────────────────────────────────────────────────────────────────────────
//  DrawBrush_PrefabContents — the prefab-render slice of DrawModels (0x4796F0) +
//  sub_478B10 (0x478B10), reduced to the line path.
//
//  When a misc_prefab entity's bbox brush is drawn, the original calls DrawModels ->
//  sub_478B10, which iterates the entity's prefab_s->active_brushlist (the brush
//  INSTANCES of the referenced .map, built by Entity_InitPrefabInst) and recursively
//  DrawBrush()es each through the entity's FORWARD orientation (Entity_GetOrientation,
//  NOT the inverse the bbox uses).  Those prefab brushes are plain worldspawn-class
//  brushes, so the recursion bottoms out in DrawGeo.
//
//  Skipped vs sub_478B10: the camera/XY frustum cull (CullCubic / sub_46CD80) and the
//  spawnflags drawFlags|=2 "ghost" toggle (read for fidelity; the line path ignores
//  it).  Filtered prefab brushes (FilterBrush) ARE honoured.
// ─────────────────────────────────────────────────────────────────────────────
extern char FilterBrush( selbrush_t *b, int updateFilters );     // filters.cpp 0x46A1F0

// Forward decl (DrawBrush_PrefabContents recurses into DrawBrush, defined just below).
void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                int technique, GfxColor *col, char width, int drawFlags,
                const char *layerPrefix );

// ─────────────────────────────────────────────────────────────────────────────
// 0x479d50  DrawAngles  (brush.cpp:4229) — the entity ANGLE ARROW overlay.
//
// The View→Show→Angles toggle (d_xyShowFlags bit 0x2; SET = arrows HIDDEN) draws a
// small arrowhead (a forward stick + two barbs) from a fixedsize point-entity origin
// in the direction of its "angles" key, in BOTH the XY views and the camera (DrawBrush
// is shared by both).  Transcribed verbatim from IW3xRadiant.i64 0x479d50.
//
// drawType (a1) is the view-axis index 0..2 for the XY/camera passes; a1 == -1 is the
// binary's special "use the code-default grey, then white for the barbs" rendering
// (selected/active pass) — the editor's XY/camera passes pass the view-axis (!= -1), so
// the passed colour is used, but the -1 branch is transcribed for fidelity.
//
// Reuses the line batcher already proven for the connections overlay (R_Add3DLine →
// R_AddCmd_Line3D) and the engine math helpers (AnglesToAxis / OrientationConcatenate).
// ─────────────────────────────────────────────────────────────────────────────
extern float *AnglesToAxis( float *angles, float ( *axis )[3] );                     // engine_stubs.cpp 0x4abeb0
extern void   OrientationConcatenate( const orientation_t *orFirst,
                                      const orientation_t *orSecond,
                                      orientation_t *out );                          // engine_stubs.cpp 0x4ba7d0
extern bool   HasKeyValuePair( entity_s_def *e, const char *key );                   // entity.cpp 0x4838b0
extern float  world_orient_matrix[4][3];                                             // entity.cpp (identity orient)

// flt_6DE220 (0x6de220) = the code-default arrow colour (grey) for the a1==-1 pass.
static const float kAngleArrowDefault[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

// The selbrush_t whose owner entity DrawAngles reads (the binary passes it in ECX; we
// thread it through this single-threaded-editor global so DrawAngles keeps the IDB shape).
selbrush_t *g_drawAnglesBrush = nullptr;

void DrawAngles( int drawType, const orientation_t *orient, GfxColor *col )
{
    // `b` is the selbrush_t whose owner entity supplies the angles.  The binary passes it
    // in ECX (the DrawBrush call site below threads it through as the first explicit arg).
    selbrush_t *b = g_drawAnglesBrush;
    if ( !b )
        return;

    entity_s *owner = b->owner;
    if ( owner == world_entity )
        return;                                           // worldspawn has no angle arrow

    iassert( b->owner->def == b->def->owner );

    entity_s_def *eDef = (entity_s_def *)owner->def;
    if ( !eDef || !HasKeyValuePair( eDef, "angles" ) )
        return;                                           // no "angles" key → no arrow

    eclass_t *eclass = eDef->eclass;
    if ( !eclass )
        return;
    // Drawn for fixedsize entities, or the representative def-brush of a brush entity,
    // and never for prefab classes (classtype & 0x10).  IDB (0x479dcc): the brush-entity
    // gate compares `b->def (brush_t*)` against the entity DEF's brush-list head pointer
    // at (entity_s_def + 0x0C) — the embedded def-brush list head's first pointer, a
    // brush_t* (the def list, NOT the instance list `selbrush_t.ownerPrev`).  Read it raw
    // to match the binary's `cmp ecx, [ebx+0Ch]`.
    brush_t *repBrush = *(brush_t **)( (char *)eDef + 0x0C );
    bool isRep = ( b->def == repBrush );
    if ( ( !*(int *)&eclass->fixedsize && !isRep ) || ( eclass->classtype & 0x10 ) != 0 )
        return;

    // angles → orientation axis
    float angles[3] = { 0.0f, 0.0f, 0.0f };
    if ( !Entity_GetVec3ForKey( eDef, angles, "angles" ) )
    { angles[0] = angles[1] = angles[2] = 0.0f; }

    orientation_t entOrient;
    AnglesToAxis( angles, entOrient.axis );

    // origin = centre of the eclass bbox + the entity's world origin
    entOrient.origin[0] = ( eclass->mins[0] + eclass->maxs[0] ) * 0.5f + eDef->origin[0];
    entOrient.origin[1] = ( eclass->mins[1] + eclass->maxs[1] ) * 0.5f + eDef->origin[1];
    entOrient.origin[2] = ( eclass->mins[2] + eclass->maxs[2] ) * 0.5f + eDef->origin[2];

    orientation_t worldOrient;
    OrientationConcatenate( &entOrient, orient, &worldOrient );

    // arrowhead scale = half the brush's X extent
    const float scale = ( b->def->maxs[0] - b->def->mins[0] ) * 0.5f;
    const float base[3] = { 0.0f, 0.0f, 0.0f };
    const float tip[3]  = { 1.62399995f * scale, 0.0f, 0.0f };
    const float wing1[3]= { 1.27799999f * scale,  0.34599998f * scale, 0.0f };
    const float wing2[3]= { 1.27799999f * scale, -0.34599998f * scale, 0.0f };

    GfxPointVertex verts[6];
    GfxColor arrowCol;
    if ( drawType == -1 )
        Byte4PackPixelColor( const_cast<float *>( kAngleArrowDefault ), &arrowCol );
    else
        arrowCol.packed = col->packed;

    int vc = R_Add3DLine( verts, &worldOrient, base, tip, (const unsigned int *)&arrowCol, 2, 0, 6 );
    vc      = R_Add3DLine( verts, &worldOrient, tip, wing1, (const unsigned int *)&arrowCol, 2, vc, 6 );
    if ( drawType == -1 )
        Byte4PackPixelColor( const_cast<float *>( colorWhite ), &arrowCol );
    vc      = R_Add3DLine( verts, &worldOrient, tip, wing2, (const unsigned int *)&arrowCol, 2, vc, 6 );
    if ( vc )
    {
        R_AddCmd_Line3D( (short)( vc / 2 ), 2, verts );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  RENDER DECORATIONS — the entity ORIGIN BOX + the ScriptGroup colour billboards.
//  These are the decoration half of DrawModels (0x4796F0): the MESH half (SkinModelInst)
//  is the separate Editor_DrawModelsPass / Cam_DrawModels pass (model-render epic); this
//  block ports the per-entity origin-box wireframe (DrawOriginBox 0x478E30) + the
//  script-colour name billboards (sub_46AE10 / sub_453EF0) that the binary's DrawModels
//  draws after the mesh, then a DrawModels_Decorations() wrapper that reproduces the
//  binary's DrawModels colour/extent logic and is called from DrawBrush's fixedsize
//  branch (where the binary calls DrawModels). DrawLightsMain's glow-sphere is ported
//  separately (camwnd.cpp Cam_DrawLightPreviewSpheres). [render decorations, opus]
// ═════════════════════════════════════════════════════════════════════════════
extern char *ValueForKey2( int e, const char *key );                 // entity.cpp 0x4825C0
extern int   ScriptGroup_Unreachable( const char *a1 );              // scriptgroup.cpp 0x451170
extern bool  ScriptGroup_BrushIsTrigger( selbrush_t *b );            // scriptgroup.cpp 0x453FD0
extern void  __cdecl R_AddRenderCmdDrawTris(
    Material *material, MaterialTechniqueType techType, short indexCount,
    const uint16_t *indices, short vertexCount,
    const float (*xyzw)[4], const float (*normal)[3], float *color,
    const float (*st)[2] );

// 0x479690  sub_479690 — the ScriptGroup colour-quad gate: the entity carries the
// ScriptGroupKey AND that key isn't the ScriptColorTeamKey (the colour-team path owns
// that one). Faithful vs IDA 0x479690.
static bool sub_479690_HasScriptGroupKey( selbrush_t *b )
{
    entity_s_def *eDef = (entity_s_def *)b->owner->def;
    const char *sgKey = g_PrefsDlg->ScriptGroupKey;
    if ( !HasKeyValuePair( eDef, sgKey ) )
        return false;
    return strcmp( sgKey, g_PrefsDlg->ScriptColorTeamKey ) != 0;
}

// flt_73B098 (0x73B098) — the 7 script-colour token colours (r/b/y/c/g/p/o), indexed by
// ScriptGroup_Unreachable. Shared by the camera token billboards + sub_453EF0/sub_46AE10.
static const float kScriptTokenColors[7][4] = {
    { 1.0f, 0.0f, 0.0f, 1.0f },   // r — red
    { 0.0f, 0.0f, 1.0f, 1.0f },   // b — blue
    { 1.0f, 1.0f, 0.0f, 1.0f },   // y — yellow
    { 0.0f, 1.0f, 1.0f, 1.0f },   // c — cyan
    { 0.0f, 1.0f, 0.0f, 1.0f },   // g — green
    { 1.0f, 0.0f, 1.0f, 1.0f },   // p — purple
    { 1.0f, 0.4f, 0.0f, 1.0f },   // o — orange
};

// flt_739DC8 (0x739DC8) — the 16 ScriptGroup colour-quad colours (atol(spawnflags)&0xF).
static const float kScriptGroupQuadColors[16][4] = {
    { 1.0f, 0.0f,  0.0f, 1.0f }, { 0.0f, 0.0f,  1.0f, 1.0f },
    { 1.0f, 1.0f,  0.0f, 1.0f }, { 0.0f, 1.0f,  1.0f, 1.0f },
    { 0.0f, 1.0f,  0.0f, 1.0f }, { 1.0f, 0.0f,  1.0f, 1.0f },
    { 0.8f, 0.3f,  0.0f, 1.0f }, { 1.0f, 0.75f, 0.75f,1.0f },
    { 0.3f, 0.6f,  1.0f, 1.0f }, { 0.6f, 1.0f,  0.6f, 1.0f },
    { 0.4f, 0.0f,  0.0f, 1.0f }, { 0.0f, 0.0f,  0.5f, 1.0f },
    { 0.0f, 0.5f,  0.0f, 1.0f }, { 0.0f, 0.5f,  0.5f, 1.0f },
    { 0.5f, 0.0f,  0.5f, 1.0f }, { 0.65f,0.65f, 0.65f,1.0f },
};

// ─────────────────────────────────────────────────────────────────────────────
// 0x478E30  DrawOriginBox  (brush.cpp:3938) — the entity ORIGIN-BOX wireframe.
//
// Draws (a) three axis lines through `origin` spanning mins..maxs, (b) the 12 edges of
// the mins..maxs box, and (c) — when View→Entities-as-bounding-box is on
// (m_nEntityShowState & 0x1000) and the entity is NOT a disk trigger (classtype & 0x80)
// — the 12 edges of the eclass MODEL bbox (b->def mins/maxs at +0x20/+0x2C), coloured by
// the eclass colour. Batches through R_Add3DLine → R_AddCmd_Line3D, exactly the proven
// origin-box line path (same drawer the angle arrow / connections overlays use).
//
// IDB args (__usercall): mins@edx, origin@ecx(int a2, read as float[3]), width@dil,
// orient@esi, b(selbrush)@a4, maxs@a5, lineColor@a6. Normalised to cdecl here.
// 3 axis lines + 12 box edges via R_Add3DLine.  The m_nEntityShowState&0x1000
// eclass-model-bbox gate is `test byte [eclass+0x180],0x10` (CLASS-as-bbox), reading
// b->def(brush_t) mins@+0x20 / maxs@+0x2C; the disk-trigger skip is that same 0x10 bit on
// the eclass model node (NOT the 0x80 disk bit — that one drives DrawTriggerRadius).  The
// binary reuses ONE GfxPointVertex[256] batch + one R_AddCmd_Line3D flush at the end.
// ─────────────────────────────────────────────────────────────────────────────
static void DrawOriginBox( const float *mins, const float *origin, char width,
                           const orientation_t *orient, selbrush_t *b,
                           const float *maxs, const float *lineColor )
{
    GfxColor col;
    Byte4PackPixelColor( const_cast<float *>( lineColor ), &col );
    const unsigned int *cp = (const unsigned int *)&col;

    // The IDB keeps the line batch on the stack (GfxPointVertex[256]); the editor is
    // single-threaded and this is non-reentrant, so a function-local static is equivalent.
    static GfxPointVertex s_box[256];
    const int kLimit = 256;
    int vc = 0;

    // (a) three axis lines through `origin`, spanning the mins..maxs extent on each axis.
    float p1[3], p2[3];
    p1[0] = mins[0]; p1[1] = origin[1]; p1[2] = origin[2];
    p2[0] = maxs[0]; p2[1] = origin[1]; p2[2] = origin[2];
    vc = R_Add3DLine( s_box, orient, p1, p2, cp, width, vc, kLimit );
    p1[0] = origin[0]; p1[1] = mins[1]; p1[2] = origin[2];
    p2[0] = origin[0]; p2[1] = maxs[1]; p2[2] = origin[2];
    vc = R_Add3DLine( s_box, orient, p1, p2, cp, width, vc, kLimit );
    p1[0] = origin[0]; p1[1] = origin[1]; p1[2] = mins[2];
    p2[0] = origin[0]; p2[1] = origin[1]; p2[2] = maxs[2];
    vc = R_Add3DLine( s_box, orient, p1, p2, cp, width, vc, kLimit );

    // (b) the 12 edges of the mins..maxs box. The 8 corners (binary v42..v65):
    //   c000=mins, c100=(max.x,min.y,min.z), c010=(min.x,max.y,min.z),
    //   c001=(min.x,min.y,max.z), c110=(max.x,max.y,min.z), c101=(max.x,min.y,max.z),
    //   c011=(min.x,max.y,max.z), c111=max.
    const float c000[3] = { mins[0], mins[1], mins[2] };
    const float c100[3] = { maxs[0], mins[1], mins[2] };
    const float c010[3] = { mins[0], maxs[1], mins[2] };
    const float c001[3] = { mins[0], mins[1], maxs[2] };
    const float c110[3] = { maxs[0], maxs[1], mins[2] };
    const float c101[3] = { maxs[0], mins[1], maxs[2] };
    const float c011[3] = { mins[0], maxs[1], maxs[2] };
    const float c111[3] = { maxs[0], maxs[1], maxs[2] };
    // 12 edges in the binary's exact emission order (v11..v22):
    vc = R_Add3DLine( s_box, orient, c000, c100, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c000, c010, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c000, c001, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c100, c110, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c100, c101, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c010, c110, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c010, c011, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c001, c101, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c001, c011, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c110, c111, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c101, c111, cp, width, vc, kLimit );
    vc = R_Add3DLine( s_box, orient, c011, c111, cp, width, vc, kLimit );

    iassert( b->owner->def == b->def->owner );

    // (c) the eclass MODEL bbox (12 more edges) when View→Entities-as-bounding-box is on.
    if ( ( g_PrefsDlg->m_nEntityShowState & 0x1000 ) != 0 )
    {
        // v24 = b->owner->def->eclass; gate on the eclass-model-node flag bit 0x10
        // (the binary's `(*(_BYTE*)(eclass+0x180) & 0x10)==0` — draw unless that bit set).
        eclass_t *eclass = ( (entity_s_def *)b->owner->def )->eclass;
        if ( ( *(unsigned char *)( (char *)eclass + 0x180 ) & 0x10 ) == 0 )
        {
            Byte4PackPixelColor( eclass->color, &col );
            // model bbox = the brush DEF's mins/maxs (brush_t +0x20 / +0x2C).
            brush_t *bdef = b->def;
            const float *mmins = bdef->mins;
            const float *mmaxs = bdef->maxs;
            const float m000[3] = { mmins[0], mmins[1], mmins[2] };
            const float m100[3] = { mmaxs[0], mmins[1], mmins[2] };
            const float m010[3] = { mmins[0], mmaxs[1], mmins[2] };
            const float m001[3] = { mmins[0], mmins[1], mmaxs[2] };
            const float m110[3] = { mmaxs[0], mmaxs[1], mmins[2] };
            const float m101[3] = { mmaxs[0], mmins[1], mmaxs[2] };
            const float m011[3] = { mmins[0], mmaxs[1], mmaxs[2] };
            const float m111[3] = { mmaxs[0], mmaxs[1], mmaxs[2] };
            vc = R_Add3DLine( s_box, orient, m000, m100, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m000, m010, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m000, m001, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m100, m110, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m100, m101, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m010, m110, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m010, m011, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m001, m101, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m001, m011, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m110, m111, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m101, m111, cp, width, vc, kLimit );
            vc = R_Add3DLine( s_box, orient, m011, m111, cp, width, vc, kLimit );
        }
    }

    if ( vc )
        R_AddCmd_Line3D( (short)( vc / 2 ), width, s_box );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x46AE10  Ed_DrawScriptColorQuad  (sub_46AE10) — a flat-colour billboard quad over an
// entity, placed just above the brush-instance bbox top. Drawn UNLIT with d_white as a
// per-vertex-coloured diamond (4 verts, 2 tris). Shared by sub_453EF0 (script_forcecolor)
// and CamWnd_Tokens (camera script-colour tokens; that copy lives in camwnd.cpp).
//   a1 (=int) — the entity DEF (entity_s_def*); reads its def-bbox (+0x60→brush, mins@+0x6C
//               i.e. v2[3..7]) + origin (+0x68 = v2... read raw to match the binary).
//   color     — float[4] RGBA.
// classname strstr("node") raises the quad by +32; the quad is a diamond in
// the XY plane at the bbox top z (+0.01).  One R_AddRenderCmdDrawTris.
// ─────────────────────────────────────────────────────────────────────────────
void Ed_DrawScriptColorQuad( int entDef, const float *color )
{
    GfxColor col;
    Byte4PackPixelColor( const_cast<float *>( color ), &col );

    // The binary: v2 = *(float**)(a1+96) = entity DEF's eclass (entity_s+0x60). It reads
    // the eclass bbox: v2[3]=mins[0]? No — +0x60 is eclass; eclass+0x0C=mins. The IDB
    // reads v2[3..7] = eclass+0x0C..0x1C = eclass mins[0..2]/maxs[0..1]. Matches the
    // origin-relative diamond the binary builds; entity origin is entity_s+0x68 (a1+104..112).
    entity_s_def *e = (entity_s_def *)(intptr_t)entDef;
    eclass_t     *eclass = e->eclass;          // *(float**)(a1+96) = entity_s+0x60
    const float minx = eclass->mins[0], miny = eclass->mins[1], minz = eclass->mins[2];
    const float maxx = eclass->maxs[0], maxy = eclass->maxs[1];   // v2[6], v2[7]
    // classname → raise the quad for "node" entities.
    const char *classname = "";
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, "classname" ) ) { classname = ep->value; break; }

    // origin = entity_s+0x68/0x6C/0x70 (a1+104/108/112).
    float ox = e->origin[0], oy = e->origin[1], oz = e->origin[2];
    if ( strstr( classname, "node" ) )
        oz += 32.0f;
    oz += 0.01f;
    const float qz = minz + oz;        // diamond plane z = eclass mins[2] + origin[2] (+offset)

    // diamond: 4 verts at the bbox mid-edges, in the XY plane (binary xyzw order).
    float xyzw[4][4];
    xyzw[0][0] = minx + ox;                   xyzw[0][1] = ( maxy + miny ) * 0.5f + oy; xyzw[0][2] = qz; xyzw[0][3] = 1.0f;
    xyzw[1][0] = ( minx + maxx ) * 0.5f + ox; xyzw[1][1] = maxy + oy;                   xyzw[1][2] = qz; xyzw[1][3] = 1.0f;
    xyzw[2][0] = maxx + ox;                   xyzw[2][1] = ( maxy + miny ) * 0.5f + oy; xyzw[2][2] = qz; xyzw[2][3] = 1.0f;
    xyzw[3][0] = ( minx + maxx ) * 0.5f + ox; xyzw[3][1] = miny + oy;                   xyzw[3][2] = qz; xyzw[3][3] = 1.0f;

    float normal[4][3] = { {0,0,1},{0,0,1},{0,0,1},{0,0,1} };
    float st[4][2]     = { {0,0},{0,0},{0,0},{0,0} };
    const float fcol   = *(const float *)&col.packed;
    float color4[4]    = { fcol, fcol, fcol, fcol };
    static const uint16_t indices[6] = { 3, 0, 2, 2, 0, 1 };

    if ( !g_qeglobals.d_white )
        return;
    R_AddRenderCmdDrawTris( g_qeglobals.d_white, TECHNIQUE_UNLIT, 6, indices,
                            4, xyzw, normal, color4, st );
}

// 0x453EF0  Ed_DrawScriptForceColor (sub_453EF0) — if the entity carries a
// "script_forcecolor" key, draw its colour billboard over the entity. Faithful vs IDA.
static void Ed_DrawScriptForceColor( selbrush_t *b )
{
    entity_s *owner = b->owner;
    if ( !owner || owner == world_entity )
        return;
    iassert( b->owner->def == b->def->owner );
    int eDef = (int)(intptr_t)owner->def;
    char *fc = ValueForKey2( eDef, "script_forcecolor" );
    if ( fc && fc[0] )
    {
        int ci = ScriptGroup_Unreachable( fc );   // 0..6, or -1 on no match (defensive guard)
        if ( ci >= 0 && ci <= 6 )
            Ed_DrawScriptColorQuad( eDef, kScriptTokenColors[ci] );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47AA20  Brush_GetEntityLineColor  (brush.cpp:4538) — the entity LINE colour picker.
// brushDef = the entity's representative brush DEF; outRgba receives r,g,b,a; inRgba
// supplies the default alpha. worldspawn→0 (no override); "_color" epair→that colour;
// classname "actor*"→eclass.color (spawnflags!=0 forces g=0.8); classtype&0x20 +
// "node_path*" + targetname!="auto"→green. Faithful vs IDA 0x47AA20 — this is the SOLE
// definition; the connections overlay in xywnd.cpp used to carry a second static copy
// that shadowed this one for its callers (the two bodies had drifted: only the copy had
// the 4560 assert and the eclass+0x30 alpha, so the z.cpp/DrawModels callers of THIS one
// silently got alpha=1.0). NULL owner/eclass guards are a defensive divergence on the
// never-run path. Non-static so xywnd.cpp's connections overlay and z.cpp's Z_Draw
// filled-column pass (IDB 0x49b520 calls 0x47aa20 directly) both reuse this body.
// ─────────────────────────────────────────────────────────────────────────────
char Brush_GetEntityLineColor( float *outRgba, brush_t *brushDef, const float *inRgba )
{
    if ( !brushDef ) return 0;
    entity_s *owner = brushDef->owner;
    if ( !owner ) return 0;
    eclass_t *ec = owner->eclass;
    if ( !ec || !ec->name ) return 0;

    if ( !_stricmp( ec->name, "worldspawn" ) )
        return 0;

    const char *colorString = nullptr;
    for ( epair_t *ep = owner->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, "_color" ) )
        {
            colorString = ep->value;
            iassert( colorString );   // brush.cpp:4560 (0x47aa92)
            break;
        }
    }
    if ( !colorString )
        colorString = "";                // IDB `zero` (empty string)

    float r = 0.0f, g = 0.0f, b = 0.0f;
    if ( sscanf( colorString, "%f %f %f", &r, &g, &b ) == 3 )
    {
        outRgba[0] = r; outRgba[1] = g; outRgba[2] = b; outRgba[3] = inRgba[3];
        return 1;
    }

    if ( strncmp( ec->name, "actor", 5 ) != 0 )
    {
        // classtype&0x20 + node_path* + targetname!="auto" → green path line.
        if ( ( ec->classtype & 0x20 ) != 0 && !strncmp( ec->name, "node_path", 9 ) )
        {
            if ( HasKeyValuePair( (entity_s_def *)owner, "targetname" ) )
            {
                char *tn = ValueForKey2( (int)(intptr_t)owner, "targetname" );
                if ( strncmp( tn, "auto", 4 ) != 0 )
                {
                    outRgba[0] = 0.0f; outRgba[1] = 1.0f; outRgba[2] = 0.0f; outRgba[3] = 1.0f;
                    return 1;
                }
            }
        }
        return 0;
    }

    // actor*: eclass.color; spawnflags!=0 forces g=0.8 (keeps r/b/a from eclass).
    // Alpha is the eclass slot at +0x30 (eclass_t::unk), NOT a constant: the binary reads
    // it as a field in BOTH branches (0x47ab5d / 0x47ab3a) right after color[0..2] at
    // +0x24/+0x28/+0x2C.  An earlier copy of this function hardcoded 1.0f here.
    char *sf = ValueForKey2( (int)(intptr_t)owner, "spawnflags" );
    bool sfZero = ( atol( sf ) == 0 );
    outRgba[0] = ec->color[0];
    if ( sfZero )
    {
        outRgba[1] = ec->color[1];
        outRgba[2] = ec->color[2];
        outRgba[3] = ec->unk;
    }
    else
    {
        outRgba[1] = 0.80000001f;   // IDB flt_6F42DC
        outRgba[2] = ec->color[2];
        outRgba[3] = ec->unk;
    }
    return 1;
}

// DrawModels decoration tail (0x4796F0): the origin-box + ScriptGroup colour overlays the
// binary draws after the model mesh. Reproduces DrawModels' per-class colour/extent logic
// (CLASS_MODEL → small ±model_origin_size black box; prefab → ±prefab_origin_size,
// eclass·0.8; brush-entity → eclass mins/maxs + Brush_GetEntityLineColor), the
// ScriptGroup colour quad (sub_479690 gate), the script_forcecolor billboard, and the
// DrawOriginBox call. The MESH itself is the separate Editor_DrawModelsPass.
static void DrawModels_Decorations( selbrush_t *b, const orientation_t *orient,
                                    char width, const GfxColor *brushCol )
{
    entity_s     *owner  = b->owner;
    entity_s_def *eDef   = (entity_s_def *)owner->def;
    eclass_t     *eclass = eDef->eclass;

    float lineCol[4] = { 0.0f, 0.0f, 1.0f, 1.0f };   // a1a/v31/v32/v33
    float mins[3], maxs[3];

    if ( ( eclass->classtype & 0x8 /*CLASS_MODEL*/ ) != 0 )
    {
        lineCol[0] = 0.0f; lineCol[1] = 0.0f; lineCol[2] = 1.0f; lineCol[3] = 1.0f;
        const float s = (float)(int)g_PrefsDlg->model_origin_size;
        mins[0] = mins[1] = mins[2] = -s;
        maxs[0] = maxs[1] = maxs[2] =  s;
    }
    else if ( owner->prefab )
    {
        // prefab (non-model): eclass colour ·0.8, ±prefab_origin_size box.
        // (toggle_unk05 / hidden-model recolour branch is the rare random-colour path; the
        // common branch is eclass·0.8.)
        lineCol[0] = eclass->color[0] * 0.8f;
        lineCol[1] = eclass->color[1] * 0.8f;
        lineCol[2] = eclass->color[2] * 0.8f;
        lineCol[3] = 1.0f;
        const float s = (float)(int)g_PrefsDlg->prefab_origin_size;
        mins[0] = mins[1] = mins[2] = -s;
        maxs[0] = maxs[1] = maxs[2] =  s;
    }
    else
    {
        lineCol[0] = eclass->color[0] * 0.8f;
        lineCol[1] = eclass->color[1] * 0.8f;
        lineCol[2] = eclass->color[2] * 0.8f;
        lineCol[3] = 1.0f;
        mins[0] = eclass->mins[0]; mins[1] = eclass->mins[1]; mins[2] = eclass->mins[2];
        maxs[0] = eclass->maxs[0]; maxs[1] = eclass->maxs[1]; maxs[2] = eclass->maxs[2];
        // binary: Brush_GetEntityLineColor(&a1a, a2->def, &a1a) — brush DEF, default alpha
        // from lineCol[3] (==1.0). Overrides lineCol in place for _color / actor / node_path.
        Brush_GetEntityLineColor( lineCol, b->def, lineCol );
    }

    float origin[3] = { eDef->origin[0], eDef->origin[1], eDef->origin[2] };
    mins[0] += origin[0]; mins[1] += origin[1]; mins[2] += origin[2];
    maxs[0] += origin[0]; maxs[1] += origin[1]; maxs[2] += origin[2];

    // ScriptGroup colour quad — when the entity has the ScriptGroupKey (and it isn't the
    // colour-team key), draw a colour quad keyed off atol(ScriptGroupKey value)&0xF.
    if ( sub_479690_HasScriptGroupKey( b ) )
    {
        const char *sgKey = g_PrefsDlg->ScriptGroupKey;
        const char *val = "";
        for ( epair_t *ep = eDef->epairs; ep; ep = ep->next )
            if ( !_stricmp( ep->key, sgKey ) ) { val = ep->value; break; }
        int idx = atol( val ) & 0xF;
        GfxColor qc;
        Byte4PackPixelColor( const_cast<float *>( kScriptGroupQuadColors[idx] ), &qc );
        const float fcol = *(const float *)&qc.packed;
        float xyzw[4][4], normal[4][3], st[4][2], color4[4];
        const float halfY = ( maxs[1] + mins[1] ) * 0.5f;
        const float halfX = ( mins[0] + maxs[0] ) * 0.5f;
        xyzw[0][0] = mins[0] - 8.0f; xyzw[0][1] = halfY;        xyzw[0][2] = mins[2]; xyzw[0][3] = 1.0f;
        xyzw[1][0] = halfX;          xyzw[1][1] = maxs[1] + 8.0f;xyzw[1][2] = mins[2]; xyzw[1][3] = 1.0f;
        xyzw[2][0] = maxs[0] + 8.0f; xyzw[2][1] = halfY;        xyzw[2][2] = mins[2]; xyzw[2][3] = 1.0f;
        xyzw[3][0] = halfX;          xyzw[3][1] = mins[1] - 8.0f;xyzw[3][2] = mins[2]; xyzw[3][3] = 1.0f;
        for ( int i = 0; i < 4; ++i ) { normal[i][0]=0; normal[i][1]=0; normal[i][2]=1; st[i][0]=0; st[i][1]=0; color4[i]=fcol; }
        static const uint16_t qidx[6] = { 3, 0, 2, 2, 0, 1 };
        if ( g_qeglobals.d_white )
            R_AddRenderCmdDrawTris( g_qeglobals.d_white, TECHNIQUE_UNLIT, 6, qidx, 4, xyzw, normal, color4, st );
    }

    // script_forcecolor billboard (sub_453EF0) — unless this brush is a script trigger.
    if ( !ScriptGroup_BrushIsTrigger( b ) )
        Ed_DrawScriptForceColor( b );

    DrawOriginBox( mins, origin, width, orient, b, maxs, lineCol );
    (void)brushCol;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x478A50  sub_478A50 — "is this brush a CLIP brush?" (any face material named
// "clip*").  Used by DrawModels_PrefabContents' draw gate to SKIP a prefab's clip-
// hull brushes when the prefab is drawn with drawFlags bit 1 set (0x479791: the
// entity carries a non-zero "spawnflags").  Faithful to IDA 0x478a50:
//   def = b->def;  patch = def->patch (def+0x50)
//   if (patch) return strncmp(Materialdef_GetName(&patch->texture), "clip", 4)==0
//   else for each of def->faceCount faces: MtlDef_IsValid(mtldef);
//        if strncmp(Materialdef_GetName(&face->mtldef[0]), "clip", 4)==0 return 1
//        (ANY face named clip -> 1)  return 0
// NOTE: the binary indexes mtldef[0] (face+0x24), NOT current_edit_layer — the clip
// classification is layer-independent.
// ─────────────────────────────────────────────────────────────────────────────
extern "C++" LayerMaterialDef *Materialdef_GetName( MaterialDef *m );   // materialdef.cpp 0x431640

static bool PrefabContent_IsClipBrush( selbrush_t *b )                  // 0x478a50
{
    brush_t *def = b->def;
    if ( def->patch )                                                  // def+0x50 (patchMesh_t*)
    {
        const char *nm = (const char *)Materialdef_GetName(
            (MaterialDef *)&def->patch->texture );                     // patch+0x18
        return nm && strncmp( nm, "clip", 4 ) == 0;                    // 0x478a73
    }
    for ( int i = 0; i < def->faceCount; ++i )                        // def+0x40 count / def+0x44 faces
    {
        MaterialDef *md = &def->faces[i].mtldef[0];                    // face+0x24, mtldef[0]
        iassert( ( ( md->lyrMtl != nullptr ) + ( md->radMtl != nullptr ) ) == 1 );  // MtlDef_IsValid
        const char *nm = (const char *)Materialdef_GetName( md );
        if ( nm && strncmp( nm, "clip", 4 ) == 0 )                    // 0x478ae8
            return true;
    }
    return false;
}


static void DrawBrush_PrefabContents( selbrush_t *bboxBrush, entity_s_def *eDef,
                                      const orientation_t *orient, int viewType,
                                      int technique, GfxColor *col, char width,
                                      int drawFlags, const char *layerPrefix )
{
    entity_s *owner = bboxBrush->owner;
    prefab_s *pf    = (prefab_s *)owner->prefab;
    if ( !pf )
        return;

    // Layer-key prefix for the prefab's content brushes (sub_478B10 0x478b50..0x478c0a):
    // childPrefix = layerPrefix ++ strlwr(<"model" epair value>) ++ "/".  The epair walk
    // (_stricmp key "model", miss → the `zero` empty string) is exactly ValueForKey2.
    extern char *ValueForKey2( int e, const char *key );          // entity.cpp 0x4825C0
    char childPrefix[1028];                                       // v33[1028]
    char modelLc[1024];                                           // v32[1024]
    strcpy( childPrefix, layerPrefix );                           // 0x478b50
    strcpy( modelLc, ValueForKey2( (int)(intptr_t)eDef, "model" ) );
    _strlwr( modelLc );                                           // 0x478ba3
    strcat( childPrefix, modelLc );                               // 0x478bd8
    strcat( childPrefix, "/" );                                   // 0x478c0a

    // Forward orientation of the placed prefab (sub_478B10: Entity_GetOrientation
    // on the entity DEF through the caller's matrix).
    orientation_t prefabOrient;
    Entity_GetOrientation( eDef, (orientation_t *)orient, &prefabOrient );

    // 0x478c1d — the per-content-brush CULL setup.  The binary sets up the ACTIVE VIEW's
    // clip planes in the PREFAB's local space so it can cull each content brush BEFORE
    // DrawBrush, then restores the planes to the caller's orientation at the end:
    //   camera (a5 < 0): sub_405620(m_pCamWnd, &prefabOrient) → CullCubic per brush;
    //   XY     (a5 >= 0): CXYWnd_SetupClipPlanes(&prefabOrient, m_pActiveXY) → sub_46CD80.
    // Without the cull every prefab-content model is skinned: mp_backlot camera overflow
    // (missing buildings, fixed 4cec231c) and — with the XY branch skipped — blackout's XY
    // pass attempted ~23k model draws/frame, exhausting radiant_modelSkinnedSurfs[0x4000]
    // and silently dropping ~7.7k models (XY cull ported 2026-07-05, XY_CullBrush).
    // Guarded on the wnd pointers — NULL headless → no cull (the headless selftest never
    // draws prefab contents anyway).
    extern CMainFrame *g_pParentWnd;                          // 0x25D5A70
    extern char CullCubic( selbrush_t *brush, CCamWnd *cam ); // camwnd.cpp 0x4056d0
    extern char XY_CullBrush( CXYWnd *xy, selbrush_t *b );    // xywnd.cpp 0x46cd80
    CCamWnd *cullCam = ( viewType < 0 && g_pParentWnd ) ? g_pParentWnd->m_pCamWnd : nullptr;
    if ( cullCam )
        cullCam->Cam_SetupClipPlanes( (const float *)&prefabOrient );   // 0x478c3f
    // 0x478c65/0x478c7d — the XY branch: g_pParentWnd->m_pActiveXY (fall back to the main
    // XY pane when no view has taken focus yet — the binary asserts non-NULL instead).
    CXYWnd *cullXY = nullptr;
    if ( viewType >= 0 && g_pParentWnd )
    {
        cullXY = g_pParentWnd->m_pActiveXY ? g_pParentWnd->m_pActiveXY : g_pParentWnd->m_pXYWnd;
        if ( cullXY )
            cullXY->XY_SetupClipPlanes( (const float *)&prefabOrient ); // 0x478c7d
    }

    // Iterate the prefab's instanced brush list (active_brushlist_next .. sentinel
    // &active_brushlist, linked via selbrush_t.next) and draw each.
    selbrush_t *sentinel = (selbrush_t *)&pf->active_brushlist;

    for ( selbrush_t *pb = pf->active_brushlist_next;
          pb && pb != sentinel;
          pb = pb->next )
    {
        // 0x478cbb — CullCubic per content brush (camera view, in the prefab's local
        // space set up above).  Cull → skip skinning this content brush entirely.
        if ( cullCam && CullCubic( pb, cullCam ) )
            continue;
        // 0x478cff — sub_46CD80 per content brush (XY view): outside the 2D view rect
        // (planes in prefab-local space) → skip.  This is the cull whose absence made the
        // XY pass queue every content model map-wide (the blackout FRAMEDROP overflow).
        if ( cullXY && XY_CullBrush( cullXY, pb ) )
            continue;
        // 0x478d9a — the binary's draw gate: draw UNLESS FilterBrush hides it, OR
        // (drawFlags bit 1 set AND it's a clip brush).  A misc_prefab placed with a
        // non-zero "spawnflags" sets bit 1 (DrawModels 0x479791) → its clip-hull
        // brushes are NOT drawn.  Missing this gate rendered the vehicle prefab's
        // clip hull as an opaque wc/$opaque white shell over the model (BRUSHPROBE:
        // wc/$opaque, opaque loadBits, fakelight_normal(24)). [sub_478A50 0x478a50]
        // NB: FilterBrush honours the per-filter isShown state, which the binary seeds
        // from GetProfileIntA("Filters",name,1) — DEFAULT 1 (SHOWN) for a fresh registry.
        // The stock CoD4Radiant.exe has NO "Filters" registry subkey, so every filter is
        // shown and all tool volumes (Hint/Portal/Clip/LightGridVolume/Caulk) render.  The
        // port reads its OWN app registry (HKCU\...\KisakCOD-Radiant\Filters); a session that
        // toggled filters there leaves them unchecked → those volumes hide.  That is FAITHFUL
        // behaviour (matches the binary given the same registry) — to reproduce the stock
        // default, clear that Filters subkey.  [tool-volume audit 2026-07-04]
        // 0x478d81 — the binary's FilterBrush 2nd arg is the fast-2D-drag flag, TRUE only
        // while a drag is active AND fast_2d_view_dragging is on AND this is the XY path
        // (v24 = g_qeglobals.toggle_unk02 && g_PrefsDlg->fast_2d_view_dragging && v31).
        // (The port passed a literal 0 = never fast-filter; parity restored with the cull.)
        {
            const char fastDrag = ( g_qeglobals.toggle_unk02
                                    && g_PrefsDlg->fast_2d_view_dragging
                                    && cullXY != nullptr ) ? 1 : 0;
            if ( FilterBrush( pb, fastDrag ) )
                continue;
        }
        if ( ( drawFlags & 2 ) != 0 && PrefabContent_IsClipBrush( pb ) )
            continue;
        DrawBrush( pb, &prefabOrient, viewType, technique, col, width, drawFlags,
                   childPrefix );
    }

    // 0x478df3 — restore the clip planes to the CALLER's orientation (recursion-safe
    // for nested prefabs).  Binary: sub_405620(m_pCamWnd, a4) / CXYWnd_SetupClipPlanes(a4, xy).
    if ( cullCam )
        cullCam->Cam_SetupClipPlanes( (const float *)orient );          // 0x478dfa
    if ( cullXY )
        cullXY->XY_SetupClipPlanes( (const float *)orient );            // 0x478e19
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x47afc0  DrawBrush  (brush.cpp:4729) — now the FULL dispatch: layer vis refresh →
// patch branch → Brush_CheckBuildFaceVis (0x47b07c, feeds DrawGeo's filled branches)
// → fixedsize entity: DrawAngles + the DrawModels slice (Entity_HasRenderableModel →
// xmodel MESH via SkinModelInst + prefab CONTENTS via DrawBrush_PrefabContents, then
// return) → else DrawGeo on the (entity-local for fixedsize) geometry.
// Still deferred vs the IDB: the filled patch camera draw (DrawPatches /
// DrawPatchCameraFilled) and the g_PrefsDlg technique/line-width toggles.
// ─────────────────────────────────────────────────────────────────────────────
void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                int technique, GfxColor *col, char width, int drawFlags,
                const char *layerPrefix )
{
    // 0x47aff5 — FIRST: refresh this instance's layer-derived hide/freeze flags from the
    // layer map (lazy, g_layerCount_maybe-generation-cached).
    extern void Brush_UpdateLayerVis( selbrush_t *b, const char *layerPrefix );  // layers.cpp 0x418990
    Brush_UpdateLayerVis( b, layerPrefix );

    // Drawn unless hidden-by-layer (brushFlags bit 1) and not force-drawn (drawFlags bit 0).
    if ( ( drawFlags & 1 ) == 0 && ( b->brushFlags & 2 ) != 0 )
        return;

    // [FAITHFULNESS REMEDIATION 2026-07-03] The binary's DrawBrush (0x47afc0) NEVER pushes a
    // per-brush MATERIAL_COLOR — the removed adaptation here did.  It was harmful two ways:
    //   (1) for a SELECTED MODEL drawn at tech 29 it set MATERIAL_COLOR white (w=1) which
    //       FLAT-OVERRODE the vertcol_shaded mesh to solid white (the operator's "textures turn
    //       white when selecting").  The binary instead colours the model wireframe via the
    //       PER-VERTEX stamp (SkinModelInst 0x4fe455, colorPtr=white), now restored above.
    //   (2) it was REDUNDANT for the LINE path: DrawShadedWireframe / the wireframe builders
    //       write the brush colour into each GfxPointVertex.color, and R_AddCmd_Line3D already
    //       carries that per-vertex colour to the $line UNLIT draw (RB_DrawLines3D 0x5332b0 sets
    //       tess vert color = GfxPointVertex.color, no MATERIAL_COLOR) — exactly the binary.
    // So the push is DELETED; line colour comes from the per-vertex colour (binary mechanism),
    // model wireframe colour from the per-vertex stamp.  (No MATERIAL_COLOR touched on this path.)

    // PATCH brushes — the binary's DrawBrush patch dispatch (0x47b01d):
    //   draw_meth2 == 29 && viewType <= 2  → DrawPatchesWireframeGrid  (2D views, wireframe)
    //   draw_meth2 == 29 && viewType  > 2  → DrawPatchCameraFilled     (0x4415d0)
    //   else (draw_meth2 != 29, e.g. the world draw at tech 24) → DrawPatches (0x4414e0):
    //        the FILLED per-layer surf-cache draw (PMESH_27_CheckVersion → PMESH_26_CheckFace →
    //        Editor_AddMeshCmd) UNLESS g_PrefsDlg->patch_wireframe is set, in which case it SKIPS
    //        the filled top-face and draws DrawPatchesWireframeGrid with colors[23].
    //
    // The 3D world pass arrives with viewType=-1 and a non-wireframe technique, so it
    // takes DrawPatches and emits through the editor surf cache.
    if ( b->patch )
    {
        if ( technique == 29 )
        {
            // 0x47B025: view types 0..2 use the 2D grid; the camera uses 0x4415D0.
            if ( (unsigned)viewType <= 2 )
            {
                // 2D views (wireframe technique): the tessellated grid, entity colour.
                DrawPatchesWireframeGrid( b->patch, col, orient, width, drawFlags );
            }
            else
            {
                // 3D camera: 0x4415D0 — d_white at TECHNIQUE_WIREFRAME_SOLID, both windings.
                extern bool DrawPatchCameraFilled( patch_t *inst, const orientation_t *orient,
                                                   int drawFlags );   // pmesh.cpp 0x4415d0
                DrawPatchCameraFilled( b->patch, orient, drawFlags );
            }
        }
        else
        {
            extern bool DrawPatches( patch_t *inst, const orientation_t *orient,
                                     int techType, int drawFlags ); // pmesh.cpp 0x4414E0
            if ( !DrawPatches( b->patch, orient, technique, drawFlags ) )
            {
                // Defensive asset-gap fallback; stock assumes the instance visuals exist.
                GfxColor pcol;
                Byte4PackPixelColor( g_qeglobals.d_savedinfo.colors[23], &pcol );   // COLOR_PATCH
                DrawPatchesWireframeGrid( b->patch, &pcol, orient, width, drawFlags );
            }
        }
        return;
    }

    // 0x47b07c — Brush_CheckBuildFaceVis: (re)build this instance's per-face visuals
    // (Visuals_InitFaceVis → world-space GfxWorldVertex runs uploaded to the editor VB
    // pool) when the def version advanced.  This is what DrawGeo's FILLED branches and
    // the selection paths consume.  [Was skipped while DrawGeo was line-only.]
    sub_477D70( b, (const float *)orient );

    iassert( b->owner->def == b->def->owner );

    entity_s_def *eDef   = (entity_s_def *)b->owner->def;
    eclass_t     *eclass = eDef->eclass;

    const orientation_t *useOrient = orient;
    orientation_t        localOrient;
    if ( eclass && *(int *)&eclass->fixedsize )
    {
        // Entity ANGLE ARROW overlay (View→Show→Angles, d_xyShowFlags bit 0x2 SET = hidden).
        // IDB gate: (drawFlags&1) || ((d_xyShowFlags&2)==0 && (classtype&2)!=0), and owner!=0.
        // The binary threads `b` to DrawAngles in ECX; we pass it via g_drawAnglesBrush.
        if ( ( ( drawFlags & 1 ) != 0
            || ( ( g_qeglobals.d_savedinfo.d_xyShowFlags & 2 ) == 0 && ( eclass->classtype & 2 ) != 0 ) )
            && b->owner )
        {
            extern selbrush_t *g_drawAnglesBrush;
            g_drawAnglesBrush = b;
            DrawAngles( viewType, orient, col );
        }
        // 0x47b102 — the DrawModels dispatch: when the entity has a renderable model or
        // prefab (Entity_HasRenderableModel 0x479610 = show-state filter + Model_SetModel),
        // draw it via the DrawModels (0x4796f0) slice and RETURN — the bbox below is only
        // for entities with no model.  The instance+skin runs guarded (ERR_DROP + SEH,
        // camwnd.cpp Editor_InstanceAndSkinModel) so a CoD4-format asset the CoD3 loader
        // rejects skips that model instead of crashing (model-render epic Stage B).
        // RADIANT_MODELS=0 escape hatch disables the xmodel MESH; prefab classes always
        // dispatch (their contents drew before the model epic and must keep drawing).
        extern bool Editor_ModelsEnabled();                                    // camwnd.cpp
        extern int  Editor_InstanceAndSkinModel( selbrush_t *b, const orientation_t *orient,
                                                 int meshTech, GfxColor *col, int drawFlags ); // camwnd.cpp
        const bool prefabClass = ( eclass->classtype & 0x10 /* CLASS_PREFAB */ ) != 0;
        if ( ( prefabClass || Editor_ModelsEnabled() )
             && !( b->def->unk01 & 0xFF )               // LOBYTE = modelFailed (0x47b102)
             && Editor_InstanceAndSkinModel( b, orient, technique, col, drawFlags ) )
        {
            // DrawModels 0x47974c: prefab CONTENTS.  spawnflags != 0 → drawFlags |= 2
            // (the MaterialDef_15_Drawflag_Multiply skip bit), per 0x479785.
            if ( b->owner->prefab )
            {
                extern char *ValueForKey2( int e, const char *key );          // entity.cpp 0x4825C0
                int contentFlags = drawFlags;
                if ( atol( ValueForKey2( (int)(intptr_t)eDef, "spawnflags" ) ) )
                    contentFlags |= 2;
                // draw_meth1 role (0x47b11a): View→Entities-as-wireframe (show-state bit 0)
                // forces the content GEOMETRY to 29; the xmodel mesh keeps `technique`
                // (the draw_meth2 role — SkinModelInst @0x479735 uses draw_meth2).
                int contentTech = ( g_PrefsDlg->m_nEntityShowState & 1 ) ? 29 : technique;
                DrawBrush_PrefabContents( b, eDef, orient, viewType, contentTech, col,
                                          width, contentFlags, layerPrefix );
            }
            // DrawModels' decoration tail (origin box / ScriptGroup quad) — default-off.
            extern bool Radiant_DecorEnabled();   // camwnd.cpp
            if ( Radiant_DecorEnabled() )
                DrawModels_Decorations( b, orient, width, col );
            return;                              // model/prefab drawn — no bbox (0x47b151)
        }

        // DrawModels decoration tail — the entity ORIGIN BOX + ScriptGroup colour
        // billboards.  The binary draws these inside DrawModels (reached for fixedsize
        // entities via Entity_HasRenderableModel); here they ride the FORWARD orientation
        // (`orient`, not the inverse the placeholder bbox uses below), matching the binary's
        // DrawModels which takes `ident_mtx` straight through to DrawOriginBox.  Drawn for
        // ALL fixedsize point entities (model / brush-rep / non-instanced-prefab) — the
        // mesh itself is the separate Editor_DrawModelsPass.  [render decorations, opus]
        // DEFAULT-OFF (RADIANT_DECOR), shared with the camwnd decoration layer — see
        // camwnd.cpp Radiant_DecorEnabled(); keeps the default render path unchanged.
        extern bool Radiant_DecorEnabled();   // camwnd.cpp
        if ( Radiant_DecorEnabled() )
            DrawModels_Decorations( b, orient, width, col );

        Entity_GetOrientationInverse( eDef, (orientation_t *)orient, &localOrient );
        useOrient = &localOrient;
    }

    DrawGeo( col, nullptr, b, useOrient, viewType, technique, width, drawFlags );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x475cb0  Brush_MemorySize  (105 bytes)
// Returns the heap size of a brush_t def (struct + parent_layer_string + faces
// including each face's material layer strings).
// __usercall: a1@<esi>
// ─────────────────────────────────────────────────────────────────────────────
// the first _msize is def->patch (0x50), NOT parent_layer_string (0x48)
// (disasm `mov eax,[esi+50h]`); base 88, faceCount@0x40, faces@0x44, face_t.w@0xE0=224,
// stride 232.
size_t Brush_MemorySize( brush_t *def )
{
    // IDA 0x475cb4: v1 = (void*)a1[20] = *(esi+0x50) = def->patch (a1[20]=80=0x50, NOT
    // parent_layer_string@0x48 — disasm `mov eax,[esi+50h]`); base = 0x58 = 88 (= sizeof brush_t).
    size_t total = 88;
    if ( def->patch )
        total += _msize( def->patch );
    if ( def->faceCount )
    {
        for ( int i = 0; i < def->faceCount; ++i )
        {
            // IDA: *(DWORD*)(v3 + a1[17] + 224) = per-face winding or layer str
            face_t *f = &def->faces[i];
            // face_t stride = 232; material string is at face+224 (IDA: +224 offset)
            void *faceStr = *(void **)((char *)f + 224);
            if ( faceStr )
                total += _msize( faceStr ) + 232;
            else
                total += 232;
        }
    }
    return total;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x475e80  Brush_FullClone_sub475E80  (375 bytes)
// Clones a brush DEF node (entity_brush_s = brush_t_with_custom_def).
// Arguments: a1 is typed entity_brush_s* but it IS a brush_t_with_custom_def
// (the 0x58-byte def node, NOT the 56-byte instance).  The IDA indexing
// `a1[1].*` steps by 56 bytes into what is actually the brush_t part of the
// brush_t_with_custom_def — treating the struct as two packed selbrush_t-sized
// blocks.  The relevant fields:
//   a1[1].unk1       = *(int*)((char*)a1 + 56 + unk1_offset)
//                    = brush_t_with_custom_def->patch field (offset 80)
//   a1[1].refCount   = *(int*)((char*)a1 + 56 + refCount_offset)
//                    = brush_t_with_custom_def->version (offset 78+4? = 82 → wrong)
// Actually IDA clarifies:
//   v5[21] = (void*)a1[1].refCount   → IDA raw: *(a1 + 21*4) = a1+84 = brush_t.patch
//   v5[2]  = a1->owner               → owner pointer
//   v5[8..13] = a1.mins/maxs
//   v5[16] = a1[1].owner = faceCount → *(a1 + 16+2 DWORD offsets in brush_t) ...
// The cleanest port re-reads the original sub_475E80 against the known brush_t layout:
//   offset 0x50 = 80 = patch (from brush_t static_assert via qe3.h brush_t.patch==80)
//   offset 0x4E = 78 = version
//   faceCount is at offset 64 (brush_t.faceCount from IDB), faces at offset 68
// __cdecl: a1 = entity_brush_s* (= brush_t_with_custom_def* in practice)
// ─────────────────────────────────────────────────────────────────────────────
brush_t_with_custom_def *Brush_FullClone_sub475E80( entity_brush_s *a1 )
{
    // Cast to brush_t_with_custom_def for field access
    brush_t_with_custom_def *src = (brush_t_with_custom_def *)a1;

    // IDA: unk1 = (patchMesh_t*)a1[1].unk1
    // a1[1] steps by sizeof(entity_brush_s)=56 bytes; .unk1 is at offset 20 in selbrush_t
    // So a1[1].unk1 = *(int*)((char*)a1 + 56 + 20) = a1 + 76
    // But brush_t.patch is at offset 80 in brush_t_with_custom_def (static_assert confirmed)
    // The IDA indexing is off by one struct field due to the composite cast.
    // Cross-referencing Brush_Clone: src->def->patch == the patch field.
    // For Brush_FullClone the argument IS the def, so src->patch directly.
    patchMesh_t *pPatch = (patchMesh_t *)(intptr_t)src->patch;

    if ( pPatch )
    {
        patchMesh_t *dup = Patch_Duplicate( pPatch );
        Entity_UnlinkBrush( (brush_t_with_custom_def *)dup->pSymbiot );
        brush_t_with_custom_def *ps = (brush_t_with_custom_def *)dup->pSymbiot;
        ps->owner = (entity_s *)(intptr_t)src->owner;
        Brush_BuildWindings( (brush_t *)ps, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++ps->version;
        return ps;
    }
    else
    {
        brush_t_with_custom_def *b = (brush_t_with_custom_def *)::operator new( 0x58u );
        memset( b, 0, 0x58u );
        iassert( b );

        // parent_layer_string: IDA v5[18] (offset 72 in new alloc = parent_layer_string)
        // The freshly zeroed alloc has parent_layer_string==nullptr so no free needed.
        // Copy from g_activeLayer_string (same as Brush_Clone)
        size_t layerLen = strlen( g_activeLayer_string );
        void *layerCopy = ::operator new( layerLen + 1 );
        memcpy( layerCopy, g_activeLayer_string, layerLen + 1 );
        b->parent_layer_string = (char *)layerCopy;

        // IDA: v5[2] = a1->owner;
        b->owner = (entity_s *)(intptr_t)src->owner;

        // IDA: v5[21] = (void*)a1[1].refCount; v5[21] = offset 84 = 0x54.  The disasm
        // (mov eax,[ebx+54h]; mov [esi+54h],eax) copies offset 0x54 -> 0x54 = numberId
        // (NOT version @ 0x4E -- an earlier port read mistakenly mapped this to version).
        // The binary does NOT copy version here; the clone keeps version 0 from the memset.
        b->numberId  = src->numberId;
        b->faceCount = src->faceCount;

        // IDA: v5[8..13] = a1.mins[0..2] / a1.maxs[0..2]
        b->mins[0] = src->mins[0];
        b->mins[1] = src->mins[1];
        b->mins[2] = src->mins[2];
        b->maxs[0] = src->maxs[0];
        b->maxs[1] = src->maxs[1];
        b->maxs[2] = src->maxs[2];

        // Allocate + deep-clone faces
        b->faces = Face_Alloc_R( src->faceCount );
        for ( int i = 0; i < src->faceCount; ++i )
            Face_FullClone( (int)((char *)src->faces + i * 232),
                            (int)((char *)b->faces   + i * 232) );

        return b;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Texture/lightmap-LOCK reprojection (texturevecs.cpp lineage; brush.cpp here).
//  Wrapped around the inline planept translate in Brush_Move: Save snapshots each
//  locked face layer's WORLD texcoords at the 3 plane points BEFORE the move;
//  Reproject re-derives the texdef AFTER the move so the texture stays "locked" to
//  the geometry.  Ported 1:1 from CoD4Radiant.exe (disasm = ground truth):
//    Face_TexLock_Save       0x470570
//    Face_TexLock_Reproject  0x4706F0
//  Deps (all ported): Face_MoveTexture 0x45A1C0, texturevecs_02 0x459CC0,
//  Face_MakePlane 0x470470, MaterialDef_04 0x431740, and the linearmapping.cpp
//  LU solver (LinearMapping_Setup/Apply).
//
//  face_t layout used (qe3.h):
//    planepts[3] @0x00 ; mtldef[4] @0x24 (36 each) ; plane.normal @0xC0 ;
//    plane.dist (double) @0xD0 ; w @0xE0.  Per layer the texdef is a 28-byte
//    texdef_sub_t {size[2]@0, shift[2]@8, rotate@0x10, crossterm/unk3@0x14}.
//  The binary walks `td.rotate` (= mtldef[c].mat_texDef + 0x10) and reads/writes
//  size@-0x10, shift@-8, rotate@0, crossterm@+4 relative to it — transcribed
//  verbatim with char*/float* offsets to avoid struct-stride misreads.
// ═════════════════════════════════════════════════════════════════════════════

// 0x470570  Face_TexLock_Save(saveBuf, face): per (channel 0..2, layer) with a
// real material, build the world tex matrix (Face_MoveTexture) and store the S/T
// texcoords at the face's 3 plane points into saveBuf.  IDA: saveBuf base + 8,
// then +0x18 (6 floats) per layer; channel base also +0x18 per channel (so the
// effective layout assumes 1 layer/channel — radMtl → MaterialDef_04 == 1).
static void Face_TexLock_Save( float *saveBuf, face_t *face )
{
    float *chanBase = saveBuf + 2;                         // IDA: ecx + 8 (= 2 floats)
    // td0 = &mtldef[0].mat_texDef.rotate  (face + 0x3C)
    char  *chanTd   = (char *)face + 0x3C;                 // IDA: var_4 = edi + 0x3C

    for ( int ch = 0; ch < 3; ++ch )                       // var_14 = 3 channels
    {
        MaterialDef *mtl = (MaterialDef *)( chanTd - 0x18 );   // IDA: var_4 - 0x18 = &mtldef[ch]
        int layers = MaterialDef_04( mtl );
        if ( layers > 0 )
        {
            char  *td  = chanTd;                           // var_8 (walks +0x1C/layer)
            float *out = chanBase;                         // ebx   (walks +0x18/layer)
            for ( int L = 0; L < layers; ++L )
            {
                // Build the WORLD texture matrix from this layer's texdef + face normal.
                // td points at the layer's rotate field, so td-0x10 is the texdef base.
                texdef_sub_t *tds = (texdef_sub_t *)( td - 0x10 );
                float texMat[8];
                Face_MoveTexture( (int)(intptr_t)tds,                 // size[2]
                                  &face->plane.normal[0],             // edi + 0xC0
                                  (int)(intptr_t)texMat,              // out matrix (var_38..)
                                  (int)(intptr_t)tds->shift,          // shift[2]
                                  tds->rotate,
                                  tds->crossterm );

                const float *pp = &face->planepts[0][0];   // edi (planepts[0..2])
                // S/T at planepts[0], [1], [2] via the world tex matrix (texMat row0=S, row1=T).
                // Per-layer the 6 floats land at out[-2..+3] (out = saveBuf + 2 floats):
                //   [-2]=S0 [-1]=T0 [0]=S1 [1]=T1 [2]=S2 [3]=T2.  The binary writes S2/T2
                //   AFTER its `v2 += 6` so they land at (newv2)-4/(newv2)-3 = oldv2+2/+3 —
                //   i.e. out[2]/out[3] here (NOT out[-4]/out[-3]); confirmed vs the hex-rays.
                float s0 = texMat[0]*pp[0] + texMat[1]*pp[1] + texMat[2]*pp[2] + texMat[3];
                float t0 = texMat[4]*pp[0] + texMat[5]*pp[1] + texMat[6]*pp[2] + texMat[7];
                out[-2] = s0;
                out[-1] = t0;
                float s1 = texMat[0]*pp[3] + texMat[1]*pp[4] + texMat[2]*pp[5] + texMat[3];
                float t1 = texMat[4]*pp[3] + texMat[5]*pp[4] + texMat[6]*pp[5] + texMat[7];
                out[0] = s1;
                out[1] = t1;
                float s2 = texMat[0]*pp[6] + texMat[1]*pp[7] + texMat[2]*pp[8] + texMat[3];
                float t2 = texMat[4]*pp[6] + texMat[5]*pp[7] + texMat[6]*pp[8] + texMat[7];
                out[2] = s2;
                out[3] = t2;

                td  += 0x1C;                                // next texdef_sub_t
                out += 6;                                   // ebx += 0x18
            }
        }
        chanTd   += 0x24;                                  // next MaterialDef
        chanBase += 6;                                     // var_10 += 0x18
    }
}

// 0x4706F0  Face_TexLock_Reproject(face, saveBuf, lockFlags): Face_MakePlane, then
// for each channel whose lockFlags byte is set AND MaterialDef_04 > 0, re-derive
// the texdef so the (moved) plane points still map to the saved S/T.  Per layer:
// solve the S-affine row and T-affine row via the LU solver (one Setup over the 3
// moved plane points, two Applys for S and T), then decompose the recovered 2-row
// world tex matrix back into a texdef via texturevecs_02 (the inverse of
// Face_MoveTexture), writing it into this layer's texdef record in place.
static void Face_TexLock_Reproject( face_t *face, const float *saveBuf,
                                    const unsigned char *lockFlags )
{
    Face_MakePlane( face );                                 // 0x470470 (re-derive plane)

    // Setup the LU system once from the 3 (moved) plane points projected onto the
    // plane-normal major axis.  IDA: Setup(lm, face+0xC0(normal), face+0x18(p2),
    // face+0(p0), face+0xC(p1)).  NOTE the binary passes p2=&planepts[2] as the
    // a3 arg and p0/p1 as a4/a5 (Setup's (lm,normal,p2,p0,p1) order).
    LinearMapping lm;
    if ( !LinearMapping_Setup( &lm,
                               &face->plane.normal[0],            // a2 = face + 0xC0
                               &face->planepts[2][0],             // a3 = face + 0x18 (planepts[2])
                               &face->planepts[0][0],             // a4 = face + 0x00 (planepts[0])
                               &face->planepts[1][0] ) )          // a5 = face + 0x0C (planepts[1])
        return;

    const char *chanSave = (const char *)saveBuf + 8;      // IDA: a3 + 8
    char       *chanTd   = (char *)face + 0x3C;            // var_1C: &mtldef[0].td.rotate

    for ( int ch = 0; ch < 3; ++ch )                       // var_14: 3 channels
    {
        if ( lockFlags[ch] )                               // [ebx + a4] != 0
        {
            MaterialDef *mtl = (MaterialDef *)( chanTd - 0x18 );
            int layers = MaterialDef_04( mtl );
            if ( layers > 0 )
            {
                const float *sv = (const float *)chanSave; // esi (saveBuf walker, +0x18/layer)
                char        *td = chanTd;                  // var_8 (output texdef, +0x1C/layer)
                for ( int L = 0; L < layers; ++L )
                {
                    // Two rows of the world tex matrix recovered by the LU solve.
                    // texMat[0..3] = S row (var_44), texMat[4..7] = T row (var_34).
                    float texMat[8];
                    // S row: input scalar coords = saved S at the 3 points.
                    //   IDA Apply floats: [esi-2], [esi], [esi+2]  (sv[-2], sv[0], sv[2])
                    LinearMapping_Apply( &lm, &texMat[0], sv[-2], sv[0], sv[2] );
                    // T row: input = saved T at the 3 points.
                    //   IDA Apply floats: [esi-1], [esi+1], [esi+3]  (sv[-1], sv[1], sv[3])
                    LinearMapping_Apply( &lm, &texMat[4], sv[-1], sv[1], sv[3] );

                    // Decompose the 2-row world tex matrix back into a texdef, writing
                    // size@td-0x10, shift@td-8, rotate@td, crossterm@td+4.
                    // face + 0xD0: the binary fld's qword here, but the kisak port
                    // stores plane.dist as a float (see Face_MakePlane §11 note) —
                    // readers reconstitute (double); the float value is authoritative.
                    float planeDist = face->plane.dist;
                    texturevecs_02( (int)(intptr_t)( td - 0x10 ),  // outSize (edi = eax-0x10)
                                    (int)(intptr_t)texMat,         // texMat (esi = &var_44)
                                    0.0f,                          // phantom (x87 artifact)
                                    (int)(intptr_t)&face->plane.normal[0],  // normal (var_24)
                                    planeDist,                     // plane dist
                                    (int)(intptr_t)( td - 8 ),     // outShift (ecx = eax-8)
                                    (int)(intptr_t)( td ),         // outRotate (eax)
                                    (int)(intptr_t)( td + 4 ) );   // outCrossterm (edx = eax+4)

                    sv += 6;                               // var_C += 0x18
                    td += 0x1C;                            // var_8 += 0x1C
                }
            }
        }
        chanSave += 0x18;                                  // (channel base advances 6 floats)
        chanTd   += 0x24;                                  // next MaterialDef
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  0x47ba40  Brush_Move  (P5.2 — the leaf brush-translate)
//
//  Translates one brush definition by `move` (vec3) and rebuilds it. Called for
//  every selected brush by Select_Move (select.cpp). Returns the owning entity.
//
//  Faithful to IDA sub_47BA40.  Texture/lightmap-LOCK reprojection NOW WIRED
//  (linearmapping.cpp landed 2026-06-28):
//
//  IDA builds a 3-byte lockFlags { texLock=m_bTextureLock!=0, lightmapLock=
//  m_bLightmapLock!=0, const1=1 } ONCE, then per face that has a winding
//  (face->w, the +224 gate):
//      Face_TexLock_Save(saveBuf, face)        (0x470570) — snapshot the WORLD S/T
//                                                at the 3 plane points (pre-move)
//      planepts[0..2] += move                  — the actual translate (inline)
//      Face_TexLock_Reproject(face, saveBuf, &lockFlags)  (0x4706F0)
//                                    — Face_MakePlane(face) + a per-channel texture
//                                      reproject GATED on the lock bytes AND
//                                      MaterialDef_04(layer) > 0, via the LU solver.
//  Brush_BuildWindings (called right after, exactly as IDA) recomputes every face
//  plane + winding — so the per-face Face_MakePlane inside Reproject is redundant
//  for geometry; geometry is unaffected, only the texdef-lock behaviour is added.
//  Default m_bTextureLock=1 so brush moves now reproject the texture by default
//  (matching the binary); .map texdefs change on move iff the lock is on (faithful).
// ─────────────────────────────────────────────────────────────────────────────
extern curvePatchDef_t *Patch_Move( const float *move, patchMesh_t *p );   // 0x441dd0 (pmesh.cpp)

entity_s *Brush_Move( const float *move, brush_t *def, char snap )
{
    // Texture/lightmap lock flags struct (IDA a4): {LOBYTE=texLock, BYTE1=lightmapLock,
    // BYTE2=1}.  Built once before the face loop, exactly as the binary.
    unsigned char lockFlags[3];
    lockFlags[0] = (unsigned char)( g_PrefsDlg->m_bTextureLock  != 0 );   // texLock
    lockFlags[1] = (unsigned char)( g_PrefsDlg->m_bLightmapLock != 0 );   // lightmapLock
    lockFlags[2] = 1;                                                     // const1

    // saveBuf: IDA `int a3[19]` (76 bytes) on Brush_Move's stack, passed to both
    // Save and Reproject.  Per face it stashes the channel/layer world S/T.
    float saveBuf[19];

    // Translate every face that has a built winding (IDA gates on face->w @0xE0),
    // wrapping the planept translate with Save (before) and Reproject (after).
    for ( int i = 0; i < def->faceCount; ++i )
    {
        face_t *f = &def->faces[i];
        if ( !f->w )
            continue;

        Face_TexLock_Save( saveBuf, f );           // 0x470570 (snapshot pre-move S/T)

        for ( int p = 0; p < 3; ++p )
        {
            f->planepts[p][0] += move[0];
            f->planepts[p][1] += move[1];
            f->planepts[p][2] += move[2];
        }

        Face_TexLock_Reproject( f, saveBuf, lockFlags );   // 0x4706F0 (re-derive texdef)
    }

    Brush_BuildWindings( def, snap );          // recomputes planes (Face_MakePlane) + windings
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++def->version;                            // IDA: ++HIWORD(def[1].def) == ++def->version (i16@0x4E)

    if ( def->patch )
        Patch_Move( move, def->patch );

    entity_s *ent = def->owner;
    if ( ent->eclass && ent->eclass->fixedsize )
    {
        ent->origin[0] += move[0];
        ent->origin[1] += move[1];
        ent->origin[2] += move[2];
        // IDA ++LOWORD(owner[1].xx9): the 16-bit render-version counter at entity+0x78.
        ++*(unsigned __int16 *)( (char *)ent + 0x78 );
    }
    return ent;
}

// ── Brush_SnapToGrid (0x4783D0) — snap a brush's geometry to the grid ─────────
//   Reached by Patch_SnapVertToGrid (pmesh.cpp) for the non-patch brushes of a mixed
//   selection.  For a fixedsize entity, snaps the owner's ORIGIN and moves the brush by
//   the delta (Brush_Move).  Otherwise clones the brush, snaps every face's plane points
//   to the grid, rebuilds; if the snapped brush collapsed below 1 unit on any axis, swaps
//   the original faces back and warns.  Verbatim from the decompile (owner->eclass @0x60,
//   owner->origin @0x68 confirmed in disasm).
extern brush_t *Brush_Clone( brush_t *def );                       // 0x475D20
extern void     Brush_RemoveEmptyFaces01( brush_t *b );            // 0x471720
extern void     Brush_Free_R( brush_t *def );                      // 0x475AF0
void Brush_SnapToGrid( brush_t *a1 )
{
    Brush_RemoveEmptyFaces01( a1 );
    if ( a1->faceCount < 4 )
    {
        Error( "Brush_SnapToGrid: Removing brush with no faces\n" );
        Brush_Free_R( a1 );
        return;
    }

    entity_s *owner = a1->owner;
    if ( owner && owner->eclass && owner->eclass->fixedsize )
    {
        // snap the owner's origin, move the brush by the resulting delta.
        float snapped[4];
        for ( int k = 0; k < 3; ++k )
            snapped[k + 1] = (float)floor( owner->origin[k] / grid_sizes[g_qeglobals.d_gridsize] + 0.5 )
                           * grid_sizes[g_qeglobals.d_gridsize];
        float move_delta[3];
        move_delta[0] = snapped[1] - owner->origin[0];
        move_delta[1] = snapped[2] - owner->origin[1];
        move_delta[2] = snapped[3] - owner->origin[2];
        Brush_Move( move_delta, a1, 1 );
        return;
    }

    // non-fixedsize: clone (to restore if the snap collapses the brush), snap each
    // face's 3 plane points to the grid, rebuild.
    brush_t *clone = Brush_Clone( a1 );
    for ( int fi = 0; fi < a1->faceCount; ++fi )
    {
        face_t *f = &a1->faces[fi];
        winding_t *w = f->w;
        for ( int p = 0; p < 3; ++p )
            for ( int k = 0; k < 3; ++k )
                f->planepts[p][k] = (float)floor( w->p[p][k] / grid_sizes[g_qeglobals.d_gridsize] + 0.5 )
                                  * grid_sizes[g_qeglobals.d_gridsize];
    }
    Brush_BuildWindings( a1, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++a1->version;

    // reject the snap if any axis of the resulting bbox is < 1 unit.
    int okAxes = 0;
    for ( int k = 0; k < 3; ++k )
    {
        if ( a1->maxs[k] - a1->mins[k] < 1.0f )
            break;
        ++okAxes;
    }
    if ( okAxes >= 3 )
    {
        Brush_Free_R( clone );
        return;
    }

    Sys_Printf( "WARNING: Snapped brush is smaller than 1 unit, restoring original brush\n" );
    face_t *tmp        = a1->faces;
    a1->faces          = clone->faces;
    clone->faces       = tmp;
    Brush_BuildWindings( a1, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++a1->version;
    Brush_Free_R( clone );
}

// ═════════════════════════════════════════════════════════════════════════════
//  RELOCATED HOME — functions whose embedded Assert() calls name
//  cod3src\Radiant\brush.cpp, i.e. brush.cpp is their original source file.
//  They had been ported into the TU of their principal CALLER (mayaexport /
//  entity / scriptgroup / surfacedlg / select); each is listed below in the order
//  of the brush.cpp line its asserts cite, and each kept its own asserts.
//  Functions elsewhere that carry a brush.cpp assert from an INLINED brush.cpp
//  helper stay where they are — the assert rode in with the inlined callee and
//  does not mean the enclosing function came from brush.cpp.
// ═════════════════════════════════════════════════════════════════════════════

// The maya object counters are IDB globals (dword_240A0EC / dword_240A0E8), defined
// in mayaexport.cpp; ExportTo3D below bumps them for the progress readout.
extern int s_mayaObjCount;
extern int s_mayaObjTotal;

// Callees of the relocated bodies that live in other TUs (previously declared in the
// files these functions were ported into).
extern void PMESH_56_extern( patchMesh_t *p );   // pmesh.cpp 0x44CFB0 (patch save)
extern void PMESH_57_extern( patchMesh_t *p );   // pmesh.cpp 0x44D0C0 (patch restore + rebuild)
extern void Patch_ApplyMatrix( const orientation_t *orient, patchMesh_t *p, char snap );  // pmesh.cpp 0x441E70

// ── brush.cpp:241 — relocated from mayaexport.cpp ──
// ════════════════════════════════════════════════════════════════════════════
//  0x46FDA0  ExportTo3D_CreatePolyFacet — emit ONE brush face winding as a
//  `polyCreateFacet -p x y z ...` (points REVERSED) + optional per-vertex polyEditUV.
//
//  __usercall in the binary: faceDef (the brush DEF face_t*, ECX) + face (the per-
//  instance faceVis_s*, first stack arg).  We pass both explicitly.  Returns 1 if a
//  facet was emitted, 0 if skipped (no winding, or filtered).
// ════════════════════════════════════════════════════════════════════════════
static char ExportTo3D_CreatePolyFacet( faceVis_s *face, face_t *def,
                                        const orientation_t *orient, FILE *f,
                                        char emitUVs, float scale )
{
    iassert( face );   // brush.cpp:241
    iassert( def );    // brush.cpp:242

    winding_t *w = def->w;            // [def+0xE0]
    if ( !w )
        return 0;

    // world-space face normal (scratch; the binary fills per-vertex normal arrays it
    // never emits — kept for fidelity, only the positions + UVs reach the .mel).
    float worldNormal[3];
    OrientationDirToWorldDir( worldNormal, orient, def->plane.normal );  // dir = [def+0xC0]

    // texdef + texture matrix (layer 0 — the export base layer; layerHandle=0 as in the binary).
    MaterialDef *mtldef = &def->mtldef[0];                     // [def+0x24]
    int texdef = TexWnd_06_LayerCount( (int)mtldef, 0 );

    // §11 (headless-vs-GUI material-realize invariant): the binary derefs `texdef`
    // UNCONDITIONALLY (Face_MoveTexture + the texCoord matrix), relying on the GUI's
    // material-realize pass having populated the layer (so texdef != 0).  A LAYERED
    // material whose `lyrMtl->layerCount == 0` (not realized — the PARKED texture/material
    // epic) makes TexWnd_06_LayerCount return 0; the binary would then deref a NULL texdef
    // (it never does in the GUI).  We GUARD it: with no realized layer, emit the GEOMETRY
    // (the whole point of the export — positions always exist) with identity UVs and skip
    // the colour probe (sub_46F6C0 → MaterialDef_14 would also assert on the empty layer).
    bool haveTex = ( texdef != 0 );
    float texMat[8];
    if ( haveTex )
    {
        texdef_sub_t *tdp = (texdef_sub_t *)(intptr_t)texdef;
        Face_MoveTexture( texdef, def->plane.normal, (int)texMat, texdef + 8,
                          tdp->rotate, tdp->crossterm );
    }
    else
    {
        texMat[0] = texMat[1] = texMat[2] = texMat[3] = 0.0f;
        texMat[4] = texMat[5] = texMat[6] = texMat[7] = 0.0f;
    }

    iassert( w->ptCount <= MAX_POINTS_ON_WINDING );   // brush.cpp:250

    if ( MtlDef_IsFaceFiltered( mtldef ) )
        return 0;

    fprintf( f, "\t\t$strPolyInfo = `polyCreateFacet -ch off -tx 1 -s 1" );

    if ( haveTex )                        // colour probe also needs a realized layer (MaterialDef_14)
    {
        unsigned int packedColor = 0;    // sub_46F6C0 result (unused by the -p emit, faithful)
        sub_46F6C0( (int)mtldef, (int)def, 0, (int *)&packedColor );
    }

    // texcoords are buffered then emitted in the SAME reverse order as the -p list.
    float texCoord[1024][2];

    // Emit points in REVERSE (numpoints-1 .. 0), matching the binary's facet winding.
    for ( int ptIndex = w->numpoints - 1; ptIndex >= 0; --ptIndex )
    {
        const float *p = w->p[ptIndex];        // x=p[0] y=p[1] z=p[2]

        texCoord[ptIndex][0] = texMat[0] * p[0] + texMat[1] * p[1] + texMat[2] * p[2] + texMat[3];
        texCoord[ptIndex][1] = texMat[4] * p[0] + texMat[5] * p[1] + texMat[6] * p[2] + texMat[7];

        iassert( !IS_NAN(texCoord[ptIndex][0]) );   // brush.cpp:265
        iassert( !IS_NAN(texCoord[ptIndex][1]) );   // brush.cpp:266

        fprintf( f, " -p %f %f %f", scale * p[0], p[1] * scale, scale * p[2] );
    }

    fprintf( f, "`;\r\n" );

    if ( emitUVs )
    {
        for ( int ptIndex = w->numpoints - 1; ptIndex >= 0; --ptIndex )
            fprintf( f, "\t\tpolyEditUV -r false -u %f -v %f ($strPolyInfo[0]).map[%d];\r\n",
                     texCoord[ptIndex][0], texCoord[ptIndex][1], ptIndex );
    }
    return 1;
}

// ── brush.cpp:2167 — relocated from entity.cpp ──
// ─────────────────────────────────────────────────────────────────────────────
// 0x475110  Entity_ColorSth  (brush.cpp:2167 assert — belongs here per IDB layout)
// Sets the GfxColor of all faces in a brush from the owning entity's _color or eclass color.
// Called after Entity_LinkBrush to paint the brush in the editor.
// IDA: esi = brush_t* (passed as int to avoid include confusion in callers).
// ─────────────────────────────────────────────────────────────────────────────
// 0x475110: _color/eclass-color/default branches, Byte4PackPixelColor(src,dest), face-color
// loop @+228 stride 232, version@0x4E 16-bit ++. 2167 converted (same-file after the move).
unsigned int Entity_ColorSth( brush_t *b )
{
    int a1 = (int)(intptr_t)b;

    iassert( b );   // brush.cpp:2167

    entity_s_def *entDef = *(entity_s_def **)((char *)b + 8); // b->owner

    // IDA passes &rgba[0] to Entity_GetVec3ForKey, which writes rgba[0..2].  ONE float[4],
    // never three loose locals: MSVC may reorder separate locals and the write runs off.
    float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    eclass_t *eclass = nullptr;
    if ( entDef && ( eclass = entDef->eclass ) != nullptr && *(int *)&eclass->fixedsize )
    {
        if ( HasKeyValuePair( entDef, "_color" ) )
        {
            Entity_GetVec3ForKey( entDef, rgba, "_color" );
        }
        else
        {
            rgba[0] = eclass->color[0];
            rgba[1] = eclass->color[1];
            rgba[2] = eclass->color[2];
            rgba[3] = eclass->unk;
        }
    }

    GfxColor v11;
    Byte4PackPixelColor( rgba, &v11 );

    unsigned int result = 0;
    int faceCount = b->faceCount;
    if ( faceCount )
    {
        int v6 = 0;
        do
        {
            // face_t at b->faces[result]; GfxColor at face+228
            *(GfxColor *)( (char *)b->faces + v6 + 228 ) = v11;
            ++result;
            v6 += 232;  // sizeof(face_t)
        }
        while ( result < (unsigned)faceCount );
    }
    ++b->version;
    return result;
}

// ── brush.cpp:3499 — relocated from mayaexport.cpp ──
// ════════════════════════════════════════════════════════════════════════════
//  0x477E00  ExportTo3D — emit ONE brush as a `{ ... }` MEL block: a polyCreateFacet
//  per (filtered-in) face, then polyUnite/polyMergeVertex/parent.  Point (fixedsize)
//  entities additionally get four small marker facets + move/rotate from origin/angles.
// ════════════════════════════════════════════════════════════════════════════
char ExportTo3D( selbrush_t *b, const orientation_t *orient, FILE *f,
                        char groupAsBrush, char emitUVs, float scale )
{
    iassert( b );   // brush.cpp:3499
    iassert( b->def );   // brush.cpp:3500

    char filtered = FilterBrush( b, 0 );
    if ( filtered )
        return filtered;                 // filtered-out brush -> emit nothing (returns nonzero)

    // sync the per-instance faceVis array to the def's face count (rebuild if stale).
    if ( b->faceCount != b->def->faceCount )
    {
        if ( b->faces )
            Vis_Free( b->faceCount, b->faces, (int)(intptr_t)b );
        b->faceCount = b->def->faceCount;
        b->faces     = (faceVis_s *)PlanePts_Alloc( b->faceCount );
    }

    fprintf( f, "\t{\r\n" );
    fprintf( f, "\t\tstring $strPolyInfo[];\r\n" );
    if ( groupAsBrush )
    {
        fprintf( f, "\t\tstring $strPolyList[];\r\n" );
        fprintf( f, "\t\tstring $strBrush[];\r\n\r\n" );
    }

    face_t    *defFaces = b->def->faces;        // [def+0x44]
    faceVis_s *visFaces = (faceVis_s *)b->faces;       // [inst+0x1C]
    int        polyIdx  = 0;
    for ( int i = 0; (unsigned)i < (unsigned)b->faceCount; ++i )
    {
        if ( ExportTo3D_CreatePolyFacet( &visFaces[i], &defFaces[i], orient, f, emitUVs, scale ) )
        {
            if ( groupAsBrush )
                fprintf( f, "\t\t$strPolyList[%d] = $strPolyInfo[0];\r\n", polyIdx );
            ++polyIdx;
        }
    }

    entity_s_def *ownerDef = (entity_s_def *)b->owner->def;   // owner entity def
    if ( *(int *)&ownerDef->eclass->fixedsize )       // point entity: emit 4 marker facets
    {
        const float *o = ownerDef->origin;
        // +X / +Z(8) / -Y(8) marker
        fprintf( f, "\t\t$strPolyInfo = `polyCreateFacet -ch off -tx 1 -s 1" );
        fprintf( f, " -p %f %f %f", o[0] + 32.0, o[1],       o[2] );
        fprintf( f, " -p %f %f %f", o[0],        o[1],       o[2] + 8.0 );
        fprintf( f, " -p %f %f %f", o[0],        o[1] - 8.0, o[2] );
        fprintf( f, "`;\r\n" );
        fprintf( f, "\t$strPolyList[size($strPolyList)] = $strPolyInfo[0];\r\n" );
        // +X / +Y(8) / +Z(8) marker
        fprintf( f, "\t\t$strPolyInfo = `polyCreateFacet -ch off -tx 1 -s 1" );
        fprintf( f, " -p %f %f %f", o[0] + 32.0, o[1],       o[2] );
        fprintf( f, " -p %f %f %f", o[0],        o[1] + 8.0, o[2] );
        fprintf( f, " -p %f %f %f", o[0],        o[1],       o[2] + 8.0 );
        fprintf( f, "`;\r\n" );
        fprintf( f, "\t$strPolyList[size($strPolyList)] = $strPolyInfo[0];\r\n" );
        // +X / -Z(8) / +Y(8) marker
        fprintf( f, "\t\t$strPolyInfo = `polyCreateFacet -ch off -tx 1 -s 1" );
        fprintf( f, " -p %f %f %f", o[0] + 32.0, o[1],       o[2] );
        fprintf( f, " -p %f %f %f", o[0],        o[1],       o[2] - 8.0 );
        fprintf( f, " -p %f %f %f", o[0],        o[1] + 8.0, o[2] );
        fprintf( f, "`;\r\n" );
        fprintf( f, "\t$strPolyList[size($strPolyList)] = $strPolyInfo[0];\r\n" );
        // +X / -Y(8) / -Z(8) marker
        fprintf( f, "\t\t$strPolyInfo = `polyCreateFacet -ch off -tx 1 -s 1" );
        fprintf( f, " -p %f %f %f", o[0] + 32.0, o[1],       o[2] );
        fprintf( f, " -p %f %f %f", o[0],        o[1] - 8.0, o[2] );
        fprintf( f, " -p %f %f %f", o[0],        o[1],       o[2] - 8.0 );
        fprintf( f, "`;\r\n" );
        fprintf( f, "\t$strPolyList[size($strPolyList)] = $strPolyInfo[0];\r\n" );
    }

    if ( groupAsBrush )
    {
        fprintf( f, "\t\tif (size ($strPolyList) > 1)\r\n" );
        fprintf( f, "\t\t\t$strBrush = `polyUnite -ch 0 ($strPolyList)`;\r\n" );
        fprintf( f, "\t\telse\r\n" );
        fprintf( f, "\t\t\t$strBrush[0] = $strPolyList[0];\r\n" );
        fprintf( f, "\t\tpolyMergeVertex -d 0.01 -ch 0 $strBrush[0];\r\n" );
        fprintf( f, "\t\tparent $strBrush[0] $strGroups[$iCurGroup];\r\n" );

        if ( *(int *)&ownerDef->eclass->fixedsize )
        {
            float ang[3];
            if ( !Entity_GetVec3ForKey( ownerDef, ang, "angles" ) )
            {
                ang[0] = 0.0f; ang[1] = 0.0f; ang[2] = 0.0f;
            }
            const float *o = ownerDef->origin;
            fprintf( f, "\t\tmove -x %f -y %f -z %f (($strBrush[0]) + \".scalePivot\") "
                        "(($strBrush[0]) + \".rotatePivot\");\r\n", o[0], o[1], o[2] );
            if ( HasKeyValuePair( ownerDef, "angles" ) )
                fprintf( f, "\t\trotate -os %f %f %f ($strBrush[0]);\r\n", ang[2], ang[0], ang[1] );
        }
    }
    else
    {
        fprintf( f, "\t\tparent $strPolyList $strGroups[$iCurGroup];\r\n" );
    }

    ++s_mayaObjCount;
    fprintf( f, "\t}\r\n" );
    fprintf( f, "\tprogressWindow -edit -s 1;\r\n" );
    return (char)fprintf( f, "\tprogressWindow -edit -st \"%d of %d objects\";\r\n",
                          s_mayaObjCount, s_mayaObjTotal );
}

// ── brush.cpp:4293 — relocated from scriptgroup.cpp ──
// 0x479FF0  ScriptGroup_AssignNextNumber — assign the next free group NUMBER (under
// g_PrefsDlg->ScriptGroupKey) to every selected entity.  Faithful to the disasm:
//   * guard: only run when ScriptGroupKey != ScriptColorTeamKey (the colour-team key
//     uses the colour-token path instead; a vehicle/group key is always != the team key)
//   * pass 1: walk active_brushes, max = -1; for each non-world entity carrying the key,
//     atol(value) and keep the max (skip empty values)
//   * pass 2: itoa(max+1) and SetKeyValue it onto every selected non-world entity
// (The asserts carry the binary's literal brush.cpp file/line — AssignNextNumber was
//  inlined from a brush.cpp context; type 0 → log+continue, type 1 → would-be-fatal but
//  ValueForKey2 returns the "" global, never null, so the *v guard handles it.)
void ScriptGroup_AssignNextNumber()
{
    const char *groupKey = (const char *)g_PrefsDlg->ScriptGroupKey;
    if ( !strcmp( groupKey, (const char *)g_PrefsDlg->ScriptColorTeamKey ) )
        return;   // ScriptGroupKey == ScriptColorTeamKey → the colour-token path owns it.

    // PASS 1 — find the highest existing number under groupKey across active brushes.
    int maxNum = -1;
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        entity_s_def *def = (entity_s_def *)owner->def;
        // Binary 0x47a080 derefs b->def->owner UNCONDITIONALLY (no `b->def &&` guard) — matches
        // the sibling iterators; the prior port's `b->def &&` was an invented guard, dropped.
        iassert( b->owner->def == b->def->owner );   // brush.cpp:4293 (0x47a080)

        if ( !HasKeyValuePair( def, groupKey ) )
            continue;
        const char *group = ValueForKey2( (int)(intptr_t)def, groupKey );   // the binary's local
        iassert( group );   // brush.cpp:4298
        if ( group && *group )
        {
            int n = atol( group );
            if ( maxNum < n )
                maxNum = n;
        }
    }

    // PASS 2 — assign maxNum+1 to every selected non-world entity.
    char next[16];
    _itoa( maxNum + 1, next, 10 );
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        entity_s_def *def = (entity_s_def *)owner->def;
        // Binary 0x47a149 derefs b->def->owner UNCONDITIONALLY; `b->def &&` was invented, dropped.
        iassert( b->owner->def == b->def->owner );   // brush.cpp:4313 (0x47a149)
        SetKeyValue( def, groupKey, next );
    }
}

// ── brush.cpp:5481 — relocated from surfacedlg.cpp ──
// SurfaceInspector::PostDoSurface02 (0x47CA50) — SAVE one brush def's faces: copy every
// face's CURRENT-layer mtldef into its mtldef[3] scratch, then PMESH_56 on the patch.
//   qmemcpy(&face->mtldef[3], &face->mtldef[current], 0x24)  → mtldef[3] ← mtldef[current]
void Surf_PostDoSurface02( brush_t *b )
{
    iassert( b );   // brush.cpp:5481
    for ( unsigned int i = 0; i < (unsigned int)b->faceCount; ++i )
    {
        face_t *f = &b->faces[i];
        iassert( f );   // brush.cpp:5464
        f->mtldef[3] = f->mtldef[g_qeglobals.current_edit_layer];   // mtldef[3] ← mtldef[current]
    }
    if ( b->patch )
        PMESH_56_extern( b->patch );
}

// ── brush.cpp:5495 — relocated from surfacedlg.cpp ──
// SurfaceDlg_PostDoSurface02_Patch (0x47CAF0) — RESTORE one brush def's faces: copy each
// face's mtldef[3] scratch BACK into its current-layer mtldef, ++def->version, then PMESH_57.
//   qmemcpy(&face->mtldef[current], &face->mtldef[3], 0x24)  → mtldef[current] ← mtldef[3]
void Surf_PostDoSurface02_Patch( brush_t *b )
{
    iassert( b );   // brush.cpp:5495
    for ( unsigned int i = 0; i < (unsigned int)b->faceCount; ++i )
    {
        face_t *f = &b->faces[i];
        iassert( f );   // brush.cpp:5471
        f->mtldef[g_qeglobals.current_edit_layer] = f->mtldef[3];   // mtldef[current] ← mtldef[3]
        ++b->version;                                             // ++*(WORD*)(b+0x4E)
    }
    if ( b->patch )
        PMESH_57_extern( b->patch );
}

// ── brush.cpp:5598 — relocated from select.cpp ──
// ═════════════════════════════════════════════════════════════════════════════
//  0x47CDE0  Select_ApplyMatrix — transform one brush by the orientation block.
//  `mat` is the orientation_t (origin + 3x3). bSnap = grid-snap on rebuild.
//  deg != 0 routes fixed-size (prefab/model) entities to Select_RotateFixedSize.
//  bSwap flips each face's planept winding order (planept[0]<->planept[2]) — used
//  by the mirror (Select_FlipAxis) so reflected faces keep outward-facing normals.
// ═════════════════════════════════════════════════════════════════════════════
// KISAK SUBSET of 0x47cde0: geometry faithful; the loop bound uses def->faceCount (the
// instance-vs-def adaptation below) and the texture-lock reproject (sub_470570/sub_4706F0)
// is omitted (layer-gated no-op).  ++def->version is the brush_t int16 @0x4E.  Both
// asserts (5598/5608) are now IN their own file.
extern void Select_RotateFixedSize( selbrush_t *sb, float (*mid_point)[3], const float *rot );  // select.cpp 0x47CC90

void Select_ApplyMatrix( float *mat, selbrush_t *b, int bSnap, float deg, char bSwap )
{
    if ( b->patch )
    {
        // Patch arm: transform the control points by the orientation block (rotate/flip).
        iassert( b->def->patch == b->patch->def );   // brush.cpp:5598
        Patch_ApplyMatrix( (const orientation_t *)mat, b->def->patch, (char)bSnap );
        return;
    }

    entity_s_def *ownerDef = (entity_s_def *)b->owner->def;
    iassert( b->owner->def == b->def->owner );   // brush.cpp:5608

    eclass_t *eclass = ownerDef->eclass;
    if ( *(int *)&eclass->fixedsize )
    {
        // Fixed-size entity (prefab / model bbox): rotate its origin + angles, no
        // planept transform. Only meaningful for a real rotation (deg != 0); a pure
        // mirror leaves the bbox where it is (matches the binary).
        if ( deg != 0.0f )
            Select_RotateFixedSize( b, (float (*)[3])mat, (const float *)eclass );
        return;
    }

    // instance-vs-def: the IDB loops b->faceCount (the INSTANCE's cached count, set by
    // Brush_BuildFaceVis on the camera draw, so 0 headless / before the first 3D draw).
    // Loop the DEF count (authoritative, identical when valid) so the transform also
    // applies headless.  Same adaptation as SetupVertexSelection.
    if ( b->def->faceCount )
    {
        for ( int fi = 0; fi < b->def->faceCount; ++fi )
        {
            face_t *f = &b->def->faces[fi];
            if ( bSwap )
            {
                // swap planepts[0] <-> planepts[2] (reverse winding for the mirror)
                float t0 = f->planepts[0][0], t1 = f->planepts[0][1], t2 = f->planepts[0][2];
                f->planepts[0][0] = f->planepts[2][0];
                f->planepts[0][1] = f->planepts[2][1];
                f->planepts[0][2] = f->planepts[2][2];
                f->planepts[2][0] = t0;
                f->planepts[2][1] = t1;
                f->planepts[2][2] = t2;
            }
            // sub_470570 (texture-basis stash) is layer-gated → no-op in this build.
            for ( int pi = 0; pi < 3; ++pi )
            {
                float rel[3];
                rel[0] = f->planepts[pi][0] - mat[0];   // VectorSubtract(pt - origin)
                rel[1] = f->planepts[pi][1] - mat[1];
                rel[2] = f->planepts[pi][2] - mat[2];
                OrientationPosToWorldPos( f->planepts[pi], rel,
                                          reinterpret_cast<const orientation_t *>( mat ) );
            }
            // sub_4706F0 (Face_MakePlane + texture reproject) — Face_MakePlane is
            // redundant with Brush_BuildWindings below; reproject is layer-gated → no-op.
        }
    }

    Brush_BuildWindings( b->def, bSnap );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++b->def->version;
}

