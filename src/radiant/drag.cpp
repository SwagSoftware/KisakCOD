#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\drag.cpp - the mouse press/drag/release state machine.
// Ground truth: CoD4Radiant IDA (IW3xRadiant.i64); GtkRadiant 1.6 for naming only.
//   0x47e3a0  Drag_Setup       mouse-down setup (what a press starts)
//   0x47e810  Drag_FaceAlign   copy the texture projection between two selected faces
//   0x463a80  ClampGridSize    grid index -> rotation-snap step
//   0x47e890  Drag_Begin       mouse-down dispatcher (select / drag / texture pick-apply)
//   0x47f0c0  MoveSelection    translate / scale / rotate / bend / vertex / terrain
//   0x47ff30  Drag_MouseMoved  pixel delta -> grid-snapped world move
//   0x4802a0  Drag_MouseUp     marquee box-select + undo close
// The screen->world drag basis (g_drag_xvec/yvec) is AXIALISED to UNIT vectors, so the
// move is computed in PIXEL units and then grid-snapped; m_fScale is applied only in
// rotate/bend.  BOTH xvec components must land in g_drag_xvec[] (0x23f1718..0x23f1720):
// splitting the z into a separate local leaves AxializeVector reading an uninitialised
// component.

#include "stdafx.h"
#include "qe3.h"
#include "prefs.h"       // g_PrefsDlg (prefData_t* — real settings)
#include "mainfrm.h"     // CMainFrame / CXYWnd (m_pActiveXY->m_fScale in rotate/bend path)
#include <math.h>
#include <stdlib.h>

// ─── forward declarations ─────────────────────────────────────────────────────
// engine_stubs / qe3.cpp
extern void    Assert( const char *file, int line, int type, const char *fmt, ... );
extern int     Sys_Printf( const char *fmt, ... );
extern int     g_nUpdateBits;

// undo.cpp
extern void    Undo_ClearRedo();
extern void    Undo_GeneralStart( const char *op );
extern void    Undo_AddBrushList( selbrush_t *list );
extern void    Undo_Start( const char *op );
extern void    Undo_End();
extern void    Undo_EndBrushList( selbrush_t *list );

// select.cpp
extern void    SelectFaceSth( int a1, int a2, int a3 );
extern void    Select_Deselect( int bDeselectFaces );

// engine_stubs.cpp (stubs for Phase-5 functions)
extern void    sub_43ECB0();           // addpoint mode cleanup (FATAL stub)
extern void    SelectVertexByRay( int origin, int dir );
extern void    SelectCurvePointByRay( int origin, int dir, unsigned int buttons );
extern void    Select_Edge( int dir, int origin );
// IDA a2 = (int)&brush->def->faces[idx] — face address as int, NOT a raw index.
extern void    Brush_SelectFaceForDragging( brush_t *def, int facePtr, int shear );
extern void    Brush_SideSelect( float *origin, brush_t *def, float *dir, int control );
extern void    Patch_RedispersePreDrag();   // pmesh.cpp (0x43D7A0; was sub_43D7A0)

// pmesh.cpp / engine_stubs.cpp (Phase-5 patch stubs)
extern void    Patch_TurnEdge( int origin, int dir );
extern void    Patch_UpdateSelected( int move );
extern int     Patch_DragScale( float *delta, void *patchDef, float *move );
extern void    Patch_SelectBendNormal();
extern void    Patch_SelectBendAxis();
extern int     Patch_Paint_Start();
extern int     OnlyPatchesSelected();

// brush.cpp
extern void    Brush_BuildWindings( brush_t *b, int bFull );
extern void    SetupVertexSelection();
extern void    MarkMapModified();
extern int     Brush_MoveVertex( vec3_t delta, brush_t *brush, vec3_t move_points, vec3_t end );
extern unsigned int Brush_RemoveFace( brush_t *b, unsigned int faceIndex );  // 0x471640
extern void    Brush_Free( selbrush_t *b );                                  // 0x475ba0

// select.cpp - selection movers used by MoveSelection
extern void    Select_Move( const float *delta, char bSnap );
extern void    Select_Scale( float sx, float sy, float sz );
extern void    Select_RotateAxis( int axis, float deg, float (*rot_around)[4][3] );
extern void    Select_ApplyMatrix( float *matrix, selbrush_t *b, int bSnap, float deg, char bSwap );

// mainfrm / status
extern void        MainFrm_SetStatusText( int pane, const char *text );
extern CMainFrame *g_pParentWnd;                          // 0x25d5a70
extern void        CMainFrame_UpdatePatchToolbarButtons();// select.cpp (FATAL until P5)
extern void        sub_43E6F0( int buttons, int origin, int dir ); // addpoint-drag helper

// qcommon / com_math
extern void    Com_PrintMessage( const char *fmt, ... );
extern float   grid_sizes[];                              // 0x6dde5c

// abs32 (IDA): integer absolute value.
static inline int abs32( int x ) { return x < 0 ? -x : x; }

// patch_t (the patch instance node; .def @0, .selected @6) is now the shared struct in
// qe3.h — selbrush_t.patch is a patch_t*.

// IDA renders the editor's "point-editing" select sub-modes (sel_editpoint=10 and
// its sub-states 11..15, shown as sel_editpoint|sel_vertex etc.) as a contiguous
// range [10,15]. None equal sel_brush(0), so the normal drag path is unaffected.
static inline bool Drag_IsPointEditMode( int m ) { return m >= sel_editpoint && m <= 15; }

// Test_Ray (select.cpp): casts a pick ray against the brush lists, filling an edTrace_t
// (the real 88-byte trace_t layout, qe3.h).
extern void    Test_Ray( float *start, float *dir, int contents,
                         edTrace_t *t, int num_traces );

// g_PrefsDlg (0x73c704) — the editor preference singleton (prefs.h declares it as
// a real prefData_t*, non-NULL with the binary defaults from static-init onward).
// Reads use named fields (the prior byte-offset PREFS_* macros + NULL guards are
// retired: g_PrefsDlg is never NULL now).

// g_nPatchClickedView (0x73b108)
extern int  g_nPatchClickedView;   // defined in engine_stubs.cpp or pmesh.cpp

// ─── drag globals (IDB-verified addresses) ────────────────────────────────────
// IDB address 0x23f16f8: pressdelta (3-float accumulated drag offset).
// IDA names this g_someDragGlob in the decompile.
float  g_pressdelta[3];          // 0x23f16f8
// IDB 0x23f1718: press xvec (the axialised xaxis vector).
float  g_drag_xvec[3];           // 0x23f1718  (IDA "pressx" — misleading, it's a float[3])
// IDB 0x23f170c: press yvec (the axialised yaxis vector).
float  g_drag_yvec[3];           // 0x23f170c  (IDA "flt_23F170C")
// IDB 0x23f1708: integer press X pixel.
int    g_pressx_real;            // 0x23f1708
// IDB 0x23f1704: integer press Y pixel.
int    g_pressy_real;            // 0x23f1704
// NOTE: IDB 0x23f1720 ("flt_23F1720", once mis-named g_drag_vPressStart_z) is NOT a
// separate variable — it is g_drag_xvec[2] (0x23f1718 + 8). The real press-start
// vector is g_vPressStart[] at 0x23f16ec (declared below). Drag_Setup writes the
// xvec basis's z into g_drag_xvec[2] so AxializeVector(g_drag_xvec) sees all three
// components (the prior port wrote a separate var, leaving xvec[2] uninitialised).
// IDB 0x23f16e0: drag_first flag.
int    drag_first;               // 0x23f16e0
// IDB 0x23f1724: drag_ok flag.
int    drag_ok;                  // 0x23f1724

// IDB 0x23f16ec–0x23f16f4: vPressStart (3 floats, used in MoveSelection).
float  g_vPressStart[3];         // 0x23f16ec

// IDB 0x23f16d9: g_bRotateMode (bool)
bool   g_bRotateMode;            // 0x23f16d9
// IDB 0x23f16da: g_bScaleMode (bool)
bool   g_bScaleMode;             // 0x23f16da
// IDB 0x23f16dc: g_nScaleHow (char, low 3 bits = X/Y/Z lock)
char   g_nScaleHow;              // 0x23f16dc

// SCALE_X/Y/Z bit flags (IDA: g_nScaleHow & 1/2/4)
#define SCALE_X  1
#define SCALE_Y  2
#define SCALE_Z  4

// IDB 0x25d5b04: g_bPatchBendMode (int)
extern int g_bPatchBendMode;     // 0x25d5b04  defined in pmesh.cpp or engine_stubs

// IDB 0x23f164c: g_vRotation[3] (rotation accumulator for rotate mode)
float  g_vRotation[3];           // 0x23f164c
// IDB 0x23f1658: g_vRotateOrigin[3] (center of rotation)
float  g_vRotateOrigin[3];       // 0x23f1658
// IDB 0x231f548: g_vBendOrigin[3] (patch-bend origin; written by XY_MouseDown's
// rotate/bend branch, read by the parked patch-bend MoveSelection path).
float  g_vBendOrigin[3];         // 0x231f548

// IDB 0x23f16f8: selected_brushes_next is a pointer to the second element of
// selected_brushes (the display list). This is just selected_brushes.next.
// We DON'T redefine selected_brushes here (it's in engine_stubs.cpp).
// The IDA global 'selected_brushes_next' at 0x23f1868 is the 'next' pointer
// INSIDE the sentinel node. This is selected_brushes.next.
// Access it as selected_brushes.next everywhere.

// Helper to get the face index of a faceVis_s* within a selbrush_t's face array.
// IDA: (face - &brush->faces[0]) / sizeof(faceVis_s)  (the decompile renders the byte
// distance over a refCount-typed base; b->faces is the faceVis_s[] array).
static inline int drag_face_index( selbrush_t *b, faceVis_s *face )
{
    return (int)( face - b->faces );          // typed pointer subtraction = element index
}

// ─── AxializeVector (IDB 0x47e2a0, 105 bytes) ────────────────────────────────
// Collapses a vector to its dominant axis unit vector (same as GtkRadiant).
// __usercall: a1@<ecx> (the vector to normalize in-place).
// The binary's >=-negation tie-breaking picks the same axis as the strict-> chain here.
static void AxializeVector( float *v )
{
    float a[3];
    int   i;

    // Early out if two components are already zero
    if ( !v[0] && !v[1] ) return;
    if ( !v[1] && !v[2] ) return;
    if ( !v[0] && !v[2] ) return;

    for ( i = 0; i < 3; i++ )
        a[i] = (float)fabs( (double)v[i] );

    if ( a[0] > a[1] && a[0] > a[2] )
        i = 0;
    else if ( a[1] > a[0] && a[1] > a[2] )
        i = 1;
    else
        i = 2;

    float o = v[i];
    v[0] = v[1] = v[2] = 0.0f;
    v[i] = ( o < 0.0f ) ? -1.0f : 1.0f;
}

