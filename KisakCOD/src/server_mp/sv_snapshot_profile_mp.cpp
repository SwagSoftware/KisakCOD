#include "server.h"
#include <universal/com_files.h>
#include <cgame_mp/cg_local_mp.h>


//   unsigned int *bitsUsedForPlayerstates 85032704     sv_snapshot_profile_mp.obj
//   int (*)[1024] g_currentSnapshotPerEntity 85032728     sv_snapshot_profile_mp.obj
//   unsigned int (*)[160] currentSnapshotNetworkEntityFieldsChanged 8504a730     sv_snapshot_profile_mp.obj
//   unsigned int bitsUsedForServerCommands 8504e0b0     sv_snapshot_profile_mp.obj
//   BOOL g_archivingSnapshot 8504e0b9     sv_snapshot_profile_mp.obj
//   int originsSentDueToPredicitonError 8504e0bc     sv_snapshot_profile_mp.obj
//   struct ClientSnapshotData *s_clientSnapshotData 8504e0c0     sv_snapshot_profile_mp.obj
//   int originsSentDueToServerTimeMismatch 8504ea14     sv_snapshot_profile_mp.obj
//   unsigned int (*)[160] networkEntityFieldsChanged 8504ea20     sv_snapshot_profile_mp.obj
//   unsigned int *bitsUsedPerEType   850523a0     sv_snapshot_profile_mp.obj
//   unsigned char (*)[1024] g_currentSnapshotFieldsPerEntity 850528b0     sv_snapshot_profile_mp.obj
//   unsigned char *g_currentSnapshotPlayerStateFields 850589a0     sv_snapshot_profile_mp.obj
//   int (*)[13] g_bitsSent     850589b8     sv_snapshot_profile_mp.obj
int g_bitsSent[64][13];
int s_totalPacketDataSizes[20];
int s_packetMetaDataSize[64][20];
unsigned int s_packetModeStart[64];
packetModeList s_packetMode[64];
int g_currentSnapshotPerEntity[64][1024];
unsigned __int8 g_currentSnapshotFieldsPerEntity[64][1024];
unsigned __int8 g_currentSnapshotPlayerStateFields[64];
bool newDataReady;
unsigned int bitsUsedPerEType[256];
unsigned int bitsUsedForPlayerstates[7];
int playerStateFieldsChanged[161];
bool s_packetDataEnabled;
bool g_archivingSnapshot;
int s_floatBitsCompressed[60];
int s_originDeltaBits[8];
int s_originZDeltaBits[8];
int s_originZFullBits[17];
int s_originFullBits[17];
unsigned int networkEntityFieldsChanged[23][160];
unsigned int currentSnapshotNetworkEntityFieldsChanged[23][160];
unsigned int bitsUsedForServerCommands;
int s_currentEntType;
int s_currentEntNum;
int originsSentDueToPredicitonError;
int originsSentDueToServerTimeMismatch;
float s_stdSnapshotDeviation;
int s_maxSnapshotSize;
unsigned int huffBytesSeen[256];
int s_maxSnapshotSize;
int s_numSnapshotSamples;
int s_numSnapshotsBuiltSinceLastPoll;
int s_uncompressedDataSinceLastPoll;
int s_compressedDataSinceLastPoll;
int s_numSnapshotsSentSinceLastPoll;


const char *packetModeNames[20] =
{
  "Undefined",
  "Header",
  "Overhead",
  "Binary data",
  "Reliable data",
  "Zero float data (floats == 0)",
  "Small floats (sent as ints)",
  "Full floats",
  "Zero int data (ints == 0)",
  "Small angles",
  "Zero angles (angles == 0)",
  "Time delta",
  "Full times",
  "24 bit flags",
  "Ground entity",
  "Entity number",
  "Last field changed",
  "Not network data",
  "Origin delta",
  "Origin"
}; // idb

const char *s_analyzeEntityTypeNames[23] =
{
  "General Entity",
  "Player Entity",
  "Corpse Entity",
  "Item Entity",
  "Missle Entity",
  "Invisible Entity",
  "Script Mover Entity",
  "Sound Blend Entity",
  "FX Entity",
  "Loop FX Entity",
  "Primary Light Entity",
  "MG42 Entity",
  "Helicopter",
  "Plane",
  "Vehicle",
  "Vehicle Collmap",
  "Vehicle Corpse",
  "Temp Entity",
  "Archived Entity",
  "Client State",
  "Player State",
  "Hud Elem",
  "Baselines"
}; // idb

void __cdecl SV_ClearPacketAnalysis()
{
    int client; // [esp+0h] [ebp-8h]
    int dataType; // [esp+4h] [ebp-4h]

    memset(g_bitsSent, 0, sizeof(g_bitsSent));
    for (dataType = 0; dataType < 20; ++dataType)
    {
        for (client = 0; client < 64; ++client)
            s_totalPacketDataSizes[dataType] += s_packetMetaDataSize[client][dataType];
    }
    memset(s_packetMetaDataSize, 0, sizeof(s_packetMetaDataSize));
    memset(s_packetModeStart, 0, sizeof(s_packetModeStart));
    memset(s_packetMode, 0, sizeof(s_packetMode));
    memset(g_currentSnapshotPerEntity, 0, sizeof(g_currentSnapshotPerEntity));
    memset(g_currentSnapshotFieldsPerEntity[0], 0, sizeof(g_currentSnapshotFieldsPerEntity));
    memset(g_currentSnapshotPlayerStateFields, 0, sizeof(g_currentSnapshotPlayerStateFields));
    newDataReady = 0;
}

void __cdecl SV_TrackETypeBytes(unsigned int eType, int bits)
{
    if (eType >= 0x98)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            207,
            0,
            "%s\n\t(eType) = %i",
            "(eType >= 0 && eType < ET_EVENTS + EV_MAX_EVENTS)",
            eType);
    bitsUsedPerEType[eType] += bits;
}

