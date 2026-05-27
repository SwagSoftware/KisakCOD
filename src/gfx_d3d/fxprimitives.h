#pragma once

#include <universal/q_shared.h>
#include <universal/com_math.h>

constexpr size_t MAX_EFFECTS = 1024;
constexpr size_t MAX_ELEMS = 2048;
constexpr size_t MAX_TRAILS = 128;
constexpr size_t MAX_TRAIL_ELEMS = 2048;
           
struct Material;

struct r_double_index_t // sizeof=0x4
{                             
    r_double_index_t()
    {
        kisak = 0;
    }
    r_double_index_t(int val)
    {
        kisak = val;
    }
    union
    {
        uint16_t value[2];          // ...
        uint32_t kisak;
    };
};

struct orientation_t // sizeof=0x30
{                                       // ...
    float origin[3];                    // ...
    float axis[3][3];                   // ...
};

struct GfxMarkContext // sizeof=0x6
{                                       // ...
    uint8_t lmapIndex;          // ...
    uint8_t primaryLightIndex;  // ...
    uint8_t reflectionProbeIndex; // ...
    uint8_t modelTypeAndSurf;   // ...
    uint16_t modelIndex;        // ...
};

struct FxElemDef;

/////////////////////////////////////////////////////////////////////////////////
struct FxBoltAndSortOrder // sizeof=0x4
{                                       // ...
    unsigned __int32 dobjHandle : 12;
    unsigned __int32 temporalBits : 1;
    unsigned __int32 boneIndex : 11;
    unsigned __int32 sortOrder : 8;
};

struct FxSpatialFrame // sizeof=0x1C
{                                       // ...
    float quat[4];
    float origin[3];                    // ...
};

struct FxEffectDef // sizeof=0x20
{                                       // ...
    const char *name;
    int flags;
    int totalSize;
    int msecLoopingLife;
    int elemDefCountLooping;
    int elemDefCountOneShot;
    int elemDefCountEmission;
    const FxElemDef *elemDefs;
};
static_assert(sizeof(FxEffectDef) == 32);

struct FxEffect // sizeof=0x80
{                                       // ...
    const FxEffectDef *def;
    volatile long status;
    uint16_t firstElemHandle[3];
    uint16_t firstSortedElemHandle;
    uint16_t firstTrailHandle;
    uint16_t randomSeed;
    uint16_t owner;
    uint16_t packedLighting;
    FxBoltAndSortOrder boltAndSortOrder;
    volatile long frameCount;
    int msecBegin;
    int msecLastUpdate;
    FxSpatialFrame frameAtSpawn;
    FxSpatialFrame frameNow;
    FxSpatialFrame framePrev;
    float distanceTraveled;
};

template<typename T>
struct FxPool
{
    union
    {
        int nextFree;
        T item;
    };
};
struct FxCamera // sizeof=0xB0
{                                       // ...
    float origin[3];
    volatile long isValid;
    float frustum[6][4];
    float axis[3][3];
    uint32_t frustumPlaneCount;
    float viewOffset[3];
    uint32_t pad[3];
};

struct FxSpriteInfo // sizeof=0x10
{                                       // ...
    r_double_index_t *indices;          // ...
    uint32_t indexCount;
    Material *material;
    const char *name;
};
union FxElem_u // sizeof=0x4
{                                       // ...
    float trailTexCoord;
    uint16_t lightingHandle;
};
struct FxElem // sizeof=0x28
{                                       // ...
    uint8_t defIndex;
    uint8_t sequence;
    uint8_t atRestFraction;
    uint8_t emitResidual;
    uint16_t nextElemHandleInEffect;
    uint16_t prevElemHandleInEffect;
    int msecBegin;
    float baseVel[3];
    //$A58BA6DA60295001BBA5E9F807131CF1 ___u8;
    union
    {
        int physObjId;
        float origin[3];
    };
    //FxElem::<unnamed_type_u> u;
    FxElem_u u;

