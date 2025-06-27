#pragma once

#include "net_chan_mp.h"
#include "huffman.h"

static const unsigned int kbitmask[33] =
{
    0,
    1,
    3,
    7,
    0x0F,
    0x1F,
    0x3F,
    0x7F,
    0x0FF,
    0x1FF,
    0x3FF,
    0x7FF,
    0x0FFF,
    0x1FFF,
    0x3FFF,
    0x7FFF,
    0x0FFFF,
    0x1FFFF,
    0x3FFFF,
    0x7FFFF,
    0x0FFFFF,
    0x1FFFFF,
    0x3FFFFF,
    0x7FFFFF,
    0x0FFFFFF,
    0x1FFFFFF,
    0x3FFFFFF,
    0x7FFFFFF,
    0x0FFFFFFF,
    0x1FFFFFFF,
    0x3FFFFFFF,
    0x7FFFFFFF,
    0x0FFFFFFFF,
};

enum PacketEntityType : __int32
{                                       // ...
    ANALYZE_DATATYPE_ENTITYTYPE_GENERALENTITY = 0x0,
    ANALYZE_DATATYPE_ENTITYTYPE_PLAYERENTITY = 0x1,
    ANALYZE_DATATYPE_ENTITYTYPE_PLAYERCORPSEENTITY = 0x2,
    ANALYZE_DATATYPE_ENTITYTYPE_ITEMENTITY = 0x3,
    ANALYZE_DATATYPE_ENTITYTYPE_MISSILEENTITY = 0x4,
    ANALYZE_DATATYPE_ENTITYTYPE_INVISIBLEENTITY = 0x5,
    ANALYZE_DATATYPE_ENTITYTYPE_SCRIPTMOVERENTITY = 0x6,
    ANALYZE_DATATYPE_ENTITYTYPE_SOUNDBLENDENTITY = 0x7,
    ANALYZE_DATATYPE_ENTITYTYPE_FXENTITY = 0x8,
    ANALYZE_DATATYPE_ENTITYTYPE_LOOPFXENTITY = 0x9,
    ANALYZE_DATATYPE_ENTITYTYPE_PRIMARYLIGHTENTITY = 0xA,
    ANALYZE_DATATYPE_ENTITYTYPE_MG42ENTITY = 0xB,
    ANALYZE_DATATYPE_ENTITYTYPE_HELICOPTER = 0xC,
    ANALYZE_DATATYPE_ENTITYTYPE_PLANE = 0xD,
    ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE = 0xE,
    ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_COLLMAP = 0xF,
    ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_CORPSE = 0x10,
    ANALYZE_DATATYPE_ENTITYTYPE_TEMPENTITY = 0x11,
    ANALYZE_DATATYPE_ENTITYTYPE_ARCHIVEDENTITY = 0x12,
    ANALYZE_DATATYPE_ENTITYTYPE_CLIENTSTATE = 0x13,
    ANALYZE_DATATYPE_ENTITYTYPE_PLAYERSTATE = 0x14,
    ANALYZE_DATATYPE_ENTITYTYPE_HUDELEM = 0x15,
    ANALYZE_DATATYPE_ENTITYTYPE_BASELINE = 0x16,
    ANALYZE_DATATYPE_ENTITYTYPE_COUNT = 0x17,
};

struct netFieldOrderInfo_t // sizeof=0x6C0
{                                       // ...
    int entState[64];
    int arcEntState[128];               // ...
    int clientState[32];                // ...
    int playerState[160];               // ...
    int objective[8];                   // ...
    int hudElem[40];                    // ...
};

struct SnapshotInfo_s // sizeof=0x18
{                                       // ...
    int clientNum;                      // ...
    const clientHeader_t* client;       // ...
    int snapshotDeltaTime;              // ...
    bool fromBaseline;                  // ...
    bool archived;                      // ...
    // padding byte
    // padding byte
    int* fieldChanges;                  // ...
    PacketEntityType packetEntityType;  // ...
};


