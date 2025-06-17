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

struct vector3
{
    float4 x;
    float4 y;
    float4 z;
};
struct vector4 : vector3
{
    float4 w;
};

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

    modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * drawSurf.fields.objectId);
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
        modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * drawSurf.fields.objectId);
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
    float v4; // [esp+24h] [ebp-F0h]
    float v5; // [esp+28h] [ebp-ECh]
    float v6; // [esp+2Ch] [ebp-E8h]
    float v7; // [esp+30h] [ebp-E4h]
    float v8; // [esp+34h] [ebp-E0h]
    float v9; // [esp+38h] [ebp-DCh]
    float v10; // [esp+3Ch] [ebp-D8h]
    float v11; // [esp+40h] [ebp-D4h]
    float v12; // [esp+44h] [ebp-D0h]
    float v13; // [esp+48h] [ebp-CCh]
    float v14; // [esp+4Ch] [ebp-C8h]
    float v15; // [esp+50h] [ebp-C4h]
    GfxCmdBufSourceState *matrix; // [esp+58h] [ebp-BCh]
    float worldMat[16]; // [esp+5Ch] [ebp-B8h]
    unsigned int baseGfxEntIndex; // [esp+A0h] [ebp-74h]
    GfxDrawSurf drawSurf; // [esp+A4h] [ebp-70h]
    const GfxBackEndData *data; // [esp+ACh] [ebp-68h]
    float4 quat; // [esp+B4h] [ebp-60h] BYREF
    unsigned int drawSurfIndex; // [esp+C4h] [ebp-50h]
    float4 eyeOffset; // [esp+C8h] [ebp-4Ch]
    const GfxEntity *gfxEnt; // [esp+D8h] [ebp-3Ch]
    GfxDrawSurf drawSurfMask; // [esp+DCh] [ebp-38h]
    const GfxModelRigidSurface *modelSurf; // [esp+E8h] [ebp-2Ch]
    unsigned int depthHackFlags; // [esp+ECh] [ebp-28h]
    unsigned int gfxEntIndex; // [esp+F0h] [ebp-24h]
    float4 scale; // [esp+F4h] [ebp-20h]
    float materialTime; // [esp+108h] [ebp-Ch]
    unsigned __int64 drawSurfKey; // [esp+10Ch] [ebp-8h]

    data = context.source->input.data;
    drawSurf.fields = drawSurfList->fields;
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
        quat = *(float4*)modelSurf->placement.base.quat;
        if (!Vec4IsNormalized(quat.v))
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\universal\\com_vector4_novec.h",
                842,
                0,
                "%s",
                "Vec4IsNormalized( quat.v )");
        v9 = quat.v[0] + quat.v[0];
        v14 = v9 * quat.v[0];
        v5 = v9 * quat.v[1];
        v12 = v9 * quat.v[2];
        v15 = v9 * quat.v[3];
        v10 = quat.v[1] + quat.v[1];
        v4 = v10 * quat.v[1];
        v13 = v10 * quat.v[2];
        v11 = v10 * quat.v[3];
        v6 = quat.v[2] + quat.v[2];
        v8 = v6 * quat.v[3];
        v7 = v6 * quat.v[2];
        worldMat[0] = 1.0 - (v4 + v7);
        worldMat[1] = v5 + v8;
        worldMat[2] = v12 - v11;
        worldMat[4] = v5 - v8;
        worldMat[5] = 1.0 - (v14 + v7);
        worldMat[6] = v13 + v15;
        worldMat[8] = v12 + v11;
        worldMat[9] = v13 - v15;
        worldMat[10] = 1.0 - (v14 + v4);
        scale.v[0] = modelSurf->placement.scale;
        scale.v[1] = scale.v[0];
        scale.v[2] = scale.v[0];
        scale.v[3] = scale.v[0];
        worldMat[0] = worldMat[0] * scale.v[0];
        worldMat[1] = worldMat[1] * scale.v[0];
        worldMat[2] = worldMat[2] * scale.v[0];
        worldMat[3] = 0.0 * scale.v[0];
        worldMat[4] = worldMat[4] * scale.v[0];
        worldMat[5] = worldMat[5] * scale.v[0];
        worldMat[6] = worldMat[6] * scale.v[0];
        worldMat[8] = worldMat[8] * scale.v[0];
        worldMat[9] = worldMat[9] * scale.v[0];
        worldMat[10] = worldMat[10] * scale.v[0];
        worldMat[12] = modelSurf->placement.base.origin[0] - eyeOffset.v[0];
        worldMat[13] = modelSurf->placement.base.origin[1] - eyeOffset.v[1];
        worldMat[14] = modelSurf->placement.base.origin[2] - eyeOffset.v[2];
        worldMat[15] = 0.0 - eyeOffset.v[3];
        matrix = R_GetActiveWorldMatrix(context.source);
        matrix->matrices.matrix[0].m[0][0] = worldMat[0];
        matrix->matrices.matrix[0].m[0][1] = worldMat[1];
        matrix->matrices.matrix[0].m[0][2] = worldMat[2];
        matrix->matrices.matrix[0].m[0][3] = worldMat[3];
        matrix->matrices.matrix[0].m[1][0] = worldMat[4];
        matrix->matrices.matrix[0].m[1][1] = worldMat[5];
        matrix->matrices.matrix[0].m[1][2] = worldMat[6];
        matrix->matrices.matrix[0].m[1][3] = worldMat[3];
        matrix->matrices.matrix[0].m[2][0] = worldMat[8];
        matrix->matrices.matrix[0].m[2][1] = worldMat[9];
        matrix->matrices.matrix[0].m[2][2] = worldMat[10];
        matrix->matrices.matrix[0].m[2][3] = worldMat[3];
        matrix->matrices.matrix[0].m[3][0] = worldMat[12];
        matrix->matrices.matrix[0].m[3][1] = worldMat[13];
        matrix->matrices.matrix[0].m[3][2] = worldMat[14];
        matrix->matrices.matrix[0].m[3][3] = worldMat[15];
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
    float v4; // [esp+24h] [ebp-D8h]
    float v5; // [esp+28h] [ebp-D4h]
    float v6; // [esp+2Ch] [ebp-D0h]
    float v7; // [esp+30h] [ebp-CCh]
    float v8; // [esp+34h] [ebp-C8h]
    float v9; // [esp+38h] [ebp-C4h]
    float v10; // [esp+3Ch] [ebp-C0h]
    float v11; // [esp+40h] [ebp-BCh]
    float v12; // [esp+44h] [ebp-B8h]
    float v13; // [esp+48h] [ebp-B4h]
    float v14; // [esp+4Ch] [ebp-B0h]
    float v15; // [esp+50h] [ebp-ACh]
    GfxCmdBufSourceState *matrix; // [esp+58h] [ebp-A4h]
    float worldMat[16]; // [esp+5Ch] [ebp-A0h]
    unsigned __int64 drawSurf; // [esp+9Ch] [ebp-60h]
    const GfxBackEndData *data; // [esp+A8h] [ebp-54h]
    float4 quat; // [esp+B0h] [ebp-4Ch] BYREF
    unsigned int drawSurfIndex; // [esp+C0h] [ebp-3Ch]
    float4 eyeOffset; // [esp+C4h] [ebp-38h]
    GfxDrawSurf drawSurfMask; // [esp+D4h] [ebp-28h]
    const GfxModelRigidSurface *modelSurf; // [esp+DCh] [ebp-20h]
    float4 scale; // [esp+E0h] [ebp-1Ch]
    unsigned __int64 drawSurfKey; // [esp+F4h] [ebp-8h]

    data = context.source->input.data;
    drawSurf = drawSurfList->packed;
    drawSurfMask.packed = -536870912;
    drawSurfKey = drawSurf & 0xFFFFFFFFE0000000uLL;
    drawSurfIndex = 0;
    eyeOffset.v[0] = context.source->eyeOffset[0];
    eyeOffset.v[1] = context.source->eyeOffset[1];
    eyeOffset.v[2] = context.source->eyeOffset[2];
    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0 - 1.0;
    do
    {
        modelSurf = (const GfxModelRigidSurface*)(data + 4 * drawSurf);
        quat = *(float4*)modelSurf->placement.base.quat;
        if (!Vec4IsNormalized(quat.v))
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\universal\\com_vector4_novec.h",
                842,
                0,
                "%s",
                "Vec4IsNormalized( quat.v )");
        v9 = quat.v[0] + quat.v[0];
        v14 = v9 * quat.v[0];
        v5 = v9 * quat.v[1];
        v12 = v9 * quat.v[2];
        v15 = v9 * quat.v[3];
        v10 = quat.v[1] + quat.v[1];
        v4 = v10 * quat.v[1];
        v13 = v10 * quat.v[2];
        v11 = v10 * quat.v[3];
        v6 = quat.v[2] + quat.v[2];
        v8 = v6 * quat.v[3];
        v7 = v6 * quat.v[2];
        worldMat[0] = 1.0 - (v4 + v7);
        worldMat[1] = v5 + v8;
        worldMat[2] = v12 - v11;
        worldMat[4] = v5 - v8;
        worldMat[5] = 1.0 - (v14 + v7);
        worldMat[6] = v13 + v15;
        worldMat[8] = v12 + v11;
        worldMat[9] = v13 - v15;
        worldMat[10] = 1.0 - (v14 + v4);
        scale.v[0] = modelSurf->placement.scale;
        scale.v[1] = scale.v[0];
        scale.v[2] = scale.v[0];
        scale.v[3] = scale.v[0];
        worldMat[0] = worldMat[0] * scale.v[0];
        worldMat[1] = worldMat[1] * scale.v[0];
        worldMat[2] = worldMat[2] * scale.v[0];
        worldMat[3] = 0.0 * scale.v[0];
        worldMat[4] = worldMat[4] * scale.v[0];
        worldMat[5] = worldMat[5] * scale.v[0];
        worldMat[6] = worldMat[6] * scale.v[0];
        worldMat[8] = worldMat[8] * scale.v[0];
        worldMat[9] = worldMat[9] * scale.v[0];
        worldMat[10] = worldMat[10] * scale.v[0];
        worldMat[12] = modelSurf->placement.base.origin[0] - eyeOffset.v[0];
        worldMat[13] = modelSurf->placement.base.origin[1] - eyeOffset.v[1];
        worldMat[14] = modelSurf->placement.base.origin[2] - eyeOffset.v[2];
        worldMat[15] = 0.0 - eyeOffset.v[3];
        matrix = R_GetActiveWorldMatrix(context.source);
        matrix->matrices.matrix[0].m[0][0] = worldMat[0];
        matrix->matrices.matrix[0].m[0][1] = worldMat[1];
        matrix->matrices.matrix[0].m[0][2] = worldMat[2];
        matrix->matrices.matrix[0].m[0][3] = worldMat[3];
        matrix->matrices.matrix[0].m[1][0] = worldMat[4];
        matrix->matrices.matrix[0].m[1][1] = worldMat[5];
        matrix->matrices.matrix[0].m[1][2] = worldMat[6];
        matrix->matrices.matrix[0].m[1][3] = worldMat[3];
        matrix->matrices.matrix[0].m[2][0] = worldMat[8];
        matrix->matrices.matrix[0].m[2][1] = worldMat[9];
        matrix->matrices.matrix[0].m[2][2] = worldMat[10];
        matrix->matrices.matrix[0].m[2][3] = worldMat[3];
        matrix->matrices.matrix[0].m[3][0] = worldMat[12];
        matrix->matrices.matrix[0].m[3][1] = worldMat[13];
        matrix->matrices.matrix[0].m[3][2] = worldMat[14];
        matrix->matrices.matrix[0].m[3][3] = worldMat[15];
        R_DrawXModelRigidModelSurf(context, modelSurf->surf.xsurf);
        if (++drawSurfIndex == drawSurfCount)
            break;
        LODWORD(drawSurf) = drawSurfList[drawSurfIndex].packed;
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