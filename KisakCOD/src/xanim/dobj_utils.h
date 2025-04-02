#pragma once
#include "dobj.h"
#include "xanim.h"

DObjAnimMat *__cdecl DObjGetRotTransArray(const DObj_s *obj);
int __cdecl DObjGetNumModels(const DObj_s *obj);
int __cdecl DObjGetSurfaces(const DObj_s *obj, int *partBits, const char *lods);
void __cdecl DObjGetSurfaceData(const DObj_s *obj, const float *origin, float scale, char *lods);
void __cdecl DObjGetBoneInfo(const DObj_s *obj, XBoneInfo **boneInfo);
int __cdecl DObjNumBones(const DObj_s *obj);
int __cdecl DObjGetLodForDist(const DObj_s *obj, int modelIndex, float dist);
void __cdecl DObjGetSetBones(const DObj_s *obj, int *setPartBits);
unsigned int __cdecl DObjGetRootBoneCount(const DObj_s *obj);
int __cdecl DObjSetRotTransIndex(DObj_s *obj, const int *partBits, int boneIndex);
char __cdecl DObjSetSkelRotTransIndex(DObj_s *obj, const int *partBits, int boneIndex);
void __cdecl DObjSetControlTagAngles(DObj_s *obj, int *partBits, unsigned int boneIndex, float *angles);
XModel *__cdecl DObjGetModel(const DObj_s *obj, int modelIndex);
void __cdecl DObjSetLocalTag(
    DObj_s *obj,
    int *partBits,
    unsigned int boneIndex,
    const float *trans,
    const float *angles);
void __cdecl DObjSetLocalTagInternal(const DObj_s *obj, const float *trans, const float *angles, int boneIndex);
void __cdecl DObjSetAngles(DObjAnimMat *rotTrans, const float *angles);
void __cdecl DObjClearAngles(DObjAnimMat *rotTrans);
void __cdecl DObjSetTrans(DObjAnimMat *rotTrans, const float *trans);
void __cdecl DObjCompleteHierarchyBits(const DObj_s *obj, int *partBits);
int __cdecl DObjSetControlRotTransIndex(DObj_s *obj, const int *partBits, int boneIndex);
bool __cdecl DObjSkelExists(const DObj_s *obj, int timeStamp);
void __cdecl DObjClearSkel(const DObj_s *obj);
int __cdecl DObjSkelAreBonesUpToDate(const DObj_s *obj, int *partBits);
int __cdecl DObjGetAllocSkelSize(const DObj_s *obj);
void __cdecl DObjCreateSkel(DObj_s *obj, char *buf, int timeStamp);
DObjAnimMat *__cdecl I_dmaGetDObjSkel(const DObj_s *obj);
void __cdecl DObjGetHidePartBits(const DObj_s *obj, unsigned int *partBits);

void __cdecl DObjLock(DObj_s *obj);
void __cdecl DObjUnlock(DObj_s *obj);
