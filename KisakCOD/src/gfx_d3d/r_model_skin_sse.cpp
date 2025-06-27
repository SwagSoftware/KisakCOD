#include "r_model_skin.h"

#include <xanim/xanim.h>
#include <universal/profile.h>

#include <xmmintrin.h>

__m128 sse_weightScale =
{ { 0.000015258789f, 0.000015258789f, 0.000015258789f, 0.000015258789f } };

__m128 sse_one =
{ { 1.0f, 1.0f, 1.0f, 1.0f } };

__m128 sse_encodeShift =
{ { 127.0f, 127.0f, 127.0f, -192.0f } };

__m128 sse_encodeScale =
{ { 127.0f, 127.0f, 127.0f, 255.0f } };


void __cdecl R_SkinXSurfaceWeightSseBlockInOut_3_Sse_SkinVertexSimple_1_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    const GfxPackedVertexNormal *srcVertNormals,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v8; // ecx
    __m64 v9; // mm0
    __m64 v10; // mm1
    __m64 *v11; // [esp-230h] [ebp-440h]
    __m128 v12; // [esp-10Ch] [ebp-31Ch]
    __m128 v13; // [esp-Ch] [ebp-21Ch]
    __m128 v14; // [esp+64h] [ebp-1ACh]
    __m128 *v15; // [esp+1B0h] [ebp-60h]
    __m64 v16; // [esp+1B4h] [ebp-5Ch]
    __m64 v17; // [esp+1BCh] [ebp-54h]
    __m128 normalTangent_4; // [esp+1C4h] [ebp-4Ch]
    const GfxPackedVertexNormal *v19; // [esp+1ECh] [ebp-24h]
    __m128 *v20; // [esp+1F0h] [ebp-20h]
    int i; // [esp+1F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+1FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(srcVertNormals);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(srcVertNormals) & 7));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v20 = (__m128 *) & srcVerts[(_DWORD)v];
        v19 = &srcVertNormals[(_DWORD)v];
        _mm_prefetch((const char *)&v20[8], 0);
        _mm_prefetch((const char *)&v19[16], 0);
        normalTangent_4 = *v20;
        v17 = (__m64)v20[1].m128_u64[0];
        v16 = *(__m64 *)v19;
        v14 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[0] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[1] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[2] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)),
                *(__m128 *)((char *)boneMatrix->origin + *vertexBlend)));
        v13 = _mm_shuffle_ps(v14, _mm_unpackhi_ps(v14, *v20), 196);
        v15 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v8) = HIWORD(v15);
        v12 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v15, _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(v15[1], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(_mm_mul_ps(v15[2], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)), v15[3]));
        LOWORD(v8) = vertexBlend[2];
        v9 = _mm_cvtsi32_si64(v8);
        v10 = _m_punpcklwd(v9, v9);
        v11 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v11,
            _mm_add_ps(
                v13,
                _mm_mul_ps(
                    _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v10, v10)), sse_weightScale),
                    _mm_sub_ps(_mm_shuffle_ps(v12, _mm_unpackhi_ps(v12, *v20), 196), v13))));
        _mm_stream_pi(v11 + 2, v17);
        _mm_stream_pi(v11 + 3, v16);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v16);
        vertexBlend += 3;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSseBlockInOut_1_Sse_SkinVertexSimple_0_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    const GfxPackedVertexNormal *srcVertNormals,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    __m64 *v8; // [esp-10h] [ebp-220h]
    __m128 v9; // [esp+64h] [ebp-1ACh]
    __m64 v10; // [esp+1B4h] [ebp-5Ch]
    __m64 v11; // [esp+1BCh] [ebp-54h]
    __m128 normalTangent_4; // [esp+1C4h] [ebp-4Ch]
    const GfxPackedVertexNormal *v13; // [esp+1ECh] [ebp-24h]
    __m128 *v14; // [esp+1F0h] [ebp-20h]
    int i; // [esp+1F4h] [ebp-1Ch]
    int v; // [esp+1FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(srcVertNormals);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(srcVertNormals) & 7));

    v = *pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v14 = (__m128 *) & srcVerts[v];
        v13 = &srcVertNormals[v];
        _mm_prefetch((const char *)&v14[8], 0);
        _mm_prefetch((const char *)&v13[16], 0);
        normalTangent_4 = *v14;
        v11 = (__m64)v14[1].m128_u64[0];
        v10 = *(__m64 *)v13;
        v9 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[0] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[1] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[2] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)),
                *(__m128 *)((char *)boneMatrix->origin + *vertexBlend)));
        v8 = (__m64 *) & dstVerts[v];
        _mm_stream_ps((float *)v8, _mm_shuffle_ps(v9, _mm_unpackhi_ps(v9, *v14), 196));
        _mm_stream_pi(v8 + 2, v11);
        _mm_stream_pi(v8 + 3, v10);
        _mm_stream_pi((__m64 *) & dstVertNormals[v], v10);
        ++vertexBlend;
        ++v;
    }
    *pVertexIndex = v;
}

void __cdecl R_SkinXSurfaceWeightSseBlockInOut_5_Sse_SkinVertexSimple_2_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    const GfxPackedVertexNormal *srcVertNormals,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v8; // ecx
    __m64 v9; // mm0
    __m64 v10; // mm1
    unsigned int v11; // ecx
    __m64 v12; // mm0
    __m64 v13; // mm1
    __m64 *v14; // [esp-460h] [ebp-670h]
    __m128 v15; // [esp-33Ch] [ebp-54Ch]
    __m128 v16; // [esp-23Ch] [ebp-44Ch]
    __m128 v17; // [esp-21Ch] [ebp-42Ch]
    __m128 v18; // [esp-1FCh] [ebp-40Ch]
    __m128 v19; // [esp-1FCh] [ebp-40Ch]
    __m128 v20; // [esp-17Ch] [ebp-38Ch]
    __m128 v21; // [esp-10Ch] [ebp-31Ch]
    __m128 v22; // [esp-Ch] [ebp-21Ch]
    __m128 v23; // [esp+64h] [ebp-1ACh]
    __m128 *v24; // [esp+1B0h] [ebp-60h]
    __m128 *v25; // [esp+1B0h] [ebp-60h]
    __m64 v26; // [esp+1B4h] [ebp-5Ch]
    __m64 v27; // [esp+1BCh] [ebp-54h]
    __m128 normalTangent_4; // [esp+1C4h] [ebp-4Ch]
    const GfxPackedVertexNormal *v29; // [esp+1ECh] [ebp-24h]
    __m128 *v30; // [esp+1F0h] [ebp-20h]
    int i; // [esp+1F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+1FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(srcVertNormals);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(srcVertNormals) & 7));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v30 = (__m128 *) & srcVerts[(_DWORD)v];
        v29 = &srcVertNormals[(_DWORD)v];
        _mm_prefetch((const char *)&v30[8], 0);
        _mm_prefetch((const char *)&v29[16], 0);
        normalTangent_4 = *v30;
        v27 = (__m64)v30[1].m128_u64[0];
        v26 = *(__m64 *)v29;
        v23 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[0] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[1] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[2] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)),
                *(__m128 *)((char *)boneMatrix->origin + *vertexBlend)));
        v22 = _mm_shuffle_ps(v23, _mm_unpackhi_ps(v23, *v30), 196);
        v24 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v8) = HIWORD(v24);
        v21 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v24, _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(v24[1], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(_mm_mul_ps(v24[2], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)), v24[3]));
        v20 = _mm_shuffle_ps(v21, _mm_unpackhi_ps(v21, *v30), 196);
        LOWORD(v8) = vertexBlend[2];
        v9 = _mm_cvtsi32_si64(v8);
        v10 = _m_punpcklwd(v9, v9);
        v18 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v10, v10)), sse_weightScale);
        v17 = _mm_sub_ps(sse_one, v18);
        v16 = _mm_mul_ps(v18, v20);
        v25 = (__m128 *)((char *)boneMatrix + vertexBlend[3]);
        HIWORD(v11) = HIWORD(v25);
        v15 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v25, _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(v25[1], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(_mm_mul_ps(v25[2], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)), v25[3]));
        LOWORD(v11) = vertexBlend[4];
        v12 = _mm_cvtsi32_si64(v11);
        v13 = _m_punpcklwd(v12, v12);
        v19 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v13, v13)), sse_weightScale);
        v14 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v14,
            _mm_add_ps(
                _mm_add_ps(v16, _mm_mul_ps(v19, _mm_shuffle_ps(v15, _mm_unpackhi_ps(v15, normalTangent_4), 196))),
                _mm_mul_ps(_mm_sub_ps(v17, v19), v22)));
        _mm_stream_pi(v14 + 2, v27);
        _mm_stream_pi(v14 + 3, v26);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v26);
        vertexBlend += 5;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}


