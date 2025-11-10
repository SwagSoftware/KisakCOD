#pragma once
#include <cstdint>

#include <qcommon/qcommon.h>

#include <universal/memfile.h>

#include <gfx_d3d/fxprimitives.h>
#include <gfx_d3d/r_gfx.h>

#define FX_MARK_FREE -1
#define FX_MARKS_LIMIT 512 // 0x200
#define FX_EFFECT_LIMIT 1024

enum $FFE723C3A54D7F6DDF86A219D7944B2F : int32_t
{
    FX_STATUS_REF_COUNT_MASK = 0xFFFF,
    FX_STATUS_HAS_PENDING_LOOP_ELEMS = 0x10000,
    FX_STATUS_OWNED_EFFECTS_SHIFT = 0x11,
    FX_STATUS_OWNED_EFFECTS_MASK = 0x7FE0000,
    FX_STATUS_DEFER_UPDATE = 0x8000000,
    FX_STATUS_SELF_OWNED = 0x10000000,
    FX_STATUS_IS_LOCKED = 0x20000000,
    FX_STATUS_IS_LOCKED_MASK = 0x60000000,
};

enum $390C8AB619C5D27F330E671BCD9D689E : int32_t
{
    FX_ELEM_TYPE_SPRITE_BILLBOARD = 0x0,
    FX_ELEM_TYPE_SPRITE_ORIENTED = 0x1,
    FX_ELEM_TYPE_TAIL = 0x2,
    FX_ELEM_TYPE_TRAIL = 0x3,
    FX_ELEM_TYPE_CLOUD = 0x4,
    FX_ELEM_TYPE_MODEL = 0x5,
    FX_ELEM_TYPE_OMNI_LIGHT = 0x6,
    FX_ELEM_TYPE_SPOT_LIGHT = 0x7,
    FX_ELEM_TYPE_SOUND = 0x8,
    FX_ELEM_TYPE_DECAL = 0x9,
    FX_ELEM_TYPE_RUNNER = 0xA,
    FX_ELEM_TYPE_COUNT = 0xB,
    FX_ELEM_TYPE_LAST_SPRITE = 0x3,
    FX_ELEM_TYPE_LAST_DRAWN = 0x7,
};

#define FX_BONE_INDEX_NONE 2047
#define FX_DOBJ_HANDLE_NONE 4095

void __cdecl TRACK_fx_system();
XModel *__cdecl FX_RegisterModel(const char *modelName);
FxSystem *__cdecl FX_GetSystem(int32_t clientIndex);
FxSystemBuffers *__cdecl FX_GetSystemBuffers(int32_t clientIndex);
void __cdecl FX_LinkSystemBuffers(FxSystem *system, FxSystemBuffers *systemBuffers);
void __cdecl FX_InitSystem(int32_t localClientNum);
void __cdecl FX_ResetSystem(FxSystem *system);
int32_t __cdecl FX_EffectToHandle(FxSystem *system, FxEffect *effect);
void __cdecl FX_ShutdownSystem(int32_t localClientNum);
void __cdecl FX_RelocateSystem(FxSystem *system, int32_t relocationDistance);
void __cdecl FX_EffectNoLongerReferenced(FxSystem *system, FxEffect *remoteEffect);
void __cdecl FX_DelRefToEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_RunGarbageCollection(FxSystem *system);
bool __cdecl FX_BeginIteratingOverEffects_Exclusive(FxSystem *system);
void __cdecl FX_RunGarbageCollection_FreeSpotLight(FxSystem *system, uint16_t effectHandle);
void __cdecl FX_RunGarbageCollection_FreeTrails(FxSystem *system, FxEffect *effect);
void __cdecl FX_SpawnEffect_AllocTrails(FxSystem *system, FxEffect *effect);
FxPool<FxTrail> *__cdecl FX_AllocTrail(FxSystem *system);
uint16_t __cdecl FX_CalculatePackedLighting(const float *origin);
FxEffect *__cdecl FX_SpawnEffect(
    FxSystem *system,
    const FxEffectDef *remoteDef,
    int32_t msecBegin,
    const float *origin,
    const float (*axis)[3],
    int32_t dobjHandle,
    int32_t boneIndex,
    int32_t runnerSortOrder,
    uint16_t owner,
    uint32_t markEntnum);
void __cdecl FX_AddRefToEffect(FxSystem *__formal, FxEffect *effect);
char __cdecl FX_CullEffectForSpawn(const FxCamera *camera, const FxEffectDef *effectDef, const float *origin);
bool __cdecl FX_CullElemForSpawn(const FxCamera *camera, const FxElemDef *elemDef, const float *origin);
void __cdecl FX_SetEffectRandomSeed(FxEffect *effect, const FxEffectDef *remoteDef);
char __cdecl FX_EffectAffectsGameplay(const FxEffectDef *remoteEffectDef);
char __cdecl FX_IsSpotLightEffect(FxSystem *system, const FxEffectDef *def);
bool __cdecl FX_CanAllocSpotLightEffect(const FxSystem *system);
char __cdecl FX_SpawnEffect_AllocSpotLightEffect(FxSystem *system, FxEffect *effect);
FxEffect *__cdecl FX_SpawnOrientedEffect(
    int32_t localClientNum,
    const FxEffectDef *def,
    int32_t msecBegin,
    const float *origin,
    const float (*axis)[3],
    uint32_t markEntnum);
void __cdecl FX_AssertAllocatedEffect(int32_t localClientNum, FxEffect *effect);
void __cdecl FX_PlayOrientedEffectWithMarkEntity(
    int32_t localClientNum,
    const FxEffectDef *def,
    int32_t startMsec,
    const float *origin,
    const float (*axis)[3],
    uint32_t markEntnum);
void __cdecl FX_PlayOrientedEffect(
    int32_t localClientNum,
    const FxEffectDef *def,
    int32_t startMsec,
    const float *origin,
    const float (*axis)[3]);
FxEffect *__cdecl FX_SpawnBoltedEffect(
    int32_t localClientNum,
    const FxEffectDef *def,
    int32_t msecBegin,
    uint32_t dobjHandle,
    uint32_t boneIndex);
