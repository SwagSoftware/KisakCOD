#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\pmesh.cpp
// ═════════════════════════════════════════════════════════════════════════════
//  PATCH MESH (curve/terrain) subsystem: parse, write, tessellate, edit and draw.
//
//  IDENTIFICATION CORRECTIONS (the enthusiast `PMESH_nn` names were positional guesses
//  over the 20556-byte patchMesh_t; corrected here + renamed in the IDB):
//    0x444ac0  PMESH_TexLayer     -> Patch_ParseMesh  (block parser: `mesh{...}` ->
//                                   patchMesh_t + symbiont brush; NOT a "tex layer")
//    0x4446f0  Patch_Parse        -> Patch_ParseVert  (ONE control vertex; assert
//                                   string "patchVert")
//    0x4458f0  MapLoad_ParsePatch -> Patch_Write      (block writer, NOT a load fn)
//    0x4456f0  PMESH_WriteToMap   -> Patch_WriteVert  (writes ONE control vertex)
//    0x4449e0  sub_4449E0         -> Patch_CalcVertColors
//
//  Round-trip data flow:
//    LOAD  Brush_Parse -> Patch_ParseMesh -> MakeNewPatch + Patch_ParseVert(xN) +
//          AddBrushForPatch (symbiont brush_t, brush->patch = patch).  ParseEntity links
//          the brush into the entity's def-list.
//    SAVE  Brush_Write -> (brush->patch) -> Patch_Write -> Patch_WriteVert(xN).
//  The writer reads ONLY the control grid / materials / params from patchMesh_t, never
//  the subdivided render mesh (curveDef) - so the tessellation is display-only and never
//  affects the round-trip.
//
//  CoD's .map uses `mesh`/`curve` blocks with a per-vertex
//  `v xyz [c rgba] t s0 t0 s1 t1 [f edge]` grammar - NOT GtkRadiant's patchDef2/patchDef3,
//  so the per-byte format comes from the IDA decompilation, not GtkRadiant.
// ═════════════════════════════════════════════════════════════════════════════

#include "stdafx.h"
#include "qe3.h"
#include <universal/q_parse.h>
#include <gfx_d3d/r_gfx.h>          // GfxPointVertex, GfxColor, GfxStateBits (terrain-paint ring + patch-fill)
#include <gfx_d3d/r_material.h>     // Material stateBitsEntry/stateBitsTable (patch-fill opaque gate)
#include <gfx_d3d/r_state.h>        // GFXS1_DEPTHWRITE
#include <gfx_d3d/r_rendercmds.h>   // GfxCmdDrawPoints (R_AddPointCmd_W return)
#include <stdlib.h>   // atof, atol, atoi, free
#include <string.h>
#include <stdio.h>    // sprintf

#define PMESH_CPP "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\PMESH.CPP"

// ─── editor / engine externs (ported elsewhere) ──────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern void  Error( const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );   // win_qe3.cpp (0x499E90)
extern long  j__atol( const char *s );
extern void  Vec3Cross( const float *a, const float *b, float *out );   // engine math (0x40a4d0)

// materialdef.cpp
extern void        SetMaterial( const char *name, patchMesh_material *out );
extern qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *def );
namespace LayerMat { int GetCurrentLayer( MaterialDef *def ); }

// brush.cpp
extern brush_t *Brush_Alloc( const void *materialDefSrc, eclass_t *ecls );
extern void     Brush_Create( float *mins, float *maxs, brush_t *b, eclass_t *ecls );
extern void     Brush_BuildWindings( brush_t *b, int rebuild );
extern void     SetupVertexSelection();
extern int      TexWnd_06_LayerCount( int mtlDef, int layerHandle );   // brush.cpp 0x45D360
extern void     Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                                  int uvBase, float rotate, float crossterm ); // brush.cpp 0x45A1C0

// draw.cpp (q_shared 0x4BA430) — out = orient->origin + orient->axisᵀ · pos.
extern void     OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient );
// engine_stubs.cpp (0x25d5b04) — patch bend-mode active flag (off by default).
extern int      g_bPatchBendMode;

// ── renderer / math deps for the terrain-paint cursor RING (PMESH_19/20 + sub_43ED50) ──
extern float    world_orient_matrix[4][3];                     // entity.cpp 0x6DE290 (identity world orient)
extern char     Byte4PackPixelColor( float *from, GfxColor *out );                  // 0x402AC0
extern int      R_Add3DLine( GfxPointVertex *verts, const orientation_t *orient,
                             const float *p1, const float *p2, const unsigned int *color,
                             char width, int vertCount, int maxVertCount );          // 0x40C110
extern void     R_AddCmd_Line3D( short count, char width, GfxPointVertex *verts );   // 0x4FD1A0
extern GfxCmdDrawPoints *R_AddPointCmd_W( short pointCount, char size, const GfxPointVertex *verts ); // 0x4FD080

// entity.cpp / map.cpp
extern void     Entity_LinkBrush( brush_t *b, entity_s *e );   // 0x484FC0 (def-list linker)
extern void     MarkMapModified();

// ─── PATCH CREATE + GRID MANIPULATION deps ───────────────────────────────────
// select.cpp — single-brush precondition + (de)select / delete the selection.
extern int        QE_SingleBrush();                              // qe3.cpp (0x48C8B0)
extern selbrush_t *Brush_AddToList( brush_t *def, entity_s *owner ); // brush.cpp (0x475980)
extern void       Brush_AddToList2( selbrush_t *b );             // brush.cpp (0x4765A0)
extern void       Select_Delete();                              // select.cpp (0x48E760)
extern void       Select_Brush( selbrush_t *b, char overwrite, char status, char center ); // select.cpp (0x48DCC0)
// brush.cpp — rebuild a patch symbiont's 6-face bbox brush from new bounds (0x438760).
extern void       Brush_RebuildBrush( brush_t *b, float *mins, float *maxs );
// the global brush-instance display lists (engine_stubs).
extern selbrush_t active_brushes;                                // 0x23F189C
extern selbrush_t selected_brushes;                              // 0x23F1864

// engine_stubs.cpp (mapparsing.cpp ports) — `contents <names>;` / `toolFlags <names>;`
extern int      sub_42FB80( const char **text );   // reads a "contents"  line → bits
extern int      sub_42FBA0( const char **text );   // reads a "toolFlags" line → bits
// layers.cpp — optional `<keyword> "value"` (or unquoted) line, else default.
extern void     Map_ParseEntityLayerKey( const char **text, const char *def,
                                          char *out, const char *key );
// engine_stubs.cpp — contents / toolFlags flag-name tables (for the writer).
extern int     *contents_table;
extern int     *toolflags_table;
extern void     MapLoad_ParseBrush_Content( int keyword, int (**writer)(int, const char *, ...),
                                            int value, void *table );
extern void     MapLoad_ParseBrush_Layer( int (**writer)(int, const char *, ...),
                                          int layerStr );   // engine_stubs 0x42fb40

// qtexture_s is the canonical 40-byte IDB layout in qe3.h (was a wrong local
// char[64]@0 copy; width@0x14 / height@0x18).

// ─── forward declarations (this file) ────────────────────────────────────────
patchMesh_t *MakeNewPatch();
void         Patch_CalcBounds( float *mins, float *maxs, patchMesh_t *p, bool expand );
brush_t     *AddBrushForPatch( patchMesh_t *p, entity_s *world_ent );
static char  Patch_ParseVert( const char **text, drawVert_t *v );
static int   Patch_CalcVertColors( patchMesh_t *p );
static curvePatchDef_t *Patch_TerrainTexProject( patchMesh_t *p, int layer, float sampleSize ); // sub_439350
static void  PMESH_02( patchMesh_t *p, int layer, float sampleSize );                            // 0x439580

// Patch-inspector dialog refresh hooks (patchdialog.cpp SHIPPED — GetHwnd() returns the
// live modeless CPatchInspectorDlg when open, null when it is not / headless; comment
// below predates that port and is kept for the headless-behaviour note. GetHwnd() returns
// null headless; GetPatchInfo() is a no-op).  Used by Patch_Move's GUI-guard tail.
extern CWnd *g_PatchDialog_GetHwnd();
extern void  g_PatchDialog_GetPatchInfo();

// ════════════════════════════════════════════════════════════════════════════
//  MakeNewPatch  (0x437ac0)
//  Allocates a zeroed patchMesh_t and seeds default materials + white vertices.
// ════════════════════════════════════════════════════════════════════════════
patchMesh_t *MakeNewPatch()
{
    patchMesh_t *p = (patchMesh_t *)operator new( sizeof( patchMesh_t ) );   // 0x504C
    *(float *)&p->size_of_struct_0x504C = 0.0f;
    p->width      = 0;
    p->height     = 0;
    p->contents   = 0;
    p->type       = (PATCH_TYPES)0;
    p->subDivType = 8;
    SetMaterial( "$default",         &p->texture );
    SetMaterial( "lightmap_gray",    &p->lightmap );
    SetMaterial( "smoothing_smooth", &p->smoothing );
    memset( p->ctrl, 0, sizeof( p->ctrl ) );
    for ( int w = 0; w < 16; ++w )
        for ( int h = 0; h < 16; ++h )
            *(unsigned int *)&p->ctrl[w][h].vert_color = 0xFFFFFFFF;   // white
    p->curveDef = nullptr;
    p->pSymbiot = nullptr;
    p->version  = 0;
    p->xx22b    = false;
    return p;
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_CalcBounds  (0x4385e0)
//  Computes the AABB of the control points; optionally expands degenerate axes
//  (< 8 units) to a centred 8-unit span so the symbiont bbox brush is non-empty.
//  IDA calls VectorMaxValues per vertex (a fatal stub in this build) — inlined.
// ════════════════════════════════════════════════════════════════════════════
void Patch_CalcBounds( float *mins, float *maxs, patchMesh_t *p, bool expand )
{
    ++p->version;
    mins[0] = mins[1] = mins[2] =  131072.0f;
    maxs[0] = maxs[1] = maxs[2] = -131072.0f;

    for ( int w = 0; w < p->width; ++w )
    {
        for ( int h = 0; h < p->height; ++h )
        {
            const float *xyz = p->ctrl[w][h].xyz;
            for ( int k = 0; k < 3; ++k )
            {
                if ( xyz[k] < mins[k] ) mins[k] = xyz[k];
                if ( xyz[k] > maxs[k] ) maxs[k] = xyz[k];
            }
        }
    }

    if ( expand )
    {
        for ( int k = 0; k < 3; ++k )
        {
            if ( maxs[k] - mins[k] < 8.0f )
            {
                float c = ( mins[k] + maxs[k] ) * 0.5f - 4.0f;
                mins[k] = c;
                maxs[k] = c + 8.0f;
            }
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  PATCH TESSELLATION (the Bezier subdivision that produces curveDef, the render
//  mesh).  Ported fresh from IW3xRadiant.i64 for STAGE-1 patch WIREFRAME render —
//  see the block at DrawPatchesWireframeGrid.  All structs (curveVert_t /
//  curvePatchDef_t / patchMesh_t) are editor-only (NOT shared engine structs), so
//  this is radiant-only.  The chain:
//      Patch_GenericMesh2 (0x438b60)  control grid → flat curveVert array, then:
//        TERRAIN  → copy verts as-is, compute normals (Curve_ComputeNormals)
//        BEZIER   → Curve_SubdivideSetup (subdivide both axes via de Casteljau)
//                   → Curve_MidpointFixup → Curve_ComputeNormals
//                   → Curve_RemoveDegenerate (drop collapsed rows/cols)
//  The S/T fields are filled but UNUSED by the wireframe path (geometry only); the
//  curveDef-dependent texCoord REFINEMENT (PMESH_02 / sub_439350) stays PARKED — it
//  affects only texturing, not the wireframe grid.  Math transcribed verbatim from
//  the disasm (float-exact; the de Casteljau midpoints are plain (a+b)*0.5).
// ════════════════════════════════════════════════════════════════════════════

// The binary subdivides into a fixed 512x512 curveVert scratch grid in .data
// (unk_181F520 .. 0x231F520 = 512*512*44).  We mirror that as a single static grid;
// the editor is single-threaded and tessellation is non-reentrant.  Row stride is
// MAX_CURVE_COLS verts; the binary's `unk_1824D20` alias is simply row 1.
static const int CURVE_GRID_DIM = 512;
static curveVert_t s_curveScratch[CURVE_GRID_DIM][CURVE_GRID_DIM];
// row 0 / row 1 aliases (the binary uses both as separate base symbols).
#define CURVE_ROW0 ( &s_curveScratch[0][0] )
#define CURVE_ROW1 ( &s_curveScratch[1][0] )

// ─── sub_4a47d0 — normalize `in` into `out`, return original length (0-guarded). ─
static float Curve_NormalizeTo( float *out, const float *in )
{
    float len = (float)sqrt( in[0] * in[0] + in[1] * in[1] + in[2] * in[2] );
    float d   = len;
    if ( -d >= 0.0f )                       // len <= 0
        d = 1.0f;
    float inv = 1.0f / d;
    out[0] = in[0] * inv;
    out[1] = inv * in[1];
    out[2] = inv * in[2];
    return len;
}

// ─── sub_4a54d0 — project point `p` onto the line through `base` toward `dir`,
//     store the projected point in `out`.  (used by Curve_RemoveDegenerate.)
//     a1=p, a2=out, a3=base, a4=dir.
static void Curve_ProjectOntoLine( const float *p, float *out, const float *base, const float *dir )
{
    float e0 = p[0] - base[0];
    float e1 = p[1] - base[1];
    float e2 = p[2] - base[2];
    float lenSq = e2 * e2 + e0 * e0 + e1 * e1;
    if ( lenSq == 0.0f )
    {
        out[0] = base[0];
        out[1] = base[1];
        out[2] = base[2];
    }
    else
    {
        float d0 = dir[0] - base[0];
        float d1 = dir[1] - base[1];
        float d2 = dir[2] - base[2];
        float dot = d0 * e0 + d1 * e1 + d2 * e2;
        float t   = dot / lenSq;
        out[0] = e0 * t + base[0];
        out[1] = e1 * t + base[1];
        out[2] = t  * e2 + base[2];
    }
}

// ─── sub_431f40 — de Casteljau midpoint: out = (a + b) * 0.5 (7 floats + 4 colour
//     bytes averaged with >>1).  a2=a, a3=b in the IDB (result = a1).
static void Curve_LerpHalf( curveVert_t *out, const curveVert_t *a, const curveVert_t *b )
{
    float       *o  = (float *)out;
    const float *fa = (const float *)a;
    const float *fb = (const float *)b;
    for ( int i = 0; i < 7; ++i )           // xyz(3) + st(2) + lightmap(2)
        o[i] = ( fb[i] + fa[i] ) * 0.5f;
    unsigned char *oc = (unsigned char *)&out->vert_color;
    const unsigned char *ac = (const unsigned char *)&a->vert_color;
    const unsigned char *bc = (const unsigned char *)&b->vert_color;
    oc[0] = (unsigned char)( ( ac[0] + bc[0] ) >> 1 );
    oc[1] = (unsigned char)( ( ac[1] + bc[1] ) >> 1 );
    oc[2] = (unsigned char)( ( ac[2] + bc[2] ) >> 1 );
    oc[3] = (unsigned char)( ( ac[3] + bc[3] ) >> 1 );
}

// ─── sub_432b20 — adaptive subdivision along COLUMNS (insert midpoint columns
//     until each curve segment's flatness is within tolerance).  Operates on the
//     scratch grid's row 0 (unk_181F520).  a1=&{cols,rows}, a2=indexMap (or null),
//     a4=subdiv tolerance, a5=999 chord limit.
static void Curve_SubdivideCols( int *dims, int *indexMap, int col0, float tol, float chordMax )
{
    while ( dims[0] + 2 < CURVE_GRID_DIM )
    {
        int allFlat = 0;
        if ( dims[1] > 0 )
        {
            // walk each row's three-point segment starting at (col0, row).
            curveVert_t *row = &s_curveScratch[0][col0];
            int r = 0;
            for ( ; r < dims[1]; ++r )
            {
                const float *p0 = row[r * CURVE_GRID_DIM + 0].xyz;
                const float *p1 = row[r * CURVE_GRID_DIM + 1].xyz;
                const float *p2 = row[r * CURVE_GRID_DIM + 2].xyz;
                float a0 = p1[0] - p0[0], a1f = p1[1] - p0[1], a2f = p1[2] - p0[2];
                if ( chordMax < (double)sqrt( a1f * a1f + a0 * a0 + a2f * a2f ) ) break;
                float b0 = p2[0] - p1[0], b1 = p2[1] - p1[1], b2 = p2[2] - p1[2];
                if ( chordMax < (double)sqrt( b1 * b1 + b0 * b0 + b2 * b2 ) ) break;
                float c0 = b0 - a0, c1 = b1 - a1f, c2 = b2 - a2f;
                float flat = (float)sqrt( c1 * c1 + c0 * c0 + c2 * c2 ) * 0.25f;
                if ( tol < (double)flat ) break;
            }
            allFlat = ( r == dims[1] );
        }
        if ( allFlat )
            break;

        dims[0] += 2;
        if ( indexMap )
        {
            for ( int j = dims[0] - 1; j > col0 + 3; --j )
                indexMap[j] = indexMap[j - 2];
            int v = indexMap[col0 + 1];
            indexMap[col0 + 3] = v;
            indexMap[col0 + 2] = v;
            indexMap[col0 + 1] = indexMap[col0];
        }

        if ( dims[1] > 0 )
        {
            for ( int row = 0; row < dims[1]; ++row )
            {
                curveVert_t *p = &s_curveScratch[row][col0 + 1];   // unk_181F54C base
                curveVert_t mid0, mid1, mid2;
                Curve_LerpHalf( &mid0, &p[0],  &p[-1] );
                Curve_LerpHalf( &mid1, &p[1],  &p[0]  );
                Curve_LerpHalf( &mid2, &mid1,  &mid0  );
                // shift the tail right by TWO to open the 2-column gap (IDA `v13-88`
                // = src is 2 verts back); then write mid0/mid2/mid1 at p[0..2].
                for ( int k = dims[0] - 1; k > col0 + 3; --k )
                    s_curveScratch[row][k] = s_curveScratch[row][k - 2];
                p[0] = mid0;
                p[1] = mid2;
                p[2] = mid1;
            }
        }
    }
}

// ─── sub_432e20 — adaptive subdivision along ROWS (mirror of Curve_SubdivideCols
//     with the scratch grid's column/row strides swapped).  a1=&{cols,rows}.
static void Curve_SubdivideRows( int *dims, int *indexMap, int row0, float tol, float chordMax )
{
    while ( dims[1] + 2 < CURVE_GRID_DIM )
    {
        int allFlat = 0;
        if ( dims[0] > 0 )
        {
            curveVert_t *col = &s_curveScratch[row0][0];
            int c = 0;
            for ( ; c < dims[0]; ++c )
            {
                const float *p0 = col[c + 0 * CURVE_GRID_DIM].xyz;
                const float *p1 = col[c + 1 * CURVE_GRID_DIM].xyz;
                const float *p2 = col[c + 2 * CURVE_GRID_DIM].xyz;
                float a0 = p1[0] - p0[0], a1f = p1[1] - p0[1], a2f = p1[2] - p0[2];
                if ( chordMax < (double)sqrt( a1f * a1f + a0 * a0 + a2f * a2f ) ) break;
                float b0 = p2[0] - p1[0], b1 = p2[1] - p1[1], b2 = p2[2] - p1[2];
                if ( chordMax < (double)sqrt( b1 * b1 + b0 * b0 + b2 * b2 ) ) break;
                float d0 = b0 - a0, d1 = b1 - a1f, d2 = b2 - a2f;
                float flat = (float)sqrt( d1 * d1 + d0 * d0 + d2 * d2 ) * 0.25f;
                if ( tol < (double)flat ) break;
            }
            allFlat = ( c == dims[0] );
        }
        if ( allFlat )
            break;

        dims[1] += 2;
        if ( indexMap )
        {
            for ( int j = dims[1] - 1; j > row0 + 3; --j )
                indexMap[j] = indexMap[j - 2];
            int v = indexMap[row0 + 1];
            indexMap[row0 + 3] = v;
            indexMap[row0 + 2] = v;
            indexMap[row0 + 1] = indexMap[row0];
        }

        if ( dims[0] > 0 )
        {
            for ( int col = 0; col < dims[0]; ++col )
            {
                // base = unk_1824D20 + 44*col = scratch row (row0+1), column col.
                curveVert_t *p = &s_curveScratch[row0 + 1][col];
                curveVert_t mid0, mid1, mid2;
                Curve_LerpHalf( &mid0, &p[0],                 &p[-CURVE_GRID_DIM] );
                Curve_LerpHalf( &mid1, &p[CURVE_GRID_DIM],    &p[0] );
                Curve_LerpHalf( &mid2, &mid1,                 &mid0 );
                // shift the tail right by TWO rows (IDA `v13-45056` = src is 2 rows back).
                for ( int k = dims[1] - 1; k > row0 + 3; --k )
                    s_curveScratch[k][col] = s_curveScratch[k - 2][col];
                p[0]                 = mid0;
                p[CURVE_GRID_DIM]    = mid2;
                p[2 * CURVE_GRID_DIM]= mid1;
            }
        }
    }
}

// ─── sub_433140 — subdivision SETUP: copy the control grid into the scratch grid,
//     seed the index maps, subdivide both axes, pack into a curvePatchDef_t.
//     a1=cols, a2=rows, a4=src curveVert array, a6=subdiv tol, a7=999, a8/a9=index maps.
static curvePatchDef_t *Curve_SubdivideSetup( int cols, int rows, const curveVert_t *src,
                                              float tol, float chordMax,
                                              int *colMap, int *rowMap )
{
    int dims[2] = { cols, rows };

    // copy src[col + row*cols] into scratch[row][col].
    for ( int col = 0; col < cols; ++col )
        for ( int row = 0; row < rows; ++row )
            s_curveScratch[row][col] = src[col + row * cols];

    if ( rowMap )
        for ( int i = 0; i < rows; ++i ) rowMap[i] = i;
    if ( colMap )
        for ( int j = 0; j < cols; ++j ) colMap[j] = j;

    // IDA uses do/while loops whose continue-test reads the LIVE (subdivision-grown)
    // dims[0]/dims[1] AFTER advancing the cursor by 2 (`while (v20 + 2 < v30)`).  The
    // outer guard is the ORIGINAL count (cols/rows > 2).  A `for (c+2 < dims[0])` is
    // NOT equivalent: the do/while runs one extra segment at the tail (`c < v30`, not
    // `c < v30-2`) since dims[0] grows during the body.  Transcribed verbatim.
    if ( cols > 2 )
    {
        int c = 0;
        do { Curve_SubdivideCols( dims, colMap, c, tol, chordMax ); c += 2; }
        while ( c + 2 < dims[0] );
    }
    if ( rows > 2 )
    {
        int r = 0;
        do { Curve_SubdivideRows( dims, rowMap, r, tol, chordMax ); r += 2; }
        while ( r + 2 < dims[1] );
    }

    int w = dims[0], h = dims[1];
    size_t bytes = (size_t)44 * w * h;
    curvePatchDef_t *out = (curvePatchDef_t *)malloc( bytes + 20 );
    out->width  = w;
    out->height = h;
    out->verts  = (curveVert_t *)( out + 1 );
    for ( int row = 0; row < h; ++row )
        for ( int col = 0; col < w; ++col )
            out->verts[col + row * w] = s_curveScratch[row][col];
    return out;
}

// ─── sub_4329c0 — midpoint fixup: replace every odd interior vertex with the line
//     midpoint of its two even neighbours (smooths the inserted control points),
//     first along rows then along columns.  a1=cols, a2=rows, a4=verts.
static void Curve_MidpointFixup( int cols, int rows, curveVert_t *verts )
{
    // along columns: for each col, fix odd rows from even neighbours.
    for ( int col = 0; col < cols; ++col )
    {
        if ( rows <= 1 ) continue;
        for ( int r = 1; r < rows; r += 2 )
        {
            curveVert_t *cur  = &verts[col + r * cols];
            curveVert_t *prev = &verts[col + (r - 1) * cols];
            curveVert_t *next = &verts[col + (r + 1) * cols];
            curveVert_t m0, m1;
            Curve_LerpHalf( &m1, next, cur );
            Curve_LerpHalf( &m0, prev, cur );
            Curve_LerpHalf( cur, &m0, &m1 );
        }
    }
    // along rows: for each row, fix odd cols from even neighbours.
    for ( int row = 0; row < rows; ++row )
    {
        if ( cols <= 1 ) continue;
        for ( int c = 1; c < cols; c += 2 )
        {
            curveVert_t *cur  = &verts[c + row * cols];
            curveVert_t *prev = &verts[c - 1 + row * cols];
            curveVert_t *next = &verts[c + 1 + row * cols];
            curveVert_t m0, m1;
            Curve_LerpHalf( &m1, next, cur );
            Curve_LerpHalf( &m0, prev, cur );
            Curve_LerpHalf( cur, &m0, &m1 );
        }
    }
}

// neighbour offset table from .rdata (g_curveNeighborOffsets, 0x6DE534): 8-connected
// (dcol,drow) deltas, in the binary's EXACT order (di=*p applied to the row, dj=*(p-1)
// applied to the column → neighbour vertex = verts[(row+di)*width + (col+dj)]).  As the
// port applies entry[0] to the column (ni=col+entry0) and entry[1] to the row
// (nj=row+entry1), entry = (dcol, drow).  Transcribed verbatim from the IDB:
//   IDA n: (0,1)(1,1)(1,0)(1,-1)(0,-1)(-1,-1)(-1,0)(-1,1)  as (dcol,drow).
static const int s_curveNeighbors[8][2] =
{
    {  0,  1 }, {  1,  1 }, {  1,  0 }, {  1, -1 },
    {  0, -1 }, { -1, -1 }, { -1,  0 }, { -1,  1 },
};

// ─── sub_432190 — per-vertex NORMAL computation.  For each vertex, sum the
//     normalized cross products of adjacent edge pairs to its 8 neighbours
//     (with cyclic wrap when the patch is closed in that axis), then normalize.
static void Curve_ComputeNormals( curvePatchDef_t *curve )
{
    int width  = curve->width;
    int height = curve->height;

    // Detect cyclic closure (the binary's v62 / v61 flags):
    //   wrapWidth  (v62) — every row's first vs last COLUMN vertex coincide → the
    //                      patch wraps in the WIDTH axis (cylinder seam); wraps `ni`.
    //   wrapHeight (v61) — every col's first vs last ROW vertex coincide → the patch
    //                      wraps in the HEIGHT axis; wraps `nj`.
    int wrapWidth = 0;
    if ( height > 0 )
    {
        int r = 0;
        for ( ; r < height; ++r )
        {
            const float *a = curve->verts[r * width + 0].xyz;
            const float *b = curve->verts[r * width + (width - 1)].xyz;
            float d0 = a[0] - b[0], d1 = a[1] - b[1], d2 = a[2] - b[2];
            if ( (float)sqrt( d1 * d1 + d0 * d0 + d2 * d2 ) > 1.0f ) break;
        }
        wrapWidth = ( r == height );
    }
    int wrapHeight = 0;
    if ( width > 0 )
    {
        int c = 0;
        for ( ; c < width; ++c )
        {
            const float *a = curve->verts[0 * width + c].xyz;
            const float *b = curve->verts[(height - 1) * width + c].xyz;
            float d0 = a[0] - b[0], d1 = a[1] - b[1], d2 = a[2] - b[2];
            if ( (float)sqrt( d1 * d1 + d0 * d0 + d2 * d2 ) > 1.0f ) break;
        }
        wrapHeight = ( c == width );
    }

    for ( int col = 0; col < width; ++col )
    {
        for ( int row = 0; row < height; ++row )
        {
            const float *base = curve->verts[col + row * width].xyz;
            float dirs[8][3];
            int   have[8];

            for ( int n = 0; n < 8; ++n )
            {
                have[n] = 0;
                int di = s_curveNeighbors[n][0];
                int dj = s_curveNeighbors[n][1];
                // try up to 3 steps outward to find a non-degenerate neighbour edge.
                // IDA (0x4323c0/0x4323cd init, 0x432505/0x432511 advance): the running
                // step accumulators v54/v22 are UN-wrapped (col+step*dcol, row+step*drow);
                // the wrap formula is re-derived FRESH into temporaries each step and the
                // range check / vertex fetch use the wrapped temporaries — the un-wrapped
                // accumulator is never overwritten by the wrap.  (Prior port carried the
                // WRAPPED index forward via `ci=ni;rj=nj`, which diverges on closed patches
                // when a step re-crosses the seam within 3 steps.)
                int ci = col, rj = row;
                for ( int step = 1; step <= 3; ++step )
                {
                    ci += di;               // un-wrapped accumulator (v54 += dcol)
                    rj += dj;               // (v22 += drow)
                    int ni = ci;            // wrap a FRESH copy each step (v55/v56)
                    int nj = rj;
                    // cyclic wrap (matches v62/v61 branches).
                    if ( wrapWidth )
                    {
                        if ( ni < 0 )            ni = width  + ni - 1;
                        else if ( ni >= width )  ni = ni - width + 1;
                    }
                    if ( wrapHeight )
                    {
                        if ( nj < 0 )            nj = height + nj - 1;
                        else if ( nj >= height ) nj = nj - height + 1;
                    }
                    if ( ni < 0 || ni >= width || nj < 0 || nj >= height )
                        break;
                    const float *nb = curve->verts[ni + nj * width].xyz;
                    float e[3] = { nb[0] - base[0], nb[1] - base[1], nb[2] - base[2] };
                    if ( Curve_NormalizeTo( dirs[n], e ) != 0.0f )
                    {
                        have[n] = 1;
                        break;
                    }
                }
            }

            // sum cross products of consecutive neighbour-direction pairs.
            float sum[3] = { 0.0f, 0.0f, 0.0f };
            for ( int n = 0; n < 8; ++n )
            {
                if ( !have[n] ) continue;
                int m = ( n + 1 ) & 7;
                if ( !have[m] ) continue;
                float cr[3], crn[3];
                // IDA: Vec3Cross(dirs[(n+1)&7], dirs[n], out) → cr = dirs[m] × dirs[n]
                // (cross arg order matters: it sets the normal's SIGN).
                Vec3Cross( dirs[m], dirs[n], cr );          // (a, b, out=cr)
                if ( Curve_NormalizeTo( crn, cr ) != 0.0f )
                {
                    sum[0] += crn[0];
                    sum[1] += crn[1];
                    sum[2] += crn[2];
                }
            }
            Curve_NormalizeTo( curve->verts[col + row * width].normal, sum );
        }
    }
}

// ─── sub_4332d0 — remove DEGENERATE rows/cols (segments collapsed to < 0.1 units)
//     so the tessellated grid has no zero-area quads.  Works on the scratch grid;
//     a1=&{cols,rows} (from the prior pass), a2=colMap, a3=rowMap.
static curvePatchDef_t *Curve_RemoveDegenerate( const curvePatchDef_t *in, int *colMap, int *rowMap )
{
    int cols = in->width;
    int rows = in->height;

    // load the incoming verts into the scratch grid (scratch[row][col]).
    for ( int col = 0; col < cols; ++col )
        for ( int row = 0; row < rows; ++row )
            s_curveScratch[row][col] = in->verts[col + row * cols];

    // collapse interior columns whose every-row chord is < 0.1.
    for ( int c = 1; c < cols - 1; )
    {
        float maxChord = 0.0f;
        for ( int row = 0; row < rows; ++row )
        {
            curveVert_t *cur  = &s_curveScratch[row][c];
            curveVert_t *prev = &s_curveScratch[row][c - 1];
            curveVert_t *next = &s_curveScratch[row][c + 1];
            float proj[3];
            // IDA: ProjectOntoLine(p=next, out, base=prev, dir=cur) — project CUR onto
            // the prev→next line.  (p and dir args are next/cur, NOT cur/next.)
            Curve_ProjectOntoLine( next->xyz, proj, prev->xyz, cur->xyz );
            float e0 = cur->xyz[0] - proj[0];
            float e1 = cur->xyz[1] - proj[1];
            float e2 = cur->xyz[2] - proj[2];
            float chord = (float)sqrt( e0 * e0 + e1 * e1 + e2 * e2 );
            if ( maxChord < chord ) maxChord = chord;
        }
        if ( maxChord < 0.1 )   // IDA fcomp dbl_6F43D0 = DOUBLE 0.1 (not float 0.1f); widen maxChord to double
        {
            // drop column c (shift cols c+1.. left by one).
            for ( int row = 0; row < rows; ++row )
                for ( int k = c; k < cols - 1; ++k )
                    s_curveScratch[row][k] = s_curveScratch[row][k + 1];
            if ( colMap )
                for ( int k = c; k < cols - 1; ++k )
                    colMap[k] = colMap[k + 1];
            --cols;
        }
        else
            ++c;
    }

    // collapse interior rows whose every-col chord is < 0.1.
    for ( int r = 1; r < rows - 1; )
    {
        float maxChord = 0.0f;
        for ( int col = 0; col < cols; ++col )
        {
            curveVert_t *cur  = &s_curveScratch[r][col];
            curveVert_t *prev = &s_curveScratch[r - 1][col];
            curveVert_t *next = &s_curveScratch[r + 1][col];
            float proj[3];
            // IDA: ProjectOntoLine(p=next, out, base=prev, dir=cur) — project CUR onto
            // the prev→next line.
            Curve_ProjectOntoLine( next->xyz, proj, prev->xyz, cur->xyz );
            float e0 = cur->xyz[0] - proj[0];
            float e1 = cur->xyz[1] - proj[1];
            float e2 = cur->xyz[2] - proj[2];
            float chord = (float)sqrt( e0 * e0 + e1 * e1 + e2 * e2 );
            if ( maxChord < chord ) maxChord = chord;
        }
        if ( maxChord < 0.1 )   // IDA fcomp dbl_6F43D0 = DOUBLE 0.1 (not float 0.1f); widen maxChord to double
        {
            for ( int col = 0; col < cols; ++col )
                for ( int k = r; k < rows - 1; ++k )
                    s_curveScratch[k][col] = s_curveScratch[k + 1][col];
            if ( rowMap )
                for ( int k = r; k < rows - 1; ++k )
                    rowMap[k] = rowMap[k + 1];
            --rows;
        }
        else
            ++r;
    }

    size_t bytes = (size_t)44 * cols * rows;
    curvePatchDef_t *out = (curvePatchDef_t *)malloc( bytes + 20 );
    out->width  = cols;
    out->height = rows;
    out->verts  = (curveVert_t *)( out + 1 );
    for ( int row = 0; row < rows; ++row )
        for ( int col = 0; col < cols; ++col )
            out->verts[col + row * cols] = s_curveScratch[row][col];
    return out;
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_GenericMesh2  (0x438b60)  — build curveDef from the control grid.
//  a2 = the texture layer whose S/T pair is copied into the render verts (the
//  caller passes g_qeglobals.current_edit_layer); a3/a4 = the index maps the
//  binary threads through (null on the render path).  Returns a freshly malloc'd
//  curvePatchDef_t (caller assigns it to patchMesh_t.curveDef and owns the free).
// ════════════════════════════════════════════════════════════════════════════
curvePatchDef_t *Patch_GenericMesh2( patchMesh_t *p, int layer, int colMapArg, int rowMapArg )
{
    int   width  = p->width;
    int   height = p->height;
    // The binary (0x438b82) allocates unconditionally and has NO null-return path — the copy
    // loops below simply don't execute when width/height<=0, and it returns a valid (degenerate)
    // curvePatchDef_t.  Unreachable for real patches (min control grid), so the prior
    // `if(width<=0||height<=0) return nullptr` was an invented guard — dropped to match IDA.

    // flat control-vertex array cv[col + row*width] (the binary's `operator new`).
    curveVert_t *cv = (curveVert_t *)operator new( (size_t)44 * height * width );

    for ( int row = 0; row < height; ++row )
    {
        for ( int col = 0; col < width; ++col )
        {
            const drawVert_t *src = &p->ctrl[col][row];
            curveVert_t      *dst = &cv[col + row * width];
            dst->xyz[0] = src->xyz[0];
            dst->xyz[1] = src->xyz[1];
            dst->xyz[2] = src->xyz[2];
            // layer-selected S/T pair (st[2*layer], st[2*layer+1]).
            const float *st = (const float *)&src->texCoord;
            dst->st[0]      = st[2 * layer];
            dst->st[1]      = st[2 * layer + 1];
            dst->lightmap[0]= 0.0f;
            dst->lightmap[1]= 0.0f;
            dst->normal[0]  = src->normal[0];
            dst->normal[1]  = src->normal[1];
            dst->normal[2]  = src->normal[2];
            dst->vert_color = src->vert_color;
        }
    }

    curvePatchDef_t *result;
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
    {
        // terrain patches ARE their control grid (no subdivision).
        size_t bytes = (size_t)44 * width * height;
        result = (curvePatchDef_t *)malloc( bytes + 20 );
        result->width  = width;
        result->height = height;
        result->verts  = (curveVert_t *)( result + 1 );
        memcpy( result->verts, cv, bytes );
        Curve_ComputeNormals( result );
    }
    else
    {
        float subdiv = (float)p->subDivType;
        curvePatchDef_t *sub = Curve_SubdivideSetup( width, height, cv, subdiv, 999.0f,
                                                     (int *)(intptr_t)colMapArg,
                                                     (int *)(intptr_t)rowMapArg );
        Curve_MidpointFixup( sub->width, sub->height, sub->verts );
        Curve_ComputeNormals( sub );
        result = Curve_RemoveDegenerate( sub, (int *)(intptr_t)colMapArg,
                                         (int *)(intptr_t)rowMapArg );
        free( sub );
    }

    operator delete( cv );
    return result;
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_CalcVertColors  (0x4449e0, was sub_4449E0)
//  Recomputes per-vertex colours from the texture's layered-material lightmap
//  mode. The layered material's unk_flags2 (@0x0C, filled by MaterialDef_06/15
//  off the real registration) selects the mode: only (flags & 0x70) == 0x60
//  triggers a recompute. Every other case — no layered library entry
//  (lm == null → flags 0), or any non-lightmap mode — is a PASSTHROUGH: the
//  parsed vertex colours are kept verbatim (what the round-trip needs).
//
//  Recompute (only when the whole patch is still all-white = unauthored):
//  each control vertex's alpha becomes the luminance (R+G+B)/3 and its RGB is
//  forced to white — converting the parsed colour into a lightmap blend weight.
//  If ANY vertex already carries a non-white alpha the colours are treated as
//  authored and left untouched (early-out, also a passthrough).
//
//  Under the headless selftest the renderer is not up, so the material shim
//  keeps lm == null → flags 0 → passthrough → gate-neutral. Faithful to IDA:
//  bytes summed unsigned, signed /3 (0x55555556 magic), result fits a byte.
// ════════════════════════════════════════════════════════════════════════════
static int Patch_CalcVertColors( patchMesh_t *p )
{
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)&p->texture );
    int flags = lm ? lm->unk_flags2 : 0;        // 0x4449eb..0x4449fc

    if ( ( flags & 0x70 ) != 0x60 )             // not lightmap-alpha mode → keep
        return 0;

    // Pass 1 — if any vertex alpha is already != 0xFF, the colours were authored;
    // leave them (0x444a07..0x444a44).
    for ( int i = 0; i < p->width; i++ )
        for ( int j = 0; j < p->height; j++ )
            if ( (unsigned char)p->ctrl[i][j].vert_color.a != 0xFF )
                return j;

    // Pass 2 — recompute: alpha = (R+G+B)/3, RGB = white (0x444a46..0x444aa9).
    for ( int i = 0; i < p->width; i++ )
    {
        for ( int j = 0; j < p->height; j++ )
        {
            rgba_4byte *c = &p->ctrl[i][j].vert_color;
            int sum = (unsigned char)c->r + (unsigned char)c->g + (unsigned char)c->b;
            c->a = (char)( sum / 3 );
            c->r = (char)0xFF;
            c->g = (char)0xFF;
            c->b = (char)0xFF;
        }
    }
    return p->height;
}

// ════════════════════════════════════════════════════════════════════════════
//  AddBrushForPatch  (0x438e40)
//  Creates the symbiont bounding brush for a patch, ties brush<->patch, and (only
//  when linking to world — NOT on the load path) links + builds it. The bbox brush
//  is never written to the .map (Brush_Write dispatches patches to Patch_Write and
//  returns), so its faces/bounds matter only for not crashing.
// ════════════════════════════════════════════════════════════════════════════
brush_t *AddBrushForPatch( patchMesh_t *p, entity_s *world_ent )
{
    float mins[3], maxs[3];
    Patch_CalcBounds( mins, maxs, p, true );

    MaterialDef md;
    memset( &md, 0, sizeof( md ) );
    md.lyrMtl = p->texture.lyrMtl;
    md.radMtl = p->texture.radMtl;

    qtexture_s *lm = MaterialDef_GetLayeredMaterial( &md );
    int w = lm ? lm->width  : 512;
    int h = lm ? lm->height : 512;

    texdef_sub_t *texSlot = &md.mat_texDef + LayerMat::GetCurrentLayer( &md );
    texSlot->size[0] = (double)w * 0.25;
    texSlot->size[1] = 0.25 * (double)h;

    brush_t *b = Brush_Alloc( &md, nullptr );
    Brush_Create( mins, maxs, b, nullptr );

    for ( unsigned i = 0; i < (unsigned)b->faceCount; ++i )
    {
        b->faces[i].contents  = p->contents;
        b->faces[i].toolflags = p->flags;
    }

    b->patch    = p;
    ++p->version;
    p->pSymbiot = (entity_brush_s *)b;

    if ( world_ent )
    {
        // NOT reached on the load path (Patch_ParseMesh passes world_ent = null;
        // ParseEntity links the returned brush itself).
        Entity_LinkBrush( b, world_ent );
        Brush_BuildWindings( b, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex ||
             g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++b->version;
    }
    return b;
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_ParseVert  (0x4446f0, was Patch_Parse)  — parse ONE control vertex.
//  Grammar:  v <x> <y> <z>  [c <b> <g> <r> <a>]  t <s0> <t0> <s1> <t1>  [f <edge>]
//  Texcoords are stored ÷1024 (the file holds the ×1024 integer form).
// ════════════════════════════════════════════════════════════════════════════
static char Patch_ParseVert( const char **text, drawVert_t *v )
{
    if ( !v )
        Assert( PMESH_CPP, 5375, 0, "%s", "patchVert" );

    // "v"
    if ( strcmp( Com_Parse( text )->token, "v" ) )
        return 0;

    // xyz (same line)
    v->xyz[0] = (float)atof( Com_ParseOnLine( text )->token );
    v->xyz[1] = (float)atof( Com_ParseOnLine( text )->token );
    v->xyz[2] = (float)atof( Com_ParseOnLine( text )->token );

    const char *tok = Com_Parse( text )->token;
    if ( !strcmp( tok, "c" ) )
    {
        // wire order is b g r a (bytes 50,49,48,51 → vert_color.b/g/r/a)
        v->vert_color.b = (char)j__atol( Com_ParseOnLine( text )->token );
        v->vert_color.g = (char)j__atol( Com_ParseOnLine( text )->token );
        v->vert_color.r = (char)j__atol( Com_ParseOnLine( text )->token );
        v->vert_color.a = (char)j__atol( Com_ParseOnLine( text )->token );
        tok = Com_Parse( text )->token;
    }
    else
    {
        *(unsigned int *)&v->vert_color = 0xFFFFFFFF;   // a2+48 = -1 (white)
    }

    if ( strcmp( tok, "t" ) )
        return 0;

    // texCoord is a flat float[6]: st[0..1]=tex, st[2..3]=lightmap, st[4..5]=unused.
    float *st = (float *)&v->texCoord;
    for ( int i = 0; i < 3; ++i )
    {
        if ( i == 2 )
        {
            st[4] = 0.0f;
            st[5] = 0.0f;
        }
        else
        {
            float s = (float)( (float)atof( Com_ParseOnLine( text )->token ) * 0.0009765625 );
            float t = (float)( (float)atof( Com_ParseOnLine( text )->token ) * 0.0009765625 );
            if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5412, 0, "%s", "!IS_NAN(s)" );
            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5413, 0, "%s", "!IS_NAN(t)" );
            st[2 * i]     = s;
            st[2 * i + 1] = t;
        }
    }

    // optional "f <turned_edge>"
    if ( !strcmp( Com_Parse( text )->token, "f" ) )
    {
        v->turned_edge = (int)j__atol( Com_ParseOnLine( text )->token );
        return 1;
    }
    v->turned_edge = 0;
    Com_UngetToken();
    return 1;
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_ParseMesh  (0x444ac0, was PMESH_TexLayer)  — parse a `mesh`/`curve` block.
//  Called by Brush_Parse for the patch branch. `isMesh` = the block matched the
//  terrain variant ("mesh"/"patchTerrainDef3") → patchMesh_t gets PATCH_TERRAIN.
//  Returns the symbiont brush (brush->patch set), or null on parse error.
//  Only the iwmap-4 (version >= 4) grammar is ported — every stock test map is
//  version 4. The legacy (< 4) path needs the subdivision/version-0 material
//  remap helpers (PMESH_02 / sub_439350 / sub_45AD50), all out of round-trip scope.
// ════════════════════════════════════════════════════════════════════════════
brush_t *Patch_ParseMesh( const char **text, int version, int isMesh )
{
    // mesh-block open "{"
    if ( Com_Parse( text )->token[0] != '{' )
        return nullptr;

    patchMesh_t *p = MakeNewPatch();

    if ( version < 4 )
    {
        Error( "pmesh: legacy patch version < 4 not supported (round-trip subset); "
               "needs PMESH_02 / sub_439350 / sub_45AD50" );
        return nullptr;
    }

    char layerBuf[1024];
    Map_ParseEntityLayerKey( text, "000_Global", layerBuf, "layer" );
    p->contents = sub_42FB80( text );   // optional `contents <names>;`
    p->flags    = sub_42FBA0( text );   // optional `toolFlags <names>;`

    // material names. spaceDelimited=1 so '/' in a material path isn't a delimiter;
    // it covers BOTH the texture and lightmap reads, then reset to 0 for smoothing.
    ParseThreadInfo *pi = Com_GetParseThreadInfo();
    pi->parseInfo[pi->parseInfoNum].spaceDelimited = 1;
    SetMaterial( Com_Parse( text )->token, &p->texture );
    SetMaterial( Com_Parse( text )->token, &p->lightmap );
    pi->parseInfo[pi->parseInfoNum].spaceDelimited = 0;

    char smoothBuf[1024];
    Map_ParseEntityLayerKey( text, "smoothing_smooth", smoothBuf, "smoothing" );
    SetMaterial( smoothBuf, &p->smoothing );

    // params:  <width> <height> <size> <subDivType>
    p->width  = Com_ParseInt( text );
    p->height = (int)j__atol( Com_ParseOnLine( text )->token );

    if ( isMesh )
        p->type = PATCH_TERRAIN;

    float sizeVal = (float)atof( Com_ParseOnLine( text )->token );
    *(float *)&p->size_of_struct_0x504C = sizeVal;
    p->bDirty = ( sizeVal != 0.0f );

    int sdt = (int)j__atol( Com_ParseOnLine( text )->token );
    p->subDivType = ( sdt < 1 ) ? 1 : sdt;

    // control grid:  for each column "(" <height vertices> ")"
    for ( int col = 0; col < p->width; ++col )
    {
        Com_MatchToken( text, "(", 0 );
        drawVert_t *colBase = p->ctrl[col];
        for ( int row = 0; row < p->height; ++row )
            Patch_ParseVert( text, &colBase[row] );

        parseInfo_t *t = Com_Parse( text );
        if ( strcmp( t->token, ")" ) )
            Com_ScriptErrorDrop( "MatchToken: got '%s', expected '%s'\n", t->token, ")" );
    }

    // mesh-block close "}"
    parseInfo_t *closeTok = Com_Parse( text );
    if ( strcmp( closeTok->token, "}" ) )
        Com_ScriptErrorDrop( "MatchToken: got '%s', expected '%s'\n", closeTok->token, "}" );

    if ( version <= 4 )
        Patch_CalcVertColors( p );

    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );

    brush_t *b = AddBrushForPatch( p, nullptr );
    if ( b )
    {
        if ( b->parent_layer_string )
            free( b->parent_layer_string );
        size_t n  = strlen( layerBuf );
        char  *lc = (char *)operator new( n + 1 );
        memcpy( lc, layerBuf, n + 1 );
        b->parent_layer_string = lc;
    }
    return b;
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch write path.  Brush_Write dispatches `brush->patch` to Patch_Write.
//  The writer handle is a triple-pointer to a (ctx, fmt, ...) printf-like fn; the
//  ctx is the handle pointer cast to int (same convention as brush.cpp Brush_Write).
// ════════════════════════════════════════════════════════════════════════════
typedef int  WriteFunc_t( int ctx, const char *fmt, ... );
typedef WriteFunc_t **WriteWriter_t;
#define WRITE( w, ... ) ( ( **(w) )( (int)(intptr_t)(w), __VA_ARGS__ ) )

// VC7.1-compatible "%.8g" (round-half-away-from-zero + 3-digit exponent). Defined
// once in brush.cpp (the brush planept/texdef writer); shared here so patch verts
// format identically to brush planepts.
extern const char *Fmt8g( char *buf, double v );

// ─── Patch_WriteVert  (0x4456f0, was PMESH_WriteToMap) — write ONE control vertex.
static void Patch_WriteVert( drawVert_t *v, WriteWriter_t writer )
{
    if ( !v )
        Assert( PMESH_CPP, 5621, 0, "%s", "patchVert" );

    if ( ( *(unsigned int *)&v->xyz[0] & 0x7F800000 ) == 0x7F800000 ||
         ( *(unsigned int *)&v->xyz[1] & 0x7F800000 ) == 0x7F800000 ||
         ( *(unsigned int *)&v->xyz[2] & 0x7F800000 ) == 0x7F800000 )
        Assert( PMESH_CPP, 5623, 0, "%s",
                "!IS_NAN((patchVert->qv.xyz)[0]) && !IS_NAN((patchVert->qv.xyz)[1]) "
                "&& !IS_NAN((patchVert->qv.xyz)[2])" );

    char bx[64], by[64], bz[64];
    WRITE( writer, "\tv %s %s %s",
           Fmt8g( bx, v->xyz[0] ), Fmt8g( by, v->xyz[1] ), Fmt8g( bz, v->xyz[2] ) );

    unsigned char r = (unsigned char)v->vert_color.r;
    unsigned char g = (unsigned char)v->vert_color.g;
    unsigned char b = (unsigned char)v->vert_color.b;
    unsigned char a = (unsigned char)v->vert_color.a;
    if ( r != 0xFF || g != 0xFF || b != 0xFF || a != 0xFF )
        WRITE( writer, " c %i %i %i %i", b, g, r, a );   // wire order: b g r a

    WRITE( writer, " t" );

    const float *st = (const float *)&v->texCoord;   // flat float[6]
    for ( int i = 0; i < 3; ++i )
    {
        if ( i != 2 )
        {
            float s = (float)( st[2 * i]     * 1024.0 );
            float t = (float)( st[2 * i + 1] * 1024.0 );
            if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5637, 0, "%s", "!IS_NAN(s)" );
            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5638, 0, "%s", "!IS_NAN(t)" );
            char bs[64], bt[64];
            WRITE( writer, " %s %s", Fmt8g( bs, s ), Fmt8g( bt, t ) );
        }
    }

    if ( v->turned_edge )
        WRITE( writer, " f %i", v->turned_edge );

    WRITE( writer, "\n" );
}

// ─── Patch_Write  (0x4458f0, was MapLoad_ParsePatch) — write a patch as a
//     `mesh`/`curve` block.  Called by Brush_Write.  Layer comes from the symbiont
//     brush's parent_layer_string (IDA `pSymbiot[1].ownerPrev` = brush_t+0x48 via
//     its 56-byte selbrush_t mistyping).
int Patch_Write( WriteWriter_t writer, patchMesh_t *p )
{
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
        WRITE( writer, " {\n  mesh\n  {\n" );
    else
        WRITE( writer, " {\n  curve\n  {\n" );

    const char *layerName = ( (brush_t *)p->pSymbiot )->parent_layer_string;
    if ( strcmp( layerName, "000_Global" ) )   // MapLoad_ParseBrush_Layer inlined in the binary
        MapLoad_ParseBrush_Layer( (int (**)(int, const char *, ...))writer, (int)(intptr_t)layerName );

    MapLoad_ParseBrush_Content( (int)"contents",  (int (**)(int, const char *, ...))writer,
                                p->contents, contents_table );
    MapLoad_ParseBrush_Content( (int)"toolFlags", (int (**)(int, const char *, ...))writer,
                                p->flags,    toolflags_table );

    // texture / lightmap / smoothing names (shim stores the name in lyrMtl).
    const char *texName = (const char *)p->texture.lyrMtl;
    WRITE( writer, "   %s\n", texName );
    const char *lmName = (const char *)p->lightmap.lyrMtl;
    WRITE( writer, "   %s\n", lmName );
    const char *smName = (const char *)p->smoothing.lyrMtl;
    if ( strcmp( smName, "smoothing_smooth" ) )
        WRITE( writer, "   smoothing %s\n", smName );

    // IDA uses plain "%g" for the LOD size (6 sig figs); stock patch sizes are
    // integer-valued (0 in blackout) so no exponent arises — match IDA verbatim.
    float sizeVal = p->bDirty ? *(float *)&p->size_of_struct_0x504C : 0.0f;
    WRITE( writer, "   %i %i %g %i\n", p->width, p->height, (double)sizeVal, p->subDivType );

    for ( int col = 0; col < p->width; ++col )
    {
        WRITE( writer, "   (\n" );
        drawVert_t *colBase = p->ctrl[col];
        for ( int row = 0; row < p->height; ++row )
            Patch_WriteVert( &colBase[row], writer );
        WRITE( writer, "   )\n" );
    }

    return WRITE( writer, "  }\n }\n" );
}

// ════════════════════════════════════════════════════════════════════════════
//  PATCH CREATE + GRID MANIPULATION (the control-grid DATA layer)
//    * create a patch from a brush (Patch_BrushToMesh / Patch_GenericMesh) - fill the
//      control grid + lay LINEAR per-control texCoords via Patch_Naturalize2.
//      KISAK: the binary then REFINES those texCoords with PMESH_02 0x439580 /
//      sub_439350 (they tessellate the patch and re-sample S/T off the render mesh);
//      this port skips that refinement.  The linear coords are valid, finite and
//      deterministic, so the patch still round-trips byte-stably.
//    * grow/shrink the grid (Patch_InsertColumn/Row, Patch_RemoveColumn/Row) and rebuild
//      the symbiont bbox via Patch_CalcBounds + Brush_RebuildBrush - the curveDef-free
//      half of Patch_Rebuild (0x438D80).
// ════════════════════════════════════════════════════════════════════════════

// the global brush-instance display-list head sentinels (engine_stubs). The IDB
// 'selected_brushes_next'/'active_brushes_next' (0x23F1868/0x23F18A0) are just the
// .next members of these sentinel nodes (per brush.cpp's ruling at ~3056).
extern int         g_nUpdateBits;           // pending window-update mask (0x25D5A74)

static const double kOneThird = 0.3333333432674408;   // IDB dbl_6F46D8 (1/3 as a float)

// ── Patch_WidthDistanceTo (0x438FA0) / Patch_HeightDistanceTo (0x439040) ──────
//    Cumulative 3D distance walking the first row's columns 0..n-1 (width) or the
//    first column's rows 0..n-1 (height). Used by Patch_Naturalize2 for linear S/T.
// IDA accumulates in a FLOAT (`v5`): each segment's `sqrt` result is rounded to a
// float (`v7`) and the running total `v5` is a float too.  A double accumulator would
// carry more precision than the binary and shift the laid S/T coords — keep float.
static double Patch_WidthDistanceTo( int n, patchMesh_t *p )
{
    float sum = 0.0f;
    for ( int i = 0; i < n; ++i )
    {
        float dx = p->ctrl[i + 1][0].xyz[0] - p->ctrl[i][0].xyz[0];
        float dy = p->ctrl[i + 1][0].xyz[1] - p->ctrl[i][0].xyz[1];
        float dz = p->ctrl[i + 1][0].xyz[2] - p->ctrl[i][0].xyz[2];
        float seg = (float)sqrt( (double)( dx * dx + dy * dy + dz * dz ) );
        sum = seg + sum;
    }
    return sum;
}

static double Patch_HeightDistanceTo( int n, patchMesh_t *p )
{
    float sum = 0.0f;
    for ( int j = 0; j < n; ++j )
    {
        float dx = p->ctrl[0][j + 1].xyz[0] - p->ctrl[0][j].xyz[0];
        float dy = p->ctrl[0][j + 1].xyz[1] - p->ctrl[0][j].xyz[1];
        float dz = p->ctrl[0][j + 1].xyz[2] - p->ctrl[0][j].xyz[2];
        float seg = (float)sqrt( (double)( dx * dx + dy * dy + dz * dz ) );
        sum = seg + sum;
    }
    return sum;
}

// ── Patch_Naturalize2 (0x439840) — lay LINEAR per-control-point S/T on layer `a2`.
//    S advances with cumulative width-distance / (texWidth * a3); T with negated
//    cumulative height-distance / (texHeight * a4). Self-contained (no curveDef).
//    For the degenerate headless material, GetLayeredMaterial→null → 512×512 default.
static void Patch_Naturalize2( patchMesh_t *p, int a2, float a3, float a4 )
{
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)( &p->texture + a2 ) );
    int w = lm ? lm->width  : 512;
    int h = lm ? lm->height : 512;

    // The binary keeps both reciprocals AND the distance accumulators in 32-bit FLOAT slots
    // (IDB stack frame: var_C/var_10 = sScale/tScale, var_4/arg_8 = widthDist/heightDist, all
    // `float`), and rounds the S-product widthDist*sScale to a float intermediate hoisted out
    // of the inner loop (0x4398b1 `fld var_4; fmul var_C; fstp var_4`) before storing.  Using
    // double here carried up to 1 ULP of extra precision into the serialized control-point
    // texCoords on the Naturalize/Redisperse paths.
    float sScale = (float)(  1.0 / ( (double)w * a3 ) );
    float tScale = (float)( -1.0 / ( (double)h * a4 ) );

    float widthDist = 0.0f;
    for ( int i = 0; i < p->width; ++i )
    {
        float heightDist = 0.0f;
        float sCoord = widthDist * sScale;   // float intermediate, once per row (IDA var_4 reuse)
        for ( int j = 0; j < p->height; ++j )
        {
            float *st = &p->ctrl[i][j].texCoord.st[2 * a2];
            st[0] = sCoord;
            st[1] = heightDist * tScale;
            heightDist = (float)Patch_HeightDistanceTo( j + 1, p );
        }
        widthDist = (float)Patch_WidthDistanceTo( i + 1, p );
    }
    if ( a2 == 1 )
        p->bDirty = 0;
}

// ── InterpolateInteriorPoints (0x437BB0) — fill the interior EVEN control points of
//    a freshly-created cylinder/square grid: each even column w's points become the
//    average of the two flanking odd columns (wPrev = w-1 / wrap; wNext = w+1 / wrap).
//    Skips a point whose drawVert flag byte (turned_edge low byte) has bit 1 set —
//    always 0 on a freshly-made patch (the only caller), so the average always runs.
static void InterpolateInteriorPoints( patchMesh_t *p )
{
    for ( int w = 0; w < p->width; w += 2 )
    {
        int wNext = ( w == p->width - 1 ) ? 1 : ( ( w + 1 ) % p->width );
        int wPrev = ( w == 0 ) ? ( p->width - 2 ) : ( w - 1 );
        for ( int h = 0; h < p->height; ++h )
        {
            if ( ( (unsigned char)p->ctrl[w][h].turned_edge & 2 ) == 0 )
            {
                for ( int k = 0; k < 3; ++k )
                    p->ctrl[w][h].xyz[k] =
                        ( p->ctrl[wNext][h].xyz[k] + p->ctrl[wPrev][h].xyz[k] ) * 0.5f;
            }
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_BrushToMesh  (0x43ACC0)  — create a curve patch (cylinder / bevel / endcap /
//  square / cone) from the single selected brush, then replace the brush with it.
//  Geometry transcribed verbatim from the IDB; the texturing/curve tail is the
//  curveDef-free subset (see Patch_FinishCreate).
// ════════════════════════════════════════════════════════════════════════════
void Patch_BrushToMesh( char bCone, unsigned char bBevel, unsigned char bEndcap, char bSquare )
{
    if ( !QE_SingleBrush() )
        return;

    brush_t  *def   = selected_brushes.next->def;
    entity_s *owner = selected_brushes.next->owner;
    const float *mins = def->mins;
    const float *maxs = def->maxs;

    patchMesh_t *p = MakeNewPatch();
    p->height = 3;
    p->type   = PATCH_CYLINDER;

    if ( ( bSquare == 0 ) & bBevel )
    {
        p->type  = PATCH_BEVEL;
        p->width = 3;
        int step  = (int)( ( maxs[2] - mins[2] ) * 0.5 );
        int z     = (int)mins[2];
        for ( int i = 0; i < p->height; ++i )
        {
            p->ctrl[0][i].xyz[0] = mins[0]; p->ctrl[0][i].xyz[1] = mins[1]; p->ctrl[0][i].xyz[2] = (float)z;
            p->ctrl[1][i].xyz[0] = maxs[0]; p->ctrl[1][i].xyz[1] = mins[1]; p->ctrl[1][i].xyz[2] = (float)z;
            p->ctrl[2][i].xyz[0] = maxs[0]; p->ctrl[2][i].xyz[1] = maxs[1]; p->ctrl[2][i].xyz[2] = (float)z;
            z += step;
        }
    }
    else if ( ( bSquare == 0 ) & bEndcap )
    {
        p->type  = PATCH_ENDCAP;
        p->width = 5;
        int step  = (int)( ( maxs[2] - mins[2] ) * 0.5 );
        int z     = (int)mins[2];
        for ( int i = 0; i < p->height; ++i )
        {
            p->ctrl[0][i].xyz[0] = mins[0]; p->ctrl[0][i].xyz[1] = mins[1]; p->ctrl[0][i].xyz[2] = (float)z;
            p->ctrl[1][i].xyz[0] = mins[0]; p->ctrl[1][i].xyz[1] = maxs[1]; p->ctrl[1][i].xyz[2] = (float)z;
            p->ctrl[2][i].xyz[0] = (maxs[0] - mins[0]) * 0.5f + mins[0];
            p->ctrl[2][i].xyz[1] = maxs[1]; p->ctrl[2][i].xyz[2] = (float)z;
            p->ctrl[3][i].xyz[0] = maxs[0]; p->ctrl[3][i].xyz[1] = maxs[1]; p->ctrl[3][i].xyz[2] = (float)z;
            p->ctrl[4][i].xyz[0] = maxs[0]; p->ctrl[4][i].xyz[1] = mins[1]; p->ctrl[4][i].xyz[2] = (float)z;
            z += step;
        }
    }
    else
    {
        p->width = 9;
        p->ctrl[1][0].xyz[0] = mins[0]; p->ctrl[1][0].xyz[1] = mins[1];
        p->ctrl[3][0].xyz[0] = maxs[0]; p->ctrl[3][0].xyz[1] = mins[1];
        p->ctrl[5][0].xyz[0] = maxs[0]; p->ctrl[5][0].xyz[1] = maxs[1];
        p->ctrl[7][0].xyz[0] = mins[0]; p->ctrl[7][0].xyz[1] = maxs[1];
        for ( int i = 1; i < p->width - 1; i += 2 )
        {
            p->ctrl[i][0].xyz[2] = mins[2];
            p->ctrl[i][2].xyz[0] = p->ctrl[i][0].xyz[0];
            p->ctrl[i][2].xyz[1] = p->ctrl[i][0].xyz[1];
            p->ctrl[i][2].xyz[2] = maxs[2];
            for ( int k = 0; k < 3; ++k )
                p->ctrl[i][1].xyz[k] = ( p->ctrl[i][0].xyz[k] + p->ctrl[i][2].xyz[k] ) * 0.5f;
        }
        InterpolateInteriorPoints( p );

        if ( bSquare )
        {
            if ( bBevel )
            {
                for ( int i = 0; i < p->height; ++i )
                {
                    p->ctrl[2][i].xyz[0] = p->ctrl[1][i].xyz[0];
                    p->ctrl[2][i].xyz[1] = p->ctrl[1][i].xyz[1];
                    p->ctrl[2][i].xyz[2] = p->ctrl[1][i].xyz[2];
                    p->ctrl[6][i].xyz[0] = p->ctrl[7][i].xyz[0];
                    p->ctrl[6][i].xyz[1] = p->ctrl[7][i].xyz[1];
                    p->ctrl[6][i].xyz[2] = p->ctrl[7][i].xyz[2];
                }
            }
            else if ( bEndcap )
            {
                for ( int i = 0; i < p->height; ++i )
                {
                    p->ctrl[4][i].xyz[0] = p->ctrl[5][i].xyz[0];
                    p->ctrl[4][i].xyz[1] = p->ctrl[5][i].xyz[1];
                    p->ctrl[4][i].xyz[2] = p->ctrl[5][i].xyz[2];
                    p->ctrl[2][i].xyz[0] = p->ctrl[1][i].xyz[0];
                    p->ctrl[2][i].xyz[1] = p->ctrl[1][i].xyz[1];
                    p->ctrl[2][i].xyz[2] = p->ctrl[1][i].xyz[2];
                    p->ctrl[6][i].xyz[0] = p->ctrl[7][i].xyz[0];
                    p->ctrl[6][i].xyz[1] = p->ctrl[7][i].xyz[1];
                    p->ctrl[6][i].xyz[2] = p->ctrl[7][i].xyz[2];
                    p->ctrl[7][i].xyz[0] = p->ctrl[8][i].xyz[0];
                    p->ctrl[7][i].xyz[1] = p->ctrl[8][i].xyz[1];
                    p->ctrl[7][i].xyz[2] = p->ctrl[8][i].xyz[2];
                }
            }
            else
            {
                for ( int i = 0; i < p->width - 1; ++i )
                    for ( int j = 0; j < p->height; ++j )
                    {
                        p->ctrl[i][j].xyz[0] = p->ctrl[i + 1][j].xyz[0];
                        p->ctrl[i][j].xyz[1] = p->ctrl[i + 1][j].xyz[1];
                        p->ctrl[i][j].xyz[2] = p->ctrl[i + 1][j].xyz[2];
                    }
                for ( int j = 0; j < p->height; ++j )
                {
                    p->ctrl[8][j].xyz[0] = p->ctrl[0][j].xyz[0];
                    p->ctrl[8][j].xyz[1] = p->ctrl[0][j].xyz[1];
                    p->ctrl[8][j].xyz[2] = p->ctrl[0][j].xyz[2];
                }
            }
        }
    }

    // inherit the source brush's face material (texture + lightmap layers).
    p->texture  = *(patchMesh_material *)&def->faces->mtldef[0].lyrMtl;
    p->lightmap = *(patchMesh_material *)&def->faces->mtldef[1].lyrMtl;

    // IDA order: Naturalize2 → the two texCoord refinement passes → curveDef → cone
    // reshape → link.  Cone reshape AFTER Naturalize2 (matches the binary; the cone
    // collapses each column's apex point to the bbox centre).
    // IDA passes random_texture_stuff[0].sampleSize (the layer-0 default tex scale) as
    // BOTH scale args — NOT a hardcoded 0.25.  The GUI seeds sampleSize on init; headless
    // the test harness only memcpy's the MaterialDef (sizeof 0x24) into layer 0 and leaves
    // sampleSize (offset 0x24) zero, which would make sScale=1/(w*0)=inf → NaN S/T → a
    // Patch_Write IS_NAN assert.  Read the real field; fall back to the editor's 0.25
    // default only when it is unseeded (== 0), so this matches the binary whenever the GUI
    // is up and stays gate-safe headless.
    float natScale = g_qeglobals.random_texture_stuff[0].sampleSize;
    if ( natScale == 0.0f )
        natScale = 0.25f;
    Patch_Naturalize2( p, 0, natScale, natScale );
    // texCoord refinement passes (0x43b1a4-0x43b204) — identical shape to the sibling
    // Patch_GenericMesh: layer 1 @ flt_6F427C (16.0), then size_0x504C = 16.0 / bDirty = 1
    // (0x43b1d3 / 0x43b1e0), then layer 2 @ flt_6F42F0 (0.25). Restored 2026-07-31: these
    // were parked while PMESH_02 / Patch_TerrainTexProject were unported, which left a
    // newly-created cylinder/bevel/endcap/cone with only the linear Naturalize2 S/T.
    // bDirty also gates the `size` field in Patch_Write (a created patch would otherwise
    // save `size 0` instead of `size 16`).
    const bool terrain = ( p->type & PATCH_TERRAIN ) != 0;
    if ( terrain ) Patch_TerrainTexProject( p, 1, 16.0f );
    else           PMESH_02( p, 1, 16.0f );
    *(float *)&p->size_of_struct_0x504C = 16.0f;
    p->bDirty = 1;
    if ( terrain ) Patch_TerrainTexProject( p, 2, 0.25f );
    else           PMESH_02( p, 2, 0.25f );
    if ( p->curveDef )
        free( p->curveDef );
    // STAGE 1: builds the tessellated curveDef render mesh (Bezier subdivision for the
    // cylinder/bevel/endcap/cone control grid) so the new patch renders as wireframe.
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    ++p->version;

    if ( bCone )
    {
        p->type = PATCH_CONE;
        float xc = ( maxs[0] + mins[0] ) * 0.5f;
        float yc = ( maxs[1] + mins[1] ) * 0.5f;
        for ( int i = 0; i < p->width; ++i )
        {
            p->ctrl[i][2].xyz[0] = xc;
            p->ctrl[i][2].xyz[1] = yc;
        }
    }

    brush_t   *pdef = AddBrushForPatch( p, (entity_s *)owner->def );
    selbrush_t *inst = Brush_AddToList( pdef, owner );
    if ( inst->next || inst->prev )
        Error( "Brush_AddToList: already linked" );
    if ( &active_brushes == &selected_brushes )
        Brush_AddToList2( inst );
    else
    {
        inst->next = active_brushes.next;
        active_brushes.next->prev = inst;
        active_brushes.next = inst;
        inst->prev = &active_brushes;
    }

    Select_Delete();
    Select_Brush( inst, 1, 1, 0 );
}

// ════════════════════════════════════════════════════════════════════════════
//  Patch_GenericMesh  (0x43b310)
//
//  The "Simple Patch Mesh" backend (Curve→Simple Patch Mesh, cmd 32856 → the
//  CPatchDensityDlg width/height-density dialog → CPatchDensityDlg_OnOK 0x436390).
//  Builds a FLAT nWidth×nHeight Bezier patch laid over the selected single brush's
//  bounding box, in the plane of the active XY/XZ/YZ view (nOrientation =
//  ActiveXY->m_nViewType), stamps the current-layer materials, naturalizes the
//  texcoords, builds the render curveDef, links the new patch brush into the active
//  list, and (bDeleteSource) replaces the source selection with it.
//
//  IDA-verbatim notes:
//   * nWidth/nHeight bounds: nHeight>15 || (unsigned)(nWidth-3)>12 → reject (3..15).
//   * Orientation→axis map (LABEL_9/LABEL_10): the (u,v) plane axes for the view:
//       view 0 → (u=1,v=2)   view 1 → (u=0,v=2)   view 2 → (u=0,v=1)
//     and the patch is flattened at def->maxs[nOrientation] on the normal axis.
//   * u/v coords are INT-truncated walks of def->mins[axis] stepped by the
//     fabs()'d axis span / (count-1) — transcribed exactly (the int truncations
//     and double-precision step adds are the binary's; preserved).
//   * Texture/lightmap layers come from g_qeglobals.random_texture_stuff (the
//     current-edit-layer templates), NOT the source brush's faces.
//   * Naturalize scale = random_texture_stuff[0].sampleSize; falls back to the
//     editor's 0.25 default when unseeded (headless gate-safety, matching the
//     Patch_BrushPrimit port's identical guard — identical when the GUI is up).
//   * The bDirty texCoord refinement (sub_439350 terrain / PMESH_02 curve, both at
//     16.0 then 0.25, layers 1/2) is the REAL ported refinement, not parked.
//   * type is 0 (0x43b36f) — the `type & PATCH_TERRAIN` forks below are therefore DEAD
//     in the binary too; they are kept because the binary emits them.
//
//  Terrain creation is the separate Create_Terrain function at 0x43B660. It uses
//  random_texture_stuff; it is not a PATCH_TERRAIN variant of this function and does
//  not project materials from the selected brush face.
// ════════════════════════════════════════════════════════════════════════════
selbrush_t *Patch_GenericMesh( int nWidth, int nHeight, int nOrientation,
                               char bDeleteSource, char bOverwrite )
{
    if ( nHeight > 15 || (unsigned int)( nWidth - 3 ) > 12 )     // 0x43b331
    {
        Sys_Printf( "Invalid patch width or height.\n" );
        return nullptr;
    }
    if ( !bOverwrite && !QE_SingleBrush() )                      // 0x43b33d
    {
        Sys_Printf( "Cannot generate a patch from multiple selections.\n" );
        return nullptr;
    }

    patchMesh_t *p = MakeNewPatch();
    p->height = nHeight;
    p->width  = nWidth;
    p->type   = (PATCH_TYPES)0;                                  // 0x43b36f

    // Orientation → (u-axis, v-axis) of the view plane (IDA LABEL_9/LABEL_10).
    int uAxis, vAxis;
    if ( nOrientation == 0 )       { uAxis = 1; vAxis = 2; }
    else if ( nOrientation == 1 )  { uAxis = 0; vAxis = 2; }
    else                           { uAxis = 0; vAxis = 1; }

    brush_t  *def   = selected_brushes.next->def;
    entity_s *owner = selected_brushes.next->owner;

    // |span| / (count-1) step on each plane axis (fabs'd, double-precision divides).
    float uStep = (float)fabs( (double)( def->maxs[uAxis] - def->mins[uAxis] ) / (double)( nWidth  - 1 ) );
    float vStep = (float)fabs( (double)( def->maxs[vAxis] - def->mins[vAxis] ) / (double)( nHeight - 1 ) );

    int u = (int)def->mins[uAxis];
    for ( int i = 0; i < nWidth; ++i )
    {
        int   v  = (int)def->mins[vAxis];
        float uf = (float)u;
        for ( int j = 0; j < nHeight; ++j )
        {
            float vf = (float)v;
            p->ctrl[i][j].xyz[uAxis]        = uf;
            p->ctrl[i][j].xyz[vAxis]        = vf;
            p->ctrl[i][j].xyz[nOrientation] = def->maxs[nOrientation];
            v = (int)( vf + vStep );
        }
        u = (int)( (double)u + uStep );
    }

    // Stamp the current-edit-layer templates (0x43b4f2..0x43b50c): layer 0 = texture,
    // layer 1 = lightmap.  smoothing keeps MakeNewPatch's "smoothing_smooth".
    p->texture.lyrMtl  = g_qeglobals.random_texture_stuff[0].mtl.lyrMtl;
    p->texture.radMtl  = g_qeglobals.random_texture_stuff[0].mtl.radMtl;
    p->lightmap.lyrMtl = g_qeglobals.random_texture_stuff[1].mtl.lyrMtl;
    p->lightmap.radMtl = g_qeglobals.random_texture_stuff[1].mtl.radMtl;

    float natScale = g_qeglobals.random_texture_stuff[0].sampleSize;
    if ( natScale == 0.0f )
        natScale = 0.25f;
    Patch_Naturalize2( p, 0, natScale, natScale );                // 0x43b521

    const bool terrain = ( p->type & PATCH_TERRAIN ) != 0;       // 0x43b538 (always false)
    if ( terrain ) Patch_TerrainTexProject( p, 1, 16.0f );
    else           PMESH_02( p, 1, 16.0f );
    *(float *)&p->size_of_struct_0x504C = 16.0f;                 // 0x43b55e
    p->bDirty = 1;                                               // 0x43b56b
    if ( terrain ) Patch_TerrainTexProject( p, 2, 0.25f );
    else           PMESH_02( p, 2, 0.25f );

    if ( p->curveDef )                                           // 0x43b592
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    ++p->version;

    brush_t   *pdef = AddBrushForPatch( p, (entity_s *)owner->def );
    selbrush_t *inst = Brush_AddToList( pdef, owner );
    if ( inst->next || inst->prev )
        Error( "Brush_AddToList: already linked" );
    if ( &active_brushes == &selected_brushes )
        Brush_AddToList2( inst );
    else
    {
        inst->next = active_brushes.next;
        active_brushes.next->prev = inst;
        active_brushes.next = inst;
        inst->prev = &active_brushes;
    }

    if ( bDeleteSource )
    {
        Select_Delete();
        Select_Brush( inst, 1, 1, 0 );
    }

    return inst;
}

// ── Patch_BuildCurveDef (0x438D50) — free + rebuild the tessellated render mesh ──
//    The named helper the binary calls where Patch_BrushToMesh / Patch_GenericMesh
//    inline the same three statements.
static curvePatchDef_t *Patch_BuildCurveDef( patchMesh_t *p )
{
    if ( p->curveDef )                                           // 0x438d51
        free( p->curveDef );
    curvePatchDef_t *result = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    p->curveDef = result;                                        // 0x438d78
    return result;
}

// Create_Terrain (0x43B660), called by TerrainDlg_NewPatch (0x458FB0).
selbrush_t *Create_Terrain( int width, int height, int orientation )
{
    if ( (unsigned int)( height - 2 ) > 14 || (unsigned int)( width - 2 ) > 14 )   // 0x43b684
    {
        Sys_Printf( "Invalid terrain width or height.\n" );
        return nullptr;
    }
    if ( !QE_SingleBrush() )                                                       // 0x43b68a
    {
        Sys_Printf( "Cannot generate a terrain from multiple selections.\n" );
        return nullptr;
    }

    patchMesh_t *p = MakeNewPatch();
    p->height = height;
    p->width  = width;
    p->type   = PATCH_TERRAIN;                                                     // 0x43b6bc

    int uAxis, vAxis;                                                              // LABEL_8/LABEL_9
    if ( orientation == 0 )       { uAxis = 1; vAxis = 2; }
    else if ( orientation == 1 )  { uAxis = 0; vAxis = 2; }
    else                          { uAxis = 0; vAxis = 1; }

    brush_t  *def   = selected_brushes.next->def;
    entity_s *owner = selected_brushes.next->owner;

    float uStep = (float)fabs( (double)( def->maxs[uAxis] - def->mins[uAxis] ) / (double)( width  - 1 ) );
    float vStep = (float)fabs( (double)( def->maxs[vAxis] - def->mins[vAxis] ) / (double)( height - 1 ) );

    int u = (int)def->mins[uAxis];
    for ( int i = 0; i < width; ++i )
    {
        int   v  = (int)def->mins[vAxis];
        float uf = (float)u;
        for ( int j = 0; j < height; ++j )
        {
            float vf = (float)v;
            p->ctrl[i][j].xyz[uAxis]      = uf;
            p->ctrl[i][j].xyz[vAxis]      = vf;
            p->ctrl[i][j].xyz[orientation] = def->maxs[orientation];
            v = (int)( vf + vStep );
        }
        u = (int)( (double)u + uStep );
    }

    p->texture.lyrMtl  = g_qeglobals.random_texture_stuff[0].mtl.lyrMtl;           // 0x43b841
    p->texture.radMtl  = g_qeglobals.random_texture_stuff[0].mtl.radMtl;           // 0x43b849
    p->lightmap.lyrMtl = g_qeglobals.random_texture_stuff[1].mtl.lyrMtl;           // 0x43b852
    p->lightmap.radMtl = g_qeglobals.random_texture_stuff[1].mtl.radMtl;           // 0x43b85b

    const float natScale = g_qeglobals.random_texture_stuff[0].sampleSize;         // 0x43b870
    Patch_Naturalize2( p, 0, natScale, natScale );

    const bool terrain = ( p->type & PATCH_TERRAIN ) != 0;                         // 0x43b886 (always true here)
    if ( terrain ) Patch_TerrainTexProject( p, 1, 16.0f );
    else           PMESH_02( p, 1, 16.0f );
    *(float *)&p->size_of_struct_0x504C = 16.0f;                                   // 0x43b8ad
    p->bDirty = 1;                                                                 // 0x43b8ba
    if ( terrain ) Patch_TerrainTexProject( p, 2, 0.25f );
    else           PMESH_02( p, 2, 0.25f );

    Patch_BuildCurveDef( p );                                                      // 0x43b8e1
    ++p->version;                                                                  // 0x43b8e6

    brush_t    *pdef = AddBrushForPatch( p, (entity_s *)owner->def );              // 0x43b8f7
    selbrush_t *inst = Brush_AddToList( pdef, owner );                             // 0x43b907
    // Select_Brush_2(&active_brushes, inst)  (0x476630, inlined — list != selected_brushes)
    if ( inst->next || inst->prev )
        Error( "Brush_AddToList: already linked" );
    if ( &active_brushes == &selected_brushes )
        Brush_AddToList2( inst );
    else
    {
        inst->next = active_brushes.next;
        active_brushes.next->prev = inst;
        active_brushes.next = inst;
        inst->prev = &active_brushes;
    }

    Select_Delete();                                                               // 0x43b90e
    Select_Brush( inst, 1, 1, 0 );                                                 // 0x43b91b
    return inst;
}

// ════════════════════════════════════════════════════════════════════════════
//  GRID MANIPULATION — insert / remove a column or row of control points.
//
//  Each function has a TERRAIN branch (type & PATCH_TERRAIN) that adds/removes a
//  SINGLE row/col via the 4 helpers below, and a BEZIER branch that adds/removes
//  TWO (keeping the odd 2n+1 grid).  Transcribed from the IDB (0x443410 / 0x443830 /
//  0x443B60 / 0x443C90).  CoD divergences vs GtkRadiant (IDA = ground truth):
//   * the new flanking points are offset by ±1/3 of the adjacent segment delta
//     (dbl_6F46D8 = 1/3), NOT GtkRadiant's ±1/2;
//   * the insert/remove POSITION is chosen by the `flag` arg (front vs back), NOT
//     GtkRadiant's PointInMoveList vertex-selection scan;
//   * REMOVE is a plain 2-column/row drop (no GtkRadiant extrapolation);
//   * the tail refreshes CWnd_PatchDialog instead of Patch_EditPatch (the patch
//     inspector shipped with patchdialog.cpp; Patch_AdjustSelected 0x444550 has no
//     refresh tail in the binary, verified — the refresh belongs to the callers).
// ════════════════════════════════════════════════════════════════════════════

// ─── terrain single-row/col helpers ──────────────────────────────────────────
// sub_4436A0 — insert ONE row at index `at` (height += 1), midpoint-averaged.
static void Patch_TerrainInsertRow( patchMesh_t *p, int at )
{
    if ( p->height + 1 > 16 )
        return;
    for ( int col = 0; col < p->width; ++col )
    {
        for ( int row = p->height - 1; row >= at; --row )
            p->ctrl[col][row + 1] = p->ctrl[col][row];
        // new row `at` = average of the (now shifted) neighbours, xyz + st0/st1 + color
        drawVert_t *nv = &p->ctrl[col][at];
        drawVert_t *lo = &p->ctrl[col][at - 1];
        drawVert_t *hi = &p->ctrl[col][at + 1];
        for ( int k = 0; k < 3; ++k )
            nv->xyz[k] = ( lo->xyz[k] + hi->xyz[k] ) * 0.5f;
        for ( int k = 0; k < 2; ++k )
        {
            nv->texCoord.st[k]        = ( lo->texCoord.st[k]        + hi->texCoord.st[k] )        * 0.5f;
            nv->texCoord.lightmap[k]  = ( lo->texCoord.lightmap[k]  + hi->texCoord.lightmap[k] )  * 0.5f;
            nv->texCoord.smoothing[k] = ( lo->texCoord.smoothing[k] + hi->texCoord.smoothing[k] ) * 0.5f;
        }
        ((unsigned char *)&nv->vert_color)[0] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[0] + ((unsigned char*)&hi->vert_color)[0] ) / 2 );
        ((unsigned char *)&nv->vert_color)[1] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[1] + ((unsigned char*)&hi->vert_color)[1] ) / 2 );
        ((unsigned char *)&nv->vert_color)[2] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[2] + ((unsigned char*)&hi->vert_color)[2] ) / 2 );
        ((unsigned char *)&nv->vert_color)[3] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[3] + ((unsigned char*)&hi->vert_color)[3] ) / 2 );
        // register the new control point as an editable handle (IDB d_points append).
        for ( int k = 0; k < 3; ++k )
            g_qeglobals.d_points[g_qeglobals.d_numpoints][k] = nv->xyz[k];
        if ( g_qeglobals.d_numpoints < 2047 )
            ++g_qeglobals.d_numpoints;
    }
    ++p->height;
}

// sub_443260 — insert ONE column at index `at` (width += 1), midpoint-averaged.
static void Patch_TerrainInsertColumn( patchMesh_t *p, int at )
{
    if ( p->width + 1 > 16 )
        return;
    for ( int row = 0; row < p->height; ++row )
    {
        for ( int col = p->width - 1; col >= at; --col )
            p->ctrl[col + 1][row] = p->ctrl[col][row];
        drawVert_t *nv = &p->ctrl[at][row];
        drawVert_t *lo = &p->ctrl[at - 1][row];
        drawVert_t *hi = &p->ctrl[at + 1][row];
        for ( int k = 0; k < 3; ++k )
            nv->xyz[k] = ( lo->xyz[k] + hi->xyz[k] ) * 0.5f;
        for ( int k = 0; k < 2; ++k )
        {
            nv->texCoord.st[k]        = ( lo->texCoord.st[k]        + hi->texCoord.st[k] )        * 0.5f;
            nv->texCoord.lightmap[k]  = ( lo->texCoord.lightmap[k]  + hi->texCoord.lightmap[k] )  * 0.5f;
            nv->texCoord.smoothing[k] = ( lo->texCoord.smoothing[k] + hi->texCoord.smoothing[k] ) * 0.5f;
        }
        ((unsigned char *)&nv->vert_color)[0] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[0] + ((unsigned char*)&hi->vert_color)[0] ) / 2 );
        ((unsigned char *)&nv->vert_color)[1] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[1] + ((unsigned char*)&hi->vert_color)[1] ) / 2 );
        ((unsigned char *)&nv->vert_color)[2] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[2] + ((unsigned char*)&hi->vert_color)[2] ) / 2 );
        ((unsigned char *)&nv->vert_color)[3] = (unsigned char)( ( ((unsigned char*)&lo->vert_color)[3] + ((unsigned char*)&hi->vert_color)[3] ) / 2 );
        // register the new control point as an editable handle (IDB d_points append).
        for ( int k = 0; k < 3; ++k )
            g_qeglobals.d_points[g_qeglobals.d_numpoints][k] = nv->xyz[k];
        if ( g_qeglobals.d_numpoints < 2047 )
            ++g_qeglobals.d_numpoints;
    }
    ++p->width;
}

// sub_443AE0 — remove ONE row at index `at` (height -= 1, min 3), shift rows down.
static void Patch_TerrainRemoveRow( patchMesh_t *p, int at )
{
    if ( p->height <= 2 )
        return;
    --p->height;
    if ( at == p->height )
        return;                               // removed the last row → nothing to shift
    for ( int col = 0; col < p->width; ++col )
        for ( int row = at; row < p->height; ++row )
            p->ctrl[col][row] = p->ctrl[col][row + 1];
}

// sub_443C10 — remove ONE column at index `at` (width -= 1, min 3), shift cols left.
static void Patch_TerrainRemoveColumn( patchMesh_t *p, int at )
{
    if ( p->width <= 2 )
        return;
    --p->width;
    if ( at == p->width )
        return;
    for ( int col = at; col < p->width; ++col )
        for ( int row = 0; row < p->height; ++row )
            p->ctrl[col][row] = p->ctrl[col + 1][row];
}

// ─── Patch_InsertColumn (0x443410) ───────────────────────────────────────────
void Patch_InsertColumn( patchMesh_t *p, char flag )
{
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
    {
        Patch_TerrainInsertColumn( p, flag ? ( p->width - 1 ) : 1 );
        return;
    }
    if ( p->width + 2 > 16 )
        return;

    if ( flag )
    {
        // insert 2 columns at the END: subdivide the last segment into thirds.
        int w = p->width;                     // old column count
        for ( int h = 0; h < p->height; ++h )
        {
            float third[3];
            for ( int k = 0; k < 3; ++k )
                third[k] = ( p->ctrl[w - 1][h].xyz[k] - p->ctrl[w - 2][h].xyz[k] ) * (float)kOneThird;
            p->ctrl[w + 1][h] = p->ctrl[w - 1][h];                 // old last → new endpoint
            p->ctrl[w - 1][h] = p->ctrl[w - 2][h];                 // new col w-1 base
            for ( int k = 0; k < 3; ++k ) p->ctrl[w - 1][h].xyz[k] += third[k];
            p->ctrl[w][h] = p->ctrl[w - 1][h];                     // new col w base
            for ( int k = 0; k < 3; ++k ) p->ctrl[w][h].xyz[k] += third[k];
        }
    }
    else
    {
        // insert 2 columns at the FRONT (after col 0): shift up, build cols 1 & 2.
        for ( int h = 0; h < p->height; ++h )
        {
            // IDA 0x44358C shifts col 0 INCLUSIVE (full 0x50 rep movsd): ctrl[col+2]=ctrl[col]
            // for col=width-1..0, so ctrl[2] inherits col-0's full vertex.
            for ( int col = p->width - 1; col >= 0; --col )
                p->ctrl[col + 2][h] = p->ctrl[col][h];
            float third[3];
            for ( int k = 0; k < 3; ++k )
                third[k] = ( p->ctrl[3][h].xyz[k] - p->ctrl[0][h].xyz[k] ) * (float)kOneThird;
            // IDA 0x4435F3+ writes ONLY the 3 xyz floats to the two new columns (fld/fstp, NO
            // rep movsd): ctrl[1] keeps ORIGINAL col-1's texCoord/normal/color (never a shift
            // dest), ctrl[2] keeps col-0's (from the inclusive shift above).  The prior port's
            // full-struct copies clobbered ctrl[1]'s non-xyz fields with col-0's.
            for ( int k = 0; k < 3; ++k ) p->ctrl[1][h].xyz[k] = p->ctrl[0][h].xyz[k] + third[k];
            for ( int k = 0; k < 3; ++k ) p->ctrl[2][h].xyz[k] = p->ctrl[1][h].xyz[k] + third[k];
        }
    }
    p->width += 2;
}

// ─── Patch_InsertRow (0x443830) ──────────────────────────────────────────────
void Patch_InsertRow( patchMesh_t *p, char flag )
{
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
    {
        if ( p->height + 1 > 16 )
            return;
        Patch_TerrainInsertRow( p, flag ? ( p->height - 1 ) : 1 );
        return;
    }
    if ( p->height + 2 > 16 )
        return;

    if ( flag )
    {
        int h = p->height;                    // old row count
        for ( int w = 0; w < p->width; ++w )
        {
            float third[3];
            // IDA Patch_InsertRow uses `delta / 3.0` (x87 fdiv), NOT InsertColumn's
            // `* 0.3333333432674408` — keep the exact constant (float-exact).
            for ( int k = 0; k < 3; ++k )
                third[k] = (float)( (double)( p->ctrl[w][h - 1].xyz[k] - p->ctrl[w][h - 2].xyz[k] ) / 3.0 );
            p->ctrl[w][h + 1] = p->ctrl[w][h - 1];
            p->ctrl[w][h - 1] = p->ctrl[w][h - 2];
            for ( int k = 0; k < 3; ++k ) p->ctrl[w][h - 1].xyz[k] += third[k];
            p->ctrl[w][h] = p->ctrl[w][h - 1];
            for ( int k = 0; k < 3; ++k ) p->ctrl[w][h].xyz[k] += third[k];
        }
    }
    else
    {
        for ( int w = 0; w < p->width; ++w )
        {
            for ( int row = p->height - 1; row >= 1; --row )
                p->ctrl[w][row + 2] = p->ctrl[w][row];
            float third[3];
            // IDA Patch_InsertRow: `delta / 3.0` (x87 fdiv), not `* kOneThird`.
            for ( int k = 0; k < 3; ++k )
                third[k] = (float)( (double)( p->ctrl[w][3].xyz[k] - p->ctrl[w][0].xyz[k] ) / 3.0 );
            // IDA 0x4439F8/0x443A60 write ONLY xyz to the two new rows (fld/fstp, NO rep movsd):
            // ctrl[w][1] keeps ORIGINAL row-1's non-xyz, ctrl[w][2] keeps row-2's — the row shift
            // is row>=1 (ctrl[w][2] is never a dest), unlike InsertColumn's col0-inclusive shift.
            for ( int k = 0; k < 3; ++k ) p->ctrl[w][1].xyz[k] = p->ctrl[w][0].xyz[k] + third[k];
            for ( int k = 0; k < 3; ++k ) p->ctrl[w][2].xyz[k] = p->ctrl[w][1].xyz[k] + third[k];
        }
    }
    p->height += 2;
}

// ─── Patch_RemoveColumn (0x443C90) ───────────────────────────────────────────
void Patch_RemoveColumn( patchMesh_t *p, char flag )
{
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
    {
        Patch_TerrainRemoveColumn( p, flag ? 0 : ( p->width - 1 ) );
        return;
    }
    if ( p->width <= 3 )
        return;
    p->width -= 2;
    if ( flag )                               // remove the FIRST 2 columns: shift left by 2
    {
        for ( int row = 0; row < p->height; ++row )
            for ( int col = 0; col < p->width; ++col )
                p->ctrl[col][row] = p->ctrl[col + 2][row];
    }
    // flag clear: drop the LAST 2 columns (no shift; the width-- already did it).
}

// ─── Patch_RemoveRow (0x443B60) ──────────────────────────────────────────────
void Patch_RemoveRow( patchMesh_t *p, char flag )
{
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
    {
        Patch_TerrainRemoveRow( p, flag ? 0 : ( p->height - 1 ) );
        return;
    }
    if ( p->height <= 3 )
        return;
    p->height -= 2;
    if ( flag )                               // remove the FIRST 2 rows: shift down by 2
    {
        for ( int col = 0; col < p->width; ++col )
            for ( int row = 0; row < p->height; ++row )
                p->ctrl[col][row] = p->ctrl[col][row + 2];
    }
    // flag clear: drop the LAST 2 rows.
}

// ─── Patch_Rebuild (0x438D80) — texCoord refine + bbox + curveDef rebuild ─────
//    The binary refines texCoords (PMESH_02/sub_439350) when bDirty, then rebuilds
//    the symbiont bbox (Brush_RebuildBrush) and the curveDef (Patch_GenericMesh2).
//    The refinement block was parked while PMESH_02/Patch_TerrainTexProject were
//    unported; both shipped with the terrain-texture unit, so it is RESTORED here
//    (2026-07-31). Without it, every grid edit that routes through Patch_Rebuild
//    (insert/remove row+column via Patch_AdjustSelected, OnDropSelectedRelativeZ)
//    left the patch's S/T untouched, so edited patches kept pre-edit texturing.
void Patch_Rebuild( patchMesh_t *p, char doBounds )   // non-static: OnDropSelectedRelativeZ calls it
{
    if ( doBounds )
    {
        if ( p->bDirty )
        {
            // 0x438d9b: sampleSize is SAVED to a stack slot, passed to the refine, then
            // written BACK afterwards along with bDirty=1 — the refine may clobber both,
            // and the binary deliberately preserves size / re-dirties. Layer is 1 for both
            // branches (the bezier one passes it in ecx, which is why hex-rays drops it).
            float sampleSize = *(float *)&p->size_of_struct_0x504C;
            if ( ( p->type & PATCH_TERRAIN ) != 0 )
                Patch_TerrainTexProject( p, 1, sampleSize );
            else
                PMESH_02( p, 1, sampleSize );
            p->bDirty = 1;
            *(float *)&p->size_of_struct_0x504C = sampleSize;
        }
        float mins[3], maxs[3];
        Patch_CalcBounds( mins, maxs, p, true );
        Brush_RebuildBrush( (brush_t *)p->pSymbiot, mins, maxs );
    }
    if ( p->curveDef )
        free( p->curveDef );
    // STAGE 1: rebuild the tessellated render mesh so grid edits re-render as wireframe.
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    ++p->version;
}

// ─── PMESH_49 (0x4495C0) — regenerate EVERY active patch's curveDef ───────────
//   Same curveDef free/rebuild/version-bump as Patch_Rebuild's tail, applied to every
//   patch in active_brushes. Called by CMainFrame::OnPrefs (0x426a2d) because the
//   render mesh is built for g_qeglobals.current_edit_layer, and the prefs dialog can
//   change which layer that is — without this, patches keep meshes tessellated for the
//   old layer until each is edited.
void PMESH_49()
{
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        iassert( b->patch->def == b->def->patch );   // PMESH.CPP:7147
        patchMesh_t *def = b->patch->def;
        if ( def->curveDef )
            free( def->curveDef );
        curvePatchDef_t *mesh = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
        ++def->version;
        def->curveDef = mesh;
    }
}

// ─── Patch_AdjustSelected (0x444550) — apply insert/remove to every selected patch ─
void Patch_AdjustSelected( char bInsert, char bColumn, char bFlag )
{
    char bUpdate = 0;
    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes;
          b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *p = b->def->patch;       // the patch DEF (instance.patch->def == def->patch)
        // IDA 0x44459b: patch-symbiont invariant assert (PMESH.CPP:5316), dropped in the prior
        // port; the sibling Patch_Transpose keeps the same assert.  KEEP_VERBOSE preserves the
        // exact binary PMESH.CPP:5316 string (member-expression cond).
        iassert( b->patch->def == b->def->patch );
        if ( !p )
            continue;                         // defensive: never-built patch (binary asserts above)
        if ( bInsert )
        {
            if ( bColumn ) Patch_InsertColumn( p, bFlag );
            else           Patch_InsertRow( p, bFlag );
        }
        else
        {
            if ( bColumn ) Patch_RemoveColumn( p, bFlag );
            else           Patch_RemoveRow( p, bFlag );
        }
        bUpdate = 1;
        // IDA 0x445XXX: post-op stability assert (PMESH.CPP:5333) — def didn't realloc.
        iassert( p == b->patch->def );
        Patch_Rebuild( p, 1 );
    }
    if ( bUpdate )
        g_nUpdateBits = -1;
}

// ─── patchInvert2 (0x446480) — VERTICAL FLIP: mirror each column's rows ────────
//   Transcribed from the IDB flat-pointer walk.  ctrl is drawVert_t[16][16] at
//   patchMesh_t+56 (verified), drawVert 80B; the IDB's v3 resolves to ctrl[col][r]
//   and v5 = (char*)p + 80*height + 80*(16*col - r) - 24 resolves to
//   ctrl[col][height-1-r] (since ctrl_base = p+56 ⇒ -24 = 56-80).  So it swaps
//   ctrl[col][r] <-> ctrl[col][height-1-r] for r in [0, height/2).  Terrain patches
//   then propagate each control vertex's turned_edge bit0 up one row over rows
//   0..height-2 (`*v7 ^= (*v7 ^ v7[+1row]) & 1`).
static void patchInvert2( patchMesh_t *p )
{
    for ( int col = 0; col < p->width; ++col )
    {
        for ( int r = 0; r < p->height / 2; ++r )
        {
            drawVert_t tmp                  = p->ctrl[col][r];
            p->ctrl[col][r]                 = p->ctrl[col][p->height - 1 - r];
            p->ctrl[col][p->height - 1 - r] = tmp;
        }
        if ( ( p->type & PATCH_TERRAIN ) != 0 )
        {
            for ( int r = 0; r < p->height - 1; ++r )
                p->ctrl[col][r].turned_edge ^=
                    ( p->ctrl[col][r].turned_edge ^ p->ctrl[col][r + 1].turned_edge ) & 1;
        }
    }
}

// ─── Patch_ToggleInverted (0x4465c0) — Curve→Negative: vertical-flip selection ─
//   For every selected patch: vertical-flip its control grid (patchInvert2) and
//   rebuild the tessellated render mesh (free + Patch_GenericMesh2, ++version).
//   The IDB does NOT bbox-rebuild here (a vertical flip leaves the bounds
//   unchanged), so — unlike Patch_Transpose — there is no Patch_Rebuild/
//   Brush_RebuildBrush call.  The trailing patch-inspector dialog refresh
//   (g_PatchDialog.GetPatchInfo) is deferred with the rest of patchdialog.cpp.
void Patch_ToggleInverted()
{
    char bUpdate = 0;
    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes;
          b = b->next )
    {
        if ( !b->patch )
            continue;
        // patch instance's first member is its patchMesh_t* def (patch_t::def);
        // == b->def->patch by the invariant below (cf. brush.cpp:5278 idiom).
        patchMesh_t *def = b->patch->def;
        iassert( b->patch->def == b->def->patch );
        patchInvert2( def );
        if ( def->curveDef )
            free( def->curveDef );
        def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
        bUpdate = 1;
        ++def->version;
    }
    if ( bUpdate )
        g_nUpdateBits = -1;

    // IDA 0x44666b: refresh the patch-inspector dialog (unconditional tail, outside the selection
    // block).  Parked GUI no-op headless (GetHwnd() returns null); ported here for 1:1 control flow
    // + consistency with Patch_Move's identical tail.
    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
}

// ─── Patch_TransposeGrid (sub_449020, 0x449020) — in-place control-grid TRANSPOSE ──
//   The matrix-transpose half of Patch_Transpose: reflects ctrl[c][r] across the
//   diagonal (ctrl[i][j] <-> ctrl[j][i]), then swaps width<->height.  The IDB splits
//   the reflection into two orientations (width<=height vs width>height) so the
//   rectangular cells whose mirror lies OUTSIDE the original grid are COPIED into
//   their soon-to-be-valid slot rather than swapped.  Terrain patches first toggle
//   every control vertex's turned_edge bit0.  Transcribed from the IDB flat-pointer
//   walk (ctrl @ patchMesh_t+56, drawVert 80B, 16-wide backing rows).
static patchMesh_t *Patch_TransposeGrid( patchMesh_t *p )
{
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
        for ( int c = 0; c < p->width; ++c )
            for ( int r = 0; r < p->height; ++r )
                p->ctrl[c][r].turned_edge ^= 1u;

    if ( p->width <= p->height )
    {
        for ( int c = 0; c < p->width; ++c )
            for ( int r = c + 1; r < p->height; ++r )
            {
                if ( r < p->width )                       // mirror in-grid: swap
                {
                    drawVert_t tmp = p->ctrl[c][r];
                    p->ctrl[c][r]  = p->ctrl[r][c];
                    p->ctrl[r][c]  = tmp;
                }
                else                                      // mirror off-grid: copy up
                    p->ctrl[r][c] = p->ctrl[c][r];
            }
    }
    else
    {
        for ( int c = 0; c < p->height; ++c )
            for ( int r = c + 1; r < p->width; ++r )
            {
                if ( r < p->height )                      // mirror in-grid: swap
                {
                    drawVert_t tmp = p->ctrl[c][r];
                    p->ctrl[c][r]  = p->ctrl[r][c];
                    p->ctrl[r][c]  = tmp;
                }
                else                                      // mirror off-grid: copy up
                    p->ctrl[c][r] = p->ctrl[r][c];
            }
    }

    int t     = p->height;
    p->height = p->width;
    p->width  = t;
    return p;
}

// ─── Patch_Transpose (0x4491D0) — Curve→Matrix Transpose: transpose selected patches ─
//   For every selected patch: transpose the control grid (Patch_TransposeGrid) THEN
//   vertical-flip it (patchInvert2 — the IDB pairs the two), rebuild the tessellated
//   curveDef (free + Patch_GenericMesh2, ++version), then Patch_Rebuild(def,1) (bbox +
//   curveDef rebuild — a transpose changes the bounds, unlike a pure flip).  The IDB
//   does the inline curveDef rebuild AND Patch_Rebuild (so version bumps twice and the
//   mesh builds twice); kept verbatim.  g_nUpdateBits is set by the OnCurveMatrixTranspose
//   handler, not here.
void Patch_Transpose()
{
    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes;
          b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *def = b->patch->def;     // patch_t::def (== b->def->patch)
        iassert( b->patch->def == b->def->patch );
        Patch_TransposeGrid( def );
        patchInvert2( def );
        if ( def->curveDef )
            free( def->curveDef );
        def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
        ++def->version;
        Patch_Rebuild( def, 1 );
    }
}

// ─── Patch_Duplicate (0x4486A0) — deep-copy a patch + its symbiont brush ──────
//   Make a fresh patch, copy the control grid + header, rebuild the tessellated
//   curveDef, and create a NEW symbiont brush (AddBrushForPatch) owned by the source's
//   owner.  Reached by the undo/clone path (Brush_FullClone 0x475E80, Brush_Clone,
//   Brush_CloneComplex), so undo of ANY patch edit routes through here — was a FATAL
//   stub (engine_stubs.cpp), which crashed the first Undo-bracketed patch command.
patchMesh_t *Patch_Duplicate( patchMesh_t *pFrom )
{
    patchMesh_t *p = MakeNewPatch();
    memcpy( p, pFrom, sizeof( patchMesh_t ) );
    p->curveDef = 0;
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    AddBrushForPatch( p, (entity_s *)pFrom->pSymbiot->owner );
    return p;
}

// Undo bracket helpers (undo.cpp) — used by Patch_NaturalizeSelected.
extern void Undo_ClearRedo();                          // 0x45DF20
extern void Undo_GeneralStart( const char *operation );// 0x45E3F0
extern void Undo_AddBrushList( selbrush_t *sb );       // 0x45E7C0
extern void Undo_EndBrushList( selbrush_t *sb );       // 0x45E870
extern void Undo_End();                                // 0x45EA20 (also declared below)
extern void Select_SetTexture( float *out );           // 0x456D70 select.cpp (layer sample size)

// ════════════════════════════════════════════════════════════════════════════
//  CAP / NATURAL TEXTURING CHAIN
//  Patch→Cap (and the cap arm of Patch→Naturalize): project a reference face's texture
//  plane across the whole control grid.  All geometry — the only "material" touch is
//  reading the reference material's texel width/height (MaterialDef_GetLayeredMaterial,
//  real, with a 512 fallback) and the already-real Face_MoveTexture projection.
// ════════════════════════════════════════════════════════════════════════════

extern void  Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                               int uvBase, float rotate, float crossterm );   // brush.cpp 0x45A1C0
extern float Vec3Normalize_R( float *v );                                     // 0x40A5E0 (returns length)

int g_nFaceCycle = 0;   // 0x25D5B10 — Patch_GetAxisFace's round-robin face cursor

// 8-connected neighbour directions (di=col, dj=row), IDB unk_73B130 — the order sets the
// cross-product winding so the accumulated vertex normal matches the binary.
static const int s_patchNeighborDirs[8][2] =
{ {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1} };

// ── Patch_MeshNormals (0x437c80) — recompute every control point's smooth normal ──
//  For each control point, gather edge vectors to its 8 neighbours (walking out up to 3
//  steps past coincident points), cross adjacent edges to get triangle normals, sum and
//  normalize.  Honors width/height "closed" (cylinder) wrap when the seam columns/rows
//  coincide (dist <= 1).  Output (ctrl[].normal) drives rendering only — not serialized.
static patchMesh_t *Patch_MeshNormals( patchMesh_t *p )
{
    const int width  = p->width;
    const int height = p->height;

    // closed-in-width: first and last COLUMN coincide across all rows (<= 1 unit).
    int closedW = ( height > 0 );
    for ( int j = 0; j < height; ++j )
    {
        const float *a = p->ctrl[0][j].xyz;
        const float *b = p->ctrl[width - 1][j].xyz;
        float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
        if ( (float)sqrt( dx * dx + dy * dy + dz * dz ) > 1.0f ) { closedW = 0; break; }
    }
    // closed-in-height: first and last ROW coincide across all columns.
    int closedH = ( width > 0 );
    for ( int i = 0; i < width; ++i )
    {
        const float *a = p->ctrl[i][0].xyz;
        const float *b = p->ctrl[i][height - 1].xyz;
        float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
        if ( (float)sqrt( dx * dx + dy * dy + dz * dz ) > 1.0f ) { closedH = 0; break; }
    }

    for ( int col = 0; col < width; ++col )
    {
        for ( int row = 0; row < height; ++row )
        {
            const float *center = p->ctrl[col][row].xyz;
            float edges[8][3];
            int   valid[8];

            for ( int d = 0; d < 8; ++d )
            {
                edges[d][0] = edges[d][1] = edges[d][2] = 0.0f;
                valid[d] = 0;
                const int di = s_patchNeighborDirs[d][0];
                const int dj = s_patchNeighborDirs[d][1];

                for ( int step = 1; step <= 3; ++step )
                {
                    int c = col + di * step;
                    int r = row + dj * step;
                    if ( closedW )
                    {
                        if ( c >= 0 ) { if ( c >= width ) c = c - width + 1; }
                        else            c = c + width - 1;
                    }
                    if ( closedH )
                    {
                        if ( r >= 0 ) { if ( r >= height ) r = r - height + 1; }
                        else            r = r + height - 1;
                    }
                    if ( c < 0 || c >= width || r < 0 || r >= height )
                        break;
                    const float *nb = p->ctrl[c][r].xyz;
                    float e[3] = { nb[0] - center[0], nb[1] - center[1], nb[2] - center[2] };
                    if ( Vec3Normalize_R( e ) != 0.0f )
                    {
                        edges[d][0] = e[0]; edges[d][1] = e[1]; edges[d][2] = e[2];
                        valid[d] = 1;
                        break;
                    }
                }
            }

            float acc[3] = { 0.0f, 0.0f, 0.0f };
            for ( int d = 0; d < 8; ++d )
            {
                int dn = ( d + 1 ) & 7;
                if ( valid[d] && valid[dn] )
                {
                    float fn[3];
                    Vec3Cross( edges[dn], edges[d], fn );
                    if ( Vec3Normalize_R( fn ) != 0.0f )
                    {
                        acc[0] += fn[0]; acc[1] += fn[1]; acc[2] += fn[2];
                    }
                }
            }
            Curve_NormalizeTo( p->ctrl[col][row].normal, acc );
        }
    }
    return p;
}

// ── PMESH_03 (0x439990) — pick a cap reference face (first long-edged face) ────
//  Returns the symbiont brush's first face whose first winding edge spans > 8 units in
//  any axis (else face[0]).  The auto-cap projection reference.
static face_t *PMESH_03( patchMesh_t *p )
{
    iassert( p );                                   // 1079 — #p byte-matches embedded "p"
    brush_t *b = (brush_t *)p->pSymbiot;
    iassert( b );                                   // 1081 — #b byte-matches embedded "b"

    if ( b->faceCount == 0 )
        return b->faces;

    for ( int i = 0; i < b->faceCount; ++i )
    {
        winding_t *w = b->faces[i].w;
        iassert( w );                               // 1085 — #w byte-matches embedded "w"
        const float *p0 = w->p[0];
        const float *p1 = w->p[1];
        int spans = ( 8.0f < p1[0] - p0[0] )
                  + ( 8.0f < p1[1] - p0[1] )
                  + ( 8.0f < p1[2] - p0[2] );
        if ( spans > 0 )
            return &b->faces[i];
    }
    return b->faces;
}

// ── Patch_GetAxisFace (0x439ab0) — round-robin a cap face (manual cap cycle) ───
static face_t *Patch_GetAxisFace( patchMesh_t *p )
{
    brush_t *b = (brush_t *)p->pSymbiot;
    unsigned int faceCount = (unsigned int)b->faceCount;

    unsigned int idx = (unsigned int)++g_nFaceCycle;
    unsigned int cap = ( faceCount <= 6 ) ? faceCount : 6;
    if ( idx >= cap ) { idx = 0; g_nFaceCycle = 0; }
    if ( idx >= faceCount )
    {
        Assert( PMESH_CPP, 1115, 1, "%s", "g_nFaceCycle >= 0 && g_nFaceCycle < b->faceCount" );
        idx = g_nFaceCycle;
    }
    return &b->faces[idx];
}

// ── Patch_ST (0x4390d0) — project every control point's xyz onto the texture plane ─
//  Build the 2×4 texture projection (Face_MoveTexture) from `params`
//  {sizeX,sizeY,shiftX,shiftY,rotate,crossterm} + the reference face normal, then write
//  each control point's layer S/T.  The low-12-bits "clean" snap removes fp noise.
static void Patch_ST( const float *params, const float *faceNormal, patchMesh_t *p, int layer )
{
    float mat[8];
    Face_MoveTexture( (int)(intptr_t)params, faceNormal, (int)(intptr_t)mat,
                      (int)(intptr_t)( params + 2 ), params[4], params[5] );

    for ( int col = 0; col < p->width; ++col )
    {
        for ( int row = 0; row < p->height; ++row )
        {
            drawVert_t *cp  = &p->ctrl[col][row];
            const float *xyz = cp->xyz;
            float *st = &cp->texCoord.st[2 * layer];

            float s = mat[1] * xyz[1] + mat[0] * xyz[0] + mat[2] * xyz[2] + mat[3];
            unsigned int sb = *(unsigned int *)&s;
            unsigned int slo = sb & 0xFFF;
            if ( slo > 4 ) { if ( (int)( 4096 - slo ) <= 4 ) sb += ( 4096 - slo ); }
            else            sb -= slo;
            s = *(float *)&sb;
            if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 796, 0, "%s", "!IS_NAN(st[0])" );
            st[0] = s;

            float t = mat[4] * xyz[0] + mat[5] * xyz[1] + mat[6] * xyz[2] + mat[7];
            unsigned int tb = *(unsigned int *)&t;
            unsigned int tlo = tb & 0xFFF;
            if ( tlo > 4 ) { if ( (int)( 4096 - tlo ) <= 4 ) tb += ( 4096 - tlo ); }
            else            tb -= tlo;
            t = *(float *)&tb;
            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 801, 0, "%s", "!IS_NAN(st[1])" );
            st[1] = t;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  PMESH_34 (0x4428f0) — apply the current material to a PATCH on drag-texture /
//  whole-brush texture apply.  Reached from sub_476ED0 (brush.cpp) when the brush
//  carries a patch and a5==0 (the drag-texture-onto-a-patch and patch-instance
//  whole-brush apply paths).  Two things happen:
//   1) Always: stamp the clicked material's two ptrs (lyrMtl/radMtl) into the
//      patch's per-layer texture slots and bump version.  The first arg `a1` here is
//      the SAME MaterialDef block as sub_476ED0's a2 — the IDB reads its first two
//      DWORDs (a1->width / a1->height in the curvePatchDef_t cast == lyrMtl/radMtl
//      of the MaterialDef; the union aliasing the disasm uses) into the texture
//      slots.  (Disasm: `mov [edi+layer*8+0x18], [esi]` / `[edi+layer*8+0x1C], [esi+4]`.)
//   2) Conditionally project: if a4!=0 AND the current texture template
//      (random_texture_stuff[layer]) has a stored ST-projection (flag @ +40) whose
//      stored grid w/h (@ +44/+48) match the patch's, copy that stored per-control-
//      point ST block (@ +52, the SAME block Texture_SetTexture writes — see
//      texwnd.cpp) onto the patch's control grid texCoords, then rebuild the
//      tessellated curveDef.  Otherwise, if a3 (planar) is set, planar-project via
//      Ed_Patch_PlanarTest + Patch_ApplyCapST (sub_439280).  Tail: refresh the patch
//      inspector if open (GUI-only; null headless).
//
//  Raw-offset idiom (transcribed verbatim from the disasm):
//    * the projection store block lives at random_texture_stuff[layer] + 52 (cur+0x34),
//      stepping +8 bytes per column, +128 bytes per row — the exact mirror of
//      Texture_SetTexture's writer (texwnd.cpp curBytes+52 / v8=v3<<7).
//    * the patch control-grid texCoord destination is the raw-float-stride
//      &p->ctrl[0][0].texCoord.st[2*idx + 2*layer] (idx steps +10 per column, +160 per
//      row), identical to Patch_Naturalize2's `&p->ctrl[i][j].texCoord.st[2*a2]`.
// ════════════════════════════════════════════════════════════════════════════
extern char Ed_Patch_PlanarTest( patchMesh_t *patch, void *outBlk );   // brush.cpp (0x4382D0)
extern CWnd *g_PatchDialog_GetHwnd();                                  // patchdialog.cpp shim
extern void  g_PatchDialog_GetPatchInfo();
static curvePatchDef_t *Patch_ApplyCapST( patchMesh_t *p, const float *faceNormal, const float *params ); // sub_439280 (below)

char PMESH_34( MaterialDef *a1, patchMesh_t *a2, char a3, float a4 )
{
    const int layer = g_qeglobals.current_edit_layer;

    // 1) Stamp the material's two ptrs into the patch's per-layer texture slots.
    patchMesh_material *texSlot = &a2->texture + layer;
    texSlot->lyrMtl = a1->lyrMtl;                                                   // 0x442902
    texSlot->radMtl = a1->radMtl;                                                   // 0x44290e
    ++a2->version;                                                                  // 0x442912

    // the stored ST-projection block in the current-texture template.
    curTexWndLayer_t *cur = &g_qeglobals.random_texture_stuff[layer];

    if ( a4 != 0.0f && cur->hasProjection )                                         // 0x44291a/0x44292f
    {
        if ( a2->width  == cur->gridWidth                                           // 0x44293e
          && a2->height == cur->gridHeight )                                        // 0x44294d
        {
            int v7 = 0, v17 = 0;
            if ( a2->width > 0 )                                                    // 0x442960
            {
                int v8 = 0, v16 = 0;
                do                                                                  // outer: columns (v7 < width)
                {
                    int v18 = 0;
                    if ( a2->height > 0 )                                           // 0x44296b
                    {
                        int v10 = v7 << 7;                                          // 0x44297c row byte offset in block
                        do                                                          // inner: rows (v18 < height)
                        {
                            const float *stp = (const float *)( (char *)cur->st + v10 );  // 0x44298a cur+0x34+v10
                            float s = stp[0];
                            float t = stp[1];
                            if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                                Assert( PMESH_CPP, 4624, 0, "%s", "!IS_NAN(s)" );
                            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                                Assert( PMESH_CPP, 4625, 0, "%s", "!IS_NAN(t)" );
                            a2->ctrl[0][0].texCoord.st[2 * v8     + 2 * layer] = s; // 0x442a15
                            a2->ctrl[0][0].texCoord.st[2 * v8 + 1 + 2 * layer] = t; // 0x442a25
                            v10 += 8;                                              // next column src
                            v8  += 10;                                            // next control vert (80 bytes / 8)
                            ++v18;
                        }
                        while ( v18 < a2->height );                                // 0x442a3b
                        v7 = v17;
                    }
                    ++v7;
                    v8 = v16 + 160;                                                // next row dest base
                    v17 = v7;
                    v16 += 160;
                }
                while ( v7 < a2->width );                                          // 0x442a58
            }

            if ( layer == 1 )                                                      // 0x442a5e
                a2->bDirty = 0;
            if ( a2->curveDef )                                                    // 0x442a6e
                free( a2->curveDef );
            a2->curveDef = Patch_GenericMesh2( a2, layer, 0, 0 );                  // 0x442a8e
            ++a2->version;                                                         // 0x442a96
        }
    }
    else if ( a3 )                                                                 // 0x442aaa planar project
    {
        char planeBlk[32];                                                        // {float N[3]@0, double dist@16}
        if ( Ed_Patch_PlanarTest( a2, planeBlk ) )                                // 0x442ab1
        {
            int curLayer = LayerMat::GetCurrentLayer( a1 );                       // 0x442abd
            Patch_ApplyCapST( a2, (const float *)planeBlk,                        // 0x442ad5 (sub_439280)
                              (const float *)( &a1->mat_texDef + curLayer ) );
        }
    }

    if ( g_PatchDialog_GetHwnd() )                                                 // 0x442add CWnd_PatchDialog.m_hWnd
        g_PatchDialog_GetPatchInfo();                                             // 0x442aeb refresh patch inspector
    return 0;
}

// ── sub_439280 (0x439280) — Patch_ST + rebuild the tessellated curveDef ───────
static curvePatchDef_t *Patch_ApplyCapST( patchMesh_t *p, const float *faceNormal, const float *params )
{
    Patch_ST( params, faceNormal, p, g_qeglobals.current_edit_layer );
    if ( g_qeglobals.current_edit_layer == 1 )
        p->bDirty = 0;
    if ( p->curveDef )
        free( p->curveDef );
    curvePatchDef_t *result = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    ++p->version;
    p->curveDef = result;
    return result;
}

// ─── Patch_CapTexture (0x439B10) — patch CAP texturing ────────────────────────
//  Reached by Patch_NaturalizeSelected's unk=1 branch (the "Apply Patch Cap" command).
//  Recompute mesh normals, pick a reference face (Patch_GetAxisFace round-robin when
//  cap, else PMESH_03's first long face), read its layer material's texel size (512
//  fallback), and project that face's texture plane (scaled by x/y) across the grid.
curvePatchDef_t *Patch_CapTexture( patchMesh_t *p, char cap, float x, float y )
{
    Patch_MeshNormals( p );

    face_t *axisFace = cap ? Patch_GetAxisFace( p ) : PMESH_03( p );

    qtexture_s *lm = MaterialDef_GetLayeredMaterial(
                         &axisFace->mtldef[g_qeglobals.current_edit_layer] );
    int width  = lm ? lm->width  : 512;
    int height = lm ? lm->height : 512;

    float params[6];
    params[0] = (float)width  * x;
    params[1] = (float)height * y;
    params[2] = params[3] = params[4] = params[5] = 0.0f;

    return Patch_ApplyCapST( p, axisFace->plane.normal, params );
}

// ─── Patch_NaturalizeSelected (0x447FD0) — Patch→Naturalize core ──────────────
//   For each selected patch (Undo-bracketed): unk=0 → Patch_Naturalize2 lays linear
//   per-control-point S/T at scale (x,y) on the current edit layer, then rebuilds the
//   tessellated curveDef (free + Patch_GenericMesh2) and bumps version.  unk=1 routes
//   to Patch_CapTexture (cap texturing, deferred).  No-op when no patch is selected
//   (the IDB scans for the first selected `patch` and returns early if none).
void Patch_NaturalizeSelected( bool unk, bool cap, float x, float y )
{
    selbrush_t *first = selected_brushes.next;
    if ( first == &selected_brushes )
        return;
    while ( !first->patch )
    {
        first = first->next;
        if ( first == &selected_brushes )
            return;
    }

    Undo_ClearRedo();
    Undo_GeneralStart( cap ? "Patch cap texturing" : "Patch natural texturing" );
    Undo_AddBrushList( &selected_brushes );

    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes;
          b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *def = b->patch->def;     // patch_t::def (== b->def->patch)
        iassert( b->patch->def == b->def->patch );
        if ( unk )
        {
            Patch_CapTexture( def, (char)cap, x, y );
        }
        else
        {
            Patch_Naturalize2( def, g_qeglobals.current_edit_layer, x, y );
            if ( def->curveDef )
                free( def->curveDef );
            def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
            ++def->version;
        }
    }

    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ─── Patch_Lightmap_Texturing_Sub (0x4397B0) — re-lay a single patch's lightmap-layer S/T ──
//   Reads the current layer's sample size (Select_SetTexture), then projects the lightmap-
//   layer texCoords either by the TERRAIN dominant-axis path (Patch_TerrainTexProject) or
//   the curve sample-size resampler (PMESH_02).  When the edit layer is the lightmap layer
//   (1) it also stamps the sample size into the patch (bDirty + size_of_struct_0x504C@0x5048,
//   a __int16 version bump@0x5040).  Finally rebuilds the tessellated curveDef and bumps the
//   version.  (Faithful to the disasm: the layer passed to Patch_Terrain/PMESH_02 is the
//   current edit layer in ecx, not a fixed lightmap layer.)
static curvePatchDef_t *Patch_Lightmap_Texturing_Sub( patchMesh_t *def )
{
    float v4[2];
    Select_SetTexture( v4 );

    const int layer = g_qeglobals.current_edit_layer;
    if ( ( def->type & PATCH_TERRAIN ) != 0 )
        Patch_TerrainTexProject( def, layer, v4[0] );
    else
        PMESH_02( def, layer, v4[0] );

    if ( layer == 1 )
    {
        def->bDirty = 1;
        *(float *)&def->size_of_struct_0x504C = v4[0];   // 0x5048 stores the sample size
    }

    if ( def->curveDef )
        free( def->curveDef );
    curvePatchDef_t *result = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
    ++def->version;
    def->curveDef = result;
    return result;
}

// ─── Patch_Lightmap_Texturing (0x448110) — Surface Inspector "Lightmap" for patches ───
//   For each selected patch (Undo-bracketed), re-lays its lightmap-layer texCoords via
//   Patch_Lightmap_Texturing_Sub.  No-op when no patch is selected (scans for the first
//   selected `patch` and returns early if none).
void Patch_Lightmap_Texturing()
{
    selbrush_t *first = selected_brushes.next;
    if ( first == &selected_brushes )
        return;
    while ( !first->patch )
    {
        first = first->next;
        if ( first == &selected_brushes )
            return;
    }

    Undo_ClearRedo();
    Undo_GeneralStart( "Patch lightmap texturing" );
    Undo_AddBrushList( &selected_brushes );

    for ( selbrush_t *b = selected_brushes.next;
          b != &selected_brushes;
          b = b->next )
    {
        if ( !b->patch )
            continue;
        iassert( b->patch->def == b->def->patch );
        Patch_Lightmap_Texturing_Sub( b->patch->def );
    }

    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ─── Patch_Naturalize (0x439920) — natural texturing at the editor default scale ──
//   Thin wrapper: Patch_Naturalize2 at the current layer's default tex-repeat scale
//   (random_texture_stuff[2100*layer+36]) + curveDef rebuild + ++version.  Used by the
//   Redisperse ops below (re-lays texCoords after the control grid is redistributed).
static void Patch_Naturalize( patchMesh_t *p )
{
    const float scale = g_qeglobals.random_texture_stuff[g_qeglobals.current_edit_layer].sampleSize;
    Patch_Naturalize2( p, g_qeglobals.current_edit_layer, scale, scale );
    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    ++p->version;
}

// ─── Patch_DisperseColumns (0x4443A0) — Curve→Redisperse Cols (cmd 32889) ─────
//   For each selected patch (Undo-bracketed): rebuild, then redistribute each ROW's
//   control points EVENLY along the chord from column 0 to width-1 (cumulative
//   ctrl[j][r] = ctrl[j-1][r] + (ctrl[w-1][r]-ctrl[0][r])/(w-1)), then re-naturalize once.
void Patch_DisperseColumns()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "disperse columns" );
    Undo_AddBrushList( &selected_brushes );

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *def = b->patch->def;
        iassert( b->patch->def == b->def->patch );
        Patch_Rebuild( def, 1 );
        for ( int r = 0; r < def->height; ++r )
        {
            if ( def->width <= 1 )
                continue;
            // IDA: recip = (float)(1.0/(width-1)); step = recip * delta — a float
            // reciprocal MULTIPLY, not a divide (float-exact; differs by 1 ULP).
            float recip = (float)( 1.0 / (double)( def->width - 1 ) );
            float step[3];
            for ( int k = 0; k < 3; ++k )
                step[k] = recip * ( def->ctrl[def->width - 1][r].xyz[k] - def->ctrl[0][r].xyz[k] );
            for ( int j = 1; j < def->width; ++j )
                for ( int k = 0; k < 3; ++k )
                    def->ctrl[j][r].xyz[k] = def->ctrl[j - 1][r].xyz[k] + step[k];
        }
        Patch_Naturalize( def );                 // once per patch (faithful to 0x4443A0)
    }

    Undo_EndBrushList( &selected_brushes );
    Undo_End();
    if ( g_PatchDialog_GetHwnd() )               // 0x444533: refresh patch inspector
        g_PatchDialog_GetPatchInfo();
}

// ─── Patch_DisperseRows (0x444200) — Curve→Redisperse Rows (cmd 32888) ────────
//   The row analogue: redistribute each COLUMN's control points along the chord from
//   row 0 to height-1.  NOTE the IDB calls Patch_Naturalize PER COLUMN (inside the
//   column loop), not once — transcribed faithfully (the final state is identical, each
//   call fully re-lays texCoords from the current geometry).
void Patch_DisperseRows()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "disperse rows" );
    Undo_AddBrushList( &selected_brushes );

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *def = b->patch->def;
        iassert( b->patch->def == b->def->patch );
        Patch_Rebuild( def, 1 );
        for ( int c = 0; c < def->width; ++c )
        {
            if ( def->height > 1 )
            {
                // IDA: recip = (float)(1.0/(height-1)); step = recip * delta (float
                // reciprocal multiply, not a divide — float-exact).
                float recip = (float)( 1.0 / (double)( def->height - 1 ) );
                float step[3];
                for ( int k = 0; k < 3; ++k )
                    step[k] = recip * ( def->ctrl[c][def->height - 1].xyz[k] - def->ctrl[c][0].xyz[k] );
                for ( int j = 1; j < def->height; ++j )
                    for ( int k = 0; k < 3; ++k )
                        def->ctrl[c][j].xyz[k] = def->ctrl[c][j - 1].xyz[k] + step[k];
            }
            Patch_Naturalize( def );             // per-column (faithful to 0x444200)
        }
    }

    Undo_EndBrushList( &selected_brushes );
    Undo_End();
    if ( g_PatchDialog_GetHwnd() )               // 0x44437a: refresh patch inspector
        g_PatchDialog_GetPatchInfo();
}

// ─── Patch_InvertTexture (0x446680) — Curve→Negative Texture X/Y (32899/32903) ─
//   Mirror the CURRENT edit layer's texture-coord (st) pair of every selected patch:
//     axis 0 (TextureX, 32899) mirrors across ROWS    (ctrl[c][j] <-> ctrl[c][h-1-j]);
//     axis 1 (TextureY, 32903) mirrors across COLUMNS (ctrl[i][r] <-> ctrl[w-1-i][r]).
//   xyz is untouched (texture-only flip); curveDef rebuilt + ++version per patch.  An
//   involution — inverting the same axis twice restores the original st.  Derived from the
//   IDB flat-float walk (ctrl@+56, texCoord@+12, st index 2*layer).  The IDB's per-element
//   IS_NAN guard asserts (5913/5914/5916/5917 columns, 5929/5930/5932/5933 rows; LEVEL 0,
//   defensive-only — never fire for valid st) are transcribed faithfully, interleaved with
//   the swap exactly as the binary: assert the source (a) before c=a, the saved temp before a=fTemp.
void Patch_InvertTexture( char axis )
{
    const int layer = g_qeglobals.current_edit_layer;
    char did = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *def = b->patch->def;
        iassert( b->patch->def == b->def->patch );
        did = 1;
        if ( axis )                                  // mirror COLUMNS (TextureY)
        {
            for ( int r = 0; r < def->height; ++r )
                for ( int i = 0; i < def->width / 2; ++i )
                {
                    float *a = &def->ctrl[i][r].texCoord.st[2 * layer];
                    float *c = &def->ctrl[def->width - 1 - i][r].texCoord.st[2 * layer];
                    float t0 = c[0], t1 = c[1];
                    if ( ( *(unsigned int *)&a[0] & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5913, 0, "%s", "!IS_NAN(p->ctrl[col][row].qv.texCoord[g_qeglobals.activeTexCoord][0])" );
                    if ( ( *(unsigned int *)&a[1] & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5914, 0, "%s", "!IS_NAN(p->ctrl[col][row].qv.texCoord[g_qeglobals.activeTexCoord][1])" );
                    c[0] = a[0]; c[1] = a[1];
                    if ( ( *(unsigned int *)&t0 & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5916, 0, "%s", "!IS_NAN(fTemp[0])" );
                    if ( ( *(unsigned int *)&t1 & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5917, 0, "%s", "!IS_NAN(fTemp[1])" );
                    a[0] = t0;   a[1] = t1;
                }
        }
        else                                         // mirror ROWS (TextureX)
        {
            for ( int col = 0; col < def->width; ++col )
                for ( int j = 0; j < def->height / 2; ++j )
                {
                    float *a = &def->ctrl[col][j].texCoord.st[2 * layer];
                    float *c = &def->ctrl[col][def->height - 1 - j].texCoord.st[2 * layer];
                    float t0 = c[0], t1 = c[1];
                    if ( ( *(unsigned int *)&a[0] & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5929, 0, "%s", "!IS_NAN(p->ctrl[col][row].qv.texCoord[g_qeglobals.activeTexCoord][0])" );
                    if ( ( *(unsigned int *)&a[1] & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5930, 0, "%s", "!IS_NAN(p->ctrl[col][row].qv.texCoord[g_qeglobals.activeTexCoord][1])" );
                    c[0] = a[0]; c[1] = a[1];
                    if ( ( *(unsigned int *)&t0 & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5932, 0, "%s", "!IS_NAN(fTemp[0])" );
                    if ( ( *(unsigned int *)&t1 & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 5933, 0, "%s", "!IS_NAN(fTemp[1])" );
                    a[0] = t0;   a[1] = t1;
                }
        }
        if ( layer == 1 )
            def->bDirty = 0;
        if ( def->curveDef )
            free( def->curveDef );
        def->curveDef = Patch_GenericMesh2( def, layer, 0, 0 );
        ++def->version;
    }
    if ( did )
        g_nUpdateBits = -1;
    if ( g_PatchDialog_GetHwnd() )               // 0x446ac2: refresh patch inspector
        g_PatchDialog_GetPatchInfo();
}

// ─── Patch_Scale (0x4427D0) — scale selected patch control points about `mid` ──
//   For each control point (in curve-point edit mode: only those in d_move_points[];
//   otherwise all): xyz[k] = (xyz[k] - mid[k]) * scale[k] + mid[k].  Optionally rebuilds.
//   Reached from Select_Scale's patch branch.  The trailing patch-inspector refresh
//   (CWnd_PatchDialog, 0x4428b9) goes through the GUI shim (no-op headless).
void Patch_Scale( patchMesh_t *p, const float *mid, const float *scale, char doRebuild )
{
    for ( int w = 0; w < p->width; ++w )
        for ( int row = 0; row < p->height; ++row )
        {
            if ( g_qeglobals.d_select_mode == sel_curvepoint )
            {
                bool selected = false;
                for ( int m = 0; m < g_qeglobals.d_num_move_points; ++m )
                    if ( &p->ctrl[w][row] == g_qeglobals.d_move_points[m] ) { selected = true; break; }
                if ( !selected )
                    continue;                    // not a picked control point → leave it
            }
            for ( int k = 0; k < 3; ++k )
                p->ctrl[w][row].xyz[k] = ( p->ctrl[w][row].xyz[k] - mid[k] ) * scale[k] + mid[k];
        }
    if ( doRebuild )
        Patch_Rebuild( p, 1 );
    if ( g_PatchDialog_GetHwnd() )               // 0x4428b9: refresh patch inspector
        g_PatchDialog_GetPatchInfo();
}

// ─── Patch_Move (0x441dd0) — translate an ENTIRE patch by a world delta ─────────
//   For every control point in the width×height grid: xyz[k] += move[k].  Unlike
//   Patch_Scale this is unconditional (no curve-point filter) — it is the whole-patch
//   translation reached from Brush_Move (the patch branch of MoveSelection), so dragging
//   a selected patch routed here and FATAL-stubbed before this port.  The IDA strides are
//   ctrl[col][row]: the outer (width) loop advances 320 vec_t / 0x500 (one row of 16
//   drawVerts), the inner (height) loop advances 20 vec_t / 0x50 (one drawVert) — exactly
//   the `drawVert_t ctrl[16][16]` layout (drawVert_t = 0x50, xyz @ +0), so the named
//   member access below hits the same bytes the binary does.  After translating, free +
//   rebuild the tessellated curveDef (verbatim brush.cpp:5341 / Patch_ShiftTexture regen)
//   and bump version.  Tail: the patch-inspector refresh (CWnd_PatchDialog) is GUI-only and
//   PARKED — g_PatchDialog_GetHwnd() is null headless, so the regenerated curveDef is
//   returned (matching the established brush.cpp:980/1010 dialog guard).
curvePatchDef_t *Patch_Move( const float *move, patchMesh_t *p )
{
    for ( int col = 0; col < p->width; ++col )
        for ( int row = 0; row < p->height; ++row )
        {
            p->ctrl[col][row].xyz[0] += move[0];
            p->ctrl[col][row].xyz[1] += move[1];
            p->ctrl[col][row].xyz[2] += move[2];
        }

    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
    ++p->version;

    // IDA: if (CWnd_PatchDialog.m_hWnd) return g_PatchDialog_GetPatchInfo(&CWnd_PatchDialog);
    // The patch dialog is parked (GetHwnd() == null headless), so return the new curveDef.
    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
    return p->curveDef;
}

// ─── Patch_DragScale (0x442b90) — drag-resize a patch (Drag_MoveSelection box edge) ──
//   Scale the patch so its bounding box grows by `vAmt` (the per-axis size delta the drag
//   added), then re-centre it back over `vMove`'s sign.  Algorithm (GtkRadiant Patch_DragScale):
//     1. vTemp = bbox(max-min).  If scaling an axis that has no depth (vTemp[i]==0) while the
//        drag moved that axis (vMove[i]!=0) → bail (return 0): can't scale a degenerate axis.
//     2. vMid = min + vTemp/2;  vScale[i] = vAmt[i] ? 1 + vAmt[i]/vTemp[i] : 1.
//     3. Patch_Scale(patch, vMid, vScale, 0) — scale about the box centre (no rebuild).
//     4. Re-bbox; vShift = (newExtent - vTemp)/2 (half the size change per axis).  For each
//        axis where the box actually grew (sq diff > 1e-6) negate vShift[i] when the drag
//        pulled the LOW edge (vAmt[i] != vMove[i]); then Patch_Move(vShift, patch) to keep
//        the dragged edge anchored.  If no axis grew, Patch_Move(vShift, patch) with the raw
//        (positive) half-shift.  Returns 1 on success.  Was a FATAL stub — the patch branch of
//        Drag_MoveSelection's box-edge resize crashed.
//   Reached only from drag.cpp:798 (MoveSelection patch resize).  vAmt comes in @<eax>.
int Patch_DragScale( float *vAmt, void *patchDefV, float *vMove )
{
    patchMesh_t *patch = (patchMesh_t *)patchDefV;
    float vMin[3], vMax[3], vMid[3], vScale[3], vTemp[3];

    Patch_CalcBounds( vMin, vMax, patch, 0 );
    for ( int i = 0; i < 3; ++i )
        vTemp[i] = vMax[i] - vMin[i];

    // degenerate-axis bail: can't scale a zero-depth axis the drag is trying to move.
    for ( int i = 0; i < 3; ++i )
        if ( vTemp[i] == 0.0f && vMove[i] != 0.0f )
            return 0;

    for ( int i = 0; i < 3; ++i )
        vMid[i] = vMin[i] + vTemp[i] * 0.5f;
    for ( int i = 0; i < 3; ++i )
        vScale[i] = ( vAmt[i] != 0.0f ) ? ( vAmt[i] / vTemp[i] + 1.0f ) : 1.0f;

    Patch_Scale( patch, vMid, vScale, 0 );

    // re-measure and compute the half-extent change per axis.
    Patch_CalcBounds( vMin, vMax, patch, 0 );
    float vShift[3];
    for ( int i = 0; i < 3; ++i )
        vShift[i] = ( ( vMax[i] - vMin[i] ) - vTemp[i] ) * 0.5f;

    // find the first axis that actually grew (squared diff above the IDB's 1e-6 epsilon).
    int grew = 3;
    for ( int i = 0; i < 3; ++i )
    {
        float d = vMove[i] - vAmt[i];
        if ( d * d > 9.9999998e-7f ) { grew = i; break; }
    }
    if ( grew < 3 )
    {
        // anchor the dragged edge: negate the shift on any axis pulled from its low edge.
        if ( vAmt[0] != vMove[0] ) vShift[0] = -vShift[0];
        if ( vAmt[1] != vMove[1] ) vShift[1] = -vShift[1];
        if ( vAmt[2] != vMove[2] ) vShift[2] = -vShift[2];
    }
    Patch_Move( vShift, patch );
    return 1;
}

// ─── Patch_ShiftTexture (0x446170) — shift selected patch control-point texcoords ──
//   Adds (s,t) to each control point's current-layer st (curve-point mode → only the ones
//   in d_move_points[]; otherwise all).  A |shift| >= 1 is scaled down by 10 first (the
//   IDB's coarse step → fine nudge), then curveDef is rebuilt.  Reached from
//   Brush_ShiftTexture's patch branch.
void Patch_ShiftTexture( patchMesh_t *p, float s, float t )
{
    // IDA: v9 = fabs(a2); if (v9 >= 1.0) a2 = a2 / 10.0  (double fabs + double divide).
    if ( fabs( (double)s ) >= 1.0 ) s = (float)( (double)s / 10.0 );
    if ( fabs( (double)t ) >= 1.0 ) t = (float)( (double)t / 10.0 );
    const int layer = g_qeglobals.current_edit_layer;
    for ( int w = 0; w < p->width; ++w )
        for ( int h = 0; h < p->height; ++h )
        {
            if ( g_qeglobals.d_select_mode == sel_curvepoint )
            {
                bool selected = false;
                for ( int m = 0; m < g_qeglobals.d_num_move_points; ++m )
                    if ( &p->ctrl[w][h] == g_qeglobals.d_move_points[m] ) { selected = true; break; }
                if ( !selected )
                    continue;
            }
            p->ctrl[w][h].texCoord.st[2 * layer]     += s;
            if ( ( *(unsigned int *)&p->ctrl[w][h].texCoord.st[2 * layer] & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5793, 0, "%s", "!IS_NAN(p->ctrl[w][h].qv.texCoord[g_qeglobals.activeTexCoord][0])" );
            p->ctrl[w][h].texCoord.st[2 * layer + 1] += t;
            if ( ( *(unsigned int *)&p->ctrl[w][h].texCoord.st[2 * layer + 1] & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5795, 0, "%s", "!IS_NAN(p->ctrl[w][h].qv.texCoord[g_qeglobals.activeTexCoord][1])" );
        }
    if ( layer == 1 )
        p->bDirty = 0;
    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, layer, 0, 0 );
    ++p->version;
}

// ─── Patch_RotateTexture (0x445B80) — rotate selected patch control-point texcoords ──
//   Rotate each control point's current-layer st by `deg` degrees about the texel-space
//   origin: sTex=st0*texW, tTex=st1*texH; (ns,nt)=rot(sTex,tTex); st0=ns/texW, st1=nt/texH.
//   texW/texH come from the layer's layered material (512 default).  Curve-point mode → only
//   the d_move_points[]; else all.  Reached from Brush_RotateTexture's patch branch — was a
//   FATAL stub.  (The IDB also calls Patch_CalcBounds here — its output is vestigial; kept for
//   the version bump.)  The IDB stores the angle to a FLOAT slot before cos/sin (float-precision
//   rotation) and value-print-asserts texSize>0 (5698/5699) — both transcribed faithfully.
void Patch_RotateTexture( patchMesh_t *p, float deg )
{
    const int layer = g_qeglobals.current_edit_layer;
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)( &p->texture + layer ) );
    float texW, texH;
    if ( lm )
    {
        texW = (float)lm->width;
        texH = (float)lm->height;
        if ( lm->width <= 0 )
            Assert( PMESH_CPP, 5698, 0, "%s\n\t(texSize[0]) = %i", "(texSize[0] > 0)", lm->width );
        if ( lm->height <= 0 )
            Assert( PMESH_CPP, 5699, 0, "%s\n\t(texSize[1]) = %i", "(texSize[1] > 0)", lm->height );
    }
    else
    {
        texW = 512.0f;
        texH = 512.0f;
    }

    float mins[3], maxs[3];
    Patch_CalcBounds( mins, maxs, p, false );          // IDB call (output vestigial here)

    // IDA: v13 (FLOAT slot) = deg*PI/180, then cos/sin(v13) — round the angle to float first.
    const float  rad = (float)( (double)deg * 3.141592741012573 / 180.0 );
    const float  c   = (float)cos( (double)rad );
    const float  s   = (float)sin( (double)rad );

    for ( int w = 0; w < p->width; ++w )
        for ( int h = 0; h < p->height; ++h )
        {
            if ( g_qeglobals.d_select_mode == sel_curvepoint )
            {
                bool selected = false;
                for ( int m = 0; m < g_qeglobals.d_num_move_points; ++m )
                    if ( &p->ctrl[w][h] == g_qeglobals.d_move_points[m] ) { selected = true; break; }
                if ( !selected )
                    continue;
            }
            float sTex = p->ctrl[w][h].texCoord.st[2 * layer]     * texW;
            float tTex = p->ctrl[w][h].texCoord.st[2 * layer + 1] * texH;
            float ns   = c * sTex - s * tTex;
            float nt   = sTex * s + tTex * c;
            if ( ( *(unsigned int *)&ns & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5713, 0, "%s", "!IS_NAN(s)" );
            if ( ( *(unsigned int *)&nt & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5714, 0, "%s", "!IS_NAN(t)" );
            p->ctrl[w][h].texCoord.st[2 * layer]     = ns / texW;
            p->ctrl[w][h].texCoord.st[2 * layer + 1] = nt / texH;
        }

    if ( layer == 1 )
        p->bDirty = 0;
    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, layer, 0, 0 );
    ++p->version;
}

// ─── Patch_ScaleTexture (0x445F80) — scale selected patch control-point texcoords ──
//   Multiply each control point's current-layer st by (sx,sy) — st0*=sx, st1*=sy (a zero
//   factor is treated as 1).  Curve-point mode → only the d_move_points[]; else all.  Then
//   rebuild curveDef.  Layer 1 (lightmap) additionally: when sx==sy, scale the per-patch
//   lightmap-scale accumulator (the mislabeled int field @0x5048, used as a float) by sx and
//   set bDirty; on a non-uniform scale clear bDirty.  Reached from Brush_ScaleTexture's patch
//   branch.  The IDB types a1 drawVert_t* (misaligned hex-rays labels); the
//   field offsets were decoded against the patchMesh_t layout.
void Patch_ScaleTexture( patchMesh_t *p, float sx, float sy )
{
    if ( sx == 0.0f ) sx = 1.0f;
    if ( sy == 0.0f ) sy = 1.0f;
    const int layer = g_qeglobals.current_edit_layer;
    for ( int w = 0; w < p->width; ++w )
        for ( int h = 0; h < p->height; ++h )
        {
            if ( g_qeglobals.d_select_mode == sel_curvepoint )
            {
                bool selected = false;
                for ( int m = 0; m < g_qeglobals.d_num_move_points; ++m )
                    if ( &p->ctrl[w][h] == g_qeglobals.d_move_points[m] ) { selected = true; break; }
                if ( !selected )
                    continue;
            }
            p->ctrl[w][h].texCoord.st[2 * layer]     *= sx;
            if ( ( *(unsigned int *)&p->ctrl[w][h].texCoord.st[2 * layer] & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5761, 0, "%s", "!IS_NAN(p->ctrl[w][h].qv.texCoord[g_qeglobals.activeTexCoord][0])" );
            p->ctrl[w][h].texCoord.st[2 * layer + 1] *= sy;
            if ( ( *(unsigned int *)&p->ctrl[w][h].texCoord.st[2 * layer + 1] & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5763, 0, "%s", "!IS_NAN(p->ctrl[w][h].qv.texCoord[g_qeglobals.activeTexCoord][1])" );
        }
    if ( sx == sy )
    {
        if ( layer == 1 )
        {
            p->bDirty = 1;
            // per-patch lightmap-scale accumulator @0x5048 (IDB a1[256].savedTexCoord.smoothing[1],
            // a float aliased over the size_of_struct_0x504C field).
            *(float *)&p->size_of_struct_0x504C = sx * *(float *)&p->size_of_struct_0x504C;
        }
    }
    else if ( layer == 1 )
    {
        p->bDirty = 0;
    }
    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, layer, 0, 0 );
    ++p->version;
}

// ─── Patch_SetTextureInfo (0x447760) — apply a relative texdef change to the selection ──
//   For every selected patch, dispatch to rotate / shift / scale by whichever texdef field
//   is non-zero (the Patch Inspector's texdef spin arrows feed one field at a time).
void Patch_SetTextureInfo( texdef_sub_t *texDef )
{
    for ( selbrush_t *pb = selected_brushes.next; pb != &selected_brushes; pb = pb->next )
    {
        patch_t *patch = pb->patch;
        if ( !patch )
            continue;
        if ( patch->def != pb->def->patch )
            Assert( PMESH_CPP, 6193, 0, "%s", "b->patch->def == b->def->patch" );
        patchMesh_t *def = patch->def;
        if ( texDef->rotate != 0.0f )
            Patch_RotateTexture( def, texDef->rotate );
        if ( texDef->shift[0] != 0.0f || texDef->shift[1] != 0.0f )
            Patch_ShiftTexture( def, texDef->shift[0], texDef->shift[1] );
        if ( texDef->size[0] != 0.0f || texDef->size[1] != 0.0f )
            Patch_ScaleTexture( def, texDef->size[0], texDef->size[1] );
    }
}

// ─── Patch_SetTexturing (0x446b60) — "Set..." texture layout across the selection ──
//   Lay texcoords across each selected patch on the current layer.  mode (a3):
//     2 (or non-terrain): parametric GRID — s = col*sx/(w-1), t = -row*sy/(h-1).
//     1: ARC-LENGTH — s/t follow the cumulative 3D edge length along width / height.
//     0: NATURAL — like arc-length but measured in the two dominant world axes (chosen
//        from the patch's bbox extents), giving a world-scaled projection.
//   The IDB's flat &ctrl[0][0].texCoord.st[col*320 + row*20 + ...] indexing is just
//   ctrl[col][row].texCoord.st[2*layer (+1)]; reads are ctrl[col][row].xyz.
static float Patch_TexLen3( const float *a, const float *b )
{
    const float dx = b[0] - a[0], dy = b[1] - a[1], dz = b[2] - a[2];
    return (float)sqrt( dx * dx + dy * dy + dz * dz );
}
static float Patch_TexLen2( const float *a, const float *b, int i0, int i1 )
{
    const float d0 = b[i0] - a[i0], d1 = b[i1] - a[i1];
    return (float)sqrt( d0 * d0 + d1 * d1 );
}
void Patch_SetTexturing( float sx, float sy, int mode )
{
    // bail if nothing in the selection owns a patch
    selbrush_t *scan = selected_brushes.next;
    while ( scan != &selected_brushes && !scan->patch )
        scan = scan->next;
    if ( scan == &selected_brushes )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "Patch set texturing" );
    Undo_AddBrushList( &selected_brushes );

    const int layer = g_qeglobals.current_edit_layer;
    for ( selbrush_t *pb = selected_brushes.next; pb != &selected_brushes; pb = pb->next )
    {
        patch_t *patch = pb->patch;
        if ( !patch )
            continue;
        if ( patch->def != pb->def->patch )
            Assert( PMESH_CPP, 5991, 0, "%s", "b->patch->def == b->def->patch" );
        patchMesh_t *def = patch->def;
        const int W = def->width, H = def->height;

        if ( mode == 2 || ( def->type & 0x40 ) == 0 )
        {
            for ( int col = 0; col < W; ++col )
                for ( int row = 0; row < H; ++row )
                {
                    // IDA computes col*sx/(W-1) UNCONDITIONALLY in double (a degenerate
                    // 1-wide/tall patch divides by zero -> the IS_NAN asserts below catch it).
                    float s = (float)( (double)col *   sx   / (double)( W - 1 ) );
                    float t = (float)( (double)row * ( -sy ) / (double)( H - 1 ) );
                    if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 6003, 0, "%s", "!IS_NAN(s)" );
                    if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 6004, 0, "%s", "!IS_NAN(t)" );
                    def->ctrl[col][row].texCoord.st[2 * layer]     = s;
                    def->ctrl[col][row].texCoord.st[2 * layer + 1] = t;
                }
        }
        else
        {
            // NATURAL: choose the two dominant world axes from the bbox extents.
            int ax0 = 0, ax1 = 1;
            if ( mode == 0 )
            {
                const float *mn = pb->def->mins, *mx = pb->def->maxs;
                const float ex = mx[0] - mn[0], ey = mx[1] - mn[1], ez = mx[2] - mn[2];
                if ( ex > ez * 0.5f && ey > ez * 0.5f ) { ax0 = 0; ax1 = 1; }     // X,Y
                else                                    { ax0 = ( ex <= ey ) ? 1 : 0; ax1 = 2; }  // (Y|X),Z
            }
            // s along width
            for ( int row = 0; row < H; ++row )
            {
                float total = 0.0f;
                for ( int col = 0; col < W - 1; ++col )
                    total += ( mode == 1 ) ? Patch_TexLen3( def->ctrl[col][row].xyz, def->ctrl[col + 1][row].xyz )
                                           : Patch_TexLen2( def->ctrl[col][row].xyz, def->ctrl[col + 1][row].xyz, ax0, ax1 );
                if ( total == 0.0f ) total = 1.0f;
                def->ctrl[0][row].texCoord.st[2 * layer] = 0.0f;
                float cum = 0.0f;
                for ( int col = 1; col < W; ++col )
                {
                    cum += ( mode == 1 ) ? Patch_TexLen3( def->ctrl[col - 1][row].xyz, def->ctrl[col][row].xyz )
                                         : Patch_TexLen2( def->ctrl[col - 1][row].xyz, def->ctrl[col][row].xyz, ax0, ax1 );
                    float newS = sx / total * cum;
                    if ( ( *(unsigned int *)&newS & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, ( mode == 1 ) ? 6036 : 6114, 0, "%s", "!IS_NAN(newTexCoord)" );
                    def->ctrl[col][row].texCoord.st[2 * layer] = newS;
                }
            }
            // t along height
            for ( int col = 0; col < W; ++col )
            {
                float total = 0.0f;
                for ( int row = 0; row < H - 1; ++row )
                    total += ( mode == 1 ) ? Patch_TexLen3( def->ctrl[col][row].xyz, def->ctrl[col][row + 1].xyz )
                                           : Patch_TexLen2( def->ctrl[col][row].xyz, def->ctrl[col][row + 1].xyz, ax0, ax1 );
                if ( total == 0.0f ) total = 1.0f;
                def->ctrl[col][0].texCoord.st[2 * layer + 1] = 0.0f;
                float cum = 0.0f;
                for ( int row = 1; row < H; ++row )
                {
                    cum += ( mode == 1 ) ? Patch_TexLen3( def->ctrl[col][row - 1].xyz, def->ctrl[col][row].xyz )
                                         : Patch_TexLen2( def->ctrl[col][row - 1].xyz, def->ctrl[col][row].xyz, ax0, ax1 );
                    float newT = ( -sy ) / total * cum;
                    if ( ( *(unsigned int *)&newT & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, ( mode == 1 ) ? 6062 : 6142, 0, "%s", "!IS_NAN(newTexCoord)" );
                    def->ctrl[col][row].texCoord.st[2 * layer + 1] = newT;
                }
            }
        }

        if ( layer == 1 )
            def->bDirty = 0;
        if ( def->curveDef )
            free( def->curveDef );
        def->curveDef = Patch_GenericMesh2( def, layer, 0, 0 );
        ++def->version;
    }
    Undo_EndBrushList( &selected_brushes );
    Undo_End();
}

// ─── Patch_FitTexturing (0x447600) — "Fit" texture onto a patch ──────────────────
//   Lay a single texture tile across the whole patch on the current layer: the s/t of
//   every control point becomes its NORMALIZED grid position — s = col/(width-1) runs
//   0→1 across columns, t = -row/(height-1) runs 0→-1 down rows (negative t, matching
//   the IDB).  No curve-point gating (unlike Shift/Scale/Rotate) — every control point
//   is set.  Then clear bDirty on the lightmap layer and free/rebuild curveDef + bump
//   version.  Reached from Brush_FitTextureFaces (sub_47C950) when the brush owns a
//   patch (the Surface Inspector "Fit" path) — was a FATAL tripwire.  The IDB walks the
//   ctrl array as a flat float[] (v1+=10 per row → 2*v1 = +20 floats = one drawVert row
//   stride; v6+=160 per column → +320 floats = 16-drawVert column stride); that resolves
//   exactly to ctrl[col][row].texCoord.st[2*layer{,+1}].  The IDB returns the rebuilt
//   curveDef but every caller ignores it, so this is void (cf. Patch_ScaleTexture).
void Patch_FitTexturing( patchMesh_t *p )
{
    const int layer = g_qeglobals.current_edit_layer;
    for ( int col = 0; col < p->width; ++col )
        for ( int row = 0; row < p->height; ++row )
        {
            float s = (float)(  (double)col / (double)( p->width  - 1 ) );  // IDA: double divide
            float t = (float)( -(double)row / (double)( p->height - 1 ) );
            if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 6171, 0, "%s", "!IS_NAN(s)" );
            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 6172, 0, "%s", "!IS_NAN(t)" );
            p->ctrl[col][row].texCoord.st[2 * layer]     = s;
            p->ctrl[col][row].texCoord.st[2 * layer + 1] = t;
        }
    if ( layer == 1 )
        p->bDirty = 0;
    if ( p->curveDef )
        free( p->curveDef );
    p->curveDef = Patch_GenericMesh2( p, layer, 0, 0 );
    ++p->version;
}

// ─── patch "undo snapshot" globals (IDB 0x231F558.. — a partial patchMesh_t image) ───
//   Written by Patch_Save (0x446AE0) + Patch_ApplyMatrix; read back by Patch_Restore
//   (0x446B20), Patch_BendHandleESC (0x447C00) and Patch_InsDelToggle (0x447E50) — the
//   bend/insert-delete cancel paths, none ported yet.  Defined here (the patch home) so
//   those functions can share them when they land.  ctrl image = 0x5000 bytes = the full
//   16×16 control grid (256 drawVert_t × 80).
int        patchSave_width;                 // 0x231F558
int        patchSave_height;                // 0x231F55C
int        patchSave_type;                  // 0x231F568
drawVert_t patchSave_ctrl[16][16];          // 0x231F590  (0x5000 bytes)

// ─── Patch_ApplyMatrix (0x441E70) — transform a patch's control points by an orientation ─
//   The patch arm of Select_ApplyMatrix (rotate / flip / scale of a selection containing a
//   patch).  Snapshot
//   the current grid into patchSave_*, then push every control point's xyz through
//   OrientationPosToWorldPos (rel = xyz - origin; xyz = origin + axisᵀ·rel) so it rotates/
//   reflects about orient->origin.  In curve-point or bend mode only the d_move_points[] are
//   moved; otherwise every control point.  `snap` rounds each coord to the nearest half-unit
//   (floor(0.5 + 2·c)·0.5).  Finally Patch_Rebuild(p,1) rebuilds the bbox + curveDef (++version).
//   IDB indexes ctrl[0][16*col+row] (flat) = ctrl[col][row].
void Patch_ApplyMatrix( const orientation_t *orient, patchMesh_t *p, char snap )
{
    patchSave_width  = p->width;
    patchSave_height = p->height;
    patchSave_type   = p->type;
    memcpy( patchSave_ctrl, p->ctrl, 0x5000u );

    for ( int col = 0; col < p->width; ++col )
        for ( int row = 0; row < p->height; ++row )
        {
            if ( g_qeglobals.d_select_mode == sel_curvepoint || g_bPatchBendMode )
            {
                bool selected = false;
                for ( int m = 0; m < g_qeglobals.d_num_move_points; ++m )
                    if ( &p->ctrl[col][row] == g_qeglobals.d_move_points[m] ) { selected = true; break; }
                if ( !selected )
                    continue;
            }
            float *xyz = p->ctrl[col][row].xyz;
            float rel[3];
            rel[0] = xyz[0] - orient->origin[0];
            rel[1] = xyz[1] - orient->origin[1];
            rel[2] = xyz[2] - orient->origin[2];
            OrientationPosToWorldPos( xyz, rel, orient );
            if ( snap )
                for ( int i = 0; i < 3; ++i )
                    xyz[i] = (float)floor( 0.5 + xyz[i] + xyz[i] ) * 0.5f;
        }

    Patch_Rebuild( p, 1 );
}

// ════════════════════════════════════════════════════════════════════════════
//  PATCH CONTROL-POINT INTERACTIVE EDIT (bend/drag) - the patch sibling of the shipped
//  VERTEX/EDGE editing (select.cpp SelectVertexByRay/Select_Edge + MoveSelection).  Flow:
//    1. enter curve-point mode (Selection->Drag Vertices on a pure-patch selection ->
//       OnSelectionDragVertices -> Patch_EditPatch 0x441fd0) - fills g_qeglobals.d_points[]
//       with every selected patch's control-point world positions + sets sel_curvepoint.
//    2. click a control point (XY_MouseDown -> Drag_Begin -> Drag_Setup ->
//       SelectCurvePointByRay -> Patch_ClickControlPoint 0x43cc00) - picks the nearest
//       d_points handle and pushes its drawVert_t* onto g_qeglobals.d_move_points[].
//    3. drag (Drag_MouseMoved -> MoveSelection sel_curvepoint branch ->
//       Patch_UpdateSelected 0x43ed10 / _0 0x43d800) - adds the snapped world delta to each
//       selected control point, rebuilds d_points, re-tessellates via Patch_Rebuild.
//  KISAK: the texture-lock texCoord-FOLLOW recompute (the PMESH_13/PMESH_12 chain that
//  re-projects per-control S/T as the surface bends when m_bTextureLock is OFF) is not
//  ported - behaviour equals texture-lock ON.  Zero round-trip impact: the .map serializes
//  control points, and texCoords are recomputed on parse anyway.
// ════════════════════════════════════════════════════════════════════════════

#include "prefs.h"     // g_PrefsDlg (m_bTextureLock / g_bPatchWeld / patch_drill_down)
#include "mainfrm.h"   // CMainFrame / CXYWnd (g_pParentWnd->m_pActiveXY->m_nViewType)

// patch_t (the patch INSTANCE node; .def @0, .selected @6) is the shared struct in qe3.h.

// Forward decls (used before definition within this block).
void sub_43ECB0();   // Patch_FinishCurveDrag — drag-END hook (keeps the IDB sub_ name
                     // already extern'd across drag/select/mainfrm.cpp)

// drag/mode globals (defined in drag.cpp / mainfrm.cpp); g_bXYViewIsLastPatchClick
// (IDB byte_25D5A6A) was a xywnd.cpp static — promoted to a shared global here (its
// primary reader is Patch_ClickControlPoint) so the click path can see the
// "second click in the same view" flag XY_MouseDown sets before Drag_Begin.
char         g_bXYViewIsLastPatchClick = 0;     // byte_25D5A6A (def here; extern in xywnd.cpp)
extern int   g_nPatchClickedView;               // 0x73B108 (engine_stubs.cpp)
extern CMainFrame *g_pParentWnd;                // 0x25D5A70 — only m_pActiveXY->m_nViewType read
extern void  CMainFrame_UpdatePatchToolbarButtons();        // select.cpp (no-op, no toolbar)
extern void  Undo_End();                                    // undo.cpp 0x45EA20

// ── Patch_FindMovePoint (0x43c2c0) ────────────────────────────────────────────
// Index of `pt` (a vec3 — a control point's xyz, OR a d_points[] slot reinterpreted)
// in g_qeglobals.d_move_points[] by exact xyz match (epsilon 9.5e-07/axis, the binary's
// 0x358637C0).  brush.cpp has a file-static twin (Patch_FindSelectedMovePoint) for the
// marker overlay; this is the select-path copy.
static int Patch_FindMovePoint( const float *pt )
{
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        const float *mp = g_qeglobals.d_move_points[i]->xyz;
        int k = 0;
        for ( ; k < 3; ++k )
        {
            float d = pt[k] - mp[k];
            if ( d * d > 1.00000011e-06f )
                break;
        }
        if ( k == 3 )
            return i;
    }
    return -1;
}

// ── Patch_FindMovePointByPtr ──────────────────────────────────────────────────
// Index of the exact control-point POINTER `cp` in d_move_points[], else -1.  The
// binary's move-point dedup/membership checks (Patch_ColumnAllSelected, the
// Patch_SelectCtrlPoint weld+drill-down sweeps, the Patch_ClickControlPoint
// already-selected test) all compare the drawVert_t POINTER, NOT xyz — coincident seam
// points share xyz but are DISTINCT pointers, so the weld sweep must queue each one
// (an xyz match via Patch_FindMovePoint would skip every coincident point after the first
// and defeat the weld).  Patch_FindMovePoint (xyz) remains correct only for
// Patch_RemoveMovePoint (which the binary likewise drives by xyz at 0x43c2c0).
static int Patch_FindMovePointByPtr( const drawVert_t *cp )
{
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
        if ( g_qeglobals.d_move_points[i] == cp )
            return i;
    return -1;
}

// ── Patch_LocateCtrlPoint (sub_43CE60 0x43ce60) ───────────────────────────────
// Given a control-point pointer (a drawVert_t* inside some selected patch's ctrl grid),
// find which selected patch owns it and decode its (row,col).  Returns 1 + fills
// out* on success, 0 if the pointer is not inside any selected patch's 16×16 ctrl grid.
// (The binary returns row in *a3, col in *a4 — note row = idx%16, col = idx/16, where
// idx = (ptr - &ctrl[0][0]) / 80.)
static int Patch_LocateCtrlPoint( const void *cp, patchMesh_t **outDef, int *outRow, int *outCol )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *def = b->patch->def;
        const char *base = (const char *)&def->ctrl[0][0];
        const char *last = (const char *)&def->ctrl[15][15];
        if ( (const char *)cp >= base && (const char *)cp <= last )
        {
            int idx = (int)( (const char *)cp - (const char *)def - 56 ) / 80;
            *outRow = idx % 16;
            *outCol = idx / 16;
            *outDef = def;
            return 1;
        }
    }
    return 0;
}

// ── Patch_AppendMovePoint (sub_43C3E0 0x43c3e0) ───────────────────────────────
// Append a control point (drawVert_t*) to g_qeglobals.d_move_points[].  If a move-point
// belonging to the SAME owning brush is already queued AND more than one brush is
// selected, insert at the FRONT instead (so one patch's points group at the head — the
// drill-down weld ordering).  `def->pSymbiot` holds the symbiont brush_t* (== a
// selbrush_t.def — confirmed via AddBrushForPatch: `p->pSymbiot = Brush_Alloc(...)`).
static void Patch_AppendMovePoint( patchMesh_t *def, drawVert_t *cp )
{
    if ( g_qeglobals.d_num_move_points
         && (brush_t *)def->pSymbiot == selected_brushes.next->def
         && selected_brushes.next != &selected_brushes )
    {
        memmove( &g_qeglobals.d_move_points[1], &g_qeglobals.d_move_points[0],
                 sizeof( g_qeglobals.d_move_points[0] ) * g_qeglobals.d_num_move_points );
        ++g_qeglobals.d_num_move_points;
        g_qeglobals.d_move_points[0] = cp;
    }
    else
    {
        g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = cp;
    }
}

// ── Patch_RemoveMovePoint (sub_43C320 0x43c320) ───────────────────────────────
// Remove the move-point whose xyz matches `pt` (a d_points slot) from d_move_points[]
// (repeatedly — there can be coincident duplicates from a weld).  The de-select-on-
// reclick path.
static void Patch_RemoveMovePoint( const float *pt )
{
    int idx = Patch_FindMovePoint( pt );
    while ( idx >= 0 )
    {
        for ( ; idx < g_qeglobals.d_num_move_points - 1; ++idx )
            g_qeglobals.d_move_points[idx] = g_qeglobals.d_move_points[idx + 1];
        --g_qeglobals.d_num_move_points;
        idx = Patch_FindMovePoint( pt );
    }
}

// ── Patch_SelectCtrlPoint (AddPoint 0x43c440) ─────────────────────────────────
// Queue control point `cp` (a drawVert_t* in patch `def`'s ctrl grid) as a drag
// move-point (Patch_AppendMovePoint), then — when the weld/drill-down prefs are on
// (g_bPatchWeld / patch_drill_down, both default 1) and `processWeld` is set — sweep
// every selected patch for COINCIDENT control points and queue them too (weld: same
// xyz within 1e-6; drill-down: same projected screen position in the clicked view)
// so a seam moves as one.  For a plain (unseamed) patch nothing extra is queued.
static void Patch_SelectCtrlPoint( patchMesh_t *def, drawVert_t *cp, char processWeld )
{
    // The clicked view's two on-screen axes (the depth axis is excluded for drill-down).
    int nViewType = ( g_pParentWnd && g_pParentWnd->m_pActiveXY )
                        ? g_pParentWnd->m_pActiveXY->m_nViewType : 0;
    int axisA = ( nViewType == 0 );               // IDB v24 = (m_nViewType==0) — first screen axis index
    int axisB = ( nViewType != 2 ) + 1;           // IDB v29 = (m_nViewType!=2)+1 — second screen axis index

    Patch_AppendMovePoint( def, cp );

    if ( !processWeld )
        return;
    if ( !g_PrefsDlg->g_bPatchWeld && !g_PrefsDlg->patch_drill_down )
        return;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        patchMesh_t *p = b->patch->def;
        for ( int col = 0; col < p->width; ++col )
        {
            for ( int row = 0; row < p->height; ++row )
            {
                drawVert_t *other = &p->ctrl[col][row];

                if ( g_PrefsDlg->g_bPatchWeld )
                {
                    // weld: identical xyz to the clicked point → queue it (de-dup'd).
                    float d0 = other->xyz[0] - cp->xyz[0];
                    float d1 = other->xyz[1] - cp->xyz[1];
                    float d2 = other->xyz[2] - cp->xyz[2];
                    if ( d0 * d0 <= 1.00000011e-06f && d1 * d1 <= 1.00000011e-06f
                         && d2 * d2 <= 1.00000011e-06f )
                    {
                        // dedup by POINTER (the binary): cp itself is already queued, every
                        // distinct coincident seam point gets appended so the weld moves as one.
                        if ( Patch_FindMovePointByPtr( other ) < 0 )
                            Patch_AppendMovePoint( p, other );
                        continue;       // already handled this point
                    }
                }

                if ( g_PrefsDlg->patch_drill_down && g_nPatchClickedView != 1 )
                {
                    // drill-down: same on-screen position (ignoring depth) → queue it.
                    float da = (float)fabs( cp->xyz[axisA] - other->xyz[axisA] );
                    if ( da > 0.001f )
                        continue;
                    float db = (float)fabs( cp->xyz[axisB] - other->xyz[axisB] );
                    if ( db > 0.001f )
                        continue;
                    if ( Patch_FindMovePointByPtr( other ) < 0 )      // dedup by POINTER (binary)
                        Patch_AppendMovePoint( p, other );
                }
            }
        }
    }
}

// ── Patch_ColumnAllSelected (sub_43C380 0x43c380) ─────────────────────────────
// 1 iff every (non-marked, turned_edge bit1 clear) control point in column `col` of
// patch `def` is already queued in d_move_points[].  Drives the click-toggle between
// column-select and row-select.
static char Patch_ColumnAllSelected( patchMesh_t *def, int col )
{
    if ( def->height <= 0 )
        return 1;
    for ( int row = 0; row < def->height; ++row )
    {
        drawVert_t *cp = &def->ctrl[col][row];
        if ( ( cp->turned_edge & 2 ) == 0 )
        {
            if ( Patch_FindMovePointByPtr( cp ) < 0 )      // binary: pointer equality, not xyz
                return 0;
        }
    }
    return 1;
}

// ── SelectColumn (0x43c760) / SelectRow (0x43c710) ────────────────────────────
// Queue every non-marked control point of a whole grid column / row (clearing the
// move-point list first unless `bMulti`).  The shift-modifier multi-select.
static void Patch_SelectColumn( patchMesh_t *def, int col, char bMulti )
{
    if ( !bMulti )
        g_qeglobals.d_num_move_points = 0;
    for ( int row = 0; row < def->height; ++row )
    {
        drawVert_t *cp = &def->ctrl[col][row];
        if ( ( cp->turned_edge & 2 ) == 0 )
            Patch_SelectCtrlPoint( def, cp, 0 );
    }
}

static void Patch_SelectRow( patchMesh_t *def, int row, char bMulti )
{
    if ( !bMulti )
        g_qeglobals.d_num_move_points = 0;
    for ( int col = 0; col < def->width; ++col )
    {
        drawVert_t *cp = &def->ctrl[col][row];
        if ( ( cp->turned_edge & 2 ) == 0 )
            Patch_SelectCtrlPoint( def, cp, 0 );
    }
}

// ── Patch_ShiftCtrlSelectByDelta (sub_43C820 0x43C820) ────────────────────────────
// Shift the selected control points of one patch DEF by grid-neighbour delta (dCol,dRow):
// for every cell (col,row) whose NEIGHBOUR (col+dCol, row+dRow) is currently a picked
// move-point (and this cell is not turned-edge-masked, turned_edge & 2), select THIS cell
// (and, unless bAdd, un-pick the neighbour).  The three iteration orders below match the
// binary's three passes chosen so a shifted point is never re-processed within the sweep:
//   dCol<0            : col high→low, row low→high
//   dCol>=0 & dRow>=0 : col low→high, row low→high
//   dCol>=0 & dRow<0  : col low→high, row high→low
// Bounds-checks the neighbour against [0,width)×[0,height).  NOTE `ctrl[i][j].turned_edge`
// is the `[eax+esi+84h]&2` byte test decoded to the drawVert flag at ctrl offset 0x4C.
static void Patch_ShiftCtrlSelectByDelta( patchMesh_t *def, int dCol, int dRow, char bAdd )
{
    const int w = def->width;
    const int h = def->height;

    auto tryCell = [&]( int col, int row )
    {
        int ncol = col + dCol;
        int nrow = row + dRow;
        if ( ncol < 0 || ncol >= w || nrow < 0 || nrow >= h )
            return;
        // Is the neighbour currently a picked move-point?
        drawVert_t *neighbour = &def->ctrl[ncol][nrow];
        int m = 0;
        for ( ; m < g_qeglobals.d_num_move_points; ++m )
            if ( g_qeglobals.d_move_points[m] == neighbour )
                break;
        if ( m >= g_qeglobals.d_num_move_points )
            return;
        drawVert_t *cell = &def->ctrl[col][row];
        if ( ( cell->turned_edge & 2 ) != 0 )
            return;
        if ( !bAdd )
            Patch_RemoveMovePoint( neighbour->xyz );
        Patch_SelectCtrlPoint( def, cell, 0 );
    };

    if ( dCol < 0 )
    {
        for ( int col = w - 1; col >= 0; --col )
            for ( int row = 0; row < h; ++row )
                tryCell( col, row );
    }
    else if ( dRow >= 0 )
    {
        for ( int col = 0; col < w; ++col )
            for ( int row = 0; row < h; ++row )
                tryCell( col, row );
    }
    else
    {
        for ( int col = 0; col < w; ++col )
            for ( int row = h - 1; row >= 0; --row )
                tryCell( col, row );
    }
    g_nUpdateBits = -1;
}

// ── PMESH_10 (0x43CB80) — move the patch-vertex selection by a grid delta ─────────────
// Called by the camera fly keys (Left/Right/Forward/Back).  Only acts in vertex/curve-
// point select mode: for every selected patch, shift its picked control points by
// (dCol,dRow) and return 1 (handled) — so the caller leaves the camera alone.  Returns 0
// (not handled) otherwise, letting the camera move.
char PMESH_10( char bAdd, int dCol, int dRow )
{
    if ( g_qeglobals.d_select_mode != sel_vertex && g_qeglobals.d_select_mode != sel_curvepoint )
        return 0;

    char handled = 0;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( b->patch )
        {
            iassert( b->patch->def == b->def->patch );   // PMESH.CPP:2230
            Patch_ShiftCtrlSelectByDelta( b->patch->def, dCol, dRow, bAdd );
            handled = 1;
        }
    }
    return handled;
}

// ── Patch_ClickControlPoint (PMESH_11 0x43cc00) ───────────────────────────────
// `cp` = the clicked d_points[] slot (a vec3*).  Find the control point in the
// selected patches whose xyz matches it, then apply the modifier action.  The two
// flags follow the binary's PMESH_11(a2,a3) call from SelectCurvePointByRay:
//   bMultiAppend (a2 = buttons & MK_CONTROL/8) → APPEND to the selection (don't clear).
//   bColRowSelect(a3 = buttons & MK_SHIFT/4)   → select a whole COLUMN, or a ROW once
//                                                the column is fully selected.
//   plain (neither)  → first click in a fresh view records the "same-view" flag and
//                      returns; a repeat click on an already-selected point de-selects
//                      it, else it becomes the sole selection.
// bLockPatchVerts / bUnlockPatchVerts (off by default) force the control point's
// "marked" (turned_edge bit1) flag on/off instead of selecting.
static void Patch_ClickControlPoint( const float *cp, char bMultiAppend, char bColRowSelect )
{
    // First plain click in a fresh view just arms the "same-view" flag (so the NEXT
    // click in the same view can de-select).  byte_25D5A6A is set by XY_MouseDown.
    if ( !g_bXYViewIsLastPatchClick && !bMultiAppend && !bColRowSelect )
    {
        g_bXYViewIsLastPatchClick = 1;
        return;
    }

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        if ( b->patch->def != b->def->patch )
            Assert( PMESH_CPP, 2257, 0, "%s", "pb->patch->def == pb->def->patch" );

        patchMesh_t *def = b->def->patch;
        for ( int col = 0; col < def->width; ++col )
        {
            for ( int row = 0; row < def->height; ++row )
            {
                drawVert_t *here = &def->ctrl[col][row];

                // does this control point's xyz match the clicked d_points slot?
                int k = 0;
                for ( ; k < 3; ++k )
                {
                    float d = here->xyz[k] - cp[k];
                    if ( d * d > 1.00000011e-06f )
                        break;
                }
                if ( k != 3 )
                    continue;               // not this control point

                // ── matched the clicked control point ──────────────────────────
                if ( g_qeglobals.bLockPatchVerts )
                {
                    g_qeglobals.d_num_move_points = 0;
                    here->turned_edge |= 2;
                    continue;               // keep scanning (mark every coincident)
                }
                if ( g_qeglobals.bUnlockPatchVerts )
                {
                    g_qeglobals.d_num_move_points = 0;
                    here->turned_edge &= ~2;
                    continue;
                }

                int existing = Patch_FindMovePointByPtr( here );   // binary: pointer equality (v9-19 != d_move_points[v13])
                if ( existing < 0 )
                {
                    // not yet selected
                    if ( bColRowSelect )                       // a3 → column select
                    {
                        Patch_SelectColumn( def, col, bMultiAppend );
                        return;
                    }
                    if ( !bMultiAppend )                       // !a2 → replace selection
                        g_qeglobals.d_num_move_points = 0;
                    if ( ( here->turned_edge & 2 ) == 0 )
                        Patch_SelectCtrlPoint( def, here, 1 );
                    return;
                }
                // already selected
                if ( bColRowSelect )                           // a3 → column→row toggle
                {
                    if ( Patch_ColumnAllSelected( def, col ) )
                    {
                        Patch_SelectRow( def, row, bMultiAppend );
                        return;
                    }
                    Patch_SelectColumn( def, col, bMultiAppend );
                    return;
                }
                if ( g_bXYViewIsLastPatchClick )
                {
                    Patch_RemoveMovePoint( cp );    // re-click in same view → de-select
                    return;
                }
            }
        }
    }
}

// ── SelectCurvePointByRay (0x495150) — port lives in select.cpp (sibling of
//    SelectVertexByRay); it calls Patch_ClickControlPoint via this bridge.  The
//    args are the two PMESH_11 flags: bMultiAppend=(buttons&8), bColRowSelect=(buttons&4). ──
extern "C" void Patch_ClickControlPoint_C( const float *cp, char bMultiAppend, char bColRowSelect )
{
    Patch_ClickControlPoint( cp, bMultiAppend, bColRowSelect );
}

// ── Patch_EditPatch (0x441fd0) — ENTER curve-point edit mode ──────────────────
// Reset the point lists, fill g_qeglobals.d_points[] with every selected patch's
// control-point world positions (so SelectCurvePointByRay can ray-pick them), and
// switch d_select_mode to sel_curvepoint.  Wired from CMainFrame::OnSelectionDragVertices
// (Selection→Drag Vertices, ID 33007) when only patches are selected.
void Patch_EditPatch()
{
    g_qeglobals.d_numpoints              = 0;
    g_qeglobals.d_num_move_points        = 0;
    g_qeglobals.patch_verts_array01_count = 0;
    g_qeglobals.patch_verts_array02_count = 0;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        iassert( b->patch->def == b->def->patch );  // 4351 — #expr byte-matches embedded string

        patchMesh_t *def = b->def->patch;
        for ( int col = 0; col < def->width; ++col )
        {
            for ( int row = 0; row < def->height; ++row )
            {
                int idx = g_qeglobals.d_numpoints;
                const float *src = def->ctrl[col][row].xyz;
                g_qeglobals.d_points[idx][0] = src[0];
                g_qeglobals.d_points[idx][1] = src[1];
                g_qeglobals.d_points[idx][2] = src[2];
                if ( g_qeglobals.d_numpoints < 2047 )
                    ++g_qeglobals.d_numpoints;
            }
        }
    }

    select_t prev = g_qeglobals.d_select_mode;
    g_qeglobals.d_select_mode = sel_curvepoint;
    if ( prev == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prev == sel_addpoint )
        sub_43ECB0();
}

// ── Patch_UpdateSelected_0 (0x43d800) — APPLY the drag delta + re-tessellate ──
// Add the world delta `move` to every selected control point, rebuild d_points[] from
// the new control positions, and re-tessellate each selected patch (Patch_Rebuild →
// Patch_GenericMesh2) so the curve re-bends live.  The texture-lock texCoord-FOLLOW
// recompute (the `!m_bTextureLock` block: PMESH_13/PMESH_12 re-project per-control S/T)
// is DEFERRED (see the scope note above) — geometry only.
static void Patch_UpdateSelected_0( const float *move )
{
    // [DEFERRED] texture-lock-off texCoord follow (PMESH_13 chain) — material epic.

    // (1) translate every queued control point by the snapped world delta.
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        drawVert_t *mp = g_qeglobals.d_move_points[i];
        mp->xyz[0] += move[0];
        mp->xyz[1] += move[1];
        mp->xyz[2] += move[2];
    }

    // (2) for each selected patch: rebuild d_points[] from ctrl, then re-tessellate.
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( !b->patch )
            continue;
        if ( b->patch->def != b->def->patch )
            Assert( PMESH_CPP, 2613, 0, "%s", "pb->patch->def == pb->def->patch" );

        patchMesh_t *def = b->def->patch;
        g_qeglobals.d_numpoints = 0;
        for ( int col = 0; col < def->width; ++col )
        {
            for ( int row = 0; row < def->height; ++row )
            {
                int idx = g_qeglobals.d_numpoints;
                const float *src = def->ctrl[col][row].xyz;
                g_qeglobals.d_points[idx][0] = src[0];
                g_qeglobals.d_points[idx][1] = src[1];
                g_qeglobals.d_points[idx][2] = src[2];
                if ( g_qeglobals.d_numpoints < 2047 )
                    ++g_qeglobals.d_numpoints;
            }
        }
        Patch_Rebuild( def, 1 );
    }
}

// ── Patch_UpdateSelected (0x43ed10) — the dispatcher MoveSelection calls ───────
// In paint mode 1 ("Drag Up/Down") with the AdvPatchEditDlg active (sub_401D50() &&
// sub_401DB0()==1) the drag does a SOFT-SELECTION height push (sub_43DD00 with the Z
// delta); otherwise it is a standard control-point translate (Patch_UpdateSelected_0).
// `move` is the snapped world delta (a float[3]); the binary passes it in ESI.
extern int sub_401D50();              // patchdialog.cpp — "is terrain-paint mode active?"
extern int sub_401DB0();              // patchdialog.cpp — checked mode radio index
void       sub_43DD00( float amount );   // forward (soft-sel height drag dispatch, below)
void Patch_UpdateSelected( int move )
{
    if ( sub_401D50() && sub_401DB0() == 1 )
        sub_43DD00( ( (const float *)(intptr_t)move )[2] );   // soft-sel: push Z by the drag delta
    else
        Patch_UpdateSelected_0( (const float *)(intptr_t)move );
}

// ══════════════════════════════════════════════════════════════════════════════
//  PMESH_56 / PMESH_57 (0x44cfb0 / 0x44d0c0) — the SurfaceInspector multi-layer
//  patch-texturing GET / SET pair (the patch analogue of SetTexMods/SurfaceDlg_Wnd02).
//  No longer dead code: the #2 material-epic unit ports + wires their callers
//  (SetTexMods→PostDoSurface02→PMESH_56, SurfaceDlg_Wnd02→PostDoSurface02_Patch→PMESH_57,
//  Patch_RedispersePreDrag→PMESH_56).  Every per-control-point offset below was traced
//  from the disasm (NOT hex-rays, which manges the drawVert pointer walk into [esi±N]).
//
//  Layout facts (IDB-verified, qe3.h): patchMesh_t.texture/lightmap/smoothing are three
//  contiguous 8-byte patchMesh_material {lyrMtl,radMtl} at +0x18/+0x20/+0x28 → indexable
//  as (&p->texture)[layer].  The "live/working" material slot is pad_0x0030(+0x30) +
//  mat_unk(+0x34) = one more {lyrMtl,radMtl} pair.  bDirty(+0x5043), xx21(+0x5044) is a
//  byte shadow.  Each drawVert_t ctrl point: texCoord(+0xC) live, savedTexCoord(+0x34)
//  snapshot — both pmesh_texcoord{st,lightmap,smoothing} = 3 layers × vec2.
// ══════════════════════════════════════════════════════════════════════════════

// PMESH_56 (0x44cfb0) — patch "GET": pull the CURRENT layer's material pointers into the
// live working slot, then SNAPSHOT every control point's live texCoord (all 3 layers) into
// its savedTexCoord.  Mirrors SetTexMods' per-face mtldef[current]→mtldef[3] save.
//   * live{lyrMtl,radMtl}(+0x30/+0x34) ← (&p->texture)[current_edit_layer]   (disasm 44cfbf)
//   * xx21(+0x5044) ← bDirty(+0x5043)                                         (disasm 44cfca)
//   * per ctrl point: savedTexCoord[0..2] ← texCoord[0..2]  (24 bytes)        (disasm 44d006)
static void PMESH_56( patchMesh_t *p )
{
    const int layer = g_qeglobals.current_edit_layer;

    // Working material slot ← current layer's material pair.  (&p->texture)[layer] is the
    // layer's {lyrMtl,radMtl}; the live pair lives at +0x30/+0x34 (pad_0x0030 + mat_unk).
    patchMesh_material *live = (patchMesh_material *)( (char *)p + 0x30 );
    *live = ( &p->texture )[layer];
    *( (char *)p + 0x5044 ) = *( (char *)p + 0x5043 );   // xx21 ← bDirty (byte shadow)

    for ( int row = 0; row < p->height; ++row )          // [ebx+4] = height (outer)
    {
        for ( int col = 0; col < p->width; ++col )       // [ebx]   = width
        {
            drawVert_t *v = &p->ctrl[col][row];
            // Snapshot all three layer texCoords (st/lightmap/smoothing = 3 × vec2).
            for ( int l = 0; l < 3; ++l )
            {
                float *dst = &( (float *)&v->savedTexCoord )[2 * l];
                dst[0] = ( (float *)&v->texCoord )[2 * l + 0];
                dst[1] = ( (float *)&v->texCoord )[2 * l + 1];
                if ( ( *(unsigned int *)&dst[0] & 0x7F800000 ) == 0x7F800000 )
                    Assert( PMESH_CPP, 9059, 0, "%s", "!IS_NAN(p->ctrl[col][row].savedTexCoord[textureLayer][0])" );
                if ( ( *(unsigned int *)&dst[1] & 0x7F800000 ) == 0x7F800000 )
                    Assert( PMESH_CPP, 9060, 0, "%s", "!IS_NAN(p->ctrl[col][row].savedTexCoord[textureLayer][1])" );
            }
        }
    }
}

// PMESH_57 (0x44d0c0) — patch "SET": push the live working material pair BACK onto the
// current layer, restore each control point's CURRENT-layer texCoord from its snapshot,
// then Patch_Rebuild.  Mirrors SurfaceDlg_Wnd02's per-face mtldef[3]→mtldef[current] apply.
//   * (&p->texture)[current_edit_layer] ← live{lyrMtl,radMtl}(+0x30/+0x34)    (disasm 44d0cf)
//   * bDirty(+0x5043) ← xx21(+0x5044)                                         (disasm 44d0e2)
//   * per ctrl point: texCoord[current] ← savedTexCoord[current] (current layer only) (44d18e)
//   * Patch_Rebuild(p, 1)                                                     (disasm 44d1d4)
static void PMESH_57( patchMesh_t *p )
{
    const int layer = g_qeglobals.current_edit_layer;

    const patchMesh_material *live = (const patchMesh_material *)( (char *)p + 0x30 );
    ( &p->texture )[layer] = *live;
    *( (char *)p + 0x5043 ) = *( (char *)p + 0x5044 );   // bDirty ← xx21 (byte shadow)

    for ( int row = 0; row < p->height; ++row )          // [esi+4] = height
    {
        for ( int col = 0; col < p->width; ++col )       // [esi]   = width
        {
            drawVert_t *v = &p->ctrl[col][row];
            float sx = ( (float *)&v->savedTexCoord )[2 * layer + 0];
            float sy = ( (float *)&v->savedTexCoord )[2 * layer + 1];
            if ( ( *(unsigned int *)&sx & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 9078, 0, "%s", "!IS_NAN(p->ctrl[col][row].savedTexCoord[g_qeglobals.activeTexCoord][0])" );
            if ( ( *(unsigned int *)&sy & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 9079, 0, "%s", "!IS_NAN(p->ctrl[col][row].savedTexCoord[g_qeglobals.activeTexCoord][1])" );
            ( (float *)&v->texCoord )[2 * layer + 0] = sx;
            ( (float *)&v->texCoord )[2 * layer + 1] = sy;
        }
    }
    Patch_Rebuild( p, 1 );
}

// PostDoSurface02 / PostDoSurface02_Patch reach PMESH_56/57 through a brush_t.patch
// pointer; expose thin externs to surfacedlg.cpp (where the SurfaceInspector lives).
void PMESH_56_extern( patchMesh_t *p ) { PMESH_56( p ); }
void PMESH_57_extern( patchMesh_t *p ) { PMESH_57( p ); }

// ── Patch_RedispersePreDrag (sub_43D7A0 0x43d7a0) — drag-START hook (NOW LIVE) ──
// Snapshot every touched patch's live texCoords into savedTexCoord before the drag begins
// (PMESH_56), so the curve-drag edits work off a clean baseline.  Dedupes consecutive
// ctrl points belonging to the same patch (the binary's `if (def != last)` guard).
void Patch_RedispersePreDrag()
{
    patchMesh_t *last = nullptr;
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        patchMesh_t *def; int row, col;
        if ( Patch_LocateCtrlPoint( g_qeglobals.d_move_points[i], &def, &row, &col ) )
        {
            if ( def != last )
            {
                PMESH_56( def );
                last = def;
            }
        }
    }
}

// ── PMESH_18 (0x43ec10) — paint/drag undo-id stamp ────────────────────────────
//  After a paint or soft-sel drag, for every touched patch brush in the list (paint-dirty
//  flag xx22b set): clear the flag and stamp the current undo record's id onto the brush def
//  (ownerPrev) + its entity (epairEdits) — the same idiom as Undo_LinkBrush, so the stroke
//  participates correctly in undo/redo grouping.
extern undo_s *g_lastundo;            // undo.cpp (0x23F162C) — also externed below for PMESH_16
static void PMESH_18( selbrush_t *list )
{
    for ( selbrush_t *i = list->next; i != list; i = i->next )
    {
        patch_t *patch = i->patch;
        if ( !patch )
            continue;
        if ( patch->def != i->def->patch )
            Assert( PMESH_CPP, 3137, 0, "%s", "b->patch->def == b->def->patch" );
        if ( !patch->def->xx22b )
            continue;
        patch->def->xx22b = 0;
        if ( g_lastundo && !g_lastundo->done )
        {
            i->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *owner = (entity_s *)(intptr_t)i->def->owner;
            if ( *(int *)&owner->eclass->fixedsize )
                owner->epairEdits = g_lastundo->id;
        }
    }
}

// ── sub_43ECB0 (0x43ecb0) — Patch_FinishCurveDrag, the drag-END hook ──────────
// After a "Grab Value" (mode 5) drag, switch the tool to Flatten (sub_401E10(2)); stamp the
// painted patches' undo ids (PMESH_18, the active/unselected list too when checkbox 1432 is
// set); then close the undo record Drag_Setup opened.
extern void AdvPatchEdit_SetMode( int mode );   // patchdialog.cpp — sub_401E10 (select mode radio)
extern int  CurvEditDlg_OnSomeSetting();        // patchdialog.cpp — checkbox 1432
void sub_43ECB0()
{
    if ( sub_401DB0() == 5 )
        AdvPatchEdit_SetMode( 2 );               // Grab Value -> Flatten
    if ( CurvEditDlg_OnSomeSetting() )
        PMESH_18( &active_brushes );
    PMESH_18( &selected_brushes );
    Undo_End();
}

// ── PMESH_37 (Patch_FlipTexture, 0x445e30) — flip a patch's S or T texcoords ───
//  The patch branch of Brush_FlipTexture (Select_FlipTexture, select.cpp).  axis=0
//  flips S, axis=1 flips T.  For each control point — every one outside curve-point
//  mode, only the SELECTED move-points inside it — replaces the current edit layer's
//  st[axis] with 1-st[axis].  Then (layer 1 only) clears bDirty and rebuilds the
//  tessellated curveDef.  Touches editor texCoord state only; never serialized.
curvePatchDef_t *PMESH_37( patchMesh_t *a1, int axis )
{
    const int layer = g_qeglobals.current_edit_layer;

    for ( int col = 0; col < a1->width; ++col )
    {
        for ( int row = 0; row < a1->height; ++row )
        {
            drawVert_t *cp = &a1->ctrl[col][row];

            // In curve-point mode only flip control points that are queued as move-points.
            if ( g_qeglobals.d_select_mode == sel_curvepoint )
            {
                int k = 0;
                for ( ; k < g_qeglobals.d_num_move_points; ++k )
                    if ( (void *)cp == (void *)g_qeglobals.d_move_points[k] )
                        break;
                if ( k >= g_qeglobals.d_num_move_points )
                    continue;
            }

            float *st = (float *)&cp->texCoord;
            float  nv = 1.0f - st[2 * layer + axis];
            if ( ( *(unsigned int *)&nv & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 5736, 0, "%s", "!IS_NAN(newTexCoord)" );
            st[2 * layer + axis] = nv;
        }
    }

    if ( layer == 1 )
        a1->bDirty = 0;
    if ( a1->curveDef )
        free( a1->curveDef );
    curvePatchDef_t *result = Patch_GenericMesh2( a1, layer, 0, 0 );
    ++a1->version;
    a1->curveDef = result;
    return result;
}

// ════════════════════════════════════════════════════════════════════════════
//  TERRAIN-EDGE-TURN PICKER GROUP - the "cycle edge direction" (terrain quad-diagonal
//  turn) tool, reached from drag.cpp's sel_cycle_edge_direction_quad mode:
//  XY_MouseDown -> Patch_TurnEdge -> PMESH_51 (the ray-vs-curve-edge picker), which needs
//  the tessellated curveDef.  For a PATCH_TERRAIN patch the curveDef render mesh IS the
//  control grid (1:1 copy in Patch_GenericMesh2's terrain branch), so verts[col+row*width]
//  aligns with ctrl[col][row].  Nothing here is serialized.
// ════════════════════════════════════════════════════════════════════════════

extern int     Sys_Printf( const char *fmt, ... );      // 0x499E90
extern undo_s *g_lastundo;                               // undo.cpp 0x23F162C
extern void    Undo_AddBrush( entity_brush_s *pBrushInst );   // 0x45E680
extern void    Undo_AddEntity( int a1 );                      // 0x45E8B0

// ── PlaneFromPoints_Real (0x4a9950) ───────────────────────────────────────────
//  out[0..2] = normalized normal of triangle (A,B,C) about pivot B = (C-B)×(A-B);
//  out[3] = dot(normal, B).  Returns 0 if the triangle is degenerate (both the
//  primary and the alternate-pivot cross collapse), 1 otherwise.  Faithful to the
//  binary including the area-vs-length² degeneracy guard (epsilon 1.00000011e-06).
// non-static: also called by the model ray-pick (sub_48CE60) in select.cpp.
int PlaneFromPoints_Real( float *out, const float *A, const float *B, const float *C )
{
    float e1[3], e2[3];
    e1[0] = A[0] - B[0]; e1[1] = A[1] - B[1]; e1[2] = A[2] - B[2];   // v24 = A-B
    e2[0] = C[0] - B[0]; e2[1] = C[1] - B[1]; e2[2] = C[2] - B[2];   // v21 = C-B
    Vec3Cross( e2, e1, out );                                       // out = (C-B)×(A-B)

    float lenSq = out[0]*out[0] + out[1]*out[1] + out[2]*out[2];    // v12
    if ( lenSq < 2.0f )
    {
        if ( lenSq == 0.0f )
            return 0;
        float l1 = e1[0]*e1[0] + e1[1]*e1[1] + e1[2]*e1[2];
        float l2 = e2[0]*e2[0] + e2[1]*e2[1] + e2[2]*e2[2];
        if ( (float)( l1 * l2 * 0.00000100000011116208 ) >= lenSq )
        {
            // alternate pivot A: normal = (C-A)×(B-A)
            float f1[3], f2[3];
            f1[0] = C[0] - A[0]; f1[1] = C[1] - A[1]; f1[2] = C[2] - A[2];   // v24
            f2[0] = B[0] - A[0]; f2[1] = B[1] - A[1]; f2[2] = B[2] - A[2];   // v21
            Vec3Cross( f2, f1, out );
            float m1 = f1[0]*f1[0] + f1[1]*f1[1] + f1[2]*f1[2];
            float m2 = f2[0]*f2[0] + f2[1]*f2[1] + f2[2]*f2[2];
            if ( (float)( m1 * m2 * 0.00000100000011116208 ) >= lenSq )
                return 0;
        }
    }
    float len = (float)sqrt( lenSq );
    out[0] /= len; out[1] /= len; out[2] /= len;
    out[3] = out[2]*B[2] + out[0]*B[0] + out[1]*B[1];
    return 1;
}

// ── PMESH_RaySegPick (sub_44AB10, 0x44ab10) — ray-vs-triangle pick ────────────
//  Möller–Trumbore intersection of ray (origin `org`, direction `dir`) against the
//  triangle (base, vA, vB).  edge1 = vA-base, edge2 = vB-base.  On a hit returns 1
//  and writes the ray parameter to *outT; *outU/*outV (when non-null) receive the
//  barycentric coords.  Two-sided (sign tracks the winding).  Determinant epsilon
//  1e-4 verbatim from the binary.
// non-static: also called by the model ray-pick (sub_48CE60) in select.cpp.
char PMESH_RaySegPick( const float *vB, const float *vA, const float *dir,
                              const float *base, const float *org,
                              float *outT, float *outU, float *outV )
{
    float edge1[3], edge2[3], pvec[3], tvec[3], qvec[3];
    edge1[0] = vA[0] - base[0]; edge1[1] = vA[1] - base[1]; edge1[2] = vA[2] - base[2];   // v20
    edge2[0] = vB[0] - base[0]; edge2[1] = vB[1] - base[1]; edge2[2] = vB[2] - base[2];   // v13
    Vec3Cross( dir, edge2, pvec );                                                        // v14

    float det = edge1[0]*pvec[0] + edge1[1]*pvec[1] + edge1[2]*pvec[2];   // v26
    float sign;
    if ( det >= 0.0001 )       sign = 1.0f;
    else if ( det > -0.0001 )  return 0;
    else { sign = -1.0f; det = -det; }

    tvec[0] = org[0] - base[0]; tvec[1] = org[1] - base[1]; tvec[2] = org[2] - base[2];   // v17
    float u = ( pvec[0]*tvec[0] + pvec[1]*tvec[1] + pvec[2]*tvec[2] ) * sign;             // v24
    if ( u < 0.0f || u > det )
        return 0;

    Vec3Cross( tvec, edge1, qvec );                                                      // v14 (reused)
    float v = ( dir[0]*qvec[0] + dir[1]*qvec[1] + dir[2]*qvec[2] ) * sign;               // v23
    if ( v < 0.0f || det < u + v )
        return 0;

    float t = ( edge2[0]*qvec[0] + edge2[1]*qvec[1] + edge2[2]*qvec[2] ) * sign / det;   // v11
    *outT = t;
    if ( outU ) *outU = u / det;
    if ( outV ) *outV = v / det;
    return 1;
}

// barycentric blend of three curveVerts' RGBA bytes: base*w + A*u + B*v (w=1-u-v).
static void PMESH_BlendCornerColor( const curveVert_t *base, const curveVert_t *A,
                                    const curveVert_t *B, float u, float v, float *outRGBA )
{
    double w = 1.0 - u - v;
    const unsigned char *cb = (const unsigned char *)&base->vert_color;
    const unsigned char *ca = (const unsigned char *)&A->vert_color;
    const unsigned char *cc = (const unsigned char *)&B->vert_color;
    for ( int k = 0; k < 4; ++k )
        outRGBA[k] = (float)( (double)ca[k] * u + (double)cc[k] * v + (double)cb[k] * w );
}

// ── PMESH_51 (0x44acc0) — ray-vs-curve-edge picker ────────────────────────────
//  Walk the tessellated curveDef grid, split each quad into two triangles along the
//  cell diagonal (turned for PATCH_TERRAIN cells with turned_edge&1), and ray-test
//  each.  Returns the nearest hit (dist > 2.0): *outDist = parameter, *outCol/*outRow
//  = the cell, *outColor (4 bytes, when non-null) = the barycentric vertex colour,
//  *outPlane (vec4, when non-null) = the hit triangle's plane.
// non-static: also called by the per-brush ray trace (sub_48D240) in select.cpp.
char PMESH_51( const float *org, const float *dir, patch_t *pm,
                      float *outDist, int *outCol, int *outRow,
                      unsigned char *outColor, float *outPlane )
{
    iassert( pm );          // 8042 — #pm byte-matches the embedded "pm"
    iassert( pm->def );     // 8043 — #pm->def byte-matches the embedded "pm->def"

    curvePatchDef_t *cd = pm->def->curveDef;
    if ( !cd )
        return 0;

    const int   width  = cd->width;
    const int   isTerr = ( pm->def->type & PATCH_TERRAIN ) != 0;
    float       best   = 3.4028235e38f;   // v81
    char        found  = 0;               // v89
    int         bestCol = 0, bestRow = 0; // v75 / HIDWORD(v74)
    float       color[4] = { 0, 0, 0, 0 };// v70..v73

    for ( int row = 0; row < cd->height - 1; ++row )
    {
        for ( int col = 0; col < width - 1; ++col )
        {
            const int idx = col + row * width;
            curveVert_t *verts = cd->verts;
            curveVert_t *base, *vA, *vB2, *v17;   // v15 / r / v84 / v17

            if ( isTerr && ( pm->def->ctrl[col][row].turned_edge & 1 ) )
            {
                base = &verts[idx];              // v15 = TL
                vA   = &verts[idx + 1];          // r   = TR
                v17  = &verts[idx + width + 1];  // v17 = BR  (turned diagonal TL–BR)
                vB2  = &verts[idx + width];      // v84 = BL
            }
            else
            {
                vA   = &verts[idx];              // r   = TL
                base = &verts[idx + width];      // v15 = BL
                v17  = &verts[idx + 1];          // v17 = TR  (diagonal BL–TR)
                vB2  = &verts[idx + width + 1];  // v84 = BR
            }

            // Triangle #1: (base, vA, v17), tested both windings (swap A/B and u/v).
            float dist, bu, bv;
            if ( PMESH_RaySegPick( v17->xyz, vA->xyz, dir, base->xyz, org, &dist, &bu, &bv )
              || PMESH_RaySegPick( vA->xyz, v17->xyz, dir, base->xyz, org, &dist, &bv, &bu ) )
            {
                if ( dist > 2.0f )
                {
                    found = 1;
                    if ( best > dist )
                    {
                        if ( outPlane )
                        {
                            float pl[4];
                            PlaneFromPoints_Real( pl, base->xyz, vA->xyz, v17->xyz );
                            outPlane[0] = pl[0]; outPlane[1] = pl[1]; outPlane[2] = pl[2];
                        }
                        best = dist;
                        bestCol = col;
                        bestRow = row;
                        PMESH_BlendCornerColor( base, vA, v17, bu, bv, color );
                    }
                }
            }

            // Triangle #2: (base, v17, vB2), tested both windings.
            if ( PMESH_RaySegPick( vB2->xyz, v17->xyz, dir, base->xyz, org, &dist, &bu, &bv )
              || PMESH_RaySegPick( v17->xyz, vB2->xyz, dir, base->xyz, org, &dist, &bv, &bu ) )
            {
                if ( dist > 2.0f )
                {
                    found = 1;
                    if ( best > dist )
                    {
                        if ( outPlane )
                        {
                            float pl[4];
                            PlaneFromPoints_Real( pl, base->xyz, v17->xyz, vB2->xyz );
                            outPlane[0] = pl[0]; outPlane[1] = pl[1]; outPlane[2] = pl[2];
                        }
                        best = dist;
                        bestCol = col;
                        bestRow = row;
                        PMESH_BlendCornerColor( base, v17, vB2, bu, bv, color );
                    }
                }
            }
        }
    }

    if ( !found )
        return 0;

    *outDist = best;
    if ( outCol ) *outCol = bestCol;
    if ( outRow ) *outRow = bestRow;
    if ( outColor )
    {
        // inline-fistp rounds (the binary adds 2^-30 before the cvt); colours are
        // byte-ranged so this matches the rounded original.
        outColor[0] = (unsigned char)(int)( color[0] + 9.313225746154785e-10 );
        outColor[1] = (unsigned char)(int)( color[1] + 9.313225746154785e-10 );
        outColor[2] = (unsigned char)(int)( color[2] + 9.313225746154785e-10 );
        outColor[3] = (unsigned char)(int)( color[3] + 9.313225746154785e-10 );
    }
    return found;
}

// ── sub_43DD50 (0x43DD50) — terrain-paint cursor pick ─────────────────────────
//  Ray-pick the NEAREST terrain patch (across selected + active patches, honouring
//  FilterBrush + brushFlags&0x20) and report the hit's blended vertex colour (4 bytes
//  into `colorOut`, when non-null) and the hit world position (`origin_out`).  Reuses
//  PMESH_51's outColor branch (the barycentric colour at the hit).  Returns 1 on a hit.
//  The terrain-paint geometry core under sub_43E6F0 (the AdvPatchEditDlg paint drag,
//  still parked on its dialog); ported + gated standalone so the pick is verified now.
extern char FilterBrush( selbrush_t *b, int updateFilters );    // filters.cpp 0x46A1F0
extern selbrush_t active_brushes;                               // map.cpp   0x23F189C

char sub_43DD50( const float *dir, unsigned char *colorOut,
                 const float *cam_origin, float *origin_out )
{
    float best = 3.4028235e38f;             // i — nearest hit parameter
    unsigned char cell[4];                   // a7 — the picked vertex colour

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( b->patch && !FilterBrush( b, 0 ) && ( b->brushFlags & 0x20 ) == 0 )
        {
            float dist;
            if ( PMESH_51( cam_origin, dir, b->patch, &dist, nullptr, nullptr, cell, nullptr )
              && dist < best )
            {
                best = dist;
                if ( colorOut )
                { colorOut[0]=cell[0]; colorOut[1]=cell[1]; colorOut[2]=cell[2]; colorOut[3]=cell[3]; }
            }
        }
    }
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        if ( b->patch && !FilterBrush( b, 0 ) && ( b->brushFlags & 0x20 ) == 0 )
        {
            float dist;
            if ( PMESH_51( cam_origin, dir, b->patch, &dist, nullptr, nullptr, cell, nullptr )
              && dist < best )
            {
                best = dist;
                if ( colorOut )
                { colorOut[0]=cell[0]; colorOut[1]=cell[1]; colorOut[2]=cell[2]; colorOut[3]=cell[3]; }
            }
        }
    }

    if ( best == 3.4028235e38f )
        return 0;
    origin_out[0] = dir[0] * best + cam_origin[0];
    origin_out[1] = dir[1] * best + cam_origin[1];
    origin_out[2] = dir[2] * best + cam_origin[2];
    return 1;
}

// ── sub_43DB60 (0x43DB60) — terrain-paint brush falloff weight ────────────────
//  The per-control-point weight of a paint stroke: the 2D (XY) distance from the
//  paint centre to the control point selects `strength` inside innerR, 0 beyond
//  outerR, and a smoothstep blend  w = 1 - t*t*(3-2t)  (t = (d-innerR)/(outerR-innerR))
//  in the annulus.  Pure geometry — the dialog-independent leaf of the PMESH_16 apply
//  chain; gated directly (RunPaintFalloffTest).
float sub_43DB60( const float *center, const float *pt, float innerR, float outerR, float strength )
{
    const float dx = pt[0] - center[0];
    const float dy = pt[1] - center[1];
    const float d2 = dx * dx + dy * dy;
    if ( d2 <= innerR * innerR )
        return strength;
    if ( outerR * outerR <= d2 )
        return 0.0f;
    const float d = sqrtf( d2 );
    const float t = ( d - innerR ) / ( outerR - innerR );
    const float w = 1.0f - t * t * ( 3.0f - ( t + t ) );
    return w * strength;
}

// ── Terrain-paint per-channel mode callbacks (sub_43E550..) ───────────────────
//  PMESH_16 invokes one of these per affected control point per channel:
//    callback(ctrl, channel, currentValue, scaledStrength, falloffWeight) -> newValue.
//  `channel` is 0=height, 1=B, 2=G, 3=R, 4=alpha (the PMESH_16 a5(ctrl,N,..) order).
//  g_paintChannelVal[5] (IDB flt_231F534) holds the target value (set-toward modes) or
//  the running weighted sum (average modes); g_paintChannelWt[5] (IDB flt_231F520) holds
//  the average's weight accumulator.  Both are seeded by sub_43E6F0 (the paint drag,
//  dialog-gated).  These are the dialog-independent leaf math of the apply chain.
float g_paintChannelVal[5] = { 0, 0, 0, 0, 0 };   // 0x231F534 (flt_231F534[0..4])
float g_paintChannelWt[5]  = { 0, 0, 0, 0, 0 };   // 0x231F520 (flt_231F520[0..4])

// sub_43E550 (0x43E550) — RAISE/LOWER: add the (signed) scaled-strength*weight to the value.
float sub_43E550( float * /*ctrl*/, int /*channel*/, float cur, float strength, float weight )
{
    return strength * weight + cur;
}

// sub_43E570 (0x43E570) — SET-TOWARD-TARGET ("paint"): lerp the value toward the target
//  channel (g_paintChannelVal) by t = clamp(strength*weight, 0, 1).
float sub_43E570( float * /*ctrl*/, int channel, float cur, float strength, float weight )
{
    float t = strength * weight;
    if ( t < 0.0f ) t = 0.0f;
    else if ( t > 1.0f ) t = 1.0f;
    return ( 1.0f - t ) * cur + g_paintChannelVal[channel] * t;
}

// sub_43E5D0 (0x43E5D0) — AVERAGE GATHER: accumulate the weighted value + weight into the
//  per-channel accumulators (a later pass divides to get the mean); returns the value
//  unchanged (the gather pass does not modify the grid).
float sub_43E5D0( float * /*ctrl*/, int channel, float cur, float /*strength*/, float weight )
{
    g_paintChannelVal[channel] = cur * weight + g_paintChannelVal[channel];
    g_paintChannelWt[channel]  = weight + g_paintChannelWt[channel];
    return cur;
}

// sub_43E610 (0x43E610) — AVERAGE APPLY: identical blend to sub_43E570, but g_paintChannelVal
//  now holds the gathered MEAN (computed between the gather and apply passes).
float sub_43E610( float * /*ctrl*/, int channel, float cur, float strength, float weight )
{
    float t = strength * weight;
    if ( t < 0.0f ) t = 0.0f;
    else if ( t > 1.0f ) t = 1.0f;
    return ( 1.0f - t ) * cur + g_paintChannelVal[channel] * t;
}

// ── 4D lattice value noise (sub_4B75E0) + its baked perm/value tables ──────────
//  The noise paint mode samples this.  The two 256-entry tables are STATIC baked
//  constants in the original binary's .data — there is NO runtime RNG initialiser
//  (the long-"unlocatable init" was simply precomputed constant data).  Extracted
//  verbatim from the image: dword_24BA260 (permutation) and flt_24BA660 (value
//  table, stored here as raw IEEE-754 bit patterns so the port is bit-exact).
extern float grid_sizes[];                                       // engine_stubs (0x6DDE5C)

static const int kNoisePerm[256] = {            // 0x24BA260
    170,175, 33,114, 14,215, 18,117, 51, 24,128,166,122, 22,236, 91,
     95,166,207,242,192, 58,114, 69, 62,111,224, 80,195,187,254,230,
     42, 57,188,148,  0,148,109,154,108, 26, 61, 10, 49,225,195,  5,
    215, 26,181, 22,236,124,196,178,109,209,137,212, 70,162,158,185,
    251,  6,210,187,108,142,127, 84,186, 83,253, 55, 76, 31, 59,140,
    242,192,110, 80, 46,112,236,206,149,103, 27, 67, 87,129,142,146,
    226, 18, 49, 40,168,207,164,247,  5,221,174,144, 65,250,211,100,
    183,209,  7, 33, 26, 97,134, 44,220,140,198, 62,173,195,141, 37,
     38,216, 39,159,139,226,187, 83,178, 83,190,246,  6, 30,137,128,
    172,  9, 15,141,201, 24,179,221,233, 26, 55,232,127, 83,218,165,
    141, 81,132, 97,108,209, 36,189,169,207, 28,205, 18,181,219, 81,
    216,163,149, 22,209,225, 15,116,227,107,157,228,131,155, 50,196,
     95,249,150, 50, 32, 38,188,  7, 79,237, 54,246, 92,104,201,202,
    193, 87, 38,191, 72,134,186,  4,111, 91,167, 84,240,133,213,180,
     97,201, 43, 83,255,237,226,130,139,197,250,213, 90,100,209, 93,
    110, 98,210, 10,196, 81, 83, 31,181, 67,131,218,124,125,128, 40 };

static const unsigned int kNoiseValBits[256] = {    // 0x24BA660 (float bit patterns)
    0xbf6f9fdf,0x3f200340,0xbf03b707,0xbf6e63dd,0x3f394373,0xbd9d113a,0x3ee84dd1,0xbec30d86,
    0x3f3bc378,0x3e9f253e,0xbeaa4555,0xbf24e34a,0xbf48bb91,0x3dc05181,0x3f35f76c,0xbc2c8159,
    0x3f240348,0xbf4c8b99,0x3e898d13,0xbeb9a573,0xbeb6256c,0xbede4dbd,0xbedb05b6,0xbe2b7957,
    0x3dcf119e,0x3f428b85,0xbf67f3d0,0xbe6039c0,0xbf0e871d,0x3f602fc0,0x3efc85f9,0xbe05790b,
    0x3e6ca9d9,0x3f28bb51,0xbe412982,0x3ed925b2,0x3f792ff2,0x3f6473c9,0xbda9b153,0xbe912d22,
    0x3e4a7995,0xbf791bf2,0x3e4ef99e,0xbf46d38e,0xbe6ab9d5,0xbf6effde,0xbef2d5e6,0x3e089911,
    0xbf080310,0x3f32f766,0x3ed25da5,0xbf6513ca,0x3f4c2398,0xbea37547,0x3efa5df5,0xbf398f73,
    0x3d807101,0xbf083b10,0xbf4df79c,0x3f03f308,0x3f30e762,0x3dccf19a,0xbe91e524,0x3f70f7e2,
    0x3f4c9b99,0x3eaa8555,0x3e35596b,0xbea93d52,0x3c7981f3,0xbf7a8ff5,0x3dce119c,0x3f535ba7,
    0xbee74dcf,0xbe7ed9fe,0x3db89171,0x3f4d5f9b,0xbefb45f7,0x3f392772,0x3f47cf90,0x3f443b88,
    0xbf04e30a,0xbde9f1d4,0xbf77b7ef,0x3ed615ac,0x3ead355a,0xbf2ee35e,0xbf1f4f3f,0x3f7f3ffe,
    0xbf2e5f5d,0x3f5b8fb7,0x3f36fb6e,0xbeeae5d6,0x3eafad5f,0x3f7ebbfd,0x3e81e504,0x3eca1594,
    0xbed4ada9,0x3f7e97fd,0x3f362b6c,0xbd0f211e,0x3e94bd29,0x3e90c522,0x3f421384,0x3f3b4377,
    0x3f59ffb4,0xbf6b4bd7,0x3f35df6c,0x3f00ab01,0x3dfb91f7,0x3e92f526,0xbf05670b,0x3f6c8bd9,
    0xbbcf019e,0x3f1bdb38,0x3f431b86,0xbf437b87,0xbf4aab95,0x3f7263e5,0x3efc15f8,0xbf377b6f,
    0x3edf2dbe,0x3cdc41b9,0xbef045e1,0xbe6cd9da,0x3f174f2f,0xbf377f6f,0xbec6358c,0xbe7099e1,
    0x3ecec59e,0x3f7db7fb,0x3f0b3716,0xbf076b0f,0xbf426385,0xbeb38567,0x3f24b749,0x3e60a9c1,
    0xbf5377a7,0x3f1d333a,0x3ee23dc4,0xbe87dd10,0x3e9b1536,0xbca14143,0xbec00d80,0x3f241b48,
    0xbf2d335a,0xbe917d23,0xbf45638b,0x3e6179c3,0xbf26ab4d,0x3d316163,0xbf69cbd4,0xbf14b329,
    0x3f7af3f6,0xbf6d3bda,0x3f05bb0b,0xbf645fc9,0x3f204741,0xbea00d40,0x3e97e530,0xbed335a6,
    0x3d96112c,0xbea8bd51,0xbe4a2994,0xbf27034e,0xbf315f63,0xbf67fbd0,0xbefaa5f5,0xbf6cf3da,
    0x3f14f72a,0x3f72bbe5,0x3f29d354,0x3f6897d1,0xbe84b509,0xbf795ff3,0x3e63b9c7,0x3f73e7e8,
    0x3f495f93,0xbf720be4,0xbf7027e0,0xbf786ff1,0xbe98c532,0xbf6bcbd8,0xbf15c72c,0xbe9ee53e,
    0xbef695ed,0xbf67cbd0,0xbedcedba,0x3ef02de0,0x3f23c748,0xbd8f311e,0xbe43b987,0xbf1a4f35,
    0x3ed185a3,0xbf1ea33d,0xbec42d88,0xbe7af9f6,0xbd8cb119,0xbed285a5,0x3de7d1d0,0x3da7714f,
    0x3e112922,0xbea35d47,0xbe7c09f8,0xbf10c722,0xbf5f3bbe,0xbeb5056a,0xbf398773,0xbdd891b1,
    0x3f331366,0xbf070b0e,0xbde1b1c3,0x3ea64d4d,0x3e5499a9,0xbef27de5,0x3f3b1376,0xbf1eb33d,
    0xbec44d89,0x3e40c982,0xbee765cf,0xbe7369e7,0xbe34d96a,0x3f3beb78,0x3f27474f,0xbe8a8d15,
    0x3e69a9d3,0xbf081f10,0x3decb1d9,0xbe16792d,0xbee415c8,0xbf64a3c9,0x3e5279a5,0xbf7933f2,
    0xbda3f148,0xbf170f2e,0xbf6863d1,0xbe8e0d1c,0x3e1fb93f,0xbf6f7fdf,0xbe5999b3,0x3f003300,
    0x3ec1d584,0x3f5147a3,0x3eaedd5e,0xbe6fa9df,0xbe442988,0xbe111922,0xbf36436d,0x3f468f8d,
    0xbe226945,0xbf0c4719,0x3eae155c,0xbdd151a3,0x3ebcad79,0xbe36696d,0x3eab2556,0x3df3f1e8 };

// sub_4B75E0 (0x4B75E0) — quadrilinear value noise over a 4D integer lattice.  Two w-layers
//  (the do/while runs twice with the low byte of the w-index advancing) are interpolated by
//  the w-fraction.  Lattice cell origin uses (int)(c - 0.49999999907) (truncating _ftol2, so
//  effectively floor for the +c range the caller feeds it).  All perm indexing wraps mod 256.
float Radiant_ValueNoise4D( float a1, float a2, float a3, float a4 )
{
    const int   *P = kNoisePerm;
    const float *G = (const float *)kNoiseValBits;

    int ix = (int)( a1 - 0.4999999990686774 );  float fx = a1 - (double)ix;   // v30/v27
    int iy = (int)( a2 - 0.4999999990686774 );  float fy = a2 - (double)iy;   // v31/v28
    int iz = (int)( a3 - 0.4999999990686774 );  float fz = a3 - (double)iz;   // v29/v34
    int iw = (int)( a4 - 0.4999999990686774 );  float fw = a4 - (double)iw;   // v33/v25

    unsigned char bx = (unsigned char)ix;        // v5
    unsigned char by = (unsigned char)iy;        // v4
    float fy0 = fy, fy1 = 1.0f - fy;             // v7 / v8
    float fx0 = fx, fx1 = 1.0f - fx;             // v9 / v10

    float out[3];
    int wcur = iw;                               // v33 (low byte advances per layer)
    int i = 0;                                   // v6
    do
    {
        unsigned char v11 = (unsigned char)( bx + P[(unsigned char)( by + P[(unsigned char)( P[(unsigned char)wcur] + iz )] )] );
        float v22 = G[ P[v11] ];
        unsigned char v12 = (unsigned char)( bx + P[(unsigned char)( by + P[(unsigned char)( P[(unsigned char)wcur] + iz )] + 1 )] );
        float v23 = G[ P[(unsigned char)( v11 + 1 )] ];
        int   v13 = P[(unsigned char)( v12 + 1 )];
        float v24 = G[ P[v12] ];
        by = (unsigned char)iy;
        unsigned char v15 = (unsigned char)( iy + P[(unsigned char)( P[(unsigned char)wcur] + iz + 1 )] );
        unsigned char v16 = (unsigned char)( bx + P[v15] );
        unsigned char v17 = (unsigned char)( bx + P[(unsigned char)( v15 + 1 )] );
        float v35 = G[ P[(unsigned char)( v17 + 1 )] ] * fx0 + G[ P[v17] ] * fx1;
        float v18 = v35 * fy0;
        wcur = ( wcur & ~0xFF ) | (unsigned char)( wcur + 1 );   // LOBYTE(v33) = v33 + 1
        ++i;
        float v36 = G[ P[(unsigned char)( v16 + 1 )] ] * fx0 + G[ P[v16] ] * fx1;
        float v37 = v18 + v36 * fy1;
        float v19 = v37 * fz;
        float v38 = G[ v13 ] * fx0 + v24 * fx1;
        float v20 = v38 * fy0;
        float v39 = v23 * fx0 + v22 * fx1;
        float v40 = v20 + v39 * fy1;
        out[i] = v19 + v40 * ( 1.0f - fz );      // out[1] (layer w), out[2] (layer w+1)
    }
    while ( i < 2 );

    return fw * out[2] + ( 1.0f - fw ) * out[1];
}

// sub_43E670 (0x43E670) — NOISE: add 4D value noise (scaled by strength*weight) to the value.
//  The 4th noise coord is the per-channel w-offset g_paintChannelWt[channel], which sub_43E6F0
//  seeds to a per-stroke counter so successive strokes sample different noise slices.
float sub_43E670( float *ctrl, int channel, float cur, float strength, float weight )
{
    float s  = grid_sizes[g_qeglobals.d_gridsize] * 32.0f;
    float nx = ctrl[0] / s, ny = ctrl[1] / s, nz = ctrl[2] / s;
    return Radiant_ValueNoise4D( nx, ny, nz, g_paintChannelWt[channel] ) * strength * weight + cur;
}

// ── CurvEditDlg paint-parameter control table + getters (sub_401BB0/401C00/401C50) ──
//  The terrain-paint brush params (inner radius / outer radius / amplitude) are a SLIDER +
//  buddy-EDIT pair each, mirrored into this 3-entry table (IDB dword_73C6A0, 32-byte entries).
//  Ground-truthed layout + baked defaults/ranges/steps (read from the image):
//      slot 0: trackbar 1424 / edit 1428  value 16  range [0,1024] step 16   (inner radius)
//      slot 1: trackbar 1425 / edit 1429  value 64  range [0,1024] step 16   (outer radius)
//      slot 2: trackbar 1426 / edit 1430  value  8  range [0,  16] step  1   (amplitude exp)
//  PMESH_16 reads the value through the 3 getters; sub_4010D0 (CurveEdit_SnapStore) clamps a
//  new value to [min,max], snaps it to the step grid, and moves the slider + buddy edit.
struct curveEditCtrl_t           // 32 bytes (IDB dword_73C6A0[3] stride = 8 ints)
{
    int   id;                    // +0   trackbar id (1424/1425/1426)
    int   editId;                // +4   buddy-edit id (1428/1429/1430)  (IDB unk04)
    float value;                 // +8   the control's current value
    float minVal;                // +12  range minimum
    float maxVal;                // +16  range maximum
    float step;                  // +20  snap increment (and slider unit)
    void *hTrackbar;             // +24  the slider HWND  (IDB dword_73C6B8)
    void *hEdit;                 // +28  the buddy-edit HWND (IDB dword_73C6BC)
};
curveEditCtrl_t g_curveEditCtrls[3] = { { 0 }, { 0 }, { 0 } };   // 0x73C6A0

static float CurveEdit_GetCtrlValue( int wantId )   // the shared dword_73C6A0 lookup
{
    for ( int i = 0; i < 3; ++i )
        if ( g_curveEditCtrls[i].id == wantId )
            return g_curveEditCtrls[i].value;
    return 0.0f;                  // not found in the first 3 entries (binary returns 0.0)
}

float sub_401BB0() { return CurveEdit_GetCtrlValue( 1424 ); }                       // inner radius
float sub_401C00() { return CurveEdit_GetCtrlValue( 1425 ); }                       // outer radius
float sub_401C50() { return (float)pow( 2.0, (double)CurveEdit_GetCtrlValue( 1426 ) - 8.0 ); } // strength = 2^(v-8)

// ═══════════════════════════════════════════════════════════════════════════════
//  TERRAIN-PAINT CURSOR RING OVERLAY (Cam_Draw tail) — sub_43ED50 / PMESH_19_Radius /
//  PMESH_20_Radius_2.  Draws the inner+outer brush-radius rings around the cursor world
//  position in the 3D camera view, clipped to the patch surface (lines) and the selected
//  patch control points (falloff-coloured dots).  Ported 1:1 from the IDB; the dense
//  inline-x87 clip (sub_43ED50) is transcribed op-by-op from the disassembly.
//    DrawAdvancedTerrainEditCircle 0x441240 (camwnd.cpp) builds the two 16-segment rings,
//    loops active(+selected) patches calling PMESH_19_Radius (lines) + PMESH_20_Radius_2
//    (points).  ring color = {0,1,1,1} cyan; point ramp = {1,1,0,1}→{1,0.25,0.25,1}.
// ═══════════════════════════════════════════════════════════════════════════════

// ── sub_43ED50 (0x43ED50) — clip ONE triangle (a1,a2,a3 = curveVert xyz, XY plane) against
//    the 16-segment ring polygon `ring` and append the intersection as 3D line segments to
//    `outVerts` (running count `count`, color `color`).  Returns the new running count.
//    The emitted endpoints are nudged by (-0.125 * camera.vpn) so the ring floats slightly
//    in front of the surface (avoids z-fighting).  TRANSCRIBED OP-BY-OP from the disasm:
//    the hex-rays `*((float*)&v72+1)` reads are decompiler double-high-half artifacts over
//    physically-distinct stack floats (var_64 / var_8) — resolved here to real locals.
static int sub_43ED50( const float *a1, const float *a2, const float *a3,
                       const float *ring, const unsigned int *color, int count,
                       GfxPointVertex *outVerts )
{
    // Triangle edge line-equations in XY (edge0:a1->a2, edge1:a2->a3, edge2:a3->a1).  The
    // signed distance of a point P to edge k is  ek_x*P.y + ek_y*P.x - ck  (the binary's
    // `eN[1]*y + eN[0]*x - cN` form; outside == distance > 0).
    const float e0y = a2[1] - a1[1];              // var_58
    const float e0x = a1[0] - a2[0];              // var_54
    const float c0  = a1[0] * e0y + e0x * a1[1];  // var_50
    const float e1y = a3[1] - a2[1];              // var_4C
    const float e1x = a2[0] - a3[0];              // var_48
    const float c1  = a2[0] * e1y + e1x * a2[1];  // var_44
    const float e2y = a1[1] - a3[1];              // var_40
    const float e2x = a3[0] - a1[0];              // var_3C
    const float c2  = a3[0] * e2y + e2x * a3[1];  // var_38

    // edge-k coefficients packed so dist(P,k) = ex[k]*P.y + ey[k]*P.x - ec[k].
    const float ex[3] = { e0x, e1x, e2x };
    const float ey[3] = { e0y, e1y, e2y };
    const float ec[3] = { c0,  c1,  c2  };

    // Signed distances of the LAST ring vertex (ring[15] = ring[45..47]) against each edge —
    // the "previous" distances for the first segment's cull test.
    float pd0 = ring[46] * e0x + ring[45] * e0y - c0;  // var_74
    float pd1 = ring[46] * e1x + ring[45] * e1y - c1;  // var_70
    float pd2 = ring[46] * e2x + ring[45] * e2y - c2;  // var_6C

    // Plane-fit (computed lazily, once, on the first emitting segment) for projecting the
    // ring-edge clip points back onto the triangle's plane in Z.
    bool  needPlane = true;                   // var_1
    float nx = 0.0f, ny = 0.0f, nz0 = 0.0f;   // var_30 / var_28 / var_60 (z = nx*x + ny*y + nz0)
    float vnudge[3] = { 0.0f, 0.0f, 0.0f };   // var_90/8C/88 = -0.125 * camera.vpn

    float thresh = 0.0f;                       // v16/v18 — the carried "outside" barrier (=0)
    const float *cur = ring;                   // &ring[seg] (the "current" ring vertex x,y,z)

    for ( int seg = 0; seg < 16; ++seg )       // var_2C: 16 ring vertices/segments
    {
        const float vx = cur[0];               // ring[seg].x
        const float vy = cur[1];               // ring[seg].y
        const float vz = cur[2];               // ring[seg].z

        // current ring vertex's distances to the 3 edges (var_80/7C/78, ping-pong slots).
        // 0x43EE64: same crossed-coefficient half-plane form as pd0..pd2.
        const float dc0 = e0x * vy + e0y * vx - c0;
        const float dc1 = e1x * vy + e1y * vx - c1;
        const float dc2 = e2x * vy + e2y * vx - c2;

        // Cull: if BOTH this vertex and the previous one are outside the SAME edge (distance
        // > thresh), the ring segment can't cross the triangle interior -> nothing to emit.
        if ( ( thresh < dc0 && thresh < pd0 )
          || ( thresh < dc1 && thresh < pd1 )
          || ( thresh < dc2 && thresh < pd2 ) )
        {
            pd0 = dc0; pd1 = dc1; pd2 = dc2;
            cur += 3;
            continue;                          // LABEL_27 (thresh carried unchanged)
        }

        // Clip the ring EDGE (A = previous ring vertex, B = this ring vertex) against the
        // 3 triangle half-planes (Sutherland-Hodgman), carrying the clipped span across edges.
        // The previous of segment 0 wraps to vertex 15 (binary's var_C = 0xB4 = ring[45] byte).
        const float *prevR = ( seg == 0 ) ? ( ring + 45 ) : ( cur - 3 );
        float Ax = prevR[0], Ay = prevR[1], Az = prevR[2];   // v88/89/90
        float Bx = vx,        By = vy,        Bz = vz;        // v91/92/93
        float t = thresh;                      // v18 — per-span barrier (resets to 0 on a clip)
        bool  dropSpan = false;
        for ( int k = 0; k < 3; ++k )
        {
            const float dA = ex[k] * Ay + ey[k] * Ax - ec[k];   // var_64 (A vs edge k)
            const float dB = ex[k] * By + ey[k] * Bx - ec[k];   // var_8  (B vs edge k)
            if ( dA > t && dB > t )            // span entirely outside this edge -> drop
            {
                dropSpan = true;
                break;
            }
            if ( dA > t )                      // A outside, B inside -> clip A to the edge
            {
                const float u = dB - dA;
                Ax = ( Ax * dB - Bx * dA ) / u;
                Ay = ( Ay * dB - By * dA ) / u;
                Az = ( Az * dB - Bz * dA ) / u;
                t = 0.0f;
            }
            else if ( dB > t )                 // B outside, A inside -> clip B to the edge
            {
                const float u = dB - dA;
                Bx = ( Ax * dB - Bx * dA ) / u;
                By = ( Ay * dB - By * dA ) / u;
                Bz = ( Az * dB - Bz * dA ) / u;
                t = 0.0f;
            }
            // both inside -> keep A,B and t unchanged
        }

        if ( dropSpan )
        {
            pd0 = dc0; pd1 = dc1; pd2 = dc2;
            thresh = t;                        // carry the inner-loop barrier (v18) to v16
            cur += 3;
            continue;                          // LABEL_27 via 43F1EB (v18 = inner t)
        }

        // Plane fit (computed once, on the first span that survives clipping): solve the
        // triangle plane Z = nx*x + ny*y + nz0 and the -0.125*vpn float-out nudge.
        if ( needPlane )
        {
            needPlane = false;
            const float det = ( a3[0] - a2[0] ) * a1[1]
                            + ( a2[1] - a3[1] ) * a1[0]
                            + a2[0] * a3[1] - a3[0] * a2[1];      // 2*signed-area
            if ( det != 0.0f )
            {
                const float *vpn = g_pParentWnd->m_pCamWnd->camera.vpn;
                nx = ( ( a3[2] - a2[2] ) * a1[1] + ( a2[1] - a3[1] ) * a1[2]
                     + a2[2] * a3[1] - a2[1] * a3[2] ) / det;     // var_30
                ny = ( ( a2[2] - a3[2] ) * a1[0] + a1[2] * ( a3[0] - a2[0] )
                     + a2[0] * a3[2] - a3[0] * a2[2] ) / det;     // var_28
                nz0 = a1[2] - a1[0] * nx - ny * a1[1];            // var_60
                vnudge[0] = vpn[0] * -0.125f;                     // var_90
                vnudge[1] = vpn[1] * -0.125f;                     // var_8C
                vnudge[2] = -0.125f * vpn[2];                     // var_88
            }
            else
            {
                pd0 = dc0; pd1 = dc1; pd2 = dc2;
                thresh = t;                                        // binary falls through to LABEL_27
                cur += 3;
                continue;
            }
        }

        // Emit the clipped span as a depth-nudged 3D line.
        float p0[3], p1[3];
        p0[0] = Ax + vnudge[0];
        p0[1] = Ay + vnudge[1];
        p0[2] = ( ny * Ay + nx * Ax + nz0 ) + vnudge[2];
        p1[0] = Bx + vnudge[0];
        p1[1] = By + vnudge[1];
        p1[2] = ( nz0 + nx * Bx + ny * By ) + vnudge[2];
        count = R_Add3DLine( outVerts, (const orientation_t *)world_orient_matrix,
                             p0, p1, color, 1, count, 1362 );

        pd0 = dc0; pd1 = dc1; pd2 = dc2;
        thresh = 0.0f;                          // post-emit: v18 = 0 (43f1db fldz)
        cur += 3;
    }
    return count;
}

// ── PMESH_19_Radius (0x43F230) — emit the inner+outer ring LINE segments for ONE patch ──
//   AABB-cull the cursor circle against the patch's bbox, then per tessellated curveDef quad
//   (turned-edge aware) clip the two rings against the two surface triangles via sub_43ED50.
//     patch  = patchMesh_t* (the patch DEF)
//     cursor = float[3] cursor world pos (XY used)
//     innerR / outerR = ring radii (innerR < outerR)
//     innerRing / outerRing = the two 16-vertex ring arrays (3 floats each)
//     color  = packed GfxColor; count = running R_Add3DLine vertex count; outVerts = batch
//   Returns the new running count.
int PMESH_19_Radius( patchMesh_t *patch, const float *cursor, float innerR, float outerR,
                     const float *innerRing, const float *outerRing,
                     const unsigned int *color, int count, GfxPointVertex *outVerts )
{
    curvePatchDef_t *cdef = patch->curveDef;          // *(patch+20536)
    const float innerR2 = innerR * innerR;
    const float outerR2 = outerR * outerR;
    if ( outerR <= (double)innerR )
        Assert( PMESH_CPP, 3348, 0, "%s", "radius1 < radius2" );

    // AABB early-out: nearest-point distance² from the cursor XY to the patch's symbiont
    // brush bbox (pSymbiot is the brush_t DEF — mins@0x20 / maxs@0x2C).
    brush_t *sym = (brush_t *)patch->pSymbiot;        // *(patch+20540)
    float d2 = 0.0f;
    if ( cursor[0] < (double)sym->mins[0] )      { float d = sym->mins[0] - cursor[0]; d2 += d * d; }
    else if ( cursor[0] > (double)sym->maxs[0] ) { float d = sym->maxs[0] - cursor[0]; d2 += d * d; }
    if ( cursor[1] < (double)sym->mins[1] )      { float d = sym->mins[1] - cursor[1]; d2 += d * d; }
    else if ( cursor[1] > (double)sym->maxs[1] ) { float d = sym->maxs[1] - cursor[1]; d2 += d * d; }
    if ( innerR2 < (double)d2 && outerR2 < (double)d2 )
        return count;                                  // both rings miss the patch bbox

    if ( cdef->height - 1 <= 0 )
        return count;

    // turned_edge flags live in the patchMesh CONTROL grid (ctrl[*][*].turned_edge @ +132);
    // the binary indexes ctrl[j][i].turned_edge (col by row-stride 1280, row by col-stride 80).
    const char *turnBase = (const char *)patch + 132;

    for ( int i = 0; i < cdef->height - 1; ++i )       // v41 (outer)
    {
        const int width = cdef->width;                 // *cdef (reloaded each row)
        if ( width - 1 <= 0 )
            continue;
        const char *turnRow = turnBase + 80 * i;
        for ( int j = 0; j < width - 1; ++j )          // v43 (inner)
        {
            const int   base    = j + i * width;       // v18
            const int   nextRow = base + width;         // v19
            const char  turned  = *(turnRow + 1280 * j);// ctrl[j][i].turned_edge

            // The 4 quad corners (curveVert xyz, 44-byte stride).
            const float *vB  = (const float *)( (char *)cdef->verts + 44 * base );        // base
            const float *vB1 = (const float *)( (char *)cdef->verts + 44 * ( base + 1 ) );// base+1
            const float *vN  = (const float *)( (char *)cdef->verts + 44 * nextRow );     // nextRow
            const float *vN1 = (const float *)( (char *)cdef->verts + 44 * ( nextRow + 1 ) ); // nextRow+1

            // Split the quad into 2 triangles; the diagonal flips when turned&1 (binary's
            // sub_43ED50(v24,v25,v22) + sub_43ED50(v35,v36,v22) per branch).
            const float *triA0, *triA1, *triA2;        // first triangle (call 1)
            const float *triB0, *triB1, *triB2;        // second triangle (call 2)
            if ( ( turned & 1 ) != 0 )
            {
                triA0 = vB1; triA1 = vN1; triA2 = vB;  // (base+1, nextRow+1, base)
                triB0 = vN1; triB1 = vN;  triB2 = vB;  // (nextRow+1, nextRow, base)
            }
            else
            {
                triA0 = vB;  triA1 = vB1; triA2 = vN;  // (base, base+1, nextRow)
                triB0 = vB1; triB1 = vN1; triB2 = vN;  // (base+1, nextRow+1, nextRow)
            }

            // max corner XY-distance² from the cursor — the binary tests the 4 verts of the
            // FIRST triangle's corner set (v24,v25,v22,v36): v24=triA0, v25=triA1, v22=triA2,
            // v36=triB1.  (Together they span all 4 distinct quad corners.)
            float maxD = 0.0f;
            { float dx = triA0[0] - cursor[0], dy = triA0[1] - cursor[1]; float d = dy*dy + dx*dx; if ( d > 0.0f )  maxD = d; }
            { float dx = triA1[0] - cursor[0], dy = triA1[1] - cursor[1]; float d = dy*dy + dx*dx; if ( d > maxD ) maxD = d; }
            { float dx = triA2[0] - cursor[0], dy = triA2[1] - cursor[1]; float d = dy*dy + dx*dx; if ( d > maxD ) maxD = d; }
            { float dx = triB1[0] - cursor[0], dy = triB1[1] - cursor[1]; float d = dy*dy + dx*dx; if ( d > maxD ) maxD = d; }

            // INNER ring clips both triangles; OUTER ring clips both — each only when reached.
            if ( innerR2 < (double)maxD )
            {
                count = sub_43ED50( triA0, triA1, triA2, innerRing, color, count, outVerts );
                count = sub_43ED50( triB0, triB1, triB2, innerRing, color, count, outVerts );
            }
            if ( outerR2 < (double)maxD )
            {
                count = sub_43ED50( triA0, triA1, triA2, outerRing, color, count, outVerts );
                count = sub_43ED50( triB0, triB1, triB2, outerRing, color, count, outVerts );
            }
        }
    }
    return count;
}

// ── PMESH_20_Radius_2 (0x43F580) — emit falloff-coloured POINT markers at the SELECTED
//   patch's CONTROL points within [innerR,outerR] XY-distance of the cursor.  Colour lerps
//   the ramp {1,1,0,1}(yellow) -> {1,0.25,0.25,1}(red) by the smoothstep falloff weight.
//     count  = running R_AddPointCmd_W point count (flushed at 1361)
//     patch  = patchMesh_t*; cursor = float[3]; innerR/outerR = radii; outVerts = batch
//   Returns the new running count.
int PMESH_20_Radius_2( int count, patchMesh_t *patch, const float *cursor,
                       float innerR, float outerR, GfxPointVertex *outVerts )
{
    // yellow -> red falloff ramp (IDB flt_6E0410 = {{1,1,0,1},{1,0.25,0.25,1}}; modelled as a
    // real const table, NOT the binary's magic-address 0x6E0410 deref).  A 3rd entry mirrors
    // the binary reading ramp[ri+1] when ri==1 (w==1): there frac==0 so its value is unused;
    // the extra row keeps the indexed read in-bounds (binary read benign trailing data).
    static const float s_ramp[3][4] = { { 1.0f, 1.0f, 0.0f, 1.0f },
                                        { 1.0f, 0.25f, 0.25f, 1.0f },
                                        { 1.0f, 0.25f, 0.25f, 1.0f } };

    const float innerR2 = innerR * innerR;
    const float outerR2 = outerR * outerR;
    if ( outerR <= (double)innerR )
        Assert( PMESH_CPP, 3434, 0, "%s", "radius1 < radius2" );

    // AABB early-out vs the patch symbiont brush bbox (same as PMESH_19).
    brush_t *sym = (brush_t *)patch->pSymbiot;        // patch[5135] = patch+20540
    float d2 = 0.0f;
    if ( cursor[0] < (double)sym->mins[0] )      { float d = sym->mins[0] - cursor[0]; d2 += d * d; }
    else if ( cursor[0] > (double)sym->maxs[0] ) { float d = sym->maxs[0] - cursor[0]; d2 += d * d; }
    if ( cursor[1] < (double)sym->mins[1] )      { float d = sym->mins[1] - cursor[1]; d2 += d * d; }
    else if ( cursor[1] > (double)sym->maxs[1] ) { float d = sym->maxs[1] - cursor[1]; d2 += d * d; }
    if ( innerR2 < (double)d2 && outerR2 < (double)d2 )
        return count;

    if ( patch->width <= 0 )
        return count;

    for ( int col = 0; col < patch->width; ++col )     // v30, *a2 = width
    {
        for ( int row = 0; row < patch->height; ++row )// i, v7[1] = height
        {
            drawVert_t *cp = &patch->ctrl[col][row];   // a2+16 base, 80-byte stride
            const float dx = cp->xyz[0] - cursor[0];
            const float dy = cp->xyz[1] - cursor[1];
            const float dist2 = dy * dy + dx * dx;
            if ( outerR2 <= (double)dist2 )
                continue;                               // beyond the outer ring -> skip

            // weight: 1.0 inside the inner ring, smoothstep 1->0 across inner..outer.
            float w;
            if ( innerR2 < (double)dist2 )
            {
                const float d = (float)sqrt( (double)dist2 );
                const float ti = ( d - innerR ) / ( outerR - innerR );
                w = 1.0f - ti * ti * ( 3.0f - ( ti + ti ) );   // smoothstep falloff
            }
            else
            {
                w = 1.0f;
            }

            // lerp ramp[ri] -> ramp[ri+1] by the fractional part of w (binary: v17 = (int)w*16
            // = ramp byte index; frac = w - (int)w).  ri==0 for w<1 (yellow->red across the
            // falloff); ri==1 only at w==1 (frac==0 -> exactly red, ramp[1]).
            const int   ri = (int)w;                    // 0 unless w==1.0 -> 1 (then frac=0)
            const float frac = w - (float)ri;           // v36
            float rgba[4];
            rgba[0] = ( s_ramp[ri + 1][0] - s_ramp[ri][0] ) * frac + s_ramp[ri][0];
            rgba[1] = ( s_ramp[ri + 1][1] - s_ramp[ri][1] ) * frac + s_ramp[ri][1];
            rgba[2] = ( s_ramp[ri + 1][2] - s_ramp[ri][2] ) * frac + s_ramp[ri][2];
            rgba[3] = frac * ( s_ramp[ri + 1][3] - s_ramp[ri][3] ) + s_ramp[ri][3];
            GfxColor packed;
            Byte4PackPixelColor( rgba, &packed );

            GfxPointVertex *out = &outVerts[count];
            // KEEP_VERBOSE: inlined OrientationPosToWorldPos (q_shared.cpp:1599; the port
            // carrier is draw.cpp) — the binary constant-folded its identity orientation,
            // so there is no orient to route a call through.
            if ( (const float *)cp->xyz == (const float *)out )   // pos != out self-overlap guard
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\q_shared.cpp",
                        1599, 0, "%s", "pos != out" );

            // identity transform (the binary's `x*0 + ctrl + ...` zero-matrix passthrough).
            out->xyz[0] = cp->xyz[1] * 0.0f + cp->xyz[0] + 0.0f + cp->xyz[2] * 0.0f;
            out->xyz[1] = cp->xyz[0] * 0.0f + 0.0f + cp->xyz[1] + cp->xyz[2] * 0.0f;
            out->xyz[2] = 0.0f * cp->xyz[1] + cp->xyz[0] * 0.0f + 0.0f + cp->xyz[2];
            *(GfxColor *)out->color = packed;
            ++count;
            if ( count == 1362 )                        // a1 == 1361 then ++ -> flush
            {
                R_AddPointCmd_W( (short)count, 6, outVerts );
                count = 0;
            }
        }
    }
    return count;
}


// sub_4010D0 core — bind a slot's id/range/step/default, and clamp+snap a value to the grid.
//  (Win32-free: the slider/edit HWND moves live in the dialog, which calls these.)
void CurveEdit_BindData( int slot, int trackbarId, int editId, float defVal, float mn, float mx, float step )
{
    if ( slot < 0 || slot >= 3 ) return;
    curveEditCtrl_t *c = &g_curveEditCtrls[slot];
    c->id = trackbarId; c->editId = editId; c->minVal = mn; c->maxVal = mx; c->step = step; c->value = defVal;
}
// Clamp to [min,max] then snap to the step grid (matches sub_4010D0); store + return the value.
float CurveEdit_SnapStore( int slot, float value )
{
    if ( slot < 0 || slot >= 3 ) return value;
    curveEditCtrl_t *c = &g_curveEditCtrls[slot];
    if ( c->step <= 0.0f ) { c->value = value; return value; }     // unbound (e.g. gate seed) — no snap
    if ( value < c->minVal ) value = c->minVal;
    if ( value > c->maxVal ) value = c->maxVal;
    int idx = (int)floorf( ( value - c->minVal ) / c->step + 0.5f );
    c->value = (float)idx * c->step + c->minVal;
    return c->value;
}
int   CurveEdit_Slots()                 { return 3; }
int   CurveEdit_TrackbarId( int slot )  { return ( slot >= 0 && slot < 3 ) ? g_curveEditCtrls[slot].id : 0; }
int   CurveEdit_EditId( int slot )      { return ( slot >= 0 && slot < 3 ) ? g_curveEditCtrls[slot].editId : 0; }
float CurveEdit_Value( int slot )       { return ( slot >= 0 && slot < 3 ) ? g_curveEditCtrls[slot].value : 0.0f; }
float CurveEdit_Min( int slot )         { return ( slot >= 0 && slot < 3 ) ? g_curveEditCtrls[slot].minVal : 0.0f; }
float CurveEdit_Step( int slot )        { return ( slot >= 0 && slot < 3 ) ? g_curveEditCtrls[slot].step : 1.0f; }
int   CurveEdit_StepCount( int slot )                          // slider range = [0, (max-min)/step]
{
    if ( slot < 0 || slot >= 3 ) return 0;
    curveEditCtrl_t *c = &g_curveEditCtrls[slot];
    return ( c->step > 0.0f ) ? (int)( ( c->maxVal - c->minVal ) / c->step + 0.5f ) : 0;
}
int   CurveEdit_StepIndex( int slot )                          // slider thumb pos for the current value
{
    if ( slot < 0 || slot >= 3 ) return 0;
    curveEditCtrl_t *c = &g_curveEditCtrls[slot];
    return ( c->step > 0.0f ) ? (int)floorf( ( c->value - c->minVal ) / c->step + 0.5f ) : 0;
}
void  CurveEdit_SetHwnds( int slot, void *hTrackbar, void *hEdit )
{
    if ( slot >= 0 && slot < 3 ) { g_curveEditCtrls[slot].hTrackbar = hTrackbar; g_curveEditCtrls[slot].hEdit = hEdit; }
}
float CurveEdit_SetFromStep( int slot, int pos )               // slider moved -> value = min + pos*step
{
    if ( slot < 0 || slot >= 3 ) return 0.0f;
    curveEditCtrl_t *c = &g_curveEditCtrls[slot];
    c->value = c->minVal + (float)pos * c->step;
    return c->value;
}
// Value shown in the buddy edit (sub_401000): amplitude (id 1426) shows 2^(v-8), others raw.
float CurveEdit_DisplayValue( int slot )
{
    if ( slot < 0 || slot >= 3 ) return 0.0f;
    return ( g_curveEditCtrls[slot].id == 1426 ) ? sub_401C50() : g_curveEditCtrls[slot].value;
}
// Convert a typed edit value to the stored value (sub_401CF0): amplitude types the amplitude,
// stored as the exponent log2(amp)+8; others store as-typed.
float CurveEdit_InputToValue( int slot, float typed )
{
    if ( slot >= 0 && slot < 3 && g_curveEditCtrls[slot].id == 1426 )
        return (float)( log( (double)typed ) / log( 2.0 ) + 8.0 );
    return typed;
}
// sub_4015C0's slider-range edits (IDC_SURF_DLG_TBOX_INNER/OUTER_RAD_2 = 1482/1483): set the
//  inner/outer slider's max range, step = max/64 (so the slider keeps 64 positions); re-snap
//  the current value into the new grid.  (flt_73C6B0/B4 = slot0 max/step, flt_73C6D0/D4 slot1.)
void CurveEdit_SetRange( int slot, float maxVal )
{
    if ( slot < 0 || slot >= 3 ) return;
    curveEditCtrl_t *c = &g_curveEditCtrls[slot];
    c->maxVal = maxVal;
    c->step   = maxVal * 0.015625f;            // /64
    CurveEdit_SnapStore( slot, c->value );     // re-grid the existing value
}

// ── PMESH_16 (0x43DED0) — apply one paint stroke to a patch's control grid ────
//  The terrain-paint apply core: for every control point within the brush, blend the
//  selected channels (height + RGBA) toward the stroke using the smoothstep falloff
//  weight (sub_43DB60).  channelMask bits: 1=height, 8=R, 4=G, 2=B, 0x10=A (callback
//  channel indices 0/3/2/1/4).  center = the stroke's world XY; cellInfo = the source
//  value (raw for height, *0.0625 for colour); cb = the mode callback (raise/set/avg).
//  Radii + strength come from the CurvEditDlg getters; the dialog drives those (later piece).
typedef float (*PaintCallback)( float *cp, int channel, float cur, float strength, float weight );

extern int       Sys_Printf( const char *fmt, ... );           // win_qe3.cpp
extern void      Undo_AddBrush( entity_brush_s *pBrushInst );  // undo.cpp (0x45E680)
extern void      Undo_AddEntity( int owner );                  // undo.cpp (0x45E8B0)
extern undo_s   *g_lastundo;                                   // undo.cpp (0x23F162C)

// sub_4A56B0 (0x4A56B0) — squared distance from a point to an AABB (0 when inside).
static float sub_4A56B0( const float *center, const float *mins, const float *maxs )
{
    float d2 = 0.0f;
    for ( int i = 0; i < 3; ++i )
    {
        const float lo = mins[i] - center[i];
        if ( lo > 0.0f ) { d2 += lo * lo; continue; }   // center < mins -> (mins-center)^2
        const float hi = center[i] - maxs[i];
        if ( hi > 0.0f )   d2 += hi * hi;                // center > maxs -> (center-maxs)^2
    }                                                    // else inside this axis -> 0
    return d2;
}

// sub_45E770 (0x45E770) — register a painted patch with the current undo record
//  (add the owning entity too when it is fixed-size), so a paint stroke is undoable.
static void sub_45E770( entity_brush_s *pBrushInst )
{
    if ( !g_lastundo )
    {
        Sys_Printf( "Undo_AddBrush: no last undo.\n" );
        return;
    }
    if ( g_lastundo->entitylist.next != &g_lastundo->entitylist )
        Sys_Printf( "Undo_AddBrush: WARNING adding brushes after entity.\n" );

    entity_s *owner = pBrushInst->owner;
    if ( *(int *)&owner->eclass->fixedsize )           // fixed-size entity -> also track the entity
        Undo_AddEntity( (int)(intptr_t)owner );
    Undo_AddBrush( pBrushInst );
}

// Per-colour-byte apply: blend, clamp to [0,255], round (the binary's fistp + 2^-30 guard).
static unsigned char PMESH_16_PaintByte( PaintCallback cb, float *cp, int channel,
                                         unsigned char cur, float cellInfo, float weight )
{
    float v = cb( cp, channel, (float)cur, cellInfo * 0.0625f, weight );   // dbl_6F4520 = 0.0625
    if ( v < 0.0f )        v = 0.0f;                                       // dbl_6F40B8 = 0
    else if ( v > 255.0f ) v = 255.0f;                                     // dbl_6F4190 / flt_6F4720 = 255
    return (unsigned char)lrintf( v + 9.313225746154785e-10f );            // dbl_6F4220 = 2^-30, fistp rounds
}

void PMESH_16( selbrush_t *b, char channelMask, float *center, float cellInfo, PaintCallback cb )
{
    const float strength = sub_401C50();              // v44
    if ( strength == 0.0f )
        return;
    const float innerR = sub_401BB0();                // v35
    const float outerR = sub_401C00();                // v43
    if ( outerR * outerR < sub_4A56B0( center, b->def->mins, b->def->maxs ) )
        return;                                       // stroke doesn't reach this brush's bbox
    if ( FilterBrush( b, 0 ) || ( b->brushFlags & 0x20 ) != 0 )
        return;

    iassert( b->patch->def == b->def->patch );
    patchMesh_t *def = b->patch->def;
    if ( def->width <= 0 )
        return;

    char anyChanged = 0;
    for ( int col = 0; col < def->width; ++col )
    {
        for ( int row = 0; row < def->height; ++row )
        {
            drawVert_t *cp = &def->ctrl[col][row];
            if ( ( cp->turned_edge & 2 ) != 0 )       // ctrl+76 & 2: point excluded from paint
                continue;
            const float w = sub_43DB60( center, cp->xyz, innerR, outerR, strength );
            if ( w == 0.0f )
                continue;

            anyChanged = 1;
            if ( !def->xx22b )                        // first touched point -> mark dirty + undo
            {
                def->xx22b = 1;
                sub_45E770( def->pSymbiot );
            }

            if ( channelMask & 1 )                    // HEIGHT (xyz[2], channel 0; raw cellInfo)
                cp->xyz[2] = cb( cp->xyz, 0, cp->xyz[2], cellInfo, w );
            if ( channelMask & 8 )                    // R (ctrl+48, channel 3)
                cp->vert_color.r = PMESH_16_PaintByte( cb, cp->xyz, 3, cp->vert_color.r, cellInfo, w );
            if ( channelMask & 4 )                    // G (ctrl+49, channel 2)
                cp->vert_color.g = PMESH_16_PaintByte( cb, cp->xyz, 2, cp->vert_color.g, cellInfo, w );
            if ( channelMask & 2 )                    // B (ctrl+50, channel 1)
                cp->vert_color.b = PMESH_16_PaintByte( cb, cp->xyz, 1, cp->vert_color.b, cellInfo, w );
            if ( channelMask & 0x10 )                 // A (ctrl+51, channel 4)
                cp->vert_color.a = PMESH_16_PaintByte( cb, cp->xyz, 4, cp->vert_color.a, cellInfo, w );
        }
    }
    if ( anyChanged )
        Patch_Rebuild( def, 1 );
}

// ── sub_43E4B0 (0x43E4B0) — apply the stroke to every patch in a brush list ────
void sub_43E4B0( PaintCallback cb, selbrush_t *brushlist, char channelMask, float *center, float cellInfo )
{
    for ( selbrush_t *i = brushlist->next; i != brushlist; i = i->next )
        if ( i->patch )
            PMESH_16( i, channelMask, center, cellInfo, cb );
}

// ── sub_43E4F0 (0x43E4F0) — dispatch a paint stroke to the selection (and, when the
//    CurvEditDlg "apply to unselected patches too" box is checked, the active list too) ──
extern int CurvEditDlg_OnSomeSetting();   // patchdialog.cpp — checkbox 1432 (dword_25D6570)
void sub_43E4F0( PaintCallback cb, float *center, char channelMask, float cellInfo )
{
    if ( CurvEditDlg_OnSomeSetting() )
        sub_43E4B0( cb, &active_brushes, channelMask, center, cellInfo );
    sub_43E4B0( cb, &selected_brushes, channelMask, center, cellInfo );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Soft-selection HEIGHT DRAG (paint mode 1 "Drag Up/Down"): drag terrain control
//  points up/down; nearby points follow with a smoothstep falloff over the brush radii.
// ═══════════════════════════════════════════════════════════════════════════════
// sub_43DA20 (0x43DA20) — soft-selection falloff weight for a control point at `pos` (XY):
//  amplitude if within the inner radius of ANY drag move-point; 0 if the nearest move-point
//  is beyond the outer radius; else amplitude * smoothstep(1 -> 0) across inner..outer.
static float sub_43DA20( const float *pos )
{
    const float amp = sub_401C50();
    if ( amp == 0.0f )
        return 0.0f;
    const float innerR = sub_401BB0();
    const float outerR = sub_401C00();
    const float inner2 = innerR * innerR;
    float nearest2 = 3.4028235e38f;                       // FLT_MAX
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        const float *mp = g_qeglobals.d_move_points[i]->xyz;
        const float dx = pos[0] - mp[0];
        const float dy = pos[1] - mp[1];
        const float d2 = dx * dx + dy * dy;
        if ( d2 <= inner2 )
            return amp;                                    // inside inner radius -> full weight
        if ( d2 < nearest2 )
            nearest2 = d2;
    }
    if ( outerR * outerR <= nearest2 )                     // nearest move-point beyond outer -> 0
        return 0.0f;
    const float d = sqrtf( nearest2 );
    const float t = ( d - innerR ) / ( outerR - innerR );
    return ( 1.0f - t * t * ( 3.0f - ( t + t ) ) ) * amp;  // smoothstep * amplitude
}

// sub_43DC10 (0x43DC10) — apply a soft-selection height drag to one patch: add weight*amount
//  to every control point's Z (skipping turned_edge&2 points), then rebuild.
static void sub_43DC10( patchMesh_t *def, float amount )
{
    for ( int col = 0; col < def->width; ++col )
    {
        for ( int row = 0; row < def->height; ++row )
        {
            drawVert_t *cp = &def->ctrl[col][row];
            if ( ( cp->turned_edge & 2 ) != 0 )
                continue;
            const float w = sub_43DA20( cp->xyz );
            if ( w != 0.0f )
                cp->xyz[2] += w * amount;
        }
    }
    Patch_Rebuild( def, 1 );
}

// PMESH_15 (0x43DCA0) — apply the soft-sel height drag to every patch in a brush list.
//  (Iterates the display list via ->next / ->patch, exactly like sub_43E4B0 — the IDB's
//  ->onext / ->mins[0] are struct-typing artifacts over the same offsets +4 / +0x20.)
static void PMESH_15( selbrush_t *list, float amount )
{
    for ( selbrush_t *i = list->next; i != list; i = i->next )
        if ( i->patch )
        {
            if ( i->patch->def != i->def->patch )
                Assert( PMESH_CPP, 2747, 0, "%s", "b->patch->def == b->def->patch" );
            sub_43DC10( i->patch->def, amount );
        }
}

// sub_43DD00 (0x43DD00) — soft-sel height drag dispatch: the active (unselected) patches
//  too when the dialog's "apply to unselected" box (1432) is checked, then the selection.
void sub_43DD00( float amount )
{
    if ( CurvEditDlg_OnSomeSetting() )
        PMESH_15( &active_brushes, amount );
    PMESH_15( &selected_brushes, amount );
}

// Seed a CurvEditDlg control-table slot so the getters (sub_401BB0/401C00/401C50)
// return real radii/strength; CAdvPatchEditDlg pushes edit-box changes through here.
void CurveEdit_SetCtrl( int slot, int id, float value )
{
    if ( slot >= 0 && slot < 3 ) { g_curveEditCtrls[slot].id = id; g_curveEditCtrls[slot].value = value; }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  AdvPatchEditDlg / CurvEditDlg — terrain-paint dialog + apply chain: SHIPPED.
//  ───────────────────────────────────────────────────────────────────────────────
//  The dialog (binary 0x25D6098, PE resource 0xbb "Advanced Patch Editing Options")
//  is fully built in patchdialog.cpp; the apply chain + all leaf math live above and
//  in patchdialog.cpp.  Wiring (all ported):
//   • params: trackbars 1424/1425/1426 + buddy edits 1428/1429/1430 + max-range edits
//     1482/1483 -> g_curveEditCtrls (CurveEdit_BindData/SnapStore); getters sub_401BB0
//     (inner), sub_401C00 (outer), sub_401C50 (strength = 2^(value-8)).
//   • 7 mode RADIOs (sub_401DB0): 1435 raise(0) / 1439 drag-up-down(1, soft-sel height
//     drag via Patch_UpdateSelected->sub_43DD00) / 1437 flatten(2) / 1436 smooth(3) /
//     1438 NOISE(4, sub_43E670 + Radiant_ValueNoise4D) / 1468 grab-value(5) / 1440 off(6).
//   • channel CHECKs 1469..1474 -> the PMESH_16 mask (built in sub_43E6F0).
//   • Colour... 1460 / Alpha... 1464 + owner-draw swatches 1462/1466 -> dword_25D65A4 /
//     byte_25D65A8; height edit 1467 (OnHeightText); soft-sel checkboxes 1431/1432.
//  sub_43E6F0 (paint drag) + Patch_UpdateSelected (soft-sel drag) are wired into the
//  drag path.  Gates: paintfalloff + paintapply + softseldrag.  The brush-radius circle
//  overlay (DrawAdvancedTerrainEditCircle 0x441240, camwnd.cpp) + its clip helpers
//  PMESH_19_Radius / PatchRing_ClipTriangleToRing (sub_43ED50) / PMESH_20_Radius_2 (above)
//  are now PORTED and wired into the Cam_Draw tail.  Gate: terraincircle.
// ═══════════════════════════════════════════════════════════════════════════════

// ── Patch_TurnEdge (0x44b4c0) — flip a terrain quad's diagonal ────────────────
//  Ray-pick the nearest terrain edge across all selected patches (PMESH_51), then
//  toggle that cell's turned_edge diagonal and rebuild.  Undo-bracketed.  `org`/`dir`
//  arrive as int-cast float* ray endpoints (drag.cpp's trace_start / trace_dir).
void Patch_TurnEdge( int org, int dir )
{
    const float *rayOrg = (const float *)(intptr_t)org;
    const float *rayDir = (const float *)(intptr_t)dir;

    float        best = 3.4028235e38f;   // v15
    patchMesh_t *hitDef = nullptr;       // def
    int          hitCol = 0, hitRow = 0; // v3 / v14

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        patch_t *patch = b->patch;
        if ( !patch )
            continue;
        if ( patch->def->type != PATCH_TERRAIN )
            continue;

        float dist;
        int   col, row;
        if ( PMESH_51( rayOrg, rayDir, patch, &dist, &col, &row, nullptr, nullptr ) )
        {
            if ( dist < best )
            {
                best   = dist;
                hitCol = col;
                hitDef = b->patch->def;
                hitRow = row;
            }
        }
    }

    if ( !hitDef )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "Turn terrain edge" );

    entity_brush_s *pSymbiot = hitDef->pSymbiot;
    if ( g_lastundo )
    {
        if ( g_lastundo->entitylist.next != &g_lastundo->entitylist )
            Sys_Printf( "Undo_AddBrush: WARNING adding brushes after entity.\n" );
        entity_s *owner = pSymbiot->owner;
        if ( *(int *)&owner->eclass->fixedsize )
            Undo_AddEntity( (int)(intptr_t)owner );
        Undo_AddBrush( pSymbiot );
    }
    else
    {
        Sys_Printf( "Undo_AddBrush: no last undo.\n" );
    }

    hitDef->ctrl[hitCol][hitRow].turned_edge ^= 1u;
    Patch_Rebuild( hitDef, 1 );

    g_nUpdateBits = -1;

    // Re-stamp the (possibly rebuilt) symbiont with the open undo id — Undo_EndBrushList
    // body for the single brush (Patch_Rebuild can swap pSymbiot, so reload it).
    pSymbiot = hitDef->pSymbiot;
    if ( g_lastundo && !g_lastundo->done )
    {
        pSymbiot->ownerPrev = (selbrush_t *)(intptr_t)g_lastundo->id;
        entity_s *owner = pSymbiot->owner;
        if ( *(int *)&owner->eclass->fixedsize )
            owner->epairEdits = g_lastundo->id;
    }

    Undo_End();
}

// ════════════════════════════════════════════════════════════════════════════
//  PATCH BEND MODE - the Curve->Bend interactive editing mode: pick a rotation axis (a
//  whole row/column), pick the "normal" portion of the patch that bends, set the rotate
//  origin, then drag to bend.
//  The bend-state INFO DIALOG (ShowInfoDialog / off_25D5BF0, the "select rotation
//  axis..." prompt) shipped with patchdialog.cpp (IDD_INFORMATION, PE resource 150), so
//  its calls are live.  Mode state, axis selection and the rotate origin are set
//  identically to the binary.
// ════════════════════════════════════════════════════════════════════════════

extern signed int QE_SingleBrush();                         // qe3.cpp 0x48C8B0
extern float      g_vBendOrigin[3];                         // drag.cpp 0x231F548
extern float      g_vRotateOrigin[3];                       // drag.cpp 0x23F1658

// patchdialog.cpp — the modeless "Information" state prompt (0x40BE90) + its messages.
extern void        ShowInfoDialog( const char *msg );
extern void        HideInfoDialog();
extern const char *g_pBendStateMsg[4];                      // 0x73B114
extern const char *g_pInsDelStateMsg;                       // 0x73B128

// Bend-mode state (IDB globals; defined here — pmesh is the patch home).
int  g_nPatchBendState = 0;     // 0x73B10C  bend state machine (0 = BEND_SELECT_ROTATION)
char g_bPatchAxisOnRow = 0;     // 0x739B0D  rotation axis runs along a ROW (else a column)
char g_bPatchLowerEdge = 0;     // 0x739B0E  the "lower" side of the axis is the bend normal
int  g_nPatchAxisIndex = 0;     // 0x25D5B0C row/column index of the rotation axis
int  g_nPatchInsertState = 0;   // 0x73B110  redisperse (ins/del) mode state machine

// ── Patch_Save (0x446ae0) — snapshot a patch for the bend/ins-del CANCEL path ──
//  Copies width/height/type and the full 16×16 control grid into the patchSave_* image
//  (read back by Patch_Restore / Patch_BendHandleESC on Escape).
void Patch_Save( patchMesh_t *p )
{
    patchSave_width  = p->width;
    patchSave_height = p->height;
    patchSave_type   = p->type;
    memcpy( patchSave_ctrl, p->ctrl, 0x5000u );
}

// ── Patch_InsDelToggle (0x447e50) — enter/leave the "redisperse control points" mode ──
//  Toggle OFF: clear g_qeglobals_redispersePatchVerts, refresh the patch toolbar.
//  Toggle ON: requires a single selected patch; snapshots it (patchSave_*) for the CANCEL
//  path, arms the redisperse state machine (insert-state 0, axis = row 0), and shows the
//  ins/del info prompt.  The info DIALOG (off_25D5BF0 hide / ShowInfoDialog show) is
//  DEFERRED — same parked MFC-dialog layer as the bend-state prompt above.  Reached from
//  OnPatchRedisperse (toolbar 32872) and the Curve menu.
void Patch_InsDelToggle()
{
    extern int g_qeglobals_redispersePatchVerts;            // engine_stubs.cpp (0x25D5A6B)
    if ( g_qeglobals_redispersePatchVerts )
    {
        g_qeglobals_redispersePatchVerts = 0;
        HideInfoDialog();                                 // 0x447e71
        CMainFrame_UpdatePatchToolbarButtons();
    }
    else
    {
        patch_t *patch;
        patchMesh_t *def;
        if ( QE_SingleBrush()
             && ( patch = selected_brushes.next->patch ) != nullptr
             && ( def = patch->def ) != nullptr )
        {
            patchSave_width  = def->width;
            patchSave_height = def->height;
            patchSave_type   = def->type;
            memcpy( patchSave_ctrl, def->ctrl, 0x5000u );
            g_qeglobals_redispersePatchVerts = 1;
            g_nPatchInsertState = 0;
            g_bPatchAxisOnRow   = 1;
            g_nPatchAxisIndex   = 0;
            ShowInfoDialog( g_pInsDelStateMsg );          // 0x447f01
        }
        else
        {
            Sys_Printf( "Must work with a single patch" );
        }
    }
}

// ── Patch_BendHandleTAB (0x447980) — TAB inside BEND mode ─────────────────────
//  Cycles whatever the current bend state is asking for.  Shift reverses the step.
//    state 0 (pick rotation axis) : step the axis index by +/-2, wrapping across the
//                                   row<->column boundary (matching the binary's
//                                   `4*!shift - 2` step and its wrap targets).
//    state 1 (pick bend origin)   : step g_nBendOriginIndex by +/-1 with wrap, then
//                                   re-read g_vBendOrigin from the picked control point.
//    state 2                      : toggle g_bPatchLowerEdge.
//  The control-point index arithmetic is the binary's verbatim: 5*(a + 16*b) floats,
//  i.e. the flat float offset of ctrl[b][a].xyz within the 16x16 x 80-byte grid.
int g_nBendOriginIndex = 0;     // 0x25D5B08 — index of the picked bend-origin ctrl point
void Patch_BendToggle();        // below (0x4478E0)
void Patch_InsDelToggle();      // below (0x447E50)
void Patch_BendHandleTAB()
{
    if ( !g_bPatchBendMode )
        return;

    patch_t *patch;
    patchMesh_t *def;
    if ( !QE_SingleBrush()
         || ( patch = selected_brushes.next->patch ) == nullptr
         || ( def = patch->def ) == nullptr )
    {
        Patch_BendToggle();
        Sys_Printf( "No patch to bend!" );
        return;
    }
    if ( ( def->type & 0x40 ) != 0 )
    {
        Sys_Printf( "Patch_BendHandleTAB: Bending of terrain curves not implemented\n" );
        return;
    }

    const bool shift = ( GetKeyState( VK_SHIFT ) < 0 );

    if ( g_nPatchBendState == 0 )
    {
        const int idx = 4 * ( shift ? 0 : 1 ) - 2 + g_nPatchAxisIndex;   // +2 / -2
        g_nPatchAxisIndex = idx;
        if ( g_bPatchAxisOnRow )
        {
            const bool wrap = shift ? ( idx <= 0 ) : ( idx >= def->height );
            if ( wrap )
            {
                g_bPatchAxisOnRow = 0;
                g_nPatchAxisIndex = shift ? ( def->width - 1 ) : 1;
            }
        }
        else
        {
            const bool wrap = shift ? ( idx <= 0 ) : ( idx >= def->width );
            if ( wrap )
            {
                g_bPatchAxisOnRow = 1;
                g_nPatchAxisIndex = shift ? ( def->height - 1 ) : 1;
            }
        }
        g_nUpdateBits = -1;
        return;
    }

    if ( g_nPatchBendState == 1 )
    {
        int idx = 2 * ( shift ? 0 : 1 ) - 1 + g_nBendOriginIndex;        // +1 / -1
        g_nBendOriginIndex = idx;
        // idb: v6 = 5*(a + 16*b); v7 = 4*v6; ctrl[0][0].xyz[v7] — i.e. the flat control
        // point (a + 16*b) == ctrl[b][a] (80-byte drawVert_t stride).
        const drawVert_t *cp;
        if ( g_bPatchAxisOnRow )
        {
            if ( shift ) { if ( idx < 0 )                 g_nBendOriginIndex = idx = def->width - 1; }
            else         { if ( idx > def->width - 1 )    g_nBendOriginIndex = idx = 0; }
            cp = &def->ctrl[idx][g_nPatchAxisIndex];
        }
        else
        {
            if ( shift ) { if ( idx < 0 )                 g_nBendOriginIndex = idx = def->height - 1; }
            else         { if ( idx > def->height - 1 )   g_nBendOriginIndex = idx = 0; }
            cp = &def->ctrl[g_nPatchAxisIndex][idx];
        }
        const float *xyz = cp->xyz;
        g_vBendOrigin[0] = xyz[0];
        g_vBendOrigin[1] = xyz[1];
        g_nUpdateBits = -1;
        g_vBendOrigin[2] = xyz[2];
        return;
    }

    if ( g_nPatchBendState == 2 )
        g_bPatchLowerEdge ^= 1;
    g_nUpdateBits = -1;
}

// ── Patch_InsDelHandleTAB (0x447f40) — TAB inside REDISPERSE (ins/del) mode ────
//  Not in redisperse mode -> the TAB simply toggles the mode on.  Otherwise step the
//  axis index by +2 and wrap across the row<->column boundary at height-1 / width-1.
void Patch_InsDelHandleTAB()
{
    extern int g_qeglobals_redispersePatchVerts;            // engine_stubs.cpp (0x25D5A6B)
    if ( !g_qeglobals_redispersePatchVerts )
    {
        Patch_InsDelToggle();
        return;
    }

    patch_t *patch;
    patchMesh_t *def;
    if ( !QE_SingleBrush()
         || ( patch = selected_brushes.next->patch ) == nullptr
         || ( def = patch->def ) == nullptr )
    {
        Patch_BendToggle();                 // idb: yes, BendToggle (not InsDelToggle)
        Sys_Printf( "No patch to bend!" );
        return;
    }

    const int idx = g_nPatchAxisIndex + 2;
    g_nPatchAxisIndex = idx;
    if ( g_bPatchAxisOnRow )
    {
        if ( idx >= def->height - 1 ) { g_bPatchAxisOnRow = 0; g_nPatchAxisIndex = 0; }
    }
    else if ( idx >= def->width - 1 )
    {
        g_bPatchAxisOnRow = 1;
        g_nPatchAxisIndex = 0;
    }
    g_nUpdateBits = -1;
}

// ── Patch_SetBendRotateOrigin (0x447c70) ──────────────────────────────────────
//  Zero the depth-axis component of g_vBendOrigin for the active 2D view, then copy
//  it into g_vRotateOrigin.  Returns the depth-axis index.
static int Patch_SetBendRotateOrigin()
{
    int axis = g_pParentWnd->m_pActiveXY->m_nViewType;
    if ( axis != 2 )
        axis = ( axis != 0 );
    g_vBendOrigin[axis]   = 0.0f;
    g_vRotateOrigin[0]    = g_vBendOrigin[0];
    g_vRotateOrigin[1]    = g_vBendOrigin[1];
    g_vRotateOrigin[2]    = g_vBendOrigin[2];
    return axis;
}

// ── Patch_BendToggle (0x4478e0) — enter/leave bend mode ───────────────────────
//  Toggling OFF clears the mode and refreshes the toolbar.  Toggling ON requires a
//  single selected non-terrain patch; it snapshots the patch (Patch_Save) for cancel,
//  sets the initial axis (row 1) and prompts for the rotation axis.
void Patch_BendToggle()
{
    if ( g_bPatchBendMode )
    {
        g_bPatchBendMode = 0;
        HideInfoDialog();                                 // 0x447909
        CMainFrame_UpdatePatchToolbarButtons();
        return;
    }

    patch_t *patch;
    patchMesh_t *def;
    if ( QE_SingleBrush()
         && ( patch = selected_brushes.next->patch ) != nullptr
         && ( def = patch->def ) != nullptr )
    {
        if ( ( def->type & 0x40 ) != 0 )
        {
            Sys_Printf( "Patch_BendToggle: Bending of terrain curves not implemented\n" );
        }
        else
        {
            Patch_Save( def );
            g_bPatchBendMode  = 1;
            g_nPatchBendState = 0;      // BEND_SELECT_ROTATION
            g_bPatchAxisOnRow = 1;
            g_nPatchAxisIndex = 1;
            ShowInfoDialog( g_pBendStateMsg[0] );   // 0x447979 (BEND_SELECT_ROTATION)
        }
    }
    else
    {
        Sys_Printf( "Must bend a single patch\n" );
    }
}

// ── Patch_BendHandleEnter (0x447b70) — ENTER advances the bend state machine ──
//  0 (pick bend axis) → 1 (pick rotation axis/origin) → 2 (pick which side) → 3 (drag),
//  showing the prompt for the NEW state each step; ENTER at state 3 accepts and leaves
//  bend mode.  Entering state 1 zeroes the origin and calls TAB once to seed the first
//  candidate; entering state 2 arms g_bPatchLowerEdge.
//  Was unwired (CMainFrame::OnClipSelected's bend arm was inert), so bend mode could
//  never progress past its first state — restored 2026-07-31 with ShowInfoDialog.
void Patch_BendHandleEnter()
{
    if ( !g_bPatchBendMode )
        return;
    if ( g_nPatchBendState >= 3 )      // 0x447b7e: last state — ENTER accepts the bend
    {
        Patch_BendToggle();
        g_nUpdateBits = -1;
        return;
    }
    g_nPatchBendState = g_nPatchBendState + 1;
    ShowInfoDialog( g_pBendStateMsg[g_nPatchBendState] );
    if ( g_nPatchBendState == 1 )
    {
        g_nBendOriginIndex = 0;
        g_vBendOrigin[0] = g_vBendOrigin[1] = g_vBendOrigin[2] = 0.0f;
        Patch_BendHandleTAB();         // seed the first rotation-axis candidate
    }
    else if ( g_nPatchBendState == 2 )
    {
        g_bPatchLowerEdge = 1;
    }
    g_nUpdateBits = -1;
}

// ── Patch_Deselect (0x442610) — leave every patch-edit mode ───────────────────
//  Called from Select_Deselect.  Resets the edit mode to sel_brush (closing the point
//  edit that owned the old mode: quad-cycle refreshes the toolbar, addpoint finishes the
//  curve drag), clears the per-instance selected flag on every selected patch, and drops
//  bend / redisperse mode if either is armed.
void Patch_Deselect()
{
    extern int g_qeglobals_redispersePatchVerts;     // engine_stubs.cpp (0x25d5a6b)

    const select_t prevMode = g_qeglobals.d_select_mode;
    g_qeglobals.d_select_mode = sel_brush;
    if ( prevMode == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prevMode == sel_addpoint )
        sub_43ECB0();                                // Patch_FinishCurveDrag

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->patch )
            b->patch->selected = 0;

    if ( g_bPatchBendMode )
        Patch_BendToggle();
    if ( g_qeglobals_redispersePatchVerts )
        Patch_InsDelToggle();
}

// ── Patch_SelectBendAxis (0x447cc0) — choose the rotation axis (a whole row/col) ─
void Patch_SelectBendAxis()
{
    patch_t *patch;
    patchMesh_t *def;
    if ( QE_SingleBrush()
         && ( patch = selected_brushes.next->patch ) != nullptr
         && ( def = patch->def ) != nullptr )
    {
        if ( ( def->type & 0x40 ) != 0 )
        {
            Sys_Printf( "Patch_SelectBendAxis: Bending of terrain curves not implemented\n" );
        }
        else
        {
            if ( g_bPatchAxisOnRow )
                Patch_SelectRow( def, g_nPatchAxisIndex, 0 );
            else
                Patch_SelectColumn( def, g_nPatchAxisIndex, 0 );
            Patch_SetBendRotateOrigin();
        }
    }
    else
    {
        Patch_BendToggle();
    }
}

// ── Patch_SelectBendNormal (0x447d30) — choose the bending half of the patch ───
//  Selects every row/column on the chosen side of the axis (lower edge = indices below
//  the axis; upper edge = indices above), accumulating into the move-point list.
void Patch_SelectBendNormal()
{
    patch_t *patch;
    patchMesh_t *def;
    if ( QE_SingleBrush()
         && ( patch = selected_brushes.next->patch ) != nullptr
         && ( def = patch->def ) != nullptr )
    {
        if ( ( def->type & 0x40 ) != 0 )
        {
            Sys_Printf( "Patch_SelectBendNormal: Bending of terrain curves not implemented\n" );
        }
        else
        {
            g_qeglobals.d_num_move_points = 0;
            if ( g_bPatchAxisOnRow )
            {
                if ( g_bPatchLowerEdge )
                    for ( int i = 0; i < g_nPatchAxisIndex; ++i )
                        Patch_SelectRow( def, i, 1 );
                else
                    for ( int j = def->height - 1; j > g_nPatchAxisIndex; --j )
                        Patch_SelectRow( def, j, 1 );
            }
            else if ( g_bPatchLowerEdge )
            {
                for ( int k = 0; k < g_nPatchAxisIndex; ++k )
                    Patch_SelectColumn( def, k, 1 );
            }
            else
            {
                for ( int m = def->width - 1; m > g_nPatchAxisIndex; --m )
                    Patch_SelectColumn( def, m, 1 );
            }
            Patch_SetBendRotateOrigin();
        }
    }
    else
    {
        Patch_BendToggle();
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  TERRAIN PAINT - the Curve->Paint terrain-height tool's start hook.  Patch_Paint clears
//  each patch's "painting-active" flag (xx22b @0x5042) so the drag can re-mark touched
//  cells; Patch_Paint_Start opens the undo bracket and paints the selection (+ the active
//  list when checkbox 1432 answers, CurvEditDlg_OnSomeSetting).
// ════════════════════════════════════════════════════════════════════════════

// ── Patch_Paint (0x43eb70) — clear the paint-active flag on every patch in a list ─
void Patch_Paint( selbrush_t *list )
{
    for ( selbrush_t *b = list->next; b != list; b = b->next )
    {
        patch_t *patch = b->patch;
        if ( patch )
        {
            iassert( b->patch->def == b->def->patch );  // 3109 — #expr byte-matches embedded string
            patch->def->xx22b = 0;
        }
    }
}

// ── Patch_Paint_Start (0x43ebc0) — begin a terrain-paint stroke ───────────────
//  Opens an undo bracket (closed later by the drag-end hook), clears the paint-active flag
//  on the selection (and the active/unselected list too when the dialog's checkbox 1432
//  answers — CurvEditDlg_OnSomeSetting).  Returns 1.
int Patch_Paint_Start()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "patch painting" );
    if ( CurvEditDlg_OnSomeSetting() )
        Patch_Paint( &active_brushes );
    Patch_Paint( &selected_brushes );
    return 1;
}

// ════════════════════════════════════════════════════════════════════════════
//  ADAPTIVE texCoord RESAMPLER
//  The "set patch sample size" texturing: re-distribute a patch's S/T across an
//  arc-length-ADAPTIVE subdivision so densely-curved regions get proportionally more
//  texture space.  Two outcomes: a near-planar patch projects its texture onto the
//  fitted plane (Curve_FitPlane → Patch_PlanarTexProject); a curved one spreads S/T by
//  the adaptive subdivision's index maps (Curve_AdaptiveSubdivide).  Terrain patches
//  project onto the dominant axis instead (Patch_TerrainTexProject).  All editor-only,
//  display-only (texCoords are recomputed on parse) — no .map round-trip impact.
// ════════════════════════════════════════════════════════════════════════════

extern int Vec3_MajorAxis( const float *dir );   // linearmapping.cpp 0x4A45D0 — dominant-axis index

// ── Curve_LerpVert (sub_433750) — linear-interpolate one curveVert (a→b by t) ─
static void Curve_LerpVert( curveVert_t *dst, const curveVert_t *a, const curveVert_t *b, float t )
{
    const float *fa = (const float *)a;
    const float *fb = (const float *)b;
    float       *fd = (float *)dst;
    // xyz(0..2) + st(3..4) + lightmap(5..6) = the first 7 floats (bytes 0..24).
    for ( int k = 0; k < 7; ++k )
        fd[k] = ( fb[k] - fa[k] ) * t + fa[k];
    // vert_color rgba (bytes 40..43): lerped in double then truncated to a byte.
    const unsigned char *ba = (const unsigned char *)a;
    const unsigned char *bb = (const unsigned char *)b;
    unsigned char       *bd = (unsigned char *)dst;
    for ( int k = 40; k < 44; ++k )
        bd[k] = (unsigned char)(int)( (double)( (int)bb[k] - (int)ba[k] ) * t + (double)ba[k] );
    // normal (floats 7..9, bytes 28..36), then renormalize.
    fd[7] = ( fb[7] - fa[7] ) * t + fa[7];
    fd[8] = ( fb[8] - fa[8] ) * t + fa[8];
    fd[9] = t * ( fb[9] - fa[9] ) + fa[9];
    Vec3Normalize_R( &fd[7] );
}

// ── Curve_AdaptiveSubdivide (sub_4338E0) — arc-length adaptive re-subdivision ──
//  Subdivide each column gap (then each row gap) by max-edge-length / sampleSize,
//  capped so the result never exceeds the 512×512 scratch.  Fills colMap/rowMap with,
//  for each refined index, the original control index it came from (so the resampler can
//  map control points → refined positions).  Ported at the algorithm level (clean 2-D
//  indexing into s_curveScratch); numerically identical to the binary's byte-shuffling.
static curvePatchDef_t *Curve_AdaptiveSubdivide( const curvePatchDef_t *in, float sampleSize,
                                                 int *colCounts, int *rowCounts,
                                                 int *colMap, int *rowMap )
{
    const int origW = in->width;
    const int origH = in->height;

    int W = origW, H = origH;
    for ( int col = 0; col < origW; ++col )
        for ( int row = 0; row < origH; ++row )
            s_curveScratch[row][col] = in->verts[col + row * origW];

    // PHASE 1 — subdivide columns (max edge length is taken down each column gap).
    const int capCol = ( origW > 1 ) ? ( 512 - origW ) / ( origW - 1 ) : 0;
    int outCol = 0;
    for ( int gap = 0; gap < origW - 1; ++gap )
    {
        float maxLen = 0.0f;
        for ( int row = 0; row < H; ++row )
        {
            const float *a = s_curveScratch[row][outCol].xyz;
            const float *b = s_curveScratch[row][outCol + 1].xyz;
            float dx = b[0] - a[0], dy = b[1] - a[1], dz = b[2] - a[2];
            float len = (float)sqrt( dx * dx + dy * dy + dz * dz );
            if ( maxLen < len ) maxLen = len;
        }
        int n = (int)( maxLen / sampleSize );
        if ( n > capCol ) n = capCol;
        colCounts[gap] = n + 1;
        if ( n > 0 )
        {
            W += n;
            if ( colMap )
            {
                for ( int q = W - 1; q >= n + outCol; --q ) colMap[q] = colMap[q - n];
                for ( int q = 0; q < n; ++q )               colMap[outCol + 1 + q] = colMap[outCol];
            }
            for ( int row = 0; row < H; ++row )
            {
                for ( int q = W - 1; q > outCol + n; --q )
                    s_curveScratch[row][q] = s_curveScratch[row][q - n];
                for ( int s = 1; s <= n; ++s )
                {
                    float t = (float)( (double)s / (double)( n + 1 ) );
                    Curve_LerpVert( &s_curveScratch[row][outCol + s],
                                    &s_curveScratch[row][outCol],
                                    &s_curveScratch[row][outCol + n + 1], t );
                }
            }
        }
        outCol += n + 1;
    }

    // PHASE 2 — subdivide rows (max edge length is taken across each row gap).
    const int capRow = ( origH > 1 ) ? ( 512 - origH ) / ( origH - 1 ) : 0;
    int outRow = 0;
    for ( int gap = 0; gap < origH - 1; ++gap )
    {
        float maxLen = 0.0f;
        for ( int col = 0; col < W; ++col )
        {
            const float *a = s_curveScratch[outRow][col].xyz;
            const float *b = s_curveScratch[outRow + 1][col].xyz;
            float dx = b[0] - a[0], dy = b[1] - a[1], dz = b[2] - a[2];
            float len = (float)sqrt( dx * dx + dy * dy + dz * dz );
            if ( maxLen < len ) maxLen = len;
        }
        int n = (int)( maxLen / sampleSize );
        if ( n > capRow ) n = capRow;
        rowCounts[gap] = n + 1;
        if ( n > 0 )
        {
            H += n;
            if ( rowMap )
            {
                for ( int q = H - 1; q >= n + outRow; --q ) rowMap[q] = rowMap[q - n];
                for ( int q = 0; q < n; ++q )               rowMap[outRow + 1 + q] = rowMap[outRow];
            }
            for ( int col = 0; col < W; ++col )
            {
                for ( int q = H - 1; q > outRow + n; --q )
                    s_curveScratch[q][col] = s_curveScratch[q - n][col];
                for ( int s = 1; s <= n; ++s )
                {
                    float t = (float)( (double)s / (double)( n + 1 ) );
                    Curve_LerpVert( &s_curveScratch[outRow + s][col],
                                    &s_curveScratch[outRow][col],
                                    &s_curveScratch[outRow + n + 1][col], t );
                }
            }
        }
        outRow += n + 1;
    }

    size_t bytes = (size_t)44 * W * H;
    curvePatchDef_t *out = (curvePatchDef_t *)malloc( bytes + 20 );
    out->width  = W;
    out->height = H;
    out->verts  = (curveVert_t *)( out + 1 );
    for ( int row = 0; row < H; ++row )
        for ( int col = 0; col < W; ++col )
            out->verts[col + row * W] = s_curveScratch[row][col];
    return out;
}

// ── Curve_FitPlane (sub_432690) — is the tessellated mesh planar (±1)? ─────────
//  Sum the per-cell triangle normals + corner positions; if the summed normal is
//  non-degenerate AND every vertex lies within ±1 of the fitted plane, return 1 + the
//  plane (normal in out[0..2], dist in out[3]).  Else 0.
static int Curve_FitPlane( const curvePatchDef_t *cd, float *out )
{
    const int w = cd->width;
    const curveVert_t *V = cd->verts;
    float nrm[3] = { 0, 0, 0 };          // v50/v51/v52 summed normal
    float pos[3] = { 0, 0, 0 };          // v47/v48/v49 summed corner
    int   count  = 0;                    // v54

    for ( int row = 0; row < cd->height - 1; ++row )
    {
        for ( int col = 0; col < w - 1; ++col )
        {
            const float *v00 = V[col + row * w].xyz;
            const float *v10 = V[col + 1 + row * w].xyz;
            const float *v01 = V[col + ( row + 1 ) * w].xyz;
            const float *v11 = V[col + 1 + ( row + 1 ) * w].xyz;

            // T1: edge1 = v10-v00, edge2 = v01-v00 ; accumulate v00.
            float e1[3] = { v10[0] - v00[0], v10[1] - v00[1], v10[2] - v00[2] };
            float e2[3] = { v01[0] - v00[0], v01[1] - v00[1], v01[2] - v00[2] };
            float cr[3];
            Vec3Cross( e1, e2, cr );
            if ( Vec3Normalize_R( cr ) > 0.001f )
            {
                nrm[0] += cr[0]; nrm[1] += cr[1]; nrm[2] += cr[2];
                pos[0] += v00[0]; pos[1] += v00[1]; pos[2] += v00[2];
                ++count;
            }
            // T2: edge1 = v01-v11, edge2 = v10-v11 ; accumulate v11.
            float f1[3] = { v01[0] - v11[0], v01[1] - v11[1], v01[2] - v11[2] };
            float f2[3] = { v10[0] - v11[0], v10[1] - v11[1], v10[2] - v11[2] };
            Vec3Cross( f1, f2, cr );
            if ( Vec3Normalize_R( cr ) > 0.001f )
            {
                nrm[0] += cr[0]; nrm[1] += cr[1]; nrm[2] += cr[2];
                pos[0] += v11[0]; pos[1] += v11[1]; pos[2] += v11[2];
                ++count;
            }
        }
    }

    if ( Vec3Normalize_R( nrm ) < 0.001f )
        return 0;
    float dist = ( pos[2] * nrm[2] + pos[0] * nrm[0] + pos[1] * nrm[1] ) / (float)count;

    for ( int col = 0; col < w - 1; ++col )
        for ( int row = 0; row < cd->height - 1; ++row )
        {
            const float *v = V[col + row * w].xyz;
            float d = v[1] * nrm[1] + v[0] * nrm[0] + v[2] * nrm[2] - dist;
            if ( d > 1.0f || d < -1.0f )
                return 0;
        }

    if ( out )
    {
        out[0] = nrm[0]; out[1] = nrm[1]; out[2] = nrm[2]; out[3] = dist;
    }
    return 1;
}

// ── Patch_PlanarTexProject (sub_4392e0) — project S/T onto a fitted plane ──────
static curvePatchDef_t *Patch_PlanarTexProject( patchMesh_t *p, int layer, float sampleSize,
                                                const float *planeNormal )
{
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)( &p->texture + layer ) );
    int width  = lm ? lm->width  : 512;
    int height = lm ? lm->height : 512;
    float params[6];
    params[0] = (float)width  * sampleSize;
    params[1] = sampleSize * (float)height;
    params[2] = params[3] = params[4] = params[5] = 0.0f;
    Patch_ST( params, planeNormal, p, layer );
    return p->curveDef;
}

// ── Patch_TerrainTexProject (sub_439350) — project a terrain patch's S/T onto the
//    dominant axis (no adaptive subdivision — terrain is its own control grid) ────
static curvePatchDef_t *Patch_TerrainTexProject( patchMesh_t *p, int layer, float sampleSize )
{
    if ( !p->curveDef )
        p->curveDef = Patch_GenericMesh2( p, layer, 0, 0 );
    curvePatchDef_t *cd = p->curveDef;
    curveVert_t     *V  = cd->verts;

    float summed[3] = { 0, 0, 0 };
    for ( int y = 0; y < cd->height - 1; ++y )
    {
        for ( int i = 0; i < cd->width - 1; ++i )
        {
            int pt = i + y * cd->width;
            const float *base, *eA, *eB, *eC;     // v12, v10, v11, v13
            if ( p->ctrl[i][y].turned_edge & 1 )
            {
                base = V[pt + cd->width + 1].xyz;  // (i+1,y+1)  v12
                eA   = V[pt + cd->width].xyz;      // (i,  y+1)  v10
                eB   = V[pt].xyz;                  // (i,  y)    v11
                eC   = V[pt + 1].xyz;              // (i+1,y)    v13
            }
            else
            {
                base = V[pt + 1].xyz;              // (i+1,y)    v12
                eA   = V[pt + cd->width + 1].xyz;  // (i+1,y+1)  v10
                eB   = V[pt + cd->width].xyz;      // (i,  y+1)  v11
                eC   = V[pt].xyz;                  // (i,  y)    v13
            }
            float a[3] = { eA[0] - base[0], eA[1] - base[1], eA[2] - base[2] };   // v21
            float b[3] = { eB[0] - base[0], eB[1] - base[1], eB[2] - base[2] };   // vec_a
            float c[3] = { eC[0] - base[0], eC[1] - base[1], eC[2] - base[2] };   // v26
            float cr[3];
            Vec3Cross( b, a, cr );
            summed[0] += cr[0]; summed[1] += cr[1]; summed[2] += cr[2];
            Vec3Cross( c, b, cr );
            summed[0] += cr[0]; summed[1] += cr[1]; summed[2] += cr[2];
        }
    }

    float planeNormal[3] = { 0, 0, 0 };
    planeNormal[ Vec3_MajorAxis( summed ) ] = 1.0f; // axis with the largest |component|

    qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)( &p->texture + layer ) );
    int width  = lm ? lm->width  : 512;
    int height = lm ? lm->height : 512;
    float params[6];
    params[0] = (float)width  * sampleSize;
    params[1] = sampleSize * (float)height;
    params[2] = params[3] = params[4] = params[5] = 0.0f;
    Patch_ST( params, planeNormal, p, layer );
    return p->curveDef;
}

// ── PMESH_02 (0x439580) — the curve (non-terrain) sample-size resampler ───────
//  Build the tessellated mesh (with col/row index maps), adaptively re-subdivide it, and
//  EITHER project onto the fitted plane (planar) OR spread each control point's S/T to the
//  normalized position (refinedIndex+0.5)/512 of its mapped refined vertex.
static void PMESH_02( patchMesh_t *p, int layer, float sampleSize )
{
    // IDA: the layer arrives in ecx (a __usercall arg) — NOT g_qeglobals.current_edit_layer.
    // sub_442B00 passes 1 (the lightmap layer); the parked Patch_Rebuild/BrushToMesh refinement
    // callers would pass their own layer.
    static int colCounts[512], rowCounts[512], colMap[512], rowMap[512];

    curvePatchDef_t *base = Patch_GenericMesh2( p, layer,
                                                (int)(intptr_t)colMap, (int)(intptr_t)rowMap );
    curvePatchDef_t *refined = Curve_AdaptiveSubdivide( base, sampleSize,
                                                        colCounts, rowCounts, colMap, rowMap );
    free( base );

    float plane[4];
    if ( Curve_FitPlane( refined, plane ) )
    {
        free( refined );
        Patch_PlanarTexProject( p, layer, sampleSize, plane );
        return;
    }

    const int rw = refined->width;
    const int rh = refined->height;
    for ( int col = 0; col < p->width; ++col )
    {
        int j = 0;
        if ( rw >= 2 )
            for ( ; j < rw - 1; ++j ) if ( colMap[j] >= col ) break;
        float s = (float)( ( (double)j + 0.5 ) * 0.001953125 );   // (j+0.5)/512
        if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
            Assert( PMESH_CPP, 927, 0, "%s", "!IS_NAN(s)" );

        for ( int row = 0; row < p->height; ++row )
        {
            int k = 0;
            if ( rh >= 2 )
                for ( ; k < rh - 1; ++k ) if ( rowMap[k] >= row ) break;
            float t = (float)( ( (double)k + 0.5 ) * 0.001953125 );
            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 929, 0, "%s", "!IS_NAN(t)" );

            float *st = &p->ctrl[col][row].texCoord.st[2 * layer];
            st[0] = s;
            st[1] = t;
        }
    }
    free( refined );
}

// ── sub_442B00 (0x442b00) — set a patch's texture SAMPLE SIZE ──────────────────
//  Reached from Brush_SetSampleSize (sub_477080) when a selected brush owns a patch —
//  the Surface Inspector "sample size" field with a patch selected.  Re-sample the patch
//  texCoords at the new density (terrain → dominant-axis projection; curves → adaptive
//  spread / planar projection), store the size + mark bDirty, and (layer 1) rebuild the
//  render curveDef.
void sub_442B00( patchMesh_t *p, int sampleSize )
{
    const float sz = (float)sampleSize;

    // IDA hardcodes layer 1 (the lightmap) for BOTH branches (push 1 / mov ecx,1) — the resample
    // always targets layer 1; only the curveDef REBUILD below is gated on current_edit_layer==1.
    if ( ( p->type & PATCH_TERRAIN ) != 0 )
        Patch_TerrainTexProject( p, 1, sz );
    else
        PMESH_02( p, 1, sz );

    p->bDirty = 1;
    *(float *)&p->size_of_struct_0x504C = sz;

    if ( g_qeglobals.current_edit_layer == 1 )
    {
        if ( p->curveDef )
            free( p->curveDef );
        p->curveDef = Patch_GenericMesh2( p, g_qeglobals.current_edit_layer, 0, 0 );
        ++p->version;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x44cde0  Patch_Subdivide  (PMESH.CPP:8996)
// Steps every selected non-terrain patch's tessellation level (subDivType) by
// `decrease` (negative coarsens, positive refines), rebuilding curveDef until its
// vertex count actually changes or it clamps at subDivType 1.  When refining, only
// acts on patches whose tessellated grid already exceeds the control grid.  Called
// from CMainFrame::OnOverBrightShift{Up,Down}.  (IDA is __usercall with decrease in
// ebx; normalised to __cdecl here.)
// ─────────────────────────────────────────────────────────────────────────────
void Patch_Subdivide( int decrease )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        patch_t *patch = b->patch;
        if ( !patch || ( patch->def->type & PATCH_TERRAIN ) != 0 )
            continue;

        iassert( b->patch->def == b->def->patch );

        patchMesh_t *def = b->patch->def;
        if ( decrease < 0 && def->subDivType == 1 )
            continue;

        if ( decrease <= 0
             || def->curveDef->width  > def->width
             || def->curveDef->height > def->height )
        {
            int origCount = def->curveDef->width * def->curveDef->height;
            def->subDivType += decrease;
            if ( def->subDivType < 1 )
            {
                def->subDivType = 1;
            }
            else
            {
                while ( 1 )
                {
                    if ( def->curveDef )
                        free( def->curveDef );
                    curvePatchDef_t *newDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
                    ++def->version;
                    def->curveDef = newDef;
                    if ( newDef->width * newDef->height != origCount )
                        break;
                    def->subDivType += decrease;
                    if ( def->subDivType < 1 )
                    {
                        def->subDivType = 1;
                        break;
                    }
                }
            }
        }
    }
    g_nUpdateBits = -1;
}

// ════════════════════════════════════════════════════════════════════════════
//  VERT-SNAP COLLECTOR SUBSYSTEM (the V-key mixed patch+brush vert-edit entry).
//  The 0x442xxx cluster (PMESH.CPP per the assert strings), reached via the V key
//  (cmd 33005, CMainFrame::OnSelectionDragVertices third arm) with a mixed patch+brush
//  selection.  Clears the patch_verts_array01 vert pool, then walks the selected brush
//  list and, for each brush/patch/prefab/misc_model whose vert-snap PREF is enabled,
//  collects its world-space vertices into patch_verts_array01 (dedup'd within 0.1u).
//  That pool is the snap target the subsequent curve-point drag snaps to.  All three
//  vert-snap prefs (m_bVertSnapModel/Brush/Prefab) DEFAULT OFF.
//
//  STORAGE LAYOUT (the IDB-base arithmetic, decoded byte-exact):
//    The binary stores each collected vert as a vec3 into a flat float array whose
//    base is &g_qeglobals.unkown_pmesh_float1 (abs 0x2603A30): element N is at
//    [3*N+0/1/2].  The dedup scan (sub_43C240) reads from g_qeglobals.
//    patch_verts_array01 (abs 0x2603A38 = unkown_pmesh_float1 + 8 bytes = +2 floats)
//    with per-candidate offsets x=ptr-8, y=ptr-4, z=ptr — i.e. element i's x lands at
//    patch_verts_array01 + 12*i - 8 == unkown_pmesh_float1 + 12*i.  So the store base
//    and the read base address the SAME vec3 array, 12-byte stride; the two qeglobals
//    members (unkown_pmesh_float1 @0x10070, patch_verts_array01 @0x10078) deliberately
//    overlap with the 8-byte (unkown_pmesh_float1 + pad_01) header offset between them.
//    Both views resolve to VertSnapPool() below — byte-identical to the binary.
// ════════════════════════════════════════════════════════════════════════════

// brush.cpp (0x478780) — load/realize the brush's model + copy the orientation matrix
// into the entity instance; returns true if the brush carries a (misc_)model.
extern bool      Model_SetModel( entity_brush_s *b, int orientMatrix );
// entity.cpp (0x482A70) — orParent ∘ ent's own orientation → orOut.
extern void      Entity_GetOrientation( entity_s_def *ent, orientation_t *orParent, orientation_t *orOut );
// CMainFrame_UpdatePatchToolbarButtons() already extern'd above (select.cpp).
// entity.cpp (0x6DE290) — world-space identity orientation (origin 0, axis = I).
extern float     world_orient_matrix[4][3];

// The single flat vec3 vert pool both the collectors (store) and sub_43C240 (read)
// address.  Base = &g_qeglobals.unkown_pmesh_float1 (the store base in the disasm:
//   fstp dword ptr [eax*4 + 0x2603A30]  with eax = 3*count).
static inline float *VertSnapPool() { return (float *)&g_qeglobals.unkown_pmesh_float1; }

// ── sub_43C240 (0x43c240) — dedup scan ───────────────────────────────────────
//  Returns the index of the first pooled vert within sqrt(0.01)=0.1u of `cand`,
//  else -1.  The binary reads from patch_verts_array01 (= VertSnapPool()+2 floats)
//  with offsets -8/-4/0; that resolves to VertSnapPool()[3*i + 0/1/2] (see the
//  layout note above).  count<=0 → -1 (faithful: the loop's pre-test).
static signed int VertSnap_FindDup( const float *cand )
{
    if ( g_qeglobals.patch_verts_array01_count <= 0 )
        return -1;

    const float *pool = VertSnapPool();
    int i = 0;
    for ( ;; i += 1 )
    {
        float dx = pool[3 * i + 0] - cand[0];
        float dy = pool[3 * i + 1] - cand[1];
        float dz = pool[3 * i + 2] - cand[2];
        if ( dx * dx + dy * dy + dz * dz < 0.01 )   // IDA: compare vs double 0.01
            return i;
        if ( i + 1 >= g_qeglobals.patch_verts_array01_count )
            return -1;
    }
}

// Store one (already world-space) vert into the pool if it isn't a duplicate; returns
// true once the pool fills (0x4000 = 16384 verts).  Common tail of all three collectors.
static inline bool VertSnap_Store( const float *p )
{
    if ( VertSnap_FindDup( p ) >= 0 )
        return false;
    float *pool = VertSnapPool();
    int n = g_qeglobals.patch_verts_array01_count;
    pool[3 * n + 0] = p[0];
    pool[3 * n + 1] = p[1];
    pool[3 * n + 2] = p[2];
    return ( ++g_qeglobals.patch_verts_array01_count == 0x4000 );
}

// ── sub_442520 (0x442520) — brush winding vert collector ──────────────────────
//  For each face of the brush DEF, world-transform (through `orient`) each winding
//  point and dedup-store it into the pool.  Face stride 232 (face_t), winding @+0xE0
//  (face_t.w), points start at winding+4 with 12-byte stride (winding_t.p[]).
static void VertSnap_CollectBrush( const orientation_t *orient, selbrush_t *b )
{
    brush_t *def = b->def;
    if ( !def->faceCount )
        return;

    for ( int fi = 0; fi < def->faceCount; ++fi )
    {
        winding_t *w = def->faces[fi].w;
        if ( !w || w->numpoints == 0 )
            continue;

        const float *pts = (const float *)&w->p[0][0];     // point j = pts[3*j..3*j+2]
        for ( int j = 0; j < w->numpoints; ++j )
        {
            float world[3];
            OrientationPosToWorldPos( world, pts + 3 * j, orient );
            if ( VertSnap_Store( world ) )
                return;                                     // pool full
        }
    }
}

// ── PMESH_31 (0x4422b0) — patch control-grid vert collector ───────────────────
//  Walk a patch's width×height control grid, world-transform each control point and
//  dedup-store it.  ctrl is drawVert_t[16][16] @ patchMesh_t+0x38 (row stride 1280,
//  point stride 80, xyz @+0).  The binary INLINES OrientationPosToWorldPos (and its
//  q_shared.cpp:1599 `pos != out` assert); we call the real function, which carries
//  that same assert — behaviour-identical.  Preserves the PMESH.CPP:4427
//  `b->patch->def == b->def->patch` instance/def-consistency assert.
static void VertSnap_CollectPatch( selbrush_t *b, const orientation_t *orient )
{
    if ( b->patch->def != b->def->patch )
        Assert( PMESH_CPP, 4427, 0, "%s", "b->patch->def == b->def->patch" );

    patchMesh_t *def = b->patch->def;
    for ( int row = 0; row < def->width; ++row )
    {
        for ( int col = 0; col < def->height; ++col )
        {
            float world[3];
            OrientationPosToWorldPos( world, def->ctrl[row][col].xyz, orient );
            if ( VertSnap_Store( world ) )
                return;                                     // pool full
        }
    }
}

// ── sub_442410 (0x442410) — misc_model XModel vert collector ──────────────────
//  Extract the misc_model's LOD-0 geometry (Editor_ExtractXModelGeo = IDB sub_4FEBB0,
//  now ported in gfx_d3d/r_xsurface.cpp), then world-transform each INDEXED vertex
//  through `orient` and dedup-store it into the snap pool (IDB: sub_4FEBB0 → per-index
//  OrientationPosToWorldPos → sub_43C240 dedup → store).  XModel handle chain matches the
//  binary: b->owner->def->modelClass->model->handle (entity_s.def@0x08 / modelClass@0x64 /
//  entitymodel_t.model@0x160 / models_t.handle@+4).
struct XModel;
extern int __cdecl Editor_ExtractXModelGeo( XModel *model, float *verts, int vertLimit,
                                            uint16_t *indices, int indexLimit );  // gfx_d3d/r_xsurface.cpp (0x4FEBB0)
static void VertSnap_CollectModel( selbrush_t *b, const orientation_t *orient )
{
    entity_s_def  *def   = (entity_s_def *)b->owner->def;
    entitymodel_t *mc    = (entitymodel_t *)def->modelClass;
    models_t      *model = mc->model;
    XModel        *xmodel = (XModel *)(intptr_t)model->handle;

    // IDB stack buffers v6[196612] (16384 vec3 positions) + v10[65538] (65536 indices);
    // a function-local static here (editor is single-threaded + non-reentrant, like DrawGeo's
    // s_lineVerts) avoids a ~327 KB stack frame.
    static float    s_modelVerts[0x4000 * 3];   // 16384 positions
    static uint16_t s_modelIndices[0x10000];    // 65536 indices

    int indexCount = Editor_ExtractXModelGeo( xmodel, s_modelVerts, 0x4000,
                                              s_modelIndices, 0x10000 );
    for ( int i = 0; i < indexCount; ++i )
    {
        float world[3];
        OrientationPosToWorldPos( world, &s_modelVerts[3 * s_modelIndices[i]], orient );
        if ( VertSnap_Store( world ) )
            return;                              // pool full
    }
}

// ── VertSnapTo_ModelBrushPrefab (0x442160) — recursive snap walker ─────────────
//  Walk a brush instance list (top call: &selected_brushes; prefab recursion: the
//  prefab's active_brushlist).  Per brush, dispatch by its owning entity's eclass:
//    * fixedsize entity:
//        - prefab (classtype & 0x10) + m_bVertSnapPrefab → recurse into the prefab's
//          active_brushlist with the composed orientation;
//        - else if Model_SetModel && eclass=="misc_model" + m_bVertSnapModel →
//          collect the model's verts (VertSnap_CollectModel, deferred no-op).
//    * non-fixedsize (brush/patch) + m_bVertSnapBrush:
//        - patch brush: collect its control grid (VertSnap_CollectPatch) — but ONLY
//          when this is NOT the top-level selected list (listHead != &selected_brushes),
//          i.e. only patches reached via a prefab recursion are snapped to (a selected
//          patch is the one being EDITED, not a snap target);
//        - solid brush: collect its winding verts (VertSnap_CollectBrush).
//  Stops once the pool reaches 0x4000.  The IDB signature is __usercall with an `eax`
//  return that is merely the loop's scratch entity pointer; normalised to a clean
//  void(listHead, orient) here (the return value is never consumed by the caller).
static void VertSnapTo_ModelBrushPrefab( selbrush_t *listHead, orientation_t *orient )
{
    selbrush_t *next = listHead->next;
    if ( next == listHead )
        return;

    while ( g_qeglobals.patch_verts_array01_count < 0x4000 )
    {
        selbrush_t   *cur = next;                          // the IDB carries cur (v7) + next
        entity_s_def *ownerDef = (entity_s_def *)cur->owner->def;
        eclass_t     *eclass   = ownerDef->eclass;

        if ( *(int *)&eclass->fixedsize )
        {
            if ( ( eclass->classtype & 0x10 ) != 0 )       // prefab entity
            {
                if ( g_PrefsDlg->m_bVertSnapPrefab )
                {
                    orientation_t composed;
                    Entity_GetOrientation( ownerDef, orient, &composed );
                    // &prefab->active_brushlist (selbrush_t sentinel @ prefab_s+0x0C).
                    selbrush_t *prefabList =
                        (selbrush_t *)( (char *)cur->owner->prefab + 0x0C );
                    VertSnapTo_ModelBrushPrefab( prefabList, &composed );
                }
            }
            else                                           // model entity (misc_model etc.)
            {
                if ( Model_SetModel( cur, (int)(intptr_t)orient ) )
                {
                    entity_s_def *ed = (entity_s_def *)cur->owner->def;
                    if ( !strcmp( ed->eclass->name, "misc_model" ) )
                    {
                        if ( g_PrefsDlg->m_bVertSnapModel )
                        {
                            orientation_t composed;
                            Entity_GetOrientation( ed, orient, &composed );
                            VertSnap_CollectModel( cur, &composed );
                        }
                    }
                }
            }
        }
        else if ( g_PrefsDlg->m_bVertSnapBrush )           // brush / patch geometry
        {
            if ( cur->patch )
            {
                if ( listHead != &selected_brushes )       // only snap to prefab patches
                    VertSnap_CollectPatch( cur, orient );
            }
            else
            {
                VertSnap_CollectBrush( orient, cur );
            }
        }

        next = cur->next;
        if ( next == listHead )
            break;
    }
}

// ── Terrain_Edit (0x442100) — MISNAMED in the IDB (it is NOT a terrain editor) ─
//  The V-key vert-edit entry for a MIXED patch+brush selection (the third arm of
//  CMainFrame::OnSelectionDragVertices, cmd 33005).  Clears the point/move-point/vert
//  pools, runs the vert-snap collector over the selection (through the identity world
//  orientation), switches to curve-point edit mode, and runs the mode-transition tail
//  (sel_cycle_edge_direction_quad → refresh patch toolbar; sel_addpoint → finish the
//  curve drag).  Was a FATAL stub in engine_stubs.cpp — now real.
void Terrain_Edit()
{
    g_qeglobals.d_numpoints              = 0;
    g_qeglobals.d_num_move_points        = 0;
    g_qeglobals.patch_verts_array01_count = 0;
    g_qeglobals.patch_verts_array02_count = 0;

    VertSnapTo_ModelBrushPrefab( &selected_brushes,
                                 (orientation_t *)world_orient_matrix );

    select_t prevMode = g_qeglobals.d_select_mode;
    g_qeglobals.d_select_mode = sel_curvepoint;
    if ( prevMode == sel_cycle_edge_direction_quad )
        CMainFrame_UpdatePatchToolbarButtons();
    else if ( prevMode == sel_addpoint )
        sub_43ECB0();                                      // Patch_FinishCurveDrag (0x43ecb0)
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x441AD0  PMESH_29_Winding — patch shadow-caster winding submit (the patch
//  analogue of Brush_DrawSubmitFaceWindings).  Realizes the patch material; if it
//  isn't a shadow-casting drawn material it returns.  Then walks the patch's
//  triangulated index list, building per-triangle (or per-coplanar-quad) world
//  windings and feeding them to the light-region CSG (sub_4DAA70).
//
//  Faithful to disasm 0x441AD0.  The patch instance/def fields are accessed at the
//  binary's raw offsets (the kisak patch instance/def structs are opaque here):
//    inst+0  = def (patchMesh_t*)      inst+4  = version (u16)
//    inst+12 = indexCount              inst+16 = indices (u16[])
//    inst+24 = mtldef (MaterialDef)    def+0x5040 = def version (u16)
//    def+0x5038 → [+0xC] = vertex array base (stride 0x2C = 44, xyz at +0)
// ═════════════════════════════════════════════════════════════════════════════
#include "primarylights_region.h"
extern void   MaterialDef_02( MaterialDef *m, int (*cb)( qtexture_s * ) );  // 0x431520
extern int    MaterialDef_06( qtexture_s *radMtl );                         // 0x4318A0
extern int    dword_181F51C;                                               // realize-state
extern int    g_windingAlloc;
extern void   Com_PrintMessage( const char *fmt, ... );
extern void   Assert( const char *file, int line, int type, const char *fmt, ... );
extern float  Vec3Normalize_R( float *v );
extern bool   Material_CastsStencilShadow( Material *m );                   // 0x4FEE90
extern void   OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient ); // 0x4BA430

// sub_40A4A0 (0x40A4A0) — vec3 dot.
static float Patch_Vec3Dot( const float *a, const float *b )
{
    return (float)( a[1] * b[1] + a[0] * b[0] + a[2] * b[2] );
}

// sub_441930 (0x441930) — edge-clip plane: plane normal = cross(faceNormal, a1-a3),
// normalized; plane.dist = normal·a3.  (a1/a3 = two edge verts, a2 = face plane.)
static void Patch_EdgePlane( const float *a1, const float *a2, const float *a3, float *out )
{
    float edge[3];
    edge[0] = a1[0] - a3[0];
    edge[1] = a1[1] - a3[1];
    edge[2] = a1[2] - a3[2];
    Vec3Cross( a2, edge, out );
    Vec3Normalize_R( out );
    out[3] = out[1] * a3[1] + a3[0] * out[0] + out[2] * a3[2];
}

// sub_441990 (0x441990) — true if the two triangles {p0,p1,p2} and {p2,p3,p0} of a
// patch quad are coplanar AND convex enough to emit as a single quad.  a1 points at
// { float *p[6]; float *refOrient; } (the var_38.. block from PMESH_29).
struct PatchQuad { const float *p0, *p1, *p2, *p3, *p4, *p5; };
static char Patch_QuadIsConvex( const PatchQuad *q )
{
    // disasm: requires p2 (a1+8) == p3 (a1+12) and p1 (a1+4) == p4 (a1+16) (degenerate
    // shared edges), then plane test of the four corners against a reference.
    if ( q->p2 != q->p3 )
        return 0;
    if ( q->p1 != q->p4 )
        return 0;
    float plane[4];
    if ( !PlaneFromPoints_Real( plane, (float *)q->p1, (float *)q->p0, (float *)q->p2 ) )
        return 0;
    const float *ref = q->p5;
    float d = ref[1] * plane[1] + ref[0] * plane[0] + ref[2] * plane[2] - plane[3];
    if ( I_fabs( d ) >= 0.02500000037252903f )
        return 0;
    float e[4];
    Patch_EdgePlane( q->p2, plane, q->p1, e );
    float d2 = ref[1] * e[1] + ref[0] * e[0] + ref[2] * e[2];
    if ( d2 - e[3] < -0.02500000037252903f )
        return 0;
    Patch_EdgePlane( q->p0, plane, q->p2, e );
    if ( Patch_Vec3Dot( e, ref ) - e[3] > 0.02500000037252903f )
        return 0;
    Patch_EdgePlane( q->p1, plane, q->p0, e );
    if ( Patch_Vec3Dot( e, ref ) - e[3] > 0.02500000037252903f )
        return 0;
    return 1;
}

void PMESH_29_Winding( int patchInst, const orientation_t *orient, float *desc, rface_t **outList )
{
    patch_t *pm = (patch_t *)(intptr_t)patchInst;             // the binary's local `pm`
    MaterialDef *md = (MaterialDef *)&pm->def->texture;       // def+24 = mtldef

    dword_181F51C = 0x2000;
    MaterialDef_02( md, MaterialDef_06 );
    int exterior;
    if ( dword_181F51C )
    {
        exterior = 0;   // a drawn material (the realize survivor cleared) → interior
    }
    else
    {
        // not drawn → must be a stencil-shadow layered material to submit at all
        // (the binary inlines MaterialDef_10_LayeredMatHandle here; it carries MaterialDef.cpp:246)
        extern bool MaterialDef_10_LayeredMatHandle( MaterialDef *mtlDef );
        if ( !MaterialDef_10_LayeredMatHandle( md ) )
            return;
        exterior = 1;
    }

    iassert( pm->version == pm->def->version );   // PMESH.CPP:4236

    ++g_windingAlloc;
    winding_t *w = (winding_t *)malloc( 0x34u );
    if ( !w )
        Com_PrintMessage( "out of memory: winding_t\n" );
    w->numpoints = 0;

    int indexIter = 0;
    if ( pm->indexCount >= 6 )
    {
        for ( ; ; )
        {
            const unsigned short *indices = pm->indicesFront;
            const char *verts = (const char *)pm->def->curveDef->verts;       // GfxWorldVertex base
            #define VPTR(n) (const float *)( verts + 44 * indices[indexIter + (n)] )
            const float *p0 = VPTR(0), *p1 = VPTR(1), *p2 = VPTR(2);
            const float *p3 = VPTR(3), *p4 = VPTR(4), *p5 = VPTR(5);
            #undef VPTR
            PatchQuad q = { p0, p1, p2, p3, p4, p5 };
            if ( Patch_QuadIsConvex( &q ) )
            {
                // quad winding = {i0, i1, i5, i2} (disasm: w[3]=var_30=i2; i2==i3 here).
                w->numpoints = 4;
                OrientationPosToWorldPos( w->p[0], q.p0, orient );
                OrientationPosToWorldPos( w->p[1], q.p1, orient );
                OrientationPosToWorldPos( w->p[2], q.p5, orient );
                OrientationPosToWorldPos( w->p[3], q.p2, orient );
                sub_4DAA70( w, outList, desc, exterior );
            }
            else
            {
                w->numpoints = 3;
                OrientationPosToWorldPos( w->p[0], q.p0, orient );
                OrientationPosToWorldPos( w->p[1], q.p1, orient );
                OrientationPosToWorldPos( w->p[2], q.p2, orient );
                sub_4DAA70( w, outList, desc, exterior );
                OrientationPosToWorldPos( w->p[0], q.p3, orient );
                OrientationPosToWorldPos( w->p[1], q.p4, orient );
                OrientationPosToWorldPos( w->p[2], q.p5, orient );
                sub_4DAA70( w, outList, desc, exterior );
            }
            indexIter += 6;
            if ( indexIter + 6 > pm->indexCount )
                break;
        }
    }

    // tail triangle (indexCount not a multiple of 6)
    if ( indexIter != pm->indexCount )
    {
        vassert( indexIter + 3 == pm->indexCount, "%i, %i", indexIter + 3, pm->indexCount );   // PMESH.CPP:4274
        const unsigned short *indices = pm->indicesFront;
        const char *verts = (const char *)pm->def->curveDef->verts;
        const float *t0 = (const float *)( verts + 44 * indices[indexIter] );
        const float *t1 = (const float *)( verts + 44 * indices[indexIter + 1] );
        const float *t2 = (const float *)( verts + 44 * indices[indexIter + 2] );
        w->numpoints = 3;
        OrientationPosToWorldPos( w->p[0], t0, orient );
        OrientationPosToWorldPos( w->p[1], t1, orient );
        OrientationPosToWorldPos( w->p[2], t2, orient );
        sub_4DAA70( w, outList, desc, exterior );
    }

    --g_windingAlloc;
    free( w );
}

// ═════════════════════════════════════════════════════════════════════════════
//  sel_area 3D rect-select of patch/terrain CONTROL POINTS.  When the operator alt-drags a
//  marquee in the 3D camera view while a patch point-edit mode is active (sel_area),
//  Drag_MouseUp builds a 4-plane view frustum from the drag rect
//  (Camera_GetRectSelection3D, camwnd.cpp 0x403c10) and hands it to these two collectors,
//  which add every control point inside the frustum to the move-point sets.  Both live in
//  PMESH.CPP (assert strings).
//
//  Frustum plane layout (32-byte stride, written by Camera_GetRectSelection3D):
//    normal.x @0, normal.y @4, normal.z @8, dist (DOUBLE) @16, type(int) = -1 @24.
//  A point p is INSIDE plane i iff dot(normal_i, p) < dist_i (the binary breaks out of the
//  4-plane scan on the first `>= dist`; only points inside all 4 reach count == 4).
// ═════════════════════════════════════════════════════════════════════════════

struct edFrustumPlane_t          // the 32-byte plane Camera_GetRectSelection3D emits
{
    float  normal[3];            // @0
    int    pad_C;                // @12
    double dist;                 // @16
    int    type;                 // @24 (= -1)
    int    pad_1C;               // @28
};
static_assert( sizeof(edFrustumPlane_t) == 32, "edFrustumPlane_t" );

extern int Patch_FindSelectedArrayPoint( const float *pt );   // brush.cpp (sub_43C1C0 0x43c1c0)

// Is `xyz` inside all 4 frustum planes?  (binary's inner v7-counter loop.)
//   The binary accumulates the dot in the x87 stack and rounds it to a float32 ONCE
//   (`fstp dword [var_1C]`), then compares that float against the DOUBLE plane dist
//   (`fcomp qword [a1+16]`).  Match the single-rounding: accumulate in double, snap to
//   float, then compare against the double dist.  (A per-operation float accumulation
//   would round 3× and could flip a 1-ULP boundary case (truncation-vs-rounding).)
static bool Ed_PointInFrustum4( const edFrustumPlane_t *planes, const float *xyz )
{
    int n = 0;
    for ( ; n < 4; ++n )
    {
        float v13 = (float)( (double)planes[n].normal[0] * xyz[0]
                           + (double)planes[n].normal[1] * xyz[1]
                           + (double)planes[n].normal[2] * xyz[2] );
        if ( (double)v13 >= planes[n].dist )
            break;
    }
    return n == 4;
}

// ── Terrain_SelectAreaPoints_sub (0x4481c0) ───────────────────────────────────
//  One pass over every selected PATCH's 16×16 control grid; for each control point
//  inside the frustum, either toggle the lock flag (turned_edge bit1, when the
//  lock/unlock prefs are active), or add/remove it from d_move_points[].  `a2` is
//  the "second pass" flag (0 = add to a fresh set / remove-on-deselect; the caller
//  Terrain_SelectAreaPoints runs pass 0 first and pass 1 only if pass 0 added none).
//  Returns 1 iff this pass touched a point.
static int Terrain_SelectAreaPoints_sub( const edFrustumPlane_t *planes, char a2 )
{
    unsigned char touched = 0;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        patchMesh_t *patch = b->patch ? b->patch->def : nullptr;
        if ( !patch )
            continue;

        if ( b->patch->def != b->def->patch )
            Assert( PMESH_CPP, 6659, 0, "%s", "b->patch->def == b->def->patch" );

        // grid: outer = ctrl[col][..] (width), inner = ctrl[col][row] (height).
        for ( int col = 0; col < patch->width; ++col )
        {
            for ( int row = 0; row < patch->height; ++row )
            {
                drawVert_t *cp = &patch->ctrl[col][row];
                if ( !Ed_PointInFrustum4( planes, cp->xyz ) )
                    continue;

                if ( a2 )
                {
                    // pass 1: REMOVE this control point from d_move_points[] (by ptr).
                    if ( g_qeglobals.bLockPatchVerts || g_qeglobals.bUnlockPatchVerts )
                        Assert( PMESH_CPP, 6675, 0, "%s",
                                "!g_qeglobals.bLockPatchVerts && !g_qeglobals.bUnlockPatchVerts" );
                    int idx = 0;
                    int n = g_qeglobals.d_num_move_points;
                    for ( ; idx < n; ++idx )
                        if ( g_qeglobals.d_move_points[idx] == cp )
                            break;
                    if ( idx < n )
                    {
                        int rest = n - idx - 1;
                        if ( rest > 0 )
                            memmove( &g_qeglobals.d_move_points[idx],
                                     &g_qeglobals.d_move_points[idx + 1],
                                     sizeof( g_qeglobals.d_move_points[0] ) * rest );
                        g_qeglobals.d_num_move_points = n - 1;
                    }
                }
                else if ( g_qeglobals.bLockPatchVerts )
                {
                    cp->turned_edge |= 2;       // lock
                    touched = 1;
                }
                else if ( g_qeglobals.bUnlockPatchVerts )
                {
                    cp->turned_edge &= ~2;      // unlock
                    touched = 1;
                }
                else if ( ( cp->turned_edge & 2 ) == 0 )    // skip locked points
                {
                    // pass 0 ADD: queue this control point if not already present
                    // (by ptr).  Same drill-down head-insert as Patch_AppendMovePoint:
                    // if the owning patch is the first selected brush AND >1 brushes
                    // selected, insert at the front; else append.
                    int idx = 0;
                    int n = g_qeglobals.d_num_move_points;
                    for ( ; idx < n; ++idx )
                        if ( g_qeglobals.d_move_points[idx] == cp )
                            break;
                    if ( idx >= n )             // not already queued
                    {
                        touched = 1;
                        if ( n
                             && (brush_t *)patch->pSymbiot == selected_brushes.next->def
                             && selected_brushes.next != &selected_brushes )
                        {
                            memmove( &g_qeglobals.d_move_points[1],
                                     &g_qeglobals.d_move_points[0],
                                     sizeof( g_qeglobals.d_move_points[0] ) * n );
                            g_qeglobals.d_move_points[0] = cp;
                        }
                        else
                        {
                            g_qeglobals.d_move_points[n] = cp;
                        }
                        g_qeglobals.d_num_move_points = n + 1;
                    }
                }
            }
        }
    }

    return touched;
}

// ── Terrain_SelectAreaPoints (0x448460) ───────────────────────────────────────
//  Pass-0 first; if the lock/unlock prefs are off and pass 0 added nothing, retry
//  with pass 1 (the deselect/remove pass).  `select` (the caller's (buttons>>3)&1)
//  forces the fresh-set reset when off.
void Terrain_SelectAreaPoints( const void *planes, char select )
{
    int doSecondPass = 1;
    if ( !select || g_qeglobals.bLockPatchVerts || g_qeglobals.bUnlockPatchVerts )
    {
        g_qeglobals.d_num_move_points = 0;
        doSecondPass = 0;
    }
    g_nPatchClickedView = -1;
    int r = Terrain_SelectAreaPoints_sub( (const edFrustumPlane_t *)planes, 0 );
    if ( !r && doSecondPass )
        Terrain_SelectAreaPoints_sub( (const edFrustumPlane_t *)planes, 1 );
}

// ── Patch_SelectAreaPoints_sub (0x4484c0) ─────────────────────────────────────
//  Pass over the DISPLAYED candidate-point list (patch_verts_array01: vec3's at
//  &unkown_pmesh_float1, count patch_verts_array01_count) — the curve-edit feedback
//  set built during the patch curve drag.  Each point inside the frustum is added to
//  (pass 0) or removed from (pass 1) the SELECTED set patch_verts_array02 (vec3's at
//  &unkown_pmesh_float2, count patch_verts_array02_count).  Returns 1 iff pass 0 added.
static int Patch_SelectAreaPoints_sub( const edFrustumPlane_t *planes, char a2 )
{
    unsigned char added = 0;

    if ( g_qeglobals.bLockPatchVerts || g_qeglobals.bUnlockPatchVerts )
        return 0;

    if ( g_qeglobals.patch_verts_array01_count <= 0 )
        return 0;

    float *pt   = &g_qeglobals.unkown_pmesh_float1;   // array01 base, vec3 stride
    float *sel0 = &g_qeglobals.unkown_pmesh_float2;   // array02 base, vec3 stride

    for ( int i = 0; i < g_qeglobals.patch_verts_array01_count; ++i, pt += 3 )
    {
        if ( !Ed_PointInFrustum4( planes, pt ) )
            continue;

        if ( a2 )
        {
            if ( g_qeglobals.bLockPatchVerts || g_qeglobals.bUnlockPatchVerts )
                Assert( PMESH_CPP, 6754, 0, "%s",
                        "!g_qeglobals.bLockPatchVerts && !g_qeglobals.bUnlockPatchVerts" );
            int idx = Patch_FindSelectedArrayPoint( pt );
            if ( idx >= 0 )
            {
                int rest = g_qeglobals.patch_verts_array02_count - idx - 1;
                if ( rest > 0 )
                    memmove( sel0 + 3 * idx, sel0 + 3 * ( idx + 1 ),
                             sizeof( float ) * 3 * rest );
                --g_qeglobals.patch_verts_array02_count;
            }
        }
        else if ( Patch_FindSelectedArrayPoint( pt ) < 0 )
        {
            float *dst = sel0 + 3 * g_qeglobals.patch_verts_array02_count;
            dst[0] = pt[0];
            dst[1] = pt[1];
            dst[2] = pt[2];
            ++g_qeglobals.patch_verts_array02_count;
            added = 1;
        }
    }

    return added;
}

// ── Patch_SelectAreaPoints (0x448620) ─────────────────────────────────────────
void Patch_SelectAreaPoints( const void *planes, char select )
{
    int doSecondPass = 1;
    if ( !select )
    {
        g_qeglobals.patch_verts_array02_count = 0;
        doSecondPass = 0;
    }
    g_nPatchClickedView = -1;
    int r = Patch_SelectAreaPoints_sub( (const edFrustumPlane_t *)planes, 0 );
    if ( !r && doSecondPass )
        Patch_SelectAreaPoints_sub( (const edFrustumPlane_t *)planes, 1 );
}

// ── Splay_Vec2Normalize (editor Vec2Normalize 0x4A4750) — normalize a 2D vector in
// place, return its length.  A length of 0 (or non-positive) leaves the vector
// untouched (divides by 1).  NOTE the hex-rays `-v3 >= 0.0` is `v3 <= 0.0`.  (Named to
// avoid clashing with the engine's com_math Vec2Normalize(vec2r).)
static double Splay_Vec2Normalize( float *v )
{
    double len = sqrt( (double)( v[0] * v[0] + v[1] * v[1] ) );
    float d = (float)len;
    if ( d <= 0.0f )
        d = 1.0f;
    float inv = 1.0f / d;
    v[0] = v[0] * inv;
    v[1] = v[1] * inv;
    return len;
}

// ── DoSplay (0x44A430) — Selection→Splay (33157, Ctrl+W) ──────────────────────────
// "Splays" the selected patch control points radially: computes their 2D (view-plane
// XY) bbox centre, then pushes each point to a fixed radius (Prefs→SplayDistance) along
// its direction from the centre.  Requires ONLY patches selected (no faces, all selected
// brushes are patches) and 2..1025 move-points picked.  Rebuilds every selected patch.
// Faithful to the binary; NOTE the scratch `v15[]` copy of pre-splay xyz is written but
// never read back (dead in the decompile) — preserved as a plain local pass here.

void DoSplay()
{
    if ( g_SelectedFaces.GetSize() > 0 )
    {
        Sys_Printf( "can only connect patch vertices if only patches are selected\n" );
        return;
    }
    if ( selected_brushes.next == &selected_brushes )
    {
        Sys_Printf( "can only connect patch vertices if only patches are selected\n" );
        return;
    }
    // Every selected brush must be a patch.
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( !b->patch )
        {
            Sys_Printf( "can only connect patch vertices if only patches are selected\n" );
            return;
        }

    if ( g_qeglobals.d_num_move_points < 2 )
    {
        Sys_Printf( "can only splay patch vertices if at least 2 vertices are selected\n" );
        return;
    }
    if ( (unsigned)g_qeglobals.d_num_move_points > 1025u )
    {
        Sys_Printf( "can only splay patch vertices if at most %i vertices are selected\n", 1025 );
        return;
    }

    // 2D (X/Y) bounding box of the picked control points.
    float minx = g_qeglobals.d_move_points[0]->xyz[0];
    float maxx = g_qeglobals.d_move_points[0]->xyz[0];
    float miny = g_qeglobals.d_move_points[0]->xyz[1];
    float maxy = g_qeglobals.d_move_points[0]->xyz[1];
    for ( int i = g_qeglobals.d_num_move_points - 1; i > 0; --i )
    {
        drawVert_t *v = g_qeglobals.d_move_points[i];
        if ( minx > (double)v->xyz[0] ) minx = v->xyz[0];
        if ( maxx < (double)v->xyz[0] ) maxx = v->xyz[0];
        if ( miny > (double)v->xyz[1] ) miny = v->xyz[1];
        if ( maxy < (double)v->xyz[1] ) maxy = v->xyz[1];
    }

    float cx = ( maxx + minx ) * 0.5f;
    float cy = ( maxy + miny ) * 0.5f;
    float radius = (float)g_PrefsDlg->splay;

    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        drawVert_t *v = g_qeglobals.d_move_points[i];
        float dir[2];
        dir[0] = v->xyz[0] - cx;
        dir[1] = v->xyz[1] - cy;
        Splay_Vec2Normalize( dir );
        v->xyz[0] = dir[0] * radius + cx;
        v->xyz[1] = radius * dir[1] + cy;
    }

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        if ( b->patch && b->patch->def )
            Patch_Rebuild( b->patch->def, 1 );
    g_nUpdateBits = -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  PATCH/CURVE OPERATION CLUSTER (Cap / Thicken / Weld / Split), transcribed op-by-op
//  from the CoD4Radiant decompilation + disassembly.
//  ctrl grid convention (VERIFIED): patchMesh_t.ctrl[16][16] with p->ctrl[col][row] ==
//  p+56 + col*1280 + row*80 (drawVert_t is 80 bytes).  The IDA `def->ctrl[a][b]` == col a,
//  row b - direct 1:1 transcription.
// ═════════════════════════════════════════════════════════════════════════════

// externs the cluster needs (all ported elsewhere; declared here for locality)
#define selected_brushes_next ( selected_brushes.next )   // IDB 0x23F1868 == selected_brushes.next
extern eclass_t    *Eclass_ForName( int hasBrushes, const char *name );      // eclass.cpp
extern entity_s    *Entity_Create( eclass_t *eclass );                       // entity.cpp
extern void         SetKeyValue( entity_s_def *e, const char *key, const char *value );// entity.cpp
extern void         Undo_SetIdForEntity( entity_s_def *ent );                // undo.cpp 0x45E9E0
extern void         Undo_AddEntity_W( entity_s *e );                         // 0x45E990
extern void         Entity_Free( char *owner );                             // entity.cpp 0x485750 (a1=char*)
extern void         Brush_Free( selbrush_t *b );                            // brush.cpp 0x475BA0
extern void         Select_Deselect( int bDeselectFaces );                  // select.cpp 0x48E800
extern void         Brush_RemoveFromList( selbrush_t *b );                  // brush.cpp
extern entity_s    *world_entity;                                           // 0x25D5B30 (map.cpp)
extern void        *zero;                                                   // empty-string sentinel (engine_stubs.cpp)

// VectorCompare (0x44D810) — vec3 approx-equality (epsilon^2 = 1e-6).  Small helper,
// inlined here (not otherwise ported).
static int PMESH_VectorCompare( const float *a, const float *b )
{
    for ( int k = 0; k < 3; ++k )
    {
        float d = a[k] - b[k];
        if ( d * d > 0.00000100000011116208f )
            return 0;
    }
    return 1;
}

// sub_483B40 (0x483B40) — is `classname` a weld-mergeable container (worldspawn /
// func_group / func_cullgroup)?  Inlined here (its only radiant caller is WeldMesh).
static char PMESH_IsWeldContainer( const char *classname )
{
    static const char *names[3] = { "worldspawn", "func_group", "func_cullgroup" };
    for ( int i = 0; i < 3; ++i )
        if ( !strcmp( classname, names[i] ) )
            return 1;
    return 0;
}

// Region_BoundsMidpoint (0x409F20) — midpoint of two vec3 (static in brush.cpp;
// re-inlined here for the CapSpecial endcap).
static void PMESH_Midpoint( const float *a, const float *b, float *out )
{
    out[0] = ( a[0] + b[0] ) * 0.5f;
    out[1] = ( a[1] + b[1] ) * 0.5f;
    out[2] = ( a[2] + b[2] ) * 0.5f;
}

// The Curve→Cap "special cap" dialog (sub_40A8A0 = CCapDialog, IDD 0xA1) — headless: the
// port never pops it; Patch_CapCurrent handles the no-edge-matched case as a no-op.

// ── Patch_CapFillGrid (sub_439BA0, 0x439BA0) — replicate one row's xyz down every ─
//   row of the cap grid.  a2 = the just-created cap patchMesh_t, a1 = a source vec3
//   (the picked edge point).  For every control point (col, row) of the cap: xyz = a1.
static void Patch_CapFillGrid( const float *a1, patchMesh_t *a2 )   // args: a1@ecx, a2@edi
{
    for ( int col = 0; col < a2->width; ++col )
        for ( int row = 0; row < a2->height; ++row )
        {
            a2->ctrl[col][row].xyz[0] = a1[0];
            a2->ctrl[col][row].xyz[1] = a1[1];
            a2->ctrl[col][row].xyz[2] = a1[2];
        }
}

// cap perimeter / interior index tables (IDB s_capPerimeter3x3 etc.).  Each entry is
// a {col,row} pair.  The 3x3 form is used for a source edge of length <= 9, the 5x5
// form for length > 9 (a longer edge needs a finer cap).
static const int s_capPerimeter3x3[8][2] =
{ {0,0},{1,0},{2,0},{2,1},{2,2},{1,2},{0,2},{0,1} };
static const int s_capPerimeter5x5[16][2] =
{ {0,0},{1,0},{2,0},{3,0},{4,0},{4,1},{4,2},{4,3},
  {4,4},{3,4},{2,4},{1,4},{0,4},{0,3},{0,2},{0,1} };
static const int s_capInterior3x3[1][2] = { {1,1} };
static const int s_capInterior5x5[9][2] =
{ {1,1},{2,1},{3,1},{1,2},{2,2},{3,2},{1,3},{2,3},{3,3} };

// ── Patch_Cap (0x439C00) — build ONE cap patch off a source edge ──────────────
//   bByColumn selects which axis of the source patch `pm` supplies the edge; bFirst
//   selects the low (0) or high (width-1/height-1) end.  A 3x3 (edge<=9) or 5x5
//   (edge>9) generic mesh is created, its perimeter ring filled from the source edge
//   control points (via the tables), its interior filled with the perimeter's bbox
//   centre, then cap-textured (Patch_Rebuild + Patch_ST projection — identical tail
//   to Patch_CapTexture).  Returns the new symbiont brush instance (or 0).
static selbrush_t *Patch_Cap( patchMesh_t *pm, char bByColumn, char bFirst )
{
    selbrush_t *b;
    char        bIs3x3;                       // var_AD: 1 = 3x3 cap, 0 = 5x5 cap
    if ( pm->width > 9 )
    {
        b      = Patch_GenericMesh( 5, 5, 2, 0, 0 );
        bIs3x3 = 0;
    }
    else
    {
        b      = Patch_GenericMesh( 3, 3, 2, 0, 0 );
        bIs3x3 = 1;
    }
    if ( !b )
    {
        Sys_Printf( "Unable to cap. You may need to ungroup the patch.\n" );
        return 0;
    }
    iassert( b->patch->def == b->def->patch );
    patchMesh_t *def = b->patch->def;
    def->type = (PATCH_TYPES)( def->type | PATCH_CAP );

    // bbox accumulators for the interior fill (mins/maxs, VectorMaxValues convention).
    float mins[3] = { 131072.0f, 131072.0f, 131072.0f };
    float maxs[3] = { -131072.0f, -131072.0f, -131072.0f };

    // srcCount = number of edge control points; edgeIdx = the fixed index of the edge.
    int srcCount = bByColumn ? pm->width : pm->height;
    int edgeIdx;
    if ( bFirst )
        edgeIdx = 0;
    else
        edgeIdx = ( bByColumn ? pm->width : pm->height ) - 1;

    // seed the whole cap grid with the first edge point (source's [edgeIdx][edgeIdx]
    // corner — the IDB adds `20*edgeIdx` DWORDs = 80*edgeIdx bytes to pm, then reads
    // ctrl[0][edgeIdx] at +0x38).  Then fills the ring.
    // v7 base = &pm->ctrl[0][edgeIdx] (walk +0x40 to skip to xyz below).
    drawVert_t *edgeBase = &pm->ctrl[0][edgeIdx];
    Patch_CapFillGrid( edgeBase->xyz, def );

    // ── perimeter ring: map each source-edge control point onto the cap ring ──
    for ( int i = 0; i < srcCount; ++i )
    {
        // src point: bByColumn → walk down the columns (ctrl[i][edgeIdx]);
        //            else       → walk along the row (ctrl[edgeIdx][i]).
        //   edgeBase = &pm->ctrl[0][edgeIdx]; esi advances +0x40 then +0x500 per i
        //   (=1280 bytes = one column), so src = &pm->ctrl[i][edgeIdx].xyz.
        const float *src = bByColumn
                         ? pm->ctrl[i][edgeIdx].xyz
                         : pm->ctrl[edgeIdx][i].xyz;

        // ring slot for this i (3x3 → perimeter of 3x3, 5x5 → perimeter of 5x5).
        int rc, rr;
        if ( bIs3x3 ) { rc = s_capPerimeter3x3[i][0]; rr = s_capPerimeter3x3[i][1]; }
        else          { rc = s_capPerimeter5x5[i][0]; rr = s_capPerimeter5x5[i][1]; }
        def->ctrl[rc][rr].xyz[0] = src[0];
        def->ctrl[rc][rr].xyz[1] = src[1];
        def->ctrl[rc][rr].xyz[2] = src[2];

        // VectorMaxValues(src, mins, maxs): mins = min(mins,src); maxs = max(maxs,src)
        // (0x4A8240; a FATAL stub in this build → inlined here).
        for ( int k = 0; k < 3; ++k )
        {
            if ( mins[k] > (double)src[k] ) mins[k] = src[k];
            if ( maxs[k] < (double)src[k] ) maxs[k] = src[k];
        }
    }

    // ── interior fill: centre = (mins+maxs)*0.5, written to every interior slot ──
    float centre[3];
    centre[0] = ( mins[0] + maxs[0] ) * 0.5f;
    centre[1] = ( mins[1] + maxs[1] ) * 0.5f;
    centre[2] = ( mins[2] + maxs[2] ) * 0.5f;
    int interiorCount = bIs3x3 ? 1 : 9;
    for ( int i = 0; i < interiorCount; ++i )
    {
        int ic, ir;
        if ( bIs3x3 ) { ic = s_capInterior3x3[i][0]; ir = s_capInterior3x3[i][1]; }
        else          { ic = s_capInterior5x5[i][0]; ir = s_capInterior5x5[i][1]; }
        def->ctrl[ic][ir].xyz[0] = centre[0];
        def->ctrl[ic][ir].xyz[1] = centre[1];
        def->ctrl[ic][ir].xyz[2] = centre[2];
    }

    // ── if bFirst, vertical-flip the cap grid (mirror each column's rows) so the ──
    //   winding faces outward (0x439f15 loop; identical to patchInvert2's swap).
    if ( bFirst )
    {
        for ( int col = 0; col < def->width; ++col )
            for ( int r = 0; r < def->height / 2; ++r )
            {
                drawVert_t tmp                    = def->ctrl[col][r];
                def->ctrl[col][r]                 = def->ctrl[col][def->height - 1 - r];
                def->ctrl[col][def->height - 1 - r] = tmp;
            }
    }

    // ── cap texturing tail (identical to Patch_CapTexture( def, cap=0, s, s )) ──
    Patch_Rebuild( def, 1 );
    {
        const int layer = g_qeglobals.current_edit_layer;
        float s = g_qeglobals.random_texture_stuff[layer].sampleSize;
        Patch_CapTexture( def, 0, s, s );
    }
    iassert( def->pSymbiot == (entity_brush_s *)b->def );
    iassert( b->patch->def == def );
    iassert( b->def->patch == def );
    return b;
}

// ── Patch_CapSpecial (0x43A170) — dialog-driven "special" cap shape ───────────
//   nType selects the cap silhouette (bevel / endcap / inverted etc.); bFirst picks
//   the source row (0 or height-1).  Builds a 3x3 (or 5x3 for nType==1) cap grid by
//   sampling the source ROW's control points (ctrl[col][edgeIdx]) into fixed cap-grid
//   slots per shape, optionally vertical-flips, then cap-textures.
static selbrush_t *Patch_CapSpecial( patchMesh_t *pm, int nType, char bFirst )
{
    selbrush_t *b = ( nType == 3 ) ? Patch_GenericMesh( 5, 3, 2, 0, 0 )
                                   : Patch_GenericMesh( 3, 3, 2, 0, 0 );
    if ( !b )
    {
        Sys_Printf( "Unable to cap. Make sure you ungroup before re - capping." );
        return 0;
    }
    iassert( b->patch->def == b->def->patch );
    patchMesh_t *def = b->patch->def;
    def->type = (PATCH_TYPES)( def->type | PATCH_CAP );

    int edgeIdx = bFirst ? 0 : ( pm->height - 1 );

    // src[col] = &pm->ctrl[col][edgeIdx].xyz : the IDB indexes `&pm->width + 20*edgeIdx`
    // (= pm + 80*edgeIdx bytes) as a float array, where [14]=ctrl[0][edgeIdx].x,
    // [334]=ctrl[1], [654]=ctrl[2], [974]=ctrl[3], [1294]=ctrl[4] (320 floats/column).
    #define SRC(col) ( pm->ctrl[(col)][edgeIdx].xyz )

    // Patch_CalcBounds(v36, v29, pm, 0) — v29 is a scratch mins used by nType 1.
    float capMins[3], capMaxs[3];
    Patch_CalcBounds( capMins, capMaxs, pm, false );
    float mid[3];

    char bFlipMatch;   // v10: whether this shape's grid needs the vertical flip
    if ( nType == 2 )
    {
        def->ctrl[0][0].xyz[0] = SRC(0)[0]; def->ctrl[0][0].xyz[1] = SRC(0)[1]; def->ctrl[0][0].xyz[2] = SRC(0)[2];
        def->ctrl[0][2].xyz[0] = SRC(2)[0]; def->ctrl[0][2].xyz[1] = SRC(2)[1]; def->ctrl[0][2].xyz[2] = SRC(2)[2];
        def->ctrl[0][1].xyz[0] = SRC(1)[0]; def->ctrl[0][1].xyz[1] = SRC(1)[1]; def->ctrl[0][1].xyz[2] = SRC(1)[2];
        def->ctrl[2][2].xyz[0] = SRC(1)[0]; def->ctrl[2][2].xyz[1] = SRC(1)[1]; def->ctrl[2][2].xyz[2] = SRC(1)[2];
        def->ctrl[1][0].xyz[0] = SRC(1)[0]; def->ctrl[1][0].xyz[1] = SRC(1)[1]; def->ctrl[1][0].xyz[2] = SRC(1)[2];
        def->ctrl[1][1].xyz[0] = SRC(1)[0]; def->ctrl[1][1].xyz[1] = SRC(1)[1]; def->ctrl[1][1].xyz[2] = SRC(1)[2];
        def->ctrl[1][2].xyz[0] = SRC(1)[0]; def->ctrl[1][2].xyz[1] = SRC(1)[1]; def->ctrl[1][2].xyz[2] = SRC(1)[2];
        def->ctrl[2][0].xyz[0] = SRC(1)[0]; def->ctrl[2][0].xyz[1] = SRC(1)[1]; def->ctrl[2][0].xyz[2] = SRC(1)[2];
        def->ctrl[2][1].xyz[0] = SRC(1)[0]; def->ctrl[2][1].xyz[1] = SRC(1)[1]; def->ctrl[2][1].xyz[2] = SRC(1)[2];
        bFlipMatch = 0;
    }
    else if ( nType == 0 )
    {
        // bevel: ctrl[2][2] = SRC(2) + (SRC(0) - SRC(1)); centre column/rows = that pt.
        def->ctrl[0][0].xyz[0] = SRC(2)[0]; def->ctrl[0][0].xyz[1] = SRC(2)[1]; def->ctrl[0][0].xyz[2] = SRC(2)[2];
        def->ctrl[0][1].xyz[0] = SRC(1)[0]; def->ctrl[0][1].xyz[1] = SRC(1)[1]; def->ctrl[0][1].xyz[2] = SRC(1)[2];
        def->ctrl[0][2].xyz[0] = SRC(0)[0]; def->ctrl[0][2].xyz[1] = SRC(0)[1]; def->ctrl[0][2].xyz[2] = SRC(0)[2];
        mid[0] = SRC(2)[0] + ( SRC(0)[0] - SRC(1)[0] );
        mid[1] = SRC(2)[1] + ( SRC(0)[1] - SRC(1)[1] );
        mid[2] = SRC(2)[2] + ( SRC(0)[2] - SRC(1)[2] );
        def->ctrl[2][2].xyz[0] = mid[0]; def->ctrl[2][2].xyz[1] = mid[1]; def->ctrl[2][2].xyz[2] = mid[2];
        def->ctrl[1][0].xyz[0] = mid[0]; def->ctrl[1][0].xyz[1] = mid[1]; def->ctrl[1][0].xyz[2] = mid[2];
        def->ctrl[1][1].xyz[0] = mid[0]; def->ctrl[1][1].xyz[1] = mid[1]; def->ctrl[1][1].xyz[2] = mid[2];
        def->ctrl[1][2].xyz[0] = mid[0]; def->ctrl[1][2].xyz[1] = mid[1]; def->ctrl[1][2].xyz[2] = mid[2];
        def->ctrl[2][0].xyz[0] = mid[0]; def->ctrl[2][0].xyz[1] = mid[1]; def->ctrl[2][0].xyz[2] = mid[2];
        def->ctrl[2][1].xyz[0] = mid[0]; def->ctrl[2][1].xyz[1] = mid[1]; def->ctrl[2][1].xyz[2] = mid[2];
        bFlipMatch = 1;
    }
    else if ( nType == 1 )
    {
        // endcap: interior column midpoint via Region_BoundsMidpoint(SRC(4), SRC(0), &v29).
        PMESH_Midpoint( SRC(4), SRC(0), mid );
        def->ctrl[0][0].xyz[0] = SRC(0)[0]; def->ctrl[0][0].xyz[1] = SRC(0)[1]; def->ctrl[0][0].xyz[2] = SRC(0)[2];
        def->ctrl[1][0].xyz[0] = mid[0];    def->ctrl[1][0].xyz[1] = mid[1];    def->ctrl[1][0].xyz[2] = mid[2];
        def->ctrl[2][0].xyz[0] = SRC(4)[0]; def->ctrl[2][0].xyz[1] = SRC(4)[1]; def->ctrl[2][0].xyz[2] = SRC(4)[2];
        def->ctrl[0][2].xyz[0] = SRC(2)[0]; def->ctrl[0][2].xyz[1] = SRC(2)[1]; def->ctrl[0][2].xyz[2] = SRC(2)[2];
        def->ctrl[1][2].xyz[0] = SRC(2)[0]; def->ctrl[1][2].xyz[1] = SRC(2)[1]; def->ctrl[1][2].xyz[2] = SRC(2)[2];
        def->ctrl[2][2].xyz[0] = SRC(2)[0]; def->ctrl[2][2].xyz[1] = SRC(2)[1]; def->ctrl[2][2].xyz[2] = SRC(2)[2];
        def->ctrl[1][1].xyz[0] = SRC(2)[0]; def->ctrl[1][1].xyz[1] = SRC(2)[1]; def->ctrl[1][1].xyz[2] = SRC(2)[2];
        def->ctrl[0][1].xyz[0] = SRC(1)[0]; def->ctrl[0][1].xyz[1] = SRC(1)[1]; def->ctrl[0][1].xyz[2] = SRC(1)[2];
        def->ctrl[2][1].xyz[0] = SRC(3)[0]; def->ctrl[2][1].xyz[1] = SRC(3)[1]; def->ctrl[2][1].xyz[2] = SRC(3)[2];
        bFlipMatch = 0;
    }
    else   // nType == 3 (5x3 endcap, sampling all 5 source columns)
    {
        def->ctrl[0][0].xyz[0] = SRC(0)[0]; def->ctrl[0][0].xyz[1] = SRC(0)[1]; def->ctrl[0][0].xyz[2] = SRC(0)[2];
        def->ctrl[1][0].xyz[0] = SRC(1)[0]; def->ctrl[1][0].xyz[1] = SRC(1)[1]; def->ctrl[1][0].xyz[2] = SRC(1)[2];
        def->ctrl[2][0].xyz[0] = SRC(2)[0]; def->ctrl[2][0].xyz[1] = SRC(2)[1]; def->ctrl[2][0].xyz[2] = SRC(2)[2];
        def->ctrl[3][0].xyz[0] = SRC(3)[0]; def->ctrl[3][0].xyz[1] = SRC(3)[1]; def->ctrl[3][0].xyz[2] = SRC(3)[2];
        def->ctrl[4][0].xyz[0] = SRC(4)[0]; def->ctrl[4][0].xyz[1] = SRC(4)[1]; def->ctrl[4][0].xyz[2] = SRC(4)[2];
        def->ctrl[0][1].xyz[0] = SRC(1)[0]; def->ctrl[0][1].xyz[1] = SRC(1)[1]; def->ctrl[0][1].xyz[2] = SRC(1)[2];
        def->ctrl[1][1].xyz[0] = SRC(1)[0]; def->ctrl[1][1].xyz[1] = SRC(1)[1]; def->ctrl[1][1].xyz[2] = SRC(1)[2];
        def->ctrl[2][1].xyz[0] = SRC(2)[0]; def->ctrl[2][1].xyz[1] = SRC(2)[1]; def->ctrl[2][1].xyz[2] = SRC(2)[2];
        def->ctrl[3][1].xyz[0] = SRC(3)[0]; def->ctrl[3][1].xyz[1] = SRC(3)[1]; def->ctrl[3][1].xyz[2] = SRC(3)[2];
        def->ctrl[4][1].xyz[0] = SRC(3)[0]; def->ctrl[4][1].xyz[1] = SRC(3)[1]; def->ctrl[4][1].xyz[2] = SRC(3)[2];
        def->ctrl[0][2].xyz[0] = SRC(1)[0]; def->ctrl[0][2].xyz[1] = SRC(1)[1]; def->ctrl[0][2].xyz[2] = SRC(1)[2];
        def->ctrl[1][2].xyz[0] = SRC(1)[0]; def->ctrl[1][2].xyz[1] = SRC(1)[1]; def->ctrl[1][2].xyz[2] = SRC(1)[2];
        def->ctrl[2][2].xyz[0] = SRC(2)[0]; def->ctrl[2][2].xyz[1] = SRC(2)[1]; def->ctrl[2][2].xyz[2] = SRC(2)[2];
        def->ctrl[3][2].xyz[0] = SRC(3)[0]; def->ctrl[3][2].xyz[1] = SRC(3)[1]; def->ctrl[3][2].xyz[2] = SRC(3)[2];
        def->ctrl[4][2].xyz[0] = SRC(3)[0]; def->ctrl[4][2].xyz[1] = SRC(3)[1]; def->ctrl[4][2].xyz[2] = SRC(3)[2];
        bFlipMatch = 1;   // nType==3 falls through to v10=1 (the shared else block set v10=1)
    }
    #undef SRC

    // vertical-flip the cap grid when bFirst matches this shape's flip flag.
    if ( bFirst == bFlipMatch )
    {
        for ( int col = 0; col < def->width; ++col )
            for ( int r = 0; r < def->height / 2; ++r )
            {
                drawVert_t tmp                      = def->ctrl[col][r];
                def->ctrl[col][r]                   = def->ctrl[col][def->height - 1 - r];
                def->ctrl[col][def->height - 1 - r] = tmp;
            }
    }

    Patch_Rebuild( def, 1 );
    {
        const int layer = g_qeglobals.current_edit_layer;
        float s = g_qeglobals.random_texture_stuff[layer].sampleSize;
        Patch_CapTexture( def, 0, s, s );
    }
    iassert( def->pSymbiot == (entity_brush_s *)b->def );
    iassert( b->patch->def == def );
    iassert( b->def->patch == def );
    return b;
}

// ── CCapDialog (sub_40A8A0, IDD 0xA1 = 161) — the "special cap" type picker ──────
//   Radio dialog popped by Patch_CapCurrent when the patch has no closed seam edge.
//   The binary's DoDataExchange (0x40A900) is one DDX_Radio(pDX, 1285, m_nType@+0x74)
//   over the 4-button group {1285 Bevel, 1287 Endcap, 1289 Inverted Bevel, 1291
//   Inverted Endcap} → index 0..3.  No OnInitDialog / OnOK overrides (default MFC).
//   Hand-built like CDialogThick / CPatchDensityDlg.
class CCapDialog : public CDialog
{
public:
    CCapDialog( CWnd *parent = nullptr ) : CDialog( IDD_CAP, parent ), m_nType( 0 ) {}
    int m_nType;   // this+0x74 (DDX_Radio group base 1285): 0=bevel 1=endcap 2=invbevel 3=invendcap
protected:
    virtual void DoDataExchange( CDataExchange *pDX )   // 0x40A900
    {
        CDialog::DoDataExchange( pDX );
        DDX_Radio( pDX, 1285, m_nType );
    }
};

// ── Patch_CapCurrent (0x43AA20) — Curve→Cap: cap the single selected patch ──────
//   Auto-cap all four "closed" edges (VectorCompare tests the seam), each becoming a
//   cap brush via Patch_Cap.  If none matched (open patch), pop the special-cap dialog
//   (headless: skipped) and build two special caps.  Groups the caps into a func_group.
void Patch_CapCurrent()
{
    if ( !( QE_SingleBrush() && selected_brushes_next->patch ) )
    {
        Sys_Printf( "Cannot cap multiple selection. Please select a single patch.\n" );
        return;
    }

    selbrush_t   *sel = selected_brushes_next;
    int           n   = 0;
    entity_brush_s *pCap = (entity_brush_s *)sel;
    iassert( sel->patch->def == sel->def->patch );
    patchMesh_t  *def = sel->patch->def;
    selbrush_t   *caps[4] = { 0, 0, 0, 0 };

    // The four seam tests mirror the IDB's raw-offset VectorCompare calls; they check
    // whether opposite edges of the control grid coincide (a closed loop → cap it).
    //  1) ctrl[0][0]        vs ctrl[width-1][0]   (column 0 vs last column, row 0)
    if ( PMESH_VectorCompare( def->ctrl[0][0].xyz,
                             def->ctrl[def->width - 1][0].xyz ) )
    {
        selbrush_t *c = Patch_Cap( def, 1, 0 );
        if ( c ) caps[n++] = c;
    }
    //  2) ctrl[0][height-1] vs ctrl[width-1][height-1]
    if ( PMESH_VectorCompare( def->ctrl[0][def->height - 1].xyz,
                             def->ctrl[def->width - 1][def->height - 1].xyz ) )
    {
        selbrush_t *c = Patch_Cap( def, 1, 1 );
        if ( c ) caps[n++] = c;
    }
    //  3) ctrl[0][0]        vs ctrl[0][height-1]  (row 0 vs last row, column 0)
    if ( PMESH_VectorCompare( def->ctrl[0][0].xyz,
                             def->ctrl[0][def->height - 1].xyz ) )
    {
        selbrush_t *c = Patch_Cap( def, 0, 0 );
        if ( c ) caps[n++] = c;
    }
    //  4) ctrl[width-1][0]  vs ctrl[width-1][height-1]
    if ( PMESH_VectorCompare( def->ctrl[def->width - 1][0].xyz,
                             def->ctrl[def->width - 1][def->height - 1].xyz ) )
    {
        selbrush_t *c = Patch_Cap( def, 0, 1 );
        if ( c ) caps[n++] = c;
    }

    if ( n == 0 )
    {
        // No closed edge → pop CCapDialog (sub_40A8A0, IDD 0xA1) to pick the special-cap
        // shape, then build two Patch_CapSpecial brushes (bFirst 0 and 1) — verbatim from
        // Patch_CapCurrent 0x43abaf-0x43abf3.
        CCapDialog dlg;
        dlg.m_nType = 0;
        if ( dlg.DoModal() == IDOK )
        {
            selbrush_t *c0 = Patch_CapSpecial( def, dlg.m_nType, 0 );
            selbrush_t *c1 = Patch_CapSpecial( def, dlg.m_nType, 1 );
            caps[0] = c0;
            caps[1] = c1;
            n = 2;
        }
    }

    if ( n > 0 )
    {
        for ( int i = n - 1; i >= 0; --i )
            if ( caps[i] )
                Select_Brush( caps[i], 1, 1, 0 );

        if ( (entity_s *)pCap->owner == world_entity )
        {
            eclass_t *ec = Eclass_ForName( 0, "func_group" );
            if ( ec )
            {
                entity_s *ent = Entity_Create( ec );
                SetKeyValue( (entity_s_def *)ent->def, "type", "patchCapped" );
                Undo_SetIdForEntity( (entity_s_def *)ent->def );
            }
        }
    }
}

// ── Patch_Thicken (0x448700) — Curve→Thicken: offset a copy along normals ──────
//   a1 = thickness, bseam = build the connecting seam strip patches.  Duplicates the
//   selected patch, offsets every control point of the duplicate by (-thickness)*normal
//   (mesh normals must be computed first), inverts it, and — when bseam — builds up to
//   four 3-wide "seam" patches connecting the source edges to the offset copy.
//   Reached via OnCurveThicken (cmd 32904, CDialogThick IDD 0xA4).
extern patchMesh_t *Patch_Duplicate( patchMesh_t *from );
static void patchInvert2( patchMesh_t *p );   // fwd (defined above; static)

void Patch_Thicken( int a1, char bseam )
{
    int v73 = -a1;                                // negated thickness
    if ( !( QE_SingleBrush() && selected_brushes_next->patch
            && selected_brushes_next->patch->def ) )
    {
        Sys_Printf( "Cannot thicken multiple patches. Please select a single patch.\n" );
        return;
    }
    patchMesh_t *pmesh = selected_brushes_next->patch->def;
    patchMesh_t *v70   = pmesh;                    // original (survives the seam swaps)

    if ( ( pmesh->type & PATCH_TERRAIN ) != 0 )
    {
        Sys_Printf( "Thickening of terrain patches not yet implemented.  "
                    "Tell the coders if you need this feature.\n" );
        return;
    }

    Undo_ClearRedo();
    Undo_GeneralStart( "Patch thicken" );
    Undo_AddBrushList( &selected_brushes );
    Patch_MeshNormals( pmesh );
    patchMesh_t *v4 = Patch_Duplicate( pmesh );    // the offset copy
    patchMesh_t *v63 = v4;

    // 1) offset every control point of the copy: dup.xyz = src.xyz + (-t)*src.normal
    float t = (float)v73;
    for ( int col = 0; col < pmesh->width; ++col )
        for ( int row = 0; row < pmesh->height; ++row )
        {
            drawVert_t *s = &pmesh->ctrl[col][row];
            drawVert_t *d = &v4->ctrl[col][row];
            d->xyz[0] = s->normal[0] * t + s->xyz[0];
            d->xyz[1] = s->normal[1] * t + s->xyz[1];
            d->xyz[2] = s->normal[2] * t + s->xyz[2];
        }

    Patch_Rebuild( v4, 1 );
    v4->type = (PATCH_TYPES)( v4->type | PATCH_THICK );
    selbrush_t *v13 = (selbrush_t *)Brush_AddToList( (brush_t *)v4->pSymbiot,
                                                     selected_brushes_next->owner );
    if ( v13->next || v13->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    Brush_AddToList2( v13 );

    if ( bseam )
    {
        // Non-cylinder patches get four seam strips (two along height, two along width).
        if ( ( pmesh->type & PATCH_CYLINDER ) == 0 )
        {
            // ── seam A: 3 x height strip along column 0 (src col0 ↔ dup col0) ──
            selbrush_t *bA;
            if ( pmesh->height < 3 )
            {
                Sys_Printf( "Invalid patch width or height.\n" );
                bA = 0;
            }
            else
                bA = Patch_GenericMesh( 3, pmesh->height, 2, 0, 1 );
            iassert( bA->patch->def == bA->def->patch );
            patchMesh_t *def = bA->patch->def;
            def->type = (PATCH_TYPES)( def->type | PATCH_SEAM );
            for ( int row = 0; row < pmesh->height; ++row )
            {
                // col 0 = src col0; col 2 = dup col0; col 1 = midpoint.
                def->ctrl[0][row].xyz[0] = pmesh->ctrl[0][row].xyz[0];
                def->ctrl[0][row].xyz[1] = pmesh->ctrl[0][row].xyz[1];
                def->ctrl[0][row].xyz[2] = pmesh->ctrl[0][row].xyz[2];
                def->ctrl[2][row].xyz[0] = v4->ctrl[0][row].xyz[0];
                def->ctrl[2][row].xyz[1] = v4->ctrl[0][row].xyz[1];
                def->ctrl[2][row].xyz[2] = v4->ctrl[0][row].xyz[2];
                def->ctrl[1][row].xyz[0] = ( def->ctrl[0][row].xyz[0] + def->ctrl[2][row].xyz[0] ) * 0.5f;
                def->ctrl[1][row].xyz[1] = ( def->ctrl[0][row].xyz[1] + def->ctrl[2][row].xyz[1] ) * 0.5f;
                def->ctrl[1][row].xyz[2] = ( def->ctrl[0][row].xyz[2] + def->ctrl[2][row].xyz[2] ) * 0.5f;
            }
            float smins[3], smaxs[3];
            Patch_CalcBounds( smins, smaxs, def, true );
            Brush_RebuildBrush( (brush_t *)def->pSymbiot, smins, smaxs );
            Patch_Naturalize( def );
            patchInvert2( def );
            if ( def->curveDef )
                free( def->curveDef );
            def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
            ++def->version;
            Select_Brush( bA, 1, 1, 0 );

            // ── seam B: 3 x height strip along last column (src colW ↔ dup colW) ──
            int lastCol = v70->width - 1;
            selbrush_t *bB;
            if ( v70->height < 3 )
            {
                Sys_Printf( "Invalid patch width or height.\n" );
                bB = 0;
            }
            else
                bB = Patch_GenericMesh( 3, v70->height, 2, 0, 1 );
            iassert( bB->patch->def == bB->def->patch );
            patchMesh_t *def2 = bB->patch->def;
            def2->type = (PATCH_TYPES)( def2->type | PATCH_SEAM );
            for ( int row = 0; row < v70->height; ++row )
            {
                def2->ctrl[0][row].xyz[0] = v70->ctrl[lastCol][row].xyz[0];
                def2->ctrl[0][row].xyz[1] = v70->ctrl[lastCol][row].xyz[1];
                def2->ctrl[0][row].xyz[2] = v70->ctrl[lastCol][row].xyz[2];
                def2->ctrl[2][row].xyz[0] = v63->ctrl[lastCol][row].xyz[0];
                def2->ctrl[2][row].xyz[1] = v63->ctrl[lastCol][row].xyz[1];
                def2->ctrl[2][row].xyz[2] = v63->ctrl[lastCol][row].xyz[2];
                def2->ctrl[1][row].xyz[0] = ( def2->ctrl[0][row].xyz[0] + def2->ctrl[2][row].xyz[0] ) * 0.5f;
                def2->ctrl[1][row].xyz[1] = ( def2->ctrl[0][row].xyz[1] + def2->ctrl[2][row].xyz[1] ) * 0.5f;
                def2->ctrl[1][row].xyz[2] = ( def2->ctrl[0][row].xyz[2] + def2->ctrl[2][row].xyz[2] ) * 0.5f;
            }
            Patch_CalcBounds( smins, smaxs, def2, true );
            Brush_RebuildBrush( (brush_t *)def2->pSymbiot, smins, smaxs );
            Patch_Naturalize( def2 );
            Select_Brush( bB, 1, 1, 0 );
            pmesh = v70;
            v4    = v63;
        }

        // ── seam C: width x 3 strip along row 0 (src row0 ↔ dup row0) ──
        selbrush_t *bC = Patch_GenericMesh( pmesh->width, 3, 2, 0, 1 );
        iassert( bC->patch->def == bC->def->patch );
        patchMesh_t *def3 = bC->patch->def;
        def3->type = (PATCH_TYPES)( def3->type | PATCH_SEAM );
        for ( int col = 0; col < pmesh->width; ++col )
        {
            def3->ctrl[col][0].xyz[0] = pmesh->ctrl[col][0].xyz[0];
            def3->ctrl[col][0].xyz[1] = pmesh->ctrl[col][0].xyz[1];
            def3->ctrl[col][0].xyz[2] = pmesh->ctrl[col][0].xyz[2];
            def3->ctrl[col][2].xyz[0] = v4->ctrl[col][0].xyz[0];
            def3->ctrl[col][2].xyz[1] = v4->ctrl[col][0].xyz[1];
            def3->ctrl[col][2].xyz[2] = v4->ctrl[col][0].xyz[2];
            def3->ctrl[col][1].xyz[0] = ( def3->ctrl[col][0].xyz[0] + def3->ctrl[col][2].xyz[0] ) * 0.5f;
            def3->ctrl[col][1].xyz[1] = ( def3->ctrl[col][0].xyz[1] + def3->ctrl[col][2].xyz[1] ) * 0.5f;
            def3->ctrl[col][1].xyz[2] = ( def3->ctrl[col][0].xyz[2] + def3->ctrl[col][2].xyz[2] ) * 0.5f;
        }
        float smins[3], smaxs[3];
        Patch_CalcBounds( smins, smaxs, def3, true );
        Brush_RebuildBrush( (brush_t *)def3->pSymbiot, smins, smaxs );
        Patch_Naturalize( def3 );
        patchInvert2( def3 );
        if ( def3->curveDef )
            free( def3->curveDef );
        def3->curveDef = Patch_GenericMesh2( def3, g_qeglobals.current_edit_layer, 0, 0 );
        ++def3->version;
        Select_Brush( bC, 1, 1, 0 );

        // ── seam D: width x 3 strip along last row (src rowH ↔ dup rowH) ──
        int lastRow = v70->height - 1;
        selbrush_t *bD = Patch_GenericMesh( v70->width, 3, 2, 0, 1 );
        iassert( bD->patch->def == bD->def->patch );
        patchMesh_t *def4 = bD->patch->def;
        def4->type = (PATCH_TYPES)( def4->type | PATCH_SEAM );
        for ( int col = 0; col < v70->width; ++col )
        {
            def4->ctrl[col][0].xyz[0] = v70->ctrl[col][lastRow].xyz[0];
            def4->ctrl[col][0].xyz[1] = v70->ctrl[col][lastRow].xyz[1];
            def4->ctrl[col][0].xyz[2] = v70->ctrl[col][lastRow].xyz[2];
            def4->ctrl[col][2].xyz[0] = v63->ctrl[col][lastRow].xyz[0];
            def4->ctrl[col][2].xyz[1] = v63->ctrl[col][lastRow].xyz[1];
            def4->ctrl[col][2].xyz[2] = v63->ctrl[col][lastRow].xyz[2];
            def4->ctrl[col][1].xyz[0] = ( def4->ctrl[col][0].xyz[0] + def4->ctrl[col][2].xyz[0] ) * 0.5f;
            def4->ctrl[col][1].xyz[1] = ( def4->ctrl[col][0].xyz[1] + def4->ctrl[col][2].xyz[1] ) * 0.5f;
            def4->ctrl[col][1].xyz[2] = ( def4->ctrl[col][0].xyz[2] + def4->ctrl[col][2].xyz[2] ) * 0.5f;
        }
        Patch_CalcBounds( smins, smaxs, def4, true );
        Brush_RebuildBrush( (brush_t *)def4->pSymbiot, smins, smaxs );
        Patch_Naturalize( def4 );
        Select_Brush( bD, 1, 1, 0 );
        v4 = v63;
    }

    // invert the offset copy so its faces point outward, rebuild its curveDef.
    patchInvert2( v4 );
    if ( v4->curveDef )
        free( v4->curveDef );
    v4->curveDef = Patch_GenericMesh2( v4, g_qeglobals.current_edit_layer, 0, 0 );
    ++v4->version;

    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();

    // undo id-stamp tail (identical idiom to PMESH_18).
    if ( g_lastundo && !g_lastundo->done )
    {
        for ( selbrush_t *i = selected_brushes_next; i != &selected_brushes; i = i->next )
        {
            i->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *owner = (entity_s *)(intptr_t)i->def->owner;
            if ( *(int *)&owner->eclass->fixedsize )
                owner->epairEdits = g_lastundo->id;
        }
    }
    Undo_End();
}

// ═════════════════════════════════════════════════════════════════════════════
//  WELD CLUSTER — Curve→Weld (WeldMesh) joins two patches sharing an edge into one.
// ═════════════════════════════════════════════════════════════════════════════

// ── patchInvert (0x446370) — HORIZONTAL FLIP: mirror each row's columns ────────
//   Counterpart to patchInvert2 (which flips vertically).  Swaps ctrl[col][row] with
//   ctrl[width-1-col][row] for col in [0, width/2) over every row; terrain patches then
//   propagate each control vertex's turned_edge bit0 across adjacent columns.
static void patchInvert( patchMesh_t *p )
{
    for ( int row = 0; row < p->height; ++row )
    {
        for ( int c = 0; c < p->width / 2; ++c )
        {
            drawVert_t tmp                 = p->ctrl[c][row];
            p->ctrl[c][row]                = p->ctrl[p->width - 1 - c][row];
            p->ctrl[p->width - 1 - c][row] = tmp;
        }
        if ( ( p->type & PATCH_TERRAIN ) != 0 )
        {
            for ( int c = 0; c < p->width - 1; ++c )
                p->ctrl[c][row].turned_edge ^=
                    ( p->ctrl[c][row].turned_edge ^ p->ctrl[c + 1][row].turned_edge ) & 1;
        }
    }
}

// ── Patch_Weld_EdgeParams (sub_44C1A0, 0x44C1A0) — pick edge-walk start/step ────
//   Given an edge selector (a5=byColumn, a6=atHigh, a7=reverse) fills the start
//   (col=*result, row=*a4) and step (dcol=*a3, drow=*a8) to iterate over one edge of a
//   patch grid `pm` (pm[0]=width, pm[1]=height).  Verbatim from the decompile.
static void Patch_Weld_EdgeParams( int *outStartCol, const int *pm,
                                   int *outStepCol, int *outStartRow,
                                   int a5, int a6, int a7, int *outStepRow )
{
    if ( a5 )
    {
        if ( a6 ) *outStartCol = pm[0] - 1;
        else      *outStartCol = 0;
        *outStepCol = 0;
        if ( a7 )
        {
            *outStartRow = pm[1] - 1;
            *outStepRow  = -1;
        }
        else
        {
            *outStartRow = 0;
            *outStepRow  = 1;
        }
    }
    else
    {
        if ( a7 )
        {
            *outStartCol = pm[0] - 1;
            *outStepCol  = -1;
        }
        else
        {
            *outStartCol = 0;
            *outStepCol  = 1;
        }
        if ( a6 ) *outStartRow = pm[1] - 1;
        else      *outStartRow = 0;
        *outStepRow = 0;
    }
}

// ── PMESH_52_WeldStitch (0x44C280) — stitch pm1 onto pm0 along the matched edge ──
//   pb0Inst = the surviving brush instance (its DEF's patch becomes the merged patch);
//   pb1Inst the consumed one.  a3/a4/a5 carry the {byColumn, atHigh, reverse} state of
//   each patch's matched edge (from Patch_Weld's search).  Transforms both grids into a
//   canonical orientation (transpose/invert as needed), appends pm1's columns onto pm0,
//   restores pm0's orientation, then frees pb1's brush (or its owning entity).
static void PMESH_52_WeldStitch( selbrush_t *pb0Inst, selbrush_t *pb1Inst,
                                 const int *a3, const int *a4, const int *a5 )
{
    iassert( pb1Inst->patch->def == pb1Inst->def->patch );
    // the IDB reads a1 (pb0) as the brush_t DEF: **(a1+32) = def->patch->def == pm0,
    // *(a1+20)+80 = def->def->patch — the same invariant.  Route through the def brush.
    brush_t     *pb0Def = pb0Inst->def;
    patchMesh_t *pm0    = pb0Def->patch;
    patchMesh_t *pm1    = pb1Inst->patch->def;

    int inv0 = 1 - a4[0];                           // v8 = 1 - a4[0]
    if ( a3[0] != 1 )
        Patch_TransposeGrid( pm0 );
    if ( inv0 != 1 )
        patchInvert( pm0 );
    if ( a5[0] )
        patchInvert2( pm0 );

    char didTranspose1 = 0;
    char didInvert1     = 0;
    if ( a3[1] != 1 ) { didTranspose1 = 1; Patch_TransposeGrid( pm1 ); }
    if ( a4[1] != 1 ) { didInvert1    = 1; patchInvert( pm1 ); }
    if ( a5[1] )      { didInvert1    = !didInvert1; patchInvert2( pm1 ); }

    iassert( pm0->height == pm1->height );

    // sanity: pm1's LAST column must coincide with pm0's FIRST column (shared edge).
    for ( int j = 0; j < pm0->height; ++j )
    {
        int miss = 0;
        for ( int k = 0; k < 3; ++k )
        {
            float d = pm1->ctrl[pm1->width - 1][j].xyz[k] - pm0->ctrl[0][j].xyz[k];
            if ( d * d > 0.00000100000011116208f )
                break;
            ++miss;
        }
        if ( miss < 3 )
            Assert( PMESH_CPP, 8665, 0, "%s",
                    "Vec3CompareEpsilon(pm1->ctrl[pm1->width - 1][j].qv.xyz, pm0->ctrl[0][j].qv.xyz)" );
    }

    // append pm1's columns 1..width-1 after pm0's existing columns (col0 is the shared
    // edge).  First OR pm1 col0's turned_edge bit0 into pm0's LAST column edge bit.
    for ( int j = 0; j < pm0->height; ++j )
    {
        drawVert_t *pm0Edge = &pm0->ctrl[pm0->width - 1][j];
        pm0Edge->turned_edge &= ~1;
        pm0Edge->turned_edge |= ( pm1->ctrl[0][j].turned_edge & 1 );
        for ( int i = 1; i < pm1->width; ++i )
            pm0->ctrl[pm0->width - 1 + i][j] = pm1->ctrl[i][j];
    }
    pm0->width += pm1->width - 1;

    if ( didTranspose1 )
        Patch_TransposeGrid( pm0 );
    if ( didInvert1 )
    {
        patchInvert2( pm0 );
        if ( pm0->curveDef )
            free( pm0->curveDef );
        pm0->curveDef = Patch_GenericMesh2( pm0, g_qeglobals.current_edit_layer, 0, 0 );
        ++pm0->version;
    }
    Patch_Rebuild( pm0, 1 );

    entity_s *owner = pb1Inst->owner;
    if ( owner != world_entity
      && owner->brushes.ownerNext == pb1Inst
      && owner->brushes.ownerPrev == pb1Inst )
    {
        Undo_AddEntity_W( (entity_s *)owner->def );
        Entity_Free( (char *)pb1Inst->owner );
    }
    else
    {
        Brush_Free( pb1Inst );
    }
}

// ── Patch_Weld (0x44C590) — try to weld patch instance a1 onto a2 ─────────────
//   Searches all 2x2x2 edge orientations of both patches for a coincident edge (same
//   length + coincident control points).  If EXACTLY ONE edge pair matches (more than
//   one → refuse), and the merged edge count fits in MAX_PATCH_WIDTH (16), stitches
//   them via PMESH_52_WeldStitch.  Returns 1 on a successful weld, 0 otherwise.
static char Patch_Weld( selbrush_t *a1, selbrush_t *a2 )
{
    if ( !a1 ) Assert( PMESH_CPP, 8701, 0, "%s", "pb0" );
    if ( !a2 ) Assert( PMESH_CPP, 8702, 0, "%s", "pb1" );
    if ( a1 == a2 ) Assert( PMESH_CPP, 8703, 0, "%s", "pb0 != pb1" );
    if ( a1->patch->def != a1->def->patch )
        Assert( PMESH_CPP, 8731, 0, "%s", "pb0->patch->def == pb0->def->patch" );
    if ( a2->patch->def != a2->def->patch )
        Assert( PMESH_CPP, 8732, 0, "%s", "pb1->patch->def == pb1->def->patch" );

    patchMesh_t *pm0 = a1->patch->def;              // def / v34
    patchMesh_t *pm1 = a2->patch->def;              // v5  / v35

    char foundMatch = 0;                            // v38
    int  m16[2], m17[2], m18[2];                    // matched orientation params

    for ( int e0 = 0; e0 < 2; ++e0 )                // v30: byColumn of pm0's edge
    {
        int len0 = e0 ? pm0->height : pm0->width;   // height along e0's edge
        for ( int e1 = 0; e1 < 2; ++e1 )            // v31: byColumn of pm1's edge
        {
            int len1 = e1 ? pm1->height : pm1->width;
            if ( len0 != len1 )
                continue;

            // try the two mirror orientations (v32) of pm0's edge and (v33) of pm1's.
            for ( int f0 = 0; f0 < 2; ++f0 )        // v32
            {
                int startCol0, stepCol0, startRow0, stepRow0;
                Patch_Weld_EdgeParams( &startCol0, (int *)pm0, &stepCol0, &startRow0,
                                       e0, f0, 0, &stepRow0 );

                for ( int f1 = 0; f1 < 2; ++f1 )    // v33
                {
                    for ( int rev = 0; rev < 2; ++rev )   // v27: reverse pm1's walk
                    {
                        int startCol1, stepCol1, startRow1, stepRow1;
                        Patch_Weld_EdgeParams( &startCol1, (int *)pm1, &stepCol1, &startRow1,
                                               e1, f1, rev, &stepRow1 );

                        // walk both edges in lockstep, comparing each control point.
                        int c0 = startCol0, r0 = startRow0;
                        int c1 = startCol1, r1 = startRow1;
                        int matched = 0;
                        int i;
                        for ( i = 0; i < len0; ++i )
                        {
                            int k;
                            for ( k = 0; k < 3; ++k )
                            {
                                float d = pm0->ctrl[c0][r0].xyz[k]
                                        - pm1->ctrl[c1][r1].xyz[k];
                                if ( d * d > 0.00000100000011116208f )
                                    break;
                            }
                            if ( k < 3 )
                                break;              // this point differed → edge mismatch
                            c0 += stepCol0; r0 += stepRow0;
                            c1 += stepCol1; r1 += stepRow1;
                            ++matched;
                        }
                        if ( matched != len0 )
                            continue;               // whole edge did not coincide

                        // edge coincides.  Refuse if a second match was already found.
                        if ( foundMatch )
                        {
                            Sys_Printf( "Cannot weld patches; more than one edge matches\n" );
                            return 0;
                        }
                        // new merged edge count must fit MAX_PATCH_WIDTH (16).
                        int newEdge = pm1->width + pm0->height + pm1->height
                                    - 2 * len0 + pm0->width - 1;
                        if ( newEdge > 16 )
                        {
                            Sys_Printf( "Cannot weld patches; new edge size of %i is bigger than %i\n",
                                        newEdge, 16 );
                            return 0;
                        }
                        m16[0] = e0;  m16[1] = e1;
                        m17[0] = f0;  m17[1] = f1;
                        m18[0] = 0;   m18[1] = rev;
                        foundMatch = 1;
                    }
                }
            }
        }
    }

    if ( !foundMatch )
        return 0;
    PMESH_52_WeldStitch( a2, a1, m16, m17, m18 );
    return 1;
}

// ── WeldMesh (0x44C8C0) — Curve→Weld: weld all adjacent selected patches ───────
//   Every selected brush must be a patch, and they must all belong to the same
//   worldspawn-ish owner (or all be worldspawn func_groups).  Repeatedly welds any two
//   selected patches sharing an edge until no more welds occur.

char WeldMesh()
{
    entity_s *commonOwner = 0;                     // v11
    char sawGroup = 0;                             // v13 (worldspawn-ish)
    char sawEntity = 0;                            // v12 (non-worldspawn)

    // validate the selection: all patches, consistent ownership.
    selbrush_t *v0 = selected_brushes_next;
    if ( v0 != &selected_brushes )
    {
        while ( v0->patch )
        {
            const char *value = (const char *)zero;
            epair_t *epairs = ((entity_s *)v0->owner->def)->epairs;
            if ( epairs )
            {
                while ( _stricmp( epairs->key, "classname" ) )
                {
                    epairs = epairs->next;
                    if ( !epairs ) { value = (const char *)zero; goto haveValue; }
                }
                value = epairs->value;
            }
        haveValue:
            if ( PMESH_IsWeldContainer( value ) )             // worldspawn/func_group/func_cullgroup
            {
                if ( sawEntity )
                    return 0;
                sawGroup = 1;
            }
            else
            {
                if ( sawGroup || ( commonOwner && commonOwner != v0->owner ) )
                    return 0;
                sawEntity = 1;
            }
            entity_s *o = v0->owner;
            v0 = v0->next;
            commonOwner = o;
            if ( v0 == &selected_brushes )
                goto doWeld;
        }
        return 0;                                  // a non-patch was selected
    }

doWeld:
    Undo_ClearRedo();
    Undo_GeneralStart( "weld meshes" );
    Undo_AddBrushList( &selected_brushes );

    // O(n^2) pairwise weld: for each i, try to weld a later `next` onto it.  On a
    // successful weld the binary restarts the whole outer scan (goto LABEL_17), because
    // the merged patch changed the list; we mirror that with a restart flag.
restartWeld:
    for ( selbrush_t *i = selected_brushes_next; i != &selected_brushes; i = i->next )
    {
        selbrush_t *next = i->next;
        if ( next == &selected_brushes )
            continue;
        while ( ( ( (unsigned char)i->def->patch->type
                    ^ (unsigned char)next->def->patch->type ) & 0x40 ) != 0
                || !Patch_Weld( next, i ) )
        {
            next = next->next;
            if ( next == &selected_brushes )
                goto nextI;                        // LABEL_23: this i had no match
        }
        goto restartWeld;                          // welded → restart the outer scan
    nextI:;
    }

    if ( g_lastundo && !g_lastundo->done )
    {
        for ( selbrush_t *j = selected_brushes_next; j != &selected_brushes; j = j->next )
        {
            j->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *owner = (entity_s *)(intptr_t)j->def->owner;
            if ( *(int *)&owner->eclass->fixedsize )
                owner->epairEdits = g_lastundo->id;
        }
    }
    Undo_End();
    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
    g_nUpdateBits = -1;
    return 1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  SPLIT CLUSTER — Curve→Split (SplitPatch) splits a patch at a selected mid-edge.
// ═════════════════════════════════════════════════════════════════════════════

// ── PMESH_08_Verts_RowCol_02 (0x43BFE0) — resolve the selected verts' row/col ──
//   Scans g_qeglobals.d_move_points for control points that belong to `a1`, returns
//   their common column (*px) and row (*py) — one of which is a real index and the other
//   -1 (the selection lies on a single row or a single column).  0 if the selection
//   is empty, spans both axes, or none of the points belong to this patch.
static char PMESH_08_Verts_RowCol_02( patchMesh_t *a1, int *px, int *py )
{
    if ( !a1 ) Assert( PMESH_CPP, 1935, 0, "%s", "patch" );
    if ( !px ) Assert( PMESH_CPP, 1936, 0, "%s", "px" );
    if ( !py ) Assert( PMESH_CPP, 1937, 0, "%s", "py" );

    *px = -1;
    *py = -1;
    if ( g_qeglobals.d_num_move_points <= 0 )
        return 0;

    char have = 0;
    for ( int idx = 0; idx < g_qeglobals.d_num_move_points; ++idx )
    {
        drawVert_t *dv = (drawVert_t *)g_qeglobals.d_move_points[idx];
        // in-grid test: dv within [&ctrl[0][0], &ctrl[15][15]].
        if ( dv >= &a1->ctrl[0][0] && dv <= &a1->ctrl[15][15] )
        {
            int flat = (int)( (char *)dv - (char *)a1 - 56 ) / 80;
            int col  = flat / 16;
            int row  = flat % 16;
            if ( dv != (drawVert_t *)( (char *)a1->ctrl + 80 * flat ) )
                Assert( PMESH_CPP, 1951, 0, "%s", "pdv == &patch->ctrl[x][y]" );
            if ( have )
            {
                if ( *px != col ) *px = -1;
                if ( *py != row ) *py = -1;
            }
            else
            {
                *px  = col;
                *py  = row;
                have = 1;
            }
        }
    }
    if ( !have )
        return 0;
    if ( *px < 0 )
    {
        if ( *py < 0 )
        {
            Sys_Printf( "selected vertices must be on the same column or row\n" );
            return 0;
        }
    }
    else if ( *py >= 0 )
        return 0;
    return 1;
}

// ── PMESH_53_Weld_MakeNew (0x44CA70) — split off a sub-patch at (col,row) ──────
//   Copies the control-grid region from (col,row)..(width,height) of `a1` into a new
//   patch (reusing a1's materials/type), adds a symbiont brush owned by owner, then
//   trims a1 to end at the split (width=col+1 OR height=row+1).  Exactly one of
//   col/row is nonzero (the other must be 0 — the split axis).
static void PMESH_53_Weld_MakeNew( patchMesh_t *a1, int col, int row, entity_s *owner )
{
    if ( col && row )
        Assert( PMESH_CPP, 8876, 0, "%s", "col == 0 || row == 0" );

    patchMesh_t *NewPatch = MakeNewPatch();
    // copy the 3 material channels ({lyrMtl, radMtl} pairs) from a1.
    NewPatch->texture   = a1->texture;
    NewPatch->lightmap  = a1->lightmap;
    NewPatch->smoothing = a1->smoothing;

    NewPatch->width               = a1->width  - col;
    NewPatch->height              = a1->height - row;
    NewPatch->contents            = a1->contents;
    NewPatch->type                = a1->type;
    NewPatch->subDivType          = a1->subDivType;
    NewPatch->size_of_struct_0x504C = a1->size_of_struct_0x504C;
    NewPatch->bDirty              = a1->bDirty;

    for ( int c = col; c < a1->width; ++c )
        for ( int r = row; r < a1->height; ++r )
            NewPatch->ctrl[c - col][r - row] = a1->ctrl[c][r];

    brush_t *nb = AddBrushForPatch( NewPatch, (entity_s *)owner->def );
    Patch_Rebuild( NewPatch, 1 );
    if ( nb->owner != (entity_s *)a1->pSymbiot->owner )
        Assert( PMESH_CPP, 8895, 0, "%s", "pNewBrush->owner == pm->symbiot->owner" );
    if ( nb->owner != (entity_s *)owner->def )
        Assert( PMESH_CPP, 8896, 0, "%s", "pNewBrush->owner == owner->def" );

    selbrush_t *v13 = (selbrush_t *)Brush_AddToList( nb, owner );
    if ( v13->next || v13->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    Brush_AddToList2( v13 );

    if ( col )
        a1->width = col + 1;
    else
        a1->height = row + 1;
    Patch_Rebuild( a1, 1 );
}

// ── SplitPatch (0x44CC60) — Curve→Split: split selected patches at a mid-edge ──
//   For each selected patch with verts selected on a mid row/column (odd index for a
//   non-terrain patch), split it into two patches at that seam (PMESH_53_Weld_MakeNew),
//   then restart the scan.  Reached via OnSplitPatch (cmd 33158).
void SplitPatch()
{
    Undo_ClearRedo();
    Undo_GeneralStart( "split patches" );
    Undo_AddBrushList( &selected_brushes );

    char didSplit = 0;
restartSplit:
    for ( selbrush_t *i = selected_brushes_next; i != &selected_brushes; i = i->next )
    {
        patch_t *patch = i->patch;
        if ( !patch )
            continue;
        if ( patch->def != i->def->patch )
            Assert( PMESH_CPP, 8929, 0, "%s", "pb->patch->def == pb->def->patch" );
        patchMesh_t *def = i->patch->def;
        int selCol, selRow;
        if ( !PMESH_08_Verts_RowCol_02( def, &selCol, &selRow ) )
            continue;
        if ( selCol < 0 )
        {
            // split along a ROW (selRow); must be an interior, splittable row.
            if ( selRow && selRow != def->height - 1 )
            {
                if ( ( def->type & 0x40 ) != 0 || ( selRow & 1 ) == 0 )
                {
                    PMESH_53_Weld_MakeNew( def, 0, selRow, (entity_s *)(intptr_t)i->owner );
                    didSplit = 1;
                    goto restartSplit;
                }
                Sys_Printf( "can only split curves on odd rows and columns" );
            }
        }
        else if ( selCol && selCol != def->width - 1 )
        {
            if ( ( def->type & 0x40 ) != 0 || ( selCol & 1 ) == 0 )
            {
                PMESH_53_Weld_MakeNew( def, selCol, 0, (entity_s *)(intptr_t)i->owner );
                didSplit = 1;
                goto restartSplit;
            }
            Sys_Printf( "can only split curves on odd rows and columns" );
        }
    }

    if ( g_lastundo && !g_lastundo->done )
    {
        for ( selbrush_t *j = selected_brushes_next; j != &selected_brushes; j = j->next )
        {
            j->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *owner = (entity_s *)(intptr_t)j->def->owner;
            if ( *(int *)&owner->eclass->fixedsize )
                owner->epairEdits = g_lastundo->id;
        }
    }
    Undo_End();
    if ( didSplit )
    {
        if ( g_PatchDialog_GetHwnd() )
            g_PatchDialog_GetPatchInfo();
        g_nUpdateBits = -1;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  TERRAIN ROW/COLUMN OPERATION CLUSTER
// ═════════════════════════════════════════════════════════════════════════════

// terrain single-row/col helpers already exist (Patch_TerrainInsert/RemoveRow/Column).
// Deps declared elsewhere: PMESH_08_Verts_RowCol_02 (above), Patch_Rebuild, etc.

extern int   TexWnd_06_LayerCount( int mtlDef, int layerHandle );   // brush.cpp 0x45D360
extern float grid_sizes[];                                          // engine_stubs 0x6DDE5C
extern selbrush_t active_brushes;                                   // 0x23F189C
#define active_brushes_next ( active_brushes.next )
extern bool  HasKeyValuePair( entity_s_def *e, const char *key );          // entity.cpp
extern char *ValueForKey2( int e, const char *key );                       // entity.cpp
extern bool  Entity_HasEpairMatch( entity_s *e, const char *key, const char *val ); // entity.cpp
extern void  DeleteKey( epair_t **head, const char *key );                 // entity.cpp
extern void  Checkkey_Model( entity_s_def *e, const char *key );           // entity.cpp (Checkkey_Model_0)
extern void  Checkkey_Color( entity_s_def *a1, const char *a2 );           // entity.cpp
extern void  Undo_AddEntity( int a1 );                                     // undo.cpp 0x45E8B0
extern void  Undo_AddBrush( entity_brush_s *pBrushInst );                  // undo.cpp 0x45E680
extern void  Brush_SnapToGrid( brush_t *b );                               // brush.cpp 0x4783D0

// ── PMESH_08_Verts_RowCol (0x43BE70) — resolve a selected-vertex PAIR to a row/col ─
//   Finds the (up to two) selected control points that belong to `patch`; fills the
//   pair struct pPair = {int x[2]; int y[2]} with their (col,row).  Returns 1 iff
//   exactly two points were found on the same row OR the same column.
static char PMESH_08_Verts_RowCol( int patch, char *pPair )
{
    if ( !patch ) Assert( PMESH_CPP, 1887, 0, "%s", "patch" );
    if ( !pPair ) Assert( PMESH_CPP, 1888, 0, "%s", "pPair" );

    drawVert_t *found[2] = { 0, 0 };
    if ( g_qeglobals.d_num_move_points <= 0 )
        return 0;
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        drawVert_t *dv = g_qeglobals.d_move_points[i];
        if ( (unsigned int)dv >= (unsigned int)( patch + 56 )
          && (unsigned int)dv <= (unsigned int)( patch + 20456 ) )
        {
            if ( found[0] )
            {
                if ( found[1] )
                    return 0;               // more than two → ambiguous
                found[1] = dv;
            }
            else
                found[0] = dv;
        }
    }
    if ( !found[1] )
        return 0;

    int *pair = (int *)pPair;               // pPair->x[0], x[1], y[0], y[1] laid out {x0,x1,y0,y1}
    for ( int i = 0; i < 2; ++i )
    {
        int flat = ( (int)(intptr_t)found[i] - patch - 56 ) / 80;
        pair[i]     = flat / 16;            // x[i] = col
        pair[i + 2] = flat % 16;            // y[i] = row
        if ( (int)(intptr_t)found[i]
             != 80 * ( flat % 16 + 16 * ( flat / 16 ) ) + patch + 56 )
            Assert( PMESH_CPP, 1914, 0, "%s",
                    "pdvFound[i] == &patch->ctrl[pPair->x[i]][pPair->y[i]]" );
    }
    if ( pair[0] == pair[1] || pair[2] == pair[3] )   // same col OR same row
        return 1;
    Sys_Printf( "selected vertices must be on the same column or row\n" );
    return 0;
}

// ── Patch_TerrainExtrudeColumn (sub_442DE0, 0x442DE0) — extrapolate a new column ─
//   Extends a terrain patch by ONE column at (col a2), mirroring the edge:
//   new = 2*edge - inner.  Adds each new control point to the d_points overlay so it can
//   be picked.  a1 = patchMesh_t (a1[0]=width, a1[1]=height).  Verbatim from the disasm
//   (float stride 20 = one row (80B), 320 = one column (1280B)).
static void Patch_TerrainExtrudeColumn( patchMesh_t *a1, int a2 )
{
    int width = a1->width;
    if ( width + 1 > 16 )
        return;

    if ( (double)width * 0.5 < (double)a2 || a2 == width - 1 )
    {
        // extrude at the HIGH edge: new column at `width` = 2*col[width-1] - col[width-2].
        for ( int row = 0; row < a1->height; ++row )
        {
            a1->ctrl[width][row] = a1->ctrl[width - 1][row];  // copy header/materials
            for ( int k = 0; k < 3; ++k )
                a1->ctrl[width][row].xyz[k] =
                    a1->ctrl[width - 1][row].xyz[k] - a1->ctrl[width - 2][row].xyz[k]
                  + a1->ctrl[width - 1][row].xyz[k];
            for ( int k = 0; k < 3; ++k )
                g_qeglobals.d_points[g_qeglobals.d_numpoints][k] = a1->ctrl[width][row].xyz[k];
            if ( g_qeglobals.d_numpoints < 2047 )
                ++g_qeglobals.d_numpoints;
        }
        ++a1->width;
    }
    else
    {
        // extrude at the LOW edge: shift all columns right, new column 0 = 2*col0-col1.
        for ( int c = width; c > 0; --c )
            for ( int row = 0; row < a1->height; ++row )
                a1->ctrl[c][row] = a1->ctrl[c - 1][row];
        for ( int row = 0; row < a1->height; ++row )
        {
            for ( int k = 0; k < 3; ++k )
                a1->ctrl[0][row].xyz[k] =
                    a1->ctrl[1][row].xyz[k] - a1->ctrl[2][row].xyz[k]
                  + a1->ctrl[1][row].xyz[k];
            for ( int k = 0; k < 3; ++k )
                g_qeglobals.d_points[g_qeglobals.d_numpoints][k] = a1->ctrl[0][row].xyz[k];
            if ( g_qeglobals.d_numpoints < 2047 )
                ++g_qeglobals.d_numpoints;
        }
        ++a1->width;
    }
}

// ── Patch_TerrainExtrudeRow (sub_443040, 0x443040) — extrapolate a new row ──────
//   Extends a terrain patch by ONE row at (row a2), mirroring the edge: new = 2*edge -
//   inner.  a1[0]=width, a1[1]=height.  (float stride 20 = row, 320 = column.)
static void Patch_TerrainExtrudeRow( patchMesh_t *a1, int a2 )
{
    int height = a1->height;
    if ( height + 1 > 16 )
        return;

    if ( (double)height * 0.5 < (double)a2 || a2 == height - 1 )
    {
        // extrude at the HIGH edge: new row at `height` = 2*row[height-1] - row[height-2].
        for ( int col = 0; col < a1->width; ++col )
        {
            a1->ctrl[col][height] = a1->ctrl[col][height - 1];
            for ( int k = 0; k < 3; ++k )
                a1->ctrl[col][height].xyz[k] =
                    a1->ctrl[col][height - 1].xyz[k] - a1->ctrl[col][height - 2].xyz[k]
                  + a1->ctrl[col][height - 1].xyz[k];
            for ( int k = 0; k < 3; ++k )
                g_qeglobals.d_points[g_qeglobals.d_numpoints][k] = a1->ctrl[col][height].xyz[k];
            if ( g_qeglobals.d_numpoints < 2047 )
                ++g_qeglobals.d_numpoints;
        }
        ++a1->height;
    }
    else
    {
        // extrude at the LOW edge: shift all rows up, new row 0 = 2*row0 - row1.
        for ( int r = height; r > 0; --r )
            for ( int col = 0; col < a1->width; ++col )
                a1->ctrl[col][r] = a1->ctrl[col][r - 1];
        for ( int col = 0; col < a1->width; ++col )
        {
            for ( int k = 0; k < 3; ++k )
                a1->ctrl[col][0].xyz[k] =
                    a1->ctrl[col][1].xyz[k] - a1->ctrl[col][2].xyz[k]
                  + a1->ctrl[col][1].xyz[k];
            for ( int k = 0; k < 3; ++k )
                g_qeglobals.d_points[g_qeglobals.d_numpoints][k] = a1->ctrl[col][0].xyz[k];
            if ( g_qeglobals.d_numpoints < 2047 )
                ++g_qeglobals.d_numpoints;
        }
        ++a1->height;
    }
}

// ── ExtrudeTerrainRow (0x44BD40) — cmd 33192: extrude a terrain edge outward ────
//   For each selected terrain patch with a full row/column of verts selected, extrude a
//   new edge (row or column) by extrapolation.
void ExtrudeTerrainRow()
{
    char didEdit = 0;
    selbrush_t *v0 = selected_brushes_next;
    if ( v0 == &selected_brushes )
        return;
    do
    {
        patch_t *patch = v0->patch;
        if ( patch )
        {
            if ( patch->def != v0->def->patch )
                Assert( PMESH_CPP, 8435, 0, "%s", "b->patch->def == b->def->patch" );
            patchMesh_t *def = v0->patch->def;
            if ( ( def->type & PATCH_TERRAIN ) != 0 )
            {
                int px, py;
                if ( PMESH_08_Verts_RowCol_02( def, &px, &py ) )
                {
                    if ( px < 0 )
                        Patch_TerrainExtrudeRow( def, py );
                    else
                        Patch_TerrainExtrudeColumn( def, px );
                    didEdit = 1;
                    Patch_Rebuild( def, 1 );
                }
            }
        }
        v0 = v0->next;
    } while ( v0 != &selected_brushes );

    if ( didEdit )
    {
        if ( g_PatchDialog_GetHwnd() )
            g_PatchDialog_GetPatchInfo();
        g_nUpdateBits = -1;
    }
}

// ── RemoveTerrainRowCol (0x44BE10) — cmd 33154: remove a terrain row/column ─────
//   For each selected terrain patch with a full row/column of verts selected, remove that
//   row/column.  If NO patch is selected but a single-brush entity with a targetname is,
//   the binary re-routes targets and deletes the entity (the "remove terrain entity"
//   fallback) — that whole entity-delete sub-path is ported verbatim below.
void RemoveTerrainRowCol()
{
    char didEdit = 0;
    selbrush_t *v0 = selected_brushes_next;
    selbrush_t *next = v0;
    if ( v0 == &selected_brushes )
        return;

    while ( 1 )
    {
        patch_t *patch = v0->patch;
        if ( patch )
        {
            if ( patch->def != v0->def->patch )
                Assert( PMESH_CPP, 8538, 0, "%s", "b->patch->def == b->def->patch" );
            patchMesh_t *def = v0->patch->def;
            if ( ( def->type & PATCH_TERRAIN ) != 0 )
            {
                int px, py;
                if ( PMESH_08_Verts_RowCol_02( def, &px, &py ) )
                {
                    if ( px < 0 )
                        Patch_TerrainRemoveRow( def, py );
                    else
                        Patch_TerrainRemoveColumn( def, px );
                    didEdit = 1;
                    Patch_Rebuild( def, 1 );
                    v0 = next;
                }
            }
        }
        else if ( selected_brushes_next->next == &selected_brushes
               && HasKeyValuePair( (entity_s_def *)v0->owner->def, "targetname" ) )
        {
            // No patch: a single-brush entity with a targetname → delete it (re-routing
            // its target chain).  Faithful port of the 0x44BF36.. entity-delete fallback.
            const char *value;
            epair_t *epairs = ((entity_s *)v0->owner->def)->epairs;
            if ( epairs )
            {
                while ( _stricmp( epairs->key, "targetname" ) )
                {
                    epairs = epairs->next;
                    if ( !epairs ) { value = (const char *)zero; goto haveTargetname; }
                }
                value = epairs->value;
            }
            else
                value = (const char *)zero;
        haveTargetname:;
            entity_s_def *entDef = (entity_s_def *)v0->owner->def;
            void *newTargetName = 0;                 // IDB `i` (holds the ValueForKey2 result)
            if ( HasKeyValuePair( entDef, "target" ) )
            {
                // this entity has a "target": push its target onto everything that
                // targeted it (re-route the chain through us).
                newTargetName = ValueForKey2( (int)(intptr_t)entDef, "target" );
                for ( selbrush_t *ab = active_brushes_next; ab != &active_brushes; ab = ab->next )
                {
                    entity_s_def *abDef = (entity_s_def *)ab->owner->def;
                    if ( Entity_HasEpairMatch( (entity_s *)abDef, "target", value ) )
                        SetKeyValue( abDef, "target", (const char *)newTargetName );
                }
            }
            else
            {
                // no target: clear the "target" of everything that targeted us.
                for ( selbrush_t *ab = active_brushes_next; ab != &active_brushes; ab = ab->next )
                {
                    entity_s_def *abDef = (entity_s_def *)ab->owner->def;
                    if ( Entity_HasEpairMatch( (entity_s *)abDef, "target", value ) )
                    {
                        DeleteKey( &abDef->epairs, "target" );
                        Checkkey_Model( abDef, "target" );
                        Checkkey_Color( abDef, "target" );
                    }
                }
                newTargetName = 0;
            }

            Undo_ClearRedo();
            Undo_GeneralStart( "delete" );
            Undo_AddBrushList( &selected_brushes );
            for ( selbrush_t *j = selected_brushes_next; j != &selected_brushes; j = j->next )
            {
                entity_s *owner = j->owner;
                entity_s_def *ownerDef = (entity_s_def *)owner->def;
                if ( g_lastundo )
                {
                    Undo_AddEntity( (int)(intptr_t)owner->def );
                    if ( ownerDef != (entity_s_def *)world_entity->def )
                    {
                        for ( entity_brush_s *ob = (entity_brush_s *)ownerDef->brushes.ownerPrev;
                              ob != (entity_brush_s *)&ownerDef->brushes;   // sentinel = &def->brushes
                              ob = (entity_brush_s *)ob->ownerNext )
                            Undo_AddBrush( ob );
                    }
                }
                else
                    Sys_Printf( "Undo_AddEntity: no last undo.\n" );
            }
            Select_Delete();
            if ( g_lastundo && !g_lastundo->done )
            {
                for ( selbrush_t *m = selected_brushes_next; m != &selected_brushes; m = m->next )
                {
                    m->def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
                    entity_s *o = (entity_s *)(intptr_t)m->def->owner;
                    if ( *(int *)&o->eclass->fixedsize )
                        o->epairEdits = g_lastundo->id;
                }
            }
            Undo_End();
            if ( g_PatchDialog_GetHwnd() )
                g_PatchDialog_GetPatchInfo();

            // re-select everything that carries the pushed targetname (the new head).
            const char *tn = (const char *)newTargetName;
            if ( tn )
            {
                for ( selbrush_t *ab = active_brushes_next; ab != &active_brushes; ab = ab->next )
                {
                    if ( Entity_HasEpairMatch( (entity_s *)ab->owner->def, "targetname", tn ) )
                        Select_Brush( ab, 1, 1, 0 );
                }
            }
            g_nUpdateBits = -1;
            return;
        }
        next = v0->next;
        if ( next == &selected_brushes )
            break;
        v0 = v0->next;
    }

    if ( !didEdit )
        return;
    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
    g_nUpdateBits = -1;
}

// ── Patch_InsertRemoveFromVertPair (0x44BB90) — insert a terrain row/col at a pair ─
//   From a selected pair of ADJACENT terrain verts (same row/col, adjacent index),
//   insert a new row or column between them.  Parked in the IDB port comment as
//   sel_vertex-dependent; the vertex-pair path (PMESH_08_Verts_RowCol) is now ported.
void Patch_InsertRemoveFromVertPair()
{
    char didEdit = 0;
    selbrush_t *v0 = selected_brushes_next;
    selbrush_t *next = v0;
    if ( v0 == &selected_brushes )
        return;

    patchMesh_t *def = 0;
    int pair[4] = { 0, 0, 0, 0 };       // {x0, x1, y0, y1}
    while ( 1 )
    {
        patch_t *patch = v0->patch;
        if ( patch )
        {
            if ( patch->def != v0->def->patch )
                Assert( PMESH_CPP, 8379, 0, "%s", "b->patch->def == b->def->patch" );
            def = v0->patch->def;
            if ( ( def->type & PATCH_TERRAIN ) != 0
              && PMESH_08_Verts_RowCol( (int)(intptr_t)def, (char *)pair ) )
                break;                  // found a valid pair → do the insert below
        }
        next = v0->next;
        if ( next == &selected_brushes )
        {
            if ( didEdit )
            {
                if ( g_PatchDialog_GetHwnd() )
                    g_PatchDialog_GetPatchInfo();
                g_nUpdateBits = -1;
            }
            return;
        }
        v0 = v0->next;
    }

    // pair {x0,x1,y0,y1}: insert between the two adjacent verts.
    int x0 = pair[0], x1 = pair[1], y0 = pair[2], y1 = pair[3];
    if ( x0 == x1 )                     // same column → the pair differs in row → insert a ROW
    {
        if ( y0 == y1 + 1 )
            Patch_TerrainInsertRow( def, y0 );
        else if ( y1 == y0 + 1 )
            Patch_TerrainInsertRow( def, y1 );
        else
            Sys_Printf( "vertices must be adjacent to insert a row or column\n" );
    }
    else                               // differ in column → insert a COLUMN
    {
        if ( y0 != y1 )
            Assert( PMESH_CPP, 8397, 0, "%s", "pair.y[0] == pair.y[1]" );
        if ( x0 == x1 + 1 )
            Patch_TerrainInsertColumn( def, x0 );
        else if ( x1 == x0 + 1 )
            Patch_TerrainInsertColumn( def, x1 );
        else
            Sys_Printf( "vertices must be adjacent to insert a row or column\n" );
    }
    // rebuild the tessellated curveDef.
    if ( def->curveDef )
        free( def->curveDef );
    def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );
    ++def->version;

    if ( g_PatchDialog_GetHwnd() )
        g_PatchDialog_GetPatchInfo();
    g_nUpdateBits = -1;
}

// ── PMESH_58 (0x44D1F0) — Face→Terrain: convert one brush face to a terrain patch ─
//   a1 = the face_t*, a2 = the owning brush instance (selbrush_t*).  Builds a 2-row
//   terrain patch of width (winding->numpoints+1)/2 from the face's winding, projecting
//   each control point's texcoords through the face's 3 layer texture matrices.  Returns
//   the new brush instance (linked into active_brushes), or 0 if the strip would exceed
//   MAX_PATCH_WIDTH.  Reached via OnFaceToTerrain (cmd 36102).  Transcribed from disasm.
extern void Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                              int uvBase, float rotate, float crossterm );  // brush.cpp 0x45A1C0
brush_t *PMESH_58( face_t *face, selbrush_t *ownerInst )
{
    // build the 3 layer texture-projection matrices (8 floats each) from the face's
    // mtldef + plane normal (the same call Face_BuildLayerGeom uses).
    float mats[24];                             // 3 layers * 8 floats
    for ( int L = 0; L < 3; ++L )
    {
        float *td = (float *)TexWnd_06_LayerCount( (int)(intptr_t)&face->mtldef[L], 0 );
        Face_MoveTexture( (int)(intptr_t)td, face->plane.normal,
                          (int)(intptr_t)&mats[8 * L], (int)(intptr_t)( td + 2 ), td[4], td[5] );
    }

    winding_t *w = face->w;
    int numpoints = w->numpoints;
    int newWidth  = (unsigned int)( numpoints + 1 ) >> 1;
    if ( (unsigned int)newWidth > 16 )
        return 0;

    patchMesh_t *NewPatch = MakeNewPatch();
    NewPatch->width  = newWidth;
    NewPatch->height = 2;
    NewPatch->type   = PATCH_TERRAIN;

    // Row 0: walk winding points backward from the last (index numpoints-1); each maps to
    // ctrl[col][0].  Row 1: continue backward into ctrl[col][1] (cols width-1..0).
    int backIdx = numpoints - 1;                // var_C

    for ( int col = 0; col < NewPatch->width; ++col )
    {
        const float *pt = w->p[backIdx];
        drawVert_t *cp = &NewPatch->ctrl[col][0];
        cp->xyz[0] = pt[0];
        cp->xyz[1] = pt[1];
        cp->xyz[2] = pt[2];
        for ( int L = 0; L < 3; ++L )
        {
            const float *m = &mats[8 * L];
            float s = pt[1] * m[1] + pt[0] * m[0] + pt[2] * m[2] + m[3];
            float t = pt[1] * m[5] + pt[0] * m[4] + pt[2] * m[6] + m[7];
            if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 9122, 0, "%s", "!IS_NAN(texCoord[0])" );
            if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                Assert( PMESH_CPP, 9123, 0, "%s", "!IS_NAN(texCoord[1])" );
            cp->texCoord.st[2 * L]     = s;
            cp->texCoord.st[2 * L + 1] = t;
        }
        cp->normal[0]           = face->plane.normal[0];
        cp->normal[1]           = face->plane.normal[1];
        cp->normal[2]           = face->plane.normal[2];
        *(int *)&cp->vert_color = *(int *)&face->plane.dist;   // IDB a1+228 (dword after normal)
        --backIdx;
    }

    if ( NewPatch->width - 1 > -1 )
    {
        for ( int col = NewPatch->width - 1; col >= 0; --col )
        {
            const float *pt = w->p[backIdx];
            drawVert_t *cp = &NewPatch->ctrl[col][1];
            cp->xyz[0] = pt[0];
            cp->xyz[1] = pt[1];
            cp->xyz[2] = pt[2];
            for ( int L = 0; L < 3; ++L )
            {
                const float *m = &mats[8 * L];
                float s = pt[1] * m[1] + pt[0] * m[0] + pt[2] * m[2] + m[3];
                float t = pt[1] * m[5] + pt[0] * m[4] + pt[2] * m[6] + m[7];
                if ( ( *(unsigned int *)&s & 0x7F800000 ) == 0x7F800000 )
                    Assert( PMESH_CPP, 9140, 0, "%s", "!IS_NAN(texCoord[0])" );
                if ( ( *(unsigned int *)&t & 0x7F800000 ) == 0x7F800000 )
                    Assert( PMESH_CPP, 9141, 0, "%s", "!IS_NAN(texCoord[1])" );
                cp->texCoord.st[2 * L]     = s;
                cp->texCoord.st[2 * L + 1] = t;
            }
            cp->normal[0]           = face->plane.normal[0];
            cp->normal[1]           = face->plane.normal[1];
            cp->normal[2]           = face->plane.normal[2];
            *(int *)&cp->vert_color = *(int *)&face->plane.dist;
            if ( backIdx > 0 )
                --backIdx;
        }
    }

    NewPatch->texture.lyrMtl   = face->mtldef[0].lyrMtl;
    NewPatch->texture.radMtl   = face->mtldef[0].radMtl;
    NewPatch->lightmap.lyrMtl  = face->mtldef[1].lyrMtl;
    NewPatch->lightmap.radMtl  = face->mtldef[1].radMtl;
    NewPatch->smoothing.lyrMtl = face->mtldef[2].lyrMtl;
    NewPatch->smoothing.radMtl = face->mtldef[2].radMtl;
    if ( NewPatch->curveDef )
        free( NewPatch->curveDef );
    NewPatch->curveDef = Patch_GenericMesh2( NewPatch, g_qeglobals.current_edit_layer, 0, 0 );
    ++NewPatch->version;

    brush_t *nb = AddBrushForPatch( NewPatch, (entity_s *)( (entity_s *)ownerInst->owner )->def );
    selbrush_t *result = (selbrush_t *)Brush_AddToList( nb, ownerInst->owner );
    if ( result->next || result->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    if ( &active_brushes == &selected_brushes )
    {
        Brush_AddToList2( result );
    }
    else
    {
        result->next = active_brushes_next;
        active_brushes_next->prev = result;
        active_brushes_next = result;
        result->prev = &active_brushes;
    }
    return (brush_t *)result;
}

// ══════════════════════════════════════════════════════════════════════════════
//  PATCH COMMAND CORES: drop-vertices / naturalize-cap / redisperse.
// ══════════════════════════════════════════════════════════════════════════════

extern edTrace_t *Trace_AllDirectionsIfFailed( float *cam_origin, edTrace_t *trace_result,
                                               float *dir, int contents );  // select.cpp 0x48DAA0

// ── Patch_RedistributeVerts (0x449930) — "drop vertices" (Shift+Alt+Ctrl+D) ────
//  For every PICKED control point, trace straight down (0,0,-1) against contents 4608
//  and, on a hit, move the point onto the hit.  Requires vertex/curve-point mode and a
//  single selected patch.  Verbatim (the IDB's v15[17] is edTrace_t.dist at +0x44).
void Patch_RedistributeVerts()
{
    if ( g_qeglobals.d_select_mode != sel_vertex && g_qeglobals.d_select_mode != sel_curvepoint )
        return;

    patch_t     *patch;
    patchMesh_t *def;
    if ( !QE_SingleBrush()
         || ( patch = selected_brushes.next->patch ) == nullptr
         || ( def = patch->def ) == nullptr )
    {
        Sys_Printf( "must have a single patch selected to redistribute" );
        return;
    }

    float dir[3] = { 0.0f, 0.0f, -1.0f };
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        drawVert_t *cp = g_qeglobals.d_move_points[i];
        float pos[3] = { cp->xyz[0], cp->xyz[1], cp->xyz[2] };
        edTrace_t tr;
        Trace_AllDirectionsIfFailed( pos, &tr, dir, 4608 );
        if ( tr.hit.brush )
        {
            drawVert_t *dst = g_qeglobals.d_move_points[i];
            dst->xyz[0] = dir[0] * tr.dist + pos[0];
            dst->xyz[1] = dir[1] * tr.dist + pos[1];
            dst->xyz[2] = tr.dist * dir[2] + pos[2];
        }
    }
    Patch_Rebuild( def, 1 );
    g_nUpdateBits = -1;
}

// Is this control point one of the picked (d_move_points) ones?  The binary open-codes
// this linear scan in both redisperse helpers below.
static bool Patch_PointIsPicked( const drawVert_t *cp )
{
    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
    {
        if ( g_qeglobals.d_move_points[i] == cp )
            return true;
    }
    return false;
}

// ── sub_4497C0 / sub_449650 — the two redisperse line passes ──────────────────
//  Each walks ONE line of the 16x16 control grid, finds the FIRST and SECOND picked
//  points on it, then linearly interpolates every picked point in between (skipping any
//  whose turned_edge bit 1 is set).  The interpolation parameter is
//  (index - first) / (second - first), which for points outside [first,second] simply
//  extrapolates — reproduced verbatim (the binary does not clamp).
//    sub_4497C0(col, def) walks ctrl[r][col] for r in [0, width)
//    sub_449650(def, row) walks ctrl[row][c] for c in [0, height)
static void Patch_RedispColumn( patchMesh_t *def, int col )          // 0x4497C0
{
    bool haveFirst = false, haveSecond = false;
    int  iFirst = 0, iSecond = 0;
    float first[3] = { 0, 0, 0 }, second[3] = { 0, 0, 0 };

    for ( int r = 0; r < def->width; ++r )
    {
        const drawVert_t *cp = &def->ctrl[r][col];
        if ( !Patch_PointIsPicked( cp ) )
            continue;
        if ( haveFirst )
        {
            haveSecond = true;  iSecond = r;
            second[0] = cp->xyz[0]; second[1] = cp->xyz[1]; second[2] = cp->xyz[2];
        }
        else
        {
            haveFirst = true;   iFirst = r;
            first[0] = cp->xyz[0]; first[1] = cp->xyz[1]; first[2] = cp->xyz[2];
        }
    }
    if ( !haveFirst || !haveSecond )
        return;

    for ( int r = 0; r < def->width; ++r )
    {
        drawVert_t *cp = &def->ctrl[r][col];
        if ( !Patch_PointIsPicked( cp ) )
            continue;
        if ( ( cp->turned_edge & 2 ) != 0 )
            continue;
        const float t = (float)( (double)( r - iFirst ) / (double)( iSecond - iFirst ) );
        const float u = 1.0f - t;
        cp->xyz[0] = second[0] * t + first[0] * u;
        cp->xyz[1] = second[1] * t + first[1] * u;
        cp->xyz[2] = t * second[2] + u * first[2];
    }
}

static void Patch_RedispRow( patchMesh_t *def, int row )             // 0x449650
{
    bool haveFirst = false, haveSecond = false;
    int  iFirst = 0, iSecond = 0;
    float first[3] = { 0, 0, 0 }, second[3] = { 0, 0, 0 };

    for ( int c = 0; c < def->height; ++c )
    {
        const drawVert_t *cp = &def->ctrl[row][c];
        if ( !Patch_PointIsPicked( cp ) )
            continue;
        if ( haveFirst )
        {
            haveSecond = true;  iSecond = c;
            second[0] = cp->xyz[0]; second[1] = cp->xyz[1]; second[2] = cp->xyz[2];
        }
        else
        {
            haveFirst = true;   iFirst = c;
            first[0] = cp->xyz[0]; first[1] = cp->xyz[1]; first[2] = cp->xyz[2];
        }
    }
    if ( !haveFirst || !haveSecond )
        return;

    for ( int c = 0; c < def->height; ++c )
    {
        drawVert_t *cp = &def->ctrl[row][c];
        if ( !Patch_PointIsPicked( cp ) )
            continue;
        if ( ( cp->turned_edge & 2 ) != 0 )
            continue;
        const float t = (float)( (double)( c - iFirst ) / (double)( iSecond - iFirst ) );
        const float u = 1.0f - t;
        cp->xyz[0] = second[0] * t + first[0] * u;
        cp->xyz[1] = second[1] * t + first[1] * u;
        cp->xyz[2] = t * second[2] + u * first[2];
    }
}

// ── DoRedistPatchPts (0x449A90) — "redisperse vertices" (Shift+F) ─────────────
//  Requires vertex/curve-point mode + a single selected patch.  If ANY control point is
//  picked, run the column pass over every column then the row pass over every row and
//  rebuild; otherwise print the "no vertices" message.
void DoRedistPatchPts()
{
    if ( g_qeglobals.d_select_mode != sel_vertex && g_qeglobals.d_select_mode != sel_curvepoint )
        return;

    patch_t     *patch;
    patchMesh_t *def;
    if ( !QE_SingleBrush()
         || ( patch = selected_brushes.next->patch ) == nullptr
         || ( def = patch->def ) == nullptr )
    {
        Sys_Printf( "must have a single patch selected to redistribute" );
        return;
    }

    bool any = false;
    for ( int r = 0; r < def->width; ++r )
    {
        for ( int c = 0; c < def->height; ++c )
        {
            if ( Patch_PointIsPicked( &def->ctrl[r][c] ) )
            {
                any = true;
                break;
            }
        }
    }
    if ( !any )
    {
        Sys_Printf( "No vertices selected, can't redistribute.\n" );
        return;
    }

    for ( int c = 0; c < def->height; ++c )
        Patch_RedispColumn( def, c );
    for ( int r = 0; r < def->width; ++r )
        Patch_RedispRow( def, r );

    Sys_Printf( "Redistributing patch pts.\n" );
    g_nUpdateBits = -1;
    Patch_Rebuild( def, 1 );
}

// ── DoTurnTerrainEdges (0x44B260) — "auto edge turn" (Alt+F2) ─────────────────
//  For every selected TERRAIN patch, walk each (width-1) x (height-1) quad and flip its
//  diagonal (ctrl[i][j].turned_edge bit 0) whenever the diagonal lengths differ in the
//  direction the binary tests.  The undo record is opened lazily, once per patch, on the
//  first flip; the tail stamps the undo id onto the patch's brush def + owning entity
//  (the inlined single-brush Undo_EndBrushList).  Prints the "Autofliped" tally.
//  sub_44B1B0 (which takes two drawVert_t BY VALUE) is just the xyz distance.
extern void Undo_AddBrush( entity_brush_s *pBrushInst );   // undo.cpp 0x45E680
extern void Undo_AddEntity( int a1 );                      // undo.cpp 0x45E8B0
extern undo_s *g_lastundo;                                 // undo.cpp 0x23F162C

static float Patch_CtrlDist( const drawVert_t *a, const drawVert_t *b )   // sub_44B1B0
{
    const float dx = a->xyz[0] - b->xyz[0];
    const float dy = a->xyz[1] - b->xyz[1];
    const float dz = a->xyz[2] - b->xyz[2];
    return (float)sqrt( (double)( dx * dx + dy * dy + dz * dz ) );
}

void DoTurnTerrainEdges()
{
    int flipped = 0;
    int patches = 0;

    for ( selbrush_t *inst = selected_brushes.next; inst != &selected_brushes; inst = inst->next )
    {
        patch_t *patch = inst->patch;
        if ( !patch || patch->def->type != PATCH_TERRAIN )
            continue;

        patchMesh_t *def = patch->def;
        bool touched = false;

        for ( int i = 0; i < def->width - 1; ++i )
        {
            for ( int j = 0; j < def->height - 1; ++j )
            {
                const float d1 = Patch_CtrlDist( &def->ctrl[i + 1][j + 1], &def->ctrl[i][j] );
                const float d2 = Patch_CtrlDist( &def->ctrl[i][j + 1], &def->ctrl[i + 1][j] );
                if ( d2 == d1 )
                    continue;
                if ( ( def->ctrl[i][j].turned_edge & 1 ) != 0 )
                {
                    if ( d2 > d1 )
                        continue;
                }
                else if ( d2 < d1 )
                {
                    continue;
                }

                if ( !touched )
                {
                    Undo_ClearRedo();
                    Undo_GeneralStart( "Turn terrain edge" );
                    entity_brush_s *sym = def->pSymbiot;
                    if ( g_lastundo )
                    {
                        if ( g_lastundo->entitylist.next != &g_lastundo->entitylist )
                            Sys_Printf( "Undo_AddBrush: WARNING adding brushes after entity.\n" );
                        entity_s *owner = (entity_s *)( (brush_t *)(intptr_t)sym )->owner;
                        if ( *(int *)&owner->eclass->fixedsize )
                            Undo_AddEntity( (int)(intptr_t)owner );
                        Undo_AddBrush( sym );
                    }
                    else
                    {
                        Sys_Printf( "Undo_AddBrush: no last undo.\n" );
                    }
                }

                def->ctrl[i][j].turned_edge ^= 1;
                ++flipped;
                touched = true;
            }
        }

        if ( touched )
        {
            ++patches;
            Patch_Rebuild( def, 1 );
        }

        // Inlined single-brush Undo_EndBrushList tail (0x44B4xx).
        entity_brush_s *sym = def->pSymbiot;
        if ( g_lastundo && !g_lastundo->done )
        {
            brush_t *bdef = (brush_t *)(intptr_t)sym;
            bdef->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
            entity_s *owner = (entity_s *)bdef->owner;
            if ( *(int *)&owner->eclass->fixedsize )
                owner->epairEdits = g_lastundo->id;
        }
        Undo_End();
    }

    Sys_Printf( "Autofliped %i faces in %i brushes.\n", flipped, patches );
    g_nUpdateBits = -1;
}

// ── Patch_SnapVertToGrid (0x449280) — cmd: snap patch control points to the grid ─
//   Two modes: if verts are picked (d_num_move_points>0) and every selected brush is a
//   patch, snap ONLY the picked control points; otherwise snap every control point of
//   each selected patch (and Brush_SnapToGrid the non-patch brushes).
void Patch_SnapVertToGrid()
{
    selbrush_t *v0 = selected_brushes_next;

    // fast path: verts picked + all-patch selection → snap only the picked points.
    if ( g_qeglobals.d_num_move_points > 0 && v0 != &selected_brushes )
    {
        selbrush_t *scan = v0;
        while ( scan->patch )
        {
            scan = scan->next;
            if ( scan == &selected_brushes )
            {
                // all selected brushes are patches.
                if ( (unsigned)g_qeglobals.d_num_move_points <= 0x401u )
                {
                    for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
                    {
                        float *p = g_qeglobals.d_move_points[i]->xyz;
                        for ( int k = 0; k < 3; ++k )
                            p[k] = (float)floor( p[k] / grid_sizes[g_qeglobals.d_gridsize] + 0.5 )
                                 * grid_sizes[g_qeglobals.d_gridsize];
                    }
                    for ( selbrush_t *b = selected_brushes_next; b != &selected_brushes; b = b->next )
                    {
                        if ( b->patch )
                        {
                            if ( b->patch->def != b->def->patch )
                                Assert( PMESH_CPP, 7079, 0, "%s", "b->patch->def == b->def->patch" );
                            Patch_Rebuild( b->patch->def, 1 );
                        }
                    }
                    g_nUpdateBits = -1;
                }
                else
                    Sys_Printf( "Can only grid snap selected vertices if less than %i selected\n", 1025 );
                return;
            }
        }
    }

    // slow path: snap every control point of each selected patch, Brush_SnapToGrid the rest.
    selbrush_t *next = selected_brushes_next;
    if ( next == &selected_brushes )
        return;
    while ( 1 )
    {
        patch_t *patch = v0->patch;
        if ( patch )
        {
            if ( patch->def != v0->def->patch )
                Assert( PMESH_CPP, 7096, 0, "%s", "b->patch->def == b->def->patch" );
            patchMesh_t *def = v0->patch->def;
            for ( int col = 0; col < def->width; ++col )
                for ( int row = 0; row < def->height; ++row )
                {
                    float *p = def->ctrl[col][row].xyz;
                    for ( int k = 0; k < 3; ++k )
                        p[k] = (float)floor( p[k] / grid_sizes[g_qeglobals.d_gridsize] + 0.5 )
                             * grid_sizes[g_qeglobals.d_gridsize];
                }
            Patch_Rebuild( def, 1 );
        }
        else
            Brush_SnapToGrid( v0->def );
        next = v0->next;
        if ( next == &selected_brushes )
            break;
        v0 = v0->next;
    }
}

// ── PMESH_07_Width (0x43B950) — Curve→Terrain: tessellate a patch into terrain ─
//   Tessellates the patch `a1` (Patch_GenericMesh2), then splits the tessellated grid
//   into 16x16 (or remainder) blocks, building a terrain patch for each block that copies
//   the tessellated verts' xyz / per-layer ST / normal / colour.  Returns the last created
//   brush instance.  Reached via OnCurveToTerrain (cmd 35041).  Verbatim from the
//   decompile (three per-layer tessellations v43[0]/v43[1]/v44 supply the 3 ST channels).
selbrush_t *PMESH_07_Width( selbrush_t *a1 )
{
    patchMesh_t *def = a1->patch->def;
    if ( def->curveDef )
        free( def->curveDef );
    def->curveDef = Patch_GenericMesh2( def, g_qeglobals.current_edit_layer, 0, 0 );

    // per-layer tessellations: v43[0]=layer0, v43[1]=layer1, layer2 via v44 (built with 0).
    curvePatchDef_t *layerMesh[3];
    for ( int i = 0; i < 3; ++i )
        layerMesh[i] = ( i == 2 ) ? Patch_GenericMesh2( def, 0, 0, 0 )
                                  : Patch_GenericMesh2( def, i, 0, 0 );

    int numPatchesX = (int)( (double)( def->curveDef->width  - 1 ) / 15.0 + 0.4999999990686774 );
    int numPatchesY = (int)( (double)( def->curveDef->height - 1 ) / 15.0 + 0.4999999990686774 );
    if ( numPatchesX < 1 ) Assert( PMESH_CPP, 1791, 0, "%s", "numPatchesX >= 1" );
    if ( numPatchesY < 1 ) Assert( PMESH_CPP, 1792, 0, "%s", "numPatchesY >= 1" );

    selbrush_t *lastInst = 0;
    int baseX = 0;
    for ( int px = 0; px < numPatchesX; ++px )
    {
        int baseY = 0;
        int lastWidth = 16;                     // v30 advances by NewPatch->width-1
        for ( int py = 0; py < numPatchesY; ++py )
        {
            patchMesh_t *NewPatch = MakeNewPatch();
            NewPatch->width  = ( px >= numPatchesX - 1 ) ? ( def->curveDef->width  - baseX ) : 16;
            if ( px >= numPatchesX - 1 && ( NewPatch->width  < 2 || NewPatch->width  > 16 ) )
                Assert( PMESH_CPP, 1811, 0, "p->width not in [2, MAX_PATCH_WIDTH]\n\t%i not in [%i, %i]",
                        NewPatch->width, 2, 16 );
            NewPatch->height = ( py >= numPatchesY - 1 ) ? ( def->curveDef->height - baseY ) : 16;
            if ( py >= numPatchesY - 1 && ( NewPatch->height < 2 || NewPatch->height > 16 ) )
                Assert( PMESH_CPP, 1821, 0, "p->height not in [2, MAX_PATCH_HEIGHT]\n\t%i not in [%i, %i]",
                        NewPatch->height, 2, 16 );
            NewPatch->type = PATCH_TERRAIN;

            for ( int row = 0; row < NewPatch->height; ++row )
            {
                for ( int col = 0; col < NewPatch->width; ++col )
                {
                    int vert = baseX + col + ( baseY + row ) * def->curveDef->width;
                    curveVert_t *cv = &def->curveDef->verts[vert];
                    if ( ( *(unsigned int *)&cv->xyz[0] & 0x7F800000 ) == 0x7F800000
                      || ( *(unsigned int *)&cv->xyz[1] & 0x7F800000 ) == 0x7F800000
                      || ( *(unsigned int *)&cv->xyz[2] & 0x7F800000 ) == 0x7F800000 )
                        Assert( PMESH_CPP, 1831, 0, "%s",
                                "!IS_NAN((pDef->mesh->verts[vert].xyz)[0]) && "
                                "!IS_NAN((pDef->mesh->verts[vert].xyz)[1]) && "
                                "!IS_NAN((pDef->mesh->verts[vert].xyz)[2])" );
                    drawVert_t *cp = &NewPatch->ctrl[col][row];
                    cp->xyz[0] = cv->xyz[0];
                    cp->xyz[1] = cv->xyz[1];
                    cp->xyz[2] = cv->xyz[2];
                    for ( int L = 0; L < 3; ++L )
                    {
                        curveVert_t *lv = &layerMesh[L]->verts[vert];  // per-layer ST source
                        if ( ( *(unsigned int *)&lv->st[0] & 0x7F800000 ) == 0x7F800000 )
                            Assert( PMESH_CPP, 1836, 0, "%s", "!IS_NAN(mesh[layerIndex]->verts[vert].st[0])" );
                        if ( ( *(unsigned int *)&lv->st[1] & 0x7F800000 ) == 0x7F800000 )
                            Assert( PMESH_CPP, 1837, 0, "%s", "!IS_NAN(mesh[layerIndex]->verts[vert].st[1])" );
                        cp->texCoord.st[2 * L]     = lv->st[0];
                        cp->texCoord.st[2 * L + 1] = lv->st[1];
                    }
                    cp->normal[0]           = cv->normal[0];
                    cp->normal[1]           = cv->normal[1];
                    cp->normal[2]           = cv->normal[2];
                    *(int *)&cp->vert_color = *(int *)&cv->vert_color;
                }
            }

            NewPatch->texture.lyrMtl   = def->texture.lyrMtl;
            NewPatch->texture.radMtl   = def->texture.radMtl;
            NewPatch->lightmap.lyrMtl  = def->lightmap.lyrMtl;
            NewPatch->lightmap.radMtl  = def->lightmap.radMtl;
            NewPatch->smoothing.lyrMtl = def->smoothing.lyrMtl;
            NewPatch->smoothing.radMtl = def->smoothing.radMtl;
            if ( NewPatch->curveDef )
                free( NewPatch->curveDef );
            NewPatch->curveDef = Patch_GenericMesh2( NewPatch, g_qeglobals.current_edit_layer, 0, 0 );
            ++NewPatch->version;

            brush_t *nb = AddBrushForPatch( NewPatch, (entity_s *)a1->owner->def );
            selbrush_t *inst = (selbrush_t *)Brush_AddToList( nb, a1->owner );
            lastInst = inst;
            if ( inst->next || inst->prev )
                Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
            if ( &active_brushes == &selected_brushes )
            {
                Brush_AddToList2( inst );
            }
            else
            {
                inst->next = active_brushes_next;
                active_brushes_next->prev = inst;
                active_brushes_next = inst;
                inst->prev = &active_brushes;
            }
            baseY += NewPatch->height - 1;
            lastWidth = NewPatch->width;
        }
        baseX += lastWidth - 1;   // IDB: v30 += NewPatch->width - 1
    }

    for ( int j = 0; j < 3; ++j )
        free( layerMesh[j] );
    return lastInst;
}

// ── Patch_GetTextureName (0x448670) — the current-layer material name of the ──────
//   single selected patch (UNIT C leftover; feeds Select_SetTexture_2 patch-mode name).
//   Returns the empty-string sentinel when the selection isn't a patch.
extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );   // materialdef.cpp 0x431640
LayerMaterialDef *Patch_GetTextureName()
{
    patch_t *patch = selected_brushes_next->patch;
    if ( patch )
        return Materialdef_GetName(
                   (MaterialDef *)( &patch->def->texture + g_qeglobals.current_edit_layer ) );
    return (LayerMaterialDef *)zero;
}

// ═════════════════════════════════════════════════════════════════════════════
//  ConnectVertices (0x44A920) — Selection→Connect in curve-point mode: weld/snap
//  the selected patch control vertices.  Wired from CMainFrame::OnSelectionConnect's
//  sel_curvepoint arm.  Ported VERBATIM from the CoD4Radiant decompilation + disasm.
// ═════════════════════════════════════════════════════════════════════════════

// The "reference" vertex list is a vec3[] view over three contiguous qeglobals fields:
//   unkown_pmesh_float2 (x0, 0x40074), unkown_pmesh_float3 (y0, 0x40078), then
//   patch_verts_array02[] (z0 + the rest, 0x4007C).  Element[i] = base + 12*i as 3 floats;
//   count = patch_verts_array02_count.  (The IDB reads it via those raw offsets.)
static inline float *PatchVerts02Elem( int i )
{
    return (float *)( (char *)&g_qeglobals.unkown_pmesh_float2 + 12 * i );
}

extern char g_nScaleHow;                                 // drag.cpp 0x23F16DC (axis-lock mask, low 3 bits)

// ── Patch_ConnectVerts (0x449BC0) — tolerant weld of the selected move-points to ─
//   the reference vert list (patch_verts_array02).  For each selected move point,
//   find the nearest reference vert within tolerant_weld² and snap the move point to it.
static void Patch_ConnectVerts()
{
    if ( g_qeglobals.d_num_move_points < 1 )
    {
        Sys_Printf( "can only connect patch vertices if at least 1 vertices are selected\n" );
        return;
    }
    if ( (unsigned)g_qeglobals.d_num_move_points > 0x1001u )
    {
        Sys_Printf( "can only connect patch vertices if at most %i vertices are selected\n", 4097 );
        return;
    }

    // v15/v13/v14 form a scratch save buffer (v12[..]) the binary writes but never reads;
    // the observable effect is only the snap.  Kept as behaviour, buffer omitted.
    float tol   = (float)( g_PrefsDlg->tolerant_weld * g_PrefsDlg->tolerant_weld );
    int   merged = 0;
    if ( g_qeglobals.d_num_move_points > 0 )
    {
        for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
        {
            float bestDistSq = tol + 1.0f;   // v17 = tol + 1.0 (nothing found yet)
            int   best       = -1;           // v5
            for ( int j = 0; j < g_qeglobals.patch_verts_array02_count; ++j )
            {
                drawVert_t *mp = g_qeglobals.d_move_points[i];
                float *ref = PatchVerts02Elem( j );
                float dx = ref[0] - mp->xyz[0];
                float dy = ref[1] - mp->xyz[1];
                float d2 = dy * dy + dx * dx;
                if ( d2 <= tol && bestDistSq > (double)d2 )
                {
                    bestDistSq = d2;
                    best       = j;
                }
            }
            if ( bestDistSq <= tol )
            {
                drawVert_t *mp = g_qeglobals.d_move_points[i];
                ++merged;
                float *ref = PatchVerts02Elem( best );
                mp->xyz[0] = ref[0];
                mp->xyz[1] = ref[1];
                mp->xyz[2] = ref[2];
            }
        }
    }
    if ( merged )
    {
        if ( merged == 1 )
            Sys_Printf( "Merged 1 vertex\n" );
        else
            Sys_Printf( "Merged %i vertices\n", merged );
    }
    else
    {
        Sys_Printf( "No vertices are close enough to do Tolerant Weld\n" );
    }
    g_qeglobals.d_num_move_points = 0;
    for ( selbrush_t *b = selected_brushes_next; b != &selected_brushes; b = b->next )
        if ( b->patch )
            Patch_Rebuild( b->patch->def, 1 );
    g_nUpdateBits = -1;
}

// ── Patch_ConnectVerts_02 (0x449D90) — snap the selected move-points to reference ─
//   element[0] (patch_verts_array02[0]), per the g_nScaleHow axis-lock mask (a bit
//   SET locks that axis; a clear bit snaps it).  Resets both point counts on exit.
static void Patch_ConnectVerts_02()
{
    if ( g_qeglobals.d_num_move_points < 1 )
    {
        Sys_Printf( "can only connect patch vertices if at least 1 patch vertices are selected\n" );
        return;
    }
    if ( (unsigned)g_qeglobals.d_num_move_points > 0x401u )
    {
        Sys_Printf( "can only connect patch vertices if at most %i vertices are selected\n", 1025 );
        return;
    }
    if ( g_qeglobals.d_num_move_points > 0 )
    {
        int lockX = g_nScaleHow & 1;
        int lockY = g_nScaleHow & 2;
        int lockZ = g_nScaleHow & 4;
        float *ref = PatchVerts02Elem( 0 );          // {unkown_pmesh_float2, _float3, patch_verts_array02[0]}
        for ( int i = 0; i < g_qeglobals.d_num_move_points; ++i )
        {
            drawVert_t *mp = g_qeglobals.d_move_points[i];
            if ( lockX || lockY || lockZ )
            {
                if ( !lockX ) mp->xyz[0] = ref[0];
                if ( !lockY ) mp->xyz[1] = ref[1];
                if ( !lockZ ) mp->xyz[2] = ref[2];
            }
            else
            {
                mp->xyz[0] = ref[0];
                mp->xyz[1] = ref[1];
                mp->xyz[2] = ref[2];
            }
        }
    }
    g_qeglobals.d_num_move_points        = 0;
    g_qeglobals.patch_verts_array02_count = 0;
    for ( selbrush_t *b = selected_brushes_next; b != &selected_brushes; b = b->next )
        if ( b->patch )
            Patch_Rebuild( b->patch->def, 1 );
    g_nUpdateBits = -1;
}

// ── Patch_ConnectVerts_03 (0x449EF0) — tolerant weld AMONG the selected move-points ─
//   (no reference list): every pair (outer i > inner j) within tolerant_weld² is welded
//   by moving the outer point onto the inner.  Requires an all-patch, no-face selection.
//   The IDB's inner loop is a 4× unroll over d_move_points[]; the ~48KB v18[] buffer is a
//   write-only undo snapshot (observed once — never read back), so it is omitted.
static void Patch_ConnectVerts_03()
{
    if ( g_SelectedFaces.GetSize() > 0 )
    {
        Sys_Printf( "can only connect patch vertices if only patches are selected\n" );
        return;
    }
    selbrush_t *b = selected_brushes_next;
    if ( b == &selected_brushes )
    {
        Sys_Printf( "can only connect patch vertices if only patches are selected\n" );
        return;
    }
    while ( b->patch )
    {
        b = b->next;
        if ( b == &selected_brushes )
        {
            // whole selection is patches → do the weld
            if ( g_qeglobals.d_num_move_points < 2 )
            {
                Sys_Printf( "can only connect patch vertices if at least 2 vertices are selected\n" );
                return;
            }
            if ( (unsigned)g_qeglobals.d_num_move_points > 0x1001u )
            {
                Sys_Printf( "can only connect patch vertices if at most %i vertices are selected\n", 4097 );
                return;
            }
            float tol    = (float)( g_PrefsDlg->tolerant_weld * g_PrefsDlg->tolerant_weld );
            int   merged = 0;
            // outer index i runs high→low over d_move_points; each is welded against every
            // lower-index point j.  (Binary: for(i=N-1; i>0; --i) for(j=i-1; j>=0; --j).)
            for ( int i = g_qeglobals.d_num_move_points - 1; i > 0; --i )
            {
                float *outer = g_qeglobals.d_move_points[i]->xyz;   // *(float**)&d_move_points[i]
                for ( int j = i - 1; j >= 0; --j )
                {
                    float *inner = g_qeglobals.d_move_points[j]->xyz;
                    float dx = inner[0] - outer[0];
                    float dy = inner[1] - outer[1];
                    float d2 = dy * dy + dx * dx;
                    if ( d2 <= tol )
                    {
                        ++merged;
                        // save old outer to the (unused) buffer, then outer ← inner
                        outer[0] = inner[0];
                        outer[1] = inner[1];
                        outer[2] = inner[2];
                    }
                }
            }
            if ( merged )
            {
                if ( merged == 1 )
                    Sys_Printf( "Merged 1 vertex\n" );
                else
                    Sys_Printf( "Merged %i vertices\n", merged );
            }
            else
            {
                Sys_Printf( "No vertices are close enough to do Tolerant Weld\n" );
            }
            g_qeglobals.d_num_move_points = 0;
            for ( selbrush_t *sb = selected_brushes_next; sb != &selected_brushes; sb = sb->next )
                if ( sb->patch )
                    Patch_Rebuild( sb->patch->def, 1 );
            g_nUpdateBits = -1;
            return;
        }
    }
    // a non-patch was in the selection → the "only patches" contract failed
    Sys_Printf( "can only connect patch vertices if only patches are selected\n" );
}

// ── ConnectVertices (0x44A920) — dispatch on the TolerantWeld pref ────────────────
void ConnectVertices()
{
    if ( g_PrefsDlg->m_bTolerantWeld != 1 )
    {
        // manual axis-snap path: require a patch in the selection.
        selbrush_t *b = selected_brushes_next;
        if ( b == &selected_brushes )
        {
            Sys_Printf( "can only connect patch vertices if a patch selected\n" );
            return;
        }
        while ( !b->patch )
        {
            b = b->next;
            if ( b == &selected_brushes )
            {
                Sys_Printf( "can only connect patch vertices if a patch selected\n" );
                return;
            }
        }
        if ( g_qeglobals.patch_verts_array02_count > 0 )
        {
            Patch_ConnectVerts_02();
            return;
        }
        if ( g_qeglobals.d_num_move_points < 2 )
        {
            Sys_Printf( "can only connect patch vertices if at least 2 vertices are selected\n" );
            return;
        }
        if ( (unsigned)g_qeglobals.d_num_move_points > 0x401u )
        {
            Sys_Printf( "can only connect patch vertices if at most %i vertices are selected\n", 1025 );
            return;
        }
        // snap each selected move point (1..N-1) to move_points[0] on the axes NOT locked
        // in g_nScaleHow (a set bit locks that axis; g_nScaleHow==0 snaps all three).
        // (The binary splits this into a while-body for g_nScaleHow!=0 and a tail for
        // ==0, but both reduce to the same per-axis rule: snap axis a iff bit a is clear.)
        drawVert_t *pt0 = g_qeglobals.d_move_points[0];
        for ( int i = 1; i < g_qeglobals.d_num_move_points; ++i )
        {
            drawVert_t *mp = g_qeglobals.d_move_points[i];
            if ( !( g_nScaleHow & 1 ) ) mp->xyz[0] = pt0->xyz[0];
            if ( !( g_nScaleHow & 2 ) ) mp->xyz[1] = pt0->xyz[1];
            if ( !( g_nScaleHow & 4 ) ) mp->xyz[2] = pt0->xyz[2];
        }
        g_qeglobals.d_num_move_points = 0;
        for ( selbrush_t *sb = selected_brushes_next; sb != &selected_brushes; sb = sb->next )
            if ( sb->patch )
                Patch_Rebuild( sb->patch->def, 1 );
        g_nUpdateBits = -1;
        return;
    }

    // TolerantWeld pref set: all-patch + no-face selection → weld among the move points
    // (Patch_ConnectVerts_03); otherwise weld to the reference list (Patch_ConnectVerts).
    if ( g_SelectedFaces.GetSize() <= 0 )
    {
        selbrush_t *b = selected_brushes_next;
        if ( b != &selected_brushes )
        {
            while ( b->patch )
            {
                b = b->next;
                if ( b == &selected_brushes )
                {
                    Patch_ConnectVerts_03();
                    return;
                }
            }
        }
    }
    Patch_ConnectVerts();
}

// ═════════════════════════════════════════════════════════════════════════════
//  FILLED / TEXTURED PATCH SURFACE - the camera/world patch draw.  The binary routes 3D
//  camera world patches (DrawBrush 0x47b070, draw_meth2 != 29) to DrawPatches (0x4414e0),
//  whose DEFAULT arm (patch_wireframe OFF) draws the patch FILLED per-layer through the
//  same surf cache the models use:
//    DrawPatches -> PMESH_27_CheckVersion (0x440670) -> PMESH_26_CheckFace (0x4405f0)
//                -> Editor_AddMeshCmd (r_ed_scene.cpp)
//  gated by PMESH_25_PatchVersion (0x4405a0) -> Patch_BuildInstanceVisuals (0x440240),
//  which builds the patch INSTANCE per-layer visuals (the patch analogue of the brush
//  Visuals_InitFaceVis / Face_BuildLayerGeom -> Editor_VB_Upload flow).
//  PMESH_23 computes per-vertex TANGENT/BINORMAL via sub_4BAF60 (TangentSpace_Calc,
//  universal/tangentspace.cpp) before Editor_VB_Upload; those vectors are render-visible
//  for normal/specular techniques, so keep them faithful rather than uploading zeroes.
// ═════════════════════════════════════════════════════════════════════════════

// ── deps (all ported elsewhere) ──────────────────────────────────────────────
extern int          MaterialDef_11( MaterialDef *m );                              // materialdef.cpp — layer count
extern Material    *MaterialDef_14( unsigned int visIndex, MaterialDef *m );        // materialdef.cpp 0x431C60
extern void         OrientationDirToWorldDir( float *out, const orientation_t *orient, const float *dir ); // draw.cpp 0x4BA4B0
extern float        Vec3Normalize_R( float *v );                                   // 0x40A5E0
extern void         PerpendicularVector( const float *src, float *dst );             // 0x4A5340
extern char         Material_GetConstantValue( Material *mtl, const char *name, float *out ); // materialdef 0x51AD50
extern unsigned int Editor_VB_Upload( Material *material, int vertCount,
                        const float *xyz, const float *tangent, const float *binormal,
                        const float *normal, const float *texCoord, const float *color ); // r_ed_vertbuf.cpp 0x51D1D0
extern char         R_Ed_FreeVertices( Material *handle, int vertCount, int vertHandle );  // r_ed_vertbuf.cpp 0x51CB70
extern int          Editor_MaterialSortKey( Material *handle );                            // r_ed_scene.cpp 0x4FDBB0
extern void         Editor_AddMeshCmd( Material *handle, int techType, int sortKey,
                        int vertCount, int vbIndexAndOffs, int indexCount, int indexTable ); // r_ed_scene.cpp 0x4FDA50
extern const MaterialTechnique *__cdecl Material_GetTechnique( const Material *material,
                        MaterialTechniqueType techType );

// PM_FRONT_FACE / PM_BACK_FACE index selectors come from qe3.h.

static float Patch_TangentDot( const float *a, const float *b )
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static float Patch_TangentAngle( const float *a, const float *b )
{
    const float dot = Patch_TangentDot( a, b );
    if ( dot <= -1.0f )
        return -3.1415927f;
    if ( dot >= 1.0f )
        return 3.1415927f;
    return (float)acos( dot );
}

// sub_4BAC40: derive one triangle's tangent/binormal candidates from xyz+st.
static void Patch_TangentBasisForTri( const float *st0, const float *xyz0,
                                      const float *st1, const float *xyz1,
                                      const float *st2, const float *xyz2,
                                      float *outTangent, float *outBinormal )
{
    const float du1 = st1[0] - st0[0];
    const float du2 = st2[0] - st0[0];
    const float dv1 = st1[1] - st0[1];
    const float dv2 = st2[1] - st0[1];
    const float e1[3] = { xyz1[0] - xyz0[0], xyz1[1] - xyz0[1], xyz1[2] - xyz0[2] };
    const float e2[3] = { xyz2[0] - xyz0[0], xyz2[1] - xyz0[1], xyz2[2] - xyz0[2] };

    if ( dv2 * du1 >= dv1 * du2 )
    {
        outTangent[0] = e1[0] * dv2 - e2[0] * dv1;
        outTangent[1] = e1[1] * dv2 - e2[1] * dv1;
        outTangent[2] = e1[2] * dv2 - e2[2] * dv1;
        outBinormal[0] = e2[0] * du1 - e1[0] * du2;
        outBinormal[1] = e2[1] * du1 - e1[1] * du2;
        outBinormal[2] = e2[2] * du1 - e1[2] * du2;
    }
    else
    {
        outTangent[0] = e2[0] * dv1 - e1[0] * dv2;
        outTangent[1] = e2[1] * dv1 - e1[1] * dv2;
        outTangent[2] = e2[2] * dv1 - e1[2] * dv2;
        outBinormal[0] = e1[0] * du2 - e2[0] * du1;
        outBinormal[1] = e1[1] * du2 - e2[1] * du1;
        outBinormal[2] = e1[2] * du2 - e2[2] * du1;
    }

    Vec3Normalize_R( outTangent );
    Vec3Normalize_R( outBinormal );
}

// sub_4BAE90: angle weights for a triangle's three vertices.
static void Patch_TangentTriWeights( const float *xyz, int i0, int i1, int i2, float *outWeights )
{
    const float *p0 = xyz + 3 * i0;
    const float *p1 = xyz + 3 * i1;
    const float *p2 = xyz + 3 * i2;
    float v01[3] = { p0[0] - p1[0], p0[1] - p1[1], p0[2] - p1[2] };
    float v12[3] = { p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] };
    float v20[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };

    Vec3Normalize_R( v01 );
    Vec3Normalize_R( v12 );
    Vec3Normalize_R( v20 );
    outWeights[0] = Patch_TangentAngle( v01, v20 );
    outWeights[1] = Patch_TangentAngle( v12, v01 );
    outWeights[2] = Patch_TangentAngle( v20, v12 );
}

// TangentSpace_Calc 0x4BAF60, specialized to PMESH_23's contiguous scratch arrays.
static void Patch_TangentSpaceCalc( int vertCount, const uint16_t *indices, int indexCount,
                                    float *xyz, float *normal, float *st,
                                    float *tangent, float *binormal )
{
    for ( int i = 0; i < vertCount; ++i )
    {
        tangent[3*i+0] = tangent[3*i+1] = tangent[3*i+2] = 0.0f;
        binormal[3*i+0] = binormal[3*i+1] = binormal[3*i+2] = 0.0f;
    }

    for ( int tri = 0; tri + 2 < indexCount; tri += 3 )
    {
        const int i0 = indices[tri + 0];
        const int i1 = indices[tri + 1];
        const int i2 = indices[tri + 2];
        iassert( i0 >= 0 && i0 < vertCount );
        iassert( i1 >= 0 && i1 < vertCount );
        iassert( i2 >= 0 && i2 < vertCount );

        float triTan[3], triBin[3], weights[3];
        Patch_TangentBasisForTri( st + 2*i0, xyz + 3*i0,
                                  st + 2*i1, xyz + 3*i1,
                                  st + 2*i2, xyz + 3*i2,
                                  triTan, triBin );
        Patch_TangentTriWeights( xyz, i0, i1, i2, weights );

        const int idx[3] = { i0, i1, i2 };
        for ( int corner = 0; corner < 3; ++corner )
        {
            float *dstT = tangent + 3 * idx[corner];
            float *dstB = binormal + 3 * idx[corner];
            const float w = weights[corner];
            dstT[0] += triTan[0] * w; dstT[1] += triTan[1] * w; dstT[2] += triTan[2] * w;
            dstB[0] += triBin[0] * w; dstB[1] += triBin[1] * w; dstB[2] += triBin[2] * w;
        }
    }

    for ( int i = 0; i < vertCount; ++i )
    {
        float *n = normal + 3 * i;
        float *t = tangent + 3 * i;
        float *b = binormal + 3 * i;
        const float ndott = Patch_TangentDot( n, t );
        t[0] -= n[0] * ndott;
        t[1] -= n[1] * ndott;
        t[2] -= n[2] * ndott;
        if ( Vec3Normalize_R( t ) < 0.001f )
        {
            Vec3Cross( b, n, t );
            if ( Vec3Normalize_R( t ) < 0.001f )
                PerpendicularVector( n, t );
        }

        float cross[3];
        Vec3Cross( n, t, cross );
        const float signDot = Patch_TangentDot( b, cross );
        if ( signDot >= 0.0f )
        {
            b[0] = cross[0]; b[1] = cross[1]; b[2] = cross[2];
        }
        else
        {
            b[0] = -cross[0]; b[1] = -cross[1]; b[2] = -cross[2];
        }
    }
}

// ── sub_43FB70 — build the FRONT-face triangle indices for the tessellated grid ──
// Two triangles per quad cell over the (width-1)x(height-1) grid; for a TERRAIN patch
// (type==64) the quad's split diagonal honours ctrl[col][row].turned_edge&1.  Returns the
// index count written.  (a1 = patchMesh_t DEF; out = uint16_t* index buffer.)
static int Patch_Fill_BuildFrontIndices( patchMesh_t *def, uint16_t *out )
{
    curvePatchDef_t *mesh = def->curveDef;
    if ( !mesh ) return 0;
    const int mw = mesh->width;          // *(_DWORD *)v2
    const int mh = mesh->height;         // *(_DWORD *)(v2+4)
    const bool terrain = ( (int)def->type == 64 );
    int idx = 0;                         // running index-write count (v13)
    // rows: 0..mh-2 ; cols: 0..mw-2
    for ( int row = 0; row + 1 < mh; ++row )
    {
        for ( int col = 0; col + 1 < mw; ++col )
        {
            const int base = row * mw + col;              // v5 + v6 * mw  (v6=row, v5=col)
            const int v00 = base;                         // (row,   col)
            const int v01 = base + 1;                     // (row,   col+1)
            const int v10 = base + mw;                    // (row+1, col)
            const int v11 = base + mw + 1;                // (row+1, col+1)
            // IDA 0x43fb9f starts at ctrl[0][0].turned_edge, advances +80 per row
            // and +1280 per column, so the flag is ctrl[col][row].
            const bool turned = terrain && ( def->ctrl[col][row].turned_edge & 1 );
            if ( turned )
            {
                // 0x43fbee: {v00, v11, v01} , {v11, v00, v10}
                out[idx+0] = (uint16_t)v00; out[idx+1] = (uint16_t)v11; out[idx+2] = (uint16_t)v01;
                out[idx+3] = (uint16_t)v11; out[idx+4] = (uint16_t)v00; out[idx+5] = (uint16_t)v10;
            }
            else
            {
                // 0x43fc4e: {v00, v10, v01} , {v01, v10, v11}
                out[idx+0] = (uint16_t)v00; out[idx+1] = (uint16_t)v10; out[idx+2] = (uint16_t)v01;
                out[idx+3] = (uint16_t)v01; out[idx+4] = (uint16_t)v10; out[idx+5] = (uint16_t)v11;
            }
            idx += 6;
        }
    }
    return idx;
}

// ── PMESH_24 (0x4401a0) — allocate + fill the front indices, then the back (reverse) ──
static void Patch_Fill_BuildIndices( patch_t *inst )
{
    curvePatchDef_t *mesh = inst->def->curveDef;
    const int mw = mesh->width, mh = mesh->height;
    const int indexCount = ( mh - 1 ) * ( 6 * mw - 6 );   // 0x4401b7
    inst->indexCount = indexCount;
    if ( indexCount <= 0 ) { inst->indicesFront = inst->indicesBack = nullptr; return; }
    // ONE alloc for both faces: front[indexCount] then back[indexCount] (IDB &v2[2*indexCount]).
    uint16_t *buf = (uint16_t *)operator new( (size_t)( 4 * indexCount ) );  // 2*indexCount uint16 each face
    inst->indicesFront = buf;
    inst->indicesBack  = buf + indexCount;
    int written = Patch_Fill_BuildFrontIndices( inst->def, buf );
    iassert( written == indexCount );                    // 0x4401f1 "indexCount == pm->indexCount"
    // back face = front reversed (0x440216 loop: back[i] = front[indexCount-i-1]).
    for ( int i = 0; i < indexCount; ++i )
        inst->indicesBack[i] = inst->indicesFront[indexCount - i - 1];
}

// ── PMESH_23_ColorTint (0x43fd40) — build per-vertex world geometry + upload per layer ──
static void Patch_Fill_BuildVisuals( patch_t *inst, const orientation_t *orient, MaterialDef *mtldef )
{
    curvePatchDef_t *mesh = inst->def->curveDef;
    const int vertCount = mesh->width * mesh->height;    // v4
    inst->vertCount = vertCount;
    curveVert_t *src = mesh->verts;

    // Scratch per-vertex arrays (freed at the end — the binary's operator new / j__free_0).
    float *xyz    = (float *)operator new( (size_t)( 12 * vertCount ) );  // 3 floats
    float *normal = (float *)operator new( (size_t)( 12 * vertCount ) );
    float *st     = (float *)operator new( (size_t)( 8  * vertCount ) );  // 2 floats
    float *tangent  = (float *)operator new( (size_t)( 12 * vertCount ) );
    float *binormal = (float *)operator new( (size_t)( 12 * vertCount ) );
    unsigned int *color = (unsigned int *)operator new( (size_t)( 4 * vertCount ) );

    // colorTint (material constant "colorTint"; default white) — 0x43fec8.
    Material *m0 = MaterialDef_14( 0, mtldef );
    iassert( m0 );                                       // MaterialDef.cpp:336
    float tint[4];
    if ( !Material_GetConstantValue( m0, "colorTint", tint ) )
        tint[0] = tint[1] = tint[2] = tint[3] = 1.0f;
    GfxColor tintC;
    Byte4PackPixelColor( tint, &tintC );

    // Lightmap-alpha mode flag: (layeredMaterial->unk_flags2 & 0x70) == 96  (0x43ff10).
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( mtldef );
    const bool lightmapAlpha = lm && ( ( lm->unk_flags2 & 0x70 ) == 96 );

    for ( int i = 0; i < vertCount; ++i )
    {
        curveVert_t *cv = &src[i];
        // position (world) + normal (world); degenerate normal -> +Z (0x43ff88).
        OrientationPosToWorldPos( &xyz[3*i], cv->xyz, orient );
        OrientationDirToWorldDir( &normal[3*i], orient, cv->normal );
        if ( Vec3Normalize_R( &normal[3*i] ) < 0.001f )
        { normal[3*i+0] = 0.0f; normal[3*i+1] = 0.0f; normal[3*i+2] = 1.0f; }
        // texcoords (curveVert.st @ +12/+16).
        st[2*i+0] = cv->st[0];
        st[2*i+1] = cv->st[1];
        // per-vertex colour, BGRA byte order (Byte4PackPixelColor: array[0]=B, [1]=G, [2]=R, [3]=A).
        // alpha byte = curveVert.vert_color.a (cv @ +43 = 0x28+3).
        const unsigned a = ( (const unsigned char *)&cv->vert_color )[3];
        unsigned char B, G, R, A;
        if ( !lightmapAlpha )
        {
            // 0x440048: colour bytes = colorTint verbatim; alpha byte from the vert.
            B = tintC.array[0]; G = tintC.array[1]; R = tintC.array[2]; A = (unsigned char)a;
        }
        else
        {
            // 0x43ffea: lightmap-alpha mode multiplies RGB by vertex alpha; final alpha byte = 0xFF.
            auto mul255 = []( unsigned c, unsigned aa ) -> unsigned char {
                unsigned t = c * aa + 127;               // +127 rounding (IDB 2155905153*x>>32>>7 == x/255)
                return (unsigned char)( t / 255 );
            };
            B = mul255( tintC.array[0], a );
            G = mul255( tintC.array[1], a );
            R = mul255( tintC.array[2], a );
            A = 0xFF;
        }
        color[i] = (unsigned)B | ( (unsigned)G << 8 ) | ( (unsigned)R << 16 ) | ( (unsigned)A << 24 );
    }

    iassert( inst->indicesFront );                       // 0x4400b5 PMESH.CPP:3670
    Patch_TangentSpaceCalc( vertCount, inst->indicesFront, inst->indexCount,
                            xyz, normal, st, tangent, binormal ); // 0x4400e4

    // Upload one VB run per layer + record {material, vertHandle} (0x4400f6 loop).
    for ( int L = 0; L < inst->visCount; ++L )
    {
        Material *lmat = MaterialDef_14( (unsigned)L, mtldef );
        inst->visArray[L].material   = lmat;
        inst->visArray[L].vertHandle = (int)Editor_VB_Upload( lmat, vertCount,
            xyz, tangent, binormal, normal, st, (const float *)color );
    }

    operator delete( color );
    operator delete( binormal );
    operator delete( tangent );
    operator delete( st );
    operator delete( normal );
    operator delete( xyz );
}

// ── PMESH_21_Indices (0x43f8c0) — FREE the current instance visuals (before rebuild) ──
static void Patch_Fill_FreeVisuals( patch_t *inst )
{
    if ( !inst )
        Assert( PMESH_CPP, 3491, 0, "%s", "pm" );
    if ( inst->visArray )
    {
        for ( int i = 0; i < inst->visCount; ++i )
            R_Ed_FreeVertices( inst->visArray[i].material, inst->vertCount, inst->visArray[i].vertHandle );
        operator delete( inst->visArray );
        // indicesBack == indicesFront + indexCount (ONE allocation, back half in the same
        // block); the binary checks it as a byte offset of 2*indexCount (uint16 indices).
        const bool contiguous = ( inst->indicesBack == inst->indicesFront + inst->indexCount );
        inst->visArray = nullptr;
        inst->visCount = 0;
        if ( !contiguous )
            Assert( PMESH_CPP, 3509, 0, "%s",
                    "pm->indices[PM_BACK_FACE] == pm->indices[PM_FRONT_FACE] + pm->indexCount" );
        if ( inst->indicesFront )
            operator delete( inst->indicesFront );       // one alloc (front, back = front+indexCount)
        inst->indicesFront = nullptr;
        inst->indicesBack  = nullptr;
        inst->indexCount   = 0;
        inst->vertCount    = 0;
    }
    else
    {
        if ( inst->visCount )     Assert( PMESH_CPP, 3495, 0, "%s", "!pm->visCount" );
        if ( inst->indicesFront ) Assert( PMESH_CPP, 3496, 0, "%s", "!pm->indices[PM_FRONT_FACE]" );
        if ( inst->indicesBack )  Assert( PMESH_CPP, 3497, 0, "%s", "!pm->indices[PM_BACK_FACE]" );
        if ( inst->indexCount )   Assert( PMESH_CPP, 3498, 0, "%s", "!pm->indexCount" );
        if ( inst->vertCount )    Assert( PMESH_CPP, 3499, 0, "%s", "!pm->vertCount" );
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PATCH DEALLOC CHAIN - the three patch free paths.  Skipping any of them leaks the
//  mesh/instance/index buffers and leaves symbiot->patch dangling at a freed patchMesh_t.
// ═══════════════════════════════════════════════════════════════════════════════

// ── PMESH_22_Indices (0x43FA20) — free the instance's index/vis buffers and ARM the
//  rebuild (version = def->version - 1).  DELIBERATE TWIN of PMESH_21_Indices
//  (0x43f8c0 = Patch_Fill_FreeVisuals above): verified in disasm (0x43faff–0x43fb5e)
//  that this one has NO R_Ed_FreeVertices loop — it does not hand vertices back to the
//  renderer — and, unlike PMESH_21, it ends by resetting the instance version.  Do NOT
//  "unify" these two; the asymmetry is the binary's (see the U1 conflated-twin lesson).
void PMESH_22_Indices( patch_t *pm )
{
    iassert( pm );   // pmesh.cpp:3520
    if ( pm->visArray )                                     // 0x43fa44
    {
        free( pm->visArray );                               // 0x43fb00 (no R_Ed_FreeVertices loop)
        bool contiguous = ( pm->indicesBack == pm->indicesFront + pm->indexCount );  // 0x43fb11
        pm->visArray  = nullptr;                            // 0x43fb14
        pm->visCount  = 0;                                  // 0x43fb17
        if ( !contiguous ) {}                               // (evaluated pre-clear above)
        iassert( pm->indices[PM_BACK_FACE] == pm->indices[PM_FRONT_FACE] + pm->indexCount );   // PMESH.CPP:3536
        free( pm->indicesFront );                           // 0x43fb3d (one alloc: back = front + count)
        pm->indicesFront = nullptr;                         // 0x43fb44
        pm->indicesBack  = nullptr;                         // 0x43fb47
        pm->indexCount   = 0;                               // 0x43fb4a
        pm->vertCount    = 0;                               // 0x43fb4d
        pm->version = (short)( pm->def->version - 1 );      // 0x43fb50/0x43fb5e — arm the rebuild
    }
    else
    {
        iassert( !pm->visCount );   // pmesh.cpp:3524
        iassert( !pm->indices[PM_FRONT_FACE] );   // PMESH.CPP:3525
        iassert( !pm->indices[PM_BACK_FACE] );    // PMESH.CPP:3526
        iassert( !pm->indexCount );   // pmesh.cpp:3527
        iassert( !pm->vertCount );   // pmesh.cpp:3528
    }
}

// ── PMESH_33 (0x442760) — free a patch INSTANCE: release its visuals through
//  PMESH_21_Indices (the R_Ed_FreeVertices variant — this path DOES return the GPU
//  vertices), free the instance, refresh the Patch Inspector.
void PMESH_33( patch_t *p )
{
    iassert( p );   // pmesh.cpp:4549
    iassert( p->def );   // pmesh.cpp:4550
    Patch_Fill_FreeVisuals( p );                            // 0x4427a7  PMESH_21_Indices
    free( p );                                              // 0x4427ad
    if ( g_PatchDialog_GetHwnd() )                          // 0x4427bc  CWnd_PatchDialog.m_hWnd
        g_PatchDialog_GetPatchInfo();                       // 0x4427c4
}

// ── Patch_Delete / PMESH_32_Symbiot (0x4426A0) — free a patch DEF (patchMesh_t):
//  break the symbiot back-pointer (brush_t.patch @0x50) so it can't dangle, free the
//  tessellated curveDef, free the def, refresh the Patch Inspector.  The three asserts
//  are the binary's (symbiot present / back-pointer identity / refCount drained).
void PMESH_32_Symbiot( int patchDef )
{
    patchMesh_t *p = (patchMesh_t *)(intptr_t)patchDef;
    brush_t *symbiot = p->symbiot;                          // 0x503C
    iassert( p->symbiot );                // PMESH.CPP:4534
    iassert( p->symbiot->patch == p );    // PMESH.CPP:4535 (brush_t.patch @0x50)
    vassert( (p->symbiot->refCount == 0), "(p->symbiot->refCount) = %i", p->symbiot->refCount );   // PMESH.CPP:4536
    if ( symbiot )
        symbiot->patch = nullptr;                           // 0x442723 — kill the dangling back-ref
    if ( p->curveDef )                                      // 0x44272a
        free( p->curveDef );                                // 0x442735
    free( p );                                              // 0x44273e
    if ( g_PatchDialog_GetHwnd() )                          // 0x44274d
        g_PatchDialog_GetPatchInfo();                       // 0x442754
}

// ── Patch_BuildInstanceVisuals (0x440240) — free-then-rebuild the instance visuals ──
// `orient` is the PLACEMENT orientation (prefab-content patches ride the composed prefabOrient;
// top-level patches ride world_orient_matrix).  The binary threads it DrawPatches->PMESH_25->
// here->PMESH_23 so each vertex is world-transformed (OrientationPosToWorldPos); hardcoding
// world_orient_matrix here builds every filled patch at its LOCAL coords.
static void Patch_BuildInstanceVisuals( patch_t *inst, const orientation_t *orient )
{
    Patch_Fill_FreeVisuals( inst );                      // PMESH_21_Indices
    // Current-edit-layer material def (patchMesh_t.texture + 8*layer; the IDB reads
    // def + 8*current_edit_layer + 24 = &texture[layer]).
    MaterialDef *mtldef = (MaterialDef *)( &inst->def->texture + g_qeglobals.current_edit_layer );
    // Prefab-content patches load with a DEGENERATE material (headless/prefab shim, layerCount 0);
    // realize it by name now that the renderer is up, else the filled draw has nothing to bind.
    extern bool Materialdef_Realize( MaterialDef *md );  // materialdef.cpp
    Materialdef_Realize( mtldef );
    const int layerCount = MaterialDef_11( mtldef );
    inst->visCount = layerCount;
    if ( layerCount <= 0 ) { inst->visArray = nullptr; return; }
    inst->visArray = (patchVisuals_s *)operator new( (size_t)( 8 * layerCount ) );
    if ( !inst->visArray )
        Error( "Out of memory on patch visuals array" );
    Patch_Fill_BuildIndices( inst );                     // PMESH_24
    Patch_Fill_BuildVisuals( inst, orient, mtldef );     // PMESH_23 (world-transform each vertex by `orient`)
    // sync the instance version to the def (0x4402b1: *(a1+4) = def->version).
    inst->version = inst->def->version;
}

// ── PMESH_25_PatchVersion (0x4405a0) — rebuild only when the def version advanced ──
// `orient` = the PLACEMENT orientation (threaded from DrawPatches, a3).  Used only on rebuild;
// version-gated exactly like the brush faceVis (sub_477D70) — a per-placement patch instance
// builds once with its own placement orient, then the cache holds.
// NOT static: the sun-preview PATCH shadow arm (Patch_AddShadowSilhouette 0x4418B0,
// shadowvolume.cpp) drives it too, exactly as the binary does (0x4418fa).
void Patch_Fill_SyncVersion( patch_t *inst, const orientation_t *orient )
{
    // Rebuild if no visuals yet OR the instance version != def version.
    if ( !inst->visArray || inst->version != inst->def->version )
        Patch_BuildInstanceVisuals( inst, orient );
}

extern bool MaterialDef_15_Drawflag_Multiply( int drawFlags, MaterialDef *m );

// ── PMESH_27_CheckVersion / PMESH_26_CheckFace (0x440670 / 0x4405f0) — the draw emit ──
// Emit the patch's tessellated mesh into the surf cache at `techType`.  `face` = PM_FRONT_FACE
// or PM_BACK_FACE.  When `mtlOverride` is set, use it for ALL layers (the d_white patch_wireframe
// under-fill); otherwise use each layer's real material (visArray[L].material).  Per-layer the
// binary's DrawPatches draw gate is NOT here: DrawPatches gates once, before PMESH_27, with
// MaterialDef_15_Drawflag_Multiply(drawFlags, patch material). PMESH_27 then emits every layer.
// Keep that shape: day_water_mud is translucent in D3D state but has toolFlags&0x70 == 0x20, so
// the binary draws it in the first/skip-multiply world pass. A depth-write substitute would skip
// it and the water strip would vanish.
// Returns true when at least one mesh was handed to the surf cache.
static bool Patch_Fill_Emit( patch_t *inst, Material *mtlOverride, int face, int techType )
{
    if ( !inst->visArray || inst->vertCount <= 0 || !inst->indicesFront )
        return false;
    uint16_t *indexTable = ( face == PM_BACK_FACE ) ? inst->indicesBack : inst->indicesFront;
    bool emitted = false;
    if ( mtlOverride )
    {
        Editor_AddMeshCmd( mtlOverride, techType, Editor_MaterialSortKey( mtlOverride ),
                           inst->vertCount, inst->visArray[0].vertHandle,
                           inst->indexCount, (int)indexTable );
        emitted = true;
    }
    else
    {
        int sortKey = inst->visArray[0].material ? Editor_MaterialSortKey( inst->visArray[0].material ) : 0;
        for ( int L = 0; L < inst->visCount; ++L )
        {
            Material *m = inst->visArray[L].material;
            if ( !m ) continue;
            Editor_AddMeshCmd( m, techType, sortKey + L,
                               inst->vertCount, inst->visArray[L].vertHandle,
                               inst->indexCount, (int)indexTable );
            emitted = true;
        }
    }
    return emitted;
}

// DrawPatches (0x4414E0): filled front, selected white under-fill, back-face
// wireframe, optional patch grid, and control-point tail.
bool DrawPatches( patch_t *inst, const orientation_t *orient, int techType, int drawFlags )
{
    if ( !inst || !inst->def )
        return false;
    // Ensure the tessellated render mesh exists (built at load; safety net).
    if ( !inst->def->curveDef )
    {
        extern curvePatchDef_t *Patch_GenericMesh2( patchMesh_t *p, int layer, int a3, int a4 );
        inst->def->curveDef = Patch_GenericMesh2( inst->def, g_qeglobals.current_edit_layer, 0, 0 );
        ++inst->def->version;
        if ( !inst->def->curveDef )
            return false;
    }
    Patch_Fill_SyncVersion( inst, orient );              // PMESH_25 -> rebuild if stale (with placement orient)
    if ( !inst->visArray || inst->visCount <= 0 )
        return false;                                    // no realized layer -> wireframe fallback

    // FAITHFUL DRAW GATE: IDA DrawPatches 0x4414e0 calls
    //   PMESH_25_PatchVersion(pm, orient);
    //   if (override || MaterialDef_15_Drawflag_Multiply(drawFlags, &patch->texture))
    //       PMESH_27_CheckVersion(...)
    // and PMESH_27 emits every layer. Do not substitute material depth-write here: CoD4 water
    // materials intentionally have depth-write off but toolFlags&0x70 != 0x70, so stock Radiant
    // draws them in the first world pass.
    // 0x4414FA/0x4414FF — `mov eax,[edi] ; add eax,18h`: DrawPatches reads &def->texture
    // FLAT (layer 0 ALWAYS), it does NOT index by current_edit_layer.  (Only the INSTANCE
    // VISUAL build, Patch_BuildInstanceVisuals 0x440255, uses +8*current_edit_layer.)
    // DrawPatchCameraFilled 0x4415D8 does the same flat read — they must agree, or the
    // white selected-outline pass and the filled pass can disagree on the same patch when
    // the edit layer is not 0.
    MaterialDef *mtldef = (MaterialDef *)&inst->def->texture;
    if ( !MaterialDef_15_Drawflag_Multiply( drawFlags, mtldef ) )
        return true;

    const int patchWireframe = g_PrefsDlg->patch_wireframe;
    const bool drawFront = patchWireframe == 0 || ( patchWireframe == 2 && techType != TECHNIQUE_WIREFRAME_SHADED );
    if ( drawFront )
        Patch_Fill_Emit( inst, nullptr, PM_FRONT_FACE, techType );          // 0x44153E

    if ( ( drawFlags & 1 ) != 0 )
    {
        if ( !drawFront )
            Patch_Fill_Emit( inst, g_qeglobals.d_white, PM_FRONT_FACE, techType ); // 0x44155D
    }
    else
    {
        Patch_Fill_Emit( inst, nullptr, PM_BACK_FACE,
                         TECHNIQUE_WIREFRAME_SHADED );                     // 0x441572
        if ( patchWireframe )
        {
            extern void DrawPatchesWireframeGrid( patch_t *, GfxColor *,
                                                   const orientation_t *, char, int );
            GfxColor color;
            Byte4PackPixelColor( g_qeglobals.d_savedinfo.colors[23], &color );
            DrawPatchesWireframeGrid( inst, &color, orient, 2, drawFlags ); // 0x4415A6
        }
    }

    extern void Patch_DrawControlPoints( patch_t *, const orientation_t * );
    Patch_DrawControlPoints( inst, orient );                                // 0x4415B7
    return true;
}

// ── DrawPatchCameraFilled (0x4415D0) — the 3D-CAMERA tech-29 patch draw ───────
// The selected-camera pass emits d_white at TECHNIQUE_WIREFRAME_SOLID through
// both winding lists after the layer-0 draw gate.
bool DrawPatchCameraFilled( patch_t *inst, const orientation_t *orient, int drawFlags )
{
    bool front = false, back = false;

    if ( inst && inst->def && inst->def->curveDef )
    {
        Patch_Fill_SyncVersion( inst, orient );              // 0x4415D3
        if ( inst->visArray && inst->visCount > 0 )
        {
            // 0x4415D8/0x4415DD — def + 0x18 == &def->texture, no layer index.
            if ( MaterialDef_15_Drawflag_Multiply( drawFlags, (MaterialDef *)&inst->def->texture ) )
            {                                                // 0x4415E8 jz -> nothing drawn
                front = Patch_Fill_Emit( inst, g_qeglobals.d_white, PM_FRONT_FACE,
                                         TECHNIQUE_WIREFRAME_SOLID );
                back  = Patch_Fill_Emit( inst, g_qeglobals.d_white, PM_BACK_FACE,
                                         TECHNIQUE_WIREFRAME_SOLID );
            }
        }
    }

    return front || back;
}

// ═════════════════════════════════════════════════════════════════════════════
//  RELOCATED HOME — this function's embedded Assert() calls name THIS file as
//  their source (see the brush.cpp relocation protocol / line-uniqueness test).
// ═════════════════════════════════════════════════════════════════════════════
// ══════════════════════════════════════════════════════════════════════════════
//  Patch_FindReplaceTexture  (0x449520) — patch (pmesh) face material swap.
//  b = the patch brush DEF; replaceName / findName; flags.  When (flags&2)==0 the
//  patch's current-layer material name must equal findName.  On a match → SetMaterial
//  (replaceName) into &patch->texture[layer] + ++version.  Returns 1 if it replaced.
// ══════════════════════════════════════════════════════════════════════════════
char Patch_FindReplaceTexture( brush_t *b, const char *replaceName,
                               const char *findName, char flags )
{
    iassert( b );          // PMESH.CPP:7120
    iassert( b->patch );   // PMESH.CPP:7121

    patchMesh_t *patch = b->patch;
    patchMesh_material *slot = &patch->texture + g_qeglobals.current_edit_layer;
    if ( ( flags & 2 ) == 0 )
    {
        const char *name = (const char *)Materialdef_GetName( (MaterialDef *)slot );
        if ( _stricmp( name, findName ) )
            return 0;                       // current name != find → skip
    }
    SetMaterial( replaceName, slot );
    ++patch->version;
    return 1;
}

// 0x44cf00 Patch_AllocInstance — the patch analogue of Brush_AddToList, called from
// Brush_AddToList when a brush owns a patch.  The instance version word is seeded to
// def->version-1 (the rebuild trigger, mirroring selbrush.version): without it a fresh
// instance whose version happened to equal def->version would skip its first curveDef
// rebuild.  refCount lives at def->symbiot(+0x501C) + 0x1c.
patch_t *PMESH_55(patchMesh_t *def)
{
    iassert( def );   // PMESH.CPP:9031
    iassert( def->symbiot );   // PMESH.CPP:9032
    vassert( (def->symbiot->refCount >= 0), "(def->symbiot->refCount) = %i", def->symbiot->refCount );   // PMESH.CPP:9033",

    patch_t *p = (patch_t *)calloc( 1, sizeof( patch_t ) );   // = IDB new(0x20) + zero [1..7]
    if ( p )
    {
        p->def     = def;
        p->version  = (short)( def->version - 1 );
    }
    return p;
}
