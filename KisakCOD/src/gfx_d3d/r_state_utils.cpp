#include "r_utils.h"
#include <universal/com_memory.h>
#include <aim_assist/aim_assist.h>
#include "r_state.h"
#include "r_dvars.h"
#include "r_cmdbuf.h"
#include <universal/surfaceflags.h>
#include "rb_logfile.h"
#include "rb_state.h"

void __cdecl R_DeriveNearPlaneConstantsForView(GfxCmdBufSourceState *source)
{
    float v1; // [esp+8h] [ebp-60h]
    float v2; // [esp+Ch] [ebp-5Ch]
    float v3; // [esp+10h] [ebp-58h]
    float v4; // [esp+14h] [ebp-54h]
    float v5; // [esp+18h] [ebp-50h]
    float v6; // [esp+1Ch] [ebp-4Ch]
    float v7; // [esp+20h] [ebp-48h]
    float v8; // [esp+28h] [ebp-40h]
    float v9; // [esp+2Ch] [ebp-3Ch]
    float v10; // [esp+30h] [ebp-38h]
    float v11; // [esp+38h] [ebp-30h]
    float v12; // [esp+3Ch] [ebp-2Ch]
    float v13; // [esp+40h] [ebp-28h]
    float scale; // [esp+64h] [ebp-4h]
    float scalea; // [esp+64h] [ebp-4h]

    v4 = fabs(source->viewParms.inverseViewProjectionMatrix.m[0][3]);
    v3 = source->viewParms.inverseViewProjectionMatrix.m[3][3] * 0.000009999999747378752;
    if (v4 >= (double)v3)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            286,
            0,
            "%s\n\t(mtx->m[0][3]) = %g",
            "(I_fabs( mtx->m[0][3] ) < 1.0e-5f * mtx->m[3][3])",
            source->viewParms.inverseViewProjectionMatrix.m[0][3]);
    v2 = fabs(source->viewParms.inverseViewProjectionMatrix.m[1][3]);
    v1 = source->viewParms.inverseViewProjectionMatrix.m[3][3] * 0.000009999999747378752;
    if (v2 >= (double)v1)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            287,
            0,
            "%s\n\t(mtx->m[1][3]) = %g",
            "(I_fabs( mtx->m[1][3] ) < 1.0e-5f * mtx->m[3][3])",
            source->viewParms.inverseViewProjectionMatrix.m[1][3]);
    if (source->viewParms.inverseViewProjectionMatrix.m[3][3] == 0.0)
        MyAssertHandler(".\\r_state_utils.cpp", 288, 0, "%s", "mtx->m[3][3] != 0");
    scale = 1.0 / source->viewParms.inverseViewProjectionMatrix.m[3][3];
    v11 = source->viewParms.inverseViewProjectionMatrix.m[3][0] * scale - source->viewParms.origin[0];
    v12 = source->viewParms.inverseViewProjectionMatrix.m[3][1] * scale - source->viewParms.origin[1];
    v13 = source->viewParms.inverseViewProjectionMatrix.m[3][2] * scale - source->viewParms.origin[2];
    source->input.consts[5][0] = v11;
    source->input.consts[5][1] = v12;
    source->input.consts[5][2] = v13;
    source->input.consts[5][3] = 0.0;
    R_DirtyCodeConstant(source, 5u);
    scalea = scale + scale;
    v8 = source->viewParms.inverseViewProjectionMatrix.m[0][0] * scalea;
    v9 = source->viewParms.inverseViewProjectionMatrix.m[0][1] * scalea;
    v10 = source->viewParms.inverseViewProjectionMatrix.m[0][2] * scalea;
    source->input.consts[6][0] = v8;
    source->input.consts[6][1] = v9;
    source->input.consts[6][2] = v10;
    source->input.consts[6][3] = 0.0;
    R_DirtyCodeConstant(source, 6u);
    v5 = source->viewParms.inverseViewProjectionMatrix.m[1][0] * -scalea;
    v6 = source->viewParms.inverseViewProjectionMatrix.m[1][1] * -scalea;
    v7 = source->viewParms.inverseViewProjectionMatrix.m[1][2] * -scalea;
    source->input.consts[7][0] = v5;
    source->input.consts[7][1] = v6;
    source->input.consts[7][2] = v7;
    source->input.consts[7][3] = 0.0;
    R_DirtyCodeConstant(source, 7u);
}

