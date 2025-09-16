#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "sentient.h"
#include <script/scr_variable.h>
#include "g_main.h"
#include <script/scr_vm.h>
#include "actor_threat.h"
#include "g_public.h"

sentient_s *__cdecl Sentient_Get(scr_entref_t entref)
{
    sentient_s *result; // r3

    if (entref.classnum)
        goto LABEL_5;

    iassert(entref.entnum < MAX_GENTITIES);

    result = g_entities[entref.entnum].sentient;
    if (!result)
    {
    LABEL_5:
        Scr_ObjectError("not a sentient");
        return 0;
    }
    return result;
}

void __cdecl SentientCmd_GetEnemySqDist(scr_entref_t entref)
{
    Sentient_Get(entref);
    Scr_Error("GetEnemySqDist is depricated, use GetClosestEnemySqDist.\n");
}

void __cdecl SentientCmd_GetClosestEnemySqDist(scr_entref_t entref)
{
    sentient_s *pSelf; // r29
    double closestDist; // fp31
    team_t v3; // r3
    int iTeamFlags; // r27
    sentient_s *i; // r31
    float selfOrigin[3]; // [sp+50h] [-60h] BYREF // v6
    float enemyOrigin[3]; // [sp+60h] [-50h] BYREF // v9

    pSelf = Sentient_Get(entref);
    closestDist = 100000000.0f;
    v3 = Sentient_EnemyTeam(pSelf->eTeam);
    if (v3)
    {
        iTeamFlags = 1 << v3;
        Sentient_GetOrigin(pSelf, selfOrigin);
        for (i = Sentient_FirstSentient(iTeamFlags); i; i = Sentient_NextSentient(i, iTeamFlags))
        {
            if (pSelf->ent->actor->sentientInfo[i - level.sentients].lastKnownPosTime > 0
                && (i->ent->flags & 4) == 0
                && !Actor_CheckIgnore(pSelf, i))
            {
                Sentient_GetOrigin(i, enemyOrigin);
                float dist = Vec3DistanceSq(enemyOrigin, selfOrigin);
                if (dist < closestDist)
                {
                    closestDist = dist;
                }
            }
        }
        Scr_AddFloat(closestDist);
    }
}

void SentientCmd_CreateThreatBiasGroup()
{
    unsigned int ConstString; // r3

    if (Scr_GetNumParam() != 1)
        Scr_ParamError(0, "createthreatbiasgroup [name]");
    ConstString = Scr_GetConstString(0);
    Actor_CreateThreatBiasGroup(ConstString);
}

void SentientCmd_ThreatBiasGroupExists()
{
    unsigned int ConstString; // r3
    unsigned int ThreatBiasGroupIndex; // r3

    if (Scr_GetNumParam() != 1)
        Scr_ParamError(0, "threatbiasgroupexists [name]");
    ConstString = Scr_GetConstString(0);
    ThreatBiasGroupIndex = Actor_FindThreatBiasGroupIndex(ConstString);
    //Scr_AddBool(_cntlzw(ThreatBiasGroupIndex) != 0);
    Scr_AddBool((bool)ThreatBiasGroupIndex);
}

void SentientCmd_GetThreatBias()
{
    unsigned int ConstString; // r3
    int ThreatBiasGroupIndex; // r31
    unsigned int v2; // r3
    int v3; // r3
    const char *v4; // r3
    const char *v5; // r3
    const char *String; // r3
    const char *v7; // r3
    int ThreatBias; // r3

    if (Scr_GetNumParam() != 2)
        Scr_ParamError(0, "getthreatbias [group for] [group against]");
    ConstString = Scr_GetConstString(0);
    ThreatBiasGroupIndex = Actor_FindThreatBiasGroupIndex(ConstString);
    v2 = Scr_GetConstString(1);
    v3 = Actor_FindThreatBiasGroupIndex(v2);
    if (ThreatBiasGroupIndex >= 0)
    {
        if (v3 >= 0)
        {
            ThreatBias = Actor_GetThreatBias(ThreatBiasGroupIndex, v3);
            Scr_AddInt(ThreatBias);
        }
        else
        {
            String = Scr_GetString(1);
            v7 = va("Invalid threat bias group '%s'.\n", String);
            Scr_Error(v7);
        }
    }
    else
    {
        v4 = Scr_GetString(0);
        v5 = va("Invalid threat bias group '%s'.\n", v4);
        Scr_Error(v5);
    }
}

