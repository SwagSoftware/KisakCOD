#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif


int __cdecl Path_IsValidClaimNode(const pathnode_t *node);
int __cdecl Path_IsCoverNode(const pathnode_t *node);
team_t __cdecl Sentient_EnemyTeam(unsigned int eTeam);
void __cdecl TRACK_actor();
void __cdecl VisCache_Copy(vis_cache_t *pDstCache, const vis_cache_t *pSrcCache);
void __cdecl VisCache_Update(vis_cache_t *pCache, bool bVisible);
void __cdecl SentientInfo_Clear(sentient_info_t *pInfo);
void __cdecl SentientInfo_ForceCopy(sentient_info_t *pTo, const sentient_info_t *pFrom);
int __cdecl Actor_droptofloor(gentity_s *ent);
int __cdecl Actor_IsDeletable(actor_s *actor);
void __cdecl G_InitActors();
unsigned int __cdecl G_GetActorIndex(actor_s *actor);
XAnimTree_s *__cdecl G_GetActorAnimTree(actor_s *actor);
XAnimTree_s *__cdecl G_AllocAnimClientTree();
void __cdecl G_FreeAnimClientTree(XAnimTree_s *tree);
void __cdecl Actor_SetDefaults(actor_s *actor);
void __cdecl Actor_FinishSpawning(actor_s *self);
void __cdecl Actor_InitAnimScript(actor_s *self);
actor_s *__cdecl Actor_FirstActor(int iTeamFlags);
actor_s *__cdecl Actor_NextActor(actor_s *pPrevActor, int iTeamFlags);
void __cdecl Actor_ClearArrivalPos(actor_s *self);
void __cdecl Actor_PreThink(actor_s *self);
void __cdecl Actor_ValidateReacquireNodes(actor_s *self);
void __cdecl Actor_Touch(gentity_s *self, gentity_s *other, int bTouched);
bool __cdecl Actor_InScriptedState(const actor_s *self);
int __cdecl Actor_CheckDeathAllowed(actor_s *self, int damage);
void __cdecl Actor_Pain(
    gentity_s *self,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    float *vDir,
    __int32 hitLoc,
    unsigned int weaponIdx);
void __cdecl Actor_Die(
    gentity_s *self,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    int iWeapon,
    float *vDir,
    hitLocation_t hitLoc);
bool __cdecl Actor_IsDying(const actor_s *self);
bool __cdecl usingCodeGoal(actor_s *actor);
gentity_s *__cdecl Actor_GetTargetEntity(actor_s *self);
sentient_s *__cdecl Actor_GetTargetSentient(actor_s *self);
void __cdecl Actor_GetTargetPosition(actor_s *self, float *position);
void __cdecl Actor_GetTargetLookPosition(actor_s *self, float *position);
void __cdecl Actor_InitMove(actor_s *self);
bool __cdecl Actor_IsDodgeEntity(actor_s *self, int entnum);
int __cdecl Actor_Physics_GetLeftOrRightDodge(actor_s *self, bool dodgeRight, double length);
void __cdecl Actor_PhysicsBackupInputs(actor_s *self, PhysicsInputs *inputs);
void __cdecl Actor_PhysicsRestoreInputs(actor_s *self, PhysicsInputs *inputs);
bool __cdecl Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther);
float __cdecl Actor_CalcultatePlayerPushDelta(const actor_s *self, const gentity_s *pusher, float *pushDir);
bool __cdecl Actor_ShouldMoveAwayFromCloseEnt(actor_s *self);
void __cdecl Actor_UpdateProneInformation(actor_s *self, int bDoProneCheck, float *a3, float *a4, long double a5);
void __cdecl actor_controller(const gentity_s *self, int *partBits);
bool __cdecl Actor_PointNear(const float *vPoint, const float *vGoalPos);
bool __cdecl Actor_PointNearNode(const float *vPoint, const pathnode_t *node);
int __cdecl Actor_PointAtGoal(const float *vPoint, const actor_goal_s *goal);
bool __cdecl Actor_PointNearGoal(const float *vPoint, const actor_goal_s *goal, double buffer);
bool __cdecl Actor_PointNearPoint(const float *vPoint, const float *vGoalPos, double buffer);
bool __cdecl Actor_PointAt(const float *vPoint, const float *vGoalPos);
void __cdecl Actor_HandleInvalidPath(actor_s *self);
pathnode_t *__cdecl Actor_FindClaimedNode(actor_s *self);
bool __cdecl Actor_EnemyInPathFightDist(actor_s *self, sentient_s *enemy);
bool __cdecl Actor_IsCloseToSegment(
    float *origin,
    float *pathPoint,
    double len,
    float *dir,
    double requiredDistFromPathSq,
    float *a6);
