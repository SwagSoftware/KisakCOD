#pragma once
#include <xanim/xanim.h>
#include "r_scene.h"


void __cdecl TRACK_r_model();
void __cdecl R_ModelList_f();
void __cdecl R_GetModelList(XAssetHeader header, XAssetHeader *data);
XModel *__cdecl R_RegisterModel(const char *name);
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

void R_DObjReplaceMaterial(DObj_s *obj, int lod, int surfaceIndex, Material *material);
void R_DObjGetSurfMaterials(DObj_s *obj, int lod, Material **matHandleArray);
void R_SetIgnorePrecacheErrors(unsigned int ignore);