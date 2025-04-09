#pragma once

// LWSS: This file has way too many structs. KISAKTODO: move out later.

#include "xanim_public.h"

#include <script/scr_stringlist.h>

#include <gfx_d3d/r_font.h>

#include <universal/com_math.h>

#include <bgame/bg_weapons.h>

#include <xanim/xasset.h>

union XAnimIndices // sizeof=0x4
{                                       // ...
    unsigned __int8 *_1;
    unsigned __int16 *_2;
    void *data;
};
struct XAnimNotifyInfo // sizeof=0x8
{
    unsigned __int16 name;
    // padding byte
    // padding byte
    float time;
};
union XAnimDynamicIndices // sizeof=0x2
{                                       // ...
    unsigned __int8 _1[1];
    unsigned __int16 _2[1];
};
struct __declspec(align(4)) XAnimDeltaPartQuatDataFrames // sizeof=0x8
{                                       // ...
    __int16 (*frames)[2];
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimDynamicFrames // sizeof=0x4
{                                       // ...
    unsigned __int8 (*_1)[3];
    unsigned __int16 (*_2)[3];
};
struct __declspec(align(4)) XAnimPartTransFrames // sizeof=0x20
{                                       // ...
    float mins[3];
    float size[3];
    XAnimDynamicFrames frames;
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimPartTransData // sizeof=0x20
{                                       // ...
    XAnimPartTransFrames frames;
    float frame0[3];
};
struct XAnimPartTrans // sizeof=0x24
{
    unsigned __int16 size;
    unsigned __int8 smallTrans;
    // padding byte
    XAnimPartTransData u;
};
union XAnimDeltaPartQuatData // sizeof=0x8
{                                       // ...
    XAnimDeltaPartQuatDataFrames frames;
    __int16 frame0[2];
};

struct XAnimDeltaPartQuat // sizeof=0xC
{
    unsigned __int16 size;
    // padding byte
    // padding byte
    XAnimDeltaPartQuatData u;
};
struct XAnimDeltaPart // sizeof=0x8
{
    XAnimPartTrans *trans;
    XAnimDeltaPartQuat *quat;
};
struct XAnimParts // sizeof=0x58
{                                       // ...
    const char *name;
    unsigned __int16 dataByteCount;
    unsigned __int16 dataShortCount;
    unsigned __int16 dataIntCount;
    unsigned __int16 randomDataByteCount;
    unsigned __int16 randomDataIntCount;
    unsigned __int16 numframes;
    bool bLoop;
    bool bDelta;
    unsigned __int8 boneCount[10];
    unsigned __int8 notifyCount;
    unsigned __int8 assetType;
    bool isDefault;
    // padding byte
    unsigned int randomDataShortCount;
    unsigned int indexCount;
    float framerate;
    float frequency;
    unsigned __int16 *names;
    unsigned __int8 *dataByte;
    __int16 *dataShort;
    int *dataInt;
    __int16 *randomDataShort;
    unsigned __int8 *randomDataByte;
    int *randomDataInt;
    XAnimIndices indices;
    XAnimNotifyInfo *notify;
    XAnimDeltaPart *deltaPart;
};

struct XModelNameMap // sizeof=0x4
{                                       // ...
    unsigned __int16 name;              // ...
    unsigned __int16 index;
};

struct XAnimParent // sizeof=0x4
{                                       // ...
    unsigned __int16 flags;
    unsigned __int16 children;
};
struct XAnimEntry // sizeof=0x8
{                                       // ...
    unsigned __int16 numAnims;
    unsigned __int16 parent;
    union //$7F333398CC08E12E110886895274CBFC
    {
        XAnimParts * parts;
        XAnimParent animParent;
    };
};
struct XAnim_s // sizeof=0x14
{
    const char *debugName;
    unsigned int size;
    const char **debugAnimNames;
    XAnimEntry entries[1];
};

struct __declspec(align(4)) XAnimTree_s // sizeof=0x14
{
    XAnim_s *anims;
    int info_usage;
    volatile long calcRefCount;
    volatile long modifyRefCount;
    unsigned __int16 children;
    // padding byte
    // padding byte
};
struct mnode_t // sizeof=0x4
{
    unsigned __int16 cellIndex;
    unsigned __int16 rightChildOffset;
};

struct DynEntityServer // sizeof=0x24
{
    GfxPlacement pose;
    unsigned __int16 flags;
    // padding byte
    // padding byte
    int health;
};

struct __declspec(align(4)) XAnimState // sizeof=0x20
{                                       // ...
    float currentAnimTime;              // ...
    float oldTime;                      // ...
    __int16 cycleCount;                 // ...
    __int16 oldCycleCount;              // ...
    float goalTime;                     // ...
    float goalWeight;                   // ...
    float weight;                       // ...
    float rate;                         // ...
    bool instantWeightChange;           // ...
    // padding byte
    // padding byte
    // padding byte
};

struct XAnimInfo // sizeof=0x40
{                                       // ...
    unsigned __int16 notifyChild;
    __int16 notifyIndex;
    unsigned __int16 notifyName;
    unsigned __int16 notifyType;
    unsigned __int16 prev;              // ...
    unsigned __int16 next;              // ...
    unsigned __int16 children;          // ...
    unsigned __int16 parent;            // ...
    unsigned __int16 animIndex;         // ...
    unsigned __int16 animToModel;
    bool inuse;                         // ...
    // padding byte
    // padding byte
    // padding byte
    XAnimTree_s* tree;
    //$7F333398CC08E12E110886895274CBFC ___u12;
    union
    {                                       // ...
        XAnimParts* parts;
        XAnimParent animParent;
    };
    XAnimState state;                   // ...
};

struct XAnimSimpleRotPos // sizeof=0x18
{                                       // ...
    float rot[2];                       // ...
    float posWeight;                    // ...
    float pos[3];                       // ...
};

struct XAnimDeltaInfo // sizeof=0x4
{                                       // ...
    bool bClear;                        // ...
    bool bNormQuat;                     // ...
    bool bAbs;                          // ...
    bool bUseGoalWeight;                // ...
};

struct XAnimNotify_s // sizeof=0xC
{                                       // ...
    const char* name;
    unsigned int type;
    float timeFrac;
};

struct cStaticModelWritable // sizeof=0x2
{                                       // ...
    unsigned __int16 nextModelInWorldSector;
};
struct cStaticModel_s // sizeof=0x50
{
    cStaticModelWritable writable;
    // padding byte
    // padding byte
    XModel* xmodel;
    float origin[3];
    float invScaledAxis[3][3];
    float absmin[3];
    float absmax[3];
};
struct dmaterial_t // sizeof=0x48
{
    char material[64];
    int surfaceFlags;
    int contentFlags;
};
struct cNode_t // sizeof=0x8
{
    cplane_s* plane;
    __int16 children[2];
};
struct __declspec(align(4)) cLeaf_t // sizeof=0x2C
{                                       // ...
    unsigned __int16 firstCollAabbIndex;
    unsigned __int16 collAabbCount;
    int brushContents;                  // ...
    int terrainContents;                // ...
    float mins[3];                      // ...
    float maxs[3];                      // ...
    int leafBrushNode;                  // ...
    __int16 cluster;
    // padding byte
    // padding byte
};
struct cLeafBrushNodeLeaf_t // sizeof=0x4
{                                       // ...
    unsigned __int16* brushes;
};
struct cLeafBrushNodeChildren_t // sizeof=0xC
{                                       // ...
    float dist;
    float range;
    unsigned __int16 childOffset[2];
};
union cLeafBrushNodeData_t // sizeof=0xC
{                                       // ...
    cLeafBrushNodeLeaf_t leaf;
    cLeafBrushNodeChildren_t children;
};
struct cLeafBrushNode_s // sizeof=0x14
{
    unsigned __int8 axis;
    // padding byte
    __int16 leafBrushCount;
    int contents;
    cLeafBrushNodeData_t data;
};
struct CollisionBorder // sizeof=0x1C
{
    float distEq[3];
    float zBase;
    float zSlope;
    float start;
    float length;
};
struct CollisionPartition // sizeof=0xC
{
    unsigned __int8 triCount;
    unsigned __int8 borderCount;
    // padding byte
    // padding byte
    int firstTri;
    CollisionBorder* borders;
};
union CollisionAabbTreeIndex // sizeof=0x4
{                                       // ...
    int firstChildIndex;
    int partitionIndex;
};
struct CollisionAabbTree // sizeof=0x20
{
    float origin[3];
    float halfSize[3];
    unsigned __int16 materialIndex;
    unsigned __int16 childCount;
    CollisionAabbTreeIndex u;
};
struct cmodel_t // sizeof=0x48
{                                       // ...
    float mins[3];
    float maxs[3];
    float radius;
    cLeaf_t leaf;                       // ...
};

struct __declspec(align(2)) cbrushside_t // sizeof=0xC
{                                       // ...
    cplane_s* plane;                    // ...
    unsigned int materialNum;           // ...
    __int16 firstAdjacentSideOffset;
    unsigned __int8 edgeCount;
    // padding byte
};

struct __declspec(align(16)) cbrush_t // sizeof=0x50
{                                       // ...
    float mins[3];
    int contents;
    float maxs[3];
    unsigned int numsides;
    cbrushside_t* sides;
    __int16 axialMaterialNum[2][3];
    unsigned __int8* baseAdjacentSide;
    __int16 firstAdjacentSideOffsets[2][3];
    unsigned __int8 edgeCount[2][3];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct MapEnts // sizeof=0xC
{                                       // ...
    const char* name;
    char* entityString;
    int numEntityChars;
};

struct DynEntityDef;
struct DynEntityPose;
struct DynEntityClient;
struct DynEntityColl;

struct clipMap_t // sizeof=0x11C
{                                       // ...
    const char* name;                   // ...
    int isInUse;                        // ...
    int planeCount;                     // ...
    cplane_s* planes;                   // ...
    unsigned int numStaticModels;       // ...
    cStaticModel_s* staticModelList;    // ...
    unsigned int numMaterials;          // ...
    dmaterial_t* materials;             // ...
    unsigned int numBrushSides;         // ...
    cbrushside_t* brushsides;           // ...
    unsigned int numBrushEdges;         // ...
    unsigned __int8* brushEdges;        // ...
    unsigned int numNodes;              // ...
    cNode_t* nodes;                     // ...
    unsigned int numLeafs;              // ...
    cLeaf_t* leafs;                     // ...
    unsigned int leafbrushNodesCount;   // ...
    cLeafBrushNode_s* leafbrushNodes;   // ...
    unsigned int numLeafBrushes;        // ...
    unsigned __int16* leafbrushes;      // ...
    unsigned int numLeafSurfaces;       // ...
    unsigned int* leafsurfaces;         // ...
    unsigned int vertCount;             // ...
    float (*verts)[3];                  // ...
    int triCount;                       // ...
    unsigned __int16* triIndices;       // ...
    unsigned __int8* triEdgeIsWalkable; // ...
    int borderCount;                    // ...
    CollisionBorder* borders;           // ...
    int partitionCount;                 // ...
    CollisionPartition* partitions;     // ...
    int aabbTreeCount;                  // ...
    CollisionAabbTree* aabbTrees;       // ...
    unsigned int numSubModels;          // ...
    cmodel_t* cmodels;                  // ...
    unsigned __int16 numBrushes;        // ...
    // padding byte
    // padding byte
    cbrush_t* brushes;                  // ...
    int numClusters;                    // ...
    int clusterBytes;                   // ...
    unsigned __int8* visibility;        // ...
    int vised;                          // ...
    MapEnts* mapEnts;                   // ...
    cbrush_t* box_brush;                // ...
    cmodel_t box_model;                 // ...
    unsigned __int16 dynEntCount[2];    // ...
    DynEntityDef* dynEntDefList[2];     // ...
    DynEntityPose* dynEntPoseList[2];   // ...
    DynEntityClient* dynEntClientList[2]; // ...
    DynEntityColl* dynEntCollList[2];   // ...
    unsigned int checksum;              // ...
};

struct ComPrimaryLight // sizeof=0x44
{
    unsigned __int8 type;
    unsigned __int8 canUseShadowMap;
    unsigned __int8 exponent;
    unsigned __int8 unused;
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    float cosHalfFovExpanded;
    float rotationLimit;
    float translationLimit;
    const char* defName;
};
struct ComWorld // sizeof=0x10
{                                       // ...
    const char* name;                   // ...
    int isInUse;                        // ...
    unsigned int primaryLightCount;     // ...
    ComPrimaryLight* primaryLights;     // ...
};

struct pathlink_s // sizeof=0xC
{
    float fDist;
    unsigned __int16 nodeNum;
    unsigned __int8 disconnectCount;
    unsigned __int8 negotiationLink;
    unsigned __int8 ubBadPlaceCount[4];
};
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
    pathlink_s* Links;
};
struct pathnode_dynamic_t // sizeof=0x20
{                                       // ...
    void* pOwner;
    int iFreeTime;
    int iValidTime[3];
    int inPlayerLOSTime;
    __int16 wLinkCount;
    __int16 wOverlapCount;
    __int16 turretEntNumber;
    __int16 userCount;
};
struct pathnode_t;
struct pathnode_transient_t // sizeof=0x1C
{
    int iSearchFrame;
    pathnode_t* pNextOpen;
    pathnode_t* pPrevOpen;
    pathnode_t* pParent;
    float fCost;
    float fHeuristic;
    float costFactor;
};
struct pathnode_t // sizeof=0x80
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

struct pathnode_tree_nodes_t // sizeof=0x8
{                                       // ...
    int nodeCount;
    unsigned __int16* nodes;
};

struct pathnode_tree_t;
union pathnode_tree_info_t // sizeof=0x8
{
    pathnode_tree_t* child[2];
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
    pathnode_t* nodes;
    pathbasenode_t* basenodes;
    unsigned int chainNodeCount;
    unsigned __int16* chainNodeForNode;
    unsigned __int16* nodeForChainNode;
    int visBytes;
    unsigned __int8* pathVis;
    int nodeTreeCount;
    pathnode_tree_t* nodeTree;
};
struct GameWorldSp // sizeof=0x2C
{
    const char* name;
    PathData path;
};
struct GameWorldMp // sizeof=0x4
{                                       // ...
    const char* name;
};

struct XModelDrawInfo // sizeof=0x4
{                                       // ...
    unsigned __int16 lod;
    unsigned __int16 surfId;
};
struct GfxSceneDynModel // sizeof=0x6
{
    XModelDrawInfo info;
    unsigned __int16 dynEntId;
};
struct BModelDrawInfo // sizeof=0x2
{                                       // ...
    unsigned __int16 surfId;
};
struct GfxSceneDynBrush // sizeof=0x4
{
    BModelDrawInfo info;
    unsigned __int16 dynEntId;
};
struct GfxShadowGeometry // sizeof=0xC
{
    unsigned __int16 surfaceCount;
    unsigned __int16 smodelCount;
    unsigned __int16* sortedSurfIndex;
    unsigned __int16* smodelIndex;
};
struct GfxLightRegionAxis // sizeof=0x14
{
    float dir[3];
    float midPoint;
    float halfSize;
};
struct GfxLightRegionHull // sizeof=0x50
{
    float kdopMidPoint[9];
    float kdopHalfSize[9];
    unsigned int axisCount;
    GfxLightRegionAxis* axis;
};
struct GfxLightRegion // sizeof=0x8
{
    unsigned int hullCount;
    GfxLightRegionHull* hulls;
};
struct sunflare_t // sizeof=0x60
{                                       // ...
    bool hasValidData;
    // padding byte
    // padding byte
    // padding byte
    Material* spriteMaterial;
    Material* flareMaterial;
    float spriteSize;
    float flareMinSize;
    float flareMinDot;
    float flareMaxSize;
    float flareMaxDot;
    float flareMaxAlpha;
    int flareFadeInTime;
    int flareFadeOutTime;
    float blindMinDot;
    float blindMaxDot;
    float blindMaxDarken;
    int blindFadeInTime;
    int blindFadeOutTime;
    float glareMinDot;
    float glareMaxDot;
    float glareMaxLighten;
    int glareFadeInTime;
    int glareFadeOutTime;
    float sunFxPosition[3];
};
struct GfxWorld // sizeof=0x2DC
{                                       // ...
    const char* name;                   // ...
    const char* baseName;               // ...
    int planeCount;                     // ...
    int nodeCount;                      // ...
    int indexCount;                     // ...
    unsigned __int16* indices;          // ...
    int surfaceCount;                   // ...
    GfxWorldStreamInfo streamInfo;
    // padding byte
    // padding byte
    // padding byte
    int skySurfCount;                   // ...
    int* skyStartSurfs;                 // ...
    GfxImage* skyImage;                 // ...
    unsigned __int8 skySamplerState;    // ...
    // padding byte
    // padding byte
    // padding byte
    unsigned int vertexCount;           // ...
    GfxWorldVertexData vd;              // ...
    unsigned int vertexLayerDataSize;   // ...
    GfxWorldVertexLayerData vld;        // ...
    SunLightParseParams sunParse;       // ...
    GfxLight* sunLight;                 // ...
    float sunColorFromBsp[3];
    unsigned int sunPrimaryLightIndex;  // ...
    unsigned int primaryLightCount;     // ...
    int cullGroupCount;                 // ...
    unsigned int reflectionProbeCount;  // ...
    GfxReflectionProbe* reflectionProbes; // ...
    GfxTexture* reflectionProbeTextures; // ...
    GfxWorldDpvsPlanes dpvsPlanes;      // ...
    int cellBitsCount;                  // ...
    GfxCell* cells;                     // ...
    int lightmapCount;                  // ...
    GfxLightmapArray* lightmaps;        // ...
    GfxLightGrid lightGrid;             // ...
    GfxTexture* lightmapPrimaryTextures; // ...
    GfxTexture* lightmapSecondaryTextures; // ...
    int modelCount;                     // ...
    GfxBrushModel* models;              // ...
    float mins[3];                      // ...
    float maxs[3];                      // ...
    unsigned int checksum;
    int materialMemoryCount;            // ...
    MaterialMemory* materialMemory;     // ...
    sunflare_t sun;                     // ...
    float outdoorLookupMatrix[4][4];
    GfxImage* outdoorImage;
    unsigned int* cellCasterBits;       // ...
    GfxSceneDynModel* sceneDynModel;    // ...
    GfxSceneDynBrush* sceneDynBrush;    // ...
    unsigned int* primaryLightEntityShadowVis; // ...
    unsigned int* primaryLightDynEntShadowVis[2]; // ...
    unsigned __int8* nonSunPrimaryLightForModelDynEnt; // ...
    GfxShadowGeometry* shadowGeom;      // ...
    GfxLightRegion* lightRegion;        // ...
    GfxWorldDpvsStatic dpvs;            // ...
    GfxWorldDpvsDynamic dpvsDyn;        // ...
};



struct rectDef_s // sizeof=0x18
{                                       // ...
    float x;                            // ...
    float y;                            // ...
    float w;                            // ...
    float h;                            // ...
    int horzAlign;                      // ...
    int vertAlign;                      // ...
};
struct windowDef_t // sizeof=0x9C
{                                       // ...
    const char* name;
    rectDef_s rect;
    rectDef_s rectClient;
    const char* group;
    int style;
    int border;
    int ownerDraw;
    int ownerDrawFlags;
    float borderSize;
    int staticFlags;
    int dynamicFlags[1];
    int nextTime;
    float foreColor[4];
    float backColor[4];
    float borderColor[4];
    float outlineColor[4];
    Material* background;
};
struct ItemKeyHandler // sizeof=0xC
{
    int key;
    const char* action;
    ItemKeyHandler* next;
};

union operandInternalDataUnion // sizeof=0x4
{                                       // ...
    operandInternalDataUnion()
    {
        intVal = 0;
    }
    operandInternalDataUnion(int i)
    {
        intVal = i;
    }
    operandInternalDataUnion(float f)
    {
        floatVal = f;
    }
    operandInternalDataUnion(const char *str)
    {
        string = str;
    }

