#include "game_public.h"
#include <script/scr_vm.h>
#include <script/scr_const.h>
#include <game_mp/g_public_mp.h>
#include <server_mp/server.h>

const client_fields_s fields[18] =
{
  { "name", 0, F_LSTRING, &ClientScr_ReadOnly, &ClientScr_GetName },
  {
    "sessionteam",
    0,
    F_STRING,
    &ClientScr_SetSessionTeam,
    &ClientScr_GetSessionTeam
  },
  {
    "sessionstate",
    0,
    F_STRING,
    &ClientScr_SetSessionState,
    &ClientScr_GetSessionState
  },
  { "maxhealth", 12264, F_INT, &ClientScr_SetMaxHealth, NULL },
  { "score", 12152, F_INT, &ClientScr_SetScore, NULL },
  { "deaths", 12156, F_INT, NULL, NULL },
  {
    "statusicon",
    0,
    F_STRING,
    &ClientScr_SetStatusIcon,
    &ClientScr_GetStatusIcon
  },
  { "headicon", 0, F_STRING, &ClientScr_SetHeadIcon, &ClientScr_GetHeadIcon },
  {
    "headiconteam",
    0,
    F_STRING,
    &ClientScr_SetHeadIconTeam,
    &ClientScr_GetHeadIconTeam
  },
  { "kills", 12160, F_INT, NULL, NULL },
  { "assists", 12164, F_INT, NULL, NULL },
  { "hasradar", 12664, F_INT, NULL, NULL },
  { "spectatorclient", 12136, F_INT, &ClientScr_SetSpectatorClient, NULL },
  { "killcamentity", 12140, F_INT, &ClientScr_SetKillCamEntity, NULL },
  {
    "archivetime",
    12148,
    F_FLOAT,
    &ClientScr_SetArchiveTime,
    &ClientScr_GetArchiveTime
  },
  {
    "psoffsettime",
    12400,
    F_INT,
    &ClientScr_SetPSOffsetTime,
    &ClientScr_GetPSOffsetTime
  },
  { "pers", 12168, F_OBJECT, &ClientScr_ReadOnly, NULL },
  { NULL, 0, F_INT, NULL, NULL }
}; // idb

void __cdecl ClientScr_ReadOnly(gclient_s *pSelf, const client_fields_s *pField)
{
    const char *v2; // eax

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 24, 0, "%s", "pSelf");
    if (!pField)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 25, 0, "%s", "pField");
    v2 = va("player field %s is read-only", pField->name);
    Scr_Error(v2);
}

void __cdecl ClientScr_SetSessionTeam(gclient_s *pSelf, const client_fields_s *pField)
{
    char *v1; // eax
    const char *v2; // eax
    unsigned __int16 newTeam; // [esp+0h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 34, 0, "%s", "pSelf");
    newTeam = Scr_GetConstString(0);
    if (newTeam == scr_const.axis)
    {
        pSelf->sess.cs.team = TEAM_AXIS;
    }
    else if (newTeam == scr_const.allies)
    {
        pSelf->sess.cs.team = TEAM_ALLIES;
    }
    else if (newTeam == scr_const.spectator)
    {
        pSelf->sess.cs.team = TEAM_SPECTATOR;
    }
    else if (newTeam == scr_const.none)
    {
        pSelf->sess.cs.team = TEAM_FREE;
    }
    else
    {
        v1 = SL_ConvertToString(newTeam);
        v2 = va("'%s' is an illegal sessionteam string. Must be allies, axis, none, or spectator.", v1);
        Scr_Error(v2);
    }
    if (pSelf - level.clients >= 64)
        Scr_Error("client is not pointing to the level.clients array");
    KISAK_NULLSUB();
    ClientUserinfoChanged(pSelf - level.clients);
    CalculateRanks();
}

void __cdecl ClientScr_GetName(gclient_s *pSelf, const client_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 64, 0, "%s", "pSelf");
    Scr_AddString(pSelf->sess.cs.name);
}

