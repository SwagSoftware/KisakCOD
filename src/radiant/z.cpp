#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Vertical ruler view: world-Z grid, brush extents at the camera XY column, and
// camera/brush-height interaction.

#include "stdafx.h"
#include "xywnd.h"                  // XY_SetupProjectionMtx, ED_VIEW_*
#include "mainfrm.h"                // CZWnd
#include "qe3.h"                    // g_qeglobals, selbrush_t, brush_t, grid_sizes
#include <gfx_d3d/r_gfx.h>          // GfxMatrix, GfxColor, GfxPointVertex
#include <gfx_d3d/r_init.h>         // dx, R_SetupRendertarget_CheckDevice, R_Hwnd_Resize
#include <gfx_d3d/r_scene.h>        // R_Ed_SetSceneParms
#include <gfx_d3d/r_rendercmds.h>   // R_AddCmd_Line3D, R_BeginFrame/R_EndFrame, clear/material-color
#include <math.h>

// ─── Editor globals shared with xywnd.cpp / engine_stubs / map.cpp ───────────────
extern float grid_sizes[];                                    // engine_stubs.cpp (0x6dde5c)
extern float region_mins[3];                                  // map.cpp          (0x739c14)
extern float region_maxs[3];                                  // map.cpp          (0x739d24)
extern float world_orient_matrix[4][3];                       // entity.cpp (identity orientation)
extern char  Byte4PackPixelColor(float *from, GfxColor *out); // engine_stubs.cpp (0x402ac0)
extern selbrush_t active_brushes;                             // map.cpp          (0x23F189C)
extern selbrush_t selected_brushes;                           // engine_stubs     (0x23F1864)
extern void  Radiant_FL_Log( const char *fmt, ... );          // mainfrm.cpp

// R_Add3DLine (draw.cpp, IDB 0x40c110): append one line segment (p1→p2, through `orient`)
// into a caller batch; flushes via R_AddCmd_Line3D on overflow; returns the new vert count.
extern int R_Add3DLine( GfxPointVertex *verts, const orientation_t *orient,
                        const float *p1, const float *p2, const unsigned int *color,
                        char width, int vertCount, int maxVertCount );

// ── Z_Draw active/selected filled-column deps (all already ported) ────────────────
//  Ed_BrushFloorRay (select.cpp) is the headless-safe wrapper over the binary's Brush_Ray
//  (0x475fe0): does the ray [start, start+262144*dir] enter the convex brush def?  Fills
//  *outDist = the entry distance along dir (== the binary's *a5 = dot(dir, entryPt-start)),
//  returns true on entry.  Clips against def->faceCount (authoritative), so it is correct
//  without a prior camera draw (the §11 instance-vs-def trap Brush_Ray itself hits headless).
extern bool Ed_BrushFloorRay( brush_t *def, const float *start, const float *dir, float *outDist ); // select.cpp
// Brush_GetEntityLineColor (brush.cpp 0x47aa20): RGBA of a brush's entity line (worldspawn→0,
//   _color epair / actor / node_path overrides).  Canonical body, now non-static.
extern char Brush_GetEntityLineColor( float *outRgba, brush_t *brushDef, const float *inRgba );    // brush.cpp
extern CMainFrame *g_pParentWnd;                                                                   // 0x25D5A70 (also re-declared in the input cluster below)
// colorWhite[4] = {1,1,1,1} is static const float in q_shared.h (via stdafx.h → qe3.h).

// ─── Z-view globals (IDB z.cpp: z_width 0x241a598 / z_height 0x241a59c / z_scale 0x241a5b0
//     / m_Camera.origin 0x241a5a4..ac). The Z view's "camera" is the editor camera position
//     — there is no camera WINDOW until P5.4, so z.cpp owns these here; P5.4 reconciles
//     m_Camera with CCamWnd. The defaults centre an empty view on the origin. ──────────────
int   z_width  = 0;                       // Z window client width  (px), set in OnSize
int   z_height = 0;                       // Z window client height (px)
float z_scale  = 1.0f;                    // pixels per world Z unit
float m_Camera_origin[3] = { 0, 0, 0 };   // editor camera world position (origin0/1/2)

