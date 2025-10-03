#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "server_mp.h"
#include <game_mp/g_public_mp.h>
#include <server/sv_game.h>
#include <win32/win_local.h>
#include <universal/com_files.h>
#include <qcommon/files.h>
#include <universal/profile.h>

serverStaticHeader_t svsHeader;
//struct dvar_s const *const sv_clientArchive 84ff2600     sv_snapshot_mp.obj
//int svsHeaderValid       84ff262c     sv_snapshot_mp.obj
int svsHeaderValid;
//struct serverStaticHeader_t svsHeader 85012680     sv_snapshot_mp.obj

msg_t g_archiveMsg;
unsigned __int8 tempServerMsgBuf[131072];

void __cdecl SV_WriteSnapshotToClient(client_t *client, msg_t *msg)
{
    int v2; // edx
    clientSnapshot_t *frame; // [esp+0h] [ebp-54h]
    int bitsUsed; // [esp+4h] [ebp-50h]
    int lastframe; // [esp+8h] [ebp-4Ch]
    int from_num_entities; // [esp+Ch] [ebp-48h]
    int from_first_entity; // [esp+14h] [ebp-40h]
    SnapshotInfo_s snapInfo; // [esp+18h] [ebp-3Ch] BYREF
    int sendAsActive; // [esp+30h] [ebp-24h]
    int from_num_clients; // [esp+34h] [ebp-20h]
    clientSnapshot_t *remoteFrame; // [esp+38h] [ebp-1Ch]
    int lastServerTime; // [esp+3Ch] [ebp-18h]
    int i; // [esp+40h] [ebp-14h]
    const clientSnapshot_t *oldframe; // [esp+44h] [ebp-10h]
    int snapFlags; // [esp+48h] [ebp-Ch]
    int clientNum; // [esp+4Ch] [ebp-8h]
    int from_first_client; // [esp+50h] [ebp-4h]

    if (!svsHeaderValid)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 499, 0, "%s", "svsHeaderValid");
    memset(
        (unsigned __int8 *)currentSnapshotNetworkEntityFieldsChanged,
        0,
        sizeof(currentSnapshotNetworkEntityFieldsChanged));
    if (!client)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 506, 0, "%s", "localClient");
    clientNum = client - svsHeader.clients;
    if ((unsigned int)clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            510,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    memset(&snapInfo.snapshotDeltaTime, 0, 16);
    snapInfo.clientNum = clientNum;
    snapInfo.client = &client->header;
    remoteFrame = &client->frames[client->header.netchan.outgoingSequence & 0x1F];
    remoteFrame->serverTime = svsHeader.time;
    frame = remoteFrame;
    if (!remoteFrame)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 523, 0, "%s", "frame");
    if (client->header.deltaMessage > 0 && client->header.state == 4)
    {
        if (client->header.netchan.outgoingSequence - client->header.deltaMessage < 29)
        {
            v2 = client->header.deltaMessage & 0x1F;
            oldframe = &client->frames[v2];
            if ((client_t *)((char *)client + v2 * 12164) == (client_t *)-136296)
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 551, 0, "%s", "oldframe");
            lastframe = client->header.netchan.outgoingSequence - client->header.deltaMessage;
            lastServerTime = oldframe->serverTime;
            if (oldframe->first_entity < svsHeader.nextSnapshotEntities - svsHeader.numSnapshotEntities)
            {
                Com_PrintWarning(
                    15,
                    "%s: Delta request from out of date entities - delta against entity %i, oldest is %i, current is %i.  Their old"
                    " snapshot had %i entities in it\n",
                    client->name,
                    oldframe->first_entity,
                    svs.nextSnapshotEntities - svs.numSnapshotEntities,
                    svs.nextSnapshotEntities,
                    oldframe->num_entities);
                oldframe = 0;
                lastframe = 0;
                lastServerTime = 0;
            }
            if (oldframe && oldframe->first_client < svsHeader.nextSnapshotClients - svsHeader.numSnapshotClients)
            {
                Com_PrintWarning(
                    15,
                    "%s: Delta request from out of date clients - delta against client %i, oldest is %i, current is %i.  Their old "
                    "snapshot had %i clients in it\n",
                    client->name,
                    oldframe->first_client,
                    svs.nextSnapshotClients - svs.numSnapshotClients,
                    svs.nextSnapshotClients,
                    oldframe->num_clients);
                oldframe = 0;
                lastframe = 0;
                lastServerTime = 0;
            }
        }
        else
        {
            Com_DPrintf(15, "%s: Delta request from out of date packet.\n", client->name);
            oldframe = 0;
            lastframe = 0;
            lastServerTime = 0;
        }
    }
    else
    {
        oldframe = 0;
        lastframe = 0;
        lastServerTime = 0;
    }
    SV_PacketDataIsHeader(clientNum, msg);
    MSG_WriteByte(msg, 6u);
    MSG_WriteLong(msg, svsHeader.time);
    MSG_WriteByte(msg, lastframe);
    snapInfo.snapshotDeltaTime = lastServerTime;
    snapFlags = svsHeader.snapFlagServerBit;
    if (client->header.rateDelayed)
        snapFlags |= 1u;
    sendAsActive = client->header.sendAsActive;
    if (client->header.state == 4)
    {
        sendAsActive = 1;
    }
    else if (client->header.state != 1)
    {
        sendAsActive = 0;
    }
    if (client->header.sendAsActive != sendAsActive)
        client->header.sendAsActive = sendAsActive;
    if (!sendAsActive)
        snapFlags |= 2u;
    MSG_WriteByte(msg, snapFlags);
    bitsUsed = MSG_GetUsedBitCount(msg);
    SV_PacketDataIsUnknown(clientNum, msg);
    if (oldframe)
    {
        MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, &oldframe->ps, &frame->ps);
        SV_TrackPacketData(clientNum, ANALYZE_SNAPSHOT_DELTAPLAYERSTATE, 0, 0, bitsUsed, msg);
        from_num_entities = oldframe->num_entities;
        from_first_entity = oldframe->first_entity;
        from_num_clients = oldframe->num_clients;
        from_first_client = oldframe->first_client;
    }
    else
    {
        MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &frame->ps);
        SV_TrackPacketData(clientNum, ANALYZE_SNAPSHOT_NODELTAPLAYERSTATE, 0, 0, bitsUsed, msg);
        from_num_entities = 0;
        from_first_entity = 0;
        from_num_clients = 0;
        from_first_client = 0;
    }
    SV_PacketDataIsUnknown(clientNum, msg);
    SV_EmitPacketEntities(&snapInfo, from_num_entities, from_first_entity, frame->num_entities, frame->first_entity, msg);
    SV_PacketDataIsUnknown(clientNum, msg);
    SV_EmitPacketClients(&snapInfo, from_num_clients, from_first_client, frame->num_clients, frame->first_client, msg);
    SV_PacketDataIsUnknown(clientNum, msg);
    for (i = 0; i < sv_padPackets->current.integer; ++i)
        MSG_WriteByte(msg, 0);
}

void __cdecl SV_EmitPacketEntities(
    SnapshotInfo_s *snapInfo,
    int from_num_entities,
    int from_first_entity,
    int to_num_entities,
    int to_first_entity,
    msg_t *msg)
{
    const char *v6; // eax
    int number; // [esp+30h] [ebp-42Ch]
    int bitsUsed; // [esp+34h] [ebp-428h]
    int newnum; // [esp+38h] [ebp-424h]
    entityState_s *oldent; // [esp+3Ch] [ebp-420h]
    int oldindex; // [esp+40h] [ebp-41Ch]
    int oldnum; // [esp+44h] [ebp-418h]
    int newindex; // [esp+48h] [ebp-414h]
    bool entityFound[1024]; // [esp+4Ch] [ebp-410h] BYREF
    const entityState_s *baseline; // [esp+450h] [ebp-Ch]
    const entityState_s *newent; // [esp+454h] [ebp-8h]
    int bitsStart; // [esp+458h] [ebp-4h]

    iassert(svsHeaderValid);

    bitsStart = MSG_GetUsedBitCount(msg);
    bitsUsed = bitsStart;

    PROF_SCOPED("SV_EmitPacketEntities");

    MSG_ClearLastReferencedEntity(msg);
    memset((unsigned __int8 *)entityFound, 0, sizeof(entityFound));
    newent = 0;
    newindex = 0;
    oldindex = 0;
    while (newindex < to_num_entities || oldindex < from_num_entities)
    {
        if (newindex < to_num_entities)
        {
            newent = &svsHeader.snapshotEntities[(newindex + to_first_entity) % svsHeader.numSnapshotEntities];
            newnum = newent->number;
        }
        else
        {
            newent = 0;
            newnum = 9999;
        }
        if (oldindex < from_num_entities)
        {
            oldent = &svsHeader.snapshotEntities[(oldindex + from_first_entity) % svsHeader.numSnapshotEntities];
            oldnum = oldent->number;
        }
        else
        {
            oldent = 0;
            oldnum = 9999;
        }
        if (newnum <= oldnum)
        {
            number = newent->number;
            if (newent->number < 0 || number >= 1024)
                MyAssertHandler(
                    ".\\server_mp\\sv_snapshot_mp.cpp",
                    188,
                    0,
                    "%s\n\t(number) = %i",
                    "(( number >= 0 && number < (1<<10) ))",
                    number);
            if (entityFound[number])
            {
                v6 = va(
                    "number is %i, oldnum is %i, newnum is %i, to_first is %i, from_first is %i, newindex is %i, oldindex is %i",
                    number,
                    oldnum,
                    newnum,
                    to_first_entity,
                    from_first_entity,
                    newindex,
                    oldindex);
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 189, 0, "%s\n\t%s", "!entityFound[ number ]", v6);
            }
            entityFound[number] = 1;
        }
        if (newent)
            SV_SetNextEntityStart(newent->eType, newent->number);
        if (newnum == oldnum)
        {
            MSG_WriteEntity(snapInfo, msg, svsHeader.time, oldent, newent, 0);
            if (newent->eType < ET_EVENTS)
                bitsUsed = SV_TrackPacketData(
                    snapInfo->clientNum,
                    ANALYZE_SNAPSHOT_DELTAENTITY,
                    newent->eType,
                    newent->number,
                    bitsUsed,
                    msg);
            else
                bitsUsed = SV_TrackPacketData(
                    snapInfo->clientNum,
                    ANALYZE_SNAPSHOT_TEMPENTITY,
                    newent->eType,
                    newent->number,
                    bitsUsed,
                    msg);
            ++oldindex;
            ++newindex;
        }
        else if (newnum >= oldnum)
        {
            if (newnum > oldnum)
            {
                MSG_WriteEntity(snapInfo, msg, svsHeader.time, oldent, 0, 1);
                if (oldent->eType < ET_EVENTS)
                    bitsUsed = SV_TrackPacketData(
                        snapInfo->clientNum,
                        ANALYZE_SNAPSHOT_REMOVEDENTITY,
                        oldent->eType,
                        oldent->number,
                        bitsUsed,
                        msg);
                else
                    bitsUsed = SV_TrackPacketData(
                        snapInfo->clientNum,
                        ANALYZE_SNAPSHOT_TEMPENTITY,
                        oldent->eType,
                        oldent->number,
                        bitsUsed,
                        msg);
                ++oldindex;
            }
        }
        else
        {
            baseline = &svsHeader.svEntities[newnum].baseline.s;
            if (&svsHeader.svEntities[newnum] == (svEntity_s *)-4)
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 229, 0, "%s", "baseline");
            snapInfo->fromBaseline = 1;
            MSG_WriteEntity(snapInfo, msg, svsHeader.time, (entityState_s *)baseline, newent, 1);
            snapInfo->fromBaseline = 0;
            if (newent->eType < ET_EVENTS)
                bitsUsed = SV_TrackPacketData(
                    snapInfo->clientNum,
                    ANALYZE_SNAPSHOT_NEWENTITY,
                    newent->eType,
                    newent->number,
                    bitsUsed,
                    msg);
            else
                bitsUsed = SV_TrackPacketData(
                    snapInfo->clientNum,
                    ANALYZE_SNAPSHOT_TEMPENTITY,
                    newent->eType,
                    newent->number,
                    bitsUsed,
                    msg);
            ++newindex;
        }
    }
    SV_PacketDataIsHeader(snapInfo->clientNum, msg);
    MSG_WriteEntityIndex(snapInfo, msg, ENTITYNUM_NONE, 10);
    SV_TrackPacketData(snapInfo->clientNum, ANALYZE_SNAPSHOT_ALLENTITIES, 0, 0, bitsStart, msg);
}

