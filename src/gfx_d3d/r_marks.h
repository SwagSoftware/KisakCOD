#pragma once

#include "r_gfx.h"
#include <EffectsCore/fx_system.h>
#include <xanim/xanim.h>

struct FxWorldMarkPoint // sizeof=0x18
{                                       // ...
    float xyz[3];
    float vertWeights[3];
};

struct FxModelMarkPoint // sizeof=0x18
{                                       // ...
    float xyz[3];                       // ...
    float vertWeights[3];               // ...
};

void __cdecl DObjSkelMatToMatrix43(const DObjSkelMat *inSkelMat, float (*outMatrix)[3]);
void  R_BoxSurfaces(
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    uint32_t surfListSize,
    uint32_t *surfCounts,
    uint32_t listCount);
void __cdecl R_BoxSurfaces_r(
    mnode_t *node,
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    uint32_t surfListSize,
    uint32_t *surfCounts,
    uint32_t listCount,
    uint8_t *cellBits);
void __cdecl R_CellSurfaces(
    int cellIndex,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    uint32_t surfListSize,
    uint32_t *surfCount,
    uint8_t *cellBits);
void __cdecl R_AABBTreeSurfaces_r(
    GfxAabbTree *tree,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    uint32_t surfListSize,
    uint32_t *surfCount);
void __cdecl R_AddSurfaceToList(GfxSurface *surf, GfxSurface **surfList, uint32_t *surfCount);
void __cdecl R_CellSurfacesTwoLists(
    int cellIndex,
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    uint32_t surfListSize,
    uint32_t *surfCounts,
    uint8_t *cellBits);