void __cdecl R_SetGameTime(GfxCmdBufSourceState *source, float gameTime)
{
    float v2; // [esp+8h] [ebp-30h]
    float v3; // [esp+28h] [ebp-10h]
    float cosOfFracPartOfGameTime; // [esp+2Ch] [ebp-Ch]
    float sinOfFracPartOfGameTime; // [esp+30h] [ebp-8h]
    float fracPartOfGameTime; // [esp+34h] [ebp-4h]

    v2 = floor(gameTime);
    fracPartOfGameTime = gameTime - v2;
    v3 = fracPartOfGameTime * 6.283185482025146;
    cosOfFracPartOfGameTime = cos(v3);
    sinOfFracPartOfGameTime = sin(v3);
    source->input.consts[18][0] = sinOfFracPartOfGameTime;
    source->input.consts[18][1] = cosOfFracPartOfGameTime;
    source->input.consts[18][2] = fracPartOfGameTime;
    source->input.consts[18][3] = gameTime;
    R_DirtyCodeConstant(source, 0x12u);
}

void __cdecl R_BeginView(GfxCmdBufSourceState *source, const GfxSceneDef *sceneDef, const GfxViewParms *viewParms)
{
    source->sceneDef = *sceneDef;
    source->viewParms3D = viewParms;
    source->skinnedPlacement.base.origin[0] = sceneDef->viewOffset[0];
    source->skinnedPlacement.base.origin[1] = sceneDef->viewOffset[1];
    source->skinnedPlacement.base.origin[2] = sceneDef->viewOffset[2];
    source->viewMode = VIEW_MODE_NONE;
    R_Set3D(source);
    source->materialTime = 0.0;
    R_SetGameTime(source, source->sceneDef.floatTime);
    R_DeriveNearPlaneConstantsForView(source);
}

int __cdecl R_PickMaterial(
    int traceMask,
    const float *org,
    const float *dir,
    char *name,
    char *surfaceFlags,
    char *contents,
    unsigned int charLimit)
{
    int v8; // ecx
    char v9; // [esp+4Fh] [ebp-55h]
    const char *material; // [esp+58h] [ebp-4Ch]
    float end[3]; // [esp+5Ch] [ebp-48h] BYREF
    trace_t trace; // [esp+68h] [ebp-3Ch] BYREF
    int contentsLen; // [esp+94h] [ebp-10h]
    int index; // [esp+98h] [ebp-Ch]
    int surfaceFlagsLen; // [esp+9Ch] [ebp-8h]
    int i; // [esp+A0h] [ebp-4h]

    Vec3Mad(org, 262144.0, dir, end);
    CM_BoxTrace(&trace, org, end, vec3_origin, vec3_origin, 0, traceMask);
    if (trace.startsolid || trace.allsolid || trace.fraction == 1.0 || !trace.material)
        return 0;
    material = trace.material;
    do
    {
        v9 = *material;
        *name++ = *material++;
    } while (v9);
    *surfaceFlags = 0;
    surfaceFlags[charLimit - 1] = 0;
    surfaceFlagsLen = 0;
    *contents = 0;
    contents[charLimit - 1] = 0;
    contentsLen = 0;
    v8 = (trace.surfaceFlags & 0x1F00000) >> 20;
    index = (unsigned __int8)v8;
    if ((_BYTE)v8 && index < 29)
        strncpy((unsigned __int8 *)surfaceFlags, (unsigned __int8 *)infoParms[index - 1].name, charLimit);
    else
        strncpy((unsigned __int8 *)surfaceFlags, (unsigned __int8 *)"^1default^7", charLimit);
    if (surfaceFlags[charLimit - 1])
        return 0;
    surfaceFlagsLen = strlen(surfaceFlags);
    if ((trace.contents & 1) != 0)
        strncpy((unsigned __int8 *)contents, (unsigned __int8 *)"solid", charLimit);
    else
        strncpy((unsigned __int8 *)contents, (unsigned __int8 *)"^3nonsolid^7", charLimit);
    if (contents[charLimit - 1])
        return 0;
    contentsLen = strlen(contents);
    for (i = 28; infoParms[i].name; ++i)
    {
        if ((trace.surfaceFlags & infoParms[i].surfaceFlags) != 0)
        {
            surfaceFlags[surfaceFlagsLen++] = 32;
            strncpy(
                (unsigned __int8 *)&surfaceFlags[surfaceFlagsLen],
                (unsigned __int8 *)infoParms[i].name,
                charLimit - surfaceFlagsLen);
            if (surfaceFlags[charLimit - 1])
                return 0;
            surfaceFlagsLen += strlen(&surfaceFlags[surfaceFlagsLen]);
        }
        if ((trace.contents & infoParms[i].contents) != 0)
        {
            contents[contentsLen++] = 32;
            strncpy((unsigned __int8 *)&contents[contentsLen], (unsigned __int8 *)infoParms[i].name, charLimit - contentsLen);
            if (contents[charLimit - 1])
                return 0;
            contentsLen += strlen(&contents[contentsLen]);
        }
    }
    return 1;
}

