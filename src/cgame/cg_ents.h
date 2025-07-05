#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif


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

int __fastcall CompressUnit(double unit);
void __fastcall LocalConvertQuatToMat(const DObjAnimMat *mat, float (*axis)[3]);
unsigned int __fastcall Com_GetPrimaryLightCount();
const ComPrimaryLight *__fastcall Com_GetPrimaryLight(unsigned int primaryLightIndex);
void __fastcall CG_LockLightingOrigin(centity_s *cent, float *lightingOrigin);
int __fastcall CG_GetRenderFlagForRefEntity(__int16 eFlags);
void __fastcall CG_General(int localClientNum, centity_s *cent);
void __fastcall CG_Item(centity_s *cent);
void __fastcall CG_AddEntityLoopSound(int localClientNum, const centity_s *cent);
void __fastcall CG_EntityEffects(int localClientNum, centity_s *cent);
void __fastcall CG_mg42_PreControllers(int localClientNum, const DObj_s *obj, centity_s *cent);
void __fastcall CG_mg42(int localClientNum, centity_s *cent);
bool __fastcall JavelinSoftLaunch(WeaponDef *weapDef, cg_s *cgameGlob, entityState_s *s1);
void __fastcall CG_Missile(int localClientNum, centity_s *cent);
bool __fastcall CG_VecLessThan(float *a, float *b);
void __fastcall CG_UpdateBModelWorldBounds(unsigned int localClientNum, centity_s *cent, int forceFilter);
void __fastcall CG_ScriptMover(int localClientNum, centity_s *cent);
void __fastcall CG_AdjustPositionForMover(
    int localClientNum,
    float *in,
    int moverNum,
    int fromTime,
    int toTime,
    float *out,
    float *outDeltaAngles);
void __fastcall CG_SetFrameInterpolation(int localClientNum);
void __fastcall CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj_s *obj, bool notify);
centity_s *__fastcall CG_GetPose(int localClientNum, int handle);
void __fastcall CG_Vehicle_PreControllers(int localClientNum, const DObj_s *obj, centity_s *cent);
void __fastcall CG_Vehicle(int localClientNum, centity_s *cent);
void __fastcall CG_SoundBlend(int localClientNum, centity_s *cent);
FxEffect *__fastcall CG_StartFx(int localClientNum, centity_s *cent, int startAtTime);
void __fastcall CG_Fx(int localClientNum, centity_s *cent);
void __fastcall CG_LoopFx(int localClientNum, centity_s *cent);
void __fastcall CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight);
void __fastcall CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight);
// local variable allocation has failed, the output may be wrong!
void __fastcall CG_PrimaryLight(int localClientNum, centity_s *cent);
void __fastcall CG_InterpolateEntityOrigin(const cg_s *cgameGlob, centity_s *cent);
void __fastcall CG_InterpolateEntityAngles(const cg_s *cgameGlob, centity_s *cent);
void __fastcall CG_CreatePhysicsObject(int localClientNum, centity_s *cent);
void __fastcall CG_UpdatePhysicsPose(centity_s *cent);
int __fastcall CG_ExpiredLaunch(int localClientNum, centity_s *cent);
void __fastcall CG_CalcEntityPhysicsPositions(int localClientNum, centity_s *cent);
void __fastcall CG_SaveEntityPhysics(centity_s *cent, SaveGame *save);
void __fastcall CG_LoadEntityPhysics(centity_s *cent, SaveGame *save);
void __fastcall CG_CreateRagdollObject(int localClientNum, centity_s *cent);
void __fastcall CG_UpdateRagdollPose(centity_s *cent);
void __fastcall CG_CalcEntityRagdollPositions(int localClientNum, centity_s *cent);
void __fastcall CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent);
void __fastcall CG_DObjCalcBone(const cpose_t *pose, DObj_s *obj, int boneIndex);
void __fastcall CG_DrawEntEqDebug(const centity_s *cent);
void __fastcall CG_ClearUnion(int localClientNum, centity_s *cent);
void __fastcall CG_SetUnionType(int localClientNum, centity_s *cent);
void __fastcall CG_UpdatePoseUnion(int localClientNum, centity_s *cent);
void __fastcall CG_ProcessEntity(int localClientNum, centity_s *cent);
void __fastcall CG_SaveEntityFX(centity_s *cent, SaveGame *save);
void __fastcall CG_LoadEntityFX(centity_s *cent, SaveGame *save);
void __fastcall CG_SaveEntity(unsigned int entnum, SaveGame *save);
void __fastcall CG_LoadEntity(unsigned int entnum, SaveGame *save);
void __fastcall CG_SaveEntities(SaveGame *save);
void __fastcall CG_LoadEntities(SaveGame *save);
void __fastcall CG_GetPoseOrigin(const cpose_t *pose, float *origin);
void __fastcall CG_GetPoseAngles(const cpose_t *pose, float *angles);
float *__fastcall CG_GetEntityOrigin(int localClientNum, int entnum);
void __fastcall CG_GetPoseLightingHandle(const cpose_t *pose);
void __fastcall CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt);
void __fastcall CG_AddPacketEntity(unsigned int localClientNum, unsigned int entnum);
int __fastcall CG_AddPacketEntities(int localClientNum);
DObjAnimMat *__fastcall CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, int boneIndex);
DObjAnimMat *__fastcall CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj_s *obj, unsigned int tagName);
DObjAnimMat *__fastcall CG_DObjGetWorldBoneMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    int boneIndex,
    float (*tagMat)[3],
    float *origin);
DObjAnimMat *__fastcall CG_DObjGetWorldTagMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    unsigned int tagName,
    float (*tagMat)[3],
    float *origin);
int __fastcall CG_DObjGetWorldTagPos(const cpose_t *pose, DObj_s *obj, unsigned int tagName, float *pos);
