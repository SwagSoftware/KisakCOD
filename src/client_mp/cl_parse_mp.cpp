#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "client_mp.h"
#include <qcommon/mem_track.h>
#include <qcommon/msg_mp.h>
#include <cgame_mp/cg_local_mp.h>
#include <universal/com_files.h>
#include <win32/win_local.h>
#include <qcommon/cmd.h>
#include <qcommon/dl_main.h>
#include <universal/com_constantconfigstrings.h>
#include <database/database.h>
#include <qcommon/files.h>
#include <client/client.h>

const char* svc_strings[256] = {
    "svc_nop",
    "svc_gamestate",
    "svc_configstring",
    "svc_baseline",
    "svc_serverCommand",
    "svc_download",
    "svc_snapshot",
    "svc_EOF"
};
int autoupdateStarted;
char autoupdateFilename[64];
int cl_connectedToPureServer;

constexpr size_t CLIENT_ARCHIVE_SIZE = 256;

void __cdecl TRACK_cl_parse()
{
    track_static_alloc_internal(svc_strings, 1024, "svc_strings", 9);
}

void __cdecl SHOWNET(msg_t *msg, const char *s)
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

    lastIndex = (cl->clientArchiveIndex + CLIENT_ARCHIVE_SIZE - 1) % CLIENT_ARCHIVE_SIZE;
    if (lastIndex >= CLIENT_ARCHIVE_SIZE)
        MyAssertHandler(
            ".\\client_mp\\cl_parse_mp.cpp",
            80,
            0,
            "lastIndex doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
            lastIndex,
            CLIENT_ARCHIVE_SIZE);
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
        cl->clientArchiveIndex = (cl->clientArchiveIndex + 1) % CLIENT_ARCHIVE_SIZE;
        if (cl->clientArchiveIndex >= CLIENT_ARCHIVE_SIZE)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                92,
                0,
                "cl->clientArchiveIndex doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                cl->clientArchiveIndex,
                CLIENT_ARCHIVE_SIZE);
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

    for (cmd = 0; cmd < CLIENT_ARCHIVE_SIZE; ++cmd)
    {
        v7 = (cl->clientArchiveIndex + CLIENT_ARCHIVE_SIZE - cmd - 1) % CLIENT_ARCHIVE_SIZE;
        index = v7;
        if (v7 >= CLIENT_ARCHIVE_SIZE)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                108,
                0,
                "index doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                v7,
                CLIENT_ARCHIVE_SIZE);
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
            iassert(!IS_NAN((predictedOrigin)[0]) && !IS_NAN((predictedOrigin)[1]) && !IS_NAN((predictedOrigin)[2]));
            iassert(!IS_NAN((predictedVelocity)[0]) && !IS_NAN((predictedVelocity)[1]) && !IS_NAN((predictedVelocity)[2]));
            return 1;
        }
    }
    Com_PrintError(14, "Unable to find predicted origin for server time %i.  Here's what we have:\n", serverTime);
    for (cmda = 0; cmda < CLIENT_ARCHIVE_SIZE; ++cmda)
    {
        v9 = (cl->clientArchiveIndex + CLIENT_ARCHIVE_SIZE - cmda - 1) % CLIENT_ARCHIVE_SIZE;
        indexa = v9;
        if (v9 >= 0x100)
            MyAssertHandler(
                ".\\client_mp\\cl_parse_mp.cpp",
                130,
                0,
                "index doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                v9,
                CLIENT_ARCHIVE_SIZE);
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

    state = &cl->parseClients[cl->parseClientsNum & (MAX_PARSE_CLIENTS - 1)];
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
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-28h]
    const char *t; // [esp+8h] [ebp-24h]
    char *systemInfo; // [esp+14h] [ebp-18h]
    char (*key)[8192]; // [esp+18h] [ebp-14h]
    const char *s; // [esp+24h] [ebp-8h] BYREF
    char (*value)[8192]; // [esp+28h] [ebp-4h]

    LargeLocal key_large_local(0x2000); // [esp+1Ch] [ebp-10h] BYREF
    //LargeLocal::LargeLocal(&key_large_local, 0x2000);
    //key = (char (*)[8192])LargeLocal::GetBuf(&key_large_local);
    key = (char (*)[8192])key_large_local.GetBuf();

    LargeLocal value_large_local(0x2000); // [esp+Ch] [ebp-20h] BYREF
    //LargeLocal::LargeLocal(&value_large_local, 0x2000);
    //value = (char (*)[8192])LargeLocal::GetBuf(&value_large_local);
    value = (char (*)[8192])value_large_local.GetBuf();

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    systemInfo = &LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.stringOffsets[1]];
    v1 = Info_ValueForKey(systemInfo, "sv_serverid");
    LocalClientGlobals->serverId = atoi(v1);
    if (CL_GetLocalClientConnection(localClientNum)->demoplaying)
    {
        //LargeLocal::~LargeLocal(&value_large_local);
       // LargeLocal::~LargeLocal(&key_large_local);
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
            if (clientUIActives[0].connectionState < CA_ACTIVE)
            {
                s = Info_ValueForKey(systemInfo, "sv_cheats");
                if (!atoi(s))
                    Dvar_SetCheatState();
            }
        }
        s = Info_ValueForKey(systemInfo, "sv_iwds");
        v2 = Info_ValueForKey(systemInfo, "sv_iwdNames");
        FS_PureServerSetLoadedIwds((char *)s, (char*)v2);
        s = Info_ValueForKey(systemInfo, "sv_referencedIwds");
        v3 = Info_ValueForKey(systemInfo, "sv_referencedIwdNames");
        FS_ServerSetReferencedIwds((char *)s, (char*)v3);
        s = Info_ValueForKey(systemInfo, "sv_referencedFFCheckSums");
        t = Info_ValueForKey(systemInfo, "sv_referencedFFNames");
        FS_ServerSetReferencedFFs((char *)s, (char*)t);
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
        //LargeLocal::~LargeLocal(&value_large_local);
        //LargeLocal::~LargeLocal(&key_large_local);
    }
}

