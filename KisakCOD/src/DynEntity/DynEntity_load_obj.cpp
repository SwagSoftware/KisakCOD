#include "DynEntity_client.h"
#include <xanim/xmodel.h>
#include <universal/com_memory.h>
#include <EffectsCore/fx_system.h>
#include <universal/q_parse.h>
#include <qcommon/com_bsp.h>

const char *dynEntClassNames[2] =
{
    "dyn_brushmodel",
    "dyn_model"
};

const DynEntityProps dynEntProps[3] =
{
    { "invalid", false, false, false, false },
    { "clutter", true, false, true, false },
    { "destruct", true, false, true, true }
};

unsigned __int8 *__cdecl DynEnt_AllocXModel(int size)
{
    if (size <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 160, 0, "%s", "size > 0");
    return Hunk_Alloc(size, "DynEnt_AllocXModel", 21);
}

unsigned __int8 *__cdecl DynEnt_AllocXModelColl(int size)
{
    if (size <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 170, 0, "%s", "size > 0");
    return Hunk_Alloc(size, "DynEnt_AllocXModelColl", 27);
}

char __cdecl DynEnt_IsValidClassName(const char *className)
{
    unsigned int classIndex; // [esp+0h] [ebp-4h]

    if (!className)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 74, 0, "%s", "className");
    for (classIndex = 0; classIndex < 2; ++classIndex)
    {
        if (!I_stricmp(className, dynEntClassNames[classIndex]))
            return 1;
    }
    return 0;
}

XModel *__cdecl DynEnt_XModelPrecache(const char *modelName)
{
    if (!modelName)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 180, 0, "%s", "modelName");
    return XModelPrecache(
        (char*)modelName,
        (void *(__cdecl *)(int))DynEnt_AllocXModel,
        (void *(__cdecl *)(int))DynEnt_AllocXModelColl);
}

int __cdecl DynEnt_GetType(const char *typeName)
{
    int type; // [esp+0h] [ebp-4h]

    if (!typeName)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 93, 0, "%s", "typeName");
    for (type = 0; type < 3; ++type)
    {
        if (!I_stricmp(typeName, dynEntProps[type].name))
            return type;
    }
    return 0;
}

unsigned __int8 *__cdecl DynEnt_AllocXModelPieces(int size)
{
    if (size <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 200, 0, "%s", "size > 0");
    return Hunk_Alloc(size, "DynEnt_AllocXModelPieces", 21);
}

XModelPieces *__cdecl DynEnt_XModelPiecesPrecache(const char *name)
{
    if (!name)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 220, 0, "%s", "name");
    return XModelPiecesPrecache(name, (void *(__cdecl *)(int))DynEnt_AllocXModelPieces);
}

unsigned __int8 *__cdecl DynEnt_AllocPhysPreset(int size)
{
    if (size <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 190, 0, "%s", "size > 0");
    return Hunk_Alloc(size, "DynEnt_AllocPhysPreset", 21);
}

PhysPreset *__cdecl DynEnt_PhysPresetPrecache(const char *name)
{
    if (!name)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 210, 0, "%s", "name");
    return PhysPresetPrecache(name, (void *(__cdecl *)(int))DynEnt_AllocPhysPreset);
}

