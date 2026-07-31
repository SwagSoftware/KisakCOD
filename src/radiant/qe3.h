#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif

// ─────────────────────────────────────────────────────────────────────────────
// qe3.h — CoD4Radiant (IW3xRadiant) editor object model.
// Phase 3 (plan §5). Layouts are ported field-for-field from the CoD4Radiant IDB
// (port 13346) and pinned with permanent static_assert(sizeof) + offsetof asserts
// on load-bearing fields (the layout regression net for Phase 4 pointer-walking
// code). IDA is the authority; GtkRadiant 1.6 supplies field names only.
//
// CoD asset types (Material/GfxImage/MaterialTechniqueSet) come from the kisak
// headers below — never redeclared — and their sizes are asserted against the IDB.
// ─────────────────────────────────────────────────────────────────────────────

#include <windows.h>            // HWND / HINSTANCE (qeglobals_t window handles)
#include "qedefs.h"             // vec*, enums, winding_t, plane_t, texdef_sub_t, ...

// CoD asset types — kisak headers, never redeclared (sizes asserted at bottom).
#include <gfx_d3d/r_material.h> // Material, MaterialTechniqueSet
#include <gfx_d3d/r_image.h>    // GfxImage

// winding_t comes from qcommon.h (pulled in by r_material.h) — kisak's definition
// is byte-identical to the editor IDB's. Pin it as the rest of the regression net.
static_assert(sizeof(winding_t) == 52, "winding_t (qcommon.h) != IDB layout");

// ── forward declarations ──────────────────────────────────────────────────────
struct entity_s;
struct brush_t;
struct selbrush_t;         // canonical 56-byte brush-instance / list node (ruling below)
struct patchMesh_t;
struct eclass_t;
struct entitymodel_t;      // eclass-like model node (defined below)
struct LayerMaterialDef;   // pointer-only (Phase 6 layeredmaterials)
struct undo_s;             // forward-decl; full definition below (after entity_s)
struct Material;           // gfx_d3d r_material.h — referenced pointer-only by faceVisuals_s

// ── qtexture_s — a registered editor texture/material wrapper (TexWnd.cpp) ──────
// 40 bytes, IDB ground truth (search_structs qtexture_s ordinal 463, verified via
// Texture_GetHandle 0x45a8e0 / Editor_AddRadiantMaterial 0x45a5b0). This is the
// CANONICAL definition: brush.cpp / csg.cpp / pmesh.cpp / materialdef.cpp used to
// each carry a TU-local copy and TWO of them had the GtkRadiant char[64]@0 layout
// (name-ptr@4 / width@0x14 is the truth — the documented landmine). Centralised
// here so the layout can never drift again. The fields are load-bearing: the
// renderer fills them in Editor_AddRadiantMaterial and Brush_BuildWindings / csg /
// pmesh read width/height/in_use/color_or_surfacetype_filter back out.
struct qtexture_s
{
    union {                                   // 0x00  registered render material handle
        Material *next;                       //       (port's historical name)
        Material *handle;                     //       (the binary's name — assert strings)
    };
    const char  *name;                        // 0x04
    bool         is_in_use;                   // 0x08
    char         unk1;                         // 0x09  (MaterialDef_07 ANDs this)
    char         usage_index;                  // 0x0A
    char         unk2;                         // 0x0B
    int          unk_flags2;                   // 0x0C  (MaterialDef_06 low word; MaterialDef_15)
    int          tex_num_or_localefilter;      // 0x10  (MaterialDef_05 ANDs this)
    union {                                    // 0x14/0x18  (autoTexScale)
        struct { int width; int height; };
        int size[2];                           //   (the binary's name — assert strings)
    };
    int          color_or_surfacetype_filter;  // 0x1C  (MaterialDef_09 ANDs this)
    int          in_use;                       // 0x20  (contents; MaterialDef_08 ANDs this)
    qtexture_s  *prev;                         // 0x24  (texWndGlob list link)
};
static_assert(sizeof(qtexture_s) == 40, "qtexture_s must be 40 bytes (IDB ordinal 463)");

// ── patch control-point vertex (editor-only; absent from the kisak tree) ───────
struct drawVert_t
{
    vec3_t         xyz;           // 0x00
    pmesh_texcoord texCoord;      // 0x0C
    vec3_t         normal;        // 0x24
    rgba_4byte     vert_color;    // 0x30
    pmesh_texcoord savedTexCoord; // 0x34
    int            turned_edge;   // 0x4C
};
static_assert(sizeof(drawVert_t) == 80, "drawVert_t");

// ── tessellated patch RENDER mesh (curveDef) — editor-only; absent from kisak ──
//    Built by Patch_GenericMesh2 from the control grid; read by the patch wireframe
//    drawer (DrawPatchesWireframeGrid) and the filled draw.  IDB ordinals
//    curveVert_t(44) / curvePatchDef_t(20).
struct curveVert_t
{
    vec3_t     xyz;           // 0x00
    vec2_t     st;            // 0x0C  texture coords (layer-selected at tessellate time)
    vec2_t     lightmap;      // 0x14  ("unk" in IDB; zeroed by the tessellator)
    vec3_t     normal;        // 0x1C
    rgba_4byte vert_color;    // 0x28
};
static_assert(sizeof(curveVert_t) == 44, "curveVert_t");

struct curvePatchDef_t
{
    int          width;       // 0x00  tessellated grid width
    int          height;      // 0x04  tessellated grid height
    int          random_one;  // 0x08
    curveVert_t *verts;       // 0x0C  width*height verts (allocated immediately after)
    int          unk_after;   // 0x10
};
static_assert(sizeof(curvePatchDef_t) == 20, "curvePatchDef_t");

// ── per-face / per-patch material binding ─────────────────────────────────────
// MaterialDef = the two material pointers + an inline texture-projection block.
struct MaterialDef
{
    LayerMaterialDef *lyrMtl;     // 0x00  layered-material def (editor)
    qtexture_s       *radMtl;     // 0x04  radiant texture wrapper
    texdef_sub_t      mat_texDef; // 0x08
};
static_assert(sizeof(MaterialDef) == 36, "MaterialDef");

// patchMesh_t.texture/lightmap/smoothing — the pointer pair without the texDef.
struct patchMesh_material
{
    LayerMaterialDef *lyrMtl;     // 0x00
    qtexture_s       *radMtl;     // 0x04
};
static_assert(sizeof(patchMesh_material) == 8, "patchMesh_material");

// ── face texture definition (face_t.texdef points at the sub-block; texdef_t is
//    the full per-surface record used by the surface inspector) ────────────────
struct texdef_t
{
    MaterialDef mtlDef;               // 0x00  (binary name — assert strings)
    int         unk3;                 // 0x24
    float       sample_size;          // 0x28
    int         xx001;                // 0x2C
    int         xx002;                // 0x30
    int         xx003;                // 0x34
    int         xx004;                // 0x38
    int         xx005;                // 0x3C
    int         xx006;                // 0x40
    int         texdef_0x24_question; // 0x44
};
static_assert(sizeof(texdef_t) == 72, "texdef_t");

// ── brush face (IDB face_t_new — the corrected 232-byte layout; the older IDB
//    `face_t` flattened the MaterialDef[4] block and mislabeled contents@0x24) ──
struct face_t
{
    vec3_t      planepts[3];   // 0x00  three points defining the plane
    union {                    // 0x24  base / lightmap / smoothing / unused layers
        MaterialDef mtldef[4];                       // (port's historical name)
        struct { MaterialDef mtlDef; } surfDef[4];   // (the binary's names — assert strings)
    };
    int         contents;      // 0xB4
    int         toolflags;     // 0xB8
    int         unk02;         // 0xBC
    plane_t     plane;         // 0xC0
    int         unk03;         // 0xD8
    int         unk04;         // 0xDC
    winding_t  *w;             // 0xE0  computed winding
    union {                    // 0xE4  packed per-face RGBA (IDB "minus1____totalsize_0xE8")
        int packedColor;
        int field_0xE4;        //       (historical name; init -1)
    };
};
static_assert(sizeof(face_t) == 232, "face_t");
static_assert(offsetof(face_t, mtldef) == 36,  "face_t.mtldef");
static_assert(offsetof(face_t, contents) == 180, "face_t.contents");
static_assert(offsetof(face_t, plane) == 192,  "face_t.plane");
static_assert(offsetof(face_t, w) == 224,      "face_t.w");