// ─── Z_SetupScene — CXYWnd::SetupScene (IDB 0x5064c0) with the Z view's explicit org/axis ─
// org = {0, cameraZ, 0}; axis = the XY-top basis (forward=+Z, right=-X, up=+Y). The ortho
// projection is sized by the live target-window dimensions / z_scale (matches XY_SetupScene).
static void Z_SetupScene()
{
    iassert( z_scale != 0 );

    float org[3]  = { 0.0f, m_Camera_origin[2], 0.0f };
    float axis[3][3];
    memset(axis, 0, sizeof(axis));
    axis[0][2] =  1.0f;   // forward = +Z
    axis[1][0] = -1.0f;   // right   = -X
    axis[2][1] =  1.0f;   // up      = +Y

    const float invScale = 1.0f / z_scale;
    const float projW = invScale * (float)dx.windows[dx.targetWindowIndex].width;
    const float projH = invScale * (float)dx.windows[dx.targetWindowIndex].height;

    GfxMatrix proj;
    XY_SetupProjectionMtx(&proj, projW, projH, -262144.0f);
    R_Ed_SetSceneParms(org, (const float(*)[3])axis, &proj);
}

// Line batch — sized generously; R_Add3DLine flushes via R_AddCmd_Line3D when full, so any
// size is correct, this just controls how often it flushes (IDB uses 1362 for the grid).
static const int  Z_LINE_MAX = 2048;
static GfxPointVertex s_zLines[Z_LINE_MAX];
// The active/selected column passes use TWO 681-vertex batches like the IDB (v51 thick spine,
// v52 thin box) — Z_AddSeg caps both at 681 (R_Add3DLine flushes on overflow).
static GfxPointVertex s_zSpine[681];

// ─── Z_DrawGrid (IDB 0x49acf0), grid lines only (text labels = P5.5) ─────────────
static void Z_DrawGrid()
{
    const int   halfW   = (int)( (double)z_width  * 0.5 / z_scale );   // half view width  (world units)
    const float halfH   = (float)(int)( 0.5 * (double)z_height / z_scale ); // half view height (world units)
    const float camZ    = m_Camera_origin[2];

    float zmin = camZ - halfH;
    if ( region_mins[2] > zmin ) zmin = region_mins[2];
    zmin = floorf(zmin * 0.015625f) * 64.0f;

    float zmax = camZ + halfH;
    if ( region_maxs[2] < zmax ) zmax = region_maxs[2];
    zmax = ceilf(zmax * 0.015625f) * 64.0f;

    int   n  = 0;
    GfxColor col;

    // Minor grid (snap-size horizontal lines) — only when legibly zoomed and the minor
    // colour differs from the background.
    const float minorStep = grid_sizes[g_qeglobals.d_gridsize];
    const float *colMinor = g_qeglobals.d_savedinfo.colors[2];
    const float *colBack  = g_qeglobals.d_savedinfo.colors[1];
    const bool   minorSameAsBack =
        colMinor[0] == colBack[0] && colMinor[1] == colBack[1] &&
        colMinor[2] == colBack[2] && colMinor[3] == colBack[3];

    if ( g_qeglobals.d_showgrid )
    {
        if ( minorStep * z_scale >= 4.0f && !minorSameAsBack && zmax > zmin )
        {
            Byte4PackPixelColor(const_cast<float *>(colMinor), &col);
            for ( float z = zmin; z < zmax; z += minorStep )
            {
                float a[3] = { (float)-halfW, z, 0.0f };
                float b[3] = { (float) halfW, z, 0.0f };
                n = R_Add3DLine(s_zLines, (const orientation_t *)world_orient_matrix, a, b, &col.packed, 1, n, Z_LINE_MAX);
            }
        }
    }

    // Major grid: central vertical axis (x=0, zmin..zmax) + horizontal lines every 64 units.
    Byte4PackPixelColor(g_qeglobals.d_savedinfo.colors[3], &col);
    {
        float a[3] = { 0.0f, zmin, 0.0f };
        float b[3] = { 0.0f, zmax, 0.0f };
        n = R_Add3DLine(s_zLines, (const orientation_t *)world_orient_matrix, a, b, &col.packed, 1, n, Z_LINE_MAX);
    }
    if ( zmax > zmin )
    {
        for ( float z = zmin; z < zmax; z += 64.0f )
        {
            float a[3] = { (float)-halfW, z, 0.0f };
            float b[3] = { (float) halfW, z, 0.0f };
            n = R_Add3DLine(s_zLines, (const orientation_t *)world_orient_matrix, a, b, &col.packed, 1, n, Z_LINE_MAX);
        }
    }
    if ( n > 0 )
        R_AddCmd_Line3D((short)(n / 2), 1, s_zLines);

    // P5.5: Z scale labels — the world-Z value at each major (64-unit) grid line, down the
    // left edge. Same text path as the XY view (R_AddCmdDrawTextAtPosition → editor font).
    // Line coords are {x, z, 0}: [0]=horizontal (X), [1]=the Z value (vertical). xPixelStep
    // advances along +[0], yPixelStep screen-down along -[1].
    static int s_noText = -1;
    if ( s_noText < 0 ) s_noText = getenv( "RADIANT_NOTEXT" ) ? 1 : 0;
    Font_s *font = (Font_s *)g_qeglobals.d_font_list;
    if ( !s_noText && font && zmax > zmin )
    {
        const float inv = 1.0f / z_scale;
        float a4[3] = { inv, 0.0f, 0.0f };     // xPixelStep (horizontal)
        float yp[3] = { 0.0f, -inv, 0.0f };    // yPixelStep (screen-down)
        float *colText = g_qeglobals.d_savedinfo.colors[8];
        const float leftX = (float)( 1 - halfW );   // IDB 0x49b023: a3 = (float)(1 - z_width/2/z_scale)
        char text[32];
        for ( float z = zmin; z < zmax; z += 64.0f )
        {
            sprintf( text, "%i", (int)z );           // IDB 0x49b08d: "%i", (int)z (truncates; exact for 64-multiples)
            float org[3] = { leftX, z, 0.0f };
            R_AddCmdDrawTextAtPosition( text, font, org, a4, yp, colText );
        }
    }
}

