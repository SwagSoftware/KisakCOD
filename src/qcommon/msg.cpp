#ifndef KISAK_SP
#error This File is SinglePlayer Only
#endif

#include "msg.h"
#include "mem_track.h"
#include <universal/assertive.h>

#include <bgame/bg_local.h>
#include <bgame/bg_public.h>
#include <universal/com_math.h>
#include <client/client.h>

struct netField_t *hudElemFields;

const netField_t playerStateFields[143] =
{
  { (char*)"commandTime", 0, 32 },
  { (char*)"pm_type", 4, 8 },
  { (char*)"bobCycle", 8, 8 },
  { (char*)"pm_flags", 12, 26 },
  { (char*)"weapFlags", 16, 12 },
  { (char*)"otherFlags", 20, 1 },
  { (char*)"pm_time", 24, -16 },
  { (char*)"origin[0]", 28, 0 },
  { (char*)"origin[1]", 32, 0 },
  { (char*)"origin[2]", 36, 0 },
  { (char*)"velocity[0]", 40, 0 },
  { (char*)"velocity[1]", 44, 0 },
  { (char*)"velocity[2]", 48, 0 },
  { (char*)"weaponTime", 60, -16 },
  { (char*)"weaponRestrictKickTime", 80, -16 },
  { (char*)"weaponDelay", 64, -16 },
  { (char*)"grenadeTimeLeft", 68, -16 },
  { (char*)"throwBackGrenadeTimeLeft", 76, -16 },
  { (char*)"throwBackGrenadeOwner", 72, 12 },
  { (char*)"foliageSoundTime", 84, 32 },
  { (char*)"gravity", 88, 16 },
  { (char*)"leanf", 92, 0 },
  { (char*)"speed", 96, 16 },
  { (char*)"delta_angles[0]", 100, 0 },
  { (char*)"delta_angles[1]", 104, 0 },
  { (char*)"delta_angles[2]", 108, 0 },
  { (char*)"groundEntityNum", 112, 12 },
  { (char*)"vLadderVec[0]", 116, 0 },
  { (char*)"vLadderVec[1]", 120, 0 },
  { (char*)"vLadderVec[2]", 124, 0 },
  { (char*)"jumpTime", 128, 32 },
  { (char*)"jumpOriginZ", 132, 0 },
  { (char*)"movementDir", 136, -8 },
  { (char*)"eFlags", 140, 24 },
  { (char*)"eventSequence", 144, 32 },
  { (char*)"oldEventSequence", 180, 32 },
  { (char*)"entityEventSequence", 1564, 32 },
  { (char*)"events[0]", 148, 8 },
  { (char*)"events[1]", 152, 8 },
  { (char*)"events[2]", 156, 8 },
  { (char*)"events[3]", 160, 8 },
  { (char*)"eventParms[0]", 164, 32 },
  { (char*)"eventParms[1]", 168, 32 },
  { (char*)"eventParms[2]", 172, 32 },
  { (char*)"eventParms[3]", 176, 32 },
  { (char*)"clientNum", 184, 8 },
  { (char*)"weapons[0]", 1332, 32 },
  { (char*)"weapons[1]", 1336, 32 },
  { (char*)"weapons[2]", 1340, 32 },
  { (char*)"weapons[3]", 1344, 32 },
  { (char*)"weaponold[0]", 1348, 32 },
  { (char*)"weaponold[1]", 1352, 32 },
  { (char*)"weaponold[2]", 1356, 32 },
  { (char*)"weaponold[3]", 1360, 32 },
  { (char*)"weaponrechamber[0]", 1364, 32 },
  { (char*)"weaponrechamber[1]", 1368, 32 },
  { (char*)"weaponrechamber[2]", 1372, 32 },
  { (char*)"weaponrechamber[3]", 1376, 32 },
  { (char*)"offHandIndex", 188, 7 },
  { (char*)"sprintState.lastSprintStart", 1452, 32 },
  { (char*)"sprintState.lastSprintEnd", 1456, 32 },
  { (char*)"sprintState.sprintStartMaxLength", 1460, 14 },
  { (char*)"sprintState.sprintDelay", 1448, 1 },
  { (char*)"sprintState.sprintButtonUpRequired", 1444, 1 },
  { (char*)"weapon", 196, 7 },
  { (char*)"weapAnim", 1568, 10 },
  { (char*)"weaponstate", 200, 5 },
  { (char*)"weaponShotCount", 204, 3 },
  { (char*)"fWeaponPosFrac", 208, 0 },
  { (char*)"spreadOverride", 216, 6 },
  { (char*)"spreadOverrideState", 220, 2 },
  { (char*)"groundTiltAngles[0]", 1416, 0 },
  { (char*)"groundTiltAngles[1]", 1420, 0 },
  { (char*)"groundTiltAngles[2]", 1424, 0 },
  { (char*)"viewangles[0]", 228, 0 },
  { (char*)"viewangles[1]", 232, 0 },
  { (char*)"viewangles[2]", 236, 0 },
  { (char*)"viewHeightTarget", 240, -8 },
  { (char*)"viewHeightCurrent", 244, 0 },
  { (char*)"viewHeightLerpTime", 248, 32 },
  { (char*)"viewHeightLerpTarget", 252, -8 },
  { (char*)"viewHeightLerpDown", 256, 1 },
  { (char*)"damageEvent", 276, 8 },
  { (char*)"damageYaw", 280, 8 },
  { (char*)"damagePitch", 284, 8 },
  { (char*)"damageCount", 288, 7 },
  { (char*)"proneDirection", 1380, 0 },
  { (char*)"proneDirectionPitch", 1384, 0 },
  { (char*)"proneTorsoPitch", 1388, 0 },
  { (char*)"viewlocked", 1392, 2 },
  { (char*)"viewlocked_entNum", 1396, 16 },
  { (char*)"vehicleType", 1400, 16 },
  { (char*)"linkAngles[0]", 1404, 0 },
  { (char*)"linkAngles[1]", 1408, 0 },
  { (char*)"linkAngles[2]", 1412, 0 },
  { (char*)"aimSpreadScale", 1572, 0 },
  { (char*)"cursorHint", 1428, 8 },
  { (char*)"cursorHintString", 1432, -8 },
  { (char*)"cursorHintEntIndex", 1436, 12 },
  { (char*)"viewmodelIndex", 224, 9 },
  { (char*)"shellshockIndex", 1576, 4 },
  { (char*)"shellshockTime", 1580, 32 },
  { (char*)"shellshockDuration", 1584, 16 },
  { (char*)"offhandSecondary", 192, 1 },
  { (char*)"holdBreathScale", 1472, 0 },
  { (char*)"holdBreathTimer", 1476, 16 },
  { (char*)"locationSelectionInfo", 1440, 8 },
  { (char*)"mantleState.yaw", 1484, 0 },
  { (char*)"mantleState.timer", 1488, 32 },
  { (char*)"mantleState.transIndex", 1492, 4 },
  { (char*)"mantleState.flags", 1496, 5 },
  { (char*)"viewAngleClampBase[0]", 260, 0 },
  { (char*)"viewAngleClampBase[1]", 264, 0 },
  { (char*)"viewAngleClampRange[0]", 268, 0 },
  { (char*)"viewAngleClampRange[1]", 272, 0 },
  { (char*)"moveSpeedScaleMultiplier", 1480, 0 },
  { (char*)"adsDelayTime", 212, 32 },
  { (char*)"oldVelocity[0]", 52, 0 },
  { (char*)"oldVelocity[1]", 56, 0 },
  { (char*)"weapLockFlags", 1512, 6 },
  { (char*)"weapLockedEntnum", 1516, 12 },
  { (char*)"forcedViewAnimWeaponIdx", 1520, 7 },
  { (char*)"forcedViewAnimWeaponState", 1524, 5 },
  { (char*)"forcedViewAnimOriginalWeaponIdx", 1528, 7 },
  { (char*)"actionSlotType[0]", 1532, 2 },
  { (char*)"actionSlotType[1]", 1536, 2 },
  { (char*)"actionSlotType[2]", 1540, 2 },
  { (char*)"actionSlotType[3]", 1544, 2 },
  { (char*)"actionSlotParam[0]", 1548, 7 },
  { (char*)"actionSlotParam[1]", 1552, 7 },
  { (char*)"actionSlotParam[2]", 1556, 7 },
  { (char*)"actionSlotParam[3]", 1560, 7 },
  { (char*)"dofNearStart", 1588, 32 },
  { (char*)"dofNearEnd", 1592, 32 },
  { (char*)"dofFarStart", 1596, 32 },
  { (char*)"dofFarEnd", 1600, 32 },
  { (char*)"dofNearBlur", 1604, 32 },
  { (char*)"dofFarBlur", 1608, 32 },
  { (char*)"dofViewmodelStart", 1612, 32 },
  { (char*)"dofViewmodelEnd", 1616, 32 },
  { (char*)"meleeChargeYaw", 1500, 32 },
  { (char*)"meleeChargeDist", 1504, 8 },
  { (char*)"meleeChargeTime", 1508, 32 }
};



void __cdecl TRACK_msg()
{
    track_static_alloc_internal((void *)playerStateFields, 1716, "playerStateFields", 9);
}

