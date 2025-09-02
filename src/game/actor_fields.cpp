#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_fields.h"
#include "actor.h"
#include <script/scr_vm.h>
#include "g_local.h"
#include <universal/surfaceflags.h>
#include <script/scr_const.h>
#include <qcommon/cmd.h>
#include "g_main.h"

const actor_fields_s aifields[82] =
{
  { "type", 8, F_INT, &ActorScr_SetSpecies, &ActorScr_GetSpecies },
  { "accuracy", 188, F_FLOAT, &ActorScr_Clamp_0_Positive, NULL },
  { "lookforward", 228, F_VECTOR, &ActorScr_ReadOnly, NULL },
  { "lookright", 240, F_VECTOR, &ActorScr_ReadOnly, NULL },
  { "lookup", 252, F_VECTOR, &ActorScr_ReadOnly, NULL },
  { "fovcosine", 2088, F_FLOAT, &ActorScr_Clamp_0_1, NULL },
  { "maxsightdistsqrd", 2092, F_FLOAT, NULL, NULL },
  { "ignoreclosefoliage", 2096, F_INT, NULL, NULL },
  { "followmin", 1804, F_INT, NULL, NULL },
  { "followmax", 1808, F_INT, NULL, NULL },
  { "chainfallback", 1840, F_SHORT, NULL, NULL },
  { "interval", 1812, F_FLOAT, NULL, NULL },
  { "damagetaken", 468, F_INT, &ActorScr_ReadOnly, NULL },
  { "damagedir", 476, F_VECTOR, &ActorScr_ReadOnly, NULL },
  { "damageyaw", 472, F_INT, &ActorScr_ReadOnly, NULL },
  { "damagelocation", 488, F_STRING, &ActorScr_ReadOnly, NULL },
  { "damageweapon", 490, F_STRING, &ActorScr_ReadOnly, NULL },
  { "proneok", 332, F_INT, &ActorScr_ReadOnly, NULL },
  { "walkdist", 1792, F_FLOAT, NULL, NULL },
  { "desiredangle", 296, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { "pacifist", 2008, F_INT, NULL, NULL },
  { "pacifistwait", 2012, F_INT, &ActorScr_SetTime, &ActorScr_GetTime },
  { "ignoresuppression", 3564, F_INT, NULL, NULL },
  { "suppressionwait", 3568, F_INT, NULL, NULL },
  { "suppressionduration", 3572, F_INT, NULL, NULL },
  { "suppressionstarttime", 3576, F_INT, &ActorScr_ReadOnly, NULL },
  { "suppressionmeter", 3580, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { "name", 212, F_STRING, NULL, NULL },
  { "weapon", 214, F_STRING, NULL, NULL },
  { "dontavoidplayer", 1836, F_INT, NULL, NULL },
  { "grenadeawareness", 3604, F_FLOAT, &ActorScr_Clamp_0_1, NULL },
  { "grenade", 3608, F_ENTHANDLE, &ActorScr_ReadOnly, NULL },
  { "grenadeweapon", 3612, F_INT, &ActorScr_SetWeapon, &ActorScr_GetWeapon },
  { "grenadeammo", 3628, F_INT, NULL, NULL },
  { "favoriteenemy", 3420, F_SENTIENTHANDLE, NULL, NULL },
  { "allowpain", 184, F_BYTE, NULL, NULL },
  { "allowdeath", 185, F_BYTE, NULL, NULL },
  { "delayeddeath", 186, F_BYTE, &ActorScr_ReadOnly, NULL },
  { "providecoveringfire", 187, F_BYTE, NULL, NULL },
  { "useable", 3687, F_BYTE, NULL, NULL },
  { "ignoretriggers", 3688, F_BYTE, NULL, NULL },
  { "pushable", 3689, F_BYTE, NULL, NULL },
  { "dropweapon", 3668, F_INT, NULL, NULL },
  { "drawoncompass", 3672, F_INT, NULL, NULL },
  { "scriptstate", 3712, F_STRING, &ActorScr_ReadOnly, NULL },
  { "lastscriptstate", 3714, F_STRING, &ActorScr_ReadOnly, NULL },
  { "statechangereason", 3716, F_STRING, &ActorScr_ReadOnly, NULL },
  { "groundtype", 568, F_STRING, &ActorScr_ReadOnly, &ActorScr_GetGroundType },
  { "anim_pose", 318, F_STRING, &ActorScr_SetAnimPos, NULL },
  { "goalradius", 1920, F_FLOAT, &ActorScr_SetGoalRadius, NULL },
  { "goalheight", 1924, F_FLOAT, &ActorScr_SetGoalHeight, NULL },
  { "goalpos", 1876, F_VECTOR, &ActorScr_ReadOnly, NULL },
  { "ignoreforfixednodesafecheck", 1956, F_BYTE, NULL, NULL },
  { "fixednode", 1957, F_BYTE, &ActorScr_SetFixedNode, NULL },
  { "fixednodesaferadius", 1960, F_FLOAT, &ActorScr_Clamp_0_Positive, NULL },
  {
    "pathgoalpos",
    1704,
    F_VECTOR,
    &ActorScr_ReadOnly,
    &ActorScr_GetPathGoalPos
  },
  { "stopanimdistsq", 1784, F_FLOAT, NULL, NULL },
  {
    "lastenemysightpos",
    3428,
    F_VECTOR,
    &ActorScr_SetLastEnemySightPos,
    &ActorScr_GetLastEnemySightPos
  },
  { "pathenemylookahead", 1940, F_FLOAT, NULL, NULL },
  { "pathenemyfightdist", 1944, F_FLOAT, NULL, NULL },
  { "meleeattackdist", 1948, F_FLOAT, NULL, NULL },
  { "chainnode", 1972, F_PATHNODE, &ActorScr_ReadOnly, NULL },
  { "movemode", 516, F_STRING, &ActorScr_ReadOnly, &ActorScr_GetMoveMode },
  { "safetochangescript", 517, F_BYTE, NULL, NULL },
  { "keepclaimednode", 1848, F_BYTE, NULL, NULL },
  { "keepclaimednodeingoal", 1849, F_BYTE, NULL, NULL },
  { "keepnodeduringscriptedanim", 1850, F_BYTE, NULL, NULL },
  { "nododgemove", 1851, F_BYTE, NULL, NULL },
  { "leanamount", 1864, F_FLOAT, NULL, NULL },
  { "badplaceawareness", 3692, F_FLOAT, &ActorScr_Clamp_0_1, NULL },
  { "goodshootpos", 3696, F_VECTOR, NULL, NULL },
  { "goodshootposvalid", 3708, F_INT, NULL, NULL },
  { "flashbangimmunity", 3816, F_INT, NULL, NULL },
  { "lookaheaddir", 1716, F_VECTOR, &ActorScr_ReadOnly, NULL },
  { "exposedduration", 1872, F_INT, NULL, NULL },
  { "requestarrivalnotify", 1976, F_INT, NULL, NULL },
  { "engagemindist", 2068, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { "engageminfalloffdist", 2072, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { "engagemaxdist", 2076, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { "engagemaxfalloffdist", 2080, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { "finalaccuracy", 204, F_FLOAT, &ActorScr_ReadOnly, NULL },
  { NULL, 0, F_INT, NULL, NULL }
};

const actor_fields_s sentientfields[9] =
{
  { "threatbias", 8, F_INT, NULL, NULL },
  { "node", 88, F_PATHNODE, &ActorScr_ReadOnly, NULL },
  { "prevnode", 92, F_PATHNODE, &ActorScr_ReadOnly, NULL },
  { "enemy", 52, F_ENTHANDLE, &ActorScr_ReadOnly, NULL },
  { "syncedmeleetarget", 48, F_ENTHANDLE, NULL, NULL },
  { "ignoreme", 16, F_BYTE, NULL, NULL },
  { "ignoreall", 17, F_BYTE, NULL, NULL },
  { "maxvisibledist", 32, F_FLOAT, NULL, NULL },
  { NULL, 0, F_INT, NULL, NULL }
};

const actor_fields_s entfields[8] =
{
  { "health", 324, F_INT, NULL, NULL },
  { "maxhealth", 328, F_INT, NULL, NULL },
  { "targetname", 292, F_STRING, NULL, NULL },
  { "classname", 284, F_STRING, &ActorScr_ReadOnly, NULL },
  { "spawnflags", 300, F_INT, NULL, NULL },
  { "model", 280, F_MODEL, &ActorScr_ReadOnly, NULL },
  { "takedamage", 277, F_INT, NULL, NULL },
  { NULL, 0, F_INT, NULL, NULL }
};

actor_fields_s aifield_list = { 0 };
actor_fields_s aifield_delete = { 0 };

unsigned __int8 *__cdecl BaseForFields(unsigned __int8 *actor, const actor_fields_s *fields)
{
    if (fields != aifields)
    {
        if (fields == sentientfields)
        {
            return (unsigned __int8 *)*((unsigned int *)actor + 1);
        }
        else if (fields == entfields)
        {
            return *(unsigned __int8 **)actor;
        }
        else
        {
            if (!alwaysfails)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
                    209,
                    0,
                    "BaseForFields: invalid fields[]");
            Com_Error(ERR_DROP, "BaseForFields: invalid fields");
            return 0;
        }
    }
    return actor;
}

const actor_fields_s *__cdecl FindFieldForName(const actor_fields_s *fields, const char *pszFieldName)
{
    int v4; // r31
    const actor_fields_s *v5; // r11

    v4 = 0;
    if (!fields->name)
        return 0;
    v5 = fields;
    while (I_stricmp(pszFieldName, v5->name))
    {
        v5 = &fields[++v4];
        if (!v5->name)
            return 0;
    }
    return &fields[v4];
}

void __cdecl ActorScr_SetSpecies(actor_s *pSelf, const actor_fields_s *pField)
{
    unsigned int type; // [esp+4h] [ebp-4h]

    iassert(pSelf);
    type = Scr_GetConstString(0);
    for (int i = 0; i < MAX_AI_SPECIES; ++i)
    {
        if (type == *g_AISpeciesNames[i])
        {
            pSelf->species = (AISpecies)i;
            pSelf->ent->s.lerp.u.loopFx.period = (unsigned __int8)i;
            G_DObjUpdate(pSelf->ent);
            return;
        }
    }
    Scr_Error(va("unknown type '%s', should be human, dog, zombie, zombie_dog\n", SL_ConvertToString(type)));
}

void __cdecl ActorScr_GetSpecies(actor_s *pSelf, const actor_fields_s *pField)
{
    AISpecies species; // r7

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 269, 0, "%s", "pSelf");
    species = pSelf->species;
    if ((unsigned int)species >= MAX_AI_SPECIES)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            270,
            0,
            "pSelf->species doesn't index MAX_AI_SPECIES\n\t%i not in [0, %i)",
            species,
            2);
    Scr_AddConstString(*g_AISpeciesNames[pSelf->species]);
}

void __cdecl ActorScr_Clamp_0_1(actor_s *pSelf, const actor_fields_s *pField)
{
    double Float; // fp1
    double v5; // fp31
    const char *v6; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 283, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 288, 0, "%s", "pField");
    Float = Scr_GetFloat(0);
    v5 = 1.0;
    if (Float > 1.0)
    {
        v6 = va("actor field %s clamped from %g to 1\n", pField->name, Float);
    LABEL_9:
        Scr_Error(v6);
        Float = v5;
        goto LABEL_10;
    }
    v5 = 0.0;
    if (Float < 0.0)
    {
        v6 = va("actor field %s clamped from %g to 0\n", pField->name, Float);
        goto LABEL_9;
    }
LABEL_10:
    *(float *)((char *)&pSelf->ent + pField->ofs) = Float;
}

void __cdecl ActorScr_Clamp_0_Positive(actor_s *pSelf, const actor_fields_s *pField)
{
    double Float; // fp1
    const char *v5; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 316, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 321, 0, "%s", "pField");
    Float = Scr_GetFloat(0);
    if (Float < 0.0)
    {
        v5 = va("actor field %s clamped from %g to 0\n", pField->name, Float);
        Scr_Error(v5);
        Float = 0.0;
    }
    *(float *)((char *)&pSelf->ent + pField->ofs) = Float;
}

void __cdecl ActorScr_ReadOnly(actor_s *pSelf, const actor_fields_s *pField)
{
    const char *v3; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 344, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 347, 0, "%s", "pField");
    v3 = va("actor field %s is read-only", pField->name);
    Scr_Error(v3);
}