int __cdecl Actor_IsAlongPath(actor_s *self, float *origin, float *pathPoint, int hadPath);
int __cdecl Actor_IsDoingCover(actor_s *self);
bool __cdecl Actor_IsReactingToEnemyDuringReacquireMove(actor_s *self);
gentity_s *__cdecl Actor_IsKnownEnemyInRegion(
    const actor_s *self,
    const gentity_s *volume,
    const float *position,
    double radius);
int __cdecl Actor_InFixedNodeExposedCombat(actor_s *self);
bool __cdecl Actor_HasPath(actor_s *self);
void __cdecl Actor_InitPath(actor_s *self);
void __cdecl Actor_ClearPath(actor_s *self);
void __cdecl Actor_GetAnimDeltas(actor_s *self, float *rotation, float *translation);
int __cdecl Actor_IsMovingToMeleeAttack(actor_s *self);
bool __cdecl Actor_SkipPathEndActions(actor_s *self);
void __cdecl Actor_PathEndActions(actor_s *self);
void __cdecl Actor_PredictAnim(actor_s *self);
bool __cdecl Actor_AtClaimNode(actor_s *self);
bool __cdecl Actor_NearClaimNode(actor_s *self, double dist);
void __cdecl Actor_CheckCollisions(actor_s *self);
void __cdecl Actor_ClearPileUp(actor_s *self);
void __cdecl Actor_ClipPathToGoal(actor_s *self);
void __cdecl Actor_BeginTrimPath(actor_s *self);
int __cdecl Actor_TrimPathToAttack(actor_s *self);
int __cdecl Actor_MayReacquireMove(actor_s *self);
void __cdecl Actor_ClearMoveHistory(actor_s *self);
void __cdecl Actor_GetMoveHistoryAverage(actor_s *self, float *vDir);
void __cdecl Actor_UpdateMoveHistory(actor_s *self);
void __cdecl Path_UpdateLeanAmount(actor_s *self, float *vWishDir);
float __cdecl Path_UpdateMomentum(actor_s *self, float *vWishDir, double fMoveDist);
bool __cdecl Path_UseMomentum(actor_s *self);
void __cdecl Path_UpdateMovementDelta(actor_s *self, double fMoveDist);
void __cdecl Actor_AddStationaryMoveHistory(actor_s *self);
int __cdecl Actor_IsMoving(actor_s *self);
unsigned int __cdecl G_GetActorFriendlyIndex(int iEntNum);
void __cdecl G_BypassForCG_GetClientActorIndexAndTeam(int iEntNum, int *actorIndex, int *team);
unsigned int __cdecl G_BypassForCG_GetClientActorFriendlyIndex(int iEntNum);
gentity_s *__cdecl G_GetFriendlyIndexActor(int iFriendlyIndex);
void __cdecl Actor_SetFlashed(actor_s *self, int flashed, double strength);
void __cdecl Actor_UpdateDesiredChainPosInternal(
    actor_s *self,
    int iFollowMin,
    int iFollowMax,
    sentient_s *pGoalSentient);
void __cdecl Actor_UpdateDesiredChainPos(actor_s *self);
void __cdecl Actor_CheckOverridePos(actor_s *self, const float *prevGoalPos);
void __cdecl Actor_SetGoalRadius(actor_goal_s *goal, double radius);
void __cdecl Actor_SetGoalHeight(actor_goal_s *goal, double height);
bool __cdecl Actor_IsInsideArc(
    actor_s *self,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight);
