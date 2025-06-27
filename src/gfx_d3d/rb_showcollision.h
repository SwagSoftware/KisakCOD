#pragma once
#include "r_gfx.h"


void __cdecl TRACK_rb_showcollision();
void __cdecl RB_ShowCollision(const GfxViewParms *viewParms);
void __cdecl BuildFrustumPlanes(const GfxViewParms *viewParms, cplane_s *frustumPlanes);
void __cdecl RB_DrawCollisionPoly(int numPoints, float (*points)[3], const float *colorFloat);
void __cdecl RB_SetPolyVert(float *xyz, GfxColor color, int tessVertIndex);