void __cdecl R_SetShadowLookupMatrix(GfxCmdBufSourceState *source, const GfxMatrix *matrix)
{
    ++source->matrixVersions[6];
    memcpy(&source->shadowLookupMatrix, matrix, sizeof(source->shadowLookupMatrix));
}

void __cdecl R_Set2D(GfxCmdBufSourceState *source)
{
    if (source->viewMode != VIEW_MODE_2D)
    {
        source->viewMode = VIEW_MODE_2D;
        source->viewportIsDirty = 1;
        source->eyeOffset[0] = 0.0;
        source->eyeOffset[1] = 0.0;
        source->eyeOffset[2] = 0.0;
        source->eyeOffset[3] = 1.0;

        GfxViewport viewport;
        R_GetViewport(source, &viewport);
        R_CmdBufSet2D(source, &viewport);
    }
}

void __cdecl R_CmdBufSet2D(GfxCmdBufSourceState* source, GfxViewport* viewport)
{
    GfxViewParms* v2; // ebp
    float v3[16]; // [esp-8h] [ebp-9Ch] BYREF
    GfxMatrix identity_52; // [esp+38h] [ebp-5Ch] BYREF
    GfxViewParms* transform_56; // [esp+7Ch] [ebp-18h]
    float transform_60; // [esp+80h] [ebp-14h]
    float v7; // [esp+84h] [ebp-10h]
    GfxViewParms* viewParms; // [esp+88h] [ebp-Ch]
    float invHeight; // [esp+8Ch] [ebp-8h]
    float retaddr; // [esp+94h] [ebp+0h]

    //viewParms = v2;
    //invHeight = retaddr;
    if (viewport->width <= 0)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            168,
            0,
            "%s\n\t(viewport->width) = %i",
            "(viewport->width > 0)",
            viewport->width);
    if (viewport->height <= 0)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            169,
            0,
            "%s\n\t(viewport->height) = %i",
            "(viewport->height > 0)",
            viewport->height);
    v7 = 1.0 / (double)viewport->width;
    transform_60 = 1.0 / (double)viewport->height;
    transform_56 = &source->viewParms;
    memset((unsigned __int8*)&identity_52, 0, sizeof(identity_52));
    identity_52.m[0][0] = v7 * 2.0;
    identity_52.m[1][1] = transform_60 * -2.0;
    identity_52.m[3][0] = -1.0 - v7;
    identity_52.m[3][1] = transform_60 + 1.0;
    identity_52.m[3][2] = 1.0;
    identity_52.m[3][3] = 1.0;
    R_MatrixIdentity44((float (*)[4])v3);
    R_MatrixIdentity44(transform_56->viewMatrix.m);
    memcpy(&transform_56->projectionMatrix, &identity_52, sizeof(transform_56->projectionMatrix));
    memcpy(&transform_56->viewProjectionMatrix, &identity_52, sizeof(transform_56->viewProjectionMatrix));
    memset(
        (unsigned __int8*)&transform_56->inverseViewProjectionMatrix,
        0,
        sizeof(transform_56->inverseViewProjectionMatrix));
    ++source->matrixVersions[1];
    ++source->matrixVersions[2];
    ++source->matrixVersions[4];
    memcpy(R_GetActiveWorldMatrix(source), v3, 0x40u);
}

void __cdecl R_WorldMatrixChanged(GfxCmdBufSourceState *source)
{
    ++source->matrixVersions[0];
    ++source->matrixVersions[3];
    ++source->matrixVersions[5];
    ++source->matrixVersions[7];
    source->constVersions[58] = source->matrixVersions[0];
}

GfxCmdBufSourceState *__cdecl R_GetActiveWorldMatrix(GfxCmdBufSourceState *source)
{
    R_WorldMatrixChanged(source);
    return source;
}