char __cdecl DynEnt_Create(DynEntityDef *dynEntDef, const DynEntityCreateParams *params)
{
    unsigned int brushModel; // [esp+58h] [ebp-4h]
    int brushModela; // [esp+58h] [ebp-4h]

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 233, 0, "%s", "dynEntDef");
    if (!params)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 234, 0, "%s", "params");
    if ((COERCE_UNSIGNED_INT(params->origin[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(params->origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(params->origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            236,
            0,
            "%s",
            "!IS_NAN((params->origin)[0]) && !IS_NAN((params->origin)[1]) && !IS_NAN((params->origin)[2])");
    }
    if ((COERCE_UNSIGNED_INT(params->angles[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(params->angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(params->angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            237,
            0,
            "%s",
            "!IS_NAN((params->angles)[0]) && !IS_NAN((params->angles)[1]) && !IS_NAN((params->angles)[2])");
    }
    Com_Memset((unsigned int *)dynEntDef, 0, 96);
    if (params->typeName[0])
    {
        dynEntDef->type = (DynEntityType)DynEnt_GetType(params->typeName);
        if (dynEntDef->type == DYNENT_TYPE_INVALID)
        {
            Com_Error(ERR_DROP, "Invalid Dyn Entity type [%s]\n", params->typeName);
            return 0;
        }
    }
    else
    {
        dynEntDef->type = DYNENT_TYPE_CLUTTER;
    }
    if (params->modelName[0] == 42)
    {
        brushModel = atoi(&params->modelName[1]);
        dynEntDef->brushModel = brushModel;
        if (dynEntDef->brushModel != brushModel)
            MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 264, 0, "%s", "dynEntDef->brushModel == brushModel");
        if (brushModel >= cm.numSubModels)
            MyAssertHandler(
                ".\\DynEntity\\DynEntity_load_obj.cpp",
                266,
                0,
                "brushModel doesn't index cm.numSubModels\n\t%i not in [0, %i)",
                brushModel,
                cm.numSubModels);
        dynEntDef->contents = cm.cmodels[brushModel].leaf.terrainContents | cm.cmodels[brushModel].leaf.brushContents;
    }
    else
    {
        dynEntDef->xModel = DynEnt_XModelPrecache(params->modelName);
        if (!dynEntDef->xModel)
        {
            Com_Error(ERR_DROP, "Couldn't find xmodel [%s] for Dyn Entity.\n", params->modelName);
            return 0;
        }
        dynEntDef->contents = XModelGetContents(dynEntDef->xModel);
        if (dynEntDef->xModel->collLod < 0)
        {
            Com_Error(ERR_DROP, "Dyn Entity xmodel [%s] has no collision LOD set.\n", params->modelName);
            return 0;
        }
    }
    if (params->physModelName[0] == 42)
    {
        brushModela = atoi(&params->physModelName[1]);
        dynEntDef->physicsBrushModel = brushModela;
        if (dynEntDef->physicsBrushModel != brushModela)
            MyAssertHandler(
                ".\\DynEntity\\DynEntity_load_obj.cpp",
                294,
                0,
                "%s",
                "dynEntDef->physicsBrushModel == brushModel");
    }
    if (!params->destroyFxFile[0] || (dynEntDef->destroyFx = FX_Register(params->destroyFxFile)) != 0)
    {
        if (!params->destroyPiecesFile[0]
            || (dynEntDef->destroyPieces = DynEnt_XModelPiecesPrecache(params->destroyPiecesFile)) != 0)
        {
            dynEntDef->physPreset = 0;
            if (DynEnt_GetEntityProps(dynEntDef->type)->usePhysics)
            {
                if (dynEntDef->xModel && dynEntDef->xModel->physPreset)
                    dynEntDef->physPreset = dynEntDef->xModel->physPreset;
                if (params->physPresetFile[0])
                    dynEntDef->physPreset = DynEnt_PhysPresetPrecache(params->physPresetFile);
                if (!dynEntDef->physPreset)
                {
                    dynEntDef->physPreset = DynEnt_PhysPresetPrecache("default");
                    if (dynEntDef->xModel)
                        Com_PrintError(
                            20,
                            "ERROR: no physics preset specified for the DynEntity at [%.1f,%.1f,%.1f] with xModel [%s]\n",
                            params->origin[0],
                            params->origin[1],
                            params->origin[2],
                            params->modelName);
                    else
                        Com_PrintError(
                            20,
                            "ERROR: no physics preset specified for the DynEntity at [%.1f,%.1f,%.1f]\n",
                            params->origin[0],
                            params->origin[1],
                            params->origin[2]);
                }
            }
            dynEntDef->pose.origin[0] = params->origin[0];
            dynEntDef->pose.origin[1] = params->origin[1];
            dynEntDef->pose.origin[2] = params->origin[2];
            AnglesToQuat(params->angles, dynEntDef->pose.quat);
            dynEntDef->mass = *(PhysMass *)params->centerOfMass;
            dynEntDef->health = params->health;
            return 1;
        }
        else
        {
            Com_Error(ERR_DROP, "Couldn't find pieces [%s] for Dyn Entity.\n", params->destroyPiecesFile);
            return 0;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Couldn't find fx [%s] for Dyn Entity.\n", params->destroyFxFile);
        return 0;
    }
}

int __cdecl DynEnt_GetEntityCountFromString(const char *entityString)
{
    char key[68]; // [esp+0h] [ebp-98h] BYREF
    const char *ptr; // [esp+44h] [ebp-54h] BYREF
    const char *token; // [esp+48h] [ebp-50h]
    int count; // [esp+4Ch] [ebp-4Ch]
    char value[68]; // [esp+50h] [ebp-48h] BYREF

    if (!entityString)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 116, 0, "%s", "entityString");
    ptr = entityString;
    count = 0;
    while (1)
    {
        token = (const char *)Com_Parse(&ptr);
        if (!ptr || *token != 123)
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
            if (!I_stricmp(key, "classname") && DynEnt_IsValidClassName(value))
                ++count;
        }
    }
    return count;
}

int __cdecl DynEnt_CompareEntities(_DWORD *arg0, _DWORD *arg1)
{
    int hasModel0; // [esp+8h] [ebp-14h]
    int value1; // [esp+10h] [ebp-Ch]
    int hasModel1; // [esp+14h] [ebp-8h]
    int value0; // [esp+18h] [ebp-4h]

    if (!arg0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 361, 0, "%s", "arg0");
    if (!arg1)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 362, 0, "%s", "arg1");
    if (*arg0 >= 3u)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            367,
            0,
            "dynEntDef0->type doesn't index DYNENT_TYPE_COUNT\n\t%i not in [0, %i)",
            *arg0,
            3);
    if (*arg1 >= 3u)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            368,
            0,
            "dynEntDef1->type doesn't index DYNENT_TYPE_COUNT\n\t%i not in [0, %i)",
            *arg1,
            3);
    hasModel0 = arg0[8] != 0;
    hasModel1 = arg1[8] != 0;
    if (hasModel0 != hasModel1)
        return hasModel0 - hasModel1;
    value0 = dynEntProps[*arg0].clientOnly;
    value1 = dynEntProps[*arg1].clientOnly;
    if (value0 != value1)
        return value0 - value1;
    if (arg0[8])
    {
        if (!arg1[8])
            MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 387, 0, "%s", "dynEntDef1->xModel");
        return arg0[8] < arg1[8] ? -1 : 1;
    }
    else
    {
        if (arg1[8])
            MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 392, 0, "%s", "!dynEntDef1->xModel");
        return *((unsigned __int16 *)arg0 + 18) - *((unsigned __int16 *)arg1 + 18);
    }
}