void __cdecl SV_EmitPacketClients(
    SnapshotInfo_s *snapInfo,
    int from_num_clients,
    int from_first_client,
    int to_num_clients,
    int to_first_client,
    msg_t *msg)
{
    int bitsUsed; // [esp+30h] [ebp-20h]
    int newnum; // [esp+34h] [ebp-1Ch]
    int newnuma; // [esp+34h] [ebp-1Ch]
    int newnumb; // [esp+34h] [ebp-1Ch]
    clientState_s *oldclient; // [esp+38h] [ebp-18h]
    int oldindex; // [esp+3Ch] [ebp-14h]
    int oldnum; // [esp+40h] [ebp-10h]
    int oldnuma; // [esp+40h] [ebp-10h]
    int oldnumb; // [esp+40h] [ebp-10h]
    int newindex; // [esp+44h] [ebp-Ch]
    const clientState_s *newclient; // [esp+48h] [ebp-8h]
    const clientState_s *newclienta; // [esp+48h] [ebp-8h]
    clientState_s *newclientb; // [esp+48h] [ebp-8h]
    int bitsStart; // [esp+4Ch] [ebp-4h]

    bitsStart = MSG_GetUsedBitCount(msg);
    bitsUsed = bitsStart;
    if (!svsHeaderValid)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 305, 0, "%s", "svsHeaderValid");
    oldnum = -1;
    for (newnum = 0; newnum < from_num_clients; ++newnum)
    {
        newclient = &svsHeader.snapshotClients[(newnum + from_first_client) % svsHeader.numSnapshotClients];
        if (newclient->clientIndex <= oldnum)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 312, 0, "%s", "newclient->clientIndex > oldnum");
        oldnum = newclient->clientIndex;
    }
    oldnuma = -1;
    for (newnuma = 0; newnuma < to_num_clients; ++newnuma)
    {
        newclienta = &svsHeader.snapshotClients[(newnuma + to_first_client) % svsHeader.numSnapshotClients];
        if (newclienta->clientIndex <= oldnuma)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 319, 0, "%s", "newclient->clientIndex > oldnum");
        oldnuma = newclienta->clientIndex;
    }

    PROF_SCOPED("SV_EmitPacketClients");

    MSG_ClearLastReferencedEntity(msg);
    newindex = 0;
    oldindex = 0;
    while (newindex < to_num_clients || oldindex < from_num_clients)
    {
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        if (newindex < to_num_clients)
        {
            newclientb = &svsHeader.snapshotClients[(newindex + to_first_client) % svsHeader.numSnapshotClients];
            newnumb = newclientb->clientIndex;
        }
        else
        {
            newclientb = 0;
            newnumb = 9999;
        }
        if (oldindex < from_num_clients)
        {
            oldclient = &svsHeader.snapshotClients[(oldindex + from_first_client) % svsHeader.numSnapshotClients];
            oldnumb = oldclient->clientIndex;
        }
        else
        {
            oldclient = 0;
            oldnumb = 9999;
        }
        if (newnumb == oldnumb)
        {
            MSG_WriteDeltaClient(snapInfo, msg, svsHeader.time, oldclient, newclientb, 0);
            bitsUsed = SV_TrackPacketData(
                snapInfo->clientNum,
                ANALYZE_SNAPSHOT_DELTACLIENT,
                0,
                newclientb->clientIndex,
                bitsUsed,
                msg);
            ++oldindex;
            ++newindex;
        }
        else if (newnumb >= oldnumb)
        {
            if (newnumb > oldnumb)
            {
                if (!oldclient)
                    MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 435, 0, "%s", "oldclient");
                if (oldclient->clientIndex <= msg->lastEntityRef)
                {
                    Com_Printf(15, "** Client index LE msg->lastEntityRef:\n");
                    Com_Printf(15, "**   lastEntityRef is %d  clientIndex is %d\n", msg->lastEntityRef, oldclient->clientIndex);
                    Com_Printf(
                        15,
                        "**   newnum %d  oldnum %d  from_num_clients %d  to_num_clients %d\n",
                        newnumb,
                        oldnumb,
                        from_num_clients,
                        to_num_clients);
                    Com_Printf(
                        15,
                        "**   from_first_client %d  to_first_client %d  numSnapshotClients %d\n",
                        from_first_client,
                        to_first_client,
                        svsHeader.numSnapshotClients);
                    if (!alwaysfails)
                        MyAssertHandler(
                            ".\\server_mp\\sv_snapshot_mp.cpp",
                            442,
                            0,
                            "oldclient->clientIndex <= msg->lastEntityRef (please attach console log to bug)");
                }
                MSG_WriteDeltaClient(snapInfo, msg, svsHeader.time, oldclient, 0, 1);
                bitsUsed = SV_TrackPacketData(
                    snapInfo->clientNum,
                    ANALYZE_SNAPSHOT_REMOVEDCLIENT,
                    0,
                    oldclient->clientIndex,
                    bitsUsed,
                    msg);
                ++oldindex;
            }
        }
        else
        {
            MSG_WriteDeltaClient(snapInfo, msg, svsHeader.time, 0, newclientb, 1);
            bitsUsed = SV_TrackPacketData(
                snapInfo->clientNum,
                ANALYZE_SNAPSHOT_NEWCLIENT,
                0,
                newclientb->clientIndex,
                bitsUsed,
                msg);
            ++newindex;
        }
    }
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    MSG_WriteBit0(msg);
    SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
    SV_TrackPacketData(snapInfo->clientNum, ANALYZE_SNAPSHOT_ALLCLIENTS, 0, 0, bitsStart, msg);
}

void __cdecl SV_UpdateServerCommandsToClient(client_t *client, msg_t *msg)
{
    int bitsUsed; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (client->reliableAcknowledge + 1 < client->reliableSequence && sv_debugReliableCmds->current.enabled)
        Com_Printf(15, "Client %s has the following un-ack'd reliable commands:\n", client->name);
    bitsUsed = MSG_GetUsedBitCount(msg);
    for (i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i)
    {
        SV_PacketDataIsHeader(client - svs.clients, msg);
        MSG_WriteByte(msg, 4u);
        MSG_WriteLong(msg, i);
        SV_PacketDataIsReliableData(client - svs.clients, msg);
        MSG_WriteString(msg, client->reliableCommandInfo[i & 0x7F].cmd);
        if (sv_debugReliableCmds->current.enabled)
            Com_Printf(15, "%i: %s\n", i - (client->reliableAcknowledge + 1), client->reliableCommandInfo[i & 0x7F].cmd);
    }
    SV_TrackPacketData(client - svs.clients, ANALYZE_SNAPSHOT_SERVERCMDS, 0, 0, bitsUsed, msg);
    client->reliableSent = client->reliableSequence;
}

void __cdecl SV_UpdateServerCommandsToClient_PreventOverflow(client_t *client, msg_t *msg, int iMsgSize)
{
    svscmd_info_t *v3; // [esp+Ch] [ebp-Ch]
    int i; // [esp+14h] [ebp-4h]
    int ia; // [esp+14h] [ebp-4h]

    for (i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i)
    {
        v3 = &client->reliableCommandInfo[i & 0x7F];
        if (&v3->cmd[strlen(v3->cmd) + 1] - &client->reliableCommandInfo[i & 0x7F].cmd[1] + 6 + msg->cursize >= iMsgSize)
            break;
        MSG_WriteByte(msg, 4u);
        MSG_WriteLong(msg, i);
        MSG_WriteString(msg, client->reliableCommandInfo[i & 0x7F].cmd);
    }
    ia = i - 1;
    if (ia > client->reliableSent)
        client->reliableSent = ia;
}

