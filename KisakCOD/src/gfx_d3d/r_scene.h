#pragma once

#include <xanim/xanim.h>

#include "r_rendercmds.h"

struct GfxSceneParms // sizeof=0xA0
{                                       // ...
    int localClientNum;
    float blurRadius;
    GfxDepthOfField dof;
    GfxFilm film;
    GfxGlow glow;
    bool isRenderingFullScreen;
    // padding byte
    // padding byte
    // padding byte
    GfxViewport sceneViewport;
    GfxViewport displayViewport;
    GfxViewport scissorViewport;
    const GfxLight *primaryLights;
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
struct GfxLodRamp // sizeof=0x8
{                                       // ...
    float scale;                        // ...
    float bias;                         // ...
};
struct __declspec(align(4)) GfxLodParms // sizeof=0x20
{                                       // ...
    float origin[3];                    // ...
    GfxLodRamp ramp[2];                 // ...
    bool valid;                         // ...
    // padding byte
    // padding byte
    // padding byte
};
struct GfxSkinnedXModelSurfs // sizeof=0x4
{                                       // ...
    void *firstSurf;
};
struct GfxSceneEntityCull // sizeof=0x40
{                                       // ...
    volatile unsigned int state;
    float mins[3];
    float maxs[3];
    char lods[32];
    GfxSkinnedXModelSurfs skinnedSurfs;
};

struct cpose_t;

union GfxSceneEntityInfo // sizeof=0x4
{                                       // ...
    const cpose_t *pose;
    unsigned __int16 *cachedLightingHandle;
};
struct __declspec(align(4)) GfxSceneEntity // sizeof=0x7C
{                                       // ...
    float lightingOrigin[3];
    GfxScaledPlacement placement;
    GfxSceneEntityCull cull;
    unsigned __int16 gfxEntIndex;
    unsigned __int16 entnum;
    const DObj_s *obj;
    GfxSceneEntityInfo info;
    unsigned __int8 reflectionProbeIndex;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxVisibleLight // sizeof=0x2008
{                                       // ...
    int drawSurfCount;                  // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxDrawSurf drawSurfs[1024];        // ...
};

struct GfxShadowCookie // sizeof=0x868
{                                       // ...
    DpvsPlane planes[5];                // ...
    volatile int drawSurfCount;         // ...
    GfxDrawSurf drawSurfs[256];         // ...
};

struct __declspec(align(4)) GfxSceneModel // sizeof=0x48
{                                       // ...
    XModelDrawInfo info;
    const XModel *model;
    const DObj_s *obj;
    GfxScaledPlacement placement;
    unsigned __int16 gfxEntIndex;
    unsigned __int16 entnum;
    float radius;
    unsigned __int16 *cachedLightingHandle;
    float lightingOrigin[3];
    unsigned __int8 reflectionProbeIndex;
    // padding byte
    // padding byte
    // padding byte
};
struct __declspec(align(4)) GfxSceneBrush // sizeof=0x28
{                                       // ...
    BModelDrawInfo info;
    unsigned __int16 entnum;
    const GfxBrushModel *bmodel;
    GfxPlacement placement;
    unsigned __int8 reflectionProbeIndex;
    // padding byte
    // padding byte
    // padding byte
};

union GfxEntCellRefInfo // sizeof=0x4
{                                       // ...
    float radius;
    GfxBrushModel *bmodel;
};

struct GfxSceneDpvs // sizeof=0x38
{                                       // ...
    unsigned int localClientNum;        // ...
    unsigned __int8 *entVisData[7];     // ...
    unsigned __int16 *sceneXModelIndex; // ...
    unsigned __int16 *sceneDObjIndex;   // ...
    GfxEntCellRefInfo *entInfo[4];      // ...
};

struct __declspec(align(64)) GfxScene // sizeof=0x154D00
{                                       // ...
    GfxDrawSurf bspDrawSurfs[8192];
    GfxDrawSurf smodelDrawSurfsLight[8192]; // ...
    GfxDrawSurf entDrawSurfsLight[8192]; // ...
    GfxDrawSurf bspDrawSurfsDecal[512]; // ...
    GfxDrawSurf smodelDrawSurfsDecal[512]; // ...
    GfxDrawSurf entDrawSurfsDecal[512]; // ...
    GfxDrawSurf bspDrawSurfsEmissive[8192]; // ...
    GfxDrawSurf smodelDrawSurfsEmissive[8192]; // ...
    GfxDrawSurf entDrawSurfsEmissive[8192]; // ...
    GfxDrawSurf fxDrawSurfsEmissive[8192]; // ...
    GfxDrawSurf fxDrawSurfsEmissiveAuto[8192]; // ...
    GfxDrawSurf fxDrawSurfsEmissiveDecal[8192]; // ...
    GfxDrawSurf bspSunShadowDrawSurfs0[4096]; // ...
    GfxDrawSurf smodelSunShadowDrawSurfs0[4096]; // ...
    GfxDrawSurf entSunShadowDrawSurfs0[4096]; // ...
    GfxDrawSurf bspSunShadowDrawSurfs1[8192]; // ...
    GfxDrawSurf smodelSunShadowDrawSurfs1[8192]; // ...
    GfxDrawSurf entSunShadowDrawSurfs1[8192]; // ...
    GfxDrawSurf bspSpotShadowDrawSurfs0[256]; // ...
    GfxDrawSurf smodelSpotShadowDrawSurfs0[256]; // ...
    GfxDrawSurf entSpotShadowDrawSurfs0[512]; // ...
    GfxDrawSurf bspSpotShadowDrawSurfs1[256]; // ...
    GfxDrawSurf smodelSpotShadowDrawSurfs1[256]; // ...
    GfxDrawSurf entSpotShadowDrawSurfs1[512]; // ...
    GfxDrawSurf bspSpotShadowDrawSurfs2[256]; // ...
    GfxDrawSurf smodelSpotShadowDrawSurfs2[256]; // ...
    GfxDrawSurf entSpotShadowDrawSurfs2[512]; // ...
    GfxDrawSurf bspSpotShadowDrawSurfs3[256]; // ...
    GfxDrawSurf smodelSpotShadowDrawSurfs3[256]; // ...
    GfxDrawSurf entSpotShadowDrawSurfs3[512]; // ...
    GfxDrawSurf shadowDrawSurfs[512];   // ...
    unsigned int shadowableLightIsUsed[32]; // ...
    int maxDrawSurfCount[34];           // ...
    volatile int drawSurfCount[34];     // ...
    GfxDrawSurf *drawSurfs[34];         // ...
    GfxDrawSurf fxDrawSurfsLight[8192]; // ...
    GfxDrawSurf fxDrawSurfsLightAuto[8192]; // ...
    GfxDrawSurf fxDrawSurfsLightDecal[8192]; // ...
    GfxSceneDef def;                    // ...
    int addedLightCount;                // ...
    GfxLight addedLight[32];            // ...
    bool isAddedLightCulled[32];        // ...
    float dynamicSpotLightNearPlaneOffset; // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxVisibleLight visLight[4];        // ...
    GfxVisibleLight visLightShadow[1];  // ...
    GfxShadowCookie cookie[24];         // ...
    unsigned int *entOverflowedDrawBuf; // ...
    volatile unsigned int sceneDObjCount;        // ...
    GfxSceneEntity sceneDObj[512];      // ...
    unsigned __int8 sceneDObjVisData[7][512]; // ...
    volatile unsigned int sceneModelCount;       // ...
    GfxSceneModel sceneModel[1024];     // ...
    unsigned __int8 sceneModelVisData[7][1024]; // ...
    volatile unsigned int sceneBrushCount;       // ...
    GfxSceneBrush sceneBrush[512];      // ...
    unsigned __int8 sceneBrushVisData[3][512]; // ...
    unsigned unsigned int sceneDynModelCount;    // ...
    unsigned unsigned int sceneDynBrushCount;    // ...
    DpvsPlane shadowFarPlane[2];        // ...
    DpvsPlane shadowNearPlane[2];       // ...
    GfxSceneDpvs dpvs;                  // ...
};

void __cdecl TRACK_r_scene();
unsigned int __cdecl R_AllocSceneDObj();
unsigned int __cdecl R_AllocSceneModel();
unsigned int __cdecl R_AllocSceneBrush();
GfxBrushModel *__cdecl R_GetBrushModel(unsigned int modelIndex);
void __cdecl R_AddBrushModelToSceneFromAngles(
    const GfxBrushModel *bmodel,
    const float *origin,
    const float *angles,
    unsigned __int16 entnum);
void __cdecl R_AddDObjToScene(
    const DObj_s *obj,
    const cpose_t *pose,
    unsigned int entnum,
    unsigned int renderFxFlags,
    float *lightingOrigin,
    float materialTime);
GfxParticleCloud *__cdecl R_AddParticleCloudToScene(Material *material);
void __cdecl R_AddOmniLightToScene(const float *org, float radius, float r, float g, float b);
void __cdecl R_AddSpotLightToScene(const float *org, const float *dir, float radius, float r, float g, float b);
double __cdecl R_GetDefaultNearClip();
void __cdecl R_SetupViewProjectionMatrices(GfxViewParms *viewParms);
void __cdecl R_AddBModelSurfacesCamera(
    BModelDrawInfo *bmodelInfo,
    const GfxBrushModel *bmodel,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs,
    unsigned int reflectionProbeIndex);
GfxDrawSurf *__cdecl R_AddBModelSurfaces(
    BModelDrawInfo *bmodelInfo,
    const GfxBrushModel *bmodel,
    MaterialTechniqueType techType,
    GfxDrawSurf *drawSurf,
    GfxDrawSurf *lastDrawSurf);
const XSurface *__cdecl R_GetXSurface(unsigned int *modelSurf, surfaceType_t surfType);
void __cdecl R_AddXModelSurfacesCamera(
    XModelDrawInfo *modelInfo,
    const XModel *model,
    float *origin,
    unsigned __int16 gfxEntIndex,
    unsigned int lightingHandle,
    unsigned __int8 primaryLightIndex,
    char isShadowReceiver,
    int depthHack,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs,
    unsigned int reflectionProbeIndex);
void __cdecl R_AddXModelDebugString(const float *origin, char *string);
GfxDrawSurf *__cdecl R_AddXModelSurfaces(
    XModelDrawInfo *modelInfo,
    const XModel *model,
    MaterialTechniqueType techType,
    GfxDrawSurf *drawSurf,
    GfxDrawSurf *lastDrawSurf);
void __cdecl R_AddDObjSurfacesCamera(
    GfxSceneEntity *sceneEnt,
    __int16 lightingHandle,
    unsigned __int8 primaryLightIndex,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs);
GfxDrawSurf *__cdecl R_AddDObjSurfaces(
    GfxSceneEntity *sceneEnt,
    MaterialTechniqueType techType,
    GfxDrawSurf *drawSurf,
    GfxDrawSurf *lastDrawSurf);
bool __cdecl R_EndFencePending();
void __cdecl R_SetEndTime(int endTime);
void __cdecl R_WaitEndTime();
void __cdecl R_InitScene();
void __cdecl R_ClearScene(unsigned int localClientNum);
unsigned int __cdecl R_GetLocalClientNum();
void __cdecl R_SetLodOrigin(const refdef_s *refdef);
unsigned __int8 R_UpdateFrameFog();
unsigned __int8 __cdecl LerpByte(unsigned __int8 from, unsigned __int8 to, float frac);
void __cdecl R_SetViewParmsForScene(const refdef_s *refdef, GfxViewParms *viewParms);
void __cdecl R_SetupProjection(float tanHalfFovX, float tanHalfFovY, GfxViewParms *viewParms);
bool R_UpdateFrameSun();
void __cdecl R_LerpDir(
    const float *dirBegin,
    const float *dirEnd,
    int beginLerpTime,
    int endLerpTime,
    int currTime,
    float *result);
void __cdecl R_UpdateLodParms(const refdef_s *refdef, GfxLodParms *lodParms);
void __cdecl R_CorrectLodScale(const refdef_s *refdef);
void __cdecl R_RenderScene(const refdef_s *refdef);
void __cdecl R_GenerateSortedDrawSurfs(
    const GfxSceneParms *sceneParms,
    const GfxViewParms *viewParmsDpvs,
    const GfxViewParms *viewParmsDraw);
bool __cdecl R_GetAllowShadowMaps();
int __cdecl R_DynamicShadowType();
void __cdecl R_SetDepthOfField(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetFilmInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_UpdateColorManipulation(GfxViewInfo *viewInfo);
void __cdecl R_SetGlowInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms);
void __cdecl R_SetFullSceneViewMesh(int viewInfoIndex, GfxViewInfo *viewInfo);
void __cdecl R_GenerateSortedSunShadowDrawSurfs(GfxViewInfo *viewInfo);
void __cdecl R_AddEmissiveSpotLight(GfxViewInfo *viewInfo);
void R_GenerateMarkVertsForDynamicModels();
int __cdecl R_GetVisibleDLights(const GfxLight **visibleLights);
void __cdecl R_GetLightSurfs(int visibleLightCount, const GfxLight **visibleLights);
void __cdecl R_GetPointLightShadowSurfs(GfxViewInfo *viewInfo, GfxVisibleLight *visibleLights, const GfxLight **lights);
MaterialTechniqueType __cdecl R_GetEmissiveTechnique(const GfxViewInfo *viewInfo, MaterialTechniqueType baseTech);
void __cdecl R_SetSunShadowConstants(GfxCmdBufInput *input, const GfxSunShadowProjection *sunProj);
void __cdecl R_SetSunConstants(GfxCmdBufInput *input);
void R_DrawCineWarning();
void __cdecl R_SetSceneParms(const refdef_s *refdef, GfxSceneParms *sceneParms);
void __cdecl R_LinkDObjEntity(unsigned int localClientNum, unsigned int entnum, float *origin, float radius);
void __cdecl R_LinkBModelEntity(unsigned int localClientNum, unsigned int entnum, GfxBrushModel *bmodel);
void __cdecl R_UnlinkEntity(unsigned int localClientNum, unsigned int entnum);
void __cdecl R_LinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs);
void __cdecl R_UnlinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType);


extern GfxScene scene;
