#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include "g_main.h"
#include <script/scr_const.h>
#include <universal/com_math.h>
#include "actor.h"
#include <server/sv_game.h>

void __cdecl G_FinishSetupSpawnPoint(gentity_s *ent)
{
    unsigned __int16 EntityHitId; // r3
    double fraction; // fp0
    float start[3]; // [sp+58h] [-88h] BYREF
    float end[3]; // [sp+68h] [-78h] BYREF
    trace_t trace; // [sp+80h] [-60h] BYREF

    start[0] = ent->r.currentOrigin[0];
    start[1] = ent->r.currentOrigin[1];
    start[2] = ent->r.currentOrigin[2];

    end[0] = ent->r.currentOrigin[0];
    end[1] = ent->r.currentOrigin[1];
    end[2] = ent->r.currentOrigin[2] + 128.0f;

    G_TraceCapsule(&trace, start, playerMins, playerMaxs, end, ent->s.number, 42057745);

    start[0] = ((end[0] - start[0]) * trace.fraction) + start[0];
    end[0] = start[0];
    start[2] = ((end[2] - start[2]) * trace.fraction) + start[2];
    end[1] = ((end[1] - start[1]) * trace.fraction) + start[1];
    start[1] = (((end[1] - start[1]) * trace.fraction) + start[1]);
    end[2] = start[2] - 256.0f;
    G_TraceCapsule(&trace, start, playerMins, playerMaxs, end, ent->s.number, 42057745);
    EntityHitId = Trace_GetEntityHitId(&trace);
    fraction = trace.fraction;
    ent->s.groundEntityNum = EntityHitId;
    start[0] = ((end[0] - start[0]) * fraction) + start[0];
    start[1] = ((end[1] - start[1]) * fraction) + start[1];
    start[2] = ((end[2] - start[2]) * fraction) + start[2];
    g_entities[EntityHitId].flags |= FL_GROUND_ENT;
    G_TraceCapsule(&trace, start, playerMins, playerMaxs, start, ent->s.number, 42057745);

    if (trace.allsolid)
        Com_PrintWarning(
            15,
            "WARNING: Spawn point entity %i is in solid at (%i, %i, %i)\n",
            ent->s.number,
            (int)ent->r.currentOrigin[0],
            (int)ent->r.currentOrigin[1],
            (int)ent->r.currentOrigin[2]);

    G_SetOrigin(ent, start);
}

void __cdecl G_SetupSpawnPoint(gentity_s *ent)
{
    int time; // r11

    time = level.time;
    ent->handler = ENT_HANDLER_SPAWN_POINT;
    ent->nextthink = time + 100;
}

void __cdecl SP_info_player_start(gentity_s *ent)
{
    int time; // r11

    Scr_SetString(&ent->classname, scr_const.info_player_deathmatch);
    time = level.time;
    ent->handler = ENT_HANDLER_SPAWN_POINT;
    ent->nextthink = time + 100;
}

int __cdecl SpotWouldTelefrag(gentity_s *spot)
{
    double v1; // fp13
    int v2; // r3
    int v3; // r8
    int *i; // r9
    gentity_s *v5; // r11
    gclient_s *client; // r10
    float v8[4]; // [sp+50h] [-2230h] BYREF
    float v9[4]; // [sp+60h] [-2220h] BYREF
    int v10[132]; // [sp+70h] [-2210h] BYREF

    v9[0] = spot->r.currentOrigin[0] + (float)-15.0;
    v9[1] = spot->r.currentOrigin[1] + (float)-15.0;
    v9[2] = spot->r.currentOrigin[2] + (float)0.0;
    v1 = spot->r.currentOrigin[1];
    v8[0] = spot->r.currentOrigin[0] + (float)15.0;
    v8[1] = (float)v1 + (float)15.0;
    v8[2] = spot->r.currentOrigin[2] + (float)70.0;
    v2 = CM_AreaEntities(v9, v8, v10, MAX_GENTITIES, 33603584);
    v3 = 0;
    if (v2 <= 0)
        return 0;
    for (i = v10; ; ++i)
    {
        v5 = &g_entities[*i];
        client = v5->client;
        if (client)
        {
            if (client->ps.pm_type < 5)
                break;
        }
        if (v5->actor && v5->health > 0)
            break;
        if (++v3 >= v2)
            return 0;
    }
    return 1;
}

