#include "r_state.h"
#include "rb_logfile.h"
#include "r_dvars.h"
#include "rb_stats.h"
#include "r_draw_bsp.h"
#include "rb_state.h"
#include "rb_pixelcost.h"
#include "r_rendertarget.h"
#include "r_utils.h"

//float const *const shadowmapClearColor 820ebb50     gfx_d3d : r_state.obj
//bool g_renderTargetIsOverridden 85b5dd38     gfx_d3d : r_state.obj
//unsigned int *s_decodeSamplerFilterState 85b5dcb8     gfx_d3d : r_state.obj
//struct GfxScaledPlacement s_manualObjectPlacement 85b5dd18     gfx_d3d : r_state.obj

unsigned int s_decodeSamplerFilterState[24];

const _D3DTEXTUREFILTERTYPE s_mipFilterTable[4][3] =
{
  { D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_LINEAR },
  { D3DTEXF_NONE, D3DTEXF_LINEAR, D3DTEXF_LINEAR },
  { D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_POINT },
  { D3DTEXF_NONE, D3DTEXF_NONE, D3DTEXF_NONE }
}; // idb

const unsigned int s_cullTable_30[4] = { 0u, 1u, 3u, 2u }; // idb
const unsigned int s_blendTable_30[11] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
const unsigned int s_blendOpTable_30[6] = { 0, 1, 2, 3, 4, 5 };
const unsigned int s_depthTestTable_30[4] = { 8, 2, 3, 4 };
const unsigned int s_stencilOpTable_30[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
const unsigned int s_stencilFuncTable_30[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

void __cdecl R_ChangeIndices(GfxCmdBufPrimState *state, IDirect3DIndexBuffer9 *ib)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+4h] [ebp-4h]

    if (ib == state->indexBuffer)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h", 611, 0, "%s", "ib != state->indexBuffer");
    state->indexBuffer = ib;
    device = state->device;
    if (!state->device)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h", 615, 0, "%s", "device");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetIndices( ib )\n");
        //hr = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, IDirect3DIndexBuffer9 *))device->SetIndices)(
        //    device,
        //    device,
        //    ib);
        hr = device->SetIndices(ib);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetIndices( ib ) failed: %s\n",
                    616,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_ChangeStreamSource(
    GfxCmdBufPrimState *state,
    unsigned int streamIndex,
    IDirect3DVertexBuffer9 *vb,
    unsigned int vertexOffset,
    unsigned int vertexStride)
{
    const char *v5; // eax
    int hr; // [esp+0h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+4h] [ebp-4h]

    iassert(state->streams[streamIndex].vb != vb || state->streams[streamIndex].offset != vertexOffset || state->streams[streamIndex].stride != vertexStride);

    state->streams[streamIndex].vb = vb;
    state->streams[streamIndex].offset = vertexOffset;
    state->streams[streamIndex].stride = vertexStride;
    device = state->device;

    iassert(device);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetStreamSource( streamIndex, vb, vertexOffset, vertexStride )\n");
        hr = device->SetStreamSource(streamIndex, vb, vertexOffset, vertexStride);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetStreamSource( streamIndex, vb, vertexOffset, vertexSt"
                    "ride ) failed: %s\n",
                    638,
                    R_ErrorDescription(hr));
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_SetTexFilter()
{
    int v0; // [esp+0h] [ebp-60h]
    int v1; // [esp+4h] [ebp-5Ch]
    int v2; // [esp+8h] [ebp-58h]
    int v3; // [esp+Ch] [ebp-54h]
    int v4; // [esp+10h] [ebp-50h]
    int v5; // [esp+14h] [ebp-4Ch]
    int v6; // [esp+1Ch] [ebp-44h]
    int integer; // [esp+24h] [ebp-3Ch]
    unsigned int entryIndex; // [esp+30h] [ebp-30h]
    int maxAniso; // [esp+34h] [ebp-2Ch]
    unsigned int mipFilterMode; // [esp+38h] [ebp-28h]
    int linearMippedAnisotropy; // [esp+3Ch] [ebp-24h]
    int linearNonMippedFilter; // [esp+40h] [ebp-20h]
    int linearMippedFilter; // [esp+44h] [ebp-1Ch]
    int texFilter; // [esp+48h] [ebp-18h]
    int anisotropicFilter; // [esp+50h] [ebp-10h]
    int anisotropyFor4x; // [esp+54h] [ebp-Ch]
    _D3DTEXTUREFILTERTYPE decoded; // [esp+58h] [ebp-8h]
    unsigned int decodeda; // [esp+58h] [ebp-8h]
    int anisotropyFor2x; // [esp+5Ch] [ebp-4h]

    maxAniso = r_texFilterAnisoMax->current.integer;
    if (maxAniso > gfxMetrics.maxAnisotropy)
        maxAniso = gfxMetrics.maxAnisotropy;
    if (maxAniso > 1)
    {
        anisotropicFilter = (gfxMetrics.hasAnisotropicMinFilter ? 768 : 512)
            | (gfxMetrics.hasAnisotropicMagFilter ? 12288 : 0x2000);
    }
    else
    {
        maxAniso = 1;
        anisotropicFilter = 8704;
    }
    if (maxAniso >= r_texFilterAnisoMin->current.integer)
        integer = r_texFilterAnisoMin->current.integer;
    else
        integer = maxAniso;
    if (integer <= 1)
        v6 = 1;
    else
        v6 = integer;
    if (v6 != integer)
        MyAssertHandler(".\\r_state.cpp", 136, 0, "%s", "max( minAniso, min( maxAniso, 1 ) ) == minAniso");
    linearMippedAnisotropy = integer;
    if (integer == 1)
    {
        linearMippedFilter = 8704;
    }
    else
    {
        if (integer <= 1)
            MyAssertHandler(".\\r_state.cpp", 145, 0, "%s", "linearMippedAnisotropy > 1");
        linearMippedFilter = anisotropicFilter;
    }
    if (maxAniso >= 2)
        v5 = 2;
    else
        v5 = maxAniso;
    if (integer <= v5)
    {
        if (maxAniso >= 2)
            v3 = 2;
        else
            v3 = maxAniso;
        v4 = v3;
    }
    else
    {
        v4 = integer;
    }
    anisotropyFor2x = v4;
    if (maxAniso >= 4)
        v2 = 4;
    else
        v2 = maxAniso;
    if (integer <= v2)
    {
        if (maxAniso >= 4)
            v0 = 4;
        else
            v0 = maxAniso;
        v1 = v0;
    }
    else
    {
        v1 = integer;
    }
    anisotropyFor4x = v1;
    if (r_texFilterDisable->current.enabled)
    {
        linearMippedAnisotropy = 1;
        anisotropyFor2x = 1;
        anisotropyFor4x = 1;
        anisotropicFilter = 4352;
        linearMippedFilter = 4352;
        linearNonMippedFilter = 4352;
        mipFilterMode = 3;
    }
    else
    {
        linearNonMippedFilter = 8704;
        mipFilterMode = r_texFilterMipMode->current.unsignedInt;
        if (mipFilterMode >= 4)
            MyAssertHandler(
                ".\\r_state.cpp",
                169,
                0,
                "mipFilterMode doesn't index R_MIP_FILTER_COUNT\n\t%i not in [0, %i)",
                mipFilterMode,
                4);
    }
    for (entryIndex = 0; entryIndex < 0x18; ++entryIndex)
    {
        texFilter = entryIndex & 7;
        decoded = (_D3DTEXTUREFILTERTYPE)(s_mipFilterTable[mipFilterMode][(int)(entryIndex & 0x18) >> 3] << 16);
        switch (texFilter)
        {
        case 2:
            if (decoded)
                decodeda = decoded | linearMippedAnisotropy | linearMippedFilter;
            else
                decodeda = linearNonMippedFilter | 1;
            break;
        case 3:
            decodeda = decoded | anisotropyFor2x | anisotropicFilter;
            break;
        case 4:
            decodeda = decoded | anisotropyFor4x | anisotropicFilter;
            break;
        default:
            decodeda = decoded | 0x1101;
            break;
        }
        s_decodeSamplerFilterState[entryIndex] = decodeda;
    }
    dx.mipBias = r_texFilterMipBias->current.unsignedInt;
}

void __cdecl R_SetInitialContextState(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // [esp+0h] [ebp-Ch]
    int v5; // [esp+4h] [ebp-8h]
    int hr; // [esp+8h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, 1 )\n");
        hr = device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_state.cpp (%i) device->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, 1 ) failed: %s\n",
                    211,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, 1 )\n");
        v5 = device->SetRenderState(
            D3DRS_TWOSIDEDSTENCILMODE,
            1);
        if (v5 < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(v5);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_state.cpp (%i) device->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, 1 ) failed: %s\n",
                    212,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE )\n");
        v4 = device->SetRenderState(D3DRS_ZENABLE, 0);
        if (v4 < 0)
        {
            do
            {
                ++g_disableRendering;
                v3 = R_ErrorDescription(v4);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_state.cpp (%i) device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) failed: %s\n",
                    217,
                    v3);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_ChangeDepthHackNearClip(GfxCmdBufSourceState *source, unsigned int depthHackFlags)
{
    if (!source)
        MyAssertHandler(".\\r_state.cpp", 260, 0, "%s", "source");
    if (depthHackFlags != source->depthHackFlags)
    {
        R_DepthHackNearClipChanged(source);
        source->depthHackFlags = depthHackFlags;
    }
}

void __cdecl R_DepthHackNearClipChanged(GfxCmdBufSourceState *source)
{
    ++source->matrixVersions[2];
    ++source->matrixVersions[4];
    ++source->matrixVersions[5];
    source->input.consts[54][3] = -source->input.consts[54][3];
    R_DirtyCodeConstant(source, 0x36u);
}

