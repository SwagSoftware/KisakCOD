#include "msg_mp.h"
#include "huffman.h"

#include <Windows.h>
#include <bgame/bg_local.h>
#include "sv_msg_write_mp.h"
#include <server_mp/server_mp.h>
#include <client_mp/client_mp.h>
#include <cgame/cg_local.h>

#define NETF_OBJ(x) NETF_BASE(objective_t, x)

static const NetField objectiveFields[6] =
{
  { NETF_OBJ(origin[0]), 0, 0u },
  { NETF_OBJ(origin[1]), 0, 0u },
  { NETF_OBJ(origin[2]), 0, 0u },
  { NETF_OBJ(icon), 12, 0u },
  { NETF_OBJ(entNum), 10, 0u },
  { NETF_OBJ(teamNum), 4, 0u }
}; // idb

//  struct netFieldOrderInfo_t orderInfo 82f87210     msg_mp.obj
//  unsigned int *huffBytesSeen      82f878d0     msg_mp.obj
//  struct huffman_t msgHuff   82f87cd8     msg_mp.obj

int msgInit;
unsigned int huffBytesSeen[256];

int __cdecl GetMinBitCountForNum(unsigned int num)
{
    int v2; // eax

    if (!_BitScanReverse((unsigned long*)&v2, num))
    {
        //v2 = `CountLeadingZeros'::`2': : notFound;
        v2 = 63;
    }

    return 32 - (v2 ^ 0x1F);
}

void __cdecl MSG_Init(msg_t *buf, unsigned __int8 *data, int length)
{
    if (!msgInit)
        MSG_InitHuffman();
    buf->overflowed = 0;
    buf->readOnly = 0;
    buf->data = 0;
    buf->splitData = 0;
    buf->maxsize = 0;
    buf->cursize = 0;
    buf->splitSize = 0;
    buf->readcount = 0;
    buf->bit = 0;
    buf->lastEntityRef = 0;
    buf->data = data;
    buf->maxsize = length;
    buf->readOnly = 0;
    buf->splitData = 0;
    buf->splitSize = 0;
}

void __cdecl MSG_InitReadOnly(msg_t *buf, unsigned __int8 *data, int length)
{
    iassert( data );
    if (!msgInit)
        MSG_InitHuffman();
    buf->readOnly = 1;
    buf->data = data;
    buf->maxsize = length;
    buf->cursize = length;
    buf->splitData = 0;
    buf->splitSize = 0;
}

void __cdecl MSG_InitReadOnlySplit(msg_t *buf, unsigned __int8 *data, int length, unsigned __int8 *data2, int length2)
{
    iassert( data );
    iassert( data2 );
    if (!msgInit)
        MSG_InitHuffman();
    buf->readOnly = 1;
    buf->data = data;
    buf->maxsize = length2 + length;
    buf->cursize = length;
    buf->splitData = data2;
    buf->splitSize = length2;
}

void __cdecl MSG_BeginReading(msg_t *msg)
{
    msg->overflowed = 0;
    msg->readcount = 0;
    msg->bit = 0;
}

void __cdecl MSG_Discard(msg_t *msg)
{
    msg->overflowed = 1;
    msg->cursize = msg->readcount;
    msg->splitSize = 0;
}

int __cdecl MSG_GetUsedBitCount(const msg_t *msg)
{
    return 8 * (msg->splitSize + msg->cursize) - ((8 - msg->bit) & 7);
}

void __cdecl MSG_WriteBits(msg_t *msg, int value, unsigned int bits)
{
    int bit; // [esp+4h] [ebp-4h]

    iassert( (unsigned)bits <= 32 );
    iassert( !msg->readOnly );
    if (msg->maxsize - msg->cursize >= 4)
    {
        while (bits)
        {
            --bits;
            bit = msg->bit & 7;
            if (!bit)
            {
                msg->bit = 8 * msg->cursize;
                msg->data[msg->cursize++] = 0;
            }
            if ((value & 1) != 0)
                msg->data[msg->bit >> 3] |= 1 << bit;
            ++msg->bit;
            value >>= 1;
        }
    }
    else
    {
        msg->overflowed = 1;
    }
}

void __cdecl MSG_WriteBit0(msg_t *msg)
{
    iassert( !msg->readOnly );
    if ((msg->bit & 7) == 0)
    {
        if (msg->cursize >= msg->maxsize)
        {
            msg->overflowed = 1;
            return;
        }
        msg->bit = 8 * msg->cursize;
        msg->data[msg->cursize++] = 0;
    }
    ++msg->bit;
}

void __cdecl MSG_WriteBit1(msg_t *msg)
{
    int bit; // [esp+4h] [ebp-4h]

    iassert( !msg->readOnly );
    bit = msg->bit & 7;
    if (!bit)
    {
        if (msg->cursize >= msg->maxsize)
        {
            msg->overflowed = 1;
            return;
        }
        msg->bit = 8 * msg->cursize;
        msg->data[msg->cursize++] = 0;
    }
    msg->data[msg->bit++ >> 3] |= 1 << bit;
}

int __cdecl MSG_ReadBits(msg_t *msg, unsigned int bits)
{
    int bit; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int value; // [esp+8h] [ebp-4h]

    iassert( (unsigned)bits <= 32 );
    value = 0;
    for (i = 0; i < (int)bits; ++i)
    {
        bit = msg->bit & 7;
        if (!bit)
        {
            if (msg->readcount >= msg->splitSize + msg->cursize)
            {
                msg->overflowed = 1;
                return -1;
            }
            msg->bit = 8 * msg->readcount++;
        }
        value |= ((MSG_GetByte(msg, msg->bit >> 3) >> bit) & 1) << i;
        ++msg->bit;
    }
    return value;
}

int __cdecl MSG_GetByte(msg_t *msg, int where)
{
    if (where < msg->cursize)
        return msg->data[where];
    iassert( msg->splitData );
    return msg->splitData[where - msg->cursize];
}

int __cdecl MSG_ReadBit(msg_t *msg)
{
    int Byte; // eax
    int bit; // [esp+0h] [ebp-8h]

    bit = msg->bit & 7;
    if (!bit)
    {
        if (msg->readcount >= msg->splitSize + msg->cursize)
        {
            msg->overflowed = 1;
            return -1;
        }
        msg->bit = 8 * msg->readcount++;
    }
    Byte = MSG_GetByte(msg, msg->bit >> 3);
    ++msg->bit;
    return (Byte >> bit) & 1;
}

int __cdecl MSG_WriteBitsCompress(bool trainHuffman, const unsigned __int8 *from, unsigned __int8 *to, int size)
{
    int bit; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    if (trainHuffman)
    {
        for (i = 0; i < size; ++i)
            ++huffBytesSeen[from[i]];
    }
    bit = 0;
    i = size;
    while (i)
    {
        Huff_offsetTransmit(&msgHuff.compressDecompress, *from, to, &bit);
        --i;
        ++from;
    }
    return (bit + 7) >> 3;
}

int __cdecl MSG_ReadBitsCompress(const unsigned __int8 *from, unsigned __int8 *to, int size)
{
    int bit; // [esp+0h] [ebp-14h] BYREF
    unsigned __int8 *data; // [esp+4h] [ebp-10h]
    int bits; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int get; // [esp+10h] [ebp-4h] BYREF

    bits = 8 * size;
    i = 0;
    data = to;
    bit = 0;
    while (bit < bits)
    {
        Huff_offsetReceive(msgHuff.compressDecompress.tree, &get, from, &bit);
        *data++ = get;
    }
    return data - to;
}

void __cdecl MSG_WriteByte(msg_t *msg, unsigned __int8 c)
{
    iassert( !msg->readOnly );
    if (msg->cursize >= msg->maxsize)
        msg->overflowed = 1;
    else
        msg->data[msg->cursize++] = c;
}

