#include "client_mp.h"
#include <qcommon/mem_track.h>
#include <qcommon/msg_mp.h>

char *svc_strings[256];

void __cdecl TRACK_cl_parse()
{
    track_static_alloc_internal(svc_strings, 1024, "svc_strings", 9);
}

void __cdecl SHOWNET(msg_t *msg, char *s)
{
    if (cl_shownet->current.integer >= 2)
        Com_Printf(14, "%3i:%s\n", msg->readcount - 1, s);
}

void __cdecl CL_SavePredictedOriginForServerTime(
    clientActive_t *cl,
    int serverTime,
    float *predictedOrigin,
    float *predictedVelocity,
    float *viewangles,
    int bobCycle,
    int movementDir)
{
    float *v7; // [esp+0h] [ebp-10h]
    float *velocity; // [esp+4h] [ebp-Ch]
    float *origin; // [esp+8h] [ebp-8h]
    unsigned int lastIndex; // [esp+Ch] [ebp-4h]

    lastIndex = (cl->clientArchiveIndex + 255) % 256;
    if (lastIndex >= 0x100)
        MyAssertHandler(
            ".\\client_mp\\cl_parse_mp.cpp",
            80,
            0,
            "lastIndex doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
            lastIndex,
            256);
    if (cl->clientArchive[lastIndex].serverTime != serverTime)
    {
        cl->clientArchive[cl->clientArchiveIndex].serverTime = serverTime;
        origin = cl->clientArchive[cl->clientArchiveIndex].origin;
        *origin = *predictedOrigin;
        origin[1] = predictedOrigin[1];
        origin[2] = predictedOrigin[2];
        velocity = cl->clientArchive[cl->clientArchiveIndex].velocity;
        *velocity = *predictedVelocity;
        velocity[1] = predictedVelocity[1];
        velocity[2] = predictedVelocity[2];
        cl->clientArchive[cl->clientArchiveIndex].bobCycle = bobCycle;
        cl->clientArchive[cl->clientArchiveIndex].movementDir = movementDir;
        v7 = cl->clientArchive[cl->clientArchiveIndex].viewangles;
        *v7 = *viewangles;
        v7[1] = viewangles[1];
        v7[2] = viewangles[2];
        cl->clientArchiveIndex = (cl->clientArchiveIndex + 1) % 256;
        if (cl->clientArchiveIndex >= 0x100u)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                92,
                0,
                "cl->clientArchiveIndex doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                cl->clientArchiveIndex,
                256);
    }
}

