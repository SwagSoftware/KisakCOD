#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __fastcall Path_IsPathStanceNode(const pathnode_t *node);
float __fastcall Path_GetPathDir(float *delta, const float *vFrom, const float *vTo);
pathnode_t *__fastcall Path_GetNegotiationNode(const path_t *pPath);
void __fastcall Path_IncrementNodeUserCount(path_t *pPath);
void __fastcall Path_DecrementNodeUserCount(path_t *pPath);
void __fastcall Path_Backup(const path_t *path);
float __fastcall Path_GetDistToPathSegment(const float *vStartPos, const pathpoint_t *pt);
void __fastcall Path_AddTrimmedAmount(path_t *pPath, const float *vStartPos);
void __fastcall Path_SubtractTrimmedAmount(path_t *pPath, const float *vStartPos);
void __fastcall Path_BeginTrim(path_t *pPath, path_trim_t *pTrim);
void __fastcall Path_Begin(path_t *pPath);
void __fastcall Path_Clear(path_t *pPath);
bool __fastcall Path_Exists(const path_t *pPath);
int __fastcall Path_CompleteLookahead(const path_t *pPath);
unsigned int __fastcall Path_AttemptedCompleteLookahead(const path_t *pPath);
bool __fastcall Path_UsesObstacleNegotiation(const path_t *pPath);
bool __fastcall Path_HasNegotiationNode(const path_t *path);
unsigned int __fastcall Path_AllowsObstacleNegotiation(const path_t *pPath);
void __fastcall Path_GetObstacleNegotiationScript(const path_t *pPath, scr_animscript_t *animscript);
int __fastcall Path_NeedsReevaluation(const path_t *pPath);
int __fastcall Path_EncroachesPoint2D(path_t *pPath, const float *vStart, const float *vPoint, double fMinDistSqrd);
int __fastcall Path_DistanceGreaterThan(path_t *pPath, double fDist);
void __fastcall Path_ReduceLookaheadAmount(path_t *pPath, double maxLookaheadAmountIfReduce);
bool __fastcall Path_FailedLookahead(path_t *pPath);
void __fastcall Path_IncreaseLookaheadAmount(path_t *pPath);
bool __fastcall Path_PredictionTrace(
    float *vStartPos,
    float *vEndPos,
    int entityIgnore,
    int mask,
    float *vTraceEndPos,
    double stepheight,
    int allowStartSolid,
    int a8);
int __fastcall Path_IsTrimmed(path_t *pPath);
void __fastcall Path_RemoveCompletedPathPoints(path_t *pPath, __int16 pathPointIndex);
void __fastcall Path_TrimCompletedPath(path_t *pPath, const float *vStartPos);
void __fastcall Path_BacktrackCompletedPath(path_t *pPath, const float *vStartPos);
void __fastcall PathCalcLookahead_CheckMinLookaheadNodes(path_t *pPath, const pathpoint_t *pt, int currentNode);
int __fastcall Path_GetForwardStartPos(path_t *pPath, const float *vStartPos, float *vForwardStartPos);
void __fastcall Path_UpdateForwardLookahead_IncompletePath(
    path_t *pPath,
    const pathpoint_t *pt,
    const float *vForwardStartPos,
    double area,
    double height);
