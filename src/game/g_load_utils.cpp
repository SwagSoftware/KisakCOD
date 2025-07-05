#include "game_public.h"

const char *g_entityBeginParsePoint;
const char *g_entityEndParsePoint;

char *__cdecl CM_EntityString()
{
    if (!cm.mapEnts)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 1435, 0, "%s", "cm.mapEnts");
    return cm.mapEnts->entityString;
}

void __cdecl G_ResetEntityParsePoint()
{
    g_entityBeginParsePoint = CM_EntityString();
    g_entityEndParsePoint = g_entityBeginParsePoint;
}

const char *__cdecl G_GetEntityParsePoint()
{
    return g_entityEndParsePoint;
}

void __cdecl G_SetEntityParsePoint(const char *beginParsePoint)
{
    g_entityBeginParsePoint = beginParsePoint;
    g_entityEndParsePoint = beginParsePoint;
}

int32_t __cdecl G_GetEntityToken(char *buffer, int32_t bufferSize)
{
    parseInfo_t *v2; // eax

    v2 = Com_Parse(&g_entityBeginParsePoint);
    I_strncpyz(buffer, v2->token, bufferSize);
    if (!g_entityBeginParsePoint)
        return 0;
    g_entityEndParsePoint = g_entityBeginParsePoint;
    return 1;
}

int32_t __cdecl G_ParseSpawnVars(SpawnVar *spawnVar)
{
    char com_token[1024]; // [esp+0h] [ebp-808h] BYREF
    char keyname[1028]; // [esp+400h] [ebp-408h] BYREF

    spawnVar->spawnVarsValid = 0;
    spawnVar->numSpawnVars = 0;
    spawnVar->numSpawnVarChars = 0;
    if (!G_GetEntityToken(com_token, 1024))
        return 0;
    if (com_token[0] != 123)
        Com_Error(ERR_DROP, "G_ParseSpawnVars: found %s when expecting {", com_token);
    while (1)
    {
        if (!G_GetEntityToken(keyname, 1024))
            Com_Error(ERR_DROP, "G_ParseSpawnVars: EOF without closing brace");
        if (keyname[0] == 125)
            break;
        if (!G_GetEntityToken(com_token, 1024))
            Com_Error(ERR_DROP, "G_ParseSpawnVars: EOF without closing brace");
        if (com_token[0] == 125)
            Com_Error(ERR_DROP, "G_ParseSpawnVars: closing brace without data");
        if (spawnVar->numSpawnVars == 64)
            Com_Error(ERR_DROP, "G_ParseSpawnVars: MAX_SPAWN_VARS");
        spawnVar->spawnVars[spawnVar->numSpawnVars][0] = G_AddSpawnVarToken(keyname, spawnVar);
        spawnVar->spawnVars[spawnVar->numSpawnVars++][1] = G_AddSpawnVarToken(com_token, spawnVar);
    }
    spawnVar->spawnVarsValid = 1;
    return 1;
}

char *__cdecl G_AddSpawnVarToken(char *string, SpawnVar *spawnVar)
{
    uint32_t v3; // [esp+0h] [ebp-18h]
    char *dest; // [esp+14h] [ebp-4h]

    v3 = strlen(string);
    if ((int)(spawnVar->numSpawnVarChars + v3 + 1) > 2048)
        Com_Error(ERR_DROP, "G_AddSpawnVarToken: MAX_SPAWN_VARS");
    dest = &spawnVar->spawnVarChars[spawnVar->numSpawnVarChars];
    memcpy((uint8_t *)dest, (uint8_t *)string, v3 + 1);
    spawnVar->numSpawnVarChars += v3 + 1;
    return dest;
}

int32_t __cdecl G_SpawnString(const SpawnVar *spawnVar, const char *key, const char *defaultString, const char **out)
{
    int32_t i; // [esp+0h] [ebp-4h]

    if (!spawnVar->spawnVarsValid)
        MyAssertHandler(".\\game\\g_load_utils.cpp", 161, 0, "%s", "spawnVar->spawnVarsValid");
    for (i = 0; i < spawnVar->numSpawnVars; ++i)
    {
        if (!I_stricmp(key, spawnVar->spawnVars[i][0]))
        {
            *out = spawnVar->spawnVars[i][1];
            return 1;
        }
    }
    *out = defaultString;
    return 0;
}

HashEntry_unnamed_type_u __cdecl G_NewString(const char *string)
{
    char str[0x4000]; // [esp+10h] [ebp-4010h] BYREF
    uint32_t v3; // [esp+4014h] [ebp-Ch]
    char *v4; // [esp+4018h] [ebp-8h]
    uint32_t i; // [esp+401Ch] [ebp-4h]

    v3 = strlen(string) + 1;
    if (v3 > 0x4000)
        Com_Error(ERR_DROP, "G_NewString: len = %i > %i", v3, 0x4000);
    v4 = str;
    for (i = 0; i < v3; ++i)
    {
        if (string[i] == 92 && i < v3 - 1)
        {
            if (string[++i] == 110)
                *v4 = 10;
            else
                *v4 = 92;
            ++v4;
        }
        else
        {
            *v4++ = string[i];
        }
    }
    return SL_GetString(str, 0);
}

char str[8][32];
int32_t index;
char *__cdecl vtos(const float *v)
{
    char *s; // [esp+0h] [ebp-4h]

    s = str[index];
    index = ((_BYTE)index + 1) & 7;
    Com_sprintf(s, 0x20u, "(%i %i %i)", (int)*v, (int)v[1], (int)v[2]);
    return s;
}

