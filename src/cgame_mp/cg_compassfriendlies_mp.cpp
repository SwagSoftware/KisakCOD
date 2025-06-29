#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <qcommon/mem_track.h>
#include <cgame/cg_public.h>
#include <ui/ui.h>
#include <client/client.h>
#include <universal/profile.h>

CompassActor s_compassActors[1][64];
CompassVehicle s_compassVehicles[1][8];

void __cdecl TRACK_cg_compassfriendlies()
{
    track_static_alloc_internal(s_compassActors, 3072, "s_compassActors", 9);
    track_static_alloc_internal(s_compassVehicles, 224, "s_compassVehicles", 9);
}

void __cdecl CG_ClearCompassPingData()
{
    memset((unsigned __int8 *)s_compassActors, 0, sizeof(s_compassActors));
    memset((unsigned __int8 *)s_compassVehicles, 0, sizeof(s_compassVehicles));
}

void __cdecl CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex)
{
    CompassVehicle *Vehicle; // eax
    centity_s *cent; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->nextSnap);

    cent = CG_GetEntity(localClientNum, entityIndex);
    iassert(cent->nextState.eType == ET_VEHICLE || cent->nextState.eType == ET_HELICOPTER || cent->nextState.eType == ET_PLANE);

    Vehicle = GetVehicle(localClientNum, entityIndex);
    Vehicle->lastUpdate = cgameGlob->time;
    *(double *)Vehicle->lastPos = *(double *)cent->pose.origin;
    Vehicle->lastYaw = cent->pose.angles[1];
    Vehicle->team = (team_t)(cent->nextState.lerp.u.vehicle.teamAndOwnerIndex & 3);
    Vehicle->ownerIndex = cent->nextState.lerp.u.vehicle.teamAndOwnerIndex >> 2;
}

CompassVehicle *__cdecl GetVehicle(int localClientNum, int entityNum)
{
    CompassVehicle *v3; // edx
    int veh; // [esp+4h] [ebp-8h]
    int veha; // [esp+4h] [ebp-8h]
    int vehb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    for (veh = 0; veh < 8; ++veh)
    {
        if (s_compassVehicles[localClientNum][veh].entityNum == entityNum)
            return &s_compassVehicles[localClientNum][veh];
    }
    for (veha = 0; veha < 8 && s_compassVehicles[localClientNum][veha].lastUpdate; ++veha)
        ;
    if (veha >= 8)
    {
        oldest = 0;
        for (vehb = 1; vehb < 8; ++vehb)
        {
            if (s_compassVehicles[localClientNum][vehb].lastUpdate < s_compassVehicles[localClientNum][oldest].lastUpdate)
                oldest = vehb;
        }
        veha = oldest;
    }
    v3 = &s_compassVehicles[localClientNum][veha];
    v3->entityNum = 0;
    v3->lastUpdate = 0;
    v3->lastPos[0] = 0.0;
    v3->lastPos[1] = 0.0;
    v3->lastYaw = 0.0;
    v3->team = TEAM_FREE;
    v3->ownerIndex = 0;
    v3->entityNum = entityNum;
    return v3;
}

void __cdecl CG_CompassRadarPingEnemyPlayers(int localClientNum, float oldRadarProgress, float newRadarProgress)
{
    bool v3; // [esp+8h] [ebp-34h]
    centity_s *cent; // [esp+Ch] [ebp-30h]
    clientInfo_t *localClientInfo; // [esp+10h] [ebp-2Ch]
    unsigned int actorIndex; // [esp+1Ch] [ebp-20h]
    float radarLine1[3]; // [esp+20h] [ebp-1Ch] BYREF
    CompassActor *actor; // [esp+2Ch] [ebp-10h]
    float radarLine2[3]; // [esp+30h] [ebp-Ch] BYREF
    cg_s *cgameGlob;

    if (newRadarProgress >= (double)oldRadarProgress)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
        localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
        GetRadarLine(cgameGlob, oldRadarProgress, radarLine1);
        GetRadarLine(cgameGlob, newRadarProgress, radarLine2);
        bcassert(localClientNum, STATIC_MAX_LOCAL_CLIENTS);
        actor = s_compassActors[localClientNum];
        for (actorIndex = 0; (int)actorIndex < 64; ++actorIndex)
        {
            bcassert(actorIndex, MAX_CLIENTS);
            if (cgameGlob->bgs.clientinfo[actorIndex].infoValid)
            {
                v3 = localClientInfo->team == TEAM_FREE || cgameGlob->bgs.clientinfo[actorIndex].team != localClientInfo->team;
                actor->enemy = v3;
                actor->perks = cgameGlob->bgs.clientinfo[actorIndex].perks;
                if (actor->enemy)
                {
                    cent = CG_GetEntity(localClientNum, actorIndex);
                    if (!cent->nextValid || cent->nextState.eType == 1 && (cent->nextState.lerp.eFlags & 0x20000) == 0)
                    {
                        if (DoLinesSurroundPoint(cgameGlob, radarLine1, radarLine2, actor->lastPos))
                            RadarPingEnemyPlayer(actor, cgameGlob->time);
                    }
                }
            }
            ++actor;
        }
    }
}

