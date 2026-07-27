#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\shadowvolume.cpp
// The CPU stencil shadow-volume builder + flush for the sun/light preview.
// 0x4561b0 SunLightPreview_PolyOffsetShadows (the FLUSH), 0x456350 vertex hash,
// 0x456470 edge hash, 0x4565b0 AddSilhouetteTri, 0x4567b0 AddFaceFan,
// 0x456850 AddModelSilhouette, 0x456930 ShadVol_Init, 0x47b190 AddBrushFaces,
// 0x47b2a0 DrawBrushShadow, 0x47b310 BrushShadow, 0x479bf0 model arm,
// 0x4418b0 patch arm.  Asserts: shadowvolume.cpp:133, :164, :205, :209.

#include "stdafx.h"
#include <gfx_d3d/r_rendercmds.h>   // MaterialTechniqueType, TECHNIQUE_UNLIT
#include <gfx_d3d/r_material.h>     // Material*
#include <cstdlib>                   // abs()

// Assert/Sys_Printf — defined in engine_stubs.cpp (same pattern as all other radiant TUs).
extern void Assert( const char *file, int line, int type, const char *fmt, ... );
extern int  Sys_Printf( const char *fmt, ... );

// ─────────────────────────────────────────────────────────────────────────────
// R_AddRenderCmdDrawTris (0x4FD1C0) — not declared in kisak headers for KISAK_RADIANT.
// ─────────────────────────────────────────────────────────────────────────────
extern void R_AddRenderCmdDrawTris(
    Material              *material,
    MaterialTechniqueType  techType,
    short                  indexCount,
    const uint16_t        *indices,
    short                  vertexCount,
    const float          (*xyzw)[4],
    const float          (*normal)[3],
    float                 *color,
    const float          (*st)[2] );

// ─────────────────────────────────────────────────────────────────────────────
// Global arrays (BSS equivalents of the binary's fixed-address globals).
//
// VERTEX HASH (0x23315A8, 0x40000 B = 0x8000 slots x 8 B):
//   { DWORD xyzwPtr @+0, INT16 vertIndex @+4, INT16 pad @+6 }.  The IDA "2*v2" /
//   "4*v2" indices are the DWORD / WORD views of that 8-byte stride; kept here as
//   two parallel arrays.
// EDGE HASH (0x23715AC, 0x80000 B = 0x10000 slots x 8 B):
//   { INT16 lo @+0, INT16 hi @+2, INT32 parity @+4 }.  IDA magic 37164460 == 0x23715AC.
// SHADOW INDEX BUFFER (0x232E5A8): the six "arrays" word_232E5A8..B2 in the decompile
//   are byte offsets +0..+10 into ONE flat short[], i.e. [indexCount + 0..5].
// VERTEX DATA (0x23245A8 xyzw / 0x23285A8 normal / 0x232B5A8 color / 0x232C5A8 st):
//   every hashed vertex owns a PAIR of slots — real (w=1) then extruded (w=0).
// ─────────────────────────────────────────────────────────────────────────────

static const int SHADVOL_VERT_HASH_SIZE = 0x8000;
static const int SHADVOL_EDGE_HASH_SIZE = 0x10000;
static const int SHADVOL_MAX_VERTS      = SHADVOL_VERT_HASH_SIZE * 2; // real+extruded pairs

// Vertex hash table.
static DWORD  s_vertHashPtr  [SHADVOL_VERT_HASH_SIZE];  // IDB 0x23315A8 ptr column
static short  s_vertHashIdx  [SHADVOL_VERT_HASH_SIZE];  // IDB 0x23315AC index column

// Edge hash table.
struct ShadVolEdgeSlot { short lo, hi; int parity; };
static ShadVolEdgeSlot s_edgeHash[SHADVOL_EDGE_HASH_SIZE]; // IDB 0x23715AC

// Shadow index buffer: one flat array; see layout note above.
static short  s_indexBuf [SHADVOL_MAX_VERTS * 6];  // IDB 0x232E5A8

// Vertex data arrays.
static float  s_xyzw  [SHADVOL_MAX_VERTS * 4];  // IDB 0x23245A8  vec4 (w=1 real, w=0 extruded)
static float  s_normal[SHADVOL_MAX_VERTS * 3];  // IDB 0x23285A8  vec3 normal
static float  s_color [SHADVOL_MAX_VERTS * 4];  // IDB 0x232B5A8  vec4 color
static float  s_st    [SHADVOL_MAX_VERTS * 2];  // IDB 0x232C5A8  vec2 texcoord

