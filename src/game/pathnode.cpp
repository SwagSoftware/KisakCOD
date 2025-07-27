#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "pathnode.h"
#include "g_scr_main.h"
#include <script/scr_vm.h>
#include "game_public.h"
#include "g_main.h"
#include "g_local.h"
#include "g_bsp.h"
#include <script/scr_const.h>
#include <server/sv_game.h>
#include "savememory.h"
#include "actor_senses.h"

//Line 37928:  0006 : 00006700       char const **nodeStringTable  82696700     pathnode.obj

path_t *g_pPath;
path_t some_path; // idk name

node_field_t fields_3[12] =
{
  { "targetname", 6, F_STRING, NULL },
  { "target", 12, F_STRING, NULL },
  { "animscript", 14, F_STRING, NULL },
  { "script_linkname", 8, F_STRING, NULL },
  { "script_noteworthy", 10, F_STRING, NULL },
  { "origin", 20, F_VECTOR, NULL },
  { "angles", 32, F_VECTORHACK, NULL },
  { "radius", 44, F_FLOAT, NULL },
  { "minusedistsq", 48, F_FLOAT, NULL },
  { "spawnflags", 4, F_SHORT, NULL },
  { "type", 0, F_INT, &Path_GetType },
  { NULL, 0, F_INT, NULL }
};

struct node_field_t
{
    const char *name;
    int ofs;
    fieldtype_t type;
    void(*getter)(struct pathnode_t *, int);
};

pathlocal_t g_path;

const float nodeColorTable[20][4] =
{
  { 1.0f, 0.0f, 0.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f, 1.0f },
  { 0.0f, 0.54f, 0.66f, 1.0f },
  { 0.0f, 0.93f, 0.72f, 1.0f },
  { 0.0f, 0.7f, 0.5f, 1.0f },
  { 0.0f, 0.6f, 0.46f, 1.0f },
  { 0.85f, 0.85f, 0.1f, 1.0f },
  { 1.0f, 0.7f, 0.0f, 1.0f },
  { 0.75f, 0.75f, 0.0f, 1.0f },
  { 0.75f, 0.53f, 0.38f, 1.0f },
  { 0.0f, 0.0f, 1.0f, 1.0f },
  { 0.0f, 0.0f, 0.75f, 1.0f },
  { 0.0f, 0.0f, 0.5f, 1.0f },
  { 0.52f, 0.52f, 0.6f, 1.0f },
  { 0.5f, 0.5f, 0.0f, 1.0f },
  { 0.72f, 0.72f, 0.83f, 1.0f },
  { 0.5f, 0.6f, 0.5f, 1.0f },
  { 0.6f, 0.5f, 0.5f, 1.0f },
  { 0.0f, 0.93f, 0.72f, 1.0f },
  { 0.7f, 0.0f, 0.0f, 1.0f }
};

path_t debugPathBuf;

NodeTypeToName priorityAllowedNodes[9] =
{
  { NODE_TURRET, "NODE_TURRET" },
  { NODE_COVER_STAND, "NODE_COVER_STAND" },
  { NODE_COVER_CROUCH, "NODE_COVER_CROUCH" },
  { NODE_COVER_PRONE, "NODE_COVER_PRONE" },
  { NODE_CONCEALMENT_STAND, "NODE_CONCEALMENT_STAND" },
  { NODE_CONCEALMENT_CROUCH, "NODE_CONCEALMENT_CROUCH" },
  { NODE_CONCEALMENT_PRONE, "NODE_CONCEALMENT_PRONE" },
  { NODE_COVER_RIGHT, "NODE_COVER_RIGHT" },
  { NODE_COVER_LEFT, "NODE_COVER_LEFT" }
};

void __cdecl TRACK_pathnode()
{
    track_static_alloc_internal(&g_path, 16512, "g_path", 6);
    track_static_alloc_internal((void *)nodeColorTable, 320, "nodeColorTable", 5);
    track_static_alloc_internal(nodeStringTable, 80, "nodeStringTable", 5);
    track_static_alloc_internal(&debugPathBuf, 996, "debugPathBuf", 0);
}

int __cdecl NodeTypeCanHavePriority(nodeType type)
{
    unsigned int v1; // r10
    NodeTypeToName *i; // r11

    v1 = 0;
    for (i = priorityAllowedNodes; type != i->type; ++i)
    {
        v1 += 8;
        if (v1 >= 0x48)
            return 0;
    }
    return 1;
}

void __cdecl TurretNode_GetAngles(const pathnode_t *node, float *angleMin, float *angleMax)
{
    int turretEntNumber; // r11
    gentity_s *v7; // r29

    iassert(node);
    iassert(angleMin);
    iassert(angleMax);
    iassert(node->constant.type == NODE_TURRET);
    
    turretEntNumber = node->dynamic.turretEntNumber;
    if (turretEntNumber >= 0)
    {
        if (turretEntNumber >= 2176)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                525,
                0,
                "%s",
                "node->dynamic.turretEntNumber < MAX_GENTITIES");
        v7 = &g_entities[node->dynamic.turretEntNumber];
        if (!v7->pTurretInfo)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 527, 0, "%s", "ent->pTurretInfo");
        *angleMin = AngleNormalize360(v7->pTurretInfo->arcmin[1]);
        *angleMax = AngleNormalize360(v7->pTurretInfo->arcmax[1]);
    }
    else
    {
        *angleMin = 315.0;
        *angleMax = 45.0;
    }
}

bool __cdecl TurretNode_HasTurret(const pathnode_t *node)
{
    int turretEntNumber; // r11

    iassert(node);
    iassert(node->constant.type == NODE_TURRET);

    turretEntNumber = node->dynamic.turretEntNumber;

    if (turretEntNumber < 0)
        return 0;

    iassert(node->dynamic.turretEntNumber < MAX_GENTITIES);

    //return (_cntlzw((unsigned int)g_entities[node->dynamic.turretEntNumber].pTurretInfo) & 0x20) == 0;

    return g_entities[node->dynamic.turretEntNumber].pTurretInfo != 0;
}

void __cdecl Path_ReadOnly(int offset)
{
    const char *v1; // r3

    v1 = va("pathnode property '%s' is read-only", fields_3[offset].name);
    Scr_Error(v1);
}

void __cdecl Path_NonNegativeFloat(pathnode_t *node, int offset)
{
    double Float; // fp31
    const char *v5; // r3
    const char *v6; // r3

    Float = Scr_GetFloat(0);
    if (Float < 0.0)
    {
        Float = 0.0;
        if (node->constant.targetname)
        {
            v5 = va("pathnode property '%s' must be non-negative\n", fields_3[offset].name);
        }
        else
        {
            v6 = SL_ConvertToString(0);
            v5 = va("pathnode property '%s' must be non-negative for pathnode '%s'\n", fields_3[offset].name, v6);
        }
        Scr_Error(v5);
    }
    *(float *)((char *)&node->constant.type + fields_3[offset].ofs) = Float;
}

void __cdecl Path_GetType(pathnode_t *node, int offset)
{
    if (node->constant.type >= (unsigned int)NODE_NUMTYPES)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            577,
            0,
            "%s",
            "node->constant.type >= NODE_BADNODE && node->constant.type < NODE_NUMTYPES");
    Scr_AddString(nodeStringTable[node->constant.type]);
}

void __cdecl Scr_SetPathnodeField(unsigned int entnum, unsigned int offset)
{
    const char *v4; // r3

    if (offset >= 0xB)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            584,
            0,
            "%s",
            "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
    if (entnum >= 0x2000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            585,
            0,
            "%s",
            "(unsigned)entnum < PATH_MAX_NODES");
    v4 = va("pathnode property '%s' is read-only", fields_3[offset].name);
    Scr_Error(v4);
}

void __cdecl Scr_GetPathnodeField(unsigned int entnum, unsigned int offset)
{
    node_field_t *v4; // r11
    unsigned __int8 *v5; // r3
    void(__cdecl * getter)(pathnode_t *, int); // r10

    if (offset >= 0xB)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            596,
            0,
            "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
            offset,
            11);
    if (entnum >= g_path.actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            597,
            0,
            "entnum doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            entnum,
            g_path.actualNodeCount);
    v4 = &fields_3[offset];
    v5 = (unsigned __int8 *)&gameWorldSp.path.nodes[entnum];
    getter = v4->getter;
    if (getter)
        getter((pathnode_t *)v5, offset);
    else
        Scr_GetGenericField(v5, v4->type, v4->ofs);
}

void __cdecl PathNode_ClearStringField(unsigned __int16 *destScrString)
{
    unsigned int v2; // r3

    v2 = *destScrString;
    if (v2)
    {
        if ((SL_GetUser(v2) & 1) != 0)
            *destScrString = 0;
    }
}

void __cdecl PathNode_UpdateStringField(
    const char *destKey,
    unsigned __int16 *destScrString,
    const char *key,
    const char *value)
{
    const char *v8; // r3

    if (!key)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 686, 0, "%s", "key");
    if (!destKey)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 687, 0, "%s", "destKey");
    if (!I_stricmp(key, destKey))
    {
        v8 = SL_ConvertToString(*destScrString);
        if (!v8 || I_stricmp(v8, value))
            *destScrString = SL_GetString(value, 1u);
    }
}

void __cdecl PathNode_UpdateFloatField(const char *destKey, float *destFloat, const char *key, const char *value)
{
    float v8; // [sp+50h] [-40h] BYREF

    if (!key)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 705, 0, "%s", "key");
    if (!destKey)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 706, 0, "%s", "destKey");
    if (!I_stricmp(key, destKey))
    {
        if (sscanf(value, "%f", &v8) != 1)
            Com_Error(ERR_DROP, "Malformed '%s' float field for node '%s'\n", key, value);
        *destFloat = v8;
    }
}

void __cdecl PathNode_OriginMatches(const char *value, const float *nodeOrigin)
{
    float v4; // [sp+50h] [-30h] BYREF
    float v5; // [sp+54h] [-2Ch] BYREF
    char v6; // [sp+58h] [-28h] BYREF

    if (sscanf(value, "%f %f %f", &v4, &v5, &v6) != 3)
        Com_Error(ERR_DROP, "Malformed origin for path node '%s'\n", value);
    if (v4 != *nodeOrigin || v5 != nodeOrigin[1])
        ++g_path.originErrors;
}

void __cdecl node_droptofloor(pathnode_t *node)
{
    double v2; // fp29
    double v3; // fp31
    double v4; // fp30
    double v5; // fp13
    double v6; // fp12
    float v7; // [sp+50h] [-B0h] BYREF
    float v8; // [sp+54h] [-ACh]
    float v9; // [sp+58h] [-A8h]
    float v10; // [sp+60h] [-A0h] BYREF
    float v11; // [sp+64h] [-9Ch]
    float v12; // [sp+68h] [-98h]
    float v13[4]; // [sp+70h] [-90h] BYREF
    float v14[4]; // [sp+80h] [-80h] BYREF
    trace_t v15; // [sp+90h] [-70h] BYREF

    v2 = node->constant.vOrigin[2];
    v14[0] = 15.0;
    v3 = node->constant.vOrigin[0];
    v14[1] = 15.0;
    v13[0] = -15.0;
    v4 = node->constant.vOrigin[1];
    v13[2] = 0.0;
    v13[1] = -15.0;
    v10 = v3;
    v11 = v4;
    v14[2] = (float)((float)15.0 - (float)-15.0) + (float)0.0;
    v12 = (float)v2 - (float)256.0;
    v7 = v3;
    v8 = v4;
    v9 = (float)v2 + (float)1.0;
    G_TraceCapsule(&v15, &v7, v13, v14, &v10, 2175, 8519697);
    if (v15.startsolid || v15.allsolid)
    {
    LABEL_8:
        Com_PrintError(
            1,
            "ERROR: Pathnode (%s) at (%g %g %g) is in solid\n",
            nodeStringTable[node->constant.type],
            v3,
            v4,
            v2);
        goto LABEL_9;
    }
    if (v15.fraction != 1.0)
    {
        v7 = (float)((float)(v10 - v7) * v15.fraction) + v7;
        v8 = (float)((float)(v11 - v8) * v15.fraction) + v8;
        v9 = (float)((float)(v12 - v9) * v15.fraction) + v9;
        G_TraceCapsule(&v15, &v7, actorMins, actorMaxs, &v7, 2175, 8519697);
        if (!v15.startsolid && !v15.allsolid)
        {
            v5 = v8;
            v6 = v9;
            node->constant.vOrigin[0] = v7;
            node->constant.vOrigin[1] = v5;
            node->constant.vOrigin[2] = v6;
            return;
        }
        goto LABEL_8;
    }
    Com_PrintError(
        1,
        "ERROR: Pathnode (%s) at (%g %g %g) is floating\n",
        nodeStringTable[node->constant.type],
        v3,
        v4,
        v2);
LABEL_9:
    node->constant.type = NODE_BADNODE;
}

void __cdecl G_UpdateTrackExtraNodes()
{
    unsigned int nodeCount; // r11

    if (g_path.originErrors)
        Com_PrintError(
            1,
            "There are %d path node origins that don't match.  If this number is higher than expected then you may want to reco"
            "mpile the map before using MyMapEnts..\n",
            g_path.originErrors);
    if (g_path.extraNodes)
        Com_PrintError(
            18,
            "There are %d extra path nodes in the entity string.  Ignoring extra path nodes, and some of the key value pairs ar"
            "e likely mapped to the wrong nodes.\n",
            g_path.extraNodes);
    nodeCount = gameWorldSp.path.nodeCount;
    if (g_path.actualNodeCount < gameWorldSp.path.nodeCount)
    {
        Com_PrintError(
            18,
            "There %d less path nodes in the entity string than in the compiled map.  Some of the key value pairs are likely ma"
            "pped to the wrong nodes.\n",
            gameWorldSp.path.nodeCount - g_path.actualNodeCount);
        nodeCount = gameWorldSp.path.nodeCount;
    }
    g_path.actualNodeCount = nodeCount;
}

void __cdecl GScr_AddFieldsForPathnode()
{
    node_field_t *v0; // r30
    int v1; // r29

    v0 = fields_3;
    if (fields_3[0].name)
    {
        v1 = 0;
        do
        {
            if (v1 >> 4 != (unsigned __int16)(v1 >> 4))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                    977,
                    0,
                    "%s",
                    "(f - fields) == (unsigned short)( f - fields )");
            Scr_AddClassField(2u, (char*)v0->name, (unsigned __int16)(v1 >> 4));
            ++v0;
            v1 += 16;
        } while (v0->name);
    }
}

pathnode_t *__cdecl Scr_GetPathnode(unsigned int index)
{
    scr_entref_t entref; // [sp+50h] [-20h]

    entref = Scr_GetEntityRef(index);

    if (entref.classnum == 2)
    {
        bcassert(entref.entnum, g_path.actualNodeCount);

        return &gameWorldSp.path.nodes[entref.entnum];
    }
    else
    {
        Scr_ParamError((unsigned int)index, "not a pathnode");
        return 0;
    }
}

void __cdecl G_FreePathnodesScriptInfo()
{
    unsigned int nodeIndex; // r31

    Path_ShutdownBadPlaces();
    nodeIndex = 0;
    if (g_path.actualNodeCount)
    {
        do
        {
            iassert(!gameWorldSp.path.nodes[nodeIndex].dynamic.pOwner.isDefined());
            Scr_FreeEntityNum(nodeIndex++, 2);
        } while (nodeIndex < g_path.actualNodeCount);
    }
}

bool __cdecl Path_CompareNodesIncreasing(const pathsort_t *ps1, const pathsort_t *ps2)
{
    if (ps1->node->dynamic.wLinkCount)
    {
        if (!ps2->node->dynamic.wLinkCount)
            return 1;
    }
    else if (ps2->node->dynamic.wLinkCount)
    {
        return 0;
    }
    return ps1->metric < (double)ps2->metric;
}

unsigned int __cdecl Path_ConvertNodeToIndex(const pathnode_t *node)
{
    unsigned int v2; // r31

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1175, 0, "%s", "node");
    v2 = node - gameWorldSp.path.nodes;
    if (v2 >= g_path.actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            1178,
            0,
            "nodeIndex doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            v2,
            g_path.actualNodeCount);
    return v2;
}

pathnode_t *__cdecl Path_ConvertIndexToNode(unsigned int index)
{
    if (index >= g_path.actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            1185,
            0,
            "index doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            index,
            g_path.actualNodeCount);
    return &gameWorldSp.path.nodes[index];
}

unsigned int __cdecl Path_NodeCount()
{
    return g_path.actualNodeCount;
}

void __cdecl Path_Init(int restart)
{
    memset(&g_path, 0, sizeof(g_path));
    if (!gameWorldSp.path.nodes)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1210, 0, "%s", "gameWorldSp.path.nodes");
    g_pPath = 0;
    Path_InitBadPlaces();
}

int __cdecl NodeVisCacheEntry(int i, int j)
{
    if (i >= j)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1255, 0, "%s", "i < j");
    return g_path.actualNodeCount * i + j;
}

int __cdecl ExpandedNodeVisCacheEntry(int i, int j)
{
    if (i <= j)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1262, 0, "%s", "i > j");
    return (i - 1) * g_path.actualNodeCount + j;
}

