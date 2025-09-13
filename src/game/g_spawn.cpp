#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include <script/scr_vm.h>
#include "g_main.h"
#include "actor_fields.h"
#include "sentient_fields.h"
#include "g_save.h"
#include <script/scr_const.h>
#include <server/sv_public.h>
#include <cgame/cg_local.h>
#include "actor_grenade.h"
#include "turret.h"
#include "g_vehicle_path.h"

const ent_field_t fields_1[16] =
{
  { "classname", 284, F_STRING, &Scr_ReadOnlyField },
  { "origin", 224, F_VECTOR, &Scr_SetOrigin },
  { "model", 280, F_MODEL, &Scr_ReadOnlyField },
  { "spawnflags", 300, F_INT, &Scr_ReadOnlyField },
  { "target", 290, F_STRING, NULL },
  { "targetname", 292, F_STRING, NULL },
  { "count", 340, F_INT, NULL },
  { "health", 324, F_INT, &Scr_SetHealth },
  { "dmg", 336, F_INT, NULL },
  { "angles", 236, F_VECTOR, &Scr_SetAngles },
  { "script_linkname", 286, F_STRING, NULL },
  { "script_noteworthy", 288, F_STRING, NULL },
  { "maxhealth", 328, F_INT, NULL },
  { "anglelerprate", 616, F_FLOAT, NULL },
  { "activator", 348, F_ENTITY, &Scr_ReadOnlyField },
  { NULL, 0, F_INT, NULL }
};


void SP_script_vehicle_collmap(gentity_s *pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = ET_VEHICLE_COLLMAP;
}

const SpawnFuncEntry s_bspOrDynamicSpawns[9] =
{
  { "info_player_start", &SP_info_player_start },
  { "info_notnull", &SP_info_notnull },
  { "info_notnull_big", &SP_info_notnull },
  { "info_volume", &SP_info_volume },
  { "trigger_radius", &SP_trigger_radius },
  { "sound_blend", &SP_sound_blend },
  { "script_model", &SP_script_model },
  { "script_origin", &SP_script_origin },
  { "script_vehicle_collmap", &SP_script_vehicle_collmap }
};

void trigger_use_touch(gentity_s *ent)
{
    trigger_use_shared(ent);
}

void SP_script_vehicle(gentity_s *pSelf)
{
    const char *v2; // [sp+50h] [-20h] BYREF

    G_LevelSpawnString("vehicletype", 0, &v2);
    G_SpawnVehicle(pSelf, v2, 1);
}

const SpawnFuncEntry s_bspOnlySpawns[16] =
{
  { "info_grenade_hint", &SP_info_grenade_hint },
  { "trigger_use", &trigger_use },
  { "trigger_use_touch", &trigger_use_touch },
  { "trigger_multiple", &SP_trigger_multiple },
  { "trigger_disk", &SP_trigger_disk },
  { "trigger_friendlychain", &SP_trigger_friendlychain },
  { "trigger_hurt", &SP_trigger_hurt },
  { "trigger_once", &SP_trigger_once },
  { "trigger_damage", &SP_trigger_damage },
  { "trigger_lookat", &SP_trigger_lookat },
  { "light", &SP_light },
  { "misc_mg42", &SP_turret },
  { "misc_turret", &SP_turret },
  { "script_brushmodel", &SP_script_brushmodel },
  { "script_struct", &G_FreeEntity },
  { "script_vehicle", &SP_script_vehicle }
};

int __cdecl G_LevelSpawnString(const char *key, const char *defaultString, const char **out)
{
    return G_SpawnString(&level.spawnVar, key, defaultString, out);
}

int __cdecl G_SpawnFloat(const char *key, const char *defaultString, float *out)
{
    int v4; // r30
    long double v5; // fp2
    int result; // r3
    const char *v7; // [sp+50h] [-20h] BYREF

    v4 = G_SpawnString(&level.spawnVar, key, defaultString, &v7);
    v5 = atof(v7);
    result = v4;
    *out = *(double *)&v5;
    return result;
}

int __cdecl G_SpawnInt(const char *key, const char *defaultString, int *out)
{
    int v4; // r30
    const char *v6; // [sp+50h] [-20h] BYREF

    v4 = G_SpawnString(&level.spawnVar, key, defaultString, &v6);
    *out = atol(v6);
    return v4;
}

int __cdecl G_SpawnVector(const char *key, const char *defaultString, float *out)
{
    int v4; // r30
    const char *v5; // r3
    const char *v7; // [sp+50h] [-20h] BYREF

    v4 = G_SpawnString(&level.spawnVar, key, defaultString, &v7);
    v5 = v7;
    *out = 0.0;
    out[1] = 0.0;
    out[2] = 0.0;
    sscanf(v5, "%f %f %f", out, out + 1, out + 2);
    return v4;
}

void __cdecl Scr_ReadOnlyField(gentity_s *ent, int offset)
{
    Scr_Error("Tried to set a read only entity field");
}

unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, const char *value)
{
    unsigned int result; // r3
    unsigned int v4; // r30
    int type; // [sp+50h] [-30h] BYREF
    float vec[3]; // [sp+58h] [-28h] BYREF

    result = Scr_FindField(key, &type);
    v4 = result;
    if (result)
    {
        switch (type)
        {
        case 2:
            Scr_AddString(value);
            break;
        case 4:
            vec[0] = 0.0f;
            vec[1] = 0.0f;
            vec[2] = 0.0f;
            sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
            Scr_AddVector(vec);
            break;
        case 5:
            Scr_AddFloat(atof(value));
            break;
        case 6:
            Scr_AddInt(atol(value));
            break;
        default:
            if (!alwaysfails)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 204, 0, va("G_SetEntityScriptVariableInternal: bad case %d", type));
            }
            break;
        }
        return v4;
    }
    return result;
}

void G_SpawnStruct()
{
    int createstruct; // r3
    unsigned int Object; // r29
    int v2; // r30
    const char **v3; // r31
    unsigned int v4; // r4

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 311, 0, "%s", "level.spawnVar.spawnVarsValid");
    createstruct = g_scr_data.createstruct;
    if (!g_scr_data.createstruct)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 313, 0, "%s", "g_scr_data.createstruct");
        createstruct = g_scr_data.createstruct;
    }
    Scr_AddExecThread(createstruct, 0);
    Object = Scr_GetObject(0);
    v2 = 0;
    if (level.spawnVar.numSpawnVars > 0)
    {
        v3 = (const char **)level.spawnVar.spawnVars[0];
        do
        {
            v4 = G_SetEntityScriptVariableInternal(*v3, v3[1]);
            if (v4)
                Scr_SetStructField(Object, v4);
            ++v2;
            v3 += 2;
        } while (v2 < level.spawnVar.numSpawnVars);
    }
}

void __cdecl G_DuplicateEntityFields(gentity_s *dest, const gentity_s *source)
{
    //const int *p_ofs; // r31
    //float *v5; // r11
    //float *v6; // r10
    //
    //if ("classname")
    //{
    //    p_ofs = &fields_1[0].ofs;
    //    do
    //    {
    //        switch (p_ofs[1])
    //        {
    //        case 0:
    //            *(unsigned int *)(&dest->s.eType + *p_ofs) = *(unsigned int *)(&source->s.eType + *p_ofs);
    //            break;
    //        case 1:
    //        case 14:
    //            *(_WORD *)(&dest->s.eType + *p_ofs) = *(_WORD *)(&source->s.eType + *p_ofs);
    //            break;
    //        case 2:
    //            *(&dest->s.eType + *p_ofs) = *(&source->s.eType + *p_ofs);
    //            break;
    //        case 3:
    //            *(float *)(&dest->s.eType + *p_ofs) = *(float *)(&source->s.eType + *p_ofs);
    //            break;
    //        case 4:
    //            Scr_SetString((unsigned __int16 *)(&dest->s.eType + *p_ofs), *(unsigned __int16 *)(&source->s.eType + *p_ofs));
    //            break;
    //        case 5:
    //            v5 = (float *)(&source->s.eType + *p_ofs);
    //            v6 = (float *)(&dest->s.eType + *p_ofs);
    //            *v6 = *v5;
    //            v6[1] = v5[1];
    //            v6[2] = v5[2];
    //            break;
    //        default:
    //            break;
    //        }
    //        p_ofs += 4;
    //    } while (*(p_ofs - 1));
    //}


    float *destVec; // [esp+8h] [ebp-14h]
    float *sourceVec; // [esp+Ch] [ebp-10h]
    const ent_field_t *f; // [esp+14h] [ebp-8h]

    for (f = fields_1; f->name; ++f)
    {
        switch (f->type)
        {
        case F_INT:
            *(int *)((char *)dest + f->ofs) = *(int *)((char *)source + f->ofs);
            break;
        case F_SHORT:
        case F_MODEL:
            *(_WORD *)((char *)dest + f->ofs) = *(_WORD *)((char *)source + f->ofs);
            break;
        case F_BYTE:
            *(byte *)((char*)dest + f->ofs) = *(byte *)((char *)source + f->ofs);
            break;
        case F_FLOAT:
            *(float *)((char *)dest + f->ofs) = *(float *)((char *)source + f->ofs);
            break;
        case F_STRING:
            Scr_SetString((unsigned __int16 *)((char *)dest + f->ofs), *(unsigned __int16 *)((char *)source + f->ofs));
            break;
        case F_VECTOR:
            destVec = (float *)((char *)dest + f->ofs);
            sourceVec = (float *)((char *)source + f->ofs);
            destVec[0] = sourceVec[0];
            destVec[1] = sourceVec[1];
            destVec[2] = sourceVec[2];
            break;
        default:
            continue;
        }
    }
}

void __cdecl G_DuplicateScriptFields(gentity_s *dest, const gentity_s *source)
{
    if (dest->s.number != dest - g_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            372,
            0,
            "%s",
            "dest->s.number == dest - g_entities");
    if (source->s.number != source - g_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            373,
            0,
            "%s",
            "source->s.number == source - g_entities");
    Scr_CopyEntityNum(source->s.number, dest->s.number, 0);
}

const gitem_s *__cdecl G_GetItemForClassname(const char *classname, unsigned __int8 model)
{
    unsigned int WeaponIndexForName; // r3
    int v5; // r31

    if (strncmp(classname, "weapon_", 7u))
        return 0;
    WeaponIndexForName = G_GetWeaponIndexForName(classname + 7);
    v5 = WeaponIndexForName;
    if (!WeaponIndexForName)
        return 0;
    BG_GetWeaponDef(WeaponIndexForName);
    return BG_FindItemForWeapon(v5, model);
}

