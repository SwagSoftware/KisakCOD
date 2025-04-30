#pragma once
#include "r_material.h"
#include <universal/q_shared.h>

void __cdecl Vec3UnpackUnitVec(PackedUnitVec in, float *out);
void __cdecl Vec2UnpackTexCoords(PackedTexCoords in, float *out);


void  R_SkinXModelCmd(_WORD *data);
void __cdecl R_SkinXSurfaceSkinned(
    const struct XSurface *xsurf,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *skinVerticesOut);
void __cdecl R_SkinXSurfaceWeight(
    const GfxPackedVertex *inVerts,
    const struct XSurfaceVertexInfo *vertexInfo,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *outVerts);
void __cdecl R_SkinXSurfaceWeight0(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut);
void __cdecl R_SkinXSurfaceWeight1(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut);
void __cdecl R_SkinXSurfaceWeight2(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut);
void __cdecl R_SkinXSurfaceWeight3(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut);
void __cdecl R_SkinXSurfaceRigid(
    const XSurface *surf,
    int totalVertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertices);
void __cdecl R_MultiplySkelMat(const DObjSkelMat *mat0, const DObjSkelMat *mat1, DObjSkelMat *out);
