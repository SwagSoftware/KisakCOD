#include "qcommon.h"
#include <xanim/xmodel.h>
#include <universal/com_memory.h>
#include "com_bsp.h"
#include <universal/q_parse.h>
#include <DynEntity/DynEntity_client.h>
#include <game/game_public.h>

struct DiskCollBorder // sizeof=0x1C
{
    float distEq[3];
    float zBase;
    float zSlope;
    float start;
    float length;
};

struct DiskLeaf_Version14 // sizeof=0x24
{
    int cluster;
    int unused0;
    int firstCollAabbIndex;
    int collAabbCount;
    int firstLeafBrush;
    int numLeafBrushes;
    int cellNum;
    int unused1;
    int unused2;
};

struct dbrush_t // sizeof=0x4
{
    __int16 numSides;
    __int16 materialNum;
};

union dbrushside_t_u // sizeof=0x4
{                                       // ...
    int planeNum;
    float bound;
};
struct dbrushside_t // sizeof=0x8
{
    dbrushside_t_u u;
    int materialNum;
};

void __cdecl CM_InitStaticModel(cStaticModel_s *staticModel, float *origin, float *angles, float scale)
{
    float v4; // [esp+10h] [ebp-48h]
    float axis[3][3]; // [esp+34h] [ebp-24h] BYREF

    if ((COERCE_UNSIGNED_INT(*origin) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            20,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*angles) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            21,
            0,
            "%s",
            "!IS_NAN((angles)[0]) && !IS_NAN((angles)[1]) && !IS_NAN((angles)[2])");
    }
    if ((LODWORD(scale) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 22, 0, "%s", "!IS_NAN(scale)");
    staticModel->origin[0] = *origin;
    staticModel->origin[1] = origin[1];
    staticModel->origin[2] = origin[2];
    AnglesToAxis(angles, axis);
    MatrixTranspose(axis, staticModel->invScaledAxis);
    v4 = 1.0 / scale;
    Vec3Scale(staticModel->invScaledAxis[0], v4, staticModel->invScaledAxis[0]);
    Vec3Scale(staticModel->invScaledAxis[1], v4, staticModel->invScaledAxis[1]);
    Vec3Scale(staticModel->invScaledAxis[2], v4, staticModel->invScaledAxis[2]);
    Vec3Scale(axis[0], scale, axis[0]);
    Vec3Scale(axis[1], scale, axis[1]);
    Vec3Scale(axis[2], scale, axis[2]);
    if (XModelGetStaticBounds(staticModel->xmodel, axis, staticModel->absmin, staticModel->absmax))
    {
        Vec3Add(staticModel->absmin, origin, staticModel->absmin);
        Vec3Add(staticModel->absmax, origin, staticModel->absmax);
        if (staticModel->absmax[0] - staticModel->absmin[0] < 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                45,
                0,
                "%s",
                "staticModel->absmax[0] - staticModel->absmin[0] >= 0");
        if (staticModel->absmax[1] - staticModel->absmin[1] < 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                46,
                0,
                "%s",
                "staticModel->absmax[1] - staticModel->absmin[1] >= 0");
        if (staticModel->absmax[2] - staticModel->absmin[2] < 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                47,
                0,
                "%s",
                "staticModel->absmax[2] - staticModel->absmin[2] >= 0");
    }
}

unsigned __int8 *__cdecl CM_Hunk_AllocXModel(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocXModel", 21);
}
unsigned __int8 *__cdecl CM_Hunk_AllocXModelColl(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocXModelColl", 27);
}

unsigned __int8 *__cdecl CM_Hunk_Alloc(unsigned int size, const char *name, int type)
{
    return Hunk_Alloc(size, name, type);
}

XModel *__cdecl CM_XModelPrecache(char *name)
{
    return XModelPrecache(
        name,
        (void *(__cdecl *)(int))CM_Hunk_AllocXModel,
        (void *(__cdecl *)(int))CM_Hunk_AllocXModelColl);
}

char __cdecl CM_CreateStaticModel(cStaticModel_s *staticModel, char *name, float *origin, float *angles, float scale)
{
    XModel *model; // [esp+34h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(*origin) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            60,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*angles) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            61,
            0,
            "%s",
            "!IS_NAN((angles)[0]) && !IS_NAN((angles)[1]) && !IS_NAN((angles)[2])");
    }
    if ((LODWORD(scale) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 62, 0, "%s", "!IS_NAN(scale)");
    if (!name || !*name)
        Com_Error(ERR_DROP, "Missing model name at %.0f %.0f %.0f", *origin, origin[1], origin[2]);
    if (scale == 0.0)
        Com_Error(ERR_DROP, "Static model [%s] has scale of 0.0", name);
    model = CM_XModelPrecache(name);
    if (!model)
        return 0;
    staticModel->xmodel = model;
    CM_InitStaticModel(staticModel, origin, angles, scale);
    return 1;
}

void __cdecl CM_LoadStaticModels()
{
    char v0; // [esp+7h] [ebp-115h]
    char *v1; // [esp+Ch] [ebp-110h]
    char *v2; // [esp+10h] [ebp-10Ch]
    char v3; // [esp+17h] [ebp-105h]
    char *v4; // [esp+1Ch] [ebp-100h]
    char *v5; // [esp+20h] [ebp-FCh]
    unsigned int numStaticModels; // [esp+24h] [ebp-F8h]
    float origin[3]; // [esp+28h] [ebp-F4h] BYREF
    char modelName[64]; // [esp+34h] [ebp-E8h] BYREF
    float angles[3]; // [esp+74h] [ebp-A8h] BYREF
    int bMiscModel; // [esp+80h] [ebp-9Ch]
    char key[68]; // [esp+84h] [ebp-98h] BYREF
    const char *ptr; // [esp+C8h] [ebp-54h] BYREF
    const char *token; // [esp+CCh] [ebp-50h]
    float scale; // [esp+D0h] [ebp-4Ch]
    char value[68]; // [esp+D4h] [ebp-48h] BYREF

    ptr = Com_EntityString(0);
    if (!ptr)
        MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 100, 0, "%s", "ptr");
    cm.numStaticModels = 0;
    cm.staticModelList = 0;
    numStaticModels = 0;
    while (1)
    {
        token = (const char *)Com_Parse(&ptr);
        if (!ptr || *token != '{')
            break;
        while (1)
        {
            token = (const char *)Com_Parse(&ptr);
            if (!ptr)
                break;
            if (*token == 125)
                break;
            I_strncpyz(key, (char *)token, 64);
            token = (const char *)Com_Parse(&ptr);
            if (!ptr)
                break;
            I_strncpyz(value, (char *)token, 64);
            if (!I_stricmp(key, "classname") && !I_stricmp(value, "misc_model"))
                ++numStaticModels;
        }
    }
    if (numStaticModels)
    {
        cm.staticModelList = (cStaticModel_s *)CM_Hunk_Alloc(80 * numStaticModels, "CM_CreateStaticModel", 27);
        ptr = Com_EntityString(0);
        if (!ptr)
            MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 146, 0, "%s", "ptr");
        ProfLoad_Begin("Create static model collision");
        while (1)
        {
            token = (const char *)Com_Parse(&ptr);
            if (!ptr || *token != 123)
                break;
            modelName[0] = 0;
            origin[2] = 0.0;
            origin[1] = 0.0;
            origin[0] = 0.0;
            angles[2] = 0.0;
            angles[1] = 0.0;
            angles[0] = 0.0;
            scale = 1.0;
            bMiscModel = 0;
            while (1)
            {
                token = (const char *)Com_Parse(&ptr);
                if (!ptr)
                    break;
                if (*token == 125)
                    break;
                I_strncpyz(key, (char *)token, 64);
                token = (const char *)Com_Parse(&ptr);
                if (!ptr)
                    break;
                I_strncpyz(value, (char *)token, 64);
                if (I_stricmp(key, "classname"))
                {
                    if (I_stricmp(key, "model"))
                    {
                        if (I_stricmp(key, "origin"))
                        {
                            if (I_stricmp(key, "angles"))
                            {
                                if (!I_stricmp(key, "modelscale"))
                                    scale = atof(value);
                            }
                            else
                            {
                                sscanf(value, "%f %f %f", angles, &angles[1], &angles[2]);
                            }
                        }
                        else
                        {
                            sscanf(value, "%f %f %f", origin, &origin[1], &origin[2]);
                        }
                    }
                    else if (Com_IsLegacyXModelName(value))
                    {
                        v5 = &value[7];
                        v4 = modelName;
                        do
                        {
                            v3 = *v5;
                            *v4++ = *v5++;
                        } while (v3);
                    }
                    else
                    {
                        v2 = value;
                        v1 = modelName;
                        do
                        {
                            v0 = *v2;
                            *v1++ = *v2++;
                        } while (v0);
                    }
                }
                else if (!I_stricmp(value, "misc_model"))
                {
                    bMiscModel = 1;
                }
            }
            if (bMiscModel)
            {
                if (cm.numStaticModels >= numStaticModels)
                    MyAssertHandler(
                        ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                        207,
                        0,
                        "%s",
                        "cm.numStaticModels < numStaticModels");
                if (CM_CreateStaticModel(&cm.staticModelList[cm.numStaticModels], modelName, origin, angles, scale))
                    ++cm.numStaticModels;
            }
        }
        ProfLoad_End();
    }
}