void __cdecl GetRadarLine(cg_s *cgameGlob, float radarProgress, float *line)
{
    float v3; // [esp+0h] [ebp-18h]
    float margin; // [esp+14h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame_mp\\cg_compassfriendlies_mp.cpp", 113, 0, "%s", "cgameGlob");
    margin = GetRadarLineMargin(cgameGlob);
    *line = cgameGlob->compassNorth[1];
    line[1] = -cgameGlob->compassNorth[0];
    v3 = line[1] * cgameGlob->compassMapUpperLeft[1] + *line * cgameGlob->compassMapUpperLeft[0];
    line[2] = (margin * 2.0 + cgameGlob->compassMapWorldSize[0]) * radarProgress + v3 - margin;
}

double __cdecl GetRadarLineMargin(cg_s *cgameGlob)
{
    static float SQRT2 = sqrt(2.0);

    float v2; // [esp+0h] [ebp-10h]
    float marginForRadara; // [esp+4h] [ebp-Ch]
    float marginForRadar; // [esp+4h] [ebp-Ch]
    float marginForMap; // [esp+8h] [ebp-8h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame_mp\\cg_compassfriendlies_mp.cpp", 83, 0, "%s", "cgameGlob");
    if (cgameGlob->compassMapWorldSize[0] == 0.0)
        MyAssertHandler(".\\cgame_mp\\cg_compassfriendlies_mp.cpp", 84, 0, "%s", "cgameGlob->compassMapWorldSize[0]");
    marginForRadara = compassRadarLineThickness->current.value * compassMaxRange->current.value;
    marginForRadar = compassMaxRange->current.value * SQRT2 + marginForRadara;
    marginForMap = cg_hudMapRadarLineThickness->current.value * cgameGlob->compassMapWorldSize[0];
    if (marginForMap >= (double)marginForRadar)
        v2 = cg_hudMapRadarLineThickness->current.value * cgameGlob->compassMapWorldSize[0];
    else
        v2 = marginForRadar;
    return (float)(v2 * 0.5);
}

bool __cdecl DoLinesSurroundPoint(cg_s *cgameGlob, float *radarLine1, float *radarLine2, float *pos)
{
    float v5; // [esp+8h] [ebp-10h]
    float v6; // [esp+Ch] [ebp-Ch]
    float v2; // [esp+10h] [ebp-8h]
    float v1; // [esp+14h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame_mp\\cg_compassfriendlies_mp.cpp", 148, 0, "%s", "cgameGlob");
    v6 = pos[1] * radarLine1[1] + *pos * *radarLine1;
    v1 = v6 - radarLine1[2];
    v5 = pos[1] * radarLine2[1] + *pos * *radarLine2;
    v2 = v5 - radarLine2[2];
    return v1 < 0.0 != v2 < 0.0;
}

void __cdecl RadarPingEnemyPlayer(CompassActor *actor, int time)
{
    if (!actor)
        MyAssertHandler(".\\cgame_mp\\cg_compassfriendlies_mp.cpp", 159, 0, "%s", "actor");
    if (actor->lastUpdate > time)
        actor->lastUpdate = 0;
    if (actor->lastUpdate >= time - 1500 && (actor->perks & 1) == 0)
    {
        *(double *)actor->lastEnemyPos = *(double *)actor->lastPos;
        actor->beginRadarFadeTime = time;
    }
}

void __cdecl CG_CompassIncreaseRadarTime(int localClientNum)
{
    float v1; // [esp+8h] [ebp-14h]
    float oldRadarProgress; // [esp+14h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob);

    if (cgameGlob->predictedPlayerState.radarEnabled)
    {
        oldRadarProgress = cgameGlob->radarProgress;
        cgameGlob->radarProgress = (double)cgameGlob->frametime / (compassRadarUpdateTime->current.value * 1000.0)
            + oldRadarProgress;
        v1 = floor(cgameGlob->radarProgress);
        cgameGlob->radarProgress = cgameGlob->radarProgress - v1;
        CG_CompassRadarPingEnemyPlayers(localClientNum, oldRadarProgress, cgameGlob->radarProgress);
    }
    else
    {
        cgameGlob->radarProgress = 0.0;
    }
}

void __cdecl CG_CompassAddWeaponPingInfo(int localClientNum, const centity_s *cent, const float *origin, int msec)
{
    bool v4; // [esp+0h] [ebp-20h]
    int playerIndex; // [esp+8h] [ebp-18h]
    clientInfo_t *localClientInfo; // [esp+Ch] [ebp-14h]
    team_t playerTeam; // [esp+14h] [ebp-Ch]
    CompassActor *actor; // [esp+1Ch] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cent);

    if (cent->nextState.eType != 2)
    {
        iassert(cent->nextState.eType == ET_PLAYER);
        bcassert(cent->nextState.number, MAX_CLIENTS);
        bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);

        localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];

        if (cgameGlob->nextSnap->ps.clientNum != cent->nextState.clientNum)
        {
            playerIndex = cent->nextState.number;
            playerTeam = cgameGlob->bgs.clientinfo[cent->nextState.clientNum].team;
            if (playerTeam != TEAM_SPECTATOR)
            {
                iassert(playerTeam == TEAM_ALLIES || playerTeam == TEAM_AXIS || playerTeam == TEAM_FREE);
                actor = &s_compassActors[localClientNum][playerIndex];
                actor->beginFadeTime = msec + cgameGlob->time;
                v4 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                actor->enemy = v4;
                actor->perks = cgameGlob->bgs.clientinfo[playerIndex].perks;
                if (actor->enemy)
                {
                    ActorUpdatePos(localClientNum, actor, origin, cent->nextState.clientNum);
                    actor->lastEnemyPos[0] = *origin;
                    actor->lastEnemyPos[1] = origin[1];
                }
            }
        }
    }
}

