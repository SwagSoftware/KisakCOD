#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_fields.h"

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
            Com_Error(ERR_DROP, byte_82025D98);
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
    unsigned int ConstString; // r3
    AISpecies v4; // r10
    const unsigned __int16 **v5; // r11
    const char *v6; // r3
    const char *v7; // r3
    gentity_s *ent; // r11

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 243, 0, "%s", "pSelf");
    ConstString = Scr_GetConstString(0);
    v4 = AI_SPECIES_HUMAN;
    v5 = g_AISpeciesNames;
    while (ConstString != **v5)
    {
        ++v5;
        ++v4;
        if ((int)v5 >= (int)g_entinfoAITextNames)
        {
            v6 = SL_ConvertToString(ConstString);
            v7 = va("unknown type '%s', should be human, dog\n", v6);
            Scr_Error(v7);
            return;
        }
    }
    ent = pSelf->ent;
    pSelf->species = v4;
    ent->s.lerp.u.actor.species = v4;
    G_DObjUpdate(pSelf->ent);
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
    LODWORD(v2) = *(gentity_s **)((char *)&pSelf->ent + pField->ofs);
    Scr_AddFloat((float)((float)v2 * (float)0.001));
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
        Scr_ErrorWithDialogMessage(v9, byte_82003CDD);
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
    self->fixedNode = (_cntlzw(Int) & 0x20) == 0;
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
            (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64((float)((float)v7 * (float)0.001))),
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
                (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(*(float *)&pBase[pField->ofs])),
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
            if (!EntHandle::isDefined((EntHandle *)&pBase[pField->ofs]))
                goto LABEL_18;
            v18 = EntHandle::entnum((EntHandle *)&pBase[pField->ofs]);
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
            if (SentientHandle::isDefined((SentientHandle *)&pBase[pField->ofs]))
            {
                v18 = SentientHandle::sentient((SentientHandle *)&pBase[pField->ofs])->ent->s.number;
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
    int v10; // r26
    actor_s *Actor; // r31
    bool v12; // r30
    unsigned __int16 String; // [sp+50h] [-40h] BYREF

    v10 = I_strnicmp(szName, "actor_", 6) == 0 ? 284 : 292;
    String = SL_GetString(szName, 0);
    Actor = Actor_FirstActor(-1);
    if (Actor)
    {
        v12 = bInvertSelection == 0;
        do
        {
            if ((String == *(unsigned __int16 *)(&Actor->ent->s.eType + v10)) == v12)
                Cmd_AI_Dispatch(argc, Actor, fields, pField);
            Actor = Actor_NextActor(Actor, -1);
        } while (Actor);
    }
    Scr_SetString("", 0);
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
    const actor_fields_s *v0; // r25
    int v1; // r27

    v0 = aifields;
    if ("type")
    {
        v1 = 0;
        do
        {
            if (((v1 / 20) & 0xC000) != 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
                    1267,
                    0,
                    "%s",
                    "!((f - aifields) & ENTFIELD_MASK)");
            if (v1 / 20 != (unsigned __int16)(v1 / 20))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
                    1268,
                    0,
                    "%s",
                    "(f - aifields) == (unsigned short)( f - aifields )");
            Scr_AddClassField(0, v0->name, (unsigned __int16)(v1 / 20) | 0x8000);
            ++v0;
            v1 += 20;
        } while (v0->name);
    }
}

void __cdecl Scr_SetActorField(actor_s *actor, unsigned int offset)
{
    const actor_fields_s *v4; // r4
    void(__cdecl * setter)(actor_s *, const actor_fields_s *); // r11

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1283, 0, "%s", "actor");
    if (offset >= 0x51)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            1284,
            0,
            "%s",
            "(unsigned)offset < ARRAY_COUNT( aifields ) - 1");
    v4 = &aifields[offset];
    setter = v4->setter;
    if (setter)
        ((void(__cdecl *)(actor_s *))setter)(actor);
    else
        Scr_SetGenericField((unsigned __int8 *)actor, v4->type, v4->ofs);
}

void __cdecl Scr_GetActorField(actor_s *actor, unsigned int offset)
{
    const actor_fields_s *v4; // r4
    void(__cdecl * getter)(actor_s *, const actor_fields_s *); // r11

    if (!actor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp", 1306, 0, "%s", "actor");
    if (offset >= 0x51)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_fields.cpp",
            1307,
            0,
            "%s",
            "(unsigned)offset < ARRAY_COUNT( aifields ) - 1");
    v4 = &aifields[offset];
    getter = v4->getter;
    if (getter)
        ((void(__cdecl *)(actor_s *))getter)(actor);
    else
        Scr_GetGenericField((unsigned __int8 *)actor, v4->type, v4->ofs);
}

