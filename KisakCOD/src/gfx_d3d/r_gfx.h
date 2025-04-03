#pragma once

#include <d3d9.h>

#include <universal/com_math.h>

struct Material;
struct XModel;

struct srfTriangles_t // sizeof=0x10
{                                       // ...
    int vertexLayerData;
    int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 triCount;
    int baseIndex;
};

union GfxColor // sizeof=0x4
{                                       // ...
    operator unsigned int()
    {
        return packed;
    }
    GfxColor()
    {
        packed = 0;
    }
    GfxColor(int i)
    {
        packed = i;
    }
    unsigned int packed;
    unsigned __int8 array[4];
};

struct PackedTexCoords // sizeof=0x4
{                                       // ...
    unsigned int packed;
};

struct GfxPackedVertex // sizeof=0x20
{                                       // ...
    float xyz[3];
    float binormalSign;
    GfxColor color;
    PackedTexCoords texCoord;
    PackedUnitVec normal;
    PackedUnitVec tangent;
};

struct GfxPackedVertexNormal // sizeof=0x8
{                                       // ...
    PackedUnitVec normal;
    PackedUnitVec tangent;
};

struct GfxDynamicIndices // sizeof=0xC
{                                       // ...
    volatile int used;
    int total;
    unsigned __int16* indices;          // ...
};

struct GfxVertexBufferState // sizeof=0x10
{                                       // ...
    volatile int used;
    int total;
    IDirect3DVertexBuffer9* buffer;     // ...
    unsigned __int8* verts;
};

struct GfxMeshData // sizeof=0x20
{                                       // ...
    unsigned int indexCount;
    unsigned int totalIndexCount;
    unsigned __int16* indices;
    GfxVertexBufferState vb;
    unsigned int vertSize;
};


struct GfxMatrix // sizeof=0x40
{                                       // ...
    float m[4][4];                      // ...
};

struct GfxPlacement // sizeof=0x1C
{                                       // ...
    float quat[4];                      // ...
    float origin[3];                    // ...
};

struct GfxScaledPlacement // sizeof=0x20
{                                       // ...
    GfxPlacement base;                  // ...
    float scale;                        // ...
};

struct GfxViewParms // sizeof=0x140
{                                       // ...
    GfxMatrix viewMatrix;
    GfxMatrix projectionMatrix;         // ...
    GfxMatrix viewProjectionMatrix;     // ...
    GfxMatrix inverseViewProjectionMatrix; // ...
    float origin[4];                    // ...
    float axis[3][3];                   // ...
    float depthHackNearClip;
    float zNear;
    int pad;
};

struct GfxIndexBufferState // sizeof=0x10
{                                       // ...
    volatile int used;
    int total;
    IDirect3DIndexBuffer9* buffer;      // ...
    unsigned __int16* indices;
};

enum GfxPrimStatsTarget : __int32
{                                       // ...
    GFX_PRIM_STATS_WORLD = 0x0,
    GFX_PRIM_STATS_SMODELCACHED = 0x1,
    GFX_PRIM_STATS_SMODELRIGID = 0x2,
    GFX_PRIM_STATS_XMODELRIGID = 0x3,
    GFX_PRIM_STATS_XMODELSKINNED = 0x4,
    GFX_PRIM_STATS_BMODEL = 0x5,
    GFX_PRIM_STATS_FX = 0x6,
    GFX_PRIM_STATS_HUD = 0x7,
    GFX_PRIM_STATS_DEBUG = 0x8,
    GFX_PRIM_STATS_CODE = 0x9,
    GFX_PRIM_STATS_COUNT = 0xA,
};

enum GfxViewMode : __int32
{                                       // ...
    VIEW_MODE_NONE = 0x0,
    VIEW_MODE_3D = 0x1,
    VIEW_MODE_2D = 0x2,
    VIEW_MODE_IDENTITY = 0x3,
};

enum GfxViewportBehavior : __int32
{                                       // ...
    GFX_USE_VIEWPORT_FOR_VIEW = 0x0,    // ...
    GFX_USE_VIEWPORT_FULL = 0x1,    // ...
};

