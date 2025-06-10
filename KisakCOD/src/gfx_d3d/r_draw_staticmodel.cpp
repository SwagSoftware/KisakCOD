#include "r_draw_staticmodel.h"
#include "rb_stats.h"
#include <database/database.h>
#include "r_state.h"
#include "r_model_lighting.h"
#include "r_shade.h"
#include "r_draw_bsp.h"
#include "r_utils.h"
#include "r_staticmodelcache.h"
#include "rb_tess.h"
#include "r_xsurface.h"


void __cdecl R_SetupStaticModelPrim(XSurface *xsurf, GfxDrawPrimArgs *args, GfxCmdBufPrimState *primState)
{
    IDirect3DIndexBuffer9 *ib; // [esp+10h] [ebp-4h] BYREF

    iassert(xsurf);
    args->vertexCount = xsurf->vertCount;
    args->triCount = xsurf->triCount;
    //if (!useFastFile->current.enabled)
    //    MyAssertHandler(".\\r_draw_staticmodel.cpp", 266, 0, "%s", "XSurfaceHasOptimizedIndices()");
    DB_GetIndexBufferAndBase(xsurf->zoneHandle, xsurf->triIndices, (void **)&ib, &args->baseIndex);
    iassert(ib);
    if (primState->indexBuffer != ib)
        R_ChangeIndices(primState, ib);
}

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

    iassert(g_primStats);
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

    iassert(xsurf);
    //if (xsurf->deformed || !useFastFile->current.enabled)
    //    MyAssertHandler(".\\r_draw_staticmodel.cpp", 246, 0, "%s", "XSurfaceHasOptimizedVertices( xsurf )");
    DB_GetVertexBufferAndOffset(xsurf->zoneHandle, (uint8*)xsurf->verts0, (void **)&vb, &vertexOffset);
    iassert(vb);
    R_SetStreamSource(primState, vb, vertexOffset, 0x20u);
}

void __cdecl R_DrawStaticModelDrawSurfPlacement(
    const GfxStaticModelDrawInst *smodelDrawInst,
    GfxCmdBufSourceState *source)
{
    GfxCmdBufSourceState *matrix; // [esp+5Ch] [ebp-38h]
    float origin[3]; // [esp+60h] [ebp-34h] BYREF
    float scale; // [esp+6Ch] [ebp-28h]
    mat3x3 axis; // [esp+70h] [ebp-24h] BYREF

    axis[0][0] = smodelDrawInst->placement.axis[0][0]; // LWSS: is it really necessary to copy this? prob not. remove later when working
    axis[0][1] = smodelDrawInst->placement.axis[0][1];
    axis[0][2] = smodelDrawInst->placement.axis[0][2];
    axis[1][0] = smodelDrawInst->placement.axis[1][0];
    axis[1][1] = smodelDrawInst->placement.axis[1][1];
    axis[1][2] = smodelDrawInst->placement.axis[1][2];
    axis[2][0] = smodelDrawInst->placement.axis[2][0];
    axis[2][1] = smodelDrawInst->placement.axis[2][1];
    axis[2][2] = smodelDrawInst->placement.axis[2][2];

    scale = smodelDrawInst->placement.scale;
    matrix = R_GetActiveWorldMatrix(source);
    Vec3Sub(smodelDrawInst->placement.origin, source->eyeOffset, origin);
    MatrixSet44(*(mat4x4*)matrix, origin, axis, scale);
}

void __cdecl R_SetupCachedStaticModelLighting(GfxCmdBufSourceState *source)
{
    source->input.consts[57][0] = 0.0;
    source->input.consts[57][1] = 0.0;
    source->input.consts[57][2] = 0.5;
    source->input.consts[57][3] = 1.0;
    R_DirtyCodeConstant(source, 57);
}

