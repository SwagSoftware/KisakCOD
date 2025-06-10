#include "g_public_mp.h"

#include <game_mp/g_utils_mp.h>

#include <script/scr_const.h>
#include <script/scr_vm.h>

#include <server/sv_game.h>
#include <server/sv_world.h>

int __cdecl G_LevelSpawnString(const char *key, const char *defaultString, const char **out)
{
    return G_SpawnString(&level.spawnVar, key, defaultString, out);
}

int __cdecl G_SpawnFloat(const char *key, const char *defaultString, float *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_LevelSpawnString(key, defaultString, &s);
    *out = atof(s);
    return present;
}

int __cdecl G_SpawnInt(const char *key, const char *defaultString, int *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_LevelSpawnString(key, defaultString, &s);
    *out = atoi(s);
    return present;
}

void __cdecl Scr_ReadOnlyField(gentity_s *ent, int i)
{
    Scr_Error("Tried to set a read only entity field");
}

const SpawnFuncEntry s_bspOrDynamicSpawns[6] =
{
  { "info_notnull", &SP_info_notnull },
  { "info_notnull_big", &SP_info_notnull },
  { "trigger_radius", &SP_trigger_radius },
  { "script_model", &SP_script_model },
  { "script_origin", &SP_script_origin },
  { "script_vehicle_collmap", &G_VehCollmapSpawner }
}; // idb
const SpawnFuncEntry s_bspOnlySpawns[14] =
{
  { "trigger_use", &trigger_use },
  { "trigger_use_touch", &trigger_use },
  { "trigger_multiple", &SP_trigger_multiple },
  { "trigger_disk", &SP_trigger_disk },
  { "trigger_hurt", &SP_trigger_hurt },
  { "trigger_once", &SP_trigger_once },
  { "trigger_damage", &SP_trigger_damage },
  { "trigger_lookat", &SP_trigger_lookat },
  { "light", &SP_light },
  { "misc_mg42", &SP_turret },
  { "misc_turret", &SP_turret },
  { "script_brushmodel", &SP_script_brushmodel },
  { "script_struct", &G_FreeEntity },
  { "script_vehicle_mp", &G_VehSpawner }
}; // idb

int __cdecl G_CallSpawnEntity(gentity_s *ent)
{
    const gitem_s *item; // [esp+0h] [ebp-Ch]
    void(__cdecl * spawnFunc)(gentity_s *); // [esp+4h] [ebp-8h]
    char *classname; // [esp+8h] [ebp-4h]

    if (level.spawnVar.spawnVarsValid)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 489, 1, "%s", "!level.spawnVar.spawnVarsValid");
    if (ent->classname)
    {
        classname = SL_ConvertToString(ent->classname);
        item = G_GetItemForClassname(classname, 0);
        if (item)
        {
            G_SpawnItem(ent, item);
            return 1;
        }
        else
        {
            spawnFunc = G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, 6);
            if (spawnFunc)
            {
                if (spawnFunc == G_FreeEntity)
                    MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 513, 0, "%s", "spawnFunc != G_FreeEntity");
                spawnFunc(ent);
                if (!ent->r.inuse)
                    MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 515, 0, "%s", "ent->r.inuse");
                return 1;
            }
            else
            {
                Com_Printf(15, "%s cannot be spawned dynamically\n", classname);
                return 0;
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawnEntity: NULL classname\n");
        return 0;
    }
}

const gitem_s *__cdecl G_GetItemForClassname(const char *classname, unsigned __int8 model)
{
    int weapIndex; // [esp+0h] [ebp-8h]

    if (strncmp(classname, "weapon_", 7u))
        return 0;
    weapIndex = G_GetWeaponIndexForName(classname + 7);
    if (!weapIndex)
        return 0;
    BG_GetWeaponDef(weapIndex);
    return BG_FindItemForWeapon(weapIndex, model);
}

void(__cdecl *__cdecl G_FindSpawnFunc(
    const char *classname,
    const SpawnFuncEntry *spawnFuncArray,
    int spawnFuncCount))(gentity_s *)
{
    int spawnFuncIter; // [esp+14h] [ebp-4h]

    for (spawnFuncIter = 0; spawnFuncIter < spawnFuncCount; ++spawnFuncIter)
    {
        if (!strcmp(classname, spawnFuncArray[spawnFuncIter].classname))
            return spawnFuncArray[spawnFuncIter].callback;
    }
    return 0;
}

