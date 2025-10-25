#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "scr_readwrite.h"
#include "scr_main.h"
#include "scr_memorytree.h"
#include "scr_vm.h"
#include "scr_parsetree.h"
#include <game/savedevice.h>

static unsigned int g_idHistoryIndex;
static short idHistory[16];

void __cdecl WriteByte(
    unsigned __int8 b,
    MemoryFile *memFile)
{
    MemFile_WriteData(memFile, 1, &b);
}

void __cdecl WriteShort(unsigned __int16 i, MemoryFile *memFile)
{
    MemFile_WriteData(memFile, 2, &i);
}

void __cdecl WriteString(unsigned __int16 str, MemoryFile *memFile)
{
    const char *v3; // r3

    v3 = SL_ConvertToString(str);
    MemFile_WriteCString(memFile, v3);
}

void __cdecl SafeWriteString(unsigned __int16 str, MemoryFile *memFile)
{
    unsigned int v3; // r30
    const char *v4; // r3
    _BYTE v5[8]; // [sp+50h] [-20h] BYREF

    v3 = str;
    if (str)
    {
        v5[0] = 1;
        MemFile_WriteData(memFile, 1, v5);
        v4 = SL_ConvertToString(v3);
        MemFile_WriteCString(memFile, v4);
    }
    else
    {
        v5[0] = 0;
        MemFile_WriteData(memFile, 1, v5);
    }
}

int __cdecl Scr_ReadString(MemoryFile *memFile)
{
    const char *CString; // r3
    const char *v2; // r11

    CString = MemFile_ReadCString(memFile);
    v2 = CString;
    while (*(unsigned __int8 *)v2++)
        ;
    return (unsigned __int16)SL_GetStringOfSize(CString, 0, v2 - CString, 15);
}

int __cdecl Scr_ReadOptionalString(MemoryFile *memFile)
{
    const char *CString; // r3
    const char *v4; // r11
    _BYTE v6[16]; // [sp+50h] [-20h] BYREF

    MemFile_ReadData(memFile, 1, v6);
    if (!v6[0])
        return 0;
    CString = MemFile_ReadCString(memFile);
    v4 = CString;
    while (*(unsigned __int8 *)v4++)
        ;
    return (unsigned __int16)SL_GetStringOfSize(CString, 0, v4 - CString, 15);
}

void __cdecl WriteInt(int i, MemoryFile *memFile)
{
    MemFile_WriteData(memFile, 4, &i);
}

void WriteFloat(float f, MemoryFile *memFile)
{
    iassert(!IS_NAN(f));
    MemFile_WriteData(memFile, 4, &f);
}

void __cdecl WriteVector(float *v, MemoryFile *memFile)
{
    iassert(v);
    iassert(!IS_NAN(v[0]) && !IS_NAN(v[1]) && !IS_NAN(v[2]));
    
    WriteFloat(v[0], memFile);
    WriteFloat(v[1], memFile);
    WriteFloat(v[2], memFile);
}

