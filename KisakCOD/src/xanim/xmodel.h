#pragma once
#include "xanim.h"

enum XModelLodRampType : __int32
{                                       // ...
    XMODEL_LOD_RAMP_RIGID = 0x0,
    XMODEL_LOD_RAMP_SKINNED = 0x1,
    XMODEL_LOD_RAMP_COUNT = 0x2,
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
    const XSurfaceCollisionTree *tree;  // ...
    const unsigned __int16 *inIndices;  // ...
    const GfxPackedVertex *inVertices0; // ...
    bool(__cdecl *visitorFunc)(void *, const GfxPackedVertex **, const GfxPackedVertex **); // ...
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
    XSurface *surfs;                    // ...
    int partBits[4];                    // ...
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

// xmodel
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
    const XSurface *surface,
    unsigned int vertListIndex,
    const float *aabbMins,
    const float *aabbMaxs,
    bool(__cdecl *visitorFunc)(void *, const GfxPackedVertex **, const GfxPackedVertex **),
    void *visitorContext);
void __cdecl XSurfaceVisitTrianglesInAabb_ConvertAabb(
    const XSurfaceCollisionTree *tree,
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
int __cdecl XModelGetSurfaces(const XModel *model, XSurface **surfaces, int lod);
XSurface *__cdecl XModelGetSurface(const XModel *model, int lod, int surfIndex);
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