void __cdecl ActorUpdatePos(int localClientNum, CompassActor *actor, const float *newPos, int actorClientIndex)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(actor);
    iassert(actorClientIndex < MAX_CLIENTS);

    if (actor->enemy)
    {
        if (cgameGlob->predictedPlayerState.radarEnabled
            && actor->lastUpdate > cgameGlob->time - 1500
            && DoesMovementCrossRadar(cgameGlob, cgameGlob->radarProgress, actor->lastPos, newPos))
        {
            RadarPingEnemyPlayer(actor, cgameGlob->time);
        }
        if (CanLocalPlayerHearActorFootsteps(localClientNum, newPos, actorClientIndex))
            RadarPingEnemyPlayer(actor, cgameGlob->time);
    }
    *(double *)actor->lastPos = *(double *)newPos;
}

bool __cdecl DoesMovementCrossRadar(cg_s *cgameGlob, float radarProgress, const float *p1, const float *p2)
{
    float v5; // [esp+10h] [ebp-1Ch]
    float v6; // [esp+14h] [ebp-18h]
    float radarLine[3]; // [esp+18h] [ebp-14h] BYREF
    float v2; // [esp+24h] [ebp-8h]
    float v1; // [esp+28h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame_mp\\cg_compassfriendlies_mp.cpp", 133, 0, "%s", "cgameGlob");
    GetRadarLine(cgameGlob, radarProgress, radarLine);
    v6 = p1[1] * radarLine[1] + *p1 * radarLine[0];
    v1 = v6 - radarLine[2];
    v5 = p2[1] * radarLine[1] + *p2 * radarLine[0];
    v2 = v5 - radarLine[2];
    return v1 < 0.0 != v2 < 0.0;
}

bool __cdecl CanLocalPlayerHearActorFootsteps(int localClientNum, const float *actorPos, unsigned int actorClientIndex)
{
    float v4; // [esp+4h] [ebp-74h]
    float v5; // [esp+Ch] [ebp-6Ch]
    float v6; // [esp+10h] [ebp-68h]
    float v7; // [esp+18h] [ebp-60h]
    float v8; // [esp+28h] [ebp-50h]
    float delta[3]; // [esp+30h] [ebp-48h] BYREF
    cg_s *cgameGlob; // [esp+3Ch] [ebp-3Ch]
    const centity_s *cent; // [esp+40h] [ebp-38h]
    const snapshot_s *nextSnap; // [esp+44h] [ebp-34h]
    float xyvel[2]; // [esp+48h] [ebp-30h]
    const snapshot_s *snap; // [esp+50h] [ebp-28h]
    float currentPos[3]; // [esp+54h] [ebp-24h] BYREF
    const playerState_s *ps; // [esp+60h] [ebp-18h]
    float nextPos[3]; // [esp+64h] [ebp-14h] BYREF
    float deltaTime; // [esp+70h] [ebp-8h]
    float xyspeedSq; // [esp+74h] [ebp-4h]

    if (!compassEnemyFootstepEnabled->current.enabled)
        return 0;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if (cgameGlob->predictedPlayerState.pm_type)
        return 0;
    if ((cgameGlob->bgs.clientinfo[actorClientIndex].perks & 0x100) != 0)
        return 0;
    Vec3Sub(actorPos, ps->origin, delta);
    v8 = delta[0] * delta[0] + delta[1] * delta[1];
    v6 = compassEnemyFootstepMaxRange->current.value * compassEnemyFootstepMaxRange->current.value;
    if (v6 < (double)v8)
        return 0;
    v5 = fabs(delta[2]);
    if (compassEnemyFootstepMaxZ->current.value < (double)v5)
        return 0;
    snap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;
    deltaTime = (double)(nextSnap->serverTime - snap->serverTime) * EQUAL_EPSILON;
    if (deltaTime <= 0.0)
        return 0;
    cent = CG_GetEntity(localClientNum, actorClientIndex);
    BG_EvaluateTrajectory(&cent->currentState.pos, snap->serverTime, currentPos);
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, nextSnap->serverTime, nextPos);
    xyvel[0] = nextPos[0] - currentPos[0];
    xyvel[1] = nextPos[1] - currentPos[1];
    v7 = 1.0 / deltaTime;
    xyvel[0] = v7 * xyvel[0];
    xyvel[1] = v7 * xyvel[1];
    xyspeedSq = xyvel[1] * xyvel[1] + xyvel[0] * xyvel[0];
    v4 = compassEnemyFootstepMinSpeed->current.value * compassEnemyFootstepMinSpeed->current.value;
    return v4 <= (double)xyspeedSq;
}