void __cdecl ActorScr_SetGoalRadius(actor_s *pSelf, const actor_fields_s *pField)
{
    double Float; // fp31

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 364, 0, "%s", "pSelf");
    Float = Scr_GetFloat(0);
    if (Float < 0.0)
        Scr_ParamError(0, "radius must be >= 0");
    Actor_SetGoalRadius(&pSelf->scriptGoal, Float);
}

void __cdecl ActorScr_SetGoalHeight(actor_s *pSelf, const actor_fields_s *pField)
{
    double Float; // fp1

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 382, 0, "%s", "pSelf");
    Float = Scr_GetFloat(0);
    Actor_SetGoalHeight(&pSelf->scriptGoal, Float);
}

void __cdecl ActorScr_SetTime(actor_s *pSelf, const actor_fields_s *pField)
{
    long double v4; // fp2
    long double v5; // fp2

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 396, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 399, 0, "%s", "pField");
    if (pField->type)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 401, 0, "%s", "pField->type == F_INT");
    *(double *)&v4 = (float)((float)(Scr_GetFloat(0) * (float)1000.0) + (float)0.5);
    v5 = floor(v4);
    *(gentity_s **)((char *)&pSelf->ent + pField->ofs) = (gentity_s *)(int)(float)*(double *)&v5;
}