char __cdecl SV_GetClientPositionAtTime(int client, int gametime, float *pos)
{
    float end[3]; // [esp+10h] [ebp-301Ch] BYREF
    int v5; // [esp+1Ch] [ebp-3010h]
    int v6; // [esp+20h] [ebp-300Ch]
    char v7; // [esp+27h] [ebp-3005h]
    int i; // [esp+28h] [ebp-3004h]
    float progress; // [esp+2Ch] [ebp-3000h]
    float start[3]; // [esp+30h] [ebp-2FFCh] BYREF
    int pArchiveTime[2]; // [esp+3Ch] [ebp-2FF0h] BYREF
    clientState_s cs; // [esp+44h] [ebp-2FE8h] BYREF
    bool foundEnd; // [esp+B3h] [ebp-2F79h]
    playerState_s ps; // [esp+B4h] [ebp-2F78h] BYREF
    int v15; // [esp+3020h] [ebp-Ch]
    int steps; // [esp+3024h] [ebp-8h]
    int v17; // [esp+3028h] [ebp-4h]

    v6 = 1000 / sv_fps->current.integer;
    pArchiveTime[1] = v6 * (svs.time / v6);
    v5 = svs.time - gametime;
    steps = 400 / v6 + 1;                         // LWSS: this is 500 on xbox...
    v7 = 0;
    foundEnd = 0;
    pArchiveTime[0] = 0;
    v15 = 0;
    v17 = 0;
    for (i = 0; i < steps; ++i)
    {
        if (SV_GetArchivedClientInfo(client, pArchiveTime, &ps, &cs))
        {
            if (ps.stats[0] <= 0 || (ps.otherFlags & 4) == 0 || (ps.otherFlags & 2) != 0)
                return 0;
            if (foundEnd)
            {
                start[0] = ps.origin[0];
                start[1] = ps.origin[1];
                start[2] = ps.origin[2];
                v15 = pArchiveTime[0];
                v7 = 1;
                break;
            }
            if (pArchiveTime[0] == v5)
            {
                end[0] = ps.origin[0];
                end[1] = ps.origin[1];
                end[2] = ps.origin[2];
                start[0] = ps.origin[0];
                start[1] = ps.origin[1];
                start[2] = ps.origin[2];
                foundEnd = 1;
                v7 = 1;
                v15 = pArchiveTime[0];
                v17 = pArchiveTime[0];
                break;
            }
            if (pArchiveTime[0] >= v5)
            {
                foundEnd = 1;
                start[0] = ps.origin[0];
                start[1] = ps.origin[1];
                start[2] = ps.origin[2];
                v15 = pArchiveTime[0];
                v7 = 1;
                break;
            }
            end[0] = ps.origin[0];
            end[1] = ps.origin[1];
            end[2] = ps.origin[2];
            v17 = pArchiveTime[0];
        }
        pArchiveTime[0] += v6;
    }
    if (v7)
    {
        iassert(foundEnd);
        if (v17 == v15)
            progress = 1.0;
        else
            progress = (v15 - v5) / (v15 - v17);
        iassert(progress >= 0);
        iassert(progress <= 1);
    }
    else
    {
        if (!foundEnd)
            return 0;
        progress = 0.0;
        start[0] = end[0];
        start[1] = end[1];
        start[2] = end[2];
    }
    Vec3Lerp(start, end, progress, pos);
    return 1;
}

int __cdecl SV_GetArchivedClientInfo(int clientNum, int *pArchiveTime, playerState_s *ps, clientState_s *cs)
{
    cachedSnapshot_t *cachedFrame; // [esp+74h] [ebp-10h]
    unsigned int i; // [esp+78h] [ebp-Ch]
    unsigned int ia; // [esp+78h] [ebp-Ch]
    int deltaTime; // [esp+7Ch] [ebp-8h]
    cachedClient_s *cachedClient; // [esp+80h] [ebp-4h]

    PROF_SCOPED("SV_GetArchivedClientInfo"); // AKA "SV_GetArchivedPlayerState"
    cachedFrame = SV_GetCachedSnapshot(pArchiveTime);
    if (cachedFrame)
    {
        if (*pArchiveTime <= 0)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1597, 0, "%s", "*pArchiveTime > 0");
        deltaTime = svs.time - cachedFrame->time;
        for (i = 0; ; ++i)
        {
            if (i >= cachedFrame->num_clients)
                goto LABEL_14;
            cachedClient = &svs.cachedSnapshotClients[(i + cachedFrame->first_client) % 0x1000];
            if (cachedClient->cs.clientIndex == clientNum)
                break;
        }
        if (!cachedClient->playerStateExists)
        {
        LABEL_14:
            return 0;
        }
        if (!cachedClient)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1618, 0, "%s", "cachedClient");
        memcpy((unsigned __int8 *)ps, (unsigned __int8 *)&cachedClient->ps, sizeof(playerState_s));
        memcpy(cs, &cachedClient->cs, sizeof(clientState_s));
        if (ps->commandTime)
            ps->commandTime += deltaTime;
        if (ps->pm_time)
            ps->pm_time += deltaTime;
        if (ps->foliageSoundTime)
            ps->foliageSoundTime += deltaTime;
        if (ps->jumpTime)
            ps->jumpTime += deltaTime;
        if (ps->viewHeightLerpTime)
            ps->viewHeightLerpTime += deltaTime;
        if (ps->shellshockTime)
            ps->shellshockTime += deltaTime;
        for (ia = 0; ia < 0x1F && ps->hud.archival[ia].type; ++ia)
        {
            if (ps->hud.archival[ia].time)
                ps->hud.archival[ia].time += deltaTime;
            if (ps->hud.archival[ia].fadeStartTime)
            {
                ps->hud.archival[ia].fadeStartTime += deltaTime;
                if (ps->hud.archival[ia].fadeStartTime > svs.time)
                    ps->hud.archival[ia].fadeStartTime = svs.time;
            }
            if (ps->hud.archival[ia].scaleStartTime)
                ps->hud.archival[ia].scaleStartTime += deltaTime;
            if (ps->hud.archival[ia].moveStartTime)
                ps->hud.archival[ia].moveStartTime += deltaTime;
        }
        ps->deltaTime += deltaTime;
        return 1;
    }
    else
    {
        if (*pArchiveTime > 0)
            return 0;
        else
            return SV_GetCurrentClientInfo(clientNum, ps, cs);
    }
}

cachedSnapshot_t *__cdecl SV_GetCachedSnapshot(int *pArchiveTime)
{
    int archivedFrame; // [esp+4h] [ebp-8h]
    cachedSnapshot_t *cachedFrame; // [esp+8h] [ebp-4h]

    if (!SV_Loaded())
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1398, 0, "%s", "SV_Loaded()");
    if (!sv_fps->current.integer)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1399, 0, "%s", "sv_fps->current.integer");
    if (*pArchiveTime <= 0)
        return 0;
    archivedFrame = svs.nextArchivedSnapshotFrames - sv_fps->current.integer * *pArchiveTime / 1000;
    if (archivedFrame < svs.nextArchivedSnapshotFrames - 1200)
    {
        archivedFrame = svs.nextArchivedSnapshotFrames - 1200;
        *pArchiveTime = 1000
            * (svs.nextArchivedSnapshotFrames - (svs.nextArchivedSnapshotFrames - 1200))
            / sv_fps->current.integer;
    }
    if (archivedFrame < 0)
    {
        archivedFrame = 0;
        *pArchiveTime = 1000 * svs.nextArchivedSnapshotFrames / sv_fps->current.integer;
    }
    while (archivedFrame < svs.nextArchivedSnapshotFrames)
    {
        cachedFrame = SV_GetCachedSnapshotInternal(archivedFrame);
        if (cachedFrame)
            return cachedFrame;
        ++archivedFrame;
    }
    *pArchiveTime = 0;
    return 0;
}