gentity_s *__cdecl SelectNearestDeathmatchSpawnPoint(const float *from)
{
    gentity_s *v2; // r29
    double v3; // fp31
    gentity_s *i; // r3
    double v5; // fp0
    double v6; // fp13
    double v7; // fp12
    double v8; // fp0

    v2 = 0;
    v3 = 999999.0;
    for (i = G_Find(0, 284, scr_const.info_player_deathmatch); i; i = G_Find(i, 284, scr_const.info_player_deathmatch))
    {
        v5 = (float)(i->r.currentOrigin[0] - *from);
        v6 = (float)(i->r.currentOrigin[2] - from[2]);
        v7 = (float)(i->r.currentOrigin[1] - from[1]);
        v8 = sqrtf((float)((float)((float)v7 * (float)v7) + (float)((float)((float)v6 * (float)v6) + (float)((float)v5 * (float)v5))));
        if (v8 < v3)
        {
            v3 = v8;
            v2 = i;
        }
    }
    return v2;
}

gentity_s *__cdecl SelectRandomDeathmatchSpawnPoint()
{
    signed int numSpots; // r30
    gentity_s *spot; // r31
    gentity_s **ptr; // r29
    int randnum; // r3
    gentity_s *spotPtrs[140]; // [sp+50h] [-230h] BYREF

    numSpots = 0;
    spot = G_Find(0, 284, scr_const.info_player_deathmatch);

    if (!spot)
        return G_Find(0, 284, scr_const.info_player_deathmatch);

    // Populate List of spots and count them
    ptr = &spotPtrs[0];
    do
    {
        if (!SpotWouldTelefrag(spot))
        {
            *ptr = spot;
            ++numSpots;
            ++ptr;
        }
        spot = G_Find(spot, 284, scr_const.info_player_deathmatch);
    } while (spot);

    if (!numSpots)
        return G_Find(0, 284, scr_const.info_player_deathmatch);

    randnum = G_rand();
    //__twllei(v0, 0);
    //__twlgei(v0 & ~(__ROL4__(v4, 1) - 1), 0xFFFFFFFF);

    //uint32_t rotated = (v4 << 1) | (v4 >> (32 - 1)); // rotate-left by 1
    //uint32_t mask = ~(rotated - 1);
    //v0 = v0 & mask;

    return (gentity_s *)spotPtrs[randnum % numSpots];
}

gentity_s *__cdecl SelectSpawnPoint(const float *avoidPoint, float *origin, float *angles)
{
    gentity_s *v5; // r30
    gentity_s *v6; // r31
    gentity_s *result; // r3

    v5 = SelectNearestDeathmatchSpawnPoint(avoidPoint);
    v6 = SelectRandomDeathmatchSpawnPoint();
    if (v6 == v5)
    {
        v6 = SelectRandomDeathmatchSpawnPoint();
        if (v6 == v5)
            v6 = SelectRandomDeathmatchSpawnPoint();
    }
    if (!v6)
        Com_Error(ERR_DROP, "Couldn't find a spawn point");
    *origin = v6->r.currentOrigin[0];
    result = v6;
    origin[1] = v6->r.currentOrigin[1];
    origin[2] = v6->r.currentOrigin[2] + (float)9.0;
    *angles = v6->r.currentAngles[0];
    angles[1] = v6->r.currentAngles[1];
    angles[2] = v6->r.currentAngles[2];
    return result;
}

