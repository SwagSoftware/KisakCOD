#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\src\universal\g_vehicle_path.cpp — the vehicle-path node graph and the editor's
// path-preview overlay.  Asserts in this TU cite
// "C:\trees\cod3-pc\cod3-modtools\cod3src\src\universal\g_vehicle_path.cpp" (lines 153,
// 1143-1145), which is how the file was identified.  Only two of its functions carry
// assert strings; the rest were recovered by sweeping 0x4B5150-0x4B6C10 (a single link-
// order .obj run bounded by fx code below and linearmapping.cpp above) plus the xrefs of
// the node table / node count.
//
// The subsystem is a real ENGINE system (a vehicle drives a linked list of
// info_vehicle_node entities, interpolating speed / lookahead / rotation between them);
// CoD4Radiant compiles it so the editor can SIMULATE a vehicle along the selected path
// and draw the resulting curve + heading arrows in the XY/camera views.
//
//   0x4B5150  VehiclePath_AddPathSegment       collinear-merging 3D line emitter
//   0x4B52C0  VehiclePath_DrawArrow            5-point heading arrow (g_vehicle_path.cpp:153)
//   0x4B5550  VehiclePathNode_Init             node ctor (IDA mis-named _WinMain@16_199_0)
//   0x4B55B0  VehiclePathNode_Copy             node assignment (IDA _WinMain@16_197_0)
//   0x4B5630  VehiclePath_FindNode             (origin, targetname) -> index or -1
//   0x4B56E0  VehiclePath_IsNodeReachable      forward-walk connectivity test
//   0x4B5740  VehiclePath_ResolveNodeSpeed     inherit/interpolate a missing "speed"
//   0x4B58A0  VehiclePath_ResolveNodeLookahead inherit/interpolate a missing "lookahead"
//   0x4B5A00  VehiclePath_ResolveNodeAngles    inherit/interpolate missing rotate angles
//   0x4B5E50  VehiclePath_RotateNodeBlend      rotate-node blend weight for the tracer
//   0x4B5EE0  VehiclePath_BlendRotateAngles    blend the tracer heading toward node angles
//   0x4B6050  VehiclePath_GetLookaheadPos      point `speed*lookahead` ahead on the path
//   0x4B6100  VehiclePath_AdvanceToLookahead   re-seat the tracer on its current segment
//   0x4B6370  VehiclePath_DrawPath             simulate + draw one path (50 ms per step)
//   0x4B64B0  VehiclePath_LinkNodes            resolve target/targetname into next/prev
//   0x4B6710  VehiclePath_AddNode              the overlay entry point (DrawConnectionLinks)
//   0x4B6AC0  VehiclePath_InitTracer           seed a tracer on a node
//   0x4B6B50  VehiclePath_StepTracer           one 50 ms simulation step
//
// Globals: g_vehiclePathNodeCount 0x2473D58, g_vehiclePathNodes 0x2473D60 (stride 72),
// g_vehiclePathAnglesUnset 0x739360, g_vehiclePathFirstSegment 0x73935C,
// g_vehiclePathSegStart/End/Dir 0x26661B8/C4/D0.

#include "stdafx.h"
#include "prefs.h"                  // g_PrefsDlg->vehicle_arrow_time / vehicle_arrow_size
#include <gfx_d3d/r_gfx.h>          // GfxColor, GfxPointVertex
#include <gfx_d3d/r_rendercmds.h>   // R_AddCmd_Line3D, GfxPointVertex, GfxColor
#include <universal/com_math.h>     // AnglesToAxis, MatrixTransformVector43
#include <universal/q_shared.h>     // I_stricmp
#include <universal/assertive.h>    // iassert
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ── engine / editor bridges ──────────────────────────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );
extern void  R_Warn( int warnType, const char *fmt, ... );        // cmdlib.cpp (0x40B630)
extern float Vec3Normalize_R( float *v );                         // engine_stubs.cpp (0x40A5E0)
extern void  vectoangles( float *angles, int vec );               // engine_stubs.cpp (0x4A5020)
extern char  Byte4PackPixelColor( float *from, GfxColor *out );   // engine_stubs.cpp (0x402AC0)
extern int   R_Add3DLine( GfxPointVertex *verts, const orientation_t *orient,
                          const float *p1, const float *p2, const unsigned int *color,
                          char width, int vertCount, int maxVertCount );   // draw.cpp (0x40C110)
extern float world_orient_matrix[4][3];                           // entity.cpp (0x6DE290)

extern entity_s   entityInsts;        // entity.cpp  (0x23F1748) — entity-instance ring
extern selbrush_t selected_brushes;   // map.cpp     (0x23F1864) — selection ring sentinel

extern char   FilterBrush( selbrush_t *b, int updateFilters );        // filters.cpp (0x46A1F0)
extern char  *ValueForKey2( int e, const char *key );                 // entity.cpp  (0x4825C0)
extern int    Entity_GetIntValueForKey( int e, const char *key );     // entity.cpp  (0x483820)
extern int    Entity_GetVec3ForKey( entity_s_def *e, float *out, const char *key ); // entity.cpp (0x483860)
extern bool   HasKeyValuePair( entity_s_def *e, const char *key );    // entity.cpp  (0x4838B0)

// ─────────────────────────────────────────────────────────────────────────────
//  VehiclePathNode — the 72-byte node record (IDB: g_vehiclePathNodes indexed as
//  dword_2473D60[18 * i]).  Layout recovered from VehiclePathNode_Init (0x4B5550,
//  which writes every field), VehiclePathNode_Copy (0x4B55B0, a field-by-field
//  assignment covering the whole record) and the interior aliases IDA had named
//  (+8 spawnflags1, +16 isRotateNode, +20 speed, +24 lookahead, +28 origin,
//  +64 dist, +68 nextIndex, +70 prevIndex).  +12 is written as a WORD only (both by
//  the ctor and by the assignment operator), so +14 is padding.
// ─────────────────────────────────────────────────────────────────────────────
struct VehiclePathNode
{
    const char *targetname;    // 0x00  "targetname" epair
    const char *target;        // 0x04  "target" epair (the NEXT node's targetname)
    int         spawnflags1;   // 0x08  spawnflags & 1  (the "path start" flag)
    __int16     index;         // 0x0C  this node's own index
    __int16     pad_0e;        // 0x0E  (never written)
    int         isRotateNode;  // 0x10  eclass == info_vehicle_node_rotate
    float       speed;         // 0x14  units/sec (mph * 17.6); -1 = "inherit"
    float       lookahead;     // 0x18  -1 = "inherit"
    float       origin[3];     // 0x1C  world position (the entity's origin)
    float       dir[3];        // 0x28  UNIT direction to the next node (LinkNodes)
    float       angles[3];     // 0x34  heading; (PI,PI,PI) = "unset" (see below)
    float       dist;          // 0x40  distance to the next node (LinkNodes)
    __int16     nextIndex;     // 0x44  node this one targets, or -1
    __int16     prevIndex;     // 0x46  node that targets this one, or -1
};
static_assert( sizeof( VehiclePathNode ) == 72, "VehiclePathNode must be 72 bytes (IDB stride)" );
static_assert( offsetof( VehiclePathNode, isRotateNode ) == 0x10, "VehiclePathNode.isRotateNode" );
static_assert( offsetof( VehiclePathNode, speed )        == 0x14, "VehiclePathNode.speed" );
static_assert( offsetof( VehiclePathNode, lookahead )    == 0x18, "VehiclePathNode.lookahead" );
static_assert( offsetof( VehiclePathNode, origin )       == 0x1C, "VehiclePathNode.origin" );
static_assert( offsetof( VehiclePathNode, dir )          == 0x28, "VehiclePathNode.dir" );
static_assert( offsetof( VehiclePathNode, angles )       == 0x34, "VehiclePathNode.angles" );
static_assert( offsetof( VehiclePathNode, dist )         == 0x40, "VehiclePathNode.dist" );
static_assert( offsetof( VehiclePathNode, nextIndex )    == 0x44, "VehiclePathNode.nextIndex" );
static_assert( offsetof( VehiclePathNode, prevIndex )    == 0x46, "VehiclePathNode.prevIndex" );