void __cdecl ClientScr_GetSessionTeam(gclient_s *pSelf, const client_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 81, 0, "%s", "pSelf");
    switch (pSelf->sess.cs.team)
    {
    case TEAM_FREE:
        Scr_AddConstString(scr_const.none);
        break;
    case TEAM_AXIS:
        Scr_AddConstString(scr_const.axis);
        break;
    case TEAM_ALLIES:
        Scr_AddConstString(scr_const.allies);
        break;
    case TEAM_SPECTATOR:
        Scr_AddConstString(scr_const.spectator);
        break;
    default:
        return;
    }
}

void __cdecl ClientScr_SetSessionState(gclient_s *pSelf, const client_fields_s *pField)
{
    char *v1; // eax
    const char *v2; // eax
    unsigned __int16 newState; // [esp+0h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 110, 0, "%s", "pSelf");
    if (pSelf->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 111, 0, "%s", "pSelf->sess.connected != CON_DISCONNECTED");
    newState = Scr_GetConstString(0);
    if (newState == scr_const.playing)
    {
        pSelf->sess.sessionState = SESS_STATE_PLAYING;
    }
    else if (newState == scr_const.dead)
    {
        pSelf->sess.sessionState = SESS_STATE_DEAD;
    }
    else if (newState == scr_const.spectator)
    {
        pSelf->sess.sessionState = SESS_STATE_SPECTATOR;
    }
    else if (newState == scr_const.intermission)
    {
        pSelf->ps.eFlags ^= 2u;
        pSelf->sess.sessionState = SESS_STATE_INTERMISSION;
    }
    else
    {
        v1 = SL_ConvertToString(newState);
        v2 = va("'%s' is an illegal sessionstate string. Must be playing, dead, spectator, or intermission.", v1);
        Scr_Error(v2);
    }
}

void __cdecl ClientScr_GetSessionState(gclient_s *pSelf, const client_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 144, 0, "%s", "pSelf");
    if (pSelf->sess.connected == CON_DISCONNECTED)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 145, 0, "%s", "pSelf->sess.connected != CON_DISCONNECTED");
    switch (pSelf->sess.sessionState)
    {
    case SESS_STATE_PLAYING:
        Scr_AddConstString(scr_const.playing);
        break;
    case SESS_STATE_DEAD:
        Scr_AddConstString(scr_const.dead);
        break;
    case SESS_STATE_SPECTATOR:
        Scr_AddConstString(scr_const.spectator);
        break;
    case SESS_STATE_INTERMISSION:
        Scr_AddConstString(scr_const.intermission);
        break;
    default:
        return;
    }
}

void __cdecl ClientScr_SetMaxHealth(gclient_s *pSelf, const client_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 174, 0, "%s", "pSelf");
    pSelf->sess.maxHealth = Scr_GetInt(0);
    if (pSelf->sess.maxHealth < 1)
        pSelf->sess.maxHealth = 1;
    if (pSelf->ps.stats[0] > pSelf->sess.maxHealth)
        pSelf->ps.stats[0] = pSelf->sess.maxHealth;
    g_entities[pSelf - level.clients].health = pSelf->ps.stats[0];
    pSelf->ps.stats[2] = pSelf->sess.maxHealth;
}

void __cdecl ClientScr_SetScore(gclient_s *pSelf, const client_fields_s *pField)
{
    pSelf->sess.score = Scr_GetInt(0);
    CalculateRanks();
}

void __cdecl ClientScr_SetSpectatorClient(gclient_s *pSelf, const client_fields_s *pField)
{
    int iNewSpectatorClient; // [esp+0h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 215, 0, "%s", "pSelf");
    iNewSpectatorClient = Scr_GetInt(0);
    if (iNewSpectatorClient < -1 || iNewSpectatorClient >= 64)
        Scr_Error("spectatorclient can only be set to -1, or a valid client number");
    pSelf->sess.forceSpectatorClient = iNewSpectatorClient;
}