// Counters / state.  The four 4-byte-slot counters are int (0x23715A8, 0x23F15AC,
// 0x23F15B4, 0x23F15B8 each own a full dword); only indexCount/vertexCount are 16-bit
// (0x23F15C8 / 0x23F15CA are two bytes apart).
static int    s_vertHashCount = 0;      // IDB 0x23715A8  shadVol.vertHashCount
static int    s_edgeHashCount = 0;      // IDB 0x23F15AC  shadVol.edgeHashCount
static int    s_edgeIndexCount = 0;     // IDB 0x23F15B0
static int    s_quadsPerEdge   = 6;     // IDB 0x23F15B4  shadVol_quadsPerEdge
static int    s_frontCapIndices = 6;    // IDB 0x23F15B8  shadVol_frontCapIndices
static int    s_silhouetteSign  = 0;    // IDB 0x23F15BC  silhouette facing reference
       Material *mat_stencilshadow = nullptr; // IDB 0x23F15C0 (== rgp.stencilShadowMaterial)
       // mat_white_multiply (0x23F15C4), registered by name in ShadVol_Init.  NOT an rgp
       // builtin: the #26 sun-preview BLACK-WORLD quad multiplies the framebuffer by the
       // worldspawn ambient through it.  NULL if the modtools assets lack it — the caller
       // (camwnd Cam_SunPrev) then skips the multiply quad.  Exposed for camwnd.cpp.
       Material *mat_white_multiply = nullptr; // IDB 0x23F15C4
static short  s_indexCount     = 0;    // IDB 0x23F15C8  indexCount
static short  s_vertexCount    = 0;    // IDB 0x23F15CA  vertexCount

// mat_stencilshadow (0x23F15C0) == rgp.stencilShadowMaterial; read lazily on first build.
#include <gfx_d3d/r_init.h>             // rgp