void __cdecl SV_TrackPSBits(int bits)
{
    bitsUsedForPlayerstates[0] += bits;
}

void __cdecl SV_TrackPSFieldDeltasBits(int bits)
{
    bitsUsedForPlayerstates[1] += bits;
}

void __cdecl SV_TrackPSHudelemBits(int bits)
{
    bitsUsedForPlayerstates[5] += bits;
}

void __cdecl SV_TrackPSStatsBits(int bits)
{
    bitsUsedForPlayerstates[2] += bits;
}

void __cdecl SV_TrackPSAmmoBits(int bits)
{
    bitsUsedForPlayerstates[3] += bits;
}

void __cdecl SV_TrackPSObjectivesBits(int bits)
{
    bitsUsedForPlayerstates[4] += bits;
}

void __cdecl SV_TrackPSWeaponModelBits(int bits)
{
    bitsUsedForPlayerstates[6] += bits;
}

void __cdecl SV_TrackFieldsChanged(int lc)
{
    ++playerStateFieldsChanged[lc];
}

void __cdecl SV_DisablePacketData()
{
    s_packetDataEnabled = 0;
}

void __cdecl SV_EnablePacketData()
{
    s_packetDataEnabled = 1;
}

void __cdecl SV_ResetPacketData(int clientNum, const msg_t *msg)
{
    s_packetModeStart[clientNum] = MSG_GetUsedBitCount(msg);
    s_packetMode[clientNum] = PACKETDATA_FIRST;
}

bool __cdecl SV_IsPacketDataNetworkData()
{
    return !g_archivingSnapshot;
}

void __cdecl SV_PacketDataIsGroundEntity(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_GROUNDENTITY);
}

void __cdecl SV_PacketDataIsType(int clientNum, const msg_t *msg, packetModeList mode)
{
    const char *v3; // eax
    const char *PacketDataTypeName; // eax
    unsigned int bitsUsed; // [esp+4Ch] [ebp-Ch]
    unsigned int bitsUsedPrev; // [esp+50h] [ebp-8h]
    packetModeList oldMode; // [esp+54h] [ebp-4h]

    if (s_packetDataEnabled)
    {
        //Profile_Begin(291);
        oldMode = s_packetMode[clientNum];
        if (oldMode != mode || mode == PACKETDATA_FIRST)
        {
            bitsUsed = MSG_GetUsedBitCount(msg);
            if (bitsUsed < s_packetModeStart[clientNum])
            {
                v3 = va("bitsUsed is %i, start marker is %i", bitsUsed, s_packetModeStart[clientNum]);
                MyAssertHandler(
                    ".\\server_mp\\sv_snapshot_profile_mp.cpp",
                    361,
                    0,
                    "%s\n\t%s",
                    "bitsUsed >= s_packetModeStart[clientNum]",
                    v3);
            }
            bitsUsedPrev = s_packetModeStart[clientNum];
            s_packetMode[clientNum] = mode;
            s_packetModeStart[clientNum] = bitsUsed;
            s_packetMetaDataSize[clientNum][oldMode] += bitsUsed - bitsUsedPrev;
            if (sv_debugPacketContents->current.enabled && oldMode != PACKETDATA_NOTNETWORKDATA && bitsUsed > bitsUsedPrev)
            {
                if (oldMode == PACKETDATA_FIRST)
                    Com_Printf(15, "Unknown data!\n");
                PacketDataTypeName = SV_GetPacketDataTypeName(oldMode);
                Com_Printf(15, "%i bits of %s\n", bitsUsed - bitsUsedPrev, PacketDataTypeName);
            }
        }
        //Profile_EndInternal(0);
    }
}

const char *__cdecl SV_GetPacketDataTypeName(int dataType)
{
    const char *result; // eax
    const char *v2; // eax

    switch (dataType)
    {
    case 0:
        result = "Uncategorized data";
        break;
    case 1:
        result = "Header data";
        break;
    case 2:
        result = "Overhead";
        break;
    case 3:
        result = "Generic game data";
        break;
    case 4:
        result = "Reliable server commands";
        break;
    case 5:
        result = "Zero-float data";
        break;
    case 6:
        result = "Small float data";
        break;
    case 7:
        result = "Large float data";
        break;
    case 8:
        result = "Zero value data";
        break;
    case 9:
        result = "Small angle data";
        break;
    case 10:
        result = "Zero value angle data";
        break;
    case 11:
        result = "Time delta";
        break;
    case 12:
        result = "Time (32 bits)";
        break;
    case 13:
        result = "Flag index (5 bits)";
        break;
    case 14:
        result = "Ground entity";
        break;
    case 15:
        result = "Entity number";
        break;
    case 16:
        result = "Last field changed";
        break;
    case 18:
        result = "Origin delta";
        break;
    case 19:
        result = "Full origin";
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("Missing text for packet data type %i\n", dataType);
            MyAssertHandler(".\\server_mp\\sv_snapshot_profile_mp.cpp", 303, 0, v2);
        }
        result = "";
        break;
    }
    return result;
}

void __cdecl SV_PacketDataIsEntityNum(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_ENTITYNUM);
}

void __cdecl SV_PacketDataIsLastFieldChanged(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_LASTFIELDCHANGED);
}

void __cdecl SV_PacketDataIsUnknown(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_FIRST);
}

void __cdecl SV_PacketDataIsHeader(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_HEADER);
}

void __cdecl SV_PacketDataIsNotNetworkData(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_NOTNETWORKDATA);
}

void __cdecl SV_PacketDataIsOverhead(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_OVERHEAD);
}

void __cdecl SV_PacketDataIs24BitFlagIndex(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_24BITFLAGINDEX);
}

void __cdecl SV_PacketDataIsTimeDelta(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_TIMEDELTA);
}

void __cdecl SV_PacketDataIsTime(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_TIME);
}

void __cdecl SV_PacketDataIsReliableData(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_RELIABLEDATA);
}