cachedSnapshot_t *__cdecl SV_GetCachedSnapshotInternal(int archivedFrame)
{
    int v2; // edx
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int oldArchivedFrame; // [esp+4h] [ebp-5Ch]
    signed int newnum; // [esp+8h] [ebp-58h]
    unsigned int newnuma; // [esp+8h] [ebp-58h]
    unsigned int newnumb; // [esp+8h] [ebp-58h]
    unsigned int newnumc; // [esp+8h] [ebp-58h]
    int oldindex; // [esp+14h] [ebp-4Ch]
    int oldnum; // [esp+18h] [ebp-48h]
    msg_t msg; // [esp+1Ch] [ebp-44h] BYREF
    cachedSnapshot_t *cachedFrame; // [esp+44h] [ebp-1Ch]
    cachedClient_s *oldCachedClient; // [esp+48h] [ebp-18h]
    int partSize; // [esp+4Ch] [ebp-14h]
    int firstCachedSnapshotFrame; // [esp+50h] [ebp-10h]
    int i; // [esp+54h] [ebp-Ch]
    cachedSnapshot_t *oldCachedFrame; // [esp+58h] [ebp-8h]
    cachedClient_s *cachedClient; // [esp+5Ch] [ebp-4h]

    if (svs.archivedSnapshotFrames[archivedFrame % 1200].start < svs.nextArchivedSnapshotBuffer - 0x2000000)
        return 0;
    firstCachedSnapshotFrame = svs.nextCachedSnapshotFrames - 512;
    if (svs.nextCachedSnapshotFrames - 512 < 0)
        firstCachedSnapshotFrame = 0;
    for (i = svs.nextCachedSnapshotFrames - 1; i >= firstCachedSnapshotFrame; --i)
    {
        cachedFrame = &svs.cachedSnapshotFrames[i % 512];
        if (cachedFrame->archivedFrame == archivedFrame)
        {
            if (cachedFrame->first_entity >= svs.nextCachedSnapshotEntities - 0x4000
                && cachedFrame->first_client >= svs.nextCachedSnapshotClients - 4096)
            {
                return cachedFrame;
            }
            break;
        }
    }
    v2 = svs.archivedSnapshotFrames[archivedFrame % 1200].start % 0x2000000;
    partSize = 0x2000000 - v2;
    if (*(_DWORD *)&svs.archivedSnapshotBuffer[8 * (archivedFrame % 1200) - 9596] > 0x2000000 - v2)
        MSG_InitReadOnlySplit(
            &msg,
            &svs.archivedSnapshotBuffer[v2],
            partSize,
            svs.archivedSnapshotBuffer,
            *(_DWORD *)&svs.archivedSnapshotBuffer[8 * (archivedFrame % 1200) - 9596] - partSize);
    else
        MSG_InitReadOnly(
            &msg,
            &svs.archivedSnapshotBuffer[v2],
            *(_DWORD *)&svs.archivedSnapshotBuffer[8 * (archivedFrame % 1200) - 9596]);
    MSG_BeginReading(&msg);
    if (MSG_ReadBit(&msg))
    {
        cachedFrame = &svs.cachedSnapshotFrames[svs.nextCachedSnapshotFrames % 512];
        cachedFrame->archivedFrame = archivedFrame;
        cachedFrame->num_entities = 0;
        cachedFrame->first_entity = svs.nextCachedSnapshotEntities;
        cachedFrame->num_clients = 0;
        cachedFrame->first_client = svs.nextCachedSnapshotClients;
        cachedFrame->usesDelta = 0;
        v6 = MSG_ReadLong(&msg);
        cachedFrame->time = v6;
        MSG_ClearLastReferencedEntity(&msg);
        while (MSG_ReadBit(&msg))
        {
            newnumb = MSG_ReadEntityIndex(&msg, 6u);
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            cachedClient = &svs.cachedSnapshotClients[svs.nextCachedSnapshotClients % 4096];
            MSG_ReadDeltaClient(&msg, cachedFrame->time, 0, &cachedClient->cs, newnumb);
            v7 = MSG_ReadBit(&msg);
            cachedClient->playerStateExists = v7;
            if (cachedClient->playerStateExists)
                MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, 0, &cachedClient->ps, 0);
            if (svsHeaderValid)
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1340, 0, "%s", "!svsHeaderValid");
            if (++svs.nextCachedSnapshotClients >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextCachedSnapshotClients wrapped");
            ++cachedFrame->num_clients;
        }
        MSG_ClearLastReferencedEntity(&msg);
        while (1)
        {
            newnumc = MSG_ReadEntityIndex(&msg, 0xAu);
            if (newnumc == ENTITYNUM_NONE)
                break;
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            MSG_ReadDeltaArchivedEntity(
                &msg,
                cachedFrame->time,
                &sv.svEntities[newnumc].baseline,
                &svs.cachedSnapshotEntities[svs.nextCachedSnapshotEntities % 0x4000],
                newnumc);
            if (++svs.nextCachedSnapshotEntities >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextCachedSnapshotEntities wrapped");
            ++cachedFrame->num_entities;
        }
        if (++svs.nextCachedSnapshotFrames >= 2147483646)
            Com_Error(ERR_FATAL, "svs.nextCachedSnapshotFrames wrapped");
    }
    else
    {
        oldArchivedFrame = MSG_ReadLong(&msg);
        if (oldArchivedFrame < svs.nextArchivedSnapshotFrames - 1200)
            return 0;
        if (svs.archivedSnapshotFrames[oldArchivedFrame % 1200].start < svs.nextArchivedSnapshotBuffer - 0x2000000)
            return 0;
        oldCachedFrame = SV_GetCachedSnapshotInternal(oldArchivedFrame);
        if (!oldCachedFrame)
            return 0;
        if (oldCachedFrame->usesDelta)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1174, 0, "%s", "!oldCachedFrame->usesDelta");
        cachedFrame = &svs.cachedSnapshotFrames[svs.nextCachedSnapshotFrames % 512];
        cachedFrame->archivedFrame = archivedFrame;
        cachedFrame->num_entities = 0;
        cachedFrame->first_entity = svs.nextCachedSnapshotEntities;
        cachedFrame->num_clients = 0;
        cachedFrame->first_client = svs.nextCachedSnapshotClients;
        cachedFrame->usesDelta = 1;
        v3 = MSG_ReadLong(&msg);
        cachedFrame->time = v3;
        oldindex = 0;
        oldCachedClient = 0;
        if (oldCachedFrame->num_clients > 0)
        {
            oldCachedClient = &svs.cachedSnapshotClients[oldCachedFrame->first_client % 4096];
            oldnum = oldCachedClient->cs.clientIndex;
        }
        else
        {
            oldnum = 99999;
        }
        MSG_ClearLastReferencedEntity(&msg);
        while (MSG_ReadBit(&msg))
        {
            newnum = MSG_ReadEntityIndex(&msg, 6u);
            if (newnum < 0)
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1209, 0, "%s\n\t(newnum) = %i", "(newnum >= 0)", newnum);
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            while (oldnum < newnum)
            {
                if (++oldindex < oldCachedFrame->num_clients)
                {
                    oldCachedClient = &svs.cachedSnapshotClients[(oldindex + oldCachedFrame->first_client) % 4096];
                    oldnum = oldCachedClient->cs.clientIndex;
                }
                else
                {
                    oldnum = 99999;
                }
            }
            if (oldnum == newnum)
            {
                cachedClient = &svs.cachedSnapshotClients[svs.nextCachedSnapshotClients % 4096];
                MSG_ReadDeltaClient(&msg, cachedFrame->time, &oldCachedClient->cs, &cachedClient->cs, newnum);
                v4 = MSG_ReadBit(&msg);
                cachedClient->playerStateExists = v4;
                if (cachedClient->playerStateExists)
                {
                    if (oldCachedClient->playerStateExists)
                        MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, &oldCachedClient->ps, &cachedClient->ps, 0);
                    else
                        MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, 0, &cachedClient->ps, 0);
                }
                if (svsHeaderValid)
                    MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1243, 0, "%s", "!svsHeaderValid");
                if (++svs.nextCachedSnapshotClients >= 2147483646)
                    Com_Error(ERR_FATAL, "svs.nextCachedSnapshotClients wrapped");
                ++cachedFrame->num_clients;
                if (++oldindex < oldCachedFrame->num_clients)
                {
                    oldCachedClient = &svs.cachedSnapshotClients[(oldindex + oldCachedFrame->first_client) % 4096];
                    oldnum = oldCachedClient->cs.clientIndex;
                }
                else
                {
                    oldnum = 99999;
                }
            }
            else
            {
                if (oldnum <= newnum)
                    MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1264, 0, "%s", "oldnum > newnum");
                cachedClient = &svs.cachedSnapshotClients[svs.nextCachedSnapshotClients % 4096];
                MSG_ReadDeltaClient(&msg, cachedFrame->time, 0, &cachedClient->cs, newnum);
                v5 = MSG_ReadBit(&msg);
                cachedClient->playerStateExists = v5;
                if (cachedClient->playerStateExists)
                    MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, 0, &cachedClient->ps, 0);
                if (svsHeaderValid)
                    MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1273, 0, "%s", "!svsHeaderValid");
                if (++svs.nextCachedSnapshotClients >= 2147483646)
                    Com_Error(ERR_FATAL, "svs.nextCachedSnapshotClients wrapped");
                ++cachedFrame->num_clients;
            }
        }
        MSG_ClearLastReferencedEntity(&msg);
        while (1)
        {
            newnuma = MSG_ReadEntityIndex(&msg, 0xAu);
            if (newnuma == ENTITYNUM_NONE)
                break;
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            MSG_ReadDeltaArchivedEntity(
                &msg,
                cachedFrame->time,
                &sv.svEntities[newnuma].baseline,
                &svs.cachedSnapshotEntities[svs.nextCachedSnapshotEntities % 0x4000],
                newnuma);
            if (++svs.nextCachedSnapshotEntities >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextCachedSnapshotEntities wrapped");
            ++cachedFrame->num_entities;
        }
        if (++svs.nextCachedSnapshotFrames >= 2147483646)
            Com_Error(ERR_FATAL, "svs.nextCachedSnapshotFrames wrapped");
    }
    if (cachedFrame->num_entities >= 0x4000)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            1377,
            1,
            "%s\n\t(cachedFrame->num_entities) = %i",
            "(cachedFrame->num_entities < (64 * 64 * 4))",
            cachedFrame->num_entities);
    if (cachedFrame->num_clients >= 4096)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            1378,
            1,
            "%s\n\t(cachedFrame->num_clients) = %i",
            "(cachedFrame->num_clients < (64 * 64))",
            cachedFrame->num_clients);
    if (cachedFrame->first_entity < svs.nextCachedSnapshotEntities - 0x4000)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            1379,
            1,
            "%s",
            "cachedFrame->first_entity >= svs.nextCachedSnapshotEntities - NUM_CACHED_SNAPSHOT_ENTITIES");
    if (cachedFrame->first_client < svs.nextCachedSnapshotClients - 4096)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            1380,
            1,
            "%s",
            "cachedFrame->first_client >= svs.nextCachedSnapshotClients - NUM_CACHED_SNAPSHOT_CLIENTS");
    return cachedFrame;
}

int __cdecl SV_GetCurrentClientInfo(int clientNum, playerState_s *ps, clientState_s *cs)
{
    if (svs.clients[clientNum].header.state != 4)
        return 0;
    if (!GetFollowPlayerState(clientNum, ps))
        return 0;
    memcpy(cs, G_GetClientState(clientNum), sizeof(clientState_s));
    return 1;
}

