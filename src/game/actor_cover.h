#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl TRACK_actor_cover();
void __cdecl DebugDrawNodeSelectionOverlay();
void __cdecl DebugDrawNodePicking(const char *msg, actor_s *self, const pathnode_t *node, float *color);
int __cdecl Actor_Cover_IsWithinNodeAngle(
    const float *pos,
    const pathnode_t *node,
    const pathnodeRange_t *range,
    double a4,
    double a5,
    long double a6);
int __cdecl Actor_Cover_NodeRangeValid(const float *pos, const pathnode_t *node, pathnodeRange_t *range);
void __cdecl Actor_Cover_InitRange(pathnodeRange_t *rangeOut, const pathnode_t *node);
bool __cdecl Actor_Cover_GetAttackScript(
    actor_s *self,
    const pathnode_t *node,
    scr_animscript_t **pAttackScriptFunc);
bool __cdecl Actor_Cover_CheckWithEnemy(actor_s *self, const pathnode_t *node, bool checkEnemyRange);
bool __cdecl Actor_Cover_PickAttackScript(
    actor_s *self,
    const pathnode_t *node,
    bool checkEnemyRange,
    scr_animscript_t **pAttackScriptFunc);
float __cdecl Actor_Cover_ScoreOnDistance(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnEngagement(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnNodeAngle(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnTargetDir(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnPriority(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnPlayerLOS(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnVisibility(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_ScoreOnCoverType(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_GetNodeDistMetric(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_GetNodeMetric(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_FromPoint_GetNodeMetric(actor_s *self, const pathnode_t *node);
int __cdecl Actor_Cover_IsValidReacquire(actor_s *self, const pathnode_t *node);
int __cdecl Actor_Cover_IsValidCoverDir(actor_s *self, const pathnode_t *node);
int __cdecl Actor_Cover_IsValidCover(actor_s *self, const pathnode_t *node);
float __cdecl Actor_Cover_MinHeightAtCover(pathnode_t *node);
pathnode_t *__cdecl Actor_Cover_FindCoverFromPoint(actor_s *self, const float *vPoint, double fMinSafeDist);
int __cdecl isNodeInRegion(pathnode_t *node, gentity_s *volume);
int __cdecl Actor_Cover_FindBestCoverListInList(actor_s *self, pathsort_t *nodes, int iNodeCount, gentity_s *volume);
int __cdecl compare_node_sort(float *pe1, float *pe2);
int __cdecl Actor_Cover_FindBestCoverList(actor_s *self, pathnode_t **bestNodes, int bestNodesInList);
void __cdecl Actor_Cover_FindCoverNode(actor_s *self);
pathnode_t *__cdecl Actor_Cover_GetCoverNode(actor_s *self);
int __cdecl Actor_Cover_UseCoverNode(actor_s *self, pathnode_t *node);
void __cdecl Actor_DebugDrawNodesInVolume(actor_s *self);
pathnode_t *__cdecl Actor_Cover_FindBestCover(actor_s *self);