void __cdecl MSG_WriteData(msg_t *buf, unsigned __int8 *data, unsigned int length)
{
    int newsize; // [esp+0h] [ebp-4h]

    iassert( !buf->readOnly );
    newsize = length + buf->cursize;
    if (newsize > buf->maxsize)
    {
        buf->overflowed = 1;
    }
    else
    {
        memcpy(&buf->data[buf->cursize], data, length);
        buf->cursize = newsize;
    }
}

void __cdecl MSG_WriteShort(msg_t *msg, __int16 c)
{
    int newsize; // [esp+0h] [ebp-4h]

    iassert( !msg->readOnly );
    newsize = msg->cursize + 2;
    if (newsize > msg->maxsize)
    {
        msg->overflowed = 1;
    }
    else
    {
        *(_WORD *)&msg->data[msg->cursize] = c;
        msg->cursize = newsize;
    }
}

void __cdecl MSG_WriteLong(msg_t *msg, int c)
{
    int newsize; // [esp+0h] [ebp-4h]

    iassert( !msg->readOnly );
    newsize = msg->cursize + 4;
    if (newsize > msg->maxsize)
    {
        msg->overflowed = 1;
    }
    else
    {
        *(unsigned int *)&msg->data[msg->cursize] = c;
        msg->cursize = newsize;
    }
}

void __cdecl MSG_WriteString(msg_t *sb, const char *s)
{
    unsigned __int8 v2; // al
    int l; // [esp+10h] [ebp-40Ch]
    char string[1024]; // [esp+14h] [ebp-408h] BYREF
    int i; // [esp+418h] [ebp-4h]

    iassert( s );
    iassert( !sb->readOnly );
    l = strlen(s);
    if (l < 1024)
    {
        for (i = 0; i < l; ++i)
        {
            v2 = I_CleanChar(s[i]);
            string[i] = v2;
        }
        string[i] = 0;
        MSG_WriteData(sb, (unsigned __int8 *)string, l + 1);
    }
    else
    {
        Com_Printf(16, "MSG_WriteString: MAX_STRING_CHARS");
        MSG_WriteData(sb, (unsigned __int8 *)"", 1u);
    }
}

void __cdecl MSG_WriteBigString(msg_t *sb, char *s)
{
    unsigned __int8 v2; // al
    int v3; // [esp+10h] [ebp-200Ch]
    char dest[8192]; // [esp+14h] [ebp-2008h] BYREF
    int i; // [esp+2018h] [ebp-4h]

    iassert( s );
    iassert( !sb->readOnly );
    v3 = strlen(s);
    if (v3 < 0x2000)
    {
        I_strncpyz(dest, s, 0x2000);
        for (i = 0; i < v3; ++i)
        {
            v2 = I_CleanChar(dest[i]);
            dest[i] = v2;
        }
        MSG_WriteData(sb, (unsigned __int8 *)dest, v3 + 1);
    }
    else
    {
        Com_Printf(16, "MSG_WriteString: BIG_INFO_STRING");
        MSG_WriteData(sb, (unsigned __int8 *)"", 1u);
    }
}

void __cdecl MSG_WriteAngle16(msg_t *sb, float f)
{
    iassert( !sb->readOnly );
    MSG_WriteShort(sb, (int)(f * 182.0444488525391));
}

int __cdecl MSG_ReadByte(msg_t *msg)
{
    int c; // [esp+0h] [ebp-4h]

    if (msg->readcount >= msg->splitSize + msg->cursize)
    {
        msg->overflowed = 1;
        return -1;
    }
    else
    {
        c = MSG_GetByte(msg, msg->readcount);
        if (c != (unsigned __int8)c)
            MyAssertHandler(
                ".\\qcommon\\msg_mp.cpp",
                509,
                0,
                "c == static_cast< byte >( c )\n\t%i, %i",
                c,
                (unsigned __int8)c);
        ++msg->readcount;
        return c;
    }
}

int __cdecl MSG_ReadShort(msg_t *msg)
{
    int where; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    int c; // [esp+8h] [ebp-Ch]
    int newcount; // [esp+Ch] [ebp-8h]
    __int16 read[2]; // [esp+10h] [ebp-4h]

    newcount = msg->readcount + 2;
    if (newcount > msg->splitSize + msg->cursize)
    {
        msg->overflowed = 1;
        return -1;
    }
    else
    {
        where = msg->readcount;
        for (i = 0; i < 2; ++i)
            *((_BYTE *)read + i) = MSG_GetByte(msg, where++);
        c = read[0];
        msg->readcount = newcount;
        return c;
    }
}

int __cdecl MSG_ReadLong(msg_t *msg)
{
    int where; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    int c; // [esp+8h] [ebp-8h]
    int newcount; // [esp+Ch] [ebp-4h]

    newcount = msg->readcount + 4;
    if (newcount > msg->splitSize + msg->cursize)
    {
        msg->overflowed = 1;
        return -1;
    }
    else
    {
        where = msg->readcount;
        for (i = 0; i < 4; ++i)
            *((_BYTE *)&c + i) = MSG_GetByte(msg, where++);
        msg->readcount = newcount;
        return c;
    }
}

static char string[1024];
char *__cdecl MSG_ReadString(msg_t *msg)
{
    int c; // [esp+0h] [ebp-8h]
    unsigned int l; // [esp+4h] [ebp-4h]

    for (l = 0; ; ++l)
    {
        c = MSG_ReadByte(msg);
        if (c == -1)
            c = 0;
        if (l < 0x400)
            string[l] = I_CleanChar(c);
        if (!c)
            break;
    }
    string[1023] = 0;
    return string;
}

static char bigstring[8192];
char *__cdecl MSG_ReadBigString(msg_t *msg)
{
    int c; // [esp+0h] [ebp-8h]
    unsigned int l; // [esp+4h] [ebp-4h]

    for (l = 0; ; ++l)
    {
        c = MSG_ReadByte(msg);
        if (c == 37)
        {
            c = 46;
        }
        else if (c == -1)
        {
            c = 0;
        }
        if (l < 0x2000)
            bigstring[l] = I_CleanChar(c);
        if (!c)
            break;
    }
    bigstring[8191] = 0;
    return bigstring;
}

static char stringread[1024];
char *__cdecl MSG_ReadStringLine(msg_t *msg)
{
    int c; // [esp+0h] [ebp-8h]
    unsigned int l; // [esp+4h] [ebp-4h]

    for (l = 0; ; ++l)
    {
        c = MSG_ReadByte(msg);
        if (c == 37)
        {
            c = 46;
        }
        else if (c == 10 || c == -1)
        {
            c = 0;
        }
        if (l < 0x400)
            stringread[l] = I_CleanChar(c);
        if (!c)
            break;
    }
    stringread[1023] = 0;
    return stringread;
}

double __cdecl MSG_ReadAngle16(msg_t *msg)
{
    return (float)((double)MSG_ReadShort(msg) * 0.0054931640625);
}

void __cdecl MSG_ReadData(msg_t *msg, unsigned __int8 *data, int len)
{
    int newcount; // [esp+0h] [ebp-8h]
    signed int cursize; // [esp+4h] [ebp-4h]

    newcount = len + msg->readcount;
    if (newcount > msg->cursize)
    {
        if (newcount > msg->splitSize + msg->cursize)
        {
            msg->overflowed = 1;
            memset(data, 0xFFu, len);
        }
        else
        {
            cursize = msg->cursize - msg->readcount;
            if (cursize > 0)
            {
                memcpy(data, &msg->data[msg->readcount], cursize);
                len -= cursize;
                data += cursize;
            }
            if (len > 0)
                memcpy(data, &msg->splitData[msg->readcount - msg->cursize], len);
            msg->readcount = newcount;
        }
    }
    else
    {
        memcpy(data, &msg->data[msg->readcount], len);
        msg->readcount = newcount;
    }
}

void __cdecl MSG_WriteDeltaKey(msg_t *msg, int key, int oldV, int newV, unsigned int bits)
{
    iassert( !msg->readOnly );
    if (oldV == newV)
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteBits(msg, key ^ newV, bits);
    }
}