struct ent_field_t // sizeof=0x10
{                                       // ...
    const char *name;
    int ofs;
    fieldtype_t type;
    void(__cdecl *callback)(gentity_s *, int);
};

const ent_field_t fields_1[11] =
{
  { "classname", 368, F_STRING, &Scr_ReadOnlyField },
  { "origin", 316, F_VECTOR, &Scr_SetOrigin },
  { "model", 360, F_MODEL, &Scr_ReadOnlyField },
  { "spawnflags", 380, F_INT, &Scr_ReadOnlyField },
  { "target", 370, F_STRING, NULL },
  { "targetname", 372, F_STRING, NULL },
  { "count", 428, F_INT, NULL },
  { "health", 416, F_INT, &Scr_SetHealth },
  { "dmg", 424, F_INT, NULL },
  { "angles", 328, F_VECTOR, &Scr_SetAngles },
  { NULL, 0, F_INT, NULL }
}; // idb

void __cdecl GScr_AddFieldsForEntity()
{
    const ent_field_t *f; // [esp+0h] [ebp-4h]

    for (f = fields_1; f->name; ++f)
    {
        if (((f - fields_1) & 0xC000) != 0)
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 531, 0, "%s", "((f - fields) & ENTFIELD_MASK) == ENTFIELD_ENTITY");
        if (f - fields_1 != (unsigned __int16)(f - fields_1))
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 532, 0, "%s", "(f - fields) == (unsigned short)( f - fields )");
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - fields_1));
    }
    GScr_AddFieldsForClient();
}

void __cdecl GScr_AddFieldsForRadiant()
{
    Scr_AddFields("radiant", "txt");
}

void __cdecl Scr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs)
{
    VariableUnion v3; // eax
    gentity_s *EntityAllowNull; // eax
    float vec[3]; // [esp+4h] [ebp-Ch] BYREF

    switch (type)
    {
    case F_INT:
        *(VariableUnion *)&b[ofs] = Scr_GetInt(0);
        break;
    case F_FLOAT:
        *(float *)&b[ofs] = Scr_GetFloat(0);
        break;
    case F_STRING:
        v3.intValue = Scr_GetConstStringIncludeNull(0);
        Scr_SetString((unsigned __int16 *)&b[ofs], v3.stringValue);
        break;
    case F_VECTOR:
        Scr_GetVector(0, vec);
        *(float *)&b[ofs] = vec[0];
        *(float *)&b[ofs + 4] = vec[1];
        *(float *)&b[ofs + 8] = vec[2];
        break;
    case F_ENTITY:
        *(unsigned int *)&b[ofs] = (unsigned int)Scr_GetEntityAllowNull(0);
        break;
    case F_ENTHANDLE:
        EntityAllowNull = Scr_GetEntityAllowNull(0);
        EntHandle::setEnt((EntHandle *)&b[ofs], EntityAllowNull);
        break;
    case F_VECTORHACK:
        Scr_GetVector(0, vec);
        *(float *)&b[ofs] = vec[1];
        break;
    default:
        return;
    }
}

int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    const char *v4; // eax

    if (!classnum)
        return Scr_SetEntityField(entnum, offset);
    if (classnum == 1)
    {
        Scr_SetHudElemField(entnum, offset);
    }
    else if (!alwaysfails)
    {
        v4 = va("bad class num %u", classnum);
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 646, 0, v4);
    }
    return 1;
}

int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset)
{
    const ent_field_t *f; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if (entnum >= 0x400)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 561, 0, "%s", "(unsigned)entnum < MAX_GENTITIES");
    ent = &g_entities[entnum];
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 564, 0, "%s\n\t(ent->s.number) = %i", "(ent->r.inuse)", ent->s.number);
    if ((offset & 0xC000) == 0xC000)
    {
        if (ent->client)
        {
            Scr_SetClientField(ent->client, offset & 0xFFFF3FFF);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (offset >= 0xA)
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 576, 0, "%s", "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
        f = &fields_1[offset];
        if (f->callback)
            f->callback(ent, offset);
        else
            Scr_SetGenericField((unsigned __int8 *)ent, f->type, f->ofs);
        return 1;
    }
}

void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if (entnum >= 0x400)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 664, 0, "%s", "(unsigned)entnum < MAX_GENTITIES");
    ent = &g_entities[entnum];
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 667, 0, "%s\n\t(ent->s.number) = %i", "(ent->r.inuse)", ent->s.number);
    if ((offset & 0xC000) == 0xC000)
    {
        if (ent->client)
            Scr_GetClientField(ent->client, offset & 0xFFFF3FFF);
    }
    else
    {
        if (offset >= 0xA)
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 679, 0, "%s", "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
        Scr_GetGenericField((unsigned __int8 *)ent, fields_1[offset].type, fields_1[offset].ofs);
    }
}