void __cdecl CG_CompassUpdateActors(int localClientNum)
{
    bool v1; // [esp+Ch] [ebp-4Ch]
    bool v2; // [esp+10h] [ebp-48h]
    clientInfo_t *localClientInfo; // [esp+18h] [ebp-40h]
    float newPos[2]; // [esp+1Ch] [ebp-3Ch] BYREF
    float clientPos[3]; // [esp+24h] [ebp-34h]
    int team; // [esp+30h] [ebp-28h]
    cg_s *cgameGlob; // [esp+34h] [ebp-24h]
    const centity_s *cent; // [esp+38h] [ebp-20h]
    int playerTeam; // [esp+3Ch] [ebp-1Ch]
    float posDelta[2]; // [esp+40h] [ebp-18h]
    int index; // [esp+48h] [ebp-10h]
    int clientIndex; // [esp+4Ch] [ebp-Ch]
    CompassActor *actor; // [esp+50h] [ebp-8h]
    int num; // [esp+54h] [ebp-4h]

    PROF_SCOPED("CG_CompassUpdateActors");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->nextSnap);
    bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);

    if (cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid)
    {
        bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
        team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
        if (team != 3)
        {
            clientIndex = localClientNum;
            for (num = 0; num < cgameGlob->nextSnap->numEntities; ++num)
            {
                index = cgameGlob->nextSnap->entities[num].number;
                iassert(index >= 0);
                if ((unsigned int)index < MAX_CLIENTS)
                {
                    if (cgameGlob->bgs.clientinfo[index].infoValid)
                    {
                        playerTeam = cgameGlob->bgs.clientinfo[index].team;
                        bcassert(clientIndex, STATIC_MAX_LOCAL_CLIENTS);
                        bcassert(index, MAX_COMPASS_ACTORS);
                        actor = &s_compassActors[clientIndex][index];
                        v2 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                        actor->enemy = v2;
                        actor->perks = cgameGlob->bgs.clientinfo[num].perks;
                        cent = CG_GetEntity(localClientNum, index);
                        if (cent->nextState.eType == 1 && (cent->nextState.lerp.eFlags & 0x20000) == 0)
                        {
                            iassert(cent->nextState.clientNum == index);
                            actor = &s_compassActors[clientIndex][index];
                            ActorUpdatePos(localClientNum, actor, cent->pose.origin, index);
                            actor->lastUpdate = cgameGlob->time;
                            actor->lastYaw = cent->pose.angles[1];
                            if (CL_IsPlayerTalking(localClientNum, index))
                                actor->beginVoiceFadeTime = cgameGlob->time + 50;
                            if ((cent->nextState.lerp.eFlags & 0x400000) != 0 && actor->pingTime <= cgameGlob->time)
                                actor->pingTime = cgameGlob->time + 3000;
                        }
                    }
                }
            }
            if (cgameGlob->nextSnap->ps.iCompassPlayerInfo)
            {
                index = cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0x3F;
                actor = &s_compassActors[clientIndex][index];
                playerTeam = cgameGlob->bgs.clientinfo[index].team;
                v1 = localClientInfo->team == TEAM_FREE || playerTeam != localClientInfo->team;
                actor->enemy = v1;
                actor->perks = cgameGlob->bgs.clientinfo[index].perks;
                posDelta[0] = (float)(4 * ((cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0x7FC0) >> 6) - 1020);
                posDelta[1] = (4 * ((cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0xFF8000) >> 15) - 1020);
                clientPos[0] = cgameGlob->refdef.vieworg[0];
                clientPos[1] = cgameGlob->refdef.vieworg[1];
                clientPos[2] = cgameGlob->refdef.vieworg[2];
                newPos[0] = posDelta[0] + clientPos[0];
                newPos[1] = posDelta[1] + clientPos[1];
                ActorUpdatePos(localClientNum, actor, newPos, index);
                actor->lastYaw = (double)((int)(cgameGlob->nextSnap->ps.iCompassPlayerInfo & 0xFF000000) >> 24) * 1.40625;
                actor->lastUpdate = cgameGlob->time;
                if ((cgameGlob->nextSnap->ps.eFlags & 0x800000) != 0 && actor->pingTime <= cgameGlob->time)
                    actor->pingTime = cgameGlob->time + 3000;
            }
        }
    }
}

