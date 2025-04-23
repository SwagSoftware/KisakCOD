#pragma once
#include <xanim/xanim.h>
#include "r_scene.h"


void __cdecl TRACK_r_model();
void __cdecl R_ModelList_f();
bool __cdecl R_ModelSort(XModel **model1, XModel **model2);
void __cdecl R_GetModelList(XAssetHeader header, XAssetHeader *data);
XModel *__cdecl R_RegisterModel(const char *name);
unsigned __int8 *__cdecl Hunk_AllocXModelPrecache(unsigned int size);
unsigned __int8 *__cdecl Hunk_AllocXModelPrecacheColl(unsigned int size);
void __cdecl R_XModelDebug(const DObj_s *obj, int *partBits);
void __cdecl R_XModelDebugBoxes(const DObj_s *obj, int *partBits);
void __cdecl R_XModelDebugAxes(const DObj_s *obj, int *partBits);
int __cdecl R_SkinXModel(
    XModelDrawInfo* modelInfo,
    const XModel* model,
    const DObj_s* obj,
    const GfxPlacement* placement,
    float scale,
    __int16 gfxEntIndex);
void __cdecl R_SkinSceneEnt(GfxSceneEntity *sceneEnt);
int __cdecl R_SkinAndBoundSceneEnt(GfxSceneEntity *sceneEnt);
void __cdecl R_UnlockSkinnedCache();
void __cdecl R_LockSkinnedCache();