gentity_s *__cdecl SelectInitialSpawnPoint(float *origin, float *angles)
{
    gentity_s *v4; // r31
    gentity_s *result; // r3

    v4 = G_Find(0, 284, scr_const.info_player_deathmatch);
    if (!v4)
        return SelectSpawnPoint(vec3_origin, origin, angles);
    while ((v4->spawnflags & 1) == 0)
    {
        v4 = G_Find(v4, 284, scr_const.info_player_deathmatch);
        if (!v4)
            return SelectSpawnPoint(vec3_origin, origin, angles);
    }
    if (SpotWouldTelefrag(v4))
        return SelectSpawnPoint(vec3_origin, origin, angles);
    *origin = v4->r.currentOrigin[0];
    result = v4;
    origin[1] = v4->r.currentOrigin[1];
    origin[2] = v4->r.currentOrigin[2] + (float)9.0;
    *angles = v4->r.currentAngles[0];
    angles[1] = v4->r.currentAngles[1];
    angles[2] = v4->r.currentAngles[2];
    return result;
}

void __cdecl SetClientOrigin(gentity_s *ent, float *origin)
{
    gclient_s *client; // r11
    gclient_s *v5; // r11

    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 268, 0, "%s", "ent->client");
    client = ent->client;
    client->ps.origin[0] = *origin;
    client->ps.origin[1] = origin[1];
    client->ps.origin[2] = origin[2];
    ent->client->ps.origin[2] = ent->client->ps.origin[2] + (float)1.0;
    ent->client->ps.eFlags ^= 2u;
    BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, 1, 1u);
    v5 = ent->client;
    ent->r.currentOrigin[0] = v5->ps.origin[0];
    ent->r.currentOrigin[1] = v5->ps.origin[1];
    ent->r.currentOrigin[2] = v5->ps.origin[2];
}

//void __cdecl InitClientDeltaAngles(gclient_s *client, long double a2)
//{
//    int v2; // r30
//    int *angles; // r31
//    __int64 v4; // r11
//    double v5; // fp31
//
//    v2 = 3;
//    angles = client->pers.cmd.angles;
//    do
//    {
//        LODWORD(v4) = *angles;
//        HIDWORD(v4) = angles - 11392;
//        v5 = (float)((float)-(float)((float)((float)v4 * (float)0.0054931641) - *((float *)angles - 11392))
//            * (float)0.0027777778);
//        *(double *)&a2 = (float)((float)((float)-(float)((float)((float)v4 * (float)0.0054931641)
//            - *((float *)angles - 11392))
//            * (float)0.0027777778)
//            + (float)0.5);
//        a2 = floor(a2);
//        --v2;
//        *((float *)angles++ - 11424) = (float)((float)v5 - (float)*(double *)&a2) * (float)360.0;
//    } while (v2);
//}

// aislop
void InitClientDeltaAngles(gclient_s *client)
{
    int i;
    int *angles = client->pers.cmd.angles;
    double a2;

    for (i = 0; i < 3; i++, angles++)
    {
        float curr = (float)*angles;
        float prev = (float)*(angles - 11392);
        float diff = (curr * 0.0054931641f) - prev;      // 0.0054931641 = 1/182 
        float turns = -diff * 0.0027777778f;             // 1/360
        a2 = turns + 0.5;
        a2 = floor(a2);
        float frac = turns - a2;
        client->pers.cmd.angles[i] = frac * 360.0f;
    }
}