void __cdecl Path_NodesInCylinder_r(pathnode_tree_t *tree)
{
    pathnode_tree_t *v1; // r18
    int axis; // r11
    double maxDist; // fp13
    double v4; // fp0
    int v5; // r17
    unsigned __int16 *nodes; // r27
    int v7; // r11
    int v8; // r28
    float *v9; // r10
    float *v10; // r29
    double v11; // fp31
    int v12; // r11
    int v13; // r11

    v1 = tree;
    axis = tree->axis;
    if (tree->axis >= 0)
    {
        maxDist = g_path.circle.maxDist;
        while (1)
        {
            v4 = (float)(g_path.circle.origin[axis] - v1->dist);
            if (v4 > maxDist)
                goto LABEL_7;
            if (v4 >= -maxDist)
                break;
            v1 = v1->u.child[0];
        LABEL_8:
            axis = v1->axis;
            if (v1->axis < 0)
                goto LABEL_9;
        }
        Path_NodesInCylinder_r(v1->u.child[0]);
        maxDist = g_path.circle.maxDist;
    LABEL_7:
        v1 = v1->u.child[1];
        goto LABEL_8;
    }
LABEL_9:
    v5 = 0;
    if (v1->u.s.nodeCount > 0)
    {
        nodes = v1->u.s.nodes;
        do
        {
            v7 = *nodes;
            v8 = __ROL4__(v7, 7);
            v9 = (float *)((char *)gameWorldSp.path.nodes + v8);
            v10 = (float *)((char *)gameWorldSp.path.basenodes->vOrigin + __ROL4__(v7, 4));
            if (*v10 != *(float *)((char *)gameWorldSp.path.nodes->constant.vOrigin + v8)
                || v10[1] != v9[6]
                || v10[2] != v9[7])
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                    1301,
                    0,
                    "%s",
                    "Vec3Compare( node->vOrigin, gameWorldSp.path.nodes[leafNodes[i]].constant.vOrigin )");
            }
            v11 = (float)((float)((float)(v10[1] - g_path.circle.origin[1]) * (float)(v10[1] - g_path.circle.origin[1]))
                + (float)((float)(*v10 - g_path.circle.origin[0]) * (float)(*v10 - g_path.circle.origin[0])));
            if (v11 <= g_path.circle.maxDistSq)
            {
                v12 = (1 << *(nodeType *)((char *)&gameWorldSp.path.nodes->constant.type + __ROL4__(*nodes, 7)))
                    & g_path.circle.typeFlags;
                if (((unsigned int)v10[3] & g_path.circle.typeFlags) != 0)
                {
                    if (!v12)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                            1314,
                            0,
                            "%s",
                            "g_path.circle.typeFlags & (1 << gameWorldSp.path.nodes[leafNodes[i]].constant.type)");
                    v13 = *(unsigned __int16 *)((_BYTE *)&gameWorldSp.path.nodes->constant.spawnflags + __ROL4__(*nodes, 7)) & 1;
                    if (((unsigned int)v10[3] & 0x100000) == 0)
                    {
                        if (v13)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                                1326,
                                0,
                                "%s",
                                "!(gameWorldSp.path.nodes[leafNodes[i]].constant.spawnflags & PNF_DONTLINK)");
                    LABEL_28:
                        if ((float)((float)(v10[2] - g_path.circle.origin[2]) * (float)(v10[2] - g_path.circle.origin[2])) <= (double)g_path.circle.maxHeightSq)
                        {
                            if (g_path.circle.nodeCount == g_path.circle.maxNodes)
                                return;
                            g_path.circle.nodes[g_path.circle.nodeCount].node = (pathnode_t *)((char *)gameWorldSp.path.nodes + v8);
                            g_path.circle.nodes[g_path.circle.nodeCount++].metric = v11;
                        }
                        goto LABEL_31;
                    }
                    if (!v13)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                            1318,
                            0,
                            "%s",
                            "gameWorldSp.path.nodes[leafNodes[i]].constant.spawnflags & PNF_DONTLINK");
                    if ((g_path.circle.typeFlags & 1) != 0)
                        goto LABEL_28;
                }
                else if (v12)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                        1310,
                        0,
                        "%s",
                        "!(g_path.circle.typeFlags & (1 << gameWorldSp.path.nodes[leafNodes[i]].constant.type))");
                }
            }
        LABEL_31:
            ++v5;
            ++nodes;
        } while (v5 < v1->u.s.nodeCount);
    }
}

int __cdecl Path_NodesInCylinder(
    float *origin,
    double maxDist,
    double maxHeight,
    pathsort_t *nodes,
    int maxNodes,
    int typeFlags)
{
    if (gameWorldSp.path.nodeTree)
    {
        g_path.circle.origin[0] = origin[0];
        g_path.circle.origin[1] = origin[1];
        g_path.circle.origin[2] = origin[2];
        g_path.circle.maxDist = maxDist;
        g_path.circle.maxDistSq = (float)maxDist * (float)maxDist;
        g_path.circle.typeFlags = typeFlags;
        g_path.circle.maxHeightSq = (float)maxHeight * (float)maxHeight;
        g_path.circle.nodes = nodes;
        g_path.circle.maxNodes = maxNodes;
        g_path.circle.nodeCount = 0;
        Path_NodesInCylinder_r(gameWorldSp.path.nodeTree);
        return g_path.circle.nodeCount;
    }

    return 0;
}

int __cdecl Path_NodesInRadius(
    float *origin,
    double maxDist,
    pathsort_t *nodes,
    int maxNodes,
    int typeFlags)
{
    return Path_NodesInCylinder(origin, maxDist, 1000000000.0, nodes, maxNodes, typeFlags);
}

int __cdecl Path_IsDynamicBlockingEntity(gentity_s *ent)
{
    return ent->flags & 0x100;
}

bool __cdecl Path_IsBadPlaceLink(unsigned int nodeNumFrom, unsigned int nodeNumTo, team_t eTeam)
{
    unsigned int actualNodeCount; // r8
    unsigned int v7; // r11
    pathnode_t *v8; // r10
    unsigned int totalLinkCount; // r9
    pathlink_s *Links; // r8
    unsigned __int16 *p_nodeNum; // r10
    const char *v12; // r3

    actualNodeCount = g_path.actualNodeCount;
    if (nodeNumFrom >= g_path.actualNodeCount)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            1381,
            0,
            "nodeNumFrom doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            nodeNumFrom,
            g_path.actualNodeCount);
        actualNodeCount = g_path.actualNodeCount;
    }
    if (nodeNumTo >= actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            1382,
            0,
            "nodeNumTo doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            nodeNumTo,
            actualNodeCount);
    if ((unsigned int)eTeam >= TEAM_DEAD)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            1383,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4);
    v7 = 0;
    v8 = &gameWorldSp.path.nodes[nodeNumFrom];
    totalLinkCount = v8->constant.totalLinkCount;
    if (v8->constant.totalLinkCount)
    {
        Links = v8->constant.Links;
        p_nodeNum = &Links->nodeNum;
        while (*p_nodeNum != nodeNumTo)
        {
            ++v7;
            p_nodeNum += 6;
            if (v7 >= totalLinkCount)
                goto LABEL_11;
        }
        //return (_cntlzw(Links[v7].ubBadPlaceCount[eTeam]) & 0x20) == 0;
        return Links[v7].ubBadPlaceCount[eTeam] != 0;
    }
    else
    {
    LABEL_11:
        if (!alwaysfails)
        {
            v12 = va("Path_IsBadPlaceLink called from %i to %i, but there is no such link", nodeNumFrom, nodeNumTo);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1392, 0, v12);
        }
        return 0;
    }
}

unsigned int Path_InitLinkCounts()
{
    unsigned int result; // r3
    pathnode_t *nodes; // r7
    int v2; // r4
    unsigned int v3; // r6
    int v4; // r11
    int v5; // r9
    int v6; // r7

    result = 0;
    if (gameWorldSp.path.nodeCount)
    {
        nodes = gameWorldSp.path.nodes;
        v2 = 0;
        do
        {
            v3 = 0;
            nodes[v2].dynamic.wLinkCount = nodes[v2].constant.totalLinkCount;
            nodes = gameWorldSp.path.nodes;
            if (gameWorldSp.path.nodes[v2].constant.totalLinkCount)
            {
                v4 = 0;
                v5 = v2 * 128 + 64;
                do
                {
                    ++v3;
                    *(_BYTE *)(*(nodeType *)((char *)&nodes->constant.type + v5) + v4 + 8) = 0;
                    *(_BYTE *)(*(nodeType *)((char *)&gameWorldSp.path.nodes->constant.type + v5) + v4 + 9) = 0;
                    *(_BYTE *)(*(nodeType *)((char *)&gameWorldSp.path.nodes->constant.type + v5) + v4 + 10) = 0;
                    v6 = *(nodeType *)((char *)&gameWorldSp.path.nodes->constant.type + v5) + v4;
                    v4 += 12;
                    *(_BYTE *)(v6 + 11) = 0;
                    nodes = gameWorldSp.path.nodes;
                } while (v3 < gameWorldSp.path.nodes[v2].constant.totalLinkCount);
            }
            ++result;
            ++v2;
        } while (result < gameWorldSp.path.nodeCount);
    }
    return result;
}

void Path_InitLinkInfoArray()
{
    int i; // r11
    unsigned __int16 v1; // r10
    int v2; // r9
    unsigned __int16 v3; // r10

    g_path.pathLinkInfoArrayInited = 1;
    for (i = 0; i < 2048; ++i)
    {
        v1 = i - 1;
        if (!i)
            v1 = 2047;
        v2 = i;
        g_path.pathLinkInfoArray[i].prev = v1;
        v3 = i + 1;
        if (i == 2047)
            v3 = 0;
        g_path.pathLinkInfoArray[v2].next = v3;
    }
}

void __cdecl Path_InitNodeDynamic(pathnode_t *loadNode)
{
    pathnode_dynamic_t *p_dynamic; // r11
    int v3; // ctr
    pathnode_transient_t *p_transient; // r11
    int v5; // ctr
    double v6; // fp13
    float v7[4]; // [sp+50h] [-20h] BYREF

    p_dynamic = &loadNode->dynamic;
    v3 = 8;
    do
    {
        p_dynamic->pOwner = (SentientHandle)0;
        p_dynamic = (pathnode_dynamic_t *)((char *)p_dynamic + 4);
        --v3;
    } while (v3);
    p_transient = &loadNode->transient;
    v5 = 7;
    do
    {
        p_transient->iSearchFrame = 0;
        p_transient = (pathnode_transient_t *)((char *)p_transient + 4);
        --v5;
    } while (v5);
    YawVectors(loadNode->constant.fAngle, (float *)loadNode, v7);
    v6 = v7[1];
    loadNode->constant.forward[0] = v7[0];
    loadNode->constant.forward[1] = v6;
    loadNode->dynamic.turretEntNumber = -1;
}

void __cdecl Path_InitNodesDynamic(float *a1)
{
    unsigned int v1; // r28
    int v2; // r30
    pathnode_t *v3; // r31
    pathnode_dynamic_t *p_dynamic; // r11
    int v5; // ctr
    pathnode_transient_t *p_transient; // r11
    int v7; // ctr
    float v8[16]; // [sp+50h] [-40h] BYREF

    v1 = 0;
    if (gameWorldSp.path.nodeCount)
    {
        v2 = 0;
        do
        {
            v3 = &gameWorldSp.path.nodes[v2];
            p_dynamic = &gameWorldSp.path.nodes[v2].dynamic;
            v5 = 8;
            do
            {
                p_dynamic->pOwner = (SentientHandle)0;
                p_dynamic = (pathnode_dynamic_t *)((char *)p_dynamic + 4);
                --v5;
            } while (v5);
            p_transient = &v3->transient;
            v7 = 7;
            do
            {
                p_transient->iSearchFrame = 0;
                p_transient = (pathnode_transient_t *)((char *)p_transient + 4);
                --v7;
            } while (v7);
            YawVectors(v3->constant.fAngle, a1, v8);
            v3->constant.forward[0] = v8[0];
            ++v1;
            ++v2;
            v3->constant.forward[1] = v8[1];
            v3->dynamic.turretEntNumber = -1;
        } while (v1 < gameWorldSp.path.nodeCount);
    }
}

void __cdecl Path_PreSpawnInitPaths(float *a1)
{
    Path_InitNodesDynamic(a1);
    Path_InitLinkCounts();
}

void __cdecl Path_DrawDebugNoLinks(const pathnode_t *node, const float (*color)[4], int duration)
{
    double v5; // fp31
    double v6; // fp30
    double v7; // fp29
    float v8; // [sp+50h] [-60h] BYREF
    float v9; // [sp+54h] [-5Ch]
    float v10; // [sp+58h] [-58h]
    float v11; // [sp+60h] [-50h] BYREF
    float v12; // [sp+64h] [-4Ch]
    float v13; // [sp+68h] [-48h]

    v5 = node->constant.vOrigin[0];
    v6 = node->constant.vOrigin[1];
    v7 = (float)((float)(node->constant.vOrigin[2] + (float)0.0) + (float)1.0);
    v10 = (float)(node->constant.vOrigin[2] + (float)0.0) + (float)1.0;
    v8 = (float)v5 + (float)6.9282031;
    v13 = v7;
    v9 = (float)v6 + (float)4.0;
    v11 = (float)v5 - (float)6.9282031;
    v12 = (float)v6 - (float)4.0;
    G_DebugLineWithDuration(&v8, &v11, (const float *)color, 0, duration);
    v8 = (float)v5 - (float)4.0;
    v9 = (float)v6 + (float)6.9282031;
    v11 = (float)v5 + (float)4.0;
    v10 = v7;
    v12 = (float)v6 - (float)6.9282031;
    v13 = v7;
    G_DebugLineWithDuration(&v8, &v11, (const float *)color, 0, duration);
}


// aislop
void Path_DrawDebugLink(const pathnode_t *node, const int i, bool bShowAll)
{
    pathlink_s *link = &node->constant.Links[i];
    int nodeNum = link->nodeNum;

    float start[3] =
    {
        node->constant.vOrigin[0],
        node->constant.vOrigin[1],
        node->constant.vOrigin[2] + 16.0f
    };

    float *destNode = (float *)((char *)gameWorldSp.path.nodes + (nodeNum << 7));

    float end[3] =
    {
        destNode[5],
        destNode[6],
        destNode[7] + 16.0f
    };

    int linkCount = bShowAll ? *((uint16_t *)destNode + 31) : *((int16_t *)destNode + 46);
    int found = 0;

    if (linkCount > 0)
    {
        uint16_t *linkedNodes = (uint16_t *)((char *)destNode + 64); // Offset assumption
        for (int j = 0; j < linkCount; ++j)
        {
            const pathnode_t *linked = (const pathnode_t *)((char *)gameWorldSp.path.nodes + (linkedNodes[j * 3] << 7));
            if (linked == node)
            {
                found = 1;
                break;
            }
        }
    }

    const float *lineColor = colorRed;

    if (!found)
    {
        for (int j = 3; j >= 0; --j)
        {
            if (link->ubBadPlaceCount[j])
            {
                lineColor = colorMagenta;
                break;
            }
        }

        G_DebugLine(start, end, lineColor, 1);

        float mid[3] =
        {
            0.5f * (start[0] + end[0]),
            0.5f * (start[1] + end[1]),
            0.5f * (start[2] + end[2])
        };

        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float dz = end[2] - start[2];

        float length = sqrtf(dx * dx + dy * dy + dz * dz);

        if (length != 0.0f)
        {
            float invLen = 1.0f / length;
            float dir[3] =
            {
                dx * invLen,
                dy * invLen,
                dz * invLen
            };

            float arrowStart[3] =
            {
                mid[0] + dir[0] * 8.0f,
                mid[1] + dir[1] * 8.0f,
                mid[2] + dir[2] * 8.0f
            };

            float arrowEnd[3] =
            {
                mid[0] + dir[0] * 8.0f,
                mid[1] + dir[1] * 8.0f,
                mid[2] - dir[2] * 8.0f
            };

            G_DebugLine(arrowStart, mid, colorRed, 0);
            G_DebugLine(mid, arrowEnd, colorRed, 0);
        }

        return;
    }

    int flags = 0;

    if (node > (const pathnode_t *)destNode)
    {
        for (int j = 3; j >= 0; --j)
        {
            if (link->ubBadPlaceCount[j])
            {
                flags |= 1;
                break;
            }
        }

        uint8_t *badLinkData = (uint8_t *)((uint8_t *)destNode + 64 + 12 * i + 11);
        for (int j = 3; j >= 0; --j)
        {
            if (badLinkData[-j])
            {
                flags |= 2;
                break;
            }
        }
    }

    const float *color;

    if (flags == 0)
    {
        color = colorCyan;
    }
    else if (flags == 3)
    {
        color = colorYellow;
    }
    else
    {
        color = colorBlue;
    }

    G_DebugLine(start, end, color, 1);

    if (flags == 1 || flags == 2)
    {
        float mid[3] =
        {
            0.5f * (start[0] + end[0]),
            0.5f * (start[1] + end[1]),
            0.5f * (start[2] + end[2])
        };

        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float dz = end[2] - start[2];

        float length = sqrtf(dx * dx + dy * dy + dz * dz);

        if (length != 0.0f)
        {
            float invLen = 1.0f / length;
            float dir[3] =
            {
                dx * invLen,
                dy * invLen,
                dz * invLen
            };

            if (flags == 1)
            {
                dir[0] = -dir[0];
                dir[1] = -dir[1];
                dir[2] = -dir[2];
            }

            float arrowStart[3] =
            {
                mid[0] + dir[0] * 8.0f,
                mid[1] + dir[1] * 8.0f,
                mid[2] + dir[2] * 8.0f
            };

            float arrowEnd[3] =
            {
                mid[0] + dir[0] * 8.0f,
                mid[1] + dir[1] * 8.0f,
                mid[2] - dir[2] * 8.0f
            };

            G_DebugLine(arrowStart, mid, colorBlue, 1);
            G_DebugLine(mid, arrowEnd, (flags == 1 ? colorBlue : colorRed), 1);
        }
    }
}


float __cdecl Path_GetDebugStringScale(const float *cameraPos, const float *origin)
{
    gentity_s *v4; // r3
    double v5; // fp13
    double v6; // fp12
    double v7; // fp1

    if (!cameraPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1662, 0, "%s", "cameraPos");
    if (!origin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1663, 0, "%s", "origin");
    v4 = G_Find(0, 284, scr_const.player);
    if (v4)
    {
        v5 = (float)(cameraPos[1] - origin[1]);
        v6 = (float)((float)(cameraPos[2] - origin[2]) + v4->client->ps.viewHeightCurrent);
        //v7 = (float)((float)__fsqrts((float)((float)((float)v6 * (float)v6)
        v7 = (float)((float)sqrtf((float)((float)((float)v6 * (float)v6)
            + (float)((float)((float)(*cameraPos - *origin) * (float)(*cameraPos - *origin))
                + (float)((float)v5 * (float)v5))))
            * (float)0.0022222223);
    }
    else
    {
        v7 = 1.0;
    }
    return *((float *)&v7 + 1);
}