// ─────────────────────────────────────────────────────────────────────────────
//  VehiclePathTracer — the 200-byte simulation state (the IDB shows it only as the
//  `__int16 v12[100]` stack local in VehiclePath_AddNode).  Field roles recovered
//  from VehiclePath_InitTracer (0x4B6AC0, which writes +0..+52 then default-
//  constructs the two embedded nodes) and VehiclePath_AdvanceToLookahead (0x4B6100).
// ─────────────────────────────────────────────────────────────────────────────
struct VehiclePathTracer
{
    __int16         nodeIndex;      // 0x00  node whose segment we are on
    __int16         atEnd;          // 0x02  set once the walk cannot advance
    float           frac;           // 0x04  fraction along nodeIndex's segment
    float           speed;          // 0x08  interpolated speed at `frac`
    float           lookahead;      // 0x0C  interpolated lookahead at `frac`
    float           rotateBlend;    // 0x10  rotate-node blend weight
    float           origin[3];      // 0x14  simulated position
    float           angles[3];      // 0x20  simulated heading
    float           lookaheadPos[3];// 0x2C  the point being steered toward
    VehiclePathNode nodeOverride;   // 0x38  see VehiclePath_StepTracer
    VehiclePathNode nodeSaved;      // 0x80  see VehiclePath_StepTracer
};
static_assert( sizeof( VehiclePathTracer ) == 200, "VehiclePathTracer must be 200 bytes (the IDB __int16[100] local)" );
static_assert( offsetof( VehiclePathTracer, origin )       == 0x14, "VehiclePathTracer.origin" );
static_assert( offsetof( VehiclePathTracer, angles )       == 0x20, "VehiclePathTracer.angles" );
static_assert( offsetof( VehiclePathTracer, lookaheadPos ) == 0x2C, "VehiclePathTracer.lookaheadPos" );
static_assert( offsetof( VehiclePathTracer, nodeOverride ) == 0x38, "VehiclePathTracer.nodeOverride" );
static_assert( offsetof( VehiclePathTracer, nodeSaved )    == 0x80, "VehiclePathTracer.nodeSaved" );

#define MAX_VEHICLE_NODES 4000     // the Sys_Printf("...Max vehicle Nodes hit [%d]", 4000) cap

// The node table.  IDB 0x2473D58 is a DWORD slot but EVERY access to the count is
// 16-bit (`mov word ptr`, `add word ptr [..],1`, `cmp ax,0FA0h`, `(__int16)` compares)
// and the upper half is never touched — a `short` reproduces all of them exactly
// (§11 int16-truncation).
static __int16         g_vehiclePathNodeCount;                       // 0x2473D58
static VehiclePathNode g_vehiclePathNodes[MAX_VEHICLE_NODES];        // 0x2473D60

// 0x739360 — the "angles were never authored" sentinel a fresh node is stamped with.
// The three initialised floats there are literally (PI,PI,PI): an angle triple no map
// ever produces, so the resolve pass can tell "unset" from a genuine 0,0,0 heading.
static const float g_vehiclePathAnglesUnset[3] =
    { 3.1415927410125732f, 3.1415927410125732f, 3.1415927410125732f };

// 0x73935C / 0x26661B8 / 0x26661C4 / 0x26661D0 — VehiclePath_AddPathSegment's pending
// segment.  Initialised to 1 in .data (the first AddPathSegment call after a
// VehiclePath_DrawPath reset only records, it does not emit).
static int   g_vehiclePathFirstSegment = 1;                          // 0x73935C
static float g_vehiclePathSegStart[3];                               // 0x26661B8
static float g_vehiclePathSegEnd[3];                                 // 0x26661C4
static float g_vehiclePathSegDir[3];                                 // 0x26661D0

