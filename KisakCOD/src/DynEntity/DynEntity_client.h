#pragma once
#include <xanim/xanim.h>
#include <xanim/dobj.h>
#include <utility>

enum DynEntityCollType : __int32
{                                       // ...
    DYNENT_COLL_CLIENT_FIRST = 0x0,
    DYNENT_COLL_CLIENT_MODEL = 0x0,
    DYNENT_COLL_CLIENT_BRUSH = 0x1,
    DYNENT_COLL_COUNT = 0x2,
};

enum DynEntityType : __int32
{                                       // ...
    DYNENT_TYPE_INVALID = 0x0,
    DYNENT_TYPE_CLUTTER = 0x1,
    DYNENT_TYPE_DESTRUCT = 0x2,
    DYNENT_TYPE_COUNT = 0x3,
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

struct __declspec(align(4)) DynEntSortStruct // sizeof=0x8
{
    float distSq;
    unsigned __int16 id;
    // padding byte
    // padding byte
};
//std::pair<DynEntSortStruct *first, DynEntSortStruct *second>; // sizeof=0x8

struct __declspec(align(2)) BreakablePiece // sizeof=0xC
{                                       // ...
    const XModel *model;                // ...
    int physObjId;                      // ...
    unsigned __int16 lightingHandle;    // ...
    bool active;                        // ...
    // padding byte
};

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
void __cdecl Vec3AddScalar(const float *a, float s, float *sum);
unsigned int __cdecl DynEntCl_GetClosestEntities(
    DynEntityDrawType drawType,
    float *radiusMins,
    float *radiusMaxs,
    float *origin,
    unsigned __int16 *hitEnts,
    bool isCylinder);
bool __cdecl DynEntCl_CompareDynEntsForExplosion(const DynEntSortStruct *ent1, const DynEntSortStruct *ent2);
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

extern int numPieces;
extern BreakablePiece g_breakablePieces[100];