unsigned int __cdecl MSG_ReadDeltaKey(msg_t *msg, int key, int oldV, unsigned int bits)
{
    if (MSG_ReadBit(msg))
        return kbitmask[bits] & key ^ MSG_ReadBits(msg, bits);
    else
        return oldV;
}

void __cdecl MSG_WriteKey(msg_t *msg, int key, int newV, unsigned int bits)
{
    iassert( !msg->readOnly );
    MSG_WriteBits(msg, key ^ newV, bits);
}

unsigned int __cdecl MSG_ReadKey(msg_t *msg, int key, unsigned int bits)
{
    return kbitmask[bits] & key ^ MSG_ReadBits(msg, bits);
}

void __cdecl MSG_WriteDeltaKeyByte(msg_t *msg, char key, char oldV, char newV)
{
    iassert( !msg->readOnly );
    if (oldV == newV)
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteByte(msg, key ^ newV);
    }
}

int __cdecl MSG_ReadDeltaKeyByte(msg_t *msg, unsigned __int8 key, int oldV)
{
    if (MSG_ReadBit(msg))
        return key ^ (unsigned __int8)MSG_ReadByte(msg);
    else
        return oldV;
}

void __cdecl MSG_WriteDeltaKeyShort(msg_t *msg, __int16 key, __int16 oldV, __int16 newV)
{
    iassert( !msg->readOnly );
    if (oldV == newV)
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteShort(msg, key ^ newV);
    }
}

int __cdecl MSG_ReadDeltaKeyShort(msg_t *msg, __int16 key, int oldV)
{
    if (MSG_ReadBit(msg))
        return key ^ (unsigned __int16)MSG_ReadShort(msg);
    else
        return oldV;
}

void __cdecl MSG_SetDefaultUserCmd(playerState_s *ps, usercmd_s *cmd)
{
    int i; // [esp+0h] [ebp-4h]

    cmd->serverTime = 0;
    cmd->buttons = 0;
    cmd->angles[0] = 0;
    cmd->angles[1] = 0;
    cmd->angles[2] = 0;
    *(unsigned int *)&cmd->weapon = 0;
    cmd->meleeChargeYaw = 0.0;
    *(unsigned int *)&cmd->meleeChargeDist = 0;
    cmd->weapon = ps->weapon;
    cmd->offHandIndex = ps->offHandIndex;
    for (i = 0; i < 2; ++i)
        cmd->angles[i] = (unsigned __int16)(int)((ps->viewangles[i] - ps->delta_angles[i]) * 182.0444488525391);
    if ((ps->otherFlags & 4) != 0)
    {
        if ((ps->eFlags & 8) != 0)
        {
            cmd->buttons |= 0x100u;
        }
        else if ((ps->eFlags & 4) != 0)
        {
            cmd->buttons |= 0x200u;
        }
        if (ps->leanf <= 0.0)
        {
            if (ps->leanf < 0.0)
                cmd->buttons |= 0x40u;
        }
        else
        {
            cmd->buttons |= 0x80u;
        }
        if (ps->fWeaponPosFrac != 0.0)
            cmd->buttons |= 0x800u;
        if ((ps->pm_flags & PMF_SPRINTING) != 0)
            cmd->buttons |= 2u;
    }
}

static int __cdecl MSG_HorMoveTo(int iForwardMove, int iRightMove)
{
    int iFlags; // [esp+0h] [ebp-4h]

    iFlags = 0;
    if (iForwardMove <= 10)
    {
        if (iForwardMove < -10)
            iFlags = 2;
    }
    else
    {
        iFlags = 1;
    }
    if (iRightMove > 10)
        return iFlags | 4;
    if (iRightMove < -10)
        return iFlags | 8;
    return iFlags;
}

void __cdecl MSG_HorMoveFrom(char iFlags, char *pForwardMove, char *pRightMove)
{
    if ((iFlags & 1) != 0)
    {
        *pForwardMove = 127;
    }
    else if ((iFlags & 2) != 0)
    {
        *pForwardMove = -127;
    }
    else
    {
        *pForwardMove = 0;
    }
    if ((iFlags & 4) != 0)
    {
        *pRightMove = 127;
    }
    else if ((iFlags & 8) != 0)
    {
        *pRightMove = -127;
    }
    else
    {
        *pRightMove = 0;
    }
}

void __cdecl MSG_WriteDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, const usercmd_s *to)
{
    int horToMove; // [esp+4h] [ebp-Ch]
    unsigned int delta; // [esp+8h] [ebp-8h]
    int horFromMove; // [esp+Ch] [ebp-4h]
    int keyb; // [esp+1Ch] [ebp+Ch]
    int keya; // [esp+1Ch] [ebp+Ch]

    iassert( !msg->readOnly );
    iassert( from->buttons < (1 << BUTTON_BIT_COUNT) );
    iassert( to->buttons < (1 << BUTTON_BIT_COUNT) );
    iassert( from->weapon < (1 << MAX_WEAPONS_BITS) );
    iassert( to->weapon < (1 << MAX_WEAPONS_BITS) );
    iassert( from->offHandIndex < (1 << MAX_WEAPONS_BITS) );
    iassert( to->offHandIndex < (1 << MAX_WEAPONS_BITS) );
    delta = to->serverTime - from->serverTime;
    if (delta >= 0x100)
    {
        MSG_WriteBit0(msg);
        MSG_WriteLong(msg, to->serverTime);
    }
    else
    {
        MSG_WriteBit1(msg);
        MSG_WriteByte(msg, delta);
    }
    horToMove = MSG_HorMoveTo(to->forwardmove, to->rightmove);
    horFromMove = MSG_HorMoveTo(from->forwardmove, from->rightmove);
    if (from->buttons >> 1 == to->buttons >> 1
        && from->weapon == to->weapon
        && from->offHandIndex == to->offHandIndex
        && from->angles[2] == to->angles[2]
        && to->meleeChargeYaw == from->meleeChargeYaw
        && from->meleeChargeDist == to->meleeChargeDist)
    {
        if (from->angles[0] == to->angles[0]
            && from->angles[1] == to->angles[1]
            && (from->buttons & 1) == (to->buttons & 1)
            && horFromMove == horToMove)
        {
            MSG_WriteKey(msg, key, 0, 1u);
        }
        else
        {
            MSG_WriteKey(msg, key, 1, 1u);
            MSG_WriteKey(msg, key, 0, 1u);
            keyb = to->serverTime ^ key;
            MSG_WriteKey(msg, keyb, to->buttons, 1u);
            MSG_WriteDeltaKeyShort(msg, keyb, from->angles[0], to->angles[0]);
            MSG_WriteDeltaKeyShort(msg, keyb, from->angles[1], to->angles[1]);
            MSG_WriteDeltaKey(msg, keyb, horFromMove, horToMove, 4u);
        }
    }
    else
    {
        MSG_WriteKey(msg, key, 1, 1u);
        MSG_WriteKey(msg, key, 1, 1u);
        MSG_WriteKey(msg, key, to->buttons, 1u);
        MSG_WriteDeltaKeyShort(msg, key, from->angles[0], to->angles[0]);
        MSG_WriteDeltaKeyShort(msg, key, from->angles[1], to->angles[1]);
        MSG_WriteDeltaKey(msg, key, horFromMove, horToMove, 4u);
        keya = to->serverTime ^ key;
        MSG_WriteDeltaKeyShort(msg, keya, from->angles[2], to->angles[2]);
        MSG_WriteDeltaKey(msg, keya, from->buttons >> 1, to->buttons >> 1, 0x14u);
        MSG_WriteDeltaKey(msg, keya, from->weapon, to->weapon, 7u);
        MSG_WriteDeltaKey(msg, keya, from->offHandIndex, to->offHandIndex, 7u);
        if ((to->buttons & 0x10000) != 0)
        {
            MSG_WriteDeltaKeyByte(msg, keya, from->selectedLocation[0], to->selectedLocation[0]);
            MSG_WriteDeltaKeyByte(msg, keya, from->selectedLocation[1], to->selectedLocation[1]);
        }
        if ((to->buttons & 4) != 0)
        {
            MSG_WriteDeltaKeyShort(
                msg,
                keya,
                (int)(from->meleeChargeYaw * 182.0444488525391),
                (int)(to->meleeChargeYaw * 182.0444488525391));
            MSG_WriteDeltaKey(msg, keya, from->meleeChargeDist, to->meleeChargeDist, 8u);
        }
    }
}