void __fastcall Path_UpdateForwardLookahead(path_t *pPath, const float *vStartPos);
void __fastcall Path_DebugDraw(path_t *pPath, float *vStartPos, int bDrawLookahead);
bool __fastcall Path_WithinApproxDist(path_t *pPath, double checkDist);
ai_stance_e __fastcall Path_AllowedStancesForPath(path_t *pPath);
void __fastcall Path_DodgeDrawRaisedLine(float *start, float *end, const float *color);
int __fastcall Path_MayFaceEnemy(path_t *pPath, float *vEnemyDir, float *vOrg);
void __fastcall Path_Restore(path_t *path);
int __fastcall Path_FindPathFromTo(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
void __fastcall Path_TrimLastNodes(path_t *pPath, const int iNodeCount, bool bMaintainGoalPos);
int __fastcall Path_ClipToGoal(path_t *pPath, const actor_goal_s *goal);
int __fastcall Path_TrimToSeePoint(
    path_t *pPath,
    path_trim_t *pTrim,
    actor_s *pActor,
    double fMaxDistSqrd,
    int iIgnoreEntityNum,
    const float *vPoint,
    const float *a7);
PredictionTraceResult __fastcall Path_PredictionTraceCheckForEntities(
    float *vStartPos,
    float *vEndPos,
    int *entities,
    int entityCount,
    int entityIgnore,
    int mask,
    float *vTraceEndPos);
bool __fastcall Path_LookaheadPredictionTrace(path_t *pPath, float *vStartPos, float *vEndPos, int a4, int a5, int a6);
void __fastcall Path_UpdateLookaheadAmount(
    path_t *pPath,
    float *vStartPos,
    float *vLookaheadPos,
    int bReduceLookaheadAmount,
    double dist,
    int lookaheadNextNode,
    double maxLookaheadAmountIfReduce,
    __int16 a8);
void __fastcall Path_CalcLookahead_Completed(
    path_t *pPath,
    float *vStartPos,
    int bReduceLookaheadAmount,
    double totalArea,
    int a5,
    int a6);
void __fastcall Path_CalcLookahead(path_t *pPath, float *vStartPos, int bReduceLookaheadAmount, int a4, int a5);
void __fastcall Path_CheckNodeCountForDodge(path_t *pPath, int numNeeded, pathpoint_t **pt, int *startIndex);
void __fastcall Path_TrimToBadPlaceLink(path_t *pPath, team_t eTeam);
int __fastcall Path_FindPath(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    int bAllowNegotiationLinks);
pathnode_t *__fastcall Path_FindPathFrom(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
void __fastcall Path_UpdateLookahead(
    path_t *pPath,
    float *vStartPos,
    int bReduceLookaheadAmount,
    int bTrimAmount,
    int bAllowBacktrack);
void __fastcall Path_SetLookaheadToStart(path_t *pPath, float *vStartPos, int bTrimAmount);
void __fastcall Path_TransferLookahead(path_t *pPath, float *vStartPos);
int __fastcall Path_GeneratePath(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vGoalPos,
    pathnode_t *pNodeFrom,
    pathnode_t *pNodeTo,
    int bIncludeGoalPos,
    int bAllowNegotiationLinks);
void __fastcall Path_UpdateLookahead_NonCodeMove(path_t *pPath, const float *vPrevPos, float *vStartPos);
int __fastcall Path_AttemptDodge(
    path_t *pPath,
    float *vOrg,
    float *vDodgeStart,
    float *vDodgeEnd,
    int startIndex,
    int *entities,
    int entityCount,
    int entityIgnore,
    int mask,
    int bCheckLookahead,
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
    int a29,
    int a30);
pathnode_t *__fastcall Path_FindCloseNode(
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
int __fastcall Path_FindPathFromToWithWidth(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks,
    double width,
    float *perp,
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
    float *a29);
int __fastcall Path_FindPathFromToNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    int bAllowNegotiationLinks,
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
    int a29,
    int a30);
int __fastcall Path_FindPathFromAway(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    int bAllowNegotiationLinks,
    int a8);
int __fastcall Path_FindPathFromAwayNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int bAllowNegotiationLinks,
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
    int a29,
    int a30,
    int a31);
int __fastcall Path_FindPathInCylinderWithLOS(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    double fWithinDistSqrd,
    int bAllowNegotiationLinks,
    int a8);
int __fastcall Path_FindPathInCylinderWithLOSNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    double fWithinDistSqrd,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int bAllowNegotiationLinks,
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
    int *a29,
    int a30,
    int a31);
pathnode_t *__fastcall Path_FindPathFromInCylinder(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    const float *vGoalPos,
    float *vOrigin,
    double fRadiusSqrd,
    double fHalfHeightSqrd,
    int bAllowNegotiationLinks,
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
    int a29,
    int a30);
int __fastcall Path_FindPathFromInCylinderNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    const float *vGoalPos,
    float *vOrigin,
    double fRadiusSqrd,
    double fHalfHeightSqrd,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    int bAllowNegotiationLinks,
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
    int a29,
    float *a30,
    int a31,
    float *a32,
    int a33,
    int *a34,
    int a35,
    int a36);
const pathnode_t *__fastcall Path_FindFacingNode(sentient_s *pSelf, sentient_s *pOther, sentient_info_t *pInfo);
int __fastcall Path_FindPathGetCloseAsPossible(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
int __fastcall Path_FindPathWithWidth(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    int bAllowNegotiationLinks,
    double width,
    float *perp,
    float *a8);
int __fastcall Path_FindPathNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int *iPlaneCount,
    int bAllowNegotiationLinks);
pathnode_t *__fastcall Path_FindPathFromNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    const float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int *iPlaneCount,
    int bAllowNegotiationLinks,
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
    int a28);
pathnode_t *__fastcall Path_FindPathAway(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    int bAllowNegotiationLinks,
    int a7);
pathnode_t *__fastcall Path_FindPathAwayNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int *bAllowNegotiationLinks,
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
