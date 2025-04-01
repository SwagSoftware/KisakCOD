#include "fx_system.h"

static FxBeamInfo g_beamInfo;

void __cdecl FX_Beam_GenerateVerts(FxGenerateVertsCmd *cmd)
{
    float4 v1; // [esp-24h] [ebp-2DCh]
    float v2; // [esp+8h] [ebp-2B0h]
    float v3; // [esp+2Ch] [ebp-28Ch]
    float v4; // [esp+30h] [ebp-288h]
    float v5; // [esp+34h] [ebp-284h]
    float v6; // [esp+38h] [ebp-280h]
    unsigned __int8 v7; // [esp+40h] [ebp-278h]
    int v8; // [esp+44h] [ebp-274h]
    float v9; // [esp+48h] [ebp-270h]
    float4 *v10; // [esp+4Ch] [ebp-26Ch]
    float4 *v11; // [esp+50h] [ebp-268h]
    float4 *v12; // [esp+54h] [ebp-264h]
    float4 v13[2]; // [esp+58h] [ebp-260h]
    float4 v14; // [esp+78h] [ebp-240h]
    float4 *v15; // [esp+8Ch] [ebp-22Ch]
    float v16; // [esp+90h] [ebp-228h]
    float *end; // [esp+94h] [ebp-224h]
    float *v18; // [esp+98h] [ebp-220h]
    float4 v19; // [esp+9Ch] [ebp-21Ch]
    float4 v20; // [esp+ACh] [ebp-20Ch]
    float4 v21; // [esp+BCh] [ebp-1FCh]
    float4 wiggleVec; // [esp+CCh] [ebp-1ECh]
    float4 scaledWiggle; // [esp+DCh] [ebp-1DCh]
    int dim; // [esp+ECh] [ebp-1CCh]
    float4 basePos; // [esp+F0h] [ebp-1C8h]
    float lerpedRadius; // [esp+100h] [ebp-1B8h]
    float4 wiggleYs; // [esp+104h] [ebp-1B4h]
    float alpha; // [esp+114h] [ebp-1A4h]
    float4 wiggleXs; // [esp+118h] [ebp-1A0h]
    int indexPairIter; // [esp+128h] [ebp-190h]
    unsigned __int16 offset; // [esp+12Ch] [ebp-18Ch]
    float endRadius; // [esp+130h] [ebp-188h]
    r_double_index_t workingIndex; // [esp+134h] [ebp-184h]
    r_double_index_t *baseIndices; // [esp+138h] [ebp-180h] BYREF
    float4 flatDelta; // [esp+13Ch] [ebp-17Ch] BYREF
    int segIter; // [esp+14Ch] [ebp-16Ch]
    float4 *baseArgs; // [esp+150h] [ebp-168h]
    const FxBeam *beam; // [esp+154h] [ebp-164h]
    float4 beamWorldEnd; // [esp+158h] [ebp-160h]
    float4 vertPos; // [esp+168h] [ebp-150h]
    GfxColor endColor; // [esp+178h] [ebp-140h]
    float beginRadius; // [esp+17Ch] [ebp-13Ch]
    float wiggleDist; // [esp+180h] [ebp-138h]
    int vertexCount; // [esp+184h] [ebp-134h]
    int segCount; // [esp+188h] [ebp-130h]
    int indexCount; // [esp+18Ch] [ebp-12Ch]
    GfxPackedVertex *verts; // [esp+190h] [ebp-128h]
    float4 beamDot; // [esp+194h] [ebp-124h]
    r_double_index_t *indices; // [esp+1A4h] [ebp-114h]
    float4 beamWorldBegin; // [esp+1A8h] [ebp-110h]
    GfxColor beginColor; // [esp+1B8h] [ebp-100h]
    GfxColor lerpedColor; // [esp+1BCh] [ebp-FCh]
    float4 tpos0; // [esp+1C0h] [ebp-F8h]
    float4 normDelta; // [esp+1D0h] [ebp-E8h] BYREF
    unsigned int argOffset; // [esp+1E0h] [ebp-D8h] BYREF
    unsigned __int16 baseVertex; // [esp+1E4h] [ebp-D4h] BYREF
    float4 perpFlatDelta; // [esp+1E8h] [ebp-D0h] BYREF
    float4 *args; // [esp+1F8h] [ebp-C0h]
    GfxPackedVertex *baseVerts; // [esp+1FCh] [ebp-BCh]
    float4 tpos1; // [esp+200h] [ebp-B8h]
    FxBeamInfo *beamInfo; // [esp+210h] [ebp-A8h]
    float4 viewAxis; // [esp+218h] [ebp-A0h] BYREF
    mat4x4 clipMtx; // [esp+228h] [ebp-90h] BYREF
    int beamIter; // [esp+26Ch] [ebp-4Ch]
    mat4x4 invClipMtx; // [esp+270h] [ebp-48h] BYREF
    int savedregs; // [esp+2B8h] [ebp+0h] BYREF

    if (!cmd)
        MyAssertHandler(".\\EffectsCore\\fx_beam.cpp", 287, 0, "%s", "cmd");
    if (!cmd->beamInfo)
        MyAssertHandler(".\\EffectsCore\\fx_beam.cpp", 288, 0, "%s", "cmd->beamInfo");
    beamInfo = cmd->beamInfo;
    CreateClipMatrix(&clipMtx, cmd->vieworg, cmd->viewaxis);
    MatrixInverse44(clipMtx.x.v, invClipMtx.x.v);
    v18 = cmd->viewaxis[0];
    viewAxis.v[0] = cmd->viewaxis[0][0];
    viewAxis.v[1] = cmd->viewaxis[0][1];
    viewAxis.v[2] = cmd->viewaxis[0][2];
    viewAxis.v[3] = 0.0;
    for (beamIter = 0; beamIter != beamInfo->beamCount; ++beamIter)
    {
        beam = &beamInfo->beams[beamIter];
        beginRadius = beam->beginRadius;
        endRadius = beam->endRadius;
        segCount = beam->segmentCount;
        v21 = g_zero;
        flatDelta = g_zero;
        beamWorldBegin.v[0] = beam->begin[0];
        beamWorldBegin.v[1] = beam->begin[1];
        beamWorldBegin.v[2] = beam->begin[2];
        beamWorldBegin.v[3] = 0.0;
        end = beam->end;
        beamWorldEnd.v[0] = beam->end[0];
        beamWorldEnd.v[1] = beam->end[1];
        beamWorldEnd.v[2] = beam->end[2];
        beamWorldEnd.v[3] = 0.0;
        *(_QWORD *)v1.v = *(_QWORD *)beamWorldBegin.v;
        v1.u[2] = beamWorldBegin.u[2];
        v1.v[3] = 0.0;
        if (FX_GenerateBeam_GetFlatDelta(COERCE_FLOAT(&savedregs), &clipMtx, &invClipMtx, v1, beamWorldEnd, &flatDelta))
        {
            indexCount = 3 * (2 * segCount + 2);
            vertexCount = 2 * segCount + 4;
            if (!R_ReserveCodeMeshIndices(indexCount, &baseIndices)
                || !R_ReserveCodeMeshVerts(vertexCount, &baseVertex)
                || !R_ReserveCodeMeshArgs(2, &argOffset))
            {
                return;
            }
            indices = baseIndices;
            indexPairIter = 0;
            workingIndex.value[0] = templateIndices[0] + baseVertex;
            workingIndex.value[1] = baseVertex + 2;
            *baseIndices = workingIndex;
            ++indexPairIter;
            workingIndex.value[0] = baseVertex + 1;
            for (segIter = 0; segIter != segCount; ++segIter)
            {
                offset = 2 * segIter;
                workingIndex.value[1] = 2 * segIter + baseVertex + 2;
                indices[indexPairIter++] = workingIndex;
                workingIndex.value[0] = offset + baseVertex + 4;
                workingIndex.value[1] = offset + baseVertex + 1;
                indices[indexPairIter++] = workingIndex;
                workingIndex.value[0] = offset + baseVertex + 1;
                workingIndex.value[1] = offset + baseVertex + 4;
                indices[indexPairIter++] = workingIndex;
                workingIndex.value[0] = offset + baseVertex + 3;
            }
            offset = 2 * segCount - 2;
            workingIndex.value[1] = offset + baseVertex + 3;
            indices[indexPairIter++] = workingIndex;
            workingIndex.value[0] = offset + baseVertex + 4;
            workingIndex.value[1] = offset + baseVertex + 5;
            indices[indexPairIter] = workingIndex;
            R_AddCodeMeshDrawSurf(beam->material, baseIndices, indexCount, argOffset, 2u, "Beam");
            baseArgs = (float4 *)R_GetCodeMeshArgs(argOffset);
            baseVerts = R_GetCodeMeshVerts(baseVertex);
            Vec3Cross(flatDelta.v, viewAxis.v, perpFlatDelta.v);
            Vec3NormalizeTo(perpFlatDelta.v, perpFlatDelta.v);
            normDelta.v[0] = beamWorldEnd.v[0] - beamWorldBegin.v[0];
            normDelta.v[1] = beamWorldEnd.v[1] - beamWorldBegin.v[1];
            normDelta.v[2] = beamWorldEnd.v[2] - beamWorldBegin.v[2];
            normDelta.v[3] = beamWorldEnd.v[3] - beamWorldBegin.v[3];
            Vec3NormalizeTo(normDelta.v, normDelta.v);
            v16 = beamWorldBegin.v[0] * normDelta.v[0]
                + beamWorldBegin.v[1] * normDelta.v[1]
                + beamWorldBegin.v[2] * normDelta.v[2];
            beamDot.v[0] = -v16;
            beamDot.v[1] = beamDot.v[0];
            beamDot.v[2] = beamDot.v[0];
            beamDot.v[3] = beamDot.v[0];
            args = baseArgs;
            v20 = g_swizzleXYZA;
            v13[0] = normDelta;
            v13[1] = beamDot;
            v14 = g_swizzleXYZA;
            v15 = baseArgs;
            baseArgs->unitVec[0].array[0] = v13[0].unitVec[0].array[g_swizzleXYZA.unitVec[0].array[3]];
            v15->unitVec[0].array[1] = v13[0].unitVec[0].array[v14.unitVec[0].array[2]];
            v15->unitVec[0].array[2] = v13[0].unitVec[0].array[v14.unitVec[0].array[1]];
            v15->unitVec[0].array[3] = v13[0].unitVec[0].array[v14.unitVec[0].array[0]];
            v15->unitVec[1].array[0] = v13[0].unitVec[0].array[v14.unitVec[1].array[3]];
            v15->unitVec[1].array[1] = v13[0].unitVec[0].array[v14.unitVec[1].array[2]];
            v15->unitVec[1].array[2] = v13[0].unitVec[0].array[v14.unitVec[1].array[1]];
            v15->unitVec[1].array[3] = v13[0].unitVec[0].array[v14.unitVec[1].array[0]];
            v15->unitVec[2].array[0] = v13[0].unitVec[0].array[v14.unitVec[2].array[3]];
            v15->unitVec[2].array[1] = v13[0].unitVec[0].array[v14.unitVec[2].array[2]];
            v15->unitVec[2].array[2] = v13[0].unitVec[0].array[v14.unitVec[2].array[1]];
            v15->unitVec[2].array[3] = v13[0].unitVec[0].array[v14.unitVec[2].array[0]];
            v15->unitVec[3].array[0] = v13[0].unitVec[0].array[v14.unitVec[3].array[3]];
            v15->unitVec[3].array[1] = v13[0].unitVec[0].array[v14.unitVec[3].array[2]];
            v15->unitVec[3].array[2] = v13[0].unitVec[0].array[v14.unitVec[3].array[1]];
            v15->unitVec[3].array[3] = v13[0].unitVec[0].array[v14.unitVec[3].array[0]];
            v19 = g_unit;
            v12 = args + 1;
            args[1].v[0] = 0.0;
            v12->v[1] = v19.v[1];
            v12->v[2] = v19.v[2];
            v12->v[3] = v19.v[3];
            v10 = args + 1;
            v11 = args + 1;
            args[1].v[0] = args[1].v[0] + beamWorldBegin.v[0];
            v10->v[1] = v11->v[1] + beamWorldBegin.v[1];
            v10->v[2] = v11->v[2] + beamWorldBegin.v[2];
            v10->v[3] = v11->v[3] + beamWorldBegin.v[3];
            beginColor.packed = (unsigned int)beam->beginColor;
            endColor.packed = (unsigned int)beam->endColor;
            verts = baseVerts;
            v9 = -beginRadius;
            vertPos.v[0] = v9 * flatDelta.v[0] + beamWorldBegin.v[0];
            vertPos.v[1] = v9 * flatDelta.v[1] + beamWorldBegin.v[1];
            vertPos.v[2] = v9 * flatDelta.v[2] + beamWorldBegin.v[2];
            vertPos.v[3] = v9 * flatDelta.v[3] + beamWorldBegin.v[3];
            baseVerts->xyz[0] = vertPos.v[0];
            verts->xyz[1] = vertPos.v[1];
            verts->xyz[2] = vertPos.v[2];
            verts->color = beginColor;
            verts->binormalSign = 1.0;
            verts->normal.packed = 1073643391;
            verts->tangent.packed = 1065320446;
            verts->texCoord.packed = 0;
            ++verts;
            wiggleDist = beam->wiggleDist;
            lerpedColor.packed = -1;
            for (segIter = 0; segIter <= segCount; ++segIter)
            {
                alpha = (double)segIter / (double)segCount;
                wiggleVec = wiggle[segIter % 8u];
                scaledWiggle.v[0] = wiggleDist * wiggleVec.v[0];
                scaledWiggle.v[1] = wiggleDist * wiggleVec.v[1];
                scaledWiggle.v[2] = wiggleDist * wiggleVec.v[2];
                scaledWiggle.v[3] = wiggleDist * wiggleVec.v[3];
                wiggleXs.v[0] = scaledWiggle.v[0];
                wiggleXs.v[1] = scaledWiggle.v[0];
                wiggleXs.v[2] = scaledWiggle.v[0];
                wiggleXs.v[3] = scaledWiggle.v[0];
                wiggleYs.v[0] = scaledWiggle.v[1];
                wiggleYs.v[1] = scaledWiggle.v[1];
                wiggleYs.v[2] = scaledWiggle.v[1];
                wiggleYs.v[3] = scaledWiggle.v[1];
                for (dim = 0; dim != 4; ++dim)
                {
                    v8 = (int)((double)(endColor.array[dim] - beginColor.array[dim]) * alpha + (double)beginColor.array[dim]);
                    if (v8 >= 0)
                    {
                        if (v8 <= 255)
                            v7 = (int)((double)(endColor.array[dim] - beginColor.array[dim]) * alpha + (double)beginColor.array[dim]);
                        else
                            v7 = -1;
                    }
                    else
                    {
                        v7 = 0;
                    }
                    lerpedColor.array[dim] = v7;
                }
                v3 = beamWorldEnd.v[0] - beamWorldBegin.v[0];
                v4 = beamWorldEnd.v[1] - beamWorldBegin.v[1];
                v5 = beamWorldEnd.v[2] - beamWorldBegin.v[2];
                v6 = beamWorldEnd.v[3] - beamWorldBegin.v[3];
                basePos.v[0] = alpha * v3 + beamWorldBegin.v[0];
                basePos.v[1] = alpha * v4 + beamWorldBegin.v[1];
                basePos.v[2] = alpha * v5 + beamWorldBegin.v[2];
                basePos.v[3] = alpha * v6 + beamWorldBegin.v[3];
                lerpedRadius = (endRadius - beginRadius) * alpha + beginRadius;
                basePos.v[0] = wiggleXs.v[0] * perpFlatDelta.v[0] + basePos.v[0];
                basePos.v[1] = wiggleXs.v[1] * perpFlatDelta.v[1] + basePos.v[1];
                basePos.v[2] = wiggleXs.v[2] * perpFlatDelta.v[2] + basePos.v[2];
                basePos.v[3] = wiggleXs.v[3] * perpFlatDelta.v[3] + basePos.v[3];
                basePos.v[0] = wiggleYs.v[0] * flatDelta.v[0] + basePos.v[0];
                basePos.v[1] = wiggleYs.v[1] * flatDelta.v[1] + basePos.v[1];
                basePos.v[2] = wiggleYs.v[2] * flatDelta.v[2] + basePos.v[2];
                basePos.v[3] = wiggleYs.v[3] * flatDelta.v[3] + basePos.v[3];
                v2 = -lerpedRadius;
                tpos0.v[0] = v2 * perpFlatDelta.v[0] + basePos.v[0];
                tpos0.v[1] = v2 * perpFlatDelta.v[1] + basePos.v[1];
                tpos0.v[2] = v2 * perpFlatDelta.v[2] + basePos.v[2];
                tpos0.v[3] = v2 * perpFlatDelta.v[3] + basePos.v[3];
                tpos1.v[0] = lerpedRadius * perpFlatDelta.v[0] + basePos.v[0];
                tpos1.v[1] = lerpedRadius * perpFlatDelta.v[1] + basePos.v[1];
                tpos1.v[2] = lerpedRadius * perpFlatDelta.v[2] + basePos.v[2];
                tpos1.v[3] = lerpedRadius * perpFlatDelta.v[3] + basePos.v[3];
                verts->xyz[0] = tpos0.v[0];
                verts->xyz[1] = tpos0.v[1];
                verts->xyz[2] = tpos0.v[2];
                verts->color = lerpedColor;
                verts->binormalSign = 1.0;
                verts->normal.packed = 1073643391;
                verts->tangent.packed = 1065320446;
                verts->texCoord.packed = 15360;
                ++verts;
                verts->xyz[0] = tpos1.v[0];
                verts->xyz[1] = tpos1.v[1];
                verts->xyz[2] = tpos1.v[2];
                verts->color = lerpedColor;
                verts->binormalSign = 1.0;
                verts->normal.packed = 1073643391;
                verts->tangent.packed = 1065320446;
                verts->texCoord.packed = 1006632960;
                ++verts;
            }
            tpos0.v[0] = endRadius * flatDelta.v[0] + beamWorldEnd.v[0];
            tpos0.v[1] = endRadius * flatDelta.v[1] + beamWorldEnd.v[1];
            tpos0.v[2] = endRadius * flatDelta.v[2] + beamWorldEnd.v[2];
            tpos0.v[3] = endRadius * flatDelta.v[3] + beamWorldEnd.v[3];
            verts->xyz[0] = tpos0.v[0];
            verts->xyz[1] = tpos0.v[1];
            verts->xyz[2] = tpos0.v[2];
            verts->color = endColor;
            verts->binormalSign = 1.0;
            verts->normal.packed = 1073643391;
            verts->tangent.packed = 1065320446;
            verts->texCoord.packed = 1006648320;
        }
    }
}