void __cdecl Scr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs)
{
    gentity_s *v3; // eax
    unsigned int value; // eax
    unsigned __int16 str; // [esp+8h] [ebp-18h]
    float vec[3]; // [esp+10h] [ebp-10h] BYREF
    unsigned __int16 id; // [esp+1Ch] [ebp-4h]

    switch (type)
    {
    case F_INT:
        Scr_AddInt(*(unsigned int *)&b[ofs]);
        break;
    case F_FLOAT:
        Scr_AddFloat(*(float *)&b[ofs]);
        break;
    case F_LSTRING:
        Scr_AddString((char *)&b[ofs]);
        break;
    case F_STRING:
        str = *(_WORD *)&b[ofs];
        if (str)
            Scr_AddConstString(str);
        break;
    case F_VECTOR:
        Scr_AddVector((float *)&b[ofs]);
        break;
    case F_ENTITY:
        if (*(unsigned int *)&b[ofs])
            Scr_AddEntity(*(gentity_s **)&b[ofs]);
        break;
    case F_ENTHANDLE:
        if (EntHandle::isDefined((EntHandle *)&b[ofs]))
        {
            v3 = EntHandle::ent((EntHandle *)&b[ofs]);
            Scr_AddEntity(v3);
        }
        break;
    case F_VECTORHACK:
        vec[0] = 0.0;
        vec[1] = *(float *)&b[ofs];
        vec[2] = 0.0;
        Scr_AddVector(vec);
        break;
    case F_OBJECT:
        id = *(_WORD *)&b[ofs];
        if (id)
            Scr_AddObject(id);
        break;
    case F_MODEL:
        value = G_ModelName(*(unsigned __int16 *)&b[ofs]);
        Scr_AddConstString(value);
        break;
    default:
        return;
    }
}

void __cdecl Scr_FreeEntityConstStrings(gentity_s *pEnt)
{
    const ent_field_t *f; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (f = fields_1; f->name; ++f)
    {
        if (f->type == F_STRING)
            Scr_SetString((unsigned __int16 *)((char *)pEnt + f->ofs), 0);
    }
    for (i = 0; i < 19; ++i)
    {
        pEnt->attachModelNames[i] = 0;
        Scr_SetString(&pEnt->attachTagNames[i], 0);
    }
}

void __cdecl Scr_FreeEntity(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 803, 0, "%s", "ent");
    if (ent->s.number != ent - g_entities)
        MyAssertHandler(
            ".\\game_mp\\g_spawn_mp.cpp",
            804,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            ent->s.number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 805, 0, "%s\n\t(ent->s.number) = %i", "(ent->r.inuse)", ent->s.number);
    Scr_FreeEntityConstStrings(ent);
    Scr_FreeEntityNum(ent->s.number, 0);
}

void __cdecl Scr_AddEntity(gentity_s *ent)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 819, 0, "%s", "ent");
    if (ent->s.number != ent - g_entities)
        MyAssertHandler(
            ".\\game_mp\\g_spawn_mp.cpp",
            820,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            ent->s.number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 821, 0, "%s\n\t(ent->s.number) = %i", "(ent->r.inuse)", ent->s.number);
    Scr_AddEntityNum(ent->s.number, 0);
}

gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index)
{
    scr_entref_t entref; // [esp+4h] [ebp-8h]

    if (!Scr_GetType(index))
        return 0;
    entref = Scr_GetEntityRef(index);
    if (entref.classnum)
        return 0;
    if (entref.entnum >= 0x400u)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 844, 0, "%s", "entref.entnum < MAX_GENTITIES");
    return &g_entities[entref.entnum];
}

gentity_s *__cdecl Scr_GetEntity(unsigned int index)
{
    scr_entref_t entref; // [esp+4h] [ebp-4h]

    entref = Scr_GetEntityRef(index);
    if (entref.classnum)
    {
        Scr_ParamError(index, "not an entity");
        return 0;
    }
    else
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 864, 0, "%s", "entref.entnum < MAX_GENTITIES");
        return &g_entities[entref.entnum];
    }
}