void __cdecl CM_LoadMapFromBsp(const char *name, bool usePvs)
{
    char v2; // [esp+3h] [ebp-21h]
    char *v3; // [esp+8h] [ebp-1Ch]
    const char *v4; // [esp+Ch] [ebp-18h]
    unsigned int version; // [esp+20h] [ebp-4h]

    Com_Memset(&cm, 0, 284);
    cm.name = (const char *)CM_Hunk_Alloc(strlen(name) + 1, "CM_LoadMapFromBsp", 25);
    v4 = name;
    v3 = (char *)cm.name;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    } while (v2);
    version = Com_GetBspVersion();
    CMod_LoadMaterials();
    CMod_LoadPlanes();
    CMod_LoadBrushRelated(version, usePvs);
    CMod_LoadNodes();
    CMod_LoadLeafSurfaces();
    CMod_LoadCollisionVerts();
    CMod_LoadCollisionTriangles();
    CMod_LoadCollisionEdgeWalkable();
    CMod_LoadCollisionBorders();
    CMod_LoadCollisionPartitions();
    if (usePvs)
    {
        CMod_LoadVisibility();
    }
    else if (Com_BspHasLump(LUMP_VISIBILITY))
    {
        Com_Error(ERR_DROP, "In single player, do not compile the bsp with visibility");
    }
    ProfLoad_Begin("Load entity string");
    CMod_LoadEntityString();
    ProfLoad_End();
}

void __cdecl CM_LoadMapData_LoadObj(const char *name)
{
    if (!cm.isInUse || I_stricmp(cm.name, name))
    {
        ProfLoad_Begin("Load bsp collision");
        CM_LoadMapFromBsp(name, 1);
        ProfLoad_End();
        ProfLoad_Begin("Load static model collision");
        CM_LoadStaticModels();
        ProfLoad_End();
        ProfLoad_Begin("Load dynamic entities");
        DynEnt_LoadEntities();
        ProfLoad_End();
    }
}

cplane_s *__cdecl CM_GetPlanes()
{
    if (!cm.planes)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 1412, 0, "%s", "cm.planes");
    return cm.planes;
}

int __cdecl CM_GetPlaneCount()
{
    return cm.planeCount;
}


void __cdecl CMod_LoadPlanes()
{
    char v0; // [esp+4h] [ebp-28h]
    char v1; // [esp+8h] [ebp-24h]
    char v2; // [esp+Ch] [ebp-20h]
    cplane_s *out; // [esp+10h] [ebp-1Ch]
    unsigned int planeIter; // [esp+14h] [ebp-18h]
    unsigned __int8 bits; // [esp+1Fh] [ebp-Dh]
    char *in; // [esp+20h] [ebp-Ch]
    unsigned int axisIter; // [esp+24h] [ebp-8h]
    unsigned int count; // [esp+28h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_PLANES, 0x10u, &count);
    if (!count)
        Com_Error(ERR_DROP, "Map with no planes");
    if (count > 0x10000)
        Com_Error(ERR_DROP, "Number of planes exceeds 65536");
    cm.planes = (cplane_s*)CM_Hunk_Alloc(20 * count, "CMod_LoadPlanes", 25);
    cm.planeCount = count;
    out = cm.planes;
    for (planeIter = 0; planeIter < count; ++planeIter)
    {
        bits = 0;
        for (axisIter = 0; axisIter < 3; ++axisIter)
        {
            out->normal[axisIter] = *(float *)&in[4 * axisIter];
            if (out->normal[axisIter] < 0.0)
                bits |= 1 << axisIter;
        }
        out->dist = *((float *)in + 3);
        if (out->normal[0] == 1.0)
        {
            v2 = 0;
        }
        else
        {
            if (out->normal[1] == 1.0)
            {
                v1 = 1;
            }
            else
            {
                if (out->normal[2] == 1.0)
                    v0 = 2;
                else
                    v0 = 3;
                v1 = v0;
            }
            v2 = v1;
        }
        out->type = v2;
        out->signbits = bits;
        in += 16;
        ++out;
    }
}

bool __cdecl CMod_HasSpawnString(const SpawnVar *userData, const char *key)
{
    const char *value; // [esp+4h] [ebp-4h] BYREF

    return G_SpawnString(userData, key, "", &value) != 0;
}

int CMod_LoadMaterials()
{
    int result; // eax
    dmaterial_t *in; // [esp+4h] [ebp-Ch]
    unsigned int matIndex; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = (dmaterial_t*)Com_GetBspLump(LUMP_MATERIALS, 0x48u, &count);
    if (!count)
        Com_Error(ERR_DROP, "Map with no materials");
    cm.materials = (dmaterial_t*)(CM_Hunk_Alloc(72 * (count + 1), "CMod_LoadMaterials", 25) + 72);
    cm.numMaterials = count;
    Com_Memcpy(cm.materials->material, in->material, 72 * count);
    for (matIndex = 0; matIndex < cm.numMaterials; ++matIndex)
    {
        result = 72 * matIndex;
        cm.materials[matIndex].contentFlags &= 0xDFFFFFFB;
    }
    return result;
}

cNode_t *CMod_LoadNodes()
{
    cNode_t *result; // eax
    int j; // [esp+0h] [ebp-1Ch]
    cNode_t *out; // [esp+4h] [ebp-18h]
    char *in; // [esp+Ch] [ebp-10h]
    unsigned int nodeIter; // [esp+10h] [ebp-Ch]
    int child; // [esp+14h] [ebp-8h]
    unsigned int count; // [esp+18h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_NODES, 0x24u, &count);
    if (!count)
        Com_Error(ERR_DROP, "Map has no nodes");
    cm.nodes = (cNode_t*)CM_Hunk_Alloc(8 * count, "CMod_LoadNodes", 25);
    result = (cNode_t*)count;
    cm.numNodes = count;
    out = cm.nodes;
    for (nodeIter = 0; nodeIter < count; ++nodeIter)
    {
        out->plane = &cm.planes[*(_DWORD *)in];
        for (j = 0; j < 2; ++j)
        {
            child = *(_DWORD *)&in[4 * j + 4];
            out->children[j] = child;
            if (out->children[j] != child)
                Com_Error(ERR_DROP, "CMod_LoadNodes: children exceeded");
        }
        result = ++out;
        in += 36;
    }
    return result;
}

