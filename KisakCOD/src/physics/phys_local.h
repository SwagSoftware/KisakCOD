#pragma once
#include <xanim/xanim.h>
#include <xanim/xmodel.h>
#include <xanim/dobj.h>

#include "ode/joint.h"
#include "ode/collision_trimesh_internal.h"
#include <ode/collision_trimesh.h>

#include <universal/memfile.h>
#include <universal/pool_allocator.h>


enum BodyState : __int32
{                                       // ...
    BS_DEAD = 0x0,
    BS_DOBJ_WAIT = 0x1,
    BS_VELOCITY_CAPTURE = 0x2,
    BS_TUNNEL_TEST = 0x3,
    BS_RUNNING = 0x4,
    BS_IDLE = 0x5,
    RAGDOLL_NUM_STATES = 0x6,
};

enum physStuckState_t : __int32
{                                       // ...
    PHYS_OBJ_STATE_POSSIBLY_STUCK = 0x0,
    PHYS_OBJ_STATE_STUCK = 0x1,
    PHYS_OBJ_STATE_FREE = 0x2,
};

union objInfo_u // sizeof=0xC
{                                       // ...
    float sideExtents[3];
    const cmodel_t *brushModel;
    const cbrush_t *brush;
};
struct objInfo // sizeof=0xA8
{                                       // ...
    int clipMask;                       // ...
    int cylDirection;                   // ...
    TraceThreadInfo threadInfo;         // ...
    float bounds[2][3];                 // ...
    float radius;                       // ...
    PhysicsGeomType type;               // ...
    float pos[3];                       // ...
    float R[3][3];                      // ...
    float RTransposed[3][3];            // ...
    objInfo_u u;        // ...
    float bodyCenter[3];                // ...
    bool isNarrow;                      // ...
    // padding byte
    // padding byte
    // padding byte
};
struct Results // sizeof=0x10
{                                       // ...
    struct dContactGeomExt *contacts;          // ...
    int contactCount;                   // ...
    int maxContacts;                    // ...
    int stride;                         // ...
};
struct InputOutput // sizeof=0x8
{                                       // ...
    const objInfo *Input;               // ...
    Results *Output;                    // ...
};

struct BrushWrapper // sizeof=0x50
{
    float mins[3];
    int contents;
    float maxs[3];
    unsigned int numsides;
    cbrushside_t *sides;
    __int16 axialMaterialNum[2][3];
    unsigned __int8 *baseAdjacentSide;
    __int16 firstAdjacentSideOffsets[2][3];
    unsigned __int8 edgeCount[2][3];
    // padding byte
    // padding byte
    int totalEdgeCount;
    cplane_s *planes;
};

struct PhysMass // sizeof=0x24
{                                       // ...
    float centerOfMass[3];
    float momentsOfInertia[3];
    float productsOfInertia[3];
};

struct PhysGeomInfo // sizeof=0x44
{
    BrushWrapper *brush;
    int type;
    float orientation[3][3];
    float offset[3];
    float halfLengths[3];
};

struct PhysGeomList // sizeof=0x2C
{
    unsigned int count;
    PhysGeomInfo *geoms;
    PhysMass mass;
};

struct PhysContact // sizeof=0x24
{                                       // ...
    float pos[3];
    float normal[3];                    // ...
    float depth;                        // ...
    float friction;                     // ...
    float bounce;                       // ...
};

