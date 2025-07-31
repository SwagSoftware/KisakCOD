#pragma once
#include <script/scr_variable.h>
#include <bgame/bg_public.h>
#include <bgame/bg_local.h>

#define PNF_PRIORITY 0x40

#define MAX_NODES_IN_BRUSH 0x200

enum nearestNodeHeightCheck : __int32
{
    NEAREST_NODE_DO_HEIGHT_CHECK = 0x0,
    NEAREST_NODE_DONT_DO_HEIGHT_CHECK = 0x1,
};

enum nodeType : __int32
{                                       // ...
    NODE_BADNODE = 0x0,
    NODE_PATHNODE = 0x1,
    NODE_COVER_STAND = 0x2,
    NODE_COVER_CROUCH = 0x3,
    NODE_COVER_CROUCH_WINDOW = 0x4,
    NODE_COVER_PRONE = 0x5,
    NODE_COVER_RIGHT = 0x6,
    NODE_COVER_LEFT = 0x7,
    NODE_COVER_WIDE_RIGHT = 0x8,
    NODE_COVER_WIDE_LEFT = 0x9,
    NODE_CONCEALMENT_STAND = 0xA,
    NODE_CONCEALMENT_CROUCH = 0xB,
    NODE_CONCEALMENT_PRONE = 0xC,
    NODE_REACQUIRE = 0xD,
    NODE_BALCONY = 0xE,
    NODE_SCRIPTED = 0xF,
    NODE_NEGOTIATION_BEGIN = 0x10,
    NODE_NEGOTIATION_END = 0x11,
    NODE_TURRET = 0x12,
    NODE_GUARD = 0x13,
    NODE_NUMTYPES = 0x14,
    NODE_DONTLINK = 0x14,
};




#ifdef KISAK_SP
struct NodeTypeToName
{
    nodeType type;
    const char *name;
};

struct PathLinkInfo
{
    unsigned __int16 from;
    unsigned __int16 to;
    unsigned __int16 prev;
    unsigned __int16 next;
};
struct pathlocal_t_tag
{
    float origin[3];
    float maxDist;
    float maxDistSq;
    float maxHeightSq;
    int typeFlags;
    pathsort_t *nodes;
    int maxNodes;
    int nodeCount;
};
struct __declspec(align(128)) pathlocal_t
{
    PathLinkInfo pathLinkInfoArray[2048];
    int pathLinkInfoArrayInited;
    unsigned int actualNodeCount;
    unsigned int extraNodes;
    unsigned int originErrors;
    pathlocal_t_tag circle;
};
struct pathnodeRange_t
{
    float minSqDist;
    float fAngleMin;
    float fAngleMax;
};
struct pathsort_t
{
    pathnode_t *node;
    float metric;
    float distMetric;
};
#endif

struct pathlink_s // sizeof=0xC
{
    float fDist;
    unsigned __int16 nodeNum;
    unsigned __int8 disconnectCount;
    unsigned __int8 negotiationLink;
    unsigned __int8 ubBadPlaceCount[4];
};
static_assert(sizeof(pathlink_s) == 12);

struct pathnode_constant_t // sizeof=0x44
{                                       // ...
    nodeType type;
    unsigned __int16 spawnflags;
    unsigned __int16 targetname;
    unsigned __int16 script_linkName;
    unsigned __int16 script_noteworthy;
    unsigned __int16 target;
    unsigned __int16 animscript;
    int animscriptfunc;
    float vOrigin[3];
    float fAngle;
    float forward[2];
    float fRadius;
    float minUseDistSq;
    __int16 wOverlapNode[2];
    __int16 wChainId;
    __int16 wChainDepth;
    __int16 wChainParent;
    unsigned __int16 totalLinkCount;
    pathlink_s *Links;
};