void __cdecl R_SkinXSurfaceWeightSseBlockInOut_7_Sse_SkinVertexSimple_3_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    const GfxPackedVertexNormal *srcVertNormals,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v8; // ecx
    __m64 v9; // mm0
    __m64 v10; // mm1
    unsigned int v11; // ecx
    __m64 v12; // mm0
    __m64 v13; // mm1
    unsigned int v14; // ecx
    __m64 v15; // mm0
    __m64 v16; // mm1
    __m64 *v17; // [esp-660h] [ebp-870h]
    __m128 v18; // [esp-53Ch] [ebp-74Ch]
    __m128 v19; // [esp-33Ch] [ebp-54Ch]
    __m128 v20; // [esp-23Ch] [ebp-44Ch]
    __m128 v21; // [esp-23Ch] [ebp-44Ch]
    __m128 v22; // [esp-21Ch] [ebp-42Ch]
    __m128 v23; // [esp-21Ch] [ebp-42Ch]
    __m128 v24; // [esp-1FCh] [ebp-40Ch]
    __m128 v25; // [esp-1FCh] [ebp-40Ch]
    __m128 v26; // [esp-1FCh] [ebp-40Ch]
    __m128 v27; // [esp-17Ch] [ebp-38Ch]
    __m128 v28; // [esp-10Ch] [ebp-31Ch]
    __m128 v29; // [esp-Ch] [ebp-21Ch]
    __m128 v30; // [esp+64h] [ebp-1ACh]
    __m128 *v31; // [esp+1B0h] [ebp-60h]
    __m128 *v32; // [esp+1B0h] [ebp-60h]
    __m128 *v33; // [esp+1B0h] [ebp-60h]
    __m64 v34; // [esp+1B4h] [ebp-5Ch]
    __m64 v35; // [esp+1BCh] [ebp-54h]
    __m128 normalTangent_4; // [esp+1C4h] [ebp-4Ch]
    const GfxPackedVertexNormal *v37; // [esp+1ECh] [ebp-24h]
    __m128 *v38; // [esp+1F0h] [ebp-20h]
    int i; // [esp+1F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+1FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(srcVertNormals);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(srcVertNormals) & 7));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v38 = (__m128 *) & srcVerts[(_DWORD)v];
        v37 = &srcVertNormals[(_DWORD)v];
        _mm_prefetch((const char *)&v38[8], 0);
        _mm_prefetch((const char *)&v37[16], 0);
        normalTangent_4 = *v38;
        v35 = (__m64)v38[1].m128_u64[0];
        v34 = *(__m64 *)v37;
        v30 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[0] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[1] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(
                _mm_mul_ps(
                    *(__m128 *)((char *)boneMatrix->axis[2] + *vertexBlend),
                    _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)),
                *(__m128 *)((char *)boneMatrix->origin + *vertexBlend)));
        v29 = _mm_shuffle_ps(v30, _mm_unpackhi_ps(v30, *v38), 196);
        v31 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v8) = HIWORD(v31);
        v28 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v31, _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(v31[1], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(_mm_mul_ps(v31[2], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)), v31[3]));
        v27 = _mm_shuffle_ps(v28, _mm_unpackhi_ps(v28, *v38), 196);
        LOWORD(v8) = vertexBlend[2];
        v9 = _mm_cvtsi32_si64(v8);
        v10 = _m_punpcklwd(v9, v9);
        v24 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v10, v10)), sse_weightScale);
        v22 = _mm_mul_ps(v24, v27);
        v20 = _mm_sub_ps(sse_one, v24);
        v32 = (__m128 *)((char *)boneMatrix + vertexBlend[3]);
        HIWORD(v11) = HIWORD(v32);
        v19 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v32, _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(v32[1], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(_mm_mul_ps(v32[2], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)), v32[3]));
        LOWORD(v11) = vertexBlend[4];
        v12 = _mm_cvtsi32_si64(v11);
        v13 = _m_punpcklwd(v12, v12);
        v25 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v13, v13)), sse_weightScale);
        v21 = _mm_sub_ps(v20, v25);
        v23 = _mm_add_ps(v22, _mm_mul_ps(v25, _mm_shuffle_ps(v19, _mm_unpackhi_ps(v19, normalTangent_4), 196)));
        v33 = (__m128 *)((char *)boneMatrix + vertexBlend[5]);
        HIWORD(v14) = HIWORD(v33);
        v18 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v33, _mm_shuffle_ps(normalTangent_4, normalTangent_4, 0)),
                _mm_mul_ps(v33[1], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 85))),
            _mm_add_ps(_mm_mul_ps(v33[2], _mm_shuffle_ps(normalTangent_4, normalTangent_4, 170)), v33[3]));
        LOWORD(v14) = vertexBlend[6];
        v15 = _mm_cvtsi32_si64(v14);
        v16 = _m_punpcklwd(v15, v15);
        v26 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v16, v16)), sse_weightScale);
        v17 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v17,
            _mm_add_ps(
                _mm_add_ps(v23, _mm_mul_ps(v26, _mm_shuffle_ps(v18, _mm_unpackhi_ps(v18, normalTangent_4), 196))),
                _mm_mul_ps(_mm_sub_ps(v21, v26), v29)));
        _mm_stream_pi(v17 + 2, v35);
        _mm_stream_pi(v17 + 3, v34);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v34);
        vertexBlend += 7;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSseInOut(
    const GfxPackedVertex *inVerts,
    const XSurfaceVertexInfo *vertexInfo,
    const DObjSkelMat *boneMatrix,
    const GfxPackedVertexNormal *inVertNormals,
    GfxPackedVertexNormal *outVertNormals,
    GfxPackedVertex *outVerts)
{
    const unsigned __int16 *vertsBlend; // [esp+30h] [ebp-8h]
    int vertIndex; // [esp+34h] [ebp-4h] BYREF
    int savedregs; // [esp+38h] [ebp+0h] BYREF

    Profile_Begin(111);
    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if (vertexInfo->vertCount[0])
    {
        R_SkinXSurfaceWeightSseBlockInOut_1_Sse_SkinVertexSimple_0_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[0],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += vertexInfo->vertCount[0];
    }
    if (vertexInfo->vertCount[1])
    {
        R_SkinXSurfaceWeightSseBlockInOut_3_Sse_SkinVertexSimple_1_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[1],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if (vertexInfo->vertCount[2])
    {
        R_SkinXSurfaceWeightSseBlockInOut_5_Sse_SkinVertexSimple_2_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[2],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if (vertexInfo->vertCount[3])
        R_SkinXSurfaceWeightSseBlockInOut_7_Sse_SkinVertexSimple_3_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[3],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
    Profile_EndInternal(0);
}

void __cdecl R_SkinXSurfaceRigidSseInOut(
    const XSurface *surf,
    int totalVertCount,
    const DObjSkelMat *boneMatrix,
    __m64 *srcVertNormals,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts)
{
    __m128 v6; // [esp+64h] [ebp-1BCh]
    __m64 m64_u64; // [esp+164h] [ebp-BCh]
    __m64 v8; // [esp+16Ch] [ebp-B4h]
    int v9; // [esp+1A0h] [ebp-80h]
    __m128 v10; // [esp+1A4h] [ebp-7Ch]
    __m128 matrix_4; // [esp+1B4h] [ebp-6Ch]
    __m128 matrix_20; // [esp+1C4h] [ebp-5Ch]
    __m128 matrix_36; // [esp+1D4h] [ebp-4Ch]
    __m128 *v14; // [esp+1F0h] [ebp-30h]
    int v15; // [esp+1F4h] [ebp-2Ch]
    XRigidVertList *v16; // [esp+1F8h] [ebp-28h]
    unsigned int matrixAddress; // [esp+1FCh] [ebp-24h]
    __m64 *vertCount; // [esp+200h] [ebp-20h]
    __m64 *vertList; // [esp+204h] [ebp-1Ch]
    __m128 *vertexNormal; // [esp+20Ch] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    Profile_Begin(111);
    vertexNormal = (__m128 *)surf->verts0;
    vertList = (__m64 *)dstVerts;
    vertCount = (__m64 *)dstVertNormals;
    for (matrixAddress = 0; matrixAddress < surf->vertListCount; ++matrixAddress)
    {
        v16 = &surf->vertList[matrixAddress];
        v15 = v16->vertCount;
        v14 = (__m128 *)((char *)boneMatrix + v16->boneOffset);
        v10 = *v14;
        matrix_4 = v14[1];
        matrix_20 = v14[2];
        matrix_36 = v14[3];
        v9 = 0;
        while (v9 < v15)
        {
            _mm_prefetch((const char *)&vertexNormal[8], 0);
            _mm_prefetch((const char *)&srcVertNormals[16], 0);
            v8 = (__m64)vertexNormal[1].m128_u64[0];
            m64_u64 = (__m64)srcVertNormals->m64_u64;
            v6 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(v10, _mm_shuffle_ps(*vertexNormal, *vertexNormal, 0)),
                    _mm_mul_ps(matrix_4, _mm_shuffle_ps(*vertexNormal, *vertexNormal, 85))),
                _mm_add_ps(_mm_mul_ps(matrix_20, _mm_shuffle_ps(*vertexNormal, *vertexNormal, 170)), matrix_36));
            _mm_stream_ps((float *)vertList, _mm_shuffle_ps(v6, _mm_unpackhi_ps(v6, *vertexNormal), 196));
            _mm_stream_pi(vertList + 2, v8);
            _mm_stream_pi(vertList + 3, m64_u64);
            _mm_stream_pi(vertCount, m64_u64);
            ++v9;
            vertexNormal += 2;
            ++srcVertNormals;
            vertList += 4;
            ++vertCount;
        }
    }
    if (((char *)vertList - (char *)dstVerts) >> 5 != totalVertCount)
        MyAssertHandler(".\\r_model_skin.cpp", 2137, 0, "%s", "vertex - dstVerts == totalVertCount");
    if (((char *)vertCount - (char *)dstVertNormals) >> 3 != totalVertCount)
        MyAssertHandler(".\\r_model_skin.cpp", 2138, 0, "%s", "vertexNormal - dstVertNormals == totalVertCount");
    Profile_EndInternal(0);
}