void __cdecl ActorScr_GetTime(actor_s *pSelf, const actor_fields_s *pField)
{
    __int64 v2; // r11

    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 414, 0, "%s", "pField");
    if (pField->type)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 415, 0, "%s", "pField->type == F_INT");
    //LODWORD(v2) = *(gentity_s **)((char *)&pSelf->ent + pField->ofs);
    //Scr_AddFloat((float)((float)v2 * (float)0.001));
    Scr_AddFloat((float)*(int *)((char *)&pSelf->ent + pField->ofs) * 0.001);
}

void __cdecl ActorScr_SetWeapon(actor_s *pSelf, const actor_fields_s *pField)
{
    const char *String; // r31
    const char *v5; // r3
    const char *v6; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 430, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 433, 0, "%s", "pField");
    if (pField->type)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 435, 0, "%s", "pField->type == F_INT");
    String = Scr_GetString(0);
    if (!G_GetWeaponIndexForName(String))
    {
        v5 = va("Can't find weapon [%s].  It probably needs to be precached.", String);
        Scr_ParamError(0, v5);
    }
    v6 = Scr_GetString(0);
    *(gentity_s **)((char *)&pSelf->ent + pField->ofs) = (gentity_s *)G_GetWeaponIndexForName(v6);
}

void __cdecl ActorScr_GetWeapon(actor_s *pSelf, const actor_fields_s *pField)
{
    WeaponDef *WeaponDef; // r3

    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 454, 0, "%s", "pField");
    if (pField->type)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 455, 0, "%s", "pField->type == F_INT");
    WeaponDef = BG_GetWeaponDef(*(unsigned int *)((char *)&pSelf->ent + pField->ofs));
    if (WeaponDef)
        Scr_AddString(WeaponDef->szInternalName);
}