// ─────────────────────────────────────────────────────────────────────────────
// 0x456350 — vertex hash lookup/insert.  sunlight = float[4] {dir.xyz, w},
// vtx = float[3].  Returns the hash-table index of the vertex; on insert it writes
// BOTH slots of the pair (real xyz/w=1, then extruded vtx*w - light.xyz / w=0).
// ─────────────────────────────────────────────────────────────────────────────
static short shadowvolume_01( const float *sunlight, const float *vtx )
{
    // Hash over the LOW half of each component's bit pattern (0x45636b).
    uint16_t iw0 = (uint16_t)(*(const uint32_t *)(vtx + 0));
    uint16_t iw1 = (uint16_t)(*(const uint32_t *)(vtx + 1));
    uint16_t iw2 = (uint16_t)(*(const uint32_t *)(vtx + 2));
    uint16_t v2  = (uint16_t)( iw2 + 12517u * iw1 - 12519u * iw0 ) & 0x7FFF;

    if ( s_vertHashIdx[v2] == -1 )
    {
LABEL_6:
        if ( (unsigned)s_vertHashCount >= (unsigned)SHADVOL_VERT_HASH_SIZE )
        {
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\shadowvolume.cpp",
                    164, 0, "%s", "shadVol.vertHashCount < SHADVOL_VERT_HASH_SIZE" );
        }
        float *v4 = &s_xyzw[4 * (int)s_vertexCount];
        s_vertexCount += 2;

        v4[0] = vtx[0];  v4[1] = vtx[1];  v4[2] = vtx[2];  v4[3] = 1.0f;
        v4[4] = v4[0] * sunlight[3] - sunlight[0];
        v4[5] = v4[1] * sunlight[3] - sunlight[1];
        v4[6] = v4[2] * sunlight[3] - sunlight[2];
        v4[7] = 0.0f;

        s_vertHashPtr[v2] = (DWORD)(uintptr_t)v4;
        s_vertHashIdx[v2] = (short)s_vertHashCount++;
        return s_vertHashIdx[v2];
    }
    else
    {
        for ( ;; )
        {
            const float *cached = (const float *)(uintptr_t)s_vertHashPtr[v2];
            if (   *(const uint32_t *)(vtx + 0) == *(const uint32_t *)(cached + 0)
                && *(const uint32_t *)(vtx + 1) == *(const uint32_t *)(cached + 1)
                && *(const uint32_t *)(vtx + 2) == *(const uint32_t *)(cached + 2) )
            {
                return s_vertHashIdx[v2];
            }
            v2 = (v2 + 1u) & 0x7FFF;
            if ( s_vertHashIdx[v2] == -1 )
                goto LABEL_6;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x456470 — edge hash insert/accumulate.  The edge is stored with lo<hi and the
// parity sign flipped when the caller's order was reversed, so an interior edge shared
// by two tris accumulates to zero parity and drops out of the silhouette.
// Returns the edgeIndexCount delta (+/- shadVol_quadsPerEdge).
// ─────────────────────────────────────────────────────────────────────────────
static int shadowvolume_02_edge( uint16_t a1, uint16_t a2, int a3 )
{
    uint16_t v3 = a1, v4 = a2;
    if ( a2 > a1 )
    {
        v4 = a1;  v3 = a2;  a3 = -a3;
    }

    // Hash (IDA 0x45649c): (v3 - 25033*v4) truncated to uint16.
    uint16_t v5 = (uint16_t)(v3 - 25033u * v4);

    if ( s_edgeHash[v5].lo == -1 )
    {
LABEL_7:
        if ( (unsigned)s_edgeHashCount >= SHADVOL_EDGE_HASH_SIZE - 1u )
        {
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\shadowvolume.cpp",
                    205, 0, "%s", "shadVol.edgeHashCount < SHADVOL_EDGE_HASH_SIZE - 1" );
        }
        ++s_edgeHashCount;
        s_edgeIndexCount += s_quadsPerEdge;

        if ( s_edgeHash[v5].parity != 0 )
        {
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\shadowvolume.cpp",
                    209, 1, "%s\n\t(edge->parity) = %i",
                    "(edge->parity == 0)", s_edgeHash[v5].parity );
        }
        s_edgeHash[v5].lo     = (short)v4;
        s_edgeHash[v5].hi     = (short)v3;
        s_edgeHash[v5].parity = a3;
        return a3;
    }
    else
    {
        while ( s_edgeHash[v5].lo != (short)v4 || s_edgeHash[v5].hi != (short)v3 )
        {
            v5 = (uint16_t)(v5 + 1u);
            if ( s_edgeHash[v5].lo == -1 )
                goto LABEL_7;
        }
        int result = s_quadsPerEdge;
        if ( a3 * s_edgeHash[v5].parity < 0 )
            result = -s_quadsPerEdge;
        s_edgeIndexCount       += result;
        s_edgeHash[v5].parity  += a3;
        return result;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x4561b0 SunLightPreview_PolyOffsetShadows — a misnomer: this is the shadow-volume
// FLUSH.  Walk the edge hash; every slot with non-zero parity emits |parity| side quads
// (6 indices each: v4,v3,v3+1 / v3+1,v4+1,v4 — two tris of the extruded silhouette
// quad), then the vertex/edge hashes are reset and the batch is issued through
// rgp.stencilShadowMaterial.  Assert shadowvolume.cpp:133 "edgeIndexCount == 0".
// Also called MID-BUILD by ShadVol_AddSilhouetteTri on overflow, so one frame issues
// many draws.  Loop bound: the binary's `v0 < &shadVol_edgeIndexCount` is exactly
// 0x10000 slots (0x23F15B0 - 0x23715B0 = 0x80000 bytes / 8).
// IDA keeps v7 as a running copy of indexCount but it always equals indexCount at the
// one point it is used, so indexCount is used directly here.
// ─────────────────────────────────────────────────────────────────────────────
void SunLightPreview_PolyOffsetShadows()
{
    ShadVolEdgeSlot *slotBase = s_edgeHash;
    ShadVolEdgeSlot *v0 = slotBase;
    const ShadVolEdgeSlot *end = s_edgeHash + SHADVOL_EDGE_HASH_SIZE;

    do
    {
        if ( v0->parity )
        {
            short v1, v2;
            if ( v0->parity >= 0 )
            {
                v1 = v0->hi;   // ((short*)v0)[-1] in IDA (hi is at +2 in slot, lo at +0)
                v2 = v0->lo;   // ((short*)v0)[-2]
            }
            else
            {
                v1 = v0->lo;
                v2 = v0->hi;
            }
            short v3 = (short)(2 * v2);
            short v4 = (short)(2 * v1);
            unsigned int v5 = (unsigned int)abs( v0->parity );
            short v6 = (short)(v3 + 1);

            do
            {
                // 6 indices = one silhouette quad (two tris).
                short *out = s_indexBuf + s_indexCount;
                out[0] = v4;
                out[1] = v3;
                out[2] = v6;
                out[3] = v6;
                out[4] = (short)(v4 + 1);
                out[5] = v4;

                s_edgeIndexCount -= s_quadsPerEdge;
                --v5;
                s_indexCount = (short)(s_indexCount + s_quadsPerEdge);
            } while ( v5 );

            v0 = slotBase;
            slotBase->parity = 0;
        }
        v0->lo = -1;
        ++v0;
        slotBase = v0;
    }
    while ( v0 < end );

    if ( s_edgeIndexCount )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\shadowvolume.cpp",
                133, 0, "%s", "shadVol.edgeIndexCount == 0" );
    }

    s_vertHashCount = 0;
    s_edgeHashCount = 0;
    // Mark every vertex-hash slot empty.  The binary memsets the whole 0x40000-byte
    // table (ptr column included); only the index column is ever tested for -1.
    memset( s_vertHashIdx, 0xFF, sizeof(s_vertHashIdx) );

    if ( s_vertexCount && s_indexCount )
    {
        if ( !mat_stencilshadow )
            mat_stencilshadow = rgp.stencilShadowMaterial;   // engine builtin "stencilshadow"
        R_AddRenderCmdDrawTris(
            mat_stencilshadow,
            TECHNIQUE_UNLIT,
            s_indexCount,
            (const uint16_t *)s_indexBuf,
            s_vertexCount,
            (const float (*)[4])s_xyzw,
            (const float (*)[3])s_normal,
            s_color,
            (const float (*)[2])s_st );
    }

    s_vertexCount = 0;
    s_indexCount  = 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// THE SILHOUETTE-EMIT CHAIN — pure CPU geometry.  These build the stencil volume from
// the brushes between the light and the world; the flush above draws it through
// rgp.stencilShadowMaterial (a stencil-WRITING material) so the lit pass can respect
// the stencil.  The LIT pass itself is NOT here: it needs g_useTechnique[0x1A]
// (SUNLIGHT_PREVIEW), the sunpre techset and three un-ported render commands.
// ═════════════════════════════════════════════════════════════════════════════

#include "qe3.h"                         // face_t, brush_t, winding_t, MaterialDef
extern selbrush_t active_brushes;        // map.cpp (0x23F189C) — world brush DISPLAY list head
extern selbrush_t selected_brushes;      // map.cpp (0x23F1864) — selected brush DISPLAY list head
extern float world_orient_matrix[4][3];  // entity.cpp (0x6DE290) — world-space identity orientation
extern void  Vec3Cross( const vec3_t v0, const vec3_t v1, vec3_t cross );          // 0x40a4d0
extern void  OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient ); // 0x4ba430
extern char  FilterBrush( selbrush_t *b, int a2 );                                  // 0x46a1f0
extern qtexture_s *MaterialDef_GetLayeredMaterial( MaterialDef *def );              // 0x4314a0
extern void  PlaneToOrientationVec4( float *out, const float *plane,
                                      const orientation_t *orient );                // draw.cpp 0x4ba720