void __cdecl CG_CompassDrawFriendlies(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color)
{
    float v5; // [esp+24h] [ebp-E8h]
    float v6; // [esp+28h] [ebp-E4h]
    float v7; // [esp+2Ch] [ebp-E0h]
    float v8; // [esp+30h] [ebp-DCh]
    float v9; // [esp+34h] [ebp-D8h]
    float v10; // [esp+38h] [ebp-D4h]
    float v11; // [esp+44h] [ebp-C8h]
    float angle; // [esp+48h] [ebp-C4h]
    float v13; // [esp+4Ch] [ebp-C0h]
    float v14; // [esp+50h] [ebp-BCh]
    BOOL icon; // [esp+68h] [ebp-A4h]
    float yawVector[2]; // [esp+6Ch] [ebp-A0h] BYREF
    bool clipped; // [esp+77h] [ebp-95h]
    clientInfo_t *localClientInfo; // [esp+78h] [ebp-94h]
    float xy[2]; // [esp+7Ch] [ebp-90h] BYREF
    float baseColorModdedByComapassFadeOut[4]; // [esp+84h] [ebp-88h] BYREF
    int team; // [esp+94h] [ebp-78h]
    int timeDiff; // [esp+98h] [ebp-74h]
    const ScreenPlacement *scrPlace; // [esp+9Ch] [ebp-70h]
    cg_s *cgameGlob; // [esp+A0h] [ebp-6Ch]
    float dist; // [esp+A4h] [ebp-68h]
    float compassFadeOutAlpha; // [esp+A8h] [ebp-64h]
    float yawTo; // [esp+ACh] [ebp-60h]
    float posDelta[2]; // [esp+B0h] [ebp-5Ch] BYREF
    Material *firingMaterial; // [esp+B8h] [ebp-54h]
    float voiceFade; // [esp+BCh] [ebp-50h]
    Material *voiceMaterial; // [esp+C0h] [ebp-4Ch]
    float centerY; // [esp+C4h] [ebp-48h]
    int clientIndex; // [esp+C8h] [ebp-44h]
    CompassActor *actor; // [esp+CCh] [ebp-40h]
    int num; // [esp+D0h] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+D4h] [ebp-38h] BYREF
    float centerX; // [esp+ECh] [ebp-20h]
    float h; // [esp+F0h] [ebp-1Ch] BYREF
    float fadedColor[4]; // [esp+F4h] [ebp-18h] BYREF
    float firingFade; // [esp+104h] [ebp-8h]
    float w; // [esp+108h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->nextSnap);
    bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    CG_CompassUpYawVector(cgameGlob, yawVector);
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (compassFadeOutAlpha != 0.0f)
    {
        bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
        if (cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid)
        {
            bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
            team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
            if (team != 3 && team)
            {
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &scaledRect.x,
                    &scaledRect.y,
                    &scaledRect.w,
                    &scaledRect.h);
                centerX = scaledRect.w * 0.5 + scaledRect.x;
                centerY = scaledRect.h * 0.5 + scaledRect.y;
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                clientIndex = localClientNum;
                if (color[3] > (double)compassFadeOutAlpha)
                    color[3] = compassFadeOutAlpha;
                if (fadedColor[3] > (double)compassFadeOutAlpha)
                    fadedColor[3] = compassFadeOutAlpha;
                actor = s_compassActors[clientIndex];
                num = 0;
                while (num < 64)
                {
                    if (actor->lastUpdate > cgameGlob->time)
                        actor->lastUpdate = 0;
                    if (actor->lastUpdate >= cgameGlob->time - 1500 && !actor->enemy && cgameGlob->nextSnap->ps.clientNum != num)
                    {
                        v14 = fabs(actor->lastPos[0]);
                        if (v14 > 1.0 || (v13 = fabs(actor->lastPos[1]), v13 > 1.0))
                        {
                            posDelta[0] = actor->lastPos[0] - cgameGlob->refdef.vieworg[0];
                            posDelta[1] = actor->lastPos[1] - cgameGlob->refdef.vieworg[1];
                            dist = Vec2Length(posDelta);
                            fadedColor[3] = dist;
                            if (compassObjectiveMaxRange->current.value >= (double)dist)
                            {
                                if (compassMaxRange->current.value > (double)fadedColor[3])
                                    fadedColor[3] = compassMaxRange->current.value;
                            }
                            else
                            {
                                fadedColor[3] = compassObjectiveMaxRange->current.value;
                            }
                            fadedColor[3] = fadedColor[3] - compassMaxRange->current.value;
                            fadedColor[3] = fadedColor[3] / (compassObjectiveMaxRange->current.value - compassMaxRange->current.value);
                            fadedColor[3] = (compassObjectiveMinAlpha->current.value - 1.0) * fadedColor[3] + 1.0;
                        }
                        else
                        {
                            fadedColor[3] = (1.0 - compassObjectiveMinAlpha->current.value) * 0.5
                                + compassObjectiveMinAlpha->current.value;
                        }
                        clipped = CG_WorldPosToCompass(
                            compassType,
                            cgameGlob,
                            &scaledRect,
                            yawVector,
                            cgameGlob->refdef.vieworg,
                            actor->lastPos,
                            0,
                            xy);
                        if (!clipped || compassClampIcons->current.enabled)
                        {
                            CalcCompassFriendlySize(compassType, &w, &h);
                            xy[0] = centerX - w * 0.5 + xy[0];
                            xy[1] = centerY - h * 0.5 + xy[1];
                            if (compassType || !compassRotation->current.enabled)
                            {
                                v11 = cgameGlob->compassNorthYaw - actor->lastYaw;
                                yawTo = AngleNormalize360(v11);
                            }
                            else
                            {
                                angle = cgameGlob->refdefViewAngles[1] - actor->lastYaw;
                                yawTo = AngleNormalize360(angle);
                            }
                            if (actor->pingTime <= cgameGlob->time)
                            {
                                icon = 0;
                            }
                            else
                            {
                                timeDiff = (actor->pingTime - cgameGlob->time) % 500;
                                icon = timeDiff >= 250;
                            }
                            firingMaterial = 0;
                            firingFade = 0.0;
                            if ((double)cgameGlob->time <= (double)actor->beginFadeTime
                                + compassSoundPingFadeTime->current.value * 1000.0)
                            {
                                if (actor->beginFadeTime < cgameGlob->time)
                                    firingFade = 1.0
                                    - (double)(cgameGlob->time - actor->beginFadeTime)
                                    / (compassSoundPingFadeTime->current.value
                                        * 1000.0);
                                else
                                    firingFade = 1.0;
                                firingMaterial = cgMedia.compassping_friendlyfiring;
                            }
                            voiceMaterial = 0;
                            voiceFade = 0.0;
                            if ((double)cgameGlob->time <= (double)actor->beginVoiceFadeTime
                                + compassSoundPingFadeTime->current.value * 1000.0)
                            {
                                if (actor->beginVoiceFadeTime < cgameGlob->time)
                                    voiceFade = 1.0
                                    - (double)(cgameGlob->time - actor->beginVoiceFadeTime)
                                    / (compassSoundPingFadeTime->current.value
                                        * 1000.0);
                                else
                                    voiceFade = 1.0;
                                voiceMaterial = cgMedia.compassping_friendlyyelling;
                            }
                            baseColorModdedByComapassFadeOut[0] = *color;
                            baseColorModdedByComapassFadeOut[1] = color[1];
                            baseColorModdedByComapassFadeOut[2] = color[2];
                            baseColorModdedByComapassFadeOut[3] = color[3];
                            v10 = compassFadeOutAlpha - baseColorModdedByComapassFadeOut[3];
                            if (v10 < 0.0)
                                v9 = compassFadeOutAlpha;
                            else
                                v9 = baseColorModdedByComapassFadeOut[3];
                            baseColorModdedByComapassFadeOut[3] = v9;
                            v8 = compassFadeOutAlpha - firingFade;
                            if (v8 < 0.0)
                                v7 = compassFadeOutAlpha;
                            else
                                v7 = firingFade;
                            firingFade = v7;
                            v6 = compassFadeOutAlpha - voiceFade;
                            if (v6 < 0.0)
                                v5 = compassFadeOutAlpha;
                            else
                                v5 = voiceFade;
                            voiceFade = v5;
                            if (icon)
                            {
                                UI_DrawHandlePic(
                                    scrPlace,
                                    xy[0],
                                    xy[1],
                                    w,
                                    h,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    fadedColor,
                                    cgMedia.friendMaterials[1]);
                            }
                            else
                            {
                                if ((!firingMaterial || firingFade != 1.0) && (!voiceMaterial || voiceFade != 1.0))
                                    CG_DrawRotatedPic(
                                        scrPlace,
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        baseColorModdedByComapassFadeOut,
                                        cgMedia.friendMaterials[icon]);
                                if (voiceMaterial)
                                {
                                    fadedColor[3] = voiceFade;
                                    CG_DrawRotatedPic(
                                        scrPlace,
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        voiceMaterial);
                                }
                                if (firingMaterial)
                                {
                                    fadedColor[3] = firingFade;
                                    CG_DrawRotatedPic(
                                        scrPlace,
                                        xy[0],
                                        xy[1],
                                        w,
                                        h,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        yawTo,
                                        fadedColor,
                                        firingMaterial);
                                }
                            }
                        }
                    }
                    ++num;
                    ++actor;
                }
            }
        }
    }
}