void __cdecl SV_BuildClientSnapshot(client_t *client)
{
    unsigned __int8 *v1; // eax
    clientState_s *ClientState; // eax
    int v3; // [esp+18h] [ebp-1154h]
    entityState_s *v4; // [esp+20h] [ebp-114Ch]
    clientSnapshot_t *v5; // [esp+24h] [ebp-1148h]
    int pArchiveTime; // [esp+28h] [ebp-1144h] BYREF
    snapshotEntityNumbers_t eNums; // [esp+2Ch] [ebp-1140h] BYREF
    archivedEntity_s *v8; // [esp+1034h] [ebp-138h]
    clientState_s *v9; // [esp+1038h] [ebp-134h]
    cachedSnapshot_t *CachedSnapshot; // [esp+1040h] [ebp-12Ch]
    unsigned int v12[65]; // [esp+1044h] [ebp-128h] BYREF
    float position[3]; // [esp+1148h] [ebp-24h] BYREF
    //float v14; // [esp+1150h] [ebp-1Ch]
    gentity_s *v15; // [esp+1154h] [ebp-18h]
    int i; // [esp+1158h] [ebp-14h]
    int clientNum; // [esp+115Ch] [ebp-10h]
    unsigned __int8 *dst; // [esp+1160h] [ebp-Ch]
    int v19; // [esp+1164h] [ebp-8h]
    cachedClient_s *v20; // [esp+1168h] [ebp-4h]
    client_t *clients; // [esp+1174h] [ebp+8h]

    v5 = &client->frames[client->header.netchan.outgoingSequence & 0x1F];
    v5->num_entities = 0;
    v5->num_clients = 0;
    if (client->gentity)
    {
        if (client->header.state != 1)
        {
            v5->first_entity = svs.nextSnapshotEntities;
            v5->first_client = svs.nextSnapshotClients;
            if (sv.state == SS_GAME)
            {
                eNums.numSnapshotEntities = 0;
                clientNum = client - svs.clients;
                pArchiveTime = G_GetClientArchiveTime(clientNum);
                CachedSnapshot = SV_GetCachedSnapshot(&pArchiveTime);
                G_SetClientArchiveTime(clientNum, pArchiveTime);
                if (CachedSnapshot)
                    v3 = svs.time - CachedSnapshot->time;
                else
                    v3 = 0;
                v19 = v3;
                dst = (unsigned __int8 *)v5;
                v1 = (unsigned __int8 *)SV_GameClientNum(clientNum);
                memcpy(dst, v1, 0x2F64u);
                clientNum = *((unsigned int *)dst + 55);
                if ((unsigned int)clientNum >= 0x400)
                    Com_Error(ERR_DROP, "SV_BuildClientSnapshot: bad gEnt");
                position[0] = *((float *)dst + 7);
                position[1] = *((float *)dst + 8);
                position[2] = *((float *)dst + 9);
                position[2] = position[2] + *((float *)dst + 70);
                AddLeanToPosition(position, *((float *)dst + 67), *((float *)dst + 23), 16.0, 20.0);
                memset((unsigned __int8 *)v12, 0, 0x100u);
                if (CachedSnapshot)
                {
                    SV_AddCachedEntitiesVisibleFromPoint(
                        CachedSnapshot->num_entities,
                        CachedSnapshot->first_entity,
                        position,
                        clientNum,
                        &eNums);
                    for (i = 0; i < eNums.numSnapshotEntities; ++i)
                    {
                        v8 = &svs.cachedSnapshotEntities[(eNums.snapshotEntities[i] + CachedSnapshot->first_entity) % 0x4000];
                        v4 = &svs.snapshotEntities[svs.nextSnapshotEntities % svs.numSnapshotEntities];
                        memcpy(v4, v8, sizeof(entityState_s));
                        if (v4->lerp.pos.trTime)
                            v4->lerp.pos.trTime += v19;
                        if (v4->lerp.apos.trTime)
                            v4->lerp.apos.trTime += v19;
                        if (v4->time2)
                            v4->time2 += v19;
                        switch (v4->eType)
                        {
                        case ET_GENERAL:
                            v4->lerp.u.anonymous.data[0] += v19;
                            break;
                        case ET_MISSILE:
                            v4->lerp.u.missile.launchTime += v19;
                            break;
                        case ET_EVENTS + EV_CUSTOM_EXPLODE:
                            v4->lerp.u.customExplode.startTime += v19;
                            break;
                        }
                        if (++svs.nextSnapshotEntities >= 2147483646)
                            Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
                        ++v5->num_entities;
                    }
                    for (i = 0; i < CachedSnapshot->num_clients; ++i)
                    {
                        v20 = &svs.cachedSnapshotClients[(i + CachedSnapshot->first_client) % 4096];
                        v9 = &svs.snapshotClients[svs.nextSnapshotClients % svs.numSnapshotClients];
                        memcpy(v9, &v20->cs, sizeof(clientState_s));
                        if (v12[v9->clientIndex])
                            MyAssertHandler(
                                ".\\server_mp\\sv_snapshot_mp.cpp",
                                1837,
                                0,
                                "%s",
                                "!clientIndex[clientState->clientIndex]");
                        v12[v9->clientIndex] = 1;
                        if (++svs.nextSnapshotClients >= 2147483646)
                            Com_Error(ERR_FATAL, "svs.nextSnapshotClients wrapped");
                        if (++v5->num_clients + v5->first_client != svs.nextSnapshotClients)
                            MyAssertHandler(
                                ".\\server_mp\\sv_snapshot_mp.cpp",
                                1847,
                                0,
                                "%s",
                                "frame->first_client + frame->num_clients == svs.nextSnapshotClients");
                    }
                }
                else
                {
                    SV_AddEntitiesVisibleFromPoint(position, clientNum, &eNums);
                    for (i = 0; i < eNums.numSnapshotEntities; ++i)
                    {
                        v15 = SV_GentityNum(eNums.snapshotEntities[i]);
                        memcpy(
                            &svs.snapshotEntities[svs.nextSnapshotEntities++ % svs.numSnapshotEntities],
                            v15,
                            sizeof(svs.snapshotEntities[svs.nextSnapshotEntities++ % svs.numSnapshotEntities]));
                        if (svs.nextSnapshotEntities >= 2147483646)
                            Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
                        ++v5->num_entities;
                    }
                    i = 0;
                    clients = svs.clients;
                    while (i < sv_maxclients->current.integer)
                    {
                        if (clients->header.state >= 2)
                        {
                            v9 = &svs.snapshotClients[svs.nextSnapshotClients % svs.numSnapshotClients];
                            ClientState = G_GetClientState(i);
                            memcpy(v9, ClientState, sizeof(clientState_s));
                            if (v9->clientIndex == i)
                            {
                                if (v12[v9->clientIndex])
                                    MyAssertHandler(
                                        ".\\server_mp\\sv_snapshot_mp.cpp",
                                        1784,
                                        0,
                                        "%s",
                                        "!clientIndex[clientState->clientIndex]");
                                v12[v9->clientIndex] = 1;
                                if (++svs.nextSnapshotClients >= 2147483646)
                                    Com_Error(ERR_FATAL, "svs.nextSnapshotClients wrapped");
                                if (++v5->num_clients + v5->first_client != svs.nextSnapshotClients)
                                    MyAssertHandler(
                                        ".\\server_mp\\sv_snapshot_mp.cpp",
                                        1794,
                                        0,
                                        "%s",
                                        "frame->first_client + frame->num_clients == svs.nextSnapshotClients");
                            }
                        }
                        ++i;
                        ++clients;
                    }
                }
            }
        }
    }
}

void __cdecl SV_AddEntitiesVisibleFromPoint(float *org, int clientNum, snapshotEntityNumbers_t *eNums)
{
    const char *v3; // eax
    int e; // [esp+18h] [ebp-28h]
    int clientcluster; // [esp+1Ch] [ebp-24h]
    float fogOpaqueDistSqrd; // [esp+20h] [ebp-20h]
    svEntity_s *svEnt; // [esp+24h] [ebp-1Ch]
    int l; // [esp+28h] [ebp-18h]
    unsigned int leafnum; // [esp+2Ch] [ebp-14h]
    gentity_s *ent; // [esp+30h] [ebp-10h]
    int i; // [esp+34h] [ebp-Ch]
    unsigned __int8 *clientpvs; // [esp+3Ch] [ebp-4h]

    if (!SV_Loaded())
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 907, 0, "%s", "SV_Loaded()");
    leafnum = CM_PointLeafnum(org);
    clientcluster = CM_LeafCluster(leafnum);
    if (clientcluster >= 0)
    {
        clientpvs = CM_ClusterPVS(clientcluster);
        fogOpaqueDistSqrd = G_GetFogOpaqueDistSqrd();
        if (fogOpaqueDistSqrd == FLT_MAX)
            fogOpaqueDistSqrd = 0.0;
        for (e = 0; ; ++e)
        {
            if (e >= sv.num_entities)
                return;
            ent = SV_GentityNum(e);
            if (ent->r.linked)
            {
                if (ent->s.number != e)
                {
                    v3 = va(
                        "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                        ent->s.number,
                        e,
                        ent->s.eType,
                        ent->r.currentOrigin[0],
                        ent->r.currentOrigin[1],
                        ent->r.currentOrigin[2]);
                    MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 929, 0, "%s\n\t%s", "ent->s.number == e", v3);
                }
                if (e != clientNum)
                {
                    if (ent->r.broadcastTime)
                    {
                        if (ent->r.broadcastTime < 0 || ent->r.broadcastTime - svs.time >= 0)
                            goto LABEL_36;
                        ent->r.broadcastTime = 0;
                    }
                    else if ((ent->r.svFlags & 1) != 0 || (ent->r.clientMask[clientNum >> 5] & (1 << (clientNum & 0x1F))) != 0)
                    {
                        continue;
                    }
                    if ((ent->r.svFlags & 0x18) != 0)
                        goto LABEL_36;
                    svEnt = SV_SvEntityForGentity(ent);
                    if (!svEnt->numClusters)
                        goto LABEL_36;
                    l = 0;
                    for (i = 0; i < svEnt->numClusters; ++i)
                    {
                        l = svEnt->clusternums[i];
                        if (((1 << (l & 7)) & clientpvs[l >> 3]) != 0)
                            break;
                    }
                    if (i != svEnt->numClusters)
                        goto LABEL_39;
                    if (svEnt->lastCluster)
                    {
                        while (l <= svEnt->lastCluster && ((1 << (l & 7)) & clientpvs[l >> 3]) == 0)
                            ++l;
                        if (l != svEnt->lastCluster)
                        {
                        LABEL_39:
                            if (fogOpaqueDistSqrd == 0.0 || !BoxDistSqrdExceeds(ent->r.absmin, ent->r.absmax, org, fogOpaqueDistSqrd))
                            {
                            LABEL_36:
                                SV_AddArchivedEntToSnapshot(e, eNums);
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl SV_AddCachedEntitiesVisibleFromPoint(
    int from_num_entities,
    int from_first_entity,
    float *org,
    int clientNum,
    snapshotEntityNumbers_t *eNums)
{
    int e; // [esp+4h] [ebp-1130h]
    int cluster; // [esp+8h] [ebp-112Ch]
    int lastLeaf; // [esp+Ch] [ebp-1128h] BYREF
    float fogOpaqueDistSqrd; // [esp+10h] [ebp-1124h]
    unsigned int dst[1025]; // [esp+14h] [ebp-1120h] BYREF
    int v10; // [esp+1018h] [ebp-11Ch]
    unsigned __int16 list[128]; // [esp+101Ch] [ebp-118h] BYREF
    int leafnum; // [esp+1120h] [ebp-14h]
    archivedEntity_s *v13; // [esp+1124h] [ebp-10h]
    int v14; // [esp+1128h] [ebp-Ch]
    int i; // [esp+112Ch] [ebp-8h]
    unsigned __int8 *v16; // [esp+1130h] [ebp-4h]

    if (!SV_Loaded())
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1027, 0, "%s", "SV_Loaded()");
    leafnum = CM_PointLeafnum(org);
    cluster = CM_LeafCluster(leafnum);
    if (cluster >= 0)
    {
        v16 = CM_ClusterPVS(cluster);
        fogOpaqueDistSqrd = G_GetFogOpaqueDistSqrd();
        if (fogOpaqueDistSqrd == FLT_MAX)
            fogOpaqueDistSqrd = 0.0;
        memset((unsigned __int8 *)dst, 0, 0x1000u);
        for (e = 0; e < from_num_entities; ++e)
        {
            v13 = &svs.cachedSnapshotEntities[(e + from_first_entity) % 0x4000];
            if (dst[v13->s.number])
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1051, 0, "%s", "!entityIndex[ent->s.number]");
            dst[v13->s.number] = 1;
            if ((v13->r.clientMask[clientNum >> 5] & (1 << (clientNum & 0x1F))) == 0 && v13->s.number != clientNum)
            {
                if ((v13->r.svFlags & 0x18) != 0)
                    goto LABEL_14;
                v14 = CM_BoxLeafnums(v13->r.absmin, v13->r.absmax, list, 128, &lastLeaf);
                if (v14)
                {
                    for (i = 0; i < v14; ++i)
                    {
                        v10 = CM_LeafCluster(list[i]);
                        if (v10 != -1 && ((1 << (v10 & 7)) & v16[v10 >> 3]) != 0)
                            break;
                    }
                    if (i != v14
                        && (fogOpaqueDistSqrd == 0.0 || !BoxDistSqrdExceeds(v13->r.absmin, v13->r.absmax, org, fogOpaqueDistSqrd)))
                    {
                    LABEL_14:
                        SV_AddArchivedEntToSnapshot(e, eNums);
                    }
                }
            }
        }
    }
}

void __cdecl SV_AddArchivedEntToSnapshot(int e, snapshotEntityNumbers_t *eNums)
{
    if (eNums->numSnapshotEntities != 1024)
        eNums->snapshotEntities[eNums->numSnapshotEntities++] = e;
}

unsigned __int8 svCompressedBuf[131072];
void __cdecl SV_SendMessageToClient(msg_t *msg, client_t *client)
{
    int v2; // [esp+Ch] [ebp-30h]
    double v3; // [esp+14h] [ebp-28h]
    int compressedSize; // [esp+2Ch] [ebp-10h]
    int lastFrame; // [esp+34h] [ebp-8h]
    int rateMsec; // [esp+38h] [ebp-4h]

    if (msg->cursize < 4)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1916, 0, "%s", "msg->cursize >= SV_ENCODE_START");
    *(unsigned int *)svCompressedBuf = *(unsigned int *)msg->data;
    compressedSize = MSG_WriteBitsCompress(
        client->header.state == 4,
        (const unsigned __int8 *)msg->data + 4,
        svCompressedBuf + 4,
        msg->cursize - 4)
        + 4;
    if (client->header.netchan.remoteAddress.type != NA_LOOPBACK)
        SV_TrackPacketCompression(client - svs.clients, msg->cursize, compressedSize);
    if (client->dropReason)
    {
        SV_DropClient(client, client->dropReason, 1);
        if (client->dropReason)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1926, 0, "%s", "!client->dropReason");
        if (client->header.state != 1)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1927, 0, "%s", "client->header.state == CS_ZOMBIE");
    }
    client->frames[client->header.netchan.outgoingSequence & 0x1F].messageSize = compressedSize;
    client->frames[client->header.netchan.outgoingSequence & 0x1F].messageSent = Sys_Milliseconds();
    client->frames[client->header.netchan.outgoingSequence & 0x1F].messageAcked = -1;
    lastFrame = client->header.netchan.outgoingSequence - client->header.deltaMessage;
    SV_Netchan_Transmit(client, svCompressedBuf, compressedSize);
    if (client->header.state == 4 && client->header.deltaMessage >= 0 && lastFrame >= 29)
    {
        if (client->snapshotBackoffCount < 0)
            MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 1951, 0, "%s", "client->snapshotBackoffCount >= 0");
        //v3 = _Pow_int<double>(2.0, client->snapshotBackoffCount);
        v3 = pow(2.0, (double)client->snapshotBackoffCount);
        client->nextSnapshotTime = svs.time + irand(0, (int)v3) * client->snapshotMsec;
        if (client->snapshotBackoffCount + 1 < 8)
            v2 = client->snapshotBackoffCount + 1;
        else
            v2 = 8;
        client->snapshotBackoffCount = v2;
    }
    else
    {
        client->snapshotBackoffCount = 0;
        if (client->header.netchan.remoteAddress.type == NA_LOOPBACK
            || Sys_IsLANAddress(client->header.netchan.remoteAddress))
        {
            client->nextSnapshotTime = svs.time - 1;
        }
        else
        {
            rateMsec = SV_RateMsec(client, compressedSize);
            if (rateMsec >= client->snapshotMsec)
            {
                client->header.rateDelayed = 1;
            }
            else
            {
                rateMsec = client->snapshotMsec;
                client->header.rateDelayed = 0;
            }
            client->nextSnapshotTime = rateMsec + svs.time;
            if (client->header.state != 4 && !client->downloadName[0] && client->nextSnapshotTime < svs.time + 1000)
                client->nextSnapshotTime = svs.time + 1000;
            sv.bpsTotalBytes += compressedSize;
        }
    }
}

