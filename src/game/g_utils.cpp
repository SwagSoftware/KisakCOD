#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include <server/sv_public.h>
#include <script/scr_const.h>
#include <script/scr_vm.h>
#include <stringed/stringed_hooks.h>
#include "g_main.h"
#include <server/sv_game.h>
#include "turret.h"
#include "actor_corpse.h"
#include <script/scr_animtree.h>
#include <database/database.h>
#include <cgame/cg_ents.h>
#include <xanim/dobj_utils.h>
#include <script/scr_memorytree.h>
#include "actor_turret.h"
#include "actor_event_listeners.h"

XModel *cached_models[512]{ NULL };

void __cdecl TRACK_g_utils()
{
    track_static_alloc_internal(entityTypeNames, 68, "entityTypeNames", 9);
    track_static_alloc_internal(cached_models, 2048, "cached_models", 9);
}

static bool dumpedOnce = false;
void __cdecl G_DumpConfigStrings(int start, int max)
{
    int v4; // r31
    unsigned int ConfigstringConst; // r3
    const char *v6; // r3

    if (!dumpedOnce)
    {
        v4 = 1;
        for (dumpedOnce = 1; v4 < max; ++v4)
        {
            ConfigstringConst = SV_GetConfigstringConst(v4 + start);
            v6 = SL_ConvertToString(ConfigstringConst);
            Com_Printf(24, "G_FindConfigstringIndex: overflow (%d) [%d] %s\n", start, v4, v6);
        }
    }
}

int __cdecl G_FindConfigstringIndex(const char *name, int start, int max, int create, const char *errormsg)
{
    unsigned int String; // r3
    unsigned int v11; // r26
    int i; // r31
    unsigned int ConfigstringConst; // r3
    const char *v14; // r3
    const char *v16; // r3

    //Profile_Begin(247);
    if (!name || !*name)
    {
        //Profile_EndInternal(0);
        return 0;
    }
    if (start < 1114)
        String = SL_FindString(name);
    else
        String = SL_FindLowercaseString(name);
    v11 = String;
    for (i = 1; i < max; ++i)
    {
        ConfigstringConst = SV_GetConfigstringConst(i + start);
        if (ConfigstringConst == scr_const._)
            break;
        if (ConfigstringConst == v11)
            goto LABEL_16;
    }
    if (!create)
    {
        //Profile_EndInternal(0);
        if (errormsg)
        {
            v14 = va("%s \"%s\" not precached", errormsg, name);
            Scr_Error(v14);
            return 0;
        }
        return 0;
    }
    if (i == max)
    {
        G_DumpConfigStrings(start, max);
        v16 = va("G_FindConfigstringIndex overflow (%d) : %s", start, name);
        Com_Error(ERR_DROP, v16);
    }
    SV_SetConfigstring(i + start, name);
LABEL_16:
    //Profile_EndInternal(0);
    return i;
}

static const char *origErrorMsg = "localized string";
int __cdecl G_LocalizedStringIndex(const char *string)
{
    const char *v3; // r29
    int initializing; // r28
    unsigned int v5; // r30
    int i; // r31
    unsigned int ConfigstringConst; // r3
    const char *v8; // r3
    const char *v9; // r3
    unsigned int v10; // r30
    unsigned int v11; // r3
    const char *v12; // r3

    if (!string)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 170, 0, "%s", "string");
    if (!*string)
        return 0;
    v3 = origErrorMsg;
    initializing = level.initializing;
    if (!level.initializing)
    {
        if (loc_warnings->current.enabled)
        {
            if (!loc_warningsAsErrors->current.enabled)
                v3 = 0;
        }
        else
        {
            initializing = 1;
        }
    }
    //Profile_Begin(247);
    if (*string)
    {
        v5 = SL_FindString(string);
        for (i = 1; i < 1023; ++i)
        {
            ConfigstringConst = SV_GetConfigstringConst(i + 91);
            if (ConfigstringConst == scr_const._)
                break;
            if (ConfigstringConst == v5)
                goto LABEL_22;
        }
        if (!initializing)
        {
            //Profile_EndInternal(0);
            if (v3)
            {
                v8 = va("%s \"%s\" not precached", v3, string);
                Scr_Error(v8);
            }
            i = 0;
            goto LABEL_24;
        }
        if (i == 1023)
        {
            G_DumpConfigStrings(91, 1023);
            v9 = va("G_LocalizedStringIndex: overflow (%d) : %s", 91, string);
            Com_Error(ERR_DROP, v9);
        }
        SV_SetConfigstring(i + 91, string);
    LABEL_22:
        //Profile_EndInternal(0);
        if (i)
            return i;
    }
    else
    {
        //Profile_EndInternal(0);
        i = 0;
    }
    if (initializing)
        return i;
LABEL_24:
    if (!loc_warnings->current.enabled || loc_warningsAsErrors->current.enabled)
        return i;
    //Profile_Begin(247);
    if (*string)
    {
        v10 = SL_FindString(string);
        for (i = 1; i < 1023; ++i)
        {
            v11 = SV_GetConfigstringConst(i + 91);
            if (v11 == scr_const._)
                break;
            if (v11 == v10)
                goto LABEL_34;
        }
        if (i == 1023)
        {
            G_DumpConfigStrings(91, 1023);
            v12 = va("G_LocalizedStringIndex: overflow (%d) : %s", 91, string);
            Com_Error(ERR_DROP, v12);
        }
        SV_SetConfigstring(i + 91, string);
    LABEL_34:
        //Profile_EndInternal(0);
        if (i)
            Com_PrintWarning(24, "WARNING: %s \"%s\" not precached\n", origErrorMsg, string);
        return i;
    }
    //Profile_EndInternal(0);
    return 0;
}

int __cdecl G_MaterialIndex(const char *name)
{
    char *v2; // r11
    int v3; // r10
    char v5[96]; // [sp+50h] [-60h] BYREF

    iassert(name);
    iassert(name[0]);

    v2 = (char*)name;
    do
    {
        v3 = *(unsigned __int8 *)v2;
        (v2++)[v5 - name] = v3;
    } while (v3);
    I_strlwr(v5);
    return G_FindConfigstringIndex(v5, 2583, 128, level.initializing, "material");
}

void __cdecl G_SetModelIndex(int modelIndex, const char *name)
{
    if (modelIndex <= 0 || modelIndex >= 512)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            225,
            0,
            "%s\n\t(modelIndex) = %i",
            "(modelIndex > 0 && modelIndex < (1 << 9))",
            modelIndex);
    cached_models[modelIndex] = SV_XModelGet((char*)name);
    SV_SetConfigstring(modelIndex + 1155, name);
}

int __cdecl G_ModelIndex(const char *name)
{
    unsigned int LowercaseString; // r29
    int v4; // r31
    unsigned int ConfigstringConst; // r3
    const char *v6; // r3

    //Profile_Begin(248);
    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 240, 0, "%s", "name");
    if (!*name)
    {
        //Profile_EndInternal(0);
        return 0;
    }
    LowercaseString = SL_FindLowercaseString(name);
    v4 = 1;
    while (1)
    {
        ConfigstringConst = SV_GetConfigstringConst(v4 + 1155);
        if (ConfigstringConst == scr_const._)
        {
        LABEL_9:
            if (!level.initializing)
            {
                //Profile_EndInternal(0);
                v6 = va("model '%s' not precached", name);
                Scr_Error(v6);
            }
            if (v4 == 512)
                Com_Error(ERR_DROP, "G_ModelIndex: overflow");
            G_SetModelIndex(v4, name);
            goto LABEL_14;
        }
        if (ConfigstringConst == LowercaseString)
            break;
        if (++v4 >= 512)
            goto LABEL_9;
    }
    if (cached_models[v4])
    {
    LABEL_14:
        //Profile_EndInternal(0);
        return v4;
    }
    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 258, 0, "%s", "cached_models[i]");
    //Profile_EndInternal(0);
    return v4;
}

XModel *__cdecl G_GetModel(int index)
{
    if (index <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 282, 0, "%s", "index > 0");
    if (index >= 512)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 283, 0, "%s", "index < MAX_MODELS");
    return cached_models[index];
}

bool __cdecl G_GetModelBounds(int index, float *outMins, float *outMaxs)
{
    const XModel *Model; // r29
    float v8[8][3]; // [sp+50h] [-60h] BYREF

    if (!outMins)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 293, 0, "%s", "outMins");
    if (!outMaxs)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 294, 0, "%s", "outMaxs");
    Model = G_GetModel(index);
    if (!Model)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 296, 0, "%s", "xmodel");
    AxisClear((mat3x3&)v8);
    //return (_cntlzw(XModelGetStaticBounds(Model, (mat3x3&)v8, outMins, outMaxs)) & 0x20) == 0;
    return XModelGetStaticBounds(Model, (mat3x3&)v8, outMins, outMaxs) != 0;
}

int __cdecl G_XModelBad(int index)
{
    const XModel *Model; // r3

    if (!index)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 309, 0, "%s", "index");
    Model = G_GetModel(index);
    return XModelBad(Model);
}

unsigned int __cdecl G_ModelName(unsigned int index)
{
    if (index >= 0x200)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 316, 0, "%s", "(unsigned)index < MAX_MODELS");
    return SV_GetConfigstringConst(index + 1155);
}

void __cdecl G_EntityCentroidWithBounds(const gentity_s *ent, const float *mins, const float *maxs, float *centroid)
{
    double v8; // fp11
    double v9; // fp13
    float v10[4]; // [sp+50h] [-70h] BYREF
    float v11[8][3]; // [sp+60h] [-60h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 323, 0, "%s", "ent");
    if (!centroid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 324, 0, "%s", "centroid");
    AnglesToAxis(ent->r.currentAngles, v11);
    v8 = (float)(mins[2] + maxs[2]);
    v9 = (float)((float)(mins[1] + maxs[1]) * (float)0.5);
    v10[0] = (float)(*mins + *maxs) * (float)0.5;
    v10[1] = v9;
    v10[2] = (float)v8 * (float)0.5;
    MatrixTransformVector(v10, (const mat3x3&)v11, centroid);
    *centroid = ent->r.currentOrigin[0] + *centroid;
    centroid[1] = ent->r.currentOrigin[1] + centroid[1];
    centroid[2] = ent->r.currentOrigin[2] + centroid[2];
}

void __cdecl G_EntityCentroid(const gentity_s *ent, float *centroid)
{
    double v4; // fp10
    double v5; // fp13
    float v6[4]; // [sp+50h] [-60h] BYREF
    float v7[6][3]; // [sp+60h] [-50h] BYREF

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 340, 0, "%s", "ent");
    if (!centroid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 341, 0, "%s", "centroid");
    AnglesToAxis(ent->r.currentAngles, v7);
    v4 = ent->r.mins[2];
    v5 = (float)(ent->r.mins[1] + ent->r.maxs[1]);
    v6[0] = (float)(ent->r.mins[0] + ent->r.maxs[0]) * (float)0.5;
    v6[2] = (float)((float)v4 + ent->r.maxs[2]) * (float)0.5;
    v6[1] = (float)v5 * (float)0.5;
    MatrixTransformVector(v6, (const mat3x3 &)v7, centroid);
    *centroid = ent->r.currentOrigin[0] + *centroid;
    centroid[1] = ent->r.currentOrigin[1] + centroid[1];
    centroid[2] = ent->r.currentOrigin[2] + centroid[2];
}

int __cdecl G_EffectIndex(const char *name)
{
    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 357, 0, "%s", "name");
    if (!I_strncmp(name, "fx/", 3))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            358,
            0,
            "%s\n\t(name) = %s",
            "(I_strncmp( name, \"fx/\", 3 ))",
            name);
    return G_FindConfigstringIndex(name, 2179, 100, level.initializing, "effect");
}

int __cdecl G_ShellShockIndex(const char *name)
{
    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 365, 0, "%s", "name");
    return G_FindConfigstringIndex(name, 2535, 16, 1, 0);
}

unsigned int __cdecl G_SoundAliasIndexTransientAdvance(unsigned __int16 aliasIndex, int offset)
{
    unsigned int v4; // r31

    if (aliasIndex < 0x100u || aliasIndex >= 0x200u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            372,
            0,
            "%s\n\t(aliasIndex) = %i",
            "(aliasIndex >= 256 && aliasIndex < 512)",
            aliasIndex);
    if (offset <= 0 || offset >= 256)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            373,
            0,
            "%s\n\t(offset) = %i",
            "(offset > 0 && offset < 512 - 256)",
            offset);
    v4 = (unsigned __int16)(aliasIndex + offset);
    if (v4 >= 0x200)
        v4 = (unsigned __int16)(v4 - 256);
    if ((unsigned __int16)v4 < 0x100u || (unsigned __int16)v4 >= 0x200u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            379,
            0,
            "%s\n\t(aliasIndex) = %i",
            "(aliasIndex >= 256 && aliasIndex < 512)",
            (unsigned __int16)v4);
    return v4;
}