void(__cdecl *__cdecl G_FindSpawnFunc(const char *classname, const SpawnFuncEntry *spawnFuncArray, int spawnFuncCount))(gentity_s *)
{
    int spawnFuncIter; // [esp+14h] [ebp-4h]

    for (spawnFuncIter = 0; spawnFuncIter < spawnFuncCount; ++spawnFuncIter)
    {
        if (!strcmp(classname, spawnFuncArray[spawnFuncIter].classname))
            return spawnFuncArray[spawnFuncIter].callback;
    }

    return NULL;
}

void __cdecl G_PrintBadModelMessage(gentity_s *ent)
{
    unsigned int v2; // r3
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 415, 0, "%s", "ent");
    if (ent->model)
    {
        if (G_XModelBad(ent->model))
        {
            v2 = G_ModelName(ent->model);
            SL_ConvertToString(v2);
            v3 = ent->r.currentOrigin[2];
            v4 = ent->r.currentOrigin[1];
            v5 = ent->r.currentOrigin[0];
            SL_ConvertToString(ent->classname);
            Com_PrintError(
                1,
                "Error: '%s' at ( %.0f %.0f %.0f ) uses missing model '%s'\n",
                (const char *)HIDWORD(v5),
                v5,
                v4,
                v3,
                (const char *)HIDWORD(v4));
        }
    }
}

int __cdecl G_CallSpawnEntity(gentity_s *ent)
{
    const char *v3; // r30
    const gitem_s *ItemForClassname; // r4
    int v5; // r6
    const SpawnFuncEntry *v6; // r7
    const char *classname; // r10
    const char *v8; // r11
    int v9; // r8
    void(__cdecl * callback)(gentity_s *); // r31

    if (level.spawnVar.spawnVarsValid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 518, 1, "%s", "!level.spawnVar.spawnVarsValid");
    if (ent->classname)
    {
        v3 = SL_ConvertToString(ent->classname);
        if (strncmp(v3, "actor_", 6u))
        {
            ItemForClassname = G_GetItemForClassname(v3, 0);
            if (ItemForClassname)
            {
                G_SpawnItem(ent, ItemForClassname);
                return 1;
            }
            else
            {
                v5 = 0;
                v6 = s_bspOrDynamicSpawns;
                while (1)
                {
                    classname = v6->classname;
                    v8 = v3;
                    do
                    {
                        v9 = *(unsigned __int8 *)v8 - *(unsigned __int8 *)classname;
                        if (!*v8)
                            break;
                        ++v8;
                        ++classname;
                    } while (!v9);
                    if (!v9)
                        break;
                    ++v6;
                    ++v5;
                    if ((int)v6 >= (int)"Tried to set a read only entity field")
                        goto LABEL_17;
                }
                callback = s_bspOrDynamicSpawns[v5].callback;
                if (!callback)
                {
                LABEL_17:
                    Com_Printf(15, "%s cannot be spawned dynamically\n", v3);
                    return 0;
                }
                if (callback == G_FreeEntity)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 549, 0, "%s", "spawnFunc != G_FreeEntity");
                callback(ent);
                if (!ent->r.inuse)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 551, 0, "%s", "ent->r.inuse");
                return 1;
            }
        }
        else
        {
            Com_Error(ERR_DROP, "cannot spawn AI directly; use spawners instead");
            return 0;
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawnEntity: NULL classname\n");
        return 0;
    }
}

void __cdecl GScr_AddFieldsForEntity()
{
    const ent_field_t *f;

    for (f = fields_1; f->name; ++f)
    {
        iassert(((f - fields_1) & ENTFIELD_MASK) == ENTFIELD_ENTITY);
        iassert((f - fields_1) == (unsigned short)(f - fields_1));

        Scr_AddClassField(0, (char*)f->name, (unsigned __int16)(f - fields_1));
    }

    GScr_AddFieldsForActor();
    GScr_AddFieldsForSentient();
    GScr_AddFieldsForClient();
}

void __cdecl GScr_AddFieldsForRadiant()
{
    Scr_AddFields("radiant", "txt");
}

void __cdecl Scr_FreeEntity(gentity_s *ent)
{
    int number; // r7

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 860, 0, "%s", "ent");
    number = ent->s.number;
    if (number != ent - g_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            861,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            862,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            ent->s.number);
    Scr_FreeEntityFields(ent);
    Scr_FreeEntityNum(ent->s.number, 0);
}

void __cdecl Scr_AddEntity(gentity_s *ent)
{
    int number; // r7

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 871, 0, "%s", "ent");
    number = ent->s.number;
    if (number != ent - g_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            872,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            873,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            ent->s.number);
    Scr_AddEntityNum(ent->s.number, 0);
}

gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index)
{
    scr_entref_t entref; // [sp+50h] [-20h]

    if (!Scr_GetType(0))
        return 0;
    entref = (scr_entref_t)Scr_GetEntityRef(index);
    if (entref.classnum)
        return 0;
    iassert(entref.entnum < MAX_GENTITIES);
    return &g_entities[entref.entnum];
}

gentity_s *__cdecl Scr_GetEntity(unsigned int index)
{
    scr_entref_t entref; // [sp+50h] [-20h]

    entref = Scr_GetEntityRef(index);
    if (entref.classnum)
    {
        Scr_ParamError(index, "not an entity");
        return 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        return &g_entities[entref.entnum];
    }
}

