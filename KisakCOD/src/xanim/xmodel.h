#pragma once
#include <universal/com_math.h>
#include <physics/phys_local.h>

enum XModelLodRampType : __int32
{                                       // ...
    XMODEL_LOD_RAMP_RIGID = 0x0,
    XMODEL_LOD_RAMP_SKINNED = 0x1,
    XMODEL_LOD_RAMP_COUNT = 0x2,
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

struct XModel // sizeof=0xDC
{                                       // ...
    const char* name;
    unsigned __int8 numBones;
    unsigned __int8 numRootBones;
    unsigned __int8 numsurfs;
    unsigned __int8 lodRampType;
    unsigned __int16* boneNames;
    unsigned __int8* parentList;
    __int16* quats;
    float* trans;
    unsigned __int8* partClassification;
    DObjAnimMat* baseMat;
    struct XSurface* surfs;
    Material** materialHandles;
    XModelLodInfo lodInfo[4];
    XModelCollSurf_s* collSurfs;
    int numCollSurfs;
    int contents;
    XBoneInfo* boneInfo;
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
    struct PhysPreset* physPreset;
    struct PhysGeomList* physGeoms;
};

struct XModelPiece // sizeof=0x10
{
    XModel *model;
    float offset[3];
};

struct XModelPieces // sizeof=0xC
{                                       // ...
    const char *name;
    int numpieces;
    XModelPiece *pieces;
};

struct QueueElement // sizeof=0x8
{                                       // ...
    unsigned int beginIndex;            // ...
    unsigned int count;                 // ...
};

struct XSurfaceGetTriCandidatesLocals // sizeof=0x2A4
{                                       // ...
    int mins[3];
    int maxs[3];                        // ...
    const struct XSurfaceCollisionTree *tree;  // ...
    const unsigned __int16 *inIndices;  // ...
    const struct GfxPackedVertex *inVertices0; // ...
    bool(__cdecl *visitorFunc)(void *, const struct GfxPackedVertex **, const struct GfxPackedVertex **); // ...
    void *visitorContext;               // ...
    unsigned int nodeQueueBegin;        // ...
    unsigned int nodeQueueEnd;          // ...
    unsigned int leafQueueBegin;        // ...
    unsigned int leafQueueEnd;          // ...
    unsigned int triangleQueueBegin;    // ...
    unsigned int triangleQueueEnd;      // ...
    unsigned int vertexQueueBegin;      // ...
    unsigned int vertexQueueEnd;        // ...
    QueueElement nodeQueue[64];         // ...
    QueueElement leafQueue[4];
    QueueElement triangleQueue[4];
    unsigned __int16 vertexQueue[4][3];
};

struct XModelSurfs // sizeof=0x14
{                                       // ...
    struct XSurface *surfs;                    // ...
    int partBits[4];                    // ...
};
struct XModelConfigEntry // sizeof=0x404
{                                       // ...
    char filename[1024];                // ...
    float dist;                         // ...
};
struct __declspec(align(4)) XModelConfig // sizeof=0x1430
{                                       // ...
    XModelConfigEntry entries[4];       // ...
    float mins[3];
    float maxs[3];                      // ...
    int collLod;                        // ...
    unsigned __int8 flags;              // ...
    char physicsPresetFilename[1024];   // ...
    // padding byte
    // padding byte
    // padding byte
};
struct XModelPartsLoad // sizeof=0x1C
{                                       // ...
    unsigned __int8 numBones;
    unsigned __int8 numRootBones;
    // padding byte
    // padding byte
    unsigned __int16 *boneNames;
    unsigned __int8 *parentList;
    __int16 *quats;
    float *trans;
    unsigned __int8 *partClassification;
    DObjAnimMat *baseMat;
};
struct XModelDefault // sizeof=0x4C
{                                       // ...
    unsigned __int16 boneNames[1];
    unsigned __int8 parentList[1];      // ...
    // padding byte
    XModelPartsLoad modelParts;         // ...
    XBoneInfo boneInfo;                 // ...
    unsigned __int8 partClassification[1]; // ...
    // padding byte
    unsigned __int16 surfNames[1];
};

struct XVertexInfo_s // sizeof=0x40
{                                       // ...
    float normal[3];
    unsigned __int8 color[4];
    float binormal[3];
    float texCoordX;
    float tangent[3];
    float texCoordY;
    float offset[3];
    unsigned __int8 numWeights;
    unsigned __int8 pad;
    __int16 boneOffset;
};
struct XBlendLoadInfo // sizeof=0x4
{                                       // ...
    unsigned __int16 boneOffset;
    unsigned __int16 boneWeight;
};
struct XVertexBuffer // sizeof=0x44
{
    XVertexInfo_s v;
    XBlendLoadInfo w[1];
};

struct XVertexInfo0 // sizeof=0x2
{                                       // ...
    unsigned __int16 boneOffset;
};
struct XVertexInfo3 // sizeof=0xE
{
    XVertexInfo0 vert0;
    XBlendLoadInfo blend[3];
};
struct XVertexInfo2 // sizeof=0xA
{
    XVertexInfo0 vert0;
    XBlendLoadInfo blend[2];
};
struct XVertexInfo1 // sizeof=0x6
{
    XVertexInfo0 vert0;
    XBlendLoadInfo blend[1];
};

// xmodel
void __cdecl XModelPartsFree(XModelPartsLoad *modelParts);
bool __cdecl XModelBad(const XModel *model);
void __cdecl TRACK_xmodel();
XModel *__cdecl XModelPrecache(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));
XModel *__cdecl XModelPrecache_LoadObj(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));
XModel *__cdecl XModelPrecache_FastFile(const char *name);
XModel *__cdecl XModelLoad(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));
XModelPartsLoad *__cdecl XModelPartsPrecache(XModel *model, const char *name, void *(__cdecl *Alloc)(int));
XModel *__cdecl XModelLoadFile(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));