enum GfxDepthRangeType : __int32
{                                       // ...
    GFX_DEPTH_RANGE_SCENE = 0x0,
    GFX_DEPTH_RANGE_VIEWMODEL = 0x2,
    GFX_DEPTH_RANGE_FULL = -0x1,
};

enum MapType : __int32
{                                       // ...
    MAPTYPE_NONE = 0x0,
    MAPTYPE_INVALID1 = 0x1,
    MAPTYPE_INVALID2 = 0x2,
    MAPTYPE_2D = 0x3,
    MAPTYPE_3D = 0x4,
    MAPTYPE_CUBE = 0x5,
    MAPTYPE_COUNT = 0x6,
};

struct Picmip // sizeof=0x2
{                                       // ...
    unsigned __int8 platform[2];        // ...
};

struct CardMemory // sizeof=0x8
{                                       // ...
    int platform[2];                    // ...
};

__declspec(align(4)) struct GfxImageLoadDef // sizeof=0x14
{
    unsigned __int8 levelCount;
    unsigned __int8 flags;
    __int16 dimensions[3];
    int format;
    int resourceSize;
    unsigned __int8 data[1];
    // padding byte
    // padding byte
    // padding byte
};

union GfxTexture // sizeof=0x4
{                                       // ...
    IDirect3DBaseTexture9* basemap;
    IDirect3DTexture9* map;
    IDirect3DVolumeTexture9* volmap;
    IDirect3DCubeTexture9* cubemap;
    GfxImageLoadDef* loadDef;
};

struct GfxImage // sizeof=0x24
{                                       // ...
    MapType mapType;                    // ...
    GfxTexture texture;
    Picmip picmip;
    bool noPicmip;
    unsigned __int8 semantic;
    unsigned __int8 track;
    // padding byte
    // padding byte
    // padding byte
    CardMemory cardMemory;
    unsigned __int16 width;
    unsigned __int16 height;
    unsigned __int16 depth;
    unsigned __int8 category;
    bool delayLoadPixels;
    const char* name;
};

struct GfxCodeMatrices // sizeof=0x800
{                                       // ...
    GfxMatrix matrix[32];
};

struct __declspec(align(4)) GfxBuffers // sizeof=0x2400A0
{                                       // ...
    GfxDynamicIndices smodelCache;      // ...
    IDirect3DVertexBuffer9* smodelCacheVb; // ...
    GfxIndexBufferState preTessIndexBufferPool[2]; // ...
    GfxIndexBufferState* preTessIndexBuffer; // ...
    int preTessBufferFrame;             // ...
    GfxIndexBufferState dynamicIndexBufferPool[1]; // ...
    GfxIndexBufferState* dynamicIndexBuffer; // ...
    GfxVertexBufferState skinnedCacheVbPool[2]; // ...
    unsigned __int8* skinnedCacheLockAddr; // ...
    GfxVertexBufferState dynamicVertexBufferPool[1]; // ...
    GfxVertexBufferState* dynamicVertexBuffer; // ...
    IDirect3DVertexBuffer9* particleCloudVertexBuffer; // ...
    IDirect3DIndexBuffer9* particleCloudIndexBuffer; // ...
    int dynamicBufferFrame;             // ...
    GfxPackedVertexNormal skinnedCacheNormals[2][147456]; // ...
    GfxPackedVertexNormal* skinnedCacheNormalsAddr; // ...
    GfxPackedVertexNormal* oldSkinnedCacheNormalsAddr; // ...
    unsigned int skinnedCacheNormalsFrameCount; // ...
    bool fastSkin;                      // ...
    bool skinCache;                     // ...
    // padding byte
    // padding byte
};

struct GfxPointVertex // sizeof=0x10
{                                       // ...
    float xyz[3];
    unsigned __int8 color[4];           // ...
};


struct GfxStaticModelInst // sizeof=0x1C
{                                       // ...
    float mins[3];
    float maxs[3];
    GfxColor groundLighting;
};