void __cdecl Path_DrawDebugNodeBox(const pathnode_t *node)
{
    long double v5; // fp2
    double v6; // fp31
    double v7; // fp12
    double v8; // fp13
    long double v9; // fp2
    double v10; // fp0
    double v11; // fp31
    long double v12; // fp2
    nodeType type; // r11
    float maxs[3]; // [sp+50h] [-70h] BYREF
    float mins[3]; // [sp+60h] [-60h] BYREF
    float v20; // [sp+70h] [-50h] BYREF
    float v21; // [sp+74h] [-4Ch]
    float v22; // [sp+78h] [-48h]
    float v23[6]; // [sp+80h] [-40h] BYREF

    mins[0] = -16.0f;
    mins[1] = -16.0f;
    mins[2] = 0.0f;

    maxs[0] = 16.0f;
    maxs[1] = 16.0f;
    maxs[2] = 16.0f;

    iassert(node);

    G_DebugBox(node->constant.vOrigin, mins, maxs, node->constant.fAngle, nodeColorTable[node->constant.type], 1, 1);
    if ((node->constant.spawnflags & 0x8000) != 0)
    {
        v6 = (float)(node->constant.fAngle * (float)0.017453292);
        *(double *)&v5 = v6;
        v7 = (float)((float)(node->constant.vOrigin[2] + maxs[2]) + (float)(node->constant.vOrigin[2] + mins[2]));
        v8 = (float)((float)((float)(node->constant.vOrigin[1] + maxs[1]) + (float)(node->constant.vOrigin[1] + mins[1]))
            * (float)0.5);
        v20 = (float)((float)(node->constant.vOrigin[0] + maxs[0]) + (float)(node->constant.vOrigin[0] + mins[0])) * (float)0.5;
        v21 = v8;
        v22 = (float)v7 * (float)0.5;
        v9 = sin(v5);
        v10 = *(double *)&v9;
        *(double *)&v9 = v6;
        v11 = (float)v10;
        v12 = cos(v9);
        type = node->constant.type;
        v23[2] = v22;
        v23[1] = (float)((float)v11 * maxs[0]) + v21;
        v23[0] = (float)((float)*(double *)&v12 * maxs[0]) + v20;
        G_DebugLine(&v20, v23, nodeColorTable[type], 1);
    }
}

void __cdecl Path_DrawDebugNode(const float *cameraPos, const pathnode_t *node)
{
    double DebugStringScale; // fp1
    const char *v7; // r5
    float xyz[16]; // [sp+50h] [-40h] BYREF

    if (!cameraPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1724, 0, "%s", "cameraPos");
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1725, 0, "%s", "node");
    Path_DrawDebugNodeBox(node);
    xyz[0] = node->constant.vOrigin[0];
    xyz[2] = node->constant.vOrigin[2] + 8.0f;
    xyz[1] = node->constant.vOrigin[1];
    DebugStringScale = Path_GetDebugStringScale(cameraPos, xyz);
    G_AddDebugString(xyz, colorYellow, DebugStringScale, nodeStringTable[node->constant.type]);
}

static int iValidBits;
static float vStartPos[3];
static float vGoalPos[3];
static float width;
static float perp[2];

void __cdecl Path_DrawDebugFindPath(float *vOrigin)
{
    int integer; // r11
    int v2; // r11
    path_t *v3; // r3
    double value; // fp0
    float *v5; // r8
    unsigned int actualNodeCount; // r9
    unsigned int v7; // r30
    int v8; // r26
    pathnode_t *v9; // r31
    char *v10; // r3
    const char *v11; // r5
    int v12; // r11
    float v13; // [sp+50h] [-A0h] BYREF
    float v14; // [sp+54h] [-9Ch]
    float v15; // [sp+58h] [-98h] BYREF
    float v16; // [sp+5Ch] [-94h]
    float v17; // [sp+60h] [-90h]
    float v18[4]; // [sp+68h] [-88h] BYREF
    float v19[6]; // [sp+78h] [-78h] BYREF

    integer = ai_debugFindPath->current.integer;
    switch (integer)
    {
    case 0:
        iValidBits = 0;
        return;
    case 1:
    case 4:
    case 5:
        vStartPos[0] = *vOrigin;
        v2 = iValidBits | 1;
        vStartPos[1] = vOrigin[1];
        vStartPos[2] = vOrigin[2];
        goto LABEL_10;
    case 2:
        vGoalPos[0] = *vOrigin;
        v2 = iValidBits | 2;
        vGoalPos[1] = vOrigin[1];
        vGoalPos[2] = vOrigin[2];
    LABEL_10:
        iValidBits = v2;
        goto LABEL_11;
    }
    if (integer != 3)
    {
        Dvar_SetInt(ai_debugFindPath, 0);
        Com_Printf(
            18,
            "^51 continuously copies your position to path start\n"
            "2 continuously copies your position to path goal\n"
            "3 doesn't change path start or path goal, but will still find a path\n"
            "4 continuously moves through the path simulating AI movement\n"
            "5 continuously moves through the path simulating AI movement, but continuously recalculates the path\n"
            "0 resets start and end and disables path finding\n"
            "While there are valid start and goal positions, it will find and draw a path\n");
    }
LABEL_11:
    if (iValidBits != 3 || vStartPos[0] == vGoalPos[0] && vStartPos[1] == vGoalPos[1] && vStartPos[2] == vGoalPos[2])
        return;
    v3 = (path_t *)g_pPath;
    if (!g_pPath)
    {
        g_pPath = &some_path;
        Path_Begin(&some_path);
        Path_FindPath((path_t *)g_pPath, TEAM_FREE, vStartPos, vGoalPos, 1);
        v3 = (path_t *)g_pPath;
    }
    if (ai_debugFindPath->current.integer == 4)
        goto LABEL_36;
    if (ai_debugFindPathDirect->current.enabled)
    {
        if (!Path_FindPath(v3, TEAM_FREE, vStartPos, vGoalPos, 1) || !Path_Exists((const path_t *)g_pPath))
            return;
        goto LABEL_30;
    }
    if (!ai_debugFindPathLock->current.enabled)
    {
        value = ai_debugFindPathWidth->current.value;
        width = ai_debugFindPathWidth->current.value;
        if (value <= 5000.0)
        {
            if (value < -5000.0)
                width = -5000.0;
        }
        else
        {
            width = 5000.0;
        }
        v13 = vStartPos[0] - vGoalPos[0];
        v14 = vStartPos[1] - vGoalPos[1];
        Vec2Normalize(&v13);
        perp[0] = -v14;
        perp[1] = v13;
    }
    v17 = vGoalPos[2];
    v15 = vStartPos[0];
    v16 = vStartPos[1];
    G_DebugLine(&v15, vGoalPos, colorCyan, 0);
    v18[2] = v17;
    v19[2] = v17;
    v18[0] = (float)(v15 - (float)(perp[1] * (float)5000.0)) + (float)(perp[0] * width);
    v18[1] = (float)((float)(perp[0] * (float)5000.0) + v16) + (float)(perp[1] * width);
    v19[0] = (float)((float)(perp[1] * (float)5000.0) + v15) + (float)(perp[0] * width);
    v19[1] = (float)(v16 - (float)(perp[0] * (float)5000.0)) + (float)(perp[1] * width);
    G_DebugLine(v18, v19, colorCyan, 0);
    if (Path_FindPathWithWidth((path_t *)g_pPath, TEAM_FREE, vStartPos, vGoalPos, 1, width, v5, perp)
        && Path_Exists((const path_t *)g_pPath))
    {
    LABEL_30:
        actualNodeCount = g_path.actualNodeCount;
        v7 = 0;
        if (g_path.actualNodeCount)
        {
            v8 = 0;
            do
            {
                v9 = &gameWorldSp.path.nodes[v8];
                if (gameWorldSp.path.nodes[v8].transient.iSearchFrame == level.iSearchFrame)
                {
                    v10 = va("%i", v7);
                    G_AddDebugString(v9->constant.vOrigin, colorWhite, 1.0, v11);
                    actualNodeCount = g_path.actualNodeCount;
                }
                ++v7;
                ++v8;
            } while (v7 < actualNodeCount);
        }
        v3 = (path_t *)g_pPath;
    LABEL_36:
        v12 = ai_debugFindPath->current.integer;
        if (v12 == 4 || v12 == 5)
        {
            Path_UpdateLookahead(v3, vStartPos, 0, 0, 1);
            Path_DebugDraw((path_t *)g_pPath, vStartPos, 1);
        }
        else
        {
            Path_DebugDraw(v3, vStartPos, 0);
        }
    }
}

void __cdecl Path_DrawFriendlyChain()
{
    const float *v0; // r6
    int v1; // r5
    gentity_s *v2; // r3
    gentity_s *v3; // r23
    int integer; // r11
    pathnode_t *pActualChainPos; // r3
    __int16 wChainId; // r11
    int v7; // r21
    unsigned int v8; // r24
    int v9; // r28
    pathnode_t *v10; // r31
    const char *v11; // r3
    char *v12; // r3
    const char *v13; // r5
    const float *v14; // r4
    double fRadius; // fp1
    actor_s *i; // r31
    pathnode_t *pDesiredChainPos; // r11
    float v18[4]; // [sp+50h] [-90h] BYREF
    float v19[4]; // [sp+60h] [-80h] BYREF

    CL_GetViewPos(v18);
    v2 = G_Find(0, 284, scr_const.player);
    v3 = v2;
    if (v2)
    {
        integer = ai_showFriendlyChains->current.integer;
        if (integer == 2)
        {
            pActualChainPos = Sentient_NearestNode(v2->sentient);
        }
        else
        {
            if (integer != 1)
                return;
            pActualChainPos = v2->sentient->pActualChainPos;
        }
        if (pActualChainPos)
        {
            wChainId = pActualChainPos->constant.wChainId;
            v7 = wChainId;
            if (wChainId)
            {
                v8 = 0;
                if (g_path.actualNodeCount)
                {
                    v9 = 0;
                    while (1)
                    {
                        v10 = &gameWorldSp.path.nodes[v9];
                        if (&gameWorldSp.path.nodes[v9] == v3->sentient->pActualChainPos)
                            break;
                        if (v10->constant.wChainId == v7)
                        {
                            Path_DrawDebugNode(v18, &gameWorldSp.path.nodes[v9]);
                            v11 = "%i";
                        LABEL_14:
                            v12 = va(v11, v10->constant.wChainDepth);
                            G_AddDebugString(v10->constant.vOrigin, colorWhite, 1.0, v13);
                            fRadius = v10->constant.fRadius;
                            if (fRadius != 0.0)
                                G_DebugCircle(v10->constant.vOrigin, fRadius, v14, (int)colorWhite, 1, 1);
                        }
                        ++v8;
                        ++v9;
                        if (v8 >= g_path.actualNodeCount)
                            goto LABEL_17;
                    }
                    Path_DrawDebugNode(v18, &gameWorldSp.path.nodes[v9]);
                    v11 = "CURRENT %i";
                    goto LABEL_14;
                }
            LABEL_17:
                for (i = Actor_FirstActor(4); i; i = Actor_NextActor(i, 4))
                {
                    Actor_GetEyePosition(i, v19);
                    pDesiredChainPos = i->pDesiredChainPos;
                    if (pDesiredChainPos)
                        G_DebugLine(v19, pDesiredChainPos->constant.vOrigin, colorCyan, 0);
                }
            }
        }
    }
}

bool __cdecl Path_IsNodeIndex(const pathnode_t *node, unsigned int nodeIndexToCheck)
{
    unsigned int v4; // r31
    pathnode_t *nodes; // r11

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2134, 0, "%s", "node");
    v4 = 0;
    if (g_path.actualNodeCount)
    {
        nodes = gameWorldSp.path.nodes;
        do
        {
            if (nodes == node)
                break;
            ++v4;
            ++nodes;
        } while (v4 < g_path.actualNodeCount);
    }
    if (v4 == g_path.actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2141,
            0,
            "%s",
            "nodeIndex != g_path.actualNodeCount");
    return nodeIndexToCheck == v4;
}

int __cdecl Path_NodesVisible(const pathnode_t *node0, const pathnode_t *node1)
{
    unsigned int v4; // r31
    unsigned int v5; // r29
    unsigned int actualNodeCount; // r8
    int v7; // r4
    int v8; // r3
    int v9; // r3
    int result; // r3
    int v11; // r11
    int v12; // r10

    if (!node0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2159, 0, "%s", "node0");
    if (!node1)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2160, 0, "%s", "node1");
    v4 = Path_ConvertNodeToIndex(node0);
    v5 = Path_ConvertNodeToIndex(node1);
    actualNodeCount = g_path.actualNodeCount;
    if (v4 >= g_path.actualNodeCount)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2165,
            0,
            "nodeIndex0 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            v4,
            g_path.actualNodeCount);
        actualNodeCount = g_path.actualNodeCount;
    }
    if (v5 >= actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2166,
            0,
            "nodeIndex1 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            v5,
            actualNodeCount);
    if (v4 >= v5)
    {
        if (v4 <= v5)
            return 1;
        v7 = v4;
        v8 = v5;
    }
    else
    {
        v7 = v5;
        v8 = v4;
    }
    v9 = NodeVisCacheEntry(v8, v7);
    if (!gameWorldSp.path.pathVis)
        return 0;
    v11 = gameWorldSp.path.pathVis[v9 >> 3];
    v12 = 1 << (v9 & 7);
    result = 1;
    if ((v12 & v11) == 0)
        return 0;
    return result;
}

int __cdecl Path_ExpandedNodeVisible(const pathnode_t *node0, const pathnode_t *node1)
{
    unsigned int v4; // r31
    unsigned int v5; // r29
    unsigned int actualNodeCount; // r8
    int v7; // r4
    int v8; // r3
    int v9; // r3
    int result; // r3
    int v11; // r11
    int v12; // r10

    if (!node0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2187, 0, "%s", "node0");
    if (!node1)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2188, 0, "%s", "node1");
    v4 = Path_ConvertNodeToIndex(node0);
    v5 = Path_ConvertNodeToIndex(node1);
    actualNodeCount = g_path.actualNodeCount;
    if (v4 >= g_path.actualNodeCount)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2193,
            0,
            "nodeIndex0 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            v4,
            g_path.actualNodeCount);
        actualNodeCount = g_path.actualNodeCount;
    }
    if (v5 >= actualNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2194,
            0,
            "nodeIndex1 doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
            v5,
            actualNodeCount);
    if (v4 <= v5)
    {
        if (v4 >= v5)
            return 1;
        v7 = v4;
        v8 = v5;
    }
    else
    {
        v7 = v5;
        v8 = v4;
    }
    v9 = ExpandedNodeVisCacheEntry(v8, v7);
    if (!gameWorldSp.path.pathVis)
        return 0;
    v11 = gameWorldSp.path.pathVis[v9 >> 3];
    v12 = 1 << (v9 & 7);
    result = 1;
    if ((v12 & v11) == 0)
        return 0;
    return result;
}

pathnode_t *__cdecl Path_FindChainPos(const float *vOrigin, pathnode_t *pPrevChainPos)
{
    pathnode_t *result; // r3
    __int16 wChainId; // r31
    unsigned int v6; // r7
    double v7; // fp11
    unsigned __int16 *v8; // r10
    unsigned int v9; // r5
    pathnode_t *v10; // r11
    double v11; // fp0
    double v12; // fp13
    double v13; // fp12
    double v14; // fp0
    pathnode_t *v15; // r11
    double v16; // fp0
    double v17; // fp13
    double v18; // fp12
    double v19; // fp0
    pathnode_t *v20; // r11
    double v21; // fp0
    double v22; // fp13
    double v23; // fp12
    double v24; // fp0
    pathnode_t *v25; // r11
    double v26; // fp0
    double v27; // fp13
    double v28; // fp12
    double v29; // fp0
    unsigned __int16 *v30; // r10
    unsigned int v31; // r7
    pathnode_t *v32; // r11
    double v33; // fp0
    double v34; // fp13
    double v35; // fp12
    double v36; // fp0

    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2323, 0, "%s", "vOrigin");
    result = 0;
    if (pPrevChainPos)
        wChainId = pPrevChainPos->constant.wChainId;
    else
        wChainId = 0;
    v6 = 0;
    v7 = 3.4028235e38;
    if ((int)gameWorldSp.path.chainNodeCount >= 4)
    {
        v8 = gameWorldSp.path.nodeForChainNode + 2;
        v9 = ((gameWorldSp.path.chainNodeCount - 4) >> 2) + 1;
        v6 = 4 * v9;
        do
        {
            v10 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(*(v8 - 2), 7));
            if (v10->constant.wChainId == wChainId)
            {
                v13 = (float)(v10->constant.vOrigin[2] - vOrigin[2]);
                v12 = (float)(v10->constant.vOrigin[1] - vOrigin[1]);
                v11 = (float)(v10->constant.vOrigin[0] - *vOrigin);
                v14 = (float)((float)((float)v12 * (float)v12)
                    + (float)((float)((float)v11 * (float)v11) + (float)((float)v13 * (float)v13)));
                if (v14 < v7)
                {
                    v7 = v14;
                    result = v10;
                }
            }
            v15 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(*(v8 - 1), 7));
            if (v15->constant.wChainId == wChainId)
            {
                v18 = (float)(v15->constant.vOrigin[2] - vOrigin[2]);
                v17 = (float)(v15->constant.vOrigin[1] - vOrigin[1]);
                v16 = (float)(v15->constant.vOrigin[0] - *vOrigin);
                v19 = (float)((float)((float)v17 * (float)v17)
                    + (float)((float)((float)v16 * (float)v16) + (float)((float)v18 * (float)v18)));
                if (v19 < v7)
                {
                    v7 = v19;
                    result = v15;
                }
            }
            v20 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(*v8, 7));
            if (v20->constant.wChainId == wChainId)
            {
                v23 = (float)(v20->constant.vOrigin[2] - vOrigin[2]);
                v22 = (float)(v20->constant.vOrigin[1] - vOrigin[1]);
                v21 = (float)(v20->constant.vOrigin[0] - *vOrigin);
                v24 = (float)((float)((float)v22 * (float)v22)
                    + (float)((float)((float)v21 * (float)v21) + (float)((float)v23 * (float)v23)));
                if (v24 < v7)
                {
                    v7 = v24;
                    result = v20;
                }
            }
            v25 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(v8[1], 7));
            if (v25->constant.wChainId == wChainId)
            {
                v28 = (float)(v25->constant.vOrigin[2] - vOrigin[2]);
                v27 = (float)(v25->constant.vOrigin[1] - vOrigin[1]);
                v26 = (float)(v25->constant.vOrigin[0] - *vOrigin);
                v29 = (float)((float)((float)v27 * (float)v27)
                    + (float)((float)((float)v26 * (float)v26) + (float)((float)v28 * (float)v28)));
                if (v29 < v7)
                {
                    v7 = v29;
                    result = v25;
                }
            }
            --v9;
            v8 += 4;
        } while (v9);
    }
    if (v6 < gameWorldSp.path.chainNodeCount)
    {
        v30 = &gameWorldSp.path.nodeForChainNode[v6];
        v31 = gameWorldSp.path.chainNodeCount - v6;
        do
        {
            v32 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(*v30, 7));
            if (v32->constant.wChainId == wChainId)
            {
                v35 = (float)(v32->constant.vOrigin[2] - vOrigin[2]);
                v34 = (float)(v32->constant.vOrigin[1] - vOrigin[1]);
                v33 = (float)(v32->constant.vOrigin[0] - *vOrigin);
                v36 = (float)((float)((float)v34 * (float)v34)
                    + (float)((float)((float)v33 * (float)v33) + (float)((float)v35 * (float)v35)));
                if (v36 < v7)
                {
                    v7 = v36;
                    result = v32;
                }
            }
            --v31;
            ++v30;
        } while (v31);
    }
    return result;
}

