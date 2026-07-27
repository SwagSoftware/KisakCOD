#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Light-region CSG builder API.

struct winding_t;

// The light's region face node (48 bytes; see primarylights_region.cpp banner).
struct rface_t;

// The light descriptor passed to the merge driver: { int class; float p[8] } where
// p[0..2]=origin, p[3..5]=cone axis, p[6]=radius, p[7]=cosHalfFov.  Built by the
// CamWnd region chain (sub_406CE0) from Entity_Light's classification + cone params.
struct lightDesc_t
{
    int   cls;     // +0x00  Entity_Light classification (2=cone, 3=ambient/no-cone)
    float p[8];    // +0x04  [0..2]=origin, [3..5]=dir, [6]=radius, [7]=cosHalfFov
};

// sub_4DAA70 — build a region face from a world-space winding `w` (light origin =
// desc->p[1..3] passed as the bare float[8] `a3`, radius = a3[7]) and link it onto
// *outList.  `exterior` = the shadow-caster "is exterior" flag.
void sub_4DAA70( const winding_t *w, rface_t **outList, const float *a3, int exterior );

// sub_4DAD20 — add the light's bounding-cube exterior faces to *outList. desc = the
// region descriptor; *(desc+28) = radius.
void sub_4DAD20( rface_t **outList, const float *desc );

// sub_4DD260 — decompose *faceList into convex sub-regions, build a GfxLightRegionHull
// for each, append to outHulls[], return the hull count.  desc = light descriptor.
int  sub_4DD260( rface_t **faceList, const lightDesc_t *desc, void **outHulls );

// Winding_Clip_real_ (polylib 0x4D83B0) — destructive in-place clip keeping the FRONT
// side (dist > epsilon).  *inout freed+replaced (NULL if all-front clipped away).
void Winding_Clip_real_( winding_t **inout, const float *normal, float dist, float epsilon );
