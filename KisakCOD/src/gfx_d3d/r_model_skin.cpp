#include "r_model_skin.h"
#include "r_init.h"
#include "r_dvars.h"
#include <qcommon/com_playerprofile.h>
#include "r_buffers.h"
#include <universal/profile.h>
#include "r_dobj_skin.h"


//unsigned int const *const g_shortBoneWeightPerm__uint4 820f4950     gfx_d3d : r_model_skin.obj
//struct __vector4 const g_shortBoneWeightPerm 85b981d0     gfx_d3d : r_model_skin.obj


void R_SkinXModelCmd(_WORD *data)
{
    if (dx.deviceLost) return;

    Profile_Begin(89);

    bool sseEnabled = sys_SSE->current.enabled && r_sse_skinning->current.enabled;
    bool sseStateUsed = false;

    SkinXModelCmd* cmd = (SkinXModelCmd*)data;

    iassert(cmd);
    iassert(cmd->surfCount);

    unsigned int* surfPos = (unsigned int*)cmd->modelSurfs;

    int boneIndex = -1;
    DObjSkelMat __declspec(align(16)) boneSkelMats[128];
    memset(boneSkelMats, 0, sizeof(boneSkelMats));

    for (unsigned int i = 0; i < cmd->surfCount; i++)
    {
        if (*(int*)surfPos == -3)
        {
            ++surfPos;
        }
        else
        {
            GfxModelSkinnedSurface* skinnedSurf = (GfxModelSkinnedSurface*)surfPos;
            if (boneIndex != skinnedSurf->info.boneIndex)
            {
                boneIndex = skinnedSurf->info.boneIndex;
                const DObjAnimMat* baseMats = &skinnedSurf->info.baseMat[-boneIndex];
                for (unsigned int j = boneIndex; j < boneIndex + skinnedSurf->info.boneCount; j++)
                {
                    if (!(cmd->surfacePartBits[j >> 5] & (0x80000000 >> (j & 0x1F)))) continue;

                    if (sseStateUsed)
                    {
                        sseStateUsed = false;
                        /*_m_empty()*/;
                    }

                    DObjSkelMat mat0, mat1;
#if 0
                    ConvertQuatToInverseSkelMat(&baseMats[j], &mat0);
                    ConvertQuatToInverseSkelMat(&cmd->mat[j], &mat1);
#else
                    //DObjSkelMat mat0_c, mat1_c;
                    //ConvertQuatToInverseSkelMat(&baseMats[j], &mat0_c);
                    //ConvertQuatToInverseSkelMat(&cmd->mat[j], &mat1_c);

                    // NOTE(mrsteyk): never let bro decompile again
                    {
                        auto baseMats_j = &baseMats[j];
                        // Lines 581, 582
                        iassert(!IS_NAN(baseMats_j->quat[0]) && !IS_NAN(baseMats_j->quat[1]) && !IS_NAN(baseMats_j->quat[2]) && !IS_NAN(baseMats_j->quat[3]));
                        iassert(!IS_NAN(baseMats_j->transWeight));

                        float quatScaled[3];
                        Vec3Scale(baseMats_j->quat, baseMats_j->transWeight, quatScaled);

                        float v37 = quatScaled[0] * baseMats_j->quat[0];
                        float v36 = quatScaled[0] * baseMats_j->quat[1];
                        float v35 = quatScaled[0] * baseMats_j->quat[2];
                        float v34 = quatScaled[0] * baseMats_j->quat[3];
                        
                        float v33 = quatScaled[1] * baseMats_j->quat[1];
                        float v32 = quatScaled[1] * baseMats_j->quat[2];
                        float v31 = quatScaled[1] * baseMats_j->quat[3];
                        
                        float v30 = quatScaled[2] * baseMats_j->quat[2];
                        float v29 = quatScaled[2] * baseMats_j->quat[3];

                        mat0.axis[0][0] = 1.0 - (v33 + v30);
                        mat0.axis[0][1] = v36 - v29;
                        mat0.axis[0][2] = v35 + v31;
                        mat0.axis[0][3] = 0.0;
                        mat0.axis[1][0] = v36 + v29;
                        mat0.axis[1][1] = 1.0 - (v37 + v30);
                        mat0.axis[1][2] = v32 - v34;
                        mat0.axis[1][3] = 0.0;
                        mat0.axis[2][0] = v35 - v31;
                        mat0.axis[2][1] = v32 + v34;
                        mat0.axis[2][2] = 1.0 - (v37 + v33);
                        mat0.axis[2][3] = 0.0;
                        mat0.origin[0] = -(baseMats_j->trans[0] * mat0.axis[0][0]
                            + baseMats_j->trans[1] * mat0.axis[1][0]
                            + baseMats_j->trans[2] * mat0.axis[2][0]);
                        mat0.origin[1] = -(baseMats_j->trans[0] * mat0.axis[0][1]
                            + baseMats_j->trans[1] * mat0.axis[1][1]
                            + baseMats_j->trans[2] * mat0.axis[2][1]);
                        mat0.origin[2] = -(baseMats_j->trans[0] * mat0.axis[0][2]
                            + baseMats_j->trans[1] * mat0.axis[1][2]
                            + baseMats_j->trans[2] * mat0.axis[2][2]);
                        mat0.origin[3] = 1.0;
                    }
                    {
                        auto mat_j = &cmd->mat[j];
                        // Lines 473, 474
                        iassert(!IS_NAN(mat_j->quat[0]) && !IS_NAN(mat_j->quat[1]) && !IS_NAN(mat_j->quat[2]) && !IS_NAN(mat_j->quat[3]));
                        iassert(!IS_NAN(mat_j->transWeight));

                        float quatScaled[3];
                        Vec3Scale(mat_j->quat, mat_j->transWeight, quatScaled);

                        float v20 = quatScaled[0] * mat_j->quat[0];
                        float v19 = quatScaled[0] * mat_j->quat[1];
                        float v18 = quatScaled[0] * mat_j->quat[2];
                        float v17 = quatScaled[0] * mat_j->quat[3];

                        float v16 = quatScaled[1] * mat_j->quat[1];
                        float v15 = quatScaled[1] * mat_j->quat[2];
                        float v14 = quatScaled[1] * mat_j->quat[3];
                        
                        float v13 = quatScaled[2] * mat_j->quat[2];
                        float v12 = quatScaled[2] * mat_j->quat[3];
                        
                        mat1.axis[0][0] = 1.0 - (v16 + v13);
                        mat1.axis[0][1] = v19 + v12;
                        mat1.axis[0][2] = v18 - v14;
                        mat1.axis[0][3] = 0.0;
                        mat1.axis[1][0] = v19 - v12;
                        mat1.axis[1][1] = 1.0 - (v20 + v13);
                        mat1.axis[1][2] = v15 + v17;
                        mat1.axis[1][3] = 0.0;
                        mat1.axis[2][0] = v18 + v14;
                        mat1.axis[2][1] = v15 - v17;
                        mat1.axis[2][2] = 1.0 - (v20 + v16);
                        mat1.axis[2][3] = 0.0;
                        mat1.origin[0] = mat_j->trans[0];
                        mat1.origin[1] = mat_j->trans[1];
                        mat1.origin[2] = mat_j->trans[2];
                        mat1.origin[3] = 1.0;
                    }

                    //iassert(!memcmp(&mat0, &mat0_c, sizeof(mat0)));
                    //iassert(!memcmp(&mat1, &mat1_c, sizeof(mat1)));
#endif
                    R_MultiplySkelMat(&mat0, &mat1, &boneSkelMats[j]);
                    boneSkelMats[j].axis[0][3] = 0.0;
                    boneSkelMats[j].axis[1][3] = 0.0;
                    boneSkelMats[j].axis[2][3] = 0.0;
                    boneSkelMats[j].origin[3] = 1.0;
                }
            }

            GfxPackedVertex* skinnedVert = 0;

            if (skinnedSurf->skinnedCachedOffset == -2)
            {
                surfPos += 14;
            }
            else
            {
                iassert(skinnedSurf->xsurf);

                if (skinnedSurf->skinnedCachedOffset < 0)
                {
                    iassert(((reinterpret_cast<uint>(skinnedSurf->skinnedVert) & 15) == 0));
                    skinnedVert = skinnedSurf->skinnedVert;
                }
                else
                {
                    iassert(gfxBuf.skinnedCacheLockAddr);
                    iassert(((reinterpret_cast<uint>(gfxBuf.skinnedCacheLockAddr) & 15) == 0));
                    iassert(((skinnedSurf->skinnedCachedOffset & 15) == 0));
                    skinnedVert = (GfxPackedVertex*)&gfxBuf.skinnedCacheLockAddr[skinnedSurf->skinnedCachedOffset];
                }

                // LWSS: this makes the viewmodels flicker. Decomp is not fully accurate (should just have templated functions)
                //if (sseEnabled)
                //{
                //    if (!sseStateUsed)
                //    {
                //        sseStateUsed = true;
                //        //_m_empty();
                //    }
                //
                //    GfxPackedVertexNormal *skinVertNormalIn = 0, *skinVertNormalOut = 0;
                //    if (gfxBuf.fastSkin)
                //    {
                //        if (skinnedSurf->skinnedCachedOffset >= 0)
                //            skinVertNormalOut = &gfxBuf.skinnedCacheNormalsAddr[skinnedSurf->skinnedCachedOffset >> 5];
                //        if (skinnedSurf->skinnedVert)
                //            skinVertNormalIn = &gfxBuf.oldSkinnedCacheNormalsAddr[(int)skinnedSurf->skinnedVert >> 5];
                //    }
                //    R_SkinXSurfaceSkinnedSse(skinnedSurf->xsurf, &boneSkelMats[boneIndex], skinVertNormalIn, skinVertNormalOut, skinnedVert);
                //}
                //else
                //{
                    R_SkinXSurfaceSkinned(skinnedSurf->xsurf, &boneSkelMats[boneIndex], skinnedVert);
                //}

                surfPos += (sizeof(GfxModelSkinnedSurface) / 4);
            }
        }
    }

    if (sseStateUsed)
        /*_m_empty()*/;

    Profile_EndInternal(0);
}