void __cdecl Path_UpdateBestChainNode(pathnode_t *node, pathnode_t **bestNode, unsigned int *foundCount)
{
    int wChainDepth; // r11
    int v6; // r10

    if (!*bestNode)
    {
    LABEL_6:
        *bestNode = node;
        *foundCount = 1;
        return;
    }
    wChainDepth = (*bestNode)->constant.wChainDepth;
    v6 = node->constant.wChainDepth;
    if (v6 != wChainDepth)
    {
        if (v6 <= wChainDepth)
            return;
        goto LABEL_6;
    }
    ++ * foundCount;
    if (G_rand() * *foundCount <= 0x7FFF)
        *bestNode = node;
}

int __cdecl Path_CanSetDesiredChainPos(actor_s *claimer, const pathnode_t *node)
{
    int v4; // r30
    actor_s *Actor; // r3

    v4 = ~(1 << Sentient_EnemyTeam(claimer->sentient->eTeam));
    Actor = Actor_FirstActor(v4);
    if (!Actor)
        return 1;
    while (Actor == claimer || Actor->pDesiredChainPos != node)
    {
        Actor = Actor_NextActor(Actor, v4);
        if (!Actor)
            return 1;
    }
    return 0;
}

void __cdecl Path_AttachSentientToChainNode(sentient_s *sentient, unsigned __int16 targetname)
{
    unsigned int v4; // r28
    pathnode_t *pActualChainPos; // r11
    unsigned int v6; // r31
    unsigned __int16 *nodeForChainNode; // r7
    pathnode_t *nodes; // r9
    unsigned __int16 *v9; // r11
    pathnode_t *v10; // r10
    unsigned int actualNodeCount; // r10
    int v12; // r29
    const char *v13; // r3
    const char *v14; // r3

    if (!sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2755, 0, "%s", "sentient");
    v4 = targetname;
    if (!targetname)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2756, 0, "%s", "targetname");
    if (g_spawnai->current.enabled)
    {
        pActualChainPos = sentient->pActualChainPos;
        if (!pActualChainPos || pActualChainPos->constant.targetname != targetname)
        {
            v6 = 0;
            nodeForChainNode = gameWorldSp.path.nodeForChainNode;
            nodes = gameWorldSp.path.nodes;
            if (gameWorldSp.path.chainNodeCount)
            {
                v9 = gameWorldSp.path.nodeForChainNode;
                while (1)
                {
                    v10 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(*v9, 7));
                    if (v10->constant.targetname == targetname)
                        break;
                    ++v6;
                    ++v9;
                    if (v6 >= gameWorldSp.path.chainNodeCount)
                        goto LABEL_12;
                }
                sentient->pActualChainPos = v10;
            }
            else
            {
            LABEL_12:
                actualNodeCount = g_path.actualNodeCount;
                if (v6 < g_path.actualNodeCount)
                {
                    v12 = v6;
                    do
                    {
                        if (*(unsigned __int16 *)((char *)&nodes->constant.targetname + __ROL4__(nodeForChainNode[v12], 7)) == v4)
                        {
                            v13 = SL_ConvertToString(v4);
                            Com_Error(ERR_DROP, "node '%s' is not part of a friendly chain", v13);
                            nodeForChainNode = gameWorldSp.path.nodeForChainNode;
                            nodes = gameWorldSp.path.nodes;
                            actualNodeCount = g_path.actualNodeCount;
                        }
                        ++v6;
                        ++v12;
                    } while (v6 < actualNodeCount);
                }
                v14 = SL_ConvertToString(v4);
                Com_Error(ERR_DROP, "friendly chain node '%s' does not exist", v14);
            }
        }
    }
}

pathnode_t *__cdecl Path_FirstNode(int typeFlags)
{
    int v1; // r10
    pathnode_t *i; // r11

    v1 = 0;
    if (!g_path.actualNodeCount)
        return 0;
    for (i = gameWorldSp.path.nodes; ((1 << i->constant.type) & typeFlags) == 0; ++i)
    {
        if (++v1 >= g_path.actualNodeCount)
            return 0;
    }
    return &gameWorldSp.path.nodes[v1];
}

pathnode_t *__cdecl Path_NextNode(pathnode_t *prevNode, int typeFlags)
{
    pathnode_t *nodes; // r11
    unsigned int actualNodeCount; // r9
    unsigned int v6; // r10
    pathnode_t *i; // r8

    if (!prevNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2815, 0, "%s", "prevNode");
    nodes = gameWorldSp.path.nodes;
    if (prevNode < gameWorldSp.path.nodes
        || (actualNodeCount = g_path.actualNodeCount, prevNode >= &gameWorldSp.path.nodes[g_path.actualNodeCount]))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2816,
            0,
            "%s",
            "prevNode >= gameWorldSp.path.nodes && prevNode < &gameWorldSp.path.nodes[g_path.actualNodeCount]");
        nodes = gameWorldSp.path.nodes;
        actualNodeCount = g_path.actualNodeCount;
    }
    if (prevNode != &nodes[prevNode - nodes])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2817,
            0,
            "%s",
            "prevNode == &gameWorldSp.path.nodes[prevNode - gameWorldSp.path.nodes]");
        nodes = gameWorldSp.path.nodes;
        actualNodeCount = g_path.actualNodeCount;
    }
    v6 = prevNode - nodes + 1;
    if (v6 >= actualNodeCount)
        return 0;
    for (i = &nodes[v6]; ((1 << i->constant.type) & typeFlags) == 0; ++i)
    {
        if (++v6 >= actualNodeCount)
            return 0;
    }
    return &nodes[v6];
}

sentient_s *__cdecl Path_GetNodeOwner(const pathnode_t *node)
{
    const pathnode_dynamic_t *p_dynamic; // r31
    int v4; // r11
    pathnode_dynamic_t *v5; // r30
    int v6; // r11

    iassert(node);

    p_dynamic = &node->dynamic;

    if (node->dynamic.pOwner.isDefined())
        return p_dynamic->pOwner.sentient();

    v4 = node->constant.wOverlapNode[0];
    if (v4 >= 0)
    {
        v5 = &gameWorldSp.path.nodes[v4].dynamic;
        if (v5->pOwner.isDefined())
            return v5->pOwner.sentient();
        v6 = node->constant.wOverlapNode[1];
        if (v6 >= 0)
        {
            p_dynamic = &gameWorldSp.path.nodes[v6].dynamic;
            if (p_dynamic->pOwner.isDefined())
                return p_dynamic->pOwner.sentient();
        }
    }
    return 0;
}

int __cdecl Path_CanStealPriorityNode(const pathnode_t *node, sentient_s *claimer)
{
    sentient_s *NodeOwner; // r3
    sentient_s *v5; // r29
    int result; // r3
    const actor_s *actor; // r3
    double v8; // fp31
    double v9; // fp1

    iassert(node);
    iassert(node->constant.spawnflags & PNF_PRIORITY);
    iassert(claimer);

    NodeOwner = Path_GetNodeOwner(node);
    v5 = NodeOwner;
    if (!NodeOwner)
        return 1;
    if (node->dynamic.wOverlapCount > 1)
    {
        iassert(!node->dynamic.pOwner.isDefined());
        return 0;
    }
    actor = NodeOwner->ent->actor;
    if (!actor)
        return 0;
    if (v5 == claimer)
        return 1;
    if (v5->eTeam != claimer->eTeam)
        return 0;
    if (Actor_IsDying(actor))
        return 1;
    v8 = Vec2DistanceSq(v5->ent->r.currentOrigin, node->constant.vOrigin);
    if (v8 < 225.0)
        return 0;
    v9 = Vec2DistanceSq(claimer->ent->r.currentOrigin, node->constant.vOrigin);
    result = 1;
    if ((float)((float)v9 + (float)225.0) >= v8)
        return 0;
    return result;
}

int __cdecl Path_CanStealNode(const pathnode_t *node, sentient_s *claimer)
{
    sentient_s *NodeOwner; // r3
    sentient_s *v5; // r28
    int result; // r3
    actor_s *actor; // r3
    bool v8; // zf

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2942, 0, "%s", "node");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2943, 0, "%s", "claimer");
    NodeOwner = Path_GetNodeOwner(node);
    v5 = NodeOwner;
    if (!NodeOwner)
        return 1;
    if (node->dynamic.wOverlapCount > 1)
    {
        iassert(!node->dynamic.pOwner.isDefined());
        return 0;
    }
    if (!NodeOwner->ent->actor)
        return 0;
    if (NodeOwner == claimer)
        return 1;
    actor = claimer->ent->actor;
    if (!actor)
        return 0;
    if (claimer->pClaimedNode)
        return 0;
    if ((unsigned __int8)Actor_IsMoving(actor))
        return 0;
    if (!Actor_PointNearNode(claimer->ent->r.currentOrigin, node))
        return 0;
    v8 = !Actor_PointNearNode(v5->ent->r.currentOrigin, node);
    result = 1;
    if (!v8)
        return 0;
    return result;
}

void __cdecl Path_ClaimNodeInternal(pathnode_t *node, sentient_s *claimer)
{
    actor_s *actor; // r3

    actor = claimer->ent->actor;
    if (actor && (unsigned __int8)Actor_KeepClaimedNode(actor))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3066,
            0,
            "%s",
            "!claimer->ent->actor || !Actor_KeepClaimedNode(claimer->ent->actor)");
    claimer->pClaimedNode = node;
    claimer->banNodeTime = 0;
    Sentient_BanNearNodes(claimer);
}

void __cdecl Path_MarkNodeOverlap(pathnode_t *node)
{
    iassert(node);
    node->dynamic.pOwner.setSentient(NULL);
    ++node->dynamic.wOverlapCount;
    node->dynamic.iFreeTime = 0x7FFFFFFF;
}

void __cdecl Path_ClaimNode(pathnode_t *node, sentient_s *claimer)
{
    int iFreeTime; // r8
    int loopItr; // r18
    __int16 *wOverlapNode; // r20
    int v8; // r11
    pathnode_t *otherNode; // r31
    SentientHandle *p_pOwner; // r30
    int v11; // r5
    const char *v12; // r3
    actor_s *actor; // r3

    iassert(node);
    iassert(claimer);
    iassert(claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES || claimer->eTeam == TEAM_NEUTRAL);
    iassert(((node->dynamic.pOwner.isDefined() && (node->dynamic.pOwner.sentient() == claimer)) || node->dynamic.iFreeTime < level.time));
    iassert(!node->dynamic.pOwner.isDefined() || node->dynamic.pOwner.sentient() == claimer || level.time > node->dynamic.iValidTime[claimer->eTeam - TEAM_AXIS]);

    node->dynamic.pOwner.setSentient(claimer);

    loopItr = 0;
    wOverlapNode = node->constant.wOverlapNode;
    node->dynamic.iFreeTime = 0x7FFFFFFF;
    do
    {
        v8 = *wOverlapNode;
        if (v8 < 0)
            break;
        otherNode = &gameWorldSp.path.nodes[v8];
        p_pOwner = &otherNode->dynamic.pOwner;

        iassert(((otherNode->dynamic.pOwner.isDefined() && (otherNode->dynamic.pOwner.sentient() == claimer)) || (otherNode->dynamic.iFreeTime != INT_MAX) || (otherNode->dynamic.wOverlapCount)));
        iassert(otherNode);

        p_pOwner->setSentient(NULL);

        otherNode->dynamic.iFreeTime = 0x7FFFFFFF;
        otherNode->dynamic.wOverlapCount++;

        bcassert(otherNode->dynamic.wOverlapCount, ARRAY_COUNT(otherNode->constant.wOverlapNode) + 1);
        iassert(otherNode->constant.wOverlapNode[otherNode->dynamic.wOverlapCount - 1] >= 0);
        ++loopItr;
        ++wOverlapNode;
    } while (loopItr < 2);

    iassert(!claimer->ent->actor || !Actor_KeepClaimedNode(claimer->ent->actor));

    claimer->pClaimedNode = node;
    claimer->banNodeTime = 0;
    Sentient_BanNearNodes(claimer);
}

void __cdecl Path_RevokeClaim(pathnode_t *node, sentient_s *pNewClaimer)
{
    pathnode_dynamic_t *p_dynamic; // r31

    p_dynamic = &node->dynamic;
    if ((!node->dynamic.pOwner.isDefined()
        || !p_dynamic->pOwner.sentient()->ent
        || p_dynamic->pOwner.sentient()->ent->actor)
        && p_dynamic->pOwner.isDefined()
        && p_dynamic->pOwner.sentient() != pNewClaimer
        && node->dynamic.iFreeTime == 0x7FFFFFFF)
    {
        Sentient_NodeClaimRevoked(p_dynamic->pOwner.sentient(), node);
        iassert(node->dynamic.iFreeTime != INT_MAX);
    }
}

void __cdecl Path_RelinquishNode(sentient_s *claimer, int timeUntilRelinquished)
{
    pathnode_t *node; // r27
    gentity_s *ent; // r3
    gentity_s *v7; // r11
    actor_s *actor; // r11
    int v9; // r29
    sentient_s *v10; // r28
    __int16 *wOverlapNode; // r26
    int i; // r25
    int v13; // r11
    pathnode_t *otherNode; // r31
    int v15; // r11

    node = claimer->pClaimedNode;

    iassert(node);
    iassert(node->dynamic.pOwner.isDefined());
    iassert(node->dynamic.pOwner.sentient() == claimer);
    iassert(claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES || claimer->eTeam == TEAM_NEUTRAL);
    
    ent = claimer->ent;
    claimer->pClaimedNode = 0;
    Scr_Notify(ent, scr_const.node_relinquished, 0);
    v7 = claimer->ent;
    claimer->bNearestNodeValid = 0;
    actor = v7->actor;
    if (actor)
        actor->arrivalInfo.animscriptOverrideRunTo = 0;
    if (timeUntilRelinquished > 0)
    {
        v10 = claimer;
        v9 = level.time + timeUntilRelinquished;
    }
    else
    {
        v9 = 0;
        v10 = 0;
    }
    node->dynamic.iFreeTime = v9;
    node->dynamic.pOwner.setSentient(v10);
    wOverlapNode = node->constant.wOverlapNode;
    for (i = 0; i < 2; ++i)
    {
        v13 = *wOverlapNode;
        if (v13 < 0)
            break;
        otherNode = &gameWorldSp.path.nodes[v13];
        iassert(!otherNode->dynamic.pOwner.isDefined());
        v15 = (__int16)(otherNode->dynamic.wOverlapCount - 1);
        otherNode->dynamic.wOverlapCount = v15;
        if (!v15)
        {
            otherNode->dynamic.pOwner.setSentient(v10);
            otherNode->dynamic.iFreeTime = v9;
        }
        ++wOverlapNode;
    }
}

int __cdecl Path_AllowedStancesForNode(pathnode_t *node)
{
    int spawnflags; // r8
    int v3; // r31

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3323, 0, "%s", "node");
    spawnflags = node->constant.spawnflags;
    v3 = 7;
    if ((spawnflags & 4) != 0)
        v3 = 6;
    if ((spawnflags & 8) != 0)
        v3 &= ~2u;
    if ((spawnflags & 0x10) != 0)
        v3 &= ~4u;
    if (!v3)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3334,
            0,
            "%s\n\t(node->constant.spawnflags) = %i",
            "(eAllowedStances)",
            spawnflags);
    return v3;
}

int __cdecl Path_SaveIndex(const pathnode_t *node)
{
    pathnode_t *nodes; // r11

    if (!node)
        return 0;
    nodes = gameWorldSp.path.nodes;
    if (node < gameWorldSp.path.nodes || node >= &gameWorldSp.path.nodes[g_path.actualNodeCount])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3341,
            0,
            "%s",
            "node == NULL || (node >= gameWorldSp.path.nodes && node < &gameWorldSp.path.nodes[g_path.actualNodeCount])");
        nodes = gameWorldSp.path.nodes;
    }
    return node - nodes + 1;
}

pathnode_t *__cdecl Path_LoadNode(unsigned int index)
{
    if (index > g_path.actualNodeCount)
        Com_Error(ERR_DROP, "Path_LoadNode: node out of range (%i)", index);
    if (index)
        return &gameWorldSp.path.nodes[index - 1];
    else
        return 0;
}

void __cdecl Path_ValidateNode(pathnode_t *node)
{
    int v2; // r31
    int v3; // r27
    pathlink_s *v4; // r10
    const char *v5; // r3
    int v6; // r27
    pathlink_s *v7; // r10
    const char *v8; // r3

    v2 = node->constant.totalLinkCount - 1;
    if (v2 >= node->dynamic.wLinkCount)
    {
        v3 = v2;
        do
        {
            v4 = &node->constant.Links[v3];
            if (!v4->disconnectCount)
            {
                v5 = va("%d, %d, %d", node - gameWorldSp.path.nodes, v2, v4->nodeNum);
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                    3372,
                    0,
                    "%s\n\t%s",
                    "link->disconnectCount > 0",
                    v5);
            }
            --v2;
            --v3;
        } while (v2 >= node->dynamic.wLinkCount);
    }
    if (v2 != node->dynamic.wLinkCount - 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3375,
            0,
            "%s",
            "j == node->dynamic.wLinkCount - 1");
    if (v2 >= 0)
    {
        v6 = v2;
        do
        {
            v7 = &node->constant.Links[v6];
            if (v7->disconnectCount)
            {
                v8 = va("%d, %d, %d", node - gameWorldSp.path.nodes, v2, v7->nodeNum);
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                    3379,
                    0,
                    "%s\n\t%s",
                    "!link->disconnectCount",
                    v8);
            }
            --v2;
            --v6;
        } while (v2 >= 0);
    }
}

void __cdecl Path_ValidateAllNodes()
{
    unsigned int v0; // r31
    int v1; // r30

    v0 = 0;
    if (g_path.actualNodeCount)
    {
        v1 = 0;
        do
        {
            Path_ValidateNode(&gameWorldSp.path.nodes[v1]);
            ++v0;
            ++v1;
        } while (v0 < g_path.actualNodeCount);
    }
}