const float *__cdecl Scr_ReadVec3(MemoryFile *memFile)
{
    double v2; // fp0
    float v4; // [sp+50h] [-50h] BYREF
    float v5; // [sp+54h] [-4Ch]
    float v6; // [sp+58h] [-48h] BYREF
    float v7; // [sp+5Ch] [-44h]
    float v8; // [sp+60h] [-40h]

    MemFile_ReadData(memFile, 4, (unsigned char*)&v4);
    v5 = v4;
    if ((LODWORD(v4) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\../universal/memfile.h", 173, 0, "%s", "!IS_NAN(value)");
    v6 = v4;
    MemFile_ReadData(memFile, 4, (unsigned char *)&v4);
    v5 = v4;
    if ((LODWORD(v4) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\../universal/memfile.h", 173, 0, "%s", "!IS_NAN(value)");
    v7 = v4;
    MemFile_ReadData(memFile, 4, (unsigned char *)&v4);
    v2 = v4;
    v5 = v4;
    if ((LODWORD(v4) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\../universal/memfile.h", 173, 0, "%s", "!IS_NAN(value)");
        v2 = v4;
    }
    v5 = v6;
    v8 = v2;
    if ((LODWORD(v6) & 0x7F800000) == 0x7F800000
        || (v5 = v7, (LODWORD(v7) & 0x7F800000) == 0x7F800000)
        || (v5 = v2, (LODWORD(v5) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            119,
            0,
            "%s",
            "!IS_NAN((v)[0]) && !IS_NAN((v)[1]) && !IS_NAN((v)[2])");
    }
    return Scr_AllocVector(&v6);
}

void __cdecl WriteCodepos(const char *pos, MemoryFile *memFile)
{
    int v4; // r11
    int v5; // [sp+50h] [-20h] BYREF

    if (pos)
    {
        if (!Scr_IsInOpcodeMemory(pos))
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                128,
                0,
                "%s",
                "!pos || Scr_IsInOpcodeMemory( pos )");
        v4 = pos - scrVarPub.programBuffer;
    }
    else
    {
        v4 = -1;
    }
    v5 = v4;
    MemFile_WriteData(memFile, 4, &v5);
}

const char *__cdecl Scr_ReadCodepos(MemoryFile *memFile)
{
    int v1; // r31
    const char *v2; // r31
    const char *result; // r3
    int v4; // [sp+50h] [-30h] BYREF

    MemFile_ReadData(memFile, 4, (unsigned char*)&v4);
    v1 = v4;
    if (v4 < -1)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 141, 0, "%s", "offset >= -1");
    if (v1 < 0)
        return 0;
    v2 = &scrVarPub.programBuffer[v1];
    result = v2;
    if (v2)
    {
        if (!Scr_IsInOpcodeMemory(v2))
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                143,
                0,
                "%s",
                "!pos || Scr_IsInOpcodeMemory( pos )");
        return v2;
    }
    return result;
}

unsigned int Scr_CheckIdHistory(unsigned int index)
{
    unsigned int result; // r3
    unsigned int v3; // r11
    __int16 *v4; // r10
    int v5; // r9
    __int16 *v6; // r11
    unsigned int v7; // r10
    int v8; // r9

    result = 1;
    v3 = g_idHistoryIndex + 1;
    if (g_idHistoryIndex + 1 >= 0x10)
    {
    LABEL_6:
        v6 = idHistory;
        v7 = 0;
        while (1)
        {
            v8 = (unsigned __int16)*v6;
            if (index == v8 + 1)
                break;
            if (index == v8)
                return ++result;
            ++v7;
            ++v6;
            result += 2;
            if (v7 > g_idHistoryIndex)
                return 0;
        }
    }
    else
    {
        v4 = &idHistory[v3];
        while (1)
        {
            v5 = (unsigned __int16)*v4;
            if (index == v5 + 1)
                break;
            if (index == v5)
                return ++result;
            ++v3;
            ++v4;
            result += 2;
            if (v3 >= 0x10)
                goto LABEL_6;
        }
    }
    return result;
}

void WriteId(unsigned int id, unsigned int opcode, MemoryFile *memFile)
{
    unsigned int v3; // r31
    unsigned __int16 v6; // r29
    unsigned int v7; // r31
    int v8; // r10
    _WORD v9[32]; // [sp+50h] [-40h] BYREF

    v3 = id;
    if (!scrVarPub.saveIdMap[id] && id)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            188,
            0,
            "%s",
            "scrVarPub.saveIdMap[id] || !id");
    if ((opcode & 0xFFFFFFF8) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            189,
            0,
            "%s\n\t(opcode) = %i",
            "(!(opcode & ~7))",
            opcode);
    v6 = scrVarPub.saveIdMap[v3];
    v7 = 8 * Scr_CheckIdHistory(v6);
    if (v7 >= 0x100)
        v7 = 0;
    if ((v7 & 7) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            195,
            0,
            "%s\n\t(historyIndex) = %i",
            "(!(historyIndex & 7))",
            v7);
    //HIBYTE(v9[0]) = v7 + opcode;
    v9[0] = (v9[0] & 0x00FF) | ((uint16_t)(v7 + opcode) << 8);
    MemFile_WriteData(memFile, 1, v9);
    if (!v7)
    {
        v9[0] = v6;
        MemFile_WriteData(memFile, 2, v9);
    }
    v8 = ((_BYTE)g_idHistoryIndex - 1) & 0xF;
    idHistory[g_idHistoryIndex] = v6;
    g_idHistoryIndex = v8;
}

unsigned int Scr_ReadId(MemoryFile *memFile, unsigned int opcode)
{
    unsigned int v2; // r11
    unsigned int v3; // r29
    unsigned int v4; // r30
    unsigned int result; // r3
    __int16 v6; // [sp+50h] [-30h] BYREF

    if (opcode >> 3)
    {
        v2 = g_idHistoryIndex;
        v3 = *(unsigned __int16 *)((char *)idHistory + ((2 * ((((opcode >> 3) + 1) >> 1) + g_idHistoryIndex)) & 0x1E))
            + ((opcode >> 3) & 1);
    }
    else
    {
        MemFile_ReadData(memFile, 2, (unsigned char*)&v6);
        v3 = v6;
        v2 = g_idHistoryIndex;
    }
    v4 = scrVarPub.saveIdMapRev[v3];
    if (scrVarPub.saveIdMapRev[v3])
    {
        AddRefToObject(scrVarPub.saveIdMapRev[v3]);
        v2 = g_idHistoryIndex;
    }
    result = v4;
    idHistory[v2] = v3;
    g_idHistoryIndex = ((_BYTE)v2 - 1) & 0xF;
    return result;
}

void __cdecl WriteStack(const VariableStackBuffer *stackBuf, MemoryFile *memFile)
{
    int v4; // r30
    __int16 v5; // r11
    const char *buf; // r31
    unsigned int v7; // r3
    __int16 v8; // r30
    VariableUnion *v9; // r4
    _WORD v10[24]; // [sp+50h] [-30h] BYREF

    iassert(stackBuf);
    v10[0] = stackBuf->size;
    v4 = v10[0];
    MemFile_WriteData(memFile, 2, v10);
    WriteCodepos(stackBuf->pos, memFile);
    WriteId(stackBuf->localId, 0, memFile);
    //HIBYTE(v10[0]) = stackBuf->time;
    v10[0] = (v10[0] & 0x00FF) | (stackBuf->time << 8);
    MemFile_WriteData(memFile, 1, v10);
    v5 = v4;
    buf = stackBuf->buf;
    if (v4)
    {
        do
        {
            v7 = (unsigned __int8)*buf;
            v8 = v5 - 1;
            v9 = *(VariableUnion **)(buf + 1);
            buf += 5;
            DoSaveEntryInternal(v7, v9, memFile);
            v5 = v8;
        } while (v8);
    }
}

VariableStackBuffer *__cdecl Scr_ReadStack(MemoryFile *memFile)
{
    __int16 v2; // r27
    int v3; // r28
    int v4; // r31
    unsigned __int16 v5; // r29
    _WORD *v6; // r31
    char *v7; // r29
    __int16 v8; // r11
    __int16 v9; // r28
    unsigned __int8 v11[8]; // [sp+50h] [-40h] BYREF
    VariableValue v12; // [sp+58h] [-38h] BYREF

    MemFile_ReadData(memFile, 2, v11);
    v2 = *(_WORD *)v11;
    v3 = *(unsigned __int16 *)v11;
    v4 = 5 * *(unsigned __int16 *)v11 + 11;
    v5 = 5 * *(_WORD *)v11 + 11;
    if (v4 != v5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            268,
            0,
            "%s",
            "bufLen == (unsigned short)bufLen");
    v6 = (uint16*)MT_Alloc(v4, 1);
    ++scrVarPub.numScriptThreads;
    v6[2] = v2;
    v6[3] = v5;
    *(unsigned int *)v6 = (unsigned int)Scr_ReadCodepos(memFile);
    MemFile_ReadData(memFile, 1, v11);
    v6[4] = Scr_ReadId(memFile, v11[0]);
    MemFile_ReadData(memFile, 1, v11);
    v7 = (char *)v6 + 11;
    *((_BYTE *)v6 + 10) = v11[0];
    if (v3)
    {
        v8 = v3;
        do
        {
            v9 = v8 - 1;
            Scr_DoLoadEntryInternal(&v12, memFile);
            v8 = v9;
            *v7 = v12.type;
            *(unsigned int *)(v7 + 1) = v12.u.intValue;
            v7 += 5;
        } while (v9);
    }
    return (VariableStackBuffer *)v6;
}

void __cdecl Scr_DoLoadEntryInternal(VariableValue *value, MemoryFile *memFile)
{
    unsigned int v4; // r4
    int v5; // r4
    const char *v6; // r3
    _BYTE v7[4]; // [sp+50h] [-20h] BYREF
    VariableUnion v8; // [sp+54h] [-1Ch] BYREF

    MemFile_ReadData(memFile, 1, v7);
    v4 = v7[0];
    if ((v7[0] & 7) != 0)
    {
        value->type = VAR_POINTER;
        value->u.intValue = Scr_ReadId(memFile, v4);
    }
    else
    {
        v5 = v7[0] >> 3;
        value->type = (Vartype_t)v5;
        switch (v5)
        {
        case 0:
        case 8:
            return;
        case 2:
        case 3:
            value->u.intValue = (unsigned __int16)Scr_ReadString(memFile);
            break;
        case 4:
            value->u.intValue = (int)Scr_ReadVec3(memFile);
            break;
        case 5:
            value->u.floatValue = MemFile_ReadFloat(memFile);
            break;
        case 6:
        case 11:
            MemFile_ReadData(memFile, 4, (unsigned char*)&v8);
            value->u = v8;
            break;
        case 7:
        case 9:
            value->u.intValue = (int)Scr_ReadCodepos(memFile);
            break;
        case 10:
            value->u.intValue = (int)Scr_ReadStack(memFile);
            break;
        default:
            if (!alwaysfails)
            {
                v6 = va("unknown type %i", v5);
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 501, 1, v6);
            }
            break;
        }
    }
}

int __cdecl Scr_DoLoadEntry(VariableValue *value, bool isArray, MemoryFile *memFile)
{
    int v5; // r30
    MemoryFile *v6; // r3
    int v7; // r31
    int result; // r3
    unsigned __int8 v9; // [sp+50h] [-20h] BYREF
    _BYTE v10[3]; // [sp+51h] [-1Fh] BYREF
    int v11; // [sp+54h] [-1Ch] BYREF

    Scr_DoLoadEntryInternal(value, memFile);
    if (isArray)
    {
        MemFile_ReadData(memFile, 1, (unsigned char*)&v11);
        switch (HIBYTE(v11) & 7)
        {
        case 0:
            result = 0x800000;
            break;
        case 1:
            MemFile_ReadData(memFile, 1, (unsigned char *)&v11);
            result = SHIBYTE(v11) + 0x800000;
            break;
        case 2:
            MemFile_ReadData(memFile, 2, (unsigned char *)&v11);
            result = SHIWORD(v11) + 0x800000;
            break;
        case 3:
            MemFile_ReadData(memFile, 4, (unsigned char *)&v11);
            result = v11 + 0x800000;
            break;
        case 4:
            result = (unsigned __int16)Scr_ReadString(memFile);
            break;
        case 5:
            result = Scr_ReadId(memFile, HIBYTE(v11)) + 0x10000;
            break;
        default:
            if (!alwaysfails)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 539, 1, "bad case");
            result = 0;
            break;
        }
    }
    else
    {
        MemFile_ReadData(memFile, 1, &v9);
        v5 = v9;
        MemFile_ReadData(memFile, 1, v10);
        v6 = memFile;
        v7 = v10[0];
        MemFile_ReadData(v6, 1, (unsigned char *)&v11);
        return ((__ROL4__(HIBYTE(v11), 8) + v7) << 8) + v5;
    }
    return result;
}

void __cdecl AddSaveObjectInternal(unsigned int parentId)
{
    if (parentId)
    {
        if (!scrVarPub.saveIdMap[parentId])
        {
            scrVarPub.saveIdMap[parentId] = ++scrVarPub.savecount;
            *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = parentId;
        }
    }
}

unsigned int __cdecl Scr_ConvertThreadFromLoad(unsigned __int16 handle)
{
    int v2; // r30
    unsigned int v3; // r30

    if (!handle)
        return 0;
    v2 = handle;
    if (!scrVarPub.saveIdMapRev[v2])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            746,
            0,
            "%s",
            "scrVarPub.saveIdMapRev[handle]");
    v3 = scrVarPub.saveIdMapRev[v2];
    ++scrVarPub.ext_threadcount;
    AddRefToObject(v3);
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[v3];
    return v3;
}