char __cdecl FX_NeedsBoltUpdate(const FxEffectDef *def);
void __cdecl FX_PlayBoltedEffect(
    int32_t localClientNum,
    const FxEffectDef *def,
    int32_t startMsec,
    uint32_t dobjHandle,
    uint32_t boneIndex);
void __cdecl FX_RetriggerEffect(int32_t localClientNum, FxEffect *effect, int32_t msecBegin);
void __cdecl FX_GetTrailHandleList_Last(
    FxSystem *system,
    FxEffect *effect,
    uint16_t *outHandleList,
    int32_t *outTrailCount);
void __cdecl FX_ThroughWithEffect(int32_t localClientNum, FxEffect *effect);
void __cdecl FX_StopEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_StopEffectNonRecursive(FxSystem *system, FxEffect *effect);
void __cdecl FX_KillEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_RemoveAllEffectElems(FxSystem *system, FxEffect *effect);
void __cdecl FX_KillEffectDef(int32_t localClientNum, const FxEffectDef *def);
void __cdecl FX_KillAllEffects(int32_t localClientNum);
void __cdecl FX_SpawnTrailElem_NoCull(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t msecWhenPlayed,
    float distanceWhenPlayed);
FxPool<FxTrailElem> *__cdecl FX_AllocTrailElem(FxSystem *system);
void __cdecl FX_SpawnTrailElem_Cull(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t msecWhenPlayed,
    float distanceWhenPlayed);
bool __cdecl FX_CullTrailElem(
    const FxCamera *camera,
    const FxElemDef *elemDef,
    const float *origin,
    uint8_t sequence);
void __cdecl FX_SpawnSpotLightElem(FxSystem *system, FxElem *elem);
void __cdecl FX_SpawnElem(
    FxSystem *system,
    FxEffect *effect,
    int32_t elemDefIndex,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t msecWhenPlayed,
    float distanceWhenPlayed,
    int32_t sequence);
FxPool<FxElem> *__cdecl FX_AllocElem(FxSystem *system);
void __cdecl FX_SpawnRunner(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *remoteElemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t randomSeed,
    int32_t msecWhenPlayed);
bool __cdecl FX_SpawnModelPhysics(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *elemDef,
    int32_t randomSeed,
    FxElem *elem);
void __cdecl FX_GetOriginForElem(
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t randomSeed,
    float *outOrigin);
void __cdecl FX_SpawnSound(
    int32_t localClientNumber,
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t randomSeed);
void __cdecl FX_FreeElem(FxSystem *system, uint16_t elemHandle, FxEffect *effect, uint32_t elemClass);
void __cdecl FX_FreeTrailElem(FxSystem *system, uint16_t trailElemHandle, FxEffect *effect, FxTrail *trail);
void __cdecl FX_FreeSpotLightElem(FxSystem *system, uint16_t elemHandle, FxEffect *effect);
double __cdecl FX_GetClientVisibility(int32_t localClientNum, const float *start, const float *end);
void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3]);

double FX_GetServerVisibility(const float *start, const float *end);
FxEffect *FX_GetClientEffectByIndex(int clientIndex, unsigned int index);
int FX_GetClientEffectIndex(int clientIndex, FxEffect *effect);

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
enum MarkFragmentsAgainstEnum : int32_t
{                                       // ...
    MARK_FRAGMENTS_AGAINST_BRUSHES = 0x0,
    MARK_FRAGMENTS_AGAINST_MODELS = 0x1,
};
struct FxMarkTri // sizeof=0xC
{                                       // ...
    uint16_t indices[3];        // ...
    GfxMarkContext context;             // ...
};
static_assert(sizeof(FxMarkTri) == 0xC);

struct MarkInfoCollidedDObj // sizeof=0xC
{                                       // ...
    DObj_s *dObj;
    cpose_t *pose;
    uint16_t entnum;
    // padding byte
    // padding byte
};
static_assert(sizeof(MarkInfoCollidedDObj) == 0xC);

struct MarkInfoCollidedBModel // sizeof=0xC
{                                       // ...
    GfxBrushModel *brushModel;
    cpose_t *pose;
    uint16_t entnum;
    // padding byte
    // padding byte
};
static_assert(sizeof(MarkInfoCollidedBModel) == 0xC);

struct MarkInfo // sizeof=0x448
{                                       // ...
    float origin[3];
    float localOrigin[3];
    float localTexCoordAxis[3];
    float axis[3][3];
    float radius;
    Material *material;
    int32_t maxTris;
    FxMarkTri *tris;
    int32_t maxPoints;
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
    uint16_t smodelsCollided[32];
    int32_t smodelCollidedCount;
    MarkInfoCollidedDObj sceneDObjsCollided[32];
    int32_t sceneDObjCollidedCount;
    MarkInfoCollidedBModel sceneBModelsCollided[32];
    int32_t sceneBModelCollidedCount;
    int32_t usedTriCount;
    int32_t usedPointCount;
    void(__cdecl *callback)(void *, int32_t, FxMarkTri *, int32_t, FxMarkPoint *, const float *, const float *);
    void *callbackContext;
};
static_assert(sizeof(MarkInfo) == 0x448);

struct MarkModelCoreContext // sizeof=0x1C
{                                       // ...
    MarkInfo *markInfo;                 // ...
    GfxMarkContext *markContext;        // ...
    const float *markOrigin;            // ...
    const float *markDir;               // ...
    const float (*clipPlanes)[4];       // ...
    const float (*transformMatrix)[3];  // ...
    const float (*transformNormalMatrix)[3]; // ...
};
static_assert(sizeof(MarkModelCoreContext) == 0x1C);

struct FxMarkDObjUpdateContext // sizeof=0x108
{                                       // ...
    XModel *models[32];
    const char *modelParentBones[32];
    int32_t modelCount;
    bool isBrush;
    // padding byte
    uint16_t brushIndex;
};
static_assert(sizeof(FxMarkDObjUpdateContext) == 0x108);

struct FxActiveMarkSurf // sizeof=0x14
{                                       // ...
    Material *material;
    GfxMarkContext context;
    // padding byte
    // padding byte
    int32_t indexCount;
    uint16_t *indices;
};
static_assert(sizeof(FxActiveMarkSurf) == 0x14);

