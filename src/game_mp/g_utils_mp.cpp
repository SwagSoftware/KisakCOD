#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_utils_mp.h"
#include <qcommon/mem_track.h>
#include <universal/com_constantconfigstrings.h>
#include <client_mp/client_mp.h>
#include <server_mp/server_mp.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel.h>

#include <script/scr_const.h>
#include <script/scr_memorytree.h>
#include <script/scr_vm.h>
#include <stringed/stringed_hooks.h>
#include <server/sv_game.h>
#include "g_main_mp.h"
#include <database/database.h>
#include "g_public_mp.h"
#include <server/sv_world.h>
#include <universal/profile.h>

XModel *cached_models[512];

void __cdecl G_SafeDObjFree(unsigned int handle, int unusedLocalClientNum)
{
    if (unusedLocalClientNum != -1)
        MyAssertHandler(
            ".\\game_mp\\g_main_mp.cpp",
            850,
            0,
            "unusedLocalClientNum == UNUSED_LOCAL_CLIENT_NUM\n\t%i, %i",
            unusedLocalClientNum,
            -1);
    Com_SafeServerDObjFree(handle);
}

void __cdecl TRACK_g_utils()
{
    //track_static_alloc_internal(entityTypeNames, 68, "entityTypeNames", 9);
}

int __cdecl G_FindConfigstringIndex(char *name, int start, int max, int create, const char *errormsg)
{
    const char *v6; // eax
    unsigned int ConfigstringConst; // eax
    char *v8; // eax
    const char *v9; // eax
    HashEntry_unnamed_type_u v10; // [esp+0h] [ebp-14h]
    unsigned int s; // [esp+Ch] [ebp-8h]
    signed int i; // [esp+10h] [ebp-4h]
    signed int ia; // [esp+10h] [ebp-4h]
    int ib; // [esp+10h] [ebp-4h]
    int ic; // [esp+10h] [ebp-4h]

    if (!name || !*name)
        return 0;
    if (start < 821)
        v10.prev = SL_FindString(name);
    else
        v10.prev = SL_FindLowercaseString(name);
    if (create
        && (i = CCS_GetConstConfigStringIndex(name), i >= 0)
        && (ia = CCS_GetConfigStringNumForConstIndex(i), ia >= start)
        && ia < max + start)
    {
        s = SV_GetConfigstringConst(ia);
        if (s == v10.prev)
        {
            return ia - start;
        }
        else
        {
            if (s != scr_const._)
                MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 92, 0, "%s", "s == scr_const._");
            SV_SetConfigstring(ia, name);
            return ia - start;
        }
    }
    else
    {
        for (ib = 1; ib < max; ++ib)
        {
            if (SV_GetConfigstringConst(ib + start) == v10.prev)
                return ib;
        }
        if (create)
        {
            for (ic = 1;
                ic < max
                && (SV_GetConfigstringConst(ic + start) != scr_const._ || CCS_IsConfigStringIndexConstant(ic + start));
                ++ic)
            {
                ;
            }
            if (ic == max)
            {
                Com_PrintWarning(14, "Warning: abandoning const config string model slot for string %s\n", name);
                for (ic = 1; ic < max && SV_GetConfigstringConst(ic + start) != scr_const._; ++ic)
                    ;
            }
            if (ic == max)
            {
                Com_Printf(15, "G_FindConfigstringIndex: overflow...\n");
                Com_Printf(15, "Dumping these %i Config Strings:\n", max);
                for (ic = 1; ic < max; ++ic)
                {
                    ConfigstringConst = SV_GetConfigstringConst(ic + start);
                    v8 = SL_ConvertToString(ConfigstringConst);
                    Com_Printf(15, "%i: %s\n", ic, v8);
                }
                v9 = va("G_FindConfigstringIndex: overflow (%d): %s", start, name);
                Com_Error(ERR_DROP, v9);
            }
            SV_SetConfigstring(ic + start, name);
            return ic;
        }
        else
        {
            if (errormsg)
            {
                v6 = va("%s \"%s\" not precached", errormsg, name);
                Scr_Error(v6);
            }
            return 0;
        }
    }
}

const char *origErrorMsg;
int __cdecl G_LocalizedStringIndex(char *string)
{
    int v2; // eax
    int configStringIndex; // [esp+0h] [ebp-Ch]
    const char *errormsg; // [esp+4h] [ebp-8h]
    int allowCreate; // [esp+8h] [ebp-4h]

    if (!string)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 157, 0, "%s", "string");
    if (!*string)
        return 0;
    allowCreate = level.initializing;
    errormsg = origErrorMsg;
    if (!level.initializing)
    {
        if (!loc_warnings->current.enabled)
        {
            allowCreate = 1;
            v2 = G_FindConfigstringIndex(string, 309, 512, 1, origErrorMsg);
            goto LABEL_11;
        }
        if (!loc_warningsAsErrors->current.enabled)
            errormsg = 0;
    }
    v2 = G_FindConfigstringIndex(string, 309, 512, level.initializing, errormsg);
LABEL_11:
    configStringIndex = v2;
    if (!v2 && !allowCreate && loc_warnings->current.enabled && !loc_warningsAsErrors->current.enabled)
    {
        configStringIndex = G_FindConfigstringIndex(string, 309, 512, 1, origErrorMsg);
        if (configStringIndex)
            Com_PrintWarning(24, "WARNING: %s \"%s\" not precached\n", origErrorMsg, string);
    }
    return configStringIndex;
}

int __cdecl G_MaterialIndex(const char *name)
{
    char v2; // [esp+3h] [ebp-55h]
    char *v3; // [esp+8h] [ebp-50h]
    const char *v4; // [esp+Ch] [ebp-4Ch]
    char shaderName[68]; // [esp+10h] [ebp-48h] BYREF

    if (!name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 201, 0, "%s", "name");
    if (!*name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 202, 0, "%s", "name[0]");
    v4 = name;
    v3 = shaderName;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    } while (v2);
    I_strlwr(shaderName);
    return G_FindConfigstringIndex(shaderName, 2002, 256, level.initializing, "material");
}