void __cdecl CL_ParseMapCenter(int localClientNum)
{
    char *mapCenterString; // [esp+0h] [ebp-4h]

    mapCenterString = CL_GetConfigString(localClientNum, 0xCu);
    sscanf(mapCenterString, "%f %f %f", cls.mapCenter, &cls.mapCenter[1], &cls.mapCenter[2]);
}

void __cdecl CL_ParseWWWDownload(int localClientNum, msg_t *msg)
{
    char *String; // eax
    char *fs_homepath; // [esp+10h] [ebp-10Ch]
    char toOSPath[260]; // [esp+14h] [ebp-108h] BYREF

    fs_homepath = (char *)Dvar_GetString("fs_homepath");
    I_strncpyz(cls.originalDownloadName, cls.downloadName, sizeof(cls.originalDownloadName));
    String = MSG_ReadString(msg);
    I_strncpyz(cls.downloadName, String, sizeof(cls.downloadName));
    cls.downloadSize = MSG_ReadLong(msg);
    cls.downloadFlags = MSG_ReadLong(msg);
    if ((cls.downloadFlags & 2) != 0)
    {
        Sys_OpenURL(cls.downloadName, 1);
        Cbuf_AddText(localClientNum, "quit\n");
        CL_AddReliableCommand(localClientNum, "wwwdl bbl8r");
        DL_CancelDownload();
        cls.wwwDlInProgress = 0;
    }
    else
    {
        legacyHacks.cl_downloadSize = cls.downloadSize;
        Com_DPrintf(14, "Server redirected download: %s\n", cls.downloadName);
        cls.wwwDlInProgress = 1;
        CL_AddReliableCommand(localClientNum, "wwwdl ack");
        FS_BuildOSPath(fs_homepath, cls.downloadTempName, (char *)"", toOSPath);
        I_strncpyz(cls.downloadTempName, toOSPath, sizeof(cls.downloadTempName));
        cls.downloadTempName[strlen(cls.downloadTempName) - 1] = 0;
        if (!DL_BeginDownload(cls.downloadTempName, cls.downloadName))
        {
            CL_AddReliableCommand(localClientNum, "wwwdl fail");
            DL_CancelDownload();
            cls.wwwDlInProgress = 0;
            Com_Printf(14, "Failed to initialize download for '%s'\n", cls.downloadName);
        }
        if ((cls.downloadFlags & 1) != 0)
        {
            CL_AddReliableCommand(localClientNum, "wwwdl bbl8r");
            cls.wwwDlDisconnected = 1;
        }
    }
}

