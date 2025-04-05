#pragma once
#include <xanim/xanim.h>
#include "rb_backend.h"


void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to);
unsigned int __cdecl R_HashAssetName(const char *name);
unsigned int __cdecl R_HashString(const char *string);
char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name);
char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount);
void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned __int8 *colorBytes);
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
void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to);
unsigned int __cdecl R_HashAssetName(const char *name);
unsigned int __cdecl R_HashString(const char *string);
char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name);
char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount);
void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned __int8 *colorBytes);
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
void __cdecl R_CmdBufSet2D(GfxCmdBufSourceState* source, GfxViewport* viewport);
GfxCmdBufSourceState *__cdecl R_GetActiveWorldMatrix(GfxCmdBufSourceState *source);