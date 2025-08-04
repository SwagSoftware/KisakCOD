#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <bgame/bg_local.h>
#include "pathnode.h"
#include <bgame/bg_actor_prone.h>

#include "actor_animapi.h"
#include "actor_physics.h"
#include "actor_navigation.h"
#include "actor_suppression.h"

#include "sentient.h"
#include "teams.h"

enum AISpecies : __int32
{
    AI_SPECIES_HUMAN = 0x0,
    AI_SPECIES_DOG = 0x1,
    MAX_AI_SPECIES = 0x2,
    AI_SPECIES_ALL = 0x2,
};

enum ai_state_t : __int32
{
    AIS_INVALID = 0x0,
    AIS_KEEPCURRENT = 0x0,
    AIS_EXPOSED = 0x1,
    AIS_TURRET = 0x2,
    AIS_GRENADE_RESPONSE = 0x3,
    AIS_BADPLACE_FLEE = 0x4,
    AIS_COVERARRIVAL = 0x5,
    AIS_DEATH = 0x6,
    AIS_DEFAULT = 0x1,
    AIS_SETABLE_FIRST = 0x1,
    AIS_SETABLE_LAST = 0x6,
    AIS_PAIN = 0x7,
    AIS_SCRIPTEDANIM = 0x8,
    AIS_CUSTOMANIM = 0x9,
    AIS_NEGOTIATION = 0xA,
    AIS_PUSHABLE_FIRST = 0x7,
    AIS_PUSHABLE_LAST = 0xA,
    AIS_COUNT = 0xB,
};

enum ai_substate_t : __int32
{
    STATE_EXPOSED_COMBAT = 0x64,
    STATE_EXPOSED_NONCOMBAT = 0x65,
    STATE_EXPOSED_REACQUIRE_MOVE = 0x66,
    STATE_EXPOSED_FLASHBANGED = 0x67,
    STATE_DEATH_PRECLEANUP = 0xC8,
    STATE_DEATH_POSTCLEANUP = 0xC9,
    STATE_GRENADE_FLEE = 0x12C,
    STATE_GRENADE_TAKECOVER = 0x12D,
    STATE_GRENADE_COWER = 0x12E,
    STATE_GRENADE_COMBAT = 0x12F,
    STATE_GRENADE_COVERATTACK = 0x130,
    STATE_GRENADE_ACQUIRE = 0x131,
    STATE_GRENADE_THROWBACK = 0x132,
};

enum ai_state_transition_t : __int32
{
    AIS_TRANSITION_CANONICAL = 0xFFFFFFFF,
    AIS_TRANSITION_NONE = 0x0,
    AIS_TRANSITION_SET = 0x1,
    AIS_TRANSITION_PUSH = 0x2,
    AIS_TRANSITION_POP = 0x3,
};

enum actor_think_result_t : __int32
{
    ACTOR_THINK_DONE = 0x0,
    ACTOR_THINK_REPEAT = 0x1,
    ACTOR_THINK_MOVE_TO_BODY_QUEUE = 0x2,
};

enum aiGoalSources : __int32
{
    AI_GOAL_SRC_SCRIPT_GOAL = 0x0,
    AI_GOAL_SRC_SCRIPT_ENTITY_GOAL = 0x1,
    AI_GOAL_SRC_FRIENDLY_CHAIN = 0x2,
    AI_GOAL_SRC_ENEMY = 0x3,
};

enum ai_orient_mode_t : __int32
{
    AI_ORIENT_INVALID = 0x0,
    AI_ORIENT_DONT_CHANGE = 0x1,
    AI_ORIENT_TO_MOTION = 0x2,
    AI_ORIENT_TO_ENEMY = 0x3,
    AI_ORIENT_TO_ENEMY_OR_MOTION = 0x4,
    AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP = 0x5,
    AI_ORIENT_TO_GOAL = 0x6,
    AI_ORIENT_COUNT = 0x7,
};

enum ai_traverse_mode_t : __int32
{
    AI_TRAVERSE_INVALID = 0x0,
    AI_TRAVERSE_GRAVITY = 0x1,
    AI_TRAVERSE_NOGRAVITY = 0x2,
    AI_TRAVERSE_NOCLIP = 0x3,
    AI_TRAVERSE_COUNT = 0x4,
};

enum ai_stance_e : __int32
{
    STANCE_BAD = 0x0,
    STANCE_STAND = 0x1,
    STANCE_CROUCH = 0x2,
    STANCE_PRONE = 0x4,
    STANCE_ANY = 0x7,
};
inline ai_stance_e operator|(ai_stance_e a, ai_stance_e b) 
{
    return static_cast<ai_stance_e>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}