GfxCmdBufSourceState *__cdecl R_GetCodeMatrix(
    GfxCmdBufSourceState *source,
    unsigned int sourceIndex,
    unsigned int firstRow)
{
    unsigned int baseIndex; // [esp+ECh] [ebp-18h]
    unsigned int transposeIndex; // [esp+F0h] [ebp-14h]
    unsigned int matrixIndex; // [esp+F8h] [ebp-Ch]
    unsigned int inverseIndex; // [esp+FCh] [ebp-8h]
    unsigned int matrixVersion; // [esp+100h] [ebp-4h]

    iassert((source->matrixVersions[(((sourceIndex)-CONST_SRC_FIRST_CODE_MATRIX) >> 2)]));
    iassert(firstRow >= 0 && firstRow <= 3);

    matrixVersion = source->matrixVersions[(sourceIndex - CONST_SRC_FIRST_CODE_MATRIX) >> 2];
    matrixIndex = sourceIndex - CONST_SRC_FIRST_CODE_MATRIX;

    if (source->constVersions[sourceIndex] == matrixVersion)
        return (GfxCmdBufSourceState *)((char *)source + 64 * matrixIndex + 16 * firstRow);

    baseIndex = matrixIndex & 0xFFFFFFFC;
    if (source->constVersions[(matrixIndex & 0xFFFFFFFC) + CONST_SRC_FIRST_CODE_MATRIX] != matrixVersion)
    {
        R_DeriveCodeMatrix(source, &source->matrices, baseIndex);
        if (matrixIndex == baseIndex)
            return (GfxCmdBufSourceState *)((char *)source + 64 * matrixIndex + 16 * firstRow);

        iassert(source->constVersions[sourceIndex] != matrixVersion);
    }
    source->constVersions[sourceIndex] = matrixVersion;
    transposeIndex = matrixIndex ^ 2;
    if (source->constVersions[(matrixIndex ^ 2) + CONST_SRC_FIRST_CODE_MATRIX] == matrixVersion)
    {
        MatrixTranspose44(source->matrices.matrix[transposeIndex].m, source->matrices.matrix[matrixIndex].m);

        return (GfxCmdBufSourceState *)((char *)source + 64 * matrixIndex + 16 * firstRow);
    }
    else
    {
        inverseIndex = matrixIndex ^ 1;
        if (source->constVersions[(matrixIndex ^ 1) + CONST_SRC_FIRST_CODE_MATRIX] == matrixVersion)
        {
            MatrixInverse44(source->matrices.matrix[inverseIndex].m, source->matrices.matrix[matrixIndex].m);

            return (GfxCmdBufSourceState *)((char *)source + 64 * matrixIndex + 16 * firstRow);
        }
        else
        {
            iassert(matrixIndex == (baseIndex | CONST_SRC_MATRIX_INVERSE_BIT | CONST_SRC_MATRIX_TRANSPOSE_BIT));
            iassert(transposeIndex == (baseIndex | CONST_SRC_MATRIX_INVERSE_BIT));
            iassert(inverseIndex == (baseIndex | CONST_SRC_MATRIX_TRANSPOSE_BIT));

            MatrixTranspose44(source->matrices.matrix[baseIndex].m, source->matrices.matrix[inverseIndex].m);
            source->constVersions[inverseIndex + 58] = matrixVersion;
            MatrixInverse44(source->matrices.matrix[inverseIndex].m, source->matrices.matrix[matrixIndex].m);

            return (GfxCmdBufSourceState *)((char *)source + 64 * matrixIndex + 16 * firstRow);
        }
    }
}

void __cdecl R_DeriveCodeMatrix(GfxCmdBufSourceState *source, GfxCodeMatrices *activeMatrices, unsigned int baseIndex)
{
    const char *v3; // eax

    switch (baseIndex) // LWSS: mystery enum of sorts
    {
    case 4:
        R_DeriveViewMatrix(source);
        break;
    case 8:
        R_DeriveProjectionMatrix(source);
        break;
    case 12:
        R_DeriveWorldViewMatrix(source);
        break;
    case 16:
        R_DeriveViewProjectionMatrix(source);
        break;
    case 20:
        R_DeriveWorldViewProjectionMatrix(source);
        break;
    case 24:
        R_DeriveShadowLookupMatrix(source);
        break;
    case 28:
        R_GenerateWorldOutdoorLookupMatrix(source, activeMatrices->matrix[baseIndex].m);
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("unhandled case %i", baseIndex);
            MyAssertHandler(".\\r_state.cpp", 543, 1, v3);
        }
        break;
    }
}

void __cdecl R_DeriveViewMatrix(GfxCmdBufSourceState *source)
{
    memcpy(&source->matrices.matrix[4], &source->viewParms, sizeof(source->matrices.matrix[4]));
    MatrixTransformVector44(source->eyeOffset, source->matrices.matrix[4].m, source->matrices.matrix[4].m[3]);
    source->constVersions[62] = source->matrixVersions[1];
}

void  R_DeriveWorldViewMatrix(GfxCmdBufSourceState *source)
{
    GfxMatrix world;
    GfxViewParms *p_viewParms; // [esp+44h] [ebp-10h]
    GfxCodeMatrices *activeMatrices; // [esp+4Ch] [ebp-8h]
    GfxCodeMatrices *retaddr; // [esp+54h] [ebp+0h]

    //activeMatrices = retaddr;
    p_viewParms = &source->viewParms;
    if (source->constVersions[58] != source->matrixVersions[0])
        MyAssertHandler(".\\r_state.cpp", 338, 0, "%s", "R_IsMatrixConstantUpToDate( source, CONST_SRC_CODE_WORLD_MATRIX )");
    memcpy(&world, (float *)source, sizeof(GfxMatrix));
    Vec3Add(world.m[3], source->eyeOffset, world.m[3]);
    MatrixMultiply44(world.m, p_viewParms->viewMatrix.m, source->matrices.matrix[12].m);
    source->constVersions[70] = source->matrixVersions[3];
}

void __cdecl R_DeriveProjectionMatrix(GfxCmdBufSourceState *source)
{
    memcpy(&source->matrices.matrix[8], &source->viewParms.projectionMatrix, sizeof(source->matrices.matrix[8]));
    if (source->depthHackFlags == 2)
        source->matrices.matrix[8].m[3][2] = source->viewParms.depthHackNearClip;
    source->constVersions[66] = source->matrixVersions[2];
}

void __cdecl R_DeriveViewProjectionMatrix(GfxCmdBufSourceState *source)
{
    GfxMatrix *viewProj; // [esp+14h] [ebp-10h]

    viewProj = &source->matrices.matrix[16];
    if (source->depthHackFlags == 2)
    {
        if (source->constVersions[66] != source->matrixVersions[2])
            R_DeriveProjectionMatrix(source);
        MatrixMultiply44(source->viewParms.viewMatrix.m, source->matrices.matrix[8].m, viewProj->m);
    }
    else
    {
        memcpy(viewProj, &source->viewParms.viewProjectionMatrix, sizeof(GfxMatrix));
    }
    MatrixTransformVector44(source->eyeOffset, source->matrices.matrix[16].m, source->matrices.matrix[16].m[3]);
    source->constVersions[74] = source->matrixVersions[4];
}

void  R_DeriveWorldViewProjectionMatrix(GfxCmdBufSourceState *source)
{
    float *mat20; // [esp-8h] [ebp-60h]
    GfxMatrix mat;
    GfxViewParms *p_viewParms; // [esp+48h] [ebp-10h]
    int v6; // [esp+4Ch] [ebp-Ch]
    GfxCodeMatrices *activeMatrices; // [esp+50h] [ebp-8h]
    GfxCodeMatrices *retaddr; // [esp+58h] [ebp+0h]

    //activeMatrices = retaddr;
    p_viewParms = &source->viewParms;

    //iassert(R_IsMatrixConstantUpToDate(source, CONST_SRC_CODE_WORLD_MATRIX));

    memcpy(&mat, (float*)source, sizeof(GfxMatrix));
    mat20 = (float *)&source->matrices.matrix[20];

    if (source->depthHackFlags == 2)
    {
        if (source->constVersions[74] != source->matrixVersions[4])
            R_DeriveViewProjectionMatrix(source);
        MatrixMultiply44(mat.m, source->matrices.matrix[16].m, *(mat4x4*)mat20);
    }
    else
    {
        Vec3Add(mat.m[3], source->eyeOffset, mat.m[3]);
        MatrixMultiply44(mat.m, p_viewParms->viewProjectionMatrix.m, *(mat4x4*)mat20);
    }
    source->constVersions[78] = source->matrixVersions[5];
}

void __cdecl R_DeriveShadowLookupMatrix(GfxCmdBufSourceState *source)
{
    memcpy(&source->matrices.matrix[24], &source->shadowLookupMatrix, sizeof(source->matrices.matrix[24]));
    MatrixTransformVector44(source->eyeOffset, source->matrices.matrix[24].m, source->matrices.matrix[24].m[3]);
    source->constVersions[82] = source->matrixVersions[6];
}

void  R_GenerateWorldOutdoorLookupMatrix(
    GfxCmdBufSourceState *source,
    float (*outMatrix)[4])
{
    GfxMatrix v3; // [esp-Ch] [ebp-8Ch] BYREF
    float world_52[4]; // [esp+34h] [ebp-4Ch] BYREF
    float zInTimesInvViewTimesOutdoorLookup_4[4]; // [esp+44h] [ebp-3Ch] BYREF
    float zInTimesInvView_4[4]; // [esp+54h] [ebp-2Ch] BYREF
    //GfxCmdBufSourceState *matrix0; // [esp+64h] [ebp-1Ch]
    const void *zIn_8; // [esp+68h] [ebp-18h]
    float zIn_12; // [esp+6Ch] [ebp-14h]
    float awayBias; // [esp+70h] [ebp-10h]
    GfxCodeMatrices *activeMatrices; // [esp+74h] [ebp-Ch]
    float downBias; // [esp+78h] [ebp-8h]
    float retaddr; // [esp+80h] [ebp+0h]

    //downBias = retaddr;
    awayBias = r_outdoorAwayBias->current.value;
    zIn_12 = r_outdoorDownBias->current.value;
    zIn_8 = source;
    //matrix0 = R_GetCodeMatrix(source, 63u, 0);
    zInTimesInvView_4[0] = 0.0;
    zInTimesInvView_4[1] = 0.0;
    zInTimesInvView_4[2] = -awayBias;
    zInTimesInvView_4[3] = 0.0;
    MatrixTransformVector44(zInTimesInvView_4, *(const mat4x4*)R_GetCodeMatrix(source, 63u, 0), zInTimesInvViewTimesOutdoorLookup_4);
    zInTimesInvViewTimesOutdoorLookup_4[2] = zInTimesInvViewTimesOutdoorLookup_4[2] + zIn_12;
    iassert(rgp.world);
    MatrixTransformVector44(zInTimesInvViewTimesOutdoorLookup_4, rgp.world->outdoorLookupMatrix, world_52);
    //iassert(R_IsMatrixConstantUpToDate(source, CONST_SRC_CODE_WORLD_MATRIX));
    qmemcpy(&v3, zIn_8, sizeof(v3));
    Vec3Add(v3.m[3], source->eyeOffset, v3.m[3]);
    MatrixMultiply44(v3.m, rgp.world->outdoorLookupMatrix, *(mat4x4*)outMatrix);
    Vec4Add(&(*outMatrix)[12], world_52, &(*outMatrix)[12]);
    source->constVersions[86] = source->matrixVersions[7];
}