void __cdecl TRACK_fx_marks();
void __cdecl FX_InitMarksSystem(FxMarksSystem *marksSystem);
uint16_t __cdecl FX_MarkToHandle(FxMarksSystem *marksSystem, FxMark *mark);
void __cdecl FX_BeginMarks(int32_t clientIndex);
void __cdecl FX_CreateImpactMark(
    int32_t localClientNum,
    const FxElemDef *elemDef,
    const FxSpatialFrame *spatialFrame,
    int32_t randomSeed,
    uint32_t markEntnum);
void __cdecl FX_ImpactMark(
    int32_t localClientNum,
    Material *worldMaterial,
    Material *modelMaterial,
    float *origin,
    const float *quat,
    float orientation,
    const uint8_t *nativeColor,
    float radius,
    uint32_t markEntnum);
void __cdecl FX_ImpactMark_Generate(
    int32_t localClientNum,
    MarkFragmentsAgainstEnum markAgainst,
    Material *material,
    float *origin,
    const float (*axis)[3],
    float orientation,
    const uint8_t *nativeColor,
    float radius,
    uint32_t markEntnum);
void __cdecl FX_ImpactMark_Generate_AddEntityBrush(
    int32_t localClientNum,
    MarkInfo *markInfo,
    uint32_t entityIndex,
    const float *origin,
    float radius);
void __cdecl FX_ImpactMark_Generate_AddEntityModel(
    int32_t localClientNum,
    MarkInfo *markInfo,
    uint32_t entityIndex,
    const float *origin,
    float radius);
void __cdecl FX_ImpactMark_Generate_Callback(
    void *context,
    int32_t triCount,
    FxMarkTri *tris,
    int32_t pointCount,
    FxMarkPoint *points,
    const float *markOrigin,
    const float *markTexCoordAxis);
void __cdecl FX_AllocAndConstructMark(
    int32_t localClientNum,
    int32_t triCount,
    int32_t pointCount,
    Material *material,
    FxMarkTri *markTris,
    const FxMarkPoint *markPoints,
    const float *origin,
    float radius,
    const float *texCoordAxis,
    const uint8_t *nativeColor);
FxMark *__cdecl FX_MarkFromHandle(FxMarksSystem *marksSystem, uint16_t handle);
void __cdecl FX_FreeLruMark(FxMarksSystem *marksSystem);
void __cdecl FX_FreeMark(FxMarksSystem *marksSystem, FxMark *mark);
void __cdecl FX_FreeMarkFromList(FxMarksSystem *marksSystem, FxMark *mark, uint16_t *listHead);
void __cdecl FX_FreeMarkTriGroups(FxMarksSystem *marksSystem, FxMark *mark);
FxTriGroupPool *__cdecl FX_TriGroupFromHandle(FxMarksSystem *marksSystem, uint32_t handle);
void __cdecl FX_FreeMarkPointGroups(FxMarksSystem *marksSystem, FxMark *mark);
FxPointGroupPool *__cdecl FX_PointGroupFromHandle(FxMarksSystem *marksSystem, uint32_t handle);
int32_t __cdecl FX_AllocMarkTris(FxMarksSystem *marksSystem, const FxMarkTri *markTris, int32_t triCount);
int32_t __cdecl FX_TriGroupToHandle(FxMarksSystem *marksSystem, FxTriGroup *group);
int32_t __cdecl FX_AllocMarkPoints(FxMarksSystem *marksSystem, int32_t pointCount);
int32_t __cdecl FX_PointGroupToHandle(FxMarksSystem *marksSystem, FxPointGroup *group);
void __cdecl FX_LinkMarkIntoList(FxMarksSystem *marksSystem, uint16_t *head, FxMark *mark);
void __cdecl FX_CopyMarkTris(
    FxMarksSystem *marksSystem,
    const FxMarkTri *srcTris,
    uint32_t dstGroupHandle,
    int32_t triCount);
void __cdecl FX_CopyMarkPoints(
    FxMarksSystem *marksSystem,
    const FxMarkPoint *srcPoints,
    uint32_t dstGroupHandle,
    int32_t pointCount);
uint16_t __cdecl FX_FindModelHead(FxMarksSystem *marksSystem, uint16_t modelIndex, int32_t type);
int32_t __cdecl FX_CompareMarkTris(const FxMarkTri &tri0, const FxMarkTri &tri1);
int32_t __cdecl FX_MarkContextsCompare(const GfxMarkContext *context0, const GfxMarkContext *context1);
void __cdecl FX_MarkEntDetachAll(int32_t localClientNum, int32_t entnum);
void __cdecl FX_MarkEntUpdateHidePartBits(
    const uint32_t *oldHidePartBits,
    const uint32_t *newHidePartBits,
    int32_t localClientNum,
    int32_t entnum);
void __cdecl FX_MarkEntDetachMatchingBones(
    FxMarksSystem *marksSystem,
    int32_t entnum,
    const uint32_t *unsetHidePartBits);
void __cdecl FX_MarkEntUpdateBegin(
    FxMarkDObjUpdateContext *context,
    DObj_s *obj,
    bool isBrush,
    uint16_t brushIndex);
void __cdecl FX_MarkEntUpdateEnd(
    FxMarkDObjUpdateContext *context,
    int32_t localClientNum,
    int32_t entnum,
    DObj_s *obj,
    bool isBrush,
    uint16_t brushIndex);
void __cdecl FX_MarkEntDetachAllOfType(int32_t localClientNum, int32_t entnum, int32_t markType);
void __cdecl FX_MarkEntUpdateEndDObj(FxMarkDObjUpdateContext *context, int32_t localClientNum, int32_t entnum, DObj_s *obj);
void __cdecl FX_MarkEntDetachModel(FxMarksSystem *marksSystem, int32_t entnum, int32_t oldModelIndex);
void __cdecl FX_BeginGeneratingMarkVertsForEntModels(int32_t localClientNum, uint32_t *indexCount);
void __cdecl FX_GenerateMarkVertsForEntXModel(
    int32_t localClientNum,
    int32_t entId,
    uint32_t *indexCount,
    uint16_t lightHandle,
    uint8_t reflectionProbeIndex,
    const GfxScaledPlacement *placement);