void __cdecl Path_CheckLinkLeaks()
{
    int next; // r11
    int v1; // r10
    int prev; // r11
    int v3; // r10

    if (g_path.pathLinkInfoArrayInited)
    {
        next = g_path.pathLinkInfoArray[0].next;
        v1 = 0;
        if (!g_path.pathLinkInfoArray[0].next)
            goto LABEL_5;
        do
        {
            ++v1;
            next = g_path.pathLinkInfoArray[next].next;
        } while (next);
        if (v1 != 2047)
            LABEL_5:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3404,
            0,
            "%s",
            "count == PATHLINK_BUFFERSIZE - 1");
        prev = g_path.pathLinkInfoArray[0].prev;
        v3 = 0;
        if (!g_path.pathLinkInfoArray[0].prev)
            goto LABEL_9;
        do
        {
            ++v3;
            prev = g_path.pathLinkInfoArray[prev].prev;
        } while (prev);
        if (v3 != 2047)
            LABEL_9:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3409,
            0,
            "%s",
            "count == PATHLINK_BUFFERSIZE - 1");
    }
}

void __cdecl Path_CheckUserCountLeaks()
{
    unsigned int v0; // r25
    unsigned int actualNodeCount; // r10
    int v2; // r28

    v0 = 0;
    actualNodeCount = g_path.actualNodeCount;
    if (g_path.actualNodeCount)
    {
        v2 = 0;
        do
        {
            if (gameWorldSp.path.nodes[v2].dynamic.userCount)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                    3418,
                    0,
                    "%s",
                    "gameWorldSp.path.nodes[i].dynamic.userCount == 0");
                actualNodeCount = g_path.actualNodeCount;
            }
            ++v0;
            ++v2;
        } while (v0 < actualNodeCount);
    }
}

void __cdecl Path_DisconnectPath(pathnode_t *node, pathlink_s *link)
{
    int v4; // r11
    pathlink_s *v5; // r11
    int wLinkCount; // r5
    unsigned int v7; // r9
    unsigned int v8; // r11
    const char *v9; // r3
    int v10; // r11
    const char *v11; // r3
    pathlink_s *Links; // r9
    const char *v13; // r3
    float fDist; // r10
    int v15; // r9
    int v16; // r8
    pathlink_s *v17; // r11
    pathlink_s *v18; // r11

    Path_ValidateNode(node);
    v4 = (unsigned __int8)(link->disconnectCount + 1);
    link->disconnectCount = v4;
    if (!v4)
        Scr_Error("too many disconnects on a single path link (overflow on disconnect count)");
    if (!link->disconnectCount)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3435, 0, "%s", "link->disconnectCount");
    if (link->disconnectCount <= 1u)
    {
        v10 = (__int16)(node->dynamic.wLinkCount - 1);
        node->dynamic.wLinkCount = v10;
        if (v10 < 0)
        {
            v11 = va("node: %d, %d", node - gameWorldSp.path.nodes, node->dynamic.wLinkCount);
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                3446,
                0,
                "%s\n\t%s",
                "node->dynamic.wLinkCount >= 0",
                v11);
        }
        Links = node->constant.Links;
        if (&Links[node->dynamic.wLinkCount] < link)
        {
            v13 = va(
                "node: %d, %d (%d) %d (%d)",
                node - gameWorldSp.path.nodes,
                node->dynamic.wLinkCount,
                Links[node->dynamic.wLinkCount].nodeNum,
                link - Links,
                link->nodeNum);
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                3447,
                0,
                "%s\n\t%s",
                "&node->constant.Links[node->dynamic.wLinkCount] >= link",
                v13);
        }
        fDist = link->fDist;
        v15 = *(unsigned int *)&link->nodeNum;
        v16 = *(unsigned int *)link->ubBadPlaceCount;
        v17 = &node->constant.Links[node->dynamic.wLinkCount];
        link->fDist = v17->fDist;
        *(unsigned int *)&link->nodeNum = *(unsigned int *)&v17->nodeNum;
        *(unsigned int *)link->ubBadPlaceCount = *(unsigned int *)v17->ubBadPlaceCount;
        v18 = &node->constant.Links[node->dynamic.wLinkCount];
        v18->fDist = fDist;
        *(unsigned int *)&v18->nodeNum = v15;
        *(unsigned int *)v18->ubBadPlaceCount = v16;
    }
    else
    {
        v5 = node->constant.Links;
        wLinkCount = node->dynamic.wLinkCount;
        v7 = (unsigned int)&v5[wLinkCount];
        if (v7 > (unsigned int)link)
        {
            v8 = (int)((unsigned __int64)(715827883LL * ((char *)link - (char *)v5)) >> 32) >> 1;
            v9 = va(
                "node: %d, %d (%d) %d (%d)",
                node - gameWorldSp.path.nodes,
                wLinkCount,
                *(unsigned __int16 *)(v7 + 4),
                v8 + (v8 >> 31),
                link->nodeNum);
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                3439,
                0,
                "%s\n\t%s",
                "&node->constant.Links[node->dynamic.wLinkCount] <= link",
                v9);
            Path_ValidateNode(node);
            return;
        }
    }
    Path_ValidateNode(node);
}

void __cdecl Path_ConnectPath(pathnode_t *node, pathlink_s *link)
{
    int v4; // r11
    float fDist; // r10
    int v6; // r9
    int v7; // r8
    pathlink_s *v8; // r11
    pathlink_s *v9; // r11

    if (!link->disconnectCount)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3461, 0, "%s", "link->disconnectCount");
    if (&node->constant.Links[node->dynamic.wLinkCount] > link)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3462,
            0,
            "%s",
            "&node->constant.Links[node->dynamic.wLinkCount] <= link");
    Path_ValidateNode(node);
    v4 = (unsigned __int8)(link->disconnectCount - 1);
    link->disconnectCount = v4;
    if (!v4)
    {
        fDist = link->fDist;
        v6 = *(unsigned int *)&link->nodeNum;
        v7 = *(unsigned int *)link->ubBadPlaceCount;
        v8 = &node->constant.Links[node->dynamic.wLinkCount];
        link->fDist = v8->fDist;
        *(unsigned int *)&link->nodeNum = *(unsigned int *)&v8->nodeNum;
        *(unsigned int *)link->ubBadPlaceCount = *(unsigned int *)v8->ubBadPlaceCount;
        v9 = &node->constant.Links[node->dynamic.wLinkCount];
        v9->fDist = fDist;
        *(unsigned int *)&v9->nodeNum = v6;
        *(unsigned int *)v9->ubBadPlaceCount = v7;
        ++node->dynamic.wLinkCount;
    }
    Path_ValidateNode(node);
}

static void __cdecl Path_ConnectPath_0(pathnode_t *node, int toNodeNum)
{
    int totalLinkCount; // r8
    int wLinkCount; // r11
    pathlink_s *v4; // r10

    totalLinkCount = node->constant.totalLinkCount;
    wLinkCount = node->dynamic.wLinkCount;
    if (wLinkCount >= totalLinkCount)
    {
    LABEL_5:
        if (!alwaysfails)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                3499,
                0,
                "Path_ConnectPath: should be unreachable");
    }
    else
    {
        v4 = &node->constant.Links[wLinkCount];
        while (v4->nodeNum != toNodeNum)
        {
            ++wLinkCount;
            ++v4;
            if (wLinkCount >= totalLinkCount)
                goto LABEL_5;
        }
        Path_ConnectPath(node, v4);
    }
}

void __cdecl Path_ConnectPathsForEntity(gentity_s *ent)
{
    int disconnectedLinks; // r7
    int next; // r11
    unsigned __int16 prev; // r8
    int v5; // r30

    if ((ent->flags & 0x100) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3546,
            0,
            "%s",
            "Path_IsDynamicBlockingEntity( ent )");
    disconnectedLinks = ent->disconnectedLinks;
    ent->flags |= 0x400u;
    next = disconnectedLinks;
    if (disconnectedLinks)
    {
        ent->disconnectedLinks = 0;
        *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].next
            + __ROL4__(g_path.pathLinkInfoArray[disconnectedLinks].prev, 3)) = 0;
        *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].next + __ROL4__(g_path.pathLinkInfoArray[0].prev, 3)) = disconnectedLinks;
        prev = g_path.pathLinkInfoArray[0].prev;
        g_path.pathLinkInfoArray[0].prev = g_path.pathLinkInfoArray[disconnectedLinks].prev;
        g_path.pathLinkInfoArray[disconnectedLinks].prev = prev;
        do
        {
            v5 = next;
            Path_ConnectPath_0(
                (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(g_path.pathLinkInfoArray[next].from, 7)),
                g_path.pathLinkInfoArray[next].to);
            next = g_path.pathLinkInfoArray[v5].next;
        } while (g_path.pathLinkInfoArray[v5].next);
    }
}

void __cdecl Path_DisconnectPath_0(gentity_s *ent, pathnode_t *node, pathlink_s *link)
{
    int next; // r29
    unsigned __int16 v7; // r28
    PathLinkInfo *v8; // r11
    int v9; // r7
    int v10; // r10

    next = g_path.pathLinkInfoArray[0].next;
    v7 = g_path.pathLinkInfoArray[0].next;
    if (!g_path.pathLinkInfoArray[0].next)
        Com_Error(ERR_DROP, "max number of disconnected paths exceeded");
    v8 = &g_path.pathLinkInfoArray[next];
    v9 = node - gameWorldSp.path.nodes;
    g_path.pathLinkInfoArray[0].next = v8->next;
    *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].prev + __ROL4__(v8->next, 3)) = 0;
    v8->from = v9;
    v8->to = link->nodeNum;
    if (ent->disconnectedLinks)
    {
        v8->prev = ent->disconnectedLinks;
        v10 = *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].next + __ROL4__(ent->disconnectedLinks, 3));
        v8->next = v10;
        *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].prev + __ROL4__(v10, 3)) = v7;
        *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].next + __ROL4__(v8->prev, 3)) = v7;
    }
    else
    {
        ent->disconnectedLinks = v7;
        v8->next = v7;
        v8->prev = v7;
    }
    Path_DisconnectPath(node, link);
}

static float disconnectMins[3] = { -15.0, -15.0, 18.0 };
static float disconnectMaxs[3] = { 15.0, 15.0, 72.0 };
void __cdecl Path_DisconnectPathsForEntity(gentity_s *ent)
{
    int number; // r26
    unsigned int v3; // r21
    unsigned int actualNodeCount; // r10
    int v5; // r22
    pathnode_t *v6; // r30
    int totalLinkCount; // r24
    int v8; // r29
    pathlink_s *v9; // r31
    float v10[32]; // [sp+50h] [-80h] BYREF

    if ((ent->flags & 0x100) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3579,
            0,
            "%s",
            "Path_IsDynamicBlockingEntity( ent )");
    Path_ConnectPathsForEntity(ent);
    number = ent->s.number;
    v3 = 0;
    ent->flags &= ~0x400u;
    ent->iDisconnectTime = level.time;
    actualNodeCount = g_path.actualNodeCount;
    if (g_path.actualNodeCount)
    {
        v5 = 0;
        do
        {
            v6 = &gameWorldSp.path.nodes[v5];
            v10[0] = gameWorldSp.path.nodes[v5].constant.vOrigin[0];
            v10[1] = gameWorldSp.path.nodes[v5].constant.vOrigin[1];
            v10[2] = gameWorldSp.path.nodes[v5].constant.vOrigin[2];
            if (gameWorldSp.path.nodes[v5].constant.totalLinkCount)
            {
                totalLinkCount = v6->constant.totalLinkCount;
                v8 = gameWorldSp.path.nodes[v5].constant.totalLinkCount - 1;
                do
                {
                    v9 = &v6->constant.Links[v8];
                    if (SV_SightTraceToEntity(
                        v10,
                        disconnectMins,
                        disconnectMaxs,
                        (float *)((char *)gameWorldSp.path.nodes->constant.vOrigin + __ROL4__(v9->nodeNum, 7)),
                        number,
                        42074129))
                    {
                        Path_DisconnectPath_0(ent, v6, v9);
                    }
                    --totalLinkCount;
                    --v8;
                } while (totalLinkCount);
                actualNodeCount = g_path.actualNodeCount;
            }
            ++v3;
            ++v5;
        } while (v3 < actualNodeCount);
    }
}

void __cdecl Path_UpdateBadPlaceCountForLink(pathlink_s *link, int teamflags, int delta)
{
    unsigned int i; // r29
    int v7; // r31
    const char *v8; // r3

    for (i = 0; i < 4; ++i)
    {
        if (((1 << i) & teamflags) == 0)
            continue;
        v7 = link->ubBadPlaceCount[i] + delta;
        if (v7 < 0)
        {
            v8 = "Bad place underflow -- negative count";
        LABEL_7:
            Scr_Error(v8);
            goto LABEL_8;
        }
        if (v7 > 255)
        {
            v8 = "Bad place overflow -- count exceeds 255";
            goto LABEL_7;
        }
    LABEL_8:
        link->ubBadPlaceCount[i] = v7;
    }
}

// 
void Path_UpdateArcBadPlaceCount(badplace_arc_t *arc, int teamFlags, int delta)
{
    // Compute angular difference between arc start and end
    float angleDelta = arc->angle1 - arc->angle0;
    if (angleDelta < 0.0f)
        angleDelta += 360.0f;

    bool clockwise = angleDelta < 180.0f;

    // Compute unit vectors for Yaw
    float arcMidAngle = arc->angle0 + (angleDelta * 0.5f);
    float direction[3], rightVec[3], upVec[3];
    YawVectors(arc->angle0, arc->origin, 0);
    YawVectors(arc->angle1, direction, 0);
    YawVectors(arcMidAngle, rightVec, upVec);

    float angleRad = angleDelta * 0.0087266462f;  // Convert degrees to radians
    float sinAngle = sinf(angleRad);

    float arcRadiusSq = arc->radius * arc->radius;
    float arcHalfHeightSq = arc->halfheight * arc->halfheight;

    float extendedRadius = arc->radius + 256.0f;
    float extendedHeight = arc->halfheight + 128.0f;
    float extendedRadiusSq = extendedRadius * extendedRadius;
    float extendedHeightSq = extendedHeight * extendedHeight;

    // Compute center point of the arc
    float scalar = ((sinAngle / angleDelta) * 76.394371f) * arc->radius;
    float arcCenter[3] = {
        arc->origin[0] + (rightVec[0] * scalar),
        arc->origin[1] + (rightVec[1] * scalar),
        arc->origin[2] + (rightVec[2] * scalar)
    };

    if (!g_path.actualNodeCount)
        return;

    for (int nodeIdx = 0; nodeIdx < g_path.actualNodeCount; ++nodeIdx)
    {
        pathnode_t *node = &gameWorldSp.path.nodes[nodeIdx];
        float *nodeOrigin = node->constant.vOrigin;

        float dx = nodeOrigin[0] - arc->origin[0];
        float dy = nodeOrigin[1] - arc->origin[1];
        float dz = nodeOrigin[2] - arc->origin[2];

        float horizontalDistSq = dx * dx + dy * dy;
        float verticalDistSq = dz * dz;

        if (horizontalDistSq >= extendedRadiusSq || verticalDistSq >= extendedHeightSq)
            continue;

        float distToArcCenterSq = Vec2DistanceSq(nodeOrigin, arcCenter);

        for (int linkIdx = 0; linkIdx < node->constant.totalLinkCount; ++linkIdx)
        {
            pathlink_s *link = &node->constant.Links[linkIdx];

            // Convert link->nodeNum to real pointer using PPC-specific __ROL4__ trick
            float *targetNode = (float *)((char *)gameWorldSp.path.nodes + __ROL4__(link->nodeNum, 7));
            float *targetOrigin = targetNode + 5;

            // TODO: Insert further checks & cleanup inside the loop
            // We'll handle this in the next step.
        }
    }
}


void __cdecl Path_UpdateBrushBadPlaceCount(gentity_s *brushEnt, int teamflags, int delta)
{
    int v6; // r5
    pathsort_t *v7; // r4
    double radius; // fp29
    double height; // fp30
    int nodeCount; // r29
    pathsort_t *nodePtr; // r30
    pathnode_t *node; // r31
    unsigned int v13; // r28
    unsigned int actualNodeCount; // r10
    int v15; // r27
    pathnode_t *v16; // r30
    unsigned int v17; // r29
    int v18; // r31
    pathlink_s *v19; // r3
    pathsort_t nodes[768]; // [sp+50h] [-3870h] BYREF

    iassert(brushEnt);
    iassert(brushEnt->r.currentAngles[PITCH] == 0.f && brushEnt->r.currentAngles[ROLL] == 0.f);

    memset(nodes, 0, sizeof(nodes));

    radius = RadiusFromBounds2D(brushEnt->r.mins, brushEnt->r.maxs);
    height = (float)(brushEnt->r.maxs[2] - brushEnt->r.mins[2]);

    iassert(radius > 0.0f);
    iassert(height > 0.0f);

    nodeCount = Path_NodesInCylinder(brushEnt->r.currentOrigin, radius, height, nodes, 512, -2);

    iassert((unsigned int)nodeCount < MAX_NODES_IN_BRUSH);

    if (nodeCount)
    {
        nodePtr = nodes;
        do
        {
            node = nodePtr->node;

            if (SV_EntityContact(nodePtr->node->constant.vOrigin, nodePtr->node->constant.vOrigin, brushEnt))
                *((_BYTE *)&nodes[512].node + (uintptr_t)node - (uintptr_t)gameWorldSp.path.nodes) = 1; // KISAKTODO 

            nodeCount--;
            ++nodePtr;
        } while (nodeCount);
    }

    v13 = 0;
    actualNodeCount = g_path.actualNodeCount;
    if (g_path.actualNodeCount)
    {
        v15 = 0;
        do
        {
            v16 = &gameWorldSp.path.nodes[v15];
            if (!*((_BYTE *)&nodes[512].node + v13))// KISAKTODO 
            {
                v17 = 0;
                if (v16->constant.totalLinkCount)
                {
                    v18 = 0;
                    do
                    {
                        v19 = &v16->constant.Links[v18];
                        if (*((_BYTE *)&nodes[512].node + v19->nodeNum))// KISAKTODO 
                            Path_UpdateBadPlaceCountForLink(v19, teamflags, delta);
                        ++v17;
                        ++v18;
                    } while (v17 < v16->constant.totalLinkCount);
                    actualNodeCount = g_path.actualNodeCount;
                }
            }
            ++v13;
            ++v15;
        } while (v13 < actualNodeCount);
    }
}

int __cdecl Path_IsNodeInArc(
    pathnode_t *pNode,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight)
{
    return IsPosInsideArc(pNode->constant.vOrigin, 15.0, origin, radius, angle0, angle1, halfHeight);
}