// ─── Z_Draw column helpers (faithful to IDB 0x49b520, 2026-06-25) ────────────────
// sub_40C1E0 (IDB 0x40c1e0): a thin 2D-point wrapper around R_Add3DLine — build two vec3
//   from the (x1,y1)/(x2,y2) screen-column pairs (z=0) and append.  Inlined as Z_AddSeg.
static inline int Z_AddSeg( GfxPointVertex *verts, float x1, float y1, float x2, float y2,
                            const unsigned int *color, char width, int vertCount )
{
    float p1[3] = { x1, y1, 0.0f };
    float p2[3] = { x2, y2, 0.0f };
    return R_Add3DLine( verts, (const orientation_t *)world_orient_matrix, p1, p2,
                        color, width, vertCount, 681 );
}

// Brush_Ray at the camera's XY column: cast a ray straight DOWN from z=+131072 to find the
// brush's TOP Z, and straight UP from z=-131072 to find the BOTTOM Z, at (camX,camY).  The
// binary uses Brush_Ray (0x475fe0) whose *outDist = dot(dir, entryPt-start); for dir={0,0,-1}
// start.z=+131072 that gives 131072-topZ (→ topZ after the 131072- fixup), and for dir={0,0,+1}
// start.z=-131072 it gives bottomZ+131072 (→ bottomZ after the -131072 fixup).  Ed_BrushFloorRay
// (select.cpp) reproduces exactly that distance.  Returns true only if BOTH rays enter.
static bool Z_RayBrushZExtent( brush_t *def, float *outBottom, float *outTop )
{
    const float camX = m_Camera_origin[0];
    const float camY = m_Camera_origin[1];
    const float dirDown[3] = { 0.0f, 0.0f, -1.0f };
    const float dirUp[3]   = { 0.0f, 0.0f,  1.0f };
    const float startHigh[3] = { camX, camY,  131072.0f };
    const float startLow[3]  = { camX, camY, -131072.0f };

    float dTop, dBot;
    if ( !Ed_BrushFloorRay( def, startHigh, dirDown, &dTop ) )
        return false;
    *outTop = 131072.0f - dTop;                       // IDB: v50 = 131072 - v50 → top Z
    if ( !Ed_BrushFloorRay( def, startLow, dirUp, &dBot ) )
        return false;
    *outBottom = dBot - 131072.0f;                    // IDB: v49 = v49 - 131072 → bottom Z
    return true;
}

