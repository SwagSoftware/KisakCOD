#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif

// ─────────────────────────────────────────────────────────────────────────────
// qedefs.h — CoD4Radiant (IW3xRadiant) editor primitive types & enums.
// Phase 3 (plan §5). Every struct here is ported field-for-field from the
// CoD4Radiant IDB (port 13346) and carries a permanent static_assert(sizeof)
// (plus offsetof asserts on the load-bearing fields) — these are the layout
// regression net for all pointer-walking editor code ported in Phase 4+.
//
// Layout authority is ALWAYS the IDB. GtkRadiant 1.6 supplies field *names*
// only. Where the IDB's enthusiast names were placeholder (xxNN), they are kept
// verbatim so offsets stay traceable; rename as Phase 4 identifies real uses.
//
// kisak does NOT define vec2_t/vec3_t/drawVert_t anywhere (verified) — they are
// defined here; CoD asset types (Material/GfxImage/...) come from kisak headers
// via qe3.h and are never redeclared.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>

// Material texture channels: 0=$default, 1=lightmap_gray, 2=smoothing_hard.
// (IDB asserts reference QER_TEX_CHAN_COUNT in Face_InitMaterialChannel @0x472C90.)
#define QER_TEX_CHAN_COUNT 3

// selbrush_t.brushFlags (0x34) bits. BRUSHFLAG_SELECTED = bit 7 (0x80), the value the
// IDB asserts test (Brush_Deselect/Select_Helper "b->brushFlags & BRUSHFLAG_SELECTED").
#define BRUSHFLAG_SELECTED 0x80
#include <cstddef>   // offsetof

// ── vector primitives (IDA's vec3_t/vec2_t; absent from the kisak tree) ───────
#ifndef KISAK_RADIANT_VEC_DEFINED
#define KISAK_RADIANT_VEC_DEFINED
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];
#endif

// ── window update bits (g_nUpdateBits / Sys_UpdateWindows) — Q3Radiant qedefs ──
// Sys_UpdateWindows(bits) ORs these into g_nUpdateBits; CMainFrame::UpdateWindows
// (flushed each idle by RoutineProcessing) RedrawWindow's the matching views.
// Values verified against the binary's CMainFrame::UpdateWindows (0x427090) RAW masks:
//   `test bl, 6` = XY group; `test bl, 1`/`test ebx, 100h` = camera; `test bl, 28h`
//   = Z group (so W_Z_OVERLAY == 0x20); `test bl, 10h` = texture (W_TEXTURE == 0x10).
// W_TEXTURE/W_Z_OVERLAY were once SWAPPED here (0x20/0x10);
// symbolic set+check sites still worked (both flipped together), but every RAW binary
// immediate kept in the port (0x10 texture sets in layeredmaterial code, camwnd 0x21,
// mainfrm 0x28) landed on the WRONG window.  Re-aligned to the binary values — do not
// swap them back.
#ifndef W_CAMERA
#define W_CAMERA        0x0001
#endif
#ifndef W_XY
#define W_XY            0x0002
#endif
#ifndef W_XY_OVERLAY
#define W_XY_OVERLAY    0x0004
#endif
#ifndef W_Z
#define W_Z             0x0008
#endif
#ifndef W_TEXTURE
#define W_TEXTURE       0x0010
#endif
#ifndef W_Z_OVERLAY
#define W_Z_OVERLAY     0x0020
#endif
#ifndef W_CAMERA_IFON
// 0x0100, NOT 0x40 — verified vs CMainFrame::UpdateWindows disasm (0x4270c7 `test ebx, 100h`)
// and the QERadiant convention.  The XY/Z camera-drag arms set g_nUpdateBits|=0x104 (raw IDB
// value = W_XY_OVERLAY|W_CAMERA_IFON); with the old 0x40 the macro check never matched the
// 0x100 bit, so the 3D camera didn't redraw in realtime during a middle-mouse camera turn.
#define W_CAMERA_IFON   0x0100
#endif
#ifndef W_ALL
#define W_ALL           0xFFFFFFFF
#endif

// ── inspector mode (inspector_mode @ IDB 0x240a110) — CEntityWnd_SetInspectorMode ─
// These are a SEPARATE constant set from the g_nUpdateBits invalidation bits above
// (the IDB renders them through its UPDATEBITS enum — QE/GtkRadiant convention
// W_TEXTURE=0x10/W_CONSOLE=0x40/W_ENTITY=0x80; the
// invalidation W_TEXTURE is also 0x10, but keep the namespaces separate).
// Read as RAW immediates from SetInspectorMode (0x496b00):
// the inspector window (d_hwndEntity) shows ONE pane-group at a time, selected by the
// tab control (Entities/Filters/Textures/Console).  Named INSPECTOR_* to avoid the
// W_TEXTURE macro collision.
#define INSPECTOR_ENTITY    0x0080   // "Entity"   — eclass list + key/value editor
#define INSPECTOR_TEXTURE   0x0010   // "Textures" — the texture browser
#define INSPECTOR_CONSOLE   0x0040   // "Console"  — the editor log
#define INSPECTOR_FILTER    0x2000   // "Filters"  — the filter dialog content
// 0x0800 = W_GROUP "Group" exists in the enum but its SetInspectorMode branch is dead
// in this build (OnViewGroups is unwired); only reachable via the floating-style tab 2.

