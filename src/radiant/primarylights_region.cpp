#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Convex light-region CSG used by the selected-light region preview.

#include "stdafx.h"
#include "winding.h"
#include "primarylights_region.h"        // rface_t (defined here), lightDesc_t, public API
#include <universal/com_math.h>          // Vec3Cross, Vec3Normalize_R, Vec3NormalizeTo, PerpendicularVector
#include <universal/com_convexhull.h>    // Com_ConvexHull, Com_ConvexHullIndices
#include <universal/assertive.h>         // iassert / vassert (USE_ASSERTS → MyAssertHandler)
#include <math.h>

// ─── engine helpers ──────────────────────────────────────────────────────────
// In the radiant build Com_PrintMessage is the cmdlib varargs console printer.
extern void Com_PrintMessage( const char *fmt, ... );

// Vec3Normalize_R (0x40A5E0) — in-place normalize returning the pre-normalize length
// (engine_stubs.cpp; no header).  Vec3Cross / Vec3NormalizeTo / PerpendicularVector
// come from com_math.h.
extern float Vec3Normalize_R( float *v );

static inline void PLR_OutOfMem( const char *msg ) { Com_PrintMessage( "%s", msg ); }

// winding alloc counter (dword_24CE4FC) — winding.cpp's g_windingAlloc.
extern int g_windingAlloc;

// Region_ReverseWinding (0x4D7DC0) — fresh winding with reversed point order.  brush.cpp
// has a file-static copy (edwind::); ported locally here (15-line shared polylib fn).
static winding_t *Region_ReverseWinding( const winding_t *w )
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

// Polylib helpers (winding.cpp; exposed via primarylights_region.h for this file).
winding_t *Polylib_HullForPoints( const vec3_t *xyz, const float *normal, int xyzCount );
void       Polylib_PickProjectionAxes( const float *normal, int *axis0, int *axis1 );

// ─── the region-face node ────────────────────────────────────────────────────
struct rface_t
{
    float    plane[4];    // +0x00 normal.xyz + dist
    float    dir[3];      // +0x10 cone axis
    float    cosHalfFov;  // +0x1C
    int      exterior;    // +0x20 (a4)
    winding_t *w;         // +0x24
    float    pad;         // +0x28
    rface_t *next;        // +0x2C
};
static_assert( sizeof(rface_t) == 0x30, "rface_t (primarylights region node) != 0x30" );

// ═════════════════════════════════════════════════════════════════════════════
//  s_kdopNormals (unk_6DD214 / unk_6DD218 / flt_6DD258) — the 26-direction k-DOP
//  axis table: 6 axial, 12 edge-diagonal (±1/√2), 8 corner (±1/√3).  Read exactly
//  from the IDB.  sub_4DAD20 walks 26 dirs starting at unk_6DD218 (== &table[-? ];
//  see below) and clips against the 20 diagonal dirs from flt_6DD258.
// ═════════════════════════════════════════════════════════════════════════════
static const float SQRT2_INV = 0.70710677f;   // 0x3F3504F3
static const float SQRT3_INV = 0.5773502588272095f;

// unk_6DD218: the 26 k-DOP directions sub_4DAD20 iterates (it dereferences v3-2..v3,
// stepping +3 each loop for 26 iters → 26 vec3 starting two floats BEFORE the named
// label, i.e. the table actually begins at a vec3 boundary).  IDA's unk_6DD218 dump
// (26 vec3) is the authoritative sequence:
static const float s_kdopDirs[26][3] =
{
    {  0.0f,  0.0f, -1.0f }, {  0.0f,  0.0f,  1.0f }, {  0.0f, -1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  1.0f,  0.0f,  0.0f },
    {  SQRT2_INV,  SQRT2_INV,  0.0f }, { -SQRT2_INV, -SQRT2_INV,  0.0f },
    {  SQRT2_INV, -SQRT2_INV,  0.0f }, { -SQRT2_INV,  SQRT2_INV,  0.0f },
    {  SQRT2_INV,  0.0f,  SQRT2_INV }, { -SQRT2_INV,  0.0f, -SQRT2_INV },
    {  SQRT2_INV,  0.0f, -SQRT2_INV }, { -SQRT2_INV,  0.0f,  SQRT2_INV },
    {  0.0f,  SQRT2_INV,  SQRT2_INV }, {  0.0f, -SQRT2_INV, -SQRT2_INV },
    {  0.0f,  SQRT2_INV, -SQRT2_INV }, {  0.0f, -SQRT2_INV,  SQRT2_INV },
    {  SQRT3_INV,  SQRT3_INV,  SQRT3_INV }, { -SQRT3_INV, -SQRT3_INV, -SQRT3_INV },
    {  SQRT3_INV,  SQRT3_INV, -SQRT3_INV }, { -SQRT3_INV, -SQRT3_INV,  SQRT3_INV },
    {  SQRT3_INV, -SQRT3_INV,  SQRT3_INV }, { -SQRT3_INV,  SQRT3_INV, -SQRT3_INV },
    {  SQRT3_INV, -SQRT3_INV, -SQRT3_INV }, { -SQRT3_INV,  SQRT3_INV,  SQRT3_INV },
};

// flt_6DD258: the 20 diagonal directions sub_4DAD20 clips each base box plane against
// (= s_kdopDirs[6..25], the non-axial subset).
static const float (* const s_kdopDiag)[3] = &s_kdopDirs[6];   // 20 entries

// s_kdopNormals (unk_6DD214): used by sub_4DCE20 — 13 antipodal pairs (the same table,
// one float EARLIER: starts at the {0,0,-1}/{0,0,1} pair as 13 *pairs* of normals, read
// with stride 6 floats).  Same data as s_kdopDirs flattened; sub_4DCE20 reads it as
// 13 entries with a +6-float stride and verifies each pair is antipodal.
static const float *s_kdopNormalsFlat = &s_kdopDirs[0][0];

// ─── d_lightRegionHulls global sink (dword_1807E00 / dword_25D5A4C) lives in
//     camwnd.cpp (sub_406C50 fills it; the parked GPU light draw consumes it). ───

// =============================================================================
//  Winding_Plane (sub_4D6EF0) — winding best-fit plane: normal in out[0..2],
//  dist in out[3]; returns 0.5 * sum-of-cross-magnitudes (the winding area).
//  (common\polylib.cpp inline; not the editor winding.cpp Winding_Plane.)
// =============================================================================
float Region_WindingPlane( float *out, const winding_t *w )
{
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    for ( int i = 2; i < w->numpoints; i++ )
    {
        float v10[3], v9[3], v8[3];
        v10[0] = w->p[i - 1][0] - w->p[0][0];
        v10[1] = w->p[i - 1][1] - w->p[0][1];
        v10[2] = w->p[i - 1][2] - w->p[0][2];
        v9[0]  = w->p[i][0] - w->p[0][0];
        v9[1]  = w->p[i][1] - w->p[0][1];
        v9[2]  = w->p[i][2] - w->p[0][2];
        Vec3Cross( v9, v10, v8 );
        out[0] += v8[0];
        out[1] += v8[1];
        out[2] += v8[2];
    }
    float area = Vec3Normalize_R( out ) * 0.5f;
    out[3] = w->p[0][0] * out[0] + w->p[0][1] * out[1] + w->p[0][2] * out[2];
    return area;
}

