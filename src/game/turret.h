#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct gentity_s;
struct actor_s;
struct ScriptFunctions;
struct weaponParms;

void __cdecl TRACK_turret();
void __cdecl G_InitTurrets();
int __cdecl Turret_GetRemainingConvergenceTime(const TurretInfo *turretInfo, unsigned int type);
void __cdecl Turret_FillWeaponParms(const gentity_s *ent, const gentity_s *activator, weaponParms *wp);
bool __cdecl MayHitTarget(const weaponParms *weapon, const gentity_s *target, const float *forward);
void __cdecl Fire_Lead(gentity_s *ent, gentity_s *activator, int bUseAccuracy);
void __cdecl clamp_playerbehindgun(gentity_s *self, gentity_s *other);
void __cdecl turret_clientaim(gentity_s *self, gentity_s *other);
void __cdecl turret_shoot_internal(gentity_s *self, gentity_s *other);
void __cdecl turret_track(gentity_s *self, gentity_s *other);
void __cdecl turret_UpdateSound(gentity_s *self);
void __cdecl turret_CalculateConvergenceAngularVelocity(const gentity_s *self, const float *desiredAngles, float *angularVelocity);
int __cdecl turret_UpdateTargetAngles(gentity_s *self, const float *desiredAngles, int bManned);
bool __cdecl turret_IsFiringInternal(int state);
bool __cdecl turret_IsFiring(gentity_s *self);
void __cdecl turret_SetState(gentity_s *self, unsigned int state);
void __cdecl turret_ClearTargetEnt(gentity_s *self);
int __cdecl turret_ReturnToDefaultPos(gentity_s *self, int bManned);
void __cdecl turret_SetDefaultDropPitch(gentity_s *self, double pitch);
void __cdecl turret_shoot(gentity_s *self);
int __cdecl turret_CanTargetPoint(const gentity_s *self, const float *vPoint, float *vSource, float *localAngles);
int __cdecl turret_CanTargetSentient(
    const gentity_s *self,
    const sentient_s *sentient,
    float *targetPosition,
    float *muzzlePosition,
    float *localAngles);
void __cdecl turret_aimat_vector_internal(gentity_s *self, float *origin, int bShoot, const float *desiredAngles);
int __cdecl turret_aimat_vector(gentity_s *self, float *origin, int bShoot, float *desiredAngles);
void __cdecl turret_SetTargetEnt(gentity_s *self, gentity_s *ent);
int __cdecl turret_aimat_Sentient_Internal(
    gentity_s *self,
    sentient_s *enemy,
    int bShoot,
    int missTime,
    float *desiredAngles);
int __cdecl turret_aimat_Sentient(gentity_s *self, sentient_s *enemy, int bShoot, int missTime);
int __cdecl turret_aimat_Ent(gentity_s *self, gentity_s *ent, int bShoot);
bool __cdecl turret_SightTrace(const float *start, const float *end, int passEnt1, int passEnt2);
int __cdecl turret_isTargetTooCloseToPlayer(
    const float *flashOrigin,
    const gentity_s *turret,
    const gentity_s *target);
int __cdecl turret_isTargetVisible(gentity_s *self, const gentity_s *target, float *distSqr);
sentient_s *__cdecl turret_findBestTarget(gentity_s *self);
void __cdecl turret_think_auto_nonai(gentity_s *self);
int __cdecl turret_think_auto(gentity_s *self, actor_s *actor);
int __cdecl turret_think_manual(gentity_s *self, actor_s *actor);
void __cdecl turret_RestoreDefaultDropPitch(gentity_s *self);
void __cdecl turret_think(gentity_s *self);
bool __cdecl Actor_IsTurretCloserThenCurrent(actor_s *actor, gentity_s *turret);
bool __cdecl turret_canuse_auto(gentity_s *self, actor_s *actor);
bool __cdecl turret_canuse_manual(gentity_s *self, actor_s *actor);
bool __cdecl turret_canuse(actor_s *actor, gentity_s *pTurret);
void __cdecl turret_controller(const gentity_s *self, int *partBits);
void __cdecl SP_turret_XAnimPrecache(ScriptFunctions *functions, const char *classname);
bool __cdecl turret_behind(gentity_s *self, gentity_s *other);
bool __cdecl G_IsTurretUsable(gentity_s *self, gentity_s *owner);
void __cdecl G_DeactivateTurret(gentity_s *self);
void __cdecl turret_use(gentity_s *self, gentity_s *owner, gentity_s *activator);
int __cdecl G_CanSpawnTurret();
void __cdecl G_SpawnTurret(gentity_s *self, const char *weaponinfoname);
void __cdecl SP_turret(gentity_s *self);
void __cdecl G_ClientStopUsingTurret(gentity_s *self);
void __cdecl turret_think_client(gentity_s *self);
void __cdecl turret_think_init(gentity_s *self);
void __cdecl G_FreeTurret(gentity_s *self);