int __cdecl SV_RateMsec(client_t *client, int messageSize)
{
    int rate; // [esp+0h] [ebp-8h]

    if (messageSize > 1500)
        messageSize = 1500;
    rate = client->rate;
    if (sv_maxRate->current.integer)
    {
        if (sv_maxRate->current.integer < 1000)
            Dvar_SetInt((dvar_s *)sv_maxRate, 1000);
        if (sv_maxRate->current.integer < rate)
            rate = sv_maxRate->current.integer;
    }
    if (sv_debugRate->current.enabled)
        Com_Printf(
            15,
            "It would take %ims to send %i bytes to client %s (rate %i)\n",
            1000 * (messageSize + 48) / rate,
            messageSize,
            client->name,
            client->rate);
    return 1000 * (messageSize + 48) / rate;
}

unsigned __int8 tempSnapshotMsgBuf[131072];
void __cdecl SV_BeginClientSnapshot(client_t *client, msg_t *msg)
{
    unsigned int clientNum; // [esp+0h] [ebp-4h]

    client->tempPacketDebugging = 0;
    if (sv_debugPacketContentsForClientThisFrame->current.enabled)
    {
        client->tempPacketDebugging = 1;
        Dvar_SetBool((dvar_s *)sv_debugPacketContentsForClientThisFrame, 0);
        Dvar_SetBool((dvar_s *)sv_debugPacketContents, 1);
    }
    if (sv_debugPacketContents->current.enabled)
        Com_Printf(15, "Starting snapshot for %s\n", client->name);
    clientNum = client - svs.clients;
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2017,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    MSG_Init(msg, tempSnapshotMsgBuf, 0x20000);
    MSG_ClearLastReferencedEntity(msg);
    SV_ResetPacketData(clientNum, msg);
    SV_PacketDataIsHeader(clientNum, msg);
    MSG_WriteLong(msg, client->lastClientCommand);
    if (client->header.state == 4 || client->header.state == 1)
    {
        SV_PacketDataIsReliableData(clientNum, msg);
        SV_UpdateServerCommandsToClient(client, msg);
        SV_PacketDataIsUnknown(clientNum, msg);
    }
}

void __cdecl SV_Download_Clear(client_t *cl)
{
    if (!cl)
        MyAssertHandler(".\\server_mp\\sv_client_mp.cpp", 1711, 0, "%s", "cl");
    cl->downloading = 0;
    cl->download = 0;
    cl->downloadName[0] = 0;
}

void __cdecl SV_WriteDownloadErrorMessage(client_t *cl, msg_t *msg, const char *errorMessage)
{
    MSG_WriteByte(msg, 5u);
    MSG_WriteShort(msg, 0);
    MSG_WriteLong(msg, -1);
    MSG_WriteString(msg, errorMessage);
    SV_Download_Clear(cl);
}

int __cdecl SV_WWWRedirectClient(client_t *cl, msg_t *msg)
{
    char *v2; // eax
    int handle; // [esp+0h] [ebp-Ch] BYREF
    int download_flag; // [esp+4h] [ebp-8h]
    int downloadSize; // [esp+8h] [ebp-4h]

    download_flag = 0;
    downloadSize = FS_SV_FOpenFileRead(cl->downloadName, &handle);
    if (downloadSize)
    {
        FS_FCloseFile(handle);
        v2 = va("%s/%s", sv_wwwBaseURL->current.string, cl->downloadName);
        I_strncpyz(cl->downloadURL, v2, 256);
        Com_Printf(15, "Redirecting client '%s' to %s\n", cl->name, cl->downloadURL);
        cl->downloadingWWW = 1;
        MSG_WriteByte(msg, 5u);
        MSG_WriteLong(msg, -1);
        MSG_WriteString(msg, cl->downloadURL);
        MSG_WriteLong(msg, downloadSize);
        if (sv_wwwDlDisconnected->current.enabled)
            download_flag |= 1u;
        MSG_WriteLong(msg, download_flag);
        SV_Download_Clear(cl);
        return 1;
    }
    else
    {
        Com_Printf(15, "ERROR: Client '%s': couldn't extract file size for %s\n", cl->downloadName, handle);
        return 0;
    }
}