void __cdecl SV_PacketDataIsData(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_DATA);
}

void __cdecl SV_PacketDataIsZeroFloat(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_ZEROFLOAT);
}

void __cdecl SV_PacketDataIsSmallFloat(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_SMALLFLOAT);
}

void __cdecl SV_PacketDataIsLargeFloat(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_LARGEFLOAT);
}

void __cdecl SV_PacketDataIsOriginDelta(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_ORIGINDELTA);
}

void __cdecl SV_PacketDataIsOrigin(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_ORIGIN);
}

void __cdecl SV_PacketDataIsZeroAngle(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_ZEROANGLE);
}

void __cdecl SV_PacketDataIsSmallAngle(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_SMALLANGLE);
}

void __cdecl SV_PacketDataIsZeroInt(int clientNum, const msg_t *msg)
{
    SV_PacketDataIsType(clientNum, msg, PACKETDATA_ZEROINT);
}

void __cdecl SV_TrackFloatCompressedBits(unsigned int bits)
{
    if (bits >= 0x3C)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            502,
            0,
            "bits doesn't index MAX_COMPRESSED_FLOAT_BITS\n\t%i not in [0, %i)",
            bits,
            60);
    ++s_floatBitsCompressed[bits];
}

void __cdecl SV_TrackOriginDeltaBits(int bits)
{
    if (bits > 7)
        MyAssertHandler(".\\server_mp\\sv_snapshot_profile_mp.cpp", 509, 0, "%s\n\t(bits) = %i", "(bits <= 7)", bits);
    ++s_originDeltaBits[bits];
}

void __cdecl SV_TrackOriginZDeltaBits(int bits)
{
    if (bits > 7)
        MyAssertHandler(".\\server_mp\\sv_snapshot_profile_mp.cpp", 516, 0, "%s\n\t(bits) = %i", "(bits <= 7)", bits);
    ++s_originZDeltaBits[bits];
}

void __cdecl SV_TrackOriginZFullBits(int bits)
{
    if (bits > 16)
        MyAssertHandler(".\\server_mp\\sv_snapshot_profile_mp.cpp", 523, 0, "%s\n\t(bits) = %i", "(bits <= 16)", bits);
    ++s_originZFullBits[bits];
}

void __cdecl SV_TrackOriginFullBits(int bits)
{
    if (bits > 16)
        MyAssertHandler(".\\server_mp\\sv_snapshot_profile_mp.cpp", 530, 0, "%s\n\t(bits) = %i", "(bits <= 16)", bits);
    ++s_originFullBits[bits];
}

const char *__cdecl SV_GetEntityTypeString(unsigned int packetEntityType)
{
    if (packetEntityType >= 0x17)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            537,
            0,
            "packetEntityType doesn't index ANALYZE_DATATYPE_ENTITYTYPE_COUNT\n\t%i not in [0, %i)",
            packetEntityType,
            23);
    return s_analyzeEntityTypeNames[packetEntityType];
}

int largestSize;
float s_avgSnapshotSize;
void __cdecl SV_AnalyzePacketData(int clientNum, const msg_t *msg)
{
    const char *PacketDataTypeName; // eax
    DWORD v3; // eax
    const char *EntityTypeString; // eax
    int cursize; // [esp-8h] [ebp-2Ch]
    int v6; // [esp-4h] [ebp-28h]
    int v7; // [esp+0h] [ebp-24h]
    double v8; // [esp+0h] [ebp-24h]
    int v9; // [esp+4h] [ebp-20h]
    int bitsUsed; // [esp+Ch] [ebp-18h]
    int field; // [esp+10h] [ebp-14h]
    int dataType; // [esp+14h] [ebp-10h]
    signed int entityType; // [esp+18h] [ebp-Ch]
    int packetType; // [esp+1Ch] [ebp-8h]
    const char *type; // [esp+20h] [ebp-4h]

    if (sv_debugPacketContents->current.enabled
        || msg->cursize > largestSize && svs.clients[clientNum].ping < 999 && svs.clients[clientNum].header.state == 4)
    {
        largestSize = msg->cursize;
        Com_Printf(15, "Client %s's snapshot\n", svs.clients[clientNum].name);
        bitsUsed = MSG_GetUsedBitCount(msg);
        for (packetType = 0; packetType < 20; ++packetType)
        {
            if (s_packetMetaDataSize[clientNum][packetType])
            {
                if (packetType != 17)
                {
                    v9 = 100 * s_packetMetaDataSize[clientNum][packetType] / bitsUsed;
                    v7 = s_packetMetaDataSize[clientNum][packetType] / 8;
                    v6 = s_packetMetaDataSize[clientNum][packetType];
                    PacketDataTypeName = SV_GetPacketDataTypeName(packetType);
                    Com_Printf(15, "%30s: %9i (%9i bytes) - %3i%%\n", PacketDataTypeName, v6, v7, v9);
                }
            }
        }
        type = "";
        Com_Printf(
            15,
            "begin---------------------------------------------------------------------------------------------\n");
        v8 = s_avgSnapshotSize;
        cursize = msg->cursize;
        v3 = Sys_Milliseconds();
        Com_Printf(
            15,
            "%i] %i byte snapshot for %s (average snapshot size is %f)\n",
            v3,
            cursize,
            svs.clients[clientNum].name,
            v8);
        for (dataType = 0; dataType < 13; ++dataType)
        {
            switch (dataType)
            {
            case 0:
                type = "Entity deltas";
                Com_Printf(15, "%s - %i bits\n", "Entity deltas", g_bitsSent[clientNum][dataType]);
                break;
            case 1:
                type = "New entities";
                Com_Printf(15, "%s - %i bits\n", "New entities", g_bitsSent[clientNum][dataType]);
                break;
            case 2:
                type = "Removed entities";
                Com_Printf(15, "%s - %i bits\n", "Removed entities", g_bitsSent[clientNum][dataType]);
                break;
            case 3:
            case 5:
            case 9:
                continue;
            case 4:
                type = "Temp entity";
                Com_Printf(15, "%s - %i bits\n", "Temp entity", g_bitsSent[clientNum][dataType]);
                break;
            case 6:
                type = "Client deltas";
                Com_Printf(15, "%s - %i bits\n", "Client deltas", g_bitsSent[clientNum][dataType]);
                break;
            case 7:
                type = "New clients";
                Com_Printf(15, "%s - %i bits\n", "New clients", g_bitsSent[clientNum][dataType]);
                break;
            case 8:
                type = "Removed clients";
                Com_Printf(15, "%s - %i bits\n", "Removed clients", g_bitsSent[clientNum][dataType]);
                break;
            case 10:
                type = "Playerstate deltas";
                Com_Printf(15, "%s - %i bits\n", "Playerstate deltas", g_bitsSent[clientNum][dataType]);
                break;
            case 11:
                type = "Playerstate nodelta";
                Com_Printf(15, "%s - %i bits\n", "Playerstate nodelta", g_bitsSent[clientNum][dataType]);
                break;
            case 12:
                type = "Server commands";
                Com_Printf(15, "%s - %i bits\n", "Server commands", g_bitsSent[clientNum][dataType]);
                break;
            default:
                if (!alwaysfails)
                    MyAssertHandler(
                        ".\\server_mp\\sv_snapshot_profile_mp.cpp",
                        613,
                        0,
                        "Unknown field in the snapshot analysis in SV_AnalyzePacketData()!\n");
                Com_Printf(15, "%s - %i bits\n", type, g_bitsSent[clientNum][dataType]);
                break;
            }
        }
        for (entityType = 0; entityType < 23; ++entityType)
        {
            EntityTypeString = SV_GetEntityTypeString(entityType);
            Com_Printf(15, "%s fields changed: (format fieldnum: timeschanged)\n", EntityTypeString);
            if (sv_debugPacketContents->current.enabled)
            {
                for (field = 0; field < 160; ++field)
                {
                    if (currentSnapshotNetworkEntityFieldsChanged[entityType][field])
                        Com_Printf(15, "%i: %u\n", field, currentSnapshotNetworkEntityFieldsChanged[entityType][field]);
                }
            }
        }
        Com_Printf(
            15,
            "end-----------------------------------------------------------------------------------------------\n");
    }
    memset(s_packetMetaDataSize[clientNum], 0, sizeof(int[20]));
}

