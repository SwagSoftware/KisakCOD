#pragma once
#include "r_scene.h"
#include "rb_tess.h"

struct GfxModelRigidSurface // sizeof=0x38
{
    GfxModelSkinnedSurface surf;
    GfxScaledPlacement placement;
};
static_assert(sizeof(GfxModelRigidSurface) == 56);

struct SkinXModelCmd // sizeof=0x1C
{                                       // ...
    void *modelSurfs;
    const DObjAnimMat *mat;
    int surfacePartBits[4];
    unsigned __int16 surfCount;
    // padding byte
    // padding byte
};

int __cdecl DObjBad(const DObj_s *obj);
void __cdecl R_SkinSceneDObj(
    GfxSceneEntity *sceneEnt,
    GfxSceneEntity *localSceneEnt,
    const DObj_s *obj,
    DObjAnimMat *boneMatrix,
    int waitForCullState);
int  R_SkinSceneDObjModels(
    GfxSceneEntity *sceneEnt,
    const DObj_s *obj,
    DObjAnimMat *boneMatrix);
void __cdecl R_SkinGfxEntityCmd(GfxSceneEntity **data);
