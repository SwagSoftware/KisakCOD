#pragma once

#include <qcommon/qcommon.h>

#include <universal/memfile.h>

#include <gfx_d3d/fxprimitives.h>
#include <gfx_d3d/r_gfx.h>

void __cdecl TRACK_fx_system();
FxSystem *__cdecl FX_GetSystem(int clientIndex);
FxSystemBuffers *__cdecl FX_GetSystemBuffers(int clientIndex);
void __cdecl FX_LinkSystemBuffers(FxSystem *system, FxSystemBuffers *systemBuffers);
void __cdecl FX_InitSystem(int localClientNum);
void __cdecl FX_ResetSystem(FxSystem *system);
int __cdecl FX_EffectToHandle(FxSystem *system, FxEffect *effect);
void __cdecl FX_ShutdownSystem(int localClientNum);
void __cdecl FX_RelocateSystem(FxSystem *system, int relocationDistance);
void __cdecl FX_EffectNoLongerReferenced(FxSystem *system, FxEffect *remoteEffect);
void __cdecl FX_DelRefToEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_RunGarbageCollection(FxSystem *system);
bool __cdecl FX_BeginIteratingOverEffects_Exclusive(FxSystem *system);
void __cdecl FX_RunGarbageCollection_FreeSpotLight(FxSystem *system, unsigned __int16 effectHandle);
void __cdecl FX_RunGarbageCollection_FreeTrails(FxSystem *system, FxEffect *effect);
void __cdecl FX_SpawnEffect_AllocTrails(FxSystem *system, FxEffect *effect);
FxPool<FxTrail> *__cdecl FX_AllocTrail(FxSystem *system);
unsigned __int16 __cdecl FX_CalculatePackedLighting(const float *origin);
FxEffect *__cdecl FX_SpawnEffect(
    FxSystem *system,
    const FxEffectDef *remoteDef,
    int msecBegin,
    const float *origin,
    const float (*axis)[3],
    int dobjHandle,
    int boneIndex,
    int runnerSortOrder,
    unsigned __int16 owner,
    unsigned int markEntnum);
void __cdecl FX_AddRefToEffect(FxSystem *__formal, FxEffect *effect);
char __cdecl FX_CullEffectForSpawn(const FxCamera *camera, const FxEffectDef *effectDef, const float *origin);
bool __cdecl FX_CullElemForSpawn(const FxCamera *camera, const FxElemDef *elemDef, const float *origin);
void __cdecl FX_SetEffectRandomSeed(FxEffect *effect, const FxEffectDef *remoteDef);
char __cdecl FX_EffectAffectsGameplay(const FxEffectDef *remoteEffectDef);
char __cdecl FX_IsSpotLightEffect(FxSystem *system, const FxEffectDef *def);
bool __cdecl FX_CanAllocSpotLightEffect(const FxSystem *system);
char __cdecl FX_SpawnEffect_AllocSpotLightEffect(FxSystem *system, FxEffect *effect);
FxEffect *__cdecl FX_SpawnOrientedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int msecBegin,
    const float *origin,
    const float (*axis)[3],
    unsigned int markEntnum);
void __cdecl FX_AssertAllocatedEffect(int localClientNum, FxEffect *effect);
void __cdecl FX_PlayOrientedEffectWithMarkEntity(
    int localClientNum,
    const FxEffectDef *def,
    int startMsec,
    const float *origin,
    const float (*axis)[3],
    unsigned int markEntnum);
void __cdecl FX_PlayOrientedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int startMsec,
    const float *origin,
    const float (*axis)[3]);
FxEffect *__cdecl FX_SpawnBoltedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int msecBegin,
    unsigned int dobjHandle,
    unsigned int boneIndex);
char __cdecl FX_NeedsBoltUpdate(const FxEffectDef *def);
void __cdecl FX_PlayBoltedEffect(
    int localClientNum,
    const FxEffectDef *def,
    int startMsec,
    unsigned int dobjHandle,
    unsigned int boneIndex);
void __cdecl FX_RetriggerEffect(int localClientNum, FxEffect *effect, int msecBegin);
void __cdecl FX_GetTrailHandleList_Last(
    FxSystem *system,
    FxEffect *effect,
    unsigned __int16 *outHandleList,
    int *outTrailCount);
void __cdecl FX_ThroughWithEffect(int localClientNum, FxEffect *effect);
void __cdecl FX_StopEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_StopEffectNonRecursive(FxSystem *system, FxEffect *effect);
void __cdecl FX_KillEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_RemoveAllEffectElems(FxSystem *system, FxEffect *effect);
void __cdecl FX_KillEffectDef(int localClientNum, const FxEffectDef *def);
void __cdecl FX_KillAllEffects(int localClientNum);
void __cdecl FX_SpawnTrailElem_NoCull(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int msecWhenPlayed,
    float distanceWhenPlayed);
FxPool<FxTrailElem> *__cdecl FX_AllocTrailElem(FxSystem *system);
void __cdecl FX_SpawnTrailElem_Cull(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int msecWhenPlayed,
    float distanceWhenPlayed);
bool __cdecl FX_CullTrailElem(
    const FxCamera *camera,
    const FxElemDef *elemDef,
    const float *origin,
    unsigned __int8 sequence);
void __cdecl FX_SpawnSpotLightElem(FxSystem *system, FxElem *elem);
void __cdecl FX_SpawnElem(
    FxSystem *system,
    FxEffect *effect,
    int elemDefIndex,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int msecWhenPlayed,
    float distanceWhenPlayed,
    int sequence);
FxPool<FxElem> *__cdecl FX_AllocElem(FxSystem *system);
void __cdecl FX_SpawnRunner(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *remoteElemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed,
    int msecWhenPlayed);
bool __cdecl FX_SpawnModelPhysics(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *elemDef,
    int randomSeed,
    FxElem *elem);
void __cdecl FX_GetOriginForElem(
    FxEffect *effect,
    const FxElemDef *elemDef,
    FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed,
    float *outOrigin);
void __cdecl FX_SpawnSound(
    int localClientNumber,
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed);
void __cdecl FX_FreeElem(FxSystem *system, unsigned __int16 elemHandle, FxEffect *effect, unsigned int elemClass);
void __cdecl FX_FreeTrailElem(FxSystem *system, unsigned __int16 trailElemHandle, FxEffect *effect, FxTrail *trail);
void __cdecl FX_FreeSpotLightElem(FxSystem *system, unsigned __int16 elemHandle, FxEffect *effect);
double __cdecl FX_GetClientVisibility(int localClientNum, const float *start, const float *end);
void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3]);

