#pragma once
#include <xanim/dobj.h>
#include "r_scene.h"


DObjAnimMat * R_UpdateSceneEntBounds(
    GfxSceneEntity *sceneEnt,
    GfxSceneEntity **pLocalSceneEnt,
    const DObj_s **pObj,
    int waitForCullState);
DObjAnimMat *__cdecl R_DObjCalcPose(const GfxSceneEntity *sceneEnt, const DObj_s *obj, int *partBits);
void __cdecl R_SetNoDraw(GfxSceneEntity *sceneEnt);
void __cdecl R_UpdateGfxEntityBoundsCmd(GfxSceneEntity **data);