void __cdecl CL_BeginDownload(char *localName, char *remoteName)
{
    const char *v2; // eax

    Com_DPrintf(
        14,
        "***** CL_BeginDownload *****\nLocalname: %s\nRemotename: %s\n****************************\n",
        localName,
        remoteName);
    CL_GetLocalClientConnection(0);
    I_strncpyz(cls.downloadName, localName, sizeof(cls.downloadName));
    Com_sprintf(cls.downloadTempName, sizeof(cls.downloadTempName), "%s.tmp", localName);
    I_strncpyz(legacyHacks.cl_downloadName, remoteName, 64);
    legacyHacks.cl_downloadSize = 0;
    legacyHacks.cl_downloadCount = 0;
    legacyHacks.cl_downloadTime = cls.realtime;
    cls.downloadBlock = 0;
    cls.downloadCount = 0;
    v2 = va("download %s", remoteName);
    CL_AddReliableCommand(0, v2);
}

void __cdecl CL_NextDownload(int localClientNum)
{
    char *localName; // [esp+24h] [ebp-Ch]
    char *s; // [esp+28h] [ebp-8h]
    char *sa; // [esp+28h] [ebp-8h]
    char *sb; // [esp+28h] [ebp-8h]
    unsigned __int8 *sc; // [esp+28h] [ebp-8h]
    char *remoteName; // [esp+2Ch] [ebp-4h]

    CL_GetLocalClientConnection(localClientNum);
    if (!cls.downloadList[0])
        goto LABEL_11;
    if (com_sv_running->current.enabled)
        MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2721, 0, "%s", "!com_sv_running->current.enabled");
    s = cls.downloadList;
    if (cls.downloadList[0] == 64)
        s = &cls.downloadList[1];
    remoteName = s;
    sa = strchr(s, '@');
    if (sa)
    {
        *sa = 0;
        localName = sa + 1;
        sb = strchr(sa + 1, '@');
        if (sb)
        {
            *sb = 0;
            sc = (unsigned __int8 *)(sb + 1);
        }
        else
        {
            sc = (unsigned __int8 *)&localName[strlen(localName)];
        }
        CL_BeginDownload(localName, remoteName);
        cls.downloadRestart = 1;
        memmove((unsigned __int8 *)cls.downloadList, sc, strlen((const char *)sc) + 1);
    }
    else
    {
    LABEL_11:
        CL_DownloadsComplete(localClientNum);
    }
}

