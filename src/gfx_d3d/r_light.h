#pragma once
#include "r_gfx.h"
#include <xanim/dobj.h>
#include "r_rendercmds.h"

#define MAX_VISIBLE_SHADOWABLE_DLIGHTS 1

struct AnnotatedLightGridPoint // sizeof=0xA
{                                       // ...
    unsigned __int16 pos[3];            // ...
    GfxLightGridEntry entry;            // ...
};

struct GfxLightGridEntry_Version15 // sizeof=0x8
{
    unsigned int xyzHighBits;
    unsigned __int8 xyzLowBitsAndPrimaryVis;
    unsigned __int8 needsTrace;
    unsigned __int16 colorsIndex;
};

struct DiskGfxCell_Version14 // sizeof=0x34
{
    float mins[3];
    float maxs[3];
    int aabbTreeIndex;
    int firstPortal;
    int portalCount;
    int firstCullGroup;
    int cullGroupCount;
    int unused0;
    int unused1;
};

struct DiskGfxCell // sizeof=0x70
{
    float mins[3];
    float maxs[3];
    unsigned __int16 aabbTreeIndex[2];
    int firstPortal;
    int portalCount;
    int firstCullGroup;
    int cullGroupCount;
    unsigned __int8 reflectionProbeCount;
    unsigned __int8 reflectionProbes[64];
    // padding byte
    // padding byte
    // padding byte
};

struct LightGlobals // sizeof=0x104
{                                       // ...
    int defCount;                       // ...
    GfxLightDef *defs[64];              // ...
};

struct BspOmniLightCallback // sizeof=0x14
{                                       // ...
    const unsigned __int8 *surfaceVisData; // ...
    float position[3];                  // ...
    float radiusSq;                     // ...
};
struct BspSpotLightCallback // sizeof=0x64
{                                       // ...
    const unsigned __int8 *surfaceVisData; // ...
    float planes[6][4];                 // ...
};

GfxLightDef *__cdecl R_RegisterLightDef(const char *name);
GfxLightDef *__cdecl R_RegisterLightDef_FastFile(const char *name);
void __cdecl R_EnumLightDefs(void(__cdecl *func)(GfxLightDef *, void *), void *data);
void __cdecl R_InitLightDefs();
void __cdecl R_ShutdownLightDefs();
int __cdecl R_GetPointLightPartitions(const GfxLight **visibleLights);
void __cdecl R_MostImportantLights(const GfxLight **lights, int lightCount, int keepCount);
bool __cdecl R_LightImportanceGreaterEqual(const GfxLight *light0, const GfxLight *light1);
void __cdecl R_GetBspLightSurfs(const GfxLight **visibleLights, int visibleCount);
void __cdecl R_GetBspOmniLightSurfs(const GfxLight *light, int lightIndex, GfxBspDrawSurfData *surfData);
int __cdecl R_AllowBspOmniLight(int surfIndex, void *bspLightCallbackAsVoid);
void __cdecl R_GetBspSpotLightSurfs(const GfxLight *light, int lightIndex, GfxBspDrawSurfData *surfData);
int __cdecl R_AllowBspSpotLightShadows(int surfIndex, void *bspLightCallbackAsVoid);
int __cdecl R_BoxInPlanes(const float (*planes)[4], const float *mins, const float *maxs);
int __cdecl R_AllowBspSpotLight(int surfIndex, void *bspLightCallbackAsVoid);
void __cdecl R_CalcSpotLightPlanes(const GfxLight *light, float (*planes)[4]);
void __cdecl R_CalcPlaneFromPointDir(float *plane, const float *origin, const float *dir);
void __cdecl R_ComputeSpotLightCrossDirs(const GfxLight *light, float (*crossDirs)[3]);
void __cdecl R_CalcPlaneFromCosSinPointDirs(
    float *plane,
    float fCos,
    float fSin,
    const float *origin,
    const float *forward,
    const float *lateral);
void __cdecl R_GetStaticModelLightSurfs(const GfxLight **visibleLights, int visibleCount);
int __cdecl R_AllowStaticModelOmniLight(int smodelIndex);
int __cdecl R_AllowStaticModelSpotLight(int smodelIndex);
void __cdecl R_GetSceneEntLightSurfs(const GfxLight **visibleLights, int visibleCount);
int __cdecl R_SphereInPlanes(const float (*planes)[4], const float *center, float radius);
bool __cdecl R_SpotLightIsAttachedToDobj(const DObj_s *obj);
int __cdecl R_EmitPointLightPartitionSurfs(
    GfxViewInfo *viewInfo,
    const GfxLight **visibleLights,
    int visibleCount,
    const float *viewOrigin);
int __cdecl R_GetTechniqueForLightType(const GfxLight *light, const GfxViewInfo *viewInfo);
void __cdecl R_EmitShadowedLightPartitionSurfs(
    GfxViewInfo *viewInfo,
    unsigned int lightDrawSurfCount,
    GfxDrawSurf *lightDrawSurfs,
    GfxDrawSurfListInfo *info);



// r_light_load_obj
void __cdecl R_LoadLightGridColors(unsigned int bspVersion);
void R_LoadLightGridRowData();
unsigned __int8 *R_LoadLightGridEntries();
void R_LoadLightGridHeader();
void __cdecl R_LoadLightGridPoints_Version15(unsigned int bspVersion);
GfxLightDef *__cdecl R_LoadLightDef(const char *name);