void __cdecl SV_WriteDownloadToClient(client_t *cl, msg_t *msg)
{
    int v2; // edx
    int rate; // [esp+0h] [ebp-414h]
    char errorMessage[1024]; // [esp+4h] [ebp-410h] BYREF
    int blockspersnap; // [esp+40Ch] [ebp-8h]
    int curindex; // [esp+410h] [ebp-4h]

    if (cl->downloadName[0] && !cl->clientDownloadingWWW)
    {
        if (cl->download)
        {
        LABEL_20:
            while (cl->downloadCurrentBlock - cl->downloadClientBlock < 8 && cl->downloadSize != cl->downloadCount)
            {
                curindex = cl->downloadCurrentBlock % 8;
                if (!cl->downloadBlocks[curindex])
                    cl->downloadBlocks[curindex] = (unsigned char*)Z_Malloc(2048, "SV_WriteDownloadToClient", 9);
                cl->downloadBlockSize[curindex] = FS_Read(cl->downloadBlocks[curindex], 0x800u, cl->download);
                if (cl->downloadBlockSize[curindex] < 0)
                {
                    cl->downloadCount = cl->downloadSize;
                    break;
                }
                cl->downloadCount += cl->downloadBlockSize[curindex];
                ++cl->downloadCurrentBlock;
            }
            if (cl->downloadCount == cl->downloadSize
                && !cl->downloadEOF
                && cl->downloadCurrentBlock - cl->downloadClientBlock < 8)
            {
                cl->downloadBlockSize[cl->downloadCurrentBlock++ % 8] = 0;
                cl->downloadEOF = 1;
            }
            rate = cl->rate;
            if (sv_maxRate->current.integer)
            {
                if (sv_maxRate->current.integer < 1000)
                    Dvar_SetInt(sv_maxRate, 1000);
                if (sv_maxRate->current.integer < rate)
                    rate = sv_maxRate->current.integer;
            }
            if (rate)
                blockspersnap = (cl->snapshotMsec * rate / 1000 + 2048) / 2048;
            else
                blockspersnap = 1;
            if (blockspersnap < 0)
                blockspersnap = 1;
            while (1)
            {
                v2 = blockspersnap--;
                if (!v2 || cl->downloadClientBlock == cl->downloadCurrentBlock)
                    break;
                if (cl->downloadXmitBlock == cl->downloadCurrentBlock)
                {
                    if (svs.time - cl->downloadSendTime <= 1000)
                        return;
                    cl->downloadXmitBlock = cl->downloadClientBlock;
                }
                curindex = cl->downloadXmitBlock % 8;
                MSG_WriteByte(msg, 5u);
                MSG_WriteLong(msg, cl->downloadXmitBlock);
                if (!cl->downloadXmitBlock)
                    MSG_WriteLong(msg, cl->downloadSize);
                MSG_WriteShort(msg, cl->downloadBlockSize[curindex]);
                if (cl->downloadBlockSize[curindex])
                    MSG_WriteData(msg, cl->downloadBlocks[curindex], cl->downloadBlockSize[curindex]);
                Com_DPrintf(15, "clientDownload: %d : writing block %d\n", cl - svs.clients, cl->downloadXmitBlock);
                ++cl->downloadXmitBlock;
                cl->downloadSendTime = svs.time;
            }
        }
        else
        {
            if (sv_allowDownload->current.enabled)
            {
                if (FS_iwIwd(cl->downloadName, (char*)"main"))
                {
                    Com_Printf(
                        15,
                        "clientDownload: %d : \"%s\" cannot download IW iwd files\n",
                        cl - svs.clients,
                        cl->downloadName);
                    Com_sprintf(errorMessage, 0x400u, "EXE_CANTAUTODLGAMEPAK %s", cl->downloadName);
                    SV_WriteDownloadErrorMessage(cl, msg, errorMessage);
                    return;
                }
                Com_Printf(15, "clientDownload: %d : beginning \"%s\"\n", cl - svs.clients, cl->downloadName);
                cl->downloadSize = FS_SV_FOpenFileRead(cl->downloadName, &cl->download);
                if (cl->downloadSize <= 0)
                {
                    Com_Printf(15, "clientDownload: %d : \"%s\" file not found on server\n", cl - svs.clients, cl->downloadName);
                    Com_sprintf(errorMessage, 0x400u, "EXE_AUTODL_FILENOTONSERVER %s", cl->downloadName);
                    SV_WriteDownloadErrorMessage(cl, msg, errorMessage);
                    return;
                }
                if (sv_wwwDownload->current.enabled && cl->wwwOk)
                {
                    if (cl->wwwFallback)
                    {
                        cl->wwwFallback = 0;
                        return;
                    }
                    if (SV_WWWRedirectClient(cl, msg))
                        return;
                }
                cl->downloadingWWW = 0;
                cl->downloadXmitBlock = 0;
                cl->downloadClientBlock = 0;
                cl->downloadCurrentBlock = 0;
                cl->downloadCount = 0;
                cl->downloadEOF = 0;
                goto LABEL_20;
            }
            Com_Printf(15, "clientDownload: %d : \"%s\" download disabled", cl - svs.clients, cl->downloadName);
            if (sv_pure->current.enabled)
                Com_sprintf(errorMessage, 0x400u, "EXE_AUTODL_SERVERDISABLED_PURE %s", cl->downloadName);
            else
                Com_sprintf(errorMessage, 0x400u, "EXE_AUTODL_SERVERDISABLED %s", cl->downloadName);
            SV_WriteDownloadErrorMessage(cl, msg, errorMessage);
        }
    }
}

void __cdecl SV_EndClientSnapshot(client_t *client, msg_t *msg)
{
    unsigned int clientNum; // [esp+30h] [ebp-4h]

    clientNum = client - svs.clients;
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2056,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    SV_PacketDataIsUnknown(clientNum, msg);
    if (client->header.state != 1)
        SV_WriteDownloadToClient(client, msg);
    SV_PacketDataIsHeader(clientNum, msg);
    MSG_WriteByte(msg, 7u);
    if (msg->overflowed)
    {
        Com_PrintWarning(15, "WARNING: msg overflowed for %s, trying to recover\n", client->name);
        if (client->header.state == 4 || client->header.state == 1)
        {
            SV_PrintServerCommandsForClient(client);
            MSG_Init(msg, tempSnapshotMsgBuf, 0x20000);
            MSG_WriteLong(msg, client->lastClientCommand);
            SV_UpdateServerCommandsToClient_PreventOverflow(client, msg, 0x20000);
            MSG_WriteByte(msg, 7u);
        }
        if (msg->overflowed)
        {
            Com_PrintWarning(15, "WARNING: client disconnected for msg overflow: %s\n", client->name);
            NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "disconnect");
            SV_DropClient(client, "EXE_SERVERMESSAGEOVERFLOW", 1);
        }
    }
    if (sv_debugPacketContents->current.enabled)
        Com_Printf(15, "Snapshot finished for %s\n", client->name);

    if (client->tempPacketDebugging)
        Dvar_SetBool((dvar_s *)sv_debugPacketContents, 0);

    SV_AnalyzePacketData(clientNum, msg);

    if (client->header.netchan.remoteAddress.type != NA_LOOPBACK)
        SV_TrackSnapshotSize(msg->cursize);

    PROF_SCOPED("SV_SendMessageToClient");
    SV_SendMessageToClient(msg, client);
}

void __cdecl SV_PrintServerCommandsForClient(client_t *client)
{
    int i; // [esp+0h] [ebp-4h]

    Com_Printf(15, "-- Unacknowledged Server Commands for client %i:%s --\n", client - svs.clients, client->name);
    for (i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i)
        Com_Printf(
            15,
            "cmd %5d: %8d: %s\n",
            i,
            client->reliableCommandInfo[i & 0x7F].time,
            client->reliableCommandInfo[i & 0x7F].cmd);
    Com_Printf(15, "----------");
}

void __cdecl SV_SetServerStaticHeader()
{
    svsHeader.clients = svs.clients;
    svsHeader.time = svs.time;
    svsHeader.snapFlagServerBit = svs.snapFlagServerBit;
    svsHeader.numSnapshotEntities = svs.numSnapshotEntities;
    svsHeader.numSnapshotClients = svs.numSnapshotClients;
    svsHeader.nextSnapshotEntities = svs.nextSnapshotEntities;
    svsHeader.nextSnapshotClients = svs.nextSnapshotClients;
    svsHeader.snapshotEntities = svs.snapshotEntities;
    svsHeader.snapshotClients = svs.snapshotClients;
    svsHeader.svEntities = sv.svEntities;
    svsHeader.mapCenter[0] = svs.mapCenter[0];
    svsHeader.mapCenter[1] = svs.mapCenter[1];
    svsHeader.mapCenter[2] = svs.mapCenter[2];
    svsHeader.cachedSnapshotEntities = svs.cachedSnapshotEntities;
    svsHeader.cachedSnapshotClients = svs.cachedSnapshotClients;
    svsHeader.archivedSnapshotBuffer = svs.archivedSnapshotBuffer;
    svsHeader.cachedSnapshotFrames = (cachedSnapshot_t *)&svs;
    svsHeader.nextCachedSnapshotFrames = svs.nextCachedSnapshotFrames;
    svsHeader.nextArchivedSnapshotFrames = svs.nextArchivedSnapshotFrames;
    svsHeader.nextCachedSnapshotEntities = svs.nextCachedSnapshotEntities;
    svsHeader.nextCachedSnapshotClients = svs.nextCachedSnapshotClients;
    svsHeader.num_entities = sv.num_entities;
    svsHeader.maxclients = sv_maxclients->current.integer;
    svsHeader.fps = sv_fps->current.integer;
    svsHeader.clientArchive = sv_clientArchive->current.color[0];
    svsHeader.gentities = sv.gentities;
    svsHeader.gentitySize = sv.gentitySize;
    svsHeader.firstClientState = G_GetClientState(0);
    svsHeader.firstPlayerState = (playerState_s *)G_GetPlayerState(0);
    svsHeader.clientSize = G_GetClientSize();
    svsHeaderValid = 1;
}

void __cdecl SV_GetServerStaticHeader()
{
    iassert(svsHeader.clients == svs.clients);
    iassert(svsHeader.snapFlagServerBit == svs.snapFlagServerBit);
    iassert(svsHeader.numSnapshotEntities == svs.numSnapshotEntities);
    iassert(svsHeader.numSnapshotClients == svs.numSnapshotClients);
    iassert(svsHeader.nextSnapshotEntities == svs.nextSnapshotEntities);
    iassert(svsHeader.nextSnapshotClients == svs.nextSnapshotClients);
    iassert(svsHeader.snapshotEntities == svs.snapshotEntities);
    iassert(svsHeader.snapshotClients == svs.snapshotClients);
    iassert(svsHeader.svEntities == sv.svEntities);
    if (svsHeader.mapCenter[0] != svs.mapCenter[0]
        || svsHeader.mapCenter[1] != svs.mapCenter[1]
        || svsHeader.mapCenter[2] != svs.mapCenter[2])
    {
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2175,
            0,
            "%s",
            "Vec3Compare( svs.mapCenter, svsHeader.mapCenter )");
    }
    if (svsHeader.cachedSnapshotEntities != svs.cachedSnapshotEntities)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2176,
            0,
            "%s",
            "svsHeader.cachedSnapshotEntities == svs.cachedSnapshotEntities");
    if (svsHeader.cachedSnapshotClients != svs.cachedSnapshotClients)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2177,
            0,
            "%s",
            "svsHeader.cachedSnapshotClients == svs.cachedSnapshotClients");
    if (svsHeader.archivedSnapshotBuffer != svs.archivedSnapshotBuffer)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2178,
            0,
            "%s",
            "svsHeader.archivedSnapshotBuffer == svs.archivedSnapshotBuffer");
    if ((serverStatic_t *)svsHeader.cachedSnapshotFrames != &svs)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2179,
            0,
            "%s",
            "svsHeader.cachedSnapshotFrames == svs.cachedSnapshotFrames");
    svs.nextCachedSnapshotFrames = svsHeader.nextCachedSnapshotFrames;
    if (svsHeader.nextArchivedSnapshotFrames != svs.nextArchivedSnapshotFrames)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2181,
            0,
            "%s",
            "svsHeader.nextArchivedSnapshotFrames == svs.nextArchivedSnapshotFrames");
    svs.nextCachedSnapshotEntities = svsHeader.nextCachedSnapshotEntities;
    svs.nextCachedSnapshotClients = svsHeader.nextCachedSnapshotClients;
    if (svsHeader.num_entities != sv.num_entities)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 2184, 0, "%s", "svsHeader.num_entities == sv.num_entities");
    if (svsHeader.maxclients != sv_maxclients->current.integer)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2185,
            0,
            "%s",
            "svsHeader.maxclients == sv_maxclients->current.integer");
    if (svsHeader.fps != sv_fps->current.integer)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 2186, 0, "%s", "svsHeader.fps == sv_fps->current.integer");
    if (svsHeader.clientArchive != sv_clientArchive->current.color[0])
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2187,
            0,
            "%s",
            "svsHeader.clientArchive == (qboolean)sv_clientArchive->current.enabled");
    if (svsHeader.gentities != sv.gentities)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 2188, 0, "%s", "svsHeader.gentities == sv.gentities");
    if (svsHeader.gentitySize != sv.gentitySize)
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 2189, 0, "%s", "svsHeader.gentitySize == sv.gentitySize");
    if (svsHeader.firstClientState != G_GetClientState(0))
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2190,
            0,
            "%s",
            "svsHeader.firstClientState == G_GetClientState( 0 )");
    if ((gclient_s *)svsHeader.firstPlayerState != G_GetPlayerState(0))
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_mp.cpp",
            2191,
            0,
            "%s",
            "svsHeader.firstPlayerState == G_GetPlayerState( 0 )");
    if (svsHeader.clientSize != G_GetClientSize())
        MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 2192, 0, "%s", "svsHeader.clientSize == G_GetClientSize()");
    svsHeaderValid = 0;
}