void __cdecl CG_CompassDrawEnemies(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color)
{
    bool v5; // [esp+28h] [ebp-84h]
    bool v6; // [esp+2Ch] [ebp-80h]
    bool withinRadarPingTime; // [esp+32h] [ebp-7Ah]
    float fadeTimeAmount; // [esp+34h] [ebp-78h]
    float radarTimeAmount; // [esp+38h] [ebp-74h]
    float yawVector[2]; // [esp+3Ch] [ebp-70h] BYREF
    bool clipped; // [esp+47h] [ebp-65h]
    clientInfo_t *localClientInfo; // [esp+48h] [ebp-64h]
    float xy[2]; // [esp+4Ch] [ebp-60h] BYREF
    int team; // [esp+54h] [ebp-58h]
    const ScreenPlacement *scrPlace; // [esp+58h] [ebp-54h]
    cg_s *cgameGlob; // [esp+5Ch] [ebp-50h]
    float compassFadeOutAlpha; // [esp+60h] [ebp-4Ch]
    float centerY; // [esp+64h] [ebp-48h]
    int clientIndex; // [esp+68h] [ebp-44h]
    CompassActor *actor; // [esp+6Ch] [ebp-40h]
    int num; // [esp+70h] [ebp-3Ch]
    rectDef_s scaledRect; // [esp+74h] [ebp-38h] BYREF
    float centerX; // [esp+8Ch] [ebp-20h]
    float h; // [esp+90h] [ebp-1Ch] BYREF
    float fadedColor[4]; // [esp+94h] [ebp-18h] BYREF
    bool alwaysShowEnemies; // [esp+A7h] [ebp-5h]
    float w; // [esp+A8h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->nextSnap);
    bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
    localClientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
    CG_CompassUpYawVector(cgameGlob, yawVector);
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (compassFadeOutAlpha != 0.0)
    {
        bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
        if (cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid)
        {
            bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
            team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
            if (team != 3)
            {
                CG_CompassCalcDimensions(
                    compassType,
                    cgameGlob,
                    parentRect,
                    rect,
                    &scaledRect.x,
                    &scaledRect.y,
                    &scaledRect.w,
                    &scaledRect.h);
                centerX = scaledRect.w * 0.5 + scaledRect.x;
                centerY = scaledRect.h * 0.5 + scaledRect.y;
                fadedColor[0] = *color;
                fadedColor[1] = color[1];
                fadedColor[2] = color[2];
                fadedColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                clientIndex = localClientNum;
                alwaysShowEnemies = Dvar_GetBool("g_compassShowEnemies");
                actor = s_compassActors[clientIndex];
                num = 0;
                while (num < 64)
                {
                    if (actor->enemy)
                    {
                        if (!alwaysShowEnemies)
                            goto LABEL_54;
                        if (actor->lastUpdate > cgameGlob->time)
                            actor->lastUpdate = 0;
                        if (actor->lastUpdate >= cgameGlob->time - 1500)
                        {
                        LABEL_54:
                            v6 = (double)actor->beginFadeTime + compassSoundPingFadeTime->current.value * 1000.0 > (double)cgameGlob->time
                                && actor->beginFadeTime;
                            if (alwaysShowEnemies)
                            {
                                withinRadarPingTime = 1;
                            }
                            else
                            {
                                v5 = (double)actor->beginRadarFadeTime + compassRadarPingFadeTime->current.value * 1000.0 > (double)cgameGlob->time
                                    && actor->beginRadarFadeTime;
                                withinRadarPingTime = v5;
                            }
                            if (v6 || withinRadarPingTime)
                            {
                                clipped = alwaysShowEnemies
                                    ? CG_WorldPosToCompass(
                                        compassType,
                                        cgameGlob,
                                        &scaledRect,
                                        yawVector,
                                        cgameGlob->refdef.vieworg,
                                        actor->lastPos,
                                        0,
                                        xy)
                                    : CG_WorldPosToCompass(
                                        compassType,
                                        cgameGlob,
                                        &scaledRect,
                                        yawVector,
                                        cgameGlob->refdef.vieworg,
                                        actor->lastEnemyPos,
                                        0,
                                        xy);
                                if (!clipped || compassClampIcons->current.enabled)
                                {
                                    CalcCompassFriendlySize(compassType, &w, &h);
                                    xy[0] = centerX - w * 0.5 + xy[0];
                                    xy[1] = centerY - h * 0.5 + xy[1];
                                    if (actor->beginRadarFadeTime >= cgameGlob->time || alwaysShowEnemies)
                                        radarTimeAmount = 1.0;
                                    else
                                        radarTimeAmount = 1.0
                                        - (double)(cgameGlob->time - actor->beginRadarFadeTime)
                                        / (compassRadarPingFadeTime->current.value
                                            * 1000.0);
                                    if (actor->beginFadeTime < cgameGlob->time)
                                        fadeTimeAmount = 1.0
                                        - (double)(cgameGlob->time - actor->beginFadeTime)
                                        / (compassSoundPingFadeTime->current.value
                                            * 1000.0);
                                    else
                                        fadeTimeAmount = 1.0;
                                    if (radarTimeAmount >= 0.0)
                                    {
                                        fadedColor[3] = radarTimeAmount * compassFadeOutAlpha;
                                        UI_DrawHandlePic(
                                            scrPlace,
                                            xy[0],
                                            xy[1],
                                            w,
                                            h,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            fadedColor,
                                            cgMedia.compassping_enemy);
                                    }
                                    if (fadeTimeAmount >= 0.0)
                                    {
                                        fadedColor[3] = fadeTimeAmount * compassFadeOutAlpha;
                                        UI_DrawHandlePic(
                                            scrPlace,
                                            xy[0],
                                            xy[1],
                                            w,
                                            h,
                                            rect->horzAlign,
                                            rect->vertAlign,
                                            fadedColor,
                                            cgMedia.compassping_enemyfiring);
                                    }
                                }
                            }
                        }
                    }
                    ++num;
                    ++actor;
                }
            }
        }
    }
}

