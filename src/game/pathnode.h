#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_pathnode();
int __fastcall NodeTypeCanHavePriority(nodeType type);
void __fastcall TurretNode_GetAngles(const pathnode_t *node, float *angleMin, float *angleMax);
bool __fastcall TurretNode_HasTurret(const pathnode_t *node);
void __fastcall Path_ReadOnly(int offset);
void __fastcall Path_NonNegativeFloat(pathnode_t *node, int offset);
void __fastcall Path_GetType(pathnode_t *node, int offset);
void __fastcall Scr_SetPathnodeField(unsigned int entnum, unsigned int offset);
void __fastcall Scr_GetPathnodeField(unsigned int entnum, unsigned int offset);
void __fastcall PathNode_ClearStringField(unsigned __int16 *destScrString);
void __fastcall PathNode_UpdateStringField(
    const char *destKey,
    unsigned __int16 *destScrString,
    const char *key,
    const char *value);
void __fastcall PathNode_UpdateFloatField(const char *destKey, float *destFloat, const char *key, const char *value);
void __fastcall PathNode_OriginMatches(const char *value, const float *nodeOrigin);
void __fastcall node_droptofloor(pathnode_t *node);
void __fastcall G_UpdateTrackExtraNodes();
void __fastcall GScr_AddFieldsForPathnode();
pathnode_t *__fastcall Scr_GetPathnode(scr_entref_t *index, unsigned int a2);
void __fastcall G_FreePathnodesScriptInfo();
bool __fastcall Path_CompareNodesIncreasing(const pathsort_t *ps1, const pathsort_t *ps2);
unsigned int __fastcall Path_ConvertNodeToIndex(const pathnode_t *node);
pathnode_t *__fastcall Path_ConvertIndexToNode(unsigned int index);
unsigned int __fastcall Path_NodeCount();
void __fastcall Path_Init(int restart);
int __fastcall NodeVisCacheEntry(int i, int j);
int __fastcall ExpandedNodeVisCacheEntry(int i, int j);
void __fastcall Path_NodesInCylinder_r(pathnode_tree_t *tree);
pathnode_tree_t *__fastcall Path_NodesInCylinder(
    float *origin,
    double maxDist,
    double maxHeight,
    pathsort_t *nodes,
    int maxNodes,
    pathsort_t *typeFlags,
    int a7,
    int a8);
pathnode_tree_t *__fastcall Path_NodesInRadius(
    float *origin,
    double maxDist,
    pathsort_t *nodes,
    pathsort_t *maxNodes,
    int typeFlags,
    int a6);
int __fastcall Path_IsDynamicBlockingEntity(gentity_s *ent);
bool __fastcall Path_IsBadPlaceLink(unsigned int nodeNumFrom, unsigned int nodeNumTo, team_t eTeam);
unsigned int Path_InitLinkCounts();
void Path_InitLinkInfoArray();
void __fastcall Path_InitNodeDynamic(pathnode_t *loadNode);
void __fastcall Path_InitNodesDynamic(float *a1);
void __fastcall Path_PreSpawnInitPaths(float *a1);
void __fastcall Path_DrawDebugNoLinks(const pathnode_t *node, const float (*color)[4], int duration);
void __fastcall Path_DrawDebugLink(const pathnode_t *node, const int i, bool bShowAll);
float __fastcall Path_GetDebugStringScale(const float *cameraPos, const float *origin);
void __fastcall Path_DrawDebugNodeBox(const pathnode_t *node, int a2, int a3, const float *a4);
void __fastcall Path_DrawDebugNode(const float *cameraPos, const pathnode_t *node, int a3, const float *a4);
void __fastcall Path_DrawDebugFindPath(float *vOrigin);
void __fastcall Path_DrawFriendlyChain();
bool __fastcall Path_IsNodeIndex(const pathnode_t *node, unsigned int nodeIndexToCheck);
int __fastcall Path_NodesVisible(const pathnode_t *node0, const pathnode_t *node1);
int __fastcall Path_ExpandedNodeVisible(const pathnode_t *node0, const pathnode_t *node1);
pathnode_t *__fastcall Path_FindChainPos(const float *vOrigin, pathnode_t *pPrevChainPos);
void __fastcall Path_UpdateBestChainNode(pathnode_t *node, pathnode_t **bestNode, unsigned int *foundCount);
int __fastcall Path_CanSetDesiredChainPos(actor_s *claimer, const pathnode_t *node);
void __fastcall Path_AttachSentientToChainNode(sentient_s *sentient, unsigned __int16 targetname);
pathnode_t *__fastcall Path_FirstNode(int typeFlags);
pathnode_t *__fastcall Path_NextNode(pathnode_t *prevNode, int typeFlags);
sentient_s *__fastcall Path_GetNodeOwner(const pathnode_t *node);
int __fastcall Path_CanStealPriorityNode(const pathnode_t *node, sentient_s *claimer);
int __fastcall Path_CanStealNode(const pathnode_t *node, sentient_s *claimer);
void __fastcall Path_ClaimNodeInternal(pathnode_t *node, sentient_s *claimer);
void __fastcall Path_MarkNodeOverlap(pathnode_t *node);
void __fastcall Path_ClaimNode(pathnode_t *node, sentient_s *claimer);
void __fastcall Path_RevokeClaim(pathnode_t *node, sentient_s *pNewClaimer);
void __fastcall Path_RelinquishNode(sentient_s *claimer, int timeUntilRelinquished);
int __fastcall Path_AllowedStancesForNode(pathnode_t *node);
int __fastcall Path_SaveIndex(const pathnode_t *node);
pathnode_t *__fastcall Path_LoadNode(unsigned int index);
void __fastcall Path_ValidateNode(pathnode_t *node);
void __fastcall Path_ValidateAllNodes();
void __fastcall Path_CheckLinkLeaks();
void __fastcall Path_CheckUserCountLeaks();
void __fastcall Path_DisconnectPath(pathnode_t *node, pathlink_s *link);
void __fastcall Path_ConnectPath(pathnode_t *node, pathlink_s *link);
void __fastcall Path_ConnectPathsForEntity(gentity_s *ent);
void __fastcall Path_DisconnectPathsForEntity(gentity_s *ent);
void __fastcall Path_UpdateBadPlaceCountForLink(pathlink_s *link, int teamflags, int delta);
void __fastcall Path_UpdateArcBadPlaceCount(badplace_arc_t *arc, int teamflags, int delta);
void __fastcall Path_UpdateBrushBadPlaceCount(gentity_s *brushEnt, int teamflags, int delta);
int __fastcall Path_IsNodeInArc(
    pathnode_t *pNode,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight);