const GfxImage *__cdecl R_GetTextureFromCode(
    GfxCmdBufSourceState *source,
    unsigned int codeTexture,
    unsigned __int8 *samplerState)
{
    const char *v3; // eax

    if (codeTexture >= 0x1B)
        MyAssertHandler(
            ".\\r_state.cpp",
            607,
            0,
            "codeTexture doesn't index TEXTURE_SRC_CODE_COUNT\n\t%i not in [0, %i)",
            codeTexture,
            27);
    if (!source)
        MyAssertHandler(".\\r_state.cpp", 608, 0, "%s", "source");
    *samplerState = source->input.codeImageSamplerStates[codeTexture];
    if ((*samplerState & 7) == 0)
    {
        v3 = va("R_GetTextureFromCode %d, %d", codeTexture, *samplerState);
        MyAssertHandler(".\\r_state.cpp", 611, 0, "%s\n\t%s", "*samplerState & SAMPLER_FILTER_MASK", v3);
    }
    return source->input.codeImages[codeTexture];
}

void __cdecl R_TextureFromCodeError(GfxCmdBufSourceState *source, unsigned int codeTexture)
{
    if (!rg.codeImageNames[codeTexture])
        MyAssertHandler(
            ".\\r_state.cpp",
            618,
            0,
            "%s\n\t(codeTexture) = %i",
            "(rg.codeImageNames[codeTexture])",
            codeTexture);
    Com_Error(ERR_DROP, "Tried to use '%s' when it isn't valid\n", rg.codeImageNames[codeTexture]);
}

const GfxImage *__cdecl R_OverrideGrayscaleImage(const dvar_s *dvar)
{
    const char *v2; // eax

    if (dvar->current.integer == 2)
        return rgp.whiteImage;
    if (!dvar->current.integer)
        return rgp.blackImage;
    if (dvar->current.integer != 3)
    {
        v2 = va("%s = %i", dvar->name, dvar->current.integer);
        MyAssertHandler(".\\r_state.cpp", 631, 0, "%s\n\t%s", "dvar->current.integer == R_COLOR_OVERRIDE_GRAY", v2);
    }
    return rgp.grayImage;
}

void __cdecl R_SetLightmap(GfxCmdBufContext context, unsigned int lmapIndex)
{
    const MaterialPass *pass; // [esp+0h] [ebp-8h]
    const GfxImage *overrideImage; // [esp+4h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_state.cpp", 641, 0, "%s", "rgp.world");
    pass = context.state->pass;
    if (lmapIndex == 31)
    {
        if ((pass->customSamplerFlags & 6) != 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                647,
                0,
                "%s",
                "!(pass->customSamplerFlags & ((1 << CUSTOM_SAMPLER_LIGHTMAP_PRIMARY) | (1 << CUSTOM_SAMPLER_LIGHTMAP_SECONDARY)))");
    }
    else
    {
        if (lmapIndex >= rgp.world->lightmapCount)
            MyAssertHandler(
                ".\\r_state.cpp",
                651,
                0,
                "lmapIndex doesn't index rgp.world->lightmapCount\n\t%i not in [0, %i)",
                lmapIndex,
                rgp.world->lightmapCount);
        if (r_lightMap->current.integer == 1)
        {
            if ((pass->customSamplerFlags & 2) != 0)
            {
                if (context.source->input.data->prim.hasSunDirChanged)
                    R_SetSampler(context, 2u, 0x62u, rgp.whiteImage);
                else
                    R_SetSampler(context, 2u, 0x62u, rgp.world->lightmaps[lmapIndex].primary);
            }
            if ((pass->customSamplerFlags & 4) != 0)
                R_SetSampler(context, 3u, 0x62u, rgp.world->lightmaps[lmapIndex].secondary);
        }
        else
        {
            overrideImage = R_OverrideGrayscaleImage(r_lightMap);
            if ((pass->customSamplerFlags & 2) != 0)
                R_SetSampler(context, 2u, 0x62u, overrideImage);
            if ((pass->customSamplerFlags & 4) != 0)
                R_SetSampler(context, 3u, 0x62u, overrideImage);
        }
    }
}

void __cdecl R_SetReflectionProbe(GfxCmdBufContext context, unsigned int reflectionProbeIndex)
{
    if (!rgp.world)
        MyAssertHandler(".\\r_state.cpp", 681, 0, "%s", "rgp.world");
    if (reflectionProbeIndex == 255)
        MyAssertHandler(".\\r_state.cpp", 682, 0, "%s", "reflectionProbeIndex != REFLECTION_PROBE_INVALID");
    if (reflectionProbeIndex >= rgp.world->reflectionProbeCount)
        MyAssertHandler(
            ".\\r_state.cpp",
            683,
            0,
            "reflectionProbeIndex doesn't index rgp.world->reflectionProbeCount\n\t%i not in [0, %i)",
            reflectionProbeIndex,
            rgp.world->reflectionProbeCount);
    if (!rgp.world->reflectionProbes[reflectionProbeIndex].reflectionImage)
        MyAssertHandler(".\\r_state.cpp", 684, 0, "%s", "rgp.world->reflectionProbes[reflectionProbeIndex].reflectionImage");
    if ((context.state->pass->customSamplerFlags & 1) != 0)
        R_SetSampler(context, 1u, 0x72u, rgp.world->reflectionProbes[reflectionProbeIndex].reflectionImage);
}

void __cdecl R_ChangeDepthRange(GfxCmdBufState *state, GfxDepthRangeType depthRangeType)
{
    float v2; // [esp+8h] [ebp-28h]
    float v3; // [esp+Ch] [ebp-24h]
    IDirect3DDevice9 *device; // [esp+2Ch] [ebp-4h]

    if (state->depthRangeType == depthRangeType)
        MyAssertHandler(
            ".\\r_state.cpp",
            707,
            1,
            "state->depthRangeType != depthRangeType\n\t%i, %i",
            state->depthRangeType,
            depthRangeType);
    state->depthRangeType = depthRangeType;
    if (depthRangeType)
        v3 = 0.0;
    else
        v3 = 0.015625;
    state->depthRangeNear = v3;
    if (depthRangeType)
        v2 = 0.015625;
    else
        v2 = 1.0;
    state->depthRangeFar = v2;
    device = state->prim.device;
    if (!device)
        MyAssertHandler(".\\r_state.cpp", 713, 0, "%s", "device");
    R_HW_SetViewport(device, &state->viewport, state->depthRangeNear, state->depthRangeFar);
}

void __cdecl R_HW_SetViewport(IDirect3DDevice9 *device, const GfxViewport *viewport, float nearValue, float farValue)
{
    const char *v4; // eax
    int hr; // [esp+10h] [ebp-1Ch]
    _D3DVIEWPORT9 d3dViewport; // [esp+14h] [ebp-18h] BYREF

    d3dViewport.X = viewport->x;
    d3dViewport.Y = viewport->y;
    d3dViewport.Width = viewport->width;
    d3dViewport.Height = viewport->height;
    d3dViewport.MinZ = nearValue;
    d3dViewport.MaxZ = farValue;
    if (farValue <= (double)nearValue)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h",
            247,
            0,
            "d3dViewport.MinZ < d3dViewport.MaxZ\n\t%g, %g",
            d3dViewport.MinZ,
            d3dViewport.MaxZ);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetViewport( &d3dViewport )\n");
        hr = device->SetViewport(&d3dViewport);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetViewport( &d3dViewport ) failed: %s\n",
                    248,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

int __cdecl R_BeginMaterial(GfxCmdBufState *state, const Material *material, MaterialTechniqueType techType)
{
    const char *v4; // eax
    const char *v5; // eax
    const MaterialTechnique *technique; // [esp+8h] [ebp-4h]

    if (!material)
        MyAssertHandler(".\\r_state.cpp", 722, 0, "%s", "material");
    technique = Material_GetTechnique(material, techType);
    if (!technique)
        return 0;
    if (r_logFile->current.integer)
    {
        v4 = RB_LogTechniqueType(techType);
        v5 = va("R_BeginMaterial( %s, %s, %s )\n", material->info.name, technique->name, v4);
        RB_LogPrint(v5);
    }
    state->material = material;
    state->techType = techType;
    state->technique = technique;
    return 1;
}

void __cdecl R_ClearAllStreamSources(GfxCmdBufPrimState *state)
{
    if (state->streams[0].vb || state->streams[0].offset || state->streams[0].stride)
        R_ChangeStreamSource(state, 0, 0, 0, 0);
    if (state->streams[1].vb || state->streams[1].offset || state->streams[1].stride)
        R_ChangeStreamSource(state, 1u, 0, 0, 0);
}

void __cdecl R_DrawIndexedPrimitive(GfxCmdBufPrimState *state, const GfxDrawPrimArgs *args)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-Ch]
    int triCount; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+8h] [ebp-4h]

    triCount = args->triCount;
    if (triCount >= r_drawPrimFloor->current.integer
        && (!r_drawPrimCap->current.integer || triCount <= r_drawPrimCap->current.integer))
    {
        if (r_skipDrawTris->current.enabled)
            triCount = 1;
        device = state->device;
        if (!state->device)
            MyAssertHandler(".\\r_state.cpp", 770, 0, "%s", "device");
        RB_TrackDrawPrimCall(triCount);
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseIndex, triCount )\n");
            hr = device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseIndex, triCount);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v2 = R_ErrorDescription(hr);
                    Com_Error(
                        ERR_FATAL,
                        ".\\r_state.cpp (%i) device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseInd"
                        "ex, triCount ) failed: %s\n",
                        782,
                        v2);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
}

