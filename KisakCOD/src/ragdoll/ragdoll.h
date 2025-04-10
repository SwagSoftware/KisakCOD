#pragma once
#include <physics/phys_local.h>
#include <ode/collision.h>
#include <bgame/bg_local.h>

enum JointType : __int32
{                                       // ...
    RAGDOLL_JOINT_NONE = 0x0,
    RAGDOLL_JOINT_HINGE = 0x1,
    RAGDOLL_JOINT_SWIVEL = 0x2,
};
const char *jointNames[3] = { "none", "hinge", "swivel" }; // idb

struct BoneDef // sizeof=0x4C
{                                       // ...
    char animBoneTextNames[2][20];
    unsigned int animBoneNames[2];
    float radius;
    bool mirror;
    // padding byte
    // padding byte
    // padding byte
    float percent;
    float mass;
    float friction;
    int parentBone;
    PhysicsGeomType geomType;
};

struct BaseLerpBoneDef // sizeof=0x20
{                                       // ...
    char animBoneTextName[20];
    unsigned int animBoneName;
    unsigned int parentBoneIndex;
    int lerpTime;
};

struct Joint // sizeof=0x8
{                                       // ...
    void *joint;
    void *joint2;
};

struct JointDef // sizeof=0x54
{                                       // ...
    unsigned __int8 bone;
    // padding byte
    // padding byte
    // padding byte
    JointType type;
    int numLimitAxes;
    float limitAxes[3][3];
    float minAngles[3];
    float maxAngles[3];
    float axisFriction[3];
};

struct SelfPairDef // sizeof=0x2
{                                       // ...
    unsigned __int8 bones[2];
};

struct __declspec(align(4)) RagdollDef // sizeof=0xED0
{
    bool inUse;
    bool bound;
    // padding byte
    // padding byte
    int numBones;
    BoneDef boneDefs[14];
    int numBaseLerpBones;
    BaseLerpBoneDef baseLerpBoneDefs[9];
    int numJoints;
    JointDef jointDefs[28];
    int numSelfPairs;
    SelfPairDef selfPairDefs[33];
    // padding byte
    // padding byte
};

struct Bone // sizeof=0x1C
{                                       // ...
    int parentBone;
    unsigned __int8 animBones[2];
    // padding byte
    // padding byte
    dxBody *rigidBody; // TODO change to void* or uintptr_t if this ends up being wrong
    float length;
    float center[3];
};
struct __declspec(align(4)) LerpBone // sizeof=0x8
{                                       // ...
    int parentBone;
    unsigned __int8 animBone;
    // padding byte
    // padding byte
    // padding byte
};

struct BoneOrientation // sizeof=0x20
{                                       // ...
    float origin[3];
    int boneFlags;
    float orientation[4];
};

struct RagdollBody // sizeof=0x9D4
{
    int references;
    int ragdollDef;
    int dobj;
    DObj_s *obj;
    const cpose_t *pose;
    float poseOffset[3];
    int localClientNum;
    BodyState state;
    int stateMsec;
    int stateFrames;
    int velCaptureMsec;
    int numBones;
    Bone bones[14];
    int numLerpBones;
    LerpBone lerpBones[9];
    int numJoints;
    Joint joints[28];
    int curOrientationBuffer;
    BoneOrientation boneOrientations[2][23];
    BoneOrientation lerpBoneOffsets[9];
};

struct StateEnt // sizeof=0xC
{                                       // ...
    bool(__cdecl *enterFunc)(RagdollBody *, BodyState, BodyState);
    bool(__cdecl *exitFunc)(RagdollBody *, BodyState prevState, BodyState currState);
    void(__cdecl *updateFunc)(RagdollBody *);
};