void __cdecl Scr_DoLoadObjectInfo(unsigned __int16 parentId, MemoryFile *memFile)
{
    unsigned int v2; // r19
    VariableValueInternal *parentValue; // r31
    int v5; // r30
    unsigned int v6; // r3
    VariableValueInternal_w w; // r11
    int v8; // r3
    VariableValueInternal_w v9; // r11
    unsigned int v10; // r29
    unsigned int v11; // r11
    bool v12; // r29
    int v13; // r24
    int v14; // r31
    VariableValueInternal *entryValue; // r30
    unsigned int v16; // r4
    __int64 v17; // r3
    int v18; // r11
    int type; // r31
    int v20; // r11
    __int64 v21; // [sp+50h] [-90h] BYREF
    VariableValue value; // [sp+58h] [-88h] BYREF

    v2 = parentId;
    parentValue = &scrVarGlob.variableList[parentId + 1];

    iassert((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
    iassert(IsObject(parentValue));
    MemFile_ReadData(memFile, 1, (unsigned char*)&v21);

    switch (HIBYTE(v21) & 7)
    {
    case 1:
        v5 = 14;
        parentValue->u.o.u.size = Scr_ReadId(memFile, HIBYTE(v21));
        break;
    case 2:
        v5 = 15;
        v6 = Scr_ReadId(memFile, HIBYTE(v21));
        w = parentValue->w;
        parentValue->u.o.u.entnum = v6;
        iassert(!(parentValue->w.notifyName & VAR_NAME_HIGH_MASK));
        parentValue->w.type |= (Scr_ReadOptionalString(memFile) << 8) & 0xFFFF00;
        break;
    case 3:
        v5 = 16;
        v8 = Scr_ReadId(memFile, HIBYTE(v21));
        v9 = parentValue->w;
        parentValue->u.o.u.entnum = v8;
        iassert(!(parentValue->w.waitTime & VAR_NAME_HIGH_MASK));
        MemFile_ReadData(memFile, 4, (unsigned char*)&v21);
        parentValue->w.type |= HIDWORD(v21) << 8;
        break;
    case 4:
        v5 = 17;
        parentValue->u.o.u.size = Scr_ReadId(memFile, HIBYTE(v21));
        MemFile_ReadData(memFile, 1, (unsigned char *)&v21);
        v10 = HIBYTE(v21);
        iassert(!(parentValue->w.parentLocalId & VAR_NAME_HIGH_MASK));
        parentValue->w.type |= Scr_ReadId(memFile, v10) << 8;
        break;
    case 5:
        v5 = 19;
        parentValue->u.o.u.size = Scr_ReadId(memFile, HIBYTE(v21));
        break;
    default:
        v5 = HIBYTE(v21) >> 3;
        if (v5 == 20)
        {
            MemFile_ReadData(memFile, 2, (unsigned char *)&v21);
            v11 = parentValue->w.classnum & 0xFFFFFF00;
            parentValue->u.o.u.size = HIWORD(v21);
            if (v11)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    827,
                    0,
                    "%s",
                    "!(parentValue->w.classnum & VAR_NAME_HIGH_MASK)");
            MemFile_ReadData(memFile, 2, (unsigned char *)&v21);
            parentValue->w.type |= SHIWORD(v21) << 8;
        }
        else if (v5 == 21)
        {
            parentValue->u.o.u.size = 0;
        }
        break;
    }
    if ((v5 & 0xFFFFFFE0) != 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 837, 0, "%s", "!(type & ~VAR_MASK)");
    v12 = v5 == 21;
    parentValue->w.type = parentValue->w.type & 0xFFFFFFE0 | v5;
    MemFile_ReadData(memFile, 2, (unsigned char *)&v21);
    if (HIWORD(v21))
    {
        v13 = HIWORD(v21);
        do
        {
            v14 = Scr_DoLoadEntry(&value, v12, memFile);
            entryValue = &scrVarGlob.variableList[GetVariable(v2, (unsigned int)v14) + VARIABLELIST_CHILD_BEGIN];
            if (v12)
            {
                //LODWORD(v17) = Scr_GetArrayIndexValue(v14).u.intValue;
                //v21 = v17;
                VariableValue val = Scr_GetArrayIndexValue(v14);
                RemoveRefToValue(&val);
            }
            v18 = entryValue->w.status & 0x60;
            if (!v18 || v18 == 96)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    856,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL");
            iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED);
            type = value.type;
            iassert(!(value.type & ~VAR_MASK));
            iassert(!(entryValue->w.type & VAR_MASK));
            --v13;
            v20 = type | entryValue->w.type;
            entryValue->u.u.intValue = value.u.intValue;
            entryValue->w.type = v20;
        } while (v13);
    }
}

void __cdecl Scr_ReadGameEntry(MemoryFile *memFile)
{
    int type; // r28
    unsigned int gameId; // r11
    VariableUnion v4; // r8
    VariableValue v5; // [sp+50h] [-30h] BYREF

    if (scrVarPub.gameId)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 910, 0, "%s", "!scrVarPub.gameId");
    scrVarPub.gameId = AllocValue();
    Scr_DoLoadEntryInternal(&v5, memFile);
    type = v5.type;
    if ((v5.type & 0xFFFFFFE0) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            915,
            0,
            "%s",
            "!(tempValue.type & ~VAR_MASK)");
    gameId = scrVarPub.gameId;
    v4.intValue = (int)v5.u.intValue;
    scrVarGlob.variableList[gameId + VARIABLELIST_CHILD_BEGIN].w.type |= type;
    scrVarGlob.variableList[gameId + VARIABLELIST_CHILD_BEGIN].u.u = v4;
}

static void Scr_RemoveElementValue(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *i; // r31

    if (element->valueDefined)
    {
        element->valueDefined = 0;
        RemoveRefToValue(element->value.type, element->value.u);
    }
    for (i = element->childHead; i; i = i->next)
        Scr_RemoveElementValue(i);
}

void Scr_RemoveElementValues()
{
    Scr_WatchElement_s *i; // r30
    Scr_WatchElement_s *j; // r31

    for (i = scrDebuggerGlob.scriptWatch.elementHead; i; i = i->next)
    {
        if (i->valueDefined)
        {
            i->valueDefined = 0;
            RemoveRefToValue(i->value.type, i->value.u);
        }
        for (j = i->childHead; j; j = j->next)
            Scr_RemoveElementValue(j);
        if (!i->breakpoint)
        {
            if (!i->expr.exprHead)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp", 8570, 0, "%s", "expr->exprHead");
            Scr_ClearDebugExpr(i->expr.exprHead);
        }
    }
}

static void Scr_AddDebuggerRefs()
{
    if (scrVarPub.developer)
    {
        if (scrVmPub.function_count)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp",
                8605,
                0,
                "%s",
                "!scrVmPub.function_count");
        scrDebuggerGlob.scriptWatch.localId = 0;
        iassert(!scrVarPub.evaluate);
        scrVarPub.evaluate = 1;
        //Scr_ScriptWatch::Evaluate(&scrDebuggerGlob.scriptWatch);
        scrDebuggerGlob.scriptWatch.Evaluate();
        //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 1);
        scrDebuggerGlob.scriptWatch.UpdateBreakpoints(true);
        iassert(scrVarPub.evaluate);
        scrVarPub.evaluate = 0;
    }
}

static void Scr_RemoveDebuggerRefs()
{
    if (scrVarPub.developer)
    {
        if (!Scr_IsStackClear())
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp", 8582, 0, "%s", "Scr_IsStackClear()");
        if (scrVmPub.function_count)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp",
                8584,
                0,
                "%s",
                "!scrVmPub.function_count");
        scrDebuggerGlob.scriptWatch.localId = 0;
        if (scrVarPub.evaluate)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp", 8587, 0, "%s", "!scrVarPub.evaluate");
        scrVarPub.evaluate = 1;
        //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 0);
        scrDebuggerGlob.scriptWatch.UpdateBreakpoints(false);
        Scr_RemoveElementValues();
        if (!scrVarPub.evaluate)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp", 8593, 0, "%s", "scrVarPub.evaluate");
        scrVarPub.evaluate = 0;
        if (!Scr_IsStackClear())
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp", 8596, 0, "%s", "Scr_IsStackClear()");
    }
}