struct GfxSurface // sizeof=0x30
{                                       // ...
    srfTriangles_t tris;
    Material* material;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 primaryLightIndex;
    unsigned __int8 flags;
    float bounds[2][3];
};

struct GfxCullGroup // sizeof=0x20
{
    float mins[3];
    float maxs[3];
    int surfaceCount;
    int startSurfIndex;
};

struct GfxPackedPlacement // sizeof=0x34
{                                       // ...
    float origin[3];
    float axis[3][3];
    float scale;
};

struct __declspec(align(4)) GfxStaticModelDrawInst // sizeof=0x4C
{                                       // ...
    float cullDist;
    GfxPackedPlacement placement;
    XModel* model;
    unsigned __int16 smodelCacheIndex[4];
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 primaryLightIndex;
    unsigned __int16 lightingHandle;
    unsigned __int8 flags;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxDrawSurfFields // sizeof=0x8
{                                       // ...
    unsigned __int64 objectId : 16;
    unsigned __int64 reflectionProbeIndex : 8;
    unsigned __int64 customIndex : 5;
    unsigned __int64 materialSortedIndex : 11;
    unsigned __int64 prepass : 2;
    unsigned __int64 primaryLightIndex : 8;
    unsigned __int64 surfType : 4;
    unsigned __int64 primarySortKey : 6;
    unsigned __int64 unused : 4;
};

union GfxDrawSurf // sizeof=0x8
{                                       // ...
    operator GfxDrawSurfFields()
    {
        return fields;
    }
    operator const GfxDrawSurfFields()
    {
        return fields;
    }
    GfxDrawSurfFields fields;
    unsigned __int64 packed;
};

struct GfxDrawSurfList // sizeof=0x8
{                                       // ...
    GfxDrawSurf *current;               // ...
    GfxDrawSurf *end;                   // ...
};
struct GfxDelayedCmdBuf // sizeof=0x10
{                                       // ...
    int primDrawSurfPos;
    unsigned int primDrawSurfSize;
    GfxDrawSurf drawSurfKey;
};
struct GfxBspDrawSurfData // sizeof=0x18
{                                       // ...
    GfxDelayedCmdBuf delayedCmdBuf;
    GfxDrawSurfList drawSurfList;       // ...
};

struct GfxWorldDpvsStatic // sizeof=0x68
{                                       // ...
    unsigned int smodelCount;           // ...
    unsigned int staticSurfaceCount;    // ...
    unsigned int staticSurfaceCountNoDecal; // ...
    unsigned int litSurfsBegin;         // ...
    unsigned int litSurfsEnd;           // ...
    unsigned int decalSurfsBegin;       // ...
    unsigned int decalSurfsEnd;         // ...
    unsigned int emissiveSurfsBegin;    // ...
    unsigned int emissiveSurfsEnd;      // ...
    unsigned int smodelVisDataCount;    // ...
    unsigned int surfaceVisDataCount;   // ...
    unsigned __int8* smodelVisData[3];  // ...
    unsigned __int8* surfaceVisData[3]; // ...
    unsigned int* lodData;              // ...
    unsigned __int16* sortedSurfIndex;  // ...
    GfxStaticModelInst* smodelInsts;    // ...
    GfxSurface* surfaces;               // ...
    GfxCullGroup* cullGroups;           // ...
    GfxStaticModelDrawInst* smodelDrawInsts; // ...
    GfxDrawSurf* surfaceMaterials;      // ...
    unsigned int* surfaceCastsSunShadow; // ...
    volatile int usageCount;
};

struct GfxWorldDpvsDynamic // sizeof=0x30
{                                       // ...
    unsigned int dynEntClientWordCount[2]; // ...
    unsigned int dynEntClientCount[2];  // ...
    unsigned int* dynEntCellBits[2];    // ...
    unsigned __int8* dynEntVisData[2][3]; // ...
};

struct GfxWorldStreamInfo // sizeof=0x0
{                                       // ...
};

struct GfxWorldVertex // sizeof=0x2C
{                                       // ...
    float xyz[3];
    float binormalSign;
    GfxColor color;
    float texCoord[2];
    float lmapCoord[2];
    PackedUnitVec normal;
    PackedUnitVec tangent;
};

struct GfxWorldVertexData // sizeof=0x8
{                                       // ...
    GfxWorldVertex* vertices;           // ...
    IDirect3DVertexBuffer9* worldVb;    // ...
};

struct GfxWorldVertexLayerData // sizeof=0x8
{                                       // ...
    unsigned __int8* data;              // ...
    IDirect3DVertexBuffer9* layerVb;    // ...
};

struct SunLightParseParams // sizeof=0x80
{                                       // ...
    char name[64];
    float ambientScale;
    float ambientColor[3];
    float diffuseFraction;
    float sunLight;
    float sunColor[3];
    float diffuseColor[3];
    bool diffuseColorHasBeenSet;
    // padding byte
    // padding byte
    // padding byte
    float angles[3];
};

struct __declspec(align(4)) GfxLightImage // sizeof=0x8
{                                       // ...
    GfxImage* image;
    unsigned __int8 samplerState;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxLightDef // sizeof=0x10
{                                       // ...
    const char* name;
    GfxLightImage attenuation;
    int lmapLookupStart;
};

struct GfxLight // sizeof=0x40
{                                       // ...
    unsigned __int8 type;
    unsigned __int8 canUseShadowMap;
    unsigned __int8 unused[2];
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
    unsigned int spotShadowIndex;
    GfxLightDef* def;
};

struct GfxReflectionProbe // sizeof=0x10
{
    float origin[3];
    GfxImage* reflectionImage;
};

struct GfxWorldDpvsPlanes // sizeof=0x10
{                                       // ...
    int cellCount;                      // ...
    cplane_s* planes;                   // ...
    unsigned __int16* nodes;            // ...
    unsigned int* sceneEntCellBits;     // ...
};

struct GfxAabbTree // sizeof=0x2C
{
    float mins[3];
    float maxs[3];
    unsigned __int16 childCount;
    unsigned __int16 surfaceCount;
    unsigned __int16 startSurfIndex;
    unsigned __int16 surfaceCountNoDecal;
    unsigned __int16 startSurfIndexNoDecal;
    unsigned __int16 smodelIndexCount;
    unsigned __int16* smodelIndexes;
    int childrenOffset;
};

struct GfxPortal;

struct GfxPortalWritable // sizeof=0xC
{
    bool isQueued;
    bool isAncestor;
    unsigned __int8 recursionDepth;
    unsigned __int8 hullPointCount;
    float (*hullPoints)[2];
    GfxPortal* queuedParent;
};

struct DpvsPlane // sizeof=0x14
{                                       // ...
    float coeffs[4];                    // ...
    unsigned __int8 side[3];            // ...
    unsigned __int8 pad;
};

struct GfxCell;

struct GfxPortal // sizeof=0x44
{
    GfxPortalWritable writable;
    DpvsPlane plane;
    GfxCell* cell;
    float (*vertices)[3];
    unsigned __int8 vertexCount;
    // padding byte
    // padding byte
    // padding byte
    float hullAxis[2][3];
};

struct GfxCell // sizeof=0x38
{
    float mins[3];
    float maxs[3];
    int aabbTreeCount;
    GfxAabbTree* aabbTree;
    int portalCount;
    GfxPortal* portals;
    int cullGroupCount;
    int* cullGroups;
    unsigned __int8 reflectionProbeCount;
    // padding byte
    // padding byte
    // padding byte
    unsigned __int8* reflectionProbes;
};

struct GfxLightmapArray // sizeof=0x8
{
    GfxImage* primary;
    GfxImage* secondary;
};

struct GfxLightGridEntry // sizeof=0x4
{                                       // ...
    unsigned __int16 colorsIndex;
    unsigned __int8 primaryLightIndex;  // ...
    unsigned __int8 needsTrace;
};

struct GfxLightGridColors // sizeof=0xA8
{                                       // ...
    unsigned __int8 rgb[56][3];
};

struct GfxLightGrid // sizeof=0x38
{                                       // ...
    bool hasLightRegions;               // ...
    // padding byte
    // padding byte
    // padding byte
    unsigned int sunPrimaryLightIndex;  // ...
    unsigned __int16 mins[3];           // ...
    unsigned __int16 maxs[3];           // ...
    unsigned int rowAxis;               // ...
    unsigned int colAxis;               // ...
    unsigned __int16* rowDataStart;     // ...
    unsigned int rawRowDataSize;        // ...
    unsigned __int8* rawRowData;        // ...
    unsigned int entryCount;            // ...
    GfxLightGridEntry* entries;         // ...
    unsigned int colorCount;            // ...
    GfxLightGridColors* colors;         // ...
};

struct GfxBrushModelWritable // sizeof=0x18
{                                       // ...
    float mins[3];
    float maxs[3];
};

struct __declspec(align(4)) GfxBrushModel // sizeof=0x38
{
    GfxBrushModelWritable writable;
    float bounds[2][3];
    unsigned __int16 surfaceCount;
    unsigned __int16 startSurfIndex;
    unsigned __int16 surfaceCountNoDecal;
    // padding byte
    // padding byte
};

struct GfxStateBits // sizeof=0x8
{
    unsigned int loadBits[2];
};

struct GfxFog // sizeof=0x14
{                                       // ...
    int startTime;                      // ...
    int finishTime;                     // ...
    GfxColor color;                     // ...
    float fogStart;                     // ...
    float density;                      // ...
};

struct GfxVertex // sizeof=0x20
{                                       // ...
    float xyzw[4];
    GfxColor color;                     // ...
    float texCoord[2];                  // ...
    PackedUnitVec normal;               // ...
};

struct GfxVertexShaderLoadDef // sizeof=0x8
{                                       // ...
    void* program;
    unsigned __int16 programSize;
    unsigned __int16 loadForRenderer;
};

struct GfxPixelShaderLoadDef // sizeof=0x8
{                                       // ...
    void* program;
    unsigned __int16 programSize;
    unsigned __int16 loadForRenderer;
};

struct GfxDepthOfField // sizeof=0x20
{                                       // ...
    float viewModelStart;
    float viewModelEnd;
    float nearStart;
    float nearEnd;
    float farStart;
    float farEnd;
    float nearBlur;
    float farBlur;
};
struct GfxFilm // sizeof=0x2C
{                                       // ...
    bool enabled;
    // padding byte
    // padding byte
    // padding byte
    float brightness;
    float contrast;
    float desaturation;
    bool invert;
    // padding byte
    // padding byte
    // padding byte
    float tintDark[3];
    float tintLight[3];
};
struct GfxGlow // sizeof=0x14
{                                       // ...
    bool enabled;
    // padding byte
    // padding byte
    // padding byte
    float bloomCutoff;
    float bloomDesaturation;
    float bloomIntensity;
    float radius;
};

struct GfxQuadMeshData // sizeof=0x30
{                                       // ...
    float x;
    float y;
    float width;
    float height;
    GfxMeshData meshData;               // ...
};

struct GfxEntity // sizeof=0x8
{                                       // ...
    unsigned int renderFxFlags;
    float materialTime;
};

struct GfxSceneDef // sizeof=0x14
{                                       // ...
    int time;                           // ...
    float floatTime;                    // ...
    float viewOffset[3];                // ...
};

struct GfxViewport // sizeof=0x10
{                                       // ...
    int x;                              // ...
    int y;                              // ...
    int width;                          // ...
    int height;                         // ...
};

struct refdef_s // sizeof=0x4098
{                                       // ...
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    float tanHalfFovX;
    float tanHalfFovY;
    float vieworg[3];
    float viewaxis[3][3];
    float viewOffset[3];
    int time;
    float zNear;
    float blurRadius;
    GfxDepthOfField dof;
    GfxFilm film;
    GfxGlow glow;
    GfxLight primaryLights[255];
    GfxViewport scissorViewport;
    bool useScissorViewport;
    // padding byte
    // padding byte
    // padding byte
    int localClientNum;
};