#ifdef KISAK_MP
struct pathnode_dynamic_t // sizeof=0x20
{                                       // ...
    void *pOwner;
    int iFreeTime;
    int iValidTime[3];
    int inPlayerLOSTime;
    __int16 wLinkCount;
    __int16 wOverlapCount;
    __int16 turretEntNumber;
    __int16 userCount;
};
#elif KISAK_SP
struct pathnode_dynamic_t
{
    SentientHandle pOwner;
    int iFreeTime;
    int iValidTime[3];
    int inPlayerLOSTime;
    __int16 wLinkCount;
    __int16 wOverlapCount;
    __int16 turretEntNumber;
    __int16 userCount;
};
#endif


struct pathnode_t;
struct pathnode_transient_t // sizeof=0x1C
{
    int iSearchFrame;
    pathnode_t *pNextOpen;
    pathnode_t *pPrevOpen;
    pathnode_t *pParent;
    float fCost;
    float fHeuristic;
    float costFactor;
};

struct pathnode_t // sizeof=0x80 (SP/MP Same)
{
    pathnode_constant_t constant;
    pathnode_dynamic_t dynamic;
    pathnode_transient_t transient;
};


struct pathbasenode_t // sizeof=0x10
{
    float vOrigin[3];
    unsigned int type;
};
static_assert(sizeof(pathbasenode_t) == 16);

struct pathnode_tree_nodes_t // sizeof=0x8
{                                       // ...
    int nodeCount;
    unsigned __int16 *nodes;
};
static_assert(sizeof(pathnode_tree_nodes_t) == 8);

struct pathnode_tree_t;
union pathnode_tree_info_t // sizeof=0x8
{
    pathnode_tree_t *child[2];
    pathnode_tree_nodes_t s;
};

struct pathnode_tree_t // sizeof=0x10
{
    int axis;
    float dist;
    pathnode_tree_info_t u;
};
struct PathData // sizeof=0x28
{                                       // ...
    unsigned int nodeCount;
    pathnode_t *nodes;
    pathbasenode_t *basenodes;
    unsigned int chainNodeCount;
    unsigned __int16 *chainNodeForNode;
    unsigned __int16 *nodeForChainNode;
    int visBytes;
    unsigned __int8 *pathVis;
    int nodeTreeCount;
    pathnode_tree_t *nodeTree;
};


#ifndef KISAK_MP

#include "actor_badplace.h"
#include <client/client.h>

struct actor_s;


void __cdecl TRACK_pathnode();
int __cdecl NodeTypeCanHavePriority(nodeType type);
void __cdecl TurretNode_GetAngles(const pathnode_t *node, float *angleMin, float *angleMax);
bool __cdecl TurretNode_HasTurret(const pathnode_t *node);
void __cdecl Path_ReadOnly(int offset);
void __cdecl Path_NonNegativeFloat(pathnode_t *node, int offset);
void __cdecl Path_GetType(pathnode_t *node, int offset);
void __cdecl Scr_SetPathnodeField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_GetPathnodeField(unsigned int entnum, unsigned int offset);
void __cdecl PathNode_ClearStringField(unsigned __int16 *destScrString);
void __cdecl PathNode_UpdateStringField(
    const char *destKey,
    unsigned __int16 *destScrString,
    const char *key,
    const char *value);
void __cdecl PathNode_UpdateFloatField(const char *destKey, float *destFloat, const char *key, const char *value);
void __cdecl PathNode_OriginMatches(const char *value, const float *nodeOrigin);
void __cdecl node_droptofloor(pathnode_t *node);
void __cdecl G_UpdateTrackExtraNodes();
void __cdecl GScr_AddFieldsForPathnode();
pathnode_t *__cdecl Scr_GetPathnode(unsigned int index);
void __cdecl G_FreePathnodesScriptInfo();
bool __cdecl Path_CompareNodesIncreasing(const pathsort_t *ps1, const pathsort_t *ps2);
unsigned int __cdecl Path_ConvertNodeToIndex(const pathnode_t *node);
pathnode_t *__cdecl Path_ConvertIndexToNode(unsigned int index);
unsigned int __cdecl Path_NodeCount();
void __cdecl Path_Init(int restart);
int __cdecl NodeVisCacheEntry(int i, int j);
int __cdecl ExpandedNodeVisCacheEntry(int i, int j);
void __cdecl Path_NodesInCylinder_r(pathnode_tree_t *tree);