void CMod_LoadLeafSurfaces()
{
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_LEAFSURFACES, 4u, &count);
    cm.leafsurfaces = (unsigned int*)CM_Hunk_Alloc(4 * count, "CMod_LoadLeafSurfaces", 28);
    cm.numLeafSurfaces = count;
    Com_Memcpy(cm.leafsurfaces, in, 4 * count);
}

void CMod_LoadCollisionVerts()
{
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONVERTS, 0xCu, &count);
    cm.verts = (float(*)[3])CM_Hunk_Alloc(12 * count, "CMod_LoadCollisionVerts", 28);
    cm.vertCount = count;
    Com_Memcpy(cm.verts, in, 12 * count);
}

void CMod_LoadCollisionTriangles()
{
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONTRIS, 2u, &count);
    cm.triIndices = (unsigned short*)CM_Hunk_Alloc(2 * count, "CMod_LoadCollisionTriangles", 28);
    cm.triCount = count / 3;
    Com_Memcpy(cm.triIndices, in, 2 * count);
}

void CMod_LoadCollisionEdgeWalkable()
{
    char *in; // [esp+0h] [ebp-8h]
    unsigned int count; // [esp+4h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONEDGEWALKABLE, 1u, &count);
    cm.triEdgeIsWalkable = CM_Hunk_Alloc(count, "CMod_LoadCollisionEdgeWalkable", 28);
    Com_Memcpy(cm.triEdgeIsWalkable, in, count);
}

const DiskCollBorder *CMod_LoadCollisionBorders()
{
    const DiskCollBorder *result; // eax
    CollisionBorder *out; // [esp+0h] [ebp-14h]
    const DiskCollBorder *in; // [esp+8h] [ebp-Ch]
    unsigned int index; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = (const DiskCollBorder*)Com_GetBspLump(LUMP_COLLISIONBORDERS, 0x1Cu, &count);
    cm.borders = (CollisionBorder*)CM_Hunk_Alloc(28 * count, "CMod_LoadCollisionBorders", 28);
    result = (const DiskCollBorder*)count;
    cm.borderCount = count;
    out = cm.borders;
    for (index = 0; index < count; ++index)
    {
        out->distEq[0] = in->distEq[0];
        out->distEq[1] = in->distEq[1];
        out->distEq[2] = in->distEq[2];
        out->zBase = in->zBase;
        out->zSlope = in->zSlope;
        out->start = in->start;
        out->length = in->length;
        result = ++in;
        ++out;
    }
    return result;
}

char *CMod_LoadCollisionPartitions()
{
    char *result; // eax
    CollisionPartition *out; // [esp+0h] [ebp-14h]
    char *in; // [esp+8h] [ebp-Ch]
    unsigned int index; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONPARTITIONS, 0xCu, &count);
    cm.partitions = (CollisionPartition*)CM_Hunk_Alloc(12 * count, "CMod_LoadCollisionPartitions", 28);
    result = (char*)count;
    cm.partitionCount = count;
    out = cm.partitions;
    index = 0;
    while (index < count)
    {
        out->triCount = in[2];
        out->borderCount = in[3];
        out->firstTri = *(in + 1);
        out->borders = &cm.borders[*(in + 2)];
        if (out->firstTri + out->triCount > cm.triCount)
            MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 1069, 0, "%s", "out->firstTri + out->triCount <= cm.triCount");
        if (out->firstTri < 0)
            MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 1070, 0, "%s", "out->firstTri >= 0");
        ++index;
        result = in + 12;
        in += 12;
        ++out;
    }
    return result;
}

const char *g_purgeableEnts[7] =
{
  "misc_model",
  "misc_prefab",
  "dyn_brushmodel",
  "dyn_model",
  "reflection_probe",
  "info_null",
  "func_group"
};

bool __cdecl MapEnts_CanPurgeEntity(
    const char *classname,
    void *userData,
    bool(__cdecl *HasKeyCallback)(void *, const char *))
{
    unsigned int classnameIndex; // [esp+0h] [ebp-4h]

    for (classnameIndex = 0; classnameIndex < 7; ++classnameIndex)
    {
        if (!I_stricmp(classname, g_purgeableEnts[classnameIndex]))
            return 1;
    }
    return !I_stricmp(classname, "light") && !HasKeyCallback(userData, "pl#");
}

MapEnts *__cdecl MapEnts_GetFromString(char *name, const char *entityString, int numEntityChars)
{
    MapEnts *mapEnts; // [esp+10h] [ebp-A2Ch]
    SpawnVar spawnVar; // [esp+14h] [ebp-A28h] BYREF
    const char *end; // [esp+A28h] [ebp-14h]
    int size; // [esp+A2Ch] [ebp-10h]
    int nameLen; // [esp+A30h] [ebp-Ch]
    const char *classname; // [esp+A34h] [ebp-8h] BYREF
    const char *begin; // [esp+A38h] [ebp-4h]
    char *entityStringa; // [esp+A48h] [ebp+Ch]

    mapEnts = (MapEnts*)CM_Hunk_Alloc(0xCu, "CMod_LoadEntityString", 30);
    nameLen = strlen(name);
    mapEnts->name = (const char*)CM_Hunk_Alloc(nameLen + 1, "CMod_LoadEntityString", 30);
    memcpy((void*)mapEnts->name, name, nameLen + 1);
    entityStringa = Com_EntityString(&numEntityChars);
    mapEnts->entityString = (char*)CM_Hunk_Alloc(numEntityChars, "CMod_LoadEntityString", 30);
    mapEnts->numEntityChars = 0;
    G_SetEntityParsePoint(entityStringa);
    while (1)
    {
        begin = G_GetEntityParsePoint();
        if (!G_ParseSpawnVars(&spawnVar))
            break;
        G_SpawnString(&spawnVar, "classname", "", &classname);
        if (!MapEnts_CanPurgeEntity(classname, &spawnVar, (bool(*)(void*, const char*))CMod_HasSpawnString))
        {
            end = G_GetEntityParsePoint();
            size = end - begin;
            if (end - begin + mapEnts->numEntityChars > numEntityChars)
                MyAssertHandler(
                    ".\\qcommon\\cm_load_obj.cpp",
                    1206,
                    0,
                    "%s",
                    "mapEnts->numEntityChars + size <= numEntityChars");
            memcpy(&mapEnts->entityString[mapEnts->numEntityChars], begin, size);
            mapEnts->numEntityChars += size;
        }
    }
    if (mapEnts->numEntityChars >= numEntityChars)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 1211, 0, "%s", "mapEnts->numEntityChars < numEntityChars");
    mapEnts->entityString[mapEnts->numEntityChars++] = 0;
    return mapEnts;
}

MapEnts *__cdecl MapEnts_RealLoad(const char *name)
{
    int numEntityChars; // [esp+0h] [ebp-8h] BYREF
    const char *entityString; // [esp+4h] [ebp-4h]

    entityString = Com_EntityString(&numEntityChars);
    return MapEnts_GetFromString((char*)name, entityString, numEntityChars);
}

MapEnts *__cdecl MapEnts_VirtualLoad(const char *name)
{
    return MapEnts_RealLoad(name);
}

MapEnts *CMod_LoadEntityString()
{
    MapEnts *result; // eax

    result = MapEnts_VirtualLoad(cm.name);
    cm.mapEnts = result;
    return result;
}