void __cdecl WriteEntityDisconnectedLinks(gentity_s *ent, SaveGame *save)
{
    int disconnectedLinks; // r11
    int v5; // r31

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3845, 0, "%s", "save");
    disconnectedLinks = ent->disconnectedLinks;
    if (ent->disconnectedLinks)
    {
        do
        {
            v5 = disconnectedLinks;
            SaveMemory_SaveWrite(&g_path.pathLinkInfoArray[disconnectedLinks], 8, save);
            disconnectedLinks = g_path.pathLinkInfoArray[v5].next;
        } while (disconnectedLinks != ent->disconnectedLinks);
    }
}

void __cdecl ReadEntityDisconnectedLinks(gentity_s *ent, SaveGame *save)
{
    int disconnectedLinks; // r27
    int v5; // r31
    pathnode_t *v6; // r29
    int v7; // r31
    int i; // r28
    int prev; // r10
    int next; // r9
    unsigned __int64 v11; // r4
    _WORD v12[2]; // [sp+50h] [-70h] BYREF
    unsigned int v13; // [sp+54h] [-6Ch]

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3869, 0, "%s", "save");
    disconnectedLinks = ent->disconnectedLinks;
    if (ent->disconnectedLinks)
    {
        do
        {
            SaveMemory_LoadRead(v12, 8, save);
            v5 = v12[0];
            if (v12[0] >= g_path.actualNodeCount)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                    1185,
                    0,
                    "index doesn't index g_path.actualNodeCount\n\t%i not in [0, %i)",
                    v12[0],
                    g_path.actualNodeCount);
            v6 = &gameWorldSp.path.nodes[v5];
            v7 = v6->constant.totalLinkCount - 1;
            for (i = v7; ; --i)
            {
                if (v7 < v6->dynamic.wLinkCount)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                        3882,
                        0,
                        "%s",
                        "j >= node->dynamic.wLinkCount");
                if (v6->constant.Links[i].nodeNum == v12[1])
                    break;
                --v7;
            }
            ++v6->constant.Links[v7].disconnectCount;
            prev = g_path.pathLinkInfoArray[disconnectedLinks].prev;
            next = g_path.pathLinkInfoArray[disconnectedLinks].next;
            v11 = __PAIR64__(__ROL4__(next, 3), v13);
            *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].next + __ROL4__(prev, 3)) = next;
            *(unsigned __int16 *)((char *)&g_path.pathLinkInfoArray[0].prev + HIDWORD(v11)) = prev;
            g_path.pathLinkInfoArray[disconnectedLinks] = (PathLinkInfo)v11;
            disconnectedLinks = g_path.pathLinkInfoArray[disconnectedLinks].next;
        } while (disconnectedLinks != ent->disconnectedLinks);
    }
}

void __cdecl Scr_SetNodePriority()
{
    unsigned int v0; // r4
    pathnode_t *Pathnode; // r29
    int Int; // r28
    unsigned int v3; // r10
    NodeTypeToName *v4; // r11
    char v5; // r11
    int v6; // r31
    const char **p_name; // r30
    unsigned __int16 v8; // r11

    if (Scr_GetNumParam() == 2)
    {
        Pathnode = Scr_GetPathnode(0);
        Int = Scr_GetInt(1u);
        v3 = 0;
        v4 = priorityAllowedNodes;
        while (Pathnode->constant.type != v4->type)
        {
            v3 += 8;
            ++v4;
            if (v3 >= 0x48)
            {
                v5 = 0;
                goto LABEL_7;
            }
        }
        v5 = 1;
    LABEL_7:
        if (!v5)
        {
            Scr_Error("Cannot enable disable priority for this node type. Priority can only be set for :");
            v6 = 9;
            p_name = &priorityAllowedNodes[0].name;
            do
            {
                Scr_Error(*p_name);
                --v6;
                p_name += 2;
            } while (v6);
        }
        if (Int)
            v8 = Pathnode->constant.spawnflags | 0x40;
        else
            v8 = Pathnode->constant.spawnflags & 0xFFBF;
        Pathnode->constant.spawnflags = v8;
    }
    else
    {
        Scr_Error("Illegal call to setnodepriority");
    }
}

void __cdecl Scr_IsNodeOccupied()
{
    unsigned int v0; // r4
    bool v1; // r3

    if (Scr_GetNumParam() == 1)
    {
        v1 = level.time <= Scr_GetPathnode(0)->dynamic.iFreeTime;
        Scr_AddBool(v1);
    }
    else
    {
        Scr_Error("illegal call to isnodeoccupied()");
    }
}

void __cdecl Scr_SetTurretNode()
{
    unsigned int v0; // r4
    pathnode_t *Pathnode; // r30
    unsigned int v2; // r4
    gentity_s *Entity; // r31

    if (Scr_GetNumParam() == 2)
    {
        Pathnode = Scr_GetPathnode(0);
        if (Pathnode->constant.type == NODE_TURRET)
        {
            Entity = Scr_GetEntity((scr_entref_t *)1, v2);
            if (!Entity->pTurretInfo)
                Scr_Error("Entity is not a turret");
            Pathnode->dynamic.turretEntNumber = Entity->s.number;
        }
        else
        {
            Scr_Error("Can only set arc angle for node_turret");
        }
    }
    else
    {
        Scr_Error("Wrong number of arguments to setturretnode");
    }
}

void __cdecl Scr_UnsetTurretNode()
{
    unsigned int v0; // r4
    pathnode_t *Pathnode; // r3

    if (Scr_GetNumParam() == 1)
    {
        Pathnode = Scr_GetPathnode(0);
        if (Pathnode->constant.type == NODE_TURRET)
            Pathnode->dynamic.turretEntNumber = -1;
        else
            Scr_Error("Can only do this call for node_turret");
    }
    else
    {
        Scr_Error(" USAGE : Must have a node_turret as a parameter.");
    }
}

void __cdecl GScr_SetDynamicPathnodeField(pathnode_t *node, unsigned int index)
{
    unsigned int v3; // r3

    v3 = Path_ConvertNodeToIndex(node);
    Scr_SetDynamicEntityField(v3, 2u, index);
}

void __cdecl G_DropPathnodesToFloor()
{
    int v0; // r7
    int num_entities; // r8
    int v2; // r10
    int *v3; // r9
    gentity_s *v4; // r11
    int v5; // r5
    unsigned int v6; // r27
    int v7; // r29
    int v8; // r30
    pathnode_t *v9; // r11
    pathbasenode_t *v10; // r10
    int v11; // r7
    int v12; // r10
    int *v13; // r9
    _BYTE v14[576]; // [sp+50h] [-2240h] BYREF

    if (g_path.actualNodeCount > gameWorldSp.path.nodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            855,
            0,
            "%s",
            "g_path.actualNodeCount <= gameWorldSp.path.nodeCount");
    v0 = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        v2 = 0;
        v3 = (int *)v14;
        do
        {
            v4 = &level.gentities[v2];
            if (level.gentities[v2].r.inuse)
            {
                v5 = v4->flags & 0x100;
                *v3 = v4->r.contents;
                if (v5)
                {
                    v4->r.contents = 0;
                    num_entities = level.num_entities;
                }
            }
            ++v0;
            ++v2;
            ++v3;
        } while (v0 < num_entities);
    }
    v6 = 0;
    if (gameWorldSp.path.nodeCount)
    {
        v7 = 0;
        v8 = 0;
        do
        {
            node_droptofloor(&gameWorldSp.path.nodes[v8]);
            v9 = &gameWorldSp.path.nodes[v8];
            v10 = &gameWorldSp.path.basenodes[v7];
            v10->vOrigin[0] = gameWorldSp.path.nodes[v8].constant.vOrigin[0];
            v10->vOrigin[1] = v9->constant.vOrigin[1];
            v10->vOrigin[2] = v9->constant.vOrigin[2];
            gameWorldSp.path.basenodes[v7].type = 1 << gameWorldSp.path.nodes[v8].constant.type;
            if ((gameWorldSp.path.nodes[v8].constant.spawnflags & 1) != 0)
                gameWorldSp.path.basenodes[v7].type |= 0x100000u;
            ++v6;
            ++v8;
            ++v7;
        } while (v6 < gameWorldSp.path.nodeCount);
        num_entities = level.num_entities;
    }
    v11 = 0;
    if (num_entities > 0)
    {
        v12 = 0;
        v13 = (int *)v14;
        do
        {
            if (level.gentities[v12].r.inuse)
            {
                level.gentities[v12].r.contents = *v13;
                num_entities = level.num_entities;
            }
            ++v11;
            ++v12;
            ++v13;
        } while (v11 < num_entities);
    }
}

void __cdecl Scr_FreePathnode(pathnode_t *node)
{
    iassert(!node->dynamic.pOwner.isDefined());

    Scr_FreeEntityNum(Path_ConvertNodeToIndex(node), 2u);
}

void __cdecl Scr_AddPathnode(pathnode_t *node)
{
    unsigned int v1; // r3

    v1 = Path_ConvertNodeToIndex(node);
    Scr_AddEntityNum(v1, 2u);
}

void __cdecl Scr_GetNode()
{
    unsigned int ConstString; // r26
    const char *String; // r3
    int Offset; // r3
    int v3; // r31
    node_field_t *v4; // r27
    const pathnode_t *v5; // r3
    unsigned int actualNodeCount; // r10
    pathnode_t *nodes; // r11
    pathnode_t *v8; // r31
    unsigned int v9; // r3

    ConstString = Scr_GetConstString(0);
    String = Scr_GetString(1u);
    Offset = Scr_GetOffset(2u, String);
    v3 = Offset;
    if (Offset >= 0)
    {
        if ((unsigned int)Offset >= 0xB)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                1029,
                0,
                "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
                Offset,
                11);
        v4 = &fields_3[v3];
        if (v4->type != F_STRING)
            Scr_ParamError(1u, "key is not internally a string");
        v5 = 0;
        actualNodeCount = g_path.actualNodeCount;
        nodes = gameWorldSp.path.nodes;
        v8 = gameWorldSp.path.nodes;
        if (gameWorldSp.path.nodes != &gameWorldSp.path.nodes[g_path.actualNodeCount])
        {
            do
            {
                if (*(_WORD *)((char *)&v8->constant.type + v4->ofs)
                    && *(unsigned __int16 *)((char *)&v8->constant.type + v4->ofs) == ConstString)
                {
                    if (v5)
                    {
                        Scr_Error("getnode used with more than one node");
                        nodes = gameWorldSp.path.nodes;
                        actualNodeCount = g_path.actualNodeCount;
                    }
                    v5 = v8;
                }
                ++v8;
            } while (v8 != &nodes[actualNodeCount]);
            if (v5)
            {
                v9 = Path_ConvertNodeToIndex(v5);
                Scr_AddEntityNum(v9, 2u);
            }
        }
    }
}

void __cdecl Scr_GetNodeArray()
{
    unsigned int ConstString; // r27
    const char *String; // r30
    int Offset; // r31
    const char *v3; // r3
    node_field_t *v4; // r28
    unsigned int actualNodeCount; // r10
    pathnode_t *nodes; // r11
    pathnode_t *v7; // r31
    unsigned int v8; // r3

    ConstString = Scr_GetConstString(0);
    String = Scr_GetString(1u);
    Offset = Scr_GetOffset(2u, String);
    if (Offset < 0)
    {
        v3 = va("key '%s' does not internally belong to nodes", String);
        Scr_ParamError(1u, v3);
    }
    if ((unsigned int)Offset >= 0xB)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            1082,
            0,
            "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
            Offset,
            11);
    v4 = &fields_3[Offset];
    if (v4->type != F_STRING)
        Scr_ParamError(1u, "key is not internally a string");
    Scr_MakeArray();
    actualNodeCount = g_path.actualNodeCount;
    nodes = gameWorldSp.path.nodes;
    v7 = gameWorldSp.path.nodes;
    if (gameWorldSp.path.nodes != &gameWorldSp.path.nodes[g_path.actualNodeCount])
    {
        do
        {
            if (*(_WORD *)((char *)&v7->constant.type + v4->ofs))
            {
                if (*(unsigned __int16 *)((char *)&v7->constant.type + v4->ofs) == ConstString)
                {
                    v8 = Path_ConvertNodeToIndex(v7);
                    Scr_AddEntityNum(v8, 2u);
                    Scr_AddArray();
                    nodes = gameWorldSp.path.nodes;
                    actualNodeCount = g_path.actualNodeCount;
                }
            }
            ++v7;
        } while (v7 != &nodes[actualNodeCount]);
    }
}

void __cdecl Scr_GetAllNodes()
{
    unsigned int v0; // r31
    int v1; // r30
    unsigned int v2; // r3

    Scr_MakeArray();
    v0 = 0;
    if (g_path.actualNodeCount)
    {
        v1 = 0;
        do
        {
            v2 = Path_ConvertNodeToIndex(&gameWorldSp.path.nodes[v1]);
            Scr_AddEntityNum(v2, 2u);
            Scr_AddArray();
            ++v0;
            ++v1;
        } while (v0 < g_path.actualNodeCount);
    }
}

void __cdecl Path_Shutdown()
{
    pathnode_t *i; // r31
    pathnode_t *node; // r31

    Path_ShutdownBadPlaces();
    for (i = gameWorldSp.path.nodes; i != &gameWorldSp.path.nodes[g_path.actualNodeCount]; ++i)
    {
        if (i->constant.target && (SL_GetUser(i->constant.target) & 1) != 0)
            i->constant.target = 0;
        if (i->constant.targetname && (SL_GetUser(i->constant.targetname) & 1) != 0)
            i->constant.targetname = 0;
        if (i->constant.script_linkName && (SL_GetUser(i->constant.script_linkName) & 1) != 0)
            i->constant.script_linkName = 0;
        if (i->constant.script_noteworthy && (SL_GetUser(i->constant.script_noteworthy) & 1) != 0)
            i->constant.script_noteworthy = 0;
    }
    node = gameWorldSp.path.nodes;
    g_path.extraNodes = 0;
    for (g_path.originErrors = 0; node != &gameWorldSp.path.nodes[g_path.actualNodeCount]; ++node)
    {
        iassert(!node->dynamic.pOwner.isDefined());
        Scr_FreeEntityNum(Path_ConvertNodeToIndex(node), 2);
    }
    g_path.actualNodeCount = 0;
    g_pPath = 0;
}

void __cdecl Path_AutoDisconnectPaths()
{
    int v0; // r29
    int num_entities; // r10
    int v2; // r31
    gentity_s *v3; // r3
    int flags; // r11

    Path_ValidateAllNodes();
    v0 = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        v2 = 0;
        do
        {
            v3 = &level.gentities[v2];
            if (level.gentities[v2].r.inuse)
            {
                flags = v3->flags;
                if ((flags & 0x100) != 0 && (flags & 0x200) != 0)
                {
                    Path_DisconnectPathsForEntity(v3);
                    num_entities = level.num_entities;
                }
            }
            ++v0;
            ++v2;
        } while (v0 < num_entities);
    }
}

void __cdecl Path_InitPaths()
{
    Path_InitLinkInfoArray();
    Path_ValidateAllNodes();
}

void __cdecl Path_DrawVisData()
{
    gentity_s *v0; // r3
    const float *v1; // r6
    int v2; // r5
    const pathnode_t *v3; // r3
    const pathnode_t *v4; // r28
    int v5; // r20
    int i; // r26
    pathnode_t *v7; // r31
    const float *v8; // r6
    int v9; // r5
    const float *v10; // r5
    const float *v11; // r6
    int v12; // r5
    const float *v13; // r6
    int v14; // r5
    float v15[32]; // [sp+50h] [-80h] BYREF

    CL_GetViewPos(v15);
    v0 = G_Find(0, 284, scr_const.player);
    if (v0)
    {
        v3 = Sentient_NearestNode(v0->sentient);
        v4 = v3;
        if (v3)
        {
            Path_DrawDebugNode(v15, v3);
            v5 = 0;
            if (g_path.actualNodeCount)
            {
                for (i = 0; ; ++i)
                {
                    v7 = &gameWorldSp.path.nodes[i];
                    if (&gameWorldSp.path.nodes[i] == v4
                        || ai_showVisData->current.integer == 2 && !Path_IsCoverNode(v7)
                        || Vec2Distance(v4->constant.vOrigin, v7->constant.vOrigin) > (double)ai_showVisDataDist->current.value)
                    {
                        goto LABEL_17;
                    }
                    if (Path_ExpandedNodeVisible(v4, v7) && Path_NodesVisible(v4, v7))
                        break;
                    if (Path_ExpandedNodeVisible(v4, v7))
                    {
                        Path_DrawDebugNode(v15, v7);
                        v10 = colorRed;
                        goto LABEL_16;
                    }
                    if (Path_NodesVisible(v4, v7))
                    {
                        Path_DrawDebugNode(v15, v7);
                        v10 = colorYellow;
                        goto LABEL_16;
                    }
                LABEL_17:
                    if (++v5 >= g_path.actualNodeCount)
                        return;
                }
                Path_DrawDebugNode(v15, v7);
                v10 = colorGreen;
            LABEL_16:
                G_DebugLine(v4->constant.vOrigin, v7->constant.vOrigin, v10, 0);
                goto LABEL_17;
            }
        }
    }
}

void __cdecl Path_RelinquishNodeNow(sentient_s *claimer)
{
    Path_RelinquishNode(claimer, 0);
}

void __cdecl Path_RelinquishNodeSoon(sentient_s *claimer)
{
    Path_RelinquishNode(claimer, 5000);
}

void __cdecl Path_MarkNodeInvalid(pathnode_t *node, team_t eTeam)
{
    pathnode_dynamic_t *p_dynamic; // r23
    int v6; // r27
    __int16 *wOverlapNode; // r28
    int v8; // r11
    pathnode_t *v9; // r31

    iassert(node);
    iassert(eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES || eTeam == TEAM_NEUTRAL);

    p_dynamic = &node->dynamic;
    if (node->dynamic.pOwner.isDefined() && node->dynamic.pOwner.sentient()->eTeam == eTeam)
    {
        if (node->dynamic.pOwner.sentient()->pClaimedNode)
        {
            Path_RelinquishNode(node->dynamic.pOwner.sentient(), 5000);
        }
        node->dynamic.pOwner.setSentient(NULL);
    }
    *(&node->dynamic.iFreeTime + eTeam) = level.time + 5000;
    if (!node->dynamic.wOverlapCount)
    {
        v6 = 0;
        wOverlapNode = node->constant.wOverlapNode;
        node->dynamic.iFreeTime = 0;
        do
        {
            v8 = *wOverlapNode;
            if (v8 < 0)
                break;
            v9 = &gameWorldSp.path.nodes[v8];
            if (!v9->dynamic.wOverlapCount && v9->dynamic.iFreeTime != 0x7FFFFFFF)
            {
                if (v9->dynamic.pOwner.isDefined() && v9->dynamic.pOwner.sentient()->eTeam == eTeam)
                {
                    v9->dynamic.pOwner.setSentient(NULL);
                }
                v9->dynamic.iFreeTime = 0;
            }
            ++v6;
            ++wOverlapNode;
        } while (v6 < 2);
    }

    iassert(!node->dynamic.pOwner.isDefined() || node->dynamic.pOwner.sentient()->eTeam != eTeam);
}