void __cdecl R_SkinXSurfaceWeightSseBlockOut_1_Sse_SkinVertex_0_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    __m64 *v7; // [esp-10h] [ebp-720h]
    __m64 v8; // [esp-Ch] [ebp-71Ch]
    __m128 v9; // [esp+D4h] [ebp-63Ch]
    __m128 v10; // [esp+1C4h] [ebp-54Ch]
    __m128 v11; // [esp+204h] [ebp-50Ch]
    __m128 v12; // [esp+234h] [ebp-4DCh]
    __m128 v13; // [esp+344h] [ebp-3CCh]
    __m128 v14; // [esp+434h] [ebp-2DCh]
    __m128 v15; // [esp+474h] [ebp-29Ch]
    __m128 v16; // [esp+4A4h] [ebp-26Ch]
    __m128 v17; // [esp+4F4h] [ebp-21Ch]
    __m128 v18; // [esp+564h] [ebp-1ACh]
    __m128 v19; // [esp+664h] [ebp-ACh]
    __m128 v20; // [esp+674h] [ebp-9Ch]
    __m128 v21; // [esp+684h] [ebp-8Ch]
    __m128 v22; // [esp+694h] [ebp-7Ch]
    __m128 *v23; // [esp+6A8h] [ebp-68h]
    unsigned int v24; // [esp+6B0h] [ebp-60h]
    __m64 v25; // [esp+6BCh] [ebp-54h]
    __m128 *v26; // [esp+6F0h] [ebp-20h]
    int i; // [esp+6F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v26 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v26[8], 0);
        v25 = (__m64)v26[1].m128_u64[0];
        v24 = v26[1].m128_u32[3];
        v23 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v19 = *v23;
        v20 = v23[1];
        v21 = v23[2];
        v22 = v23[3];
        v18 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(*v23, _mm_shuffle_ps(*v26, *v26, 0)), _mm_mul_ps(v20, _mm_shuffle_ps(*v26, *v26, 85))),
            _mm_add_ps(_mm_mul_ps(v21, _mm_shuffle_ps(*v26, *v26, 170)), v22));
        v17 = _mm_shuffle_ps(v18, _mm_unpackhi_ps(v18, *v26), 196);
        v15 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v26[1].m128_u32[2]), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v14 = _mm_mul_ps(v15, _mm_shuffle_ps(v15, v15, 255));
        v13 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v19, _mm_shuffle_ps(v14, v14, 0)), _mm_mul_ps(v20, _mm_shuffle_ps(v14, v14, 85))),
            _mm_mul_ps(v21, _mm_shuffle_ps(v14, v14, 170)));
        v16 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v13, _mm_unpackhi_ps(v13, v22), 196), sse_encodeScale), sse_encodeShift);
        v11 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v24), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v10 = _mm_mul_ps(v11, _mm_shuffle_ps(v11, v11, 255));
        v9 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v19, _mm_shuffle_ps(v10, v10, 0)), _mm_mul_ps(v20, _mm_shuffle_ps(v10, v10, 85))),
            _mm_mul_ps(v21, _mm_shuffle_ps(v10, v10, 170)));
        v12 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v9, _mm_unpackhi_ps(v9, v22), 196), sse_encodeScale), sse_encodeShift);
        v8 = _m_packuswb(
            _m_packuswb(_mm_cvt_ps2pi(v16), _mm_cvt_ps2pi(_mm_movehl_ps(v16, v16))),
            _m_packuswb(_mm_cvt_ps2pi(v12), _mm_cvt_ps2pi(_mm_movehl_ps(v12, v12))));
        v7 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps((float *)v7, v17);
        _mm_stream_pi(v7 + 2, v25);
        _mm_stream_pi(v7 + 3, v8);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v8);
        ++vertexBlend;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}


void __cdecl R_SkinXSurfaceWeightSseBlockOut_3_Sse_SkinVertex_1_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v7; // edx
    __m64 v8; // mm0
    __m64 v9; // mm1
    __m64 *v10; // [esp-230h] [ebp-940h]
    __m128 v11; // [esp-10Ch] [ebp-81Ch]
    __m64 v12; // [esp-Ch] [ebp-71Ch]
    __m128 v13; // [esp+D4h] [ebp-63Ch]
    __m128 v14; // [esp+1C4h] [ebp-54Ch]
    __m128 v15; // [esp+204h] [ebp-50Ch]
    __m128 v16; // [esp+234h] [ebp-4DCh]
    __m128 v17; // [esp+344h] [ebp-3CCh]
    __m128 v18; // [esp+434h] [ebp-2DCh]
    __m128 v19; // [esp+474h] [ebp-29Ch]
    __m128 v20; // [esp+4A4h] [ebp-26Ch]
    __m128 v21; // [esp+4F4h] [ebp-21Ch]
    __m128 v22; // [esp+564h] [ebp-1ACh]
    __m128 v23; // [esp+664h] [ebp-ACh]
    __m128 v24; // [esp+674h] [ebp-9Ch]
    __m128 v25; // [esp+684h] [ebp-8Ch]
    __m128 v26; // [esp+694h] [ebp-7Ch]
    __m128 *v27; // [esp+6A8h] [ebp-68h]
    __m128 *v28; // [esp+6A8h] [ebp-68h]
    unsigned int v29; // [esp+6B0h] [ebp-60h]
    __m64 v30; // [esp+6BCh] [ebp-54h]
    __m128 inNormal; // [esp+6C4h] [ebp-4Ch]
    __m128 *v32; // [esp+6F0h] [ebp-20h]
    int i; // [esp+6F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v32 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v32[8], 0);
        inNormal = *v32;
        v30 = (__m64)v32[1].m128_u64[0];
        v29 = v32[1].m128_u32[3];
        v27 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v23 = *v27;
        v24 = v27[1];
        v25 = v27[2];
        v26 = v27[3];
        v22 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v27, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v24, _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v25, _mm_shuffle_ps(inNormal, inNormal, 170)), v26));
        v21 = _mm_shuffle_ps(v22, _mm_unpackhi_ps(v22, *v32), 196);
        v19 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v32[1].m128_u32[2]), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v18 = _mm_mul_ps(v19, _mm_shuffle_ps(v19, v19, 255));
        v17 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v23, _mm_shuffle_ps(v18, v18, 0)), _mm_mul_ps(v24, _mm_shuffle_ps(v18, v18, 85))),
            _mm_mul_ps(v25, _mm_shuffle_ps(v18, v18, 170)));
        v20 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v17, _mm_unpackhi_ps(v17, v26), 196), sse_encodeScale), sse_encodeShift);
        v15 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v29), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v14 = _mm_mul_ps(v15, _mm_shuffle_ps(v15, v15, 255));
        v13 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v23, _mm_shuffle_ps(v14, v14, 0)), _mm_mul_ps(v24, _mm_shuffle_ps(v14, v14, 85))),
            _mm_mul_ps(v25, _mm_shuffle_ps(v14, v14, 170)));
        v16 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v13, _mm_unpackhi_ps(v13, v26), 196), sse_encodeScale), sse_encodeShift);
        v12 = _m_packuswb(
            _m_packuswb(_mm_cvt_ps2pi(v20), _mm_cvt_ps2pi(_mm_movehl_ps(v20, v20))),
            _m_packuswb(_mm_cvt_ps2pi(v16), _mm_cvt_ps2pi(_mm_movehl_ps(v16, v16))));
        v28 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v7) = HIWORD(v28);
        v11 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v28, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v28[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v28[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v28[3]));
        LOWORD(v7) = vertexBlend[2];
        v8 = _mm_cvtsi32_si64(v7);
        v9 = _m_punpcklwd(v8, v8);
        v10 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v10,
            _mm_add_ps(
                v21,
                _mm_mul_ps(
                    _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v9, v9)), sse_weightScale),
                    _mm_sub_ps(_mm_shuffle_ps(v11, _mm_unpackhi_ps(v11, inNormal), 196), v21))));
        _mm_stream_pi(v10 + 2, v30);
        _mm_stream_pi(v10 + 3, v12);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v12);
        vertexBlend += 3;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}