char parseDownloadData[2048];
void __cdecl CL_ParseDownload(int localClientNum, msg_t *msg)
{
    const char *String; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int block; // [esp+0h] [ebp-Ch]
    int size; // [esp+4h] [ebp-8h]

    CL_GetLocalClientConnection(localClientNum);
    block = MSG_ReadLong(msg);
    if (block == -1)
    {
        if (cls.wwwDlInProgress)
        {
            MSG_ReadString(msg);
            MSG_ReadLong(msg);
            MSG_ReadLong(msg);
        }
        else
        {
            CL_ParseWWWDownload(localClientNum, msg);
        }
    }
    else
    {
        if (!block)
        {
            cls.downloadSize = MSG_ReadLong(msg);
            legacyHacks.cl_downloadSize = cls.downloadSize;
            if (cls.downloadSize < 0)
            {
                String = MSG_ReadString(msg);
                v3 = va("%s", String);
                Com_Error(ERR_DROP, v3);
                return;
            }
        }
        size = MSG_ReadShort(msg);
        if (size > 0)
            MSG_ReadData(msg, (unsigned __int8 *)parseDownloadData, size);
        if (cls.downloadBlock == block)
        {
            if (cls.download)
                goto LABEL_19;
            if (!cls.downloadTempName[0])
            {
                Com_Printf(14, "Server sending download, but no download was requested\n");
                CL_AddReliableCommand(localClientNum, "stopdl");
                return;
            }
            cls.download = FS_SV_FOpenFileWrite(cls.downloadTempName);
            if (cls.download)
            {
            LABEL_19:
                if (size)
                    FS_Write(parseDownloadData, size, cls.download);
                v5 = va("nextdl %d", cls.downloadBlock);
                CL_AddReliableCommand(localClientNum, v5);
                ++cls.downloadBlock;
                cls.downloadCount += size;
                legacyHacks.cl_downloadCount = cls.downloadCount;
                if (!size)
                {
                    if (cls.download)
                    {
                        FS_FCloseFile(cls.download);
                        cls.download = 0;
                        FS_SV_Rename(cls.downloadTempName, cls.downloadName);
                    }
                    cls.downloadName[0] = 0;
                    cls.downloadTempName[0] = 0;
                    legacyHacks.cl_downloadName[0] = 0;
                    CL_WritePacket(localClientNum);
                    CL_WritePacket(localClientNum);
                    CL_NextDownload(localClientNum);
                }
            }
            else
            {
                Com_Printf(14, "Could not create %s\n", cls.downloadTempName);
                CL_AddReliableCommand(localClientNum, "stopdl");
                CL_NextDownload(localClientNum);
            }
        }
        else
        {
            Com_DPrintf(14, "CL_ParseDownload: Expected block %d, got %d\n", cls.downloadBlock, block);
            if (block > cls.downloadBlock)
            {
                Com_DPrintf(14, "CL_ParseDownload: Sending retransmit request to get the missed block\n");
                v4 = va("retransdl %d", cls.downloadBlock);
                CL_AddReliableCommand(localClientNum, v4);
            }
        }
    }
}

unsigned __int8 msgCompressed_buf[0x20000];
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
    MSG_Init(&msgCompressed, msgCompressed_buf, sizeof(msgCompressed_buf));
    if ((unsigned int)(msg->cursize - msg->readcount) > sizeof(msgCompressed_buf))
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
            SHOWNET(&msgCompressed, (char*)"END OF MESSAGE");
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
            file = FS_FOpenFileWrite((char*)"badpacket.dat");
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

clSnapshot_t newSnap;
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
    SHOWNET(msg, (char *)"playerstate");
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
    SHOWNET(msg, (char *)"packet entities");
    CL_ParsePacketEntities(LocalClientGlobals, msg, newSnap.serverTime, old, &newSnap);
    MSG_ClearLastReferencedEntity(msg);
    SHOWNET(msg, (char*)"packet clients");
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