unsigned int __cdecl G_SoundAliasIndexTransient(const char *name)
{
    unsigned __int16 soundAliasFirst; // r11
    unsigned __int16 soundAliasLast; // r3
    int v5; // r31
    const char *v6; // r3
    unsigned int LowercaseString; // r3
    unsigned int v8; // r30
    unsigned int v9; // r29
    unsigned __int16 v10; // r31
    unsigned __int16 v11; // r11
    int v12; // r31
    unsigned __int16 v13; // r31
    unsigned int v14; // r31
    unsigned __int16 v15; // r31
    unsigned int v16; // r31

    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 392, 0, "%s", "name");
    if (!*name)
        return 0;
    soundAliasFirst = level.soundAliasFirst;
    if (level.soundAliasFirst < 0x100u || level.soundAliasFirst >= 0x200u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            396,
            0,
            "%s\n\t(level.soundAliasFirst) = %i",
            "(level.soundAliasFirst >= 256 && level.soundAliasFirst < 512)",
            level.soundAliasFirst);
        soundAliasFirst = level.soundAliasFirst;
    }
    soundAliasLast = level.soundAliasLast;
    if (level.soundAliasLast < 0x100u || soundAliasFirst >= 0x200u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            397,
            0,
            "%s\n\t(level.soundAliasLast) = %i",
            "(level.soundAliasLast >= 256 && level.soundAliasFirst < 512)",
            level.soundAliasLast);
        soundAliasLast = level.soundAliasLast;
        soundAliasFirst = level.soundAliasFirst;
    }
    if (soundAliasFirst != 256)
    {
        v5 = soundAliasFirst;
        if (soundAliasFirst != (unsigned __int16)G_SoundAliasIndexTransientAdvance(soundAliasLast, 128))
        {
            v6 = va("%i, %i", v5, level.soundAliasLast);
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                398,
                0,
                "%s\n\t%s",
                "level.soundAliasFirst == SOUNDALIAS_PERMANENT_COUNT || level.soundAliasFirst == G_SoundAliasIndexTransientAdvanc"
                "e( level.soundAliasLast, (MAX_SOUNDALIASES - SOUNDALIAS_PERMANENT_COUNT) / 2 )",
                v6);
        }
    }
    LowercaseString = SL_FindLowercaseString(name);
    v8 = level.soundAliasFirst;
    v9 = LowercaseString;
    if (level.soundAliasFirst == level.soundAliasLast)
    {
    LABEL_27:
        v10 = level.soundAliasLast;
        if (level.soundAliasLast < 0x100u || level.soundAliasLast >= 0x200u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                372,
                0,
                "%s\n\t(aliasIndex) = %i",
                "(aliasIndex >= 256 && aliasIndex < 512)",
                level.soundAliasLast);
        v11 = v10 + 128;
        if ((unsigned __int16)(v10 + 128) >= 0x200u)
            v11 = v10 - 128;
        v12 = v11;
        if (v11 < 0x100u || v11 >= 0x200u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                379,
                0,
                "%s\n\t(aliasIndex) = %i",
                "(aliasIndex >= 256 && aliasIndex < 512)",
                v11);
        if (v12 == level.soundAliasFirst)
        {
            v13 = level.soundAliasFirst;
            if (level.soundAliasFirst < 0x100u || level.soundAliasFirst >= 0x200u)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    372,
                    0,
                    "%s\n\t(aliasIndex) = %i",
                    "(aliasIndex >= 256 && aliasIndex < 512)",
                    level.soundAliasFirst);
            v14 = (unsigned __int16)(v13 + 1);
            if (v14 >= 0x200)
                v14 = v14 - 256;
            if ((unsigned __int16)v14 < 0x100u || (unsigned __int16)v14 >= 0x200u)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    379,
                    0,
                    "%s\n\t(aliasIndex) = %i",
                    "(aliasIndex >= 256 && aliasIndex < 512)",
                    (unsigned __int16)v14);
            level.soundAliasFirst = v14;
        }
        if ((unsigned __int16)v8 != level.soundAliasLast)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                414,
                1,
                "%s",
                "aliasIndex == level.soundAliasLast");
        v15 = level.soundAliasLast;
        if (level.soundAliasLast < 0x100u || level.soundAliasLast >= 0x200u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                372,
                0,
                "%s\n\t(aliasIndex) = %i",
                "(aliasIndex >= 256 && aliasIndex < 512)",
                level.soundAliasLast);
        v16 = (unsigned __int16)(v15 + 1);
        if (v16 >= 0x200)
            v16 = v16 - 256;
        if ((unsigned __int16)v16 < 0x100u || (unsigned __int16)v16 >= 0x200u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                379,
                0,
                "%s\n\t(aliasIndex) = %i",
                "(aliasIndex >= 256 && aliasIndex < 512)",
                (unsigned __int16)v16);
        level.soundAliasLast = v16;
        SV_SetConfigstring((unsigned __int16)v8 + 1667, name);
    }
    else
    {
        while (1)
        {
            if (!(_WORD)v8)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 404, 0, "%s", "aliasIndex != 0");
            if (SV_GetConfigstringConst((unsigned __int16)v8 + 1667) == v9)
                break;
            if ((unsigned __int16)v8 < 0x100u || (unsigned __int16)v8 >= 0x200u)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    372,
                    0,
                    "%s\n\t(aliasIndex) = %i",
                    "(aliasIndex >= 256 && aliasIndex < 512)",
                    (unsigned __int16)v8);
            v8 = (unsigned __int16)(v8 + 1);
            if (v8 >= 0x200)
                v8 = (unsigned __int16)(v8 - 256);
            if ((unsigned __int16)v8 < 0x100u || (unsigned __int16)v8 >= 0x200u)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    379,
                    0,
                    "%s\n\t(aliasIndex) = %i",
                    "(aliasIndex >= 256 && aliasIndex < 512)",
                    (unsigned __int16)v8);
            if ((unsigned __int16)v8 == level.soundAliasLast)
                goto LABEL_27;
        }
    }
    return v8;
}

int __cdecl G_SoundAliasIndexPermanent(const char *name)
{
    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 424, 0, "%s", "name");
    return (unsigned __int16)G_FindConfigstringIndex(name, 1667, 256, 1, 0);
}

int __cdecl G_RumbleIndex(const char *name)
{
    if (!name)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 432, 0, "%s", "name");
    return G_FindConfigstringIndex(name, 1115, 32, 1, 0);
}

void __cdecl G_SetClientDemoTime(int time)
{
    level.framenum = time / 50;
    if (time - 50 > 0)
        level.previousTime = time - 50;
    else
        level.previousTime = 0;
    level.time = time;
}

void __cdecl G_SetClientDemoServerSnapTime(int time)
{
    level.snapTime = time;
}

void __cdecl G_ClearDemoEntities()
{
    int v0; // r20
    gentity_s *ent; // r31
    XAnimTree_s *Tree; // r3
    XAnimTree_s *pAnimTree; // r3
    actor_s *actor; // r3
    scr_vehicle_s *scr_vehicle; // r11
    TurretInfo *pTurretInfo; // r11

    if (g_entities[0].r.inuse)
    {
        G_FreeEntities();
        memset(g_entities, 0, 628 * level.num_entities);
    }
    else
    {
        v0 = 0;
        for (ent = level.gentities; v0 < level.num_entities; ++ent)
        {
            if (ent->r.inuse)
            {
                Tree = SV_DObjGetTree(ent);
                if (Tree)
                    XAnimClearTree(Tree);
                Com_SafeServerDObjFree(ent->s.number);
                pAnimTree = ent->pAnimTree;
                if (pAnimTree)
                {
                    Com_XAnimFreeSmallTree(pAnimTree);
                    ent->pAnimTree = 0;
                }
                actor = ent->actor;
                if (actor)
                    Actor_Free(actor);
                scr_vehicle = ent->scr_vehicle;
                if (scr_vehicle)
                {
                    scr_vehicle->entNum = ENTITYNUM_NONE;
                    iassert(!ent->scr_vehicle->idleSndEnt.isDefined());
                    iassert(!ent->scr_vehicle->engineSndEnt.isDefined());
                    ent->scr_vehicle = 0;
                }
                pTurretInfo = ent->pTurretInfo;
                if (pTurretInfo)
                {
                    if (!pTurretInfo->inuse)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 501, 0, "%s", "ent->pTurretInfo->inuse");
                    G_FreeTurret(ent);
                    if (ent->pTurretInfo)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                            503,
                            0,
                            "%s",
                            "ent->pTurretInfo == NULL");
                }
                ent->r.inuse = 0;
            }
            if (ent->actor)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 509, 0, "%s", "ent->actor == NULL");
            if (ent->sentient && !ent->client)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    510,
                    0,
                    "%s",
                    "(ent->sentient == NULL) || (ent->client != NULL)");
            if (ent->scr_vehicle)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 511, 0, "%s", "ent->scr_vehicle == NULL");
            ++v0;
        }
    }
}

void __cdecl G_UpdateDemoEntity(entityState_s *es)
{
    unsigned int number; // r26
    gentity_s *v3; // r31
    int v4; // r25
    int eType; // r11
    sentient_s *v6; // r3
    actor_s *v7; // r3
    scr_vehicle_s *v8; // r11

    if (!es)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 522, 0, "%s", "es");
    number = es->number;
    if (number >= 0x880)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            524,
            0,
            "%s",
            "entnum >= 0 && entnum < MAX_GENTITIES");
    if ((int)number >= level.num_entities)
        level.num_entities = number + 1;
    v3 = &g_entities[number];
    v4 = level.specialIndex[number];
    if (v3->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 532, 0, "%s", "!ent->r.inuse");
    if (v3->actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 533, 0, "%s", "ent->actor == NULL");
    if (v3->sentient && !v3->client)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            534,
            0,
            "%s",
            "(ent->sentient == NULL) || (ent->client != NULL)");
    if (v3->scr_vehicle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 535, 0, "%s", "ent->scr_vehicle == NULL");
    eType = v3->s.eType;
    v3->r.inuse = 1;
    switch (eType)
    {
    case 11:
        if (v4 >= 64)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                565,
                0,
                "%s",
                "specialIndex >= 0 && specialIndex < MAX_VEHICLES");
        v8 = &level.vehicles[v4];
        v3->scr_vehicle = v8;
        v8->entNum = number;
        break;
    case 14:
        if (v4 >= 32)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                541,
                0,
                "%s",
                "specialIndex >= 0 && specialIndex < MAX_ACTORS");
        v6 = Sentient_Alloc();
        v3->sentient = v6;
        v6->ent = v3;
        v7 = &level.actors[v4];
        v3->actor = v7;
        memset(v7, 0, sizeof(actor_s));
        v3->actor->inuse = 1;
        v3->actor->ent = v3;
        v3->actor->sentient = v3->sentient;
        if (!v3->actor)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 550, 0, "%s", "ent->actor");
        if (v3->actor->sentient != v3->sentient)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                551,
                0,
                "%s",
                "ent->actor->sentient == ent->sentient");
        if (v3->actor->ent != v3)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 552, 0, "%s", "ent->actor->ent == ent");
        if (!v3->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 553, 0, "%s", "ent->sentient");
        if (v3->sentient->ent != v3)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 554, 0, "%s", "ent->sentient->ent == ent");
        if (v3->client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 555, 0, "%s", "ent->client == NULL");
        if (v3->scr_vehicle)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 556, 0, "%s", "ent->scr_vehicle == NULL");
        break;
    case 16:
        if (v4 < 32 || v4 >= 48)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                560,
                0,
                "%s",
                "specialIndex >= MAX_ACTORS && specialIndex < MAX_ACTORS + MAX_ACTOR_CORPSES");
        g_scr_data.dogAnim.weapons[4 * v4 + 18].func = number;
        break;
    }
}

unsigned int __cdecl G_GetEntAnimTreeId(int entnum)
{
    gentity_s *v2; // r31
    int eType; // r11
    unsigned int v5; // r31
    const XAnim_s *Anims; // r31

    if (entnum < 0 || entnum >= level.num_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            579,
            0,
            "%s",
            "entnum >= 0 && entnum < level.num_entities");
    v2 = &g_entities[entnum];
    eType = v2->s.eType;
    if (eType != 14)
    {
        if (eType != 16)
            goto LABEL_18;
        if (!v2->s.lerp.u.actor.species)
        {
            if (v2->pAnimTree)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 586, 0, "%s", "ent->pAnimTree == NULL");
            return G_GetActorCorpseIndex(v2);
        }
    }
    if (!v2->s.lerp.u.actor.species)
    {
        if (v2->pAnimTree)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 594, 0, "%s", "ent->pAnimTree == NULL");
        if (!v2->actor)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 595, 0, "%s", "ent->actor");
        v5 = v2->actor - level.actors;
        if (v5 >= 0x20)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 597, 0, "%s", "id >= 0 && id < MAX_ACTORS");
        return v5;
    }
LABEL_18:
    if (!v2->pAnimTree)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 602, 0, "%s", "ent->pAnimTree != NULL");
    Anims = XAnimGetAnims(v2->pAnimTree);
    if (!Anims)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 604, 0, "%s", "anims");
    return Scr_GetAnimsIndex(Anims);
}

XAnimTree_s *__cdecl G_GetEntAnimTreeForId(int entnum, unsigned int id)
{
    gentity_s *v4; // r30
    int eType; // r11
    const XAnimTree_s *pAnimTree; // r3
    const XAnim_s *Anims; // r31
    XAnim_s *v9; // r31
    XAnimTree_s *SmallTree; // r3

    if (entnum < 0 || entnum >= level.num_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            616,
            0,
            "%s",
            "entnum >= 0 && entnum < level.num_entities");
    v4 = &g_entities[entnum];
    eType = v4->s.eType;
    if (eType != 14)
    {
        if (eType != 16)
            goto LABEL_14;
        if (!v4->s.lerp.u.actor.species)
        {
            if (g_scr_data.actorCorpseInfo[id].entnum != entnum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    623,
                    0,
                    "%s",
                    "g_scr_data.actorCorpseInfo[id].entnum == entnum");
            return G_GetActorCorpseIndexAnimTree(id);
        }
    }
    if (!v4->s.lerp.u.actor.species)
    {
        if (id >= 0x20)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 630, 0, "%s", "id >= 0 && id < MAX_ACTORS");
        return G_GetActorAnimTree(&level.actors[id]);
    }
LABEL_14:
    pAnimTree = v4->pAnimTree;
    if (pAnimTree)
    {
        Anims = XAnimGetAnims(pAnimTree);
        if (!Anims)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 638, 0, "%s", "anims");
        if (Scr_GetAnimsIndex(Anims) != id)
        {
            XAnimClearTree(v4->pAnimTree);
            Com_XAnimFreeSmallTree(v4->pAnimTree);
            v4->pAnimTree = 0;
        }
    }
    if (!v4->pAnimTree)
    {
        v9 = Scr_GetAnims(id);
        if (!v9)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 649, 0, "%s", "anims");
        SmallTree = Com_XAnimCreateSmallTree(v9);
        v4->pAnimTree = SmallTree;
        if (!SmallTree)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 651, 0, "%s", "ent->pAnimTree");
    }
    return v4->pAnimTree;
}