extern char  MtlDef_IsFaceFiltered( MaterialDef *mtlDef );                          // mayaexport.cpp 0x46fbe0
extern bool  MaterialDef_10_LayeredMatHandle( MaterialDef *mtlDef );                // materialdef.cpp 0x431a60 (same inline, with the assert)
extern void  Patch_Fill_SyncVersion( patch_t *inst, const orientation_t *orient );  // pmesh.cpp 0x4405a0

// prefab_s — local mirror of entity.cpp's prefab_s (0x54 bytes); only the instanced-
// brush list sentinels are read here (the prefab-content recursion at 0x47b359).
struct prefab_s
{
    entity_s    *prev_entity;           // 0x00
    entity_s    *next_entity;           // 0x04
    void        *unk;                   // 0x08
    selbrush_t  *active_brushlist;      // 0x0C   tail sentinel (prev side)
    selbrush_t  *active_brushlist_next; // 0x10   head sentinel (next side)
    char         _pad[0x54 - 0x14];     // 0x14 .. 0x53
};
static_assert(sizeof(prefab_s) == 0x54, "prefab_s (shadowvolume.cpp mirror != entity.cpp)");

// ── model-shadow (sub_479BF0) dependencies — all ported in their home files ──
extern void  SetupModelInst( float *ident_mtx, entity_s *e );                       // entity.cpp 0x485890
extern void  Entity_GetOrientation( entity_s_def *ent, orientation_t *orParent,
                                     orientation_t *orOut );                         // entity.cpp 0x482a70
extern char  Entity_HasRenderableModel( brush_t_with_custom_def *b, int orient );   // brush.cpp 0x479610
extern void  sub_477D70( selbrush_t *b, const float *orient );                     // brush.cpp 0x477d70 Brush_CheckBuildFaceVis
// Editor_ExtractXModelGeo (sub_4FEBB0, r_xsurface.cpp) — XModel CPU geometry → vert/idx
// buffers; returns the INDEX count.  XModel/entitymodel_t come from qe3.h's engine headers.
struct XModel;
extern int   Editor_ExtractXModelGeo( XModel *model, float *verts, int vertLimit,
                                       uint16_t *indices, int indexLimit );          // gfx_d3d 0x4febb0

// 0x456930 ShadVol_Init — the silhouette facing reference (-1 = extrude FRONT-facing
// tris), the two materials, and the static normal/colour/st arrays for every vertex
// slot.  The kisak stencil pass never reads per-vertex normal/colour/st (it only
// extrudes xyzw), so those stay zero-initialised (BSS).
static bool s_shadVolInit = false;
static void ShadVol_Init()
{
    s_shadVolInit = true;
    s_silhouetteSign = -1;                                 // shadVol_silhouetteFacing = -1
    if ( !mat_stencilshadow )
        mat_stencilshadow = rgp.stencilShadowMaterial;     // "stencilshadow" builtin
    if ( !mat_white_multiply )                             // IDB: Material_RegisterHandle("white_multiply",0)
    {
        extern Material *__cdecl Material_RegisterHandle( const char *name, int imageTrack );
        mat_white_multiply = Material_RegisterHandle( "white_multiply", 0 );
    }
    memset( s_vertHashIdx, 0xFF, sizeof(s_vertHashIdx) );  // all slots empty
    for ( int i = 0; i < SHADVOL_EDGE_HASH_SIZE; ++i )
        s_edgeHash[i].lo = -1;
}

// Accessor for the sun-preview BLACK-WORLD multiply quad (camwnd Cam_SunPrev).  The
// binary registers "white_multiply" inside ShadVol_Init but draws the quad BEFORE the
// shadow-volume build, so register it on demand here.  NULL if the assets lack it.
Material *Radiant_GetWhiteMultiplyMaterial()
{
    if ( !mat_white_multiply )
    {
        extern Material *__cdecl Material_RegisterHandle( const char *name, int imageTrack );
        mat_white_multiply = Material_RegisterHandle( "white_multiply", 0 );
    }
    return mat_white_multiply;
}

