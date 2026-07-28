#pragma once

#include <gfx_d3d/fxprimitives.h>

#define FX_MARK_FREE -1
#define FX_MARKS_LIMIT 512

#define FX_TRI_GROUP_LIMIT 2048
#define FX_POINT_GROUP_LIMIT 3072

enum $7B978A8EAF4AE2627C6F063D7A9BDEE5 : __int32
{
    MARK_MODEL_TYPE_WORLD_BRUSH = 0x0,
    MARK_MODEL_TYPE_WORLD_MODEL = 0x40,
    MARK_MODEL_TYPE_ENT_BRUSH   = 0x80,
    MARK_MODEL_TYPE_ENT_MODEL   = 0xC0,
    MARK_MODEL_TYPE_MASK        = 0xC0,
    MARK_MODEL_SURF_MASK        = 0x3F,
};

struct FxMarksSystem
{                                       // ...
    int frameCount;
    uint16_t firstFreeMarkHandle;
    uint16_t firstActiveWorldMarkHandle;
    uint16_t entFirstMarkHandles[MAX_GENTITIES];
    FxTriGroupPool *firstFreeTriGroup;
    FxPointGroupPool *firstFreePointGroup;
    FxMark marks[FX_MARKS_LIMIT];
    FxTriGroupPool triGroups[FX_TRI_GROUP_LIMIT];
    FxPointGroupPool pointGroups[FX_POINT_GROUP_LIMIT]; // ...
    bool noMarks;
    bool hasCarryIndex;
    uint16_t carryIndex;
    uint32_t allocedMarkCount;
    uint32_t freedMarkCount;
};

struct FX_ImpactMark_Generate_CB
{
    int32_t localClientNum;
    Material *material;
    float radius;
    const byte *nativeColor;
};

extern FxMarksSystem fx_marksSystemPool[1];

inline FxMarksSystem *FX_GetMarksSystem(int clientIndex)
{
    iassert(clientIndex == 0); // line 139
    return &fx_marksSystemPool[clientIndex];
}