struct Jitter // sizeof=0x24
{                                       // ...
    float origin[3];
    float innerRadiusSq;
    float outerRadiusSq;
    float innerRadius;
    float outerRadius;
    float minDisplacement;
    float maxDisplacement;
};
struct PhysWorldData // sizeof=0xCC
{                                       // ...
    int timeLastSnapshot;               // ...
    int timeLastUpdate;                 // ...
    float timeNowLerpFrac;              // ...
    void(__cdecl *collisionCallback)(); // ...
    int numJitterRegions;               // ...
    bool useContactCentroids;           // ...
    // padding byte
    // padding byte
    // padding byte
    Jitter jitterRegions[5];
};
struct PhysObjUserData // sizeof=0x70
{                                       // ...
    float translation[3];
    dxBody *body;
    float savedPos[3];
    float savedRot[3][3];
    int sndClass;
    float friction;
    float bounce;
    physStuckState_t state;
    float contactCentroid[3];
    int timeLastAsleep;
    float awakeTooLongLastPos[3];
    bool hasDisplayedAwakeTooLongWarning;
    bool debugContacts;
    // padding byte
    // padding byte
};
 struct PhysTriMeshInfo // sizeof=0x14
 {                                       // ...
     float *verts;                       // ...
     int vertStride;
     int vertCount;
     unsigned int *indices;
     unsigned int indexCount;
 };


 template<typename T, int N>
 struct PhysStaticArray
 {
     int freeEntry;
     T entries[N];

     void init()
     {
         memset(this->entries, 0, sizeof(this->entries));
         this->freeEntry = -1;
         for (int i = N-1; i >= 0; --i)
         {
             release(&this->entries[i]);
         }
     }

     T *allocate()
     {
         T *ptr;

         if (this->freeEntry == -1)
             return NULL;
         if (this->freeEntry >= N)
         {
             MyAssertHandler(
                 "c:\\trees\\cod3\\src\\physics\\phys_alloc.h",
                 49,
                 0,
                 "freeEntry doesn't index numEntries\n\t%i not in [0, %i)",
                 this->freeEntry,
                 N);
         }

         ptr = &this->entries[this->freeEntry];
         this->freeEntry++;
         return ptr;
     }

     void release(T *ptr)
     {
         iassert(ptr >= &entries[0] && ptr < &entries[N]);
         memset(ptr, 0, sizeof(T));
         //*(int *)ptr = this->freeEntry;
         this->freeEntry--;
     }


     bool isMember(void* ptr)
     {
         uintptr_t p = (uintptr_t)ptr;
         return (p >= (uintptr_t)&entries[0] && p < (uintptr_t)&entries[N]);
     }
 };


struct PhysGlob // sizeof=0x26508
{                                       // ...
    dxWorld *world[3];                  // ...
    PhysWorldData worldData[3];         // ...
    dxSpace *space[3];                  // ...
    dxJointGroup *contactgroup[3];      // ...
    PhysObjUserData userData[512];      // ...
    pooldata_t userDataPool;            // ...
    PhysTriMeshInfo triMeshInfo;        // ...
    dxTriMeshData *triMeshDataID;
    bool dumpContacts;                  // ...
    // padding byte
    // padding byte
    // padding byte
    dxGeom *visTrisGeom;                // ...
    dxGeom *worldGeom;                  // ...
    int debugActiveObjCount;            // ...
    PhysStaticArray<dxJointHinge, 192> hingeArray; // ...
    PhysStaticArray<dxJointBall, 160> ballArray; // ...
    PhysStaticArray<dxJointAMotor, 160> aMotorArray; // ...
    float gravityDirection[3];          // ...
    unsigned int physPreviousFrameTimes[10]; // ...
    unsigned int physPerformanceFrame;  // ...
    float performanceAverage;           // ...
    unsigned int performanceMintime;    // ...
    unsigned int performanceMaxtime;    // ...
};

struct dContactGeomExt // sizeof=0x30
{                                       // ...
    dContactGeom contact;
    int surfFlags;
};

union BrushInfo_u // sizeof=0x4
{                                       // ...
    unsigned __int16 brushModel;
    const cbrush_t *brush;
};
struct BrushInfo // sizeof=0x10
{
    BrushInfo_u u;
    float centerOfMass[3];
};

// phys_ode
struct ScreenPlacement;
struct ContactList // sizeof=0x1804
{                                       // ...
    dContactGeomExt contacts[128];
    int contactCount;                   // ...
};

void __cdecl TRACK_phys();
void __cdecl Phys_Init();
void __cdecl Phys_Go_f();
void __cdecl Phys_EnableGeom(dxBody *body);
void __cdecl Phys_Stop_f();
void __cdecl Phys_DisableBodyAndGeom(dxBody *body);
dxBody *__cdecl Phys_ObjCreateAxis(
    PhysWorld worldIndex,
    float *position,
    const float (*axis)[3],
    float *velocity,
    const PhysPreset *physPreset);