void __cdecl MSG_Init(msg_t *buf, unsigned __int8 *data, int length)
{
    msg_t *v3; // r11
    int v4; // ctr

    v3 = buf;
    v4 = 10;
    do
    {
        v3->overflowed = 0;
        v3 = (msg_t *)((char *)v3 + 4);
        --v4;
    } while (v4);
    buf->data = data;
    buf->maxsize = length;
}

void __cdecl MSG_BeginReading(msg_t *msg)
{
    msg->overflowed = 0;
    msg->readcount = 0;
    msg->bit = 0;
}

void __cdecl MSG_Truncate(msg_t *msg)
{
    msg->cursize = msg->readcount;
}

void __cdecl MSG_WriteBit0(msg_t *msg)
{
    int cursize; // r11
    unsigned __int8 *data; // r9

    cursize = msg->cursize;
    if (cursize < msg->maxsize)
    {
        if ((msg->bit & 7) == 0)
        {
            data = msg->data;
            msg->bit = 8 * cursize;
            data[cursize] = 0;
            ++msg->cursize;
        }
        ++msg->bit;
    }
    else
    {
        msg->overflowed = 1;
    }
}

void __cdecl MSG_WriteBit1(msg_t *msg)
{
    int cursize; // r11
    int v3; // r9
    unsigned __int8 *data; // r8

    if (msg->readOnly)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 333, 0, "%s", "!msg->readOnly");
    cursize = msg->cursize;
    if (cursize < msg->maxsize)
    {
        v3 = msg->bit & 7;
        if (!v3)
        {
            data = msg->data;
            msg->bit = 8 * cursize;
            data[cursize] = 0;
            ++msg->cursize;
        }
        msg->data[msg->bit++ >> 3] |= 1 << v3;
    }
    else
    {
        msg->overflowed = 1;
    }
}

void __cdecl MSG_WriteBits(msg_t *msg, int value, unsigned int bits)
{
    unsigned int v3; // r30
    int v6; // r9
    int cursize; // r11
    unsigned __int8 *data; // r10

    v3 = bits;
    if (bits > 0x20)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 357, 0, "%s", "(unsigned)bits <= 32");
    if (msg->maxsize - msg->cursize >= 4)
    {
        for (; v3; ++msg->bit)
        {
            --v3;
            v6 = msg->bit & 7;
            if (!v6)
            {
                cursize = msg->cursize;
                data = msg->data;
                msg->bit = 8 * cursize;
                data[cursize] = 0;
                ++msg->cursize;
            }
            if ((value & 1) != 0)
                msg->data[msg->bit >> 3] |= 1 << v6;
            value >>= 1;
        }
    }
    else
    {
        msg->overflowed = 1;
    }
}

int __cdecl MSG_ReadBits(msg_t *msg, unsigned int bits)
{
    int result; // r3
    signed int i; // r11
    int v6; // r9
    int readcount; // r10
    int bit; // r10
    int v9; // r7
    unsigned int v10; // r10
    int v11; // r10

    if (bits > 0x20)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 390, 0, "%s", "(unsigned)bits <= 32");
    result = 0;
    for (i = 0; i < (int)bits; result |= v11)
    {
        v6 = msg->bit & 7;
        if (!v6)
        {
            readcount = msg->readcount;
            if (readcount >= msg->cursize)
            {
                result = -1;
                msg->overflowed = 1;
                return result;
            }
            msg->bit = 8 * readcount;
            msg->readcount = readcount + 1;
        }
        bit = msg->bit;
        v9 = bit + 1;
        v10 = msg->data[bit >> 3];
        msg->bit = v9;
        v11 = ((v10 >> v6) & 1) << i++;
    }
    return result;
}

int __cdecl MSG_ReadBit(msg_t *msg)
{
    int v2; // r30
    int readcount; // r11
    int result; // r3
    int Byte; // r3

    v2 = msg->bit & 7;
    if (!v2)
    {
        readcount = msg->readcount;
        if (readcount >= msg->splitSize + msg->cursize)
        {
            result = -1;
            msg->overflowed = 1;
            return result;
        }
        msg->bit = 8 * readcount;
        msg->readcount = readcount + 1;
    }
    Byte = MSG_GetByte(msg, msg->bit >> 3);
    ++msg->bit;
    return (Byte >> v2) & 1;
}

void __cdecl MSG_WriteByte(msg_t *msg, unsigned __int8 c)
{
    int cursize; // r11

    cursize = msg->cursize;
    if (cursize >= msg->maxsize)
    {
        msg->overflowed = 1;
    }
    else
    {
        msg->data[cursize] = c;
        ++msg->cursize;
    }
}

void __cdecl MSG_WriteData(msg_t *buf, unsigned char *data, int length)
{
    int i; // r10
    int cursize; // r11

    for (i = 0; i < length; ++i)
    {
        cursize = buf->cursize;
        if (cursize >= buf->maxsize)
        {
            buf->overflowed = 1;
        }
        else
        {
            buf->data[cursize] = data[i];
            ++buf->cursize;
        }
    }
}

void __cdecl MSG_WriteShort(msg_t *msg, __int16 c)
{
    int cursize; // r11

    cursize = msg->cursize;
    if (cursize + 2 > msg->maxsize)
    {
        msg->overflowed = 1;
    }
    else
    {
        *(_WORD *)&msg->data[cursize] = c;
        msg->cursize = cursize + 2;
    }
}

void __cdecl MSG_WriteLong(msg_t *msg, int c)
{
    int cursize; // r11

    cursize = msg->cursize;
    if (cursize + 4 > msg->maxsize)
    {
        msg->overflowed = 1;
    }
    else
    {
        *(unsigned int *)&msg->data[cursize] = c;
        msg->cursize = cursize + 4;
    }
}

void __cdecl MSG_WriteFloat(msg_t *sb, double f)
{
    int v3; // [sp+50h] [-20h]

    *(float *)&v3 = f;
    if (*(float *)&v3 == NAN)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 509, 0, "%s", "dat.l != -1");
    MSG_WriteBits(sb, v3, 0x20u);
}

void __cdecl MSG_WriteString(msg_t *sb, char *s)
{
    const char *v4; // r11

    if (!s)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 518, 0, "%s", "s");
    v4 = s;
    while (*(unsigned __int8 *)v4++)
        ;
    MSG_WriteData(sb, (unsigned char*)s, v4 - s);
}

void __cdecl MSG_WriteAngle(msg_t *sb, double f)
{
    int cursize; // r11

    cursize = sb->cursize;
    if (cursize >= sb->maxsize)
    {
        sb->overflowed = 1;
    }
    else
    {
        sb->data[cursize] = (int)(float)((float)f * (float)0.71111113);
        ++sb->cursize;
    }
}

void __cdecl MSG_WriteAngle16(msg_t *sb, double f)
{
    int cursize; // r11

    cursize = sb->cursize;
    if (cursize + 2 > sb->maxsize)
    {
        sb->overflowed = 1;
    }
    else
    {
        *(_WORD *)&sb->data[cursize] = (int)(float)((float)f * (float)182.04445);
        sb->cursize = cursize + 2;
    }
}

void __cdecl MSG_WriteInt64(msg_t *msg, unsigned __int64 c)
{
    int newsize; // [esp+4h] [ebp-4h]

    iassert(!msg->readOnly);

    newsize = msg->cursize + 8;
    if (newsize > msg->maxsize)
    {
        msg->overflowed = 1;
    }
    else
    {
        *(_QWORD *)&msg->data[msg->cursize] = LittleLong64(c);
        msg->cursize = newsize;
    }
}

int __cdecl MSG_ReadByte(msg_t *msg)
{
    int readcount; // r10
    int result; // r3

    readcount = msg->readcount;
    if (readcount >= msg->cursize)
    {
        result = -1;
        msg->overflowed = 1;
    }
    else
    {
        result = msg->data[readcount];
        msg->readcount = readcount + 1;
    }
    return result;
}

int __cdecl MSG_ReadShort(msg_t *msg)
{
    int readcount; // r10
    int v3; // r9
    __int16 v4; // r10
    int result; // r3

    readcount = msg->readcount;
    v3 = readcount + 2;
    if (readcount + 2 > msg->cursize)
    {
        result = -1;
        msg->overflowed = 1;
    }
    else
    {
        v4 = *(_WORD *)&msg->data[readcount];
        msg->readcount = v3;
        return v4;
    }
    return result;
}

int __cdecl MSG_ReadLong(msg_t *msg)
{
    int readcount; // r10
    int result; // r3

    readcount = msg->readcount;
    if (readcount + 4 > msg->cursize)
    {
        result = -1;
        msg->overflowed = 1;
    }
    else
    {
        result = *(unsigned int *)&msg->data[readcount];
        msg->readcount = readcount + 4;
    }
    return result;
}

float __cdecl MSG_ReadFloat(msg_t *msg)
{
    double v2; // fp1
    float v4; // [sp+50h] [-20h]

    v4 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
    if (v4 == NAN)
    {
        msg->overflowed = 1;
        v2 = -1.0;
    }
    else
    {
        v2 = v4;
    }
    return *((float *)&v2 + 1);
}