// ─── 0x47e3a0  Drag_Setup (1134 bytes) ───────────────────────────────────────
// KISAK (safer, kept per directive): the port adds `if(hitBrush)` null-guards in the
// buttons 9/0xD face paths, where the binary derefs v19.hit.brush unconditionally - a latent
// null-deref reachable via alt-edge-drag with no ray hit.
// Initialises a drag operation given the current mouse position and selection state.
// Called by Drag_Begin when MK_LBUTTON is pressed (non-area-select path).
//
// IDA __usercall params (normalised to cdecl):
//   trace_dir   (ecx) = 3D ray direction
//   trace_start (edx) = 3D ray origin
//   origin      (eax) = vPressStart / origin vector (3 floats, a3 in IDA)
//   x, y           = pixel coords of press
//   buttons        = MK_ button flags
//   a7             = xyvec (yvec in XY coords)
//
// We normalise to: Drag_Setup(origin, trace_start, trace_dir, x, y, buttons, xyvec)
// matching the Drag_Begin callsite:
//   Drag_Setup(trace_dir, trace_start, y, a3, x, buttons, a6)  [IDA 0x47eade]
// Callers pass:
//   trace_dir  -> 1st  (orig ecx)
//   trace_start-> 2nd  (orig edx)
//   y          -> 3rd  (orig eax, the origin/press-start float[3])
//   a3         -> 4th  (original x int)
//   x          -> 5th  (original y int)
//   buttons    -> 6th
//   a6         -> 7th  (xyvec)
// So the prototype visible to Drag_Begin is identical to Drag_Begin's call.
//
// Here we expose it as the engine calls it: with the three register args
// collapsed into explicit pointer params.
void Drag_Setup( float *trace_dir, float *trace_start, float *press_origin,
                 int x, int y, unsigned int buttons, float *xyvec )
{
    edTrace_t t;

    // pressdelta = { 0, 0, 0 }
    g_pressdelta[0] = 0.0f;
    g_pressdelta[1] = 0.0f;
    g_pressdelta[2] = 0.0f;

    // Store the screen X-axis world basis (IDA "pressx" @ 0x23f1718 — really the xvec).
    // All three components go into g_drag_xvec so AxializeVector below sees them.
    g_drag_xvec[0] = press_origin[0];  // IDA: pressx[0] = *a3
    g_drag_xvec[1] = press_origin[1];  // IDA: pressx[1]/"pressy" = a3[1]
    g_drag_xvec[2] = press_origin[2];  // IDA: flt_23F1720 (== xvec[2]) = a3[2]

    g_pressx_real = x;                // IDA: pressx_real = x
    drag_first    = 1;
    g_pressy_real = y;

    // Axialise the xvec (IDA: sub_47E2A0((int)pressx))
    AxializeVector( g_drag_xvec );

    // Copy and axialise the yvec (IDA: sub_47E2A0((int)flt_23F170C))
    g_drag_yvec[0] = xyvec[0];
    g_drag_yvec[1] = xyvec[1];
    g_drag_yvec[2] = xyvec[2];
    AxializeVector( g_drag_yvec );

    // ── sel_curvepoint mode ───────────────────────────────────────────────────
    if ( g_qeglobals.d_select_mode == sel_curvepoint )
    {
        if ( GetAsyncKeyState( VK_MENU ) < 0 )  // VK_MENU = 18 = Alt key
        {
            drag_ok = g_qeglobals.d_num_move_points > 0;
        }
        else
        {
            SelectCurvePointByRay( (int)trace_start, (int)trace_dir, buttons );
            if ( g_qeglobals.d_num_move_points || g_qeglobals.d_select_mode == sel_area )
            {
                drag_ok = 1;
                // Store selection rect (IDA: drag_selectionbox_x_1/x_2/y_1/y_2)
                g_qeglobals.drag_selectionbox_x_1 = x;
                g_qeglobals.drag_selectionbox_x_2 = y;
                g_qeglobals.drag_selectionbox_y_1 = x;
                g_qeglobals.drag_selectionbox_y_2 = y;
            }
        }
        g_nUpdateBits = -1;
        Undo_ClearRedo();
        Undo_GeneralStart( "drag curve point" );
        Undo_AddBrushList( &selected_brushes );
        Patch_RedispersePreDrag();   // snapshot live texCoords → savedTexCoord (PMESH_56)
        return;
    }

    // ── sel_cycle_edge_direction_quad mode ──────────────────────────────────
    if ( g_qeglobals.d_select_mode == sel_cycle_edge_direction_quad )
    {
        Patch_TurnEdge( (int)trace_start, (int)trace_dir );
        return;
    }

    // ── reset move points and terrain points ─────────────────────────────────
    g_qeglobals.d_num_move_points = 0;
    g_qeglobals.d_numterrapoints  = 0;

    // ── area select mode (sel_areatall equivalent) ────────────────────────────
    // IDA: if g_nPatchClickedView != 1 && !g_bRotateMode && !g_bScaleMode && buttons == 5
    //      with an additional check for Alt key or buttons == 6.
    //      The combined condition triggers sel_areatall with the selectionbox coords.
    if ( g_nPatchClickedView != 1 && !g_bRotateMode && !g_bScaleMode && buttons == 5 )
    {
        if ( ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 )
        {
            goto LABEL_15;
        }
    LABEL_19:
        // 0x47e57b: d_select_mode = (buttons & 2 | MK_CONTROL | MK_MBUTTON) >> 1, i.e.
        // (buttons & 2) ? 0x1A>>1 = 13 (box DESELECT) : 0x18>>1 = 12 (box ADD).
        g_qeglobals.d_select_mode = (select_t)(( (buttons & 2) | (MK_CONTROL | MK_MBUTTON) ) >> 1);
        g_qeglobals.drag_selectionbox_x_1 = x;
        g_qeglobals.drag_selectionbox_x_2 = y;
        g_qeglobals.drag_selectionbox_y_1 = x;
        g_qeglobals.drag_selectionbox_y_2 = y;
        drag_ok       = 1;
        g_nUpdateBits = -1;
        return;
    }

    // Additional check: buttons == 6 && Alt down → LABEL_19
    if ( buttons == 6 && GetAsyncKeyState( VK_MENU ) < 0 )
    {
        goto LABEL_19;
    }

LABEL_15:
    // ── no selection? start a 'create brush' undo and return ─────────────────
    if ( selected_brushes.next == &selected_brushes )
    {
        Undo_ClearRedo();
        Undo_GeneralStart( "create brush" );
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0,
                      (LPARAM)"No selection to drag\n" );
        return;
    }

    // ── sel_vertex mode ───────────────────────────────────────────────────────
    if ( g_qeglobals.d_select_mode == sel_vertex )
    {
        SelectVertexByRay( (int)trace_start, (int)trace_dir );
        if ( g_qeglobals.d_num_move_points )
        {
            drag_ok = 1;
            Undo_Start( "drag vertex" );
            // LABEL_41:
            Undo_AddBrushList( &selected_brushes );
            return;
        }
    }

    // ── sel_edge mode ─────────────────────────────────────────────────────────
    if ( g_qeglobals.d_select_mode == sel_edge )
    {
        Select_Edge( (int)trace_dir, (int)trace_start );
        if ( g_qeglobals.d_num_move_points )
        {
            drag_ok = 1;
            Undo_Start( "drag edge" );
            Undo_AddBrushList( &selected_brushes );
            return;
        }
    }

    // ── ray test: direct hit on selection or Alt+drag face-extrude ───────────
    memset( &t, 0, sizeof(t) );
    Test_Ray( trace_start, trace_dir, 257, &t, 1 );
    // m_bALTEdge (ALTEdgeDrag, default 1): Alt+drag face-extrude. Inert at rest (the
    // AND is false unless Alt is physically held), so default behaviour is unchanged.
    if ( t.selected || ( g_PrefsDlg->m_bALTEdge && GetAsyncKeyState( VK_MENU ) < 0 ) )
    {
        drag_ok = 1;
        Undo_Start( "drag selection" );
        Undo_AddBrushList( &selected_brushes );

        selbrush_t *hitBrush = (selbrush_t *)t.hit.brush;
        faceVis_s  *hitFace  = (faceVis_s  *)t.hit.face;

        if ( buttons == 9 )  // MK_LBUTTON|MK_CONTROL = 9
        {
            // Shear dragging: select single face
            if ( hitBrush )
            {
                iassert( t.hit.brush->version == t.hit.brush->def->version );   // drag.cpp:203
                iassert( t.hit.face >= &t.hit.brush->faces[0] && t.hit.face < &t.hit.brush->faces[t.hit.brush->faceCount] );   // drag.cpp:204
                int fIdx = drag_face_index( hitBrush, hitFace );
                // IDA: arg2 = (int)&brush->def->faces[fIdx]
                Brush_SelectFaceForDragging( hitBrush->def,
                    (int)&hitBrush->def->faces[fIdx], 1 );
            }
        }
        else if ( buttons == (MK_LBUTTON | MK_SHIFT | MK_CONTROL) )  // 0xD
        {
            // Sticky drag: select all faces on the hit brush
            if ( hitBrush && hitBrush->faceCount )
            {
                for ( int v17 = 0; (unsigned)v17 < (unsigned)hitBrush->faceCount; v17++ )
                {
                    // IDA: arg2 = (int)&brush->def->faces[v17]
                    Brush_SelectFaceForDragging( hitBrush->def,
                        (int)&hitBrush->def->faces[v17], 0 );
                }
            }
        }
        return;
    }

    // ── side-hit check (not vertex/edge mode) ─────────────────────────────────
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
    {
        return;  // no side-hit in vertex/edge mode
    }

    // IDA: check if single vs multiple selected brushes.
    // IDA shows (selbrush_t **)selected_brushes_next->next — the cast is a
    // decompiler artifact; this is just a selbrush_t* comparison.
    if ( selected_brushes.next->next == &selected_brushes )
    {
        // Single brush selected
        if ( (buttons & MK_CONTROL) != 0 )
        {
            Brush_SideSelect( trace_start, selected_brushes.next->def, trace_dir, 1 );
        }
        else
        {
            Brush_SideSelect( trace_start, selected_brushes.next->def, trace_dir, 0 );
        }
    }
    else
    {
        // Multiple brushes selected — iterate
        for ( selbrush_t *v10 = selected_brushes.next;
              v10 != &selected_brushes;
              v10 = v10->next )
        {
            if ( buttons & MK_CONTROL )
            {
                Brush_SideSelect( trace_start, v10->def, trace_dir, 1 );
            }
            else
            {
                Brush_SideSelect( trace_start, v10->def, trace_dir, 0 );
            }
        }
    }

    drag_ok = 1;
    Undo_Start( "side stretch" );
    Undo_AddBrushList( &selected_brushes );
}