void __cdecl G_ShutdownClientDemo()
{
    int v0; // r29
    int num_entities; // r11
    XAnimTree_s **p_pAnimTree; // r31

    v0 = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        p_pAnimTree = &g_entities[0].pAnimTree;
        do
        {
            if (*p_pAnimTree)
            {
                XAnimClearTree(*p_pAnimTree);
                Com_XAnimFreeSmallTree(*p_pAnimTree);
                num_entities = level.num_entities;
                *p_pAnimTree = 0;
            }
            ++v0;
            p_pAnimTree += 157;
        } while (v0 < num_entities);
    }
}

XAnimTree_s *__cdecl G_GetEntAnimTree(gentity_s *ent)
{
    int eType; // r11

    if (ent->s.lerp.u.actor.species)
        return ent->pAnimTree;
    eType = ent->s.eType;
    if (eType == 14)
        return G_GetActorAnimTree(ent->actor);
    if (eType != 16)
        return ent->pAnimTree;
    return G_GetActorCorpseAnimTree(ent);
}

void __cdecl G_CheckDObjUpdate(gentity_s *ent)
{
    const DObj_s *ServerDObj; // r3
    int model; // r29
    XAnimTree_s *pAnimTree; // r3
    int eType; // r11
    XModel *v6; // r29
    int v7; // r27
    XModel **v8; // r28
    unsigned __int16 *attachTagNames; // r26
    int v10; // r29
    unsigned __int16 v11; // [sp+50h] [-190h] BYREF
    unsigned __int16 v12; // [sp+52h] [-18Eh] BYREF
    XAnimTree_s *v13; // [sp+54h] [-18Ch] BYREF
    DObjModel_s v14; // [sp+60h] [-180h] BYREF
    char v15; // [sp+6Ch] [-174h] BYREF

    ServerDObj = Com_GetServerDObj(ent->s.number);
    model = ent->model;
    if (ent->model)
    {
        DObjGetCreateParms(ServerDObj, &v14, &v11, &v13, &v12);
        if (ent->s.lerp.u.actor.species)
        {
            pAnimTree = ent->pAnimTree;
        }
        else
        {
            eType = ent->s.eType;
            if (eType == 14)
            {
                pAnimTree = G_GetActorAnimTree(ent->actor);
            }
            else if (eType == 16)
            {
                pAnimTree = G_GetActorCorpseAnimTree(ent);
            }
            else
            {
                pAnimTree = ent->pAnimTree;
            }
        }
        if (v13 != pAnimTree)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                807,
                0,
                "%s",
                "tree == G_GetEntAnimTree( ent )");
        v6 = G_GetModel(model);
        if (!v6)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 812, 0, "%s", "model");
        if (v14.model != v6)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                815,
                0,
                "%s",
                "dobjModels[numModels].model == model");
        if (v14.boneName)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                816,
                0,
                "%s",
                "!dobjModels[numModels].boneName");
        if (v14.ignoreCollision)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                817,
                0,
                "%s",
                "!dobjModels[numModels].ignoreCollision");
        v7 = 1;
        v8 = (XModel **)&v15;
        attachTagNames = ent->attachTagNames;
        do
        {
            v10 = *(attachTagNames - 31);
            if (!*(attachTagNames - 31))
                break;
            if (v7 >= 32)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    826,
                    0,
                    "%s",
                    "numModels < DOBJ_MAX_SUBMODELS");
            if (v7 >= v11)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 827, 0, "%s", "numModels < modelCount");
            if (*(v8 - 1) != G_GetModel(v10))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    828,
                    0,
                    "%s",
                    "dobjModels[numModels].model == G_GetModel( modelIndex )");
            if (!*(v8 - 1))
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 829, 0, "%s", "dobjModels[numModels].model");
            if (!*attachTagNames)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 830, 0, "%s", "ent->attachTagNames[i]");
            if (*(_WORD *)v8 && *(unsigned __int16 *)v8 != *attachTagNames)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    831,
                    0,
                    "%s",
                    "!dobjModels[numModels].boneName || (dobjModels[numModels].boneName == ent->attachTagNames[i] )");
            //if (*((_BYTE *)v8 + 2) != ((_cntlzw((1 << (v7 - 1)) & ent->attachIgnoreCollision) & 0x20) == 0))
            //    MyAssertHandler(
            //        "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            //        832,
            //        0,
            //        "%s",
            //        "dobjModels[numModels].ignoreCollision == ((ent->attachIgnoreCollision & (1 << i)) != 0)");
            ++v7;
            ++attachTagNames;
            v8 += 2;
        } while (v7 - 1 < 31);
        if (v7 != v11)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 836, 0, "%s", "numModels == modelCount");
    }
    else if (ServerDObj)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 801, 0, "%s", "!obj");
    }
}

void __cdecl G_SetModel(gentity_s *ent, const char *modelName)
{
    int v3; // r3
    unsigned __int16 v4; // r31

    if (*modelName)
    {
        v3 = G_ModelIndex(modelName);
        v4 = v3;
        if (v3 != (unsigned __int16)v3)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                858,
                0,
                "%s",
                "modelIndex == (modelNameIndex_t) modelIndex");
        ent->model = v4;
    }
    else
    {
        ent->model = *(unsigned __int8 *)modelName;
    }
}

// attributes: thunk
void __cdecl G_ReplaceModel_FastFile(const char *originalName, const char *replacementName)
{
    DB_ReplaceModel(originalName, replacementName);
}

void __cdecl G_OverrideModel(unsigned int modelindex, const char *defaultModelName)
{
    int v4; // r4
    const char *v5; // r7
    unsigned int ConfigstringConst; // r3
    const char *v7; // r29

    if (modelindex)
    {
        if (modelindex < 0x200)
            goto LABEL_6;
        v4 = 316;
        v5 = "(unsigned)index < MAX_MODELS";
    }
    else
    {
        v4 = 893;
        v5 = "modelindex";
    }
    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", v4, 0, "%s", v5);
LABEL_6:
    ConfigstringConst = SV_GetConfigstringConst(modelindex + 1155);
    v7 = SL_ConvertToString(ConfigstringConst);
    if (!*v7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 895, 0, "%s", "modelName[0]");
    DB_ReplaceModel(v7, defaultModelName);
}

void __cdecl G_PrecacheDefaultModels()
{
    G_ModelIndex("defaultactor");
    G_PrecacheDefaultVehicle();
}

int __cdecl G_EntIsLinkedTo(gentity_s *ent, gentity_s *parent)
{
    tagInfo_s *tagInfo; // r11
    int result; // r3

    tagInfo = ent->tagInfo;
    if (!tagInfo)
        return 0;
    result = 1;
    if (tagInfo->parent != parent)
        return 0;
    return result;
}

void __cdecl G_UpdateViewAngleClamp(gclient_s *client, const float *worldAnglesCenter)
{
    double v4; // fp0
    double v5; // fp13
    double v6; // fp0
    double v7; // fp0
    double v8; // fp13
    double v9; // fp0

    if (!client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1501, 0, "%s", "client");
    if (!worldAnglesCenter)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1502, 0, "%s", "worldAnglesCenter");
    v4 = client->linkAnglesMaxClamp[0];
    v5 = (float)((float)(client->linkAnglesMaxClamp[0] - client->linkAnglesMinClamp[0]) * (float)0.5);
    client->ps.viewAngleClampRange[0] = (float)(client->linkAnglesMaxClamp[0] - client->linkAnglesMinClamp[0])
        * (float)0.5;
    v6 = (float)((float)v4 + *worldAnglesCenter);
    client->ps.viewAngleClampBase[0] = v6;
    client->ps.viewAngleClampBase[0] = AngleNormalize360((float)((float)v6 - (float)v5));
    v7 = client->linkAnglesMaxClamp[1];
    v8 = (float)((float)(client->linkAnglesMaxClamp[1] - client->linkAnglesMinClamp[1]) * (float)0.5);
    client->ps.viewAngleClampRange[1] = (float)(client->linkAnglesMaxClamp[1] - client->linkAnglesMinClamp[1])
        * (float)0.5;
    v9 = (float)(worldAnglesCenter[1] + (float)v7);
    client->ps.viewAngleClampBase[1] = v9;
    client->ps.viewAngleClampBase[1] = AngleNormalize360((float)((float)v9 - (float)v8));
}

void __cdecl G_UpdateGroundTilt(gclient_s *client)
{
    int groundTiltEntNum; // r10
    gentity_s *v2; // r11

    groundTiltEntNum = client->groundTiltEntNum;
    if (groundTiltEntNum == ENTITYNUM_NONE)
    {
        client->ps.groundTiltAngles[0] = 0.0;
        client->ps.groundTiltAngles[1] = 0.0;
        client->ps.groundTiltAngles[2] = 0.0;
    }
    else
    {
        v2 = &g_entities[groundTiltEntNum];
        if (v2->r.inuse)
        {
            client->ps.groundTiltAngles[0] = v2->r.currentAngles[0];
            client->ps.groundTiltAngles[1] = v2->r.currentAngles[1];
            client->ps.groundTiltAngles[2] = v2->r.currentAngles[2];
        }
    }
}

bool __cdecl G_SlideMove(
    double deltaT,
    float *origin,
    float *velocity,
    float *mins,
    const float *maxs,
    const float *gravity,
    unsigned __int8 passEntityNum,
    int clipMask,
    int a9)
{
    double v10; // fp27
    int v11; // r18
    double v17; // fp0
    int v18; // r26
    signed int v19; // r19
    float *v20; // r24
    double v21; // fp0
    double fraction; // fp0
    double v23; // fp12
    double v24; // fp11
    double v25; // fp9
    double v26; // fp8
    double v27; // fp0
    int v28; // r30
    double v29; // fp13
    double v30; // fp12
    float *v31; // r11
    double v32; // fp11
    double v33; // fp9
    int v34; // r28
    float *v35; // r11
    double v36; // fp8
    double v37; // fp9
    int v38; // r27
    double v39; // fp10
    float *v40; // r30
    int v41; // r11
    double v42; // fp10
    float *v43; // r10
    double v44; // fp9
    double v45; // fp8
    double v48; // fp12
    double v49; // fp13
    double v50; // fp10
    double v51; // fp11
    double v52; // fp11
    double v53; // fp12
    double v54; // fp0
    bool result; // r3
    float v56; // [sp+50h] [-160h] BYREF
    float v57; // [sp+54h] [-15Ch]
    float v58; // [sp+58h] [-158h]
    float v59; // [sp+60h] [-150h] BYREF
    float v60; // [sp+64h] [-14Ch]
    float v61; // [sp+68h] [-148h]
    float v62; // [sp+70h] [-140h] BYREF
    float v63; // [sp+74h] [-13Ch]
    float v64; // [sp+78h] [-138h]
    float v65; // [sp+80h] [-130h] BYREF
    float v66; // [sp+84h] [-12Ch]
    float v67; // [sp+88h] [-128h]
    float v68; // [sp+90h] [-120h] BYREF
    float v69; // [sp+94h] [-11Ch]
    float v70; // [sp+98h] [-118h]
    trace_t v71; // [sp+A0h] [-110h] BYREF
    float v72; // [sp+D0h] [-E0h] BYREF
    char v73; // [sp+D4h] [-DCh] BYREF
    _BYTE v74[12]; // [sp+D8h] [-D8h] BYREF
    char v75; // [sp+E4h] [-CCh] BYREF

    v10 = deltaT;
    v11 = passEntityNum;
    v56 = *mins;
    v17 = mins[2];
    v57 = mins[1];
    v58 = v17;
    if (passEntityNum)
    {
        v58 = -(float)((float)((float)deltaT * (float)800.0) - (float)v17);
        mins[2] = (float)((float)v17 - (float)((float)((float)deltaT * (float)800.0) - (float)v17)) * (float)0.5;
    }
    Vec3NormalizeTo(mins, &v72);
    v18 = 1;
    v19 = 0;
    v20 = (float *)&v75;
    do
    {
        v21 = (float)((float)(mins[1] * (float)v10) + velocity[1]);
        v65 = (float)(*mins * (float)v10) + *velocity;
        v67 = (float)(mins[2] * (float)v10) + velocity[2];
        v66 = v21;
        G_TraceCapsule(&v71, velocity, maxs, gravity, &v65, clipMask, a9);
        if (v71.allsolid)
        {
            result = 1;
            mins[2] = 0.0;
            return result;
        }
        fraction = v71.fraction;
        if (v71.fraction > 0.0)
        {
            v23 = velocity[1];
            v24 = velocity[2];
            v25 = (float)(v66 - velocity[1]);
            v26 = (float)(v67 - velocity[2]);
            *velocity = (float)((float)(v65 - *velocity) * v71.fraction) + *velocity;
            velocity[1] = (float)((float)v25 * (float)fraction) + (float)v23;
            velocity[2] = (float)((float)v26 * (float)fraction) + (float)v24;
        }
        if (fraction == 1.0)
            break;
        v10 = (float)-(float)((float)((float)fraction * (float)v10) - (float)v10);
        if (v18 >= 5)
            goto LABEL_36;
        v27 = v71.normal[2];
        v28 = 0;
        v29 = v71.normal[1];
        v30 = v71.normal[0];
        if (v18 <= 0)
            goto LABEL_15;
        v31 = (float *)v74;
        while ((float)((float)(*v31 * v71.normal[2])
            + (float)((float)(*(v31 - 1) * v71.normal[1]) + (float)(*(v31 - 2) * v71.normal[0]))) <= 0.99000001)
        {
            ++v28;
            v31 += 3;
            if (v28 >= v18)
                goto LABEL_15;
        }
        PM_ClipVelocity(mins, v71.normal, mins);
        v30 = v71.normal[0];
        v29 = v71.normal[1];
        v27 = v71.normal[2];
        v32 = (float)(mins[1] + v71.normal[1]);
        v33 = (float)(mins[2] + v71.normal[2]);
        *mins = *mins + v71.normal[0];
        mins[1] = v32;
        mins[2] = v33;
        if (v28 >= v18)
        {
        LABEL_15:
            ++v18;
            *(v20 - 2) = v30;
            *(v20 - 1) = v29;
            v34 = 0;
            *v20 = v27;
            v20 += 3;
            if (v18 > 0)
            {
                v35 = (float *)&v73;
                while ((float)((float)(*v35 * mins[1]) + (float)((float)(v35[1] * mins[2]) + (float)(*(v35 - 1) * *mins))) >= 0.1)
                {
                    ++v34;
                    v35 += 3;
                    if (v34 >= v18)
                        goto LABEL_31;
                }
                PM_ClipVelocity(mins, &v72 + 3 * v34, &v59);
                PM_ClipVelocity(&v56, &v72 + 3 * v34, &v62);
                v36 = v61;
                v37 = v60;
                v38 = 0;
                v39 = v59;
                v40 = &v72;
                while (1)
                {
                    if (v38 != v34
                        && (float)((float)((float)v36 * v40[2]) + (float)((float)(v40[1] * (float)v37) + (float)(*v40 * (float)v39))) < 0.1)
                    {
                        PM_ClipVelocity(&v59, v40, &v59);
                        PM_ClipVelocity(&v62, v40, &v62);
                        v36 = v61;
                        v37 = v60;
                        v39 = v59;
                        if ((float)((float)(*(&v72 + 3 * v34) * v59)
                            + (float)((float)(*(&v72 + 3 * v34 + 1) * v60) + (float)(*(&v72 + 3 * v34 + 2) * v61))) < 0.0)
                            break;
                    }
                LABEL_29:
                    ++v38;
                    v40 += 3;
                    if (v38 >= v18)
                    {
                        v56 = v62;
                        v57 = v63;
                        v54 = v64;
                        *mins = v39;
                        mins[1] = v37;
                        mins[2] = v36;
                        v58 = v54;
                        goto LABEL_31;
                    }
                }
                Vec3Cross(&v72 + 3 * v34, v40, &v68);
                v41 = 0;
                v42 = mins[2];
                v43 = (float *)v74;
                v44 = mins[1];
                v45 = *mins;

                {
                    float _FP5 = -sqrtf((float)((float)(v68 * v68) + (float)((float)(v70 * v70) + (float)(v69 * v69))));
                    //__asm { fsel      f12, f5, f31, f12 }
                    float _FP12 = (_FP5 > 0.0f) ? _FP5 : 0.0f;
                    v48 = (float)((float)1.0 / (float)_FP12);
                }

                v49 = (float)(v70 * (float)v48);
                v69 = v69 * (float)v48;
                v68 = v68 * (float)v48;
                v50 = (float)((float)v42 * (float)(v70 * (float)v48));
                v51 = (float)((float)((float)v45 * v68) + (float)((float)((float)v44 * v69) + (float)v50));
                v37 = (float)(v69 * (float)((float)((float)v45 * v68) + (float)((float)((float)v44 * v69) + (float)v50)));
                v60 = v37;
                v39 = (float)(v68 * (float)v51);
                v59 = v68 * (float)v51;
                v36 = (float)((float)(v70 * (float)v48) * (float)v51);
                v61 = (float)(v70 * (float)v48) * (float)v51;
                v52 = (float)((float)((float)(v70 * (float)v48) * v58) + (float)((float)(v69 * v57) + (float)(v68 * v56)));
                v63 = v69 * (float)((float)((float)(v70 * (float)v48) * v58) + (float)((float)(v69 * v57) + (float)(v68 * v56)));
                v53 = (float)(v68
                    * (float)((float)((float)(v70 * (float)v48) * v58) + (float)((float)(v69 * v57) + (float)(v68 * v56))));
                v70 = v49;
                v62 = v53;
                v64 = (float)v49 * (float)v52;
                while (v41 == v34
                    || v41 == v38
                    || (float)((float)(*v43 * (float)v36)
                        + (float)((float)(*(v43 - 1) * (float)v37) + (float)(*(v43 - 2) * (float)v39))) >= 0.1)
                {
                    ++v41;
                    v43 += 3;
                    if (v41 >= v18)
                        goto LABEL_29;
                }
            LABEL_36:
                *mins = 0.0;
                result = 1;
                mins[1] = 0.0;
                mins[2] = 0.0;
                return result;
            }
        }
    LABEL_31:
        ++v19;
    } while (v19 < 4);
    if (v11)
    {
        *mins = v56;
        mins[1] = v57;
        mins[2] = v58;
    }
    //return (_cntlzw(v19) & 0x20) == 0;
    return v19 != 0;
}