void __cdecl MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, usercmd_s *to)
{
    char horToMove; // [esp+Ch] [ebp-8h]
    char horToMovea; // [esp+Ch] [ebp-8h]
    int horFromMove; // [esp+10h] [ebp-4h]
    int horFromMovea; // [esp+10h] [ebp-4h]
    int keyb; // [esp+20h] [ebp+Ch]
    int keya; // [esp+20h] [ebp+Ch]

    iassert( from->buttons < (1 << BUTTON_BIT_COUNT) );
    iassert( from->weapon < (1 << MAX_WEAPONS_BITS) );
    iassert( from->offHandIndex < (1 << MAX_WEAPONS_BITS) );
    memcpy(to, from, sizeof(usercmd_s));
    if (MSG_ReadBit(msg))
        to->serverTime = from->serverTime + MSG_ReadByte(msg);
    else
        to->serverTime = MSG_ReadLong(msg);
    if (MSG_ReadKey(msg, key, 1u))
    {
        to->buttons &= ~1u;
        if (MSG_ReadKey(msg, key, 1u))
        {
            to->buttons |= MSG_ReadKey(msg, key, 1u);
            to->angles[0] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, key, from->angles[0]);
            to->angles[1] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, key, from->angles[1]);
            horFromMovea = MSG_HorMoveTo(from->forwardmove, from->rightmove);
            horToMovea = MSG_ReadDeltaKey(msg, key, horFromMovea, 4u);
            MSG_HorMoveFrom(horToMovea, &to->forwardmove, &to->rightmove);
            keya = to->serverTime ^ key;
            to->angles[2] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, keya, from->angles[2]);
            to->buttons &= 1u;
            to->buttons |= 2 * MSG_ReadDeltaKey(msg, keya, from->buttons >> 1, 0x14u);
            to->weapon = MSG_ReadDeltaKey(msg, keya, from->weapon, 7u);
            to->offHandIndex = MSG_ReadDeltaKey(msg, keya, from->offHandIndex, 7u);
            if ((to->buttons & 0x10000) != 0)
            {
                to->selectedLocation[0] = MSG_ReadDeltaKeyByte(msg, keya, from->selectedLocation[0]);
                to->selectedLocation[1] = MSG_ReadDeltaKeyByte(msg, keya, from->selectedLocation[1]);
            }
            if ((to->buttons & 4) != 0)
            {
                to->meleeChargeYaw = (double)MSG_ReadDeltaKeyShort(
                    msg,
                    keya,
                    (unsigned __int16)(int)(from->meleeChargeYaw * 182.0444488525391))
                    * 0.0054931640625;
                to->meleeChargeDist = MSG_ReadDeltaKey(msg, keya, from->meleeChargeDist, 8u);
            }
            if (to->buttons >= 0x200000)
            {
                Com_PrintError(15, "client sent an invalid buttons value %i\n", to->buttons);
                to->buttons = from->buttons;
            }
            if (to->weapon >= 0x80u)
            {
                Com_PrintError(15, "client sent an invalid weapon number %i\n", to->weapon);
                to->weapon = from->weapon;
            }
            if (to->offHandIndex >= 0x80u)
            {
                Com_PrintError(15, "client sent an invalid offhand index %i\n", to->offHandIndex);
                to->offHandIndex = from->offHandIndex;
            }
        }
        else
        {
            keyb = to->serverTime ^ key;
            to->buttons |= MSG_ReadKey(msg, keyb, 1u);
            to->angles[0] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, keyb, from->angles[0]);
            to->angles[1] = (unsigned __int16)MSG_ReadDeltaKeyShort(msg, keyb, from->angles[1]);
            horFromMove = MSG_HorMoveTo(from->forwardmove, from->rightmove);
            horToMove = MSG_ReadDeltaKey(msg, keyb, horFromMove, 4u);
            MSG_HorMoveFrom(horToMove, &to->forwardmove, &to->rightmove);
        }
    }
}

void __cdecl MSG_ClearLastReferencedEntity(msg_t *msg)
{
    msg->lastEntityRef = -1;
}

int __cdecl MSG_ReadEntityIndex(msg_t *msg, unsigned int indexBits)
{
    if (MSG_ReadBit(msg))
    {
        if (msg_printEntityNums->current.enabled)
            Com_Printf(16, "Entity num: 1 bit (inc)\n");
        ++msg->lastEntityRef;
    }
    else if (indexBits != 10 || MSG_ReadBit(msg))
    {
        if (msg_printEntityNums->current.enabled)
            Com_Printf(16, "Entity num: %i bits (full)\n", indexBits + 2);
        msg->lastEntityRef = MSG_ReadBits(msg, indexBits);
    }
    else
    {
        if (msg_printEntityNums->current.enabled)
            Com_Printf(16, "Entity num: %i bits (delta)\n", 6);
        msg->lastEntityRef += MSG_ReadBits(msg, 4u);
    }
    if (msg_printEntityNums->current.enabled)
        Com_Printf(16, "Read entity num %i\n", msg->lastEntityRef);
    if (msg->lastEntityRef < 0)
        MyAssertHandler(
            ".\\qcommon\\msg_mp.cpp",
            1309,
            0,
            "%s\n\t(msg->lastEntityRef) = %i",
            "(msg->lastEntityRef >= 0)",
            msg->lastEntityRef);
    return msg->lastEntityRef;
}