// ─── 0x47e810  Drag_FaceAlign (115 bytes, was sub_47E810) ────────────────────
// Aligns the texture of the second selected face to the first selected face.
// Called from Drag_Begin when exactly 2 faces are selected and the middle-button
// with Alt is released (IDA line 270 assert).
//
// IDA: Assert(count == 2), then:
//   sub_476A30(&g_SelectedFaces.GetAt( 0 ).brush->def->faces[g_SelectedFaces.GetAt( 0 ).index],
//              g_SelectedFaces.GetAt( 1 ).brush->def,
//              &g_SelectedFaces.GetAt( 1 ).brush->def->faces[g_SelectedFaces.GetAt( 1 ).index])
// sub_476A30 = Brush_ApplyTextureProjection (brush.cpp, 0x476a30).
//
// NOTE: g_SelectedFaces lives in select.cpp (model in qe3.h).

// Forward: Brush_ApplyTextureProjection (brush.cpp 0x476a30)
// IDA a1=srcFace, a2=dstBrushDef (brush_t*), a3=dstFace — brush is the middle arg.
extern brush_t *Brush_ApplyTextureProjection( int srcFace, brush_t *b, int dstFace );

// count != 2 -> Assert 270; count < 2 -> unknown_libname_291 (CRT range-check abort,
// noreturn).  The port's explicit `return` after the abort is harmless; the binary falls
// through into dead code.
void Drag_FaceAlign()
{
    int count = g_SelectedFaces.GetSize();
    iassert( g_SelectedFaces.GetCount() == 2 );   // drag.cpp:270

    if ( count < 2 )
    {
        // IDA: unknown_libname_291() — CRT bounds-check abort
        extern void unknown_libname_291();
        unknown_libname_291();
        return;
    }

    // Apply the texture projection from g_SelectedFaces.GetAt( 0 ) to g_SelectedFaces.GetAt( 1 )
    // IDA: sub_476A30(&g_SelectedFaces.GetAt( 0 ).brush->def->faces[g_SelectedFaces.GetAt( 0 ).index],
    //                  g_SelectedFaces.GetAt( 1 ).brush->def,            ← a2 = brush_t*
    //                  &g_SelectedFaces.GetAt( 1 ).brush->def->faces[g_SelectedFaces.GetAt( 1 ).index])
    Brush_ApplyTextureProjection(
        (int)&g_SelectedFaces.GetAt( 0 ).brush->def->faces[ g_SelectedFaces.GetAt( 0 ).index ],
             g_SelectedFaces.GetAt( 1 ).brush->def,
        (int)&g_SelectedFaces.GetAt( 1 ).brush->def->faces[ g_SelectedFaces.GetAt( 1 ).index ]
    );
}

// ─── 0x463a80  ClampGridSize ─────────────────────────────────────────────────
// The angle/grid snap table (IDA name is misleading — it maps the grid index to a
// rotation-snap step). Used by Drag_MouseMoved / MoveSelection in rotate/bend mode.
// 0/1->1, 2->2, 3->5, 4->15, 5->30, 6/7->45, default->90.
extern "C" int ClampGridSize();
int ClampGridSize()
{
    switch ( g_qeglobals.d_gridsize )
    {
        case 0: case 1: return 1;
        case 2:         return 2;
        case 3:         return 5;
        case 4:         return 15;
        case 5:         return 30;
        case 6: case 7: return 45;
        default:        return 90;
    }
}

// ─── 0x47e890  Drag_Begin (the mouse-down dispatcher) ─────────────────────────
// Decides, from the button/modifier combination and the select mode, what a press starts:
// a group/face select (SelectFaceSth), a brush drag (Drag_Setup), an alt-terrain-paint, or
// a texture pick/apply.  Contents-flag prefs: entities_off 0x200 / models 0x400 / sky 0x800
// / viewz 2 -> 0x1000, 0 -> 4.  The middle/right-button tail has five sub-branches by
// nMouseButton + modifier bits (pick / apply-brush / apply-face / "set face but leave
// info" / face-align).  KISAK: the brush-apply branches route to sub_476ED0, a benign
// no-op until the per-brush texture-projection helpers are ported.
//
// Clean cdecl signature (the IDA is __usercall with eax/edx/ecx args — see
// CXYWnd::XY_MouseDown 0x467850 for the call). Params:
//   pressFunc   = press/snap callback ptr (IDA a1 → g_qeglobals.camera_fov_setup)
//   buttons     = MK_ button flags
//   viewz       = view depth-axis selector (XY view passes 0)  [IDA ecx0]
//   px, py      = press pixel coords
//   xvec, yvec  = the view's screen-X / screen-Y world basis (1/scale magnitude)
//   trace_start, trace_dir = the picking ray
//
extern int  sub_401D50();                       // "is terrain-paint mode" gate (stub: 0)
extern int  CurvEditDlg_OnSomeSetting();        // CurvEditDlg (P5 dialog) — FATAL stub
extern int  g_nPatchClickedView;                // 0x73b108

// --- middle/right-button texture pick/apply tail deps ---
// (Test_Ray + g_SelectedFaces + Drag_FaceAlign already declared above.)
extern void UpdatePatchInspector();                                   // patchdialog.cpp 0x436db0
namespace SurfaceInspector { void UpdateSurfaceDialog(); }            // 0x458590 (engine_stubs no-op)
// brush.cpp ports used by the apply branches:
//   Brush_SetFaceTexdefSize == Face_SetMaterial (0x476740): write {lyrMtl,radMtl} into face->mtldef[layer]
extern void Brush_SetFaceTexdefSize( const float *size2, face_t *f, brush_t *b ); // 0x476740
extern void sub_4766F0( brush_t *def, const face_t *srcFace );        // brush.cpp 0x4766f0 (propagate face texdef)
// per-BRUSH apply (set every face to the current material) — documented benign NO-OP stub
// (the faithful body needs the unported per-brush texture-projection helpers; engine_stubs.cpp).
extern void sub_476ED0( brush_t *def, MaterialDef *srcTexBlock, char a3, float a4, char a5 ); // 0x476ed0 (real port in brush.cpp)
// texture-pick deps:
//   Texture_SetTexture (0x45be50, texwnd.cpp): writes the picked MaterialDef into
//       random_texture_stuff[layer], realizes it, applies it (Brush_SetTexture) and
//       scrolls the browser to it.
//   sub_45D320 (0x45d320): &mtldef->mat_texDef + LayerMat::GetCurrentLayer (below).
//   sub_44B620 (0x44b620, brush.cpp Ed_Patch_GetTexdef): patch-texdef extraction
//       (inline-FPU).  Always returns 0 (the binary has no success path); the texdef
//       WRITE is the observable.
namespace LayerMat { int GetCurrentLayer( MaterialDef *md ); }   // materialdef.cpp 0x431b30
extern char Texture_SetTexture( const int *a1, MaterialDef *a2 ); // texwnd.cpp 0x45be50
extern char Radiant_PatchGetTexdef( patchMesh_t *patch, texdef_sub_t *texdef ); // brush.cpp 0x44b620

// sub_45D320 (0x45d320): return &md->mat_texDef + GetCurrentLayer(md).  texdef_sub_t is the
// per-layer texdef element; the IDB adds the LayerMat current-layer index to the texdef ptr.
static texdef_sub_t *sub_45D320( MaterialDef *md )
{
    return &md->mat_texDef + LayerMat::GetCurrentLayer( md );
}

