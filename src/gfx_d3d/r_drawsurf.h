#pragma once

#include "fxprimitives.h"

#include "r_gfx.h"


#define MTL_TECHFLAG_NEEDS_RESOLVED_SCENE 2
#define MTL_TECHFLAG_NEEDS_RESOLVED_POST_SUN 1

#define MTL_SORT_OBJECT_ID_BITS 16 //0x10000 // 65536

enum $7289DBEFE9BA94617DD3EA143BDD93C8 : __int32
{
    MTL_PREPASS_STANDARD = 0x0,
    MTL_PREPASS_ALPHA = 0x1,
    MTL_PREPASS_FLOATZ = 0x2,
    MTL_PREPASS_NONE = 0x3,
    MTL_PREPASS_TYPECOUNT = 0x4,
};

enum DrawSurfType : __int32 // LWSS: (not a real enum name)
{
    DRAW_SURF_CAMERA_LIT_BEGIN = 0x0,
    DRAW_SURF_BSP_CAMERA_LIT = 0x0,
    DRAW_SURF_SMODEL_CAMERA_LIT = 0x1,
    DRAW_SURF_ENT_CAMERA_LIT = 0x2,
    DRAW_SURF_CAMERA_LIT_END = 0x3,
    DRAW_SURF_CAMERA_DECAL_BEGIN = 0x3,
    DRAW_SURF_BSP_CAMERA_DECAL = 0x3,
    DRAW_SURF_SMODEL_CAMERA_DECAL = 0x4,
    DRAW_SURF_ENT_CAMERA_DECAL = 0x5,
    DRAW_SURF_FX_CAMERA_LIT = 0x6,
    DRAW_SURF_FX_CAMERA_LIT_AUTO = 0x7,
    DRAW_SURF_FX_CAMERA_LIT_DECAL = 0x8,
    DRAW_SURF_CAMERA_DECAL_END = 0x9,
    DRAW_SURF_CAMERA_EMISSIVE_BEGIN = 0x9,
    DRAW_SURF_BSP_CAMERA_EMISSIVE = 0x9,
    DRAW_SURF_SMODEL_CAMERA_EMISSIVE = 0xA,
    DRAW_SURF_ENT_CAMERA_EMISSIVE = 0xB,
    DRAW_SURF_FX_CAMERA_EMISSIVE = 0xC,
    DRAW_SURF_FX_CAMERA_EMISSIVE_AUTO = 0xD,
    DRAW_SURF_FX_CAMERA_EMISSIVE_DECAL = 0xE,
    DRAW_SURF_CAMERA_EMISSIVE_END = 0xF,

    DRAW_SURF_SUNSHADOW_0_BEGIN = 0xF,
    DRAW_SURF_BSP_SUNSHADOW_0 = 0xF,
    DRAW_SURF_SMODEL_SUNSHADOW_0 = 0x10,
    DRAW_SURF_ENT_SUNSHADOW_0 = 0x11,

    DRAW_SURF_SUNSHADOW_1_BEGIN = 0x12,
    DRAW_SURF_BSP_SUNSHADOW_1 = 0x12,
    DRAW_SURF_SMODEL_SUNSHADOW_1 = 0x13,
    DRAW_SURF_ENT_SUNSHADOW_1 = 0x14,

    DRAW_SURF_SPOTSHADOW_0_BEGIN = 0x15,
    DRAW_SURF_BSP_SPOTSHADOW_0 = 0x15,
    DRAW_SURF_SMODEL_SPOTSHADOW_0 = 0x16,
    DRAW_SURF_ENT_SPOTSHADOW_0 = 0x17,

    DRAW_SURF_SPOTSHADOW_1_BEGIN = 0x18,
    DRAW_SURF_BSP_SPOTSHADOW_1 = 0x18,
    DRAW_SURF_SMODEL_SPOTSHADOW_1 = 0x19,
    DRAW_SURF_ENT_SPOTSHADOW_1 = 0x1A,

    DRAW_SURF_SPOTSHADOW_2_BEGIN = 0x1B,
    DRAW_SURF_BSP_SPOTSHADOW_2 = 0x1B,
    DRAW_SURF_SMODEL_SPOTSHADOW_2 = 0x1C,
    DRAW_SURF_ENT_SPOTSHADOW_2 = 0x1D,

    DRAW_SURF_SPOTSHADOW_3_BEGIN = 0x1E,
    DRAW_SURF_BSP_SPOTSHADOW_3 = 0x1E,
    DRAW_SURF_SMODEL_SPOTSHADOW_3 = 0x1F,
    DRAW_SURF_ENT_SPOTSHADOW_3 = 0x20,

    DRAW_SURF_SHADOW_COOKIE = 0x21,
    DRAW_SURF_TYPE_COUNT = 0x22,
};

GfxDrawSurf R_GetMaterialInfoPacked(const Material* material);

void R_EndMeshVerts(GfxMeshData* mesh);
void R_EndCodeMeshVerts();
void R_BeginCodeMeshVerts();
void R_EndMarkMeshVerts();

char __cdecl R_ReserveMeshIndices(GfxMeshData* mesh, int indexCount, r_double_index_t** indicesOut);

char __cdecl R_ReserveCodeMeshIndices(int indexCount, r_double_index_t** indicesOut);
char __cdecl R_ReserveCodeMeshVerts(int vertCount, unsigned __int16* baseVertex);
char __cdecl R_ReserveCodeMeshArgs(int argCount, unsigned int* argOffsetOut);

void __cdecl R_AddCodeMeshDrawSurf(
    Material* material,
    r_double_index_t* indices,
    unsigned int indexCount,
    unsigned int argOffset,
    unsigned int argCount,
    const char* fxName);
float (*__cdecl R_GetCodeMeshArgs(unsigned int argOffset))[4];
GfxPackedVertex* __cdecl R_GetCodeMeshVerts(unsigned __int16 baseVertex);

char __cdecl R_ReserveMarkMeshVerts(int vertCount, unsigned __int16 *baseVertex);
char __cdecl R_ReserveMarkMeshIndices(int indexCount, r_double_index_t** indicesOut);

void __cdecl R_BeginMarkMeshVerts();
void __cdecl R_AddMarkMeshDrawSurf(
    Material *material,
    const GfxMarkContext *context,
    unsigned __int16 *indices,
    unsigned int indexCount);

void __cdecl R_SortDrawSurfs(GfxDrawSurf *drawSurfList, int surfCount);
GfxWorldVertex *__cdecl R_GetMarkMeshVerts(unsigned __int16 baseVertex);
GfxDrawSurf __cdecl R_GetWorldDrawSurf(GfxSurface *worldSurf);
void __cdecl R_SortWorldSurfaces();
char __cdecl R_AddParticleCloudDrawSurf(volatile unsigned int cloudIndex, Material *material);