char __cdecl FX_GenerateMarkVertsForList_EntXModel(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount,
    uint16_t lightHandleOverride,
    uint8_t reflectionProbeIndexOverride,
    const GfxScaledPlacement *placement);
char __cdecl FX_GenerateMarkVertsForMark_Begin(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    uint32_t *indexCount,
    uint16_t *outBaseVertex,
    FxActiveMarkSurf *outDrawSurf);
void __cdecl FX_DrawMarkTris(
    FxMarksSystem *marksSystem,
    const FxMark *mark,
    uint16_t baseVertex,
    uint16_t *indices,
    FxActiveMarkSurf *outSurf);
void __cdecl FX_EmitMarkTri(
    FxMarksSystem *marksSystem,
    const uint16_t *indices,
    const GfxMarkContext *markContext,
    uint16_t baseVertex,
    FxActiveMarkSurf *outSurf);
void __cdecl FX_GenerateMarkVertsForMark_SetLightHandle(
    FxActiveMarkSurf *drawSurf,
    uint16_t lightHandleOverride);
void __cdecl FX_GenerateMarkVertsForMark_SetReflectionProbeIndex(
    FxActiveMarkSurf *drawSurf,
    uint8_t reflectionProbeIndexOverride);
void __cdecl FX_GenerateMarkVertsForMark_FinishAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    uint16_t baseVertex,
    FxActiveMarkSurf *drawSurf,
    const float (*transform)[3]);
void __cdecl FX_GenerateMarkVertsForMark_MatrixFromScaledPlacement(
    const GfxScaledPlacement *placement,
    const float *viewOffset,
    float (*outTransform)[3]);
void  FX_GenerateMarkVertsForMark_MatrixFromPlacement(
    const GfxPlacement *placement,
    const float *viewOffset,
    float (*outTransform)[3]);
void __cdecl FX_GenerateMarkVertsForEntDObj(
    int32_t localClientNum,
    int32_t entId,
    uint32_t *indexCount,
    uint16_t lightHandle,
    uint8_t reflectionProbeIndex,
    const DObj_s *dobj,
    const cpose_t *pose);
char __cdecl FX_GenerateMarkVertsForList_EntDObj(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount,
    uint16_t lightHandleOverride,
    uint8_t reflectionProbeIndexOverride,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList);
void  FX_GenerateMarkVertsForMark_MatrixFromAnim(
    FxMark *mark,
    const DObj_s *dobj,
    const DObjAnimMat *boneMtxList,
    const vec3r viewOffset,
    mat4x3 &outTransform);
void __cdecl FX_GenerateMarkVertsForEntBrush(
    int32_t localClientNum,
    int32_t entId,
    uint32_t *indexCount,
    uint8_t reflectionProbeIndex,
    const GfxPlacement *placement);
char __cdecl FX_GenerateMarkVertsForList_EntBrush(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount,
    const GfxPlacement *placement,
    uint8_t reflectionProbeIndex);
void __cdecl FX_EndGeneratingMarkVertsForEntModels(int32_t localClientNum);
void __cdecl FX_FinishGeneratingMarkVerts(FxMarksSystem *marksSystem);
void __cdecl FX_GenerateMarkVertsForStaticModels(
    int32_t localClientNum,
    int32_t smodelCount,
    const uint8_t *smodelVisLods);
char __cdecl FX_GenerateMarkVertsForList_WorldXModel(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount);
void __cdecl FX_GenerateMarkVertsForMark_FinishNonAnimated(
    FxMarksSystem *marksSystem,
    FxMark *mark,
    uint16_t baseVertex,
    FxActiveMarkSurf *drawSurf);
void __cdecl FX_GenerateMarkVertsForWorld(int32_t localClientNum);
char __cdecl FX_GenerateMarkVertsForList_WorldBrush(
    FxMarksSystem *marksSystem,
    uint16_t head,
    const FxCamera *camera,
    uint32_t *indexCount);




// fx_Draw
enum FxRandKey : int32_t
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
    int32_t randomSeed;
    float msecLifeSpan;
    float msecElapsed;
    float normTimeUpdateEnd;
    float posWorld[3];
    float velDirWorld[3];
    FxElemVisualState visState;
    FxElemPreVisualState preVisState;
    float physicsLerpFrac;
    int32_t msecDraw;                       // ...
};
static_assert(sizeof(FxDrawState) == 0xA8);

struct FxTrailSegmentDrawState // sizeof=0x3C
{                                       // ...
    FxTrailDef *trailDef;
    float posWorld[3];                  // ...
    float basis[2][3];                  // ...
    float rotation;
    float size[2];
    float uCoord;                       // ...
    uint8_t color[4];           // ...
};
static_assert(sizeof(FxTrailSegmentDrawState) == 0x3C);

struct FxBeam // sizeof=0x34
{                                       // ...
    float begin[3];                     // ...
    float end[3];                       // ...
    GfxColor beginColor;                // ...
    GfxColor endColor;                  // ...
    float beginRadius;                  // ...
    float endRadius;                    // ...
    Material *material;                 // ...
    int32_t segmentCount;                   // ...
    float wiggleDist;                   // ...
};
static_assert(sizeof(FxBeam) == 0x34);

struct FxBeamInfo // sizeof=0x1384
{                                       // ...
    FxBeam beams[96];
    int32_t beamCount;                      // ...
};
static_assert(sizeof(FxBeamInfo) == 0x1384);

struct FxPostLight // sizeof=0x24
{                                       // ...
    float begin[3];                     // ...
    float end[3];                       // ...
    float radius;                       // ...
    GfxColor color;                     // ...
    Material *material;                 // ...
};
static_assert(sizeof(FxPostLight) == 0x24);

struct FxPostLightInfo // sizeof=0xD84
{                                       // ...
    FxPostLight postLights[96];
    int32_t postLightCount;                 // ...
};
static_assert(sizeof(FxPostLightInfo) == 0xD84);

struct FxGenerateVertsCmd // sizeof=0x44
{                                       // ...
    FxSystem *system;
    FxBeamInfo *beamInfo;
    FxPostLightInfo *postLightInfo;
    FxSpriteInfo *spriteInfo;
    int32_t localClientNum;
    float vieworg[3];
    float viewaxis[3][3];
};
static_assert(sizeof(FxGenerateVertsCmd) == 0x44);