void Drag_Begin( void *pressFunc, unsigned int buttons, int viewz,
                 int px, int py, float *xvec, float *yvec,
                 float *trace_start, float *trace_dir )
{
    int contents = 0;

    g_pressdelta[0] = g_pressdelta[1] = g_pressdelta[2] = 0.0f;
    g_qeglobals.camera_fov_setup = pressFunc;
    g_vPressStart[0] = g_vPressStart[1] = g_vPressStart[2] = 0.0f;
    drag_ok    = 0;
    drag_first = 1;

    if ( g_PrefsDlg->entities_off )         contents  = 0x200;   // entities_off (default 0)
    if ( g_PrefsDlg->m_bSelectableModels )  contents |= 0x400;   // ModelSelection (default 0)
    if ( g_PrefsDlg->sky_brush_off )        contents |= 0x800;   // sky_brush_off (default 0)
    if ( viewz == 2 )       contents |= 0x1000;
    else if ( viewz == 0 )  contents |= 4;

    iassert( (buttons & MK_ALT) == 0 );   // drag.cpp:323 (MK_ALT=0x20, oleidl.h)

    const select_t mode = g_qeglobals.d_select_mode;

    if ( buttons == ( MK_LBUTTON | MK_SHIFT ) )            // 3 = select group
    {
        if ( ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 )
            goto LABEL_29;
        goto LABEL_18;
    }
    if ( buttons == ( MK_RBUTTON | MK_SHIFT ) )            // 6
    {
        if ( ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 )
            goto LABEL_34;
    LABEL_18:
        if ( mode != sel_curvepoint )
        {
            if ( g_nPatchClickedView == 1 )
            {
            LABEL_33:
                SelectFaceSth( (int)trace_dir, (int)trace_start, contents );
                return;
            }
            goto LABEL_20;
        }
    }
    if ( buttons == 13 )                                  // LMB|SHIFT|CTRL
    {
        if ( GetAsyncKeyState( VK_MENU ) < 0 && mode != sel_curvepoint )
        {
            SelectFaceSth( (int)trace_dir, (int)trace_start, contents | 0x40 );
            return;
        }
        if ( ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 && mode != sel_curvepoint )
        {
            SelectFaceSth( (int)trace_dir, (int)trace_start, 520 );
            return;
        }
        goto LABEL_34;
    }
    if ( buttons != 5 )                                   // LMB|CTRL
        goto LABEL_34;
LABEL_29:
    if ( ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 && mode != sel_curvepoint )
    {
        if ( g_PrefsDlg->enable_light_preview )   // light/sun preview pick (default 1)
            contents |= 0x4000;
        goto LABEL_33;
    }
LABEL_34:
    // Alt + LMB/RMB terrain-paint start. sub_401D50() is a 0-stub (terrain paint
    // mode off) so this never enters in the current build; Alt+drag falls through
    // to Drag_Setup. (CurvEditDlg / Patch_Paint_Start are Phase-5 deps inside.)
    if ( GetAsyncKeyState( VK_MENU ) < 0
         && ( buttons == 1 || buttons == 2 )
         && ( mode == sel_brush || mode == sel_addpoint )
         && sub_401D50() )
    {
        if ( OnlyPatchesSelected()
             || ( selected_brushes.next == &selected_brushes && CurvEditDlg_OnSomeSetting() ) )
        {
            if ( Patch_Paint_Start() )
            {
                bool wasCycle = ( g_qeglobals.d_select_mode == sel_cycle_edge_direction_quad );
                g_qeglobals.d_select_mode = sel_addpoint;
                if ( wasCycle )
                    CMainFrame_UpdatePatchToolbarButtons();
                drag_ok = 1;
            }
        }
        return;
    }

    if ( ( buttons & MK_LBUTTON ) != 0 )
    {
    LABEL_20:
        // press_origin = xvec, xyvec = yvec (see Drag_Setup's IDA mapping).
        Drag_Setup( trace_dir, trace_start, xvec, px, py, buttons, yvec );
        return;
    }

    // ── middle/right-button texture pick / apply / face-align ────────────────
    // FAITHFUL vs IDA 0x47eb00..0x47f0b8.  nMouseButton selects the texture-edit
    // button (16 for 3-button mice, 2 for 2-button).  Five sub-branches by modifier
    // bits; the texture-PICK branch (branch 1) needs the unported texture-window
    // subsystem (Texture_SetTexture 0x45be50 + the patch-texdef inline-FPU sub_44B620)
    // so it FATALs precisely there.  All other branches are real ports (brush/face
    // apply via sub_476ED0 [benign no-op] / Face_SetMaterial / sub_4766F0; face-align
    // via the ported Drag_FaceAlign).  Not reachable from a plain left-click drag.
    const int nMouseButton = ( g_PrefsDlg->m_nMouseButtons != MK_RBUTTON ) ? 16 : 2;
    const bool altDown = ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0;
    const int layer = g_qeglobals.current_edit_layer;

    edTrace_t t;

    // ── BRANCH 1: nMouseButton, no Alt → pick the hit face's texture (0x47eb00) ──
    if ( buttons == (unsigned)nMouseButton && !altDown )
    {
        Test_Ray( trace_start, trace_dir, 512, &t, 1 );
        if ( !t.hit.face )
        {
            Sys_Printf( "Did not select a texture\n" );
            return;
        }
        selbrush_t *brush = t.hit.brush;
        brush_t    *def   = brush->def;
        // Remember the hit brush's vertical extent (bottom/top) as the new-brush template.
        g_qeglobals.d_new_brush_bottom_x = def->mins[0];   // def+0x20
        g_qeglobals.d_new_brush_bottom_y = def->mins[1];
        g_qeglobals.d_new_brush_bottom_z = def->mins[2];
        g_qeglobals.d_new_brush_top_x = def->maxs[0];      // def+0x2C
        g_qeglobals.d_new_brush_top_y = def->maxs[1];
        g_qeglobals.d_new_brush_top_z = def->maxs[2];

        iassert( t.hit.face >= &t.hit.brush->faces[0] && t.hit.face < &t.hit.brush->faces[t.hit.brush->faceCount] );   // drag.cpp:413
        iassert( t.hit.brush->version == t.hit.brush->def->version );   // drag.cpp:414

        int faceIdx = (int)( t.hit.face - brush->faces );   // faceVis_s stride 12
        face_t *face = &def->faces[ faceIdx ];
        if ( brush->patch )
        {
            texdef_sub_t *td = sub_45D320( &face->mtldef[ layer ] );
            Radiant_PatchGetTexdef( def->patch, td );   // 0x44b620 (return discarded — see brush.cpp)
        }
        // patch-consistency assert
        patch_t *pinst = brush->patch;
        // Verbatim condition (drag.cpp:422). Short-circuit keeps it deref-safe for both
        // consistent states (plain brush: first clause true; patch brush: patch != NULL);
        // the inconsistent patch==NULL && def->patch!=NULL state derefs NULL exactly as
        // the original source would have.
        iassert( (t.hit.brush->patch == NULL && t.hit.brush->def->patch == NULL) || (t.hit.brush->patch->def == t.hit.brush->def->patch) );
        // a1 = the patch-vertex projection block (patch_t def) for a patch pick, else 0.
        // The brush-FACE pick (non-patch, the common case) passes 0 → no projection copy,
        // and is now fully functional.  The PATCH pick reaches sub_44B620 (FATAL) above first.
        const int *patchDef = pinst ? (const int *)pinst->def : nullptr;
        Texture_SetTexture( patchDef, &face->mtldef[ layer ] );   // NOW PORTED (texwnd.cpp 0x45be50)
        SurfaceInspector::UpdateSurfaceDialog();
        UpdatePatchInspector();
        return;
    }

    // ── BRANCH 2: nMouseButton|8, no Alt → apply current material to whole brush ──
    if ( buttons == (unsigned)( nMouseButton | 8 ) && !altDown )
    {
        Test_Ray( trace_start, trace_dir, 0, &t, 1 );
        if ( !t.hit.brush )
        {
            Sys_Printf( "Didn't hit a brush\n" );
            return;
        }
        // IDA reads eclass via owner->def (entity_s_def), NOT owner->eclass directly
        // (the instance-vs-def pattern): [owner+8]=def, [def+0x60]=eclass, [eclass+8]=fixedsize.
        if ( ( (entity_s_def *)t.hit.brush->owner->def )->eclass->fixedsize )
        {
            Sys_Printf( "Can't change an entity texture\n" );
            return;
        }
        // sub_476ED0 = per-brush apply (benign no-op stub until the per-brush
        // texture-projection helpers are ported).
        sub_476ED0( t.hit.brush->def, (MaterialDef *)&g_qeglobals.random_texture_stuff[ layer ], 1, 1.0f, 0 );
        g_nUpdateBits = -1;
        return;
    }

    // ── BRANCH 3: nMouseButton|0xC, no Alt → apply current material to the face ──
    if ( buttons == (unsigned)( nMouseButton | 0xC ) && !altDown )
    {
        Test_Ray( trace_start, trace_dir, 0, &t, 1 );
        selbrush_t *brush = t.hit.brush;
        if ( !brush )
        {
            Sys_Printf( "Didn't hit a brush\n" );
            return;
        }
        if ( ( (entity_s_def *)brush->owner->def )->eclass->fixedsize )
        {
            Sys_Printf( "Can't change an entity texture\n" );
            return;
        }
        brush_t *def = brush->def;
        if ( brush->patch )   // IDA: cmp [ebx+20h],0 — patch INSTANCE present ⇒ whole-brush apply
        {
            sub_476ED0( def, (MaterialDef *)&g_qeglobals.random_texture_stuff[ layer ], 1, 0.0f, 0 );
            g_nUpdateBits = -1;
            return;
        }
        iassert( t.hit.face >= &t.hit.brush->faces[0] && t.hit.face < &t.hit.brush->faces[t.hit.brush->faceCount] );   // drag.cpp:473
        iassert( t.hit.brush->version == t.hit.brush->def->version );   // drag.cpp:474
        int faceIdx = (int)( t.hit.face - brush->faces );
        face_t *face = &def->faces[ faceIdx ];
        // copy the whole current-layer MaterialDef (36 bytes) onto the picked face.
        memcpy( &face->mtldef[ layer ], &g_qeglobals.random_texture_stuff[ layer ].mtl, sizeof(MaterialDef) );
        if ( def->patch )
            sub_4766F0( def, face );          // propagate to all faces (patch case)
        else
            ++def->version;
        Brush_BuildWindings( def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++def->version;
        g_nUpdateBits = -1;
        return;
    }

    // ── BRANCH 4: nMouseButton|4, no Alt → set brush face texture but leave info ──
    if ( buttons == (unsigned)( nMouseButton | 4 ) && !altDown )
    {
        Sys_Printf( "Set brush face texture but leave info\n" );
        Test_Ray( trace_start, trace_dir, 0, &t, 1 );
        selbrush_t *brush = t.hit.brush;
        if ( !brush )
        {
            Sys_Printf( "Didn't hit a brush\n" );
            return;
        }
        if ( ( (entity_s_def *)brush->owner->def )->eclass->fixedsize )
        {
            Sys_Printf( "Can't select an entity brush face\n" );
            return;
        }
        brush_t *def = brush->def;
        if ( brush->patch )   // patch instance present ⇒ write the patch's texture/lightmap/smoothing pair
        {
            patchMesh_t *pd = brush->patch->def;          // [esi+20h] then [eax]
            patchMesh_material *slot = (&pd->texture) + layer;   // 0x18 + 8*layer
            slot->lyrMtl = g_qeglobals.random_texture_stuff[ layer ].mtl.lyrMtl;
            slot->radMtl = g_qeglobals.random_texture_stuff[ layer ].mtl.radMtl;
            ++pd->version;                                 // patch+0x5040
            UpdatePatchInspector();                        // if (CWnd_PatchDialog.m_hWnd) GetPatchInfo()
            g_nUpdateBits = -1;
            return;
        }
        iassert( t.hit.face >= &t.hit.brush->faces[0] && t.hit.face < &t.hit.brush->faces[t.hit.brush->faceCount] );   // drag.cpp:504
        iassert( t.hit.brush->version == t.hit.brush->def->version );   // drag.cpp:505
        int faceIdx = (int)( t.hit.face - brush->faces );
        face_t *face = &def->faces[ faceIdx ];
        // Face_SetMaterial(&random_texture_stuff[layer], face, def): write the {lyrMtl,radMtl}
        // pair into face->mtldef[layer]; ++def->version.
        Brush_SetFaceTexdefSize( (const float *)&g_qeglobals.random_texture_stuff[ layer ], face, def );
        g_nUpdateBits = -1;
        return;
    }

    // ── BRANCH 5: nMouseButton + Alt → align texture between two selected faces ──
    if ( buttons == (unsigned)nMouseButton && altDown )
    {
        Select_Deselect( 0 );
        int n = g_SelectedFaces.GetSize();
        if ( n > 1 )
            g_SelectedFaces.RemoveAt( 0, n - 1 );   // keep only the last selected face
        SelectFaceSth( (int)trace_dir, (int)trace_start, 520 );
        if ( g_SelectedFaces.GetSize() == 2 )
            Drag_FaceAlign();                   // ported; internally parks on texturevecs_02
        return;
    }
}

// ─── 0x4a5930  ProjectPointOntoRay (IDB-misnamed "Vec3Normalize") ─────────────
// Ported verbatim from IDA 0x4a5930.  NOT a normalize: projects `point` onto the
// ray (origin, dir) and writes the foot of the perpendicular into `out`.
//   v = point - origin;  t = dot(dir, v);  out = origin + dir*t
// `dir` must be unit length (the binary asserts Vec3IsNormalized at com_math.cpp:1357).
// Register map (verified at the MoveSelection call site 0x47f714: ebx=out, edi=origin,
// esi=dir, push &point): out@<ebx>, origin@<edi>, dir@<esi>, point@arg.
static void Drag_ProjectPointOntoRay( float *out, const float *origin,
                                      const float *dir, const float *point )
{
    // KEEP_VERBOSE (also Drag_RayPointDistSq below): local copy of a CoD3 com_math fn
    // kisak's engine doesn't carry — the com_math.cpp:1357/1344 strings stay verbatim.
    if ( !Vec3IsNormalized( dir ) )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp",
                1357, 0, "%s", "Vec3IsNormalized( dir )" );

    float v6 = point[0] - origin[0];
    float v7 = point[1] - origin[1];
    float v8 = point[2] - origin[2];
    float v9 = dir[0] * v6 + dir[1] * v7 + dir[2] * v8;
    out[0] = dir[0] * v9 + origin[0];
    out[1] = dir[1] * v9 + origin[1];
    out[2] = v9 * dir[2] + origin[2];
}