int __cdecl R_GetNextStaticModelCachedSurf(GfxStaticModelDrawStream *drawStream)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-Ch]
    XSurface *xsurf; // [esp+8h] [ebp-4h]

    drawStream->smodelCount = *drawStream->primDrawSurfPos++;
    if (!drawStream->smodelCount)
        return 0;
    xsurf = (XSurface *)*drawStream->primDrawSurfPos++;
    drawStream->smodelList = (const unsigned short*)drawStream->primDrawSurfPos;
    drawStream->primDrawSurfPos += (drawStream->smodelCount + 1) >> 1;
    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[R_GetCachedSModelSurf(*drawStream->smodelList)->smodelIndex];
    drawStream->localSurf = xsurf;
    drawStream->reflectionProbeIndex = smodelDrawInst->reflectionProbeIndex;
    g_frameStatsCur.geoIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;

    iassert(g_primStats);
    g_primStats->dynamicIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
    g_primStats->dynamicVertexCount += drawStream->smodelCount * xsurf->vertCount;
    return 1;
}

XSurface *__cdecl R_GetCurrentStaticModelCachedSurf(
    GfxStaticModelDrawStream *drawStream,
    unsigned int *reflectionProbeIndex)
{
    if (reflectionProbeIndex)
        *reflectionProbeIndex = drawStream->reflectionProbeIndex;
    return drawStream->localSurf;
}

void __cdecl R_SetStaticModelCachedPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args)
{
    iassert(xsurf);
    args->vertexCount = 0x10000;
    args->triCount = xsurf->triCount;
}

void __cdecl R_SetStaticModelCachedBuffer(GfxCmdBufState *state, unsigned int cachedIndex)
{
    R_SetStreamSource(&state->prim, gfxBuf.smodelCacheVb, ((cachedIndex - 1) & 0xFFFFF000) << 9, 32);
}

void __cdecl R_DrawStaticModelsCachedDrawSurfLighting(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    unsigned int copyBaseIndex; // [esp+0h] [ebp-30h]
    unsigned int baseIndex; // [esp+4h] [ebp-2Ch]
    unsigned int surfBaseIndex; // [esp+8h] [ebp-28h]
    unsigned int reflectionProbeIndex; // [esp+10h] [ebp-20h] BYREF
    const unsigned __int16 *list; // [esp+14h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+18h] [ebp-18h]
    unsigned int index; // [esp+1Ch] [ebp-14h]
    const XSurface *xsurf; // [esp+20h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+24h] [ebp-Ch] BYREF

    xsurf = R_GetCurrentStaticModelCachedSurf(drawStream, &reflectionProbeIndex);
    list = drawStream->smodelList;
    smodelCount = drawStream->smodelCount;
    R_SetStaticModelCachedPrimArgs(xsurf, &args);
    R_SetStaticModelCachedBuffer(context.state, *list);
    R_SetupPassPerPrimArgs(context);
    R_SetReflectionProbe(context, reflectionProbeIndex);
    surfBaseIndex = 3 * xsurf->baseTriIndex;
    args.triCount = smodelCount * xsurf->triCount;
    args.baseIndex = R_ReserveIndexData(&context.state->prim, args.triCount);
    index = 0;
    do
    {
        baseIndex = surfBaseIndex + 4 * R_GetCachedSModelSurf(list[index])->baseVertIndex;
        iassert(baseIndex < SMC_MAX_INDEX_IN_CACHE);
        iassert(baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE);
        copyBaseIndex = R_SetIndexData(&context.state->prim, (unsigned char*)&gfxBuf.smodelCache.indices[baseIndex], xsurf->triCount);
        iassert(copyBaseIndex == args.baseIndex + xsurf->triCount * 3 * index);
        ++index;
    } while (index < smodelCount);
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

void __cdecl R_DrawStaticModelsCachedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    unsigned int copyBaseIndex; // [esp+0h] [ebp-2Ch]
    unsigned int baseIndex; // [esp+4h] [ebp-28h]
    unsigned int surfBaseIndex; // [esp+8h] [ebp-24h]
    const unsigned __int16 *list; // [esp+10h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+14h] [ebp-18h]
    unsigned int index; // [esp+18h] [ebp-14h]
    const XSurface *xsurf; // [esp+1Ch] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+20h] [ebp-Ch] BYREF

    xsurf = R_GetCurrentStaticModelCachedSurf(drawStream, 0);
    list = drawStream->smodelList;
    smodelCount = drawStream->smodelCount;
    R_SetStaticModelCachedPrimArgs(xsurf, &args);
    R_SetStaticModelCachedBuffer(context.state, *list);
    R_SetupPassPerPrimArgs(context);
    surfBaseIndex = 3 * xsurf->baseTriIndex;
    args.triCount = smodelCount * xsurf->triCount;
    args.baseIndex = R_ReserveIndexData(&context.state->prim, args.triCount);
    index = 0;
    do
    {
        baseIndex = surfBaseIndex + 4 * R_GetCachedSModelSurf(list[index])->baseVertIndex;
        iassert(baseIndex < SMC_MAX_INDEX_IN_CACHE);
        iassert(baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE);
        copyBaseIndex = R_SetIndexData(&context.state->prim, (unsigned char*)&gfxBuf.smodelCache.indices[baseIndex], xsurf->triCount);
        iassert(copyBaseIndex == args.baseIndex + xsurf->triCount * 3 * index);
        ++index;
    } while (index < smodelCount);
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

void __cdecl R_DrawStaticModelCachedSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxStaticModelDrawStream drawStream; // [esp+0h] [ebp-1Ch] BYREF

    R_SetCodeImageTexture(context.source, 16, rgp.whiteImage);
    R_SetupCachedStaticModelLighting(context.source);
    R_SetupPassPerObjectArgs(context);
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    while (R_GetNextStaticModelCachedSurf(&drawStream))
        R_DrawStaticModelsCachedDrawSurfLighting(&drawStream, context);
    context.state->samplerTexture[1] = drawStream.reflectionProbeTexture;
}