void CMod_LoadVisibility()
{
    const char *v0; // eax
    char *buf; // [esp+0h] [ebp-8h]
    unsigned int len; // [esp+4h] [ebp-4h] BYREF

    buf = Com_GetBspLump(LUMP_VISIBILITY, 1u, &len);
    if (len)
    {
        cm.vised = 1;
        cm.numClusters = *buf;
        cm.clusterBytes = *(buf + 1);
        if (len != cm.clusterBytes * cm.numClusters + 8)
        {
            v0 = va("%i != %i == %i * %i + %i", len, cm.clusterBytes * cm.numClusters + 8, cm.numClusters, cm.clusterBytes, 8);
            MyAssertHandler(
                ".\\qcommon\\cm_load_obj.cpp",
                1254,
                0,
                "%s\n\t%s",
                "static_cast< int >( len ) == cm.numClusters * cm.clusterBytes + VIS_HEADER",
                v0);
        }
        cm.visibility = CM_Hunk_Alloc(len - 8, "CMod_LoadVisibility", 9);
        Com_Memcpy(cm.visibility, buf + 8, len - 8);
    }
    else
    {
        cm.clusterBytes = ((cm.numClusters + 63) & 0xFFFFFFC0) >> 3;
        cm.numClusters = 1;
        cm.visibility = CM_Hunk_Alloc(cm.clusterBytes, "CMod_LoadVisibility", 9);
        Com_Memset(cm.visibility, 255, cm.clusterBytes);
    }
}

unsigned __int16 *CM_InitBoxHull()
{
    cLeafBrushNode_s *v0; // eax
    unsigned __int16 *result; // eax

    cm.box_brush = &cm.brushes[cm.numBrushes];
    cm.brushes[cm.numBrushes].numsides = 0;
    cm.box_brush->sides = 0;
    cm.box_brush->contents = -1;
    cm.box_model.leaf.brushContents = -1;
    cm.box_model.leaf.terrainContents = 0;
    cm.box_model.leaf.mins[0] = FLT_MAX;
    cm.box_model.leaf.mins[1] = FLT_MAX;
    cm.box_model.leaf.mins[2] = FLT_MAX;
    cm.box_model.leaf.maxs[0] = -FLT_MAX;
    cm.box_model.leaf.maxs[1] = -FLT_MAX;
    cm.box_model.leaf.maxs[2] = -FLT_MAX;
    cm.box_brush->axialMaterialNum[0][0] = -1;
    cm.box_brush->axialMaterialNum[0][1] = -1;
    cm.box_brush->axialMaterialNum[0][2] = -1;
    cm.box_brush->axialMaterialNum[1][0] = -1;
    cm.box_brush->axialMaterialNum[1][1] = -1;
    cm.box_brush->axialMaterialNum[1][2] = -1;
    v0 = CMod_AllocLeafBrushNode();
    cm.box_model.leaf.leafBrushNode = v0 - cm.leafbrushNodes;
    v0->leafBrushCount = 1;
    v0->data.leaf.brushes = &cm.leafbrushes[cm.numLeafBrushes];
    result = cm.leafbrushes;
    cm.leafbrushes[cm.numLeafBrushes] = cm.numBrushes;
    return result;
}

void __cdecl CMod_LoadBrushRelated(unsigned int version, bool usePvs)
{
    int leafbrushNodesCount; // [esp+0h] [ebp-10h]
    HunkUser *user; // [esp+8h] [ebp-8h]
    cLeafBrushNode_s *leafbrushNodes; // [esp+Ch] [ebp-4h]

    CMod_LoadBrushes();
    CMod_LoadLeafBrushes();
    CMod_LoadCollisionAabbTrees();
    if (version > 0xE)
        CMod_LoadLeafs(usePvs);
    else
        CMod_LoadLeafs_Version14(usePvs);
    CMod_LoadSubmodels();
    user = Hunk_UserCreate(0x400000, "CMod_LoadBrushRelated", 1, 0, 26);
    TempMemoryReset(user);
    cm.leafbrushNodes = (cLeafBrushNode_s*)(TempMalloc(0) - 20);
    if (version > 0xE)
        CMod_LoadLeafBrushNodes();
    else
        CMod_LoadLeafBrushNodes_Version14();
    CMod_LoadSubmodelBrushNodes();
    CM_InitBoxHull();
    ++cm.leafbrushNodes;
    leafbrushNodesCount = (TempMalloc(0) - (char*)cm.leafbrushNodes) / 20;
    cm.leafbrushNodesCount = leafbrushNodesCount + 1;
    leafbrushNodes = (cLeafBrushNode_s*)CM_Hunk_Alloc(20 * (leafbrushNodesCount + 1), "CMod_LoadBrushRelated", 26);
    memcpy(&leafbrushNodes[1].axis, &cm.leafbrushNodes->axis, 20 * leafbrushNodesCount);
    cm.leafbrushNodes = leafbrushNodes;
    Hunk_UserDestroy(user);
}

unsigned int CMod_LoadSubmodels()
{
    unsigned int result; // eax
    float v1; // [esp+0h] [ebp-50h]
    float v2; // [esp+4h] [ebp-4Ch]
    float v3; // [esp+8h] [ebp-48h]
    float v4; // [esp+Ch] [ebp-44h]
    int j; // [esp+24h] [ebp-2Ch]
    cmodel_t *out; // [esp+28h] [ebp-28h]
    char *in; // [esp+30h] [ebp-20h]
    unsigned int bmodelIndex; // [esp+34h] [ebp-1Ch]
    int firstCollAabbIndex; // [esp+38h] [ebp-18h]
    float extent[3]; // [esp+3Ch] [ebp-14h] BYREF
    unsigned int count; // [esp+48h] [ebp-8h] BYREF
    int collAabbCount; // [esp+4Ch] [ebp-4h]

    in = Com_GetBspLump(LUMP_MODELS, 0x30u, &count);
    if (!count)
        Com_Error(ERR_DROP, "Map with no brush models (should at least have the world model)");
    cm.cmodels = (cmodel_t*)CM_Hunk_Alloc(72 * count, "CMod_LoadSubmodels", 26);
    cm.numSubModels = count;
    if (count > 0xFFF)
        Com_Error(ERR_DROP, "MAX_SUBMODELS exceeded");
    for (bmodelIndex = 0; ; ++bmodelIndex)
    {
        result = bmodelIndex;
        if (bmodelIndex >= count)
            break;
        out = &cm.cmodels[bmodelIndex];
        for (j = 0; j < 3; ++j)
        {
            out->mins[j] = *(float *)&in[4 * j] - 1.0;
            out->maxs[j] = *(float *)&in[4 * j + 12] + 1.0;
            v4 = fabs(out->maxs[j]);
            v3 = fabs(out->mins[j]);
            v2 = v3 - v4;
            if (v2 < 0.0)
                v1 = v4;
            else
                v1 = v3;
            extent[j] = v1;
        }
        out->radius = Vec3Length(extent);
        if (bmodelIndex)
        {
            collAabbCount = *((_DWORD *)in + 9);
            out->leaf.collAabbCount = collAabbCount;
            if (out->leaf.collAabbCount != collAabbCount)
                Com_Error(ERR_DROP, "CMod_LoadSubmodels: collAabbCount exceeded");
            firstCollAabbIndex = *((_DWORD *)in + 8);
            out->leaf.firstCollAabbIndex = firstCollAabbIndex;
            if (out->leaf.firstCollAabbIndex != firstCollAabbIndex)
                Com_Error(ERR_DROP, "CMod_LoadSubmodels: firstCollAabbIndex exceeded");
        }
        in += 48;
    }
    return result;
}

