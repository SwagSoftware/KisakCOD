#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\winding.cpp
// Plane/winding primitives.  Named in the binary (all __usercall -> cdecl here):
//   Plane_Equal 0x462470, Plane_FromPoints 0x462550, Point_Equal 0x4625e0,
//   Winding_PlanesConcave 0x4627a0, Winding_Clip 0x462860 (CoD's name for
//   Winding_SplitEpsilon), Winding_FindSharedEdge 0x462c50, Winding_TryMerge 0x462d40,
//   Winding_Alloc 0x4d6d10, Winding_BaseForPlane 0x4d7bc0 (+ its common/polylib.cpp
//   helper chain sub_4A4650 / sub_4D99D0 / sub_4D7670 / sub_4D7AB0), Winding_Clone
//   0x4d7d60.
// The rest (Winding_Free/Reverse/RemovePoint/InsertPoint/IsTiny/IsHuge/Plane/Area/
// Bounds/PointInside/VectorIntersect) are not named symbols in the binary — they are
// inlined at their call sites; the bodies come from the GtkRadiant 1.6 reference.
// Not in this TU: BaseWindingForPlane 0x4d7380 (unused, no xrefs), CM_ReverseWinding
// 0x4d7dc0 (qcommon), the destructive in-place clip variant 0x4d83b0.

#include "stdafx.h"
#include "winding.h"
#include <universal/com_math.h>        // IntersectPlanes, PointInBounds (engine reuse)
#include <universal/com_convexhull.h>  // Com_ConvexHullIndices (engine reuse)

#include <universal/assertive.h>

// Live-winding malloc/free counter.  IDB dword_24CE4FC (0x24CE4FC); the binary
// increments it in every winding allocator, including the ones it inlines.
int g_windingAlloc = 0;

// Epsilon constants.  NORMAL/DIST are stored in the binary as the FLOAT values widened
// (dbl_6F4490 == (double)0.0001f, dbl_6F4488 == (double)0.02f), so an `f` literal is
// exact for them.  CONTINUOUS (dbl_6F45C8) and WCONVEX (dbl_6F42F8) are TRUE doubles —
// 0.005f and 0.2f are NOT equal to them, so those two comparisons must stay in double
// (the binary compares the float dot against the double constant on the x87 stack).
#define NORMAL_EPSILON     0.0001f
#define DIST_EPSILON       0.02f
#define CONTINUOUS_EPSILON 0.005          // dbl_6F45C8 — exact double, NOT 0.005f
#define WCONVEX_EPSILON    0.2            // dbl_6F42F8 — exact double, NOT 0.2f
#define EDGE_LENGTH        0.2f
#define MAX_POINTS_ON_WINDING 1024

// BOGUS_RANGE = g_MaxWorldCoord + 1; used only by Winding_IsHuge.
#define BOGUS_RANGE_WINDING  65537.0f

// com_math.h blocks VectorCopy/VectorMA/VectorNormalize (error typedefs).
#define W_VectorCopy(src, dst)  do { (dst)[0]=(src)[0]; (dst)[1]=(src)[1]; (dst)[2]=(src)[2]; } while(0)
#define W_DotProduct(a, b)      ( (a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2] )

// =============================================================================
// 0x462470 Plane_Equal(a@<eax>, b@<ecx>, flip).  flip negates the FIRST arg, not the
// second (the opposite of GtkRadiant).  plane.dist lives in a DOUBLE slot at +0x10 but
// only ever holds float-precision values, so reading it as float is exact.
// =============================================================================
bool Plane_Equal( plane_t *a, plane_t *b, int flip )
{
    float normal[3];
    float dist;

    if ( flip )
    {
        normal[0] = -a->normal[0];
        normal[1] = -a->normal[1];
        normal[2] = -a->normal[2];
        dist      = -a->dist;
    }
    else
    {
        normal[0] = a->normal[0];
        normal[1] = a->normal[1];
        normal[2] = a->normal[2];
        dist      = a->dist;
    }

    if ( fabsf( b->normal[0] - normal[0] ) < NORMAL_EPSILON &&
         fabsf( b->normal[1] - normal[1] ) < NORMAL_EPSILON &&
         fabsf( b->normal[2] - normal[2] ) < NORMAL_EPSILON &&
         fabsf( b->dist - dist ) < DIST_EPSILON )
    {
        return true;
    }
    return false;
}

