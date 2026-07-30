#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\select.cpp - selection state: the pick-ray chain, the brush/face
// selection lists, selection transforms, hide/show, and the vertex/edge edit handles.
// Ground truth: CoD4Radiant IDA (IW3xRadiant.i64); GtkRadiant 1.6 for naming only.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"   // CMainFrame / CXYWnd::m_nViewType (region-select view axis)
#include "prefs.h"     // CPrefsDlg / g_PrefsDlg (Test_Ray pick-chain m_nEntityShowState/...)
#include <gfx_d3d/r_xsurface.h>  // XModel, Editor_ExtractXModelGeo (sub_48CE60 model ray-pick)
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vector>     // Test_Ray cycle-pick (contents&0x40) brush gather (IDB CPtrArray)

// ─── CRT import thunk (IDA: j__free_0 = __imp__free) ─────────────────────────
static inline void j__free_0( void *p ) { free( p ); }

// ─── editor helpers from engine_stubs.cpp / qe3.cpp ──────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );

// ─── world_entity from map.cpp ────────────────────────────────────────────────
extern entity_s *world_entity;  // 0x25D5B30

// ─── forward declarations ──────────────────────────────────────────────────────
// brush.cpp
extern void        Brush_RemoveFromList( selbrush_t *b );
extern void        Brush_AddToList2( selbrush_t *b );
extern void        Brush_Free( selbrush_t *b );
extern void        Brush_BuildWindings( brush_t *b, int bFull );
// Made non-static in brush.cpp so select.cpp can call them.
extern void Brush_Deselect_Helper( selbrush_t *b );
extern void Brush_Select_Helper( selbrush_t *b );

// entity.cpp
extern bool        Entity_HasEpairMatch( entity_s *e, const char *key, const char *val );
extern bool        HasKeyValuePair( entity_s_def *e, const char *key );
extern char       *ValueForKey2( int defPtr, const char *key );
extern void        SetKeyValue( entity_s_def *e, const char *key, const char *val );
extern int         Entity_GetVec3ForKey( entity_s_def *def, float *out, const char *key );
extern void        Entity_RebuildBounds( entity_s *e );
extern char       *va( const char *fmt, ... );

// filters.cpp (0x46A1F0) — real implementation; was safe-stub in engine_stubs.cpp
extern char        FilterBrush( selbrush_t *b, int flag );

// qe3.cpp / engine_stubs.cpp
extern void        SetupVertexSelection();
extern void        MarkMapModified();
extern int         UpdateSelection( int wParam, eclass_t *cls );   // win_ent.cpp (0x497180)

// undo.cpp (all FATAL-stubbed in engine_stubs.cpp until undo.cpp is ported)
extern void        Undo_ClearRedo();
extern void        Undo_GeneralStart( const char *op );
extern void        Undo_AddBrushList( selbrush_t *list );
extern void        Undo_EndBrushList( selbrush_t *list );
extern void        Undo_AddBrush( entity_brush_s *b );
extern void        Undo_AddEntity( int entPtr );
extern void        Undo_AddEntity_W( entity_s *e );
extern void        Undo_End();

// engine_stubs.cpp
extern void        sub_43ECB0();   // addpoint mode cleanup (FATAL stub)

// patch (pmesh.cpp / engine_stubs.cpp — all FATAL-stubbed)
extern void        Patch_Deselect();
extern void        Patch_ShiftTexture( patchMesh_t *p, float s, float t );
extern curvePatchDef_t *PMESH_37( patchMesh_t *def, int axis );    // pmesh.cpp 0x445e30
extern void        Patch_ScaleTexture( patchMesh_t *p, float s, float t );
extern void        Patch_RotateTexture( patchMesh_t *p, float deg );
extern void        Patch_Scale( patchMesh_t *p, const float *mid, const float *scale, char doRebuild );
extern void        Patch_ApplyMatrix( const orientation_t *orient, patchMesh_t *p, char snap ); // pmesh.cpp 0x441E70
extern int         g_bPatchBendMode;                     // engine_stubs.cpp 0x25d5b04
extern void        sub_47C950( int brushDef, float y, float x );
extern void        Texture_Fit( int facePtr, float y, float x, int flag );
extern void        sub_477D70( selbrush_t *b, const float *mat );
extern void        Vis_Free( int count, faceVis_s *f, int brushPtr );   // brush.cpp 0x4702b0
extern void        sub_477080( brush_t *b, int sampleSize );
extern void        sub_476ED0( brush_t *def, MaterialDef *mtldef, char flag, float f4, char flag2 ); // 0x476ed0 (real port in brush.cpp)
extern void        sub_477020( brush_t *def, const texdef_sub_t *texdef );

// brush.cpp texture helpers
extern void        sub_47B940( brush_t *def );
extern void        sub_4767E0( const texdef_sub_t *texDef, int facePtr, int brushPtr );
extern void        sub_4768B0( face_t *f, brush_t *b, int sz );

// materialdef.cpp
extern void        TexMatToFakeTexCoords( MaterialDef *def, texdef_sub_t *texDef );
namespace LayerMat { int GetCurrentLayer( MaterialDef *def ); }

// engine_stubs.cpp / qe3.cpp stubs for selection bounds
void Select_GetBounds( float *mins, float *maxs );
void Select_GetMid( float *mid );


// engine_stubs.cpp / undo.cpp globals
extern float       world_orient_matrix[4][3];  // 0x6DE290
extern void        unknown_libname_291();       // CRT bounds-check abort
extern undo_s     *g_lastundo;                  // 0x23F162C — defined in undo.cpp
extern char        g_ptrSelectedFaces_GetSize[4]; // 4-byte int alias (IDB 0x73C714)
extern int         g_nUpdateBits;              // 0x25D5A74
extern void       *zero;                       // empty-string sentinel (engine_stubs.cpp)

// --- Test_Ray pick chain deps ---
// Brush_Ray (0x475fe0): convex-clip the instance against the ray; returns the entry
// faceVis_s* (&b->faces[entryIdx]) on a hit, NULL otherwise. *outDist = entry distance
// along dir, *outNormal = entry face normal. Built faithfully below (uses the real
// instance faceVis array sub_477D70 builds), NOT a separate struct.
extern faceVis_s  *Brush_Ray( selbrush_t *b, const float *dir, const float *start,
                              const orientation_t *orient, float *outDist, float *outNormal );
extern char        PMESH_51( const float *org, const float *dir, patch_t *pm,
                             float *outDist, int *outCol, int *outRow,
                             unsigned char *outColor, float *outPlane );        // pmesh.cpp 0x44acc0
extern char        PMESH_RaySegPick( const float *vB, const float *vA, const float *dir,
                                     const float *base, const float *org,
                                     float *outT, float *outU, float *outV );   // pmesh.cpp 0x44ab10
extern int         PlaneFromPoints_Real( float *out, const float *A, const float *B,
                                         const float *C );                      // pmesh.cpp 0x4a9950
extern void        Entity_GetOrientationMatrix( entity_s *ent, float (*axis)[3] ); // entity.cpp 0x482940
extern void        VectorRotateByAxis( float *out, const float *axisMatrix, const float *dir ); // draw.cpp 0x4ba6b0
// The pick chain transforms the ray WORLD->LOCAL (0x4ba610), the INVERSE of
// OrientationPosToWorldPos (0x4ba430) - two distinct binary functions.
extern void        OrientationWorldPosToLocalPos( float *out, const float *pos,
                                                  const orientation_t *orient ); // draw.cpp 0x4ba610 (sub_4BA610)
extern void        OrientationConcatenate( const orientation_t *orFirst,
                                           const orientation_t *orSecond,
                                           orientation_t *out );                // engine_stubs.cpp 0x4ba7d0
extern void        OrientationDirToWorldDir( float *out, const orientation_t *orient,
                                             const float *dir );                // engine_stubs.cpp 0x4ba4b0
extern float      *AnglesToAxis( float *angles, float (*axis)[3] );             // engine_stubs.cpp 0x4abeb0
extern bool        Model_SetModel( entity_brush_s *b, int orientMatrix );       // brush.cpp 0x478780
extern LayerMaterialDef *Materialdef_GetName( MaterialDef *mtlDef );            // materialdef.cpp 0x431640
extern char        sub_46FCF0( Material *faceMtl );                             // filters.cpp 0x46fcf0
extern void        Select_Deselect( int updateScene );                         // select.cpp 0x48e800
// g_PrefsDlg (prefData_t*) comes from prefs.h; Editor_ExtractXModelGeo from r_xsurface.h.

// prefab_s (brush.cpp): the recursed prefab brush list passed to sub_48D460 is the
// EMBEDDED sentinel at prefab+0x0C — the {active_brushlist(prev)@0x0C,
// active_brushlist_next(next)@0x10} pair is a selbrush_t list head, exactly like the
// &active_brushes / &selected_brushes sentinels. The binary takes &prefab->active_brushlist.
static inline selbrush_t *Prefab_BrushListSentinel( void *prefab )
{
    return (selbrush_t *)( (char *)prefab + 0x0C );
}

// W_CAMERA bitmask (g_nUpdateBits flag for camera redraw)
#ifndef W_CAMERA
#define W_CAMERA 0x01
#endif

// undo_s is defined in qe3.h (included above via stdafx.h).

// patch_t (the patch instance node; .def @0, .selected @6) is the shared struct in qe3.h;
// selbrush_t.patch is a patch_t*.

// selface_t is defined in qe3.h (moved so drag.cpp can use it).

// ── helper macros ────────────────────────────────────────────────────────────

// Cast selbrush_t.faces (void*) to faceVis_s[] and index it.
#define SEL_FACES(b)      ((faceVis_s *)(b)->faces)

// String zero — engine_stubs.cpp has `void *zero = nullptr`. In the original
// binary this is an MFC CString "zero" value used as an empty string sentinel.
// For plain-C usage we cast to (const char*) and guard against null.
static inline const char *zero_str() {
    return zero ? (const char *)zero : "";
}

// ── select.cpp globals ───────────────────────────────────────────────────────
// selFace (0x73C710) — heap array of selface_t
selface_t *selFace = nullptr;
// g_selFaceSize (0x73C718) — allocated capacity of selFace[]
int        g_selFaceSize = 0;

// Helper: access the count of valid selFace entries.
// All IDA read/write sites use *(_DWORD*)g_ptrSelectedFaces_GetSize.
#define SEL_FACE_COUNT() (*(int *)g_ptrSelectedFaces_GetSize)

// ── undo helpers ─────────────────────────────────────────────────────────────
// The "adding brushes after entity" guard tests entitylist.next (undo+0x70, IDA 0x48f210),
// NOT the embedded brush head (+0x7C) - Undo only links entities via entity_s.next.
static void Undo_TryAddBrush( brush_t *bDef )
{
    if ( g_lastundo )
    {
        if ( g_lastundo->entitylist.next != &g_lastundo->entitylist )   // IDA 0x48f210: entity_s.next@undo+0x70 vs &entitylist@+0x6C (NOT the embedded brushes head @+0x7C/+0x78)
            Sys_Printf( "Undo_AddBrush: WARNING adding brushes after entity.\n" );
        entity_s *owner = (entity_s *)(intptr_t)bDef->owner;
        if ( *(int *)&owner->eclass->fixedsize )
            Undo_AddEntity( (int)(intptr_t)owner );
        Undo_AddBrush( (entity_brush_s *)bDef );
    }
    else
    {
        Sys_Printf( "Undo_AddBrush: no last undo.\n" );
    }
}

// UNDO_LINK_BRUSH — record undo id in brush_t.ownerPrev and entity.epairEdits.
// IDA: `v10->ownerPrev = (entity_s *)g_lastundo->id;`
// We match that logic exactly.
static void Undo_LinkBrush( brush_t *bDef )
{
    if ( g_lastundo && !g_lastundo->done )
    {
        bDef->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
        entity_s *owner = (entity_s *)(intptr_t)bDef->owner;
        if ( *(int *)&owner->eclass->fixedsize )
            owner->epairEdits = g_lastundo->id;
    }
}

// ── mode reset helper ─────────────────────────────────────────────────────────
// CMainFrame::UpdatePatchToolbarButtons (0x42aa70) - refresh the patch-edit toolbar button
// check-states AND the three Curve-menu check marks.  Disasm 0x42aa8c-0x42ab82: five
// TB_CHECKBUTTON (0x402) sends on m_wndToolBar - 0x8072 patch-bend, 0x8068 redisperse,
// 0x8174 lock, 0x8173 unlock, 0x8175 cycle-edge - then CheckMenuItem(0x8174/0x8173/0x8175).
// TB_CHECKBUTTON for ids absent from the port's IDR_TOOLBAR152 is a harmless FALSE.
void CMainFrame_UpdatePatchToolbarButtons()
{
    extern CMainFrame *g_pParentWnd;                                   // 0x25D5A70
    extern int g_bPatchBendMode;                                       // engine_stubs
    extern int g_qeglobals_redispersePatchVerts;                       // engine_stubs
    if ( !g_pParentWnd )
        return;
    HWND tb = g_pParentWnd->m_wndToolBar.GetSafeHwnd();
    if ( tb )
    {
        ::SendMessageA( tb, TB_CHECKBUTTON, 0x8072, g_bPatchBendMode != 0 );                 // 0x42aa97
        ::SendMessageA( tb, TB_CHECKBUTTON, 0x8068, g_qeglobals_redispersePatchVerts != 0 ); // 0x42aab6
        ::SendMessageA( tb, TB_CHECKBUTTON, 0x8174, g_qeglobals.bLockPatchVerts != 0 );      // 0x42aad5
        ::SendMessageA( tb, TB_CHECKBUTTON, 0x8173, g_qeglobals.bUnlockPatchVerts != 0 );    // 0x42aaf4
        ::SendMessageA( tb, TB_CHECKBUTTON, 0x8175,
                        g_qeglobals.d_select_mode == sel_cycle_edge_direction_quad );        // 0x42ab14
    }
    HWND mainHwnd = g_pParentWnd->GetSafeHwnd();                       // binary: g_qeglobals.d_hwndMain
    HMENU menu = mainHwnd ? ::GetMenu( mainHwnd ) : nullptr;
    if ( menu )
    {
        ::CheckMenuItem( menu, 0x8174, g_qeglobals.bLockPatchVerts   ? MF_CHECKED : MF_UNCHECKED );  // 0x42ab3f
        ::CheckMenuItem( menu, 0x8173, g_qeglobals.bUnlockPatchVerts ? MF_CHECKED : MF_UNCHECKED );  // 0x42ab5d
        ::CheckMenuItem( menu, 0x8175,
                         ( g_qeglobals.d_select_mode == sel_cycle_edge_direction_quad )
                             ? MF_CHECKED : MF_UNCHECKED );                                          // 0x42ab82
    }
}

static void ResetSelectMode()
{
    select_t prev = g_qeglobals.d_select_mode;
    g_qeglobals.d_select_mode = sel_brush;
    if ( prev == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prev == sel_addpoint )
        sub_43ECB0();
}