void __cdecl SetClientViewAngle(gentity_s *ent, const float *angle)
{
    double v4; // fp31
    double v5; // fp30
    double v6; // fp28
    gclient_s *client; // r11
    double v8; // fp1
    double v9; // fp1
    const dvar_s *v10; // r11
    double value; // fp0
    double v12; // fp0
    gclient_s *v13; // r10
    double v14; // fp1
    double v15; // fp1
    double v16; // fp13
    gclient_s *v17; // r11
    double v18; // fp1
    gclient_s *v19; // r3

    v4 = *angle;
    v5 = angle[1];
    v6 = angle[2];
    client = ent->client;
    if ((client->ps.pm_flags & 1) != 0 && (client->ps.eFlags & 0x300) == 0)
    {
        v8 = AngleDelta(client->ps.proneDirection, angle[1]);
        v9 = AngleNormalize180(v8);
        v10 = bg_prone_yawcap;
        value = bg_prone_yawcap->current.value;
        if (v9 > value || v9 < -value)
        {
            if (v9 <= value)
                v12 = (float)(bg_prone_yawcap->current.value + (float)v9);
            else
                v12 = (float)((float)v9 - bg_prone_yawcap->current.value);
            v13 = ent->client;
            v13->ps.delta_angles[1] = v13->ps.delta_angles[1] + (float)v12;
            if (v12 <= 0.0)
                v14 = (float)(v13->ps.proneDirection + v10->current.value);
            else
                v14 = (float)(v13->ps.proneDirection - v10->current.value);
            v5 = AngleNormalize360(v14);
        }
        v15 = AngleDelta(ent->client->ps.proneTorsoPitch, v4);
        float normalized = AngleNormalize180(v15);
        if (normalized > 45.0 || normalized < -45.0)
        {
            if (normalized <= 45.0)
                v16 = (float)((float)normalized + (float)45.0);
            else
                v16 = (float)((float)normalized - (float)45.0);
            v17 = ent->client;
            v17->ps.delta_angles[0] = (float)v16 + v17->ps.delta_angles[0];
            if (v16 <= 0.0)
                v18 = (float)(v17->ps.proneTorsoPitch + (float)45.0);
            else
                v18 = (float)(v17->ps.proneTorsoPitch - (float)45.0);
            normalized = AngleNormalize180(v18);
            v4 = normalized;
        }
    }
    v19 = ent->client;
    ent->r.currentAngles[0] = v4;
    ent->r.currentAngles[1] = v5;
    ent->r.currentAngles[2] = v6;
    v19->ps.viewangles[0] = v4;
    v19->ps.viewangles[1] = ent->r.currentAngles[1];
    v19->ps.viewangles[2] = ent->r.currentAngles[2];
    InitClientDeltaAngles(v19);
}

void __cdecl G_GetPlayerViewOrigin(const playerState_s *ps, float *origin)
{
    double v4; // fp0

    if ((ps->eFlags & 0x300) != 0)
    {
        if (ps->viewlocked == PLAYERVIEWLOCK_NONE)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 382, 0, "%s", "ps->viewlocked");
        if (ps->viewlocked_entNum == ENTITYNUM_NONE)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp",
                383,
                0,
                "%s",
                "ps->viewlocked_entNum != ENTITYNUM_NONE");
        G_DObjGetWorldTagPos_CheckTagExists(&g_entities[ps->viewlocked_entNum], scr_const.tag_player, origin);
    }
    else
    {
        *origin = ps->origin[0];
        origin[1] = ps->origin[1];
        v4 = ps->origin[2];
        origin[2] = ps->origin[2];
        origin[2] = ps->viewHeightCurrent + (float)v4;
        AddLeanToPosition(origin, ps->viewangles[1], ps->leanf, 16.0, 20.0);
    }
}

void __cdecl G_GetPlayerViewDirection(const gentity_s *ent, float *forward, float *right, float *up)
{
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 403, 0, "%s", "ent");
    if (!ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 404, 0, "%s", "ent->client");
    BG_GetPlayerViewDirection(&ent->client->ps, forward, right, up);
}

int __cdecl Client_GetPushed(gentity_s *pSelf, gentity_s *pOther)
{
    gclient_s *client; // r10
    int result; // r3
    double v5; // fp13
    double v6; // fp12
    double v7; // fp0
    double v8; // fp31
    gclient_s *v9; // r11
    float v10; // [sp+50h] [-20h] BYREF
    float v11; // [sp+54h] [-1Ch]

    if ((pOther->r.contents & 0xC000) == 0)
        return 0;
    client = pSelf->client;
    if ((client->ps.eFlags & 0x300) != 0 || pSelf->tagInfo)
        return 0;
    if (level.time < client->lastTouchTime + 500 && level.time >= client->inControlTime + 20000)
        return 1;
    v5 = pOther->r.currentOrigin[1];
    v6 = pSelf->r.currentOrigin[1];
    v10 = pSelf->r.currentOrigin[0] - pOther->r.currentOrigin[0];
    v11 = (float)v6 - (float)v5;
    v7 = (float)((float)(v11 * v11) + (float)(v10 * v10));
    if (v7 >= 900.0)
        return 0;
    v8 = (float)((float)((float)30.0 - (float)sqrtf(v7)) * (float)20.0);
    Vec2Normalize(&v10);
    v9 = pSelf->client;
    result = 1;
    v9->ps.velocity[0] = v10 * (float)v8;
    v9->ps.velocity[1] = v11 * (float)v8;
    return result;
}

