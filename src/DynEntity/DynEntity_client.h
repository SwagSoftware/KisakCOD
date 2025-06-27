#pragma once

#include <qcommon/qcommon.h>

#include <physics/phys_local.h>

#include <xanim/xanim.h>
#include <xanim/dobj.h>

const float traceOffsets[5][2] = { { 0.0, 0.0 }, { 1.0, 1.0 }, { -1.0, 1.0 }, { 1.0, -1.0 }, { -1.0, -1.0 } }; // idb

#define DYNENT_CL_ACTIVE 1


enum DynEntityType : __int32
{                                       // ...
    DYNENT_TYPE_INVALID = 0x0,
    DYNENT_TYPE_CLUTTER = 0x1,
    DYNENT_TYPE_DESTRUCT = 0x2,
    DYNENT_TYPE_COUNT = 0x3,
};

enum DynEntityCollType : __int32
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
    unsigned __int16 brushModel;
    unsigned __int16 physicsBrushModel;
    const FxEffectDef *destroyFx;
    XModelPieces *destroyPieces;
    PhysPreset *physPreset;
    int health;
    PhysMass mass;
    int contents;
};

struct DynEntityPose // sizeof=0x20
{
    GfxPlacement pose;
    float radius;
};
struct DynEntityClient // sizeof=0xC
{
    int physObjId;
    unsigned __int16 flags;
    unsigned __int16 lightingHandle;
    int health;
};
struct DynEntityColl // sizeof=0x14
{
    unsigned __int16 sector;
    unsigned __int16 nextEntInSector;
    float linkMins[2];
    float linkMaxs[2];
};
struct DynEntityAreaParms // sizeof=0x14
{                                       // ...
    const float *mins;                  // ...
    const float *maxs;                  // ...
    int contentMask;                    // ...
    unsigned __int16 *list;             // ...
    unsigned __int16 maxCount;          // ...
    unsigned __int16 count;             // ...
};

struct DynEntSortStruct // sizeof=0x8
{
    float distSq;
    unsigned __int16 id;
    // padding byte
    // padding byte
};
//std::pair<DynEntSortStruct *first, DynEntSortStruct *second>; // sizeof=0x8

struct BreakablePiece // sizeof=0xC
{                                       // ...
    const XModel *model;                // ...
    int physObjId;                      // ...
    unsigned __int16 lightingHandle;    // ...
    bool active;                        // ...
    // padding byte
};

struct pointtrace_t;
struct trace_t;
struct moveclip_t;

void __cdecl DynEntCl_RegisterDvars();
void __cdecl DynEntCl_InitEntities(int localClientNum);
void __cdecl DynEntCl_LinkModel(unsigned __int16 dynEntId);
void __cdecl DynEntCl_LinkBrush(unsigned __int16 dynEntId);
double __cdecl DynEntCl_UpdateBModelWorldBounds(const DynEntityDef *dynEntDef, const GfxPlacement *pose);
void __cdecl DynEntCl_ProcessEntities(int localClientNum);
void __cdecl DynEntCl_Shutdown(int localClientNum);
void __cdecl DynEntCl_UnlinkEntity(unsigned __int16 dynEntId, DynEntityCollType drawType);
void __cdecl DynEntCl_PointTrace(const pointtrace_t *clip, trace_t *results);
void __cdecl DynEntCl_PointTrace_r(
    DynEntityCollType drawType,
    const pointtrace_t *clip,
    unsigned int sectorIndex,
    float *p1,
    float *p2,
    trace_t *results);
void __cdecl DynEntCl_ClipMoveTrace(const moveclip_t *clip, trace_t *results);
void __cdecl DynEntCl_ClipMoveTrace_r(
    const moveclip_t *clip,
    unsigned int sectorIndex,
    float *p1,
    float *p2,
    trace_t *results);
unsigned __int16 __cdecl DynEntCl_AreaEntities(
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs,
    int contentMask,
    unsigned __int16 dynEntMaxCount,
    unsigned __int16 *dynEntList);
void __cdecl DynEntCl_AreaEntities_r(
    DynEntityCollType drawType,
    unsigned int sectorIndex,
    DynEntityAreaParms *areaParms);
void __cdecl DynEntCl_EntityImpactEvent(
    const trace_t *trace,
    int localClientNum,
    int sourceEntityNum,
    const float *start,
    const float *hitPos,
    bool isMelee);
void __cdecl DynEntCl_PlayImpactEffects(
    int localClientNum,
    unsigned int sourceEntityNum,
    unsigned int surfType,
    const float *hitPos,
    const float *hitNormal);
void __cdecl DynEntCl_PlayEventFx(const FxEffectDef *def, const float *origin, const float (*axis)[3]);
char __cdecl DynEntCl_EventNeedsProcessed(int localClientNum, int sourceEntityNum);
char __cdecl DynEntCl_DynEntImpactEvent(
    int localClientNum,
    int sourceEntityNum,
    float *start,
    float *end,
    int damage,
    bool isMelee);
dxBody *__cdecl DynEntCl_CreatePhysObj(const DynEntityDef *dynEntDef, const GfxPlacement *pose);
void __cdecl DynEntCl_Damage(
    int localClientNum,
    unsigned __int16 dynEntId,
    DynEntityCollType drawType,
    const float *hitPos,
    const float *hitDir,
    int damage);
void __cdecl DynEntCl_TestPhysicsEntities(
    int localClientNum,
    int sourceEntityNum,
    float *start,
    float *end,
    bool isMelee);
