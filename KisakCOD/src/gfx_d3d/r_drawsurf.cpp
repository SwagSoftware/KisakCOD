#include <qcommon/qcommon.h>
#include <universal/q_shared.h>

#include "r_buffers.h"

#include "r_drawsurf.h"
#include "r_draw_method.h"

#include "r_meshdata.h"

#include "r_dvars.h"

#include "r_scene.h"

static bool g_processCodeMesh;
static bool g_processMarkMesh;

char __cdecl R_ReserveCodeMeshIndices(int indexCount, r_double_index_t** indicesOut)
{
    if (!g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 673, 0, "%s", "g_processCodeMesh");
    if (R_ReserveMeshIndices(&frontEndDataOut->codeMesh, indexCount, indicesOut))
        return 1;
    R_WarnOncePerFrame(R_WARN_MAX_CODE_INDS);
    return 0;
}

char __cdecl R_ReserveCodeMeshVerts(int vertCount, unsigned __int16* baseVertex)
{
    if (!g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 725, 0, "%s", "g_processCodeMesh");
    if (R_ReserveMeshVerts(&frontEndDataOut->codeMesh, vertCount, baseVertex))
        return 1;
    R_WarnOncePerFrame(R_WARN_MAX_CODE_VERTS);
    return 0;
}

char __cdecl R_ReserveCodeMeshArgs(int argCount, unsigned int* argOffsetOut)
{
    volatile int oldArgCount; // [esp+8h] [ebp-4h]

    if (!g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 750, 0, "%s", "g_processCodeMesh");
    if (argCount < 0)
        MyAssertHandler(".\\r_drawsurf.cpp", 751, 0, "%s\n\t(argCount) = %i", "(argCount >= 0)", argCount);
    if (!argOffsetOut)
        MyAssertHandler(".\\r_drawsurf.cpp", 752, 0, "%s", "argOffsetOut");
    oldArgCount = frontEndDataOut->codeMeshArgsCount;
    if ((unsigned int)(argCount + oldArgCount) < 0x100)
    {
        *argOffsetOut = oldArgCount;
        InterlockedExchange(&frontEndDataOut->codeMeshArgsCount, argCount + oldArgCount);
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_CODE_ARGS);
        return 0;
    }
}

byte R_GetMaterialSortKey(const Material* material)
{
    return material->info.sortKey;
}

GfxDrawSurf R_GetMaterialInfoPacked(const Material* material)
{
    return material->info.drawSurf;
}

// KISAKTODO THIS MIGHT BE WRONG?? FIELD IS A RANDOM ASS GUESS, IDA IS NOT BEING HELPFUL
GfxDrawSurf* __cdecl R_AllocFxDrawSurf(unsigned int region)
{
    int drawSurfCount; // [esp+8h] [ebp-4h]

    drawSurfCount = InterlockedIncrement(&scene.drawSurfCount[region]) - 1;
    if (drawSurfCount < scene.maxDrawSurfCount[region])
        return &scene.drawSurfs[region][drawSurfCount];
    InterlockedDecrement(&scene.drawSurfCount[region]);
    R_WarnOncePerFrame(R_WARN_MAX_FX_DRAWSURFS);
    return 0;
}