dxBody *__cdecl Phys_CreateBodyFromState(PhysWorld worldIndex, const BodyState *state);
void __cdecl Phys_BodyGetCenterOfMass(dxBody *body, float *outPosition);
void __cdecl Phys_BodyAddGeomAndSetMass(
    PhysWorld worldIndex,
    dxBody *body,
    float totalMass,
    const GeomState *geomState,
    const float *centerOfMass);
void __cdecl Phys_AdjustForNewCenterOfMass(dxBody *body, const float *newRelCenterOfMass);
void __cdecl Phys_BodyGetRotation(dxBody *body, float (*outRotation)[3]);
void __cdecl Phys_OdeMatrix3ToAxis(const float *inMatrix, float (*outAxis)[3]);
void __cdecl Phys_ObjGetPositionFromCenterOfMass(
    dxBody *body,
    const float (*rotation)[3],
    const float *centerOfGravity,
    float *objPos);
void __cdecl Phys_MassSetBrushTotal(dMass *m, float totalMass, float *momentsOfInertia, const float *productsOfInertia);
dxBody *__cdecl Phys_ObjCreate(
    PhysWorld worldIndex,
    float *position,
    float *quat,
    float *velocity,
    const PhysPreset *physPreset);
void __cdecl Phys_ObjSetOrientation(
    PhysWorld worldIndex,
    dxBody *id,
    const float *newPosition,
    const float *newOrientation);
void __cdecl Phys_ObjAddGeomBox(PhysWorld worldIndex, dxBody *id, const float *boxMin, const float *boxMax);
void __cdecl Phys_ObjAddGeomBoxRotated(
    PhysWorld worldIndex,
    dxBody *id,
    const float *center,
    const float *halfLengths,
    const float (*orientation)[3]);
void __cdecl Phys_ObjAddGeomBrushModel(
    PhysWorld worldIndex,
    dxBody *id,
    unsigned __int16 brushModel,
    const PhysMass *physMass);
void __cdecl Phys_ObjAddGeomBrush(PhysWorld worldIndex, dxBody *id, const cbrush_t *brush, const PhysMass *physMass);
void __cdecl Phys_ObjAddGeomCylinder(PhysWorld worldIndex, dxBody *id, const float *boxMin, const float *boxMax);
void __cdecl Phys_ObjAddGeomCylinderDirection(
    PhysWorld worldIndex,
    dxBody *id,
    int direction,
    float radius,
    float halfHeight,
    const float *centerOfMass);
void __cdecl Phys_ObjAddGeomCylinderRotated(
    PhysWorld worldIndex,
    dxBody *id,
    int direction,
    float radius,
    float halfHeight,
    const float *center,
    const float (*orientation)[3]);
void __cdecl Phys_ObjAddGeomCapsule(
    PhysWorld worldIndex,
    dxBody *id,
    int direction,
    float radius,
    float halfHeight,
    const float *centerOfMass);
void __cdecl Phys_ObjSetCollisionFromXModel(const XModel *model, PhysWorld worldIndex, dxBody *physId);
void __cdecl Phys_ObjSetAngularVelocity(dxBody *id, float *angularVel);
void __cdecl Phys_ObjSetAngularVelocityRaw(dxBody *id, float *angularVel);
void __cdecl Phys_ObjSetVelocity(dxBody *id, float *velocity);
void __cdecl Phys_ObjGetPosition(dxBody *id, float *outPosition, float (*outRotation)[3]);
void __cdecl Phys_ObjGetCenterOfMass(dxBody *id, float *outPosition);
void __cdecl Phys_ObjDestroy(PhysWorld worldIndex, dxBody *id);
void __cdecl Phys_ObjAddForce(PhysWorld worldIndex, dxBody *id, float *worldPos, const float *impulse);
int __cdecl Phys_IndexFromODEWorld(dxWorld *world);
void __cdecl Phys_ObjBulletImpact(
    PhysWorld worldIndex,
    dxBody *id,
    const float *worldPosRaw,
    const float *bulletDirRaw,
    float bulletSpeed,
    float scale);
