#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "sentient.h"

sentient_s *__cdecl Sentient_Get(scr_entref_t *entref)
{
    sentient_s *result; // r3
    unsigned __int16 v2; // [sp+74h] [+14h]

    v2 = HIWORD(entref);
    if ((_WORD)entref)
        goto LABEL_5;
    if (HIWORD(entref) >= 0x880u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_script_cmd.cpp",
            47,
            0,
            "%s",
            "entref.entnum < MAX_GENTITIES");
    result = g_entities[v2].sentient;
    if (!result)
    {
    LABEL_5:
        Scr_ObjectError("not a sentient");
        return 0;
    }
    return result;
}

void __cdecl SentientCmd_GetEnemySqDist(scr_entref_t *entref)
{
    Sentient_Get(entref);
    Scr_Error("GetEnemySqDist is depricated, use GetClosestEnemySqDist.\n");
}

void __cdecl SentientCmd_GetClosestEnemySqDist(scr_entref_t *entref)
{
    sentient_s *v1; // r29
    double v2; // fp31
    team_t v3; // r3
    int v4; // r27
    sentient_s *i; // r31
    float v6; // [sp+50h] [-60h] BYREF
    float v7; // [sp+54h] [-5Ch]
    float v8; // [sp+58h] [-58h]
    float v9; // [sp+60h] [-50h] BYREF
    float v10; // [sp+64h] [-4Ch]
    float v11; // [sp+68h] [-48h]

    v1 = Sentient_Get(entref);
    v2 = 100000000.0;
    v3 = Sentient_EnemyTeam(v1->eTeam);
    if (v3)
    {
        v4 = 1 << v3;
        Sentient_GetOrigin(v1, &v6);
        for (i = Sentient_FirstSentient(v4); i; i = Sentient_NextSentient(i, v4))
        {
            if (v1->ent->actor->sentientInfo[i - level.sentients].lastKnownPosTime > 0
                && (i->ent->flags & 4) == 0
                && !Actor_CheckIgnore(v1, i))
            {
                Sentient_GetOrigin(i, &v9);
                if ((float)((float)((float)(v10 - v7) * (float)(v10 - v7))
                    + (float)((float)((float)(v11 - v8) * (float)(v11 - v8))
                        + (float)((float)(v9 - v6) * (float)(v9 - v6)))) < v2)
                    v2 = (float)((float)((float)(v10 - v7) * (float)(v10 - v7))
                        + (float)((float)((float)(v11 - v8) * (float)(v11 - v8))
                            + (float)((float)(v9 - v6) * (float)(v9 - v6))));
            }
        }
        Scr_AddFloat(v2);
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
    Scr_AddBool(_cntlzw(ThreatBiasGroupIndex) != 0);
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
    v2 = Scr_GetConstString(1u);
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
            String = Scr_GetString(1u);
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
    v2 = Scr_GetConstString(1u);
    v3 = Actor_FindThreatBiasGroupIndex(v2);
    Int = Scr_GetInt(2u);
    if (ThreatBiasGroupIndex >= 0)
    {
        if (v3 >= 0)
        {
            Actor_SetThreatBias(ThreatBiasGroupIndex, v3, Int);
        }
        else
        {
            String = Scr_GetString(1u);
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
    Int = Scr_GetInt(1u);
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
    v2 = Scr_GetConstString(1u);
    v3 = Actor_FindThreatBiasGroupIndex(v2);
    if (ThreatBiasGroupIndex >= 0)
    {
        if (v3 >= 0)
        {
            Actor_SetIgnoreMeGroup(ThreatBiasGroupIndex, v3);
        }
        else
        {
            String = Scr_GetString(1u);
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

void __cdecl SentientCmd_SetThreatBiasGroup(scr_entref_t *entref)
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

void __cdecl SentientCmd_GetThreatBiasGroup(scr_entref_t *entref)
{
    int iThreatBiasGroupIndex; // r11
    const char *v2; // r3

    iThreatBiasGroupIndex = Sentient_Get(entref)->iThreatBiasGroupIndex;
    if (iThreatBiasGroupIndex <= 0)
    {
        Scr_AddString(byte_82003CDD);
    }
    else
    {
        v2 = SL_ConvertToString(g_threatBias.groupName[iThreatBiasGroupIndex]);
        Scr_AddString(v2);
    }
}

void(__cdecl *__cdecl Sentient_GetMethod(const char **pName))(scr_entref_t)
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinMethodDef *i; // r7
    const char *actionString; // r10
    _BYTE *v5; // r11
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

void(__cdecl *__cdecl Sentient_GetFunction(const char **pName))()
{
    int v1; // r6
    unsigned int v2; // r5
    const BuiltinFunctionDef *i; // r7
    const char *actionString; // r10
    _BYTE *v5; // r11
    int v6; // r8

    v1 = 0;
    v2 = 0;
    for (i = functions; ; ++i)
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
    *pName = functions[v1].actionString;
    return functions[v1].actionFunc;
}