void __cdecl R_DrawStaticModelCachedSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxStaticModelDrawStream drawStream; // [esp+0h] [ebp-1Ch] BYREF

    R_SetupPassPerObjectArgs(context);
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    while (R_GetNextStaticModelCachedSurf(&drawStream))
        R_DrawStaticModelsCachedDrawSurf(&drawStream, context);
    if (context.state->samplerTexture[1] != drawStream.reflectionProbeTexture)
        MyAssertHandler(
            ".\\r_draw_staticmodel.cpp",
            2076,
            0,
            "%s",
            "context.state->samplerTexture[TEXTURE_DEST_CODE_REFLECTION_PROBE] == drawStream.reflectionProbeTexture");
}

const unsigned int *__cdecl R_ReadPrimDrawSurfData(GfxReadCmdBuf *cmdBuf, unsigned int count)
{
    const unsigned int *result; // [esp+0h] [ebp-4h]

    result = cmdBuf->primDrawSurfPos;
    cmdBuf->primDrawSurfPos += count;
    return result;
}

unsigned int __cdecl R_ReadPrimDrawSurfInt(GfxReadCmdBuf *cmdBuf)
{
    return *cmdBuf->primDrawSurfPos++;
}

int __cdecl R_ReadStaticModelPreTessDrawSurf(
    GfxReadCmdBuf *readCmdBuf,
    GfxStaticModelPreTessSurf *pretessSurf,
    unsigned int *firstIndex,
    unsigned int *count)
{
    *count = R_ReadPrimDrawSurfInt(readCmdBuf);
    if (!*count)
        return 0;
    pretessSurf->packed = R_ReadPrimDrawSurfInt(readCmdBuf);
    *firstIndex = R_ReadPrimDrawSurfInt(readCmdBuf);
    if (*firstIndex >= 0x100000)
        MyAssertHandler(
            ".\\r_draw_staticmodel.cpp",
            1894,
            0,
            "*firstIndex doesn't index R_MAX_PRETESS_INDICES\n\t%i not in [0, %i)",
            *firstIndex,
            0x100000);
    return 1;
}

const GfxStaticModelDrawInst *__cdecl R_SetupCachedSModelSurface(
    GfxCmdBufState *state,
    unsigned int cachedIndex,
    unsigned int lod,
    unsigned int surfIndex,
    unsigned int count,
    GfxDrawPrimArgs *args,
    unsigned int *baseIndex)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+14h] [ebp-Ch]
    const XSurface *xsurf; // [esp+1Ch] [ebp-4h]

    if (!cachedIndex)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 1822, 0, "%s", "cachedIndex");
    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[R_GetCachedSModelSurf(cachedIndex)->smodelIndex];
    xsurf = XModelGetSurface(smodelDrawInst->model, lod, surfIndex);
    if (baseIndex)
        *baseIndex = 3 * xsurf->baseTriIndex;
    args->vertexCount = 0x10000;
    args->triCount = count * xsurf->triCount;
    R_SetStreamSource(&state->prim, gfxBuf.smodelCacheVb, ((cachedIndex - 1) & 0xFFFFF000) << 9, 32);
    g_frameStatsCur.geoIndexCount += 3 * count * xsurf->triCount;
    iassert(g_primStats);
    g_primStats->dynamicIndexCount += 3 * count * xsurf->triCount;
    g_primStats->dynamicVertexCount += count * xsurf->vertCount;
    return smodelDrawInst;
}