void __cdecl Phys_TweakBulletImpact(float *worldPos, float *bulletDir, const float *centerOfMass);
void __cdecl Phys_PlayCollisionSound(int localClientNum, dxBody *body, unsigned int sndClass, ContactList *contactList);
void __cdecl Phys_BodyGetPointVelocity(dxBody *body, float *point, float *outVelocity);
void __cdecl Phys_DrawDebugText(const ScreenPlacement *scrPlace);
int __cdecl Phys_DrawDebugTextForWorld(
    unsigned int worldIndex,
    char *worldText,
    float *x,
    float *y,
    float charHeight,
    const ScreenPlacement *scrPlace);
void __cdecl Phys_ObjCountIfActive(dxBody *body);
void __cdecl dxPostProcessIslands(PhysWorld worldIndex);
void __cdecl Phys_CheckIfAliveTooLong(dxBody *body);
int __cdecl Phys_DoBodyOncePerFrame(unsigned int worldIndex, dxBody *body, float deltaT);
void __cdecl Phys_GeomUserGetAAContainedBox(dxGeom *geom, float *mins, float *maxs);
int __cdecl Phys_ObjGetSnapshot(PhysWorld worldIndex, dxBody *id, float *outPos, float (*outMat)[3]);
void __cdecl Phys_RewindCurrentTime(PhysWorld worldIndex, int timeNow);
void __cdecl Phys_GetPerformance(float *average, int *mintime, int *maxtime);
void __cdecl Phys_PerformanceEndFrame();
void __cdecl Phys_RunToTime(int localClientNum, PhysWorld worldIndex, int timeNow);
void __cdecl Phys_ObjDraw(dxBody *body);
void __cdecl Phys_RunFrame(int localClientNum, PhysWorld worldIndex, float seconds);
void __cdecl Phys_BodyGrabSnapshot(dxBody *body);
void __cdecl Phys_DoBodyOncePerRun(dxBody *body);
void __cdecl Phys_ObjTraceNewPos(dxBody *body);
void __cdecl Phys_PerformanceAddTime(int time);
void __cdecl Phys_ObjGetInterpolatedState(PhysWorld worldIndex, dxBody *id, float *outPos, float *outQuat);
void __cdecl Phys_ObjSetInertialTensor(dxBody *id, const PhysMass *physMass);
bool __cdecl Phys_ObjIsAsleep(dxBody *id);
void __cdecl Phys_Shutdown();
void __cdecl Phys_ObjSave(dxBody *id, MemoryFile *memFile);
void __cdecl Phys_GetStateFromBody(dxBody *body, BodyState *state);
dxBody *__cdecl Phys_ObjLoad(PhysWorld worldIndex, MemoryFile *memFile);
void __cdecl Phys_InitJoints();
void __cdecl Phys_SetHingeParams(
    PhysWorld worldIndex,
    dxJointHinge *id,
    float motorSpeed,
    float motorMaxForce,
    float lowStop,
    float highStop);
dxJointHinge *__cdecl Phys_CreateHinge(
    PhysWorld worldIndex,
    dxBody *obj1,
    dxBody *obj2,
    float *anchor,
    float *axis,
    float motorSpeed,
    float motorMaxForce,
    float lowStop,
    float highStop);
dxJointBall *__cdecl Phys_CreateBallAndSocket(PhysWorld worldIndex, dxBody *obj1, dxBody *obj2, float *anchor);
void __cdecl Phys_SetAngularMotorParams(
    PhysWorld worldIndex,
    dxJointAMotor *id,
    const float *motorSpeeds,
    const float *motorFMaxs,
    const float *lowStops,
    const float *highStops);
dxJointAMotor *__cdecl Phys_CreateAngularMotor(
    PhysWorld worldIndex,
    dxBody *obj1,
    dxBody *obj2,
    unsigned int numAxes,
    const float (*axes)[3],
    const float *motorSpeeds,
    const float *motorFMaxs,
    const float *lowStops,
    const float *highStops);