void __cdecl MSG_ReadDeltaField(
    msg_t *msg,
    int time,
    const char * const from,
    char *to,
    const NetField *field,
    int print,
    bool noXor)
{
    int Bit; // eax
    int Byte; // eax
    int Long; // eax
    int v10; // eax
    int v11; // eax
    int v12; // eax
    int v13; // eax
    int v14; // eax
    int DeltaTime; // eax
    int DeltaGroundEntity; // eax
    int DeltaEventParamField; // eax
    int v18; // eax
    double OriginFloat; // st7
    double OriginZFloat; // st7
    double Angle16; // st7
    double v22; // st7
    int v23; // eax
    int v24; // [esp+8h] [ebp-50h]
    int v25; // [esp+Ch] [ebp-4Ch]
    const hudelem_color_t *fromColor; // [esp+24h] [ebp-34h]
    hudelem_color_t *toColora; // [esp+28h] [ebp-30h]
    hudelem_color_t *toColor; // [esp+28h] [ebp-30h]
    int j; // [esp+2Ch] [ebp-2Ch]
    int zeroVal; // [esp+30h] [ebp-28h] BYREF
    int trunc; // [esp+34h] [ebp-24h]
    int rawValue; // [esp+38h] [ebp-20h]
    int *toF; // [esp+3Ch] [ebp-1Ch]
    int bits; // [esp+40h] [ebp-18h]
    const int *fromF; // [esp+44h] [ebp-14h]
    int mask; // [esp+48h] [ebp-10h]
    int sgn; // [esp+4Ch] [ebp-Ch]
    int partialBits; // [esp+50h] [ebp-8h]
    int value; // [esp+54h] [ebp-4h]

    zeroVal = 0;
    if (noXor)
        fromF = &zeroVal;
    else
        fromF = (const int *)&from[field->offset];
    toF = (int *)&to[field->offset];
    iassert( !msg->overflowed );
    if (field->changeHints != 2 && !MSG_ReadBit(msg))
    {
        *toF = *fromF;
        return;
    }
    iassert( !msg->overflowed );
    switch (field->bits)
    {
    case 0:
        if (MSG_ReadBit(msg))
        {
            if (MSG_ReadBit(msg))
            {
                Long = MSG_ReadLong(msg);
                *toF = Long;
                *toF ^= *fromF;
                if (print)
                    Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            }
            else
            {
                trunc = MSG_ReadBits(msg, 5u);
                Byte = MSG_ReadByte(msg);
                trunc += 32 * Byte;
                trunc ^= (int)*(float *)fromF + 4096;
                trunc -= 4096;
                *(float *)toF = (float)trunc;
                if (print)
                    Com_Printf(16, "%s:%i ", field->name, trunc);
            }
        }
        else
        {
            Bit = MSG_ReadBit(msg);
            *toF = Bit << 31;
            iassert( *reinterpret_cast< float * >( toF ) == 0.0f );
        }
        return;
    case 0xFFFFFFA7:
        if (MSG_ReadBit(msg))
        {
            v11 = MSG_ReadLong(msg);
            *toF = v11;
            *toF ^= *fromF;
            if (print)
                Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
        }
        else
        {
            trunc = MSG_ReadBits(msg, 5u);
            v10 = MSG_ReadByte(msg);
            trunc += 32 * v10;
            trunc ^= (int)*(float *)fromF + 4096;
            trunc -= 4096;
            *(float *)toF = (float)trunc;
            if (print)
                Com_Printf(16, "%s:%i ", field->name, trunc);
        }
        return;
    case 0xFFFFFFA8:
        v12 = MSG_ReadLong(msg);
        *toF = v12;
        *toF ^= *fromF;
        if (print)
            Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
        return;
    case 0xFFFFFF9D:
        if (MSG_ReadBit(msg))
        {
            if (MSG_ReadBit(msg))
            {
                value = MSG_ReadLong(msg);
                value ^= *fromF;
                *toF = value;
                if (print)
                    Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
            }
            else
            {
                trunc = MSG_ReadBits(msg, 4u);
                v13 = MSG_ReadByte(msg);
                trunc += 16 * v13;
                trunc ^= (int)*(float *)fromF + 2048;
                trunc -= 2048;
                *(float *)toF = (float)trunc;
                if (print)
                    Com_Printf(16, "%s:%i ", field->name, trunc);
            }
        }
        else
        {
            *toF = 0;
            iassert( *reinterpret_cast< float * >( toF ) == 0.0f );
        }
        if ((unsigned int)(__int64)(*(float *)toF + 2048.0) >= 0x1000)
            MyAssertHandler(
                ".\\qcommon\\msg_mp.cpp",
                1476,
                0,
                "*(float *)toF + HUDELEM_COORD_BIAS doesn't index 1 << HUDELEM_COORD_BITS\n\t%i not in [0, %i)",
                (int)(*(float *)toF + 2048.0),
                4096);
        return;
    case 0xFFFFFF9E:
        v14 = MSG_Read24BitFlag(msg, *fromF);
        *toF = v14;
        return;
    case 0xFFFFFF9F:
        DeltaTime = MSG_ReadDeltaTime(msg, time);
        *toF = DeltaTime;
        return;
    case 0xFFFFFFA0:
        DeltaGroundEntity = MSG_ReadDeltaGroundEntity(msg);
        *toF = DeltaGroundEntity;
        return;
    case 0xFFFFFFA2:
    case 0xFFFFFFA3:
        DeltaEventParamField = MSG_ReadDeltaEventParamField(msg);
        *toF = DeltaEventParamField;
        return;
    case 0xFFFFFFA1:
        v18 = MSG_ReadBits(msg, 7u);
        *toF = 100 * v18;
        return;
    case 0xFFFFFFA4:
    case 0xFFFFFFA5:
        OriginFloat = MSG_ReadOriginFloat(field->bits, msg, *(float *)fromF);
        *(float *)toF = OriginFloat;
        if (print)
            Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
        return;
    case 0xFFFFFFA6:
        OriginZFloat = MSG_ReadOriginZFloat(msg, *(float *)fromF);
        *(float *)toF = OriginZFloat;
        if (print)
            Com_Printf(16, "%s:%f ", field->name, *(float *)toF);
        return;
    case 0xFFFFFF9C:
        if (!MSG_ReadBit(msg))
        {
            *(float *)toF = 0.0;
            return;
        }
        goto LABEL_74;
    case 0xFFFFFFA9:
    LABEL_74:
        Angle16 = MSG_ReadAngle16(msg);
        *(float *)toF = Angle16;
        return;
    case 0xFFFFFFAA:
        v22 = (double)MSG_ReadBits(msg, 5u) * 1.0 / 10.0 + 1.399999976158142;
        *(float *)toF = v22;
        break;
    case 0xFFFFFFAB:
        if (MSG_ReadBit(msg))
        {
            fromColor = (const hudelem_color_t *)fromF;
            toColora = (hudelem_color_t *)toF;
            *toF = *fromF;
            toColora->a = fromColor->a != 0 ? 0 : -1;
        }
        else
        {
            toColor = (hudelem_color_t *)toF;
            if (!MSG_ReadBit(msg))
            {
                toColor->r = MSG_ReadByte(msg);
                toColor->g = MSG_ReadByte(msg);
                toColor->b = MSG_ReadByte(msg);
            }
            toColor->a = 8 * MSG_ReadBits(msg, 5u);
        }
        break;
    default:
        if (MSG_ReadBit(msg))
        {
            sgn = field->bits < 0;
            if (sgn)
                v25 = -field->bits;
            else
                v25 = field->bits;
            bits = v25;
            partialBits = v25 & 7;
            if ((v25 & 7) != 0)
                v24 = MSG_ReadBits(msg, partialBits);
            else
                v24 = 0;
            rawValue = v24;
            for (j = partialBits; j < bits; j += 8)
            {
                v23 = MSG_ReadByte(msg);
                rawValue |= v23 << j;
            }
            iassert( (bits <= 32) );
            if (bits == 32)
                mask = -1;
            else
                mask = (1 << bits) - 1;
            value = rawValue ^ mask & *fromF;
            if (sgn && (value & (1 << (bits - 1))) != 0)
                value |= ~mask;
            if (print)
                Com_Printf(16, "%s:%i ", field->name, *toF);
            *toF = value;
        }
        else
        {
            *toF = 0;
        }
        break;
    }
}

int __cdecl MSG_ReadDeltaTime(msg_t *msg, int timeBase)
{
    if (MSG_ReadBit(msg))
        return MSG_ReadLong(msg);
    else
        return timeBase - MSG_ReadBits(msg, 8u);
}

int __cdecl MSG_ReadDeltaGroundEntity(msg_t *msg)
{
    int j; // [esp+4h] [ebp-10h]
    int value; // [esp+10h] [ebp-4h]

    if (MSG_ReadBit(msg) == 1)
        return ENTITYNUM_WORLD;
    if (MSG_ReadBit(msg) == 1)
        return 0;
    value = MSG_ReadBits(msg, 2u);
    for (j = 2; j < 10; j += 8)
        value |= MSG_ReadByte(msg) << j;
    return value;
}

int __cdecl MSG_ReadDeltaEventParamField(msg_t *msg)
{
    return MSG_ReadByte(msg);
}

int __cdecl MSG_Read24BitFlag(msg_t *msg, int oldFlags)
{
    unsigned int bitChanged; // [esp+0h] [ebp-10h]
    int j; // [esp+4h] [ebp-Ch]
    int value; // [esp+Ch] [ebp-4h]

    iassert( !msg->overflowed );
    if (MSG_ReadBit(msg) == 1)
    {
        value = 0;
        for (j = 0; j < 24; j += 8)
            value |= MSG_ReadByte(msg) << j;
    }
    else
    {
        bitChanged = MSG_ReadBits(msg, 5u);
        if (bitChanged > 0x18)
            MyAssertHandler(
                ".\\qcommon\\msg_mp.cpp",
                1189,
                0,
                "%s\n\t(bitChanged) = %i",
                "(bitChanged >= 0 && bitChanged <= 24)",
                bitChanged);
        return oldFlags ^ (1 << bitChanged);
    }
    return value;
}

