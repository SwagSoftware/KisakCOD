#include "r_draw_xmodel.h"


void __cdecl R_DrawXModelRigidModelSurf(GfxCmdBufContext context, XSurface *xsurf)
{
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    int vertexOffset0; // [esp+20h] [ebp-18h] BYREF
    IDirect3DVertexBuffer9 *vb0; // [esp+24h] [ebp-14h] BYREF
    IDirect3DIndexBuffer9 *ib; // [esp+28h] [ebp-10h] BYREF
    GfxDrawPrimArgs args; // [esp+2Ch] [ebp-Ch] BYREF

    if (!xsurf)
        MyAssertHandler(".\\r_draw_xmodel.cpp", 160, 0, "%s", "xsurf");
    args.vertexCount = xsurf->vertCount;
    args.triCount = xsurf->triCount;
    g_frameStatsCur.geoIndexCount += 3 * args.triCount;
    DB_GetIndexBufferAndBase(xsurf->zoneHandle, xsurf->triIndices, (void **)&ib, &args.baseIndex);
    if (!ib)
        MyAssertHandler(".\\r_draw_xmodel.cpp", 172, 0, "%s", "ib");
    if (context.state->prim.indexBuffer != ib)
        R_ChangeIndices(&context.state->prim, ib);
    DB_GetVertexBufferAndOffset(xsurf->zoneHandle, xsurf->verts0, (void **)&vb0, &vertexOffset0);
    R_SetStreamSource(&context.state->prim, vb0, vertexOffset0, 0x20u);
    R_SetupPassPerPrimArgs(context);
    R_DrawIndexedPrimitive(&context.state->prim, &args);
    g_primStats->staticIndexCount += 3 * args.triCount;
    g_primStats->staticVertexCount += args.vertexCount;
    if (r_showTess->current.integer)
    {
        ActiveWorldMatrix = R_GetActiveWorldMatrix(context.source);
        RB_ShowTess(context, ActiveWorldMatrix->matrices.matrix[0].m[3], "XMRigid", colorWhite);
    }
}

