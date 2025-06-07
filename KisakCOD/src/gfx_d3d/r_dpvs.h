#pragma once
#include "r_gfx.h"
#include "r_init.h"
#include "r_rendercmds.h"
#include "r_scene.h"

enum DpvsClipChildren : __int32
{                                       // ...
    DPVS_DONT_CLIP_CHILDREN = 0x0,
    DPVS_CLIP_CHILDREN = 0x1,
};

enum DpvsForceBevels : __int32
{                                       // ...
    DPVS_DONT_FORCE_BEVELS = 0x0,
    DPVS_FORCE_BEVELS = 0x1,
};

struct DpvsPlanes // sizeof=0x8
{                                       // ...
    const DpvsPlane *planes;            // ...
    int count;                          // ...
};
struct DpvsClipPlaneSet // sizeof=0x44
{                                       // ...
    const DpvsPlane *planes[16];        // ...
    unsigned int count;                 // ...
};

struct DpvsDynamicCellCmd // sizeof=0xC
{                                       // ...
    const DpvsPlane *planes;            // ...
    unsigned int cellIndex;             // ...
    unsigned __int8 planeCount;         // ...
    unsigned __int8 frustumPlaneCount;  // ...
    unsigned __int16 viewIndex;         // ...
};

struct DpvsStaticCellCmd // sizeof=0xC
{                                       // ...
    const DpvsPlane *planes;            // ...
    const GfxCell *cell;                // ...
    unsigned __int8 planeCount;         // ...
    unsigned __int8 frustumPlaneCount;  // ...
    unsigned __int16 viewIndex;         // ...
};

struct DpvsEntityCmd // sizeof=0x10
{                                       // ...
    GfxSceneEntity *sceneEnt;
    const DpvsPlane *planes;
    unsigned __int16 planeCount;
    unsigned __int16 cellIndex;
    unsigned __int8 *entVisData;
};

struct FilterEntInfo // sizeof=0x10
{                                       // ...
    unsigned int localClientNum;        // ...
    unsigned int entnum;                // ...
    GfxEntCellRefInfo info;             // ...
    unsigned int cellOffset;            // ...
};

union GfxHullPointsPool // sizeof=0x200
{
    GfxHullPointsPool *nextFree;
    float points[64][2];
};

struct PortalHeapNode // sizeof=0x8
{                                       // ...
    GfxPortal *portal;
    float dist;
};

struct DpvsView // sizeof=0x120
{                                       // ...
    unsigned int renderFxFlagsCull;
    DpvsPlane frustumPlanes[14];        // ...
    int frustumPlaneCount;              // ...
};

struct DpvsGlobals // sizeof=0xAE78
{                                       // ...
    DpvsPlane viewPlane;                // ...
    DpvsPlane fogPlane;                 // ...
    DpvsPlane *nearPlane;               // ...
    DpvsPlane *farPlane;                // ...
    const GfxMatrix *viewProjMtx;       // ...
    const GfxMatrix *invViewProjMtx;    // ...
    float viewOrg[4];                   // ...
    int viewOrgIsDir;                   // ...
    int queuedCount;                    // ...
    PortalHeapNode *portalQueue;        // ...
    GfxHullPointsPool *nextFreeHullPoints; // ...
    float cullDist;                     // ...
    DpvsPlane childPlanes[2048];        // ...
    DpvsView views[4][3];               // ...
    unsigned int cameraCellIndex;       // ...
    DpvsPlane *sideFrustumPlanes;       // ...
    unsigned int *entVisBits[4];        // ...
    unsigned int *cellBits;             // ...
    unsigned int cellVisibleBits[32];   // ...
};

void __cdecl TRACK_r_dpvs();
void __cdecl R_FrustumClipPlanes(
    const GfxMatrix *viewProjMtx,
    const float (*sidePlanes)[4],
    int sidePlaneCount,
    DpvsPlane *frustumPlanes);
char *__cdecl R_PortalAssertMsg();
int __cdecl R_CellForPoint(const float *origin);
unsigned int __cdecl R_FindNearestReflectionProbeInCell(
    const GfxWorld *world,
    const GfxCell *cell,
    const float *origin);
unsigned int __cdecl R_FindNearestReflectionProbe(const GfxWorld *world, const float *origin);
unsigned int __cdecl R_CalcReflectionProbeIndex(const float *origin);
void __cdecl R_AddAllSceneEntSurfacesCamera(const GfxViewInfo *viewInfo);
void __cdecl R_AddAllSceneEntSurfacesSunShadow();
void __cdecl R_AddAllSceneEntSurfacesRangeSunShadow(unsigned int partitionIndex);
void __cdecl R_AddAllSceneEntSurfacesSpotShadow(
    const GfxViewInfo *viewInfo,
    unsigned int spotShadowIndex,
    unsigned int primaryLightIndex);
