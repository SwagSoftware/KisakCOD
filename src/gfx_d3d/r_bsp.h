#pragma once
#include "r_gfx.h"
#include "r_sky.h"
#include "r_primarylights.h"

#include <universal/q_shared.h>

enum TrisType : __int32
{                                       // ...
    TRIS_TYPE_LAYERED = 0x0,
    TRIS_TYPE_SIMPLE = 0x1,
    TRIS_TYPE_COUNT = 0x2,
};

struct DiskGfxAabbTree // sizeof=0xC
{
    unsigned int firstSurface;
    unsigned int surfaceCount;
    unsigned int childCount;
};

struct DiskGfxCullGroup // sizeof=0x20
{
    float mins[3];
    float maxs[3];
    unsigned int firstSurface;
    unsigned int surfaceCount;
};

struct DiskGfxVertex // sizeof=0x44
{
    float xyz[3];
    float normal[3];
    unsigned __int8 color[4];
    float texCoord[2];
    float lmapCoord[2];
    float tangent[3];
    float binormal[3];
};

struct DiskLeaf // sizeof=0x18
{
    int cluster;
    int firstCollAabbIndex;
    int collAabbCount;
    int firstLeafBrush;
    int numLeafBrushes;
    int cellNum;
};

struct mnode_load_t // sizeof=0x10
{
    int cellIndex;
    int planeIndex;
    unsigned int children[2];
};
struct r_lightmapGroup_t // sizeof=0x8
{                                       // ...
    int wideCount;                      // ...
    int highCount;                      // ...
};
struct LightDefCopyConfig // sizeof=0x8
{                                       // ...
    unsigned __int8 *dest;              // ...
    unsigned int zoom;                  // ...
};
struct DiskLightRegion // sizeof=0x1
{
    unsigned __int8 hullCount;
};
struct r_lightmapMerge_t // sizeof=0x14
{                                       // ...
    unsigned __int8 index;
    // padding byte
    // padding byte
    // padding byte
    float shift[2];
    float scale[2];
};

struct DiskTriangleSoup // sizeof=0x18
{
    unsigned __int16 materialIndex;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 primaryLightIndex;
    bool castsSunShadow;
    unsigned __int8 unused[2];
    int vertexLayerData;
    unsigned int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 indexCount;
    int firstIndex;
};

struct DiskTriangleSoup_Version8 // sizeof=0x10
{
    unsigned __int16 materialIndex;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 indexCount;
    int firstIndex;
};

struct DiskTriangleSoup_Version12 // sizeof=0x14
{
    unsigned __int16 materialIndex;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    int vertexLayerData;
    int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 indexCount;
    int firstIndex;
};


struct GfxBspLoad // sizeof=0x2A8
{                                       // ...
    unsigned int bspVersion;            // ...
    TrisType trisType;                  // ...
    const struct dmaterial_t *diskMaterials;   // ...
    unsigned int materialCount;
    float outdoorMins[3];               // ...
    float outdoorMaxs[3];               // ...
    r_lightmapMerge_t lmapMergeInfo[32];
};

struct r_globals_load_t // sizeof=0x2C8
{                                       // ...
    int *cullGroupIndices;              // ...
    float (*portalVerts)[3];            // ...
    GfxAabbTree *aabbTrees;             // ...
    int aabbTreeCount;                  // ...
    int nodeCount;                      // ...
    mnode_load_t *nodes;                // ...
    int reflectionProbesLoaded;         // ...
    int staticModelReflectionProbesLoaded; // ...
    GfxBspLoad load;                    // ...
};

struct GfxWorld // sizeof=0x2DC
{                                       // ...
    const char *name;                   // ...
    const char *baseName;               // ...
    int planeCount;                     // ...
    int nodeCount;                      // ...
    int indexCount;                     // ...
    unsigned __int16 *indices;          // ...
    int surfaceCount;                   // ...
    GfxWorldStreamInfo streamInfo;
    // padding byte
    // padding byte
    // padding byte
    int skySurfCount;                   // ...
    int *skyStartSurfs;                 // ...
    GfxImage *skyImage;                 // ...
    unsigned __int8 skySamplerState;    // ...
    // padding byte
    // padding byte
    // padding byte
    unsigned int vertexCount;           // ...
    GfxWorldVertexData vd;              // ...
    unsigned int vertexLayerDataSize;   // ...
    GfxWorldVertexLayerData vld;        // ...
    SunLightParseParams sunParse;       // ...
    GfxLight *sunLight;                 // ...
    float sunColorFromBsp[3];
    unsigned int sunPrimaryLightIndex;  // ...
    unsigned int primaryLightCount;     // ...
    int cullGroupCount;                 // ...
    unsigned int reflectionProbeCount;  // ...
    GfxReflectionProbe *reflectionProbes; // ...
    GfxTexture *reflectionProbeTextures; // ...
    GfxWorldDpvsPlanes dpvsPlanes;      // ...
    int cellBitsCount;                  // ...
    GfxCell *cells;                     // ...
    int lightmapCount;                  // ...
    GfxLightmapArray *lightmaps;        // ...
    GfxLightGrid lightGrid;             // ...
    GfxTexture *lightmapPrimaryTextures; // ...
    GfxTexture *lightmapSecondaryTextures; // ...
    int modelCount;                     // ...
    GfxBrushModel *models;              // ...
    float mins[3];                      // ...
    float maxs[3];                      // ...
    unsigned int checksum;
    int materialMemoryCount;            // ...
    struct MaterialMemory *materialMemory;     // ...
    sunflare_t sun;                     // ...
    float outdoorLookupMatrix[4][4];
    GfxImage *outdoorImage;
    unsigned int *cellCasterBits;       // ...
    struct GfxSceneDynModel *sceneDynModel;    // ...
    struct GfxSceneDynBrush *sceneDynBrush;    // ...
    unsigned int *primaryLightEntityShadowVis; // ...
    unsigned int *primaryLightDynEntShadowVis[2]; // ...
    unsigned __int8 *nonSunPrimaryLightForModelDynEnt; // ...
    struct GfxShadowGeometry *shadowGeom;      // ...
    struct GfxLightRegion *lightRegion;        // ...
    GfxWorldDpvsStatic dpvs;            // ...
    GfxWorldDpvsDynamic dpvsDyn;        // ...
};
static_assert(sizeof(GfxWorld) == 0x2DC);

