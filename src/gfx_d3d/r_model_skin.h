#pragma once
#include "r_material.h"
#include <universal/q_shared.h>

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


// r_model_skin_sse
// This isn't used anyway currently, causing issues with clang as well
#if 0
void __cdecl R_SkinXSurfaceSkinnedSse(
    const XSurface *xsurf,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *skinVertNormalIn,
    GfxPackedVertexNormal *skinVertNormalOut,
    GfxPackedVertex *skinVerticesOut);
#endif