void CMod_LoadSubmodelBrushNodes()
{
    int contents; // [esp+0h] [ebp-24h]
    cmodel_t *out; // [esp+4h] [ebp-20h]
    int numLeafBrushes; // [esp+8h] [ebp-1Ch]
    int leafBrushIndex; // [esp+Ch] [ebp-18h]
    char *in; // [esp+10h] [ebp-14h]
    const DiskBrushModel *ina; // [esp+10h] [ebp-14h]
    unsigned int bmodelIndex; // [esp+14h] [ebp-10h]
    unsigned __int16 *indexes; // [esp+18h] [ebp-Ch]
    unsigned int count; // [esp+1Ch] [ebp-8h] BYREF
    int firstBrush; // [esp+20h] [ebp-4h]

    in = Com_GetBspLump(LUMP_MODELS, 0x30u, &count);
    if (count != cm.numSubModels)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 451, 0, "count == cm.numSubModels\n\t%i, %i", count, cm.numSubModels);
    ina = (const DiskBrushModel*)(in + 48);
    for (bmodelIndex = 1; bmodelIndex < cm.numSubModels; ++bmodelIndex)
    {
        out = &cm.cmodels[bmodelIndex];
        numLeafBrushes = ina->numBrushes;
        indexes = (unsigned short*)CM_Hunk_Alloc(2 * numLeafBrushes, "CMod_LoadSubmodelBrushNodes", 26);
        contents = 0;
        for (leafBrushIndex = 0; leafBrushIndex < numLeafBrushes; ++leafBrushIndex)
        {
            firstBrush = leafBrushIndex + ina->firstBrush;
            indexes[leafBrushIndex] = firstBrush;
            if (indexes[leafBrushIndex] != firstBrush)
                Com_Error(ERR_DROP, "CMod_LoadSubmodelBrushNodes: leafBrushes exceeded");
            contents |= cm.brushes[firstBrush].contents;
        }
        out->leaf.brushContents = contents;
        out->leaf.terrainContents = CMod_GetLeafTerrainContents(&out->leaf);
        CMod_PartionLeafBrushes(indexes, numLeafBrushes, &out->leaf);
        ++ina;
    }
}

void __cdecl CM_Hunk_CheckTempMemoryHighClear()
{
    Hunk_CheckTempMemoryHighClear();
}

void __cdecl CM_Hunk_ClearTempMemoryHigh()
{
    Hunk_ClearTempMemoryHigh();
}

void __cdecl CMod_PartionLeafBrushes(unsigned __int16 *leafBrushes, int numLeafBrushes, cLeaf_t *leaf)
{
    int j; // [esp+8h] [ebp-28h]
    int ja; // [esp+8h] [ebp-28h]
    float mins[3]; // [esp+Ch] [ebp-24h] BYREF
    int k; // [esp+18h] [ebp-18h]
    cbrush_t *b; // [esp+1Ch] [ebp-14h]
    float maxs[3]; // [esp+20h] [ebp-10h] BYREF
    int brushnum; // [esp+2Ch] [ebp-4h]

    if (numLeafBrushes)
    {
        mins[0] = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;
        maxs[0] = -FLT_MAX;
        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        for (k = 0; k < numLeafBrushes; ++k)
        {
            brushnum = leafBrushes[k];
            b = &cm.brushes[brushnum];
            for (j = 0; j < 3; ++j)
            {
                if (b->mins[j] < mins[j])
                    mins[j] = b->mins[j];
                if (b->maxs[j] > maxs[j])
                    maxs[j] = b->maxs[j];
            }
        }
        leaf->mins[0] = mins[0];
        leaf->mins[1] = mins[1];
        leaf->mins[2] = mins[2];
        leaf->maxs[0] = maxs[0];
        leaf->maxs[1] = maxs[1];
        leaf->maxs[2] = maxs[2];
        for (ja = 0; ja < 3; ++ja)
        {
            leaf->mins[ja] = leaf->mins[ja] - 0.125;
            leaf->maxs[ja] = leaf->maxs[ja] + 0.125;
        }
        CM_Hunk_CheckTempMemoryHighClear();
        leaf->leafBrushNode = CMod_PartionLeafBrushes_r(leafBrushes, numLeafBrushes, mins, maxs) - cm.leafbrushNodes;
        CM_Hunk_ClearTempMemoryHigh();
    }
    else if (leaf->leafBrushNode)
    {
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 379, 0, "%s", "!leaf->leafBrushNode");
    }
}

unsigned int __cdecl CM_Hunk_AllocateTempMemoryHigh(int size, const char *name)
{
    return Hunk_AllocateTempMemoryHigh(size, name);
}

cLeafBrushNode_s *__cdecl CMod_PartionLeafBrushes_r(
    unsigned __int16 *leafBrushes,
    int numLeafBrushes,
    const float *mins,
    const float *maxs)
{
    float v5; // [esp+0h] [ebp-78h]
    float v6; // [esp+4h] [ebp-74h]
    float v7; // [esp+8h] [ebp-70h]
    float v8; // [esp+Ch] [ebp-6Ch]
    float v9; // [esp+10h] [ebp-68h]
    float v10; // [esp+14h] [ebp-64h]
    cLeafBrushNode_s *node; // [esp+18h] [ebp-60h]
    int side; // [esp+1Ch] [ebp-5Ch]
    int nodeOffset; // [esp+20h] [ebp-58h]
    float testDist; // [esp+24h] [ebp-54h] BYREF
    int numLeafBrushesChild; // [esp+28h] [ebp-50h]
    unsigned __int8 testAxis; // [esp+2Fh] [ebp-49h]
    int k; // [esp+30h] [ebp-48h]
    float dist; // [esp+34h] [ebp-44h]
    float range; // [esp+38h] [ebp-40h]
    float childMaxs[3]; // [esp+3Ch] [ebp-3Ch] BYREF
    cbrush_t *b; // [esp+48h] [ebp-30h]
    float bestScore; // [esp+4Ch] [ebp-2Ch]
    int len; // [esp+50h] [ebp-28h]
    float childMins[3]; // [esp+54h] [ebp-24h] BYREF
    cLeafBrushNode_s *childNode; // [esp+60h] [ebp-18h]
    unsigned __int16 *leafBrushesCopy; // [esp+64h] [ebp-14h]
    cLeafBrushNode_s *returnNode; // [esp+68h] [ebp-10h]
    int axis; // [esp+6Ch] [ebp-Ch]
    int brushnum; // [esp+70h] [ebp-8h]
    float score; // [esp+74h] [ebp-4h]

    if (!numLeafBrushes)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 222, 0, "%s", "numLeafBrushes");
    node = CMod_AllocLeafBrushNode();
    bestScore = 0.0;
    axis = -1;
    dist = 0.0;
    for (testAxis = 0; testAxis < 3u; ++testAxis)
    {
        for (k = 0; k < numLeafBrushes; ++k)
        {
            brushnum = leafBrushes[k];
            b = &cm.brushes[brushnum];
            testDist = b->mins[testAxis];
            score = CMod_GetPartitionScore(leafBrushes, numLeafBrushes, testAxis, mins, maxs, &testDist);
            if (bestScore < score)
            {
                bestScore = score;
                axis = testAxis;
                dist = testDist;
            }
            testDist = b->maxs[testAxis];
            score = CMod_GetPartitionScore(leafBrushes, numLeafBrushes, testAxis, mins, maxs, &testDist);
            if (bestScore < score)
            {
                bestScore = score;
                axis = testAxis;
                dist = testDist;
            }
        }
    }
    if (axis >= 0)
    {
        len = 2 * numLeafBrushes;
        leafBrushesCopy = (unsigned short*)CM_Hunk_AllocateTempMemoryHigh(2 * numLeafBrushes, "CMod_PartionLeafBrushes_r");
        memcpy(leafBrushesCopy, leafBrushes, len);
        numLeafBrushesChild = 0;
        for (k = 0; k < numLeafBrushes; ++k)
        {
            brushnum = leafBrushesCopy[k];
            b = &cm.brushes[brushnum];
            if (dist > b->mins[axis] && dist < b->maxs[axis])
                leafBrushes[numLeafBrushesChild++] = brushnum;
        }
        if (numLeafBrushesChild)
        {
            returnNode = CMod_PartionLeafBrushes_r(leafBrushes, numLeafBrushesChild, mins, maxs);
            if (returnNode != &node[1])
                MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 297, 0, "%s", "returnNode == node + 1");
            node->leafBrushCount = -1;
            node->contents = returnNode->contents;
            leafBrushes += numLeafBrushesChild;
        }
        range = FLT_MAX;
        node->axis = axis;
        node->data.children.dist = dist;
        side = 0;
    LABEL_33:
        if (side > 1)
        {
            node->data.children.range = range;
            return node;
        }
        numLeafBrushesChild = 0;
        for (k = 0; ; ++k)
        {
            if (k >= numLeafBrushes)
            {
                if (!numLeafBrushesChild)
                    MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 336, 0, "%s", "numLeafBrushesChild");
                childMins[0] = *mins;
                childMins[1] = mins[1];
                childMins[2] = mins[2];
                childMaxs[0] = *maxs;
                childMaxs[1] = maxs[1];
                childMaxs[2] = maxs[2];
                if (side)
                    childMaxs[axis] = dist - range;
                else
                    childMins[axis] = dist + range;
                childNode = CMod_PartionLeafBrushes_r(leafBrushes, numLeafBrushesChild, childMins, childMaxs);
                nodeOffset = childNode - node;
                node->data.children.childOffset[side] = nodeOffset;
                if (node->data.children.childOffset[side] != nodeOffset)
                    Com_Error(ERR_DROP, "CMod_PartionLeafBrushes_r: child exceeded");
                node->contents |= childNode->contents;
                leafBrushes += numLeafBrushesChild;
                ++side;
                goto LABEL_33;
            }
            brushnum = leafBrushesCopy[k];
            b = &cm.brushes[brushnum];
            if (side)
            {
                if (dist < b->maxs[axis])
                    continue;
                v9 = dist - b->maxs[axis];
                v6 = v9 - range;
                if (v6 < 0.0)
                    v5 = dist - b->maxs[axis];
                else
                    v5 = range;
                range = v5;
            }
            else
            {
                if (dist > b->mins[axis])
                    continue;
                v10 = b->mins[axis] - dist;
                v8 = v10 - range;
                if (v8 < 0.0)
                    v7 = b->mins[axis] - dist;
                else
                    v7 = range;
                range = v7;
            }
            leafBrushes[numLeafBrushesChild++] = brushnum;
        }
    }
    node->leafBrushCount = numLeafBrushes;
    if (node->leafBrushCount != numLeafBrushes)
        Com_Error(ERR_DROP, "CMod_PartionLeafBrushes_r: leafBrushCount exceeded");
    for (k = 0; k < numLeafBrushes; ++k)
    {
        brushnum = leafBrushes[k];
        b = &cm.brushes[brushnum];
        node->contents |= b->contents;
    }
    if (!node->contents)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 270, 0, "%s", "node->contents");
    node->data.leaf.brushes = leafBrushes;
    return node;
}