// ── per-face render visuals (IDB faceVis_s, 12 bytes) ─────────────────────────
// Each selbrush_t.faces is a heap array of faceVis_s with faceCount elements.
// faceVis_s was originally local to brush.cpp; moved here so select.cpp can
// address individual face entries via ((faceVis_s*)b->faces)[i].
struct faceVisuals_s
{
    Material *mtlHandle;  // 0x00  the layer's render material (IDB "mtlHandle")
    int       vertHandle; // 0x04  editor vertex-buffer handle
};
static_assert(sizeof(faceVisuals_s) == 8, "faceVisuals_s");

struct faceVis_s
{
    int           vertcount; // 0x00
    int           visCount;  // 0x04
    faceVisuals_s *visArray; // 0x08
};
static_assert(sizeof(faceVis_s) == 12, "faceVis_s");

// ── vertex/edge-selection edge record (g_qeglobals.d_edges) ───────────────────
// IDB pedge_t (16 bytes): two point indices into d_points + the (up to two) faces
// that share the edge. Built by FindEdge during SetupVertexSelection; read by the
// edge-handle draw and Select_Edge. p1/p2 index d_points[]; f1/f2 are face_t*.
struct pedge_t
{
    int     p1;   // 0x00  index into g_qeglobals.d_points
    int     p2;   // 0x04
    face_t *f1;   // 0x08  first face sharing this edge
    face_t *f2;   // 0x0C  second face (set when the reverse edge is found)
};
static_assert(sizeof(pedge_t) == 16, "pedge_t");

// ── EdLayerGeom — Stage 1d editor surf-cache per-layer scratch (NOT an IDB
//    layout). Holds one material layer's faithful per-vertex geometry, computed
//    by Face_BuildLayerGeom (the Visuals_InitFaceVis 0x46F7A0 inner body). The
//    surf-cache draw (Cam_DrawFaceCached, RADIANT_SURFCACHE) and the faithful
//    immediate draw (Cam_DrawFace under RADIANT_FAITHFUL_TEX) BOTH derive their
//    geometry from this helper so they emit bit-identical pixels (the Stage-1d
//    RADIANT_SURFCACHE pixel-identity gate). A brush face has at most
//    MAX_POINTS_ON_WINDING points; 64 is ample for convex brush faces. ────────
struct EdLayerGeom
{
    Material    *material;   // MaterialDef_14(layer) handle
    int          vertcount;  // winding point count
    float        xyz[64][3]; // world position (OrientationPosToWorldPos)
    float        normal[64][3];
    float        tangent[64][3];
    float        binormal[64][3];
    float        st[64][2];  // real texdef texcoords (Face_MoveTexture)
    unsigned int color[64];  // packed per-face color (sub_46F6C0)
};
// Build one material layer's faithful per-vertex geometry for `faceDef`. Returns
// false (emit nothing) when the layer has no winding or texdef (defensive).
bool Face_BuildLayerGeom( face_t *faceDef, const orientation_t *orient,
                          int layer, EdLayerGeom *out );

// ── selface_t — per-face selection record (12 bytes, IDB-verified) ──────────
// Defined here (moved from select.cpp local) so drag.cpp can use it.
// IDA layout: {brush@0, face@4, index@8} — brush is the selbrush_t*, face is
// the faceVis_s*, index is the face index within the brush def.
struct selface_t
{
    selbrush_t *brush;  // 0x00
    faceVis_s  *face;   // 0x04
    int         index;  // 0x08
};
static_assert(sizeof(selface_t) == 12, "selface_t");

// ── g_SelectedFaces — the face-selection array ───────────────────────────────
// IDB: MFC CArray<selface_t, selface_t&> at 0x73C70C (m_pData@0x73C710,
// m_nSize@0x73C714, m_nMaxSize@0x73C718).  The binary's assert strings reference
// it by name ("g_SelectedFaces.GetAt( i ).brush", "g_SelectedFaces.GetSize()"),
// so this is the original global's identity; the port used to model it as three
// raw globals (selFace / g_ptrSelectedFaces_GetSize / g_selFaceSize).  The
// `i < 0 || i >= size → unknown_libname_291()` checks the binary inlines at every
// access site are MFC's ENSURE inside GetAt/SetSize — they live here now instead
// of being copy-pasted at each call site.
void unknown_libname_291();     // MFC ENSURE abort (engine_stubs.cpp)
struct SelectedFaceArray
{
    selface_t *m_pData    = nullptr;   // 0x73C710
    int        m_nSize    = 0;         // 0x73C714  live count
    int        m_nMaxSize = 0;         // 0x73C718  allocated capacity
    int GetSize() const { return m_nSize; }
    int GetCount() const { return m_nSize; }   // MFC alias (drag.cpp:270 string)
    selface_t &GetAt( int i )
    {
        if ( i < 0 || i >= m_nSize ) unknown_libname_291();
        return m_pData[i];
    }
    void SetSize( int n );               // select.cpp (sub_494610)
    int  Add( const selface_t &f );      // select.cpp (sub_4947A0 append)
    void RemoveAt( int i, int count );   // select.cpp (sub_480670)
};
extern SelectedFaceArray g_SelectedFaces;

// IDB surfDlgGlob (0x23F1624) — the surface-inspector global block; .hwnd is the
// inspector HWND when open, 0 when closed (the binary's assert strings name it).
// The port's old `int g_surfwin` was this member.
struct SurfDlgGlob_t { int hwnd; };
extern SurfDlgGlob_t surfDlgGlob;

// IDB lyrMtlGlob (0x1814CF8) — the layered-material library block (assert strings
// name its members).  Entry stride 84 bytes, 512 entries (== layeredmaterials.cpp's
// LYR_ENTRY_SIZE/LYR_MAX_ENTRIES, static_asserted at the definition).
struct LyrMtlGlob_t
{
    int     crcToken;              // 0x1814CF8  clean-library CRC (was dword_1814CF8)
    int     entryCount;            // 0x1814CFC
    uint8_t Layers[512 * 84];      // 0x1814D00  inline array
};
extern LyrMtlGlob_t lyrMtlGlob;    // layeredmaterials.cpp

// ── LayeredMaterialWnd global state (IDB lyrMtlWndGlob @ 0x181F500) ───────────
// Field names from the binary's assert strings (LayeredMaterialWnd.cpp).
struct LyrMtlWndGlob_t
{
    HWND hwnd;               // 0x00  frame window
    HWND toolbar;            // 0x04  COMCTL32 toolbar child
    HWND layerList;          // 0x08  custom layer-list child
    int  liveAddActive;      // 0x0C  "clicking adds a layer" flag (only low byte used)
    int  activeLyrMtl;       // 0x10  pointer-as-int into lyrMtlGlob.Layers (IDB type)
    int  selectedLayerIndex; // 0x14
};
extern LyrMtlWndGlob_t lyrMtlWndGlob;   // layeredmaterialwnd.cpp

// ── editor draw-filter flags + skinned-surface sentinels ─────────────────────
// Names from the binary's assert strings (r_ed_scene.cpp Editor_SurfFilter /
// SkinModelInst, MaterialDef.cpp MaterialDef_15).  The multiply-pair assert is
// symmetric in the two flags, and the filter comment maps 4=effect, 8=opaque.
#define DRAWFLAG_ONLY_MULTIPLY 4
#define DRAWFLAG_SKIP_MULTIPLY 8
#define RIGID_SKINNED_CACHE_OFFSET (-2)
#define MAX_POINTS_ON_WINDING 1024
enum { PM_FRONT_FACE = 0, PM_BACK_FACE = 1 };   // patch index-buffer slots (assert strings)

// ── LinkList_t — Map_ParseLinkList's parse buffer (qe3.cpp:567 names links.size) ──
struct LinkList_t
{
    int  id[1024];     // 0x0000  parsed link ids; id[size] = -1 terminator
    int  size;         // 0x1000  live count
    bool overflowed;   // 0x1004  set when the 30-entry parse cap is hit
};
void Map_ParseLinkList( LinkList_t *buf, const char *linkTo );   // qe3.cpp 0x48BE20        // the winding cap the brush.cpp assert strings cite
#define MAX_LINKS 4096   // script_link number census cap (map.cpp assert strings cite it)
#define HIDDEN_SURFACE_OFFSET      (-3)
#define ECLASS_PREFAB 0x10                // eclass_t.nShowFlags prefab-class bit (assert strings)