double __cdecl MSG_ReadOriginFloat(int bits, msg_t *msg, float oldValue)
{
    int roundedCenter; // [esp+8h] [ebp-14h]
    int index; // [esp+Ch] [ebp-10h]

    if (MSG_ReadBit(msg))
    {
        if (bits == -92)
        {
            index = 0;
        }
        else
        {
            iassert( bits == MSG_FIELD_ORIGINY );
            index = 1;
        }
        roundedCenter = (int)((*CL_GetMapCenter())[index] + 0.5);
        return (float)(roundedCenter + (((int)oldValue + 0x8000 - roundedCenter) ^ MSG_ReadBits(msg, 0x10u)) - 0x8000);
    }
    else
    {
        return (float)((double)(MSG_ReadBits(msg, 7u) - 64) + oldValue);
    }
}

double __cdecl MSG_ReadOriginZFloat(msg_t *msg, float oldValue)
{
    int roundedCenter; // [esp+8h] [ebp-10h]

    if (MSG_ReadBit(msg))
    {
        roundedCenter = (int)((*CL_GetMapCenter())[2] + 0.5);
        return (float)(roundedCenter + (((int)oldValue + 0x8000 - roundedCenter) ^ MSG_ReadBits(msg, 0x10u)) - 0x8000);
    }
    else
    {
        return (float)((double)(MSG_ReadBits(msg, 7u) - 64) + oldValue);
    }
}

int __cdecl MSG_ReadDeltaEntity(msg_t *msg, int time, entityState_s *from, entityState_s *to, unsigned int number)
{
    return MSG_ReadDeltaEntityStruct(msg, time, (char *)from, (char *)to, number);
}

int __cdecl MSG_ReadDeltaEntityStruct(msg_t *msg, int time, char *from, char *to, unsigned int number)
{
    char *EntityTypeName; // eax
    const NetFieldList *stateFieldList; // [esp+38h] [ebp-20h]
    int print; // [esp+3Ch] [ebp-1Ch]
    const NetField *field; // [esp+40h] [ebp-18h]
    const NetField *fielda; // [esp+40h] [ebp-18h]
    unsigned int lc; // [esp+44h] [ebp-14h]
    const NetField *stateFields; // [esp+48h] [ebp-10h]
    unsigned int i; // [esp+54h] [ebp-4h]
    unsigned int ia; // [esp+54h] [ebp-4h]

    iassert( number < (1 << GENTITYNUM_BITS) );
    if (MSG_ReadBit(msg) == 1)
    {
        if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1))
            Com_Printf(16, "%3i: #%-3i remove\n", msg->readcount, number);
        return 1;
    }
    else if (MSG_ReadBit(msg))
    {
        lc = MSG_ReadLastChangedField(msg, 61);
        if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1))
        {
            print = 1;
            Com_Printf(16, "%3i: #%-3i ", msg->readcount, *(unsigned int *)to);
        }
        else
        {
            print = 0;
        }
        *(unsigned int *)to = number;
        if (strcmp(entityStateFields[0].name, "eType"))
            MyAssertHandler(".\\qcommon\\msg_mp.cpp", 1763, 0, "%s", "strcmp( entityStateFields[0].name, \"eType\" ) == 0");
        MSG_ReadDeltaField(msg, time, from, to, entityStateFields, print, 0);
        stateFieldList = MSG_GetStateFieldListForEntityType(*((unsigned int *)to + 1));
        stateFields = stateFieldList->array;
        if (lc <= stateFieldList->count)
        {
            if (msg_dumpEnts->current.enabled)
            {
                EntityTypeName = BG_GetEntityTypeName(*((unsigned int *)to + 1));
                Com_Printf(14, "%3i: changed ent, eType %s\n", number, EntityTypeName);
            }
            if (strcmp(stateFields->name, "eType"))
                MyAssertHandler(".\\qcommon\\msg_mp.cpp", 1782, 0, "%s", "strcmp( stateFields[0].name, \"eType\" ) == 0");
            i = 1;
            field = stateFields + 1;
            while (i < lc)
            {
                MSG_ReadDeltaField(msg, time, from, to, field, print, 0);
                ++i;
                ++field;
            }
            ia = lc;
            fielda = &stateFields[lc];
            while (ia < stateFieldList->count)
            {
                *(unsigned int *)&to[fielda->offset] = *(unsigned int *)&from[fielda->offset];
                ++ia;
                ++fielda;
            }
            return 0;
        }
        else
        {
            msg->overflowed = 1;
            return 0;
        }
    }
    else
    {
        memcpy(to, from, 0xF4u);
        return 0;
    }
}

int __cdecl MSG_ReadLastChangedField(msg_t *msg, int totalFields)
{
    const char *v2; // eax
    int lastChanged; // [esp+0h] [ebp-8h]
    unsigned int idealBits; // [esp+4h] [ebp-4h]

    idealBits = GetMinBitCountForNum(totalFields);
    lastChanged = MSG_ReadBits(msg, idealBits);
    if (lastChanged > totalFields)
    {
        v2 = va("lastChanged was %i, totalFields is %i\n", lastChanged, totalFields);
        MyAssertHandler(".\\qcommon\\msg_mp.cpp", 1321, 0, "%s\n\t%s", "lastChanged <= totalFields", v2);
    }
    return lastChanged;
}


const int numEntityStateFields = 59;
const int numEventEntityStateFields = 59;
const int numPlayerEntityStateFields = 59;
const int numCorpseEntityStateFields = 59;
const int numVehicleEntityStateFields = 59;
const int numItemEntityStateFields = 59;
const int numSoundBlendEntityStateFields = 59;
const int numLoopFxEntityStateFields = 59;
const int numMissileEntityStateFields = 59;
const int numArchivedEntityFields = 69;
const int numClientStateFields = 24;
const int numPlayerStateFields = 141;
const int numObjectiveFields = 6;
const int numHudElemFields = 40;


int __cdecl MSG_ReadDeltaArchivedEntity(
    msg_t *msg,
    int time,
    archivedEntity_s *from,
    archivedEntity_s *to,
    unsigned int number)
{
    return MSG_ReadDeltaStruct(
        msg,
        time,
        (char *)from,
        (char *)to,
        number,
        numArchivedEntityFields,
        10,
        archivedEntityFields,
        numArchivedEntityFields);
}

int __cdecl MSG_ReadDeltaStruct(
    msg_t *msg,
    int time,
    char *from,
    char *to,
    unsigned int number,
    int numFields,
    char indexBits,
    const NetField *stateFields,
    int totalFields)
{
    int print; // [esp+0h] [ebp-18h]
    const NetField *field; // [esp+4h] [ebp-14h]
    const NetField *fielda; // [esp+4h] [ebp-14h]
    int lc; // [esp+8h] [ebp-10h]
    int i; // [esp+14h] [ebp-4h]
    int ia; // [esp+14h] [ebp-4h]

    iassert( number < (1u << indexBits) );
    if (MSG_ReadBit(msg) == 1)
    {
        if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1))
            Com_Printf(16, "%3i: #%-3i remove\n", msg->readcount, number);
        return 1;
    }
    else if (MSG_ReadBit(msg))
    {
        lc = MSG_ReadLastChangedField(msg, totalFields);
        if (lc <= numFields)
        {
            if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1))
            {
                print = 1;
                Com_Printf(16, "%3i: #%-3i ", msg->readcount, *(unsigned int *)to);
            }
            else
            {
                print = 0;
            }
            *(unsigned int *)to = number;
            i = 0;
            field = stateFields;
            while (i < lc)
            {
                MSG_ReadDeltaField(msg, time, from, to, field, print, 0);
                ++i;
                ++field;
            }
            ia = lc;
            fielda = &stateFields[lc];
            while (ia < numFields)
            {
                *(unsigned int *)&to[fielda->offset] = *(unsigned int *)&from[fielda->offset];
                ++ia;
                ++fielda;
            }
            return 0;
        }
        else
        {
            msg->overflowed = 1;
            return 0;
        }
    }
    else
    {
        memcpy((unsigned __int8 *)to, (unsigned __int8 *)from, 4 * numFields + 4);
        return 0;
    }
}