// ─────────────────────────────────────────────────────────────────────────────
//  AngleNormalize180, as g_vehicle_path.cpp inlines it everywhere:
//      a = a * (1/360);  a = (a - floorf(a + 0.5)) * 360
//  NOT kisak's com_math AngleNormalize180 (which is the fmodf form) — same range,
//  different float rounding, and this file's output feeds the drawn overlay.
//  The two intermediate float stores (fstp dword after the multiply and after the
//  +0.5) and the double slot the pre-add value is kept in are the binary's, verbatim:
//  dbl_6F42C0 = (double)(1.0f/360.0f), dbl_6F4160 = 0.5, dbl_6F42B8 = 360.0.
// ─────────────────────────────────────────────────────────────────────────────
static float VehiclePath_AngleNormalize180( float angle )
{
    float  scaled = (float)( (double)angle * 0.0027777778450399637 );   // fmul dbl_6F42C0; fstp dword
    double kept   = scaled;                                             // fst  qword (the pre-add copy)
    float  rounded = (float)( kept + 0.5 );                             // fadd dbl_6F4160; fstp dword
    return (float)( ( kept - (double)floorf( rounded ) ) * 360.0 );
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5550  VehiclePathNode_Init — the node constructor.  IDA named this
//  `_WinMain@16_199_0` (a COMDAT-folding artifact: VC7.1 gave the outlined
//  __usercall(this@<eax>, index@<dx>) body a WinMain alias).  Zeroes the record,
//  parks speed/lookahead at -1 ("inherit from a neighbour"), stamps the angles with
//  the (PI,PI,PI) unset sentinel and both link indices with -1.
// ─────────────────────────────────────────────────────────────────────────────
static VehiclePathNode *VehiclePathNode_Init( VehiclePathNode *node, __int16 index )
{
    node->index      = index;
    node->speed      = -1.0f;
    node->lookahead  = -1.0f;
    node->targetname = 0;
    node->target     = 0;
    node->spawnflags1 = 0;
    node->isRotateNode = 0;
    node->origin[0] = 0.0f;
    node->origin[1] = 0.0f;
    node->origin[2] = 0.0f;
    node->dir[0]    = 0.0f;
    node->dir[1]    = 0.0f;
    node->dir[2]    = 0.0f;
    node->angles[0] = g_vehiclePathAnglesUnset[0];
    node->angles[1] = g_vehiclePathAnglesUnset[1];
    node->angles[2] = g_vehiclePathAnglesUnset[2];
    node->nextIndex = -1;
    node->prevIndex = -1;
    node->dist      = 0.0f;
    return node;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B55B0  VehiclePathNode_Copy — field-by-field node assignment (IDA
//  `_WinMain@16_197_0`; dst in EAX, src in ECX).  Note it copies +0x0C as a WORD and
//  never touches +0x0E — the padding proof for VehiclePathNode.index.
// ─────────────────────────────────────────────────────────────────────────────
static VehiclePathNode *VehiclePathNode_Copy( VehiclePathNode *dst, const VehiclePathNode *src )
{
    dst->targetname   = src->targetname;
    dst->target       = src->target;
    dst->spawnflags1  = src->spawnflags1;
    dst->index        = src->index;
    dst->isRotateNode = src->isRotateNode;
    dst->speed        = src->speed;
    dst->lookahead    = src->lookahead;
    dst->origin[0]    = src->origin[0];
    dst->origin[1]    = src->origin[1];
    dst->origin[2]    = src->origin[2];
    dst->dir[0]       = src->dir[0];
    dst->dir[1]       = src->dir[1];
    dst->dir[2]       = src->dir[2];
    dst->angles[0]    = src->angles[0];
    dst->angles[1]    = src->angles[1];
    dst->angles[2]    = src->angles[2];
    dst->dist         = src->dist;
    dst->nextIndex    = src->nextIndex;
    dst->prevIndex    = src->prevIndex;
    return dst;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5630  VehiclePath_FindNode — index of the node whose targetname matches, and
//  (when `origin` is non-NULL) whose origin matches EXACTLY.  -1 = not found.
//  The origin clause is what lets two nodes share a targetname: the editor entry
//  point passes the selected entity's origin to disambiguate; the link pass passes
//  NULL because it only has the target STRING to go on.
// ─────────────────────────────────────────────────────────────────────────────
static __int16 VehiclePath_FindNode( const float *origin, const char *targetname )
{
    if ( !targetname )
        return -1;

    __int16 i = 0;
    if ( g_vehiclePathNodeCount <= 0 )
        return -1;

    while ( strcmp( g_vehiclePathNodes[i].targetname, targetname )
            || ( origin && ( origin[0] != g_vehiclePathNodes[i].origin[0]
                          || origin[1] != g_vehiclePathNodes[i].origin[1]
                          || origin[2] != g_vehiclePathNodes[i].origin[2] ) ) )
    {
        if ( ++i >= g_vehiclePathNodeCount )
            return -1;
    }
    return i;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B56E0  VehiclePath_IsNodeReachable — walk `from` forward along nextIndex; true
//  if `to` is hit.  The walk stops on a dangling link, on a self-loop back to `from`,
//  and after g_vehiclePathNodeCount hops (the cycle guard).
// ─────────────────────────────────────────────────────────────────────────────
static int VehiclePath_IsNodeReachable( __int16 from, __int16 to )
{
    if ( from < 0 )
        return 0;
    if ( to < 0 )
        return 0;

    __int16 hops = 0;
    VehiclePathNode *node = &g_vehiclePathNodes[from];
    if ( g_vehiclePathNodeCount <= 0 )
        return 0;

    for ( ;; )
    {
        __int16 next = node->nextIndex;
        ++hops;
        if ( next == to )
            break;
        if ( next >= 0 && next != from )
        {
            node = &g_vehiclePathNodes[next];
            if ( hops < g_vehiclePathNodeCount )
                continue;
        }
        return 0;
    }
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5740  VehiclePath_ResolveNodeSpeed — a node with no "speed" key (speed < 0)
//  inherits from its neighbours: walk BACKWARD (prevIndex) for the nearest authored
//  speed accumulating distance, walk FORWARD (nextIndex) for the other, then blend by
//  the distance ratio.  One side only -> that side; neither -> 0.
//  NOTE the asymmetry vs the lookahead twin below: this one tests `>= 0.0`, the
//  lookahead one tests `> 0.0`.  Faithful — the binary really does differ.
// ─────────────────────────────────────────────────────────────────────────────
static float VehiclePath_ResolveNodeSpeed( __int16 index )
{
    VehiclePathNode *fwdNode = &g_vehiclePathNodes[index];
    if ( g_vehiclePathNodes[index].speed >= 0.0f )
        return g_vehiclePathNodes[index].speed;

    __int16 prev = g_vehiclePathNodes[index].prevIndex;
    float distFwd  = 0.0f;
    float distBack = 0.0f;
    float fwdSpeed  = -1.0f;
    float backSpeed = -1.0f;

    if ( prev >= 0 )
    {
        __int16 hops = 0;
        VehiclePathNode *backNode = &g_vehiclePathNodes[prev];
        if ( g_vehiclePathNodeCount > 0 )
        {
            for ( ;; )
            {
                ++hops;
                distBack = backNode->dist + distBack;
                if ( backNode->speed >= 0.0f )
                    break;
                __int16 pp = backNode->prevIndex;
                if ( pp >= 0 && pp != index )
                {
                    backNode = &g_vehiclePathNodes[pp];
                    if ( hops < g_vehiclePathNodeCount )
                        continue;
                }
                goto backDone;
            }
            backSpeed = backNode->speed;
        }
    }
backDone:
    {
        __int16 hops = 0;
        if ( g_vehiclePathNodeCount > 0 )
        {
            for ( ;; )
            {
                ++hops;
                if ( fwdNode->speed >= 0.0f )
                    break;
                __int16 nx = fwdNode->nextIndex;
                if ( nx >= 0 && nx != index )
                {
                    float acc = fwdNode->dist + distFwd;
                    fwdNode = &g_vehiclePathNodes[nx];
                    distFwd = acc;
                    if ( hops < g_vehiclePathNodeCount )
                        continue;
                }
                goto fwdDone;
            }
            fwdSpeed = fwdNode->speed;
        }
    }
fwdDone:
    {
        float base = backSpeed;
        if ( backSpeed < 0.0f )
        {
            base = fwdSpeed;
            if ( fwdSpeed < 0.0f )
                return 0.0f;
            return base;
        }
        if ( fwdSpeed < 0.0f )
            return base;
        float total = distFwd + distBack;
        if ( total > 0.0f )
        {
            float f = distBack / total;
            return ( fwdSpeed - base ) * f + base;
        }
        return 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B58A0  VehiclePath_ResolveNodeLookahead — the exact twin of the speed resolve
//  above over the `lookahead` field, except the "is it authored?" test is `> 0.0`
//  instead of `>= 0.0`.  Kept as a separate function (the binary does not share one).
// ─────────────────────────────────────────────────────────────────────────────
static float VehiclePath_ResolveNodeLookahead( __int16 index )
{
    VehiclePathNode *fwdNode = &g_vehiclePathNodes[index];
    if ( g_vehiclePathNodes[index].lookahead >= 0.0f )
        return g_vehiclePathNodes[index].lookahead;

    __int16 prev = g_vehiclePathNodes[index].prevIndex;
    float distFwd  = 0.0f;
    float distBack = 0.0f;
    float fwdLook  = -1.0f;
    float backLook = -1.0f;

    if ( prev >= 0 )
    {
        __int16 hops = 0;
        VehiclePathNode *backNode = &g_vehiclePathNodes[prev];
        if ( g_vehiclePathNodeCount > 0 )
        {
            for ( ;; )
            {
                ++hops;
                distBack = backNode->dist + distBack;
                if ( backNode->lookahead > 0.0f )
                    break;
                __int16 pp = backNode->prevIndex;
                if ( pp >= 0 && pp != index )
                {
                    backNode = &g_vehiclePathNodes[pp];
                    if ( hops < g_vehiclePathNodeCount )
                        continue;
                }
                goto backDone;
            }
            backLook = backNode->lookahead;
        }
    }
backDone:
    {
        __int16 hops = 0;
        if ( g_vehiclePathNodeCount > 0 )
        {
            for ( ;; )
            {
                ++hops;
                if ( fwdNode->lookahead > 0.0f )
                    break;
                __int16 nx = fwdNode->nextIndex;
                if ( nx >= 0 && nx != index )
                {
                    float acc = fwdNode->dist + distFwd;
                    fwdNode = &g_vehiclePathNodes[nx];
                    distFwd = acc;
                    if ( hops < g_vehiclePathNodeCount )
                        continue;
                }
                goto fwdDone;
            }
            fwdLook = fwdNode->lookahead;
        }
    }
fwdDone:
    {
        float base = backLook;
        if ( backLook < 0.0f )
        {
            base = fwdLook;
            if ( fwdLook < 0.0f )
                return 0.0f;
            return base;
        }
        if ( fwdLook < 0.0f )
            return base;
        float total = distFwd + distBack;
        if ( total > 0.0f )
        {
            float f = distBack / total;
            return ( fwdLook - base ) * f + base;
        }
        return 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5A00  VehiclePath_ResolveNodeAngles — the angles twin of the two resolves
//  above.  "Authored" here means "!= the (PI,PI,PI) unset sentinel".  Blending is a
//  shortest-arc lerp (AngleNormalize180 on the delta) weighted by the distance ratio.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_ResolveNodeAngles( float *out, int index )
{
    VehiclePathNode *fwdNode = &g_vehiclePathNodes[(__int16)index];

    if ( g_vehiclePathAnglesUnset[0] != g_vehiclePathNodes[(__int16)index].angles[0]
      || g_vehiclePathAnglesUnset[1] != g_vehiclePathNodes[(__int16)index].angles[1]
      || g_vehiclePathAnglesUnset[2] != g_vehiclePathNodes[(__int16)index].angles[2] )
    {
        out[0] = g_vehiclePathNodes[(__int16)index].angles[0];
        out[1] = g_vehiclePathNodes[(__int16)index].angles[1];
        out[2] = g_vehiclePathNodes[(__int16)index].angles[2];
        return;
    }

    __int16 prev = g_vehiclePathNodes[(__int16)index].prevIndex;
    float distFwd  = 0.0f;
    float distBack = 0.0f;
    float backAng[3] = { g_vehiclePathAnglesUnset[0], g_vehiclePathAnglesUnset[1], g_vehiclePathAnglesUnset[2] };
    float fwdAng[3]  = { g_vehiclePathAnglesUnset[0], g_vehiclePathAnglesUnset[1], g_vehiclePathAnglesUnset[2] };

    if ( prev >= 0 )
    {
        __int16 hops = 0;
        VehiclePathNode *backNode = &g_vehiclePathNodes[prev];
        if ( g_vehiclePathNodeCount > 0 )
        {
            for ( ;; )
            {
                ++hops;
                distBack = backNode->dist + distBack;
                if ( g_vehiclePathAnglesUnset[0] != backNode->angles[0]
                  || g_vehiclePathAnglesUnset[1] != backNode->angles[1]
                  || g_vehiclePathAnglesUnset[2] != backNode->angles[2] )
                    break;
                __int16 pp = backNode->prevIndex;
                if ( pp >= 0 && pp != (__int16)index )
                {
                    backNode = &g_vehiclePathNodes[pp];
                    if ( hops < g_vehiclePathNodeCount )
                        continue;
                }
                goto backDone;
            }
            backAng[0] = backNode->angles[0];
            backAng[1] = backNode->angles[1];
            backAng[2] = backNode->angles[2];
        }
    }
backDone:
    {
        __int16 hops = 0;
        if ( g_vehiclePathNodeCount > 0 )
        {
            for ( ;; )
            {
                ++hops;
                if ( g_vehiclePathAnglesUnset[0] != fwdNode->angles[0]
                  || g_vehiclePathAnglesUnset[1] != fwdNode->angles[1]
                  || g_vehiclePathAnglesUnset[2] != fwdNode->angles[2] )
                    break;
                __int16 nx = fwdNode->nextIndex;
                if ( nx >= 0 && nx != (__int16)index )
                {
                    float acc = fwdNode->dist + distFwd;
                    fwdNode = &g_vehiclePathNodes[nx];
                    distFwd = acc;
                    if ( hops < g_vehiclePathNodeCount )
                        continue;
                }
                goto fwdDone;
            }
            fwdAng[0] = fwdNode->angles[0];
            fwdAng[1] = fwdNode->angles[1];
            fwdAng[2] = fwdNode->angles[2];
        }
    }
fwdDone:
    {
        const bool backUnset = ( g_vehiclePathAnglesUnset[0] == backAng[0]
                              && g_vehiclePathAnglesUnset[1] == backAng[1]
                              && g_vehiclePathAnglesUnset[2] == backAng[2] );
        const bool fwdUnset  = ( g_vehiclePathAnglesUnset[0] == fwdAng[0]
                              && g_vehiclePathAnglesUnset[1] == fwdAng[1]
                              && g_vehiclePathAnglesUnset[2] == fwdAng[2] );

        if ( backUnset && fwdUnset )
        {
            out[0] = 0.0f;
            out[1] = 0.0f;
            out[2] = 0.0f;
            return;
        }
        if ( backUnset )
        {
            out[0] = fwdAng[0];
            out[1] = fwdAng[1];
            out[2] = fwdAng[2];
            return;
        }
        if ( fwdUnset )
        {
            out[0] = backAng[0];
            out[1] = backAng[1];
            out[2] = backAng[2];
            return;
        }

        float total = distFwd + distBack;
        if ( total > 0.0f )
        {
            float f = distBack / total;
            out[0] = VehiclePath_AngleNormalize180( fwdAng[0] - backAng[0] ) * f + backAng[0];
            out[1] = VehiclePath_AngleNormalize180( fwdAng[1] - backAng[1] ) * f + backAng[1];
            out[2] = VehiclePath_AngleNormalize180( fwdAng[2] - backAng[2] ) * f + backAng[2];
        }
        else
        {
            out[0] = 0.0f;
            out[1] = 0.0f;
            out[2] = 0.0f;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5E50  VehiclePath_RotateNodeBlend — the tracer's rotate-node blend weight.
//  1 while fully inside a rotate segment, ramping out of / into one across the
//  current segment, 0 otherwise.  (The IDB shows a dead ECX parameter — the function
//  is __fastcall but only ever reads EDX; normalised away here.)
// ─────────────────────────────────────────────────────────────────────────────
static float VehiclePath_RotateNodeBlend( const VehiclePathTracer *tracer )
{
    __int16 next = g_vehiclePathNodes[tracer->nodeIndex].nextIndex;
    if ( next >= 0 )
    {
        VehiclePathNode *nextNode = &g_vehiclePathNodes[next];
        if ( g_vehiclePathNodes[tracer->nodeIndex].isRotateNode )
        {
            if ( nextNode->isRotateNode )
                return 1.0f;
            return 1.0f - tracer->frac;      // leaving a rotate run
        }
        if ( nextNode->isRotateNode )
            return tracer->frac;             // entering a rotate run
        return 0.0f;
    }
    return g_vehiclePathNodes[tracer->nodeIndex].isRotateNode ? 1.0f : 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5EE0  VehiclePath_BlendRotateAngles — blend the tracer's heading toward the
//  authored rotate-node angles.  Whichever end of the current segment is a rotate
//  node supplies the target; when BOTH are, the blend runs node->node and the
//  movement-derived heading is discarded; when neither is, nothing happens.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_BlendRotateAngles( const VehiclePathTracer *tracer, float *angles )
{
    __int16 next = g_vehiclePathNodes[tracer->nodeIndex].nextIndex;
    VehiclePathNode *node = &g_vehiclePathNodes[tracer->nodeIndex];

    if ( next < 0 )
    {
        if ( node->isRotateNode )
        {
            angles[0] = node->angles[0];
            angles[1] = node->angles[1];
            angles[2] = node->angles[2];
        }
        return;
    }

    VehiclePathNode *nextNode = &g_vehiclePathNodes[next];
    float from[3];
    float to[3];

    if ( node->isRotateNode )
    {
        if ( nextNode->isRotateNode )
        {
            from[0] = node->angles[0];
            from[1] = node->angles[1];
            from[2] = node->angles[2];
            to[0] = nextNode->angles[0];
            to[1] = nextNode->angles[1];
            to[2] = nextNode->angles[2];
        }
        else
        {
            // leaving a rotate run: node angles -> the movement-derived heading
            from[0] = node->angles[0];
            from[1] = node->angles[1];
            from[2] = node->angles[2];
            to[0] = angles[0];
            to[1] = angles[1];
            to[2] = angles[2];
        }
    }
    else
    {
        if ( !nextNode->isRotateNode )
            return;
        // entering a rotate run: the movement-derived heading -> next node's angles
        from[0] = angles[0];
        from[1] = angles[1];
        from[2] = angles[2];
        to[0] = nextNode->angles[0];
        to[1] = nextNode->angles[1];
        to[2] = nextNode->angles[2];
    }

    for ( int i = 0; i < 3; ++i )
    {
        float base  = from[i];
        float delta = VehiclePath_AngleNormalize180( to[i] - base );
        angles[i]   = VehiclePath_AngleNormalize180( delta * tracer->frac + base );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6050  VehiclePath_GetLookaheadPos — the point `speed * lookahead` units ahead
//  of the tracer along the node chain (the steering target).  Walks forward
//  consuming each segment's length; a dangling link or a zero-length segment clamps
//  to that node's own origin.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_GetLookaheadPos( const VehiclePathTracer *tracer, float *out )
{
    VehiclePathNode *node = &g_vehiclePathNodes[tracer->nodeIndex];
    float ahead = tracer->lookahead * tracer->speed;
    __int16 hops = 0;
    float remaining = ahead + g_vehiclePathNodes[tracer->nodeIndex].dist * tracer->frac;
    float along = 0.0f;

    if ( g_vehiclePathNodeCount <= 0 )
    {
        along = remaining;
    }
    else
    {
        for ( ;; )
        {
            __int16 next = node->nextIndex;
            ++hops;
            if ( next < 0 || 0.0f == node->dist )
            {
                along = 0.0f;
                goto emit;
            }
            if ( node->dist > remaining )
            {
                along = remaining;
                goto emit;
            }
            remaining = remaining - node->dist;
            node = &g_vehiclePathNodes[next];
            if ( hops >= g_vehiclePathNodeCount )
            {
                along = remaining;
                goto emit;
            }
        }
    }
emit:
    out[0] = along * node->dir[0] + node->origin[0];
    out[1] = node->dir[1] * along + node->origin[1];
    out[2] = along * node->dir[2] + node->origin[2];
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6100  VehiclePath_AdvanceToLookahead — after the tracer has been moved, find
//  which segment it is now on and where along it.  Walk forward until the tracer sits
//  BETWEEN a node and its successor along `dir` (both projections non-negative); the
//  fraction is the ratio of the two projections.  Returns 1 if `stopNode` was visited
//  during the walk (VehiclePath_DrawPath's loop terminator).
// ─────────────────────────────────────────────────────────────────────────────
static int VehiclePath_AdvanceToLookahead( VehiclePathTracer *tracer, const float *dir, __int16 stopNode )
{
    __int16 cur = tracer->nodeIndex;
    float   frac = tracer->frac;
    VehiclePathNode *node = &g_vehiclePathNodes[cur];
    int hitStop = 0;
    __int16 hops = 0;

    if ( g_vehiclePathNodeCount > 0 )
    {
        float t = 0.0f;
        for ( ;; )
        {
            ++hops;
            node = &g_vehiclePathNodes[cur];
            if ( cur == stopNode )
                hitStop = 1;
            __int16 next = node->nextIndex;
            if ( next < 0 )
                break;
            if ( 0.0f == node->dist )
                break;

            float back[3];
            back[0] = tracer->origin[0] - g_vehiclePathNodes[cur].origin[0];
            back[1] = tracer->origin[1] - g_vehiclePathNodes[cur].origin[1];
            back[2] = tracer->origin[2] - g_vehiclePathNodes[cur].origin[2];
            float dBack = dir[1] * back[1] + dir[0] * back[0] + dir[2] * back[2];

            float fwd[3];
            fwd[0] = g_vehiclePathNodes[next].origin[0] - tracer->origin[0];
            fwd[1] = g_vehiclePathNodes[next].origin[1] - tracer->origin[1];
            fwd[2] = g_vehiclePathNodes[next].origin[2] - tracer->origin[2];
            float dFwd = dir[1] * fwd[1] + dir[0] * fwd[0] + dir[2] * fwd[2];

            if ( dBack == 0.0f && 0.0f == dFwd )
                break;
            if ( dBack >= 0.0f && dFwd >= 0.0f )
            {
                t = dBack / ( dFwd + dBack );
                break;
            }
            cur = next;
            if ( hops >= g_vehiclePathNodeCount )
                goto seat;      // FAITHFUL: `frac` keeps its incoming value here, and
                                // `node` is deliberately NOT re-pointed at the new `cur`
                                // (the binary leaves ECX on the PREVIOUS node — see below).
        }
        frac = t;
    }
seat:
    tracer->nodeIndex = cur;
    // FAITHFUL QUIRK: on the hop-limit exit above, `cur` has already advanced but
    // `node` still points at the node we came from, so atEnd is computed from the
    // PREVIOUS node's link (0x4B624B reads the stale ECX).  Harmless in practice —
    // that path is only reached after following a link that was >= 0 — but transcribed
    // as-is rather than "fixed".
    tracer->atEnd = (__int16)( node->nextIndex < 0 );
    tracer->frac  = frac;

    __int16 nx = g_vehiclePathNodes[cur].nextIndex;
    if ( nx >= 0 )
        tracer->speed = ( g_vehiclePathNodes[nx].speed - g_vehiclePathNodes[cur].speed ) * frac
                      + g_vehiclePathNodes[cur].speed;
    else
        tracer->speed = g_vehiclePathNodes[cur].speed;

    nx = g_vehiclePathNodes[cur].nextIndex;
    if ( nx >= 0 )
        tracer->lookahead = frac * ( g_vehiclePathNodes[nx].lookahead - g_vehiclePathNodes[cur].lookahead )
                          + g_vehiclePathNodes[cur].lookahead;
    else
        tracer->lookahead = g_vehiclePathNodes[cur].lookahead;

    tracer->rotateBlend = VehiclePath_RotateNodeBlend( tracer );
    return hitStop;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B5150  VehiclePath_AddPathSegment — emit one path segment, merging runs of
//  near-collinear segments into a single 3D line.  The pending segment lives in the
//  file globals; a new segment whose direction matches the pending one to within
//  dot >= 0.9999 just extends the pending end (unless `forceFlush`, the last step).
//  Colour is the editor's d_savedinfo.colors[10].
//  NOTE (faithful): the LAST pending segment of a path is never flushed — the pass
//  always emits the PREVIOUS segment and then records the new one.  The final tail is
//  picked up by the forceFlush call the caller makes on the terminating step.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_AddPathSegment( const float *from, const float *to, int forceFlush )
{
    float dir[3];
    dir[0] = to[0] - from[0];
    dir[1] = to[1] - from[1];
    dir[2] = to[2] - from[2];
    Vec3Normalize_R( dir );

    if ( g_vehiclePathFirstSegment )
    {
        g_vehiclePathFirstSegment = 0;
    }
    else
    {
        float dot = g_vehiclePathSegDir[1] * dir[1]
                  + g_vehiclePathSegDir[0] * dir[0]
                  + g_vehiclePathSegDir[2] * dir[2];
        if ( dot >= 0.9999f && !forceFlush )
        {
            // collinear: just push the pending segment's end out to the new point
            g_vehiclePathSegEnd[0] = to[0];
            g_vehiclePathSegEnd[1] = to[1];
            g_vehiclePathSegEnd[2] = to[2];
            return;
        }

        GfxPointVertex verts[2];
        verts[0].xyz[0] = g_vehiclePathSegStart[0];
        verts[0].xyz[1] = g_vehiclePathSegStart[1];
        verts[0].xyz[2] = g_vehiclePathSegStart[2];
        Byte4PackPixelColor( g_qeglobals.d_savedinfo.colors[10], (GfxColor *)verts[0].color );
        verts[1].xyz[0] = g_vehiclePathSegEnd[0];
        verts[1].xyz[1] = g_vehiclePathSegEnd[1];
        verts[1].xyz[2] = g_vehiclePathSegEnd[2];
        *(unsigned int *)verts[1].color = *(unsigned int *)verts[0].color;
        R_AddCmd_Line3D( 1, 1, verts );
    }

    g_vehiclePathSegStart[0] = from[0];
    g_vehiclePathSegStart[1] = from[1];
    g_vehiclePathSegStart[2] = from[2];
    g_vehiclePathSegEnd[0] = to[0];
    g_vehiclePathSegEnd[1] = to[1];
    g_vehiclePathSegEnd[2] = to[2];
    g_vehiclePathSegDir[0] = dir[0];
    g_vehiclePathSegDir[1] = dir[1];
    g_vehiclePathSegDir[2] = dir[2];
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B52C0  VehiclePath_DrawArrow — a heading arrow at `origin` oriented by
//  `angles`, sized by the "VehArrowSize" preference.  Five local-space points scaled
//  by the size, run through the angle matrix, and drawn as two triangles: the
//  horizontal head (tip / left wing / right wing) and a vertical fin (top / tail /
//  the un-offset origin).  g_vehicle_path.cpp:153.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_DrawArrow( const float *angles, const float *origin )
{
    int arrowSize = g_PrefsDlg->vehicle_arrow_size;
    if ( arrowSize <= 0 )
        return;

    float size = (float)arrowSize;

    // 5 local-space points (the binary keeps them as one contiguous float[15]).
    float pts[5][3];
    pts[0][0] =  0.5f;  pts[0][1] =  0.0f;         pts[0][2] = 0.0f;          // tip
    pts[1][0] = -0.5f;  pts[1][1] = -0.40000001f;  pts[1][2] = 0.0f;          // right wing
    pts[2][0] = -0.5f;  pts[2][1] =  0.40000001f;  pts[2][2] = 0.0f;          // left wing
    pts[3][0] = -0.5f;  pts[3][1] =  0.0f;         pts[3][2] = 0.40000001f;   // fin top
    pts[4][0] = -0.5f;  pts[4][1] =  0.0f;         pts[4][2] = 0.0f;          // fin tail

    // orientation as a mat4x3: rows 0-2 = the angle axis, row 3 = the world origin.
    float mtx[4][3];
    AnglesToAxis( angles, mtx );
    mtx[3][0] = origin[0];
    mtx[3][1] = origin[1];
    mtx[3][2] = origin[2];

    float world[5][3];
    for ( int i = 0; i < 5; ++i )
    {
        pts[i][0] = size * pts[i][0];
        pts[i][1] = pts[i][1] * size;
        pts[i][2] = size * pts[i][2];
        MatrixTransformVector43( pts[i], mtx, world[i] );
    }

    GfxColor col;
    Byte4PackPixelColor( g_qeglobals.d_savedinfo.colors[10], &col );

    const orientation_t *ident = (const orientation_t *)world_orient_matrix;
    const unsigned int  *cp    = (const unsigned int *)&col;

    GfxPointVertex verts[12];
    int vertCount;
    vertCount = R_Add3DLine( verts, ident, world[0], world[1], cp, 1, 0,         12 );
    vertCount = R_Add3DLine( verts, ident, world[1], world[2], cp, 1, vertCount, 12 );
    vertCount = R_Add3DLine( verts, ident, world[2], world[0], cp, 1, vertCount, 12 );
    vertCount = R_Add3DLine( verts, ident, world[3], world[4], cp, 1, vertCount, 12 );
    vertCount = R_Add3DLine( verts, ident, world[4], origin,   cp, 1, vertCount, 12 );
    vertCount = R_Add3DLine( verts, ident, origin,   world[3], cp, 1, vertCount, 12 );

    // KEEP_VERBOSE: level-1 assert (iassert/vassert hardcode level 0 — converting
    // would downgrade it).
    if ( vertCount != 12 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\g_vehicle_path.cpp",
                153, 1, "%s", "vertCount == ARRAY_COUNT( verts )" );

    if ( vertCount )
        R_AddCmd_Line3D( (short)( vertCount / 2 ), 1, verts );
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6AC0  VehiclePath_InitTracer — seed a tracer at the start of `index`'s
//  segment.  The two embedded nodes are default-constructed (targetname NULL), which
//  is what makes VehiclePath_StepTracer's node write-back a no-op in the editor.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_InitTracer( __int16 index, VehiclePathTracer *tracer )
{
    float blend = 0.0f;
    tracer->frac      = 0.0f;
    tracer->nodeIndex = index;
    tracer->atEnd     = 0;
    tracer->speed     = g_vehiclePathNodes[index].speed;
    tracer->lookahead = g_vehiclePathNodes[index].lookahead;
    if ( g_vehiclePathNodes[index].isRotateNode )
        blend = 1.0f;
    tracer->rotateBlend = blend;
    tracer->origin[0] = g_vehiclePathNodes[index].origin[0];
    tracer->origin[1] = g_vehiclePathNodes[index].origin[1];
    tracer->origin[2] = g_vehiclePathNodes[index].origin[2];
    tracer->angles[0] = g_vehiclePathNodes[index].angles[0];
    tracer->angles[1] = g_vehiclePathNodes[index].angles[1];
    tracer->angles[2] = g_vehiclePathNodes[index].angles[2];
    tracer->lookaheadPos[0] = g_vehiclePathNodes[index].origin[0];
    tracer->lookaheadPos[1] = g_vehiclePathNodes[index].origin[1];
    tracer->lookaheadPos[2] = g_vehiclePathNodes[index].origin[2];
    VehiclePathNode_Init( &tracer->nodeOverride, -1 );
    VehiclePathNode_Init( &tracer->nodeSaved,    -1 );   // the binary reuses the DX it
                                                         // already loaded (still -1)
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6B50  VehiclePath_StepTracer — one 50 ms simulation step: steer toward the
//  lookahead point, advance by speed*0.05, re-seat on the node chain and blend the
//  rotate-node heading in.  Returns AdvanceToLookahead's "visited stopNode" flag.
//
//  The two VehiclePathNode_Copy calls are the ENGINE's per-vehicle node override: it
//  installs `nodeOverride` into the shared table for the duration of the step (keyed
//  by that node's targetname) and writes `nodeSaved` back afterwards.  BOTH lookups
//  use nodeOverride.targetname — faithful; that is what the binary does.  In the
//  editor both embedded nodes come out of VehiclePath_InitTracer default-constructed
//  with a NULL targetname, so VehiclePath_FindNode returns -1 immediately and neither
//  copy ever runs; the table is never mutated by the preview.
// ─────────────────────────────────────────────────────────────────────────────
static int VehiclePath_StepTracer( VehiclePathTracer *tracer, __int16 stopNode )
{
    int result = 0;

    if ( tracer->atEnd )
        return 0;

    __int16 slot = VehiclePath_FindNode( 0, tracer->nodeOverride.targetname );
    if ( slot >= 0 )
        VehiclePathNode_Copy( &g_vehiclePathNodes[slot], &tracer->nodeOverride );

    VehiclePath_GetLookaheadPos( tracer, tracer->lookaheadPos );

    float vec[3];
    vec[0] = tracer->lookaheadPos[0] - tracer->origin[0];
    vec[1] = tracer->lookaheadPos[1] - tracer->origin[1];
    vec[2] = tracer->lookaheadPos[2] - tracer->origin[2];

    if ( Vec3Normalize_R( vec ) <= 0.0f )
    {
        tracer->atEnd = 1;
    }
    else
    {
        vectoangles( tracer->angles, (int)(intptr_t)vec );
        tracer->angles[0] = VehiclePath_AngleNormalize180( tracer->angles[0] );
        tracer->angles[1] = VehiclePath_AngleNormalize180( tracer->angles[1] );
        tracer->angles[2] = VehiclePath_AngleNormalize180( tracer->angles[2] );

        float step = tracer->speed * 0.05f;      // dbl_6F44D0 = (double)0.05f — 50 ms
        tracer->origin[0] = step * vec[0] + tracer->origin[0];
        tracer->origin[1] = step * vec[1] + tracer->origin[1];
        tracer->origin[2] = step * vec[2] + tracer->origin[2];

        result = VehiclePath_AdvanceToLookahead( tracer, vec, stopNode );
        VehiclePath_BlendRotateAngles( tracer, tracer->angles );
    }

    slot = VehiclePath_FindNode( 0, tracer->nodeOverride.targetname );
    if ( slot >= 0 )
        VehiclePathNode_Copy( &g_vehiclePathNodes[slot], &tracer->nodeSaved );

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6370  VehiclePath_DrawPath — run a tracer from `start` until it stops, drawing
//  the curve it traces and a heading arrow every "VehArrowTime" milliseconds of
//  simulated time.  Bails with a console warning after 50000 steps (a path that loops
//  without ever revisiting its start node).
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_DrawPath( const VehiclePathTracer *start )
{
    int arrowTime = g_PrefsDlg->vehicle_arrow_time;
    int steps   = 0;
    int elapsed = 0;
    int maxTime = arrowTime;
    if ( arrowTime < 0 )
        maxTime = 0;

    VehiclePathTracer prev;
    VehiclePathTracer cur;
    memcpy( &prev, start, sizeof( prev ) );

    int done = 0;
    int stopNode = -1;
    memcpy( &cur, start, sizeof( cur ) );
    g_vehiclePathFirstSegment = 1;

    for ( ;; )
    {
        if ( steps + 1 > 50000 )
            break;

        __int16 startIndex = start->nodeIndex;
        // Arm the loop terminator once the walk has actually left the start node.
        if ( prev.nodeIndex != startIndex )
            stopNode = (unsigned __int16)startIndex;

        memcpy( &prev, &cur, sizeof( prev ) );
        int hitStop = VehiclePath_StepTracer( &cur, (__int16)stopNode );
        if ( cur.atEnd || hitStop )
            done = 1;

        VehiclePath_AddPathSegment( prev.origin, cur.origin, done );

        if ( elapsed <= maxTime )
        {
            elapsed += 50;
        }
        else
        {
            VehiclePath_DrawArrow( cur.angles, cur.origin );
            elapsed = 0;
        }

        if ( done )
            return;

        steps = steps + 1;
    }

    // The binary's R_Warn ignores its warning-type argument entirely and forwards
    // (fmt, va) to the console handler; 16 is the literal it passes.
    R_Warn( 16, "WARNING: Invalid vehicle path.  Possible infinite loop\n" );
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B64B0  VehiclePath_LinkNodes — turn the collected target/targetname strings
//  into the next/prev index links, then derive each node's direction, segment length
//  and heading, then resolve the inherited speed / lookahead / angles.
//    pass 1: nextIndex = the node named by our "target";
//            prevIndex = the first node whose "target" names US.
//            A link onto ourselves is dropped (-1).
//    pass 2: dir = normalize(next.origin - origin), dist = |that|; non-rotate nodes
//            take their heading from the direction.
//    pass 3: resolve speed/lookahead/angles, normalise the angles, and break the
//            forward link of any node that ended up with a non-positive speed or
//            lookahead (a terminal node), defaulting those to 1.
// ─────────────────────────────────────────────────────────────────────────────
static void VehiclePath_LinkNodes()
{
    __int16 count = g_vehiclePathNodeCount;
    __int16 i = 0;

    if ( g_vehiclePathNodeCount > 0 )
    {
        VehiclePathNode *node = &g_vehiclePathNodes[0];
        __int16 last = 0;
        do
        {
            if ( node->target )
                node->nextIndex = VehiclePath_FindNode( 0, node->target );

            __int16 j = 0;
            for ( ;; )
            {
                last = i;
                if ( i != j && !strcmp( node->targetname, g_vehiclePathNodes[j].target ) )
                    break;
                if ( ++j >= g_vehiclePathNodeCount )
                    goto selfLinkCheck;
            }
            node->prevIndex = j;
selfLinkCheck:
            if ( node->nextIndex == i )
                node->nextIndex = -1;
            if ( node->prevIndex == i )
                node->prevIndex = -1;
            count = g_vehiclePathNodeCount;
            ++node;
            ++i;
        }
        while ( (__int16)( last + 1 ) < g_vehiclePathNodeCount );

        if ( g_vehiclePathNodeCount > 0 )
        {
            VehiclePathNode *n = &g_vehiclePathNodes[0];
            unsigned __int16 left = (unsigned __int16)g_vehiclePathNodeCount;
            do
            {
                __int16 next = n->nextIndex;
                if ( next >= 0 )
                {
                    n->dir[0] = g_vehiclePathNodes[next].origin[0] - n->origin[0];
                    n->dir[1] = g_vehiclePathNodes[next].origin[1] - n->origin[1];
                    n->dir[2] = g_vehiclePathNodes[next].origin[2] - n->origin[2];
                    n->dist   = Vec3Normalize_R( n->dir );
                    if ( !n->isRotateNode )
                        vectoangles( n->angles, (int)(intptr_t)n->dir );
                }
                ++n;
                --left;
            }
            while ( left );
            count = g_vehiclePathNodeCount;
        }
    }

    int k = 0;
    if ( count > 0 )
    {
        VehiclePathNode *n = &g_vehiclePathNodes[0];
        do
        {
            n->speed     = VehiclePath_ResolveNodeSpeed( (__int16)k );
            n->lookahead = VehiclePath_ResolveNodeLookahead( (__int16)k );
            if ( n->isRotateNode )
                VehiclePath_ResolveNodeAngles( n->angles, k );

            n->angles[0] = VehiclePath_AngleNormalize180( n->angles[0] );
            n->angles[1] = VehiclePath_AngleNormalize180( n->angles[1] );
            n->angles[2] = VehiclePath_AngleNormalize180( n->angles[2] );

            if ( n->speed <= 0.0f || n->lookahead <= 0.0f )
                n->nextIndex = -1;
            if ( n->nextIndex < 0 )
            {
                if ( n->speed <= 0.0f )
                    n->speed = 1.0f;
                if ( n->lookahead <= 0.0f )
                    n->lookahead = 1.0f;
            }
            ++k;
            ++n;
        }
        while ( (__int16)k < g_vehiclePathNodeCount );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6710  VehiclePath_AddNode — the overlay entry point, called from
//  DrawConnectionLinks (0x40C9F0, the tail of both XY_Draw and Cam_Draw).
//  Only draws when an info_vehicle_node / info_vehicle_node_rotate brush is selected;
//  rebuilds the whole node table from the live entity instances, links it, then walks
//  every path START node (spawnflags&1) that can reach the selected node and
//  simulates it.  Asserts at g_vehicle_path.cpp:1143-1145.
// ─────────────────────────────────────────────────────────────────────────────
void VehiclePath_AddNode()
{
    if ( ( g_qeglobals.d_savedinfo.d_xyShowFlags & 4 ) != 0 )
        return;                                   // connections hidden

    if ( selected_brushes.next == &selected_brushes )
        return;                                   // nothing selected

    // The binary takes the sentinel's PREV link as its representative brush
    // (`mov edi, selected_brushes` loads the dword AT the sentinel = selbrush_t.prev,
    // NOT `selected_brushes_next` which is the +4 field it just compared).  It stashes
    // it in a stack slot and restores it after the rebuild loop clobbers EDI; the port
    // never clobbers `brush`, so no restore is needed.
    selbrush_t *brush = selected_brushes.prev;

    iassert( brush );                                   // g_vehicle_path.cpp:1143
    iassert( brush->owner );                            // g_vehicle_path.cpp:1144
    iassert( brush->owner->def == brush->def->owner );  // g_vehicle_path.cpp:1145

    eclass_t *eclass = brush->owner->def->eclass;
    // The dword at &fixedsize (+0x8, bool + 3 pad) gates non-point classes out.
    if ( !*(int *)&eclass->fixedsize
      || ( I_stricmp( eclass->name, "info_vehicle_node" )
        && I_stricmp( eclass->name, "info_vehicle_node_rotate" ) ) )
    {
        return;
    }

    // ── rebuild the node table from the live entity instances ────────────────
    // (the IDB keeps the walk cursor in BOTH a register and a stack slot, so hex-rays
    // shows a phantom second `next` variable — they are always the same entity.)
    g_vehiclePathNodeCount = 0;

    for ( entity_s *ent = entityInsts.next; ent != &entityInsts; ent = ent->next )
    {
        eclass_t *ecls = ent->def->eclass;
        if ( !*(int *)&ecls->fixedsize
          || ( I_stricmp( ecls->name, "info_vehicle_node" )
            && I_stricmp( ecls->name, "info_vehicle_node_rotate" ) )
          || FilterBrush( ent->brushes.ownerNext, 0 )
          || ( ent->brushes.ownerNext->brushFlags & 2 ) != 0 )
        {
            continue;
        }

        if ( g_vehiclePathNodeCount >= MAX_VEHICLE_NODES )
        {
            Sys_Printf( "Warning: Max vehicle Nodes hit [%d]\n", MAX_VEHICLE_NODES );
            break;                                  // the binary abandons the rebuild here
        }

        VehiclePathNode *node = &g_vehiclePathNodes[g_vehiclePathNodeCount];
        VehiclePathNode_Init( node, g_vehiclePathNodeCount );

        if ( !I_stricmp( ecls->name, "info_vehicle_node_rotate" ) )
            node->isRotateNode = 1;

        node->targetname  = ValueForKey2( (int)(intptr_t)ent->def, "targetname" );
        node->target      = ValueForKey2( (int)(intptr_t)ent->def, "target" );
        node->spawnflags1 = Entity_GetIntValueForKey( (int)(intptr_t)ent->def, "spawnflags" ) & 1;

        // A node without a non-empty targetname is unusable — the slot is left for the
        // next entity (the count is NOT advanced, so the record is overwritten).
        if ( node->targetname && *node->targetname )
        {
            const char *speed = ValueForKey2( (int)(intptr_t)ent->def, "speed" );
            if ( speed && *speed )
                node->speed = (float)( atof( speed ) * 17.6f );   // mph -> units/sec

            const char *lookahead = ValueForKey2( (int)(intptr_t)ent->def, "lookahead" );
            if ( lookahead && *lookahead )
                node->lookahead = (float)atof( lookahead );

            entity_s_def *def = ent->def;
            node->origin[0] = def->origin[0];
            node->origin[1] = def->origin[1];
            node->origin[2] = def->origin[2];
            if ( HasKeyValuePair( def, "angles" ) )
                Entity_GetVec3ForKey( def, node->angles, "angles" );
            ++g_vehiclePathNodeCount;
        }
    }

    VehiclePath_LinkNodes();

    // The selected entity's own targetname, via a raw epair walk (case-insensitive,
    // "" — the binary's `zero` global — when absent).  NOT ValueForKey2.
    const char *value = "";
    for ( epair_t *ep = brush->owner->def->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, "targetname" ) )
        {
            value = ep->value;
            break;
        }
    }

    __int16 selNode = VehiclePath_FindNode( brush->owner->def->origin, value );
    if ( selNode >= 0 )
    {
        for ( __int16 i = 0; i < g_vehiclePathNodeCount; ++i )
        {
            if ( g_vehiclePathNodes[i].spawnflags1 )       // path START nodes only
            {
                if ( i == selNode || VehiclePath_IsNodeReachable( i, selNode ) )
                {
                    VehiclePathTracer tracer;
                    VehiclePath_InitTracer( i, &tracer );
                    VehiclePath_DrawPath( &tracer );
                }
            }
        }
    }
}
