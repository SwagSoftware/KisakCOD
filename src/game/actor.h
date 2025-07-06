#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall Path_IsValidClaimNode(const pathnode_t *node);
int __fastcall Path_IsCoverNode(const pathnode_t *node);
team_t __fastcall Sentient_EnemyTeam(unsigned int eTeam);
void __fastcall TRACK_actor();
void __fastcall VisCache_Copy(vis_cache_t *pDstCache, const vis_cache_t *pSrcCache);
void __fastcall VisCache_Update(vis_cache_t *pCache, bool bVisible);
void __fastcall SentientInfo_Clear(sentient_info_t *pInfo);
void __fastcall SentientInfo_ForceCopy(sentient_info_t *pTo, const sentient_info_t *pFrom);
int __fastcall Actor_droptofloor(gentity_s *ent);
int __fastcall Actor_IsDeletable(actor_s *actor);
void __fastcall G_InitActors();
unsigned int __fastcall G_GetActorIndex(actor_s *actor);
XAnimTree_s *__fastcall G_GetActorAnimTree(actor_s *actor);
XAnimTree_s *__fastcall G_AllocAnimClientTree();
void __fastcall G_FreeAnimClientTree(XAnimTree_s *tree);
void __fastcall Actor_SetDefaults(actor_s *actor);
void __fastcall Actor_FinishSpawning(actor_s *self);
void __fastcall Actor_InitAnimScript(actor_s *self);
actor_s *__fastcall Actor_FirstActor(int iTeamFlags);
actor_s *__fastcall Actor_NextActor(actor_s *pPrevActor, int iTeamFlags);
void __fastcall Actor_ClearArrivalPos(actor_s *self);
void __fastcall Actor_PreThink(actor_s *self);
void __fastcall Actor_ValidateReacquireNodes(actor_s *self);
void __fastcall Actor_Touch(gentity_s *self, gentity_s *other, int bTouched);
bool __fastcall Actor_InScriptedState(const actor_s *self);
int __fastcall Actor_CheckDeathAllowed(actor_s *self, int damage);
void __fastcall Actor_Pain(
    gentity_s *self,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    float *vDir,
    __int32 hitLoc,
    unsigned int weaponIdx);
void __fastcall Actor_Die(
    gentity_s *self,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    int iWeapon,
    float *vDir,
    hitLocation_t hitLoc);
bool __fastcall Actor_IsDying(const actor_s *self);
bool __fastcall usingCodeGoal(actor_s *actor);
gentity_s *__fastcall Actor_GetTargetEntity(actor_s *self);
sentient_s *__fastcall Actor_GetTargetSentient(actor_s *self);
void __fastcall Actor_GetTargetPosition(actor_s *self, float *position);
void __fastcall Actor_GetTargetLookPosition(actor_s *self, float *position);
void __fastcall Actor_InitMove(actor_s *self);
bool __fastcall Actor_IsDodgeEntity(actor_s *self, int entnum);
int __fastcall Actor_Physics_GetLeftOrRightDodge(actor_s *self, bool dodgeRight, double length);
void __fastcall Actor_PhysicsBackupInputs(actor_s *self, PhysicsInputs *inputs);
void __fastcall Actor_PhysicsRestoreInputs(actor_s *self, PhysicsInputs *inputs);
bool __fastcall Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther);
float __fastcall Actor_CalcultatePlayerPushDelta(const actor_s *self, const gentity_s *pusher, float *pushDir);
bool __fastcall Actor_ShouldMoveAwayFromCloseEnt(actor_s *self);
void __fastcall Actor_UpdateProneInformation(actor_s *self, int bDoProneCheck, float *a3, float *a4, long double a5);
void __fastcall actor_controller(const gentity_s *self, int *partBits);
bool __fastcall Actor_PointNear(const float *vPoint, const float *vGoalPos);
bool __fastcall Actor_PointNearNode(const float *vPoint, const pathnode_t *node);
int __fastcall Actor_PointAtGoal(const float *vPoint, const actor_goal_s *goal);
bool __fastcall Actor_PointNearGoal(const float *vPoint, const actor_goal_s *goal, double buffer);
bool __fastcall Actor_PointNearPoint(const float *vPoint, const float *vGoalPos, double buffer);
bool __fastcall Actor_PointAt(const float *vPoint, const float *vGoalPos);
void __fastcall Actor_HandleInvalidPath(actor_s *self);
pathnode_t *__fastcall Actor_FindClaimedNode(actor_s *self);
bool __fastcall Actor_EnemyInPathFightDist(actor_s *self, sentient_s *enemy);
bool __fastcall Actor_IsCloseToSegment(
    float *origin,
    float *pathPoint,
    double len,
    float *dir,
    double requiredDistFromPathSq,
    float *a6);