extern FxSystem fx_systemPool[1];
extern FxSystemBuffers fx_systemBufferPool[1];
extern FxMarksSystem fx_marksSystemPool[1];


// fx_Dvars
void __cdecl FX_RegisterDvars(void);

extern const dvar_t *fx_mark_profile;
extern const dvar_t *fx_marks_smodels;
extern const dvar_t *fx_enable;
extern const dvar_t *fx_cull_effect_spawn;
extern const dvar_t *fx_cull_elem_draw;
extern const dvar_t *fx_freeze;
extern const dvar_t *fx_debugBolt;
extern const dvar_t *fx_count;
extern const dvar_t *fx_visMinTraceDist;
extern const dvar_t *fx_drawClouds;
extern const dvar_t *fx_marks;
extern const dvar_t *fx_draw;
extern const dvar_t *fx_profile;
extern const dvar_t *fx_cull_elem_spawn;
extern const dvar_t *fx_marks_ents;

struct cpose_t;

// fx_marks
enum MarkFragmentsAgainstEnum : __int32
{                                       // ...
    MARK_FRAGMENTS_AGAINST_BRUSHES = 0x0,
    MARK_FRAGMENTS_AGAINST_MODELS = 0x1,
};
struct FxMarkTri // sizeof=0xC
{                                       // ...
    unsigned __int16 indices[3];        // ...
    GfxMarkContext context;             // ...
};
struct __declspec(align(4)) MarkInfoCollidedDObj // sizeof=0xC
{                                       // ...
    DObj_s *dObj;
    cpose_t *pose;
    unsigned __int16 entnum;
    // padding byte
    // padding byte
};
struct __declspec(align(4)) MarkInfoCollidedBModel // sizeof=0xC
{                                       // ...
    GfxBrushModel *brushModel;
    cpose_t *pose;
    unsigned __int16 entnum;
    // padding byte
    // padding byte
};
struct MarkInfo // sizeof=0x448
{                                       // ...
    float origin[3];
    float localOrigin[3];
    float localTexCoordAxis[3];
    float axis[3][3];
    float radius;
    Material *material;
    int maxTris;
    FxMarkTri *tris;
    int maxPoints;
    FxMarkPoint *points;
    float mins[3];
    float maxs[3];
    float planes[6][4];
    float viewOffset[3];
    bool markHasLightmap;
    bool markHasReflection;
    // padding byte
    // padding byte
    MarkFragmentsAgainstEnum markAgainst;
    unsigned __int16 smodelsCollided[32];
    int smodelCollidedCount;
    MarkInfoCollidedDObj sceneDObjsCollided[32];
    int sceneDObjCollidedCount;
    MarkInfoCollidedBModel sceneBModelsCollided[32];
    int sceneBModelCollidedCount;
    int usedTriCount;
    int usedPointCount;
    void(__cdecl *callback)(void *, int, FxMarkTri *, int, FxMarkPoint *, const float *, const float *);
    void *callbackContext;
};
struct FxMarkDObjUpdateContext // sizeof=0x108
{                                       // ...
    XModel *models[32];
    const char *modelParentBones[32];
    int modelCount;
    bool isBrush;
    // padding byte
    unsigned __int16 brushIndex;
};
struct FxActiveMarkSurf // sizeof=0x14
{                                       // ...
    Material *material;
    GfxMarkContext context;
    // padding byte
    // padding byte
    int indexCount;
    unsigned __int16 *indices;
};
void __cdecl TRACK_fx_marks();
void __cdecl FX_InitMarksSystem(FxMarksSystem *marksSystem);
unsigned __int16 __cdecl FX_MarkToHandle(FxMarksSystem *marksSystem, FxMark *mark);
void __cdecl FX_BeginMarks(int clientIndex);
void __cdecl FX_CreateImpactMark(
    int localClientNum,
    const FxElemDef *elemDef,
    const FxSpatialFrame *spatialFrame,
    int randomSeed,
    unsigned int markEntnum);
void __cdecl FX_ImpactMark(
    int localClientNum,
    Material *worldMaterial,
    Material *modelMaterial,
    const float *origin,
    const float *quat,
    float orientation,
    const unsigned __int8 *nativeColor,
    float radius,
    unsigned int markEntnum);
void __cdecl FX_ImpactMark_Generate(
    int localClientNum,
    MarkFragmentsAgainstEnum markAgainst,
    Material *material,
    float *origin,
    const float (*axis)[3],
    float orientation,
    const unsigned __int8 *nativeColor,
    float radius,
    unsigned int markEntnum);
void __cdecl FX_ImpactMark_Generate_AddEntityBrush(
    int localClientNum,
    MarkInfo *markInfo,
    unsigned int entityIndex,
    const float *origin,
    float radius);
void __cdecl FX_ImpactMark_Generate_AddEntityModel(
    int localClientNum,
    MarkInfo *markInfo,
    unsigned int entityIndex,
    const float *origin,
    float radius);
void __cdecl FX_ImpactMark_Generate_Callback(
    void *context,
    int triCount,
    FxMarkTri *tris,
    int pointCount,
    FxMarkPoint *points,
    const float *markOrigin,
    const float *markTexCoordAxis);
void __cdecl FX_AllocAndConstructMark(
    int localClientNum,
    int triCount,
    int pointCount,
    Material *material,
    FxMarkTri *markTris,
    const FxMarkPoint *markPoints,
    const float *origin,
    float radius,
    const float *texCoordAxis,
    const unsigned __int8 *nativeColor);
FxMark *__cdecl FX_MarkFromHandle(FxMarksSystem *marksSystem, unsigned __int16 handle);
void __cdecl FX_FreeLruMark(FxMarksSystem *marksSystem);
void __cdecl FX_FreeMark(FxMarksSystem *marksSystem, FxMark *mark);
void __cdecl FX_FreeMarkFromList(FxMarksSystem *marksSystem, FxMark *mark, unsigned __int16 *listHead);
void __cdecl FX_FreeMarkTriGroups(FxMarksSystem *marksSystem, FxMark *mark);
FxTriGroupPool *__cdecl FX_TriGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle);
void __cdecl FX_FreeMarkPointGroups(FxMarksSystem *marksSystem, FxMark *mark);
FxPointGroupPool *__cdecl FX_PointGroupFromHandle(FxMarksSystem *marksSystem, unsigned int handle);
int __cdecl FX_AllocMarkTris(FxMarksSystem *marksSystem, const FxMarkTri *markTris, int triCount);
int __cdecl FX_TriGroupToHandle(FxMarksSystem *marksSystem, FxTriGroup *group);
int __cdecl FX_AllocMarkPoints(FxMarksSystem *marksSystem, int pointCount);
int __cdecl FX_PointGroupToHandle(FxMarksSystem *marksSystem, FxPointGroup *group);
void __cdecl FX_LinkMarkIntoList(FxMarksSystem *marksSystem, unsigned __int16 *head, FxMark *mark);
void __cdecl FX_CopyMarkTris(
    FxMarksSystem *marksSystem,
    const FxMarkTri *srcTris,
    unsigned int dstGroupHandle,
    int triCount);