void __cdecl R_AABBTreeSurfacesTwoLists_r(
    GfxAabbTree *tree,
    const float *mins,
    const float *maxs,
    int(__cdecl **allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface ***surfLists,
    uint32_t surfListSize,
    uint32_t *surfCounts);
int  R_BoxStaticModels(
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    uint16_t *smodelList,
    int smodelListSize);
void __cdecl R_BoxStaticModels_r(
    mnode_t *node,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    uint16_t *smodelList,
    int smodelListSize,
    int *smodelCount,
    uint8_t *cellBits);
void __cdecl R_CellStaticModels(
    int cellIndex,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    uint16_t *smodelList,
    int smodelListSize,
    int *smodelCount,
    uint8_t *cellBits);
void __cdecl R_AABBTreeStaticModels_r(
    GfxAabbTree *tree,
    const float *mins,
    const float *maxs,
    int(__cdecl *allowSModel)(int),
    uint16_t *smodelList,
    int smodelListSize,
    int *smodelCount);
void __cdecl R_AddStaticModelToList(int smodelIndex, uint16_t *smodelList, int *smodelCount);
uint32_t  R_CylinderSurfaces(
    const float *start,
    const float *end,
    float radius,
    const DpvsPlane *planes,
    uint32_t planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    uint32_t surfListSize);
void __cdecl R_CylinderSurfaces_r(
    mnode_t *node,
    const float *start,
    const float *end,
    float radius,
    const DpvsPlane *planes,
    uint32_t planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    uint32_t surfListSize,
    uint32_t *surfCount,
    uint8_t *cellBits);
void __cdecl R_CellCylinderSurfaces(
    int cellIndex,
    const DpvsPlane *planes,
    uint32_t planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    uint32_t surfListSize,
    uint32_t *surfCount,
    uint8_t *cellBits);
int __cdecl R_OutsideFrustumPlanes(const DpvsPlane *planes, uint32_t planeCount, const float *minmax);
void __cdecl R_AABBTreeCylinderSurfaces_r(
    GfxAabbTree *tree,
    const DpvsPlane *planes,
    uint32_t planeCount,
    int(__cdecl *allowSurf)(int, void *),
    void *callbackContext,
    GfxSurface **surfList,
    uint32_t surfListSize,
    uint32_t *surfCount);
void __cdecl R_MarkUtil_GetDObjAnimMatAndHideParts(
    const DObj_s *dobj,
    const cpose_t *pose,
    const DObjAnimMat **outBoneMtxList,
    uint32_t *outHidePartBits);
void __cdecl R_MarkFragments_Begin(
    MarkInfo *markInfo,
    MarkFragmentsAgainstEnum markAgainst,
    const float *origin,
    const float (*axis)[3],
    float radius,
    const float *viewOffset,
    Material *material);
void __cdecl R_GetMarkFragmentBounds(
    const float *origin,
    const float (*axis)[3],
    float radius,
    float *mins,
    float *maxs);
void __cdecl R_GetMarkFragmentClipPlanes(const float *origin, const float (*axis)[3], float radius, float (*planes)[4]);
char __cdecl R_MarkFragments_AddDObj(MarkInfo *markInfo, DObj_s *dObj, cpose_t *pose, uint16_t entityIndex);
char __cdecl R_MarkFragments_AddBModel(
    MarkInfo *markInfo,
    GfxBrushModel *brushModel,
    cpose_t *pose,
    uint16_t entityIndex);
void __cdecl R_MarkFragments_Go(
    MarkInfo *markInfo,
    void(__cdecl *callback)(void *, int, FxMarkTri *, int, FxMarkPoint *, const float *, const float *),
    void *callbackContext,
    int maxTris,
    FxMarkTri *tris,
    int maxPoints,
    FxMarkPoint *points);
char __cdecl R_MarkFragments_WorldBrushes(MarkInfo *markInfo);
bool __cdecl R_AllowMarks(int surfIndex, const Material *markMaterialAsVoid);
bool __cdecl R_Mark_MaterialAllowsMarks(const Material *markReceiverMaterialHandle, const Material *markMaterialHandle);
bool __cdecl R_MarkFragments_BrushSurface(
    MarkInfo *markInfo,
    GfxMarkContext *markContext,
    const float (*clipPlanes)[4],
    const float *markDir,
    const GfxSurface *surface,
    bool *anyMarks);
int __cdecl R_ChopWorldPolyBehindPlane(
    int inPointCount,
    FxWorldMarkPoint *inPoints,
    FxWorldMarkPoint *outPoints,
    const float *plane);
bool __cdecl R_MarkFragment_IsTriangleRejected(
    const float *markNormal,
    const float *xyz0,
    const float *xyz1,
    const float *xyz2);
void __cdecl R_MarkFragment_SetupWorldClipPoints(
    const GfxWorldVertex *triVerts0,
    const uint16_t *indices,
    FxWorldMarkPoint(*clipPoints)[9]);
char __cdecl R_MarkFragments_EntBrushes(MarkInfo *markInfo);
void __cdecl R_Mark_TransformClipPlanes(const float (*inClipPlanes)[4], float (*matrix)[3], float (*outClipPlanes)[4]);
char __cdecl R_MarkFragments_Models(MarkInfo *markInfo);
char __cdecl R_MarkFragments_SceneDObjs(MarkInfo *markInfo);
char __cdecl R_MarkFragments_AnimatedXModel(
    MarkInfo *markInfo,
    const XModel *model,
    const uint32_t *hidePartBits,
    int boneIndex,
    const DObjAnimMat *boneMtxList,
    int boneCount,
    GfxMarkContext *markContext);
char  R_MarkFragments_AnimatedXModel_VertList(
    MarkInfo *markInfo,
    uint32_t vertListIndex,
    const DObjAnimMat *poseBone,
    const DObjAnimMat *baseBone,
    GfxMarkContext *markContext,
    XSurface *surface);
char __cdecl R_MarkFragments_StaticModels(MarkInfo *markInfo);
char __cdecl R_MarkFragments_EntirelyRigidXModel(
    MarkInfo *markInfo,
    const XModel *xmodel,
    const float (*modelAxis)[3],
    const float *modelOrigin,
    float modelScale,
    GfxMarkContext *markContext);
char __cdecl R_MarkFragments_XModelSurface_Basic(
    MarkInfo *markInfo,
    const XSurface *surface,
    const float (*modelAxis)[3],
    const float *modelOrigin,
    float modelScale,
    GfxMarkContext *markContext);
int __cdecl R_ChopPolyBehindPlane(
    int inPointCount,
    FxModelMarkPoint *inPoints,
    FxModelMarkPoint *outPoints,
    const float *plane);
void __cdecl R_LerpModelMarkPoints(
    const FxModelMarkPoint *from,
    const FxModelMarkPoint *to,
    float lerp,
    FxModelMarkPoint *out);