void __cdecl CG_CompassDrawRadarEffects(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *color)
{
    float radiusST; // [esp+38h] [ebp-64h]
    float v6; // [esp+3Ch] [ebp-60h]
    float texTop; // [esp+58h] [ebp-44h]
    float texRight; // [esp+5Ch] [ebp-40h]
    float radarXAmount; // [esp+60h] [ebp-3Ch]
    float texLeft; // [esp+64h] [ebp-38h]
    float xy[2]; // [esp+68h] [ebp-34h] BYREF
    float radarLine[3]; // [esp+70h] [ebp-2Ch] BYREF
    float rotation; // [esp+7Ch] [ebp-20h]
    float radarTextureCenterX; // [esp+80h] [ebp-1Ch]
    float playerPosRelativeToRadarLine; // [esp+84h] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+88h] [ebp-14h]
    cg_s *cgameGlob; // [esp+8Ch] [ebp-10h]
    float radarLineThickness; // [esp+90h] [ebp-Ch]
    float h; // [esp+94h] [ebp-8h] BYREF
    float w; // [esp+98h] [ebp-4h] BYREF

    iassert(rect);
    iassert(parentRect);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob);

    if (cgameGlob->predictedPlayerState.radarEnabled)
    {
        scrPlace = &scrPlaceView[localClientNum];
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, xy, &xy[1], &w, &h);
        iassert(w > 0.0f);
        iassert(h > 0.0f);
        if (compassType)
        {
            iassert(compassType == COMPASS_TYPE_FULL);
            radarXAmount = GetRadarLineEastWestPercentage(cgameGlob, cgameGlob->radarProgress);
            radarLineThickness = cg_hudMapRadarLineThickness->current.value;
            if (radarLineThickness > 0.0)
            {
                texLeft = -radarXAmount / radarLineThickness + 0.5;
                texRight = (1.0 - radarXAmount) / radarLineThickness + 0.5;
                iassert(h);
                texTop = 1.0 / (radarLineThickness * w / h);
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    xy[0],
                    xy[1],
                    w,
                    h,
                    rect->horzAlign,
                    rect->vertAlign,
                    texLeft,
                    0.0,
                    texRight,
                    texTop,
                    color,
                    cgMedia.compass_radarline);
            }
        }
        else
        {
            GetRadarLine(cgameGlob, cgameGlob->radarProgress, radarLine);
            radarLineThickness = compassRadarLineThickness->current.value;
            v6 = cgameGlob->refdef.vieworg[1] * radarLine[1] + cgameGlob->refdef.vieworg[0] * radarLine[0];
            playerPosRelativeToRadarLine = v6 - radarLine[2];
            playerPosRelativeToRadarLine = playerPosRelativeToRadarLine / compassMaxRange->current.value;
            radarTextureCenterX = playerPosRelativeToRadarLine / radarLineThickness + 0.5;
            if (compassRotation->current.enabled)
                rotation = cgameGlob->compassNorthYaw - cgameGlob->refdefViewAngles[1];
            else
                rotation = 0.0;
            radiusST = 0.5 / radarLineThickness;
            CL_DrawStretchPicRotatedST(
                scrPlace,
                xy[0],
                xy[1],
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                radarTextureCenterX,
                0.0,
                radiusST,
                1.0,
                1.0,
                rotation,
                color,
                cgMedia.compass_radarline);
        }
    }
}

