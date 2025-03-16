#pragma once

#include "xanim_public.h"

#include <d3d9.h> // KISAKTODO: move to gfx I think
#include <script/scr_stringlist.h>
#include <cgame_mp/cg_local_mp.h>

enum MapType
{                                       // ...
    MAPTYPE_NONE = 0x0,
    MAPTYPE_INVALID1 = 0x1,
    MAPTYPE_INVALID2 = 0x2,
    MAPTYPE_2D = 0x3,
    MAPTYPE_3D = 0x4,
    MAPTYPE_CUBE = 0x5,
    MAPTYPE_COUNT = 0x6,
};

union XAnimIndices // sizeof=0x4
{                                       // ...
    unsigned __int8 *_1;
    unsigned __int16 *_2;
    void *data;
};
struct XAnimNotifyInfo // sizeof=0x8
{
    unsigned __int16 name;
    // padding byte
    // padding byte
    float time;
};
union XAnimDynamicIndices // sizeof=0x2
{                                       // ...
    unsigned __int8 _1[1];
    unsigned __int16 _2[1];
};
struct __declspec(align(4)) XAnimDeltaPartQuatDataFrames // sizeof=0x8
{                                       // ...
    __int16 (*frames)[2];
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimDynamicFrames // sizeof=0x4
{                                       // ...
    unsigned __int8 (*_1)[3];
    unsigned __int16 (*_2)[3];
};
struct __declspec(align(4)) XAnimPartTransFrames // sizeof=0x20
{                                       // ...
    float mins[3];
    float size[3];
    XAnimDynamicFrames frames;
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimPartTransData // sizeof=0x20
{                                       // ...
    XAnimPartTransFrames frames;
    float frame0[3];
};
struct XAnimPartTrans // sizeof=0x24
{
    unsigned __int16 size;
    unsigned __int8 smallTrans;
    // padding byte
    XAnimPartTransData u;
};
union XAnimDeltaPartQuatData // sizeof=0x8
{                                       // ...
    XAnimDeltaPartQuatDataFrames frames;
    __int16 frame0[2];
};

struct XAnimDeltaPartQuat // sizeof=0xC
{
    unsigned __int16 size;
    // padding byte
    // padding byte
    XAnimDeltaPartQuatData u;
};
struct XAnimDeltaPart // sizeof=0x8
{
    XAnimPartTrans *trans;
    XAnimDeltaPartQuat *quat;
};
struct XAnimParts // sizeof=0x58
{                                       // ...
    const char *name;
    unsigned __int16 dataByteCount;
    unsigned __int16 dataShortCount;
    unsigned __int16 dataIntCount;
    unsigned __int16 randomDataByteCount;
    unsigned __int16 randomDataIntCount;
    unsigned __int16 numframes;
    bool bLoop;
    bool bDelta;
    unsigned __int8 boneCount[10];
    unsigned __int8 notifyCount;
    unsigned __int8 assetType;
    bool isDefault;
    // padding byte
    unsigned int randomDataShortCount;
    unsigned int indexCount;
    float framerate;
    float frequency;
    unsigned __int16 *names;
    unsigned __int8 *dataByte;
    __int16 *dataShort;
    int *dataInt;
    __int16 *randomDataShort;
    unsigned __int8 *randomDataByte;
    int *randomDataInt;
    XAnimIndices indices;
    XAnimNotifyInfo *notify;
    XAnimDeltaPart *deltaPart;
};

struct XModelNameMap // sizeof=0x4
{                                       // ...
    unsigned __int16 name;              // ...
    unsigned __int16 index;
};

struct XAnimParent // sizeof=0x4
{                                       // ...
    unsigned __int16 flags;
    unsigned __int16 children;
};
struct XAnimEntry // sizeof=0x8
{                                       // ...
    unsigned __int16 numAnims;
    unsigned __int16 parent;
    union //$7F333398CC08E12E110886895274CBFC
    {
        XAnimParts * parts;
        XAnimParent animParent;
    };
};
struct XAnim_s // sizeof=0x14
{
    const char *debugName;
    unsigned int size;
    const char **debugAnimNames;
    XAnimEntry entries[1];
};

struct __declspec(align(4)) XAnimTree_s // sizeof=0x14
{
    XAnim_s *anims;
    int info_usage;
    volatile int calcRefCount;
    volatile int modifyRefCount;
    unsigned __int16 children;
    // padding byte
    // padding byte
};

struct DObjAnimMat // sizeof=0x20
{                                       // ...
    float quat[4];                      // ...
    float trans[3];                     // ...
    float transWeight;                  // ...
};

struct XSurfaceVertexInfo // sizeof=0xC
{                                       // ...
    __int16 vertCount[4];
    unsigned __int16 *vertsBlend;
};

union GfxColor // sizeof=0x4
{                                       // ...
    unsigned int packed;
    unsigned __int8 array[4];
};

struct PackedTexCoords // sizeof=0x4
{                                       // ...
    unsigned int packed;
};

union PackedUnitVec // sizeof=0x4
{                                       // ...
    unsigned int packed;
    unsigned __int8 array[4];
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

struct XSurfaceCollisionAabb // sizeof=0xC
{                                       // ...
    unsigned __int16 mins[3];
    unsigned __int16 maxs[3];
};

struct XSurfaceCollisionNode // sizeof=0x10
{
    XSurfaceCollisionAabb aabb;
    unsigned __int16 childBeginIndex;
    unsigned __int16 childCount;
};

struct XSurfaceCollisionLeaf // sizeof=0x2
{
    unsigned __int16 triangleBeginIndex;
};

struct XSurfaceCollisionTree // sizeof=0x28
{
    float trans[3];
    float scale[3];
    unsigned int nodeCount;
    XSurfaceCollisionNode *nodes;
    unsigned int leafCount;
    XSurfaceCollisionLeaf *leafs;
};
struct XRigidVertList // sizeof=0xC
{                                       // ...
    unsigned __int16 boneOffset;        // ...
    unsigned __int16 vertCount;         // ...
    unsigned __int16 triOffset;         // ...
    unsigned __int16 triCount;          // ...
    XSurfaceCollisionTree *collisionTree;
};

struct XSurface // sizeof=0x38
{
    unsigned __int8 tileMode;
    bool deformed;
    unsigned __int16 vertCount;
    unsigned __int16 triCount;
    unsigned __int8 zoneHandle;
    // padding byte
    unsigned __int16 baseTriIndex;
    unsigned __int16 baseVertIndex;
    unsigned __int16 *triIndices;
    XSurfaceVertexInfo vertInfo;
    GfxPackedVertex *verts0;
    unsigned int vertListCount;
    XRigidVertList *vertList;
    int partBits[4];
};

struct MaterialStreamRouting // sizeof=0x2
{                                       // ...
    unsigned __int8 source;             // ...
    unsigned __int8 dest;               // ...
};

struct MaterialVertexStreamRouting // sizeof=0x60
{                                       // ...
    MaterialStreamRouting data[16];
    IDirect3DVertexDeclaration9 *decl[16];
};

struct MaterialVertexDeclaration // sizeof=0x64
{                                       // ...
    unsigned __int8 streamCount;
    bool hasOptionalSource;
    bool isLoaded;
    // padding byte
    MaterialVertexStreamRouting routing;
};
struct GfxVertexShaderLoadDef // sizeof=0x8
{                                       // ...
    unsigned int* program;
    unsigned __int16 programSize;
    unsigned __int16 loadForRenderer;
};
struct MaterialVertexShaderProgram // sizeof=0xC
{                                       // ...
    IDirect3DVertexShader9* vs;
    GfxVertexShaderLoadDef loadDef;
};
struct MaterialVertexShader // sizeof=0x10
{                                       // ...
    const char* name;
    MaterialVertexShaderProgram prog;
};
struct GfxPixelShaderLoadDef // sizeof=0x8
{                                       // ...
    unsigned int* program;
    unsigned __int16 programSize;
    unsigned __int16 loadForRenderer;
};
struct MaterialPixelShaderProgram // sizeof=0xC
{                                       // ...
    IDirect3DPixelShader9* ps;
    GfxPixelShaderLoadDef loadDef;
};
struct MaterialPixelShader // sizeof=0x10
{                                       // ...
    const char* name;
    MaterialPixelShaderProgram prog;
};
struct MaterialArgumentCodeConst // sizeof=0x4
{                                       // ...
    unsigned __int16 index;
    unsigned __int8 firstRow;
    unsigned __int8 rowCount;
};
union MaterialArgumentDef // sizeof=0x4
{                                       // ...
    const float* literalConst;
    MaterialArgumentCodeConst codeConst;
    unsigned int codeSampler;
    unsigned int nameHash;
};
struct MaterialShaderArgument // sizeof=0x8
{                                       // ...
    unsigned __int16 type;
    unsigned __int16 dest;
    MaterialArgumentDef u;
};
struct MaterialPass // sizeof=0x14
{                                       // ...
    MaterialVertexDeclaration * vertexDecl; // ...
    MaterialVertexShader * vertexShader;
    MaterialPixelShader* pixelShader;
    unsigned __int8 perPrimArgCount;
    unsigned __int8 perObjArgCount;
    unsigned __int8 stableArgCount;
    unsigned __int8 customSamplerFlags;
    MaterialShaderArgument * args;
};

struct MaterialTechnique // sizeof=0x1C
{
    const char *name;
    unsigned __int16 flags;
    unsigned __int16 passCount;
    MaterialPass passArray[1];
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
    IDirect3DBaseTexture9 *basemap;
    IDirect3DTexture9 *map;
    IDirect3DVolumeTexture9 *volmap;
    IDirect3DCubeTexture9 *cubemap;
    GfxImageLoadDef *loadDef;
};

struct Picmip // sizeof=0x2
{                                       // ...
    unsigned __int8 platform[2];        // ...
};

struct CardMemory // sizeof=0x8
{                                       // ...
    int platform[2];                    // ...
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
    const char *name;
};
struct WaterWritable // sizeof=0x4
{                                       // ...
    float floatTime;
};

struct complex_s // sizeof=0x8
{                                       // ...
    float real;                         // ...
    float imag;                         // ...
};
struct water_t // sizeof=0x44
{                                       // ...
    WaterWritable writable;
    complex_s * H0;
    float* wTerm;
    int M;                              // ...
    int N;                              // ...
    float Lx;                           // ...
    float Lz;                           // ...
    float gravity;                      // ...
    float windvel;                      // ...
    float winddir[2];                   // ...
    float amplitude;                    // ...
    float codeConstant[4];
    GfxImage * image;                    // ...
};

union MaterialTextureDefInfo // sizeof=0x4
{                                       // ...
    GfxImage *image;
    water_t *water;
};

struct MaterialTextureDef // sizeof=0xC
{
    unsigned int nameHash;
    char nameStart;
    char nameEnd;
    unsigned __int8 samplerState;
    unsigned __int8 semantic;
    MaterialTextureDefInfo u;
};

struct MaterialConstantDef // sizeof=0x20
{
    unsigned int nameHash;
    char name[12];
    float literal[4];
};

struct GfxStateBits // sizeof=0x8
{
    unsigned int loadBits[2];
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
    GfxDrawSurfFields fields;
    unsigned __int64 packed;
};
 struct __declspec(align(4)) MaterialInfo // sizeof=0x18
 {                                       // ...
     const char* name;                   // ...
     unsigned __int8 gameFlags;
     unsigned __int8 sortKey;
     unsigned __int8 textureAtlasRowCount; // ...
     unsigned __int8 textureAtlasColumnCount; // ...
     GfxDrawSurf drawSurf;
     unsigned int surfaceTypeBits;
     unsigned __int16 hashIndex;
     // padding byte
     // padding byte
 };