void __cdecl G_StepSlideMove(
    double deltaT,
    float *origin,
    float *velocity,
    float *mins,
    const float *maxs,
    const float *gravity,
    unsigned __int8 passEntityNum,
    int clipMask,
    int a9)
{
    double v17; // fp29
    double v18; // fp28
    double v19; // fp30
    float *v20; // r3
    double v21; // fp1
    double v22; // fp13
    double v23; // fp11
    double v24; // fp10
    double v25; // fp8
    double fraction; // fp0
    double v27; // fp31
    double v28; // fp13
    double v29; // fp0
    double v30; // fp12
    double v31; // fp11
    double v32; // fp9
    double v33; // fp8
    float v34; // [sp+50h] [-D0h] BYREF
    float v35; // [sp+54h] [-CCh]
    float v36; // [sp+58h] [-C8h]
    float v37; // [sp+60h] [-C0h] BYREF
    float v38; // [sp+64h] [-BCh]
    float v39; // [sp+68h] [-B8h]
    float v40; // [sp+70h] [-B0h] BYREF
    float v41; // [sp+74h] [-ACh]
    float v42; // [sp+78h] [-A8h]
    trace_t v43[2]; // [sp+80h] [-A0h] BYREF

    v34 = *velocity;
    v35 = velocity[1];
    v17 = *mins;
    v18 = mins[1];
    v19 = mins[2];
    v36 = velocity[2];
    if (G_SlideMove(deltaT, origin, velocity, mins, maxs, gravity, passEntityNum, clipMask, a9))
    {
        v37 = v34;
        v38 = v35;
        v39 = v36 - (float)18.0;
        G_TraceCapsule(v43, &v34, maxs, gravity, &v37, clipMask, a9);
        if (v19 <= 0.0 || v43[0].fraction != 1.0 && v43[0].normal[2] >= 0.69999999)
        {
            v40 = v34;
            v41 = v35;
            v42 = v36 + (float)18.0;
            G_TraceCapsule(v43, &v34, maxs, gravity, &v40, clipMask, a9);
            if (!v43[0].startsolid)
            {
                v21 = deltaT;
                v22 = v34;
                v23 = v36;
                v24 = (float)(v40 - v34);
                v25 = (float)(v42 - v36);
                fraction = v43[0].fraction;
                velocity[1] = (float)((float)(v41 - v35) * v43[0].fraction) + v35;
                *velocity = (float)((float)v24 * (float)fraction) + (float)v22;
                v27 = (float)((float)((float)v25 * (float)fraction) + (float)v23);
                velocity[2] = (float)((float)v25 * (float)fraction) + (float)v23;
                *mins = v17;
                mins[1] = v18;
                mins[2] = v19;
                G_SlideMove(v21, v20, velocity, mins, maxs, gravity, passEntityNum, clipMask, a9);
                v28 = velocity[2];
                v37 = *velocity;
                v38 = velocity[1];
                v39 = (float)(v36 - (float)v27) + (float)v28;
                G_TraceCapsule(v43, velocity, maxs, gravity, &v37, clipMask, a9);
                v29 = v43[0].fraction;
                if (!v43[0].startsolid)
                {
                    v30 = velocity[1];
                    v31 = velocity[2];
                    v32 = (float)(v38 - velocity[1]);
                    v33 = (float)(v39 - velocity[2]);
                    *velocity = (float)((float)(v37 - *velocity) * v43[0].fraction) + *velocity;
                    velocity[1] = (float)((float)v32 * (float)v29) + (float)v30;
                    velocity[2] = (float)((float)v33 * (float)v29) + (float)v31;
                }
                if (v29 < 1.0)
                    PM_ClipVelocity(mins, v43[0].normal, mins);
            }
        }
    }
}

void __cdecl G_SafeDObjFree(gentity_s *ent)
{
    if (ent->s.number == level.cachedTagMat.entnum)
        level.cachedTagMat.entnum = ENTITYNUM_NONE;
    if (ent->s.number == level.cachedEntTargetTagMat.entnum)
        level.cachedEntTargetTagMat.entnum = ENTITYNUM_NONE;
    Com_SafeServerDObjFree(ent->s.number);
}

int __cdecl G_DObjUpdateServerTime(gentity_s *ent, int bNotify)
{
    ent->flags &= ~0x40000u;
    return SV_DObjUpdateServerTime(ent, 0.050000001, bNotify);
}

void __cdecl G_DObjCalcPose(gentity_s *ent, int *partBits)
{
    DObj_s *ServerDObj; // r29
    void(__cdecl * controller)(const gentity_s *, int *); // r11

    ServerDObj = Com_GetServerDObj(ent->s.number);
    if (!ServerDObj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1940, 0, "%s", "obj");
    if (!SV_DObjCreateSkelForBones(ServerDObj, partBits))
    {
        controller = entityHandlers[ent->handler].controller;
        if (controller)
            controller(ent, partBits);
        DObjCalcSkel(ServerDObj, partBits);
    }
}

void __cdecl G_DObjCalcBone(const gentity_s *ent, int boneIndex)
{
    DObj_s *ServerDObj; // r31
    void(__cdecl * controller)(const gentity_s *, int *); // r11
    int v6[12]; // [sp+50h] [-30h] BYREF

    ServerDObj = Com_GetServerDObj(ent->s.number);
    if (!ServerDObj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1963, 0, "%s", "obj");
    if (!SV_DObjCreateSkelForBone(ServerDObj, boneIndex))
    {
        DObjGetHierarchyBits(ServerDObj, boneIndex, v6);
        controller = entityHandlers[ent->handler].controller;
        if (controller)
            controller(ent, v6);
        DObjCalcSkel(ServerDObj, v6);
    }
}

DObjAnimMat *__cdecl G_DObjGetLocalBoneIndexMatrix(const gentity_s *ent, int boneIndex)
{
    //Profile_Begin(318);
    G_DObjCalcBone(ent, boneIndex);
    //Profile_EndInternal(0);
    return &SV_DObjGetMatrixArray(ent)[boneIndex];
}

void __cdecl G_DObjGetWorldBoneIndexMatrix(const gentity_s *ent, int boneIndex, float (*tagMat)[3])
{
    DObjAnimMat *v6; // r30
    float v7[4][3]; // [sp+50h] [-80h] BYREF
    float v8[20]; // [sp+80h] [-50h] BYREF

    //Profile_Begin(318);
    G_DObjCalcBone(ent, boneIndex);
    //Profile_EndInternal(0);
    v6 = &SV_DObjGetMatrixArray(ent)[boneIndex];
    AnglesToAxis(ent->r.currentAngles, (float (*)[3])v8);
    v8[9] = ent->r.currentOrigin[0];
    v8[10] = ent->r.currentOrigin[1];
    v8[11] = ent->r.currentOrigin[2];
    LocalConvertQuatToMat(v6, v7);
    MatrixMultiply((const mat3x3&)v7, (const mat3x3&)v8, (mat3x3&)tagMat);
    MatrixTransformVector43(v6->trans, (const mat4x3&)v8, &(*tagMat)[9]);
}

void __cdecl G_DObjGetWorldBoneIndexPos(const gentity_s *ent, int boneIndex, float *pos)
{
    DObjAnimMat *MatrixArray; // r28
    float v7[24]; // [sp+50h] [-60h] BYREF

    //Profile_Begin(318);
    G_DObjCalcBone(ent, boneIndex);
    //Profile_EndInternal(0);
    MatrixArray = SV_DObjGetMatrixArray(ent);
    AnglesToAxis(ent->r.currentAngles, (float (*)[3])v7);
    v7[9] = ent->r.currentOrigin[0];
    v7[10] = ent->r.currentOrigin[1];
    v7[11] = ent->r.currentOrigin[2];
    MatrixTransformVector43(MatrixArray[boneIndex].trans, (const mat4x3&)v7, pos);
}

DObjAnimMat *__cdecl G_DObjGetLocalTagMatrix(const gentity_s *ent, unsigned int tagName)
{
    int BoneIndex; // r30

    BoneIndex = SV_DObjGetBoneIndex(ent, tagName);
    if (BoneIndex < 0)
        return 0;
    //Profile_Begin(318);
    G_DObjCalcBone(ent, BoneIndex);
    //Profile_EndInternal(0);
    return &SV_DObjGetMatrixArray(ent)[BoneIndex];
}

int __cdecl G_DObjGetWorldTagMatrix(const gentity_s *ent, unsigned int tagName, float (*tagMat)[3])
{
    int BoneIndex; // r4

    BoneIndex = SV_DObjGetBoneIndex(ent, tagName);
    if (BoneIndex < 0)
        return 0;
    G_DObjGetWorldBoneIndexMatrix(ent, BoneIndex, tagMat);
    return 1;
}

int __cdecl G_DObjGetWorldTagPos(const gentity_s *ent, unsigned int tagName, float *pos)
{
    int BoneIndex; // r30
    DObjAnimMat *MatrixArray; // r29
    float v8[24]; // [sp+50h] [-60h] BYREF

    BoneIndex = SV_DObjGetBoneIndex(ent, tagName);
    if (BoneIndex < 0)
        return 0;
    //Profile_Begin(318);
    G_DObjCalcBone(ent, BoneIndex);
    //Profile_EndInternal(0);
    MatrixArray = SV_DObjGetMatrixArray(ent);
    AnglesToAxis(ent->r.currentAngles, (float (*)[3])v8);
    v8[9] = ent->r.currentOrigin[0];
    v8[10] = ent->r.currentOrigin[1];
    v8[11] = ent->r.currentOrigin[2];
    MatrixTransformVector43(MatrixArray[BoneIndex].trans, (const mat4x3&)v8, pos);
    return 1;
}

