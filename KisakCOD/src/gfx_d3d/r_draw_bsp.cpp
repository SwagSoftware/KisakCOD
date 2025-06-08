#include "r_draw_bsp.h"
#include "r_state.h"
#include "rb_logfile.h"
#include "r_dvars.h"
#include "r_state.h"
#include "r_shade.h"
#include "rb_stats.h"
#include "rb_tess.h"
#include "r_pretess.h"

const int g_layerDataStride[16] = { 0, 0, 0, 8, 12, 16, 20, 24, 24, 28, 32, 32, 36, 40, 0, 0 }; // idb

void __cdecl R_SetStreamSource(
    GfxCmdBufPrimState *primState,
    IDirect3DVertexBuffer9 *vb,
    unsigned int vertexOffset,
    unsigned int vertexStride)
{
    if (primState->streams[0].vb != vb
        || primState->streams[0].offset != vertexOffset
        || primState->streams[0].stride != vertexStride)
    {
        R_ChangeStreamSource(primState, 0, vb, vertexOffset, vertexStride);
    }
    if (primState->streams[1].vb || primState->streams[1].offset || primState->streams[1].stride)
        R_ChangeStreamSource(primState, 1u, 0, 0, 0);
}

void __cdecl R_HW_SetSamplerTexture(IDirect3DDevice9 *device, unsigned int samplerIndex, const GfxTexture *texture)
{
    int hr; // [esp+0h] [ebp-4h]

    iassert(texture);
    iassert(texture->basemap);

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetTexture( samplerIndex, texture->basemap )\n");

        hr = device->SetTexture(samplerIndex, texture->basemap);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetTexture( samplerIndex, texture->basemap ) failed: %s\n",
                    121,
                    R_ErrorDescription(hr));
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_SetStreamsForBspSurface(GfxCmdBufPrimState *state, const srfTriangles_t *tris)
{
    int vertexLayerData; // [esp+0h] [ebp-2Ch]
    IDirect3DVertexBuffer9 *layerVb; // [esp+4h] [ebp-28h]
    int vertexOffset; // [esp+8h] [ebp-24h]
    IDirect3DVertexBuffer9 *vb; // [esp+Ch] [ebp-20h]
    unsigned int layerDataStride; // [esp+28h] [ebp-4h]

    layerDataStride = g_layerDataStride[state->vertDeclType];
    if (layerDataStride)
    {
        vertexLayerData = tris->vertexLayerData;
        layerVb = rgp.world->vld.layerVb;
        vertexOffset = 44 * tris->firstVertex;
        vb = rgp.world->vd.worldVb;
        if (state->streams[0].vb != vb || state->streams[0].offset != vertexOffset || state->streams[0].stride != 44)
            R_ChangeStreamSource(state, 0, vb, vertexOffset, 0x2Cu);
        if (state->streams[1].vb != layerVb
            || state->streams[1].offset != vertexLayerData
            || state->streams[1].stride != layerDataStride)
        {
            R_ChangeStreamSource(state, 1u, layerVb, vertexLayerData, layerDataStride);
        }
    }
    else
    {
        R_SetStreamSource(state, rgp.world->vd.worldVb, 44 * tris->firstVertex, 0x2Cu);
    }
}