char __cdecl CL_GetPredictedOriginForServerTime(
    clientActive_t *cl,
    int serverTime,
    float *predictedOrigin,
    float *predictedVelocity,
    float *viewangles,
    int *bobCycle,
    int *movementDir)
{
    unsigned int v7; // edx
    unsigned int v9; // ecx
    int index; // [esp+24h] [ebp-8h]
    int indexa; // [esp+24h] [ebp-8h]
    int cmd; // [esp+28h] [ebp-4h]
    int cmda; // [esp+28h] [ebp-4h]

    for (cmd = 0; cmd < 256; ++cmd)
    {
        v7 = (cl->clientArchiveIndex + 256 - cmd - 1) % 256;
        index = v7;
        if (v7 >= 0x100)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                108,
                0,
                "index doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                v7,
                256);
        if (cl->clientArchive[index].serverTime <= serverTime)
        {
            if (cl->clientArchive[index].serverTime != serverTime)
                Com_Printf(
                    14,
                    "Couldn't find exact match for servertime %i, using servertime %i\n",
                    serverTime,
                    cl->clientArchive[index].serverTime);
            *predictedOrigin = cl->clientArchive[index].origin[0];
            predictedOrigin[1] = cl->clientArchive[index].origin[1];
            predictedOrigin[2] = cl->clientArchive[index].origin[2];
            *predictedVelocity = cl->clientArchive[index].velocity[0];
            predictedVelocity[1] = cl->clientArchive[index].velocity[1];
            predictedVelocity[2] = cl->clientArchive[index].velocity[2];
            *viewangles = cl->clientArchive[index].viewangles[0];
            viewangles[1] = cl->clientArchive[index].viewangles[1];
            viewangles[2] = cl->clientArchive[index].viewangles[2];
            *bobCycle = cl->clientArchive[index].bobCycle;
            *movementDir = cl->clientArchive[index].movementDir;
            if ((COERCE_UNSIGNED_INT(*predictedOrigin) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(predictedOrigin[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(predictedOrigin[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\client_mp\\cl_parse_mp.cpp",
                    120,
                    0,
                    "%s",
                    "!IS_NAN((predictedOrigin)[0]) && !IS_NAN((predictedOrigin)[1]) && !IS_NAN((predictedOrigin)[2])");
            }
            if ((COERCE_UNSIGNED_INT(*predictedVelocity) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(predictedVelocity[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(predictedVelocity[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\client_mp\\cl_parse_mp.cpp",
                    121,
                    0,
                    "%s",
                    "!IS_NAN((predictedVelocity)[0]) && !IS_NAN((predictedVelocity)[1]) && !IS_NAN((predictedVelocity)[2])");
            }
            return 1;
        }
    }
    Com_PrintError(14, "Unable to find predicted origin for server time %i.  Here's what we have:\n", serverTime);
    for (cmda = 0; cmda < 256; ++cmda)
    {
        v9 = (cl->clientArchiveIndex + 256 - cmda - 1) % 256;
        indexa = v9;
        if (v9 >= 0x100)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                130,
                0,
                "index doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                v9,
                256);
        Com_PrintError(14, "%i: %i\n", indexa, cl->clientArchive[indexa].serverTime);
    }
    return 0;
}

void __cdecl CL_DeltaClient(
    clientActive_t *cl,
    msg_t *msg,
    int time,
    clSnapshot_t *frame,
    unsigned int newnum,
    clientState_s *old,
    int unchanged)
{
    clientState_s *state; // [esp+8h] [ebp-4h]

    state = &cl->parseClients[cl->parseClientsNum & 0x7FF];
    if (unchanged)
    {
        memcpy(state, old, sizeof(clientState_s));
    }
    else if (MSG_ReadDeltaClient(msg, time, old, state, newnum))
    {
        return;
    }
    ++cl->parseClientsNum;
    ++frame->numClients;
}

void __cdecl CL_SystemInfoChanged(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-28h]
    char *t; // [esp+8h] [ebp-24h]
    LargeLocal value_large_local; // [esp+Ch] [ebp-20h] BYREF
    char *systemInfo; // [esp+14h] [ebp-18h]
    char (*key)[8192]; // [esp+18h] [ebp-14h]
    LargeLocal key_large_local; // [esp+1Ch] [ebp-10h] BYREF
    const char *s; // [esp+24h] [ebp-8h] BYREF
    char (*value)[8192]; // [esp+28h] [ebp-4h]

    LargeLocal::LargeLocal(&key_large_local, 0x2000);
    key = (char (*)[8192])LargeLocal::GetBuf(&key_large_local);
    LargeLocal::LargeLocal(&value_large_local, 0x2000);
    value = (char (*)[8192])LargeLocal::GetBuf(&value_large_local);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    systemInfo = &LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.stringOffsets[1]];
    v1 = Info_ValueForKey(systemInfo, "sv_serverid");
    LocalClientGlobals->serverId = atoi(v1);
    if (CL_GetLocalClientConnection(localClientNum)->demoplaying)
    {
        LargeLocal::~LargeLocal(&value_large_local);
        LargeLocal::~LargeLocal(&key_large_local);
    }
    else
    {
        if (!com_sv_running->current.enabled)
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
                    1112,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            if (MEMORY[0xE7A7CC][0] < 9)
            {
                s = Info_ValueForKey(systemInfo, "sv_cheats");
                if (!atoi(s))
                    Dvar_SetCheatState();
            }
        }
        s = Info_ValueForKey(systemInfo, "sv_iwds");
        v2 = Info_ValueForKey(systemInfo, "sv_iwdNames");
        FS_PureServerSetLoadedIwds((char *)s, v2);
        s = Info_ValueForKey(systemInfo, "sv_referencedIwds");
        v3 = Info_ValueForKey(systemInfo, "sv_referencedIwdNames");
        FS_ServerSetReferencedIwds((char *)s, v3);
        s = Info_ValueForKey(systemInfo, "sv_referencedFFCheckSums");
        t = Info_ValueForKey(systemInfo, "sv_referencedFFNames");
        FS_ServerSetReferencedFFs((char *)s, t);
        if (!com_sv_running->current.enabled)
        {
            s = systemInfo;
            while (s)
            {
                Info_NextPair(&s, (char *)key, (char *)value);
                if (!*(_BYTE *)key)
                    break;
                Dvar_SetFromStringByName((const char *)key, (char *)value);
            }
        }
        cl_connectedToPureServer = Dvar_GetBool("sv_pure");
        LargeLocal::~LargeLocal(&value_large_local);
        LargeLocal::~LargeLocal(&key_large_local);
    }
}

void __cdecl CL_ParseMapCenter(int localClientNum)
{
    char *mapCenterString; // [esp+0h] [ebp-4h]

    mapCenterString = CL_GetConfigString(localClientNum, 0xCu);
    sscanf(mapCenterString, "%f %f %f", cls.mapCenter, &cls.mapCenter[1], &cls.mapCenter[2]);
}

void __cdecl CL_ParseServerMessage(netsrc_t localClientNum, msg_t *msg)
{
    msg_t msgCompressed; // [esp+4h] [ebp-30h] BYREF
    int file; // [esp+2Ch] [ebp-8h]
    int cmd; // [esp+30h] [ebp-4h]

    if (cl_shownet->current.integer == 1)
    {
        Com_Printf(14, "%i ", msg->cursize);
    }
    else if (cl_shownet->current.integer >= 2)
    {
        Com_Printf(14, "------------------\n");
    }
    MSG_Init(&msgCompressed, msgCompressed_buf, 0x20000);
    if ((unsigned int)(msg->cursize - msg->readcount) > 0x20000)
        Com_Error(ERR_DROP, "Compressed msg overflow in CL_ParseServerMessage");
    msgCompressed.cursize = MSG_ReadBitsCompress(
        &msg->data[msg->readcount],
        msgCompressed_buf,
        msg->cursize - msg->readcount);
    while (2)
    {
        if (msgCompressed.overflowed)
            goto LABEL_23;
        cmd = MSG_ReadByte(&msgCompressed);
        if (cmd == 7)
        {
            SHOWNET(&msgCompressed, "END OF MESSAGE");
        LABEL_23:
            if (msgCompressed.overflowed)
                MSG_Discard(msg);
            return;
        }
        if (cl_shownet->current.integer >= 2)
        {
            if (svc_strings[cmd])
                SHOWNET(&msgCompressed, svc_strings[cmd]);
            else
                Com_Printf(14, "%3i:BAD CMD %i\n", msgCompressed.readcount - 1, cmd);
        }
        switch (cmd)
        {
        case 0:
            continue;
        case 1:
            CL_ParseGamestate(localClientNum, &msgCompressed);
            continue;
        case 4:
            CL_ParseCommandString(localClientNum, &msgCompressed);
            continue;
        case 5:
            CL_ParseDownload(localClientNum, &msgCompressed);
            continue;
        case 6:
            CL_ParseSnapshot(localClientNum, &msgCompressed);
            continue;
        default:
            file = FS_FOpenFileWrite("badpacket.dat");
            if (file)
            {
                FS_Write((char *)msg->data, msg->cursize, file);
                FS_FCloseFile(file);
            }
            Com_PrintError(1, "CL_ParseServerMessage: Illegible server message %d\n", cmd);
            MSG_Discard(msg);
            break;
        }
        break;
    }
}

void __cdecl CL_ParseSnapshot(int localClientNum, msg_t *msg)
{
    const char *v2; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-20h]
    int serverTimeBackup; // [esp+4h] [ebp-1Ch]
    clientConnection_t *clc; // [esp+8h] [ebp-18h]
    int deltaNum; // [esp+Ch] [ebp-14h]
    int oldMessageNum; // [esp+10h] [ebp-10h]
    int i; // [esp+14h] [ebp-Ch]
    clSnapshot_t *old; // [esp+18h] [ebp-8h]
    int packetNum; // [esp+1Ch] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    memset((unsigned __int8 *)&newSnap, 0, sizeof(newSnap));
    newSnap.serverCommandNum = clc->serverCommandSequence;
    newSnap.serverTime = MSG_ReadLong(msg);
    newSnap.messageNum = clc->serverMessageSequence;
    deltaNum = MSG_ReadByte(msg);
    if (deltaNum)
        newSnap.deltaNum = newSnap.messageNum - deltaNum;
    else
        newSnap.deltaNum = -1;
    newSnap.snapFlags = MSG_ReadByte(msg);
    if (newSnap.deltaNum > 0)
    {
        old = &LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F];
        if (!old->valid)
        {
            Com_PrintError(14, "Delta from invalid frame (not supposed to happen!).\n");
            MSG_Discard(msg);
            return;
        }
        if (LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F].messageNum != newSnap.deltaNum)
        {
            Com_DPrintf(14, "Delta frame too old.\n");
            MSG_Discard(msg);
            return;
        }
        if (LocalClientGlobals->parseEntitiesNum - LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F].parseEntitiesNum > 1920)
        {
            Com_DPrintf(14, "Delta parseEntitiesNum too old.\n");
            MSG_Discard(msg);
            return;
        }
        if (LocalClientGlobals->parseClientsNum - LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F].parseClientsNum > 1920)
        {
            Com_DPrintf(14, "Delta parseClientsNum too old.\n");
            MSG_Discard(msg);
            return;
        }
        newSnap.valid = 1;
    }
    else
    {
        newSnap.valid = 1;
        old = 0;
        clc->demowaiting = 0;
    }
    serverTimeBackup = LocalClientGlobals->serverTime;
    SHOWNET(msg, "playerstate");
    if (old)
        MSG_ReadDeltaPlayerstate(localClientNum, msg, newSnap.serverTime, &old->ps, &newSnap.ps, 1);
    else
        MSG_ReadDeltaPlayerstate(localClientNum, msg, newSnap.serverTime, 0, &newSnap.ps, 1);
    if (serverTimeBackup != LocalClientGlobals->serverTime)
    {
        v2 = va(
            "cl->serverTime changed from %i to %i in MSG_ReadDeltaPlayerstate()\n",
            serverTimeBackup,
            LocalClientGlobals->serverTime);
        MyAssertHandler(".\\client_mp\\cl_parse_mp.cpp", 669, 0, "%s\n\t%s", "serverTimeBackup == cl->serverTime", v2);
    }
    MSG_ClearLastReferencedEntity(msg);
    SHOWNET(msg, "packet entities");
    CL_ParsePacketEntities(
        __SPAIR64__((unsigned int)msg, (unsigned int)LocalClientGlobals),
        newSnap.serverTime,
        old,
        &newSnap);
    MSG_ClearLastReferencedEntity(msg);
    SHOWNET(msg, "packet clients");
    CL_ParsePacketClients(LocalClientGlobals, msg, newSnap.serverTime, old, &newSnap);
    if (msg->overflowed)
    {
        newSnap.valid = 0;
    }
    else if (newSnap.valid)
    {
        oldMessageNum = LocalClientGlobals->snap.messageNum + 1;
        if (newSnap.messageNum - oldMessageNum >= 32)
            oldMessageNum = newSnap.messageNum - 31;
        while (oldMessageNum < newSnap.messageNum)
            LocalClientGlobals->snapshots[oldMessageNum++ & 0x1F].valid = 0;
        LocalClientGlobals->oldSnapServerTime = LocalClientGlobals->snap.serverTime;
        memcpy((unsigned __int8 *)&LocalClientGlobals->snap, (unsigned __int8 *)&newSnap, sizeof(LocalClientGlobals->snap));
        LocalClientGlobals->snap.ping = 999;
        for (i = 0; i < 32; ++i)
        {
            packetNum = ((unsigned __int8)clc->netchan.outgoingSequence - 1 - (_BYTE)i) & 0x1F;
            if (LocalClientGlobals->snap.ps.commandTime >= LocalClientGlobals->outPackets[packetNum].p_serverTime)
            {
                LocalClientGlobals->snap.ping = cls.realtime - LocalClientGlobals->outPackets[packetNum].p_realtime;
                break;
            }
        }
        memcpy(
            (unsigned __int8 *)&LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F],
            (unsigned __int8 *)&LocalClientGlobals->snap,
            sizeof(LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F]));
        if (cl_shownet->current.integer == 3)
            Com_Printf(
                14,
                "   snapshot:%i  delta:%i  ping:%i\n",
                LocalClientGlobals->snap.messageNum,
                LocalClientGlobals->snap.deltaNum,
                LocalClientGlobals->snap.ping);
        LocalClientGlobals->newSnapshots = 1;
    }
}