void __cdecl FX_CopyMarkPoints(
    FxMarksSystem *marksSystem,
    const FxMarkPoint *srcPoints,
    unsigned int dstGroupHandle,
    int pointCount);
unsigned __int16 __cdecl FX_FindModelHead(FxMarksSystem *marksSystem, unsigned __int16 modelIndex, int type);
bool __cdecl FX_CompareMarkTris(const FxMarkTri *tri0, const FxMarkTri *tri1);
int __cdecl FX_MarkContextsCompare(const GfxMarkContext *context0, const GfxMarkContext *context1);
void __cdecl FX_MarkEntDetachAll(int localClientNum, int entnum);
void __cdecl FX_MarkEntUpdateHidePartBits(
    const unsigned int *oldHidePartBits,
    const unsigned int *newHidePartBits,
    int localClientNum,
    int entnum);
void __cdecl FX_MarkEntDetachMatchingBones(
    FxMarksSystem *marksSystem,
    int entnum,
    const unsigned int *unsetHidePartBits);
void __cdecl FX_MarkEntUpdateBegin(
    FxMarkDObjUpdateContext *context,
    DObj_s *obj,
    bool isBrush,
    unsigned __int16 brushIndex);
void __cdecl FX_MarkEntUpdateEnd(
    FxMarkDObjUpdateContext *context,
    int localClientNum,
    int entnum,
    DObj_s *obj,
    bool isBrush,
    unsigned __int16 brushIndex);
void __cdecl FX_MarkEntDetachAllOfType(int localClientNum, int entnum, int markType);
void __cdecl FX_MarkEntUpdateEndDObj(FxMarkDObjUpdateContext *context, int localClientNum, int entnum, DObj_s *obj);
void __cdecl FX_MarkEntDetachModel(FxMarksSystem *marksSystem, int entnum, int oldModelIndex);
void __cdecl FX_BeginGeneratingMarkVertsForEntModels(int localClientNum, unsigned int *indexCount);
void __cdecl FX_GenerateMarkVertsForEntXModel(
    int localClientNum,
    int entId,
    unsigned int *indexCount,
    unsigned __int16 lightHandle,
    unsigned __int8 reflectionProbeIndex,
    const GfxScaledPlacement *placement);
char __cdecl FX_GenerateMarkVertsForList_EntXModel(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount,
    unsigned __int16 lightHandleOverride,
    unsigned __int8 reflectionProbeIndexOverride,
    const GfxScaledPlacement *placement);
char __cdecl FX_GenerateMarkVertsForMark_Begin(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    unsigned int *indexCount,
    unsigned __int16 *outBaseVertex,
    FxActiveMarkSurf *outDrawSurf);
void __cdecl FX_DrawMarkTris(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    unsigned __int16 baseVertex,
    unsigned __int16 *indices,
    FxActiveMarkSurf *outSurf);
void __cdecl FX_EmitMarkTri(
    FxMarksSystem *marksSystem,
    const unsigned __int16 *indices,
    const GfxMarkContext *markContext,
    unsigned __int16 baseVertex,
    FxActiveMarkSurf *outSurf);
void __cdecl FX_GenerateMarkVertsForMark_SetLightHandle(
    FxActiveMarkSurf *drawSurf,
    unsigned __int16 lightHandleOverride);
void __cdecl FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(
    FxActiveMarkSurf *drawSurf,
    unsigned __int8 reflectionProbeIndexOverride);
void __cdecl FX_GenerateMarkVertsForMark_FinishAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    unsigned __int16 baseVertex,
    FxActiveMarkSurf *drawSurf,
    const float (*transform)[3]);
void __cdecl FX_GenerateMarkVertsForMark_MatrixFromScaledPlacement(
    const GfxScaledPlacement *placement,
    __int64 viewOffset);
void  FX_GenerateMarkVertsForMark_MatrixFromPlacement(
    const GfxPlacement *placement,
    const float *viewOffset,
    float (*outTransform)[3]);
void __cdecl FX_GenerateMarkVertsForEntDObj(
    int localClientNum,
    int entId,
    unsigned int *indexCount,
    unsigned __int16 lightHandle,
    unsigned __int8 reflectionProbeIndex,
    const DObj_s *dobj,
    const cpose_t *pose);
char __cdecl FX_GenerateMarkVertsForList_EntDObj(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount,
    unsigned __int16 lightHandleOverride,
    unsigned __int8 reflectionProbeIndexOverride,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList);
void  FX_GenerateMarkVertsForMark_MatrixFromAnim(
    FxMark *mark,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList,
    const float *viewOffset,
    float (*outTransform)[3]);
void __cdecl FX_GenerateMarkVertsForEntBrush(
    int localClientNum,
    int entId,
    unsigned int *indexCount,
    unsigned __int8 reflectionProbeIndex,
    const GfxPlacement *placement);
char __cdecl FX_GenerateMarkVertsForList_EntBrush(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount,
    const GfxPlacement *placement,
    unsigned __int8 reflectionProbeIndex);
void __cdecl FX_EndGeneratingMarkVertsForEntModels(int localClientNum);
void __cdecl FX_FinishGeneratingMarkVerts(FxMarksSystem *marksSystem);
void __cdecl FX_GenerateMarkVertsForStaticModels(
    int localClientNum,
    int smodelCount,
    const unsigned __int8 *smodelVisLods);
char __cdecl FX_GenerateMarkVertsForList_WorldXModel(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount);
void __cdecl FX_GenerateMarkVertsForMark_FinishNonAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    unsigned __int16 baseVertex,
    FxActiveMarkSurf *drawSurf);
void __cdecl FX_GenerateMarkVertsForWorld(int localClientNum);
char __cdecl FX_GenerateMarkVertsForList_WorldBrush(
    FxMarksSystem *marksSystem,
    unsigned __int16 head,
    const FxCamera *camera,
    unsigned int *indexCount);