// ─── 0x4a5860  RayPointDistSq (terrain distance² helper) ──────────────────────
// Ported verbatim from IDA 0x4a5860.  Returns the squared perpendicular distance of
// `point` from the ray (origin, dir).  `dir` must be unit length (the binary asserts
// Vec3IsNormalized at com_math.cpp:1344).
//   d = point - origin;  proj = dot(dir, d);  perp = d - dir*proj;  return |perp|²
// Register map (verified at 0x47f7fd: ebx=point, edi=origin, esi=dir): point@<ebx>,
// origin@<edi>, dir@<esi>.  (Hex-rays prints the math with proj negated then re-added;
// transcribed in that exact order to preserve float32 rounding.)
static float Drag_RayPointDistSq( const float *point, const float *origin,
                                  const float *dir )
{
    if ( !Vec3IsNormalized( dir ) )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp",
                1344, 0, "%s", "Vec3IsNormalized( dir )" );

    float v5 = point[0] - origin[0];
    float v7 = point[1] - origin[1];
    float v9 = point[2] - origin[2];
    float v11 = dir[2] * v9 + dir[0] * v5 + dir[1] * v7;
    float v12 = -v11;
    float v6 = v5 + dir[0] * v12;
    float v8 = v7 + dir[1] * v12;
    float v10 = v12 * dir[2] + v9;
    return v10 * v10 + v8 * v8 + v6 * v6;
}