int __fastcall Actor_IsAlongPath(actor_s *self, float *origin, float *pathPoint, int hadPath);
int __fastcall Actor_IsDoingCover(actor_s *self);
bool __fastcall Actor_IsReactingToEnemyDuringReacquireMove(actor_s *self);
gentity_s *__fastcall Actor_IsKnownEnemyInRegion(
    const actor_s *self,
    const gentity_s *volume,
    const float *position,
    double radius);
int __fastcall Actor_InFixedNodeExposedCombat(actor_s *self);
bool __fastcall Actor_HasPath(actor_s *self);
void __fastcall Actor_InitPath(actor_s *self);
void __fastcall Actor_ClearPath(actor_s *self);
void __fastcall Actor_GetAnimDeltas(actor_s *self, float *rotation, float *translation);
int __fastcall Actor_IsMovingToMeleeAttack(actor_s *self);
bool __fastcall Actor_SkipPathEndActions(actor_s *self);
void __fastcall Actor_PathEndActions(actor_s *self);
void __fastcall Actor_PredictAnim(actor_s *self);
bool __fastcall Actor_AtClaimNode(actor_s *self);
bool __fastcall Actor_NearClaimNode(actor_s *self, double dist);
void __fastcall Actor_CheckCollisions(actor_s *self);
void __fastcall Actor_ClearPileUp(actor_s *self);
void __fastcall Actor_ClipPathToGoal(actor_s *self);
void __fastcall Actor_BeginTrimPath(actor_s *self);
int __fastcall Actor_TrimPathToAttack(actor_s *self);
int __fastcall Actor_MayReacquireMove(actor_s *self);
void __fastcall Actor_ClearMoveHistory(actor_s *self);
void __fastcall Actor_GetMoveHistoryAverage(actor_s *self, float *vDir);
void __fastcall Actor_UpdateMoveHistory(actor_s *self);
void __fastcall Path_UpdateLeanAmount(actor_s *self, float *vWishDir);
float __fastcall Path_UpdateMomentum(actor_s *self, float *vWishDir, double fMoveDist);
bool __fastcall Path_UseMomentum(actor_s *self);
void __fastcall Path_UpdateMovementDelta(actor_s *self, double fMoveDist);
void __fastcall Actor_AddStationaryMoveHistory(actor_s *self);
int __fastcall Actor_IsMoving(actor_s *self);
unsigned int __fastcall G_GetActorFriendlyIndex(int iEntNum);
void __fastcall G_BypassForCG_GetClientActorIndexAndTeam(int iEntNum, int *actorIndex, int *team);
unsigned int __fastcall G_BypassForCG_GetClientActorFriendlyIndex(int iEntNum);
gentity_s *__fastcall G_GetFriendlyIndexActor(int iFriendlyIndex);
void __fastcall Actor_SetFlashed(actor_s *self, int flashed, double strength);
void __fastcall Actor_UpdateDesiredChainPosInternal(
    actor_s *self,
    int iFollowMin,
    int iFollowMax,
    sentient_s *pGoalSentient);
void __fastcall Actor_UpdateDesiredChainPos(actor_s *self);
void __fastcall Actor_CheckOverridePos(actor_s *self, const float *prevGoalPos);
void __fastcall Actor_SetGoalRadius(actor_goal_s *goal, double radius);
void __fastcall Actor_SetGoalHeight(actor_goal_s *goal, double height);
bool __fastcall Actor_IsInsideArc(
    actor_s *self,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight);