cLeafBrushNode_s *__cdecl CMod_AllocLeafBrushNode()
{
    cLeafBrushNode_s *result; // eax

    result = (cLeafBrushNode_s*)TempMalloc(0x14u);
    result->axis = 0;
    result->leafBrushCount = 0;
    result->contents = 0;
    result->data.leaf.brushes = 0;
    result->data.children.range = 0.0;
    result->data.children.childOffset[0] = 0;
    result->data.children.childOffset[1] = 0;
    result->data.children.dist = -FLT_MAX;
    return result;
}

double __cdecl CMod_GetPartitionScore(
    unsigned __int16 *leafBrushes,
    int numLeafBrushes,
    int axis,
    const float *mins,
    const float *maxs,
    float *dist)
{
    float v9; // [esp+Ch] [ebp-34h]
    float v10; // [esp+10h] [ebp-30h]
    int v11; // [esp+14h] [ebp-2Ch]
    float v12; // [esp+18h] [ebp-28h]
    float v13; // [esp+1Ch] [ebp-24h]
    float max; // [esp+20h] [ebp-20h]
    int rightBrushCount; // [esp+24h] [ebp-1Ch]
    int k; // [esp+28h] [ebp-18h]
    cbrush_t *b; // [esp+2Ch] [ebp-14h]
    float min; // [esp+30h] [ebp-10h]
    int leftBrushCount; // [esp+34h] [ebp-Ch]

    rightBrushCount = -1;
    leftBrushCount = -1;
    min = -FLT_MAX;
    max = FLT_MAX;
    for (k = 0; k < numLeafBrushes; ++k)
    {
        b = &cm.brushes[leafBrushes[k]];
        if (*dist > b->mins[axis])
        {
            if (*dist >= b->maxs[axis])
            {
                ++leftBrushCount;
                if (min < b->maxs[axis])
                    min = b->maxs[axis];
            }
        }
        else
        {
            ++rightBrushCount;
            if (max > b->mins[axis])
                max = b->mins[axis];
        }
    }
    if (*dist < min)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 183, 0, "%s", "min <= *dist");
    if (max < *dist)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 184, 0, "%s", "*dist <= max");
    if (rightBrushCount < leftBrushCount)
        v11 = rightBrushCount;
    else
        v11 = leftBrushCount;
    *dist = (min + max) * 0.5;
    if (v11 <= 0)
    {
        return 0.0;
    }
    else
    {
        v12 = max - mins[axis];
        v13 = maxs[axis] - min;
        v10 = v13 - v12;
        if (v10 < 0.0)
            v9 = maxs[axis] - min;
        else
            v9 = max - mins[axis];
        return (v11 * v9);
    }
}

int __cdecl CMod_GetLeafTerrainContents(cLeaf_t *leaf)
{
    int contents; // [esp+0h] [ebp-8h]
    int k; // [esp+4h] [ebp-4h]

    contents = 0;
    for (k = 0; k < leaf->collAabbCount; ++k)
        contents |= cm.materials[cm.aabbTrees[k + leaf->firstCollAabbIndex].materialIndex].contentFlags;
    return contents;
}

