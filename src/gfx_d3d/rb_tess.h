#pragma once
#include "rb_backend.h"
#include "r_dobj_skin.h"

struct GfxReadCmdBuf // sizeof=0x4
{                                       // ...
    const uint32_t *primDrawSurfPos; // ...
};

struct GfxStaticModelPreTessSurf_s // sizeof=0x4
{                                       // ...
    uint8_t surfIndex;
    uint8_t lod;
    uint16_t cachedIndex;
};
union GfxStaticModelPreTessSurf // sizeof=0x4
{                                       // ...
    GfxStaticModelPreTessSurf_s fields;
    uint32_t packed;
};

//union $B667868682928995E3CB40CE466D3989 // sizeof=0x4
//{                                       // ...
//    GfxPackedVertex *skinnedVert;
//    int oldSkinnedCachedOffset;
//};


void __cdecl RB_ShowTess(GfxCmdBufContext context, const float *center, const char *tessName, const float *color);
uint32_t __cdecl R_TessCodeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetVertexDeclTypeNormal(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType);
void __cdecl R_TessCodeMeshList_AddCodeMeshArgs(
    GfxCmdBufSourceState *source,
    const GfxBackEndData *data,
    const FxCodeMeshData *codeMesh);
void __cdecl R_SetParticleCloudConstants(GfxCmdBufSourceState *source, const GfxParticleCloud *cloud);
void __cdecl RB_Vec3DirWorldToView(const GfxCmdBufSourceState *source, const float *worldDir, float *viewDir);
void __cdecl RB_CreateParticleCloud2dAxis(const GfxParticleCloud *cloud, const float *viewUp, float (*viewAxis)[2][2]);
void __cdecl R_DrawXModelSkinnedUncached(GfxCmdBufContext context, XSurface *xsurf, GfxPackedVertex *skinnedVert);
void __cdecl R_DrawXModelSkinnedModelSurf(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf);
void __cdecl R_DrawXModelSkinnedCached(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf);
void __cdecl R_SetVertexDeclTypeWorld(GfxCmdBufState *state);

uint32_t __cdecl R_TessTrianglesList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessCodeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessMarkMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessParticleCloudList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessXModelSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessXModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessXModelRigidSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessStaticModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessStaticModelSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessStaticModelPreTessList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessStaticModelCachedList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessTrianglesPreTessList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
uint32_t __cdecl R_TessBModel(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);



extern GfxScaledPlacement s_manualObjectPlacement;