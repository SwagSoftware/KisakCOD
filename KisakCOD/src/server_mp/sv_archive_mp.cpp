#include "server.h"

void __cdecl SV_ArchiveSnapshot(msg_t *msg)
{
    clientState_s *ClientStateLocal; // eax
    clientState_s *v2; // eax
    const char *v3; // eax
    float v4; // eax
    const char *v5; // eax
    const clientState_s *v6; // eax
    int FollowPlayerStateLocal; // eax
    const char *v8; // eax
    float v9; // ecx
    float *absmax; // [esp+3Ch] [ebp-3138h]
    float *v11; // [esp+40h] [ebp-3134h]
    float *absmin; // [esp+44h] [ebp-3130h]
    float *v13; // [esp+48h] [ebp-312Ch]
    cachedClient_s *v14; // [esp+68h] [ebp-310Ch]
    int num; // [esp+6Ch] [ebp-3108h]
    int numa; // [esp+6Ch] [ebp-3108h]
    client_t *clients; // [esp+74h] [ebp-3100h]
    int v18; // [esp+80h] [ebp-30F4h]
    SnapshotInfo_s snapInfo; // [esp+90h] [ebp-30E4h] BYREF
    archivedEntity_s *v20; // [esp+A8h] [ebp-30CCh]
    int v21; // [esp+ACh] [ebp-30C8h]
    int clientIndex; // [esp+B0h] [ebp-30C4h]
    cachedSnapshot_t *v23; // [esp+B4h] [ebp-30C0h]
    cachedSnapshot_t *v24; // [esp+B8h] [ebp-30BCh]
    int maxclients; // [esp+BCh] [ebp-30B8h]
    int num_clients; // [esp+C0h] [ebp-30B4h]
    gentity_s *v27; // [esp+C4h] [ebp-30B0h]
    int v28; // [esp+C8h] [ebp-30ACh]
    int number; // [esp+CCh] [ebp-30A8h]
    gentity_s *v30; // [esp+D0h] [ebp-30A4h]
    archivedEntity_s to; // [esp+D4h] [ebp-30A0h] BYREF
    int i; // [esp+1F0h] [ebp-2F84h]
    playerState_s ps; // [esp+1F4h] [ebp-2F80h] BYREF
    int clientNum; // [esp+3164h] [ebp-10h]
    archivedEntity_s *from; // [esp+3168h] [ebp-Ch]
    cachedSnapshot_t *v36; // [esp+316Ch] [ebp-8h]
    cachedClient_s *v37; // [esp+3170h] [ebp-4h]

    clientNum = 0;
    snapInfo.archived = 1;
    if (!svsHeaderValid)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 131, 0, "%s", "svsHeaderValid");
    if (!svsHeader.cachedSnapshotEntities)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 132, 0, "%s", "svsHeader.cachedSnapshotEntities");
    if (!svsHeader.cachedSnapshotClients)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 133, 0, "%s", "svsHeader.cachedSnapshotClients");
    if (!svsHeader.archivedSnapshotBuffer)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 134, 0, "%s", "svsHeader.archivedSnapshotBuffer");
    if (!svsHeader.cachedSnapshotFrames)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 135, 0, "%s", "svsHeader.cachedSnapshotFrames");
    //Profile_Begin(258);
    SV_ResetPacketData(clientNum, msg);
    SV_PacketDataIsNotNetworkData(clientNum, msg);
    v28 = svsHeader.nextCachedSnapshotFrames - 512;
    if (svsHeader.nextCachedSnapshotFrames - 512 < 0)
        v28 = 0;
    for (i = svsHeader.nextCachedSnapshotFrames - 1; i >= v28; --i)
    {
        v36 = &svsHeader.cachedSnapshotFrames[i % 512];
        if (v36->archivedFrame >= svsHeader.nextArchivedSnapshotFrames - svsHeader.fps && !v36->usesDelta)
        {
            if (v36->first_entity >= svsHeader.nextCachedSnapshotEntities - 0x4000
                && v36->first_client >= svsHeader.nextCachedSnapshotClients - 4096)
            {
                MSG_WriteBit0(msg);
                MSG_WriteLong(msg, v36->archivedFrame);
                MSG_WriteLong(msg, svsHeader.time);
                maxclients = svsHeader.maxclients;
                num_clients = v36->num_clients;
                v37 = 0;
                v18 = 0;
                v21 = 0;
                MSG_ClearLastReferencedEntity(msg);
                while (v18 < maxclients || v21 < num_clients)
                {
                    if (v18 >= maxclients || svsHeader.clients[v18].header.state >= 2)
                    {
                        if (v21 < num_clients)
                        {
                            v14 = &svsHeader.cachedSnapshotClients[(v21 + v36->first_client) % 4096];
                            clientIndex = v14->cs.clientIndex;
                        }
                        else
                        {
                            v14 = 0;
                            clientIndex = 9999;
                        }
                        snapInfo.clientNum = v18;
                        if (v18 == clientIndex)
                        {
                            if (!v14)
                                MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 214, 0, "%s", "cachedClient2");
                            ClientStateLocal = (clientState_s *)G_GetClientStateLocal(v18);
                            MSG_WriteDeltaClient(&snapInfo, msg, svsHeader.time, &v14->cs, ClientStateLocal, 1);
                            if (GetFollowPlayerStateLocal(v18, &ps))
                            {
                                MSG_WriteBit1(msg);
                                if (v14->playerStateExists)
                                    MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, &v14->ps, &ps);
                                else
                                    MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &ps);
                            }
                            else
                            {
                                MSG_WriteBit0(msg);
                            }
                            ++v21;
                            ++v18;
                        }
                        else if (v18 >= clientIndex)
                        {
                            if (v18 > clientIndex)
                                ++v21;
                        }
                        else
                        {
                            v2 = (clientState_s *)G_GetClientStateLocal(v18);
                            MSG_WriteDeltaClient(&snapInfo, msg, svsHeader.time, 0, v2, 1);
                            if (GetFollowPlayerStateLocal(v18, &ps))
                            {
                                MSG_WriteBit1(msg);
                                MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &ps);
                            }
                            else
                            {
                                MSG_WriteBit0(msg);
                            }
                            ++v18;
                        }
                    }
                    else
                    {
                        ++v18;
                    }
                }
                MSG_WriteBit0(msg);
                MSG_ClearLastReferencedEntity(msg);
                number = -1;
                for (num = 0; num < svsHeader.num_entities; ++num)
                {
                    v27 = SV_GentityNumLocal(num);
                    if (v27->r.linked)
                    {
                        v30 = v27;
                        if (v27->s.number != num)
                        {
                            v3 = va(
                                "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                                v30->s.number,
                                num,
                                v30->s.eType,
                                v30->r.currentOrigin[0],
                                v30->r.currentOrigin[1],
                                v30->r.currentOrigin[2]);
                            MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 272, 0, "%s\n\t%s", "ent->s.number == e", v3);
                        }
                        if (v30->r.broadcastTime
                            || (v30->r.svFlags & 1) == 0
                            && ((v30->r.svFlags & 0x18) != 0 || svsHeader.svEntities[v30->s.number].numClusters))
                        {
                            LODWORD(v4) = 376 * v30->s.number;
                            from = (archivedEntity_s *)((char *)&svsHeader.svEntities->baseline + LODWORD(v4));
                            if ((svEntity_s *)((char *)svsHeader.svEntities + LODWORD(v4)) == (svEntity_s *)-4)
                                MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 286, 0, "%s", "baseline");
                            memcpy(&to, v30, 0xF4u);
                            to.r.svFlags = v30->r.svFlags;
                            if (v30->r.broadcastTime)
                                to.r.svFlags |= 8u;
                            to.r.clientMask[0] = v30->r.clientMask[0];
                            to.r.clientMask[1] = v30->r.clientMask[1];
                            to.r.absmin[0] = v30->r.absmin[0];
                            to.r.absmin[1] = v30->r.absmin[1];
                            to.r.absmin[2] = v30->r.absmin[2];
                            to.r.absmax[0] = v30->r.absmax[0];
                            to.r.absmax[1] = v30->r.absmax[1];
                            to.r.absmax[2] = v30->r.absmax[2];
                            if (number == v30->s.number)
                            {
                                v5 = va("lastEntityNum is %i, cur entnum is %i", number, v30->s.number);
                                MyAssertHandler(
                                    ".\\server_mp\\sv_archive_mp.cpp",
                                    297,
                                    0,
                                    "%s\n\t%s",
                                    "lastEntityNum != ent->s.number",
                                    v5);
                            }
                            snapInfo.fromBaseline = 1;
                            MSG_WriteDeltaArchivedEntity(&snapInfo, msg, svsHeader.time, from, &to, 1);
                            snapInfo.fromBaseline = 0;
                            number = v30->s.number;
                        }
                    }
                }
                goto skipDelta;
            }
            break;
        }
    }
    MSG_WriteBit1(msg);
    MSG_WriteLong(msg, svsHeader.time);
    v24 = &svsHeader.cachedSnapshotFrames[svsHeader.nextCachedSnapshotFrames % 512];
    v23 = v24;
    v24->archivedFrame = svsHeader.nextArchivedSnapshotFrames;
    v23->num_entities = 0;
    v23->first_entity = svsHeader.nextCachedSnapshotEntities;
    v23->num_clients = 0;
    v23->first_client = svsHeader.nextCachedSnapshotClients;
    v23->usesDelta = 0;
    v23->time = svsHeader.time;
    MSG_ClearLastReferencedEntity(msg);
    i = 0;
    clients = svsHeader.clients;
    while (i < svsHeader.maxclients)
    {
        if (clients->header.state >= 2)
        {
            v37 = &svsHeader.cachedSnapshotClients[svsHeader.nextCachedSnapshotClients % 4096];
            v6 = G_GetClientStateLocal(i);
            memcpy(&v37->cs, v6, sizeof(v37->cs));
            MSG_WriteDeltaClient(&snapInfo, msg, svsHeader.time, 0, &v37->cs, 1);
            FollowPlayerStateLocal = GetFollowPlayerStateLocal(i, &v37->ps);
            v37->playerStateExists = FollowPlayerStateLocal;
            if (v37->playerStateExists)
            {
                MSG_WriteBit1(msg);
                MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &v37->ps);
            }
            else
            {
                MSG_WriteBit0(msg);
            }
            if (++svsHeader.nextCachedSnapshotClients >= 2147483646)
                Com_Error(ERR_FATAL, &byte_8AD8AC);
            ++v23->num_clients;
        }
        ++i;
        ++clients;
    }
    MSG_WriteBit0(msg);
    MSG_ClearLastReferencedEntity(msg);
    for (numa = 0; numa < svsHeader.num_entities; ++numa)
    {
        v27 = SV_GentityNumLocal(numa);
        if (v27->r.linked)
        {
            v30 = v27;
            if (v27->s.number != numa)
            {
                v8 = va(
                    "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                    v30->s.number,
                    numa,
                    v30->s.eType,
                    v30->r.currentOrigin[0],
                    v30->r.currentOrigin[1],
                    v30->r.currentOrigin[2]);
                MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 378, 0, "%s\n\t%s", "ent->s.number == e", v8);
            }
            if (v30->r.broadcastTime
                || (v30->r.svFlags & 1) == 0
                && ((v30->r.svFlags & 0x18) != 0 || svsHeader.svEntities[v30->s.number].numClusters))
            {
                LODWORD(v9) = 376 * v30->s.number;
                from = (archivedEntity_s *)((char *)&svsHeader.svEntities->baseline + LODWORD(v9));
                if ((svEntity_s *)((char *)svsHeader.svEntities + LODWORD(v9)) == (svEntity_s *)-4)
                    MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 392, 0, "%s", "baseline");
                v20 = &svsHeader.cachedSnapshotEntities[svsHeader.nextCachedSnapshotEntities % 0x4000];
                memcpy(v20, v30, 0xF4u);
                v20->r.svFlags = v30->r.svFlags;
                if (v30->r.broadcastTime)
                    v20->r.svFlags |= 8u;
                v20->r.clientMask[0] = v30->r.clientMask[0];
                v20->r.clientMask[1] = v30->r.clientMask[1];
                absmin = v20->r.absmin;
                v13 = v30->r.absmin;
                v20->r.absmin[0] = v30->r.absmin[0];
                absmin[1] = v13[1];
                absmin[2] = v13[2];
                absmax = v20->r.absmax;
                v11 = v30->r.absmax;
                v20->r.absmax[0] = v30->r.absmax[0];
                absmax[1] = v11[1];
                absmax[2] = v11[2];
                snapInfo.fromBaseline = 1;
                MSG_WriteDeltaArchivedEntity(&snapInfo, msg, svsHeader.time, from, v20, 1);
                snapInfo.fromBaseline = 0;
                if (++svsHeader.nextCachedSnapshotEntities >= 2147483646)
                    Com_Error(ERR_FATAL, &byte_8AD87C);
                ++v23->num_entities;
            }
        }
    }
    if (++svsHeader.nextCachedSnapshotFrames >= 2147483646)
        Com_Error(ERR_FATAL, &byte_8AD850);