void __cdecl CL_ParsePacketEntities(__int64 cl, int time, clSnapshot_t *oldframe, clSnapshot_t *newframe)
{
    char *EntityTypeName; // eax
    char *v5; // eax
    double v6; // [esp+0h] [ebp-28h]
    double v7; // [esp+8h] [ebp-20h]
    double v8; // [esp+10h] [ebp-18h]
    entityState_s *oldstate; // [esp+18h] [ebp-10h]
    signed int newnum; // [esp+1Ch] [ebp-Ch]
    int oldindex; // [esp+20h] [ebp-8h]
    int oldnum; // [esp+24h] [ebp-4h]

    newframe->parseEntitiesNum = *(unsigned int *)(cl + 153128);
    newframe->numEntities = 0;
    oldindex = 0;
    oldstate = 0;
    if (oldframe)
    {
        if (oldframe->numEntities > 0)
        {
            oldstate = (entityState_s *)(cl + 244 * (oldframe->parseEntitiesNum & 0x7FF) + 1071824);
            oldnum = oldstate->number;
        }
        else
        {
            oldnum = 99999;
        }
    }
    else
    {
        oldnum = 99999;
    }
    while (!*(unsigned int *)HIDWORD(cl))
    {
        newnum = MSG_ReadEntityIndex((msg_t *)HIDWORD(cl), 0xAu);
        if ((unsigned int)newnum >= 0x400)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                313,
                0,
                "%s\n\t(newnum) = %i",
                "(newnum >= 0 && newnum < (1<<10))",
                newnum);
        if (newnum == 1023)
            break;
        if (*(unsigned int *)(HIDWORD(cl) + 28) > *(unsigned int *)(HIDWORD(cl) + 20))
            Com_Error(ERR_DROP, &byte_8776F8);
        while (oldnum < newnum && !*(unsigned int *)HIDWORD(cl))
        {
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  unchanged: %i\n", *(unsigned int *)(HIDWORD(cl) + 28), oldnum);
            CL_CopyOldEntity((clientActive_t *)cl, newframe, oldstate);
            if (++oldindex < oldframe->numEntities)
            {
                oldstate = (entityState_s *)(cl
                    + 244 * (((_WORD)oldindex + (unsigned __int16)oldframe->parseEntitiesNum) & 0x7FF)
                    + 1071824);
                oldnum = oldstate->number;
            }
            else
            {
                oldnum = 99999;
            }
            if (msg_dumpEnts->current.enabled)
            {
                EntityTypeName = BG_GetEntityTypeName(oldstate->eType);
                Com_Printf(14, "%3i: unchanged ent, eType %s\n", oldnum, EntityTypeName);
            }
        }
        if (oldnum == newnum)
        {
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  delta: %i\n", *(unsigned int *)(HIDWORD(cl) + 28), newnum);
            CL_DeltaEntity((clientActive_t *)cl, (msg_t *)HIDWORD(cl), time, newframe, newnum, oldstate);
            if (++oldindex < oldframe->numEntities)
            {
                oldstate = (entityState_s *)(cl
                    + 244 * (((_WORD)oldindex + (unsigned __int16)oldframe->parseEntitiesNum) & 0x7FF)
                    + 1071824);
                oldnum = oldstate->number;
            }
            else
            {
                oldnum = 99999;
            }
        }
        else
        {
            if (oldnum <= newnum)
                MyAssertHandler(".\\client_mp\\cl_parse_mp.cpp", 374, 0, "%s", "oldnum > newnum");
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  baseline: %i\n", *(unsigned int *)(HIDWORD(cl) + 28), newnum);
            CL_DeltaEntity(
                (clientActive_t *)cl,
                (msg_t *)HIDWORD(cl),
                time,
                newframe,
                newnum,
                (entityState_s *)(cl + 244 * newnum + 821968));
        }
    }
    while (oldnum != 99999 && !*(unsigned int *)HIDWORD(cl))
    {
        if (cl_shownet->current.integer == 3)
            Com_Printf(14, "%3i:  unchanged: %i\n", *(unsigned int *)(HIDWORD(cl) + 28), oldnum);
        CL_CopyOldEntity((clientActive_t *)cl, newframe, oldstate);
        if (++oldindex < oldframe->numEntities)
        {
            oldstate = (entityState_s *)(cl
                + 244 * (((_WORD)oldindex + (unsigned __int16)oldframe->parseEntitiesNum) & 0x7FF)
                + 1071824);
            oldnum = oldstate->number;
        }
        else
        {
            oldnum = 99999;
        }
        if (msg_dumpEnts->current.enabled)
        {
            v8 = oldstate->lerp.pos.trBase[2];
            v7 = oldstate->lerp.pos.trBase[1];
            v6 = oldstate->lerp.pos.trBase[0];
            v5 = BG_GetEntityTypeName(oldstate->eType);
            Com_Printf(14, "%3i: unchanged ent, eType %s at %f, %f, %f\n", oldnum, v5, v6, v7, v8);
        }
    }
    if (cl_shownuments->current.enabled || msg_dumpEnts->current.enabled)
        Com_Printf(14, "Entities in packet: %i\n", newframe->numEntities);
}