void __cdecl R_DrawBspDrawSurfsLit(
    const unsigned int *primDrawSurfPos,
    GfxCmdBufContext context,
    GfxCmdBufContext prepassContext)
{
    GfxTrianglesDrawStream drawStream; // [esp+4h] [ebp-38h] BYREF
    const MaterialPass *pass; // [esp+34h] [ebp-8h]
    unsigned int customSamplerFlags; // [esp+38h] [ebp-4h]

    pass = context.state->pass;
    customSamplerFlags = pass->customSamplerFlags;
    if ((customSamplerFlags & 1) != 0)
        R_SetSamplerState(context.state, 1u, 0x72u);
    if ((customSamplerFlags & 2) != 0)
        R_SetSamplerState(context.state, 2u, 0x62u);
    if ((customSamplerFlags & 4) != 0)
        R_SetSamplerState(context.state, 3u, 0x62u);
    drawStream.reflectionProbeTexture = context.state->samplerTexture[1];
    drawStream.lightmapPrimaryTexture = context.state->samplerTexture[2];
    drawStream.lightmapSecondaryTexture = context.state->samplerTexture[3];
    drawStream.reflectionProbeCount = rgp.world->reflectionProbeCount;
    drawStream.lightmapCount = rgp.world->lightmapCount;
    drawStream.reflectionProbeTextures = rgp.world->reflectionProbeTextures;
    drawStream.lightmapPrimaryTextures = rgp.world->lightmapPrimaryTextures;
    drawStream.lightmapSecondaryTextures = rgp.world->lightmapSecondaryTextures;
    drawStream.whiteTexture = &rgp.whiteImage->texture;
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.customSamplerFlags = pass->customSamplerFlags;
    drawStream.hasSunDirChanged = context.source->input.data->prim.hasSunDirChanged;
    if (prepassContext.state)
        R_DrawTrianglesLit(&drawStream, &context.state->prim, &prepassContext.state->prim);
    else
        R_DrawTrianglesLit(&drawStream, &context.state->prim, 0);
    context.state->samplerTexture[1] = drawStream.reflectionProbeTexture;
    context.state->samplerTexture[2] = drawStream.lightmapPrimaryTexture;
    context.state->samplerTexture[3] = drawStream.lightmapSecondaryTexture;
}