void __cdecl CL_ParsePacketEntities(
    clientActive_t *cl,
    msg_t *msg,
    int time,
    clSnapshot_t *oldframe,
    clSnapshot_t *newframe)
{
    char *EntityTypeName; // eax
    char *v6; // eax
    double v7; // [esp+0h] [ebp-28h]
    double v8; // [esp+8h] [ebp-20h]
    double v9; // [esp+10h] [ebp-18h]
    entityState_s *oldstate; // [esp+18h] [ebp-10h]
    signed int newnum; // [esp+1Ch] [ebp-Ch]
    int oldindex; // [esp+20h] [ebp-8h]
    int oldnum; // [esp+24h] [ebp-4h]

    newframe->parseEntitiesNum = cl->parseEntitiesNum;
    newframe->numEntities = 0;
    oldindex = 0;
    oldstate = 0;
    if (!oldframe)
    {
        oldnum = 99999;
    }
    else
    {
        if (oldindex >= oldframe->numEntities)
        {
            oldnum = 99999;
        }
        else
        {
            oldstate = &cl->parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES - 1)];
            oldnum = oldstate->number;
        }
    }
    while (!msg->overflowed)
    {
        newnum = MSG_ReadEntityIndex(msg, 0xAu);
        vassert(newnum >= 0 && newnum < (1 << 10), "(newnum) = %i", newnum);

        if (newnum == 1023)
            break;
        if (msg->readcount > msg->cursize)
            Com_Error(ERR_DROP, "CL_ParsePacketEntities: end of message");
        while (oldnum < newnum && !msg->overflowed)
        {
            // one or more entities from the old packet are unchanged
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  unchanged: %i\n", msg->readcount, oldnum);
            CL_CopyOldEntity(cl, newframe, oldstate);

            oldindex++;

            if (oldindex >= oldframe->numEntities)
            {
                oldnum = 99999;
            }
            else
            {
                oldstate = &cl->parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES - 1)];
                oldnum = oldstate->number;
            }

            if (msg_dumpEnts->current.enabled)
            {
                EntityTypeName = BG_GetEntityTypeName(oldstate->eType);
                Com_Printf(14, "%3i: unchanged ent, eType %s\n", oldnum, EntityTypeName);
            }
        }

        if (oldnum == newnum)
        {
            // delta from previous state
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  delta: %i\n", msg->readcount, newnum);
            CL_DeltaEntity(cl, msg, time, newframe, newnum, oldstate);

            oldindex++;

            if (oldindex >= oldframe->numEntities)
            {
                oldnum = 99999;
            }
            else
            {
                oldstate = &cl->parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES - 1)];
                oldnum = oldstate->number;
            }

            continue;
        }
        
        iassert(oldnum > newnum);

        //if (oldnum > newnum)
        {

            // delta from baseline
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  baseline: %i\n", msg->readcount, newnum);
            CL_DeltaEntity(cl, msg, time, newframe, newnum, &cl->entityBaselines[newnum]);

            continue;
        }
    }

    // any remaining entities in the old frame are copied over
    while (oldnum != 99999 && !msg->overflowed)
    {
        // one or more entities from the old packet are unchanged
        if (cl_shownet->current.integer == 3)
            Com_Printf(14, "%3i:  unchanged: %i\n", msg->readcount, oldnum);
        CL_CopyOldEntity(cl, newframe, oldstate);
        
        oldindex++;

        if (oldindex >= oldframe->numEntities)
        {
            oldnum = 99999;
        }
        else
        {
            oldstate = &cl->parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES - 1)];
            oldnum = oldstate->number;
        }
        
        if (msg_dumpEnts->current.enabled)
        {
            v9 = oldstate->lerp.pos.trBase[2];
            v8 = oldstate->lerp.pos.trBase[1];
            v7 = oldstate->lerp.pos.trBase[0];
            v6 = BG_GetEntityTypeName(oldstate->eType);
            Com_Printf(14, "%3i: unchanged ent, eType %s at %f, %f, %f\n", oldnum, v6, v7, v8, v9);
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

    if (!oldframe)
    {
        oldnum = 99999;
    }
    else
    {
        if (oldindex >= oldframe->numClients)
        {
            oldnum = 99999;
        }
        else
        {
            oldstate = &cl->parseClients[(oldframe->parseClientsNum + oldindex) & (MAX_PARSE_CLIENTS - 1)];
            oldnum = oldstate->clientIndex;
        }
    }

    while (!msg->overflowed && MSG_ReadBit(msg))
    {
        newnum = MSG_ReadEntityIndex(msg, 6u);
        if (msg->readcount > msg->cursize)
            Com_Error(ERR_DROP, "CL_ParsePacketClients: end of message");
        while (oldnum < newnum)
        {
            // one or more clients from the old packet are unchanged
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  unchanged: %i\n", msg->readcount, oldnum);
            CL_DeltaClient(cl, msg, time, newframe, oldnum, oldstate, 1);

            oldindex++;

            if (oldindex >= oldframe->numClients)
            {
                oldnum = 99999;
            }
            else
            {
                oldstate = &cl->parseClients[(oldframe->parseClientsNum + oldindex) & (MAX_PARSE_CLIENTS - 1)];
                oldnum = oldstate->clientIndex;
            }
        }
        
        if (oldnum == newnum)
        {
            // delta from previous state
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  delta: %i\n", msg->readcount, newnum);
            CL_DeltaClient(cl, msg, time, newframe, newnum, oldstate, 0);

            oldindex++;

            if (oldindex >= oldframe->numClients)
            {
                oldnum = 99999;
            }
            else
            {
                oldstate = &cl->parseClients[(oldframe->parseClientsNum + oldindex) & (MAX_PARSE_CLIENTS - 1)];
                oldnum = oldstate->clientIndex;
            }
        }
        else
        {
            // delta from baseline
            iassert(oldnum > newnum);
            if (cl_shownet->current.integer == 3)
                Com_Printf(14, "%3i:  baseline: %i\n", msg->readcount, newnum);
            memset(&dummy, 0, sizeof(dummy));
            CL_DeltaClient(cl, msg, time, newframe, newnum, &dummy, 0);
        }
    }

    // any remaining clients in the old frame are copied over
    while (oldnum != 99999 && !msg->overflowed)
    {
        if (cl_shownet->current.integer == 3)
            Com_Printf(14, "%3i:  unchanged: %i\n", msg->readcount, oldnum);
        CL_DeltaClient(cl, msg, time, newframe, oldnum, oldstate, 1);
        
        oldindex++;

        if (oldindex >= oldframe->numClients)
        {
            oldnum = 99999;
        }
        else
        {
            oldstate = &cl->parseClients[(oldframe->parseClientsNum + oldindex) & (MAX_PARSE_CLIENTS - 1)];
            oldnum = oldstate->clientIndex;
        }
    }

    if (cl_shownuments->current.enabled)
        Com_Printf(14, "Clients in packet: %i\n", newframe->numClients);
}