void __cdecl R_AddCodeMeshDrawSurf(
    Material* material,
    r_double_index_t* indices,
    unsigned int indexCount,
    unsigned int argOffset,
    unsigned int argCount,
    const char* fxName)
{
    int packed_high; // ecx
    unsigned int v7; // edx
    int MaterialSortKey; // [esp+20h] [ebp-28h]
    FxCodeMeshData* localCodeMesh; // [esp+30h] [ebp-18h]
    GfxDrawSurf* drawSurf; // [esp+34h] [ebp-14h]
    int region; // [esp+3Ch] [ebp-Ch]
    unsigned int codeMeshIndex; // [esp+44h] [ebp-4h]

    if (!indexCount)
        MyAssertHandler(".\\r_drawsurf.cpp", 422, 0, "%s", "indexCount");
    if (!g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 423, 0, "%s", "g_processCodeMesh");
    if (rgp.sortedMaterials[(material->info.drawSurf.packed >> 29) & 0x7FF] != material)
        MyAssertHandler(
            ".\\r_drawsurf.cpp",
            426,
            0,
            "%s",
            "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material");
    if (Material_GetTechnique(material, gfxDrawMethod.emissiveTechType) && (material->info.gameFlags & 2) == 0)
    {
        codeMeshIndex = InterlockedExchangeAdd(&frontEndDataOut->codeMeshCount, 1);
        if (codeMeshIndex < 0x800)
        {
            localCodeMesh = &frontEndDataOut->codeMeshes[codeMeshIndex];
            localCodeMesh->triCount = indexCount / 3;
            localCodeMesh->indices = (unsigned __int16*)indices;
            if (!argCount && argOffset)
                MyAssertHandler(".\\r_drawsurf.cpp", 467, 0, "%s", "argCount != 0 || argOffset == 0");
            if (argOffset >= 0x100)
                MyAssertHandler(
                    ".\\r_drawsurf.cpp",
                    468,
                    0,
                    "%s\n\t(argOffset) = %i",
                    "(argOffset >= 0 && argOffset < 256)",
                    argOffset);
            localCodeMesh->argCount = argCount;
            if (localCodeMesh->argCount != argCount)
                MyAssertHandler(
                    ".\\r_drawsurf.cpp",
                    470,
                    0,
                    "%s\n\t(argCount) = %i",
                    "(localCodeMesh->argCount == argCount)",
                    argCount);
            localCodeMesh->argOffset = argOffset;
            if (localCodeMesh->argOffset != argOffset)
                MyAssertHandler(
                    ".\\r_drawsurf.cpp",
                    472,
                    0,
                    "%s\n\t(argOffset) = %i",
                    "(localCodeMesh->argOffset == argOffset)",
                    argOffset);
            MaterialSortKey = R_GetMaterialSortKey(material);
            region = (MaterialSortKey == 48) + (MaterialSortKey != 24 ? 0 : 2) + 12;
            drawSurf = R_AllocFxDrawSurf(region);
            if (drawSurf)
            {
                drawSurf->fields = R_GetMaterialInfoPacked(material).fields;
                // packed_high = HIDWORD(drawSurf->packed);

                drawSurf->fields.objectId = codeMeshIndex;
                
                // LODWORD(drawSurf->packed) = (unsigned __int16)codeMeshIndex | *(_DWORD*)&drawSurf->fields & 0xFFFF0000;
                // HIDWORD(drawSurf->packed) = packed_high;
                
                v7 = HIDWORD(drawSurf->packed) & 0xFFC3FFFF | 0x280000;
                
                LODWORD(drawSurf->packed) = drawSurf->packed;
                HIDWORD(drawSurf->packed) = v7;
                
                // LODWORD(drawSurf->packed) = drawSurf->packed;
                // HIDWORD(drawSurf->packed) = HIDWORD(drawSurf->packed);

                if (((drawSurf->packed >> 40) & 3) != 3)
                    MyAssertHandler(".\\r_drawsurf.cpp", 491, 1, "%s", "localDrawSurf->fields.prepass == MTL_PREPASS_NONE");
                if ((MaterialSortKey == 48) + (MaterialSortKey != 24 ? 0 : 2)
                    && drawSurf != scene.drawSurfs[region]
                    && ((drawSurf->packed >> 54) & 0x3F) < ((drawSurf[-1].packed >> 54) & 0x3F))
                {
                    MyAssertHandler(
                        ".\\r_drawsurf.cpp",
                        493,
                        1,
                        "%s\n\t(region) = %i",
                        "((region == DRAW_SURF_FX_CAMERA_EMISSIVE) || (drawSurf == scene.drawSurfs[region]) || (drawSurf->fields.prim"
                        "arySortKey >= (drawSurf - 1)->fields.primarySortKey))",
                        region);
                }
            }
        }
        else
        {
            R_WarnOncePerFrame(R_WARN_GFX_CODE_MESH_LIMIT);
        }
    }
    else
    {
        if (!r_fullbright)
            MyAssertHandler(".\\r_drawsurf.cpp", 432, 0, "%s", "r_fullbright");
        if (!r_fullbright->current.enabled)
            R_WarnOncePerFrame(R_WARN_NONEMISSIVE_FX_MATERIAL, material->info.name, fxName);
    }
}