void __cdecl FX_EvaluateVisAlpha(FxElemPreVisualState *preVisState, FxElemVisualState *visState);
uint8_t __cdecl FX_InterpolateColor(
    const FxElemVisStateSample *refState,
    float valueLerp,
    float valueLerpInv,
    float sampleLerp,
    float sampleLerpInv,
    int32_t channel);
void __cdecl FX_SetupVisualState(
    const FxElemDef *elemDef,
    const FxEffect *effect,
    int32_t randomSeed,
    float normTimeUpdateEnd,
    FxElemPreVisualState *preVisState);
void __cdecl FX_EvaluateSize(FxElemPreVisualState *preVisState, FxElemVisualState *visState);
double __cdecl FX_InterpolateSize(
    const FxElemVisStateSample *refState,
    int32_t randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float sampleLerpInv,
    int32_t channel);
void __cdecl FX_EvaluateVisualState(FxElemPreVisualState *preVisState, float msecLifeSpan, FxElemVisualState *visState);
double __cdecl FX_IntegrateRotationFromZero(
    const FxElemVisStateSample *refState,
    int32_t randomSeed,
    FxRandKey randomKey,
    float sampleLerp,
    float msecLifeSpan);
void __cdecl FX_EvaluateVisualState_DoLighting(
    FxElemPreVisualState *preVisState,
    FxElemVisualState *visState,
    const FxElemDef *elemDef);
void __cdecl FX_UnpackColor565(
    uint16_t packed,
    uint8_t *outR,
    uint8_t *outG,
    uint8_t *outB);
void __cdecl FX_DrawElem_BillboardSprite(FxDrawState *draw);
void __cdecl FX_GenSpriteVerts(FxDrawState *draw, const float *tangent, const float *binormal, const float *normal);
void __cdecl FX_GetSpriteTexCoords(const FxDrawState *draw, float *s0, float *ds, float *t0, float *dt);
bool __cdecl FX_CullElementForDraw_Sprite(const FxDrawState *draw);
uint32_t __cdecl FX_CullElementForDraw_FrustumPlaneCount(const FxDrawState *draw);
void __cdecl FX_DrawElem_OrientedSprite(FxDrawState *draw);
void __cdecl FX_DrawElem_Tail(FxDrawState *draw);
bool __cdecl FX_CullElementForDraw_Tail(const FxDrawState *draw);
char __cdecl FX_CullCylinder(
    const FxCamera *camera,
    uint32_t frustumPlaneCount,
    const float *posWorld0,
    const float *posWorld1,
    float radius);
void __cdecl FX_DrawElem_Cloud(FxDrawState *draw);
void __cdecl FX_SetPlacement(const FxDrawState *draw, GfxScaledPlacement *placement);
double __cdecl FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int32_t atRestFraction);
double __cdecl FX_InterpolateScale(
    const FxElemVisStateSample *refState,
    int32_t randomSeed,
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
void __cdecl FX_DrawNonSpriteEffect(FxSystem *system, FxEffect *effect, uint32_t elemClass, int32_t drawTime);
void __cdecl FX_DrawElement(FxSystem *system, const FxElemDef *elemDef, const FxElem *elem, FxDrawState *draw);
void __cdecl FX_DrawSpotLight(FxSystem *system);
void __cdecl FX_DrawSpotLightEffect(FxSystem *system, FxEffect *effect, int32_t drawTime);
void __cdecl FX_DrawSpriteElems(FxSystem *system, int32_t drawTime);
void __cdecl FX_DrawTrailsForEffect(FxSystem *system, FxEffect *effect, int32_t drawTime);
void __cdecl FX_DrawTrail(FxSystem *system, FxDrawState *draw, FxTrail *trail);
void __cdecl FX_TrailElem_UncompressBasis(const char (*inBasis)[3], float (*basis)[3]);
void __cdecl FX_GenTrail_IndsForSegment(
    FxDrawState *draw,
    uint16_t reservedBaseVertex,
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
void __cdecl FX_DrawSpriteEffect(FxSystem *system, FxEffect *effect, int32_t drawTime);
void __cdecl FX_GenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl FX_FillGenerateVertsCmd(int32_t localClientNum, FxGenerateVertsCmd *cmd);
void __cdecl FX_EvaluateDistanceFade(FxDrawState *draw);
double __cdecl FX_ClampRangeLerp(float dist, const FxFloatRange *range);
void __cdecl FX_DrawElement_Setup_1_(
    FxSystem* system,
    FxDrawState* draw,
    int32_t elemMsecBegin,
    int32_t elemSequence,
    const float* elemOrigin,
    float* outRealNormTime);

// fx_update_util
void __cdecl FX_OffsetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int32_t randomSeed,
    float *spawnOrigin);
void __cdecl FX_GetOriginForTrailElem(
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t randomSeed,
    float *outOrigin,
    float *outRight,
    float *outUp);
void __cdecl FX_GetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int32_t randomSeed,
    float *spawnOrigin);
void __cdecl FX_TransformPosFromLocalToWorld(const FxSpatialFrame *frame, float *posLocal, float *posWorld);
void __cdecl FX_SpatialFrameToOrientation(const FxSpatialFrame *frame, orientation_t *orient);
void __cdecl FX_OrientationDirToWorldDir(const orientation_t *orient, const float *dir, float *out);
void __cdecl FX_GetOrientation(
    const FxElemDef* elemDef,
    const FxSpatialFrame* frameAtSpawn,
    const FxSpatialFrame* frameNow,
    int32_t randomSeed,
    orientation_t* orient);
char  FX_GenerateBeam_GetFlatDelta(
    const float4x4* clipMtx,
    const float4x4* invClipMtx,
    float4 beamWorldBegin,
    float4 beamWorldEnd,
    float4* outFlatDelta);