void __cdecl CreateClipMatrix(mat4x4 *clipMtx, const float *vieworg, const float (*viewaxis)[3])
{
    unsigned int v3; // [esp+Ch] [ebp-90h]
    unsigned int LocalClientNum; // [esp+10h] [ebp-8Ch]
    mat4x4 viewMtx; // [esp+14h] [ebp-88h] BYREF
    mat4x4 projMtx; // [esp+54h] [ebp-48h] BYREF

    Float4x4ForViewer(&viewMtx, vieworg, viewaxis);
    LocalClientNum = R_GetLocalClientNum();
    if (LocalClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            LocalClientNum);
    v3 = R_GetLocalClientNum();
    if (v3)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\effectscore\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            v3);
    Float4x4InfinitePerspectiveMatrix(&projMtx, MEMORY[0x9D8710], MEMORY[0x9D8714], 1.0);
    clipMtx->x.v[0] = viewMtx.x.v[0] * projMtx.x.v[0]
        + viewMtx.x.v[1] * projMtx.y.v[0]
        + viewMtx.x.v[2] * projMtx.z.v[0]
        + viewMtx.x.v[3] * projMtx.w.v[0];
    clipMtx->x.v[1] = viewMtx.x.v[0] * projMtx.x.v[1]
        + viewMtx.x.v[1] * projMtx.y.v[1]
        + viewMtx.x.v[2] * projMtx.z.v[1]
        + viewMtx.x.v[3] * projMtx.w.v[1];
    clipMtx->x.v[2] = viewMtx.x.v[0] * projMtx.x.v[2]
        + viewMtx.x.v[1] * projMtx.y.v[2]
        + viewMtx.x.v[2] * projMtx.z.v[2]
        + viewMtx.x.v[3] * projMtx.w.v[2];
    clipMtx->x.v[3] = viewMtx.x.v[0] * projMtx.x.v[3]
        + viewMtx.x.v[1] * projMtx.y.v[3]
        + viewMtx.x.v[2] * projMtx.z.v[3]
        + viewMtx.x.v[3] * projMtx.w.v[3];
    clipMtx->y.v[0] = viewMtx.y.v[0] * projMtx.x.v[0]
        + viewMtx.y.v[1] * projMtx.y.v[0]
        + viewMtx.y.v[2] * projMtx.z.v[0]
        + viewMtx.y.v[3] * projMtx.w.v[0];
    clipMtx->y.v[1] = viewMtx.y.v[0] * projMtx.x.v[1]
        + viewMtx.y.v[1] * projMtx.y.v[1]
        + viewMtx.y.v[2] * projMtx.z.v[1]
        + viewMtx.y.v[3] * projMtx.w.v[1];
    clipMtx->y.v[2] = viewMtx.y.v[0] * projMtx.x.v[2]
        + viewMtx.y.v[1] * projMtx.y.v[2]
        + viewMtx.y.v[2] * projMtx.z.v[2]
        + viewMtx.y.v[3] * projMtx.w.v[2];
    clipMtx->y.v[3] = viewMtx.y.v[0] * projMtx.x.v[3]
        + viewMtx.y.v[1] * projMtx.y.v[3]
        + viewMtx.y.v[2] * projMtx.z.v[3]
        + viewMtx.y.v[3] * projMtx.w.v[3];
    clipMtx->z.v[0] = viewMtx.z.v[0] * projMtx.x.v[0]
        + viewMtx.z.v[1] * projMtx.y.v[0]
        + viewMtx.z.v[2] * projMtx.z.v[0]
        + viewMtx.z.v[3] * projMtx.w.v[0];
    clipMtx->z.v[1] = viewMtx.z.v[0] * projMtx.x.v[1]
        + viewMtx.z.v[1] * projMtx.y.v[1]
        + viewMtx.z.v[2] * projMtx.z.v[1]
        + viewMtx.z.v[3] * projMtx.w.v[1];
    clipMtx->z.v[2] = viewMtx.z.v[0] * projMtx.x.v[2]
        + viewMtx.z.v[1] * projMtx.y.v[2]
        + viewMtx.z.v[2] * projMtx.z.v[2]
        + viewMtx.z.v[3] * projMtx.w.v[2];
    clipMtx->z.v[3] = viewMtx.z.v[0] * projMtx.x.v[3]
        + viewMtx.z.v[1] * projMtx.y.v[3]
        + viewMtx.z.v[2] * projMtx.z.v[3]
        + viewMtx.z.v[3] * projMtx.w.v[3];
    clipMtx->w.v[0] = viewMtx.w.v[0] * projMtx.x.v[0]
        + viewMtx.w.v[1] * projMtx.y.v[0]
        + viewMtx.w.v[2] * projMtx.z.v[0]
        + viewMtx.w.v[3] * projMtx.w.v[0];
    clipMtx->w.v[1] = viewMtx.w.v[0] * projMtx.x.v[1]
        + viewMtx.w.v[1] * projMtx.y.v[1]
        + viewMtx.w.v[2] * projMtx.z.v[1]
        + viewMtx.w.v[3] * projMtx.w.v[1];
    clipMtx->w.v[2] = viewMtx.w.v[0] * projMtx.x.v[2]
        + viewMtx.w.v[1] * projMtx.y.v[2]
        + viewMtx.w.v[2] * projMtx.z.v[2]
        + viewMtx.w.v[3] * projMtx.w.v[2];
    clipMtx->w.v[3] = viewMtx.w.v[0] * projMtx.x.v[3]
        + viewMtx.w.v[1] * projMtx.y.v[3]
        + viewMtx.w.v[2] * projMtx.z.v[3]
        + viewMtx.w.v[3] * projMtx.w.v[3];
}