void __cdecl Scr_FreeHudElem(game_hudelem_s *hud)
{
    unsigned int v2; // r31

    iassert(hud);
    v2 = hud - g_hudelems;
    if (v2 >= 0x100)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            919,
            0,
            "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
            v2,
            256);
    if (hud->elem.type == HE_TYPE_FREE)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 920, 0, "%s", "hud->elem.type != HE_TYPE_FREE");
    Scr_NotifyNum(v2, 1u, scr_const.death, 0);
    Scr_FreeHudElemConstStrings(hud);
    Scr_FreeEntityNum(v2, 1u);
}

void __cdecl Scr_AddHudElem(game_hudelem_s *hud)
{
    unsigned int v2; // r31

    if (!hud)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 934, 0, "%s", "hud");
    v2 = hud - g_hudelems;
    if (v2 >= 0x100)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            935,
            0,
            "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
            v2,
            256);
    if (hud->elem.type == HE_TYPE_FREE)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 936, 0, "%s", "hud->elem.type != HE_TYPE_FREE");
    Scr_AddEntityNum(v2, 1u);
}

game_hudelem_s *__cdecl Scr_GetHudElem(unsigned int index)
{
    scr_entref_t entref; // [sp+50h] [-20h]

    entref = Scr_GetEntityRef(index);
    if (entref.classnum == 1)
    {
        iassert(entref.entnum < MAX_HUDELEMS_TOTAL);
        return &g_hudelems[entref.entnum];
    }
    else
    {
        Scr_ParamError(index, "not a hudelem");
        return 0;
    }
}

int __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount)
{
    int number; // r7

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 959, 0, "%s", "ent");
    number = ent->s.number;
    if (number != ent - g_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            960,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            961,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            ent->s.number);
    return Scr_ExecEntThreadNum(ent->s.number, 0, handle, paramcount);
}

void __cdecl Scr_AddExecEntThread(gentity_s *ent, int handle, unsigned int paramcount)
{
    int number; // r7

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 969, 0, "%s", "ent");
    number = ent->s.number;
    if (number != ent - g_entities)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            970,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            971,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            ent->s.number);
    Scr_AddExecEntThreadNum(ent->s.number, 0, handle, paramcount);
}

void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount)
{
    const char *v6; // r3
    const char *v7; // r3

    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 979, 0, "%s", "ent");
    if (ent->s.number != ent - g_entities)
    {
        v6 = SL_ConvertToString(stringValue);
        v7 = va("s.number = %i, index = %i, stringValue = %s", ent->s.number, ent - g_entities, v6);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            980,
            0,
            "%s\n\t%s",
            "ent->s.number == ent - g_entities",
            v7);
    }
    if (!ent->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            981,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            ent->s.number);
    Scr_NotifyNum(ent->s.number, 0, stringValue, paramcount);
}

void __cdecl Scr_GetGenericEnt(unsigned int offset, unsigned int name)
{
    const ent_field_t *f; // r27
    gentity_s *v5; // r3
    gentity_s *entItr; // r31
    int i; // r30
    int num_entities; // r10

    bcassert(offset, ARRAY_COUNT(fields_1));

    f = &fields_1[offset];
    if (f->type != F_STRING)
        Scr_ParamError(1u, "key is not internally a string");


    v5 = 0;
    entItr = g_entities;
    i = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        do
        {
            if (entItr->r.inuse && *(_WORD *)((char*)entItr + f->ofs) && *(unsigned __int16 *)((char*)entItr + f->ofs) == name)
            {
                if (v5)
                {
                    Scr_Error("getent used with more than one entity");
                    num_entities = level.num_entities;
                }
                v5 = entItr;
            }
            ++i;
            ++entItr;
        } while (i < num_entities);
        if (v5)
            Scr_AddEntity(v5);
    }
}

void __cdecl Scr_GetEnt()
{
    unsigned int ConstString; // r31
    const char *String; // r3
    signed int Offset; // r3

    if (Scr_GetNumParam() != 2)
        Scr_Error("illegal call to getent()\n");
    ConstString = Scr_GetConstString(0);
    String = Scr_GetString(1);
    Offset = Scr_GetOffset(0, String);
    if (Offset >= 0)
    {
        if ((Offset & 0xC000) != 0)
            Scr_ParamError(1u, "key does not internally belong to generic entities");
        else
            Scr_GetGenericEnt(Offset, ConstString);
    }
}

void __cdecl Scr_GetGenericEntArray(unsigned int offset, unsigned int name)
{
    const ent_field_t *v4; // r28
    int v5; // r30
    gentity_s *v6; // r31
    int i; // r10

    if (offset >= 0xF)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            1073,
            0,
            "offset doesn't index ARRAY_COUNT( fields ) - 1\n\t%i not in [0, %i)",
            offset,
            15);
    v4 = &fields_1[offset];
    if (v4->type != F_STRING)
        Scr_ParamError(1u, "key is not internally a string");
    Scr_MakeArray();
    v5 = 0;
    v6 = g_entities;
    for (i = level.num_entities; v5 < i; ++v6)
    {
        if (v6->r.inuse && *(_WORD *)(&v6->s.eType + v4->ofs) && *(unsigned __int16 *)(&v6->s.eType + v4->ofs) == name)
        {
            Scr_AddEntity(v6);
            Scr_AddArray();
            i = level.num_entities;
        }
        ++v5;
    }
}