// ── edTrace_t — editor ray-pick result (IDB trace_t, 88 bytes) ───────────────
// Filled by Test_Ray. hit@0 {brush, face, index}, dist@68, selected@72,
// normal@76. The binary's assert strings ("t.hit.face", "t.hit.brush->version ==
// t.hit.brush->def->version") show the original nested the hit record as a
// selface_t member named `hit` — layout-identical to the flat brush/face/xx1 the
// port used before. (Named edTrace_t, not trace_t, because q_shared.h's engine
// trace_t — 44 bytes, totally different — is already in scope via stdafx.h.)
struct edTrace_t
{
    selface_t hit;             // 0x00  hit record {brush@0, face@4, index@8}
    int   xx2, xx3, xx4;       // 0x0C..0x14  (xx4 begins a 0x30 orientation copy)
    int   xx5, xx6, xx7, xx8;  // 0x18..0x24
    int   xx9, xx10, xx11;     // 0x28..0x30
    short xx12, xx12_2;        // 0x34
    int   xx13, xx14, xx15;    // 0x38..0x40
    float dist;                // 0x44  param along dir to the hit
    bool  selected;            // 0x48  hit brush was in the selected list
    char  _pad[3];             // 0x49
    vec3_t normal;             // 0x4C  hit face normal
};
static_assert(sizeof(edTrace_t) == 88, "edTrace_t");
static_assert(offsetof(edTrace_t, dist) == 68, "edTrace_t.dist");
static_assert(offsetof(edTrace_t, selected) == 72, "edTrace_t.selected");
static_assert(offsetof(edTrace_t, normal) == 76, "edTrace_t.normal");

// ═════════════════════════════════════════════════════════════════════════════
//  RULING: the CoD4Radiant brush list model.
//
//  CoD4Radiant splits GtkRadiant's monolithic brush_s into TWO distinct objects:
//
//    1. selbrush_t  — the 56-byte "brush instance" / list node (THIS struct).
//    2. brush_t     — the 88-byte brush DEFINITION (geometry: faces, planepts,
//                     contents, patch, refCount, parent_layer_string — see below).
//
//  A selbrush_t instance points at its definition through .def (0x14); many
//  instances may share one brush_t def (def->refCount). Evidence:
//    * Brush_Alloc (0x4751e0)  : operator new(0x58) → the 88-byte DEF.
//    * Brush_AddToList (0x475980 — misnamed; really "instance-for-def"):
//        operator new(0x38) → the 56-byte selbrush_t, sets .def, ++def->refCount,
//        then links the instance into its entity via sub_475730.
//    * Brush_Free (0x475ba0)   : frees the instance, --def->refCount, and frees the
//        DEF via Brush_Free_R only when refCount hits 0.
//
//  Each selbrush_t lives in TWO doubly-linked lists at once — exactly GtkRadiant's
//  brush_s next/prev + onext/oprev, just at different offsets:
//    * the global DISPLAY list  via prev(0x00)/next(0x04) — selected_brushes /
//      active_brushes / filtered_brushes
//      (Brush_AddToList2 0x4765a0, Brush_RemoveFromList 0x476680).
//    * the owning ENTITY's brush list via ownerNext(0x0C)/ownerPrev(0x10), headed
//      by the entity's embedded sentinel entity_s.brushes
//      (combined Entity link/unlink = sub_475730 @ 0x475730).
//    owner(0x08) is the parent entity_s* (set by sub_475730; read by Brush_Free).
//
//  SENTINELS: selected_brushes(0x23F1864), active_brushes(0x23F189C),
//  filtered_brushes(0x23F182C) are EMBEDDED 56-byte selbrush_t nodes — spaced
//  exactly 0x38 apart in .bss; the IDB types them `selbrush_t*`/`entity_brush_s*`
//  only because just their first two link DWORDs (base, base+4) are individually
//  named. They are NOT 88-byte brush_t and NOT bare pointers. Iterate them as
//  `for (b = sel.next; b != &sel; b = b->next)`.
//
//  IDB CAVEAT: the database carries TWO enthusiast models of this one node —
//  `selbrush_t`(64) and `entity_brush_s`(56). entity_brush_s had the right SIZE but
//  mislabeled 0x00/0x04 as oprev/onext (they are the display prev/next) and
//  0x20/0x2C as mins/maxs (they are patch/version/...). selbrush_t had the right
//  FIELD interpretation (patch@0x20, version@0x24(i16), faces@0x1C — all confirmed
//  by live code) but was over-padded to 64 (xx9/xx10 @0x38/0x3C do not exist; the
//  allocation, the embedded entity_s.brushes span, and the sentinel spacing are all
//  0x38=56). The struct below is the reconciled truth: 56 bytes, selbrush_t naming.
//  `entity_brush_s` is kept as a typedef alias — entity_s embeds this node by value
//  as its brush-list head, and qeglobals_t.d_select_order points at instances.
// ═════════════════════════════════════════════════════════════════════════════
//  patch INSTANCE node (IDB pPatch_t, 68 bytes) — selbrush_t.patch points here;
//  created by PMESH_55 (the patch analogue of Brush_AddToList).  Distinct from
//  patchMesh_t (the DEF): instance.def == the owning brush_t.patch (the runtime
//  `b->patch->def == b->def->patch` invariant).  Only `def` and the per-instance
//  `selected` flag are used by the port; the remainder is opaque IDB padding.
//  (Was accessed via raw casts `*(patchMesh_t**)inst` / `*((uint8_t*)inst+6)` before.)
// pPatch_t visuals (0x08..0x1C) — the FILLED per-layer surf-cache state, built by
// Patch_BuildInstanceVisuals (0x440240) and consumed by PMESH_27/26 -> Editor_AddMeshCmd.
// Field offsets reconstructed from PMESH_21_Indices (0x43f8c0) / PMESH_24 (0x4401a0) /
// PMESH_23_ColorTint (0x43fd40) / Patch_BuildInstanceVisuals: {material,vertHandle} pair.
struct patchVisuals_s
{
    Material *material;   // 0x00  MaterialDef_14(layer) handle
    int       vertHandle; // 0x04  Editor_VB_Upload return (buffer<<16 | firstIndex)
};
static_assert(sizeof(patchVisuals_s) == 8, "patchVisuals_s");

struct patch_t
{
    patchMesh_t     *def;          // 0x00  the patch DEF (symbiont patchMesh_t)
    __int16          version;      // 0x04  instance version (Patch_AllocInstance: def->version-1; rebuild trigger)
    unsigned char    selected;     // 0x06  per-instance selected flag (BYTE2 of dword@+4 in IDB)
    unsigned char    pad_07;       // 0x07
    int              vertCount;    // 0x08  a1[2]  tessellated vert count (width*height)
    int              indexCount;   // 0x0C  a1[3]  triangle index count = (h-1)*(6w-6)
    union {                        // 0x10/0x14 — the source indexed these as indices[2]
        struct {
            uint16_t *indicesFront; // 0x10  a1[4]  PM_FRONT_FACE index buffer
            uint16_t *indicesBack;  // 0x14  a1[5]  PM_BACK_FACE  index buffer (reverse)
        };
        uint16_t *indices[2];
    };
    int              visCount;     // 0x18  a1[6]  per-layer material count (MaterialDef_11)
    patchVisuals_s  *visArray;     // 0x1C  a1[7]  visCount * {material, vertHandle}
    unsigned char    pad_20[36];   // 0x20  (to the 68-byte IDB instance size)
};
static_assert(sizeof(patch_t) == 68, "patch_t (pPatch_t instance) != 68");
static_assert(offsetof(patch_t, vertCount)  == 8,  "patch_t.vertCount");
static_assert(offsetof(patch_t, indicesFront) == 16, "patch_t.indicesFront");
static_assert(offsetof(patch_t, visArray)   == 28, "patch_t.visArray");