void SentientCmd_SetThreatBias()
{
    unsigned int ConstString; // r3
    int ThreatBiasGroupIndex; // r31
    unsigned int v2; // r3
    int v3; // r30
    int Int; // r5
    const char *v5; // r3
    const char *v6; // r3
    const char *String; // r3
    const char *v8; // r3

    if (Scr_GetNumParam() != 3)
        Scr_ParamError(0, "setthreatbias [group for] [group against] [threat]");
    ConstString = Scr_GetConstString(0);
    ThreatBiasGroupIndex = Actor_FindThreatBiasGroupIndex(ConstString);
    v2 = Scr_GetConstString(1);
    v3 = Actor_FindThreatBiasGroupIndex(v2);
    Int = Scr_GetInt(2);
    if (ThreatBiasGroupIndex >= 0)
    {
        if (v3 >= 0)
        {
            Actor_SetThreatBias(ThreatBiasGroupIndex, v3, Int);
        }
        else
        {
            String = Scr_GetString(1);
            v8 = va("Invalid threat bias group '%s'.\n", String);
            Scr_Error(v8);
        }
    }
    else
    {
        v5 = Scr_GetString(0);
        v6 = va("Invalid threat bias group '%s'.\n", v5);
        Scr_Error(v6);
    }
}

void SentientCmd_SetThreatBiasAgainstAll()
{
    unsigned int ConstString; // r3
    int ThreatBiasGroupIndex; // r31
    int Int; // r4
    const char *String; // r3
    const char *v4; // r3

    if (Scr_GetNumParam() != 2)
        Scr_ParamError(0, "setthreatbiasagainstall [group for] [threat]");
    ConstString = Scr_GetConstString(0);
    ThreatBiasGroupIndex = Actor_FindThreatBiasGroupIndex(ConstString);
    Int = Scr_GetInt(1);
    if (ThreatBiasGroupIndex >= 0)
    {
        Actor_SetThreatBiasEntireGroup(ThreatBiasGroupIndex, Int);
    }
    else
    {
        String = Scr_GetString(0);
        v4 = va("Invalid threat bias group '%s'.\n", String);
        Scr_Error(v4);
    }
}

void SentientCmd_SetIgnoreMeGroup()
{
    unsigned int ConstString; // r3
    int ThreatBiasGroupIndex; // r31
    unsigned int v2; // r3
    int v3; // r3
    const char *v4; // r3
    const char *v5; // r3
    const char *String; // r3
    const char *v7; // r3

    if (Scr_GetNumParam() != 2)
        Scr_ParamError(0, "setignoremegroup [group for] [group ignoring]");
    ConstString = Scr_GetConstString(0);
    ThreatBiasGroupIndex = Actor_FindThreatBiasGroupIndex(ConstString);
    v2 = Scr_GetConstString(1);
    v3 = Actor_FindThreatBiasGroupIndex(v2);
    if (ThreatBiasGroupIndex >= 0)
    {
        if (v3 >= 0)
        {
            Actor_SetIgnoreMeGroup(ThreatBiasGroupIndex, v3);
        }
        else
        {
            String = Scr_GetString(1);
            v7 = va("Invalid threat bias group '%s'.\n", String);
            Scr_Error(v7);
        }
    }
    else
    {
        v4 = Scr_GetString(0);
        v5 = va("Invalid threat bias group '%s'.\n", v4);
        Scr_Error(v5);
    }
}