void __cdecl Scr_SaveShutdown(bool savegame)
{
    char v2; // r20
    unsigned __int16 *v3; // r25
    int v4; // r30
    VariableValueInternal_w *p_w; // r27
    int v6; // r26
    const char *v7; // r31

    v2 = 0;
    if (scrVarDebugPub)
    {
        v3 = &scrVarPub.saveIdMap[1];
        v4 = 1;
        p_w = &scrVarGlob.variableList[2].w;
        v6 = 0x7FFF;
        do
        {
            if ((p_w->type & 0x60) != 0)
            {
                if (!IsObject((VariableValueInternal *)&p_w[-2]))
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                        968,
                        0,
                        "%s",
                        "IsObject( entryValue )");
                v7 = scrVarDebugPub->varUsage[v4 + 1];
                if (!v7)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 970, 0, "%s", "pos");
                if (!*v3 && (p_w->type & 0x1F) != 0x15)
                {
                    if (!v2)
                    {
                        v2 = 1;
                        Com_Printf(23, "\n****script variable cyclic leak*****\n");
                    }
                    Scr_PrintPrevCodePos(23, (char*)v7, 0);
                }
            }
            --v6;
            p_w += 4;
            ++v4;
            ++v3;
        } while (v6);
    }
    Scr_AddDebuggerRefs();
    if (v2)
    {
        Com_Printf(23, "************************************\n");
        if (savegame)
            Com_Error(ERR_DROP, "Script variable leak due to cyclic usage (see console for details)");
        else
            Com_Printf(23, "script variable leak due to cyclic usage\n");
    }
}

void __cdecl Scr_LoadPre(int sys, MemoryFile *memFile)
{
    unsigned __int16 savecount; // r11
    unsigned int v4; // r30
    unsigned __int16 *v5; // r29
    unsigned int v6; // r3
    unsigned int v7; // r29
    unsigned __int16 *v8; // r30
    unsigned int v9; // r30
    unsigned int Id; // r3
    unsigned int v11; // r30
    unsigned int v12; // r3
    unsigned int v13; // r30
    unsigned int v14; // r3
    unsigned int v15; // r30
    unsigned int v16; // r3
    unsigned int v17; // r30
    unsigned int v18; // r3
    int v19; // r29
    unsigned __int16 *p_entArrayId; // r31
    unsigned __int16 v21; // r10
    scrVarDebugPub_t *v22; // r11
    unsigned __int8 v23[96]; // [sp+50h] [-60h] BYREF

    if (sys != 1)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1008, 0, "%s", "sys == SCR_SYS_GAME");
    if (scrVarPub.varUsagePos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1011, 0, "%s", "!scrVarPub.varUsagePos");
    scrVarPub.varUsagePos = "<save game variable>";
    memset(scrVmDebugPub.profileEnable, 0, sizeof(scrVmDebugPub.profileEnable));
    MemFile_ReadData(memFile, 4, v23);
    scrVarPub.time = *(unsigned int *)v23;
    MemFile_ReadData(memFile, 2, v23);
    scrVarPub.savecount = *(_WORD *)v23;
    Com_Memset(scrVarPub.saveIdMap, 0, 0x10000);
    Com_Memset(scrVarPub.saveIdMapRev, 0, 0x10000);
    memset(idHistory, 0, 32);
    savecount = scrVarPub.savecount;
    g_idHistoryIndex = 0;
    v4 = 1;
    if (scrVarPub.savecount)
    {
        v5 = &scrVarPub.saveIdMapRev[1];
        do
        {
            v6 = AllocObject();
            *v5++ = v6;
            scrVarPub.saveIdMap[v6] = v4++;
            savecount = scrVarPub.savecount;
        } while (v4 <= scrVarPub.savecount);
    }
    v7 = 1;
    if (savecount)
    {
        v8 = &scrVarPub.saveIdMapRev[1];
        do
        {
            Scr_DoLoadObjectInfo(*v8, memFile);
            ++v7;
            ++v8;
        } while (v7 <= scrVarPub.savecount);
    }
    Scr_ReadGameEntry(memFile);
    MemFile_ReadData(memFile, 1, v23);
    v9 = v23[0];
    if (scrVarPub.levelId)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1042, 0, "%s", "!scrVarPub.levelId");
    Id = Scr_ReadId(memFile, v9);
    scrVarPub.levelId = Id;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[Id];
    MemFile_ReadData(memFile, 1, v23);
    v11 = v23[0];
    if (scrVarPub.animId)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1050, 0, "%s", "!scrVarPub.animId");
    v12 = Scr_ReadId(memFile, v11);
    scrVarPub.animId = v12;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[v12];
    MemFile_ReadData(memFile, 1, v23);
    v13 = v23[0];
    if (scrVarPub.timeArrayId)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1058, 0, "%s", "!scrVarPub.timeArrayId");
    v14 = Scr_ReadId(memFile, v13);
    scrVarPub.timeArrayId = v14;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[v14];
    MemFile_ReadData(memFile, 1, v23);
    v15 = v23[0];
    if (scrVarPub.pauseArrayId)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            1066,
            0,
            "%s",
            "!scrVarPub.pauseArrayId");
    v16 = Scr_ReadId(memFile, v15);
    scrVarPub.pauseArrayId = v16;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[v16];
    MemFile_ReadData(memFile, 1, v23);
    v17 = v23[0];
    if (scrVarPub.freeEntList)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1074, 0, "%s", "!scrVarPub.freeEntList");
    v18 = Scr_ReadId(memFile, v17);
    scrVarPub.freeEntList = v18;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[v18];
    v19 = 4;
    p_entArrayId = &g_classMap[0].entArrayId;
    do
    {
        if (GetArraySize(*p_entArrayId))
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                1083,
                0,
                "%s",
                "!GetArraySize( g_classMap[classnum].entArrayId )");
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[*p_entArrayId];
        RemoveRefToObject(*p_entArrayId);
        MemFile_ReadData(memFile, 1, v23);
        v21 = Scr_ReadId(memFile, v23[0]);
        v22 = scrVarDebugPub;
        *p_entArrayId = v21;
        if (v22)
            ++v22->extRefCount[v21];
        --v19;
        p_entArrayId += 6;
    } while (v19);
}

static void Scr_AddDebugExprValueRefCount(unsigned __int16 *refCount, sval_u *val)
{
    if (val->type == 81)
        ++refCount[val[1].type];
}

static void Scr_AddDebugExprRefCount(unsigned __int16 *refCount, sval_u *debugExprHead)
{
    sval_u *i; // r31

    for (i = debugExprHead; i; i = (sval_u *)i->type)
        Scr_AddDebugExprValueRefCount(refCount, i + 1);
}

static void Scr_AddDebugRefCountChildren(Scr_WatchElement_s *element, unsigned __int16 *refCount)
{
    Scr_WatchElement_s *i; // r31

    if (element->valueDefined && element->value.type == 1)
        ++refCount[element->value.u.intValue];
    for (i = element->childHead; i; i = i->next)
        Scr_AddDebugRefCountChildren(i, refCount);
}

static void Scr_AddDebugRefCount(unsigned __int16 *refCount)
{
    Scr_WatchElement_s *i; // r31

    for (i = scrDebuggerGlob.scriptWatch.elementHead; i; i = i->next)
    {
        if (!i->breakpoint)
        {
            if (!i->expr.exprHead)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp", 7231, 0, "%s", "expr->exprHead");
            Scr_AddDebugExprRefCount(refCount, (sval_u*)i->expr.exprHead);
        }
        Scr_AddDebugRefCountChildren(i, refCount);
    }
}

static bool Scr_IsVariableBreakpoint(unsigned int id)
{
    Scr_WatchElementDoubleNode_t **variableBreakpoints; // r11

    variableBreakpoints = scrDebuggerGlob.variableBreakpoints;
    if (!scrDebuggerGlob.variableBreakpoints)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_debugger.cpp",
            7193,
            0,
            "%s",
            "scrDebuggerGlob.variableBreakpoints");
        variableBreakpoints = scrDebuggerGlob.variableBreakpoints;
    }
    return variableBreakpoints[id] != 0;
}