int __cdecl MSG_ReadString(msg_t *msg, char *buffer, int bufsize)
{
    int i; // r10
    int readcount; // r11
    int v5; // r6
    int v6; // r11
    int result; // r3

    for (i = 0; ; ++i)
    {
        readcount = msg->readcount;
        if (readcount >= msg->cursize)
        {
            v6 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v5 = readcount + 1;
            v6 = msg->data[readcount];
            msg->readcount = v5;
        }
        if (i >= bufsize)
        {
            result = 0;
            buffer[bufsize - 1] = 0;
            return result;
        }
        if (v6 == -1)
        {
            result = 0;
            buffer[i] = 0;
            return result;
        }
        buffer[i] = v6;
        if (!v6)
            break;
    }
    return 1;
}

float __cdecl MSG_ReadAngle16(msg_t *msg)
{
    __int64 v1; // r11
    double v2; // fp1

    LODWORD(v1) = msg->readcount;
    HIDWORD(v1) = v1 + 2;
    if ((int)v1 + 2 > msg->cursize)
    {
        v1 = 0x1FFFFFFFFLL;
        msg->overflowed = 1;
    }
    else
    {
        //LOWORD(v1) = *(_WORD *)&msg->data[v1];
        v1 = (v1 & 0xFFFF0000) | (*(_WORD *)&msg->data[v1]);
        msg->readcount = HIDWORD(v1);
        LODWORD(v1) = (__int16)v1;
    }
    v2 = (float)((float)v1 * (float)0.0054931641);
    return *((float *)&v2 + 1);
}

void __cdecl MSG_ReadData(msg_t *msg, _BYTE *data, int len)
{
    int i; // r10
    int readcount; // r11
    int v5; // r7
    unsigned __int8 v6; // r11

    for (i = 0; i < len; ++i)
    {
        readcount = msg->readcount;
        if (readcount >= msg->cursize)
        {
            v6 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v5 = readcount + 1;
            v6 = msg->data[readcount];
            msg->readcount = v5;
        }
        data[i] = v6;
    }
}

int __cdecl MSG_ReadInt64(msg_t *msg)
{
    int readcount; // r4
    int v3; // r30
    unsigned __int64 v4; // r4
    unsigned __int64 v5; // r4
    __int64 v7; // [sp+50h] [-20h] BYREF

    readcount = msg->readcount;
    v3 = readcount + 8;
    if (readcount + 8 > msg->splitSize + msg->cursize)
    {
        HIDWORD(v5) = 0;
        msg->overflowed = 1;
    }
    else
    {
        MSG_GetBytes(msg, readcount, (unsigned __int8 *)&v7, 8);
        HIDWORD(v4) = v7;
        v5 = LittleLong64(v4);
        msg->readcount = v3;
    }
    return HIDWORD(v5);
}

void __cdecl MSG_WriteDelta(msg_t *msg, int oldV, int newV, unsigned int bits)
{
    if (oldV == newV)
    {
        MSG_WriteBits(msg, 0, 1u);
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        MSG_WriteBits(msg, newV, bits);
    }
}

int __cdecl MSG_ReadDelta(msg_t *msg, int oldV, unsigned int bits)
{
    if (MSG_ReadBits(msg, 1u))
        return MSG_ReadBits(msg, bits);
    else
        return oldV;
}

void __cdecl MSG_WriteDeltaFloat(msg_t *msg, double oldV, double newV)
{
    msg_t *v3; // r31
    unsigned int v4; // r5
    int v5; // r4
    float v6; // [sp+84h] [+24h]

    v6 = newV;
    v3 = msg;
    v4 = 1;
    if (oldV == newV)
    {
        v5 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v5 = LODWORD(v6);
        v4 = 32;
        msg = v3;
    }
    MSG_WriteBits(msg, v5, v4);
}

float __cdecl MSG_ReadDeltaFloat(msg_t *msg, double oldV)
{
    double v4; // fp1

    if (MSG_ReadBits(msg, 1u))
        v4 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
    else
        v4 = oldV;
    return *((float *)&v4 + 1);
}

void __cdecl MSG_WriteDeltaUsercmd(msg_t *msg, const usercmd_s *from, const usercmd_s *to)
{
    unsigned int v6; // r5
    int serverTime; // r4
    char v8; // r11
    msg_t *v9; // r3
    int v10; // r29
    unsigned int v11; // r5
    msg_t *v12; // r3
    int v13; // r4
    int v14; // r29
    unsigned int v15; // r5
    msg_t *v16; // r3
    int v17; // r4
    int v18; // r29
    unsigned int v19; // r5
    msg_t *v20; // r3
    int v21; // r4
    unsigned int v22; // r5
    int forwardmove; // r29
    msg_t *v24; // r3
    int v25; // r4
    unsigned int v26; // r5
    int rightmove; // r29
    msg_t *v28; // r3
    int v29; // r4
    unsigned int v30; // r5
    int upmove; // r29
    msg_t *v32; // r3
    int v33; // r4
    int buttons; // r29
    unsigned int v35; // r5
    msg_t *v36; // r3
    int v37; // r4
    int weapon; // r29
    unsigned int v39; // r5
    msg_t *v40; // r3
    int v41; // r4
    int offHandIndex; // r29
    unsigned int v43; // r5
    msg_t *v44; // r3
    int v45; // r4
    unsigned int v46; // r5
    msg_t *v47; // r3
    int v48; // r4
    unsigned int v49; // r5
    msg_t *v50; // r3
    int v51; // r4
    unsigned int v52; // r5
    msg_t *v53; // r3
    int v54; // r4
    unsigned int v55; // r5
    msg_t *v56; // r3
    int v57; // r4
    unsigned int v58; // r5
    msg_t *v59; // r3
    int v60; // r4
    unsigned int v61; // r5
    msg_t *v62; // r3
    int v63; // r4
    int meleeChargeDist; // r30
    float gunPitch; // [sp+50h] [-40h]
    float gunYaw; // [sp+50h] [-40h]
    float gunXOfs; // [sp+50h] [-40h]
    float gunYOfs; // [sp+50h] [-40h]
    float gunZOfs; // [sp+50h] [-40h]
    float meleeChargeYaw; // [sp+50h] [-40h]

    if (from->buttons >= 0x100000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
            759,
            0,
            "%s",
            "from->buttons < (1 << BUTTON_BIT_COUNT)");
    if (to->buttons >= 0x100000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
            760,
            0,
            "%s",
            "to->buttons < (1 << BUTTON_BIT_COUNT)");
    if (to->serverTime - from->serverTime >= 256)
    {
        MSG_WriteBits(msg, 0, 1u);
        serverTime = to->serverTime;
        v6 = 32;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v6 = 8;
        serverTime = to->serverTime - from->serverTime;
    }
    MSG_WriteBits(msg, serverTime, v6);
    if (from->angles[0] != to->angles[0]
        || from->angles[1] != to->angles[1]
        || from->angles[2] != to->angles[2]
        || from->forwardmove != to->forwardmove
        || from->rightmove != to->rightmove
        || from->upmove != to->upmove
        || from->buttons != to->buttons
        || from->weapon != to->weapon
        || from->offHandIndex != to->offHandIndex
        || from->gunPitch != to->gunPitch
        || from->gunYaw != to->gunYaw
        || from->gunXOfs != to->gunXOfs
        || from->gunYOfs != to->gunYOfs
        || from->gunZOfs != to->gunZOfs
        || from->meleeChargeYaw != to->meleeChargeYaw
        || (v8 = 1, from->meleeChargeDist != to->meleeChargeDist))
    {
        v8 = 0;
    }
    v9 = msg;
    if (v8)
        goto LABEL_26;
    MSG_WriteBits(msg, 1, 1u);
    v10 = to->angles[0];
    v11 = 1;
    v12 = msg;
    if (from->angles[0] == v10)
    {
        v13 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v11 = 16;
        v13 = v10;
        v12 = msg;
    }
    MSG_WriteBits(v12, v13, v11);
    v14 = to->angles[1];
    v15 = 1;
    v16 = msg;
    if (from->angles[1] == v14)
    {
        v17 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v15 = 16;
        v17 = v14;
        v16 = msg;
    }
    MSG_WriteBits(v16, v17, v15);
    v18 = to->angles[2];
    v19 = 1;
    v20 = msg;
    if (from->angles[2] == v18)
    {
        v21 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v19 = 16;
        v21 = v18;
        v20 = msg;
    }
    MSG_WriteBits(v20, v21, v19);
    v22 = 1;
    forwardmove = to->forwardmove;
    v24 = msg;
    if (from->forwardmove == forwardmove)
    {
        v25 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v22 = 8;
        v25 = forwardmove;
        v24 = msg;
    }
    MSG_WriteBits(v24, v25, v22);
    v26 = 1;
    rightmove = to->rightmove;
    v28 = msg;
    if (from->rightmove == rightmove)
    {
        v29 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v26 = 8;
        v29 = rightmove;
        v28 = msg;
    }
    MSG_WriteBits(v28, v29, v26);
    v30 = 1;
    upmove = to->upmove;
    v32 = msg;
    if (from->upmove == upmove)
    {
        v33 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v30 = 8;
        v33 = upmove;
        v32 = msg;
    }
    MSG_WriteBits(v32, v33, v30);
    buttons = to->buttons;
    v35 = 1;
    v36 = msg;
    if (from->buttons == buttons)
    {
        v37 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v35 = 20;
        v37 = buttons;
        v36 = msg;
    }
    MSG_WriteBits(v36, v37, v35);
    weapon = to->weapon;
    v39 = 1;
    v40 = msg;
    if (from->weapon == weapon)
    {
        v41 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v39 = 8;
        v41 = weapon;
        v40 = msg;
    }
    MSG_WriteBits(v40, v41, v39);
    offHandIndex = to->offHandIndex;
    v43 = 1;
    v44 = msg;
    if (from->offHandIndex == offHandIndex)
    {
        v45 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v43 = 8;
        v45 = offHandIndex;
        v44 = msg;
    }
    MSG_WriteBits(v44, v45, v43);
    v46 = 1;
    gunPitch = to->gunPitch;
    v47 = msg;
    if (from->gunPitch == gunPitch)
    {
        v48 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v48 = LODWORD(gunPitch);
        v46 = 32;
        v47 = msg;
    }
    MSG_WriteBits(v47, v48, v46);
    v49 = 1;
    gunYaw = to->gunYaw;
    v50 = msg;
    if (from->gunYaw == gunYaw)
    {
        v51 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v51 = LODWORD(gunYaw);
        v49 = 32;
        v50 = msg;
    }
    MSG_WriteBits(v50, v51, v49);
    v52 = 1;
    gunXOfs = to->gunXOfs;
    v53 = msg;
    if (from->gunXOfs == gunXOfs)
    {
        v54 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v54 = LODWORD(gunXOfs);
        v52 = 32;
        v53 = msg;
    }
    MSG_WriteBits(v53, v54, v52);
    v55 = 1;
    gunYOfs = to->gunYOfs;
    v56 = msg;
    if (from->gunYOfs == gunYOfs)
    {
        v57 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v57 = LODWORD(gunYOfs);
        v55 = 32;
        v56 = msg;
    }
    MSG_WriteBits(v56, v57, v55);
    v58 = 1;
    gunZOfs = to->gunZOfs;
    v59 = msg;
    if (from->gunZOfs == gunZOfs)
    {
        v60 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v60 = LODWORD(gunZOfs);
        v58 = 32;
        v59 = msg;
    }
    MSG_WriteBits(v59, v60, v58);
    v61 = 1;
    meleeChargeYaw = to->meleeChargeYaw;
    v62 = msg;
    if (from->meleeChargeYaw == meleeChargeYaw)
    {
        v63 = 0;
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        v63 = LODWORD(meleeChargeYaw);
        v61 = 32;
        v62 = msg;
    }
    MSG_WriteBits(v62, v63, v61);
    meleeChargeDist = to->meleeChargeDist;
    v9 = msg;
    if (from->meleeChargeDist == meleeChargeDist)
    {
    LABEL_26:
        MSG_WriteBits(v9, 0, 1u);
    }
    else
    {
        MSG_WriteBits(msg, 1, 1u);
        MSG_WriteBits(msg, meleeChargeDist, 8u);
    }
}