void __cdecl Phys_JointDestroy(PhysWorld worldIndex, dxJointHinge *id);
void __cdecl Phys_SetCollisionCallback(PhysWorld worldIndex, void(__cdecl *callback)());
void __cdecl Phys_AddJitterRegion(
    PhysWorld worldIndex,
    const float *origin,
    float innerRadius,
    float outerRadius,
    float minDisplacement,
    float maxDisplacement);
void __cdecl Phys_ObjSetContactCentroid(dxBody *id, const float *worldPos);

// phys_world_collision
int __cdecl Phys_GetSurfaceFlagsFromBrush(const cbrush_t *brush, unsigned int brushSideIndex);
void __cdecl CM_ForEachBrushInLeafBrushNode_r(
    cLeafBrushNode_s *node,
    const float *mins,
    const float *maxs,
    bool testMask,
    int clipMask,
    void(__cdecl *f)(const cbrush_t *, void *),
    void *userData);
void __cdecl CM_MeshTestGeomInLeaf(cLeaf_t *leaf, const objInfo *input, Results *results);
void __cdecl CM_PositionGeomTestInAabbTree_r(CollisionAabbTree *aabbTree, const objInfo *input, Results *results);
bool __cdecl CM_CullBox2(const objInfo *input, const float *origin, const float *halfSize);
int __cdecl dCollideWorldGeom(dxGeom *o1, dxGeom *o2, unsigned __int16 flags, dContactGeomExt *contact, int skip);
void __cdecl CM_TestGeomInLeaf(cLeaf_t *leaf, const objInfo *input, Results *results);
void __cdecl CM_TestGeomInLeafBrushNode(cLeaf_t *leaf, const objInfo *input, Results *results);
void __cdecl Phys_TestGeomInBrush(const cbrush_t *brush, unsigned int *userData);
void __cdecl Phys_TestAgainstEntities(const objInfo *input, Results *results);
void __cdecl Phys_InitWorldCollision();
int(__cdecl *__cdecl dGetColliderWorld(int classnum))(dxGeom *o1, dxGeom *o2, unsigned __int16 flags, dContactGeomExt *contact, int skip);
void __cdecl Phys_InitBrushmodelGeomClass();
void __cdecl Phys_GetBrushmodelAABB(dxGeom *geom, float *aabb);
void __cdecl Phys_InitBrushGeomClass();
void __cdecl Phys_GetBrushAABB(dxGeom *geom, float *aabb);
void __cdecl Phys_InitCylinderGeomClass();
void __cdecl Phys_GetCylinderAABB(dxGeom *geom, float *aabb);
void __cdecl Phys_InitCapsuleGeomClass();
void __cdecl Phys_GetCapsuleAABB(dxGeom *geom, float *aabb);
dxGeom *__cdecl Phys_CreateBrushmodelGeom(
    dxSpace *space,
    dxBody *body,
    unsigned __int16 brushModel,
    const float *centerOfMass);
dxGeom *__cdecl Phys_CreateBrushGeom(dxSpace *space, dxBody *body, const cbrush_t *brush, const float *centerOfMass);
dxGeom *__cdecl Phys_CreateCylinderGeom(dxSpace *space, dxBody *body, const GeomStateCylinder *cyl);
dxGeom *__cdecl Phys_CreateCapsuleGeom(dxSpace *space, dxBody *body, const GeomStateCylinder *cyl);