static void CheckReferenceRange(unsigned int begin, unsigned int end)
{
#if 0
    signed int parentValue; // r20
    signed int parentType; // r23
    VariableValueInternal::<unnamed_tag> *p_u; // r27
    VariableValueInternal::<unnamed_tag> entryValue; // r11
    unsigned int ParentLocalId; // r3
    int v7; // r10
    unsigned int i; // r28
    unsigned int v9; // r4
    int v10; // r2
    int v11; // r10
    unsigned __int8 *v12; // r8
    int v13; // r9
    int v14; // r7
    int v15; // r10

    parentValue = end - begin;
    parentType = 1;
    if ((int)(end - begin) > 1)
    {
        p_u = &scrVarGlob.variableList[begin + 1].u;
        do
        {
            entryValue = p_u[1];
            if ((entryValue.o.u.size & 0x60) != 0)
            {
                switch (entryValue.o.u.size & 0x1F)
                {
                case 1:
                    ++scrVarDebugPub->refCount[p_u->u.intValue];
                    break;
                case 0xA:
                    if (!*(_WORD *)(p_u->u.intValue + 8))
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_variable.cpp",
                            271,
                            0,
                            "%s",
                            "entryValue->u.u.stackValue->localId");
                    v11 = 2 * (*(unsigned __int16 *)(p_u->u.intValue + 8) + 0x38000);
                    ++*(_WORD *)((char *)scrVarDebugPub->varUsage + v11);
                    v12 = (unsigned __int8 *)(p_u->u.intValue + 11);
                    v13 = *(unsigned __int16 *)(p_u->u.intValue + 4);
                    if (*(_WORD *)(p_u->u.intValue + 4))
                    {
                        do
                        {
                            v14 = *v12;
                            --v13;
                            v15 = *(_DWORD *)(v12 + 1);
                            v12 += 5;
                            if (v14 == 1)
                                ++scrVarDebugPub->refCount[v15];
                        } while (v13);
                    }
                    break;
                case 0xE:
                case 0xF:
                case 0x10:
                case 0x13:
                    goto LABEL_7;
                case 0x11:
                    ParentLocalId = GetParentLocalId(parentType);
                    ++scrVarDebugPub->refCount[ParentLocalId];
                LABEL_7:
                    v7 = 2 * (p_u->o.u.size + 0x38000);
                    ++*(_WORD *)((char *)scrVarDebugPub->varUsage + v7);
                    break;
                case 0x15:
                    for (i = FindFirstSibling(parentType); i; i = FindNextSibling(i))
                    {
                        if (IsObject(&scrVarGlob.variableList[i + 32770]))
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_variable.cpp",
                                261,
                                0,
                                "%s",
                                "!IsObject( entryValue2 )");
                        if (Scr_GetArrayIndexValue(
                            (VariableValue *)((unsigned int)scrVarGlob.variableList[i + 32770].w.u.intValue >> 8),
                            v9) == (VariableValue *)1)
                            ++scrVarDebugPub->refCount[v10];
                    }
                    break;
                default:
                    break;
                }
            }
            ++parentType;
            p_u += 4;
        } while (parentType < parentValue);
    }
#endif
}

static int CheckReferences()
{
    int v0; // r11
    unsigned int i; // r31
    int v2; // r30
    int v3; // r9
    unsigned int v4; // r7
    VariableValueInternal_w *j; // r11

    if (!scrVarDebugPub || scrStringDebugGlob && scrStringDebugGlob->ignoreLeaks)
        return 1;
    memcpy(scrVarDebugPub->refCount, scrVarDebugPub->extRefCount, sizeof(scrVarDebugPub->refCount));
    Scr_AddDebugRefCount(scrVarDebugPub->refCount);
    CheckReferenceRange(1u, 0x8001u);
    CheckReferenceRange(0x8002u, 0x18000u);
    if (scrVarPub.developer)
    {
        v0 = 1;
        for (i = 458754; i < 0x80000; i += 2)
        {
            v2 = v0 + 1;
            if (Scr_IsVariableBreakpoint(v0 + 1))
                ++*(_WORD *)((char *)scrVarDebugPub->varUsage + i);
            v0 = v2;
        }
    }
    v3 = 458754;
    v4 = 16;
    for (j = &scrVarGlob.variableList[2].w;
        (j->status & 0x60) == 0
        || (j->type & 0x1Fu) < 0xE
        || *(_WORD *)((char *)scrVarDebugPub->varUsage + v3)
        && *(unsigned __int16 *)((char *)scrVarDebugPub->varUsage + v3) == j[-1].status + 1;
        j += 4)
    {
        v4 += 16;
        v3 += 2;
        if (v4 >= 0x80000)
            return 1;
    }
    return 0;
}

void __cdecl Scr_LoadShutdown()
{
    unsigned int v0; // r30
    unsigned __int16 *v1; // r31

    v0 = 1;
    if (scrVarPub.savecount)
    {
        v1 = &scrVarPub.saveIdMapRev[1];
        do
        {
            RemoveRefToObject(*v1);
            ++v0;
            ++v1;
        } while (v0 <= scrVarPub.savecount);
    }
    Scr_InitDebuggerSystem();
    scrVarPub.varUsagePos = 0;
    if (!CheckReferences())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1115, 0, "%s", "CheckReferences()");
}

void __cdecl DoSaveEntryInternal(unsigned int type, VariableUnion *u, MemoryFile *memFile)
{
    unsigned int UsedSize; // r3
    unsigned int v7; // r3
    unsigned int v8; // r3
    unsigned int v9; // r3
    MemoryFile *v10; // r3
    double v11; // fp8
    double v12; // fp7
    double v13; // fp6
    double v14; // fp5
    double v15; // fp4
    double v16; // fp3
    double v17; // fp2
    double v18; // fp1
    unsigned int v19; // r3
    const char *v20; // r3
    unsigned int v21; // r3
    unsigned int v22; // r3
    unsigned int v23; // r3
    unsigned int v24; // r3
    unsigned int v25; // r3
    float v26; // [sp+8h] [-78h]
    float v27; // [sp+10h] [-70h]
    float v28; // [sp+18h] [-68h]
    float v29; // [sp+20h] [-60h]
    _BYTE v30[4]; // [sp+50h] [-30h] BYREF
    unsigned int v31[11]; // [sp+54h] [-2Ch] BYREF

    if (type != (unsigned __int8)type)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            1122,
            0,
            "%s",
            "type == (unsigned char)type");
    if (type == 1)
    {
        UsedSize = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("pointer", UsedSize);
        WriteId((unsigned int)u, 1u, memFile);
        v7 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v7);
    }
    else
    {
        v8 = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("type", v8);
        v30[0] = 8 * type;
        MemFile_WriteData(memFile, 1, v30);
        v9 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v9);
        switch (type)
        {
        case 0u:
        case 8u:
            return;
        case 2u:
        case 3u:
            v19 = MemFile_GetUsedSize(memFile);
            //ProfMem_Begin("string", v19);
            v20 = SL_ConvertToString((unsigned __int16)u);
            MemFile_WriteCString(memFile, v20);
            v21 = MemFile_GetUsedSize(memFile);
            //ProfMem_End(v21);
            break;
        case 4u:
            WriteVector(&u->floatValue, memFile);
            break;
        case 5u:
            WriteFloat(*(float *)&u, memFile);
            break;
        case 6u:
            v22 = MemFile_GetUsedSize(memFile);
            //ProfMem_Begin("int", v22);
            v31[0] = u->intValue;
            MemFile_WriteData(memFile, 4, v31);
            v23 = MemFile_GetUsedSize(memFile);
            //ProfMem_End(v23);
            break;
        case 7u:
        case 9u:
            WriteCodepos((const char *)u, memFile);
            break;
        case 0xAu:
            v24 = MemFile_GetUsedSize(memFile);
            //ProfMem_Begin("stack", v24);
            WriteStack((const VariableStackBuffer *)u, memFile);
            v25 = MemFile_GetUsedSize(memFile);
            //ProfMem_End(v25);
            break;
        case 0xBu:
            v31[0] = u->intValue;
            MemFile_WriteData(memFile, 4, v31);
            break;
        default:
            if (!alwaysfails)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 1172, 0, "unknown type");
            break;
        }
    }
}

void __cdecl Scr_SaveSource(MemoryFile *memFile)
{
    MemFile_WriteData(memFile, 1, &scrVarPub.developer);
    MemFile_WriteData(memFile, 1, &scrVarPub.developer_script);
}