void __cdecl G_DObjGetWorldTagPos_CheckTagExists(const gentity_s *ent, unsigned int tagName, float *pos)
{
    const DObj_s *ServerDObj; // r3
    const DObj_s *v6; // r30
    const char *Name; // r30
    int number; // r31
    const char *v9; // r3
    int v10; // r31
    const char *v11; // r3

    if (!G_DObjGetWorldTagPos(ent, tagName, pos))
    {
        ServerDObj = Com_GetServerDObj(ent->s.number);
        v6 = ServerDObj;
        if (ServerDObj && DObjGetNumModels(ServerDObj) && DObjGetName(v6))
        {
            Name = DObjGetName(v6);
            number = ent->s.number;
            v9 = SL_ConvertToString(tagName);
            Com_Error(ERR_DROP, "Missing tag [%s] on entity [%d] (%s)\n", v9, number, Name);
        }
        else
        {
            v10 = ent->s.number;
            v11 = SL_ConvertToString(tagName);
            Com_Error(ERR_DROP, "Missing tag [%s] on entity [%d]\n", v11, v10);
        }
    }
}

gentity_s *__cdecl G_Find(gentity_s *from, int fieldofs, unsigned __int16 match)
{
    gentity_s *result; // r3
    gentity_s *v4; // r9
    unsigned __int8 *i; // r11

    if (from)
        result = from + 1;
    else
        result = g_entities;
    v4 = &g_entities[level.num_entities];
    if (result >= v4)
        return 0;
    for (i = (unsigned char*)&result->s.eType + fieldofs; !i[168 - fieldofs] || !*(_WORD *)i || *(unsigned __int16 *)i != match; i += 628)
    {
        if (++result >= v4)
            return 0;
    }
    return result;
}

void __cdecl G_InitGentity(gentity_s *e)
{
    e->nextFree = 0;
    iassert(!e->r.inuse);
    e->r.inuse = 1;
    Scr_SetString(&e->classname, scr_const.noclass);
    e->s.number = (unsigned __int16)(e - g_entities);
    iassert(e->s.number == e - g_entities);
    iassert(!e->r.ownerNum.isDefined());
    e->r.eventType = 0;
    e->r.eventTime = 0;
    e->angleLerpRate = 540.0;
}

void __cdecl G_PrintEntities()
{
    int v0; // r30
    unsigned __int16 *p_model; // r31
    unsigned int v2; // r11
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29
    const char *v6; // r24
    const char *EntityTypeName; // r3
    double v8; // fp31
    double v9; // fp30
    double v10; // fp29
    const char *v11; // r3

    v0 = 0;
    if (level.num_entities > 0)
    {
        p_model = &g_entities[0].model;
        do
        {
            if (*((_BYTE *)p_model - 112))
            {
                v2 = p_model[2];
                if (scr_const.script_model == v2 && *p_model)
                {
                    G_GetModel(*p_model);
                    v3 = *((float *)p_model - 12);
                    v4 = *((float *)p_model - 13);
                    v5 = *((float *)p_model - 14);
                    v6 = SL_ConvertToStringSafe(p_model[2]);
                    EntityTypeName = BG_GetEntityTypeName(*((unsigned __int8 *)p_model - 280));
                    Com_Printf(
                        15,
                        "%4i: Type: %s, Class: %s, model '%s', origin: %6.1f %6.1f %6.1f\n",
                        v0,
                        EntityTypeName,
                        v6,
                        (const char *)HIDWORD(v5),
                        v5,
                        v4,
                        v3);
                }
                else
                {
                    v8 = *((float *)p_model - 12);
                    v9 = *((float *)p_model - 13);
                    v10 = *((float *)p_model - 14);
                    SL_ConvertToStringSafe(v2);
                    v11 = BG_GetEntityTypeName(*((unsigned __int8 *)p_model - 280));
                    Com_Printf(
                        15,
                        "%4i: Type: %s, Class: %s, origin: %6.1f %6.1f %6.1f\n",
                        v0,
                        v11,
                        (const char *)HIDWORD(v10),
                        v10,
                        v9,
                        v8);
                }
            }
            ++v0;
            p_model += 314;
        } while (v0 < level.num_entities);
    }
}

gentity_s *__cdecl G_Spawn()
{
    gentity_s *e; // r30

    e = level.firstFreeEnt;
    if (level.firstFreeEnt)
    {
        level.firstFreeEnt = level.firstFreeEnt->nextFree;
        if (!level.firstFreeEnt)
            level.lastFreeEnt = 0;
        e->nextFree = 0;
    }
    else
    {
        if (level.num_entities == ENTITYNUM_WORLD)
        {
            G_PrintEntities();
            Scr_Error("G_Spawn: no free entities");
            Com_Error(ERR_DROP, "G_Spawn: no free entities");
        }
        e = &level.gentities[level.num_entities++];
        //SV_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_s), &level.clients->ps, 46104);
        SV_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_s), &level.clients->ps, sizeof(playerState_s));
    }
    G_InitGentity(e);
    return e;
}

void __cdecl G_FreeEntityRefs(gentity_s *ed)
{
    int number; // r29
    int num_entities; // r10
    unsigned __int16 *p_groundEntityNum; // r11
    gclient_s *client; // r31

    number = ed->s.number;
    if ((ed->flags & 0x100000) != 0 && level.num_entities > 0)
    {
        num_entities = level.num_entities;
        p_groundEntityNum = &g_entities[0].s.groundEntityNum;
        do
        {
            if (*((_BYTE *)p_groundEntityNum + 46) && *p_groundEntityNum == number)
                *p_groundEntityNum = ENTITYNUM_NONE;
            --num_entities;
            p_groundEntityNum += 314;
        } while (num_entities);
    }
    if ((ed->flags & 0x400000) != 0 && g_entities[0].r.inuse)
    {
        client = g_entities[0].client;
        if (!g_entities[0].client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2338, 0, "%s", "pClient");
        if (client->ps.cursorHintEntIndex == number)
            client->ps.cursorHintEntIndex = ENTITYNUM_NONE;
    }
    G_FreeVehicleRefs(ed);
    if ((ed->flags & 0x1000000) != 0)
        Missile_FreeAttractorRefs(ed);
    if ((ed->flags & 0x2000000) != 0)
        Targ_Remove(ed);
}

void __cdecl G_FreeAllEntityRefs()
{
    actor_s *actors; // r11
    int i; // r31
    EntHandle *droppedWeaponCue; // r31

    actors = level.actors;
    if (level.actors)
    {
        for (i = 0; i < 32; ++i)
        {
            if (actors[i].inuse)
            {
                Actor_ClearPileUp(&actors[i]);
                actors = level.actors;
            }
        }
    }
    droppedWeaponCue = level.droppedWeaponCue;
    do
        droppedWeaponCue++->setEnt(0);
    while ((int)droppedWeaponCue < (int)&level.droppedWeaponCue[32]);
    Targ_RemoveAll();
}

void __cdecl G_FreeEntityDelay(gentity_s *ed)
{
    unsigned __int16 hThread; // [esp+0h] [ebp-4h]

    iassert(g_scr_data.delete_);
    hThread = Scr_ExecEntThread(ed, g_scr_data.delete_, 0);
    Scr_FreeThread(hThread);
    //int delete; // r4
    //unsigned __int16 v3; // r3
    //
    //delete = g_scr_data.delete_;
    //if (!g_scr_data.delete_)
    //{
    //    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2529, 0, "%s", "g_scr_data.delete_");
    //    delete = g_scr_data.delete_;
    //}
    //v3 = Scr_ExecEntThread(ed, delete, 0);
    //Scr_FreeThread(v3);
}

void __cdecl G_BroadcastEntity(gentity_s *ent)
{
    ;
}

void __cdecl G_FreeEntityAfterEvent(gentity_s *ent)
{
    ent->r.eventType |= 1u;
}

int __cdecl G_SaveFreeEntities(unsigned __int8 *buf)
{
    gentity_s *firstFreeEnt; // r9
    int result; // r3
    unsigned __int8 *v4; // r11

    if (buf)
    {
        *(unsigned int *)buf = (unsigned int)level.firstFreeEnt;
        *((unsigned int *)buf + 1) = (unsigned int)level.lastFreeEnt;
    }
    firstFreeEnt = level.firstFreeEnt;
    result = 8;
    if (level.firstFreeEnt)
    {
        v4 = buf + 8;
        do
        {
            if (buf)
            {
                *v4 = HIBYTE(firstFreeEnt->nextFree);
                v4[1] = BYTE1(firstFreeEnt->nextFree);
                v4[2] = BYTE2(firstFreeEnt->nextFree);
                v4[3] = (unsigned __int8)firstFreeEnt->nextFree;
            }
            firstFreeEnt = firstFreeEnt->nextFree;
            result += 4;
            v4 += 4;
        } while (firstFreeEnt);
    }
    return result;
}

void __cdecl G_LoadFreeEntities(unsigned __int8 *buf)
{
    _BYTE *v2; // r11
    bool v3; // cr58
    unsigned __int8 *v4; // r9
    unsigned __int8 v5; // r10

    if (!buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2608, 0, "%s", "buf");
    v2 = *(_BYTE **)buf;
    v3 = *(unsigned int *)buf == 0;
    level.firstFreeEnt = *(gentity_s **)buf;
    level.lastFreeEnt = (gentity_s *)*((unsigned int *)buf + 1);
    if (!v3)
    {
        v4 = buf + 8;
        do
        {
            v2[624] = *v4;
            v2[625] = v4[1];
            v2[626] = v4[2];
            v5 = v4[3];
            v4 += 4;
            v2[627] = v5;
            v2 = (_BYTE *)*((unsigned int *)v2 + 156);
        } while (v2);
    }
}

void __cdecl G_AddPredictableEvent(gentity_s *ent, int event, unsigned int eventParm)
{
    gclient_s *client; // r5

    client = ent->client;
    if (client)
        BG_AddPredictableEventToPlayerstate(event, eventParm, &client->ps);
}

void __cdecl G_AddEvent(gentity_s *ent, unsigned int event, unsigned int eventParm)
{
    unsigned __int8 v3; // r30
    gclient_s *client; // r11

    v3 = event;
    if (event)
    {
        if (event >= 0x100)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                2690,
                0,
                "event doesn't index 256\n\t%i not in [0, %i)",
                event,
                256);
    }
    else
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2689, 0, "%s", "event");
    }
    if (ent->s.eType >= 0x11u)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2692, 0, "%s", "ent->s.eType < ET_EVENTS");
    client = ent->client;
    if (client)
    {
        client->ps.events[client->ps.eventSequence & 3] = v3;
        ent->client->ps.eventParms[ent->client->ps.eventSequence++ & 3] = eventParm;
    }
    else
    {
        ent->s.events[ent->s.eventSequence & 3] = v3;
        ent->s.eventParms[ent->s.eventSequence++ & 3] = eventParm;
    }
    ent->r.eventTime = level.time;
}

void __cdecl G_RegisterSoundWait(gentity_s *ent, unsigned __int16 index, unsigned int notifyString, int stoppable)
{
    const char *v7; // r20
    double v8; // fp31
    double v9; // fp30
    double v10; // fp29
    const char *v11; // r3
    const char *v12; // r3
    int time; // r11
    unsigned __int16 v14[8]; // [sp+80h] [-890h] BYREF
    char v15[1024]; // [sp+90h] [-880h] BYREF
    char v16[1024]; // [sp+490h] [-480h] BYREF

    v14[0] = 0;
    Scr_SetString(v14, notifyString);
    if (ent->snd_wait.notifyString)
    {
        Scr_Notify(ent, ent->snd_wait.notifyString, 0);
        if (!ent->snd_wait.stoppable || !stoppable)
        {
            SV_GetConfigstring(ent->snd_wait.index + 1667, v16, 1024);
            SV_GetConfigstring(index + 1667, v15, 1024);
            Scr_SetString(v14, 0);
            if (ent->targetname)
                SL_ConvertToString(ent->targetname);
            SL_ConvertToString(v14[0]);
            v7 = SL_ConvertToString(ent->snd_wait.notifyString);
            v8 = ent->r.currentOrigin[2];
            v9 = ent->r.currentOrigin[1];
            v10 = ent->r.currentOrigin[0];
            v11 = SL_ConvertToString(ent->classname);
            v12 = va(
                "issued a second playsound with notification string before the first finished on entity %i classname %s tar"
                "getname %s location %g %g %g old string %s alias %s at time %i new string %s alias %s at time %i\n",
                ent->s.number,
                v11,
                HIDWORD(v10),
                HIDWORD(v9),
                HIDWORD(v8),
                LODWORD(v8),
                v7);
            Scr_Error(v12);
        }
    }
    Scr_SetString(&ent->snd_wait.notifyString, v14[0]);
    Scr_SetString(v14, 0);
    ent->snd_wait.index = index;
    time = level.time;
    ent->snd_wait.stoppable = stoppable;
    ent->snd_wait.duration = -1;
    ent->snd_wait.basetime = time;
}

void __cdecl G_PlaySoundAliasWithNotify(
    gentity_s *ent,
    unsigned __int16 index,
    unsigned int notifyString,
    int stoppable,
    unsigned int event,
    unsigned int notifyevent)
{
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2773, 0, "%s", "ent");
    if (event != 3 && event != 4)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            2774,
            0,
            "%s",
            "event == EV_SOUND_ALIAS || event == EV_SOUND_ALIAS_AS_MASTER");
    if (notifyevent != 42 && notifyevent != 43)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            2775,
            0,
            "%s",
            "notifyevent == EV_SOUND_ALIAS_NOTIFY || notifyevent == EV_SOUND_ALIAS_NOTIFY_AS_MASTER");
    if (index)
    {
        if (notifyString)
        {
            G_AddEvent(ent, notifyevent, index);
            G_RegisterSoundWait(ent, index, notifyString, stoppable);
        }
        else
        {
            G_AddEvent(ent, event, index);
        }
    }
}

void __cdecl G_PlaySoundAlias(gentity_s *ent, unsigned __int16 index)
{
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2773, 0, "%s", "ent");
    if (index)
        G_AddEvent(ent, 3u, index);
}

