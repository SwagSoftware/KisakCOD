#pragma once

// LWSS ADD

int __cdecl dCollideBoxTriangleList(
    const unsigned __int16 *indices,
    const float (*verts)[3],
    int triCount,
    const float *boxR,
    const float *boxPos,
    const float *boxHalfExtents,
    const float *bodyCenter,
    int Flags,
    struct dContactGeom *Contacts,
    int Stride);