// The brush passes the XY-footprint test if the camera's (X,Y) is inside the brush def's
// XY AABB (IDB: m_Camera_origin0 in [def->mins[0],def->maxs[0]] etc; def[8..12]).
static inline bool Z_CamInBrushXY( const brush_t *def )
{
    return m_Camera_origin[0] > def->mins[0] && def->maxs[0] > m_Camera_origin[0] &&
           m_Camera_origin[1] > def->mins[1] && def->maxs[1] > m_Camera_origin[1];
}

// Seed the entity line colour from the brush's owner entity's DEF eclass (color[0..2] +
// eclass->unk as alpha — IDB reads eclass[9..12]), then let Brush_GetEntityLineColor override
// (_color / actor / node_path).
// §11 INSTANCE-vs-DEF (mp_test read-AV fix): the IDB reads `instance->owner->def->eclass` — from
// the selbrush_t INSTANCE's owner.  The prior version took the brush DEF (b->def) and read
// `def->owner->def->eclass`, which by the b->def->owner == b->owner->def invariant equals
// (b->owner->def)->def->eclass — ONE ->def too deep.  When owner->def isn't self-referential that
// extra hop lands on a bogus eclass pointer → read AV (crash opening mp_test, ec=0x8592D47).  Take
// the INSTANCE and read b->owner->def->eclass exactly like the binary; b->def still feeds
// Brush_GetEntityLineColor (the IDB passes v2->def there).
static void Z_EntLineColor( selbrush_t *b, GfxColor *out )
{
    float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    entity_s     *owner = b ? b->owner : nullptr;
    entity_s_def *odef  = owner ? (entity_s_def *)owner->def : nullptr;
    eclass_t     *ec    = odef ? odef->eclass : nullptr;
    if ( ec )
    {
        rgba[0] = ec->color[0];
        rgba[1] = ec->color[1];
        rgba[2] = ec->color[2];
        rgba[3] = ec->unk;           // IDB eclass[12] = eclass+0x30 = the 4th seed value
    }
    Brush_GetEntityLineColor( rgba, b ? b->def : nullptr, rgba );
    Byte4PackPixelColor( rgba, out );
}

// ─── Z_Draw active-brush filled column (the v51 spine + v52 box of IDB's first loop) ──
// For each ACTIVE brush whose XY footprint contains the camera, ray its top/bottom Z at the
// column and draw a THICK vertical spine (entity-line colour, pixel-width = 2*(z_width/3)*z_scale)
// at x=0 plus a thin WHITE box outline of the ±(z_width/3) column × [bottom,top].
static void Z_DrawActiveColumns( int v0 )
{
    if ( active_brushes.next == &active_brushes )
        return;

    GfxColor white;
    Byte4PackPixelColor( const_cast<float *>( colorWhite ), &white );
    // IDB v42[0]: the spine pixel-width truncates into a CHAR (R_AddCmd_Line3D's width is char) —
    // faithful, even though wide views wrap it negative (the binary does exactly this).
    const char spineWidth = (char)(int)( (double)( 2 * v0 ) * z_scale );
    const float colX  =  (float)v0;
    const float colXn = -(float)v0;

    int nSpine = 0;   // v51 (thick spine, per-brush entity colour)
    int nBox   = 0;   // v52 (thin white box outline)

    // The binary walks active_brushes via v2->onext, but hex-rays mistypes the instance node
    // as brush_t_with_custom_def* (88B) — at byte offset 0x04 that field is selbrush_t.next
    // (the global display-list link), NOT ownerNext (0x0C).  Walk .next.
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def || !Z_CamInBrushXY( def ) )
            continue;
        float bottom, top;
        if ( !Z_RayBrushZExtent( def, &bottom, &top ) )
            continue;

        GfxColor entCol;
        Z_EntLineColor( b, &entCol );   // pass the INSTANCE (b), not b->def — see Z_EntLineColor

        // thick spine at x=0, bottom..top, entity-line colour
        nSpine = Z_AddSeg( s_zSpine, 0.0f, bottom, 0.0f, top, &entCol.packed, spineWidth, nSpine );
        // white box outline of the column (±v0) × [bottom,top]
        nBox = Z_AddSeg( s_zLines, colXn, bottom, colX,  bottom, &white.packed, 1, nBox );  // bottom
        nBox = Z_AddSeg( s_zLines, colX,  bottom, colX,  top,    &white.packed, 1, nBox );  // right
        nBox = Z_AddSeg( s_zLines, colX,  top,    colXn, top,    &white.packed, 1, nBox );  // top
        nBox = Z_AddSeg( s_zLines, colXn, top,    colXn, bottom, &white.packed, 1, nBox );  // left
    }
    if ( nSpine )
        R_AddCmd_Line3D( (short)( nSpine / 2 ), spineWidth, s_zSpine );
    if ( nBox )
        R_AddCmd_Line3D( (short)( nBox / 2 ), 1, s_zLines );
}