void __cdecl MSG_ReadDeltaUsercmd(msg_t *msg, const usercmd_s *from, usercmd_s *to)
{
    int v6; // r28
    int Bits; // r11
    int v8; // r28
    int v9; // r11
    int v10; // r28
    int v11; // r11
    char forwardmove; // r28
    char v13; // r3
    char rightmove; // r28
    char v15; // r3
    char upmove; // r28
    char v17; // r3
    int buttons; // r28
    int v19; // r11
    unsigned __int8 weapon; // r28
    unsigned __int8 v21; // r3
    unsigned __int8 offHandIndex; // r28
    unsigned __int8 v23; // r3
    double gunPitch; // fp31
    double v25; // fp0
    double gunYaw; // fp31
    double v27; // fp0
    double gunXOfs; // fp31
    double v29; // fp0
    double gunYOfs; // fp31
    double v31; // fp0
    double gunZOfs; // fp31
    double v33; // fp0
    double meleeChargeYaw; // fp31
    unsigned __int8 meleeChargeDist; // r30

    if (from->buttons >= 0x100000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
            810,
            0,
            "%s",
            "from->buttons < (1 << BUTTON_BIT_COUNT)");
    if (MSG_ReadBits(msg, 1u))
        to->serverTime = MSG_ReadBits(msg, 8u) + from->serverTime;
    else
        to->serverTime = MSG_ReadBits(msg, 0x20u);
    if (MSG_ReadBits(msg, 1u))
    {
        v6 = from->angles[0];
        if (MSG_ReadBits(msg, 1u))
            Bits = MSG_ReadBits(msg, 0x10u);
        else
            Bits = v6;
        to->angles[0] = Bits;
        v8 = from->angles[1];
        if (MSG_ReadBits(msg, 1u))
            v9 = MSG_ReadBits(msg, 0x10u);
        else
            v9 = v8;
        to->angles[1] = v9;
        v10 = from->angles[2];
        if (MSG_ReadBits(msg, 1u))
            v11 = MSG_ReadBits(msg, 0x10u);
        else
            v11 = v10;
        to->angles[2] = v11;
        forwardmove = from->forwardmove;
        if (MSG_ReadBits(msg, 1u))
            v13 = MSG_ReadBits(msg, 8u);
        else
            v13 = forwardmove;
        to->forwardmove = v13;
        rightmove = from->rightmove;
        if (MSG_ReadBits(msg, 1u))
            v15 = MSG_ReadBits(msg, 8u);
        else
            v15 = rightmove;
        to->rightmove = v15;
        upmove = from->upmove;
        if (MSG_ReadBits(msg, 1u))
            v17 = MSG_ReadBits(msg, 8u);
        else
            v17 = upmove;
        to->upmove = v17;
        buttons = from->buttons;
        if (MSG_ReadBits(msg, 1u))
            v19 = MSG_ReadBits(msg, 0x14u);
        else
            v19 = buttons;
        to->buttons = v19;
        weapon = from->weapon;
        if (MSG_ReadBits(msg, 1u))
            v21 = MSG_ReadBits(msg, 8u);
        else
            v21 = weapon;
        to->weapon = v21;
        offHandIndex = from->offHandIndex;
        if (MSG_ReadBits(msg, 1u))
            v23 = MSG_ReadBits(msg, 8u);
        else
            v23 = offHandIndex;
        to->offHandIndex = v23;
        gunPitch = from->gunPitch;
        if (MSG_ReadBits(msg, 1u))
            v25 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
        else
            v25 = gunPitch;
        to->gunPitch = v25;
        gunYaw = from->gunYaw;
        if (MSG_ReadBits(msg, 1u))
            v27 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
        else
            v27 = gunYaw;
        to->gunYaw = v27;
        gunXOfs = from->gunXOfs;
        if (MSG_ReadBits(msg, 1u))
            v29 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
        else
            v29 = gunXOfs;
        to->gunXOfs = v29;
        gunYOfs = from->gunYOfs;
        if (MSG_ReadBits(msg, 1u))
            v31 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
        else
            v31 = gunYOfs;
        to->gunYOfs = v31;
        gunZOfs = from->gunZOfs;
        if (MSG_ReadBits(msg, 1u))
            v33 = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
        else
            v33 = gunZOfs;
        to->gunZOfs = v33;
        meleeChargeYaw = from->meleeChargeYaw;
        if (MSG_ReadBits(msg, 1u))
            meleeChargeYaw = COERCE_FLOAT(MSG_ReadBits(msg, 0x20u));
        to->meleeChargeYaw = meleeChargeYaw;
        meleeChargeDist = from->meleeChargeDist;
        if (MSG_ReadBits(msg, 1u))
            to->meleeChargeDist = MSG_ReadBits(msg, 8u);
        else
            to->meleeChargeDist = meleeChargeDist;
    }
    else
    {
        to->angles[0] = from->angles[0];
        to->angles[1] = from->angles[1];
        to->angles[2] = from->angles[2];
        to->forwardmove = from->forwardmove;
        to->rightmove = from->rightmove;
        to->upmove = from->upmove;
        to->buttons = from->buttons;
        to->weapon = from->weapon;
        to->offHandIndex = from->offHandIndex;
        to->gunPitch = from->gunPitch;
        to->gunYaw = from->gunYaw;
        to->gunXOfs = from->gunXOfs;
        to->gunYOfs = from->gunYOfs;
        to->gunZOfs = from->gunZOfs;
        to->meleeChargeYaw = from->meleeChargeYaw;
        to->meleeChargeDist = from->meleeChargeDist;
    }
    if (to->buttons >= 0x100000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
            864,
            0,
            "%s",
            "to->buttons < (1 << BUTTON_BIT_COUNT)");
}

