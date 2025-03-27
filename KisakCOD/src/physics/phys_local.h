#pragma once
#include <xanim/xanim.h>
#include <xanim/dobj.h>

#include "ode/joint.h"
#include "ode/collision_trimesh_internal.h"
#include <ode/collision_trimesh.h>

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

enum PhysicsGeomType : __int32
{                                       // ...
    PHYS_GEOM_NONE = 0x0,
    PHYS_GEOM_BOX = 0x1,
    PHYS_GEOM_BRUSHMODEL = 0x2,
    PHYS_GEOM_BRUSH = 0x3,
    PHYS_GEOM_CYLINDER = 0x4,
    PHYS_GEOM_CAPSULE = 0x5,
    PHYS_GEOM_COUNT = 0x6,
};

enum physStuckState_t : __int32
{                                       // ...
    PHYS_OBJ_STATE_POSSIBLY_STUCK = 0x0,
    PHYS_OBJ_STATE_STUCK = 0x1,
    PHYS_OBJ_STATE_FREE = 0x2,
};

struct GeomStateCylinder // sizeof=0xC
{                                       // ...
    int direction;
    float radius;
    float halfHeight;
};
struct GeomStateBox // sizeof=0xC
{                                       // ...
    float extent[3];
};
union GeomStateBrush_u // sizeof=0x4
{                                       // ...
    unsigned __int16 brushModel;
    const cbrush_t *brush;
};
struct GeomStateBrush // sizeof=0x1C
{                                       // ...
    GeomStateBrush_u u;
    float momentsOfInertia[3];
    float productsOfInertia[3];
};
union GeomState_u// sizeof=0x1C
{                                       // ...
    GeomStateCylinder cylinderState;
    GeomStateBox boxState;
    GeomStateBrush brushState;
};
struct GeomState // sizeof=0x48
{                                       // ...
    PhysicsGeomType type;               // ...
    bool isOriented;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float orientation[3][3];            // ...
    GeomState_u u;      // ...
};

struct dContactGeom // sizeof=0x2C
{                                       // ...
    float pos[4];
    float normal[4];
    float depth;
    dxGeom *g1;
    dxGeom *g2;
};

struct dContactGeomExt // sizeof=0x30
{                                       // ...
    dContactGeom contact;
    int surfFlags;
};

struct ContactList // sizeof=0x1804
{                                       // ...
    dContactGeomExt contacts[128];
    int contactCount;                   // ...
};

struct dxJointLimitMotor // sizeof=0x2C
{                                       // ...
    float vel;
    float fmax;
    float lostop;
    float histop;
    float fudge_factor;
    float normal_cfm;
    float stop_erp;
    float stop_cfm;
    float bounce;
    int limit;
    float limit_err;
};
struct dxJointHinge : dxJoint // sizeof=0xBC
{                                       // ...
    float anchor1[4];
    float anchor2[4];
    float axis1[4];
    float axis2[4];
    float qrel[4];
    dxJointLimitMotor limot;
};
struct dxJointAMotor : dxJoint // sizeof=0x128
{                                       // ...
    int num;
    int mode;
    int rel[3];
    float axis[3][4];
    dxJointLimitMotor limot[3];
    float reference1[4];
    float reference2[4];
};
struct dxJointSlider : dxJoint // sizeof=0x9C
{
    float axis1[4];
    float qrel[4];
    float offset[4];
    dxJointLimitMotor limot;
};
struct dxJointFixed : dxJoint // sizeof=0x60
{
    float qrel[4];
    float offset[4];
};
struct dxJointNull : dxJoint // sizeof=0x40
{
};
struct dxJointUniversal : dxJoint // sizeof=0xF8
{
    float anchor1[4];
    float anchor2[4];
    float axis1[4];
    float axis2[4];
    float qrel1[4];
    float qrel2[4];
    dxJointLimitMotor limot1;
    dxJointLimitMotor limot2;
};
struct dxJointBall : dxJoint // sizeof=0x60
{                                       // ...
    float anchor1[4];
    float anchor2[4];
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
struct __declspec(align(4)) PhysObjUserData // sizeof=0x70
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

struct dSurfaceParameters // sizeof=0x2C
{                                       // ...
    int mode;                           // ...
    float mu;                           // ...
    float mu2;                          // ...
    float bounce;                       // ...
    float bounce_vel;                   // ...
    float soft_erp;                     // ...
    float soft_cfm;                     // ...
    float motion1;
    float motion2;
    float slip1;
    float slip2;
};

 struct pooldata_t // sizeof=0x8
 {                                       // ...
     void *firstFree;
     int activeCount;
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
     T entries[N]
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





// phys_ode
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
void __cdecl Phys_RewindCurrentTime(PhysWorld worldIndex, dxSpace *timeNow);
void __cdecl Phys_GetPerformance(float *average, int *mintime, int *maxtime);
void __cdecl Phys_PerformanceEndFrame();
void __cdecl Phys_RunToTime(int localClientNum, PhysWorld worldIndex, dxSpace *timeNow);
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