// fx_Draw
enum FxRandKey : __int32
{                                       // ...
    FXRAND_VELOCITY_X = 0x0,
    FXRAND_VELOCITY_Y = 0x1,
    FXRAND_VELOCITY_Z = 0x2,
    FXRAND_ANGULAR_VELOCITY_PITCH = 0x3,
    FXRAND_ANGULAR_VELOCITY_YAW = 0x4,
    FXRAND_ANGULAR_VELOCITY_ROLL = 0x5,
    FXRAND_ORIGIN_X = 0x6,
    FXRAND_ORIGIN_Y = 0x7,
    FXRAND_ORIGIN_Z = 0x8,
    FXRAND_OFFSET_YAW = 0x9,
    FXRAND_OFFSET_HEIGHT = 0xA,
    FXRAND_OFFSET_RADIUS = 0xB,
    FXRAND_ANGLES_PITCH = 0xC,
    FXRAND_ANGLES_YAW = 0xD,
    FXRAND_ANGLES_ROLL = 0xE,
    FXRAND_GRAVITY = 0xF,
    FXRAND_REFLECTION_FACTOR = 0x10,
    FXRAND_LIFE_SPAN = 0x11,
    FXRAND_SPAWN_DELAY = 0x12,
    FXRAND_SPAWN_COUNT = 0x13,
    FXRAND_EMIT_DIST = 0x14,
    FXRAND_VISUAL = 0x15,
    FXRAND_TILE_START = 0x16,
    FXRAND_COLOR = 0x17,
    FXRAND_ROTATION = 0x18,
    FXRAND_ROTATION_DELTA = 0x19,
    FXRAND_SIZE_0 = 0x1A,
    FXRAND_SIZE_1 = 0x1B,
    FXRAND_SCALE = 0x1C,
    FXRAND_COUNT = 0x1D,
};
struct FxDrawState // sizeof=0xA8
{                                       // ...
    FxSystem *system;             // ...
    const FxEffect *effect;             // ...
    const FxElem *elem;
    const FxElemDef *elemDef;
    orientation_t orient;
    FxCamera *camera;
    int randomSeed;
    float msecLifeSpan;
    float msecElapsed;
    float normTimeUpdateEnd;
    float posWorld[3];
    float velDirWorld[3];
    FxElemVisualState visState;
    FxElemPreVisualState preVisState;
    float physicsLerpFrac;
    int msecDraw;                       // ...
};
struct FxTrailSegmentDrawState // sizeof=0x3C
{                                       // ...
    FxTrailDef *trailDef;
    float posWorld[3];                  // ...
    float basis[2][3];                  // ...
    float rotation;
    float size[2];
    float uCoord;                       // ...
    unsigned __int8 color[4];           // ...
};
struct FxBeam // sizeof=0x34
{                                       // ...
    float begin[3];                     // ...
    float end[3];                       // ...
    GfxColor beginColor;                // ...
    GfxColor endColor;                  // ...
    float beginRadius;                  // ...
    float endRadius;                    // ...
    Material *material;                 // ...
    int segmentCount;                   // ...
    float wiggleDist;                   // ...
};
struct FxBeamInfo // sizeof=0x1384
{                                       // ...
    FxBeam beams[96];
    int beamCount;                      // ...
};
struct FxPostLight // sizeof=0x24
{                                       // ...
    float begin[3];                     // ...
    float end[3];                       // ...
    float radius;                       // ...
    GfxColor color;                     // ...
    Material *material;                 // ...
};
struct FxPostLightInfo // sizeof=0xD84
{                                       // ...
    FxPostLight postLights[96];
    int postLightCount;                 // ...
};
struct FxGenerateVertsCmd // sizeof=0x44
{                                       // ...
    FxSystem *system;
    FxBeamInfo *beamInfo;
    FxPostLightInfo *postLightInfo;
    FxSpriteInfo *spriteInfo;
    int localClientNum;
    float vieworg[3];
    float viewaxis[3][3];
};
void __cdecl FX_EvaluateVisAlpha(FxElemPreVisualState *preVisState, FxElemVisualState *visState);
unsigned __int8 __cdecl FX_InterpolateColor(
    const FxElemVisStateSample *refState,
    float valueLerp,
    float valueLerpInv,
    float sampleLerp,
    float sampleLerpInv,
    int channel);
void __cdecl FX_SetupVisualState(
    const FxElemDef *elemDef,
    const FxEffect *effect,
    int randomSeed,
    float normTimeUpdateEnd,
    FxElemPreVisualState *preVisState);
void __cdecl FX_EvaluateSize(FxElemPreVisualState *preVisState, FxElemVisualState *visState);
double __cdecl FX_InterpolateSize(
    const FxElemVisStateSample *refState,
    int randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float sampleLerpInv,
    int channel);
void __cdecl FX_EvaluateVisualState(FxElemPreVisualState *preVisState, float msecLifeSpan, FxElemVisualState *visState);
double __cdecl FX_IntegrateRotationFromZero(
    const FxElemVisStateSample *refState,
    int randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float msecLifeSpan);
void __cdecl FX_EvaluateVisualState_DoLighting(
    FxElemPreVisualState *preVisState,
    FxElemVisualState *visState,
    const FxElemDef *elemDef);
void __cdecl FX_UnpackColor565(
    unsigned __int16 packed,
    unsigned __int8 *outR,
    unsigned __int8 *outG,
    unsigned __int8 *outB);
void __cdecl FX_DrawElem_BillboardSprite(FxDrawState *draw);
void __cdecl FX_GenSpriteVerts(FxDrawState *draw, const float *tangent, const float *binormal, const float *normal);
void __cdecl FX_GetSpriteTexCoords(const FxDrawState *draw, float *s0, float *ds, float *t0, float *dt);
bool __cdecl FX_CullElementForDraw_Sprite(const FxDrawState *draw);
unsigned int __cdecl FX_CullElementForDraw_FrustumPlaneCount(const FxDrawState *draw);
void __cdecl FX_DrawElem_OrientedSprite(FxDrawState *draw);
void __cdecl FX_DrawElem_Tail(FxDrawState *draw);
bool __cdecl FX_CullElementForDraw_Tail(const FxDrawState *draw);
char __cdecl FX_CullCylinder(
    const FxCamera *camera,
    unsigned int frustumPlaneCount,
    const float *posWorld0,
    const float *posWorld1,
    float radius);
void __cdecl FX_DrawElem_Cloud(FxDrawState *draw);
void __cdecl FX_SetPlacement(const FxDrawState *draw, GfxScaledPlacement *placement);
double __cdecl FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int atRestFraction);
double __cdecl FX_InterpolateScale(
    const FxElemVisStateSample *refState,
    int randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float sampleLerpInv);
