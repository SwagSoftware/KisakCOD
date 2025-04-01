#pragma once

struct PhysPreset;

struct XAnimTree_s;
struct XModel;

enum PhysWorld : __int32
{                                       // ...
    PHYS_WORLD_DYNENT = 0x0,
    PHYS_WORLD_FX = 0x1,
    PHYS_WORLD_RAGDOLL = 0x2,
    PHYS_WORLD_COUNT = 0x3,
};

// KISAKTODO replace with ode
struct dxBody;
struct dxJoint;
struct dxSpace;

struct dxWorld;

struct dBase // sizeof=0x0
{                                       // ...
};

struct dxQuickStepParameters // sizeof=0x8
{                                       // ...
    int num_iterations;
    float w;
};

struct dxContactParameters // sizeof=0x8
{                                       // ...
    float max_vel;
    float min_depth;
};

struct dxWorldStepInfo // sizeof=0x2C
{                                       // ...
    float gravity[4];
    float global_erp;
    float global_cfm;
    dxQuickStepParameters qs;
    unsigned int holdrand;
    dxContactParameters contactp;
};

struct dxAutoDisable // sizeof=0x10
{                                       // ...
    float linear_threshold;
    float angular_threshold;
    float idle_time;
    int idle_steps;
};

struct SorLcpData // sizeof=0x6F0
{                                       // ...
    int order[444];
};

struct ConstraintRowData // sizeof=0x90
{                                       // ...
    float J_body1Linear[3];
    float lambda;
    float J_body1Angular[3];
    int body1;
    float J_body2Linear[3];
    int body2;
    float J_body2Angular[3];
    float padding1;
    float iMJ_body1Linear[3];
    float padding2;
    float iMJ_body1Angular[3];
    float padding3;
    float iMJ_body2Linear[3];
    float padding4;
    float iMJ_body2Angular[3];
    float padding5;
    float lo;
    float hi;
    float rhs;
    float Ad;
};

struct QuickstepData // sizeof=0xF9C0
{                                       // ...
    ConstraintRowData rowData[444];
};

struct dObject : dBase // sizeof=0x14
{                                       // ...
    dxWorld *world;
    dObject *next;
    dObject **tome;
    void *userdata;
    int tag;
};

struct dxJointNode // sizeof=0x10
{                                       // ...
    dxJoint *joint;
    dxBody *body;
    int bodyTag;
    dxJointNode *next;
};

struct __declspec(align(4)) dxJoint : dObject // sizeof=0x40
{                                       // ...
    int typenum;
    int flags;
    dxJointNode node[2];
    bool debug;
    // padding byte
    // padding byte
    // padding byte
};

struct dxGeom : dBase // sizeof=0x4C
{                                       // ...
    //dxGeom_vtbl *__vftable;
    virtual ~dxGeom()
    {
    }
    virtual void computeAABB()
    {
        return;
    }
    virtual int AABBTest(dxGeom* o, float* aabb)
    {
        return 1;
    }

    int type;
    int gflags;
    void* data;
    dxBody* body;
    dxGeom* body_next;
    float* pos;
    float* R;
    dxGeom* next;
    dxGeom** tome;
    dxSpace* parent_space;
    float aabb[6];
    unsigned int category_bits;
    unsigned int collide_bits;
};

struct __declspec(align(4)) dxSpace : dxGeom // sizeof=0x64
{                                       // ...
    int count;
    dxGeom *first;
    int cleanup;
    int current_index;
    dxGeom *current_geom;
    int lock_count;
};

struct dMass // sizeof=0x44
{                                       // ...
    float mass;                         // ...
    float c[4];
    float I[12];                        // ...
};

struct dxBodyInfo // sizeof=0x70
{                                       // ...
    float pos[4];
    float q[4];
    float R[12];
    float lvel[4];
    float avel[4];
};

struct dxBody : dObject // sizeof=0x150
{                                       // ...
    dxJointNode *firstjoint;
    int flags;
    dxGeom *geom;
    dMass mass;
    float invI[12];
    float invMass;
    float facc[4];
    float tacc[4];
    float finite_rot_axis[4];
    dxAutoDisable adis;
    float adis_timeleft;
    int adis_stepsleft;
    dxBodyInfo info;
};

struct dxWorld : dBase // sizeof=0x10110
{                                       // ...
    dxBody* firstbody;
    dxJoint* firstjoint;
    int nb;
    int nj;
    dxWorldStepInfo stepInfo;
    dxAutoDisable adis;
    int adis_flag;
    float seconds;
    SorLcpData sd;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    QuickstepData qd;
};

struct __declspec(align(4)) DObjTrace_s // sizeof=0x1C
{                                       // ...
    float fraction;                     // ...
    int surfaceflags;                   // ...
    float normal[3];                    // ...
    unsigned __int16 modelIndex;        // ...
    unsigned __int16 partName;          // ...
    unsigned __int16 partGroup;         // ...
    // padding byte
    // padding byte
};