void __cdecl R_SkinXSurfaceSkinned(
    const XSurface *xsurf,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *skinVerticesOut)
{
    if (xsurf->deformed)
        R_SkinXSurfaceWeight(xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVerticesOut);
    else
        R_SkinXSurfaceRigid(xsurf, xsurf->vertCount, boneMatrix, skinVerticesOut);
}

void __cdecl R_SkinXSurfaceWeight(
    const GfxPackedVertex *inVerts,
    const XSurfaceVertexInfo *vertexInfo,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *outVerts)
{
    const unsigned __int16 *vertsBlend; // [esp+30h] [ebp-8h]
    int vertIndex; // [esp+34h] [ebp-4h]

    //Profile_Begin(111);
    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if (vertexInfo->vertCount[0])
    {
        R_SkinXSurfaceWeight0(inVerts, vertsBlend, vertexInfo->vertCount[0], boneMatrix, outVerts);
        vertIndex = vertexInfo->vertCount[0];
        vertsBlend += vertIndex;
    }
    if (vertexInfo->vertCount[1])
    {
        R_SkinXSurfaceWeight1(&inVerts[vertIndex], vertsBlend, vertexInfo->vertCount[1], boneMatrix, &outVerts[vertIndex]);
        vertIndex += vertexInfo->vertCount[1];
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if (vertexInfo->vertCount[2])
    {
        R_SkinXSurfaceWeight2(&inVerts[vertIndex], vertsBlend, vertexInfo->vertCount[2], boneMatrix, &outVerts[vertIndex]);
        vertIndex += vertexInfo->vertCount[2];
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if (vertexInfo->vertCount[3])
        R_SkinXSurfaceWeight3(&inVerts[vertIndex], vertsBlend, vertexInfo->vertCount[3], boneMatrix, &outVerts[vertIndex]);
    //Profile_EndInternal(0);
}


void __cdecl MatrixTransformVertexAndBasis(
    const float *offset,
    float binormalSign,
    PackedUnitVec normal,
    PackedUnitVec tangent,
    const DObjSkelMat *mat,
    GfxPackedVertex *vert)
{
    float rotated[3]; // [esp+B8h] [ebp-18h]
    float unpacked[3]; // [esp+C4h] [ebp-Ch]

    vert->xyz[0] = *offset * mat->axis[0][0] + offset[1] * mat->axis[1][0] + offset[2] * mat->axis[2][0] + mat->origin[0];
    vert->xyz[1] = *offset * mat->axis[0][1] + offset[1] * mat->axis[1][1] + offset[2] * mat->axis[2][1] + mat->origin[1];
    vert->xyz[2] = *offset * mat->axis[0][2] + offset[1] * mat->axis[1][2] + offset[2] * mat->axis[2][2] + mat->origin[2];
    vert->binormalSign = binormalSign;

    Vec3UnpackUnitVec(normal, (float *)&unpacked);
    rotated[0] = unpacked[0] * mat->axis[0][0] + unpacked[1] * mat->axis[1][0] + unpacked[2] * mat->axis[2][0];
    rotated[1] = unpacked[0] * mat->axis[0][1] + unpacked[1] * mat->axis[1][1] + unpacked[2] * mat->axis[2][1];
    rotated[2] = unpacked[0] * mat->axis[0][2] + unpacked[1] * mat->axis[1][2] + unpacked[2] * mat->axis[2][2];
    normal.array[0] = (int)(rotated[0] * 127.0 + 127.5);
    normal.array[1] = (int)(rotated[1] * 127.0 + 127.5);
    normal.array[2] = (int)(rotated[2] * 127.0 + 127.5);
    normal.array[3] = 63;
    vert->normal = normal;

    Vec3UnpackUnitVec(tangent, unpacked);
    rotated[0] = unpacked[0] * mat->axis[0][0] + unpacked[1] * mat->axis[1][0] + unpacked[2] * mat->axis[2][0];
    rotated[1] = unpacked[0] * mat->axis[0][1] + unpacked[1] * mat->axis[1][1] + unpacked[2] * mat->axis[2][1];
    rotated[2] = unpacked[0] * mat->axis[0][2] + unpacked[1] * mat->axis[1][2] + unpacked[2] * mat->axis[2][2];
    tangent.array[0] = (int)(rotated[0] * 127.0 + 127.5);
    tangent.array[1] = (int)(rotated[1] * 127.0 + 127.5);
    tangent.array[2] = (int)(rotated[2] * 127.0 + 127.5);
    tangent.array[3] = 63;
    vert->tangent = tangent;
}

void __cdecl R_SkinXSurfaceWeight0(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut)
{
    int vertIndex; // [esp+1Ch] [ebp-4h]

    if (!vertsOut)
        MyAssertHandler(".\\r_model_skin.cpp", 632, 0, "%s", "vertsOut");
    vertIndex = 0;
    while (vertIndex < vertCount)
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        ++vertexBlend;
        ++vertsOut;
    }
}

void __cdecl Vec2UnpackTexCoords(PackedTexCoords in, float *out)
{
    float v2; // [esp+0h] [ebp-20h]
    float v3; // [esp+10h] [ebp-10h]

    if (HIWORD(in.packed))
        LODWORD(v3) = (HIWORD(in.packed) << 16) & 0x80000000
        | (((((HIWORD(in.packed) << 14) & 0xFFFC000) - (~(HIWORD(in.packed) << 14) & 0x10000000)) ^ 0x80000000) >> 1);
    else
        v3 = 0.0;
    *out = v3;
    if (LOWORD(in.packed))
        LODWORD(v2) = (LOWORD(in.packed) << 16) & 0x80000000
        | (((((LOWORD(in.packed) << 14) & 0xFFFC000) - (~(LOWORD(in.packed) << 14) & 0x10000000)) ^ 0x80000000) >> 1);
    else
        v2 = 0.0;
    out[1] = v2;
}

void __cdecl Vec3UnpackUnitVec(PackedUnitVec in, float *out)
{
    float decodeScale; // [esp+10h] [ebp-4h]

    decodeScale = ((double)in.array[3] - -192.0) / 32385.0;
    out[0] = ((double)in.array[0] - 127.0) * decodeScale;
    out[1] = ((double)in.array[1] - 127.0) * decodeScale;
    out[2] = ((double)in.array[2] - 127.0) * decodeScale;
}

void __cdecl R_SkinXSurfaceWeight1(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut)
{
    const DObjSkelMat *matrix; // [esp+18h] [ebp-1Ch]
    float offset[3]; // [esp+20h] [ebp-14h] BYREF
    float boneWeight; // [esp+2Ch] [ebp-8h]
    int vertIndex; // [esp+30h] [ebp-4h]

    if (!vertsOut)
        MyAssertHandler(".\\r_model_skin.cpp", 655, 0, "%s", "vertsOut");
    vertIndex = 0;
    while (vertIndex < vertCount)
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const DObjSkelMat *)((char *)boneMatrix + vertexBlend[1]);
        boneWeight = (double)vertexBlend[2] * 0.0000152587890625;
        LocalTransformVector(vertsIn->xyz, (const float4 *)matrix, offset);
        Vec3Scale(offset, boneWeight, offset);
        boneWeight = 1.0 - boneWeight;
        Vec3Mad(offset, boneWeight, vertsOut->xyz, vertsOut->xyz);
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 3;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceWeight2(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut)
{
    const DObjSkelMat *matrix; // [esp+1Ch] [ebp-2Ch]
    const DObjSkelMat *matrixa; // [esp+1Ch] [ebp-2Ch]
    float offset[3]; // [esp+24h] [ebp-24h] BYREF
    float boneWeight; // [esp+30h] [ebp-18h]
    float totalOffset[3]; // [esp+34h] [ebp-14h] BYREF
    int vertIndex; // [esp+40h] [ebp-8h]
    float totalBoneWeight; // [esp+44h] [ebp-4h]

    if (!vertsOut)
        MyAssertHandler(".\\r_model_skin.cpp", 688, 0, "%s", "vertsOut");
    vertIndex = 0;
    while (vertIndex < vertCount)
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const DObjSkelMat *)((char *)boneMatrix + vertexBlend[1]);
        totalBoneWeight = (double)vertexBlend[2] * 0.0000152587890625;
        LocalTransformVector(vertsIn->xyz, (const float4 *)matrix, totalOffset);
        Vec3Scale(totalOffset, totalBoneWeight, totalOffset);
        matrixa = (const DObjSkelMat *)((char *)boneMatrix + vertexBlend[3]);
        boneWeight = (double)vertexBlend[4] * 0.0000152587890625;
        totalBoneWeight = totalBoneWeight + boneWeight;
        LocalTransformVector(vertsIn->xyz, (const float4 *)matrixa, offset);
        Vec3Mad(totalOffset, boneWeight, offset, totalOffset);
        boneWeight = 1.0 - totalBoneWeight;
        Vec3Mad(totalOffset, boneWeight, vertsOut->xyz, vertsOut->xyz);
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 5;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceWeight3(
    const GfxPackedVertex *vertsIn,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertsOut)
{
    const DObjSkelMat *matrix; // [esp+20h] [ebp-2Ch]
    const DObjSkelMat *matrixa; // [esp+20h] [ebp-2Ch]
    const DObjSkelMat *matrixb; // [esp+20h] [ebp-2Ch]
    float offset[3]; // [esp+28h] [ebp-24h] BYREF
    float boneWeight; // [esp+34h] [ebp-18h]
    float totalOffset[3]; // [esp+38h] [ebp-14h] BYREF
    int vertIndex; // [esp+44h] [ebp-8h]
    float totalBoneWeight; // [esp+48h] [ebp-4h]

    if (!vertsOut)
        MyAssertHandler(".\\r_model_skin.cpp", 727, 0, "%s", "vertsOut");
    vertIndex = 0;
    while (vertIndex < vertCount)
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const DObjSkelMat *)((char *)boneMatrix + vertexBlend[1]);
        totalBoneWeight = (double)vertexBlend[2] * 0.0000152587890625;
        LocalTransformVector(vertsIn->xyz, (const float4 *)matrix, totalOffset);
        Vec3Scale(totalOffset, totalBoneWeight, totalOffset);
        matrixa = (const DObjSkelMat *)((char *)boneMatrix + vertexBlend[3]);
        boneWeight = (double)vertexBlend[4] * 0.0000152587890625;
        totalBoneWeight = totalBoneWeight + boneWeight;
        LocalTransformVector(vertsIn->xyz, (const float4 *)matrixa, offset);
        Vec3Mad(totalOffset, boneWeight, offset, totalOffset);
        matrixb = (const DObjSkelMat *)((char *)boneMatrix + vertexBlend[5]);
        boneWeight = (double)vertexBlend[6] * 0.0000152587890625;
        totalBoneWeight = totalBoneWeight + boneWeight;
        LocalTransformVector(vertsIn->xyz, (const float4 *)matrixb, offset);
        Vec3Mad(totalOffset, boneWeight, offset, totalOffset);
        boneWeight = 1.0 - totalBoneWeight;
        Vec3Mad(totalOffset, boneWeight, vertsOut->xyz, vertsOut->xyz);
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 7;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceRigid(
    const XSurface *surf,
    int totalVertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *vertices)
{
    unsigned int i; // [esp+44h] [ebp-1Ch]
    int vertCount; // [esp+48h] [ebp-18h]
    int vertIndex; // [esp+4Ch] [ebp-14h]
    GfxPackedVertex *vertex; // [esp+50h] [ebp-10h]
    const XRigidVertList *vertList; // [esp+54h] [ebp-Ch]
    GfxPackedVertex *v; // [esp+58h] [ebp-8h]
    const DObjSkelMat *bone; // [esp+5Ch] [ebp-4h]

    if (!vertices)
        MyAssertHandler(".\\r_model_skin.cpp", 1940, 0, "%s", "vertices");
    if (((unsigned __int8)vertices & 0xF) != 0)
        MyAssertHandler(".\\r_model_skin.cpp", 1941, 0, "%s", "!(reinterpret_cast< unsigned >( vertices ) & 15)");
    if (((unsigned __int8)boneMatrix & 0xF) != 0)
        MyAssertHandler(".\\r_model_skin.cpp", 1942, 0, "%s", "!(reinterpret_cast< unsigned >( boneMatrix ) & 15)");
    //Profile_Begin(111);
    v = surf->verts0;
    vertex = vertices;
    for (i = 0; i < surf->vertListCount; ++i)
    {
        vertList = &surf->vertList[i];
        vertCount = vertList->vertCount;
        bone = (const DObjSkelMat *)((char *)boneMatrix + vertList->boneOffset);
        for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
        {
            MatrixTransformVertexAndBasis(v->xyz, v->binormalSign, v->normal, v->tangent, bone, vertex);
            vertex->color.packed = v->color.packed;
            vertex->texCoord.packed = v->texCoord.packed;
            ++v;
            ++vertex;
        }
    }
    if (vertex - vertices != totalVertCount)
        MyAssertHandler(".\\r_model_skin.cpp", 1963, 0, "%s", "vertex - vertices == totalVertCount");
    //Profile_EndInternal(0);
}

void __cdecl R_MultiplySkelMat(const DObjSkelMat *mat0, const DObjSkelMat *mat1, DObjSkelMat *out)
{
    out->axis[0][0] = mat0->axis[0][0] * mat1->axis[0][0]
        + mat0->axis[0][1] * mat1->axis[1][0]
        + mat0->axis[0][2] * mat1->axis[2][0];
    out->axis[0][1] = mat0->axis[0][0] * mat1->axis[0][1]
        + mat0->axis[0][1] * mat1->axis[1][1]
        + mat0->axis[0][2] * mat1->axis[2][1];
    out->axis[0][2] = mat0->axis[0][0] * mat1->axis[0][2]
        + mat0->axis[0][1] * mat1->axis[1][2]
        + mat0->axis[0][2] * mat1->axis[2][2];
    out->axis[1][0] = mat0->axis[1][0] * mat1->axis[0][0]
        + mat0->axis[1][1] * mat1->axis[1][0]
        + mat0->axis[1][2] * mat1->axis[2][0];
    out->axis[1][1] = mat0->axis[1][0] * mat1->axis[0][1]
        + mat0->axis[1][1] * mat1->axis[1][1]
        + mat0->axis[1][2] * mat1->axis[2][1];
    out->axis[1][2] = mat0->axis[1][0] * mat1->axis[0][2]
        + mat0->axis[1][1] * mat1->axis[1][2]
        + mat0->axis[1][2] * mat1->axis[2][2];
    out->axis[2][0] = mat0->axis[2][0] * mat1->axis[0][0]
        + mat0->axis[2][1] * mat1->axis[1][0]
        + mat0->axis[2][2] * mat1->axis[2][0];
    out->axis[2][1] = mat0->axis[2][0] * mat1->axis[0][1]
        + mat0->axis[2][1] * mat1->axis[1][1]
        + mat0->axis[2][2] * mat1->axis[2][1];
    out->axis[2][2] = mat0->axis[2][0] * mat1->axis[0][2]
        + mat0->axis[2][1] * mat1->axis[1][2]
        + mat0->axis[2][2] * mat1->axis[2][2];
    out->origin[0] = mat0->origin[0] * mat1->axis[0][0]
        + mat0->origin[1] * mat1->axis[1][0]
        + mat0->origin[2] * mat1->axis[2][0]
        + mat1->origin[0];
    out->origin[1] = mat0->origin[0] * mat1->axis[0][1]
        + mat0->origin[1] * mat1->axis[1][1]
        + mat0->origin[2] * mat1->axis[2][1]
        + mat1->origin[1];
    out->origin[2] = mat0->origin[0] * mat1->axis[0][2]
        + mat0->origin[1] * mat1->axis[1][2]
        + mat0->origin[2] * mat1->axis[2][2]
        + mat1->origin[2];
}