bool __cdecl FX_CullElementForDraw_Cloud(const FxDrawState *draw);
void __cdecl FX_DrawElem_Model(FxDrawState *draw);
void __cdecl FX_SetPlacementFromPhysics(const FxDrawState *draw, GfxPlacement *placement);
void __cdecl FX_DrawElem_Light(FxDrawState *draw);
bool __cdecl FX_CullElementForDraw_Light(const FxDrawState *draw);
void __cdecl FX_DrawElem_SpotLight(FxDrawState *draw);
void __cdecl FX_DrawNonSpriteElems(FxSystem *system);
void __cdecl FX_BeginIteratingOverEffects_Cooperative(FxSystem *system);
void __cdecl FX_DrawNonSpriteEffect(FxSystem *system, FxEffect *effect, unsigned int elemClass, int drawTime);
void __cdecl FX_DrawElement(FxSystem *system, const FxElemDef *elemDef, const FxElem *elem, FxDrawState *draw);
void __cdecl FX_DrawSpotLight(FxSystem *system);
void __cdecl FX_DrawSpotLightEffect(FxSystem *system, FxEffect *effect, int drawTime);
void __cdecl FX_DrawSpriteElems(FxSystem *system, int drawTime);
void __cdecl FX_DrawTrailsForEffect(FxSystem *system, FxEffect *effect, int drawTime);
void __cdecl FX_DrawTrail(FxSystem *system, FxDrawState *draw, FxTrail *trail);
void __cdecl FX_TrailElem_UncompressBasis(const char (*inBasis)[3], float (*basis)[3]);
void __cdecl FX_GenTrail_IndsForSegment(
    FxDrawState *draw,
    unsigned __int16 reservedBaseVertex,
    r_double_index_t *outIndices);
void __cdecl Fx_GenTrail_PopulateSegmentDrawState(
    FxDrawState *draw,
    float spawnDist,
    float uCoordOffset,
    const float (*basis)[3],
    FxTrailSegmentDrawState *outState);
void __cdecl FX_GenTrail_VertsForSegment(const FxTrailSegmentDrawState *segmentDrawState, GfxPackedVertex *remoteVerts);
void __cdecl Vec3MadMad(
    const float *start,
    float scale0,
    const float *dir0,
    float scale1,
    const float *dir1,
    float *result);
void __cdecl FX_DrawSpriteEffect(FxSystem *system, FxEffect *effect, int drawTime);
void __cdecl FX_GenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl FX_FillGenerateVertsCmd(int localClientNum, FxGenerateVertsCmd *cmd);
void __cdecl FX_EvaluateDistanceFade(FxDrawState *draw);
double __cdecl FX_ClampRangeLerp(float dist, const FxFloatRange *range);
void __cdecl FX_DrawElement_Setup_1_(
    FxSystem* system,
    FxDrawState* draw,
    int elemMsecBegin,
    int elemSequence,
    const float* elemOrigin,
    float* outRealNormTime);

// fx_update_util
void __cdecl FX_OffsetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int randomSeed,
    float *spawnOrigin);
void __cdecl FX_GetOriginForTrailElem(
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed,
    float *outOrigin,
    float *outRight,
    float *outUp);
void __cdecl FX_GetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int randomSeed,
    float *spawnOrigin);
void __cdecl FX_TransformPosFromLocalToWorld(const FxSpatialFrame *frame, float *posLocal, float *posWorld);
void __cdecl FX_SpatialFrameToOrientation(const FxSpatialFrame *frame, orientation_t *orient);
void __cdecl FX_OrientationDirToWorldDir(const orientation_t *orient, const float *dir, float *out);
void __cdecl FX_GetOrientation(
    const FxElemDef* elemDef,
    const FxSpatialFrame* frameAtSpawn,
    const FxSpatialFrame* frameNow,
    int randomSeed,
    orientation_t* orient);
char  FX_GenerateBeam_GetFlatDelta(
    const float4x4* clipMtx,
    const float4x4* invClipMtx,
    float4 beamWorldBegin,
    float4 beamWorldEnd,
    float4* outFlatDelta);
void __cdecl FX_GetVelocityAtTime(
    const FxElemDef *elemDef,
    int randomSeed,
    float msecLifeSpan,
    float msecElapsed,
    const orientation_t *orient,
    const float *baseVel,
    float *velocity);
void __cdecl FX_GetVelocityAtTimeInFrame(
    const FxElemVelStateInFrame *statePrev,
    const FxElemVelStateInFrame *stateNext,
    const float *rangeLerp,
    const float *weight,
    float *velocity);
void __cdecl FX_OrientationPosToWorldPos(const orientation_t *orient, const float *pos, float *out);
void __cdecl FX_OrientationPosFromWorldPos(const orientation_t *orient, const float *pos, float *out);
void __cdecl FX_AddVisBlocker(FxSystem *system, const float *posWorld, float radius, float opacity);
void __cdecl FX_ToggleVisBlockerFrame(FxSystem *system);
char __cdecl FX_CullSphere(const FxCamera *camera, unsigned int frustumPlaneCount, const float *posWorld, float radius);
void __cdecl FX_GetElemAxis(
    const FxElemDef *elemDef,
    int randomSeed,
    const orientation_t *orient,
    float msecElapsed,
    mat3x3& axis);
void __cdecl FX_AnglesToOrientedAxis(const float *anglesInRad, const orientation_t *orient, mat3x3 &axisOut);


// fx_random
void __cdecl TRACK_fx_random();
void __cdecl FX_RandomDir(int seed, float *dir);
void __cdecl FX_RandomlyRotateAxis(const float (*axisIn)[3], int randomSeed, mat3x3& axisOut);



// fx_sort
struct __declspec(align(4)) FxInsertSortElem // sizeof=0x14
{                                       // ...
    int defSortOrder;                   // ...
    float distToCamSq;
    int msecBegin;
    int defIndex;
    unsigned __int8 elemType;
    // padding byte
    // padding byte
    // padding byte
};
void __cdecl FX_SortEffects(FxSystem *system);
void __cdecl FX_WaitBeginIteratingOverEffects_Exclusive(FxSystem *system);
bool __cdecl FX_FirstEffectIsFurther(FxEffect *firstEffect, FxEffect *secondEffect);
int __cdecl FX_CalcRunnerParentSortOrder(FxEffect *effect);
void __cdecl FX_SortNewElemsInEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_SortSpriteElemIntoEffect(FxSystem *system, FxEffect *effect, FxElem *elem);
void __cdecl FX_GetInsertSortElem(
    const FxSystem *system,
    const FxEffect *effect,
    const FxElem *elem,
    FxInsertSortElem *sortElem);
bool __cdecl FX_ExistingElemSortsBeforeNewElem(
    const FxSystem *system,
    const FxEffect *effect,
    const FxElem *elem,
    const FxInsertSortElem *sortElemNew);