void __cdecl R_DrawTrianglesLit(
    GfxTrianglesDrawStream *drawStream,
    GfxCmdBufPrimState *primState,
    GfxCmdBufPrimState *prepassPrimState)
{
    const GfxTexture *v3; // [esp+0h] [ebp-78h]
    int baseIndex; // [esp+10h] [ebp-68h]
    unsigned int surfIndex; // [esp+14h] [ebp-64h]
    const GfxSurface *tris; // [esp+18h] [ebp-60h]
    const srfTriangles_t *prevTris; // [esp+1Ch] [ebp-5Ch]
    unsigned int lightmapSecondaryFlag; // [esp+20h] [ebp-58h]
    unsigned int reflectionProbeFlag; // [esp+24h] [ebp-54h]
    const GfxTexture *lightmapPrimaryTexture; // [esp+28h] [ebp-50h]
    const unsigned __int16 *list; // [esp+2Ch] [ebp-4Ch] BYREF
    int triCount; // [esp+30h] [ebp-48h]
    unsigned int reflectionProbeIndex; // [esp+34h] [ebp-44h]
    const GfxTexture *reflectionProbeTexture; // [esp+38h] [ebp-40h]
    const GfxTexture *newLightmapPrimaryTexture; // [esp+3Ch] [ebp-3Ch]
    GfxTexture *reflectionProbeTextures; // [esp+40h] [ebp-38h]
    const GfxSurface *bspSurf; // [esp+44h] [ebp-34h]
    unsigned int index; // [esp+48h] [ebp-30h]
    unsigned int lightmapIndex; // [esp+4Ch] [ebp-2Ch]
    const GfxTexture *lightmapSecondaryTexture; // [esp+50h] [ebp-28h]
    const GfxTexture *newLightmapSecondaryTexture; // [esp+54h] [ebp-24h]
    IDirect3DDevice9 *device; // [esp+58h] [ebp-20h]
    unsigned int lightmapPrimaryFlag; // [esp+5Ch] [ebp-1Ch]
    const GfxImage *overrideImage; // [esp+60h] [ebp-18h]
    unsigned int count; // [esp+64h] [ebp-14h] BYREF
    int baseVertex; // [esp+68h] [ebp-10h]
    const GfxTexture *newReflectionProbeTexture; // [esp+6Ch] [ebp-Ch]
    int hasSunDirChanged; // [esp+70h] [ebp-8h]
    int override; // [esp+74h] [ebp-4h]

    reflectionProbeIndex = 255;
    lightmapIndex = 31;
    prevTris = 0;
    triCount = 0;
    baseVertex = -1;
    baseIndex = 0;
    reflectionProbeTexture = drawStream->reflectionProbeTexture;
    lightmapPrimaryTexture = drawStream->lightmapPrimaryTexture;
    lightmapSecondaryTexture = drawStream->lightmapSecondaryTexture;
    reflectionProbeFlag = drawStream->customSamplerFlags & 1;
    lightmapPrimaryFlag = drawStream->customSamplerFlags & 2;
    lightmapSecondaryFlag = drawStream->customSamplerFlags & 4;
    reflectionProbeTextures = drawStream->reflectionProbeTextures;
    hasSunDirChanged = drawStream->hasSunDirChanged;
    override = r_lightMap->current.integer != 1;
    device = primState->device;
    while (R_ReadBspDrawSurfs(&drawStream->primDrawSurfPos, &list, &count))
    {
        for (index = 0; index < count; ++index)
        {
            surfIndex = list[index];
            if (surfIndex >= rgp.world->surfaceCount)
                MyAssertHandler(
                    ".\\r_draw_bsp.cpp",
                    303,
                    0,
                    "surfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                    surfIndex,
                    rgp.world->surfaceCount);
            bspSurf = &rgp.world->dpvs.surfaces[surfIndex];
            tris = bspSurf;
            if (reflectionProbeIndex == bspSurf->reflectionProbeIndex && lightmapIndex == bspSurf->lightmapIndex)
            {
                if (baseVertex != bspSurf->tris.firstVertex || baseIndex + 3 * triCount != bspSurf->tris.baseIndex)
                {
                    if (prevTris)
                        R_DrawBspTris(primState, prevTris, triCount);
                    prevTris = &tris->tris;
                    triCount = 0;
                    baseIndex = tris->tris.baseIndex;
                    if (baseVertex != tris->tris.firstVertex)
                    {
                        baseVertex = tris->tris.firstVertex;
                        R_SetStreamsForBspSurface(primState, &tris->tris);
                    }
                }
            }
            else
            {
                if (prevTris)
                    R_DrawBspTris(primState, prevTris, triCount);
                prevTris = &tris->tris;
                triCount = 0;
                baseIndex = tris->tris.baseIndex;
                if (baseVertex != tris->tris.firstVertex)
                {
                    baseVertex = tris->tris.firstVertex;
                    R_SetStreamsForBspSurface(primState, &tris->tris);
                }
                reflectionProbeIndex = bspSurf->reflectionProbeIndex;
                lightmapIndex = bspSurf->lightmapIndex;
                if (reflectionProbeFlag)
                {
                    if (reflectionProbeIndex >= drawStream->reflectionProbeCount)
                        MyAssertHandler(
                            ".\\r_draw_bsp.cpp",
                            337,
                            0,
                            "reflectionProbeIndex doesn't index drawStream->reflectionProbeCount\n\t%i not in [0, %i)",
                            reflectionProbeIndex,
                            drawStream->reflectionProbeCount);
                    newReflectionProbeTexture = &reflectionProbeTextures[reflectionProbeIndex];
                    if (reflectionProbeTexture != newReflectionProbeTexture)
                    {
                        reflectionProbeTexture = newReflectionProbeTexture;
                        R_HW_SetSamplerTexture(device, 1u, newReflectionProbeTexture);
                    }
                }
                if (lightmapIndex == 31)
                {
                    if (lightmapPrimaryFlag)
                        MyAssertHandler(
                            ".\\r_draw_bsp.cpp",
                            390,
                            0,
                            "%s\n\t(bspSurf->material->info.name) = %s",
                            "(!lightmapPrimaryFlag)",
                            bspSurf->material->info.name);
                    if (lightmapSecondaryFlag)
                        MyAssertHandler(
                            ".\\r_draw_bsp.cpp",
                            391,
                            0,
                            "%s\n\t(bspSurf->material->info.name) = %s",
                            "(!lightmapSecondaryFlag)",
                            bspSurf->material->info.name);
                }
                else
                {
                    if (lightmapIndex >= drawStream->lightmapCount)
                        MyAssertHandler(
                            ".\\r_draw_bsp.cpp",
                            348,
                            0,
                            "lightmapIndex doesn't index drawStream->lightmapCount\n\t%i not in [0, %i)",
                            lightmapIndex,
                            drawStream->lightmapCount);
                    if (lightmapPrimaryFlag)
                    {
                        if (override)
                        {
                            overrideImage = R_OverrideGrayscaleImage(r_lightMap);
                            newLightmapPrimaryTexture = &overrideImage->texture;
                        }
                        else
                        {
                            v3 = hasSunDirChanged ? drawStream->whiteTexture : &drawStream->lightmapPrimaryTextures[lightmapIndex];
                            newLightmapPrimaryTexture = v3;
                        }
                        if (lightmapPrimaryTexture != newLightmapPrimaryTexture)
                        {
                            lightmapPrimaryTexture = newLightmapPrimaryTexture;
                            R_HW_SetSamplerTexture(device, 2u, newLightmapPrimaryTexture);
                        }
                    }
                    if (lightmapSecondaryFlag)
                    {
                        if (override)
                        {
                            overrideImage = R_OverrideGrayscaleImage(r_lightMap);
                            newLightmapSecondaryTexture = &overrideImage->texture;
                        }
                        else
                        {
                            newLightmapSecondaryTexture = &drawStream->lightmapSecondaryTextures[lightmapIndex];
                        }
                        if (lightmapSecondaryTexture != newLightmapSecondaryTexture)
                        {
                            lightmapSecondaryTexture = newLightmapSecondaryTexture;
                            R_HW_SetSamplerTexture(device, 3u, newLightmapSecondaryTexture);
                        }
                    }
                }
            }
            triCount += tris->tris.triCount;
            if (prepassPrimState)
                MyAssertHandler(".\\r_draw_bsp.cpp", 448, 0, "%s", "!prepassPrimState");
        }
    }
    if (prevTris)
        R_DrawBspTris(primState, prevTris, triCount);
    drawStream->reflectionProbeTexture = reflectionProbeTexture;
    drawStream->lightmapPrimaryTexture = lightmapPrimaryTexture;
    drawStream->lightmapSecondaryTexture = lightmapSecondaryTexture;
}