void __fastcall WriteEntityDisconnectedLinks(gentity_s *ent, SaveGame *save);
void __fastcall ReadEntityDisconnectedLinks(gentity_s *ent, SaveGame *save);
void __fastcall Scr_SetNodePriority();
void __fastcall Scr_IsNodeOccupied();
void __fastcall Scr_SetTurretNode();
void __fastcall Scr_UnsetTurretNode();
void __fastcall GScr_SetDynamicPathnodeField(pathnode_t *node, unsigned int index);
void __fastcall G_DropPathnodesToFloor();
void __fastcall Scr_FreePathnode(pathnode_t *node);
void __fastcall Scr_AddPathnode(pathnode_t *node);
void __fastcall Scr_GetNode();
void __fastcall Scr_GetNodeArray();
void __fastcall Scr_GetAllNodes();
void __fastcall Path_Shutdown();
void __fastcall Path_AutoDisconnectPaths();
void __fastcall Path_InitPaths();
void __fastcall Path_DrawVisData();
void __fastcall Path_RelinquishNodeNow(sentient_s *claimer);
void __fastcall Path_RelinquishNodeSoon(sentient_s *claimer);
void __fastcall Path_MarkNodeInvalid(pathnode_t *node, team_t eTeam);
void __fastcall G_SetPathnodeScriptVariable(const char *key, const char *value, pathnode_t *ent);
void __fastcall G_ParsePathnodeScriptField(const char *key, const char *value, pathnode_t *node);
void __fastcall G_ParsePathnodeScriptFields(pathnode_t *node);
void __fastcall G_SpawnPathnodeDynamic();
int __fastcall Path_CanClaimNode(const pathnode_t *node, sentient_s *claimer);
void __fastcall Path_ForceClaimNode(pathnode_t *node, sentient_s *claimer);
pathnode_t *__fastcall Path_ChooseSubsequentChainNode_r(
    int depthMin,
    int depthMax,
    pathnode_t *pParent,
    actor_s *claimer);
pathnode_t *__fastcall Path_ChooseAnyChainNodeIfDeadEnd(
    int depthMin,
    int depthMax,
    pathnode_t *chainPos,
    actor_s *claimer);
pathnode_t *__fastcall Path_ChoosePreviousChainNode(int depthMin, int depthMax, pathnode_t *chainPos, actor_s *claimer);
pathnode_t *__fastcall Path_ChooseDesperationChainNode(
    int depthMin,
    int depthMax,
    pathnode_t *refPos,
    actor_s *claimer);
pathnode_t *__fastcall Path_ChooseDesperationNewChainNode(
    int depthMin,
    int depthMax,
    pathnode_t *refPos,
    actor_s *claimer);
pathnode_t *__fastcall Path_ChooseChainPos(
    pathnode_t *refPos,
    int iFollowMin,
    int iFollowMax,
    actor_s *claimer,
    int chainFallback);
pathnode_t *__fastcall Path_NearestNodeNotCrossPlanes(
    float *vOrigin,
    pathsort_t *nodes,
    int typeFlags,
    double fMaxDist,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int *returnCount,
    unsigned int *maxNodes,
    nearestNodeHeightCheck heightCheck,
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
pathnode_t *__fastcall Path_NearestNode(
    float *vOrigin,
    pathsort_t *nodes,
    int typeFlags,
    double fMaxDist,
    float *returnCount,
    unsigned int *maxNodes,
    int heightCheck,
    int a8);
void __fastcall Path_DrawDebugNearestNode(float *vOrigin, int numNodes);
void __fastcall Path_DrawDebugClaimedNodes(float *origin, int numNodes);
void __fastcall Path_DrawDebug();