struct selbrush_t
{
    selbrush_t *prev;        // 0x00  global DISPLAY list (selected/active/filtered)
    selbrush_t *next;        // 0x04  global DISPLAY list
    entity_s   *owner;       // 0x08  parent entity
    selbrush_t *ownerNext;   // 0x0C  owning-entity brush list (GtkRadiant onext)
    selbrush_t *ownerPrev;   // 0x10  owning-entity brush list (GtkRadiant oprev)
    brush_t    *def;         // 0x14  → the 88-byte brush definition (geometry)
    int         faceCount;   // 0x18  cached visible-face count (node copy of def->faceCount)
    faceVis_s  *faces;       // 0x1C  cached per-instance visibility faces (faceVis_s[faceCount])
    patch_t    *patch;       // 0x20  patch instance (PMESH_55 of def->patch); ->def, ->selected
    __int16     version;     // 0x24
    bool        cullFlag;    // 0x26
    bool        unk_bool;    // 0x27
    int         xx5;         // 0x28  skip-flag tested by CSG_MakeHollow
    int         xx6;         // 0x2C  (zeroed on add-to-selection — Brush_AddToList2)
    int         xx7;         // 0x30  (zeroed on add-to-selection — Brush_AddToList2)
    int         brushFlags;  // 0x34  low 5 bits cleared on select; bit7 gates UI on remove
};
static_assert(sizeof(selbrush_t) == 56,              "selbrush_t (brush-instance node) != 0x38");
static_assert(offsetof(selbrush_t, owner) == 8,      "selbrush_t.owner");
static_assert(offsetof(selbrush_t, ownerNext) == 12, "selbrush_t.ownerNext");
static_assert(offsetof(selbrush_t, ownerPrev) == 16, "selbrush_t.ownerPrev");
static_assert(offsetof(selbrush_t, def) == 20,       "selbrush_t.def");
static_assert(offsetof(selbrush_t, faceCount) == 24, "selbrush_t.faceCount");
static_assert(offsetof(selbrush_t, patch) == 32,     "selbrush_t.patch");
static_assert(offsetof(selbrush_t, version) == 36,   "selbrush_t.version");
static_assert(offsetof(selbrush_t, brushFlags) == 52,"selbrush_t.brushFlags");

// entity_s embeds this node as its brush-list head; d_select_order holds pointers
// to it. Same 56-byte node, different role → an alias, not a second struct.
typedef selbrush_t entity_brush_s;

// ── brush (the live geometry brush). RECONCILIATION ───────────────────────────
// The IDB carries three enthusiast guesses: brush_t_def(92), brush_t_OLD(88,
// faces@0x40) and brush_t_with_custom_def(88, faces@0x44). Live code
// uses the LAST one:
//   * Brush_Alloc (0x4751e0), Brush_Clone (0x475d20), sub_473BD0, sub_475E80 all
//     `operator new(0x58)` + `memset(_,0,0x58)` → the brush is 88 (0x58) bytes,
//     ruling out brush_t_def(92).
//   * Brush_Create (0x475300) reads the face list at offset 0x44 (`b[1].ownerNext`
//     → *(char*)b+0x44), matching faces@0x44 here, ruling out brush_t_OLD
//     (faces@0x40). selbrush_t.def is typed brush_t_with_custom_def* as well.
// So brush_t below == IDB brush_t_with_custom_def. (Link-pointer element types are
// the IDB's best guess; only sizes/offsets are load-bearing for Phase 3.)
struct brush_t
{
    brush_t     *oprev;        // 0x00
    brush_t     *onext;        // 0x04
    entity_s    *owner;        // 0x08
    entity_s    *ownerNext;    // 0x0C
    entity_s    *ownerPrev;    // 0x10
    brush_t     *def;          // 0x14
    int          unk1;         // 0x18
    int          refCount;     // 0x1C
    vec3_t       mins;         // 0x20
    vec3_t       maxs;         // 0x2C
    int          xx1;          // 0x38
    int          contents;     // 0x3C
    int          faceCount;    // 0x40
    face_t      *faces;  // 0x44
    char        *parent_layer_string; // 0x48
    union {                    // 0x4C
        __int16      unk01;
        unsigned char modelFailed; // 0x4C low byte (assert strings; == Brush_ModelFailedByte)
    };
    __int16      version;      // 0x4E
    patchMesh_t *patch;        // 0x50
    int          numberId;     // 0x54  (IDB "total_size_0x58"; GtkRadiant id region — unverified)
};
static_assert(sizeof(brush_t) == 88, "brush_t");

static_assert(offsetof(brush_t, owner) == 8,        "brush_t.owner");
static_assert(offsetof(brush_t, faces) == 68, "brush_t.faces");
static_assert(offsetof(brush_t, version) == 78,     "brush_t.version");
static_assert(offsetof(brush_t, patch) == 80,       "brush_t.patch");

// IDA uses several names for the same 88-byte def type; alias them all to brush_t.
typedef brush_t brush_t_with_custom_def;
typedef brush_t brush_t_def;

// selbrush_t (the 56-byte brush-instance / list node) is defined ABOVE, before
// brush_t — entity_s embeds it by value, so it must be a complete type here.
// (The old IDB-verbatim 64-byte selbrush_t was over-padded; see the ruling above.)

// ── map entity ────────────────────────────────────────────────────────────────
struct entity_s
{
    entity_s      *prev;                // 0x00
    entity_s      *next;                // 0x04
    entity_s      *def;                 // 0x08  entity_s_def* (== entity_s; typed so the
                                        //       assert chains ent->def->modelClass… compile)
    entity_brush_s brushes;             // 0x0C  embedded brush-list head (0x38)
    int            modelInst;           // 0x44
    void          *prefab;              // 0x48  prefab_s*
    int            version;             // 0x4C
    char          *mapLayer;            // 0x50
    int            someCount;           // 0x54
    bool           bModelFailed;        // 0x58
    bool           patchBrush;          // 0x59
    bool           hiddenBrush;         // 0x5A
    bool           terrainBrush;        // 0x5B
    void          *pPatch;              // 0x5C
    eclass_t      *eclass;              // 0x60
    entitymodel_t *modelClass;          // 0x64
    vec3_t         origin;              // 0x68
    epair_t       *epairs;              // 0x74
    int            version_prob_wrong;  // 0x78
    int            epairEdits;          // 0x7C  undo-id stamp (undo.cpp)
    int            redoId;              // 0x80  redo-id stamp (undo.cpp phases 2/3)
    int            numberId;            // 0x84  unique entity number (dword_739DC4++)
    int            refCount;            // 0x88
};
static_assert(sizeof(entity_s) == 140, "entity_s");
static_assert(offsetof(entity_s, def) == 8,        "entity_s.def");        // IDB owner->def @+8
static_assert(offsetof(entity_s, brushes) == 12, "entity_s.brushes");
static_assert(offsetof(entity_s, prefab) == 0x48,  "entity_s.prefab");     // IDB owner->prefab @+0x48
static_assert(offsetof(entity_s, eclass) == 96,  "entity_s.eclass");
static_assert(offsetof(entity_s, modelClass) == 0x64, "entity_s.modelClass"); // IDB def->modelClass @+0x64
static_assert(offsetof(entity_s, origin) == 104, "entity_s.origin");
static_assert(offsetof(entity_s, epairs) == 116, "entity_s.epairs");
static_assert(offsetof(entity_s, redoId) == 0x80 && offsetof(entity_s, numberId) == 0x84, "entity_s id stamps");

// ── undo_s — full definition (shared by undo.cpp, select.cpp) ─────────────────
// IDA-verified layout (sizeof = 256 = operator new(0x100)).
// brushlist: brush_t (88-byte) sentinel; linked via oprev/onext.
//   offset 20 + sizeof(brush_t)=88 = 108 = entitylist start. No padding needed.
// entitylist: entity_s (140-byte) sentinel; linked via prev/next.
//   offset 108 + sizeof(entity_s)=140 = 248 = prev.
// IDA field name: "next____totalsize_0x100" = next (at offset 252 of the 256B struct).
struct undo_s
{
    double      time;        // 0x00  wall-clock time of operation
    int         id;          // 0x08  unique undo/redo ID
    int         done;        // 0x0C  non-zero when record is finalised (Undo_End)
    const char *operation;   // 0x10  human-readable operation name (pointer, NOT owned)
    brush_t     brushlist;   // 0x14  brush DEF sentinel (88 bytes); linked via oprev/onext
    entity_s    entitylist;  // 0x6C  entity sentinel (140 bytes); linked via prev/next
    undo_s     *prev;        // 0xF8  = offset 248
    undo_s     *next;        // 0xFC  = offset 252  (IDA: next____totalsize_0x100)
};
static_assert(sizeof(undo_s)              == 256, "undo_s size");
static_assert(offsetof(undo_s, id)        ==   8, "undo_s.id");
static_assert(offsetof(undo_s, done)      ==  12, "undo_s.done");
static_assert(offsetof(undo_s, brushlist) ==  20, "undo_s.brushlist");
static_assert(offsetof(undo_s, entitylist)== 108, "undo_s.entitylist");
static_assert(offsetof(undo_s, prev)      == 248, "undo_s.prev");
static_assert(offsetof(undo_s, next)      == 252, "undo_s.next");