inline ai_stance_e &operator|=(ai_stance_e &a, ai_stance_e b) 
{
    a = a | b;
    return a;
}

struct potential_threat_t
{
    bool isEnabled;
    float direction[2];
};

struct ai_transition_cmd_t
{
    ai_state_transition_t eTransition;
    ai_state_t eState;
};

struct ActorCoverArrivalInfo
{
    int arrivalNotifyRequested;
    int animscriptOverrideRunTo;
    float animscriptOverrideRunToPos[3];
    float animscriptOverrideOriginError[3];
};

struct ActorCachedInfo
{
    int time;
    float pos[3];
    float dir[3];
};

struct ActorLookAtInfo
{
    float vLookAtPos[3];
    float fLookAtTurnAngle;
    float fLookAtTurnSpeed;
    float fLookAtTurnAccel;
    float fLookAtAnimYawLimit;
    float fLookAtYawLimit;
    unsigned __int16 animLookAtStraight;
    unsigned __int16 animLookAtLeft;
    unsigned __int16 animLookAtRight;
    bool bDoLookAt;
    bool bLookAtSetup;
    int iLookAtBlendEndTime;
    float fLookAtAnimBlendRate;
    float fLookAtLimitBlendRate;
};

struct ActorAnimSets
{
    unsigned __int16 aimLow;
    unsigned __int16 aimLevel;
    unsigned __int16 aimHigh;
    unsigned __int16 shootLow;
    unsigned __int16 shootLevel;
    unsigned __int16 shootHigh;
    unsigned __int16 animProneLow;
    unsigned __int16 animProneLevel;
    unsigned __int16 animProneHigh;
};


struct ai_orient_t
{
    ai_orient_mode_t eMode;
    float fDesiredLookPitch;
    float fDesiredLookYaw;
    float fDesiredBodyYaw;
};

struct PhysicsInputs
{
    float vVelocity[3];
    unsigned __int16 groundEntNum;
    int bHasGroundPlane;
    float groundplaneSlope;
    int iFootstepTimer;
};

struct ai_funcs_t
{
    bool( *pfnStart)(actor_s *, ai_state_t);
    void( *pfnFinish)(actor_s *, ai_state_t);
    void( *pfnSuspend)(actor_s *, ai_state_t);
    bool( *pfnResume)(actor_s *, ai_state_t);
    actor_think_result_t( *pfnThink)(actor_s *);
    void( *pfnTouch)(actor_s *, gentity_s *);
    void( *pfnPain)(actor_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t);
};