int __cdecl G_ModelIndex(char *name)
{
    const char *v2; // eax
    unsigned int nameString; // [esp+68h] [ebp-10h]
    unsigned int s; // [esp+6Ch] [ebp-Ch]
    int i; // [esp+70h] [ebp-8h]
    signed int constIndex; // [esp+74h] [ebp-4h]
    signed int constIndexa; // [esp+74h] [ebp-4h]

    PROF_SCOPED("G_ModelIndex");
    if (!name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 221, 0, "%s", "name");
    if (!*name)
    {
        return 0;
    }
    nameString = SL_FindLowercaseString(name);
    if (!level.initializing
        || (constIndex = CCS_GetConstConfigStringIndex(name), constIndex < 0)
        || (constIndexa = CCS_GetConfigStringNumForConstIndex(constIndex), constIndexa < 830)
        || constIndexa >= 1342)
    {
        for (i = 1; i < 512; ++i)
        {
            if (SV_GetConfigstringConst(i + 830) == nameString)
            {
                if (!cached_models[i])
                    MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 262, 0, "%s", "cached_models[i]");
                goto LABEL_14;
            }
        }
        if (!level.initializing)
        {
            v2 = va("model '%s' not precached", name);
            Scr_Error(v2);
        }
        if (level.initializing && i == 512)
        {
            for (i = 1;
                i < 512 && (SV_GetConfigstringConst(i + 830) != scr_const._ || CCS_IsConfigStringIndexConstant(i + 830));
                ++i)
            {
                ;
            }
            if (i == 512)
            {
                Com_PrintWarning(14, "Warning: abandoning const config string model slot for string %s\n", name);
                for (i = 1; i < 512 && SV_GetConfigstringConst(i + 830) != scr_const._; ++i)
                    ;
            }
        }
        goto haveIndex;
    }
    s = SV_GetConfigstringConst(constIndexa);
    if (s != nameString)
    {
        if (s != scr_const._)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 249, 0, "%s", "s == scr_const._");
        i = constIndexa - 830;
        if ((unsigned int)(constIndexa - 830) >= 0x200)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 251, 0, "i doesn't index MAX_MODELS\n\t%i not in [0, %i)", i, 512);
    haveIndex:
        if ((unsigned int)i >= 0x200)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 298, 0, "i doesn't index MAX_MODELS\n\t%i not in [0, %i)", i, 512);
        if (i == 512)
            Com_Error(ERR_DROP, "G_ModelIndex: overflow");
        cached_models[i] = SV_XModelGet(name);
        SV_SetConfigstring(i + 830, name);
        return i;
    }
    i = constIndexa - 830;
    if ((unsigned int)(constIndexa - 830) >= 0x200)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 243, 0, "i doesn't index MAX_MODELS\n\t%i not in [0, %i)", i, 512);
    if (!cached_models[i])
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 244, 0, "%s", "cached_models[i]");
LABEL_14:
    return i;
}

bool __cdecl G_GetModelBounds(int index, float *outMins, float *outMaxs)
{
    float identityBasis[3][3]; // [esp+0h] [ebp-28h] BYREF
    XModel *xmodel; // [esp+24h] [ebp-4h]

    if (!outMins)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 323, 0, "%s", "outMins");
    if (!outMaxs)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 324, 0, "%s", "outMaxs");
    xmodel = G_GetModel(index);
    if (!xmodel)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 326, 0, "%s", "xmodel");
    AxisClear(identityBasis);
    return XModelGetStaticBounds(xmodel, identityBasis, outMins, outMaxs) != 0;
}

XModel *__cdecl G_GetModel(int index)
{
    if (index <= 0)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 313, 0, "%s", "index > 0");
    if (index >= 512)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 314, 0, "%s", "index < MAX_MODELS");
    return cached_models[index];
}

bool __cdecl G_XModelBad(int index)
{
    const XModel *Model; // eax

    if (!index)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 338, 0, "%s", "index");
    Model = G_GetModel(index);
    return XModelBad(Model);
}

unsigned int __cdecl G_ModelName(unsigned int index)
{
    if (index >= 0x200)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 345, 0, "%s", "(unsigned)index < MAX_MODELS");
    return SV_GetConfigstringConst(index + 830);
}

int __cdecl G_TagIndex(char *name)
{
    if (!name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 352, 0, "%s", "name");
    return G_FindConfigstringIndex(name, 2282, 32, 1, 0);
}

int __cdecl G_EffectIndex(char *name)
{
    if (!name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 359, 0, "%s", "name");
    return G_FindConfigstringIndex(name, 1598, 100, level.initializing, "effect");
}

int __cdecl G_ShellShockIndex(char *name)
{
    if (!name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 366, 0, "%s", "name");
    return G_FindConfigstringIndex(name, 1954, 16, 1, 0);
}

int __cdecl G_SoundAliasIndex(char *name)
{
    if (!name)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 374, 0, "%s", "name");
    return G_FindConfigstringIndex(name, 1342, 256, 1, 0);
}

void __cdecl G_DObjUpdate(gentity_s *ent)
{
    DObj_s *dobj; // [esp+0h] [ebp-114h]
    XModel *model; // [esp+4h] [ebp-110h]
    int numModels; // [esp+8h] [ebp-10Ch]
    int i; // [esp+Ch] [ebp-108h]
    int modelIndex; // [esp+10h] [ebp-104h]
    int modelIndexa; // [esp+10h] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+14h] [ebp-100h] BYREF

    if (!ent->client)
    {
        G_SafeDObjFree(ent);
        modelIndex = ent->model;
        if (ent->model)
        {
            model = G_GetModel(modelIndex);
            if (!model)
                MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 420, 0, "%s", "model");
            dobjModels[0].model = model;
            dobjModels[0].boneName = 0;
            dobjModels[0].ignoreCollision = 0;
            numModels = 1;
            if (!ent->s.eType || ent->s.eType == 6 || ent->s.eType == 13 || ent->s.eType == 11)
            {
                ent->s.index.brushmodel = modelIndex;
            }
            else if (ent->s.eType == 14 || ent->s.eType == 12)
            {
                ent->s.un2.hintString = modelIndex;
            }
            for (i = 0; i < 19; ++i)
            {
                modelIndexa = ent->attachModelNames[i];
                if (!ent->attachModelNames[i])
                    break;
                if (numModels >= 32)
                    MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 447, 0, "%s", "numModels < DOBJ_MAX_SUBMODELS");
                dobjModels[numModels].model = G_GetModel(modelIndexa);
                if (!dobjModels[numModels].model)
                    MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 449, 0, "%s", "dobjModels[numModels].model");
                if (!ent->attachTagNames[i])
                    MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 450, 0, "%s", "ent->attachTagNames[i]");
                dobjModels[numModels].boneName = ent->attachTagNames[i];
                dobjModels[numModels++].ignoreCollision = (ent->attachIgnoreCollision & (1 << i)) != 0;
            }
            dobj = Com_ServerDObjCreate(dobjModels, numModels, 0, ent->s.number);
            DObjSetHidePartBits(dobj, ent->s.partBits);
            G_UpdateTagInfoOfChildren(ent, 1);
        }
        else
        {
            G_UpdateTagInfoOfChildren(ent, 0);
        }
    }
}