    static constexpr size_t HANDLE_SCALE = 4;
};
struct FxTrail // sizeof=0x8
{                                       // ...
    uint16_t nextTrailHandle;   // ...
    uint16_t firstElemHandle;   // ...
    uint16_t lastElemHandle;    // ...
    char defIndex;                      // ...
    char sequence;                      // ...

    static constexpr size_t HANDLE_SCALE = 4;
};
struct FxTrailElem // sizeof=0x20
{                                       // ...
    float origin[3];
    float spawnDist;
    int msecBegin;
    uint16_t nextTrailElemHandle;
    __int16 baseVelZ;
    char basis[2][3];
    uint8_t sequence;
    uint8_t unused;

    static constexpr size_t HANDLE_SCALE = 4;
};
struct FxVisBlocker // sizeof=0x10
{                                       // ...
    float origin[3];
    uint16_t radius;
    uint16_t visibility;
};
struct FxVisState // sizeof=0x1010
{                                       // ...
    FxVisBlocker blocker[256];
    volatile long blockerCount;
    uint32_t pad[3];
};
struct FxSystem // sizeof=0xA60
{                                       // ...
    FxCamera camera;
    FxCamera cameraPrev;
    FxSpriteInfo sprite;
    FxEffect *effects;
    FxPool<FxElem> *elems;
    FxPool<FxTrail> *trails;
    FxPool<FxTrailElem> *trailElems;
    uint16_t *deferredElems;
    volatile long firstFreeElem;
    volatile long firstFreeTrailElem;
    volatile long firstFreeTrail;
    volatile long deferredElemCount;
    volatile long activeElemCount;
    volatile long activeTrailElemCount;
    volatile long activeTrailCount;
    volatile long gfxCloudCount;
    FxVisState *visState;
    const FxVisState *visStateBufferRead;
    FxVisState *visStateBufferWrite;
    volatile long firstActiveEffect;
    volatile long firstNewEffect;
    volatile long firstFreeEffect;
    uint16_t allEffectHandles[1024];
    volatile long activeSpotLightEffectCount;
    volatile long activeSpotLightElemCount;
    uint16_t activeSpotLightEffectHandle;
    uint16_t activeSpotLightElemHandle;
    __int16 activeSpotLightBoltDobj;
    // padding byte
    // padding byte
    volatile long iteratorCount;
    int msecNow;
    volatile long msecDraw;
    int frameCount;
    bool isInitialized;
    bool needsGarbageCollection;
    bool isArchiving;
    uint8_t localClientNum;
    uint32_t restartList[32];
};
struct FxMarkPoint // sizeof=0x20
{                                       // ...
    float xyz[3];
    float lmapCoord[2];
    float normal[3];
};
struct FxPointGroup // sizeof=0x44
{                                       // ...
    FxMarkPoint points[2];
    int next;
};
union FxPointGroupPool // sizeof=0x44
{                                       // ...
    FxPointGroupPool *nextFreePointGroup;
    FxPointGroup pointGroup;
};

struct FxTriGroup // sizeof=0x18
{                                       // ...
    uint16_t indices[2][3];
    GfxMarkContext context;
    uint8_t triCount;
    uint8_t unused[1];
    int next;
};

union FxTriGroupPool // sizeof=0x18
{                                       // ...
    FxTriGroupPool* nextFreeTriGroup;
    FxTriGroup triGroup;
};

struct FxMark // sizeof=0x44
{                                       // ...
    uint16_t prevMark;
    uint16_t nextMark;
    int frameCountDrawn;
    int frameCountAlloced;
    float origin[3];
    float radius;
    float texCoordAxis[3];
    uint8_t nativeColor[4];
    Material *material;
    GfxMarkContext context;
    uint8_t triCount;
    // padding byte
    uint16_t pointCount;
    // padding byte
    // padding byte
    int tris;
    int points;
};