void __cdecl ClientScr_SetKillCamEntity(gclient_s *pSelf, const client_fields_s *pField)
{
    int iNewKillCamEntity; // [esp+0h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 236, 0, "%s", "pSelf");
    iNewKillCamEntity = Scr_GetInt(0);
    if (iNewKillCamEntity < -1 || iNewKillCamEntity >= 1024)
        Scr_Error("killcamentity can only be set to -1, or a valid entity number");
    pSelf->sess.killCamEntity = iNewKillCamEntity;
}

void __cdecl ClientScr_SetStatusIcon(gclient_s *pSelf, const client_fields_s *pField)
{
    char *pszIcon; // [esp+0h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 258, 0, "%s", "pSelf");
    pszIcon = Scr_GetString(0);
    pSelf->sess.status_icon = GScr_GetStatusIconIndex(pszIcon);
}

void __cdecl ClientScr_GetStatusIcon(gclient_s *pSelf, const client_fields_s *pField)
{
    char szConfigString[1028]; // [esp+0h] [ebp-408h] BYREF

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 272, 0, "%s", "pSelf");
    if (pSelf->sess.status_icon > 8u)
        MyAssertHandler(
            ".\\game\\g_client_fields.cpp",
            274,
            0,
            "%s",
            "pSelf->sess.status_icon >= 0 && pSelf->sess.status_icon <= MAX_STATUS_ICONS");
    if (pSelf->sess.status_icon)
    {
        SV_GetConfigstring(pSelf->sess.status_icon + 2258, szConfigString, 1024);
        Scr_AddString(szConfigString);
    }
    else
    {
        Scr_AddString((char *)"");
    }
}

void __cdecl ClientScr_SetHeadIcon(gclient_s *pSelf, const client_fields_s *pField)
{
    gentity_s *pEnt; // [esp+0h] [ebp-8h]
    char *pszIcon; // [esp+4h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 299, 0, "%s", "pSelf");
    pEnt = &g_entities[pSelf - level.clients];
    pszIcon = Scr_GetString(0);
    pEnt->s.iHeadIcon = GScr_GetHeadIconIndex(pszIcon);
}

void __cdecl ClientScr_GetHeadIcon(gclient_s *pSelf, const client_fields_s *pField)
{
    char szConfigString[1024]; // [esp+0h] [ebp-408h] BYREF
    gentity_s *pEnt; // [esp+404h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 317, 0, "%s", "pSelf");
    pEnt = &g_entities[pSelf - level.clients];
    if (pEnt->s.iHeadIcon)
    {
        if (pEnt->s.iHeadIcon <= 15)
        {
            SV_GetConfigstring(pEnt->s.iHeadIcon + 2266, szConfigString, 1024);
            Scr_AddString(szConfigString);
        }
    }
    else
    {
        Scr_AddString((char *)"");
    }
}

void __cdecl ClientScr_SetHeadIconTeam(gclient_s *pSelf, const client_fields_s *pField)
{
    char *v1; // eax
    const char *v2; // eax
    gentity_s *pEnt; // [esp+0h] [ebp-8h]
    unsigned __int16 sTeam; // [esp+4h] [ebp-4h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 343, 0, "%s", "pSelf");
    pEnt = &g_entities[pSelf - level.clients];
    sTeam = Scr_GetConstString(0);
    if (sTeam == scr_const.none)
    {
        pEnt->s.iHeadIconTeam = 0;
    }
    else if (sTeam == scr_const.allies)
    {
        pEnt->s.iHeadIconTeam = 2;
    }
    else if (sTeam == scr_const.axis)
    {
        pEnt->s.iHeadIconTeam = 1;
    }
    else if (sTeam == scr_const.spectator)
    {
        v1 = SL_ConvertToString(sTeam);
        v2 = va("'%s' is an illegal head icon team string. Must be none, allies, axis, or spectator.", v1);
        Scr_Error(v2);
    }
    else
    {
        pEnt->s.iHeadIconTeam = 3;
    }
}