void __cdecl Scr_FreeHudElem(game_hudelem_s *hud)
{
    if (!hud)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 881, 0, "%s", "hud");
    if ((unsigned int)(hud - g_hudelems) >= 0x400)
        MyAssertHandler(
            ".\\game_mp\\g_spawn_mp.cpp",
            882,
            0,
            "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
            hud - g_hudelems,
            1024);
    if (hud->elem.type == HE_TYPE_FREE)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 883, 0, "%s", "hud->elem.type != HE_TYPE_FREE");
    Scr_NotifyNum(hud - g_hudelems, 1u, scr_const.death, 0);
    Scr_FreeHudElemConstStrings(hud);
    Scr_FreeEntityNum(hud - g_hudelems, 1u);
}

void __cdecl Scr_AddHudElem(game_hudelem_s *hud)
{
    if (!hud)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 902, 0, "%s", "hud");
    if ((unsigned int)(hud - g_hudelems) >= 0x400)
        MyAssertHandler(
            ".\\game_mp\\g_spawn_mp.cpp",
            903,
            0,
            "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
            hud - g_hudelems,
            1024);
    if (hud->elem.type == HE_TYPE_FREE)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 904, 0, "%s", "hud->elem.type != HE_TYPE_FREE");
    Scr_AddEntityNum(hud - g_hudelems, 1u);
}

unsigned __int16 __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount)
{
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 937, 0, "%s", "ent");
    if (ent->s.number != ent - g_entities)
        MyAssertHandler(
            ".\\game_mp\\g_spawn_mp.cpp",
            938,
            0,
            "ent->s.number == ent - g_entities\n\t%i, %i",
            ent->s.number,
            ent - g_entities);
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 939, 0, "%s\n\t(ent->s.number) = %i", "(ent->r.inuse)", ent->s.number);
    return Scr_ExecEntThreadNum(ent->s.number, 0, handle, paramcount);
}

void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount)
{
    char *v3; // eax
    const char *v4; // eax

    if (!ent)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 967, 0, "%s", "ent");
    if (ent->s.number != ent - g_entities)
    {
        v3 = SL_ConvertToString(stringValue);
        v4 = va("s.number = %i, index = %i, stringValue = %s", ent->s.number, ent - g_entities, v3);
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 968, 0, "%s\n\t%s", "ent->s.number == ent - g_entities", v4);
    }
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 969, 0, "%s\n\t(ent->s.number) = %i", "(ent->r.inuse)", ent->s.number);
    Scr_NotifyNum(ent->s.number, 0, stringValue, paramcount);
}

void __cdecl Scr_GetEnt()
{
    gentity_s *result; // [esp+0h] [ebp-24h]
    unsigned __int16 name; // [esp+8h] [ebp-1Ch]
    int offset; // [esp+Ch] [ebp-18h]
    char *key; // [esp+10h] [ebp-14h]
    gentity_s *ent; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]
    unsigned __int16 value; // [esp+20h] [ebp-4h]

    name = Scr_GetConstString(0);
    key = Scr_GetString(1u);
    offset = Scr_GetOffset(0, key);
    if (offset >= 0)
    {
        if (offset >= 10)
            MyAssertHandler(
                ".\\game_mp\\g_spawn_mp.cpp",
                999,
                0,
                "%s",
                "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( fields ) - 1 )");
        if (fields_1[offset].type == F_STRING)
        {
            result = 0;
            i = 0;
            ent = g_entities;
            while (i < level.num_entities)
            {
                if (ent->r.inuse)
                {
                    value = *(_WORD *)((char *)&ent->s.number + fields_1[offset].ofs);
                    if (value)
                    {
                        if (value == name)
                        {
                            if (result)
                                Scr_Error("getent used with more than one entity");
                            result = ent;
                        }
                    }
                }
                ++i;
                ++ent;
            }
            if (result)
                Scr_AddEntity(result);
        }
    }
}