double __cdecl XModelGetRadius(const XModel *model);
void __cdecl XModelGetBounds(const XModel *model, float *mins, float *maxs);
int __cdecl XModelGetMemUsage(const XModel *model);
int __cdecl XModelTraceLine(
    const XModel *model,
    trace_t *results,
    const float *localStart,
    const float *localEnd,
    int contentmask);
int __cdecl XModelTraceLineAnimated(
    const DObj_s *obj,
    unsigned int modelIndex,
    int baseBoneIndex,
    trace_t *results,
    const DObjAnimMat *boneMtxList,
    float *localStart,
    float *localEnd,
    int contentmask);
void __cdecl LocalMatrixTransposeTransformVector(const float *in1, const float (*in2)[3], float *out);
void __cdecl XModelTraceLineAnimatedPartBits(
    const DObj_s *obj,
    unsigned int modelIndex,
    int baseBoneIndex,
    int contentmask,
    int *partBits);
char __cdecl XSurfaceVisitTrianglesInAabb(
    const struct XSurface *surface,
    unsigned int vertListIndex,
    const float *aabbMins,
    const float *aabbMaxs,
    bool(__cdecl *visitorFunc)(void *, const struct GfxPackedVertex **, const struct GfxPackedVertex **),
    void *visitorContext);
void __cdecl XSurfaceVisitTrianglesInAabb_ConvertAabb(
    const struct XSurfaceCollisionTree *tree,
    const float *aabbMins,
    const float *aabbMaxs,
    int *mins,
    int *maxs);
int __cdecl XSurfaceVisitTrianglesInAabb_ProcessVertices(XSurfaceGetTriCandidatesLocals *locals);
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessTriangles(XSurfaceGetTriCandidatesLocals *locals);
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessLeaf(XSurfaceGetTriCandidatesLocals *locals);
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessNode(XSurfaceGetTriCandidatesLocals *locals);
int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned int offset, unsigned __int8 *index);
unsigned __int16 *__cdecl XModelBoneNames(XModel *model);


// xmodel_utils
const char *__cdecl XModelGetName(const XModel *model);
int __cdecl XModelGetSurfaces(const XModel *model, struct XSurface **surfaces, int lod);
struct XSurface *__cdecl XModelGetSurface(const XModel *model, int lod, int surfIndex);
const XModelLodInfo *__cdecl XModelGetLodInfo(const XModel *model, int lod);
unsigned int __cdecl XModelGetSurfCount(const XModel *model, int lod);
Material **__cdecl XModelGetSkins(const XModel *model, int lod);
XModelLodRampType __cdecl XModelGetLodRampType(const XModel *model);
int __cdecl XModelGetNumLods(const XModel *model);
double __cdecl XModelGetLodOutDist(const XModel *model);
int __cdecl XModelNumBones(const XModel *model);
const DObjAnimMat *__cdecl XModelGetBasePose(const XModel *model);
int __cdecl XModelGetLodForDist(const XModel *model, float dist);
void __cdecl XModelSetTestLods(unsigned int lodLevel, float dist);
double __cdecl XModelGetLodDist(const XModel *model, unsigned int lod);
int __cdecl XModelGetContents(const XModel *model);
int __cdecl XModelGetStaticModelCacheVertCount(XModel *model, unsigned int lod);



// xmodel_load_obj
int __cdecl XModelGetStaticBounds(const XModel *model, mat3x3 &axis, float *mins, float *maxs);
void __cdecl ConsumeQuatNoSwap(const unsigned __int8 **pos, __int16 *out);
int __cdecl XModelSurfsPrecache(
    XModel *model,
    const char *name,
    void *(__cdecl *Alloc)(int),
    __int16 modelNumsurfs,
    const char *modelName,
    XModelSurfs *outModelSurfs);


// xmodel_load_phys_collmap
PhysGeomList *__cdecl XModel_LoadPhysicsCollMap(const char *name, void *(__cdecl *Alloc)(int));