void __cdecl R_ChangeState_0(GfxCmdBufState *state, unsigned int stateBits0)
{
    bool blendOpRgbWasEnabled; // [esp+2Fh] [ebp-Dh]
    int changedBits; // [esp+30h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+38h] [ebp-4h]

    changedBits = state->activeStateBits[0] ^ stateBits0;
    if (changedBits || ((state->refStateBits[0] ^ stateBits0) & 0x7000700) != 0)
    {
        if (r_logFile->current.integer)
            RB_LogPrintState_0(stateBits0, changedBits);
        if (!dx.d3d9 || !dx.device)
            MyAssertHandler(".\\r_state.cpp", 851, 0, "%s", "dx.d3d9 && dx.device");
        device = state->prim.device;
        if (!device)
            MyAssertHandler(".\\r_state.cpp", 854, 0, "%s", "device");
        if ((changedBits & 0x18000000) != 0)
            R_HW_SetColorMask(device, stateBits0);
        if ((changedBits & 0x800) != 0)
            R_HW_SetAlphaTestEnable(state->prim.device, stateBits0);
        if ((stateBits0 & 0x800) != 0)
        {
            if ((stateBits0 & 0x3000) != 0)
                MyAssertHandler(".\\r_state.cpp", 864, 0, "%s", "(stateBits0 & GFXS0_ATEST_MASK) == 0");
            stateBits0 |= state->activeStateBits[0] & 0x3000;
            changedBits &= 0xFFFFCFFF;
            if ((state->activeStateBits[0] ^ stateBits0) != changedBits)
                MyAssertHandler(".\\r_state.cpp", 867, 0, "%s", "(stateBits0 ^ state->activeStateBits[0]) == changedBits");
        }
        if ((changedBits & 0x3000) != 0)
            R_SetAlphaTestFunction(state, stateBits0);
        if ((changedBits & 0xC000) != 0)
            R_HW_SetCullFace(device, stateBits0);
        if (changedBits < 0)
            R_HW_SetPolygonMode(device, stateBits0);
        blendOpRgbWasEnabled = (state->refStateBits[0] & 0x700) != 0;
        if ((stateBits0 & 0x700) != 0)
        {
            if ((stateBits0 & 0x7000000) == 0)
            {
                stateBits0 = stateBits0 & 0xF800FFFF | ((stateBits0 & 0x7FF) << 16);
                changedBits = changedBits & 0xF800FFFF | (state->activeStateBits[0] ^ stateBits0) & 0x7FF0000;
                if ((state->activeStateBits[0] ^ stateBits0) != changedBits)
                    MyAssertHandler(".\\r_state.cpp", 906, 0, "%s", "(stateBits0 ^ state->activeStateBits[0]) == changedBits");
            }
            R_HW_SetBlend(device, blendOpRgbWasEnabled, changedBits, stateBits0);
        }
        else
        {
            if ((stateBits0 & 0x7000000) != 0)
                MyAssertHandler(
                    ".\\r_state.cpp",
                    883,
                    0,
                    "%s",
                    "(stateBits0 & GFXS0_BLENDOP_ALPHA_MASK) == (GFXS_BLENDOP_DISABLED << GFXS0_BLENDOP_ALPHA_SHIFT)");
            stateBits0 = stateBits0 & 0xF800F800 | state->activeStateBits[0] & 0x7FF07FF;
            changedBits &= 0xF800F800;
            if ((state->activeStateBits[0] ^ stateBits0) != changedBits)
                MyAssertHandler(".\\r_state.cpp", 887, 0, "%s", "(stateBits0 ^ state->activeStateBits[0]) == changedBits");
            if (blendOpRgbWasEnabled)
                R_HW_DisableBlend(device);
        }
        if (gfxMetrics.hasTransparencyMsaa && r_aaAlpha->current.integer && (changedBits & 0xF00) != 0)
            R_SetAlphaAntiAliasingState(device, stateBits0);
        state->activeStateBits[0] = stateBits0;
    }
}