void __cdecl G_SetPathnodeScriptVariable(const char *key, const char *value, pathnode_t *ent)
{
    unsigned int Field; // r30
    long double v6; // fp2
    int v7; // r3
    unsigned int v8; // r3
    const char *v9; // r3
    int v10; // [sp+50h] [-30h] BYREF

    Field = Scr_FindField(key, &v10);
    if (Field)
    {
        switch (v10)
        {
        case 2:
            Scr_AddString(value);
            goto LABEL_7;
        case 4:
            Scr_Error("G_SetPathnodeScriptVariable: vector is an unsupported script variable type for pathnodes");
            goto LABEL_7;
        case 5:
            v6 = atof(value);
            Scr_AddFloat((float)*(double *)&v6);
            goto LABEL_7;
        case 6:
            v7 = atol(value);
            Scr_AddInt(v7);
        LABEL_7:
            v8 = Path_ConvertNodeToIndex(ent);
            Scr_SetDynamicEntityField(v8, 2u, Field);
            break;
        default:
            if (!alwaysfails)
            {
                v9 = va("G_SetPathnodeScriptVariable: bad case %d", v10);
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 654, 0, v9);
            }
            break;
        }
    }
}

void __cdecl G_ParsePathnodeScriptField(const char *key, const char *value, pathnode_t *node)
{
    node_field_t *v6; // r28

    PathNode_UpdateStringField("target", &node->constant.target, key, value);
    PathNode_UpdateStringField("targetname", &node->constant.targetname, key, value);
    PathNode_UpdateStringField("script_linkname", &node->constant.script_linkName, key, value);
    PathNode_UpdateStringField("script_noteworthy", &node->constant.script_noteworthy, key, value);
    PathNode_UpdateFloatField("radius", &node->constant.fRadius, key, value);
    if (!I_stricmp(key, "origin"))
        PathNode_OriginMatches(value, node->constant.vOrigin);
    v6 = fields_3;
    if (fields_3[0].name)
    {
        while (I_stricmp(v6->name, key))
        {
            ++v6;
            if (!v6->name)
                goto LABEL_6;
        }
    }
    else
    {
    LABEL_6:
        G_SetPathnodeScriptVariable(key, value, node);
    }
}

void __cdecl G_ParsePathnodeScriptFields(pathnode_t *node)
{
    int v2; // r30
    const char **v3; // r31

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 768, 0, "%s", "level.spawnVar.spawnVarsValid");
    v2 = 0;
    if (level.spawnVar.numSpawnVars > 0)
    {
        v3 = (const char **)level.spawnVar.spawnVars[0];
        do
        {
            G_ParsePathnodeScriptField(*v3, v3[1], node);
            ++v2;
            v3 += 2;
        } while (v2 < level.spawnVar.numSpawnVars);
    }
}

int overCount;

void __cdecl G_SpawnPathnodeDynamic()
{
    unsigned int v0; // r6
    pathnode_t *v1; // r31
    pathnode_dynamic_t *p_dynamic; // r11
    int v3; // ctr
    pathnode_transient_t *p_transient; // r11
    int v5; // ctr
    float *v6; // r3
    nodeType type; // r30
    float v8[6]; // [sp+50h] [-30h] BYREF

    if (g_spawnai->current.enabled)
    {
        if (g_path.actualNodeCount < 0x2000)
        {
            if (g_path.actualNodeCount == gameWorldSp.path.nodeCount)
            {
                ++g_path.extraNodes;
            }
            else
            {
                v1 = &gameWorldSp.path.nodes[g_path.actualNodeCount++];
                if (g_path.actualNodeCount > gameWorldSp.path.nodeCount)
                {
                    p_dynamic = &v1->dynamic;
                    v3 = 8;
                    do
                    {
                        p_dynamic->pOwner = (SentientHandle)0;
                        p_dynamic = (pathnode_dynamic_t *)((char *)p_dynamic + 4);
                        --v3;
                    } while (v3);
                    p_transient = &v1->transient;
                    v5 = 7;
                    do
                    {
                        p_transient->iSearchFrame = 0;
                        p_transient = (pathnode_transient_t *)((char *)p_transient + 4);
                        --v5;
                    } while (v5);
                }
                G_ParsePathnodeScriptFields(v1);
                YawVectors(v1->constant.fAngle, v6, v8);
                v1->constant.forward[0] = v8[0];
                v1->constant.forward[1] = v8[1];
                type = v1->constant.type;
                if (v1->constant.type >= NODE_NUMTYPES)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 931, 0, "%s", "type < NODE_NUMTYPES");
                if (type == NODE_TURRET)
                    v1->dynamic.turretEntNumber = -1;
            }
        }
        else
        {
            v0 = ++overCount + g_path.actualNodeCount;
            Com_Printf(18, "PATH_MAX_NODES (%i) exceeded.  Nodecount: %d\n", 0x2000, v0);
        }
    }
}

int __cdecl Path_CanClaimNode(const pathnode_t *node, sentient_s *claimer)
{
    int iFreeTime; // r10
    sentient_s *NodeOwner; // r3
    int result; // r3
    bool v8; // zf

    iassert(node);
    iassert(claimer);
    iassert(claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES || claimer->eTeam == TEAM_NEUTRAL);

    if (node->dynamic.pOwner.isDefined() && node->dynamic.pOwner.sentient() == claimer)
        return 1;
    if (level.time <= *(&node->dynamic.iFreeTime + claimer->eTeam))
        return 0;
    iFreeTime = node->dynamic.iFreeTime;
    if (level.time > iFreeTime)
        return 1;
    if (iFreeTime == 0x7FFFFFFF && (unsigned __int8)Path_CanStealNode(node, claimer))
    {
        NodeOwner = Path_GetNodeOwner(node);
        Path_RelinquishNode(NodeOwner, 0);
        return 1;
    }
    if ((node->constant.spawnflags & 0x40) == 0)
        return 0;
    v8 = Path_CanStealPriorityNode(node, claimer) != 0;
    result = 1;
    if (!v8)
        return 0;
    return result;
}

void __cdecl Path_ForceClaimNode(pathnode_t *node, sentient_s *claimer)
{
    team_t eTeam; // r8
    pathnode_t *pClaimedNode; // r11
    int v6; // r11
    int v7; // r11
    __int16 v8; // r11
    int v9; // r11

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3145, 0, "%s", "node");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 3146, 0, "%s", "claimer");
    eTeam = claimer->eTeam;
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            3147,
            0,
            "%s\n\t(claimer->eTeam) = %i",
            "(claimer->eTeam == TEAM_AXIS || claimer->eTeam == TEAM_ALLIES || claimer->eTeam == TEAM_NEUTRAL)",
            eTeam);
    pClaimedNode = claimer->pClaimedNode;
    if (pClaimedNode && pClaimedNode != node)
        Path_RelinquishNode(claimer, 5000);
    if (node->dynamic.pOwner.isDefined() && node->dynamic.pOwner.sentient() == claimer)
    {
        if (claimer->pClaimedNode != node)
            Path_ClaimNode(node, claimer);
    }
    else
    {
        Path_RevokeClaim(node, claimer);
        v6 = node->constant.wOverlapNode[0];
        if (v6 >= 0)
        {
            Path_RevokeClaim(&gameWorldSp.path.nodes[v6], claimer);
            v7 = node->constant.wOverlapNode[1];
            if (v7 >= 0)
                Path_RevokeClaim(&gameWorldSp.path.nodes[v7], claimer);
        }
        if (node->dynamic.iFreeTime == 0x7FFFFFFF)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                3170,
                0,
                "%s",
                "node->dynamic.iFreeTime != INT_MAX");
        node->dynamic.pOwner.setSentient(claimer);
        v8 = node->constant.wOverlapNode[0];
        node->dynamic.iFreeTime = 0x7FFFFFFF;
        if (v8 >= 0)
        {
            Path_MarkNodeOverlap(&gameWorldSp.path.nodes[v8]);
            v9 = node->constant.wOverlapNode[1];
            if (v9 >= 0)
                Path_MarkNodeOverlap(&gameWorldSp.path.nodes[v9]);
        }
        Path_ClaimNodeInternal(node, claimer);
    }
}

pathnode_t *__cdecl Path_ChooseSubsequentChainNode_r(
    int depthMin,
    int depthMax,
    pathnode_t *pParent,
    actor_s *claimer)
{
    __int16 wChainId; // r9
    int v9; // r23
    __int16 v10; // r10
    int v11; // r24
    pathnode_t *v12; // r25
    int v13; // r11
    unsigned int v14; // r28
    int v15; // r26
    unsigned int v16; // r29
    pathnode_t *v17; // r31
    int wChainDepth; // r11
    pathnode_t *v19; // r3
    int v20; // r11
    pathnode_t *v22; // [sp+50h] [-70h] BYREF
    unsigned int v23[27]; // [sp+54h] [-6Ch] BYREF

    if (depthMin > depthMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2404, 0, "%s", "depthMin <= depthMax");
    if (!pParent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2405, 0, "%s", "pParent");
    if (pParent->constant.wChainDepth >= depthMax)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2406,
            0,
            "%s",
            "pParent->constant.wChainDepth < depthMax");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2407, 0, "%s", "claimer");
    wChainId = pParent->constant.wChainId;
    v9 = depthMax;
    v10 = pParent - gameWorldSp.path.nodes;
    if (pParent->constant.wChainDepth + 1 < depthMax)
        v9 = pParent->constant.wChainDepth + 1;
    v11 = v10;
    v12 = 0;
    v23[0] = 0;
    v13 = gameWorldSp.path.chainNodeForNode[v10];
    v22 = 0;
    v14 = v13 + 1;
    if (v13 + 1 < gameWorldSp.path.chainNodeCount)
    {
        v15 = wChainId;
        v16 = v14;
        do
        {
            v17 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(gameWorldSp.path.nodeForChainNode[v16], 7));
            if (v17->constant.wChainId != v15)
                break;
            wChainDepth = v17->constant.wChainDepth;
            if (wChainDepth > v9)
                break;
            if (v17->constant.wChainParent == v11)
            {
                if (wChainDepth < depthMax)
                {
                    v19 = Path_ChooseSubsequentChainNode_r(depthMin, depthMax, v17, claimer);
                    if (v19)
                    {
                        Path_UpdateBestChainNode(v19, &v22, v23);
                        v12 = v22;
                    }
                }
                v20 = v17->constant.wChainDepth;
                if ((v20 == depthMax || !v12 && v20 >= depthMin)
                    && Path_CanClaimNode(v17, claimer->sentient)
                    && (unsigned __int8)Path_CanSetDesiredChainPos(claimer, v17))
                {
                    Path_UpdateBestChainNode(v17, &v22, v23);
                    v12 = v22;
                }
            }
            ++v14;
            ++v16;
        } while (v14 < gameWorldSp.path.chainNodeCount);
    }
    return v12;
}

pathnode_t *__cdecl Path_ChooseAnyChainNodeIfDeadEnd(
    int depthMin,
    int depthMax,
    pathnode_t *chainPos,
    actor_s *claimer)
{
    __int16 wChainId; // r10
    int v9; // r26
    unsigned int v10; // r29
    int v11; // r27
    unsigned int v12; // r28
    pathnode_t *v13; // r30
    int wChainDepth; // r11
    pathnode_t *v16; // [sp+50h] [-60h] BYREF
    unsigned int v17; // [sp+54h] [-5Ch] BYREF

    if (depthMin > depthMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2457, 0, "%s", "depthMin <= depthMax");
    if (!chainPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2458, 0, "%s", "chainPos");
    if (chainPos->constant.wChainDepth >= depthMax)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2459,
            0,
            "%s",
            "chainPos->constant.wChainDepth < depthMax");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2460, 0, "%s", "claimer");
    wChainId = chainPos->constant.wChainId;
    v9 = (__int16)(chainPos - gameWorldSp.path.nodes);
    v17 = 0;
    v16 = 0;
    v10 = gameWorldSp.path.chainNodeForNode[v9] + 1;
    if (v10 < gameWorldSp.path.chainNodeCount)
    {
        v11 = wChainId;
        v12 = v10;
        do
        {
            v13 = (pathnode_t *)((char *)gameWorldSp.path.nodes + __ROL4__(gameWorldSp.path.nodeForChainNode[v12], 7));
            if (v13->constant.wChainId != v11)
                break;
            wChainDepth = v13->constant.wChainDepth;
            if (wChainDepth > depthMax)
                break;
            if (v13->constant.wChainParent == v9)
                return 0;
            if (wChainDepth >= depthMin && Path_CanClaimNode(v13, claimer->sentient))
            {
                if ((unsigned __int8)Path_CanSetDesiredChainPos(claimer, v13))
                    Path_UpdateBestChainNode(v13, &v16, &v17);
            }
            ++v10;
            ++v12;
        } while (v10 < gameWorldSp.path.chainNodeCount);
    }
    return v16;
}

pathnode_t *__cdecl Path_ChoosePreviousChainNode(int depthMin, int depthMax, pathnode_t *chainPos, actor_s *claimer)
{
    int wChainParent; // r11
    int v9; // r11

    if (depthMin > depthMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2500, 0, "%s", "depthMin <= depthMax");
    if (!chainPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2501, 0, "%s", "chainPos");
    if (chainPos->constant.wChainDepth < depthMin)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2502,
            0,
            "%s",
            "chainPos->constant.wChainDepth >= depthMin");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2503, 0, "%s", "claimer");
    for (; chainPos->constant.wChainDepth > depthMax; chainPos = &gameWorldSp.path.nodes[wChainParent])
    {
        wChainParent = chainPos->constant.wChainParent;
        if (wChainParent < 0)
            return 0;
    }
    if (chainPos->constant.wChainDepth < depthMin)
        return 0;
    while (!Path_CanClaimNode(chainPos, claimer->sentient)
        || !(unsigned __int8)Path_CanSetDesiredChainPos(claimer, chainPos))
    {
        v9 = chainPos->constant.wChainParent;
        if (v9 >= 0)
        {
            chainPos = &gameWorldSp.path.nodes[v9];
            if (chainPos->constant.wChainDepth >= depthMin)
                continue;
        }
        return 0;
    }
    return chainPos;
}

pathnode_t *__cdecl Path_ChooseDesperationChainNode(
    int depthMin,
    int depthMax,
    pathnode_t *refPos,
    actor_s *claimer)
{
    const pathnode_t *v8; // r25
    int v9; // r26
    pathnode_t *nodes; // r10
    unsigned int v11; // r28
    unsigned int chainNodeCount; // r9
    unsigned int v13; // r27
    const pathnode_t *v14; // r31
    int wChainDepth; // r11
    int v16; // r30

    if (!refPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2534, 0, "%s", "refPos");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2535, 0, "%s", "claimer");
    if (depthMin > depthMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2536, 0, "%s", "depthMin <= depthMax");
    if ((unsigned __int16)refPos->constant.wChainParent != 0xFFFF)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2537,
            0,
            "%s",
            "refPos->constant.wChainParent == -1");
    v8 = 0;
    v9 = 0x7FFFFFFF;
    nodes = gameWorldSp.path.nodes;
    v11 = gameWorldSp.path.chainNodeForNode[refPos - gameWorldSp.path.nodes];
    chainNodeCount = gameWorldSp.path.chainNodeCount;
    if (v11 < gameWorldSp.path.chainNodeCount)
    {
        v13 = v11;
        do
        {
            v14 = (pathnode_t *)((char *)nodes + __ROL4__(gameWorldSp.path.nodeForChainNode[v13], 7));
            if (v14->constant.wChainId != refPos->constant.wChainId)
                return (pathnode_t *)v8;
            wChainDepth = v14->constant.wChainDepth;
            if (wChainDepth >= depthMin)
            {
                if (wChainDepth <= depthMax)
                    goto LABEL_21;
                v16 = wChainDepth - depthMax;
            }
            else
            {
                v16 = depthMin - wChainDepth;
            }
            if (v16 <= v9)
            {
                if (Path_CanClaimNode(v14, claimer->sentient))
                {
                    if ((unsigned __int8)Path_CanSetDesiredChainPos(claimer, v14))
                    {
                        v9 = v16;
                        v8 = v14;
                        if (depthMax < v14->constant.wChainDepth)
                            return (pathnode_t *)v8;
                    }
                }
                chainNodeCount = gameWorldSp.path.chainNodeCount;
                nodes = gameWorldSp.path.nodes;
            }
        LABEL_21:
            ++v11;
            ++v13;
        } while (v11 < chainNodeCount);
    }
    return (pathnode_t *)v8;
}

pathnode_t *__cdecl Path_ChooseDesperationNewChainNode(
    int depthMin,
    int depthMax,
    pathnode_t *refPos,
    actor_s *claimer)
{
    const pathnode_t *v8; // r25
    int v9; // r26
    pathnode_t *nodes; // r10
    unsigned int v11; // r28
    unsigned int chainNodeCount; // r9
    unsigned int v13; // r27
    const pathnode_t *v14; // r31
    int wChainDepth; // r11
    int v16; // r30

    if (!refPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2588, 0, "%s", "refPos");
    if (!claimer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2589, 0, "%s", "claimer");
    if (depthMin > depthMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2590, 0, "%s", "depthMin <= depthMax");
    if ((unsigned __int16)refPos->constant.wChainParent != 0xFFFF)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2591,
            0,
            "%s",
            "refPos->constant.wChainParent == -1");
    v8 = 0;
    v9 = 0x7FFFFFFF;
    nodes = gameWorldSp.path.nodes;
    v11 = gameWorldSp.path.chainNodeForNode[refPos - gameWorldSp.path.nodes];
    chainNodeCount = gameWorldSp.path.chainNodeCount;
    if (v11 < gameWorldSp.path.chainNodeCount)
    {
        v13 = v11;
        do
        {
            v14 = (pathnode_t *)((char *)nodes + __ROL4__(gameWorldSp.path.nodeForChainNode[v13], 7));
            if (v14->constant.wChainId != refPos->constant.wChainId)
                break;
            if (level.time <= v14->dynamic.iFreeTime)
            {
                wChainDepth = v14->constant.wChainDepth;
                if (wChainDepth >= depthMin)
                {
                    v16 = wChainDepth - depthMax;
                    if (wChainDepth <= depthMax)
                        v16 = 0;
                }
                else
                {
                    v16 = depthMin - wChainDepth;
                }
                if (v16 <= v9)
                {
                    if (Path_CanClaimNode(v14, claimer->sentient))
                    {
                        if ((unsigned __int8)Path_CanSetDesiredChainPos(claimer, v14))
                        {
                            v9 = v16;
                            v8 = v14;
                            if (depthMax < v14->constant.wChainDepth)
                                return (pathnode_t *)v8;
                        }
                    }
                    chainNodeCount = gameWorldSp.path.chainNodeCount;
                    nodes = gameWorldSp.path.nodes;
                }
            }
            ++v11;
            ++v13;
        } while (v11 < chainNodeCount);
    }
    return (pathnode_t *)v8;
}