void __cdecl ActorScr_GetGroundType(actor_s *pSelf, const actor_fields_s *pField)
{
    int iSurfaceType; // r3
    const char *v5; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 472, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 473, 0, "%s", "pField");
    if (pField->type != F_STRING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 474, 0, "%s", "pField->type == F_STRING");
    if (pField->ofs != 568)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            475,
            0,
            "%s",
            "pField->ofs == AFOFS( Physics.iSurfaceType )");
    iSurfaceType = pSelf->Physics.iSurfaceType;
    if (iSurfaceType)
    {
        v5 = Com_SurfaceTypeToName(iSurfaceType);
        Scr_AddString(v5);
    }
}

void __cdecl ActorScr_SetAnimPos(actor_s *pSelf, const actor_fields_s *pField)
{
    unsigned int ConstString; // r30
    int IsProne; // r3
    const char *v6; // r28
    const char *v7; // r30
    const char *v8; // r3
    const char *v9; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 492, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 495, 0, "%s", "pField");
    ConstString = Scr_GetConstString(0);
    IsProne = BG_ActorGoalIsProne(&pSelf->ProneInfo);
    if ((scr_const.prone == ConstString) == IsProne)
    {
        Scr_SetString(&pSelf->anim_pose, ConstString);
    }
    else
    {
        if (IsProne)
            v6 = "ExitProne";
        else
            v6 = "EnterProne";
        v7 = SL_ConvertToString(ConstString);
        v8 = SL_ConvertToString(pSelf->anim_pose);
        v9 = va(
            "entnum %d is attempting to change anim_pose from \"%s\" to \"%s\" but %s was not called",
            pSelf->ent->s.number,
            v8,
            v7,
            v6);
        Scr_ErrorWithDialogMessage(v9, "");
    }
}

void __cdecl ActorScr_SetLastEnemySightPos(actor_s *pSelf, const actor_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 523, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 524, 0, "%s", "pField");
    if (pField->type != F_VECTOR)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 525, 0, "%s", "pField->type == F_VECTOR");
    if (pField->ofs != 3428)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            526,
            0,
            "%s",
            "pField->ofs == AFOFS( lastEnemySightPos )");
    if (Scr_GetType(0))
    {
        Scr_GetVector(0, pSelf->lastEnemySightPos);
        pSelf->lastEnemySightPosValid = 1;
    }
    else
    {
        pSelf->lastEnemySightPosValid = 0;
    }
}

void __cdecl ActorScr_GetLastEnemySightPos(actor_s *pSelf, const actor_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 546, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 547, 0, "%s", "pField");
    if (pField->type != F_VECTOR)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 548, 0, "%s", "pField->type == F_VECTOR");
    if (pField->ofs != 3428)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            549,
            0,
            "%s",
            "pField->ofs == AFOFS( lastEnemySightPos )");
    if (pSelf->lastEnemySightPosValid)
        Scr_AddVector(pSelf->lastEnemySightPos);
}

void __cdecl ActorScr_GetPathGoalPos(actor_s *self, const actor_fields_s *field)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 564, 0, "%s", "self");
    if (!field)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 565, 0, "%s", "field");
    if (field->type != F_VECTOR)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 566, 0, "%s", "field->type == F_VECTOR");
    if (field->ofs != 1704)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            567,
            0,
            "%s",
            "field->ofs == AFOFS( Path.vFinalGoal )");
    if (Actor_HasPath(self))
        Scr_AddVector(self->Path.vFinalGoal);
}

void __cdecl ActorScr_SetFixedNode(actor_s *self, const actor_fields_s *field)
{
    unsigned int Int; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 582, 0, "%s", "self");
    if (!field)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 583, 0, "%s", "field");
    if (field->type != F_BYTE)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 584, 0, "%s", "field->type == F_BYTE");
    Int = Scr_GetInt(0);
    self->exposedStartTime = 0x80000000;
    //self->fixedNode = (_cntlzw(Int) & 0x20) == 0;
    self->fixedNode = Int != 0;
}

void __cdecl ActorScr_GetMoveMode(actor_s *pSelf, const actor_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 598, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 599, 0, "%s", "pField");
    if (pField->type != F_STRING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 600, 0, "%s", "pField->type == F_STRING");
    if (pField->ofs != 516)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            601,
            0,
            "%s",
            "pField->ofs == AFOFS( moveMode )");
    switch (pSelf->moveMode)
    {
    case 0u:
        Scr_AddConstString(scr_const.stop);
        break;
    case 1u:
        Scr_AddConstString(scr_const.stop_soon);
        break;
    case 2u:
        Scr_AddConstString(scr_const.walk);
        break;
    case 3u:
        Scr_AddConstString(scr_const.run);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 622, 0, "unhandled");
        break;
    }
}

