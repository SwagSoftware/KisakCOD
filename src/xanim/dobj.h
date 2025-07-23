#pragma once

#include <ode/ode.h>
#include <qcommon/bitarray.h>

struct PhysPreset;

struct XAnimTree_s;
struct XModel;

#define DOBJ_MAX_PARTS 0x80
#define DOBJ_MAX_SUBMODELS 32

#define HIGH_BIT 0x80000000

#define NO_BONEINDEX 255

struct DObjTrace_s // sizeof=0x1C
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
static_assert(sizeof(DObjAnimMat) == 32);

struct DSkelPartBits // sizeof=0x30
{                                       // ...
    //int anim[4];                        // ...
    //int control[4];                     // ...
    //int skel[4];                        // ...
    bitarray<128> anim;
    bitarray<128> control;
    bitarray<128> skel;
};

struct DSkel // sizeof=0x38
{                                       // ...
    DSkelPartBits partBits;             // ...
    int timeStamp;                      // ...
    DObjAnimMat* mat;                   // ...
};

struct DObjSkelMat // sizeof=0x40
{                                       // ...
    float axis[3][4];
    float origin[4];
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
    volatile unsigned int locked;
    DSkel skel;
    float radius;
    unsigned int hidePartBits[4];
    XModel** models;
};

struct DObjModel_s // sizeof=0x8
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
void __cdecl DObjCreate(DObjModel_s *dobjModels, unsigned int numModels, XAnimTree_s *tree, DObj_s *buf, __int16 entnum);
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
const char *__cdecl DObjGetBoneName(const DObj_s *obj, int boneIndex);
char *__cdecl DObjGetModelParentBoneName(const DObj_s *obj, int modelIndex);
XAnimTree_s *__cdecl DObjGetTree(const DObj_s *obj);
void __cdecl DObjTraceline(DObj_s *obj, float *start, float *end, unsigned __int8 *priorityMap, DObjTrace_s *trace);
void __cdecl DObjTracelinePartBits(DObj_s *obj, int *partBits);
void __cdecl DObjGeomTraceline(
    DObj_s *obj,
    float *localStart,
    float *const localEnd,
    int contentmask,
    DObjTrace_s *results);
void __cdecl DObjGeomTracelinePartBits(DObj_s *obj, int contentmask, int *partBits);
int __cdecl DObjHasContents(DObj_s *obj, int contentmask);
int __cdecl DObjGetContents(const DObj_s *obj);
int __cdecl DObjSetLocalBoneIndex(DObj_s *obj, int *partBits, int boneIndex, const float *trans, const float *angles);
int __cdecl DObjGetBoneIndex(const DObj_s *obj, unsigned int name, unsigned __int8 *index);
int __cdecl DObjGetModelBoneIndex(const DObj_s *obj, const char *modelName, unsigned int name, unsigned __int8 *index);
void __cdecl DObjGetBasePoseMatrix(const DObj_s *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat);
void __cdecl DObjSetHidePartBits(DObj_s *obj, const unsigned int *partBits);


// dobj_skel
void __cdecl DObjCalcSkel(const DObj_s *obj, int *partBits);
void __cdecl GetControlAndDuplicatePartBits(
    const DObj_s *obj,
    const int *partBits,
    const int *ignorePartBits,
    const int *savedDuplicatePartBits,
    int *calcPartBits,
    int *controlPartBits);
const unsigned __int8 *__cdecl CalcSkelDuplicateBones(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    const unsigned __int8 *pos);
void __cdecl CalcSkelRootBonesNoParentOrDuplicate(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    int *calcPartBits);
void __cdecl CalcSkelRootBonesWithParent(
    const XModel *model,
    DSkel *skel,
    unsigned int minBoneIndex,
    unsigned int modelParent,
    int *calcPartBits,
    const int *controlPartBits);
void __cdecl CalcSkelNonRootBones(
    const XModel *model,
    DSkel *skel,
    int minBoneIndex,
    int *calcPartBits,
    const int *controlPartBits);
void __cdecl DObjCalcBaseSkel(const DObj_s *obj, DObjAnimMat *mat, int *partBits);
void __cdecl DObjCalcBaseAnim(const DObj_s *obj, DObjAnimMat *mat, int *partBits);
void __cdecl DObjGetBaseControlAndDuplicatePartBits(
    const DObj_s *obj,
    const int *partBits,
    const int *ignorePartBits,
    const int *savedDuplicatePartBits,
    int *calcPartBits,
    int *controlPartBits);