float (*__cdecl R_GetCodeMeshArgs(unsigned int argOffset))[4]
{
    if (!g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 797, 0, "%s", "g_processCodeMesh");
    return (float (*)[4])frontEndDataOut->codeMeshArgs[argOffset];
}

GfxPackedVertex* __cdecl R_GetCodeMeshVerts(unsigned __int16 baseVertex)
{
    if (!g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 804, 0, "%s", "g_processCodeMesh");
    return (GfxPackedVertex*)R_GetMeshVerts(&frontEndDataOut->codeMesh, baseVertex);
}

void R_EndMeshVerts(GfxMeshData* mesh)
{
    if (mesh->vb.verts)
    {
        R_UnlockVertexBuffer(mesh->vb.buffer);
        mesh->vb.verts = 0;
    }
}

void R_EndCodeMeshVerts()
{
    g_processCodeMesh = 0;
    R_EndMeshVerts(&frontEndDataOut->codeMesh);
}

void R_BeginCodeMeshVerts()
{
    if (g_processCodeMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 616, 0, "%s", "!g_processCodeMesh");
    g_processCodeMesh = 1;
    R_BeginMeshVerts(&frontEndDataOut->codeMesh);
}

void R_EndMarkMeshVerts()
{
    g_processMarkMesh = 0;
    R_EndMeshVerts(&frontEndDataOut->markMesh);
}

char __cdecl R_ReserveMarkMeshIndices(int indexCount, r_double_index_t** indicesOut)
{
    if (!g_processMarkMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 772, 0, "%s", "g_processMarkMesh");
    if (R_ReserveMeshIndices(&frontEndDataOut->markMesh, indexCount, indicesOut))
        return 1;
    R_WarnOncePerFrame(R_WARN_MAX_MARK_INDS);
    return 0;
}

char __cdecl R_ReserveMarkMeshVerts(int vertCount, unsigned __int16 *baseVertex)
{
    if (!g_processMarkMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 784, 0, "%s", "g_processMarkMesh");
    if (R_ReserveMeshVerts(&frontEndDataOut->markMesh, vertCount, baseVertex))
        return 1;
    R_WarnOncePerFrame(R_WARN_MAX_MARK_VERTS);
    return 0;
}

void __cdecl R_BeginMarkMeshVerts()
{
    if (g_processMarkMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 649, 0, "%s", "!g_processMarkMesh");
    g_processMarkMesh = 1;
    R_BeginMeshVerts(&frontEndDataOut->markMesh);
}