int __cdecl SV_TrackPacketData(
    unsigned int clientNum,
    PacketDataType datatype,
    int eType,
    int entNum,
    int bitsUsedPrev,
    const msg_t *msg)
{
    int bitsUsedNow; // [esp+34h] [ebp-4h]

    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            644,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    if (datatype > ANALYZE_SNAPSHOT_SERVERCMDS)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            645,
            0,
            "%s\n\t(datatype) = %i",
            "(datatype >= 0 && datatype < ANALYZE_SNAPSHOT_DATATYPE_COUNT)",
            datatype);
    //Profile_Begin(297);
    bitsUsedNow = MSG_GetUsedBitCount(msg);
    g_bitsSent[clientNum][datatype] += bitsUsedNow - bitsUsedPrev;
    g_currentSnapshotPerEntity[clientNum][entNum] += bitsUsedNow - bitsUsedPrev;
    newDataReady = 1;
    if (datatype == ANALYZE_SNAPSHOT_SERVERCMDS)
        bitsUsedForServerCommands += bitsUsedNow - bitsUsedPrev;
    //Profile_EndInternal(0);
    return bitsUsedNow;
}

void __cdecl SV_SetNextEntityStart(int eType, int entNum)
{
    s_currentEntType = eType;
    s_currentEntNum = entNum;
}

bool __cdecl SV_NewPacketAnalysisReady()
{
    return newDataReady;
}

void __cdecl SV_TrackFieldChange(int clientNum, int entityType, unsigned int field)
{
    const char *string; // [esp+30h] [ebp-4h]

    if (field >= 0xA0)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            684,
            0,
            "%s\n\t(field) = %i",
            "(field >= 0 && field < 160)",
            field);
    if (entityType > 0x16)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            685,
            0,
            "%s\n\t(entityType) = %i",
            "(entityType >= 0 && entityType < ANALYZE_DATATYPE_ENTITYTYPE_COUNT)",
            entityType);
    if (s_packetDataEnabled)
    {
        //Profile_Begin(296);
        if (entityType >= 18)
        {
            if (entityType == 20)
                ++g_currentSnapshotPlayerStateFields[clientNum];
        }
        else
        {
            if (s_currentEntType < 0)
                MyAssertHandler(
                    ".\\server_mp\\sv_snapshot_profile_mp.cpp",
                    699,
                    0,
                    "%s\n\t(s_currentEntType) = %i",
                    "(s_currentEntType >= 0)",
                    s_currentEntType);
            if (s_currentEntNum >= 0x400)
                MyAssertHandler(
                    ".\\server_mp\\sv_snapshot_profile_mp.cpp",
                    700,
                    0,
                    "%s\n\t(s_currentEntNum) = %i",
                    "(s_currentEntNum >= 0 && s_currentEntNum < (1<<10))",
                    s_currentEntNum);
            ++g_currentSnapshotFieldsPerEntity[clientNum][s_currentEntNum];
        }
        if (sv_debugPacketContents->current.enabled)
        {
            string = SV_GetEntityTypeString(entityType);
            if (*string)
                Com_Printf(15, "%s - field %i changed\n", string, field);
        }
        //Profile_EndInternal(0);
    }
    else
    {
        ++networkEntityFieldsChanged[entityType][field];
        ++currentSnapshotNetworkEntityFieldsChanged[entityType][field];
    }
}