void __cdecl CL_DeltaEntity(
    clientActive_t *cl,
    msg_t *msg,
    int time,
    clSnapshot_t *frame,
    unsigned int newnum,
    entityState_s *old)
{
    if (!MSG_ReadDeltaEntity(msg, time, old, &cl->parseEntities[cl->parseEntitiesNum & 0x7FF], newnum))
    {
        ++cl->parseEntitiesNum;
        ++frame->numEntities;
    }
}

void __cdecl CL_CopyOldEntity(clientActive_t *cl, clSnapshot_t *frame, entityState_s *old)
{
    memcpy(
        &cl->parseEntities[cl->parseEntitiesNum++ & 0x7FF],
        old,
        sizeof(cl->parseEntities[cl->parseEntitiesNum++ & 0x7FF]));
    ++frame->numEntities;
}

void __cdecl CL_ParsePacketClients(
    clientActive_t *cl,
    msg_t *msg,
    int time,
    clSnapshot_t *oldframe,
    clSnapshot_t *newframe)
{
    clientState_s *oldstate; // [esp+0h] [ebp-80h]
    signed int newnum; // [esp+4h] [ebp-7Ch]
    clientState_s dummy; // [esp+8h] [ebp-78h] BYREF
    int oldindex; // [esp+78h] [ebp-8h]
    int oldnum; // [esp+7Ch] [ebp-4h]

    newframe->parseClientsNum = cl->parseClientsNum;
    newframe->numClients = 0;
    oldindex = 0;
    oldstate = 0;
    if (oldframe)
    {
        if (oldindex < oldframe->numClients)
        {
            oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
            oldnum = oldstate->clientIndex;
        }
        else
        {
            oldnum = 99999;
        }
    }
    else
    {
        oldnum = 99999;
    }
    while (!msg->overflowed && MSG_ReadBit(msg))
    {
        newnum = MSG_ReadEntityIndex(msg, 6u);
        if (msg->readcount > msg->cursize)
            Com_Error(ERR_DROP, &byte_877770);
        while (oldnum < newnum)
        {
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  unchanged: %i\n", msg->readcount, oldnum);
            CL_DeltaClient(cl, msg, time, newframe, oldnum, oldstate, 1);
            if (++oldindex < oldframe->numClients)
            {
                oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
                oldnum = oldstate->clientIndex;
            }
            else
            {
                oldnum = 99999;
            }
        }
        if (oldnum == newnum)
        {
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  delta: %i\n", msg->readcount, newnum);
            CL_DeltaClient(cl, msg, time, newframe, newnum, oldstate, 0);
            if (++oldindex < oldframe->numClients)
            {
                oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
                oldnum = oldstate->clientIndex;
            }
            else
            {
                oldnum = 99999;
            }
        }
        else
        {
            if (oldnum <= newnum)
                MyAssertHandler(".\\client_mp\\cl_parse_mp.cpp", 512, 0, "%s", "oldnum > newnum");
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  baseline: %i\n", msg->readcount, newnum);
            memset((unsigned __int8 *)&dummy, 0, sizeof(dummy));
            CL_DeltaClient(cl, msg, time, newframe, newnum, &dummy, 0);
        }
    }
    while (oldnum != 99999 && !msg->overflowed)
    {
        if (cl_shownet->current.integer == 3)
            Com_Printf(14, "%3i:  unchanged: %i\n", msg->readcount, oldnum);
        CL_DeltaClient(cl, msg, time, newframe, oldnum, oldstate, 1);
        if (++oldindex < oldframe->numClients)
        {
            oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
            oldnum = oldstate->clientIndex;
        }
        else
        {
            oldnum = 99999;
        }
    }
    if (cl_shownuments->current.enabled)
        Com_Printf(14, "Clients in packet: %i\n", newframe->numClients);
}