void __cdecl FX_GetVelocityAtTime(
    const FxElemDef *elemDef,
    int32_t randomSeed,
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
char __cdecl FX_CullSphere(const FxCamera *camera, uint32_t frustumPlaneCount, const float *posWorld, float radius);
void __cdecl FX_GetElemAxis(
    const FxElemDef *elemDef,
    int32_t randomSeed,
    const orientation_t *orient,
    float msecElapsed,
    mat3x3& axis);
void __cdecl FX_AnglesToOrientedAxis(const float *anglesInRad, const orientation_t *orient, float (*axisOut)[3][3]);


// fx_random
void __cdecl TRACK_fx_random();
void __cdecl FX_RandomDir(int32_t seed, float *dir);
void __cdecl FX_RandomlyRotateAxis(const float (*axisIn)[3], int32_t randomSeed, mat3x3& axisOut);



// fx_sort
struct FxInsertSortElem // sizeof=0x14
{                                       // ...
    int32_t defSortOrder;                   // ...
    float distToCamSq;
    int32_t msecBegin;
    int32_t defIndex;
    uint8_t elemType;
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(FxInsertSortElem) == 0x14);

void __cdecl FX_SortEffects(FxSystem *system);
void __cdecl FX_WaitBeginIteratingOverEffects_Exclusive(FxSystem *system);
bool __cdecl FX_FirstEffectIsFurther(FxEffect *firstEffect, FxEffect *secondEffect);
int32_t __cdecl FX_CalcRunnerParentSortOrder(FxEffect *effect);
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
    uint32_t key;
    const FxEffectDef *effectDef;
};
static_assert(sizeof(FxEffectDefTableEntry) == 0x8);

struct FxEffectDefTable // sizeof=0x2004
{                                       // ...
    int32_t count;
    FxEffectDefTableEntry entries[1024];
};
static_assert(sizeof(FxEffectDefTable) == 0x2004);

void __cdecl FX_Restore(int32_t clientIndex, MemoryFile *memFile);
void __cdecl FX_RestoreEffectDefTable(MemoryFile *memFile, FxEffectDefTable *table);
void __cdecl FX_AddEffectDefTableEntry(FxEffectDefTable *table, uint32_t key, const FxEffectDef *effectDef);
void __cdecl FX_FixupEffectDefHandles(FxSystem *system, FxEffectDefTable *table);
FxEffect *__cdecl FX_EffectFromHandle(FxSystem *system, uint16_t handle);
const FxEffectDef *__cdecl FX_FindEffectDefInTable(const FxEffectDefTable *table, uint32_t key);
void __cdecl FX_RestorePhysicsData(FxSystem *system, MemoryFile *memFile);
FxElemVisuals __cdecl FX_GetElemVisuals(const FxElemDef *elemDef, int32_t randomSeed);
void __cdecl FX_Save(int32_t clientIndex, MemoryFile *memFile);
void __cdecl FX_SaveEffectDefTable(FxSystem *system, MemoryFile *memFile);
void __cdecl FX_SaveEffectDefTable_FastFile(MemoryFile *memFile);
void __cdecl FX_SaveEffectDefTable_LoadObj(MemoryFile* memFile);
void __cdecl FX_SavePhysicsData(FxSystem *system, MemoryFile *memFile);
void __cdecl FX_Archive(int32_t clientIndex, MemoryFile *memFile);

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
    int32_t effectCount;
    int32_t activeElemCount;
    int32_t pendingElemCount;
    int32_t trailCount;
    int32_t activeTrailElemCount;
    int32_t pendingTrailElemCount;
};
static_assert(sizeof(FxProfileEntry) == 0x1C);

void __cdecl FX_DrawProfile(int32_t clientIndex, void(__cdecl *drawFunc)(char *), float *profilePos);
FxProfileEntry *__cdecl FX_GetProfileEntry(const FxEffectDef *effectDef, FxProfileEntry *entryPool, int32_t *entryCount);
void __cdecl FX_ProfileSingleEffect(FxSystem *system, const FxEffect *effect, FxProfileEntry *entry);
int32_t __cdecl FX_CompareProfileEntries(const FxProfileEntry *e0, const FxProfileEntry *e1);
double __cdecl FX_GetProfileEntryCost(const FxProfileEntry *entry);
void __cdecl FX_DrawMarkProfile(int32_t clientIndex, void(__cdecl *drawFunc)(const char *, float *), float *profilePos);
void __cdecl FX_DrawMarkProfile_MarkPrint(
    FxMarksSystem* marksSystem,
    uint16_t head,
    const char* name,
    int32_t index,
    void(__cdecl* drawFunc)(const char*, float*),
    float* profilePos);



// fx_sprite
struct FxSprite // sizeof=0x20
{                                       // ...
    Material *material;
    float pos[3];
    uint8_t rgbaColor[4];
    float radius;
    float minScreenRadius;
    int32_t flags;
};
static_assert(sizeof(FxSprite) == 0x20);

void __cdecl FX_SpriteGenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl FX_GenerateSpriteCodeMeshVerts(FxSprite *sprite, FxGenerateVertsCmd *cmd);
void __cdecl FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
    Material *material,
    const float *pos,
    float radius,
    const uint8_t *rgbaColor,
    char spriteFlags,
    FxGenerateVertsCmd *cmd);
void __cdecl FX_BuildSpriteCodeMeshVerts(
    Material *material,
    const float *pos,
    float worldRadius,
    const uint8_t *rgbaColor,
    char spriteFlags);
void __cdecl FX_BuildQuadStampCodeMeshVerts(
    Material *material,
    const float *viewAxis,
    const float *origin,
    const float *left,
    const float *up,
    const uint8_t *rgbaColor,
    int32_t s0,
    int32_t t0,
    int32_t s1,
    int32_t t1);
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
    const uint8_t* rgbaColor,
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
enum FxUpdateResult : int32_t
{                                       // ...
    FX_UPDATE_REMOVE = 0x0,
    FX_UPDATE_KEEP = 0x1,
};

void __cdecl FX_SpawnAllFutureLooping(
    FxSystem *system,
    FxEffect *effect,
    int32_t elemDefFirst,
    int32_t elemDefCount,
    const FxSpatialFrame *frameBegin,
    const FxSpatialFrame *frameEnd,
    long double msecWhenPlayed,
    long double mescUpdateBegin,
    long double msecUpdateEnd);
void __cdecl FX_SpawnLoopingElems(
    FxSystem *system,
    FxEffect *effect,
    int32_t elemDefIndex,
    const FxSpatialFrame *frameBegin,
    const FxSpatialFrame *frameEnd,
    long double msecWhenPlayed,
    long double msecUpdateBegin,
    long double msecUpdateEnd);