void __cdecl SaveMemory_SaveWriteImmediate(const void *buffer, unsigned int len, SaveImmediate *save)
{
    //WriteToMemoryCardFile(save->f, buffer, len); // KISAKSAVE
}

void __cdecl Scr_SaveSourceImmediate(SaveImmediate *save)
{
    // KISAKSAVE
#if 0
    int parentValue; // r9
    unsigned int sourceBufferLookupLen; // r11
    bool *p_archive; // r10
    unsigned int entryValue; // r28
    int v6; // r29
    SourceBufferInfo *v7; // r31
    signed int len; // r5
    int v9; // [sp+50h] [-40h] BYREF

    if (scrVarPub.developer)
    {
        parentValue = 0;
        v9 = 0;
        sourceBufferLookupLen = scrParserPub.sourceBufferLookupLen;
        if (scrParserPub.sourceBufferLookupLen)
        {
            p_archive = &scrParserPub.sourceBufferLookup->archive;
            do
            {
                if (*p_archive)
                    v9 = ++parentValue;
                --sourceBufferLookupLen;
                p_archive += 44;
            } while (sourceBufferLookupLen);
        }
        WriteToMemoryCardFile(save->f, &v9, 4u);
        entryValue = 0;
        if (scrParserPub.sourceBufferLookupLen)
        {
            v6 = 0;
            do
            {
                v7 = &scrParserPub.sourceBufferLookup[v6];
                if (scrParserPub.sourceBufferLookup[v6].archive)
                {
                    WriteToMemoryCardFile(save->f, &v7->len, 4u);
                    len = v7->len;
                    if (len > 0)
                        WriteToMemoryCardFile(save->f, v7->sourceBuf, len);
                }
                ++entryValue;
                ++v6;
            } while (entryValue < scrParserPub.sourceBufferLookupLen);
        }
    }
#endif
}

void __cdecl Scr_LoadSource(MemoryFile *memFile, void *fileHandle)
{
    SaveSourceBufferInfo *saveSourceBufferLookup; // r3
    signed int v5; // r25
    int v6; // r27
    SaveSourceBufferInfo *v7; // r31
    int *p_len; // r30
    int len; // r3
    char *v10; // r3
    int v11; // r4
    char v12; // [sp+50h] [-50h] BYREF

    if (scrParserGlob.saveSourceBufferLookup)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            1236,
            0,
            "%s",
            "!scrParserGlob.saveSourceBufferLookup");
    MemFile_ReadData(memFile, 1, (unsigned char*)&v12);
    MemFile_ReadData(memFile, 1, (unsigned char*)&scrVarPub.developer_script);
    if (v12)
    {
        ReadFromDevice(&scrParserGlob.saveSourceBufferLookupLen, 4, fileHandle);
        saveSourceBufferLookup = (SaveSourceBufferInfo *)Hunk_AllocDebugMem(
            8 * scrParserGlob.saveSourceBufferLookupLen,
            "Scr_LoadSource");
        scrParserGlob.saveSourceBufferLookup = saveSourceBufferLookup;
        v5 = scrParserGlob.saveSourceBufferLookupLen - 1;
        if ((signed int)(scrParserGlob.saveSourceBufferLookupLen - 1) >= 0)
        {
            v6 = v5;
            while (1)
            {
                v7 = &saveSourceBufferLookup[v6];
                p_len = &saveSourceBufferLookup[v6].len;
                ReadFromDevice(p_len, 4, fileHandle);
                len = v7->len;
                if (len <= 0)
                {
                    v7->sourceBuf = 0;
                }
                else
                {
                    v10 = (char *)Hunk_AllocDebugMem(len, "Scr_LoadSource");
                    v11 = *p_len;
                    v7->sourceBuf = v10;
                    ReadFromDevice(v10, v11, fileHandle);
                }
                --v5;
                --v6;
                if (v5 < 0)
                    break;
                saveSourceBufferLookup = scrParserGlob.saveSourceBufferLookup;
            }
        }
    }
}

void __cdecl Scr_SkipSource(MemoryFile *memFile, void *fileHandle)
{
    int i; // r30
    _BYTE v5[4]; // [sp+50h] [-30h] BYREF
    int v6; // [sp+54h] [-2Ch] BYREF
    int v7[4]; // [sp+58h] [-28h] BYREF

    MemFile_ReadData(memFile, 1, v5);
    MemFile_ReadData(memFile, 1, (unsigned char*)&scrVarPub.developer_script);
    if (v5[0])
    {
        if (fileHandle)
        {
            ReadFromDevice(&v6, 4, fileHandle);
            for (i = v6 - 1; i >= 0; --i)
            {
                ReadFromDevice(v7, 4, fileHandle);
                if (v7[0] > 0)
                    ReadFromDevice(0, v7[0], fileHandle);
            }
        }
    }
}

void __cdecl AddSaveStackInternal(const VariableStackBuffer *stackBuf)
{
    int localId; // r7
    unsigned __int16 size; // r11
    const char *buf; // r31
    unsigned int v4; // r3
    unsigned __int16 v5; // r30
    VariableUnion *v6; // r4

    localId = stackBuf->localId;
    if (stackBuf->localId && !scrVarPub.saveIdMap[localId])
    {
        scrVarPub.saveIdMap[localId] = ++scrVarPub.savecount;
        *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = localId;
    }
    size = stackBuf->size;
    buf = stackBuf->buf;
    if (size)
    {
        do
        {
            v4 = (unsigned __int8)*buf;
            v5 = size - 1;
            v6 = *(VariableUnion **)(buf + 1);
            buf += 5;
            AddSaveEntryInternal(v4, (const VariableStackBuffer*)v6);
            size = v5;
        } while (v5);
    }
}

