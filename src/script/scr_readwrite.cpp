#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "scr_readwrite.h"
#include "scr_main.h"

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

    MemFile_ReadData(memFile, 4, &v4);
    v5 = v4;
    if ((LODWORD(v4) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\../universal/memfile.h", 173, 0, "%s", "!IS_NAN(value)");
    v6 = v4;
    MemFile_ReadData(memFile, 4, &v4);
    v5 = v4;
    if ((LODWORD(v4) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\../universal/memfile.h", 173, 0, "%s", "!IS_NAN(value)");
    v7 = v4;
    MemFile_ReadData(memFile, 4, &v4);
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

    MemFile_ReadData(memFile, 4, &v4);
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

unsigned int __cdecl Scr_CheckIdHistory(unsigned int index)
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
        v6 = word_83711768;
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
        v4 = &word_83711768[v3];
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

void __cdecl WriteId(unsigned int id, unsigned int opcode, MemoryFile *memFile)
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
    HIBYTE(v9[0]) = v7 + opcode;
    MemFile_WriteData(memFile, 1, v9);
    if (!v7)
    {
        v9[0] = v6;
        MemFile_WriteData(memFile, 2, v9);
    }
    v8 = ((_BYTE)g_idHistoryIndex - 1) & 0xF;
    word_83711768[g_idHistoryIndex] = v6;
    g_idHistoryIndex = v8;
}

unsigned int __cdecl Scr_ReadId(MemoryFile *memFile, unsigned int opcode)
{
    unsigned int v2; // r11
    unsigned int v3; // r29
    unsigned int v4; // r30
    unsigned int result; // r3
    __int16 v6; // [sp+50h] [-30h] BYREF

    if (opcode >> 3)
    {
        v2 = g_idHistoryIndex;
        v3 = *(unsigned __int16 *)((char *)word_83711768 + ((2 * ((((opcode >> 3) + 1) >> 1) + g_idHistoryIndex)) & 0x1E))
            + ((opcode >> 3) & 1);
    }
    else
    {
        MemFile_ReadData(memFile, 2, &v6);
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
    word_83711768[v2] = v3;
    g_idHistoryIndex = ((_BYTE)v2 - 1) & 0xF;
    return result;
}

void __cdecl WriteStack(const VariableStackBuffer *stackBuf, MemoryFile *memFile)
{
    int v4; // r30
    __int16 v5; // r11
    char *buf; // r31
    unsigned int v7; // r3
    __int16 v8; // r30
    VariableUnion *v9; // r4
    _WORD v10[24]; // [sp+50h] [-30h] BYREF

    if (!stackBuf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 236, 0, "%s", "stackBuf");
    v10[0] = stackBuf->size;
    v4 = v10[0];
    MemFile_WriteData(memFile, 2, v10);
    WriteCodepos(stackBuf->pos, memFile);
    WriteId(stackBuf->localId, 0, memFile);
    HIBYTE(v10[0]) = stackBuf->time;
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
    v6 = MT_Alloc(v4, 1);
    ++scrVarPub.numScriptThreads;
    v6[2] = v2;
    v6[3] = v5;
    *(unsigned int *)v6 = Scr_ReadCodepos(memFile);
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
        value->type = 1;
        value->u.intValue = Scr_ReadId(memFile, v4);
    }
    else
    {
        v5 = v7[0] >> 3;
        value->type = v5;
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
            MemFile_ReadData(memFile, 4, &v8);
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
        MemFile_ReadData(memFile, 1, &v11);
        switch (HIBYTE(v11) & 7)
        {
        case 0:
            result = 0x800000;
            break;
        case 1:
            MemFile_ReadData(memFile, 1, &v11);
            result = SHIBYTE(v11) + 0x800000;
            break;
        case 2:
            MemFile_ReadData(memFile, 2, &v11);
            result = SHIWORD(v11) + 0x800000;
            break;
        case 3:
            MemFile_ReadData(memFile, 4, &v11);
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
        MemFile_ReadData(v6, 1, &v11);
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
    VariableValueInternal *v4; // r31
    int v5; // r30
    ObjectInfo::<unnamed_tag> v6; // r3
    VariableValueInternal::<unnamed_tag> w; // r11
    ObjectInfo::<unnamed_tag> v8; // r3
    VariableValueInternal::<unnamed_tag> v9; // r11
    unsigned int v10; // r29
    unsigned int v11; // r11
    bool v12; // r29
    int v13; // r24
    VariableValue *v14; // r31
    VariableValueInternal *v15; // r30
    unsigned int v16; // r4
    __int64 v17; // r3
    int v18; // r11
    int type; // r31
    int v20; // r11
    __int64 v21; // [sp+50h] [-90h] BYREF
    VariableValue v22; // [sp+58h] [-88h] BYREF

    v2 = parentId;
    v4 = &scrVarGlob.variableList[parentId + 1];
    if ((v4->w.u.intValue & 0x60) != 0x60)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            779,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL");
    if (!IsObject(v4))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 780, 0, "%s", "IsObject( parentValue )");
    MemFile_ReadData(memFile, 1, &v21);
    switch (HIBYTE(v21) & 7)
    {
    case 1:
        v5 = 14;
        v4->u.o.u.size = Scr_ReadId(memFile, HIBYTE(v21));
        break;
    case 2:
        v5 = 15;
        v6.size = Scr_ReadId(memFile, HIBYTE(v21));
        w = v4->w;
        v4->u.o.u = v6;
        if ((w.u.intValue & 0xFFFFFF00) != 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                795,
                0,
                "%s",
                "!(parentValue->w.notifyName & VAR_NAME_HIGH_MASK)");
        v4->w.u.intValue |= (Scr_ReadOptionalString(memFile) << 8) & 0xFFFF00;
        break;
    case 3:
        v5 = 16;
        v8.size = Scr_ReadId(memFile, HIBYTE(v21));
        v9 = v4->w;
        v4->u.o.u = v8;
        if ((v9.u.intValue & 0xFFFFFF00) != 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                803,
                0,
                "%s",
                "!(parentValue->w.waitTime & VAR_NAME_HIGH_MASK)");
        MemFile_ReadData(memFile, 4, &v21);
        v4->w.u.intValue |= HIDWORD(v21) << 8;
        break;
    case 4:
        v5 = 17;
        v4->u.o.u.size = Scr_ReadId(memFile, HIBYTE(v21));
        MemFile_ReadData(memFile, 1, &v21);
        v10 = HIBYTE(v21);
        if ((v4->w.u.intValue & 0xFFFFFF00) != 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                812,
                0,
                "%s",
                "!(parentValue->w.parentLocalId & VAR_NAME_HIGH_MASK)");
        v4->w.u.intValue |= Scr_ReadId(memFile, v10) << 8;
        break;
    case 5:
        v5 = 19;
        v4->u.o.u.size = Scr_ReadId(memFile, HIBYTE(v21));
        break;
    default:
        v5 = HIBYTE(v21) >> 3;
        if (v5 == 20)
        {
            MemFile_ReadData(memFile, 2, &v21);
            v11 = v4->w.u.intValue & 0xFFFFFF00;
            v4->u.o.u.size = HIWORD(v21);
            if (v11)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    827,
                    0,
                    "%s",
                    "!(parentValue->w.classnum & VAR_NAME_HIGH_MASK)");
            MemFile_ReadData(memFile, 2, &v21);
            v4->w.u.intValue |= SHIWORD(v21) << 8;
        }
        else if (v5 == 21)
        {
            v4->u.o.u.size = 0;
        }
        break;
    }
    if ((v5 & 0xFFFFFFE0) != 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 837, 0, "%s", "!(type & ~VAR_MASK)");
    v12 = v5 == 21;
    v4->w.u.intValue = v4->w.u.intValue & 0xFFFFFFE0 | v5;
    MemFile_ReadData(memFile, 2, &v21);
    if (HIWORD(v21))
    {
        v13 = HIWORD(v21);
        do
        {
            v14 = (VariableValue *)Scr_DoLoadEntry(&v22, v12, memFile);
            v15 = &scrVarGlob.variableList[GetVariable(v2, (unsigned int)v14) + 32770];
            if (v12)
            {
                LODWORD(v17) = Scr_GetArrayIndexValue(v14, v16);
                v21 = v17;
                RemoveRefToValue(v17, (VariableUnion *)HIDWORD(v17));
            }
            v18 = v15->w.u.intValue & 0x60;
            if (!v18 || v18 == 96)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    856,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL");
            if ((v15->w.u.intValue & 0x1F) != 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    857,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_UNDEFINED");
            type = v22.type;
            if ((v22.type & 0xFFFFFFE0) != 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    859,
                    0,
                    "%s",
                    "!(value.type & ~VAR_MASK)");
            if ((v15->w.u.intValue & 0x1F) != 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                    860,
                    0,
                    "%s",
                    "!(entryValue->w.type & VAR_MASK)");
            --v13;
            v20 = type | v15->w.u.intValue;
            v15->u.u.intValue = v22.u.intValue;
            v15->w.u.intValue = v20;
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
    v4.intValue = (int)v5.u;
    scrVarGlob.variableList[gameId + 32770].w.u.intValue |= type;
    scrVarGlob.variableList[gameId + 32770].u.u = v4;
}

void __cdecl Scr_SaveShutdown(bool savegame)
{
    char v2; // r20
    unsigned __int16 *v3; // r25
    int v4; // r30
    VariableValueInternal::<unnamed_tag> *p_w; // r27
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
            if ((p_w->u.intValue & 0x60) != 0)
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
                if (!*v3 && (p_w->u.intValue & 0x1F) != 0x15)
                {
                    if (!v2)
                    {
                        v2 = 1;
                        Com_Printf(23, "\n****script variable cyclic leak*****\n");
                    }
                    Scr_PrintPrevCodePos(23, v7, 0);
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
            Com_Error(ERR_DROP, byte_82055B08);
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
    memset(word_83711768, 0, 0x20u);
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
        ProfMem_Begin("pointer", UsedSize);
        WriteId((unsigned int)u, 1u, memFile);
        v7 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v7);
    }
    else
    {
        v8 = MemFile_GetUsedSize(memFile);
        ProfMem_Begin("type", v8);
        v30[0] = 8 * type;
        MemFile_WriteData(memFile, 1, v30);
        v9 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v9);
        switch (type)
        {
        case 0u:
        case 8u:
            return;
        case 2u:
        case 3u:
            v19 = MemFile_GetUsedSize(memFile);
            ProfMem_Begin("string", v19);
            v20 = SL_ConvertToString((unsigned __int16)u);
            MemFile_WriteCString(memFile, v20);
            v21 = MemFile_GetUsedSize(memFile);
            ProfMem_End(v21);
            break;
        case 4u:
            WriteVector(&u->floatValue, memFile, v18, v17, v16, v15, v14, v13, v12, v11);
            break;
        case 5u:
            WriteFloat(*(float *)&u, v10, memFile, v17, v16, v15, v14, v13, v12, v11, v26, v27, v28, v29);
            break;
        case 6u:
            v22 = MemFile_GetUsedSize(memFile);
            ProfMem_Begin("int", v22);
            v31[0] = u;
            MemFile_WriteData(memFile, 4, v31);
            v23 = MemFile_GetUsedSize(memFile);
            ProfMem_End(v23);
            break;
        case 7u:
        case 9u:
            WriteCodepos((const char *)u, memFile);
            break;
        case 0xAu:
            v24 = MemFile_GetUsedSize(memFile);
            ProfMem_Begin("stack", v24);
            WriteStack((const VariableStackBuffer *)u, memFile);
            v25 = MemFile_GetUsedSize(memFile);
            ProfMem_End(v25);
            break;
        case 0xBu:
            v31[0] = u;
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
    WriteToMemoryCardFile(save->f, buffer, len);
}

void __cdecl Scr_SaveSourceImmediate(SaveImmediate *save)
{
    int v2; // r9
    unsigned int sourceBufferLookupLen; // r11
    bool *p_archive; // r10
    unsigned int v5; // r28
    int v6; // r29
    SourceBufferInfo *v7; // r31
    signed int len; // r5
    int v9; // [sp+50h] [-40h] BYREF

    if (scrVarPub.developer)
    {
        v2 = 0;
        v9 = 0;
        sourceBufferLookupLen = scrParserPub.sourceBufferLookupLen;
        if (scrParserPub.sourceBufferLookupLen)
        {
            p_archive = &scrParserPub.sourceBufferLookup->archive;
            do
            {
                if (*p_archive)
                    v9 = ++v2;
                --sourceBufferLookupLen;
                p_archive += 44;
            } while (sourceBufferLookupLen);
        }
        WriteToMemoryCardFile(save->f, &v9, 4u);
        v5 = 0;
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
                ++v5;
                ++v6;
            } while (v5 < scrParserPub.sourceBufferLookupLen);
        }
    }
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
    MemFile_ReadData(memFile, 1, &v12);
    MemFile_ReadData(memFile, 1, &scrVarPub.developer_script);
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
    MemFile_ReadData(memFile, 1, &scrVarPub.developer_script);
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
    char *buf; // r31
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
            AddSaveEntryInternal(v4, v6);
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
    ProfMem_Begin("DoSaveEntry", UsedSize);
    v9 = MemFile_GetUsedSize(memFile);
    ProfMem_Begin("DoSaveEntryInternal", v9);
    DoSaveEntryInternal(value->type, (VariableUnion *)value->u.intValue, memFile);
    v10 = MemFile_GetUsedSize(memFile);
    ProfMem_End(v10);
    if (!isArray)
    {
        v12 = MemFile_GetUsedSize(memFile);
        ProfMem_Begin("non-array", v12);
        if (((unsigned int)name & 0xFF000000) != 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                402,
                0,
                "%s\n\t(name) = %i",
                "(!(name & 0xFF000000))",
                name);
        HIBYTE(v27[0]) = (_BYTE)name;
        MemFile_WriteData(memFile, 1, v27);
        HIBYTE(v27[0]) = BYTE2(name);
        MemFile_WriteData(memFile, 1, v27);
        HIBYTE(v27[0]) = BYTE1(name);
        MemFile_WriteData(memFile, 1, v27);
        v13 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v13);
        v14 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v14);
        return;
    }
    ArrayIndexValue = Scr_GetArrayIndexValue(name, v11);
    v28 = *(_QWORD *)&ArrayIndexValue;
    if (ArrayIndexValue == (VariableValue *)1)
    {
        WriteId(HIDWORD(v28), 5u, memFile);
    LABEL_23:
        v26 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v26);
        return;
    }
    if (ArrayIndexValue == (VariableValue *)2)
    {
        v22 = MemFile_GetUsedSize(memFile);
        ProfMem_Begin("string array index", v22);
        HIBYTE(v27[0]) = 4;
        MemFile_WriteData(memFile, 1, v27);
        v23 = SL_ConvertToString(HIDWORD(v28) & 0xFFFF);
        MemFile_WriteCString(memFile, v23);
        v24 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v24);
        v25 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v25);
        return;
    }
    if (ArrayIndexValue != (VariableValue *)6)
    {
        if (!alwaysfails)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 449, 1, "bad case");
            v16 = MemFile_GetUsedSize(memFile);
            ProfMem_End(v16);
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
                HIBYTE(v27[0]) = 3;
                MemFile_WriteData(memFile, 1, v27);
                v27[0] = v17;
                MemFile_WriteData(memFile, 4, v27);
                v21 = MemFile_GetUsedSize(memFile);
                ProfMem_End(v21);
            }
            else
            {
                HIBYTE(v27[0]) = 2;
                MemFile_WriteData(memFile, 1, v27);
                HIWORD(v27[0]) = v17;
                MemFile_WriteData(memFile, 2, v27);
                v20 = MemFile_GetUsedSize(memFile);
                ProfMem_End(v20);
            }
        }
        else
        {
            HIBYTE(v27[0]) = 1;
            MemFile_WriteData(memFile, 1, v27);
            HIBYTE(v27[0]) = v17;
            MemFile_WriteData(memFile, 1, v27);
            v19 = MemFile_GetUsedSize(memFile);
            ProfMem_End(v19);
        }
    }
    else
    {
        HIBYTE(v27[0]) = 0;
        MemFile_WriteData(memFile, 1, v27);
        v18 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v18);
    }
}