void __cdecl R_HW_SetAlphaTestEnable(IDirect3DDevice9 *device, __int16 stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ALPHATESTENABLE, (stateBits0 & GFXS0_ATEST_DISABLE) ? 0 : 1 )\n");
        hr = device->SetRenderState(
            D3DRS_ALPHATESTENABLE,
            (stateBits0 & 0x800) == 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ALPHATESTENABLE, (stateBits"
                    "0 & GFXS0_ATEST_DISABLE) ? 0 : 1 ) failed: %s\n",
                    297,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetColorMask(IDirect3DDevice9 *device, unsigned int stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-8h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_COLORWRITEENABLE, mask )\n");
        hr = device->SetRenderState(
            D3DRS_COLORWRITEENABLE,
            ((stateBits0 & 0x8000000) != 0 ? 7 : 0) | ((stateBits0 & 0x10000000) != 0 ? 8 : 0));
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_COLORWRITEENABLE, mask ) failed: %s\n",
                    307,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetCullFace(IDirect3DDevice9 *device, __int16 stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    if ((stateBits0 & 0xC000) != 0x4000 && (stateBits0 & 0xC000) != 0xC000 && (stateBits0 & 0xC000) != 0x8000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h",
            313,
            0,
            "%s",
            "(stateBits0 & GFXS0_CULL_MASK) == GFXS0_CULL_NONE || (stateBits0 & GFXS0_CULL_MASK) == GFXS0_CULL_FRONT || (stateB"
            "its0 & GFXS0_CULL_MASK) == GFXS0_CULL_BACK");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_CULLMODE, s_cullTable[(stateBits0 & GFXS0_CULL_MASK) >> GFXS0_CULL_SHIFT] )\n");
        hr = device->SetRenderState(D3DRS_CULLMODE, s_cullTable_30[(unsigned __int16)(stateBits0 & 0xC000) >> 14]);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CULLMODE, s_cullTable[(stat"
                    "eBits0 & GFXS0_CULL_MASK) >> GFXS0_CULL_SHIFT] ) failed: %s\n",
                    314,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetPolygonMode(IDirect3DDevice9 *device, signed int stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "device->SetRenderState( D3DRS_FILLMODE, (stateBits0 & GFXS0_POLYMODE_LINE) ? D3DFILL_WIREFRAME : D3DFILL_SOLID )\n");
        hr = device->SetRenderState(
            D3DRS_FILLMODE,
            3 - (stateBits0 < 0));
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_FILLMODE, (stateBits0 & GFX"
                    "S0_POLYMODE_LINE) ? D3DFILL_WIREFRAME : D3DFILL_SOLID ) failed: %s\n",
                    320,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_DisableBlend(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 )\n");
        hr = device->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 ) failed: %s\n",
                    327,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetBlend(
    IDirect3DDevice9 *device,
    bool blendWasEnabled,
    unsigned int changedBits,
    unsigned int stateBits0)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v11; // [esp+0h] [ebp-1Ch]
    int v12; // [esp+4h] [ebp-18h]
    int v13; // [esp+8h] [ebp-14h]
    int v14; // [esp+Ch] [ebp-10h]
    int v15; // [esp+10h] [ebp-Ch]
    int v16; // [esp+14h] [ebp-8h]
    int hr; // [esp+18h] [ebp-4h]

    if (!blendWasEnabled)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 )\n");
            hr = device->SetRenderState(
                D3DRS_ALPHABLENDENABLE,
                1);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v4 = R_ErrorDescription(hr);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 ) failed: %s\n",
                        353,
                        v4);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((changedBits & 0x700) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint(
                    "device->SetRenderState( D3DRS_BLENDOP, s_blendOpTable[(stateBits0 >> GFXS0_BLENDOP_RGB_SHIFT) & GFXS_BLENDOP_MASK] )\n");
            v16 = device->SetRenderState(D3DRS_BLENDOP, s_blendOpTable_30[(stateBits0 >> 8) & 7]);
            if (v16 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v5 = R_ErrorDescription(v16);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_BLENDOP, s_blendOpTable[("
                        "stateBits0 >> GFXS0_BLENDOP_RGB_SHIFT) & GFXS_BLENDOP_MASK] ) failed: %s\n",
                        356,
                        v5);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((changedBits & 0xF) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint(
                    "device->SetRenderState( D3DRS_SRCBLEND, s_blendTable[(stateBits0 >> GFXS0_SRCBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] )\n");
            v15 = device->SetRenderState(D3DRS_SRCBLEND, s_blendTable_30[stateBits0 & 0xF]);
            if (v15 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v6 = R_ErrorDescription(v15);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SRCBLEND, s_blendTable[(s"
                        "tateBits0 >> GFXS0_SRCBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                        359,
                        v6);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((changedBits & 0xF0) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint(
                    "device->SetRenderState( D3DRS_DESTBLEND, s_blendTable[(stateBits0 >> GFXS0_DSTBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] )\n");
            v14 = device->SetRenderState(
                D3DRS_DESTBLEND,
                s_blendTable_30[(unsigned __int8)stateBits0 >> 4]);
            if (v14 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v7 = R_ErrorDescription(v14);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_DESTBLEND, s_blendTable[("
                        "stateBits0 >> GFXS0_DSTBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                        362,
                        v7);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((changedBits & 0x7000000) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint(
                    "device->SetRenderState( D3DRS_BLENDOPALPHA, s_blendOpTable[(stateBits0 >> GFXS0_BLENDOP_ALPHA_SHIFT) & GFXS_BLENDOP_MASK] )\n");
            v13 = device->SetRenderState(D3DRS_BLENDOPALPHA, s_blendOpTable_30[HIBYTE(stateBits0) & 7]);
            if (v13 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v8 = R_ErrorDescription(v13);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_BLENDOPALPHA, s_blendOpTa"
                        "ble[(stateBits0 >> GFXS0_BLENDOP_ALPHA_SHIFT) & GFXS_BLENDOP_MASK] ) failed: %s\n",
                        365,
                        v8);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((changedBits & 0xF0000) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint(
                    "device->SetRenderState( D3DRS_SRCBLENDALPHA, s_blendTable[(stateBits0 >> GFXS0_SRCBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] )\n");
            v12 = device->SetRenderState(D3DRS_SRCBLENDALPHA, s_blendTable_30[HIWORD(stateBits0) & 0xF]);
            if (v12 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v9 = R_ErrorDescription(v12);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SRCBLENDALPHA, s_blendTab"
                        "le[(stateBits0 >> GFXS0_SRCBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                        368,
                        v9);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((changedBits & 0xF00000) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint(
                    "device->SetRenderState( D3DRS_DESTBLENDALPHA, s_blendTable[(stateBits0 >> GFXS0_DSTBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] )\n");
            v11 = device->SetRenderState(
                D3DRS_DESTBLENDALPHA,
                s_blendTable_30[(stateBits0 >> 20) & 0xF]);
            if (v11 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v10 = R_ErrorDescription(v11);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_DESTBLENDALPHA, s_blendTa"
                        "ble[(stateBits0 >> GFXS0_DSTBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                        371,
                        v10);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
}

void __cdecl R_SetAlphaTestFunction(GfxCmdBufState *state, __int16 stateBits0)
{
    const char *v2; // eax
    const char *v3; // eax
    int v4; // [esp+0h] [ebp-14h]
    int hr; // [esp+4h] [ebp-10h]
    unsigned __int8 ref; // [esp+Bh] [ebp-9h]
    IDirect3DDevice9 *device; // [esp+Ch] [ebp-8h]
    unsigned int function; // [esp+10h] [ebp-4h]

    if ((stateBits0 & 0x3000) == 0x1000)
    {
        function = 5;
        ref = 0;
    }
    else if ((stateBits0 & 0x3000) == 0x2000)
    {
        function = 2;
        ref = 0x80;
    }
    else
    {
        if ((stateBits0 & 0x3000) != 0x3000)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
                705,
                1,
                "%s",
                "(stateBits0 & GFXS0_ATEST_MASK) == GFXS0_ATEST_GE_128");
        function = 7;
        ref = 0x80;
    }
    device = state->prim.device;
    if (!device)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h", 711, 0, "%s", "device");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ALPHAFUNC, function )\n");
        hr = device->SetRenderState(D3DRS_ALPHAFUNC, function);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderState( D3DRS_ALPHAFUNC, function ) failed: %s\n",
                    713,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    if (state->alphaRef != ref)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetRenderState( D3DRS_ALPHAREF, ref )\n");
            v4 = device->SetRenderState(D3DRS_ALPHAREF, ref);
            if (v4 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v3 = R_ErrorDescription(v4);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderState( D3DRS_ALPHAREF, ref ) failed: %s\n",
                        716,
                        v3);
                } while (alwaysfails);
            }
        } while (alwaysfails);
        state->alphaRef = ref;
    }
}

void __cdecl R_ChangeState_1(GfxCmdBufState *state, unsigned int stateBits1)
{
    int changedBits; // [esp+30h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+38h] [ebp-4h]

    changedBits = state->activeStateBits[1] ^ stateBits1;
    if (changedBits)
    {
        if (!(stateBits1 & 0x40 | ((stateBits1 & 0x80) == 0)))
            MyAssertHandler(
                ".\\r_state.cpp",
                937,
                0,
                "%s",
                "!(stateBits1 & GFXS1_STENCIL_BACK_ENABLE) | (stateBits1 & GFXS1_STENCIL_FRONT_ENABLE)");
        if (r_logFile->current.integer)
            RB_LogPrintState_1(stateBits1, changedBits);
        if (!dx.d3d9 || !dx.device)
            MyAssertHandler(".\\r_state.cpp", 942, 0, "%s", "dx.d3d9 && dx.device");
        device = state->prim.device;
        if (!device)
            MyAssertHandler(".\\r_state.cpp", 945, 0, "%s", "device");
        if ((changedBits & 1) != 0)
            R_HW_SetDepthWriteEnable(device, stateBits1);
        if ((changedBits & 2) != 0)
            R_HW_SetDepthTestEnable(device, stateBits1);
        if ((stateBits1 & 2) != 0)
        {
            if ((stateBits1 & 0xC) != 0)
                MyAssertHandler(".\\r_state.cpp", 953, 0, "%s", "(stateBits1 & GFXS1_DEPTHTEST_MASK) == 0");
            stateBits1 |= state->activeStateBits[1] & 0xC;
            changedBits &= 0xFFFFFFF3;
            if ((state->activeStateBits[1] ^ stateBits1) != changedBits)
                MyAssertHandler(".\\r_state.cpp", 956, 0, "%s", "(stateBits1 ^ state->activeStateBits[1]) == changedBits");
        }
        if ((changedBits & 0xC) != 0)
            R_HW_SetDepthTestFunction(device, stateBits1);
        if ((changedBits & 0x30) != 0)
            R_ForceSetPolygonOffset(device, stateBits1);
        if ((stateBits1 & 0x40) != 0)
        {
            if ((changedBits & 0x40) != 0)
                R_HW_EnableStencil(device);
        }
        else
        {
            if ((changedBits & 0x40) != 0)
                R_HW_DisableStencil(device);
            stateBits1 = stateBits1 & 0x7F | state->activeStateBits[1] & 0xFFFFFF80;
            changedBits &= 0x7Fu;
        }
        if ((stateBits1 & 0x80) == 0)
        {
            stateBits1 = stateBits1 & 0xFFFFF | ((stateBits1 & 0xFFF00) << 12);
            changedBits = state->activeStateBits[1] ^ stateBits1;
        }
        if ((changedBits & 0x1FF00) != 0)
            R_HW_SetFrontStencilOp(device, (stateBits1 >> 8) & 7, (stateBits1 >> 11) & 7, (stateBits1 >> 14) & 7);
        if ((changedBits & 0x1FF00000) != 0)
            R_HW_SetBackStencilOp(device, (stateBits1 >> 20) & 7, (stateBits1 >> 23) & 7, (stateBits1 >> 26) & 7);
        if ((changedBits & 0xE0000) != 0)
            R_HW_SetFrontStencilFunc(device, (stateBits1 >> 17) & 7);
        if ((changedBits & 0xE0000000) != 0)
            R_HW_SetBackStencilFunc(device, stateBits1 >> 29);
        state->activeStateBits[1] = stateBits1;
    }
}

void __cdecl R_HW_SetDepthWriteEnable(IDirect3DDevice9 *device, char stateBits1)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ZWRITEENABLE, (stateBits1 & GFXS1_DEPTHWRITE) ? 1 : 0 )\n");
        hr = device->SetRenderState(
            D3DRS_ZWRITEENABLE,
            (stateBits1 & 1) != 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ZWRITEENABLE, (stateBits1 &"
                    " GFXS1_DEPTHWRITE) ? 1 : 0 ) failed: %s\n",
                    395,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetDepthTestEnable(IDirect3DDevice9 *device, char stateBits1)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ZENABLE, (stateBits1 & GFXS1_DEPTHTEST_DISABLE) ? D3DZB_FALSE : D3DZB_TRUE )\n");
        hr = device->SetRenderState(
            D3DRS_ZENABLE,
            (stateBits1 & 2) == 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ZENABLE, (stateBits1 & GFXS"
                    "1_DEPTHTEST_DISABLE) ? D3DZB_FALSE : D3DZB_TRUE ) failed: %s\n",
                    401,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetDepthTestFunction(IDirect3DDevice9 *device, char stateBits1)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "device->SetRenderState( D3DRS_ZFUNC, s_depthTestTable[(stateBits1 & GFXS1_DEPTHTEST_MASK) >> GFXS1_DEPTHTEST_SHIFT] )\n");
        hr = device->SetRenderState(D3DRS_ZFUNC, s_depthTestTable_30[(unsigned __int8)(stateBits1 & 0xC) >> 2]);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ZFUNC, s_depthTestTable[(st"
                    "ateBits1 & GFXS1_DEPTHTEST_MASK) >> GFXS1_DEPTHTEST_SHIFT] ) failed: %s\n",
                    407,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_EnableStencil(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_STENCILENABLE, 1 )\n");
        hr = device->SetRenderState(D3DRS_STENCILENABLE, 1u);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILENABLE, 1 ) failed: %s\n",
                    413,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_DisableStencil(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_STENCILENABLE, 0 )\n");
        hr = device->SetRenderState(D3DRS_STENCILENABLE, 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILENABLE, 0 ) failed: %s\n",
                    419,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetFrontStencilOp(
    IDirect3DDevice9 *device,
    unsigned int stencilOpPass,
    unsigned int stencilOpFail,
    unsigned int stencilOpZFail)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int v7; // [esp+0h] [ebp-Ch]
    int v8; // [esp+4h] [ebp-8h]
    int hr; // [esp+8h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_STENCILPASS, s_stencilOpTable[stencilOpPass] )\n");
        hr = device->SetRenderState(D3DRS_STENCILPASS, s_stencilOpTable_30[stencilOpPass]);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILPASS, s_stencilOpTab"
                    "le[stencilOpPass] ) failed: %s\n",
                    425,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_STENCILFAIL, s_stencilOpTable[stencilOpFail] )\n");
        v8 = device->SetRenderState(D3DRS_STENCILFAIL, s_stencilOpTable_30[stencilOpFail]);
        if (v8 < 0)
        {
            do
            {
                ++g_disableRendering;
                v5 = R_ErrorDescription(v8);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILFAIL, s_stencilOpTab"
                    "le[stencilOpFail] ) failed: %s\n",
                    426,
                    v5);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_STENCILZFAIL, s_stencilOpTable[stencilOpZFail] )\n");
        v7 = device->SetRenderState(D3DRS_STENCILZFAIL, s_stencilOpTable_30[stencilOpZFail]);
        if (v7 < 0)
        {
            do
            {
                ++g_disableRendering;
                v6 = R_ErrorDescription(v7);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILZFAIL, s_stencilOpTa"
                    "ble[stencilOpZFail] ) failed: %s\n",
                    427,
                    v6);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetBackStencilOp(
    IDirect3DDevice9 *device,
    unsigned int stencilOpPass,
    unsigned int stencilOpFail,
    unsigned int stencilOpZFail)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int v7; // [esp+0h] [ebp-Ch]
    int v8; // [esp+4h] [ebp-8h]
    int hr; // [esp+8h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILPASS, s_stencilOpTable[stencilOpPass] )\n");
        hr = device->SetRenderState(D3DRS_CCW_STENCILPASS, s_stencilOpTable_30[stencilOpPass]);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILPASS, s_stencilO"
                    "pTable[stencilOpPass] ) failed: %s\n",
                    433,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILFAIL, s_stencilOpTable[stencilOpFail] )\n");
        v8 = device->SetRenderState(D3DRS_CCW_STENCILFAIL, s_stencilOpTable_30[stencilOpFail]);
        if (v8 < 0)
        {
            do
            {
                ++g_disableRendering;
                v5 = R_ErrorDescription(v8);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILFAIL, s_stencilO"
                    "pTable[stencilOpFail] ) failed: %s\n",
                    434,
                    v5);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILZFAIL, s_stencilOpTable[stencilOpZFail] )\n");
        v7 = device->SetRenderState(D3DRS_CCW_STENCILZFAIL, s_stencilOpTable_30[stencilOpZFail]);
        if (v7 < 0)
        {
            do
            {
                ++g_disableRendering;
                v6 = R_ErrorDescription(v7);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILZFAIL, s_stencil"
                    "OpTable[stencilOpZFail] ) failed: %s\n",
                    435,
                    v6);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetFrontStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_STENCILFUNC, s_stencilFuncTable[stencilFunc] )\n");
        hr = device->SetRenderState(D3DRS_STENCILFUNC, s_stencilFuncTable_30[stencilFunc]);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILFUNC, s_stencilFuncT"
                    "able[stencilFunc] ) failed: %s\n",
                    441,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_HW_SetBackStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILFUNC, s_stencilFuncTable[stencilFunc] )\n");
        hr = device->SetRenderState(D3DRS_CCW_STENCILFUNC, s_stencilFuncTable_30[stencilFunc]);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILFUNC, s_stencilF"
                    "uncTable[stencilFunc] ) failed: %s\n",
                    447,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_SetSampler(
    GfxCmdBufContext context,
    unsigned int samplerIndex,
    unsigned __int8 samplerState,
    const GfxImage *image)
{
    unsigned int decodedSamplerState; // [esp+Ch] [ebp-4h]

    iassert(image);
    if (context.state->samplerTexture[samplerIndex] != &image->texture)
    {
        context.state->samplerTexture[samplerIndex] = &image->texture;
        if (r_logFile->current.integer)
        {
            RB_LogPrint(va("---------- texture %i: %s\n", samplerIndex, image->name));
        }
        R_HW_SetSamplerTexture(context.state->prim.device, samplerIndex, &image->texture);
    }
    iassert((samplerState & (SAMPLER_FILTER_MASK | SAMPLER_MIPMAP_MASK)) != 0);
    if (context.state->refSamplerState[samplerIndex] != samplerState)
    {
        context.state->refSamplerState[samplerIndex] = samplerState;
        decodedSamplerState = R_DecodeSamplerState(samplerState);
        if (context.state->samplerState[samplerIndex] != decodedSamplerState)
            context.state->samplerState[samplerIndex] = R_HW_SetSamplerState(
                context.state->prim.device,
                samplerIndex,
                decodedSamplerState,
                context.state->samplerState[samplerIndex]);
    }
}

unsigned int __cdecl R_HW_SetSamplerState(
    IDirect3DDevice9 *device,
    unsigned int samplerIndex,
    unsigned int samplerState,
    unsigned int oldSamplerState)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v12; // [esp+0h] [ebp-38h]
    int v13; // [esp+4h] [ebp-34h]
    int v14; // [esp+8h] [ebp-30h]
    int v15; // [esp+Ch] [ebp-2Ch]
    int v16; // [esp+10h] [ebp-28h]
    int v17; // [esp+14h] [ebp-24h]
    int hr; // [esp+18h] [ebp-20h]
    unsigned int finalSamplerState; // [esp+1Ch] [ebp-1Ch]
    unsigned int diffSamplerState; // [esp+2Ch] [ebp-Ch]

    finalSamplerState = samplerState;
    diffSamplerState = oldSamplerState ^ samplerState;
    if (oldSamplerState == samplerState)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h", 143, 0, "%s", "diffSamplerState");
    if ((diffSamplerState & 0xF00) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MINFILTER, minFilter )\n");
            hr = device->SetSamplerState(
                samplerIndex,
                D3DSAMP_MINFILTER,
                (unsigned __int16)(samplerState & 0xF00) >> 8);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v4 = R_ErrorDescription(hr);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MINFILTE"
                        "R, minFilter ) failed: %s\n",
                        148,
                        v4);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((diffSamplerState & 0xF000) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAGFILTER, magFilter )\n");
            v17 = device->SetSamplerState(
                samplerIndex,
                D3DSAMP_MAGFILTER,
                (unsigned __int16)(samplerState & 0xF000) >> 12);
            if (v17 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v5 = R_ErrorDescription(v17);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MAGFILTE"
                        "R, magFilter ) failed: %s\n",
                        154,
                        v5);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((_BYTE)diffSamplerState)
    {
        if ((unsigned __int8)samplerState <= 1u)
        {
            finalSamplerState = (unsigned __int8)oldSamplerState | samplerState & 0xFFFFFF00;
        }
        else
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAXANISOTROPY, anisotropy )\n");
                v16 = device->SetSamplerState(
                    samplerIndex,
                    D3DSAMP_MAXANISOTROPY,
                    (unsigned __int8)samplerState);
                if (v16 < 0)
                {
                    do
                    {
                        ++g_disableRendering;
                        v6 = R_ErrorDescription(v16);
                        Com_Error(
                            ERR_FATAL,
                            "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MAXANI"
                            "SOTROPY, anisotropy ) failed: %s\n",
                            161,
                            v6);
                    } while (alwaysfails);
                }
            } while (alwaysfails);
        }
    }
    if ((diffSamplerState & 0xF0000) != 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MIPFILTER, mipFilter )\n");
            v15 = device->SetSamplerState(samplerIndex, D3DSAMP_MIPFILTER, (samplerState & 0xF0000) >> 16);
            if (v15 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v7 = R_ErrorDescription(v15);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MIPFILTE"
                        "R, mipFilter ) failed: %s\n",
                        169,
                        v7);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if ((diffSamplerState & 0x3F00000) != 0)
    {
        if ((diffSamplerState & 0x300000) != 0)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSU, address )\n");
                v14 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, (samplerState & 0x300000) >> 20);
                if (v14 < 0)
                {
                    do
                    {
                        ++g_disableRendering;
                        v8 = R_ErrorDescription(v14);
                        Com_Error(
                            ERR_FATAL,
                            "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_ADDRES"
                            "SU, address ) failed: %s\n",
                            177,
                            v8);
                    } while (alwaysfails);
                }
            } while (alwaysfails);
        }
        if ((diffSamplerState & 0xC00000) != 0)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSV, address )\n");
                v13 = device->SetSamplerState(
                    samplerIndex,
                    D3DSAMP_ADDRESSV,
                    (samplerState & 0xC00000) >> 22);
                if (v13 < 0)
                {
                    do
                    {
                        ++g_disableRendering;
                        v9 = R_ErrorDescription(v13);
                        Com_Error(
                            ERR_FATAL,
                            "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_ADDRES"
                            "SV, address ) failed: %s\n",
                            182,
                            v9);
                    } while (alwaysfails);
                }
            } while (alwaysfails);
        }
        if ((diffSamplerState & 0x3000000) != 0)
        {
            do
            {
                if (r_logFile && r_logFile->current.integer)
                    RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSW, address )\n");
                v12 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSW, (samplerState & 0x3000000) >> 24);
                if (v12 < 0)
                {
                    do
                    {
                        ++g_disableRendering;
                        v10 = R_ErrorDescription(v12);
                        Com_Error(
                            ERR_FATAL,
                            "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_ADDRES"
                            "SW, address ) failed: %s\n",
                            187,
                            v10);
                    } while (alwaysfails);
                }
            } while (alwaysfails);
        }
    }
    return finalSamplerState;
}