void __cdecl Scr_GetEntArray()
{
    int v0; // r30
    gentity_s *v1; // r31
    int i; // r11
    unsigned int ConstString; // r30
    const char *String; // r31
    signed int Offset; // r3
    const char *v6; // r3

    if (Scr_GetNumParam())
    {
        ConstString = Scr_GetConstString(0);
        String = Scr_GetString(1);
        Offset = Scr_GetOffset(0, String);
        if (Offset >= 0)
        {
            if ((Offset & 0xC000) == 0)
            {
                Scr_GetGenericEntArray(Offset, ConstString);
                return;
            }
            v6 = va("key '%s' does not internally belong to generic entities", String);
        }
        else
        {
            v6 = va("key '%s' does not internally belong to entities", String);
        }
        Scr_ParamError(1u, v6);
    }
    else
    {
        Scr_MakeArray();
        v0 = 0;
        v1 = g_entities;
        for (i = level.num_entities; v0 < i; ++v1)
        {
            if (v1->r.inuse)
            {
                Scr_AddEntity(v1);
                Scr_AddArray();
                i = level.num_entities;
            }
            ++v0;
        }
    }
}

void __cdecl GScr_SetDynamicEntityField(gentity_s *ent, unsigned int index)
{
    Scr_SetDynamicEntityField(ent->s.number, 0, index);
}

void __cdecl SP_worldspawn()
{
    double ambient; // fp31
    double diffuse; // fp30
    double sunlight; // fp29
    float color[3]; // [sp+58h] [-78h] BYREF

    const char *classname;

    G_SpawnString(&level.spawnVar, "classname", "", &classname);

    if (I_stricmp(classname, "worldspawn"))
        Com_Error(ERR_DROP, "SP_worldspawn: the first entity isn't worldspawn");

    SV_SetConfigstring(2, "cod-sp");

    const char *ambienttrack;
    G_SpawnString(&level.spawnVar, "ambienttrack", "", &ambienttrack);
    if (ambienttrack[0])
        SV_SetConfigstring(1114, va("n\\%s", ambienttrack));
    else
        SV_SetConfigstring(1114, "");

    const char *message;
    G_SpawnString(&level.spawnVar, "message", "", &message);
    SV_SetConfigstring(3, message);

    const char *gravity;
    G_SpawnString(&level.spawnVar, "gravity", "800", &gravity);
    iassert(g_gravity);
    Dvar_SetFloat(g_gravity, atof(gravity));

    const char *northyaw;
    G_SpawnString(&level.spawnVar, "northyaw", "", &northyaw);
    if (northyaw[0])
    {
        SV_SetConfigstring(1147, northyaw);

        float v3 = atof(northyaw) * 0.017453292f;
        level.compassNorth[1] = sin(v3);
        level.compassNorth[0] = cos(v3);;
    }
    else
    {
        SV_SetConfigstring(1147, "0");
        level.compassNorth[0] = 1.0;
        level.compassNorth[1] = 0.0;
    }
    g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
    Scr_SetString(&g_entities[ENTITYNUM_WORLD].classname, scr_const.worldspawn);
    g_entities[ENTITYNUM_WORLD].r.inuse = 1;
    iassert(!g_entities[ENTITYNUM_WORLD].r.ownerNum.isDefined());

    const char *ambientLight;
    G_SpawnString(&level.spawnVar, "ambient", "0", &ambientLight);
    ambient = atof(ambientLight);

    const char *diffuseFraction;
    G_SpawnString(&level.spawnVar, "diffuseFraction", "0", &diffuseFraction);
    diffuse = atof(diffuseFraction);

    const char *sunLight;
    G_SpawnString(&level.spawnVar, "sunLight", "1", &sunLight);
    sunlight = atof(sunLight);

    const char *sunColor;
    G_SpawnString(&level.spawnVar, "sunColor", "1 1 1", &sunColor);
    sscanf(sunColor, "%g %g %g", &color[0], &color[1], &color[2]);
    ColorNormalize(color, color);

    //_FP13 = -(float)((float)((float)v11 - (float)v7) * (float)((float)1.0 - (float)v9));
    //__asm { fsel      f0, f13, f27, f0 }
    //level.mapSunColor[0] = color[0] * (float)_FP0;
    //level.mapSunColor[1] = color[1] * (float)_FP0;
    //level.mapSunColor[2] = color[2] * (float)_FP0;

    float scale = (sunlight - ambient) * (1.0f - diffuse);
    scale = (scale < 0.0f) ? 1.0f : scale;

    level.mapSunColor[0] = color[0] * scale;
    level.mapSunColor[1] = color[1] * scale;
    level.mapSunColor[2] = color[2] * scale;

    const char *sunDirection;
    float sunDir[3];
    G_SpawnString(&level.spawnVar, "sunDirection", "0 0 0", &sunDirection);
    sscanf(sunDirection, "%g %g %g", &sunDir[0], &sunDir[1], &sunDir[2]);
    AngleVectors(sunDir, level.mapSunDirection, 0, 0);
}