void __cdecl Float4x4ForViewer(mat4x4 *mtx, const float *origin3, const float (*axis3)[3])
{
    float v3; // [esp+8h] [ebp-1E4h]
    float v4; // [esp+Ch] [ebp-1E0h]
    float v5; // [esp+10h] [ebp-1DCh]
    float v6; // [esp+14h] [ebp-1D8h]
    float4 v7[2]; // [esp+1Ch] [ebp-1D0h]
    float4 v8; // [esp+3Ch] [ebp-1B0h]
    float4 *p_z; // [esp+50h] [ebp-19Ch]
    float4 v10[2]; // [esp+54h] [ebp-198h]
    float4 v11; // [esp+74h] [ebp-178h]
    float4 *p_y; // [esp+88h] [ebp-164h]
    _QWORD v13[4]; // [esp+8Ch] [ebp-160h]
    float4 v14; // [esp+ACh] [ebp-140h]
    mat4x4 *v15; // [esp+C0h] [ebp-12Ch]
    float v16[20]; // [esp+C4h] [ebp-128h] BYREF
    float4 v17; // [esp+114h] [ebp-D8h]
    float4 v18; // [esp+124h] [ebp-C8h]
    float4 v19; // [esp+134h] [ebp-B8h]
    float4 origin; // [esp+144h] [ebp-A8h]
    mat4x4 tAxis; // [esp+154h] [ebp-98h] BYREF
    float4 transRow; // [esp+194h] [ebp-58h]
    mat4x4 axis; // [esp+1A4h] [ebp-48h] BYREF

    origin.v[0] = *origin3;
    origin.v[1] = origin3[1];
    origin.v[2] = origin3[2];
    origin.v[3] = 0.0;
    LODWORD(v16[19]) = axis3;
    tAxis.x.v[0] = (*axis3)[0];
    tAxis.x.v[1] = (*axis3)[1];
    tAxis.x.v[2] = (*axis3)[2];
    tAxis.x.v[3] = 0.0;
    tAxis.y.v[0] = (*axis3)[3];
    tAxis.y.v[1] = (*axis3)[4];
    tAxis.y.v[2] = (*axis3)[5];
    tAxis.y.v[3] = 0.0;
    tAxis.z.v[0] = (*axis3)[6];
    tAxis.z.v[1] = (*axis3)[7];
    tAxis.z.v[2] = (*axis3)[8];
    tAxis.z.v[3] = 0.0;
    tAxis.w.v[0] = 0.0;
    tAxis.w.v[1] = 0.0;
    tAxis.w.v[2] = 0.0;
    tAxis.w.v[3] = 1.0;
    LODWORD(v16[17]) = tAxis.y.v;
    LODWORD(v16[18]) = tAxis.y.v;
    tAxis.y.v[0] = -tAxis.y.v[0];
    tAxis.y.v[1] = -tAxis.y.v[1];
    tAxis.y.v[2] = -tAxis.y.v[2];
    tAxis.y.v[3] = -(float)0.0;
    v16[0] = tAxis.x.v[0];
    v16[1] = tAxis.y.v[0];
    v16[2] = tAxis.z.v[0];
    v16[3] = 0.0;
    v16[4] = tAxis.x.v[1];
    v16[5] = tAxis.y.v[1];
    v16[6] = tAxis.z.v[1];
    v16[7] = 0.0;
    v16[8] = tAxis.x.v[2];
    v16[9] = tAxis.y.v[2];
    v16[10] = tAxis.z.v[2];
    v16[11] = 0.0;
    v16[12] = 0.0;
    v16[13] = tAxis.y.v[3];
    v16[14] = 0.0;
    v16[15] = 1.0;
    memcpy(&axis, v16, sizeof(axis));
    v19 = g_swizzleYZXW;
    v13[0] = *(_QWORD *)axis.x.v;
    v13[1] = *(_QWORD *)&axis.x.unitVec[2].packed;
    v13[2] = *(_QWORD *)axis.x.v;
    v13[3] = *(_QWORD *)&axis.x.unitVec[2].packed;
    v14 = g_swizzleYZXW;
    v15 = mtx;
    mtx->x.unitVec[0].array[0] = *((_BYTE *)v13 + g_swizzleYZXW.unitVec[0].array[3]);
    v15->x.unitVec[0].array[1] = *((_BYTE *)v13 + v14.unitVec[0].array[2]);
    v15->x.unitVec[0].array[2] = *((_BYTE *)v13 + v14.unitVec[0].array[1]);
    v15->x.unitVec[0].array[3] = *((_BYTE *)v13 + v14.unitVec[0].array[0]);
    v15->x.unitVec[1].array[0] = *((_BYTE *)v13 + v14.unitVec[1].array[3]);
    v15->x.unitVec[1].array[1] = *((_BYTE *)v13 + v14.unitVec[1].array[2]);
    v15->x.unitVec[1].array[2] = *((_BYTE *)v13 + v14.unitVec[1].array[1]);
    v15->x.unitVec[1].array[3] = *((_BYTE *)v13 + v14.unitVec[1].array[0]);
    v15->x.unitVec[2].array[0] = *((_BYTE *)v13 + v14.unitVec[2].array[3]);
    v15->x.unitVec[2].array[1] = *((_BYTE *)v13 + v14.unitVec[2].array[2]);
    v15->x.unitVec[2].array[2] = *((_BYTE *)v13 + v14.unitVec[2].array[1]);
    v15->x.unitVec[2].array[3] = *((_BYTE *)v13 + v14.unitVec[2].array[0]);
    v15->x.unitVec[3].array[0] = *((_BYTE *)v13 + v14.unitVec[3].array[3]);
    v15->x.unitVec[3].array[1] = *((_BYTE *)v13 + v14.unitVec[3].array[2]);
    v15->x.unitVec[3].array[2] = *((_BYTE *)v13 + v14.unitVec[3].array[1]);
    v15->x.unitVec[3].array[3] = *((_BYTE *)v13 + v14.unitVec[3].array[0]);
    v18 = g_swizzleYZXW;
    v10[0] = axis.y;
    v10[1] = axis.y;
    v11 = g_swizzleYZXW;
    p_y = &mtx->y;
    mtx->y.unitVec[0].array[0] = v10[0].unitVec[0].array[g_swizzleYZXW.unitVec[0].array[3]];
    p_y->unitVec[0].array[1] = v10[0].unitVec[0].array[v11.unitVec[0].array[2]];
    p_y->unitVec[0].array[2] = v10[0].unitVec[0].array[v11.unitVec[0].array[1]];
    p_y->unitVec[0].array[3] = v10[0].unitVec[0].array[v11.unitVec[0].array[0]];
    p_y->unitVec[1].array[0] = v10[0].unitVec[0].array[v11.unitVec[1].array[3]];
    p_y->unitVec[1].array[1] = v10[0].unitVec[0].array[v11.unitVec[1].array[2]];
    p_y->unitVec[1].array[2] = v10[0].unitVec[0].array[v11.unitVec[1].array[1]];
    p_y->unitVec[1].array[3] = v10[0].unitVec[0].array[v11.unitVec[1].array[0]];
    p_y->unitVec[2].array[0] = v10[0].unitVec[0].array[v11.unitVec[2].array[3]];
    p_y->unitVec[2].array[1] = v10[0].unitVec[0].array[v11.unitVec[2].array[2]];
    p_y->unitVec[2].array[2] = v10[0].unitVec[0].array[v11.unitVec[2].array[1]];
    p_y->unitVec[2].array[3] = v10[0].unitVec[0].array[v11.unitVec[2].array[0]];
    p_y->unitVec[3].array[0] = v10[0].unitVec[0].array[v11.unitVec[3].array[3]];
    p_y->unitVec[3].array[1] = v10[0].unitVec[0].array[v11.unitVec[3].array[2]];
    p_y->unitVec[3].array[2] = v10[0].unitVec[0].array[v11.unitVec[3].array[1]];
    p_y->unitVec[3].array[3] = v10[0].unitVec[0].array[v11.unitVec[3].array[0]];
    v17 = g_swizzleYZXW;
    v7[0] = axis.z;
    v7[1] = axis.z;
    v8 = g_swizzleYZXW;
    p_z = &mtx->z;
    mtx->z.unitVec[0].array[0] = v7[0].unitVec[0].array[g_swizzleYZXW.unitVec[0].array[3]];
    p_z->unitVec[0].array[1] = v7[0].unitVec[0].array[v8.unitVec[0].array[2]];
    p_z->unitVec[0].array[2] = v7[0].unitVec[0].array[v8.unitVec[0].array[1]];
    p_z->unitVec[0].array[3] = v7[0].unitVec[0].array[v8.unitVec[0].array[0]];
    p_z->unitVec[1].array[0] = v7[0].unitVec[0].array[v8.unitVec[1].array[3]];
    p_z->unitVec[1].array[1] = v7[0].unitVec[0].array[v8.unitVec[1].array[2]];
    p_z->unitVec[1].array[2] = v7[0].unitVec[0].array[v8.unitVec[1].array[1]];
    p_z->unitVec[1].array[3] = v7[0].unitVec[0].array[v8.unitVec[1].array[0]];
    p_z->unitVec[2].array[0] = v7[0].unitVec[0].array[v8.unitVec[2].array[3]];
    p_z->unitVec[2].array[1] = v7[0].unitVec[0].array[v8.unitVec[2].array[2]];
    p_z->unitVec[2].array[2] = v7[0].unitVec[0].array[v8.unitVec[2].array[1]];
    p_z->unitVec[2].array[3] = v7[0].unitVec[0].array[v8.unitVec[2].array[0]];
    p_z->unitVec[3].array[0] = v7[0].unitVec[0].array[v8.unitVec[3].array[3]];
    p_z->unitVec[3].array[1] = v7[0].unitVec[0].array[v8.unitVec[3].array[2]];
    p_z->unitVec[3].array[2] = v7[0].unitVec[0].array[v8.unitVec[3].array[1]];
    p_z->unitVec[3].array[3] = v7[0].unitVec[0].array[v8.unitVec[3].array[0]];
    mtx->w = g_unit;
    v3 = origin.v[0] * mtx->x.v[0] + origin.v[1] * mtx->y.v[0] + origin.v[2] * mtx->z.v[0] + mtx->w.v[0];
    v4 = origin.v[0] * mtx->x.v[1] + origin.v[1] * mtx->y.v[1] + origin.v[2] * mtx->z.v[1] + mtx->w.v[1];
    v5 = origin.v[0] * mtx->x.v[2] + origin.v[1] * mtx->y.v[2] + origin.v[2] * mtx->z.v[2] + mtx->w.v[2];
    v6 = origin.v[0] * mtx->x.v[3] + origin.v[1] * mtx->y.v[3] + origin.v[2] * mtx->z.v[3] + mtx->w.v[3];
    transRow.v[0] = -v3;
    transRow.v[1] = -v4;
    transRow.v[2] = -v5;
    transRow.v[3] = -v6;
    transRow.v[0] = transRow.v[0] + 0.0;
    transRow.v[1] = transRow.v[1] + 0.0;
    transRow.v[2] = transRow.v[2] + 0.0;
    transRow.v[3] = transRow.v[3] + 2.0;
    mtx->w = transRow;
}