// ─── Z_Draw selected-brush column (the v51 spine + v52 box of IDB's second loop) ──────
// Per SELECTED brush: if the column ray hits, draw the THICK entity-coloured spine; ALWAYS
// draw the mins/maxs box outline (selection colour colors[10]) — the box uses def->mins/maxs[2]
// directly (not the ray result), matching the IDB's unconditional R_Add3DLine box tail.
static void Z_DrawSelectedColumns( int v0 )
{
    if ( selected_brushes.next == &selected_brushes )
        return;

    GfxColor selCol;
    Byte4PackPixelColor( g_qeglobals.d_savedinfo.colors[10], &selCol );
    const char spineWidth = (char)(int)( (double)( 2 * v0 ) * z_scale );
    const float colX  =  (float)v0;
    const float colXn = -(float)v0;

    int nSpine = 0;   // v51 (thick entity-coloured spine, only on a ray hit)
    int nBox   = 0;   // v52 (selection-coloured mins/maxs box, always)

    // Same as the active loop: the IDB v13->onext at offset 0x04 == selbrush_t.next.
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def )
            continue;

        if ( Z_CamInBrushXY( def ) )
        {
            float bottom, top;
            if ( Z_RayBrushZExtent( def, &bottom, &top ) )
            {
                GfxColor entCol;
                Z_EntLineColor( b, &entCol );   // pass the INSTANCE (b), not b->def — see Z_EntLineColor
                nSpine = Z_AddSeg( s_zSpine, 0.0f, bottom, 0.0f, top, &entCol.packed, spineWidth, nSpine );
            }
        }

        const float zmin = def->mins[2];
        const float zmax = def->maxs[2];
        nBox = Z_AddSeg( s_zLines, colXn, zmin, colX,  zmin, &selCol.packed, 1, nBox );  // bottom
        nBox = Z_AddSeg( s_zLines, colX,  zmin, colX,  zmax, &selCol.packed, 1, nBox );  // right
        nBox = Z_AddSeg( s_zLines, colX,  zmax, colXn, zmax, &selCol.packed, 1, nBox );  // top
        nBox = Z_AddSeg( s_zLines, colXn, zmax, colXn, zmin, &selCol.packed, 1, nBox );  // left
    }
    if ( nSpine )
        R_AddCmd_Line3D( (short)( nSpine / 2 ), spineWidth, s_zSpine );
    if ( nBox )
        R_AddCmd_Line3D( (short)( nBox / 2 ), 1, s_zLines );
}

// ─── R_RenderSomething_Important_ (IDB 0x49b100) — the camera-height marker ───────────
// A fixed 8-segment glyph (16 verts, blue flt_6DE160={0,0,1,1}) drawn at the live camera Z
// (g_pParentWnd->m_pCamWnd->camera.origin[2]), spanning x = ±(z_width/4): a centre bar at
// camZ with a small arrowhead (camZ±8) and a downward tick (camZ-48).  Marks where the 3D
// camera sits on the Z ruler.
static void Z_DrawCameraMarker()
{
    if ( !g_pParentWnd || !g_pParentWnd->m_pCamWnd )
        return;

    const float camZ = g_pParentWnd->m_pCamWnd->camera.origin[2];
    const float hw   = (float)( z_width / 4 );     // IDB v12 = z_width/4
    const float x    =  hw;                        // v8 = +hw
    const float xn   = -hw;                        // v9 = -hw

    static const float kMarkerBlue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };   // flt_6DE160
    GfxColor col;
    Byte4PackPixelColor( const_cast<float *>( kMarkerBlue ), &col );

    GfxPointVertex verts[16];
    int n = 0;
    n = Z_AddSeg( verts, xn,   camZ,        0.0f, camZ + 8.0f, &col.packed, 1, n );
    n = Z_AddSeg( verts, x,    camZ,        0.0f, camZ + 8.0f, &col.packed, 1, n );
    n = Z_AddSeg( verts, x,    camZ,        0.0f, camZ - 8.0f, &col.packed, 1, n );
    n = Z_AddSeg( verts, xn,   camZ,        0.0f, camZ - 8.0f, &col.packed, 1, n );
    n = Z_AddSeg( verts, xn,   camZ,        x,    camZ,        &col.packed, 1, n );
    n = Z_AddSeg( verts, x,    camZ,        x,    camZ - 48.0f,&col.packed, 1, n );
    n = Z_AddSeg( verts, xn,   camZ - 48.0f,x,    camZ - 48.0f,&col.packed, 1, n );
    n = Z_AddSeg( verts, xn,   camZ - 48.0f,xn,   camZ,        &col.packed, 1, n );
    iassert( n == 16 );                            // IDB z.cpp:279 vertCount == ARRAY_COUNT(verts)
    if ( n )
        R_AddCmd_Line3D( (short)( n / 2 ), 1, verts );
}

