#pragma once
#include "rb_backend.h"


union $B667868682928995E3CB40CE466D3989 // sizeof=0x4
{                                       // ...
    GfxPackedVertex *skinnedVert;
    int oldSkinnedCachedOffset;
};

struct __declspec(align(4)) GfxModelSurfaceInfo // sizeof=0xC
{                                       // ...
    const DObjAnimMat *baseMat;
    unsigned __int8 boneIndex;
    unsigned __int8 boneCount;
    unsigned __int16 gfxEntIndex;
    unsigned __int16 lightingHandle;
    // padding byte
    // padding byte
};

struct GfxModelSkinnedSurface // sizeof=0x18
{                                       // ...
    int skinnedCachedOffset;
    XSurface *xsurf;
    GfxModelSurfaceInfo info;
    $B667868682928995E3CB40CE466D3989 ___u3;
};


void __cdecl RB_ShowTess(GfxCmdBufContext context, const float *center, const char *tessName, const float *color);
unsigned int __cdecl R_TessCodeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetVertexDeclTypeNormal(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType);
void __cdecl R_TessCodeMeshList_AddCodeMeshArgs(
    GfxCmdBufSourceState *source,
    const GfxBackEndData *data,
    const FxCodeMeshData *codeMesh);
unsigned int __cdecl R_TessMarkMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessParticleCloudList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetParticleCloudConstants(GfxCmdBufSourceState *source, const GfxParticleCloud *cloud);
void __cdecl RB_Vec3DirWorldToView(const GfxCmdBufSourceState *source, const float *worldDir, float *viewDir);
void __cdecl RB_CreateParticleCloud2dAxis(const GfxParticleCloud *cloud, const float *viewUp, float (*viewAxis)[2]);
unsigned int __cdecl R_TessXModelSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
void __cdecl R_DrawXModelSkinnedModelSurf(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf);
void __cdecl R_DrawXModelSkinnedCached(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf);
unsigned int __cdecl R_TessXModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessStaticModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessTrianglesList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetVertexDeclTypeWorld(GfxCmdBufState *state);
unsigned int __cdecl R_TessBModel(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