// ─── 0x47f0c0  MoveSelection (apply a world-space move to the selection) ──────
// Called by Drag_MouseMoved with the snapped world delta.  Dispatches by mode:
//   rotate/bend (LABEL_119) - grid-snap (move/grid * ClampGridSize) + sub-unit 0.1, axis and
//     angle from m_pActiveXY->m_nViewType, g_vRotation accumulate + status text, then the
//     free-rotate (Select_RotateAxis -> Select_ApplyMatrix per selected brush) and the
//     patch-bend (Patch_SelectBendNormal @2*angle, Patch_SelectBendAxis @angle) sub-branches.
//     The patch-bend check runs BEFORE the scale dispatch (IDA order).
//   scale - axis locks from scale-how; factor move[1]<0 -> 0.9 / >0 -> 1.1
//   translate - move-points add, per-brush BuildWindings + the maxs<=mins / >262144 validity
//     test + Patch_DragScale; a blocked move is undone
//   sel_curvepoint (Patch_UpdateSelected) / sel_vertex (Brush_MoveVertex accumulate)
//   terrain points - the sculpt path (d_terrapoints), see below
// Asserts 711/800/747/748.  KISAK: the status text goes to the decoupled
// MainFrm_SetStatusText (the binary uses get_m_strStatus + UpdateStatusText).
void MoveSelection( float *origin, float *dir, float *move )
{
    char statusBuf[160];

    if ( move[0] == 0.0f && move[1] == 0.0f && move[2] == 0.0f )
        return;

    if ( g_bRotateMode )
        goto ROTATE_OR_BEND;

    // Scale-how axis lock: zero the locked component(s) of `move`.
    {
        char how = g_nScaleHow;
        if ( how & SCALE_X ) move[0] = 0.0f;
        if ( how & SCALE_Y ) move[1] = 0.0f;
        if ( how & SCALE_Z ) move[2] = 0.0f;

        // IDA 0x47f1a8 checks patch-bend BEFORE scale: if ::g_bPatchBendMode is set the binary
        // jumps to the rotate/bend path (LABEL_119) regardless of scale mode. (Both modes are off
        // by default + mutually exclusive in practice, but match the binary's check order.)
        if ( g_bPatchBendMode )
            goto ROTATE_OR_BEND;

        if ( g_bScaleMode )
        {
            // IDA: factor from sign of move[1] (v12==0 vs v13==move[1]); 0.9/1.1/1.0.
            float f = 1.0f;
            if      ( move[1] < 0.0f ) f = 0.89999998f;
            else if ( move[1] > 0.0f ) f = 1.1f;
            float sx = ( how & SCALE_X ) ? 1.0f : f;
            float sy = ( how & SCALE_Y ) ? 1.0f : f;
            float sz = ( how & SCALE_Z ) ? 1.0f : f;
            Select_Scale( sx, sy, sz );
            g_nUpdateBits = -1;
            return;
        }
    }

    // ── translation ─────────────────────────────────────────────────────────
    {
        float dist[3];
        dist[0] = g_pressdelta[0] - g_vPressStart[0];
        dist[1] = g_pressdelta[1] - g_vPressStart[1];
        dist[2] = g_pressdelta[2] - g_vPressStart[2];
        _snprintf( statusBuf, sizeof( statusBuf ),
                   "Distance x: %.1f  y: %.1f  z: %.1f", dist[0], dist[1], dist[2] );
        MainFrm_SetStatusText( 3, statusBuf );
    }

    {
        char v73 = 0;
        if ( g_qeglobals.d_num_move_points == 0 && g_qeglobals.d_select_mode != sel_area )
            goto ON_BRUSH_MOVE;
        v73 = 1;

        if ( g_qeglobals.d_select_mode == sel_area )
        {
            Sys_Printf( "MoveSelection for sel_area: should never happen, tell a coder\n" );
            return;
        }
        if ( g_qeglobals.d_select_mode == sel_curvepoint )
        {
            Patch_UpdateSelected( (int)move );
            return;
        }
        if ( g_qeglobals.d_select_mode != sel_vertex )
        {
            // ── normal brush / move-point drag ───────────────────────────────
            // (nested scope so the early `goto ON_BRUSH_MOVE` above does not jump
            //  over these initialisers — they are out of scope at the label.)
            {
            for ( int ii = 0; ii < g_qeglobals.d_num_move_points; ++ii )
            {
                drawVert_t *mp = g_qeglobals.d_move_points[ii];
                mp->xyz[0] += move[0]; mp->xyz[1] += move[1]; mp->xyz[2] += move[2];
            }

            selbrush_t *blocked = nullptr;
            selbrush_t *b = selected_brushes.next;
            if ( b != &selected_brushes )
            {
                for ( ; b != &selected_brushes; b = b->next )
                {
                    brush_t *def = b->def;
                    float preExt[3] = { def->maxs[0] - def->mins[0],
                                        def->maxs[1] - def->mins[1],
                                        def->maxs[2] - def->mins[2] };
                    Brush_BuildWindings( def, 1 );
                    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                        SetupVertexSelection();
                    MarkMapModified();
                    ++def->version;

                    int okAxes = 0;
                    for ( ; okAxes < 3; ++okAxes )
                    {
                        if ( def->maxs[okAxes] <= def->mins[okAxes] ) break;
                        if ( def->maxs[okAxes] - def->mins[okAxes] > 262144.0f ) break;
                    }
                    if ( okAxes != 3 ) { blocked = b; break; }

                    if ( b->patch )
                    {
                        float center[3] = {
                            ( def->maxs[0] - def->mins[0] ) - preExt[0],
                            ( def->maxs[1] - def->mins[1] ) - preExt[1],
                            ( def->maxs[2] - def->mins[2] ) - preExt[2] };
                        iassert(b->def->patch == b->patch->def);
                        if ( !Patch_DragScale( center, b->patch->def, move ) )
                        { blocked = b; break; }
                    }
                }

                if ( blocked )
                {
                    Com_PrintMessage( "MoveSelection: Attempted drag or nudge blocked because "
                                      "it would invalidate or destroy a brush.\n" );
                    // Undo the move-point translation and rebuild.
                    for ( int jj = 0; jj < g_qeglobals.d_num_move_points; ++jj )
                    {
                        drawVert_t *mp = g_qeglobals.d_move_points[jj];
                        mp->xyz[0] -= move[0]; mp->xyz[1] -= move[1]; mp->xyz[2] -= move[2];
                    }
                    for ( selbrush_t *kk = selected_brushes.next; kk != &selected_brushes; kk = kk->next )
                    {
                        Brush_BuildWindings( kk->def, 1 );
                        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                            SetupVertexSelection();
                        MarkMapModified();
                        ++kk->def->version;
                    }
                }
            }
            }  // end nested scope (blocked/b/ii)

        ON_BRUSH_MOVE:
            if ( g_qeglobals.d_numterrapoints )
            {
                // --- terrain-point drag (sculpt), IDA 0x47f646..0x47fbc3 ---
                // d_terrapoints[] are the side-face planept vec3's collected by
                // Brush_SideSelect (numberId==2 brush).  Each d_terrapoints[i] is a terrainVert_t*
                // read as a contiguous vec3: ->height(x) / ->scale(y) / [1].height(z).
                // d_select_translate_unk = the dragged plane's normal (set by Brush_SideSelect).
                // Algorithm: centroid of the cluster → project onto the click ray → find the
                // nearest cluster point → scale the WHOLE cluster about the centroid by the
                // dragged radius ratio (capped at 5×); rebuild the selected brushes; if any
                // brush goes degenerate, undo the scale (×1/ratio) and rebuild.
                float centroid[3] = { 0.0f, 0.0f, 0.0f };
                int v36 = 0;
                if ( g_qeglobals.d_numterrapoints > 0 )
                {
                    do
                    {
                        terrainVert_t *v37 = g_qeglobals.d_terrapoints[v36++];
                        centroid[0] = v37->height    + centroid[0];
                        centroid[1] = v37->scale     + centroid[1];
                        centroid[2] = v37[1].height  + centroid[2];
                    } while ( v36 < g_qeglobals.d_numterrapoints );
                }
                float inv = 1.0f / (double)g_qeglobals.d_numterrapoints;
                centroid[0] = inv * centroid[0];
                centroid[1] = centroid[1] * inv;
                centroid[2] = inv * centroid[2];

                iassert( origin );   // drag.cpp:747
                iassert( dir );   // drag.cpp:748

                // project the centroid onto the click ray (foot of perpendicular).
                float end[3];
                Drag_ProjectPointOntoRay( end, origin, dir, centroid );

                // find the cluster point nearest the projected point.
                int best = 0;
                {
                    terrainVert_t *t0 = g_qeglobals.d_terrapoints[0];
                    float d0[3] = { t0->height - end[0], t0->scale - end[1], t0[1].height - end[2] };
                    float bestDist = d0[2] * d0[2] + d0[0] * d0[0] + d0[1] * d0[1];
                    for ( int v38 = 0; v38 < g_qeglobals.d_numterrapoints; ++v38 )
                    {
                        terrainVert_t *v39 = g_qeglobals.d_terrapoints[v38];
                        float dd[3] = { v39->height - end[0], v39->scale - end[1], v39[1].height - end[2] };
                        float dist = dd[2] * dd[2] + dd[0] * dd[0] + dd[1] * dd[1];
                        if ( bestDist > dist )
                        {
                            bestDist = dist;
                            best = v38;
                        }
                    }
                }

                // current radius of the nearest point from the ray (origin=centroid, dir=normal).
                float startRadius = (float)sqrt(
                    Drag_RayPointDistSq( (const float *)g_qeglobals.d_terrapoints[best],
                                         centroid, g_qeglobals.d_select_translate_unk ) );
                if ( startRadius > 0.0f )
                {
                    // radius after applying the drag `move` to the nearest point.
                    terrainVert_t *vb = g_qeglobals.d_terrapoints[best];
                    float moved[3] = { vb->height + move[0], vb->scale + move[1], vb[1].height + move[2] };
                    float endRadius = (float)sqrt(
                        Drag_RayPointDistSq( moved, centroid, g_qeglobals.d_select_translate_unk ) );
                    float ratio = ( endRadius - startRadius ) / startRadius + 1.0f;
                    if ( ratio > 0.0f )
                    {
                        if ( ratio > 5.0f )
                            ratio = 5.0f;

                        // scale the whole cluster about the centroid by `ratio`.
                        for ( int v43 = 0; v43 < g_qeglobals.d_numterrapoints; ++v43 )
                        {
                            terrainVert_t *v46 = g_qeglobals.d_terrapoints[v43];
                            float p[3] = { v46->height   - centroid[0],
                                           v46->scale    - centroid[1],
                                           v46[1].height - centroid[2] };
                            p[0] = p[0] * ratio;
                            p[1] = p[1] * ratio;
                            p[2] = p[2] * ratio;
                            v46->height   = p[0] + centroid[0];
                            v46->scale    = p[1] + centroid[1];
                            v46[1].height = p[2] + centroid[2];
                        }

                        // rebuild the selected brushes; bail to undo if any goes degenerate.
                        selbrush_t *v47 = selected_brushes.next;
                        if ( v47 != &selected_brushes )
                        {
                            while ( 1 )
                            {
                                brush_t *v48 = v47->def;
                                float pre[3] = { v48->maxs[0] - v48->mins[0],
                                                 v48->maxs[1] - v48->mins[1],
                                                 v48->maxs[2] - v48->mins[2] };
                                Brush_BuildWindings( v48, 1 );
                                if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                                    SetupVertexSelection();
                                MarkMapModified();
                                ++v48->version;

                                brush_t *v49 = v47->def;
                                int v50 = 0;
                                for ( ; v50 < 3; ++v50 )
                                {
                                    if ( v49->maxs[v50] <= (double)v49->mins[v50] )
                                        break;
                                    if ( v49->maxs[v50] - v49->mins[v50] > 262144.0 )
                                        break;
                                }
                                if ( v50 != 3 )
                                    break;

                                patch_t *v52 = v47->patch;
                                if ( v52 )
                                {
                                    float center[3] = {
                                        ( v49->maxs[0] - v49->mins[0] ) - pre[0],
                                        ( v49->maxs[1] - v49->mins[1] ) - pre[1],
                                        ( v49->maxs[2] - v49->mins[2] ) - pre[2] };
                                    iassert( v49->patch == v52->def );  // drag.cpp:800
                                    if ( !Patch_DragScale( center, v47->patch->def, move ) )
                                    {
                                        v47 = nullptr;
                                        break;
                                    }
                                }
                                v47 = v47->next;
                                if ( v47 == &selected_brushes )
                                    goto LABEL_148;
                            }

                            if ( v47 != &selected_brushes )
                            {
                                Com_PrintMessage( "MoveSelection: Attempted drag or nudge blocked because "
                                                  "it would invalidate or destroy a brush.\n" );
                                // undo: scale the cluster back about the centroid by 1/ratio.
                                float invRatio = 1.0f / ratio;
                                for ( int v53 = 0; v53 < g_qeglobals.d_numterrapoints; ++v53 )
                                {
                                    terrainVert_t *v58 = g_qeglobals.d_terrapoints[v53];
                                    float p[3] = { v58->height   - centroid[0],
                                                   v58->scale    - centroid[1],
                                                   v58[1].height - centroid[2] };
                                    p[0] = p[0] * invRatio;
                                    p[1] = p[1] * invRatio;
                                    p[2] = p[2] * invRatio;
                                    v58->height   = p[0] + centroid[0];
                                    v58->scale    = p[1] + centroid[1];
                                    v58[1].height = p[2] + centroid[2];
                                }
                                for ( selbrush_t *i1 = selected_brushes.next; i1 != &selected_brushes; i1 = i1->next )
                                {
                                    brush_t *v60 = i1->def;
                                    Brush_BuildWindings( v60, 1 );
                                    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                                        SetupVertexSelection();
                                    MarkMapModified();
                                    ++v60->version;
                                }
                            }
                        }
                    }
                }
            LABEL_148:
                ;
            }
            else if ( !v73 )
            {
                Select_Move( move, 0 );      // ← the brush-drag demo path
            }
            return;
        }

        // ── sel_vertex: per-brush vertex move ────────────────────────────────
        {
            float end[3] = { 0, 0, 0 };
            int allOk = 1;
            for ( selbrush_t *v21 = selected_brushes.next; v21 != &selected_brushes; v21 = v21->next )
                allOk &= Brush_MoveVertex( move, v21->def,
                                           g_qeglobals.d_move_points[0]->xyz, end );
            if ( allOk )
            {
                drawVert_t *mp0 = g_qeglobals.d_move_points[0];
                mp0->xyz[0] = end[0]; mp0->xyz[1] = end[1]; mp0->xyz[2] = end[2];
            }
        }
        return;
    }

ROTATE_OR_BEND:
    // ── Rotate (g_bRotateMode) / patch-bend (g_bPatchBendMode) drag ──────────────
    // IDA 0x47f0c0 LABEL_119 (0x47fbe9..0x47fee8).  Reached from `goto ROTATE_OR_BEND` at
    // the top (g_bRotateMode) and after the scale dispatch (g_bPatchBendMode).  KISAK: the
    // CString status machinery is replaced by MainFrm_SetStatusText.
    {
        // (1) Grid-snap each move component to the angle/grid step (ClampGridSize),
        //     matching the binary's float32 round-trip: move[i] = (move[i]/grid)*step.
        //     (0x47fbe9..0x47fc26 — fdiv grid_sizes[gs] then fimul (int)ClampGridSize.)
        for ( int i = 0; i < 3; ++i )
        {
            if ( move[i] != 0.0f )
            {
                move[i] = move[i] / grid_sizes[g_qeglobals.d_gridsize];
                move[i] = move[i] * (float)ClampGridSize();   // fimul: integer multiply
            }
        }
        // (2) Sub-unit grid → extra 0.1 scale (0x47fc28..0x47fc61; runs when grid<=1).
        //     dbl_6F4260 is the QWORD constant 0.10000000149011612 (= (double)0.1f); the
        //     binary's x87 fmul promotes move[j] to 80-bit, so multiply in double then
        //     truncate to float32 (matches the fld qword / fmul / fstp dword sequence).
        if ( grid_sizes[g_qeglobals.d_gridsize] <= 1.0f )
        {
            for ( int j = 0; j < 3; ++j )
                if ( move[j] != 0.0f )
                    move[j] = (float)( (double)move[j] * 0.10000000149011612 );   // dbl_6F4260
        }

        // (3) Pick the rotation axis + the signed angle from the active XY view type
        //     (0x47fc63..0x47fcbf). m_nViewType: 2=XY(default), 1=XZ, 0=YZ.
        //       rotAxisAngle (= -move[axis], hex-rays v75) is the rotation amount;
        //       rotationDelta (= +move[axis], hex-rays mm) accumulates into g_vRotation.
        int   axisIdx       = 0;
        float rotAxisAngle  = -move[2];
        float rotationDelta =  move[2];
        CXYWnd *activeXY = g_pParentWnd->m_pActiveXY;
        if ( activeXY->m_nViewType == 2 )        // XY (top) → rotate about Z
        {
            rotAxisAngle  = -move[1];
            rotationDelta =  move[1];
            axisIdx       =  2;
        }
        else if ( activeXY->m_nViewType == 1 )   // XZ (front) → rotate about Y
        {
            rotAxisAngle  =  move[2];
            rotationDelta =  move[2];
            axisIdx       =  1;
        }
        // else YZ (side) → axisIdx 0 (rotate about X), rotAxisAngle = -move[2].

        // (4) Accumulate the rotation total + status text (0x47fcc1..0x47fd49).
        g_vRotation[axisIdx] += rotationDelta;
        {
            char statusRot[160];
            const char *label = g_bPatchBendMode ? "Bend angle" : "Rotation";
            _snprintf( statusRot, sizeof( statusRot ),
                       "%s x:: %.1f  y:: %.1f  z:: %.1f",
                       label, g_vRotation[0], g_vRotation[1], g_vRotation[2] );
            MainFrm_SetStatusText( 2, statusRot );
        }

        // (5) Apply the rotation. rot_around[0] = pivot origin (g_vRotateOrigin),
        //     rot_around[1..3] = the 3x3 built by Select_RotateAxis. The buffer is
        //     [4][3] (12 floats) per the established caller pattern (mainfrm.cpp).
        float rot_around[4][3];
        if ( g_bPatchBendMode )
        {
            // Patch-bend: bend the patch about its NORMAL (double angle) then its
            // AXIS (single angle). The bend selectors pick which half/row to move.
            Patch_SelectBendNormal();             // 0x447d30
            float bendNormalAngle = rotAxisAngle + rotAxisAngle;   // 2*v75
            if ( bendNormalAngle != 0.0f )
            {
                rot_around[0][0] = g_vRotateOrigin[0];
                rot_around[0][1] = g_vRotateOrigin[1];
                rot_around[0][2] = g_vRotateOrigin[2];
                Select_RotateAxis( axisIdx, bendNormalAngle, (float (*)[4][3])rot_around );
                for ( selbrush_t *k = selected_brushes.next; k != &selected_brushes; k = k->next )
                    Select_ApplyMatrix( rot_around[0], k, 0, bendNormalAngle, 0 );
            }
            Patch_SelectBendAxis();               // 0x447cc0
            if ( rotAxisAngle != 0.0f )
            {
                rot_around[0][0] = g_vRotateOrigin[0];
                rot_around[0][1] = g_vRotateOrigin[1];
                rot_around[0][2] = g_vRotateOrigin[2];
                Select_RotateAxis( axisIdx, rotAxisAngle, (float (*)[4][3])rot_around );
                for ( selbrush_t *m = selected_brushes.next; m != &selected_brushes; m = m->next )
                    Select_ApplyMatrix( rot_around[0], m, 0, rotAxisAngle, 0 );
            }
        }
        else if ( rotAxisAngle != 0.0f )
        {
            // Free rotate: rotate every selected brush about the pivot by rotAxisAngle.
            rot_around[0][0] = g_vRotateOrigin[0];
            rot_around[0][1] = g_vRotateOrigin[1];
            rot_around[0][2] = g_vRotateOrigin[2];
            Select_RotateAxis( axisIdx, rotAxisAngle, (float (*)[4][3])rot_around );
            for ( selbrush_t *n = selected_brushes.next; n != &selected_brushes; n = n->next )
                Select_ApplyMatrix( rot_around[0], n, 0, rotAxisAngle, 0 );
        }
    }
    return;
}