pathnode_t *__cdecl Path_ChooseChainPos(
    pathnode_t *refPos,
    int iFollowMin,
    int iFollowMax,
    actor_s *claimer,
    int chainFallback)
{
    pathnode_t *v5; // r31
    signed int v10; // r30
    pathnode_t *pDesiredChainPos; // r28
    int wChainDepth; // r11
    __int16 wChainId; // r23
    int v14; // r29
    int v15; // r30
    int v16; // r11
    pathnode_t *result; // r3
    int i; // r11

    v5 = refPos;
    if (!refPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2647, 0, "%s", "refPos");
    if (iFollowMin > iFollowMax)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 2648, 0, "%s", "iFollowMin <= iFollowMax");
    v10 = v5 - gameWorldSp.path.nodes;
    if (v10 < 0 || gameWorldSp.path.chainNodeForNode[v10] >= gameWorldSp.path.chainNodeCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2653,
            0,
            "%s",
            "refNodeIndex >= 0 && gameWorldSp.path.chainNodeForNode[refNodeIndex] < gameWorldSp.path.chainNodeCount");
    if (v5 != &gameWorldSp.path.nodes[v10])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
            2654,
            0,
            "%s",
            "refPos == &gameWorldSp.path.nodes[refNodeIndex]");
    pDesiredChainPos = claimer->pDesiredChainPos;
    wChainDepth = v5->constant.wChainDepth;
    wChainId = v5->constant.wChainId;
    v14 = wChainDepth + iFollowMin;
    v15 = wChainDepth + iFollowMax;
    if (!pDesiredChainPos || pDesiredChainPos->constant.wChainId != wChainId)
        goto LABEL_39;
    v16 = pDesiredChainPos->constant.wChainDepth;
    if (v16 == v15)
        return pDesiredChainPos;
    if (v16 <= v15)
    {
        result = Path_ChooseSubsequentChainNode_r(v14, v15, claimer->pDesiredChainPos, claimer);
        if (result)
            return result;
        result = Path_ChooseAnyChainNodeIfDeadEnd(v14, v15, pDesiredChainPos, claimer);
        if (result)
            return result;
        v16 = pDesiredChainPos->constant.wChainDepth;
        if (v16 >= v14)
            return pDesiredChainPos;
    }
    if (!chainFallback)
        return pDesiredChainPos;
    if (v16 < v14 || (result = Path_ChoosePreviousChainNode(v14, v15, pDesiredChainPos, claimer)) == 0)
    {
    LABEL_39:
        if ((iFollowMax <= 0 || (result = Path_ChooseSubsequentChainNode_r(v14, v15, v5, claimer)) == 0)
            && (iFollowMin > 0 || (result = Path_ChoosePreviousChainNode(v14, v15, v5, claimer)) == 0))
        {
            for (i = v5->constant.wChainParent; i >= 0; i = v5->constant.wChainParent)
                v5 = &gameWorldSp.path.nodes[i];
            if (v5->constant.wChainDepth >= v15 || (result = Path_ChooseSubsequentChainNode_r(v14, v15, v5, claimer)) == 0)
            {
                if (pDesiredChainPos && pDesiredChainPos->constant.wChainId == wChainId)
                    return pDesiredChainPos;
                if ((unsigned __int16)v5->constant.wChainParent != 0xFFFF)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp",
                        2737,
                        0,
                        "%s",
                        "refPos->constant.wChainParent == -1");
                result = Path_ChooseDesperationChainNode(v14, v15, v5, claimer);
                if (!result)
                {
                    if (pDesiredChainPos)
                        return 0;
                    else
                        return Path_ChooseDesperationNewChainNode(v14, v15, v5, claimer);
                }
            }
        }
    }
    return result;
}

pathnode_t *__cdecl Path_NearestNodeNotCrossPlanes(
    const float *const vOrigin,
    pathsort_t *nodes,
    int typeFlags,
    float fMaxDist,
    float (*vNormal)[2],
    const float *fDist,
    int iPlaneCount,
    int *returnCount,
    int maxNodes,
    nearestNodeHeightCheck heightCheck)
{
    int v39; // r5
    pathsort_t *v40; // r4
    float *pOrigin; // r3
    double maxHeight; // fp2
    int numNodes; // r28
    int v44; // r24
    int v45; // r25
    pathnode_t **v46; // r23
    pathnode_t *node; // r29
    double v48; // fp13
    double v49; // fp13
    double v50; // fp13
    int v51; // r9
    float *v52; // r10
    float *v53; // r11
    int v54; // r30
    unsigned int *v55; // r31
    const float *v56; // r7
    int v58[2]; // [sp+50h] [-4C0h] BYREF
    float v59[4]; // [sp+58h] [-4B8h] BYREF
    float v60[4]; // [sp+68h] [-4A8h] BYREF
    float v61[6]; // [sp+78h] [-498h] BYREF
    unsigned int v62[256]; // [sp+90h] [-480h] BYREF

    //Profile_Begin(232);
    if (a31)
    {
        pOrigin = vOrigin;
        maxHeight = 1000000000.0;
    }
    else
    {
        v61[0] = vOrigin[0];
        v61[1] = vOrigin[1];
        v61[2] = vOrigin[2] - (float)120.0;
        pOrigin = v61;
        maxHeight = 184.0;
    }
    numNodes = Path_NodesInCylinder(pOrigin, fMaxDist, maxHeight, nodes, a29, typeFlags);
    std::_Sort<pathsort_t *, int, bool(__cdecl *)(pathsort_t const &, pathsort_t const &)>(
        nodes,
        &nodes[(unsigned int)v43],
        12 * (int)v43 / 12,
        (bool(__cdecl *)(const pathsort_t *, const pathsort_t *))Path_CompareNodesIncreasing);
    v44 = 0;
    v45 = 0;
    v59[0] = -15.0;
    v59[1] = -15.0;
    v60[0] = 15.0;
    v60[1] = 15.0;
    v60[2] = 72.0;
    *maxNodes = numNodes;
    v59[2] = (float)0.0 + (float)17.0;
    if ((int)numNodes > 0)
    {
        v46 = (pathnode_t **)v62;
        while (1)
        {
            node = nodes->node;
            if (!nodes->node->dynamic.wLinkCount)
                goto failed_node;
            v48 = *vOrigin;
            if (v48 > (float)((float)(node->constant.vOrigin[0] + -15.0) - (float)1.0)
                && v48 < (float)((float)(node->constant.vOrigin[0] + 15.0) + (float)1.0))
            {
                v49 = vOrigin[1];
                if (v49 > (float)((float)(node->constant.vOrigin[1] + -15.0) - (float)1.0)
                    && v49 < (float)((float)(node->constant.vOrigin[1] + 15.0) + (float)1.0))
                {
                    v50 = vOrigin[2];
                    if (v50 > (float)((float)(node->constant.vOrigin[2] + 0.0) - (float)1.0)
                        && v50 < (float)((float)(node->constant.vOrigin[2] + 72.0) + (float)1.0))
                    {
                    LABEL_25:
                        //Profile_EndInternal(0);
                        return node;
                    }
                }
            }
            v51 = 0;
            if ((int)returnCount > 0)
                break;
        LABEL_17:
            v58[0] = 0;
            SV_SightTrace(v58, vOrigin, v59, v60, node->constant.vOrigin, 2175, 2175, 8519697);
            if (!v58[0])
                goto LABEL_25;
        LABEL_18:
            ++v45;
            ++nodes;
            if (v45 >= (int)numNodes)
                goto LABEL_19;
        }
        v52 = iPlaneCount;
        v53 = fDist;
        while ((float)((float)(v53[1] * node->constant.vOrigin[1]) + (float)(node->constant.vOrigin[0] * *v53)) <= (double)*v52)
        {
            ++v51;
            v53 += 2;
            ++v52;
            if (v51 >= (int)returnCount)
                goto LABEL_17;
        }
    failed_node:
        *v46 = node;
        ++v44;
        ++v46;
        goto LABEL_18;
    }
LABEL_19:
    v54 = 0;
    if (v44 <= 0)
    {
    LABEL_23:
        //Profile_EndInternal(0);
        return 0;
    }
    else
    {
        v55 = v62;
        while (1)
        {
            v56 = (const float *)(*v55 + 20);
            v58[0] = 0;
            SV_SightTrace(v58, vOrigin, v59, v60, v56, 2175, 2175, 8519697);
            if (!v58[0])
                break;
            ++v54;
            ++v55;
            if (v54 >= v44)
                goto LABEL_23;
        }
        //Profile_EndInternal(0);
        return (pathnode_t *)v62[v54];
    }
}

pathnode_t *__cdecl Path_NearestNode(
    const float *const vOrigin,
    pathsort_t *nodes,
    int typeFlags,
    float fMaxDist,
    int *returnCount,
    int maxNodes,
    nearestNodeHeightCheck heightCheck)
{
    return Path_NearestNodeNotCrossPlanes(
        vOrigin,
        nodes,
        typeFlags,
        fMaxDist,
        (float (*)[2])returnCount,
        0,
        0,
        0,
        maxNodes,
        heightCheck);
}

void __cdecl Path_DrawDebugNearestNode(float *vOrigin, int numNodes)
{
    const float *v3; // r6
    int v4; // r5
    pathnode_t *v5; // r4
    const dvar_s *v6; // r11
    int v7; // r28
    int v8; // r31
    pathsort_t *v9; // r30
    nearestNodeHeightCheck v10; // [sp+8h] [-C98h]
    int v30[4]; // [sp+60h] [-C40h] BYREF
    pathsort_t v31[260]; // [sp+70h] [-C30h] BYREF

    v5 = Path_NearestNodeNotCrossPlanes(
        vOrigin,
        v31,
        -1,
        ai_showNodesDist->current.value,
        (float (*)[2])0x100,
        0,
        0,
        0,
        256,
        NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    v6 = ai_showNearestNode;
    if (ai_showNearestNode->current.integer <= 1)
    {
        if (v5)
            Path_DrawDebugNode(vOrigin, v5);
        else
            G_DebugCircle(vOrigin, 192.0, 0, (int)colorRed, 1, 1);
    }
    else
    {
        v7 = v30[0];
        v8 = 0;
        if (v30[0] > 0)
        {
            v9 = v31;
            while (v8 < v6->current.integer)
            {
                Path_DrawDebugNode(vOrigin, v9->node);
                ++v8;
                ++v9;
                if (v8 >= v7)
                    break;
                v6 = ai_showNearestNode;
            }
        }
    }
}

void __cdecl Path_DrawDebugClaimedNodes(float *origin, int numNodes)
{
    const float *v3; // r6
    int v4; // r5
    int v5; // r4
    int v6; // r14
    int v7; // r16
    pathsort_t *v8; // r17
    const pathnode_t *node; // r29
    float *vOrigin; // r31
    double DebugStringScale; // fp1
    double v12; // fp30
    const char *v13; // r5
    sentient_s *v14; // r3
    const char *v15; // r6
    const float *v16; // r4
    __int64 v17; // r11
    double v18; // r4
    char *v19; // r3
    const char *v20; // r5
    double v21; // r4
    char *v22; // r3
    const char *v23; // r5
    double v24; // r4
    char *v25; // r3
    const char *v26; // r5
    nearestNodeHeightCheck v27; // [sp+8h] [-D48h]
    float v47[2]; // [sp+60h] [-CF0h] BYREF
    float v48; // [sp+68h] [-CE8h]
    const char *v49; // [sp+6Ch] [-CE4h]
    const char *v50; // [sp+70h] [-CE0h]
    _QWORD v51[3]; // [sp+78h] [-CD8h] BYREF
    pathsort_t v52[259]; // [sp+90h] [-CC0h] BYREF

    v50 = "c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp";
    v49 = "%s";
    if (!origin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.cpp", 1776, 0, "%s", "origin");
    Path_NearestNodeNotCrossPlanes(
        origin,
        v52,
        -1,
        ai_showNodesDist->current.value,
        (float (*)[2])1,
        0,
        0,
        0,
        256,
        NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    v6 = HIDWORD(v51[0]);
    v7 = 0;
    if (SHIDWORD(v51[0]) > 0)
    {
        v8 = v52;
        do
        {
            if (v7 >= numNodes)
                break;
            node = v8->node;
            vOrigin = v8->node->constant.vOrigin;
            Path_DrawDebugNodeBox(v8->node);
            DebugStringScale = Path_GetDebugStringScale(origin, node->constant.vOrigin);
            v47[0] = *vOrigin;
            v12 = (float)((float)DebugStringScale * (float)0.5);
            v47[1] = node->constant.vOrigin[1];
            v48 = node->constant.vOrigin[2];
            if (node->dynamic.pOwner.isDefined())
            {
                iassert(node->dynamic.pOwner.sentient()->ent);
                v14 = node->dynamic.pOwner.sentient();
                v15 = va("Owner: %d", v14->ent->s.number);
                v16 = colorGreen;
            }
            else
            {
                v15 = "Owner: None";
                v16 = colorWhite;
            }
            G_AddDebugString(v47, v16, v12, v13);
            LODWORD(v17) = level.time;
            v48 = (float)((float)v12 * (float)12.0) + v48;
            HIDWORD(v17) = node->dynamic.iValidTime[1];
            if (level.time < SHIDWORD(v17))
            {
                LODWORD(v17) = HIDWORD(v17) - level.time;
                v51[0] = v17;
                v18 = (float)((float)v17 * (float)0.001);
                v19 = va((const char *)HIDWORD(v18), LODWORD(v18));
                G_AddDebugString(v47, colorYellow, v12, v20);
                LODWORD(v17) = level.time;
                v48 = (float)((float)v12 * (float)12.0) + v48;
            }
            HIDWORD(v17) = node->dynamic.iValidTime[0];
            if ((int)v17 <= SHIDWORD(v17))
            {
                LODWORD(v17) = HIDWORD(v17) - v17;
                v51[1] = v17;
                v21 = (float)((float)v17 * (float)0.001);
                v22 = va((const char *)HIDWORD(v21), LODWORD(v21));
                G_AddDebugString(v47, colorYellow, v12, v23);
                LODWORD(v17) = level.time;
                v48 = (float)((float)v12 * (float)12.0) + v48;
            }
            HIDWORD(v17) = node->dynamic.iFreeTime;
            if (HIDWORD(v17) != 0x7FFFFFFF && (int)v17 < SHIDWORD(v17))
            {
                LODWORD(v17) = HIDWORD(v17) - v17;
                v51[2] = v17;
                v24 = (float)((float)v17 * (float)0.001);
                v25 = va((const char *)HIDWORD(v24), LODWORD(v24));
                G_AddDebugString(v47, colorYellow, v12, v26);
                v48 = (float)((float)v12 * (float)12.0) + v48;
            }
            ++v7;
            ++v8;
        } while (v7 < v6);
    }
}

void __cdecl Path_DrawDebug()
{
    unsigned int v0; // r24
    int v1; // r25
    pathnode_t *v2; // r29
    int integer; // r11
    char *v4; // r3
    const char *v5; // r5
    bool v6; // r28
    unsigned int wLinkCount; // r30
    unsigned int i; // r31
    float v9; // [sp+50h] [-A0h] BYREF
    float v10; // [sp+54h] [-9Ch]

    if (level.gentities->client)
    {
        if (ai_showNodes->current.integer)
        {
            CL_GetViewPos(&v9);
            v0 = 0;
            if (g_path.actualNodeCount)
            {
                v1 = 0;
                do
                {
                    v2 = &gameWorldSp.path.nodes[v1];
                    if (ai_showNodesDist->current.value == 0.0
                        || (float)((float)((float)(v2->constant.vOrigin[0] - v9) * (float)(v2->constant.vOrigin[0] - v9))
                            + (float)((float)(v2->constant.vOrigin[1] - v10) * (float)(v2->constant.vOrigin[1] - v10))) <= (double)(float)(ai_showNodesDist->current.value * ai_showNodesDist->current.value))
                    {
                        integer = ai_showNodes->current.integer;
                        if (integer == 2 || integer == 4)
                        {
                            v4 = va("%i", v0);
                            G_AddDebugString(v2->constant.vOrigin, colorWhite, 1.0, v5);
                        }
                        v6 = ai_showNodes->current.integer >= 3;
                        if (ai_showNodes->current.integer < 3)
                            wLinkCount = v2->dynamic.wLinkCount;
                        else
                            wLinkCount = v2->constant.totalLinkCount;
                        if (wLinkCount)
                        {
                            for (i = 0; i < wLinkCount; ++i)
                                Path_DrawDebugLink(v2, i, v6);
                        }
                        else
                        {
                            Path_DrawDebugNoLinks(v2, (const float (*)[4])colorRed, 0);
                        }
                    }
                    ++v0;
                    ++v1;
                } while (v0 < g_path.actualNodeCount);
            }
        }
        if (ai_showNearestNode->current.integer)
        {
            CL_GetViewPos(&v9);
            Path_DrawDebugNearestNode(&v9, ai_showNearestNode->current.integer);
        }
        if (ai_debugClaimedNodes->current.integer)
        {
            CL_GetViewPos(&v9);
            Path_DrawDebugClaimedNodes(&v9, ai_debugClaimedNodes->current.integer);
        }
        if (ai_debugFindPath->current.integer)
            Path_DrawDebugFindPath(level.gentities->client->ps.origin);
        if (ai_showFriendlyChains->current.integer)
            Path_DrawFriendlyChain();
        if (ai_showVisData->current.integer)
            Path_DrawVisData();
    }
}