void __cdecl Float4x4InfinitePerspectiveMatrix(mat4x4 *mtx, float tanHalfFovX, float tanHalfFovY, float zNear)
{
    float4 M1; // [esp+0h] [ebp-48h]
    float4 M2; // [esp+10h] [ebp-38h]
    __int64 M3; // [esp+24h] [ebp-24h]
    float M3_8; // [esp+2Ch] [ebp-1Ch]
    __int64 M0; // [esp+34h] [ebp-14h]
    __int64 M0_8; // [esp+3Ch] [ebp-Ch]

    M2.v[0] = 0.0;
    M2.v[1] = 0.0;
    M2.v[2] = 0.99951172;
    M2.v[3] = 1.0;
    *(float *)&M3 = 0.0;
    *((float *)&M3 + 1) = 0.0;
    M3_8 = -zNear * (float)0.99951172;
    *(float *)&M0 = (float)0.99951172 / tanHalfFovX;
    *((float *)&M0 + 1) = 0.0;
    *(float *)&M0_8 = 0.0;
    *((float *)&M0_8 + 1) = 0.0;
    M1.v[0] = 0.0;
    M1.v[1] = (float)0.99951172 / tanHalfFovY;
    M1.v[2] = 0.0;
    M1.v[3] = 0.0;
    *(_QWORD *)mtx->x.v = M0;
    *(_QWORD *)&mtx->x.unitVec[2].packed = M0_8;
    mtx->y = M1;
    mtx->z = M2;
    *(_QWORD *)mtx->w.v = M3;
    mtx->w.v[2] = M3_8;
    mtx->w.v[3] = 0.0;
}

