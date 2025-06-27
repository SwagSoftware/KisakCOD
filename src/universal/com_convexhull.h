#pragma once


unsigned int __cdecl Com_ConvexHull(float (*points)[2], unsigned int pointCount, float (*hull)[2]);
void __cdecl Com_InitialHull(
    const float (*points)[2],
    unsigned int *pointOrder,
    unsigned int pointCount,
    unsigned int *hullOrder);
void __cdecl Com_SwapHullPoints(unsigned int *pointOrder, unsigned int pointIndex0, unsigned int pointIndex1);
unsigned int __cdecl Com_GrowInitialHull(
    const float (*points)[2],
    unsigned int *pointOrder,
    unsigned int pointCount,
    unsigned int *hullOrder);
unsigned int __cdecl Com_AddPointToHull(
    unsigned int pointIndex,
    unsigned int newIndex,
    unsigned int *hullOrder,
    unsigned int hullPointCount);
unsigned int __cdecl Com_RecursivelyGrowHull(
    const float (*points)[2],
    unsigned int *pointOrder,
    unsigned int pointCount,
    unsigned int firstIndex,
    unsigned int secondIndex,
    unsigned int *hullOrder,
    unsigned int hullPointCount);
void __cdecl Com_TranslatePoints(float (*points)[2], unsigned int pointCount, float *offset);