void __cdecl CL_ParseGamestate(netsrc_t localClientNum, msg_t *msg)
{
    int Long; // eax
    int v3; // eax
    unsigned int v4; // [esp+0h] [ebp-164h]
    unsigned int v5; // [esp+10h] [ebp-154h]
    unsigned int v6; // [esp+20h] [ebp-144h]
    int constConfigStringIndex; // [esp+34h] [ebp-130h]
    int constConfigStringIndexa; // [esp+34h] [ebp-130h]
    int lastStringIndex; // [esp+3Ch] [ebp-128h]
    int numConfigStrings; // [esp+40h] [ebp-124h]
    int currentConstConfigString; // [esp+44h] [ebp-120h]
    clientActive_t *LocalClientGlobals; // [esp+48h] [ebp-11Ch]
    unsigned int configStringIndex; // [esp+4Ch] [ebp-118h]
    unsigned int newnum; // [esp+50h] [ebp-114h]
    entityState_s nullstate; // [esp+54h] [ebp-110h] BYREF
    clientConnection_t *clc; // [esp+150h] [ebp-14h]
    entityState_s *to; // [esp+154h] [ebp-10h]
    int file; // [esp+158h] [ebp-Ch]
    char *s; // [esp+15Ch] [ebp-8h]
    int cmd; // [esp+160h] [ebp-4h]

    Con_Close(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    clc->connectPacketCount = 0;
    CL_ClearState(localClientNum);
    MSG_ClearLastReferencedEntity(msg);
    cls.mapCenter[0] = 0.0;
    cls.mapCenter[1] = 0.0;
    cls.mapCenter[2] = 0.0;
    clc->serverCommandSequence = MSG_ReadLong(msg);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->gameState.dataCount = 1;
    while (1)
    {
        cmd = MSG_ReadByte(msg);
        if (cmd == 7)
            break;
        if (cmd == 2)
        {
            currentConstConfigString = 0;
            lastStringIndex = -1;
            for (numConfigStrings = MSG_ReadShort(msg); numConfigStrings; --numConfigStrings)
            {
                if (MSG_ReadBit(msg))
                    configStringIndex = lastStringIndex + 1;
                else
                    configStringIndex = MSG_ReadBits(msg, 0xCu);
                if (configStringIndex >= 0x98A)
                    Com_Error(ERR_DROP, &byte_871558);
                while (constantConfigStrings[currentConstConfigString].configStringNum
                    && constantConfigStrings[currentConstConfigString].configStringNum < (signed int)configStringIndex)
                {
                    constConfigStringIndex = constantConfigStrings[currentConstConfigString].configStringNum;
                    s = (&MEMORY[0x94A784])[4 * currentConstConfigString];
                    v6 = strlen(s);
                    LocalClientGlobals->gameState.stringOffsets[constConfigStringIndex] = LocalClientGlobals->gameState.dataCount;
                    memcpy(
                        (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                        (unsigned __int8 *)s,
                        v6 + 1);
                    LocalClientGlobals->gameState.dataCount += v6 + 1;
                    ++currentConstConfigString;
                }
                if (constantConfigStrings[currentConstConfigString].configStringNum == configStringIndex)
                    ++currentConstConfigString;
                s = MSG_ReadBigString(msg);
                v5 = strlen(s);
                if ((int)(v5 + LocalClientGlobals->gameState.dataCount + 1) > 0x20000)
                    Com_Error(ERR_DROP, &byte_871538);
                LocalClientGlobals->gameState.stringOffsets[configStringIndex] = LocalClientGlobals->gameState.dataCount;
                memcpy(
                    (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                    (unsigned __int8 *)s,
                    v5 + 1);
                LocalClientGlobals->gameState.dataCount += v5 + 1;
                lastStringIndex = configStringIndex;
            }
            while (constantConfigStrings[currentConstConfigString].configStringNum)
            {
                constConfigStringIndexa = constantConfigStrings[currentConstConfigString].configStringNum;
                s = (&MEMORY[0x94A784])[4 * currentConstConfigString];
                v4 = strlen(s);
                LocalClientGlobals->gameState.stringOffsets[constConfigStringIndexa] = LocalClientGlobals->gameState.dataCount;
                memcpy(
                    (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                    (unsigned __int8 *)s,
                    v4 + 1);
                LocalClientGlobals->gameState.dataCount += v4 + 1;
                ++currentConstConfigString;
            }
            CL_ParseMapCenter(localClientNum);
        }
        else
        {
            if (cmd != 3)
            {
                file = FS_FOpenFileWrite("badpacket.dat");
                if (file)
                {
                    FS_Write((char *)msg->data, msg->cursize, file);
                    FS_FCloseFile(file);
                }
                Com_PrintError(1, "CL_ParseGamestate: bad command byte %d\n", cmd);
                MSG_Discard(msg);
                return;
            }
            newnum = MSG_ReadEntityIndex(msg, 0xAu);
            if (newnum >= 0x400)
                Com_Error(ERR_DROP, &byte_8777C0, newnum);
            memset((unsigned __int8 *)&nullstate, 0, sizeof(nullstate));
            to = &LocalClientGlobals->entityBaselines[newnum];
            MSG_ReadDeltaEntity(msg, 0, &nullstate, to, newnum);
        }
    }
    Long = MSG_ReadLong(msg);
    clc->clientNum = Long;
    v3 = MSG_ReadLong(msg);
    clc->checksumFeed = v3;
    if (useFastFile->current.enabled)
        DB_SyncXAssets();
    CL_SystemInfoChanged(localClientNum);
    cls.gameDirChanged = fs_gameDirVar->modified;
    if (FS_NeedRestart(clc->checksumFeed))
        FS_Restart(localClientNum, clc->checksumFeed);
    if (net_lanauthorize->current.enabled || !Sys_IsLANAddress(clc->serverAddress))
        CL_RequestAuthorization(localClientNum);
    CL_InitDownloads(localClientNum);
    Dvar_SetInt((dvar_s *)cl_paused, 0);
}

void __cdecl CL_ParseCommandString(int localClientNum, msg_t *msg)
{
    int seq; // [esp+0h] [ebp-10h]
    clientConnection_t *clc; // [esp+4h] [ebp-Ch]
    char *s; // [esp+Ch] [ebp-4h]

    seq = MSG_ReadLong(msg);
    s = MSG_ReadString(msg);
    clc = CL_GetLocalClientConnection(localClientNum);
    if (clc->serverCommandSequence < seq)
    {
        clc->serverCommandSequence = seq;
        I_strncpyz(clc->serverCommands[seq & 0x7F], s, 1024);
    }
}