void __cdecl Client_Touch(gentity_s *pSelf, gentity_s *pOther, int bTouched)
{
    actor_s *actor; // r30

    if (!pSelf->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 456, 0, "%s", "pSelf->sentient");
    if (!pSelf->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 457, 0, "%s", "pSelf->client");
    if (!Client_GetPushed(pSelf, pOther))
        pSelf->client->inControlTime = level.time;
    pSelf->client->lastTouchTime = level.time;
    actor = pOther->actor;
    if (actor
        && !actor->pCloseEnt.isDefined()
        && !actor->bDontAvoidPlayer
        && (actor->Physics.iTraceMask & 0x2000000) != 0
        && actor->eState[actor->stateLevel] != AIS_TURRET)
    {
        if (!pOther->sentient)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 470, 0, "%s", "pOther->sentient");
        if (Actor_AtClaimNode(actor))
        {
            if (Vec2DistanceSq(pSelf->r.currentOrigin, pSelf->sentient->oldOrigin) >= 0.0099999998)
                return;
            Sentient_StealClaimNode(pSelf->sentient, pOther->sentient);
        }
        if (((1 << pOther->sentient->eTeam) & ~(1 << Sentient_EnemyTeam(pSelf->sentient->eTeam))) != 0)
            actor->pCloseEnt.setEnt(pSelf);
    }
}

void __cdecl respawn(gentity_s *ent)
{
    const dvar_s *v2; // r30
    const char *v3; // r3
    const char *v4; // r3
    const char *v5; // r3
    const dvar_s *Var; // r3
    const char *v7; // r3

    if (!g_reloading->current.integer && (!Dvar_GetInt("arcademode") || Dvar_GetInt("arcademode_lives") >= 0))
    {
        if (!ent->client)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 501, 0, "%s", "ent->client");
        Dvar_SetInt(g_reloading, 1);
        v2 = g_deathDelay;
        level.absoluteReloadDelayTime = Sys_Milliseconds() + v2->current.integer;
        v3 = va("snd_fade 0 %i", v2->current.integer);
        SV_GameSendServerCommand(-1, v3);
        SV_GameSendServerCommand(-1, "clear_blur");
        v4 = va("scr_blur %i %f %i %i", g_deathDelay->current.integer, 32.0, 0, 1);
        SV_GameSendServerCommand(-1, v4);
        v5 = va("time_slow %i %f %f", g_deathDelay->current.integer, 1.0, 0.1000000014901161);
        SV_GameSendServerCommand(-1, v5);
        Var = Dvar_FindVar("ui_deadquote");
        if (Var)
        {
            v7 = va("opendeadscreen %s", Var->current.string);
            SV_GameSendServerCommand(-1, v7);
        }
    }
}

char *__cdecl ClientConnect(int clientNum)
{
    gclient_s *v2; // r31
    gentity_s *v3; // r29
    int integer; // r10
    sentient_s *v5; // r30

    Path_InitPaths();
    v2 = &level.clients[clientNum];
    v3 = &g_entities[clientNum];
    memset(v2, 0, sizeof(gclient_s));
    v2->pers.connected = CON_CONNECTING;
    integer = g_player_maxhealth->current.integer;
    v2->ps.clientNum = (unsigned __int16)clientNum;
    v2->pers.maxHealth = integer;
    v2->ps.stats[2] = integer;
    if ((unsigned __int16)clientNum != clientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp",
            554,
            0,
            "%s",
            "client->ps.clientNum == clientNum");
    v2->ps.moveSpeedScaleMultiplier = 1.0;
    v2->pers.moveSpeedScaleMultiplier = 1.0;
    G_InitGentity(&g_entities[clientNum]);
    v3->client = v2;
    v3->handler = ENT_HANDLER_CLIENT;
    v5 = Sentient_Alloc();
    if (!v5)
        Com_Error(ERR_DROP, "no sentient for player");
    v3->sentient = v5;
    v5->attackerAccuracy = 1.0;
    v5->ent = v3;
    v5->maxVisibleDist = 8192.0;
    v5->eTeam = TEAM_ALLIES;
    if (v2->ps.eFlags)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 573, 0, "%s", "!client->ps.eFlags");
    if (v3->r.svFlags)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 574, 0, "%s", "!ent->r.svFlags");
    return 0;
}