void __cdecl R_SkinXSurfaceWeightSseBlockOut_5_Sse_SkinVertex_2_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v7; // edx
    __m64 v8; // mm0
    __m64 v9; // mm1
    unsigned int v10; // edx
    __m64 v11; // mm0
    __m64 v12; // mm1
    __m64 *v13; // [esp-460h] [ebp-B50h]
    __m128 v14; // [esp-33Ch] [ebp-A2Ch]
    __m128 v15; // [esp-23Ch] [ebp-92Ch]
    __m128 v16; // [esp-21Ch] [ebp-90Ch]
    __m128 v17; // [esp-1FCh] [ebp-8ECh]
    __m128 v18; // [esp-1FCh] [ebp-8ECh]
    __m128 v19; // [esp-10Ch] [ebp-7FCh]
    __m64 v20; // [esp-Ch] [ebp-6FCh]
    __m128 v21; // [esp+D4h] [ebp-61Ch]
    __m128 v22; // [esp+1C4h] [ebp-52Ch]
    __m128 v23; // [esp+204h] [ebp-4ECh]
    __m128 v24; // [esp+234h] [ebp-4BCh]
    __m128 v25; // [esp+344h] [ebp-3ACh]
    __m128 v26; // [esp+434h] [ebp-2BCh]
    __m128 v27; // [esp+474h] [ebp-27Ch]
    __m128 v28; // [esp+4A4h] [ebp-24Ch]
    __m128 v29; // [esp+4D4h] [ebp-21Ch]
    __m128 v30; // [esp+544h] [ebp-1ACh]
    __m128 v31; // [esp+644h] [ebp-ACh]
    __m128 v32; // [esp+654h] [ebp-9Ch]
    __m128 v33; // [esp+664h] [ebp-8Ch]
    __m128 v34; // [esp+674h] [ebp-7Ch]
    __m128 *v35; // [esp+688h] [ebp-68h]
    __m128 *v36; // [esp+688h] [ebp-68h]
    __m128 *v37; // [esp+688h] [ebp-68h]
    unsigned int v38; // [esp+690h] [ebp-60h]
    __m64 v39; // [esp+69Ch] [ebp-54h]
    __m128 inNormal; // [esp+6A4h] [ebp-4Ch]
    __m128 *v41; // [esp+6D0h] [ebp-20h]
    int i; // [esp+6D4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6DCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v41 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v41[8], 0);
        inNormal = *v41;
        v39 = (__m64)v41[1].m128_u64[0];
        v38 = v41[1].m128_u32[3];
        v35 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v31 = *v35;
        v32 = v35[1];
        v33 = v35[2];
        v34 = v35[3];
        v30 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v35, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v32, _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v33, _mm_shuffle_ps(inNormal, inNormal, 170)), v34));
        v29 = _mm_shuffle_ps(v30, _mm_unpackhi_ps(v30, *v41), 196);
        v27 = _mm_div_ps(_mm_sub_ps(_mm_cvtpu8_ps(_mm_cvtsi32_si64(v41[1].m128_u32[2])), sse_encodeShift), sse_encodeScale);
        v26 = _mm_mul_ps(v27, _mm_shuffle_ps(v27, v27, 255));
        v25 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v31, _mm_shuffle_ps(v26, v26, 0)), _mm_mul_ps(v32, _mm_shuffle_ps(v26, v26, 85))),
            _mm_mul_ps(v33, _mm_shuffle_ps(v26, v26, 170)));
        v28 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v25, _mm_unpackhi_ps(v25, v34), 196), sse_encodeScale), sse_encodeShift);
        v23 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v38), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v22 = _mm_mul_ps(v23, _mm_shuffle_ps(v23, v23, 255));
        v21 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v31, _mm_shuffle_ps(v22, v22, 0)), _mm_mul_ps(v32, _mm_shuffle_ps(v22, v22, 85))),
            _mm_mul_ps(v33, _mm_shuffle_ps(v22, v22, 170)));
        v24 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v21, _mm_unpackhi_ps(v21, v34), 196), sse_encodeScale), sse_encodeShift);
        v20 = _m_packuswb(
            _m_packuswb(_mm_cvt_ps2pi(v28), _mm_cvt_ps2pi(_mm_movehl_ps(v28, v28))),
            _m_packuswb(_mm_cvt_ps2pi(v24), _mm_cvt_ps2pi(_mm_movehl_ps(v24, v24))));
        v36 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v7) = HIWORD(v36);
        v19 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v36, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v36[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v36[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v36[3]));
        LOWORD(v7) = vertexBlend[2];
        v8 = _mm_cvtsi32_si64(v7);
        v9 = _m_punpcklwd(v8, v8);
        v17 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v9, v9)), sse_weightScale);
        v16 = _mm_sub_ps(sse_one, v17);
        v15 = _mm_mul_ps(v17, _mm_shuffle_ps(v19, _mm_unpackhi_ps(v19, inNormal), 196));
        v37 = (__m128 *)((char *)boneMatrix + vertexBlend[3]);
        HIWORD(v10) = HIWORD(v37);
        v14 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v37, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v37[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v37[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v37[3]));
        LOWORD(v10) = vertexBlend[4];
        v11 = _mm_cvtsi32_si64(v10);
        v12 = _m_punpcklwd(v11, v11);
        v18 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v12, v12)), sse_weightScale);
        v13 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v13,
            _mm_add_ps(
                _mm_add_ps(v15, _mm_mul_ps(v18, _mm_shuffle_ps(v14, _mm_unpackhi_ps(v14, inNormal), 196))),
                _mm_mul_ps(_mm_sub_ps(v16, v18), v29)));
        _mm_stream_pi(v13 + 2, v39);
        _mm_stream_pi(v13 + 3, v20);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v20);
        vertexBlend += 5;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSseBlockOut_7_Sse_SkinVertex_3_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v7; // edx
    __m64 v8; // mm0
    __m64 v9; // mm1
    unsigned int v10; // edx
    __m64 v11; // mm0
    __m64 v12; // mm1
    unsigned int v13; // edx
    __m64 v14; // mm0
    __m64 v15; // mm1
    __m64 *v16; // [esp-660h] [ebp-D50h]
    __m128 v17; // [esp-53Ch] [ebp-C2Ch]
    __m128 v18; // [esp-33Ch] [ebp-A2Ch]
    __m128 v19; // [esp-23Ch] [ebp-92Ch]
    __m128 v20; // [esp-23Ch] [ebp-92Ch]
    __m128 v21; // [esp-21Ch] [ebp-90Ch]
    __m128 v22; // [esp-21Ch] [ebp-90Ch]
    __m128 v23; // [esp-1FCh] [ebp-8ECh]
    __m128 v24; // [esp-1FCh] [ebp-8ECh]
    __m128 v25; // [esp-1FCh] [ebp-8ECh]
    __m128 v26; // [esp-10Ch] [ebp-7FCh]
    __m64 v27; // [esp-Ch] [ebp-6FCh]
    __m128 v28; // [esp+D4h] [ebp-61Ch]
    __m128 v29; // [esp+1C4h] [ebp-52Ch]
    __m128 v30; // [esp+204h] [ebp-4ECh]
    __m128 v31; // [esp+234h] [ebp-4BCh]
    __m128 v32; // [esp+344h] [ebp-3ACh]
    __m128 v33; // [esp+434h] [ebp-2BCh]
    __m128 v34; // [esp+474h] [ebp-27Ch]
    __m128 v35; // [esp+4A4h] [ebp-24Ch]
    __m128 v36; // [esp+4D4h] [ebp-21Ch]
    __m128 v37; // [esp+544h] [ebp-1ACh]
    __m128 v38; // [esp+644h] [ebp-ACh]
    __m128 v39; // [esp+654h] [ebp-9Ch]
    __m128 v40; // [esp+664h] [ebp-8Ch]
    __m128 v41; // [esp+674h] [ebp-7Ch]
    __m128 *v42; // [esp+688h] [ebp-68h]
    __m128 *v43; // [esp+688h] [ebp-68h]
    __m128 *v44; // [esp+688h] [ebp-68h]
    __m128 *v45; // [esp+688h] [ebp-68h]
    unsigned int v46; // [esp+690h] [ebp-60h]
    __m64 v47; // [esp+69Ch] [ebp-54h]
    __m128 inNormal; // [esp+6A4h] [ebp-4Ch]
    __m128 *v49; // [esp+6D0h] [ebp-20h]
    int i; // [esp+6D4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6DCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(srcVerts) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v49 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v49[8], 0);
        inNormal = *v49;
        v47 = (__m64)v49[1].m128_u64[0];
        v46 = v49[1].m128_u32[3];
        v42 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v38 = *v42;
        v39 = v42[1];
        v40 = v42[2];
        v41 = v42[3];
        v37 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v42, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v39, _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v40, _mm_shuffle_ps(inNormal, inNormal, 170)), v41));
        v36 = _mm_shuffle_ps(v37, _mm_unpackhi_ps(v37, *v49), 196);
        v34 = _mm_div_ps(_mm_sub_ps(_mm_cvtpu8_ps(_mm_cvtsi32_si64(v49[1].m128_u32[2])), sse_encodeShift), sse_encodeScale);
        v33 = _mm_mul_ps(v34, _mm_shuffle_ps(v34, v34, 255));
        v32 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v38, _mm_shuffle_ps(v33, v33, 0)), _mm_mul_ps(v39, _mm_shuffle_ps(v33, v33, 85))),
            _mm_mul_ps(v40, _mm_shuffle_ps(v33, v33, 170)));
        v35 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v32, _mm_unpackhi_ps(v32, v41), 196), sse_encodeScale), sse_encodeShift);
        v30 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v46), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v29 = _mm_mul_ps(v30, _mm_shuffle_ps(v30, v30, 255));
        v28 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v38, _mm_shuffle_ps(v29, v29, 0)), _mm_mul_ps(v39, _mm_shuffle_ps(v29, v29, 85))),
            _mm_mul_ps(v40, _mm_shuffle_ps(v29, v29, 170)));
        v31 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v28, _mm_unpackhi_ps(v28, v41), 196), sse_encodeScale), sse_encodeShift);
        v27 = _m_packuswb(
            _m_packuswb(_mm_cvt_ps2pi(v35), _mm_cvt_ps2pi(_mm_movehl_ps(v35, v35))),
            _m_packuswb(_mm_cvt_ps2pi(v31), _mm_cvt_ps2pi(_mm_movehl_ps(v31, v31))));
        v43 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v7) = HIWORD(v43);
        v26 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v43, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v43[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v43[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v43[3]));
        LOWORD(v7) = vertexBlend[2];
        v8 = _mm_cvtsi32_si64(v7);
        v9 = _m_punpcklwd(v8, v8);
        v23 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v9, v9)), sse_weightScale);
        v21 = _mm_mul_ps(v23, _mm_shuffle_ps(v26, _mm_unpackhi_ps(v26, inNormal), 196));
        v19 = _mm_sub_ps(sse_one, v23);
        v44 = (__m128 *)((char *)boneMatrix + vertexBlend[3]);
        HIWORD(v10) = HIWORD(v44);
        v18 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v44, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v44[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v44[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v44[3]));
        LOWORD(v10) = vertexBlend[4];
        v11 = _mm_cvtsi32_si64(v10);
        v12 = _m_punpcklwd(v11, v11);
        v24 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v12, v12)), sse_weightScale);
        v20 = _mm_sub_ps(v19, v24);
        v22 = _mm_add_ps(v21, _mm_mul_ps(v24, _mm_shuffle_ps(v18, _mm_unpackhi_ps(v18, inNormal), 196)));
        v45 = (__m128 *)((char *)boneMatrix + vertexBlend[5]);
        HIWORD(v13) = HIWORD(v45);
        v17 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v45, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v45[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v45[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v45[3]));
        LOWORD(v13) = vertexBlend[6];
        v14 = _mm_cvtsi32_si64(v13);
        v15 = _m_punpcklwd(v14, v14);
        v25 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v15, v15)), sse_weightScale);
        v16 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v16,
            _mm_add_ps(
                _mm_add_ps(v22, _mm_mul_ps(v25, _mm_shuffle_ps(v17, _mm_unpackhi_ps(v17, inNormal), 196))),
                _mm_mul_ps(_mm_sub_ps(v20, v25), v36)));
        _mm_stream_pi(v16 + 2, v47);
        _mm_stream_pi(v16 + 3, v27);
        _mm_stream_pi((__m64 *) & dstVertNormals[(_DWORD)v], v27);
        vertexBlend += 7;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSseOut(
    const GfxPackedVertex *inVerts,
    const XSurfaceVertexInfo *vertexInfo,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *outVertNormals,
    GfxPackedVertex *outVerts)
{
    const unsigned __int16 *vertsBlend; // [esp+30h] [ebp-8h]
    int vertIndex; // [esp+34h] [ebp-4h] BYREF

    Profile_Begin(111);
    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if (vertexInfo->vertCount[0])
    {
        R_SkinXSurfaceWeightSseBlockOut_1_Sse_SkinVertex_0_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[0],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += vertexInfo->vertCount[0];
    }
    if (vertexInfo->vertCount[1])
    {
        R_SkinXSurfaceWeightSseBlockOut_3_Sse_SkinVertex_1_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[1],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if (vertexInfo->vertCount[2])
    {
        R_SkinXSurfaceWeightSseBlockOut_5_Sse_SkinVertex_2_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[2],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if (vertexInfo->vertCount[3])
        R_SkinXSurfaceWeightSseBlockOut_7_Sse_SkinVertex_3_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[3],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
    Profile_EndInternal(0);
}

void __cdecl R_SkinXSurfaceRigidSseOut(
    const XSurface *surf,
    int totalVertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *dstVertNormals,
    GfxPackedVertex *dstVerts)
{
    __m64 v5; // [esp-Ch] [ebp-71Ch]
    __m128 v6; // [esp+D4h] [ebp-63Ch]
    __m128 v7; // [esp+1C4h] [ebp-54Ch]
    __m128 v8; // [esp+214h] [ebp-4FCh]
    __m128 v9; // [esp+234h] [ebp-4DCh]
    __m128 v10; // [esp+344h] [ebp-3CCh]
    __m128 v11; // [esp+434h] [ebp-2DCh]
    __m128 v12; // [esp+484h] [ebp-28Ch]
    __m128 v13; // [esp+4A4h] [ebp-26Ch]
    __m128 v14; // [esp+4F4h] [ebp-21Ch]
    __m128 v15; // [esp+564h] [ebp-1ACh]
    unsigned int v16; // [esp+664h] [ebp-ACh]
    __m64 v17; // [esp+66Ch] [ebp-A4h]
    int v18; // [esp+6A0h] [ebp-70h]
    __m128 v19; // [esp+6A4h] [ebp-6Ch]
    __m128 matrix_4; // [esp+6B4h] [ebp-5Ch]
    __m128 matrix_20; // [esp+6C4h] [ebp-4Ch]
    __m128 matrix_36; // [esp+6D4h] [ebp-3Ch]
    __m128 *matrix_52; // [esp+6E4h] [ebp-2Ch]
    int matrix_56; // [esp+6E8h] [ebp-28h]
    XRigidVertList *matrix_60; // [esp+6ECh] [ebp-24h]
    unsigned int matrixAddress; // [esp+6F0h] [ebp-20h]
    __m64 *vertCount; // [esp+6F4h] [ebp-1Ch]
    __m64 *vertList; // [esp+6F8h] [ebp-18h]
    __m128 *i; // [esp+6FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(dstVertNormals);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(dstVertNormals) & 7));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    Profile_Begin(111);
    i = (__m128 *)surf->verts0;
    vertList = (__m64 *)dstVerts;
    vertCount = (__m64 *)dstVertNormals;
    for (matrixAddress = 0; matrixAddress < surf->vertListCount; ++matrixAddress)
    {
        matrix_60 = &surf->vertList[matrixAddress];
        matrix_56 = matrix_60->vertCount;
        matrix_52 = (__m128 *)((char *)boneMatrix + matrix_60->boneOffset);
        v19 = *matrix_52;
        matrix_4 = matrix_52[1];
        matrix_20 = matrix_52[2];
        matrix_36 = matrix_52[3];
        v18 = 0;
        while (v18 < matrix_56)
        {
            _mm_prefetch((const char *)&i[8], 0);
            v17 = (__m64)i[1].m128_u64[0];
            v16 = i[1].m128_u32[3];
            v15 = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(v19, _mm_shuffle_ps(*i, *i, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(*i, *i, 85))),
                _mm_add_ps(_mm_mul_ps(matrix_20, _mm_shuffle_ps(*i, *i, 170)), matrix_36));
            v14 = _mm_shuffle_ps(v15, _mm_unpackhi_ps(v15, *i), 196);
            v12 = _mm_div_ps(
                _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(i[1].m128_u32[2]), (__m64)0)), sse_encodeShift),
                sse_encodeScale);
            v11 = _mm_mul_ps(v12, _mm_shuffle_ps(v12, v12, 255));
            v10 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(v19, _mm_shuffle_ps(v11, v11, 0)),
                    _mm_mul_ps(matrix_4, _mm_shuffle_ps(v11, v11, 85))),
                _mm_mul_ps(matrix_20, _mm_shuffle_ps(v11, v11, 170)));
            v13 = _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v10, _mm_unpackhi_ps(v10, matrix_36), 196), sse_encodeScale),
                sse_encodeShift);
            v8 = _mm_div_ps(
                _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v16), (__m64)0)), sse_encodeShift),
                sse_encodeScale);
            v7 = _mm_mul_ps(v8, _mm_shuffle_ps(v8, v8, 255));
            v6 = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(v19, _mm_shuffle_ps(v7, v7, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(v7, v7, 85))),
                _mm_mul_ps(matrix_20, _mm_shuffle_ps(v7, v7, 170)));
            v9 = _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v6, _mm_unpackhi_ps(v6, matrix_36), 196), sse_encodeScale),
                sse_encodeShift);
            v5 = _m_packuswb(
                _m_packuswb(_mm_cvt_ps2pi(v13), _mm_cvt_ps2pi(_mm_movehl_ps(v13, v13))),
                _m_packuswb(_mm_cvt_ps2pi(v9), _mm_cvt_ps2pi(_mm_movehl_ps(v9, v9))));
            _mm_stream_ps((float *)vertList, v14);
            _mm_stream_pi(vertList + 2, v17);
            _mm_stream_pi(vertList + 3, v5);
            _mm_stream_pi(vertCount, v5);
            ++v18;
            i += 2;
            vertList += 4;
            ++vertCount;
        }
    }
    if (((char *)vertList - (char *)dstVerts) >> 5 != totalVertCount)
        MyAssertHandler(".\\r_model_skin.cpp", 2077, 0, "%s", "vertex - dstVerts == totalVertCount");
    if (((char *)vertCount - (char *)dstVertNormals) >> 3 != totalVertCount)
        MyAssertHandler(".\\r_model_skin.cpp", 2078, 0, "%s", "vertexNormal - dstVertNormals == totalVertCount");
    Profile_EndInternal(0);
}