void __cdecl R_DrawStaticModelsPreTessDrawSurf(
    GfxStaticModelPreTessSurf pretessSurf,
    unsigned int firstIndex,
    unsigned int count,
    GfxCmdBufContext context)
{
    IDirect3DIndexBuffer9 *ib; // [esp+0h] [ebp-2Ch]
    GfxDrawPrimArgs args; // [esp+20h] [ebp-Ch] BYREF

    if (!count)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 2034, 0, "%s", "count");
    R_SetupCachedSModelSurface(
        context.state,
        pretessSurf.fields.cachedIndex,
        pretessSurf.fields.lod,
        pretessSurf.fields.surfIndex,
        count,
        &args,
        0);
    R_SetupPassPerPrimArgs(context);
    ib = context.source->input.data->preTessIb;
    if (context.state->prim.indexBuffer != ib)
        R_ChangeIndices(&context.state->prim, ib);
    args.baseIndex = firstIndex;
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

void __cdecl R_DrawStaticModelsPreTessDrawSurfLighting(
    GfxStaticModelPreTessSurf pretessSurf,
    unsigned int firstIndex,
    unsigned int count,
    GfxCmdBufContext context)
{
    IDirect3DIndexBuffer9 *ib; // [esp+0h] [ebp-30h]
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+20h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+24h] [ebp-Ch] BYREF

    if (!count)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 1970, 0, "%s", "count");
    smodelDrawInst = R_SetupCachedSModelSurface(
        context.state,
        pretessSurf.fields.cachedIndex,
        pretessSurf.fields.lod,
        pretessSurf.fields.surfIndex,
        count,
        &args,
        0);
    R_SetupPassPerPrimArgs(context);
    R_SetReflectionProbe(context, smodelDrawInst->reflectionProbeIndex);
    ib = context.source->input.data->preTessIb;
    if (context.state->prim.indexBuffer != ib)
        R_ChangeIndices(&context.state->prim, ib);
    args.baseIndex = firstIndex;
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

void __cdecl R_SetStaticModelSkinnedPrimArgs(GfxCmdBufPrimState *state, const XSurface *xsurf, GfxDrawPrimArgs *args)
{
    iassert(xsurf);
    args->triCount = XSurfaceGetNumTris(xsurf);
    args->vertexCount = XSurfaceGetNumVerts(xsurf);
    args->baseIndex = R_SetIndexData(state, (unsigned char*)xsurf->triIndices, args->triCount);
}

void __cdecl R_DrawStaticModelSkinnedDrawSurfLighting(
    const GfxStaticModelDrawInst *smodelDrawInst,
    unsigned __int16 lightingHandle,
    GfxDrawPrimArgs *args,
    GfxCmdBufContext context)
{
    R_SetReflectionProbe(context, smodelDrawInst->reflectionProbeIndex);
    R_DrawStaticModelDrawSurfPlacement(smodelDrawInst, context.source);
    R_SetModelLightingCoordsForSource(lightingHandle, context.source);
    R_SetupPassPerPrimArgs(context);
    R_DrawIndexedPrimitive(&context.state->prim, args);
}

