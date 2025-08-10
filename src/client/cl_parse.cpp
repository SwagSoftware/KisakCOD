#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <qcommon/msg.h>
#include <qcommon/mem_track.h>
#include <universal/q_shared.h>
#include "client.h"
#include <cgame/cg_main.h>
#include <server/server.h>
#include <cgame/cg_servercmds.h>
#include "cl_demo.h"
#include <game/g_local.h>

const char *svc_strings[256] =
{
  "svc_gamestate",
  "svc_configstring",
  "svc_snapshot",
  "svc_EOF",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};



void __cdecl TRACK_cl_parse()
{
    track_static_alloc_internal(svc_strings, 1024, "svc_strings", 9);
}

void __cdecl SHOWNET(msg_t *msg, char *s)
{
    if (cl_shownet->current.integer >= 2)
        Com_Printf(14, "%3i %3i:%s\n", msg->readcount - 1, msg->cursize, s);
}

void __cdecl CL_ParsePacketEntities(clientActive_t *cl, msg_t *msg, clSnapshot_t *newframe)
{
    int *p_parseEntitiesNum; // r30
    int *p_numEntities; // r29
    int parseEntitiesNum; // r9
    unsigned int i; // r31

    p_parseEntitiesNum = &cl->parseEntitiesNum;
    p_numEntities = &newframe->numEntities;
    parseEntitiesNum = cl->parseEntitiesNum;
    newframe->numEntities = 0;
    newframe->parseEntitiesNum = parseEntitiesNum;
    for (i = MSG_ReadBits(msg, 12); i != ENTITYNUM_NONE; i = MSG_ReadBits(msg, 12))
    {
        if (i >= 0x880)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp",
                78,
                0,
                "newnum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                i,
                2176);
        cl->parseEntityNums[(*p_parseEntitiesNum)++ & 0x7FF] = i;
        ++*p_numEntities;
    }
}

void __cdecl CL_ParseSnapshot(msg_t *msg)
{
    memset(clients, 0, 0xB2F8u);
    clients[0].snap.serverCommandNum = clientConnections[0].serverCommands.header.sequence;
    clients[0].snap.serverTime = MSG_ReadLong(msg);
    clients[0].snap.messageNum = clientConnections[0].serverMessageSequence;
    clients[0].snap.snapFlags = MSG_ReadByte(msg);
    clients[0].snap.valid = 1;
    if (cl_shownet->current.integer >= 2)
        Com_Printf(14, "%3i %3i:%s\n", msg->readcount - 1, msg->cursize, "playerstate");
    MSG_ReadDeltaPlayerstate(msg, &clients[0].snap.ps);
    if (cl_shownet->current.integer >= 2)
        Com_Printf(14, "%3i %3i:%s\n", msg->readcount - 1, msg->cursize, "packet entities");
    CL_ParsePacketEntities(clients, msg, &clients[0].snap);
    if (!clients[0].snap.valid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 128, 0, "%s", "cl->snap.valid");
    memcpy(clients[0].snapshots, clients, sizeof(clients[0].snapshots));
    if (cl_shownet->current.integer == 3)
        Com_Printf(14, "   snapshot:%i\n", clients[0].snap.messageNum);
}

void __cdecl CL_ParseGamestate(char *configstrings)
{
    int v2; // r28
    unsigned __int16 *v3; // r31
    int v4; // r22
    unsigned int v5; // r30

    if (!clientUIActives[0].isRunning)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 154, 0, "%s", "clUI->isRunning");
    if (clientUIActives[0].cgameInitialized)
    {
        CG_SetTime(com_time);
        CL_SetFrametime(0, 0);
    }
    v2 = 0;
    v3 = clients[0].configstrings;
    v4 = configstrings - (char *)clients[0].configstrings;
    do
    {
        v5 = *(unsigned __int16 *)((char *)v3 + v4);
        if (!*(unsigned __int16 *)((char *)v3 + v4))
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 164, 0, "%s", "s");
        if (*v3)
        {
            if (v5 == *v3)
                goto LABEL_15;
        }
        else if (clientUIActives[0].cgameInitialized)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 173, 0, "%s", "!clUI->cgameInitialized");
        }
        Scr_SetString(v3, v5);
        if (clientUIActives[0].isLoadComplete)
            CG_ConfigStringModifiedInternal(0, v2);
    LABEL_15:
        ++v3;
        ++v2;
    } while ((int)v3 < (int)clients[0].mapname);
}

void __cdecl CL_ParseServerCommands(msg_t *msg)
{
    int Short; // r3
    int v3; // r31
    char v4[32]; // [sp+50h] [-4020h] BYREF

    Short = MSG_ReadShort(msg);
    if (Short > 0)
    {
        v3 = Short;
        do
        {
            MSG_ReadString(msg, v4, 0x4000);
            if (CL_PreprocessServerCommand(v4))
                CG_ServerCommand(0);
            --v3;
        } while (v3);
    }
}

