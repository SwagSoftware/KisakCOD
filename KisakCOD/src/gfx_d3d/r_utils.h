#pragma once
#include <xanim/xanim.h>
#include "rb_backend.h"

_DWORD s_codeConstUpdateFreq[90] =
{
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  0
}; // weak


void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to);
unsigned int __cdecl R_HashAssetName(const char *name);
unsigned int __cdecl R_HashString(const char *string);
char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name);
char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount);
void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned int *colorBytes);
inline void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned char *colorBytes)
{
    R_ConvertColorToBytes(colorFloat, (unsigned int *)colorBytes);
}
inline void __cdecl R_ConvertColorToBytes(const float *colorFloat, GfxColor *colorBytes)
{
    R_ConvertColorToBytes(colorFloat, (unsigned int *)colorBytes);
}
int __cdecl R_PickMaterial(
    int traceMask,
    const float *org,
    const float *dir,
    char *name,
    char *surfaceFlags,
    char *contents,
    unsigned int charLimit);
double __cdecl FresnelTerm(float n0, float n1, float cosIncidentAngle);
char __cdecl R_GetClearColor(float *unpackedRgba);
void __cdecl Byte4UnpackBgra(const unsigned __int8 *from, float *to);


// r_state_utils
void __cdecl R_BeginView(GfxCmdBufSourceState *source, const GfxSceneDef *sceneDef, const GfxViewParms *viewParms);
void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to);
unsigned int __cdecl R_HashAssetName(const char *name);
unsigned int __cdecl R_HashString(const char *string);
char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name);
char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount);
int __cdecl R_PickMaterial(
    int traceMask,
    const float *org,
    const float *dir,
    char *name,
    char *surfaceFlags,
    char *contents,
    unsigned int charLimit);
double __cdecl FresnelTerm(float n0, float n1, float cosIncidentAngle);
char __cdecl R_GetClearColor(float *unpackedRgba);
void __cdecl Byte4UnpackBgra(const unsigned __int8 *from, float *to);

void __cdecl R_SetShadowLookupMatrix(GfxCmdBufSourceState *source, const GfxMatrix *matrix);
void __cdecl R_Set2D(GfxCmdBufSourceState *source);
void __cdecl R_Set3D(GfxCmdBufSourceState *source);
void __cdecl R_CmdBufSet2D(GfxCmdBufSourceState* source, GfxViewport* viewport);
GfxCmdBufSourceState *__cdecl R_GetActiveWorldMatrix(GfxCmdBufSourceState *source);
void __cdecl R_InitCmdBufState(GfxCmdBufState *state);
void __cdecl R_InitCmdBufSourceState(GfxCmdBufSourceState *source, const GfxCmdBufInput *input, int cameraView);

void __cdecl R_MatrixIdentity44(float (*out)[4]);
double __cdecl R_GetBaseLodDist(const float *origin);
double __cdecl R_GetAdjustedLodDist(float dist, XModelLodRampType lodRampType);

template <typename T>
inline void __cdecl R_ReleaseAndSetNULL(
    IDirect3DSurface9 *var,
    const char *fn,
    const char *filename,
    int line)
{
    const char *v4; // eax
    unsigned int useCount; // [esp+0h] [ebp-4h]

    if (!var)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_types_d3d.h", 198, 0, "%s", "var");
    useCount = var->Release();
    if (useCount)
    {
        v4 = va("%s (%i) %s->Release() failed: %i leak(s)!", filename, line, fn, useCount);
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_types_d3d.h", 209, 0, "%s\n\t%s", "!useCount", v4);
    }
}