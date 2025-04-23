#include "g_public_mp.h"
#include "g_utils_mp.h"
#include <server_mp/server.h>
#include <server/sv_game.h>
#include <server/sv_world.h>
#include <script/scr_vm.h>


void __cdecl SetClientViewAngle(gentity_s *ent, const float *angle)
{
    double v2; // st7
    double v3; // st7
    float v4; // [esp+8h] [ebp-7Ch]
    float v5; // [esp+14h] [ebp-70h]
    float v6; // [esp+18h] [ebp-6Ch]
    float v7; // [esp+1Ch] [ebp-68h]
    float v8; // [esp+20h] [ebp-64h]
    float v9; // [esp+24h] [ebp-60h]
    float v10; // [esp+28h] [ebp-5Ch]
    float v11; // [esp+2Ch] [ebp-58h]
    float v12; // [esp+30h] [ebp-54h]
    float v13; // [esp+34h] [ebp-50h]
    float v14; // [esp+38h] [ebp-4Ch]
    float v15; // [esp+3Ch] [ebp-48h]
    float *viewangles; // [esp+40h] [ebp-44h]
    float v17; // [esp+4Ch] [ebp-38h]
    float v18; // [esp+50h] [ebp-34h]
    float v19; // [esp+54h] [ebp-30h]
    float v20; // [esp+58h] [ebp-2Ch]
    float v21; // [esp+5Ch] [ebp-28h]
    float v22; // [esp+60h] [ebp-24h]
    float v23; // [esp+64h] [ebp-20h]
    float v24; // [esp+68h] [ebp-1Ch]
    float v25; // [esp+6Ch] [ebp-18h]
    float fDeltab; // [esp+70h] [ebp-14h]
    float fDelta; // [esp+70h] [ebp-14h]
    float fDeltac; // [esp+70h] [ebp-14h]
    float fDeltad; // [esp+70h] [ebp-14h]
    float fDeltaa; // [esp+70h] [ebp-14h]
    float fDeltae; // [esp+70h] [ebp-14h]
    float newAngle[3]; // [esp+74h] [ebp-10h]
    int i; // [esp+80h] [ebp-4h]

    newAngle[0] = *angle;
    newAngle[1] = angle[1];
    newAngle[2] = angle[2];
    if ((ent->client->ps.pm_flags & 1) != 0 && (ent->client->ps.eFlags & 0x300) == 0)
    {
        fDeltab = AngleDelta(ent->client->ps.proneDirection, newAngle[1]);
        v25 = fDeltab * 0.002777777845039964;
        v15 = v25 + 0.5;
        v14 = floor(v15);
        fDelta = (v25 - v14) * 360.0;
        if (bg_prone_yawcap->current.value < (double)fDelta || fDelta < -bg_prone_yawcap->current.value)
        {
            if (bg_prone_yawcap->current.value >= (double)fDelta)
                v2 = fDelta + bg_prone_yawcap->current.value;
            else
                v2 = fDelta - bg_prone_yawcap->current.value;
            fDeltac = v2;
            ent->client->ps.delta_angles[1] = ent->client->ps.delta_angles[1] + fDeltac;
            if (fDeltac <= 0.0)
            {
                v12 = ent->client->ps.proneDirection + bg_prone_yawcap->current.value;
                newAngle[1] = AngleNormalize360(v12);
            }
            else
            {
                v13 = ent->client->ps.proneDirection - bg_prone_yawcap->current.value;
                newAngle[1] = AngleNormalize360(v13);
            }
        }
        fDeltad = AngleDelta(ent->client->ps.proneTorsoPitch, newAngle[0]);
        v24 = fDeltad * 0.002777777845039964;
        v11 = v24 + 0.5;
        v10 = floor(v11);
        fDeltaa = (v24 - v10) * 360.0;
        if (fDeltaa > 45.0 || fDeltaa < -45.0)
        {
            if (fDeltaa <= 45.0)
                v3 = fDeltaa + 45.0;
            else
                v3 = fDeltaa - 45.0;
            fDeltae = v3;
            ent->client->ps.delta_angles[0] = ent->client->ps.delta_angles[0] + fDeltae;
            if (fDeltae <= 0.0)
            {
                v20 = ent->client->ps.proneTorsoPitch + 45.0;
                v21 = v20 * 0.002777777845039964;
                v7 = v21 + 0.5;
                v6 = floor(v7);
                newAngle[0] = (v21 - v6) * 360.0;
            }
            else
            {
                v22 = ent->client->ps.proneTorsoPitch - 45.0;
                v23 = v22 * 0.002777777845039964;
                v9 = v23 + 0.5;
                v8 = floor(v9);
                newAngle[0] = (v23 - v8) * 360.0;
            }
        }
    }
    for (i = 0; i < 3; ++i)
    {
        v18 = newAngle[i] - (double)ent->client->sess.cmd.angles[i] * 0.0054931640625;
        v19 = v18 * 0.002777777845039964;
        v5 = v19 + 0.5;
        v4 = floor(v5);
        v17 = (v19 - v4) * 360.0;
        ent->client->ps.delta_angles[i] = v17;
    }
    ent->r.currentAngles[0] = newAngle[0];
    ent->r.currentAngles[1] = newAngle[1];
    ent->r.currentAngles[2] = newAngle[2];
    viewangles = ent->client->ps.viewangles;
    *viewangles = ent->r.currentAngles[0];
    viewangles[1] = ent->r.currentAngles[1];
    viewangles[2] = ent->r.currentAngles[2];
}