void __cdecl G_LoadStructs()
{
    int initstructs; // r3
    unsigned __int16 v1; // r3
    const char *v2; // r10
    const char *v3; // r11
    int v4; // r8
    const char *v5; // [sp+50h] [-30h] BYREF

    initstructs = g_scr_data.initstructs;
    if (!g_scr_data.initstructs)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 1276, 0, "%s", "g_scr_data.initstructs");
        initstructs = g_scr_data.initstructs;
    }
    v1 = Scr_ExecThread(initstructs, 0);
    Scr_FreeThread(v1);
    while (G_ParseSpawnVars(&level.spawnVar))
    {
        G_SpawnString(&level.spawnVar, "classname", "", &v5);
        v2 = v5;
        v3 = "script_struct";
        do
        {
            v4 = *(unsigned __int8 *)v3 - *(unsigned __int8 *)v2;
            if (!*v3)
                break;
            ++v3;
            ++v2;
        } while (!v4);
        if (!v4)
            G_SpawnStruct();
    }
    G_ResetEntityParsePoint();
}

void __cdecl G_SetEntityScriptVariable(const char *key, const char *value, gentity_s *ent)
{
    unsigned int v4; // r5

    v4 = G_SetEntityScriptVariableInternal(key, value);
    if (v4)
        Scr_SetDynamicEntityField(ent->s.number, 0, v4);
}

typedef unsigned __int16 modelNameIndex_t;
void __cdecl G_ParseEntityField(const char *key, const char *value, gentity_s *ent, int ignoreModel)
{
    const ent_field_t *f; // r31
    int modelIndex; // r3
    float vec[3]; // [sp+50h] [-40h] BYREF


    for (f = fields_1; ; ++f)
    {
        if (!f->name)
        {
            G_SetEntityScriptVariable(key, value, ent);
            return;
        }
        if (!I_stricmp(f->name, key))
            break;
    }

    switch (f->type)
    {
    case F_INT:
        *(int32_t *)((char *)ent + f->ofs) = atoi(value);
        break;
    case F_SHORT:
        *(_WORD *)((char*)ent + f->ofs) = atol(value);
        break;
    case F_BYTE:
        *((unsigned char*)ent + f->ofs) = (entityType_t)atol(value);
        break;
    case F_FLOAT:
        *(float *)((char *)ent + f->ofs) = atof(value);
        break;
    case F_STRING:
        Scr_SetString((uint16_t *)((char *)ent + f->ofs), 0);
        *(_WORD *)((char *)ent + f->ofs) = G_NewString(value).prev;
        break;
    case F_VECTOR:
        vec[0] = 0.0;
        vec[1] = 0.0;
        vec[2] = 0.0;
        sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
        *(float *)((char *)ent + f->ofs) = vec[0];
        *(float *)((char *)ent + f->ofs + 4) = vec[1];
        *(float *)((char *)ent + f->ofs + 8) = vec[2];
        break;
    case F_MODEL:
        if (!ignoreModel)
        {
            if (*value == 42)
            {
                modelIndex = atol(value + 1);
                iassert(modelIndex == (modelNameIndex_t)modelIndex);
                ent->s.index.item = modelIndex;
            }
            else
            {
                if (Com_IsLegacyXModelName(value))
                    value += 7;
                G_SetModel(ent, value);
            }
        }
        break;
    default:
        return;
    }
}

void __cdecl G_ParseEntityFields(gentity_s *ent, int ignoreModel)
{
    int v4; // r30
    const char **v5; // r31

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 295, 0, "%s", "level.spawnVar.spawnVarsValid");
    v4 = 0;
    if (level.spawnVar.numSpawnVars > 0)
    {
        v5 = (const char **)level.spawnVar.spawnVars[0];
        do
        {
            G_ParseEntityField(*v5, v5[1], ent, ignoreModel);
            ++v4;
            v5 += 2;
        } while (v4 < level.spawnVar.numSpawnVars);
    }
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
}

