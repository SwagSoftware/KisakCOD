#pragma once
#include <cstdint>

#include <qcommon/qcommon.h>

#include <physics/phys_local.h>

#include <xanim/xanim.h>
#include <xanim/dobj.h>

const float traceOffsets[5][2] = { { 0.0, 0.0 }, { 1.0, 1.0 }, { -1.0, 1.0 }, { 1.0, -1.0 }, { -1.0, -1.0 } }; // idb

#define DYNENT_CL_ACTIVE 1


enum DynEntityType : int32_t
{                                       // ...
    DYNENT_TYPE_INVALID = 0x0,
    DYNENT_TYPE_CLUTTER = 0x1,
    DYNENT_TYPE_DESTRUCT = 0x2,
    DYNENT_TYPE_COUNT = 0x3,
};

enum DynEntityCollType : int32_t
{                                       // ...
    DYNENT_COLL_CLIENT_FIRST = 0x0,
    DYNENT_COLL_CLIENT_MODEL = 0x0,
    DYNENT_COLL_CLIENT_BRUSH = 0x1,
    DYNENT_COLL_COUNT = 0x2,
};

struct DynEntityDef // sizeof=0x60
{
    DynEntityType type;
    GfxPlacement pose;
    XModel *xModel;
    uint16_t brushModel;
    uint16_t physicsBrushModel;
    const FxEffectDef *destroyFx;
    XModelPieces *destroyPieces;
    PhysPreset *physPreset;
    int32_t health;
    PhysMass mass;
    int32_t contents;
};
static_assert(sizeof(DynEntityDef) == 0x60);

struct DynEntityPose // sizeof=0x20
{
    GfxPlacement pose;
    float radius;
};
static_assert(sizeof(DynEntityPose) == 0x20);;

struct DynEntityClient // sizeof=0xC
{
    int32_t physObjId;
    uint16_t flags;
    uint16_t lightingHandle;
    int32_t health;
};
static_assert(sizeof(DynEntityClient) == 0xC);

struct DynEntityColl // sizeof=0x14
{
    uint16_t sector;
    uint16_t nextEntInSector;
    float linkMins[2];
    float linkMaxs[2];
};
static_assert(sizeof(DynEntityColl) == 0x14);

struct DynEntityAreaParms // sizeof=0x14
{                                       // ...
    const float *mins;                  // ...
    const float *maxs;                  // ...
    int32_t contentMask;                    // ...
    uint16_t *list;             // ...
    uint16_t maxCount;          // ...
    uint16_t count;             // ...
};
static_assert(sizeof(DynEntityAreaParms) == 0x14);

struct DynEntSortStruct // sizeof=0x8
{
    float distSq;
    uint16_t id;
    // padding byte
    // padding byte
};
static_assert(sizeof(DynEntSortStruct) == 0x8);

//std::pair<DynEntSortStruct *first, DynEntSortStruct *second>; // sizeof=0x8

struct BreakablePiece // sizeof=0xC
{                                       // ...
    const XModel *model;                // ...
    int32_t physObjId;                      // ...
    uint16_t lightingHandle;    // ...
    bool active;                        // ...
    // padding byte
};
static_assert(sizeof(BreakablePiece) == 0xC);

struct pointtrace_t;
struct trace_t;
struct moveclip_t;

void __cdecl DynEntCl_RegisterDvars();
void __cdecl DynEntCl_InitEntities(int32_t localClientNum);
void __cdecl DynEntCl_LinkModel(uint16_t dynEntId);
void __cdecl DynEntCl_LinkBrush(uint16_t dynEntId);
double __cdecl DynEntCl_UpdateBModelWorldBounds(const DynEntityDef *dynEntDef, const GfxPlacement *pose);
void __cdecl DynEntCl_ProcessEntities(int32_t localClientNum);
void __cdecl DynEntCl_Shutdown(int32_t localClientNum);
void __cdecl DynEntCl_UnlinkEntity(uint16_t dynEntId, DynEntityCollType drawType);
void __cdecl DynEntCl_PointTrace(const pointtrace_t *clip, trace_t *results);
void __cdecl DynEntCl_PointTrace_r(
    DynEntityCollType drawType,
    const pointtrace_t *clip,
    uint32_t sectorIndex,
    float *p1,
    float *p2,
    trace_t *results);
void __cdecl DynEntCl_ClipMoveTrace(const moveclip_t *clip, trace_t *results);
void __cdecl DynEntCl_ClipMoveTrace_r(
    const moveclip_t *clip,
    uint32_t sectorIndex,
    float *p1,
    float *p2,
    trace_t *results);