void __cdecl CL_InitDownloads(int localClientNum)
{
    char *v1; // eax
    const char *dir; // [esp+10h] [ebp-414h]
    FS_SERVER_COMPARE_RESULT compareResult; // [esp+14h] [ebp-410h]
    FS_SERVER_COMPARE_RESULT compareResulta; // [esp+14h] [ebp-410h]
    clientConnection_t *clc; // [esp+18h] [ebp-40Ch]
    char missingfiles[1028]; // [esp+1Ch] [ebp-408h] BYREF

    dir = FS_ShiftStr("ni]Zm^l", 7);
    clc = CL_GetLocalClientConnection(localClientNum);
    cls.wwwDlInProgress = 0;
    cls.wwwDlDisconnected = 0;
    CL_ClearStaticDownload();
    if (autoupdateStarted && NET_CompareAdr(cls.autoupdateServer, clc->serverAddress))
    {
        if (strlen(cl_updatefiles->current.string) > 4)
        {
            I_strncpyz(autoupdateFilename, (char *)cl_updatefiles->current.integer, 64);
            v1 = va("@%s/%s@%s/%s", dir, cl_updatefiles->current.string, dir, cl_updatefiles->current.string);
            I_strncpyz(cls.downloadList, v1, 1024);
            clientUIActives[localClientNum].connectionState = CA_CONNECTED;
            CL_NextDownload(localClientNum);
            return;
        }
    }
    else if (!com_sv_running->current.enabled)
    {
        if (cl_allowDownload->current.enabled)
        {
            compareResulta = FS_CompareWithServerFiles(cls.downloadList, 1024, 1);
            if (compareResulta == NEED_DOWNLOAD)
            {
                Com_Printf(14, "Need files: %s\n", cls.downloadList);
                if (cls.downloadList[0])
                {
                    clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                    CL_NextDownload(localClientNum);
                    return;
                }
            }
            else if (compareResulta == NOT_DOWNLOADABLE)
            {
                Com_Error(ERR_DROP, "%s is different from the server", cls.downloadList);
            }
        }
        else
        {
            compareResult = FS_CompareWithServerFiles(missingfiles, 1024, 0);
            if (compareResult == NEED_DOWNLOAD)
            {
                Com_Error(ERR_DROP, "You are missing some files referenced by the server: %sGo to the Multiplayer options menu to allow downloads", missingfiles);
            }
            else if (compareResult == NOT_DOWNLOADABLE)
            {
                Com_Error(ERR_DROP, "%s is different from the server", missingfiles);
            }
        }
    }
    CL_DownloadsComplete(localClientNum);
}

