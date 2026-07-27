#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\winding.h
// Declarations for winding.cpp — ported from GtkRadiant 1.6 radiant/winding.h.
// All types (plane_t, winding_t, vec3_t, etc.) come from stdafx.h → qe3.h → qedefs.h.

// Global winding allocation counter (defined in winding.cpp).
// Tracks live winding_t malloc/free calls for leak detection.
extern int g_windingAlloc;

// ── Plane utilities ──────────────────────────────────────────────────────────────
// Returns true if the planes are equal within normal/dist epsilon.
bool Plane_Equal( plane_t *a, plane_t *b, int flip );

// Returns true (non-zero) if three points define a valid plane; writes *plane.
int Plane_FromPoints( vec3_t p1, vec3_t p2, vec3_t p3, plane_t *plane );

// Returns true if the two points are equal within epsilon.
int Point_Equal( vec3_t p1, vec3_t p2, float epsilon );

// ── Winding allocation ────────────────────────────────────────────────────────────
// Allocates a new winding with room for 'points' vertices (numpoints=0 on return).
winding_t *Winding_Alloc( int points );

// Frees the winding (decrements g_windingAlloc).
void Winding_Free( winding_t *w );

// Clones a winding (full copy including numpoints and all point data).
winding_t *Winding_Clone( winding_t *w );

// ── Winding construction ──────────────────────────────────────────────────────────
// Creates a base winding for 'plane' clipped to the [mins,maxs] AABB (common/polylib.cpp
// 0x4D7BC0). Arg order matches the binary: (maxs, mins, plane). Returns NULL if the
// plane does not intersect the box in ≥3 points.
winding_t *Winding_BaseForPlane( vec3_t maxs, vec3_t mins, plane_t *plane );

// Returns a new winding with points in reverse order.
winding_t *Winding_Reverse( winding_t *w );

// Removes point at index 'point' from the winding in-place.
void Winding_RemovePoint( winding_t *w, int point );

// Returns a new winding with 'point' inserted at position 'spot'.
winding_t *Winding_InsertPoint( winding_t *w, vec3_t point, int spot );

// ── Winding queries ────────────────────────────────────────────────────────────────
// Returns true if fewer than 3 edges exceed EDGE_LENGTH (0.2 units).
int Winding_IsTiny( winding_t *w );

// Returns true if any vertex coordinate exceeds the world coordinate bounds.
int Winding_IsHuge( winding_t *w );

// Returns true if the planes of two windings are concave with respect to each other.
int Winding_PlanesConcave( winding_t *w1, winding_t *w2,
                            vec3_t normal1, vec3_t normal2,
                            float dist1, float dist2 );

// ── Winding splitting / merging ───────────────────────────────────────────────────
// Splits the winding with a plane; both front and back are returned (not freed).
// CoD4Radiant's "Winding_Clip" implements Winding_SplitEpsilon semantics.
// Returns the IDA side code: 0=all-front, 1=all-back, 2=all-on-plane (coplanar), 3=split.
// (CSG_FaceVisible gates on ==2; most callers ignore the return.)
int Winding_Clip( winding_t *in, vec3_t normal, double dist,
                  float epsilon, winding_t **front, winding_t **back );

// Merges two windings sharing a common edge if the result is convex.
// Returns NULL if they cannot be merged. Originals are not freed.
winding_t *Winding_TryMerge( winding_t *f1, winding_t *f2,
                               vec3_t planenormal, int keep );

// ── Winding geometry ──────────────────────────────────────────────────────────────
// Computes the best-fit plane normal and distance for a winding.
void Winding_Plane( winding_t *w, vec3_t normal, double *dist );

// Returns the total surface area of the winding.
float Winding_Area( winding_t *w );

// Computes the axis-aligned bounding box of the winding.
void Winding_Bounds( winding_t *w, vec3_t mins, vec3_t maxs );

// Returns true if 'point' lies inside the winding (on the plane).
int Winding_PointInside( winding_t *w, plane_t *plane, vec3_t point, float epsilon );

// Returns true if the line segment p1-p2 intersects the winding.
int Winding_VectorIntersect( winding_t *w, plane_t *plane,
                              vec3_t p1, vec3_t p2, float epsilon );