int __cdecl MSG_ReadDeltaClient(msg_t *msg, int time, clientState_s *from, clientState_s *to, unsigned int number)
{
    clientState_s dummy; // [esp+4h] [ebp-70h] BYREF

    if (!from)
    {
        from = &dummy;
        memset((unsigned __int8 *)&dummy, 0, sizeof(dummy));
    }
    return MSG_ReadDeltaStruct(
        msg,
        time,
        (char *)from,
        (char *)to,
        number,
        numClientStateFields,
        6,
        clientStateFields,
        numClientStateFields);
}

static void __cdecl MSG_ReadDeltaFields(
    msg_t *msg,
    int time,
    const char *const from,
    char *to,
    int numFields,
    const NetField *stateFields)
{
    if (MSG_ReadBit(msg))
    {
        for (int i = 0; i < numFields; ++i)
            MSG_ReadDeltaField(msg, time, from, to, &stateFields[i], 0, 0);
    }
    // LWSS: redundant since the caller does memcpy(to, from) on the entire block
    //else
    //{
    //    for (int i = 0; i < numFields; ++i)
    //        *(unsigned int *)&to[stateFields[i].offset] = *(unsigned int *)&from[stateFields[i].offset];
    //}
}

static void __cdecl MSG_ReadDeltaHudElems(msg_t *msg, int time, const hudelem_s *from, hudelem_s *to, int count)
{
    unsigned int j; // [esp+8h] [ebp-18h]
    unsigned int lc; // [esp+Ch] [ebp-14h]

    if (count != 31)
        MyAssertHandler(
            ".\\qcommon\\msg_mp.cpp",
            1884,
            0,
            "%s",
            "count == MAX_HUDELEMS_ARCHIVAL || count == MAX_HUDELEMS_CURRENT");

    int inuse = MSG_ReadBits(msg, 5u);

    for (int i = 0; i < inuse; ++i)
    {
        lc = MSG_ReadBits(msg, 6);

        for (j = 0; j <= lc; ++j)
            MSG_ReadDeltaField(msg, time, (const char *)&from[i], (char *)&to[i], &hudElemFields[j], 0, 0);

        // Redundant since caller does memcpy(to, from)
        //while (j < numHudElemFields)
        //{
        //    *(&to[i].type + hudElemFields[j].offset) = *(&from[i].type + hudElemFields[j].offset);
        //    ++j;
        //}

        iassert(!(from[i].alignOrg & ~15));
        iassert(!(to[i].alignOrg & ~15));

        {
            int alignX = ((from[i].alignOrg >> 2) & 3);
            int alignY = (from[i].alignOrg & 3);
            iassert(alignX == 0 || alignX == 1 || alignX == 2);
            iassert(alignY == 0 || alignY == 1 || alignY == 2);
        }
        {
            int alignX = ((to[i].alignOrg >> 2) & 3);
            int alignY = (to[i].alignOrg & 3);
            iassert(alignX == 0 || alignX == 1 || alignX == 2);
            iassert(alignY == 0 || alignY == 1 || alignY == 2);
        }
    }

    while (inuse < count && to[inuse].type)
    {
        memset(&to[inuse], 0, sizeof(hudelem_s));
        iassert(to[inuse].type == HE_TYPE_FREE);
        ++inuse;
    }
}

void __cdecl MSG_ReadDeltaPlayerstate(
    int localClientNum,
    msg_t *msg,
    int time,
    const playerState_s *from,
    playerState_s *to,
    bool predictedFieldsIgnoreXor)
{
    int Short; // eax
    int v7; // eax
    objectiveState_t v8; // eax
    unsigned __int8 Byte; // al
    clientActive_t *LocalClientGlobals; // [esp+1Ch] [ebp-2F9Ch]
    int i; // [esp+20h] [ebp-2F98h]
    int k; // [esp+20h] [ebp-2F98h]
    int print; // [esp+24h] [ebp-2F94h]
    int LastChangedField; // [esp+30h] [ebp-2F88h]
    int Bits; // [esp+2FA8h] [ebp-10h]
    int *v19; // [esp+2FACh] [ebp-Ch]
    bool lc; // [esp+2FB3h] [ebp-5h]

    unsigned __int8 dst[sizeof(playerState_s) + 8]; // [esp+38h] [ebp-2F80h] BYREF

    if (!from)
    {
        from = (playerState_s *)dst;
        memset(dst, 0, sizeof(playerState_s));
    }

    // Copy entire `from` into `to`
    memcpy(to, from, sizeof(playerState_s));

    if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -2))
    {
        print = 1;
        Com_Printf(16, "%3i: playerstate ", msg->readcount);
    }
    else
    {
        print = 0;
    }

    lc = MSG_ReadBit(msg) > 0;
    LastChangedField = MSG_ReadLastChangedField(msg, numPlayerStateFields);

    {
        int itr = 0;
        NetField *field = (NetField *)playerStateFields;
        while (itr < LastChangedField)
        {
            iassert(!msg->overflowed);

            if (predictedFieldsIgnoreXor && lc && field->changeHints == 3)
                MSG_ReadDeltaField(msg, time, (const char *)from, (char *)to, field, print, 1);
            else
                MSG_ReadDeltaField(msg, time, (const char *)from, (char *)to, field, print, 0);

            iassert(!msg->overflowed);
            ++itr;
            ++field;
        }
    }

    // LWSS: this is made redundant via the memcpy at the start (#36)
    //{
    //    int itr = LastChangedField;
    //    NetField *field = (NetField *)&playerStateFields[LastChangedField];
    //    while (itr < numPlayerStateFields)
    //    {
    //        v19 = (int *)((char *)&from->commandTime + field->offset);
    //        *(int *)((char *)&to->commandTime + field->offset) = *v19;
    //        ++itr;
    //        ++field;
    //    }
    //}


    if (!lc)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);

        if (!CL_GetPredictedOriginForServerTime(
            LocalClientGlobals,
            to->commandTime,
            to->origin,
            to->velocity,
            to->viewangles,
            &to->bobCycle,
            &to->movementDir))
        {
            Com_PrintError(14, "Unable to find the origin we sent, delta is not going to work");
            // LWSS: if the above function returns false, there is no data set in `to`. 
            // The below copy code is redundant via the memcpy at the top (#36)
            //to->origin[0] = from->origin[0];
            //to->origin[1] = from->origin[1];
            //to->origin[2] = from->origin[2];
            //to->velocity[0] = from->velocity[0];
            //to->velocity[1] = from->velocity[1];
            //to->velocity[2] = from->velocity[2];
            //to->bobCycle = from->bobCycle;
            //to->movementDir = from->movementDir;
            //to->viewangles[0] = from->viewangles[0];
            //to->viewangles[1] = from->viewangles[1];
            //to->viewangles[2] = from->viewangles[2];
        }
    }

    if (MSG_ReadBit(msg))
    {
        if (cl_shownet && cl_shownet->current.integer == 4)
            Com_Printf(16, "%s ", "PS_STATS");
        Bits = MSG_ReadBits(msg, 5u);
        if ((Bits & 1) != 0)
            to->stats[0] = MSG_ReadShort(msg);
        if ((Bits & 2) != 0)
            to->stats[1] = MSG_ReadShort(msg);
        if ((Bits & 4) != 0)
            to->stats[2] = MSG_ReadShort(msg);
        if ((Bits & 8) != 0)
            to->stats[3] = MSG_ReadBits(msg, 6u);
        if ((Bits & 0x10) != 0)
            to->stats[4] = MSG_ReadByte(msg);
    }

    if (MSG_ReadBit(msg))
    {
        for (i = 0; i < 4; ++i)
        {
            if (MSG_ReadBit(msg))
            {
                if (cl_shownet && cl_shownet->current.integer == 4)
                    Com_Printf(16, "%s ", "PS_AMMO");
                Bits = MSG_ReadShort(msg);
                for (int j = 0; j < 16; ++j)
                {
                    if ((Bits & (1 << j)) != 0)
                    {
                        Short = MSG_ReadShort(msg);
                        to->ammo[16 * i + j] = Short;
                    }
                }
            }
        }
    }

    for (k = 0; k < 8; ++k)
    {
        if (MSG_ReadBit(msg))
        {
            if (cl_shownet && cl_shownet->current.integer == 4)
                Com_Printf(16, "%s ", "PS_AMMOCLIP");
            Bits = MSG_ReadShort(msg);
            for (int j = 0; j < 16; ++j)
            {
                if ((Bits & (1 << j)) != 0)
                {
                    v7 = MSG_ReadShort(msg);
                    to->ammoclip[16 * k + j] = v7;
                }
            }
        }
    }

    if (MSG_ReadBit(msg))
    {
        for (int j = 0; j < 16; ++j)
        {
            to->objective[j].state = (objectiveState_t)MSG_ReadBits(msg, 3);
            MSG_ReadDeltaFields(
                msg,
                time,
                (const char *)&from->objective[j],
                (char *)&to->objective[j],
                numObjectiveFields,
                objectiveFields);
        }
    }

    if (MSG_ReadBit(msg))
    {
        MSG_ReadDeltaHudElems(msg, time, from->hud.archival, to->hud.archival, 31);
        MSG_ReadDeltaHudElems(msg, time, from->hud.current, to->hud.current, 31);
    }

    if (MSG_ReadBit(msg))
    {
        for (int j = 0; j < 128; ++j)
        {
            Byte = MSG_ReadByte(msg);
            to->weaponmodels[j] = Byte;
        }
    }
}