void __cdecl G_SetModel(gentity_s *ent, char *modelName)
{
    int modelIndex; // [esp+0h] [ebp-4h]

    if (*modelName)
    {
        modelIndex = G_ModelIndex(modelName);
        if (modelIndex != (unsigned __int16)modelIndex)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 481, 0, "%s", "modelIndex == (modelNameIndex_t) modelIndex");
        ent->model = modelIndex;
    }
    else
    {
        ent->model = 0;
    }
}

void __cdecl G_OverrideModel(unsigned int modelIndex, char *defaultModelName)
{
    unsigned int v2; // eax
    XModel *v3; // eax
    char *modelName; // [esp+8h] [ebp-4h]

    if (!modelIndex)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 515, 0, "%s", "modelIndex");
    v2 = G_ModelName(modelIndex);
    modelName = SL_ConvertToString(v2);
    if (!*modelName)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 517, 0, "%s", "modelName[0]");
    if (useFastFile->current.enabled)
    {
        DB_ReplaceModel(modelName, defaultModelName);
    }
    else
    {
        v3 = SV_XModelGet(defaultModelName);
        cached_models[modelIndex] = v3;
        Hunk_OverrideDataForFile(5, modelName, v3);
    }
}

int __cdecl G_EntAttach(gentity_s *ent, char *modelName, unsigned int tagName, int ignoreCollision)
{
    int i; // [esp+0h] [ebp-8h]
    int modelIndex; // [esp+4h] [ebp-4h]

    if (!tagName)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 539, 0, "%s", "tagName");
    if (G_EntDetach(ent, modelName, tagName))
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 542, 0, "%s", "!G_EntDetach( ent, modelName, tagName )");
    for (i = 0; ; ++i)
    {
        if (i >= 19)
            return 0;
        if (!ent->attachModelNames[i])
            break;
    }
    modelIndex = G_ModelIndex(modelName);
    if (!modelIndex)
        return 0;
    if (modelIndex != (unsigned __int16)modelIndex)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 553, 0, "%s", "modelIndex == (modelNameIndex_t) modelIndex");
    ent->attachModelNames[i] = modelIndex;
    if (ent->attachTagNames[i])
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 555, 0, "%s", "!ent->attachTagNames[i]");
    Scr_SetString(&ent->attachTagNames[i], tagName);
    if ((ent->attachIgnoreCollision & (1 << i)) != 0)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 557, 0, "%s", "!(ent->attachIgnoreCollision & (1 << i))");
    if (ignoreCollision)
        ent->attachIgnoreCollision |= 1 << i;
    G_DObjUpdate(ent);
    return 1;
}

int __cdecl G_EntDetach(gentity_s *ent, const char *modelName, unsigned int tagName)
{
    unsigned int v4; // edx
    unsigned int modelNameString; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if (!tagName)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 578, 0, "%s", "tagName");
    modelNameString = SL_FindLowercaseString(modelName).prev;
    if (!modelNameString || modelNameString == scr_const._)
        return 0;
    for (i = 0; ; ++i)
    {
        if (i >= 19)
            return 0;
        if (ent->attachTagNames[i] == tagName && G_ModelName(ent->attachModelNames[i]) == modelNameString)
            break;
    }
    if (!ent->attachModelNames[i])
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 592, 0, "%s", "ent->attachModelNames[i]");
    ent->attachModelNames[i] = 0;
    Scr_SetString(&ent->attachTagNames[i], 0);
    while (i < 18)
    {
        ent->attachModelNames[i] = ent->attachModelNames[i + 1];
        ent->attachTagNames[i] = ent->attachTagNames[i + 1];
        if ((ent->attachIgnoreCollision & (1 << (i + 1))) != 0)
            v4 = ent->attachIgnoreCollision | (1 << i);
        else
            v4 = ent->attachIgnoreCollision & ~(1 << i);
        ent->attachIgnoreCollision = v4;
        ++i;
    }
    ent->attachModelNames[i] = 0;
    ent->attachTagNames[i] = 0;
    ent->attachIgnoreCollision &= ~(1 << i);
    G_DObjUpdate(ent);
    return 1;
}

void __cdecl G_EntDetachAll(gentity_s *ent)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 19; ++i)
    {
        ent->attachModelNames[i] = 0;
        Scr_SetString(&ent->attachTagNames[i], 0);
    }
    ent->attachIgnoreCollision = 0;
    G_DObjUpdate(ent);
}

int __cdecl G_EntLinkTo(gentity_s *ent, gentity_s *parent, unsigned int tagName)
{
    if (!G_EntLinkToInternal(ent, parent, tagName))
        return 0;
    G_CalcTagAxis(ent, 0);
    return 1;
}

int __cdecl G_EntLinkToInternal(gentity_s *ent, gentity_s *parent, unsigned int tagName)
{
    char *v4; // eax
    int pm_type; // [esp+0h] [ebp-10h]
    char *tagInfo; // [esp+4h] [ebp-Ch]
    gentity_s *checkEnt; // [esp+8h] [ebp-8h]
    int index; // [esp+Ch] [ebp-4h]

    if (!parent)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 651, 0, "%s", "parent");
    if ((ent->flags & 0x1000) == 0)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 652, 0, "%s", "ent->flags & FL_SUPPORTS_LINKTO");
    G_EntUnlink(ent);
    if (ent->tagInfo)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 656, 0, "%s", "!ent->tagInfo");
    if (tagName)
    {
        if (!SV_DObjExists(parent))
            return 0;
        index = SV_DObjGetBoneIndex(parent, tagName);
        if (index < 0)
            return 0;
    }
    else
    {
        index = -1;
    }
    for (checkEnt = parent; ; checkEnt = checkEnt->tagInfo->parent)
    {
        if (!checkEnt)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 675, 0, "%s", "checkEnt");
        if (checkEnt == ent)
            return 0;
        if (!checkEnt->tagInfo)
            break;
    }
    tagInfo = (char*)MT_Alloc(112, 17);
    *(unsigned int *)tagInfo = (unsigned int)parent;
    *((_WORD *)tagInfo + 4) = 0;
    if (tagName && !SL_IsLowercaseString(tagName))
    {
        v4 = SL_ConvertToString(tagName);
        MyAssertHandler(
            ".\\game_mp\\g_utils_mp.cpp",
            686,
            0,
            "%s\n\t(SL_ConvertToString( tagName )) = %s",
            "(!tagName || SL_IsLowercaseString( tagName ))",
            v4);
    }
    Scr_SetString((unsigned __int16 *)tagInfo + 4, tagName);
    *((unsigned int *)tagInfo + 1) = (unsigned int)parent->tagChildren;
    *((unsigned int *)tagInfo + 3) = index;
    memset((unsigned __int8 *)tagInfo + 16, 0, 0x30u);
    parent->tagChildren = ent;
    ent->tagInfo = (tagInfo_s *)tagInfo;
    memset((unsigned __int8 *)tagInfo + 64, 0, 0x30u);
    if (ent->client)
    {
        pm_type = ent->client->ps.pm_type;
        if (pm_type)
        {
            if (pm_type == PM_DEAD)
                ent->client->ps.pm_type = PM_DEAD_LINKED;
        }
        else
        {
            ent->client->ps.pm_type = PM_NORMAL_LINKED;
        }
    }
    return 1;
}

