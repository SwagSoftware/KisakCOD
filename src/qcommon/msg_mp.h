#pragma once

#ifndef KISAK_MP
#error This File is for MultiPlayer Only
#endif

#include "qcommon.h"
#include "ent.h"

// LWSS: these are aislop generated, may be inaccurate.
#define BUTTON_ATTACK         (1 <<  0)  // +attack
#define BUTTON_SPRINT         (1 <<  1)  // set in CL_KeyMove from kb[KEY_SPRINT=27]
#define BUTTON_MELEE          (1 <<  2)  // +melee
#define BUTTON_USE            (1 <<  3)  // +activate
#define BUTTON_RELOAD         (1 <<  4)  // +reload
#define BUTTON_USE_RELOAD     (1 <<  5)  // +usereload (combined-bind, distinct from USE/RELOAD)
#define BUTTON_LEAN_LEFT      (1 <<  6)  // +leanleft
#define BUTTON_LEAN_RIGHT     (1 <<  7)  // +leanright
#define BUTTON_PRONE          (1 <<  8)  // +prone (held)
#define BUTTON_CROUCH         (1 <<  9)  // crouch stance (set by CL_AddCurrentStanceToCmd)
#define BUTTON_JUMP           (1 << 10)  // +moveup / +gostand
#define BUTTON_ADS            (1 << 11)  // +speed in COD (aim-down-sights)
#define BUTTON_TEMP_STANCE    (1 << 12)  // holding stance key (vs toggled stance)
#define BUTTON_BREATH         (1 << 13)  // +breath (hold-breath while scoped)
#define BUTTON_FRAG           (1 << 14)  // +frag
#define BUTTON_SMOKE          (1 << 15)  // +smoke
#define BUTTON_LOC_CONFIRM    (1 << 16)  // artillery/location confirm
#define BUTTON_LOC_CANCEL     (1 << 17)  // artillery/location cancel
#define BUTTON_NIGHTVISION    (1 << 18)  // +nightvision
#define BUTTON_THROW          (1 << 19)  // +throw (frag throwback)
#define BUTTON_LOC_SELECTING  (1 << 20)  // location-selection UI active
#define BUTTON_BIT_COUNT 21

#define MAX_WEAPONS_BITS 7
#define GENTITYNUM_BITS 10

#define MSG_FIELD_ORIGINY -91

struct msg_t // sizeof=0x28
{                                       // ...
    int overflowed;                     // ...
    int readOnly;                       // ...
    uint8_t* data;              // ...
    uint8_t* splitData;         // ...
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
    uint8_t changeHints;
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

int __cdecl GetMinBitCountForNum(uint32_t num);
void __cdecl MSG_Init(msg_t *buf, uint8_t *data, int length);
void __cdecl MSG_InitReadOnly(msg_t *buf, uint8_t *data, int length);
void __cdecl MSG_InitReadOnlySplit(msg_t *buf, uint8_t *data, int length, uint8_t *data2, int length2);
void __cdecl MSG_BeginReading(msg_t *msg);
void __cdecl MSG_Discard(msg_t *msg);
int __cdecl MSG_GetUsedBitCount(const msg_t *msg);
void __cdecl MSG_WriteBits(msg_t *msg, int value, uint32_t bits);
void __cdecl MSG_WriteBit0(msg_t *msg);
void __cdecl MSG_WriteBit1(msg_t *msg);
int __cdecl MSG_ReadBits(msg_t *msg, uint32_t bits);
int __cdecl MSG_GetByte(msg_t *msg, int where);
int __cdecl MSG_ReadBit(msg_t *msg);
int __cdecl MSG_WriteBitsCompress(bool trainHuffman, const uint8_t *from, uint8_t *to, int size);
int __cdecl MSG_ReadBitsCompress(const uint8_t *from, uint8_t *to, int size);
void __cdecl MSG_WriteByte(msg_t *msg, uint8_t c);
void __cdecl MSG_WriteData(msg_t *buf, uint8_t *data, uint32_t length);
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
void __cdecl MSG_ReadData(msg_t *msg, uint8_t *data, int len);
void __cdecl MSG_WriteDeltaKey(msg_t *msg, int key, int oldV, int newV, uint32_t bits);
uint32_t __cdecl MSG_ReadDeltaKey(msg_t *msg, int key, int oldV, uint32_t bits);
void __cdecl MSG_WriteKey(msg_t *msg, int key, int newV, uint32_t bits);
uint32_t __cdecl MSG_ReadKey(msg_t *msg, int key, uint32_t bits);
void __cdecl MSG_WriteDeltaKeyByte(msg_t *msg, char key, char oldV, char newV);
int __cdecl MSG_ReadDeltaKeyByte(msg_t *msg, uint8_t key, int oldV);
void __cdecl MSG_WriteDeltaKeyShort(msg_t *msg, __int16 key, __int16 oldV, __int16 newV);
int __cdecl MSG_ReadDeltaKeyShort(msg_t *msg, __int16 key, int oldV);
void __cdecl MSG_SetDefaultUserCmd(playerState_s *ps, usercmd_s *cmd);
void __cdecl MSG_WriteDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, const usercmd_s *to);
void __cdecl MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, usercmd_s *to);
void __cdecl MSG_ClearLastReferencedEntity(msg_t *msg);
int __cdecl MSG_ReadEntityIndex(msg_t *msg, uint32_t indexBits);
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
int __cdecl MSG_ReadDeltaEntity(msg_t *msg, int time, entityState_s *from, entityState_s *to, uint32_t number);
int __cdecl MSG_ReadDeltaEntityStruct(msg_t *msg, int time, char *from, char *to, uint32_t number);
int __cdecl MSG_ReadLastChangedField(msg_t *msg, int totalFields);
int __cdecl MSG_ReadDeltaArchivedEntity(
    msg_t *msg,
    int time,
    archivedEntity_s *from,
    archivedEntity_s *to,
    uint32_t number);
int __cdecl MSG_ReadDeltaStruct(
    msg_t *msg,
    int time,
    char *from,
    char *to,
    uint32_t number,
    int numFields,
    char indexBits,
    const NetField *stateFields,
    int totalFields);
int __cdecl MSG_ReadDeltaClient(msg_t *msg, int time, clientState_s *from, clientState_s *to, uint32_t number);
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