void G_CallSpawn()
{
    gentity_s *ent; // r31
    const gitem_s *item; // r30
    void(__cdecl * spawnFunc)(gentity_s *); // r31
    const char *classname; // [sp+50h] [-20h] BYREF

    iassert(level.spawnVar.spawnVarsValid);

    G_SpawnString(&level.spawnVar, "classname", "", &classname);

    if (classname)
    {
        if (!strncmp(classname, "dyn_", 4))
        {
            return;
        }

        if (!strncmp(classname, "node_", 5))
        {
            //if (!IsFastFileLoad())
            //    G_SpawnPathnodeStatic(spawnVar, classname);
            G_SpawnPathnodeDynamic();
        }
        else if (!strcmp(classname, "info_vehicle_node"))
        {
            SP_info_vehicle_node(0);
        }
        else if (!strcmp(classname, "info_vehicle_node_rotate"))
        {
            SP_info_vehicle_node(1);
        }
        else if (!strncmp(classname, "actor_", 6))
        {
            ent = G_Spawn();
            G_ParseEntityFields(ent, 1);
            SP_actor(ent);
        }
        else
        {
            item = G_GetItemForClassname(classname, 0);
            if (item)
            {
                ent = G_Spawn();
                G_ParseEntityFields(ent, 0);
                G_SpawnItem(ent, item);
            }
            else
            {
                spawnFunc = G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, ARRAY_COUNT(s_bspOrDynamicSpawns));

                if (!spawnFunc && level.spawnVar.spawnVarsValid)
                {
                    spawnFunc = G_FindSpawnFunc(classname, s_bspOnlySpawns, ARRAY_COUNT(s_bspOnlySpawns));
                }

                if (!spawnFunc)
                {
                    Com_Printf(15, "%s doesn't have a spawn function\n", classname);
                    return;
                }
                ent = G_Spawn();
                G_ParseEntityFields(ent, 0);
                G_PrintBadModelMessage(ent);
                G_DObjUpdate(ent);
                spawnFunc(ent);
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawn: NULL classname\n");
    }
}

void __cdecl Scr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs)
{
    unsigned int ConstStringIncludeNull; // r3
    unsigned __int8 *v6; // r11
    double v7; // fp13
    gentity_s *Int; // r3
    gentity_s *EntityAllowNull; // r3
    gentity_s *v10; // r3
    gentity_s *v11; // r3
    gentity_s *v12; // r3
    float v13; // [sp+50h] [-30h] BYREF
    float v14; // [sp+54h] [-2Ch]
    float v15; // [sp+58h] [-28h]

    EntHandle *enthand;
    SentientHandle *senthand;

    switch (type)
    {
    case F_INT:
        Int = (gentity_s *)Scr_GetInt(0);
        goto LABEL_20;
    case F_SHORT:
        *(_WORD *)&b[ofs] = Scr_GetInt(0);
        return;
    case F_BYTE:
        b[ofs] = Scr_GetInt(0);
        return;
    case F_FLOAT:
        *(float *)&b[ofs] = Scr_GetFloat(0);
        return;
    case F_STRING:
        ConstStringIncludeNull = Scr_GetConstStringIncludeNull(0);
        Scr_SetString((unsigned __int16 *)&b[ofs], ConstStringIncludeNull);
        return;
    case F_VECTOR:
        Scr_GetVector(0, &v13);
        v6 = &b[ofs];
        *(float *)&b[ofs] = v13;
        v7 = v15;
        *((float *)v6 + 1) = v14;
        *((float *)v6 + 2) = v7;
        return;
    case F_ENTITY:
        Int = Scr_GetEntityAllowNull(0);
        goto LABEL_20;
    case F_ENTHANDLE:
        EntityAllowNull = Scr_GetEntityAllowNull(0);
        enthand = (EntHandle *)&b[ofs];
        enthand->setEnt(EntityAllowNull);
        return;
    case F_ACTOR:
        v10 = Scr_GetEntityAllowNull(0);
        if (!v10)
            goto LABEL_12;
        *(uintptr_t *)&b[ofs] = (uintptr_t)v10->actor;
        break;
    case F_SENTIENT:
        v11 = Scr_GetEntityAllowNull(0);
        if (v11)
            *(uintptr_t *)&b[ofs] = (uintptr_t)v11->sentient;
        else
            LABEL_12:
        *(uintptr_t *)&b[ofs] = 0;
        break;
    case F_SENTIENTHANDLE:
        v12 = Scr_GetEntityAllowNull(0);
        if (v12)
        {
            senthand = (SentientHandle *)&b[ofs];
            senthand->setSentient(v12->sentient);
        }
        else
        {
            senthand = (SentientHandle *)&b[ofs];
            senthand->setSentient(NULL);
        }
        break;
    case F_PATHNODE:
        Int = (gentity_s *)Scr_GetPathnode(0);
    LABEL_20:
        *(uintptr_t *)&b[ofs] = (uintptr_t)Int;
        break;
    case F_VECTORHACK:
        Scr_GetVector(0, &v13);
        *(float *)&b[ofs] = v14;
        break;
    default:
        return;
    }
}

void __cdecl Scr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs)
{
    unsigned int v4; // r3
    gentity_s *v5; // r3
    gentity_s **v8; // r11
    pathnode_t *v11; // r3
    float v12[4]; // [sp+50h] [-20h] BYREF

    EntHandle *enthand;
    SentientHandle *senthand;

    switch (type)
    {
    case F_INT:
        Scr_AddInt(*(unsigned int *)&b[ofs]);
        break;
    case F_SHORT:
        Scr_AddInt(*(__int16 *)&b[ofs]);
        break;
    case F_BYTE:
        Scr_AddInt(b[ofs]);
        break;
    case F_FLOAT:
        Scr_AddFloat(*(float *)&b[ofs]);
        break;
    case F_STRING:
        v4 = *(unsigned __int16 *)&b[ofs];
        if (*(_WORD *)&b[ofs])
            goto LABEL_21;
        break;
    case F_VECTOR:
        Scr_AddVector((const float *)&b[ofs]);
        break;
    case F_ENTITY:
        v5 = *(gentity_s **)&b[ofs];
        if (v5)
            Scr_AddEntity(v5);
        break;
    case F_ENTHANDLE:
        enthand = (EntHandle *)&b[ofs];
        if (enthand->isDefined())
        {
            Scr_AddEntity(enthand->ent());
        }
        break;
    case F_ACTOR:
    case F_SENTIENT:
        v8 = *(gentity_s ***)&b[ofs];
        if (v8)
            Scr_AddEntity(*v8);
        break;
    case F_SENTIENTHANDLE:
        senthand = (SentientHandle *)&b[ofs];
        if (senthand->isDefined())
        {
            Scr_AddEntity(senthand->sentient()->ent);
        }
        break;
    case F_PATHNODE:
        v11 = *(pathnode_t **)&b[ofs];
        if (v11)
            Scr_AddPathnode(v11);
        break;
    case F_VECTORHACK:
        v12[1] = *(float *)&b[ofs];
        v12[0] = 0.0;
        v12[2] = 0.0;
        Scr_AddVector(v12);
        break;
    case F_MODEL:
        v4 = G_ModelName(*(unsigned __int16 *)&b[ofs]);
    LABEL_21:
        Scr_AddConstString(v4);
        break;
    default:
        return;
    }
}