 struct MaterialTechniqueSet // sizeof=0x94
 {                                       // ...
     const char* name;
     unsigned __int8 worldVertFormat;
     bool hasBeenUploaded;
     unsigned __int8 unused[1];
     // padding byte
     MaterialTechniqueSet* remappedTechniqueSet;
     MaterialTechnique* techniques[34];
 };

struct Material // sizeof=0x50
{                                       // ...
    MaterialInfo info;
    unsigned __int8 stateBitsEntry[34];
    unsigned __int8 textureCount;
    unsigned __int8 constantCount;
    unsigned __int8 stateBitsCount;
    unsigned __int8 stateFlags;
    unsigned __int8 cameraRegion;
    // padding byte
    MaterialTechniqueSet *techniqueSet;
    MaterialTextureDef *textureTable;
    MaterialConstantDef *constantTable;
    GfxStateBits *stateBitsTable;
};

struct XModelLodInfo // sizeof=0x1C
{                                       // ...
    float dist;
    unsigned __int16 numsurfs;
    unsigned __int16 surfIndex;
    int partBits[4];
    unsigned __int8 lod;
    unsigned __int8 smcIndexPlusOne;
    unsigned __int8 smcAllocBits;
    unsigned __int8 unused;
};

struct XModelCollTri_s // sizeof=0x30
{
    float plane[4];
    float svec[4];
    float tvec[4];
};

struct XBoneInfo // sizeof=0x28
{                                       // ...
    float bounds[2][3];
    float offset[3];
    float radiusSquared;
};

struct XModelCollSurf_s // sizeof=0x2C
{
    XModelCollTri_s* collTris;
    int numCollTris;
    float mins[3];
    float maxs[3];
    int boneIdx;
    int contents;
    int surfFlags;
};

struct XModelStreamInfo // sizeof=0x0
{                                       // ...
};

struct __declspec(align(4)) PhysPreset // sizeof=0x2C
{                                       // ...
    const char* name;                   // ...
    int type;                           // ...
    float mass;                         // ...
    float bounce;                       // ...
    float friction;                     // ...
    float bulletForceScale;             // ...
    float explosiveForceScale;          // ...
    const char* sndAliasPrefix;         // ...
    float piecesSpreadFraction;
    float piecesUpwardVelocity;
    bool tempDefaultToCylinder;
    // padding byte
    // padding byte
    // padding byte
};

struct cplane_s // sizeof=0x14
{                                       // ...
    float normal[3];                    // ...
    float dist;
    unsigned __int8 type;
    unsigned __int8 signbits;
    unsigned __int8 pad[2];
};

struct __declspec(align(2)) cbrushside_t // sizeof=0xC
{                                       // ...
    cplane_s* plane;                    // ...
    unsigned int materialNum;           // ...
    __int16 firstAdjacentSideOffset;
    unsigned __int8 edgeCount;
    // padding byte
};

struct BrushWrapper // sizeof=0x50
{
    float mins[3];
    int contents;
    float maxs[3];
    unsigned int numsides;
    cbrushside_t* sides;
    __int16 axialMaterialNum[2][3];
    unsigned __int8* baseAdjacentSide;
    __int16 firstAdjacentSideOffsets[2][3];
    unsigned __int8 edgeCount[2][3];
    // padding byte
    // padding byte
    int totalEdgeCount;
    cplane_s* planes;
};

struct PhysMass // sizeof=0x24
{                                       // ...
    float centerOfMass[3];
    float momentsOfInertia[3];
    float productsOfInertia[3];
};

__declspec(align(4)) struct PhysPreset // sizeof=0x2C
{                                       // ...
    const char* name;                   // ...
    int type;                           // ...
    float mass;                         // ...
    float bounce;                       // ...
    float friction;                     // ...
    float bulletForceScale;             // ...
    float explosiveForceScale;          // ...
    const char* sndAliasPrefix;         // ...
    float piecesSpreadFraction;
    float piecesUpwardVelocity;
    bool tempDefaultToCylinder;
    // padding byte
    // padding byte
    // padding byte
};
struct PhysGeomInfo // sizeof=0x44
{
    BrushWrapper* brush;
    int type;
    float orientation[3][3];
    float offset[3];
    float halfLengths[3];
};

struct PhysGeomList // sizeof=0x2C
{
    unsigned int count;
    PhysGeomInfo* geoms;
    PhysMass mass;
};

struct XModel // sizeof=0xDC
{                                       // ...
    const char *name;
    unsigned __int8 numBones;
    unsigned __int8 numRootBones;
    unsigned __int8 numsurfs;
    unsigned __int8 lodRampType;
    unsigned __int16 *boneNames;
    unsigned __int8 *parentList;
    __int16 *quats;
    float *trans;
    unsigned __int8 *partClassification;
    DObjAnimMat *baseMat;
    XSurface *surfs;
    Material **materialHandles;
    XModelLodInfo lodInfo[4];
    XModelCollSurf_s *collSurfs;
    int numCollSurfs;
    int contents;
    XBoneInfo *boneInfo;
    float radius;
    float mins[3];
    float maxs[3];
    __int16 numLods;
    __int16 collLod;
    XModelStreamInfo streamInfo;
    // padding byte
    // padding byte
    // padding byte
    int memUsage;
    unsigned __int8 flags;
    bool bad;
    // padding byte
    // padding byte
    PhysPreset *physPreset;
    PhysGeomList *physGeoms;
};

struct __declspec(align(4)) XAnimState // sizeof=0x20
{                                       // ...
    float currentAnimTime;              // ...
    float oldTime;                      // ...
    __int16 cycleCount;                 // ...
    __int16 oldCycleCount;              // ...
    float goalTime;                     // ...
    float goalWeight;                   // ...
    float weight;                       // ...
    float rate;                         // ...
    bool instantWeightChange;           // ...
    // padding byte
    // padding byte
    // padding byte
};
struct XAnimInfo // sizeof=0x40
{                                       // ...
    unsigned __int16 notifyChild;
    __int16 notifyIndex;
    unsigned __int16 notifyName;
    unsigned __int16 notifyType;
    unsigned __int16 prev;              // ...
    unsigned __int16 next;              // ...
    unsigned __int16 children;          // ...
    unsigned __int16 parent;            // ...
    unsigned __int16 animIndex;         // ...
    unsigned __int16 animToModel;
    bool inuse;                         // ...
    // padding byte
    // padding byte
    // padding byte
    XAnimTree_s* tree;
    //$7F333398CC08E12E110886895274CBFC ___u12;
    union
    {                                       // ...
        XAnimParts* parts;
        XAnimParent animParent;
    };
    XAnimState state;                   // ...
};

struct DObj_s // sizeof=0x64
{
    XAnimTree_s* tree;
    unsigned __int16 duplicateParts;
    unsigned __int16 entnum;
    unsigned __int8 duplicatePartsSize;
    unsigned __int8 numModels;
    unsigned __int8 numBones;
    // padding byte
    unsigned int ignoreCollision;
    volatile int locked;
    DSkel skel;
    float radius;
    unsigned int hidePartBits[4];
    XModel** models;
};

struct DSkelPartBits // sizeof=0x30
{                                       // ...
    int anim[4];                        // ...
    int control[4];                     // ...
    int skel[4];                        // ...
};
struct DSkel // sizeof=0x38
{                                       // ...
    DSkelPartBits partBits;             // ...
    int timeStamp;                      // ...
    DObjAnimMat* mat;                   // ...
};

struct XAnimSimpleRotPos // sizeof=0x18
{                                       // ...
    float rot[2];                       // ...
    float posWeight;                    // ...
    float pos[3];                       // ...
};

struct XAnimDeltaInfo // sizeof=0x4
{                                       // ...
    bool bClear;                        // ...
    bool bNormQuat;                     // ...
    bool bAbs;                          // ...
    bool bUseGoalWeight;                // ...
};

struct XAnimParent // sizeof=0x4
{                                       // ...
    unsigned __int16 flags;
    unsigned __int16 children;
};
struct XAnimEntry // sizeof=0x8
{                                       // ...
    unsigned __int16 numAnims;
    unsigned __int16 parent;
    //$7F333398CC08E12E110886895274CBFC ___u2;
    union
    {
        XAnimParts* parts;
        XAnimParent animParent;
    };
};
struct XAnim_s // sizeof=0x14
{
    const char* debugName;
    unsigned int size;
    const char** debugAnimNames;
    XAnimEntry entries[1];
};

struct XAnimNotify_s // sizeof=0xC
{                                       // ...
    const char* name;
    unsigned int type;
    float timeFrac;
};

void __cdecl SV_DObjInitServerTime(gentity_s* ent, float dtime);
void __cdecl SV_DObjDisplayAnim(gentity_s* ent, const char* header);

int __cdecl XAnimGetTreeHighMemUsage();
int __cdecl XAnimGetTreeMemUsage();
void __cdecl TRACK_xanim();
int __cdecl XAnimGetTreeMaxMemUsage();
void __cdecl XAnimInit();
void __cdecl XAnimShutdown();
XAnimParts* __cdecl XAnimFindData_LoadObj(const char* name);
XAnimParts* __cdecl XAnimFindData_FastFile(const char* name);
void __cdecl XAnimCreate(XAnim_s* anims, unsigned int animIndex, const char* name);
void __cdecl XAnimBlend(
    XAnim_s* anims,
    unsigned int animIndex,
    const char* name,
    unsigned int children,
    unsigned int num,
    unsigned int flags);
bool __cdecl IsNodeAdditive(const XAnimEntry* node);
BOOL __cdecl IsLeafNode(const XAnimEntry* anim);
XAnim_s* __cdecl XAnimCreateAnims(const char* debugName, unsigned int size, void* (__cdecl* Alloc)(int));
void __cdecl XAnimFreeList(XAnim_s* anims);
int __cdecl XAnimTreeSize();
XAnimTree_s* __cdecl XAnimCreateTree(XAnim_s* anims, void* (__cdecl* Alloc)(int));
void __cdecl XAnimFreeTree(XAnimTree_s* tree, void(__cdecl* Free)(void*, int));
void XAnimCheckTreeLeak();
XAnim_s* __cdecl XAnimGetAnims(const XAnimTree_s* tree);
void XAnimResetAnimMap(XModelNameMap a1, const DObj_s* obj, unsigned int infoIndex);
void __cdecl XAnimInitModelMap(XModel* const* models, unsigned int numModels, XModelNameMap* modelMap);
void __cdecl XAnimResetAnimMap_r(XModelNameMap* modelMap, unsigned int infoIndex);
void __cdecl XAnimResetAnimMapLeaf(const XModelNameMap* modelMap, unsigned int infoIndex);
HashEntry_unnamed_type_u __cdecl XAnimGetAnimMap(const XAnimParts* parts, const XModelNameMap* modelMap);
double __cdecl XAnimGetLength(const XAnim_s* anims, unsigned int animIndex);
int __cdecl XAnimGetLengthMsec(const XAnim_s* anims, unsigned int anim);
double __cdecl XAnimGetTime(const XAnimTree_s* tree, unsigned int animIndex);
unsigned int __cdecl XAnimGetInfoIndex(const XAnimTree_s* tree, unsigned int animIndex);
unsigned int __cdecl XAnimGetInfoIndex_r(const XAnimTree_s* tree, unsigned int animIndex, unsigned int infoIndex);
double __cdecl XAnimGetWeight(const XAnimTree_s* tree, unsigned int animIndex);
bool __cdecl XAnimHasFinished(const XAnimTree_s* tree, unsigned int animIndex);
int __cdecl XAnimGetNumChildren(const XAnim_s* anims, unsigned int animIndex);
unsigned int __cdecl XAnimGetChildAt(const XAnim_s* anims, unsigned int animIndex, unsigned int childIndex);
const char* __cdecl XAnimGetAnimName(const XAnim_s* anims, unsigned int animIndex);
char* __cdecl XAnimGetAnimDebugName(const XAnim_s* anims, unsigned int animIndex);
const char* __cdecl XAnimGetAnimTreeDebugName(const XAnim_s* anims);
unsigned int __cdecl XAnimGetAnimTreeSize(const XAnim_s* anims);
void __cdecl XAnimInitInfo(XAnimInfo* info);
void __cdecl XAnimUpdateOldTime(
    DObj_s* obj,
    unsigned int infoIndex,
    XAnimState* syncState,
    float dtime,
    bool parentHasWeight,
    bool* childHasTimeForParent);
unsigned int __cdecl XAnimInitTime(XAnimTree_s* tree, unsigned int infoIndex, float goalTime);
void __cdecl XAnimResetTime(unsigned int infoIndex);
void __cdecl XAnimResetTimeInternal(unsigned int infoIndex);
unsigned int __cdecl XAnimCloneInitTime(XAnimTree_s* tree, unsigned int infoIndex, unsigned int parentIndex);
void __cdecl DObjInitServerTime(DObj_s* obj, float dtime);
void __cdecl DObjUpdateClientInfo(DObj_s* obj, float dtime, bool notify);
void __cdecl XAnimUpdateTimeAndNotetrack(const DObj_s* obj, unsigned int infoIndex, float dtime, bool bNotify);
void __cdecl XAnimCheckFreeInfo(XAnimTree_s* tree, unsigned int infoIndex, int hasWeight);
void __cdecl XAnimFreeInfo(XAnimTree_s* tree, unsigned int infoIndex);
void __cdecl XAnimClearServerNotify(XAnimInfo* info);
double __cdecl XAnimGetAverageRateFrequency(__int64 tree);
void __cdecl XAnimUpdateTimeAndNotetrackLeaf(
    const DObj_s* obj,
    const XAnimParts* parts,
    unsigned int infoIndex,
    float dtime,
    bool bNotify);
void __cdecl XAnimProcessClientNotify(XAnimInfo* info, float dtime);
unsigned __int16 __cdecl XAnimGetNextNotifyIndex(const XAnimParts* parts, float time);
double __cdecl XAnimGetNotifyFracLeaf(const XAnimState* state, const XAnimState* nextState, float time, float dtime);
void __cdecl XAnimAddClientNotify(unsigned int notetrackName, float frac, unsigned int notifyType);
void __cdecl XAnimUpdateTimeAndNotetrackSyncSubTree(
    const DObj_s* obj,
    unsigned int infoIndex,
    float dtime,
    bool bNotify);
void __cdecl XAnimUpdateInfoSync(
    const DObj_s* obj,
    unsigned int infoIndex,
    bool bNotify,
    XAnimState* syncState,
    float dtime);
void __cdecl XAnimProcessServerNotify(const DObj_s* obj, XAnimInfo* info, float time);
XAnimParts* __cdecl XAnimGetParts(const XAnimTree_s* tree, XAnimInfo* info);
void __cdecl NotifyServerNotetrack(const DObj_s* obj, unsigned int notifyName, unsigned int notetrackName);
int __cdecl DObjUpdateServerInfo(DObj_s* obj, float dtime, int bNotify);
double __cdecl XAnimFindServerNoteTrack(const DObj_s* obj, unsigned int infoIndex, float dtime);
double __cdecl XAnimFindServerNoteTrackLeafNode(const DObj_s* obj, XAnimInfo* info, float dtime);
double __cdecl XAnimGetNextServerNotifyFrac(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime);
double __cdecl XAnimFindServerNoteTrackSyncSubTree(const DObj_s* obj, XAnimInfo* info, float dtime);
double __cdecl XAnimGetServerNotifyFracSyncTotal(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime);
int __cdecl DObjGetClientNotifyList(XAnimNotify_s** notifyList);
void __cdecl DObjDisplayAnimToBuffer(const DObj_s* obj, const char* header, char* buffer, int bufferSize);
void __cdecl XAnimDisplay(const XAnimTree_s* tree, __int64 infoIndex, char* buffer, __int64 bufferSize);
void __cdecl DObjDisplayAnim(const DObj_s* obj, const char* header);
void __cdecl XAnimCalcDelta(DObj_s* obj, unsigned int animIndex, float* rot, float* trans, bool bUseGoalWeight);
void __cdecl XAnimCalcDeltaTree(
    const DObj_s* obj,
    unsigned int infoIndex,
    float weightScale,
    XAnimDeltaInfo deltaInfo,
    XAnimSimpleRotPos* rotPos);
void __cdecl XAnimCalcRelDeltaParts(
    const XAnimParts* parts,
    float weightScale,
    float time1,
    float time2,
    XAnimSimpleRotPos* rotPos,
    int quatIndex);
void __cdecl TransformToQuatRefFrame(const float* rot, float* trans);
void __cdecl XAnimCalcAbsDeltaParts(const XAnimParts* parts, float weightScale, float time, XAnimSimpleRotPos* rotPos);
void __cdecl XAnimCalcAbsDelta(DObj_s* obj, unsigned int animIndex, float* rot, float* trans);
void __cdecl XAnimGetRelDelta(
    const XAnim_s* anims,
    unsigned int animIndex,
    float* rot,
    float* trans,
    float time1,
    float time2);
void __cdecl XAnimGetAbsDelta(const XAnim_s* anims, unsigned int animIndex, float* rot, float* trans, float time);
unsigned int __cdecl XAnimAllocInfoWithParent(
    XAnimTree_s* tree,
    unsigned __int16 animToModel,
    unsigned int animIndex,
    unsigned int parentInfoIndex,
    int after);
unsigned int XAnimAllocInfoIndex(
    unsigned int a1,
    DObj_s* obj,
    unsigned int animIndex,
    int after);
unsigned int __cdecl XAnimEnsureGoalWeightParent(DObj_s* obj, unsigned int animIndex);
void __cdecl XAnimClearGoalWeightInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime);
void __cdecl XAnimClearTreeGoalWeightsInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime);
void __cdecl XAnimClearTreeGoalWeights(XAnimTree_s* tree, unsigned int animIndex, float blendTime);
void __cdecl XAnimClearTreeGoalWeightsStrict(XAnimTree_s* tree, unsigned int animIndex, float blendTime);
void __cdecl XAnimClearGoalWeightKnobInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime);
int __cdecl XAnimSetCompleteGoalWeightNode(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType);
int __cdecl XAnimSetGoalWeightKnobAll(
    DObj_s* obj,
    unsigned int animIndex,
    unsigned int rootIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    int bRestart);