void __cdecl PrintFieldUsage(const actor_fields_s *fields)
{
    int v2; // r15
    const actor_fields_s *v3; // r10
    fieldtype_t type; // r4
    const char *v5; // r3

    v2 = 0;
    if (fields->name)
    {
        v3 = fields;
        do
        {
            type = v3->type;
            switch (type)
            {
            case F_INT:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "int");
                break;
            case F_SHORT:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "short");
                break;
            case F_BYTE:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "byte");
                break;
            case F_FLOAT:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "float");
                break;
            case F_STRING:
            case F_MODEL:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "string");
                break;
            case F_VECTOR:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "vector");
                break;
            case F_ENTITY:
            case F_ENTHANDLE:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "entnum");
                break;
            case F_ACTOR:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "actor");
                break;
            case F_SENTIENT:
            case F_SENTIENTHANDLE:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "sentient");
                break;
            case F_CLIENT:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "clientnum");
                break;
            case F_PATHNODE:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "pathnode");
                break;
            case F_ACTORGROUP:
                Com_Printf(0, "^5  %-20s: %s\n", v3->name, "actorgroup");
                break;
            default:
                if (!alwaysfails)
                {
                    v5 = va("Cmd_AI_f: unhandled field type %i\n", type);
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 685, 0, v5);
                }
                break;
            }
            v3 = &fields[++v2];
        } while (v3->name);
    }
}

void Cmd_AI_PrintUsage()
{
    Com_Printf(0, "^5USAGE: ai (!)target field (value), or ai (!) target [list/delete]\n");
    Com_Printf(
        0,
        "^5target can be an entity number, a targetname, an entity classname,\n    'all', 'axis', 'allies', or 'neutral'\n");
    Com_Printf(0, "^5if ! immediately precedes target, it uses AI that don't match target\n");
    Com_Printf(0, "^5field can be one of:\n");
    PrintFieldUsage(aifields);
    PrintFieldUsage(sentientfields);
    PrintFieldUsage(entfields);
}

void __cdecl Cmd_AI_DisplayInfo(actor_s *actor)
{
    const char *v2; // r31
    const char *v3; // r3
    const char *v4; // r3

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 720, 0, "%s", "actor");
    if (!actor->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 721, 0, "%s", "actor->ent");
    if (!actor->ent->classname)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 722, 0, "%s", "actor->ent->classname");
    if (!actor->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 723, 0, "%s", "actor->sentient");
    v2 = SL_ConvertToString(actor->ent->classname);
    v3 = Sentient_NameForTeam(actor->sentient->eTeam);
    Com_Printf(0, "ent %i (%-7s) %-24s", actor->ent->s.number, v3, v2);
    if (actor->ent->targetname)
    {
        v4 = SL_ConvertToString(actor->ent->targetname);
        Com_Printf(0, " targetname %s", v4);
    }
    Com_Printf(0, "\n");
}

void __cdecl Cmd_AI_Delete(actor_s *actor)
{
    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 742, 0, "%s", "actor");
    if (!actor->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 743, 0, "%s", "actor->ent");
    G_FreeEntityDelay(actor->ent);
}

