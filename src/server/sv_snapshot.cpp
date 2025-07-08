#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "server.h"
#include <client/cl_parse.h>
#include "sv_game.h"
#include <bgame/bg_public.h>
#include <qcommon/msg.h>
#include <game/g_local.h>
#include <client/cl_demo.h>
#include <client/client.h>


void __cdecl SV_WriteSnapshotToClient(client_t *client, msg_t *msg)
{
    int Time; // r3
    int numSnapshotEntities; // r7
    int v6; // r29
    int *snapshotEntities; // r31

    MSG_WriteByte(msg, 2);
    Time = G_GetTime();
    MSG_WriteLong(msg, Time);
    if (client->state != 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp",
            49,
            0,
            "%s\n\t(client->state) = %i",
            "(client->state == CS_ACTIVE)",
            client->state);
    MSG_WriteByte(msg, svs.snapFlagServerBit);
    MSG_WriteDeltaPlayerstate(msg, client->frames);
    numSnapshotEntities = sv.entityNumbers.numSnapshotEntities;
    if (sv.entityNumbers.numSnapshotEntities > 0x800u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp",
            56,
            0,
            "sv.entityNumbers.numSnapshotEntities not in [0, MAX_SNAPSHOT_ENTITIES]\n\t%i not in [%i, %i]",
            sv.entityNumbers.numSnapshotEntities,
            0,
            2048);
        numSnapshotEntities = sv.entityNumbers.numSnapshotEntities;
    }
    v6 = 0;
    if (numSnapshotEntities > 0)
    {
        snapshotEntities = sv.entityNumbers.snapshotEntities;
        do
        {
            if ((unsigned int)*snapshotEntities >= 0x880)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp",
                    60,
                    0,
                    "sv.entityNumbers.snapshotEntities[i] doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                    *snapshotEntities,
                    2176);
            MSG_WriteBits(msg, *snapshotEntities, 12);
            ++v6;
            ++snapshotEntities;
        } while (v6 < sv.entityNumbers.numSnapshotEntities);
    }
    MSG_WriteBits(msg, 2175, 12);
}

void __cdecl SV_UpdateServerCommandsToClient(client_t *client)
{
    CL_ParseCommandString(&client->reliableCommands);
    client->reliableCommands.header.sent = client->reliableCommands.header.sequence;
    client->reliableCommands.header.rover = 0;
}

void __cdecl SV_AddEntToSnapshot(int entnum)
{
    int numSnapshotEntities; // r11

    numSnapshotEntities = sv.entityNumbers.numSnapshotEntities;
    if (sv.entityNumbers.numSnapshotEntities == 2048)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp",
            95,
            0,
            "%s",
            "sv.entityNumbers.numSnapshotEntities != MAX_SNAPSHOT_ENTITIES");
        numSnapshotEntities = sv.entityNumbers.numSnapshotEntities;
    }
    *(unsigned int *)&sv.cmd[4 * numSnapshotEntities - 0x2000] = entnum;
    ++sv.entityNumbers.numSnapshotEntities;
}

void __cdecl SV_AddEntitiesVisibleFromPoint(int clientNum)
{
    int i; // r30
    gentity_s *v3; // r3
    gentity_s *v4; // r31
    int number; // r4
    const char *v6; // r3

    if (sv.state)
    {
        for (i = 0; i < sv.num_entities; ++i)
        {
            v3 = SV_GentityNum(i);
            v4 = v3;
            if (v3->r.linked)
            {
                number = v3->s.number;
                if (number != i)
                {
                    v6 = va(
                        "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                        number,
                        i,
                        (unsigned int)HIDWORD(COERCE_UNSIGNED_INT64(v3->r.currentOrigin[0])),
                        (unsigned int)COERCE_UNSIGNED_INT64(v3->r.currentOrigin[0]),
                        (unsigned int)COERCE_UNSIGNED_INT64(v3->r.currentOrigin[1]),
                        (unsigned int)COERCE_UNSIGNED_INT64(v3->r.currentOrigin[2]));
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp",
                        121,
                        0,
                        "%s\n\t%s",
                        "ent->s.number == e",
                        v6);
                }
                if ((v4->r.svFlags & 1) == 0 && i != clientNum)
                    SV_AddEntToSnapshot(i);
            }
        }
    }
}

void __cdecl SV_BuildClientSnapshot(client_t *client)
{
    playerState_s *frames; // r31
    playerState_s *v2; // r3
    unsigned int clientNum; // r31

    if (client->gentity)
    {
        frames = client->frames;
        sv.entityNumbers.numSnapshotEntities = 0;
        v2 = SV_GameClientNum(client - svs.clients);
        memcpy(frames, v2, sizeof(playerState_s));
        clientNum = frames->clientNum;
        if (clientNum >= 0x880)
            Com_Error(ERR_DROP, "SV_BuildClientSnapshot: bad gEnt");
        SV_AddEntitiesVisibleFromPoint(clientNum);
    }
}

void __cdecl SV_SendMessageToClient(msg_t *msg, client_t *client)
{
    int outgoingSequence; // r4

    MSG_WriteByte(msg, 4);
    outgoingSequence = client->netchan.outgoingSequence;
    client->netchan.outgoingSequence = outgoingSequence + 1;
    CL_PacketEvent(msg, outgoingSequence);
}

void __cdecl SV_BuildAndSendClientSnapshot(client_t *client)
{
    int outgoingSequence; // r4
    msg_t v3; // [sp+50h] [-4040h] BYREF
    unsigned __int8 v4[16]; // [sp+80h] [-4010h] BYREF

    SV_BuildClientSnapshot(client);
    MSG_Init(&v3, v4, 0x4000);
    SV_WriteSnapshotToClient(client, &v3);
    if (v3.overflowed)
        Com_Error(ERR_DROP, "SV_BuildAndSendClientSnapshot: bad gEnt");
    MSG_WriteByte(&v3, 4);
    outgoingSequence = client->netchan.outgoingSequence;
    client->netchan.outgoingSequence = outgoingSequence + 1;
    CL_PacketEvent(&v3, outgoingSequence);
}

void __cdecl SV_SendClientMessages()
{
    client_t *clients; // r11
    client_t *v1; // r30
    serverCommands_s *p_reliableCommands; // r29

    clients = svs.clients;
    if (!svs.clients)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp", 216, 0, "%s", "svs.clients");
        clients = svs.clients;
    }
    if (!clients->state)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp", 217, 0, "%s", "svs.clients[0].state");
    if (!CL_DemoPlaying())
    {
        G_SendClientMessages();
        v1 = svs.clients;
        p_reliableCommands = &svs.clients->reliableCommands;
        CL_ParseCommandString(&svs.clients->reliableCommands);
        v1->reliableCommands.header.sent = v1->reliableCommands.header.sequence;
        p_reliableCommands->header.rover = 0;
        //Profile_Begin(31);
        SV_BuildAndSendClientSnapshot(svs.clients);
        //Profile_EndInternal(0);
    }
}

void __cdecl SV_WriteSnapshotToClientCmd(void *cmdData)
{
    if (!alwaysfails)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp", 236, 0, "MP only");
}

void __cdecl SV_ArchiveSnapshotCmd(void *cmdData)
{
    if (!alwaysfails)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_snapshot.cpp", 242, 0, "MP only");
}