void __cdecl R_AddSceneDObj(unsigned int entnum, unsigned int viewIndex);
void __cdecl R_DrawAllSceneEnt(const GfxViewInfo *viewInfo);
int __cdecl R_DrawBModel(BModelDrawInfo *bmodelInfo, const GfxBrushModel *bmodel, const GfxPlacement *placement);
void __cdecl R_DrawAllDynEnt(const GfxViewInfo *viewInfo);
void __cdecl R_UnfilterEntFromCells(unsigned int localClientNum, unsigned int entnum);
void __cdecl R_UnfilterDynEntFromCells(unsigned int dynEntId, DynEntityDrawType drawType);
void __cdecl R_FilterXModelIntoScene(
    const XModel *model,
    const GfxScaledPlacement *placement,
    unsigned __int16 renderFxFlags,
    unsigned __int16 *cachedLightingHandle);
void __cdecl R_FilterDObjIntoCells(unsigned int localClientNum, unsigned int entnum, float *origin, float radius);
void __cdecl R_FilterEntIntoCells_r(FilterEntInfo *entInfo, mnode_t *node, const float *mins, const float *maxs);
void __cdecl R_AddEntToCell(FilterEntInfo *entInfo, unsigned int cellIndex);
void __cdecl R_FilterBModelIntoCells(unsigned int localClientNum, unsigned int entnum, GfxBrushModel *bmodel);
void __cdecl R_FilterDynEntIntoCells(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs);
void __cdecl R_FilterDynEntIntoCells_r(
    mnode_t *node,
    unsigned int dynEntIndex,
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs);
void __cdecl R_AddDynEntToCell(unsigned int cellIndex, unsigned int dynEntIndex, DynEntityDrawType drawType);
void __cdecl R_FilterEntitiesIntoCells(int cameraCellIndex);
unsigned int __cdecl R_SetVisData(unsigned int viewIndex);
void __cdecl R_AddCellDynBrushSurfacesInFrustumCmd(const DpvsDynamicCellCmd *data);
void __cdecl R_CullDynBrushInCell(unsigned int cellIndex, const DpvsPlane *planes, int planeCount);
void __cdecl R_GenerateShadowMapCasterCells();
void __cdecl R_VisitPortalsNoFrustum(const GfxCell *cell);
unsigned int __cdecl R_PortalClipPlanesNoFrustum(
    DpvsPlane *planes,
    unsigned int vertexCount,
    const float (*winding)[3]);
void __cdecl R_GetSidePlaneNormals(const float (*winding)[3], unsigned int vertexCount, float (*normals)[3]);
GfxPortal *__cdecl R_NextQueuedPortal();
int R_AssertValidQueue();
void __cdecl R_VisitPortalsForCellNoFrustum(
    const GfxCell *cell,
    GfxPortal *parentPortal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    int frustumPlaneCount,
    signed int recursionDepth);
void __cdecl R_EnqueuePortal(GfxPortal *portal);
double __cdecl R_FurthestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane);
bool __cdecl R_ShouldSkipPortal(const GfxPortal *portal, const DpvsPlane *planes, int planeCount);
char __cdecl R_PortalBehindAnyPlane(const GfxPortal *portal, const DpvsPlane *planes, int planeCount);
char __cdecl R_PortalBehindPlane(const GfxPortal *portal, const DpvsPlane *plane);
char __cdecl R_ChopPortalAndAddHullPointsNoFrustum(
    GfxPortal *portal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount);
void __cdecl R_AddVertToPortalHullPoints(GfxPortal *portal, const float *v);
GfxHullPointsPool *__cdecl R_AllocHullPoints();
int __cdecl R_ChopPortal(
    const GfxPortal *portal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    float (*v)[128][3],
    const float (**finalVerts)[3]);
const float (*__cdecl R_ChopPortalWinding(
    const float (*vertsIn)[3],
    int *vertexCount,
    const DpvsPlane *plane,
    float (*vertsOut)[3]))[3];
void __cdecl R_SetCellVisible(const GfxCell *cell);
void __cdecl R_SetAncestorListStatus(GfxPortal *portal, bool isAncestor);
void __cdecl R_AddWorldSurfacesFrustumOnly();
void __cdecl R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
    const GfxCell *cell,
    const DpvsPlane *planes,
    unsigned __int8 planeCount,
    unsigned __int8 frustumPlaneCount);