void __cdecl MSG_WriteDeltaField(msg_t *msg, unsigned __int8 *to, const netField_t *field)
{
    msg_t *v5; // r31
    int offset; // r29
    double v7; // fp31
    __int64 v8; // r10
    int v9; // r28
    unsigned int bits; // r5
    int v11; // [sp+50h] [-40h]

    v5 = msg;
    offset = field->offset;
    if (!*(unsigned int *)&to[offset])
        goto LABEL_2;
    MSG_WriteBits(msg, 1, 1u);
    msg = v5;
    if (field->bits)
    {
        if (*(unsigned int *)&to[offset])
        {
            MSG_WriteBits(v5, 1, 1u);
            bits = field->bits;
            goto LABEL_12;
        }
    LABEL_2:
        MSG_WriteBits(msg, 0, 1u);
        return;
    }
    v7 = *(float *)&to[offset];
    if (v7 == 0.0)
        goto LABEL_2;
    MSG_WriteBits(v5, 1, 1u);
    v11 = (int)v7;
    LODWORD(v8) = (int)v7;
    if ((float)v8 == v7)
    {
        v9 = v11 + 4096;
        if (v11 + 4096 >= 0 && v9 < 0x2000)
        {
            MSG_WriteBits(v5, 0, 1u);
            MSG_WriteBits(v5, v9, 0xDu);
            return;
        }
    }
    MSG_WriteBits(v5, 1, 1u);
    bits = 32;
LABEL_12:
    MSG_WriteBits(v5, *(unsigned int *)&to[offset], bits);
}

void __cdecl MSG_ReadDeltaField(msg_t *msg, unsigned __int8 *to, const netField_t *field, int print)
{
    int offset; // r30
    int Bits; // r6
    __int64 v10; // r11

    offset = field->offset;
    if (!MSG_ReadBits(msg, 1u))
        goto LABEL_10;
    if (field->bits)
    {
        if (MSG_ReadBits(msg, 1u))
        {
            Bits = MSG_ReadBits(msg, field->bits);
            *(unsigned int *)&to[offset] = Bits;
        LABEL_12:
            if (print)
                Com_Printf(16, "%s:%i ", field->name, Bits);
            return;
        }
    LABEL_10:
        *(unsigned int *)&to[offset] = 0;
        return;
    }
    if (!MSG_ReadBits(msg, 1u))
    {
        *(float *)&to[offset] = 0.0;
        return;
    }
    if (!MSG_ReadBits(msg, 1u))
    {
        Bits = MSG_ReadBits(msg, 0xDu) - 4096;
        LODWORD(v10) = Bits;
        *(float *)&to[offset] = (float)v10;
        goto LABEL_12;
    }
    *(unsigned int *)&to[offset] = MSG_ReadBits(msg, 0x20u);
    if (print)
        Com_Printf(16, "%s:%f ", field->name, *(float *)&to[offset]);
}

void __cdecl MSG_WriteDeltaHudElems(msg_t *msg, hudelem_s *to, int count)
{
    int v6; // r31
    hudelem_s *v7; // r11
    int v8; // r28
    int v9; // r24
    int v10; // r31
    int v11; // r9
    int *p_offset; // r11
    int v13; // r8
    const netField_t *v14; // r30
    int v15; // r31

    if (count != 256)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
            1032,
            0,
            "%s",
            "count == MAX_HUDELEMS_PER_CLIENT");
    v6 = 0;
    if (count > 0)
    {
        v7 = to;
        do
        {
            if (v7->type == HE_TYPE_FREE)
                break;
            ++v6;
            ++v7;
        } while (v6 < count);
    }
    MSG_WriteBits(msg, v6, 8u);
    if (v6 > 0)
    {
        v8 = 0;
        v9 = v6;
        do
        {
            v10 = 0;
            v11 = 0;
            p_offset = &hudElemFields[0].offset;
            v13 = 43;
            do
            {
                if (*(he_type_t *)((char *)&to->type + *p_offset + v8 * 172))
                    v10 = v11;
                --v13;
                ++v11;
                p_offset += 3;
            } while (v13);
            MSG_WriteBits(msg, v10, 6u);
            v14 = hudElemFields;
            v15 = v10 + 1;
            do
            {
                MSG_WriteDeltaField(msg, (unsigned __int8 *)&to[v8], v14);
                --v15;
                ++v14;
            } while (v15);
            --v9;
            ++v8;
        } while (v9);
    }
}

void __cdecl MSG_ReadDeltaHudElems(msg_t *msg, hudelem_s *to, unsigned int count)
{
    int Bits; // r3
    unsigned int v7; // r22
    int v8; // r26
    int i; // r21
    unsigned int v10; // r27
    int *p_bits; // r29
    unsigned int v12; // r25
    int v13; // r31
    msg_t *v14; // r3
    bool v15; // zf
    __int64 v16; // r11
    unsigned int v17; // r4
    unsigned int v18; // r11
    int *p_offset; // r10
    int v20; // r9
    hudelem_s *v21; // r31

    if (count != 256)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
            1069,
            0,
            "%s",
            "count == MAX_HUDELEMS_PER_CLIENT");
    Bits = MSG_ReadBits(msg, 8u);
    v7 = Bits;
    if (Bits)
    {
        v8 = 0;
        for (i = Bits; i; --i)
        {
            v10 = MSG_ReadBits(msg, 6u) + 1;
            p_bits = &hudElemFields[0].bits;
            v12 = v10;
            do
            {
                v13 = *(p_bits - 1) + v8;
                if (!MSG_ReadBits(msg, 1u))
                {
                    *(he_type_t *)((char *)&to->type + v13) = HE_TYPE_FREE;
                    goto LABEL_18;
                }
                if (*p_bits)
                {
                    if (!MSG_ReadBits(msg, 1u))
                    {
                        *(he_type_t *)((char *)&to->type + v13) = HE_TYPE_FREE;
                        goto LABEL_18;
                    }
                    v17 = *p_bits;
                    v14 = msg;
                }
                else
                {
                    if (!MSG_ReadBits(msg, 1u))
                    {
                        *(float *)((char *)&to->type + v13) = 0.0;
                        goto LABEL_18;
                    }
                    v15 = MSG_ReadBits(msg, 1u) != 0;
                    v14 = msg;
                    if (!v15)
                    {
                        LODWORD(v16) = MSG_ReadBits(msg, 0xDu) - 4096;
                        *(float *)((char *)&to->type + v13) = (float)v16;
                        goto LABEL_18;
                    }
                    v17 = 32;
                }
                *(he_type_t *)((char *)&to->type + v13) = (he_type_t)MSG_ReadBits(v14, v17);
            LABEL_18:
                --v12;
                p_bits += 3;
            } while (v12);
            if (v10 < 0x2B)
            {
                v18 = 43 - v10;
                p_offset = &hudElemFields[v10].offset;
                do
                {
                    --v18;
                    v20 = *p_offset + v8;
                    p_offset += 3;
                    *(he_type_t *)((char *)&to->type + v20) = HE_TYPE_FREE;
                } while (v18);
            }
            v8 += 172;
        }
    }
    if (v7 < count)
    {
        v21 = &to[v7];
        do
        {
            if (v21->type == HE_TYPE_FREE)
                break;
            memset(v21, 0, sizeof(hudelem_s));
            if (v21->type)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp",
                    1084,
                    0,
                    "%s",
                    "to[inuse].type == HE_TYPE_FREE");
            ++v7;
            ++v21;
        } while (v7 < count);
    }
}