void __cdecl SV_WriteEntityFieldNumbers()
{
    char *EntityTypeName; // eax
    __int64 v1; // [esp+4h] [ebp-34h]
    unsigned int numFields; // [esp+14h] [ebp-24h] BYREF
    bool estimate; // [esp+1Bh] [ebp-1Dh] BYREF
    int totalData; // [esp+1Ch] [ebp-1Ch]
    NetFieldList stateFields; // [esp+20h] [ebp-18h] BYREF
    const char *entityTypeString; // [esp+28h] [ebp-10h]
    int f; // [esp+2Ch] [ebp-Ch]
    int entity; // [esp+30h] [ebp-8h]
    unsigned int i; // [esp+34h] [ebp-4h]

    f = FS_FOpenFileWrite((char*)"mp_entityStats.txt");
    if (f)
    {
        totalData = bitsUsedForServerCommands + bitsUsedForPlayerstates[0];
        for (i = 0; i < 0x97; ++i)
            totalData += bitsUsedPerEType[i];
        if (!totalData)
            totalData = 1;
        FS_Printf(f, "Total data sent: %i\n", totalData);
        FS_Printf(f, "Bits used per entity type: (format: bitsUsed - entityType)\n");
        for (i = 0; i < 0x97; ++i)
        {
            if (bitsUsedPerEType[i])
            {
                v1 = 100LL * bitsUsedPerEType[i] / totalData;
                EntityTypeName = BG_GetEntityTypeName(i);
                FS_Printf(f, "%18u - %s (%i%%)\n", bitsUsedPerEType[i], EntityTypeName, v1);
            }
        }
        FS_Printf(
            f,
            "%18u - Server commands (%i%%)\n",
            bitsUsedForServerCommands,
            (100LL * bitsUsedForServerCommands / totalData));
        FS_Printf(
            f,
            "%18u - Player states (%i%%)\n",
            bitsUsedForPlayerstates[0],
            (100LL * bitsUsedForPlayerstates[0] / totalData));
        FS_Printf(
            f,
            "  * %18u - Player state field deltas (%i%%)\n",
            bitsUsedForPlayerstates[1],
            (100LL * bitsUsedForPlayerstates[1] / totalData));
        FS_Printf(
            f,
            "  * %18u - Player state stats (%i%%)\n",
            bitsUsedForPlayerstates[2],
            (100LL * bitsUsedForPlayerstates[2] / totalData));
        FS_Printf(
            f,
            "  * %18u - Player state ammo (%i%%)\n",
            bitsUsedForPlayerstates[3],
            (100LL * bitsUsedForPlayerstates[3] / totalData));
        FS_Printf(
            f,
            "  * %18u - Player state objectives (%i%%)\n",
            bitsUsedForPlayerstates[4],
            (100LL * bitsUsedForPlayerstates[4] / totalData));
        FS_Printf(
            f,
            "  * %18u - Player state hudelems (%i%%)\n",
            bitsUsedForPlayerstates[5],
            (100LL * bitsUsedForPlayerstates[5] / totalData));
        FS_Printf(f, "\n\n");
        FS_Printf(f, "PS origin and velocity sent due to client prediction error: %i\n", originsSentDueToPredicitonError);
        FS_Printf(f, "PS origin and velocity sent due to server time mismatch: %i\n", originsSentDueToServerTimeMismatch);
        FS_Printf(f, "\n\n");
        FS_Printf(
            f,
            "Avg Snapshot size: %i bytes, std deviation: %i bytes, max snapshot size: %i bytes\n",
            s_avgSnapshotSize,
            s_stdSnapshotDeviation,
            s_maxSnapshotSize);
        for (entity = 0; entity < 23; ++entity)
        {
            entityTypeString = SV_GetEntityTypeString(entity);
            FS_Printf(f, "%s fields changed: (format fieldnum: timeschanged)\n", entityTypeString);
            SV_GetAnalyzeEntityFields(entity, &stateFields, &numFields);
            totalData = 0;
            for (i = 0; i < numFields; ++i)
            {
                stateFields.count = MSG_GetBitCount(stateFields.array[i].bits, &estimate, 0, -1);
                totalData += stateFields.count * networkEntityFieldsChanged[entity][i];
            }
            if (!totalData)
                totalData = 1;
            for (i = 0; i < numFields; ++i)
            {
                stateFields.count = MSG_GetBitCount(stateFields.array[i].bits, &estimate, 0, -1);
                FS_Printf(f, "%i (%s): %u", i, stateFields.array[i].name, networkEntityFieldsChanged[entity][i]);
                if (networkEntityFieldsChanged[entity][i])
                {
                    if (estimate)
                        FS_Printf(
                            f,
                            " (%i%%%s)",
                            networkEntityFieldsChanged[entity][i] * 100 * stateFields.count / totalData,
                            " [estimated]");
                    else
                        FS_Printf(
                            f,
                            " (%i%%%s)",
                            networkEntityFieldsChanged[entity][i] * 100 * stateFields.count / totalData,
                            "");
                    if (stateFields.array[i].changeHints == 1 && i)
                        FS_Printf(f, " (marked as NEVER_CHANGES and it changed!)");
                }
                FS_Printf(f, "\n");
            }
            FS_Printf(f, "\n\n");
        }
        FS_Printf(f, "\n\nPacket data types:\n");
        totalData = 0;
        for (i = 0; i < 0x14; ++i)
            totalData += s_totalPacketDataSizes[i];
        for (i = 0; i < 0x14; ++i)
        {
            if (s_totalPacketDataSizes[i])
                FS_Printf(
                    f,
                    "%18i bits (%.1f%%): %s\n",
                    s_totalPacketDataSizes[i],
                    s_totalPacketDataSizes[i] / totalData,
                    packetModeNames[i]);
        }
        FS_Printf(f, "\n\nPost-Huffman Full Float bits:\n");
        for (i = 0; i < 0x3C; ++i)
        {
            if (s_floatBitsCompressed[i])
                FS_Printf(f, "%i bits: %i times\n", i, s_floatBitsCompressed[i]);
        }
        FS_Printf(f, "\n\nX/Y Origin Deltas:\n");
        for (i = 0; i < 8; ++i)
        {
            if (s_originDeltaBits[i])
                FS_Printf(f, "%i bits: %i times\n", i, s_originDeltaBits[i]);
        }
        FS_Printf(f, "\n\nX/Y Origin Full Sends:\n");
        for (i = 0; i < 0x11; ++i)
        {
            if (s_originFullBits[i])
                FS_Printf(f, "sent as full %i bits, but number only needed %i bits: %i times\n", 16, i, s_originFullBits[i]);
        }
        FS_Printf(f, "\n\nZ Origin Deltas:\n");
        for (i = 0; i < 8; ++i)
        {
            if (s_originZDeltaBits[i])
                FS_Printf(f, "%i bits: %i times\n", i, s_originZDeltaBits[i]);
        }
        FS_Printf(f, "\n\nZ Origin Full Sends:\n");
        for (i = 0; i < 0x11; ++i)
        {
            if (s_originZFullBits[i])
                FS_Printf(f, "sent as full %i bits, but number only needed %i bits: %i times\n", 16, i, s_originZFullBits[i]);
        }
        FS_Printf(f, "Last PS field changed in snapshot:\n");
        for (i = 0; i < 0xA0; ++i)
        {
            if (playerStateFieldsChanged[i])
                FS_Printf(f, "%10i times - field %i [%s]\n", playerStateFieldsChanged[i], i, playerStateFields[i].name);
        }
        FS_Printf(f, "\n\n");
        FS_Printf(f, "\n\nHuffman data:\n");
        for (i = 0; i < 0x100; ++i)
            FS_Printf(f, "\t%i,\t\t// %i\n", huffBytesSeen[i] >> 4, i);
        FS_FCloseFile(f);
    }
}