// ─── Z_Draw (IDB 0x49b520) — grid + active/selected filled columns + camera marker ───
void Z_Draw()
{
    if ( !active_brushes.next )    // brush lists not bootstrapped → no map loaded
        return;
    const int v0 = z_width / 3;    // IDB v0 = z_width/3 (column half-width, world units)
    Z_SetupScene();
    Z_DrawGrid();
    Z_DrawActiveColumns( v0 );     // IDB first loop: active brushes under the cursor column
    Z_DrawSelectedColumns( v0 );   // IDB second loop: selected brushes' spine + mins/maxs box
    Z_DrawCameraMarker();          // IDB R_RenderSomething_Important_ (0x49b100)
}

// Centre the Z view on the loaded map's Z range (the editor does this from the camera /
// start entity; there is no camera window until P5.4, so do it from the brush bounds like
// Radiant_CenterXYOnMap does for XY). After this, any selected brush's Z extent is in view.
void Z_CenterOnMap()
{
    float zmin =  1e30f, zmax = -1e30f;
    bool  any  = false;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def ) continue;
        if ( def->mins[2] < zmin ) zmin = def->mins[2];
        if ( def->maxs[2] > zmax ) zmax = def->maxs[2];
        any = true;
    }
    if ( !any ) return;

    m_Camera_origin[2] = 0.5f * ( zmin + zmax );
    float zext = ( zmax - zmin ) > 1.0f ? ( zmax - zmin ) : 1.0f;
    int   h    = z_height > 0 ? z_height : 768;
    float s    = (float)h / zext * 0.9f;   // fit the map Z range with a 10% margin
    if ( s < 0.02f ) s = 0.02f;
    if ( s > 4.0f  ) s = 4.0f;
    z_scale = s;
    Radiant_FL_Log( "Z view fit: cameraZ=%g z_scale=%g zext=%g", m_Camera_origin[2], z_scale, zext );
}

// ═════════════════════════════════════════════════════════════════════════════
//  CZWnd — minimal real MFC window (same skeleton as CXYWnd). Read-only in P5.3
//  (no input handlers): it renders the Z grid + selected-brush Z extents.
// ═════════════════════════════════════════════════════════════════════════════
// ════════════════════════════════════════════════════════════════════════════
//  Z-VIEW MOUSE INTERACTION — opus 2026-06-19
//  The Z (height) window had NO mouse handlers, so you couldn't drag a brush's height
//  in it.  Ports Drag_MouseOrigin (0x49aa60 the Z pick ray), Z_MouseDown (0x49aae0),
//  Z_MouseMoved (0x49abd0) + the CZWnd handlers (0x46eda0/0x46f220/0x46efc0/0x46ee60/
//  0x46f2a0).  The drag itself reuses the already-ported Drag_Begin / Drag_MouseMoved /
//  Drag_MouseUp with viewz=1 (the Z axis).  The binary's single m_Camera global is the
//  port's per-window split, so the Z origin uses z.cpp's m_Camera_origin and the
//  "set camera height" (Ctrl+click) writes the live CCamWnd camera.
// ════════════════════════════════════════════════════════════════════════════
#include "prefs.h"                                                  // g_PrefsDlg (m_nMouseButtons)
extern void  Drag_Begin( void *pressFunc, unsigned int buttons, int viewz, int px, int py,
                         float *xvec, float *yvec, float *trace_start, float *trace_dir ); // 0x47E890