// returns nodecount
int __cdecl Path_NodesInCylinder(
    float *origin,
    double maxDist,
    double maxHeight,
    pathsort_t *nodes,
    int maxNodes,
    int typeFlags);
int __cdecl Path_NodesInRadius(
    float *origin,
    double maxDist,
    pathsort_t *nodes,
    int maxNodes,
    int typeFlags);
int __cdecl Path_IsDynamicBlockingEntity(gentity_s *ent);
bool __cdecl Path_IsBadPlaceLink(unsigned int nodeNumFrom, unsigned int nodeNumTo, team_t eTeam);
unsigned int Path_InitLinkCounts();
void Path_InitLinkInfoArray();
void __cdecl Path_InitNodeDynamic(pathnode_t *loadNode);
void __cdecl Path_InitNodesDynamic();
void __cdecl Path_PreSpawnInitPaths();
void __cdecl Path_DrawDebugNoLinks(const pathnode_t *node, const float (*color)[4], int duration);
void __cdecl Path_DrawDebugLink(const pathnode_t *node, const int i, bool bShowAll);
float __cdecl Path_GetDebugStringScale(const float *cameraPos, const float *origin);
void __cdecl Path_DrawDebugNodeBox(const pathnode_t *node);
void __cdecl Path_DrawDebugNode(const float *cameraPos, const pathnode_t *node);
void __cdecl Path_DrawDebugFindPath(float *vOrigin);
void __cdecl Path_DrawFriendlyChain();
bool __cdecl Path_IsNodeIndex(const pathnode_t *node, unsigned int nodeIndexToCheck);
int __cdecl Path_NodesVisible(const pathnode_t *node0, const pathnode_t *node1);
int __cdecl Path_ExpandedNodeVisible(const pathnode_t *node0, const pathnode_t *node1);
pathnode_t *__cdecl Path_FindChainPos(const float *vOrigin, pathnode_t *pPrevChainPos);
void __cdecl Path_UpdateBestChainNode(pathnode_t *node, pathnode_t **bestNode, unsigned int *foundCount);
int __cdecl Path_CanSetDesiredChainPos(actor_s *claimer, const pathnode_t *node);
void __cdecl Path_AttachSentientToChainNode(sentient_s *sentient, unsigned __int16 targetname);
pathnode_t *__cdecl Path_FirstNode(int typeFlags);
pathnode_t *__cdecl Path_NextNode(pathnode_t *prevNode, int typeFlags);
sentient_s *__cdecl Path_GetNodeOwner(const pathnode_t *node);
int __cdecl Path_CanStealPriorityNode(const pathnode_t *node, sentient_s *claimer);
int __cdecl Path_CanStealNode(const pathnode_t *node, sentient_s *claimer);
void __cdecl Path_ClaimNodeInternal(pathnode_t *node, sentient_s *claimer);
void __cdecl Path_MarkNodeOverlap(pathnode_t *node);
void __cdecl Path_ClaimNode(pathnode_t *node, sentient_s *claimer);
void __cdecl Path_RevokeClaim(pathnode_t *node, sentient_s *pNewClaimer);
void __cdecl Path_RelinquishNode(sentient_s *claimer, int timeUntilRelinquished);
int __cdecl Path_AllowedStancesForNode(pathnode_t *node);
int __cdecl Path_SaveIndex(const pathnode_t *node);
pathnode_t *__cdecl Path_LoadNode(unsigned int index);
void __cdecl Path_ValidateNode(pathnode_t *node);
void __cdecl Path_ValidateAllNodes();
void __cdecl Path_CheckLinkLeaks();
void __cdecl Path_CheckUserCountLeaks();
void __cdecl Path_DisconnectPath(pathnode_t *node, pathlink_s *link);
void __cdecl Path_ConnectPath(pathnode_t *node, pathlink_s *link);
void __cdecl Path_ConnectPathsForEntity(gentity_s *ent);
void __cdecl Path_DisconnectPathsForEntity(gentity_s *ent);
void __cdecl Path_UpdateBadPlaceCountForLink(pathlink_s *link, int teamflags, int delta);
void __cdecl Path_UpdateArcBadPlaceCount(badplace_arc_t *arc, int teamflags, int delta);
void __cdecl Path_UpdateBrushBadPlaceCount(gentity_s *brushEnt, int teamflags, int delta);
int __cdecl Path_IsNodeInArc(
    pathnode_t *pNode,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight);