    operator int()
    {
        return intVal;
    }
    operator float()
    {
        return floatVal;
    }
    int intVal;
    float floatVal;
    const char* string;
};
struct Operand // sizeof=0x8
{                                       // ...
    expDataType dataType;               // ...
    operandInternalDataUnion internals; // ...
};
union entryInternalData // sizeof=0x8
{                                       // ...
    operationEnum op;
    Operand operand;
};
struct expressionEntry // sizeof=0xC
{
    int type;
    entryInternalData data;
};
struct statement_s // sizeof=0x8
{                                       // ...
    int numEntries;
    expressionEntry** entries;
};
struct columnInfo_s // sizeof=0x10
{                                       // ...
    int pos;
    int width;
    int maxChars;
    int alignment;
};
struct listBoxDef_s // sizeof=0x154
{
    int mousePos;
    int startPos[1];
    int endPos[1];
    int drawPadding;
    float elementWidth;
    float elementHeight;
    int elementStyle;
    int numColumns;
    columnInfo_s columnInfo[16];
    const char* doubleClick;
    int notselectable;
    int noScrollBars;
    int usePaging;
    float selectBorder[4];
    float disableColor[4];
    Material* selectIcon;
};
struct editFieldDef_s // sizeof=0x20
{
    float minVal;
    float maxVal;
    float defVal;
    float range;
    int maxChars;
    int maxCharsGotoNext;
    int maxPaintChars;
    int paintOffset;
};
struct multiDef_s // sizeof=0x188
{
    const char* dvarList[32];
    const char* dvarStr[32];
    float dvarValue[32];
    int count;
    int strDef;
};
union itemDefData_t // sizeof=0x4
{                                       // ...
    listBoxDef_s* listBox;
    editFieldDef_s* editField;
    multiDef_s* multi;
    const char* enumDvarName;
    void* data;
};
struct itemDef_s // sizeof=0x174
{                                       // ...
    windowDef_t window;
    rectDef_s textRect[1];
    int type;
    int dataType;
    int alignment;
    int fontEnum;
    int textAlignMode;
    float textalignx;
    float textaligny;
    float textscale;
    int textStyle;
    int gameMsgWindowIndex;
    int gameMsgWindowMode;
    const char* text;
    int itemFlags;
    menuDef_t* parent;                  // ...
    const char* mouseEnterText;
    const char* mouseExitText;
    const char* mouseEnter;
    const char* mouseExit;
    const char* action;
    const char* onAccept;
    const char* onFocus;
    const char* leaveFocus;
    const char* dvar;
    const char* dvarTest;
    ItemKeyHandler* onKey;
    const char* enableDvar;
    int dvarFlags;
    snd_alias_list_t* focusSound;
    float special;
    int cursorPos[1];
    itemDefData_t typeData;
    int imageTrack;
    statement_s visibleExp;
    statement_s textExp;
    statement_s materialExp;
    statement_s rectXExp;
    statement_s rectYExp;
    statement_s rectWExp;
    statement_s rectHExp;
    statement_s forecolorAExp;
};
struct menuDef_t // sizeof=0x11C
{                                       // ...
    windowDef_t window;
    const char* font;
    int fullScreen;
    int itemCount;
    int fontIndex;
    int cursorItem[1];
    int fadeCycle;
    float fadeClamp;
    float fadeAmount;
    float fadeInAmount;
    float blurRadius;
    const char* onOpen;
    const char* onClose;
    const char* onESC;
    ItemKeyHandler* onKey;
    statement_s visibleExp;
    const char* allowedBinding;
    const char* soundName;
    int imageTrack;
    float focusColor[4];
    float disableColor[4];
    statement_s rectXExp;
    statement_s rectYExp;
    itemDef_s** items;
};
struct MenuList // sizeof=0xC
{                                       // ...
    const char* name;
    int menuCount;                      // ...
    menuDef_t** menus;                  // ...
};

struct LocalizeEntry // sizeof=0x8
{                                       // ...
    const char* value;
    const char* name;
};


struct WeaponDef // sizeof=0x878
{                                       // ...
    const char* szInternalName;
    const char* szDisplayName;
    const char* szOverlayName;
    XModel* gunXModel[16];
    XModel* handXModel;
    const char* szXAnims[33];
    const char* szModeName;
    unsigned __int16 hideTags[8];
    unsigned __int16 notetrackSoundMapKeys[16];
    unsigned __int16 notetrackSoundMapValues[16];
    int playerAnimType;
    weapType_t weapType;
    weapClass_t weapClass;
    PenetrateType penetrateType;
    ImpactType impactType;
    weapInventoryType_t inventoryType;
    weapFireType_t fireType;
    OffhandClass offhandClass;
    weapStance_t stance;
    const FxEffectDef* viewFlashEffect;
    const FxEffectDef* worldFlashEffect;
    snd_alias_list_t* pickupSound;
    snd_alias_list_t* pickupSoundPlayer;
    snd_alias_list_t* ammoPickupSound;
    snd_alias_list_t* ammoPickupSoundPlayer;
    snd_alias_list_t* projectileSound;
    snd_alias_list_t* pullbackSound;
    snd_alias_list_t* pullbackSoundPlayer;
    snd_alias_list_t* fireSound;
    snd_alias_list_t* fireSoundPlayer;
    snd_alias_list_t* fireLoopSound;
    snd_alias_list_t* fireLoopSoundPlayer;
    snd_alias_list_t* fireStopSound;
    snd_alias_list_t* fireStopSoundPlayer;
    snd_alias_list_t* fireLastSound;
    snd_alias_list_t* fireLastSoundPlayer;
    snd_alias_list_t* emptyFireSound;
    snd_alias_list_t* emptyFireSoundPlayer;
    snd_alias_list_t* meleeSwipeSound;
    snd_alias_list_t* meleeSwipeSoundPlayer;
    snd_alias_list_t* meleeHitSound;
    snd_alias_list_t* meleeMissSound;
    snd_alias_list_t* rechamberSound;
    snd_alias_list_t* rechamberSoundPlayer;
    snd_alias_list_t* reloadSound;
    snd_alias_list_t* reloadSoundPlayer;
    snd_alias_list_t* reloadEmptySound;
    snd_alias_list_t* reloadEmptySoundPlayer;
    snd_alias_list_t* reloadStartSound;
    snd_alias_list_t* reloadStartSoundPlayer;
    snd_alias_list_t* reloadEndSound;
    snd_alias_list_t* reloadEndSoundPlayer;
    snd_alias_list_t* detonateSound;
    snd_alias_list_t* detonateSoundPlayer;
    snd_alias_list_t* nightVisionWearSound;
    snd_alias_list_t* nightVisionWearSoundPlayer;
    snd_alias_list_t* nightVisionRemoveSound;
    snd_alias_list_t* nightVisionRemoveSoundPlayer;
    snd_alias_list_t* altSwitchSound;
    snd_alias_list_t* altSwitchSoundPlayer;
    snd_alias_list_t* raiseSound;
    snd_alias_list_t* raiseSoundPlayer;
    snd_alias_list_t* firstRaiseSound;
    snd_alias_list_t* firstRaiseSoundPlayer;
    snd_alias_list_t* putawaySound;
    snd_alias_list_t* putawaySoundPlayer;
    snd_alias_list_t** bounceSound;
    const FxEffectDef* viewShellEjectEffect;
    const FxEffectDef* worldShellEjectEffect;
    const FxEffectDef* viewLastShotEjectEffect;
    const FxEffectDef* worldLastShotEjectEffect;
    Material* reticleCenter;
    Material* reticleSide;
    int iReticleCenterSize;
    int iReticleSideSize;
    int iReticleMinOfs;
    activeReticleType_t activeReticleType;
    float vStandMove[3];
    float vStandRot[3];
    float vDuckedOfs[3];
    float vDuckedMove[3];
    float vDuckedRot[3];
    float vProneOfs[3];
    float vProneMove[3];
    float vProneRot[3];
    float fPosMoveRate;
    float fPosProneMoveRate;
    float fStandMoveMinSpeed;
    float fDuckedMoveMinSpeed;
    float fProneMoveMinSpeed;
    float fPosRotRate;
    float fPosProneRotRate;
    float fStandRotMinSpeed;
    float fDuckedRotMinSpeed;
    float fProneRotMinSpeed;
    XModel* worldModel[16];
    XModel* worldClipModel;
    XModel* rocketModel;
    XModel* knifeModel;
    XModel* worldKnifeModel;
    Material* hudIcon;
    weaponIconRatioType_t hudIconRatio;
    Material* ammoCounterIcon;
    weaponIconRatioType_t ammoCounterIconRatio;
    ammoCounterClipType_t ammoCounterClip;
    int iStartAmmo;
    const char* szAmmoName;
    int iAmmoIndex;
    const char* szClipName;
    int iClipIndex;
    int iMaxAmmo;
    int iClipSize;
    int shotCount;
    const char* szSharedAmmoCapName;
    int iSharedAmmoCapIndex;
    int iSharedAmmoCap;
    int damage;
    int playerDamage;
    int iMeleeDamage;
    int iDamageType;
    int iFireDelay;
    int iMeleeDelay;
    int meleeChargeDelay;
    int iDetonateDelay;
    int iFireTime;
    int iRechamberTime;
    int iRechamberBoltTime;
    int iHoldFireTime;
    int iDetonateTime;
    int iMeleeTime;
    int meleeChargeTime;
    int iReloadTime;
    int reloadShowRocketTime;
    int iReloadEmptyTime;
    int iReloadAddTime;
    int iReloadStartTime;
    int iReloadStartAddTime;
    int iReloadEndTime;
    int iDropTime;
    int iRaiseTime;
    int iAltDropTime;
    int iAltRaiseTime;
    int quickDropTime;
    int quickRaiseTime;
    int iFirstRaiseTime;
    int iEmptyRaiseTime;
    int iEmptyDropTime;
    int sprintInTime;
    int sprintLoopTime;
    int sprintOutTime;
    int nightVisionWearTime;
    int nightVisionWearTimeFadeOutEnd;
    int nightVisionWearTimePowerUp;
    int nightVisionRemoveTime;
    int nightVisionRemoveTimePowerDown;
    int nightVisionRemoveTimeFadeInStart;
    int fuseTime;
    int aiFuseTime;
    int requireLockonToFire;
    int noAdsWhenMagEmpty;
    int avoidDropCleanup;
    float autoAimRange;
    float aimAssistRange;
    float aimAssistRangeAds;
    float aimPadding;
    float enemyCrosshairRange;
    int crosshairColorChange;
    float moveSpeedScale;
    float adsMoveSpeedScale;
    float sprintDurationScale;
    float fAdsZoomFov;
    float fAdsZoomInFrac;
    float fAdsZoomOutFrac;
    Material* overlayMaterial;
    Material* overlayMaterialLowRes;
    weapOverlayReticle_t overlayReticle;
    WeapOverlayInteface_t overlayInterface;
    float overlayWidth;
    float overlayHeight;
    float fAdsBobFactor;
    float fAdsViewBobMult;
    float fHipSpreadStandMin;
    float fHipSpreadDuckedMin;
    float fHipSpreadProneMin;
    float hipSpreadStandMax;
    float hipSpreadDuckedMax;
    float hipSpreadProneMax;
    float fHipSpreadDecayRate;
    float fHipSpreadFireAdd;
    float fHipSpreadTurnAdd;
    float fHipSpreadMoveAdd;
    float fHipSpreadDuckedDecay;
    float fHipSpreadProneDecay;
    float fHipReticleSidePos;
    int iAdsTransInTime;
    int iAdsTransOutTime;
    float fAdsIdleAmount;
    float fHipIdleAmount;
    float adsIdleSpeed;
    float hipIdleSpeed;
    float fIdleCrouchFactor;
    float fIdleProneFactor;
    float fGunMaxPitch;
    float fGunMaxYaw;
    float swayMaxAngle;
    float swayLerpSpeed;
    float swayPitchScale;
    float swayYawScale;
    float swayHorizScale;
    float swayVertScale;
    float swayShellShockScale;
    float adsSwayMaxAngle;
    float adsSwayLerpSpeed;
    float adsSwayPitchScale;
    float adsSwayYawScale;
    float adsSwayHorizScale;
    float adsSwayVertScale;
    int bRifleBullet;
    int armorPiercing;
    int bBoltAction;
    int aimDownSight;
    int bRechamberWhileAds;
    float adsViewErrorMin;
    float adsViewErrorMax;
    int bCookOffHold;
    int bClipOnly;
    int adsFireOnly;
    int cancelAutoHolsterWhenEmpty;
    int suppressAmmoReserveDisplay;
    int enhanced;
    int laserSightDuringNightvision;
    Material* killIcon;
    weaponIconRatioType_t killIconRatio;
    int flipKillIcon;
    Material* dpadIcon;
    weaponIconRatioType_t dpadIconRatio;
    int bNoPartialReload;
    int bSegmentedReload;
    int iReloadAmmoAdd;
    int iReloadStartAdd;
    const char* szAltWeaponName;
    unsigned int altWeaponIndex;
    int iDropAmmoMin;
    int iDropAmmoMax;
    int blocksProne;
    int silenced;
    int iExplosionRadius;
    int iExplosionRadiusMin;
    int iExplosionInnerDamage;
    int iExplosionOuterDamage;
    float damageConeAngle;
    int iProjectileSpeed;
    int iProjectileSpeedUp;
    int iProjectileSpeedForward;
    int iProjectileActivateDist;
    float projLifetime;
    float timeToAccelerate;
    float projectileCurvature;
    XModel* projectileModel;
    weapProjExposion_t projExplosion;
    const FxEffectDef* projExplosionEffect;
    int projExplosionEffectForceNormalUp;
    const FxEffectDef* projDudEffect;
    snd_alias_list_t* projExplosionSound;
    snd_alias_list_t* projDudSound;
    int bProjImpactExplode;
    WeapStickinessType stickiness;
    int hasDetonator;
    int timedDetonation;
    int rotate;
    int holdButtonToThrow;
    int freezeMovementWhenFiring;
    float lowAmmoWarningThreshold;
    float parallelBounce[29];
    float perpendicularBounce[29];
    const FxEffectDef* projTrailEffect;
    float vProjectileColor[3];
    guidedMissileType_t guidedMissileType;
    float maxSteeringAccel;
    int projIgnitionDelay;
    const FxEffectDef* projIgnitionEffect;
    snd_alias_list_t* projIgnitionSound;
    float fAdsAimPitch;
    float fAdsCrosshairInFrac;
    float fAdsCrosshairOutFrac;
    int adsGunKickReducedKickBullets;
    float adsGunKickReducedKickPercent;
    float fAdsGunKickPitchMin;
    float fAdsGunKickPitchMax;
    float fAdsGunKickYawMin;
    float fAdsGunKickYawMax;
    float fAdsGunKickAccel;
    float fAdsGunKickSpeedMax;
    float fAdsGunKickSpeedDecay;
    float fAdsGunKickStaticDecay;
    float fAdsViewKickPitchMin;
    float fAdsViewKickPitchMax;
    float fAdsViewKickYawMin;
    float fAdsViewKickYawMax;
    float fAdsViewKickCenterSpeed;
    float fAdsViewScatterMin;
    float fAdsViewScatterMax;
    float fAdsSpread;
    int hipGunKickReducedKickBullets;
    float hipGunKickReducedKickPercent;
    float fHipGunKickPitchMin;
    float fHipGunKickPitchMax;
    float fHipGunKickYawMin;
    float fHipGunKickYawMax;
    float fHipGunKickAccel;
    float fHipGunKickSpeedMax;
    float fHipGunKickSpeedDecay;
    float fHipGunKickStaticDecay;
    float fHipViewKickPitchMin;
    float fHipViewKickPitchMax;
    float fHipViewKickYawMin;
    float fHipViewKickYawMax;
    float fHipViewKickCenterSpeed;
    float fHipViewScatterMin;
    float fHipViewScatterMax;
    float fightDist;
    float maxDist;
    const char* accuracyGraphName[2];
    float (*accuracyGraphKnots[2])[2];
    float (*originalAccuracyGraphKnots[2])[2];
    int accuracyGraphKnotCount[2];
    int originalAccuracyGraphKnotCount[2];
    int iPositionReloadTransTime;
    float leftArc;
    float rightArc;
    float topArc;
    float bottomArc;
    float accuracy;
    float aiSpread;
    float playerSpread;
    float minTurnSpeed[2];
    float maxTurnSpeed[2];
    float pitchConvergenceTime;
    float yawConvergenceTime;
    float suppressTime;
    float maxRange;
    float fAnimHorRotateInc;
    float fPlayerPositionDist;
    const char* szUseHintString;
    const char* dropHintString;
    int iUseHintStringIndex;
    int dropHintStringIndex;
    float horizViewJitter;
    float vertViewJitter;
    const char* szScript;
    float fOOPosAnimLength[2];
    int minDamage;
    int minPlayerDamage;
    float fMaxDamageRange;
    float fMinDamageRange;
    float destabilizationRateTime;
    float destabilizationCurvatureMax;
    int destabilizeDistance;
    float locationDamageMultipliers[19];
    const char* fireRumble;
    const char* meleeImpactRumble;
    float adsDofStart;
    float adsDofEnd;
};

struct SndDriverGlobals // sizeof=0x4
{                                       // ...
    const char* name;
};

struct RawFile // sizeof=0xC
{                                       // ...
    const char* name;
    int len;
    const char* buffer;
};
struct DObj_s;

struct gentity_s;

void __cdecl SV_DObjInitServerTime(gentity_s* ent, float dtime);
void __cdecl SV_DObjDisplayAnim(gentity_s* ent, const char* header);

unsigned __int8 *__cdecl Hunk_AllocXAnimPrecache(unsigned int size);

int __cdecl XAnimGetTreeHighMemUsage();
int __cdecl XAnimGetTreeMemUsage();
void __cdecl TRACK_xanim();
int __cdecl XAnimGetTreeMaxMemUsage();
void __cdecl XAnimInit();
void __cdecl XAnimShutdown();
XAnimParts* __cdecl XAnimFindData_LoadObj(const char* name);
XAnimParts* __cdecl XAnimFindData_FastFile(const char* name);
void __cdecl XAnimCreate(XAnim_s* anims, unsigned int animIndex, const char* name);
XAnimParts *__cdecl XAnimPrecache(char *name, void *(__cdecl *Alloc)(int));
void __cdecl XAnimBlend(
    XAnim_s* anims,
    unsigned int animIndex,
    const char* name,
    unsigned int children,
    unsigned int num,
    unsigned int flags);
bool __cdecl IsNodeAdditive(const XAnimEntry* node);
BOOL __cdecl IsLeafNode(const XAnimEntry* anim);
XAnim_s* __cdecl XAnimCreateAnims(const char* debugName, unsigned int size, void* (__cdecl* Alloc)(int));
void __cdecl XAnimFreeList(XAnim_s* anims);
int __cdecl XAnimTreeSize();
XAnimTree_s* __cdecl XAnimCreateTree(XAnim_s* anims, void* (__cdecl* Alloc)(int));
void __cdecl XAnimFreeTree(XAnimTree_s* tree, void(__cdecl* Free)(void*, int));
void XAnimCheckTreeLeak();
XAnim_s* __cdecl XAnimGetAnims(const XAnimTree_s* tree);
void XAnimResetAnimMap(XModelNameMap a1, const DObj_s* obj, unsigned int infoIndex);
void __cdecl XAnimInitModelMap(XModel* const* models, unsigned int numModels, XModelNameMap* modelMap);
void __cdecl XAnimResetAnimMap_r(XModelNameMap* modelMap, unsigned int infoIndex);
void __cdecl XAnimResetAnimMapLeaf(const XModelNameMap* modelMap, unsigned int infoIndex);
HashEntry_unnamed_type_u __cdecl XAnimGetAnimMap(const XAnimParts* parts, const XModelNameMap* modelMap);
double __cdecl XAnimGetLength(const XAnim_s* anims, unsigned int animIndex);
int __cdecl XAnimGetLengthMsec(const XAnim_s* anims, unsigned int anim);
double __cdecl XAnimGetTime(const XAnimTree_s* tree, unsigned int animIndex);
unsigned int __cdecl XAnimGetInfoIndex(const XAnimTree_s* tree, unsigned int animIndex);
unsigned int __cdecl XAnimGetInfoIndex_r(const XAnimTree_s* tree, unsigned int animIndex, unsigned int infoIndex);
double __cdecl XAnimGetWeight(const XAnimTree_s* tree, unsigned int animIndex);
bool __cdecl XAnimHasFinished(const XAnimTree_s* tree, unsigned int animIndex);
int __cdecl XAnimGetNumChildren(const XAnim_s* anims, unsigned int animIndex);
unsigned int __cdecl XAnimGetChildAt(const XAnim_s* anims, unsigned int animIndex, unsigned int childIndex);
const char* __cdecl XAnimGetAnimName(const XAnim_s* anims, unsigned int animIndex);
char* __cdecl XAnimGetAnimDebugName(const XAnim_s* anims, unsigned int animIndex);
const char* __cdecl XAnimGetAnimTreeDebugName(const XAnim_s* anims);
unsigned int __cdecl XAnimGetAnimTreeSize(const XAnim_s* anims);
void __cdecl XAnimInitInfo(XAnimInfo* info);
void __cdecl XAnimUpdateOldTime(
    DObj_s* obj,
    unsigned int infoIndex,
    XAnimState* syncState,
    float dtime,
    bool parentHasWeight,
    bool* childHasTimeForParent);
unsigned int __cdecl XAnimInitTime(XAnimTree_s* tree, unsigned int infoIndex, float goalTime);
void __cdecl XAnimResetTime(unsigned int infoIndex);
void __cdecl XAnimResetTimeInternal(unsigned int infoIndex);
unsigned int __cdecl XAnimCloneInitTime(XAnimTree_s* tree, unsigned int infoIndex, unsigned int parentIndex);
void __cdecl DObjInitServerTime(DObj_s* obj, float dtime);
void __cdecl DObjUpdateClientInfo(DObj_s* obj, float dtime, bool notify);
void __cdecl XAnimUpdateTimeAndNotetrack(const DObj_s* obj, unsigned int infoIndex, float dtime, bool bNotify);
void __cdecl XAnimCheckFreeInfo(XAnimTree_s* tree, unsigned int infoIndex, int hasWeight);
void __cdecl XAnimFreeInfo(XAnimTree_s* tree, unsigned int infoIndex);
void __cdecl XAnimClearServerNotify(XAnimInfo* info);
double __cdecl XAnimGetAverageRateFrequency(__int64 tree);
void __cdecl XAnimUpdateTimeAndNotetrackLeaf(
    const DObj_s* obj,
    const XAnimParts* parts,
    unsigned int infoIndex,
    float dtime,
    bool bNotify);
void __cdecl XAnimProcessClientNotify(XAnimInfo* info, float dtime);
unsigned __int16 __cdecl XAnimGetNextNotifyIndex(const XAnimParts* parts, float time);
double __cdecl XAnimGetNotifyFracLeaf(const XAnimState* state, const XAnimState* nextState, float time, float dtime);
void __cdecl XAnimAddClientNotify(unsigned int notetrackName, float frac, unsigned int notifyType);
void __cdecl XAnimUpdateTimeAndNotetrackSyncSubTree(
    const DObj_s* obj,
    unsigned int infoIndex,
    float dtime,
    bool bNotify);
void __cdecl XAnimUpdateInfoSync(
    const DObj_s* obj,
    unsigned int infoIndex,
    bool bNotify,
    XAnimState* syncState,
    float dtime);
void __cdecl XAnimProcessServerNotify(const DObj_s* obj, XAnimInfo* info, float time);
XAnimParts* __cdecl XAnimGetParts(const XAnimTree_s* tree, XAnimInfo* info);
void __cdecl NotifyServerNotetrack(const DObj_s* obj, unsigned int notifyName, unsigned int notetrackName);
int __cdecl DObjUpdateServerInfo(DObj_s* obj, float dtime, int bNotify);
double __cdecl XAnimFindServerNoteTrack(const DObj_s* obj, unsigned int infoIndex, float dtime);
double __cdecl XAnimFindServerNoteTrackLeafNode(const DObj_s* obj, XAnimInfo* info, float dtime);
double __cdecl XAnimGetNextServerNotifyFrac(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime);
double __cdecl XAnimFindServerNoteTrackSyncSubTree(const DObj_s* obj, XAnimInfo* info, float dtime);
double __cdecl XAnimGetServerNotifyFracSyncTotal(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime);
int __cdecl DObjGetClientNotifyList(XAnimNotify_s** notifyList);
void __cdecl DObjDisplayAnimToBuffer(const DObj_s* obj, const char* header, char* buffer, int bufferSize);
void __cdecl XAnimDisplay(const XAnimTree_s* tree, __int64 infoIndex, char* buffer, __int64 bufferSize);
void __cdecl DObjDisplayAnim(const DObj_s* obj, const char* header);
void __cdecl XAnimCalcDelta(DObj_s* obj, unsigned int animIndex, float* rot, float* trans, bool bUseGoalWeight);
void __cdecl XAnimCalcDeltaTree(
    const DObj_s* obj,
    unsigned int infoIndex,
    float weightScale,
    XAnimDeltaInfo deltaInfo,
    XAnimSimpleRotPos* rotPos);
void __cdecl XAnimCalcRelDeltaParts(
    const XAnimParts* parts,
    float weightScale,
    float time1,
    float time2,
    XAnimSimpleRotPos* rotPos,
    int quatIndex);
void __cdecl TransformToQuatRefFrame(const float* rot, float* trans);
void __cdecl XAnimCalcAbsDeltaParts(const XAnimParts* parts, float weightScale, float time, XAnimSimpleRotPos* rotPos);
void __cdecl XAnimCalcAbsDelta(DObj_s* obj, unsigned int animIndex, float* rot, float* trans);
void __cdecl XAnimGetRelDelta(
    const XAnim_s* anims,
    unsigned int animIndex,
    float* rot,
    float* trans,
    float time1,
    float time2);
void __cdecl XAnimGetAbsDelta(const XAnim_s* anims, unsigned int animIndex, float* rot, float* trans, float time);
unsigned int __cdecl XAnimAllocInfoWithParent(
    XAnimTree_s* tree,
    unsigned __int16 animToModel,
    unsigned int animIndex,
    unsigned int parentInfoIndex,
    int after);
unsigned int XAnimAllocInfoIndex(
    unsigned int a1,
    DObj_s* obj,
    unsigned int animIndex,
    int after);
unsigned int __cdecl XAnimEnsureGoalWeightParent(DObj_s* obj, unsigned int animIndex);
void __cdecl XAnimClearGoalWeightInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime);
void __cdecl XAnimClearTreeGoalWeightsInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime);
void __cdecl XAnimClearTreeGoalWeights(XAnimTree_s* tree, unsigned int animIndex, float blendTime);
void __cdecl XAnimClearTreeGoalWeightsStrict(XAnimTree_s* tree, unsigned int animIndex, float blendTime);
void __cdecl XAnimClearGoalWeightKnobInternal(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime);
int __cdecl XAnimSetCompleteGoalWeightNode(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType);
int __cdecl XAnimSetGoalWeightKnobAll(
    DObj_s* obj,
    unsigned int animIndex,
    unsigned int rootIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    int bRestart);