char  FX_GenerateBeam_GetFlatDelta(
    const mat4x4 *clipMtx,
    const mat4x4 *invClipMtx,
    float4 beamWorldBegin,
    float4 beamWorldEnd,
    float4 *outFlatDelta)
{
    float v7; // [esp-14h] [ebp-154h]
    float v8; // [esp-10h] [ebp-150h]
    float v9; // [esp-10h] [ebp-150h]
    float4 v10; // [esp-Ch] [ebp-14Ch] BYREF
    float4 temp_8; // [esp+8h] [ebp-138h]
    float v12; // [esp+18h] [ebp-128h]
    __int64 v13; // [esp+1Ch] [ebp-124h]
    float4 worldSpaceFlatDelta; // [esp+24h] [ebp-11Ch]
    float v15; // [esp+34h] [ebp-10Ch]
    __int64 v16; // [esp+38h] [ebp-108h]
    __int64 v17; // [esp+40h] [ebp-100h]
    float4 v18; // [esp+48h] [ebp-F8h]
    __int64 v19; // [esp+58h] [ebp-E8h]
    __int64 v20; // [esp+60h] [ebp-E0h]
    float clipSpaceFlatDelta_4; // [esp+68h] [ebp-D8h]
    float clipSpaceFlatDelta_8; // [esp+6Ch] [ebp-D4h]
    float clipSpaceFlatDelta_12; // [esp+70h] [ebp-D0h]
    float4 clipSpaceBeamEndDivided; // [esp+74h] [ebp-CCh]
    float v25; // [esp+84h] [ebp-BCh]
    float4 clipSpaceBeamBeginDivided; // [esp+88h] [ebp-B8h] BYREF
    float v27; // [esp+98h] [ebp-A8h]
    float4 clipSpaceBeamEnd; // [esp+9Ch] [ebp-A4h]
    float v29; // [esp+ACh] [ebp-94h]
    float v30; // [esp+B0h] [ebp-90h]
    float v31; // [esp+B4h] [ebp-8Ch]
    float v32; // [esp+B8h] [ebp-88h]
    float v33; // [esp+BCh] [ebp-84h]
    float v34; // [esp+C0h] [ebp-80h] BYREF
    float v35; // [esp+C4h] [ebp-7Ch]
    float v36; // [esp+C8h] [ebp-78h]
    float4 clipSpaceBeamBegin; // [esp+CCh] [ebp-74h]
    float v38; // [esp+DCh] [ebp-64h]
    float v39; // [esp+E0h] [ebp-60h]
    float v40; // [esp+E4h] [ebp-5Ch]
    float v41; // [esp+E8h] [ebp-58h]
    float v42; // [esp+ECh] [ebp-54h]
    float v43; // [esp+F0h] [ebp-50h]
    float v44; // [esp+F4h] [ebp-4Ch]
    float v45; // [esp+F8h] [ebp-48h]
    float worldSpaceBeamEnd[5]; // [esp+FCh] [ebp-44h]
    float v47; // [esp+110h] [ebp-30h]
    float v48; // [esp+114h] [ebp-2Ch]
    float v49; // [esp+118h] [ebp-28h]
    float worldSpaceBeamBegin[9]; // [esp+11Ch] [ebp-24h]
    float retaddr; // [esp+140h] [ebp+0h]

    worldSpaceBeamBegin[6] = a1;
    worldSpaceBeamBegin[7] = retaddr;
    *(float4 *)&worldSpaceBeamBegin[1] = g_unit;
    v47 = beamWorldBegin.v[0] + 0.0;
    v48 = beamWorldBegin.v[1] + 0.0;
    v49 = beamWorldBegin.v[2] + 0.0;
    worldSpaceBeamBegin[0] = beamWorldBegin.v[3] + 1.0;
    *(float4 *)&worldSpaceBeamEnd[1] = g_unit;
    v43 = beamWorldEnd.v[0] + 0.0;
    v44 = beamWorldEnd.v[1] + 0.0;
    v45 = beamWorldEnd.v[2] + 0.0;
    worldSpaceBeamEnd[0] = beamWorldEnd.v[3] + 1.0;
    v39 = v47;
    v40 = v48;
    v41 = v49;
    v42 = worldSpaceBeamBegin[0];
    clipSpaceBeamBegin.v[1] = v47 * clipMtx->x.v[0]
        + v48 * clipMtx->y.v[0]
        + v49 * clipMtx->z.v[0]
        + worldSpaceBeamBegin[0] * clipMtx->w.v[0];
    clipSpaceBeamBegin.v[2] = v47 * clipMtx->x.v[1]
        + v48 * clipMtx->y.v[1]
        + v49 * clipMtx->z.v[1]
        + worldSpaceBeamBegin[0] * clipMtx->w.v[1];
    clipSpaceBeamBegin.v[3] = v47 * clipMtx->x.v[2]
        + v48 * clipMtx->y.v[2]
        + v49 * clipMtx->z.v[2]
        + worldSpaceBeamBegin[0] * clipMtx->w.v[2];
    v38 = v47 * clipMtx->x.v[3] + v48 * clipMtx->y.v[3] + v49 * clipMtx->z.v[3] + worldSpaceBeamBegin[0] * clipMtx->w.v[3];
    v34 = clipSpaceBeamBegin.v[1];
    v35 = clipSpaceBeamBegin.v[2];
    v36 = clipSpaceBeamBegin.v[3];
    clipSpaceBeamBegin.v[0] = v38;
    v30 = v43;
    v31 = v44;
    v32 = v45;
    v33 = worldSpaceBeamEnd[0];
    clipSpaceBeamEnd.v[1] = v43 * clipMtx->x.v[0]
        + v44 * clipMtx->y.v[0]
        + v45 * clipMtx->z.v[0]
        + worldSpaceBeamEnd[0] * clipMtx->w.v[0];
    clipSpaceBeamEnd.v[2] = v43 * clipMtx->x.v[1]
        + v44 * clipMtx->y.v[1]
        + v45 * clipMtx->z.v[1]
        + worldSpaceBeamEnd[0] * clipMtx->w.v[1];
    clipSpaceBeamEnd.v[3] = v43 * clipMtx->x.v[2]
        + v44 * clipMtx->y.v[2]
        + v45 * clipMtx->z.v[2]
        + worldSpaceBeamEnd[0] * clipMtx->w.v[2];
    v29 = v43 * clipMtx->x.v[3] + v44 * clipMtx->y.v[3] + v45 * clipMtx->z.v[3] + worldSpaceBeamEnd[0] * clipMtx->w.v[3];
    clipSpaceBeamBeginDivided.v[2] = clipSpaceBeamEnd.v[1];
    clipSpaceBeamBeginDivided.v[3] = clipSpaceBeamEnd.v[2];
    v27 = clipSpaceBeamEnd.v[3];
    clipSpaceBeamEnd.v[0] = v29;
    if (!Vec4HomogenousClipBothZ((float4 *)&v34, (float4 *)&clipSpaceBeamBeginDivided.unitVec[2]))
        return 0;
    if (clipSpaceBeamBegin.v[0] == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_mat4x4_novec.h", 599, 0, "%s", "in.v[3]");
    clipSpaceBeamBeginDivided.v[1] = 1.0 / clipSpaceBeamBegin.v[0];
    clipSpaceBeamEndDivided.v[2] = v34 * clipSpaceBeamBeginDivided.v[1];
    clipSpaceBeamEndDivided.v[3] = v35 * clipSpaceBeamBeginDivided.v[1];
    v25 = v36 * clipSpaceBeamBeginDivided.v[1];
    clipSpaceBeamBeginDivided.v[0] = 1.0;
    if (clipSpaceBeamEnd.v[0] == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_mat4x4_novec.h", 599, 0, "%s", "in.v[3]");
    clipSpaceBeamEndDivided.v[1] = 1.0 / clipSpaceBeamEnd.v[0];
    clipSpaceFlatDelta_4 = clipSpaceBeamBeginDivided.v[2] * clipSpaceBeamEndDivided.v[1];
    clipSpaceFlatDelta_8 = clipSpaceBeamBeginDivided.v[3] * clipSpaceBeamEndDivided.v[1];
    clipSpaceFlatDelta_12 = v27 * clipSpaceBeamEndDivided.v[1];
    clipSpaceBeamEndDivided.v[0] = 1.0;
    *(float *)&v19 = clipSpaceFlatDelta_4 - clipSpaceBeamEndDivided.v[2];
    *((float *)&v19 + 1) = clipSpaceFlatDelta_8 - clipSpaceBeamEndDivided.v[3];
    *(float *)&v20 = clipSpaceFlatDelta_12 - v25;
    *((float *)&v20 + 1) = (float)1.0 - clipSpaceBeamBeginDivided.v[0];
    v18 = g_keepXYW;
    v19 &= *(_QWORD *)g_keepXYW.v;
    v20 &= *(_QWORD *)&g_keepXYW.unitVec[2].packed;
    v16 = v19;
    v17 = v20;
    worldSpaceFlatDelta.v[1] = *(float *)&v19 * invClipMtx->x.v[0]
        + *((float *)&v19 + 1) * invClipMtx->y.v[0]
        + *(float *)&v20 * invClipMtx->z.v[0]
        + *((float *)&v20 + 1) * invClipMtx->w.v[0];
    worldSpaceFlatDelta.v[2] = *(float *)&v19 * invClipMtx->x.v[1]
        + *((float *)&v19 + 1) * invClipMtx->y.v[1]
        + *(float *)&v20 * invClipMtx->z.v[1]
        + *((float *)&v20 + 1) * invClipMtx->w.v[1];
    worldSpaceFlatDelta.v[3] = *(float *)&v19 * invClipMtx->x.v[2]
        + *((float *)&v19 + 1) * invClipMtx->y.v[2]
        + *(float *)&v20 * invClipMtx->z.v[2]
        + *((float *)&v20 + 1) * invClipMtx->w.v[2];
    v15 = *(float *)&v19 * invClipMtx->x.v[3]
        + *((float *)&v19 + 1) * invClipMtx->y.v[3]
        + *(float *)&v20 * invClipMtx->z.v[3]
        + *((float *)&v20 + 1) * invClipMtx->w.v[3];
    v12 = worldSpaceFlatDelta.v[1];
    v13 = *(_QWORD *)&worldSpaceFlatDelta.unitVec[2].packed;
    worldSpaceFlatDelta.v[0] = v15;
    temp_8 = g_keepXYZ;
    outFlatDelta->u[0] = g_keepXYZ.u[0] & worldSpaceFlatDelta.u[1];
    *(_QWORD *)&outFlatDelta->unitVec[1].packed = *(_QWORD *)&temp_8.unitVec[1].packed & v13;
    outFlatDelta->u[3] = temp_8.u[3] & worldSpaceFlatDelta.u[0];
    v10 = *outFlatDelta;
    if (Vec4LengthSq(v10.v) < 0.000002)
        return 0;
    v8 = outFlatDelta->v[0] * outFlatDelta->v[0]
        + outFlatDelta->v[1] * outFlatDelta->v[1]
        + outFlatDelta->v[2] * outFlatDelta->v[2]
        + outFlatDelta->v[3] * outFlatDelta->v[3];
    if (v8 == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_mat4x4_novec.h", 585, 0, "%s", "len");
    v7 = sqrt(v8);
    v9 = 1.0 / v7;
    outFlatDelta->v[0] = outFlatDelta->v[0] * v9;
    outFlatDelta->v[1] = outFlatDelta->v[1] * v9;
    outFlatDelta->v[2] = outFlatDelta->v[2] * v9;
    outFlatDelta->v[3] = outFlatDelta->v[3] * v9;
    return 1;
}

bool __cdecl Vec4HomogenousClipBothZ(float4 *pt0, float4 *pt1)
{
    float4 v3; // [esp-10h] [ebp-178h]
    float4 coeffZGtW; // [esp+144h] [ebp-24h]
    __int64 coeffZLt0_4; // [esp+158h] [ebp-10h]

    *(float *)&coeffZLt0_4 = 0.0;
    *((float *)&coeffZLt0_4 + 1) = 1.0;
    v3.v[0] = 0.0;
    *(_QWORD *)&v3.unitVec[1].packed = coeffZLt0_4;
    v3.v[3] = 0.0;
    if (!Vec4HomogenousClipZW(pt0, pt1, v3))
        return 0;
    coeffZGtW.v[1] = 0.0;
    coeffZGtW.v[0] = 0.0;
    coeffZGtW.v[3] = 1.0;
    coeffZGtW.v[2] = -1.0;
    return Vec4HomogenousClipZW(pt0, pt1, coeffZGtW);
}

bool __cdecl Vec4HomogenousClipZW(float4 *pt0, float4 *pt1, float4 coeffZW)
{
    float v4; // [esp+4h] [ebp-D8h]
    float v5; // [esp+8h] [ebp-D4h]
    float v6; // [esp+Ch] [ebp-D0h]
    float v7; // [esp+10h] [ebp-CCh]
    int v8; // [esp+14h] [ebp-C8h]
    int v9; // [esp+18h] [ebp-C4h]
    int v10; // [esp+1Ch] [ebp-C0h]
    int v11; // [esp+20h] [ebp-BCh]
    int v12; // [esp+24h] [ebp-B8h]
    int v13; // [esp+28h] [ebp-B4h]
    int v14; // [esp+2Ch] [ebp-B0h]
    int v15; // [esp+30h] [ebp-ACh]
    float v16; // [esp+3Ch] [ebp-A0h]
    float v17; // [esp+40h] [ebp-9Ch]
    __int64 dist1Cmp; // [esp+44h] [ebp-98h]
    __int64 dist1Cmp_8; // [esp+4Ch] [ebp-90h]
    float clipped; // [esp+58h] [ebp-84h]
    __int64 clippeda; // [esp+58h] [ebp-84h]
    float clipped_4; // [esp+5Ch] [ebp-80h]
    float clipped_8; // [esp+60h] [ebp-7Ch]
    __int64 clipped_8a; // [esp+60h] [ebp-7Ch]
    float clipped_12; // [esp+64h] [ebp-78h]
    __int64 dist1Sel1; // [esp+78h] [ebp-64h]
    __int64 dist1Sel1_8; // [esp+80h] [ebp-5Ch]
    __int64 dist0Cmp; // [esp+98h] [ebp-44h]
    __int64 dist0Cmp_8; // [esp+A0h] [ebp-3Ch]
    float alpha; // [esp+B8h] [ebp-24h]
    float alphaa; // [esp+B8h] [ebp-24h]
    float alpha_4; // [esp+BCh] [ebp-20h]
    float alpha_8; // [esp+C0h] [ebp-1Ch]
    float alpha_12; // [esp+C4h] [ebp-18h]
    __int64 dist1Sel0_8; // [esp+D0h] [ebp-Ch]

    v17 = coeffZW.v[0] * pt0->v[0] + coeffZW.v[1] * pt0->v[1] + coeffZW.v[2] * pt0->v[2] + coeffZW.v[3] * pt0->v[3];
    v16 = coeffZW.v[0] * pt1->v[0] + coeffZW.v[1] * pt1->v[1] + coeffZW.v[2] * pt1->v[2] + coeffZW.v[3] * pt1->v[3];
    if (v17 >= 0.0)
        v15 = 0;
    else
        v15 = -1;
    LODWORD(dist0Cmp) = v15;
    if (v17 >= 0.0)
        v14 = 0;
    else
        v14 = -1;
    HIDWORD(dist0Cmp) = v14;
    if (v17 >= 0.0)
        v13 = 0;
    else
        v13 = -1;
    LODWORD(dist0Cmp_8) = v13;
    if (v17 >= 0.0)
        v12 = 0;
    else
        v12 = -1;
    HIDWORD(dist0Cmp_8) = v12;
    alpha = v17 - v16;
    if (v16 >= 0.0)
        v11 = 0;
    else
        v11 = -1;
    LODWORD(dist1Cmp) = v11;
    if (v16 >= 0.0)
        v10 = 0;
    else
        v10 = -1;
    HIDWORD(dist1Cmp) = v10;
    if (v16 >= 0.0)
        v9 = 0;
    else
        v9 = -1;
    LODWORD(dist1Cmp_8) = v9;
    if (v16 >= 0.0)
        v8 = 0;
    else
        v8 = -1;
    HIDWORD(dist1Cmp_8) = v8;
    if (alpha == 0.0)
        v7 = 3.4028235e38;
    else
        v7 = 1.0 / alpha;
    if (alpha == 0.0)
        v6 = 3.4028235e38;
    else
        v6 = 1.0 / alpha;
    if (alpha == 0.0)
        v5 = 3.4028235e38;
    else
        v5 = I_fres(alpha);
    if (alpha == 0.0)
        v4 = 3.4028235e38;
    else
        v4 = I_fres(alpha);
    alphaa = v17 * v7;
    alpha_4 = v17 * v6;
    alpha_8 = v17 * v5;
    alpha_12 = v17 * v4;
    clipped = pt1->v[0] - pt0->v[0];
    clipped_4 = pt1->v[1] - pt0->v[1];
    clipped_8 = pt1->v[2] - pt0->v[2];
    clipped_12 = pt1->v[3] - pt0->v[3];
    *(float *)&clippeda = alphaa * clipped + pt0->v[0];
    *((float *)&clippeda + 1) = alpha_4 * clipped_4 + pt0->v[1];
    *(float *)&clipped_8a = alpha_8 * clipped_8 + pt0->v[2];
    *((float *)&clipped_8a + 1) = alpha_12 * clipped_12 + pt0->v[3];
    dist1Sel0_8 = *(_QWORD *)&pt0->unitVec[2].packed & dist1Cmp_8 | clipped_8a & ~dist1Cmp_8;
    dist1Sel1 = clippeda & dist1Cmp | *(_QWORD *)pt1->v & ~dist1Cmp;
    dist1Sel1_8 = clipped_8a & dist1Cmp_8 | *(_QWORD *)&pt1->unitVec[2].packed & ~dist1Cmp_8;
    *(_QWORD *)pt0->v = (*(_QWORD *)pt0->v & dist1Cmp | clippeda & ~dist1Cmp) & dist0Cmp | *(_QWORD *)pt0->v & ~dist0Cmp;
    *(_QWORD *)&pt0->unitVec[2].packed = dist1Sel0_8 & dist0Cmp_8 | *(_QWORD *)&pt0->unitVec[2].packed & ~dist0Cmp_8;
    *(_QWORD *)pt1->v = *(_QWORD *)pt1->v & dist0Cmp | dist1Sel1 & ~dist0Cmp;
    *(_QWORD *)&pt1->unitVec[2].packed = *(_QWORD *)&pt1->unitVec[2].packed & dist0Cmp_8 | dist1Sel1_8 & ~dist0Cmp_8;
    return (v11 & v15) == 0;
}

void __cdecl FX_Beam_Begin()
{
    g_beamInfo.beamCount = 0;
}

void __cdecl FX_Beam_Add(FxBeam *beam)
{
    if (g_beamInfo.beamCount != 96)
        memcpy(&g_beamInfo.beams[g_beamInfo.beamCount++], beam, sizeof(g_beamInfo.beams[g_beamInfo.beamCount++]));
}

FxBeamInfo *__cdecl FX_Beam_GetInfo()
{
    return &g_beamInfo;
}