void __cdecl Scr_GetEntArray()
{
    unsigned __int16 name; // [esp+4h] [ebp-1Ch]
    int offset; // [esp+8h] [ebp-18h]
    char *key; // [esp+Ch] [ebp-14h]
    gentity_s *ent; // [esp+14h] [ebp-Ch]
    gentity_s *enta; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    int ia; // [esp+18h] [ebp-8h]
    unsigned __int16 value; // [esp+1Ch] [ebp-4h]

    if (Scr_GetNumParam())
    {
        name = Scr_GetConstString(0);
        key = Scr_GetString(1u);
        offset = Scr_GetOffset(0, key);
        if (offset >= 0)
        {
            if (offset >= 10)
                MyAssertHandler(
                    ".\\game_mp\\g_spawn_mp.cpp",
                    1064,
                    0,
                    "%s",
                    "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( fields ) - 1 )");
            if (fields_1[offset].type == F_STRING)
            {
                Scr_MakeArray();
                ia = 0;
                enta = g_entities;
                while (ia < level.num_entities)
                {
                    if (enta->r.inuse)
                    {
                        value = *(_WORD *)((char *)&enta->s.number + fields_1[offset].ofs);
                        if (value)
                        {
                            if (value == name)
                            {
                                Scr_AddEntity(enta);
                                Scr_AddArray();
                            }
                        }
                    }
                    ++ia;
                    ++enta;
                }
            }
        }
    }
    else
    {
        Scr_MakeArray();
        i = 0;
        ent = g_entities;
        while (i < level.num_entities)
        {
            if (ent->r.inuse)
            {
                Scr_AddEntity(ent);
                Scr_AddArray();
            }
            ++i;
            ++ent;
        }
    }
}

void __cdecl SP_worldspawn()
{
    char *v0; // eax
    float v1; // [esp+4h] [ebp-14h]
    float value; // [esp+8h] [ebp-10h]
    float v3; // [esp+10h] [ebp-8h]
    const char *s; // [esp+14h] [ebp-4h] BYREF

    G_LevelSpawnString("classname", "", &s);
    if (I_stricmp(s, "worldspawn"))
        Com_Error(ERR_DROP, "SP_worldspawn: The first entity isn't worldspawn");
    SV_SetConfigstring(2, (char*)"cod");
    G_LevelSpawnString("ambienttrack", "", &s);
    if (*s)
    {
        v0 = va("n\\%s", s);
        SV_SetConfigstring(821, v0);
    }
    else
    {
        SV_SetConfigstring(821, (char *)"");
    }
    G_LevelSpawnString("message", "", &s);
    SV_SetConfigstring(3, (char *)s);
    SV_SetConfigstring(10, (char *)g_motd->current.integer);
    G_LevelSpawnString("gravity", "800", &s);
    if (!g_gravity)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 1123, 0, "%s", "g_gravity");
    value = atof(s);
    Dvar_SetFloat((dvar_s *)g_gravity, value);
    G_LevelSpawnString("northyaw", "", &s);
    if (*s)
    {
        SV_SetConfigstring(822, (char *)s);
        v1 = atof(s);
        v3 = v1 * 0.01745329238474369;
        level.compassNorth[0] = cos(v3);
        level.compassNorth[1] = sin(v3);
    }
    else
    {
        SV_SetConfigstring(822, (char*)"0");
        level.compassNorth[0] = 1.0;
        level.compassNorth[1] = 0.0;
    }
    G_LevelSpawnString("spawnflags", "0", &s);
    g_entities[1022].spawnflags = atoi(s);
    g_entities[1022].s.number = 1022;
    Scr_SetString(&g_entities[1022].classname, scr_const.worldspawn);
    g_entities[1022].r.inuse = 1;
    if (EntHandle::isDefined(&g_entities[1022].r.ownerNum))
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 1145, 0, "%s", "!g_entities[ENTITYNUM_WORLD].r.ownerNum.isDefined()");
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

void G_CallSpawn()
{
    const gitem_s *item; // [esp+0h] [ebp-10h]
    void(__cdecl * spawnFunc)(gentity_s *); // [esp+4h] [ebp-Ch]
    const char *classname; // [esp+8h] [ebp-8h] BYREF
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 437, 1, "%s", "level.spawnVar.spawnVarsValid");
    G_LevelSpawnString("classname", "", &classname);
    if (classname)
    {
        if (strncmp(classname, "dyn_", 4u))
        {
            item = G_GetItemForClassname(classname, 0);
            if (item)
            {
                ent = G_Spawn();
                G_ParseEntityFields(ent);
                G_SpawnItem(ent, item);
            }
            else
            {
                spawnFunc = G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, 6);
                if (!spawnFunc)
                    spawnFunc = G_FindSpawnFunc(classname, s_bspOnlySpawns, 14);
                if (spawnFunc != G_FreeEntity)
                {
                    ent = G_Spawn();
                    G_ParseEntityFields(ent);
                    if (spawnFunc)
                        spawnFunc(ent);
                }
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawn: NULL classname\n");
    }
}

void __cdecl G_ParseEntityFields(gentity_s *ent)
{
    int i; // [esp+0h] [ebp-4h]

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 293, 0, "%s", "level.spawnVar.spawnVarsValid");
    for (i = 0; i < level.spawnVar.numSpawnVars; ++i)
        G_ParseEntityField(level.spawnVar.spawnVars[i][0], level.spawnVar.spawnVars[i][1], ent);
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
}