void __cdecl G_SetOrigin(gentity_s *ent, float *origin)
{
    if ((COERCE_UNSIGNED_INT(*origin) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            2814,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
    }
    ent->s.lerp.pos.trBase[0] = *origin;
    ent->s.lerp.pos.trBase[1] = origin[1];
    ent->s.lerp.pos.trBase[2] = origin[2];
    ent->s.lerp.pos.trType = TR_STATIONARY;
    ent->s.lerp.pos.trTime = 0;
    ent->s.lerp.pos.trDuration = 0;
    ent->s.lerp.pos.trDelta[0] = 0.0;
    ent->s.lerp.pos.trDelta[1] = 0.0;
    ent->s.lerp.pos.trDelta[2] = 0.0;
    ent->r.currentOrigin[0] = *origin;
    ent->r.currentOrigin[1] = origin[1];
    ent->r.currentOrigin[2] = origin[2];
}

void __cdecl G_SetAngle(gentity_s *ent, float *angle)
{
    ent->s.lerp.apos.trBase[0] = *angle;
    ent->s.lerp.apos.trBase[1] = angle[1];
    ent->s.lerp.apos.trBase[2] = angle[2];
    ent->s.lerp.apos.trType = TR_STATIONARY;
    ent->s.lerp.apos.trTime = 0;
    ent->s.lerp.apos.trDuration = 0;
    ent->s.lerp.apos.trDelta[0] = 0.0;
    ent->s.lerp.apos.trDelta[1] = 0.0;
    ent->s.lerp.apos.trDelta[2] = 0.0;
    ent->r.currentAngles[0] = *angle;
    ent->r.currentAngles[1] = angle[1];
    ent->r.currentAngles[2] = angle[2];
}

void __cdecl G_SetConstString(unsigned __int16 *to, const char *from)
{
    Scr_SetString(to, 0);
    *to = SL_GetString(from, 0);
}

const char *__cdecl G_GetEntityTypeName(const gentity_s *ent)
{
    if (ent->s.eType >= 0x11u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            2863,
            0,
            "%s",
            "(unsigned)ent->s.eType < ET_EVENTS");
    return *(const char **)((char *)entityTypeNames + __ROL4__(ent->s.eType, 2));
}

unsigned int holdrand;

void __cdecl G_SetPM_MPViewer(bool setting)
{
    level.mpviewer = setting;
}

void __cdecl G_srand(unsigned int seed)
{
    holdrand = seed;
}

unsigned int __cdecl G_GetRandomSeed()
{
    return holdrand;
}

//unsigned int __cdecl G_rand()
//{
//    unsigned int result; // r3
//
//    result = (214013 * holdrand + 2531011) >> 17;
//    holdrand = 214013 * holdrand + 2531011;
//    return result;
//}

unsigned int __cdecl G_rand()
{
    holdrand = 214013 * holdrand + 2531011;
    return holdrand >> 17;
}

//float __cdecl G_flrand(double min, double max)
//{
//    __int64 v2; // r11
//    double v3; // fp1
//
//    HIDWORD(v2) = _xri_a;
//    holdrand = 214013 * holdrand + 2531011;
//    LODWORD(v2) = holdrand >> 17;
//    v3 = (float)((float)((float)((float)v2 * (float)((float)max - (float)min)) * (float)0.000030517578) + (float)min);
//    return *((float *)&v3 + 1);
//}

float __cdecl G_flrand(float min, float max)
{
    return (float)G_rand() * (max - min) / 32768.0f + min;
}

//int __cdecl G_irand(int min, int max)
//{
//    __int128 v2; // r11
//
//    DWORD1(v2) = 214013;
//    holdrand = 214013 * holdrand + 2531011;
//    DWORD2(v2) = max - min;
//    LODWORD(v2) = holdrand >> 17;
//    return ((__int64)(v2 * *(_QWORD *)((char *)&v2 + 4)) >> 15) + min;
//}

int __cdecl G_irand(int min, int max)
{
    return ((G_rand() * (__int64)(max - min)) >> 15) + min;
}

//float __cdecl G_random()
//{
//    __int64 v0; // r11
//    double v1; // fp1
//
//    HIDWORD(v0) = _xri_a;
//    holdrand = 214013 * holdrand + 2531011;
//    LODWORD(v0) = holdrand >> 17;
//    v1 = (float)((float)v0 * (float)0.000030517578);
//    return *((float *)&v1 + 1);
//}

float __cdecl G_random()
{
    return (float)G_rand() / 32768.0f;
}

//float __cdecl G_crandom()
//{
//    __int64 v0; // r11
//    double v1; // fp1
//
//    HIDWORD(v0) = _xri_a;
//    holdrand = 214013 * holdrand + 2531011;
//    LODWORD(v0) = holdrand >> 17;
//    v1 = (float)((float)((float)v0 * (float)0.000061035156) - (float)1.0);
//    return *((float *)&v1 + 1);
//}

float __cdecl G_crandom()
{
    return G_random() * 2.0 - 1.0;
}

void __cdecl G_CalcTagParentAxis(gentity_s *ent, float (*parentAxis)[3])
{
    tagInfo_s *tagInfo; // r29
    const gentity_s *parent; // r31
    const float *currentAngles; // r3
    DObjAnimMat *v7; // r31
    float *v8; // r29
    float v9[4][3]; // [sp+60h] [-90h] BYREF
    float v10[24]; // [sp+90h] [-60h] BYREF

    //Profile_Begin(317);
    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1375, 0, "%s", "tagInfo");
    parent = tagInfo->parent;
    if (!tagInfo->parent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1377, 0, "%s", "parent");
    currentAngles = parent->r.currentAngles;
    if (tagInfo->index < 0)
    {
        AnglesToAxis(currentAngles, parentAxis);
        v8 = &(*parentAxis)[9];
        (*parentAxis)[9] = parent->r.currentOrigin[0];
        (*parentAxis)[10] = parent->r.currentOrigin[1];
        (*parentAxis)[11] = parent->r.currentOrigin[2];
    }
    else
    {
        AnglesToAxis(currentAngles, (float (*)[3])v10);
        v10[9] = parent->r.currentOrigin[0];
        v10[10] = parent->r.currentOrigin[1];
        v10[11] = parent->r.currentOrigin[2];
        G_DObjCalcBone(parent, tagInfo->index);
        v7 = &SV_DObjGetMatrixArray(parent)[tagInfo->index];
        LocalConvertQuatToMat(v7, v9);
        MatrixMultiply((const mat3x3&)v9, (const mat3x3&)v10, (mat3x3&)parentAxis);
        v8 = &(*parentAxis)[9];
        MatrixTransformVector43(v7->trans, (const mat4x3&)v10, &(*parentAxis)[9]);
    }
    if ((COERCE_UNSIGNED_INT((*parentAxis)[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            1399,
            0,
            "%s",
            "!IS_NAN((parentAxis[0])[0]) && !IS_NAN((parentAxis[0])[1]) && !IS_NAN((parentAxis[0])[2])");
    }
    if ((COERCE_UNSIGNED_INT((*parentAxis)[3]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[4]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[5]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            1400,
            0,
            "%s",
            "!IS_NAN((parentAxis[1])[0]) && !IS_NAN((parentAxis[1])[1]) && !IS_NAN((parentAxis[1])[2])");
    }
    if ((COERCE_UNSIGNED_INT((*parentAxis)[6]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[7]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[8]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            1401,
            0,
            "%s",
            "!IS_NAN((parentAxis[2])[0]) && !IS_NAN((parentAxis[2])[1]) && !IS_NAN((parentAxis[2])[2])");
    }
    if ((COERCE_UNSIGNED_INT(*v8) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[10]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*parentAxis)[11]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            1402,
            0,
            "%s",
            "!IS_NAN((parentAxis[3])[0]) && !IS_NAN((parentAxis[3])[1]) && !IS_NAN((parentAxis[3])[2])");
    }
    //Profile_EndInternal(0);
}

void __cdecl G_CalcTagParentRelAxis(gentity_s *ent, float (*parentRelAxis)[3])
{
    tagInfo_s *tagInfo; // r30
    float v5[6][3]; // [sp+50h] [-50h] BYREF

    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1419, 0, "%s", "tagInfo");
    G_CalcTagParentAxis(ent, v5);
    MatrixMultiply43(tagInfo->parentInvAxis, (const mat4x3&)v5, (mat4x3&)parentRelAxis);
}

void __cdecl G_CalcTagAxis(gentity_s *ent, int bAnglesOnly)
{
    tagInfo_s *tagInfo; // r30
    float v5[12]; // [sp+50h] [-B0h] BYREF
    float v6[4][3]; // [sp+80h] [-80h] BYREF
    float v7[6][3]; // [sp+B0h] [-50h] BYREF

    G_CalcTagParentAxis(ent, v7);
    AnglesToAxis(ent->r.currentAngles, (float (*)[3])v5);
    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1443, 0, "%s", "tagInfo");
    if (bAnglesOnly)
    {
        MatrixTranspose((const mat3x3&)v7, (mat3x3&)v6);
        MatrixMultiply((const mat3x3&)v5, (const mat3x3&)v6, (mat3x3&)tagInfo->axis);
    }
    else
    {
        MatrixInverseOrthogonal43((const mat4x3&)v7, v6);
        v5[9] = ent->r.currentOrigin[0];
        v5[10] = ent->r.currentOrigin[1];
        v5[11] = ent->r.currentOrigin[2];
        MatrixMultiply43((const mat4x3&)v5, v6, tagInfo->axis);
    }
}

void __cdecl G_SetFixedLink(gentity_s *ent, unsigned int eAngles)
{
    tagInfo_s *tagInfo; // r30
    double v5; // fp13
    double v6; // fp12
    double v7; // fp13
    double v8; // fp12
    double v9; // fp13
    double v10; // fp12
    float v11[3][3]; // [sp+50h] [-80h] BYREF
    float v12; // [sp+74h] [-5Ch] BYREF
    float v13; // [sp+78h] [-58h]
    float v14; // [sp+7Ch] [-54h]
    float v15[6][3]; // [sp+80h] [-50h] BYREF

    G_CalcTagParentAxis(ent, v15);
    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1473, 0, "%s", "tagInfo");
    if (eAngles)
    {
        if (eAngles == 1)
        {
            MatrixMultiply43(tagInfo->axis, (const mat4x3&)v15, (mat4x3&)v11);
            v7 = v13;
            v8 = v14;
            ent->r.currentOrigin[0] = v12;
            ent->r.currentOrigin[1] = v7;
            ent->r.currentOrigin[2] = v8;
            ent->r.currentAngles[1] = vectoyaw(v11[0]);
        }
        else if (eAngles < 3)
        {
            MatrixTransformVector43(tagInfo->axis[3], (const mat4x3&)v15, &v12);
            v5 = v13;
            v6 = v14;
            ent->r.currentOrigin[0] = v12;
            ent->r.currentOrigin[1] = v5;
            ent->r.currentOrigin[2] = v6;
        }
    }
    else
    {
        MatrixMultiply43(tagInfo->axis, (const mat4x3&)v15, (mat4x3&)v11);
        v9 = v13;
        v10 = v14;
        ent->r.currentOrigin[0] = v12;
        ent->r.currentOrigin[1] = v9;
        ent->r.currentOrigin[2] = v10;
        AxisToAngles(v11, ent->r.currentAngles);
    }
}

void __cdecl G_SetPlayerFixedLink(gentity_s *ent)
{
    gclient_s *client; // r28
    float *v3; // r5
    float *v4; // r31
    long double v5; // fp2
    float *v6; // r29
    int v7; // r30
    double v8; // fp31
    double v9; // fp12
    long double v10; // fp2
    float *v11; // r29
    float *viewangles; // r31
    int v13; // r30
    double v14; // fp31
    double v15; // fp12
    gclient_s *v16; // r11
    double viewHeightCurrent; // fp0
    float *currentOrigin; // r4
    double v19; // fp13
    float *v20; // r3
    float v21[4]; // [sp+50h] [-200h] BYREF
    float v22[4]; // [sp+60h] [-1F0h] BYREF
    float v23[4]; // [sp+70h] [-1E0h] BYREF
    float v24[4]; // [sp+80h] [-1D0h] BYREF
    float v25[4]; // [sp+90h] [-1C0h] BYREF
    float v26[4]; // [sp+A0h] [-1B0h] BYREF
    float v27[2]; // [sp+B0h] [-1A0h] BYREF
    float v28; // [sp+B8h] [-198h]
    float v29[4]; // [sp+C0h] [-190h] BYREF
    float v30[4]; // [sp+D0h] [-180h] BYREF
    float v31[4]; // [sp+E0h] [-170h] BYREF
    float v32[3][3]; // [sp+F0h] [-160h] BYREF
    float v33[3]; // [sp+114h] [-13Ch] BYREF
    float v34[4]; // [sp+120h] [-130h] BYREF
    float v35[4]; // [sp+130h] [-120h] BYREF
    float v36[4]; // [sp+140h] [-110h] BYREF
    float v37[4]; // [sp+150h] [-100h] BYREF
    float v38[4][3]; // [sp+160h] [-F0h] BYREF
    float v39[4][3]; // [sp+190h] [-C0h] BYREF
    float v40[7][3]; // [sp+1C0h] [-90h] BYREF

    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1778, 0, "%s", "ent->client");
    client = ent->client;
    if (!ent->tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1782, 0, "%s", "tagInfo");
    G_CalcTagParentAxis(ent, v32);
    if ((client->ps.pm_flags & 0x1000000) != 0)
    {
        AxisToAngles(v32, v25);
        v11 = v25;
        viewangles = client->ps.viewangles;
        v13 = 3;
        do
        {
            v14 = (float)((float)(*v11 - *viewangles) * (float)0.0027777778);
            *(double *)&v10 = (float)((float)((float)(*v11 - *viewangles) * (float)0.0027777778) + (float)0.5);
            v10 = floor(v10);
            v15 = *viewangles;
            --v13;
            ++v11;
            *(viewangles - 32) = (float)((float)((float)v14 - (float)*(double *)&v10) * (float)360.0) + *(viewangles - 32);
            *viewangles++ = (float)v15 + (float)((float)((float)v14 - (float)*(double *)&v10) * (float)360.0);
        } while (v13);
    }
    else
    {
        AxisToQuat(v32, v31);
        AxisToAngles(v32, v24);
        G_UpdateViewAngleClamp(client, v24);
        if (client->prevLinkAnglesSet)
        {
            QuatMultiply(client->prevLinkedInvQuat, v31, v21);
            QuatInverse(v31, client->prevLinkedInvQuat);
            v30[0] = 0.0;
            v30[1] = 0.0;
            v30[2] = 0.0;
            v30[3] = 1.0;
            QuatLerp(v30, v21, client->linkAnglesFrac, v3);
        }
        else
        {
            client->prevLinkAnglesSet = 1;
            QuatInverse(v31, client->prevLinkedInvQuat);
            v21[0] = 0.0;
            v21[1] = 0.0;
            v21[2] = 0.0;
            v21[3] = 1.0;
        }
        v4 = client->ps.viewangles;
        AnglesToAxis(client->ps.viewangles, v40);
        AxisToQuat(v40, v35);
        QuatMultiply(v35, v21, v36);
        QuatToAxis(v36, (mat3x3&)v39);
        AxisToAngles((const mat3x3&)v39, v25);
        v6 = v25;
        v7 = 2;
        do
        {
            v8 = (float)((float)(*v6 - *v4) * (float)0.0027777778);
            *(double *)&v5 = (float)((float)((float)(*v6 - *v4) * (float)0.0027777778) + (float)0.5);
            v5 = floor(v5);
            v9 = *v4;
            --v7;
            ++v6;
            *(v4 - 32) = *(v4 - 32) + (float)((float)((float)v8 - (float)*(double *)&v5) * (float)360.0);
            *v4++ = (float)v9 + (float)((float)((float)v8 - (float)*(double *)&v5) * (float)360.0);
        } while (v7);
        if (ent->client->link_useTagAnglesForViewAngles)
        {
            client->ps.linkAngles[0] = v24[0];
            client->ps.linkAngles[1] = v24[1];
            client->ps.linkAngles[2] = v24[2];
        }
        else
        {
            AnglesToAxis(client->ps.linkAngles, v38);
            AxisToQuat(v38, v37);
            QuatMultiply(v37, v21, v34);
            QuatToAxis(v34, (mat3x3&)v38);
            AxisToAngles((const mat3x3&)v38, client->ps.linkAngles);
        }
    }
    v16 = ent->client;
    if (v16->link_rotationMovesEyePos)
    {
        viewHeightCurrent = client->ps.viewHeightCurrent;
        v29[0] = 0.0;
        v29[1] = 0.0;
        v29[2] = viewHeightCurrent;
        MatrixTransformVector43(v29, (const mat4x3&)v32, v27);
        currentOrigin = v27;
        v28 = v28 - client->ps.viewHeightCurrent;
    }
    else if (v16->link_doCollision)
    {
        v23[0] = ent->r.mins[0];
        v23[1] = ent->r.mins[1];
        v23[2] = ent->r.mins[2];
        v22[0] = ent->r.maxs[0];
        v22[1] = ent->r.maxs[1];
        v22[2] = ent->r.maxs[2];
        v19 = (float)(v33[1] - ent->r.currentOrigin[1]);
        v26[0] = v33[0] - ent->r.currentOrigin[0];
        v26[1] = v19;
        v26[2] = v33[2] - ent->r.currentOrigin[2];
        ExpandBoundsToWidth(v23, v22);
        G_StepSlideMove(1.0, v20, ent->r.currentOrigin, v26, v23, v22, 0, ent->s.number, ent->clipmask & 0xFFFFBFFF);
        currentOrigin = ent->r.currentOrigin;
    }
    else
    {
        currentOrigin = v33;
    }
    G_SetOrigin(ent, currentOrigin);
    ent->s.lerp.pos.trType = TR_INTERPOLATE;
    SV_LinkEntity(ent);
}