// 0x4565b0 ShadVol_AddSilhouetteTri.  One world-space triangle (v1,v2,v3) of a shadow-
// casting face; light is a vec4 {dir.xyz, w}.  Emits the cap indices and records the
// three edges (parity cancels interior edges; the silhouette survives).  Only tris
// whose facing != shadVol_silhouetteFacing (-1) are emitted — i.e. the FRONT (lit) cap.
static void ShadVol_AddSilhouetteTri( const float *v1, const float *v2, const float *v3, const float *light )
{
    vec3_t e0 = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
    vec3_t e1 = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
    vec3_t nrm;
    Vec3Cross( e0, e1, nrm );
    // light-relative plane vector (v1*w - light.xyz)
    float lx = v1[0] * light[3] - light[0];
    float ly = v1[1] * light[3] - light[1];
    float lz = v1[2] * light[3] - light[2];
    // sign = (dot >= 0) ? 1 : -1 and a tri is a cap when sign != shadVol_silhouetteFacing.
    // KISAK: the dot is NEGATED vs the binary.  kisak's Brush_BuildWindings stores face
    // windings wound opposite the CoD4 binary, so the recomputed fan normal comes out
    // negated — without this every toward-light tri scored dot<0 and built 0 tris.  The
    // IDB s_silhouetteSign and all index math below are untouched.
    float dot = -( nrm[0] * lx + nrm[1] * ly + nrm[2] * lz );
    int sign = ( dot >= 0.0f ) ? 1 : -1;
    if ( sign == s_silhouetteSign )                        // != front-cap facing -> skip
        return;

    if ( s_vertexCount > 1021 || s_indexCount + s_edgeIndexCount > 6120 )
        SunLightPreview_PolyOffsetShadows();

    uint16_t i1 = (uint16_t)shadowvolume_01( light, v1 );
    uint16_t i2 = (uint16_t)shadowvolume_01( light, v2 );
    uint16_t i3 = (uint16_t)shadowvolume_01( light, v3 );

    shadowvolume_02_edge( i2, i1, sign );
    shadowvolume_02_edge( i3, i2, sign );
    shadowvolume_02_edge( i1, i3, sign );

    // Front cap (out[0..2]) + back cap (out[3..5]) of the (i1,i2,i3) face.  Each hashed
    // vertex owns TWO xyzw slots, so the real vertex is index 2*i and its extruded twin
    // 2*i+1; the back cap is the front cap with every index +1 and the winding reversed.
    // Winding flips with sign (0x456708: the v22<=0 vs >0 branch).  indexCount advances by
    // shadVol_frontCapIndices, which is 3 for a directional light — the back cap then
    // degenerates (every extruded vertex is the same point at infinity) and out[3..5] is
    // scratch that the next tri overwrites.
    short *out = s_indexBuf + s_indexCount;
    short r1 = (short)(2 * i1), r3 = (short)(2 * i3);
    out[1]     = (short)(2 * i2);                          // word_232E5AA[ic]
    out[1 + 3] = (short)(2 * i2 + 1);                      // word_232E5AA[ic+3]
    if ( sign > 0 )
    {
        out[0]     = r1;               out[2]     = r3;
        out[0 + 3] = (short)(r3 + 1);  out[2 + 3] = (short)(r1 + 1);
    }
    else
    {
        out[0]     = r3;               out[2]     = r1;
        out[0 + 3] = (short)(r1 + 1);  out[2 + 3] = (short)(r3 + 1);
    }
    s_indexCount = (short)(s_indexCount + s_frontCapIndices);
}

// ShadVol_AddFaceFan (IDA 0x4567b0). Fan a face winding (orientation-transformed to
// world) into triangles (v0, v[i-1], v[i]) and feed each to ShadVol_AddSilhouetteTri.
static void ShadVol_AddFaceFan( const winding_t *w, const orientation_t *orient, const float *light )
{
    if ( w->numpoints <= 2 )
        return;
    const float *pts = (const float *)&w->p[0][0];         // point i = pts[3*i .. 3*i+2]
    float v0[3], vPrev[3], vCur[3];
    OrientationPosToWorldPos( v0,    pts + 0, orient );     // fan apex (point 0)
    OrientationPosToWorldPos( vPrev, pts + 3, orient );     // point 1
    for ( int i = 2; i < w->numpoints; ++i )
    {
        OrientationPosToWorldPos( vCur, pts + 3 * i, orient );
        // IDB order: ShadVol_AddSilhouetteTri(newPt, prevPt, pt0). Preserved exactly so the
        // vertex hashing + edge-parity + front-cap index math match the binary bit-for-bit.
        ShadVol_AddSilhouetteTri( vCur, vPrev, v0, light );
        vPrev[0] = vCur[0]; vPrev[1] = vCur[1]; vPrev[2] = vCur[2];
    }
}