void __cdecl R_DrawStaticModelsSkinnedDrawSurfLighting(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+10h] [ebp-30h]
    IDirect3DVertexBuffer9 *vb; // [esp+14h] [ebp-2Ch]
    unsigned int vertexOffset; // [esp+18h] [ebp-28h]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1Ch] [ebp-24h]
    const unsigned __int16 *list; // [esp+20h] [ebp-20h]
    unsigned int smodelCount; // [esp+24h] [ebp-1Ch]
    unsigned int index; // [esp+28h] [ebp-18h]
    XSurface *xsurf; // [esp+2Ch] [ebp-14h]
    GfxDrawPrimArgs args; // [esp+30h] [ebp-10h] BYREF
    unsigned __int16 lightingHandle; // [esp+3Ch] [ebp-4h]

    xsurf = drawStream->localSurf;
    R_SetStaticModelSkinnedPrimArgs(&context.state->prim, xsurf, &args);
    R_CheckVertexDataOverflow(32 * args.vertexCount);
    vertexOffset = R_SetVertexData(context.state, xsurf->verts0, args.vertexCount, 32);
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    if (!vb)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 1714, 0, "%s", "vb");
    R_SetStreamSource(&context.state->prim, vb, vertexOffset, 32);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    for (index = 0; index < smodelCount; ++index)
    {
        smodelDrawInst = &smodelDrawInsts[list[index]];
        lightingHandle = smodelDrawInst->lightingHandle;
        R_DrawStaticModelSkinnedDrawSurfLighting(smodelDrawInst, lightingHandle, &args, context);
    }
}

void __cdecl R_DrawStaticModelSkinnedSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxStaticModelDrawStream drawStream; // [esp+0h] [ebp-20h] BYREF
    XSurface *surf; // [esp+1Ch] [ebp-4h] BYREF

    R_SetCodeImageTexture(context.source, 0x10u, rgp.whiteImage);
    R_SetupPassPerObjectArgs(context);
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    while (R_GetNextStaticModelSurf(&drawStream, &surf))
        R_DrawStaticModelsSkinnedDrawSurfLighting(&drawStream, context);
    context.state->samplerTexture[1] = drawStream.reflectionProbeTexture;
}

void __cdecl R_DrawStaticModelSkinnedDrawSurf(
    const GfxStaticModelDrawInst *smodelDrawInst,
    GfxDrawPrimArgs *args,
    GfxCmdBufContext context)
{
    R_DrawStaticModelDrawSurfPlacement(smodelDrawInst, context.source);
    R_SetupPassPerPrimArgs(context);
    R_DrawIndexedPrimitive(&context.state->prim, args);
}

void __cdecl R_DrawStaticModelsSkinnedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    IDirect3DVertexBuffer9 *vb; // [esp+14h] [ebp-28h]
    unsigned int vertexOffset; // [esp+18h] [ebp-24h]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1Ch] [ebp-20h]
    const unsigned __int16 *list; // [esp+20h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+24h] [ebp-18h]
    unsigned int index; // [esp+28h] [ebp-14h]
    XSurface *xsurf; // [esp+2Ch] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+30h] [ebp-Ch] BYREF

    xsurf = drawStream->localSurf;
    R_SetStaticModelSkinnedPrimArgs(&context.state->prim, xsurf, &args);
    R_CheckVertexDataOverflow(32 * args.vertexCount);
    vertexOffset = R_SetVertexData(context.state, xsurf->verts0, args.vertexCount, 32);
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    if (!vb)
        MyAssertHandler(".\\r_draw_staticmodel.cpp", 1752, 0, "%s", "vb");
    R_SetStreamSource(&context.state->prim, vb, vertexOffset, 0x20u);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    for (index = 0; index < smodelCount; ++index)
        R_DrawStaticModelSkinnedDrawSurf(&smodelDrawInsts[list[index]], &args, context);
}

void __cdecl R_DrawStaticModelSkinnedSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxStaticModelDrawStream drawStream; // [esp+0h] [ebp-20h] BYREF
    XSurface *surf; // [esp+1Ch] [ebp-4h] BYREF

    R_SetupPassPerObjectArgs(context);
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    while (R_GetNextStaticModelSurf(&drawStream, &surf))
        R_DrawStaticModelsSkinnedDrawSurf(&drawStream, context);
    if (context.state->samplerTexture[1] != drawStream.reflectionProbeTexture)
        MyAssertHandler(
            ".\\r_draw_staticmodel.cpp",
            1799,
            0,
            "%s",
            "context.state->samplerTexture[TEXTURE_DEST_CODE_REFLECTION_PROBE] == drawStream.reflectionProbeTexture");
}