void __cdecl R_SkinXSurfaceWeightSseBlock_1_Sse_SkinVertex_0_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    __m64 *v6; // [esp-10h] [ebp-720h]
    __m128 v7; // [esp+D4h] [ebp-63Ch]
    __m128 v8; // [esp+1C4h] [ebp-54Ch]
    __m128 v9; // [esp+204h] [ebp-50Ch]
    __m128 v10; // [esp+234h] [ebp-4DCh]
    __m128 v11; // [esp+344h] [ebp-3CCh]
    __m128 v12; // [esp+434h] [ebp-2DCh]
    __m128 v13; // [esp+474h] [ebp-29Ch]
    __m128 v14; // [esp+4A4h] [ebp-26Ch]
    __m128 v15; // [esp+4F4h] [ebp-21Ch]
    __m128 v16; // [esp+564h] [ebp-1ACh]
    __m128 v17; // [esp+664h] [ebp-ACh]
    __m128 v18; // [esp+674h] [ebp-9Ch]
    __m128 v19; // [esp+684h] [ebp-8Ch]
    __m128 v20; // [esp+694h] [ebp-7Ch]
    __m128 *v21; // [esp+6A8h] [ebp-68h]
    unsigned int v22; // [esp+6B0h] [ebp-60h]
    __m64 v23; // [esp+6BCh] [ebp-54h]
    __m128 *v24; // [esp+6F0h] [ebp-20h]
    int i; // [esp+6F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v24 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v24[8], 0);
        v23 = (__m64)v24[1].m128_u64[0];
        v22 = v24[1].m128_u32[3];
        v21 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v17 = *v21;
        v18 = v21[1];
        v19 = v21[2];
        v20 = v21[3];
        v16 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(*v21, _mm_shuffle_ps(*v24, *v24, 0)), _mm_mul_ps(v18, _mm_shuffle_ps(*v24, *v24, 85))),
            _mm_add_ps(_mm_mul_ps(v19, _mm_shuffle_ps(*v24, *v24, 170)), v20));
        v15 = _mm_shuffle_ps(v16, _mm_unpackhi_ps(v16, *v24), 196);
        v13 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v24[1].m128_u32[2]), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v12 = _mm_mul_ps(v13, _mm_shuffle_ps(v13, v13, 255));
        v11 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v17, _mm_shuffle_ps(v12, v12, 0)), _mm_mul_ps(v18, _mm_shuffle_ps(v12, v12, 85))),
            _mm_mul_ps(v19, _mm_shuffle_ps(v12, v12, 170)));
        v14 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v11, _mm_unpackhi_ps(v11, v20), 196), sse_encodeScale), sse_encodeShift);
        v9 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v22), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v8 = _mm_mul_ps(v9, _mm_shuffle_ps(v9, v9, 255));
        v7 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v17, _mm_shuffle_ps(v8, v8, 0)), _mm_mul_ps(v18, _mm_shuffle_ps(v8, v8, 85))),
            _mm_mul_ps(v19, _mm_shuffle_ps(v8, v8, 170)));
        v10 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v7, _mm_unpackhi_ps(v7, v20), 196), sse_encodeScale), sse_encodeShift);
        v6 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps((float *)v6, v15);
        _mm_stream_pi(v6 + 2, v23);
        _mm_stream_pi(
            v6 + 3,
            _m_packuswb(
                _m_packuswb(_mm_cvt_ps2pi(v14), _mm_cvt_ps2pi(_mm_movehl_ps(v14, v14))),
                _m_packuswb(_mm_cvt_ps2pi(v10), _mm_cvt_ps2pi(_mm_movehl_ps(v10, v10)))));
        ++vertexBlend;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}