void __cdecl AddSaveEntryInternal(unsigned int type, const VariableStackBuffer *u)
{
    if (type == 1)
    {
        if (u && !scrVarPub.saveIdMap[(unsigned int)u])
        {
            scrVarPub.saveIdMap[(unsigned int)u] = ++scrVarPub.savecount;
            *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = (unsigned __int16)u;
        }
    }
    else if (type == 10)
    {
        AddSaveStackInternal(u);
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl DoSaveEntry(VariableValue *value, VariableValue *name, bool isArray, MemoryFile *memFile)
{
    unsigned int UsedSize; // r3
    unsigned int v9; // r3
    unsigned int v10; // r3
    unsigned int v11; // r4
    unsigned int v12; // r3
    unsigned int v13; // r3
    unsigned int v14; // r3
    VariableValue *ArrayIndexValue; // r3 OVERLAPPED
    unsigned int v16; // r3
    int v17; // r30
    unsigned int v18; // r3
    unsigned int v19; // r3
    unsigned int v20; // r3
    unsigned int v21; // r3
    unsigned int v22; // r3
    const char *v23; // r3
    unsigned int v24; // r3
    unsigned int v25; // r3
    unsigned int v26; // r3
    unsigned int v27[2]; // [sp+50h] [-40h] BYREF
    __int64 v28; // [sp+58h] [-38h]

    if (!value)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 391, 0, "%s", "value");
    UsedSize = MemFile_GetUsedSize(memFile);
    //ProfMem_Begin("DoSaveEntry", UsedSize);
    v9 = MemFile_GetUsedSize(memFile);
    //ProfMem_Begin("DoSaveEntryInternal", v9);
    DoSaveEntryInternal(value->type, (VariableUnion *)value->u.intValue, memFile);
    v10 = MemFile_GetUsedSize(memFile);
    //ProfMem_End(v10);
    if (!isArray)
    {
        v12 = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("non-array", v12);
        if (((unsigned int)name & 0xFF000000) != 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                402,
                0,
                "%s\n\t(name) = %i",
                "(!(name & 0xFF000000))",
                name);
        //HIBYTE(v27[0]) = (_BYTE)name;
        v27[0] = (v27[0] & 0x00FFFFFF) | (((uint32_t)((uint8_t)name) & 0xFF) << 24);
        MemFile_WriteData(memFile, 1, v27);
        //HIBYTE(v27[0]) = BYTE2(name);
        v27[0] = (v27[0] & 0x00FFFFFF) | (((uint32_t)(((uint32_t)name >> 16) & 0xFF)) << 24);
        MemFile_WriteData(memFile, 1, v27);
        //HIBYTE(v27[0]) = BYTE1(name);
        v27[0] = (v27[0] & 0x00FFFFFF) | (((uint32_t)(((uint32_t)name >> 8) & 0xFF)) << 24);
        MemFile_WriteData(memFile, 1, v27);
        v13 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v13);
        v14 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v14);
        return;
    }
    VariableValue val = Scr_GetArrayIndexValue((unsigned int)name);
    ArrayIndexValue = &val;
    v28 = *(_QWORD *)&ArrayIndexValue;
    if (ArrayIndexValue == (VariableValue *)1)
    {
        WriteId(HIDWORD(v28), 5u, memFile);
    LABEL_23:
        v26 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v26);
        return;
    }
    if (ArrayIndexValue == (VariableValue *)2)
    {
        v22 = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("string array index", v22);
        //HIBYTE(v27[0]) = 4;
        v27[0] = (v27[0] & 0x00FFFFFF) | (4 << 24);
        MemFile_WriteData(memFile, 1, v27);
        v23 = SL_ConvertToString(HIDWORD(v28) & 0xFFFF);
        MemFile_WriteCString(memFile, v23);
        v24 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v24);
        v25 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v25);
        return;
    }
    if (ArrayIndexValue != (VariableValue *)6)
    {
        if (!alwaysfails)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 449, 1, "bad case");
            v16 = MemFile_GetUsedSize(memFile);
            //ProfMem_End(v16);
            return;
        }
        goto LABEL_23;
    }
    v17 = HIDWORD(v28);
    if (HIDWORD(v28))
    {
        if (SHIDWORD(v28) < -128 || SHIDWORD(v28) >= 128)
        {
            if (SHIDWORD(v28) < -32768 || SHIDWORD(v28) >= 0x8000)
            {
                //HIBYTE(v27[0]) = 3;
                v27[0] = (v27[0] & 0x00FFFFFF) | (3 << 24);
                MemFile_WriteData(memFile, 1, v27);
                v27[0] = v17;
                MemFile_WriteData(memFile, 4, v27);
                v21 = MemFile_GetUsedSize(memFile);
                //ProfMem_End(v21);
            }
            else
            {
                //HIBYTE(v27[0]) = 2;
                v27[0] = (v27[0] & 0x00FFFFFF) | (2 << 24);
                MemFile_WriteData(memFile, 1, v27);
                //HIWORD(v27[0]) = v17;
                v27[0] = (v27[0] & 0x0000FFFF) | ((uint32_t)(v17 & 0xFFFF) << 16);
                MemFile_WriteData(memFile, 2, v27);
                v20 = MemFile_GetUsedSize(memFile);
                //ProfMem_End(v20);
            }
        }
        else
        {
            //HIBYTE(v27[0]) = 1;
            v27[0] = (v27[0] & 0x00FFFFFF) | ((1 & 0xFF) << 24);
            MemFile_WriteData(memFile, 1, v27);
            //HIBYTE(v27[0]) = v17;
            v27[0] = (v27[0] & 0x00FFFFFF) | ((v17 & 0xFF) << 24);
            MemFile_WriteData(memFile, 1, v27);
            v19 = MemFile_GetUsedSize(memFile);
            //ProfMem_End(v19);
        }
    }
    else
    {
        //HIBYTE(v27[0]) = 0;
        v27[0] = v27[0] & 0x00FFFFFF;
        MemFile_WriteData(memFile, 1, v27);
        v18 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v18);
    }
}

void __cdecl AddSaveObjectChildren(unsigned int parentId)
{
    VariableValueInternal *parentValue; // r23
    int parentType; // r24
    unsigned int i; // r29
    VariableValueInternal *entryValue; // r30
    unsigned int v6; // r4
    int v7; // r2
    VariableValueInternal_u u; // r3
    int v9; // r11
    VariableValueInternal_w w; // r11
    int v11; // r9
    int size; // r9

    parentValue = &scrVarGlob.variableList[parentId + 1];

    iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
    iassert(IsObject(parentValue));
    parentType = parentValue->w.type & 0x1F;
    for (i = FindLastSibling(parentId); i; i = FindPrevSibling(i))
    {
        entryValue = (VariableValueInternal *)((char *)&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN]
            + __ROL4__(scrVarGlob.variableList[i + VARIABLELIST_CHILD_BEGIN].hash.id, 4));
        iassert(!IsObject(entryValue));
        if (parentType == VAR_ARRAY
            && (v7 = Scr_GetArrayIndexValue(((unsigned int)entryValue->w.status >> 8)).u.intValue, v7 == 1)
            && v7
            && !scrVarPub.saveIdMap[v7])
        {
            scrVarPub.saveIdMap[v7] = ++scrVarPub.savecount;
            *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = v7;
        }
        u = entryValue->u;
        v9 = entryValue->w.type & 0x1F;
        if (v9 == 1)
        {
            if (u.u.intValue && !scrVarPub.saveIdMap[u.u.intValue])
            {
                scrVarPub.saveIdMap[u.u.intValue] = ++scrVarPub.savecount;
                *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = u.o.u.size;
            }
        }
        else if (v9 == 10)
        {
            AddSaveStackInternal(u.u.stackValue);
        }
    }
    switch (parentType)
    {
    case 14:
    case 15:
    case 16:
    case 19:
        goto LABEL_24;
    case 17:
        w = parentValue->w;
        v11 = (unsigned __int16)((unsigned int)w.status >> 8);
        if ((unsigned __int16)((unsigned int)w.status >> 8) && !scrVarPub.saveIdMap[v11])
        {
            scrVarPub.saveIdMap[v11] = ++scrVarPub.savecount;
            *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = v11;
        }
    LABEL_24:
        size = parentValue->u.o.u.size;
        if (parentValue->u.o.u.size)
        {
            if (!scrVarPub.saveIdMap[size])
            {
                scrVarPub.saveIdMap[size] = ++scrVarPub.savecount;
                *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = size;
            }
        }
        break;
    default:
        return;
    }
}

void __cdecl AddSaveObject(unsigned int parentId)
{
    unsigned __int16 savecount; // r11
    int v2; // r29
    unsigned __int16 *v3; // r30

    savecount = scrVarPub.savecount;
    v2 = scrVarPub.savecount;
    if (parentId && !scrVarPub.saveIdMap[parentId])
    {
        ++scrVarPub.savecount;
        scrVarPub.saveIdMap[parentId] = v2 + 1;
        *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = parentId;
        savecount = scrVarPub.savecount;
    }
    if (v2 < savecount)
    {
        v3 = &scrVarPub.saveIdMapRev[v2];
        do
        {
            ++v3;
            ++v2;
            AddSaveObjectChildren(*v3);
        } while (v2 < scrVarPub.savecount);
    }
}