// r_bsp
void __cdecl R_ReloadWorld();
void __cdecl R_ReleaseWorld();
void __cdecl R_ShutdownWorld();
void __cdecl R_InterpretSunLightParseParams(SunLightParseParams *sunParse);
void __cdecl R_UpdateLightsFromDvars();
void __cdecl R_CopyParseParamsFromDvars(SunLightParseParams *sunParse);
void __cdecl R_GetNormalizedColorFromDvar(const dvar_s *dvar, float *outVec);
void __cdecl R_LoadWorld(char *name, int *checksum, int savegame);
void __cdecl R_CopyParseParamsToDvars(const SunLightParseParams *sunParse, int savegame);
void R_InitDynamicData();
void __cdecl R_SetWorldPtr_FastFile(const char *name);
void __cdecl R_SetWorldPtr_LoadObj(const char *name);
void R_SetSunLightOverride(float *sunColor);
void R_ResetSunLightOverride();
void R_SetSunDirectionOverride(float *sunDir);
void R_LerpSunDirectionOverride(float *sunDirBegin, float *sunDirEnd, int lerpBeginTime, int lerpEndTime);
void R_ResetSunDirectionOverride();
void R_ResetSunLightParseParams();

unsigned int R_GetDebugReflectionProbeLocs(float (*locArray)[3], unsigned int maxCount);

extern GfxWorld s_world;
extern r_globals_load_t rgl;

// r_bsp_load_obj
void __cdecl R_ModernizeLegacyLightGridColors(const unsigned __int8 *legacyColors, GfxLightGridColors *modernColors);
GfxWorld *__cdecl R_LoadWorldInternal(const char *name);
void __cdecl R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *sunParse, GfxLight *sunLight);
void __cdecl R_SetUpSunLight(const float *sunColor, const float *sunDirection, GfxLight *light);
void __cdecl R_InitPrimaryLights(GfxLight *primaryLights);
void __cdecl R_AddShadowSurfaceToPrimaryLight(
    GfxWorld *world,
    unsigned int primaryLightIndex,
    unsigned int sortedSurfIndex);
void __cdecl R_ForEachPrimaryLightAffectingSurface(
    GfxWorld *world,
    const GfxSurface *surface,
    unsigned int sortedSurfIndex,
    void(__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int));
void __cdecl R_GetXModelBounds(XModel *model, const float (*axes)[3], float *mins, float *maxs);



// r_add_bsp
struct GfxSModelDrawSurfLightingData // sizeof=0x28
{                                       // ...
    GfxDelayedCmdBuf delayedCmdBuf;
    GfxDrawSurfList drawSurf[3];        // ...
};
void __cdecl R_InitBspDrawSurf(GfxBspDrawSurfData *surfData);
void __cdecl R_AddBspDrawSurfs(
    GfxDrawSurf drawSurf,
    unsigned __int8 *list,
    unsigned int count,
    GfxBspDrawSurfData *surfData);
void __cdecl R_AddAllBspDrawSurfacesCamera();
void __cdecl R_AddAllBspDrawSurfacesRangeCamera(
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int stage,
    unsigned int maxDrawSurfCount);
void __cdecl R_AddAllBspDrawSurfacesCameraNonlit(
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int stage);
void __cdecl R_AddAllBspDrawSurfacesSunShadow();
void __cdecl R_AddAllBspDrawSurfacesRangeSunShadow(
    unsigned int partitionIndex,
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int maxDrawSurfCount);
void __cdecl R_AddAllBspDrawSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex);


// r_add_cmdbuf
void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
void __cdecl R_EndCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
int __cdecl R_AllocDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    GfxDrawSurf drawSurf,
    GfxDrawSurfList *drawSurfList,
    unsigned int size);
void __cdecl R_WritePrimDrawSurfInt(GfxDelayedCmdBuf *delayedCmdBuf, unsigned int value);
void __cdecl R_WritePrimDrawSurfData(GfxDelayedCmdBuf *delayedCmdBuf, unsigned __int8 *data, unsigned int count);