// ── entity class (eclass) ─────────────────────────────────────────────────────
struct eclass_t
{
    eclass_t *next;                 // 0x00
    char     *name;                 // 0x04
    bool      fixedsize;            // 0x08
    char      pad_0x0009[3];        // 0x09
    vec3_t    mins;                 // 0x0C
    vec3_t    maxs;                 // 0x18
    vec3_t    color;                // 0x24
    float     unk;                  // 0x30
    char      material[4];          // 0x34
    void     *textureTableOrSth;    // 0x38
    char     *comments;             // 0x3C
    char      flagname0[32];        // 0x40
    char      flagname1[32];        // 0x60
    char      flagname2[32];        // 0x80
    char      flagname3[32];        // 0xA0
    char      flagname4[32];        // 0xC0
    char      flagname5[32];        // 0xE0
    char      flagname6[32];        // 0x100
    char      flagname7[32];        // 0x120
    char      flagname8[32];        // 0x140
    int       xx1;                  // 0x160
    union {                         // 0x164  five model names; w_cyclePreviewMode indexes them
        struct {
            const char *default_model_name; // 0x164
            const char *xx3;        // 0x168
            const char *xx4;        // 0x16C
            const char *xx5;        // 0x170
            const char *xx6;        // 0x174
        };
        const char *cycleModelName[5];
    };
    int       modelpath;            // 0x178
    int       xx8;                  // 0x17C
    union {                         // 0x180
        int classtype;              //   (port's historical name)
        int nShowFlags;             //   (the binary's name — assert strings)
    };
    int       xx10;                 // 0x184
    int       xx11;                 // 0x188
    int       xx12;                 // 0x18C
    char     *commands;             // 0x190  (IDB "commands_total_0x184_eclass_initfromtext")
};
static_assert(sizeof(eclass_t) == 404, "eclass_t");
static_assert(offsetof(eclass_t, fixedsize) == 8,  "eclass_t.fixedsize");
static_assert(offsetof(eclass_t, classtype) == 384, "eclass_t.classtype");

// ── per-map model entity class (IDB models_t, 0x184 bytes) ───────────────────
// Used by Eclass_01, Eclass_CheckEntities, Model_FreeMapModels, etc.
// Fields are mostly opaque ints; only the ones accessed by eclass.cpp are named.
// Size confirmed: operator new(0x184u) in Eclass_01 (0x482300).
struct models_t
{
    int x0;          // 0x000  next node in g_models / g_eclass linked list
    int handle;      // 0x004  char* model name (heap string)
    int x2;          // 0x008
    struct { int next; } entities; // 0x00C  from eclass field[3] (source name: entities.next)
    int x4;          // 0x010  from eclass field[4]
    int x5;          // 0x014  from eclass field[5]
    int x6;          // 0x018  from eclass field[6]
    int x7;          // 0x01C  from eclass field[7]
    int x8;          // 0x020  from eclass field[8]
    int x9;          // 0x024  float — scale factor 0.85
    int x10;         // 0x028
    int x11;         // 0x02C  float — scale factor 0.85
    int x12;         // 0x030
    int x13;         // 0x034
    int x14;         // 0x038
    int x15;         // 0x03C  char* (freed in Eclass_CheckEntities)
    int x16;         // 0x040
    int x17;         // 0x044
    int x18;         // 0x048
    int x19;         // 0x04C
    int x20;         // 0x050
    int x21;         // 0x054
    int x22;         // 0x058
    int x23;         // 0x05C
    int x24;         // 0x060
    int x25;         // 0x064
    int x26;         // 0x068
    int x27;         // 0x06C
    int x28;         // 0x070
    int x29;         // 0x074
    int x30;         // 0x078
    int x31;         // 0x07C
    int x32;         // 0x080
    int x33;         // 0x084
    int x34;         // 0x088
    int x35;         // 0x08C
    int x36;         // 0x090
    int x37;         // 0x094
    int x38;         // 0x098
    int x39;         // 0x09C
    int x40;         // 0x0A0
    int x41;         // 0x0A4
    int x42;         // 0x0A8
    int x43;         // 0x0AC
    int x44;         // 0x0B0
    int x45;         // 0x0B4
    int x46;         // 0x0B8
    int x47;         // 0x0BC
    int x48;         // 0x0C0
    int x49;         // 0x0C4
    int x50;         // 0x0C8
    int x51;         // 0x0CC
    int x52;         // 0x0D0
    int x53;         // 0x0D4
    int x54;         // 0x0D8
    int x55;         // 0x0DC
    int x56;         // 0x0E0
    int x57;         // 0x0E4
    int x58;         // 0x0E8
    int x59;         // 0x0EC
    int x60;         // 0x0F0
    int x61;         // 0x0F4
    int x62;         // 0x0F8
    int x63;         // 0x0FC
    int x64;         // 0x100
    int x65;         // 0x104
    int x66;         // 0x108
    int x67;         // 0x10C
    int x68;         // 0x110
    int x69;         // 0x114
    int x70;         // 0x118
    int x71;         // 0x11C
    int x72;         // 0x120
    int x73;         // 0x124
    int x74;         // 0x128
    int x75;         // 0x12C
    int x76;         // 0x130
    int x77;         // 0x134
    int x78;         // 0x138
    int x79;         // 0x13C
    int x80;         // 0x140
    int x81;         // 0x144
    int x82;         // 0x148
    int x83;         // 0x14C
    int x84;         // 0x150
    int x85;         // 0x154
    int x86;         // 0x158
    int x87;         // 0x15C
    int x88;         // 0x160  head of entity sub-node linked list
    int x89;         // 0x164  char* material string (freed in Eclass_CheckEntities)
    int x90;         // 0x168  char* material string 1 of 4
    int x91;         // 0x16C  char* material string 2 of 4
    int x92;         // 0x170  char* material string 3 of 4
    int x93;         // 0x174  char* material string 4 of 4
    int x94;         // 0x178  char* name copy (freed in Eclass_CheckEntities)
    int x95;         // 0x17C
    int x96;         // 0x180  model-type flags (bit 4 = 0x10 = prefab)
};
static_assert(sizeof(models_t) == 388, "models_t");
static_assert(offsetof(models_t, x88) == 0x160, "models_t.x88");
static_assert(offsetof(models_t, x96) == 0x180, "models_t.x96");

// ── patch mesh (Bezier/terrain). Bulk is a 16x16 control-point grid. ──────────
struct patchMesh_t
{
    int                width;       // 0x00
    int                height;      // 0x04
    int                contents;    // 0x08
    int                flags;       // 0x0C
    PATCH_TYPES        type;        // 0x10
    int                subDivType;  // 0x14
    patchMesh_material texture;     // 0x18
    patchMesh_material lightmap;    // 0x20
    patchMesh_material smoothing;   // 0x28
    char               pad_0x0030[4]; // 0x30
    texdef_t          *mat_unk;     // 0x34
    drawVert_t         ctrl[16][16];// 0x38  (0x5000 bytes)
    curvePatchDef_t   *curveDef;    // 0x5038  tessellated render mesh (Patch_GenericMesh2)
    union {                         // 0x503C
        entity_brush_s *pSymbiot;   //       (port's historical name/type)
        brush_t        *symbiot;    //       (the binary's name — assert strings; refCount@+0x1C)
    };
    __int16            version;     // 0x5040
    bool               xx22b;       // 0x5042
    bool               bDirty;      // 0x5043
    int                xx21;        // 0x5044
    int                size_of_struct_0x504C; // 0x5048
};
static_assert(sizeof(patchMesh_t) == 20556, "patchMesh_t");
static_assert(offsetof(patchMesh_t, ctrl) == 56,       "patchMesh_t.ctrl");
static_assert(offsetof(patchMesh_t, pSymbiot) == 20540, "patchMesh_t.pSymbiot");

// ── SavedInfo_t (708 bytes = 0x2C4) — persistent editor prefs block inside ────
// qeglobals_t.d_savedinfo.  Ports field-by-field from IDA type SavedInfo_t.
// Verified: sizeof=708, layout below matches radiant binary assertions.
struct SavedInfo_t
{
    int   iSize;            // 0x000  structure size (self-check)
    int   iTextMenu;        // 0x004  texture menu state
    char  szProject[256];   // 0x008  current project path
    vec4_t colors[27];      // 0x108  editor color settings (27 * 16 = 432 bytes)
    int   d_xyShowFlags;    // 0x2B8  XY view visibility flags (bit 0x40 = reverse filter)
    float d_gridsize;       // 0x2BC  grid snap size
    int   d_picmip;         // 0x2C0  texture detail level
};
static_assert(sizeof(SavedInfo_t) == 708, "SavedInfo_t size != 708 (0x2C4)");
static_assert(offsetof(SavedInfo_t, szProject) == 8,        "SavedInfo_t.szProject");
static_assert(offsetof(SavedInfo_t, colors) == 0x108,       "SavedInfo_t.colors");
static_assert(offsetof(SavedInfo_t, d_xyShowFlags) == 0x2B8,"SavedInfo_t.d_xyShowFlags");

