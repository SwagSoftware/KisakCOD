#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl TRACK_actor_grenade();
void __cdecl SP_info_grenade_hint(gentity_s *ent);
bool __cdecl Actor_Grenade_IsValidTrajectory(
    actor_s *self,
    const float *vFrom,
    float *vVelocity,
    const float *vGoal);
void __cdecl Actor_Grenade_GetTossFromPosition(
    actor_s *self,
    const float *vStandPos,
    const float *vOffset,
    float *vFrom);
void __cdecl Actor_Grenade_GetTossPositions(
    const float *vFrom,
    float *vTargetPos,
    float *vLand,
    unsigned int grenadeWPID);
int __cdecl Actor_Grenade_GetTossPositionsFromHints(
    const float *vFrom,
    const float *vTargetPos,
    const unsigned int method,
    float *vLand);
bool __cdecl Actor_Grenade_CheckMaximumEnergyToss(
    actor_s *self,
    float *vFrom,
    float *vLand,
    int bLob,
    float *vVelOut);
bool __cdecl Actor_Grenade_CheckInfiniteEnergyToss(actor_s *self, float *vFrom, float *vLand, float *vVelOut);
bool __cdecl Actor_Grenade_CheckMinimumEnergyToss(actor_s *self, float *vFrom, float *vLand, float *vVelOut);
int __cdecl Actor_Grenade_CheckGrenadeHintToss(actor_s *self, float *vFrom, float *vLand, float *vVelOut);
int __cdecl compare_desperate_hints(float *pe0, float *pe1);
int __cdecl Actor_Grenade_CheckDesperateToss(actor_s *self, float *vFrom, float *vVelOut);
bool __cdecl Actor_GrenadeLauncher_CheckPos(
    actor_s *self,
    const float *vStandPos,
    const float *vOffset,
    const float *vTargetPos,
    double speed,
    float *vPosOut,
    float *vVelOut,
    float *a8);
int __cdecl Actor_Grenade_IsSafeTarget(actor_s *self, const float *vTargetPos, unsigned int iWeapID);
void __cdecl Actor_PredictGrenadeLandPos(gentity_s *pGrenade);
bool __cdecl Actor_Grenade_IsPointSafe(actor_s *self, const float *vPoint);
float __cdecl Actor_Grenade_EscapePlane(actor_s *self, float *normal);
void __cdecl Actor_Grenade_GetPickupPos(actor_s *self, const float *enemyPos, float *vGrenadePickupPos);
bool __cdecl Actor_Grenade_ShouldIgnore(actor_s *self, gentity_s *grenade);
int __cdecl Actor_IsAwareOfGrenade(actor_s *self);
void __cdecl Actor_GrenadePing(actor_s *self, gentity_s *pGrenade);
void __cdecl Actor_DissociateGrenade(gentity_s *pGrenade);
void __cdecl Actor_Grenade_Attach(actor_s *self);
void __cdecl Actor_Grenade_Detach(actor_s *self);
int __cdecl Actor_Grenade_InActorHands(gentity_s *grenade);
int __cdecl Actor_Grenade_Resume(actor_s *self, ai_state_t ePrevState);
void __cdecl Actor_Grenade_Cower(actor_s *self);
void __cdecl Actor_Grenade_Combat(actor_s *self);
void __cdecl Actor_Grenade_CoverAttack(actor_s *self);
void __cdecl Actor_Grenade_Flee(actor_s *self);
void __cdecl Actor_Grenade_TakeCover(actor_s *self);
int __cdecl Actor_Grenade_ThrowBack(actor_s *self);
void __cdecl G_DrawGrenadeHints(int a1, const float *a2, int a3, int a4, __int64 a5);
bool __cdecl Actor_Grenade_CheckTossPos(
    actor_s *self,
    float *vStandPos,
    float *vOffset,
    float *vTargetPos,
    unsigned int method,
    float *vPosOut,
    float *vVelOut,
    double randomRange,
    int bRechecking,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29);
bool __cdecl Actor_Grenade_AttemptReturnTo(
    actor_s *self,
    float *vFrom,
    float *vEnemyPos,
    float *vLand,
    float *vVelocity);
int __cdecl Actor_Grenade_AttemptReturn(actor_s *self);
void __cdecl Actor_Grenade_DropIfHeld(actor_s *self);
void __cdecl Actor_Grenade_Finish(actor_s *self, ai_state_t eNextState);
void __cdecl Actor_Grenade_Suspend(actor_s *self, ai_state_t eNextState);
bool __cdecl Actor_Grenade_CheckToss(
    actor_s *self,
    float *vStandPos,
    float *vOffset,
    unsigned int method,
    float *vPosOut,
    float *vVelOut,
    double randomRange,
    int bRechecking,
    int a9);
void __cdecl Actor_Grenade_AttemptEscape(actor_s *self, int bForceAbortPath);
void __cdecl Actor_Grenade_DecideResponse(actor_s *self);
int __cdecl Actor_Grenade_ReevaluateResponse(actor_s *self, ActorGrenadeReevaluateMode reevaluateMode);
void __cdecl Actor_GrenadeBounced(gentity_s *pGrenade, gentity_s *pHitEnt);
int __cdecl Actor_Grenade_Start(actor_s *self, ai_state_t ePrevState);
int __cdecl Actor_Grenade_Acquire(actor_s *self);
actor_think_result_t __cdecl Actor_Grenade_Think(actor_s *self);