// 0x47b190 ShadVol_AddBrushFaces.  Fan every face of a convex brush that faces the
// light (and is neither material-filtered nor non-shadow-casting) into the volume.
// cdecl(def, orient, light) — 0x47b2fc pushes ecx=sb->def, ebx=orient, edi=sundir.
//
// FIRST statement: PlaneToOrientationVec4(localLight, light, orient) (0x47b1a2).  Brush
// face planes live in the brush's LOCAL frame, so the world light vec4 must be brought
// into that frame before the facing test.  Identity (a no-op) for top-level world
// brushes, but omitting it inverts the test on every rotated PREFAB.  The FAN still gets
// the WORLD light — AddFaceFan transforms each winding point to world space itself.
//
// Facing test (0x47b1ce..0x47b203, x87): proceed when local.w*plane.dist < dot.
// plane.dist is read as a QWORD (the binary's plane_t carries a double slot at +0x10)
// but only ever holds float-precision values, so the (double) widen is exact.
//
// Material gates: MtlDef_IsFaceFiltered on the CURRENT LAYER's mtldef (0x47b205 reads
// `4*(9*layer+9)` = face+36+36*layer), then the BASE layer's resolved handle ->
// Material_CastsStencilShadow (0x47b21e..0x47b25c, inlined in the binary).

static void ShadVol_AddBrushFaces( const brush_t *def, const orientation_t *orient, const float *light )
{
    float localLight[4];
    PlaneToOrientationVec4( localLight, light, orient );          // 0x47b1a2
    const int layer = g_qeglobals.current_edit_layer;
    for ( int fi = 0; fi < def->faceCount; ++fi )
    {
        const face_t *f = &def->faces[fi];
        if ( !f->w )                                             // 0x47b1c1
            continue;
        const float *n = f->plane.normal;
        float d = n[0] * localLight[0] + n[1] * localLight[1] + n[2] * localLight[2]; // 0x47b1ed
        if ( (double)localLight[3] * f->plane.dist >= d )         // 0x47b1f0..0x47b203
            continue;
        if ( MtlDef_IsFaceFiltered( (MaterialDef *)&f->mtldef[layer] ) )   // 0x47b212
            continue;
        qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)&f->mtldef[0] ); // 0x47b221
        if ( !lm )
            continue;
        // 0x47b22c — the binary Asserts MaterialDef.cpp:246 "radMtl->handle" then derefs
        // it regardless.  KISAK: kisak realizes editor materials lazily, so an unrealized
        // layer handle is a normal transient state here; skip the face instead.
        if ( !lm->next )
            continue;
        // ASSET-DRIVEN: Material_CastsStencilShadow's last clause compares
        // material->techniqueSet->techniques[1] ("build floatz") against
        // rgp.shadowCasterMaterial's.  EVERY CoD4 shadowcaster techset variant omits
        // "build floatz" -> NULL, while every opaque world techset has it -> non-NULL, so
        // with CoD4 assets the clause rejects every solid world material and the RETAIL
        // binary casts no convex-brush shadows either (only patches and misc_models,
        // whose arms skip this gate).  Kept FAITHFUL.  RADIANT_SHADOWVOL_ALLMTL=1 relaxes
        // only that last clause so the CoD3-era behaviour can be inspected.
        static const bool s_allMtl = []{ const char *e = getenv( "RADIANT_SHADOWVOL_ALLMTL" );
                                         return e && e[0] == '1'; }();
        bool casts = Material_CastsStencilShadow( lm->next );  // 0x47b252
        if ( !casts && s_allMtl )
        {
            const Material *m = lm->next;
            int bidx = ( m->techniqueSet && m->techniqueSet->techniques[5] )
                         ? (unsigned char)m->stateBitsEntry[4] : 0;
            casts = ( m->surfaceFlags & 0x40000 ) == 0 && m->stateBitsTable
                    && ( m->stateBitsTable[bidx].loadBits[0] & 0x7000F00u ) == 0x800u
                    && ( m->stateBitsTable[bidx].loadBits[1] & 1 ) != 0;
        }
        if ( !casts )
            continue;
        ShadVol_AddFaceFan( f->w, orient, light );               // 0x47b26c
    }
}

// 0x456850 ShadVol_AddModelSilhouette.  Walk an extracted XModel index buffer 3 at a
// time, transform each triangle's verts to world space and feed the same silhouette
// builder the brush fan uses.  verts/indices come from Editor_ExtractXModelGeo; stride
// is the vertex byte stride.  Tri count is (indexCount-1)/3 + 1 (the binary's v15).
// Returns indexCount (the binary's __usercall return is unused by its callers).
static int ShadVol_AddModelSilhouette( int indexCount, const uint16_t *indices,
                                       const orientation_t *orient,
                                       const unsigned char *verts, int stride,
                                       const float *light )
{
    if ( indexCount <= 0 )
        return indexCount;                              // 0x45685d
    const uint16_t *ix = indices;                       // v7 = a2 + 4 → indices[2] is *v7
    unsigned tris = (unsigned)( indexCount - 1 ) / 3u + 1;   // v15
    for ( ;; )
    {
        float v0[3], v1[3], v2[3];                      // v10/v11/v12
        const float *p0 = (const float *)( verts + stride * ix[0] );   // a4 + a5*idx
        const float *p1 = (const float *)( verts + stride * ix[1] );
        const float *p2 = (const float *)( verts + stride * ix[2] );
        OrientationPosToWorldPos( v0, p0, orient );
        OrientationPosToWorldPos( v1, p1, orient );
        OrientationPosToWorldPos( v2, p2, orient );
        ShadVol_AddSilhouetteTri( v0, v1, v2, light );   // 0x4568d4 (kisak port returns void)
        ix += 3;                                        // v16 += 3
        if ( !--tris )                                  // 0x4568e4
            break;
    }
    return indexCount;
}