int __cdecl XAnimSetGoalWeightKnob(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    int bRestart);
void __cdecl XAnimClearTree(XAnimTree_s* tree);
int __cdecl XAnimSetGoalWeightNode(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType);
unsigned int __cdecl XAnimGetDescendantWithGreatestWeight(const XAnimTree_s* tree, unsigned int infoIndex);
void __cdecl XAnimSetupSyncNodes(XAnim_s* anims);
void __cdecl XAnimSetupSyncNodes_r(XAnim_s* anims, unsigned int animIndex);
void __cdecl XAnimFillInSyncNodes_r(XAnim_s* anims, unsigned int animIndex, bool bLoop);
bool __cdecl XAnimHasTime(const XAnim_s* anims, unsigned int animIndex);
BOOL __cdecl XAnimIsPrimitive(XAnim_s* anims, unsigned int animIndex);
void __cdecl XAnimSetTime(__int64 tree, float time);
void __cdecl XAnimUpdateServerNotifyIndex(XAnimInfo* info, const XAnimParts* parts);
unsigned int __cdecl XAnimRestart(XAnimTree_s* tree, unsigned int infoIndex, float goalTime);
int __cdecl XAnimSetGoalWeight(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType,
    int bRestart);
void __cdecl XAnimSetAnimRate(XAnimTree_s* tree, unsigned int animIndex, float rate);
bool __cdecl XAnimIsLooped(const XAnim_s* anims, unsigned int animIndex);
char __cdecl XAnimNotetrackExists(const XAnim_s* anims, unsigned int animIndex, unsigned int name);
void __cdecl XAnimAddNotetrackTimesToScriptArray(const XAnim_s* anims, unsigned int animIndex, unsigned int name);
int __cdecl XAnimSetCompleteGoalWeight(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType,
    int bRestart);
void __cdecl XAnimCloneAnimInfo(const XAnimInfo* from, XAnimInfo* to);
void __cdecl XAnimCloneAnimTree(const XAnimTree_s* from, XAnimTree_s* to);
void __cdecl XAnimCloneAnimTree_r(
    const XAnimTree_s* from,
    XAnimTree_s* to,
    unsigned int fromInfoIndex,
    unsigned int toInfoParentIndex);
XAnimInfo* __cdecl GetAnimInfo(int infoIndex);
//void __thiscall bitarray<128>::setBit(bitarray<128>* this, unsigned int pos);