struct FxMarksSystem
{                                       // ...
    int frameCount;
    uint16_t firstFreeMarkHandle;
    uint16_t firstActiveWorldMarkHandle;
    uint16_t entFirstMarkHandles[MAX_GENTITIES];
    FxTriGroupPool *firstFreeTriGroup;
    FxPointGroupPool *firstFreePointGroup;
    FxMark marks[512];
    FxTriGroupPool triGroups[2048];
    FxPointGroupPool pointGroups[3072]; // ...
    bool noMarks;
    bool hasCarryIndex;
    uint16_t carryIndex;
    uint32_t allocedMarkCount;
    uint32_t freedMarkCount;
};
struct FxUpdateElem // sizeof=0x7C
{                                       // ...
    FxEffect *effect;
    int elemIndex;
    int atRestFraction;                 // ...
    orientation_t orient;
    int randomSeed;
    int sequence;
    float msecLifeSpan;
    int msecElemBegin;
    int msecElemEnd;
    int msecUpdateBegin;
    int msecUpdateEnd;                  // ...
    float msecElapsed;
    float normTimeUpdateEnd;
    float *elemOrigin;
    float *elemBaseVel;                 // ...
    float posWorld[3];
    bool onGround;                      // ...
    // padding byte
    // padding byte
    // padding byte
    int physObjId;                      // ...
};
struct FxCmd // sizeof=0xC
{                                       // ...
    FxSystem *system;
    int localClientNum;
    volatile int *spawnLock;
};

struct FxFloatRange // sizeof=0x8
{                                       // ...
    float base;
    float amplitude;
};
struct FxSpawnDefLooping // sizeof=0x8
{                                       // ...
    int intervalMsec;
    int count;
};
struct FxIntRange // sizeof=0x8
{                                       // ...
    int base;
    int amplitude;
};
struct FxSpawnDefOneShot // sizeof=0x8
{                                       // ...
    FxIntRange count;
};
union FxSpawnDef // sizeof=0x8
{                                       // ...
    FxSpawnDefLooping looping;
    FxSpawnDefOneShot oneShot;
};
struct FxElemAtlas // sizeof=0x8
{                                       // ...
    uint8_t behavior;
    uint8_t index;
    uint8_t fps;
    uint8_t loopCount;
    uint8_t colIndexBits;
    uint8_t rowIndexBits;
    __int16 entryCount;
};
struct FxElemVec3Range // sizeof=0x18
{                                       // ...
    float base[3];
    float amplitude[3];
};
struct FxElemVisualState // sizeof=0x18
{                                       // ...
    uint8_t color[4];
    float rotationDelta;
    float rotationTotal;                // ...
    float size[2];                      // ...
    float scale;
};
const struct FxElemVisStateSample // sizeof=0x30
{
    FxElemVisualState base;
    FxElemVisualState amplitude;
};
struct FxElemPreVisualState // sizeof=0x1C
{                                       // ...
    float sampleLerp;                   // ...
    float sampleLerpInv;                // ...
    const FxElemDef *elemDef;
    const FxEffect *effect;
    const FxElemVisStateSample *refState; // ...
    int randomSeed;
    uint32_t distanceFade;
};
struct FxElemVelStateInFrame // sizeof=0x30
{                                       // ...
    FxElemVec3Range velocity;
    FxElemVec3Range totalDelta;
};
const struct FxElemVelStateSample // sizeof=0x60
{
    FxElemVelStateInFrame local;
    FxElemVelStateInFrame world;
};
union FxEffectDefRef // sizeof=0x4
{                                       // ...
    const FxEffectDef *handle;
    const char *name;
};
union FxElemVisuals // sizeof=0x4
{                                       // ...
    const void *anonymous;
    struct Material *material;
    struct XModel *model;
    FxEffectDefRef effectDef;
    const char *soundName;

    FxElemVisuals() = default;