int32_t __cdecl FX_LimitStabilizeTimeForElemDef_Recurse(
    const FxElemDef *elemDef,
    bool needToSpawnSystem,
    int32_t originalUpdateTime);
int32_t __cdecl FX_LimitStabilizeTimeForElemDef_SelfOnly(const FxElemDef *elemDef, bool needToSpawnSystem);
int32_t __cdecl FX_LimitStabilizeTimeForEffectDef_Recurse(const FxEffectDef *remoteEffectDef, int32_t originalUpdateTime);
void __cdecl FX_BeginLooping(
    FxSystem* system,
    FxEffect* effect,
    int32_t elemDefFirst,
    int32_t elemDefCount,
    FxSpatialFrame* frameWhenPlayed,
    FxSpatialFrame* a2,
    int32_t msecWhenPlayed,
    int32_t msecNow);
void __cdecl FX_SpawnTrailLoopingElems(
    FxSystem* system,
    FxEffect* effect,
    FxTrail* trail,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int32_t msecWhenPlayed,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd);
void __cdecl FX_TriggerOneShot(
    FxSystem *system,
    FxEffect *effect,
    int32_t elemDefFirst,
    int32_t elemDefCount,
    const FxSpatialFrame *frameWhenPlayed,
    int32_t msecWhenPlayed);
void __cdecl FX_SpawnOneShotElems(
    FxSystem *system,
    FxEffect *effect,
    int32_t elemDefIndex,
    const FxSpatialFrame *frameWhenPlayed,
    int32_t msecWhenPlayed);
void __cdecl FX_StartNewEffect(FxSystem* system, FxEffect* effect);
bool __cdecl FX_GetBoltTemporalBits(int32_t localClientNum, int32_t dobjHandle);
char __cdecl FX_GetBoneOrientation(int32_t localClientNum, uint32_t dobjHandle, int32_t boneIndex, orientation_t *orient);
bool __cdecl FX_GetBoneOrientation_IsDObjEntityValid(int32_t localClientNum, int32_t dobjHandle);
void __cdecl FX_UpdateEffectPartial(
    FxSystem* system,
    FxEffect* effect,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd,
    uint16_t* elemHandleStart,
    uint16_t* elemHandleStop,
    uint16_t* trailElemStart,
    uint16_t* trailElemStop);
void __cdecl FX_ProcessLooping(
    FxSystem* system,
    FxEffect* effect,
    int32_t elemDefFirst,
    int32_t elemDefCount,
    FxSpatialFrame* frameBegin,
    FxSpatialFrame* frameEnd,
    int32_t msecWhenPlayed,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd);
void __cdecl FX_UpdateEffectPartialForClass(
    FxSystem *system,
    FxEffect *effect,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    uint16_t elemHandleStart,
    uint16_t elemHandleStop,
    uint32_t elemClass);
FxUpdateResult __cdecl FX_UpdateElement(
    FxSystem *system,
    FxEffect *effect,
    FxElem *elem,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd);
const FxElemDef *__cdecl FX_GetUpdateElemDef(const FxUpdateElem *update);
double __cdecl FX_GetAtRestFraction(const FxUpdateElem *update, float msec);
int32_t __cdecl FX_UpdateElementPosition(FxSystem *system, FxUpdateElem *update);
int32_t __cdecl FX_UpdateElementPosition_Colliding(FxSystem *system, FxUpdateElem *update);
int32_t __cdecl FX_UpdateElementPosition_CollidingStep(
    FxSystem *system,
    FxUpdateElem *update,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float *xyzWorldOld);
void __cdecl FX_NextElementPosition(FxUpdateElem *update, int32_t msecUpdateBegin, int32_t msecUpdateEnd);
void __cdecl FX_NextElementPosition_NoExternalForces(
    FxUpdateElem* update,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float* posLocal,
    float* posWorld);
void __cdecl FX_IntegrateVelocity(const FxUpdateElem *update, float t0, float t1, float *posLocal, float *posWorld);
void __cdecl FX_IntegrateVelocityAcrossSegments(
    int32_t elemDefFlags,
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
    int32_t elemDefFlags,
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
int32_t __cdecl FX_CollisionResponse(
    FxSystem *system,
    FxUpdateElem *update,
    const trace_t *trace,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float *xyzWorldOld);
void __cdecl FX_SpawnImpactEffect(
    FxSystem *system,
    const FxUpdateElem *update,
    const FxEffectDef *impactEffect,
    int32_t msecOnImpact,
    const float *impactNormal);
int32_t __cdecl FX_UpdateElementPosition_NonColliding(FxUpdateElem *update);
int32_t __cdecl FX_UpdateElementPosition_Local(FxUpdateElem *update);
void __cdecl FX_SpawnDeathEffect(FxSystem *system, FxUpdateElem *update);
char __cdecl FX_UpdateElement_SetupUpdate(
    FxEffect *effect,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    uint32_t elemDefIndex,
    int32_t elemAtRestFraction,
    int32_t elemMsecBegin,
    int32_t elemSequence,
    float *elemOrigin,
    FxUpdateElem *update);
void __cdecl FX_UpdateElement_TruncateToElemEnd(FxUpdateElem *update, FxUpdateResult *outUpdateResult);
void __cdecl FX_UpdateElement_HandleEmitting(
    FxSystem *system,
    FxElem *elem,
    FxUpdateElem *update,
    const float *elemOriginPrev,
    FxUpdateResult *outUpdateResult);
uint8_t __cdecl FX_ProcessEmitting(
    FxSystem *system,
    FxUpdateElem *update,
    uint8_t emitResidual,
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
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd,
    float distanceTravelledBegin,
    float distanceTravelledEnd,
    uint16_t trailElemHandleStart,
    uint16_t trailElemHandleStop,
    FxSpatialFrame *frameNow);
void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3]);
FxUpdateResult __cdecl FX_UpdateTrailElement(
    FxSystem *system,
    FxEffect *effect,
    FxTrail *trail,
    FxTrailElem *trailElem,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd);
void __cdecl FX_UpdateSpotLight(FxCmd *cmd);
void __cdecl FX_UpdateSpotLightEffect(FxSystem* system, FxEffect* effect);
void __cdecl FX_UpdateSpotLightEffectPartial(
    FxSystem* system,
    FxEffect* effect,
    int32_t msecUpdateBegin,
    int32_t msecUpdateEnd);