void CMod_LoadBrushes()
{
    unsigned __int8 *v0; // [esp+0h] [ebp-68h]
    unsigned __int8 *v1; // [esp+4h] [ebp-64h]
    unsigned int edgesCount; // [esp+8h] [ebp-60h] BYREF
    cbrushside_t *outSides; // [esp+Ch] [ebp-5Ch]
    cbrush_t *outBrush; // [esp+10h] [ebp-58h]
    const dbrush_t *inBrush; // [esp+14h] [ebp-54h]
    const unsigned __int8 *inEdgeCountsBase; // [esp+18h] [ebp-50h]
    unsigned int allocSizeSides; // [esp+1Ch] [ebp-4Ch]
    unsigned int brushCount; // [esp+20h] [ebp-48h] BYREF
    float sign; // [esp+24h] [ebp-44h]
    const dbrushside_t *inSides; // [esp+28h] [ebp-40h]
    int countAllocatedBrushes; // [esp+2Ch] [ebp-3Ch]
    const unsigned __int8 *inEdgeCounts; // [esp+30h] [ebp-38h]
    unsigned int allocSizeEdges; // [esp+34h] [ebp-34h]
    unsigned __int8 *outEdges; // [esp+38h] [ebp-30h]
    unsigned int materialNum; // [esp+3Ch] [ebp-2Ch]
    unsigned int allocSizeBrushes; // [esp+40h] [ebp-28h]
    int edgeOffset; // [esp+44h] [ebp-24h]
    unsigned int axisIter; // [esp+48h] [ebp-20h]
    int index; // [esp+4Ch] [ebp-1Ch]
    int num; // [esp+50h] [ebp-18h]
    const unsigned __int8 *inEdges; // [esp+54h] [ebp-14h]
    unsigned int sideEdgeCountsCount; // [esp+58h] [ebp-10h] BYREF
    unsigned int sideIter; // [esp+5Ch] [ebp-Ch]
    unsigned int sidesCount; // [esp+60h] [ebp-8h] BYREF
    unsigned int brushIter; // [esp+64h] [ebp-4h]

    inBrush = (const dbrush_t*)Com_GetBspLump(LUMP_BRUSHES, 4u, &brushCount);
    inSides = (const dbrushside_t*)Com_GetBspLump(LUMP_BRUSHSIDES, 8u, &sidesCount);
    inEdgeCounts = (const unsigned char*)Com_GetBspLump(LUMP_BRUSHSIDEEDGECOUNTS, 1u, &sideEdgeCountsCount);
    if (sideEdgeCountsCount != sidesCount)
        Com_Error(ERR_DROP, "CMod_LoadBrushes: bad number of side edge counts: %i != %i", sideEdgeCountsCount, sidesCount);
    inEdgeCountsBase = inEdgeCounts;
    inEdges = (const unsigned char*)Com_GetBspLump(LUMP_BRUSHEDGES, 1u, &edgesCount);
    allocSizeEdges = edgesCount;
    if (edgesCount)
        v1 = CM_Hunk_Alloc(allocSizeEdges, "CMod_LoadBrushSides", 26);
    else
        v1 = 0;
    cm.brushEdges = v1;
    cm.numBrushEdges = edgesCount;
    memcpy(v1, inEdges, allocSizeEdges);
    outEdges = cm.brushEdges;
    sidesCount -= 6 * brushCount;
    allocSizeSides = 12 * sidesCount;
    if (sidesCount)
        v0 = CM_Hunk_Alloc(allocSizeSides, "CMod_LoadBrushSides", 26);
    else
        v0 = 0;
    cm.brushsides = (cbrushside_t*)v0;
    cm.numBrushSides = sidesCount;
    outSides = (cbrushside_t*)v0;
    countAllocatedBrushes = brushCount + 1;
    allocSizeBrushes = 80 * (brushCount + 1);
    cm.brushes = (cbrush_t*)CM_Hunk_Alloc(allocSizeBrushes, "CMod_LoadBrushes", 26);
    cm.numBrushes = brushCount;
    if (brushCount != brushCount)
        Com_Error(ERR_DROP, "CMod_LoadBrushes: cm.numBrushes exceeded");
    outBrush = cm.brushes;
    brushIter = 0;
    while (brushIter < brushCount)
    {
        outBrush->numsides = inBrush->numSides - 6;
        outBrush->baseAdjacentSide = outEdges;
        edgeOffset = 0;
        outBrush->sides = outBrush->numsides != 0 ? outSides : 0;
        for (axisIter = 0; axisIter < 3; ++axisIter)
        {
            sign = -1.0;
            index = 0;
            while (index < 2)
            {
                if (index)
                    outBrush->maxs[axisIter] = inSides->u.bound;
                else
                    outBrush->mins[axisIter] = inSides->u.bound;
                materialNum = inSides->materialNum;
                if (materialNum >= cm.numMaterials)
                    Com_Error(ERR_DROP, "CMod_LoadBrushes: bad materialNum: %brushIter", materialNum);
                outBrush->axialMaterialNum[index][axisIter] = materialNum;
                if (outBrush->axialMaterialNum[index][axisIter] != materialNum)
                    Com_Error(ERR_DROP, "CMod_LoadBrushes: axialMaterialNum exceeded");
                outBrush->edgeCount[index][axisIter] = *inEdgeCounts;
                outBrush->firstAdjacentSideOffsets[index][axisIter] = edgeOffset;
                if (outBrush->firstAdjacentSideOffsets[index][axisIter] != edgeOffset)
                    Com_Error(ERR_DROP, "CMod_LoadBrushes: maximum edge offset exceeded");
                edgeOffset += *inEdgeCounts;
                outEdges += *inEdgeCounts;
                ++index;
                ++inSides;
                ++inEdgeCounts;
                sign = 1.0;
            }
        }
        sideIter = 0;
        while (sideIter < outBrush->numsides)
        {
            num = inSides->u.planeNum;
            outSides->plane = &cm.planes[num];
            outSides->materialNum = inSides->materialNum;
            if (outSides->materialNum >= cm.numMaterials)
                Com_Error(ERR_DROP, "CMod_LoadBrushes: bad materialNum: %brushIter", outSides->materialNum);
            outSides->edgeCount = *inEdgeCounts;
            outSides->firstAdjacentSideOffset = edgeOffset;
            if (outSides->firstAdjacentSideOffset != edgeOffset)
                Com_Error(ERR_DROP, "CMod_LoadBrushes: maximum edge offset exceeded");
            edgeOffset += *inEdgeCounts;
            outEdges += *inEdgeCounts;
            ++sideIter;
            ++inSides;
            ++outSides;
            ++inEdgeCounts;
        }
        materialNum = inBrush->materialNum;
        if (materialNum >= cm.numMaterials)
            Com_Error(ERR_DROP, "CMod_LoadBrushes: bad materialNum: %brushIter", materialNum);
        outBrush->contents = cm.materials[materialNum].contentFlags;
        ++brushIter;
        ++outBrush;
        ++inBrush;
    }
    if (sideEdgeCountsCount != inEdgeCounts - inEdgeCountsBase)
        MyAssertHandler(
            ".\\qcommon\\cm_load_obj.cpp",
            660,
            1,
            "sideEdgeCountsCount == static_cast< uint >( inEdgeCounts - inEdgeCountsBase )\n\t%i, %i",
            sideEdgeCountsCount,
            inEdgeCounts - inEdgeCountsBase);
}

void __cdecl CMod_LoadLeafs(bool usePvs)
{
    cLeaf_t *out; // [esp+0h] [ebp-20h]
    int cluster; // [esp+4h] [ebp-1Ch]
    unsigned int leafIter; // [esp+8h] [ebp-18h]
    char *in; // [esp+10h] [ebp-10h]
    int firstCollAabbIndex; // [esp+14h] [ebp-Ch]
    unsigned int count; // [esp+18h] [ebp-8h] BYREF
    int collAabbCount; // [esp+1Ch] [ebp-4h]

    in = Com_GetBspLump(LUMP_LEAFS, 24u, &count);
    if (!count)
        Com_Error(ERR_DROP, "Map with no leafs");
    cm.leafs = (cLeaf_t*)CM_Hunk_Alloc(44 * count, "CMod_LoadLeafs", 25);
    cm.numLeafs = count;
    cluster = 0;
    out = cm.leafs;
    for (leafIter = 0; leafIter < count; ++leafIter)
    {
        if (usePvs)
        {
            cluster = *in;
            out->cluster = *in;
            if (out->cluster != cluster)
                Com_Error(ERR_DROP, "CMod_LoadLeafs: cluster exceeded");
        }
        firstCollAabbIndex = *(in + 1);
        out->firstCollAabbIndex = firstCollAabbIndex;
        if (out->firstCollAabbIndex != firstCollAabbIndex)
            Com_Error(ERR_DROP, "CMod_LoadLeafs: firstCollAabbIndex exceeded");
        collAabbCount = *(in + 2);
        out->collAabbCount = collAabbCount;
        if (out->collAabbCount != collAabbCount)
            Com_Error(ERR_DROP, "CMod_LoadLeafs: collAabbCount exceeded");
        if (usePvs && cluster >= cm.numClusters)
            cm.numClusters = cluster + 1;
        in += 24;
        ++out;
    }
}