unsigned __int8 *__cdecl DynEnt_Alloc(int count, int size)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-4h]

    if (count <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 404, 0, "%s", "count > 0");
    if (size <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 405, 0, "%s", "size > 0");
    buf = Hunk_Alloc(size * count, "DynEnt_LoadEntities", 9);
    Com_Memset((unsigned int *)buf, 0, size * count);
    return buf;
}

void __cdecl DynEnt_LoadEntities()
{
    char *v0; // eax
    char v1; // [esp+3h] [ebp-2D1h]
    char *physPresetFile; // [esp+8h] [ebp-2CCh]
    char *v3; // [esp+Ch] [ebp-2C8h]
    char v4; // [esp+13h] [ebp-2C1h]
    char *v5; // [esp+18h] [ebp-2BCh]
    char *v6; // [esp+1Ch] [ebp-2B8h]
    char v7; // [esp+23h] [ebp-2B1h]
    char *physModelName; // [esp+28h] [ebp-2ACh]
    char *v9; // [esp+2Ch] [ebp-2A8h]
    char v10; // [esp+33h] [ebp-2A1h]
    char *v11; // [esp+38h] [ebp-29Ch]
    char *v12; // [esp+3Ch] [ebp-298h]
    char v13; // [esp+43h] [ebp-291h]
    char *modelName; // [esp+48h] [ebp-28Ch]
    char *v15; // [esp+4Ch] [ebp-288h]
    char v16; // [esp+53h] [ebp-281h]
    DynEntityCreateParams *p_params; // [esp+58h] [ebp-27Ch]
    char *v18; // [esp+5Ch] [ebp-278h]
    unsigned int drawType; // [esp+60h] [ebp-274h]
    unsigned int drawTypea; // [esp+60h] [ebp-274h]
    int dynEntIndex; // [esp+68h] [ebp-26Ch]
    unsigned __int8 *dynEntDefList; // [esp+6Ch] [ebp-268h]
    int dynEntStringCount; // [esp+70h] [ebp-264h]
    DynEntityDef *dynEntDef; // [esp+74h] [ebp-260h]
    signed int dynEntCount; // [esp+78h] [ebp-25Ch]
    char key[64]; // [esp+7Ch] [ebp-258h] BYREF
    DynEntityCreateParams params; // [esp+BCh] [ebp-218h] BYREF
    const char *ptr; // [esp+280h] [ebp-54h] BYREF
    bool isDynEnt; // [esp+287h] [ebp-4Dh]
    const char *token; // [esp+288h] [ebp-4Ch]
    char value[68]; // [esp+28Ch] [ebp-48h] BYREF

    if (!Com_EntityString(0))
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 433, 0, "%s", "Com_EntityString( NULL )");
    cm.dynEntDefList[0] = 0;
    cm.dynEntDefList[1] = 0;
    cm.dynEntPoseList[0] = 0;
    cm.dynEntPoseList[1] = 0;
    cm.dynEntClientList[0] = 0;
    cm.dynEntClientList[1] = 0;
    cm.dynEntCollList[0] = 0;
    cm.dynEntCollList[1] = 0;
    *(unsigned int *)cm.dynEntCount = 0;
    v0 = Com_EntityString(0);
    dynEntStringCount = DynEnt_GetEntityCountFromString(v0);
    if (dynEntStringCount >= 4096)
        Com_Error(ERR_DROP, "Found [%d] Dyn Entities, Max is [%d]\n", dynEntStringCount, 4095);
    if (dynEntStringCount)
    {
        dynEntDefList = Hunk_Alloc(96 * dynEntStringCount, "DynEnt_LoadDefs", 9);
        dynEntCount = 0;
        ptr = Com_EntityString(0);
        while (1)
        {
            token = (const char *)Com_Parse(&ptr);
            if (!ptr || *token != 123)
                break;
            isDynEnt = 0;
            Com_Memset((unsigned int *)&params, 0, 448);
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
                    if (I_stricmp(key, "type"))
                    {
                        if (I_stricmp(key, "model"))
                        {
                            if (I_stricmp(key, "physicsmodel"))
                            {
                                if (I_stricmp(key, "destroyEfx"))
                                {
                                    if (I_stricmp(key, "destroyPieces"))
                                    {
                                        if (I_stricmp(key, "origin"))
                                        {
                                            if (I_stricmp(key, "angles"))
                                            {
                                                if (I_stricmp(key, "health"))
                                                {
                                                    if (I_stricmp(key, "physPreset"))
                                                    {
                                                        if (I_stricmp(key, "centerofmass"))
                                                        {
                                                            if (I_stricmp(key, "momofinertia"))
                                                            {
                                                                if (!I_stricmp(key, "prodofinertia"))
                                                                    sscanf_s(value, "%f %f %f", 
                                                                        &params.productsOfInertia[0], sizeof(float),
                                                                        &params.productsOfInertia[1], sizeof(float), 
                                                                        &params.productsOfInertia[2], sizeof(float));
                                                            }
                                                            else
                                                            {
                                                                sscanf_s(
                                                                    value,
                                                                    "%f %f %f",
                                                                    &params.momentsOfInertia[0], sizeof(float),
                                                                    &params.momentsOfInertia[1], sizeof(float),
                                                                    &params.momentsOfInertia[2], sizeof(float));

                                                            }
                                                        }
                                                        else
                                                        {
                                                            sscanf_s(
                                                                value,
                                                                "%f %f %f",
                                                                &params.centerOfMass[0], sizeof(float),
                                                                &params.centerOfMass[1], sizeof(float),
                                                                &params.centerOfMass[2], sizeof(float));

                                                        }
                                                    }
                                                    else
                                                    {
                                                        v3 = value;
                                                        physPresetFile = params.physPresetFile;
                                                        do
                                                        {
                                                            v1 = *v3;
                                                            *physPresetFile++ = *v3++;
                                                        } while (v1);
                                                    }
                                                }
                                                else
                                                {
                                                    params.health = atoi(value);
                                                }
                                            }
                                            else
                                            {
                                                sscanf_s(value, "%f %f %f", &params.angles[0], sizeof(float), &params.angles[1], sizeof(float), &params.angles[2], sizeof(float));
                                            }
                                        }
                                        else
                                        {
                                            sscanf_s(value, "%f %f %f", &params.origin[0], sizeof(float), &params.origin[1], sizeof(float), &params.origin[2], sizeof(float));
                                        }
                                    }
                                    else
                                    {
                                        Com_StripExtension(value, params.destroyPiecesFile);
                                    }
                                }
                                else
                                {
                                    Com_StripExtension(value, params.destroyFxFile);
                                }
                            }
                            else if (Com_IsLegacyXModelName(value))
                            {
                                v9 = &value[7];
                                physModelName = params.physModelName;
                                do
                                {
                                    v7 = *v9;
                                    *physModelName++ = *v9++;
                                } while (v7);
                            }
                            else
                            {
                                v6 = value;
                                v5 = params.physModelName;
                                do
                                {
                                    v4 = *v6;
                                    *v5++ = *v6++;
                                } while (v4);
                            }
                        }
                        else if (Com_IsLegacyXModelName(value))
                        {
                            v15 = &value[7];
                            modelName = params.modelName;
                            do
                            {
                                v13 = *v15;
                                *modelName++ = *v15++;
                            } while (v13);
                        }
                        else
                        {
                            v12 = value;
                            v11 = params.modelName;
                            do
                            {
                                v10 = *v12;
                                *v11++ = *v12++;
                            } while (v10);
                        }
                    }
                    else
                    {
                        v18 = value;
                        p_params = &params;
                        do
                        {
                            v16 = *v18;
                            p_params->typeName[0] = *v18++;
                            p_params = (DynEntityCreateParams *)((char *)p_params + 1);
                        } while (v16);
                    }
                }
                else
                {
                    isDynEnt = DynEnt_IsValidClassName(value);
                }
            }
            if (isDynEnt)
            {
                if (dynEntCount >= dynEntStringCount)
                    MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 545, 0, "%s", "dynEntCount < dynEntStringCount");
                if (DynEnt_Create((DynEntityDef *)&dynEntDefList[96 * dynEntCount], &params))
                    ++dynEntCount;
            }
        }
        if (dynEntCount)
        {
            qsort(dynEntDefList, dynEntCount, 0x60u, (int(__cdecl *)(const void *, const void *))DynEnt_CompareEntities);
            for (dynEntIndex = 0; dynEntIndex < dynEntCount; ++dynEntIndex)
            {
                dynEntDef = (DynEntityDef *)&dynEntDefList[96 * dynEntIndex];
                if (dynEntDef->xModel)
                {
                    drawType = 0;
                }
                else
                {
                    if (!dynEntDef->brushModel)
                        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 566, 0, "%s", "dynEntDef->brushModel");
                    drawType = 1;
                }
                if (!cm.dynEntDefList[drawType])
                    cm.dynEntDefList[drawType] = dynEntDef;
                ++cm.dynEntCount[drawType];
            }
            if (cm.dynEntCount[1] + cm.dynEntCount[0] != dynEntCount)
                MyAssertHandler(
                    ".\\DynEntity\\DynEntity_load_obj.cpp",
                    585,
                    0,
                    "%s",
                    "cm.dynEntCount[DYNENT_COLL_CLIENT_MODEL] + cm.dynEntCount[DYNENT_COLL_CLIENT_BRUSH] == dynEntCount");
            for (drawTypea = 0; drawTypea < 2; ++drawTypea)
            {
                if (cm.dynEntCount[drawTypea])
                {
                    cm.dynEntPoseList[drawTypea] = (DynEntityPose *)DynEnt_Alloc(cm.dynEntCount[drawTypea], 32);
                    cm.dynEntClientList[drawTypea] = (DynEntityClient *)DynEnt_Alloc(cm.dynEntCount[drawTypea], 12);
                    cm.dynEntCollList[drawTypea] = (DynEntityColl *)DynEnt_Alloc(cm.dynEntCount[drawTypea], 20);
                }
            }
        }
    }
}