void __cdecl G_SpawnEntitiesFromString()
{
    if (!G_ParseSpawnVars(&level.spawnVar))
        Com_Error(ERR_DROP, "SpawnEntities: no entities");
    SP_worldspawn();
    while (G_ParseSpawnVars(&level.spawnVar))
        G_CallSpawn();
    G_ResetEntityParsePoint();
}

int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset)
{
    gentity_s *v4; // r29
    int number; // r8
    int v6; // r11
    actor_s *actor; // r3
    sentient_s *sentient; // r3
    gclient_s *client; // r3
    gentity_s *v11; // r3
    const ent_field_t *v12; // r11
    void(__cdecl * callback)(gentity_s *, int); // r10

    if (entnum >= 0x880)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            585,
            0,
            "%s",
            "(unsigned)entnum < MAX_GENTITIES");
    v4 = &g_entities[entnum];
    number = v4->s.number;
    if (entnum != number)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            588,
            0,
            "entnum == ent->s.number\n\t%i, %i",
            entnum,
            number);
    if (!v4->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            589,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            v4->s.number);
    v6 = offset & 0xC000;
    switch (v6)
    {
    case 32768:
        actor = v4->actor;
        if (!actor)
            return 0;
        Scr_SetActorField(actor, offset & 0xFFFF3FFF);
        return 1;
    case 16384:
        sentient = v4->sentient;
        if (!sentient)
            return 0;
        Scr_SetSentientField(sentient, offset & 0xFFFF3FFF);
        return 1;
    case 49152:
        client = v4->client;
        if (!client)
            return 0;
        Scr_SetClientField(client, offset & 0xFFFF3FFF);
        return 1;
    default:
        if (offset >= 0xF)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
                618,
                0,
                "%s",
                "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
        v11 = &g_entities[entnum];
        v12 = &fields_1[offset];
        callback = v12->callback;
        if (callback)
            callback(v11, offset);
        else
            Scr_SetGenericField((unsigned char*)&v11->s.eType, v12->type, v12->ofs);
        return 1;
    }
}

int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, int offset)
{
    int result; // r3
    const char *v4; // r3
    const char *v5; // r3

    switch (classnum)
    {
    case 0u:
        result = Scr_SetEntityField(entnum, offset);
        break;
    case 1u:
        Scr_SetHudElemField(entnum, offset);
        result = 1;
        break;
    case 2u:
        Scr_SetPathnodeField(entnum, offset);
        result = 1;
        break;
    case 3u:
        v4 = va("vehicle node is read-only", offset);
        Scr_Error(v4);
        result = 1;
        break;
    default:
        if (!alwaysfails)
        {
            v5 = va("bad class num %u", classnum);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 715, 0, v5);
        }
        result = 1;
        break;
    }
    return result;
}

void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset)
{
    gentity_s *v4; // r30
    int number; // r8
    int v6; // r11
    actor_s *actor; // r3
    sentient_s *sentient; // r3
    gclient_s *client; // r3

    if (entnum >= 0x880)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            728,
            0,
            "%s",
            "(unsigned)entnum < MAX_GENTITIES");
    v4 = &g_entities[entnum];
    number = v4->s.number;
    if (entnum != number)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            731,
            0,
            "entnum == ent->s.number\n\t%i, %i",
            entnum,
            number);
    if (!v4->r.inuse)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
            732,
            0,
            "%s\n\t(ent->s.number) = %i",
            "(ent->r.inuse)",
            v4->s.number);
    v6 = offset & 0xC000;
    switch (v6)
    {
    case 32768:
        actor = v4->actor;
        if (actor)
            Scr_GetActorField(actor, offset & 0xFFFF3FFF);
        break;
    case 16384:
        sentient = v4->sentient;
        if (sentient)
            Scr_GetSentientField(sentient, offset & 0xFFFF3FFF);
        break;
    case 49152:
        client = v4->client;
        if (client)
            Scr_GetClientField(client, offset & 0xFFFF3FFF);
        break;
    default:
        if (offset >= 0xF)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp",
                761,
                0,
                "%s",
                "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
        Scr_GetGenericField((unsigned char*)&g_entities[entnum].s.eType, fields_1[offset].type, fields_1[offset].ofs);
        break;
    }
}

void __cdecl Scr_GetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    const char *v3; // r3

    switch (classnum)
    {
    case 0u:
        Scr_GetEntityField(entnum, offset);
        break;
    case 1u:
        Scr_GetHudElemField(entnum, offset);
        break;
    case 2u:
        Scr_GetPathnodeField(entnum, offset);
        break;
    case 3u:
        GScr_GetVehicleNodeField(entnum, offset);
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("bad class num %u", classnum);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_spawn.cpp", 852, 0, v3);
        }
        break;
    }
}