    FxElemVisuals(Material *mat) : material(mat) { }
};
struct FxElemMarkVisuals // sizeof=0x8
{                                       // ...
    struct Material *materials[2];
};
union FxElemDefVisuals // sizeof=0x4
{                                       // ...
    FxElemMarkVisuals *markArray;
    FxElemVisuals *array;
    FxElemVisuals instance;
};
struct FxTrailVertex // sizeof=0x14
{                                       // ...
    float pos[2];
    float normal[2];
    float texCoord;
};
struct FxTrailDef // sizeof=0x1C
{
    int scrollTimeMsec;
    int repeatDist;
    int splitDist;
    int vertCount;
    FxTrailVertex *verts;
    int indCount;
    uint16_t *inds;
};
const struct FxElemDef // sizeof=0xFC
{
    int flags;
    FxSpawnDef spawn;
    FxFloatRange spawnRange;
    FxFloatRange fadeInRange;
    FxFloatRange fadeOutRange;
    float spawnFrustumCullRadius;
    FxIntRange spawnDelayMsec;
    FxIntRange lifeSpanMsec;
    FxFloatRange spawnOrigin[3];
    FxFloatRange spawnOffsetRadius;
    FxFloatRange spawnOffsetHeight;
    FxFloatRange spawnAngles[3];
    FxFloatRange angularVelocity[3];
    FxFloatRange initialRotation;
    FxFloatRange gravity;
    FxFloatRange reflectionFactor;
    FxElemAtlas atlas;
    uint8_t elemType;
    uint8_t visualCount;
    uint8_t velIntervalCount;
    uint8_t visStateIntervalCount;
    FxElemVelStateSample *velSamples;
    FxElemVisStateSample *visSamples;
    FxElemDefVisuals visuals;
    float collMins[3];
    float collMaxs[3];
    FxEffectDefRef effectOnImpact;
    FxEffectDefRef effectOnDeath;
    FxEffectDefRef effectEmitted;
    FxFloatRange emitDist;
    FxFloatRange emitDistVariance;
    FxTrailDef *trailDef;
    uint8_t sortOrder;
    uint8_t lightingFrac;
    uint8_t useItemClip;
    uint8_t unused[1];
};

struct FxImpactEntry // sizeof=0x84
{
    const FxEffectDef *nonflesh[29];
    const FxEffectDef *flesh[4];
};

struct FxImpactTable // sizeof=0x8
{                                       // ...
    const char *name;
    FxImpactEntry *table;
};
static_assert(sizeof(FxImpactTable) == 8);

struct FxSystemBuffers // sizeof=0x47480
{                                       // ...
    FxEffect effects[MAX_EFFECTS];
    FxPool<FxElem> elems[MAX_ELEMS];
    FxPool<FxTrail> trails[MAX_TRAILS];
    FxPool<FxTrailElem> trailElems[MAX_TRAIL_ELEMS];
    FxVisState visState[2];
    uint16_t deferredElems[2048];
    uint8_t padBuffer[96];
};

template<typename ITEM_TYPE, size_t LIMIT>
uint16 FX_PoolToHandle_Generic(FxPool<ITEM_TYPE>* poolArray, ITEM_TYPE* item)
{
    static_assert((LIMIT * ITEM_TYPE::HANDLE_SCALE) <= 0xFFFF, "do not support huge pools at the moment");

    vassert(item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item, "%p %p", poolArray, item);
    return ((char*)item - (char*)poolArray) / ITEM_TYPE::HANDLE_SCALE;
}

template<typename ITEM_TYPE, size_t LIMIT>
FxPool<ITEM_TYPE>* FX_PoolFromHandle_Generic(FxPool<ITEM_TYPE>* poolArray, uint handle)
{
    vassert(handle < (LIMIT * sizeof(ITEM_TYPE) / ITEM_TYPE::HANDLE_SCALE) && handle % (sizeof(ITEM_TYPE) / ITEM_TYPE::HANDLE_SCALE) == 0, "%p %u", poolArray, handle);
    return (FxPool<ITEM_TYPE> *)((char*)poolArray + (handle * ITEM_TYPE::HANDLE_SCALE));
}