const DynEntityProps *__cdecl DynEnt_GetEntityProps(DynEntityType dynEntType)
{
    if (dynEntType == DYNENT_TYPE_INVALID)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 618, 0, "%s", "dynEntType != DYNENT_TYPE_INVALID");
    if ((unsigned int)dynEntType >= DYNENT_TYPE_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            619,
            0,
            "dynEntType doesn't index DYNENT_TYPE_COUNT\n\t%i not in [0, %i)",
            dynEntType,
            3);
    //return (const DynEntityProps *)(8 * dynEntType + 8895660);
    return &dynEntProps[dynEntType];
}

unsigned __int16 __cdecl DynEnt_GetId(const DynEntityDef *dynEntDef, DynEntityDrawType drawType)
{
    unsigned __int16 dynEntId; // [esp+0h] [ebp-4h]

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 632, 0, "%s", "dynEntDef");
    dynEntId = dynEntDef - cm.dynEntDefList[drawType];
    if (dynEntId >= (unsigned int)cm.dynEntCount[drawType])
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            636,
            1,
            "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
            dynEntId,
            cm.dynEntCount[drawType]);
    return dynEntId;
}

unsigned __int16 __cdecl DynEnt_GetEntityCount(DynEntityCollType collType)
{
    return cm.dynEntCount[collType];
}

