#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __cdecl Path_IsPathStanceNode(const pathnode_t *node);
float __cdecl Path_GetPathDir(float *delta, const float *vFrom, const float *vTo);
pathnode_t *__cdecl Path_GetNegotiationNode(const path_t *pPath);
void __cdecl Path_IncrementNodeUserCount(path_t *pPath);
void __cdecl Path_DecrementNodeUserCount(path_t *pPath);
void __cdecl Path_Backup(const path_t *path);
float __cdecl Path_GetDistToPathSegment(const float *vStartPos, const pathpoint_t *pt);
void __cdecl Path_AddTrimmedAmount(path_t *pPath, const float *vStartPos);
void __cdecl Path_SubtractTrimmedAmount(path_t *pPath, const float *vStartPos);
void __cdecl Path_BeginTrim(path_t *pPath, path_trim_t *pTrim);
void __cdecl Path_Begin(path_t *pPath);
void __cdecl Path_Clear(path_t *pPath);
bool __cdecl Path_Exists(const path_t *pPath);
int __cdecl Path_CompleteLookahead(const path_t *pPath);
unsigned int __cdecl Path_AttemptedCompleteLookahead(const path_t *pPath);
bool __cdecl Path_UsesObstacleNegotiation(const path_t *pPath);
bool __cdecl Path_HasNegotiationNode(const path_t *path);
unsigned int __cdecl Path_AllowsObstacleNegotiation(const path_t *pPath);
void __cdecl Path_GetObstacleNegotiationScript(const path_t *pPath, scr_animscript_t *animscript);
int __cdecl Path_NeedsReevaluation(const path_t *pPath);
int __cdecl Path_EncroachesPoint2D(path_t *pPath, const float *vStart, const float *vPoint, double fMinDistSqrd);
int __cdecl Path_DistanceGreaterThan(path_t *pPath, double fDist);
void __cdecl Path_ReduceLookaheadAmount(path_t *pPath, double maxLookaheadAmountIfReduce);
bool __cdecl Path_FailedLookahead(path_t *pPath);
void __cdecl Path_IncreaseLookaheadAmount(path_t *pPath);
bool __cdecl Path_PredictionTrace(
    float *vStartPos,
    float *vEndPos,
    int entityIgnore,
    int mask,
    float *vTraceEndPos,
    double stepheight,
    int allowStartSolid,
    int a8);
int __cdecl Path_IsTrimmed(path_t *pPath);
void __cdecl Path_RemoveCompletedPathPoints(path_t *pPath, __int16 pathPointIndex);
void __cdecl Path_TrimCompletedPath(path_t *pPath, const float *vStartPos);
void __cdecl Path_BacktrackCompletedPath(path_t *pPath, const float *vStartPos);
void __cdecl PathCalcLookahead_CheckMinLookaheadNodes(path_t *pPath, const pathpoint_t *pt, int currentNode);
int __cdecl Path_GetForwardStartPos(path_t *pPath, const float *vStartPos, float *vForwardStartPos);
void __cdecl Path_UpdateForwardLookahead_IncompletePath(
    path_t *pPath,
    const pathpoint_t *pt,
    const float *vForwardStartPos,
    double area,
    double height);