void __cdecl G_GeneralLink(gentity_s *ent)
{
    tagInfo_s *tagInfo; // r28
    double v3; // fp13
    double v4; // fp12
    float v5[12]; // [sp+50h] [-90h] BYREF
    float v6[8][3]; // [sp+80h] [-60h] BYREF

    iassert(ent->tagInfo);
    G_CalcTagParentAxis(ent, v6);
    tagInfo = ent->tagInfo;
    iassert(tagInfo);
    MatrixMultiply43(tagInfo->axis, (const mat4x3&)v6, (mat4x3&)v5);
    v3 = v5[10];
    v4 = v5[11];
    ent->r.currentOrigin[0] = v5[9];
    ent->r.currentOrigin[1] = v3;
    ent->r.currentOrigin[2] = v4;
    AxisToAngles((const mat3x3&)v5, ent->r.currentAngles);
    G_SetOrigin(ent, ent->r.currentOrigin);
    ent->s.lerp.apos.trBase[0] = ent->r.currentAngles[0];
    ent->s.lerp.apos.trBase[1] = ent->r.currentAngles[1];
    ent->s.lerp.apos.trBase[2] = ent->r.currentAngles[2];
    ent->s.lerp.apos.trType = TR_STATIONARY;
    ent->s.lerp.apos.trTime = 0;
    ent->s.lerp.apos.trDuration = 0;
    ent->s.lerp.apos.trDelta[0] = 0.0;
    ent->s.lerp.apos.trDelta[1] = 0.0;
    ent->s.lerp.apos.trDelta[2] = 0.0;
    ent->r.currentAngles[0] = ent->r.currentAngles[0];
    ent->r.currentAngles[1] = ent->r.currentAngles[1];
    ent->r.currentAngles[2] = ent->r.currentAngles[2];
    ent->s.lerp.pos.trType = TR_INTERPOLATE;
    ent->s.lerp.apos.trType = TR_INTERPOLATE;
    SV_LinkEntity(ent);
}

gentity_s *__cdecl G_TempEntity(float *origin, int event)
{
    gentity_s *v4; // r3
    gentity_s *v5; // r31
    bool v6; // zf
    float v8[6]; // [sp+50h] [-30h] BYREF

    v4 = G_Spawn();
    v4->s.eType = (entityType_t)(event + 17);
    v6 = (unsigned __int8)(event + 17) == event + 17;
    v5 = v4;
    if (!v6)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            2643,
            0,
            "%s",
            "e->s.eType == ET_EVENTS + event");
    Scr_SetString(&v5->classname, scr_const.tempEntity);
    v5->r.eventType = 1;
    v5->r.eventTime = level.time;
    v8[0] = *origin;
    v8[1] = origin[1];
    v8[2] = origin[2];
    G_SetOrigin(v5, v8);
    SV_LinkEntity(v5);
    return v5;
}

void __cdecl G_PlaySoundAliasAtPoint(float *origin, unsigned __int16 index)
{
    if (!origin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2756, 0, "%s", "origin");
    if (index)
        G_TempEntity(origin, 3)->s.eventParm = index;
}

void __cdecl G_EntUnlink(gentity_s *ent)
{
    tagInfo_s *tagInfo; // r22
    animscripted_s *scripted; // r30
    float *anglesError; // r30
    long double v5; // fp2
    gclient_s *client; // r11
    gentity_s *parent; // r24
    gentity_s *tagChildren; // r30
    gentity_s *v9; // r27
    gclient_s *v10; // r11
    int pm_type; // r10
    float v12[4]; // [sp+50h] [-130h] BYREF
    float v13[3][3]; // [sp+60h] [-120h] BYREF
    float v14[3]; // [sp+84h] [-FCh] BYREF
    float v15[12]; // [sp+90h] [-F0h] BYREF
    float v16[4][3]; // [sp+C0h] [-C0h] BYREF
    float v17[4][3]; // [sp+F0h] [-90h] BYREF

    //Profile_Begin(255);
    tagInfo = ent->tagInfo;
    if (tagInfo)
    {
        scripted = ent->scripted;
        if (scripted)
        {
            G_CalcTagParentRelAxis(ent, v16);
            MatrixTranspose((const mat3x3&)v16, (mat3x3&)v17);
            MatrixMultiply43(scripted->axis, v16, (mat4x3&)v15);
            AxisCopy((const mat3x3&)v15, (mat3x3&)scripted->axis);
            scripted->axis[3][0] = v15[9];
            scripted->axis[3][1] = v15[10];
            scripted->axis[3][2] = v15[11];
            v14[0] = scripted->originError[0];
            v14[1] = scripted->originError[1];
            v14[2] = scripted->originError[2];
            MatrixTransformVector(v14, (const mat3x3&)v16, scripted->originError);
            anglesError = scripted->anglesError;
            AnglesToAxis(anglesError, v13);
            MatrixMultiply((const mat3x3&)v17, v13, (mat3x3&)v15);
            MatrixMultiply((const mat3x3&)v15, (const mat3x3&)v16, v13);
            AxisToSignedAngles(v13, anglesError);
        }
        G_SetOrigin(ent, ent->r.currentOrigin);
        ent->s.lerp.apos.trBase[0] = ent->r.currentAngles[0];
        ent->s.lerp.apos.trBase[1] = ent->r.currentAngles[1];
        ent->s.lerp.apos.trBase[2] = ent->r.currentAngles[2];
        ent->s.lerp.apos.trType = TR_STATIONARY;
        ent->s.lerp.apos.trTime = 0;
        ent->s.lerp.apos.trDuration = 0;
        ent->s.lerp.apos.trDelta[0] = 0.0;
        ent->s.lerp.apos.trDelta[1] = 0.0;
        ent->s.lerp.apos.trDelta[2] = 0.0;
        ent->r.currentAngles[0] = ent->r.currentAngles[0];
        ent->r.currentAngles[1] = ent->r.currentAngles[1];
        ent->r.currentAngles[2] = ent->r.currentAngles[2];
        client = ent->client;
        if (client)
        {
            v12[0] = client->ps.viewangles[0];
            v12[1] = client->ps.viewangles[1];
            v12[2] = 0.0;
            SetClientViewAngle(ent, v12);
            ent->r.currentAngles[0] = 0.0;
        }
        parent = tagInfo->parent;
        if (!tagInfo->parent)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1238, 0, "%s", "parent");
        tagChildren = parent->tagChildren;
        if (tagChildren == ent)
            goto LABEL_16;
        do
        {
            if (!tagChildren->tagInfo)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1244, 0, "%s", "next->tagInfo");
            v9 = tagChildren;
            tagChildren = tagChildren->tagInfo->next;
            if (!tagChildren)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1247, 0, "%s", "next");
        } while (tagChildren != ent);
        if (!v9)
            LABEL_16:
        parent->tagChildren = tagInfo->next;
        else
            v9->tagInfo->next = tagInfo->next;
        v10 = ent->client;
        ent->tagInfo = 0;
        if (v10)
        {
            pm_type = v10->ps.pm_type;
            if (pm_type == PM_NORMAL_LINKED)
            {
                v10->ps.pm_type = PM_NORMAL;
            }
            else if (pm_type == PM_DEAD_LINKED)
            {
                v10->ps.pm_type = PM_DEAD;
            }
        }
        Scr_SetString(&tagInfo->name, 0);
        MT_Free((unsigned char *)tagInfo, 112);
    }
    //Profile_EndInternal(0);
}

void __cdecl G_UpdateTagInfo(gentity_s *ent, int bParentHasDObj)
{
    tagInfo_s *tagInfo; // r31
    int BoneIndex; // r3

    //Profile_Begin(256);
    tagInfo = ent->tagInfo;
    if (!tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1311, 0, "%s", "tagInfo");
    if (tagInfo->name)
    {
        if (!bParentHasDObj
            || (BoneIndex = SV_DObjGetBoneIndex(tagInfo->parent, tagInfo->name), tagInfo->index = BoneIndex, BoneIndex < 0))
        {
            G_EntUnlink(ent);
            //Profile_EndInternal(0);
            return;
        }
    }
    else
    {
        tagInfo->index = -1;
    }
    //Profile_EndInternal(0);
}

void __cdecl G_UpdateTagInfoOfChildren(gentity_s *parent, int bHasDObj)
{
    gentity_s *tagChildren; // r3
    gentity_s *next; // r31

    //Profile_Begin(252);
    tagChildren = parent->tagChildren;
    if (tagChildren)
    {
        do
        {
            next = tagChildren->tagInfo->next;
            G_UpdateTagInfo(tagChildren, bHasDObj);
            tagChildren = next;
        } while (next);
    }
    //Profile_EndInternal(0);
}

void __cdecl G_EntUnlinkFree(gentity_s *ent)
{
    animscripted_s *scripted; // r3

    scripted = ent->scripted;
    if (scripted)
    {
        MT_Free((unsigned char *)scripted, 96);
        ent->scripted = 0;
    }
    G_EntUnlink(ent);
}