void __cdecl SV_GetAnalyzeEntityFields(int analyzeEntityType, NetFieldList *stateFields, unsigned int *numFields)
{
    if (analyzeEntityType > 17)
    {
        switch (analyzeEntityType)
        {
        case 18:
            stateFields->array = archivedEntityFields;
            *numFields = 128;
            break;
        case 19:
            stateFields->array = clientStateFields;
            *numFields = 32;
            break;
        case 20:
            stateFields->array = playerStateFields;
            *numFields = 160;
            break;
        case 21:
            stateFields->array = hudElemFields;
            *numFields = 40;
            break;
        default:
            if (analyzeEntityType != 22)
                MyAssertHandler(
                    ".\\server_mp\\sv_snapshot_profile_mp.cpp",
                    774,
                    0,
                    "%s",
                    "analyzeEntityType == ANALYZE_DATATYPE_ENTITYTYPE_BASELINE");
            stateFields->array = entityStateFields;
            *numFields = 64;
            break;
        }
    }
    else
    {
        *stateFields = *MSG_GetStateFieldListForEntityType(analyzeEntityType);
    }
}

int __cdecl SV_GetClientSnapshotPing(int clientNum, char snapshotNum)
{
    int start; // [esp+8h] [ebp-4h]

    start = (snapshotNum + svs.clients[clientNum].header.netchan.outgoingSequence) & 0x1F;
    if (svs.clients[clientNum].frames[start].messageAcked < 0)
        return -1;
    else
        return svs.clients[clientNum].frames[start].messageAcked - svs.clients[clientNum].frames[start].messageSent;
}

void __cdecl SV_TrackSnapshotSize(int size)
{
    if (size > s_maxSnapshotSize)
        s_maxSnapshotSize = size;
    s_stdSnapshotDeviation = (s_numSnapshotSamples * s_stdSnapshotDeviation + size - s_avgSnapshotSize)
        / (s_numSnapshotSamples + 1);
    s_avgSnapshotSize = (s_numSnapshotSamples * s_avgSnapshotSize + size) / (s_numSnapshotSamples + 1);
    ++s_numSnapshotSamples;
    ++s_numSnapshotsBuiltSinceLastPoll;
    s_uncompressedDataSinceLastPoll += size;
}

void __cdecl SV_TrackPacketCompression(unsigned int clientNum, int originalSize, int compressedSize)
{
    int slot; // [esp+0h] [ebp-4h]

    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_snapshot_profile_mp.cpp",
            1030,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    slot = s_clientSnapshotData[clientNum].index & 7;
    s_clientSnapshotData[clientNum].compressedSize[slot] = compressedSize;
    s_clientSnapshotData[clientNum].snapshotSize[slot] = originalSize;
    ++s_clientSnapshotData[clientNum].index;
    ++s_numSnapshotsSentSinceLastPoll;
    s_compressedDataSinceLastPoll += compressedSize;
}

int __cdecl SV_GetPacketCompressionForClient(int clientNum)
{
    int slot; // [esp+0h] [ebp-Ch]
    int avgCompressedSize; // [esp+4h] [ebp-8h]
    int avgSize; // [esp+8h] [ebp-4h]

    avgSize = 0;
    avgCompressedSize = 0;
    for (slot = 0; slot < 8; ++slot)
    {
        avgSize += s_clientSnapshotData[clientNum].snapshotSize[slot];
        avgCompressedSize += s_clientSnapshotData[clientNum].compressedSize[slot];
    }
    if (avgSize)
        return 100 - 100 * avgCompressedSize / avgSize;
    else
        return 0;
}