void __cdecl Path_UpdateForwardLookahead(path_t *pPath, const float *vStartPos);
void __cdecl Path_DebugDraw(path_t *pPath, float *vStartPos, int bDrawLookahead);
bool __cdecl Path_WithinApproxDist(path_t *pPath, double checkDist);
ai_stance_e __cdecl Path_AllowedStancesForPath(path_t *pPath);
void __cdecl Path_DodgeDrawRaisedLine(float *start, float *end, const float *color);
int __cdecl Path_MayFaceEnemy(path_t *pPath, float *vEnemyDir, float *vOrg);
void __cdecl Path_Restore(path_t *path);
int __cdecl Path_FindPathFromTo(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
void __cdecl Path_TrimLastNodes(path_t *pPath, const int iNodeCount, bool bMaintainGoalPos);
int __cdecl Path_ClipToGoal(path_t *pPath, const actor_goal_s *goal);
int __cdecl Path_TrimToSeePoint(
    path_t *pPath,
    path_trim_t *pTrim,
    actor_s *pActor,
    double fMaxDistSqrd,
    int iIgnoreEntityNum,
    const float *vPoint,
    const float *a7);
PredictionTraceResult __cdecl Path_PredictionTraceCheckForEntities(
    float *vStartPos,
    float *vEndPos,
    int *entities,
    int entityCount,
    int entityIgnore,
    int mask,
    float *vTraceEndPos);
bool __cdecl Path_LookaheadPredictionTrace(path_t *pPath, float *vStartPos, float *vEndPos, int a4, int a5, int a6);
void __cdecl Path_UpdateLookaheadAmount(
    path_t *pPath,
    float *vStartPos,
    float *vLookaheadPos,
    int bReduceLookaheadAmount,
    double dist,
    int lookaheadNextNode,
    double maxLookaheadAmountIfReduce,
    __int16 a8);
void __cdecl Path_CalcLookahead_Completed(
    path_t *pPath,
    float *vStartPos,
    int bReduceLookaheadAmount,
    double totalArea,
    int a5,
    int a6);
void __cdecl Path_CalcLookahead(path_t *pPath, float *vStartPos, int bReduceLookaheadAmount, int a4, int a5);
void __cdecl Path_CheckNodeCountForDodge(path_t *pPath, int numNeeded, pathpoint_t **pt, int *startIndex);
void __cdecl Path_TrimToBadPlaceLink(path_t *pPath, team_t eTeam);
int __cdecl Path_FindPath(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    int bAllowNegotiationLinks);
pathnode_t *__cdecl Path_FindPathFrom(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
void __cdecl Path_UpdateLookahead(
    path_t *pPath,
    float *vStartPos,
    int bReduceLookaheadAmount,
    int bTrimAmount,
    int bAllowBacktrack);
void __cdecl Path_SetLookaheadToStart(path_t *pPath, float *vStartPos, int bTrimAmount);
void __cdecl Path_TransferLookahead(path_t *pPath, float *vStartPos);
int __cdecl Path_GeneratePath(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vGoalPos,
    pathnode_t *pNodeFrom,
    pathnode_t *pNodeTo,
    int bIncludeGoalPos,
    int bAllowNegotiationLinks);
void __cdecl Path_UpdateLookahead_NonCodeMove(path_t *pPath, const float *vPrevPos, float *vStartPos);
int __cdecl Path_AttemptDodge(
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
pathnode_t *__cdecl Path_FindCloseNode(
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
int __cdecl Path_FindPathFromToWithWidth(
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
int __cdecl Path_FindPathFromToNotCrossPlanes(
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
int __cdecl Path_FindPathFromAway(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    int bAllowNegotiationLinks,
    int a8);
int __cdecl Path_FindPathFromAwayNotCrossPlanes(
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
int __cdecl Path_FindPathInCylinderWithLOS(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    double fWithinDistSqrd,
    int bAllowNegotiationLinks,
    int a8);
int __cdecl Path_FindPathInCylinderWithLOSNotCrossPlanes(
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
pathnode_t *__cdecl Path_FindPathFromInCylinder(
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
int __cdecl Path_FindPathFromInCylinderNotCrossPlanes(
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
const pathnode_t *__cdecl Path_FindFacingNode(sentient_s *pSelf, sentient_s *pOther, sentient_info_t *pInfo);
int __cdecl Path_FindPathGetCloseAsPossible(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks);
int __cdecl Path_FindPathWithWidth(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    int bAllowNegotiationLinks,
    double width,
    float *perp,
    float *a8);
int __cdecl Path_FindPathNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int *iPlaneCount,
    int bAllowNegotiationLinks);
pathnode_t *__cdecl Path_FindPathFromNotCrossPlanes(
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
pathnode_t *__cdecl Path_FindPathAway(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    int bAllowNegotiationLinks,
    int a7);
pathnode_t *__cdecl Path_FindPathAwayNotCrossPlanes(
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