double __cdecl GetRadarLineEastWestPercentage(cg_s *cgameGlob, float radarProgress)
{
    float margin; // [esp+10h] [ebp-4h]

    margin = GetRadarLineMargin(cgameGlob) / cgameGlob->compassMapWorldSize[0];
    return (float)((margin * 2.0 + 1.0) * radarProgress - margin);
}

void __cdecl CG_CompassDrawVehicles(
    int localClientNum,
    CompassType compassType,
    int eType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *enemyMaterial,
    Material *friendlyMaterial,
    const float *color)
{
    double v8; // st7
    float v9; // [esp+24h] [ebp-74h]
    float angle; // [esp+28h] [ebp-70h]
    float yawVector[2]; // [esp+2Ch] [ebp-6Ch] BYREF
    bool clipped; // [esp+37h] [ebp-61h]
    float xy[2]; // [esp+38h] [ebp-60h] BYREF
    CompassVehicle *veh; // [esp+40h] [ebp-58h]
    cg_s *cgameGlob; // [esp+44h] [ebp-54h]
    team_t myTeam; // [esp+48h] [ebp-50h]
    centity_s *cent; // [esp+4Ch] [ebp-4Ch]
    float compassFadeOutAlpha; // [esp+50h] [ebp-48h]
    bool enemy; // [esp+57h] [ebp-41h]
    float yawTo; // [esp+58h] [ebp-40h]
    float centerY; // [esp+5Ch] [ebp-3Ch]
    int num; // [esp+60h] [ebp-38h]
    rectDef_s scaledRect; // [esp+64h] [ebp-34h] BYREF
    float centerX; // [esp+7Ch] [ebp-1Ch]
    float h; // [esp+80h] [ebp-18h]
    float fadedColor[4]; // [esp+84h] [ebp-14h] BYREF
    float w; // [esp+94h] [ebp-4h]

    iassert(compassType == COMPASS_TYPE_PARTIAL);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->nextSnap);
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if (compassFadeOutAlpha != 0.0)
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = scaledRect.w * 0.5 + scaledRect.x;
        centerY = scaledRect.h * 0.5 + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        CG_CompassUpYawVector(cgameGlob, yawVector);
        veh = s_compassVehicles[localClientNum];
        num = 0;
        while (num < 8)
        {
            cent = CG_GetEntity(localClientNum, veh->entityNum);
            if (cent->nextState.eType == eType)
            {
                if (veh->lastUpdate > cgameGlob->time)
                    veh->lastUpdate = 0;
                if (veh->lastUpdate >= cgameGlob->time - 1500)
                {
                    clipped = CG_WorldPosToCompass(
                        compassType,
                        cgameGlob,
                        &scaledRect,
                        yawVector,
                        cgameGlob->refdef.vieworg,
                        veh->lastPos,
                        0,
                        xy);
                    if (!clipped || compassClampIcons->current.enabled)
                    {
                        if (eType == 12)
                        {
                            w = compassSize->current.value * 40.0;
                            v8 = compassSize->current.value * 40.0;
                        }
                        else
                        {
                            w = compassSize->current.value * 20.0;
                            v8 = compassSize->current.value * 20.0;
                        }
                        h = v8;
                        xy[0] = centerX - w * 0.5 + xy[0];
                        xy[1] = centerY - h * 0.5 + xy[1];
                        if (compassType || !compassRotation->current.enabled)
                        {
                            v9 = cgameGlob->compassNorthYaw - veh->lastYaw;
                            yawTo = AngleNormalize360(v9);
                        }
                        else
                        {
                            angle = cgameGlob->refdefViewAngles[1] - veh->lastYaw;
                            yawTo = AngleNormalize360(angle);
                        }
                        if (fadedColor[3] > (double)compassFadeOutAlpha)
                            fadedColor[3] = compassFadeOutAlpha;
                        myTeam = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
                        if (myTeam)
                            enemy = myTeam != veh->team;
                        else
                            enemy = cgameGlob->clientNum != veh->ownerIndex;
                        if (enemy)
                            CG_DrawRotatedPic(
                                &scrPlaceView[localClientNum],
                                xy[0],
                                xy[1],
                                w,
                                h,
                                rect->horzAlign,
                                rect->vertAlign,
                                yawTo,
                                fadedColor,
                                enemyMaterial);
                        else
                            CG_DrawRotatedPic(
                                &scrPlaceView[localClientNum],
                                xy[0],
                                xy[1],
                                w,
                                h,
                                rect->horzAlign,
                                rect->vertAlign,
                                yawTo,
                                fadedColor,
                                friendlyMaterial);
                    }
                }
            }
            ++num;
            ++veh;
        }
    }
}