extern void  Drag_MouseMoved( int a1, int a2, int buttons, float *a4, float *a5 );          // 0x47FF30
extern void  Drag_MouseUp( unsigned int buttons );                                          // 0x4802A0
extern int   g_nUpdateBits;                                                                 // 0x25D5A74
extern CMainFrame *g_pParentWnd;                                                            // 0x25D5A70

static int cursorx = 0, cursory = 0;     // screen cursor at RMB press (IDB 0x241a594/0x241a590)

// ── Drag_MouseOrigin (0x49aa60) — build the Z-view pick ray (X = sel brush centre,
//    Z = cursor world height, pointing +Y into the scene) ───────────────────────
static void Drag_MouseOrigin( int flippedY, float *outOrigin, float *outDir )
{
    outOrigin[0] = m_Camera_origin[0];
    outOrigin[1] = m_Camera_origin[1];
    outOrigin[2] = (float)( (double)( flippedY - z_height / 2 ) / z_scale ) + m_Camera_origin[2];
    outOrigin[1] = -262144.0f;
    selbrush_t *sel = selected_brushes.next;
    if ( sel != &selected_brushes && sel->def )
        outOrigin[0] = ( sel->def->maxs[0] + sel->def->mins[0] ) * 0.5f;
    outDir[0] = 0.0f; outDir[1] = 1.0f; outDir[2] = 0.0f;
}

// ── Z_MouseDown (0x49aae0) ────────────────────────────────────────────────────
static void Z_MouseDown( int flippedY, unsigned int nFlags, int x )
{
    POINT pt; GetCursorPos( &pt );
    cursorx = pt.x; cursory = pt.y;
    // xvec={0,0,0}, yvec={0,0,1/z_scale}: the Z view's screen→world drag basis (IDB 0x49aae0
    // builds yvec from the contiguous v10/Point.x/Point.y stack floats = {0,0,1/z_scale}).
    // Drag_Setup AxializeVectors yvec → {0,0,±1}, so mouse-Y maps to world-Z. A zero yvec
    // (the prior port) axializes to {0,0,0} → zero drag delta → the brush never moves.
    float origin[3], dir[3], xvec[3] = { 0, 0, 0 };
    float yvec[3] = { 0.0f, 0.0f, 1.0f / z_scale };
    Drag_MouseOrigin( flippedY, origin, dir );
    int v5 = ( g_PrefsDlg->m_nMouseButtons != 2 ) ? 16 : 2;
    if ( nFlags == 1 || nFlags == 5 || nFlags == 16 || nFlags == (unsigned)( v5 | 0xC ) )
    {
        Drag_Begin( nullptr, nFlags, 1, x, flippedY, xvec, yvec, origin, dir );
    }
    else if ( nFlags == (unsigned)( v5 | 8 ) || nFlags == 9 )   // Ctrl+click → set camera height
    {
        if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
            g_pParentWnd->m_pCamWnd->camera.origin[2] = origin[2];
        g_nUpdateBits |= W_CAMERA | W_XY_OVERLAY | W_Z;
    }
}

// ── Z_MouseMoved (0x49abd0) ───────────────────────────────────────────────────
static void Z_MouseMoved( unsigned int buttons, int flippedY, int x )
{
    if ( !buttons )
        return;
    if ( buttons == MK_LBUTTON )
    {
        if ( g_qeglobals.toggle_unk03_mousedrag_state1 || g_qeglobals.toggle_unk04_mousedrag_state2 )
        {
            g_qeglobals.toggle_unk03_mousedrag_state1 = 0;
            g_qeglobals.toggle_unk04_mousedrag_state2 = 0;
        }
        else
        {
            float origin[3], dir[3];
            Drag_MouseOrigin( flippedY, origin, dir );
            Drag_MouseMoved( x, flippedY, 1, origin, dir );
            g_nUpdateBits |= W_XY | W_Z | W_CAMERA_IFON;
        }
    }
    else if ( buttons == MK_RBUTTON )            // RMB drag → scroll the Z view
    {
        POINT pt; GetCursorPos( &pt );
        if ( pt.y != cursory )
        {
            m_Camera_origin[2] += (float)( pt.y - cursory );
            SetCursorPos( cursorx, cursory );
            g_nUpdateBits |= W_Z;
        }
    }
    else
    {
        unsigned int ctrlCombo = (unsigned)( ( g_PrefsDlg->m_nMouseButtons != 2 ? MK_MBUTTON : MK_RBUTTON ) | MK_CONTROL );
        if ( buttons == ctrlCombo || buttons == (unsigned)( MK_LBUTTON | MK_CONTROL ) )
        {
            float z = (float)( (double)( flippedY - z_height / 2 ) / z_scale );
            if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
                g_pParentWnd->m_pCamWnd->camera.origin[2] = z;
            g_nUpdateBits |= W_CAMERA | W_XY_OVERLAY | W_Z;
        }
    }
}

void CZWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
    SetFocus();
    SetCapture();
    CRect rc; GetClientRect( &rc );
    Z_MouseDown( rc.Height() - point.y - 1, nFlags, point.x );
}

void CZWnd::OnRButtonDown( UINT nFlags, CPoint point )
{
    SetFocus();
    SetCapture();
    CRect rc; GetClientRect( &rc );
    Z_MouseDown( rc.Height() - point.y - 1, nFlags, point.x );
}

void CZWnd::OnMouseMove( UINT nFlags, CPoint point )
{
    CRect rc; GetClientRect( &rc );
    // (Binary also writes the "Z:: %.1f" status-bar text here; deferred — cosmetic.)
    Z_MouseMoved( nFlags, rc.Height() - point.y - 1, point.x );
    CWnd::OnMouseMove( nFlags, point );
}

void CZWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
    Drag_MouseUp( nFlags );
    if ( ( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) == 0 )
        ReleaseCapture();
    CWnd::OnLButtonUp( nFlags, point );
}

void CZWnd::OnRButtonUp( UINT nFlags, CPoint point )
{
    Drag_MouseUp( nFlags );
    if ( ( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) == 0 )
        ReleaseCapture();
    CWnd::OnRButtonUp( nFlags, point );
}

BEGIN_MESSAGE_MAP(CZWnd, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

CZWnd::CZWnd()
{
}

BOOL CZWnd::PreCreateWindow( CREATESTRUCT& cs )
{
    // Own DC + no background brush: we present via D3D, MFC must not paint the client area.
    cs.lpszClass = AfxRegisterWndClass(
        CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor( NULL, IDC_ARROW ),
        NULL,
        NULL );
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    return CWnd::PreCreateWindow( cs );
}

int CZWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    CRect rc;
    GetClientRect( &rc );
    m_nWidth = rc.Width();
    m_nHeight = rc.Height();
    z_width  = m_nWidth;
    z_height = m_nHeight;
    return 0;
}

void CZWnd::OnSize( UINT nType, int cx, int cy )
{
    CWnd::OnSize( nType, cx, cy );
    m_nWidth  = cx;
    m_nHeight = cy;
    z_width   = cx;
    z_height  = cy;
    // P5.3: re-create this window's swap chain at the new size (pixel-correct, no stretch).
    if ( dx.device && cx > 0 && cy > 0 )
        R_Hwnd_Resize( (HWND__ *)GetSafeHwnd(), cx, cy );
}

BOOL CZWnd::OnEraseBkgnd( CDC* /*pDC*/ )
{
    return TRUE;   // suppress GDI erase — the D3D present owns the client area
}

// The CZWnd::OnPaint pipeline (IDB 0x46eec0) — identical structure to CXYWnd::OnPaint,
// targeting d_hwndZ. Grid + selected Z-extents subset.
void CZWnd::OnPaint()
{
    CPaintDC dc( this );

    if ( !dx.device )
        return;

    // Keep z_width/z_height in step with this window (a WM_SIZE may not have arrived yet).
    z_width  = m_nWidth;
    z_height = m_nHeight;

    HWND__ *hwnd = (HWND__ *)GetSafeHwnd();
    if ( !R_SetupRendertarget_CheckDevice( hwnd ) )
        return;

    R_BeginFrame();
    R_BeginSharedCmdList();
    R_AddCmdClearScreen( 7, g_qeglobals.d_savedinfo.colors[1], 1.0f, 0 );
    static const float s_edWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    R_AddCmdSetMaterialColor( s_edWhite );

    Z_Draw();

    R_EndFrame();
    R_IssueRenderCommands( (uint32_t)-1 );
    R_SortMaterials();
    R_CheckTargetWindow( hwnd );
}