void __cdecl CL_ParseGamestate(netsrc_t localClientNum, msg_t *msg)
{
    int Long; // eax
    int v3; // eax
    int v4; // eax
    unsigned int v5; // [esp+0h] [ebp-164h]
    unsigned int v6; // [esp+10h] [ebp-154h]
    unsigned int v7; // [esp+20h] [ebp-144h]
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
    const char *s; // [esp+15Ch] [ebp-8h]
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
                if (configStringIndex >= 2442)
                    Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
                while (constantConfigStrings[currentConstConfigString].configStringNum
                    && constantConfigStrings[currentConstConfigString].configStringNum < (signed int)configStringIndex)
                {
                    constConfigStringIndex = constantConfigStrings[currentConstConfigString].configStringNum;
                    s = constantConfigStrings[currentConstConfigString].configString;
                    v7 = strlen(s);
                    LocalClientGlobals->gameState.stringOffsets[constConfigStringIndex] = LocalClientGlobals->gameState.dataCount;
                    memcpy(
                        (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                        (unsigned __int8 *)s,
                        v7 + 1);
                    LocalClientGlobals->gameState.dataCount += v7 + 1;
                    ++currentConstConfigString;
                }
                if (constantConfigStrings[currentConstConfigString].configStringNum == configStringIndex)
                    ++currentConstConfigString;
                s = MSG_ReadBigString(msg);
                v6 = strlen(s);
                if ((int)(v6 + LocalClientGlobals->gameState.dataCount + 1) > 0x20000)
                    Com_Error(ERR_DROP, "MAX_GAMESTATE_CHARS exceeded");
                LocalClientGlobals->gameState.stringOffsets[configStringIndex] = LocalClientGlobals->gameState.dataCount;
                memcpy(
                    (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                    (unsigned __int8 *)s,
                    v6 + 1);
                LocalClientGlobals->gameState.dataCount += v6 + 1;
                lastStringIndex = configStringIndex;
            }
            while (constantConfigStrings[currentConstConfigString].configStringNum)
            {
                constConfigStringIndexa = constantConfigStrings[currentConstConfigString].configStringNum;
                s = constantConfigStrings[currentConstConfigString].configString;
                v5 = strlen(s);
                LocalClientGlobals->gameState.stringOffsets[constConfigStringIndexa] = LocalClientGlobals->gameState.dataCount;
                memcpy(
                    (unsigned __int8 *)&LocalClientGlobals->gameState.stringData[LocalClientGlobals->gameState.dataCount],
                    (unsigned __int8 *)s,
                    v5 + 1);
                LocalClientGlobals->gameState.dataCount += v5 + 1;
                ++currentConstConfigString;
            }
            CL_ParseMapCenter(localClientNum);
        }
        else
        {
            if (cmd != 3)
            {
                file = FS_FOpenFileWrite((char*)"badpacket.dat");
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
                Com_Error(ERR_DROP, "Baseline number out of range: %i", newnum);
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
    Dvar_SetInt(cl_paused, 0);
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