void __fastcall SentientInfo_Copy(actor_s *pTo, const actor_s *pFrom, int index);
actor_s *__fastcall Actor_Alloc();
void __fastcall Actor_Free(actor_s *actor);
void __fastcall Actor_FreeExpendable();
void __fastcall Actor_FinishSpawningAll();
void __fastcall Actor_DissociateSentient(actor_s *self, sentient_s *other, team_t eOtherTeam);
void __fastcall Actor_NodeClaimRevoked(actor_s *self, int invalidTime);
void __fastcall Actor_CheckClearNodeClaimCloseEnt(actor_s *self);
int __fastcall Actor_KeepClaimedNode(actor_s *self);
void __fastcall Actor_ClearKeepClaimedNode(actor_s *self);
void __fastcall Actor_CheckNodeClaim(actor_s *self);
void __fastcall Actor_UpdatePlayerPush(actor_s *self, gentity_s *player);
void __fastcall Actor_UpdateCloseEnt(actor_s *self);
actor_think_result_t __fastcall Actor_CallThink(actor_s *self);
void __fastcall Actor_EntInfo(gentity_s *self, float *source);
int __fastcall Actor_MoveAwayNoWorse(actor_s *self);
int __fastcall Actor_PhysicsCheckMoveAwayNoWorse(
    actor_s *self,
    gentity_s *other,
    int flags,
    double distanceSqrd,
    double lengthSqrd);
int __fastcall Actor_PhysicsMoveAway(actor_s *self);
int __fastcall Actor_IsAtScriptGoal(actor_s *self);
bool __fastcall Actor_IsNearClaimedNode(actor_s *self);
int __fastcall Actor_IsFixedNodeUseable(actor_s *self);
bool __fastcall Actor_FindPath(
    actor_s *self,
    const float *vGoalPos,
    int bAllowNegotiationLinks,
    bool ignoreSuppression);
void __fastcall Actor_RecalcPath(actor_s *self);
bool __fastcall Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable);
bool __fastcall Actor_FindPathToSentient(actor_s *self, sentient_s *pGoalEnt, int bSuppressable);
void __fastcall Actor_FindPathInGoalWithLOS(
    actor_s *self,
    const float *vGoalPos,
    double fWithinDistSqrd,
    bool ignoreSuppression,
    char a5);
void __fastcall Actor_FindPathAway(
    actor_s *self,
    const float *vBadPos,
    double fMinSafeDist,
    int bAllowNegotiationLinks);
void __fastcall Actor_FindPathAwayNotCrossPlanes(
    actor_s *self,
    const float *vBadPos,
    double fMinSafeDist,
    float *normal,
    double dist,
    float *bSuppressable,
    int bAllowNegotiationLinks,
    int a8);
void __fastcall Actor_BadPlacesChanged();
void __fastcall Actor_UpdateAnglesAndDelta(actor_s *self);
void __fastcall Actor_UpdatePileUp(actor_s *self);
void __fastcall Actor_UpdateGoalPos(actor_s *self);
int __fastcall SP_actor(gentity_s *ent);
int __fastcall Actor_CheckGoalNotify(actor_s *self);
void __fastcall Actor_CheckNotify(actor_s *self);
void __fastcall Actor_Think(gentity_s *self);
int __fastcall Actor_PhysicsAndDodge(actor_s *self);
void __fastcall Actor_DoMove(actor_s *self);
bool __fastcall Actor_IsAtGoal(actor_s *self);
bool __fastcall Actor_FindPathToGoalDirectInternal(actor_s *self);
void __fastcall Actor_FindPathToGoalDirect(actor_s *self);
int __fastcall Actor_FindPathToClaimNode(actor_s *self, pathnode_t *node);
int __fastcall Actor_CheckStop(actor_s *self, bool canUseEnemyGoal, pathnode_t *node, int hadPath);
void __fastcall Actor_TryPathToArrivalPos(actor_s *self);
void __fastcall Actor_FindPathToFixedNode(actor_s *self);
void __fastcall Actor_FindPathToGoal(actor_s *self);
void __fastcall Actor_UpdateOriginAndAngles(actor_s *self);
void __fastcall Actor_PredictOriginAndAngles(actor_s *self);
void __fastcall Actor_PostThink(actor_s *self);