void __cdecl SV_WriteVoiceDataToClient(client_t *client, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-4h]

    if (client->voicePacketCount <= 0)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 30, 0, "%s", "client->voicePacketCount > 0");
    if (client->voicePacketCount > 40)
        MyAssertHandler(
            ".\\server_mp\\sv_voice_mp.cpp",
            31,
            0,
            "%s",
            "client->voicePacketCount <= MAX_SERVER_QUEUED_VOICE_PACKETS");
    MSG_WriteByte(msg, client->voicePacketCount);
    for (packet = 0; packet < client->voicePacketCount; ++packet)
    {
        MSG_WriteByte(msg, client->voicePackets[packet].talker);
        if (client->voicePackets[packet].dataSize >= 0x10000)
            MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 40, 0, "%s", "client->voicePackets[packet].dataSize < (2<<15)");
        MSG_WriteByte(msg, client->voicePackets[packet].dataSize);
        MSG_WriteData(msg, client->voicePackets[packet].data, client->voicePackets[packet].dataSize);
    }
    if (msg->overflowed)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 50, 0, "%s", "!msg->overflowed");
}

void __cdecl SV_SendClientVoiceData(client_t *client)
{
    unsigned __int8 *msg_buf; // [esp+0h] [ebp-34h]
    msg_t msg; // [esp+Ch] [ebp-28h] BYREF

    LargeLocal msg_buf_large_local(0x20000);
    //LargeLocal::LargeLocal(&msg_buf_large_local, 0x20000);
    //msg_buf = LargeLocal::GetBuf(&msg_buf_large_local);
    msg_buf = msg_buf_large_local.GetBuf();
    if (client->voicePacketCount < 0)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 66, 0, "%s", "client->voicePacketCount >= 0");
    if (client->header.state == 4 && client->voicePacketCount)
    {
        MSG_Init(&msg, msg_buf, 0x20000);
        if (msg.cursize)
            MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 75, 0, "%s", "msg.cursize == 0");
        if (msg.bit)
            MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 76, 0, "%s", "msg.bit == 0");
        MSG_WriteString(&msg, "v");
        SV_WriteVoiceDataToClient(client, &msg);
        if (msg.overflowed)
        {
            Com_PrintWarning(15, "WARNING: voice msg overflowed for %s\n", client->name);
        }
        else
        {
            NET_OutOfBandVoiceData(NS_SERVER, client->header.netchan.remoteAddress, msg.data, msg.cursize);
            client->voicePacketCount = 0;
        }
    }
    //LargeLocal::~LargeLocal(&msg_buf_large_local);
}

void __cdecl SV_SendClientMessages()
{
    float comp_ratio; // [esp+C0h] [ebp-94h]
    float ave; // [esp+C4h] [ebp-90h]
    float avea; // [esp+C4h] [ebp-90h]
    float aveb; // [esp+C4h] [ebp-90h]
    float uave; // [esp+C8h] [ebp-8Ch]
    float uavea; // [esp+C8h] [ebp-8Ch]
    float uaveb; // [esp+C8h] [ebp-8Ch]
    archivedSnapshot_s *frame; // [esp+CCh] [ebp-88h]
    client_t *c; // [esp+D0h] [ebp-84h]
    client_t *ca; // [esp+D0h] [ebp-84h]
    int maxclients; // [esp+D4h] [ebp-80h]
    int numclients; // [esp+D8h] [ebp-7Ch]
    bool valid[64]; // [esp+DCh] [ebp-78h] BYREF
    int startIndex; // [esp+120h] [ebp-34h]
    msg_t msg; // [esp+124h] [ebp-30h] BYREF
    int partSize; // [esp+14Ch] [ebp-8h]
    int i; // [esp+150h] [ebp-4h]

    PROF_SCOPED("SV_SendClientMessages");

    numclients = 0;
    sv.bpsTotalBytes = 0;
    sv.ubpsTotalBytes = 0;
    memset((unsigned __int8 *)valid, 0, sizeof(valid));
    maxclients = sv_maxclients->current.integer;
    i = 0;
    c = svs.clients;
    while (i < maxclients)
    {
        if (c->header.state && svs.time >= c->nextSnapshotTime)
        {
            ++numclients;
            if (c->header.netchan.unsentFragments)
            {
                c->nextSnapshotTime = svs.time
                    + SV_RateMsec(c, c->header.netchan.unsentLength - c->header.netchan.unsentFragmentStart);
                SV_Netchan_TransmitNextFragment(c, &c->header.netchan);
            }
            else
            {
                valid[i] = 1;
                if (c->header.state == 4 || c->header.state == 1)
                {
                    PROF_SCOPED("SV_BuildClientSnapshot");
                    SV_BuildClientSnapshot(c);
                }
            }
        }
        ++i;
        ++c;
    }
    SV_SetServerStaticHeader();
    i = 0;
    ca = svs.clients;
    while (i < maxclients)
    {
        if (valid[i])
        {
            {
                PROF_SCOPED("SV_SendClientSnapshot");
                SV_BeginClientSnapshot(ca, &msg);
                if (ca->header.state == 4 || ca->header.state == 1)
                    SV_WriteSnapshotToClient(ca, &msg);
                SV_EndClientSnapshot(ca, &msg);
            }
            SV_SendClientVoiceData(ca);
        }
        ++i;
        ++ca;
    }
    if (sv_showAverageBPS->current.enabled && numclients > 0)
    {
        ave = 0.0;
        uave = 0.0;
        for (i = 0; i < 19; ++i)
        {
            sv.bpsWindow[i] = sv.bpsWindow[i + 1];
            ave = (double)sv.bpsWindow[i] + ave;
            sv.ubpsWindow[i] = sv.ubpsWindow[i + 1];
            uave = (double)sv.ubpsWindow[i] + uave;
        }
        sv.bpsWindow[19] = sv.bpsTotalBytes;
        avea = (double)sv.bpsTotalBytes + ave;
        sv.ubpsWindow[19] = sv.ubpsTotalBytes;
        uavea = (double)sv.ubpsTotalBytes + uave;
        if (sv.bpsTotalBytes >= sv.bpsMaxBytes)
            sv.bpsMaxBytes = sv.bpsTotalBytes;
        if (sv.ubpsTotalBytes >= sv.ubpsMaxBytes)
            sv.ubpsMaxBytes = sv.ubpsTotalBytes;
        if (++sv.bpsWindowSteps >= 20)
        {
            sv.bpsWindowSteps = 0;
            aveb = avea / 20.0;
            uaveb = uavea / 20.0;
            comp_ratio = (1.0 - aveb / uaveb) * 100.0;
            sv.ucompAve = sv.ucompAve + comp_ratio;
            Com_DPrintf(
                15,
                "bpspc(%2.0f) bps(%2.0f) pk(%i) ubps(%2.0f) upk(%i) cr(%2.2f) acr(%2.2f)\n",
                aveb / (double)numclients,
                aveb,
                sv.bpsMaxBytes,
                uaveb,
                sv.ubpsMaxBytes,
                comp_ratio,
                sv.ucompAve / (double)++sv.ucompNum);
        }
    }
    SV_DisablePacketData();
    g_archivingSnapshot = 1;
    if (sv.state == SS_GAME)
    {
        MSG_Init(&g_archiveMsg, tempServerMsgBuf, 0x20000);
        SV_ArchiveSnapshot(&g_archiveMsg);
    }
    SV_GetServerStaticHeader();
    if (sv.state == SS_GAME)
    {
        if (g_archiveMsg.overflowed)
        {
            Com_DPrintf(15, "SV_ArchiveSnapshot: ignoring snapshot because it overflowed.\n");
        }
        else
        {
            if (!svs.archivedSnapshotFrames)
                MyAssertHandler(".\\server_mp\\sv_snapshot_mp.cpp", 2415, 0, "%s", "svs.archivedSnapshotFrames");
            frame = &svs.archivedSnapshotFrames[svs.nextArchivedSnapshotFrames % 1200];
            frame->start = svs.nextArchivedSnapshotBuffer;
            frame->size = g_archiveMsg.cursize;
            startIndex = svs.nextArchivedSnapshotBuffer % 0x2000000;
            svs.nextArchivedSnapshotBuffer += g_archiveMsg.cursize;
            if (svs.nextArchivedSnapshotBuffer >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextArchivedSnapshotBuffer wrapped");
            partSize = 0x2000000 - startIndex;
            if (g_archiveMsg.cursize > 0x2000000 - startIndex)
            {
                memcpy(&svs.archivedSnapshotBuffer[startIndex], g_archiveMsg.data, partSize);
                memcpy(svs.archivedSnapshotBuffer, &g_archiveMsg.data[partSize], g_archiveMsg.cursize - partSize);
            }
            else
            {
                memcpy(&svs.archivedSnapshotBuffer[startIndex], g_archiveMsg.data, g_archiveMsg.cursize);
            }
            if (++svs.nextArchivedSnapshotFrames >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextArchivedSnapshotFrames wrapped");
        }
    }
    g_archivingSnapshot = 0;
    if (sv_debugPacketContents->current.enabled || net_showprofile->current.integer)
        SV_EnablePacketData();
}