void __cdecl G_ParseEntityField(const char *key, char *value, gentity_s *ent)
{
    float v3; // [esp+0h] [ebp-20h]
    const ent_field_t *f; // [esp+Ch] [ebp-14h]
    float vec[3]; // [esp+10h] [ebp-10h] BYREF
    int modelIndex; // [esp+1Ch] [ebp-4h]

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
        *(int *)((char *)&ent->s.number + f->ofs) = atoi(value);
        break;
    case F_FLOAT:
        v3 = atof(value);
        *(float *)((char *)&ent->s.number + f->ofs) = v3;
        break;
    case F_STRING:
        Scr_SetString((unsigned __int16 *)((char *)ent + f->ofs), 0);
        *(_WORD *)((char *)&ent->s.number + f->ofs) = G_NewString(value).prev;
        break;
    case F_VECTOR:
        vec[0] = 0.0;
        vec[1] = 0.0;
        vec[2] = 0.0;
        sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
        *(float *)((char *)&ent->s.number + f->ofs) = vec[0];
        *(float *)((char *)&ent->s.eType + f->ofs) = vec[1];
        *(float *)((char *)&ent->s.lerp.eFlags + f->ofs) = vec[2];
        break;
    case F_MODEL:
        if (*value == 42)
        {
            modelIndex = atoi(value + 1);
            if (modelIndex != (unsigned __int16)modelIndex)
                MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 262, 0, "%s", "modelIndex == (modelNameIndex_t)modelIndex");
            ent->s.index.brushmodel = (unsigned __int16)modelIndex;
        }
        else
        {
            if (Com_IsLegacyXModelName(value))
                value += 7;
            G_SetModel(ent, value);
        }
        break;
    default:
        return;
    }
}

void __cdecl G_SetEntityScriptVariable(const char *key, char *value, gentity_s *ent)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    index = G_SetEntityScriptVariableInternal(key, value);
    if (index)
        GScr_SetDynamicEntityField(ent, index);
}

unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, char *value)
{
    unsigned int index; // [esp+Ch] [ebp-14h]
    int type; // [esp+10h] [ebp-10h] BYREF
    float vec[3]; // [esp+14h] [ebp-Ch] BYREF

    index = Scr_FindField(key, &type);
    if (!index)
        return 0;
    switch (type)
    {
    case 2:
        Scr_AddString(value);
        break;
    case 4:
        vec[0] = 0.0;
        vec[1] = 0.0;
        vec[2] = 0.0;
        sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
        Scr_AddVector(vec);
        break;
    case 5:
        Scr_AddFloat(atof(value));
        break;
    case 6:
        Scr_AddInt(atoi(value));
        break;
    default:
        if (!alwaysfails)
        {
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 191, 0, va("G_SetEntityScriptVariableInternal: bad case %d", type));
        }
        break;
    }
    return index;
}

void __cdecl G_LoadStructs()
{
    unsigned __int16 hThread; // [esp+14h] [ebp-8h]
    const char *classname; // [esp+18h] [ebp-4h] BYREF

    if (!g_scr_data.initstructs)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 1185, 0, "%s", "g_scr_data.initstructs");
    hThread = Scr_ExecThread(g_scr_data.initstructs, 0);
    Scr_FreeThread(hThread);
    while (G_ParseSpawnVars(&level.spawnVar))
    {
        G_LevelSpawnString("classname", "", &classname);
        if (!strcmp("script_struct", classname))
            G_SpawnStruct();
    }
    G_ResetEntityParsePoint();
}

int G_SpawnStruct()
{
    int result; // eax
    unsigned int index; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    unsigned int structId; // [esp+8h] [ebp-4h]

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 315, 0, "%s", "level.spawnVar.spawnVarsValid");
    if (!g_scr_data.createstruct)
        MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 317, 0, "%s", "g_scr_data.createstruct");
    Scr_AddExecThread(g_scr_data.createstruct, 0);
    structId = Scr_GetObject(0);
    for (i = 0; ; ++i)
    {
        result = i;
        if (i >= level.spawnVar.numSpawnVars)
            break;
        index = G_SetEntityScriptVariableInternal(level.spawnVar.spawnVars[i][0], level.spawnVar.spawnVars[i][1]);
        if (index)
            Scr_SetStructField(structId, index);
    }
    return result;
}