// phys_coll_boxbrush
enum PolyOrientation : __int32
{                                       // ...
    POLY_COUNTERCLOCKWISE = 0x0,
    POLY_CLOCKWISE = 0x1,
    POLY_ERROR = 0x2,
};
struct Poly // sizeof=0x8
{                                       // ...
    float (*pts)[3];                    // ...
    unsigned int ptCount;               // ...
};
struct BrushTrimeshData // sizeof=0x18
{                                       // ...
    const unsigned __int16 *indices;    // ...
    const float (*verts)[3];            // ...
    int triCount;                       // ...
    const objInfo *input;               // ...
    int surfaceFlags;                   // ...
    Results *results;                   // ...
};
struct BrushBrushData // sizeof=0xC
{                                       // ...
    const cbrush_t *fixedBrush;         // ...
    const objInfo *input;               // ...
    Results *results;                   // ...
};
void __cdecl Phys_DrawPoly(const Poly *poly, const float *color);
dContactGeomExt *__cdecl AddContact(Results *results);
bool __cdecl Phys_AddContactData(Results *results, float depth, float *normal, float *pos, int surfaceFlags);
PolyOrientation __cdecl GetPolyOrientation(const float *polyNormal, const float (*poly)[3], unsigned int ptCount);
char __cdecl Phys_GetChoppingPlaneForPolyEdge(
    const float *polyNormal,
    const float *pt1,
    const float *pt2,
    bool clockwise,
    float *outPlane);
unsigned int __cdecl Phys_ClipLineSegmentAgainstPlane(float *pt1, float *pt2, const float *choppingPlane);
unsigned int __cdecl Phys_ClipLineSegmentAgainstPoly(
    const float *polyNormal,
    const float (*poly)[3],
    unsigned int polyCount,
    float *pt1,
    float *pt2);
void __cdecl Phys_ProjectFaceOntoFaceAndClip(
    const float *referencePlane,
    const Poly *referencePoly,
    const Poly *poly2,
    int surfaceFlags,
    Results *results,
    float *collisionNormal);
unsigned int __cdecl ClipPolys(
    const float *polyNormal,
    const float (*poly1)[3],
    unsigned int poly1Count,
    float (*poly2)[3],
    unsigned int poly2Count,
    float (*result)[3],
    unsigned int maxCount);
void __cdecl Phys_GetWindingForBrushFace2(
    const cbrush_t *brush,
    unsigned int brushSide,
    Poly *outWinding,
    int maxVerts,
    const float (*axialPlanes)[4]);
void __cdecl Phys_CollideBoxWithBrush(const cbrush_t *brush, const objInfo *info, Results *results);
void __cdecl Phys_ProjectBoxFaceOntoBrushFaceAndClip(
    const objInfo *info,
    int boxAxis,
    int boxSign,
    const float *brushPlane,
    const Poly *winding,
    int surfaceFlags,
    Results *results,
    float *collisionNormal);
int __cdecl GetClosestBrushFace(
    const float *normal,
    const cbrush_t *brush,
    const Poly *brushWindings,
    float *outBrushPlane);
char __cdecl Phys_TestBoxAgainstEachBrushPlane(
    const cbrush_t *brush,
    const objInfo *info,
    float *outBrushPlane,
    int *outSideIndex,
    float *outMaxSeparation);
void __cdecl Phys_CollideBoxWithBrushFace(
    const cbrush_t *brush,
    unsigned int brushSideIndex,
    const float *bestBrushPlane,
    const Poly *brushWinding,
    const objInfo *info,
    Results *results,
    float *collisionNormal);
void __cdecl GetClosestBoxFace(const objInfo *info, const float *normal, int *minAxis, int *minSign);
char __cdecl Phys_DoesPolyIntersectBox(const Poly *poly, const objInfo *info);
void __cdecl Phys_CollideOrientedBrushWithBrush(
    const cbrush_t *orientedBrush,
    const cbrush_t *fixedBrush,
    const objInfo *input,
    Results *results);
unsigned int __cdecl Phys_BuildWindingsForBrush(
    const cbrush_t *brush,
    const float (*planes)[4],
    Poly *outPolys,
    unsigned int maxPolys,
    float (*outVerts)[3],
    unsigned int maxVerts);
void __cdecl Phys_GetWindingForBrushFace(
    const cbrush_t *brush,
    const float (*inPlanes)[4],
    unsigned int brushSide,
    Poly *outWinding,
    int maxVerts);
unsigned int __cdecl Phys_BuildWindingsForBrush2(
    const cbrush_t *brush,
    Poly *outPolys,
    unsigned int maxPolys,
    float (*outVerts)[3],
    unsigned int maxVerts);