void __cdecl SentientInfo_Copy(actor_s *pTo, const actor_s *pFrom, int index);
actor_s *__cdecl Actor_Alloc();
void __cdecl Actor_Free(actor_s *actor);
void __cdecl Actor_FreeExpendable();
void __cdecl Actor_FinishSpawningAll();
void __cdecl Actor_DissociateSentient(actor_s *self, sentient_s *other, team_t eOtherTeam);
void __cdecl Actor_NodeClaimRevoked(actor_s *self, int invalidTime);
void __cdecl Actor_CheckClearNodeClaimCloseEnt(actor_s *self);
int __cdecl Actor_KeepClaimedNode(actor_s *self);
void __cdecl Actor_ClearKeepClaimedNode(actor_s *self);
void __cdecl Actor_CheckNodeClaim(actor_s *self);
void __cdecl Actor_UpdatePlayerPush(actor_s *self, gentity_s *player);
void __cdecl Actor_UpdateCloseEnt(actor_s *self);
actor_think_result_t __cdecl Actor_CallThink(actor_s *self);
void __cdecl Actor_EntInfo(gentity_s *self, float *source);
int __cdecl Actor_MoveAwayNoWorse(actor_s *self);
int __cdecl Actor_PhysicsCheckMoveAwayNoWorse(
    actor_s *self,
    gentity_s *other,
    int flags,
    double distanceSqrd,
    double lengthSqrd);
int __cdecl Actor_PhysicsMoveAway(actor_s *self);
int __cdecl Actor_IsAtScriptGoal(actor_s *self);
bool __cdecl Actor_IsNearClaimedNode(actor_s *self);
int __cdecl Actor_IsFixedNodeUseable(actor_s *self);
bool __cdecl Actor_FindPath(
    actor_s *self,
    const float *vGoalPos,
    int bAllowNegotiationLinks,
    bool ignoreSuppression);
void __cdecl Actor_RecalcPath(actor_s *self);
bool __cdecl Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable);
bool __cdecl Actor_FindPathToSentient(actor_s *self, sentient_s *pGoalEnt, int bSuppressable);
void __cdecl Actor_FindPathInGoalWithLOS(
    actor_s *self,
    const float *vGoalPos,
    double fWithinDistSqrd,
    bool ignoreSuppression,
    char a5);
void __cdecl Actor_FindPathAway(
    actor_s *self,
    const float *vBadPos,
    double fMinSafeDist,
    int bAllowNegotiationLinks);
void __cdecl Actor_FindPathAwayNotCrossPlanes(
    actor_s *self,
    const float *vBadPos,
    double fMinSafeDist,
    float *normal,
    double dist,
    float *bSuppressable,
    int bAllowNegotiationLinks,
    int a8);
void __cdecl Actor_BadPlacesChanged();
void __cdecl Actor_UpdateAnglesAndDelta(actor_s *self);
void __cdecl Actor_UpdatePileUp(actor_s *self);
void __cdecl Actor_UpdateGoalPos(actor_s *self);
int __cdecl SP_actor(gentity_s *ent);
int __cdecl Actor_CheckGoalNotify(actor_s *self);
void __cdecl Actor_CheckNotify(actor_s *self);
void __cdecl Actor_Think(gentity_s *self);
int __cdecl Actor_PhysicsAndDodge(actor_s *self);
void __cdecl Actor_DoMove(actor_s *self);
bool __cdecl Actor_IsAtGoal(actor_s *self);
bool __cdecl Actor_FindPathToGoalDirectInternal(actor_s *self);
void __cdecl Actor_FindPathToGoalDirect(actor_s *self);
int __cdecl Actor_FindPathToClaimNode(actor_s *self, pathnode_t *node);
int __cdecl Actor_CheckStop(actor_s *self, bool canUseEnemyGoal, pathnode_t *node, int hadPath);
void __cdecl Actor_TryPathToArrivalPos(actor_s *self);
void __cdecl Actor_FindPathToFixedNode(actor_s *self);
void __cdecl Actor_FindPathToGoal(actor_s *self);
void __cdecl Actor_UpdateOriginAndAngles(actor_s *self);
void __cdecl Actor_PredictOriginAndAngles(actor_s *self);
void __cdecl Actor_PostThink(actor_s *self);


static const float actorMins[3] = { -15.0, -15.0, 0.0 };
static const float actorMaxs[3] = { 15.0, 15.0, 72.0 };