void __cdecl ClientSpawn(gentity_s *ent)
{
    gclient_s *client; // r31
    int v3; // r23
    int v4; // r26
    int viewmodelIndex; // r25
    int maxHealth; // r11
    int v7; // r11
    long double v8; // fp2
    int v9; // r11
    int time; // r11
    sentient_s *sentient; // r11
    float v12[4]; // [sp+50h] [-100h] BYREF
    float v13[4]; // [sp+60h] [-F0h] BYREF
    _BYTE v14[124]; // [sp+70h] [-E0h] BYREF

    client = ent->client;
    v3 = ent - g_entities;
    if (client != &level.clients[v3])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp",
            623,
            0,
            "%s",
            "client == &level.clients[index]");
    if (!ent->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 624, 0, "%s", "ent->r.inuse");
    if (client->ps.clientNum != v3)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 625, 0, "%s", "client->ps.clientNum == index");
    SelectInitialSpawnPoint(v12, v13);
    v4 = ~client->ps.eFlags & 2;
    memcpy(v14, &client->pers, sizeof(v14));
    viewmodelIndex = client->ps.viewmodelIndex;
    memset(client, 0, sizeof(gclient_s));
    memcpy(&client->pers, v14, sizeof(client->pers));
    client->ps.viewmodelIndex = viewmodelIndex;
    client->ps.eFlags = v4;
    maxHealth = client->pers.maxHealth;
    client->ps.viewlocked_entNum = ENTITYNUM_NONE;
    client->groundTiltEntNum = ENTITYNUM_NONE;
    client->ps.stats[2] = maxHealth;
    ent->s.groundEntityNum = ENTITYNUM_NONE;
    ent->takedamage = 1;
    Scr_SetString(&ent->classname, scr_const.player);
    ent->r.contents = 0x2000000;
    ent->clipmask = 42057745;
    ent->flags = (FL_SUPPORTS_LINKTO | FL_OBSTACLE);
    ent->r.mins[0] = -15.0;
    ent->r.mins[1] = -15.0;
    ent->r.mins[2] = 0.0;
    ent->r.maxs[0] = 15.0;
    ent->r.maxs[1] = 15.0;
    ent->r.maxs[2] = 70.0;
    v7 = client->ps.stats[2];
    client->ps.viewHeightCurrent = 60.0;
    client->ps.viewHeightTarget = 60;
    client->ps.dofNearBlur = 6.0;
    client->ps.viewHeightLerpTime = 0;
    client->ps.dofFarBlur = 1.8;
    client->ps.spreadOverride = 0;
    client->ps.spreadOverrideState = 0;
    client->ps.throwBackGrenadeTimeLeft = 0;
    client->ps.throwBackGrenadeOwner = ENTITYNUM_NONE;
    client->ps.stats[0] = v7;
    ent->health = v7;
    G_SetOrigin(ent, v12);
    client->ps.origin[0] = v12[0];
    client->ps.origin[1] = v12[1];
    client->ps.origin[2] = v12[2];
    client->ps.pm_flags |= 0x400u;
    SV_GetUsercmd(client - level.clients, &client->pers.cmd);
    SetClientViewAngle(ent, v13);
    SV_LinkEntity(ent);
    v9 = client->ps.pm_flags | 0x100;
    client->ps.pm_time = 100;
    client->ps.pm_flags = v9;
    time = level.time;
    client->latched_buttons = 0;
    client->respawnTime = time;
    iassert(!client->useHoldEntity.isDefined());
    client->invulnerableEnabled = 1;
    level.clientIsSpawning = 1;
    client->playerLOSCheckPos[0] = 0.0;
    client->playerLOSCheckPos[1] = 0.0;
    client->playerLOSCheckDir[0] = 0.0;
    client->playerLOSCheckDir[1] = 0.0;
    client->playerLOSPosTime = 0;
    client->playerADSTargetTime = 0;
    client->ps.commandTime = level.time - 200;
    ClientThink(v3);
    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
    ent->r.currentOrigin[0] = client->ps.origin[0];
    ent->r.currentOrigin[1] = client->ps.origin[1];
    ent->r.currentOrigin[2] = client->ps.origin[2];
    sentient = ent->sentient;
    sentient->oldOrigin[0] = client->ps.origin[0];
    sentient->oldOrigin[1] = client->ps.origin[1];
    sentient->oldOrigin[2] = client->ps.origin[2];
    SV_LinkEntity(ent);
    ClientEndFrame(ent);
    level.clientIsSpawning = 0;
    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
}