// 0x479bf0 SunLightPreview_DrawModelShadow — the misc_model arm of DrawBrushShadow:
// instance the model, and if an instance exists extract its CPU geometry and add its
// silhouette through the entity orientation.
static char SunLightPreview_DrawModelShadow( selbrush_t *b, orientation_t *orient,
                                             const float *light )
{
    static unsigned char vbuf[196612];                  // v8  — 0x4000 verts * 12 bytes
    static unsigned char ibuf[131076];                  // v11 — 0x10000 indices * 2 + slack

    entity_s *owner = (entity_s *)b->owner;
    SetupModelInst( ((orientation_t *)orient)->origin, owner );   // 0x479c19 (ecx0->origin)
    iassert( !b->owner->prefab );                       // brush.cpp:4190
    entity_s *def = (entity_s *)b->owner->def;          // owner->def___or_firstActive
    if ( !((entity_s *)b->owner)->modelInst )           // owner->modelInst (instance @ +0x44)
        return 0;
    iassert( def->modelClass );                         // brush.cpp:4194
    entitymodel_t *mc = (entitymodel_t *)def->modelClass;
    iassert( mc->model );                               // brush.cpp:4195
    iassert( mc->model->handle );                       // brush.cpp:4196

    orientation_t mor;                                  // v9
    Entity_GetOrientation( (entity_s_def *)def, orient, &mor );   // 0x479ce8
    int indexCount = Editor_ExtractXModelGeo( (XModel *)(intptr_t)mc->model->handle,
                                              (float *)vbuf, 0x4000,
                                              (uint16_t *)ibuf, 0x10000 );   // 0x479d18
    // sub_456850(indexCount, indices, &orient, verts, stride=12, light)
    return (char)ShadVol_AddModelSilhouette( indexCount, (const uint16_t *)ibuf, &mor,
                                             vbuf, 12, light );   // 0x479d38
}

// 0x4418B0 Patch_AddShadowSilhouette — the PATCH arm of DrawBrushShadow.  It lives in
// pmesh.cpp's .text cluster and INLINES MaterialDef_10_LayeredMatHandle, but on the
// patch DEF's LAYER-0 material (patchMesh_t+0x18 `texture`, NOT current_edit_layer).
// It then syncs the instance's tessellated visuals (0x4405A0) and feeds the tessellated
// mesh to the SAME builder the misc_model arm uses, with stride sizeof(curveVert_t):
// a patch casts its shadow from its render tessellation, not from a convex hull.
static char Patch_AddShadowSilhouette( orientation_t *orient, patch_t *patch, const float *light )
{
    qtexture_s *lm = MaterialDef_GetLayeredMaterial( (MaterialDef *)&patch->def->texture ); // 0x4418b9
    if ( !lm )
        return 0;
    // 0x4418c4 — same KISAK reduction as the brush gate (skip an unrealized handle).
    if ( !lm->next )
        return 0;
    if ( !Material_CastsStencilShadow( lm->next ) )                        // 0x4418ea
        return 0;
    Patch_Fill_SyncVersion( patch, orient );                               // 0x4418fa (PMESH_25)
    if ( !patch->def->curveDef || !patch->indicesFront )
        return 0;                        // no tessellation yet (headless / pre-realize)
    return (char)ShadVol_AddModelSilhouette( patch->indexCount,            // 0x441919
                                             patch->indicesFront, orient,
                                             (const unsigned char *)patch->def->curveDef->verts,
                                             (int)sizeof( curveVert_t ), light );
}


// The misc_model arm's BODY (0x47b2e1 + 0x47b2f0), factored out so camwnd.cpp can run
// it inside the asset-load guard bracket (see the call site below).
char Radiant_ShadVol_ModelShadowArm( selbrush_t *sb, orientation_t *orient, const float *light )
{
    if ( !Entity_HasRenderableModel( (brush_t_with_custom_def *)sb, (int)orient ) )  // 0x47b2e1
        return 0;
    return SunLightPreview_DrawModelShadow( sb, orient, light );                     // 0x47b2f0
}

static void SunLightPreview_BrushShadow( selbrush_t *listHead, orientation_t *orient, const float *light );