void __cdecl WriteEntityDisconnectedLinks(gentity_s *ent, SaveGame *save);
void __cdecl ReadEntityDisconnectedLinks(gentity_s *ent, SaveGame *save);
void __cdecl Scr_SetNodePriority();
void __cdecl Scr_IsNodeOccupied();
void __cdecl Scr_SetTurretNode();
void __cdecl Scr_UnsetTurretNode();
void __cdecl GScr_SetDynamicPathnodeField(pathnode_t *node, unsigned int index);
void __cdecl G_DropPathnodesToFloor();
void __cdecl Scr_FreePathnode(pathnode_t *node);
void __cdecl Scr_AddPathnode(pathnode_t *node);
void __cdecl Scr_GetNode();
void __cdecl Scr_GetNodeArray();
void __cdecl Scr_GetAllNodes();
void __cdecl Path_Shutdown();
void __cdecl Path_AutoDisconnectPaths();
void __cdecl Path_InitPaths();
void __cdecl Path_DrawVisData();
void __cdecl Path_RelinquishNodeNow(sentient_s *claimer);
void __cdecl Path_RelinquishNodeSoon(sentient_s *claimer);
void __cdecl Path_MarkNodeInvalid(pathnode_t *node, team_t eTeam);
void __cdecl G_SetPathnodeScriptVariable(const char *key, const char *value, pathnode_t *ent);
void __cdecl G_ParsePathnodeScriptField(const char *key, const char *value, pathnode_t *node);
void __cdecl G_ParsePathnodeScriptFields(pathnode_t *node);
void __cdecl G_SpawnPathnodeDynamic();
int __cdecl Path_CanClaimNode(const pathnode_t *node, sentient_s *claimer);
void __cdecl Path_ForceClaimNode(pathnode_t *node, sentient_s *claimer);
pathnode_t *__cdecl Path_ChooseSubsequentChainNode_r(
    int depthMin,
    int depthMax,
    pathnode_t *pParent,
    actor_s *claimer);
pathnode_t *__cdecl Path_ChooseAnyChainNodeIfDeadEnd(
    int depthMin,
    int depthMax,
    pathnode_t *chainPos,
    actor_s *claimer);
pathnode_t *__cdecl Path_ChoosePreviousChainNode(int depthMin, int depthMax, pathnode_t *chainPos, actor_s *claimer);
pathnode_t *__cdecl Path_ChooseDesperationChainNode(
    int depthMin,
    int depthMax,
    pathnode_t *refPos,
    actor_s *claimer);
pathnode_t *__cdecl Path_ChooseDesperationNewChainNode(
    int depthMin,
    int depthMax,
    pathnode_t *refPos,
    actor_s *claimer);
pathnode_t *__cdecl Path_ChooseChainPos(
    pathnode_t *refPos,
    int iFollowMin,
    int iFollowMax,
    actor_s *claimer,
    int chainFallback);
pathnode_t *__cdecl Path_NearestNodeNotCrossPlanes(
    const float * const vOrigin,
    pathsort_t *nodes,
    int typeFlags,
    float fMaxDist,
    float (*vNormal)[2],
    const float *fDist,
    int iPlaneCount,
    int *returnCount,
    int maxNodes,
    nearestNodeHeightCheck heightCheck);
pathnode_t *__cdecl Path_NearestNode(
    const float * const vOrigin,
    pathsort_t *nodes,
    int typeFlags,
    float fMaxDist,
    int *returnCount,
    int maxNodes,
    nearestNodeHeightCheck heightCheck);
void __cdecl Path_DrawDebugNearestNode(float *vOrigin, int numNodes);
void __cdecl Path_DrawDebugClaimedNodes(float *origin, int numNodes);
void __cdecl Path_DrawDebug();
#endif