void __cdecl DoSaveObjectInfo(unsigned int parentId, MemoryFile *memFile)
{
    VariableValueInternal *v4; // r31
    int v5; // r30
    unsigned int UsedSize; // r3
    unsigned int v7; // r3
    int v8; // r4
    bool v9; // r26
    unsigned int v10; // r3
    __int16 v11; // r31
    unsigned int i; // r3
    unsigned int j; // r30
    VariableValueInternal *v14; // r31
    int v15; // r11
    VariableValueInternal_w w; // r11
    unsigned int v17; // r3
    VariableValue v18[12]; // [sp+50h] [-60h] BYREF

    v4 = &scrVarGlob.variableList[parentId + 1];
    if ((v4->w.status & 0x60) != 0x60)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            655,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL");
    if (!IsObject(v4))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 656, 0, "%s", "IsObject( parentValue )");
    v5 = v4->w.type & 0x1F;
    switch (v5)
    {
    case 14:
        WriteId(v4->u.o.u.size, 1u, memFile);
        goto LABEL_14;
    case 15:
        UsedSize = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("VAR_NOTIFY_THREAD", UsedSize);
        WriteId(v4->u.o.u.size, 2u, memFile);
        SafeWriteString((unsigned int)v4->w.status >> 8, memFile);
        v7 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v7);
        goto LABEL_14;
    case 16:
        WriteId(v4->u.o.u.size, 3u, memFile);
        v8 = 4;
        v18[0].u.intValue = (unsigned int)v4->w.status >> 8;
        goto LABEL_13;
    case 17:
        WriteId(v4->u.o.u.size, 4u, memFile);
        WriteId((unsigned __int16)((unsigned int)v4->w.status >> 8), 0, memFile);
        goto LABEL_14;
    case 19:
        WriteId(v4->u.o.u.size, 5u, memFile);
        goto LABEL_14;
    case 20:
        //HIBYTE(v18[0].u.floatValue) = -96;
        v18[0].u.floatValue = (v18[0].u.intValue & 0x00FFFFFF) | ((uint32_t)((-96) & 0xFF) << 24);
        MemFile_WriteData(memFile, 1, v18);
        //HIWORD(v18[0].u.floatValue) = v4->u.o.u.size;
        v18[0].u.floatValue = (v18[0].u.intValue & 0x0000FFFF) | ((uint32_t)(v4->u.o.u.size & 0xFFFF) << 16);
        MemFile_WriteData(memFile, 2, v18);
        v8 = 2;
        //HIWORD(v18[0].u.floatValue) = (unsigned int)v4->w.u.intValue >> 8;
        v18[0].u.floatValue = (v18[0].u.intValue & 0x0000FFFF) | ((uint32_t)(((unsigned int)v4->w.status >> 8) & 0xFFFF) << 16);
        goto LABEL_13;
    default:
        v8 = 1;
        //HIBYTE(v18[0].u.floatValue) = 8 * v5;
        v18[0].u.floatValue = (v18[0].u.intValue & 0x00FFFFFF) | ((uint32_t)((8 * v5) & 0xFF) << 24);
    LABEL_13:
        MemFile_WriteData(memFile, v8, v18);
    LABEL_14:
        v9 = v5 == 21;
        v10 = MemFile_GetUsedSize(memFile);
        //ProfMem_Begin("children", v10);
        v11 = 0;
        for (i = FindFirstSibling(parentId); i; i = FindNextSibling(i))
            ++v11;
        //HIWORD(v18[0].u.floatValue) = v11;
        v18[0].u.floatValue = (v18[0].u.intValue & 0x0000FFFF) | ((uint32_t)(v11 & 0xFFFF) << 16);
        MemFile_WriteData(memFile, 2, v18);
        for (j = FindLastSibling(parentId); j; j = FindPrevSibling(j))
        {
            v14 = (VariableValueInternal *)((char *)&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN]
                + __ROL4__(scrVarGlob.variableList[j + VARIABLELIST_CHILD_BEGIN].hash.id, 4));
            v15 = v14->w.status & 0x60;
            if (!v15 || v15 == 96)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    715,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL");
            if (IsObject(v14))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    716,
                    0,
                    "%s",
                    "!IsObject( entryValue )");
            w = v14->w;
            v18[0].u.intValue = v14->u.u.intValue;
            v18[0].type = (Vartype_t)(w.type & 0x1F);
            DoSaveEntry(v18, (VariableValue *)((unsigned int)w.status >> 8), v9, memFile);
        }
        v17 = MemFile_GetUsedSize(memFile);
        //ProfMem_End(v17);
        return;
    }
}

int __cdecl Scr_ConvertThreadToSave(unsigned __int16 handle)
{
    int v1; // r31
    int v3; // r31

    v1 = handle;
    if (!handle)
        return 0;
    AddSaveObject(handle);
    v3 = v1;
    if (!scrVarPub.saveIdMap[v3])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            736,
            0,
            "%s",
            "scrVarPub.saveIdMap[handle]");
    return scrVarPub.saveIdMap[v3];
}

void __cdecl WriteGameEntry(MemoryFile *memFile)
{
    DoSaveEntryInternal(
        scrVarGlob.variableList[scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN].w.type & 0x1F,
        (VariableUnion *)scrVarGlob.variableList[scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN].u.u.intValue,
        memFile);
}

void __cdecl Scr_SavePost(MemoryFile *memFile)
{
    unsigned int UsedSize; // r3
    unsigned int v3; // r29
    unsigned __int16 *v4; // r28
    unsigned int v5; // r3
    int v6; // r30
    unsigned __int16 *p_entArrayId; // r29
    unsigned int v8[12]; // [sp+50h] [-30h] BYREF

    memset(idHistory, 0, 0x20u);
    g_idHistoryIndex = 0;
    v8[0] = scrVarPub.time;
    MemFile_WriteData(memFile, 4, v8);
    //HIWORD(v8[0]) = scrVarPub.savecount;
    v8[0] = (v8[0] & 0x0000FFFF) | ((uint32_t)(scrVarPub.savecount & 0xFFFF) << 16);
    MemFile_WriteData(memFile, 2, v8);
    UsedSize = MemFile_GetUsedSize(memFile);
    //ProfMem_Begin("DoSaveObjectInfo", UsedSize);
    v3 = 1;
    if (scrVarPub.savecount)
    {
        v4 = &scrVarPub.saveIdMapRev[1];
        do
        {
            DoSaveObjectInfo(*v4, memFile);
            ++v3;
            ++v4;
        } while (v3 <= scrVarPub.savecount);
    }
    v5 = MemFile_GetUsedSize(memFile);
    //ProfMem_End(v5);
    DoSaveEntryInternal(
        scrVarGlob.variableList[scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN].w.type & VAR_MASK,
        (VariableUnion *)scrVarGlob.variableList[scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN].u.u.intValue,
        memFile);
    WriteId(scrVarPub.levelId, 0, memFile);
    WriteId(scrVarPub.animId, 0, memFile);
    WriteId(scrVarPub.timeArrayId, 0, memFile);
    WriteId(scrVarPub.pauseArrayId, 0, memFile);
    WriteId(scrVarPub.freeEntList, 0, memFile);
    v6 = 4;
    p_entArrayId = &g_classMap[0].entArrayId;
    do
    {
        WriteId(*p_entArrayId, 0, memFile);
        --v6;
        p_entArrayId += 6;
    } while (v6);
}

void __cdecl AddSaveStack(const VariableStackBuffer *stackBuf)
{
    int size; // r9
    CONST char *buf; // r31
    int v4; // r10
    __int16 v5; // r29
    const VariableStackBuffer *v6; // r3

    AddSaveObject(stackBuf->localId);
    size = stackBuf->size;
    buf = stackBuf->buf;
    if (size)
    {
        do
        {
            v4 = (unsigned __int8)*buf;
            v5 = size - 1;
            v6 = *(const VariableStackBuffer **)(buf + 1);
            buf += 5;
            if (v4 == 1)
            {
                AddSaveObject((unsigned int)v6);
            }
            else if (v4 == 10)
            {
                AddSaveStack(v6);
            }
            //LOWORD(size) = v5;
            size = (size & 0xFFFF0000) | ((uint32_t)v5 & 0xFFFF);

        } while (v5);
    }
}

void __cdecl AddSaveEntry(unsigned int type, const VariableStackBuffer *u)
{
    if (type == 1)
    {
        AddSaveObject((unsigned int)u);
    }
    else if (type == 10)
    {
        AddSaveStack(u);
    }
}

void __cdecl Scr_SavePre(int sys)
{
    int v2; // r30
    unsigned __int16 *p_entArrayId; // r29
    VariableValueInternal *v4; // r11
    const VariableStackBuffer *stackValue; // r3
    int v6; // r11

    if (!scrVarPub.timeArrayId)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 871, 0, "%s", "scrVarPub.timeArrayId");
    if (!CheckReferences())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 874, 0, "%s", "CheckReferences()");
    if (sys != 1)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 876, 0, "%s", "sys == SCR_SYS_GAME");
    Scr_RemoveDebuggerRefs();
    Com_Memset(scrVarPub.saveIdMap, 0, 0x10000);
    Com_Memset(scrVarPub.saveIdMapRev, 0, 0x10000);
    scrVarPub.savecount = 0;
    AddSaveObject(scrVarPub.levelId);
    AddSaveObject(scrVarPub.animId);
    AddSaveObject(scrVarPub.timeArrayId);
    AddSaveObject(scrVarPub.pauseArrayId);
    AddSaveObject(scrVarPub.freeEntList);
    v2 = 4;
    p_entArrayId = &g_classMap[0].entArrayId;
    do
    {
        AddSaveObject(*p_entArrayId);
        --v2;
        p_entArrayId += 6;
    } while (v2);
    v4 = &scrVarGlob.variableList[scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN];
    stackValue = v4->u.u.stackValue;
    v6 = v4->w.type & VAR_MASK;
    if (v6 == VAR_POINTER)
    {
        AddSaveObject((unsigned int)stackValue);
    }
    else if (v6 == 10)
    {
        AddSaveStack(stackValue);
    }
}