void __cdecl ClientScr_GetHeadIconTeam(gclient_s *pSelf, const client_fields_s *pField)
{
    int iHeadIconTeam; // [esp+0h] [ebp-8h]

    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 370, 0, "%s", "pSelf");
    iHeadIconTeam = g_entities[pSelf - level.clients].s.iHeadIconTeam;
    switch (iHeadIconTeam)
    {
    case 1:
        Scr_AddConstString(scr_const.axis);
        break;
    case 2:
        Scr_AddConstString(scr_const.allies);
        break;
    case 3:
        Scr_AddConstString(scr_const.spectator);
        break;
    default:
        Scr_AddConstString(scr_const.none);
        break;
    }
}

void __cdecl ClientScr_SetArchiveTime(gclient_s *pSelf, const client_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 394, 0, "%s", "pSelf");
    pSelf->sess.archiveTime = (int)(Scr_GetFloat(0) * 1000.0);
}

void __cdecl ClientScr_GetArchiveTime(gclient_s *pSelf, const client_fields_s *pField)
{
    Scr_AddFloat(pSelf->sess.archiveTime * EQUAL_EPSILON);
}

void __cdecl ClientScr_SetPSOffsetTime(gclient_s *pSelf, const client_fields_s *pField)
{
    if (!pSelf)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 417, 0, "%s", "pSelf");
    pSelf->sess.psOffsetTime = Scr_GetInt(0);
}

void __cdecl ClientScr_GetPSOffsetTime(gclient_s *pSelf, const client_fields_s *pField)
{
    Scr_AddInt(pSelf->sess.archiveTime);
}

void __cdecl GScr_AddFieldsForClient()
{
    const client_fields_s *f; // [esp+4h] [ebp-4h]

    for (f = fields; f->name; ++f)
    {
        if (((f - fields) & 0xC000) != 0)
            MyAssertHandler(".\\game\\g_client_fields.cpp", 478, 0, "%s", "!((f - fields) & ENTFIELD_MASK)");
        if (f - fields != (unsigned __int16)(f - fields))
            MyAssertHandler(".\\game\\g_client_fields.cpp", 479, 0, "%s", "(f - fields) == (unsigned short)( f - fields )");
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - fields) | 0xC000);
    }
}

void __cdecl Scr_SetClientField(gclient_s *client, int offset)
{
    const client_fields_s *f; // [esp+0h] [ebp-4h]

    if (!client)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 494, 0, "%s", "client");
    if ((unsigned int)offset >= 0x11)
        MyAssertHandler(
            ".\\game\\g_client_fields.cpp",
            495,
            0,
            "%s",
            "static_cast<unsigned int>( offset ) < ARRAY_COUNT( fields ) - 1");
    if (offset < 0)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 496, 0, "%s", "offset >= 0");
    f = &fields[offset];
    if (f->setter)
    {
        f->setter(client, f);
    }
    else
    {
        if (!f->ofs)
            MyAssertHandler(".\\game\\g_client_fields.cpp", 506, 0, "%s", "f->ofs");
        Scr_SetGenericField((unsigned __int8 *)client, f->type, f->ofs);
    }
}

void __cdecl Scr_GetClientField(gclient_s *client, int offset)
{
    const client_fields_s *f; // [esp+0h] [ebp-4h]

    if (!client)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 520, 0, "%s", "client");
    if ((unsigned int)offset >= 0x11)
        MyAssertHandler(
            ".\\game\\g_client_fields.cpp",
            521,
            0,
            "%s",
            "static_cast<unsigned int>( offset ) < ARRAY_COUNT( fields ) - 1");
    if (offset < 0)
        MyAssertHandler(".\\game\\g_client_fields.cpp", 522, 0, "%s", "offset >= 0");
    f = &fields[offset];
    if (f->getter)
    {
        f->getter(client, f);
    }
    else
    {
        if (!f->ofs)
            MyAssertHandler(".\\game\\g_client_fields.cpp", 532, 0, "%s", "f->ofs");
        Scr_GetGenericField((unsigned __int8 *)client, f->type, f->ofs);
    }
}