void __cdecl R_DrawBspTris(GfxCmdBufPrimState *state, const srfTriangles_t *tris, unsigned int triCount)
{
    GfxDrawPrimArgs args; // [esp+0h] [ebp-Ch] BYREF

    args.vertexCount = tris->vertexCount;
    args.triCount = triCount;
    args.baseIndex = R_SetIndexData(state, (unsigned __int8 *)&rgp.world->indices[tris->baseIndex], triCount);
    R_DrawIndexedPrimitive(state, &args);
    g_frameStatsCur.geoIndexCount += 3 * triCount;
    if (!g_primStats)
        MyAssertHandler(".\\r_draw_bsp.cpp", 167, 0, "%s", "g_primStats");
    g_primStats->dynamicIndexCount += 3 * triCount;
}

int __cdecl R_ReadBspDrawSurfs(
    const unsigned int **primDrawSurfPos,
    const unsigned __int16 **list,
    unsigned int *count)
{
    *count = *(*primDrawSurfPos)++;
    if (!*count)
        return 0;
    *list = (const unsigned __int16 *)*primDrawSurfPos;
    *primDrawSurfPos += (*count + 1) >> 1;
    return 1;
}

void __cdecl R_DrawBspDrawSurfs(const unsigned int *primDrawSurfPos, GfxCmdBufState *state)
{
    GfxTrianglesDrawStream drawStream; // [esp+0h] [ebp-30h] BYREF

    drawStream.primDrawSurfPos = primDrawSurfPos;
    R_DrawTriangles(&drawStream, &state->prim);
}