const DynEntityDef *__cdecl DynEnt_GetEntityDef(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if (dynEntId >= (unsigned int)cm.dynEntCount[drawType])
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            656,
            0,
            "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
            dynEntId,
            cm.dynEntCount[drawType]);
    return &cm.dynEntDefList[drawType][dynEntId];
}

DynEntityPose *__cdecl DynEnt_GetClientModelPoseList()
{
    return cm.dynEntPoseList[0];
}

DynEntityPose *__cdecl DynEnt_GetClientPose(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if (dynEntId >= (unsigned int)cm.dynEntCount[drawType])
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            684,
            0,
            "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
            dynEntId,
            cm.dynEntCount[drawType]);
    return &cm.dynEntPoseList[drawType][dynEntId];
}

DynEntityClient *__cdecl DynEnt_GetClientEntity(unsigned __int16 dynEntId, DynEntityDrawType drawType)
{
    if (dynEntId >= (unsigned int)cm.dynEntCount[drawType])
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            694,
            0,
            "dynEntId doesn't index cm.dynEntCount[DynEntGetClientCollType( drawType )]\n\t%i not in [0, %i)",
            dynEntId,
            cm.dynEntCount[drawType]);
    return &cm.dynEntClientList[drawType][dynEntId];
}

DynEntityColl *__cdecl DynEnt_GetEntityColl(DynEntityCollType collType, unsigned __int16 dynEntId)
{
    if ((unsigned int)collType >= DYNENT_COLL_COUNT)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            718,
            0,
            "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
            collType,
            2);
    if (dynEntId >= (unsigned int)cm.dynEntCount[collType])
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_load_obj.cpp",
            719,
            0,
            "dynEntId doesn't index cm.dynEntCount[collType]\n\t%i not in [0, %i)",
            dynEntId,
            cm.dynEntCount[collType]);
    return &cm.dynEntCollList[collType][dynEntId];
}

int __cdecl DynEnt_GetXModelUsageCount(const XModel *xModel)
{
    unsigned int drawType; // [esp+0h] [ebp-Ch]
    unsigned __int16 dynEntId; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    if (!xModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_load_obj.cpp", 839, 0, "%s", "xModel");
    count = 0;
    for (drawType = 0; drawType < 2; ++drawType)
    {
        for (dynEntId = 0; dynEntId < (int)cm.dynEntCount[drawType]; ++dynEntId)
        {
            if (cm.dynEntDefList[drawType][dynEntId].xModel == xModel)
                ++count;
        }
    }
    return count;
}