// ─── 0x47ff30  Drag_MouseMoved (a mouse-move during a press) ─────────────────
// Drag_IsPointEditMode(m) = (m >= sel_editpoint(10) && m <= 15) EXACTLY equals the binary's
// explicit exclusion chains (the OR'd-enum lists {13,15,11,12,14,10} = [10,15]).
// Converts the pixel delta since the press into a grid-snapped world move and
// hands it to MoveSelection. a1/a2 are the CURRENT pixel coords (CXYWnd passes
// raw pixels — see XY_MouseMoved 0x468230); a4/a5 are the ray origin/dir (only
// used by the addpoint path).  The move basis g_drag_xvec/yvec is the AXIALISED (unit)
// view basis, so the move is in pixel units and then grid-snapped; m_fScale is applied
// only in rotate/bend mode.
void Drag_MouseMoved( int a1, int a2, int buttons, float *a4, float *a5 )
{
    if ( !buttons )
    {
        drag_ok = 0;
        g_qeglobals.toggle_unk03_mousedrag_state1 = 0;
        g_qeglobals.toggle_unk04_mousedrag_state2 = 0;
        return;
    }

    const select_t d_select_mode = g_qeglobals.d_select_mode;
    unsigned int v5 = buttons;

    // Shift held: constrain the drag to the dominant pixel axis.
    if ( buttons == MK_SHIFT && !Drag_IsPointEditMode( g_qeglobals.d_select_mode ) )
    {
        drag_first = 0;
        v5 = MK_SHIFT;
        if ( abs32( a1 - g_pressx_real ) <= abs32( a2 - g_pressy_real ) )
            a1 = g_pressx_real;
        else
            a2 = g_pressy_real;
    }

    if ( g_qeglobals.toggle_unk03_mousedrag_state1 || g_qeglobals.toggle_unk04_mousedrag_state2 )
    {
        if ( g_qeglobals.d_select_mode != sel_curvepoint
             && !Drag_IsPointEditMode( g_qeglobals.d_select_mode ) )
        {
            switch ( v5 )
            {
                case MK_LBUTTON | MK_SHIFT:                       return;
                case MK_LBUTTON | MK_CONTROL:
                    if ( GetAsyncKeyState( VK_MENU ) < 0 )        return;
                    break;
                case MK_LBUTTON | MK_SHIFT | MK_CONTROL:         return;
            }
        }
        g_qeglobals.toggle_unk03_mousedrag_state1 = 0;
        g_qeglobals.toggle_unk04_mousedrag_state2 = 0;
        return;
    }

    if ( !drag_ok )
        return;

    // (v5 & MK_CONTROL) → re-apply the shift axis-lock for the control-drag case.
    if ( ( v5 & 4 ) != 0 && !Drag_IsPointEditMode( g_qeglobals.d_select_mode ) )
    {
        v5 = buttons;
        drag_first = 0;
        if ( abs32( a1 - g_pressx_real ) <= abs32( a2 - g_pressy_real ) )
            a1 = g_pressx_real;
        else
            a2 = g_pressy_real;
    }

    if ( d_select_mode == sel_addpoint )
    {
        if ( GetAsyncKeyState( VK_MENU ) < 0 )
        {
            sub_43E6F0( v5, (int)a4, (int)a5 );
        }
        else
        {
            select_t prev = g_qeglobals.d_select_mode;
            g_qeglobals.d_select_mode = sel_brush;
            if ( prev == sel_cycle_edge_direction_quad )
                CMainFrame_UpdatePatchToolbarButtons();
            else if ( prev == sel_addpoint )
                sub_43ECB0();
            drag_ok = 0;
        }
        return;
    }

    // ── the drag: pixel delta → snapped world move → MoveSelection ───────────
    const bool rotate = g_bRotateMode;
    float dx = (float)( a1 - g_pressx_real );
    float dy = (float)( a2 - g_pressy_real );
    float drag_vec[3];

    for ( int i = 0; i < 3; ++i )
    {
        float w = g_drag_xvec[i] * dx + g_drag_yvec[i] * dy;
        drag_vec[i] = w;

        if ( rotate || g_bPatchBendMode )
        {
            w *= g_pParentWnd->m_pActiveXY->m_fScale;     // scale only in rotate/bend
            drag_vec[i] = w;
            int c = ClampGridSize();
            if ( c == 1 )      drag_vec[i] = w * 0.5f;
            else if ( c == 2 ) drag_vec[i] = w * 0.75f;
        }

        if ( selected_brushes.next != &selected_brushes
             && selected_brushes.next->patch && rotate && ClampGridSize() == 1 )
        {
            drag_vec[i] *= 0.5f;
        }
        else if ( !g_PrefsDlg->m_bNoClamp )   // m_bNoClamp → grid snap when off (default 0)
        {
            float gs = grid_sizes[g_qeglobals.d_gridsize];
            drag_vec[i] = (float)floor( 0.5 + drag_vec[i] / gs ) * gs;
        }
    }

    float move[3];
    move[0] = drag_vec[0] - g_pressdelta[0];
    move[1] = drag_vec[1] - g_pressdelta[1];
    move[2] = drag_vec[2] - g_pressdelta[2];
    g_pressdelta[0] = drag_vec[0];
    g_pressdelta[1] = drag_vec[1];
    g_pressdelta[2] = drag_vec[2];

    if ( d_select_mode == sel_area || Drag_IsPointEditMode( d_select_mode ) )
    {
        g_qeglobals.drag_selectionbox_y_1 = a1;
        g_qeglobals.drag_selectionbox_y_2 = a2;
    }
    else
    {
        MoveSelection( a4, a5, move );
    }
}

// ── marquee box-select wiring (select.cpp / xywnd.cpp) ───────────────────────
extern void Select_FlipFilteredBrushes( const float *boxMins, const float *boxMaxs, char bActiveList );
extern void Ed_XY_ToPoint( CXYWnd *wnd, int x, int y, float *start, float *dir );

// ── sel_area control-point rect-select (pmesh.cpp 0x448460 / 0x448620) ────────
extern void Terrain_SelectAreaPoints( const void *planes, char select );
extern void Patch_SelectAreaPoints( const void *planes, char select );

// ─── 0x4718d0  Select_RemoveEmptyFaces (post vertex/edge-drag cleanup) ────────
// For every selected brush: drop any face whose winding went null during the drag
// (Brush_RemoveFace shifts the array down, so don't advance the index after a
// removal), then either bump the def version (>=4 faces survive → still a valid
// brush) or free the brush outright (<4 faces → collapsed). Called by Drag_MouseUp
// when the closed drag was a vertex/edge edit (sel_vertex/sel_edge). IDA reads the
// DEF faceCount (the authoritative count), matching the binary's def->faceCount.
// The binary's dual indices v2 (RemoveFace arg + bound) and v3 (winding test) ALWAYS stay
// equal, so the port's single running index `i` is exact.
static void Select_RemoveEmptyFaces()
{
    selbrush_t *sb = selected_brushes.next;
    while ( sb != &selected_brushes )
    {
        selbrush_t *next = sb->next;
        brush_t    *def  = sb->def;

        // Remove null-winding faces. IDA keeps a single running index `i` (its v2/v3
        // stay equal: both step on a kept face, both hold on a removed one).
        for ( unsigned int i = 0; (int)i < def->faceCount; )
        {
            if ( def->faces[i].w )
                ++i;
            else
                Brush_RemoveFace( def, i );   // shifts down → re-test the same index
        }

        if ( sb->def->faceCount >= 4 )
            ++sb->def->version;
        else
            Brush_Free( sb );

        sb = next;
    }
}

