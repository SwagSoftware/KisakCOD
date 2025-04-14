#include "r_meshdata.h"
#include "r_shade.h"
#include "r_state.h"
#include "r_drawsurf.h"
#include "r_utils.h"
#include "rb_stats.h"


//struct GfxMeshGlobals gfxMeshGlob 85b93690     gfx_d3d : r_meshdata.obj

GfxMeshGlobals gfxMeshGlob;

char __cdecl R_ReserveMeshIndices(GfxMeshData *mesh, int indexCount, r_double_index_t **indicesOut)
{
    unsigned int usedCodeMeshIndexCount; // [esp+0h] [ebp-4h]

    if (indexCount < 0)
        MyAssertHandler(".\\r_meshdata.cpp", 48, 0, "%s\n\t(indexCount) = %i", "(indexCount >= 0)", indexCount);
    if ((indexCount & 1) != 0)
        MyAssertHandler(".\\r_meshdata.cpp", 49, 0, "%s\n\t(indexCount) = %i", "(!(indexCount & 1))", indexCount);
    if (!indicesOut)
        MyAssertHandler(".\\r_meshdata.cpp", 50, 0, "%s", "indicesOut");
    if (mesh->indexCount > mesh->totalIndexCount)
        MyAssertHandler(".\\r_meshdata.cpp", 51, 0, "%s", "mesh->indexCount <= mesh->totalIndexCount");
    usedCodeMeshIndexCount = mesh->indexCount;
    if (indexCount + mesh->indexCount > mesh->totalIndexCount)
        return 0;
    mesh->indexCount = indexCount + usedCodeMeshIndexCount;
    *indicesOut = (r_double_index_t *)&mesh->indices[usedCodeMeshIndexCount];
    if (((unsigned int)*indicesOut & 3) != 0)
        MyAssertHandler(
            ".\\r_meshdata.cpp",
            67,
            0,
            "%s\n\t((uint)(*indicesOut)) = %i",
            "(!((uint)(*indicesOut) & 3))",
            *indicesOut);
    return 1;
}

char __cdecl R_ReserveMeshVerts(GfxMeshData *mesh, int vertCount, unsigned __int16 *baseVertex)
{
    volatile unsigned int usedCodeMeshVertBytes; // [esp+8h] [ebp-8h]

    if (vertCount < 0)
        MyAssertHandler(".\\r_meshdata.cpp", 80, 0, "%s\n\t(vertCount) = %i", "(vertCount >= 0)", vertCount);
    if (!baseVertex)
        MyAssertHandler(".\\r_meshdata.cpp", 81, 0, "%s", "baseVertex");
    usedCodeMeshVertBytes = mesh->vb.used;
    mesh->vb.used = mesh->vertSize * vertCount + usedCodeMeshVertBytes;
    if (mesh->vb.used > mesh->vb.total)
        return 0;
    *baseVertex = usedCodeMeshVertBytes / mesh->vertSize;
    return 1;
}

unsigned __int8 *__cdecl R_GetMeshVerts(GfxMeshData *mesh, unsigned __int16 baseVertex)
{
    return &mesh->vb.verts[mesh->vertSize * baseVertex];
}

void __cdecl R_ResetMesh(GfxMeshData *mesh)
{
    mesh->vb.used = 0;
    mesh->indexCount = 0;
}

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
    unsigned int color)
{
    float v10; // [esp+1Ch] [ebp-14h]
    float v11; // [esp+20h] [ebp-10h]
    GfxVertex *verts; // [esp+24h] [ebp-Ch]
    r_double_index_t *indices; // [esp+28h] [ebp-8h]

    if (!mesh)
        MyAssertHandler(".\\r_meshdata.cpp", 120, 0, "%s", "mesh");
    if (mesh->vertSize != 32)
        MyAssertHandler(".\\r_meshdata.cpp", 121, 0, "mesh->vertSize == sizeof( GfxVertex )\n\t%i, %i", mesh->vertSize, 32);
    if (mesh->vb.total != 128)
        MyAssertHandler(
            ".\\r_meshdata.cpp",
            122,
            0,
            "mesh->vb.total == sizeof( GfxVertex ) * 4\n\t%i, %i",
            mesh->vb.total,
            128);
    if (mesh->totalIndexCount != 6)
        MyAssertHandler(".\\r_meshdata.cpp", 123, 0, "mesh->totalIndexCount == 6\n\t%i, %i", mesh->totalIndexCount, 6);
    R_BeginMeshVerts(mesh);
    indices = (r_double_index_t *)mesh->indices;
    *indices = (r_double_index_t)3;
    indices[1] = (r_double_index_t)131074;
    indices[2] = (r_double_index_t)0x10000;
    verts = (GfxVertex *)mesh->vb.verts;
    R_SetVertex2d(verts, x, y, s0, t0, color);
    v11 = x + w;
    R_SetVertex2d(verts + 1, v11, y, s1, t0, color);
    v10 = y + h;
    R_SetVertex2d(verts + 2, v11, v10, s1, t1, color);
    R_SetVertex2d(verts + 3, x, v10, s0, t1, color);
    R_EndMeshVerts(mesh);
    mesh->indexCount = 6;
    mesh->vb.used = 128;
}

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
    unsigned int color)
{
    if (!quadMesh)
        MyAssertHandler(".\\r_meshdata.cpp", 161, 0, "%s", "quadMesh");
    quadMesh->x = x;
    quadMesh->y = y;
    quadMesh->width = w;
    quadMesh->height = h;
    R_SetQuadMeshData(&quadMesh->meshData, x, y, w, h, s0, t0, s1, t1, color);
}

void __cdecl R_DrawQuadMesh(GfxCmdBufContext context, const Material *material, GfxMeshData *quadMesh)
{
    GfxViewport viewport; // [esp+0h] [ebp-1Ch] BYREF
    GfxDrawPrimArgs args; // [esp+10h] [ebp-Ch] BYREF

    if (R_BeginMaterial(context.state, material, TECHNIQUE_UNLIT))
    {
        context.state->prim.vertDeclType = VERTDECL_GENERIC;
        R_Set2D(context.source);
        if (context.source->viewportIsDirty)
        {
            R_GetViewport(context.source, &viewport);
            R_SetViewport(context.state, &viewport);
            R_UpdateViewport(context.source, &viewport);
        }
        R_SetupPass(context, 0);
        R_UpdateVertexDecl(context.state);
        R_SetupPassCriticalPixelShaderArgs(context);
        R_SetMeshStream(context.state, quadMesh);
        args.vertexCount = 4;
        args.triCount = 2;
        args.baseIndex = R_SetIndexData(&context.state->prim, (unsigned __int8 *)quadMesh->indices, 2);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_TrackPrims(context.state, GFX_PRIM_STATS_CODE);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
        g_primStats = 0;
    }
}