void __cdecl MSG_InitHuffman()
{
    msgInit = 1;
    MSG_initHuffmanInternal();
}

const int msg_hData[256] =
{
  274054,
  68777,
  40460,
  40266,
  48059,
  39006,
  48630,
  27692,
  17712,
  15439,
  12386,
  10758,
  9420,
  9979,
  9346,
  15256,
  13184,
  14319,
  7750,
  7221,
  6095,
  5666,
  12606,
  7263,
  7322,
  5807,
  11628,
  6199,
  7826,
  6349,
  7698,
  9656,
  28968,
  5164,
  13629,
  6058,
  4745,
  4519,
  5199,
  4807,
  5323,
  3433,
  3455,
  3563,
  6979,
  5229,
  5002,
  4423,
  14108,
  13631,
  11908,
  11801,
  10261,
  7635,
  7215,
  7218,
  9353,
  6161,
  5689,
  4649,
  5026,
  5866,
  8002,
  10534,
  15381,
  8874,
  11798,
  7199,
  12814,
  6103,
  4982,
  5972,
  6779,
  4929,
  5333,
  3503,
  4345,
  6098,
  14117,
  16440,
  6446,
  3062,
  4695,
  3085,
  4198,
  4013,
  3878,
  3414,
  5514,
  4092,
  3261,
  4740,
  4544,
  3127,
  3385,
  7688,
  11126,
  6417,
  5297,
  4529,
  6333,
  4210,
  7056,
  4658,
  6190,
  3512,
  2843,
  3479,
  9369,
  5203,
  4980,
  5881,
  7509,
  4292,
  6097,
  5492,
  4648,
  2996,
  4988,
  4163,
  6534,
  4001,
  4342,
  4488,
  6039,
  4827,
  7112,
  8654,
  26712,
  8688,
  9677,
  9368,
  7209,
  3399,
  4473,
  4677,
  11087,
  4094,
  3404,
  4176,
  6733,
  3702,
  11420,
  4867,
  5968,
  3475,
  3722,
  3560,
  4571,
  2720,
  3189,
  3099,
  4595,
  4044,
  4402,
  3889,
  4989,
  3186,
  3153,
  5387,
  8020,
  3322,
  3775,
  2886,
  4191,
  2879,
  3110,
  2576,
  3693,
  2436,
  4935,
  3017,
  3538,
  5688,
  3444,
  3410,
  9170,
  4708,
  3425,
  3273,
  3684,
  4564,
  6957,
  4817,
  5224,
  3285,
  3143,
  4227,
  5630,
  6053,
  5851,
  6507,
  13692,
  8270,
  8260,
  5583,
  7568,
  4082,
  3984,
  4574,
  6440,
  3533,
  2992,
  2708,
  5190,
  3889,
  3799,
  4582,
  6020,
  3464,
  4431,
  3495,
  2906,
  2243,
  3856,
  3321,
  8759,
  3928,
  2905,
  3875,
  4382,
  3885,
  5869,
  6235,
  10685,
  4433,
  4639,
  4305,
  4683,
  2849,
  3379,
  4683,
  5477,
  4127,
  3853,
  3515,
  4913,
  3601,
  5237,
  6617,
  9019,
  4857,
  4112,
  5180,
  5998,
  4925,
  4986,
  6365,
  7930,
  5948,
  8085,
  7732,
  8643,
  8901,
  9653,
  32647
};



void MSG_initHuffmanInternal()
{
    DWORD time2; // [esp+0h] [ebp-8h]
    DWORD time; // [esp+4h] [ebp-4h]

    Huff_Init(&msgHuff);
    time = Sys_Milliseconds();
    Huff_BuildFromData(&msgHuff.compressDecompress, msg_hData);
    time2 = Sys_Milliseconds();
    Com_Printf(16, "Huffman Took %d Milliseconds\n", time2 - time);
}

void __cdecl MSG_DumpNetFieldChanges_f()
{
    int arraySize[6]; // [esp+0h] [ebp-58h]
    const int *array; // [esp+18h] [ebp-40h]
    int iSize; // [esp+1Ch] [ebp-3Ch]
    const char *arrayNames[6]; // [esp+20h] [ebp-38h]
    int i; // [esp+38h] [ebp-20h]
    unsigned int iArrayNum; // [esp+3Ch] [ebp-1Ch]
    const int *changeArray[6]; // [esp+40h] [ebp-18h]

    changeArray[0] = (const int *)&orderInfo;
    changeArray[1] = orderInfo.arcEntState;
    changeArray[2] = orderInfo.clientState;
    changeArray[3] = orderInfo.playerState;
    changeArray[4] = orderInfo.objective;
    changeArray[5] = orderInfo.hudElem;
    arraySize[0] = 64;
    arraySize[1] = 128;
    arraySize[2] = 32;
    arraySize[3] = 160;
    arraySize[4] = 8;
    arraySize[5] = 40;
    arrayNames[0] = "Entity State";
    arrayNames[1] = "Archived Entity State";
    arrayNames[2] = "Client State";
    arrayNames[3] = "Player State";
    arrayNames[4] = "Objective";
    arrayNames[5] = "HUD Elem";
    Com_Printf(0, "========================================\n");
    Com_Printf(0, "NetField changes. format: field# : #changes\n");
    for (iArrayNum = 0; iArrayNum < 6; ++iArrayNum)
    {
        Com_Printf(0, "========================================\n");
        Com_Printf(0, "    %s\n", arrayNames[iArrayNum]);
        Com_Printf(0, "--------------------\n");
        array = changeArray[iArrayNum];
        iSize = arraySize[iArrayNum];
        for (i = 0; i < iSize; ++i)
        {
            if (array[i])
                Com_Printf(0, "%3i :%8i\n", i, array[i]);
        }
    }
    Com_Printf(0, "========================================\n");
    Com_Printf(0, "========================================\n");
}