double __cdecl Phys_TestVertsAgainstPlane_Wrapper(const float *plane, const Poly *verts);
double __cdecl Phys_TestVertsAgainstPlane(const float (*verts)[3], unsigned int vertCount, const float *plane);
char __cdecl Phys_TestVertsAgainstPlanes(
    const float (*verts)[3],
    unsigned int vertCount,
    const cbrush_t *brushContainingThePlanes,
    const float (*planes)[4],
    float *outPlane,
    int *outSideIndex,
    float *outMaxSeparation);
void __cdecl Phys_TransformPlane(
    const float *normal,
    float dist,
    const float *translate,
    const float (*rotate)[3],
    float *outPlane);
void __cdecl Phys_CollideOrientedBrushAgainstFixedBrushFace(
    const cbrush_t *fixedBrush,
    unsigned int fixedBrushSideIndex,
    float *bestFixedBrushPlane,
    const Poly *fixedBrushPolys,
    const cbrush_t *orientedBrush,
    const Poly *orientedBrushPolys,
    const float (*transformedPlanes)[4],
    Results *results);
int __cdecl GetClosestOrientedBrushFace(
    const float *normal,
    const cbrush_t *brush,
    const Poly *brushPolys,
    const float (*orientedPlanes)[4],
    float *outBrushPlane);
int __cdecl Phys_CollideBrushAgainstBrushFace(
    const cbrush_t *brush,
    const Poly *brushPolys,
    const cbrush_t *referenceBrush,
    unsigned int referenceBrushSideIndex,
    float *referenceBrushPlane,
    const Poly *referenceBrushPolys,
    Results *results);
char __cdecl Phys_DoesPolyIntersectOrientedBrush(
    const Poly *poly,
    const float (*transformedPlanes)[4],
    unsigned int brushSides);
void __cdecl Phys_CollideOrientedBrushModelWithBrush(const cbrush_t *fixedBrush, const objInfo *info, Results *results);
void __cdecl Phys_CollideOrientedBrushWithBrush_Wrapper(const cbrush_t *orientedBrush, void *userData);
void __cdecl Phys_CollideOrientedBrushWithTriangleList(
    const cbrush_t *orientedBrush,
    const unsigned __int16 *indices,
    const float (*verts)[3],
    int triCount,
    const objInfo *input,
    int surfaceFlags,
    Results *results);
int __cdecl Phys_GetPlaneForTriangle2(const float (*triangle)[3], const float *origin, float radius, float *result);
void __cdecl Phys_CollideOrientedBrushWithTriangle(
    const cbrush_t *orientedBrush,
    const float *tri0,
    const float *tri1,
    const float *tri2,
    BrushTrimeshData *data);
void __cdecl Phys_CollideFixedBrushWithTriangle(const cbrush_t *brush, float (*triangle)[3], BrushTrimeshData *data);
void __cdecl Phys_GetPlaneForTriangle(const float (*triangle)[3], float *result);
unsigned int __cdecl Phys_AxialSideToJ(unsigned int axialSide);
void __cdecl Phys_DrawPolyTransformed(const Poly *poly, const float *color, const float *pos, const float (*R)[3]);
double __cdecl Phys_TestTriangleAgainstBrushPlane(const float *brushPlane, const float (*triangle)[3]);
void __cdecl Phys_CollideOrientedBrushModelWithTriangleList(
    const unsigned __int16 *indices,
    const float (*verts)[3],
    int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
void __cdecl Phys_CollideOrientedBrushWithTriangleList_Wrapper(const cbrush_t *orientedBrush, void *userData);
void __cdecl Phys_CollideBoxWithTriangleList(
    const unsigned __int16 *indices,
    const float (*verts)[3],
    unsigned int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results);
void __cdecl Phys_AxisToOdeMatrix3(const float (*inAxis)[3], float *outMatrix);

int __cdecl Phys_ClipPolyAgainstPlane(
    float (*poly)[3],
    unsigned int polyCount,
    unsigned int maxCount,
    float *choppingPlane);


// physpreset_load_obj
PhysPreset *__cdecl PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int));