// ── Select_Brush_2 — links brush into list head ───────────────────────────────
// 0x476630
static void Select_Brush_2( selbrush_t *list, selbrush_t *b )
{
    if ( b->next || b->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    if ( list == &selected_brushes )
    {
        Brush_AddToList2( b );
    }
    else
    {
        b->next = list->next;
        list->next->prev = b;
        list->next = b;
        b->prev = list;
    }
}

// ── sub_48BE20 — parse SPACE-delimited int list (Map_ParseLinkList) ───────────────────────────────
// 0x48be20 Map_ParseLinkList. __usercall(buf@ebx, str).  Parses str (script_linkTo /
// script_linkName) as a SPACE-delimited int list with DEDUP, capped at 30 entries.
// Layout: buf[0..count-1]=values, buf[count]=-1 terminator, buf[1024]=count (byte 0x1000),
// byte buf+0x1004 = overflow flag (cleared each call, set when the 30-cap is hit).  Parses
// in place; cross-file Assert qe3.cpp:446.  Caller buf must be int[1026]+ so buf+0x1004 is
// in bounds.
void Map_ParseLinkList( int *buf, const char *str )
{
    int  count = 0;
    bool atBoundary = true;                          // v7: at a token start
    *( (unsigned __int8 *)buf + 0x1004 ) = 0;        // overflow flag cleared (IDA 0x48be31)
    size_t i   = 0;
    size_t len = strlen( str );
    if ( len )
    {
        for ( ;; )
        {
            const char *p = &str[i];
            if ( !str[i] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\qe3.cpp", 446, 0, "%s", "linkTo[i]" );
            if ( *p == ' ' )
            {
                atBoundary = true;
            }
            else if ( atBoundary )
            {
                int  value = atol( p );
                bool dup   = false;                  // dedup scan buf[0..count-1] (IDA 0x48be9a)
                for ( int k = 0; k < count; ++k )
                    if ( buf[k] == value ) { dup = true; break; }
                if ( !dup )
                {
                    buf[count++] = value;
                    atBoundary   = false;
                    if ( count >= 30 )               // overflow (IDA cmp esi,1Eh)
                    {
                        *( (unsigned __int8 *)buf + 0x1004 ) = 1;
                        break;
                    }
                }
            }
            ++i;
            len = strlen( str );                      // IDA recomputes strlen each pass
            if ( i >= len )
                break;
        }
    }
    buf[count] = -1;                                  // -1 terminator at buf[count]
    buf[1024]  = count;                               // count at buf[0x1000]
}

// ═════════════════════════════════════════════════════════════════════════════
//  THE Test_Ray PICK CHAIN (brush + patch + prefab + model ray-pick):
//    Brush_Ray   (0x475fe0)  convex-brush plane clip -> entry faceVis_s*
//    sub_48CE60  (0x48ce60)  XModel per-triangle ray-pick
//    sub_48D240  (0x48d240)  per-brush inner trace: brush -> patch -> prefab -> model
//    sub_48D460  (0x48d460)  brush-list walker (keeps nearest, dedup by ~0.5 dist)
//    Test_Ray    (0x48d7c0)  top entry (incl. the contents&0x40 cycle-pick branch)
//  edTrace_t (qe3.h) IS the byte-exact IDB trace_t (88 bytes: brush@0, face@4, xx1@8,
//  xx2@0xC, xx3@0x10, xx4@0x14 = a 0x30-byte orientation copy, dist@0x44, selected@0x48,
//  _pad[0]@0x49, normal@0x4C).  The faceVis identity t->face == &brush->faces[idx] holds
//  because Brush_Ray returns into the real instance faceVis array (built by sub_477D70).
//  KISAK: sub_48D460's camera/XY frustum cull (sub_405620 + CXYWnd_SetupClipPlanes +
//  CullCubic + sub_46CD80) is skipped - it needs m_pCamWnd/m_pActiveXY, and it is a pure
//  optimization (a brush is processed unless culled by BOTH views), so never culling is safe.
// ═════════════════════════════════════════════════════════════════════════════
extern selbrush_t active_brushes;          // map.cpp (0x23F189C)

// Forward declarations — the chain is mutually recursive (sub_48D240 → sub_48D460 →
// sub_48D240 for prefab recursion; Test_Ray → sub_48D460; sub_48D460 → sub_48CE60).
static char sub_48CE60( float *outDist, float *outNormal, const float *org,
                        const float *dir, selbrush_t *brush );
static char sub_48D240( const float *start, const float *dir, int contents,
                        selbrush_t *a4, const orientation_t *a5, edTrace_t *a6 );
static void sub_48D460( const float *start, const float *dir, int contents,
                        selbrush_t *sb, const orientation_t *a5, edTrace_t *a6, int a7 );

// ── Brush_Ray (0x475fe0) — ray-vs-convex-brush, returns the entry faceVis_s* ──────
//  Builds the instance faceVis (Brush_CheckBuildFaceVis), then clips the ray segment
//  [start, start+262144*dir] against the b->faceCount planes (b->def->faces[i].plane).
//  On entry returns &b->faces[entryFaceIdx] and *outDist = the entry distance along dir;
//  on miss returns NULL with *outDist = 0.  The patch/model/prefab fallback (no instance
//  faces or no entry face) returns &b->faces[0] (non-NULL, dist 0) with normal (0,0,1) - a
//  pretend-hit so the caller's finer test (PMESH_51 / prefab recursion / model triangle
//  pick) decides; plain brushes miss.
faceVis_s *Brush_Ray( selbrush_t *b, const float *dir, const float *start,
                      const orientation_t *orient, float *outDist, float *outNormal )
{
    // far endpoint = start + 262144*dir
    float endX = dir[0] * 262144.0f + start[0];
    float endY = dir[1] * 262144.0f + start[1];
    float endZ = dir[2] * 262144.0f + start[2];

    sub_477D70( b, (const float *)orient );                // 0x47602a — Brush_CheckBuildFaceVis

    double entryDist = 0.0;       // v7
    int    hitFace   = 0;         // v9 = entryFaceIdx*12 + (int)b->faces (the returned faceVis_s*)
    int    i         = 0;         // v8 — loop counter against b->faceCount (a1->unk1)

    if ( b->faceCount )           // 0x47603b — a1->unk1
    {
        // near point (p1) clipped forward, far point (p2) clipped back, as planes pass.
        float p1x = start[0], p1y = start[1], p1z = start[2];   // v14/v12/v13
        float p2x = endX,     p2y = endY,     p2z = endZ;       // v30/v11/v10
        int   faceByteOff = 0;    // v44 (12*i)
        for ( ;; )
        {
            const plane_t &pl = b->def->faces[i].plane;         // &a1->def->brush_faces[v45].plane
            float d1 = pl.normal[1]*p1y + pl.normal[0]*p1x + pl.normal[2]*p1z - pl.dist;  // v50
            float d2 = pl.normal[1]*p2y + p2x*pl.normal[0] + pl.normal[2]*p2z - pl.dist;  // v48
            if ( d1 >= 0.0f && d2 >= 0.0f )                     // both in front → outside (convex)
            {
                *outDist = 0.0f;
                return nullptr;
            }
            if ( d1 <= 0.0f && d2 <= 0.0f )                     // both behind → plane doesn't clip
            {
                // entryDist unchanged (binary: v23=0.0, carried to next iter)
            }
            else
            {
                float frac = d1 / ( d1 - d2 );                  // v49
                if ( d1 <= 0.0f )                               // exit: clip the far endpoint
                {
                    p2x = ( p2x - p1x ) * frac + p1x;
                    p2y = ( p2y - p1y ) * frac + p1y;
                    p2z = frac * ( p2z - p1z ) + p1z;
                    // v23 = 0.0 (entryDist unchanged)
                }
                else                                            // entry: record face/normal, clip near
                {
                    if ( outNormal )
                    {
                        outNormal[0] = pl.normal[0];
                        outNormal[1] = pl.normal[1];
                        outNormal[2] = pl.normal[2];
                    }
                    hitFace = faceByteOff + (int)(intptr_t)b->faces;   // v9 = v44 + a1->refCount(=faces)
                    p1x = p1x + ( p2x - p1x ) * frac;           // v36 -> v14
                    p1y = p1y + ( p2y - p1y ) * frac;           // v39 -> v12
                    p1z = frac * ( p2z - p1z ) + p1z;           // v42 -> v19
                    // v21 = 0.0 (entryDist source for the both-behind/exit cases)
                }
            }
            ++i;                                                // ++v45 / ++v8 (advance together)
            faceByteOff += 12;                                  // v44 += 12
            if ( (unsigned)i >= (unsigned)b->faceCount )        // 0x4761d4 — vs a1->unk1
                break;
        }
        entryDist = 0.0;   // v7 = v21 (the both-behind/exit entryDist carry — always 0 in this path)
        if ( hitFace )     // 0x4761e0 — an entry face was recorded
        {
            // *a5 = a2[1]*v40 + *a2*v37 + a2[2]*v43  (entry point - start, dotted with dir)
            *outDist = dir[1] * ( p1y - start[1] )
                     + dir[0] * ( p1x - start[0] )
                     + dir[2] * ( p1z - start[2] );
            return (faceVis_s *)(intptr_t)hitFace;
        }
        // hitFace == 0: fall through to the no-entry-face fallback (LABEL_17).
    }

    // LABEL_17 (0x4761e8) — no instance faces, or no entry face recorded.
    *outDist = (float)entryDist;
    // 0x4761ed `cmp dword ptr [esi+20h],0`: esi is the brush INSTANCE and selbrush_t has
    // `patch` at +0x20 (the IDB's brush_t_with_custom_def mislabels +0x20 as mins, which
    // hex-rays renders as a1->mins[0]).  PATCH instances - like prefabs / shown models
    // below - get the pretend-hit so the caller's finer test (PMESH_51) decides.
    if ( !b->patch )
    {
        entity_s *owner = b->owner;
        // owner==NULL only via Ed_BrushFloorRay's transient headless instance (the
        // binary always passes real owned instances) — treat as a plain-brush miss.
        if ( !owner
          || ( !owner->prefab
            && ( !owner->modelInst || ( g_PrefsDlg->m_nEntityShowState & 0x10000 ) == 0 ) ) )  // ENTITY_SKINNED
            return nullptr;          // 0x476216
    }
    if ( outNormal )
    {
        outNormal[0] = (float)entryDist;
        outNormal[1] = (float)entryDist;
        outNormal[2] = 1.0f;
    }
    return b->faces;                 // return a1->refCount (== &b->faces[0])
}

// Ed_BrushFloorRay - KISAK: headless distance wrapper over Brush_Ray for the
// Cam_ChangeFloor / z.cpp floor-march callers (which hold only a def).
bool Ed_BrushFloorRay( brush_t *def, const float *start, const float *dir, float *outDist )
{
    // Transient instance header so Brush_Ray (instance-based) can clip the def faces.
    // owner/patch stay NULL so the no-entry fallback returns NULL, not a bogus hit.
    selbrush_t tmp;
    memset( &tmp, 0, sizeof( tmp ) );
    tmp.def       = def;
    tmp.version   = (short)( def->version ^ 1 );   // force a faceVis (re)build
    float normal[3];
    faceVis_s *f = Brush_Ray( &tmp, dir, start, (const orientation_t *)world_orient_matrix,
                              outDist, normal );
    // sub_477D70 may have built real faceVis (incl. surf-cache VB handles), so tear it
    // down with Vis_Free rather than free() or the floor-march leaks a VB per face.
    if ( tmp.faces )
        Vis_Free( tmp.faceCount, tmp.faces, (int)(intptr_t)&tmp );
    return f != nullptr;
}

// ── sub_48CE60 (0x48ce60) — XModel PMESH per-triangle ray-pick ────────────────────
//  Extract the brush's model geometry (Editor_ExtractXModelGeo), transform the ray into
//  the model's local frame (inverse rotation by "angles", scaled by 1/modelscale), then
//  ray-test every extracted triangle (PMESH_RaySegPick).  Keeps the nearest hit; returns
//  1 on a hit, 0 on miss (and 1 when the model has no geometry).
//  Args (the binary's __fastcall): outDist, outNormal, org(start), dir, brush.
static char sub_48CE60( float *outDist, float *outNormal, const float *org,
                        const float *dir, selbrush_t *brush )
{
    static unsigned char vbuf[196612];   // v16  — extracted vertex bytes (4000h verts * 12)
    static unsigned char ibuf[131072];   // v37  — extracted tri indices (10000h)

    entity_s *owner = brush->owner;                     // [esi+8]
    entity_s *ownerDef = (entity_s *)owner->def;        // [eax+8]
    iassert( ownerDef->modelClass );                    // select.cpp:64 brush->owner->def->modelClass
    entitymodel_t *mc = (entitymodel_t *)ownerDef->modelClass;
    iassert( mc->model );                               // select.cpp:65 ->model
    iassert( mc->model->handle );                       // select.cpp:66 ->model->handle

    int triCount = Editor_ExtractXModelGeo( (XModel *)(intptr_t)mc->model->handle,
                                            (float *)vbuf, 0x4000,
                                            (uint16_t *)ibuf, 0x10000 );   // v29
    if ( !triCount )
        return 1;                                       // 0x48cf57

    float angles[3];                                    // v21
    if ( !Entity_GetVec3ForKey( (entity_s_def *)ownerDef, angles, "angles" ) )
    {
        angles[0] = 0.0f; angles[1] = 0.0f; angles[2] = 0.0f;
    }
    float axis[3][3];                                   // v18
    AnglesToAxis( angles, axis );

    // localOrg = axis · (org - entityOrigin) — WORLD→LOCAL (sub_4BA610, 0x4ba610), the
    // INVERSE of OrientationPosToWorldPos. v17 = entity origin (def+0x68 = float[26..28]).
    float localOrg[3];                                  // v26..v28
    orientation_t modelOr;                              // {origin, axis}
    modelOr.origin[0] = ownerDef->origin[0];           // v7[26]
    modelOr.origin[1] = ownerDef->origin[1];
    modelOr.origin[2] = ownerDef->origin[2];
    modelOr.axis[0][0] = axis[0][0]; modelOr.axis[0][1] = axis[0][1]; modelOr.axis[0][2] = axis[0][2];
    modelOr.axis[1][0] = axis[1][0]; modelOr.axis[1][1] = axis[1][1]; modelOr.axis[1][2] = axis[1][2];
    modelOr.axis[2][0] = axis[2][0]; modelOr.axis[2][1] = axis[2][1]; modelOr.axis[2][2] = axis[2][2];
    OrientationWorldPosToLocalPos( localOrg, org, &modelOr );   // sub_4BA610(a3, &v26, v17)

    // modelscale epair (default 1.0); when > 0, divide the local origin by it.
    float scale = 1.0f;                                 // v31
    // zero_str(), NOT (const char*)zero: the port stubs the binary's empty-string global
    // `zero` (0x6D58F0) as a NULL void*, and atof(NULL) crashes.
    const char *mscale = zero_str();                    // v10
    for ( epair_t *ep = (epair_t *)ownerDef->epairs; ep; ep = (epair_t *)ep->next )   // [def+116]
    {
        if ( !_stricmp( ep->key, "modelscale" ) )
        {
            mscale = ep->value ? ep->value : zero_str();   // v10 = v9[2]; if !v10 -> LABEL_17
            break;
        }
    }
    float invScale = 1.0f;                              // used to scale the result distance back
    scale = (float)atof( mscale );                      // v31
    if ( scale <= 0.0f )
    {
        scale = 1.0f;                                   // LABEL_17
    }
    else
    {
        invScale = 1.0f / scale;                        // v34
        localOrg[0] *= invScale;                        // v26 *= v34
        localOrg[1] *= invScale;                        // v27 *= v34
        localOrg[2] *= invScale;                        // v28 *= v34
    }

    // localDir = dir rotated into model space.  VectorRotateByAxis reads the rotation from
    // axisMatrix[3..11], i.e. it expects the ORIENTATION block (origin[0..2] THEN
    // axis[3..11]) - pass &modelOr, NOT the bare `axis` array (offset by 3 floats).
    // [0x48d095 &var_20098]
    float localDir[3];                                  // v19
    VectorRotateByAxis( localDir, (const float *)&modelOr, dir );   // VectorRotateByAxis(v19, &modelOr(origin+axis), v33(dir))

    float best = 3.4028235e38f;                         // v33
    float planeV[4];                                    // v20
    int   idx = 0;                                      // v11
    if ( triCount > 0 )
    {
        const uint16_t *i16 = (const uint16_t *)ibuf;
        do
        {
            int a = i16[idx];                           // &v35[2*v11] — first index stream
            int c = i16[idx + 1];                       // &v36[2*v11]
            int bIdx = i16[idx + 2];                    // &v37[2*v11] (interleaved triplets)
            float t;                                    // v32 (outU/outV passed NULL — binary 0,0)
            const float *vA = (const float *)&vbuf[12 * a];
            const float *vB = (const float *)&vbuf[12 * c];
            const float *vC = (const float *)&vbuf[12 * bIdx];
            if ( PMESH_RaySegPick( vA, vB, localDir, vC, localOrg, &t, nullptr, nullptr ) )
            {
                t *= scale;                             // v32 *= v31
                if ( t < best )
                {
                    if ( outNormal )
                    {
                        // PlaneFromPoints_Real(v20, &v16[12*v24], &v16[12*v25], &v16[12*v22])
                        // == (out, vC, vB, vA) in the index order above.
                        PlaneFromPoints_Real( planeV, vC, vB, vA );
                        outNormal[0] = planeV[0];
                        outNormal[1] = planeV[1];
                        outNormal[2] = planeV[2];
                    }
                    best = t;
                }
            }
            idx += 3;                                   // LODWORD(v11) += 3
        } while ( idx < triCount );
    }

    if ( best == 3.4028235e38f )                        // 0x48d204 — no hit
        return 0;
    *outDist = best;                                    // *v23 = v33
    return 1;
}

// ── sub_48D240 (0x48d240) — per-brush inner trace ─────────────────────────────────
//  Ray-test ONE brush instance: convex brush (Brush_Ray) -> patch (PMESH_51) -> prefab
//  (recurse via sub_48D460 through the prefab's local orientation) -> model (sub_48CE60).
//  Fills a6 (a trace_t) with the hit; clears a6->brush/face on miss.
static char sub_48D240( const float *start, const float *dir, int contents,
                        selbrush_t *a4, const orientation_t *a5, edTrace_t *a6 )
{
    float *normal = a6->normal;                         // a6->normal
    float *p_dist = &a6->dist;                          // &a6->dist

    faceVis_s *face = Brush_Ray( a4, dir, start, a5, &a6->dist, a6->normal );   // 0x48d26b
    a6->face = face;                                    // a6->face = patch
    if ( !face )
    {
        a6->brush = nullptr;
        return 0;
    }
    a6->brush = a4;                                     // a6->brush = a4
    a6->xx1   = (int)(intptr_t)a4;                      // a6->xx1 = a4
    a6->xx2   = (int)(intptr_t)face;                    // a6->xx2 = patch
    memcpy( &a6->xx4, a5, 0x30u );                      // qmemcpy(&a6->xx4, a5, 0x30)
    a6->xx3   = 0;
    a6->_pad[0] = 0;                                    // *(&a6->selected + 1) = 0
    a6->selected = 0;

    patch_t *patch = a4->patch;                         // a4->patch
    if ( patch )
    {
        if ( ( contents & 0x100 ) != 0 )               // patch-pick disabled this pass
            return 1;
        char hit = PMESH_51( start, dir, patch, p_dist, nullptr, nullptr, nullptr, normal );   // 0x48d2d5
        if ( !hit )                                     // LABEL_16
        {
            a6->brush = nullptr;
            a6->face  = nullptr;
        }
        return hit;
    }

    entity_s *owner = a4->owner;                        // patch = a4->owner
    if ( owner->prefab )                                // owner->prefab
    {
        orientation_t orFirst;                          // [ebp-B0h]
        orientation_t out;                              // [ebp-E0h]
        float localDir[3];                              // v19
        float localStart[3];                            // a1a (int[3] in IDA = float[3])
        Entity_GetOrientationMatrix( (entity_s *)owner->def, &orFirst.origin );   // fills {origin, axis}
        VectorRotateByAxis( localDir, (const float *)&orFirst, dir );   // VectorRotateByAxis(v19, &orFirst, a2)
        OrientationWorldPosToLocalPos( localStart, start, &orFirst );   // sub_4BA610(a1, a1a, &orFirst)
        OrientationConcatenate( &orFirst, a5, &out );                   // OrientationConcatenate(&orFirst, a5, &out)

        edTrace_t a6a;                                  // [ebp-80h]
        a6a.dist = 262144.0f;                           // a6a.dist = 262144.0
        sub_48D460( localStart, localDir, contents,
                    Prefab_BrushListSentinel( owner->prefab ),
                    &out, &a6a, 1 );                    // 0x48d366
        char result;
        if ( a6a.dist == 262144.0f )                    // nothing hit in the prefab
        {
            a6->face  = nullptr;
            a6->brush = nullptr;
            result = (char)(intptr_t)a6;                // LOBYTE(patch) = (_BYTE)a6
        }
        else
        {
            a6->xx1 = a6a.xx1;
            a6->xx2 = a6a.xx2;
            a6->selected = a6a.selected;
            memcpy( &a6->xx4, &a6a.xx4, 0x30u );        // qmemcpy(p_xx4, &a6a.xx4, 0x30)
            *p_dist = a6a.dist;
            a6->_pad[0] = a6a._pad[0];                  // *(&a6->selected+1) = *(&a6a.selected+1)
            OrientationDirToWorldDir( normal, &out, a6a.normal );        // map the prefab normal back out
            a6->xx3 = a6a.xx3;
            result = (char)a6a.xx3;
            if ( a6a.xx3 == 0 )                         // prefab hit had no sub-prefab owner
            {
                a6->xx3 = (int)(intptr_t)a4->owner;     // a6->xx3 = a4->owner
                result = (char)(intptr_t)a4;
            }
        }
        return result;
    }
    else if ( owner->modelInst )                        // patch->modelInst
    {
        // model pick: only when "show models" (0x10000) is on and (contents&4 unset OR
        // the eclass is not an actor — actors are excluded from the &4 world pass).
        if ( ( g_PrefsDlg->m_nEntityShowState & 0x10000 ) != 0
          && ( ( contents & 4 ) == 0
            || _strnicmp( ((entity_s *)owner->def)->eclass->name, "actor", 5 ) != 0 ) )
        {
            char hit = sub_48CE60( p_dist, normal, start, dir, a4 );     // 0x48d43e
            if ( !hit )                                  // LABEL_16
            {
                a6->brush = nullptr;
                a6->face  = nullptr;
            }
            return hit;
        }
    }
    return (char)(intptr_t)face;                        // return (char)patch
}

// ── sub_48D460 (0x48d460) — brush-list walker ─────────────────────────────────────
//  Iterate a brush list (sb is the sentinel head), ray-test each pickable brush via
//  sub_48D240, and keep the nearest hit into the a6/a7 trace array (a7 = maxTraces).
//  The camera/XY frustum cull is skipped headless (see the chain header).
static void sub_48D460( const float *start, const float *dir, int contents,
                        selbrush_t *sb, const orientation_t *a5, edTrace_t *a6, int a7 )
{
    for ( selbrush_t *sbn = sb->next; sbn != sb; sbn = sbn->next )      // 0x48d46d
    {
        // contents&4 (world pass): skip prefab/model entity brushes (owner != world &
        // owner is a prefab class) — those are picked via the prefab/model recursion.
        if ( ( contents & 4 ) != 0 )
        {
            entity_s *owner = sbn->owner;
            if ( owner != world_entity
              && ( ((entity_s *)owner->def)->eclass->classtype & 0x10 ) != 0 )   // CLASS_PREFAB
            {
                continue;
            }
        }
        if ( FilterBrush( sbn, 0 ) )                                    // 0x48d4be
        {
            continue;
        }
        int brushflags = sbn->brushFlags;                              // 0x48d4ce
        if ( ( brushflags & BRUSHFLAG_SELECTED ) != 0 )
        {
            continue;
        }
        if ( ( brushflags & 0x20 ) != 0 && ( contents & 0x1000 ) == 0 )
        {
            continue;
        }

        iassert( sbn->def->owner == (entity_s *)sbn->owner->def );   // select.cpp:233 brush->def->owner==brush->owner->def

        eclass_t *eclass = ((entity_s *)sbn->owner->def)->eclass;       // 0x48d579
        // hidden/fixedsize/model gate (0x48d5d2): pickable unless a point/model entity the
        // current pass excludes.
        bool ok = ( eclass->classtype & 0x10 ) != 0
               || ( ( ( contents & 0x200 ) == 0 || !*(int *)&eclass->fixedsize )
                 && ( ( contents & 0x400 ) == 0 || !Model_SetModel( sbn, (int)(intptr_t)a5 ) ) );
        if ( !ok )
        {
            continue;
        }
        if ( sbn->patch && !( *(int *)&g_PrefsDlg->m_bSelectCurves && ( contents & 0x10 ) == 0 ) )
        {
            continue;
        }

        // sky-brush disable (0x48d602): when SkyBrushOff is set, skip faces whose base
        // material name contains "sky_".
        if ( g_PrefsDlg->sky_brush_off )
        {
            LayerMaterialDef *nm = Materialdef_GetName(
                &sbn->def->faces->mtldef[g_qeglobals.current_edit_layer] );
            if ( strstr( (const char *)nm, "sky_" ) )
                continue;
        }

        edTrace_t v27;                                                  // [ebp-58h]
        sub_48D240( start, dir, contents, sbn, a5, &v27 );             // 0x48d628
        if ( !v27.face )
            continue;
        // face-texture-filter test: skip if the hit face's material is filtered out.
        // (CPU mode: visArray is NULL — derive the Material* from the def face's layer.)
        Material *faceMtl = nullptr;                                    // v27.face->visArray->handle
        if ( v27.face->visArray )
            faceMtl = v27.face->visArray->mtlHandle;
        if ( sub_46FCF0( faceMtl ) )                                   // 0x46FCF0
            continue;

        float dist = v27.dist;                                         // v27.dist
        // de-dup: walk forward in the output trace array, stopping at the first slot
        // whose dist differs from this hit by > 0.5 (so a coincident multi-list hit
        // isn't recorded twice). Then keep it if it's strictly nearer than that slot.
        edTrace_t *slot = a6;                                          // a6 advances
        int v16 = 0;
        while ( v16 < a7 - 1 )
        {
            if ( fabs( dist - slot->dist ) > 0.5f )
                break;
            ++v16;
            ++slot;
        }
        if ( dist > 0.0f && slot->dist > dist )                        // 0x48d757
        {
            memcpy( slot, &v27, sizeof( edTrace_t ) );
            slot->selected = ( sb == &selected_brushes );             // a6->selected = sb==&selected_brushes
            if ( ( contents & 0x1000 ) != 0 && ( sbn->brushFlags & 0x20 ) != 0 )
                slot->_pad[0] = 1;                                     // *(&a6->selected+1) = 1
        }
    }
}

// ── Test_Ray (0x48d7c0) — the top pick-ray entry ──────────────────────────────────
//  Trace `dir` from `start` through up to num_traces nearest brushes (the array t).
//  contents bit 0x40 = cycle pick: deselect, gather every hit brush under the cursor and
//  pick the one BEFORE the current selection.  Otherwise walk the active + selected lists
//  (a set bit SUPPRESSES that list) and then the &4 world-priority retry.
void Test_Ray( float *start, float *dir, int contents, edTrace_t *t, int num_traces )
{
    memset( t, 0, sizeof( edTrace_t ) * num_traces );        // memset(t, 0, 0x58*num_traces)
    for ( int k = 0; k < num_traces; ++k )                   // t[k].dist = 262144.0
        t[k].dist = 262144.0f;

    int x_contents = contents;                               // x_contents (the &1/&2/&4 mask below)
    if ( ( contents & 0x40 ) != 0 )
    {
        // ── cycle-pick branch (contents & 0x40) ──
        std::vector<selbrush_t *> hits;                      // CPtrArray array (0x48d831)
        // pToSelect = the currently-selected brush (or NULL): used to pick the one before it.
        selbrush_t *pToSelect = ( &selected_brushes != selected_brushes.next )
                              ? selected_brushes.next : nullptr;
        Select_Deselect( 1 );                                // 0x48d856

        int passFlags = contents & 0x1000;                  // x_contents = contents & 0x1000
        for ( selbrush_t *brush = active_brushes.next;
              brush != &active_brushes;
              brush = brush->next )
        {
            if ( FilterBrush( brush, 0 ) )
                continue;
            int bf = brush->brushFlags;                      // [esi+34h]
            if ( ( bf & BRUSHFLAG_SELECTED ) != 0 )          // shr 1, test 1 → bit1 (0x2)
                continue;
            if ( ( bf & 0x20 ) != 0 )                        // shr 5, test 1 → bit5 (0x20)
                continue;
            if ( brush->patch && !*(int *)&g_PrefsDlg->m_bSelectCurves )
                continue;

            edTrace_t a4a;                                   // [ebp-68h]
            sub_48D240( start, dir, passFlags, brush,
                        (const orientation_t *)world_orient_matrix, &a4a );   // 0x48d8c5
            if ( !a4a.face )                                 // a4a.onext (== trace.face @+4)
                continue;
            // a4a.face->...+8 dereferenced as a brush owner→prev, fed to sub_46FCF0 — but
            // sub_46FCF0 takes a Material*; the binary's [eax+8]/[eax] chain reads the
            // faceVis owner. In CPU mode (visArray NULL) this filter is inert; gather all.
            if ( sub_46FCF0( a4a.face->visArray ? a4a.face->visArray->mtlHandle : nullptr ) )
                continue;
            hits.push_back( brush );                         // array_addsize(array, nSize, brush)
        }

        selbrush_t *winner = pToSelect;
        if ( !hits.empty() )
        {
            int x = 0;
            int nSize = (int)hits.size();
            for ( ; x < nSize; ++x )
                if ( hits[x] == pToSelect )
                    break;
            if ( x >= nSize )                                // current selection not among hits → first
            {
                winner = hits[0];
            }
            else
            {
                int v15 = ( x <= 0 ) ? ( nSize - 1 ) : ( x - 1 );   // the brush BEFORE the selected one
                winner = hits[v15];
            }
        }

        if ( winner )
        {
            iassert( num_traces == 1 );                      // select.cpp:352 "maxTraces == 1"
            t->normal[0] = 0.0f;
            t->normal[1] = 0.0f;
            t->normal[2] = 1.0f;
            float dist;
            faceVis_s *face = Brush_Ray( winner, dir, start,
                                         (const orientation_t *)world_orient_matrix,
                                         &dist, t->normal );
            t->dist     = dist;
            t->brush    = winner;
            t->face     = face;
            t->selected = 0;
            return;                                          // 0x48d9d9
        }
        // 0x48d9ff: cycle-pick found nothing -> FALL THROUGH to the ordinary two-list walk,
        // and with the cycle-pass mask (contents & 0x1000), not the original contents.  That
        // retry is the only path that can still reach a flag-0x20 brush (sub_48D460 admits
        // one when contents&0x1000; the gather loop above rejects it unconditionally).
        x_contents = passFlags;
    }

    // ── normal walk: active list, selected list, then the &4 world-priority retry ──
    if ( ( x_contents & 1 ) == 0 )
        sub_48D460( start, dir, x_contents, &active_brushes,
                    (const orientation_t *)world_orient_matrix, t, num_traces );
    if ( ( x_contents & 2 ) == 0 )
        sub_48D460( start, dir, x_contents, &selected_brushes,
                    (const orientation_t *)world_orient_matrix, t, num_traces );
    if ( ( x_contents & 4 ) != 0 && !t->brush )
    {
        edTrace_t a4a;
        Test_Ray( start, dir, x_contents - 4, &a4a, 1 );
        memcpy( t, &a4a, sizeof( edTrace_t ) );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48DAA0  Trace_AllDirectionsIfFailed - drop-to-floor ray with a jitter fallback.
//  Trace `dir` from `cam_origin`; if nothing was hit, retry from four origins jittered
//  +/-1 on X and Y (so a ray that slips between two world brushes still finds the floor).
//  The IDB jitters via three contiguous stack floats (a1/v6/v7); the port uses a vec3.
// ═════════════════════════════════════════════════════════════════════════════
edTrace_t *Trace_AllDirectionsIfFailed( float *cam_origin, edTrace_t *trace_result,
                                        float *dir, int contents )
{
    Test_Ray( cam_origin, dir, contents, trace_result, 1 );
    if ( !trace_result->brush )
    {
        static const float jit[4][2] = { { 1.0f, 1.0f }, { 1.0f, -1.0f },
                                         { -1.0f, 1.0f }, { -1.0f, -1.0f } };
        for ( int k = 0; k < 4 && !trace_result->brush; ++k )
        {
            float o[3] = { cam_origin[0] + jit[k][0],
                           cam_origin[1] + jit[k][1],
                           cam_origin[2] };
            Test_Ray( o, dir, contents, trace_result, 1 );
        }
    }
    return trace_result;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48DCC0  Select_Brush - move brushes between the active and selected lists.
//  KISAK SUBSET: the MFC bStatus status-text + the center_grid PositionView are dropped.
//  __usercall: b in ECX -> first param.
// ═════════════════════════════════════════════════════════════════════════════
void Select_Brush( selbrush_t *b, char some_overwrite, char bStatus, char center_grid_on_selection )
{
    if ( SEL_FACE_COUNT() )
    {
        if ( !b->patch )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    426, 0, "%s", "(g_SelectedFaces.GetSize() == 0) || brush->patch" );
    }
    entity_s *e = b->owner;
    iassert( e );

    if ( e == world_entity || some_overwrite != 1 )
    {
        Brush_RemoveFromList( b );
        Brush_AddToList2( b );
    }
    else
    {
        selbrush_t *v6 = active_brushes.next;
        if ( v6 != &active_brushes )
        {
            do
            {
                selbrush_t *v6next = v6->next;
                if ( v6->owner == e )
                {
                    Brush_RemoveFromList( v6 );
                    Brush_AddToList2( v6 );
                }
                v6 = v6next;
            }
            while ( v6 != &active_brushes );
        }
    }

    // MFC status bar and grid centering are Phase 5; skip silently.
    (void)bStatus;
    (void)center_grid_on_selection;
}

// ═════════════════════════════════════════════════════════════════════════════
//  g_SelectedFaces CArray backbone (IDB CArray at off_73C70C / sub_494610).
//  selFace = the selface_t[] data; SEL_FACE_COUNT() = the live count; g_selFaceSize = the
//  allocated capacity.  SelFace_SetSize models MFC CArray::SetSize (grow-by =
//  clamp(count/8, 4, 1024)).
// ═════════════════════════════════════════════════════════════════════════════
static void SelFace_SetSize( int n )            // sub_494610
{
    int *pCount = (int *)g_ptrSelectedFaces_GetSize;
    if ( n < 0 ) { unknown_libname_291(); return; }
    if ( n == 0 )
    {
        if ( selFace ) { j__free_0( selFace ); selFace = nullptr; }
        g_selFaceSize = 0;
        *pCount       = 0;
        return;
    }
    if ( !selFace )                              // first allocation
    {
        int cap = ( n <= g_selFaceSize ) ? g_selFaceSize : n;
        selFace = (selface_t *)malloc( sizeof( selface_t ) * cap );
        memset( selFace, 0, sizeof( selface_t ) * cap );
        g_selFaceSize = cap;
        *pCount       = n;
        return;
    }
    if ( n > g_selFaceSize )                      // grow capacity
    {
        // IDA 0x4946cc: grow-by = *(a2+8)/8 = the current COUNT/8 (MFC CArray m_nSize/8),
        // not capacity/8.
        int growby = *pCount / 8;
        if ( growby < 4 )       growby = 4;
        else if ( growby > 1024 ) growby = 1024;
        int newcap = g_selFaceSize + growby;
        if ( n >= newcap ) newcap = n;
        selface_t *nf = (selface_t *)malloc( sizeof( selface_t ) * newcap );
        memcpy( nf, selFace, sizeof( selface_t ) * (*pCount) );
        memset( nf + *pCount, 0, sizeof( selface_t ) * ( newcap - *pCount ) );
        j__free_0( selFace );
        selFace       = nf;
        g_selFaceSize = newcap;
    }
    else if ( n > *pCount )                       // grow size within capacity
    {
        memset( selFace + *pCount, 0, sizeof( selface_t ) * ( n - *pCount ) );
    }
    *pCount = n;
}

// sub_4947A0 — append one face record {brush, faceVis*, index}; return its index.
int sub_4947A0( selbrush_t **rec )
{
    int idx = SEL_FACE_COUNT();
    if ( idx < 0 ) unknown_libname_291();
    SelFace_SetSize( idx + 1 );
    selFace[idx].brush = rec[0];
    selFace[idx].face  = (faceVis_s *)rec[1];
    selFace[idx].index = (int)(intptr_t)rec[2];
    return idx;
}

// sub_480670 — RemoveAt(a1, a2): drop a2 entries starting at index a1.
void sub_480670( int a1, int a2 )
{
    int *pCount = (int *)g_ptrSelectedFaces_GetSize;
    int  result = a1 + a2;
    if ( a1 < 0 || a2 < 0 || result > *pCount || result < a1 || result < a2 )
        unknown_libname_291();
    if ( *pCount != result )
        memmove( &selFace[a1], &selFace[result], sizeof( selface_t ) * ( *pCount - result ) );
    *pCount -= a2;
}

// Insert a non-patch brush at the head of active_brushes (the cluster's
// "demote a brush back to the active list" idiom; &active==&selected never holds).
static void Brush_ToActiveHead( selbrush_t *b )
{
    Brush_RemoveFromList( b );
    if ( b->next || b->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    b->next             = active_brushes.next;
    active_brushes.next->prev = b;
    active_brushes.next = b;
    b->prev             = &active_brushes;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48E050  sub_48E050 - remove ALL of a brush's faces from g_SelectedFaces.
//  (Patch brushes: move the brush back to the active list instead.)
// ═════════════════════════════════════════════════════════════════════════════
void sub_48E050( selbrush_t *a1 )
{
    if ( a1->patch )
    {
        Brush_ToActiveHead( a1 );
        return;
    }
    int *pCount = (int *)g_ptrSelectedFaces_GetSize;
    for ( unsigned v7 = 0; v7 < (unsigned)a1->faceCount; ++v7 )
    {
        faceVis_s *target = &SEL_FACES( a1 )[v7];
        int sz = *pCount;
        int v3 = 0;
        while ( v3 < sz && selFace[v3].face != target )
            ++v3;
        if ( v3 < sz )                       // found → RemoveAt(v3, 1)
        {
            int v5 = v3 + 1;
            if ( v5 > sz || v5 < v3 || v5 < 1 ) unknown_libname_291();
            if ( sz != v5 )
                memmove( &selFace[v3], &selFace[v5], sizeof( selface_t ) * ( sz - v5 ) );
            *pCount = sz - 1;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48DEC0  sub_48DEC0 - ADD all of a brush's (winding-bearing) faces to
//  g_SelectedFaces, skipping any already present.  (Patch brushes: select-as-brush.)
//  Brush_CheckBuildFaceVis runs once at the top; the version compare is 16-bit (0x48df89).
//  The tautological assert 505 is folded away by hex-rays (omitted, inert).
// ═════════════════════════════════════════════════════════════════════════════
void sub_48DEC0( selbrush_t *a1 )
{
    sub_477D70( a1, (const float *)world_orient_matrix );   // Brush_CheckBuildFaceVis
    if ( a1->patch )
    {
        Brush_RemoveFromList( a1 );
        if ( a1->next || a1->prev )
            Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
        Brush_AddToList2( a1 );
        return;
    }
    if ( !a1->faceCount )
        return;
    int *pCount = (int *)g_ptrSelectedFaces_GetSize;
    for ( unsigned v10 = 0; v10 < (unsigned)a1->faceCount; ++v10 )
    {
        faceVis_s *target = &SEL_FACES( a1 )[v10];
        int sz = *pCount;
        int v4 = 0;
        while ( v4 < sz && selFace[v4].face != target )
            ++v4;
        if ( v4 < sz )                        // already selected
        {
            if ( a1 != selFace[v4].brush )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        494, 0, "%s", "brush == g_SelectedFaces.GetAt( i ).brush" );
        }
        else                                  // add it
        {
            if ( a1->version != a1->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        506, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
            if ( sz < 0 ) unknown_libname_291();
            SelFace_SetSize( sz + 1 );
            selFace[sz].brush = a1;
            selFace[sz].face  = target;
            selFace[sz].index = (int)v10;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48E170  sub_48E170 - convert the current whole-brush selection into a face
//  selection: for every selected (non-patch) brush, clear any of its faces already
//  recorded, add all winding-bearing faces, then demote the brush to the active list.
// ═════════════════════════════════════════════════════════════════════════════
void sub_48E170()
{
    extern CWnd *g_PatchDialog_GetHwnd();        // brush.cpp / engine_stubs
    extern void  g_PatchDialog_GetPatchInfo();
    if ( selected_brushes.next == &selected_brushes )
        return;

    int *pCount = (int *)g_ptrSelectedFaces_GetSize;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( b->patch )
            continue;
        sub_48E050( b );
        for ( unsigned i = 0; i < (unsigned)b->faceCount; ++i )
        {
            // IDA 0x48e1c2: capture the faceVis ptr, rebuild faceVis PER-FACE (idempotent,
            // version-gated), then assert it didn't move (570) and the instance/def
            // versions are synced (571).
            faceVis_s *fv = &SEL_FACES( b )[i];
            sub_477D70( b, (const float *)world_orient_matrix );
            if ( &SEL_FACES( b )[i] != fv )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        570, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( b->version != b->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        571, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
            if ( !b->def->faces[i].w )                   // skip faces with no winding
                continue;
            int sz = *pCount;
            if ( sz < 0 ) unknown_libname_291();
            SelFace_SetSize( sz + 1 );
            selFace[sz].brush = b;
            selFace[sz].face  = fv;
            selFace[sz].index = (int)i;
        }
    }
    // Demote every non-patch selected brush back to the active list.
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; )
    {
        selbrush_t *next = b->next;
        if ( !b->patch )
            Brush_ToActiveHead( b );
        b = next;
    }
    // IDA 0x48e314: re-bind the Patch Inspector to the new selection (sibling idiom).
    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
}

// Ed_SelectFirstEditableFace - KISAK helper (selftest texmod gate + the GUI texmod demo
// hook): select the first active non-patch, non-fixedsize brush's first winding-bearing
// face whose base-layer material is not "lightmap_gray" (which TexMatToFakeTexCoords
// canonicalises, clobbering a shift/rotate edit on reload).  Returns 1 on success.
extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );  // materialdef.cpp 0x431640
extern char        g_bNewFace;                           // surfacedlg.cpp

int Ed_SelectFirstEditableFace()
{
    *(int *)g_ptrSelectedFaces_GetSize = 0;              // start from an empty face selection

    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        if ( b->patch || !b->def || b->def->faceCount <= 0 )
            continue;
        entity_s_def *od = (entity_s_def *)b->owner->def;
        if ( *(int *)&od->eclass->fixedsize )            // bbox brushes have no editable faces
            continue;

        int before = SEL_FACE_COUNT();
        sub_48DEC0( b );                                  // build faceVis + add all winding faces
        int added = SEL_FACE_COUNT() - before;
        if ( added <= 0 )
            continue;
        if ( added > 1 )                                  // keep only the first face
            sub_480670( before + 1, added - 1 );

        MaterialDef *md = &b->def->faces[ selFace[before].index ]
                                .mtldef[ g_qeglobals.current_edit_layer ];
        const char *nm = (const char *)Materialdef_GetName( md );
        if ( nm && !strcmp( nm, "lightmap_gray" ) )       // unsuitable → drop and keep looking
        {
            sub_480670( before, 1 );
            continue;
        }
        g_bNewFace = 1;
        return 1;
    }
    return 0;
}

// Select_SetTexture (0x456D70) - the default texture-repeat scale (x,y) for
// Patch->Naturalize.  KISAK SUBSET: the IDB's inspector-OPEN branch multiplies the default
// by the surface inspector's IDC_SURFACE_INSP_TEX_REP_X/Y edits, which the port's
// hand-built inspector does not have; both IDB branches converge on the default-scale slot
// random_texture_stuff[2100*layer + 36] (== [+0x24]) that the closed branch reads.
void Select_SetTexture( float *out )
{
    const int layer = g_qeglobals.current_edit_layer;
    const float scale = g_qeglobals.random_texture_stuff[layer].sampleSize;
    out[0] = scale;
    out[1] = scale;
}

// Select_Invert (0x493F10) - Selection->Invert (Ctrl+I, cmd 33101).  Swaps the active and
// selected display lists wholesale, then re-runs the per-brush bookkeeping
// (Brush_Deselect_Helper on the now-active set, Brush_Select_Helper on the now-selected)
// and the patch instance's selected byte (instance offset 6).  Empty-list cases reset the
// sentinel to itself.  IDB selected_brushes / selected_brushes_next == .prev / .next here.
void Select_Invert()
{
    Sys_Printf( "inverting selection...\n" );
    selbrush_t *oldActiveHead = active_brushes.next;
    selbrush_t *oldActiveTail = active_brushes.prev;

    if ( selected_brushes.next == &selected_brushes )      // selected list empty → active empties
    {
        active_brushes.next = &active_brushes;
        active_brushes.prev = &active_brushes;
    }
    else                                                   // active list ← old selected list
    {
        active_brushes.next        = selected_brushes.next;
        active_brushes.prev        = selected_brushes.prev;
        selected_brushes.next->prev = &active_brushes;
        active_brushes.prev->next   = &active_brushes;
    }

    if ( oldActiveHead == &active_brushes )                // old active list empty → selected empties
    {
        selected_brushes.next = &selected_brushes;
        selected_brushes.prev = &selected_brushes;
    }
    else                                                   // selected list ← old active list
    {
        selected_brushes.prev      = oldActiveTail;
        selected_brushes.next      = oldActiveHead;
        oldActiveHead->prev        = &selected_brushes;
        selected_brushes.prev->next = &selected_brushes;
    }

    for ( selbrush_t *i = active_brushes.next; i != &active_brushes; i = i->next )
    {
        Brush_Deselect_Helper( i );
        if ( i->patch )
            i->patch->selected = 0;                        // per-instance selected flag (IDB BYTE2(patch[1].def))
    }
    for ( selbrush_t *j = selected_brushes.next; j != &selected_brushes; j = j->next )
    {
        Brush_Select_Helper( j );
        if ( j->patch )
            j->patch->selected = 1;
    }
    g_nUpdateBits = -1;
    Sys_Printf( "done.\n" );
}


// ═════════════════════════════════════════════════════════════════════════════
//  0x48DC60  Deselect_Brush — move a brush from the selected list back to active.
// ═════════════════════════════════════════════════════════════════════════════
void Deselect_Brush( selbrush_t *b )
{
    Brush_RemoveFromList( b );
    if ( b->next || b->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    // Insert at the head of active_brushes.
    b->next = active_brushes.next;
    active_brushes.next->prev = b;
    active_brushes.next = b;
    b->prev = &active_brushes;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48E340  SelectFaceSth - pick a brush/face under the ray and select/deselect.
//    contents&8 (Ctrl+Shift+LMB, 520): single-FACE select/toggle into g_SelectedFaces.
//    else, with faces already selected: brush<->faceset toggle (sub_48DEC0/sub_48E050).
//    else: whole-brush select/deselect; Alt = the whole entity group.
//  IDA args: (trace_dir, trace_start, contents) in ecx/edx/eax.
// ═════════════════════════════════════════════════════════════════════════════
extern int  g_nUpdateBits;                       // 0x25D5A74
extern void CMainFrame_UpdatePatchToolbarButtons();

void SelectFaceSth( int a1_dir, int a2_start, int a3_contents )
{
    float *trace_dir   = (float *)(intptr_t)a1_dir;
    float *trace_start = (float *)(intptr_t)a2_start;
    int    contents    = a3_contents;

    edTrace_t trace;
    Test_Ray( trace_start, trace_dir, contents, &trace, 1 );
    selbrush_t *brush = trace.brush;
    if ( !brush )
        return;
    // 0x48e381: on the Z-view (contents & 0x1000, set by Drag_Begin when viewz==2,
    // drag.cpp:588) reject the pick when brush->brushFlags & 0x20 OR the trace's +0x49 byte
    // is set (sub_48D460 sets it when a 0x20 brush was admitted by the 0x1000 pass - which
    // for a prefab hit is an INNER brush, not this one).
    if ( ( contents & 0x1000 ) && ( ( brush->brushFlags & 0x20 ) || trace._pad[0] ) )
        return;

    // KISAK: 0x4000 = light-preview face pick.  The binary diverts to the per-light preview
    // (sub_406330) only when the ray hit a LIGHT entity (eclass flag @+0x180) and otherwise
    // falls through to the normal select.  sub_406330 is parked and the port does not
    // populate the light-pick target (trace.xx1), so we fall through unconditionally:
    // clicking a light bbox simply selects it.
    (void)contents;

    // Reset transient edit modes (IDA resets d_select_mode to sel_brush first).
    select_t prevMode = g_qeglobals.d_select_mode;
    g_qeglobals.d_select_mode = sel_brush;
    if ( prevMode == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prevMode == sel_addpoint )
        sub_43ECB0();

    // ── single-FACE selection (Ctrl+Shift+LMB) ────────────────────────────────
    if ( ( contents & 8 ) != 0 && !brush->patch )
    {
        faceVis_s *face = trace.face;
        if ( !face )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    631, 0, "%s", "t.hit.face" );

        // Fixed-size entities (bbox brushes) have no editable faces — ignore.
        entity_s_def *od = (entity_s_def *)brush->owner->def;
        if ( *(int *)&od->eclass->fixedsize )
            return;

        sub_48E170();   // fold any whole-brush selection into face selection first

        int sz = SEL_FACE_COUNT();
        int hit = -1;
        for ( int i = 0; i < sz; ++i )
            if ( selFace[i].face == face ) { hit = i; break; }

        if ( hit >= 0 )
        {
            // already selected → toggle off
            if ( !g_qeglobals.toggle_unk03_mousedrag_state1 )
            {
                sub_480670( hit, 1 );
                g_qeglobals.toggle_unk04_mousedrag_state2 = 1;
                g_nUpdateBits = -1;
            }
            return;
        }

        // not selected → add it
        if ( g_qeglobals.toggle_unk04_mousedrag_state2 )
            return;
        int idx = (int)( (char *)face - (char *)brush->faces ) / 12;
        if ( (unsigned)idx >= (unsigned)brush->faceCount )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    664, 0, "%s", "selFace.index >= 0 && selFace.index < t.hit.brush->faceCount" );
        if ( brush->version != brush->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    665, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        selbrush_t *rec[3] = { brush, (selbrush_t *)face, (selbrush_t *)(intptr_t)idx };
        sub_4947A0( rec );
        g_qeglobals.toggle_unk03_mousedrag_state1 = 1;
        g_nUpdateBits = -1;
        return;
    }

    // ── no face selection active (or a patch hit) → whole-brush select/deselect ──
    if ( !SEL_FACE_COUNT() || brush->patch )
    {
        if ( trace.selected )
        {
            if ( !g_qeglobals.toggle_unk03_mousedrag_state1 )
            {
                entity_s *owner = brush->owner;
                if ( owner && owner != world_entity && GetAsyncKeyState( VK_MENU ) < 0 )
                {
                    for ( selbrush_t *v = selected_brushes.next; v != &selected_brushes; )
                    {
                        selbrush_t *next = v->next;
                        if ( v->owner == owner ) Deselect_Brush( v );
                        v = next;
                    }
                }
                else
                {
                    Deselect_Brush( brush );
                }
                {
                    extern CWnd *g_PatchDialog_GetHwnd();        // brush.cpp / engine_stubs
                    extern void  g_PatchDialog_GetPatchInfo();
                    if ( g_PatchDialog_GetHwnd() )               // IDA 0x48e6f4: rebind Patch Inspector after deselect
                        g_PatchDialog_GetPatchInfo();
                }
                g_qeglobals.toggle_unk04_mousedrag_state2 = 1;
                g_nUpdateBits = -1;
            }
            return;
        }
        if ( g_qeglobals.toggle_unk04_mousedrag_state2 )
            return;
        bool alt = GetKeyState( VK_MENU ) < 0;
        Select_Brush( brush, alt, 1, 0 );
        g_qeglobals.toggle_unk03_mousedrag_state1 = 1;
        g_nUpdateBits = -1;
        return;
    }

    // ── faces already selected, hit a non-patch brush → brush↔faceset toggle ─────
    entity_s *grpOwner = brush->owner;
    bool alt = ( grpOwner && grpOwner != world_entity && GetAsyncKeyState( VK_MENU ) < 0 );

    int sz  = SEL_FACE_COUNT();
    int hit = -1;
    for ( int i = 0; i < sz; ++i )
        if ( (face_t *)trace.face == (face_t *)selFace[i].face ) { hit = i; break; }

    if ( hit >= 0 )
    {
        // some of the brush's faces are already selected → remove them (toggle off)
        if ( !g_qeglobals.toggle_unk03_mousedrag_state1 )
        {
            if ( alt && active_brushes.next != &active_brushes )
            {
                for ( selbrush_t *v = active_brushes.next; v != &active_brushes; )
                {
                    selbrush_t *next = v->next;
                    if ( v->owner == grpOwner ) sub_48E050( v );
                    v = next;
                }
            }
            else if ( !alt )
            {
                sub_48E050( brush );
            }
            g_qeglobals.toggle_unk04_mousedrag_state2 = 1;
            g_nUpdateBits = -1;
        }
        return;
    }

    // not selected → add the brush's faces (toggle on)
    if ( g_qeglobals.toggle_unk04_mousedrag_state2 )
        return;
    if ( alt )
    {
        for ( selbrush_t *v = active_brushes.next; v != &active_brushes; )
        {
            selbrush_t *next = v->next;
            if ( v->owner == grpOwner ) sub_48DEC0( v );
            v = next;
        }
    }
    else
    {
        sub_48DEC0( brush );
    }
    g_qeglobals.toggle_unk03_mousedrag_state1 = 1;
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48E800  Select_Deselect - a1=1 also frees selFace; a1=0 leaves the face selection.
//  Head-node aliasing (selected.prev->next), 3-axis maxs>mins new-brush bounds template.
// ═════════════════════════════════════════════════════════════════════════════
void Select_Deselect( int a1 )
{
    Patch_Deselect();

    for ( selbrush_t *i = selected_brushes.next;
          i != &selected_brushes;
          i = i->next )
    {
        i->xx6 = 0;
        Brush_Deselect_Helper( i );
    }
    ++g_qeglobals.d_workcount;

    selbrush_t *v2 = selected_brushes.next;
    g_qeglobals.d_select_count             = 0;
    g_qeglobals.d_select_info.numPatches   = 0;
    g_qeglobals.d_select_info.numBrushes   = 0;
    g_qeglobals.d_select_info.numFixedSize = 0;
    g_qeglobals.d_num_move_points          = 0;

    if ( a1 )
    {
        if ( selFace )
        {
            j__free_0( selFace );
            selFace = nullptr;
        }
        g_selFaceSize        = 0;
        SEL_FACE_COUNT()     = 0;
    }

    if ( v2 != &selected_brushes )
    {
        ResetSelectMode();

        // Copy last-selected brush's bounds as the new-brush template.
        if ( v2->def->maxs[0] > (double)v2->def->mins[0] )
        {
            g_qeglobals.d_new_brush_bottom_x = v2->def->mins[0];
            g_qeglobals.d_new_brush_top_x    = v2->def->maxs[0];
        }
        if ( v2->def->maxs[1] > (double)v2->def->mins[1] )
        {
            g_qeglobals.d_new_brush_bottom_y = v2->def->mins[1];
            g_qeglobals.d_new_brush_top_y    = v2->def->maxs[1];
        }
        if ( v2->def->maxs[2] > (double)v2->def->mins[2] )
        {
            g_qeglobals.d_new_brush_bottom_z = v2->def->mins[2];
            g_qeglobals.d_new_brush_top_z    = v2->def->maxs[2];
        }

        // Splice selected_brushes list into front of active_brushes.
        selected_brushes.next->prev = &active_brushes;
        selected_brushes.prev->next = active_brushes.next;
        active_brushes.next->prev   = selected_brushes.prev;
        active_brushes.next         = selected_brushes.next;
        selected_brushes.next       = &selected_brushes;
        selected_brushes.prev       = &selected_brushes;

        UpdateSelection( 0xFFFFFFFF, 0 );
    }
    g_nUpdateBits = 0xFFFFFFFF;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48E760  Select_Delete - free every selected brush, then free any owner entity left
//  with no brushes (worldspawn excepted).  Used by Map_RegionBrush / Map_RegionTallBrush
//  (delete the defining brush after caging the region) and by Edit->Delete.  The transient
//  face/patch-edit modes are reset first.
// ═════════════════════════════════════════════════════════════════════════════
extern void Entity_Free( char *a1 );    // entity.cpp (0x485750)

void Select_Delete()
{
    if ( selFace )
    {
        j__free_0( selFace );
        selFace = nullptr;
    }
    select_t prevMode      = g_qeglobals.d_select_mode;
    g_selFaceSize          = 0;
    SEL_FACE_COUNT()       = 0;
    g_qeglobals.d_select_mode = sel_brush;
    if ( prevMode == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prevMode == sel_addpoint )
        sub_43ECB0();

    g_qeglobals.d_num_move_points = 0;
    while ( selected_brushes.next != &selected_brushes )
    {
        selbrush_t *sbn   = selected_brushes.next;
        entity_s   *owner = sbn->owner;
        Brush_Free( sbn );   // unlinks sbn from both the display list and its owner list
        // If the owner entity has no brushes left (and isn't worldspawn), free it.
        if ( (void *)owner->brushes.ownerNext == (void *)&owner->brushes
             && owner != world_entity )
            Entity_Free( (char *)owner );
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48EE10  Select_BrushByLayer
// ═════════════════════════════════════════════════════════════════════════════
void Select_BrushByLayer( char *layer_str )
{
    Select_Deselect( 1 );

    selbrush_t *brush = active_brushes.next;
    if ( brush != &active_brushes )
    {
        do
        {
            selbrush_t *brushNext = brush->next;
            if ( !FilterBrush( brush, 0 ) )
            {
                int bf = brush->brushFlags;
                if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
                {
                    iassert( brush->owner );
                    iassert( brush->owner->def == brush->def->owner );

                    if ( !strncmp( brush->def->parent_layer_string, layer_str, strlen(layer_str) )
                         || !strcmp( layer_str, "The Map" ) )
                    {
                        Brush_RemoveFromList( brush );
                        Brush_AddToList2( brush );
                    }
                }
            }
            brush = brushNext;
        }
        while ( brush != &active_brushes );
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  REGION / MARQUEE SELECTION  (Select_*Tall / Select_Touching_R / Select_Inside_R
//  + sub_4902C0 marquee box-build)
//
//  The four "Select ... Tall / Touching / Inside" menu commands take the SINGLE selected
//  brush as a selection BOX (QE_SingleBrush), grab its def mins/maxs, DELETE that brush,
//  then walk active_brushes and (re)select every brush satisfying the box test.  "Tall"
//  tests only the two SCREEN axes of the active XY view; Touching/Inside test all three.
//  sub_4902C0 is the live MARQUEE drag-box select (Drag_MouseUp): it takes a precomputed
//  world box + a flag selecting which list to walk (Alt+Shift+LMB box-ADD mode 12 /
//  Alt+Shift+MMB box-DESELECT mode 13).
//  View-axis selection (identical in all five, IDA-verified):
//    v_a = (m_nViewType == 0)        // YZ view -> axis 1 (Y), else axis 0 (X)
//    v_b = (m_nViewType != 2) + 1    // XY view -> axis 1 (Y), else axis 2 (Z)
//  giving the two on-screen axes: XY {0,1}, XZ {0,2}, YZ {1,2}.
// ═════════════════════════════════════════════════════════════════════════════
extern CMainFrame *g_pParentWnd;                    // engine_stubs.cpp (0x25d5a70)
extern signed int  QE_SingleBrush();                // qe3.cpp (0x48c8b0)

// Active XY view's two on-screen axes (the depth axis is dropped for "Tall").
static inline int Region_ViewAxisA() {
    const CXYWnd *xy = g_pParentWnd ? g_pParentWnd->m_pActiveXY : nullptr;
    int vt = xy ? xy->m_nViewType : 2;   // default XY (top-down)
    return (vt == 0);                    // YZ→1, XZ/XY→0
}
static inline int Region_ViewAxisB() {
    const CXYWnd *xy = g_pParentWnd ? g_pParentWnd->m_pActiveXY : nullptr;
    int vt = xy ? xy->m_nViewType : 2;
    return (vt != 2) + 1;                // XY→1, XZ/YZ→2
}

// Common prologue for the four menu cores (head of Select_CompleteTall 0x490183..0x4901db):
// snapshot the single selected brush's box, reset the transient select mode, delete it.
static bool Region_BeginFromSingleBrush( float boxMins[3], float boxMaxs[3] )
{
    if ( !QE_SingleBrush() )
        return false;

    ResetSelectMode();   // sel_cycle→UpdatePatchToolbar / sel_addpoint→sub_43ECB0; mode→sel_brush

    brush_t *def = selected_brushes.next->def;
    boxMins[0] = def->mins[0];  boxMins[1] = def->mins[1];  boxMins[2] = def->mins[2];
    boxMaxs[0] = def->maxs[0];  boxMaxs[1] = def->maxs[1];  boxMaxs[2] = def->maxs[2];
    Select_Delete();           // free the brush that defined the box
    return true;
}

// ── 0x490170  Select_CompleteTall ────────────────────────────────────────────
// Select every brush COMPLETELY CONTAINED by the box on the two screen axes.
void Select_CompleteTall()
{
    float mins[3], maxs[3];
    if ( !Region_BeginFromSingleBrush( mins, maxs ) )
        return;

    const int a = Region_ViewAxisA();
    const int b = Region_ViewAxisB();

    for ( selbrush_t *sb = active_brushes.next; sb != &active_brushes; )
    {
        brush_t    *def  = sb->def;
        selbrush_t *next = sb->next;
        // containment: box.maxs >= brush.maxs && box.mins <= brush.mins on both axes
        if ( maxs[a] >= def->maxs[a] && mins[a] <= def->mins[a]
          && maxs[b] >= def->maxs[b] && mins[b] <= def->mins[b]
          && !FilterBrush( sb, 0 ) )
        {
            int bf = sb->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                Brush_RemoveFromList( sb );
                Select_Brush_2( &selected_brushes, sb );
            }
        }
        sb = next;
    }
    g_nUpdateBits = -1;
}

// ── 0x4903D0  Select_PartialTall ─────────────────────────────────────────────
// Select every brush that OVERLAPS the box on the two screen axes.
void Select_PartialTall()
{
    float mins[3], maxs[3];
    if ( !Region_BeginFromSingleBrush( mins, maxs ) )
        return;

    const int a = Region_ViewAxisA();
    const int b = Region_ViewAxisB();

    for ( selbrush_t *sb = active_brushes.next; sb != &active_brushes; )
    {
        brush_t    *def  = sb->def;
        selbrush_t *next = sb->next;
        // overlap: box.maxs >= brush.mins && box.mins <= brush.maxs on both axes
        if ( maxs[a] >= def->mins[a] && mins[a] <= def->maxs[a]
          && maxs[b] >= def->mins[b] && mins[b] <= def->maxs[b]
          && !FilterBrush( sb, 0 ) )
        {
            int bf = sb->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                Brush_RemoveFromList( sb );
                Select_Brush_2( &selected_brushes, sb );
            }
        }
        sb = next;
    }
    g_nUpdateBits = -1;
}

// ── 0x490520  Select_Touching_R ──────────────────────────────────────────────
// Select every brush whose bounds touch the box on ALL THREE axes (with a +1 unit
// epsilon — "touching" includes brushes that just abut the box face).
void Select_Touching_R()
{
    float mins[3], maxs[3];
    if ( !Region_BeginFromSingleBrush( mins, maxs ) )
        return;

    for ( selbrush_t *sb = active_brushes.next; sb != &active_brushes; )
    {
        selbrush_t *next = sb->next;
        brush_t    *def  = sb->def;
        int i = 0;
        for ( ; i < 3; ++i )
        {
            if ( maxs[i] + 1.0f < def->mins[i] ) break;   // box.maxs+1 < brush.mins → miss
            if ( mins[i] - 1.0f > def->maxs[i] ) break;   // box.mins-1 > brush.maxs → miss
        }
        if ( i == 3 && !FilterBrush( sb, 0 ) )
        {
            int bf = sb->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                Brush_RemoveFromList( sb );
                if ( sb->next || sb->prev )
                    Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
                Brush_AddToList2( sb );
            }
        }
        sb = next;
    }
    g_nUpdateBits = -1;
}

// ── 0x490650  Select_Inside_R ────────────────────────────────────────────────
// Select every brush fully CONTAINED by the box on ALL THREE axes (no epsilon).
void Select_Inside_R()
{
    float mins[3], maxs[3];
    if ( !Region_BeginFromSingleBrush( mins, maxs ) )
        return;

    for ( selbrush_t *sb = active_brushes.next; sb != &active_brushes; )
    {
        selbrush_t *next = sb->next;
        brush_t    *def  = sb->def;
        int i = 0;
        for ( ; i < 3; ++i )
        {
            if ( maxs[i] < def->maxs[i] ) break;   // box.maxs < brush.maxs → not contained
            if ( mins[i] > def->mins[i] ) break;   // box.mins > brush.mins → not contained
        }
        if ( i == 3 && !FilterBrush( sb, 0 ) )
        {
            int bf = sb->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                Brush_RemoveFromList( sb );
                if ( sb->next || sb->prev )
                    Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
                Brush_AddToList2( sb );
            }
        }
        sb = next;
    }
    g_nUpdateBits = -1;
}

// ── 0x4902C0  Select_FlipFilteredBrushes (marquee drag box-select) ────────────
//  Called from Drag_MouseUp for the live marquee. The box [boxMins,boxMaxs] is
//  pre-built in world space from the drag rectangle. bActiveList selects which
//  list to scan and where matches go:
//    bActiveList = 1 (mode 12, box ADD)     → walk active_brushes,   move matches → selected
//    bActiveList = 0 (mode 13, box DESELECT) → walk selected_brushes, move matches → active
//  The bounds test is CONTAINMENT on the active XY view's two screen axes (Tall),
//  identical to Select_CompleteTall.
void Select_FlipFilteredBrushes( const float *boxMins, const float *boxMaxs, char bActiveList )
{
    const int a = Region_ViewAxisA();
    const int b = Region_ViewAxisB();

    selbrush_t *head = bActiveList ? &active_brushes : &selected_brushes;
    selbrush_t *dst  = bActiveList ? &selected_brushes : &active_brushes;

    for ( selbrush_t *sb = head->next; sb != head; )
    {
        brush_t    *def  = sb->def;
        selbrush_t *next = sb->next;
        if ( boxMaxs[a] >= def->maxs[a] && boxMins[a] <= def->mins[a]
          && boxMaxs[b] >= def->maxs[b] && boxMins[b] <= def->mins[b]
          && !FilterBrush( sb, 0 ) )
        {
            int bf = sb->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                Brush_RemoveFromList( sb );
                Select_Brush_2( dst, sb );
            }
        }
        sb = next;
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48F170  Brush_SetTexture - mtldef copy stride 36*(layer+1), def version @0x4E is
//  16-bit, patch brushes fan the copy out to every face.
// ═════════════════════════════════════════════════════════════════════════════
void Brush_SetTexture( MaterialDef *a1, char a3 )
{
    int  v24 = SEL_FACE_COUNT();
    const char *opName;

    if ( v24 )
        opName = "set face textures";
    else
    {
        if ( selected_brushes.next == &selected_brushes )
            return;
        opName = "set brush textures";
    }

    Undo_ClearRedo();
    Undo_GeneralStart( opName );

    int v28 = 0;
    if ( v24 > 0 )
    {
        for ( int v4 = 0; v28 < v24; v4 += 12, ++v28 )
        {
            if ( v28 < 0 || v28 >= SEL_FACE_COUNT() )
                unknown_libname_291();

            selbrush_t *b     = *(selbrush_t **)((char *)selFace + v4);
            int         index = *(int *)         ((char *)selFace + v4 + 8);
            brush_t    *b_def = b->def;
            // IDA: face = *(face_t **)(&selFace->face + v4) - DEREFERENCE the stored
            // faceVis_s*, not the address of the field (as all 7 sibling selFace loops do).
            faceVis_s  *face  = *(faceVis_s **)  ((char *)selFace + v4 + 4);

            Undo_TryAddBrush( b_def );

            int v25 = index;
            faceVis_s *fv = &SEL_FACES(b)[index];
            if ( (void *)face != (void *)fv )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1114, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( b->version != b->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1115, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
            if ( b->faceCount != b->def->faceCount )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1116, 0, "%s", "selFace.brush->faceCount == selFace.brush->def->faceCount" );

            // Copy MaterialDef into the face's current layer slot: the IDB writes to
            // &def->faces[index].mtldef[layer] (= v26 + 4*(9*layer+9) = +36*(layer+1)).
            MaterialDef *dstSlot = &b_def->faces[index].mtldef[g_qeglobals.current_edit_layer];
            qmemcpy( dstSlot, a1, sizeof(MaterialDef) );

            if ( b_def->patch )
            {
                for ( unsigned v10 = 0; v10 < (unsigned)b_def->faceCount; ++v10 )
                {
                    qmemcpy( (char *)&b_def->faces[v10].mtldef[g_qeglobals.current_edit_layer],
                             dstSlot,
                             sizeof(MaterialDef) );
                }
            }

            ++b_def->version;
            Brush_BuildWindings( b->def, 0 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            MarkMapModified();
            ++b->def->version;
            sub_47B940( b->def );
            Undo_LinkBrush( b->def );
            sub_477D70( b, (const float *)world_orient_matrix );
            if ( (void *)face != (void *)&SEL_FACES( b )[index] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1125, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( b->version != b->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1126, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        }
    }

    // Per-brush pass (non-fixedsize brushes only).
    for ( selbrush_t *v16 = selected_brushes.next;
          v16 != &selected_brushes;
          v16 = v16->next )
    {
        entity_s_def *def = (entity_s_def *)v16->owner->def;
        if ( !*(int *)&def->eclass->fixedsize )
        {
            Undo_TryAddBrush( v16->def );
            sub_476ED0( v16->def, a1, a3, 0.0f, 1 );
            Undo_LinkBrush( v16->def );
        }
    }

    Undo_End();
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48F4F0  Brush_SetTextureMapping - Brush_SetFaceTexdef stride 232, def version @0x4E.
// ═════════════════════════════════════════════════════════════════════════════
void Brush_SetTextureMapping( texdef_sub_t *a2 )
{
    int v1 = SEL_FACE_COUNT();
    const char *opName;

    if ( v1 )
        opName = "set face texture mapping";
    else
    {
        if ( selected_brushes.next == &selected_brushes )
            return;
        opName = "set brush texture mapping";
    }

    Undo_ClearRedo();
    Undo_GeneralStart( opName );

    int v22 = 0;
    if ( v1 > 0 )
    {
        for ( int v3 = 0; v22 < v1; v3 += 12, ++v22 )
        {
            if ( v22 < 0 || v22 >= SEL_FACE_COUNT() )
                unknown_libname_291();

            selbrush_t *v4  = *(selbrush_t **)((char *)selFace + v3);
            int         v5  = *(int *)         ((char *)selFace + v3 + 8);
            brush_t    *v6  = v4->def;
            faceVis_s  *v19 = *(faceVis_s **)  ((char *)selFace + v3 + 4);

            Undo_TryAddBrush( v6 );

            if ( v19 != &SEL_FACES(v4)[v5] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1161, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( v4->version != v4->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1162, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
            if ( v4->faceCount != v4->def->faceCount )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1163, 0, "%s", "selFace.brush->faceCount == selFace.brush->def->faceCount" );

            sub_4767E0( a2, (int)(intptr_t)&v6->faces[v5], (int)(intptr_t)v6 );

            Brush_BuildWindings( v6, 1 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            MarkMapModified();
            ++v6->version;
            sub_47B940( v6 );
            Undo_LinkBrush( v6 );
            sub_477D70( v4, (const float *)world_orient_matrix );
            if ( v19 != &SEL_FACES( v4 )[v5] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1172, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( v4->version != v4->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1173, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        }
    }

    for ( selbrush_t *v13 = selected_brushes.next;
          v13 != &selected_brushes;
          v13 = v13->next )
    {
        entity_s_def *def = (entity_s_def *)v13->owner->def;
        if ( !*(int *)&def->eclass->fixedsize )
        {
            Undo_TryAddBrush( v13->def );
            sub_477020( v13->def, a2 );
            Undo_LinkBrush( v13->def );
        }
    }

    Undo_End();
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48F800  Brush_SetSampleSize
// ═════════════════════════════════════════════════════════════════════════════
void Brush_SetSampleSize( int size )
{
    int v1 = SEL_FACE_COUNT();
    const char *opName;

    if ( v1 )
        opName = "set face samplesize";
    else
    {
        if ( selected_brushes.next == &selected_brushes )
            return;
        opName = "set brush samplesize";
    }

    Undo_ClearRedo();
    Undo_GeneralStart( opName );

    int v22 = 0;
    if ( v1 > 0 )
    {
        for ( int v3 = 0; v22 < v1; v3 += 12, ++v22 )
        {
            if ( v22 < 0 || v22 >= SEL_FACE_COUNT() )
                unknown_libname_291();

            selbrush_t *v4  = *(selbrush_t **)((char *)selFace + v3);
            int         v5  = *(int *)         ((char *)selFace + v3 + 8);
            brush_t    *def = v4->def;
            faceVis_s  *v19 = *(faceVis_s **)  ((char *)selFace + v3 + 4);

            Undo_TryAddBrush( def );

            if ( v19 != &SEL_FACES(v4)[v5] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1208, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( v4->version != v4->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1209, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
            if ( v4->faceCount != v4->def->faceCount )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1210, 0, "%s", "selFace.brush->faceCount == selFace.brush->def->faceCount" );

            sub_4768B0( &v4->def->faces[v5], v4->def, size );
            Brush_BuildWindings( v4->def, 1 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            MarkMapModified();
            ++v4->def->version;
            sub_47B940( v4->def );
            Undo_LinkBrush( v4->def );
            sub_477D70( v4, (const float *)world_orient_matrix );
            if ( v19 != &SEL_FACES( v4 )[v5] )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1219, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
            if ( v4->version != v4->def->version )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1220, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        }
    }

    for ( selbrush_t *v13 = selected_brushes.next;
          v13 != &selected_brushes;
          v13 = v13->next )
    {
        entity_s_def *def = (entity_s_def *)v13->owner->def;
        if ( !*(int *)&def->eclass->fixedsize )
        {
            Undo_TryAddBrush( v13->def );
            sub_477080( v13->def, size );
            Undo_LinkBrush( v13->def );
        }
    }

    Undo_End();
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48FDC0  Select_Scale
// ═════════════════════════════════════════════════════════════════════════════
void Select_Scale( float a1, float a2, float a3 )
{
    float mid[3];
    Select_GetMid( mid );

    selbrush_t *v3 = selected_brushes.next;
    if ( v3 == &selected_brushes )
        return;

    do
    {
        patch_t *patch = (patch_t *)v3->patch;
        if ( patch )
        {
            float scale[3] = { a1, a2, a3 };
            if ( patch->def != v3->def->patch )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        1351, 0, "%s", "b->patch->def == b->def->patch" );   // IDA 0x48fdc0
            Patch_Scale( v3->def->patch, mid, scale, 1 );
        }
        else
        {
            if ( v3->faceCount )
            {
                for ( int fi = 0; fi < v3->faceCount; ++fi )
                {
                    float *planepts = &v3->def->faces[fi].planepts[0][0];
                    for ( int vi = 0; vi < 3; ++vi )
                    {
                        float *pt = planepts + vi * 3;
                        pt[0] = ( pt[0] - mid[0] ) * a1 + mid[0];
                        pt[1] = ( pt[1] - mid[1] ) * a2 + mid[1];
                        pt[2] = ( pt[2] - mid[2] ) * a3 + mid[2];
                    }
                }
            }
            Brush_BuildWindings( v3->def, 0 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            MarkMapModified();
            ++v3->def->version;
        }
        v3 = v3->next;
    }
    while ( v3 != &selected_brushes );
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48FB10  Select_GetBounds - AABB enclosing every selected brush def.  Init to the
//  editor's +/-131072 sentinel box, then union each brush's [mins,maxs].  The IDA calls
//  vec3_clamp per brush; the union is inlined here (behaviour-identical).
// ═════════════════════════════════════════════════════════════════════════════
void Select_GetBounds( float *mins, float *maxs )
{
    mins[0] = mins[1] = mins[2] =  131072.0f;
    maxs[0] = maxs[1] = maxs[2] = -131072.0f;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        const float *bmin = b->def->mins;
        const float *bmax = b->def->maxs;
        for ( int i = 0; i < 3; ++i )
        {
            if ( bmin[i] < mins[i] ) mins[i] = bmin[i];
            if ( bmax[i] > maxs[i] ) maxs[i] = bmax[i];
        }
    }
}

// ─── Select_GetTrueMid (0x48FC20) — centre of the selection bounding box ──────────
// Used by CXYWnd::SetRotateMode to seat the mouse-rotation pivot (g_vRotateOrigin).
void Select_GetTrueMid( float *center )
{
    float mins[3], maxs[3];
    Select_GetBounds( mins, maxs );
    center[0] = ( maxs[0] + mins[0] ) * 0.5f;
    center[1] = ( maxs[1] + mins[1] ) * 0.5f;
    center[2] = ( maxs[2] + mins[2] ) * 0.5f;
}

// ─── sub_48FB70 (0x48FB70) — bounds of the current selection for Texture_Fit (a4!=0,
//     "fit texture across the whole selection") ─────────────────────────────────────
// If individual FACES are selected, expand [mins,maxs] (already seeded +/-131072 by the
// caller) by every selected face's winding points; otherwise fall back to the selected
// BRUSHES' bbox (Select_GetBounds).  a4!=0 is unreachable from OnFit / Brush_FitTexture
// today.  BINARY QUIRK (disasm 0x48fbb1): numpoints is read from the brush's FACE 0
// winding while the points iterated belong to the selected face (selFace[i].index) -
// transcribed verbatim, with null guards the binary lacks.
void sub_48FB70( int minsOut, int maxsOut )
{
    float *mins = (float *)(intptr_t)minsOut;
    float *maxs = (float *)(intptr_t)maxsOut;

    const int count = SEL_FACE_COUNT();
    if ( !count )
    {
        Select_GetBounds( mins, maxs );
        return;
    }

    for ( int i = 0; i < count; ++i )
    {
        face_t *faces = selFace[i].brush->def->faces;
        if ( !faces[0].w )
            continue;
        const int numpoints = faces[0].w->numpoints;             // face 0's count (IDB quirk)
        if ( !numpoints )
            continue;
        winding_t *w = faces[selFace[i].index].w;                // selected face's winding
        if ( !w )
            continue;
        for ( int n = 0; n < numpoints; ++n )
        {
            const float *p = w->p[n];
            if ( p[0] < mins[0] ) mins[0] = p[0];
            if ( p[0] > maxs[0] ) maxs[0] = p[0];
            if ( p[1] < mins[1] ) mins[1] = p[1];
            if ( p[1] > maxs[1] ) maxs[1] = p[1];
            if ( p[2] < mins[2] ) mins[2] = p[2];
            if ( p[2] > maxs[2] ) maxs[2] = p[2];
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48E9C0  Select_Move - move every selected brush by `delta`, then refresh the status
//  bar with the new selection origin.  Called by MoveSelection (drag.cpp) and XY_MouseDown.
//  The IDA builds an MFC CString for the status text; the port routes the same values to
//  MainFrm_SetStatusText (in the binary str_set is already a no-op stub here).
// ═════════════════════════════════════════════════════════════════════════════
extern entity_s *Brush_Move( const float *move, brush_t *def, char snap );  // brush.cpp (0x47ba40)
extern void      MainFrm_SetStatusText( int pane, const char *text );       // engine_stubs

void Select_Move( const float *delta, char bSnap )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        Brush_Move( delta, b->def, bSnap );

    float bmins[3], bmaxs[3];
    Select_GetBounds( bmins, bmaxs );
    char buf[128];
    // IDA 0x48ea59-74: the VA reads X from the mins buffer but Y/Z from the maxs buffer
    // (fld var_4C=mins[0], var_54=maxs[1], var_50=maxs[2]) - a mixed-corner readout in the
    // original CoD3 source, reproduced verbatim.
    _snprintf( buf, sizeof( buf ), "Origin X:: %.1f  Y:: %.1f  Z:: %.1f",
               bmins[0], bmaxs[1], bmaxs[2] );
    MainFrm_SetStatusText( 2, buf );
}

// ═════════════════════════════════════════════════════════════════════════════
//  SELECTION TRANSFORMS - rotate / flip / arbitrary-matrix apply.
//  The Flip/Rotate menu commands (CMainFrame::OnBrushFlipx/y/z + OnBrushRotatex/y/z) and
//  the rotate-mode keyboard nudge (NudgeSelection) all build a rotation/mirror matrix into
//  a `float rot_around[4][3]` ORIENTATION block - row 0 = the pivot origin (Select_GetMid),
//  rows 1..3 = the 3x3 axis matrix - then push every selected brush through
//  Select_ApplyMatrix_SelectedBrushes -> Select_ApplyMatrix, which transforms each face's
//  3 planepts about the pivot and rebuilds windings.
//  The matrix layout IS an orientation_t: the IDB's Select_FlipAxis/Select_RotateAxis
//  declare `float v2[3]` + `float v3[9]` at CONTIGUOUS stack offsets and pass only &v2, so
//  they are declared here as one `float rot_around[4][3]`.  Select_ApplyMatrix's `mid` arg
//  IS that block; it casts to orientation_t* and calls OrientationPosToWorldPos.
//  KISAK: the texture/lightmap-lock reproject (sub_470570/sub_4706F0) is gated per layer
//  on MaterialDef_04(layer) > 0, which the degenerate materialdef shim reports as 0, so it
//  no-ops; the geometry plane rebuild it would do is redundant with the final
//  Brush_BuildWindings.  Same ruling already taken for Brush_Move.
// ═════════════════════════════════════════════════════════════════════════════

// ── deps for the transform core ──────────────────────────────────────────────
// com_math.h supplies MatrixMultiply / AnglesToAxis / AxisToAngles / Vec3RotateTranspose /
// MatrixTranspose with their exact compiled (mat3x3&) signatures - hand-declaring mangles
// wrong.
#include <universal/com_math.h>

// OrientationPosToWorldPos is DEFINED in draw.cpp (q_shared 0x4BA430), not com_math.h.
// Declared as in brush.cpp (orientation_t is visible via qe3.h / fxprimitives.h).
extern void OrientationPosToWorldPos( float *out, const float *localPos, const orientation_t *orient );

extern entity_s *Brush_Move( const float *move, brush_t *def, char snap );  // brush.cpp (0x47ba40)
extern brush_t  *Brush_Clone( brush_t *def );                               // brush.cpp (0x475d20)
extern void      Entity_LinkBrush( brush_t *b, entity_s *world_ent );       // entity.cpp (0x484fc0)
extern selbrush_t *Brush_AddToList( brush_t *def, entity_s *owner );        // brush.cpp (0x475980)
extern void      Select_Brush( selbrush_t *b, char overwrite, char status, char center ); // select.cpp (0x48dcc0)
extern void      Select_Deselect( int bDeselectFaces );                     // select.cpp (0x48E800)
extern char      g_nScaleHow;                                               // drag.cpp (0x23F16DC)
extern float     grid_sizes[];                                              // engine_stubs (0x6DDE5C)
#include "prefs.h"                                                          // g_PrefsDlg (m_bNoClamp ...)

// 0x48FC70  Select_GetMid — selection-bounds midpoint. Grid-snapped (floor toward the
// grid) unless the NoClamp pref is set. Used by rotate/flip/scale as the pivot.
// FLOAT floorf here - contrast sub_47CFD0's DOUBLE floor.
void Select_GetMid( float *mid )
{
    float bmins[3], bmaxs[3];
    Select_GetBounds( bmins, bmaxs );
    if ( g_PrefsDlg->m_bNoClamp )
    {
        mid[0] = ( bmaxs[0] + bmins[0] ) * 0.5f;
        mid[1] = ( bmaxs[1] + bmins[1] ) * 0.5f;
        mid[2] = ( bmaxs[2] + bmins[2] ) * 0.5f;
    }
    else
    {
        for ( int i = 0; i < 3; ++i )
        {
            float gs = grid_sizes[g_qeglobals.d_gridsize];
            // IDB: mid[i] = floorf( ((mins+maxs)*0.5) / gridSize ) * gridSize  (floor-snap)
            mid[i] = floorf( ( bmins[i] + bmaxs[i] ) * 0.5f / gs ) * gs;
        }
    }
}

// 0x4AABC0  SinCosDeg — sin/cos of an angle in DEGREES, with exact 0/90/180/270 cases.
// Negative angles are wrapped +360 first. (IDB sub_4AABC0; com_math.cpp:3864.)
static void SinCosDeg( float deg, float *s, float *c )
{
    // The 2 NULL-ptr asserts are cross-file (com_math.cpp:3864/3865).
    if ( !s )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp", 3864, 0, "%s", "s" );
    if ( !c )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp", 3865, 0, "%s", "c" );
    if ( deg < 0.0f )
        deg += 360.0f;
    if ( deg == 0.0f )        { *c = 1.0f;  *s = 0.0f; }
    else if ( deg == 90.0f )  { *c = 0.0f;  *s = 1.0f; }
    else if ( deg == 180.0f ) { *c = -1.0f; *s = 0.0f; }
    else if ( deg == 270.0f ) { *c = 0.0f;  *s = -1.0f; }
    else
    {
        float rad = deg * 0.01745329238474369f;
        *c = (float)cos( rad );
        *s = (float)sin( rad );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x47CDE0  Select_ApplyMatrix — transform one brush by the orientation block.
//  `mat` is the orientation_t (origin + 3x3). bSnap = grid-snap on rebuild.
//  deg != 0 routes fixed-size (prefab/model) entities to Select_RotateFixedSize.
//  bSwap flips each face's planept winding order (planept[0]<->planept[2]) — used
//  by the mirror (Select_FlipAxis) so reflected faces keep outward-facing normals.
// ═════════════════════════════════════════════════════════════════════════════
extern void Select_RotateFixedSize( selbrush_t *sb, float (*mid_point)[3], const float *rot );

// KISAK SUBSET of 0x47cde0: geometry faithful; the loop bound uses def->faceCount (the
// instance-vs-def adaptation below) and the texture-lock reproject (sub_470570/sub_4706F0)
// is omitted (layer-gated no-op).  ++def->version is the brush_t int16 @0x4E.  Both asserts
// are cross-file (brush.cpp 5598/5608).
void Select_ApplyMatrix( float *mat, selbrush_t *b, int bSnap, float deg, char bSwap )
{
    if ( b->patch )
    {
        // Patch arm: transform the control points by the orientation block (rotate/flip).
        if ( b->def->patch != b->patch->def )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                    5598, 0, "%s", "b->def->patch == b->patch->def" );
        Patch_ApplyMatrix( (const orientation_t *)mat, b->def->patch, (char)bSnap );
        return;
    }

    entity_s_def *ownerDef = (entity_s_def *)b->owner->def;
    if ( ownerDef != (entity_s_def *)b->def->owner )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                5608, 0, "%s", "b->owner->def == b->def->owner" );

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

// 0x48FD10  Select_ApplyMatrix_SelectedBrushes — apply `mat` to every selected brush.
void Select_ApplyMatrix_SelectedBrushes( int bSnap, float *mat, float deg, char bSwap )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        Select_ApplyMatrix( mat, b, bSnap, deg, bSwap );
}

extern float grid_sizes[];   // engine_stubs.cpp (0x6dde5c)

// ═════════════════════════════════════════════════════════════════════════════
//  0x47CFD0  sub_47CFD0 — the drop-to-floor scatter PIVOT: per-axis centre of a
//  brush's bbox. NoClamp → the raw midpoint; otherwise the midpoint snapped DOWN
//  to the grid. Writes mid[0..2]. (OnDropSelected's model scatter rotates the
//  entity about this pivot; the IDA reads mins@def+32, maxs@def+44.)
// ═════════════════════════════════════════════════════════════════════════════
// DOUBLE floor here - contrast Select_GetMid's float floorf.  def@0x14.
static void sub_47CFD0( float *mid, selbrush_t *b )
{
    int      d_gridsize = g_qeglobals.d_gridsize;
    int      noClamp    = g_PrefsDlg->m_bNoClamp;
    brush_t *def        = b->def;
    for ( int k = 0; k < 3; ++k )
    {
        if ( noClamp )
            mid[k] = ( def->maxs[k] - def->mins[k] ) * 0.5f + def->mins[k];
        else
            mid[k] = (float)floor( 0.5 * ( def->maxs[k] + def->mins[k] ) / grid_sizes[d_gridsize] )
                     * grid_sizes[d_gridsize];
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x47CBA0  sub_47CBA0 — rotate one brush `deg` degrees about its bbox pivot on
//  `axis` (0=X/pitch, 1=Y/roll, 2=Z/yaw). Builds the orientation block (pivot +
//  per-axis 3x3) and hands it to Select_ApplyMatrix. deg==0 is a no-op. The binary
//  negates the angle before SinCosDeg + ApplyMatrix. Used by OnDropSelected's
//  random model scatter (3 calls, axes 0/1/2).
// ═════════════════════════════════════════════════════════════════════════════
void sub_47CBA0( selbrush_t *b, int axis, float deg )
{
    if ( deg == 0.0f )
        return;

    float m[12];                 // orientation_t: m[0..2]=pivot, m[3..11]=3x3 (row-major)
    sub_47CFD0( m, b );          // pivot = bbox mid

    float rot = -deg;
    float s, c;
    SinCosDeg( rot, &s, &c );

    m[3] = 1.0f; m[4]  = 0.0f; m[5]  = 0.0f;   // identity 3x3
    m[6] = 0.0f; m[7]  = 1.0f; m[8]  = 0.0f;
    m[9] = 0.0f; m[10] = 0.0f; m[11] = 1.0f;

    switch ( axis )
    {
    case 0:  m[7] = c;  m[10] = -s;  m[8] =  s;  m[11] = c;  break;   // about X
    case 1:  m[3] = c;  m[9]  =  s;  m[5] = -s;  m[11] = c;  break;   // about Y
    case 2:  m[3] = c;  m[6]  = -s;  m[4] =  s;  m[7]  = c;  break;   // about Z
    }

    Select_ApplyMatrix( m, b, 1, rot, 0 );
    g_nUpdateBits = -1;
}

//  0x47CC90  Select_RotateFixedSize — rotate a fixed-size entity about the pivot.
//  Rotates its origin around the pivot (Vec3RotateTranspose by the 3x3) and composes
//  the rotation onto its `angles` key. mid_point = the orientation block (mid_point[0]
//  = pivot origin, mid_point[1] = the 3x3). `rot` is the eclass (unused; kept for sig).
// ═════════════════════════════════════════════════════════════════════════════
// origin@0x68 / owner@0x8 / def@0x14; ++def->version is the int16 @0x4E.
void Select_RotateFixedSize( selbrush_t *sb, float (*mid_point)[3], const float * /*rot*/ )
{
    entity_s_def *def = (entity_s_def *)sb->owner->def;

    float moveDelta[3];
    moveDelta[0] = def->origin[0] - mid_point[0][0];
    moveDelta[1] = def->origin[1] - mid_point[0][1];
    moveDelta[2] = def->origin[2] - mid_point[0][2];
    float distSq = moveDelta[0] * moveDelta[0] + moveDelta[1] * moveDelta[1]
                 + moveDelta[2] * moveDelta[2];
    // mid_point[1..3] is the 3x3 axis of the orientation block (mid_point[0] = pivot).
    const mat3x3 &rotM = *reinterpret_cast<const mat3x3 *>( &mid_point[1] );
    if ( distSq != 0.0f )
    {
        // rotated offset of the origin about the pivot
        float rotated[3];
        Vec3RotateTranspose( moveDelta, rotM, rotated );
        moveDelta[0] = rotated[0] - moveDelta[0];
        moveDelta[1] = rotated[1] - moveDelta[1];
        moveDelta[2] = rotated[2] - moveDelta[2];
        Brush_Move( moveDelta, sb->def, 1 );
    }

    float angles[3];
    if ( !Entity_GetVec3ForKey( def, angles, "angles" ) )
    {
        angles[0] = angles[1] = angles[2] = 0.0f;
    }
    float curAxis[3][3], newAxis[3][3];
    AnglesToAxis( angles, curAxis );
    // IDA 0x47cd62: MatrixMultiply(in1=curAxis, in2=rotM, out=newAxis), i.e. newAxis =
    // curAxis * rotM (in2@eax=mid_point+0Ch=rotM, in1@ecx=curAxis, out@edx).  NOT commutative.
    MatrixMultiply( curAxis, rotM, newAxis );
    AxisToAngles( newAxis, angles );
    char *v = va( "%g %g %g", angles[0], angles[1], angles[2] );
    SetKeyValue( def, "angles", v );

    Brush_BuildWindings( sb->def, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++sb->def->version;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48FF40  Select_RotateAxis — build the rotation orientation block.
//  rot_around[0] = pivot origin (filled by the caller via Select_GetMid).
//  rot_around[1..3] = the 3x3 rotation matrix about `axis` by `deg` degrees.
//  When the lone selection is a fixed-size entity, the rotation axis is re-derived
//  from the entity's local frame + the scale-how flags (so prefabs rotate in their
//  own space); `fixedsize` then enables the orientation re-base at the tail.
// ═════════════════════════════════════════════════════════════════════════════
void Select_RotateAxis( int axis, float deg, float (*rot_around)[4][3] )
{
    selbrush_t *first = selected_brushes.next;
    bool fixedsize = false;
    float localAxis[3][3];   // the entity's own frame, for the fixed-size re-base

    iassert( selected_brushes.next != &selected_brushes );

    // Single fixed-size entity selected → rotate about its own axis.
    if ( first->next == &selected_brushes )
    {
        entity_s_def *def = (entity_s_def *)first->owner->def;
        if ( *(int *)&def->eclass->fixedsize )
        {
            float ang[3];
            if ( !Entity_GetVec3ForKey( def, ang, "angles" ) )
                ang[0] = ang[1] = ang[2] = 0.0f;
            AnglesToAxis( ang, localAxis );
            switch ( g_nScaleHow & 7 )   // g_nScaleHow (0x23F16DC)
            {
                case 6: axis = 0; fixedsize = true; break;
                case 5: axis = 1; fixedsize = true; break;
                case 3: axis = 2; fixedsize = true; break;
                default: break;   // not a single-axis scale-how → ordinary axis rotate
            }
        }
    }

    float s, c;
    SinCosDeg( -deg, &s, &c );

    // Identity-init the 3x3 (rows 1..3 of rot_around).
    float (*m)[3] = &(*rot_around)[1];   // m[0..2] = the 3x3
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;

    switch ( axis )
    {
        case 0:   // rotate about X
            m[1][1] = c;  m[2][1] = -s;
            m[1][2] = s;  m[2][2] = c;
            break;
        case 1:   // rotate about Y
            m[0][0] = c;  m[2][0] = s;
            m[0][2] = -s; m[2][2] = c;
            break;
        case 2:   // rotate about Z
            m[0][0] = c;  m[1][0] = -s;
            m[0][1] = s;  m[1][1] = c;
            break;
    }

    if ( fixedsize )
    {
        // Re-base the rotation into the entity's local frame, matching the binary's
        // LEFT-association + single-temp operand sequence (0x4900ab/be/ce):
        //   result2 = localAxisᵀ;  result = localAxisᵀ · M;  M = (localAxisᵀ · M) · localAxis
        // (algebraically == localAxisᵀ·M·localAxis, but the FP op order must match the binary).
        mat3x3 &M = *reinterpret_cast<mat3x3 *>( m );   // rows 1..3 of rot_around
        float lt[3][3], tmp[3][3];
        MatrixTranspose( localAxis, lt );    // lt  = localAxisᵀ              (result2)
        MatrixMultiply( lt, M, tmp );        // tmp = localAxisᵀ · M          (result)
        MatrixMultiply( tmp, localAxis, M ); // M   = (localAxisᵀ · M) · localAxis
    }
}
// 0x48FF40: assert 1389 -> iassert; no version field.

// ═════════════════════════════════════════════════════════════════════════════
//  0x48FD50  Select_FlipAxis — mirror the selection across the `axis` plane through
//  the pivot. Builds an identity 3x3 with the chosen axis negated, then applies it
//  with bSwap=1 (winding reverse so reflected faces stay outward-facing).
// ═════════════════════════════════════════════════════════════════════════════
// The -1.0 is flt_6F40C4; rot_around[4][3] keeps the binary's v2/v3 stack contiguity.
void Select_FlipAxis( int axis )
{
    float rot_around[4][3];
    Select_GetMid( rot_around[0] );

    // identity 3x3, then negate the chosen axis (a reflection)
    float (*m)[3] = &rot_around[1];
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
    m[axis][axis] = -1.0f;

    Select_ApplyMatrix_SelectedBrushes( 1, rot_around[0], 0.0f, 1 );
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x424F30  DoFlip — Brush→Flip→{X,Y,Z} core (called by CMainFrame::OnBrushFlipx/y/z).
//  Undo-brackets the selection, mirrors it (Select_FlipAxis), and for every selected
//  fixed-size entity ALSO flips its `angles` key by 180° on the mirror axis (so a
//  mirrored prefab/model faces the reflected direction), rebuilding windings.
// ═════════════════════════════════════════════════════════════════════════════
// version bump is the brush_t int16 @0x4E; ToAngles == AngleNormalize360 (180.0 dbl_6F42A8).
void DoFlip( int axis, const char *opName )
{
    Undo_ClearRedo();
    Undo_GeneralStart( opName );
    Undo_AddBrushList( &selected_brushes );
    Select_FlipAxis( axis );

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s_def *def = (entity_s_def *)b->owner->def;
        if ( !*(int *)&def->eclass->fixedsize )
            continue;

        // Read the entity's `angles` epair (default 0 0 0), flip the mirror axis by 180,
        // normalize, write it back. (IDB: sscanf "%f %f %f"; ToAngles = AngleNormalize360.)
        float ang[3] = { 0.0f, 0.0f, 0.0f };
        const char *val = "0 0 0";
        for ( epair_t *e = def->epairs; e; e = e->next )
            if ( _stricmp( e->key, "angles" ) == 0 ) { val = e->value; break; }
        if ( sscanf( val, "%f %f %f", &ang[0], &ang[1], &ang[2] ) != 3 )
            ang[0] = ang[1] = ang[2] = 0.0f;
        ang[axis] = AngleNormalize360( ang[axis] + 180.0f );
        SetKeyValue( def, "angles", va( "%g %g %g", ang[0], ang[1], ang[2] ) );

        Brush_BuildWindings( b->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++b->def->version;
    }

    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48F0D0  Clone_Selection — duplicate every selected brush into its owner entity,
//  then select the duplicates.
//
//  KISAK SUBSET: the binary clones via the OLE clipboard (CXYWnd::Copy ->
//  Entity_WriteSelected_R -> CMemFile -> CXYWnd::Paste -> Map_ImportBuffer with full
//  target/targetname auto-remap), and follows the paste with NudgeSelection(2)+(3).  This
//  is an IN-MEMORY clone with the same observable effect for brush selections: per selected
//  brush, deep-copy the DEF (Brush_Clone), link it into the same owner entity's def-list
//  (Entity_LinkBrush) + display list (Brush_AddToList/_2), rebuild windings, re-select the
//  copies.  The clipboard's entity-with-epairs duplication, the auto-target renumbering and
//  the cosmetic grid offset are NOT reproduced (`a1` = grid size, unused for that reason).
// ═════════════════════════════════════════════════════════════════════════════
void Clone_Selection( float /*a1*/ )
{
    // SNAPSHOT the selection FIRST: Brush_AddToList2 - despite its name - appends the new
    // instance to selected_brushes, the SAME list, so iterating while adding would clone
    // the clones forever.
    selbrush_t *src[4096];
    int nSrc = 0;
    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes && nSrc < 4096;
          b = b->next )
    {
        if ( b->patch )                       // patch clone = pmesh path (deferred)
            continue;
        entity_s_def *ownerDef = (entity_s_def *)b->owner->def;
        if ( *(int *)&ownerDef->eclass->fixedsize )
            continue;                         // fixed-size bbox entities: clipboard path
        src[nSrc++] = b;
    }
    if ( nSrc == 0 )
        return;                               // nothing cloneable (all patches/bboxes)

    // Deselect the originals (they return to active_brushes); the clones become the
    // new selection (matches the binary's paste-selects-the-new-brushes net effect).
    Select_Deselect( 1 );

    for ( int i = 0; i < nSrc; ++i )
    {
        selbrush_t *b = src[i];
        brush_t *cdef = Brush_Clone( b->def );      // owner already = b->def->owner
        // Link the copy into the owner entity's DEF list (sets cdef->owner = entity def).
        Entity_LinkBrush( cdef, (entity_s *)b->def->owner );
        Brush_BuildWindings( cdef, 1 );
        ++cdef->version;

        // Create the display instance (refCount→2) + add it to the selection.
        selbrush_t *inst = Brush_AddToList( cdef, b->owner );
        if ( inst->next || inst->prev )
            Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
        Brush_AddToList2( inst );             // appends to selected_brushes
    }

    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x490A70  Select_ByClass - forward ->next walk; brushFlags@0x34 &2/&0x20 gate.  The
//  non-1:1 `v7 &&` null-guard is dropped so the structure matches the binary
//  (assert-then-deref).
// ═════════════════════════════════════════════════════════════════════════════
void Select_ByClass( const char *a1 )
{
    if ( selected_brushes.next == &selected_brushes )
        return;
    if ( selected_brushes.next->next != &selected_brushes )
        return;

    selbrush_t *brush = selected_brushes.next;
    entity_s *owner = brush->owner;
    if ( owner == world_entity )
        return;

    iassert( brush->owner->def == brush->def->owner );   // 1776

    char *v8 = ValueForKey2( (int)(intptr_t)owner->def, a1 );
    Sys_Printf( "Selecting %s %s\n", a1, v8 );

    Select_Deselect( 1 );

    brush = active_brushes.next;
    if ( brush != &active_brushes )
    {
        do
        {
            selbrush_t *brushNext = brush->next;
            if ( !FilterBrush( brush, 0 ) )
            {
                int bf = brush->brushFlags;
                if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
                {
                    iassert( brush->owner );   // 1793
                    entity_s *v7 = brush->owner;
                    if ( v7 != world_entity )
                    {
                        iassert( brush->owner->def == brush->def->owner );   // 1796
                        if ( Entity_HasEpairMatch( (entity_s *)brush->owner->def, a1, v8 ) )
                        {
                            Brush_RemoveFromList( brush );
                            Select_Brush_2( &selected_brushes, brush );
                        }
                    }
                }
            }
            brush = brushNext;
        }
        while ( brush != &active_brushes );
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x490C00  Select_ByKeyValue  — Selection→Select by Key/Value (menu/cmd 33133).
//
//  The binary constructs CKeyValueSelectDlg (IDD 0xBC, ctor sub_416760) which pre-
//  fills its key/value CString members from the entity-window key/value edit fields,
//  DoModals it, and — if OK — runs the active_brushes match-walk below.  The dialog's
//  two CString members (key/value) and two flags (key_substr/value_substr) drive an
//  8-way matcher dispatch (exact/substring × key/value × present/absent).
//
//  PORT: the dialog is HAND-BUILT (radiant.rc has no IDD template — same as
//  CFindBrushDlg/CMapInfo, win_dlg.cpp).  The deterministic match-walk is factored
//  into Select_ByKeyValue_Core so it can be driven without a message pump; the public
//  Select_ByKeyValue() opens the modeless popup whose OK button calls the core.
//
//  The IDB reads the dialog members via raw esp slots (`key`/`value` at ebp-20h/-24h =
//  this+120 / this+116; key_substr/value_substr at +124/+128); transcribed as named members.
//  The active-list walk captures `saved=b->next` BEFORE relinking, then advances b=saved.
// ═════════════════════════════════════════════════════════════════════════════

// The eight epair matchers (entity.cpp).  HasKeyValuePair / Entity_HasEpairMatch are
// already extern'd above; the six substring variants are declared here.
extern bool Entity_HasKeySubstr( entity_s_def *e, const char *key );               // sub_483900
extern bool Entity_HasValue( entity_s_def *e, const char *val );                   // sub_483AC0
extern bool Entity_HasValueSubstr( entity_s_def *e, const char *val );             // sub_483B10
extern bool Entity_HasKeySubstrValue( entity_s_def *e, const char *key, const char *val );       // sub_4839C0
extern bool Entity_HasKeyValueSubstr( entity_s_def *e, const char *key, const char *val );       // sub_483A20
extern bool Entity_HasKeySubstrValueSubstr( entity_s_def *e, const char *key, const char *val ); // sub_483A80

// Win_GetEntityKeyValueFields (win_ent.cpp) — the entity-window key/value edit text.
extern void Win_GetEntityKeyValueFields( char *keyOut, char *valueOut );

// ── the deterministic match-walk (the body of 0x490C00 after DoModal returns OK) ──
void Select_ByKeyValue_Core( const char *key, const char *value, bool keySubstr, bool valueSubstr )
{
    const bool hasKey = ( key   && key[0]   != 0 );   // IDB: *(key  -12) != 0  (CString length)
    const bool hasVal = ( value && value[0] != 0 );   // IDB: *(value-12) != 0

    if ( hasKey )
    {
        if ( !hasVal )
        {
            if ( keySubstr )
                Sys_Printf( "Selecting entities that have a substring key of '%s'\n", key );
            else
                Sys_Printf( "Selecting entities that have the key '%s'\n", key );
        }
        else if ( keySubstr )
        {
            if ( valueSubstr )
                Sys_Printf( "Selecting entities that have a substring key of '%s' with a substring value of '%s'\n", key, value );
            else
                Sys_Printf( "Selecting entities that have a substring key of '%s' with the value '%s'\n", key, value );
        }
        else if ( valueSubstr )
            Sys_Printf( "Selecting entities that have the key '%s' with a substring value of '%s'\n", key, value );
        else
            Sys_Printf( "Selecting entities that have the key '%s' with the value '%s'\n", key, value );
    }
    else
    {
        if ( !hasVal )
            return;                                   // nothing entered → no-op (IDB LABEL_2)
        if ( valueSubstr )
            Sys_Printf( "Selecting entities that have a key substring value of '%s'\n", value );
        else
            Sys_Printf( "Selecting entities that have a key value of '%s'\n", value );
    }

    Select_Deselect( 1 );

    selbrush_t *b = active_brushes.next;
    while ( b != &active_brushes )
    {
        selbrush_t *saved = b->next;                  // IDB &b->next->prev (prev@0) captured pre-relink
        if ( !FilterBrush( b, 0 )
          && ( b->brushFlags & 2 ) == 0
          && ( b->brushFlags & 0x20 ) == 0 )
        {
            entity_s *owner = b->owner;
            if ( owner != world_entity )
            {
                if ( (void *)owner->def != (void *)b->def->owner )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1882, 0,
                            "%s", "brush->owner->def == brush->def->owner" );

                entity_s_def *def = (entity_s_def *)owner->def;
                bool match;
                if ( hasKey )
                {
                    if ( hasVal )
                    {
                        if ( valueSubstr )
                        {
                            if ( keySubstr )
                                match = Entity_HasKeySubstrValueSubstr( def, key, value );  // sub_483A80
                            else
                                match = Entity_HasKeyValueSubstr( def, key, value );        // sub_483A20
                        }
                        else if ( keySubstr )
                            match = Entity_HasKeySubstrValue( def, key, value );            // sub_4839C0
                        else
                            match = Entity_HasEpairMatch( def, key, value );               // exact
                    }
                    else if ( keySubstr )
                        match = Entity_HasKeySubstr( def, key );                            // sub_483900
                    else
                        match = HasKeyValuePair( def, key );
                }
                else if ( valueSubstr )
                    match = Entity_HasValueSubstr( def, value );                            // sub_483B10
                else
                    match = Entity_HasValue( def, value );                                  // sub_483AC0

                if ( match )
                {
                    Brush_RemoveFromList( b );
                    Select_Brush_2( &selected_brushes, b );   // Brush_AddToList2 into selected list
                }
            }
        }
        b = saved;
    }

    g_nUpdateBits = -1;
}

// ── CKeyValueSelectDlg — hand-built modeless popup (the win_dlg.cpp pattern) ──────
class CKeyValueSelectDlg : public CWnd
{
public:
    CKeyValueSelectDlg() {}
    afx_msg int  OnCreate( LPCREATESTRUCT lpcs );
    afx_msg void OnOk2();
    afx_msg void OnCancel2();
    afx_msg void OnClose();
    virtual void PostNcDestroy() { CWnd::PostNcDestroy(); }
    DECLARE_MESSAGE_MAP()
};

enum
{
    IDC_KVS_OK = 5400,
    IDC_KVS_CANCEL,
    IDC_KVS_KEY,
    IDC_KVS_VALUE,
    IDC_KVS_KEYSUB,
    IDC_KVS_VALSUB,
};

static CKeyValueSelectDlg *g_dlgKeyValue = nullptr;
static HWND s_kvsKey    = nullptr;
static HWND s_kvsValue  = nullptr;
static HWND s_kvsKeySub = nullptr;
static HWND s_kvsValSub = nullptr;
static HFONT s_kvsFont  = nullptr;

BEGIN_MESSAGE_MAP( CKeyValueSelectDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_KVS_OK,     &CKeyValueSelectDlg::OnOk2 )
    ON_BN_CLICKED( IDC_KVS_CANCEL, &CKeyValueSelectDlg::OnCancel2 )
END_MESSAGE_MAP()

static HWND KVS_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                           int x, int y, int w, int h, const char *text = nullptr )
{
    HWND wnd = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                                x, y, w, h, parent, (HMENU)(INT_PTR)id,
                                AfxGetInstanceHandle(), NULL );
    if ( wnd && s_kvsFont )
        SendMessageA( wnd, WM_SETFONT, (WPARAM)s_kvsFont, 0 );
    return wnd;
}

int CKeyValueSelectDlg::OnCreate( LPCREATESTRUCT lpcs )
{
    if ( CWnd::OnCreate( lpcs ) == -1 )
        return -1;
    if ( !s_kvsFont )
        s_kvsFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 12, lblW = 56, inW = 180, rowH = 26;
    int ix = M + lblW + 6;
    int y  = M;

    KVS_MakeChild( self, "static", SS_LEFT, 0, M, y + 4, lblW, 16, "Key:" );
    s_kvsKey   = KVS_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_KVS_KEY, ix, y, inW, 22 );
    y += rowH;
    s_kvsKeySub = KVS_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_KVS_KEYSUB, ix, y, inW, 18, "Key is a substring" );
    y += rowH;
    KVS_MakeChild( self, "static", SS_LEFT, 0, M, y + 4, lblW, 16, "Value:" );
    s_kvsValue = KVS_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_KVS_VALUE, ix, y, inW, 22 );
    y += rowH;
    s_kvsValSub = KVS_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_KVS_VALSUB, ix, y, inW, 18, "Value is a substring" );
    y += rowH + 8;

    int btnW = 80, btnGap = 12;
    KVS_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_KVS_OK,     ix,                 y, btnW, 24, "OK" );
    KVS_MakeChild( self, "button", BS_PUSHBUTTON    | WS_TABSTOP, IDC_KVS_CANCEL, ix + btnW + btnGap, y, btnW, 24, "Cancel" );

    // Pre-fill key/value from the entity window's key/value edit fields (ctor sub_416760).
    char key[0x1000] = { 0 }, value[0x1000] = { 0 };
    Win_GetEntityKeyValueFields( key, value );
    if ( s_kvsKey )   ::SetWindowTextA( s_kvsKey, key );
    if ( s_kvsValue ) ::SetWindowTextA( s_kvsValue, value );
    if ( s_kvsKey )
        ::SetFocus( s_kvsKey );
    return 0;
}

void CKeyValueSelectDlg::OnOk2()      // DoModal returns 1 → run the match-walk
{
    char key[0x1000] = { 0 }, value[0x1000] = { 0 };
    if ( s_kvsKey )   ::GetWindowTextA( s_kvsKey,   key,   sizeof( key )   - 1 );
    if ( s_kvsValue ) ::GetWindowTextA( s_kvsValue, value, sizeof( value ) - 1 );
    bool keySub = s_kvsKeySub && ::SendMessageA( s_kvsKeySub, BM_GETCHECK, 0, 0 ) == BST_CHECKED;
    bool valSub = s_kvsValSub && ::SendMessageA( s_kvsValSub, BM_GETCHECK, 0, 0 ) == BST_CHECKED;
    Select_ByKeyValue_Core( key, value, keySub, valSub );
    ShowWindow( SW_HIDE );      // binary EndDialog(,1); modeless → hide
    g_nUpdateBits |= 1;
}

void CKeyValueSelectDlg::OnCancel2() { ShowWindow( SW_HIDE ); }
void CKeyValueSelectDlg::OnClose()   { ShowWindow( SW_HIDE ); }

void Select_ByKeyValue()
{
    if ( g_dlgKeyValue && ::IsWindow( g_dlgKeyValue->GetSafeHwnd() ) )
    {
        // Re-seed from the current entity-window fields, then re-show.
        char key[0x1000] = { 0 }, value[0x1000] = { 0 };
        Win_GetEntityKeyValueFields( key, value );
        if ( s_kvsKey )   ::SetWindowTextA( s_kvsKey, key );
        if ( s_kvsValue ) ::SetWindowTextA( s_kvsValue, value );
        g_dlgKeyValue->ShowWindow( SW_SHOW );
        g_dlgKeyValue->SetForegroundWindow();
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgKeyValue = new CKeyValueSelectDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 260, py = 180;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 180;  py = r.top + 130;
    }
    CRect rc( px, py, px + 280, py + 180 );
    if ( !g_dlgKeyValue->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "Select by Key/Value", style, rc, parent, 0 ) )
    {
        delete g_dlgKeyValue;
        g_dlgKeyValue = nullptr;
        return;
    }
    g_nUpdateBits |= 1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x490EC0  Select_Connected  (1407 bytes)
//  Walks target/targetname and script_linkTo/script_linkName epairs.
// ═════════════════════════════════════════════════════════════════════════════
void Select_Connected()
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    ResetSelectMode();

    int v37[1026];  // [0..count-1]=values, [count]=-1, [1024]=count; byte +0x1004=overflow flag (Map_ParseLinkList)

    char v30;
    do
    {
        v30 = 0;
        selbrush_t *brush = selected_brushes.next;
        if ( brush == &selected_brushes )
            break;

        do
        {
            entity_s *owner = brush->owner;
            if ( owner != world_entity )
            {
                iassert( brush->def->owner == brush->owner->def );   // 1969

                entity_s_def *v36 = (entity_s_def *)brush->owner->def;

                // 1. targetname → select entities that target this one.
                if ( HasKeyValuePair( v36, "targetname" ) )
                {
                    const char *value = zero_str();
                    for ( epair_t *ep = v36->epairs; ep; ep = ep->next )
                    {
                        if ( !_stricmp( ep->key, "targetname" ) )
                        { value = ep->value; break; }
                    }
                    selbrush_t *v5 = active_brushes.next;
                    while ( v5 != &active_brushes )
                    {
                        selbrush_t *v5next = v5->next;
                        if ( v5->owner != world_entity
                             && !FilterBrush( v5, 0 )
                             && (v5->brushFlags & 2) == 0
                             && (v5->brushFlags & 0x20) == 0
                             && Entity_HasEpairMatch( (entity_s *)v5->owner->def, "target", value ) )
                        {
                            Brush_RemoveFromList( v5 );
                            Brush_AddToList2( v5 );
                            v30 = 1;
                        }
                        v5 = v5next;
                    }
                }

                // 2. script_linkTo → select entities whose script_linkName is in the list.
                if ( HasKeyValuePair( v36, "script_linkTo" ) )
                {
                    const char *v9 = zero_str();
                    for ( epair_t *ep = v36->epairs; ep; ep = ep->next )
                    {
                        if ( !_stricmp( ep->key, "script_linkTo" ) )
                        { v9 = ep->value; break; }
                    }
                    Map_ParseLinkList( v37, v9 );
                    int count = v37[1024];

                    selbrush_t *v10 = active_brushes.next;
                    while ( v10 != &active_brushes )
                    {
                        selbrush_t *v10next = v10->next;
                        if ( v10->owner != world_entity && !FilterBrush( v10, 0 )
                             && (v10->brushFlags & 2) == 0 && (v10->brushFlags & 0x20) == 0 )
                        {
                            const char *v13 = nullptr;
                            for ( epair_t *ep = ((entity_s *)v10->owner->def)->epairs; ep; ep = ep->next )
                            {
                                if ( !_stricmp( ep->key, "script_linkName" ) )
                                { v13 = ep->value; break; }
                            }
                            if ( v13 && *v13 )
                            {
                                int v14 = (int)atol( v13 );
                                for ( int si = 0; si < count; ++si )
                                {
                                    if ( v14 == v37[si] )
                                    {
                                        Brush_RemoveFromList( v10 );
                                        Brush_AddToList2( v10 );
                                        v30 = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        v10 = v10next;
                    }
                }

                // 3. script_linkName → select entities whose script_linkTo contains this id.
                if ( HasKeyValuePair( v36, "script_linkName" ) )
                {
                    const char *v18 = zero_str();
                    for ( epair_t *ep = v36->epairs; ep; ep = ep->next )
                    {
                        if ( !_stricmp( ep->key, "script_linkName" ) )
                        { v18 = ep->value; break; }
                    }
                    int v19 = (int)atol( v18 );

                    selbrush_t *v20 = active_brushes.next;
                    while ( v20 != &active_brushes )
                    {
                        selbrush_t *v20next = v20->next;
                        if ( v20->owner != world_entity && !FilterBrush( v20, 0 )
                             && (v20->brushFlags & 2) == 0 && (v20->brushFlags & 0x20) == 0 )
                        {
                            const char *v24 = nullptr;
                            for ( epair_t *ep = ((entity_s *)v20->owner->def)->epairs; ep; ep = ep->next )
                            {
                                if ( !_stricmp( ep->key, "script_linkTo" ) )
                                { v24 = ep->value; break; }
                            }
                            if ( v24 && *v24 )
                            {
                                Map_ParseLinkList( v37, v24 );
                                int count2 = v37[1024];
                                for ( int si = 0; si < count2; ++si )
                                {
                                    if ( v19 == v37[si] )
                                    {
                                        Brush_RemoveFromList( v20 );
                                        Brush_AddToList2( v20 );
                                        v30 = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        v20 = v20next;
                    }
                }

                // 4. target → select entities with matching targetname.
                if ( HasKeyValuePair( v36, "target" ) )
                {
                    const char *v34 = zero_str();
                    for ( epair_t *ep = v36->epairs; ep; ep = ep->next )
                    {
                        if ( !_stricmp( ep->key, "target" ) )
                        { v34 = ep->value; break; }
                    }
                    selbrush_t *v28 = active_brushes.next;
                    while ( v28 != &active_brushes )
                    {
                        selbrush_t *v28next = v28->next;
                        if ( v28->owner != world_entity && !FilterBrush( v28, 0 )
                             && (v28->brushFlags & 2) == 0 && (v28->brushFlags & 0x20) == 0
                             && Entity_HasEpairMatch( (entity_s *)v28->owner->def, "targetname", v34 ) )
                        {
                            Brush_RemoveFromList( v28 );
                            Brush_AddToList2( v28 );
                        }
                        v28 = v28next;
                    }
                }
            }
            brush = brush->next;
        }
        while ( brush != &selected_brushes );
    }
    while ( v30 );

    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x491440  SelectTargettedEntity  (840 bytes)
// Two 512-stride dedup loops; brushFlags@0x34 gate; restart-from-head on a match.  The
// binary checks targets<16 ONCE at loop-1 exit, AFTER the strcpys - kept there.
// ═════════════════════════════════════════════════════════════════════════════
void SelectTargettedEntity()
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    ResetSelectMode();

    int         targets = 0;
    char        v20[16][512];
    selbrush_t *brush = selected_brushes.next;

    while ( brush != &selected_brushes )
    {
        entity_s *owner = brush->owner;
        if ( owner != world_entity )
        {
            iassert( brush->def->owner == brush->owner->def );   // 2148

            entity_s_def *def = (entity_s_def *)brush->owner->def;
            if ( HasKeyValuePair( def, "target" ) )
            {
                const char *value = zero_str();
                for ( epair_t *ep = def->epairs; ep; ep = ep->next )
                {
                    if ( !_stricmp( ep->key, "target" ) )
                    { value = ep->value; break; }
                }
                int found = 0;
                for ( int k = 0; k < targets; ++k )
                    if ( !strcmp( v20[k], value ) ) { found = 1; break; }
                if ( !found )
                    strcpy( v20[targets++], value );   // unguarded, matches binary (LABEL_23)
            }
        }
        brush = brush->next;
    }
    iassert( targets < 16 );   // 2172 (binary checks once at loop-1 exit, AFTER the strcpys)

    brush = active_brushes.next;
    while ( brush != &active_brushes )
    {
        selbrush_t *brushNext = brush->next;
        if ( brush->owner != world_entity && !FilterBrush( brush, 0 ) )
        {
            int bf = brush->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                iassert( brush->def->owner == brush->owner->def );   // 2189

                entity_s_def *v11 = (entity_s_def *)brush->owner->def;
                if ( HasKeyValuePair( v11, "targetname" ) )
                {
                    const char *v13 = zero_str();
                    for ( epair_t *ep = v11->epairs; ep; ep = ep->next )
                    {
                        if ( !_stricmp( ep->key, "targetname" ) )
                        { v13 = ep->value; break; }
                    }
                    for ( int k = 0; k < targets; ++k )
                    {
                        if ( !strcmp( v20[k], v13 ) )
                        {
                            Brush_RemoveFromList( brush );
                            Brush_AddToList2( brush );
                            // Restart the outer loop (IDA uses goto).
                            brush = active_brushes.next;
                            goto NEXT_ITER;
                        }
                    }
                }
            }
        }
        brush = brushNext;
        NEXT_ITER:;
    }

    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x491790  Select_ChangeBrushType  (253 bytes)
//  The def->contents RMW lives INSIDE the faceCount-gated face loop (IDA 0x4917eb/0x4917f1)
//  and the patch contents write targets i->patch->def->contents (IDA 0x491830).
// ═════════════════════════════════════════════════════════════════════════════
void Select_ChangeBrushType( int a1, int a2 )
{
    int mask = ~a2;
    for ( selbrush_t *i = selected_brushes.next;
          i != &selected_brushes;
          i = i->next )
    {
        for ( int fi = 0; fi < i->faceCount; ++fi )
        {
            i->def->faces[fi].contents &= mask;
            i->def->faces[fi].contents |= a1;
            // def->contents RMW is INSIDE the face loop (IDA 0x4917eb/0x4917f1) — redundant per
            // face (idempotent) but SKIPPED entirely for faceCount==0 (patch) brushes, whose
            // authoritative contents live in patchMesh_t and which the binary leaves untouched here.
            i->def->contents &= mask;
            i->def->contents |= a1;
        }

        patch_t *patch = (patch_t *)i->patch;
        if ( patch )
        {
            if ( patch->def != i->def->patch )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        2237, 0, "%s", "b->patch->def == b->def->patch" );
            patch->def->contents &= mask;   // IDA 0x491830: i->patch->def->contents (instance patch)
            patch->def->contents |= a1;
        }
    }
    for ( selbrush_t *result = selected_brushes.next;
          result != &selected_brushes;
          result = result->next )
    {
        result->xx6 = 0;
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x491890  Select_ChangeBrushToolflags  (200 bytes)
//  The patch flags write targets v2->patch->def->flags (IDA 0x491927, the instance patch).
// ═════════════════════════════════════════════════════════════════════════════
void Select_ChangeBrushToolflags( int a1, int a2 )
{
    int mask = ~a2;
    for ( selbrush_t *v2 = selected_brushes.next;
          v2 != &selected_brushes;
          v2 = v2->next )
    {
        for ( int fi = 0; fi < v2->faceCount; ++fi )
        {
            v2->def->faces[fi].toolflags &= mask;
            v2->def->faces[fi].toolflags |= a1;
        }
        patch_t *patch = (patch_t *)v2->patch;
        if ( patch )
        {
            if ( patch->def != v2->def->patch )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        2265, 0, "%s", "b->patch->def == b->def->patch" );
            patch->def->flags &= mask;   // IDA 0x491927: v2->patch->def->flags (instance patch)
            patch->def->flags |= a1;
        }
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x491F20  Brush_ShiftTexture  (850 bytes)
//  Non-patch: (int)a1 is an _ftol2 TRUNCATE.  Patch: a1*0.001f.  The base shift comes from
//  the current-layer slot mat_texDef.shift[7*curLayer].  ++def->version is the int16 @0x4E;
//  the version asserts compare selbrush@0x24 vs def@0x4E (both 16-bit).
// ═════════════════════════════════════════════════════════════════════════════
void Brush_ShiftTexture( float a1, float a2 )
{
    unsigned v28 = (unsigned)SEL_FACE_COUNT();

    if ( selected_brushes.next == &selected_brushes && !v28 )
        return;

    selbrush_t *v3 = selected_brushes.next;
    if ( v3 != &selected_brushes )
    {
        do
        {
            float v24, v23;
            if ( v3->patch )
            {
                v24 = a1 * 0.001f;
                v23 = a2 * 0.001f;
            }
            else
            {
                v24 = (float)(int)a1;
                v23 = (float)(int)a2;
            }

            for ( int fi = 0; fi < v3->faceCount; ++fi )
            {
                MaterialDef *v8 = &v3->def->faces[fi].mtldef[g_qeglobals.current_edit_layer];
                int curLayer    = LayerMat::GetCurrentLayer( v8 );
                texdef_sub_t *v10 = &v8->mat_texDef + curLayer;
                // IDA 0x491ff8/0x492002: the base shift is read from the SAME (current-layer)
                // texdef_sub_t slot v10, not from sub-layer 0.  texdef_sub_t is 7 floats, so
                // shift[7*N] == (&mat_texDef)[N].shift[0].
                v10->shift[0] = v10->shift[0] + v24;
                v10->shift[1] = v10->shift[1] + v23;
                TexMatToFakeTexCoords( v8, v10 );
            }

            Brush_BuildWindings( v3->def, 1 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            MarkMapModified();
            ++v3->def->version;

            patch_t *patch = v3->patch;
            if ( patch )
            {
                if ( patch->def != v3->def->patch )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                            2460, 0, "%s", "b->patch->def == b->def->patch" );
                Patch_ShiftTexture( patch->def, v24, v23 );   // the patch DEF, not the instance
            }

            v3 = v3->next;
        }
        while ( v3 != &selected_brushes );
    }

    for ( unsigned v21 = 0; v21 < v28; ++v21 )
    {
        int v13 = (int)(v21 * 12);
        if ( (int)v21 < 0 || (int)v21 >= SEL_FACE_COUNT() )
            unknown_libname_291();

        int         v14 = *(int *)         ((char *)selFace + v13 + 8);
        selbrush_t *v15 = *(selbrush_t **)  ((char *)selFace + v13);
        faceVis_s  *v29 = *(faceVis_s **)   ((char *)selFace + v13 + 4);
        int         v25 = v14;

        if ( v29 != &SEL_FACES(v15)[v14] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2470, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v15->version != v15->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2471, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );

        MaterialDef  *v16 = &v15->def->faces[v14].mtldef[g_qeglobals.current_edit_layer];
        int           v17 = LayerMat::GetCurrentLayer( v16 );
        texdef_sub_t *v18 = &v16->mat_texDef + v17;
        v18->shift[0] = v18->shift[0] + a1;
        v18->shift[1] = v18->shift[1] + a2;
        TexMatToFakeTexCoords( v16, v18 );

        Brush_BuildWindings( v15->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v15->def->version;
        sub_477D70( v15, (const float *)world_orient_matrix );

        if ( v29 != &SEL_FACES(v15)[v25] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2482, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v15->version != v15->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2483, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
    }
    g_nUpdateBits |= 1u;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x492280  Brush_FlipTexture  (967 bytes)
//  size[axis] = -size[axis], shift[axis] = 1-shift[axis]; PMESH_37 takes the patch DEF.
// ═════════════════════════════════════════════════════════════════════════════
void Brush_FlipTexture( int axis )
{
    if ( selected_brushes.next == &selected_brushes
         && !SEL_FACE_COUNT() )
        return;

    iassert( axis == 0 || axis == 1 );

    Undo_ClearRedo();
    Undo_GeneralStart( axis ? "flip texture y" : "flip texture x" );
    Undo_AddBrushList( &selected_brushes );

    selbrush_t *v3 = selected_brushes.next;
    while ( v3 != &selected_brushes )
    {
        for ( int fi = 0; fi < v3->faceCount; ++fi )
        {
            MaterialDef  *v4 = &v3->def->faces[fi].mtldef[g_qeglobals.current_edit_layer];
            texdef_sub_t *v5 = &v4->mat_texDef + LayerMat::GetCurrentLayer( v4 );
            v5->size[axis]  = -v5->size[axis];
            v5->shift[axis] = 1.0f - v5->shift[axis];
            TexMatToFakeTexCoords( v4, v5 );
        }
        Brush_BuildWindings( v3->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v3->def->version;

        patch_t *patch = (patch_t *)v3->patch;
        if ( patch )
        {
            // IDA passes the patch DEF (v3->patch->def), NOT the instance; the binary
            // also asserts the instance/def link first (select.cpp:2524).
            if ( patch->def != v3->def->patch )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        2524, 0, "%s", "b->patch->def == b->def->patch" );
            PMESH_37( v3->patch->def, axis );
        }

        v3 = v3->next;
    }

    Undo_EndBrushList( &selected_brushes );

    unsigned v23 = (unsigned)SEL_FACE_COUNT();
    for ( unsigned v21 = 0; v21 < v23; ++v21 )
    {
        int v8 = (int)(v21 * 12);
        if ( (int)v21 < 0 || (int)v21 >= SEL_FACE_COUNT() )
            unknown_libname_291();

        selbrush_t *v9  = *(selbrush_t **)((char *)selFace + v8);
        int         v25 = *(int *)         ((char *)selFace + v8 + 8);
        faceVis_s  *v24 = *(faceVis_s **)  ((char *)selFace + v8 + 4);

        if ( v24 != &SEL_FACES(v9)[v25] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2534, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v9->version != v9->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2535, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );

        Undo_TryAddBrush( v9->def );

        MaterialDef  *v12 = &v9->def->faces[v25].mtldef[g_qeglobals.current_edit_layer];
        texdef_sub_t *v13 = &v12->mat_texDef + LayerMat::GetCurrentLayer( v12 );
        v13->size[axis]  = -v13->size[axis];
        v13->shift[axis] = 1.0f - v13->shift[axis];
        TexMatToFakeTexCoords( v12, v13 );

        Brush_BuildWindings( v9->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v9->def->version;
        Undo_LinkBrush( v9->def );
        sub_477D70( v9, (const float *)world_orient_matrix );

        if ( v24 != &SEL_FACES(v9)[v25] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2549, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v9->version != v9->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2550, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
    }

    Undo_End();
    g_nUpdateBits |= 1u;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x492650  Brush_ScaleTexture  (918 bytes)
//  (float)a1 == (double)a1 for the small int a1 here (no precision divergence);
//  Patch_ScaleTexture takes the patch DEF.
// ═════════════════════════════════════════════════════════════════════════════
void Brush_ScaleTexture( int a1, int a2 )
{
    if ( selected_brushes.next == &selected_brushes
         && !SEL_FACE_COUNT() )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "scale texture" );
    Undo_AddBrushList( &selected_brushes );

    selbrush_t *v2 = selected_brushes.next;
    while ( v2 != &selected_brushes )
    {
        for ( int fi = 0; fi < v2->faceCount; ++fi )
        {
            MaterialDef  *v3    = &v2->def->faces[fi].mtldef[g_qeglobals.current_edit_layer];
            int           curL  = LayerMat::GetCurrentLayer( v3 );
            texdef_sub_t *v5    = &v3->mat_texDef + curL;
            v5->size[0]         = (float)a1 + v5->size[0];
            v5->size[1]         = (float)a2 + v5->size[1];
            TexMatToFakeTexCoords( v3, v5 );
        }
        Brush_BuildWindings( v2->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v2->def->version;

        patch_t *patch = v2->patch;
        if ( patch )
        {
            if ( patch->def != v2->def->patch )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        2589, 0, "%s", "b->patch->def == b->def->patch" );
            Patch_ScaleTexture( patch->def, (float)a1, (float)a2 );   // the patch DEF
        }

        v2 = v2->next;
    }

    Undo_EndBrushList( &selected_brushes );

    unsigned v28 = (unsigned)SEL_FACE_COUNT();
    for ( unsigned v25 = 0; v25 < v28; ++v25 )
    {
        int v8 = (int)(v25 * 12);
        if ( (int)v25 < 0 || (int)v25 >= SEL_FACE_COUNT() )
            unknown_libname_291();

        int         v9  = *(int *)         ((char *)selFace + v8 + 8);
        selbrush_t *v10 = *(selbrush_t **)  ((char *)selFace + v8);
        faceVis_s  *v31 = *(faceVis_s **)   ((char *)selFace + v8 + 4);
        int         v30 = v9;

        if ( v31 != &SEL_FACES(v10)[v9] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2599, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v10->version != v10->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2600, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );

        Undo_TryAddBrush( v10->def );

        MaterialDef  *v13 = &v10->def->faces[v9].mtldef[g_qeglobals.current_edit_layer];
        int           v14 = LayerMat::GetCurrentLayer( v13 );
        texdef_sub_t *v15 = &v13->mat_texDef + v14;
        v15->size[0]      = (float)a1 + v15->size[0];
        v15->size[1]      = (float)a2 + v15->size[1];
        TexMatToFakeTexCoords( v13, v15 );

        Brush_BuildWindings( v10->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v10->def->version;
        Undo_LinkBrush( v10->def );
        sub_477D70( v10, (const float *)world_orient_matrix );

        if ( v31 != &SEL_FACES(v10)[v30] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2613, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v10->version != v10->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2614, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
    }

    Undo_End();
    g_nUpdateBits |= 1u;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x4929F0  Brush_RotateTexture  (1008 bytes)
//  Fistp rounding: (int)(val + 9.313225746154785e-10) % 360.
//  The inline fistp ROUNDS (the 2^-30 bias is preserved) and is then %360 (0xB60B60B7
//  reciprocal divide); Patch_RotateTexture takes the patch DEF.
void Brush_RotateTexture( int a1 )
{
    unsigned v25 = (unsigned)SEL_FACE_COUNT();

    if ( selected_brushes.next == &selected_brushes && !v25 )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "rotate texture" );
    Undo_AddBrushList( &selected_brushes );

    selbrush_t *v2 = selected_brushes.next;
    while ( v2 != &selected_brushes )
    {
        for ( int fi = 0; fi < v2->faceCount; ++fi )
        {
            MaterialDef  *v3    = &v2->def->faces[fi].mtldef[g_qeglobals.current_edit_layer];
            int           curL  = LayerMat::GetCurrentLayer( v3 );
            texdef_sub_t *v5    = &v3->mat_texDef + curL;
            float         v28   = (float)a1 + v5->rotate;
            // Inline-fistp rounding: the 2^-30 bias must survive in DOUBLE.
            v5->rotate = (float)( (int)(v28 + 9.313225746154785e-10) % 360 );
            TexMatToFakeTexCoords( v3, v5 );
        }
        Brush_BuildWindings( v2->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v2->def->version;

        patch_t *patch = v2->patch;
        if ( patch )
        {
            if ( patch->def != v2->def->patch )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                        2652, 0, "%s", "b->patch->def == b->def->patch" );
            Patch_RotateTexture( patch->def, (float)a1 );   // the patch DEF
        }

        v2 = v2->next;
    }

    Undo_EndBrushList( &selected_brushes );

    for ( unsigned rot = 0; rot < v25; ++rot )
    {
        int v8 = (int)(rot * 12);
        if ( (int)rot < 0 || (int)rot >= SEL_FACE_COUNT() )
            unknown_libname_291();

        int         v9  = *(int *)         ((char *)selFace + v8 + 8);
        selbrush_t *v10 = *(selbrush_t **)  ((char *)selFace + v8);
        faceVis_s  *v30 = *(faceVis_s **)   ((char *)selFace + v8 + 4);
        int         v27 = v9;

        if ( v30 != &SEL_FACES(v10)[v9] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2661, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v10->version != v10->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2662, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );

        Undo_TryAddBrush( v10->def );

        MaterialDef  *v13 = &v10->def->faces[v9].mtldef[g_qeglobals.current_edit_layer];
        int           v14 = LayerMat::GetCurrentLayer( v13 );
        texdef_sub_t *v15 = &v13->mat_texDef + v14;
        float         v29 = (float)a1 + v15->rotate;
        // Inline-fistp rounding: the 2^-30 bias must survive in DOUBLE.
        v15->rotate = (float)( (int)(v29 + 9.313225746154785e-10) % 360 );
        TexMatToFakeTexCoords( v13, v15 );

        Brush_BuildWindings( v10->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++v10->def->version;
        Undo_LinkBrush( v10->def );
        sub_477D70( v10, (const float *)world_orient_matrix );

        if ( v30 != &SEL_FACES(v10)[v27] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2675, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( v10->version != v10->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2676, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
    }

    Undo_End();
    g_nUpdateBits |= W_CAMERA;
}

// ─── prefab_s mirror (IDB 0x54; same layout as map.cpp / entity.cpp / mayaexport.cpp).
//     entity_s.prefab (+0x48) points here; the prefab's instanced brush list lives at
//     active_brushlist (+0x0C, tail sentinel) / active_brushlist_next (+0x10, head).
struct prefab_s_select
{
    entity_s    *prev_entity;           // 0x00
    entity_s    *next_entity;           // 0x04
    void        *unk;                   // 0x08
    selbrush_t  *active_brushlist;      // 0x0C   tail sentinel (== &this->active_brushlist)
    selbrush_t  *active_brushlist_next; // 0x10   head (first node)
    char         _pad[0x54 - 0x14];     // 0x14 .. 0x53
};

// ═════════════════════════════════════════════════════════════════════════════
//  0x493210  sub_493210  — recursive prefab/patch texture-name search.
//
//  Walks a1's owning entity's PREFAB brush list (a1->owner->prefab : prefab_s, whose
//  brush sentinel is at prefab+0x0C with the first node at prefab+0x10).  For each
//  unfiltered brush: a patch → compare the patch's per-layer material name; a nested
//  prefab → recurse; otherwise → loop the def faces comparing the per-layer material
//  name.  Returns 1 the moment a name matches a2 (caller selects the whole top-level
//  brush), 0 if nothing in the subtree matches.  Match uses Materialdef_GetName
//  (lyrMtl ? (char*)lyrMtl : radMtl->name) + _stricmp, exactly like the binary inline.
//
//  The binary brackets the body with two empty-CString RAII temporaries that are never
//  read - VC7.1 cleanup scaffolding with no observable effect - so the port omits them.
//  The inline name idiom == Materialdef_GetName; the face loop stride is 232.
// ═════════════════════════════════════════════════════════════════════════════
int sub_493210( int a1, char *a2 )
{
    selbrush_t *pfbBrush = (selbrush_t *)(intptr_t)a1;
    prefab_s_select *prefab = (prefab_s_select *)pfbBrush->owner->prefab;

    selbrush_t *sentinel = (selbrush_t *)&prefab->active_brushlist; // prefab + 0x0C
    selbrush_t *b = prefab->active_brushlist_next;
    if ( b == sentinel )
        return 0;

    while ( 1 )
    {
        if ( !FilterBrush( b, 0 ) )
        {
            patch_t *patch = b->patch;
            if ( patch )
            {
                // patch path
                if ( patch->def != b->def->patch )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                            2799, 0, "%s", "pfb->patch->def == pfb->def->patch" );
                MaterialDef *md =
                    (MaterialDef *)( &b->patch->def->texture + g_qeglobals.current_edit_layer );
                // MtlDef_IsValid + name resolution inlined (matches the binary's inline idiom).
                if ( !md || (md->lyrMtl != 0) + (md->radMtl != 0) != 1 )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                            85, 0, "%s", "MtlDef_IsValid( mtlDef )" );
                const char *name = md->lyrMtl ? (const char *)md->lyrMtl : md->radMtl->name;
                if ( !_stricmp( a2, name ) )
                    return 1;
            }
            else if ( b->owner->prefab )
            {
                // nested prefab → recurse
                if ( sub_493210( (int)(intptr_t)b, a2 ) )
                    return 1;
            }
            else
            {
                // face loop
                brush_t *def = b->def;
                if ( def->faceCount )
                {
                    unsigned int i = 0;
                    while ( 1 )
                    {
                        MaterialDef *md = &def->faces[i].mtldef[g_qeglobals.current_edit_layer];
                        if ( !md || (md->lyrMtl != 0) + (md->radMtl != 0) != 1 )
                            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                                    85, 0, "%s", "MtlDef_IsValid( mtlDef )" );
                        const char *name = md->lyrMtl ? (const char *)md->lyrMtl : md->radMtl->name;
                        if ( !_stricmp( a2, name ) )
                            return 1;            // LABEL_16
                        def = b->def;            // reload (IDA: def = v5->def)
                        if ( ++i >= (unsigned int)def->faceCount )
                            break;               // → next brush (LABEL_33)
                    }
                }
            }
        }
        b = b->next;
        if ( b == sentinel )
            return 0;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x4934F0  Select_ByTexture  — select every active brush/patch whose current-layer
//  material matches the "source" material name.
//
//  Source name: if a face is selected, it's that face's per-layer material
//  (selFace[0].brush->def->faces[index].mtldef[layer]); otherwise the texture window's
//  current-layer template (g_qeglobals.random_texture_stuff[layer].mtl).  Then walk
//  active_brushes: filtered / brushFlags &2 / &0x20 skipped; a patch compares its
//  per-layer material name (match → Brush_RemoveFromList + Select_Brush_2 to selected);
//  a prefab owner (when a1 != 0) recurses via sub_493210 (match → select the whole brush);
//  a normal brush loops its faces (match → Brush_RemoveFromList + Brush_AddToList2).
//
//  Like sub_493210 the binary wraps the body in two unused empty-CString RAII temporaries;
//  the port omits them.  a1 = the "search prefabs" flag (passed to sub_493210).
//  A patch match uses Select_Brush_2(&selected_brushes, ...); a face match uses
//  Brush_AddToList2 - both as in the binary.
// ═════════════════════════════════════════════════════════════════════════════
void Select_ByTexture( int a1 )
{
    MaterialDef *src;

    if ( SEL_FACE_COUNT() )
    {
        if ( SEL_FACE_COUNT() <= 0 )
            unknown_libname_291();          // CRT subscript bounds check on selFace[0]
        int         index = selFace->index;
        selbrush_t *brush = selFace->brush;
        if ( selFace->face != (faceVis_s *)&SEL_FACES( brush )[index] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2842, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( brush->version != brush->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2843, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
        src = &brush->def->faces[index].mtldef[g_qeglobals.current_edit_layer];
    }
    else
    {
        src = &g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].mtl;
    }

    const char *Name = (const char *)Materialdef_GetName( src );
    Select_Deselect( 1 );

    selbrush_t *b = active_brushes.next;
    while ( b != &active_brushes )
    {
        selbrush_t *bNext = b->next;            // p_prev = &b->next->prev (advance = b->next)
        if ( !FilterBrush( b, 0 ) )
        {
            int bf = b->brushFlags;
            if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
            {
                patch_t *patch = b->patch;
                if ( patch )
                {
                    // patch path
                    if ( patch->def != b->def->patch )
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                                2861, 0, "%s", "b->patch->def == b->def->patch" );
                    MaterialDef *md =
                        (MaterialDef *)( &b->patch->def->texture + g_qeglobals.current_edit_layer );
                    const char *nm = (const char *)Materialdef_GetName( md );
                    if ( !_stricmp( Name, nm ) )
                    {
                        Brush_RemoveFromList( b );
                        Select_Brush_2( &selected_brushes, b );
                    }
                }
                else if ( b->owner->prefab && a1 )
                {
                    // prefab owner → recurse; match selects the whole top-level brush
                    if ( sub_493210( (int)(intptr_t)b, (char *)Name ) )
                    {
                        Brush_RemoveFromList( b );
                        Select_Brush_2( &selected_brushes, b );
                    }
                }
                else
                {
                    // face loop
                    brush_t *def = b->def;
                    if ( def->faceCount )
                    {
                        unsigned int i = 0;
                        while ( 1 )
                        {
                            MaterialDef *md = &def->faces[i].mtldef[g_qeglobals.current_edit_layer];
                            if ( !md || (md->lyrMtl != 0) + (md->radMtl != 0) != 1 )
                                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\MaterialDef.cpp",
                                        85, 0, "%s", "MtlDef_IsValid( mtlDef )" );
                            const char *nm = md->lyrMtl ? (const char *)md->lyrMtl : md->radMtl->name;
                            if ( !_stricmp( Name, nm ) )
                            {
                                Brush_RemoveFromList( b );
                                if ( b->next || b->prev )
                                    Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
                                Brush_AddToList2( b );
                                break;           // → next brush (LABEL_38)
                            }
                            if ( ++i >= (unsigned int)b->def->faceCount )
                                break;           // → next brush (LABEL_38)
                        }
                    }
                }
            }
        }
        b = bNext;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x493830  Select_ByClassSimilar  (368 bytes)
//  Selects all brushes sharing the same entity class as any
//  currently-selected brush.  Iterates selected list in reverse (prev chain).
//  The non-1:1 `&& v2->owner` fold is dropped so the _stricmp deref is unconditional.
// ═════════════════════════════════════════════════════════════════════════════
void Select_ByClassSimilar()
{
    // IDA 0x493835: v0 = selected_brushes (IDB +0 == kisak selected_brushes.PREV, the list
    // TAIL), then walks ->prev to the head.  Starting at .next would break after the first
    // node (head->prev == sentinel) and match only the head brush's class.
    selbrush_t *brush = selected_brushes.prev;

    if ( brush == &selected_brushes )
    {
        g_nUpdateBits = -1;
        return;
    }

    // Iterate selected list in reverse (IDA uses prev).
    while ( 1 )
    {
        entity_s *owner = brush->owner;
        if ( owner != world_entity )
        {
            iassert( brush->owner->def == brush->def->owner );   // 2909
            const char *name = ((entity_s_def *)owner->def)->eclass->name;
            selbrush_t *b    = active_brushes.next;

            while ( b != &active_brushes )
            {
                selbrush_t *bnext = b->next;
                if ( !FilterBrush( b, 0 ) )
                {
                    int bf = b->brushFlags;
                    if ( (bf & 2) == 0 && (bf & 0x20) == 0 )
                    {
                        iassert( b->owner );                          // 2923
                        iassert( b->owner->def == b->def->owner );    // 2924
                        if ( !_stricmp( ((entity_s_def *)b->owner->def)->eclass->name, name ) )
                        {
                            Brush_RemoveFromList( b );
                            Brush_AddToList2( b );
                        }
                    }
                }
                b = bnext;
            }
        }

        if ( brush->prev == &selected_brushes )
            break;
        brush = brush->prev;
    }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x4939E0  Brush_FitTexture  (721 bytes)
// ═════════════════════════════════════════════════════════════════════════════
void Brush_FitTexture( float x, float y, int a4 )
{
    int  v3  = SEL_FACE_COUNT();
    int  v20 = v3;

    if ( selected_brushes.next == &selected_brushes && !v3 )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "Fit texture" );
    Undo_AddBrushList( &selected_brushes );

    for ( selbrush_t *i = selected_brushes.next;
          i != &selected_brushes;
          i = i->next )
    {
        sub_47C950( (int)(intptr_t)i->def, y, x );
        Brush_BuildWindings( i->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++i->def->version;
    }

    for ( int v23 = 0; v23 < v20; ++v23 )
    {
        int v6 = v23 * 12;
        if ( v23 < 0 || v23 >= SEL_FACE_COUNT() )
            unknown_libname_291();

        int         v7  = *(int *)         ((char *)selFace + v6 + 8);
        selbrush_t *b   = *(selbrush_t **)  ((char *)selFace + v6);
        faceVis_s  *v19 = *(faceVis_s **)   ((char *)selFace + v6 + 4);
        int         v21 = v7;

        if ( v19 != &SEL_FACES(b)[v7] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2973, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( b->version != b->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2974, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );

        Undo_TryAddBrush( b->def );
        Texture_Fit( (int)(intptr_t)&b->def->faces[v7], y, x, a4 );
        Brush_BuildWindings( b->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++b->def->version;
        Undo_LinkBrush( b->def );
        sub_477D70( b, (const float *)world_orient_matrix );

        if ( v19 != &SEL_FACES(b)[v21] )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2982, 0, "%s", "selFace.face == &selFace.brush->faces[selFace.index]" );
        if ( b->version != b->def->version )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp",
                    2983, 0, "%s", "selFace.brush->version == selFace.brush->def->version" );
    }

    // Undo linkage for the selected list (IDA trailing loop).
    if ( g_lastundo && !g_lastundo->done )
    {
        for ( selbrush_t *k = selected_brushes.next;
              k != &selected_brushes;
              k = k->next )
        {
            k->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *v18 = (entity_s *)(intptr_t)k->def->owner;
            if ( *(int *)&v18->eclass->fixedsize )
                v18->epairEdits = g_lastundo->id;
        }
    }

    Undo_End();
    g_nUpdateBits |= 1u;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x494030  Entity_SetAngles  (302 bytes)
//  a2 = axis index (0=pitch, 1=yaw, 2=roll).
//  sscanf must zero ALL 3 components on a !=3 return; angles[3] is the separate-floats
//  packing trap.
// ═════════════════════════════════════════════════════════════════════════════
void Entity_SetAngles( float a1, int a2 )
{
    Undo_ClearRedo();
    Undo_GeneralStart( "set entity angles" );

    for ( selbrush_t *brush = selected_brushes.next;
          brush != &selected_brushes;
          brush = brush->next )
    {
        iassert( brush->owner->def == brush->def->owner );   // 3149

        entity_s     *owner = brush->owner;
        entity_s_def *def   = (entity_s_def *)owner->def;

        if ( *(int *)&def->eclass->fixedsize )
        {
            Undo_AddEntity_W( (entity_s *)def );

            const char *value = zero_str();
            for ( epair_t *ep = def->epairs; ep; ep = ep->next )
            {
                if ( !_stricmp( ep->key, "angles" ) )
                { value = ep->value; break; }
            }

            // IDA 0x4940ee: zero ALL three only when sscanf != 3 (partial parse must not
            // retain the one/two values it did read). Pre-zeroing then ignoring the return
            // (the prior port) kept stale components on a partial "angles" string.
            float v8, v9, v10;
            if ( sscanf( value, "%f %f %f", &v8, &v9, &v10 ) != 3 )
                v8 = v9 = v10 = 0.0f;
            float angles[3] = { v8, v9, v10 };
            angles[a2] = a1;
            SetKeyValue( def, "angles", va( "%g %g %g", angles[0], angles[1], angles[2] ) );
        }
    }
    Undo_End();
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x494160  Entity_SetAngles_Old  (163 bytes)
//  a1 is a float[3].
// ═════════════════════════════════════════════════════════════════════════════
void Entity_SetAngles_Old( float *a1 )
{
    Undo_ClearRedo();
    Undo_GeneralStart( "set entity angles" );

    for ( selbrush_t *brush = selected_brushes.next;
          brush != &selected_brushes;
          brush = brush->next )
    {
        iassert( brush->owner->def == brush->def->owner );   // 3171
        entity_s_def *def = (entity_s_def *)brush->owner->def;
        if ( *(int *)&def->eclass->fixedsize )
        {
            Undo_AddEntity_W( (entity_s *)def );
            SetKeyValue( def, "angles", va( "%g %g %g", (double)a1[0], (double)a1[1], (double)a1[2] ) );
        }
    }
    Undo_End();
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x494210  SelectNext  (326 bytes)
//  The `&&` short-circuit is dropped so the owner->def deref is unconditional (the binary
//  asserts then derefs).
// ═════════════════════════════════════════════════════════════════════════════
void SelectNext()
{
    if ( g_qeglobals.d_select_count != 1 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have 1 entity selected!" );
        MessageBeep( 0x40u );
        return;
    }

    iassert( g_qeglobals.d_select_order[0] );                 // 3195
    iassert( g_qeglobals.d_select_order[0]->owner );          // 3196
    iassert( g_qeglobals.d_select_order[0]->owner->def );     // 3197

    // d_select_order[0]->owner is the selbrush's entity INSTANCE; ->def (entity_s.def@8)
    // is the DEF the epair lookups read (assert 3197 guards it non-null).
    entity_s_def *def = (entity_s_def *)g_qeglobals.d_select_order[0]->owner->def;
    if ( !HasKeyValuePair( def, "target" ) )
        return;

    char *v1 = ValueForKey2( (int)(intptr_t)def, "target" );
    selbrush_t *b = active_brushes.next;
    while ( b != &active_brushes )
    {
        iassert( b->owner );   // 3205 (binary asserts then derefs UNCONDITIONALLY)
        if ( Entity_HasEpairMatch( (entity_s *)b->owner->def, "targetname", v1 ) )
        {
            Select_Deselect( 1 );
            Select_Brush( b, 1, 1, 0 );
            return;
        }
        b = b->next;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x494360  SelectPrev  (326 bytes)
//  Mirror of SelectNext; the `&&` short-circuit is likewise dropped.
// ═════════════════════════════════════════════════════════════════════════════
void SelectPrev()
{
    if ( g_qeglobals.d_select_count != 1 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have 1 entity selected!" );
        MessageBeep( 0x40u );
        return;
    }

    iassert( g_qeglobals.d_select_order[0] );                 // 3230
    iassert( g_qeglobals.d_select_order[0]->owner );          // 3231
    iassert( g_qeglobals.d_select_order[0]->owner->def );     // 3232

    // d_select_order[0]->owner is the entity INSTANCE; ->def (entity_s.def@8) is the DEF
    // the epair lookups read (assert 3232 guards it non-null).
    entity_s_def *def = (entity_s_def *)g_qeglobals.d_select_order[0]->owner->def;
    if ( !HasKeyValuePair( def, "targetname" ) )
        return;

    char *v1 = ValueForKey2( (int)(intptr_t)def, "targetname" );
    selbrush_t *b = active_brushes.next;
    while ( b != &active_brushes )
    {
        iassert( b->owner );   // 3240 (binary asserts then derefs UNCONDITIONALLY)
        if ( Entity_HasEpairMatch( (entity_s *)b->owner->def, "target", v1 ) )
        {
            Select_Deselect( 1 );
            Select_Brush( b, 1, 1, 0 );
            return;
        }
        b = b->next;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48C530  ConnectEntities_R — link the two selected entities (target/targetname).
//
//  Selection→Connect Entities (and the alt+shift+MMB clone-and-connect tail in
//  CXYWnd::XY_MouseDown).  Requires exactly two selected brushes from two different
//  non-world entities.  The FIRST (d_select_order[0]) gets "target" = name; the SECOND
//  (d_select_order[1]) gets "targetname" = name.  The name is the source's existing
//  "target", else the dest's existing "targetname", else a fresh "auto%i" computed by
//  scanning every entity DEF's "targetname" for the max "auto<N>" suffix.  If the source
//  is a light (eclass classtype bit 0), default exponent/fov_inner/fov_outer epairs are
//  seeded when absent.  Finally the second brush is re-selected (the first deselected
//  unless Prefs→linking_keeps_selection).
//
//  atol(targetname+4) skips the "auto" prefix (4 chars) to read the numeric suffix -
//  transcribed verbatim.  entities list = the entity-DEF sentinel (next@+4);
//  d_select_order[i]->owner->def are the two entity defs.
// ═════════════════════════════════════════════════════════════════════════════
extern entity_s entities;   // 0x23F17A0 — entity-DEF doubly-linked list sentinel (map.cpp)

void ConnectEntities_R()
{
    if ( g_qeglobals.d_select_count != 2 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have two brushes selected." );
        Sys_Printf( "Must have two brushes selected.\n" );
        MessageBeep( 0x40u );
        return;
    }

    entity_s *owner0 = g_qeglobals.d_select_order[0]->owner;
    entity_s *owner1 = g_qeglobals.d_select_order[1]->owner;
    if ( owner0 == world_entity || owner1 == world_entity )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Can't connect to the world." );
        Sys_Printf( "Can't connect to the world.\n" );
        MessageBeep( 0x40u );
        return;
    }

    entity_s_def *dest = (entity_s_def *)owner1->def;   // gets "targetname"
    entity_s_def *src  = (entity_s_def *)owner0->def;   // gets "target"
    if ( src == dest )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Brushes are from same entity." );
        MessageBeep( 0x40u );
        return;
    }

    char name[1028];
    char *existing = ValueForKey2( (int)(intptr_t)src, "target" );
    if ( ( existing && *existing )
      || ( ( existing = ValueForKey2( (int)(intptr_t)dest, "targetname" ) ) != nullptr && *existing ) )
    {
        // Reuse an existing link name (source's target, else dest's targetname).
        strncpy( name, existing, 1024 );
        name[1024] = 0;
    }
    else
    {
        // Generate a fresh "auto<N>" name: N = max existing "auto<num>" suffix + 1.
        int maxNum = 0;
        for ( entity_s *e = entities.next; e != &entities; e = e->next )
        {
            char *tn = ValueForKey2( (int)(intptr_t)e, "targetname" );
            if ( tn && *tn )
            {
                int n = atol( tn + 4 );   // skip the "auto" prefix (verbatim: atol(tn+4))
                if ( n > maxNum )
                    maxNum = n;
            }
        }
        sprintf( name, "auto%i", maxNum + 1 );
    }

    SetKeyValue( src, "target", name );
    if ( ( src->eclass->classtype & 1 ) != 0 )   // light: seed default cone epairs
    {
        if ( !HasKeyValuePair( src, "exponent" ) )
            SetKeyValue( src, "exponent", "1" );
        if ( !HasKeyValuePair( src, "fov_inner" ) )
            SetKeyValue( src, "fov_inner", "60" );
        if ( !HasKeyValuePair( src, "fov_outer" ) )
            SetKeyValue( src, "fov_outer", "90" );
    }
    SetKeyValue( dest, "targetname", name );
    g_nUpdateBits |= 3;

    if ( !g_PrefsDlg->linking_keeps_selection )
        Select_Deselect( 1 );
    Select_Brush( (selbrush_t *)g_qeglobals.d_select_order[1], 1, 1, 0 );
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48EB30  Select_InsertMidpointEntity (IDB sub_48EB30) — the NON-PATCH arm of
//  Patch→Insert→Add Terrain Row/Column (CMainFrame::OnAddTerrainRowColumn 0x42B080,
//  cmd 33153 / Shift+Ctrl+A).  With exactly TWO selected fixed-size entities of the
//  SAME eclass that are already linked target↔targetname, it CLONES the source entity
//  onto the midpoint of the pair and re-links the chain src → clone → dest, leaving the
//  clone selected.  (That is how a terrain/path row is grown one node at a time when no
//  patch is selected; the patch arm is Patch_InsertRemoveFromVertPair.)
//
//  Reproduced verbatim from the disassembly, including the four reject paths
//  (count != 2 / world entity / different eclass / not fixedsize) with their exact
//  status-bar + console strings and MessageBeep(0x40).  `eclass->fixedsize` is read as
//  a DWORD at eclass+8 in the binary (`cmp dword ptr [eax+8], 0`); the port's eclass_t
//  declares it bool@8 with 3 pad bytes, so the bool read is equivalent.
//
//  KISAK: when NEITHER link direction matches, the binary leaves both local brush pointers
//  NULL and then dereferences them (0x48EC5A `mov eax,[v4+8]`) - an original NULL-deref
//  crash.  The port returns instead.
// ═════════════════════════════════════════════════════════════════════════════
extern void Sys_Status( const char *psz );            // win_qe3.cpp 0x499B90

// sub_409F20 — midpoint of two vec3 (also file-local in brush.cpp; the binary inlines it here).
static void Select_BoundsMidpoint( const float *a, const float *b, float *out )
{
    out[0] = ( a[0] + b[0] ) * 0.5f;
    out[1] = ( a[1] + b[1] ) * 0.5f;
    out[2] = 0.5f * ( a[2] + b[2] );
}

void Select_InsertMidpointEntity()
{
    if ( g_qeglobals.d_select_count != 2 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have two brushes selected." );
        Sys_Printf( "Must have two brushes selected.\n" );
        MessageBeep( 0x40u );
        return;
    }

    entity_s *owner0 = g_qeglobals.d_select_order[0]->owner;
    entity_s *owner1 = g_qeglobals.d_select_order[1]->owner;
    if ( owner0 == world_entity || owner1 == world_entity )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Can't connect to the world." );
        Sys_Printf( "Can't connect to the world.\n" );
        MessageBeep( 0x40u );
        return;
    }

    eclass_t *eclass = ( (entity_s_def *)owner0->def )->eclass;
    if ( eclass != ( (entity_s_def *)owner1->def )->eclass )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Brushes are not the same type." );
        Sys_Printf( "Brushes are not the same type.\n" );
        MessageBeep( 0x40u );
        return;
    }
    if ( !eclass->fixedsize )
    {
        Sys_Status( "Brushes must be fixed size entities." );
        Sys_Printf( "Brushes must be fixed size entities.\n" );
        MessageBeep( 0x40u );
        return;
    }

    // Which of the two selected brushes owns the "target" end of the link?
    selbrush_t *first  = selected_brushes.next;
    selbrush_t *second = selected_brushes.next->next;
    selbrush_t *src  = nullptr;   // the entity that gets cloned (link SOURCE)
    selbrush_t *dest = nullptr;   // the far end of the link
    {
        char *tn = ValueForKey2( (int)(intptr_t)second->owner->def, "targetname" );
        if ( !strcmp( ValueForKey2( (int)(intptr_t)first->owner->def, "target" ), tn ) )
        {
            src  = first;
            dest = second;
        }
        else
        {
            char *tg = ValueForKey2( (int)(intptr_t)second->owner->def, "target" );
            if ( !strcmp( ValueForKey2( (int)(intptr_t)first->owner->def, "targetname" ), tg ) )
            {
                dest = first;
                src  = second;
            }
        }
    }
    if ( !src || !dest )
    {
        // See the KISAK note above - the binary walks off a NULL here.
        Sys_Printf( "The two selected entities are not linked to each other.\n" );
        return;
    }

    ResetSelectMode();                                       // idb sub_48CDF0

    const float *origin = ( (entity_s_def *)src->owner->def )->origin;
    float mid[3];
    Select_BoundsMidpoint( origin, ( (entity_s_def *)dest->owner->def )->origin, mid );
    const float srcOrg[3] = { origin[0], origin[1], origin[2] };

    Select_Deselect( 1 );
    Brush_RemoveFromList( src );
    Select_Brush_2( &selected_brushes, src );
    Clone_Selection( 0.0f );

    const float move[3] = { mid[0] - srcOrg[0], mid[1] - srcOrg[1], mid[2] - srcOrg[2] };
    Select_Move( move, 0 );

    selbrush_t *clone = selected_brushes.next;

    Select_Deselect( 1 );
    Select_Brush( src,   0, 0, 0 );
    Select_Brush( clone, 0, 0, 0 );
    ConnectEntities_R();

    Select_Deselect( 1 );
    Select_Brush( clone, 0, 0, 0 );
    Select_Brush( dest,  0, 0, 0 );
    ConnectEntities_R();

    Select_Deselect( 1 );
    Select_Brush( clone, 0, 0, 0 );
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48C3D0  SetViewToEntity — snap the 3D camera onto the single selected entity
//  and aim it at that entity's "target".  Selection→"Set view to entity" (F6).
//
//  Requires exactly one selected brush from a non-world entity.  Reads the entity's
//  "target" key; walks the entity-DEF list for the entity whose "targetname" matches;
//  moves the camera to the source origin and points it along (dest - src) via
//  vectoangles (pitch is negated to the camera's convention).  Faithful to the binary.
//  entities list = entity-DEF sentinel (next@+4); d_select_order[0]->owner->def is the
//  selected entity's def.  All deps real (ValueForKey2/vectoangles/entities list).
// ═════════════════════════════════════════════════════════════════════════════
extern void vectoangles( float *ang, int vecAddr );   // engine_stubs.cpp 0x4A5020

void SetViewToEntity()
{
    if ( g_qeglobals.d_select_count != 1 )
        return;

    entity_s *owner = g_qeglobals.d_select_order[0]->owner;
    if ( owner == world_entity )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Can't connect to the world." );
        Sys_Printf( "Can't connect to the world.\n" );
        MessageBeep( 0x40u );
        return;
    }

    entity_s_def *src = (entity_s_def *)owner->def;
    char *target = ValueForKey2( (int)(intptr_t)src, "target" );
    if ( !target || !*target )
        return;

    for ( entity_s *e = entities.next; e != &entities; e = e->next )
    {
        if ( !strcmp( target, ValueForKey2( (int)(intptr_t)e, "targetname" ) ) )
        {
            CCamWnd *cam = g_pParentWnd->m_pCamWnd;
            cam->camera.origin[0] = src->origin[0];
            cam->camera.origin[1] = src->origin[1];
            cam->camera.origin[2] = src->origin[2];

            float vec[3];
            vec[0] = e->origin[0] - src->origin[0];
            vec[1] = e->origin[1] - src->origin[1];
            vec[2] = e->origin[2] - src->origin[2];

            float ang[3];
            vectoangles( ang, (int)(intptr_t)vec );
            g_nUpdateBits |= 3u;
            cam->camera.angles[0] = ang[0];
            cam->camera.angles[1] = ang[1];
            cam->camera.angles[2] = ang[2];
            cam->camera.angles[0] = cam->camera.angles[0] * -1.0f;
            return;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  HIDE / SHOW workflow (select.cpp lineage).  Each brush node carries a hidden bit
//  (brushFlags & 4) plus a "hide depth" counter (xx5@0x28) so Show-Last-Hidden can
//  peel one hide level at a time.  Faithful to the CoD4Radiant binary.
//    Select_Hide           0x493D50 — hide the SELECTED brushes (deepen depth on all)
//    Select_HideUnselected 0x493DD0 — hide the ACTIVE (unselected) brushes
//    ShowHidden            0x493E50 — clear all hidden bits + depth
//    ShowLastHidden        0x493EA0 — peel one hide level; unhide when depth hits 0
//  The hex-rays pointer-arith on xx5 (`(int)&xx5->prev + 1`, `(int)&v3[-1].xx10 + 3`)
//  is decompiler noise for the plain integer `xx5 + 1` / `xx5 - 1` (prev@0, so
//  `(int)&node->prev == (int)node`; the second is `v3 - 1`).
// ═════════════════════════════════════════════════════════════════════════════
void Select_Hide()
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = b->xx5 + 1;
        }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
        if ( b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = b->xx5 + 1;
        }
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        b->brushFlags |= 4u;
        b->xx5 = 1;
    }
    g_nUpdateBits = -1;
}

void Select_HideUnselected()
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = b->xx5 + 1;
        }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
        if ( b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = b->xx5 + 1;
        }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        b->brushFlags |= 4u;
        b->xx5 = 1;
    }
    g_nUpdateBits = -1;
}

// 0x48EF40  Select_HideUnselected2_unused — "hide by classname" (Shift+Alt+Ctrl+H).
// Same first two depth-deepening passes as Select_HideUnselected, but the third pass only
// hides the ACTIVE brushes whose owning entity has the SAME eclass as the first selected
// brush's.  The IDB name carries "_unused" because no MENU item reaches it — the keyboard
// table does (HideByClassname, cmd 32925).  Verbatim.
void Select_HideUnselected2_unused()
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = b->xx5 + 1;
        }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
        if ( b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = b->xx5 + 1;
        }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        entity_s *mine  = (entity_s *)selected_brushes.next->owner->def;
        entity_s *theirs = (entity_s *)b->owner->def;
        if ( theirs->eclass == mine->eclass && !b->xx5 )
        {
            b->brushFlags |= 4u;
            b->xx5 = 1;
        }
    }
    g_nUpdateBits = -1;
}

void ShowHidden()
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        b->brushFlags &= ~4u;
        b->xx5 = 0;
    }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        b->brushFlags &= ~4u;
        b->xx5 = 0;
    }
    g_nUpdateBits = -1;
}

void ShowLastHidden()
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->xx5 )
        {
            int depth = b->xx5;
            b->xx5 = depth - 1;
            if ( depth == 1 )
                b->brushFlags &= ~4u;
        }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
        if ( b->xx5 )
        {
            int depth = b->xx5;
            b->xx5 = depth - 1;
            if ( depth == 1 )
                b->brushFlags &= ~4u;
        }
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48C7B0  LinkSelected — Selection→Link Selected (33211, Ctrl+Shift+K).
//  Requires exactly two selected brushes from two DIFFERENT non-world entities and
//  Script_Links their DEFs (script_linkname / script_linkto epairs — core in qe3.cpp).
//  Faithful to the binary; deselects (unless Prefs→linking_keeps_selection) and
//  re-selects the second brush.
// ═════════════════════════════════════════════════════════════════════════════
extern void Script_Link( entity_s_def *a, entity_s_def *b );   // qe3.cpp 0x48BF10

void LinkSelected()
{
    if ( g_qeglobals.d_select_count != 2 )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Must have two entities selected." );
        Sys_Printf( "Must have two entities selected.\n" );
        MessageBeep( 0x40u );
        return;
    }

    entity_s *owner0 = g_qeglobals.d_select_order[0]->owner;
    entity_s *owner1 = g_qeglobals.d_select_order[1]->owner;
    if ( owner0 == world_entity || owner1 == world_entity )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Can't connect to the world." );
        Sys_Printf( "Can't connect to the world.\n" );
        MessageBeep( 0x40u );
        return;
    }

    entity_s_def *a = (entity_s_def *)owner0->def;
    entity_s_def *b = (entity_s_def *)owner1->def;
    if ( a == b )
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"Brushes are from same entity." );
        MessageBeep( 0x40u );
        return;
    }

    Script_Link( a, b );
    g_nUpdateBits |= 3u;
    if ( !g_PrefsDlg->linking_keeps_selection )
        Select_Deselect( 1 );
    Select_Brush( (selbrush_t *)g_qeglobals.d_select_order[1], 1, 1, 0 );
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x47A260  Get_DistanceBetweenEnts — Misc→Get Distance (33178, Alt+F1).
//  Walks the selected brushes; between each consecutive pair of non-world entities
//  prints the 2D (XY) distance between their origins.  Faithful to the binary (the
//  distance uses only origin[0]/origin[1]; origin[2] is not read).
// ═════════════════════════════════════════════════════════════════════════════
void Get_DistanceBetweenEnts()
{
    bool first = true;
    float prevX = 0.0f, prevY = 0.0f;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->owner || b->owner == world_entity )
            continue;
        entity_s *e = b->def->owner;
        float x = e->origin[0];
        float y = e->origin[1];
        if ( first )
        {
            first = false;
        }
        else
        {
            float dx = x - prevX;
            float dy = y - prevY;
            float dist = (float)sqrt( (double)( dy * dy + dx * dx ) );
            Sys_Printf( "Distance between entities: %f\n", dist );
        }
        prevX = x;
        prevY = y;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x429570  NudgeSelection — keyboard nudge of the selection (Alt+arrows, dir 0..3).
//  Three modes (checked in order):
//    * g_bRotateMode : accumulate a rotation about the view axis by ±amt, show it in
//      the status bar, and rotate every selected brush (Select_RotateAxis + the whole-
//      selection apply).  Direction 2/3 (right/down) flips the sign; XY view rotates
//      about Z (axis 2), XZ about Y (axis 1, sign flipped), YZ about X (axis 0).
//    * g_bScaleMode  : scale the selection by 1.1 / 0.9 (dir up/right vs down/left),
//      masked per axis by g_nScaleHow (X=1/Y=2/Z=4).
//    * default       : plain translate via CMainFrame::Nudge, axis picked from the
//      active XY view type + the nudge direction.
//  dir enum: 0=Left, 1=Up, 2=Right, 3=Down.  Faithful to the binary; the CString/OLE
//  status machinery is replaced by the established MainFrm_SetStatusText(2, va(...)) sink.
// ═════════════════════════════════════════════════════════════════════════════
extern float g_vRotation[3];        // drag.cpp 0x23F164C
extern float g_vRotateOrigin[3];    // drag.cpp 0x23F1658
extern bool  g_bRotateMode;         // drag.cpp 0x23F16D9
extern bool  g_bScaleMode;          // drag.cpp 0x23F16DA

void NudgeSelection( int dir, CMainFrame *frame, float amt )
{
    if ( g_bRotateMode )
    {
        int axis = 0;
        if ( frame->m_pActiveXY->m_nViewType == 2 )        // XY (top) → about Z
        {
            axis = 2;
        }
        else if ( g_pParentWnd->m_pActiveXY->m_nViewType == 1 )   // XZ (front) → about Y
        {
            axis = 1;
            amt = -amt;
        }
        if ( dir == 2 || dir == 3 )   // right / down flip the sign
            amt = -amt;

        float rotAxisAngle = -amt;
        g_vRotation[axis] += amt;
        MainFrm_SetStatusText( 2, va( "Rotation x:: %.1f  y:: %.1f  z:: %.1f",
                                      g_vRotation[0], g_vRotation[1], g_vRotation[2] ) );
        if ( rotAxisAngle != 0.0f )
        {
            float rot_around[4][3];
            rot_around[0][0] = g_vRotateOrigin[0];
            rot_around[0][1] = g_vRotateOrigin[1];
            rot_around[0][2] = g_vRotateOrigin[2];
            Select_RotateAxis( axis, rotAxisAngle, (float (*)[4][3])rot_around );
            Select_ApplyMatrix_SelectedBrushes( 0, rot_around[0], rotAxisAngle, 0 );
        }
        g_nUpdateBits = -1;
        return;
    }

    if ( g_bScaleMode )
    {
        if ( dir == 0 || dir == 3 )   // left / down shrink
            amt = -amt;
        float f = ( amt <= 0.0f ) ? 0.89999998f : 1.1f;
        float sx = ( g_nScaleHow & 1 ) ? f : 1.0f;
        float sy = ( g_nScaleHow & 2 ) ? f : 1.0f;
        float sz = ( g_nScaleHow & 4 ) ? f : 1.0f;
        Select_Scale( sx, sy, sz );
        g_nUpdateBits = -1;
        return;
    }

    // Plain translate: pick the axis from the view type + nudge direction.
    int nDim;
    int vt = frame->m_pActiveXY->m_nViewType;
    switch ( dir )
    {
    case 0:  nDim = ( vt == 0 );          amt = -amt; break;   // Left
    case 1:  nDim = ( vt != 2 ) + 1;                  break;   // Up
    case 2:  nDim = ( vt == 0 );                      break;   // Right
    default: nDim = ( vt != 2 ) + 1;      amt = -amt; break;   // Down
    }
    frame->Nudge( nDim, amt );
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x485B70  Select_AllByKeyValue — the "same target(name)" command core (36121/36123).
//  Reads the FIRST selected entity that has key `key`, then writes that key's value onto
//  EVERY selected entity's DEF (so a group shares one target / targetname).  Faithful to
//  the binary (which uses the CString status buffer only as a scratch value holder — the
//  net effect is this value-propagate).  Deps HasKeyValuePair/ValueForKey2/SetKeyValue.
// ═════════════════════════════════════════════════════════════════════════════
void Select_AllByKeyValue( const char *key )
{
    const char *value = "";
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s_def *def = (entity_s_def *)b->owner->def;
        if ( HasKeyValuePair( def, key ) )
        {
            char *v = ValueForKey2( (int)(intptr_t)def, key );
            value = ( v && *v ) ? v : "";
            break;
        }
    }
    if ( value[0] )
    {
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
            SetKeyValue( (entity_s_def *)b->owner->def, key, value );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  VERTEX-EDIT MODE — the point/edge handle subsystem (select.cpp lineage).
//  Builds the global d_points/d_edges handle lists from the selected brushes'
//  faces, picks a vertex by ray for dragging, and feeds the drag path
//  (Drag_Setup → SelectVertexByRay → MoveSelection → Brush_MoveVertex).
//  Ported from the CoD4Radiant IDB (port 13343): FindPoint 0x494a10, FindEdge
//  0x494ac0, MakeFace 0x494b30, SetupVertexSelection 0x494bc0,
//  SelectVertexByRay 0x494fd0, AddPlanept 0x477250, SelectFaceEdge 0x494c30,
//  Select_Edge 0x494dc0.
// ═════════════════════════════════════════════════════════════════════════════

extern winding_t *Brush_MakeFaceWinding( face_t *f, brush_t *def );   // brush.cpp 0x471260
extern int        g_windingAlloc;                                    // winding.cpp dword_24CE4FC
extern float      grid_sizes[];                                      // engine_stubs/qe3 (0x6dde5c)

// MAX_POINTS_ON_WINDING is #define'd locally in winding.cpp (1024); mirror it here
// for the MakeFace point-index scratch (a single face winding never exceeds it).
#ifndef MAX_POINTS_ON_WINDING
#define MAX_POINTS_ON_WINDING 1024
#endif

// ─── FindPoint (0x494a10) ────────────────────────────────────────────────────
// The dedup epsilon is compared against a DOUBLE 0.1 (dbl_6F43D0 =
// 0x3FB999999999999A), NOT float 0.1f (== 0.100000001490116, a wider window).
// Return the index of `pt` in g_qeglobals.d_points[], appending it (capped at
// 2047) if not already present (0.1 tolerance per-axis).
static int FindPoint( const float *pt )
{
    for ( int i = 0; i < g_qeglobals.d_numpoints; ++i )
    {
        int axis = 0;
        for ( ; axis < 3; ++axis )
            // IDA 0x494a4c fcomp vs dbl_6F43D0 = DOUBLE 0.1 (0x3FB999999999999A), NOT float 0.1f
            // (=0.100000001490116). Keep the (float) magnitude round (binary fstp dword), compare double.
            if ( (float)fabs( pt[axis] - g_qeglobals.d_points[i][axis] ) > 0.1 )
                break;
        if ( axis == 3 )
            return i;          // already in the list
    }

    int idx = g_qeglobals.d_numpoints;
    g_qeglobals.d_points[idx][0] = pt[0];
    g_qeglobals.d_points[idx][1] = pt[1];
    g_qeglobals.d_points[idx][2] = pt[2];
    if ( g_qeglobals.d_numpoints < 2047 )
        ++g_qeglobals.d_numpoints;
    return g_qeglobals.d_numpoints - 1;
}

// ─── FindEdge (0x494ac0) ─────────────────────────────────────────────────────
// pedge_t stride 16, 511 cap.
// Register the edge (p1,p2) of face `f`. If the REVERSE edge (p1==.p2, p2==.p1 —
// the binary tests d_edges[i].p1==p2 && .p2==p1) already exists, record `f` as its
// second face and return that index; else append a new edge (capped at 511).
static int FindEdge( face_t *f, int p1, int p2 )
{
    for ( int i = 0; i < g_qeglobals.d_numedges; ++i )
    {
        if ( g_qeglobals.d_edges[i].p1 == p2 && g_qeglobals.d_edges[i].p2 == p1 )
        {
            g_qeglobals.d_edges[i].f2 = f;
            return i;
        }
    }

    int idx = g_qeglobals.d_numedges;
    g_qeglobals.d_edges[idx].p1 = p1;
    g_qeglobals.d_edges[idx].p2 = p2;
    g_qeglobals.d_edges[idx].f1 = f;
    if ( g_qeglobals.d_numedges < 511 )
        ++g_qeglobals.d_numedges;
    return g_qeglobals.d_numedges - 1;
}

// ─── MakeFace (0x494b30) ─────────────────────────────────────────────────────
// Rebuild face `f`'s winding (Brush_MakeFaceWinding), then register each corner
// point (FindPoint) and each edge (FindEdge) into the global handle lists. The
// temporary winding is freed afterward (the binary's --g_windingAlloc; free()).
// IDA __usercall (a1@<eax>=def, a2=face); the call Brush_MakeFaceWinding(a2,a1)
// maps to (face,def) per the disasm of the prologue (edi=face, esi=def).
static void MakeFace( brush_t *def, face_t *f )
{
    winding_t *w = Brush_MakeFaceWinding( f, def );
    if ( !w )
        return;

    int ptIdx[MAX_POINTS_ON_WINDING];
    int n = w->numpoints;
    if ( n > MAX_POINTS_ON_WINDING )
        n = MAX_POINTS_ON_WINDING;
    for ( int i = 0; i < n; ++i )
        ptIdx[i] = FindPoint( w->p[i] );
    for ( int i = 0; i < n; ++i )
        FindEdge( f, ptIdx[i], ptIdx[( i + 1 ) % n] );

    --g_windingAlloc;
    free( w );
}

// ─── SetupVertexSelection (0x494bc0) ─────────────────────────────────────────
// (Re)build the point/edge handle lists from every face of every selected brush.
// Called when entering vertex/edge mode and after each drag rebuilds a winding.
void SetupVertexSelection()
{
    g_qeglobals.d_numpoints = 0;
    g_qeglobals.d_numedges  = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        // KISAK: the binary loops b->faceCount (the INSTANCE count at selbrush_t+0x18, a
        // cached copy of def->faceCount set by Brush_BuildFaceVis on the 3D draw), which is
        // 0 right after a load or headless.  The DEF count is authoritative, so iterate
        // that - identical when valid, and also correct before the first camera draw.
        if ( !b->def )
            continue;
        for ( int i = 0; i < b->def->faceCount; ++i )
            MakeFace( b->def, &b->def->faces[i] );
    }
}

// ─── SelectVertexByRay (0x494fd0) ────────────────────────────────────────────
// Pick the d_points[] handle closest to the ray (start,dir) within 8 units and
// push it as the single drag move-point. The dragged vertex's drawVert_t* is the
// d_points[] slot reinterpreted (drawVert_t.xyz is at offset 0), exactly as the
// binary does (12*idx + &d_points). MoveSelection reads d_move_points[0]->xyz.
void SelectVertexByRay( int origin, int dir )
{
    const float *start = (const float *)(intptr_t)origin;
    const float *dirv  = (const float *)(intptr_t)dir;

    float best     = 8.0f;
    int   bestIdx  = -1;

    if ( g_qeglobals.d_numpoints <= 0 )
    {
        Sys_Printf( "Click didn't hit a vertex\n" );
        return;
    }

    for ( int i = 0; i < g_qeglobals.d_numpoints; ++i )
    {
        const float *p = g_qeglobals.d_points[i];
        // project p onto the ray, then measure the perpendicular distance.
        float rel[3] = { p[0] - start[0], p[1] - start[1], p[2] - start[2] };
        float t = dirv[0] * rel[0] + dirv[1] * rel[1] + dirv[2] * rel[2];
        float proj[3] = { start[0] + dirv[0] * t,
                          start[1] + dirv[1] * t,
                          start[2] + dirv[2] * t };
        float d[3] = { p[0] - proj[0], p[1] - proj[1], p[2] - proj[2] };
        float dist = (float)sqrt( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] );
        if ( best > dist )
        {
            best    = dist;
            bestIdx = i;
        }
    }

    if ( bestIdx == -1 )
    {
        Sys_Printf( "Click didn't hit a vertex\n" );
        return;
    }

    Sys_Printf( "hit vertex\n" );
    g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] =
        (drawVert_t *)&g_qeglobals.d_points[bestIdx];
}

// ─── SelectCurvePointByRay (0x495150) ────────────────────────────────────────
// The patch sibling of SelectVertexByRay (the curve-point pick for dragging).
// Called by Drag_Setup when a non-Alt press starts in sel_curvepoint mode.
//   * SHIFT held (buttons & MK_SHIFT/4): ray-pick the nearest control-point handle in
//     g_qeglobals.d_points[] (filled by Patch_EditPatch) within (28 / g_zoomLevel)
//     units, then hand it to Patch_ClickControlPoint (PMESH_11) which queues the
//     matching control point(s) as drag move-points.  Drag_Setup then arms drag_ok
//     and MoveSelection → Patch_UpdateSelected bends the patch.
//   * SHIFT not held (or the ray missed every handle): fall through to the MISS path —
//     switch to sel_area so the press rubber-bands a control-point selection box
//     instead of dragging (the binary's LABEL_10; the mode switch is faithful, the
//     box-point rect-select itself is deferred).
// `origin`/`dir` are the pick-ray vectors (passed as int per the IDB __usercall ebx/edi
// shape); `buttons` is the mouse-button/modifier mask.
extern float g_zoomLevel;                              // 0x25D5A90 (mainfrm.cpp)
extern bool  g_bRotateMode;                            // 0x23F16D9 (drag.cpp)
extern bool  g_bScaleMode;                             // 0x23F16DA (drag.cpp)
extern "C" void Patch_ClickControlPoint_C( const float *cp, char bMultiAppend, char bColRowSelect ); // pmesh.cpp

void SelectCurvePointByRay( int origin, int dir, unsigned int buttons )
{
    const float *start = (const float *)(intptr_t)origin;
    const float *dirv  = (const float *)(intptr_t)dir;

    int bestIdx = -1;

    if ( ( buttons & 4 ) != 0 )         // MK_SHIFT — do the ray-pick
    {
        // Pick radius scales with the zoom (a fixed pixel radius in world units).
        float best = 28.0f;
        if ( g_zoomLevel > 0.0f )
            best = 28.0f / g_zoomLevel;

        for ( int i = 0; i < g_qeglobals.d_numpoints; ++i )
        {
            const float *p = g_qeglobals.d_points[i];
            // project p onto the ray, measure perpendicular distance.
            float rel[3] = { p[0] - start[0], p[1] - start[1], p[2] - start[2] };
            float t = dirv[0] * rel[0] + dirv[1] * rel[1] + dirv[2] * rel[2];
            float proj[3] = { start[0] + dirv[0] * t,
                              start[1] + dirv[1] * t,
                              start[2] + dirv[2] * t };
            float d[3] = { p[0] - proj[0], p[1] - proj[1], p[2] - proj[2] };
            float dist = (float)sqrt( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] );
            if ( best >= dist )
            {
                best    = dist;
                bestIdx = i;
            }
        }

        if ( bestIdx != -1 )
        {
            // hand the clicked handle to PMESH_11: append=(buttons&8), colRow=(buttons&4).
            Patch_ClickControlPoint_C( g_qeglobals.d_points[bestIdx],
                                       (char)( ( buttons & 8 ) != 0 ),
                                       (char)( ( buttons & 4 ) != 0 ) );
            return;
        }
    }

    // ── MISS / no-shift: switch to sel_area (rubber-band point select) ──────────
    if ( !g_bRotateMode && !g_bScaleMode )
    {
        select_t prev = g_qeglobals.d_select_mode;
        g_qeglobals.d_select_mode = sel_area;
        if ( prev == sel_cycle_edge_direction_quad )
            CMainFrame_UpdatePatchToolbarButtons();
        else if ( prev == sel_addpoint )
            sub_43ECB0();
    }
}

// ─── AddPlanept (0x477250) ───────────────────────────────────────────────────
// Append a point POINTER to g_qeglobals.d_move_points[] (cap 4096), de-duped
// against the existing terrain points (d_terrapoints) and the move-points already
// queued. Returns 1 if newly added, 0 if a duplicate / list full. SelectFaceEdge
// uses it to register the two edge-endpoint planepts as the drag move-points; the
// dedup is what stops the SHARED endpoint of the two faces being added twice.
static int AddPlanept( float *pt )
{
    if ( g_qeglobals.d_num_move_points == 4096 )
        return 0;
    for ( int i = 0; i < g_qeglobals.d_numterrapoints; ++i )
        if ( (float *)g_qeglobals.d_terrapoints[i] == pt )
            return 0;
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
        if ( (float *)g_qeglobals.d_move_points[i] == pt )
            return 0;
    g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = (drawVert_t *)pt;
    return 1;
}

// ─── ClipLineToFace (0x477150) ───────────────────────────────────────────────
// Clip the segment (a2 = far point, a3 = near point) to the FRONT side of face f's
// plane. Returns 0 when both endpoints are in front (segment fully outside the
// half-space), 1 otherwise (fully behind/inside, or after clipping the front endpoint
// to the plane). Ported verbatim from IDA 0x477150 (plane.dist read as float — the
// IDB `*(double*)&dist` is the float-load-then-double-compare hex-rays artifact).
static char ClipLineToFace( face_t *f, float *a2, float *a3 )
{
    float v9  = f->plane.normal[1] * a3[1] + a3[0] * f->plane.normal[0] + f->plane.normal[2] * a3[2];
    float v7  = v9 - f->plane.dist;
    float v10 = f->plane.normal[1] * a2[1] + a2[0] * f->plane.normal[0] + f->plane.normal[2] * a2[2];
    float v11 = v10 - f->plane.dist;
    if ( v7 >= 0.0f && v11 >= 0.0f )
        return 0;
    if ( v7 <= 0.0f && v11 <= 0.0f )
        return 1;
    float  v8 = v7 / ( v7 - v11 );
    float *v6 = ( v7 <= 0.0f ) ? a2 : a3;
    v6[0] = ( a2[0] - a3[0] ) * v8 + a3[0];
    v6[1] = ( a2[1] - a3[1] ) * v8 + a3[1];
    v6[2] = v8 * ( a2[2] - a3[2] ) + a3[2];
    return 1;
}

// ─── Brush_SelectFaceForDragging (0x477330) ──────────────────────────────────
// Add the face's 3 planepts to the move-point set, then (recursively) every face in
// the OTHER selected brushes whose planepts are coplanar with this face's plane — so
// dragging a shared wall drags all the brushes that share it.
// KISAK SUBSET of 0x477330: the SHEAR branch (shear set, non-terrain: rebuild each
// adjacent face's winding, find the edge coplanar with the dragged plane, AddPlanept its
// endpoints + insert rotated planepts) is deferred - it is the Ctrl+LMB face-SHEAR
// gesture, not the plain side-stretch.  TERRAIN handling lives in Brush_SideSelect.
void Brush_SelectFaceForDragging( brush_t *a1, int a2, int shear )
{
    if ( *(int *)&a1->owner->eclass->fixedsize )   // fixed-size entity → no side-drag
        return;
    face_t *dragFace = (face_t *)(intptr_t)a2;

    int added = 0;
    added += AddPlanept( dragFace->planepts[0] );
    added += AddPlanept( dragFace->planepts[1] );
    added += AddPlanept( dragFace->planepts[2] );
    if ( !added )
        return;

    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        brush_t *def = i->def;
        if ( def == a1 )
            continue;
        for ( unsigned fi = 0; fi < (unsigned)def->faceCount; ++fi )
        {
            face_t *cf = &def->faces[fi];
            int k = 0;
            do {
                float d = cf->planepts[k][1] * dragFace->plane.normal[1]
                        + cf->planepts[k][0] * dragFace->plane.normal[0]
                        + cf->planepts[k][2] * dragFace->plane.normal[2];
                if ( fabs( d - dragFace->plane.dist ) > 0.1 )   // IDB tolerance v6 = 0.1
                    break;
                ++k;
            } while ( k < 3 );
            if ( k == 3 ) {
                Brush_SelectFaceForDragging( def, (int)(intptr_t)cf, shear );
                break;
            }
        }
    }
    // SHEAR branch (if shear && !terrain) deferred — see header.
    (void)shear;
}

// ─── Brush_SideSelect (0x4777d0) ─────────────────────────────────────────────
// The click missed the brush surface, so grab one or more side planes for dragging.
// For each face: extend the ray, clip it to the brush interior (every OTHER face),
// and if it still exits through this face, either (a) for a numberId==2 brush past
// face index 2, collect every side face's planept vec3's as draggable "terrain"
// points (then RETURN), or (b) hand the face to Brush_SelectFaceForDragging.
// The TERRAIN branch:
//   * a2->numberId==2 (IDB total_size_0x58) AND the exiting face index >= 2 (byte
//     offset >= 0x1D0 = 2*232) triggers the collector.
//   * For each face >= 2 of a non-fixedsize brush, the 3 planept vec3's
//     (face_t.planepts[0..2], cast to terrainVert_t*) are appended to
//     g_qeglobals.d_terrapoints[] (cap 4096), de-duped against the existing
//     terrapoints AND d_move_points (raw pointer compare).
//   * d_select_translate_unk = a2->faces[0].plane.normal (the dragged plane normal;
//     IDB reads faces base + 0xC0 = face[0].plane.normal — the hex-rays
//     brush_faces[8].normal is a plane_t-stride artifact, disasm is +0xC0).
//   The collector RETURNS once it fires (does NOT fall through to
//   Brush_SelectFaceForDragging).  MoveSelection's terrain-drag branch consumes
//   d_terrapoints/d_numterrapoints/d_select_translate_unk.
void Brush_SideSelect( float *trace_start, brush_t *a2, float *trace_dir, int shear )
{
    int facecount = a2->faceCount;
    for ( int v28 = 0; v28 < facecount; ++v28 )
    {
        float v25[3] = { trace_start[0], trace_start[1], trace_start[2] };           // near point
        float v24[3] = { trace_dir[0] * 262144.0f + trace_start[0],                  // far point
                         trace_dir[1] * 262144.0f + trace_start[1],
                         262144.0f * trace_dir[2] + trace_start[2] };
        for ( int v7 = 0; v7 < a2->faceCount; ++v7 )                                 // clip to the brush interior
            if ( v7 != v28 )
                ClipLineToFace( &a2->faces[v7], v24, v25 );
        int v9 = 0;                                                                  // did the near point move?
        for ( ; v9 < 3; ++v9 )
        {
            float d = v25[v9] - trace_start[v9];
            if ( d * d > 0.00000100000011116208 )
                break;
        }
        if ( v9 >= 3 )
            continue;                                                                // ray never reached this face
        if ( !ClipLineToFace( &a2->faces[v28], v24, v25 ) )                          // exits through this face → grab it
        {
            // numberId==2 (terrain-style) brush AND the exiting face is index >= 2:
            // collect every side face's planepts as draggable terrain points, then bail.
            if ( a2->numberId == 2 && (unsigned)v28 >= 2 )                           // v27(byte off) >= 0x1D0 ⇔ v28 >= 2
            {
                if ( (unsigned)a2->faceCount > 2u )                                  // IDB: jbe (facecount<=2) skips collection
                {
                    int d_numterrapoints = g_qeglobals.d_numterrapoints;
                    int d_num_move_points = g_qeglobals.d_num_move_points;
                    for ( int v29 = 2; v29 < a2->faceCount; ++v29 )                  // faces 2..N
                    {
                        // candidate terrain verts = the face's 3 planept vec3's.
                        terrainVert_t *v18 = (terrainVert_t *)&a2->faces[v29];       // == &faces[v29].planepts[0]
                        if ( !*(int *)&a2->owner->eclass->fixedsize )                // skip fixed-size (bbox) brushes
                        {
                            for ( int v19 = 3; v19; --v19 )
                            {
                                if ( d_numterrapoints != 4096 )
                                {
                                    bool dup = false;
                                    for ( int v20 = 0; v20 < d_numterrapoints; ++v20 )
                                        if ( g_qeglobals.d_terrapoints[v20] == v18 ) { dup = true; break; }
                                    if ( !dup )
                                    {
                                        for ( int v21 = 0; v21 < d_num_move_points; ++v21 )
                                            if ( (terrainVert_t *)g_qeglobals.d_move_points[v21] == v18 ) { dup = true; break; }
                                    }
                                    if ( !dup )
                                    {
                                        g_qeglobals.d_terrapoints[d_numterrapoints] = v18;
                                        d_num_move_points = g_qeglobals.d_num_move_points;
                                        d_numterrapoints = ++g_qeglobals.d_numterrapoints;
                                    }
                                }
                                v18 = (terrainVert_t *)((char *)v18 + 12);           // next planept (vec3)
                            }
                        }
                    }
                }
                // d_select_translate_unk = face[0]'s plane normal (faces base + 0xC0).
                g_qeglobals.d_select_translate_unk[0] = a2->faces[0].plane.normal[0];
                g_qeglobals.d_select_translate_unk[1] = a2->faces[0].plane.normal[1];
                g_qeglobals.d_select_translate_unk[2] = a2->faces[0].plane.normal[2];
                return;
            }
            Brush_SelectFaceForDragging( a2, (int)(intptr_t)&a2->faces[v28], shear );
        }
    }
}

// ─── SelectFaceEdge (0x494c30) ───────────────────────────────────────────────
// For face `f` of brush `def`, find the directed edge (p1→p2, indices into
// g_qeglobals.d_points[]) in the face's winding, then REWRITE the face's three
// defining planepts to the three consecutive winding points spanning that edge
// (grid-snapped) and queue the first two (the edge endpoints) as drag move-points
// via AddPlanept. The face plane is preserved (the 3 points are co-planar winding
// verts), so when MoveSelection later translates those planepts + rebuilds the
// windings, the brush reshapes along the dragged edge.
//
// IDA __usercall: a1@<eax>=def, a2@<ebx>=face_t* (also the planepts output buffer,
// since face_t.planepts is at offset 0), a3=p1, a4=p2. The call site is
// Brush_MakeFaceWinding(a2,a1) → (face,def) per the disasm (edi=face, esi=def).
static void SelectFaceEdge( brush_t *def, face_t *f, int p1, int p2 )
{
    winding_t *w = Brush_MakeFaceWinding( f, def );
    if ( !w )
        return;

    int n = w->numpoints;
    // v22[129] in the binary; a single brush-face winding never approaches that.
    int idx[129];
    if ( n > 129 )
        n = 129;
    for ( int i = 0; i < n; ++i )
        idx[i] = FindPoint( w->p[i] );

    // search for the consecutive index pair (p1,p2) in the winding's point list.
    int j = 0;
    if ( n > 0 )
    {
        for ( ; j < n; ++j )
            if ( idx[j] == p1 && idx[( j + 1 ) % n] == p2 )
                break;
    }
    if ( n <= 0 || j >= n )
    {
        Sys_Printf( "SelectFaceEdge: failed\n" );
        --g_windingAlloc;
        free( w );
        return;
    }

    // rewrite the face's three planepts to the three winding points spanning the
    // edge (planepts[2] = the next point, keeping the plane well-defined).
    int i0 = idx[j];
    int i1 = idx[( j + 1 ) % n];
    int i2 = idx[( j + 2 ) % n];
    for ( int k = 0; k < 3; ++k )
    {
        f->planepts[0][k] = g_qeglobals.d_points[i0][k];
        f->planepts[1][k] = g_qeglobals.d_points[i1][k];
        f->planepts[2][k] = g_qeglobals.d_points[i2][k];
    }

    // grid-snap all 9 planept coords: floor(v/grid + 0.5) * grid.
    {
        float grid = grid_sizes[g_qeglobals.d_gridsize];
        float *fp  = &f->planepts[0][0];
        for ( int p = 0; p < 3; ++p )
            for ( int k = 0; k < 3; ++k )
            {
                *fp = (float)( floorf( *fp / grid + 0.5f ) * grid );
                ++fp;
            }
    }

    // queue the two edge endpoints (planepts[0], planepts[1]) as drag move-points.
    AddPlanept( &f->planepts[0][0] );
    AddPlanept( &f->planepts[1][0] );

    --g_windingAlloc;
    free( w );
}

// ─── Select_Edge (0x494dc0) ──────────────────────────────────────────────────
// Midpoint-closest-to-ray pick (best>dist, 8u init); per-brush SelectFaceEdge for both
// edge directions.
// Pick the d_edges[] edge whose midpoint is closest to the ray (within 8 units),
// then for every selected brush register the two faces sharing that edge via
// SelectFaceEdge — once for each direction (f1: p1→p2, f2: p2→p1). The result is
// the edge's two endpoints queued as drag move-points (in each face's planepts).
//
// IDA __usercall: a1@<eax>=dir, a2@<ecx>=origin. Drag_Setup calls
// Select_Edge(trace_dir, trace_start) → (dir, origin).
void Select_Edge( int dir, int origin )
{
    const float *dirv  = (const float *)(intptr_t)dir;
    const float *start = (const float *)(intptr_t)origin;

    float best    = 8.0f;
    int   bestIdx = -1;

    for ( int i = 0; i < g_qeglobals.d_numedges; ++i )
    {
        const pedge_t *e = &g_qeglobals.d_edges[i];
        // edge midpoint.
        float mid[3];
        mid[0] = ( g_qeglobals.d_points[e->p2][0] + g_qeglobals.d_points[e->p1][0] ) * 0.5f;
        mid[1] = ( g_qeglobals.d_points[e->p2][1] + g_qeglobals.d_points[e->p1][1] ) * 0.5f;
        mid[2] = 0.5f * ( g_qeglobals.d_points[e->p2][2] + g_qeglobals.d_points[e->p1][2] );
        // perpendicular distance from the midpoint to the ray.
        float rel[3] = { mid[0] - start[0], mid[1] - start[1], mid[2] - start[2] };
        float t = dirv[1] * rel[1] + dirv[0] * rel[0] + dirv[2] * rel[2];
        float proj[3] = { dirv[0] * t + start[0],
                          dirv[1] * t + start[1],
                          t * dirv[2] + start[2] };
        float d[3] = { mid[0] - proj[0], mid[1] - proj[1], mid[2] - proj[2] };
        float dist = (float)sqrt( d[2] * d[2] + d[1] * d[1] + d[0] * d[0] );
        if ( best > dist )
        {
            best    = dist;
            bestIdx = i;
        }
    }

    if ( bestIdx == -1 )
    {
        Sys_Printf( "Click didn't hit an edge\n" );
        return;
    }

    Sys_Printf( "hit edge\n" );
    const pedge_t *e = &g_qeglobals.d_edges[bestIdx];
    g_qeglobals.d_num_move_points = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        // KISAK: the binary calls SelectFaceEdge unconditionally for BOTH faces with no
        // NULL guard, so it AVs in Brush_MakeFaceWinding(NULL) when an edge has only one
        // face registered - which FindEdge leaves whenever the reverse direction never
        // registered (a face whose winding failed to build).  Such an edge's midpoint IS
        // drawn as a handle, so a click near it can pick it.  Skip the NULL side.
        if ( e->f1 )
            SelectFaceEdge( b->def, e->f1, e->p1, e->p2 );
        if ( e->f2 )
            SelectFaceEdge( b->def, e->f2, e->p2, e->p1 );
    }
}