struct actor_s
{
    gentity_s *ent;
    sentient_s *sentient;
    AISpecies species;
    ai_state_t eState[5];
    ai_substate_t eSubState[5];
    unsigned int stateLevel;
    int iStateTime;
    int preThinkTime;
    ai_transition_cmd_t StateTransitions[11];
    unsigned int transitionCount;
    ai_state_t eSimulatedState[5];
    unsigned int simulatedStateLevel;
    int iPainTime;
    bool allowPain;
    bool allowDeath;
    bool delayedDeath;
    bool provideCoveringFire;
    float accuracy;
    float playerSightAccuracy;
    unsigned int missCount;
    unsigned int hitCount;
    float debugLastAccuracy;
    int lastShotTime;
    unsigned __int16 properName;
    unsigned __int16 weaponName;
    int iTraceCount;
    float fLookPitch;
    float fLookYaw;
    float vLookForward[3];
    float vLookRight[3];
    float vLookUp[3];
    ai_orient_t CodeOrient;
    ai_orient_t ScriptOrient;
    float fDesiredBodyYaw;
    ActorAnimSets animSets;
    unsigned __int16 anim_pose;
    float fInvProneAnimLowPitch;
    float fInvProneAnimHighPitch;
    float fProneLastDiff;
    int bProneOK;
    actor_prone_info_s ProneInfo;
    ActorCachedInfo eyeInfo;
    ActorCachedInfo muzzleInfo;
    ActorLookAtInfo lookAtInfo;
    int iDamageTaken;
    int iDamageYaw;
    float damageDir[3];
    unsigned __int16 damageHitLoc;
    unsigned __int16 damageWeapon;
    ai_stance_e eAllowedStances;
    unsigned __int16 AnimScriptHandle;
    scr_animscript_t *pAnimScriptFunc;
    scr_animscript_t AnimScriptSpecific;
    ai_traverse_mode_t eTraverseMode;
    unsigned __int8 moveMode;
    bool safeToChangeScript;
    bool bUseGoalWeight;
    ai_animmode_t eAnimMode;
    ai_animmode_t eScriptSetAnimMode;
    actor_physics_t Physics;
    path_t Path;
    float fWalkDist;
    path_trim_t TrimInfo;
    int iFollowMin;
    int iFollowMax;
    float fInterval;
    int pathWaitTime;
    int iTeamMoveWaitTime;
    int iTeamMoveDodgeTime;
    actor_s *pPileUpActor;
    gentity_s *pPileUpEnt;
    int bDontAvoidPlayer;
    __int16 chainFallback;
    float sideMove;
    unsigned __int8 keepClaimedNode;
    unsigned __int8 keepClaimedNodeInGoal;
    unsigned __int8 keepNodeDuringScriptedAnim;
    bool noDodgeMove;
    int mayMoveTime;
    float prevMoveDir[2];
    float leanAmount;
    int exposedStartTime;
    int exposedDuration;
    actor_goal_s codeGoal;
    aiGoalSources codeGoalSrc;
    actor_goal_s scriptGoal;
    EntHandle scriptGoalEnt;
    float pathEnemyLookahead;
    float pathEnemyFightDist;
    float meleeAttackDist;
    bool useEnemyGoal;
    bool useMeleeAttackSpot;
    bool goalPosChanged;
    bool commitToFixedNode;
    bool ignoreForFixedNodeSafeCheck;
    bool fixedNode;
    float fixedNodeSafeRadius;
    float fixedNodeSafeVolumeRadiusSq;
    EntHandle fixedNodeSafeVolume;
    pathnode_t *pDesiredChainPos;
    ActorCoverArrivalInfo arrivalInfo;
    int bPacifist;
    int iPacifistWait;
    int numCoverNodesInGoal;
    int iPotentialCoverNodeCount;
    pathnode_t *pPotentialReacquireNode[10];
    int iPotentialReacquireNodeCount;
    float engageMinDist;
    float engageMinFalloffDist;
    float engageMaxDist;
    float engageMaxFalloffDist;
    scr_animscript_t *pAttackScriptFunc;
    float fovDot;
    float fMaxSightDistSqrd;
    int ignoreCloseFoliage;
    sentient_info_t sentientInfo[33];
    SentientHandle pFavoriteEnemy;
    int talkToSpecies;
    float lastEnemySightPos[3];
    bool lastEnemySightPosValid;
    float anglesToLikelyEnemyPath[3];
    int faceLikelyEnemyPathNeedCheckTime;
    int faceLikelyEnemyPathNeedRecalculateTime;
    const pathnode_t *faceLikelyEnemyPathNode;
    ai_suppression_t Suppressant[4];
    int ignoreSuppression;
    int suppressionWait;
    int suppressionDuration;
    int suppressionStartTime;
    float suppressionMeter;
    potential_threat_t potentialThreat;
    int threatUpdateTime;
    int hasThreateningEnemy;
    float grenadeAwareness;
    EntHandle pGrenade;
    int iGrenadeWeaponIndex;
    unsigned __int16 GrenadeTossMethod;
    int bGrenadeTossValid;
    int bGrenadeTargetValid;
    int iGrenadeAmmo;
    float vGrenadeTossPos[3];
    float vGrenadeTargetPos[3];
    float vGrenadeTossVel[3];
    int bDropWeapon;
    int bDrawOnCompass;
    int iUseHintString;
    gentity_s *pTurret;
    unsigned __int16 turretAnim;
    unsigned __int8 turretAnimSet;
    unsigned __int8 useable;
    bool ignoreTriggers;
    bool pushable;
    bool inuse;
    bool isInBadPlace;
    float badPlaceAwareness;
    float goodShootPos[3];
    int goodShootPosValid;
    unsigned __int16 scriptState;
    unsigned __int16 lastScriptState;
    unsigned __int16 stateChangeReason;
    EntHandle pCloseEnt;
    int moveHistoryIndex;
    float moveHistory[10][2];
    int flashBanged;
    float flashBangedStrength;
    int flashBangImmunity;
    const char *pszDebugInfo;
    pathnode_t *pPotentialCoverNode[1000];
};

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
    bool ignoreSuppression);
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
    int bAllowNegotiationLinks);
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

static const float ACTOR_EYE_OFFSET = 64.0f;
static const int ACTOR_MAX_HEALTH = 100;
static const int ACTOR_TEAMMOVE_WAIT_TIME = 500;

static const float meleeAttackOffsets[4][2] = { { 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 } };
static const float g_actorAssumedSpeed[2] = { 190.0, 300.0 };

extern struct AnimScriptList *g_animScriptTable[2];

extern const unsigned __int16 *g_AISpeciesNames[2];
extern const char *g_entinfoAITextNames[6];

// actor_function_table
extern const ai_funcs_t *AIFuncTable[2];