unsigned int __cdecl R_DecodeSamplerState(unsigned __int8 samplerState)
{
    unsigned int tableIndex; // [esp+0h] [ebp-8h]

    tableIndex = samplerState & 0x1F;
    bcassert(tableIndex, ARRAY_COUNT(s_decodeSamplerFilterState));

    return s_decodeSamplerFilterState[tableIndex]
        | 0x1500000
            | ((samplerState & 0x20) << 16)
            | ((samplerState & 0x40) << 17)
            | ((samplerState & 0x80) << 18);
}

void __cdecl R_SetSamplerState(GfxCmdBufState *state, unsigned int samplerIndex, unsigned __int8 samplerState)
{
    unsigned int decodedSamplerState; // [esp+8h] [ebp-4h]

    state->samplerTexture[samplerIndex] = 0;
    if ((samplerState & 0x1F) == 0)
        MyAssertHandler(
            ".\\r_state.cpp",
            1071,
            0,
            "%s\n\t(samplerState) = %i",
            "((samplerState & (SAMPLER_FILTER_MASK | SAMPLER_MIPMAP_MASK)) != 0)",
            samplerState);
    if (state->refSamplerState[samplerIndex] != samplerState)
    {
        state->refSamplerState[samplerIndex] = samplerState;
        decodedSamplerState = R_DecodeSamplerState(samplerState);
        if (state->samplerState[samplerIndex] != decodedSamplerState)
            state->samplerState[samplerIndex] = R_HW_SetSamplerState(
                state->prim.device,
                samplerIndex,
                decodedSamplerState,
                state->samplerState[samplerIndex]);
    }
}

void __cdecl R_ForceSetBlendState(IDirect3DDevice9 *device, unsigned int stateBits0)
{
    if ((stateBits0 & 0x700) != 0)
        R_HW_SetBlend(device, 0, 0xFFFFFFFF, stateBits0);
    else
        R_HW_DisableBlend(device);
}

void __cdecl R_ForceSetStencilState(IDirect3DDevice9 *device, unsigned int stateBits1)
{
    if ((stateBits1 & 0x40) != 0)
    {
        R_HW_EnableStencil(device);
        if ((stateBits1 & 0x80) == 0)
            stateBits1 = stateBits1 & 0xFFFFF | ((stateBits1 & 0xFFF00) << 12);
        R_HW_SetFrontStencilOp(device, (stateBits1 >> 8) & 7, (stateBits1 >> 11) & 7, (stateBits1 >> 14) & 7);
        R_HW_SetBackStencilOp(device, (stateBits1 >> 20) & 7, (stateBits1 >> 23) & 7, (stateBits1 >> 26) & 7);
        R_HW_SetFrontStencilFunc(device, (stateBits1 >> 17) & 7);
        R_HW_SetBackStencilFunc(device, stateBits1 >> 29);
    }
    else
    {
        R_HW_DisableStencil(device);
    }
}

void __cdecl R_GetViewport(GfxCmdBufSourceState *source, GfxViewport *outViewport)
{
    int v2; // [esp+0h] [ebp-10h]
    int v3; // [esp+4h] [ebp-Ch]

    if (!source)
        MyAssertHandler(".\\r_state.cpp", 1163, 0, "%s", "source");
    if (source->viewportBehavior == GFX_USE_VIEWPORT_FULL)
    {
        if (source->renderTargetWidth <= 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                1167,
                0,
                "%s\n\t(source->renderTargetWidth) = %i",
                "(source->renderTargetWidth > 0)",
                source->renderTargetWidth);
        if (source->renderTargetHeight <= 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                1168,
                0,
                "%s\n\t(source->renderTargetHeight) = %i",
                "(source->renderTargetHeight > 0)",
                source->renderTargetHeight);
        outViewport->x = 0;
        outViewport->y = 0;
        outViewport->width = source->renderTargetWidth;
        outViewport->height = source->renderTargetHeight;
    }
    else
    {
        if (source->sceneViewport.width <= 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                1177,
                0,
                "%s\n\t(source->sceneViewport.width) = %i",
                "(source->sceneViewport.width > 0)",
                source->sceneViewport.width);
        if (source->sceneViewport.height <= 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                1178,
                0,
                "%s\n\t(source->sceneViewport.height) = %i",
                "(source->sceneViewport.height > 0)",
                source->sceneViewport.height);
        *outViewport = source->sceneViewport;
        if (outViewport->width <= 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                1181,
                0,
                "%s\n\t(outViewport->width) = %i",
                "(outViewport->width > 0)",
                outViewport->width);
        if (outViewport->height <= 0)
            MyAssertHandler(
                ".\\r_state.cpp",
                1182,
                0,
                "%s\n\t(outViewport->height) = %i",
                "(outViewport->height > 0)",
                outViewport->height);
    }
    if (source->viewMode != VIEW_MODE_2D && r_scaleViewport->current.value != 1.0)
    {
        outViewport->x += (int)((double)outViewport->width * (1.0 - r_scaleViewport->current.value) * 0.5);
        outViewport->y += (int)((double)outViewport->height * (1.0 - r_scaleViewport->current.value) * 0.5);
        if ((int)(r_scaleViewport->current.value * (double)outViewport->width) > 1)
            v3 = (int)(r_scaleViewport->current.value * (double)outViewport->width);
        else
            v3 = 1;
        outViewport->width = v3;
        if ((int)(r_scaleViewport->current.value * (double)outViewport->height) > 1)
            v2 = (int)(r_scaleViewport->current.value * (double)outViewport->height);
        else
            v2 = 1;
        outViewport->height = v2;
    }
}

