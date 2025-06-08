#include "r_draw_xmodel.h"
#include "rb_stats.h"
#include <database/database.h>
#include "r_state.h"
#include "r_draw_bsp.h"
#include "r_shade.h"
#include "r_dvars.h"
#include "r_utils.h"
#include "rb_tess.h"
#include <cgame/cg_local.h>
#include "r_model_lighting.h"
#include "r_dobj_skin.h"


void __cdecl R_DrawXModelRigidModelSurf(GfxCmdBufContext context, XSurface *xsurf)
{
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    int vertexOffset0; // [esp+20h] [ebp-18h] BYREF
    IDirect3DVertexBuffer9 *vb0; // [esp+24h] [ebp-14h] BYREF
    IDirect3DIndexBuffer9 *ib; // [esp+28h] [ebp-10h] BYREF
    GfxDrawPrimArgs args; // [esp+2Ch] [ebp-Ch] BYREF

    iassert(xsurf);
    args.baseIndex = 0; // LWSS: backport fix from blops
    args.vertexCount = xsurf->vertCount;
    args.triCount = xsurf->triCount;
    g_frameStatsCur.geoIndexCount += 3 * args.triCount;
    DB_GetIndexBufferAndBase(xsurf->zoneHandle, xsurf->triIndices, (void **)&ib, &args.baseIndex);
    iassert(ib);

    if (context.state->prim.indexBuffer != ib)
        R_ChangeIndices(&context.state->prim, ib);

    DB_GetVertexBufferAndOffset(xsurf->zoneHandle, (uint8*)xsurf->verts0, (void **)&vb0, &vertexOffset0);
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

void __cdecl R_GetWorldMatrixForModelSurf(const GfxModelRigidSurface *modelSurf, float4 eyeOffset, vector4 *worldMat)
{
    float v3; // [esp+24h] [ebp-58h]
    float v4; // [esp+28h] [ebp-54h]
    float v5; // [esp+30h] [ebp-4Ch]
    float v6; // [esp+34h] [ebp-48h]
    float v7; // [esp+40h] [ebp-3Ch]
    float v8; // [esp+44h] [ebp-38h]
    float v9; // [esp+48h] [ebp-34h]
    float v10; // [esp+4Ch] [ebp-30h]
    float v11; // [esp+50h] [ebp-2Ch]
    float4 quat; // [esp+54h] [ebp-28h] BYREF
    float scale; // [esp+64h] [ebp-18h]
    float4 scaleVec; // [esp+68h] [ebp-14h]

    quat = *(float4 *)modelSurf->placement.base.quat;
    iassert(Vec4IsNormalized(quat.v));

    v10 = (float)(quat.v[0] + quat.v[0]) * quat.v[0];
    v4 = (float)(quat.v[0] + quat.v[0]) * quat.v[1];
    v8 = (float)(quat.v[0] + quat.v[0]) * quat.v[2];
    v11 = (float)(quat.v[0] + quat.v[0]) * quat.v[3];
    v3 = (float)(quat.v[1] + quat.v[1]) * quat.v[1];
    v9 = (float)(quat.v[1] + quat.v[1]) * quat.v[2];
    v7 = (float)(quat.v[1] + quat.v[1]) * quat.v[3];
    v6 = (float)(quat.v[2] + quat.v[2]) * quat.v[3];
    v5 = (float)(quat.v[2] + quat.v[2]) * quat.v[2];
    worldMat->x.v[0] = 1.0 - (float)(v3 + v5);
    worldMat->x.v[1] = v4 + v6;
    worldMat->x.v[2] = v8 - v7;
    worldMat->x.u[3] = 0.0f;
    worldMat->y.v[0] = v4 - v6;
    worldMat->y.v[1] = 1.0 - (float)(v10 + v5);
    worldMat->y.v[2] = v9 + v11;
    worldMat->y.u[3] = 0.0f;
    worldMat->z.v[0] = v8 + v7;
    worldMat->z.v[1] = v9 - v11;
    worldMat->z.v[2] = 1.0 - (float)(v10 + v3);
    worldMat->z.u[3] = 0.0f;
    scale = modelSurf->placement.scale;
    scaleVec.v[0] = scale;
    scaleVec.v[1] = scale;
    scaleVec.v[2] = scale;
    scaleVec.v[3] = scale;
    worldMat->w.v[0] = modelSurf->placement.base.origin[0];
    worldMat->w.v[1] = modelSurf->placement.base.origin[1];
    worldMat->w.v[2] = modelSurf->placement.base.origin[2];
    worldMat->w.u[3] = 0.0f;
    worldMat->x.v[0] = worldMat->x.v[0] * scaleVec.v[0];
    worldMat->x.v[1] = worldMat->x.v[1] * scaleVec.v[1];
    worldMat->x.v[2] = worldMat->x.v[2] * scaleVec.v[2];
    worldMat->x.v[3] = worldMat->x.v[3] * scaleVec.v[3];
    worldMat->y.v[0] = worldMat->y.v[0] * scaleVec.v[0];
    worldMat->y.v[1] = worldMat->y.v[1] * scaleVec.v[1];
    worldMat->y.v[2] = worldMat->y.v[2] * scaleVec.v[2];
    worldMat->y.v[3] = worldMat->y.v[3] * scaleVec.v[3];
    worldMat->z.v[0] = worldMat->z.v[0] * scaleVec.v[0];
    worldMat->z.v[1] = worldMat->z.v[1] * scaleVec.v[1];
    worldMat->z.v[2] = worldMat->z.v[2] * scaleVec.v[2];
    worldMat->z.v[3] = worldMat->z.v[3] * scaleVec.v[3];
    worldMat->w.v[0] = worldMat->w.v[0] - eyeOffset.v[0];
    worldMat->w.v[1] = worldMat->w.v[1] - eyeOffset.v[1];
    worldMat->w.v[2] = worldMat->w.v[2] - eyeOffset.v[2];
    worldMat->w.v[3] = worldMat->w.v[3] - eyeOffset.v[3];
}

unsigned int __cdecl R_DrawXModelRigidSurfLitInternal(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    GfxCmdBufSourceState *matrix; // [esp+58h] [ebp-BCh]
    vector4 worldMat;
    unsigned int baseGfxEntIndex; // [esp+A0h] [ebp-74h]
    GfxDrawSurf drawSurf; // [esp+A4h] [ebp-70h]
    const GfxBackEndData *data; // [esp+ACh] [ebp-68h]
    unsigned int drawSurfIndex; // [esp+C4h] [ebp-50h]
    float4 eyeOffset; // [esp+C8h] [ebp-4Ch]
    const GfxEntity *gfxEnt; // [esp+D8h] [ebp-3Ch]
    GfxDrawSurf drawSurfMask; // [esp+DCh] [ebp-38h]
    const GfxModelRigidSurface *modelSurf; // [esp+E8h] [ebp-2Ch]
    unsigned int depthHackFlags; // [esp+ECh] [ebp-28h]
    unsigned int gfxEntIndex; // [esp+F0h] [ebp-24h]
    float materialTime; // [esp+108h] [ebp-Ch]
    unsigned __int64 drawSurfKey; // [esp+10Ch] [ebp-8h]

    data = context.source->input.data;
    drawSurf.fields = drawSurfList->fields;
    drawSurfMask.packed = 0xFFFFFFFFFF000000uLL;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFFF000000uLL;
    drawSurfIndex = 0;

    eyeOffset.v[0] = context.source->eyeOffset[0];
    eyeOffset.v[1] = context.source->eyeOffset[1];
    eyeOffset.v[2] = context.source->eyeOffset[2];

    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0f - 1.0f;

    modelSurf = (const GfxModelRigidSurface*)((char*)data + 4 * drawSurf.fields.objectId);
    baseGfxEntIndex = modelSurf->surf.info.gfxEntIndex;
    depthHackFlags = context.source->depthHackFlags;
    materialTime = context.source->materialTime;

    while (1)
    {
        R_GetWorldMatrixForModelSurf(modelSurf, eyeOffset, &worldMat);
        matrix = R_GetActiveWorldMatrix(context.source);
        memcpy(&matrix->matrices.matrix[0], &worldMat, sizeof(GfxMatrix));

        R_SetModelLightingCoordsForSource(modelSurf->surf.info.lightingHandle, context.source);
        R_SetReflectionProbe(context, drawSurf.fields.reflectionProbeIndex);
        R_DrawXModelRigidModelSurf(context, modelSurf->surf.xsurf);
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf.packed = drawSurfList[drawSurfIndex].packed;
        if ((drawSurfMask.packed & drawSurfList[drawSurfIndex].packed) != drawSurfKey)
            break;
        modelSurf = (const GfxModelRigidSurface *)((char*)data + 4 * drawSurf.fields.objectId);
        gfxEntIndex = modelSurf->surf.info.gfxEntIndex;
        if (gfxEntIndex != baseGfxEntIndex)
        {
            if (gfxEntIndex)
            {
                gfxEnt = &data->gfxEnts[gfxEntIndex];
                if ((gfxEnt->renderFxFlags & 2) != depthHackFlags || materialTime != gfxEnt->materialTime)
                    return drawSurfIndex;
            }
            else if (depthHackFlags || materialTime != 0.0)
            {
                return drawSurfIndex;
            }
        }
    }
    return drawSurfIndex;
}

unsigned int __cdecl R_DrawXModelRigidSurfLit(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    return R_DrawXModelRigidSurfLitInternal(drawSurfList, drawSurfCount, context);
}

unsigned int __cdecl R_DrawXModelRigidSurfCameraInternal(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    GfxCmdBufSourceState *matrix; // [esp+58h] [ebp-BCh]
    vector4 worldMat;
    unsigned int baseGfxEntIndex; // [esp+A0h] [ebp-74h]
    GfxDrawSurf drawSurf; // [esp+A4h] [ebp-70h]
    const GfxBackEndData *data; // [esp+ACh] [ebp-68h]
    unsigned int drawSurfIndex; // [esp+C4h] [ebp-50h]
    float4 eyeOffset; // [esp+C8h] [ebp-4Ch]
    const GfxEntity *gfxEnt; // [esp+D8h] [ebp-3Ch]
    GfxDrawSurf drawSurfMask; // [esp+DCh] [ebp-38h]
    const GfxModelRigidSurface *modelSurf; // [esp+E8h] [ebp-2Ch]
    unsigned int depthHackFlags; // [esp+ECh] [ebp-28h]
    unsigned int gfxEntIndex; // [esp+F0h] [ebp-24h]
    float materialTime; // [esp+108h] [ebp-Ch]
    unsigned __int64 drawSurfKey; // [esp+10Ch] [ebp-8h]

    data = context.source->input.data;
    drawSurf = *drawSurfList;
    drawSurfMask.packed = 0xFFFFFFFFE0000000uLL;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFE0000000uLL;
    drawSurfIndex = 0;

    eyeOffset.v[0] = context.source->eyeOffset[0];
    eyeOffset.v[1] = context.source->eyeOffset[1];
    eyeOffset.v[2] = context.source->eyeOffset[2];

    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0 - 1.0;

    modelSurf = (const GfxModelRigidSurface*)((char*)data + 4 * drawSurf.fields.objectId);
    baseGfxEntIndex = modelSurf->surf.info.gfxEntIndex;
    depthHackFlags = context.source->depthHackFlags;
    materialTime = context.source->materialTime;
    while (1)
    {
        R_GetWorldMatrixForModelSurf(modelSurf, eyeOffset, &worldMat);
        matrix = R_GetActiveWorldMatrix(context.source);
        memcpy(&matrix->matrices.matrix[0], &worldMat, sizeof(GfxMatrix));
        R_DrawXModelRigidModelSurf(context, modelSurf->surf.xsurf);
        if (++drawSurfIndex == drawSurfCount || (drawSurfMask.packed & drawSurfList[drawSurfIndex].packed) != drawSurfKey)
            break;
        modelSurf = (const GfxModelRigidSurface*)((char*)data + 4 * drawSurfList[drawSurfIndex].fields.objectId);
        gfxEntIndex = modelSurf->surf.info.gfxEntIndex;
        if (gfxEntIndex != baseGfxEntIndex)
        {
            if (gfxEntIndex)
            {
                gfxEnt = &data->gfxEnts[gfxEntIndex];
                if ((gfxEnt->renderFxFlags & 2) != depthHackFlags || materialTime != gfxEnt->materialTime)
                    return drawSurfIndex;
            }
            else if (depthHackFlags || materialTime != 0.0)
            {
                return drawSurfIndex;
            }
        }
    }
    return drawSurfIndex;
}

unsigned int __cdecl R_DrawXModelRigidSurfCamera(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    return R_DrawXModelRigidSurfCameraInternal(drawSurfList, drawSurfCount, context);
}

unsigned int __cdecl R_DrawXModelRigidSurfInternal(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    GfxCmdBufSourceState *matrix; // [esp+58h] [ebp-A4h]
    GfxDrawSurf drawSurf; // [esp+A4h] [ebp-70h]
    const GfxBackEndData *data; // [esp+A8h] [ebp-54h]
    unsigned int drawSurfIndex; // [esp+C0h] [ebp-3Ch]
    float4 eyeOffset; // [esp+C4h] [ebp-38h]
    GfxDrawSurf drawSurfMask; // [esp+D4h] [ebp-28h]
    const GfxModelRigidSurface *modelSurf; // [esp+DCh] [ebp-20h]
    unsigned __int64 drawSurfKey; // [esp+F4h] [ebp-8h]
    vector4 worldMat;

    data = context.source->input.data;
    drawSurf.packed = drawSurfList->packed;
    drawSurfMask.packed = 0xFFFFFFFFE0000000uLL;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFE0000000uLL;
    drawSurfIndex = 0;

    eyeOffset.v[0] = context.source->eyeOffset[0];
    eyeOffset.v[1] = context.source->eyeOffset[1];
    eyeOffset.v[2] = context.source->eyeOffset[2];

    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0f - 1.0f;

    do
    {
        modelSurf = (const GfxModelRigidSurface*)((char *)data + 4 * drawSurf.fields.objectId);
        R_GetWorldMatrixForModelSurf(modelSurf, eyeOffset, &worldMat);
        matrix = R_GetActiveWorldMatrix(context.source);
        memcpy(&matrix->matrices.matrix[0], &worldMat, sizeof(GfxMatrix));
        R_DrawXModelRigidModelSurf(context, modelSurf->surf.xsurf);
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf.packed = drawSurfList[drawSurfIndex].packed;
    } while ((drawSurfMask.packed & drawSurfList[drawSurfIndex].packed) == drawSurfKey);

    return drawSurfIndex;
}

unsigned int __cdecl R_DrawXModelRigidSurf(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    return R_DrawXModelRigidSurfInternal(drawSurfList, drawSurfCount, context);
}