int __cdecl G_EntLinkToWithOffset(
    gentity_s *ent,
    gentity_s *parent,
    unsigned int tagName,
    const float *originOffset,
    const float *anglesOffset)
{
    tagInfo_s *tagInfo; // [esp+4h] [ebp-4h]

    if (!G_EntLinkToInternal(ent, parent, tagName))
        return 0;
    tagInfo = ent->tagInfo;
    AnglesToAxis(anglesOffset, tagInfo->axis);
    tagInfo->axis[3][0] = *originOffset;
    tagInfo->axis[3][1] = originOffset[1];
    tagInfo->axis[3][2] = originOffset[2];
    return 1;
}

void __cdecl G_EntUnlink(gentity_s *ent)
{
    gclient_s *client; // ecx
    int pm_type; // [esp+0h] [ebp-24h]
    tagInfo_s *tagInfo; // [esp+8h] [ebp-1Ch]
    gentity_s *next; // [esp+Ch] [ebp-18h]
    gentity_s *parent; // [esp+10h] [ebp-14h]
    gentity_s *prev; // [esp+14h] [ebp-10h]
    float viewAngles[3]; // [esp+18h] [ebp-Ch] BYREF

    tagInfo = ent->tagInfo;
    if (tagInfo)
    {
        G_SetOrigin(ent, ent->r.currentOrigin);
        G_SetAngle(ent, ent->r.currentAngles);
        parent = tagInfo->parent;
        if (!tagInfo->parent)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 770, 0, "%s", "parent");
        if (ent->client)
        {
            client = ent->client;
            viewAngles[0] = client->ps.viewangles[0];
            viewAngles[1] = client->ps.viewangles[1];
            viewAngles[2] = 0.0;
            SetClientViewAngle(ent, viewAngles);
            ent->r.currentAngles[0] = 0.0;
            if (parent->classname == scr_const.script_vehicle)
                G_VehUnlinkPlayer(parent, ent);
        }
        prev = 0;
        next = parent->tagChildren;
        while (next != ent)
        {
            if (!next->tagInfo)
                MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 790, 0, "%s", "next->tagInfo");
            prev = next;
            next = next->tagInfo->next;
            if (!next)
                MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 793, 0, "%s", "next");
        }
        if (prev)
            prev->tagInfo->next = tagInfo->next;
        else
            parent->tagChildren = tagInfo->next;
        ent->tagInfo = 0;
        if (ent->client)
        {
            pm_type = ent->client->ps.pm_type;
            if (pm_type == PM_NORMAL_LINKED)
            {
                ent->client->ps.pm_type = PM_NORMAL;
            }
            else if (pm_type == PM_DEAD_LINKED)
            {
                ent->client->ps.pm_type = PM_DEAD;
            }
        }
        Scr_SetString(&tagInfo->name, 0);
        MT_Free((byte*)tagInfo, 112);
    }
}

void __cdecl G_UpdateTagInfo(gentity_s *ent, int bParentHasDObj)
{
    tagInfo_s *tagInfo; // [esp+0h] [ebp-4h]

    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 853, 0, "%s", "tagInfo");
    if (tagInfo->name)
    {
        if (!bParentHasDObj || (tagInfo->index = SV_DObjGetBoneIndex(tagInfo->parent, tagInfo->name), tagInfo->index < 0))
            G_EntUnlink(ent);
    }
    else
    {
        tagInfo->index = -1;
    }
}

void __cdecl G_UpdateTagInfoOfChildren(gentity_s *parent, int bHasDObj)
{
    gentity_s *next; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    for (ent = parent->tagChildren; ent; ent = next)
    {
        next = ent->tagInfo->next;
        G_UpdateTagInfo(ent, bHasDObj);
    }
}