// ─── 0x4802a0  Drag_MouseUp (end of a press) ─────────────────────────────────
// Box-select min/max + the Select_FlipFilteredBrushes flag verified NOT inverted (the mode-A
// click branch = sel_areabrush_sub(13) -> flip(0); 12 -> flip(1)).
// Resets the drag toggles, performs the MARQUEE box-select if one was active, and
// closes the undo record opened by Drag_Setup/Drag_Begin (Undo_End/Undo_EndBrushList
// self-guard when no record is open, e.g. a shift+click select).
//
// Selection modes set by Drag_Setup's box-drag branch:
//   sel_areabrush     (12)  Alt+Shift+LMB drag → box ADD-select   (active → selected)
//   sel_areabrush_sub (13)  Alt+Shift+MMB drag → box DESELECT     (selected → active),
//                            or a single click-select via SelectFaceSth if barely moved.
// The world box is built from the drag rectangle (press corner = x_1,x_2; current
// corner = y_1,y_2) via XY_ToPoint per the IDB (0x480484..0x48057b).
//
// sel_area (5) is the 3D camera-view point rect-select (terrain/patch CONTROL POINTS):
// build the view frustum from the drag rect via the press-callback (g_qeglobals.camera_fov_setup,
// installed = Camera_GetRectSelection3D by Drag_Begin for a camera-view area drag), then add the
// covered control points to the move-point sets (Terrain/Patch_SelectAreaPoints).  After it the
// binary switches the tool to curve-point mode (4) and re-reads d_select_mode for the
// UpdatePatchToolbarButtons (mode 9) / Patch_FinishCurveDrag (mode 8) tail — both inert when the
// entry mode was sel_area (5), but transcribed verbatim.  Editpoint siblings 11/14/15 = point edit.
void Drag_MouseUp( unsigned int buttons )
{
    g_qeglobals.toggle_unk03_mousedrag_state1 = 0;
    g_qeglobals.toggle_unk04_mousedrag_state2 = 0;
    drag_ok = 0;

    // IDA 0x4802cc: status-bar pane 0 = "drag completed." (the prior port dropped this).
    // SendMessageA + d_hwndStatus is the established status pattern here — cf. Drag_Setup
    // drag.cpp:354 and the brush.cpp/select.cpp/win_dlg.cpp sites.
    SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"drag completed." );

    const select_t mode = g_qeglobals.d_select_mode;

    // ── sel_area (5): camera-view rect-select of terrain/patch CONTROL POINTS ──
    // IDA 0x4802e8..0x480391.  Build the 4-plane view frustum from the drag rect via the
    // installed press callback (g_qeglobals.camera_fov_setup == Camera_GetRectSelection3D),
    // then collect the covered control points.  `(buttons>>3)&1` (the cleaned MK flag) =
    // the ADD-vs-fresh select arg.
    if ( mode == sel_area )
    {
        if ( g_qeglobals.camera_fov_setup )
        {
            int planeBuf[32];   // 4 planes × 32 bytes (v25[32] in the IDB)
            ( (void (__cdecl *)( int, int, int, int, int * ))g_qeglobals.camera_fov_setup )(
                g_qeglobals.drag_selectionbox_x_1, g_qeglobals.drag_selectionbox_x_2,
                g_qeglobals.drag_selectionbox_y_1, g_qeglobals.drag_selectionbox_y_2, planeBuf );

            char select = (char)( ( buttons >> 3 ) & 1 );
            Terrain_SelectAreaPoints( planeBuf, select );
            Patch_SelectAreaPoints( planeBuf, select );

            const select_t v2 = g_qeglobals.d_select_mode;   // re-read (unchanged by the above)
            g_qeglobals.d_select_mode = sel_curvepoint;      // switch to curve-point edit (4)
            if ( v2 == sel_cycle_edge_direction_quad )       // 9
            {
                CMainFrame_UpdatePatchToolbarButtons();
            }
            else if ( v2 == sel_addpoint )                   // 8
            {
                sub_43ECB0();                                // Patch_FinishCurveDrag
            }
            g_nUpdateBits = -1;
        }
        // (mode now sel_curvepoint(4), so the marquee branch below is skipped and the
        //  shared LABEL_41 translate/undo-close tail runs — matching the binary's fall-through.)
    }
    // ── sel_areabrush / sel_areabrush_sub (12/13): the BRUSH marquee box-select ─
    // IDA gates the whole marquee/click dispatch on g_nPatchClickedView != W_CAMERA
    // (0x48039c: a camera-view press skips straight to the undo-close at LABEL_41).
    // Drag_Setup only ARMS modes 12/13 when g_nPatchClickedView != 1, so the two are
    // mutually exclusive in practice, but mirror the binary's guard for faithfulness.
    else if ( ( mode == sel_areabrush || mode == sel_areabrush_sub )
              && g_nPatchClickedView != W_CAMERA )
    {
        CXYWnd *xy = g_pParentWnd ? g_pParentWnd->m_pActiveXY : nullptr;

        // mode 13 (box deselect) with a sub-pixel drag = a click → single face/brush
        // pick via SelectFaceSth (IDA 0x4803a5..0x48047a).
        bool didClick = false;
        if ( mode == sel_areabrush_sub )
        {
            float dist = sqrtf( g_pressdelta[0]*g_pressdelta[0]
                              + g_pressdelta[1]*g_pressdelta[1]
                              + g_pressdelta[2]*g_pressdelta[2] );
            if ( dist < 1.0f )
            {
                float start[3] = { 0.0f, 0.0f, 0.0f };
                float dir[3];
                if ( xy )
                    Ed_XY_ToPoint( xy, g_qeglobals.drag_selectionbox_x_1,
                                       g_qeglobals.drag_selectionbox_x_2, start, dir );
                // Alt+LMB(==4) selects a single FACE (flag 4); else the whole brush (64).
                int flag = 64;
                if ( buttons == 4 && GetAsyncKeyState( VK_MENU ) < 0
                     && mode != sel_curvepoint && mode != sel_terrainpoint
                     && mode != sel_terraintexture )
                    flag = 4;
                SelectFaceSth( (int)dir, (int)start, flag );
                g_qeglobals.d_select_mode = sel_brush;
                g_nUpdateBits = -1;
                didClick = true;
            }
        }

        if ( !didClick )
        {
            // Build the world box from the two snapped corners (only the .start of
            // XY_ToPoint is used; the depth axis carries the 131072 ray offset which
            // is harmless for a "Tall" two-axis bounds test).
            float c1[3] = { 0.0f, 0.0f, 0.0f };   // press corner (x_1, x_2)
            float c2[3] = { 0.0f, 0.0f, 0.0f };   // current corner (y_1, y_2)
            float dir[3];
            if ( xy )
            {
                Ed_XY_ToPoint( xy, g_qeglobals.drag_selectionbox_x_1,
                                   g_qeglobals.drag_selectionbox_x_2, c1, dir );
                Ed_XY_ToPoint( xy, g_qeglobals.drag_selectionbox_y_1,
                                   g_qeglobals.drag_selectionbox_y_2, c2, dir );
            }
            float boxMins[3], boxMaxs[3];
            for ( int i = 0; i < 3; ++i )
            {
                boxMins[i] = ( c1[i] <= c2[i] ) ? c1[i] : c2[i];
                boxMaxs[i] = ( c1[i] >= c2[i] ) ? c1[i] : c2[i];
            }
            // mode 12 → walk active list (ADD); mode 13 → walk selected list (DESELECT).
            Select_FlipFilteredBrushes( boxMins, boxMaxs, mode == sel_areabrush ? 1 : 0 );
            g_qeglobals.d_select_mode = sel_brush;
            g_nUpdateBits = -1;
        }
    }
    // ── point-edit rectangle siblings 14/15: close the rect back to sel_vertex ──
    // IDA 0x4805a6: the `else if (d_select_mode != 12)` arm tests 14/15 and does
    // `d_select_mode = 1; g_nUpdateBits = -1;` before falling into LABEL_41.  Same
    // g_nPatchClickedView != W_CAMERA gate as the marquee arm above (0x48039c).
    else if ( ( mode == sel_areapoint_curve || mode == sel_areapoint )
              && g_nPatchClickedView != W_CAMERA )
    {
        g_qeglobals.d_select_mode = sel_vertex;   // 0x4805a8: mode = 1
        g_nUpdateBits = -1;
    }

    // ── LABEL_41: pending nudge-translate, then close the undo record ──────────
    // IDA 0x4805db: if a translate was queued (Select_Move w/ snap), apply it and
    // clear it. d_select_translate is written by the keyboard-nudge path; it is {0,0,0}
    // until that lands, so this is currently a no-op, but transcribe it verbatim.
    if ( g_qeglobals.d_select_translate[0] != 0.0f
      || g_qeglobals.d_select_translate[1] != 0.0f
      || g_qeglobals.d_select_translate[2] != 0.0f )
    {
        Select_Move( g_qeglobals.d_select_translate, W_CAMERA );
        g_qeglobals.d_select_translate[0] = 0.0f;
        g_qeglobals.d_select_translate[1] = 0.0f;
        g_qeglobals.d_select_translate[2] = 0.0f;
        g_nUpdateBits |= W_CAMERA;
    }

    // IDA 0x480627 here: get_m_strStatus(&m_strStatus[3], "") + CMainFrame::UpdateStatusText
    // (clears status pane 3 + refresh). The port DECOUPLED the MFC m_strStatus mechanism (see
    // win_qe3.cpp:192) and drives status panes via SendMessageA instead; UpdateStatusText is
    // unported, so this pane-3 clear is intentionally omitted (cosmetic, decoupled).

    if ( g_qeglobals.d_select_mode == sel_addpoint )
    {
        g_qeglobals.d_select_mode = sel_brush;
        sub_43ECB0();
    }
    else
    {
        // A vertex/edge drag can collapse a face to a null winding — IDA 0x48064d calls
        // sub_4718D0 (Select_RemoveEmptyFaces) to drop empties / free sub-4-face brushes
        // before closing the undo record.
        if ( g_qeglobals.d_select_mode == sel_edge || g_qeglobals.d_select_mode == sel_vertex )
            Select_RemoveEmptyFaces();
        Undo_EndBrushList( &selected_brushes );
        Undo_End();
    }
}