// ── selection mode (g_qeglobals.d_select_mode) — IDB enum select_t ────────────
enum select_t : int
{
    sel_brush                       = 0,
    sel_vertex                      = 1,
    sel_edge                        = 2,
    sel_singlevertex                = 3,
    sel_curvepoint                  = 4,
    sel_area                        = 5,
    sel_terrainpoint                = 6,
    sel_terraintexture              = 7,
    sel_addpoint                    = 8,
    sel_cycle_edge_direction_quad   = 9,
    sel_editpoint                   = 10,
    // ── transient marquee / point-edit RECTANGLE modes (11..15) ───────────────
    // These are NOT named in the IDB enum (only 0..10 are). Drag_Setup computes
    // the active drag-rectangle mode arithmetically and stores the raw int into
    // d_select_mode; Drag_MouseUp / Drag_MouseMoved then dispatch on the value.
    //  - 12 / 13 are the BRUSH marquee box-select modes (the win):
    //      Drag_Setup (0x47e57b) does d_select_mode = ((buttons & 2) | 0x18) >> 1
    //      → 12 when buttons&2==0 (Alt+Shift+LMB drag  = box ADD-select),
    //        13 when buttons&2!=0 (Alt+Shift+MMB drag  = box DESELECT).
    //      Drag_MouseUp (0x480484/0x48056d/0x48057b) builds the world box from the
    //      drag rectangle and calls sub_4902C0 (Select_FlipFilteredBrushes) with
    //      bActiveList = 1 for 12 (move active→selected) / 0 for 13 (selected→active).
    //  - 11/14/15 are the patch/terrain point-edit rectangle siblings
    //      (sel_editpoint|sel_vertex, |sel_curvepoint, |sel_area) — only used by
    //      the Drag_MouseMoved drag-box-update condition (point editing).
    //  Values match the IDB's inline integer compares (cmp edi,0Ch/0Dh, etc.).
    sel_areapoint_vertex            = 11,   // sel_editpoint | sel_vertex
    sel_areabrush                   = 12,   // marquee box-select ADD   (Alt+Shift+LMB)
    sel_areabrush_sub               = 13,   // marquee box-select DESELECT (Alt+Shift+MMB)
    sel_areapoint_curve             = 14,   // sel_editpoint | sel_curvepoint
    sel_areapoint                   = 15,   // sel_editpoint | sel_area
};

// ── patch kind flags (patchMesh_t.type) — IDB enum PATCH_TYPES ────────────────
enum PATCH_TYPES : int
{
    PATCH_CYLINDER   = 0x1,
    PATCH_BEVEL      = 0x2,
    PATCH_ENDCAP     = 0x4,
    PATCH_HEMISPHERE = 0x8,
    PATCH_CONE       = 0x10,
    PATCH_TRIANGLE   = 0x20,
    PATCH_TERRAIN    = 0x40,
    PATCH_CAP        = 0x1000,
    PATCH_SEAM       = 0x2000,
    PATCH_THICK      = 0x4000,
};

// ── 4-byte vertex color (drawVert_t.vert_color) ───────────────────────────────
struct rgba_4byte
{
    char r;
    char g;
    char b;
    char a;
};
static_assert(sizeof(rgba_4byte) == 4, "rgba_4byte");

// ── per-vertex texcoord triple (drawVert_t.texCoord/savedTexCoord) ────────────
struct pmesh_texcoord
{
    vec2_t st;          // 0x00
    vec2_t lightmap;    // 0x08
    vec2_t smoothing;   // 0x10
};
static_assert(sizeof(pmesh_texcoord) == 24, "pmesh_texcoord");

// ── brush face plane (face_t.plane) ───────────────────────────────────────────
struct plane_t
{
    vec3_t normal;      // 0x00
    int    type;        // 0x0C
    float  dist;        // 0x10
    float  unk;         // 0x14
};
static_assert(sizeof(plane_t) == 24, "plane_t");

// ── texture-projection sub-block (embedded in MaterialDef.mat_texDef) ─────────
struct texdef_sub_t
{
    float size[2];      // 0x00
    float shift[2];     // 0x08
    float rotate;       // 0x10
    int   unk3;         // 0x14
    float sample_size;  // 0x18
};
static_assert(sizeof(texdef_sub_t) == 28, "texdef_sub_t");

// ── brush face winding ────────────────────────────────────────────────────────
// winding_t (`{int numpoints; float p[4][3];}`, sizeof 0x34) is ALREADY defined
// by kisak in qcommon/qcommon.h and is byte-identical to the editor's IDB
// winding_t — so it is NOT redefined here. qe3.h pulls it in via r_material.h ->
// qcommon.h and pins sizeof(winding_t)==52 there. (Windings are allocated as a
// header + numpoints*vec3; the 4-point base is the in-struct minimum.)

// ── entity key/value pair ─────────────────────────────────────────────────────
struct epair_t
{
    epair_t *next;      // 0x00
    char    *key;       // 0x04
    char    *value;     // 0x08
};
static_assert(sizeof(epair_t) == 12, "epair_t");

// ── vertex/edge selection point (drag.cpp / select.cpp) ───────────────────────
struct BrushPt_t
{
    vec3_t xyz;         // 0x00
    int    sideIndex[3];// 0x0C
};
static_assert(sizeof(BrushPt_t) == 24, "BrushPt_t");