void __cdecl G_GetPlayerViewOrigin(const playerState_s *ps, float *origin)
{
    if ((ps->eFlags & 0x300) != 0)
    {
        if (ps->viewlocked == PLAYERVIEWLOCK_NONE)
            MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 97, 0, "%s", "ps->viewlocked");
        if (ps->viewlocked_entNum == 1023)
            MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 98, 0, "%s", "ps->viewlocked_entNum != ENTITYNUM_NONE");
        if (!G_DObjGetWorldTagPos(&g_entities[ps->viewlocked_entNum], scr_const.tag_player, origin))
            Com_Error(ERR_DROP, "G_GetPlayerViewOrigin: Couldn't find [tag_player] on turret");
    }
    else
    {
        BG_GetPlayerViewOrigin(ps, origin, level.time);
    }
}

void __cdecl ClientUserinfoChanged(unsigned int clientNum)
{
    gclient_s *client; // [esp+0h] [ebp-814h]
    char oldname[1024]; // [esp+4h] [ebp-810h] BYREF
    char userinfo[1024]; // [esp+404h] [ebp-410h] BYREF
    clientInfo_t *ci; // [esp+808h] [ebp-Ch]
    gentity_s *ent; // [esp+80Ch] [ebp-8h]
    const char *s; // [esp+810h] [ebp-4h]

    ent = &g_entities[clientNum];
    client = ent->client;
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\game_mp\\g_client_mp.cpp",
            207,
            0,
            "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            clientNum,
            64);
    SV_GetUserinfo(clientNum, userinfo, 1024);
    if (!Info_Validate(userinfo))
        strcpy(userinfo, "\\name\\badinfo");
    client->sess.localClient = SV_IsLocalClient(clientNum);
    s = Info_ValueForKey(userinfo, "cg_predictItems");
    if (atoi(s))
        client->sess.predictItemPickup = 1;
    else
        client->sess.predictItemPickup = 0;
    if (client->sess.connected == CON_CONNECTED && level.manualNameChange)
    {
        s = Info_ValueForKey(userinfo, "name");
        ClientCleanName(s, client->sess.newnetname, 16);
    }
    else
    {
        I_strncpyz(oldname, client->sess.cs.name, 1024);
        s = Info_ValueForKey(userinfo, "name");
        ClientCleanName(s, client->sess.cs.name, 16);
        I_strncpyz(client->sess.newnetname, client->sess.cs.name, 16);
    }
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\game_mp\\g_client_mp.cpp",
            253,
            0,
            "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            clientNum,
            64);
    ci = &level_bgs.clientinfo[clientNum];
    if (!ci->infoValid)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 255, 0, "%s", "ci->infoValid");
    ci->clientNum = clientNum;
    I_strncpyz(ci->name, client->sess.cs.name, 16);
    ci->team = client->sess.cs.team;
}

void __cdecl ClientCleanName(const char *in, char *out, int outSize)
{
    char v3; // [esp+3h] [ebp-11h]
    int len; // [esp+4h] [ebp-10h]
    int colorlessLen; // [esp+8h] [ebp-Ch]
    char *p; // [esp+Ch] [ebp-8h]
    int spaces; // [esp+10h] [ebp-4h]
    int outSizea; // [esp+24h] [ebp+10h]

    outSizea = outSize - 1;
    len = 0;
    colorlessLen = 0;
    p = out;
    *out = 0;
    spaces = 0;
    while (1)
    {
        v3 = *in++;
        if (!v3)
            break;
        if (*p || v3 != 32)
        {
            if (v3 == 94)
            {
                ++in;
            }
            else
            {
                if (v3 != 32)
                {
                    spaces = 0;
                    goto LABEL_11;
                }
                if (++spaces <= 3)
                {
                LABEL_11:
                    if (len > outSizea - 1)
                        break;
                    *out++ = v3;
                    ++colorlessLen;
                    ++len;
                }
            }
        }
    }
    *out = 0;
    if (!*p || !colorlessLen)
        I_strncpyz(p, "UnnamedPlayer", outSizea);
}