void __cdecl G_CalcTagParentAxis(gentity_s *ent, float (*parentAxis)[3])
{
    float *currentOrigin; // [esp+Ch] [ebp-C4h]
    float v3; // [esp+24h] [ebp-ACh]
    float v4; // [esp+28h] [ebp-A8h]
    float v5; // [esp+2Ch] [ebp-A4h]
    float v6; // [esp+30h] [ebp-A0h]
    float result[3]; // [esp+34h] [ebp-9Ch] BYREF
    float v8; // [esp+40h] [ebp-90h]
    float v9; // [esp+44h] [ebp-8Ch]
    float v10; // [esp+48h] [ebp-88h]
    float v11; // [esp+4Ch] [ebp-84h]
    float v12; // [esp+50h] [ebp-80h]
    float *v13; // [esp+68h] [ebp-68h]
    float *v14; // [esp+6Ch] [ebp-64h]
    tagInfo_s *tagInfo; // [esp+70h] [ebp-60h]
    float tempAxis[4][3]; // [esp+74h] [ebp-5Ch] BYREF
    gentity_s *parent; // [esp+A4h] [ebp-2Ch]
    DObjAnimMat *mat; // [esp+A8h] [ebp-28h]
    float axis[3][3]; // [esp+ACh] [ebp-24h] BYREF

    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 904, 0, "%s", "tagInfo");
    parent = tagInfo->parent;
    if (!parent)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 906, 0, "%s", "parent");
    if (tagInfo->index < 0)
    {
        AnglesToAxis(parent->r.currentAngles, parentAxis);
        currentOrigin = parent->r.currentOrigin;
        (*parentAxis)[9] = parent->r.currentOrigin[0];
        (*parentAxis)[10] = currentOrigin[1];
        (*parentAxis)[11] = currentOrigin[2];
    }
    else
    {
        AnglesToAxis(parent->r.currentAngles, tempAxis);
        v13 = tempAxis[3];
        v14 = parent->r.currentOrigin;
        tempAxis[3][0] = parent->r.currentOrigin[0];
        tempAxis[3][1] = parent->r.currentOrigin[1];
        tempAxis[3][2] = parent->r.currentOrigin[2];
        {
            PROF_SCOPED("G_CalcTagParentAxis");
            G_DObjCalcBone(parent, tagInfo->index);
        }
        mat = &SV_DObjGetMatrixArray(parent)[tagInfo->index];
        if ((COERCE_UNSIGNED_INT(mat->quat[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(mat->quat[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(mat->quat[2]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(mat->quat[3]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                432,
                0,
                "%s",
                "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
        }
        if ((COERCE_UNSIGNED_INT(mat->transWeight) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
        Vec3Scale(mat->quat, mat->transWeight, result);
        v11 = result[0] * mat->quat[0];
        v4 = result[0] * mat->quat[1];
        v9 = result[0] * mat->quat[2];
        v12 = result[0] * mat->quat[3];
        v3 = result[1] * mat->quat[1];
        v10 = result[1] * mat->quat[2];
        v8 = result[1] * mat->quat[3];
        v5 = result[2] * mat->quat[2];
        v6 = result[2] * mat->quat[3];
        axis[0][0] = 1.0 - (v3 + v5);
        axis[0][1] = v4 + v6;
        axis[0][2] = v9 - v8;
        axis[1][0] = v4 - v6;
        axis[1][1] = 1.0 - (v11 + v5);
        axis[1][2] = v10 + v12;
        axis[2][0] = v9 + v8;
        axis[2][1] = v10 - v12;
        axis[2][2] = 1.0 - (v11 + v3);
        MatrixMultiply(axis, *(const mat3x3*)&tempAxis, *(mat3x3*)parentAxis);
        MatrixTransformVector43(mat->trans, tempAxis, &(*parentAxis)[9]);
    }
}

void __cdecl G_CalcTagAxis(gentity_s *ent, int bAnglesOnly)
{
    tagInfo_s *tagInfo; // [esp+8h] [ebp-94h]
    float invParentAxis[4][3]; // [esp+Ch] [ebp-90h] BYREF
    float parentAxis[4][3]; // [esp+3Ch] [ebp-60h] BYREF
    float axis[4][3]; // [esp+6Ch] [ebp-30h] BYREF

    G_CalcTagParentAxis(ent, parentAxis);
    AnglesToAxis(ent->r.currentAngles, axis);
    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 965, 0, "%s", "tagInfo");
    if (bAnglesOnly)
    {
        MatrixTranspose(*(const mat3x3*)&parentAxis, *(mat3x3*)&invParentAxis);
        MatrixMultiply(*(const mat3x3*)&axis, *(const mat3x3*)&invParentAxis, *(mat3x3*)&tagInfo->axis);
    }
    else
    {
        MatrixInverseOrthogonal43(parentAxis, invParentAxis);
        axis[3][0] = ent->r.currentOrigin[0];
        axis[3][1] = ent->r.currentOrigin[1];
        axis[3][2] = ent->r.currentOrigin[2];
        MatrixMultiply43(axis, invParentAxis, tagInfo->axis);
    }
}

void __cdecl G_SetFixedLink(gentity_s *ent, int eAngles)
{
    tagInfo_s *tagInfo; // [esp+1Ch] [ebp-64h]
    float parentAxis[4][3]; // [esp+20h] [ebp-60h] BYREF
    float axis[4][3]; // [esp+50h] [ebp-30h] BYREF

    G_CalcTagParentAxis(ent, parentAxis);
    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 995, 0, "%s", "tagInfo");
    if (eAngles)
    {
        if (eAngles == 1)
        {
            MatrixMultiply43(tagInfo->axis, parentAxis, axis);
            ent->r.currentOrigin[0] = axis[3][0];
            ent->r.currentOrigin[1] = axis[3][1];
            ent->r.currentOrigin[2] = axis[3][2];
            ent->r.currentAngles[1] = vectoyaw(axis[0]);
        }
        else if (eAngles == 2)
        {
            MatrixTransformVector43(tagInfo->axis[3], parentAxis, axis[3]);
            ent->r.currentOrigin[0] = axis[3][0];
            ent->r.currentOrigin[1] = axis[3][1];
            ent->r.currentOrigin[2] = axis[3][2];
        }
    }
    else
    {
        MatrixMultiply43(tagInfo->axis, parentAxis, axis);
        ent->r.currentOrigin[0] = axis[3][0];
        ent->r.currentOrigin[1] = axis[3][1];
        ent->r.currentOrigin[2] = axis[3][2];
        AxisToAngles(*(const mat3x3*)&axis, ent->r.currentAngles);
    }
}

void __cdecl G_GeneralLink(gentity_s *ent)
{
    if (!ent->tagInfo)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1026, 0, "%s", "ent->tagInfo");
    G_SetFixedLink(ent, 0);
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
    ent->s.lerp.pos.trType = TR_INTERPOLATE;
    ent->s.lerp.pos.trDelta[0] = 0.0;
    ent->s.lerp.pos.trDelta[1] = 0.0;
    ent->s.lerp.pos.trDelta[2] = 0.0;
    ent->s.lerp.pos.trTime = 0;
    ent->s.lerp.pos.trDuration = 0;
    ent->s.lerp.apos.trType = TR_INTERPOLATE;
    ent->s.lerp.apos.trDelta[0] = 0.0;
    ent->s.lerp.apos.trDelta[1] = 0.0;
    ent->s.lerp.apos.trDelta[2] = 0.0;
    ent->s.lerp.apos.trTime = 0;
    ent->s.lerp.apos.trDuration = 0;
    SV_LinkEntity(ent);
}

void __cdecl G_SafeDObjFree(gentity_s *ent)
{
    Com_SafeServerDObjFree(ent->s.number);
}

int __cdecl G_DObjUpdateServerTime(gentity_s *ent, int bNotify)
{
    float dtime; // [esp+8h] [ebp-4h]

    dtime = (double)level.frametime * EQUAL_EPSILON;
    return SV_DObjUpdateServerTime(ent, dtime, bNotify);
}

void __cdecl G_DObjCalcPose(gentity_s *ent, int *partBits)
{
    void(__cdecl * controller)(const gentity_s *, int *); // [esp+0h] [ebp-8h]
    DObj_s *obj; // [esp+4h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    iassert(obj);
    if (!SV_DObjCreateSkelForBones(obj, partBits))
    {
        controller = entityHandlers[ent->handler].controller;
        if (controller)
            controller(ent, partBits);
        DObjCalcSkel(obj, partBits);
    }
}

void __cdecl G_DObjCalcBone(const gentity_s *ent, int boneIndex)
{
    void(__cdecl * controller)(const gentity_s *, int *); // [esp+0h] [ebp-18h]
    DObj_s *obj; // [esp+4h] [ebp-14h]
    int partBits[4]; // [esp+8h] [ebp-10h] BYREF

    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1103, 0, "%s", "obj");
    if (!SV_DObjCreateSkelForBone(obj, boneIndex))
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        controller = entityHandlers[ent->handler].controller;
        if (controller)
            controller(ent, partBits);
        DObjCalcSkel(obj, partBits);
    }
}

DObjAnimMat *__cdecl G_DObjGetLocalTagMatrix(gentity_s *ent, unsigned int tagName)
{
    int boneIndex; // [esp+30h] [ebp-8h]

    boneIndex = SV_DObjGetBoneIndex(ent, tagName);
    if (boneIndex < 0)
        return 0;

    {
        PROF_SCOPED("G_DObjGetLocalTagMatrix");
        G_DObjCalcBone(ent, boneIndex);
    }

    return &SV_DObjGetMatrixArray(ent)[boneIndex];
}

int __cdecl G_DObjGetWorldTagMatrix(gentity_s *ent, unsigned int tagName, mat4x3 &tagMat)
{
    float v4; // [esp+1Ch] [ebp-90h]
    float v5; // [esp+20h] [ebp-8Ch]
    float v6; // [esp+24h] [ebp-88h]
    float v7; // [esp+28h] [ebp-84h]
    float result[3]; // [esp+2Ch] [ebp-80h] BYREF
    float v9; // [esp+38h] [ebp-74h]
    float v10; // [esp+3Ch] [ebp-70h]
    float v11; // [esp+40h] [ebp-6Ch]
    float v12; // [esp+44h] [ebp-68h]
    float v13; // [esp+48h] [ebp-64h]
    float *v14; // [esp+4Ch] [ebp-60h]
    float *currentOrigin; // [esp+50h] [ebp-5Ch]
    mat4x3 ent_axis; // [esp+54h] [ebp-58h] BYREF
    DObjAnimMat *mat; // [esp+84h] [ebp-28h]
    float axis[3][3]; // [esp+88h] [ebp-24h] BYREF

    mat = G_DObjGetLocalTagMatrix(ent, tagName);
    if (!mat)
        return 0;
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    v14 = ent_axis[3];
    currentOrigin = ent->r.currentOrigin;
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    
    iassert(!IS_NAN(mat->quat[0]) && !IS_NAN(mat->quat[1]) && !IS_NAN(mat->quat[2]) && !IS_NAN(mat->quat[3]));
    iassert(!IS_NAN(mat->transWeight));

    Vec3Scale(mat->quat, mat->transWeight, result);

    v12 = result[0] * mat->quat[0];
    v5 = result[0] * mat->quat[1];
    v10 = result[0] * mat->quat[2];
    v13 = result[0] * mat->quat[3];
    v4 = result[1] * mat->quat[1];
    v11 = result[1] * mat->quat[2];
    v9 = result[1] * mat->quat[3];
    v6 = result[2] * mat->quat[2];
    v7 = result[2] * mat->quat[3];

    axis[0][0] = 1.0 - (v4 + v6);
    axis[0][1] = v5 + v7;
    axis[0][2] = v10 - v9;
    axis[1][0] = v5 - v7;
    axis[1][1] = 1.0 - (v12 + v6);
    axis[1][2] = v11 + v13;
    axis[2][0] = v10 + v9;
    axis[2][1] = v11 - v13;
    axis[2][2] = 1.0 - (v12 + v4);

    MatrixMultiply(axis, *(const mat3x3*)&ent_axis, *(mat3x3*)&tagMat);
    MatrixTransformVector43(mat->trans, ent_axis, tagMat[3]);

    return 1;
}

int __cdecl G_DObjGetWorldTagPos(gentity_s *ent, unsigned int tagName, float *pos)
{
    float ent_axis[4][3]; // [esp+8h] [ebp-34h] BYREF
    DObjAnimMat *mat; // [esp+38h] [ebp-4h]

    mat = G_DObjGetLocalTagMatrix(ent, tagName);
    if (!mat)
        return 0;
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    MatrixTransformVector43(mat->trans, ent_axis, pos);
    return 1;
}

DObjAnimMat *__cdecl G_DObjGetLocalBoneIndexMatrix(gentity_s *ent, int boneIndex)
{
    {
        PROF_SCOPED("G_DObjGetLocalTagMatrix");
        G_DObjCalcBone(ent, boneIndex);
    }
    return &SV_DObjGetMatrixArray(ent)[boneIndex];
}

void __cdecl G_DObjGetWorldBoneIndexPos(gentity_s *ent, int boneIndex, float *pos)
{
    float ent_axis[4][3]; // [esp+8h] [ebp-34h] BYREF
    DObjAnimMat *mat; // [esp+38h] [ebp-4h]

    mat = G_DObjGetLocalBoneIndexMatrix(ent, boneIndex);
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    MatrixTransformVector43(mat->trans, ent_axis, pos);
}

void __cdecl G_DObjGetWorldBoneIndexMatrix(gentity_s *ent, int boneIndex, float (*tagMat)[3])
{
    float v3; // [esp+1Ch] [ebp-90h]
    float v4; // [esp+20h] [ebp-8Ch]
    float v5; // [esp+24h] [ebp-88h]
    float v6; // [esp+28h] [ebp-84h]
    float result[3]; // [esp+2Ch] [ebp-80h] BYREF
    float v8; // [esp+38h] [ebp-74h]
    float v9; // [esp+3Ch] [ebp-70h]
    float v10; // [esp+40h] [ebp-6Ch]
    float v11; // [esp+44h] [ebp-68h]
    float v12; // [esp+48h] [ebp-64h]
    float *v13; // [esp+4Ch] [ebp-60h]
    float *currentOrigin; // [esp+50h] [ebp-5Ch]
    float ent_axis[4][3]; // [esp+54h] [ebp-58h] BYREF
    DObjAnimMat *mat; // [esp+84h] [ebp-28h]
    float axis[3][3]; // [esp+88h] [ebp-24h] BYREF

    mat = G_DObjGetLocalBoneIndexMatrix(ent, boneIndex);
    AnglesToAxis(ent->r.currentAngles, ent_axis);
    v13 = ent_axis[3];
    currentOrigin = ent->r.currentOrigin;
    ent_axis[3][0] = ent->r.currentOrigin[0];
    ent_axis[3][1] = ent->r.currentOrigin[1];
    ent_axis[3][2] = ent->r.currentOrigin[2];
    if ((COERCE_UNSIGNED_INT(mat->quat[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[2]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[3]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
            432,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    if ((COERCE_UNSIGNED_INT(mat->transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h", 433, 0, "%s", "!IS_NAN(mat->transWeight)");
    Vec3Scale(mat->quat, mat->transWeight, result);
    v11 = result[0] * mat->quat[0];
    v4 = result[0] * mat->quat[1];
    v9 = result[0] * mat->quat[2];
    v12 = result[0] * mat->quat[3];
    v3 = result[1] * mat->quat[1];
    v10 = result[1] * mat->quat[2];
    v8 = result[1] * mat->quat[3];
    v5 = result[2] * mat->quat[2];
    v6 = result[2] * mat->quat[3];
    axis[0][0] = 1.0 - (v3 + v5);
    axis[0][1] = v4 + v6;
    axis[0][2] = v9 - v8;
    axis[1][0] = v4 - v6;
    axis[1][1] = 1.0 - (v11 + v5);
    axis[1][2] = v10 + v12;
    axis[2][0] = v9 + v8;
    axis[2][1] = v10 - v12;
    axis[2][2] = 1.0 - (v11 + v3);
    MatrixMultiply(axis, *(const mat3x3*)&ent_axis, *(mat3x3*)tagMat);
    MatrixTransformVector43(mat->trans, ent_axis, &(*tagMat)[9]);
}

gentity_s *__cdecl G_Find(gentity_s *from, int fieldofs, unsigned __int16 match)
{
    unsigned __int16 s; // [esp+0h] [ebp-4h]
    gentity_s *froma; // [esp+Ch] [ebp+8h]

    if (from)
        froma = from + 1;
    else
        froma = g_entities;
    while (froma < &g_entities[level.num_entities])
    {
        if (froma->r.inuse)
        {
            s = *(_WORD *)((char *)&froma->s.number + fieldofs);
            if (s)
            {
                if (s == match)
                    return froma;
            }
        }
        ++froma;
    }
    return 0;
}

void __cdecl G_InitGentity(gentity_s *e)
{
    e->nextFree = 0;
    e->r.inuse = 1;
    Scr_SetString(&e->classname, scr_const.noclass);
    e->s.number = e - g_entities;

    iassert(!e->r.ownerNum.isDefined());

    e->eventTime = 0;
    e->freeAfterEvent = 0;
}

void __cdecl G_PrintEntities()
{
    char *v0; // [esp+18h] [ebp-8h]
    int entityIndex; // [esp+1Ch] [ebp-4h]

    for (entityIndex = 0; entityIndex < level.num_entities; ++entityIndex)
    {
        if (g_entities[entityIndex].classname)
            v0 = SL_ConvertToString(g_entities[entityIndex].classname);
        else
            v0 = (char *)"";
        Com_Printf(
            15,
            "%4i: '%s', origin: %f %f %f\n",
            entityIndex,
            v0,
            g_entities[entityIndex].r.currentOrigin[0],
            g_entities[entityIndex].r.currentOrigin[1],
            g_entities[entityIndex].r.currentOrigin[2]);
    }
}

gentity_s *__cdecl G_Spawn()
{
    gentity_s *e; // [esp+0h] [ebp-4h]

    e = level.firstFreeEnt;
    if (G_MaySpawnEntity(level.firstFreeEnt))
    {
        level.firstFreeEnt = level.firstFreeEnt->nextFree;
        if (!level.firstFreeEnt)
            level.lastFreeEnt = 0;
        e->nextFree = 0;
    }
    else
    {
        if (level.num_entities == 1022)
        {
            G_PrintEntities();
            Com_Error(ERR_DROP, "G_Spawn: no free entities");
        }
        e = &level.gentities[level.num_entities++];
        SV_LocateGameData(level.gentities, level.num_entities, 628, &level.clients->ps, 12676);
    }
    G_InitGentity(e);
    return e;
}

bool __cdecl G_MaySpawnEntity(gentity_s *e)
{
    if (!e)
        return 0;
    return level.time - e->eventTime >= 500 || level.num_entities >= 1022;
}

gentity_s *__cdecl G_SpawnPlayerClone()
{
    gentity_s *e; // [esp+0h] [ebp-8h]
    int flags; // [esp+4h] [ebp-4h]

    e = &level.gentities[level.currentPlayerClone + 64];
    level.currentPlayerClone = (level.currentPlayerClone + 1) % 8;
    flags = e->s.lerp.eFlags & 2 ^ 2;
    if (e->r.inuse)
        G_FreeEntity(e);
    G_InitGentity(e);
    e->s.lerp.eFlags = flags;
    return e;
}

void __cdecl G_FreeEntityRefs(gentity_s *ed)
{
    gentity_s *other; // [esp+0h] [ebp-10h]
    gentity_s *othera; // [esp+0h] [ebp-10h]
    gclient_s *pClient; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    int ib; // [esp+8h] [ebp-8h]
    int entnum; // [esp+Ch] [ebp-4h]

    entnum = ed->s.number;
    if ((ed->flags & 0x400000) != 0)
    {
        for (i = 0; i < level.num_entities; ++i)
        {
            other = &g_entities[i];
            if (other->r.inuse
                && other->r.ownerNum.isDefined()
                && other->r.ownerNum.entnum() == entnum
                && other->s.eType == 11)
            {
                other->active = 0;
                break;
            }
        }
    }
    if ((ed->flags & 0x100000) != 0)
    {
        for (ia = 0; ia < level.num_entities; ++ia)
        {
            othera = &g_entities[ia];
            if (othera->r.inuse)
            {
                if (othera->s.groundEntityNum == entnum)
                    othera->s.groundEntityNum = 1023;
            }
        }
    }
    if ((ed->flags & 0x200000) != 0)
    {
        for (ib = 0; ib < 64; ++ib)
        {
            if (g_entities[ib].r.inuse)
            {
                pClient = g_entities[ib].client;
                if (!pClient)
                    MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1473, 0, "%s", "pClient");
                if (pClient->ps.cursorHintEntIndex == entnum)
                    pClient->ps.cursorHintEntIndex = 1023;
            }
        }
    }
    if ((ed->flags & 0x800000) != 0)
        Missile_FreeAttractorRefs(ed);
}

void __cdecl G_FreeEntity(gentity_s *ed)
{
    XAnimTree_s *tree; // [esp+0h] [ebp-8h]
    int useCount; // [esp+4h] [ebp-4h]

    G_EntUnlink(ed);
    while (ed->tagChildren)
        G_EntUnlink(ed->tagChildren);
    SV_UnlinkEntity(ed);
    tree = SV_DObjGetTree(ed);
    if (tree)
        XAnimClearTree(tree);
    Com_SafeServerDObjFree(ed->s.number);
    G_FreeEntityRefs(ed);
    if (ed->pTurretInfo)
    {
        if (!ed->pTurretInfo->inuse)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1513, 0, "%s", "ed->pTurretInfo->inuse");
        G_FreeTurret(ed);
        if (ed->pTurretInfo)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1515, 0, "%s", "ed->pTurretInfo == NULL");
    }
    if (ed->scr_vehicle)
    {
        G_VehFreeEntity(ed);
        if (ed->scr_vehicle)
            MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1522, 0, "%s", "ed->scr_vehicle == NULL");
    }
    if (ed->s.eType == 2)
        PlayerCorpse_Free(ed);
    EntHandleDissociate(ed);
    ed->r.ownerNum.setEnt(NULL);
    ed->parent.setEnt(NULL);
    ed->missileTargetEnt.setEnt(NULL);
    if (!ed->r.inuse)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1537, 0, "%s", "ed->r.inuse");
    Scr_FreeEntity(ed);
    if (ed->classname)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1540, 0, "%s", "ed->classname == 0");
    useCount = ed->useCount;
    memset((unsigned __int8 *)ed, 0, sizeof(gentity_s));
    ed->eventTime = level.time;
    if (ed - level.gentities >= 72)
    {
        if (level.lastFreeEnt)
            level.lastFreeEnt->nextFree = ed;
        else
            level.firstFreeEnt = ed;
        level.lastFreeEnt = ed;
        ed->nextFree = 0;
    }
    ed->useCount = useCount + 1;
    if (ed->r.inuse)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1556, 0, "%s", "!ed->r.inuse");
}

void __cdecl G_FreeEntityDelay(gentity_s *ed)
{
    unsigned __int16 hThread; // [esp+0h] [ebp-4h]

    if (!g_scr_data.delete_)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1570, 0, "%s", "g_scr_data.delete_");
    hThread = Scr_ExecEntThread(ed, g_scr_data.delete_, 0);
    Scr_FreeThread(hThread);
}

void __cdecl G_BroadcastEntity(gentity_s *ent)
{
    ent->r.svFlags = 8;
}

void __cdecl G_FreeEntityAfterEvent(gentity_s *ent)
{
    ent->freeAfterEvent = 1;
}

gentity_s *__cdecl G_TempEntity(const float *origin, int event)
{
    float snapped[3]; // [esp+Ch] [ebp-10h] BYREF
    gentity_s *e; // [esp+18h] [ebp-4h]

    e = G_Spawn();
    e->s.eType = event + 17;
    Scr_SetString(&e->classname, scr_const.tempEntity);
    e->eventTime = level.time;
    e->r.eventTime = level.time;
    e->freeAfterEvent = 1;
    snapped[0] = *origin;
    snapped[1] = origin[1];
    snapped[2] = origin[2];
    snapped[0] = (float)(int)snapped[0];
    snapped[1] = (float)(int)snapped[1];
    snapped[2] = (float)(int)snapped[2];
    G_SetOrigin(e, snapped);
    SV_LinkEntity(e);
    return e;
}

void __cdecl G_AddPredictableEvent(gentity_s *ent, unsigned int event, unsigned int eventParm)
{
    if (ent->client)
        BG_AddPredictableEventToPlayerstate(event, eventParm, &ent->client->ps);
}

void __cdecl G_AddEvent(gentity_s *ent, unsigned int event, unsigned int eventParm)
{
    if (!event)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1713, 0, "%s", "event");
    if (event >= 0x100)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1714, 0, "event doesn't index 256\n\t%i not in [0, %i)", event, 256);
    if (eventParm >= 0xFF)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1715, 0, "%s", "eventParm < EVENT_PARM_MAX");
    if (ent->s.eType >= 17)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1716, 0, "%s", "ent->s.eType < ET_EVENTS");
    if (ent->client)
    {
        ent->client->ps.events[ent->client->ps.eventSequence & 3] = event;
        ent->client->ps.eventParms[ent->client->ps.eventSequence & 3] = eventParm;
        ent->client->ps.eventSequence = (unsigned __int8)(ent->client->ps.eventSequence + 1);
    }
    else
    {
        ent->s.events[ent->s.eventSequence & 3] = event;
        ent->s.eventParms[ent->s.eventSequence & 3] = eventParm;
        ent->s.eventSequence = (unsigned __int8)(ent->s.eventSequence + 1);
    }
    ent->eventTime = level.time;
    ent->r.eventTime = level.time;
}