double __cdecl R_GetAdjustedLodDist(float dist, XModelLodRampType lodRampType)
{
    iassert(rg.lodParms.valid);
    return (dist * rg.lodParms.ramp[lodRampType].scale + rg.lodParms.ramp[lodRampType].bias);
}

double __cdecl R_GetBaseLodDist(const float* origin)
{
    float diff[3]; // [esp+4h] [ebp-Ch] BYREF

    Vec3Sub(rg.lodParms.origin, origin, diff);
    return Vec3Length(diff);
}

const float g_identityMatrix44[4][4] =
{
  { 1.0, 0.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 }
}; // idb
void __cdecl R_MatrixIdentity44(float (*out)[4])
{
    if (!out)
        MyAssertHandler(".\\r_state_utils.cpp", 18, 0, "%s", "out");
    qmemcpy(out, g_identityMatrix44, 0x40u);
}

void __cdecl R_Set3D(GfxCmdBufSourceState *source)
{
    if (!source->viewParms3D)
        MyAssertHandler(".\\r_state_utils.cpp", 240, 0, "%s", "source->viewParms3D");
    if (source->viewMode != VIEW_MODE_3D)
    {
        source->viewMode = VIEW_MODE_3D;
        qmemcpy(&source->viewParms, source->viewParms3D, sizeof(source->viewParms));
        if (source->viewParms.origin[3] == 0.0)
        {
            source->eyeOffset[0] = 0.0;
            source->eyeOffset[1] = 0.0;
            source->eyeOffset[2] = 0.0;
        }
        else
        {
            source->eyeOffset[0] = source->viewParms.origin[0];
            source->eyeOffset[1] = source->viewParms.origin[1];
            source->eyeOffset[2] = source->viewParms.origin[2];
        }
        source->eyeOffset[3] = 1.0;
        R_CmdBufSet3D(source);
    }
}

void __cdecl R_InitCmdBufSourceState(GfxCmdBufSourceState *source, const GfxCmdBufInput *input, int cameraView)
{
    float *v3; // [esp+8h] [ebp-8h]

    if (!source)
        MyAssertHandler(".\\r_state_utils.cpp", 27, 0, "%s", "source");
    if (!input)
        MyAssertHandler(".\\r_state_utils.cpp", 28, 0, "%s", "input");
    memset((unsigned __int8 *)source, 0, sizeof(GfxCmdBufSourceState));
    qmemcpy(&source->input, input, sizeof(source->input));
    for (unsigned int constant = 0; constant < 0x3A; ++constant)
    {
        if (input == &gfxCmdBufInput || s_codeConstUpdateFreq[constant] != 2 || constant < 0x20)
        {
            v3 = source->input.consts[constant];
            v3[0] = FLT_MAX;
            v3[1] = FLT_MAX;
            v3[2] = FLT_MAX;
            v3[3] = 0.0;
        }
        else
        {
            source->constVersions[constant] = 1;
        }
    }
    if (source->shadowableLightForShadowLookupMatrix)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            57,
            1,
            "%s",
            "source->shadowableLightForShadowLookupMatrix == PRIMARY_LIGHT_NONE");
    source->skinnedPlacement.base.quat[3] = 1.0;
    source->skinnedPlacement.scale = 1.0;
    source->cameraView = cameraView;
}

void __cdecl R_SetDefaultAlphaTestFunction(GfxCmdBufState *state)
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+4h] [ebp-4h]

    device = state->prim.device;
    if (!device)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h", 727, 0, "%s", "device");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderState( D3DRS_ALPHAREF, 0 )\n");
        hr = device->SetRenderState(D3DRS_ALPHAREF, 0);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderState( D3DRS_ALPHAREF, 0 ) failed: %s\n",
                    729,
                    v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    state->alphaRef = 0;
}

void __cdecl R_SetDefaultStateBits(unsigned int *stateBits)
{
    *stateBits = 0;
    stateBits[1] = 0;
    *stateBits |= 0x800u;
    *stateBits |= 0x18000000u;
    *stateBits |= 0x4000u;
    *stateBits = *stateBits;
    *stateBits = *stateBits;
    stateBits[1] |= 2u;
    stateBits[1] = stateBits[1];
}

