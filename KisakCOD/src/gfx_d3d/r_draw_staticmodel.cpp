#include "r_draw_staticmodel.h"
#include "rb_stats.h"


void __cdecl R_DrawStaticModelDrawSurfLightingNonOptimized(
    GfxStaticModelDrawStream *drawStream,
    GfxCmdBufContext context)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-28h]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+4h] [ebp-24h]
    const unsigned __int16 *list; // [esp+8h] [ebp-20h]
    unsigned int smodelCount; // [esp+Ch] [ebp-1Ch]
    unsigned int index; // [esp+10h] [ebp-18h]
    XSurface *xsurf; // [esp+14h] [ebp-14h]
    GfxDrawPrimArgs args; // [esp+18h] [ebp-10h] BYREF
    unsigned __int16 lightingHandle; // [esp+24h] [ebp-4h]

    xsurf = drawStream->localSurf;
    R_SetupStaticModelPrim(xsurf, &args, &context.state->prim);
    R_SetStaticModelVertexBuffer(&context.state->prim, xsurf);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    for (index = 0; index < smodelCount; ++index)
    {
        smodelDrawInst = &smodelDrawInsts[list[index]];
        R_SetReflectionProbe(context, smodelDrawInst->reflectionProbeIndex);
        R_DrawStaticModelDrawSurfPlacement(smodelDrawInst, context.source);
        lightingHandle = smodelDrawInst->lightingHandle;
        R_SetModelLightingCoordsForSource(lightingHandle, context.source);
        R_SetupPassPerPrimArgs(context);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
}

void __cdecl R_DrawStaticModelSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxStaticModelDrawStream drawStream; // [esp+0h] [ebp-20h] BYREF
    XSurface *surf; // [esp+1Ch] [ebp-4h] BYREF

    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    while (R_GetNextStaticModelSurf(&drawStream, &surf))
        R_DrawStaticModelDrawSurfLightingNonOptimized(&drawStream, context);
}

int __cdecl R_GetNextStaticModelSurf(GfxStaticModelDrawStream *drawStream, XSurface **outSurf)
{
    XSurface *xsurf; // [esp+0h] [ebp-Ch]
    const unsigned int *primDrawSurfPos; // [esp+4h] [ebp-8h]

    drawStream->smodelCount = *drawStream->primDrawSurfPos++;
    if (!drawStream->smodelCount)
        return 0;
    primDrawSurfPos = drawStream->primDrawSurfPos;
    drawStream->primDrawSurfPos += ((drawStream->smodelCount + 1) >> 1) + 1;
    xsurf = (XSurface *)*primDrawSurfPos;
    drawStream->smodelList = (const unsigned __int16 *)(primDrawSurfPos + 1);
    drawStream->localSurf = xsurf;
    g_frameStatsCur.geoIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
    if (!g_primStats)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 226, 0, "%s", "g_primStats");
    g_primStats->dynamicIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
    g_primStats->dynamicVertexCount += drawStream->smodelCount * xsurf->vertCount;
    *outSurf = xsurf;
    return 1;
}

void __cdecl R_DrawStaticModelSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxStaticModelDrawStream drawStream; // [esp+0h] [ebp-20h] BYREF
    XSurface *surf; // [esp+1Ch] [ebp-4h] BYREF

    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    while (R_GetNextStaticModelSurf(&drawStream, &surf))
        R_DrawStaticModelDrawSurfNonOptimized(&drawStream, context);
}

void __cdecl R_DrawStaticModelDrawSurfNonOptimized(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+4h] [ebp-20h]
    const unsigned __int16 *list; // [esp+8h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+Ch] [ebp-18h]
    unsigned int index; // [esp+10h] [ebp-14h]
    XSurface *xsurf; // [esp+14h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+18h] [ebp-Ch] BYREF

    xsurf = drawStream->localSurf;
    R_SetupStaticModelPrim(xsurf, &args, &context.state->prim);
    R_SetStaticModelVertexBuffer(&context.state->prim, xsurf);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    for (index = 0; index < smodelCount; ++index)
    {
        R_DrawStaticModelDrawSurfPlacement(&smodelDrawInsts[list[index]], context.source);
        R_SetupPassPerPrimArgs(context);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
}

void __cdecl R_SetStaticModelVertexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf)
{
    IDirect3DVertexBuffer9 *vb; // [esp+18h] [ebp-8h] BYREF
    int vertexOffset; // [esp+1Ch] [ebp-4h] BYREF

    if (!xsurf)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 245, 0, "%s", "xsurf");
    if (xsurf->deformed || !useFastFile->current.enabled)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 246, 0, "%s", "XSurfaceHasOptimizedVertices( xsurf )");
    DB_GetVertexBufferAndOffset(xsurf->zoneHandle, xsurf->verts0, (void **)&vb, &vertexOffset);
    if (!vb)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 248, 0, "%s", "vb");
    R_SetStreamSource(primState, vb, vertexOffset, 0x20u);
}

void __cdecl R_DrawStaticModelDrawSurfPlacement(
    const GfxStaticModelDrawInst *smodelDrawInst,
    GfxCmdBufSourceState *source)
{
    float v2; // [esp+2Ch] [ebp-68h]
    float v3; // [esp+30h] [ebp-64h]
    float v4; // [esp+38h] [ebp-5Ch]
    float v5; // [esp+3Ch] [ebp-58h]
    float v6; // [esp+40h] [ebp-54h]
    float v7; // [esp+48h] [ebp-4Ch]
    float v8; // [esp+4Ch] [ebp-48h]
    float v9; // [esp+50h] [ebp-44h]
    GfxCmdBufSourceState *matrix; // [esp+5Ch] [ebp-38h]
    float origin[3]; // [esp+60h] [ebp-34h] BYREF
    float scale; // [esp+6Ch] [ebp-28h]
    float axis[3][3]; // [esp+70h] [ebp-24h] BYREF

    v2 = smodelDrawInst->placement.axis[0][1];
    v3 = smodelDrawInst->placement.axis[0][2];
    v4 = smodelDrawInst->placement.axis[1][0];
    v5 = smodelDrawInst->placement.axis[1][1];
    v6 = smodelDrawInst->placement.axis[1][2];
    v7 = smodelDrawInst->placement.axis[2][0];
    v8 = smodelDrawInst->placement.axis[2][1];
    v9 = smodelDrawInst->placement.axis[2][2];
    axis[0][0] = smodelDrawInst->placement.axis[0][0];
    axis[0][1] = v2;
    axis[0][2] = v3;
    axis[1][0] = v4;
    axis[1][1] = v5;
    axis[1][2] = v6;
    axis[2][0] = v7;
    axis[2][1] = v8;
    axis[2][2] = v9;
    scale = smodelDrawInst->placement.scale;
    matrix = R_GetActiveWorldMatrix(source);
    Vec3Sub(smodelDrawInst->placement.origin, source->eyeOffset, origin);
    MatrixSet44((float (*)[4])matrix, origin, axis, scale);
}