int __cdecl XAnimSetGoalWeightKnob(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    int bRestart);
void __cdecl XAnimClearTree(XAnimTree_s* tree);
int __cdecl XAnimSetGoalWeightNode(
    XAnimTree_s* tree,
    unsigned int infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType);
unsigned int __cdecl XAnimGetDescendantWithGreatestWeight(const XAnimTree_s* tree, unsigned int infoIndex);
void __cdecl XAnimSetupSyncNodes(XAnim_s* anims);
void __cdecl XAnimSetupSyncNodes_r(XAnim_s* anims, unsigned int animIndex);
void __cdecl XAnimFillInSyncNodes_r(XAnim_s* anims, unsigned int animIndex, bool bLoop);
bool __cdecl XAnimHasTime(const XAnim_s* anims, unsigned int animIndex);
BOOL __cdecl XAnimIsPrimitive(XAnim_s* anims, unsigned int animIndex);
void __cdecl XAnimSetTime(XAnimTree_s *tree, unsigned int animIndex, float time);
void __cdecl XAnimUpdateServerNotifyIndex(XAnimInfo* info, const XAnimParts* parts);
unsigned int __cdecl XAnimRestart(XAnimTree_s* tree, unsigned int infoIndex, float goalTime);
int __cdecl XAnimSetGoalWeight(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType,
    int bRestart);
void __cdecl XAnimSetAnimRate(XAnimTree_s* tree, unsigned int animIndex, float rate);
bool __cdecl XAnimIsLooped(const XAnim_s* anims, unsigned int animIndex);
char __cdecl XAnimNotetrackExists(const XAnim_s* anims, unsigned int animIndex, unsigned int name);
void __cdecl XAnimAddNotetrackTimesToScriptArray(const XAnim_s* anims, unsigned int animIndex, unsigned int name);
int __cdecl XAnimSetCompleteGoalWeight(
    DObj_s* obj,
    unsigned int animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    unsigned int notifyName,
    unsigned int notifyType,
    int bRestart);
void __cdecl XAnimCloneAnimInfo(const XAnimInfo* from, XAnimInfo* to);
void __cdecl XAnimCloneAnimTree(const XAnimTree_s* from, XAnimTree_s* to);
void __cdecl XAnimCloneAnimTree_r(
    const XAnimTree_s* from,
    XAnimTree_s* to,
    unsigned int fromInfoIndex,
    unsigned int toInfoParentIndex);
XAnimInfo* __cdecl GetAnimInfo(int infoIndex);
//void __thiscall bitarray<128>::setBit(bitarray<128>* this, unsigned int pos);