void __cdecl G_FreeEntity(gentity_s *ed)
{
    animscripted_s *scripted; // r3
    gentity_s *tagChildren; // r30
    animscripted_s *v4; // r3
    XAnimTree_s *Tree; // r3
    XAnimTree_s *pAnimTree; // r3
    actor_s *actor; // r3
    sentient_s *sentient; // r3
    actor_s *i; // r30
    TurretInfo *pTurretInfo; // r11
    int useCount; // r30
    int inuse; // r10

    //Profile_Begin(246);
    if (Path_IsDynamicBlockingEntity(ed))
        Path_ConnectPathsForEntity(ed);
    if ((ed->flags & 0x200000) != 0)
        Path_RemoveBadPlaceEntity(ed);
    if (ed->disconnectedLinks)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2421, 0, "%s", "!ed->disconnectedLinks");
    scripted = ed->scripted;
    if (scripted)
    {
        MT_Free((unsigned char*)scripted, 96);
        ed->scripted = 0;
    }
    G_EntUnlink(ed);
    while (ed->tagChildren)
    {
        tagChildren = ed->tagChildren;
        v4 = tagChildren->scripted;
        if (v4)
        {
            MT_Free((unsigned char *)v4, 96);
            tagChildren->scripted = 0;
        }
        G_EntUnlink(tagChildren);
    }
    SV_UnlinkEntity(ed);
    if (ed->s.number == level.cachedTagMat.entnum)
        level.cachedTagMat.entnum = ENTITYNUM_NONE;
    if (ed->s.number == level.cachedEntTargetTagMat.entnum)
        level.cachedEntTargetTagMat.entnum = ENTITYNUM_NONE;
    Tree = SV_DObjGetTree(ed);
    if (Tree)
        XAnimClearTree(Tree);
    Com_SafeServerDObjFree(ed->s.number);
    pAnimTree = ed->pAnimTree;
    if (pAnimTree)
    {
        Com_XAnimFreeSmallTree(pAnimTree);
        ed->pAnimTree = 0;
    }
    actor = ed->actor;
    if (actor)
    {
        Actor_Free(actor);
        if (ed->actor)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2450, 0, "%s", "ed->actor == NULL");
    }
    sentient = ed->sentient;
    if (sentient)
    {
        Sentient_Free(sentient);
        if (ed->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2456, 0, "%s", "ed->sentient == NULL");
    }
    else
    {
        G_FreeEntityRefs(ed);
    }
    Actor_EventListener_RemoveEntity(ed->s.number);
    if ((ed->flags & 0x4000000) != 0)
    {
        for (i = Actor_FirstActor(-1); i; i = Actor_NextActor(i, -1))
        {
            if (i->pTurret == ed)
            {
                Actor_StopUseTurret(i);
                if (i->pTurret)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2472, 0, "%s", "pActor->pTurret == NULL");
            }
        }
    }
    if (ed->s.eType == 16)
        ActorCorpse_Free(ed);
    pTurretInfo = ed->pTurretInfo;
    if (pTurretInfo)
    {
        if (!pTurretInfo->inuse)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2482, 0, "%s", "ed->pTurretInfo->inuse");
        G_FreeTurret(ed);
        if (ed->pTurretInfo)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2484, 0, "%s", "ed->pTurretInfo == NULL");
    }
    if (ed->scr_vehicle)
    {
        G_FreeVehicle(ed);
        if (ed->scr_vehicle)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2490, 0, "%s", "ed->scr_vehicle == NULL");
    }
    EntHandleDissociate(ed);
    if (!ed->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2495, 0, "%s", "ed->r.inuse");
    Scr_FreeEntity(ed);
    if (ed->classname)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2498, 0, "%s", "ed->classname == 0");
    useCount = ed->s.lerp.useCount;
    memset(ed, 0, sizeof(gentity_s));
    if (ed - level.gentities >= 1)
    {
        if (level.lastFreeEnt)
            level.lastFreeEnt->nextFree = ed;
        else
            level.firstFreeEnt = ed;
        level.lastFreeEnt = ed;
        ed->nextFree = 0;
    }
    inuse = ed->r.inuse;
    ed->s.lerp.useCount = useCount + 1;
    if (inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 2513, 0, "%s", "!ed->r.inuse");
    //Profile_EndInternal(0);
}

void __cdecl G_UpdateTags(gentity_s *ent, int bHasDObj)
{
    if (ent->scr_vehicle)
        G_UpdateVehicleTags(ent);
    G_UpdateTagInfoOfChildren(ent, bHasDObj);
}

void __cdecl G_DObjUpdate(gentity_s *ent)
{
    int model; // r31
    int v3; // r4
    XAnimTree_s *pAnimTree; // r19
    int eType; // r11
    XAnimTree_s *ActorAnimTree; // r3
    XModel *v7; // r31
    int v8; // r29
    int v9; // r26
    char *v10; // r31
    unsigned __int16 *attachTagNames; // r30
    int v12; // r28
    XModel *v13; // r3
    unsigned int attachIgnoreCollision; // r11
    unsigned int v15; // r11
    DObjModel_s v16; // [sp+50h] [-170h] BYREF
    char v17; // [sp+5Ch] [-164h] BYREF

    //Profile_Begin(251);
    if (ent->s.number == level.cachedTagMat.entnum)
        level.cachedTagMat.entnum = ENTITYNUM_NONE;
    if (ent->s.number == level.cachedEntTargetTagMat.entnum)
        level.cachedEntTargetTagMat.entnum = ENTITYNUM_NONE;
    Com_SafeServerDObjFree(ent->s.number);
    model = ent->model;
    if (!ent->model)
    {
        if (ent->scr_vehicle)
            G_UpdateVehicleTags(ent);
        v3 = 0;
        goto LABEL_31;
    }
    if (!ent->s.lerp.u.actor.species)
    {
        eType = ent->s.eType;
        if (eType == 14)
        {
            ActorAnimTree = G_GetActorAnimTree(ent->actor);
        }
        else
        {
            if (eType != 16)
            {
                pAnimTree = ent->pAnimTree;
                goto LABEL_17;
            }
            ActorAnimTree = G_GetActorCorpseAnimTree(ent);
        }
        pAnimTree = ActorAnimTree;
        goto LABEL_17;
    }
    pAnimTree = ent->pAnimTree;
LABEL_17:
    v7 = G_GetModel(model);
    if (!v7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 746, 0, "%s", "model");
    v16.model = v7;
    v8 = 0;
    v16.boneName = 0;
    v16.ignoreCollision = 0;
    v9 = 1;
    v10 = &v17;
    attachTagNames = ent->attachTagNames;
    do
    {
        v12 = *(attachTagNames - 31);
        if (!*(attachTagNames - 31))
            break;
        if (v9 >= 32)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                760,
                0,
                "%s",
                "numModels < DOBJ_MAX_SUBMODELS");
        v13 = G_GetModel(v12);
        *((unsigned int *)v10 - 1) = (unsigned int)v13;
        if (!v13)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 762, 0, "%s", "dobjModels[numModels].model");
        if (!*attachTagNames)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 763, 0, "%s", "ent->attachTagNames[i]");
        ++v9;
        attachIgnoreCollision = ent->attachIgnoreCollision;
        *(_WORD *)v10 = *attachTagNames++;
        v15 = (1 << v8++) & attachIgnoreCollision;
        //v10[2] = (_cntlzw(v15) & 0x20) == 0;
        v10[2] = v15 != 0;
        v10 += 8;
    } while (v8 < 31);
    Com_ServerDObjCreate(&v16, v9, pAnimTree, ent->s.number);
    if (ent->scr_vehicle)
        G_UpdateVehicleTags(ent);
    v3 = 1;
LABEL_31:
    G_UpdateTagInfoOfChildren(ent, v3);
    //Profile_EndInternal(0);
}

int __cdecl G_EntDetach(gentity_s *ent, const char *modelName, unsigned int tagName)
{
    unsigned int LowercaseString; // r3
    unsigned int v7; // r24
    int v8; // r31
    unsigned __int16 *attachModelNames; // r29
    unsigned int v10; // r28
    unsigned __int16 *v12; // r29
    int v13; // r8
    unsigned __int16 *v14; // r11
    unsigned __int16 v15; // r10
    unsigned int attachIgnoreCollision; // r10
    int v17; // r7
    unsigned int v18; // r10

    //Profile_Begin(250);
    if (!tagName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 977, 0, "%s", "tagName");
    LowercaseString = SL_FindLowercaseString(modelName);
    v7 = LowercaseString;
    if (!LowercaseString || LowercaseString == scr_const._)
    {
    LABEL_11:
        //Profile_EndInternal(0);
        return 0;
    }
    else
    {
        v8 = 0;
        attachModelNames = ent->attachModelNames;
        while (1)
        {
            if (attachModelNames[31] == tagName)
            {
                v10 = *attachModelNames;
                if (v10 >= 0x200)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                        316,
                        0,
                        "%s",
                        "(unsigned)index < MAX_MODELS");
                if (SV_GetConfigstringConst(v10 + 1155) == v7)
                    break;
            }
            ++v8;
            ++attachModelNames;
            if (v8 >= 31)
                goto LABEL_11;
        }
        v12 = &ent->attachModelNames[v8];
        if (!*v12)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 994, 0, "%s", "ent->attachModelNames[i]");
        *v12 = 0;
        Scr_SetString(&ent->attachTagNames[v8], 0);
        if (v8 < 30)
        {
            v13 = v8 + 1;
            v14 = &ent->attachModelNames[v8];
            do
            {
                v15 = v14[1];
                v14[31] = v14[32];
                *v14 = v15;
                attachIgnoreCollision = ent->attachIgnoreCollision;
                v17 = 1 << v8;
                if (((1 << v13) & attachIgnoreCollision) != 0)
                    v18 = v17 | attachIgnoreCollision;
                else
                    v18 = attachIgnoreCollision & ~v17;
                ++v13;
                ent->attachIgnoreCollision = v18;
                ++v8;
                ++v14;
            } while (v13 < 31);
        }
        ent->attachTagNames[v8] = 0;
        ent->attachModelNames[v8] = 0;
        ent->attachIgnoreCollision &= ~(1 << v8);
        G_DObjUpdate(ent);
        //Profile_EndInternal(0);
        return 1;
    }
}

void __cdecl G_EntDetachAll(gentity_s *ent)
{
    unsigned __int16 *attachTagNames; // r31
    int v3; // r30

    attachTagNames = ent->attachTagNames;
    //Profile_Begin(253);
    v3 = 31;
    do
    {
        *(attachTagNames - 31) = 0;
        Scr_SetString(attachTagNames, 0);
        --v3;
        ++attachTagNames;
    } while (v3);
    ent->attachIgnoreCollision = 0;
    G_DObjUpdate(ent);
    //Profile_EndInternal(0);
}

int __cdecl G_EntLinkToInternal(gentity_s *ent, gentity_s *parent, unsigned int tagName)
{
    int BoneIndex; // r24
    int result; // r3
    gentity_s *i; // r31
    gentity_s **p_parent; // r11
    tagInfo_s *v10; // r31
    const char *v11; // r3
    gentity_s *tagChildren; // r11
    animscripted_s *scripted; // r11
    gclient_s *client; // r11
    int pm_type; // r10
    float v16[10][3]; // [sp+50h] [-80h] BYREF

    if (!parent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1061, 0, "%s", "parent");
    if ((ent->flags & 0x800) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            1062,
            0,
            "%s",
            "ent->flags & FL_SUPPORTS_LINKTO");
    G_EntUnlink(ent);
    if (ent->tagInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1066, 0, "%s", "!ent->tagInfo");
    if (tagName)
    {
        if (!SV_DObjExists(parent))
            return 0;
        BoneIndex = SV_DObjGetBoneIndex(parent, tagName);
        if (BoneIndex < 0)
            return 0;
    }
    else
    {
        BoneIndex = -1;
    }
    for (i = parent; ; i = *p_parent)
    {
        if (!i)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 1085, 0, "%s", "checkEnt");
        if (i == ent)
            break;
        p_parent = &i->tagInfo->parent;
        if (!p_parent)
        {
            v10 = (tagInfo_s *)MT_Alloc(112, 17);
            v10->parent = parent;
            v10->name = 0;
            if (tagName && !SL_IsLowercaseString(tagName))
            {
                v11 = SL_ConvertToString(tagName);
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
                    1096,
                    0,
                    "%s\n\t(SL_ConvertToString( tagName )) = %s",
                    "(!tagName || SL_IsLowercaseString( tagName ))",
                    v11);
            }
            Scr_SetString(&v10->name, tagName);
            tagChildren = parent->tagChildren;
            v10->index = BoneIndex;
            v10->next = tagChildren;
            memset(v10->axis, 0, sizeof(v10->axis));
            parent->tagChildren = ent;
            scripted = ent->scripted;
            ent->tagInfo = v10;
            if (scripted)
            {
                G_CalcTagParentAxis(ent, v16);
                MatrixInverseOrthogonal43((const mat4x3&)v16, v10->parentInvAxis);
            }
            else
            {
                memset(v10->parentInvAxis, 0, sizeof(v10->parentInvAxis));
            }
            client = ent->client;
            if (client)
            {
                pm_type = client->ps.pm_type;
                if (pm_type)
                {
                    if (pm_type == PM_DEAD)
                    {
                        result = 1;
                        client->ps.pm_type = PM_DEAD_LINKED;
                        return result;
                    }
                }
                else
                {
                    client->ps.pm_type = PM_NORMAL_LINKED;
                }
            }
            return 1;
        }
    }
    return 0;
}

int __cdecl G_EntLinkTo(gentity_s *ent, gentity_s *parent, unsigned int tagName)
{
    //Profile_Begin(254);
    if (G_EntLinkToInternal(ent, parent, tagName))
    {
        G_CalcTagAxis(ent, 0);
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
        //Profile_EndInternal(0);
        return 0;
    }
}

int __cdecl G_EntLinkToWithOffset(
    gentity_s *ent,
    gentity_s *parent,
    unsigned int tagName,
    float *originOffset,
    const float *anglesOffset)
{
    tagInfo_s *tagInfo; // r31

    //Profile_Begin(254);
    if (G_EntLinkToInternal(ent, parent, tagName))
    {
        tagInfo = ent->tagInfo;
        AnglesToAxis(anglesOffset, tagInfo->axis);
        tagInfo->axis[3][0] = *originOffset;
        tagInfo->axis[3][1] = originOffset[1];
        tagInfo->axis[3][2] = originOffset[2];
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
        //Profile_EndInternal(0);
        return 0;
    }
}

int __cdecl G_EntAttach(gentity_s *ent, const char *modelName, unsigned int tagName, int ignoreCollision)
{
    int v8; // r31
    unsigned __int16 *i; // r11
    int v11; // r30
    int v12; // r31

    //Profile_Begin(249);
    if (!tagName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 930, 0, "%s", "tagName");
    if (G_EntDetach(ent, modelName, tagName))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            933,
            0,
            "%s",
            "!G_EntDetach( ent, modelName, tagName )");
    v8 = 0;
    for (i = ent->attachModelNames; *i; ++i)
    {
        if (++v8 >= 31)
        {
            //Profile_EndInternal(0);
            return 0;
        }
    }
    //Profile_EndInternal(0);
    v11 = G_ModelIndex(modelName);
    if (!v11)
        return 0;
    //Profile_Begin(249);
    if (v11 != (unsigned __int16)v11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            947,
            0,
            "%s",
            "modelIndex == (modelNameIndex_t) modelIndex");
    ent->attachModelNames[v8] = v11;
    if (ent->attachTagNames[v8])
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp", 949, 0, "%s", "!ent->attachTagNames[i]");
    Scr_SetString(&ent->attachTagNames[v8], tagName);
    v12 = 1 << v8;
    if ((ent->attachIgnoreCollision & v12) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_utils.cpp",
            951,
            0,
            "%s",
            "!(ent->attachIgnoreCollision & (1 << i))");
    if (ignoreCollision)
        ent->attachIgnoreCollision |= v12;
    G_DObjUpdate(ent);
    //Profile_EndInternal(0);
    return 1;
}