void __cdecl R_SetViewport(GfxCmdBufState *state, const GfxViewport *viewport)
{
    const char *v2; // eax

    if (r_logFile->current.integer)
    {
        v2 = va("Viewport at (%i, %i) with size %i x %i\n", viewport->x, viewport->y, viewport->width, viewport->height);
        RB_LogPrint(v2);
    }
    if (viewport->x < 0)
        MyAssertHandler(".\\r_state.cpp", 1199, 0, "%s\n\t(viewport->x) = %i", "(viewport->x >= 0)", viewport->x);
    if (viewport->y < 0)
        MyAssertHandler(".\\r_state.cpp", 1200, 0, "%s\n\t(viewport->y) = %i", "(viewport->y >= 0)", viewport->y);
    if (viewport->width <= 0)
        MyAssertHandler(".\\r_state.cpp", 1201, 0, "%s\n\t(viewport->width) = %i", "(viewport->width > 0)", viewport->width);
    if (viewport->height <= 0)
        MyAssertHandler(
            ".\\r_state.cpp",
            1202,
            0,
            "%s\n\t(viewport->height) = %i",
            "(viewport->height > 0)",
            viewport->height);
    if (viewport->x != state->viewport.x
        || viewport->y != state->viewport.y
        || viewport->width != state->viewport.width
        || viewport->height != state->viewport.height)
    {
        state->viewport = *viewport;
        R_HW_SetViewport(state->prim.device, viewport, state->depthRangeNear, state->depthRangeFar);
    }
}

void __cdecl R_SetViewportStruct(GfxCmdBufSourceState *source, const GfxViewport *viewport)
{
    iassert(viewport->width > 0);
    iassert(viewport->height > 0);
    iassert(source->viewportBehavior == GFX_USE_VIEWPORT_FOR_VIEW);

    source->sceneViewport = *viewport;
    source->viewMode = VIEW_MODE_NONE;
    source->viewportIsDirty = 1;
}

void __cdecl R_SetViewportValues(GfxCmdBufSourceState *source, int x, int y, int width, int height)
{
    GfxViewport viewport; // [esp+0h] [ebp-10h] BYREF

    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    R_SetViewportStruct(source, &viewport);
}

void __cdecl R_UpdateViewport(GfxCmdBufSourceState *source, GfxViewport *viewport)
{
    float v2; // [esp+10h] [ebp-4Ch]
    float renderTargetWidth; // [esp+18h] [ebp-44h]
    float renderTargetHeight; // [esp+1Ch] [ebp-40h]
    float v5; // [esp+24h] [ebp-38h]
    float v6; // [esp+28h] [ebp-34h]
    float v7; // [esp+2Ch] [ebp-30h]
    float v8; // [esp+30h] [ebp-2Ch]
    float lookupScale; // [esp+34h] [ebp-28h]
    float lookupScale_4; // [esp+38h] [ebp-24h]
    float invWidth; // [esp+3Ch] [ebp-20h]
    float invHeight; // [esp+40h] [ebp-1Ch]
    float lookupOffset; // [esp+54h] [ebp-8h]
    float lookupOffseta; // [esp+54h] [ebp-8h]
    float lookupOffset_4; // [esp+58h] [ebp-4h]
    float lookupOffset_4a; // [esp+58h] [ebp-4h]

    if (!source)
        MyAssertHandler(".\\r_state.cpp", 1252, 0, "%s", "source");
    if (source->viewMode == VIEW_MODE_NONE)
        MyAssertHandler(".\\r_state.cpp", 1253, 0, "%s", "source->viewMode != VIEW_MODE_NONE");
    source->viewportIsDirty = 0;
    if (source->renderTargetWidth <= 0)
        MyAssertHandler(
            ".\\r_state.cpp",
            1256,
            0,
            "%s\n\t(source->renderTargetWidth) = %i",
            "(source->renderTargetWidth > 0)",
            source->renderTargetWidth);
    if (source->renderTargetHeight <= 0)
        MyAssertHandler(
            ".\\r_state.cpp",
            1257,
            0,
            "%s\n\t(source->renderTargetHeight) = %i",
            "(source->renderTargetHeight > 0)",
            source->renderTargetHeight);
    invWidth = 1.0 / (double)source->renderTargetWidth;
    invHeight = 1.0 / (double)source->renderTargetHeight;
    v7 = invWidth * (double)viewport->width;
    v8 = invHeight * (double)viewport->height;
    lookupScale = 0.5 * v7;
    lookupScale_4 = 0.5 * v8;
    v5 = invWidth * (double)viewport->x;
    v6 = invHeight * (double)viewport->y;
    lookupOffset = 0.5 * v7 + v5;
    lookupOffset_4 = 0.5 * v8 + v6;
    lookupOffseta = invWidth * 0.5 + lookupOffset;
    lookupOffset_4a = invHeight * 0.5 + lookupOffset_4;
    renderTargetWidth = (float)source->renderTargetWidth;
    renderTargetHeight = (float)source->renderTargetHeight;
    source->input.consts[10][0] = renderTargetWidth;
    source->input.consts[10][1] = renderTargetHeight;
    source->input.consts[10][2] = invWidth;
    source->input.consts[10][3] = invHeight;
    R_DirtyCodeConstant(source, 0xAu);
    v2 = -lookupScale_4;
    source->input.consts[51][0] = lookupScale;
    source->input.consts[51][1] = v2;
    source->input.consts[51][2] = 0.0;
    source->input.consts[51][3] = 1.0;
    R_DirtyCodeConstant(source, 0x33u);
    source->input.consts[52][0] = lookupOffseta;
    source->input.consts[52][1] = lookupOffset_4a;
    source->input.consts[52][2] = 0.0;
    source->input.consts[52][3] = 0.0;
    R_DirtyCodeConstant(source, 0x34u);
}

void __cdecl R_DisableSampler(GfxCmdBufState *state, unsigned int samplerIndex)
{
    state->samplerTexture[samplerIndex] = 0;
    R_HW_DisableSampler(state->prim.device, samplerIndex);
}

void __cdecl R_HW_DisableSampler(IDirect3DDevice9 *device, unsigned int samplerIndex)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetTexture( samplerIndex, 0 )\n");
        hr = device->SetTexture(
            samplerIndex,
            0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetTexture( samplerIndex, 0 ) failed: %s\n",
                    127,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_UnbindImage(GfxCmdBufState *state, const GfxImage *image)
{
    unsigned int samplerIndex; // [esp+0h] [ebp-4h]

    for (samplerIndex = 0; samplerIndex < vidConfig.maxTextureMaps; ++samplerIndex)
    {
        if (state->samplerTexture[samplerIndex] == &image->texture)
            R_DisableSampler(state, samplerIndex);
    }
}

bool g_renderTargetIsOverridden;
void __cdecl R_SetRenderTargetSize(GfxCmdBufSourceState *source, GfxRenderTargetId newTargetId)
{
    GfxRenderTargetId actualTargetId; // [esp+0h] [ebp-4h]

    iassert(source);
    iassert(gfxRenderTargets);

    g_renderTargetIsOverridden = 0;
    if (pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        actualTargetId = RB_PixelCost_OverrideRenderTarget(newTargetId);
        g_renderTargetIsOverridden = newTargetId != actualTargetId;
        newTargetId = actualTargetId;
    }
    bcassert(newTargetId, R_RENDERTARGET_COUNT);
    source->viewportBehavior = R_ViewportBehaviorForRenderTarget(newTargetId);
    source->renderTargetWidth = gfxRenderTargets[newTargetId].width;
    source->renderTargetHeight = gfxRenderTargets[newTargetId].height;
    iassert(source->renderTargetWidth > 0);
    iassert(source->renderTargetHeight > 0);
}

const GfxViewportBehavior s_viewportBehaviorForRenderTarget[15] =
{
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW
}; // idb

GfxViewportBehavior __cdecl R_ViewportBehaviorForRenderTarget(GfxRenderTargetId renderTargetId)
{
    iassert(s_viewportBehaviorForRenderTarget);
    bcassert(renderTargetId, R_RENDERTARGET_COUNT);

    return s_viewportBehaviorForRenderTarget[renderTargetId];
}

void __cdecl R_SetRenderTarget(GfxCmdBufContext context, GfxRenderTargetId newTargetId)
{
    if (pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC)
        newTargetId = RB_PixelCost_OverrideRenderTarget(newTargetId);

    if (newTargetId != context.state->renderTargetId)
    {
        if (r_logFile->current.integer)
        {
            RB_LogPrint(va("\n========== R_SetRenderTarget( %s ) ==========\n\n", R_RenderTargetName(newTargetId)));
        }
        R_UpdateStatsTarget(newTargetId);

        if (gfxRenderTargets[newTargetId].image)
            R_UnbindImage(context.state, gfxRenderTargets[newTargetId].image);

        iassert(context.source->viewportBehavior == R_ViewportBehaviorForRenderTarget(newTargetId));
        iassert(context.source->renderTargetHeight == (int)gfxRenderTargets[newTargetId].height);
        iassert(context.source->renderTargetWidth > 0);
        iassert(context.source->renderTargetHeight > 0);

        R_HW_SetRenderTarget(context.state, newTargetId);
        context.state->renderTargetId = newTargetId;
        context.source->viewMode = VIEW_MODE_NONE;
        context.source->viewportIsDirty = 1;

        iassert(context.source->renderTargetWidth == (int)gfxRenderTargets[newTargetId].width);
    }
}

void __cdecl R_HW_SetRenderTarget(GfxCmdBufState *state, GfxRenderTargetId newTargetId)
{
    const char *v3; // eax
    int v4; // [esp+0h] [ebp-Ch]
    int hr; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+8h] [ebp-4h]

    device = state->prim.device;
    iassert(device);
    if (gfxRenderTargets[state->renderTargetId].surface.color != gfxRenderTargets[newTargetId].surface.color)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetRenderTarget( 0, gfxRenderTargets[newTargetId].surface.color )\n");
            hr = device->SetRenderTarget(0, gfxRenderTargets[newTargetId].surface.color);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderTarget( 0, gfxRenderTargets[newTargetId].surf"
                        "ace.color ) failed: %s\n",
                        892,
                        R_ErrorDescription(hr));
                } while (alwaysfails);
            }
        } while (alwaysfails);

        state->viewport.x = 0;
        state->viewport.y = 0;
        state->viewport.width = gfxRenderTargets[newTargetId].width;
        state->viewport.height = gfxRenderTargets[newTargetId].height;
        state->depthRangeType = GFX_DEPTH_RANGE_FULL;
        state->depthRangeNear = 0.0;
        state->depthRangeFar = 1.0;
    }
    if (gfxRenderTargets[state->renderTargetId].surface.depthStencil != gfxRenderTargets[newTargetId].surface.depthStencil)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetDepthStencilSurface( gfxRenderTargets[newTargetId].surface.depthStencil )\n");
            if (device->SetDepthStencilSurface(gfxRenderTargets[newTargetId].surface.depthStencil) < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetDepthStencilSurface( gfxRenderTargets[newTargetId]."
                        "surface.depthStencil ) failed: %s\n",
                        905,
                        R_ErrorDescription(v4));
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
}

