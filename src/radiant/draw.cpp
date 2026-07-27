#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\draw.cpp
// Editor line-drawing bridge — ported from CoD4Radiant.exe (IW3xRadiant.i64, port 13343).
// The grid / origin-box / angle / region / Z-view draws build batches of line segments
// here and hand them to the renderer's deferred command buffer (R_AddCmd_Line3D, added
// as a KISAK_RADIANT editor-delta at the end of gfx_d3d/r_rendercmds.cpp).
//
// No GtkRadiant reference exists (GtkRadiant draws lines through qgl/OpenGL); this is the
// CoD D3D path, transcribed from the IDB.

#include "stdafx.h"
#include <gfx_d3d/r_rendercmds.h>   // R_AddCmd_Line3D (KISAK_RADIANT editor-delta)

static_assert(sizeof(orientation_t) == 48, "orientation_t (fxprimitives.h != IDB)");
static_assert(sizeof(GfxPointVertex) == 16, "GfxPointVertex (r_gfx.h != IDB)");

bool g_radiantTerrainOverlayNoDepth = false;

// ─── Orientation helpers (IDB 0x4ba430 / 0x4ba4b0, q_shared.cpp:1599/1608) ──────
// kisak only carries the FX_-prefixed variants (different arg order); the editor's
// brush/draw code references these unprefixed forms.  Defined here - the single home in
// the radiant target - rather than touching the shared universal TU.
// out = orient.origin + axis^T * pos.
void OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient )
{
    iassert( pos != out );
    out[0] = orient->axis[0][0] * pos[0] + orient->origin[0]
           + orient->axis[1][0] * pos[1] + orient->axis[2][0] * pos[2];
    out[1] = orient->axis[0][1] * pos[0] + orient->origin[1]
           + orient->axis[1][1] * pos[1] + orient->axis[2][1] * pos[2];
    out[2] = orient->axis[0][2] * pos[0] + orient->origin[2]
           + orient->axis[1][2] * pos[1] + orient->axis[2][2] * pos[2];
}

void OrientationDirToWorldDir( float *out, const orientation_t *orient, const float *dir )
{
    iassert( dir != out );
    out[0] = orient->axis[1][0] * dir[1] + orient->axis[0][0] * dir[0] + orient->axis[2][0] * dir[2];
    out[1] = orient->axis[1][1] * dir[1] + orient->axis[0][1] * dir[0] + orient->axis[2][1] * dir[2];
    out[2] = orient->axis[1][2] * dir[1] + orient->axis[0][2] * dir[0] + orient->axis[2][2] * dir[2];
}

// ─── PlaneToOrientationVec4 (IDB 0x4BA720, q_shared.cpp:1660) ───────────────────
// Bring a HOMOGENEOUS vec4 {xyz, w} from world space into the orientation's LOCAL
// frame: subtract w·origin, then rotate by the axis ROWS (the same inverse rotation
// OrientationWorldPosToLocalPos uses), keeping w.  With w == 1 this is a point, with
// w == 0 a pure direction (the origin term drops out).
// Load-bearing for the sun-preview shadow builder (ShadVol_AddBrushFaces 0x47B190): brush
// face planes are stored in the brush's LOCAL frame, so the world-space light vec4 must be
// brought into that frame before the plane-facing test.  For top-level world brushes the
// orientation is the identity and the transform is a no-op; for PREFAB CONTENTS it is the
// composed placement orientation, and skipping it inverts the facing test.
void PlaneToOrientationVec4( float *out, const float *plane, const orientation_t *orient )
{
    iassert( plane != out );                                   // 0x4ba728 ("pos != out")
    const float v0 = plane[0] - orient->origin[0] * plane[3];  // 0x4ba751
    const float v1 = plane[1] - orient->origin[1] * plane[3];  // 0x4ba75f
    const float v2 = plane[2] - orient->origin[2] * plane[3];  // 0x4ba76d
    out[0] = orient->axis[0][2] * v2 + orient->axis[0][0] * v0 + orient->axis[0][1] * v1; // 0x4ba796
    out[1] = orient->axis[1][1] * v1 + orient->axis[1][0] * v0 + orient->axis[1][2] * v2; // 0x4ba7ab
    out[2] = v0 * orient->axis[2][0] + v1 * orient->axis[2][1] + v2 * orient->axis[2][2]; // 0x4ba7c1
    out[3] = plane[3];                                                                     // 0x4ba7c7
}

// ─── OrientationWorldPosToLocalPos (IDB sub_4BA610, 0x4BA610, q_shared.cpp:1638) ──
// The INVERSE of OrientationPosToWorldPos: bring a WORLD position into the orientation's
// LOCAL frame — subtract the origin, then rotate by the axis ROWS (axis · rel), NOT axisᵀ.
//   out[i] = axis[i] · (pos − origin)
// This is a DIFFERENT binary function from OrientationPosToWorldPos (0x4ba430): calling
// that one here double-ADDS the origin instead of subtracting it, which puts the
// transformed ray start ~2*origin off and misses every prefab/model pick.
// IDA asserts pos != out.
void OrientationWorldPosToLocalPos( float *out, const float *pos, const orientation_t *orient )
{
    iassert( pos != out );
    const float rel0 = pos[0] - orient->origin[0];
    const float rel1 = pos[1] - orient->origin[1];
    const float rel2 = pos[2] - orient->origin[2];
    out[0] = orient->axis[0][0] * rel0 + orient->axis[0][1] * rel1 + orient->axis[0][2] * rel2;
    out[1] = orient->axis[1][0] * rel0 + orient->axis[1][1] * rel1 + orient->axis[1][2] * rel2;
    out[2] = orient->axis[2][0] * rel0 + orient->axis[2][1] * rel1 + orient->axis[2][2] * rel2;
}

