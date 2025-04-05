#pragma once

constexpr size_t MAX_EFFECTS = 1024;
constexpr size_t MAX_ELEMS = 2048;
constexpr size_t MAX_TRAILS = 128;
constexpr size_t MAX_TRAIL_ELEMS = 2048;
           
struct Material;

struct r_double_index_t // sizeof=0x4
{                                       // ...
    unsigned __int16 value[2];          // ...
};

struct orientation_t // sizeof=0x30
{                                       // ...
    float origin[3];                    // ...
    float axis[3][3];                   // ...
};

struct GfxMarkContext // sizeof=0x6
{                                       // ...
    unsigned __int8 lmapIndex;          // ...
    unsigned __int8 primaryLightIndex;  // ...
    unsigned __int8 reflectionProbeIndex; // ...
    unsigned __int8 modelTypeAndSurf;   // ...
    unsigned __int16 modelIndex;        // ...
};

struct FxElemDef;

/////////////////////////////////////////////////////////////////////////////////
struct FxBoltAndSortOrder // sizeof=0x4
{                                       // ...
    unsigned __int32 dobjHandle : 12;
    unsigned __int32 temporalBits : 2;
    unsigned __int32 boneIndex : 10;
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
struct FxEffect // sizeof=0x80
{                                       // ...
    const FxEffectDef *def;
    volatile long status;
    unsigned __int16 firstElemHandle[3];
    unsigned __int16 firstSortedElemHandle;
    unsigned __int16 firstTrailHandle;
    unsigned __int16 randomSeed;
    unsigned __int16 owner;
    unsigned __int16 packedLighting;
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
    unsigned int frustumPlaneCount;
    float viewOffset[3];
    unsigned int pad[3];
};

struct FxSpriteInfo // sizeof=0x10
{                                       // ...
    r_double_index_t *indices;          // ...
    unsigned int indexCount;
    Material *material;
    const char *name;
};
union FxElem_u // sizeof=0x4
{                                       // ...
    float trailTexCoord;
    unsigned __int16 lightingHandle;
};
struct FxElem // sizeof=0x28
{                                       // ...
    unsigned __int8 defIndex;
    unsigned __int8 sequence;
    unsigned __int8 atRestFraction;
    unsigned __int8 emitResidual;
    unsigned __int16 nextElemHandleInEffect;
    unsigned __int16 prevElemHandleInEffect;
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
    unsigned __int16 nextTrailHandle;   // ...
    unsigned __int16 firstElemHandle;   // ...
    unsigned __int16 lastElemHandle;    // ...
    char defIndex;                      // ...
    char sequence;                      // ...

    static constexpr size_t HANDLE_SCALE = 4;
};
struct FxTrailElem // sizeof=0x20
{                                       // ...
    float origin[3];
    float spawnDist;
    int msecBegin;
    unsigned __int16 nextTrailElemHandle;
    __int16 baseVelZ;
    char basis[2][3];
    unsigned __int8 sequence;
    unsigned __int8 unused;