// =============================================================================
//  Winding_Clip_real_ (0x4D83B0, common\polylib.cpp:~700) — destructive in-place
//  ClipWindingEpsilon keeping the BACK side (dist <= epsilon).  *a1 is freed and
//  replaced with the clipped winding (or NULL if the whole winding is in front).
//  Faithful transcription of the disasm (SIDE arrays + edge interpolation, with the
//  axial-plane snap-to-dist optimisation for unit normals).
// =============================================================================
// Keeps the FRONT side (dist > epsilon) plus ON points; drops the BACK side
// (dist <= -epsilon).  *inout is freed+replaced with the clipped winding, set to
// NULL when nothing is in front, or left unchanged when nothing is in back.
// (Disasm 0x4D83B0: side 0=FRONT/d>eps, 1=BACK/d<=-eps, 2=ON; counts[0]=front etc.)
void Winding_Clip_real_( winding_t **inout, const float *normal, float dist, float epsilon )
{
    winding_t *in = *inout;
    unsigned int np = in->numpoints;

    float dists[1028];   // var_2048..
    int   sides[1029];   // var_1038..
    int   counts[3] = { 0, 0, 0 };   // var_14(front)/var_10(back)/var_C(on)

    unsigned int i;
    for ( i = 0; i < np; i++ )
    {
        float d = in->p[i][0] * normal[0] + in->p[i][1] * normal[1] + in->p[i][2] * normal[2] - dist;
        dists[i] = d;
        if ( d > epsilon )
            sides[i] = 0;          // SIDE_FRONT
        else if ( d > -epsilon )
            sides[i] = 2;          // SIDE_ON
        else
            sides[i] = 1;          // SIDE_BACK
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    if ( counts[0] == 0 )          // nothing in front → drop the winding entirely
    {
        --g_windingAlloc;
        free( in );
        *inout = 0;
        return;
    }
    if ( counts[1] == 0 )          // nothing in back → keep whole winding unchanged
        return;

    unsigned int maxpts = np + 4;
    ++g_windingAlloc;
    winding_t *f = (winding_t *)malloc( 12 * maxpts + 4 );
    if ( !f )
        PLR_OutOfMem( "out of memory: winding_t\n" );
    f->numpoints = 0;

    for ( i = 0; i < (unsigned int)in->numpoints; i++ )
    {
        const float *p1 = in->p[i];
        if ( sides[i] == 2 )       // SIDE_ON → keep, then next
        {
            float *o = f->p[f->numpoints];
            o[0] = p1[0]; o[1] = p1[1]; o[2] = p1[2];
            f->numpoints++;
            continue;
        }
        if ( sides[i] == 0 )       // SIDE_FRONT → keep
        {
            float *o = f->p[f->numpoints];
            o[0] = p1[0]; o[1] = p1[1]; o[2] = p1[2];
            f->numpoints++;
        }
        // edge to next point crosses the plane?
        int sn = sides[i + 1];
        if ( sn == 2 || sn == sides[i] )
            continue;
        const float *p2 = in->p[(i + 1) % in->numpoints];
        float mid[3];
        float frac = dists[i] / ( dists[i] - dists[i + 1] );
        for ( int j = 0; j < 3; j++ )
        {
            if ( normal[j] == 1.0f )
                mid[j] = dist;
            else if ( normal[j] == -1.0f )
                mid[j] = -dist;
            else
                mid[j] = p1[j] + frac * ( p2[j] - p1[j] );
        }
        float *o = f->p[f->numpoints];
        o[0] = mid[0]; o[1] = mid[1]; o[2] = mid[2];
        f->numpoints++;
    }

    if ( (unsigned int)f->numpoints > maxpts )
        vassert( (unsigned int)f->numpoints <= maxpts, "%i, %i", f->numpoints, maxpts );  // polylib.cpp:760
    if ( f->numpoints > 0x400 )
        Com_PrintMessage( "ClipWinding: MAX_POINTS_ON_WINDING" );

    --g_windingAlloc;
    free( in );
    *inout = f;
}

// fwd decls (mutual recursion / forward use)
static void Region_FitFaceCone( rface_t *face );

// =============================================================================
//  sub_4DA360 - cone-projection kernel.  Projects each of the `count` normalized
//  per-point directions (the dirs[] array, vec3 stride) onto test axis `testDir`,
//  tracking min (*outMin) and max (*outMax).  If the min beats the running best
//  (face->cosHalfFov), records testDir's negation as the new cone axis & cosine.
//  Returns 1 if it improved the cone.
// =============================================================================
static char Region_ConeProject( const float *testDir, float *outMin, rface_t *face,
                                 const float *dirs, int count, float *outMax )
{
    *outMin = 1.0f;
    *outMax = -1.0f;
    for ( int i = 0; i < count; i++ )
    {
        const float *p = dirs + 3 * i;
        float d = p[0] * testDir[0] + p[1] * testDir[1] + p[2] * testDir[2];
        if ( d < *outMin ) *outMin = d;
        if ( *outMax < d ) *outMax = d;
    }
    if ( *outMin <= face->cosHalfFov )
        return 0;
    face->cosHalfFov = *outMin;          // face[7]
    face->dir[0] = -testDir[0];          // face[4..6]
    face->dir[1] = -testDir[1];
    face->dir[2] = -testDir[2];
    return 1;
}

// =============================================================================
//  sub_4DA550 - compute the bounding cone (dir + cosHalfFov) for a region face's
//  winding.  Builds per-point unit directions from the face plane, seeds the cone
//  with the centroid then the (negated) plane normal, then iteratively re-centers
//  (minimal cone) until convergence (<=10000 iters).
// =============================================================================
static void Region_FitFaceCone( rface_t *face )
{
    winding_t *w = face->w;
    if ( !w )
        vassert( w, "%s", "face->w" );                 // primarylights_region.cpp:162
    if ( (unsigned int)w->numpoints > 0x400 )
        vassert( (unsigned int)w->numpoints <= 0x400, "%i, %i", w->numpoints, 1024 );  // :163

    float dirs[1024][3];                                // v20/v21 (~1024 vec3)
    float centroid[3] = { 0.0f, 0.0f, 0.0f };
    if ( w->numpoints != 0 )
    {
        for ( int i = 0; i < w->numpoints; i++ )
        {
            Vec3NormalizeTo( w->p[i], dirs[i] );
            centroid[0] += dirs[i][0];
            centroid[1] += dirs[i][1];
            centroid[2] += dirs[i][2];
        }
    }
    float testDir[3];
    Vec3NormalizeTo( centroid, testDir );

    face->cosHalfFov = -2.0f;
    float cosMin, cosMax;
    Region_ConeProject( testDir, &cosMin, face, &dirs[0][0], w->numpoints, &cosMax );

    if ( face->cosHalfFov <= -1.0f )
        vassert( face->cosHalfFov > -1.0f, "%g, %g", face->cosHalfFov, -1.0 );          // :176

    if ( face->cosHalfFov < 0.9800000190734863f )
    {
        // re-seed with the (negated) plane normal
        testDir[0] = -face->plane[0];
        testDir[1] = -face->plane[1];
        testDir[2] = -face->plane[2];
        Region_ConeProject( testDir, &cosMin, face, &dirs[0][0], w->numpoints, &cosMax );
        if ( cosMin <= 0.0f )
            vassert( cosMin > 0.0f, "(cosHalfFovMin) = %g", cosMin );                   // :182

        if ( face->cosHalfFov < 0.9800000190734863f )
        {
            for ( int iter = 0; ; ++iter )
            {
                if ( (unsigned int)( iter + 1 ) >= 0x2710 )
                    vassert( iter < 10000, "%s", "iterations < 10000" );                // :191

                float prevCos = face->cosHalfFov;
                float band = cosMin + ( cosMax - cosMin ) * 0.03125f;
                float acc[3];
                acc[0] = -face->dir[0];
                acc[1] = -face->dir[1];
                acc[2] = -face->dir[2];
                for ( int i = 0; i < w->numpoints; i++ )
                {
                    const float *p = dirs[i];
                    float d = face->dir[0] * p[0] + face->dir[1] * p[1] + face->dir[2] * p[2];
                    if ( -d <= band )
                    {
                        acc[0] += p[0] * 0.0625f;
                        acc[1] += p[1] * 0.0625f;
                        acc[2] += p[2] * 0.0625f;
                    }
                }
                Vec3NormalizeTo( acc, testDir );

                if ( testDir[0] == face->dir[0] && testDir[1] == face->dir[1] && testDir[2] == face->dir[2] )
                    vassert( !( testDir[0] == face->dir[0] && testDir[1] == face->dir[1] && testDir[2] == face->dir[2] ),
                             "%s", "!Vec3Compare( face->dir, testDir )" );               // :204

                if ( !Region_ConeProject( testDir, &cosMin, face, &dirs[0][0], w->numpoints, &cosMax )
                     || prevCos + 0.00009999999747378752f > face->cosHalfFov )
                    break;
            }
            if ( face->cosHalfFov <= 0.0f )
                vassert( face->cosHalfFov > 0.0f, "%g, %g", face->cosHalfFov, 0.0 );     // :212
        }
    }
}

// =============================================================================
//  sub_4DAA70 - build a region face from a world-space winding `w`, link it onto
//  *outList.  Translates the winding relative to the light position a3 (a3[1..3] =
//  light origin, a3[7] = radius), computes its plane, optionally reverses it
//  (front-facing dropped only when `exterior`), clips it back to the light sphere
//  by each edge's tangent plane, then allocates the rface node + fits its cone.
// =============================================================================
void sub_4DAA70( const winding_t *w, rface_t **outList, const float *a3, int exterior )
{
    ++g_windingAlloc;
    winding_t *lw = (winding_t *)malloc( 12 * w->numpoints + 4 );
    if ( !lw )
        Com_PrintMessage( "out of memory: winding_t\n" );
    lw->numpoints = w->numpoints;
    for ( unsigned int i = 0; i < (unsigned int)w->numpoints; i++ )
    {
        lw->p[i][0] = w->p[i][0] - a3[1];
        lw->p[i][1] = w->p[i][1] - a3[2];
        lw->p[i][2] = w->p[i][2] - a3[3];
    }

    float plane[4];
    if ( Region_WindingPlane( plane, lw ) == 0.0f )    // degenerate area
    {
        --g_windingAlloc;
        free( lw );
        return;
    }
    if ( I_fabs( plane[3] ) < 0.001f )                 // light is on the plane
    {
        --g_windingAlloc;
        free( lw );
        return;
    }
    if ( plane[3] > 0.0f )
    {
        if ( exterior )                                // front-facing & exterior -> drop
        {
            --g_windingAlloc;
            free( lw );
            return;
        }
        plane[0] = -plane[0];
        plane[1] = -plane[1];
        plane[2] = -plane[2];
        plane[3] = -plane[3];
        winding_t *rev = Region_ReverseWinding( lw );
        --g_windingAlloc;
        free( lw );
        lw = rev;
    }

    // Clip the (light-relative) winding back to the light sphere along each edge's
    // tangent plane.  v12 walks the ORIGINAL world winding `w` (disasm a1+3); the
    // tangent normal is the world light->vertex direction (light origin - w vertex).
    for ( int i = 0; i < w->numpoints; i++ )
    {
        float dx = a3[1] - w->p[i][0];
        float dy = a3[2] - w->p[i][1];
        float dz = a3[3] - w->p[i][2];
        float distSq = dy * dy + dx * dx + dz * dz;
        float radSq  = a3[7] * a3[7];
        if ( radSq < distSq )
        {
            float invLen = 1.0f / sqrtf( distSq );
            float n[3] = { invLen * dx, dy * invLen, invLen * dz };
            Winding_Clip_real_( &lw, n, -a3[7], 0.1f );
            if ( !lw )
                break;
        }
    }

    if ( lw )
    {
        rface_t *face = (rface_t *)operator new( sizeof( rface_t ) );
        if ( !face )
            Com_PrintMessage( "Out of memory" );
        face->plane[0] = plane[0];
        face->plane[1] = plane[1];
        face->plane[2] = plane[2];
        face->plane[3] = plane[3];
        face->exterior = exterior;                     // +0x20
        face->pad      = 0.0f;                          // +0x28
        face->w        = lw;                            // +0x24
        face->next     = *outList;                      // +0x2C
        *outList       = face;
        Region_FitFaceCone( face );
    }
}

// =============================================================================
//  sub_4DAD20 — add the light's bounding-cube exterior faces.  For each of the 26
//  k-DOP directions, build the box face winding (Winding_BaseForPlane over the
//  [-r,r]^3 cube), clip it against the 20 diagonal k-DOP planes, then link it as an
//  exterior region face (exterior=1).  a2 = the region descriptor; *(a2+28) = radius.
// =============================================================================
void sub_4DAD20( rface_t **outList, const float *desc )
{
    float radius = *(const float *)( (const char *)desc + 28 );   // a2+28
    float boxMin[3] = { -radius, -radius, -radius };
    float boxMax[3] = {  radius,  radius,  radius };

    for ( int d = 0; d < 26; d++ )
    {
        const float *dir = s_kdopDirs[d];
        float planeDist = -radius;

        plane_t pl;
        pl.normal[0] = dir[0];
        pl.normal[1] = dir[1];
        pl.normal[2] = dir[2];
        pl.type = 0;
        pl.dist = planeDist;
        winding_t *wnd = Winding_BaseForPlane( boxMax, boxMin, &pl );

        // Clip against the 20 diagonal k-DOP planes (flt_6DD258 = s_kdopDirs[6..25]),
        // each at dist -radius.  Disasm: the diagonal index v5 runs 6..25 and is skipped
        // when it equals the current face's k-DOP index v17(=d): if (v5 != v17) clip.
        for ( int k = 0; k < 20; k++ )
        {
            if ( ( 6 + k ) != d )      // skip the plane equal to this face's own
            {
                if ( !wnd )
                    vassert( wnd, "%s", "w" );   // primarylights_region.cpp:305
                Winding_Clip_real_( &wnd, s_kdopDiag[k], -radius, 0.1f );
            }
        }
        if ( !wnd )
            vassert( wnd, "%s", "w" );           // primarylights_region.cpp:308

        rface_t *face = (rface_t *)operator new( sizeof( rface_t ) );
        if ( !face )
            Com_PrintMessage( "Out of memory" );
        face->w        = wnd;          // +9
        face->pad      = 0.0f;         // +10
        face->exterior = 1;            // +8
        face->plane[0] = dir[0];
        face->plane[1] = dir[1];
        face->plane[2] = dir[2];
        face->plane[3] = planeDist;
        face->next     = *outList;     // +11
        *outList       = face;
        Region_FitFaceCone( face );
    }
}

// =============================================================================
//  sub_4DA240 — free a region-face list (free each node's winding @+0x24 and node).
// =============================================================================
static void Region_FreeList( rface_t *list )
{
    while ( list )
    {
        rface_t   *node = list;
        winding_t *w    = list->w;
        list = list->next;
        if ( w )
        {
            --g_windingAlloc;
            free( w );
        }
        free( node );
    }
}

// =============================================================================
//  sub_4DA280 — deep-copy a region-face list (clone each 48-byte node + winding).
//  Returns the head of the copied list.
// =============================================================================
static rface_t *Region_CopyList( rface_t *src )
{
    if ( !src )
        return 0;
    rface_t *head = 0;
    rface_t **tail = &head;
    while ( src )
    {
        rface_t *node = (rface_t *)operator new( sizeof( rface_t ) );
        if ( !node )
            Com_PrintMessage( "Out of memory" );
        ++g_windingAlloc;
        *tail = node;
        memcpy( node, src, sizeof( rface_t ) );
        winding_t *sw = src->w;
        winding_t *nw = (winding_t *)malloc( 12 * sw->numpoints + 4 );
        if ( !nw )
            Com_PrintMessage( "out of memory: winding_t\n" );
        nw->numpoints = 0;
        memcpy( nw, sw, 12 * sw->numpoints + 4 );
        node->w = nw;
        src = src->next;
        tail = &node->next;
    }
    *tail = 0;
    return head;
}

// =============================================================================
//  sub_40A020 — cos(acos(a)+acos(b)) = a*b - sqrt((1-a^2)(1-b^2)).  The cone
//  half-angle sum used to test whether two cones can overlap.
// =============================================================================
static float Region_CosSum( float a1, float a2 )
{
    float v4 = 1.0f - a2 * a2;
    float v5 = 1.0f - a1 * a1;
    float v6 = v4 * v5;
    float v7 = sqrtf( v6 );
    return (float)( a2 * a1 - v7 );
}

// =============================================================================
//  sub_4D7E40 (common\polylib.cpp:~530) — Winding_SplitEpsilon: classify/split a
//  winding by a plane.  Returns 0=front, 1=back, 2=on, 3=split; on split *front /
//  *back receive freshly-allocated windings (caller owns).  a2=dist, a3=epsilon.
// =============================================================================
static int Region_SplitWinding( const winding_t *in, const float *normal, float dist,
                                float epsilon, winding_t **front, winding_t **back )
{
    if ( !in )       vassert( in, "%s", "in" );                                      // :534
    if ( !normal )   vassert( normal, "%s", "normal" );                              // :535
    {
        float lensq = normal[1] * normal[1] + normal[0] * normal[0] + normal[2] * normal[2];
        if ( lensq <= 0.0f )
            vassert( lensq > 0.0f, "%s", "Vec3LengthSq( normal ) > 0" );             // :536
    }
    if ( !front )    vassert( front, "%s", "front" );                                // :537
    if ( !back )     vassert( back, "%s", "back" );                                  // :538

    unsigned int np = in->numpoints;
    float dists[1028];
    int   sides[1029];
    int   counts[3] = { 0, 0, 0 };
    unsigned int i;
    for ( i = 0; i < np; i++ )
    {
        float d = in->p[i][0] * normal[0] + in->p[i][1] * normal[1] + in->p[i][2] * normal[2] - dist;
        dists[i] = d;
        if ( d > epsilon )       sides[i] = 0;
        else if ( d < -epsilon ) sides[i] = 1;
        else                     sides[i] = 2;
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    // disasm: front==0 -> (back!=0 ? 1 : 2); else back==0 -> 0; else split(3).
    if ( counts[0] == 0 )                       // no front
        return 2 - ( counts[1] != 0 ? 1 : 0 );  // all-back(1) or all-on(2)
    if ( counts[1] == 0 )                       // no back (front!=0)
        return 0;                               // all front

    // axial-plane snap test: if the normal is a unit axis, the split point snaps to dist.
    float snapVal = 0.0f;
    int   axialAxis = -1;
    int   exactAxial = 1;
    for ( int j = 0; j < 3; j++ )
    {
        if ( normal[j] == 1.0f )       { axialAxis = j; snapVal = dist; }
        else if ( normal[j] == -1.0f ) { axialAxis = j; snapVal = -dist; }
        else if ( normal[j] != 0.0f )  { exactAxial = 0; break; }
    }
    if ( exactAxial && axialAxis < 0 )
        vassert( !exactAxial || ( axialAxis >= 0 && axialAxis < 3 ),
                 "(axialPlaneAxis) = %i", axialAxis );                               // :590

    unsigned int maxpts = np + 4;
    ++g_windingAlloc;
    winding_t *f = (winding_t *)malloc( 12 * maxpts + 4 );
    if ( !f ) Com_PrintMessage( "out of memory: winding_t\n" );
    f->numpoints = 0;
    ++g_windingAlloc;
    winding_t *b = (winding_t *)malloc( 12 * maxpts + 4 );
    if ( !b ) Com_PrintMessage( "out of memory: winding_t\n" );
    b->numpoints = 0;

    for ( i = 0; i < (unsigned int)in->numpoints; i++ )
    {
        const float *p1 = in->p[i];
        if ( sides[i] == 2 )
        {
            float *of = f->p[f->numpoints++]; of[0]=p1[0]; of[1]=p1[1]; of[2]=p1[2];
            float *ob = b->p[b->numpoints++]; ob[0]=p1[0]; ob[1]=p1[1]; ob[2]=p1[2];
            continue;
        }
        if ( sides[i] == 0 )
        {
            float *of = f->p[f->numpoints++]; of[0]=p1[0]; of[1]=p1[1]; of[2]=p1[2];
        }
        if ( sides[i] == 1 )
        {
            float *ob = b->p[b->numpoints++]; ob[0]=p1[0]; ob[1]=p1[1]; ob[2]=p1[2];
        }
        int sn = sides[i + 1];
        if ( sn == 2 || sn == sides[i] )
            continue;
        const float *p2 = in->p[(i + 1) % in->numpoints];
        float mid[3];
        float frac = dists[i] / ( dists[i] - dists[i + 1] );
        for ( int j = 0; j < 3; j++ )
        {
            mid[j] = ( p2[j] * dists[i] - p1[j] * dists[i + 1] ) / ( dists[i] - dists[i + 1] );
            (void)frac;
            if ( exactAxial && j == axialAxis )
                mid[j] = snapVal;
        }
        float *of = f->p[f->numpoints++]; of[0]=mid[0]; of[1]=mid[1]; of[2]=mid[2];
        float *ob = b->p[b->numpoints++]; ob[0]=mid[0]; ob[1]=mid[1]; ob[2]=mid[2];
    }

    if ( (unsigned int)f->numpoints > maxpts )  vassert( (unsigned int)f->numpoints <= maxpts, "%i, %i", f->numpoints, maxpts );  // :638
    if ( (unsigned int)b->numpoints > maxpts )  vassert( (unsigned int)b->numpoints <= maxpts, "%i, %i", b->numpoints, maxpts );  // :639
    if ( f->numpoints > 0x400 )  vassert( f->numpoints <= 1024, "(f->ptCount) = %i", f->numpoints );  // :640
    if ( b->numpoints > 0x400 )  vassert( b->numpoints <= 1024, "(b->ptCount) = %i", b->numpoints );  // :641

    *back  = b;
    *front = f;
    return 3;
}

// =============================================================================
//  sub_4D9450 (common\polylib.cpp:~1114) — minimum edge clearance of a winding
//  along a reference axis (a2); used to reject near-degenerate split pieces.
// =============================================================================
static float Region_MinEdgeClearance( const winding_t *w, const float *a2 )
{
    if ( !w )          vassert( w, "%s", "w" );                       // :1114
    if ( w->numpoints < 3 ) vassert( w->numpoints >= 3, "%s", "w->ptCount >= 3" );  // :1115

    unsigned int n = w->numpoints;
    float best = 3.4028235e38f;
    unsigned int prev = n - 1;
    for ( unsigned int i = 0; i < n; i++ )
    {
        float edge[3];
        edge[0] = w->p[i][0] - w->p[prev][0];
        edge[1] = w->p[i][1] - w->p[prev][1];
        edge[2] = w->p[i][2] - w->p[prev][2];
        float en[3];
        Vec3Cross( edge, a2, en );
        float edgeLen = Vec3Normalize_R( en );
        float base = w->p[prev][0] * en[0] + w->p[prev][1] * en[1] + w->p[prev][2] * en[2];
        float run = -3.4028235e38f;
        unsigned int j = ( i + 1 ) % n;
        while ( 1 )
        {
            float d = w->p[j][0] * en[0] + w->p[j][1] * en[1] + w->p[j][2] * en[2] - base;
            if ( d < -0.1f && edgeLen >= 1.0f )
                vassert( d >= -0.1f || edgeLen < 1.0f, "(dist) = %g", d );   // :1130
            if ( run > d )
                break;
            run = d;
            j = ( j + 1 ) % n;
            if ( j == prev )
                break;
        }
        if ( best > run )
            best = run;
        prev = i;
    }
    return best;
}

// =============================================================================
//  sub_4DBAC0 — clip every winding in a region face-list by `plane` (back side),
//  dropping faces that clip away entirely.  plane = float[4] {n[3],dist}.
// =============================================================================
static void Region_ClipListByPlane( rface_t **list, const float *plane )
{
    rface_t **pp = list;
    while ( *pp )
    {
        rface_t *node = *pp;
        Winding_Clip_real_( &node->w, plane, plane[3], 0.0099999998f );
        if ( node->w )
        {
            pp = &node->next;
        }
        else
        {
            *pp = node->next;
            if ( node->w )
            {
                --g_windingAlloc;
                free( node->w );
            }
            free( node );
        }
    }
}

// =============================================================================
//  sub_4DB800 — clip a region face-list by ONE cone-edge plane derived from the
//  caster cone (a1=axis, a2=sinHalfFov, a3=cosHalfFov).  Returns the &next slot to
//  advance.  Drops faces fully outside the cone.  (Internal to sub_4DBA60.)
// =============================================================================
static rface_t **Region_ClipListByConeEdge( const float *a1, float a2, float a3, rface_t **slot )
{
    rface_t *node = *slot;
    float axisDot = node->dir[0] * a1[0] + node->dir[1] * a1[1] + node->dir[2] * a1[2];
    if ( Region_CosSum( node->cosHalfFov, a3 ) <= axisDot )
    {
        if ( Region_CosSum( node->cosHalfFov, axisDot ) > a3 )
            return &node->next;

        winding_t *clip = Winding_Clone( node->w );
        if ( node->w->numpoints != 0 )
        {
            int wn = node->w->numpoints;
            for ( int i = 0; i < wn; i++ )
            {
                const float *p = node->w->p[i];
                float t = p[0] * a1[0] + p[1] * a1[1] + p[2] * a1[2];
                float pn = -t;
                float vx = a1[0] * pn + p[0];
                float vy = a1[1] * pn + p[1];
                float vz = pn * a1[2] + p[2];
                float len = sqrtf( vx * vx + vy * vy + vz * vz );
                float k = len * a3 + t * a2;
                if ( k > 0.0f )
                {
                    float invLen = -a3 / len;
                    float n[3];
                    n[0] = a1[0] * ( -a2 ) + invLen * vx;
                    n[1] = a1[1] * ( -a2 ) + vy * invLen;
                    n[2] = invLen * vz + ( -a2 ) * a1[2];
                    Winding_Clip_real_( &clip, n, 0.0f, 0.1f );
                    if ( !clip )
                        break;
                }
            }
        }
        if ( clip )
        {
            --g_windingAlloc;
            free( node->w );
            node->w = clip;
            return &node->next;
        }
        // clip == 0 → drop this node
        *slot = node->next;
    }
    else
    {
        *slot = node->next;
    }
    if ( node->w )
    {
        --g_windingAlloc;
        free( node->w );
    }
    free( node );
    return slot;
}

// =============================================================================
//  sub_4DBA60 — clip a region face-list by the caster cone (axis a1, cos a3).
// =============================================================================
static void Region_ClipListByCone( const float *a1, rface_t **list, float a3 )
{
    float a2 = sqrtf( 1.0f - a3 * a3 );    // sinHalfFov
    rface_t **slot = list;
    while ( *slot )
        slot = Region_ClipListByConeEdge( a1, a2, a3, slot );
}

// lightDesc_t (the descriptor from sub_406CE0) is defined in primarylights_region.h.
static_assert( sizeof(lightDesc_t) == 36, "lightDesc_t (v19+v20[8]) != 36" );

// =============================================================================
//  sub_4DBB40 — clip a region face-list by the LIGHT's cone/plane.  a2 = the light
//  descriptor.  cosHalfFov<0 → clip by a plane; else clip by the cone.
// =============================================================================
static void Region_ClipByCaster( rface_t **list, const lightDesc_t *d )
{
    if ( d->cls != 3 && d->p[7] > -1.0f )
    {
        if ( d->p[7] < 0.0f )
        {
            float pl[4];
            pl[0] = d->p[3];
            pl[1] = d->p[4];
            pl[2] = d->p[5];
            pl[3] = d->p[6] * d->p[7];     // *(a2+28) * *(a2+32)
            Region_ClipListByPlane( list, pl );
        }
        else
        {
            Region_ClipListByCone( &d->p[3], list, d->p[7] );
        }
    }
}

// =============================================================================
//  sub_4DBBB0 — clip a face-list by ALL of a region face's edge planes (the back-
//  facing silhouette planes of the caster winding) plus its own plane.  a1 = caster
//  rface; a2 = the list to clip.
// =============================================================================
static void Region_ClipByCasterEdges( const rface_t *caster, rface_t **list )
{
    float pl[4];
    pl[0] = -caster->plane[0];
    pl[1] = -caster->plane[1];
    pl[2] = -caster->plane[2];
    pl[3] = -caster->plane[3];
    Region_ClipListByPlane( list, pl );

    const winding_t *w = caster->w;
    int n = w->numpoints;
    int prev = 12 * ( n - 1 );   // byte offset of last point (v5)
    for ( int i = 0; i < n; i++ )
    {
        const float *cur  = (const float *)( (const char *)w->p[0] + 12 * i );
        const float *back = (const float *)( (const char *)w->p[0] + prev );
        float en[3];
        Vec3Cross( back, cur, en );
        Vec3Normalize_R( en );
        float d = en[0] * cur[0] + en[1] * cur[1] + en[2] * cur[2];
        float plane[4] = { en[0], en[1], en[2], d };
        Region_ClipListByPlane( list, plane );
        prev = 12 * i;
    }
}

// =============================================================================
//  sub_4DB000 — split ONE region face (*slot) against a caster region face, in the
//  convex-decomposition merge.  caster (ecx) is the other region's face; *slot is the
//  current face being processed.  If the caster's cone can't overlap, returns the
//  &next slot to advance.  Otherwise splits *slot's winding by the caster plane and
//  each caster-winding silhouette edge plane, emits the resulting convex sub-region
//  pieces (prepended via *slot), frees the original node, and returns &original->next.
//  Faithful to disasm 0x4DB000 (the var_8 "have pieces" flag, the var_1038[] piece
//  array, the SIDE_BACK vs SIDE_CROSS split branches, the min-edge-clearance reject).
// =============================================================================
static rface_t **Region_SplitFaceByCaster( rface_t **slot, rface_t *caster )
{
    rface_t *region = *slot;                       // edi

    float dot = caster->dir[0] * region->dir[0]
              + caster->dir[1] * region->dir[1]
              + caster->dir[2] * region->dir[2];   // var_4
    if ( dot < Region_CosSum( caster->cosHalfFov, region->cosHalfFov ) - 0.001000000047497451f )
        return &region->next;                       // cones disjoint → keep, advance

    winding_t *frontW = 0;                           // var_4 (after split)
    winding_t *backW  = 0;                           // var_1C
    int side = Region_SplitWinding( region->w, caster->dir, region->plane[3], 0.0099999998f,
                                    &frontW, &backW );
    if ( side == 0 || side == 2 )
        return &region->next;

    // pieces[] accumulates the back/cross sub-windings (var_1038[]).
    winding_t *pieces[1024];
    int        pieceCount;                            // esi / var_8
    winding_t *carryWinding;                          // var_10

    if ( side == 1 )                                  // SIDE_BACK: whole region behind caster
    {
        carryWinding = region->w;                     // [edi+24]
        pieceCount   = 0;
    }
    else                                              // SIDE_CROSS (3): split produced front+back
    {
        if ( side != 3 )
            vassert( side == 3, "%s", "side == SIDE_CROSS" );   // :412
        pieces[0]    = frontW;                        // var_1038[0] = front piece
        carryWinding = backW;                         // var_10 = back winding
        pieceCount   = 1;                             // esi = 1
    }

    const winding_t *cw = caster->w;                  // var_18 (ebx = caster->w)
    if ( (unsigned int)cw->numpoints > 0x400 )
        vassert( (unsigned int)cw->numpoints <= 0x400, "%i, %i", cw->numpoints, 1024 );  // :419

    // For each caster-winding edge, build the silhouette plane (light at origin →
    // edge × edge×... actually the cross of consecutive caster verts) and split the
    // carry winding; pieces that fall behind become new sub-regions.
    int n = cw->numpoints;
    if ( n != 0 )
    {
        int prev = 12 * ( n - 1 );                    // byte offset of last caster vertex
        for ( int i = 0; i < n; i++ )
        {
            const float *cur  = (const float *)( (const char *)cw->p[0] + 12 * i );
            const float *back = (const float *)( (const char *)cw->p[0] + prev );
            float en[3];
            Vec3Cross( cur, back, en );               // Vec3Cross(curVert, prevVert, en)
            if ( Vec3Normalize_R( en ) != 0.0f )
            {
                winding_t *frontPiece = 0;            // var_C target / var_1038[pieceCount]
                winding_t *backPiece  = 0;            // var_1C
                int s2 = Region_SplitWinding( carryWinding, en, 0.0f, 0.0099999998f,
                                              &frontPiece, &backPiece );
                if ( s2 != 1 )                        // not all-back
                {
                    if ( carryWinding != region->w )
                    {
                        --g_windingAlloc;
                        free( carryWinding );
                    }
                    if ( s2 != 3 )                    // not a cross → caster doesn't fully cover
                    {
                        // free any accumulated pieces and bail (return &region->next)
                        if ( pieceCount )
                        {
                            g_windingAlloc -= pieceCount;
                            while ( pieceCount )
                                free( pieces[--pieceCount] );
                        }
                        return &region->next;
                    }
                    // cross: keep the front piece as a sub-region if it has clearance
                    winding_t *fp = frontPiece;       // *var_C = [front]
                    carryWinding  = backPiece;         // var_10 = back
                    // a2 = `edi` (the region face ptr) → reads region->plane normal.
                    if ( Region_MinEdgeClearance( fp, region->plane ) >= 0.020009998f )
                    {
                        pieces[pieceCount++] = fp;     // ++var_8, advance var_C
                    }
                    else
                    {
                        --g_windingAlloc;
                        free( fp );
                    }
                }
            }
            prev = 12 * i;
        }
    }

    if ( carryWinding != region->w )
    {
        --g_windingAlloc;
        free( carryWinding );
    }

    // Re-link: replace `region` with `pieceCount` new sub-region nodes (cloning
    // region's plane/cone-seed via sub_4DA550), then free `region`.
    *slot = region->next;                              // unlink region (edi+0x2C)
    rface_t **outSlot = slot;
    for ( int p = pieceCount; p > 0; )
    {
        --p;
        rface_t *node = (rface_t *)operator new( sizeof( rface_t ) );
        if ( !node )
            Com_PrintMessage( "Out of memory" );
        node->next     = *outSlot;
        *outSlot       = node;
        node->w        = pieces[p];                    // [ebx+24] = var_1038[esi]
        outSlot        = &node->next;                  // arg_0 = &node->next
        node->pad      = region->pad;                   // [ebx+28] = [edi+28]
        node->plane[0] = region->plane[0];
        node->plane[1] = region->plane[1];
        node->plane[2] = region->plane[2];
        node->plane[3] = region->plane[3];
        Region_FitFaceCone( node );
    }

    if ( region->w )
    {
        --g_windingAlloc;
        free( region->w );
    }
    free( region );
    return outSlot;
}

// =============================================================================
//  sub_4DAEC0 — merge-sort a region face-list by cosHalfFov ascending (so the
//  tightest cones are processed first).  Recursive (split-by-pairs, merge).
//  Returns the &next slot of the last node.  Linked via +0x2C.
// =============================================================================
static rface_t **Region_SortByCone( rface_t **a1 )
{
    rface_t *head = *a1;
    if ( !head )
        return a1;

    // split into two alternating sublists v16 / v17
    rface_t *lists[2] = { 0, 0 };                     // v16, v17
    int      which = 0;
    rface_t *cur = head;
    while ( cur )
    {
        rface_t *nx = cur->next;
        cur->next = lists[which];
        lists[which] = cur;
        which = 1 - which;
        cur = nx;
    }

    // recurse on each sublist if it has >1 element
    if ( lists[0] && lists[0]->next )
    {
        Region_SortByCone( &lists[0] );
        if ( lists[1] && lists[1]->next )
            Region_SortByCone( &lists[1] );
    }

    // merge the two sorted sublists by ascending cosHalfFov (+0x1C)
    rface_t *m0 = lists[0];
    rface_t *m1 = lists[1];
    rface_t **link = a1;
    int sel = ( m1 ? m1->cosHalfFov : 3.4e38f ) <= ( m0 ? m0->cosHalfFov : 3.4e38f ) ? 1 : 0;
    rface_t *src[2] = { m0, m1 };
    while ( 1 )
    {
        rface_t *node = src[sel];
        *link = node;
        link = &node->next;
        src[sel] = node->next;
        if ( !src[sel] )
            break;
        rface_t *other = src[1 - sel];
        if ( other && src[sel]->cosHalfFov > other->cosHalfFov )
            sel = 1 - sel;
    }
    *link = src[1 - sel];
    return link;
}

// =============================================================================
//  sub_4DB3C0 — when a region face-list has >1 face (partial occluder), collapse
//  it to a SINGLE convex occluder winding: project every vertex of every face onto
//  the face's best 2D plane, convex-hull them (in 64-vtx batches via Com_ConvexHull),
//  un-project back to 3D, recompute the plane, drop the extra faces, and (if the new
//  winding faces away) reverse it.  Returns a1 (the now-single-face region head).
//  Faithful to disasm 0x4DB3C0 (Polylib_PickProjectionAxes + 64-batch convex hull).
// =============================================================================
static rface_t *Region_BuildOccluderHull( rface_t *a1 )
{
    if ( !a1 || !a1->next )
        return a1;

    int axis0, axis1;
    Polylib_PickProjectionAxes( a1->plane, &axis0, &axis1 );

    // batch buffers: two 64-vertex 2D point pools that ping-pong (v34/v35 = 512 bytes
    // each, 64 * [2 floats]); v1 selects the active pool, v41 the scratch pool.
    float pool[2][64][2];
    int   activePool = 0;     // v1
    int   scratchPool = 1;    // v41
    unsigned int v2 = 0;      // count in the active batch

    // accumulate the 2D projection of every face vertex, hulling at 64.
    for ( rface_t *f = a1; f; f = f->next )
    {
        const winding_t *w = f->w;
        for ( int i = 0; i < w->numpoints; i++ )
        {
            pool[activePool][v2][0] = w->p[i][axis0];
            pool[activePool][v2][1] = w->p[i][axis1];
            v2++;
            if ( v2 == 64 )
            {
                v2 = Com_ConvexHull( pool[activePool], 64, pool[scratchPool] );
                if ( v2 == 64 )
                    Com_PrintMessage( "Too many vertices on convex hull of partially occluded primary light portal" );
                // ping-pong: the hull is now in scratchPool; make it the active pool.
                int t = activePool; activePool = scratchPool; scratchPool = t;
            }
        }
    }

    unsigned int hullCount = Com_ConvexHull( pool[activePool], v2, pool[scratchPool] );
    const float (*hull)[2] = pool[scratchPool];

    ++g_windingAlloc;
    winding_t *out = (winding_t *)malloc( 12 * hullCount + 4 );
    if ( !out )
        Com_PrintMessage( "out of memory: winding_t\n" );
    out->numpoints = (int)hullCount;

    int axis2 = 3 - axis0 - axis1;
    float planeDist = a1->plane[3];
    for ( unsigned int i = 0; i < hullCount; i++ )
    {
        // un-project: recover the axis2 coordinate from the plane equation.
        float c0 = hull[i][0];   // projected axis0
        float c1 = hull[i][1];   // projected axis1
        out->p[i][axis0] = c0;
        out->p[i][axis1] = c1;
        out->p[i][axis2] = ( planeDist - ( c0 * a1->plane[axis0] + c1 * a1->plane[axis1] ) ) / a1->plane[axis2];
    }

    float plane[4];
    if ( Region_WindingPlane( plane, out ) == 0.0f )
    {
        Region_FreeList( a1 );
        --g_windingAlloc;
        free( out );
        return 0;
    }

    // drop the extra faces (keep only a1), free the old winding, install the hull.
    Region_FreeList( a1->next );
    a1->next = 0;
    --g_windingAlloc;
    free( a1->w );

    float facing = a1->plane[0] * plane[0] + a1->plane[1] * plane[1] + a1->plane[2] * plane[2];
    if ( facing < 0.0f )
    {
        winding_t *rev = Region_ReverseWinding( out );
        --g_windingAlloc;
        a1->w = rev;
        free( out );
        return a1;
    }
    a1->w = out;
    return a1;
}

// =============================================================================
//  sub_4DCFE0 chain (k-DOP hull build) — forward decl; defined in part 7.
// =============================================================================
struct GfxLightRegionHull;   // opaque (matches r_primarylights GfxLightRegionHull)
static void  Region_BuildKDOP( rface_t *regionFaces, const lightDesc_t *desc, void *outHullKdop, rface_t *occluder );
static void *Region_FinalizeHull( const void *kdopScratch );

// helper: decompose a freshly-copied region list against EVERY face of `against`,
// matching the disasm idiom  `for(r=against;r;r=r->next){ s=&list; if(list) do
// s=SplitFaceByCaster(s,r); while(*s); }`.
static void Region_DecomposeAgainst( rface_t **list, rface_t *against )
{
    for ( rface_t *r = against; r; r = r->next )
    {
        rface_t **s = list;
        if ( *list )
            do { s = Region_SplitFaceByCaster( s, r ); } while ( *s );
    }
}

// =============================================================================
//  sub_4DD260 — the merge driver.  Decompose `*a1` (one light's region face-list)
//  into convex sub-regions, build a k-DOP hull for each, append each finished
//  GfxLightRegionHull* to a3[], return the count.  a2 = the light descriptor.
//  Faithful to disasm 0x4DD260 (the SortByCone pre-pass, the copy/decompose/
//  clip-by-light-cone/k-DOP for the whole region, then per-non-exterior-face the
//  occluder-hull sub-regions).
// =============================================================================
int sub_4DD260( rface_t **a1, const lightDesc_t *a2, void **a3 )
{
    if ( *a1 && (*a1)->next )
        Region_SortByCone( a1 );                    // sub_4DAEC0

    // whole-region pass: copy, decompose against itself, clip by the light cone.
    rface_t *copy = Region_CopyList( *a1 );          // v23[0]
    Region_DecomposeAgainst( &copy, *a1 );
    Region_ClipByCaster( &copy, a2 );                // sub_4DBB40

    unsigned char kdop[0x16C];                        // v19 scratch hull (sub_4DCFE0 out)
    int count = 0;                                    // v22
    Region_BuildKDOP( copy, a2, kdop, 0 );           // sub_4DCFE0(.., a2, v19, 0)
    Region_FreeList( copy );                          // sub_4DA240
    if ( *(int *)kdop )
    {
        a3[0] = Region_FinalizeHull( kdop );          // sub_4DD170
        count = 1;
    }

    // per-face occluder pass: pull each non-exterior face out, build its occluder
    // hull from the decomposed region, then k-DOP the region clipped by it.
    rface_t **slot = a1;                              // v20 / v8
    while ( *a1 )
    {
        rface_t *node = *slot;                         // v9
        if ( node->exterior )                          // *(v9+32) != 0 → boundary, keep
        {
            slot = &node->next;
        }
        else
        {
            rface_t *one = node;                        // v21
            *slot = node->next;                         // unlink
            one->next = 0;

            Region_DecomposeAgainst( &one, *a1 );      // decompose v21 against the rest
            Region_ClipByCaster( &one, a2 );           // sub_4DBB40(&v21, a2)
            rface_t *occ = Region_BuildOccluderHull( one );  // sub_4DB3C0(v21)
            if ( occ )
            {
                rface_t *dec = Region_CopyList( *a1 ); // v23[0] = copy of remaining region
                Region_DecomposeAgainst( &dec, *a1 );
                Region_ClipByCasterEdges( occ, &dec ); // sub_4DBBB0(occ, v23)
                unsigned char kdop2[0x16C];
                Region_BuildKDOP( dec, a2, kdop2, occ );    // sub_4DCFE0(.., occ)
                Region_FreeList( dec );

                winding_t *ow = occ->w;                 // free the occluder hull
                if ( ow )
                {
                    --g_windingAlloc;
                    free( ow );
                }
                free( occ );

                if ( *(int *)kdop2 )
                {
                    if ( count == 32 )
                        Com_PrintMessage( "More than %i convex regions affected by primary light at %g %g %g\n",
                                          32, a2->p[1], a2->p[2], a2->p[3] );
                    a3[count] = Region_FinalizeHull( kdop2 );
                    count++;
                }
            }
        }
        if ( !*slot )
            break;
    }
    return count;
}

// =============================================================================
//  K-DOP hull builder (common\primarylights_region.cpp).  Builds a GfxLightRegionHull
//  from a convex region (face list + optional occluder).  An "axis" is 5 floats:
//  {dir.x, dir.y, dir.z, mid, halfExtent}; the candidate-axis array holds up to 1024.
// =============================================================================

// kdopAxis_t — one candidate k-DOP axis (20 bytes, matches the binary's 5-int stride).
struct kdopAxis_t { float dir[3]; float mid; float half; };
static_assert( sizeof(kdopAxis_t) == 20, "kdopAxis_t" );

// the scratch GfxLightRegionHull the builder fills (v19): { int axisCount; kdopAxis_t[] }.
struct kdopScratch_t { int axisCount; kdopAxis_t axes[1024]; };

// =============================================================================
//  sub_4DCA70 — add `dir` to the candidate axis list unless a near-parallel axis is
//  already present (|dot| > 0.98).  Returns the new axis count.
// =============================================================================
static unsigned int Kdop_AddAxis( kdopAxis_t *axes, const float *dir, unsigned int count )
{
    for ( unsigned int i = 0; i < count; i++ )
    {
        float d = axes[i].dir[0] * dir[0] + axes[i].dir[1] * dir[1] + axes[i].dir[2] * dir[2];
        if ( I_fabs( d ) > 0.9800000190734863f )
            return count;                       // already have a near-parallel axis
    }
    bcassert( count, 0x400 );   // HULL_CANDIDATE_DIR_LIMIT (:1019)
    axes[count].dir[0] = dir[0];
    axes[count].dir[1] = dir[1];
    axes[count].dir[2] = dir[2];
    return count + 1;
}

// =============================================================================
//  sub_4DCE20 — seed the 13 base k-DOP axis pairs (s_kdopNormals).  Each pair is
//  antipodal (asserted); only the first of each pair is stored as a candidate axis.
//  Returns 13.
// =============================================================================
static int Kdop_SeedBaseAxes( kdopAxis_t *axes )
{
    const float *v2 = s_kdopNormalsFlat;        // unk_6DD214
    for ( int i = 0; i < 13; i++ )
    {
        // verify the pair (v2, v2+3) is antipodal
        float dot = v2[3] * v2[0] + v2[2] * v2[-1] + v2[4] * v2[1];
        // (the disasm reads v2[-1..4] for the dot of the pair; only an assert)
        if ( dot > -0.9999899864196777f )
            vassert( dot <= -0.99999f,
                     "%s", "Vec3Dot( s_kdopNormals[kdopIter], s_kdopNormals[kdopIter + 1] ) <= -0.99999f" );  // :1101
        axes[i].dir[0] = v2[0];
        axes[i].dir[1] = v2[1];
        axes[i].dir[2] = v2[2];
        v2 += 6;
    }
    return 13;
}

// =============================================================================
//  sub_4DCB10 — add the occluder's plane + per-edge silhouette axes as candidates.
// =============================================================================
static unsigned int Kdop_AddOccluderAxes( kdopAxis_t *axes, unsigned int count, const rface_t *occ )
{
    float n[3] = { -occ->plane[0], -occ->plane[1], -occ->plane[2] };
    unsigned int c = Kdop_AddAxis( axes, n, count );
    const winding_t *w = occ->w;
    int np = w->numpoints;
    int prev = 12 * ( np - 1 );
    for ( int i = 0; i < np; i++ )
    {
        const float *cur  = (const float *)( (const char *)w->p[0] + 12 * i );
        const float *back = (const float *)( (const char *)w->p[0] + prev );
        float en[3];
        Vec3Cross( cur, back, en );
        Vec3Normalize_R( en );
        c = Kdop_AddAxis( axes, en, c );
        prev = 12 * i;
    }
    return c;
}

// =============================================================================
//  sub_4DCBC0 — add cone-derived candidate axes for the region's first face cone:
//  the cone axis itself plus 8 directions sampled around the cone rim.
// =============================================================================
static unsigned int Kdop_AddConeAxes( unsigned int count, kdopAxis_t *axes, const rface_t *face )
{
    if ( face->exterior == 3 )                  // *(a3)==3 (no cone) → nothing to add
        return count;
    const float *axis = face->dir;              // a3+16
    unsigned int c = Kdop_AddAxis( axes, axis, count );
    if ( face->cosHalfFov >= 0.1000000014901161f )
    {
        float cos = face->cosHalfFov;
        float sinv = sqrtf( 1.0f - cos * cos );
        float tanv = cos / sinv;
        float k = cosf( 0.3926990926265717f ) * tanv;   // cos(pi/8) * tan(halfFov)
        float scaled[3] = { -k * axis[0], -k * axis[1], -k * axis[2] };
        float perp[3], side[3];
        PerpendicularVector( axis, perp );
        Vec3Cross( perp, axis, side );
        // rotate the rim sample around the cone axis in 8 steps of pi/4
        float ang0 = -0.39269909f;
        float c0 = cosf( ang0 ), s0 = sinf( ang0 );
        float cur[3];
        cur[0] = perp[0] * s0 + side[0] * c0 + scaled[0];
        cur[1] = side[1] * c0 + scaled[1] + perp[1] * s0;
        cur[2] = s0 * side[2] + c0 * perp[2] + scaled[2];
        for ( int step = 0; step < 8; step++ )
        {
            float prev3[3] = { cur[0], cur[1], cur[2] };
            float ang = ( (float)step + 0.5f ) * 0.7853981852531433f;
            float cc = cosf( ang ), ss = sinf( ang );
            cur[0] = perp[0] * ss + side[0] * cc + scaled[0];
            cur[1] = side[1] * cc + scaled[1] + perp[1] * ss;
            cur[2] = ss * side[2] + cc * perp[2] + scaled[2];
            float rim[3];
            Vec3Cross( cur, prev3, rim );
            Vec3Normalize_R( rim );
            c = Kdop_AddAxis( axes, rim, c );
        }
    }
    return c;
}

// =============================================================================
//  sub_4DCEB0 — collect candidate axes for the region: base 13 + each region face
//  plane + (occluder edges | cone rim).  Returns the candidate count.
// =============================================================================
static unsigned int Kdop_CollectAxes( const rface_t *regionFaces, const rface_t *occ,
                                      const rface_t *coneFace, kdopAxis_t *axes )
{
    unsigned int c = Kdop_SeedBaseAxes( axes );
    for ( const rface_t *f = regionFaces; f; f = f->next )
    {
        float pl[4];
        Region_WindingPlane( pl, f->w );
        if ( pl[3] <= 0.0f )
            c = Kdop_AddAxis( axes, pl, c );
    }
    if ( occ )
        return Kdop_AddOccluderAxes( axes, c, occ );
    return Kdop_AddConeAxes( c, axes, coneFace );
}

// =============================================================================
//  sub_4DBC70 — compute the k-DOP slab [min,max] of (occluder + region faces) along
//  `axis`.  Returns the last region face processed.
// =============================================================================
static void Kdop_AxisExtent( const rface_t *occ, const float *axis, const rface_t *regionFaces,
                             float *outMin, float *outMax )
{
    float lo, hi;
    if ( occ )
    {
        const winding_t *w = occ->w;
        lo = hi = w->p[0][0] * axis[0] + w->p[0][1] * axis[1] + w->p[0][2] * axis[2];
        for ( int i = 1; i < w->numpoints; i++ )
        {
            float d = w->p[i][0] * axis[0] + w->p[i][1] * axis[1] + w->p[i][2] * axis[2];
            if ( d < lo ) lo = d;
            if ( hi < d ) hi = d;
        }
    }
    else
    {
        lo = 0.0f;
        hi = 0.0f;
    }
    for ( const rface_t *f = regionFaces; f; f = f->next )
    {
        const winding_t *w = f->w;
        for ( int i = 0; i < w->numpoints; i++ )
        {
            float d = w->p[i][0] * axis[0] + w->p[i][1] * axis[1] + w->p[i][2] * axis[2];
            if ( d < lo ) lo = d;
            if ( hi < d ) hi = d;
        }
    }
    *outMin = lo;
    *outMax = hi;
}

// =============================================================================
//  sub_4DC010 — for a triple of candidate axes (i,j,k) compute the box defined by
//  their slabs around the test axis `axis` (a5).  Writes 8 corner points (24 floats)
//  into out and returns 1 if the three axes are non-degenerate (|det| >= 0.001).
// =============================================================================
static int Kdop_CornerBox( const kdopAxis_t *axes, int ia, int ib, int axisIdx, float *out, int ic )
{
    // Re-diffed byte-exact against sub_4DC010 (2026-07-01). The binary uses three axes:
    //   P (v11) = axes[a5] = the INNER-loop axis i  (port: axes[ia])
    //   Q (v7)  = axes[a3] = the MIDDLE-loop axis j (port: axes[ib])
    //   R (v10) = axes[a2] = the OUTER-loop axis k  (port: axes[axisIdx]/[ic])
    // The determinant is the scalar triple product P·(Q×R); the corner points are the
    // inverse-matrix rows dotted with the slab-distance corners (Q,P,R each lo/hi).  The
    // prior port had the det/cofactor layout wrong (it did NOT equal P·(Q×R)); fixed here.
    const kdopAxis_t *P = &axes[ia];        // v11 (a5, inner i)
    const kdopAxis_t *Q = &axes[ib];        // v7  (a3, middle j)
    const kdopAxis_t *R = &axes[axisIdx];   // v10 (a2, outer k)
    (void)ic;                               // caller passes k again; unused (a5 is ia)

    float det = ( Q->dir[1] * R->dir[2] - Q->dir[2] * R->dir[1] ) * P->dir[0]
              + ( P->dir[2] * R->dir[1] - P->dir[1] * R->dir[2] ) * Q->dir[0]
              + ( P->dir[1] * Q->dir[2] - P->dir[2] * Q->dir[1] ) * R->dir[0];
    if ( I_fabs( det ) < 0.001000000047497451f )
        return 0;
    float inv = 1.0f / det;

    // inverse-matrix rows (Cramer), transcribed op-for-op from 0x4dc093-0x4dc113.
    float v15 = ( Q->dir[1] * R->dir[2] - Q->dir[2] * R->dir[1] ) * inv;
    float v16 = ( P->dir[2] * R->dir[1] - P->dir[1] * R->dir[2] ) * inv;
    float v17 = ( P->dir[1] * Q->dir[2] - P->dir[2] * Q->dir[1] ) * inv;
    float v18 = ( R->dir[0] * Q->dir[2] - Q->dir[0] * R->dir[2] ) * inv;
    float v20 = ( P->dir[0] * R->dir[2] - R->dir[0] * P->dir[2] ) * inv;
    float v21 = ( P->dir[2] * Q->dir[0] - Q->dir[2] * P->dir[0] ) * inv;
    float v22 = ( Q->dir[0] * R->dir[1] - R->dir[0] * Q->dir[1] ) * inv;
    float v23 = ( P->dir[1] * R->dir[0] - P->dir[0] * R->dir[1] ) * inv;
    float v19 = inv * ( Q->dir[1] * P->dir[0] - P->dir[1] * Q->dir[0] );

    // slab distances: lo = mid - half, hi = mid + half (per axis).
    float Plo = P->mid - P->half, Phi = P->mid + P->half;   // v25 / v27
    float Qlo = Q->mid - Q->half, Qhi = Q->mid + Q->half;   // v24 / v26
    float Rlo = R->mid - R->half, Rhi = R->mid + R->half;   // v28 / v29

    // 8 corners, ordered R(lo,hi) outer, Q(lo,hi) middle, P(lo,hi) inner — exactly the
    // a4[0..23] store order of the binary.  Each: X=Q*v16+P*v15+R*v17, Y=Q*v20+P*v18+R*v21,
    // Z=Q*v23+P*v22+R*v19.
    const float Qd[2] = { Qlo, Qhi };
    const float Pd[2] = { Plo, Phi };
    const float Rd[2] = { Rlo, Rhi };
    int n = 0;
    for ( int rr = 0; rr < 2; ++rr )
        for ( int qq = 0; qq < 2; ++qq )
            for ( int pp = 0; pp < 2; ++pp )
            {
                float q = Qd[qq], p = Pd[pp], r = Rd[rr];
                out[3*n+0] = q * v16 + p * v15 + r * v17;
                out[3*n+1] = q * v20 + p * v18 + r * v21;
                out[3*n+2] = q * v23 + p * v22 + r * v19;
                ++n;
            }
    return 1;
}

// =============================================================================
//  sub_4DC400 — true if `pt` is inside ALL `count` candidate slabs (|pt·dir - mid|
//  <= half + 0.001 for each axis).
// =============================================================================
static char Kdop_PointInside( const kdopAxis_t *axes, const float *pt, unsigned int count )
{
    for ( unsigned int i = 0; i < count; i++ )
    {
        float d = axes[i].dir[0] * pt[0] + axes[i].dir[1] * pt[1] + axes[i].dir[2] * pt[2];
        if ( I_fabs( d - axes[i].mid ) - axes[i].half > 0.001f )
            return 0;
    }
    return 1;
}

// =============================================================================
//  sub_4DC480 — enumerate every k-DOP vertex (intersection of 3 slabs that lies
//  inside all slabs) into hullPoints[] (24-byte records: {xyz, flagA, flagB, flagC}).
//  Returns the point count.
// =============================================================================
struct kdopPoint_t { float xyz[3]; int fa; int fb; int fc; };
static_assert( sizeof(kdopPoint_t) == 24, "kdopPoint_t" );

static unsigned int Kdop_EnumPoints( const kdopAxis_t *axes, unsigned int count, kdopPoint_t *out )
{
    unsigned int total = 0;
    for ( unsigned int k = 2; k < count; k++ )
        for ( unsigned int j = 1; j < k; j++ )
            for ( unsigned int i = 0; i < j; i++ )
            {
                float box[24];
                if ( !Kdop_CornerBox( axes, i, j, k, box, k ) )
                    continue;
                for ( int corner = 0; corner < 8; corner++ )
                {
                    const float *p = box + 3 * corner;
                    if ( Kdop_PointInside( axes, p, count ) )
                    {
                        bcassert( total, 0x1540 );   // HULL_POINT_LIMIT_TOTAL (:860)
                        out[total].xyz[0] = p[0];
                        out[total].xyz[1] = p[1];
                        out[total].xyz[2] = p[2];
                        out[total].fa = (corner & 1)        + 2 * (int)i;
                        out[total].fb = ((corner >> 1) & 1) + 2 * (int)j;
                        out[total].fc = ((corner >> 2) & 1) + 2 * (int)k;
                        total++;
                    }
                }
            }
    return total;
}

// =============================================================================
//  sub_4DC5D0 — collect all hull points touching candidate axis `axisVal` into a
//  per-face point list (12-byte xyz records).  Returns the count.
// =============================================================================
static unsigned int Kdop_FacePoints( unsigned int count, const kdopPoint_t *pts, vec3_t *out, int axisVal )
{
    unsigned int n = 0;
    for ( unsigned int i = 0; i < count; i++ )
    {
        if ( pts[i].fa == axisVal || pts[i].fb == axisVal || pts[i].fc == axisVal )
        {
            bcassert( n, 0x88 );   // HULL_POINT_LIMIT_FACE (:887)
            out[n][0] = pts[i].xyz[0];
            out[n][1] = pts[i].xyz[1];
            out[n][2] = pts[i].xyz[2];
            n++;
        }
    }
    return n;
}

// =============================================================================
//  sub_4DC650 — build a convex face winding from `count` 3D points lying on a face
//  plane, recursively hull-merging in 64-point batches.  a1 = the face normal.
// =============================================================================
static winding_t *Kdop_FaceWinding( const float *normal, vec3_t *pts, unsigned int count )
{
    if ( count > 0x40 )
    {
        unsigned int half = count >> 1;
        unsigned int rest = count - half;
        winding_t *w0 = Kdop_FaceWinding( normal, pts, half );
        winding_t *w1 = Kdop_FaceWinding( normal, pts + half, rest );
        if ( !w0 ) return w1;
        if ( !w1 ) return w0;
        if ( (unsigned int)w0->numpoints > half )
            vassert( (unsigned int)w0->numpoints <= half, "%i, %i", w0->numpoints, half );  // :913
        if ( (unsigned int)w1->numpoints > rest )
            vassert( (unsigned int)w1->numpoints <= rest, "%i, %i", w1->numpoints, rest );  // :914
        unsigned int merged = w0->numpoints + w1->numpoints;
        if ( merged > 0x40 )
            vassert( merged <= 0x40, "%i, %i", merged, 64 );                                 // :915
        memcpy( pts, w0->p, 12 * w0->numpoints );
        memcpy( (char *)pts + 12 * w0->numpoints, w1->p, 12 * w1->numpoints );
        count = w0->numpoints + w1->numpoints;
        free( w0 );
        g_windingAlloc -= 2;
        free( w1 );
    }
    return Polylib_HullForPoints( pts, normal, (int)count );
}

// =============================================================================
//  sub_4DC7A0 — build the per-face windings of the k-DOP from its candidate axes,
//  writing each non-empty face winding* into out[].  Returns the face count.
// =============================================================================
static int Kdop_BuildFaces( unsigned int axisCount, const kdopAxis_t *axes, winding_t **out )
{
    static kdopPoint_t hullPts[5440];   // v16 (130560 bytes)
    unsigned int total = Kdop_EnumPoints( axes, axisCount, hullPts );
    unsigned int faces = 2 * axisCount;
    int outCount = 0;
    for ( unsigned int f = 0; f < faces; f++ )
    {
        static vec3_t facePts[136];     // v17 (1636 bytes ~ 136 vec3)
        unsigned int n = Kdop_FacePoints( total, hullPts, facePts, (int)f );
        if ( n >= 3 )
        {
            unsigned int ax = f >> 1;
            float nrm[3];
            if ( f & 1 )
            {
                nrm[0] = -axes[ax].dir[0];
                nrm[1] = -axes[ax].dir[1];
                nrm[2] = -axes[ax].dir[2];
            }
            else
            {
                nrm[0] = axes[ax].dir[0];
                nrm[1] = axes[ax].dir[1];
                nrm[2] = axes[ax].dir[2];
            }
            winding_t *w = Kdop_FaceWinding( nrm, facePts, n );
            out[outCount] = w;
            if ( w )
                outCount++;
        }
    }
    return outCount;
}

// =============================================================================
//  sub_4DC870 — signed volume of a k-DOP (sum of plane·area over its faces / 3).
// =============================================================================
static float Kdop_Volume( winding_t **faces, unsigned int faceCount )
{
    float cx = 0.0f, cy = 0.0f, cz = 0.0f;
    int totalPts = 0;
    for ( unsigned int i = 0; i < faceCount; i++ )
    {
        const winding_t *w = faces[i];
        totalPts += w->numpoints;
        for ( int j = 0; j < w->numpoints; j++ )
        {
            cx += w->p[j][0];
            cy += w->p[j][1];
            cz += w->p[j][2];
        }
    }
    float inv = 1.0f / (float)totalPts;
    cx *= inv; cy *= inv; cz *= inv;
    float vol = 0.0f;
    for ( unsigned int i = 0; i < faceCount; i++ )
    {
        float pl[4];
        float area = Region_WindingPlane( pl, faces[i] );
        float d = pl[0] * cx + pl[1] * cy + pl[2] * cz - pl[3];
        vol += d * area;
    }
    return vol / 3.0f;
}

// =============================================================================
//  sub_4DCA10 — volume of the k-DOP described by the first `count` candidate axes.
// =============================================================================
static float Kdop_VolumeForAxes( unsigned int axisCount, const kdopAxis_t *axes )
{
    static winding_t *faces[35];   // v5[35]
    int fc = Kdop_BuildFaces( axisCount, axes, faces );
    float v = Kdop_Volume( faces, fc );
    if ( fc )
    {
        g_windingAlloc -= fc;
        for ( int i = 0; i < fc; i++ )
            free( faces[i] );
    }
    return v;
}

// =============================================================================
//  sub_4DCF40 — pick the candidate axis (from a remaining pool) whose addition most
//  reduces the running k-DOP volume.  a1 = the growing hull {axisCount, axes}, a2 =
//  the candidate pool, a3 = pool size.  Returns the chosen pool index.
// =============================================================================
static unsigned int Kdop_PickBestAxis( kdopScratch_t *hull, const kdopAxis_t *pool, unsigned int poolSize )
{
    unsigned int best = poolSize;
    float bestVol = Kdop_VolumeForAxes( hull->axisCount, hull->axes ) * 0.9990000128746033f;
    if ( !poolSize )
        return 0;
    for ( unsigned int i = 0; i < poolSize; i++ )
    {
        hull->axes[hull->axisCount] = pool[i];     // try adding pool[i]
        float v = Kdop_VolumeForAxes( hull->axisCount + 1, hull->axes );
        if ( v < bestVol )
        {
            bestVol = v;
            best = i;
        }
    }
    return best;
}

// =============================================================================
//  sub_4DCFE0 (Region_BuildKDOP) — build the k-DOP scratch hull for a convex region.
//  Collect candidate axes, compute each axis slab, install the 13 base axes, then
//  greedily add the best extra axes (up to 17 total) by volume reduction.
// =============================================================================
static void Region_BuildKDOP( rface_t *regionFaces, const lightDesc_t *desc, void *outHull, rface_t *occ )
{
    kdopScratch_t *hull = (kdopScratch_t *)outHull;
    (void)desc;   // the descriptor reaches the axis collector only via the cone face below

    if ( !regionFaces )
    {
        hull->axisCount = 0;
        return;
    }

    static kdopAxis_t cand[1024];   // v21 (the candidate axis pool)
    // cone face for Kdop_AddConeAxes: the binary passes the descriptor a3; we reuse
    // the region's first face's cone where there is no occluder.  (Disasm: sub_4DCBC0
    // reads a3+0/+16/+32 = the LIGHT descriptor's class/dir/cosHalfFov.)
    rface_t coneSeed;
    coneSeed.exterior   = desc->cls;
    coneSeed.dir[0]     = desc->p[3];
    coneSeed.dir[1]     = desc->p[4];
    coneSeed.dir[2]     = desc->p[5];
    coneSeed.cosHalfFov = desc->p[7];
    unsigned int axisCount = Kdop_CollectAxes( regionFaces, occ, &coneSeed, cand );

    if ( axisCount < 9 )
        vassert( axisCount >= 9, "%s", "axisCount >= HULL_KDOP_AXIS_COUNT" );    // :1179

    // compute each candidate's [min,max] slab → mid + halfExtent.
    for ( unsigned int i = 0; i < axisCount; i++ )
    {
        float mn, mx;
        Kdop_AxisExtent( occ, cand[i].dir, regionFaces, &mn, &mx );
        cand[i].mid  = ( mn + mx ) * 0.5f;
        cand[i].half = ( mx - mn ) * 0.5f;
    }

    // install the 9 base axes (0xB4 bytes = 9 * 20).
    memcpy( hull->axes, cand, 0xB4u );
    hull->axisCount = 9;

    unsigned int target = ( axisCount < 17 ) ? axisCount : 17;
    if ( target == 9 )
        return;

    // remaining pool = cand[9..axisCount); greedily add the best until target reached
    // or no axis helps.  (The disasm packs the pool in-place; we shift the chosen one
    // to the hull and remove it from the pool.)
    unsigned int poolSize = axisCount - 9;
    static kdopAxis_t pool[1024];
    memcpy( pool, &cand[9], poolSize * sizeof( kdopAxis_t ) );
    while ( poolSize && hull->axisCount != target )
    {
        unsigned int pick = Kdop_PickBestAxis( hull, pool, poolSize );
        if ( pick == poolSize )
            break;                              // none reduced the volume
        hull->axes[hull->axisCount++] = pool[pick];
        pool[pick] = pool[--poolSize];          // remove from pool
    }
}

// =============================================================================
//  sub_4DD170 (Region_FinalizeHull) — pack the scratch k-DOP into the final
//  GfxLightRegionHull: 3 axial slabs (kept as-is) + (axisCount-3) diagonal slabs
//  (mid/half divided by 1/√2 = scaled by √2).  Layout matches r_primarylights'
//  GfxLightRegionHull / R_CullBoxFromLightRegionHull.
// =============================================================================
static void *Region_FinalizeHull( const void *scratch )
{
    const kdopScratch_t *s = (const kdopScratch_t *)scratch;
    int axisCount = s->axisCount;
    // out size = 20*axisCount - 104 (the binary's malloc(20*N - 104)).
    float *out = (float *)malloc( 20 * axisCount - 104 );
    if ( !out )
        Com_PrintMessage( "Out of memory" );

    const kdopAxis_t *a = s->axes;
    const float inv = 0.7071067690849304f;
    // mid[0..8] at out[0..8], half[0..8] at out[9..17] (the K_DOP_AXIS_COUNT=9 base).
    out[0]  = a[0].mid;   out[9]  = a[0].half;        // axis 0 (axial)
    out[1]  = a[1].mid;   out[10] = a[1].half;        // axis 1 (axial)
    out[2]  = a[2].mid;   out[11] = a[2].half;        // axis 2 (axial)
    out[3]  = a[3].mid / inv;  out[12] = a[3].half / inv;   // diagonal axes scaled by √2
    out[4]  = a[4].mid / inv;  out[13] = a[4].half / inv;
    out[5]  = a[5].mid / inv;  out[14] = a[5].half / inv;
    out[6]  = a[6].mid / inv;  out[15] = a[6].half / inv;
    for ( int i = 7; i < 9; i++ )
    {
        out[i]     = a[i].mid  / inv;
        out[i + 9] = a[i].half / inv;
    }
    int extra = axisCount - 9;
    *(int *)( out + 18 ) = extra;
    // copy the extra axes verbatim (20 bytes each) after the 18-float base + count.
    memcpy( out + 19, &s->axes[9], 20 * extra );
    return out;
}