void __cdecl HeadHitEnt_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int iDamage,
    const float *vPoint,
    int iMod,
    const float *vDir,
    const hitLocation_t hitLoc,
    const int weaponIdx)
{
    EntHandle *p_ownerNum; // r30
    gentity_s *target; // r3
    int v15; // [sp+8h] [-A8h]
    hitLocation_t v16; // [sp+Ch] [-A4h]
    unsigned int v17; // [sp+10h] [-A0h]
    unsigned int v18; // [sp+14h] [-9Ch]

    p_ownerNum = &pSelf->r.ownerNum;
    pSelf->health = 99999;
    iassert(pSelf->r.ownerNum.isDefined());
    target = p_ownerNum->ent();
    if (target->takedamage)
        G_Damage(target, pAttacker, pAttacker, vDir, vPoint, iDamage, 0, iMod, -1/*Weapon*/, hitLoc, 0, 0);
}

void __cdecl HeadHitEnt_Die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int meansOfDeath,
    int iWeapon,
    const float *vDir,
    hitLocation_t hitLoc)
{
    HeadHitEnt_Pain(self, attacker, damage, self->r.currentOrigin, meansOfDeath, vDir, hitLoc, iWeapon);
}

void __cdecl G_UpdateHeadHitEnt(gentity_s *pSelf)
{
    gentity_s *pHitHitEnt; // r31
    float v3[12]; // [sp+50h] [-30h] BYREF

    if (!pSelf->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_client.cpp", 773, 0, "%s", "pSelf->client");
    G_GetPlayerViewOrigin(&pSelf->client->ps, v3);
    pHitHitEnt = pSelf->client->pHitHitEnt;
    if (!pHitHitEnt)
    {
        pSelf->client->pHitHitEnt = G_Spawn();
        pHitHitEnt = pSelf->client->pHitHitEnt;
        pHitHitEnt->r.mins[0] = -8.0;
        pHitHitEnt->r.mins[1] = -8.0;
        pHitHitEnt->r.mins[2] = -8.0;
        pHitHitEnt->r.maxs[0] = 8.0;
        pHitHitEnt->r.maxs[1] = 8.0;
        pHitHitEnt->r.maxs[2] = 8.0;
        pHitHitEnt->r.contents = 8320;
        pHitHitEnt->r.ownerNum.setEnt(pSelf);
        pHitHitEnt->handler = ENT_HANDLER_HEAD_HIT;
        pHitHitEnt->health = 99999;
        pHitHitEnt->takedamage = 1;
    }
    G_SetOrigin(pHitHitEnt, v3);
    SV_LinkEntity(pHitHitEnt);
}

void __cdecl G_RemoveHeadHitEnt(gentity_s *pSelf)
{
    gentity_s *pHitHitEnt; // r3

    pHitHitEnt = pSelf->client->pHitHitEnt;
    if (pHitHitEnt)
    {
        G_FreeEntity(pHitHitEnt);
        pSelf->client->pHitHitEnt = 0;
    }
}

void __cdecl ClientBegin(int clientNum)
{
    level.clients[clientNum].pers.connected = CON_CONNECTED;
    ClientSpawn(&g_entities[clientNum]);
}