void __cdecl SentientCmd_SetThreatBiasGroup(scr_entref_t entref)
{
    sentient_s *v1; // r31
    unsigned int ConstString; // r3
    int ThreatBiasGroupIndex; // r3
    const char *String; // r3
    const char *v5; // r3

    v1 = Sentient_Get(entref);
    if (Scr_GetNumParam() == 1)
    {
        ConstString = Scr_GetConstString(0);
        ThreatBiasGroupIndex = Actor_FindThreatBiasGroupIndex(ConstString);
        if (ThreatBiasGroupIndex >= 0)
        {
            v1->iThreatBiasGroupIndex = ThreatBiasGroupIndex;
        }
        else
        {
            String = Scr_GetString(0);
            v5 = va("Invalid threat bias group '%s'.\n", String);
            Scr_Error(v5);
        }
    }
    else
    {
        v1->iThreatBiasGroupIndex = 0;
    }
}

void __cdecl SentientCmd_GetThreatBiasGroup(scr_entref_t entref)
{
    int iThreatBiasGroupIndex; // r11
    const char *v2; // r3

    iThreatBiasGroupIndex = Sentient_Get(entref)->iThreatBiasGroupIndex;
    if (iThreatBiasGroupIndex <= 0)
    {
        Scr_AddString("");
    }
    else
    {
        v2 = SL_ConvertToString(g_threatBias.groupName[iThreatBiasGroupIndex]);
        Scr_AddString(v2);
    }
}

static const BuiltinMethodDef methods_4[4] =
{
  { "getenemysqdist", SentientCmd_GetEnemySqDist, 0 },
  { "getclosestenemysqdist", SentientCmd_GetClosestEnemySqDist, 0 },
  { "setthreatbiasgroup", SentientCmd_SetThreatBiasGroup, 0 },
  { "getthreatbiasgroup", SentientCmd_GetThreatBiasGroup, 0 }
};


void(__cdecl *__cdecl Sentient_GetMethod(const char **pName))(scr_entref_t)
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinMethodDef *i; // r7
    const char *actionString; // r10
    const char *v5; // r11
    int v6; // r8

    v1 = 0;
    v2 = 0;
    for (i = methods_4; ; ++i)
    {
        actionString = i->actionString;
        v5 = *pName;
        do
        {
            v6 = (unsigned __int8)*v5 - *(unsigned __int8 *)actionString;
            if (!*v5)
                break;
            ++v5;
            ++actionString;
        } while (!v6);
        if (!v6)
            break;
        v2 += 12;
        ++v1;
        if (v2 >= 0x30)
            return 0;
    }
    *pName = methods_4[v1].actionString;
    return methods_4[v1].actionFunc;
}


static const BuiltinFunctionDef sentfunctions[6] =
{
  { "createthreatbiasgroup", SentientCmd_CreateThreatBiasGroup, 0 },
  { "threatbiasgroupexists", SentientCmd_ThreatBiasGroupExists, 0 },
  { "getthreatbias", SentientCmd_GetThreatBias, 0 },
  { "setthreatbias", SentientCmd_SetThreatBias, 0 },
  { "setthreatbiasagainstall", SentientCmd_SetThreatBiasAgainstAll, 0 },
  { "setignoremegroup", SentientCmd_SetIgnoreMeGroup, 0 }
};


void(__cdecl *__cdecl Sentient_GetFunction(const char **pName))()
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinFunctionDef *i; // r7
    const char *actionString; // r10
    const char *v5; // r11
    int v6; // r8

    v1 = 0;
    v2 = 0;
    for (i = sentfunctions; ; ++i)
    {
        actionString = i->actionString;
        v5 = *pName;
        do
        {
            v6 = (unsigned __int8)*v5 - *(unsigned __int8 *)actionString;
            if (!*v5)
                break;
            ++v5;
            ++actionString;
        } while (!v6);
        if (!v6)
            break;
        v2 += 12;
        ++v1;
        if (v2 >= 0x48)
            return 0;
    }
    *pName = sentfunctions[v1].actionString;
    return sentfunctions[v1].actionFunc;
}