struct DObjAnimMat // sizeof=0x20
{                                       // ...
    float quat[4];                      // ...
    float trans[3];                     // ...
    float transWeight;                  // ...
};

struct DSkelPartBits // sizeof=0x30
{                                       // ...
    int anim[4];                        // ...
    int control[4];                     // ...
    int skel[4];                        // ...
};

struct DSkel // sizeof=0x38
{                                       // ...
    DSkelPartBits partBits;             // ...
    int timeStamp;                      // ...
    DObjAnimMat* mat;                   // ...
};

struct DObj_s // sizeof=0x64
{
    XAnimTree_s* tree;
    unsigned __int16 duplicateParts;
    unsigned __int16 entnum;
    unsigned __int8 duplicatePartsSize;
    unsigned __int8 numModels;
    unsigned __int8 numBones;
    // padding byte
    unsigned int ignoreCollision;
    volatile int locked;
    DSkel skel;
    float radius;
    unsigned int hidePartBits[4];
    XModel** models;
};

struct __declspec(align(2)) DObjModel_s // sizeof=0x8
{                                       // ...
    XModel* model;                      // ...
    unsigned __int16 boneName;          // ...
    bool ignoreCollision;               // ...
    // padding byte
};

void __cdecl DObjInit();
void __cdecl DObjShutdown();
void __cdecl DObjDumpInfo(const DObj_s *obj);
bool __cdecl DObjIgnoreCollision(const DObj_s *obj, char modelIndex);
void __cdecl DObjGetHierarchyBits(const DObj_s *obj, int boneIndex, int *partBits);
bool __cdecl DObjSkelIsBoneUpToDate(DObj_s *obj, int boneIndex);
void __cdecl DObjSetTree(DObj_s *obj, XAnimTree_s *tree);
void __cdecl DObjCreate(DObjModel_s *dobjModels, unsigned int numModels, XAnimTree_s *tree, char *buf, __int16 entnum);
void __cdecl DObjCreateDuplicateParts(DObj_s *obj, DObjModel_s *dobjModels, unsigned int numModels);
void __cdecl DObjDumpCreationInfo(DObjModel_s *dobjModels, unsigned int numModels);
void __cdecl DObjComputeBounds(DObj_s *obj);
void __cdecl DObjFree(DObj_s *obj);
void __cdecl DObjGetCreateParms(
    const DObj_s *obj,
    DObjModel_s *dobjModels,
    unsigned __int16 *numModels,
    XAnimTree_s **tree,
    unsigned __int16 *entnum);
void __cdecl DObjArchive(DObj_s *obj);
void __cdecl DObjUnarchive(DObj_s *obj);
void __cdecl DObjSkelClear(const DObj_s *obj);
void __cdecl DObjGetBounds(const DObj_s *obj, float *mins, float *maxs);
void __cdecl DObjPhysicsGetBounds(const DObj_s *obj, float *mins, float *maxs);
void __cdecl DObjPhysicsSetCollisionFromXModel(const DObj_s *obj, PhysWorld worldIndex, dxBody *physId);
double __cdecl DObjGetRadius(const DObj_s *obj);
PhysPreset *__cdecl DObjGetPhysPreset(const DObj_s *obj);
const char *__cdecl DObjGetName(const DObj_s *obj);
char *__cdecl DObjGetBoneName(const DObj_s *obj, int boneIndex);
char *__cdecl DObjGetModelParentBoneName(const DObj_s *obj, int modelIndex);
XAnimTree_s *__cdecl DObjGetTree(const DObj_s *obj);
void __cdecl DObjTraceline(DObj_s *obj, float *start, float *end, unsigned __int8 *priorityMap, DObjTrace_s *trace);
void __cdecl LocalInvMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out);
void __cdecl DObjTracelinePartBits(DObj_s *obj, int *partBits);
void __cdecl DObjGeomTraceline(DObj_s *obj, float *localStart, __int64 localEnd, DObjTrace_s *results);
void __cdecl DObjGeomTracelinePartBits(DObj_s *obj, int contentmask, int *partBits);
int __cdecl DObjHasContents(DObj_s *obj, int contentmask);
int __cdecl DObjGetContents(const DObj_s *obj);
int __cdecl DObjSetLocalBoneIndex(DObj_s *obj, int *partBits, int boneIndex, const float *trans, const float *angles);
int __cdecl DObjGetBoneIndex(const DObj_s *obj, unsigned int name, unsigned __int8 *index);
int __cdecl DObjGetModelBoneIndex(const DObj_s *obj, const char *modelName, unsigned int name, unsigned __int8 *index);
void __cdecl DObjGetBasePoseMatrix(const DObj_s *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat);
void __cdecl DObjSetHidePartBits(DObj_s *obj, const unsigned int *partBits);