// ragdoll
void __cdecl TRACK_ragdoll();
void __cdecl Ragdoll_DebugDraw();
RagdollDef *__cdecl Ragdoll_BodyDef(RagdollBody *body);
DObj_s *__cdecl Ragdoll_BodyDObj(RagdollBody *body);
centity_s *__cdecl Ragdoll_BodyPose(RagdollBody *body);
void __cdecl Ragdoll_BodyRootOrigin(RagdollBody *body, float *origin);
void __cdecl Ragdoll_GetRootOrigin(int ragdollHandle, float *origin);
int __cdecl Ragdoll_CountPhysicsBodies();
bool __cdecl Ragdoll_BodyHasPhysics(RagdollBody *body);
int __cdecl Ragdoll_CreateRagdollForDObj(int localClientNum, int ragdollDef, int dobj, bool reset, bool share);
int __cdecl Ragdoll_GetUnusedBody();
void __cdecl Ragdoll_InitBody(RagdollBody *body);
int __cdecl Ragdoll_ReferenceDObjBody(int dobj);
char __cdecl Ragdoll_BindDef(unsigned int ragdollDef);
bool __cdecl Ragdoll_ValidateDef(unsigned int ragdollDef);
void __cdecl Ragdoll_Remove(int ragdoll);
void __cdecl Ragdoll_FreeBody(int ragdollBody);
void __cdecl Ragdoll_InitDvars();
void __cdecl Ragdoll_Register();
void __cdecl Ragdoll_Init();
void __cdecl Ragdoll_Shutdown();



// ragdoll_controller
RagdollBody *__cdecl Ragdoll_HandleBody(int ragdollHandle);
BoneOrientation *__cdecl Ragdoll_BodyBoneOrientations(RagdollBody *body);
BoneOrientation *__cdecl Ragdoll_BodyPrevBoneOrientations(RagdollBody *body);
void __cdecl Ragdoll_DoControllers(const cpose_t *pose, DObj_s *obj, int *partBits);


// ragdoll_quat
void __cdecl Ragdoll_QuatMul(const float *qa, const float *qb, float *dest);
void __cdecl Ragdoll_QuatMulInvSecond(const float *qa, const float *qb, float *dest);
void __cdecl Ragdoll_QuatConjugate(const float *src, float *dest);
void __cdecl Ragdoll_QuatInverse(const float *src, float *dest);
void __cdecl Ragdoll_QuatPointRotate(const float *p, const float *q, float *dest);
void __cdecl Ragdoll_QuatNormalize(float *quat);
void __cdecl Ragdoll_Mat33ToQuat(const float (*axis)[3], float *quat);
void __cdecl Ragdoll_QuatToAxisAngle(const float *quat, float *axisAngle);
void __cdecl Ragdoll_QuatLerp(const float *qa, const float *qb, float t, float *dest);


// ragdoll_update
char __cdecl Ragdoll_ValidateBodyObj(RagdollBody *body);
void __cdecl Ragdoll_SnapshotBaseLerpOffsets(RagdollBody *body);
void __cdecl Ragdoll_GetDObjBaseBoneMatrix(DObj_s *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat);
void __cdecl Ragdoll_AnimMatToMat43(const DObjAnimMat *mat, float (*out)[3]);
char __cdecl Ragdoll_CreateBodyPhysics(RagdollBody *body);
char __cdecl Ragdoll_CreatePhysJoints(RagdollBody *body);
char __cdecl Ragdoll_CreatePhysJoint(RagdollBody *body, JointDef *jointDef, Joint *joint);
char __cdecl Ragdoll_CreatePhysObjs(RagdollBody *body);
char __cdecl Ragdoll_CreatePhysObj(RagdollBody *body, BoneDef *boneDef, Bone *bone);
char __cdecl Ragdoll_GetDObjBaseBoneOrigin(
    int localClientNum,
    DObj_s *obj,
    const float *offset,
    const mat3x3 &axis,
    unsigned __int8 boneIndex,
    float *origin);
char __cdecl Ragdoll_GetDObjBaseBoneOriginQuat(
    int localClientNum,
    DObj_s *obj,
    const float *offset,
    const mat3x3 &axis,
    unsigned __int8 boneIndex,
    float *origin,
    float *quat);
void __cdecl Ragdoll_PoseInvAxis(const cpose_t *pose, mat3x3 &invAxis);
void __cdecl Ragdoll_DestroyPhysJoints(RagdollBody *body);
void __cdecl Ragdoll_DestroyPhysObjs(RagdollBody *body);
void __cdecl Ragdoll_RemoveBodyPhysics(RagdollBody *body);
bool __cdecl Ragdoll_ValidatePrecalcBoneDef(RagdollDef *def, BoneDef *bone);
void __cdecl Ragdoll_GenerateAllSelfCollisionContacts();
void __cdecl Ragdoll_GenBoneCapsuleSegments(RagdollBody *body, unsigned __int8 *bones, float (*s0)[3], float (*s1)[3]);
void __cdecl Ragdoll_GenBoneCapsuleSegment(Bone *bone, float (*seg)[3]);
void __cdecl Ragdoll_AddSelfContact(
    RagdollBody *body,
    unsigned __int8 *bones,
    float radius0,
    float radius1,
    float *point0,
    float *point1);