// =============================================================================
// 0x462550 Plane_FromPoints(p1@<eax>, p2@<ecx>, plane@<edi>, p3@<esi>).  normal =
// cross(p2-p3, p1-p3); returns false when |normal| < 0.1 (colinear).  plane->dist is
// computed as a float and merely widened into the double slot at +0x10.
// =============================================================================
int Plane_FromPoints( vec3_t p1, vec3_t p2, vec3_t p3, plane_t *plane )
{
    vec3_t v1, v2;

    // v1 = p2 - p3,  v2 = p1 - p3
    v1[0] = p2[0] - p3[0];
    v1[1] = p2[1] - p3[1];
    v1[2] = p2[2] - p3[2];
    v2[0] = p1[0] - p3[0];
    v2[1] = p1[1] - p3[1];
    v2[2] = p1[2] - p3[2];

    Vec3Cross( v1, v2, plane->normal );
    if ( Vec3Normalize( plane->normal ) < 0.1f )
    {
        return 0;
    }
    // plane->dist = dot(p3, normal)
    plane->dist = p3[0] * plane->normal[0]
                + p3[1] * plane->normal[1]
                + p3[2] * plane->normal[2];
    return 1;
}

// =============================================================================
// 0x4625e0 Point_Equal(p1@<eax>, p2@<edx>, epsilon).  Rejects on |p1[i]-p2[i]| >
// epsilon (strictly greater — an exactly-equal difference still passes).
// =============================================================================
int Point_Equal( vec3_t p1, vec3_t p2, float epsilon )
{
    int i;

    for ( i = 0; i < 3; i++ )
    {
        if ( fabsf( p1[i] - p2[i] ) > epsilon )
        {
            return 0;
        }
    }
    return 1;
}

// =============================================================================
// 0x4d6d10 Winding_Alloc(points@<eax>).  4 + 12*N bytes; winding_t::p[4] is a declared
// minimum, the allocation is sized for N.  The binary writes numpoints AFTER the
// out-of-memory message, i.e. it derefs NULL on a failed malloc — reproduced.
// =============================================================================
winding_t *Winding_Alloc( int points )
{
    winding_t *w;

    ++g_windingAlloc;
    w = (winding_t *)malloc( 12 * points + 4 );
    if ( !w )
    {
        Com_PrintMessage( "out of memory: winding_t\n" );
    }
    w->numpoints = 0;
    return w;
}

// =============================================================================
// Winding_Free — not a named symbol (the binary inlines --g_windingAlloc; free(w)).
// Exposed as a function so editor callers have a uniform API.
// =============================================================================
void Winding_Free( winding_t *w )
{
    --g_windingAlloc;
    free( w );
}

// =============================================================================
// 0x4d7d60 Winding_Clone(w@<edi>).  Duplicates numpoints + all point data.
// =============================================================================
winding_t *Winding_Clone( winding_t *w )
{
    int        size;
    winding_t *c;

    size = 12 * w->numpoints + 4;
    ++g_windingAlloc;
    c = (winding_t *)malloc( size );
    if ( !c )
    {
        Com_PrintMessage( "out of memory: winding_t\n" );
    }
    c->numpoints = 0;
    memcpy( c, w, size );
    return c;
}

// =============================================================================
// Winding_Reverse — GtkRadiant reference; new winding, points in reverse order.
// =============================================================================
winding_t *Winding_Reverse( winding_t *w )
{
    int        i;
    winding_t *c;

    c = Winding_Alloc( w->numpoints );
    for ( i = 0; i < w->numpoints; i++ )
    {
        W_VectorCopy( w->p[w->numpoints - 1 - i], c->p[i] );
    }
    c->numpoints = w->numpoints;
    return c;
}

// =============================================================================
// Winding_RemovePoint — GtkRadiant reference; removes point 'point' in place.
// =============================================================================
void Winding_RemovePoint( winding_t *w, int point )
{
    if ( point < 0 || point >= w->numpoints )
    {
        Com_Error( ERR_FATAL, "Winding_RemovePoint: point out of range" );
    }
    if ( point < w->numpoints - 1 )
    {
        memmove( &w->p[point][0], &w->p[point + 1][0],
                 (size_t)( w->numpoints - point - 1 ) * 3 * sizeof( float ) );
    }
    w->numpoints--;
}

// =============================================================================
// Winding_InsertPoint — GtkRadiant reference; inserts at 'spot', returns a new winding.
// =============================================================================
winding_t *Winding_InsertPoint( winding_t *w, vec3_t point, int spot )
{
    int        i, j;
    winding_t *neww;

    if ( spot > w->numpoints )
    {
        Com_Error( ERR_FATAL, "Winding_InsertPoint: spot > w->numpoints" );
    }
    if ( spot < 0 )
    {
        Com_Error( ERR_FATAL, "Winding_InsertPoint: spot < 0" );
    }
    neww            = Winding_Alloc( w->numpoints + 1 );
    neww->numpoints = w->numpoints + 1;
    for ( i = 0, j = 0; i < neww->numpoints; i++ )
    {
        if ( i == spot )
        {
            W_VectorCopy( point, neww->p[i] );
        }
        else
        {
            W_VectorCopy( w->p[j], neww->p[i] );
            j++;
        }
    }
    return neww;
}