void __cdecl R_SkinXSurfaceWeightSseBlock_3_Sse_SkinVertex_1_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v6; // ecx
    __m64 v7; // mm0
    __m64 v8; // mm1
    __m64 *v9; // [esp-230h] [ebp-940h]
    __m128 v10; // [esp-10Ch] [ebp-81Ch]
    __m128 v11; // [esp+D4h] [ebp-63Ch]
    __m128 v12; // [esp+1C4h] [ebp-54Ch]
    __m128 v13; // [esp+204h] [ebp-50Ch]
    __m128 v14; // [esp+234h] [ebp-4DCh]
    __m128 v15; // [esp+344h] [ebp-3CCh]
    __m128 v16; // [esp+434h] [ebp-2DCh]
    __m128 v17; // [esp+474h] [ebp-29Ch]
    __m128 v18; // [esp+4A4h] [ebp-26Ch]
    __m128 v19; // [esp+4F4h] [ebp-21Ch]
    __m128 v20; // [esp+564h] [ebp-1ACh]
    __m128 v21; // [esp+664h] [ebp-ACh]
    __m128 v22; // [esp+674h] [ebp-9Ch]
    __m128 v23; // [esp+684h] [ebp-8Ch]
    __m128 v24; // [esp+694h] [ebp-7Ch]
    __m128 *v25; // [esp+6A8h] [ebp-68h]
    __m128 *v26; // [esp+6A8h] [ebp-68h]
    unsigned int v27; // [esp+6B0h] [ebp-60h]
    __m64 v28; // [esp+6BCh] [ebp-54h]
    __m128 inNormal; // [esp+6C4h] [ebp-4Ch]
    __m128 *v30; // [esp+6F0h] [ebp-20h]
    int i; // [esp+6F4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v30 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v30[8], 0);
        inNormal = *v30;
        v28 = (__m64)v30[1].m128_u64[0];
        v27 = v30[1].m128_u32[3];
        v25 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v21 = *v25;
        v22 = v25[1];
        v23 = v25[2];
        v24 = v25[3];
        v20 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v25, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v22, _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v23, _mm_shuffle_ps(inNormal, inNormal, 170)), v24));
        v19 = _mm_shuffle_ps(v20, _mm_unpackhi_ps(v20, *v30), 196);
        v17 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v30[1].m128_u32[2]), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v16 = _mm_mul_ps(v17, _mm_shuffle_ps(v17, v17, 255));
        v15 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v21, _mm_shuffle_ps(v16, v16, 0)), _mm_mul_ps(v22, _mm_shuffle_ps(v16, v16, 85))),
            _mm_mul_ps(v23, _mm_shuffle_ps(v16, v16, 170)));
        v18 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v15, _mm_unpackhi_ps(v15, v24), 196), sse_encodeScale), sse_encodeShift);
        v13 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v27), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v12 = _mm_mul_ps(v13, _mm_shuffle_ps(v13, v13, 255));
        v11 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v21, _mm_shuffle_ps(v12, v12, 0)), _mm_mul_ps(v22, _mm_shuffle_ps(v12, v12, 85))),
            _mm_mul_ps(v23, _mm_shuffle_ps(v12, v12, 170)));
        v14 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v11, _mm_unpackhi_ps(v11, v24), 196), sse_encodeScale), sse_encodeShift);
        v26 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v6) = HIWORD(v26);
        v10 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v26, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v26[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v26[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v26[3]));
        LOWORD(v6) = vertexBlend[2];
        v7 = _mm_cvtsi32_si64(v6);
        v8 = _m_punpcklwd(v7, v7);
        v9 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v9,
            _mm_add_ps(
                v19,
                _mm_mul_ps(
                    _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v8, v8)), sse_weightScale),
                    _mm_sub_ps(_mm_shuffle_ps(v10, _mm_unpackhi_ps(v10, inNormal), 196), v19))));
        _mm_stream_pi(v9 + 2, v28);
        _mm_stream_pi(
            v9 + 3,
            _m_packuswb(
                _m_packuswb(_mm_cvt_ps2pi(v18), _mm_cvt_ps2pi(_mm_movehl_ps(v18, v18))),
                _m_packuswb(_mm_cvt_ps2pi(v14), _mm_cvt_ps2pi(_mm_movehl_ps(v14, v14)))));
        vertexBlend += 3;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSseBlock_5_Sse_SkinVertex_2_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v6; // ecx
    __m64 v7; // mm0
    __m64 v8; // mm1
    unsigned int v9; // ecx
    __m64 v10; // mm0
    __m64 v11; // mm1
    __m64 *v12; // [esp-460h] [ebp-B50h]
    __m128 v13; // [esp-33Ch] [ebp-A2Ch]
    __m128 v14; // [esp-23Ch] [ebp-92Ch]
    __m128 v15; // [esp-21Ch] [ebp-90Ch]
    __m128 v16; // [esp-1FCh] [ebp-8ECh]
    __m128 v17; // [esp-1FCh] [ebp-8ECh]
    __m128 v18; // [esp-10Ch] [ebp-7FCh]
    __m128 v19; // [esp+D4h] [ebp-61Ch]
    __m128 v20; // [esp+1C4h] [ebp-52Ch]
    __m128 v21; // [esp+204h] [ebp-4ECh]
    __m128 v22; // [esp+234h] [ebp-4BCh]
    __m128 v23; // [esp+344h] [ebp-3ACh]
    __m128 v24; // [esp+434h] [ebp-2BCh]
    __m128 v25; // [esp+474h] [ebp-27Ch]
    __m128 v26; // [esp+4A4h] [ebp-24Ch]
    __m128 v27; // [esp+4D4h] [ebp-21Ch]
    __m128 v28; // [esp+544h] [ebp-1ACh]
    __m128 v29; // [esp+644h] [ebp-ACh]
    __m128 v30; // [esp+654h] [ebp-9Ch]
    __m128 v31; // [esp+664h] [ebp-8Ch]
    __m128 v32; // [esp+674h] [ebp-7Ch]
    __m128 *v33; // [esp+688h] [ebp-68h]
    __m128 *v34; // [esp+688h] [ebp-68h]
    __m128 *v35; // [esp+688h] [ebp-68h]
    unsigned int v36; // [esp+690h] [ebp-60h]
    __m64 v37; // [esp+69Ch] [ebp-54h]
    __m128 inNormal; // [esp+6A4h] [ebp-4Ch]
    __m128 *v39; // [esp+6D0h] [ebp-20h]
    int i; // [esp+6D4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6DCh] [ebp-14h]

    iassert(dstVerts);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v39 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v39[8], 0);
        inNormal = *v39;
        v37 = (__m64)v39[1].m128_u64[0];
        v36 = v39[1].m128_u32[3];
        v33 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v29 = *v33;
        v30 = v33[1];
        v31 = v33[2];
        v32 = v33[3];
        v28 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v33, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v30, _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v31, _mm_shuffle_ps(inNormal, inNormal, 170)), v32));
        v27 = _mm_shuffle_ps(v28, _mm_unpackhi_ps(v28, *v39), 196);
        v25 = _mm_div_ps(_mm_sub_ps(_mm_cvtpu8_ps(_mm_cvtsi32_si64(v39[1].m128_u32[2])), sse_encodeShift), sse_encodeScale);
        v24 = _mm_mul_ps(v25, _mm_shuffle_ps(v25, v25, 255));
        v23 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v29, _mm_shuffle_ps(v24, v24, 0)), _mm_mul_ps(v30, _mm_shuffle_ps(v24, v24, 85))),
            _mm_mul_ps(v31, _mm_shuffle_ps(v24, v24, 170)));
        v26 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v23, _mm_unpackhi_ps(v23, v32), 196), sse_encodeScale), sse_encodeShift);
        v21 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v36), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v20 = _mm_mul_ps(v21, _mm_shuffle_ps(v21, v21, 255));
        v19 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v29, _mm_shuffle_ps(v20, v20, 0)), _mm_mul_ps(v30, _mm_shuffle_ps(v20, v20, 85))),
            _mm_mul_ps(v31, _mm_shuffle_ps(v20, v20, 170)));
        v22 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v19, _mm_unpackhi_ps(v19, v32), 196), sse_encodeScale), sse_encodeShift);
        v34 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v6) = HIWORD(v34);
        v18 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v34, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v34[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v34[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v34[3]));
        LOWORD(v6) = vertexBlend[2];
        v7 = _mm_cvtsi32_si64(v6);
        v8 = _m_punpcklwd(v7, v7);
        v16 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v8, v8)), sse_weightScale);
        v15 = _mm_sub_ps(sse_one, v16);
        v14 = _mm_mul_ps(v16, _mm_shuffle_ps(v18, _mm_unpackhi_ps(v18, inNormal), 196));
        v35 = (__m128 *)((char *)boneMatrix + vertexBlend[3]);
        HIWORD(v9) = HIWORD(v35);
        v13 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v35, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v35[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v35[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v35[3]));
        LOWORD(v9) = vertexBlend[4];
        v10 = _mm_cvtsi32_si64(v9);
        v11 = _m_punpcklwd(v10, v10);
        v17 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v11, v11)), sse_weightScale);
        v12 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v12,
            _mm_add_ps(
                _mm_add_ps(v14, _mm_mul_ps(v17, _mm_shuffle_ps(v13, _mm_unpackhi_ps(v13, inNormal), 196))),
                _mm_mul_ps(_mm_sub_ps(v15, v17), v27)));
        _mm_stream_pi(v12 + 2, v37);
        _mm_stream_pi(
            v12 + 3,
            _m_packuswb(
                _m_packuswb(_mm_cvt_ps2pi(v26), _mm_cvt_ps2pi(_mm_movehl_ps(v26, v26))),
                _m_packuswb(_mm_cvt_ps2pi(v22), _mm_cvt_ps2pi(_mm_movehl_ps(v22, v22)))));
        vertexBlend += 5;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSseBlock_7_Sse_SkinVertex_3_(
    const GfxPackedVertex *srcVerts,
    const unsigned __int16 *vertexBlend,
    int vertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *dstVerts,
    int *pVertexIndex)
{
    unsigned int v6; // ecx
    __m64 v7; // mm0
    __m64 v8; // mm1
    unsigned int v9; // ecx
    __m64 v10; // mm0
    __m64 v11; // mm1
    unsigned int v12; // ecx
    __m64 v13; // mm0
    __m64 v14; // mm1
    __m64 *v15; // [esp-660h] [ebp-D50h]
    __m128 v16; // [esp-53Ch] [ebp-C2Ch]
    __m128 v17; // [esp-33Ch] [ebp-A2Ch]
    __m128 v18; // [esp-23Ch] [ebp-92Ch]
    __m128 v19; // [esp-23Ch] [ebp-92Ch]
    __m128 v20; // [esp-21Ch] [ebp-90Ch]
    __m128 v21; // [esp-21Ch] [ebp-90Ch]
    __m128 v22; // [esp-1FCh] [ebp-8ECh]
    __m128 v23; // [esp-1FCh] [ebp-8ECh]
    __m128 v24; // [esp-1FCh] [ebp-8ECh]
    __m128 v25; // [esp-10Ch] [ebp-7FCh]
    __m128 v26; // [esp+D4h] [ebp-61Ch]
    __m128 v27; // [esp+1C4h] [ebp-52Ch]
    __m128 v28; // [esp+204h] [ebp-4ECh]
    __m128 v29; // [esp+234h] [ebp-4BCh]
    __m128 v30; // [esp+344h] [ebp-3ACh]
    __m128 v31; // [esp+434h] [ebp-2BCh]
    __m128 v32; // [esp+474h] [ebp-27Ch]
    __m128 v33; // [esp+4A4h] [ebp-24Ch]
    __m128 v34; // [esp+4D4h] [ebp-21Ch]
    __m128 v35; // [esp+544h] [ebp-1ACh]
    __m128 v36; // [esp+644h] [ebp-ACh]
    __m128 v37; // [esp+654h] [ebp-9Ch]
    __m128 v38; // [esp+664h] [ebp-8Ch]
    __m128 v39; // [esp+674h] [ebp-7Ch]
    __m128 *v40; // [esp+688h] [ebp-68h]
    __m128 *v41; // [esp+688h] [ebp-68h]
    __m128 *v42; // [esp+688h] [ebp-68h]
    __m128 *v43; // [esp+688h] [ebp-68h]
    unsigned int v44; // [esp+690h] [ebp-60h]
    __m64 v45; // [esp+69Ch] [ebp-54h]
    __m128 inNormal; // [esp+6A4h] [ebp-4Ch]
    __m128 *v47; // [esp+6D0h] [ebp-20h]
    int i; // [esp+6D4h] [ebp-1Ch]
    const GfxPackedVertex *v; // [esp+6DCh] [ebp-14h]

    iassert(dstVerts);
    iassert(srcVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    v = (const GfxPackedVertex *)*pVertexIndex;
    for (i = 0; i < vertCount; ++i)
    {
        v47 = (__m128 *) & srcVerts[(_DWORD)v];
        _mm_prefetch((const char *)&v47[8], 0);
        inNormal = *v47;
        v45 = (__m64)v47[1].m128_u64[0];
        v44 = v47[1].m128_u32[3];
        v40 = (__m128 *)((char *)boneMatrix + *vertexBlend);
        v36 = *v40;
        v37 = v40[1];
        v38 = v40[2];
        v39 = v40[3];
        v35 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v40, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v37, _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v38, _mm_shuffle_ps(inNormal, inNormal, 170)), v39));
        v34 = _mm_shuffle_ps(v35, _mm_unpackhi_ps(v35, *v47), 196);
        v32 = _mm_div_ps(_mm_sub_ps(_mm_cvtpu8_ps(_mm_cvtsi32_si64(v47[1].m128_u32[2])), sse_encodeShift), sse_encodeScale);
        v31 = _mm_mul_ps(v32, _mm_shuffle_ps(v32, v32, 255));
        v30 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v36, _mm_shuffle_ps(v31, v31, 0)), _mm_mul_ps(v37, _mm_shuffle_ps(v31, v31, 85))),
            _mm_mul_ps(v38, _mm_shuffle_ps(v31, v31, 170)));
        v33 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v30, _mm_unpackhi_ps(v30, v39), 196), sse_encodeScale), sse_encodeShift);
        v28 = _mm_div_ps(
            _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v44), (__m64)0)), sse_encodeShift),
            sse_encodeScale);
        v27 = _mm_mul_ps(v28, _mm_shuffle_ps(v28, v28, 255));
        v26 = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(v36, _mm_shuffle_ps(v27, v27, 0)), _mm_mul_ps(v37, _mm_shuffle_ps(v27, v27, 85))),
            _mm_mul_ps(v38, _mm_shuffle_ps(v27, v27, 170)));
        v29 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v26, _mm_unpackhi_ps(v26, v39), 196), sse_encodeScale), sse_encodeShift);
        v41 = (__m128 *)((char *)boneMatrix + vertexBlend[1]);
        HIWORD(v6) = HIWORD(v41);
        v25 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v41, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v41[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v41[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v41[3]));
        LOWORD(v6) = vertexBlend[2];
        v7 = _mm_cvtsi32_si64(v6);
        v8 = _m_punpcklwd(v7, v7);
        v22 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v8, v8)), sse_weightScale);
        v20 = _mm_mul_ps(v22, _mm_shuffle_ps(v25, _mm_unpackhi_ps(v25, inNormal), 196));
        v18 = _mm_sub_ps(sse_one, v22);
        v42 = (__m128 *)((char *)boneMatrix + vertexBlend[3]);
        HIWORD(v9) = HIWORD(v42);
        v17 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v42, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v42[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v42[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v42[3]));
        LOWORD(v9) = vertexBlend[4];
        v10 = _mm_cvtsi32_si64(v9);
        v11 = _m_punpcklwd(v10, v10);
        v23 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v11, v11)), sse_weightScale);
        v19 = _mm_sub_ps(v18, v23);
        v21 = _mm_add_ps(v20, _mm_mul_ps(v23, _mm_shuffle_ps(v17, _mm_unpackhi_ps(v17, inNormal), 196)));
        v43 = (__m128 *)((char *)boneMatrix + vertexBlend[5]);
        HIWORD(v12) = HIWORD(v43);
        v16 = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*v43, _mm_shuffle_ps(inNormal, inNormal, 0)),
                _mm_mul_ps(v43[1], _mm_shuffle_ps(inNormal, inNormal, 85))),
            _mm_add_ps(_mm_mul_ps(v43[2], _mm_shuffle_ps(inNormal, inNormal, 170)), v43[3]));
        LOWORD(v12) = vertexBlend[6];
        v13 = _mm_cvtsi32_si64(v12);
        v14 = _m_punpcklwd(v13, v13);
        v24 = _mm_mul_ps(_mm_cvtpu16_ps(_m_punpcklwd(v14, v14)), sse_weightScale);
        v15 = (__m64 *) & dstVerts[(_DWORD)v];
        _mm_stream_ps(
            (float *)v15,
            _mm_add_ps(
                _mm_add_ps(v21, _mm_mul_ps(v24, _mm_shuffle_ps(v16, _mm_unpackhi_ps(v16, inNormal), 196))),
                _mm_mul_ps(_mm_sub_ps(v19, v24), v34)));
        _mm_stream_pi(v15 + 2, v45);
        _mm_stream_pi(
            v15 + 3,
            _m_packuswb(
                _m_packuswb(_mm_cvt_ps2pi(v33), _mm_cvt_ps2pi(_mm_movehl_ps(v33, v33))),
                _m_packuswb(_mm_cvt_ps2pi(v29), _mm_cvt_ps2pi(_mm_movehl_ps(v29, v29)))));
        vertexBlend += 7;
        v = (const GfxPackedVertex *)((char *)v + 1);
    }
    *pVertexIndex = (int)v;
}