void __cdecl R_DrawTriangles(GfxTrianglesDrawStream *drawStream, GfxCmdBufPrimState *state)
{
    int baseIndex; // [esp+0h] [ebp-28h]
    const GfxSurface *tris; // [esp+8h] [ebp-20h]
    const srfTriangles_t *prevTris; // [esp+Ch] [ebp-1Ch]
    const unsigned __int16 *list; // [esp+10h] [ebp-18h] BYREF
    int triCount; // [esp+14h] [ebp-14h]
    const GfxSurface *bspSurf; // [esp+18h] [ebp-10h]
    unsigned int index; // [esp+1Ch] [ebp-Ch]
    unsigned int count; // [esp+20h] [ebp-8h] BYREF
    int baseVertex; // [esp+24h] [ebp-4h]

    prevTris = 0;
    triCount = 0;
    baseVertex = -1;
    baseIndex = 0;
    while (R_ReadBspDrawSurfs(&drawStream->primDrawSurfPos, &list, &count))
    {
        for (index = 0; index < count; ++index)
        {
            bspSurf = &rgp.world->dpvs.surfaces[list[index]];
            tris = bspSurf;
            if (baseVertex != bspSurf->tris.firstVertex || baseIndex + 3 * triCount != bspSurf->tris.baseIndex)
            {
                if (prevTris)
                    R_DrawBspTris(state, prevTris, triCount);
                prevTris = &tris->tris;
                triCount = 0;
                baseIndex = tris->tris.baseIndex;
                if (baseVertex != tris->tris.firstVertex)
                {
                    baseVertex = tris->tris.firstVertex;
                    R_SetStreamsForBspSurface(state, &tris->tris);
                }
            }
            triCount += tris->tris.triCount;
        }
    }
    if (prevTris)
        R_DrawBspTris(state, prevTris, triCount);
}

void __cdecl R_DrawPreTessTris(
    GfxCmdBufPrimState *state,
    const srfTriangles_t *tris,
    unsigned int baseIndex,
    unsigned int triCount)
{
    GfxDrawPrimArgs args; // [esp+0h] [ebp-Ch] BYREF

    R_SetStreamsForBspSurface(state, tris);
    args.vertexCount = tris->vertexCount;
    args.triCount = triCount;
    args.baseIndex = baseIndex;
    R_DrawIndexedPrimitive(state, &args);
    g_frameStatsCur.geoIndexCount += 3 * triCount;
    if (!g_primStats)
        MyAssertHandler(".\\r_draw_bsp.cpp", 190, 0, "%s", "g_primStats");
    g_primStats->dynamicIndexCount += 3 * triCount;
}

void __cdecl R_DrawBspDrawSurfsPreTess(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    unsigned int baseIndex; // [esp+0h] [ebp-2Ch] BYREF
    unsigned int surfIndex; // [esp+4h] [ebp-28h]
    GfxReadCmdBuf cmdBuf; // [esp+8h] [ebp-24h] BYREF
    const srfTriangles_t *tris; // [esp+Ch] [ebp-20h]
    const srfTriangles_t *prevTris; // [esp+10h] [ebp-1Ch]
    const GfxBspPreTessDrawSurf *list; // [esp+14h] [ebp-18h] BYREF
    unsigned int triCount; // [esp+18h] [ebp-14h]
    const GfxSurface *bspSurf; // [esp+1Ch] [ebp-10h]
    unsigned int index; // [esp+20h] [ebp-Ch]
    unsigned int count; // [esp+24h] [ebp-8h] BYREF
    int baseVertex; // [esp+28h] [ebp-4h]

    R_SetupPassPerObjectArgs(context);
    R_SetupPassPerPrimArgs(context);
    cmdBuf.primDrawSurfPos = primDrawSurfPos;
    while (R_ReadBspPreTessDrawSurfs(&cmdBuf, &list, &count, &baseIndex))
    {
        prevTris = 0;
        triCount = 0;
        baseVertex = -1;
        for (index = 0; index < count; ++index)
        {
            surfIndex = list[index].baseSurfIndex;
            if (surfIndex >= rgp.world->surfaceCount)
                MyAssertHandler(
                    ".\\r_draw_bsp.cpp",
                    675,
                    0,
                    "surfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                    surfIndex,
                    rgp.world->surfaceCount);
            bspSurf = &rgp.world->dpvs.surfaces[surfIndex];
            tris = &bspSurf->tris;
            if (baseVertex != bspSurf->tris.firstVertex)
            {
                if (triCount)
                {
                    R_DrawPreTessTris(&context.state->prim, prevTris, baseIndex, triCount);
                    baseIndex += 3 * triCount;
                    triCount = 0;
                }
                prevTris = tris;
                baseVertex = tris->firstVertex;
            }
            triCount += list[index].totalTriCount;
        }
        R_DrawPreTessTris(&context.state->prim, prevTris, baseIndex, triCount);
    }
}