skipDelta:
    MSG_WriteEntityIndex(&snapInfo, msg, 1023, 10);
    SV_PacketDataIsUnknown(clientNum, msg);
    //Profile_EndInternal(0);
}

gentity_s *__cdecl SV_GentityNumLocal(int num)
{
    if (!svsHeaderValid)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 35, 0, "%s", "svsHeaderValid");
    return (gentity_s *)((char *)svsHeader.gentities + num * svsHeader.gentitySize);
}

const clientState_s *__cdecl G_GetClientStateLocal(int clientNum)
{
    if (!svsHeaderValid)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 42, 0, "%s", "svsHeaderValid");
    return (clientState_s *)((char *)svsHeader.firstClientState + clientNum * svsHeader.clientSize);
}

int __cdecl GetFollowPlayerStateLocal(int clientNum, playerState_s *ps)
{
    unsigned int index; // [esp+8h] [ebp-8h]

    if (!svsHeaderValid)
        MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 58, 0, "%s", "svsHeaderValid");
    if ((*(int *)((_BYTE *)&svsHeader.firstPlayerState->otherFlags + clientNum * svsHeader.clientSize) & 4) != 0)
    {
        memcpy(
            (unsigned __int8 *)ps,
            (unsigned __int8 *)svsHeader.firstPlayerState + clientNum * svsHeader.clientSize,
            sizeof(playerState_s));
        for (index = 0; index < 0x1F && ps->hud.current[index].type; ++index)
        {
            memset((unsigned __int8 *)&ps->hud.current[index], 0, sizeof(ps->hud.current[index]));
            if (ps->hud.current[index].type)
                MyAssertHandler(".\\server_mp\\sv_archive_mp.cpp", 74, 0, "%s", "ps->hud.current[index].type == HE_TYPE_FREE");
        }
        while (index < 0x1F)
        {
            if (memcmp(&ps->hud.current[index], &g_dummyHudCurrent_1, 0xA0u))
                MyAssertHandler(
                    ".\\server_mp\\sv_archive_mp.cpp",
                    81,
                    0,
                    "%s",
                    "!memcmp( &ps->hud.current[index], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )");
            ++index;
        }
        return 1;
    }
    else
    {
        memset((unsigned __int8 *)ps, 0, sizeof(playerState_s));
        return 0;
    }
}

