#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include <script/scr_variable.h>

// LWSS: not sure this .h is supposed to exist.

actor_s *__cdecl Actor_Get(scr_entref_t entref);
void __cdecl Actor_SetScriptGoalPos(actor_s *self, const float *vGoalPos, pathnode_t *node);
void __cdecl ActorCmd_StartScriptedAnim(scr_entref_t entref);
void __cdecl Actor_StartArrivalState(actor_s *self, ai_state_t newState);
void __cdecl ActorCmd_StartCoverArrival(scr_entref_t entref);
void __cdecl ActorCmd_StartTraverseArrival(scr_entref_t entref);
void __cdecl ActorCmd_CheckCoverExitPosWithPath(scr_entref_t entref);
void __cdecl ActorCmd_Shoot(scr_entref_t entref);
void __cdecl ActorCmd_ShootBlank(scr_entref_t entref);
void __cdecl ActorCmd_Melee(scr_entref_t entref);
void __cdecl ActorCmd_UpdatePlayerSightAccuracy(scr_entref_t entref);
void __cdecl ActorCmd_FindCoverNode(scr_entref_t entref);
void __cdecl ActorCmd_FindBestCoverNode(scr_entref_t entref);
void __cdecl ActorCmd_GetCoverNode(scr_entref_t entref);
void __cdecl ActorCmd_UseCoverNode(scr_entref_t entref);
void __cdecl ActorCmd_ReacquireStep(scr_entref_t entref);
void __cdecl ActorCmd_FindReacquireNode(scr_entref_t entref);
void __cdecl ActorCmd_GetReacquireNode(scr_entref_t entref);
void __cdecl ActorCmd_UseReacquireNode(scr_entref_t entref);
void __cdecl ActorCmd_FindReacquireDirectPath(scr_entref_t entref);
void __cdecl ActorCmd_FindReacquireProximatePath(scr_entref_t entref);
void __cdecl ActorCmd_TrimPathToAttack(scr_entref_t entref);
void __cdecl ActorCmd_ReacquireMove(scr_entref_t entref);
void __cdecl ActorCmd_FlagEnemyUnattackable(scr_entref_t entref);
void __cdecl ActorCmd_SetAimAnims(scr_entref_t entref);
void __cdecl ActorCmd_AimAtPos(scr_entref_t entref);
void __cdecl ActorCmd_EnterProne(scr_entref_t entref);
void __cdecl ActorCmd_ExitProne(scr_entref_t entref);
void __cdecl ActorCmd_SetProneAnimNodes(scr_entref_t entref);
void __cdecl ActorCmd_UpdateProne(scr_entref_t entref);
void __cdecl ActorCmd_ClearPitchOrient(scr_entref_t entref);
void __cdecl ActorCmd_SetLookAtAnimNodes(scr_entref_t entref);
void __cdecl ActorCmd_SetLookAt(scr_entref_t entref);
void __cdecl ActorCmd_SetLookAtYawLimits(scr_entref_t entref);
void __cdecl ActorCmd_StopLookAt(scr_entref_t entref);
void __cdecl ActorCmd_CanShoot(scr_entref_t entref);
void __cdecl ActorCmd_CanSee(scr_entref_t entref);
void __cdecl ActorCmd_DropWeapon(scr_entref_t entref);
void __cdecl ActorCmd_MayMoveToPoint(scr_entref_t entref);
void __cdecl ActorCmd_MayMoveFromPointToPoint(scr_entref_t entref);
void __cdecl ActorCmd_Teleport(scr_entref_t entref);
void __cdecl ActorCmd_WithinApproxPathDist(scr_entref_t entref);
void __cdecl ActorCmd_IsPathDirect(scr_entref_t entref);
void __cdecl ActorCmd_AllowedStances(scr_entref_t entref);
void __cdecl ActorCmd_IsStanceAllowed(scr_entref_t entref);
void __cdecl ActorCmd_IsSuppressionWaiting(scr_entref_t entref);
void __cdecl ActorCmd_IsSuppressed(scr_entref_t entref);
void __cdecl ActorCmd_IsMoveSuppressed(scr_entref_t entref);
void __cdecl ActorCmd_CheckGrenadeThrow(scr_entref_t entref);
void __cdecl ActorCmd_CheckGrenadeThrowPos(scr_entref_t entref);
void __cdecl ActorCmd_ThrowGrenade(scr_entref_t entref);
bool __cdecl Actor_CheckGrenadeLaunch(actor_s *self, const float *vStartPos, const float *vOffset);
void __cdecl ActorCmd_CheckGrenadeLaunch(scr_entref_t entref);
void __cdecl ActorCmd_CheckGrenadeLaunchPos(scr_entref_t entref);
void __cdecl ActorCmd_FireGrenadeLauncher(scr_entref_t entref);
void __cdecl ActorCmd_PickUpGrenade(scr_entref_t entref);
void __cdecl ActorCmd_UseTurret(scr_entref_t entref);
void __cdecl ActorCmd_StopUseTurret(scr_entref_t entref);
void __cdecl ActorCmd_CanUseTurret(scr_entref_t entref);
void __cdecl ActorCmd_TraverseMode(scr_entref_t entref);
void __cdecl ActorCmd_AnimMode(scr_entref_t entref);
void __cdecl ActorCmd_OrientMode(scr_entref_t entref);
void __cdecl ActorCmd_GetMotionAngle(scr_entref_t entref);
void __cdecl ActorCmd_GetAnglesToLikelyEnemyPath(scr_entref_t entref);
void __cdecl ActorCmd_SetTurretAnim(scr_entref_t entref);
void __cdecl ActorCmd_GetTurret(scr_entref_t entref);
void __cdecl ActorCmd_BeginPrediction(scr_entref_t entref);
void __cdecl ActorCmd_EndPrediction(scr_entref_t entref);
void __cdecl ActorCmd_LerpPosition(scr_entref_t entref);
void __cdecl ActorCmd_PredictOriginAndAngles(scr_entref_t entref);
void __cdecl ActorCmd_PredictAnim(scr_entref_t entref);
void __cdecl ActorCmd_GetHitEntType(scr_entref_t entref);
void __cdecl ActorCmd_GetHitYaw(scr_entref_t entref);
void __cdecl ActorCmd_GetGroundEntType(scr_entref_t entref);
void __cdecl ActorCmd_IsDeflected(scr_entref_t entref);
void __cdecl ActorCmd_trackScriptState(scr_entref_t entref);
void __cdecl ActorCmd_DumpHistory(scr_entref_t entref);
void __cdecl ScrCmd_animcustom(scr_entref_t entref);
void __cdecl ScrCmd_CanAttackEnemyNode(scr_entref_t entref);
void __cdecl ScrCmd_GetNegotiationStartNode(scr_entref_t entref);
void __cdecl ScrCmd_GetNegotiationEndNode(scr_entref_t entref);
void __cdecl ActorCmd_CheckProne(scr_entref_t entref);
void __cdecl ActorCmd_PushPlayer(scr_entref_t entref);
void __cdecl ActorCmd_SetGoalNode(scr_entref_t entref);
void __cdecl ActorCmd_SetGoalPos(scr_entref_t entref);
void __cdecl ActorCmd_SetGoalEntity(scr_entref_t entref);
void __cdecl ActorCmd_SetGoalVolume(scr_entref_t entref);
void __cdecl ActorCmd_GetGoalVolume(scr_entref_t entref);
void __cdecl ActorCmd_ClearGoalVolume(scr_entref_t entref);
void __cdecl ActorCmd_SetFixedNodeSafeVolume(scr_entref_t entref);
void __cdecl ActorCmd_GetFixedNodeSafeVolume(scr_entref_t entref);
void __cdecl ActorCmd_ClearFixedNodeSafeVolume(scr_entref_t entref);
void __cdecl ActorCmd_IsInGoal(scr_entref_t entref);
void __cdecl ActorCmd_SetOverrideRunToPos(scr_entref_t entref);
void __cdecl ActorCmd_NearNode(scr_entref_t entref);
void __cdecl ActorCmd_ClearEnemy(scr_entref_t entref);
void __cdecl ActorCmd_SetEntityTarget(scr_entref_t entref);
void __cdecl ActorCmd_ClearEntityTarget(scr_entref_t entref);
void __cdecl ActorCmd_SetPotentialThreat(scr_entref_t entref);
void __cdecl ActorCmd_ClearPotentialThreat(scr_entref_t entref);
void __cdecl ActorCmd_SetFlashBanged(scr_entref_t entref);
void __cdecl ActorCmd_SetFlashbangImmunity(scr_entref_t entref);
void __cdecl ActorCmd_GetFlashBangedStrength(scr_entref_t entref);
void __cdecl ActorCmd_SetEngagementMinDist(scr_entref_t entref);
void __cdecl ActorCmd_SetEngagementMaxDist(scr_entref_t entref);
void __cdecl ActorCmd_IsKnownEnemyInRadius(scr_entref_t entref);
void __cdecl ActorCmd_IsKnownEnemyInVolume(scr_entref_t entref);
void __cdecl ActorCmd_SetTalkToSpecies(scr_entref_t entref);


void(__cdecl *__cdecl Actor_GetMethod(const char **pName))(scr_entref_t);