void __cdecl MSG_WriteDeltaPlayerstate(msg_t *msg, playerState_s *to)
{
    int bit; // r11
    int cursize; // r5
    int v6; // r18
    int integer; // r11
    int v8; // r19
    int v9; // r23
    const int *p_bits; // r26
    int v11; // r27
    __int64 v12; // r11
    double v13; // fp0
    int v14; // r29
    int v15; // r6
    int v16; // r30
    int v17; // r28
    int v18; // r29
    int v19; // r11
    int v20; // r30
    int v21; // r11
    int *v22; // r10
    int v23; // r11
    int v24; // r8
    int *v25; // r9
    int v26; // r11
    int v27; // r11
    int v28; // r11
    int v29; // r11
    unsigned int *v30; // r11
    int *v31; // r9
    int v32; // r8
    int v33; // r10
    int v34; // r30
    int *v35; // r28
    int v36; // r29
    int v37; // r11
    int i; // r8
    int v39; // r10
    int v40; // r11
    int v41; // r10
    int v42; // r10
    int *v43; // r29
    int v44; // r27
    int v45; // r30
    int v46; // r11
    int *v47; // r28
    int v48; // r11
    int v49; // r8
    int *v50; // r9
    int v51; // r11
    int v52; // r11
    int v53; // r11
    int v54; // r11
    int j; // r10
    int v56; // r11
    int v57; // r11
    int v58; // r11
    int v59; // r11
    int v60; // r10
    int v61; // r11
    int v62; // r11
    _BYTE v63[12]; // [sp+60h] [-90h] BYREF
    int v64; // [sp+6Ch] [-84h]

    bit = msg->bit;
    cursize = msg->cursize;
    if (bit)
        v6 = 8 * cursize + bit - 20;
    else
        v6 = 8 * cursize - 12;
    if (cl_shownet && ((integer = cl_shownet->current.integer, integer >= 2) || integer == -2))
    {
        v8 = 1;
        Com_Printf(16, "W|%3i: playerstate ", cursize);
    }
    else
    {
        v8 = 0;
    }
    v9 = 143;
    p_bits = &playerStateFields[0].bits;
    do
    {
        v11 = *(p_bits - 1);
        if (!*(int *)((char *)&to->commandTime + v11))
        {
            MSG_WriteBits(msg, 0, 1u);
            goto LABEL_31;
        }
        MSG_WriteBits(msg, 1, 1u);
        LODWORD(v12) = *p_bits;
        if (*p_bits)
        {
            v16 = *(int *)((char *)&to->commandTime + v11);
            v17 = *p_bits;
            if ((int)v12 <= 0)
                v17 = -(int)v12;
            v18 = v17 & 7;
            if ((v17 & 7) != 0)
            {
                MSG_WriteBits(msg, *(int *)((char *)&to->commandTime + v11), v17 & 7);
                v17 -= v18;
                v16 >>= v18;
            }
            for (; v17; v16 >>= 8)
            {
                v19 = msg->cursize;
                if (v19 >= msg->maxsize)
                {
                    msg->overflowed = 1;
                }
                else
                {
                    msg->data[v19] = v16;
                    ++msg->cursize;
                }
                v17 -= 8;
            }
            if (v8)
            {
                v15 = *(int *)((char *)&to->commandTime + v11);
            LABEL_30:
                Com_Printf(16, "%s:%i ", (const char *)*(p_bits - 2), v15);
            }
        }
        else
        {
            v13 = *(float *)((char *)&to->commandTime + v11);
            v14 = (int)v13;
            LODWORD(v12) = (int)v13;
            *(_QWORD *)v63 = v12;
            if ((float)v12 == v13 && (unsigned int)(v14 + 4096) < 0x2000)
            {
                MSG_WriteBits(msg, 0, 1u);
                MSG_WriteBits(msg, v14 + 4096, 0xDu);
                if (!v8)
                    goto LABEL_31;
                v15 = v14;
                goto LABEL_30;
            }
            MSG_WriteBits(msg, 1, 1u);
            MSG_WriteBits(msg, *(int *)((char *)&to->commandTime + v11), 0x20u);
            if (v8)
                Com_Printf(16, "%s:%f ", (const char *)*(p_bits - 2), *(float *)((char *)&to->commandTime + v11));
        }
    LABEL_31:
        --v9;
        p_bits += 3;
    } while (v9);
    v20 = 0;
    v21 = 2;
    v22 = &to->stats[1];
    do
    {
        if (*(v22 - 1))
            v20 |= 1 << (v21 - 2);
        if (*v22)
            v20 |= 1 << (v21 - 1);
        if (v22[1])
            v20 |= 1 << v21;
        if (v22[2])
            v20 |= 1 << (v21 + 1);
        v21 += 4;
        v22 += 4;
    } while (v21 - 2 < 16);
    if (v20)
    {
        MSG_WriteBits(msg, 1, 1u);
        v23 = msg->cursize;
        if (v23 + 2 > msg->maxsize)
        {
            msg->overflowed = 1;
        }
        else
        {
            *(_WORD *)&msg->data[v23] = v20;
            msg->cursize = v23 + 2;
        }
        v24 = 2;
        v25 = &to->stats[1];
        do
        {
            if (((1 << (v24 - 2)) & v20) != 0)
            {
                v26 = msg->cursize;
                if (v26 + 2 > msg->maxsize)
                {
                    msg->overflowed = 1;
                }
                else
                {
                    *(_WORD *)&msg->data[v26] = *(v25 - 1);
                    msg->cursize = v26 + 2;
                }
            }
            if (((1 << (v24 - 1)) & v20) != 0)
            {
                v27 = msg->cursize;
                if (v27 + 2 > msg->maxsize)
                {
                    msg->overflowed = 1;
                }
                else
                {
                    *(_WORD *)&msg->data[v27] = *v25;
                    msg->cursize = v27 + 2;
                }
            }
            if (((1 << v24) & v20) != 0)
            {
                v28 = msg->cursize;
                if (v28 + 2 > msg->maxsize)
                {
                    msg->overflowed = 1;
                }
                else
                {
                    *(_WORD *)&msg->data[v28] = v25[1];
                    msg->cursize = v28 + 2;
                }
            }
            if (((1 << (v24 + 1)) & v20) != 0)
            {
                v29 = msg->cursize;
                if (v29 + 2 > msg->maxsize)
                {
                    msg->overflowed = 1;
                }
                else
                {
                    *(_WORD *)&msg->data[v29] = v25[2];
                    msg->cursize = v29 + 2;
                }
            }
            v24 += 4;
            v25 += 4;
        } while (v24 - 2 < 16);
    }
    else
    {
        MSG_WriteBits(msg, 0, 1u);
    }
    v30 = (unsigned int*)v63;
    v31 = &to->ammo[1];
    v32 = 4;
    do
    {
        v33 = 2;
        *v30 = 0;
        do
        {
            if (*(v31 - 1))
                *v30 |= 1 << (v33 - 2);
            if (*v31)
                *v30 |= 1 << (v33 - 1);
            if (v31[1])
                *v30 |= 1 << v33;
            if (v31[2])
                *v30 |= 1 << (v33 + 1);
            v33 += 4;
            v31 += 4;
        } while (v33 - 2 < 16);
        --v32;
        ++v30;
    } while (v32);
    if (*(unsigned int *)v63 || *(_QWORD *)&v63[4] || v64)
    {
        MSG_WriteBits(msg, 1, 1u);
        v34 = 78;
        v35 = (int *)v63;
        do
        {
            v36 = *v35;
            if (*v35)
            {
                MSG_WriteBits(msg, 1, 1u);
                v37 = msg->cursize;
                if (v37 + 2 > msg->maxsize)
                {
                    msg->overflowed = 1;
                }
                else
                {
                    *(_WORD *)&msg->data[v37] = v36;
                    msg->cursize = v37 + 2;
                }
                for (i = 0; i < 16; i += 4)
                {
                    if (((1 << i) & v36) != 0)
                    {
                        v39 = msg->cursize;
                        if (v39 + 2 > msg->maxsize)
                        {
                            msg->overflowed = 1;
                        }
                        else
                        {
                            *(_WORD *)&msg->data[v39] = *((unsigned int *)to + v34 + i - 1);
                            msg->cursize = v39 + 2;
                        }
                    }
                    if (((1 << (i + 1)) & v36) != 0)
                    {
                        v40 = msg->cursize;
                        if (v40 + 2 > msg->maxsize)
                        {
                            msg->overflowed = 1;
                        }
                        else
                        {
                            *(_WORD *)&msg->data[v40] = *(&to->commandTime + v34 + i);
                            msg->cursize = v40 + 2;
                        }
                    }
                    if (((1 << (i + 2)) & v36) != 0)
                    {
                        v41 = msg->cursize;
                        if (v41 + 2 > msg->maxsize)
                        {
                            msg->overflowed = 1;
                        }
                        else
                        {
                            *(_WORD *)&msg->data[v41] = *(&to->pm_type + v34 + i);
                            msg->cursize = v41 + 2;
                        }
                    }
                    if (((1 << (i + 3)) & v36) != 0)
                    {
                        v42 = msg->cursize;
                        if (v42 + 2 > msg->maxsize)
                        {
                            msg->overflowed = 1;
                        }
                        else
                        {
                            *(_WORD *)&msg->data[v42] = *(&to->bobCycle + v34 + i);
                            msg->cursize = v42 + 2;
                        }
                    }
                }
            }
            else
            {
                MSG_WriteBits(msg, 0, 1u);
            }
            v34 += 16;
            ++v35;
        } while (v34 < 142);
    }
    else
    {
        MSG_WriteBits(msg, 0, 1u);
    }
    v43 = &to->ammoclip[1];
    v44 = 8;
    do
    {
        v45 = 0;
        v46 = 2;
        v47 = v43;
        do
        {
            if (*(v47 - 1))
                v45 |= 1 << (v46 - 2);
            if (*v47)
                v45 |= 1 << (v46 - 1);
            if (v47[1])
                v45 |= 1 << v46;
            if (v47[2])
                v45 |= 1 << (v46 + 1);
            v46 += 4;
            v47 += 4;
        } while (v46 - 2 < 16);
        if (v45)
        {
            MSG_WriteBits(msg, 1, 1u);
            v48 = msg->cursize;
            if (v48 + 2 > msg->maxsize)
            {
                msg->overflowed = 1;
            }
            else
            {
                *(_WORD *)&msg->data[v48] = v45;
                msg->cursize = v48 + 2;
            }
            v49 = 2;
            v50 = v43;
            do
            {
                if (((1 << (v49 - 2)) & v45) != 0)
                {
                    v51 = msg->cursize;
                    if (v51 + 2 > msg->maxsize)
                    {
                        msg->overflowed = 1;
                    }
                    else
                    {
                        *(_WORD *)&msg->data[v51] = *(v50 - 1);
                        msg->cursize = v51 + 2;
                    }
                }
                if (((1 << (v49 - 1)) & v45) != 0)
                {
                    v52 = msg->cursize;
                    if (v52 + 2 > msg->maxsize)
                    {
                        msg->overflowed = 1;
                    }
                    else
                    {
                        *(_WORD *)&msg->data[v52] = *v50;
                        msg->cursize = v52 + 2;
                    }
                }
                if (((1 << v49) & v45) != 0)
                {
                    v53 = msg->cursize;
                    if (v53 + 2 > msg->maxsize)
                    {
                        msg->overflowed = 1;
                    }
                    else
                    {
                        *(_WORD *)&msg->data[v53] = v50[1];
                        msg->cursize = v53 + 2;
                    }
                }
                if (((1 << (v49 + 1)) & v45) != 0)
                {
                    v54 = msg->cursize;
                    if (v54 + 2 > msg->maxsize)
                    {
                        msg->overflowed = 1;
                    }
                    else
                    {
                        *(_WORD *)&msg->data[v54] = v50[2];
                        msg->cursize = v54 + 2;
                    }
                }
                v49 += 4;
                v50 += 4;
            } while (v49 - 2 < 16);
        }
        else
        {
            MSG_WriteBits(msg, 0, 1u);
        }
        --v44;
        v43 = v47;
    } while (v44);
    MSG_WriteDeltaHudElems(msg, to->hud.elem, 256);
    for (j = 0; j < 128; j += 4)
    {
        v56 = msg->cursize;
        if (v56 >= msg->maxsize)
        {
            msg->overflowed = 1;
        }
        else
        {
            msg->data[v56] = to->weaponmodels[j];
            ++msg->cursize;
        }
        v57 = msg->cursize;
        if (v57 >= msg->maxsize)
        {
            msg->overflowed = 1;
        }
        else
        {
            msg->data[v57] = to->weaponmodels[j + 1];
            ++msg->cursize;
        }
        v58 = msg->cursize;
        if (v58 >= msg->maxsize)
        {
            msg->overflowed = 1;
        }
        else
        {
            msg->data[v58] = to->weaponmodels[j + 2];
            ++msg->cursize;
        }
        v59 = msg->cursize;
        if (v59 >= msg->maxsize)
        {
            msg->overflowed = 1;
        }
        else
        {
            msg->data[v59] = to->weaponmodels[j + 3];
            ++msg->cursize;
        }
    }
    if (v8)
    {
        v60 = msg->bit;
        v61 = 8 * msg->cursize;
        if (v60)
            v62 = v61 + v60 - 20;
        else
            v62 = v61 - 12;
        Com_Printf(16, " (%i bits)\n", v62 - v6);
    }
}

