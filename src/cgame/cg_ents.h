#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <universal/q_shared.h>

struct DObjAnimMat;
struct ComPrimaryLight;
struct centity_s;
struct DObj_s;
struct WeaponDef;
struct cg_s;
struct entityState_s;
struct FxEffect;
struct GfxLight;
struct SaveGame;
struct cpose_t;
struct GfxSceneEntity;

int __cdecl CompressUnit(double unit);
void __cdecl LocalConvertQuatToMat(const DObjAnimMat *mat, float (*axis)[3]);
const ComPrimaryLight *__cdecl Com_GetPrimaryLight(unsigned int primaryLightIndex);
void __cdecl CG_LockLightingOrigin(centity_s *cent, float *lightingOrigin);
int __cdecl CG_GetRenderFlagForRefEntity(__int16 eFlags);
void __cdecl CG_General(int localClientNum, centity_s *cent);
void __cdecl CG_Item(centity_s *cent);
void __cdecl CG_AddEntityLoopSound(int localClientNum, const centity_s *cent);
void __cdecl CG_EntityEffects(int localClientNum, centity_s *cent);
void __cdecl CG_mg42_PreControllers(int localClientNum, const DObj_s *obj, centity_s *cent);
void __cdecl CG_mg42(int localClientNum, centity_s *cent);
bool __cdecl JavelinSoftLaunch(WeaponDef *weapDef, cg_s *cgameGlob, entityState_s *s1);
void __cdecl CG_Missile(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateBModelWorldBounds(unsigned int localClientNum, centity_s *cent, int forceFilter);
void __cdecl CG_ScriptMover(int localClientNum, centity_s *cent);
void __cdecl CG_AdjustPositionForMover(
    int localClientNum,
    float *in,
    int moverNum,
    int fromTime,
    int toTime,
    float *out,
    float *outDeltaAngles);
void __cdecl CG_SetFrameInterpolation(int localClientNum);
void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj_s *obj, bool notify);
struct cpose_t *__cdecl CG_GetPose(int localClientNum, int handle);
void __cdecl CG_Vehicle_PreControllers(int localClientNum, const DObj_s *obj, centity_s *cent);
void __cdecl CG_Vehicle(int localClientNum, centity_s *cent);
void __cdecl CG_SoundBlend(int localClientNum, centity_s *cent);
FxEffect *__cdecl CG_StartFx(int localClientNum, centity_s *cent, int startAtTime);
void __cdecl CG_Fx(int localClientNum, centity_s *cent);
void __cdecl CG_LoopFx(int localClientNum, centity_s *cent);
void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight);
void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight);
// local variable allocation has failed, the output may be wrong!
void __cdecl CG_PrimaryLight(int localClientNum, centity_s *cent);
void __cdecl CG_InterpolateEntityOrigin(const cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_InterpolateEntityAngles(const cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_CreatePhysicsObject(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePhysicsPose(centity_s *cent);
int __cdecl CG_ExpiredLaunch(int localClientNum, centity_s *cent);
void __cdecl CG_CalcEntityPhysicsPositions(int localClientNum, centity_s *cent);
void __cdecl CG_SaveEntityPhysics(centity_s *cent, SaveGame *save);
void __cdecl CG_LoadEntityPhysics(centity_s *cent, SaveGame *save);
void __cdecl CG_CreateRagdollObject(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateRagdollPose(centity_s *cent);
void __cdecl CG_CalcEntityRagdollPositions(int localClientNum, centity_s *cent);
void __cdecl CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent);
void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj_s *obj, int boneIndex);
void __cdecl CG_DrawEntEqDebug(const centity_s *cent);
void __cdecl CG_ClearUnion(int localClientNum, centity_s *cent);
void __cdecl CG_SetUnionType(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePoseUnion(int localClientNum, centity_s *cent);
void __cdecl CG_ProcessEntity(int localClientNum, centity_s *cent);
void __cdecl CG_SaveEntityFX(centity_s *cent, SaveGame *save);
void __cdecl CG_LoadEntityFX(centity_s *cent, SaveGame *save);
void __cdecl CG_SaveEntity(unsigned int entnum, SaveGame *save);
void __cdecl CG_LoadEntity(unsigned int entnum, SaveGame *save);
void __cdecl CG_SaveEntities(SaveGame *save);
void __cdecl CG_LoadEntities(SaveGame *save);
void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin);
void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles);
float *__cdecl CG_GetEntityOrigin(int localClientNum, int entnum);
void __cdecl CG_GetPoseLightingHandle(const cpose_t *pose);
void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt);
void __cdecl CG_AddPacketEntity(unsigned int localClientNum, unsigned int entnum);
int __cdecl CG_AddPacketEntities(int localClientNum);
DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, int boneIndex);
DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj_s *obj, unsigned int tagName);
int32_t __cdecl CG_DObjGetWorldBoneMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    int boneIndex,
    float (*tagMat)[3],
    float *origin);
int32_t __cdecl CG_DObjGetWorldTagMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    unsigned int tagName,
    float (*tagMat)[3],
    float *origin);
int __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj_s *obj, unsigned int tagName, float *pos);