void __cdecl R_SkinXSurfaceWeightSse(
    const GfxPackedVertex *inVerts,
    const XSurfaceVertexInfo *vertexInfo,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *outVerts)
{
    const unsigned __int16 *vertsBlend; // [esp+30h] [ebp-8h]
    int vertIndex; // [esp+34h] [ebp-4h] BYREF

    Profile_Begin(111);
    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if (vertexInfo->vertCount[0])
    {
        R_SkinXSurfaceWeightSseBlock_1_Sse_SkinVertex_0_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[0],
            boneMatrix,
            outVerts,
            &vertIndex);
        vertsBlend += vertexInfo->vertCount[0];
    }
    if (vertexInfo->vertCount[1])
    {
        R_SkinXSurfaceWeightSseBlock_3_Sse_SkinVertex_1_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[1],
            boneMatrix,
            outVerts,
            &vertIndex);
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if (vertexInfo->vertCount[2])
    {
        R_SkinXSurfaceWeightSseBlock_5_Sse_SkinVertex_2_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[2],
            boneMatrix,
            outVerts,
            &vertIndex);
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if (vertexInfo->vertCount[3])
        R_SkinXSurfaceWeightSseBlock_7_Sse_SkinVertex_3_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[3],
            boneMatrix,
            outVerts,
            &vertIndex);
    Profile_EndInternal(0);
}