void __cdecl AddSaveObjectChildren(unsigned int parentId)
{
    VariableValueInternal *v2; // r23
    int v3; // r24
    unsigned int i; // r29
    VariableValueInternal *v5; // r30
    unsigned int v6; // r4
    int v7; // r2
    VariableValueInternal::<unnamed_tag> u; // r3
    int v9; // r11
    VariableValueInternal::<unnamed_tag> w; // r11
    int v11; // r9
    int size; // r9

    v2 = &scrVarGlob.variableList[parentId + 1];
    if ((v2->w.u.intValue & 0x60) == 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            557,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE");
    if (!IsObject(v2))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 558, 0, "%s", "IsObject( parentValue )");
    v3 = v2->w.u.intValue & 0x1F;
    for (i = FindLastSibling(parentId); i; i = FindPrevSibling(i))
    {
        v5 = (VariableValueInternal *)((char *)&scrVarGlob.variableList[32770]
            + __ROL4__(scrVarGlob.variableList[i + 32770].hash.id, 4));
        if (IsObject(v5))
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
                567,
                0,
                "%s",
                "!IsObject( entryValue )");
        if (v3 == 21
            && Scr_GetArrayIndexValue((VariableValue *)((unsigned int)v5->w.u.intValue >> 8), v6) == (VariableValue *)1
            && v7
            && !scrVarPub.saveIdMap[v7])
        {
            scrVarPub.saveIdMap[v7] = ++scrVarPub.savecount;
            *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = v7;
        }
        u = v5->u;
        v9 = v5->w.u.intValue & 0x1F;
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
    switch (v3)
    {
    case 14:
    case 15:
    case 16:
    case 19:
        goto LABEL_24;
    case 17:
        w = v2->w;
        v11 = (unsigned __int16)((unsigned int)w.u.intValue >> 8);
        if ((unsigned __int16)((unsigned int)w.u.intValue >> 8) && !scrVarPub.saveIdMap[v11])
        {
            scrVarPub.saveIdMap[v11] = ++scrVarPub.savecount;
            *(unsigned __int16 *)((char *)scrVarPub.saveIdMapRev + __ROL4__(scrVarPub.savecount, 1)) = v11;
        }
    LABEL_24:
        size = v2->u.o.u.size;
        if (v2->u.o.u.size)
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
    VariableValueInternal::<unnamed_tag> w; // r11
    unsigned int v17; // r3
    VariableValue v18[12]; // [sp+50h] [-60h] BYREF

    v4 = &scrVarGlob.variableList[parentId + 1];
    if ((v4->w.u.intValue & 0x60) != 0x60)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp",
            655,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL");
    if (!IsObject(v4))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_readwrite.cpp", 656, 0, "%s", "IsObject( parentValue )");
    v5 = v4->w.u.intValue & 0x1F;
    switch (v5)
    {
    case 14:
        WriteId(v4->u.o.u.size, 1u, memFile);
        goto LABEL_14;
    case 15:
        UsedSize = MemFile_GetUsedSize(memFile);
        ProfMem_Begin("VAR_NOTIFY_THREAD", UsedSize);
        WriteId(v4->u.o.u.size, 2u, memFile);
        SafeWriteString((unsigned int)v4->w.u.intValue >> 8, memFile);
        v7 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v7);
        goto LABEL_14;
    case 16:
        WriteId(v4->u.o.u.size, 3u, memFile);
        v8 = 4;
        v18[0].u.intValue = (unsigned int)v4->w.u.intValue >> 8;
        goto LABEL_13;
    case 17:
        WriteId(v4->u.o.u.size, 4u, memFile);
        WriteId((unsigned __int16)((unsigned int)v4->w.u.intValue >> 8), 0, memFile);
        goto LABEL_14;
    case 19:
        WriteId(v4->u.o.u.size, 5u, memFile);
        goto LABEL_14;
    case 20:
        HIBYTE(v18[0].u.floatValue) = -96;
        MemFile_WriteData(memFile, 1, v18);
        HIWORD(v18[0].u.floatValue) = v4->u.o.u.size;
        MemFile_WriteData(memFile, 2, v18);
        v8 = 2;
        HIWORD(v18[0].u.floatValue) = (unsigned int)v4->w.u.intValue >> 8;
        goto LABEL_13;
    default:
        v8 = 1;
        HIBYTE(v18[0].u.floatValue) = 8 * v5;
    LABEL_13:
        MemFile_WriteData(memFile, v8, v18);
    LABEL_14:
        v9 = v5 == 21;
        v10 = MemFile_GetUsedSize(memFile);
        ProfMem_Begin("children", v10);
        v11 = 0;
        for (i = FindFirstSibling(parentId); i; i = FindNextSibling(i))
            ++v11;
        HIWORD(v18[0].u.floatValue) = v11;
        MemFile_WriteData(memFile, 2, v18);
        for (j = FindLastSibling(parentId); j; j = FindPrevSibling(j))
        {
            v14 = (VariableValueInternal *)((char *)&scrVarGlob.variableList[32770]
                + __ROL4__(scrVarGlob.variableList[j + 32770].hash.id, 4));
            v15 = v14->w.u.intValue & 0x60;
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
            v18[0].type = w.o.u.size & 0x1F;
            DoSaveEntry(v18, (VariableValue *)((unsigned int)w.u.intValue >> 8), v9, memFile);
        }
        v17 = MemFile_GetUsedSize(memFile);
        ProfMem_End(v17);
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
        scrVarGlob.variableList[scrVarPub.gameId + 32770].w.u.intValue & 0x1F,
        (VariableUnion *)scrVarGlob.variableList[scrVarPub.gameId + 32770].u.u.intValue,
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

    memset(word_83711768, 0, 0x20u);
    g_idHistoryIndex = 0;
    v8[0] = scrVarPub.time;
    MemFile_WriteData(memFile, 4, v8);
    HIWORD(v8[0]) = scrVarPub.savecount;
    MemFile_WriteData(memFile, 2, v8);
    UsedSize = MemFile_GetUsedSize(memFile);
    ProfMem_Begin("DoSaveObjectInfo", UsedSize);
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
    ProfMem_End(v5);
    DoSaveEntryInternal(
        scrVarGlob.variableList[scrVarPub.gameId + 32770].w.u.intValue & 0x1F,
        (VariableUnion *)scrVarGlob.variableList[scrVarPub.gameId + 32770].u.u.intValue,
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
    char *buf; // r31
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
            LOWORD(size) = v5;
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
    v4 = &scrVarGlob.variableList[scrVarPub.gameId + 32770];
    stackValue = v4->u.u.stackValue;
    v6 = v4->w.u.intValue & 0x1F;
    if (v6 == 1)
    {
        AddSaveObject((unsigned int)stackValue);
    }
    else if (v6 == 10)
    {
        AddSaveStack(stackValue);
    }
}