// fx_archive
struct FxEffectDefTableEntry // sizeof=0x8
{                                       // ...
    unsigned int key;
    const FxEffectDef *effectDef;
};
struct FxEffectDefTable // sizeof=0x2004
{                                       // ...
    int count;
    FxEffectDefTableEntry entries[1024];
};
void __cdecl FX_Restore(int clientIndex, MemoryFile *memFile);
void __cdecl FX_RestoreEffectDefTable(MemoryFile *memFile, FxEffectDefTable *table);
void __cdecl FX_AddEffectDefTableEntry(FxEffectDefTable *table, unsigned int key, const FxEffectDef *effectDef);
void __cdecl FX_FixupEffectDefHandles(FxSystem *system, FxEffectDefTable *table);
FxEffect *__cdecl FX_EffectFromHandle(FxSystem *system, unsigned __int16 handle);
const FxEffectDef *__cdecl FX_FindEffectDefInTable(const FxEffectDefTable *table, unsigned int key);
void __cdecl FX_RestorePhysicsData(FxSystem *system, MemoryFile *memFile);
FxElemVisuals __cdecl FX_GetElemVisuals(const FxElemDef *elemDef, int randomSeed);
void __cdecl FX_Save(int clientIndex, MemoryFile *memFile);
void __cdecl FX_SaveEffectDefTable(FxSystem *system, MemoryFile *memFile);
void __cdecl FX_SaveEffectDefTable_FastFile(MemoryFile *memFile);
void __cdecl FX_SaveEffectDefTable_LoadObj(MemoryFile* memFile);
void __cdecl FX_SavePhysicsData(FxSystem *system, MemoryFile *memFile);
void __cdecl FX_Archive(int clientIndex, MemoryFile *memFile);

// fx_beam
void __cdecl FX_Beam_GenerateVerts(FxGenerateVertsCmd *cmd);

void __cdecl FX_Beam_Begin();
void __cdecl FX_Beam_Add(FxBeam *beam);
FxBeamInfo *__cdecl FX_Beam_GetInfo();


// fx_postlight
void __cdecl FX_PostLight_GenerateVerts(FxPostLightInfo *postLightInfoAddr, FxSystem *system);
void __cdecl FX_PostLight_Begin();
void __cdecl FX_PostLight_Add(FxPostLight *postLight);
FxPostLightInfo *__cdecl FX_PostLight_GetInfo();


// fx_profile
struct FxProfileEntry // sizeof=0x1C
{                                       // ...
    const FxEffectDef *effectDef;
    int effectCount;
    int activeElemCount;
    int pendingElemCount;
    int trailCount;
    int activeTrailElemCount;
    int pendingTrailElemCount;
};
void __cdecl FX_DrawProfile(int clientIndex, void(__cdecl *drawFunc)(char *), float *profilePos);
FxProfileEntry *__cdecl FX_GetProfileEntry(const FxEffectDef *effectDef, FxProfileEntry *entryPool, int *entryCount);
void __cdecl FX_ProfileSingleEffect(FxSystem *system, const FxEffect *effect, FxProfileEntry *entry);
int __cdecl FX_CompareProfileEntries(const FxProfileEntry *e0, const FxProfileEntry *e1);
double __cdecl FX_GetProfileEntryCost(const FxProfileEntry *entry);
void __cdecl FX_DrawMarkProfile(int clientIndex, void(__cdecl *drawFunc)(char *), float *profilePos);
void __cdecl FX_DrawMarkProfile_MarkPrint(
    FxMarksSystem* marksSystem,
    unsigned __int16 head,
    const char* name,
    int index,
    void(__cdecl* drawFunc)(const char*, float*),
    float* profilePos);



// fx_sprite
struct FxSprite // sizeof=0x20
{                                       // ...
    Material *material;
    float pos[3];
    unsigned __int8 rgbaColor[4];
    float radius;
    float minScreenRadius;
    int flags;
};
void __cdecl FX_SpriteGenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl FX_GenerateSpriteCodeMeshVerts(FxSprite *sprite, FxGenerateVertsCmd *cmd);
void __cdecl FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
    Material *material,
    const float *pos,
    float radius,
    const unsigned __int8 *rgbaColor,
    char spriteFlags,
    FxGenerateVertsCmd *cmd);
void __cdecl FX_BuildSpriteCodeMeshVerts(
    Material *material,
    const float *pos,
    float worldRadius,
    const unsigned __int8 *rgbaColor,
    char spriteFlags);
void __cdecl FX_BuildQuadStampCodeMeshVerts(
    Material *material,
    const float *viewAxis,
    const float *origin,
    const float *left,
    const float *up,
    const unsigned __int8 *rgbaColor,
    int s0,
    int t0,
    int s1,
    int t1);
void __cdecl Byte4CopyRgbaToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo);
char __cdecl FX_HeightScreenToWorld(
    const float *worldOrigin,
    float screenHeight,
    float *worldHeight,
    FxGenerateVertsCmd *cmd);
double __cdecl FX_GetClipSpaceW(const float *worldPoint, float *vieworg, float (*viewaxis)[3]);
void __cdecl FX_GenerateSpriteCodeMeshVertsFixedWorldSize(
    Material* material,
    const float* pos,
    float radius,
    float minScreenRadius,
    const unsigned __int8* rgbaColor,
    char spriteFlags,
    FxGenerateVertsCmd* cmd);
char __cdecl FX_HeightWorldToScreen(
    const float *worldOrigin,
    float worldHeight,
    float *screenHeight,
    FxGenerateVertsCmd *cmd);
void __cdecl FX_SpriteBegin();
void __cdecl FX_SpriteAdd(FxSprite *sprite);
FxSpriteInfo *__cdecl FX_SpriteGetInfo();


// fx_update
enum FxUpdateResult : __int32
{                                       // ...
    FX_UPDATE_REMOVE = 0x0,
    FX_UPDATE_KEEP = 0x1,
};
void __cdecl FX_SpawnAllFutureLooping(
    FxSystem* system,
    FxEffect* effect,
    int elemDefFirst,
    int elemDefCount,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin);
void __cdecl FX_SpawnLoopingElems(
    FxSystem* system,
    FxEffect* effect,
    int elemDefIndex,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin,
    int msecUpdateEnd);
int __cdecl FX_LimitStabilizeTimeForElemDef_Recurse(
    const FxElemDef *elemDef,
    bool needToSpawnSystem,
    int originalUpdateTime);
int __cdecl FX_LimitStabilizeTimeForElemDef_SelfOnly(const FxElemDef *elemDef, bool needToSpawnSystem);
int __cdecl FX_LimitStabilizeTimeForEffectDef_Recurse(const FxEffectDef *remoteEffectDef, int originalUpdateTime);
void __cdecl FX_BeginLooping(
    FxSystem* system,
    FxEffect* effect,
    int elemDefFirst,
    int elemDefCount,
    FxSpatialFrame* frameWhenPlayed,
    FxSpatialFrame* a2,
    int msecWhenPlayed,
    int msecNow);