void __cdecl MSG_ReadDeltaPlayerstate(msg_t *msg, playerState_s *to)
{
    int bit; // r11
    int readcount; // r5
    int v6; // r17
    int integer; // r11
    int v8; // r18
    int v9; // r23
    const int *p_bits; // r26
    int v11; // r28
    unsigned int v12; // r11
    int Bits; // r6
    __int64 v14; // r11
    int v15; // r29
    bool v16; // r27
    int v17; // r10
    int cursize; // r9
    int v19; // r11
    int v20; // r7
    int v21; // r11
    int v22; // r11
    int v23; // r11
    int v24; // r10
    __int16 v25; // r11
    int v26; // r8
    int v27; // r9
    int *v28; // r7
    int v29; // r11
    int v30; // r10
    int v31; // r11
    int v32; // r11
    int v33; // r10
    int v34; // r11
    int v35; // r11
    int v36; // r10
    int v37; // r11
    int v38; // r11
    int v39; // r10
    int v40; // r11
    int i; // r30
    int v42; // r11
    int v43; // r10
    __int16 v44; // r11
    int v45; // r7
    int j; // r8
    int v47; // r11
    int v48; // r10
    int v49; // r11
    int v50; // r11
    int v51; // r10
    int v52; // r11
    int v53; // r11
    int v54; // r10
    int v55; // r11
    int v56; // r11
    int v57; // r10
    int v58; // r11
    int k; // r30
    int v60; // r11
    int v61; // r10
    __int16 v62; // r11
    int v63; // r7
    int m; // r8
    int v65; // r11
    int v66; // r10
    int v67; // r11
    int v68; // r11
    int v69; // r10
    int v70; // r11
    int v71; // r11
    int v72; // r10
    int v73; // r11
    int v74; // r11
    int v75; // r10
    int v76; // r11
    unsigned __int8 *v77; // r10
    int v78; // r9
    int v79; // r11
    int v80; // r7
    unsigned __int8 v81; // r11
    int v82; // r11
    int v83; // r7
    unsigned __int8 v84; // r11
    int v85; // r11
    int v86; // r7
    unsigned __int8 v87; // r11
    int v88; // r11
    int v89; // r7
    unsigned __int8 v90; // r11
    int v91; // r10
    int v92; // r11
    int v93; // r11

    memset(to, 0, sizeof(playerState_s));
    bit = msg->bit;
    readcount = msg->readcount;
    if (bit)
        v6 = 8 * readcount + bit - 20;
    else
        v6 = 8 * readcount - 12;
    if (cl_shownet && ((integer = cl_shownet->current.integer, integer >= 2) || integer == -2))
    {
        v8 = 1;
        Com_Printf(16, "%3i: playerstate ", readcount);
    }
    else
    {
        v8 = 0;
    }
    v9 = 143;
    p_bits = &playerStateFields[0].bits;
    do
    {
        v11 = *(p_bits - 1);
        if (!MSG_ReadBits(msg, 1u))
        {
            *(int *)((char *)&to->commandTime + v11) = 0;
            goto LABEL_34;
        }
        v12 = *p_bits;
        if (*p_bits)
        {
            v15 = *p_bits;
            //v16 = _cntlzw(v12) == 0;
            v16 = (v12 & 0x80000000u) != 0;
            if (v16)
                v15 = -v12;
            if ((v15 & 7) != 0)
                Bits = MSG_ReadBits(msg, v15 & 7);
            else
                Bits = 0;
            v17 = v15 & 7;
            if (v17 < v15)
            {
                cursize = msg->cursize;
                do
                {
                    v19 = msg->readcount;
                    if (v19 >= cursize)
                    {
                        v21 = -1;
                        msg->overflowed = 1;
                    }
                    else
                    {
                        v20 = v19 + 1;
                        v21 = msg->data[v19];
                        msg->readcount = v20;
                    }
                    v22 = v21 << v17;
                    v17 += 8;
                    Bits |= v22;
                } while (v17 < v15);
            }
            if (v16 && ((1 << (v15 - 1)) & Bits) != 0)
                Bits |= ~((1 << v15) - 1);
            *(int *)((char *)&to->commandTime + v11) = Bits;
            goto LABEL_32;
        }
        if (!MSG_ReadBits(msg, 1u))
        {
            Bits = MSG_ReadBits(msg, 0xDu) - 4096;
            LODWORD(v14) = Bits;
            *(float *)((char *)&to->commandTime + v11) = (float)v14;
        LABEL_32:
            if (v8)
                Com_Printf(16, "%s:%i ", (const char *)*(p_bits - 2), Bits);
            goto LABEL_34;
        }
        *(int *)((char *)&to->commandTime + v11) = MSG_ReadBits(msg, 0x20u);
        if (v8)
            Com_Printf(16, "%s:%f ", (const char *)*(p_bits - 2), *(float *)((char *)&to->commandTime + v11));
    LABEL_34:
        --v9;
        p_bits += 3;
    } while (v9);
    if (MSG_ReadBits(msg, 1u))
    {
        if (cl_shownet && cl_shownet->current.integer == 4)
            Com_Printf(16, "%s ", "PS_STATS");
        v23 = msg->readcount;
        v24 = v23 + 2;
        if (v23 + 2 > msg->cursize)
        {
            v26 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v25 = *(_WORD *)&msg->data[v23];
            msg->readcount = v24;
            v26 = v25;
        }
        v27 = 2;
        v28 = &to->stats[1];
        do
        {
            if (((1 << (v27 - 2)) & v26) != 0)
            {
                v29 = msg->readcount;
                v30 = v29 + 2;
                if (v29 + 2 > msg->cursize)
                {
                    v31 = -1;
                    msg->overflowed = 1;
                }
                else
                {
                    //LOWORD(v31) = *(_WORD *)&msg->data[v29];
                    v31 = (v31 & 0xFFFF0000) | (*(_WORD *)&msg->data[v29]);
                    msg->readcount = v30;
                    v31 = (__int16)v31;
                }
                *(v28 - 1) = v31;
            }
            if (((1 << (v27 - 1)) & v26) != 0)
            {
                v32 = msg->readcount;
                v33 = v32 + 2;
                if (v32 + 2 > msg->cursize)
                {
                    v34 = -1;
                    msg->overflowed = 1;
                }
                else
                {
                    //LOWORD(v34) = *(_WORD *)&msg->data[v32];
                    v34 = (v34 & 0xFFFF0000) | (*(_WORD *)&msg->data[v32]);
                    msg->readcount = v33;
                    v34 = (__int16)v34;
                }
                *v28 = v34;
            }
            if (((1 << v27) & v26) != 0)
            {
                v35 = msg->readcount;
                v36 = v35 + 2;
                if (v35 + 2 > msg->cursize)
                {
                    v37 = -1;
                    msg->overflowed = 1;
                }
                else
                {
                    //LOWORD(v37) = *(_WORD *)&msg->data[v35];
                    v37 = (v37 & 0xFFFF0000) | (*(_WORD *)&msg->data[v35]);
                    msg->readcount = v36;
                    v37 = (__int16)v37;
                }
                v28[1] = v37;
            }
            if (((1 << (v27 + 1)) & v26) != 0)
            {
                v38 = msg->readcount;
                v39 = v38 + 2;
                if (v38 + 2 > msg->cursize)
                {
                    v40 = -1;
                    msg->overflowed = 1;
                }
                else
                {
                    //LOWORD(v40) = *(_WORD *)&msg->data[v38];
                    v40 = (v40 & 0xFFFF0000) | (*(_WORD *)&msg->data[v38]);
                    msg->readcount = v39;
                    v40 = (__int16)v40;
                }
                v28[2] = v40;
            }
            v27 += 4;
            v28 += 4;
        } while (v27 - 2 < 16);
    }
    if (MSG_ReadBits(msg, 1u))
    {
        for (i = 78; i < 142; i += 16)
        {
            if (MSG_ReadBits(msg, 1u))
            {
                if (cl_shownet && cl_shownet->current.integer == 4)
                    Com_Printf(16, "%s ", "PS_AMMO");
                v42 = msg->readcount;
                v43 = v42 + 2;
                if (v42 + 2 > msg->cursize)
                {
                    v45 = -1;
                    msg->overflowed = 1;
                }
                else
                {
                    v44 = *(_WORD *)&msg->data[v42];
                    msg->readcount = v43;
                    v45 = v44;
                }
                for (j = 0; j < 16; j += 4)
                {
                    if (((1 << j) & v45) != 0)
                    {
                        v47 = msg->readcount;
                        v48 = v47 + 2;
                        if (v47 + 2 > msg->cursize)
                        {
                            v49 = -1;
                            msg->overflowed = 1;
                        }
                        else
                        {
                            //LOWORD(v49) = *(_WORD *)&msg->data[v47];
                            v49 = (v49 & 0xFFFF0000) | (*(_WORD *)&msg->data[v47]);
                            msg->readcount = v48;
                            v49 = (__int16)v49;
                        }
                        *((unsigned int *)to + i + j - 1) = v49;
                    }
                    if (((1 << (j + 1)) & v45) != 0)
                    {
                        v50 = msg->readcount;
                        v51 = v50 + 2;
                        if (v50 + 2 > msg->cursize)
                        {
                            v52 = -1;
                            msg->overflowed = 1;
                        }
                        else
                        {
                            //LOWORD(v52) = *(_WORD *)&msg->data[v50];
                            v52 = (v52 & 0xFFFF0000) | (*(_WORD *)&msg->data[v50]);
                            msg->readcount = v51;
                            v52 = (__int16)v52;
                        }
                        *(&to->commandTime + i + j) = v52;
                    }
                    if (((1 << (j + 2)) & v45) != 0)
                    {
                        v53 = msg->readcount;
                        v54 = v53 + 2;
                        if (v53 + 2 > msg->cursize)
                        {
                            v55 = -1;
                            msg->overflowed = 1;
                        }
                        else
                        {
                            //LOWORD(v55) = *(_WORD *)&msg->data[v53];
                            v55 = (v55 & 0xFFFF0000) | (*(_WORD *)&msg->data[v53]);
                            msg->readcount = v54;
                            v55 = (__int16)v55;
                        }
                        *(&to->pm_type + i + j) = (pmtype_t)v55;
                    }
                    if (((1 << (j + 3)) & v45) != 0)
                    {
                        v56 = msg->readcount;
                        v57 = v56 + 2;
                        if (v56 + 2 > msg->cursize)
                        {
                            v58 = -1;
                            msg->overflowed = 1;
                        }
                        else
                        {
                            //LOWORD(v58) = *(_WORD *)&msg->data[v56];
                            v58 = (v58 & 0xFFFF0000) | (*(_WORD *)&msg->data[v56]);
                            msg->readcount = v57;
                            v58 = (__int16)v58;
                        }
                        *(&to->bobCycle + i + j) = v58;
                    }
                }
            }
        }
    }
    for (k = 206; k < 334; k += 16)
    {
        if (MSG_ReadBits(msg, 1u))
        {
            if (cl_shownet && cl_shownet->current.integer == 4)
                Com_Printf(16, "%s ", "PS_AMMOCLIP");
            v60 = msg->readcount;
            v61 = v60 + 2;
            if (v60 + 2 > msg->cursize)
            {
                v63 = -1;
                msg->overflowed = 1;
            }
            else
            {
                v62 = *(_WORD *)&msg->data[v60];
                msg->readcount = v61;
                v63 = v62;
            }
            for (m = 0; m < 16; m += 4)
            {
                if (((1 << m) & v63) != 0)
                {
                    v65 = msg->readcount;
                    v66 = v65 + 2;
                    if (v65 + 2 > msg->cursize)
                    {
                        v67 = -1;
                        msg->overflowed = 1;
                    }
                    else
                    {
                        //LOWORD(v67) = *(_WORD *)&msg->data[v65];
                        v67 = (v67 & 0xFFFF0000) | (*(_WORD *)&msg->data[v65]);
                        msg->readcount = v66;
                        v67 = (__int16)v67;
                    }
                    *((unsigned int *)to + k + m - 1) = v67;
                }
                if (((1 << (m + 1)) & v63) != 0)
                {
                    v68 = msg->readcount;
                    v69 = v68 + 2;
                    if (v68 + 2 > msg->cursize)
                    {
                        v70 = -1;
                        msg->overflowed = 1;
                    }
                    else
                    {
                        //LOWORD(v70) = *(_WORD *)&msg->data[v68];
                        v70 = (v70 & 0xFFFF0000) | (*(_WORD *)&msg->data[v68]);
                        msg->readcount = v69;
                        v70 = (__int16)v70;
                    }
                    *(&to->commandTime + k + m) = v70;
                }
                if (((1 << (m + 2)) & v63) != 0)
                {
                    v71 = msg->readcount;
                    v72 = v71 + 2;
                    if (v71 + 2 > msg->cursize)
                    {
                        v73 = -1;
                        msg->overflowed = 1;
                    }
                    else
                    {
                        //LOWORD(v73) = *(_WORD *)&msg->data[v71];
                        v73 = (v73 & 0xFFFF0000) | (*(_WORD *)&msg->data[v71]);
                        msg->readcount = v72;
                        v73 = (__int16)v73;
                    }
                    *(&to->pm_type + k + m) = (pmtype_t)v73;
                }
                if (((1 << (m + 3)) & v63) != 0)
                {
                    v74 = msg->readcount;
                    v75 = v74 + 2;
                    if (v74 + 2 > msg->cursize)
                    {
                        v76 = -1;
                        msg->overflowed = 1;
                    }
                    else
                    {
                        //LOWORD(v76) = *(_WORD *)&msg->data[v74];
                        v76 = (v76 & 0xFFFF0000) | (*(_WORD *)&msg->data[v74]);
                        msg->readcount = v75;
                        v76 = (__int16)v76;
                    }
                    *(&to->bobCycle + k + m) = v76;
                }
            }
        }
    }
    MSG_ReadDeltaHudElems(msg, to->hud.elem, 0x100u);
    v77 = &to->weaponmodels[1];
    v78 = 32;
    do
    {
        v79 = msg->readcount;
        if (v79 >= msg->cursize)
        {
            v81 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v80 = v79 + 1;
            v81 = msg->data[v79];
            msg->readcount = v80;
        }
        *(v77 - 1) = v81;
        v82 = msg->readcount;
        if (v82 >= msg->cursize)
        {
            v84 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v83 = v82 + 1;
            v84 = msg->data[v82];
            msg->readcount = v83;
        }
        *v77 = v84;
        v85 = msg->readcount;
        if (v85 >= msg->cursize)
        {
            v87 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v86 = v85 + 1;
            v87 = msg->data[v85];
            msg->readcount = v86;
        }
        v77[1] = v87;
        v88 = msg->readcount;
        if (v88 >= msg->cursize)
        {
            v90 = -1;
            msg->overflowed = 1;
        }
        else
        {
            v89 = v88 + 1;
            v90 = msg->data[v88];
            msg->readcount = v89;
        }
        --v78;
        v77[2] = v90;
        v77 += 4;
    } while (v78);
    if (v8)
    {
        v91 = msg->bit;
        v92 = 8 * msg->readcount;
        if (v91)
            v93 = v92 + v91 - 20;
        else
            v93 = v92 - 12;
        Com_Printf(16, " (%i bits)\n", v93 - v6);
    }
}

int __cdecl MSG_GetByte(msg_t *msg, int where)
{
    if (where < msg->cursize)
        return msg->data[where];
    if (!msg->splitData)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 285, 0, "%s", "msg->splitData");
    return msg->splitData[where - msg->cursize];
}

void __cdecl MSG_GetBytes(msg_t *msg, int where, unsigned __int8 *dest, int len)
{
    int i; // r30
    unsigned __int8 *data; // r11
    unsigned __int8 v10; // r11

    for (i = 0; i < len; ++i)
    {
        if (where < msg->cursize)
        {
            data = msg->data;
        }
        else
        {
            if (!msg->splitData)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\qcommon\\msg.cpp", 285, 0, "%s", "msg->splitData");
            data = &msg->splitData[-msg->cursize];
        }
        v10 = data[where++];
        dest[i] = v10;
    }
}