void __cdecl CMod_LoadLeafs_Version14(bool usePvs)
{
    cLeaf_t *out; // [esp+0h] [ebp-20h]
    int cluster; // [esp+4h] [ebp-1Ch]
    unsigned int leafIter; // [esp+8h] [ebp-18h]
    char *in; // [esp+10h] [ebp-10h]
    int firstCollAabbIndex; // [esp+14h] [ebp-Ch]
    unsigned int count; // [esp+18h] [ebp-8h] BYREF
    int collAabbCount; // [esp+1Ch] [ebp-4h]

    in = Com_GetBspLump(LUMP_LEAFS, 0x24u, &count);
    if (!count)
        Com_Error(ERR_DROP, "Map with no leafs");
    cm.leafs = (cLeaf_t*)CM_Hunk_Alloc(44 * count, "CMod_LoadLeafs", 25);
    cm.numLeafs = count;
    cluster = 0;
    out = cm.leafs;
    for (leafIter = 0; leafIter < count; ++leafIter)
    {
        if (usePvs)
        {
            cluster = *in;
            out->cluster = *in;
            if (out->cluster != cluster)
                Com_Error(ERR_DROP, "CMod_LoadLeafs: cluster exceeded");
        }
        firstCollAabbIndex = *(in + 2);
        out->firstCollAabbIndex = firstCollAabbIndex;
        if (out->firstCollAabbIndex != firstCollAabbIndex)
            Com_Error(ERR_DROP, "CMod_LoadLeafs: firstCollAabbIndex exceeded");
        collAabbCount = *(in + 3);
        out->collAabbCount = collAabbCount;
        if (out->collAabbCount != collAabbCount)
            Com_Error(ERR_DROP, "CMod_LoadLeafs: collAabbCount exceeded");
        if (usePvs && cluster >= cm.numClusters)
            cm.numClusters = cluster + 1;
        in += 36;
        ++out;
    }
}

const DiskLeaf *CMod_LoadLeafBrushNodes()
{
    const DiskLeaf *result; // eax
    int contents; // [esp+0h] [ebp-24h]
    cLeaf_t *out; // [esp+4h] [ebp-20h]
    unsigned int numLeafBrushes; // [esp+8h] [ebp-1Ch]
    unsigned int leafIter; // [esp+Ch] [ebp-18h]
    const DiskLeaf *in; // [esp+10h] [ebp-14h]
    int indexFirstLeafBrush; // [esp+14h] [ebp-10h]
    unsigned int count; // [esp+18h] [ebp-Ch] BYREF
    unsigned int brushIter; // [esp+1Ch] [ebp-8h]
    int brushnum; // [esp+20h] [ebp-4h]

    result = (const DiskLeaf*)Com_GetBspLump(LUMP_LEAFS, 0x18u, &count);
    in = result;
    if (count != cm.numLeafs)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 787, 0, "count == cm.numLeafs\n\t%i, %i", count, cm.numLeafs);
    out = cm.leafs;
    for (leafIter = 0; leafIter < cm.numLeafs; ++leafIter)
    {
        numLeafBrushes = in->numLeafBrushes;
        indexFirstLeafBrush = in->firstLeafBrush;
        contents = 0;
        for (brushIter = 0; brushIter < numLeafBrushes; ++brushIter)
        {
            brushnum = cm.leafbrushes[brushIter + indexFirstLeafBrush];
            contents |= cm.brushes[brushnum].contents;
        }
        out->brushContents = contents;
        out->terrainContents = CMod_GetLeafTerrainContents(out);
        CMod_PartionLeafBrushes(&cm.leafbrushes[indexFirstLeafBrush], numLeafBrushes, out);
        result = ++in;
        ++out;
    }
    return result;
}

char *CMod_LoadLeafBrushNodes_Version14()
{
    char *result; // eax
    int contents; // [esp+0h] [ebp-24h]
    cLeaf_t *out; // [esp+4h] [ebp-20h]
    unsigned int numLeafBrushes; // [esp+8h] [ebp-1Ch]
    unsigned int leafIter; // [esp+Ch] [ebp-18h]
    const DiskLeaf_Version14 *in; // [esp+10h] [ebp-14h]
    int indexFirstLeafBrush; // [esp+14h] [ebp-10h]
    unsigned int count; // [esp+18h] [ebp-Ch] BYREF
    unsigned int brushIter; // [esp+1Ch] [ebp-8h]
    int brushnum; // [esp+20h] [ebp-4h]

    result = Com_GetBspLump(LUMP_LEAFS, 0x24u, &count);
    in = (const DiskLeaf_Version14*)result;
    if (count != cm.numLeafs)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 830, 0, "count == cm.numLeafs\n\t%i, %i", count, cm.numLeafs);
    if (!cm.numLeafs)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 832, 0, "%s", "cm.numLeafs > 0");
    out = cm.leafs;
    leafIter = 0;
    while (leafIter < cm.numLeafs)
    {
        numLeafBrushes = in->numLeafBrushes;
        indexFirstLeafBrush = in->firstLeafBrush;
        contents = 0;
        for (brushIter = 0; brushIter < numLeafBrushes; ++brushIter)
        {
            brushnum = cm.leafbrushes[brushIter + indexFirstLeafBrush];
            contents |= cm.brushes[brushnum].contents;
        }
        out->brushContents = contents;
        out->terrainContents = CMod_GetLeafTerrainContents(out);
        CMod_PartionLeafBrushes(&cm.leafbrushes[indexFirstLeafBrush], numLeafBrushes, out);
        ++leafIter;
        result = (char*)&in[1];
        ++in;
        ++out;
    }
    return result;
}

unsigned int CMod_LoadLeafBrushes()
{
    unsigned int result; // eax
    unsigned __int16 *out; // [esp+0h] [ebp-18h]
    unsigned int brushIndex; // [esp+4h] [ebp-14h]
    char *in; // [esp+Ch] [ebp-Ch]
    unsigned int iter; // [esp+10h] [ebp-8h]
    unsigned int count; // [esp+14h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_LEAFBRUSHES, 4u, &count);
    cm.leafbrushes = (unsigned short*)CM_Hunk_Alloc(2 * count + 2, "CMod_LoadLeafBrushes", 26);
    cm.numLeafBrushes = count;
    out = cm.leafbrushes;
    for (iter = 0; ; ++iter)
    {
        result = iter;
        if (iter >= count)
            break;
        brushIndex = *in;
        *out = *in;
        if (*out != brushIndex)
            Com_Error(ERR_DROP, "CMod_LoadLeafBrushes: brushIndex exceeded");
        if (*out >= cm.numBrushes)
            MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 936, 0, "%s\n\t(*out) = %i", "(*out < cm.numBrushes)", *out);
        in += 4;
        ++out;
    }
    return result;
}

char *CMod_LoadCollisionAabbTrees()
{
    char *result; // eax
    CollisionAabbTree *out; // [esp+0h] [ebp-14h]
    char *in; // [esp+8h] [ebp-Ch]
    unsigned int index; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONAABBS, 0x20u, &count);
    cm.aabbTrees = (CollisionAabbTree*)CM_Hunk_Alloc(32 * count, "CMod_LoadCollisionAabbTrees", 28);
    result = (char*)count;
    cm.aabbTreeCount = count;
    out = cm.aabbTrees;
    index = 0;
    while (index < count)
    {
        out->origin[0] = *in;
        out->origin[1] = *(in + 1);
        out->origin[2] = *(in + 2);
        out->halfSize[0] = *(in + 3);
        out->halfSize[1] = *(in + 4);
        out->halfSize[2] = *(in + 5);
        out->materialIndex = *(in + 12);
        out->childCount = *(in + 13);
        out->u.firstChildIndex = *(in + 7);
        ++index;
        result = in + 32;
        in += 32;
        ++out;
    }
    return result;
}