    static constexpr size_t HANDLE_SCALE = 4;
};
struct FxVisBlocker // sizeof=0x10
{                                       // ...
    float origin[3];
    unsigned __int16 radius;
    unsigned __int16 visibility;
};
struct FxVisState // sizeof=0x1010
{                                       // ...
    FxVisBlocker blocker[256];
    volatile int blockerCount;
    unsigned int pad[3];
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
    unsigned __int16 *deferredElems;
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
    unsigned __int16 allEffectHandles[1024];
    volatile long activeSpotLightEffectCount;
    volatile long activeSpotLightElemCount;
    unsigned __int16 activeSpotLightEffectHandle;
    unsigned __int16 activeSpotLightElemHandle;
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
    unsigned __int8 localClientNum;
    unsigned int restartList[32];
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
    unsigned __int16 indices[2][3];
    GfxMarkContext context;
    unsigned __int8 triCount;
    unsigned __int8 unused[1];
    int next;
};

union FxTriGroupPool // sizeof=0x18
{                                       // ...
    FxTriGroupPool* nextFreeTriGroup;
    FxTriGroup triGroup;
};

struct FxMark // sizeof=0x44
{                                       // ...
    unsigned __int16 prevMark;
    unsigned __int16 nextMark;
    int frameCountDrawn;
    int frameCountAlloced;
    float origin[3];
    float radius;
    float texCoordAxis[3];
    unsigned __int8 nativeColor[4];
    Material *material;
    GfxMarkContext context;
    unsigned __int8 triCount;
    // padding byte
    unsigned __int16 pointCount;
    // padding byte
    // padding byte
    int tris;
    int points;
};
struct FxMarksSystem // sizeof=0x4801C
{                                       // ...
    int frameCount;
    unsigned __int16 firstFreeMarkHandle;
    unsigned __int16 firstActiveWorldMarkHandle;
    unsigned __int16 entFirstMarkHandles[1024];
    FxTriGroupPool *firstFreeTriGroup;
    FxPointGroupPool *firstFreePointGroup;
    FxMark marks[512];
    FxTriGroupPool triGroups[2048];
    FxPointGroupPool pointGroups[3072]; // ...
    bool noMarks;
    bool hasCarryIndex;
    unsigned __int16 carryIndex;
    unsigned int allocedMarkCount;
    unsigned int freedMarkCount;
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
    unsigned __int8 behavior;
    unsigned __int8 index;
    unsigned __int8 fps;
    unsigned __int8 loopCount;
    unsigned __int8 colIndexBits;
    unsigned __int8 rowIndexBits;
    __int16 entryCount;
};
struct FxElemVec3Range // sizeof=0x18
{                                       // ...
    float base[3];
    float amplitude[3];
};
struct FxElemVisualState // sizeof=0x18
{                                       // ...
    unsigned __int8 color[4];
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
    unsigned int distanceFade;
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
    unsigned __int16 *inds;
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
    unsigned __int8 elemType;
    unsigned __int8 visualCount;
    unsigned __int8 velIntervalCount;
    unsigned __int8 visStateIntervalCount;
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
    unsigned __int8 sortOrder;
    unsigned __int8 lightingFrac;
    unsigned __int8 useItemClip;
    unsigned __int8 unused[1];
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
struct FxSystemBuffers // sizeof=0x47480
{                                       // ...
    FxEffect effects[MAX_EFFECTS];
    FxPool<FxElem> elems[MAX_ELEMS];
    FxPool<FxTrail> trails[MAX_TRAILS];
    FxPool<FxTrailElem> trailElems[MAX_TRAIL_ELEMS];
    FxVisState visState[2];
    unsigned __int16 deferredElems[2048];
    unsigned __int8 padBuffer[96];
};

template<typename ITEM_TYPE, size_t LIMIT>
uint16 FX_PoolToHandle_Generic(FxPool<ITEM_TYPE>* poolArray, ITEM_TYPE* item)
{
    static_assert((LIMIT * ITEM_TYPE::HANDLE_SCALE) <= 0xFFFF, "do not support huge pools at the moment");

    if (!item || item < poolArray || item >= &poolArray[LIMIT])
    {
        vassert(item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item, "%p %p", ppolArray, item);
    }
    return ((char*)item - (char*)poolArray) / ITEM_TYPE::HANDLE_SCALE;
}

template<typename ITEM_TYPE, size_t LIMIT>
FxPool<ITEM_TYPE>* FX_PoolFromHandle_Generic(FxPool<ITEM_TYPE>* poolArray, uint handle)
{
    vassert(handle < (LIMIT * sizeof(ITEM_TYPE) / ITEM_TYPE::HANDLE_SCALE) && handle % (sizeof(ITEM_TYPE) / ITEM_TYPE::HANDLE_SCALE) == 0, "%p %u", poolArray, handle);
    return (FxPool<FxElem> *)((char*)poolArray + (handle * ITEM_TYPE::HANDLE_SCALE));
}