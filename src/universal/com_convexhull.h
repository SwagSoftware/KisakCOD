#pragma once
#include <cstdint>

uint32_t __cdecl Com_ConvexHull(float (*points)[2], uint32_t pointCount, float (*hull)[2]);

#ifdef KISAK_RADIANT
// CoD4Radiant editor (common/polylib.cpp): like Com_ConvexHull but returns the hull
// as point INDICES (into hullOrder) instead of coords. Translates points in place.
uint32_t __cdecl Com_ConvexHullIndices(float (*points)[2], uint32_t pointCount, uint32_t *hullOrder);
#endif