char *__cdecl ClientConnect(unsigned int clientNum, unsigned __int16 scriptPersId)
{
    gclient_s *client; // [esp+14h] [ebp-418h]
    XAnimTree_s *pXAnimTree; // [esp+18h] [ebp-414h]
    char userinfo[1024]; // [esp+1Ch] [ebp-410h] BYREF
    clientInfo_t *ci; // [esp+420h] [ebp-Ch]
    gentity_s *ent; // [esp+424h] [ebp-8h]
    const char *value; // [esp+428h] [ebp-4h]

    if (!scriptPersId)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 302, 0, "%s", "scriptPersId");
    ent = &g_entities[clientNum];
    client = &level.clients[clientNum];
    ClientClearFields(client);
    memset((unsigned __int8 *)client, 0, sizeof(gclient_s));
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\game_mp\\g_client_mp.cpp",
            313,
            0,
            "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            clientNum,
            64);
    ci = &level_bgs.clientinfo[clientNum];
    pXAnimTree = ci->pXAnimTree;
    memset((unsigned __int8 *)ci, 0, sizeof(clientInfo_t));
    ci->pXAnimTree = pXAnimTree;
    ci->infoValid = 1;
    ci->nextValid = 1;
    client->sess.connected = CON_CONNECTING;
    client->sess.scriptPersId = scriptPersId;
    client->sess.cs.team = (team_t)RETURN_ZERO32();
    client->sess.sessionState = SESS_STATE_SPECTATOR;
    client->spectatorClient = -1;
    client->sess.forceSpectatorClient = -1;
    client->sess.killCamEntity = -1;
    G_InitGentity(ent);
    ent->handler = 0;
    ent->client = client;
    ent->s.clientNum = clientNum;
    client->sess.cs.clientIndex = clientNum;
    client->ps.clientNum = clientNum;
    client->sess.moveSpeedScaleMultiplier = 1.0;
    client->ps.moveSpeedScaleMultiplier = client->sess.moveSpeedScaleMultiplier;
    if (client->ps.eFlags)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 344, 0, "%s", "!client->ps.eFlags");
    if (ent->r.svFlags)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 345, 0, "%s", "!ent->r.svFlags");
    ClientUserinfoChanged(clientNum);
    SV_GetUserinfo(clientNum, userinfo, 1024);
    if (client->sess.localClient
        || (value = Info_ValueForKey(userinfo, "password"), !*(_BYTE *)g_password->current.integer)
        || !I_stricmp(g_password->current.string, "none")
        || !strcmp(g_password->current.string, value))
    {
        Scr_PlayerConnect(ent);
        CalculateRanks();
        return 0;
    }
    else
    {
        G_FreeEntity(ent);
        return (char*)"GAME_INVALIDPASSWORD";
    }
}

void __cdecl ClientClearFields(gclient_s *client)
{
    EntHandle::setEnt(&client->useHoldEntity, 0);
}

void __cdecl ClientBegin(int clientNum)
{
    gclient_s *client; // [esp+0h] [ebp-4h]

    client = &level.clients[clientNum];
    client->sess.connected = CON_CONNECTED;
    client->ps.pm_type = 4;
    CalculateRanks();
    Scr_Notify(&g_entities[clientNum], scr_const.begin, 0);
}