void __cdecl Cmd_AI_DisplayValue(actor_s *pSelf, unsigned __int8 *pBase, const actor_fields_s *pField)
{
    int number; // r28
    __int64 v7; // r11
    fieldtype_t type; // r4
    double v9; // r7
    int ofs; // r11
    const char *v11; // r7
    int v12; // r10
    gentity_s *gentities; // r11
    unsigned int v14; // r10
    unsigned int v15; // r29
    gentity_s *v16; // r11
    const char *v17; // r8
    unsigned int v18; // r30
    gentity_s *v19; // r11
    unsigned int targetname; // r3
    const char *v21; // r8
    int v22; // r11
    gentity_s *v23; // r11
    gentity_s *v24; // r11
    const pathnode_t *v25; // r3
    int v26; // r3
    unsigned int v27; // r3
    const char *v28; // r3
    const char *v29; // r3

    SentientHandle *senthand;
    EntHandle *enthand;

    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 763, 0, "%s", "pField");
    number = pSelf->ent->s.number;
    if (pField->getter == ActorScr_GetTime)
    {
        if (pField->type)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 769, 0, "%s", "pField->type == F_INT");
        HIDWORD(v7) = pField->ofs;
        LODWORD(v7) = *(unsigned int *)&pBase[HIDWORD(v7)];
        Com_Printf(
            0,
            "ent %i: %s = %g\n",
            number,
            "swag", // KISAKTODO
            (float)((float)v7 * (float)0.001));
    }
    else
    {
        type = pField->type;
        switch (type)
        {
        case F_INT:
            Com_Printf(0, "ent %i: %s = %i\n", pSelf->ent->s.number, pField->name, *(unsigned int *)&pBase[pField->ofs]);
            return;
        case F_SHORT:
            Com_Printf(0, "ent %i: %s = %i\n", pSelf->ent->s.number, pField->name, *(__int16 *)&pBase[pField->ofs]);
            return;
        case F_BYTE:
            Com_Printf(0, "ent %i: %s = %i\n", pSelf->ent->s.number, pField->name, pBase[pField->ofs]);
            return;
        case F_FLOAT:
            v9 = *(float *)&pBase[pField->ofs];
            Com_Printf(0, "ent %i: %s = %g\n", pSelf->ent->s.number, (const char *)HIDWORD(v9), v9);
            return;
        case F_STRING:
            ofs = pField->ofs;
            if (*(_WORD *)&pBase[ofs])
                v11 = SL_ConvertToString(*(unsigned __int16 *)&pBase[ofs]);
            else
                v11 = "<undefined>";
            Com_Printf(0, "ent %i: %s = %s\n", number, pField->name, v11);
            return;
        case F_VECTOR:
            Com_Printf(
                0,
                "ent %i: %s = %g %g %g\n",
                pSelf->ent->s.number,
                *(float *)&pBase[pField->ofs],
                *(float *)&pBase[pField->ofs + 4],
                *(float *)&pBase[pField->ofs + 8]);
            return;
        case F_ENTITY:
            v12 = *(unsigned int *)&pBase[pField->ofs];
            if (!v12)
                goto LABEL_18;
            gentities = level.gentities;
            v14 = (int)((unsigned __int64)(875407347LL * (v12 - (unsigned int)level.gentities)) >> 32) >> 7;
            v15 = v14 + (v14 >> 31);
            if (v15 >= 0x880)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
                    809,
                    0,
                    "%s",
                    "i >= 0 && i < MAX_GENTITIES");
                gentities = level.gentities;
            }
            v16 = &gentities[v15];
            if (v16->targetname)
                v17 = SL_ConvertToString(v16->targetname);
            else
                v17 = "<undefined>";
            Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", number, pField->name, v15, v17);
            return;
        case F_ENTHANDLE:
            enthand = (EntHandle *)&pBase[pField->ofs];
            
            //if (!EntHandle::isDefined((EntHandle *)&pBase[pField->ofs]))
            if (!enthand->isDefined())
                goto LABEL_18;
            //v18 = EntHandle::entnum((EntHandle *)&pBase[pField->ofs]);
            v18 = enthand->entnum();
            if (v18 >= 0x880)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
                    823,
                    0,
                    "%s",
                    "i >= 0 && i < MAX_GENTITIES");
            v19 = &level.gentities[v18];
            targetname = v19->targetname;
            if (v19->targetname)
                goto LABEL_38;
            goto LABEL_29;
        case F_ACTOR:
            v22 = pField->ofs;
            if (*(unsigned int *)&pBase[v22])
                goto LABEL_32;
            goto LABEL_18;
        case F_SENTIENT:
            v22 = pField->ofs;
            if (!*(unsigned int *)&pBase[v22])
                goto LABEL_18;
        LABEL_32:
            v18 = *(unsigned __int16 *)(**(unsigned int **)&pBase[v22] + 118);
            v23 = &level.gentities[v18];
            targetname = v23->targetname;
            if (v23->targetname)
                goto LABEL_38;
            goto LABEL_29;
        case F_SENTIENTHANDLE:
            senthand = (SentientHandle *)&pBase[pField->ofs];
            //if (SentientHandle::isDefined((SentientHandle *)&pBase[pField->ofs]))
            if (senthand->isDefined())
            {
                //v18 = SentientHandle::sentient((SentientHandle *)&pBase[pField->ofs])->ent->s.number;
                v18 = senthand->sentient()->ent->s.number;
                v24 = &level.gentities[v18];
                targetname = v24->targetname;
                if (v24->targetname)
                    LABEL_38:
                v21 = SL_ConvertToString(targetname);
                else
                    LABEL_29:
                v21 = "<undefined>";
                Com_Printf(0, "ent %i: %s = %i (targetname %s)\n", number, pField->name, v18, v21);
            }
            else
            {
            LABEL_18:
                Com_Printf(0, "ent %i: %s = (null)\n", number, pField->name);
            }
            break;
        case F_CLIENT:
            Com_Printf(
                0,
                "ent %i: %s = client %i\n",
                pSelf->ent->s.number,
                pField->name,
                (signed int)(*(unsigned int *)&pBase[pField->ofs] - (unsigned int)level.clients) / 46104);
            return;
        case F_PATHNODE:
            v25 = *(const pathnode_t **)&pBase[pField->ofs];
            if (v25)
            {
                v26 = Path_ConvertNodeToIndex(v25);
                Com_Printf(0, "ent %i: %s = node %i\n", number, pField->name, v26);
            }
            else
            {
                Com_Printf(0, "ent %i: %s = (null)\n", pSelf->ent->s.number, pField->name);
            }
            return;
        case F_MODEL:
            v27 = G_ModelName(pBase[pField->ofs]);
            v28 = SL_ConvertToString(v27);
            Com_Printf(0, "ent %i: %s = %s\n", number, pField->name, v28);
            return;
        case F_ACTORGROUP:
            return;
        default:
            if (!alwaysfails)
            {
                v29 = va("Cmd_AI_f: unhandled field type %i for %s\n", type, pField->name);
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 888, 0, v29);
            }
            return;
        }
    }
}