uint16_t __cdecl DynEntCl_AreaEntities(
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs,
    int32_t contentMask,
    uint16_t dynEntMaxCount,
    uint16_t *dynEntList);
void __cdecl DynEntCl_AreaEntities_r(
    DynEntityCollType drawType,
    uint32_t sectorIndex,
    DynEntityAreaParms *areaParms);
void __cdecl DynEntCl_EntityImpactEvent(
    const trace_t *trace,
    int32_t localClientNum,
    int32_t sourceEntityNum,
    const float *start,
    const float *hitPos,
    bool isMelee);
void __cdecl DynEntCl_PlayImpactEffects(
    int32_t localClientNum,
    uint32_t sourceEntityNum,
    uint32_t surfType,
    const float *hitPos,
    const float *hitNormal);
void __cdecl DynEntCl_PlayEventFx(const FxEffectDef *def, const float *origin, const float (*axis)[3]);
char __cdecl DynEntCl_EventNeedsProcessed(int32_t localClientNum, int32_t sourceEntityNum);
char __cdecl DynEntCl_DynEntImpactEvent(
    int32_t localClientNum,
    int32_t sourceEntityNum,
    float *start,
    float *end,
    int32_t damage,
    bool isMelee);
dxBody *__cdecl DynEntCl_CreatePhysObj(const DynEntityDef *dynEntDef, const GfxPlacement *pose);
void __cdecl DynEntCl_Damage(
    int32_t localClientNum,
    uint16_t dynEntId,
    DynEntityCollType drawType,
    const float *hitPos,
    const float *hitDir,
    int32_t damage);
void __cdecl DynEntCl_TestPhysicsEntities(
    int32_t localClientNum,
    int32_t sourceEntityNum,
    float *start,
    float *end,
    bool isMelee);
void __cdecl DynEntCl_MeleeEvent(int32_t localClientNum, int32_t sourceEntityNum);
void __cdecl DynEntCl_ExplosionEvent(
    int32_t localClientNum,
    bool isCylinder,
    float *origin,
    float innerRadius,
    float outerRadius,
    float *impulse,
    float inScale,
    int32_t innerDamage,
    int32_t outerDamage);
uint32_t __cdecl DynEntCl_GetClosestEntities(
    DynEntityDrawType drawType,
    float *radiusMins,
    float *radiusMaxs,
    float *origin,
    uint16_t *hitEnts,
    bool isCylinder);
bool __cdecl DynEntCl_CompareDynEntsForExplosion(const DynEntSortStruct& ent1, const DynEntSortStruct& ent2);
void __cdecl DynEntCl_JitterEvent(
    int32_t localClientNum,
    float *origin,
    float innerRadius,
    float outerRadius,
    float minDisplacement,
    float maxDisplacement);
void __cdecl DynEntCl_DestroyEvent(
    int32_t localClientNum,
    uint16_t dynEntId,
    DynEntityCollType drawType,
    const float *hitPos,
    const float *hitDir);
#ifdef KISAK_SP
void DynEntCl_WakeUpAroundPlayer(int localClientNum);
#endif



// DynEntity_pieces
void __cdecl DynEntPieces_RegisterDvars();
void __cdecl DynEntPieces_AddDrawSurfs();
void __cdecl DynEntPieces_SpawnPieces(
    int32_t localClientNum,
    const XModelPieces *pieces,
    const float *origin,
    const float (*axis)[3],
    const float *hitPos,
    const float *hitDir);
bool __cdecl DynEntPieces_SpawnPhysicsModel(
    int32_t localClientNum,
    const XModel *model,
    const float *offset,
    const float *origin,
    const float (*axis)[3],
    const float *hitPos,
    const float *hitDir);
dxBody *__cdecl DynEntPieces_SpawnPhysObj(
    const char *modelName,
    const float *mins,
    const float *maxs,
    float *position,
    float *quat,
    float *velocity,
    float *angularVelocity,
    const PhysPreset *physPreset);
void __cdecl DynEntPieces_CalcForceDir(const float *hitDir, float spreadFraction, float *forceDir);



// dynentity_load_obj
struct DynEntityProps // sizeof=0x8
{                                       // ...
    const char *name;                   // ...
    bool clientOnly;                    // ...
    bool clipMove;
    bool usePhysics;
    bool destroyable;
};
static_assert(sizeof(DynEntityProps) == 0x8);

struct DynEntityCreateParams // sizeof=0x1C0
{                                       // ...
    char typeName[64];
    char modelName[64];                 // ...
    char physModelName[64];             // ...
    char destroyFxFile[64];             // ...
    char destroyPiecesFile[64];         // ...
    char physPresetFile[64];            // ...
    float origin[3];                    // ...
    float angles[3];                    // ...
    int32_t health;                         // ...
    float centerOfMass[3];              // ...
    float momentsOfInertia[3];          // ...
    float productsOfInertia[3];         // ...
};
static_assert(sizeof(DynEntityCreateParams) == 0x1C0);