void __cdecl ClientSpawn(gentity_s *ent, const float *spawn_origin, const float *spawn_angles)
{
    gclient_s *client; // [esp+14h] [ebp-12Ch]
    int index; // [esp+18h] [ebp-128h]
    int iFlags; // [esp+1Ch] [ebp-124h]
    clientSession_t savedSess; // [esp+20h] [ebp-120h] BYREF
    int savedSpawnCount; // [esp+138h] [ebp-8h]
    int savedServerTime; // [esp+13Ch] [ebp-4h]

    index = ent - g_entities;
    client = ent->client;
    if (client != &level.clients[index])
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 432, 0, "%s", "ent->client == &level.clients[index]");
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 433, 0, "%s", "ent->r.inuse");
    if ((client->ps.otherFlags & 4) != 0 && (client->ps.eFlags & 0x300) != 0)
    {
        if (client->ps.clientNum != ent->s.number)
            MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 438, 0, "%s", "client->ps.clientNum == ent->s.number");
        if (client->ps.viewlocked_entNum == 1023)
            MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 439, 0, "%s", "client->ps.viewlocked_entNum != ENTITYNUM_NONE");
        if (!EntHandle::isDefined(&level.gentities[client->ps.viewlocked_entNum].r.ownerNum))
            MyAssertHandler(
                ".\\game_mp\\g_client_mp.cpp",
                440,
                0,
                "%s",
                "level.gentities[client->ps.viewlocked_entNum].r.ownerNum.isDefined()");
        if (EntHandle::ent(&level.gentities[client->ps.viewlocked_entNum].r.ownerNum) != ent)
            MyAssertHandler(
                ".\\game_mp\\g_client_mp.cpp",
                441,
                0,
                "%s",
                "level.gentities[client->ps.viewlocked_entNum].r.ownerNum.ent() == ent");
        G_ClientStopUsingTurret(&level.gentities[client->ps.viewlocked_entNum]);
    }
    G_EntUnlink(ent);
    if (ent->r.linked)
        SV_UnlinkEntity(ent);
    ent->s.groundEntityNum = 1023;
    Scr_SetString(&ent->classname, scr_const.player);
    ent->clipmask = (int)&sv.svEntities[446].baseline.s.lerp.apos.trBase[2] + 1;
    ent->r.svFlags |= 1u;
    ent->takedamage = 0;
    G_SetClientContents(ent);
    ent->handler = 11;
    ent->flags = 4096;
    ent->r.mins[0] = -15.0;
    ent->r.mins[1] = -15.0;
    ent->r.mins[2] = 0.0;
    ent->r.maxs[0] = 15.0;
    ent->r.maxs[1] = 15.0;
    ent->r.maxs[2] = 70.0;
    iFlags = client->ps.eFlags & 0x100002;
    memcpy(&savedSess, &client->sess, sizeof(savedSess));
    savedSpawnCount = client->ps.stats[4];
    savedServerTime = client->lastServerTime;
    ClientClearFields(client);
    memset((unsigned __int8 *)client, 0, sizeof(gclient_s));
    memcpy(&client->sess, &savedSess, sizeof(client->sess));
    client->lastServerTime = savedServerTime;
    client->spectatorClient = -1;
    client->ps.stats[4] = savedSpawnCount + 1;
    client->ps.stats[2] = client->sess.maxHealth;
    client->ps.eFlags = iFlags;
    client->sess.cs.clientIndex = index;
    client->sess.cs.attachedVehEntNum = 1023;
    client->ps.clientNum = index;
    client->ps.viewlocked_entNum = 1023;
    SV_GetUsercmd(client - level.clients, &client->sess.cmd);
    client->ps.eFlags ^= 2u;
    client->ps.viewHeightTarget = 60;
    client->ps.viewHeightCurrent = 60.0;
    client->ps.viewHeightLerpTime = 0;
    client->ps.dofNearBlur = 6.0;
    client->ps.dofFarBlur = 1.8;
    client->ps.spreadOverride = 0;
    client->ps.spreadOverrideState = 0;
    client->ps.throwBackGrenadeTimeLeft = 0;
    client->ps.throwBackGrenadeOwner = 1023;
    G_SetOrigin(ent, spawn_origin);
    client->ps.origin[0] = *spawn_origin;
    client->ps.origin[1] = spawn_origin[1];
    client->ps.origin[2] = spawn_origin[2];
    client->ps.pm_flags |= 0x400u;
    SetClientViewAngle(ent, spawn_angles);
    client->inactivityTime = level.time + 1000 * g_inactivity->current.integer;
    if (client->latched_buttons)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 525, 0, "%s", "!client->latched_buttons");
    if (client->buttonsSinceLastFrame)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 526, 0, "%s", "!client->buttonsSinceLastFrame");
    client->buttons = client->sess.cmd.buttons;
    level.clientIsSpawning = 1;
    client->lastSpawnTime = level.time;
    client->sess.cmd.serverTime = level.time;
    client->ps.commandTime = level.time - 100;
    ClientEndFrame(ent);
    ClientThink_real(ent, &client->sess.cmd);
    level.clientIsSpawning = 0;
    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
}