void __cdecl SV_Netchan_PrintProfileStats(int bPrintToConsole)
{
    int iTotalMinRecieved; // [esp+4h] [ebp-49Ch]
    int packet; // [esp+8h] [ebp-498h]
    int packeta; // [esp+8h] [ebp-498h]
    int compress; // [esp+Ch] [ebp-494h]
    char szLine[1028]; // [esp+10h] [ebp-490h] BYREF
    int iYPos; // [esp+414h] [ebp-8Ch]
    int totalPacketsSent; // [esp+418h] [ebp-88h]
    int iFragmentTotal; // [esp+41Ch] [ebp-84h]
    int totalAcked; // [esp+420h] [ebp-80h]
    char szClientName[32]; // [esp+424h] [ebp-7Ch] BYREF
    int iTotalBPSRecieved; // [esp+448h] [ebp-58h]
    int iTotalPacketsRecieved; // [esp+44Ch] [ebp-54h]
    int totalUnacked; // [esp+450h] [ebp-50h]
    int iTotalBPSSent; // [esp+454h] [ebp-4Ch]
    int iTotalMaxRecieved; // [esp+458h] [ebp-48h]
    int now; // [esp+45Ch] [ebp-44h]
    int iTotalMinSent; // [esp+460h] [ebp-40h]
    client_t *pClient; // [esp+464h] [ebp-3Ch]
    int iTotalFragmentsSent; // [esp+468h] [ebp-38h]
    int packetsSent; // [esp+46Ch] [ebp-34h]
    int iTotalPacketsSent; // [esp+470h] [ebp-30h]
    int iYStep; // [esp+474h] [ebp-2Ch]
    int iTotalMaxSent; // [esp+478h] [ebp-28h]
    clientSnapshot_t *snap; // [esp+47Ch] [ebp-24h]
    int i; // [esp+480h] [ebp-20h]
    int unacked; // [esp+484h] [ebp-1Ch]
    int packetStart; // [esp+488h] [ebp-18h]
    int totalOOBPackets; // [esp+48Ch] [ebp-14h]
    int iTotalFragmentsRecieved; // [esp+490h] [ebp-10h]
    int lastTime; // [esp+494h] [ebp-Ch]
    netProfileInfo_t *pStream; // [esp+498h] [ebp-8h]
    int dropPercent; // [esp+49Ch] [ebp-4h]

    iTotalBPSSent = 0;
    iTotalBPSRecieved = 0;
    iTotalPacketsSent = 0;
    iTotalFragmentsSent = 0;
    iTotalPacketsRecieved = 0;
    iTotalFragmentsRecieved = 0;
    iTotalMaxSent = 0;
    iTotalMinSent = 9999;
    iTotalMaxRecieved = 0;
    iTotalMinRecieved = 9999;
    iYPos = cl_profileTextY->current.integer;
    iYStep = 10;
    if (!net_profile->current.integer)
        MyAssertHandler(".\\server_mp\\sv_snapshot_profile_mp.cpp", 1185, 0, "%s", "net_profile->current.integer");
    SV_Netchan_UpdateProfileStats();
    if (bPrintToConsole)
        Com_Printf(15, "\n\n");
    Com_sprintf(szLine, 0x400u, "====================");
    if (bPrintToConsole)
        Com_Printf(15, "%s\n", szLine);
    Com_sprintf(szLine, 0x400u, "Server Network Profile:");
    if (bPrintToConsole)
        Com_Printf(15, "%s\n\n", szLine);
    Com_sprintf(szLine, 0x400u, "                    | Sent To                                   | From |");
    if (bPrintToConsole)
    {
        Com_Printf(15, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        SV_ProfDraw(iYPos, szLine, 0);
    }
    Com_sprintf(szLine, 0x400u, "              Source|   bps|  max|  min|frag%%|drop%%|ak|huff%%|p/s|   bps|");
    if (bPrintToConsole)
    {
        Com_Printf(15, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        SV_ProfDraw(iYPos, szLine, 0);
    }
    now = Sys_Milliseconds();
    pStream = &svs.OOBProf;
    iTotalBPSSent += svs.OOBProf.send.iBytesPerSecond;
    iTotalPacketsSent += svs.OOBProf.send.iCountedPackets;
    iTotalFragmentsSent += svs.OOBProf.send.iCountedFragments;
    iTotalBPSRecieved += svs.OOBProf.recieve.iBytesPerSecond;
    iTotalPacketsRecieved += svs.OOBProf.recieve.iCountedPackets;
    iTotalFragmentsRecieved += svs.OOBProf.recieve.iCountedFragments;
    if (svs.OOBProf.send.iLargestPacket > iTotalMaxSent)
        iTotalMaxSent = pStream->send.iLargestPacket;
    if (pStream->send.iSmallestPacket < iTotalMinSent)
        iTotalMinSent = pStream->send.iSmallestPacket;
    if (pStream->recieve.iLargestPacket > iTotalMaxRecieved)
        iTotalMaxRecieved = pStream->recieve.iLargestPacket;
    if (pStream->recieve.iSmallestPacket < 9999)
        iTotalMinRecieved = pStream->recieve.iSmallestPacket;
    i = 0;
    pClient = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (pClient->header.state)
        {
            pStream = &pClient->header.netchan.prof;
            if (pClient->header.netchan.remoteAddress.type != NA_LOOPBACK)
            {
                iTotalBPSSent += pStream->send.iBytesPerSecond;
                iTotalPacketsSent += pStream->send.iCountedPackets;
                iTotalFragmentsSent += pStream->send.iCountedFragments;
                iTotalBPSRecieved += pStream->recieve.iBytesPerSecond;
                iTotalPacketsRecieved += pStream->recieve.iCountedPackets;
                iTotalFragmentsRecieved += pStream->recieve.iCountedFragments;
                if (pStream->send.iLargestPacket > iTotalMaxSent)
                    iTotalMaxSent = pStream->send.iLargestPacket;
                if (pStream->send.iSmallestPacket < iTotalMinSent)
                    iTotalMinSent = pStream->send.iSmallestPacket;
                if (pStream->recieve.iLargestPacket > iTotalMaxRecieved)
                    iTotalMaxRecieved = pStream->recieve.iLargestPacket;
                if (pStream->recieve.iSmallestPacket < iTotalMinRecieved)
                    iTotalMinRecieved = pStream->recieve.iSmallestPacket;
            }
        }
        ++i;
        ++pClient;
    }
    if (iTotalPacketsRecieved + iTotalPacketsSent <= 0 || iTotalFragmentsRecieved + iTotalFragmentsSent <= 0)
        iFragmentTotal = 0;
    else
        iFragmentTotal = 100 * (iTotalFragmentsRecieved + iTotalFragmentsSent) / (iTotalPacketsRecieved + iTotalPacketsSent);
    dropPercent = 0;
    unacked = 0;
    totalPacketsSent = 0;
    if (iTotalPacketsSent)
        Com_sprintf(
            szLine,
            0x400u,
            "              Totals:%6i|%5i|%5i| %3i%%| %3i%%|%2i|%4i%%|%3i|%6i|",
            iTotalBPSSent,
            iTotalMaxSent,
            iTotalMinSent,
            100 * iTotalFragmentsSent / iTotalPacketsSent,
            dropPercent,
            unacked,
            0,
            totalPacketsSent,
            iTotalBPSRecieved);
    else
        Com_sprintf(
            szLine,
            0x400u,
            "              Totals:%6i|%5i|%5i| %3i%%| %3i%%|%2i|%4i%%|%3i|%6i|",
            iTotalBPSSent,
            iTotalMaxSent,
            iTotalMinSent,
            0,
            dropPercent,
            unacked,
            0,
            totalPacketsSent,
            iTotalBPSRecieved);
    if (bPrintToConsole)
    {
        Com_Printf(15, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        SV_ProfDraw(iYPos, szLine, 0);
    }
    pStream = &svs.OOBProf;
    if (svs.OOBProf.recieve.iCountedPackets + svs.OOBProf.send.iCountedPackets <= 0
        || pStream->recieve.iCountedFragments + pStream->send.iCountedFragments <= 0)
    {
        iFragmentTotal = 0;
    }
    else
    {
        iFragmentTotal = 100
            * (pStream->recieve.iCountedFragments + pStream->send.iCountedFragments)
            / (pStream->recieve.iCountedPackets + pStream->send.iCountedPackets);
    }
    dropPercent = 0;
    unacked = 0;
    totalOOBPackets = 0;
    for (packet = 0; packet < 60; ++packet)
    {
        if (now - pStream->send.packets[packet].iTime < 1000)
            ++totalOOBPackets;
    }
    Com_sprintf(
        szLine,
        0x400u,
        "  OutOfBand Messages: %5i|%5i|%5i| %3i%%| %3i%%|%2i|%4i%%|%3i| %5i|",
        pStream->send.iBytesPerSecond,
        pStream->send.iLargestPacket,
        pStream->send.iSmallestPacket,
        pStream->send.iFragmentPercentage,
        dropPercent,
        unacked,
        0,
        totalOOBPackets,
        pStream->recieve.iBytesPerSecond);
    if (bPrintToConsole)
    {
        Com_Printf(15, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        SV_ProfDraw(iYPos, szLine, 0);
    }
    i = 0;
    pClient = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (pClient->header.state)
        {
            strncpy(szClientName, pClient->name, 0x11u);
            szClientName[16] = 0;
            pStream = &pClient->header.netchan.prof;
            if (pClient->header.netchan.prof.recieve.iCountedPackets + pClient->header.netchan.prof.send.iCountedPackets <= 0
                || pStream->recieve.iCountedFragments + pStream->send.iCountedFragments <= 0)
            {
                iFragmentTotal = 0;
            }
            else
            {
                iFragmentTotal = 100
                    * (pStream->recieve.iCountedFragments + pStream->send.iCountedFragments)
                    / (pStream->recieve.iCountedPackets + pStream->send.iCountedPackets);
            }
            lastTime = 0;
            for (packetStart = 0; packetStart < 32 && pClient->frames[packetStart].messageSent > lastTime; ++packetStart)
                lastTime = pClient->frames[packetStart].messageSent;
            totalAcked = 0;
            totalUnacked = 0;
            unacked = 0;
            packetsSent = 0;
            for (packeta = 0; packeta < 32; ++packeta)
            {
                snap = &pClient->frames[(packetStart + packeta) & 0x1F];
                if (pClient->frames[(packetStart + packeta) & 0x1F].messageSent > now - 1000)
                    ++packetsSent;
                if (snap->messageAcked > 0)
                {
                    totalUnacked += unacked;
                    unacked = 0;
                    ++totalAcked;
                }
                else
                {
                    ++unacked;
                }
            }
            if (totalAcked)
                dropPercent = 100 * totalUnacked / (totalAcked + totalUnacked);
            else
                dropPercent = 100;
            compress = SV_GetPacketCompressionForClient(i);
            Com_sprintf(
                szLine,
                0x400u,
                "#%2i-%16s: %5i|%5i|%5i| %3i%%| %3i%%|%2i|%4i%%|%3i| %5i|",
                i,
                szClientName,
                pStream->send.iBytesPerSecond,
                pStream->send.iLargestPacket,
                pStream->send.iSmallestPacket,
                pStream->send.iFragmentPercentage,
                dropPercent,
                unacked,
                compress,
                packetsSent,
                pStream->recieve.iBytesPerSecond);
            if (bPrintToConsole)
            {
                Com_Printf(15, "%s\n", szLine);
            }
            else
            {
                iYPos += 10;
                SV_ProfDraw(iYPos, szLine, i == cg_packetAnalysisClient->current.integer);
            }
        }
        ++i;
        ++pClient;
    }
}

void __cdecl SV_ProfDraw(int y, char *string, bool showHighlight)
{
    float color[4]; // [esp+Ch] [ebp-10h] BYREF

    if (showHighlight)
    {
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        color[3] = 0.2;
        CL_DrawRect(12, y, 1024, cl_profileTextHeight->current.integer - 6, color);
    }
    CL_DrawString(12, y, string, 0, cl_profileTextHeight->current.integer);
}