void __cdecl G_PlaySoundAlias(gentity_s *ent, unsigned __int8 index)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1760, 0, "%s", "ent");
    if (index)
        G_AddEvent(ent, 3u, index);
}

int __cdecl G_AnimScriptSound(int client, snd_alias_list_t *aliasList)
{
    unsigned __int8 v2; // al

    v2 = G_SoundAliasIndex((char *)aliasList->aliasName);
    G_PlaySoundAlias(&g_entities[client], v2);
    return 0;
}

void __cdecl G_SetOrigin(gentity_s *ent, const float *origin)
{
    ent->s.lerp.pos.trBase[0] = *origin;
    ent->s.lerp.pos.trBase[1] = origin[1];
    ent->s.lerp.pos.trBase[2] = origin[2];
    ent->s.lerp.pos.trType = TR_STATIONARY;
    ent->s.lerp.pos.trTime = 0;
    ent->s.lerp.pos.trDuration = 0;
    ent->s.lerp.pos.trDelta[0] = 0.0f;
    ent->s.lerp.pos.trDelta[1] = 0.0f;
    ent->s.lerp.pos.trDelta[2] = 0.0f;
    ent->r.currentOrigin[0] = *origin;
    ent->r.currentOrigin[1] = origin[1];
    ent->r.currentOrigin[2] = origin[2];
}