void __cdecl FX_UpdateEffectBolt(FxSystem *system, FxEffect *effect);
void __cdecl FX_UpdateNonDependent(FxCmd *cmd);
void __cdecl FX_Update(FxSystem *system, int32_t localClientNum, bool nonBoltedEffectsOnly);
void __cdecl FX_UpdateEffect(FxSystem* system, FxEffect* effect);
bool __cdecl FX_ShouldProcessEffect(FxSystem *system, FxEffect *effect, bool nonBoltedEffectsOnly);
void __cdecl FX_RunPhysics(int32_t localClientNum);
void __cdecl FX_UpdateRemaining(FxCmd *cmd);
void __cdecl FX_BeginUpdate(int32_t localClientNum);
void __cdecl FX_EndUpdate(int32_t localClientNum);
void __cdecl FX_AddNonSpriteDrawSurfs(FxCmd *cmd);
void __cdecl FX_RewindTo(int32_t localClientNum, int32_t time);
void __cdecl FX_SetNextUpdateCamera(int32_t localClientNum, const refdef_s *refdef, float zfar);
void __cdecl FX_SetNextUpdateTime(int32_t localClientNum, int32_t time);
void __cdecl FX_FillUpdateCmd(int32_t localClientNum, FxCmd *cmd);



// fx_curve
struct FxCurve // sizeof=0xC
{
    int32_t dimensionCount;
    int32_t keyCount;
    float keys[1];
};
static_assert(sizeof(FxCurve) == 0xC);

struct FxCurveIterator // sizeof=0x8
{                                       // ...
    const FxCurve *master;
    int32_t currentKeyIndex;
};
static_assert(sizeof(FxCurveIterator) == 0x8);

double __cdecl FxCurve_Interpolate1d(const float *key, float intermediateTime);
void __cdecl FxCurve_Interpolate3d(const float *key, float intermediateTime, float *result);
void __cdecl FxCurveIterator_Create(FxCurveIterator *createe, const FxCurve *master);
void __cdecl FxCurveIterator_Release(FxCurveIterator *releasee);

const FxCurve *__cdecl FxCurve_AllocAndCreateWithKeys(float *keyArray, int32_t dimensionCount, int32_t keyCount);
void __cdecl FxCurveIterator_SampleTimeVec3(FxCurveIterator *source, float *replyVector, float time);
double __cdecl FxCurveIterator_SampleTime(FxCurveIterator *source, float time);
void __cdecl FxCurveIterator_MoveToTime(FxCurveIterator *source, float time);



// fx_load_obj
struct FxEditorElemAtlas // sizeof=0x1C
{                                       // ...
    int32_t behavior;
    int32_t index;
    int32_t fps;
    int32_t loopCount;
    int32_t colIndexBits;
    int32_t rowIndexBits;
    int32_t entryCount;
};
static_assert(sizeof(FxEditorElemAtlas) == 0x1C);

struct FxEditorTrailDef // sizeof=0x608
{                                       // ...
    FxTrailVertex verts[64];
    int32_t vertCount;
    uint16_t inds[128];
    int32_t indCount;
};
static_assert(sizeof(FxEditorTrailDef) == 0x608);

struct FxEditorElemDef // sizeof=0x858
{                                       // ...
    char name[48];
    int32_t editorFlags;
    int32_t flags;
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
    int32_t sortOrder;
    const FxEffectDef *emission;
    FxFloatRange emitDist;
    FxFloatRange emitDistVariance;
    uint8_t elemType;
    // padding byte
    // padding byte
    // padding byte
    int32_t visualCount;
    //$6DCA2FC3F9FD742A3C1907AE7E70399A ___u41;
    union
    {
        FxElemVisuals visuals[32];
        FxElemMarkVisuals markVisuals[16];
    }visualunion;
    int32_t trailSplitDist;
    int32_t trailRepeatDist;
    float trailScrollTime;
    FxEditorTrailDef trailDef;
};
static_assert(sizeof(FxEditorElemDef) == 0x858);

struct FxEditorEffectDef // sizeof=0x10B44
{                                       // ...
    char name[64];
    int32_t elemCount;
    FxEditorElemDef elems[32];
};
static_assert(sizeof(FxEditorEffectDef) == 0x10B44);

struct FxElemField // sizeof=0x8
{                                       // ...
    const char *keyName;                // ...
    bool(__cdecl *handler)(const char **, FxEditorElemDef *); // ...
};
static_assert(sizeof(FxElemField) == 0x8);

struct FxFlagOutputSet // sizeof=0xC
{                                       // ...
    int32_t *flags[3];                      // ...
};
static_assert(sizeof(FxFlagOutputSet) == 0xC);

struct FxFlagDef // sizeof=0x10
{
    const char *name;
    int32_t flagType;
    int32_t mask;
    int32_t value;
};
static_assert(sizeof(FxFlagDef) == 0x10);

enum FxSampleChannel : int32_t
{                                       // ...
    FX_CHAN_RGBA = 0x0,
    FX_CHAN_SIZE_0 = 0x1,
    FX_CHAN_SIZE_1 = 0x2,
    FX_CHAN_SCALE = 0x3,
    FX_CHAN_ROTATION = 0x4,
    FX_CHAN_COUNT = 0x5,
    FX_CHAN_NONE = 0x6,
};

const FxEffectDef *__cdecl FX_Register(const char *name);
const FxEffectDef *__cdecl FX_Register_FastFile(const char *name);
void __cdecl FX_RegisterDefaultEffect();
struct PhysPreset *__cdecl FX_RegisterPhysPreset(const char *name);
void __cdecl FX_ForEachEffectDef(void(__cdecl* callback)(const FxEffectDef*, void*), void* data);
void FX_UnregisterAll();

// fx_convert
const FxEffectDef *__cdecl FX_Convert(const FxEditorEffectDef *editorEffect, void *(* Alloc)(uint32_t));
int32_t __cdecl FX_DecideIntervalLimit(const FxEditorElemDef *edElemDef);

extern const float fx_randomTable[];
extern int32_t fx_serverVisClient;