void __cdecl ClientDisconnect(int clientNum)
{
    gclient_s *client; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    client = &level.clients[clientNum];
    ent = &g_entities[clientNum];
    if (!ent->r.inuse)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 570, 0, "%s", "ent->r.inuse");
    if (Scr_IsSystemActive())
    {
        Scr_AddString("disconnect");
        Scr_AddString("-1");
        Scr_Notify(ent, scr_const.menuresponse, 2u);
    }
    for (i = 0; i < level.maxclients; ++i)
    {
        if (level.clients[i].sess.connected
            && level.clients[i].sess.sessionState == SESS_STATE_SPECTATOR
            && level.clients[i].spectatorClient == clientNum)
        {
            StopFollowing(&g_entities[i]);
        }
    }
    if (ent->client != client)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 592, 0, "%s", "ent->client == client");
    HudElem_ClientDisconnect(ent);
    if (Scr_IsSystemActive())
        Scr_PlayerDisconnect(ent);
    G_FreeEntity(ent);
    ent->client = client;
    ClientClearFields(client);
    client->sess.connected = CON_DISCONNECTED;
    memset((unsigned __int8 *)&client->sess.cs, 0, sizeof(client->sess.cs));
    CalculateRanks();
    if (ent->client != client)
        MyAssertHandler(".\\game_mp\\g_client_mp.cpp", 612, 0, "%s", "ent->client == client");
}

unsigned int __cdecl G_GetNonPVSPlayerInfo(gentity_s *pSelf, float *vPosition, int iLastUpdateEnt)
{
    bool v4; // [esp+4h] [ebp-3Ch]
    team_t team; // [esp+14h] [ebp-2Ch]
    float fScale; // [esp+1Ch] [ebp-24h]
    float fScale_4; // [esp+20h] [ebp-20h]
    int iPos; // [esp+24h] [ebp-1Ch]
    int iPos_4; // [esp+28h] [ebp-18h]
    int iEntCount; // [esp+2Ch] [ebp-14h]
    gentity_s *pEnt; // [esp+30h] [ebp-10h]
    float vOfs; // [esp+34h] [ebp-Ch]
    float vOfs_4; // [esp+38h] [ebp-8h]
    int iBaseEnt; // [esp+3Ch] [ebp-4h]

    team = pSelf->client->sess.cs.team;
    if (team == TEAM_SPECTATOR)
        return 0;
    v4 = g_compassShowEnemies->current.enabled || level.teamHasRadar[team] || pSelf->client->hasRadar;
    if (team == TEAM_FREE && !v4)
        return 0;
    if (iLastUpdateEnt == 1023)
        iBaseEnt = 0;
    else
        iBaseEnt = iLastUpdateEnt + 1;
    for (iEntCount = 0; ; ++iEntCount)
    {
        if (iEntCount >= 64)
            return 0;
        pEnt = &g_entities[(iEntCount + iBaseEnt) % 64];
        if (pEnt->r.inuse
            && pEnt->client
            && pEnt->client->sess.sessionState == SESS_STATE_PLAYING
            && (pEnt->client->sess.cs.team == team || v4)
            && pSelf != pEnt
            && !SV_inSnapshot(vPosition, pEnt->s.number))
        {
            break;
        }
    }
    vOfs = pEnt->r.currentOrigin[0] - *vPosition;
    vOfs_4 = pEnt->r.currentOrigin[1] - vPosition[1];
    iPos = (int)(vOfs + 0.5);
    iPos_4 = (int)(vOfs_4 + 0.5);
    fScale = 1.0;
    fScale_4 = 1.0;
    if (iPos <= 1024)
    {
        if (iPos < -1022)
            fScale = -1022.0 / (double)iPos;
    }
    else
    {
        fScale = 1024.0 / (double)iPos;
    }
    if (iPos_4 <= 1024)
    {
        if (iPos_4 < -1022)
            fScale_4 = -1022.0 / (double)iPos_4;
    }
    else
    {
        fScale_4 = 1024.0 / (double)iPos_4;
    }
    if (fScale < 1.0 || fScale_4 < 1.0)
    {
        if (fScale_4 <= (double)fScale)
        {
            if (fScale > (double)fScale_4)
                iPos = (int)((double)iPos * fScale_4);
        }
        else
        {
            iPos_4 = (int)((double)iPos_4 * fScale);
        }
    }
    if (iPos <= 1024)
    {
        if (iPos < -1022)
            iPos = -1022;
    }
    else
    {
        iPos = 1024;
    }
    if (iPos_4 <= 1024)
    {
        if (iPos_4 < -1022)
            iPos_4 = -1022;
    }
    else
    {
        iPos_4 = 1024;
    }
 return ((int)(pEnt->r.currentAngles[1] * 0.7111111283302307) << 24)
       | (((((iPos_4 + 2) / 4) + 255) & 0x1FF) << 15) & 0xFFFFFF
       | (((((iPos + 2) / 4) + 255) & 0x1FF) << 6) & 0x7FFF
       | pEnt->s.number & 0x3F;
}