void __cdecl G_SetAngle(gentity_s *ent, const float *angle)
{
    ent->s.lerp.apos.trBase[0] = *angle;
    ent->s.lerp.apos.trBase[1] = angle[1];
    ent->s.lerp.apos.trBase[2] = angle[2];
    ent->s.lerp.apos.trType = TR_STATIONARY;
    ent->s.lerp.apos.trTime = 0;
    ent->s.lerp.apos.trDuration = 0;
    ent->s.lerp.apos.trDelta[0] = 0.0f;
    ent->s.lerp.apos.trDelta[1] = 0.0f;
    ent->s.lerp.apos.trDelta[2] = 0.0f;
    ent->r.currentAngles[0] = *angle;
    ent->r.currentAngles[1] = angle[1];
    ent->r.currentAngles[2] = angle[2];
}

void __cdecl G_SetConstString(unsigned __int16 *to, char *from)
{
    Scr_SetString(to, 0);
    *to = SL_GetString(from, 0);
}

const char *__cdecl G_GetEntityTypeName(const gentity_s *ent)
{
    if (ent->s.eType >= 0x11u)
        MyAssertHandler(".\\game_mp\\g_utils_mp.cpp", 1870, 0, "%s", "(unsigned)ent->s.eType < ET_EVENTS");
    return entityTypeNames[ent->s.eType];
}

int __cdecl G_rand()
{
    return rand();
}

float __cdecl G_flrand(float min, float max)
{
    return (float)G_rand() * (max - min) / 32768.0f + min;
}

int __cdecl G_irand(int min, int max)
{
    return min + (max - min) * G_rand() / 0x8000;
}

float __cdecl G_random()
{
    return (float)G_rand() / 32768.0f;
}

float __cdecl G_crandom()
{
    return G_random() * 2.0f - 1.0f;
}