void __cdecl Ragdoll_ExplosionEvent(
    int localClientNum,
    bool isCylinder,
    const float *origin,
    float innerRadius,
    float outerRadius,
    const float *impulse,
    float inScale);
void __cdecl Ragdoll_GetTorsoPosition(RagdollBody *body, float *center);
bool __cdecl Ragdoll_EnterTunnelTest(RagdollBody *body, BodyState curState, BodyState newState);
void __cdecl Ragdoll_SnapshotBaseLerpBones(RagdollBody *body, BoneOrientation *snapshot);
DObjAnimMat *__cdecl Ragdoll_GetDObjLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, unsigned __int8 boneIndex);
void __cdecl Ragdoll_SetCurrentPoseFromSnapshot(RagdollBody *body, BoneOrientation *snapshot);
void __cdecl Ragdoll_UpdateBodyContactCentroids(RagdollBody *body);
void __cdecl Ragdoll_BodyCenterOfMass(RagdollBody *body, float *com);
void __cdecl Ragdoll_EstimateInitialVelocities(RagdollBody *body);
char __cdecl Ragdoll_TunnelTest(RagdollBody *body);
int __cdecl Ragdoll_FindBoneChildren(RagdollBody *body, int boneIdx, int *childIndices);
char __cdecl Ragdoll_BoneTrace(trace_t *trace, trace_t *revTrace, float *start, float *end);
void __cdecl Ragdoll_PrintTunnelFail(RagdollBody *body);
void __cdecl Ragdoll_UpdateVelocityCapture(RagdollBody *body);
void __cdecl Ragdoll_SnapshotAnimOrientations(RagdollBody *body, BoneOrientation *snapshot);
char __cdecl Ragdoll_GetDObjWorldBoneOriginQuat(
    int localClientNum,
    const cpose_t *pose,
    DObj_s *obj,
    unsigned __int8 boneIndex,
    float *origin,
    float *quat);

bool __cdecl Ragdoll_EnterDead(RagdollBody *body, BodyState curState, BodyState newState);
bool __cdecl Ragdoll_ExitDead(RagdollBody *body, BodyState curState, BodyState newState);
bool __cdecl Ragdoll_ExitDObjWait(RagdollBody *body, BodyState prevState, BodyState curState);
bool __cdecl Ragdoll_ExitIdle(RagdollBody *body, BodyState curState, BodyState newState);
bool __cdecl Ragdoll_EnterIdle(RagdollBody *body, BodyState curState, BodyState newState);

void __cdecl Ragdoll_SnapshotBonePositions(RagdollBody *body, BoneOrientation *boneSnapshot);
bool __cdecl Ragdoll_EnterRunning(RagdollBody *body, BodyState curState, BodyState newState);
void __cdecl Ragdoll_UpdateDObjWait(RagdollBody *body);
void __cdecl Ragdoll_UpdateRunning(RagdollBody *body);
void __cdecl Ragdoll_UpdateFriction(RagdollBody *body);
char __cdecl Ragdoll_CheckIdle(RagdollBody *body);
void __cdecl Ragdoll_FilterBonePositions(RagdollBody *body);
char __cdecl Ragdoll_BodyNewState(RagdollBody *body, BodyState state);
void __cdecl Ragdoll_BodyUpdate(int msec, RagdollBody *body);
void __cdecl Ragdoll_Update(int msec);

void __cdecl Ragdoll_ResetBodiesUsingDef();

extern const dvar_t *ragdoll_self_collision_scale;
extern const dvar_t *ragdoll_bullet_force;
extern const dvar_t *ragdoll_jointlerp_time;
extern const dvar_t *ragdoll_jitter_scale;
extern const dvar_t *ragdoll_dump_anims;
extern const dvar_t *ragdoll_max_simulating;
extern const dvar_t *ragdoll_baselerp_time;
extern const dvar_t *ragdoll_fps;
extern const dvar_t *ragdoll_rotvel_scale;
extern const dvar_t *ragdoll_bullet_upbias;
extern const dvar_t *ragdoll_enable;
extern const dvar_t *ragdoll_debug;
extern const dvar_t *ragdoll_max_life;
extern const dvar_t *ragdoll_explode_force;
extern const dvar_t *ragdoll_explode_upbias;

extern BOOL ragdollInited;
extern RagdollDef ragdollDefs[2];
extern RagdollBody ragdollBodies[32];