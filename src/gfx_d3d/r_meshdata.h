#pragma once
#include "r_gfx.h"
#include "rb_backend.h"

struct GfxMeshGlobals // sizeof=0x180
{                                       // ...
    GfxQuadMeshData fullSceneViewMesh[4]; // ...
    GfxMeshData spotShadowClearMeshData[4]; // ...
    GfxMeshData sunShadowClearMeshData[2]; // ...
};

char __cdecl R_ReserveMeshIndices(GfxMeshData *mesh, int indexCount, r_double_index_t **indicesOut);
char __cdecl R_ReserveMeshVerts(GfxMeshData *mesh, int vertCount, uint16_t *baseVertex);
uint8_t *__cdecl R_GetMeshVerts(GfxMeshData *mesh, uint16_t baseVertex);
void __cdecl R_ResetMesh(GfxMeshData *mesh);
void __cdecl R_SetQuadMeshData(
    GfxMeshData *mesh,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    uint32_t color);
void __cdecl R_SetQuadMesh(
    GfxQuadMeshData *quadMesh,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    uint32_t color);
void __cdecl R_DrawQuadMesh(GfxCmdBufContext context, const Material *material, GfxMeshData *quadMesh);
void __cdecl R_BeginMeshVerts(GfxMeshData* mesh);


extern GfxMeshGlobals gfxMeshGlob;