// phys_contacts
void __cdecl Phys_CheckOpposingNormals(dxBody *body0, dxBody *body1, ContactList *contacts);
void __cdecl Phys_ReduceContacts(dxBody *body, const ContactList *in, ContactList *out);
void __cdecl Phys_AssignInitialGroups(const ContactList *contacts, int *group);
void __cdecl Phys_KMeans(const ContactList *contacts, float (*centroid)[3], int *group);
void __cdecl Phys_DumpGroups(const float (*centroid)[3]);
void __cdecl Phys_DumpContacts(const ContactList *contacts, const int *group);
void __cdecl Phys_MergeGroups(const ContactList *contacts, float (*centroid)[3], int *group);
void __cdecl Phys_GenerateGroupContacts(
    dxBody *body,
    const ContactList *inContacts,
    float (*centroid)[3],
    int *group,
    ContactList *outContacts);
void __cdecl Phys_CreateBasisFromNormal(const float *normal, float *binormal, float *tangent);
void __cdecl Phys_DebugDrawContactPoint(const float *pos, const float *normal, float depth, const float *color);
void __cdecl Phys_DumpContact(int contactNum, const dContactGeom *contact);
void __cdecl Phys_CreateJointForEachContact(
    ContactList *contactList,
    dxBody *body1,
    dxBody *body2,
    const dSurfaceParameters *surfParms,
    PhysWorld worldIndex);
void __cdecl Phys_ApplyContactJitter(PhysWorld worldIndex, dContactGeom *contact, dxBody *body1, dxBody *body2);
void __cdecl Phys_RemoveOpposingNormalContacts(const float *com, ContactList *contacts);
void __cdecl Phys_AddCollisionContact(PhysWorld worldId, const PhysContact *physContact, dxBody *obj0, dxBody *obj1);



extern const dvar_t *phys_contact_erp;
extern const dvar_t *phys_autoDisableAngular;
extern const dvar_t *phys_drawcontacts;
extern const dvar_t *phys_bulletUpBias;
extern const dvar_t *phys_joint_stop_erp;
extern const dvar_t *phys_noIslands;
extern const dvar_t *phys_dragLinear;
extern const dvar_t *phys_joint_cfm;
extern const dvar_t *phys_gravityChangeWakeupRadius;
extern const dvar_t *phys_drawAwake;
extern const dvar_t *phys_contact_cfm;
extern const dvar_t *phys_narrowObjMaxLength;
extern const dvar_t *phys_minImpactMomentum;
extern const dvar_t *phys_autoDisableTime;
extern const dvar_t *phys_bulletSpinScale;
extern const dvar_t *phys_drawAwakeTooLong;
extern const dvar_t *phys_cfm;
extern const dvar_t *phys_drawCollisionWorld;
extern const dvar_t *phys_visibleTris;
extern const dvar_t *phys_gravity;
extern const dvar_t *phys_collUseEntities;
extern const dvar_t *phys_autoDisableLinear;
extern const dvar_t *phys_contact_cfm_ragdoll;
extern const dvar_t *phys_drawCollisionObj;
extern const dvar_t *phys_joint_stop_cfm;
extern const dvar_t *phys_erp;
extern const dvar_t *phys_dragAngular;
extern const dvar_t *phys_frictionScale;
extern const dvar_t *phys_dumpcontacts;
extern const dvar_t *phys_mcv_ragdoll;
extern const dvar_t *phys_csl;
extern const dvar_t *phys_contact_erp_ragdoll;
extern const dvar_t *phys_reorderConst;
extern const dvar_t *phys_interBodyCollision;
extern const dvar_t *phys_drawDebugInfo;
extern const dvar_t *phys_qsi;
extern const dvar_t *phys_jitterMaxMass;
extern const dvar_t *phys_mcv;
extern const dvar_t *dynEntPieces_velocity;
extern const dvar_t *dynEntPieces_angularVelocity;
extern const dvar_t *dynEntPieces_impactForce;

extern PhysGlob physGlob;

extern int g_phys_msecStep[3];
extern int g_phys_minMsecStep[3];
extern int g_phys_maxMsecStep[3];