void __cdecl FX_SpawnTrailLoopingElems(
    FxSystem* system,
    FxEffect* effect,
    FxTrail* trail,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd);
void __cdecl FX_TriggerOneShot(
    FxSystem *system,
    FxEffect *effect,
    int elemDefFirst,
    int elemDefCount,
    const FxSpatialFrame *frameWhenPlayed,
    int msecWhenPlayed);
void __cdecl FX_SpawnOneShotElems(
    FxSystem *system,
    FxEffect *effect,
    int elemDefIndex,
    const FxSpatialFrame *frameWhenPlayed,
    int msecWhenPlayed);
void __cdecl FX_StartNewEffect(FxSystem* system, FxEffect* effect);
bool __cdecl FX_GetBoltTemporalBits(int localClientNum, int dobjHandle);
char __cdecl FX_GetBoneOrientation(int localClientNum, unsigned int dobjHandle, int boneIndex, orientation_t *orient);
bool __cdecl FX_GetBoneOrientation_IsDObjEntityValid(int localClientNum, int dobjHandle);
void __cdecl FX_UpdateEffectPartial(
    FxSystem* system,
    FxEffect* effect,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd,
    unsigned __int16* elemHandleStart,
    unsigned __int16* elemHandleStop,
    unsigned __int16* trailElemStart,
    unsigned __int16* trailElemStop);
void __cdecl FX_ProcessLooping(
    FxSystem* system,
    FxEffect* effect,
    int elemDefFirst,
    int elemDefCount,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int msecWhenPlayed,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd);
void __cdecl FX_UpdateEffectPartialForClass(
    FxSystem *system,
    FxEffect *effect,
    long double msecUpdateBegin,
    unsigned __int16 elemHandleStart,
    unsigned __int16 elemHandleStop,
    unsigned int elemClass);
FxUpdateResult __cdecl FX_UpdateElement(
    FxSystem *system,
    FxEffect *effect,
    FxElem *elem,
    int msecUpdateBegin,
    int msecUpdateEnd);
const FxElemDef *__cdecl FX_GetUpdateElemDef(const FxUpdateElem *update);
double __cdecl FX_GetAtRestFraction(const FxUpdateElem *update, float msec);
int __cdecl FX_UpdateElementPosition(FxSystem *system, FxUpdateElem *update);
int __cdecl FX_UpdateElementPosition_Colliding(FxSystem *system, FxUpdateElem *update);
int __cdecl FX_UpdateElementPosition_CollidingStep(
    FxSystem *system,
    FxUpdateElem *update,
    double msecUpdateBegin,
    float *xyzWorldOld);
void __cdecl FX_NextElementPosition(FxUpdateElem *update, double msecUpdateBegin);
void __cdecl FX_NextElementPosition_NoExternalForces(
    FxUpdateElem* update,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float* posLocal,
    float* posWorld);
void __cdecl FX_IntegrateVelocity(const FxUpdateElem *update, float t0, float t1, float *posLocal, float *posWorld);
void __cdecl FX_IntegrateVelocityAcrossSegments(
    int elemDefFlags,
    const orientation_t *orient,
    const FxElemVelStateSample *velState0,
    const FxElemVelStateSample *velState1,
    float t0,
    float t1,
    const float *amplitudeScale,
    float integralScale,
    float *posLocal,
    float *posWorld);
void __cdecl FX_IntegrateVelocityFromZeroInSegment(
    const FxElemVelStateInFrame *statePrev,
    const FxElemVelStateInFrame *stateNext,
    float *weight,
    const float *amplitudeScale,
    float integralScale,
    float *pos);
void __cdecl FX_IntegrateVelocityInSegment(
    int elemDefFlags,
    const orientation_t *orient,
    const FxElemVelStateSample *velState,
    float t0,
    float t1,
    const float *amplitudeScale,
    float integralScale,
    float *posLocal,
    float *posWorld);
void __cdecl FX_IntegrateVelocityInSegmentInFrame(
    const FxElemVelStateInFrame *statePrev,
    const FxElemVelStateInFrame *stateNext,
    const float *weight,
    const float *amplitudeScale,
    float integralScale,
    float *pos);
bool __cdecl FX_TraceHitSomething(const trace_t *trace);
int __cdecl FX_CollisionResponse(
    FxSystem *system,
    FxUpdateElem *update,
    const trace_t *trace,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float *xyzWorldOld);
void __cdecl FX_SpawnImpactEffect(
    FxSystem *system,
    const FxUpdateElem *update,
    const FxEffectDef *impactEffect,
    int msecOnImpact,
    const float *impactNormal);
int __cdecl FX_UpdateElementPosition_NonColliding(FxUpdateElem *update);
int __cdecl FX_UpdateElementPosition_Local(FxUpdateElem *update);
void __cdecl FX_SpawnDeathEffect(FxSystem *system, FxUpdateElem *update);
char __cdecl FX_UpdateElement_SetupUpdate(
    FxEffect *effect,
    int msecUpdateBegin,
    int msecUpdateEnd,
    unsigned int elemDefIndex,
    int elemAtRestFraction,
    int elemMsecBegin,
    int elemSequence,
    float *elemOrigin,
    FxUpdateElem *update);
void __cdecl FX_UpdateElement_TruncateToElemEnd(FxUpdateElem *update, FxUpdateResult *outUpdateResult);
void __cdecl FX_UpdateElement_HandleEmitting(
    FxSystem *system,
    FxElem *elem,
    FxUpdateElem *update,
    const float *elemOriginPrev,
    FxUpdateResult *outUpdateResult);
unsigned __int8 __cdecl FX_ProcessEmitting(
    FxSystem *system,
    FxUpdateElem *update,
    unsigned __int8 emitResidual,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd);
void __cdecl FX_GetQuatForOrientation(
    const FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *frameNow,
    orientation_t *orient,
    float *quat);
char __cdecl FX_UpdateElement_TruncateToElemBegin(FxUpdateElem *update, FxUpdateResult *outUpdateResult);
void __cdecl FX_UpdateEffectPartialTrail(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    int msecUpdateBegin,
    int msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd,
    unsigned __int16 trailElemHandleStart,
    unsigned __int16 trailElemHandleStop,
    FxSpatialFrame *frameNow);
void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3]);
FxUpdateResult __cdecl FX_UpdateTrailElement(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    FxTrailElem *trailElem,
    int msecUpdateBegin,
    int msecUpdateEnd);