void __cdecl TRACK_msg();
const NetFieldList* __cdecl MSG_GetStateFieldListForEntityType(int eType);
void __cdecl MSG_WriteReliableCommandToBuffer(const char* pszCommand, char* pszBuffer, int iBufferSize);
void __cdecl MSG_WriteEntityIndex(SnapshotInfo_s *snapInfo, msg_t *msg, int index, int indexBits);
void __cdecl MSG_WriteOriginFloat(const int clientNum, msg_t *msg, int bits, double value, double oldValue);
void __cdecl MSG_WriteOriginZFloat(const int clientNum, msg_t *msg, double value, double oldValue);
bool __cdecl MSG_ValuesAreEqual(const SnapshotInfo_s* snapInfo, int bits, const int* fromF, const int* toF);
void __cdecl MSG_WriteLastChangedField(msg_t* msg, int lastChangedFieldNum, unsigned int numFields);
void __cdecl MSG_WriteEventNum(int clientNum, msg_t* msg, unsigned __int8 eventNum);
void __cdecl MSG_WriteEventParam(int clientNum, msg_t* msg, unsigned __int8 eventParam);
PacketEntityType __cdecl MSG_GetPacketEntityTypeForEType(int eType);
unsigned int __cdecl MSG_GetBitCount(int bits, bool* estimate, int from, int to);
void __cdecl MSG_WriteEntity(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    entityState_s* from,
    const entityState_s* to,
    int force);
void __cdecl MSG_WriteEntityRemoval(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    unsigned __int8* from,
    int indexBits,
    bool changeBit);
void __cdecl MSG_WriteEntityDeltaForEType(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    int eType,
    const entityState_s* from,
    const entityState_s* to,
    int force);
int __cdecl MSG_WriteEntityDelta(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    const unsigned __int8* from,
    const unsigned __int8* to,
    int force,
    int numFields,
    int indexBits,
    const NetField* stateFields);
void __cdecl MSG_WriteDeltaField(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    const unsigned __int8* from,
    const unsigned __int8* to,
    const NetField* field,
    int fieldNum,
    bool forceSend);
void __cdecl MSG_WriteDeltaTime(int clientNum, msg_t* msg, int timeBase, int time);
void __cdecl MSG_Write24BitFlag(int clientNum, msg_t* msg, int oldFlags, int newFlags);
void __cdecl MSG_WriteGroundEntityNum(int clientNum, msg_t* msg, int groundEntityNum);
bool __cdecl MSG_CheckWritingEnoughBits(int value, unsigned int bits);
void __cdecl MSG_WriteDeltaArchivedEntity(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    archivedEntity_s* from,
    archivedEntity_s* to,
    int force);
int __cdecl MSG_WriteDeltaStruct(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    unsigned __int8* from,
    unsigned __int8* to,
    int force,
    int numFields,
    int indexBits,
    const NetField* stateFields,
    int bChangeBit);
void __cdecl MSG_WriteDeltaClient(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    clientState_s* from,
    clientState_s* to,
    int force);
void __cdecl MSG_WriteDeltaPlayerstate(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    const playerState_s* from,
    const playerState_s* to);
bool __cdecl MSG_ShouldSendPSField(
    const SnapshotInfo_s* snapInfo,
    bool sendOriginAndVel,
    const playerState_s* ps,
    const playerState_s* oldPs,
    const NetField* field);
void __cdecl MSG_WriteDeltaFields(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    unsigned __int8* from,
    unsigned __int8* to,
    int force,
    int numFields,
    const NetField* stateFields);
void __cdecl MSG_WriteDeltaHudElems(
    SnapshotInfo_s* snapInfo,
    msg_t* msg,
    int time,
    const hudelem_s* from,
    const hudelem_s* to,
    unsigned int count);


extern huffman_t msgHuff;
extern netFieldOrderInfo_t orderInfo;