void __cdecl R_SkinXSurfaceRigidSse(
    const XSurface *surf,
    int totalVertCount,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertex *dstVerts)
{
    __m128 v4; // [esp+D4h] [ebp-63Ch]
    __m128 v5; // [esp+1C4h] [ebp-54Ch]
    __m128 v6; // [esp+214h] [ebp-4FCh]
    __m128 v7; // [esp+234h] [ebp-4DCh]
    __m128 v8; // [esp+344h] [ebp-3CCh]
    __m128 v9; // [esp+434h] [ebp-2DCh]
    __m128 v10; // [esp+484h] [ebp-28Ch]
    __m128 v11; // [esp+4A4h] [ebp-26Ch]
    __m128 v12; // [esp+4F4h] [ebp-21Ch]
    __m128 v13; // [esp+564h] [ebp-1ACh]
    unsigned int v14; // [esp+664h] [ebp-ACh]
    __m64 v15; // [esp+66Ch] [ebp-A4h]
    int i; // [esp+6A0h] [ebp-70h]
    __m128 v17; // [esp+6A4h] [ebp-6Ch]
    __m128 matrix_4; // [esp+6B4h] [ebp-5Ch]
    __m128 matrix_20; // [esp+6C4h] [ebp-4Ch]
    __m128 matrix_36; // [esp+6D4h] [ebp-3Ch]
    __m128 *matrix_56; // [esp+6E8h] [ebp-28h]
    int matrix_60; // [esp+6ECh] [ebp-24h]
    XRigidVertList *v23; // [esp+6F0h] [ebp-20h]
    unsigned int matrixAddress; // [esp+6F4h] [ebp-1Ch]
    __m64 *vertCount; // [esp+6F8h] [ebp-18h]
    __m128 *vertList; // [esp+6FCh] [ebp-14h]

    iassert(dstVerts);
    iassert(!(reinterpret_cast<unsigned>(dstVerts) & 15));
    iassert(!(reinterpret_cast<unsigned>(boneMatrix) & 15));

    Profile_Begin(111);
    vertList = (__m128 *)surf->verts0;
    vertCount = (__m64 *)dstVerts;
    for (matrixAddress = 0; matrixAddress < surf->vertListCount; ++matrixAddress)
    {
        v23 = &surf->vertList[matrixAddress];
        matrix_60 = v23->vertCount;
        matrix_56 = (__m128 *)((char *)boneMatrix + v23->boneOffset);
        v17 = *matrix_56;
        matrix_4 = matrix_56[1];
        matrix_20 = matrix_56[2];
        matrix_36 = matrix_56[3];
        for (i = 0; i < matrix_60; ++i)
        {
            _mm_prefetch((const char *)&vertList[8], 0);
            v15 = (__m64)vertList[1].m128_u64[0];
            v14 = vertList[1].m128_u32[3];
            v13 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(v17, _mm_shuffle_ps(*vertList, *vertList, 0)),
                    _mm_mul_ps(matrix_4, _mm_shuffle_ps(*vertList, *vertList, 85))),
                _mm_add_ps(_mm_mul_ps(matrix_20, _mm_shuffle_ps(*vertList, *vertList, 170)), matrix_36));
            v12 = _mm_shuffle_ps(v13, _mm_unpackhi_ps(v13, *vertList), 196);
            v10 = _mm_div_ps(
                _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(vertList[1].m128_u32[2]), (__m64)0)), sse_encodeShift),
                sse_encodeScale);
            v9 = _mm_mul_ps(v10, _mm_shuffle_ps(v10, v10, 255));
            v8 = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(v17, _mm_shuffle_ps(v9, v9, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(v9, v9, 85))),
                _mm_mul_ps(matrix_20, _mm_shuffle_ps(v9, v9, 170)));
            v11 = _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v8, _mm_unpackhi_ps(v8, matrix_36), 196), sse_encodeScale),
                sse_encodeShift);
            v6 = _mm_div_ps(
                _mm_sub_ps(_mm_cvtpu16_ps(_m_punpcklbw(_mm_cvtsi32_si64(v14), (__m64)0)), sse_encodeShift),
                sse_encodeScale);
            v5 = _mm_mul_ps(v6, _mm_shuffle_ps(v6, v6, 255));
            v4 = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(v17, _mm_shuffle_ps(v5, v5, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(v5, v5, 85))),
                _mm_mul_ps(matrix_20, _mm_shuffle_ps(v5, v5, 170)));
            v7 = _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v4, _mm_unpackhi_ps(v4, matrix_36), 196), sse_encodeScale),
                sse_encodeShift);
            _mm_stream_ps((float *)vertCount, v12);
            _mm_stream_pi(vertCount + 2, v15);
            _mm_stream_pi(
                vertCount + 3,
                _m_packuswb(
                    _m_packuswb(_mm_cvt_ps2pi(v11), _mm_cvt_ps2pi(_mm_movehl_ps(v11, v11))),
                    _m_packuswb(_mm_cvt_ps2pi(v7), _mm_cvt_ps2pi(_mm_movehl_ps(v7, v7)))));
            vertList += 2;
            vertCount += 4;
        }
    }
    if (((char *)vertCount - (char *)dstVerts) >> 5 != totalVertCount)
        MyAssertHandler(".\\r_model_skin.cpp", 2019, 0, "%s", "vertex - dstVerts == totalVertCount");
    Profile_EndInternal(0);
}

void __cdecl R_SkinXSurfaceSkinnedSse(
    const XSurface *xsurf,
    const DObjSkelMat *boneMatrix,
    GfxPackedVertexNormal *skinVertNormalIn,
    GfxPackedVertexNormal *skinVertNormalOut,
    GfxPackedVertex *skinVerticesOut)
{
    if (skinVertNormalOut)
    {
        if (skinVertNormalIn)
        {
            if (xsurf->deformed)
                R_SkinXSurfaceWeightSseInOut(
                    xsurf->verts0,
                    &xsurf->vertInfo,
                    boneMatrix,
                    skinVertNormalIn,
                    skinVertNormalOut,
                    skinVerticesOut);
            else
                R_SkinXSurfaceRigidSseInOut(
                    xsurf,
                    xsurf->vertCount,
                    boneMatrix,
                    (__m64 *)skinVertNormalIn,
                    skinVertNormalOut,
                    skinVerticesOut);
        }
        else if (xsurf->deformed)
        {
            R_SkinXSurfaceWeightSseOut(xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVertNormalOut, skinVerticesOut);
        }
        else
        {
            R_SkinXSurfaceRigidSseOut(
                xsurf,
                xsurf->vertCount,
                boneMatrix,
                skinVertNormalOut,
                skinVerticesOut);
        }
    }
    else
    {
        if (skinVertNormalIn)
            MyAssertHandler(".\\r_model_skin.cpp", 2178, 0, "%s", "!skinVertNormalIn");
        if (xsurf->deformed)
            R_SkinXSurfaceWeightSse(xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVerticesOut);
        else
            R_SkinXSurfaceRigidSse(xsurf, xsurf->vertCount, boneMatrix, skinVerticesOut);
    }
}