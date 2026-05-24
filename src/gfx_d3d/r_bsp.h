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
    uint32_t firstSurface;
    uint32_t surfaceCount;
    uint32_t childCount;
};

struct DiskGfxCullGroup // sizeof=0x20
{
    float mins[3];
    float maxs[3];
    uint32_t firstSurface;
    uint32_t surfaceCount;
};

struct DiskGfxVertex // sizeof=0x44
{
    float xyz[3];
    float normal[3];
    uint8_t color[4];
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
    uint32_t children[2];
};
struct r_lightmapGroup_t // sizeof=0x8
{                                       // ...
    int wideCount;                      // ...
    int highCount;                      // ...
};
struct LightDefCopyConfig // sizeof=0x8
{                                       // ...
    uint8_t *dest;              // ...
    uint32_t zoom;                  // ...
};
struct DiskLightRegion // sizeof=0x1
{
    uint8_t hullCount;
};
struct r_lightmapMerge_t // sizeof=0x14
{                                       // ...
    uint8_t index;
    // padding byte
    // padding byte
    // padding byte
    float shift[2];
    float scale[2];
};

struct DiskTriangleSoup // sizeof=0x18
{
    uint16_t materialIndex;
    uint8_t lightmapIndex;
    uint8_t reflectionProbeIndex;
    uint8_t primaryLightIndex;
    bool castsSunShadow;
    uint8_t unused[2];
    int vertexLayerData;
    uint32_t firstVertex;
    uint16_t vertexCount;
    uint16_t indexCount;
    int firstIndex;
};

struct DiskTriangleSoup_Version8 // sizeof=0x10
{
    uint16_t materialIndex;
    uint8_t lightmapIndex;
    uint8_t reflectionProbeIndex;
    int firstVertex;
    uint16_t vertexCount;
    uint16_t indexCount;
    int firstIndex;
};

struct DiskTriangleSoup_Version12 // sizeof=0x14
{
    uint16_t materialIndex;
    uint8_t lightmapIndex;
    uint8_t reflectionProbeIndex;
    int vertexLayerData;
    int firstVertex;
    uint16_t vertexCount;
    uint16_t indexCount;
    int firstIndex;
};


struct GfxBspLoad // sizeof=0x2A8
{                                       // ...
    uint32_t bspVersion;            // ...
    TrisType trisType;                  // ...
    const struct dmaterial_t *diskMaterials;   // ...
    uint32_t materialCount;
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
    uint16_t *indices;          // ...
    int surfaceCount;                   // ...
    GfxWorldStreamInfo streamInfo;
    // padding byte
    // padding byte
    // padding byte
    int skySurfCount;                   // ...
    int *skyStartSurfs;                 // ...
    GfxImage *skyImage;                 // ...
    uint8_t skySamplerState;    // ...
    // padding byte
    // padding byte
    // padding byte
    uint32_t vertexCount;           // ...
    GfxWorldVertexData vd;              // ...
    uint32_t vertexLayerDataSize;   // ...
    GfxWorldVertexLayerData vld;        // ...
    SunLightParseParams sunParse;       // ...
    GfxLight *sunLight;                 // ...
    float sunColorFromBsp[3];
    uint32_t sunPrimaryLightIndex;  // ...
    uint32_t primaryLightCount;     // ...
    int cullGroupCount;                 // ...
    uint32_t reflectionProbeCount;  // ...
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
    uint32_t checksum;
    int materialMemoryCount;            // ...
    struct MaterialMemory *materialMemory;     // ...
    sunflare_t sun;                     // ...
    float outdoorLookupMatrix[4][4];
    GfxImage *outdoorImage;
    uint32_t *cellCasterBits;       // ...
    struct GfxSceneDynModel *sceneDynModel;    // ...
    struct GfxSceneDynBrush *sceneDynBrush;    // ...
    uint32_t *primaryLightEntityShadowVis; // ...
    uint32_t *primaryLightDynEntShadowVis[2]; // ...
    uint8_t *nonSunPrimaryLightForModelDynEnt; // ...
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

uint32_t R_GetDebugReflectionProbeLocs(float (*locArray)[3], uint32_t maxCount);

extern GfxWorld s_world;
extern r_globals_load_t rgl;

// r_bsp_load_obj
void __cdecl R_ModernizeLegacyLightGridColors(const uint8_t *legacyColors, GfxLightGridColors *modernColors);
GfxWorld *__cdecl R_LoadWorldInternal(const char *name);
void __cdecl R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *sunParse, GfxLight *sunLight);
void __cdecl R_SetUpSunLight(const float *sunColor, const float *sunDirection, GfxLight *light);
void __cdecl R_InitPrimaryLights(GfxLight *primaryLights);
void __cdecl R_AddShadowSurfaceToPrimaryLight(
    GfxWorld *world,
    uint32_t primaryLightIndex,
    uint32_t sortedSurfIndex);
void __cdecl R_ForEachPrimaryLightAffectingSurface(
    GfxWorld *world,
    const GfxSurface *surface,
    uint32_t sortedSurfIndex,
    void(__cdecl *Callback)(GfxWorld *, uint32_t, uint32_t));
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
    uint8_t *list,
    uint32_t count,
    GfxBspDrawSurfData *surfData);
void __cdecl R_AddAllBspDrawSurfacesCamera();
void __cdecl R_AddAllBspDrawSurfacesRangeCamera(
    uint32_t beginSurface,
    uint32_t endSurface,
    uint32_t stage,
    uint32_t maxDrawSurfCount);
void __cdecl R_AddAllBspDrawSurfacesCameraNonlit(
    uint32_t beginSurface,
    uint32_t endSurface,
    uint32_t stage);
void __cdecl R_AddAllBspDrawSurfacesSunShadow();
void __cdecl R_AddAllBspDrawSurfacesRangeSunShadow(
    uint32_t partitionIndex,
    uint32_t beginSurface,
    uint32_t endSurface,
    uint32_t maxDrawSurfCount);
void __cdecl R_AddAllBspDrawSurfacesSpotShadow(uint32_t spotShadowIndex, uint32_t primaryLightIndex);


// r_add_cmdbuf
void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
void __cdecl R_EndCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
int __cdecl R_AllocDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    GfxDrawSurf drawSurf,
    GfxDrawSurfList *drawSurfList,
    uint32_t size);
void __cdecl R_WritePrimDrawSurfInt(GfxDelayedCmdBuf *delayedCmdBuf, uint32_t value);
void __cdecl R_WritePrimDrawSurfData(GfxDelayedCmdBuf *delayedCmdBuf, uint8_t *data, uint32_t count);
