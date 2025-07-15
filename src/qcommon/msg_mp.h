#pragma once

#ifndef KISAK_MP
#error This File is for MultiPlayer Only
#endif

#include "qcommon.h"
#include "ent.h"

struct msg_t // sizeof=0x28
{                                       // ...
    int overflowed;                     // ...
    int readOnly;                       // ...
    unsigned __int8* data;              // ...
    unsigned __int8* splitData;         // ...
    int maxsize;                        // ...
    int cursize;                        // ...
    int splitSize;                      // ...
    int readcount;                      // ...
    int bit;                            // ...
    int lastEntityRef;                  // ...
};


struct NetField // sizeof=0x10
{                                       // ...
    const char* name;
    size_t offset;
    int bits;
    unsigned __int8 changeHints;
    // padding byte
    // padding byte
    // padding byte
};

struct usercmd_s // sizeof=0x20
{                                       // XREF: ?SV_BotUserMove@@YAXPAUclient_t@@@Z/r
    int32_t serverTime;                     // XREF: CG_DrawDisconnect+85/r
    int32_t buttons;                        // XREF: CG_CheckForPlayerInput+5D/r
    int32_t angles[3];                      // XREF: CG_CheckPlayerMovement+B/o
    uint8_t weapon;             // XREF: CL_CreateCmd+64/w
    uint8_t offHandIndex;
    char forwardmove;                   // XREF: CG_CheckPlayerMovement:loc_4413AE/r
    char rightmove;                     // XREF: CG_CheckPlayerMovement+26/r
    float meleeChargeYaw;               // XREF: CL_CreateCmd+67/w
    uint8_t meleeChargeDist;    // XREF: CL_CreateCmd+6A/w
    char selectedLocation[2];
};
static_assert(sizeof(usercmd_s) == 0x20);

struct hudelem_s;

struct clientState_s;
struct playerState_s;

int __cdecl GetMinBitCountForNum(unsigned int num);
void __cdecl MSG_Init(msg_t *buf, unsigned __int8 *data, int length);
void __cdecl MSG_InitReadOnly(msg_t *buf, unsigned __int8 *data, int length);
void __cdecl MSG_InitReadOnlySplit(msg_t *buf, unsigned __int8 *data, int length, unsigned __int8 *data2, int length2);
void __cdecl MSG_BeginReading(msg_t *msg);
void __cdecl MSG_Discard(msg_t *msg);
int __cdecl MSG_GetUsedBitCount(const msg_t *msg);
void __cdecl MSG_WriteBits(msg_t *msg, int value, unsigned int bits);
void __cdecl MSG_WriteBit0(msg_t *msg);
void __cdecl MSG_WriteBit1(msg_t *msg);
int __cdecl MSG_ReadBits(msg_t *msg, unsigned int bits);
int __cdecl MSG_GetByte(msg_t *msg, int where);
int __cdecl MSG_ReadBit(msg_t *msg);
int __cdecl MSG_WriteBitsCompress(bool trainHuffman, const unsigned __int8 *from, unsigned __int8 *to, int size);
int __cdecl MSG_ReadBitsCompress(const unsigned __int8 *from, unsigned __int8 *to, int size);
void __cdecl MSG_WriteByte(msg_t *msg, unsigned __int8 c);
void __cdecl MSG_WriteData(msg_t *buf, unsigned __int8 *data, unsigned int length);
void __cdecl MSG_WriteShort(msg_t *msg, __int16 c);
void __cdecl MSG_WriteLong(msg_t *msg, int c);
void __cdecl MSG_WriteString(msg_t *sb, const char *s);
void __cdecl MSG_WriteBigString(msg_t *sb, char *s);
void __cdecl MSG_WriteAngle16(msg_t *sb, float f);
int __cdecl MSG_ReadByte(msg_t *msg);
int __cdecl MSG_ReadShort(msg_t *msg);
int __cdecl MSG_ReadLong(msg_t *msg);
char *__cdecl MSG_ReadString(msg_t *msg);
char *__cdecl MSG_ReadBigString(msg_t *msg);
char *__cdecl MSG_ReadStringLine(msg_t *msg);
double __cdecl MSG_ReadAngle16(msg_t *msg);
void __cdecl MSG_ReadData(msg_t *msg, unsigned __int8 *data, int len);
void __cdecl MSG_WriteDeltaKey(msg_t *msg, int key, int oldV, int newV, unsigned int bits);
unsigned int __cdecl MSG_ReadDeltaKey(msg_t *msg, int key, int oldV, unsigned int bits);
void __cdecl MSG_WriteKey(msg_t *msg, int key, int newV, unsigned int bits);
unsigned int __cdecl MSG_ReadKey(msg_t *msg, int key, unsigned int bits);
void __cdecl MSG_WriteDeltaKeyByte(msg_t *msg, char key, char oldV, char newV);
int __cdecl MSG_ReadDeltaKeyByte(msg_t *msg, unsigned __int8 key, int oldV);
void __cdecl MSG_WriteDeltaKeyShort(msg_t *msg, __int16 key, __int16 oldV, __int16 newV);
int __cdecl MSG_ReadDeltaKeyShort(msg_t *msg, __int16 key, int oldV);
void __cdecl MSG_SetDefaultUserCmd(playerState_s *ps, usercmd_s *cmd);
void __cdecl MSG_WriteDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, const usercmd_s *to);
void __cdecl MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, usercmd_s *to);
void __cdecl MSG_ClearLastReferencedEntity(msg_t *msg);
int __cdecl MSG_ReadEntityIndex(msg_t *msg, unsigned int indexBits);
void __cdecl MSG_ReadDeltaField(
    msg_t *msg,
    int time,
    const char * const from,
    char *to,
    const NetField *field,
    int print,
    bool noXor);
int __cdecl MSG_ReadDeltaTime(msg_t *msg, int timeBase);
int __cdecl MSG_ReadDeltaGroundEntity(msg_t *msg);
int __cdecl MSG_ReadDeltaEventParamField(msg_t *msg);
int __cdecl MSG_Read24BitFlag(msg_t *msg, int oldFlags);
double __cdecl MSG_ReadOriginFloat(int bits, msg_t *msg, float oldValue);
double __cdecl MSG_ReadOriginZFloat(msg_t *msg, float oldValue);
int __cdecl MSG_ReadDeltaEntity(msg_t *msg, int time, entityState_s *from, entityState_s *to, unsigned int number);
int __cdecl MSG_ReadDeltaEntityStruct(msg_t *msg, int time, char *from, char *to, unsigned int number);
int __cdecl MSG_ReadLastChangedField(msg_t *msg, int totalFields);
int __cdecl MSG_ReadDeltaArchivedEntity(
    msg_t *msg,
    int time,
    archivedEntity_s *from,
    archivedEntity_s *to,
    unsigned int number);
int __cdecl MSG_ReadDeltaStruct(
    msg_t *msg,
    int time,
    char *from,
    char *to,
    unsigned int number,
    int numFields,
    char indexBits,
    const NetField *stateFields,
    int totalFields);
int __cdecl MSG_ReadDeltaClient(msg_t *msg, int time, clientState_s *from, clientState_s *to, unsigned int number);
void __cdecl MSG_ReadDeltaPlayerstate(
    int localClientNum,
    msg_t *msg,
    int time,
    const playerState_s *from,
    playerState_s *to,
    bool predictedFieldsIgnoreXor);
void __cdecl MSG_InitHuffman();
void MSG_initHuffmanInternal();
void __cdecl MSG_DumpNetFieldChanges_f();