// 0x47b2a0 SunLightPreview_DrawBrushShadow — __usercall(light@eax, sb@edx, orient@ecx).
// Dispatch in the binary's order: (1) sb->patch non-null -> the patch arm, RETURN;
// (2) Brush_CheckBuildFaceVis(sb, orient) — refresh this instance's per-face visuals
// (version-gated, so a no-op after the camera pass has run this frame); (3) a fixedsize
// owner -> the misc_model arm, gated on def->unk01 LOBYTE (modelFailed) == 0 and
// Entity_HasRenderableModel; (4) else the convex face fan.
// PREFAB CONTENTS are the CALLER's recursion (0x47b310), not handled here.
static void SunLightPreview_DrawBrushShadow( const float *light, selbrush_t *sb, orientation_t *orient )
{
    patch_t *patch = sb->patch;                              // 0x47b2a7
    if ( patch )
    {
        if ( patch->def )
        {
            Patch_AddShadowSilhouette( orient, patch, light ); // 0x47b2b3
        }
        return;
    }
    if ( !sb->def )
        return;
    sub_477D70( sb, (const float *)orient );                 // 0x47b2c0 Brush_CheckBuildFaceVis
    entity_s_def *ed = sb->owner ? (entity_s_def *)sb->owner->def : nullptr;
    if ( ed && ed->eclass && *(int *)&ed->eclass->fixedsize )  // 0x47b2d1
    {
        brush_t *def = sb->def;                              // 0x47b2d7
        if ( !( def->unk01 & 0xFF ) )                        // 0x47b2da (LOBYTE modelFailed)
        {
            // KISAK: 0x47b2e1/0x47b2f0 run inside the port's ASSET-LOAD GUARD bracket.
            // Entity_HasRenderableModel drives Model_SetModel -> R_RegisterModel, and stock
            // CoD4 xmodels carry phys_collmaps / .tech files the CoD3-sized kisak parser
            // ERR_DROPs on; every other model-load site brackets that the same way.  The
            // ARM itself is the binary's code.
            extern int Editor_ModelShadowGuarded( selbrush_t *b, orientation_t *orient,
                                                  const float *light );   // camwnd.cpp
            Editor_ModelShadowGuarded( sb, orient, light );
        }
        return;
    }
    ShadVol_AddBrushFaces( sb->def, orient, light );          // 0x47b302
}

// 0x47b310 SunLightPreview_BrushShadow.  Walk a brush DISPLAY list (`listHead` is the
// sentinel), skipping FilterBrush'd brushes.  A brush whose owner entity is a PLACED
// PREFAB does NOT cast its own bbox shadow: the prefab's placement orientation is
// composed onto the caller's and the prefab's own instanced brush list is walked
// recursively.  mp_backlot's world is prefab CONTENTS + patches with ZERO top-level
// convex brushes, so without this recursion it builds no volumes at all.
static void SunLightPreview_BrushShadow( selbrush_t *listHead, orientation_t *orient, const float *light )
{
    selbrush_t *sentinel = listHead;                          // x_sb (0x47b319)
    for ( selbrush_t *b = listHead->next; b && b != sentinel; b = b->next )  // 0x47b31c/0x47b372
    {
        if ( FilterBrush( b, 0 ) )                            // 0x47b328
            continue;
        entity_s *ent = b->owner;                             // 0x47b334
        if ( ent && ent->prefab )                             // 0x47b337
        {
            prefab_s *pf = (prefab_s *)ent->prefab;
            orientation_t prefabOrient;                       // orient_out
            Entity_GetOrientation( (entity_s_def *)ent->def, orient, &prefabOrient ); // 0x47b343
            // 0x47b359 — the recursion head is `&prefab->active_brushlist` reinterpreted
            // as a list node, so ->next is active_brushlist_next (the head sentinel).
            SunLightPreview_BrushShadow( (selbrush_t *)&pf->active_brushlist,
                                         &prefabOrient, light );
            continue;
        }
        SunLightPreview_DrawBrushShadow( light, b, orient );   // 0x47b36d
    }
}

// The R_SunPrev_Main (0x4069c0) shadow slice, exported for camwnd.cpp's call site.
// Between the clear-alpha/stencil quad and the lit re-add the binary runs:
//   shadVol_frontCapIndices = shadVol_quadsPerEdge = 3   (0x406a4c/0x406a66)
//   SunLightPreview_BrushShadow(&active_brushes,   &world_orient_matrix, sundir)
//   SunLightPreview_BrushShadow(&selected_brushes, &world_orient_matrix, sundir)
//   SunLightPreview_PolyOffsetShadows()
// Radiant_ShadVol_Begin does the one-time init + the two counters and reports whether
// the stencilshadow material exists; the caller makes the three calls itself so the
// sequence reads as the binary's.  3 = one cap per silhouette tri, the directional
// (w==0) case: every extruded vertex is the same point at infinity so the back cap
// degenerates away.  Point lights use 6 = front + back cap.
bool Radiant_ShadVol_Begin( int frontCapPerTri )
{
    if ( !s_shadVolInit )
        ShadVol_Init();
    if ( !mat_stencilshadow )
        return false;                     // stencilshadow material not loaded (modtools gap)

    s_frontCapIndices = frontCapPerTri;          // shadVol_frontCapIndices (0x406a66)
    s_quadsPerEdge    = frontCapPerTri;          // shadVol_quadsPerEdge    (0x406a6b)

    return true;
}

// The two faithful entry points, exported under their binary names.
void Radiant_ShadVol_BrushShadow( selbrush_t *listHead, const orientation_t *orient, const float *light )
{
    SunLightPreview_BrushShadow( listHead, (orientation_t *)orient, light );
}