// ── filter condition node (IDB filter_info_s, 12 bytes) ───────────────────────
// Used by DynamicFilter_ParseCondition / sub_412170 (filter condition evaluator).
struct filter_contents_s;  // forward-decl (recursive list node)
struct filter_info_s
{
    int               surfaceFlags;  // 0x00  condition type (1=AND, 2=OR, 3=material, 4-8=others)
    char              z_pad_0x0004[4]; // 0x04  negation flag (z_pad_0x0004[0])
    filter_contents_s *contents_ptr; // 0x08  child conditions list (recursive)
};
static_assert(sizeof(filter_info_s) == 12, "filter_info_s");

// ── filter "contents" payload node (IDB filter_contents_s, 12 bytes) ──────────
// For a key/value condition (surface case 5 = script_layer): key@0x00, value@0x04,
// a flags byte@0x08 (bit 0 = "the value is a literal key=value pair" vs an
// entity-epair match).  layers' filter (sub_411950) leaves flags=0 → the case-5
// matcher (sub_412290) takes the Entity_HasEpairMatch branch.  Modelled byte-exact
// from the layer-filter add + the case-5 evaluator decompiles (no AND/OR linkage in
// the script_layer use — `nextContents` stays 0).
struct filter_contents_s
{
    char *key;          // 0x00  condition key ("script_layer")
    char *value;        // 0x04  condition value (the layer name)
    int   flags;        // 0x08  bit0 set → literal compare; clear → epair match
};
static_assert(sizeof(filter_contents_s) == 12, "filter_contents_s");

// ── filter entry (IDB filter_entry_s, 24 bytes) ───────────────────────────────
// Singly-linked list node; four lists hang off qeglobals_t:
//   d_filterGlobals_geometryFilters / entityFilters / triggerFilters / otherFilters.
// filter_type_enum bits: 1=fdhide, 2=fdshow, 4=face (texture map filter).
// isShown: 1 = filter is active / shown in UI.
struct MaterialInfo;  // forward-decl (CoD material info block)
struct filter_entry_s
{
    int             filter_type_enum; // 0x00  type flags (fdhide=1,fdshow=2,face=4)
    bool            isShown;          // 0x04  currently shown/enabled
    char            z_pad_0x0005[3];  // 0x05  alignment padding
    MaterialInfo   *material_ptr;     // 0x08  face-texture filter material list (face type only)
    const char     *name;             // 0x0C  filter entry name (heap-allocated copy)
    filter_info_s  *info;             // 0x10  condition tree root (non-face type)
    filter_entry_s *next_filter;      // 0x14  next entry in same category list
};
static_assert(sizeof(filter_entry_s) == 24, "filter_entry_s");
static_assert(offsetof(filter_entry_s, isShown) == 4,       "filter_entry_s.isShown");
static_assert(offsetof(filter_entry_s, material_ptr) == 8,  "filter_entry_s.material_ptr");
static_assert(offsetof(filter_entry_s, name) == 12,         "filter_entry_s.name");
static_assert(offsetof(filter_entry_s, info) == 16,         "filter_entry_s.info");
static_assert(offsetof(filter_entry_s, next_filter) == 20,  "filter_entry_s.next_filter");

// ── selection info counters (qeglobals_t.d_select_info @ 0x71C08, 20 bytes) ────
struct select_info_t
{
    int numBrushesAndPatches;  // 0x00
    int numEntWithFlag;         // 0x04
    int numPatches;             // 0x08
    int numBrushes;             // 0x0C
    int numFixedSize;           // 0x10
};
static_assert(sizeof(select_info_t) == 20, "select_info_t");

// ── per-edit-layer "current texture window" template (qeglobals.random_texture_stuff).
//    2100 bytes/layer × 3 layers = 6300.  The IDB indexes &random_texture_stuff[2100*layer]
//    as the active MaterialDef stamped onto newly-created faces; [+36] is the default
//    texture sample size/scale.  The trailing bytes (texdef/layer template state) stay
//    opaque padding — only mtl + sampleSize are touched by the port.
struct curTexWndLayer_t
{
    MaterialDef mtl;                  // 0x000  current MaterialDef template
    float       sampleSize;           // 0x024  default texture sample size/scale
    int         hasProjection;        // 0x028  patch ST-projection block valid
    int         gridWidth;            // 0x02C  stored patch grid dims
    int         gridHeight;           // 0x030
    float       st[16][16][2];        // 0x034  stored ST pairs (row stride 128)
};
static_assert(sizeof(curTexWndLayer_t) == 2100, "curTexWndLayer_t");
static_assert(offsetof(curTexWndLayer_t, st) == 0x34, "curTexWndLayer_t.st");

// ── terrainVert_t (IDB terrainVert_t, 8 bytes) — qeglobals.d_terrapoints[i] points
//    here.  Recovered from MoveSelection (0x47f0c0) + Brush_SideSelect (0x4777d0):
//    both subsystems read d_terrapoints[i] as a CONTIGUOUS vec3 — `->height`(x),
//    `->scale`(y), `[1].height`(z) — i.e. a 2-element-stride access that reaches
//    offset 8.  The pointers stored in d_terrapoints[] are actually the side faces'
//    `face_t.planepts[k]` vec3's (Brush_SideSelect collects planepts[0..2] of every
//    face >= 2 of a numberId==2 brush).  So terrainVert_t overlays the first two
//    floats of a planept; [1].height reaches the planept's third float.  The IDB
//    names (height/scale) are the original CoD source's; semantically they are x/y.
struct terrainVert_t
{
    float height;   // 0x00  (planept x)
    float scale;    // 0x04  (planept y)
};
static_assert(sizeof(terrainVert_t) == 8, "terrainVert_t");

// ── MRU (Most-Recently-Used) recent-files menu structure (IDB LPMRUMENU, 16 bytes).
//    g_qeglobals.d_lpMruMenu points here (CreateMruMenuDefault 0x48A150 GlobalAlloc's it).
//    lpMRU is a flat char array of wNbLruMenu × wMaxSizeLruItem bytes.  Faithful to the
//    IDB layout: five WORDs then a pointer at +12 (the +10 gap is C alignment padding —
//    CreateMruMenuDefault stores lpMRU via `*((_DWORD*)v1 + 3)`, i.e. offset 12).
struct LPMRUMENU
{
    unsigned short wNbItemFill;      // 0x00  # of items currently held
    unsigned short wNbLruShow;       // 0x02  # of items to show in the menu (6)
    unsigned short wNbLruMenu;       // 0x04  # of item slots allocated       (9)
    unsigned short wMaxSizeLruItem;  // 0x06  bytes per item slot             (128)
    unsigned short wIdMru;           // 0x08  base command id                 (8000)
    char          *lpMRU;            // 0x0C  flat item store (wNbLruMenu × wMaxSizeLruItem)
};
static_assert(sizeof(LPMRUMENU) == 16, "LPMRUMENU");