void __cdecl FX_UpdateSpotLight(FxCmd *cmd);
void __cdecl FX_UpdateSpotLightEffect(FxSystem* system, FxEffect* effect);
void __cdecl FX_UpdateSpotLightEffectPartial(
    FxSystem* system,
    FxEffect* effect,
    int msecUpdateBegin,
    int msecUpdateEnd);
void __cdecl FX_UpdateEffectBolt(FxSystem *system, FxEffect *effect);
void __cdecl FX_UpdateNonDependent(FxCmd *cmd);
void __cdecl FX_Update(FxSystem *system, int localClientNum, bool nonBoltedEffectsOnly);
void __cdecl FX_UpdateEffect(FxSystem* system, FxEffect* effect);
bool __cdecl FX_ShouldProcessEffect(FxSystem *system, FxEffect *effect, bool nonBoltedEffectsOnly);
void __cdecl FX_RunPhysics(int localClientNum);
void __cdecl FX_UpdateRemaining(FxCmd *cmd);
void __cdecl FX_BeginUpdate(int localClientNum);
void __cdecl FX_EndUpdate(int localClientNum);
void __cdecl FX_AddNonSpriteDrawSurfs(FxCmd *cmd);
void __cdecl FX_RewindTo(int localClientNum, int time);
void __cdecl FX_SetNextUpdateCamera(int localClientNum, const refdef_s *refdef, float zfar);
void __cdecl FX_SetNextUpdateTime(int localClientNum, int time);
void __cdecl FX_FillUpdateCmd(int localClientNum, FxCmd *cmd);



// fx_curve
struct FxCurve // sizeof=0xC
{
    int dimensionCount;
    int keyCount;
    float keys[1];
};
struct FxCurveIterator // sizeof=0x8
{                                       // ...
    const FxCurve *master;
    int currentKeyIndex;
};
double __cdecl FxCurve_Interpolate1d(const float *key, float intermediateTime);
void __cdecl FxCurve_Interpolate3d(const float *key, float intermediateTime, float *result);
void __cdecl FxCurveIterator_Create(FxCurveIterator *createe, const FxCurve *master);
void __cdecl FxCurveIterator_Release(FxCurveIterator *releasee);

const FxCurve *__cdecl FxCurve_AllocAndCreateWithKeys(float *keyArray, int dimensionCount, int keyCount);
void __cdecl FxCurveIterator_SampleTimeVec3(FxCurveIterator *source, float *replyVector, float time);
double __cdecl FxCurveIterator_SampleTime(FxCurveIterator *source, float time);
void __cdecl FxCurveIterator_MoveToTime(FxCurveIterator *source, float time);



// fx_load_obj
struct FxEditorElemAtlas // sizeof=0x1C
{                                       // ...
    int behavior;
    int index;
    int fps;
    int loopCount;
    int colIndexBits;
    int rowIndexBits;
    int entryCount;
};
struct FxEditorTrailDef // sizeof=0x608
{                                       // ...
    FxTrailVertex verts[64];
    int vertCount;
    unsigned __int16 inds[128];
    int indCount;
};
struct FxEditorElemDef // sizeof=0x858
{                                       // ...
    char name[48];
    int editorFlags;
    int flags;
    FxFloatRange spawnRange;
    FxFloatRange fadeInRange;
    FxFloatRange fadeOutRange;
    float spawnFrustumCullRadius;
    FxSpawnDefLooping spawnLooping;
    FxSpawnDefOneShot spawnOneShot;
    FxIntRange spawnDelayMsec;
    FxIntRange lifeSpanMsec;
    FxFloatRange spawnOrigin[3];
    FxFloatRange spawnOffsetRadius;
    FxFloatRange spawnOffsetHeight;
    FxFloatRange spawnAngles[3];
    FxFloatRange angularVelocity[3];
    FxFloatRange initialRotation;
    FxFloatRange gravity;
    FxFloatRange elasticity;
    FxEditorElemAtlas atlas;
    float velScale[2][3];
    const FxCurve *velShape[2][3][2];
    float rotationScale;
    const FxCurve *rotationShape[2];
    float sizeScale[2];
    const FxCurve *sizeShape[2][2];
    float scaleScale;
    const FxCurve *scaleShape[2];
    const FxCurve *color[2];
    const FxCurve *alpha[2];
    float lightingFrac;
    float collOffset[3];
    float collRadius;
    const FxEffectDef *effectOnImpact;
    const FxEffectDef *effectOnDeath;
    int sortOrder;
    const FxEffectDef *emission;
    FxFloatRange emitDist;
    FxFloatRange emitDistVariance;
    unsigned __int8 elemType;
    // padding byte
    // padding byte
    // padding byte
    int visualCount;
    //$6DCA2FC3F9FD742A3C1907AE7E70399A ___u41;
    union
    {
        FxElemVisuals visuals[32];
        FxElemMarkVisuals markVisuals[16];
    }visualunion;
    int trailSplitDist;
    int trailRepeatDist;
    float trailScrollTime;
    FxEditorTrailDef trailDef;
};
struct FxEditorEffectDef // sizeof=0x10B44
{                                       // ...
    char name[64];
    int elemCount;
    FxEditorElemDef elems[32];
};
struct FxElemField // sizeof=0x8
{                                       // ...
    const char *keyName;                // ...
    bool(__cdecl *handler)(const char **, FxEditorElemDef *); // ...
};
struct FxFlagOutputSet // sizeof=0xC
{                                       // ...
    int *flags[3];                      // ...
};
struct FxFlagDef // sizeof=0x10
{
    const char *name;
    int flagType;
    int mask;
    int value;
};
enum FxSampleChannel : __int32
{                                       // ...
    FX_CHAN_RGBA = 0x0,
    FX_CHAN_SIZE_0 = 0x1,
    FX_CHAN_SIZE_1 = 0x2,
    FX_CHAN_SCALE = 0x3,
    FX_CHAN_ROTATION = 0x4,
    FX_CHAN_COUNT = 0x5,
    FX_CHAN_NONE = 0x6,
};
const FxEffectDef *__cdecl FX_Register(char *name);
const FxEffectDef *__cdecl FX_Register_FastFile(const char *name);
void __cdecl FX_RegisterDefaultEffect();
PhysPreset *__cdecl FX_RegisterPhysPreset(const char *name);
void __cdecl FX_ForEachEffectDef(void(__cdecl* callback)(const FxEffectDef*, void*), void* data);
void FX_UnregisterAll();

// fx_convert
const FxEffectDef *__cdecl FX_Convert(const FxEditorEffectDef *editorEffect, void *(* Alloc)(unsigned int));

extern const float fx_randomTable[];