void __cdecl R_ShowCull();
void __cdecl R_InitSceneData(int localClientNum);
void __cdecl DynEntCl_InitFilter();
void __cdecl R_InitSceneBuffers();
void __cdecl R_ClearDpvsScene();
bool __cdecl R_CullDynamicSpotLightInCameraView();
void __cdecl R_CullDynamicPointLightsInCameraView();
void __cdecl R_SetupWorldSurfacesDpvs(const GfxViewParms *viewParms);
int __cdecl R_AddNearAndFarClipPlanes(DpvsPlane *planes, int planeCount);
void __cdecl R_SetupDpvsForPoint(const GfxViewParms *viewParms);
void __cdecl R_SetViewFrustumPlanes(GfxViewInfo *viewInfo);
void __cdecl R_AddWorldSurfacesDpvs(const GfxViewParms *viewParms, int cameraCellIndex);
void __cdecl R_AddWorldSurfacesPortalWalk(int cameraCellIndex);
void __cdecl R_VisitPortals(const GfxCell *cell, const DpvsPlane *parentPlane, const DpvsPlane *planes, int planeCount);
unsigned int __cdecl R_PortalClipPlanes(
    DpvsPlane *planes,
    unsigned int vertexCount,
    const float (*winding)[3],
    GfxCell *cell,
    DpvsClipChildren *clipChildren);
double __cdecl R_NearestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane);
void __cdecl R_ProjectPortal(
    int vertexCount,
    const float (*winding)[3],
    float *mins,
    float *maxs,
    DpvsClipChildren *clipChildren);
unsigned int __cdecl R_AddBevelPlanes(
    DpvsPlane *planes,
    unsigned int vertexCount,
    const float (*winding)[3],
    const float (*windingNormals)[3],
    float *mins,
    float *maxs,
    DpvsForceBevels forceBevels);
void __cdecl R_UnprojectPoint(const float *projected, float *unprojected);
void __cdecl R_VisitPortalsForCell(
    const GfxCell *cell,
    GfxPortal *parentPortal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    int frustumPlaneCount,
    signed int recursionDepth,
    DpvsClipChildren clipChildren);
char __cdecl R_ChopPortalAndAddHullPoints(
    GfxPortal *portal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount);
void __cdecl R_VisitAllFurtherCells(
    const GfxCell *cell,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    unsigned __int8 frustumPlaneCount);
int __cdecl R_GetFurtherCellList_r(
    const GfxCell *cell,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    float (*v)[128][3],
    const GfxCell **list,
    int count);
char __cdecl R_IsCellInList(const GfxCell *cell, const GfxCell **list, int count);
int __cdecl R_AddCellToList(const GfxCell *cell, const GfxCell **list, int count);
void __cdecl R_SetupShadowSurfacesDpvs(
    const GfxViewParms *viewParms,
    const float (*sidePlanes)[4],
    unsigned int sidePlaneCount,
    int partitionIndex);
double __cdecl R_GetFarPlaneDist();
unsigned int __cdecl R_CalcReflectionProbeIndex(const GfxWorld *world, const float *origin);
unsigned int __cdecl R_FindNearestReflectionProbeInCell(
    const GfxWorld *world,
    const GfxCell *cell,
    const float *origin);
unsigned int __cdecl R_FindNearestReflectionProbe(const GfxWorld *world, const float *origin);
int __cdecl R_CellForPoint(const GfxWorld *world, const float *origin);

void __cdecl R_FreeHullPoints(GfxHullPointsPool *hullPoints);



// r_dpvs_entity
void __cdecl R_AddEntitySurfacesInFrustumCmd(unsigned __int16 *data);
bool __cdecl R_BoundsInCell(mnode_t *node, int findCellIndex, const float *mins, const float *maxs);
bool __cdecl R_BoundsInCell_r(mnode_t *node, int findCellIndex, const float *mins, const float *maxs);


// r_dvps_dynmodel
void __cdecl R_AddCellDynModelSurfacesInFrustumCmd(const DpvsDynamicCellCmd *data);
void __cdecl R_CullDynModelInCell(
    const unsigned int *dynEntCellBits,
    unsigned int dynEntClientWordCount,
    DynEntityPose *dynModelList,
    const DpvsPlane *planes,
    int planeCount,
    unsigned __int8 *dynEntVisData);


// r_dpvs_sceneent
void R_AddCellSceneEntSurfacesInFrustumCmd(GfxWorldDpvsPlanes *data);

// r_dpvs_static
void __cdecl R_AddCellStaticSurfacesInFrustumCmd(DpvsStaticCellCmd *data);

extern DpvsGlobals dpvsGlob;

extern thread_local DpvsView *g_dpvsView;       // +8 
extern thread_local int g_viewIndex;            // +12(0xC)
extern thread_local EntVisData g_dynEntVisData; // +16(0x10)
extern thread_local byte *g_smodelVisData;      // +24(0x18)
extern thread_local byte *g_surfaceVisData;     // +28(0x1C)