// ── editor global state (g_qeglobals @ IDB 0x25f39c0). Ported field-for-field;
//    a handful of sub-structs (pedge_t/SavedInfo_t/select_info_t/filter_entry_s)
//    are byte-padded with TODO_RADIANT markers — extend them at the marked offsets
//    in later phases (plan §5). ──────────────────────────────────────────────────
struct qeglobals_t
{
    bool      d_showgrid;              // 0x00
    int       d_gridsize;              // 0x04
    int       d_num_entities;          // 0x08
    entity_s *d_project_entity;        // 0x0C
    float     d_new_brush_bottom_x;    // 0x10
    float     d_new_brush_bottom_y;    // 0x14
    float     d_new_brush_bottom_z;    // 0x18
    float     d_new_brush_top_x;       // 0x1C
    float     d_new_brush_top_y;       // 0x20
    float     d_new_brush_top_z;       // 0x24
    HINSTANCE d_hInstance;             // 0x28
    HWND      d_hwndMain;              // 0x2C
    HWND      d_hwndCamera;            // 0x30
    HWND      d_hwndEdit;              // 0x34
    HWND      d_hwndEntity;            // 0x38
    HWND      d_hwndTexture;           // 0x3C
    HWND      d_hwndXY;                // 0x40
    HWND      d_hwndZ;                 // 0x44
    HWND      d_hwndStatus;            // 0x48
    HWND      d_hwndGroup;             // 0x4C
    HWND      d_hwndMedia;             // 0x50
    vec3_t    d_points[2048];          // 0x54
    int       d_numpoints;             // 0x6054
    pedge_t   d_edges[512];            // 0x6058  (16 bytes each = 8192)
    int       d_numedges;              // 0x8058
    int       d_num_move_points;       // 0x805C
    drawVert_t *d_move_points[4096];   // 0x8060
    int       d_numterrapoints;        // 0xC060
    terrainVert_t *d_terrapoints[4096];// 0xC064  recovered: terrainVert_t* array (side-face planept vec3's)
    vec3_t    d_select_translate_unk;  // 0x10064
    float     unkown_pmesh_float1;     // 0x10070
    int       pad_01;                  // 0x10074
    char      patch_verts_array01[196600]; // 0x10078
    int       patch_verts_array01_count;   // 0x40070
    float     unkown_pmesh_float2;     // 0x40074
    float     unkown_pmesh_float3;     // 0x40078
    char      patch_verts_array02[196600]; // 0x4007C
    int       patch_verts_array02_count;   // 0x70074
    int       pad_02;                  // 0x70078
    int       current_edit_layer;      // 0x7007C
    void     *d_activeLayer;           // 0x70080
    curTexWndLayer_t random_texture_stuff[3]; // 0x70084  per-layer current-texture template (3×2100)
    LPMRUMENU *d_lpMruMenu;            // 0x71920  MRU recent-files menu (CreateMruMenuDefault)
    SavedInfo_t d_savedinfo;           // 0x71924  persistent editor prefs (SavedInfo_t, 708 bytes)
    int       d_workcount;             // 0x71BE8
    int       d_select_count;          // 0x71BEC
    entity_brush_s *d_select_order[2]; // 0x71BF0
    vec3_t    d_select_translate;      // 0x71BF8
    select_t  d_select_mode;           // 0x71C04
    select_info_t d_select_info;       // 0x71C08
    int       surfInsp_nIDButton;      // 0x71C1C
    int       surfInsp_tex_repeatx;    // 0x71C20
    int       surfInsp_tex_repeaty;    // 0x71C24
    void     *d_font_list;             // 0x71C28  Font_s*
    Material *d_white;                 // 0x71C2C
    Material *d_opague;                // 0x71C30
    Material *d_additive;              // 0x71C34
    int       d_parsed_brushes;        // 0x71C38
    int       pad_d_parsed_brushes;    // 0x71C3C
    int       drag_selectionbox_x_1;   // 0x71C40
    int       drag_selectionbox_y_1;   // 0x71C44
    int       drag_selectionbox_x_2;   // 0x71C48
    int       drag_selectionbox_y_2;   // 0x71C4C
    void     *camera_fov_setup;        // 0x71C50
    bool      use_ini;                 // 0x71C54
    char      pad_use_ini[3];          // 0x71C55
    char      use_ini_registry[64];    // 0x71C58
    bool      dontDrawSelectedOutlines;// 0x71C98
    char      pad_ddso[3];             // 0x71C99
    bool      dontDrawSelectedTint;    // 0x71C9C
    char      pad_ddst[3];             // 0x71C9D
    bool      preview_at_max_intensity;// 0x71CA0
    char      pad_pami[3];             // 0x71CA1
    bool      toggle_unk02;            // 0x71CA4
    bool      toggle_unk03_mousedrag_state1; // 0x71CA5
    bool      toggle_unk04_mousedrag_state2; // 0x71CA6
    bool      bLockPatchVerts;         // 0x71CA7
    bool      bUnlockPatchVerts;       // 0x71CA8
    bool      toggle_unk05;            // 0x71CA9
    char      pad3[6];                 // 0x71CAA
    double    g_oldtime;               // 0x71CB0
    double    g_time;                  // 0x71CB8
    int       g_filtersUpdated;        // 0x71CC0
    int       g_layerCount_maybe;      // 0x71CC4
    __int16   w_cyclePreviewMode;      // 0x71CC8
    char      pad_cyclePreviewMode[2]; // 0x71CCA
    filter_entry_s *d_filterGlobals_geometryFilters; // 0x71CCC
    filter_entry_s *d_filterGlobals_entityFilters;   // 0x71CD0
    filter_entry_s *d_filterGlobals_triggerFilters;  // 0x71CD4
    filter_entry_s *d_filterGlobals_otherFilters;    // 0x71CD8
    filter_entry_s *d_filterGlobals_layerFilters;    // 0x71CDC
};
static_assert(sizeof(qeglobals_t) == 466144, "qeglobals_t");
static_assert(offsetof(qeglobals_t, d_points) == 84,          "qeglobals_t.d_points");
static_assert(offsetof(qeglobals_t, d_select_count) == 465900,"qeglobals_t.d_select_count");
static_assert(offsetof(qeglobals_t, d_select_mode) == 465924, "qeglobals_t.d_select_mode");
static_assert(offsetof(qeglobals_t, d_white) == 465964,       "qeglobals_t.d_white");
static_assert(offsetof(qeglobals_t, g_oldtime) == 466096,     "qeglobals_t.g_oldtime");

// entity_s is also used as an entity definition (the 140-byte "def" record with
// full epairs, eclass, origin, refCount). Both the map-level def and the truncated
// runtime instance use the SAME struct; the name alias makes the intent clear.
typedef entity_s entity_s_def;

// entitymodel_t — opaque handle to a loaded model inside an entity def.
// entity_s.modelClass points to one of these (it IS a models_t — an eclass-like node).
// Its ->model sub-node (the loaded-model record whose +4 handle is the XModel*) is at
// offset 0x160 (the IDB Entity_UpdateModelInst reads [modelClass+0x160]->[+4]; this is the
// same field eclass.cpp calls `xx1` (+0x160) — the head of the sub-model node list).
struct entitymodel_t
{
    char      _pad0[0x160];   // the models_t prefix (name/handle/eclass fields)
    models_t *model;          // 0x160  the loaded model sub-node (model->handle@+4 = XModel*)
    int       rest[1];        // placeholder — do not index
};

// ── prefab-edit stack (IDB byte_25EB240 @ 0x25EB240) ──────────────────────────
// One saved parent-map state per nested prefab level (Prefab_NextLevel writes,
// Prefab_PrevLevel restores). 16 levels: the binary's storage runs to exactly
// g_qeglobals (0x25EB240 + 16*2168 == 0x25F39C0) and the full-check is == 16.
struct prefabLevel_t
{
    int         modified;             // +0x000
    char        mapName[1024];        // +0x004  saved currentmap
    selbrush_t *activeNext;           // +0x404  active_brushes.next  (dword 257)
    selbrush_t *activePrev;           // +0x408  active_brushes.prev
    selbrush_t *selectedNext;         // +0x40C  selected_brushes.next (0 = none saved)
    selbrush_t *selectedPrev;         // +0x410  selected_brushes.prev
    entity_s   *entitiesNext;         // +0x414  entities.next
    entity_s   *entitiesPrev;         // +0x418  entities.prev
    entity_s   *entityInstsPrev;      // +0x41C  entityInsts.prev
    entity_s   *entityInstsNext;      // +0x420  entityInsts.next
    selbrush_t *prefabBrush;          // +0x424  the brush being prefab-edited
    char        activeLayer[1024];    // +0x428  saved g_activeLayer_string
    char        layerSnapshot[12];    // +0x828  binary's layer RB-tree copy head (the
                                      //         port snapshots via Layers_SavePrefabLayers)
    float       camOrigin[3];         // +0x834  (dword 525)
    float       camAngles[3];         // +0x840  (528)
    float       xyOrigin[2];          // +0x84C  (531)
    float       xyScale;              // +0x854  (533)
    int         xyViewType;           // +0x858  (534)
    int         regionActive;         // +0x85C  (535)
    float       regionMins[3];        // +0x860  (536)
    float       regionMaxs[3];        // +0x86C  (539)
};
static_assert(sizeof(prefabLevel_t) == 2168, "prefabLevel_t must match the binary's 2168-byte slot");
static_assert(offsetof(prefabLevel_t, activeNext) == 257 * 4 && offsetof(prefabLevel_t, camOrigin) == 525 * 4,
              "prefabLevel_t dword indices");
extern prefabLevel_t g_prefabStack[16];  // engine_stubs.cpp (IDB byte_25EB240)