void __cdecl Cmd_AI_SetValue(actor_s *pSelf, int argc, unsigned __int8 *pBase, const actor_fields_s *pField)
{
    void(__cdecl * setter)(actor_s *, const actor_fields_s *); // r11
    long double v9; // fp2
    long double v10; // fp2
    long double v11; // fp2
    fieldtype_t type; // r4
    long double v13; // fp2
    int v14; // r29
    int i; // r31
    long double v16; // fp2
    int ofs; // r10
    const char *v18; // r3
    _BYTE v19[24]; // [sp+58h] [-158h] BYREF
    char v20[320]; // [sp+70h] [-140h] BYREF

    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 908, 0, "%s", "pField");
    setter = pField->setter;
    if (setter == ActorScr_ReadOnly)
    {
        Com_PrintError(0, "%s is read-only\n", pField->name);
        return;
    }
    if (setter == ActorScr_SetTime)
    {
        if (pField->type)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 918, 0, "%s", "pField->type == F_INT");
        if (argc != 4)
            goto LABEL_9;
        SV_Cmd_ArgvBuffer(3, v20, 256);
        v9 = atof(v20);
        *(double *)&v9 = (float)((float)((float)*(double *)&v9 * (float)1000.0) + (float)0.5);
        v10 = floor(v9);
        *(unsigned int *)&pBase[pField->ofs] = (int)(float)*(double *)&v10;
    }
    else if (pField->getter == ActorScr_SetGoalRadius)
    {
        if (argc != 4)
        {
        LABEL_9:
            Cmd_AI_PrintUsage();
            return;
        }
        SV_Cmd_ArgvBuffer(3, v20, 256);
        v11 = atof(v20);
        Actor_SetGoalRadius(&pSelf->scriptGoal, (float)*(double *)&v11);
    }
    else
    {
        type = pField->type;
        switch (type)
        {
        case F_INT:
            if (argc != 4)
                goto LABEL_9;
            SV_Cmd_ArgvBuffer(3, v20, 256);
            *(unsigned int *)&pBase[pField->ofs] = atol(v20);
            break;
        case F_SHORT:
            if (argc != 4)
                goto LABEL_9;
            SV_Cmd_ArgvBuffer(3, v20, 256);
            *(_WORD *)&pBase[pField->ofs] = atol(v20);
            break;
        case F_BYTE:
            if (argc != 4)
                goto LABEL_9;
            SV_Cmd_ArgvBuffer(3, v20, 256);
            pBase[pField->ofs] = atol(v20);
            break;
        case F_FLOAT:
            if (argc != 4)
                goto LABEL_9;
            SV_Cmd_ArgvBuffer(3, v20, 256);
            v13 = atof(v20);
            *(float *)&pBase[pField->ofs] = *(double *)&v13;
            break;
        case F_STRING:
        case F_ENTITY:
        case F_ENTHANDLE:
        case F_ACTOR:
        case F_SENTIENT:
        case F_SENTIENTHANDLE:
        case F_CLIENT:
        case F_PATHNODE:
        case F_MODEL:
        case F_ACTORGROUP:
            Com_Printf(0, "cannot set from console\n");
            break;
        case F_VECTOR:
            if (argc != 6)
                goto LABEL_9;
            v14 = 0;
            for (i = 0; i < 12; i += 4)
            {
                SV_Cmd_ArgvBuffer(v14 + 3, v20, 256);
                v16 = atof(v20);
                ofs = pField->ofs;
                ++v14;
                *(float *)&v19[i] = *(double *)&v16;
                *(float *)&pBase[i + ofs] = *(double *)&v16;
            }
            break;
        default:
            if (!alwaysfails)
            {
                v18 = va("Cmd_AI_f: unhandled field type %i\n", type);
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1022, 0, v18);
            }
            break;
        }
    }
}

void __cdecl Cmd_AI_Dispatch(int argc, actor_s *pSelf, const actor_fields_s *fields, const actor_fields_s *pField)
{
    unsigned __int8 *v8; // r3
    unsigned __int8 *v9; // r3

    if (argc < 3)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1035, 0, "%s", "argc >= 3");
    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1036, 0, "%s", "pSelf");
    if (!pSelf->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1037, 0, "%s", "pSelf->ent");
    if (!pField)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1041, 0, "%s", "pField");
    if (pField == &aifield_list)
    {
        if (fields)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1046, 0, "%s", "fields == NULL");
        Cmd_AI_DisplayInfo(pSelf);
    }
    else if (pField == &aifield_delete)
    {
        if (fields)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1051, 0, "%s", "fields == NULL");
        Cmd_AI_Delete(pSelf);
    }
    else if (argc == 3)
    {
        if (!fields)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1056, 0, "%s", "fields != NULL");
        v8 = BaseForFields((unsigned __int8 *)pSelf, fields);
        Cmd_AI_DisplayValue(pSelf, v8, pField);
    }
    else
    {
        if (!fields)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1061, 0, "%s", "fields != NULL");
        v9 = BaseForFields((unsigned __int8 *)pSelf, fields);
        Cmd_AI_SetValue(pSelf, argc, v9, pField);
    }
}

void __cdecl Cmd_AI_EntityNumber(
    int argc,
    const actor_fields_s *fields,
    const actor_fields_s *pField,
    const char *szNum,
    int bInvertSelection)
{
    unsigned int v9; // r3
    unsigned int v10; // r30
    actor_s *i; // r31
    actor_s *actor; // r4

    v9 = atol(szNum);
    v10 = v9;
    if (bInvertSelection)
    {
        for (i = Actor_FirstActor(-1); i; i = Actor_NextActor(i, -1))
        {
            if (i->ent->s.number != v10)
                Cmd_AI_Dispatch(argc, i, fields, pField);
        }
    }
    else if (v9 > 0x87F)
    {
        Cmd_AI_PrintUsage();
        Com_PrintError(0, "%i is not a valid entity number\n", v10);
    }
    else
    {
        actor = level.gentities[v9].actor;
        if (actor)
        {
            Cmd_AI_Dispatch(argc, actor, fields, pField);
        }
        else
        {
            Cmd_AI_PrintUsage();
            Com_PrintError(0, "entity number %i is not an actor\n", v10);
        }
    }
}

