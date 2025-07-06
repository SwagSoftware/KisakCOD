#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_turret();
void __fastcall G_InitTurrets();
int __fastcall Turret_GetRemainingConvergenceTime(const TurretInfo *turretInfo, unsigned int type);
void __fastcall Turret_FillWeaponParms(const gentity_s *ent, const gentity_s *activator, weaponParms *wp);
bool __fastcall MayHitTarget(const weaponParms *weapon, const gentity_s *target, const float *forward);
void __fastcall Fire_Lead(gentity_s *ent, gentity_s *activator, int bUseAccuracy);
void __fastcall clamp_playerbehindgun(gentity_s *self, gentity_s *other);
void __fastcall turret_clientaim(gentity_s *self, gentity_s *other);
void __fastcall turret_shoot_internal(gentity_s *self, gentity_s *other);
void __fastcall turret_track(gentity_s *self, gentity_s *other);
void __fastcall turret_UpdateSound(gentity_s *self);
// local variable allocation has failed, the output may be wrong!
void __fastcall turret_CalculateConvergenceAngularVelocity(
    const gentity_s *self,
    const float *desiredAngles,
    float *angularVelocity,
    long double a4);
int __fastcall turret_UpdateTargetAngles(gentity_s *self, const float *desiredAngles, int bManned, long double a4);
bool __fastcall turret_IsFiringInternal(int state);
bool __fastcall turret_IsFiring(gentity_s *self);
void __fastcall turret_SetState(gentity_s *self, unsigned int state);
void __fastcall turret_ClearTargetEnt(gentity_s *self);
int __fastcall turret_ReturnToDefaultPos(gentity_s *self, int bManned);
void __fastcall turret_SetDefaultDropPitch(gentity_s *self, double pitch);
void __fastcall turret_shoot(gentity_s *self);
int __fastcall turret_CanTargetPoint(const gentity_s *self, const float *vPoint, float *vSource, float *localAngles);
int __fastcall turret_CanTargetSentient(
    const gentity_s *self,
    const sentient_s *sentient,
    float *targetPosition,
    float *muzzlePosition,
    float *localAngles);
void __fastcall turret_aimat_vector_internal(gentity_s *self, float *origin, int bShoot, const float *desiredAngles);
int __fastcall turret_aimat_vector(gentity_s *self, float *origin, int bShoot, float *desiredAngles);
void __fastcall turret_SetTargetEnt(gentity_s *self, gentity_s *ent);
// local variable allocation has failed, the output may be wrong!
int __fastcall turret_aimat_Sentient_Internal(
    gentity_s *self,
    sentient_s *enemy,
    int bShoot,
    int missTime,
    float *desiredAngles);
int __fastcall turret_aimat_Sentient(gentity_s *self, sentient_s *enemy, int bShoot, int missTime);
int __fastcall turret_aimat_Ent(gentity_s *self, gentity_s *ent, int bShoot);
bool __fastcall turret_SightTrace(const float *start, const float *end, int passEnt1, int passEnt2);
int __fastcall turret_isTargetTooCloseToPlayer(
    const float *flashOrigin,
    const gentity_s *turret,
    const gentity_s *target);
int __fastcall turret_isTargetVisible(gentity_s *self, const gentity_s *target, float *distSqr);
sentient_s *__fastcall turret_findBestTarget(gentity_s *self);
void __fastcall turret_think_auto_nonai(gentity_s *self);
int __fastcall turret_think_auto(gentity_s *self, actor_s *actor);
int __fastcall turret_think_manual(gentity_s *self, actor_s *actor);
void __fastcall turret_RestoreDefaultDropPitch(gentity_s *self);
void __fastcall turret_think(gentity_s *self);
bool __fastcall Actor_IsTurretCloserThenCurrent(actor_s *actor, gentity_s *turret);
bool __fastcall turret_canuse_auto(gentity_s *self, actor_s *actor);
bool __fastcall turret_canuse_manual(gentity_s *self, actor_s *actor);
bool __fastcall turret_canuse(actor_s *actor, gentity_s *pTurret);
void __fastcall turret_controller(const gentity_s *self, int *partBits);
void __fastcall SP_turret_XAnimPrecache(ScriptFunctions *functions, const char *classname);
bool __fastcall turret_behind(gentity_s *self, gentity_s *other, long double a3);
bool __fastcall G_IsTurretUsable(gentity_s *self, gentity_s *owner, long double a3);
void __fastcall G_DeactivateTurret(gentity_s *self);
void __fastcall turret_use(gentity_s *self, gentity_s *owner, gentity_s *activator);
int __fastcall G_CanSpawnTurret();
void __fastcall G_SpawnTurret(gentity_s *self, const char *weaponinfoname);
void __fastcall SP_turret(gentity_s *self);
void __fastcall G_ClientStopUsingTurret(gentity_s *self);
void __fastcall turret_think_client(gentity_s *self);
void __fastcall turret_think_init(gentity_s *self);
void __fastcall G_FreeTurret(gentity_s *self);