void __cdecl DynEntCl_MeleeEvent(int localClientNum, int sourceEntityNum);
void __cdecl DynEntCl_ExplosionEvent(
    int localClientNum,
    bool isCylinder,
    float *origin,
    float innerRadius,
    float outerRadius,
    float *impulse,
    float inScale,
    int innerDamage,
    int outerDamage);
unsigned int __cdecl DynEntCl_GetClosestEntities(
    DynEntityDrawType drawType,
    float *radiusMins,
    float *radiusMaxs,
    float *origin,
    unsigned __int16 *hitEnts,
    bool isCylinder);
bool __cdecl DynEntCl_CompareDynEntsForExplosion(const DynEntSortStruct& ent1, const DynEntSortStruct& ent2);
void __cdecl DynEntCl_JitterEvent(
    int localClientNum,
    float *origin,
    float innerRadius,
    float outerRadius,
    float minDisplacement,
    float maxDisplacement);
void __cdecl DynEntCl_DestroyEvent(
    int localClientNum,
    unsigned __int16 dynEntId,
    DynEntityCollType drawType,
    const float *hitPos,
    const float *hitDir);



// DynEntity_pieces
void __cdecl DynEntPieces_RegisterDvars();
void __cdecl DynEntPieces_AddDrawSurfs();
void __cdecl DynEntPieces_SpawnPieces(
    int localClientNum,
    const XModelPieces *pieces,
    const float *origin,
    const float (*axis)[3],
    const float *hitPos,
    const float *hitDir);
bool __cdecl DynEntPieces_SpawnPhysicsModel(
    int localClientNum,
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
    int health;                         // ...
    float centerOfMass[3];              // ...
    float momentsOfInertia[3];          // ...
    float productsOfInertia[3];         // ...
};
void __cdecl DynEnt_LoadEntities();
const DynEntityProps *__cdecl DynEnt_GetEntityProps(DynEntityType dynEntType);
unsigned __int16 __cdecl DynEnt_GetId(const DynEntityDef *dynEntDef, DynEntityDrawType drawType);
unsigned __int16 __cdecl DynEnt_GetEntityCount(DynEntityCollType collType);
const DynEntityDef *__cdecl DynEnt_GetEntityDef(unsigned __int16 dynEntId, DynEntityDrawType drawType);
DynEntityPose *__cdecl DynEnt_GetClientModelPoseList();
DynEntityPose *__cdecl DynEnt_GetClientPose(unsigned __int16 dynEntId, DynEntityDrawType drawType);
DynEntityClient *__cdecl DynEnt_GetClientEntity(unsigned __int16 dynEntId, DynEntityDrawType drawType);
DynEntityColl *__cdecl DynEnt_GetEntityColl(DynEntityCollType collType, unsigned __int16 dynEntId);
int __cdecl DynEnt_GetXModelUsageCount(const XModel *xModel);



// DynEntity_coll
union DynEntityCollTree_u // sizeof=0x2
{                                       // ...
    unsigned __int16 parent;
    unsigned __int16 nextFree;
};
struct DynEntityCollTree // sizeof=0xC
{                                       // ...
    float dist;
    unsigned __int16 axis;
    DynEntityCollTree_u u;
    unsigned __int16 child[2];
};
struct DynEntityCollSector // sizeof=0x14
{                                       // ...
    DynEntityCollTree tree;
    int contents;
    unsigned __int16 entListHead;
    // padding byte
    // padding byte
};
struct DynEntityCollWorld // sizeof=0x501C
{                                       // ...
    float mins[3];
    float maxs[3];
    unsigned __int16 freeHead;
    // padding byte
    // padding byte
    DynEntityCollSector sectors[1024];
};
void __cdecl TRACK_DynEntityCollWorld();
DynEntityCollSector *__cdecl DynEnt_GetCollSector(DynEntityCollType collType, unsigned int sectorIndex);
void __cdecl DynEnt_ClearCollWorld(DynEntityCollType collType);
void __cdecl DynEnt_UnlinkEntity(DynEntityCollType collType, unsigned __int16 dynEntId);
void __cdecl DynEnt_LinkEntity(
    DynEntityCollType collType,
    unsigned __int16 dynEntId,
    const float *absMins,
    const float *absMaxs);
void __cdecl DynEnt_AddToCollSector(
    DynEntityCollType collType,
    unsigned __int16 dynEntId,
    unsigned __int16 sectorIndex);
void __cdecl DynEnt_SortCollSector(
    DynEntityCollType collType,
    unsigned __int16 sectorIndex,
    const float *mins,
    const float *maxs);
unsigned __int16 __cdecl DynEnt_AllocCollSector(DynEntityCollType collType, const float *mins, const float *maxs);
int __cdecl DynEnt_GetContents(const DynEntityDef *dynEntDef);
void __cdecl DynEnt_GetLocalBounds(const DynEntityDef *dynEntDef, float *mins, float *maxs);
void __cdecl DynEnt_GetWorldBounds(const DynEntityPose *dynEntPose, float *mins, float *maxs);
double __cdecl DynEnt_GetRadiusDistSqr(const DynEntityPose *dynEntPose, const float *origin);
double __cdecl DynEnt_GetCylindricalRadiusDistSqr(const DynEntityPose *dynEntPose, const float *origin);
bool __cdecl DynEnt_EntityInArea(
    const DynEntityDef *dynEntDef,
    const DynEntityPose *dynEntPose,
    const float *mins,
    const float *maxs,
    int contentMask);
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


extern int numPieces;
extern BreakablePiece g_breakablePieces[100];