void __cdecl Cmd_AI_Team(
    int argc,
    const actor_fields_s *fields,
    const actor_fields_s *pField,
    int iTeamFlags,
    int bInvertSelection)
{
    int v8; // r30
    actor_s *i; // r31

    v8 = iTeamFlags;
    if (bInvertSelection)
        v8 = ~iTeamFlags;
    for (i = Actor_FirstActor(v8); i; i = Actor_NextActor(i, v8))
        Cmd_AI_Dispatch(argc, i, fields, pField);
}

void __cdecl Cmd_AI_Name(
    int argc,
    const actor_fields_s *fields,
    const actor_fields_s *pField,
    const char *szName,
    int bInvertSelection)
{
    int offset; // [esp+4h] [ebp-Ch]
    unsigned __int16 name; // [esp+8h] [ebp-8h] BYREF
    actor_s *actor; // [esp+Ch] [ebp-4h]

    if (I_strnicmp(szName, "actor_", 6))
        offset = 360;
    else
        offset = 356;
    name = SL_GetString(szName, 0);
    for (actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1))
    {
        if ((*(unsigned __int16 *)((char *)&actor->ent->s.number + offset) == name) == (bInvertSelection == 0))
            Cmd_AI_Dispatch(argc, actor, fields, pField);
    }
    Scr_SetString(&name, 0);
}



void __cdecl Cmd_AI_f()
{
    int v0; // r27
    int nesting; // r7
    int v2; // r28
    const actor_fields_s *FieldForName; // r31
    const actor_fields_s *v4; // r30
    const char *v5; // r29
    char v6[256]; // [sp+50h] [-230h] BYREF
    char v7; // [sp+150h] [-130h] BYREF
    char v8; // [sp+151h] [-12Fh] BYREF

    v0 = 0;
    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    v2 = sv_cmd_args.argc[nesting];
    if (v2 < 3)
    {
        Cmd_AI_PrintUsage();
        return;
    }
    SV_Cmd_ArgvBuffer(2, v6, 256);
    if (!I_stricmp(v6, "list"))
    {
        FieldForName = &aifield_list;
    LABEL_7:
        v4 = 0;
        goto LABEL_8;
    }
    if (!I_stricmp(v6, "delete"))
    {
        FieldForName = &aifield_delete;
        goto LABEL_7;
    }
    v4 = aifields;
    FieldForName = FindFieldForName(aifields, v6);
    if (!FieldForName)
    {
        v4 = sentientfields;
        FieldForName = FindFieldForName(sentientfields, v6);
        if (!FieldForName)
        {
            v4 = entfields;
            FieldForName = FindFieldForName(entfields, v6);
            if (!FieldForName)
            {
                Cmd_AI_PrintUsage();
                Com_PrintError(0, "%s is not an actor or entity field\n", v6);
                return;
            }
        }
    }
LABEL_8:
    SV_Cmd_ArgvBuffer(1, &v7, 256);
    v5 = &v7;
    if (v7 == 33)
    {
        v0 = 1;
        v5 = &v8;
    }
    if (isdigit(*v5))
    {
        Cmd_AI_EntityNumber(v2, v4, FieldForName, v5, v0);
    }
    else if (I_stricmp(v5, "all"))
    {
        if (I_stricmp(v5, "axis"))
        {
            if (I_stricmp(v5, "allies"))
            {
                if (I_stricmp(v5, "neutral"))
                    Cmd_AI_Name(v2, v4, FieldForName, v5, v0);
                else
                    Cmd_AI_Team(v2, v4, FieldForName, 8, v0);
            }
            else
            {
                Cmd_AI_Team(v2, v4, FieldForName, 4, v0);
            }
        }
        else
        {
            Cmd_AI_Team(v2, v4, FieldForName, 2, v0);
        }
    }
    else
    {
        Cmd_AI_Team(v2, v4, FieldForName, -1, v0);
    }
}

void __cdecl GScr_AddFieldsForActor()
{
    const actor_fields_s *f; // [esp+4h] [ebp-4h]

    for (f = aifields; f->name; ++f)
    {
        iassert(!((f - aifields) & ENTFIELD_MASK));
        iassert((f - aifields) == (unsigned short)(f - aifields));

        Scr_AddClassField(0, (char*)f->name, (unsigned __int16)(f - aifields) | ENTFIELD_ACTOR);
    }
}

void __cdecl Scr_SetActorField(actor_s *actor, unsigned int offset)
{
    const actor_fields_s *f; // r4
    void(__cdecl * setter)(actor_s *, const actor_fields_s *); // r11

    iassert(actor);
    iassert((unsigned)offset < ARRAY_COUNT(aifields) - 1);

    f = &aifields[offset];
    setter = f->setter;
    if (setter)
        (setter)(actor, f);
    else
        Scr_SetGenericField((unsigned __int8 *)actor, f->type, f->ofs);
}

void __cdecl Scr_GetActorField(actor_s *actor, unsigned int offset)
{
    const actor_fields_s *f; // r4
    void(__cdecl * getter)(actor_s *, const actor_fields_s *); // r11

    iassert(actor);
    iassert((unsigned)offset < ARRAY_COUNT(aifields) - 1);

    f = &aifields[offset];
    getter = f->getter;
    if (getter)
        (getter)(actor, f);
    else
        Scr_GetGenericField((unsigned __int8 *)actor, f->type, f->ofs);
}