// =============================================================================
// 0x4627a0 Winding_PlanesConcave(w1@<eax>, w2@<ebx>, normal1@<edi>, normal2, dist1,
// dist2).  True if any point of one winding is more than WCONVEX_EPSILON in front of
// the OTHER plane.  Loop 1 pairs normal1 with dist2, loop 2 normal2 with dist1 — the
// call sites (Brush_Convex, CSG_Merge) rely on that pairing.
// =============================================================================
int Winding_PlanesConcave( winding_t *w1, winding_t *w2,
                            vec3_t normal1, vec3_t normal2,
                            float dist1, float dist2 )
{
    int   i;
    float dot;

    if ( !w1 || !w2 )
    {
        return 0;
    }

    for ( i = 0; i < w1->numpoints; i++ )
    {
        dot = W_DotProduct( normal1, w1->p[i] );
        if ( (double)dot - (double)dist2 > WCONVEX_EPSILON )
        {
            return 1;
        }
    }
    for ( i = 0; i < w2->numpoints; i++ )
    {
        dot = W_DotProduct( normal2, w2->p[i] );
        if ( (double)dot - (double)dist1 > WCONVEX_EPSILON )
        {
            return 1;
        }
    }
    return 0;
}

// =============================================================================
// 0x462860 Winding_Clip(in, normal, dist, epsilon, front, back).  Despite the name this
// is SplitEpsilon semantics: `in` is NOT freed and BOTH pieces are allocated and
// returned.  Side codes (IDA's, the inverse of GtkRadiant's SIDE_* order):
//   0 = all front, 1 = all back, 2 = all on-plane (coplanar), 3 = split.
// CSG_FaceVisible gates its coplanar test on == 2; other callers ignore the return.
// The all-front/all-back arms clone `in` (the binary inlines the clone in the first
// arm and calls Winding_Clone in the second).
// =============================================================================
int Winding_Clip( winding_t *in, vec3_t normal, double dist, float epsilon,
                  winding_t **front, winding_t **back )
{
    float      dists[MAX_POINTS_ON_WINDING + 4];
    int        sides[MAX_POINTS_ON_WINDING + 4];
    int        counts[3];
    float      dot;
    int        i, j;
    float     *p1, *p2;
    float      mid[3];
    winding_t *f, *b;
    int        maxpts;
    int        n;

    counts[0] = counts[1] = counts[2] = 0;
    n = in->numpoints;

    for ( i = 0; i < n; i++ )
    {
        dot = (float)( in->p[i][0] * normal[0]
                     + in->p[i][1] * normal[1]
                     + in->p[i][2] * normal[2] - dist );
        dists[i] = dot;
        if ( dot > epsilon )
        {
            sides[i] = 0;   // FRONT
        }
        else if ( dot < -epsilon )
        {
            sides[i] = 1;   // BACK
        }
        else
        {
            sides[i] = 2;   // ON
        }
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    *front = *back = NULL;

    if ( !counts[0] )
    {
        // 0x4629ac: return 2 - (backCount != 0).
        *back = Winding_Clone( in );
        return counts[1] ? 1 : 2;
    }
    if ( !counts[1] )
    {
        // No BACK points — all front/on (0x4629c2).
        *front = Winding_Clone( in );
        return 0;
    }

    maxpts = n + 4;

    ++g_windingAlloc;
    f = (winding_t *)malloc( 12 * maxpts + 4 );
    if ( !f ) { Com_PrintMessage( "out of memory: winding_t\n" ); }
    *front       = f;
    f->numpoints = 0;

    ++g_windingAlloc;
    b = (winding_t *)malloc( 12 * maxpts + 4 );
    if ( !b ) { Com_PrintMessage( "out of memory: winding_t\n" ); }
    *back        = b;
    b->numpoints = 0;

    for ( i = 0; i < n; i++ )
    {
        p1 = in->p[i];

        if ( sides[i] == 2 )    // ON: add to both
        {
            W_VectorCopy( p1, f->p[f->numpoints] );
            f->numpoints++;
            W_VectorCopy( p1, b->p[b->numpoints] );
            b->numpoints++;
            continue;
        }

        if ( sides[i] == 0 )    // FRONT
        {
            W_VectorCopy( p1, f->p[f->numpoints] );
            f->numpoints++;
        }
        if ( sides[i] == 1 )    // BACK
        {
            W_VectorCopy( p1, b->p[b->numpoints] );
            b->numpoints++;
        }

        if ( sides[i + 1] == 2 || sides[i + 1] == sides[i] )
        {
            continue;
        }

        // Split point.  An axial normal component takes +/-dist verbatim (0x462b26).
        p2  = in->p[( i + 1 ) % n];
        dot = dists[i] / ( dists[i] - dists[i + 1] );
        for ( j = 0; j < 3; j++ )
        {
            if ( normal[j] == 1.0f )
            {
                mid[j] = (float)dist;
            }
            else if ( normal[j] == -1.0f )
            {
                mid[j] = -(float)dist;
            }
            else
            {
                mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
            }
        }

        W_VectorCopy( mid, f->p[f->numpoints] );
        f->numpoints++;
        W_VectorCopy( mid, b->p[b->numpoints] );
        b->numpoints++;
    }

    if ( f->numpoints > maxpts || b->numpoints > maxpts )
    {
        Com_Error( ERR_FATAL, "Winding_Clip: pts exceeded estimate" );
    }
    if ( f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING )
    {
        Com_Error( ERR_FATAL, "Winding_Clip: MAX_POINTS_ON_WINDING" );
    }
    return 3;   // IDA 0x4629a9: split (both front and back produced)
}

// =============================================================================
// 0x462c50 Winding_FindSharedEdge — internal helper for Winding_TryMerge.  Returns 1
// and the two edge-start indices when w1[i] ~= w2[(j+1)%n2] AND w1[(i+1)%n1] ~= w2[j]
// (the shared edge runs in opposite directions in the two windings).
// =============================================================================
static int Winding_FindSharedEdge( winding_t *w1, winding_t *w2,
                                   unsigned int *idx1, int *idx2 )
{
    unsigned int i, j;
    unsigned int n1 = (unsigned int)w1->numpoints;
    unsigned int n2 = (unsigned int)w2->numpoints;

    if ( !n1 || !n2 )
    {
        return 0;
    }

    for ( i = 0; i < n1; i++ )
    {
        float *pi      = w1->p[i];
        float *pi_next = w1->p[( i + 1 ) % n1];

        for ( j = 0; j < n2; j++ )
        {
            float *pj_next = w2->p[( j + 1 ) % n2];
            float *pj      = w2->p[j];

            // w1[i] ≈ w2[(j+1)%n2]  AND  w1[(i+1)%n1] ≈ w2[j]
            if ( Point_Equal( pi,      pj_next, 0.1f ) &&
                 Point_Equal( pi_next, pj,      0.1f ) )
            {
                *idx1 = i;
                *idx2 = (int)j;
                return 1;
            }
        }
    }
    return 0;
}

// =============================================================================
// 0x462d40 Winding_TryMerge(f1@<eax>, f2, planenormal).  Merges two windings across
// their shared edge if both junction corners stay convex, else NULL.  Neither input is
// freed.  `keep` exists only for GtkRadiant API compatibility — the binary always
// builds the full merged polygon (no keep1/keep2 first-point removal).
// =============================================================================
winding_t *Winding_TryMerge( winding_t *f1, winding_t *f2,
                               vec3_t planenormal, int keep )
{
    (void)keep;   // CoD binary doesn't use keep; IDA has no keep1/keep2 logic

    unsigned int  idx1;
    int           idx2;
    winding_t    *newf;
    unsigned int  i, k;
    float         delta[3], normal[3];
    float         dot;
    unsigned int  n1, n2;

    if ( !Winding_FindSharedEdge( f1, f2, &idx1, &idx2 ) )
    {
        return NULL;
    }

    n1 = (unsigned int)f1->numpoints;
    n2 = (unsigned int)f2->numpoints;

    // Convexity at f1[idx1] vs f2[(idx2+2)%n2] (0x462d6d).
    {
        float *back = f1->p[( idx1 + n1 - 1 ) % n1];
        float *p1   = f1->p[idx1];
        delta[0] = p1[0] - back[0];
        delta[1] = p1[1] - back[1];
        delta[2] = p1[2] - back[2];
        Vec3Cross( planenormal, delta, normal );
        Vec3Normalize( normal );
        float *test = f2->p[( (unsigned int)idx2 + 2 ) % n2];
        dot = normal[0] * ( test[0] - p1[0] )
            + normal[1] * ( test[1] - p1[1] )
            + normal[2] * ( test[2] - p1[2] );
        if ( (double)dot > CONTINUOUS_EPSILON )
        {
            return NULL;
        }
    }

    // Convexity at f1[(idx1+1)%n1] vs f2[(idx2+n2-1)%n2] (0x462e39).
    {
        float *p2   = f1->p[( idx1 + 1 ) % n1];
        float *next = f1->p[( idx1 + 2 ) % n1];
        delta[0] = next[0] - p2[0];
        delta[1] = next[1] - p2[1];
        delta[2] = next[2] - p2[2];
        Vec3Cross( planenormal, delta, normal );
        Vec3Normalize( normal );
        float *test = f2->p[( (unsigned int)idx2 + n2 - 1 ) % n2];
        dot = normal[0] * ( test[0] - p2[0] )
            + normal[1] * ( test[1] - p2[1] )
            + normal[2] * ( test[2] - p2[2] );
        if ( (double)dot > CONTINUOUS_EPSILON )
        {
            return NULL;
        }
    }

    newf = Winding_Alloc( n1 + n2 );

    // f1 from (idx1+1)%n1, excluding idx1; then f2 from (idx2+1)%n2, excluding idx2.
    for ( k = ( idx1 + 1 ) % n1; k != idx1; k = ( k + 1 ) % n1 )
    {
        W_VectorCopy( f1->p[k], newf->p[newf->numpoints] );
        newf->numpoints++;
    }

    i = (unsigned int)( ( idx2 + 1 ) % n2 );
    while ( i != (unsigned int)idx2 )
    {
        W_VectorCopy( f2->p[i], newf->p[newf->numpoints] );
        newf->numpoints++;
        i = ( i + 1 ) % n2;
    }

    return newf;
}

// =============================================================================
// Winding_IsTiny — GtkRadiant reference; true if <3 edges exceed EDGE_LENGTH.
// =============================================================================
int Winding_IsTiny( winding_t *w )
{
    int   i, j;
    float len;
    float delta[3];
    int   edges;

    edges = 0;
    for ( i = 0; i < w->numpoints; i++ )
    {
        j = ( i == w->numpoints - 1 ) ? 0 : i + 1;
        delta[0] = w->p[j][0] - w->p[i][0];
        delta[1] = w->p[j][1] - w->p[i][1];
        delta[2] = w->p[j][2] - w->p[i][2];
        len = Vec3Length( delta );
        if ( len > EDGE_LENGTH )
        {
            if ( ++edges == 3 )
            {
                return 0;
            }
        }
    }
    return 1;
}

// =============================================================================
// Winding_IsHuge — GtkRadiant reference; true if any coord leaves the world bounds.
// =============================================================================
int Winding_IsHuge( winding_t *w )
{
    int i, j;

    for ( i = 0; i < w->numpoints; i++ )
    {
        for ( j = 0; j < 3; j++ )
        {
            if ( w->p[i][j] < -BOGUS_RANGE_WINDING + 1.0f ||
                 w->p[i][j] >  BOGUS_RANGE_WINDING - 1.0f )
            {
                return 1;
            }
        }
    }
    return 0;
}

// =============================================================================
// Winding_BaseForPlane and its common/polylib.cpp helper chain.  These are CoD's own
// polylib functions, NOT GtkRadiant's BaseWindingForPlane (0x4d7380 has no xrefs in
// this binary).  Where GtkRadiant makes a huge quad on the plane, CoD builds the base
// winding as the CONVEX HULL of the plane's intersections with the brush AABB
// (expanded +/-1 by the caller, Brush_MakeFaceWinding 0x471260).  Map-load path.
//
// Leaf math is reused from the kisak engine: IntersectPlanes / PointInBounds
// (universal/com_math.cpp), Com_ConvexHullIndices (universal/com_convexhull.cpp, whose
// KISAK_RADIANT delta is IDA sub_49E150), Vec3Cross / Vec3Sub.
// The binary's CM_ReverseWinding (0x4d7dc0) ALLOCATES a reversed copy while kisak's is
// in-place/void (a CoD3-vs-CoD4 divergence), so the orientation flip uses this file's
// own Winding_Reverse, which allocates.
// The plane-orientation cross product is inlined verbatim from PlaneFromPoints_Real
// (0x4a9950: normal = (C-B) x (A-B), pivot B) rather than calling kisak's
// PlaneFromPoints, to stay independent of any CoD3/CoD4 cross-order divergence.
// =============================================================================

// 0x4A4650 — pick the two projection axes for a polygon facing 'normal': drop the
// dominant axis and order the other two so the projected winding keeps its orientation
// (handedness depends on the dominant axis SIGN).  Also used by primarylights_region.
void Polylib_PickProjectionAxes( const float *normal, int *axis0, int *axis1 )
{
    float nx2 = normal[0] * normal[0];
    float ny2 = normal[1] * normal[1];
    float nz2 = normal[2] * normal[2];

    if ( nx2 > nz2 || ny2 > nz2 )
    {
        if ( ny2 < nx2 || ny2 < nz2 )
        {
            // X is the dominant axis
            if ( normal[0] <= 0.0f ) { *axis0 = 2; *axis1 = 1; }
            else                     { *axis0 = 1; *axis1 = 2; }
        }
        else if ( normal[1] <= 0.0f ) { *axis0 = 0; *axis1 = 2; }   // Y dominant
        else                          { *axis0 = 2; *axis1 = 0; }
    }
    else if ( normal[2] <= 0.0f ) { *axis0 = 1; *axis1 = 0; }       // Z dominant
    else                          { *axis0 = 0; *axis1 = 1; }
}

// 0x4D99D0 — largest-area triangle among 'count' points; returns max |2*area| and the
// three vertex indices (used to derive a robust plane orientation for the hull).
static float Polylib_LargestTriangle( const vec3_t *pts, int count, const float *normal,
                                      int *i_a4, int *i_a5, int *i_a6 )
{
    float maxArea = 0.0f;
    *i_a5 = 1;
    *i_a4 = 0;
    *i_a6 = 2;

    for ( int a = 2; a < count; a++ )           // v6
    {
        for ( int b = 1; b < a; b++ )           // v9
        {
            for ( int c = 0; c < b; c++ )       // v11
            {
                float d1[3], d2[3], cr[3];
                // d1 = pts[a] - pts[b] (v20), d2 = pts[c] - pts[b] (v21)
                d1[0] = pts[a][0] - pts[b][0];
                d1[1] = pts[a][1] - pts[b][1];
                d1[2] = pts[a][2] - pts[b][2];
                d2[0] = pts[c][0] - pts[b][0];
                d2[1] = pts[c][1] - pts[b][1];
                d2[2] = pts[c][2] - pts[b][2];
                Vec3Cross( d2, d1, cr );        // IDA: Vec3Cross(v21, v20, v22)
                float area = fabsf( normal[0] * cr[0] + normal[1] * cr[1] + normal[2] * cr[2] );
                if ( maxArea < area )
                {
                    maxArea = area;
                    *i_a4 = c;
                    *i_a5 = b;
                    *i_a6 = a;
                }
            }
        }
    }
    return maxArea;
}

// 0x4D7670 — winding from a 3D point cloud: project to the dominant 2D plane, run the
// index-returning hull, map indices back to 3D, orient the result to 'normal'.  Also
// used by primarylights_region.
winding_t *Polylib_HullForPoints( const vec3_t *xyz, const float *normal, int xyzCount )
{
    // 'proj' keeps the IDB name so the array-bound assert stringizes 1:1 (polylib.cpp:336).
    float proj[64][2];

    vassert( (xyzCount <= (sizeof( proj ) / (sizeof( proj[0] ) * (sizeof( proj ) != 4 || sizeof( proj[0] ) <= 4)))), "(xyzCount) = %i", xyzCount );

    int axis0, axis1;
    Polylib_PickProjectionAxes( normal, &axis0, &axis1 );

    // Project all points onto the (axis0, axis1) plane.
    for ( int i = 0; i < xyzCount; i++ )
    {
        proj[i][0] = xyz[i][axis0];
        proj[i][1] = xyz[i][axis1];
    }

    // 2D convex hull → indices into proj (== indices into xyz).
    unsigned int hullOrder[64];
    unsigned int hullCount = Com_ConvexHullIndices( proj, (unsigned int)xyzCount, hullOrder );

    vassert( hullCount <= xyzCount, "%i, %i", hullCount, xyzCount );   // polylib.cpp:343

    ++g_windingAlloc;
    winding_t *w = (winding_t *)malloc( 12 * hullCount + 4 );
    if ( !w )
    {
        Com_PrintMessage( "out of memory: winding_t\n" );
    }
    w->numpoints = (int)hullCount;
    for ( unsigned int i = 0; i < hullCount; i++ )
    {
        W_VectorCopy( xyz[hullOrder[i]], w->p[i] );   // hull index → original 3D point
    }

    // Orient the winding so it faces 'normal' (largest-triangle plane test).
    int iA4, iA5, iA6;
    if ( Polylib_LargestTriangle( w->p, (int)hullCount, normal, &iA4, &iA5, &iA6 ) >= 0.001f )
    {
        // IDA PlaneFromPoints_Real(out, A=p[iA5], B=p[iA4], C=p[iA6]):
        //   testNormal = (C - B) × (A - B), pivot B = p[iA4].
        float e1[3], e2[3], n2[3];
        e1[0] = w->p[iA6][0] - w->p[iA4][0];   // C - B
        e1[1] = w->p[iA6][1] - w->p[iA4][1];
        e1[2] = w->p[iA6][2] - w->p[iA4][2];
        e2[0] = w->p[iA5][0] - w->p[iA4][0];   // A - B
        e2[1] = w->p[iA5][1] - w->p[iA4][1];
        e2[2] = w->p[iA5][2] - w->p[iA4][2];
        Vec3Cross( e1, e2, n2 );               // (C-B) × (A-B)

        float facing = normal[0] * n2[0] + normal[1] * n2[1] + normal[2] * n2[2];
        if ( facing >= 0.0f )
        {
            return w;
        }
        // Wound the wrong way — reverse (alloc copy, free original).
        winding_t *r = Winding_Reverse( w );
        Winding_Free( w );
        return r;
    }

    // Degenerate (no triangle with area) — discard.
    Winding_Free( w );
    return NULL;
}

// 0x4D7AB0 — for the axis pair (axisI, axisJ), test the four AABB edges parallel to the
// third axis against 'plane' and append each in-bounds intersection.  Returns 0..4.
// boundsPlanes is the 6-plane array {+X,+Y,+Z,-X,-Y,-Z}, each 4 floats {n[3], dist}.
static int Polylib_PlaneBoxEdges( const float *plane, const float *boundsPlanes,
                                  const float *mins, const float *maxs,
                                  int axisI, int axisJ, float *out )
{
    const float *planes[3];
    planes[0] = plane;
    int count = 0;

    // corner (axisI, axisJ)
    planes[1] = boundsPlanes + 4 * axisI;
    planes[2] = boundsPlanes + 4 * axisJ;
    if ( IntersectPlanes( planes, out ) && PointInBounds( out, mins, maxs ) )
        ++count;

    // corner (axisI, axisJ+3)
    planes[2] = boundsPlanes + 4 * ( axisJ + 3 );
    if ( IntersectPlanes( planes, out + 3 * count ) && PointInBounds( out + 3 * count, mins, maxs ) )
        ++count;

    // corner (axisI+3, axisJ+3)
    planes[1] = boundsPlanes + 4 * ( axisI + 3 );
    if ( IntersectPlanes( planes, out + 3 * count ) && PointInBounds( out + 3 * count, mins, maxs ) )
        ++count;

    // corner (axisI+3, axisJ)
    planes[2] = boundsPlanes + 4 * axisJ;
    if ( IntersectPlanes( planes, out + 3 * count ) && PointInBounds( out + 3 * count, mins, maxs ) )
        ++count;

    return count;
}

// =============================================================================
// 0x4d7bc0 Winding_BaseForPlane(maxs@<eax>, mins@<ebx>, plane).  Convex hull of the
// plane's intersections with the [mins,maxs] AABB; NULL below 3 points.
// =============================================================================
winding_t *Winding_BaseForPlane( vec3_t maxs, vec3_t mins, plane_t *plane )
{
    // Six AABB bounding planes, each {nx,ny,nz,dist}, normal·p = dist:
    //   +X: x<=maxs[0]  +Y: y<=maxs[1]  +Z: z<=maxs[2]
    //   -X: x>=mins[0]  -Y: y>=mins[1]  -Z: z>=mins[2]   (negated normal + dist)
    float bounds[24];
    bounds[0]  = 1.0f;  bounds[1]  = 0.0f;  bounds[2]  = 0.0f;  bounds[3]  =  maxs[0];
    bounds[4]  = 0.0f;  bounds[5]  = 1.0f;  bounds[6]  = 0.0f;  bounds[7]  =  maxs[1];
    bounds[8]  = 0.0f;  bounds[9]  = 0.0f;  bounds[10] = 1.0f;  bounds[11] =  maxs[2];
    bounds[12] = -1.0f; bounds[13] = 0.0f;  bounds[14] = 0.0f;  bounds[15] = -mins[0];
    bounds[16] = 0.0f;  bounds[17] = -1.0f; bounds[18] = 0.0f;  bounds[19] = -mins[1];
    bounds[20] = 0.0f;  bounds[21] = 0.0f;  bounds[22] = -1.0f; bounds[23] = -mins[2];

    // 'plane' as a flat {n[3],dist} for IntersectPlanes (plane_t = vec3 normal + float dist).
    float facePlane[4];
    facePlane[0] = plane->normal[0];
    facePlane[1] = plane->normal[1];
    facePlane[2] = plane->normal[2];
    facePlane[3] = (float)plane->dist;

    // Up to 12 candidate points (4 per axis pair × 3 pairs). IDA out buffer v8[144].
    float pts[12][3];
    int n = 0;
    n += Polylib_PlaneBoxEdges( facePlane, bounds, mins, maxs, 0, 1, pts[n] );
    n += Polylib_PlaneBoxEdges( facePlane, bounds, mins, maxs, 1, 2, pts[n] );
    n += Polylib_PlaneBoxEdges( facePlane, bounds, mins, maxs, 2, 0, pts[n] );

    if ( n >= 3 )
    {
        return Polylib_HullForPoints( pts, plane->normal, n );
    }
    return NULL;
}

// =============================================================================
// Winding_Plane — GtkRadiant reference; best-fit plane normal + distance.
// =============================================================================
void Winding_Plane( winding_t *w, vec3_t normal, double *dist )
{
    float v1[3], v2[3];
    int   i;

    for ( i = 0; i < w->numpoints; i++ )
    {
        v1[0] = w->p[( i + 1 ) % w->numpoints][0] - w->p[i][0];
        v1[1] = w->p[( i + 1 ) % w->numpoints][1] - w->p[i][1];
        v1[2] = w->p[( i + 1 ) % w->numpoints][2] - w->p[i][2];
        v2[0] = w->p[( i + 2 ) % w->numpoints][0] - w->p[i][0];
        v2[1] = w->p[( i + 2 ) % w->numpoints][1] - w->p[i][1];
        v2[2] = w->p[( i + 2 ) % w->numpoints][2] - w->p[i][2];
        if ( Vec3Length( v1 ) > 0.5f && Vec3Length( v2 ) > 0.5f )
        {
            break;
        }
    }
    // cross(v2, v1, normal) — GtkRadiant uses CrossProduct(v2, v1, normal)
    Vec3Cross( v2, v1, normal );
    Vec3Normalize( normal );
    *dist = (double)W_DotProduct( w->p[0], normal );
}

// =============================================================================
// Winding_Area — GtkRadiant reference; triangle-fan area sum.
// =============================================================================
float Winding_Area( winding_t *w )
{
    int   i;
    float d1[3], d2[3], cross[3];
    float total;

    total = 0.0f;
    for ( i = 2; i < w->numpoints; i++ )
    {
        d1[0] = w->p[i - 1][0] - w->p[0][0];
        d1[1] = w->p[i - 1][1] - w->p[0][1];
        d1[2] = w->p[i - 1][2] - w->p[0][2];
        d2[0] = w->p[i][0] - w->p[0][0];
        d2[1] = w->p[i][1] - w->p[0][1];
        d2[2] = w->p[i][2] - w->p[0][2];
        Vec3Cross( d1, d2, cross );
        total += 0.5f * Vec3Length( cross );
    }
    return total;
}

// =============================================================================
// Winding_Bounds — GtkRadiant reference; AABB of the winding.
// =============================================================================
void Winding_Bounds( winding_t *w, vec3_t mins, vec3_t maxs )
{
    float v;
    int   i, j;

    mins[0] = mins[1] = mins[2] =  99999.0f;
    maxs[0] = maxs[1] = maxs[2] = -99999.0f;

    for ( i = 0; i < w->numpoints; i++ )
    {
        for ( j = 0; j < 3; j++ )
        {
            v = w->p[i][j];
            if ( v < mins[j] ) { mins[j] = v; }
            if ( v > maxs[j] ) { maxs[j] = v; }
        }
    }
}

// =============================================================================
// Winding_PointInside — GtkRadiant reference; 'point' inside the winding, on-plane.
// =============================================================================
int Winding_PointInside( winding_t *w, plane_t *plane, vec3_t point, float epsilon )
{
    int   i;
    float dir[3], pointvec[3], normal[3];

    for ( i = 0; i < w->numpoints; i++ )
    {
        dir[0]      = w->p[( i + 1 ) % w->numpoints][0] - w->p[i][0];
        dir[1]      = w->p[( i + 1 ) % w->numpoints][1] - w->p[i][1];
        dir[2]      = w->p[( i + 1 ) % w->numpoints][2] - w->p[i][2];
        pointvec[0] = point[0] - w->p[i][0];
        pointvec[1] = point[1] - w->p[i][1];
        pointvec[2] = point[2] - w->p[i][2];
        Vec3Cross( dir, plane->normal, normal );
        if ( W_DotProduct( pointvec, normal ) < -epsilon )
        {
            return 0;
        }
    }
    return 1;
}

// =============================================================================
// Winding_VectorIntersect — GtkRadiant reference; segment p1-p2 hits the winding.
// =============================================================================
int Winding_VectorIntersect( winding_t *w, plane_t *plane,
                              vec3_t p1, vec3_t p2, float epsilon )
{
    float front, back, frac;
    float mid[3];

    front = W_DotProduct( p1, plane->normal ) - plane->dist;
    back  = W_DotProduct( p2, plane->normal ) - plane->dist;

    if ( front < -epsilon && back < -epsilon )
    {
        return 0;
    }
    if ( front > epsilon && back > epsilon )
    {
        return 0;
    }

    if ( fabsf( front - back ) < 0.001f )
    {
        W_VectorCopy( p2, mid );
    }
    else
    {
        frac   = front / ( front - back );
        mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
        mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
        mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;
    }

    return Winding_PointInside( w, plane, mid, epsilon );
}

// =============================================================================
// Brush_MoveVertex 0x471C30 lives in brush.cpp, not here: its asserts cite
// brush.cpp:1146..1302, its EA sits in the brush.cpp .text cluster, and it depends on
// Face_Alloc / Face_MakePlane / Brush_Convex / Brush_RemoveFace.  It is ported there.