void __cdecl CL_RecordServerCommands(serverCommands_s *serverCommands)
{
    int i; // r31
    msg_t v3; // [sp+50h] [-4050h] BYREF
    unsigned __int8 v4[32]; // [sp+80h] [-4020h] BYREF

    if (!cls.demorecording)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 213, 0, "%s", "cls.demorecording");
    MSG_Init(&v3, v4, 0x4000);
    MSG_WriteByte(&v3, 3);
    MSG_WriteShort(&v3, serverCommands->header.sequence - serverCommands->header.sent);
    for (i = serverCommands->header.sent + 1; i <= serverCommands->header.sequence; ++i)
        MSG_WriteString(&v3, &serverCommands->buf[serverCommands->commands[(unsigned __int8)i]]);
    MSG_WriteByte(&v3, 4);
    CL_WriteDemoMessage(&v3, 0);
}

void __cdecl CL_ParseCommandString(serverCommands_s *serverCommands)
{
    int sequence; // r11
    int i; // r11
    int v4; // r10
    int v5; // r8

    CG_ExecuteNewServerCommands(0, clientConnections[0].serverCommands.header.sequence);
    sequence = clientConnections[0].serverCommands.header.sequence;
    if (clientConnections[0].serverCommands.header.sent != clientConnections[0].serverCommands.header.sequence)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp",
            246,
            0,
            "%s",
            "clc->serverCommands.header.sent == clc->serverCommands.header.sequence");
        sequence = clientConnections[0].serverCommands.header.sequence;
    }
    if (sequence - serverCommands->header.sequence > 0)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp",
            249,
            0,
            "%s",
            "clc->serverCommands.header.sequence - serverCommands->header.sequence <= 0");
        sequence = clientConnections[0].serverCommands.header.sequence;
    }
    if (sequence != serverCommands->header.sequence)
    {
        clientConnections[0].serverCommands.header.rover = serverCommands->header.rover;
        clientConnections[0].serverCommands.header.sequence = serverCommands->header.sequence;
        clientConnections[0].serverCommands.header.sent = serverCommands->header.sent;
        memcpy(clientConnections[0].serverCommands.buf, serverCommands->buf, serverCommands->header.rover);
        for (i = serverCommands->header.sent + 1;
            i <= serverCommands->header.sequence;
            clientConnections[0].serverCommands.commands[v4] = *(int *)((char *)&serverCommands->header.rover + v5))
        {
            v4 = (unsigned __int8)i;
            v5 = 4 * ((unsigned __int8)i++ + 2051);
        }
        if (cls.demorecording)
            CL_RecordServerCommands(serverCommands);
    }
}

void __cdecl CL_ParseServerMessage(msg_t *msg)
{
    int integer; // r11
    int v3; // r18
    int Byte; // r31
    int v5; // r11
    const char *v6; // r7
    int v7; // r5

    integer = cl_shownet->current.integer;
    if (integer == 1)
    {
        Com_Printf(14, "%i ", msg->cursize);
    }
    else if (integer >= 2)
    {
        Com_Printf(14, "------------------\n");
    }
    v3 = 0;
    while (1)
    {
        if (msg->readcount > msg->cursize)
            Com_Error(ERR_DROP, "CL_ParseServerMessage: read past end of server message");
        Byte = MSG_ReadByte(msg);
        v5 = cl_shownet->current.integer;
        if (Byte == 4)
            break;
        if (v5 >= 2)
        {
            v6 = svc_strings[Byte];
            v7 = msg->readcount - 1;
            if (v6)
                Com_Printf(14, "%3i %3i:%s\n", v7, msg->cursize, v6);
            else
                Com_Printf(14, "%3i:BAD CMD %i\n", v7, Byte);
        }
        if (Byte == 2)
        {
            if (v3)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 327, 0, "%s", "!parsedSnapshot");
            v3 = 1;
            CL_ParseSnapshot(msg);
            if (cls.demoplaying)
            {
                CL_ReadDemoSnapshotData();
                G_SetClientDemoServerSnapTime(clients[0].snap.serverTime);
            }
        }
        else if (Byte == 3)
        {
            if (!cls.demoplaying)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_parse.cpp", 341, 0, "%s", "cls.demoplaying");
            CL_ParseServerCommands(msg);
        }
        else
        {
            Com_Error(ERR_DROP, "CL_ParseServerMessage: Illegible server message %d", Byte);
        }
    }
    if (v5 >= 2)
        Com_Printf(14, "%3i %3i:%s\n", msg->readcount - 1, msg->cursize, "END OF MESSAGE");
}