void __cdecl R_UpdateStatsTarget(int newTargetId)
{
    if (newTargetId == 9 || newTargetId == 6)
        g_viewStats = &g_frameStatsCur.viewStats[1];
    else
        g_viewStats = (GfxViewStats *)&g_frameStatsCur;
}

void __cdecl R_ClearScreenInternal(
    IDirect3DDevice9 *device,
    unsigned __int8 whichToClear,
    const float *color,
    float depth,
    unsigned __int8 stencil,
    const GfxViewport *viewport)
{
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    int hr; // [esp+9Ch] [ebp-8h]
    GfxColor nativeColor; // [esp+A0h] [ebp-4h] BYREF

    if (!device)
        MyAssertHandler(".\\r_state.cpp", 1420, 0, "%s", "device");
    if (r_logFile->current.integer)
    {
        RB_LogPrint("---------- R_ClearScreenInternal\n");
        v6 = va(
            "---------- (%c) color %g %g %g %g\n",
            (whichToClear & 1) != 0 ? 42 : 32,
            *color,
            color[1],
            color[2],
            color[3]);
        RB_LogPrint(v6);
        v7 = va("---------- (%c) depth %g\n", (whichToClear & 2) != 0 ? 42 : 32, depth);
        RB_LogPrint(v7);
        v8 = va("---------- (%c) stencil %i\n", (whichToClear & 4) != 0 ? 42 : 32, stencil);
        RB_LogPrint(v8);
    }
    if (!whichToClear)
        MyAssertHandler(".\\r_state.cpp", 1432, 0, "%s", "whichToClear");
    if (!color)
        MyAssertHandler(".\\r_state.cpp", 1433, 0, "%s", "color");
    if (depth < 0.0 || depth > 1.0)
        MyAssertHandler(".\\r_state.cpp", 1434, 0, "depth not in [0.0f, 1.0f]\n\t%g not in [%g, %g]", depth, 0.0, 1.0);
    Byte4PackVertexColor(color, (unsigned __int8 *)&nativeColor);
    if (viewport)
        MyAssertHandler(".\\r_state.cpp", 1457, 0, "%s", "!viewport");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->Clear( 0, 0, whichToClear, nativeColor.packed, depth, stencil )\n");
        //hr = ((int(__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))device->Clear)(
        //    device,
        //    0,
        //    0,
        //    whichToClear,
        //    (GfxColor)nativeColor.packed,
        //    LODWORD(depth),
        //    stencil);
        hr = device->Clear(0, 0, whichToClear, (GfxColor)nativeColor.packed, LODWORD(depth), stencil);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v9 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_state.cpp (%i) device->Clear( 0, 0, whichToClear, nativeColor.packed, depth, stencil ) failed: %s\n",
                    1458,
                    v9);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_ClearScreen(
    IDirect3DDevice9 *device,
    unsigned __int8 whichToClear,
    const float *color,
    float depth,
    unsigned __int8 stencil,
    const GfxViewport *viewport)
{
    if (!whichToClear)
        MyAssertHandler(".\\r_state.cpp", 1484, 0, "%s", "whichToClear");
    if ((whichToClear & 0xF8) != 0)
        MyAssertHandler(
            ".\\r_state.cpp",
            1485,
            0,
            "%s\n\t(whichToClear) = %i",
            "((whichToClear & ~(0x00000001l | 0x00000002l | 0x00000004l)) == 0)",
            whichToClear);
    if (!color)
        MyAssertHandler(".\\r_state.cpp", 1486, 0, "%s", "color");
    if (depth < 0.0 || depth > 1.0)
        MyAssertHandler(".\\r_state.cpp", 1487, 0, "%s\n\t(depth) = %g", "(depth >= 0.0f && depth <= 1.0f)", depth);
    if (pixelCostMode <= GFX_PIXEL_COST_MODE_MEASURE_MSEC || (whichToClear &= ~1u) != 0)
        R_ClearScreenInternal(device, whichToClear, color, depth, stencil, viewport);
}

void __cdecl R_ForceSetPolygonOffset(IDirect3DDevice9 *device, char stateBits1)
{
    __int64 v2; // [esp+10h] [ebp-24h]
    unsigned int offset; // [esp+28h] [ebp-Ch]
    float bias; // [esp+2Ch] [ebp-8h]
    float scale; // [esp+30h] [ebp-4h]

    offset = stateBits1 & 0x30;
    if (offset == 48)
    {
        bias = sm_polygonOffsetBias->current.value * 0.0000152587890625;
        scale = sm_polygonOffsetScale->current.value;
    }
    else
    {
        v2 = offset >> 4;
        bias = (double)v2 * r_polygonOffsetBias->current.value * 0.0000152587890625;
        scale = (double)v2 * r_polygonOffsetScale->current.value;
    }
    R_HW_SetPolygonOffset(device, scale, bias);
}

void __cdecl R_HW_SetPolygonOffset(IDirect3DDevice9 *device, float scale, float bias)
{
    const char *v3; // eax
    const char *v4; // eax
    int v5; // [esp+8h] [ebp-8h]
    int hr; // [esp+Ch] [ebp-4h]

    if (gfxMetrics.slopeScaleDepthBias)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, FloatAsInt( &scale ) )\n");
            hr = device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, LODWORD(scale));
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v3 = R_ErrorDescription(hr);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, Floa"
                        "tAsInt( &scale ) ) failed: %s\n",
                        468,
                        v3);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    else
    {
        bias = bias + bias;
    }
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_DEPTHBIAS, FloatAsInt( &bias ) )\n");
        v5 = device->SetRenderState(D3DRS_DEPTHBIAS, LODWORD(bias));
        if (v5 < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(v5);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_DEPTHBIAS, FloatAsInt( &bia"
                    "s ) ) failed: %s\n",
                    472,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_SetMeshStream(GfxCmdBufState *state, GfxMeshData *mesh)
{
    R_SetStreamSource(&state->prim, mesh->vb.buffer, 0, mesh->vertSize);
}

void __cdecl R_SetCompleteState(IDirect3DDevice9 *device, unsigned int *stateBits)
{
    iassert(device);
    R_HW_SetColorMask(device, stateBits[0]);
    R_HW_SetAlphaTestEnable(device, stateBits[0]);
    iassert(stateBits[0] & GFXS0_ATEST_DISABLE);
    R_HW_SetCullFace(device, stateBits[0]);
    R_HW_SetPolygonMode(device, stateBits[0]);
    R_ForceSetBlendState(device, stateBits[0]);
    R_HW_SetDepthWriteEnable(device, stateBits[1]);
    R_HW_SetDepthTestEnable(device, stateBits[1]);
    R_HW_SetDepthTestFunction(device, stateBits[1]);
    R_ForceSetPolygonOffset(device, stateBits[1]);
    R_ForceSetStencilState(device, stateBits[1]);
}

void R_DrawCall(
    DrawCallCallback callback,
    const void* userData,
    GfxCmdBufSourceState* source,
    const GfxViewInfo* viewInfo,
    const GfxDrawSurfListInfo* info,
    const GfxViewParms* viewParms,
    GfxCmdBuf* cmdBufEA,
    GfxCmdBuf* prepassCmdBufEA)
{
    GfxCmdBufState cmdBuf;
    GfxCmdBufState prepassCmdBuf;
    GfxCmdBufContext context;
    GfxCmdBufContext prepassContext;

    context.source = source;
    context.state = &cmdBuf;

    R_BeginView(source, &viewInfo->sceneDef, viewParms);
    //R_InitLocalCmdBufState(&cmdBuf);
    memcpy(&cmdBuf, &gfxCmdBufState, sizeof(cmdBuf));
    memset(cmdBuf.vertexShaderConstState, 0, sizeof(cmdBuf.vertexShaderConstState));
    memset(cmdBuf.pixelShaderConstState, 0, sizeof(cmdBuf.pixelShaderConstState));

    if (prepassCmdBufEA)
    {
        prepassContext.source = source;
        prepassContext.state = &prepassCmdBuf;
        //R_InitLocalCmdBufState(&prepassCmdBuf);
        memcpy(&prepassCmdBuf, &gfxCmdBufState, sizeof(prepassCmdBuf));
        memset(prepassCmdBuf.vertexShaderConstState, 0, sizeof(prepassCmdBuf.vertexShaderConstState));
        memset(prepassCmdBuf.pixelShaderConstState, 0, sizeof(prepassCmdBuf.pixelShaderConstState));

        callback(userData, context, prepassContext);
        memcpy(&gfxCmdBufState, &prepassCmdBuf, sizeof(gfxCmdBufState));
    }
    else
    {
        prepassContext.source = NULL;
        prepassContext.state = NULL;
        callback(userData, context, prepassContext);
    }
    memcpy(&gfxCmdBufState, &cmdBuf, sizeof(gfxCmdBufState));
}

void __cdecl R_SetCodeConstant(GfxCmdBufSourceState *source, unsigned int constant, float x, float y, float z, float w)
{
    float *v6; // [esp+0h] [ebp-4h]

    if (constant >= 0x3A)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            495,
            0,
            "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
            constant,
            58);
    v6 = source->input.consts[constant];
    v6[0] = x;
    v6[1] = y;
    v6[2] = z;
    v6[3] = w;
    R_DirtyCodeConstant(source, constant);
}


void __cdecl R_SetAlphaAntiAliasingState(IDirect3DDevice9 *device, __int16 stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+0h] [ebp-Ch]
    _D3DFORMAT aaAlphaFormat; // [esp+4h] [ebp-8h]

    if ((stateBits0 & 0xF00) != 0)
    {
        aaAlphaFormat = D3DFMT_UNKNOWN;
    }
    else if (r_aaAlpha->current.integer == 2)
    {
        aaAlphaFormat = (_D3DFORMAT)1094800211;
    }
    else
    {
        aaAlphaFormat = (_D3DFORMAT)1129272385;
    }
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ADAPTIVETESS_Y, aaAlphaFormat )\n");
        hr = device->SetRenderState(
            D3DRS_ADAPTIVETESS_Y,
            aaAlphaFormat);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_state.cpp (%i) device->SetRenderState( D3DRS_ADAPTIVETESS_Y, aaAlphaFormat ) failed: %s\n",
                    826,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}