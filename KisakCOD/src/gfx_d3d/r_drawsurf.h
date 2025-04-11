#pragma once

#include "fxprimitives.h"

#include "r_gfx.h"

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