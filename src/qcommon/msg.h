#pragma once

#ifndef KISAK_SP
#error This File is SinglePlayer Only
#endif

struct msg_t
{
	int overflowed;
	int readOnly;
	unsigned __int8 *data;
	unsigned __int8 *splitData;
	int maxsize;
	int cursize;
	int splitSize;
	int readcount;
	int bit;
	int lastEntityRef;
};

struct netField_t
{
	char *name;
	int offset;
	int bits;
};

struct usercmd_s
{
    int serverTime;
    int buttons;
    int angles[3];
    unsigned __int8 weapon;
    unsigned __int8 offHandIndex;
    char forwardmove;
    char rightmove;
    char upmove;
    char pitchmove;
    char yawmove;
    float gunPitch;
    float gunYaw;
    float gunXOfs;
    float gunYOfs;
    float gunZOfs;
    float meleeChargeYaw;
    unsigned __int8 meleeChargeDist;
    char selectedLocation[2];
};

struct hudelem_s;
struct playerState_s;

void __cdecl TRACK_msg();
void __cdecl MSG_Init(msg_t *buf, unsigned __int8 *data, int length);
void __cdecl MSG_BeginReading(msg_t *msg);
void __cdecl MSG_Truncate(msg_t *msg);
void __cdecl MSG_WriteBit0(msg_t *msg);
void __cdecl MSG_WriteBit1(msg_t *msg);
void __cdecl MSG_WriteBits(msg_t *msg, int value, unsigned int bits);
int __cdecl MSG_ReadBits(msg_t *msg, unsigned int bits);
int __cdecl MSG_ReadBit(msg_t *msg);
void __cdecl MSG_WriteByte(msg_t *msg, unsigned __int8 c);
void __cdecl MSG_WriteData(msg_t *buf, unsigned char *data, int length);
void __cdecl MSG_WriteShort(msg_t *msg, __int16 c);
void __cdecl MSG_WriteLong(msg_t *msg, int c);
void __cdecl MSG_WriteFloat(msg_t *sb, double f);
void __cdecl MSG_WriteString(msg_t *sb, char *s);
void __cdecl MSG_WriteAngle(msg_t *sb, double f);
void __cdecl MSG_WriteAngle16(msg_t *sb, double f);
void __cdecl MSG_WriteInt64(msg_t *msg, unsigned __int64 c);
int __cdecl MSG_ReadByte(msg_t *msg);
int __cdecl MSG_ReadShort(msg_t *msg);
int __cdecl MSG_ReadLong(msg_t *msg);
float __cdecl MSG_ReadFloat(msg_t *msg);
int __cdecl MSG_ReadString(msg_t *msg, char *buffer, int bufsize);
float __cdecl MSG_ReadAngle16(msg_t *msg);
void __cdecl MSG_ReadData(msg_t *msg, unsigned char *data, int len);
int __cdecl MSG_ReadInt64(msg_t *msg);
void __cdecl MSG_WriteDelta(msg_t *msg, int oldV, int newV, unsigned int bits);
int __cdecl MSG_ReadDelta(msg_t *msg, int oldV, unsigned int bits);
void __cdecl MSG_WriteDeltaFloat(msg_t *msg, double oldV, double newV);
float __cdecl MSG_ReadDeltaFloat(msg_t *msg, double oldV);
void __cdecl MSG_WriteDeltaUsercmd(msg_t *msg, const usercmd_s *from, const usercmd_s *to);
void __cdecl MSG_ReadDeltaUsercmd(msg_t *msg, const usercmd_s *from, usercmd_s *to);
void __cdecl MSG_WriteDeltaField(msg_t *msg, unsigned __int8 *to, const netField_t *field);
void __cdecl MSG_ReadDeltaField(msg_t *msg, unsigned __int8 *to, const netField_t *field, int print);
void __cdecl MSG_WriteDeltaHudElems(msg_t *msg, hudelem_s *to, int count);
void __cdecl MSG_ReadDeltaHudElems(msg_t *msg, hudelem_s *to, unsigned int count);
void __cdecl MSG_WriteDeltaPlayerstate(msg_t *msg, playerState_s *to);
void __cdecl MSG_ReadDeltaPlayerstate(msg_t *msg, playerState_s *to);
int __cdecl MSG_GetByte(msg_t *msg, int where);
void __cdecl MSG_GetBytes(msg_t *msg, int where, unsigned __int8 *dest, int len);