void __cdecl R_AddMarkMeshDrawSurf(
    Material *material,
    const GfxMarkContext *context,
    unsigned __int16 *indices,
    unsigned int indexCount)
{
    int packed_high; // edx
    unsigned __int64 v5; // rax
    int v6; // ecx
    unsigned __int64 v7; // rax
    int v8; // ecx
    unsigned int v9; // ecx
    GfxDrawSurf *drawSurf; // [esp+40h] [ebp-14h]
    int region; // [esp+48h] [ebp-Ch]
    unsigned int markMeshIndex; // [esp+4Ch] [ebp-8h]
    FxMarkMeshData *markMesh; // [esp+50h] [ebp-4h]

    if (!g_processMarkMesh)
        MyAssertHandler(".\\r_drawsurf.cpp", 507, 0, "%s", "g_processMarkMesh");
    if (rgp.sortedMaterials[(material->info.drawSurf.packed >> 29) & 0x7FF] != material)
        MyAssertHandler(
            ".\\r_drawsurf.cpp",
            508,
            0,
            "%s",
            "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material");
    if (Material_GetTechnique(material, (MaterialTechniqueType)gfxDrawMethod.litTechType[11][0]))
    {
        markMeshIndex = InterlockedExchangeAdd(&frontEndDataOut->markMeshCount, 1);
        if (markMeshIndex < 0x600)
        {
            markMesh = &frontEndDataOut->markMeshes[markMeshIndex];
            markMesh->triCount = indexCount / 3;
            markMesh->indices = indices;
            markMesh->modelIndex = context->modelIndex;
            markMesh->modelTypeAndSurf = context->modelTypeAndSurf;
            if (material->info.sortKey < 0x18u)
                MyAssertHandler(
                    ".\\r_drawsurf.cpp",
                    530,
                    0,
                    "%s\n\t(material->info.sortKey) = %i",
                    "(material->info.sortKey >= 24)",
                    material->info.sortKey);
            region = (material->info.sortKey == 48) + (material->info.sortKey != 24 ? 0 : 2) + 6;
            drawSurf = R_AllocFxDrawSurf(region);
            if (drawSurf)
            {
                drawSurf->fields = (GfxDrawSurfFields)material->info.drawSurf;

                // packed_high = HIDWORD(drawSurf->packed);
                // *(_DWORD *)&drawSurf->fields = (unsigned __int16)markMeshIndex | *(_DWORD *)&drawSurf->fields & 0xFFFF0000;
                // HIDWORD(drawSurf->packed) = packed_high;

                drawSurf->fields.objectId = markMeshIndex;

                v5 = (unsigned __int64)(context->lmapIndex & 0x1F) << 24;
                v6 = HIDWORD(v5) | HIDWORD(drawSurf->packed);
                *(_DWORD *)&drawSurf->fields = v5 | *(_DWORD *)&drawSurf->fields & 0xE0FFFFFF;
                
                HIDWORD(drawSurf->packed) = v6;
                HIDWORD(v5) = HIDWORD(drawSurf->packed) & 0xFFC3FFFF | 0x2C0000;
                
                // drawSurf->fields = drawSurf->fields;
                HIDWORD(drawSurf->packed) = HIDWORD(v5);

                drawSurf->fields.reflectionProbeIndex = context->reflectionProbeIndex;
                drawSurf->fields.primaryLightIndex = context->primaryLightIndex;

                // v7 = (unsigned __int64)context->reflectionProbeIndex << 16;
                // v8 = HIDWORD(v7) | HIDWORD(drawSurf->packed);
                // *(_DWORD *)&drawSurf->fields = v7 | *(_DWORD *)&drawSurf->fields & 0xFF00FFFF;
                // HIDWORD(drawSurf->packed) = v8;
                // v9 = (context->primaryLightIndex << 10) | HIDWORD(drawSurf->packed) & 0xFFFC03FF;
                // *(_DWORD *)&drawSurf->fields = drawSurf->fields;
                // HIDWORD(drawSurf->packed) = v9;

                if (((drawSurf->packed >> 40) & 3) != 3)
                    MyAssertHandler(".\\r_drawsurf.cpp", 546, 1, "%s", "drawSurf->fields.prepass == MTL_PREPASS_NONE");
                if ((unsigned __int8)BYTE2(*(_DWORD *)&drawSurf->fields) != (unsigned __int64)context->reflectionProbeIndex)
                    MyAssertHandler(
                        ".\\r_drawsurf.cpp",
                        547,
                        1,
                        "drawSurf->fields.reflectionProbeIndex == context->reflectionProbeIndex\n\t%i, %i",
                        (unsigned __int8)BYTE2(*(_DWORD *)&drawSurf->fields),
                        context->reflectionProbeIndex);
                if (region != 6
                    && drawSurf != scene.drawSurfs[region]
                    && ((drawSurf->packed >> 54) & 0x3F) < ((drawSurf[-1].packed >> 54) & 0x3F))
                {
                    MyAssertHandler(
                        ".\\r_drawsurf.cpp",
                        548,
                        1,
                        "%s\n\t(region) = %i",
                        "((region == DRAW_SURF_FX_CAMERA_LIT) || (drawSurf == scene.drawSurfs[region]) || (drawSurf->fields.primarySo"
                        "rtKey >= (drawSurf - 1)->fields.primarySortKey))",
                        region);
                }
            }
        }
        else
        {
            R_WarnOncePerFrame(R_WARN_GFX_MARK_MESH_LIMIT);
        }
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_NONLIGHTMAP_MARK_MATERIAL);
    }
}