// ─── VectorRotateByAxis (IDB sub_4BA6B0, 0x4BA6B0, q_shared.cpp:1652) ────────────
// Rotate `dir` by the rotation part of a 4-row [4][3] axis matrix (rows 1,2,3 — row 0
// is the origin/forward and is skipped).  out[i] = row(i+1) · dir.  Used by the
// prefab-enter (Prefab_NextLevel) camera reframe to map the saved camera axes into the
// prefab's local space.  IDA asserts dir != out.
void VectorRotateByAxis( float *out, const float *axisMatrix, const float *dir )
{
    iassert( dir != out );
    // axisMatrix laid out as float[4][3]: [3..5]=row1, [6..8]=row2, [9..11]=row3.
    out[0] = axisMatrix[4]  * dir[1] + axisMatrix[3]  * dir[0] + axisMatrix[5]  * dir[2];
    out[1] = axisMatrix[7]  * dir[1] + axisMatrix[6]  * dir[0] + axisMatrix[8]  * dir[2];
    out[2] = axisMatrix[10] * dir[1] + axisMatrix[9]  * dir[0] + axisMatrix[11] * dir[2];
}

// ─── R_Add3DLine (IDB 0x40c110) ─────────────────────────────────────────────────
// Append one line segment (p1→p2, each transformed through `orient`) into a caller-
// owned GfxPointVertex batch. Flushes the batch through R_AddCmd_Line3D when the next
// segment would overflow `maxVertCount`. Returns the new vertex count.
// Usercall (verts@eax, orient@ecx) normalized to cdecl from the call sites, e.g.
//   vertCount = R_Add3DLine(verts, &world_orient_matrix, p1, p2, &color, 2, vertCount, 1362);
// `color` is a pointer to a packed Byte4 pixel color; `width` (the 6th IDB arg) is the
// line pixel width forwarded to the flush — NOT a depth-test flag.
// KISAK: differs from 0x40c110 — the flush routes through R_AddCmd_Line3DNoDepth when
// g_radiantTerrainOverlayNoDepth is set (the binary always uses R_AddCmd_Line3D).
int R_Add3DLine( GfxPointVertex *verts, const orientation_t *orient,
                 const float *p1, const float *p2, const unsigned int *color,
                 char width, int vertCount, int maxVertCount )
{
    iassert( vertCount >= 0 );
    iassert( (vertCount & 1) == 0 );
    iassert( maxVertCount >= 2 );

    if ( vertCount + 2 > maxVertCount )
    {
        if ( g_radiantTerrainOverlayNoDepth )
            R_AddCmd_Line3DNoDepth( (short)(vertCount / 2), width, verts );
        else
            R_AddCmd_Line3D( (short)(vertCount / 2), width, verts );
        vertCount = 0;
    }

    GfxPointVertex *v = &verts[vertCount];
    OrientationPosToWorldPos( v->xyz, p1, orient );
    OrientationPosToWorldPos( v[1].xyz, p2, orient );
    *(unsigned int *)v->color   = *color;
    *(unsigned int *)v[1].color = *color;
    return vertCount + 2;
}

// ─── Draw_02 (IDB 0x40c530) ─────────────────────────────────────────────────────
// Emit the closed outline of a winding as line segments into a GfxPointVertex batch,
// including the wrap-around last→first edge. Caller: DrawRegions (0x406b30).
// `poly` layout: poly[0] = numpoints, vec3 points start at byte offset 4 (poly+1).
// Pointer arithmetic transcribed verbatim from the IDB: `cur` tracks the LAST
// float of the current point, `prevOff` the byte offset of the previous point.
int Draw_02( const unsigned int *poly, const int *color, int vertCount, GfxPointVertex *verts )
{
    iassert( vertCount >= 0 );
    iassert( (vertCount & 1) == 0 );

    if ( poly[0] )
    {
        unsigned int prevOff = 4 * (3 * poly[0] - 3);     // → last point (wrap-around prev)
        const float *cur = (const float *)(poly + 3);     // → last float of point[0]
        unsigned int i = 0;
        do
        {
            if ( vertCount + 2 > 1362 )
            {
                R_AddCmd_Line3D( (short)(vertCount / 2), 1, verts );
                vertCount = 0;
            }
            GfxPointVertex *v = &verts[vertCount];
            int col = *color;
            v->xyz[0] = *(const float *)((const char *)poly + prevOff + 4);
            v->xyz[1] = *(const float *)((const char *)poly + prevOff + 8);
            v->xyz[2] = *(const float *)((const char *)poly + prevOff + 12);
            vertCount += 2;
            v[1].xyz[0] = *(cur - 2);
            v[1].xyz[1] = *(cur - 1);
            v[1].xyz[2] = *cur;
            *(unsigned int *)v->color   = col;
            *(unsigned int *)v[1].color = col;
            prevOff = (unsigned int)((const char *)cur - 12 - (const char *)poly);
            cur += 3;
            ++i;
        }
        while ( i < poly[0] );
    }
    return vertCount;
}

// Lines_AddLinkToScript (0x46b590) is NOT a draw.cpp function: its asserts are at
// XYWnd.cpp:3238/3272 - it is CXYWnd's script-link visualizer (entity script_linkTo
// arrows).  Noted here so it is not re-added to this file by an address-range guess.