// ── known globals (plan Appendix B). extern here; defined in their home .cpp in
//    Phase 4 (g_qeglobals in qe3.cpp; the brush lists in map.cpp/select.cpp). ──
extern qeglobals_t g_qeglobals;          // 0x25f39c0
// Embedded 56-byte selbrush_t sentinel nodes (NOT 88-byte brush_t). Iterate as
// `for (b = active_brushes.next; b != &active_brushes; b = b->next)`. See ruling.
extern selbrush_t  active_brushes;       // 0x23f189c (display list sentinel)
extern selbrush_t  selected_brushes;     // 0x23f1864
extern selbrush_t  filtered_brushes;     // 0x23f182c

// ── LAYERS dialog backend (layers.cpp / filters.cpp) ─────────────────────────
// The dialog (layersdlg.cpp) drives these; they share g_layerMap (layers.cpp)
// + g_activeLayer_string + the per-brush parent_layer_string the .map writer uses.
#include <vector>
#include <string>
#include <utility>
void        Layers_Enumerate( std::vector<std::pair<std::string,int>> &out );  // sorted name+flags
int         Layers_Count();
bool        Layers_Exists( const char *name );
int         Layers_GetFlags( const char *name );          // flags (hidden=1,prefab=2,expanded=4,frozen=8)
const char *Layers_GetActive();
void        Layers_SetActive( const char *name );          // CLayerDlg::SetActiveLayer core
void        Layers_SetFlag( const char *name, int flag );  // layers_01 0x4190F0 (OR)
void        Layers_ClearFlag( const char *name, int flag );// layers_02 0x419200 (AND ~)
void        Layers_AddLayerPath( const char *name, int flags ); // sub_419630 (add/OR a layer)
int         Layers_DeleteLayer( const char *victim );      // Layers_Update_01 0x419940
int         Layers_RenameLayer( const char *oldFull, const char *newFull ); // Layers_Update_02 0x419BE0
void        Layers_RebuildVisibilityFilters();             // sub_41C200 backend (script_layer set)
// visibility (filters.cpp): toggle a layer-filter entry's isShown → FilterBrush.
struct filter_entry_s;
const char     *Layers_BuildFilter( const char *layerName );           // 0x411950
void            Layers_FreeVisibilityFilters();
filter_entry_s *Layers_FindVisibilityFilter( const char *layerName );
bool            Layers_SetLayerVisible( const char *layerName, bool visible ); // RadiantFilters07 tail
// filter condition-tree parser (filters.cpp): build/destroy a condition from text.
struct filter_info_s;
filter_info_s  *DynamicFilter_ParseCondition( const char **text );     // 0x411760
struct FaceTexNode;
FaceTexNode    *qe3_cpp_01( const char **text );                       // 0x411280 (face mtl list)

// ── CFilterWnd — the Filters inspector pane (filters.cpp loader + UI API) ─────
// RadiantFilters.txt → the four category filter lists.  FACE entries (filter_type_enum&4)
// ARE loaded + wired to the faceTexMap (the "filter faces by material" hide list, consumed
// by MtlDef_IsFaceFiltered); only non-face condition trees touching the materialdef-coupled
// cases 3/6/7 are dropped at load — see filters.cpp.  The Filters inspector tab (win_ent.cpp)
// drives these:
void            Load_RadiantFilters();                                 // 0x411190 (CMainFrame::OnCreate)
void            RadiantFilters( const char *filterFile );              // 0x410c50 (one txt file)
void            RadiantFilters_ToggleEntry( filter_entry_s *cb, bool show ); // 0x4149c0 core (checklist click)
filter_entry_s *CFilterWnd_GetCategoryHead( int category );            // 0=geo 1=trig 2=ent 3=other
// faceTexMap (filters.cpp std::map<std::string,int>): per-material-name refcount hide list.
// 05_large = ++ each material_ptr name (HIDE); 06_large = -- / erase (SHOW); the substring
// reader is the consumer hook for MtlDef_IsFaceFiltered.
struct MaterialInfo;
MaterialInfo   *RadiantFilters05_large( filter_entry_s *a1 );          // 0x411be0 (add refcounts)
MaterialInfo   *RadiantFilters06_large( filter_entry_s *a1 );          // 0x411e30 (remove refcounts)
bool            FaceTexMap_HasSubstringOf( const char *materialName );  // any key ⊂ materialName?
int             CFilterWnd_GetSavedCheck( const char *name );          // registry "Filters\<name>" default
void            CFilterWnd_SaveCheck( const char *name, bool shown );  // persist a toggle
// the six simple show-flag checkboxes (CFilterWnd::GetSettings 0x4140f0 — d_xyShowFlags bits):
int             CFilterWnd_ShowFlagCount();
const char     *CFilterWnd_ShowFlagLabel( int i );
bool            CFilterWnd_GetShowFlag( int i );
void            CFilterWnd_SetShowFlag( int i, bool checked );

// ── CLIPPER (split a brush along a clip plane) — CXYWnd cluster ───────────────
// A single 28-byte clip point: its world position, an optional terrapoint vec3*,
// the screen position it was dropped at, and a "placed" flag. IDB CClipPoint(28).
struct CClipPoint
{
    vec3_t  m_ptClip;     // 0x00  world position of the dropped point
    vec3_t *m_pVec3;      // 0x0C  terrapoint binding (unused for brush clipping)
    POINT   m_ptScreen;   // 0x10  screen coords it was dropped at
    bool    m_bSet;       // 0x18  this point has been placed
};
static_assert(sizeof(CClipPoint) == 28,            "CClipPoint != 28");
static_assert(offsetof(CClipPoint, m_pVec3) == 12, "CClipPoint.m_pVec3");
static_assert(offsetof(CClipPoint, m_ptScreen) == 16, "CClipPoint.m_ptScreen");
static_assert(offsetof(CClipPoint, m_bSet) == 24,  "CClipPoint.m_bSet");

// The three placed clip points + the clip-mode flags + the front/back split-brush
// display lists (selbrush_t sentinels, like selected_brushes). DrawClipper/DropClipPoint
// read g_Clip3 as a raw float[] in the IDB, but it is the same CClipPoint layout.
extern CClipPoint g_Clip1;               // 0x25d5b98
extern CClipPoint g_Clip2;               // 0x25d5bb4
extern CClipPoint g_Clip3;               // 0x25d5bd0
extern int        g_bClipMode;           // 0x23f16d8 (bool: clip mode active)
extern char       g_bRogueClipMode;      // 0x23245a7 (Ctrl+RMB temporary clip)
extern char       g_bSwitch;             // 0x23245a6 (which side the clip keeps)
extern float     *g_pMovingClip;         // 0x23f16d4 (the clip point being dragged)
extern selbrush_t g_brFrontSplits;       // 0x23f169c (front-split display list head)
extern selbrush_t g_brBackSplits;        // 0x23f1664 (back-split display list head)

// ── CoD asset-type size verification (kisak headers vs CoD4Radiant IDB) ───────
// INVESTIGATED (plan §5.2): the IDB asset sizes do NOT all match kisak. kisak is
// the CoD3 engine; CoD4Radiant is CoD4 — the same engine-version divergence the
// P2 review (A4) found in the material *system* (CoD3 2048/0x7FF materials vs CoD4
// 4096/0xFFF). Asset-type sizes:
//      type                  kisak (CoD3)    CoD4Radiant IDB
//      Material              0x50  (80)      0x70  (112)
//      MaterialTechniqueSet  0x94  (148)     0x90  (144)
//      GfxImage              0x24  (36)      0x24  (36)   <- matches
// The radiant target compiles kisak's CoD3 r_material.cpp/r_image.cpp and the
// editor structs reference these types ONLY by pointer (qtexture_s.next,
// qeglobals_t.d_white/d_opague/d_additive), so the size divergence is non-breaking
// — the editor is internally consistent on the kisak (CoD3) layout. The asserts
// below pin the KISAK sizes the radiant build actually uses (NOT the IDB sizes).
static_assert(sizeof(GfxImage) == 36,               "GfxImage size drifted from 36");
// KISAK_RADIANT widens Material with trailing editor-only fields (surfaceFlags@80 so
// Material_CastsStencilShadow can read IW3 surfaceFlags, + editorUsage@84/editorLocale@88
// so the texture-browser usage/locale filter has data) without disturbing SP/MP; sizeof
// rounds to 96 (GfxDrawSurf forces 8-byte align → tail pad). SP/MP keep 80.
// See gfx_d3d/r_material.h.
static_assert(sizeof(Material) == 96,               "kisak (radiant) Material size drifted from 0x60");
static_assert(sizeof(MaterialTechniqueSet) == 148,  "kisak MaterialTechniqueSet size drifted from 0x94");
