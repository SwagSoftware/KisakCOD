#include "r_model_skin.h"
#include "r_init.h"
#include "r_dvars.h"
#include <qcommon/com_playerprofile.h>
#include "r_buffers.h"
#include <universal/profile.h>


//unsigned int const *const g_shortBoneWeightPerm__uint4 820f4950     gfx_d3d : r_model_skin.obj
//struct __vector4 const g_shortBoneWeightPerm 85b981d0     gfx_d3d : r_model_skin.obj


void R_SkinXModelCmd(_WORD *data)
{
    void *v2; // esp
    int v3; // [esp+Ch] [ebp-2188h]
    GfxPackedVertexNormal *v4; // [esp+18h] [ebp-217Ch]
    GfxPackedVertexNormal *v5; // [esp+1Ch] [ebp-2178h]
    GfxPackedVertex *v6; // [esp+20h] [ebp-2174h]
    const XSurface *v7; // [esp+24h] [ebp-2170h]
    _BYTE v8[8200]; // [esp+28h] [ebp-216Ch] BYREF
    const float *v9; // [esp+2030h] [ebp-164h]
    float *origin; // [esp+2034h] [ebp-160h]
    DObjSkelMat v11; // [esp+2038h] [ebp-15Ch] BYREF
    float v12; // [esp+2080h] [ebp-114h]
    float v13; // [esp+2084h] [ebp-110h]
    float v14; // [esp+2088h] [ebp-10Ch]
    float v15; // [esp+208Ch] [ebp-108h]
    float v16; // [esp+2090h] [ebp-104h]
    float v17; // [esp+2094h] [ebp-100h]
    float v18; // [esp+2098h] [ebp-FCh]
    float v19; // [esp+209Ch] [ebp-F8h]
    float v20; // [esp+20A0h] [ebp-F4h]
    float v21[3]; // [esp+20A4h] [ebp-F0h] BYREF
    float v22; // [esp+20B0h] [ebp-E4h]
    float v23; // [esp+20B4h] [ebp-E0h]
    float v24; // [esp+20B8h] [ebp-DCh]
    float v25; // [esp+20BCh] [ebp-D8h]
    float v26; // [esp+20C0h] [ebp-D4h]
    const float *v27; // [esp+20C4h] [ebp-D0h]
    DObjSkelMat v28; // [esp+20C8h] [ebp-CCh] BYREF
    float v29; // [esp+2114h] [ebp-80h]
    float v30; // [esp+2118h] [ebp-7Ch]
    float v31; // [esp+211Ch] [ebp-78h]
    float v32; // [esp+2120h] [ebp-74h]
    float v33; // [esp+2124h] [ebp-70h]
    float v34; // [esp+2128h] [ebp-6Ch]
    float v35; // [esp+212Ch] [ebp-68h]
    float v36; // [esp+2130h] [ebp-64h]
    float v37; // [esp+2134h] [ebp-60h]
    float v38[3]; // [esp+2138h] [ebp-5Ch] BYREF
    float v39; // [esp+2144h] [ebp-50h]
    float v40; // [esp+2148h] [ebp-4Ch]
    float v41; // [esp+214Ch] [ebp-48h]
    float v42; // [esp+2150h] [ebp-44h]
    float v43; // [esp+2154h] [ebp-40h]
    const float *v44; // [esp+2158h] [ebp-3Ch]
    int j; // [esp+215Ch] [ebp-38h]
    int v46; // [esp+2160h] [ebp-34h]
    int v47; // [esp+2164h] [ebp-30h]
    _DWORD *v48; // [esp+2168h] [ebp-2Ch]
    unsigned int i; // [esp+216Ch] [ebp-28h]
    int v50; // [esp+2170h] [ebp-24h]
    int v51; // [esp+2174h] [ebp-20h]
    _DWORD *v52; // [esp+2178h] [ebp-1Ch]
    _WORD *v53; // [esp+217Ch] [ebp-18h]
    bool fastSkin; // [esp+2181h] [ebp-13h]
    char v55; // [esp+2182h] [ebp-12h]
    bool v56; // [esp+2183h] [ebp-11h]
    BOOL v57; // [esp+2184h] [ebp-10h]
    int v58; // [esp+2188h] [ebp-Ch]
    void *v59; // [esp+218Ch] [ebp-8h]
    void *retaddr; // [esp+2194h] [ebp+0h]

    //v58 = a1;
    //v59 = retaddr;
    v2 = alloca(8576);
    if (!dx.deviceLost)
    {
        Profile_Begin(89);
        v57 = sys_SSE->current.enabled && r_sse_skinning->current.enabled;
        v56 = v57;
        v55 = 0;
        fastSkin = gfxBuf.fastSkin;
        v53 = data;
        if (!data)
            MyAssertHandler(".\\r_model_skin.cpp", 2325, 0, "%s", "skinCmd");
        if (!v53[12])
            MyAssertHandler(".\\r_model_skin.cpp", 2326, 0, "%s", "skinCmd->surfCount");
        v52 = (_DWORD*)*v53;
        v51 = *(v53 + 1);
        v50 = -1;
        for (i = 0; i < v53[12]; ++i)
        {
            v48 = v52;
            if (*v52 == -3)
            {
                ++v52;
            }
            else
            {
                if (v50 != *(v48 + 12))
                {
                    v50 = *(v48 + 12);
                    v47 = v50 + *(v48 + 13);
                    v46 = v48[2] - 32 * v50;
                    for (j = v50; j < v47; ++j)
                    {
                        if ((*&v53[2 * (j >> 5) + 4] & (0x80000000 >> (j & 0x1F))) != 0)
                        {
                            if (v55)
                            {
                                v55 = 0;
                                //_m_empty();
                            }
                            v44 = (const float*)(v46 + 32 * j);
                            v43 = *v44;
                            if ((LODWORD(v43) & 0x7F800000) == 0x7F800000
                                || (v42 = v44[1], (LODWORD(v42) & 0x7F800000) == 0x7F800000)
                                || (v41 = v44[2], (LODWORD(v41) & 0x7F800000) == 0x7F800000)
                                || (v40 = v44[3], (LODWORD(v40) & 0x7F800000) == 0x7F800000))
                            {
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                    581,
                                    0,
                                    "%s",
                                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                            }
                            v39 = v44[7];
                            if ((LODWORD(v39) & 0x7F800000) == 0x7F800000)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                    582,
                                    0,
                                    "%s",
                                    "!IS_NAN(mat->transWeight)");
                            Vec3Scale(v44, v44[7], v38);
                            v37 = v38[0] * *v44;
                            v36 = v38[0] * v44[1];
                            v35 = v38[0] * v44[2];
                            v34 = v38[0] * v44[3];
                            v33 = v38[1] * v44[1];
                            v32 = v38[1] * v44[2];
                            v31 = v38[1] * v44[3];
                            v30 = v38[2] * v44[2];
                            v29 = v38[2] * v44[3];
                            v28.axis[0][0] = 1.0 - (v33 + v30);
                            v28.axis[0][1] = v36 - v29;
                            v28.axis[0][2] = v35 + v31;
                            v28.axis[0][3] = 0.0;
                            v28.axis[1][0] = v36 + v29;
                            v28.axis[1][1] = 1.0 - (v37 + v30);
                            v28.axis[1][2] = v32 - v34;
                            v28.axis[1][3] = 0.0;
                            v28.axis[2][0] = v35 - v31;
                            v28.axis[2][1] = v32 + v34;
                            v28.axis[2][2] = 1.0 - (v37 + v33);
                            v28.axis[2][3] = 0.0;
                            v28.origin[0] = -(v44[4] * v28.axis[0][0] + v44[5] * v28.axis[1][0] + v44[6] * v28.axis[2][0]);
                            v28.origin[1] = -(v44[4] * v28.axis[0][1] + v44[5] * v28.axis[1][1] + v44[6] * v28.axis[2][1]);
                            v28.origin[2] = -(v44[4] * v28.axis[0][2] + v44[5] * v28.axis[1][2] + v44[6] * v28.axis[2][2]);
                            v28.origin[3] = 1.0;
                            v27 = (const float*)(v51 + 32 * j);
                            v26 = *v27;
                            if ((LODWORD(v26) & 0x7F800000) == 0x7F800000
                                || (v25 = v27[1], (LODWORD(v25) & 0x7F800000) == 0x7F800000)
                                || (v24 = v27[2], (LODWORD(v24) & 0x7F800000) == 0x7F800000)
                                || (v23 = v27[3], (LODWORD(v23) & 0x7F800000) == 0x7F800000))
                            {
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                    473,
                                    0,
                                    "%s",
                                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                            }
                            v22 = v27[7];
                            if ((LODWORD(v22) & 0x7F800000) == 0x7F800000)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\renderer\\../xanim/xanim_public.h",
                                    474,
                                    0,
                                    "%s",
                                    "!IS_NAN(mat->transWeight)");
                            Vec3Scale(v27, v27[7], v21);
                            v20 = v21[0] * *v27;
                            v19 = v21[0] * v27[1];
                            v18 = v21[0] * v27[2];
                            v17 = v21[0] * v27[3];
                            v16 = v21[1] * v27[1];
                            v15 = v21[1] * v27[2];
                            v14 = v21[1] * v27[3];
                            v13 = v21[2] * v27[2];
                            v12 = v21[2] * v27[3];
                            v11.axis[0][0] = 1.0 - (v16 + v13);
                            v11.axis[0][1] = v19 + v12;
                            v11.axis[0][2] = v18 - v14;
                            v11.axis[0][3] = 0.0;
                            v11.axis[1][0] = v19 - v12;
                            v11.axis[1][1] = 1.0 - (v20 + v13);
                            v11.axis[1][2] = v15 + v17;
                            v11.axis[1][3] = 0.0;
                            v11.axis[2][0] = v18 + v14;
                            v11.axis[2][1] = v15 - v17;
                            v11.axis[2][2] = 1.0 - (v20 + v16);
                            v11.axis[2][3] = 0.0;
                            origin = v11.origin;
                            v9 = v27 + 4;
                            v11.origin[0] = v27[4];
                            v11.origin[1] = v27[5];
                            v11.origin[2] = v27[6];
                            v11.origin[3] = 1.0;
                            R_MultiplySkelMat(&v28, &v11, (DObjSkelMat*)&v8[64 * j]);
                            *&v8[64 * j + 12] = 0.0;
                            *&v8[64 * j + 28] = 0.0;
                            *&v8[64 * j + 44] = 0.0;
                            *&v8[64 * j + 60] = 1.0;
                        }
                    }
                }
                if (*v48 == -2)
                {
                    v52 += 14;
                }
                else
                {
                    v52 = v48 + 6;
                    v7 = (const XSurface*)v48[1];
                    if (!v7)
                        MyAssertHandler(".\\r_model_skin.cpp", 2375, 0, "%s", "xsurf");
                    if (*v48 < 0)
                    {
                        if ((v48[5] & 0xF) != 0)
                            MyAssertHandler(
                                ".\\r_model_skin.cpp",
                                2391,
                                0,
                                "%s\n\t(reinterpret_cast< uint >( skinnedSurf->skinnedVert )) = %i",
                                "((reinterpret_cast< uint >( skinnedSurf->skinnedVert ) & 15) == 0)",
                                v48[5]);
                        v6 = (GfxPackedVertex*)v48[5];
                    }
                    else
                    {
                        if (!gfxBuf.skinnedCacheLockAddr)
                            MyAssertHandler(".\\r_model_skin.cpp", 2384, 0, "%s", "gfxBuf.skinnedCacheLockAddr");
                        if (((int)gfxBuf.skinnedCacheLockAddr & 0xF) != 0)
                            MyAssertHandler(
                                ".\\r_model_skin.cpp",
                                2385,
                                0,
                                "%s\n\t(reinterpret_cast< uint >( gfxBuf.skinnedCacheLockAddr )) = %i",
                                "((reinterpret_cast< uint >( gfxBuf.skinnedCacheLockAddr ) & 15) == 0)",
                                gfxBuf.skinnedCacheLockAddr);
                        if ((*v48 & 0xF) != 0)
                            MyAssertHandler(
                                ".\\r_model_skin.cpp",
                                2386,
                                0,
                                "%s\n\t(skinnedSurf->skinnedCachedOffset) = %i",
                                "((skinnedSurf->skinnedCachedOffset & 15) == 0)",
                                *v48);
                        v6 = (GfxPackedVertex*)&gfxBuf.skinnedCacheLockAddr[*v48];
                    }
                    // KISAKTODO: SSE version
                    //if (v56)
                    //{
                    //    if (!v55)
                    //    {
                    //        v55 = 1;
                    //        //_m_empty();
                    //    }
                    //    v5 = 0;
                    //    v4 = 0;
                    //    if (fastSkin)
                    //    {
                    //        if (*v48 >= 0)
                    //            v4 = &gfxBuf.skinnedCacheNormalsAddr[*v48 >> 5];
                    //        v3 = v48[5];
                    //        if (v3 >= 0)
                    //            v5 = &gfxBuf.oldSkinnedCacheNormalsAddr[v3 >> 5];
                    //    }
                    //    R_SkinXSurfaceSkinnedSse(v7, &v8[64 * v50], v5, v4, v6);
                    //}
                    //else
                    {
                        R_SkinXSurfaceSkinned(v7, (const DObjSkelMat*)&v8[64 * v50], v6);
                    }
                }
            }
        }
        if (v55)
            //_m_empty();
        Profile_EndInternal(0);
    }
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
    PackedUnitVec v6; // [esp+48h] [ebp-88h]
    PackedUnitVec v7; // [esp+68h] [ebp-68h] BYREF
    float v8; // [esp+6Ch] [ebp-64h]
    float v9; // [esp+70h] [ebp-60h]
    PackedUnitVec v10; // [esp+74h] [ebp-5Ch]
    PackedUnitVec v11; // [esp+7Ch] [ebp-54h]
    PackedUnitVec v12; // [esp+80h] [ebp-50h]
    PackedUnitVec v13; // [esp+84h] [ebp-4Ch]
    PackedUnitVec v14; // [esp+88h] [ebp-48h]
    float v15; // [esp+8Ch] [ebp-44h]
    float v16; // [esp+90h] [ebp-40h]
    float v17; // [esp+94h] [ebp-3Ch]
    PackedUnitVec out; // [esp+9Ch] [ebp-34h] BYREF
    float v19; // [esp+A0h] [ebp-30h]
    float v20; // [esp+A4h] [ebp-2Ch]
    PackedUnitVec in; // [esp+A8h] [ebp-28h]
    PackedUnitVec v22; // [esp+B4h] [ebp-1Ch]
    float rotated[3]; // [esp+B8h] [ebp-18h]
    float unpacked[3]; // [esp+C4h] [ebp-Ch]

    vert->xyz[0] = *offset * mat->axis[0][0] + offset[1] * mat->axis[1][0] + offset[2] * mat->axis[2][0] + mat->origin[0];
    vert->xyz[1] = *offset * mat->axis[0][1] + offset[1] * mat->axis[1][1] + offset[2] * mat->axis[2][1] + mat->origin[1];
    vert->xyz[2] = *offset * mat->axis[0][2] + offset[1] * mat->axis[1][2] + offset[2] * mat->axis[2][2] + mat->origin[2];
    vert->binormalSign = binormalSign;
    out.packed = normal.packed;
    v19 = *(float *)&normal.packed;
    v20 = *(float *)&normal.packed;
    in.packed = normal.packed;
    Vec3UnpackUnitVec(normal, (float *)&out.packed);
    *(float *)&in.packed = 0.0;
    unpacked[0] = *(float *)&out.packed;
    unpacked[1] = v19;
    unpacked[2] = v20;
    rotated[0] = *(float *)&out.packed * mat->axis[0][0] + v19 * mat->axis[1][0] + v20 * mat->axis[2][0];
    rotated[1] = *(float *)&out.packed * mat->axis[0][1] + v19 * mat->axis[1][1] + v20 * mat->axis[2][1];
    rotated[2] = *(float *)&out.packed * mat->axis[0][2] + v19 * mat->axis[1][2] + v20 * mat->axis[2][2];
    v15 = rotated[1];
    v16 = rotated[2];
    v17 = 0.0;
    v11.array[0] = (int)(rotated[0] * 127.0 + 127.5);
    v11.array[1] = (int)(rotated[1] * 127.0 + 127.5);
    v11.array[2] = (int)(rotated[2] * 127.0 + 127.5);
    v11.array[3] = 63;
    v12.packed = v11.packed;
    v14.packed = v11.packed;
    v13.packed = v11.packed;
    v22.packed = v11.packed;
    vert->normal = v11;
    v7.packed = tangent.packed;
    v8 = *(float *)&tangent.packed;
    v9 = *(float *)&tangent.packed;
    v10.packed = tangent.packed;
    Vec3UnpackUnitVec(tangent, (float *)&v7.packed);
    *(float *)&v10.packed = 0.0;
    unpacked[0] = *(float *)&v7.packed;
    unpacked[1] = v8;
    unpacked[2] = v9;
    rotated[0] = *(float *)&v7.packed * mat->axis[0][0] + v8 * mat->axis[1][0] + v9 * mat->axis[2][0];
    rotated[1] = *(float *)&v7.packed * mat->axis[0][1] + v8 * mat->axis[1][1] + v9 * mat->axis[2][1];
    rotated[2] = *(float *)&v7.packed * mat->axis[0][2] + v8 * mat->axis[1][2] + v9 * mat->axis[2][2];
    v6.array[0] = (int)(rotated[0] * 127.0 + 127.5);
    v6.array[1] = (int)(rotated[1] * 127.0 + 127.5);
    v6.array[2] = (int)(rotated[2] * 127.0 + 127.5);
    v6.array[3] = 63;
    vert->tangent = v6;
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
    *out = ((double)in.array[0] - 127.0) * decodeScale;
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