void __cdecl R_HW_ForceSamplerState(IDirect3DDevice9 *device, unsigned int samplerIndex, unsigned int samplerState)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v11; // [esp+0h] [ebp-34h]
    int v12; // [esp+4h] [ebp-30h]
    int v13; // [esp+8h] [ebp-2Ch]
    int v14; // [esp+Ch] [ebp-28h]
    int v15; // [esp+10h] [ebp-24h]
    int v16; // [esp+14h] [ebp-20h]
    int v17; // [esp+18h] [ebp-1Ch]
    int hr; // [esp+1Ch] [ebp-18h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MINFILTER, minFilter )\n");
        hr = device->SetSamplerState(samplerIndex, D3DSAMP_MINFILTER, (unsigned __int16)(samplerState & 0xF00) >> 8);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v3 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MINFILTER,"
                    " minFilter ) failed: %s\n",
                    204,
                    v3);
            } while (alwaysfails);
        }
    } while (alwaysfails);
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
                v4 = R_ErrorDescription(v17);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MAGFILTER,"
                    " magFilter ) failed: %s\n",
                    207,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    if ((unsigned __int8)samplerState > 1u)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAXANISOTROPY, anisotropy )\n");
            v16 = device->SetSamplerState(samplerIndex, D3DSAMP_MAXANISOTROPY, (unsigned __int8)samplerState);
            if (v16 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v5 = R_ErrorDescription(v16);
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MAXANISO"
                        "TROPY, anisotropy ) failed: %s\n",
                        211,
                        v5);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
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
                v6 = R_ErrorDescription(v15);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MIPFILTER,"
                    " mipFilter ) failed: %s\n",
                    214,
                    v6);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MIPMAPLODBIAS, dx.mipBias )\n");
        v14 = device->SetSamplerState(samplerIndex, D3DSAMP_MIPMAPLODBIAS, dx.mipBias);
        if (v14 < 0)
        {
            do
            {
                ++g_disableRendering;
                v7 = R_ErrorDescription(v14);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_MIPMAPLODB"
                    "IAS, dx.mipBias ) failed: %s\n",
                    215,
                    v7);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSU, address )\n");
        v13 = device->SetSamplerState(
            samplerIndex,
            D3DSAMP_ADDRESSU,
            (samplerState & 0x300000) >> 20);
        if (v13 < 0)
        {
            do
            {
                ++g_disableRendering;
                v8 = R_ErrorDescription(v13);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSU, "
                    "address ) failed: %s\n",
                    218,
                    v8);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSV, address )\n");
        v12 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, (samplerState & 0xC00000) >> 22);
        if (v12 < 0)
        {
            do
            {
                ++g_disableRendering;
                v9 = R_ErrorDescription(v12);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSV, "
                    "address ) failed: %s\n",
                    221,
                    v9);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSW, address )\n");
        v11 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSW, (samplerState & 0x3000000) >> 24);
        if (v11 < 0)
        {
            do
            {
                ++g_disableRendering;
                v10 = R_ErrorDescription(v11);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSW, "
                    "address ) failed: %s\n",
                    224,
                    v10);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl R_InitCmdBufState(GfxCmdBufState *state)
{
    unsigned int samplerIndex; // [esp+10h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-8h]
    unsigned int forceSamplerState; // [esp+18h] [ebp-4h]

    if (!state)
        MyAssertHandler(".\\r_state_utils.cpp", 96, 0, "%s", "state");
    device = state->prim.device;
    memset(state->refSamplerState, 0, sizeof(GfxCmdBufState));
    state->prim.device = device;
    state->depthRangeType = GFX_DEPTH_RANGE_FULL;
    state->depthRangeNear = 0.0;
    state->depthRangeFar = 1.0;
    state->renderTargetId = R_RENDERTARGET_NONE;
    R_SetDefaultStateBits(state->activeStateBits);
    R_SetDefaultStateBits(state->refStateBits);
    R_SetCompleteState(device, state->activeStateBits);
    R_SetDefaultAlphaTestFunction(state);
    forceSamplerState = R_DecodeSamplerState(1u);
    for (samplerIndex = 0; samplerIndex < 0x10; ++samplerIndex)
    {
        R_HW_ForceSamplerState(state->prim.device, samplerIndex, forceSamplerState);
        state->refSamplerState[samplerIndex] = 1;
        state->samplerState[samplerIndex] = forceSamplerState;
    }
    state->prim.vertexDecl = (IDirect3DVertexDeclaration9 *)-1;
    state->pixelShader = (const MaterialPixelShader *)-1;
}