void __cdecl DynEnt_LoadEntities();
const DynEntityProps *__cdecl DynEnt_GetEntityProps(DynEntityType dynEntType);
uint16_t __cdecl DynEnt_GetId(const DynEntityDef *dynEntDef, DynEntityDrawType drawType);
uint16_t __cdecl DynEnt_GetEntityCount(DynEntityCollType collType);
const DynEntityDef *__cdecl DynEnt_GetEntityDef(uint16_t dynEntId, DynEntityDrawType drawType);
DynEntityPose *__cdecl DynEnt_GetClientModelPoseList();
DynEntityPose *__cdecl DynEnt_GetClientPose(uint16_t dynEntId, DynEntityDrawType drawType);
DynEntityClient *__cdecl DynEnt_GetClientEntity(uint16_t dynEntId, DynEntityDrawType drawType);
DynEntityColl *__cdecl DynEnt_GetEntityColl(DynEntityCollType collType, uint16_t dynEntId);
int32_t __cdecl DynEnt_GetXModelUsageCount(const XModel *xModel);



// DynEntity_coll
union DynEntityCollTree_u // sizeof=0x2
{                                       // ...
    uint16_t parent;
    uint16_t nextFree;
};
static_assert(sizeof(DynEntityCollTree_u) == 0x2);

struct DynEntityCollTree // sizeof=0xC
{                                       // ...
    float dist;
    uint16_t axis;
    DynEntityCollTree_u u;
    uint16_t child[2];
};
static_assert(sizeof(DynEntityCollTree) == 0xC);

struct DynEntityCollSector // sizeof=0x14
{                                       // ...
    DynEntityCollTree tree;
    int32_t contents;
    uint16_t entListHead;
    // padding byte
    // padding byte
};
static_assert(sizeof(DynEntityCollSector) == 0x14);

struct DynEntityCollWorld // sizeof=0x501C
{                                       // ...
    float mins[3];
    float maxs[3];
    uint16_t freeHead;
    // padding byte
    // padding byte
    DynEntityCollSector sectors[1024];
};
static_assert(sizeof(DynEntityCollWorld) == 0x501C);

void __cdecl TRACK_DynEntityCollWorld();
DynEntityCollSector *__cdecl DynEnt_GetCollSector(DynEntityCollType collType, uint32_t sectorIndex);
void __cdecl DynEnt_ClearCollWorld(DynEntityCollType collType);
void __cdecl DynEnt_UnlinkEntity(DynEntityCollType collType, uint16_t dynEntId);
void __cdecl DynEnt_LinkEntity(
    DynEntityCollType collType,
    uint16_t dynEntId,
    const float *absMins,
    const float *absMaxs);
void __cdecl DynEnt_AddToCollSector(
    DynEntityCollType collType,
    uint16_t dynEntId,
    uint16_t sectorIndex);
void __cdecl DynEnt_SortCollSector(
    DynEntityCollType collType,
    uint16_t sectorIndex,
    const float *mins,
    const float *maxs);
uint16_t __cdecl DynEnt_AllocCollSector(DynEntityCollType collType, const float *mins, const float *maxs);
int32_t __cdecl DynEnt_GetContents(const DynEntityDef *dynEntDef);
void __cdecl DynEnt_GetLocalBounds(const DynEntityDef *dynEntDef, float *mins, float *maxs);
void __cdecl DynEnt_GetWorldBounds(const DynEntityPose *dynEntPose, float *mins, float *maxs);
double __cdecl DynEnt_GetRadiusDistSqr(const DynEntityPose *dynEntPose, const float *origin);
double __cdecl DynEnt_GetCylindricalRadiusDistSqr(const DynEntityPose *dynEntPose, const float *origin);
bool __cdecl DynEnt_EntityInArea(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const float *mins,
    const float *maxs,
    int32_t contentMask);
void __cdecl DynEnt_PointTraceToModel(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const pointtrace_t *clip,
    trace_t *results);
void __cdecl DynEnt_PointTraceToBrush(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const pointtrace_t *clip,
    trace_t *results);
void __cdecl DynEnt_ClipMoveTraceToBrush(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const moveclip_t *clip,
    trace_t *results);
void __cdecl DynEnt_SetPhysObjCollision(const DynEntityDef *dynEntDef, dxBody *physId);


extern int32_t numPieces;
extern BreakablePiece g_breakablePieces[100];

